// ptplatform.cpp - SDL2 platform layer, see ptplatform.h.

#ifdef PT_USE_SDL

#include <windows.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>

#define SDL_MAIN_HANDLED
#include <SDL.h>
#ifdef PT_WEB
#include <emscripten.h>
void PtWin_SetKeyDown( int vk, BOOL bDown );		// ptwin: GetKeyState
#else
#include <SDL_syswm.h>
#endif

#include "ptplatform.h"

static SDL_Window	*s_pWindow = NULL;
static HWND			s_hWnd = NULL;
static WNDPROC		s_pfnGame = NULL;
static BOOL			s_bSynth = FALSE;		// TRUE while we call WndProc with a translated event

static void Log( const char *szFormat, ... )
{
	FILE *fp = fopen( "ptplatform.log", "at" );
	if ( !fp ) return;
	va_list args;
	va_start( args, szFormat );
	vfprintf( fp, szFormat, args );
	va_end( args );
	fputc( '\n', fp );
	fclose( fp );
}

BOOL PtPlat_WantSDL()
{
#ifdef PT_WEB
	return TRUE;					// the browser has nothing else
#endif
	static int s_Want = -1;
	if ( s_Want>=0 ) return s_Want;
	s_Want = 0;
	FILE *fp = fopen( "hotuk.ini", "rt" );
	if ( !fp ) return FALSE;
	char line[256];
	while ( fgets( line, sizeof(line), fp ) ) {
		char *p = line;
		while ( *p==' ' || *p=='\t' ) p++;
		if ( _strnicmp( p, "*PLATFORM", 9 ) ) continue;
		p += 9;
		while ( *p==' ' || *p=='\t' ) p++;
		s_Want = _strnicmp( p, "SDL", 3 )==0;
	}
	fclose( fp );
	return s_Want;
}

BOOL PtPlat_IsSDL() { return s_pWindow!=NULL; }
SDL_Window *PtPlat_GetSDLWindow() { return s_pWindow; }

//////////////////////////////////////////////////////////////////////////////
// Win32 side: keep raw input away from the game (SDL turns it into events)
//////////////////////////////////////////////////////////////////////////////
static BOOL IsRawInput( UINT msg )
{
	switch ( msg ) {
	case WM_KEYDOWN: case WM_KEYUP: case WM_SYSKEYDOWN: case WM_SYSKEYUP:
	case WM_CHAR: case WM_SYSCHAR: case WM_DEADCHAR: case WM_SYSDEADCHAR:
	case WM_MOUSEMOVE: case WM_MOUSEWHEEL:
	case WM_LBUTTONDOWN: case WM_LBUTTONUP: case WM_LBUTTONDBLCLK:
	case WM_RBUTTONDOWN: case WM_RBUTTONUP: case WM_RBUTTONDBLCLK:
	case WM_MBUTTONDOWN: case WM_MBUTTONUP: case WM_MBUTTONDBLCLK:
		return TRUE;
	}
	return FALSE;
}

// Window proc chain: SDL's proc -> FilterProc -> game WndProc.
static LRESULT CALLBACK FilterProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	if ( !s_bSynth && IsRawInput( msg ) ) return 0;
	return CallWindowProcA( s_pfnGame, hWnd, msg, wParam, lParam );
}

static LRESULT SendToGame( UINT msg, WPARAM wParam, LPARAM lParam )
{
#ifdef PT_WEB
	// keyboard goes to the focused window (the game's hidden EDIT box while typing)
	HWND hTarget = s_hWnd;
	if ( msg>=WM_KEYFIRST && msg<=WM_KEYLAST && GetFocus() ) hTarget = GetFocus();
	// set Module.ptDebugInput = 1 in the browser console to trace input
	if ( msg!=WM_MOUSEMOVE && EM_ASM_INT( { return Module['ptDebugInput'] ? 1 : 0; } ) ) {
		char cls[32] = "";
		GetClassNameA( hTarget, cls, sizeof(cls) );
		printf( "[input] msg %04X w %X l %lX -> %s%s\n", msg, (unsigned)wParam, (unsigned long)lParam, cls, hTarget==s_hWnd ? " (main)" : "" );
	}
	return SendMessageA( hTarget, msg, wParam, lParam );
#endif
	s_bSynth = TRUE;
	LRESULT r = CallWindowProcA( s_pfnGame, s_hWnd, msg, wParam, lParam );
	s_bSynth = FALSE;
	return r;
}

#ifdef PT_WEB
// Browser: the "game window" is the page's canvas, created here at the window's size.
BOOL PtPlat_Attach( HWND hWnd, WNDPROC pfnGameProc )
{
	if ( s_pWindow ) return FALSE;
	SDL_SetMainReady();
	if ( SDL_InitSubSystem( SDL_INIT_VIDEO | SDL_INIT_EVENTS )!=0 ) { printf( "[ptplatform] SDL init failed: %s\n", SDL_GetError() ); return FALSE; }
	SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 3 );
	SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 0 );
	SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES );
	SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
	SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 24 );
	SDL_GL_SetAttribute( SDL_GL_STENCIL_SIZE, 8 );
	RECT r;
	GetClientRect( hWnd, &r );
	s_hWnd = hWnd;
	s_pfnGame = pfnGameProc;
	s_pWindow = SDL_CreateWindow( "Priston Tale", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, r.right, r.bottom, SDL_WINDOW_OPENGL );
	if ( !s_pWindow ) { printf( "[ptplatform] SDL_CreateWindow failed: %s\n", SDL_GetError() ); return FALSE; }
	printf( "[ptplatform] canvas %dx%d\n", (int)r.right, (int)r.bottom );
	return TRUE;
}
#else
BOOL PtPlat_Attach( HWND hWnd, WNDPROC pfnGameProc )
{
	if ( s_pWindow || !PtPlat_WantSDL() ) return FALSE;
	SDL_SetMainReady();
	if ( SDL_InitSubSystem( SDL_INIT_VIDEO | SDL_INIT_EVENTS )!=0 ) {
		Log( "SDL video init failed: %s - using Win32", SDL_GetError() );
		return FALSE;
	}
	// OpenGL 3.3 core on the game's own window (pixel format is chosen in SDL_CreateWindowFrom)
	SDL_SetHint( SDL_HINT_VIDEO_FOREIGN_WINDOW_OPENGL, "1" );
	SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 3 );
	SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 3 );
	SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE );
	SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
	SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 24 );
	SDL_GL_SetAttribute( SDL_GL_STENCIL_SIZE, 8 );

	s_hWnd = hWnd;
	s_pfnGame = pfnGameProc;
	SetWindowLongPtrA( hWnd, GWLP_WNDPROC, (LONG_PTR)FilterProc );	// SDL keeps this as the "original" proc
	s_pWindow = SDL_CreateWindowFrom( hWnd );
	if ( !s_pWindow ) {
		Log( "SDL_CreateWindowFrom failed: %s - using Win32", SDL_GetError() );
		SetWindowLongPtrA( hWnd, GWLP_WNDPROC, (LONG_PTR)pfnGameProc );
		return FALSE;
	}
	SDL_version v;
	SDL_GetVersion( &v );
	Log( "SDL %d.%d.%d attached to the game window (video driver %s)", v.major, v.minor, v.patch, SDL_GetCurrentVideoDriver() );
	return TRUE;
}
#endif // PT_WEB

//////////////////////////////////////////////////////////////////////////////
// SDL events -> WM_ messages
//////////////////////////////////////////////////////////////////////////////
static int ScancodeToVK( SDL_Scancode sc )
{
	if ( sc>=SDL_SCANCODE_A && sc<=SDL_SCANCODE_Z ) return 'A' + ( sc-SDL_SCANCODE_A );
	if ( sc>=SDL_SCANCODE_1 && sc<=SDL_SCANCODE_9 ) return '1' + ( sc-SDL_SCANCODE_1 );
	if ( sc>=SDL_SCANCODE_F1 && sc<=SDL_SCANCODE_F12 ) return VK_F1 + ( sc-SDL_SCANCODE_F1 );
	if ( sc>=SDL_SCANCODE_KP_1 && sc<=SDL_SCANCODE_KP_9 ) return VK_NUMPAD1 + ( sc-SDL_SCANCODE_KP_1 );
	switch ( sc ) {
	case SDL_SCANCODE_0:			return '0';
	case SDL_SCANCODE_RETURN:		return VK_RETURN;
	case SDL_SCANCODE_KP_ENTER:		return VK_RETURN;
	case SDL_SCANCODE_ESCAPE:		return VK_ESCAPE;
	case SDL_SCANCODE_BACKSPACE:	return VK_BACK;
	case SDL_SCANCODE_TAB:			return VK_TAB;
	case SDL_SCANCODE_SPACE:		return VK_SPACE;
	case SDL_SCANCODE_MINUS:		return VK_OEM_MINUS;
	case SDL_SCANCODE_EQUALS:		return VK_OEM_PLUS;
	case SDL_SCANCODE_LEFTBRACKET:	return VK_OEM_4;
	case SDL_SCANCODE_RIGHTBRACKET:	return VK_OEM_6;
	case SDL_SCANCODE_BACKSLASH:	return VK_OEM_5;
	case SDL_SCANCODE_SEMICOLON:	return VK_OEM_1;
	case SDL_SCANCODE_APOSTROPHE:	return VK_OEM_7;
	case SDL_SCANCODE_GRAVE:		return VK_OEM_3;
	case SDL_SCANCODE_COMMA:		return VK_OEM_COMMA;
	case SDL_SCANCODE_PERIOD:		return VK_OEM_PERIOD;
	case SDL_SCANCODE_SLASH:		return VK_OEM_2;
	case SDL_SCANCODE_CAPSLOCK:		return VK_CAPITAL;
	case SDL_SCANCODE_PRINTSCREEN:	return VK_SNAPSHOT;
	case SDL_SCANCODE_SCROLLLOCK:	return VK_SCROLL;
	case SDL_SCANCODE_PAUSE:		return VK_PAUSE;
	case SDL_SCANCODE_INSERT:		return VK_INSERT;
	case SDL_SCANCODE_HOME:			return VK_HOME;
	case SDL_SCANCODE_PAGEUP:		return VK_PRIOR;
	case SDL_SCANCODE_DELETE:		return VK_DELETE;
	case SDL_SCANCODE_END:			return VK_END;
	case SDL_SCANCODE_PAGEDOWN:		return VK_NEXT;
	case SDL_SCANCODE_RIGHT:		return VK_RIGHT;
	case SDL_SCANCODE_LEFT:			return VK_LEFT;
	case SDL_SCANCODE_DOWN:			return VK_DOWN;
	case SDL_SCANCODE_UP:			return VK_UP;
	case SDL_SCANCODE_NUMLOCKCLEAR:	return VK_NUMLOCK;
	case SDL_SCANCODE_KP_DIVIDE:	return VK_DIVIDE;
	case SDL_SCANCODE_KP_MULTIPLY:	return VK_MULTIPLY;
	case SDL_SCANCODE_KP_MINUS:		return VK_SUBTRACT;
	case SDL_SCANCODE_KP_PLUS:		return VK_ADD;
	case SDL_SCANCODE_KP_0:			return VK_NUMPAD0;
	case SDL_SCANCODE_KP_PERIOD:	return VK_DECIMAL;
	case SDL_SCANCODE_LCTRL:  case SDL_SCANCODE_RCTRL:	return VK_CONTROL;
	case SDL_SCANCODE_LSHIFT: case SDL_SCANCODE_RSHIFT:	return VK_SHIFT;
	case SDL_SCANCODE_LALT:   case SDL_SCANCODE_RALT:	return VK_MENU;
	case SDL_SCANCODE_LGUI:		return VK_LWIN;
	case SDL_SCANCODE_RGUI:		return VK_RWIN;
	case SDL_SCANCODE_APPLICATION:	return VK_APPS;
	}
	return 0;
}

static BOOL IsExtendedKey( SDL_Scancode sc )
{
	switch ( sc ) {
	case SDL_SCANCODE_INSERT: case SDL_SCANCODE_DELETE: case SDL_SCANCODE_HOME: case SDL_SCANCODE_END:
	case SDL_SCANCODE_PAGEUP: case SDL_SCANCODE_PAGEDOWN:
	case SDL_SCANCODE_LEFT: case SDL_SCANCODE_RIGHT: case SDL_SCANCODE_UP: case SDL_SCANCODE_DOWN:
	case SDL_SCANCODE_RCTRL: case SDL_SCANCODE_RALT: case SDL_SCANCODE_KP_ENTER: case SDL_SCANCODE_KP_DIVIDE:
		return TRUE;
	}
	return FALSE;
}

static void KeyEvent( const SDL_KeyboardEvent &k )
{
	int vk = ScancodeToVK( k.keysym.scancode );
	if ( !vk ) return;
	BOOL bDown = k.type==SDL_KEYDOWN;
	UINT scan = MapVirtualKeyA( vk, MAPVK_VK_TO_VSC );
	LPARAM lp = 1 | ( (LPARAM)( scan & 0xFF )<<16 );
	if ( IsExtendedKey( k.keysym.scancode ) ) lp |= 1<<24;
	if ( !bDown || k.repeat ) lp |= 1<<30;
	if ( !bDown ) lp |= (LPARAM)1<<31;

#ifdef PT_WEB
	PtWin_SetKeyDown( vk, bDown );
#endif
	BOOL bAlt = ( k.keysym.mod & KMOD_ALT )!=0 || vk==VK_MENU || vk==VK_F10;
	UINT msg = bAlt ? ( bDown ? WM_SYSKEYDOWN : WM_SYSKEYUP ) : ( bDown ? WM_KEYDOWN : WM_KEYUP );
	if ( bAlt ) {
		// the original message loop drops system keys except these (Winmain.cpp)
		if ( vk!=0xF4 && vk!=VK_F10 ) return;
	}
	if ( bAlt ) lp |= 1<<29;
	SendToGame( msg, vk, lp );

	// WM_CHAR for keys SDL text input does not report (Win32 TranslateMessage does)
	if ( bDown && !bAlt ) {
		int ch = 0;
		if ( vk==VK_RETURN ) ch = 13;
		else if ( vk==VK_BACK ) ch = 8;
		else if ( vk==VK_TAB ) ch = 9;
		else if ( vk==VK_ESCAPE ) ch = 27;
		else if ( ( k.keysym.mod & KMOD_CTRL ) && vk>='A' && vk<='Z' ) ch = vk-'A'+1;
		if ( ch ) SendToGame( WM_CHAR, ch, lp );
	}
}

static void TextEvent( const SDL_TextInputEvent &t )
{
	WCHAR w[32];
	int nw = MultiByteToWideChar( CP_UTF8, 0, t.text, -1, w, 32 );
	if ( nw<=1 ) return;
	char mb[64];
	int nb = WideCharToMultiByte( CP_ACP, 0, w, nw-1, mb, sizeof(mb), NULL, NULL );
	for ( int i=0; i<nb; i++ ) SendToGame( WM_CHAR, (BYTE)mb[i], 1 );
}

static WPARAM MouseKeys()
{
	Uint32 b = SDL_GetMouseState( NULL, NULL );
	SDL_Keymod mod = SDL_GetModState();
	WPARAM w = 0;
	if ( b & SDL_BUTTON_LMASK ) w |= MK_LBUTTON;
	if ( b & SDL_BUTTON_RMASK ) w |= MK_RBUTTON;
	if ( b & SDL_BUTTON_MMASK ) w |= MK_MBUTTON;
	if ( mod & KMOD_SHIFT ) w |= MK_SHIFT;
	if ( mod & KMOD_CTRL ) w |= MK_CONTROL;
	return w;
}

static void ButtonEvent( const SDL_MouseButtonEvent &b )
{
	BOOL bDown = b.type==SDL_MOUSEBUTTONDOWN;
	UINT msg;
	switch ( b.button ) {
	case SDL_BUTTON_LEFT:	msg = bDown ? ( b.clicks==2 ? WM_LBUTTONDBLCLK : WM_LBUTTONDOWN ) : WM_LBUTTONUP; break;
	case SDL_BUTTON_RIGHT:	msg = bDown ? ( b.clicks==2 ? WM_RBUTTONDBLCLK : WM_RBUTTONDOWN ) : WM_RBUTTONUP; break;
	case SDL_BUTTON_MIDDLE:	msg = bDown ? ( b.clicks==2 ? WM_MBUTTONDBLCLK : WM_MBUTTONDOWN ) : WM_MBUTTONUP; break;
	default: return;
	}
	SendToGame( msg, MouseKeys(), MAKELPARAM( b.x, b.y ) );
}

static void WheelEvent( const SDL_MouseWheelEvent &w )
{
	int dy = w.direction==SDL_MOUSEWHEEL_FLIPPED ? -w.y : w.y;
	if ( !dy ) return;
	int x, y;
	SDL_GetMouseState( &x, &y );
	POINT pt = { x, y };
	ClientToScreen( s_hWnd, &pt );		// WM_MOUSEWHEEL carries screen coordinates
	SendToGame( WM_MOUSEWHEEL, MAKEWPARAM( MouseKeys(), (short)( dy*WHEEL_DELTA ) ), MAKELPARAM( pt.x, pt.y ) );
}

#ifdef PT_WEB
// Pasting into the chat (Ctrl+V). The browser hands us the clipboard text; it goes to the
// game as typed characters, but only while a text box (the chat) has the focus.
extern HWND hFocusWnd;			// Winmain.cpp: the game's focused text box
static unsigned short s_Paste[512];
static int s_PasteLen = 0;

extern "C" EMSCRIPTEN_KEEPALIVE void PtWeb_PasteChar( int ch )
{
	if ( s_PasteLen<(int)( sizeof(s_Paste)/sizeof(s_Paste[0]) ) ) s_Paste[s_PasteLen++] = (unsigned short)ch;
}

static void PasteHook()
{
	static BOOL bHooked = FALSE;
	if ( bHooked ) return;
	bHooked = TRUE;
	EM_ASM( {
		var send = function( t ) {
			if ( !t ) return;
			t = String( t );
			for ( var i = 0; i < t.length && i < 200; i++ ) {
				var c = t.charCodeAt( i );
				Module['_PtWeb_PasteChar']( c < 32 ? 32 : c );		/* line breaks and tabs: spaces */
			}
		};
		var mine = function( e ) { var n = e.target && e.target.tagName; return n != 'INPUT' && n != 'TEXTAREA'; };
		var got = 0;
		document.addEventListener( 'paste', function( e ) {
			if ( !mine( e ) ) return;
			var t = e.clipboardData && e.clipboardData.getData( 'text' );
			if ( t ) { got = Date.now(); send( t ); e.preventDefault(); }
		}, true );
		// SDL may swallow the key, so no paste event comes: read the clipboard ourselves
		document.addEventListener( 'keydown', function( e ) {
			if ( !( e.ctrlKey || e.metaKey ) || ( e.key != 'v' && e.key != 'V' ) || !mine( e ) ) return;
			var t0 = Date.now();
			setTimeout( function() {
				if ( got >= t0 || !navigator.clipboard || !navigator.clipboard.readText ) return;
				navigator.clipboard.readText().then( function( t ) { if ( got < t0 ) { got = Date.now(); send( t ); } } ).catch( function() {} );
			}, 80 );
		}, true );
	} );
}

static void PasteFlush()
{
	int n = s_PasteLen;
	s_PasteLen = 0;
	if ( !n || !hFocusWnd ) return;		// no chat box open: drop it
	for ( int i=0; i<n; i++ ) {
		WCHAR w = (WCHAR)s_Paste[i];
		if ( w<32 ) continue;
		char mb[8];
		int nb = WideCharToMultiByte( CP_ACP, 0, &w, 1, mb, sizeof(mb), NULL, NULL );
		for ( int k=0; k<nb; k++ ) SendToGame( WM_CHAR, (BYTE)mb[k], 1 );
	}
}
#endif

// SDL input -> window procedures (no yielding; also called from PeekMessage on the web)
void PtPlat_PumpInput()
{
	if ( !s_pWindow ) return;
#ifdef PT_WEB
	PasteHook();
	PasteFlush();
#endif
	SDL_Event e;
	while ( SDL_PollEvent( &e ) ) {		// also dispatches the Win32 message queue
#ifdef PT_WEB
		EM_ASM( { Module['ptEvents'] = ( Module['ptEvents'] | 0 ) + 1; } );		// for debugging from the console
#endif
		switch ( e.type ) {
		case SDL_KEYDOWN: case SDL_KEYUP:	KeyEvent( e.key ); break;
		case SDL_TEXTINPUT:					TextEvent( e.text ); break;
		case SDL_MOUSEMOTION:				SendToGame( WM_MOUSEMOVE, MouseKeys(), MAKELPARAM( e.motion.x, e.motion.y ) ); break;
		case SDL_MOUSEBUTTONDOWN: case SDL_MOUSEBUTTONUP:	ButtonEvent( e.button ); break;
		case SDL_MOUSEWHEEL:				WheelEvent( e.wheel ); break;
		// window close, focus, etc. reach WndProc as Win32 messages
		}
	}
}

// Once per frame from the game's main loop.
void PtPlat_PumpEvents()
{
#ifdef PT_WEB
	void PtWin_FrameDone();
	PtWin_FrameDone();
	EM_ASM( { Module['ptFrames'] = ( Module['ptFrames'] | 0 ) + 1; } );
	emscripten_sleep( 0 );			// let the browser show the frame and deliver input
#endif
	PtPlat_PumpInput();
#ifdef PT_WEB
	// SDL_PollEvent dispatches posted Win32 messages on Windows (socket events,
	// timers, PostMessage); the web build's message queue is drained here
	MSG msg;
	while ( PeekMessageA( &msg, NULL, 0, 0, PM_REMOVE ) ) {
		TranslateMessage( &msg );
		DispatchMessageA( &msg );
	}
#endif
}

#endif // PT_USE_SDL
