// ptgfx_shadow.cpp - real-time shadows of characters on the map.
//
// The engine sends screen-space vertices (x, y in pixels, z, rhw) and does its
// 3D maths on the CPU, but each vertex's 3D position can be rebuilt from them:
// camera z = 1/rhw, x and y from the perspective divide (PtGfx_ShadowCamera gives
// the projection), then the inverse camera rotation and the eye position give the
// world position. That happens in the shaders (PTSHADOW_GLSL).
//  - casters: while the game draws the characters (DrawPat3D, PTSHADOW_CASTER),
//    their triangles are kept; before the map is drawn they are rendered from the
//    sun into a depth texture - orthographic, centred on the player
//  - receivers: while the map is drawn (DisplayStage, PTSHADOW_RECEIVER), the
//    device shader looks every pixel up in that texture and darkens it where a
//    character is between it and the sun (ptgfx_device.cpp)
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

// Sun: direction the light travels (y is up), half size of the shadowed square
// around the player and depth range of the shadow map, in world units.
static const float s_SunDir[3] = { 0.35f, -1.0f, 0.50f };
#define PTSHADOW_HALF_SIZE	700.0f
#define PTSHADOW_DEPTH		2000.0f

static const char *s_CasterVS =
	"in vec4 aPos;\n"
	PTSHADOW_GLSL
	"void main() { gl_Position = vec4( ptSunPos( aPos ), 1.0 ); }\n";

static const char *s_CasterFS =
	"out vec4 oColor;\n"
	"void main() { oColor = vec4( 1.0 ); }\n";

static void Normalize( float v[3] )
{
	float l = sqrtf( v[0]*v[0] + v[1]*v[1] + v[2]*v[2] );
	if ( l>0 ) { v[0] /= l; v[1] /= l; v[2] /= l; }
}

static void SetSun()
{
	PtShadowState &S = g_PtShadow;
	float d[3] = { s_SunDir[0], s_SunDir[1], s_SunDir[2] };
	Normalize( d );
	float r[3] = { -d[2], 0, d[0] };								// d x up
	Normalize( r );
	float u[3] = { r[1]*d[2]-r[2]*d[1], r[2]*d[0]-r[0]*d[2], r[0]*d[1]-r[1]*d[0] };	// r x d
	for ( int i=0; i<3; i++ ) {
		S.LightR[i] = r[i] / PTSHADOW_HALF_SIZE;
		S.LightU[i] = u[i] / PTSHADOW_HALF_SIZE;
		S.LightD[i] = d[i] / PTSHADOW_DEPTH;
	}
}

// Shadow strength, read every frame (the page can change Module.ptShadowDark live).
static void ReadDark()
{
#ifdef PT_WEB
	g_PtShadow.Dark = (float)EM_ASM_DOUBLE({ var d = Module['ptShadowDark']; return typeof d=='number' ? d : 0.6; });
#else
	g_PtShadow.Dark = 0.6f;
#endif
}

BOOL PtShadow_Init()
{
	PtShadowState &S = g_PtShadow;
	S.bAllowed = TRUE;
	ReadDark();
#ifdef PT_WEB
	S.bAllowed = EM_ASM_INT({ return Module['ptShadows']===false ? 0 : 1; });		// page setting
	S.bTestAllowed = EM_ASM_INT({ return /[?&]shadowtest=2/.test(location.search) ? 2 : /[?&]shadowtest/.test(location.search) ? 1 : 0; });
#endif
	const char *attribs[] = { "aPos" };
	S.Prog = PtGfx_CompileProgram( s_CasterVS, s_CasterFS, attribs, 1 );
	if ( !S.Prog ) { PtGL_Log( "shadows: caster shader failed, shadows off" ); return FALSE; }
	S.u_Rot  = glGetUniformLocation( S.Prog, "uSRot" );
	S.u_Proj = glGetUniformLocation( S.Prog, "uSProj" );
	S.u_Eye  = glGetUniformLocation( S.Prog, "uSEye" );
	S.u_R    = glGetUniformLocation( S.Prog, "uSLightR" );
	S.u_U    = glGetUniformLocation( S.Prog, "uSLightU" );
	S.u_D    = glGetUniformLocation( S.Prog, "uSLightD" );

	glGenVertexArrays( 1, &S.VAO );
	glGenBuffers( 1, &S.VBO );

	// depth texture with hardware comparison (sampler2DShadow, filtered = 2x2 PCF)
	glGenTextures( 1, &S.DepthTex );
	glActiveTexture( GL_TEXTURE0 + PTSHADOW_TEX_UNIT );
	glBindTexture( GL_TEXTURE_2D, S.DepthTex );
	glTexImage2D( GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, PTSHADOW_MAP_SIZE, PTSHADOW_MAP_SIZE, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, NULL );
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

	S.nMaxVerts = 400000;
	S.pVerts = (float *)malloc( sizeof(float)*4*S.nMaxVerts );
	S.nMaxBatches = 4096;
	S.pBatches = (PtShadowBatch *)malloc( sizeof(PtShadowBatch)*S.nMaxBatches );
	if ( !S.pVerts || !S.pBatches ) { S.Prog = 0; return FALSE; }
	SetSun();
	PtGL_Log( "shadows: %s (map %dx%d)", S.bAllowed ? "on" : "off (page setting)", PTSHADOW_MAP_SIZE, PTSHADOW_MAP_SIZE );
	return TRUE;
}

void PtGfx_ShadowFrame( BOOL bOn, float eyeX, float eyeY, float eyeZ )
{
	PtShadowState &S = g_PtShadow;
	S.bFrame = bOn && S.bAllowed && S.Prog;
	S.Role = S.bFrame ? PTSHADOW_CASTER : PTSHADOW_OFF;
	if ( S.bFrame ) ReadDark();
	S.bMapReady = FALSE;
	S.bCamera = FALSE;
	S.nVerts = S.nBatches = 0;
	S.Eye[0] = eyeX; S.Eye[1] = eyeY; S.Eye[2] = eyeZ;
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

void PtShadow_Capture( const BYTE *v, DWORD count, int stride )
{
	PtShadowState &S = g_PtShadow;
	count -= count % 3;
	if ( !count || S.nVerts + (int)count > S.nMaxVerts ) return;
	PtShadowBatch *b = S.nBatches ? &S.pBatches[S.nBatches-1] : NULL;
	if ( !b || memcmp( b->Rot, S.Rot, sizeof(S.Rot) ) || memcmp( b->Proj, S.Proj, sizeof(S.Proj) ) ) {
		if ( S.nBatches>=S.nMaxBatches ) return;
		b = &S.pBatches[S.nBatches++];
		b->First = S.nVerts;
		b->Count = 0;
		memcpy( b->Rot, S.Rot, sizeof(S.Rot) );
		memcpy( b->Proj, S.Proj, sizeof(S.Proj) );
	}
	float *dst = S.pVerts + S.nVerts*4;
	for ( DWORD i=0; i<count; i++, dst += 4 )
		memcpy( dst, v + i*stride, 16 );				// x, y, z, rhw
	S.nVerts += count;
	b->Count += count;
}

// The casters of this frame, drawn from the sun into the shadow map.
static void BuildMap()
{
	PtShadowState &S = g_PtShadow;
	S.bMapReady = FALSE;
	if ( !S.nVerts ) return;

	glBindFramebuffer( GL_FRAMEBUFFER, S.FBO );
	glViewport( 0, 0, PTSHADOW_MAP_SIZE, PTSHADOW_MAP_SIZE );
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
	glUniform3f( S.u_Eye, S.Eye[0], S.Eye[1], S.Eye[2] );
	glUniform3f( S.u_R, S.LightR[0], S.LightR[1], S.LightR[2] );
	glUniform3f( S.u_U, S.LightU[0], S.LightU[1], S.LightU[2] );
	glUniform3f( S.u_D, S.LightD[0], S.LightD[1], S.LightD[2] );
	glBindVertexArray( S.VAO );
	glBindBuffer( GL_ARRAY_BUFFER, S.VBO );
	glBufferData( GL_ARRAY_BUFFER, sizeof(float)*4*S.nVerts, S.pVerts, GL_STREAM_DRAW );
	glEnableVertexAttribArray( 0 );
	glVertexAttribPointer( 0, 4, GL_FLOAT, GL_FALSE, 16, (void *)0 );
	for ( int i=0; i<S.nBatches; i++ ) {
		PtShadowBatch &b = S.pBatches[i];
		glUniformMatrix3fv( S.u_Rot, 1, GL_FALSE, b.Rot );
		glUniform4f( S.u_Proj, b.Proj[0], b.Proj[1], b.Proj[2], b.Proj[3] );
		glDrawArrays( GL_TRIANGLES, b.First, b.Count );
	}
	glColorMask( 1, 1, 1, 0 );
	PtGfx_BindRenderTarget( g_PtGL.pRenderTarget );			// also restores the viewport
	PtGfx_InvalidateDeviceState();
	S.bMapReady = TRUE;
}

void PtGfx_ShadowRole( int role )
{
	PtShadowState &S = g_PtShadow;
	if ( !S.bFrame ) { S.Role = PTSHADOW_OFF; return; }
	if ( S.Role==PTSHADOW_CASTER && role!=PTSHADOW_CASTER ) BuildMap();
	S.Role = role;
	PtGfx_InvalidateDeviceState();
}

BOOL PtGfx_ShadowActive()
{
	return g_PtShadow.bFrame;
}

// Test view: the intro scene is drawn in one go (no characters-then-map order), so
// it casts and receives at once: the map is made from the previous frame's
// triangles, centred 400 units in front of that frame's camera.
void PtGfx_ShadowTestBegin( float camX, float camY, float camZ )
{
	PtShadowState &S = g_PtShadow;
	if ( !S.Prog || !S.bAllowed || !S.bTestAllowed ) return;
	S.bMapReady = FALSE;
	static int s_Calls = 0;
	s_Calls++;
	if ( S.nVerts && S.nBatches ) {
		const float *f = &S.pBatches[0].Rot[6];				// camera forward (third column)
		for ( int i=0; i<3; i++ ) {
			S.MapCentre[i] = S.TestCam[i] + f[i]*400.0f;
			S.Eye[i] = S.TestCam[i] - S.MapCentre[i];
		}
		BuildMap();
	}
	if ( s_Calls%300==0 ) {
		// what the shader computes for the middle kept vertex (PTSHADOW_GLSL), on the CPU
		char sz[512] = "no casters";
		if ( S.nVerts && S.nBatches ) {
			PtShadowBatch &b = S.pBatches[S.nBatches/2];
			const float *p = S.pVerts + (b.First + b.Count/2)*4, *r = b.Rot;
			float cz = p[3]>0 ? 1/p[3] : 1;
			float c[3] = { (p[0]/b.Proj[2]-1)*cz/b.Proj[0], (1-p[1]/b.Proj[3])*cz/b.Proj[1], cz };
			float w[3], l[3];
			for ( int i=0; i<3; i++ ) w[i] = r[i]*c[0] + r[3+i]*c[1] + r[6+i]*c[2] + S.Eye[i];
			l[0] = w[0]*S.LightR[0]+w[1]*S.LightR[1]+w[2]*S.LightR[2];
			l[1] = w[0]*S.LightU[0]+w[1]*S.LightU[1]+w[2]*S.LightU[2];
			l[2] = w[0]*S.LightD[0]+w[1]*S.LightD[1]+w[2]*S.LightD[2];
			sprintf( sz, "verts %d batches %d | proj %.3f %.3f %.0f %.0f | rot %.3f %.3f %.3f / %.3f %.3f %.3f / %.3f %.3f %.3f | "
				"cam %.0f %.0f %.0f | v %.1f %.1f %.4f %.6f -> camspace %.1f %.1f %.1f -> world-rel %.1f %.1f %.1f -> sun %.3f %.3f %.3f",
				S.nVerts, S.nBatches, b.Proj[0], b.Proj[1], b.Proj[2], b.Proj[3], r[0], r[1], r[2], r[3], r[4], r[5], r[6], r[7], r[8],
				S.TestCam[0], S.TestCam[1], S.TestCam[2], p[0], p[1], p[2], p[3], c[0], c[1], c[2], w[0], w[1], w[2], l[0], l[1], l[2] );
		}
		PtGL_Log( "shadow test %d: %s | map %s | cameras %d (bad %d) draws %d skipped %d", s_Calls, sz, S.bMapReady ? "ready" : "not built",
			S.DbgCam, S.DbgCamBad, S.DbgDraws, S.DbgSkipped );
	}
	S.bFrame = TRUE;
	S.bCamera = FALSE;
	S.nVerts = S.nBatches = 0;
	ReadDark();
	S.TestCam[0] = camX; S.TestCam[1] = camY; S.TestCam[2] = camZ;
	for ( int i=0; i<3; i++ ) S.Eye[i] = S.TestCam[i] - S.MapCentre[i];		// receivers this frame
	S.Role = PTSHADOW_TEST;
	PtGfx_InvalidateDeviceState();
}

void PtGfx_ShadowTestEnd()
{
	PtShadowState &S = g_PtShadow;
	if ( S.Role!=PTSHADOW_TEST ) return;
	S.Role = PTSHADOW_OFF;
	S.bFrame = FALSE;
	PtGfx_InvalidateDeviceState();
}
