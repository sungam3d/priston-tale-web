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
// Test view (browser page with ?shadowtest): the intro scene casts onto itself,
// one frame late. cam = camera position in world units.
void PtGfx_ShadowTestBegin( float camX, float camY, float camZ );
void PtGfx_ShadowTestEnd();

// Fonts are forced to non-antialiased so GDI text has no colour fringes when
// drawn into the transparent text overlay.
HFONT WINAPI PtGfx_CreateFontA( int cHeight, int cWidth, int cEscapement, int cOrientation, int cWeight,
	DWORD bItalic, DWORD bUnderline, DWORD bStrikeOut, DWORD iCharSet, DWORD iOutPrecision,
	DWORD iClipPrecision, DWORD iQuality, DWORD iPitchAndFamily, LPCSTR pszFaceName );
