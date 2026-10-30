// ptsound.h - DirectSound emulation on SDL2 audio (see ptsound.cpp).
#pragma once

// compat.h renames the game's DirectSoundCreate calls to this in the GL build.
HRESULT WINAPI PtSound_DirectSoundCreate( LPCGUID pcGuidDevice, LPDIRECTSOUND *ppDS, LPUNKNOWN pUnkOuter );

// Master volumes (0 = off .. 1 = full) on top of the game's own: background music
// (buffers marked with PtSound_MarkMusic) and everything else (sound effects).
void PtSound_SetMasterVolume( float music, float effects );
void PtSound_MarkMusic( LPDIRECTSOUNDBUFFER lpBuffer );
