/*----------------------------------------------------------------------------*
*	파일명 :  sinMsg.cpp
*	하는일 :  다른언어의 컨버젼을 위해 메세지를 관리한다 
*	작성일 :  최종업데이트 4월
*	적성자 :  박상열 
*-----------------------------------------------------------------------------*/	

#include "sinLinkHeader.h"


#include "..\\Language.h"

#ifdef	_LANGUAGE_KOREAN
#include "..\\Korean\\K_sinMsg.h"
#endif

#ifdef	_LANGUAGE_CHINESE
#include "..\\chinese\\C_sinMsg.h"
#endif

#ifdef	_LANGUAGE_JAPANESE
#include "..\\Japanese\\J_sinMsg.h"
#endif

#ifdef	_LANGUAGE_TAIWAN
#include "..\\Taiwan\\T_sinMsg.h"
#endif

#ifdef	_LANGUAGE_ENGLISH
#include "..\\English\\E_sinMsg.h"
#endif

#ifdef	_LANGUAGE_THAI
#include "..\\THAI\\TH_sinMsg.h"
#endif

#ifdef _LANGUAGE_VEITNAM
#include "..\\VEITNAM\\V_sinMsg.h"
#endif

#ifdef _LANGUAGE_BRAZIL
#include "..\\BRAZIL\\B_sinMsg.h"
#endif

#ifdef _LANGUAGE_ARGENTINA
#include "..\\ARGENTINA\\A_sinMsg.h"
#endif