// weather.h - weather for the GL renderer (weather.cpp): changing weather over the
// game's world time, rain, storms with lightning, fog, and snow in the ice fields.
#pragma once

// Kinds of weather (the page's list, web\settings.js, is in this order)
enum { PTWX_CLEAR, PTWX_CLOUDY, PTWX_RAIN, PTWX_STORM, PTWX_FOG, PTWX_SNOW, PTWX_BLIZZARD, PTWX_DUST, PTWX_KINDS };

// The page's Weather setting (read in Winmain.cpp PtWeb_PollOptions): 0 = the
// server's weather (the same for everyone online), 1 + PTWX_* = that weather always.
extern int PtWeather_Setting;

class smSTAGE3D;

// Once a frame before the world is drawn: picks the weather, sets the world's look
// (ptgfx), plays the rain and thunder. bIntro: the login screen's scene (the
// weather only shows there when the setting picks one).
void PtWeather_Frame( BOOL bIntro );
// After the world: the rain or snow falling and the splashes. ground: the loaded
// map(s), for where rain lands.
void PtWeather_Draw( BOOL bIntro, smSTAGE3D *ground0, smSTAGE3D *ground1 );
