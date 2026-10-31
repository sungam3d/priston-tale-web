// ptwin_crt.cpp - Microsoft C runtime extras and the lstr*/wsprintf/char-set
// functions of kernel32/user32 (web build).
#include "ptwin_internal.h"
#include <ctype.h>
#include <strings.h>
#include <wchar.h>
#include <wctype.h>

extern "C" {

int stricmp( const char *a, const char *b ) { return strcasecmp( a, b ); }
int strnicmp( const char *a, const char *b, size_t n ) { return strncasecmp( a, b, n ); }
int _wcsicmp( const wchar_t *a, const wchar_t *b )
{
	for ( ;; a++, b++ ) {
		wint_t x = towlower( *a ), y = towlower( *b );
		if ( x!=y || !x ) return (int)x - (int)y;
	}
}

char *strupr( char *s ) { for ( char *p = s; *p; p++ ) *p = (char)toupper( (unsigned char)*p ); return s; }
char *strlwr( char *s ) { for ( char *p = s; *p; p++ ) *p = (char)tolower( (unsigned char)*p ); return s; }

static char *ToRadix( unsigned long long v, bool neg, char *buf, int radix )
{
	char tmp[72];
	int n = 0;
	if ( radix<2 || radix>36 ) radix = 10;
	do { int d = (int)( v % radix ); tmp[n++] = (char)( d<10 ? '0'+d : 'a'+d-10 ); v /= radix; } while ( v );
	char *p = buf;
	if ( neg ) *p++ = '-';
	while ( n ) *p++ = tmp[--n];
	*p = 0;
	return buf;
}
char *itoa( int value, char *buf, int radix ) { return ( radix==10 && value<0 ) ? ToRadix( -(long long)value, true, buf, 10 ) : ToRadix( (unsigned)value, false, buf, radix ); }
char *ltoa( long value, char *buf, int radix ) { return itoa( (int)value, buf, radix ); }
char *ultoa( unsigned long value, char *buf, int radix ) { return ToRadix( value, false, buf, radix ); }
char *_ui64toa( unsigned long long value, char *buf, int radix ) { return ToRadix( value, false, buf, radix ); }
char *_i64toa( long long value, char *buf, int radix ) { return ( radix==10 && value<0 ) ? ToRadix( 0ULL - (unsigned long long)value, true, buf, 10 ) : ToRadix( (unsigned long long)value, false, buf, radix ); }

void _splitpath( const char *path, char *drive, char *dir, char *fname, char *ext )
{
	const char *p = path;
	if ( drive ) { drive[0] = 0; if ( p[0] && p[1]==':' ) { drive[0] = p[0]; drive[1] = ':'; drive[2] = 0; } }
	if ( p[0] && p[1]==':' ) p += 2;
	const char *lastSlash = NULL, *lastDot = NULL;
	for ( const char *q = p; *q; q++ ) {
		if ( *q=='\\' || *q=='/' ) lastSlash = q;
		else if ( *q=='.' ) lastDot = q;
	}
	if ( lastDot && lastSlash && lastDot<lastSlash ) lastDot = NULL;
	const char *nameStart = lastSlash ? lastSlash+1 : p;
	if ( dir ) { size_t n = nameStart - p; memcpy( dir, p, n ); dir[n] = 0; }
	const char *nameEnd = lastDot ? lastDot : nameStart + strlen( nameStart );
	if ( fname ) { size_t n = nameEnd - nameStart; memcpy( fname, nameStart, n ); fname[n] = 0; }
	if ( ext ) strcpy( ext, lastDot ? lastDot : "" );
}

} // extern "C"

//////////////////////////////////////////////////////////////////////////////
// kernel32 / user32 string helpers
//////////////////////////////////////////////////////////////////////////////
LPSTR WINAPI lstrcpyA( LPSTR a, LPCSTR b ) { return strcpy( a, b ); }
// Like Windows: writes only the text and its terminator, never the rest of the n
// bytes (strncpy zero-fills them all). The game passes sizes bigger than its
// buffers (GuideHelp.cpp: 1024 for 100-byte rows), which wiped the memory after.
LPSTR WINAPI lstrcpynA( LPSTR a, LPCSTR b, int n )
{
	if ( n<=0 ) return a;
	int i = 0;
	for ( ; i<n-1 && b[i]; i++ ) a[i] = b[i];
	a[i] = 0;
	return a;
}
LPSTR WINAPI lstrcatA( LPSTR a, LPCSTR b ) { return strcat( a, b ); }
int WINAPI lstrlenA( LPCSTR a ) { return a ? (int)strlen( a ) : 0; }
int WINAPI lstrcmpA( LPCSTR a, LPCSTR b ) { return strcmp( a, b ); }
int WINAPI lstrcmpiA( LPCSTR a, LPCSTR b ) { return strcasecmp( a, b ); }

LPSTR WINAPI CharLowerA( LPSTR s )
{
	if ( (uintptr_t)s < 0x10000 ) return (LPSTR)(uintptr_t)tolower( (int)(uintptr_t)s );
	return strlwr( s );
}
LPSTR WINAPI CharUpperA( LPSTR s )
{
	if ( (uintptr_t)s < 0x10000 ) return (LPSTR)(uintptr_t)toupper( (int)(uintptr_t)s );
	return strupr( s );
}

// The data is English (code page 1252 / ASCII): no double-byte lead bytes.
BOOL WINAPI IsDBCSLeadByte( BYTE ) { return FALSE; }

int WINAPIV wsprintfA( LPSTR buf, LPCSTR fmt, ... )
{
	va_list args;
	va_start( args, fmt );
	int n = vsprintf( buf, fmt, args );
	va_end( args );
	return n;
}
int WINAPI wvsprintfA( LPSTR buf, LPCSTR fmt, va_list args ) { return vsprintf( buf, fmt, args ); }

// Code pages: UTF-8, and code page 1252 for everything else (English data).
static const unsigned short s_Cp1252[32] = {
	0x20AC,0x0081,0x201A,0x0192,0x201E,0x2026,0x2020,0x2021,0x02C6,0x2030,0x0160,0x2039,0x0152,0x008D,0x017D,0x008F,
	0x0090,0x2018,0x2019,0x201C,0x201D,0x2022,0x2013,0x2014,0x02DC,0x2122,0x0161,0x203A,0x0153,0x009D,0x017E,0x0178 };

int WINAPI MultiByteToWideChar( UINT cp, DWORD, LPCCH src, int cb, LPWSTR dst, int cch )
{
	if ( cb<0 ) cb = (int)strlen( src ) + 1;
	std::vector<WCHAR> out;
	const unsigned char *s = (const unsigned char *)src;
	for ( int i=0; i<cb; ) {
		unsigned c = s[i];
		if ( cp==CP_UTF8 && c>=0x80 ) {
			int n = c>=0xF0 ? 3 : c>=0xE0 ? 2 : c>=0xC0 ? 1 : 0;
			unsigned v = c & ( 0x3F>>n );
			i++;
			for ( int k=0; k<n && i<cb; k++, i++ ) v = ( v<<6 ) | ( s[i] & 0x3F );
			out.push_back( (WCHAR)v );
			continue;
		}
		out.push_back( (WCHAR)( c>=0x80 && c<0xA0 ? s_Cp1252[c-0x80] : c ) );
		i++;
	}
	if ( !dst || !cch ) return (int)out.size();
	int n = (int)out.size() < cch ? (int)out.size() : cch;
	for ( int i=0; i<n; i++ ) dst[i] = out[i];
	return n;
}

int WINAPI WideCharToMultiByte( UINT cp, DWORD, LPCWCH src, int cch, LPSTR dst, int cb, LPCCH, LPBOOL pUsedDefault )
{
	if ( cch<0 ) { cch = 0; while ( src[cch] ) cch++; cch++; }
	std::string out;
	for ( int i=0; i<cch; i++ ) {
		unsigned v = src[i];
		if ( cp==CP_UTF8 ) {
			if ( v<0x80 ) out += (char)v;
			else if ( v<0x800 ) { out += (char)( 0xC0 | ( v>>6 ) ); out += (char)( 0x80 | ( v & 0x3F ) ); }
			else { out += (char)( 0xE0 | ( v>>12 ) ); out += (char)( 0x80 | ( ( v>>6 ) & 0x3F ) ); out += (char)( 0x80 | ( v & 0x3F ) ); }
			continue;
		}
		char c = '?';
		if ( v<0x80 || ( v>=0xA0 && v<0x100 ) ) c = (char)v;
		else for ( int k=0; k<32; k++ ) if ( s_Cp1252[k]==v ) { c = (char)( 0x80+k ); break; }
		if ( c=='?' && v!='?' && pUsedDefault ) *pUsedDefault = TRUE;
		out += c;
	}
	if ( !dst || !cb ) return (int)out.size();
	int n = (int)out.size() < cb ? (int)out.size() : cb;
	memcpy( dst, out.data(), n );
	return n;
}

UINT WINAPI GetACP() { return 1252; }
UINT WINAPI GetOEMCP() { return 437; }
