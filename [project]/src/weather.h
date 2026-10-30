// weather.h - weather for the GL renderer (weather.cpp): changing weather over the
// game's world time, rain, storms with lightning, fog, and snow in the ice fields.
#pragma once

// the page's Weather setting (web\settings.js, read in Winmain.cpp PtWeb_PollOptions)
enum { PTWX_SET_CHANGING = 0, PTWX_SET_CLEAR, PTWX_SET_RAIN, PTWX_SET_STORM, PTWX_SET_SNOW, PTWX_SET_FOG };
extern int PtWeather_Setting;

class smSTAGE3D;

// Once a frame before the world is drawn: picks the weather, sets the world's look
// (ptgfx), plays the rain and thunder. bIntro: the login screen's scene (the
// weather only shows there when the setting picks one).
void PtWeather_Frame( BOOL bIntro );
// After the world: the rain or snow falling and the splashes. ground: the loaded
// map(s), for where rain lands.
void PtWeather_Draw( BOOL bIntro, smSTAGE3D *ground0, smSTAGE3D *ground1 );
