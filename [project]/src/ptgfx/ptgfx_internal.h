// ptgfx_internal.h - classes shared by ptgfx_surface.cpp and ptgfx_device.cpp.
#pragma once

#include "ptgfx.h"
#include "ptgfx_stubs.h"
#include "ptgl.h"

//////////////////////////////////////////////////////////////////////////////
// DirectDraw pixel format (any 16/24/32-bit RGB(A) layout) <-> RGBA8
//////////////////////////////////////////////////////////////////////////////
struct PtFormat {
	DWORD	bpp;				// bits per pixel (16, 24, 32)
	DWORD	mask[4];			// R, G, B, A
	int		shift[4];
	int		bits[4];

	void Set( const DDPIXELFORMAT &pf );
	void SetRGB( DWORD bpp, DWORD r, DWORD g, DWORD b, DWORD a );
	void ToDDPF( DDPIXELFORMAT *pf ) const;

	DWORD	BytesPerPixel() const { return (bpp+7)/8; }
	DWORD	Read( const BYTE *p ) const;
	void	Write( BYTE *p, DWORD raw ) const;
	DWORD	ToRGBA( DWORD raw ) const;		// returns R | G<<8 | B<<16 | A<<24 (GL_RGBA bytes)
	DWORD	FromRGBA( DWORD rgba ) const;
};

// Display format used for the primary/back buffer and for offscreen surfaces
// created without a pixel format (the game assumes 16-bit 565 by default).
extern PtFormat	g_PtDisplayFormat;

class PtSurface;
class PtDevice;
class PtDirectDraw;

//////////////////////////////////////////////////////////////////////////////
// IDirect3DTexture2: a view of a surface (shares its reference count)
//////////////////////////////////////////////////////////////////////////////
class PtTexture : public PtStub_IDirect3DTexture2 {
public:
	PtSurface	*m_pSurface;
	STDMETHODIMP_(HRESULT) QueryInterface( REFIID riid, LPVOID FAR * ppvObj ) override;
	STDMETHODIMP_(ULONG) AddRef() override;
	STDMETHODIMP_(ULONG) Release() override;
	STDMETHODIMP_(HRESULT) GetHandle( LPDIRECT3DDEVICE2, LPD3DTEXTUREHANDLE lpHandle ) override { if ( lpHandle ) *lpHandle = (D3DTEXTUREHANDLE)(size_t)this; return D3D_OK; }
	STDMETHODIMP_(HRESULT) Load( LPDIRECT3DTEXTURE2 ) override { return D3D_OK; }
};

//////////////////////////////////////////////////////////////////////////////
// IDirectDrawSurface4
//////////////////////////////////////////////////////////////////////////////
enum PtSurfaceKind { PTSURF_PLAIN, PTSURF_PRIMARY, PTSURF_BACK, PTSURF_ZBUFFER };

class PtSurface : public PtStub_IDirectDrawSurface4 {
public:
	PtSurface();
	virtual ~PtSurface();

	DDSURFACEDESC2	m_Desc;
	PtFormat		m_Format;
	PtSurfaceKind	m_Kind;
	BYTE			*m_pPixels;				// CPU copy, m_Pitch bytes per row
	LONG			m_Pitch;
	PtSurface		*m_pAttached;			// next mip level / flip-chain back buffer (owned)
	PtTexture		m_Texture;

	DWORD			m_dwLockFlags;			// flags of the current Lock (DDLOCK_READONLY: Unlock changes nothing)
	BOOL			m_bHasSrcKey;
	DDCOLORKEY		m_SrcKey;

	// OpenGL side
	GLuint			m_GLTex;
	BOOL			m_bDirty;				// CPU copy changed: re-upload before use
	BOOL			m_bGPUNewer;			// rendered into on the GPU: CPU copy is stale
	GLuint			m_FBO;					// when used as a render target
	GLuint			m_DepthRB;

	// GetDC support
	HDC				m_hDC;
	HBITMAP			m_hDIB;
	HGDIOBJ			m_hOldBmp;
	DWORD			*m_pDIBBits;			// 32-bit BGRA top-down

	int		Width() const	{ return (int)m_Desc.dwWidth; }
	int		Height() const	{ return (int)m_Desc.dwHeight; }
	BOOL	IsTextureCap() const { return (m_Desc.ddsCaps.dwCaps & DDSCAPS_TEXTURE)!=0; }

	BOOL	Create( DDSURFACEDESC2 *lpDesc, PtSurfaceKind kind );
	GLuint	GetGLTexture();						// uploads if dirty
	BOOL	MakeRenderTarget();					// FBO for SetRenderTarget
	void	ReadBackFromGPU();					// GPU -> CPU copy
	DWORD	PixelRGBA( int x, int y ) const;
	BOOL	IsKeyed( DWORD raw ) const { return m_bHasSrcKey && raw>=m_SrcKey.dwColorSpaceLowValue && raw<=m_SrcKey.dwColorSpaceHighValue; }

	// IUnknown
	STDMETHODIMP_(HRESULT) QueryInterface( REFIID riid, LPVOID FAR * ppvObj ) override;
	// IDirectDrawSurface4
	STDMETHODIMP_(HRESULT) AddAttachedSurface( LPDIRECTDRAWSURFACE4 ) override;
	STDMETHODIMP_(HRESULT) DeleteAttachedSurface( DWORD, LPDIRECTDRAWSURFACE4 ) override;
	STDMETHODIMP_(HRESULT) GetAttachedSurface( LPDDSCAPS2, LPDIRECTDRAWSURFACE4 FAR * ) override;
	STDMETHODIMP_(HRESULT) Blt( LPRECT, LPDIRECTDRAWSURFACE4, LPRECT, DWORD, LPDDBLTFX ) override;
	STDMETHODIMP_(HRESULT) BltFast( DWORD, DWORD, LPDIRECTDRAWSURFACE4, LPRECT, DWORD ) override;
	STDMETHODIMP_(HRESULT) Flip( LPDIRECTDRAWSURFACE4, DWORD ) override;
	STDMETHODIMP_(HRESULT) GetBltStatus( DWORD ) override { return DD_OK; }
	STDMETHODIMP_(HRESULT) GetFlipStatus( DWORD ) override { return DD_OK; }
	STDMETHODIMP_(HRESULT) GetCaps( LPDDSCAPS2 ) override;
	STDMETHODIMP_(HRESULT) GetColorKey( DWORD, LPDDCOLORKEY ) override;
	STDMETHODIMP_(HRESULT) SetColorKey( DWORD, LPDDCOLORKEY ) override;
	STDMETHODIMP_(HRESULT) GetDC( HDC FAR * ) override;
	STDMETHODIMP_(HRESULT) ReleaseDC( HDC ) override;
	STDMETHODIMP_(HRESULT) GetPixelFormat( LPDDPIXELFORMAT ) override;
	STDMETHODIMP_(HRESULT) GetSurfaceDesc( LPDDSURFACEDESC2 ) override;
	STDMETHODIMP_(HRESULT) IsLost() override { return DD_OK; }
	STDMETHODIMP_(HRESULT) Restore() override { return DD_OK; }
	STDMETHODIMP_(HRESULT) Lock( LPRECT, LPDDSURFACEDESC2, DWORD, HANDLE ) override;
	STDMETHODIMP_(HRESULT) Unlock( LPRECT ) override;
	STDMETHODIMP_(HRESULT) SetClipper( LPDIRECTDRAWCLIPPER ) override { return DD_OK; }
	STDMETHODIMP_(HRESULT) PageLock( DWORD ) override { return DD_OK; }
	STDMETHODIMP_(HRESULT) PageUnlock( DWORD ) override { return DD_OK; }
};

//////////////////////////////////////////////////////////////////////////////
// Shared OpenGL state
//////////////////////////////////////////////////////////////////////////////
struct PtGLState {
	HWND		hWnd;
	BOOL		bContext;
	PtSurface	*pPrimary;
	PtSurface	*pBack;				// game resolution render target
	PtSurface	*pRenderTarget;		// current D3D render target
	PtDevice	*pDevice;

	// 2D program: textured screen-space quad (blits, text overlay)
	GLuint		Prog2D;
	GLint		u2D_Screen, u2D_Tex, u2D_Keyed, u2D_Opaque;
	GLuint		VAO, VBO;

	// text overlay for GetDC on the back buffer
	HDC			hOverlayDC;
	HBITMAP		hOverlayDIB;
	HGDIOBJ		hOverlayOld;
	DWORD		*pOverlayBits;		// BGRA top-down, back buffer size
	GLuint		OverlayTex;
	int			OverlayW, OverlayH;
};
extern PtGLState g_PtGL;

// per-frame counters for diagnostics (logged to ptgfx.log by PtGfx_Present)
struct PtStats { int Draws, Verts, Blits, Fills, Overlays, Uploads, RTSwitches; };
extern PtStats g_PtStats;

// One-frame trace (ptgfx_trace.txt): every draw/clear/blit of frame 300 with its
// states and the colour at a few screen points afterwards.
extern FILE *g_PtTrace;
void PtGfx_TracePixels( const char *szWhat );

BOOL	PtGfx_InitGL( HWND hWnd );
GLuint	PtGfx_CompileProgram( const char *szVS, const char *szFS, const char **attribs, int nAttribs );
void	PtGfx_BindRenderTarget( PtSurface *pSurface );
void	PtGfx_Draw2DQuad( GLuint tex, int texW, int texH, const RECT &src, const RECT &dst, BOOL bKeyed, BOOL bOpaque, BOOL bBlend = FALSE );
void	PtText_Install();					// ptgfx_text.cpp: own text rasterizer in place of GDI text

// Text overlay pixel (BGRA) meaning "nothing drawn here". Our text rasterizer
// stores antialiased coverage in the alpha byte; GDI leaves it 0.
#define PT_OVERLAY_KEY	0x00FF00FF
void	PtGfx_Present( const RECT *lpSrc );
void	PtGfx_InvalidateDeviceState();		// device must re-apply its GL state
void	PtGfx_ScissorToViewport();			// clip to the game's current viewport

//////////////////////////////////////////////////////////////////////////////
// Real-time shadows (ptgfx_shadow.cpp)
//////////////////////////////////////////////////////////////////////////////
#define PTSHADOW_MAP_SIZE	2048
#define PTSHADOW_TEX_UNIT	8			// texture unit of the shadow map (stages use 0-7)

#define PTSHADOW_VERT_FLOATS	6		// kept per caster vertex: x, y, z, rhw, u, v

// Caster triangles of one draw camera; Tex = cut-out texture (alpha-tested), or 0.
struct PtShadowBatch { int First, Count; GLuint Tex; float Rot[9], Proj[4]; };

// Caster triangles kept from one frame's drawing.
struct PtShadowList {
	float	*pVerts;
	int		nVerts, nMaxVerts;
	PtShadowBatch *pBatches;
	int		nBatches, nMaxBatches;
	double	Cam[3];					// camera position when they were drawn (world units)
};

// A map mesh uploaded for casting (PtGfx_ShadowSetMesh).
#define PTSHADOW_MESH_SLOTS	2
struct PtShadowMesh {
	const void	*Key;
	int		KeyN;
	double	Origin[3];
	GLuint	VBO;
	PtShadowMeshBatch *pBatches;
	int		nBatches;
};

struct PtShadowState {
	BOOL	bAllowed;				// setting (browser: Module.ptShadows)
	BOOL	bSkipCapture;			// map mesh being drawn: it casts from PtShadowMesh
	PtShadowMesh Mesh[PTSHADOW_MESH_SLOTS];
	GLuint	MeshProg;
	GLint	um_Offset, um_R, um_U, um_D, um_AlphaTest;
	double	SunR[3], SunU[3];		// shadow map x, y axes (world units), for snapping the centre to map pixels
	BOOL	bTestAllowed;			// test view in the intro (browser: ?shadowtest)
	BOOL	bFrame;					// shadows drawn this frame
	int		Role;					// PTSHADOW_*
	BOOL	bMapReady;				// shadow map built for this frame
	BOOL	bCamera;				// Rot/Proj set for the current draws
	float	Rot[9];					// camera -> world rotation (GLSL mat3, column-major)
	float	Proj[4];				// w, h, centre x, centre y
	double	Centre[3];				// shadow centre (the player), world units
	float	Eye[3];					// camera position relative to the shadow centre
	float	Dark;					// how much a shadow darkens (0..1; browser: Module.ptShadowDark, times Strength)
	float	Strength, Soft;			// the light's (PtGfx_ShadowLight)
	float	LightDir[3];			// the direction the light travels
	float	LightR[3], LightU[3], LightD[3];	// sun axes, scaled to the shadow map's [-1,1] box
	GLuint	Prog, VAO, VBO, FBO, DepthTex;
	GLint	u_Rot, u_Proj, u_Eye, u_R, u_U, u_D, u_AlphaTest;
	PtShadowList Chars;				// characters of this frame
	PtShadowList World[2];			// the map: [WorldCur] being drawn now, the other from the frame before
	int		WorldCur;
	int		DbgCam, DbgCamBad, DbgDraws, DbgSkipped;	// counters for the test log
};
extern PtShadowState g_PtShadow;
BOOL	PtShadow_Init();									// GL objects (device Init)
// a caster draw (triangle list): uvOfs = byte offset of the texture coordinates
// in a vertex (-1: none), tex = cut-out texture or 0
void	PtShadow_Capture( const BYTE *v, DWORD count, int stride, int uvOfs, GLuint tex );

// Weather (ptgfx_weather.cpp). The device shader gives the world's draws (bWorld)
// the weather's look: multiplied by Tint, fogged towards Fog by eye distance.
struct PtWeatherState {
	BOOL	bWorld;					// the world is being drawn
	BOOL	bLook;					// Tint / Fog differ from clear weather
	float	Tint[3], Fog[3], Density;
	GLuint	Prog, SplashProg, SkyProg, VAO, VBO;
	BOOL	bInitTried;
};
extern PtWeatherState g_PtWeather;

// GLSL shared by the device and caster shaders: screen vertex (x, y pixels, z, rhw)
// -> world position relative to the shadow centre -> sun space [-1,1].
#define PTSHADOW_GLSL \
	"uniform mat3 uSRot; uniform vec4 uSProj; uniform vec3 uSEye;\n" \
	"uniform vec3 uSLightR; uniform vec3 uSLightU; uniform vec3 uSLightD;\n" \
	"vec3 ptSunPos( vec4 p ) {\n" \
	"	float cz = p.w > 0.0 ? 1.0/p.w : 1.0;\n" \
	"	vec3 cam = vec3( (p.x/uSProj.z - 1.0)*cz/uSProj.x, (1.0 - p.y/uSProj.w)*cz/uSProj.y, cz );\n" \
	"	vec3 w = uSRot*cam + uSEye;\n" \
	"	return vec3( dot( w, uSLightR ), dot( w, uSLightU ), dot( w, uSLightD ) );\n" \
	"}\n"

// GLSL header: desktop GL 3.3 now, "#version 300 es" for the browser build.
#ifdef PT_WEB
#define PTGL_GLSL_HEADER "#version 300 es\nprecision highp float;\nprecision highp int;\n"
#else
#define PTGL_GLSL_HEADER "#version 330 core\n"
#endif
