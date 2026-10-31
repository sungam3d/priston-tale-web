// ptgfx_device.cpp - Direct3D 6 emulation: IDirect3D3, viewport, and the
// IDirect3DDevice3 that draws the game's pre-transformed vertices with OpenGL.
// See ptgfx.h for the overall design.

#include <stdio.h>
#include "ptgfx_internal.h"

#define PT_MAX_STAGES	8
#define PT_MAX_RS		256
#define PT_MAX_TSS		32
#define PT_ALPHAOP_TEXORDIFFUSE	100		// shader: alpha from texture if it has alpha, else from the vertex

//////////////////////////////////////////////////////////////////////////////
// Shaders: screen-space vertices + D3D6 texture stage combiner
//////////////////////////////////////////////////////////////////////////////
static const char *s_VS =
	"in vec4 aPos;\n"						// x, y (pixels), z (0..1), rhw
	"in vec4 aDiffuse;\n"
	"in vec4 aSpecular;\n"
	"in vec2 aTex0; in vec2 aTex1; in vec2 aTex2; in vec2 aTex3;\n"
	"in vec2 aTex4; in vec2 aTex5; in vec2 aTex6; in vec2 aTex7;\n"
	"uniform vec2 uScreen;\n"
	"uniform int uSRecv;\n"					// shadow receiver (ptgfx_shadow.cpp)
	PTSHADOW_GLSL
	"out vec4 vDiffuse; out vec4 vSpecular; out float vEyeW; out vec3 vShadow;\n"
	"out vec2 vTex0; out vec2 vTex1; out vec2 vTex2; out vec2 vTex3;\n"
	"out vec2 vTex4; out vec2 vTex5; out vec2 vTex6; out vec2 vTex7;\n"
	"void main() {\n"
	"	vShadow = uSRecv!=0 ? ptSunPos( aPos )*0.5 + 0.5 : vec3( -1.0 );\n"
	"	float w = aPos.w > 0.0 ? 1.0/aPos.w : 1.0;\n"
	// D3D pixel centres are at integer coordinates, OpenGL's at .5
	"	float x = (aPos.x+0.5)/uScreen.x*2.0-1.0;\n"
	"	float y = (aPos.y+0.5)/uScreen.y*2.0-1.0;\n"		// render targets are top-down
	// D3D never clips pre-transformed vertices by depth; keep z inside OpenGL's range
	"	float z = clamp( aPos.z, 0.0, 1.0 ) * 0.9999;\n"
	"	gl_Position = vec4( x*w, y*w, (z*2.0-1.0)*w, w );\n"
	"	vDiffuse = aDiffuse.bgra; vSpecular = aSpecular.bgra; vEyeW = w;\n"
	"	vTex0 = aTex0; vTex1 = aTex1; vTex2 = aTex2; vTex3 = aTex3;\n"
	"	vTex4 = aTex4; vTex5 = aTex5; vTex6 = aTex6; vTex7 = aTex7;\n"
	"}\n";

static const char *s_FS =
	"in vec4 vDiffuse; in vec4 vSpecular; in float vEyeW; in vec3 vShadow;\n"
	"uniform highp sampler2DShadow uShadowMap; uniform int uSRecv; uniform float uShadowDark; uniform float uShadowSoft; uniform int uShadowDbg;\n"
	"in vec2 vTex0; in vec2 vTex1; in vec2 vTex2; in vec2 vTex3;\n"
	"in vec2 vTex4; in vec2 vTex5; in vec2 vTex6; in vec2 vTex7;\n"
	"uniform sampler2D uTex0; uniform sampler2D uTex1; uniform sampler2D uTex2; uniform sampler2D uTex3;\n"
	"uniform sampler2D uTex4; uniform sampler2D uTex5; uniform sampler2D uTex6; uniform sampler2D uTex7;\n"
	"uniform int uCOp[8]; uniform int uCA1[8]; uniform int uCA2[8];\n"
	"uniform int uAOp[8]; uniform int uAA1[8]; uniform int uAA2[8];\n"
	"uniform int uTCI[8]; uniform int uHasTex[8]; uniform int uKeyed[8]; uniform int uTexAlpha[8];\n"
	"uniform vec4 uTFactor;\n"
	"uniform int uSpecular; uniform int uColorKey;\n"
	"uniform int uAlphaFunc; uniform float uAlphaRef;\n"
	"uniform int uFogMode; uniform vec4 uFogColor; uniform vec2 uFogRange;\n"
	"uniform int uWxMode; uniform vec3 uWxTint; uniform vec4 uWxFog;\n"		// weather (ptgfx_weather.cpp)
	"out vec4 oColor;\n"
	"vec2 coord( int i ) {\n"
	"	if ( i==0 ) return vTex0; if ( i==1 ) return vTex1; if ( i==2 ) return vTex2; if ( i==3 ) return vTex3;\n"
	"	if ( i==4 ) return vTex4; if ( i==5 ) return vTex5; if ( i==6 ) return vTex6; return vTex7;\n"
	"}\n"
	"vec4 tex( int s, vec2 c ) {\n"
	"	if ( s==0 ) return texture( uTex0, c ); if ( s==1 ) return texture( uTex1, c );\n"
	"	if ( s==2 ) return texture( uTex2, c ); if ( s==3 ) return texture( uTex3, c );\n"
	"	if ( s==4 ) return texture( uTex4, c ); if ( s==5 ) return texture( uTex5, c );\n"
	"	if ( s==6 ) return texture( uTex6, c ); return texture( uTex7, c );\n"
	"}\n"
	// D3DTA_*: DIFFUSE 0, CURRENT 1, TEXTURE 2, TFACTOR 3, SPECULAR 4; COMPLEMENT 0x10, ALPHAREPLICATE 0x20
	"vec4 arg( int a, vec4 cur, vec4 t ) {\n"
	"	int sel = a - (a/8)*8;\n"
	"	vec4 v = sel==0 ? vDiffuse : sel==1 ? cur : sel==2 ? t : sel==3 ? uTFactor : vSpecular;\n"
	"	if ( (a/32)%2==1 ) v = vec4( v.a );\n"
	"	if ( (a/16)%2==1 ) v = vec4( 1.0 ) - v;\n"
	"	return v;\n"
	"}\n"
	// D3DTOP_*
	// depth allowance for the map's own surface (the shadow map's 0..1 spans 8000 world
	// units). Small: the map's polygon offset already keeps surfaces off themselves, and
	// ground nearer a caster than this stays lit (0.0008 = 6 units left a gap at the feet)
	"const float shadowBias = 0.0001;\n"
	"vec4 op( int o, vec4 a1, vec4 a2, vec4 cur, vec4 t ) {\n"
	"	if ( o==2 ) return a1;\n"
	"	if ( o==3 ) return a2;\n"
	"	if ( o==4 ) return a1*a2;\n"
	"	if ( o==5 ) return clamp( a1*a2*2.0, 0.0, 1.0 );\n"
	"	if ( o==6 ) return clamp( a1*a2*4.0, 0.0, 1.0 );\n"
	"	if ( o==7 ) return clamp( a1+a2, 0.0, 1.0 );\n"
	"	if ( o==8 ) return clamp( a1+a2-0.5, 0.0, 1.0 );\n"
	"	if ( o==9 ) return clamp( (a1+a2-0.5)*2.0, 0.0, 1.0 );\n"
	"	if ( o==10 ) return clamp( a1-a2, 0.0, 1.0 );\n"
	"	if ( o==11 ) return clamp( a1+a2-a1*a2, 0.0, 1.0 );\n"
	"	if ( o==12 ) return mix( a2, a1, vDiffuse.a );\n"
	"	if ( o==13 ) return mix( a2, a1, t.a );\n"
	"	if ( o==14 ) return mix( a2, a1, uTFactor.a );\n"
	"	if ( o==15 ) return clamp( a1 + a2*(1.0-t.a), 0.0, 1.0 );\n"
	"	if ( o==16 ) return mix( a2, a1, cur.a );\n"
	"	return a1*a2;\n"
	"}\n"
	"bool alphaPass( float a ) {\n"			// D3DCMP_*
	"	float v = floor( a*255.0+0.5 );\n"
	"	if ( uAlphaFunc==1 ) return false;\n"
	"	if ( uAlphaFunc==2 ) return v< uAlphaRef;\n"
	"	if ( uAlphaFunc==3 ) return v==uAlphaRef;\n"
	"	if ( uAlphaFunc==4 ) return v<=uAlphaRef;\n"
	"	if ( uAlphaFunc==5 ) return v> uAlphaRef;\n"
	"	if ( uAlphaFunc==6 ) return v!=uAlphaRef;\n"
	"	if ( uAlphaFunc==7 ) return v>=uAlphaRef;\n"
	"	return true;\n"
	"}\n"
	// light reaching this pixel: 1 lit .. 1-uShadowDark in a character's shadow;
	// 3x3 filtered taps (each a 2x2 hardware comparison) for soft edges (spread
	// wider for the moon: uShadowSoft), fading out towards the edge of the shadowed square
	"float sunLight() {\n"
	"	vec3 s = vShadow;\n"
	"	if ( uShadowDark<=0.002 || s.x<=0.0 || s.x>=1.0 || s.y<=0.0 || s.y>=1.0 || s.z>=1.0 ) return 1.0;\n"
	"	vec2 t = 1.5*uShadowSoft/vec2( textureSize( uShadowMap, 0 ) );\n"
	"	float lit = 0.0;\n"
	"	for ( int i=-1; i<=1; i++ ) for ( int j=-1; j<=1; j++ )\n"
	"		lit += texture( uShadowMap, vec3( s.xy + vec2( float(i), float(j) )*t, s.z - shadowBias ) );\n"
	"	vec2 e = min( s.xy, 1.0 - s.xy );\n"
	"	float fade = clamp( min( e.x, e.y )*12.0, 0.0, 1.0 );\n"
	"	return 1.0 - ( 1.0 - lit/9.0 )*uShadowDark*fade;\n"
	"}\n"
	"void main() {\n"
	"	vec4 cur = vDiffuse;\n"
	"	for ( int i=0; i<8; i++ ) {\n"
	"		if ( uCOp[i]<=1 ) break;\n"				// D3DTOP_DISABLE ends the cascade
	"		vec4 t = vec4( 1.0 );\n"
	"		if ( uHasTex[i]!=0 ) {\n"
	"			t = tex( i, coord( uTCI[i] ) );\n"
	"			if ( uColorKey!=0 && uKeyed[i]!=0 && t.a<0.5 ) discard;\n"
	"		}\n"
	"		vec3 rgb = op( uCOp[i], arg( uCA1[i], cur, t ), arg( uCA2[i], cur, t ), cur, t ).rgb;\n"
	"		float al = uAOp[i]<=1 ? cur.a :\n"
	// 100: DirectX 5 TEXTUREMAPBLEND MODULATE - texture alpha times vertex alpha (vertex alpha
	//      alone for a texture without alpha). The login fade draws an opaque 4444 box with
	//      vertex alpha = fade level, so the vertex alpha must count.
	"			uAOp[i]==100 ? ( uTexAlpha[i]!=0 ? t.a*vDiffuse.a : vDiffuse.a ) :\n"
	"			op( uAOp[i], arg( uAA1[i], cur, t ), arg( uAA2[i], cur, t ), cur, t ).a;\n"
	"		cur = vec4( rgb, al );\n"
	"	}\n"
	"	if ( uSpecular!=0 ) cur.rgb = clamp( cur.rgb + vSpecular.rgb, 0.0, 1.0 );\n"
	"	if ( !alphaPass( cur.a ) ) discard;\n"
	"	if ( uSRecv!=0 ) {\n"
	"		if ( uShadowDbg!=0 ) {\n"			// test view 2: blue = inside the shadow map, red = in shadow
	"			vec3 s = vShadow;\n"
	"			bool inside = s.x>0.0 && s.x<1.0 && s.y>0.0 && s.y<1.0 && s.z<1.0;\n"
	"			float lit = inside ? texture( uShadowMap, vec3( s.xy, s.z - shadowBias ) ) : 1.0;\n"
	"			cur.rgb = mix( cur.rgb, inside ? vec3( 1.0-lit, 0.0, lit ) : vec3( 0.0, 1.0, 0.0 ), 0.5 );\n"
	"		}\n"
	"		else cur.rgb *= sunLight();\n"
	"	}\n"
	"	if ( uFogMode==1 ) cur.rgb = mix( uFogColor.rgb, cur.rgb, vSpecular.a );\n"						// vertex fog
	"	else if ( uFogMode==2 ) {\n"																		// table fog (eye distance)
	"		float f = clamp( (uFogRange.y - vEyeW) / max( uFogRange.y - uFogRange.x, 0.0001 ), 0.0, 1.0 );\n"
	"		cur.rgb = mix( uFogColor.rgb, cur.rgb, f );\n"
	"	}\n"
	// weather: the world under the sky's colour, fogged with distance; glows (added
	// colour) only fade into the fog
	"	if ( uWxMode!=0 ) {\n"
	"		float f = 1.0 - exp( -vEyeW*uWxFog.a );\n"
	"		if ( uWxMode==2 ) cur.rgb *= 1.0 - f;\n"
	"		else cur.rgb = mix( cur.rgb*uWxTint, uWxFog.rgb, f );\n"
	"	}\n"
	"	oColor = cur;\n"
	"}\n";

//////////////////////////////////////////////////////////////////////////////
// IDirect3DViewport3
//////////////////////////////////////////////////////////////////////////////
class PtViewport : public PtStub_IDirect3DViewport3 {
public:
	D3DVIEWPORT2	m_VP;
	PtViewport() { ZeroMemory( &m_VP, sizeof(m_VP) ); }
	STDMETHODIMP_(HRESULT) SetViewport2( LPD3DVIEWPORT2 lpVP ) override { if ( lpVP ) m_VP = *lpVP; return D3D_OK; }
	STDMETHODIMP_(HRESULT) GetViewport2( LPD3DVIEWPORT2 lpVP ) override { if ( lpVP ) *lpVP = m_VP; return D3D_OK; }

	// Clears the current render target (colour and/or depth) in each rectangle.
	STDMETHODIMP_(HRESULT) Clear2( DWORD count, LPD3DRECT lpRects, DWORD flags, D3DCOLOR color, D3DVALUE z, DWORD ) override
	{
		PtSurface *rt = g_PtGL.pRenderTarget;
		if ( !rt ) return D3D_OK;
		GLbitfield mask = 0;
		if ( flags & D3DCLEAR_TARGET ) {
			mask |= GL_COLOR_BUFFER_BIT;
			glColorMask( 1, 1, 1, 1 );
			glClearColor( ((color>>16)&0xFF)/255.0f, ((color>>8)&0xFF)/255.0f, (color&0xFF)/255.0f, 1.0f );
		}
		if ( flags & D3DCLEAR_ZBUFFER ) {
			mask |= GL_DEPTH_BUFFER_BIT;
			glDepthMask( GL_TRUE );
			glClearDepth( z );
		}
		if ( !mask ) return D3D_OK;
		glEnable( GL_SCISSOR_TEST );
		for ( DWORD i=0; i<( lpRects ? count : 1 ); i++ ) {
			RECT r = { 0, 0, rt->Width(), rt->Height() };
			if ( lpRects ) { r.left = lpRects[i].x1; r.top = lpRects[i].y1; r.right = lpRects[i].x2; r.bottom = lpRects[i].y2; }
			glScissor( r.left, r.top, r.right-r.left, r.bottom-r.top );		// render targets are top-down
			glClear( mask );
		}
		glDisable( GL_SCISSOR_TEST );
		if ( mask & GL_COLOR_BUFFER_BIT ) { rt->m_bGPUNewer = TRUE; g_PtStats.Fills++; }
		if ( g_PtTrace ) { fprintf( g_PtTrace, "CLEAR2 flags %X color %08X z %.3f rects %d\n", flags, color, z, count ); PtGfx_TracePixels( "clear" ); }
		PtGfx_InvalidateDeviceState();
		return D3D_OK;
	}
	STDMETHODIMP_(HRESULT) Clear( DWORD count, LPD3DRECT lpRects, DWORD flags ) override
	{
		return Clear2( count, lpRects, flags & (D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER), 0, 1.0f, 0 );
	}
};

//////////////////////////////////////////////////////////////////////////////
// Device description reported to the game (a hardware device)
//////////////////////////////////////////////////////////////////////////////
static void FillDeviceDesc( D3DDEVICEDESC *d )
{
	ZeroMemory( d, sizeof(D3DDEVICEDESC) );
	d->dwSize = sizeof(D3DDEVICEDESC);
	d->dwFlags = D3DDD_COLORMODEL | D3DDD_DEVCAPS | D3DDD_TRICAPS | D3DDD_DEVICERENDERBITDEPTH | D3DDD_DEVICEZBUFFERBITDEPTH;
	d->dcmColorModel = D3DCOLOR_RGB;
	d->dwDevCaps = D3DDEVCAPS_FLOATTLVERTEX | D3DDEVCAPS_DRAWPRIMTLVERTEX |
		D3DDEVCAPS_TEXTUREVIDEOMEMORY | D3DDEVCAPS_TLVERTEXSYSTEMMEMORY;
	D3DPRIMCAPS &t = d->dpcTriCaps;
	t.dwSize = sizeof(D3DPRIMCAPS);
	t.dwMiscCaps = D3DPMISCCAPS_CULLNONE | D3DPMISCCAPS_CULLCW | D3DPMISCCAPS_CULLCCW | D3DPMISCCAPS_MASKZ;
	t.dwRasterCaps = D3DPRASTERCAPS_DITHER | D3DPRASTERCAPS_ZTEST | D3DPRASTERCAPS_FOGVERTEX | D3DPRASTERCAPS_FOGTABLE | D3DPRASTERCAPS_WFOG;
	t.dwZCmpCaps = t.dwAlphaCmpCaps = 0xFF;
	t.dwSrcBlendCaps = t.dwDestBlendCaps = 0x1FFF;
	t.dwShadeCaps = D3DPSHADECAPS_COLORGOURAUDRGB | D3DPSHADECAPS_SPECULARGOURAUDRGB | D3DPSHADECAPS_ALPHAGOURAUDBLEND | D3DPSHADECAPS_FOGGOURAUD;
	t.dwTextureCaps = D3DPTEXTURECAPS_PERSPECTIVE | D3DPTEXTURECAPS_ALPHA | D3DPTEXTURECAPS_TRANSPARENCY;
	t.dwTextureFilterCaps = D3DPTFILTERCAPS_NEAREST | D3DPTFILTERCAPS_LINEAR | D3DPTFILTERCAPS_MIPNEAREST |
		D3DPTFILTERCAPS_MIPLINEAR | D3DPTFILTERCAPS_LINEARMIPNEAREST | D3DPTFILTERCAPS_LINEARMIPLINEAR |
		D3DPTFILTERCAPS_MINFPOINT | D3DPTFILTERCAPS_MINFLINEAR | D3DPTFILTERCAPS_MAGFPOINT | D3DPTFILTERCAPS_MAGFLINEAR |
		D3DPTFILTERCAPS_MIPFPOINT | D3DPTFILTERCAPS_MIPFLINEAR;
	t.dwTextureBlendCaps = 0xFF;
	t.dwTextureAddressCaps = D3DPTADDRESSCAPS_WRAP | D3DPTADDRESSCAPS_MIRROR | D3DPTADDRESSCAPS_CLAMP | D3DPTADDRESSCAPS_INDEPENDENTUV;
	d->dpcLineCaps = t;
	d->dwDeviceRenderBitDepth = DDBD_16 | DDBD_32;
	d->dwDeviceZBufferBitDepth = DDBD_16 | DDBD_24 | DDBD_32;
	d->dwMinTextureWidth = d->dwMinTextureHeight = 1;
	d->dwMaxTextureWidth = d->dwMaxTextureHeight = 4096;
	d->dwMaxTextureRepeat = 8192;
	d->dwMaxTextureAspectRatio = 4096;
	d->dwMaxAnisotropy = 1;
	d->dwTextureOpCaps = 0xFFFFFF;
	d->wMaxTextureBlendStages = PT_MAX_STAGES;
	d->wMaxSimultaneousTextures = PT_MAX_STAGES;
	d->dwFVFCaps = PT_MAX_STAGES;
	d->dwMaxBufferSize = 0;
	d->dwMaxVertexCount = 65535;
}

//////////////////////////////////////////////////////////////////////////////
// IDirect3DDevice3
//////////////////////////////////////////////////////////////////////////////
class PtDevice : public PtStub_IDirect3DDevice3 {
public:
	DWORD		m_RS[PT_MAX_RS];
	DWORD		m_TSS[PT_MAX_STAGES][PT_MAX_TSS];
	PtSurface	*m_pTex[PT_MAX_STAGES];
	PtViewport	*m_pViewport;
	BOOL		m_bStateDirty;

	GLuint		m_Prog;
	GLint		u_Screen, u_COp, u_CA1, u_CA2, u_AOp, u_AA1, u_AA2, u_TCI, u_HasTex, u_Keyed, u_TexAlpha;
	GLint		u_TFactor, u_Specular, u_ColorKey, u_AlphaFunc, u_AlphaRef, u_FogMode, u_FogColor, u_FogRange;
	GLint		u_SRecv, u_SRot, u_SProj, u_SEye, u_SLightR, u_SLightU, u_SLightD, u_ShadowDark, u_ShadowSoft;		// shadows
	GLint		u_WxMode, u_WxTint, u_WxFog;		// weather

	PtDevice()
	{
		ZeroMemory( m_RS, sizeof(m_RS) );
		ZeroMemory( m_TSS, sizeof(m_TSS) );
		ZeroMemory( m_pTex, sizeof(m_pTex) );
		m_pViewport = 0;
		m_bStateDirty = TRUE;
		m_Prog = 0;

		// Direct3D 6 defaults
		m_RS[D3DRENDERSTATE_ZENABLE] = TRUE;
		m_RS[D3DRENDERSTATE_ZWRITEENABLE] = TRUE;
		m_RS[D3DRENDERSTATE_ZFUNC] = D3DCMP_LESSEQUAL;
		m_RS[D3DRENDERSTATE_CULLMODE] = D3DCULL_CCW;
		m_RS[D3DRENDERSTATE_SRCBLEND] = D3DBLEND_ONE;
		m_RS[D3DRENDERSTATE_DESTBLEND] = D3DBLEND_ZERO;
		m_RS[D3DRENDERSTATE_ALPHAFUNC] = D3DCMP_ALWAYS;
		m_RS[D3DRENDERSTATE_SPECULARENABLE] = TRUE;
		m_RS[D3DRENDERSTATE_TEXTUREFACTOR] = 0xFFFFFFFF;
		m_RS[D3DRENDERSTATE_FOGTABLEMODE] = D3DFOG_NONE;
		for ( int s=0; s<PT_MAX_STAGES; s++ ) {
			m_TSS[s][D3DTSS_COLOROP]   = s==0 ? D3DTOP_MODULATE : D3DTOP_DISABLE;
			m_TSS[s][D3DTSS_COLORARG1] = D3DTA_TEXTURE;
			m_TSS[s][D3DTSS_COLORARG2] = D3DTA_CURRENT;
			m_TSS[s][D3DTSS_ALPHAOP]   = s==0 ? D3DTOP_SELECTARG1 : D3DTOP_DISABLE;
			m_TSS[s][D3DTSS_ALPHAARG1] = D3DTA_TEXTURE;
			m_TSS[s][D3DTSS_ALPHAARG2] = D3DTA_CURRENT;
			m_TSS[s][D3DTSS_TEXCOORDINDEX] = s;
			m_TSS[s][D3DTSS_ADDRESSU] = m_TSS[s][D3DTSS_ADDRESSV] = D3DTADDRESS_WRAP;
			m_TSS[s][D3DTSS_MAGFILTER] = D3DTFG_POINT;
			m_TSS[s][D3DTSS_MINFILTER] = D3DTFN_POINT;
			m_TSS[s][D3DTSS_MIPFILTER] = D3DTFP_NONE;
		}
	}

	virtual ~PtDevice()
	{
		if ( g_PtGL.pDevice==this ) g_PtGL.pDevice = 0;
		for ( int s=0; s<PT_MAX_STAGES; s++ ) if ( m_pTex[s] ) m_pTex[s]->Release();
		if ( m_pViewport ) m_pViewport->Release();
	}

	BOOL Init()
	{
		const char *attribs[] = { "aPos", "aDiffuse", "aSpecular", "aTex0", "aTex1", "aTex2", "aTex3", "aTex4", "aTex5", "aTex6", "aTex7" };
		m_Prog = PtGfx_CompileProgram( s_VS, s_FS, attribs, 11 );
		if ( !m_Prog ) return FALSE;
		glUseProgram( m_Prog );
		for ( int s=0; s<PT_MAX_STAGES; s++ ) {
			char name[16];
			sprintf( name, "uTex%d", s );
			glUniform1i( glGetUniformLocation( m_Prog, name ), s );
		}
		u_Screen = glGetUniformLocation( m_Prog, "uScreen" );
		u_COp = glGetUniformLocation( m_Prog, "uCOp" );
		u_CA1 = glGetUniformLocation( m_Prog, "uCA1" );
		u_CA2 = glGetUniformLocation( m_Prog, "uCA2" );
		u_AOp = glGetUniformLocation( m_Prog, "uAOp" );
		u_AA1 = glGetUniformLocation( m_Prog, "uAA1" );
		u_AA2 = glGetUniformLocation( m_Prog, "uAA2" );
		u_TCI = glGetUniformLocation( m_Prog, "uTCI" );
		u_HasTex = glGetUniformLocation( m_Prog, "uHasTex" );
		u_Keyed = glGetUniformLocation( m_Prog, "uKeyed" );
		u_TexAlpha = glGetUniformLocation( m_Prog, "uTexAlpha" );
		u_TFactor = glGetUniformLocation( m_Prog, "uTFactor" );
		u_Specular = glGetUniformLocation( m_Prog, "uSpecular" );
		u_ColorKey = glGetUniformLocation( m_Prog, "uColorKey" );
		u_AlphaFunc = glGetUniformLocation( m_Prog, "uAlphaFunc" );
		u_AlphaRef = glGetUniformLocation( m_Prog, "uAlphaRef" );
		u_FogMode = glGetUniformLocation( m_Prog, "uFogMode" );
		u_FogColor = glGetUniformLocation( m_Prog, "uFogColor" );
		u_FogRange = glGetUniformLocation( m_Prog, "uFogRange" );
		u_WxMode = glGetUniformLocation( m_Prog, "uWxMode" );
		u_WxTint = glGetUniformLocation( m_Prog, "uWxTint" );
		u_WxFog = glGetUniformLocation( m_Prog, "uWxFog" );

		// shadows (ptgfx_shadow.cpp): the shadow map stays bound to its own texture unit
		u_SRecv = glGetUniformLocation( m_Prog, "uSRecv" );
		u_SRot = glGetUniformLocation( m_Prog, "uSRot" );
		u_SProj = glGetUniformLocation( m_Prog, "uSProj" );
		u_SEye = glGetUniformLocation( m_Prog, "uSEye" );
		u_SLightR = glGetUniformLocation( m_Prog, "uSLightR" );
		u_SLightU = glGetUniformLocation( m_Prog, "uSLightU" );
		u_SLightD = glGetUniformLocation( m_Prog, "uSLightD" );
		glUniform1i( glGetUniformLocation( m_Prog, "uShadowMap" ), PTSHADOW_TEX_UNIT );
		u_ShadowDark = glGetUniformLocation( m_Prog, "uShadowDark" );
		glUniform1f( u_ShadowDark, 0.45f );
		u_ShadowSoft = glGetUniformLocation( m_Prog, "uShadowSoft" );
		glUniform1f( u_ShadowSoft, 1.0f );
		glUniform1i( u_SRecv, 0 );
		BOOL bShadowsOk = PtShadow_Init();
		glUseProgram( m_Prog );
		glUniform1i( glGetUniformLocation( m_Prog, "uShadowDbg" ), g_PtShadow.bTestAllowed==2 ? 1 : 0 );
		if ( !bShadowsOk ) {
			// no shadow map: the sampler still needs a depth texture with comparison on its unit
			GLuint t;
			glGenTextures( 1, &t );
			glActiveTexture( GL_TEXTURE0 + PTSHADOW_TEX_UNIT );
			glBindTexture( GL_TEXTURE_2D, t );
			glTexImage2D( GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, 1, 1, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, NULL );
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE );
			glActiveTexture( GL_TEXTURE0 );
		}
		return TRUE;
	}

	// shadow receiving for this draw: on while the map is drawn and the shadow map is ready
	void ApplyShadow( BOOL bScreenVerts )
	{
		PtShadowState &S = g_PtShadow;
		BOOL on = S.Role==PTSHADOW_RECEIVER && S.bMapReady && S.bCamera && bScreenVerts;
		glUniform1i( u_SRecv, on ? 1 : 0 );
		if ( !on ) return;
		glUniformMatrix3fv( u_SRot, 1, GL_FALSE, S.Rot );
		glUniform4f( u_SProj, S.Proj[0], S.Proj[1], S.Proj[2], S.Proj[3] );
		glUniform3f( u_SEye, S.Eye[0], S.Eye[1], S.Eye[2] );
		glUniform3f( u_SLightR, S.LightR[0], S.LightR[1], S.LightR[2] );
		glUniform3f( u_SLightU, S.LightU[0], S.LightU[1], S.LightU[2] );
		glUniform3f( u_SLightD, S.LightD[0], S.LightD[1], S.LightD[2] );
		glUniform1f( u_ShadowDark, S.Dark );
		glUniform1f( u_ShadowSoft, S.Soft );
	}

	static GLenum BlendFactor( DWORD b, BOOL bSrc )
	{
		switch ( b ) {
		case D3DBLEND_ZERO:				return GL_ZERO;
		case D3DBLEND_ONE:				return GL_ONE;
		case D3DBLEND_SRCCOLOR:			return GL_SRC_COLOR;
		case D3DBLEND_INVSRCCOLOR:		return GL_ONE_MINUS_SRC_COLOR;
		case D3DBLEND_SRCALPHA:			return GL_SRC_ALPHA;
		case D3DBLEND_INVSRCALPHA:		return GL_ONE_MINUS_SRC_ALPHA;
		case D3DBLEND_DESTALPHA:		return GL_ONE;			// back buffer has no alpha (16-bit era): treat as 1
		case D3DBLEND_INVDESTALPHA:		return GL_ZERO;
		case D3DBLEND_DESTCOLOR:		return GL_DST_COLOR;
		case D3DBLEND_INVDESTCOLOR:		return GL_ONE_MINUS_DST_COLOR;
		case D3DBLEND_SRCALPHASAT:		return GL_SRC_ALPHA_SATURATE;
		case D3DBLEND_BOTHSRCALPHA:		return bSrc ? GL_SRC_ALPHA : GL_ONE_MINUS_SRC_ALPHA;
		case D3DBLEND_BOTHINVSRCALPHA:	return bSrc ? GL_ONE_MINUS_SRC_ALPHA : GL_SRC_ALPHA;
		}
		return bSrc ? GL_ONE : GL_ZERO;
	}

	static GLenum WrapMode( DWORD a )
	{
		if ( a==D3DTADDRESS_CLAMP ) return GL_CLAMP_TO_EDGE;
		if ( a==D3DTADDRESS_MIRROR ) return GL_MIRRORED_REPEAT;
		return GL_REPEAT;
	}

	void ApplyState()
	{
		PtSurface *rt = g_PtGL.pRenderTarget;
		if ( !rt ) return;

		glUseProgram( m_Prog );
		glUniform2f( u_Screen, (float)rt->Width(), (float)rt->Height() );

		// depth
		if ( m_RS[D3DRENDERSTATE_ZENABLE] ) {
			glEnable( GL_DEPTH_TEST );
			DWORD f = m_RS[D3DRENDERSTATE_ZFUNC];
			glDepthFunc( (f>=1 && f<=8) ? GL_NEVER + (f-1) : GL_LEQUAL );
		}
		else glDisable( GL_DEPTH_TEST );
		glDepthMask( m_RS[D3DRENDERSTATE_ZWRITEENABLE] ? GL_TRUE : GL_FALSE );

		// culling: our render targets keep D3D's top-down screen orientation, so
		// a triangle D3D calls clockwise is counter-clockwise for OpenGL.
		switch ( m_RS[D3DRENDERSTATE_CULLMODE] ) {
		case D3DCULL_CW:	glEnable( GL_CULL_FACE ); glFrontFace( GL_CCW ); glCullFace( GL_FRONT ); break;
		case D3DCULL_CCW:	glEnable( GL_CULL_FACE ); glFrontFace( GL_CCW ); glCullFace( GL_BACK ); break;
		default:			glDisable( GL_CULL_FACE ); break;
		}

		// blending
		if ( m_RS[D3DRENDERSTATE_ALPHABLENDENABLE] ) {
			glEnable( GL_BLEND );
			glBlendFunc( BlendFactor( m_RS[D3DRENDERSTATE_SRCBLEND], TRUE ), BlendFactor( m_RS[D3DRENDERSTATE_DESTBLEND], FALSE ) );
		}
		else glDisable( GL_BLEND );
		glColorMask( 1, 1, 1, 0 );			// keep render target alpha at 1 (no destination alpha)

		// viewport clip
		if ( m_pViewport && m_pViewport->m_VP.dwWidth ) {
			glEnable( GL_SCISSOR_TEST );
			glScissor( m_pViewport->m_VP.dwX, m_pViewport->m_VP.dwY, m_pViewport->m_VP.dwWidth, m_pViewport->m_VP.dwHeight );
		}
		else glDisable( GL_SCISSOR_TEST );

		// texture stages
		GLint cop[8], ca1[8], ca2[8], aop[8], aa1[8], aa2[8], tci[8], hasTex[8], keyed[8], texAlpha[8];
		for ( int s=0; s<PT_MAX_STAGES; s++ ) {
			cop[s] = m_TSS[s][D3DTSS_COLOROP];   ca1[s] = m_TSS[s][D3DTSS_COLORARG1]; ca2[s] = m_TSS[s][D3DTSS_COLORARG2];
			aop[s] = m_TSS[s][D3DTSS_ALPHAOP];   aa1[s] = m_TSS[s][D3DTSS_ALPHAARG1]; aa2[s] = m_TSS[s][D3DTSS_ALPHAARG2];
			tci[s] = m_TSS[s][D3DTSS_TEXCOORDINDEX] & 0xFFFF;
			PtSurface *t = m_pTex[s];
			hasTex[s] = t!=0;
			keyed[s] = t && t->m_bHasSrcKey;
			texAlpha[s] = t && ( t->m_Format.mask[3] || t->m_bHasSrcKey );
			glActiveTexture( GL_TEXTURE0 + s );
			if ( !t ) { glBindTexture( GL_TEXTURE_2D, 0 ); continue; }
			glBindTexture( GL_TEXTURE_2D, t->GetGLTexture() );
			BOOL bLinear = m_TSS[s][D3DTSS_MAGFILTER]==D3DTFG_LINEAR;
			BOOL bMinLinear = m_TSS[s][D3DTSS_MINFILTER]==D3DTFN_LINEAR;
			BOOL bMips = t->m_pAttached && m_TSS[s][D3DTSS_MIPFILTER]!=D3DTFP_NONE;
			GLenum minf = bMips ? ( bMinLinear ? GL_LINEAR_MIPMAP_LINEAR : GL_NEAREST_MIPMAP_NEAREST ) : ( bMinLinear ? GL_LINEAR : GL_NEAREST );
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, bLinear ? GL_LINEAR : GL_NEAREST );
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minf );
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, WrapMode( m_TSS[s][D3DTSS_ADDRESSU] ) );
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, WrapMode( m_TSS[s][D3DTSS_ADDRESSV] ) );
		}
		glActiveTexture( GL_TEXTURE0 );
		glUniform1iv( u_COp, 8, cop ); glUniform1iv( u_CA1, 8, ca1 ); glUniform1iv( u_CA2, 8, ca2 );
		glUniform1iv( u_AOp, 8, aop ); glUniform1iv( u_AA1, 8, aa1 ); glUniform1iv( u_AA2, 8, aa2 );
		glUniform1iv( u_TCI, 8, tci ); glUniform1iv( u_HasTex, 8, hasTex ); glUniform1iv( u_Keyed, 8, keyed ); glUniform1iv( u_TexAlpha, 8, texAlpha );

		DWORD tf = m_RS[D3DRENDERSTATE_TEXTUREFACTOR];
		glUniform4f( u_TFactor, ((tf>>16)&0xFF)/255.0f, ((tf>>8)&0xFF)/255.0f, (tf&0xFF)/255.0f, (tf>>24)/255.0f );
		glUniform1i( u_Specular, m_RS[D3DRENDERSTATE_SPECULARENABLE] ? 1 : 0 );
		glUniform1i( u_ColorKey, m_RS[D3DRENDERSTATE_COLORKEYENABLE] ? 1 : 0 );
		glUniform1i( u_AlphaFunc, m_RS[D3DRENDERSTATE_ALPHATESTENABLE] ? (int)m_RS[D3DRENDERSTATE_ALPHAFUNC] : D3DCMP_ALWAYS );
		glUniform1f( u_AlphaRef, (float)(m_RS[D3DRENDERSTATE_ALPHAREF] & 0xFF) );

		int fogMode = 0;
		if ( m_RS[D3DRENDERSTATE_FOGENABLE] )
			fogMode = m_RS[D3DRENDERSTATE_FOGTABLEMODE]!=D3DFOG_NONE ? 2 : 1;
		glUniform1i( u_FogMode, fogMode );
		DWORD fc = m_RS[D3DRENDERSTATE_FOGCOLOR];
		glUniform4f( u_FogColor, ((fc>>16)&0xFF)/255.0f, ((fc>>8)&0xFF)/255.0f, (fc&0xFF)/255.0f, 1.0f );
		float fs = *(float *)&m_RS[D3DRENDERSTATE_FOGTABLESTART], fe = *(float *)&m_RS[D3DRENDERSTATE_FOGTABLEEND];
		glUniform2f( u_FogRange, fs, fe );

		const PtWeatherState &W = g_PtWeather;
		int wx = 0;
		if ( W.bWorld && W.bLook ) wx = ( m_RS[D3DRENDERSTATE_ALPHABLENDENABLE] && m_RS[D3DRENDERSTATE_DESTBLEND]==D3DBLEND_ONE ) ? 2 : 1;
		glUniform1i( u_WxMode, wx );
		if ( wx ) {
			glUniform3f( u_WxTint, W.Tint[0], W.Tint[1], W.Tint[2] );
			glUniform4f( u_WxFog, W.Fog[0], W.Fog[1], W.Fog[2], W.Density );
		}

		m_bStateDirty = FALSE;
	}

	// IUnknown
	STDMETHODIMP_(HRESULT) QueryInterface( REFIID riid, LPVOID FAR * ppvObj ) override
	{
		if ( !ppvObj ) return E_POINTER;
		if ( riid==IID_IDirect3DDevice3 || riid==IID_IUnknown ) { *ppvObj = this; AddRef(); return D3D_OK; }
		*ppvObj = 0;
		return E_NOINTERFACE;
	}

	STDMETHODIMP_(HRESULT) GetCaps( LPD3DDEVICEDESC lpHW, LPD3DDEVICEDESC lpHEL ) override
	{
		if ( lpHW ) FillDeviceDesc( lpHW );
		if ( lpHEL ) { FillDeviceDesc( lpHEL ); lpHEL->dcmColorModel = 0; }
		return D3D_OK;
	}

	STDMETHODIMP_(HRESULT) EnumTextureFormats( LPD3DENUMPIXELFORMATSCALLBACK lpCallback, LPVOID lpArg ) override
	{
		static const DWORD fmts[][5] = {
			{ 16, 0xF800, 0x07E0, 0x001F, 0 },					// R5G6B5
			{ 16, 0x7C00, 0x03E0, 0x001F, 0 },					// X1R5G5B5
			{ 16, 0x7C00, 0x03E0, 0x001F, 0x8000 },				// A1R5G5B5
			{ 16, 0x0F00, 0x00F0, 0x000F, 0xF000 },				// A4R4G4B4
			{ 32, 0xFF0000, 0xFF00, 0xFF, 0 },					// X8R8G8B8
			{ 32, 0xFF0000, 0xFF00, 0xFF, 0xFF000000 },			// A8R8G8B8
		};
		for ( int i=0; i<6; i++ ) {
			PtFormat f;
			f.SetRGB( fmts[i][0], fmts[i][1], fmts[i][2], fmts[i][3], fmts[i][4] );
			DDPIXELFORMAT pf;
			f.ToDDPF( &pf );
			if ( lpCallback( &pf, lpArg )==D3DENUMRET_CANCEL ) break;
		}
		return D3D_OK;
	}

	STDMETHODIMP_(HRESULT) BeginScene() override { return D3D_OK; }
	STDMETHODIMP_(HRESULT) EndScene() override { return D3D_OK; }

	STDMETHODIMP_(HRESULT) AddViewport( LPDIRECT3DVIEWPORT3 ) override { return D3D_OK; }
	STDMETHODIMP_(HRESULT) DeleteViewport( LPDIRECT3DVIEWPORT3 lpVP ) override
	{
		if ( lpVP==m_pViewport && m_pViewport ) { m_pViewport->Release(); m_pViewport = 0; }
		return D3D_OK;
	}
	STDMETHODIMP_(HRESULT) SetCurrentViewport( LPDIRECT3DVIEWPORT3 lpVP ) override
	{
		if ( lpVP ) lpVP->AddRef();
		if ( m_pViewport ) m_pViewport->Release();
		m_pViewport = (PtViewport *)lpVP;
		m_bStateDirty = TRUE;
		return D3D_OK;
	}

	STDMETHODIMP_(HRESULT) SetRenderTarget( LPDIRECTDRAWSURFACE4 lpSurf, DWORD ) override
	{
		PtSurface *s = (PtSurface *)lpSurf;
		if ( !s || !s->MakeRenderTarget() ) return DDERR_INVALIDPARAMS;
		g_PtGL.pRenderTarget = s;
		PtGfx_BindRenderTarget( s );
		g_PtStats.RTSwitches++;
		m_bStateDirty = TRUE;
		return D3D_OK;
	}
	STDMETHODIMP_(HRESULT) GetRenderTarget( LPDIRECTDRAWSURFACE4 *lplpSurf ) override
	{
		if ( !lplpSurf ) return DDERR_INVALIDPARAMS;
		*lplpSurf = g_PtGL.pRenderTarget;
		if ( g_PtGL.pRenderTarget ) g_PtGL.pRenderTarget->AddRef();
		return D3D_OK;
	}

	STDMETHODIMP_(HRESULT) GetRenderState( D3DRENDERSTATETYPE rs, LPDWORD lpValue ) override
	{
		if ( !lpValue || (DWORD)rs>=PT_MAX_RS ) return DDERR_INVALIDPARAMS;
		*lpValue = m_RS[rs];
		return D3D_OK;
	}

	STDMETHODIMP_(HRESULT) SetRenderState( D3DRENDERSTATETYPE rs, DWORD value ) override
	{
		if ( (DWORD)rs>=PT_MAX_RS ) return D3D_OK;
		m_RS[rs] = value;
		m_bStateDirty = TRUE;

		// DirectX 5 style states mapped onto texture stage 0
		switch ( rs ) {
		case D3DRENDERSTATE_TEXTUREHANDLE:
			SetTexture( 0, value ? (LPDIRECT3DTEXTURE2)(size_t)value : NULL );
			break;
		case D3DRENDERSTATE_TEXTUREMAG:
			m_TSS[0][D3DTSS_MAGFILTER] = (value==D3DFILTER_NEAREST) ? D3DTFG_POINT : D3DTFG_LINEAR;
			break;
		case D3DRENDERSTATE_TEXTUREMIN:
			m_TSS[0][D3DTSS_MINFILTER] = (value==D3DFILTER_NEAREST || value==D3DFILTER_MIPNEAREST) ? D3DTFN_POINT : D3DTFN_LINEAR;
			break;
		case D3DRENDERSTATE_TEXTUREADDRESS:
			m_TSS[0][D3DTSS_ADDRESSU] = m_TSS[0][D3DTSS_ADDRESSV] = value;
			break;
		case D3DRENDERSTATE_TEXTUREMAPBLEND:
			m_TSS[0][D3DTSS_COLORARG1] = m_TSS[0][D3DTSS_ALPHAARG1] = D3DTA_TEXTURE;
			m_TSS[0][D3DTSS_COLORARG2] = m_TSS[0][D3DTSS_ALPHAARG2] = D3DTA_DIFFUSE;
			switch ( value ) {
			case D3DTBLEND_DECAL: case D3DTBLEND_COPY:
				m_TSS[0][D3DTSS_COLOROP] = D3DTOP_SELECTARG1; m_TSS[0][D3DTSS_ALPHAOP] = D3DTOP_SELECTARG1; break;
			case D3DTBLEND_MODULATE:
				m_TSS[0][D3DTSS_COLOROP] = D3DTOP_MODULATE;   m_TSS[0][D3DTSS_ALPHAOP] = PT_ALPHAOP_TEXORDIFFUSE; break;
			case D3DTBLEND_MODULATEALPHA:
				m_TSS[0][D3DTSS_COLOROP] = D3DTOP_MODULATE;   m_TSS[0][D3DTSS_ALPHAOP] = D3DTOP_MODULATE; break;
			case D3DTBLEND_DECALALPHA:
				m_TSS[0][D3DTSS_COLOROP] = D3DTOP_BLENDTEXTUREALPHA; m_TSS[0][D3DTSS_ALPHAOP] = D3DTOP_SELECTARG2; break;
			case D3DTBLEND_ADD:
				m_TSS[0][D3DTSS_COLOROP] = D3DTOP_ADD;        m_TSS[0][D3DTSS_ALPHAOP] = D3DTOP_SELECTARG2; break;
			}
			break;
		}
		return D3D_OK;
	}

	STDMETHODIMP_(HRESULT) GetTextureStageState( DWORD stage, D3DTEXTURESTAGESTATETYPE type, LPDWORD lpValue ) override
	{
		if ( !lpValue || stage>=PT_MAX_STAGES || (DWORD)type>=PT_MAX_TSS ) return DDERR_INVALIDPARAMS;
		*lpValue = m_TSS[stage][type];
		return D3D_OK;
	}

	STDMETHODIMP_(HRESULT) SetTextureStageState( DWORD stage, D3DTEXTURESTAGESTATETYPE type, DWORD value ) override
	{
		if ( stage>=PT_MAX_STAGES || (DWORD)type>=PT_MAX_TSS ) return D3D_OK;
		m_TSS[stage][type] = value;
		if ( type==D3DTSS_ADDRESS ) m_TSS[stage][D3DTSS_ADDRESSU] = m_TSS[stage][D3DTSS_ADDRESSV] = value;
		m_bStateDirty = TRUE;
		return D3D_OK;
	}

	STDMETHODIMP_(HRESULT) GetTexture( DWORD stage, LPDIRECT3DTEXTURE2 *lplpTex ) override
	{
		if ( !lplpTex || stage>=PT_MAX_STAGES ) return DDERR_INVALIDPARAMS;
		*lplpTex = m_pTex[stage] ? &m_pTex[stage]->m_Texture : NULL;
		if ( *lplpTex ) (*lplpTex)->AddRef();
		return D3D_OK;
	}

	STDMETHODIMP_(HRESULT) SetTexture( DWORD stage, LPDIRECT3DTEXTURE2 lpTex ) override
	{
		if ( stage>=PT_MAX_STAGES ) return D3D_OK;
		PtSurface *s = lpTex ? ((PtTexture *)lpTex)->m_pSurface : NULL;
		if ( s ) s->AddRef();
		if ( m_pTex[stage] ) m_pTex[stage]->Release();
		m_pTex[stage] = s;
		m_bStateDirty = TRUE;
		return D3D_OK;
	}

	STDMETHODIMP_(HRESULT) SetTransform( D3DTRANSFORMSTATETYPE, LPD3DMATRIX ) override { return D3D_OK; }

	void TraceDraw( GLenum mode, DWORD fvf, BYTE *v, DWORD count, int stride, int diffuseOfs, int nTex )
	{
		float minX = 1e9f, minY = 1e9f, maxX = -1e9f, maxY = -1e9f, minZ = 1e9f, maxZ = -1e9f, minW = 1e9f, maxW = -1e9f;
		for ( DWORD i=0; i<count; i++ ) {
			float *p = (float *)(v + i*stride);
			if ( p[0]<minX ) minX = p[0]; if ( p[0]>maxX ) maxX = p[0];
			if ( p[1]<minY ) minY = p[1]; if ( p[1]>maxY ) maxY = p[1];
			if ( p[2]<minZ ) minZ = p[2]; if ( p[2]>maxZ ) maxZ = p[2];
			if ( p[3]<minW ) minW = p[3]; if ( p[3]>maxW ) maxW = p[3];
		}
		DWORD c0 = diffuseOfs>=0 ? *(DWORD *)(v + diffuseOfs) : 0xFFFFFFFF;
		PtSurface *t = m_pTex[0];
		fprintf( g_PtTrace, "DRAW mode %d n %d fvf %X tex %d | xy %.0f,%.0f-%.0f,%.0f z %.3f..%.3f rhw %.4f..%.4f c0 %08X | tex0 %s %dx%d bpp %d a %X key %d | "
			"cop %d/%X/%X aop %d/%X/%X st1 %d | zen %d zf %d zw %d blend %d %d/%d atest %d %d/%d ckey %d fog %d/%d cull %d\n",
			mode, count, fvf, nTex, minX, minY, maxX, maxY, minZ, maxZ, minW, maxW, c0,
			t ? "yes" : "no", t ? t->Width() : 0, t ? t->Height() : 0, t ? t->m_Format.bpp : 0, t ? t->m_Format.mask[3] : 0, t ? t->m_bHasSrcKey : 0,
			m_TSS[0][D3DTSS_COLOROP], m_TSS[0][D3DTSS_COLORARG1], m_TSS[0][D3DTSS_COLORARG2],
			m_TSS[0][D3DTSS_ALPHAOP], m_TSS[0][D3DTSS_ALPHAARG1], m_TSS[0][D3DTSS_ALPHAARG2], m_TSS[1][D3DTSS_COLOROP],
			m_RS[D3DRENDERSTATE_ZENABLE], m_RS[D3DRENDERSTATE_ZFUNC], m_RS[D3DRENDERSTATE_ZWRITEENABLE],
			m_RS[D3DRENDERSTATE_ALPHABLENDENABLE], m_RS[D3DRENDERSTATE_SRCBLEND], m_RS[D3DRENDERSTATE_DESTBLEND],
			m_RS[D3DRENDERSTATE_ALPHATESTENABLE], m_RS[D3DRENDERSTATE_ALPHAFUNC], m_RS[D3DRENDERSTATE_ALPHAREF],
			m_RS[D3DRENDERSTATE_COLORKEYENABLE], m_RS[D3DRENDERSTATE_FOGENABLE], m_RS[D3DRENDERSTATE_FOGTABLEMODE], m_RS[D3DRENDERSTATE_CULLMODE] );
		PtGfx_TracePixels( "draw" );
	}

	STDMETHODIMP_(HRESULT) DrawPrimitive( D3DPRIMITIVETYPE type, DWORD fvf, LPVOID lpVerts, DWORD count, DWORD ) override
	{
		if ( !lpVerts || !count || !g_PtGL.pRenderTarget ) return D3D_OK;

		GLenum mode;
		switch ( type ) {
		case D3DPT_POINTLIST:		mode = GL_POINTS; break;
		case D3DPT_LINELIST:		mode = GL_LINES; break;
		case D3DPT_LINESTRIP:		mode = GL_LINE_STRIP; break;
		case D3DPT_TRIANGLESTRIP:	mode = GL_TRIANGLE_STRIP; break;
		case D3DPT_TRIANGLEFAN:		mode = GL_TRIANGLE_FAN; break;
		default:					mode = GL_TRIANGLES; break;
		}

		// vertex layout from the flexible vertex format
		int posSize = 0, offset = 0;
		if ( (fvf & D3DFVF_POSITION_MASK)==D3DFVF_XYZRHW ) posSize = 4;
		else if ( (fvf & D3DFVF_POSITION_MASK)==D3DFVF_XYZ ) posSize = 3;
		else return D3D_OK;					// untransformed formats are not used by the game
		offset = posSize*4;
		if ( fvf & D3DFVF_NORMAL ) offset += 12;
		if ( fvf & D3DFVF_RESERVED1 ) offset += 4;
		int diffuseOfs = -1, specularOfs = -1;
		if ( fvf & D3DFVF_DIFFUSE ) { diffuseOfs = offset; offset += 4; }
		if ( fvf & D3DFVF_SPECULAR ) { specularOfs = offset; offset += 4; }
		int nTex = (fvf & D3DFVF_TEXCOUNT_MASK) >> D3DFVF_TEXCOUNT_SHIFT;
		if ( nTex>PT_MAX_STAGES ) nTex = PT_MAX_STAGES;
		int texOfs = offset;
		int stride = offset + nTex*8;

		if ( m_bStateDirty ) ApplyState();

		// shadows: keep caster triangles (characters while casting, the map while it
		// receives - for the next frame). Solid models write depth (the engine leaves
		// alpha blending on for them too); glows and other effects do not, or add
		// their colour (DESTBLEND ONE). Textures with alpha / colour key (leaves,
		// fences) are cut out in the shadow map too.
		if ( g_PtShadow.Role==PTSHADOW_CASTER || g_PtShadow.Role==PTSHADOW_RECEIVER ) {
			g_PtShadow.DbgDraws++;
			if ( g_PtShadow.bCamera && mode==GL_TRIANGLES && posSize==4 && m_RS[D3DRENDERSTATE_ZWRITEENABLE] &&
				!( m_RS[D3DRENDERSTATE_ALPHABLENDENABLE] && m_RS[D3DRENDERSTATE_DESTBLEND]==D3DBLEND_ONE ) ) {
				int tci = m_TSS[0][D3DTSS_TEXCOORDINDEX] & 0xFFFF;
				int uvOfs = tci<nTex ? texOfs + tci*8 : -1;
				PtSurface *t = m_pTex[0];
				GLuint cut = ( uvOfs>=0 && t && ( t->m_Format.mask[3] || t->m_bHasSrcKey ) ) ? t->GetGLTexture() : 0;
				PtShadow_Capture( (const BYTE *)lpVerts, count, stride, uvOfs, cut );
			}
			else g_PtShadow.DbgSkipped++;
		}
		ApplyShadow( posSize==4 );

		glBindVertexArray( g_PtGL.VAO );
		glBindBuffer( GL_ARRAY_BUFFER, g_PtGL.VBO );
		glBufferData( GL_ARRAY_BUFFER, stride*count, lpVerts, GL_STREAM_DRAW );

		glEnableVertexAttribArray( 0 );
		glVertexAttribPointer( 0, posSize, GL_FLOAT, GL_FALSE, stride, (void *)0 );
		if ( posSize==3 ) glVertexAttrib4f( 0, 0, 0, 0, 1 );
		if ( diffuseOfs>=0 ) { glEnableVertexAttribArray( 1 ); glVertexAttribPointer( 1, 4, GL_UNSIGNED_BYTE, GL_TRUE, stride, (void *)(size_t)diffuseOfs ); }
		else { glDisableVertexAttribArray( 1 ); glVertexAttrib4f( 1, 1, 1, 1, 1 ); }
		if ( specularOfs>=0 ) { glEnableVertexAttribArray( 2 ); glVertexAttribPointer( 2, 4, GL_UNSIGNED_BYTE, GL_TRUE, stride, (void *)(size_t)specularOfs ); }
		else { glDisableVertexAttribArray( 2 ); glVertexAttrib4f( 2, 0, 0, 0, 1 ); }
		for ( int t=0; t<PT_MAX_STAGES; t++ ) {
			if ( t<nTex ) { glEnableVertexAttribArray( 3+t ); glVertexAttribPointer( 3+t, 2, GL_FLOAT, GL_FALSE, stride, (void *)(size_t)(texOfs + t*8) ); }
			else { glDisableVertexAttribArray( 3+t ); glVertexAttrib4f( 3+t, 0, 0, 0, 1 ); }
		}

		glDrawArrays( mode, 0, count );
		g_PtStats.Draws++;
		g_PtStats.Verts += count;
		if ( g_PtTrace ) TraceDraw( mode, fvf, (BYTE *)lpVerts, count, stride, diffuseOfs, nTex );
		g_PtGL.pRenderTarget->m_bGPUNewer = TRUE;
		return D3D_OK;
	}
};

// Clips to the game's current viewport, as its own draws are (weather draws).
void PtGfx_ScissorToViewport()
{
	PtDevice *d = g_PtGL.pDevice;
	if ( d && d->m_pViewport && d->m_pViewport->m_VP.dwWidth ) {
		const D3DVIEWPORT2 &vp = d->m_pViewport->m_VP;
		glEnable( GL_SCISSOR_TEST );
		glScissor( vp.dwX, vp.dwY, vp.dwWidth, vp.dwHeight );
	}
	else glDisable( GL_SCISSOR_TEST );
}

void PtGfx_InvalidateDeviceState()
{
	if ( g_PtGL.pDevice ) g_PtGL.pDevice->m_bStateDirty = TRUE;
}

//////////////////////////////////////////////////////////////////////////////
// IDirect3D3
//////////////////////////////////////////////////////////////////////////////
class PtD3D : public PtStub_IDirect3D3 {
public:
	STDMETHODIMP_(HRESULT) QueryInterface( REFIID riid, LPVOID FAR * ppvObj ) override
	{
		if ( !ppvObj ) return E_POINTER;
		if ( riid==IID_IDirect3D3 || riid==IID_IUnknown ) { *ppvObj = this; AddRef(); return D3D_OK; }
		*ppvObj = 0;
		return E_NOINTERFACE;
	}

	STDMETHODIMP_(HRESULT) EnumDevices( LPD3DENUMDEVICESCALLBACK lpCallback, LPVOID lpArg ) override
	{
		D3DDEVICEDESC hw, hel;
		FillDeviceDesc( &hw );
		FillDeviceDesc( &hel );
		hel.dcmColorModel = 0;
		GUID guid = IID_IDirect3DHALDevice;
		char szDesc[] = "ptgfx OpenGL", szName[] = "ptgfx HAL";
		lpCallback( &guid, szDesc, szName, &hw, &hel, lpArg );
		return D3D_OK;
	}

	STDMETHODIMP_(HRESULT) CreateDevice( REFCLSID, LPDIRECTDRAWSURFACE4 lpTarget, LPDIRECT3DDEVICE3 *lplpDevice, LPUNKNOWN ) override
	{
		if ( !lplpDevice ) return DDERR_INVALIDPARAMS;
		*lplpDevice = 0;
		if ( !g_PtGL.bContext ) { PtGL_Log( "CreateDevice before SetCooperativeLevel (no OpenGL context)" ); return DDERR_GENERIC; }
		PtSurface *pTarget = (PtSurface *)lpTarget;
		if ( pTarget && pTarget->MakeRenderTarget() ) {
			g_PtGL.pRenderTarget = pTarget;
			if ( !g_PtGL.pBack ) g_PtGL.pBack = pTarget;
			PtGfx_BindRenderTarget( pTarget );
		}
		PtDevice *pDev = new PtDevice;
		if ( !pDev->Init() ) { pDev->Release(); return DDERR_GENERIC; }
		g_PtGL.pDevice = pDev;
		*lplpDevice = pDev;
		return D3D_OK;
	}

	STDMETHODIMP_(HRESULT) CreateViewport( LPDIRECT3DVIEWPORT3 *lplpVP, LPUNKNOWN ) override
	{
		if ( !lplpVP ) return DDERR_INVALIDPARAMS;
		*lplpVP = new PtViewport;
		return D3D_OK;
	}

	STDMETHODIMP_(HRESULT) EnumZBufferFormats( REFCLSID, LPD3DENUMPIXELFORMATSCALLBACK lpCallback, LPVOID lpArg ) override
	{
		static const DWORD depths[] = { 24, 16 };
		for ( int i=0; i<2; i++ ) {
			DDPIXELFORMAT pf;
			ZeroMemory( &pf, sizeof(pf) );
			pf.dwSize = sizeof(pf);
			pf.dwFlags = DDPF_ZBUFFER;
			pf.dwZBufferBitDepth = depths[i];
			pf.dwZBitMask = depths[i]==24 ? 0xFFFFFF : 0xFFFF;
			if ( lpCallback( &pf, lpArg )==D3DENUMRET_CANCEL ) break;
		}
		return D3D_OK;
	}
};

IDirect3D3 *PtGfx_CreateD3D()
{
	return new PtD3D;
}
