// ptsound.cpp - DirectSound emulation on SDL2 audio (GL build).
//
// The game's sound code (srcsound\Dxwav.cpp) keeps calling DirectSoundCreate,
// CreateSoundBuffer, Lock/Unlock, Play/Stop, SetVolume/SetPan/SetFrequency,
// GetStatus and GetCurrentPosition. compat.h renames DirectSoundCreate to
// PtSound_DirectSoundCreate, which hands out the objects below. All buffers are
// mixed in software by one SDL audio callback (44.1 kHz stereo 16-bit); in the
// browser the same code runs on Emscripten's SDL2 (Web Audio).

#ifdef PT_GFX_GL

#include <windows.h>
#include <mmsystem.h>
#include <dsound.h>
#include <math.h>
#include <stdio.h>
#include <stdarg.h>
#include <vector>
#include <algorithm>

#define SDL_MAIN_HANDLED
#include <SDL.h>

#include "ptsound.h"
#include "ptsound_stubs.h"

#define PT_MIX_RATE		44100
#define PT_MIX_SAMPLES	1024		// ~23 ms per callback

class PtSoundBuffer;

static inline LONG Clamp( LONG v, LONG lo, LONG hi ) { return v<lo ? lo : v>hi ? hi : v; }

static SDL_AudioDeviceID			s_Device = 0;
static std::vector<PtSoundBuffer *>	s_Buffers;		// every secondary buffer (mixed when playing)
static float s_MusicGain = 1.0f, s_EffectsGain = 1.0f;	// master volumes (PtSound_SetMasterVolume)

static void Log( const char *szFormat, ... )
{
	FILE *fp = fopen( "ptsound.log", "at" );
	if ( !fp ) return;
	va_list args;
	va_start( args, szFormat );
	vfprintf( fp, szFormat, args );
	va_end( args );
	fputc( '\n', fp );
	fclose( fp );
}

//////////////////////////////////////////////////////////////////////////////
// Sound buffer
//////////////////////////////////////////////////////////////////////////////
class PtSoundBuffer : public PtStub_IDirectSoundBuffer {
public:
	BOOL	m_bPrimary;
	std::vector<BYTE> m_Data;
	int		m_Channels, m_Bits, m_BlockAlign;
	DWORD	m_Frames;
	DWORD	m_Freq;				// playback rate (SetFrequency), Hz
	double	m_Pos;				// play position in frames (fractional when resampling)
	BOOL	m_bPlaying, m_bLooping;
	LONG	m_Volume, m_Pan;	// DirectSound units: hundredths of a dB
	float	m_GainL, m_GainR;
	BOOL	m_bMusic;			// background music (master music volume) or a sound effect

	PtSoundBuffer( BOOL bPrimary, const WAVEFORMATEX *pwfx, DWORD dwBytes )
	{
		m_bPrimary = bPrimary;
		m_Channels = 2; m_Bits = 16; m_Freq = PT_MIX_RATE;
		if ( pwfx ) { m_Channels = pwfx->nChannels; m_Bits = pwfx->wBitsPerSample; m_Freq = pwfx->nSamplesPerSec; }
		if ( m_Channels<1 ) m_Channels = 1;
		if ( m_Bits!=8 && m_Bits!=16 ) m_Bits = 16;
		m_BlockAlign = m_Channels * m_Bits/8;
		m_Data.resize( bPrimary ? 0 : dwBytes, m_Bits==8 ? 128 : 0 );
		m_Frames = (DWORD)m_Data.size() / m_BlockAlign;
		m_Pos = 0;
		m_bPlaying = m_bLooping = FALSE;
		m_Volume = 0; m_Pan = 0;
		m_bMusic = FALSE;
		UpdateGains();
		if ( !bPrimary ) {
			SDL_LockAudioDevice( s_Device );
			s_Buffers.push_back( this );
			SDL_UnlockAudioDevice( s_Device );
		}
	}
	~PtSoundBuffer()
	{
		if ( m_bPrimary ) return;
		SDL_LockAudioDevice( s_Device );
		s_Buffers.erase( std::remove( s_Buffers.begin(), s_Buffers.end(), this ), s_Buffers.end() );
		SDL_UnlockAudioDevice( s_Device );
	}

	void UpdateGains()
	{
		float vol = m_Volume<=DSBVOLUME_MIN ? 0.0f : powf( 10.0f, m_Volume/2000.0f );
		float l = 1.0f, r = 1.0f;
		if ( m_Pan<0 ) r = powf( 10.0f, m_Pan/2000.0f );		// pan left: attenuate right
		if ( m_Pan>0 ) l = powf( 10.0f, -m_Pan/2000.0f );
		m_GainL = vol*l; m_GainR = vol*r;
	}

	DWORD PosBytes() const { return ( (DWORD)m_Pos % ( m_Frames ? m_Frames : 1 ) ) * m_BlockAlign; }

	inline float Sample( DWORD frame, int ch ) const
	{
		const BYTE *p = &m_Data[frame*m_BlockAlign];
		if ( m_Bits==8 ) return ( p[ch]-128 ) / 128.0f;
		return ((const short *)p)[ch] / 32768.0f;
	}

	// Adds this buffer into the stereo float mix; called from the audio thread.
	void Mix( float *mix, int frames )
	{
		if ( !m_bPlaying || !m_Frames ) return;
		float master = m_bMusic ? s_MusicGain : s_EffectsGain;
		float gainL = m_GainL*master, gainR = m_GainR*master;
		double step = (double)m_Freq / PT_MIX_RATE;
		for ( int i=0; i<frames; i++ ) {
			if ( m_Pos>=m_Frames ) {
				if ( !m_bLooping ) { m_bPlaying = FALSE; m_Pos = 0; return; }
				m_Pos = fmod( m_Pos, (double)m_Frames );
			}
			DWORD f0 = (DWORD)m_Pos;
			DWORD f1 = f0+1;
			if ( f1>=m_Frames ) f1 = m_bLooping ? 0 : f0;
			float t = (float)( m_Pos - f0 );
			float l = Sample( f0, 0 ) + ( Sample( f1, 0 ) - Sample( f0, 0 ) ) * t;
			float r = l;
			if ( m_Channels>1 ) r = Sample( f0, 1 ) + ( Sample( f1, 1 ) - Sample( f0, 1 ) ) * t;
			mix[i*2]   += l * gainL;
			mix[i*2+1] += r * gainR;
			m_Pos += step;
		}
	}

	STDMETHODIMP_(HRESULT) GetStatus( LPDWORD pdwStatus ) override
	{
		if ( !pdwStatus ) return DSERR_INVALIDPARAM;
		*pdwStatus = m_bPlaying ? ( DSBSTATUS_PLAYING | ( m_bLooping ? DSBSTATUS_LOOPING : 0 ) ) : 0;
		return DS_OK;
	}
	STDMETHODIMP_(HRESULT) GetCurrentPosition( LPDWORD pdwPlay, LPDWORD pdwWrite ) override
	{
		SDL_LockAudioDevice( s_Device );
		DWORD play = PosBytes();
		SDL_UnlockAudioDevice( s_Device );
		if ( pdwPlay ) *pdwPlay = play;
		if ( pdwWrite ) {
			// the mixer reads up to one callback ahead of the play position
			DWORD lead = (DWORD)( (double)PT_MIX_SAMPLES * m_Freq / PT_MIX_RATE ) * m_BlockAlign;
			*pdwWrite = m_Data.empty() ? 0 : ( play + lead ) % (DWORD)m_Data.size();
		}
		return DS_OK;
	}
	STDMETHODIMP_(HRESULT) Lock( DWORD dwOffset, DWORD dwBytes, LPVOID *ppv1, LPDWORD pdw1, LPVOID *ppv2, LPDWORD pdw2, DWORD dwFlags ) override
	{
		DWORD size = (DWORD)m_Data.size();
		if ( !size || !ppv1 || !pdw1 ) return DSERR_INVALIDPARAM;
		if ( dwFlags & DSBLOCK_FROMWRITECURSOR ) GetCurrentPosition( NULL, &dwOffset );
		if ( dwFlags & DSBLOCK_ENTIREBUFFER ) { dwOffset = 0; dwBytes = size; }
		dwOffset %= size;
		if ( dwBytes>size ) dwBytes = size;
		DWORD first = ( dwBytes<size-dwOffset ? dwBytes : size-dwOffset );
		*ppv1 = &m_Data[dwOffset];
		*pdw1 = first;
		if ( ppv2 ) *ppv2 = dwBytes>first ? &m_Data[0] : NULL;
		if ( pdw2 ) *pdw2 = dwBytes-first;
		return DS_OK;
	}
	STDMETHODIMP_(HRESULT) Unlock( LPVOID, DWORD, LPVOID, DWORD ) override { return DS_OK; }
	STDMETHODIMP_(HRESULT) Play( DWORD, DWORD, DWORD dwFlags ) override
	{
		SDL_LockAudioDevice( s_Device );
		m_bLooping = ( dwFlags & DSBPLAY_LOOPING )!=0;
		m_bPlaying = TRUE;
		SDL_UnlockAudioDevice( s_Device );
		return DS_OK;
	}
	STDMETHODIMP_(HRESULT) Stop() override
	{
		SDL_LockAudioDevice( s_Device );
		m_bPlaying = FALSE;
		SDL_UnlockAudioDevice( s_Device );
		return DS_OK;
	}
	STDMETHODIMP_(HRESULT) SetCurrentPosition( DWORD dwPos ) override
	{
		SDL_LockAudioDevice( s_Device );
		m_Pos = m_BlockAlign ? (double)( dwPos / m_BlockAlign ) : 0;
		SDL_UnlockAudioDevice( s_Device );
		return DS_OK;
	}
	STDMETHODIMP_(HRESULT) SetVolume( LONG lVolume ) override
	{
		m_Volume = Clamp( lVolume, DSBVOLUME_MIN, DSBVOLUME_MAX );
		UpdateGains();
		return DS_OK;
	}
	STDMETHODIMP_(HRESULT) GetVolume( LPLONG plVolume ) override { if ( plVolume ) *plVolume = m_Volume; return DS_OK; }
	STDMETHODIMP_(HRESULT) SetPan( LONG lPan ) override
	{
		m_Pan = Clamp( lPan, DSBPAN_LEFT, DSBPAN_RIGHT );
		UpdateGains();
		return DS_OK;
	}
	STDMETHODIMP_(HRESULT) GetPan( LPLONG plPan ) override { if ( plPan ) *plPan = m_Pan; return DS_OK; }
	STDMETHODIMP_(HRESULT) SetFrequency( DWORD dwFrequency ) override
	{
		if ( dwFrequency==DSBFREQUENCY_ORIGINAL ) return DS_OK;
		m_Freq = (DWORD)Clamp( (LONG)dwFrequency, DSBFREQUENCY_MIN, 200000 );
		return DS_OK;
	}
	STDMETHODIMP_(HRESULT) GetFrequency( LPDWORD pdwFrequency ) override { if ( pdwFrequency ) *pdwFrequency = m_Freq; return DS_OK; }
	STDMETHODIMP_(HRESULT) Restore() override { return DS_OK; }
	STDMETHODIMP_(HRESULT) SetFormat( LPCWAVEFORMATEX ) override { return DS_OK; }
};

//////////////////////////////////////////////////////////////////////////////
// Mixer
//////////////////////////////////////////////////////////////////////////////
static void SDLCALL MixCallback( void *, Uint8 *stream, int len )
{
	int frames = len / 4;
	static std::vector<float> mix;
	mix.assign( frames*2, 0.0f );
	for ( size_t i=0; i<s_Buffers.size(); i++ ) s_Buffers[i]->Mix( &mix[0], frames );
	short *out = (short *)stream;
	for ( int i=0; i<frames*2; i++ ) {
		float v = mix[i];
		if ( v>1.0f ) v = 1.0f; else if ( v<-1.0f ) v = -1.0f;
		out[i] = (short)( v*32767.0f );
	}
}

#ifdef PT_WEB
#include <emscripten.h>
EM_JS( int, PtSound_HaveAudio, (), { return ( typeof AudioContext !== 'undefined' || typeof webkitAudioContext !== 'undefined' ) ? 1 : 0; } );
#endif

static BOOL OpenDevice()
{
	if ( s_Device ) return TRUE;
#ifdef PT_WEB
	if ( !PtSound_HaveAudio() ) return FALSE;		// the offline server's Web Worker has no audio
#endif
	SDL_SetMainReady();
	if ( SDL_InitSubSystem( SDL_INIT_AUDIO )!=0 ) { Log( "SDL audio init failed: %s", SDL_GetError() ); return FALSE; }
	SDL_AudioSpec want, have;
	SDL_zero( want );
	want.freq = PT_MIX_RATE;
	want.format = AUDIO_S16SYS;
	want.channels = 2;
	want.samples = PT_MIX_SAMPLES;
	want.callback = MixCallback;
	s_Device = SDL_OpenAudioDevice( NULL, 0, &want, &have, 0 );
	if ( !s_Device ) { Log( "SDL_OpenAudioDevice failed: %s", SDL_GetError() ); return FALSE; }
	SDL_PauseAudioDevice( s_Device, 0 );
	Log( "SDL audio: %s, %d Hz, %d channels, %d samples", SDL_GetCurrentAudioDriver(), have.freq, have.channels, have.samples );
	return TRUE;
}

//////////////////////////////////////////////////////////////////////////////
// IDirectSound
//////////////////////////////////////////////////////////////////////////////
class PtDirectSound : public PtStub_IDirectSound {
public:
	STDMETHODIMP_(HRESULT) CreateSoundBuffer( LPCDSBUFFERDESC pDesc, LPDIRECTSOUNDBUFFER *ppBuffer, LPUNKNOWN ) override
	{
		if ( !pDesc || !ppBuffer ) return DSERR_INVALIDPARAM;
		BOOL bPrimary = ( pDesc->dwFlags & DSBCAPS_PRIMARYBUFFER )!=0;
		if ( !bPrimary && ( !pDesc->lpwfxFormat || !pDesc->dwBufferBytes ) ) return DSERR_INVALIDPARAM;
		*ppBuffer = new PtSoundBuffer( bPrimary, pDesc->lpwfxFormat, pDesc->dwBufferBytes );
		return DS_OK;
	}
	STDMETHODIMP_(HRESULT) SetCooperativeLevel( HWND, DWORD ) override { return DS_OK; }
	STDMETHODIMP_(HRESULT) Compact() override { return DS_OK; }
	STDMETHODIMP_(HRESULT) Initialize( LPCGUID ) override { return DS_OK; }
};

void PtSound_SetMasterVolume( float music, float effects )
{
	if ( music<0 ) music = 0; if ( music>1 ) music = 1;
	if ( effects<0 ) effects = 0; if ( effects>1 ) effects = 1;
	s_MusicGain = music;			// read by the mixer (a float store: no lock needed)
	s_EffectsGain = effects;
}

void PtSound_MarkMusic( LPDIRECTSOUNDBUFFER lpBuffer )
{
	if ( lpBuffer ) ((PtSoundBuffer *)lpBuffer)->m_bMusic = TRUE;
}

HRESULT WINAPI PtSound_DirectSoundCreate( LPCGUID, LPDIRECTSOUND *ppDS, LPUNKNOWN )
{
	if ( !ppDS ) return DSERR_INVALIDPARAM;
	*ppDS = NULL;
	if ( !OpenDevice() ) return DSERR_NODRIVER;
	*ppDS = new PtDirectSound;
	return DS_OK;
}

#endif // PT_GFX_GL
