// ptgfx_shadow.cpp - real-time shadows of characters and the world.
//
// The engine sends screen-space vertices (x, y in pixels, z, rhw) and does its
// 3D maths on the CPU, but each vertex's 3D position can be rebuilt from them:
// camera z = 1/rhw, x and y from the perspective divide (PtGfx_ShadowCamera gives
// the projection), then the inverse camera rotation and the camera position give
// the world position. That happens in the shaders (PTSHADOW_GLSL).
//
// Each frame the game draws the characters first (DrawPat3D), then the map
// (DisplayStage):
//  - PTSHADOW_CASTER: the characters' triangles are kept (Chars)
//  - PTSHADOW_RECEIVER: first the shadow map is drawn from the sun - this frame's
//    characters plus the map triangles kept in the frame before (static, so a
//    frame late does not show) - orthographic, centred on the player. Then the map
//    is drawn: the device shader darkens every pixel that something stands between
//    it and the sun (ptgfx_device.cpp), and its triangles are kept for next frame.
// Cut-out textures (leaves, fences) are alpha-tested in the shadow map too.
// World positions are relative to the shadow centre to keep the floats small.

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "ptgfx_internal.h"
#ifdef PT_WEB
#include <emscripten.h>
#endif

PtShadowState g_PtShadow;

// Half size of the shadowed square around the player and depth range of the
// shadow map, in world units (deep enough for a sun 18 degrees up, weather.cpp).
// The light's direction comes from PtGfx_ShadowLight (the sun or the moon moving
// with the game's clock); this one until it is first set.
static const float s_DefaultDir[3] = { 0.35f, -1.0f, 0.50f };
#define PTSHADOW_HALF_SIZE	1000.0f
#define PTSHADOW_DEPTH		4000.0f

static const char *s_CasterVS =
	"in vec4 aPos; in vec2 aUV;\n"
	"out vec2 vUV;\n"
	PTSHADOW_GLSL
	"void main() { vUV = aUV; gl_Position = vec4( ptSunPos( aPos ), 1.0 ); }\n";

// map meshes: world positions relative to the mesh origin; uOffset = origin - centre
static const char *s_MeshVS =
	"in vec3 aPos; in vec2 aUV;\n"
	"out vec2 vUV;\n"
	"uniform vec3 uOffset; uniform vec3 uSLightR; uniform vec3 uSLightU; uniform vec3 uSLightD;\n"
	"void main() {\n"
	"	vec3 w = aPos + uOffset;\n"
	"	vUV = aUV;\n"
	"	gl_Position = vec4( dot( w, uSLightR ), dot( w, uSLightU ), dot( w, uSLightD ), 1.0 );\n"
	"}\n";

static const char *s_CasterFS =
	"in vec2 vUV;\n"
	"uniform sampler2D uTex; uniform int uAlphaTest;\n"
	"out vec4 oColor;\n"
	"void main() {\n"
	"	if ( uAlphaTest!=0 && texture( uTex, vUV ).a<0.5 ) discard;\n"
	"	oColor = vec4( 1.0 );\n"
	"}\n";

static void Normalize( float v[3] )
{
	float l = sqrtf( v[0]*v[0] + v[1]*v[1] + v[2]*v[2] );
	if ( l>0 ) { v[0] /= l; v[1] /= l; v[2] /= l; }
}

// The shadow map lies on the ground (world x, z), not across the light: a point
// is slid along the light down to height 0 (x - y*dx/dy, z - y*dz/dy), depth is
// along the light. The map's pixel grid stays put on the ground however the light
// turns, so a moving sun moves only the shadows, smoothly - a map turning with the
// light made every edge shimmer. Low light needs no bigger map either.
static void SetSun()
{
	PtShadowState &S = g_PtShadow;
	float d[3] = { S.LightDir[0], S.LightDir[1], S.LightDir[2] };
	Normalize( d );
	if ( d[1]>-0.2f ) { d[1] = -0.2f; Normalize( d ); }		// never level (weather.cpp keeps it 18 degrees up)
	float r[3] = { 1, -d[0]/d[1], 0 }, u[3] = { 0, -d[2]/d[1], 1 };
	for ( int i=0; i<3; i++ ) {
		S.LightR[i] = r[i] / PTSHADOW_HALF_SIZE;
		S.LightU[i] = u[i] / PTSHADOW_HALF_SIZE;
		S.LightD[i] = d[i] / PTSHADOW_DEPTH;
		S.SunR[i] = r[i];
		S.SunU[i] = u[i];
	}
}

// The shadow centre moved in whole shadow-map pixels only, so the map's pixel
// grid stays put on the ground and edges do not crawl while walking. Moving the
// centre along x (z) moves its map position along the map's x (y) only.
static void SnapCentre( double c[3] )
{
	PtShadowState &S = g_PtShadow;
	const double texel = 2.0*PTSHADOW_HALF_SIZE / S.MapSize;
	double r = c[0]*S.SunR[0] + c[1]*S.SunR[1] + c[2]*S.SunR[2];
	double u = c[0]*S.SunU[0] + c[1]*S.SunU[1] + c[2]*S.SunU[2];
	c[0] += floor( r/texel + 0.5 )*texel - r;
	c[2] += floor( u/texel + 0.5 )*texel - u;
}

// Shadow strength, read every frame (the page can change Module.ptShadowDark live),
// times the light's own (low sun, moon, clouds).
static void ReadDark()
{
#ifdef PT_WEB
	g_PtShadow.Dark = (float)EM_ASM_DOUBLE({ var d = Module['ptShadowDark']; return typeof d=='number' ? d : 0.6; });
#else
	g_PtShadow.Dark = 0.6f;
#endif
	g_PtShadow.Dark *= g_PtShadow.Strength;
}

// (Re)allocates the shadow map at size x size (the framebuffer keeps the texture
// attached). Keeps the old size if the GPU cannot make the new one.
static BOOL SetMapSize( int size )
{
	PtShadowState &S = g_PtShadow;
	if ( size>S.MaxMapSize ) size = S.MaxMapSize;
	if ( size==S.MapSize ) return TRUE;
	while ( glGetError()!=GL_NO_ERROR ) ;
	glActiveTexture( GL_TEXTURE0 + PTSHADOW_TEX_UNIT );
	glBindTexture( GL_TEXTURE_2D, S.DepthTex );
	glTexImage2D( GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, size, size, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, NULL );
	BOOL ok = glGetError()==GL_NO_ERROR;
	if ( !ok && S.MapSize )
		glTexImage2D( GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, S.MapSize, S.MapSize, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, NULL );
	glActiveTexture( GL_TEXTURE0 );
	PtGL_Log( ok ? "shadows: map %dx%d" : "shadows: map %dx%d failed, kept the old size", size, size );
	if ( ok ) S.MapSize = size;
	PtGfx_InvalidateDeviceState();
	return ok;
}

// Shadow quality, read every frame (Module.ptShadowQuality: the map size, live).
static void ReadQuality()
{
#ifdef PT_WEB
	static int s_Asked = 0;					// tried once per change (a failed size is not retried)
	int q = EM_ASM_INT({ var q = Module['ptShadowQuality']; return typeof q=='number' ? q : 0; });
	if ( q<=0 || q==s_Asked ) return;
	s_Asked = q;
	int size = 1024;
	while ( size<q && size<8192 ) size *= 2;
	SetMapSize( size );
#endif
}

void PtGfx_ShadowLight( const float dir[3], float strength, float soft )
{
	PtShadowState &S = g_PtShadow;
	S.Strength = strength<0 ? 0 : strength>1 ? 1 : strength;
	S.Soft = soft<1 ? 1 : soft;
	if ( !memcmp( S.LightDir, dir, sizeof(S.LightDir) ) ) return;
	memcpy( S.LightDir, dir, sizeof(S.LightDir) );
	if ( S.Prog ) SetSun();
}

static BOOL AllocList( PtShadowList &L, int nVerts, int nBatches )
{
	L.nMaxVerts = nVerts;
	L.pVerts = (float *)malloc( sizeof(float)*PTSHADOW_VERT_FLOATS*nVerts );
	L.nMaxBatches = nBatches;
	L.pBatches = (PtShadowBatch *)malloc( sizeof(PtShadowBatch)*nBatches );
	L.nVerts = L.nBatches = 0;
	return L.pVerts && L.pBatches;
}

BOOL PtShadow_Init()
{
	PtShadowState &S = g_PtShadow;
	S.bAllowed = TRUE;
	if ( !S.Strength && !S.Soft ) {				// no light set yet
		memcpy( S.LightDir, s_DefaultDir, sizeof(S.LightDir) );
		S.Strength = S.Soft = 1;
	}
	ReadDark();
#ifdef PT_WEB
	S.bAllowed = EM_ASM_INT({ return Module['ptShadows']===false ? 0 : 1; });		// page setting
	S.bTestAllowed = EM_ASM_INT({ return /[?&]shadowtest=2/.test(location.search) ? 2 : /[?&]shadowtest/.test(location.search) ? 1 : 0; });
#endif
	const char *attribs[] = { "aPos", "aUV" };
	S.Prog = PtGfx_CompileProgram( s_CasterVS, s_CasterFS, attribs, 2 );
	if ( !S.Prog ) { PtGL_Log( "shadows: caster shader failed, shadows off" ); return FALSE; }
	S.u_Rot  = glGetUniformLocation( S.Prog, "uSRot" );
	S.u_Proj = glGetUniformLocation( S.Prog, "uSProj" );
	S.u_Eye  = glGetUniformLocation( S.Prog, "uSEye" );
	S.u_R    = glGetUniformLocation( S.Prog, "uSLightR" );
	S.u_U    = glGetUniformLocation( S.Prog, "uSLightU" );
	S.u_D    = glGetUniformLocation( S.Prog, "uSLightD" );
	S.u_AlphaTest = glGetUniformLocation( S.Prog, "uAlphaTest" );
	glUseProgram( S.Prog );
	glUniform1i( glGetUniformLocation( S.Prog, "uTex" ), 0 );

	S.MeshProg = PtGfx_CompileProgram( s_MeshVS, s_CasterFS, attribs, 2 );
	if ( !S.MeshProg ) { PtGL_Log( "shadows: map mesh shader failed, shadows off" ); S.Prog = 0; return FALSE; }
	S.um_Offset = glGetUniformLocation( S.MeshProg, "uOffset" );
	S.um_R = glGetUniformLocation( S.MeshProg, "uSLightR" );
	S.um_U = glGetUniformLocation( S.MeshProg, "uSLightU" );
	S.um_D = glGetUniformLocation( S.MeshProg, "uSLightD" );
	S.um_AlphaTest = glGetUniformLocation( S.MeshProg, "uAlphaTest" );
	glUseProgram( S.MeshProg );
	glUniform1i( glGetUniformLocation( S.MeshProg, "uTex" ), 0 );

	glGenVertexArrays( 1, &S.VAO );
	glGenBuffers( 1, &S.VBO );

	// depth texture with hardware comparison (sampler2DShadow, filtered = 2x2 PCF)
	// the largest square map the GPU can draw into
	GLint maxTex = 0, maxView[2] = { 0, 0 };
	glGetIntegerv( GL_MAX_TEXTURE_SIZE, &maxTex );
	glGetIntegerv( GL_MAX_VIEWPORT_DIMS, maxView );
	S.MaxMapSize = 1024;
	while ( S.MaxMapSize<8192 && S.MaxMapSize*2<=maxTex && S.MaxMapSize*2<=maxView[0] && S.MaxMapSize*2<=maxView[1] ) S.MaxMapSize *= 2;
	S.MapSize = 0;
	glGenTextures( 1, &S.DepthTex );
	if ( !SetMapSize( PTSHADOW_MAP_SIZE ) ) { PtGL_Log( "shadows: no shadow map, shadows off" ); S.Prog = 0; return FALSE; }
	ReadQuality();
	glActiveTexture( GL_TEXTURE0 + PTSHADOW_TEX_UNIT );
	glBindTexture( GL_TEXTURE_2D, S.DepthTex );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL );
	glActiveTexture( GL_TEXTURE0 );			// the shadow map stays bound to its own unit

	glGenFramebuffers( 1, &S.FBO );
	glBindFramebuffer( GL_FRAMEBUFFER, S.FBO );
	glFramebufferTexture2D( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, S.DepthTex, 0 );
	GLenum none = GL_NONE;
	glDrawBuffers( 1, &none );
	glReadBuffer( GL_NONE );
	BOOL ok = glCheckFramebufferStatus( GL_FRAMEBUFFER )==GL_FRAMEBUFFER_COMPLETE;
	if ( ok ) { glClearDepth( 1.0f ); glClear( GL_DEPTH_BUFFER_BIT ); }
	PtGfx_BindRenderTarget( g_PtGL.pRenderTarget );
	if ( !ok ) { PtGL_Log( "shadows: shadow map framebuffer incomplete, shadows off" ); S.Prog = 0; return FALSE; }

	if ( !AllocList( S.Chars, 300000, 4096 ) || !AllocList( S.World[0], 600000, 8192 ) || !AllocList( S.World[1], 600000, 8192 ) ) {
		S.Prog = 0;
		return FALSE;
	}
	SetSun();
	PtGL_Log( "shadows: %s (map %dx%d, up to %d)", S.bAllowed ? "on" : "off (page setting)", S.MapSize, S.MapSize, S.MaxMapSize );
	return TRUE;
}

// Start of a shadowed frame: the map kept while drawing the last frame becomes
// "the frame before"; new casters start empty.
static void BeginFrame( const double cam[3], const double centre[3] )
{
	PtShadowState &S = g_PtShadow;
	ReadQuality();								// before snapping: the pixel size may change
	S.WorldCur ^= 1;
	S.World[S.WorldCur].nVerts = S.World[S.WorldCur].nBatches = 0;
	S.Chars.nVerts = S.Chars.nBatches = 0;
	double c[3] = { centre[0], centre[1], centre[2] };
	SnapCentre( c );
	for ( int i=0; i<3; i++ ) {
		S.World[S.WorldCur].Cam[i] = S.Chars.Cam[i] = cam[i];
		S.Centre[i] = c[i];
		S.Eye[i] = (float)(cam[i] - c[i]);
	}
	S.bFrame = TRUE;
	S.bMapReady = FALSE;
	S.bCamera = FALSE;
	ReadDark();
}

void PtGfx_ShadowFrame( BOOL bOn, double camX, double camY, double camZ, double centreX, double centreY, double centreZ )
{
	PtShadowState &S = g_PtShadow;
	if ( !( bOn && S.bAllowed && S.Prog ) ) {
		// nothing to carry over into a later frame
		S.bFrame = FALSE;
		S.Role = PTSHADOW_OFF;
		S.World[0].nVerts = S.World[0].nBatches = S.World[1].nVerts = S.World[1].nBatches = 0;
		PtGfx_InvalidateDeviceState();
		return;
	}
	double cam[3] = { camX, camY, camZ }, centre[3] = { centreX, centreY, centreZ };
	BeginFrame( cam, centre );
	S.Role = PTSHADOW_CASTER;
	PtGfx_InvalidateDeviceState();
}

void PtGfx_ShadowCamera( const float m[9], float w, float h, float cx, float cy )
{
	PtShadowState &S = g_PtShadow;
	if ( !S.bFrame ) return;
	S.DbgCam++;
	// camera = offset * m, so offset = camera * inverse(m). In GLSL (mat3 * vec3 sums
	// column j times component j) that is the inverse's rows as columns: its
	// row-major elements in column-major order.
	float det = m[0]*(m[4]*m[8]-m[5]*m[7]) - m[1]*(m[3]*m[8]-m[5]*m[6]) + m[2]*(m[3]*m[7]-m[4]*m[6]);
	if ( fabsf( det )<1e-12f || w==0 || h==0 || cx<=0 || cy<=0 ) {
		if ( !S.DbgCamBad++ ) PtGL_Log( "shadows: camera not usable: det %g w %g h %g centre %g %g | m %g %g %g / %g %g %g / %g %g %g",
			det, w, h, cx, cy, m[0], m[1], m[2], m[3], m[4], m[5], m[6], m[7], m[8] );
		S.bCamera = FALSE;
		return;
	}
	float id = 1.0f/det;
	float *r = S.Rot;
	r[0] =  (m[4]*m[8]-m[5]*m[7])*id; r[1] = -(m[1]*m[8]-m[2]*m[7])*id; r[2] =  (m[1]*m[5]-m[2]*m[4])*id;
	r[3] = -(m[3]*m[8]-m[5]*m[6])*id; r[4] =  (m[0]*m[8]-m[2]*m[6])*id; r[5] = -(m[0]*m[5]-m[2]*m[3])*id;
	r[6] =  (m[3]*m[7]-m[4]*m[6])*id; r[7] = -(m[0]*m[7]-m[1]*m[6])*id; r[8] =  (m[0]*m[4]-m[1]*m[3])*id;
	S.Proj[0] = w; S.Proj[1] = h; S.Proj[2] = cx; S.Proj[3] = cy;
	S.bCamera = TRUE;
}

void PtShadow_Capture( const BYTE *v, DWORD count, int stride, int uvOfs, GLuint tex )
{
	PtShadowState &S = g_PtShadow;
	if ( S.bSkipCapture ) return;							// the map mesh casts by itself
	PtShadowList &L = S.Role==PTSHADOW_CASTER ? S.Chars : S.World[S.WorldCur];
	count -= count % 3;
	if ( !count || L.nVerts + (int)count > L.nMaxVerts ) return;
	PtShadowBatch *b = L.nBatches ? &L.pBatches[L.nBatches-1] : NULL;
	if ( !b || b->Tex!=tex || memcmp( b->Rot, S.Rot, sizeof(S.Rot) ) || memcmp( b->Proj, S.Proj, sizeof(S.Proj) ) ) {
		if ( L.nBatches>=L.nMaxBatches ) return;
		b = &L.pBatches[L.nBatches++];
		b->First = L.nVerts;
		b->Count = 0;
		b->Tex = tex;
		memcpy( b->Rot, S.Rot, sizeof(S.Rot) );
		memcpy( b->Proj, S.Proj, sizeof(S.Proj) );
	}
	float *dst = L.pVerts + L.nVerts*PTSHADOW_VERT_FLOATS;
	for ( DWORD i=0; i<count; i++, dst += PTSHADOW_VERT_FLOATS ) {
		const BYTE *p = v + i*stride;
		memcpy( dst, p, 16 );							// x, y, z, rhw
		if ( uvOfs>=0 ) memcpy( dst+4, p+uvOfs, 8 );	// u, v
		else dst[4] = dst[5] = 0;
	}
	L.nVerts += count;
	b->Count += count;
}

static void DrawList( const PtShadowList &L )
{
	PtShadowState &S = g_PtShadow;
	if ( !L.nVerts ) return;
	// camera of these triangles relative to this frame's shadow centre
	glUniform3f( S.u_Eye, (float)(L.Cam[0]-S.Centre[0]), (float)(L.Cam[1]-S.Centre[1]), (float)(L.Cam[2]-S.Centre[2]) );
	glBufferData( GL_ARRAY_BUFFER, sizeof(float)*PTSHADOW_VERT_FLOATS*L.nVerts, L.pVerts, GL_STREAM_DRAW );
	glVertexAttribPointer( 0, 4, GL_FLOAT, GL_FALSE, sizeof(float)*PTSHADOW_VERT_FLOATS, (void *)0 );
	glVertexAttribPointer( 1, 2, GL_FLOAT, GL_FALSE, sizeof(float)*PTSHADOW_VERT_FLOATS, (void *)16 );
	GLuint boundTex = 0xFFFFFFFF;
	for ( int i=0; i<L.nBatches; i++ ) {
		const PtShadowBatch &b = L.pBatches[i];
		glUniformMatrix3fv( S.u_Rot, 1, GL_FALSE, b.Rot );
		glUniform4f( S.u_Proj, b.Proj[0], b.Proj[1], b.Proj[2], b.Proj[3] );
		if ( b.Tex!=boundTex ) {
			glBindTexture( GL_TEXTURE_2D, b.Tex );
			glUniform1i( S.u_AlphaTest, b.Tex ? 1 : 0 );
			boundTex = b.Tex;
		}
		glDrawArrays( GL_TRIANGLES, b.First, b.Count );
	}
}

// Cut-out texture of a mesh batch, read now (textures may load after the mesh):
// bound with alpha testing if it has alpha or a colour key.
static void BindMeshTexture( LPDIRECT3DTEXTURE2 *ppTex, GLint uAlphaTest )
{
	PtSurface *s = ( ppTex && *ppTex ) ? ((PtTexture *)*ppTex)->m_pSurface : NULL;
	if ( s && ( s->m_Format.mask[3] || s->m_bHasSrcKey ) ) {
		glBindTexture( GL_TEXTURE_2D, s->GetGLTexture() );
		glUniform1i( uAlphaTest, 1 );
	}
	else glUniform1i( uAlphaTest, 0 );
}

// The loaded map meshes, tiles near the shadow centre only.
static void DrawMeshes()
{
	PtShadowState &S = g_PtShadow;
	// tiles reaching into the shadowed square (wider: tall things cast from further)
	const float reach = PTSHADOW_HALF_SIZE*1.5f + 1000.0f;
	BOOL bProg = FALSE;
	for ( int m=0; m<PTSHADOW_MESH_SLOTS; m++ ) {
		PtShadowMesh &M = S.Mesh[m];
		if ( !M.VBO || !M.nBatches ) continue;
		float ox = (float)(M.Origin[0]-S.Centre[0]), oy = (float)(M.Origin[1]-S.Centre[1]), oz = (float)(M.Origin[2]-S.Centre[2]);
		float cx = -ox, cz = -oz;									// shadow centre in mesh coordinates
		if ( !bProg ) {
			glUseProgram( S.MeshProg );
			glUniform3f( S.um_R, S.LightR[0], S.LightR[1], S.LightR[2] );
			glUniform3f( S.um_U, S.LightU[0], S.LightU[1], S.LightU[2] );
			glUniform3f( S.um_D, S.LightD[0], S.LightD[1], S.LightD[2] );
			bProg = TRUE;
		}
		glUniform3f( S.um_Offset, ox, oy, oz );
		glBindBuffer( GL_ARRAY_BUFFER, M.VBO );
		glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 20, (void *)0 );
		glVertexAttribPointer( 1, 2, GL_FLOAT, GL_FALSE, 20, (void *)12 );
		LPDIRECT3DTEXTURE2 *last = (LPDIRECT3DTEXTURE2 *)1;
		for ( int i=0; i<M.nBatches; i++ ) {
			const PtShadowMeshBatch &b = M.pBatches[i];
			if ( b.MaxX<cx-reach || b.MinX>cx+reach || b.MaxZ<cz-reach || b.MinZ>cz+reach ) continue;
			if ( b.ppTex!=last ) { BindMeshTexture( b.ppTex, S.um_AlphaTest ); last = b.ppTex; }
			glDrawArrays( GL_TRIANGLES, b.First, b.Count );
		}
	}
}

BOOL PtGfx_ShadowHasMesh( int slot, const void *key, int keyN )
{
	PtShadowState &S = g_PtShadow;
	if ( !S.Prog || !S.bAllowed || slot<0 || slot>=PTSHADOW_MESH_SLOTS ) return TRUE;	// nothing to upload
	return S.Mesh[slot].Key==key && S.Mesh[slot].KeyN==keyN;
}

void PtGfx_ShadowSetMesh( int slot, const void *key, int keyN, const double origin[3],
	const float *verts, int nVerts, const PtShadowMeshBatch *batches, int nBatches )
{
	PtShadowState &S = g_PtShadow;
	if ( !S.Prog || slot<0 || slot>=PTSHADOW_MESH_SLOTS ) return;
	PtShadowMesh &M = S.Mesh[slot];
	M.Key = key;
	M.KeyN = keyN;
	free( M.pBatches );
	M.pBatches = NULL;
	M.nBatches = 0;
	if ( !verts || !nVerts || !nBatches ) return;
	if ( !M.VBO ) glGenBuffers( 1, &M.VBO );
	glBindBuffer( GL_ARRAY_BUFFER, M.VBO );
	glBufferData( GL_ARRAY_BUFFER, sizeof(float)*5*nVerts, verts, GL_STATIC_DRAW );
	M.pBatches = (PtShadowMeshBatch *)malloc( sizeof(PtShadowMeshBatch)*nBatches );
	if ( !M.pBatches ) return;
	memcpy( M.pBatches, batches, sizeof(PtShadowMeshBatch)*nBatches );
	M.nBatches = nBatches;
	for ( int i=0; i<3; i++ ) M.Origin[i] = origin[i];
	PtGL_Log( "shadows: map mesh %d: %d triangles in %d batches", slot, nVerts/3, nBatches );
	PtGfx_InvalidateDeviceState();
}

void PtGfx_ShadowSkipCapture( BOOL bSkip )
{
	g_PtShadow.bSkipCapture = bSkip;
}

// This frame's characters, the loaded map meshes and the frame before's map
// objects (windmills, flags...), drawn from the sun.
static void BuildMap()
{
	PtShadowState &S = g_PtShadow;
	const PtShadowList &prev = S.World[S.WorldCur^1];
	S.bMapReady = FALSE;
	BOOL bMesh = FALSE;
	for ( int m=0; m<PTSHADOW_MESH_SLOTS; m++ ) if ( S.Mesh[m].nBatches ) bMesh = TRUE;
	if ( !S.Chars.nVerts && !prev.nVerts && !bMesh ) return;

	glBindFramebuffer( GL_FRAMEBUFFER, S.FBO );
	glViewport( 0, 0, S.MapSize, S.MapSize );
	glDisable( GL_SCISSOR_TEST );
	glDisable( GL_BLEND );
	glDisable( GL_CULL_FACE );					// both sides: the camera's back faces cast too
	glEnable( GL_DEPTH_TEST );
	glDepthFunc( GL_LESS );
	glDepthMask( GL_TRUE );
	glColorMask( 0, 0, 0, 0 );
	glClearDepth( 1.0f );
	glClear( GL_DEPTH_BUFFER_BIT );
	glUseProgram( S.Prog );
	glUniform3f( S.u_R, S.LightR[0], S.LightR[1], S.LightR[2] );
	glUniform3f( S.u_U, S.LightU[0], S.LightU[1], S.LightU[2] );
	glUniform3f( S.u_D, S.LightD[0], S.LightD[1], S.LightD[2] );
	glActiveTexture( GL_TEXTURE0 );
	glBindVertexArray( S.VAO );
	glBindBuffer( GL_ARRAY_BUFFER, S.VBO );
	glEnableVertexAttribArray( 0 );
	glEnableVertexAttribArray( 1 );
	// characters only cast, at their true depth: pushed back like the map, legs seen
	// edge-on from the sun went units behind the ground and lost their shadow at the feet
	DrawList( S.Chars );
	// the map also receives its own shadow: push its depth back a little more where it
	// slopes away from the sun, so surfaces do not shadow themselves in stripes
	glEnable( GL_POLYGON_OFFSET_FILL );
	glPolygonOffset( 1.5f, 4.0f );
	DrawList( prev );
	DrawMeshes();

	glDisable( GL_POLYGON_OFFSET_FILL );
	glColorMask( 1, 1, 1, 0 );
	PtGfx_BindRenderTarget( g_PtGL.pRenderTarget );			// also restores the viewport
	PtGfx_InvalidateDeviceState();						// the device re-binds its textures
	S.bMapReady = TRUE;
}

void PtGfx_ShadowRole( int role )
{
	PtShadowState &S = g_PtShadow;
	if ( !S.bFrame ) { S.Role = PTSHADOW_OFF; return; }
	if ( S.Role==PTSHADOW_CASTER && role==PTSHADOW_RECEIVER ) BuildMap();
	S.Role = role;
	PtGfx_InvalidateDeviceState();
}

BOOL PtGfx_ShadowActive()
{
	return g_PtShadow.bFrame;
}

// Test view (?shadowtest): the intro has no characters-then-map order, so the
// whole scene is "the map": it casts a frame late and receives. The shadow centre
// is 400 units in front of the camera.
BOOL PtGfx_ShadowTestBegin( float camX, float camY, float camZ )
{
	PtShadowState &S = g_PtShadow;
	if ( !S.Prog || !S.bAllowed || !S.bTestAllowed ) return FALSE;
	const PtShadowList &last = S.World[S.WorldCur];
	float fwd[3] = { 0, 0, 1 };
	if ( last.nBatches ) memcpy( fwd, &last.pBatches[0].Rot[6], sizeof(fwd) );	// camera forward (third column)
	double cam[3] = { camX, camY, camZ }, centre[3];
	for ( int i=0; i<3; i++ ) centre[i] = cam[i] + fwd[i]*400.0;
	BeginFrame( cam, centre );
	S.Role = PTSHADOW_CASTER;			// no characters: builds from the frame before only
	PtGfx_ShadowRole( PTSHADOW_RECEIVER );

	static int s_Calls = 0;
	if ( ++s_Calls%300==0 )
		PtGL_Log( "shadow test %d: world verts %d (batches %d) | map %s | cameras %d (bad %d) draws %d skipped %d", s_Calls,
			S.World[S.WorldCur^1].nVerts, S.World[S.WorldCur^1].nBatches, S.bMapReady ? "ready" : "not built",
			S.DbgCam, S.DbgCamBad, S.DbgDraws, S.DbgSkipped );
	return TRUE;
}

void PtGfx_ShadowTestEnd()
{
	PtShadowState &S = g_PtShadow;
	if ( !S.bTestAllowed || S.Role!=PTSHADOW_RECEIVER ) return;
	S.Role = PTSHADOW_OFF;
	S.bFrame = FALSE;
	PtGfx_InvalidateDeviceState();
}
