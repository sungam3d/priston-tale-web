// ptwin_fs.cpp - files for the web build.
//
// Windows paths ("Field\\Map01.smd", "C:\\...") are case-insensitive and use
// backslashes; the browser's file system (Emscripten) is case-sensitive with '/'.
// PtFs_Resolve maps a game path to the real file. The game's fopen/_lopen/
// CreateFileA/FindFirstFileA all come through here (see ptwin\include\ptwin_fs.h),
// which is also where game data is fetched from the web server on first use.
#define PTWIN_FS_IMPL
#include "ptwin_internal.h"
#include <shlwapi.h>
#undef fopen
#undef fclose
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>
#include <strings.h>
#include <errno.h>
#include <set>

#include <emscripten.h>
#include <emscripten/threading.h>

// Game data lives on the web server; the browser file system starts empty and
// files are fetched on first use. The manifest (all data files with sizes) is
// written to /game_manifest.txt by the page before the game starts (web\pre.js).
#define PT_GAME_ROOT	"/game"

static pthread_mutex_t s_DirLock = PTHREAD_MUTEX_INITIALIZER;
static std::map<std::string, std::map<std::string, std::string>> s_DirCache;	// abs dir -> lower name -> real name
static std::map<std::string, std::map<std::string, std::string>> s_ManDirs;	// rel dir (real case) -> lower name -> real name
static std::map<std::string, long> s_ManFiles;									// rel path (real case) -> size
static bool s_ManLoaded = false;

static std::string Lower( std::string s ) { for ( auto &c : s ) c = (char)tolower( (unsigned char)c ); return s; }

// Saves: files the game writes (characters, accounts, mail, castle...) are copied
// to the browser's IndexedDB by web\pre.js and put back before the next start;
// deleted ones are taken out of the manifest so they are not fetched again.
// s_Local: files that differ from the web server's copy (written or restored) -
// never evicted after reading.
static std::set<std::string> s_Local;			// abs paths
static void LoadRestoredList()					// written by pre.js: files restored from the saves
{
	FILE *fp = fopen( "/game_saves_restored.txt", "rb" );
	if ( !fp ) return;
	char line[1024];
	while ( fgets( line, sizeof(line), fp ) ) {
		line[strcspn( line, "\r\n" )] = 0;
		if ( line[0] ) s_Local.insert( std::string( PT_GAME_ROOT "/" ) + line );
	}
	fclose( fp );
}
static void LoadManifest()
{
	if ( s_ManLoaded ) return;
	s_ManLoaded = true;
	FILE *fp = fopen( "/game_manifest.txt", "rb" );
	if ( !fp ) { PtWin_Log( "no game manifest: game data cannot be fetched" ); return; }
	char line[1024];
	while ( fgets( line, sizeof(line), fp ) ) {
		char *tab = strchr( line, '\t' );
		if ( !tab ) continue;
		*tab = 0;
		std::string rel = line;
		s_ManFiles[rel] = atol( tab+1 );
		// register every directory level
		size_t k = 0;
		std::string dir = "";
		for ( ;; ) {
			size_t e = rel.find( '/', k );
			std::string name = rel.substr( k, e==std::string::npos ? std::string::npos : e-k );
			s_ManDirs[dir][Lower( name )] = name;
			if ( e==std::string::npos ) break;
			dir = dir.empty() ? name : dir + "/" + name;
			k = e+1;
		}
	}
	fclose( fp );
	PtWin_Log( "game manifest: %d files", (int)s_ManFiles.size() );
	LoadRestoredList();
}

// "/game/Field/x.smd" -> "Field/x.smd"; false if outside the game folder
static bool GameRel( const std::string &abs, std::string &rel )
{
	static const std::string root = PT_GAME_ROOT;
	if ( abs==root ) { rel = ""; return true; }
	if ( abs.compare( 0, root.size()+1, root + "/" ) ) return false;
	rel = abs.substr( root.size()+1 );
	return true;
}

static const std::map<std::string, std::string> &DirEntries( const std::string &dir )
{
	auto it = s_DirCache.find( dir );
	if ( it!=s_DirCache.end() ) return it->second;
	std::map<std::string, std::string> &m = s_DirCache[dir];
	std::string rel;
	if ( GameRel( dir, rel ) ) {
		auto md = s_ManDirs.find( rel );
		if ( md!=s_ManDirs.end() ) m = md->second;
	}
	DIR *d = opendir( dir.c_str() );
	if ( d ) {
		while ( struct dirent *e = readdir( d ) )
			if ( strcmp( e->d_name, "." ) && strcmp( e->d_name, ".." ) ) m[Lower( e->d_name )] = e->d_name;
		closedir( d );
	}
	return m;
}

void PtFs_Invalidate( const std::string &dir )
{
	pthread_mutex_lock( &s_DirLock );
	s_DirCache.erase( dir );
	pthread_mutex_unlock( &s_DirLock );
}

static void MakeDirs( const std::string &path )
{
	for ( size_t k = path.find( '/', 1 ); k!=std::string::npos; k = path.find( '/', k+1 ) )
		mkdir( path.substr( 0, k ).c_str(), 0777 );
}

// Fetch from the web server: the main thread waits without blocking the page
// (Asyncify); other threads use a synchronous request (allowed in workers).
// Both give up on a request after 45 s and try 3 times (a stalled request used to
// freeze the game for good, e.g. when changing maps quickly); 404 is final.
EM_JS( int, PtFs_FetchSync, ( const char *url, int *pSize ), {
	var u = UTF8ToString( url );
	for ( var attempt = 0; attempt < 3; attempt++ ) {
		var xhr = new XMLHttpRequest();
		xhr.open( 'GET', u, false );
		xhr.responseType = 'arraybuffer';
		try { xhr.timeout = 45000; } catch ( e ) {}		// allowed for synchronous requests in workers
		try { xhr.send( null ); } catch ( e ) { continue; }
		if ( xhr.status == 404 ) return 0;
		if ( xhr.status != 200 ) continue;
		var a = new Uint8Array( xhr.response );
		var p = _malloc( a.length || 1 );
		HEAPU8.set( a, p );
		HEAP32[pSize >> 2] = a.length;
		return p;
	}
	return 0;
} );

EM_ASYNC_JS( int, PtFs_FetchAsync, ( const char *url, int *pSize ), {
	var u = UTF8ToString( url );
	for ( var attempt = 0; attempt < 3; attempt++ ) {
		var ctl = self.AbortController ? new AbortController() : null;
		var timer = ctl ? setTimeout( function () { ctl.abort(); }, 45000 ) : 0;
		try {
			var r = await fetch( u, ctl ? { signal: ctl.signal } : {} );
			if ( r.status == 404 ) { clearTimeout( timer ); return 0; }
			if ( !r.ok ) throw new Error( 'HTTP ' + r.status );
			var a = new Uint8Array( await r.arrayBuffer() );
			clearTimeout( timer );
			var p = _malloc( a.length || 1 );
			HEAPU8.set( a, p );
			HEAP32[pSize >> 2] = a.length;
			return p;
		} catch ( e ) {
			clearTimeout( timer );
			await new Promise( function ( ok ) { setTimeout( ok, 400 * ( attempt + 1 ) ); } );
		}
	}
	return 0;
} );

EM_JS( char *, PtFs_DataUrl, ( const char *rel ), {
	var s = 'data/' + UTF8ToString( rel ).split( '/' ).map( encodeURIComponent ).join( '/' );
	return stringToNewUTF8( s );
} );

// One download per file, several files at once. The lock only guards the list of
// files being fetched (never held during a download): before, one lock was held for
// the whole download, so every other file - and the page itself, when the main
// thread waited on it - stood still behind one slow request. A file is written
// under a temporary name and renamed, so nobody reads it half written.
static pthread_mutex_t s_FetchLock = PTHREAD_MUTEX_INITIALIZER;
static std::set<std::string> s_Fetching;		// abs paths being downloaded

static void FetchWait()
{
	if ( emscripten_is_main_runtime_thread() ) emscripten_sleep( 5 );		// the page keeps running
	else usleep( 5000 );
}

static void EnsureFetched( const std::string &abs, const std::string &rel )
{
	struct stat st;
	if ( stat( abs.c_str(), &st )==0 ) return;
	for (;;) {			// someone else downloading it: wait for them
		while ( pthread_mutex_trylock( &s_FetchLock )!=0 ) FetchWait();
		if ( stat( abs.c_str(), &st )==0 ) { pthread_mutex_unlock( &s_FetchLock ); return; }
		if ( !s_Fetching.count( abs ) ) { s_Fetching.insert( abs ); pthread_mutex_unlock( &s_FetchLock ); break; }
		pthread_mutex_unlock( &s_FetchLock );
		FetchWait();
	}

	char *url = PtFs_DataUrl( rel.c_str() );
	int size = 0;
	void *data = emscripten_is_main_runtime_thread()
		? (void *)(intptr_t)PtFs_FetchAsync( url, &size )
		: (void *)(intptr_t)PtFs_FetchSync( url, &size );
	if ( data ) {
		MakeDirs( abs );
		std::string tmp = abs + ".ptpart";
		FILE *fp = fopen( tmp.c_str(), "wb" );
		if ( fp ) {
			if ( size ) fwrite( data, 1, size, fp );
			fclose( fp );
			if ( rename( tmp.c_str(), abs.c_str() )!=0 ) unlink( tmp.c_str() );
		}
		free( data );
	}
	else PtWin_Log( "fetch failed: %s", url );
	free( url );

	while ( pthread_mutex_trylock( &s_FetchLock )!=0 ) FetchWait();
	s_Fetching.erase( abs );
	pthread_mutex_unlock( &s_FetchLock );
}

// Game path -> path in the browser file system. Existing components (local or in
// the manifest) are matched case-insensitively; missing ones are kept as written
// (so new files can be created). A data file is fetched the first time.
std::string PtFs_Resolve( const char *szPath )
{
	std::string p = szPath ? szPath : "";
	for ( auto &c : p ) if ( c=='\\' ) c = '/';
	if ( p.size()>=2 && p[1]==':' ) p = p.substr( 2 );			// drop "C:"
	if ( p.empty() || p[0]!='/' ) {
		char cwd[512];
		if ( !getcwd( cwd, sizeof(cwd) ) ) strcpy( cwd, "/" );
		p = std::string( cwd ) + "/" + p;
	}

	std::vector<std::string> parts;
	size_t s = 0;
	while ( s<=p.size() ) {
		size_t e = p.find( '/', s );
		if ( e==std::string::npos ) e = p.size();
		std::string part = p.substr( s, e-s );
		if ( !part.empty() && part!="." ) {
			if ( part==".." ) { if ( !parts.empty() ) parts.pop_back(); }
			else parts.push_back( part );
		}
		s = e+1;
	}

	pthread_mutex_lock( &s_DirLock );
	LoadManifest();
	std::string cur = "";
	bool exists = true;
	for ( size_t i=0; i<parts.size(); i++ ) {
		std::string name = parts[i];
		if ( exists ) {
			const auto &m = DirEntries( cur.empty() ? "/" : cur );
			auto it = m.find( Lower( name ) );
			if ( it!=m.end() ) name = it->second;
			else exists = false;
		}
		cur += "/" + name;
	}
	if ( cur.empty() ) cur = "/";
	std::string rel;
	bool fetch = exists && GameRel( cur, rel ) && s_ManFiles.count( rel );
	pthread_mutex_unlock( &s_DirLock );
	if ( fetch ) EnsureFetched( cur, rel );
	return cur;
}

// Size from the manifest for files not fetched yet (FindFirstFile); -1 if unknown.
long PtFs_ManifestSize( const std::string &abs )
{
	std::string rel;
	if ( !GameRel( abs, rel ) ) return -1;
	pthread_mutex_lock( &s_DirLock );
	auto it = s_ManFiles.find( rel );
	long n = it==s_ManFiles.end() ? -1 : it->second;
	pthread_mutex_unlock( &s_DirLock );
	return n;
}

// A folder that only the manifest knows (none of its files fetched yet)
static bool PtFs_ManifestIsDir( const std::string &abs )
{
	std::string rel;
	if ( !GameRel( abs, rel ) ) return false;
	pthread_mutex_lock( &s_DirLock );
	bool d = s_ManDirs.count( rel )>0;
	pthread_mutex_unlock( &s_DirLock );
	return d;
}

// Names in a directory (local files + manifest), for FindFirstFile.
std::vector<std::string> PtFs_List( const std::string &absDir )
{
	std::vector<std::string> names;
	pthread_mutex_lock( &s_DirLock );
	LoadManifest();
	for ( auto &e : DirEntries( absDir ) ) names.push_back( e.second );
	pthread_mutex_unlock( &s_DirLock );
	return names;
}

static void InvalidateParent( const std::string &real )
{
	size_t k = real.rfind( '/' );
	PtFs_Invalidate( k==std::string::npos || k==0 ? "/" : real.substr( 0, k ) );
}

// Written files worth keeping: everything in the game folder except logs and screenshots
static bool IsSaveFile( const std::string &abs, std::string &rel )
{
	if ( !GameRel( abs, rel ) || rel.empty() ) return false;
	std::string l = Lower( rel );
	if ( l.size()>4 && !l.compare( l.size()-4, 4, ".log" ) ) return false;
	if ( l.find( "logfile/" )!=std::string::npos || l.find( "screenshot" )!=std::string::npos ) return false;
	size_t k = l.rfind( '/' );
	std::string name = l.substr( k==std::string::npos ? 0 : k+1 );
	if ( !name.compare( 0, 6, "ptgfx_" ) || name=="missing_models.txt" ) return false;		// diagnostics
	return true;
}

// The game wrote (or deleted) a file: tell pre.js, which copies it to IndexedDB
static void NoteSaved( const std::string &abs, bool deleted )
{
	pthread_mutex_lock( &s_DirLock );
	s_Local.insert( abs );
	std::string rel;
	bool save = IsSaveFile( abs, rel );
	if ( deleted && GameRel( abs, rel ) && s_ManFiles.erase( rel ) ) {		// do not fetch it again
		size_t k = rel.rfind( '/' );
		std::string dir = k==std::string::npos ? "" : rel.substr( 0, k );
		auto md = s_ManDirs.find( dir );
		if ( md!=s_ManDirs.end() ) md->second.erase( Lower( rel.substr( k==std::string::npos ? 0 : k+1 ) ) );
	}
	pthread_mutex_unlock( &s_DirLock );
	InvalidateParent( abs );
	if ( !save ) return;
	char *copy = strdup( rel.c_str() );
	MAIN_THREAD_ASYNC_EM_ASM( {
		if ( Module['ptSaveChanged'] ) Module['ptSaveChanged']( UTF8ToString( $0 ), $1 );
		_free( $0 );
	}, copy, deleted ? 1 : 0 );
}

static bool IsLocal( const std::string &abs )
{
	pthread_mutex_lock( &s_DirLock );
	bool b = s_Local.count( abs )!=0;
	pthread_mutex_unlock( &s_DirLock );
	return b;
}

//////////////////////////////////////////////////////////////////////////////
// C runtime entry points the game uses directly
//////////////////////////////////////////////////////////////////////////////
// Big data files are read once (models, maps, textures): once the game closes
// them, drop the browser's copy to save memory; opening one again fetches it again.
#define PT_EVICT_MIN_SIZE	(256*1024)
static pthread_mutex_t s_EvictLock = PTHREAD_MUTEX_INITIALIZER;
static std::map<FILE *, std::string> s_EvictOnClose;
// Files opened for writing are saved when closed - if they really changed (some
// game files are opened read/write and only read).
struct PtWriteWatch {
	std::string	path;
	bool		always;			// created or truncated
	long long	mtime, size;
};
static PtWriteWatch WatchWrite( const std::string &real, bool truncates )
{
	PtWriteWatch w = { real, truncates, 0, 0 };
	struct stat st;
	if ( stat( real.c_str(), &st )==0 ) { w.mtime = st.st_mtim.tv_sec*1000000000LL + st.st_mtim.tv_nsec; w.size = st.st_size; }
	return w;
}
static void EndWrite( const PtWriteWatch &w )
{
	struct stat st;
	if ( stat( w.path.c_str(), &st ) ) return;
	if ( w.always || w.size!=st.st_size || w.mtime!=st.st_mtim.tv_sec*1000000000LL + st.st_mtim.tv_nsec )
		NoteSaved( w.path, false );
}
static std::map<FILE *, PtWriteWatch> s_WrittenOnClose;

extern "C" FILE *PtWin_fopen( const char *szPath, const char *szMode )
{
	std::string real = PtFs_Resolve( szPath );
	FILE *fp = fopen( real.c_str(), szMode );
	bool bWrite = strpbrk( szMode, "wa+" )!=NULL;
	if ( fp && bWrite ) {
		InvalidateParent( real );
		pthread_mutex_lock( &s_EvictLock );
		s_WrittenOnClose[fp] = WatchWrite( real, strpbrk( szMode, "wa" )!=NULL );
		pthread_mutex_unlock( &s_EvictLock );
	}
	if ( fp && !bWrite && PtFs_ManifestSize( real )>=PT_EVICT_MIN_SIZE && !IsLocal( real ) ) {
		pthread_mutex_lock( &s_EvictLock );
		s_EvictOnClose[fp] = real;
		pthread_mutex_unlock( &s_EvictLock );
	}
	return fp;
}

extern "C" int PtWin_fclose( FILE *fp )
{
	std::string evict;
	PtWriteWatch written;
	pthread_mutex_lock( &s_EvictLock );
	auto it = s_EvictOnClose.find( fp );
	if ( it!=s_EvictOnClose.end() ) { evict = it->second; s_EvictOnClose.erase( it ); }
	auto wt = s_WrittenOnClose.find( fp );
	if ( wt!=s_WrittenOnClose.end() ) { written = wt->second; s_WrittenOnClose.erase( wt ); }
	else written.path.clear();
	pthread_mutex_unlock( &s_EvictLock );
	int r = fclose( fp );
	if ( !evict.empty() ) unlink( evict.c_str() );		// still listed (manifest), fetched again if reopened
	if ( !written.path.empty() ) EndWrite( written );
	return r;
}

extern "C" int PtWin_remove( const char *szPath )
{
	std::string r = PtFs_Resolve( szPath );
	int ret = remove( r.c_str() );
	if ( ret==0 ) NoteSaved( r, true );
	return ret;
}
extern "C" int PtWin_access( const char *szPath, int mode ) { return access( PtFs_Resolve( szPath ).c_str(), mode ); }
extern "C" int PtWin_mkdir( const char *szPath ) { std::string r = PtFs_Resolve( szPath ); InvalidateParent( r ); return mkdir( r.c_str(), 0777 ); }

//////////////////////////////////////////////////////////////////////////////
// Win32 file handles
//////////////////////////////////////////////////////////////////////////////
struct PtFile : PtObject {
	FILE *fp;
	PtWriteWatch written;			// opened for writing: saved when closed (path empty: read only)
	PtFile( FILE *f ) { magic = PTOBJ_MAGIC; type = PTOBJ_FILE; fp = f; }
	~PtFile() { if ( fp ) fclose( fp ); magic = 0; if ( !written.path.empty() ) EndWrite( written ); }
};

HANDLE WINAPI CreateFileA( LPCSTR szName, DWORD dwAccess, DWORD, LPSECURITY_ATTRIBUTES, DWORD dwCreation, DWORD, HANDLE )
{
	std::string real = PtFs_Resolve( szName );
	struct stat st;
	bool exists = stat( real.c_str(), &st )==0;
	BOOL bWrite = ( dwAccess & ( GENERIC_WRITE | FILE_WRITE_DATA | FILE_APPEND_DATA ) )!=0;
	const char *mode = "rb";
	switch ( dwCreation ) {
	case CREATE_NEW:		if ( exists ) { SetLastError( ERROR_FILE_EXISTS ); return INVALID_HANDLE_VALUE; } mode = "w+b"; break;
	case CREATE_ALWAYS:		mode = "w+b"; break;
	case OPEN_EXISTING:		if ( !exists ) { SetLastError( ERROR_FILE_NOT_FOUND ); return INVALID_HANDLE_VALUE; } mode = bWrite ? "r+b" : "rb"; break;
	case OPEN_ALWAYS:		mode = exists ? ( bWrite ? "r+b" : "rb" ) : "w+b"; break;
	case TRUNCATE_EXISTING:	if ( !exists ) { SetLastError( ERROR_FILE_NOT_FOUND ); return INVALID_HANDLE_VALUE; } mode = "w+b"; break;
	}
	FILE *fp = fopen( real.c_str(), mode );
	if ( !fp ) { SetLastError( ERROR_FILE_NOT_FOUND ); return INVALID_HANDLE_VALUE; }
	if ( !exists ) InvalidateParent( real );
	PtFile *f = new PtFile( fp );
	if ( strcmp( mode, "rb" ) ) f->written = WatchWrite( real, mode[0]=='w' );
	return (HANDLE)f;
}

BOOL WINAPI ReadFile( HANDLE h, LPVOID buf, DWORD n, LPDWORD pRead, LPOVERLAPPED )
{
	PtFile *f = PtObj<PtFile>( h, PTOBJ_FILE );
	if ( !f ) { if ( pRead ) *pRead = 0; return FALSE; }
	size_t got = fread( buf, 1, n, f->fp );
	if ( pRead ) *pRead = (DWORD)got;
	return TRUE;
}

BOOL WINAPI WriteFile( HANDLE h, LPCVOID buf, DWORD n, LPDWORD pWritten, LPOVERLAPPED )
{
	PtFile *f = PtObj<PtFile>( h, PTOBJ_FILE );
	if ( !f ) { if ( pWritten ) *pWritten = 0; return FALSE; }
	size_t put = fwrite( buf, 1, n, f->fp );
	if ( pWritten ) *pWritten = (DWORD)put;
	return put==n;
}

DWORD WINAPI SetFilePointer( HANDLE h, LONG lDist, PLONG pHigh, DWORD dwMethod )
{
	PtFile *f = PtObj<PtFile>( h, PTOBJ_FILE );
	if ( !f ) return INVALID_SET_FILE_POINTER;
	int whence = dwMethod==FILE_CURRENT ? SEEK_CUR : dwMethod==FILE_END ? SEEK_END : SEEK_SET;
	if ( fseek( f->fp, lDist, whence ) ) return INVALID_SET_FILE_POINTER;
	if ( pHigh ) *pHigh = 0;
	return (DWORD)ftell( f->fp );
}

DWORD WINAPI GetFileSize( HANDLE h, LPDWORD pHigh )
{
	PtFile *f = PtObj<PtFile>( h, PTOBJ_FILE );
	if ( pHigh ) *pHigh = 0;
	if ( !f ) return INVALID_FILE_SIZE;
	long cur = ftell( f->fp );
	fseek( f->fp, 0, SEEK_END );
	long size = ftell( f->fp );
	fseek( f->fp, cur, SEEK_SET );
	return (DWORD)size;
}

BOOL WINAPI FlushFileBuffers( HANDLE h ) { PtFile *f = PtObj<PtFile>( h, PTOBJ_FILE ); return f && fflush( f->fp )==0; }

// 16-bit era file functions (_lopen etc.) used by a few save routines
HFILE WINAPI _lcreat( LPCSTR szPath, int ) { HANDLE h = CreateFileA( szPath, GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL ); return h==INVALID_HANDLE_VALUE ? HFILE_ERROR : (HFILE)(intptr_t)h; }
HFILE WINAPI _lopen( LPCSTR szPath, int mode ) { HANDLE h = CreateFileA( szPath, ( mode & 3 ) ? GENERIC_READ | GENERIC_WRITE : GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL ); return h==INVALID_HANDLE_VALUE ? HFILE_ERROR : (HFILE)(intptr_t)h; }
UINT WINAPI _lread( HFILE hf, LPVOID buf, UINT n ) { DWORD got = 0; return ReadFile( (HANDLE)(intptr_t)hf, buf, n, &got, NULL ) ? got : HFILE_ERROR; }
UINT WINAPI _lwrite( HFILE hf, LPCCH buf, UINT n ) { DWORD put = 0; return WriteFile( (HANDLE)(intptr_t)hf, buf, n, &put, NULL ) ? put : HFILE_ERROR; }
LONG WINAPI _llseek( HFILE hf, LONG off, int origin ) { return (LONG)SetFilePointer( (HANDLE)(intptr_t)hf, off, NULL, origin ); }
HFILE WINAPI _lclose( HFILE hf ) { return CloseHandle( (HANDLE)(intptr_t)hf ) ? 0 : HFILE_ERROR; }

BOOL WINAPI DeleteFileA( LPCSTR szPath )
{
	std::string r = PtFs_Resolve( szPath );
	if ( unlink( r.c_str() ) ) { InvalidateParent( r ); return FALSE; }
	NoteSaved( r, true );
	return TRUE;
}
BOOL WINAPI CreateDirectoryA( LPCSTR szPath, LPSECURITY_ATTRIBUTES ) { std::string r = PtFs_Resolve( szPath ); InvalidateParent( r ); return mkdir( r.c_str(), 0777 )==0; }

BOOL WINAPI CopyFileA( LPCSTR szFrom, LPCSTR szTo, BOOL bFailIfExists )
{
	std::string from = PtFs_Resolve( szFrom ), to = PtFs_Resolve( szTo );
	if ( bFailIfExists && access( to.c_str(), F_OK )==0 ) return FALSE;
	FILE *a = fopen( from.c_str(), "rb" );
	if ( !a ) return FALSE;
	FILE *b = fopen( to.c_str(), "wb" );
	if ( !b ) { fclose( a ); return FALSE; }
	char buf[65536];
	size_t n;
	while ( ( n = fread( buf, 1, sizeof(buf), a ) )>0 ) fwrite( buf, 1, n, b );
	fclose( a ); fclose( b );
	NoteSaved( to, false );
	return TRUE;
}

DWORD WINAPI GetFileAttributesA( LPCSTR szPath )
{
	struct stat st;
	if ( stat( PtFs_Resolve( szPath ).c_str(), &st ) ) return INVALID_FILE_ATTRIBUTES;
	return S_ISDIR( st.st_mode ) ? FILE_ATTRIBUTE_DIRECTORY : FILE_ATTRIBUTE_NORMAL;
}

BOOL WINAPI PathFileExistsA( LPCSTR szPath ) { return GetFileAttributesA( szPath )!=INVALID_FILE_ATTRIBUTES; }

//////////////////////////////////////////////////////////////////////////////
// FindFirstFile / FindNextFile (wildcards * and ?)
//////////////////////////////////////////////////////////////////////////////
struct PtFind : PtObject {
	std::string dir;
	std::vector<std::string> names;
	size_t next;
	PtFind() { magic = PTOBJ_MAGIC; type = PTOBJ_FIND; next = 0; }
	~PtFind() { magic = 0; }
};

static bool WildMatch( const char *pat, const char *s )
{
	if ( !*pat ) return !*s;
	if ( *pat=='*' ) { for ( ;; s++ ) { if ( WildMatch( pat+1, s ) ) return true; if ( !*s ) return false; } }
	if ( !*s ) return false;
	if ( *pat=='?' || tolower( (unsigned char)*pat )==tolower( (unsigned char)*s ) ) return WildMatch( pat+1, s+1 );
	return false;
}

static void FillFindData( PtFind *f, const std::string &name, LPWIN32_FIND_DATAA pData )
{
	ZeroMemory( pData, sizeof(*pData) );
	struct stat st;
	ZeroMemory( &st, sizeof(st) );
	std::string full = f->dir + "/" + name;
	long manSize = PtFs_ManifestSize( full );
	if ( stat( full.c_str(), &st )!=0 ) {
		ZeroMemory( &st, sizeof(st) );
		if ( manSize>=0 ) {				// in the manifest, not fetched yet
			st.st_mode = S_IFREG;
			st.st_size = manSize;
			st.st_mtime = 1262304000;
		}
		else if ( PtFs_ManifestIsDir( full ) ) {		// a folder none of whose files are here yet
			st.st_mode = S_IFDIR;			// (was reported as a file: the effect loaders skipped it)
			st.st_mtime = 1262304000;
		}
	}
	if ( st.st_mode ) {
		pData->dwFileAttributes = S_ISDIR( st.st_mode ) ? FILE_ATTRIBUTE_DIRECTORY : FILE_ATTRIBUTE_NORMAL;
		pData->nFileSizeLow = (DWORD)st.st_size;
		ULONGLONG t = ( (ULONGLONG)st.st_mtime + 11644473600ULL ) * 10000000ULL;
		pData->ftLastWriteTime.dwLowDateTime = (DWORD)t;
		pData->ftLastWriteTime.dwHighDateTime = (DWORD)( t>>32 );
		pData->ftCreationTime = pData->ftLastAccessTime = pData->ftLastWriteTime;
	}
	strncpy( pData->cFileName, name.c_str(), MAX_PATH-1 );
}

HANDLE WINAPI FindFirstFileA( LPCSTR szPattern, LPWIN32_FIND_DATAA pData )
{
	std::string p = szPattern;
	for ( auto &c : p ) if ( c=='\\' ) c = '/';
	size_t k = p.rfind( '/' );
	std::string dirPart = k==std::string::npos ? "." : p.substr( 0, k );
	std::string pat = k==std::string::npos ? p : p.substr( k+1 );
	PtFind *f = new PtFind;
	f->dir = PtFs_Resolve( dirPart.c_str() );
	// Windows: a trailing ".*" also matches names with no dot ("*.*" is every file
	// and folder; without this the effect loaders never saw their sub-folders)
	std::string patNoExt;
	if ( pat.size()>=2 && !pat.compare( pat.size()-2, 2, ".*" ) ) patNoExt = pat.substr( 0, pat.size()-2 );
	for ( auto &n : PtFs_List( f->dir ) )
		if ( WildMatch( pat.c_str(), n.c_str() ) ||
			 ( !patNoExt.empty() && n.find( '.' )==std::string::npos && WildMatch( patNoExt.c_str(), n.c_str() ) ) )
			f->names.push_back( n );
	if ( f->names.empty() ) { delete f; SetLastError( ERROR_FILE_NOT_FOUND ); return INVALID_HANDLE_VALUE; }
	FillFindData( f, f->names[f->next++], pData );
	return (HANDLE)f;
}

BOOL WINAPI FindNextFileA( HANDLE h, LPWIN32_FIND_DATAA pData )
{
	PtFind *f = PtObj<PtFind>( h, PTOBJ_FIND );
	if ( !f || f->next>=f->names.size() ) { SetLastError( ERROR_NO_MORE_FILES ); return FALSE; }
	FillFindData( f, f->names[f->next++], pData );
	return TRUE;
}

BOOL WINAPI FindClose( HANDLE h )
{
	PtFind *f = PtObj<PtFind>( h, PTOBJ_FIND );
	if ( !f ) return FALSE;
	delete f;
	return TRUE;
}

//////////////////////////////////////////////////////////////////////////////
// File mappings: the whole file read into memory
//////////////////////////////////////////////////////////////////////////////
struct PtMapping : PtObject {
	std::vector<BYTE> data;
	PtMapping() { magic = PTOBJ_MAGIC; type = PTOBJ_MAPPING; }
	~PtMapping() { magic = 0; }
};

HANDLE WINAPI CreateFileMappingA( HANDLE hFile, LPSECURITY_ATTRIBUTES, DWORD, DWORD, DWORD dwSizeLow, LPCSTR )
{
	PtMapping *m = new PtMapping;
	PtFile *f = PtObj<PtFile>( hFile, PTOBJ_FILE );
	if ( f ) {
		DWORD size = GetFileSize( hFile, NULL );
		m->data.resize( size );
		long cur = ftell( f->fp );
		fseek( f->fp, 0, SEEK_SET );
		if ( size ) fread( &m->data[0], 1, size, f->fp );
		fseek( f->fp, cur, SEEK_SET );
	}
	else m->data.resize( dwSizeLow );		// page-file backed (shared memory)
	return (HANDLE)m;
}

static std::map<void *, PtMapping *> s_Views;

LPVOID WINAPI MapViewOfFile( HANDLE h, DWORD, DWORD, DWORD dwOffsetLow, SIZE_T )
{
	PtMapping *m = PtObj<PtMapping>( h, PTOBJ_MAPPING );
	if ( !m || m->data.empty() ) return NULL;
	void *p = &m->data[0] + dwOffsetLow;
	s_Views[p] = m;
	return p;
}

BOOL WINAPI UnmapViewOfFile( LPCVOID p ) { return s_Views.erase( (void *)p ) ? TRUE : FALSE; }

//////////////////////////////////////////////////////////////////////////////
// Closing any handle
//////////////////////////////////////////////////////////////////////////////
BOOL WINAPI CloseHandle( HANDLE h )
{
	PtObject *p = (PtObject *)h;
	if ( !p || h==INVALID_HANDLE_VALUE || p->magic!=PTOBJ_MAGIC ) return FALSE;
	if ( p->type==PTOBJ_THREAD ) {
		extern void PtWin_ReleaseThread( PtObject * );
		PtWin_ReleaseThread( p );
		return TRUE;
	}
	delete p;
	return TRUE;
}
