// pt_lang.h - the language setting: the game's built-in text in any of the 9
// languages the source has, switched while the game runs (lang\pt_lang.cpp).
//
// What switches: the text in the language folders (English\, Korean\, Japanese\ ...):
// messages, menus, skill names and descriptions, crafting descriptions, quest text,
// class names on the character screen. What stays English: names and text from the
// game data (monsters, items, NPC dialogue), the server's messages, and text written
// directly in the code.

#ifndef PT_LANG_H
#define PT_LANG_H

// PtLang_Set's numbers, also the order of the page's language list (web\settings.js)
enum {
	PT_LANG_ENGLISH, PT_LANG_KOREAN, PT_LANG_JAPANESE, PT_LANG_CHINESE, PT_LANG_TAIWAN,
	PT_LANG_THAI, PT_LANG_VIETNAMESE, PT_LANG_BRAZIL, PT_LANG_ARGENTINA, PT_LANG_COUNT
};

int PtLang_Set( int lang );			// TRUE if the language changed
int PtLang_Get();

#endif
