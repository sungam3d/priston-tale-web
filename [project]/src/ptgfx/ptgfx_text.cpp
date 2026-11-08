// ptgfx_text.cpp - our own text rasterizer (stb_truetype) in place of GDI text.
//
// The game draws text with TextOut / DrawText / GetTextExtentPoint32 on DCs it
// gets from DirectDraw surfaces. PtText_Install() points game.exe's imports of
// those functions at the versions below, so no game code changes:
//   - the font comes from the DC's selected HFONT (GetObject -> LOGFONT),
//   - glyphs are rasterized by stb_truetype (antialiased) straight into the
//     DC's 32-bit DIB section, using the DC's text colour / alignment / clip box,
//   - DCs without a 32-bit DIB, or fonts we cannot load, fall back to real GDI.
// Measuring always uses our metrics when the font loads, so layout matches what
// we draw. The browser build will call the same functions through a fake DC.

#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <map>
#include <string>
#include <vector>

#define STB_TRUETYPE_IMPLEMENTATION
#define STBTT_STATIC
#include "..\thirdparty\stb\stb_truetype.h"

#include "ptgl.h"
#include "ptgfx_internal.h"

//////////////////////////////////////////////////////////////////////////////
// Font files
//////////////////////////////////////////////////////////////////////////////
struct PtFontFile {
	std::vector<unsigned char> data;
	stbtt_fontinfo info;
	int unitsPerEm, winAscent, winDescent;
};

static std::map<std::string, PtFontFile *> s_Files;		// file name -> loaded font (NULL = not found)

static int ReadU16( const unsigned char *p ) { return (p[0]<<8) | p[1]; }

static PtFontFile *LoadFontFile( const char *szFile )
{
	std::map<std::string, PtFontFile *>::iterator it = s_Files.find( szFile );
	if ( it!=s_Files.end() ) return it->second;

	char szPath[MAX_PATH];
	const char *dirs[2] = { "fonts", NULL };
	char szWinFonts[MAX_PATH];
	if ( GetWindowsDirectoryA( szWinFonts, MAX_PATH ) ) { strcat( szWinFonts, "\\Fonts" ); dirs[1] = szWinFonts; }

	PtFontFile *pFile = NULL;
	for ( int d=0; d<2 && !pFile; d++ ) {
		if ( !dirs[d] ) continue;
		sprintf( szPath, "%s\\%s", dirs[d], szFile );
		FILE *fp = fopen( szPath, "rb" );
		if ( !fp ) continue;
		fseek( fp, 0, SEEK_END );
		long len = ftell( fp );
		fseek( fp, 0, SEEK_SET );
		PtFontFile *f = new PtFontFile;
		f->data.resize( len );
		BOOL bRead = len>0 && fread( &f->data[0], 1, len, fp )==(size_t)len;
		fclose( fp );
		int offset = bRead ? stbtt_GetFontOffsetForIndex( &f->data[0], 0 ) : -1;
		if ( offset<0 || !stbtt_InitFont( &f->info, &f->data[0], offset ) ) { delete f; continue; }

		// GDI sizes fonts from the OS/2 table's usWinAscent/usWinDescent.
		const unsigned char *head = f->info.data + f->info.head;
		f->unitsPerEm = ReadU16( head + 18 );
		int asc, desc, gap;
		stbtt_GetFontVMetrics( &f->info, &asc, &desc, &gap );
		f->winAscent = asc; f->winDescent = -desc;
		stbtt_uint32 os2 = stbtt__find_table( f->info.data, f->info.fontstart, "OS/2" );
		if ( os2 ) {
			f->winAscent = ReadU16( f->info.data + os2 + 74 );
			f->winDescent = ReadU16( f->info.data + os2 + 76 );
		}
		pFile = f;
		PtGL_Log( "text: loaded font %s", szPath );
	}
	if ( !pFile ) PtGL_Log( "text: font file %s not found", szFile );
	s_Files[szFile] = pFile;
	return pFile;
}

// Face name -> font file. Anything unknown uses Tahoma (the English client's font).
static void FaceToFiles( const char *szFace, BOOL bBold, const char **pszFile, const char **pszFallback )
{
	char lower[LF_FACESIZE+1];
	int i;
	for ( i=0; szFace[i] && i<LF_FACESIZE; i++ ) lower[i] = (char)tolower( (unsigned char)szFace[i] );
	lower[i] = 0;

	*pszFallback = NULL;
	if ( strstr( lower, "comic" ) )			*pszFile = bBold ? "comicbd.ttf" : "comic.ttf";
	else if ( strstr( lower, "arial" ) )	*pszFile = bBold ? "arialbd.ttf" : "arial.ttf";
	else									*pszFile = bBold ? "tahomabd.ttf" : "tahoma.ttf";
	if ( !bBold ) return;
	*pszFallback = strstr( lower, "comic" ) ? "comic.ttf" : strstr( lower, "arial" ) ? "arial.ttf" : "tahoma.ttf";
}

//////////////////////////////////////////////////////////////////////////////
// Sized fonts and glyph cache
//////////////////////////////////////////////////////////////////////////////
struct PtGlyph {
	int advance;
	int w, h, xoff, yoff;
	std::vector<unsigned char> bits;
};

struct PtFont {
	PtFontFile *pFile;
	PtFontFile *pExtra;		// for glyphs the main font lacks (Korean etc.)
	float scale, scaleExtra;
	int ascent, descent, height;
	int boldExtra;			// synthetic bold: draw twice, 1 pixel apart
	UINT codePage;
	std::map<int, PtGlyph *> glyphs;
};

static std::map<std::string, PtFont *> s_Fonts;		// LOGFONT key + code page -> sized font

// The language setting's code page (lang\pt_lang.cpp): the game's text is in that
// language's Windows code page whatever charset its fonts ask for. 0 = the font's own.
UINT g_PtTextCodePage = 0;

// A font for the glyphs Tahoma lacks: Korean, Japanese and Chinese. The fonts folder
// first (the browser build's data\fonts: Noto Sans CJK, SIL Open Font License), then
// what Windows has.
static PtFontFile *ExtraFont( UINT cp )
{
	static const char *s_Korean[] = { "NotoSansKR-Regular.otf", "malgun.ttf", "gulim.ttc", 0 };
	static const char *s_Japanese[] = { "NotoSansJP-Regular.otf", "msgothic.ttc", "meiryo.ttc", 0 };
	static const char *s_Simplified[] = { "NotoSansSC-Regular.otf", "msyh.ttc", "simsun.ttc", 0 };
	static const char *s_Traditional[] = { "NotoSansTC-Regular.otf", "msjh.ttc", "mingliu.ttc", 0 };
	const char **list = cp==949 ? s_Korean : cp==932 ? s_Japanese : cp==936 ? s_Simplified : cp==950 ? s_Traditional : 0;
	for ( int i=0; list && list[i]; i++ ) {
		PtFontFile *f = LoadFontFile( list[i] );
		if ( f ) return f;
	}
	return NULL;
}

static UINT CharsetToCodePage( BYTE cs )
{
	switch ( cs ) {
	case SHIFTJIS_CHARSET:		return 932;
	case HANGEUL_CHARSET:		return 949;
	case GB2312_CHARSET:		return 936;
	case CHINESEBIG5_CHARSET:	return 950;
	case THAI_CHARSET:			return 874;
	case VIETNAMESE_CHARSET:	return 1258;
	}
	return 1252;
}

static PtFont *GetFont( HDC hdc )
{
	HFONT hFont = (HFONT)GetCurrentObject( hdc, OBJ_FONT );
	LOGFONTA lf;
	if ( !hFont || GetObjectA( hFont, sizeof(lf), &lf )!=sizeof(lf) ) return NULL;

	UINT codePage = g_PtTextCodePage ? g_PtTextCodePage : CharsetToCodePage( lf.lfCharSet );
	char key[128];
	sprintf( key, "%d|%d|%u|%s", lf.lfHeight, lf.lfWeight, codePage, lf.lfFaceName );
	std::map<std::string, PtFont *>::iterator it = s_Fonts.find( key );
	if ( it!=s_Fonts.end() ) return it->second;

	BOOL bBold = lf.lfWeight>=FW_SEMIBOLD;
	const char *szFile, *szFallback;
	FaceToFiles( lf.lfFaceName, bBold, &szFile, &szFallback );
	PtFontFile *pFile = LoadFontFile( szFile );
	int boldExtra = 0;
	if ( !pFile && szFallback ) { pFile = LoadFontFile( szFallback ); boldExtra = 1; }
	if ( !pFile ) { pFile = LoadFontFile( bBold ? "tahomabd.ttf" : "tahoma.ttf" ); boldExtra = 0; }		// face not installed
	if ( !pFile && bBold ) { pFile = LoadFontFile( "tahoma.ttf" ); boldExtra = 1; }

	PtFont *f = NULL;
	if ( pFile ) {
		f = new PtFont;
		f->pFile = pFile;
		f->boldExtra = boldExtra;
		f->codePage = codePage;
		int lfh = lf.lfHeight ? lf.lfHeight : 12;
		// GDI: positive height = cell height (win ascent + descent), negative = em height.
		f->scale = lfh>0 ? (float)lfh / ( pFile->winAscent + pFile->winDescent ) : (float)-lfh / pFile->unitsPerEm;
		f->ascent = (int)floorf( pFile->winAscent * f->scale + 0.5f );
		f->descent = (int)floorf( pFile->winDescent * f->scale + 0.5f );
		f->height = f->ascent + f->descent;
		f->pExtra = ExtraFont( f->codePage );
		f->scaleExtra = f->pExtra ? (float)f->height / ( f->pExtra->winAscent + f->pExtra->winDescent ) : 0;
	}
	s_Fonts[key] = f;
	return f;
}

static PtGlyph *GetGlyph( PtFont *f, int cp )
{
	std::map<int, PtGlyph *>::iterator it = f->glyphs.find( cp );
	if ( it!=f->glyphs.end() ) return it->second;

	PtFontFile *pFile = f->pFile;
	float scale = f->scale;
	int g = stbtt_FindGlyphIndex( &pFile->info, cp );
	if ( !g && f->pExtra ) {
		int g2 = stbtt_FindGlyphIndex( &f->pExtra->info, cp );
		if ( g2 ) { pFile = f->pExtra; scale = f->scaleExtra; g = g2; }
	}

	PtGlyph *gl = new PtGlyph;
	int adv, lsb;
	stbtt_GetGlyphHMetrics( &pFile->info, g, &adv, &lsb );
	gl->advance = (int)floorf( adv * scale + 0.5f ) + f->boldExtra;
	gl->w = gl->h = gl->xoff = gl->yoff = 0;
	int x0, y0, x1, y1;
	stbtt_GetGlyphBitmapBox( &pFile->info, g, scale, scale, &x0, &y0, &x1, &y1 );
	if ( x1>x0 && y1>y0 ) {
		gl->w = x1-x0; gl->h = y1-y0; gl->xoff = x0; gl->yoff = y0;
		gl->bits.resize( gl->w * gl->h );
		stbtt_MakeGlyphBitmap( &pFile->info, &gl->bits[0], gl->w, gl->h, gl->w, scale, scale, g );
	}
	f->glyphs[cp] = gl;
	return gl;
}

// cp1252 0x80-0x9F
static const unsigned short s_Cp1252[32] = {
	0x20AC,0x0081,0x201A,0x0192,0x201E,0x2026,0x2020,0x2021,0x02C6,0x2030,0x0160,0x2039,0x0152,0x008D,0x017D,0x008F,
	0x0090,0x2018,0x2019,0x201C,0x201D,0x2022,0x2013,0x2014,0x02DC,0x2122,0x0161,0x203A,0x0153,0x009D,0x017E,0x0178 };

static void Decode( PtFont *f, const char *s, int n, std::vector<int> &out )
{
	out.clear();
	if ( n<0 ) n = (int)strlen( s );
	if ( f->codePage==1252 ) {
		for ( int i=0; i<n; i++ ) {
			unsigned char c = (unsigned char)s[i];
			out.push_back( c>=0x80 && c<0xA0 ? s_Cp1252[c-0x80] : c );
		}
		return;
	}
	if ( n==0 ) return;
	std::vector<WCHAR> w( n );
	int nw = MultiByteToWideChar( f->codePage, 0, s, n, &w[0], n );
	for ( int i=0; i<nw; i++ ) out.push_back( w[i] );
}

static int MeasureCodepoints( PtFont *f, const int *cps, int n )
{
	int cx = 0;
	for ( int i=0; i<n; i++ ) cx += GetGlyph( f, cps[i] )->advance;
	return cx;
}

//////////////////////////////////////////////////////////////////////////////
// Drawing into a DC's DIB section
//////////////////////////////////////////////////////////////////////////////
struct PtTarget {
	DWORD *bits;
	int w, h;
	BOOL bTopDown;
	BOOL bOverlay;		// the back-buffer text overlay: coverage goes into the alpha byte
	RECT clip;
	DWORD color;		// BGR
	RECT drawn;
};

static BOOL GetTarget( HDC hdc, PtTarget &t )
{
	HBITMAP hbm = (HBITMAP)GetCurrentObject( hdc, OBJ_BITMAP );
	DIBSECTION ds;
	if ( !hbm || GetObjectA( hbm, sizeof(ds), &ds )!=sizeof(ds) ) return FALSE;
	if ( ds.dsBm.bmBitsPixel!=32 || !ds.dsBm.bmBits ) return FALSE;
	t.bits = (DWORD *)ds.dsBm.bmBits;
	t.w = ds.dsBm.bmWidth;
	t.h = ds.dsBm.bmHeight;
	// GetObject reports dsBmih.biHeight positive even for top-down DIBs. The only DIB
	// sections in the process are ptgfx's (CreateDIB32, top-down); the game's own
	// bitmaps are compatible bitmaps and take the GDI fallback above.
	t.bTopDown = TRUE;
	t.bOverlay = hdc==g_PtGL.hOverlayDC;
	if ( GetClipBox( hdc, &t.clip )==ERROR ) SetRect( &t.clip, 0, 0, t.w, t.h );
	if ( t.clip.left<0 ) t.clip.left = 0;
	if ( t.clip.top<0 ) t.clip.top = 0;
	if ( t.clip.right>t.w ) t.clip.right = t.w;
	if ( t.clip.bottom>t.h ) t.clip.bottom = t.h;
	COLORREF c = GetTextColor( hdc );
	t.color = ( GetRValue(c)<<16 ) | ( GetGValue(c)<<8 ) | GetBValue(c);
	SetRect( &t.drawn, 0, 0, 0, 0 );
	GdiFlush();			// GDI may still be writing to the bitmap
	return TRUE;
}

static inline void PutPixel( PtTarget &t, int x, int y, int a )
{
	DWORD *p = t.bits + ( t.bTopDown ? y : t.h-1-y ) * t.w + x;
	if ( a>=255 && !t.bOverlay ) { *p = ( *p & 0xFF000000 ) | t.color; return; }
	DWORD old = *p;
	if ( !t.bOverlay ) {
		int r = ( ( ( old>>16 )&0xFF )*( 255-a ) + ( ( t.color>>16 )&0xFF )*a ) / 255;
		int g = ( ( ( old>>8 )&0xFF )*( 255-a ) + ( ( t.color>>8 )&0xFF )*a ) / 255;
		int b = ( ( old&0xFF )*( 255-a ) + ( t.color&0xFF )*a ) / 255;
		*p = ( old & 0xFF000000 ) | ( r<<16 ) | ( g<<8 ) | b;
		return;
	}
	// Overlay: alpha byte 0 = key colour (empty) or an opaque GDI pixel.
	int oa = old>>24;
	if ( oa==0 ) oa = ( old & 0xFFFFFF )==PT_OVERLAY_KEY ? 0 : 255;
	int na = a + oa*( 255-a )/255;
	if ( na<=0 ) return;
	int ow = oa*( 255-a )/255;
	int r = ( ( ( t.color>>16 )&0xFF )*a + ( ( old>>16 )&0xFF )*ow ) / na;
	int g = ( ( ( t.color>>8 )&0xFF )*a + ( ( old>>8 )&0xFF )*ow ) / na;
	int b = ( ( t.color&0xFF )*a + ( old&0xFF )*ow ) / na;
	*p = ( na<<24 ) | ( r<<16 ) | ( g<<8 ) | b;
}

static void DrawGlyph( PtTarget &t, PtGlyph *gl, int px, int by )
{
	int x0 = px + gl->xoff, y0 = by + gl->yoff;
	for ( int y=0; y<gl->h; y++ ) {
		int dy = y0+y;
		if ( dy<t.clip.top || dy>=t.clip.bottom ) continue;
		const unsigned char *row = &gl->bits[y*gl->w];
		for ( int x=0; x<gl->w; x++ ) {
			int dx = x0+x;
			if ( dx<t.clip.left || dx>=t.clip.right || !row[x] ) continue;
			PutPixel( t, dx, dy, row[x] );
		}
	}
	RECT r = { x0, y0, x0+gl->w, y0+gl->h };
	if ( IsRectEmpty( &t.drawn ) ) t.drawn = r; else UnionRect( &t.drawn, &t.drawn, &r );
}

// Draws a run with its top-left (cell) corner at x,y.
static void DrawRun( PtTarget &t, PtFont *f, const int *cps, int n, int x, int y )
{
	int by = y + f->ascent;
	for ( int i=0; i<n; i++ ) {
		PtGlyph *gl = GetGlyph( f, cps[i] );
		if ( gl->w ) {
			DrawGlyph( t, gl, x, by );
			if ( f->boldExtra ) DrawGlyph( t, gl, x+1, by );
		}
		x += gl->advance;
	}
}

static void FinishTarget( HDC hdc, PtTarget &t )
{
	if ( IsRectEmpty( &t.drawn ) ) return;
	// keep the overlay's dirty rectangle (GetBoundsRect) up to date
	SetBoundsRect( hdc, &t.drawn, DCB_ACCUMULATE );
}

//////////////////////////////////////////////////////////////////////////////
// The replacement functions
//////////////////////////////////////////////////////////////////////////////
typedef BOOL (WINAPI *PFN_TextOutA)( HDC, int, int, LPCSTR, int );
typedef BOOL (WINAPI *PFN_GetTextExtentPoint32A)( HDC, LPCSTR, int, LPSIZE );
typedef BOOL (WINAPI *PFN_GetTextMetricsA)( HDC, LPTEXTMETRICA );
typedef int  (WINAPI *PFN_DrawTextA)( HDC, LPCSTR, int, LPRECT, UINT );

#ifdef PT_WEB
// Browser: these ARE the GDI text functions (ptwin provides the rest of GDI).
// Without a font file nothing can be drawn; measuring still answers.
#define PtText_TextOutA					TextOutA
#define PtText_GetTextExtentPoint32A	GetTextExtentPoint32A
#define PtText_GetTextMetricsA			GetTextMetricsA
#define PtText_DrawTextA				DrawTextA
static BOOL WINAPI NoTextOut( HDC, int, int, LPCSTR, int ) { return FALSE; }
static BOOL WINAPI NoExtent( HDC, LPCSTR, int c, LPSIZE p ) { if ( p ) { p->cx = c*6; p->cy = 12; } return TRUE; }
static BOOL WINAPI NoMetrics( HDC, LPTEXTMETRICA tm ) { ZeroMemory( tm, sizeof(*tm) ); tm->tmHeight = 12; tm->tmAscent = 10; tm->tmDescent = 2; tm->tmAveCharWidth = 6; tm->tmMaxCharWidth = 12; tm->tmWeight = FW_NORMAL; return TRUE; }
static int WINAPI NoDrawText( HDC, LPCSTR, int, LPRECT, UINT ) { return 0; }
static PFN_TextOutA					Real_TextOutA = NoTextOut;
static PFN_GetTextExtentPoint32A	Real_GetTextExtentPoint32A = NoExtent;
static PFN_GetTextMetricsA			Real_GetTextMetricsA = NoMetrics;
static PFN_DrawTextA				Real_DrawTextA = NoDrawText;
BOOL WINAPI GetTextExtentPointA( HDC hdc, LPCSTR s, int c, LPSIZE p );
#else
static PFN_TextOutA					Real_TextOutA = TextOutA;
static PFN_GetTextExtentPoint32A	Real_GetTextExtentPoint32A = GetTextExtentPoint32A;
static PFN_GetTextExtentPoint32A	Real_GetTextExtentPointA = GetTextExtentPointA;
static PFN_GetTextMetricsA			Real_GetTextMetricsA = GetTextMetricsA;
static PFN_DrawTextA				Real_DrawTextA = DrawTextA;
#endif

BOOL WINAPI PtText_TextOutA( HDC hdc, int x, int y, LPCSTR lpString, int c )
{
	PtFont *f;
	PtTarget t;
	if ( !lpString || !( f = GetFont( hdc ) ) || !GetTarget( hdc, t ) ) return Real_TextOutA( hdc, x, y, lpString, c );

	std::vector<int> cps;
	Decode( f, lpString, c, cps );
	int n = (int)cps.size();
	int cx = MeasureCodepoints( f, n ? &cps[0] : NULL, n );

	UINT align = GetTextAlign( hdc );
	if ( ( align & TA_CENTER )==TA_CENTER ) x -= cx/2;
	else if ( align & TA_RIGHT ) x -= cx;
	if ( ( align & TA_BASELINE )==TA_BASELINE ) y -= f->ascent;
	else if ( align & TA_BOTTOM ) y -= f->height;

	if ( GetBkMode( hdc )==OPAQUE ) {
		COLORREF bk = GetBkColor( hdc );
		DWORD save = t.color;
		t.color = ( GetRValue(bk)<<16 ) | ( GetGValue(bk)<<8 ) | GetBValue(bk);
		for ( int yy=y; yy<y+f->height; yy++ )
			for ( int xx=x; xx<x+cx; xx++ )
				if ( xx>=t.clip.left && xx<t.clip.right && yy>=t.clip.top && yy<t.clip.bottom ) PutPixel( t, xx, yy, 255 );
		RECT r = { x, y, x+cx, y+f->height };
		if ( IsRectEmpty( &t.drawn ) ) t.drawn = r; else UnionRect( &t.drawn, &t.drawn, &r );
		t.color = save;
	}
	if ( n ) DrawRun( t, f, &cps[0], n, x, y );
	FinishTarget( hdc, t );
	return TRUE;
}

BOOL WINAPI PtText_GetTextExtentPoint32A( HDC hdc, LPCSTR lpString, int c, LPSIZE psizl )
{
	PtFont *f;
	if ( !lpString || !psizl || !( f = GetFont( hdc ) ) ) return Real_GetTextExtentPoint32A( hdc, lpString, c, psizl );
	std::vector<int> cps;
	Decode( f, lpString, c, cps );
	psizl->cx = MeasureCodepoints( f, cps.empty() ? NULL : &cps[0], (int)cps.size() );
	psizl->cy = f->height;
	return TRUE;
}

BOOL WINAPI PtText_GetTextMetricsA( HDC hdc, LPTEXTMETRICA lptm )
{
	BOOL bOK = Real_GetTextMetricsA( hdc, lptm );
	PtFont *f = GetFont( hdc );
	if ( !bOK || !f ) return bOK;
	lptm->tmHeight = f->height;
	lptm->tmAscent = f->ascent;
	lptm->tmDescent = f->descent;
	lptm->tmInternalLeading = f->height - (int)floorf( f->pFile->unitsPerEm * f->scale + 0.5f );
	if ( lptm->tmInternalLeading<0 ) lptm->tmInternalLeading = 0;
	lptm->tmExternalLeading = 0;
	lptm->tmAveCharWidth = GetGlyph( f, 'x' )->advance;
	int maxw = 0;
	for ( int ch=32; ch<127; ch++ ) { int a = GetGlyph( f, ch )->advance; if ( a>maxw ) maxw = a; }
	lptm->tmMaxCharWidth = maxw;
	return TRUE;
}

// DrawText: the game uses DT_LEFT / DT_RIGHT / DT_WORDBREAK; the common rest is here too.
int WINAPI PtText_DrawTextA( HDC hdc, LPCSTR lpchText, int cchText, LPRECT lprc, UINT format )
{
	PtFont *f;
	PtTarget t;
	BOOL bCalc = ( format & DT_CALCRECT )!=0;
	if ( !lpchText || !lprc || !( f = GetFont( hdc ) ) ) return Real_DrawTextA( hdc, lpchText, cchText, lprc, format );
	BOOL bDraw = !bCalc;
	if ( bDraw && !GetTarget( hdc, t ) ) return Real_DrawTextA( hdc, lpchText, cchText, lprc, format );

	std::vector<int> cps;
	Decode( f, lpchText, cchText, cps );
	int n = (int)cps.size();
	int width = lprc->right - lprc->left;

	// break into lines
	std::vector<int> starts, lens;
	int i = 0;
	while ( i<n || ( i==0 && n==0 ) ) {
		if ( n==0 ) { starts.push_back( 0 ); lens.push_back( 0 ); break; }
		int lineStart = i, lineEnd = i, lastBreak = -1, cx = 0;
		while ( lineEnd<n ) {
			int ch = cps[lineEnd];
			if ( !( format & DT_SINGLELINE ) && ( ch=='\n' || ch=='\r' ) ) break;
			int adv = GetGlyph( f, ch )->advance;
			if ( ( format & DT_WORDBREAK ) && cx+adv>width && lineEnd>lineStart ) {
				if ( lastBreak>lineStart ) lineEnd = lastBreak;
				break;
			}
			if ( ch==' ' ) lastBreak = lineEnd;
			cx += adv;
			lineEnd++;
		}
		int len = lineEnd - lineStart;
		while ( len>0 && ( format & DT_WORDBREAK ) && cps[lineStart+len-1]==' ' ) len--;
		starts.push_back( lineStart ); lens.push_back( len );
		i = lineEnd;
		if ( i<n && cps[i]=='\r' ) i++;
		if ( i<n && cps[i]=='\n' ) i++;
		else if ( i<n && cps[i]==' ' && ( format & DT_WORDBREAK ) ) i++;
		if ( format & DT_SINGLELINE ) break;
	}

	int lines = (int)starts.size();
	int totalH = lines * f->height;
	if ( bCalc ) {
		int maxw = 0;
		for ( int l=0; l<lines; l++ ) { int w = MeasureCodepoints( f, lens[l] ? &cps[starts[l]] : NULL, lens[l] ); if ( w>maxw ) maxw = w; }
		if ( !( format & DT_WORDBREAK ) || maxw<width ) lprc->right = lprc->left + maxw;
		lprc->bottom = lprc->top + totalH;
		return totalH;
	}

	if ( !( format & DT_NOCLIP ) ) {
		RECT c;
		IntersectRect( &c, &t.clip, lprc );
		t.clip = c;
	}
	int y = lprc->top;
	if ( format & DT_SINGLELINE ) {
		if ( format & DT_VCENTER ) y = ( lprc->top + lprc->bottom - f->height ) / 2;
		else if ( format & DT_BOTTOM ) y = lprc->bottom - f->height;
	}
	for ( int l=0; l<lines; l++, y += f->height ) {
		const int *p = lens[l] ? &cps[starts[l]] : NULL;
		int w = MeasureCodepoints( f, p, lens[l] );
		int x = lprc->left;
		if ( format & DT_CENTER ) x = ( lprc->left + lprc->right - w ) / 2;
		else if ( format & DT_RIGHT ) x = lprc->right - w;
		if ( p ) DrawRun( t, f, p, lens[l], x, y );
	}
	FinishTarget( hdc, t );
	return totalH;
}

//////////////////////////////////////////////////////////////////////////////
// Import patching (Windows): game.exe's calls to the GDI/USER text functions
// land in the functions above.
//////////////////////////////////////////////////////////////////////////////
#ifdef PT_WEB
BOOL WINAPI GetTextExtentPointA( HDC hdc, LPCSTR s, int c, LPSIZE p ) { return GetTextExtentPoint32A( hdc, s, c, p ); }
void PtText_Install() {}
#else
static void PatchImports( HMODULE hModule )
{
	struct { const char *dll, *name; void *pNew; void **ppReal; } hooks[] = {
		{ "gdi32.dll",  "TextOutA",              (void *)PtText_TextOutA,              (void **)&Real_TextOutA },
		{ "gdi32.dll",  "GetTextExtentPoint32A", (void *)PtText_GetTextExtentPoint32A, (void **)&Real_GetTextExtentPoint32A },
		{ "gdi32.dll",  "GetTextExtentPointA",   (void *)PtText_GetTextExtentPoint32A, (void **)&Real_GetTextExtentPointA },
		{ "gdi32.dll",  "GetTextMetricsA",       (void *)PtText_GetTextMetricsA,       (void **)&Real_GetTextMetricsA },
		{ "user32.dll", "DrawTextA",             (void *)PtText_DrawTextA,             (void **)&Real_DrawTextA },
	};
	BYTE *base = (BYTE *)hModule;
	IMAGE_NT_HEADERS *nt = (IMAGE_NT_HEADERS *)( base + ((IMAGE_DOS_HEADER *)base)->e_lfanew );
	IMAGE_DATA_DIRECTORY dir = nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
	if ( !dir.VirtualAddress ) return;
	int nPatched = 0;
	for ( IMAGE_IMPORT_DESCRIPTOR *imp = (IMAGE_IMPORT_DESCRIPTOR *)( base + dir.VirtualAddress ); imp->Name; imp++ ) {
		const char *dll = (const char *)( base + imp->Name );
		if ( !imp->OriginalFirstThunk ) continue;
		IMAGE_THUNK_DATA *names = (IMAGE_THUNK_DATA *)( base + imp->OriginalFirstThunk );
		IMAGE_THUNK_DATA *iat = (IMAGE_THUNK_DATA *)( base + imp->FirstThunk );
		for ( ; names->u1.AddressOfData; names++, iat++ ) {
			if ( IMAGE_SNAP_BY_ORDINAL( names->u1.Ordinal ) ) continue;
			const char *fn = (const char *)((IMAGE_IMPORT_BY_NAME *)( base + names->u1.AddressOfData ))->Name;
			for ( int h=0; h<sizeof(hooks)/sizeof(hooks[0]); h++ ) {
				if ( _stricmp( dll, hooks[h].dll ) || strcmp( fn, hooks[h].name ) ) continue;
				*hooks[h].ppReal = (void *)iat->u1.Function;
				DWORD old;
				VirtualProtect( &iat->u1.Function, sizeof(void *), PAGE_READWRITE, &old );
				iat->u1.Function = (DWORD_PTR)hooks[h].pNew;
				VirtualProtect( &iat->u1.Function, sizeof(void *), old, &old );
				nPatched++;
			}
		}
	}
	PtGL_Log( "text: own rasterizer installed (%d imports patched)", nPatched );
}

void PtText_Install()
{
	static BOOL bDone = FALSE;
	if ( bDone ) return;
	bDone = TRUE;
	PatchImports( GetModuleHandleA( NULL ) );
}
#endif // !PT_WEB
