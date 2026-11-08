#include "..\\HoBaram\\HoTextFile.h"

#include "..\\language.h"

#ifdef	_LANGUAGE_KOREAN
#include "..\\Korean\\K_HoTextFile.h"
#endif

#ifdef	_LANGUAGE_CHINESE
#include "..\\Chinese\\C_HoTextFile.h"
#endif

#ifdef	_LANGUAGE_JAPANESE
#include "..\\Japanese\\J_HoTextFile.h"
#endif

#ifdef	_LANGUAGE_TAIWAN
#include "..\\Taiwan\\T_HoTextFile.h"
#endif

#ifdef	_LANGUAGE_ENGLISH
#include "..\\English\\E_HoTextFile.h"
#endif

#ifdef	_LANGUAGE_THAI
#include "..\\THAI\\TH_HoTextFile.h"
#endif

#ifdef _LANGUAGE_VEITNAM
#include "..\\VEITNAM\\V_HoTextFile.h"
#endif

#ifdef _LANGUAGE_BRAZIL
#include "..\\BRAZIL\\B_HoTextFile.h"
#endif

#ifdef _LANGUAGE_ARGENTINA
#include "..\\ARGENTINA\\A_HoTextFile.h"
#endif