// ptgfx.h - DirectDraw / Direct3D 6 emulation on OpenGL (and later WebGL2).
//
// The game keeps calling lpDD / lpD3D / lpD3DDevice / surfaces exactly as it
// did with DirectX 6. In the GL build (build.bat Release GL, PT_GFX_GL)
// CreateD3D() gets its first object from PtGfx_DirectDrawCreate() instead of
// DirectDrawCreate(), and every other object comes from that one.
//
// Design notes:
// - The game sends pre-transformed vertices (D3DTLVERTEX, XYZRHW): the vertex
//   shader only maps screen pixels to clip space and keeps 1/rhw as w.
// - The back buffer is an OpenGL framebuffer at the game's resolution; Present
//   (primary Blt / Flip) scales it to the window.
// - Every surface keeps a CPU copy of its pixels in its own DirectDraw pixel
//   format (Lock/Blt/GetDC work on it). Textures upload on first use after a
//   change, converted to RGBA.
// - GetDC on the back buffer returns a transparent overlay bitmap. GDI's own
//   bounds tracking says which rectangle changed; only that is uploaded and
//   drawn, in order, without stalling the GPU.
#pragma once

#ifndef DIRECT3D_VERSION
#define DIRECT3D_VERSION 0x0600
#endif
#ifndef DIRECTDRAW_VERSION
#define DIRECTDRAW_VERSION 0x0600
#endif
#include <windows.h>
#include <ddraw.h>
#include <d3d.h>

// Replacement for DirectDrawCreate(): returns the IDirectDraw4 object directly.
HRESULT PtGfx_DirectDrawCreate( LPDIRECTDRAW4 *lplpDD );

// Real-time shadows (ptgfx_shadow.cpp). Each frame the game draws the characters,
// then the map: the characters of this frame and the map of the frame before are
// drawn from the sun into a shadow map, and the map's pixels are darkened where
// something blocks the sun.
//   PtGfx_ShadowFrame( on, cam, centre )  before the characters: starts casting.
//                                  cam = camera position, centre = shadow centre (the
//                                  player), in world units (fixed point / fONE)
//   PtGfx_ShadowRole( role )       PTSHADOW_RECEIVER before the map (builds the shadow
//                                  map; the map then receives and is kept for the
//                                  next frame), PTSHADOW_OFF after it
//   PtGfx_ShadowCamera( ... )      the camera of the vertices that follow (smRENDER3D)
//   PtGfx_ShadowActive()           shadows drawn this frame (skip the old round ones)
enum { PTSHADOW_OFF = 0, PTSHADOW_CASTER, PTSHADOW_RECEIVER };
void PtGfx_ShadowFrame( BOOL bOn, double camX, double camY, double camZ, double centreX, double centreY, double centreZ );
void PtGfx_ShadowRole( int role );
// m: world-to-camera rotation as the engine applies it (camera = offset * m, rows
// _11.._33); w, h, cx, cy: the perspective projection (screen x = (1 + w*x/z) * cx)
void PtGfx_ShadowCamera( const float m[9], float w, float h, float cx, float cy );
BOOL PtGfx_ShadowActive();
// The loaded maps' meshes cast directly from their world coordinates (so map
// shadows do not depend on what the camera sees). One mesh per slot (the game
// keeps two map areas loaded); vertices are x, y, z (world units relative to
// origin), u, v. Batches: triangles of one material in one tile, with the tile's
// x/z extent (relative to origin) and the address of the material's texture
// pointer (read when drawing: leaves and fences are cut out by its alpha).
struct PtShadowMeshBatch { int First, Count; LPDIRECT3DTEXTURE2 *ppTex; float MinX, MinZ, MaxX, MaxZ; };
BOOL PtGfx_ShadowHasMesh( int slot, const void *key, int keyN );		// TRUE: up to date (or shadows off)
void PtGfx_ShadowSetMesh( int slot, const void *key, int keyN, const double origin[3],
	const float *verts, int nVerts, const PtShadowMeshBatch *batches, int nBatches );
// Around the map mesh's own drawing: not kept from the screen (the mesh casts itself).
void PtGfx_ShadowSkipCapture( BOOL bSkip );

// Test view (browser page with ?shadowtest): the intro scene casts onto itself,
// one frame late. cam = camera position in world units.
BOOL PtGfx_ShadowTestBegin( float camX, float camY, float camZ );		// TRUE: test view on
void PtGfx_ShadowTestEnd();

// Weather (ptgfx_weather.cpp): rain and snow falling in the world around the
// camera, splashes where rain lands, and the look of the sky over the world: a
// colour the world is multiplied by (darker under storm clouds, bright in a
// lightning flash) and fog.
//   PtGfx_WeatherLook( ... )       every frame, before the world is drawn
//   PtGfx_WeatherWorld( on )       around the world's drawing (sky, map, characters):
//                                  those draws get the look
//   PtGfx_WeatherDraw( scene )     the falling rain or snow and the splashes, after
//                                  the world (tested against its depth)
struct PtWeatherScene {
	double	Cam[3];				// camera position, world units
	float	Rot[9];				// world-to-camera rotation, as PtGfx_ShadowCamera
	float	ProjW, ProjH, CenterX, CenterY;		// as PtGfx_ShadowCamera
	float	DepthQ1, DepthQ2;	// depth buffer value = q1 + q2/z (z: distance along the view)
	float	ClipHalfH;			// the engine clips the scene to CenterY +- this many pixels
	float	Time;				// seconds
	float	Rain, Snow;			// amount, 0..1
	float	Wind[2];			// x, z in world units per second
	float	Light;				// 0 night .. 1 day: how bright drops and flakes look
	const float *Splashes;		// per splash: x, y, z (world units, relative to Cam), age 0..1
	int		nSplashes;
};
// tint: colour the world is multiplied by; fog: colour and density (per world unit)
void PtGfx_WeatherLook( const float tint[3], const float fog[3], float density );
void PtGfx_WeatherWorld( BOOL bOn );
void PtGfx_WeatherDraw( const PtWeatherScene *s );

// Fonts are forced to non-antialiased so GDI text has no colour fringes when
// drawn into the transparent text overlay.
HFONT WINAPI PtGfx_CreateFontA( int cHeight, int cWidth, int cEscapement, int cOrientation, int cWeight,
	DWORD bItalic, DWORD bUnderline, DWORD bStrikeOut, DWORD iCharSet, DWORD iOutPrecision,
	DWORD iClipPrecision, DWORD iQuality, DWORD iPitchAndFamily, LPCSTR pszFaceName );
