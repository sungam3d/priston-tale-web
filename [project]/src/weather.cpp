// weather.cpp - weather for the GL renderer (ptgfx): rain falling in the world
// with splashes on the ground, storms with lightning and thunder, fog, cloudy
// skies, and snow in the ice fields.
//
// The weather changes every 8 game hours (6.4 minutes), picked from the game's
// world time, which the server sends at login - so everyone online sees the same
// weather. The page's Weather setting can fix one instead (PtWeather_Setting).
// Indoors (dungeons, rooms, the Bellatra arena) it is always clear.
//
// The original game had rain too (playmain.cpp WeatherSwitch, playsub.cpp
// DrawWeather: flat pictures over the screen), switched off. This uses its rain and
// thunder sounds, its rain sky, and its WeatherMode (1 cloudy, 2 rain), which
// keeps the field's own sky and wind sounds from coming back while it rains.
//
// The look moves smoothly towards the weather picked: ptgfx multiplies the world by
// a colour (darker under clouds), fogs it with distance, and draws the rain and
// snow (ptgfx_weather.cpp).

#include <math.h>
#include <string.h>
#include "smlib3d\\smd3d.h"
#include "smwsock.h"
#include "character.h"
#include "playmain.h"
#include "fileread.h"
#include "field.h"
#include "srcserver\\onserver.h"

#ifdef PT_GFX_GL
#include <dsound.h>
#include "ptgfx\\ptgfx.h"
#include "weather.h"

extern BOOL PlayWav( DWORD dwSound, DWORD dwFlags, DWORD dwVol, DWORD dwfreq );
extern BOOL PlayWav2( DWORD dwSound, DWORD dwFlags, DWORD dwVol, DWORD dwfreq );
extern BOOL StopWav( int i );
extern int ChangeSky( int codeID );
extern void smRender_GetWeatherCamera( PtWeatherScene *s );		// smRend3d.cpp
extern int WeatherMode;
extern int DarkLevel;

int PtWeather_Setting = PTWX_SET_CHANGING;

#define WX_SND_RAIN		1		// wav\rain01.wav, looped
#define WX_SND_THUNDER	2		// wav\thunder1.wav

enum { WX_CLEAR, WX_CLOUDY, WX_RAIN, WX_STORM, WX_FOG, WX_SNOW, WX_BLIZZARD, WX_DUST, WX_KINDS };

// What a kind of weather looks like. Fog density is per world unit (a character is
// about 50 units tall; the default view reaches about 2400).
struct WxLook {
	float	Tint[3];			// the world is multiplied by this
	float	Fog[3];				// fog colour (daylight)
	float	Density;
	float	Rain, Snow;			// 0..1
	float	Wind;				// world units per second
	float	Storm;				// lightning: 1 = often
};
static const WxLook s_Looks[WX_KINDS] = {
	//  tint                      fog colour                 density    rain   snow   wind  storm
	{ { 1.00f, 1.00f, 1.00f }, { 0.70f, 0.72f, 0.75f }, 0.0f,      0.0f,  0.0f,    0.0f, 0.0f },	// clear
	{ { 0.84f, 0.85f, 0.89f }, { 0.62f, 0.64f, 0.68f }, 0.00012f,  0.0f,  0.0f,   20.0f, 0.0f },	// cloudy
	{ { 0.68f, 0.70f, 0.76f }, { 0.50f, 0.53f, 0.58f }, 0.00035f,  0.6f,  0.0f,   40.0f, 0.0f },	// rain
	{ { 0.50f, 0.52f, 0.60f }, { 0.34f, 0.36f, 0.42f }, 0.0005f,   1.0f,  0.0f,  170.0f, 1.0f },	// storm
	{ { 0.88f, 0.89f, 0.91f }, { 0.74f, 0.76f, 0.78f }, 0.0016f,   0.0f,  0.0f,    0.0f, 0.0f },	// fog
	{ { 0.92f, 0.94f, 1.00f }, { 0.82f, 0.85f, 0.90f }, 0.0005f,   0.0f,  0.7f,   25.0f, 0.0f },	// snow
	{ { 0.80f, 0.83f, 0.90f }, { 0.80f, 0.83f, 0.88f }, 0.0011f,   0.0f,  1.0f,  140.0f, 0.0f },	// blizzard
	{ { 0.92f, 0.86f, 0.76f }, { 0.78f, 0.67f, 0.50f }, 0.0012f,   0.0f,  0.0f,   60.0f, 0.0f },	// dust haze (desert)
};

static WxLook s_Cur = s_Looks[WX_CLEAR];		// eases towards the weather picked
static double s_Time, s_LastTime = -1;			// seconds
static float s_Dt;
static double s_BoltTime = -100, s_NextBolt = 0, s_ThunderAt = -1;
static float s_BoltPower;
static int s_ThunderVol;
static BOOL s_RainSound;
static DWORD s_Rand = 12345;

static float Rand01()
{
	s_Rand = s_Rand*1103515245u + 12345u;
	return ( (s_Rand>>8)&0xFFFF )/65536.0f;
}

// 8 game hours (6.4 minutes) of each weather; a hash of the period number picks it
static int ChangingKind()
{
	DWORD h = ( dwGameWorldTime/480 )*2654435761u;
	h ^= h>>15; h *= 2246822519u; h ^= h>>13;
	int r = (int)( h%100 );
	if ( r<45 ) return WX_CLEAR;
	if ( r<60 ) return WX_CLOUDY;
	if ( r<77 ) return WX_RAIN;
	if ( r<87 ) return WX_STORM;
	return WX_FOG;
}

static BOOL Indoors()
{
	if ( !lpCurPlayer || lpCurPlayer->OnStageField<0 ) return TRUE;
	sFIELD *f = StageField[lpCurPlayer->OnStageField];
	if ( !f ) return TRUE;
	return f->State==FIELD_STATE_DUNGEON || f->State==FIELD_STATE_ROOM || f->FieldCode==rsSOD_FIELD;
}

static int PickKind( BOOL bIntro )
{
	switch ( PtWeather_Setting ) {
	case PTWX_SET_CLEAR:	return WX_CLEAR;
	case PTWX_SET_RAIN:		return bIntro || !Indoors() ? WX_RAIN : WX_CLEAR;
	case PTWX_SET_STORM:	return bIntro || !Indoors() ? WX_STORM : WX_CLEAR;
	case PTWX_SET_SNOW:		return bIntro || !Indoors() ? WX_SNOW : WX_CLEAR;
	case PTWX_SET_FOG:		return bIntro || !Indoors() ? WX_FOG : WX_CLEAR;
	}
	if ( bIntro || Indoors() ) return WX_CLEAR;
	int k = ChangingKind();
	int state = StageField[lpCurPlayer->OnStageField]->State;
	if ( state==FIELD_STATE_ICE ) {						// it snows there instead
		if ( k==WX_RAIN ) return WX_SNOW;
		if ( k==WX_STORM ) return WX_BLIZZARD;
	}
	else if ( state==FIELD_STATE_DESERT ) {				// no rain in the desert: dust
		if ( k==WX_RAIN || k==WX_STORM ) return WX_CLOUDY;
		if ( k==WX_FOG ) return WX_DUST;
	}
	return k;
}

static void Ease( float &v, float target, float k ) { v += ( target - v )*k; }

// A lightning flash: two quick flashes, then fading (t: seconds since it struck).
static float FlashAt( double t )
{
	if ( t<0 ) return 0;
	if ( t<0.07 ) return 1.0f;
	if ( t<0.14 ) return 0.25f;
	if ( t<0.22 ) return 0.8f;
	if ( t<0.7 ) return 0.8f*(float)( 1.0 - ( t-0.22 )/0.48 );
	return 0;
}

// 0 at night .. 1 by day (the game darkens the world with DarkLevel up to about 220)
static float Daylight()
{
	int d = DarkLevel;
	if ( d<0 ) d = 0;
	if ( d>220 ) d = 220;
	return 1.0f - d/220.0f*0.8f;
}

void PtWeather_Frame( BOOL bIntro )
{
	// time
	double now = timeGetTime()/1000.0;
	if ( s_LastTime<0 ) s_LastTime = now;
	s_Dt = (float)( now - s_LastTime );
	if ( s_Dt<0 ) s_Dt = 0;
	if ( s_Dt>0.1f ) s_Dt = 0.1f;
	s_LastTime = now;
	s_Time += s_Dt;

	if ( !bIntro ) smConfig.WeatherSwitch = 0;			// the original rain stays off

	// ease towards the weather picked: slowly as it changes by itself, quickly when
	// picked in the settings; at once when going indoors
	int kind = PickKind( bIntro );
	const WxLook &t = s_Looks[kind];
	if ( kind==WX_CLEAR && !bIntro && Indoors() ) s_Cur = t;
	else {
		float k = s_Dt/( PtWeather_Setting==PTWX_SET_CHANGING ? 25.0f : 3.0f );
		if ( k>1 ) k = 1;
		for ( int i=0; i<3; i++ ) { Ease( s_Cur.Tint[i], t.Tint[i], k ); Ease( s_Cur.Fog[i], t.Fog[i], k ); }
		Ease( s_Cur.Density, t.Density, k );
		Ease( s_Cur.Rain, t.Rain, k );
		Ease( s_Cur.Snow, t.Snow, k );
		Ease( s_Cur.Wind, t.Wind, k );
		Ease( s_Cur.Storm, t.Storm, k );
	}

	// lightning, and its thunder later (sound is slower than light: further = later, quieter)
	if ( s_Cur.Storm>0.5f && s_Time>=s_NextBolt ) {
		if ( s_NextBolt>0 ) {
			s_BoltTime = s_Time;
			s_BoltPower = 0.5f + Rand01()*0.5f;
			float delay = 0.3f + Rand01()*2.4f;
			s_ThunderAt = s_Time + delay;
			s_ThunderVol = 400 - (int)( delay*70 );
		}
		s_NextBolt = s_Time + 6.0 + Rand01()*14.0;
	}
	if ( s_ThunderAt>=0 && s_Time>=s_ThunderAt ) {
		PlayWav2( WX_SND_THUNDER, 0, s_ThunderVol, 2205 );
		s_ThunderAt = -1;
	}
	float flash = FlashAt( s_Time - s_BoltTime )*s_BoltPower;

	// rain sound
	if ( s_Cur.Rain>0.15f && !s_RainSound ) {
		PlayWav( WX_SND_RAIN, DSBPLAY_LOOPING, 260 + (int)( 140*s_Cur.Rain ), 2205 );
		s_RainSound = TRUE;
	}
	else if ( s_Cur.Rain<0.08f && s_RainSound ) {
		StopWav( WX_SND_RAIN );
		s_RainSound = FALSE;
	}

	// the game's own weather state: grey rain sky, no field wind sounds
	if ( !bIntro ) {
		float overcast = 1.0f - ( s_Cur.Tint[0] + s_Cur.Tint[1] + s_Cur.Tint[2] )/3.0f;
		int mode = s_Cur.Rain>0.25f ? 2 : ( overcast>0.05f || s_Cur.Snow>0.2f || s_Cur.Density>0.0006f ) ? 1 : 0;
		if ( mode && !WeatherMode && !Indoors() ) ChangeSky( FIELD_BACKIMAGE_RAIN );
		WeatherMode = mode;
	}

	// the world's look
	float day = Daylight();
	float tint[3], fog[3];
	static const float flashCol[3] = { 0.85f, 0.9f, 1.05f };
	for ( int i=0; i<3; i++ ) {
		tint[i] = s_Cur.Tint[i] + flash*flashCol[i];
		fog[i] = s_Cur.Fog[i]*day + flash*0.4f*flashCol[i];
	}
	PtGfx_WeatherLook( tint, fog, s_Cur.Density );
}

// Splashes: rain landing on the ground around the player (the intro: in front of
// the camera), on the highest walkable surface there.
#define WX_SPLASH_MAX	800
#define WX_SPLASH_LIFE	0.32f
struct WxSplash { double x, y, z, t0; };
static WxSplash s_Splash[WX_SPLASH_MAX];
static int s_nSplash;
static float s_SplashCarry;

static BOOL GroundAt( smSTAGE3D *g0, smSTAGE3D *g1, int x, int z, int *y )
{
	int h = -200*fONE;
	if ( g0 ) { int a = g0->GetHeight( x, z ); if ( a>h ) h = a; }
	if ( g1 ) { int b = g1->GetHeight( x, z ); if ( b>h ) h = b; }
	if ( h<=-200*fONE ) return FALSE;
	*y = h;
	return TRUE;
}

static void UpdateSplashes( smSTAGE3D *g0, smSTAGE3D *g1, double cx, double cz )
{
	int keep = 0;
	for ( int i=0; i<s_nSplash; i++ )
		if ( s_Time - s_Splash[i].t0 < WX_SPLASH_LIFE ) s_Splash[keep++] = s_Splash[i];
	s_nSplash = keep;
	if ( s_Cur.Rain<0.05f || ( !g0 && !g1 ) ) { s_SplashCarry = 0; return; }

	s_SplashCarry += 1400.0f*s_Cur.Rain*s_Dt;			// per second
	const double R = 520.0;
	while ( s_SplashCarry>=1 && s_nSplash<WX_SPLASH_MAX ) {
		s_SplashCarry -= 1;
		double x = cx + ( Rand01()*2-1 )*R, z = cz + ( Rand01()*2-1 )*R;
		int y;
		if ( !GroundAt( g0, g1, (int)( x*fONE ), (int)( z*fONE ), &y ) ) continue;
		WxSplash &s = s_Splash[s_nSplash++];
		s.x = x; s.y = (double)y/fONE; s.z = z;
		s.t0 = s_Time - Rand01()*s_Dt;
	}
	if ( s_SplashCarry>1 ) s_SplashCarry = 1;
}

void PtWeather_Draw( BOOL bIntro, smSTAGE3D *ground0, smSTAGE3D *ground1 )
{
	if ( s_Cur.Rain<0.002f && s_Cur.Snow<0.002f && !s_nSplash ) return;

	PtWeatherScene s;
	memset( &s, 0, sizeof(s) );
	smRender_GetWeatherCamera( &s );

	// splashes around the player, or in front of the camera on the login screen
	double cx, cz;
	if ( !bIntro && lpCurPlayer ) { cx = (double)lpCurPlayer->pX/fONE; cz = (double)lpCurPlayer->pZ/fONE; }
	else {
		float fx = s.Rot[2], fz = s.Rot[8], l = sqrtf( fx*fx + fz*fz );
		if ( l>1e-4f ) { fx /= l; fz /= l; }
		cx = s.Cam[0] + fx*350.0; cz = s.Cam[2] + fz*350.0;
	}
	UpdateSplashes( ground0, ground1, cx, cz );
	static float s_Verts[WX_SPLASH_MAX*4];
	for ( int i=0; i<s_nSplash; i++ ) {
		float *v = s_Verts + i*4;
		v[0] = (float)( s_Splash[i].x - s.Cam[0] );
		v[1] = (float)( s_Splash[i].y - s.Cam[1] );
		v[2] = (float)( s_Splash[i].z - s.Cam[2] );
		v[3] = (float)( ( s_Time - s_Splash[i].t0 )/WX_SPLASH_LIFE );
		if ( v[3]>1 ) v[3] = 1;
	}
	s.Splashes = s_Verts;
	s.nSplashes = s_nSplash;

	s.Time = (float)s_Time;
	s.Rain = s_Cur.Rain;
	s.Snow = s_Cur.Snow;
	float a = (float)( s_Time*0.015 );					// the wind turns slowly
	s.Wind[0] = cosf( a )*s_Cur.Wind;
	s.Wind[1] = sinf( a )*s_Cur.Wind;
	s.Light = 0.4f + 0.6f*Daylight() + 0.5f*FlashAt( s_Time - s_BoltTime )*s_BoltPower;
	PtGfx_WeatherDraw( &s );
}

#endif
