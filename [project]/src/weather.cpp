// weather.cpp - weather for the GL renderer (ptgfx): rain falling in the world
// with splashes on the ground, storms with lightning and thunder, fog, cloudy
// skies, and snow in the ice fields.
//
// The server's weather changes every 8 game hours (6.4 minutes), picked from the
// game's world time, which the server sends at login and keeps in step - so
// everyone online sees the same weather. The page's Weather setting can pick one
// instead (PtWeather_Setting). Indoors (dungeons, rooms, the Bellatra arena) it is
// always clear. The page shows the server's weather now (Module.ptWeatherNow).
//
// The original game had rain too (playmain.cpp WeatherSwitch, playsub.cpp
// DrawWeather: flat pictures over the screen), switched off. This uses its rain and
// thunder sounds, but not its rain sky or WeatherMode: while WeatherMode is set the
// game stops changing the sky between day and night.
//
// The look moves smoothly towards the weather picked: ptgfx multiplies the world by
// a colour (darker under clouds), fogs it with distance, and draws the rain and
// snow (ptgfx_weather.cpp).
//
// The sun and the moon cross the sky with the game's clock and cast the real-time
// shadows (SunAndMoon below).

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
#ifdef PT_WEB
#include <emscripten.h>
#endif
#include <dsound.h>
#include "ptgfx\\ptgfx.h"
#include "weather.h"

extern BOOL PlayWav( DWORD dwSound, DWORD dwFlags, DWORD dwVol, DWORD dwfreq );
extern BOOL PlayWav2( DWORD dwSound, DWORD dwFlags, DWORD dwVol, DWORD dwfreq );
extern BOOL StopWav( int i );
extern void smRender_GetWeatherCamera( PtWeatherScene *s );		// smRend3d.cpp
extern int DarkLevel;

int PtWeather_Setting = 0;

#define WX_SND_RAIN		1		// wav\rain01.wav, looped
#define WX_SND_THUNDER	2		// wav\thunder1.wav

enum { WX_CLEAR = PTWX_CLEAR, WX_CLOUDY, WX_RAIN, WX_STORM, WX_FOG, WX_SNOW, WX_BLIZZARD, WX_DUST, WX_KINDS };
static const char *s_Names[WX_KINDS] = { "Clear", "Cloudy", "Rain", "Storm", "Fog", "Snow", "Blizzard", "Dust haze" };

// What a kind of weather looks like. Fog density is per world unit (a character is
// about 50 units tall; the default view reaches about 2400).
struct WxLook {
	float	Tint[3];			// the world is multiplied by this
	float	Fog[3];				// fog colour (daylight)
	float	Density;
	float	Rain, Snow;			// 0..1
	float	Wind;				// world units per second
	float	Storm;				// lightning: 1 = often
	float	Sun;				// how much sun / moon gets through: its disc and its shadows
};
static const WxLook s_Looks[WX_KINDS] = {
	//  tint                      fog colour                 density    rain   snow   wind  storm  sun
	{ { 1.00f, 1.00f, 1.00f }, { 0.70f, 0.72f, 0.75f }, 0.0f,      0.0f,  0.0f,    0.0f, 0.0f, 1.00f },	// clear
	{ { 0.84f, 0.85f, 0.89f }, { 0.62f, 0.64f, 0.68f }, 0.00012f,  0.0f,  0.0f,   20.0f, 0.0f, 0.40f },	// cloudy
	{ { 0.68f, 0.70f, 0.76f }, { 0.50f, 0.53f, 0.58f }, 0.00035f,  0.6f,  0.0f,   40.0f, 0.0f, 0.12f },	// rain
	{ { 0.50f, 0.52f, 0.60f }, { 0.34f, 0.36f, 0.42f }, 0.0005f,   1.0f,  0.0f,  170.0f, 1.0f, 0.00f },	// storm
	{ { 0.88f, 0.89f, 0.91f }, { 0.74f, 0.76f, 0.78f }, 0.0016f,   0.0f,  0.0f,    0.0f, 0.0f, 0.35f },	// fog
	{ { 0.92f, 0.94f, 1.00f }, { 0.82f, 0.85f, 0.90f }, 0.0005f,   0.0f,  0.7f,   25.0f, 0.0f, 0.30f },	// snow
	{ { 0.80f, 0.83f, 0.90f }, { 0.80f, 0.83f, 0.88f }, 0.0011f,   0.0f,  1.0f,  140.0f, 0.0f, 0.05f },	// blizzard
	{ { 0.92f, 0.86f, 0.76f }, { 0.78f, 0.67f, 0.50f }, 0.0012f,   0.0f,  0.0f,   60.0f, 0.0f, 0.55f },	// dust haze (desert)
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

// The server's weather where the player is (-1: indoors / not in the world)
static int ServerKind()
{
	if ( Indoors() ) return -1;
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

static int PickKind( BOOL bIntro )
{
	int own = PtWeather_Setting - 1;
	if ( own>=0 && own<WX_KINDS ) return bIntro || !Indoors() ? own : WX_CLEAR;
	if ( bIntro ) return WX_CLEAR;						// no server time yet
	int k = ServerKind();
	return k<0 ? WX_CLEAR : k;
}

// Tells the page the server's weather here and when it changes (once a second).
static void Report( BOOL bIntro )
{
#ifdef PT_WEB
	static double s_Last = -10;
	if ( s_Time - s_Last < 1.0 ) return;
	s_Last = s_Time;
	int k = bIntro ? -2 : ServerKind();
	const char *name = k>=0 ? s_Names[k] : k==-1 ? "Indoors (dry)" : "";
	int secs = (int)( ( 480 - dwGameWorldTime%480 )*( GAME_WORLDTIME_MIN/1000.0 ) );		// game minutes left, real seconds
	EM_ASM({
		var w = {};
		w.name = UTF8ToString($0);
		w.secs = $1;
		w.at = Date.now();
		Module['ptWeatherNow'] = w;
	}, name, bIntro ? 0 : secs );
#endif
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

// ---- the sun and the moon ----
// The game's day: the sun rises at GAME_HOUR_DAY (4:00) and is down when it gets
// dark (GAME_HOUR_DARKNESS, 23:00); the moon is up from 20:00 to 7:00 and goes
// through its phases in 8 game days. A game minute is GAME_WORLDTIME_MIN ms, so a
// whole day takes about 19 real minutes: the sun visibly moves, shadows turn with
// it. The sun casts the shadows by day, the moon (fainter, softer) by night; clouds
// take both away. The page can hold the clock for testing: Module.ptSunHour = 0..24.
#define SUN_RISE		((double)GAME_HOUR_DAY)
#define SUN_SET			((double)GAME_HOUR_DARKNESS)
#define MOON_RISE		20.0
#define MOON_SET		31.0			// 7:00 the next morning
#define SHADOW_MIN_ELEV	18.0f			// a lower light: shadows stop growing (the shadow map's depth, ptgfx_shadow.cpp)
#define WX_PI			3.14159265358979

static double s_Hour, s_Day;			// game clock: hour 0..24 (with the minute's fraction), days
static PtSkyLight s_Sun, s_Moon;

static void GameClock()
{
	// the game's clock steps once a game minute, not always on time: this one runs
	// smoothly at the same rate and is pulled gently towards it (set at once when it
	// jumps: first time, server correction)
	static double s_Mins = -1;
	static DWORD s_Last;
	DWORD now = timeGetTime();
	double m = (double)( dwGameWorldTime + dwGameTimeOffset ) + 0.5;
	double dt = s_Mins<0 ? 0 : ( now - s_Last )/1000.0;
	s_Last = now;
	if ( dt>0.5 ) dt = 0.5;
	s_Mins += dt*1000.0/GAME_WORLDTIME_MIN;
	double err = m - s_Mins;
	if ( s_Mins<0 || fabs( err )>5 ) s_Mins = m;
	else s_Mins += err*( dt*0.5 );
	double mins = s_Mins;
	s_Day = mins/1440.0;
	s_Hour = fmod( mins/60.0, 24.0 );
#ifdef PT_WEB
	double held = EM_ASM_DOUBLE({ var h = Module['ptSunHour']; return typeof h=='number' ? h : -1; });
	if ( held>=0 ) s_Hour = fmod( held, 24.0 );
	s_Day += EM_ASM_DOUBLE({ var d = Module['ptSunDay']; return typeof d=='number' ? d : 0; });		// test: another moon phase
#endif
}

// test (?suntest=degrees): the sun's and moon's paths turned, to bring them in front of a camera
static double TestTurn()
{
#ifdef PT_WEB
	static double s_Turn = -1e9;
	if ( s_Turn==-1e9 ) s_Turn = EM_ASM_DOUBLE({ var m = /[?&]suntest=(-?[0-9.]+)/.exec(location.search); return m ? +m[1] : 0; });
	return s_Turn;
#else
	return 0;
#endif
}

// Direction towards a body crossing the sky, p: 0 rising .. 1 setting (outside:
// below the horizon). It rises in the east (+x) and its path leans south (+z), so
// at its highest it stands peakDeg up; turnDeg turns the whole path.
static void Arc( double p, double peakDeg, double turnDeg, float out[3] )
{
	double a = p*WX_PI, tilt = ( 90.0 - peakDeg )*WX_PI/180.0, turn = turnDeg*WX_PI/180.0;
	double x = cos( a ), up = sin( a )*cos( tilt ), side = sin( a )*sin( tilt );
	out[0] = (float)( x*cos( turn ) - side*sin( turn ) );
	out[1] = (float)up;
	out[2] = (float)( x*sin( turn ) + side*cos( turn ) );
}

static float Smooth( float a, float b, float x )
{
	float t = ( x - a )/( b - a );
	t = t<0 ? 0 : t>1 ? 1 : t;
	return t*t*( 3 - 2*t );
}

static float ElevDeg( const float d[3] ) { return asinf( d[1]<-1 ? -1 : d[1]>1 ? 1 : d[1] )*57.29578f; }

// where the sun and the moon are at hour h
static void Places( double h, float sun[3], float moon[3] )
{
	double hm = h<12 ? h + 24 : h, turn = TestTurn();
	Arc( ( h - SUN_RISE )/( SUN_SET - SUN_RISE ), 58.0, turn, sun );
	Arc( ( hm - MOON_RISE )/( MOON_SET - MOON_RISE ), 42.0, 25.0 + turn, moon );
}

static void SunAndMoon()
{
	GameClock();
	Places( s_Hour, s_Sun.Dir, s_Moon.Dir );
	float se = ElevDeg( s_Sun.Dir ), me = ElevDeg( s_Moon.Dir );
	float clear = s_Cur.Sun;
	float night = 1 - Smooth( -5, 1, se );

	// the sun: orange low down, white high up
	float w = Smooth( 0, 25, se );
	static const float low[3] = { 1.0f, 0.50f, 0.22f }, high[3] = { 1.0f, 0.93f, 0.80f };
	for ( int i=0; i<3; i++ ) s_Sun.Color[i] = low[i] + ( high[i] - low[i] )*w;
	s_Sun.Amount = Smooth( -2.5f, 1.5f, se )*clear;
	s_Sun.Lit = 1;
	s_Sun.Side = 1;
	// the moon: its phase over 8 days; faint while the sun is up
	double ph = fmod( s_Day/8.0, 1.0 );
	s_Moon.Lit = (float)( 0.5 - 0.5*cos( 2*WX_PI*ph ) );			// 0 new .. 1 full (half way)
	s_Moon.Side = ph<0.5 ? 1.0f : -1.0f;
	s_Moon.Color[0] = 0.80f; s_Moon.Color[1] = 0.84f; s_Moon.Color[2] = 0.92f;
	s_Moon.Amount = Smooth( -2.5f, 1.5f, me )*clear*( 0.25f + 0.75f*night );

	// shadows: from whichever is stronger (they hand over when both are near zero)
	float sunS = Smooth( 0, 12, se );
	float moonS = Smooth( 0, 12, me )*night*0.5f*( 0.3f + 0.7f*s_Moon.Lit );
	const float *towards = sunS>=moonS ? s_Sun.Dir : s_Moon.Dir;
	float d[3] = { -towards[0], -towards[1], -towards[2] };
	float minY = sinf( SHADOW_MIN_ELEV*(float)WX_PI/180 );
	if ( -d[1]<minY ) {
		float hl = sqrtf( d[0]*d[0] + d[2]*d[2] ), c = sqrtf( 1 - minY*minY );
		if ( hl>1e-5f ) { d[0] *= c/hl; d[2] *= c/hl; }
		else { d[0] = c; d[2] = 0; }
		d[1] = -minY;
	}
	PtGfx_ShadowLight( d, ( sunS>=moonS ? sunS : moonS )*clear, sunS>=moonS ? 1.0f : 2.6f );
}

void PtWeather_DrawSky( BOOL bIntro )
{
	if ( !bIntro && Indoors() ) return;
	PtWeatherScene s;
	memset( &s, 0, sizeof(s) );
	smRender_GetWeatherCamera( &s );			// the sky's camera (HoSky sets the player's view)
	PtGfx_SkyLights( &s, &s_Sun, &s_Moon );
#ifdef PT_WEB
	static int s_Test = -1;
	static double s_Logged = -10;
	if ( s_Test<0 ) s_Test = EM_ASM_INT({ return /[?&]suntest/.test(location.search) ? 1 : 0; });
	if ( s_Test && s_Time - s_Logged > 3 ) {		// ?suntest: where the camera looks and where the sun / moon are
		s_Logged = s_Time;
		printf( "[sun] hour %.2f day %.2f | view %.2f %.2f %.2f | sun %.2f %.2f %.2f (%.2f) | moon %.2f %.2f %.2f (%.2f lit %.2f)\n",
			s_Hour, s_Day, s.Rot[2], s.Rot[5], s.Rot[8], s_Sun.Dir[0], s_Sun.Dir[1], s_Sun.Dir[2], s_Sun.Amount,
			s_Moon.Dir[0], s_Moon.Dir[1], s_Moon.Dir[2], s_Moon.Amount, s_Moon.Lit );
	}
#endif
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
		float k = s_Dt/( PtWeather_Setting==0 ? 25.0f : 3.0f );
		if ( k>1 ) k = 1;
		for ( int i=0; i<3; i++ ) { Ease( s_Cur.Tint[i], t.Tint[i], k ); Ease( s_Cur.Fog[i], t.Fog[i], k ); }
		Ease( s_Cur.Density, t.Density, k );
		Ease( s_Cur.Rain, t.Rain, k );
		Ease( s_Cur.Snow, t.Snow, k );
		Ease( s_Cur.Wind, t.Wind, k );
		Ease( s_Cur.Storm, t.Storm, k );
		Ease( s_Cur.Sun, t.Sun, k );
	}
	SunAndMoon();

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

	Report( bIntro );

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
