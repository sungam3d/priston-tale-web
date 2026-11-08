// pt_lang_tu.h - included first by the generated lang\lang_*.cpp (web\gen_lang.py).
//
// Each lang_XX.cpp compiles one language's text headers inside namespace PtLang_XX,
// after the game's own headers, so it gets the same types as the game and its own
// copy of every text variable. It then lists those variables as PtLangEntry: name,
// address, and what they are (from the C++ type, so sizes are always right).
// lang_EN.cpp also lists the game's live variables, which pt_lang.cpp copies into.

#ifndef PT_LANG_TU_H
#define PT_LANG_TU_H

#include "..\\sinbaram\\sinLinkHeader.h"
#include "..\\fileread.h"
#include "..\\HoBaram\\HoTextFile.h"
#include "..\\TextMessage.h"
#include "pt_lang.h"

enum {
	PT_LANGK_PTR = 1,		// char *     (count: how many, for arrays of them)
	PT_LANGK_CHARS,			// char[width] (count: how many)
	PT_LANGK_MSGBOX,		// sMESSAGEBOX[count]
	PT_LANGK_MSGBOX2,		// sMESSAGEBOX2[count]
	PT_LANGK_MSGBOX3,		// sMESSAGEBOX3[count]
};

// A text variable's kind from its type. A type without a match here is a compile
// error in lang_*.cpp: add it here and in pt_lang.cpp CopyEntry.
template<class T> struct PtLangKind;
template<> struct PtLangKind<char *> { enum { kind = PT_LANGK_PTR, count = 1, width = 0 }; };
template<int N> struct PtLangKind<char *[N]> { enum { kind = PT_LANGK_PTR, count = N, width = 0 }; };
template<int N, int M> struct PtLangKind<char *[N][M]> { enum { kind = PT_LANGK_PTR, count = N*M, width = 0 }; };
template<int N> struct PtLangKind<char[N]> { enum { kind = PT_LANGK_CHARS, count = 1, width = N }; };
template<int N, int M> struct PtLangKind<char[N][M]> { enum { kind = PT_LANGK_CHARS, count = N, width = M }; };
template<int N> struct PtLangKind<sMESSAGEBOX[N]> { enum { kind = PT_LANGK_MSGBOX, count = N, width = 0 }; };
template<int N> struct PtLangKind<sMESSAGEBOX2[N]> { enum { kind = PT_LANGK_MSGBOX2, count = N, width = 0 }; };
template<int N> struct PtLangKind<sMESSAGEBOX3[N]> { enum { kind = PT_LANGK_MSGBOX3, count = N, width = 0 }; };

struct PtLangEntry {
	const char *name;
	void *addr;
	int kind, count, width;
};

#define PT_LANG_ENTRY( ns, v ) \
	{ #v, (void *)&ns::v, PtLangKind<decltype(ns::v)>::kind, PtLangKind<decltype(ns::v)>::count, PtLangKind<decltype(ns::v)>::width }

// One language: its text variables and its tables that mix text with game numbers
// (only their text is ever copied; rows are matched by code, never by position).
struct PtLangTable {
	const char *code;
	unsigned codePage;			// Windows code page its text is written in
	const PtLangEntry *entries;
	int nEntries;
	sSKILL_INFO *skills;		// SIN_MAX_SKILL rows
	sCRAFTITEM_INFO *crafts;	// MAX_CRAFTITEM_INFO rows
	JOB_DATA_CODE *jobs;		// up to a row with an empty name
};

#endif
