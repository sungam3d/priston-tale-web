// ptgfx_weather.cpp - weather drawn into the game's world: rain and snow falling
// around the camera, splashes where rain lands, and the sky's look over the world
// (colour and fog, applied by the device shader, ptgfx_device.cpp).
//
// Rain drops and snow flakes have no vertex data: each is a quad made in the
// vertex shader from its number. A drop's place is a fixed random point in a box
// of the world, moving with the wind and gravity and wrapping around the box,
// which is kept in front of the camera - so drops stay put in the world when the
// camera turns. They are projected exactly like the game's own vertices
// (PtWeatherScene: the engine's camera and projection) and depth tested against
// the world, so walls and characters hide the rain behind them.
// The game decides the weather (weather.cpp).

#include <stdio.h>
#include <math.h>
#include <string.h>
#include "ptgfx_internal.h"

PtWeatherState g_PtWeather;

// camera space (z = distance along the view) -> screen pixels x, y, depth value, z;
// then clip space the way the device's vertex shader places the game's vertices
#define WX_GLSL \
	"uniform mat3 uRot; uniform vec4 uProj; uniform vec2 uZQ; uniform vec2 uScreen;\n" \
	"vec4 wxScreen( vec3 c ) {\n" \
	"	return vec4( uProj.z*(1.0 + uProj.x*c.x/c.z), uProj.w*(1.0 - uProj.y*c.y/c.z), uZQ.x + uZQ.y/c.z, c.z );\n" \
	"}\n" \
	"vec4 wxClip( vec2 px, vec4 s ) {\n" \
	"	float w = s.w;\n" \
	"	return vec4( ((px.x+0.5)/uScreen.x*2.0-1.0)*w, ((px.y+0.5)/uScreen.y*2.0-1.0)*w, (clamp( s.z, 0.0, 1.0 )*0.9999*2.0-1.0)*w, w );\n" \
	"}\n" \
	"const float WX_NEAR = 24.0;\n" \
	"const vec4 WX_OUT = vec4( 0.0, 0.0, 2.0, 1.0 );\n"			/* beyond the far plane: not drawn */

// Drops / flakes: 6 vertices each (two triangles). Corner: side -1/+1 across,
// end 0 (tail, up the fall) / 1 (head).
static const char *s_FallVS =
	WX_GLSL
	"uniform vec3 uBox; uniform vec3 uCentre; uniform vec3 uShift; uniform vec3 uVel;\n"
	"uniform float uStreak; uniform float uTime; uniform float uPx; uniform int uSnow;\n"
	"out vec2 vC; out float vA;\n"
	"float wxHash( int n ) {\n"
	"	uint x = uint(n)*747796405u + 2891336453u;\n"
	"	x = ((x >> ((x >> 28u) + 4u)) ^ x)*277803737u;\n"
	"	x = (x >> 22u) ^ x;\n"
	"	return float(x)*(1.0/4294967296.0);\n"
	"}\n"
	"void main() {\n"
	"	int id = gl_VertexID/6, k = gl_VertexID - id*6;\n"
	"	float side = ( k==1 || k==2 || k==4 ) ? 1.0 : -1.0;\n"
	"	float end = ( k==2 || k==4 || k==5 ) ? 1.0 : 0.0;\n"
	"	vec3 q = fract( vec3( wxHash(id*3), wxHash(id*3+1), wxHash(id*3+2) ) + uShift ) - 0.5;\n"
	"	vec3 p = uCentre + q*uBox;\n"						// relative to the camera
	// fade out towards the box's sides, where drops wrap around
	"	vA = ( 1.0 - smoothstep( 0.3, 0.5, max( abs(q.x), abs(q.z) ) ) )*( 1.0 - smoothstep( 0.38, 0.5, abs(q.y) ) );\n"
	"	if ( uSnow!=0 ) {\n"
	"		float s = wxHash( id*3+7 );\n"
	"		p.x += sin( uTime*(0.6+s) + s*40.0 )*12.0;\n"	// flakes sway as they fall
	"		p.z += cos( uTime*(0.5+s*0.8) + s*30.0 )*12.0;\n"
	"		vec3 c = uRot*p;\n"
	"		if ( c.z<WX_NEAR ) { gl_Position = WX_OUT; vC = vec2( 0.0 ); return; }\n"
	"		vec4 sc = wxScreen( c );\n"
	"		float r = max( 1.4*uPx, (1.4 + s*1.6)*uProj.x*uProj.z/c.z );\n"	// radius in pixels
	"		vC = vec2( side, end*2.0 - 1.0 );\n"
	"		gl_Position = wxClip( sc.xy + vC*r, sc );\n"
	"		return;\n"
	"	}\n"
	"	vec3 ch = uRot*p, ct = uRot*( p - uVel*uStreak );\n"
	"	if ( ch.z<WX_NEAR || ct.z<WX_NEAR ) { gl_Position = WX_OUT; vC = vec2( 0.0 ); return; }\n"
	"	vec4 sh = wxScreen( ch ), st = wxScreen( ct );\n"
	"	vec2 d = sh.xy - st.xy;\n"
	"	float len = length( d );\n"
	"	vec2 dir = len>0.01 ? d/len : vec2( 0.0, 1.0 );\n"
	"	if ( len<3.0*uPx ) sh.xy = st.xy + dir*3.0*uPx;\n"	// seen from above: short streaks, not dots
	"	float hw = 260.0*uPx/ch.z;\n"							// half width in pixels, thinner further away
	"	vA *= clamp( hw/0.6, 0.3, 1.0 );\n"					// thinner than a pixel: fainter instead
	"	hw = clamp( hw, 0.6, 1.5*uPx );\n"
	"	vec4 s = end>0.5 ? sh : st;\n"
	"	vC = vec2( side, end );\n"
	"	gl_Position = wxClip( s.xy + vec2( -dir.y, dir.x )*side*hw, s );\n"
	"}\n";

static const char *s_FallFS =
	"in vec2 vC; in float vA;\n"
	"uniform vec4 uColor; uniform int uSnow;\n"
	"out vec4 oColor;\n"
	"void main() {\n"
	"	float a = uSnow!=0 ? 1.0 - smoothstep( 0.3, 1.0, length( vC ) )\n"
	"	                   : ( 1.0 - abs( vC.x ) )*mix( 0.15, 1.0, vC.y );\n"	// brightest at the head
	"	oColor = vec4( uColor.rgb, uColor.a*a*vA );\n"
	"}\n";

// Splashes: a ring growing on the ground. aS = position relative to the camera, age.
static const char *s_SplashVS =
	WX_GLSL
	"in vec4 aS;\n"
	"out vec2 vC; out float vAge;\n"
	"void main() {\n"
	"	int k = gl_VertexID - (gl_VertexID/6)*6;\n"
	"	vC = vec2( ( k==1 || k==2 || k==4 ) ? 1.0 : -1.0, ( k==2 || k==4 || k==5 ) ? 1.0 : -1.0 );\n"
	"	vAge = aS.w;\n"
	"	float r = mix( 1.2, 6.0, sqrt( aS.w ) );\n"
	"	vec3 c = uRot*( aS.xyz + vec3( vC.x*r, 1.0, vC.y*r ) );\n"	// a little above the ground
	"	if ( c.z<WX_NEAR ) { gl_Position = WX_OUT; return; }\n"
	"	vec4 s = wxScreen( c );\n"
	"	gl_Position = wxClip( s.xy, s );\n"
	"}\n";

static const char *s_SplashFS =
	"in vec2 vC; in float vAge;\n"
	"uniform vec4 uColor;\n"
	"out vec4 oColor;\n"
	"void main() {\n"
	"	float d = length( vC );\n"
	"	float ring = smoothstep( 0.5, 0.82, d )*( 1.0 - smoothstep( 0.82, 1.0, d ) );\n"
	"	oColor = vec4( uColor.rgb, uColor.a*ring*( 1.0 - vAge ) );\n"
	"}\n";

// Sun / moon: one quad around the direction towards it, on the unit sphere around
// the camera (so it never comes nearer). vC: -1..1 across the quad.
static const char *s_SkyVS =
	WX_GLSL
	"uniform vec3 uDir; uniform float uSize;\n"
	"out vec2 vC;\n"
	"void main() {\n"
	"	int k = gl_VertexID;\n"
	"	vC = vec2( ( k==1 || k==2 || k==4 ) ? 1.0 : -1.0, ( k==2 || k==4 || k==5 ) ? 1.0 : -1.0 );\n"
	"	vec3 c = uRot*uDir;\n"
	"	vec3 a = normalize( cross( abs( c.y )<0.99 ? vec3( 0.0, 1.0, 0.0 ) : vec3( 1.0, 0.0, 0.0 ), c ) );\n"
	"	vec3 p = c + ( a*vC.x + cross( c, a )*vC.y )*uSize;\n"
	"	if ( p.z<0.02 ) { gl_Position = WX_OUT; return; }\n"
	"	vec4 s = wxScreen( p );\n"
	"	gl_Position = wxClip( s.xy, s );\n"
	"}\n";

// a disc (radius uCore of the quad), the moon's dark part (a disc moved sideways
// by how lit it is), and a glow fading out to the quad's edge; added to the sky
static const char *s_SkyFS =
	"in vec2 vC;\n"
	"uniform vec3 uColor; uniform float uAmount; uniform float uCore; uniform float uGlow; uniform float uHalo;\n"
	"uniform float uLit; uniform float uSide;\n"
	"out vec4 oColor;\n"
	"void main() {\n"
	"	float d = length( vC );\n"
	"	float disc = 1.0 - smoothstep( uCore*0.86, uCore, d );\n"
	"	if ( uLit<0.999 ) {\n"
	"		float off = length( vC - vec2( uSide*uLit*2.0*uCore, 0.0 ) );\n"
	"		disc *= mix( 0.05, 1.0, smoothstep( uCore*0.88, uCore*1.04, off ) );\n"
	"	}\n"
	"	float glow = exp( -max( d - uCore, 0.0 )/uGlow )*( 1.0 - smoothstep( 0.6, 1.0, d ) );\n"
	"	oColor = vec4( uColor*( disc + glow*uHalo )*uAmount, 1.0 );\n"
	"}\n";

struct WxUniforms { GLint Rot, Proj, ZQ, Screen, Box, Centre, Shift, Vel, Streak, Time, Px, Snow, Color; };
static WxUniforms s_uFall, s_uSplash, s_uSky;
static GLint s_uSkyDir, s_uSkySize, s_uSkyAmount, s_uSkyCore, s_uSkyGlow, s_uSkyHalo, s_uSkyLit, s_uSkySide;

static void GetUniforms( GLuint prog, WxUniforms &u )
{
	u.Rot = glGetUniformLocation( prog, "uRot" );
	u.Proj = glGetUniformLocation( prog, "uProj" );
	u.ZQ = glGetUniformLocation( prog, "uZQ" );
	u.Screen = glGetUniformLocation( prog, "uScreen" );
	u.Box = glGetUniformLocation( prog, "uBox" );
	u.Centre = glGetUniformLocation( prog, "uCentre" );
	u.Shift = glGetUniformLocation( prog, "uShift" );
	u.Vel = glGetUniformLocation( prog, "uVel" );
	u.Streak = glGetUniformLocation( prog, "uStreak" );
	u.Time = glGetUniformLocation( prog, "uTime" );
	u.Px = glGetUniformLocation( prog, "uPx" );
	u.Snow = glGetUniformLocation( prog, "uSnow" );
	u.Color = glGetUniformLocation( prog, "uColor" );
}

static BOOL Init()
{
	PtWeatherState &W = g_PtWeather;
	if ( W.bInitTried ) return W.Prog!=0;
	W.bInitTried = TRUE;
	W.Prog = PtGfx_CompileProgram( s_FallVS, s_FallFS, NULL, 0 );
	const char *attribs[] = { "aS" };
	W.SplashProg = PtGfx_CompileProgram( s_SplashVS, s_SplashFS, attribs, 1 );
	if ( !W.Prog || !W.SplashProg ) {
		PtGL_Log( "weather: shader failed, no rain or snow" );
		W.Prog = 0;
		return FALSE;
	}
	GetUniforms( W.Prog, s_uFall );
	GetUniforms( W.SplashProg, s_uSplash );
	W.SkyProg = PtGfx_CompileProgram( s_SkyVS, s_SkyFS, NULL, 0 );
	if ( W.SkyProg ) {
		GetUniforms( W.SkyProg, s_uSky );
		s_uSkyDir = glGetUniformLocation( W.SkyProg, "uDir" );
		s_uSkySize = glGetUniformLocation( W.SkyProg, "uSize" );
		s_uSkyAmount = glGetUniformLocation( W.SkyProg, "uAmount" );
		s_uSkyCore = glGetUniformLocation( W.SkyProg, "uCore" );
		s_uSkyGlow = glGetUniformLocation( W.SkyProg, "uGlow" );
		s_uSkyHalo = glGetUniformLocation( W.SkyProg, "uHalo" );
		s_uSkyLit = glGetUniformLocation( W.SkyProg, "uLit" );
		s_uSkySide = glGetUniformLocation( W.SkyProg, "uSide" );
	}
	else PtGL_Log( "weather: sun / moon shader failed, none in the sky" );
	glGenVertexArrays( 1, &W.VAO );
	glGenBuffers( 1, &W.VBO );
	PtGL_Log( "weather: ready" );
	return TRUE;
}

void PtGfx_WeatherLook( const float tint[3], const float fog[3], float density )
{
	PtWeatherState &W = g_PtWeather;
	BOOL look = density>1e-6f || fabsf( tint[0]-1 )>0.004f || fabsf( tint[1]-1 )>0.004f || fabsf( tint[2]-1 )>0.004f;
	if ( !look && !W.bLook ) return;
	for ( int i=0; i<3; i++ ) { W.Tint[i] = tint[i]; W.Fog[i] = fog[i]; }
	W.Density = density;
	W.bLook = look;
	PtGfx_InvalidateDeviceState();
}

void PtGfx_WeatherWorld( BOOL bOn )
{
	PtWeatherState &W = g_PtWeather;
	if ( W.bWorld==bOn ) return;
	W.bWorld = bOn;
	if ( W.bLook ) PtGfx_InvalidateDeviceState();
}

static void SetCamera( const WxUniforms &u, const PtWeatherScene *s, PtSurface *rt )
{
	glUniformMatrix3fv( u.Rot, 1, GL_FALSE, s->Rot );
	glUniform4f( u.Proj, s->ProjW, s->ProjH, s->CenterX, s->CenterY );
	glUniform2f( u.ZQ, s->DepthQ1, s->DepthQ2 );
	glUniform2f( u.Screen, (float)rt->Width(), (float)rt->Height() );
}

// Drops (or flakes) in a box in front of the camera: box size, how far the box
// centre is ahead of the camera, velocity, count.
static void DrawFall( const PtWeatherScene *s, PtSurface *rt, BOOL bSnow, float amount )
{
	const WxUniforms &u = s_uFall;
	const float boxXZ = bSnow ? 1000.0f : 1300.0f, boxY = bSnow ? 760.0f : 900.0f;
	const double box[3] = { boxXZ, boxY, boxXZ };
	const double fall = bSnow ? 55.0 : 760.0;
	const double vel[3] = { s->Wind[0], -fall, s->Wind[1] };
	int n = (int)( amount*( bSnow ? 15000.0f : 7000.0f ) );
	if ( n<=0 ) return;

	// box centre: ahead of the camera along the ground, a little below it
	float fx = s->Rot[2], fz = s->Rot[8];				// camera forward (third column of the rotation)
	float fl = sqrtf( fx*fx + fz*fz );
	if ( fl>1e-4f ) { fx /= fl; fz /= fl; } else { fx = fz = 0; }
	const double ahead = boxXZ*0.3;
	double centre[3] = { fx*ahead, -boxY*0.18, fz*ahead };
	// the drops' lattice moves with the wind and gravity; its offset in the box,
	// wrapped (in double: world positions and time get big)
	float shift[3];
	for ( int i=0; i<3; i++ ) {
		double v = ( vel[i]*s->Time - ( s->Cam[i] + centre[i] ) )/box[i] + 0.5;
		shift[i] = (float)( v - floor( v ) );
	}

	glUseProgram( g_PtWeather.Prog );
	SetCamera( u, s, rt );
	glUniform3f( u.Box, (float)box[0], (float)box[1], (float)box[2] );
	glUniform3f( u.Centre, (float)centre[0], (float)centre[1], (float)centre[2] );
	glUniform3f( u.Shift, shift[0], shift[1], shift[2] );
	glUniform3f( u.Vel, (float)vel[0], (float)vel[1], (float)vel[2] );
	glUniform1f( u.Streak, 0.032f );
	glUniform1f( u.Time, (float)fmod( s->Time, 10000.0 ) );
	glUniform1f( u.Px, rt->Height()/600.0f );
	glUniform1i( u.Snow, bSnow ? 1 : 0 );
	float l = s->Light;
	if ( bSnow ) glUniform4f( u.Color, 0.96f*l, 0.97f*l, 1.0f*l, 0.9f );
	else glUniform4f( u.Color, 0.74f*l, 0.79f*l, 0.86f*l, 0.55f );
	glDrawArrays( GL_TRIANGLES, 0, n*6 );
}

static void DrawSplashes( const PtWeatherScene *s, PtSurface *rt )
{
	static float *s_pVerts = NULL;
	static int s_nMax = 0;
	int n = s->nSplashes;
	if ( n<=0 || !s->Splashes ) return;
	if ( n>s_nMax ) {
		delete[] s_pVerts;
		s_nMax = n + 256;
		s_pVerts = new float[s_nMax*6*4];
	}
	float *d = s_pVerts;
	for ( int i=0; i<n; i++ )
		for ( int k=0; k<6; k++, d += 4 ) memcpy( d, s->Splashes + i*4, 16 );

	const WxUniforms &u = s_uSplash;
	glUseProgram( g_PtWeather.SplashProg );
	SetCamera( u, s, rt );
	float l = s->Light;
	glUniform4f( u.Color, 0.8f*l, 0.84f*l, 0.9f*l, 0.5f );
	glBindBuffer( GL_ARRAY_BUFFER, g_PtWeather.VBO );
	glBufferData( GL_ARRAY_BUFFER, sizeof(float)*4*6*n, s_pVerts, GL_STREAM_DRAW );
	glEnableVertexAttribArray( 0 );
	glVertexAttribPointer( 0, 4, GL_FLOAT, GL_FALSE, 16, (void *)0 );
	glDrawArrays( GL_TRIANGLES, 0, n*6 );
	glDisableVertexAttribArray( 0 );
}

void PtGfx_WeatherDraw( const PtWeatherScene *s )
{
	PtSurface *rt = g_PtGL.pRenderTarget;
	if ( !s || !rt || ( s->Rain<=0.002f && s->Snow<=0.002f && !s->nSplashes ) ) return;
	if ( s->ProjW==0 || s->ProjH==0 || s->CenterX<=0 || s->CenterY<=0 ) return;
	if ( !Init() ) return;

	glBindVertexArray( g_PtWeather.VAO );
	PtGfx_ScissorToViewport();
	if ( s->ClipHalfH>0 && s->ClipHalfH<s->CenterY ) {			// the engine's own top/bottom clipping
		glEnable( GL_SCISSOR_TEST );
		glScissor( 0, (int)( s->CenterY - s->ClipHalfH ), rt->Width(), (int)( 2*s->ClipHalfH ) );
	}
	glDisable( GL_CULL_FACE );
	glEnable( GL_DEPTH_TEST );
	glDepthFunc( GL_LEQUAL );
	glDepthMask( GL_FALSE );
	glEnable( GL_BLEND );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	glColorMask( 1, 1, 1, 0 );

	DrawSplashes( s, rt );
	if ( s->Rain>0.002f ) DrawFall( s, rt, FALSE, s->Rain );
	if ( s->Snow>0.002f ) DrawFall( s, rt, TRUE, s->Snow );

	rt->m_bGPUNewer = TRUE;
	PtGfx_InvalidateDeviceState();
}

// size: half size of the quad (tangent of the angle); core, glow: of the quad's half size
static void DrawSkyLight( const PtSkyLight *L, float size, float core, float glow, float halo )
{
	if ( !L || L->Amount<=0.002f ) return;
	glUniform3f( s_uSkyDir, L->Dir[0], L->Dir[1], L->Dir[2] );
	glUniform1f( s_uSkySize, size );
	glUniform3f( s_uSky.Color, L->Color[0], L->Color[1], L->Color[2] );
	glUniform1f( s_uSkyAmount, L->Amount );
	glUniform1f( s_uSkyCore, core );
	glUniform1f( s_uSkyGlow, glow );
	glUniform1f( s_uSkyHalo, halo );
	glUniform1f( s_uSkyLit, L->Lit );
	glUniform1f( s_uSkySide, L->Side );
	glDrawArrays( GL_TRIANGLES, 0, 6 );
}

void PtGfx_SkyLights( const PtWeatherScene *s, const PtSkyLight *sun, const PtSkyLight *moon )
{
	PtSurface *rt = g_PtGL.pRenderTarget;
	BOOL bSun = sun && sun->Amount>0.002f, bMoon = moon && moon->Amount>0.002f;
	if ( !s || !rt || !( bSun || bMoon ) ) return;
	if ( s->ProjW==0 || s->ProjH==0 || s->CenterX<=0 || s->CenterY<=0 ) return;
	if ( !Init() || !g_PtWeather.SkyProg ) return;

	glBindVertexArray( g_PtWeather.VAO );
	PtGfx_ScissorToViewport();
	if ( s->ClipHalfH>0 && s->ClipHalfH<s->CenterY ) {
		glEnable( GL_SCISSOR_TEST );
		glScissor( 0, (int)( s->CenterY - s->ClipHalfH ), rt->Width(), (int)( 2*s->ClipHalfH ) );
	}
	glDisable( GL_CULL_FACE );
	glDisable( GL_DEPTH_TEST );						// on the sky: whatever is drawn later covers it
	glDepthMask( GL_FALSE );
	glEnable( GL_BLEND );
	glBlendFunc( GL_ONE, GL_ONE );
	glColorMask( 1, 1, 1, 0 );
	glUseProgram( g_PtWeather.SkyProg );
	SetCamera( s_uSky, s, rt );
	if ( bMoon ) DrawSkyLight( moon, 0.22f, 0.16f, 0.10f, 0.35f*moon->Lit );
	if ( bSun ) DrawSkyLight( sun, 0.6f, 0.06f, 0.07f, 0.7f );

	rt->m_bGPUNewer = TRUE;
	PtGfx_InvalidateDeviceState();
}
