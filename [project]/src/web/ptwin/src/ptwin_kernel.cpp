// ptwin_kernel.cpp - kernel32 for the web build: threads (pthreads), critical
// sections, waiting, time, .ini files, memory, modules and process info.
#include "ptwin_internal.h"
#include <emscripten.h>
#include <emscripten/threading.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>
#include <strings.h>

std::string PtFs_Resolve( const char *szPath );

void PtWin_Log( const char *szFormat, ... )
{
	char buf[1024];
	va_list args;
	va_start( args, szFormat );
	vsnprintf( buf, sizeof(buf), szFormat, args );
	va_end( args );
	printf( "[ptwin] %s\n", buf );
}

// The AddressSanitizer test build (web\build_web.py --asan): report every bad memory
// access (into the page's log, with its call stack) and keep playing; each place once.
#if defined(__has_feature)
#if __has_feature(address_sanitizer)
extern "C" const char *__asan_default_options()
{
	// abort_on_error: an error ASan cannot continue from stops with abort() (a browser error:
	// the game stops, the page stays usable and progress.js prints the details) - its exit()
	// left the page frozen, the report out of reach
	return "halt_on_error=0:detect_leaks=0:suppress_equal_pcs=1:malloc_context_size=16:print_summary=1:"
		"fast_unwind_on_fatal=0:fast_unwind_on_malloc=0:symbolize=1:abort_on_error=1";
}
// ASan's own stack walk does not get past the first function here: before each report,
// print the browser's call stack, which has every function's name
static char s_AsanStack[16384];
extern "C" void __asan_on_error()
{
	emscripten_get_callstack( EM_LOG_C_STACK | EM_LOG_FUNC_PARAMS, s_AsanStack, sizeof(s_AsanStack) );
	printf( "[asan] where (newest first):\n%s\n", s_AsanStack );
}

// Each report also goes to the web server (web\site\crash-report.ashx) the moment it is
// written: an error on one of the game's threads can freeze the page right after, with the
// report unreachable on screen. A synchronous request, which works on any thread.
EM_JS( void, PtWeb_SendCrashReport, ( const char *kind, const char *text ), {
	try {
		var x = new XMLHttpRequest();
		x.open( 'POST', self.location.origin + '/crash-report.ashx?kind=' + encodeURIComponent( UTF8ToString( kind ) ), false );
		x.setRequestHeader( 'Content-Type', 'text/plain;charset=utf-8' );
		x.send( UTF8ToString( text ) );
	} catch ( e ) {}
} );
#include <sanitizer/asan_interface.h>
static void OnAsanReport( const char *report )
{
	std::string all = std::string( "AddressSanitizer report, game thread " ) +
		( emscripten_is_main_runtime_thread() ? "main" : "other" ) + "\n\n" + ( report ? report : "" ) +
		"\n[asan] where (newest first):\n" + s_AsanStack + "\n";
	PtWeb_SendCrashReport( "asan", all.c_str() );
}
static struct PtAsanInit { PtAsanInit() { __asan_set_error_report_callback( OnAsanReport ); } } s_PtAsanInit;
#endif
#endif

// The trail: what the game did last (map and sky changes, models, file waits), kept by the
// page (web\progress.js Module.ptCrumb, the last 40) and printed with a crash. Held in the
// page, not in the game's memory, so a crash that wrecked that memory still shows it.
extern "C" void PtWeb_Crumb( const char *szFormat, ... )
{
	char buf[256];
	va_list args;
	va_start( args, szFormat );
	vsnprintf( buf, sizeof(buf), szFormat, args );
	va_end( args );
	if ( emscripten_is_main_runtime_thread() )
		EM_ASM( { if ( Module['ptCrumb'] ) Module['ptCrumb']( UTF8ToString( $0 ) ); }, buf );
	else			// another thread: to the page without waiting for it; the copy is freed there
		MAIN_THREAD_ASYNC_EM_ASM( { var s = UTF8ToString( $0 ); _free( $0 ); if ( Module['ptCrumb'] ) Module['ptCrumb']( '(thread) ' + s ); }, strdup( buf ) );
}

//////////////////////////////////////////////////////////////////////////////
// Errors
//////////////////////////////////////////////////////////////////////////////
static thread_local DWORD t_LastError = 0;
DWORD WINAPI GetLastError() { return t_LastError; }
void WINAPI SetLastError( DWORD e ) { t_LastError = e; }

void WINAPI OutputDebugStringA( LPCSTR s ) { if ( s ) printf( "[debug] %s", s ); }
LPTOP_LEVEL_EXCEPTION_FILTER WINAPI SetUnhandledExceptionFilter( LPTOP_LEVEL_EXCEPTION_FILTER ) { return NULL; }

//////////////////////////////////////////////////////////////////////////////
// Time
//////////////////////////////////////////////////////////////////////////////
DWORD WINAPI GetTickCount() { return (DWORD)(ULONGLONG)emscripten_get_now(); }
ULONGLONG WINAPI GetTickCount64() { return (ULONGLONG)emscripten_get_now(); }
DWORD WINAPI timeGetTime() { return GetTickCount(); }

BOOL WINAPI QueryPerformanceFrequency( LARGE_INTEGER *p ) { p->QuadPart = 1000000; return TRUE; }
BOOL WINAPI QueryPerformanceCounter( LARGE_INTEGER *p ) { p->QuadPart = (LONGLONG)( emscripten_get_now()*1000.0 ); return TRUE; }

void WINAPI GetLocalTime( LPSYSTEMTIME st )
{
	struct timespec ts;
	clock_gettime( CLOCK_REALTIME, &ts );
	time_t t = ts.tv_sec;
	struct tm lt;
	localtime_r( &t, &lt );
	st->wYear = (WORD)( lt.tm_year+1900 ); st->wMonth = (WORD)( lt.tm_mon+1 ); st->wDayOfWeek = (WORD)lt.tm_wday;
	st->wDay = (WORD)lt.tm_mday; st->wHour = (WORD)lt.tm_hour; st->wMinute = (WORD)lt.tm_min;
	st->wSecond = (WORD)lt.tm_sec; st->wMilliseconds = (WORD)( ts.tv_nsec/1000000 );
}
void WINAPI GetSystemTime( LPSYSTEMTIME st ) { GetLocalTime( st ); }

LONG WINAPI CompareFileTime( const FILETIME *a, const FILETIME *b )
{
	ULONGLONG x = ( (ULONGLONG)a->dwHighDateTime<<32 ) | a->dwLowDateTime;
	ULONGLONG y = ( (ULONGLONG)b->dwHighDateTime<<32 ) | b->dwLowDateTime;
	return x<y ? -1 : x>y ? 1 : 0;
}

// Sleep: the main thread must give the browser its turn (Asyncify); other threads block.
void WINAPI Sleep( DWORD ms )
{
	if ( emscripten_is_main_runtime_thread() ) { PtWin_CheckStall( "Sleep" ); emscripten_sleep( ms ); }
	else usleep( ms*1000 );
}

// The main thread waits without the game finishing frames: print where (once per stall)
static double s_LastFrame = 0;
static bool s_StallReported = false;
void PtWin_FrameDone() { s_LastFrame = emscripten_get_now(); s_StallReported = false; }
void PtWin_CheckStall( const char *szWhere )
{
	if ( !s_LastFrame || s_StallReported || emscripten_get_now() - s_LastFrame < 5000 ) return;
	s_StallReported = true;
	static char stack[8192];
	emscripten_get_callstack( EM_LOG_C_STACK, stack, sizeof(stack) );
	PtWin_Log( "main thread waits in %s for %.0f s without a frame:\n%s", szWhere, ( emscripten_get_now() - s_LastFrame )/1000, stack );
}
DWORD WINAPI SleepEx( DWORD ms, BOOL ) { Sleep( ms ); return 0; }

void PtWin_Yield()
{
	if ( emscripten_is_main_runtime_thread() ) emscripten_sleep( 0 );
}

//////////////////////////////////////////////////////////////////////////////
// Critical sections (a recursive pthread mutex kept in the DebugInfo field)
//////////////////////////////////////////////////////////////////////////////
static pthread_mutex_t *CSMutex( LPCRITICAL_SECTION cs )
{
	if ( !cs->DebugInfo ) InitializeCriticalSection( cs );		// game sometimes enters before init
	return (pthread_mutex_t *)cs->DebugInfo;
}
void WINAPI InitializeCriticalSection( LPCRITICAL_SECTION cs )
{
	ZeroMemory( cs, sizeof(*cs) );
	pthread_mutex_t *m = new pthread_mutex_t;
	pthread_mutexattr_t a;
	pthread_mutexattr_init( &a );
	pthread_mutexattr_settype( &a, PTHREAD_MUTEX_RECURSIVE );
	pthread_mutex_init( m, &a );
	pthread_mutexattr_destroy( &a );
	cs->DebugInfo = (PRTL_CRITICAL_SECTION_DEBUG)m;
}
BOOL WINAPI InitializeCriticalSectionAndSpinCount( LPCRITICAL_SECTION cs, DWORD ) { InitializeCriticalSection( cs ); return TRUE; }
void WINAPI DeleteCriticalSection( LPCRITICAL_SECTION cs )
{
	if ( !cs->DebugInfo ) return;
	pthread_mutex_destroy( (pthread_mutex_t *)cs->DebugInfo );
	delete (pthread_mutex_t *)cs->DebugInfo;
	cs->DebugInfo = NULL;
}
void WINAPI EnterCriticalSection( LPCRITICAL_SECTION cs )
{
	pthread_mutex_t *m = CSMutex( cs );
	if ( !emscripten_is_main_runtime_thread() ) { pthread_mutex_lock( m ); return; }
	// main thread: never block the browser - let it run until the owner leaves
	while ( pthread_mutex_trylock( m )!=0 ) { PtWin_CheckStall( "EnterCriticalSection" ); emscripten_sleep( 1 ); }
}
void WINAPI LeaveCriticalSection( LPCRITICAL_SECTION cs ) { pthread_mutex_unlock( CSMutex( cs ) ); }
BOOL WINAPI TryEnterCriticalSection( LPCRITICAL_SECTION cs ) { return pthread_mutex_trylock( CSMutex( cs ) )==0; }

//////////////////////////////////////////////////////////////////////////////
// Threads
//////////////////////////////////////////////////////////////////////////////
struct PtThread : PtObject {
	pthread_t				th;
	LPTHREAD_START_ROUTINE	proc;
	LPVOID					arg;
	DWORD					exitCode;
	BOOL					done;
	int						refs;			// handle + running thread
	int						suspend;		// SuspendThread count (the thread waits while > 0)
	pthread_mutex_t			lock;
	pthread_cond_t			cond;
	PtThread() { magic = PTOBJ_MAGIC; type = PTOBJ_THREAD; exitCode = STILL_ACTIVE; done = FALSE; refs = 2; suspend = 0;
		pthread_mutex_init( &lock, NULL ); pthread_cond_init( &cond, NULL ); }
	~PtThread() { magic = 0; pthread_mutex_destroy( &lock ); pthread_cond_destroy( &cond ); }
};

static thread_local PtThread *t_Self = NULL;

static void ThreadUnref( PtThread *t )
{
	pthread_mutex_lock( &t->lock );
	int n = --t->refs;
	pthread_mutex_unlock( &t->lock );
	if ( n==0 ) delete t;
}

static void ThreadFinish( PtThread *t, DWORD code )
{
	pthread_mutex_lock( &t->lock );
	t->exitCode = code;
	t->done = TRUE;
	pthread_cond_broadcast( &t->cond );
	pthread_mutex_unlock( &t->lock );
	ThreadUnref( t );
}

extern int g_PtNetDebug;
static void *ThreadMain( void *p )
{
	PtThread *t = (PtThread *)p;
	t_Self = t;
	if ( g_PtNetDebug ) PtWin_Log( "thread %p started", (void *)t );
	pthread_mutex_lock( &t->lock );			// CREATE_SUSPENDED: wait for ResumeThread
	while ( t->suspend>0 ) pthread_cond_wait( &t->cond, &t->lock );
	pthread_mutex_unlock( &t->lock );
	DWORD code = t->proc( t->arg );
	ThreadFinish( t, code );
	return NULL;
}

HANDLE WINAPI CreateThread( LPSECURITY_ATTRIBUTES, SIZE_T, LPTHREAD_START_ROUTINE proc, LPVOID arg, DWORD flags, LPDWORD pId )
{
	PtThread *t = new PtThread;
	t->suspend = ( flags & CREATE_SUSPENDED ) ? 1 : 0;
	t->proc = proc;
	t->arg = arg;
	pthread_attr_t a;
	pthread_attr_init( &a );
	pthread_attr_setstacksize( &a, 1024*1024 );
	pthread_attr_setdetachstate( &a, PTHREAD_CREATE_DETACHED );
	int err = pthread_create( &t->th, &a, ThreadMain, t );
	pthread_attr_destroy( &a );
	if ( err ) {
		PtWin_Log( "CreateThread failed (%d) - raise PTHREAD_POOL_SIZE?", err );
		delete t;
		return NULL;
	}
	if ( pId ) *pId = (DWORD)(uintptr_t)t;
	if ( g_PtNetDebug ) PtWin_Log( "CreateThread %p", (void *)t );
	static int s_Count = 0;					// every thread is a Web Worker: keep an eye on how many
	if ( ++s_Count > 24 ) {
		PtWin_Log( "CreateThread: thread #%d", s_Count );
		if ( s_Count <= 28 ) emscripten_log( EM_LOG_CONSOLE | EM_LOG_C_STACK, "CreateThread #%d from", s_Count );
	}
	return (HANDLE)t;
}

void PtWin_ReleaseThread( PtObject *p ) { ThreadUnref( (PtThread *)p ); }

void WINAPI ExitThread( DWORD code )
{
	if ( t_Self ) ThreadFinish( t_Self, code );
	pthread_exit( NULL );
}

BOOL WINAPI TerminateThread( HANDLE h, DWORD code )
{
	// pthreads cannot be killed from outside; the game only does this at shutdown.
	PtWin_Log( "TerminateThread ignored" );
	return FALSE;
}

BOOL WINAPI GetExitCodeThread( HANDLE h, LPDWORD pCode )
{
	PtThread *t = PtObj<PtThread>( h, PTOBJ_THREAD );
	if ( !t ) return FALSE;
	pthread_mutex_lock( &t->lock );
	*pCode = t->exitCode;
	pthread_mutex_unlock( &t->lock );
	return TRUE;
}

// A thread can only be stopped at a safe point here: SuspendThread works for the
// calling thread itself (the game's socket threads suspend themselves and are
// resumed when data arrives) and for threads created suspended.
static PtThread *ThreadOf( HANDLE h )
{
	if ( h==(HANDLE)(intptr_t)-2 ) return t_Self;		// GetCurrentThread()
	return PtObj<PtThread>( h, PTOBJ_THREAD );
}

DWORD WINAPI SuspendThread( HANDLE h )
{
	PtThread *t = ThreadOf( h );
	if ( !t ) return (DWORD)-1;
	pthread_mutex_lock( &t->lock );
	DWORD prev = (DWORD)t->suspend++;
	if ( t==t_Self ) while ( t->suspend>0 ) pthread_cond_wait( &t->cond, &t->lock );
	else if ( prev==0 ) t->suspend--;		// cannot stop another running thread: ignore
	pthread_mutex_unlock( &t->lock );
	return prev;
}

DWORD WINAPI ResumeThread( HANDLE h )
{
	PtThread *t = ThreadOf( h );
	if ( !t ) return (DWORD)-1;
	pthread_mutex_lock( &t->lock );
	DWORD prev = (DWORD)t->suspend;
	if ( g_PtNetDebug ) PtWin_Log( "ResumeThread %p (count %d)", (void *)t, t->suspend );
	if ( t->suspend>0 ) t->suspend--;
	pthread_cond_broadcast( &t->cond );
	pthread_mutex_unlock( &t->lock );
	return prev;
}
BOOL WINAPI SetThreadPriority( HANDLE, int ) { return TRUE; }
int WINAPI GetThreadPriority( HANDLE ) { return THREAD_PRIORITY_NORMAL; }
HANDLE WINAPI GetCurrentThread() { return (HANDLE)(intptr_t)-2; }
DWORD WINAPI GetCurrentThreadId() { return (DWORD)(uintptr_t)pthread_self(); }

DWORD PtWin_WaitObject( PtObject *p, DWORD dwMs );

DWORD WINAPI WaitForSingleObject( HANDLE h, DWORD dwMs )
{
	PtObject *p = (PtObject *)h;
	if ( !p || h==INVALID_HANDLE_VALUE || p->magic!=PTOBJ_MAGIC ) return WAIT_FAILED;
	if ( p->type==PTOBJ_THREAD ) {
		PtThread *t = (PtThread *)p;
		double end = emscripten_get_now() + dwMs;
		for ( ;; ) {
			pthread_mutex_lock( &t->lock );
			BOOL done = t->done;
			pthread_mutex_unlock( &t->lock );
			if ( done ) return WAIT_OBJECT_0;
			if ( dwMs!=INFINITE && emscripten_get_now()>=end ) return WAIT_TIMEOUT;
			Sleep( 1 );			// main thread: lets the browser run meanwhile
		}
	}
	return PtWin_WaitObject( p, dwMs );
}

DWORD PtWin_WaitObject( PtObject *p, DWORD )
{
	return WAIT_OBJECT_0;			// events/mutexes: not used by the game so far
}

//////////////////////////////////////////////////////////////////////////////
// Interlocked (also builtins in the headers; these are for code taking their address)
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// Memory
//////////////////////////////////////////////////////////////////////////////
HGLOBAL WINAPI GlobalAlloc( UINT flags, SIZE_T n ) { void *p = malloc( n ? n : 1 ); if ( p && ( flags & GMEM_ZEROINIT ) ) memset( p, 0, n ); return (HGLOBAL)p; }
HGLOBAL WINAPI GlobalFree( HGLOBAL h ) { free( h ); return NULL; }
LPVOID WINAPI GlobalLock( HGLOBAL h ) { return h; }
BOOL WINAPI GlobalUnlock( HGLOBAL ) { return TRUE; }
HLOCAL WINAPI LocalAlloc( UINT flags, SIZE_T n ) { void *p = malloc( n ? n : 1 ); if ( p && ( flags & LMEM_ZEROINIT ) ) memset( p, 0, n ); return (HLOCAL)p; }
HLOCAL WINAPI LocalFree( HLOCAL h ) { free( h ); return NULL; }

//////////////////////////////////////////////////////////////////////////////
// Modules and processes
//////////////////////////////////////////////////////////////////////////////
DWORD WINAPI GetModuleFileNameA( HMODULE, LPSTR buf, DWORD n )
{
	char cwd[512];
	if ( !getcwd( cwd, sizeof(cwd) ) ) strcpy( cwd, "/" );
	std::string p = std::string( "C:" ) + cwd + ( cwd[strlen( cwd )-1]=='/' ? "" : "/" ) + "game.exe";
	for ( auto &c : p ) if ( c=='/' ) c = '\\';
	lstrcpynA( buf, p.c_str(), n );
	return (DWORD)strlen( buf );
}
DWORD WINAPI GetModuleFileNameW( HMODULE h, LPWSTR buf, DWORD n )
{
	char a[MAX_PATH];
	GetModuleFileNameA( h, a, MAX_PATH );
	return (DWORD)MultiByteToWideChar( CP_ACP, 0, a, -1, buf, n );
}
HMODULE WINAPI GetModuleHandleA( LPCSTR name ) { return name ? NULL : (HMODULE)0x400000; }
HMODULE WINAPI LoadLibraryA( LPCSTR name ) { PtWin_Log( "LoadLibrary(%s): no DLLs in the browser", name ); return NULL; }
HMODULE WINAPI LoadLibraryW( LPCWSTR ) { return NULL; }
BOOL WINAPI FreeLibrary( HMODULE ) { return TRUE; }
FARPROC WINAPI GetProcAddress( HMODULE, LPCSTR ) { return NULL; }

HANDLE WINAPI GetCurrentProcess() { return (HANDLE)(intptr_t)-1; }
DWORD WINAPI GetCurrentProcessId() { return 1; }
BOOL WINAPI TerminateProcess( HANDLE, UINT code ) { PtWin_Log( "TerminateProcess(%u)", code ); emscripten_force_exit( (int)code ); return TRUE; }
BOOL WINAPI GetExitCodeProcess( HANDLE, LPDWORD p ) { *p = 0; return TRUE; }
BOOL WINAPI CreateProcessA( LPCSTR, LPSTR cmd, LPSECURITY_ATTRIBUTES, LPSECURITY_ATTRIBUTES, BOOL, DWORD, LPVOID, LPCSTR, LPSTARTUPINFOA, LPPROCESS_INFORMATION )
{
	PtWin_Log( "CreateProcess(%s): not in the browser", cmd ? cmd : "" );
	return FALSE;
}
HANDLE WINAPI CreateJobObjectA( LPSECURITY_ATTRIBUTES, LPCSTR ) { return NULL; }
BOOL WINAPI AssignProcessToJobObject( HANDLE, HANDLE ) { return FALSE; }
BOOL WINAPI SetInformationJobObject( HANDLE, JOBOBJECTINFOCLASS, LPVOID, DWORD ) { return FALSE; }

BOOL WINAPI GetComputerNameA( LPSTR buf, LPDWORD n ) { lstrcpynA( buf, "BROWSER", *n ); *n = 7; return TRUE; }
UINT WINAPI GetSystemDirectoryA( LPSTR buf, UINT n ) { lstrcpynA( buf, "C:\\Windows\\System32", n ); return (UINT)strlen( buf ); }
UINT WINAPI GetWindowsDirectoryA( LPSTR buf, UINT n ) { lstrcpynA( buf, "C:\\Windows", n ); return (UINT)strlen( buf ); }
DWORD WINAPI GetCurrentDirectoryA( DWORD n, LPSTR buf ) { char cwd[512]; if ( !getcwd( cwd, sizeof(cwd) ) ) strcpy( cwd, "/" ); lstrcpynA( buf, cwd, n ); return (DWORD)strlen( buf ); }
BOOL WINAPI SetCurrentDirectoryA( LPCSTR p ) { return chdir( PtFs_Resolve( p ).c_str() )==0; }

//////////////////////////////////////////////////////////////////////////////
// .ini files (GetPrivateProfileString & co.)
//////////////////////////////////////////////////////////////////////////////
static std::string Trim( const std::string &s )
{
	size_t a = s.find_first_not_of( " \t\r\n" ), b = s.find_last_not_of( " \t\r\n" );
	return a==std::string::npos ? "" : s.substr( a, b-a+1 );
}

static std::vector<std::string> ReadLines( LPCSTR file )
{
	std::vector<std::string> lines;
	FILE *fp = fopen( PtFs_Resolve( file ).c_str(), "rb" );
	if ( !fp ) return lines;
	char buf[1024];
	while ( fgets( buf, sizeof(buf), fp ) ) {
		std::string l = buf;
		while ( !l.empty() && ( l.back()=='\n' || l.back()=='\r' ) ) l.pop_back();
		lines.push_back( l );
	}
	fclose( fp );
	return lines;
}

DWORD WINAPI GetPrivateProfileStringA( LPCSTR app, LPCSTR key, LPCSTR def, LPSTR out, DWORD n, LPCSTR file )
{
	std::vector<std::string> lines = ReadLines( file );
	std::string section, result = def ? def : "";
	bool found = false;
	for ( auto &l : lines ) {
		std::string t = Trim( l );
		if ( t.empty() || t[0]==';' ) continue;
		if ( t[0]=='[' ) { section = Trim( t.substr( 1, t.find( ']' )-1 ) ); continue; }
		size_t eq = t.find( '=' );
		if ( eq==std::string::npos || !app || strcasecmp( section.c_str(), app ) ) continue;
		if ( key && !strcasecmp( Trim( t.substr( 0, eq ) ).c_str(), key ) ) { result = Trim( t.substr( eq+1 ) ); found = true; break; }
	}
	(void)found;
	lstrcpynA( out, result.c_str(), n );
	return (DWORD)strlen( out );
}

UINT WINAPI GetPrivateProfileIntA( LPCSTR app, LPCSTR key, INT def, LPCSTR file )
{
	char buf[64];
	char d[32];
	sprintf( d, "%d", def );
	GetPrivateProfileStringA( app, key, d, buf, sizeof(buf), file );
	return (UINT)atoi( buf );
}

BOOL WINAPI WritePrivateProfileStringA( LPCSTR app, LPCSTR key, LPCSTR value, LPCSTR file )
{
	std::vector<std::string> lines = ReadLines( file );
	std::string section;
	int sectionEnd = -1;
	bool done = false;
	for ( size_t i=0; i<lines.size() && !done; i++ ) {
		std::string t = Trim( lines[i] );
		if ( !t.empty() && t[0]=='[' ) {
			if ( sectionEnd>=0 ) break;
			section = Trim( t.substr( 1, t.find( ']' )-1 ) );
			if ( !strcasecmp( section.c_str(), app ) ) sectionEnd = (int)i+1;
			continue;
		}
		if ( sectionEnd<0 ) continue;
		sectionEnd = (int)i+1;
		size_t eq = t.find( '=' );
		if ( eq!=std::string::npos && key && !strcasecmp( Trim( t.substr( 0, eq ) ).c_str(), key ) ) {
			if ( value ) lines[i] = std::string( key ) + "=" + value; else lines.erase( lines.begin()+i );
			done = true;
		}
	}
	if ( !done && key && value ) {
		if ( sectionEnd<0 ) { lines.push_back( std::string( "[" ) + app + "]" ); sectionEnd = (int)lines.size(); }
		lines.insert( lines.begin()+sectionEnd, std::string( key ) + "=" + value );
	}
	FILE *fp = fopen( PtFs_Resolve( file ).c_str(), "wb" );
	if ( !fp ) return FALSE;
	for ( auto &l : lines ) fprintf( fp, "%s\r\n", l.c_str() );
	fclose( fp );
	return TRUE;
}
