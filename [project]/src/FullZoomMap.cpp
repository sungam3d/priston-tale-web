//######################################################################################
//작 성 일 : 2004-04-08 오후 2:46:59
//리 턴 값 :
//설    명 :
//작 성 자 : 오 영 석
//######################################################################################
#include "smlib3d\\smd3d.h"
#include "smwsock.h"
#include "character.h"
#include "playmain.h"
#include "field.h"
#include "FullZoomMap.h"
#include "sinbaram\\sinlinkheader.h"

//######################################################################################
//
#define DMAX_fzmcp_DRAW_TYPE			2
#define DMAX_fzm_CHAR_POS				6

#define dMAX_FZM_EFFECT_SUBNUM			10
#define dMAX_NAME_EFFECT				20
#define	dMAX_FZM_CHAR_POS				6


//######################################################################################
//
enum EFzmcpDrawType
{
	EFZMCP_LEFT_TOP		= 0,	// 좌측상단.
	EFZMCP_LEFT_BOTTOM	= 1,	// 좌측하단.
};

//
enum EFzmNameType
{                       
	EPilait			=  1,	// 필라이.
	Efallfore_1		=  2,	// 바람이 이끄는 길.
	Efallfore_2		=  3,	// 고요의 계곡.
	Efallfore_3		=  4,	// 황혼의 숲.
	EMcavet			=  5,	// 벌집동굴.
	Efallfore_4		=  6,	// 정령의 숲.
	ERuin_3			=  7,	// 루이넨 빌리지.
	Edunt			=  8,	// 고대 감옥.
	ERuin_4			=  9,	// 저주받은 땅.
	ERuin_2			= 10,	// 폐성곽.
	ERuin_1			= 11,	// 고대인의 안식처.
	ETcavet			= 12,	// 버섯 동굴.
	Efore_3			= 13,	// 아카시아 숲.
	Efore_2			= 14,	// 대나무 숲.
	Efore_1			= 15,	// 자유의 정원.
	ERicaT			= 16,	// 리카르텐.
	Ede_1			= 17,	// 잊혀진 땅.
	Ede_nv			= 18,	// 네비스코.
	Ede_2			= 19,	// 오아시스.
	Ede_3			= 20,	// 고대인의 전장.
	Esint			= 21,	// 저주받은 신전.
	Ede_4			= 22,	// 금단의 땅.
	Eiron_1			= 23,	// 혼돈의 철로.
	EDcavet			= 24,	// 어둠의 성전.
	Eiron_2			= 25,	// 폐룸의 심장.
	Eyura_1			= 26,   // 유라 빌리지.
	IceField_1      = 27,   // 아이스 필드
	Greedy          = 28,   // 그리디 호수
	IceField_2      = 29,   // 아이스 필드 2
	Lost_1			= 30,   // 로스트 아일랜드
	Lost_2			= 31,	// 로스트 템플
	EFzmNameMax     = 31,   // 맥스 인덱스 
};


//
struct SFullZoomMapInfo
{
	EFzmNameType			eNameType;			
	int						iNamePosX;
	int						iNamePosY;	

	EFzmcpDrawType			eFzmcpDrawType;
	int						iFzmCharPosX;
	int						iFzmCharPosY;

	int						iZoomMapIsCharPos;
	float					fRatio;
};

//
struct SFullZoomMapImage
{
	int						iCheckWideh;
	int						iCheckHeight;
	LPDIRECTDRAWSURFACE4	lpDDS_Name;

	int						iMapOffset_x;
	int						iMapOffset_y;
};

//
struct SMapStageInfo
{
	RECT			rcStageMapRect;
	int				iStageWidth;
	int				iStageHeight;
	float			fRatio;
};

//
struct SFullZoomMapEffectInfo
{
	int				MaxFrameNum;
	int				DelayTime[ dMAX_FZM_EFFECT_SUBNUM ];
	BYTE		    BlendNum[ dMAX_FZM_EFFECT_SUBNUM ];
	int				ScaleNum[ dMAX_FZM_EFFECT_SUBNUM ];
};

//
enum EFzmEffectType
{
	NAME_EFFECT				= 1,
	CHAR_POS_EFFECT			= 2,
	CHAR_POS_NAME_EFFECT	= 3,
};

//
struct SFullZoomMapEffect
{
	EFzmEffectType				eEffectType;
	SFullZoomMapEffectInfo	   *lpEffectInfo;

	int				Life;
	int				FrameTime;
	int				FrameNum;
	int				EventNum;
	POINT			Pos;
	int				HalfSize;
	float			fBlendNum;
	float			fBlendStep;
	float			fScaleNum;
	float			fScaleStep;
};

//
enum ECharType
{
	USER_CHAR  = 1,
	PARTY_CHAR = 2,
};

//
struct SFzmCharPos
{
	ECharType		eCharType;
	int				iXpos;
	int				iYpos;
};


//######################################################################################
//
static POINT				s_FzmcpInfo[ DMAX_fzm_CHAR_POS ] = 
{
	{0, 0 }, {10, 0 }, {20, 0 },
	{0, 10}, {10, 10}, {20, 10},
};

//
static int					s_FzmcpIndex[ DMAX_fzmcp_DRAW_TYPE ][ DMAX_fzm_CHAR_POS ] = 
{
	{0,1,2,3,4,5},
	{3,4,5,0,1,2},
};
/*
struct SFullZoomMapInfo
{
	EFzmNameType			eNameType;		
	int						iNamePosX;
	int						iNamePosY;	

	EFzmcpDrawType			eFzmcpDrawType;
	int						iFzmCharPosX;
	int						iFzmCharPosY;

	int						iZoomMapIsCharPos;
	float					fRatio;
};
*/
//
#ifdef _LANGUAGE_KOREAN
static SFullZoomMapInfo		s_FzmInfo[EFzmNameMax] = 
{
	{ EPilait    ,440, 18, EFZMCP_LEFT_TOP   ,440, 32 ,1,508.f },					// 0.  필라이.
	{ Efallfore_1,412, 68, EFZMCP_LEFT_TOP	 ,466, 68 ,1,0 },						// 1.  바람이 이끄는 길.
	{ Efallfore_2,411,143, EFZMCP_LEFT_BOTTOM,411, 122,1,0 },						// 2.  고요의 계곡.
	{ Efallfore_3,480,177, EFZMCP_LEFT_TOP	 ,480, 191,1,0 },						// 3.  황혼의 숲.
	{ EMcavet	 ,397,191, EFZMCP_LEFT_TOP	 ,397, 205,0,0 },						// 4.  벌집동굴.
	{ Efallfore_4,535,216, EFZMCP_LEFT_TOP	 ,535, 230,1,0 },						// 5.  정령의 숲.
	{ ERuin_3	 ,456,271, EFZMCP_LEFT_TOP	 ,493, 271,1,0 },						// 6.  루이넨 빌리지.
	{ Edunt		 ,551,298, EFZMCP_LEFT_BOTTOM,569, 276,0,0 },						// 7.  고대 감옥.
	{ ERuin_4	 ,535,327, EFZMCP_LEFT_TOP	 ,535, 341,1,0 },						// 8.  저주받은 땅.
	{ ERuin_2	 ,461,336, EFZMCP_LEFT_TOP	 ,461, 350,1,0 },						// 9.  폐성곽.
	{ ERuin_1	 ,490,392, EFZMCP_LEFT_TOP	 ,539, 392,1,0 },						// 10. 고대인의 안식처.
	{ ETcavet	 ,362,386, EFZMCP_LEFT_BOTTOM,381, 365,0,0 },						// 11. 버섯 동굴.
	{ Efore_3	 ,358,424, EFZMCP_LEFT_TOP	 ,380, 438,1,0 },						// 12. 아카시아 숲.
	{ Efore_2	 ,413,464, EFZMCP_LEFT_BOTTOM,436, 443,1,0 },						// 13. 대나무 숲.
	{ Efore_1	 ,505,449, EFZMCP_LEFT_TOP	 ,505, 463,1,0 },						// 14. 자유의 정원.
	{ ERicaT	 ,502,513, EFZMCP_LEFT_TOP	 ,502, 527,1,477.f },					// 15. 리카르텐.
	{ Ede_1		 ,568,376, EFZMCP_LEFT_TOP	 ,588, 391,1,0 },						// 16. 잊혀진 땅.
	{ Ede_nv	 ,621,403, EFZMCP_LEFT_BOTTOM,640, 382,1,0 },						// 17. 네비스코.
	{ Ede_2		 ,636,460, EFZMCP_LEFT_TOP	 ,636, 474,1,0 },						// 18. 오아시스.
	{ Ede_3		 ,678,372, EFZMCP_LEFT_TOP	 ,678, 386,1,0 },						// 19. 고대인의 전장.
	{ Esint		 ,682,417, EFZMCP_LEFT_TOP	 ,733, 417,0,0 },						// 20. 저주받은 신전.
	{ Ede_4		 ,655,314, EFZMCP_LEFT_TOP	 ,710, 314,1,0 },						// 21. 금단의 땅.
	{ Eiron_1	 ,746,249, EFZMCP_LEFT_TOP	 ,752, 277,1,0 },						// 22. 혼돈의 철로.
	{ EDcavet	 ,367,296, EFZMCP_LEFT_TOP	 ,376, 325,1,0 },						// 23. 어둠의 성전.
	{ Eiron_2	 ,653,248, EFZMCP_LEFT_TOP	 ,663, 270,1,352.f },					// 24. 폐룸의 심장.
	{ Eyura_1    ,588,178, EFZMCP_LEFT_TOP	 ,598, 205,1,378.f },					// 25. 유라 빌리지.
	{ IceField_1 ,655,166, EFZMCP_LEFT_TOP	 ,659, 182,1,0 },					    // 26. 아이스 필드.
	{ Greedy     ,565,267, EFZMCP_LEFT_TOP	 ,565, 267,1,0 },					    // 27. 그리디 호수.
	{ IceField_2 ,712,97 , EFZMCP_LEFT_TOP	 ,712, 97 ,1,0 },					    // 28. 아이스 필드2.
	{ Lost_1	 ,595,530, EFZMCP_LEFT_BOTTOM,596, 533,1,0 },					    // 29. 로스트 아일랜드
	{ Lost_2	 ,578,477, EFZMCP_LEFT_TOP	 ,578, 477,1,0 },					    // 30. 로스트 템플 

};
#endif

#ifdef _LANGUAGE_ENGLISH
#ifndef _LANGUAGE_PHILIPIN
//여기가 영문이셈
static SFullZoomMapInfo		s_FzmInfo[ EFzmNameMax ] = 
{
		{ EPilait, 438, 21, EFZMCP_LEFT_TOP, 440, 32, 1, 508.f },						// 0.  필라이.
		{ Efallfore_1, 399, 84, EFZMCP_LEFT_TOP, 466, 68, 1, 0 },						// 1.  바람이 이끄는 길.
		{ Efallfore_2, 396, 127, EFZMCP_LEFT_BOTTOM, 411, 122, 1, 0 },					// 2.  고요의 계곡.
		{ Efallfore_3, 487, 174, EFZMCP_LEFT_TOP, 480, 191, 1, 0 },						// 3.  황혼의 숲.
		{ EMcavet, 366, 191, EFZMCP_LEFT_TOP, 397, 205, 0, 0 },							// 4.  벌집동굴.
		{ Efallfore_4, 533, 215, EFZMCP_LEFT_TOP, 535, 230, 1, 0 },						// 5.  정령의 숲.
		{ ERuin_3, 451, 265, EFZMCP_LEFT_TOP, 493, 271, 1, 0 },							// 6.  루이넨 빌리지.
		{ Edunt, 542, 288, EFZMCP_LEFT_BOTTOM, 569, 276, 0, 0 },						// 7.  고대 감옥.
		{ ERuin_4, 529, 326, EFZMCP_LEFT_TOP, 535, 341, 1, 0 },							// 8.  저주받은 땅.
		{ ERuin_2, 451, 331, EFZMCP_LEFT_TOP, 461, 350, 1, 0 },							// 9.  폐성곽.
		{ ERuin_1, 470, 390, EFZMCP_LEFT_TOP, 539, 392, 1, 0 },							// 10. 고대인의 안식처.
		{ ETcavet, 363, 379, EFZMCP_LEFT_BOTTOM, 381, 365, 0, 0 },						// 11. 버섯 동굴.
		{ Efore_3, 390, 421, EFZMCP_LEFT_TOP, 380, 438, 1, 0 },							// 12. 아카시아 숲.
		{ Efore_2, 420, 460, EFZMCP_LEFT_BOTTOM, 436, 443, 1, 0 },						// 13. 대나무 숲.
		{ Efore_1, 502, 444, EFZMCP_LEFT_TOP, 505, 463, 1, 0 },							// 14. 자유의 정원.
		{ ERicaT, 494, 510, EFZMCP_LEFT_TOP, 502, 527, 1, 477.f },						// 15. 리카르텐.
		{ Ede_1, 559, 371, EFZMCP_LEFT_TOP, 588, 391, 1, 0 },							// 16. 잊혀진 땅.
		{ Ede_nv, 620, 405, EFZMCP_LEFT_BOTTOM, 640, 382, 1, 0 },						// 17. 네비스코.
		{ Ede_2, 642, 462, EFZMCP_LEFT_TOP, 636, 474, 1, 0 },							// 18. 오아시스.
		{ Ede_3, 675, 364, EFZMCP_LEFT_TOP, 678, 386, 1, 0 },							// 19. 고대인의 전장.
		{ Esint, 692, 417, EFZMCP_LEFT_TOP, 733, 417, 0, 0 },							// 20. 저주받은 신전.
		{ Ede_4, 659, 308, EFZMCP_LEFT_TOP, 710, 314, 1, 0 },							// 21. 금단의 땅.
		{ Eiron_1, 742, 237, EFZMCP_LEFT_TOP, 752, 277, 1, 0 },							// 22. 혼돈의 철로.
		{ EDcavet, 358, 297, EFZMCP_LEFT_TOP, 376, 325, 1, 0 },							// 23. 어둠의 성전.
		{ Eiron_2, 627, 246, EFZMCP_LEFT_TOP, 663, 270, 1, 352.f },						// 24. 폐룸의 심장.
		{ Eyura_1, 591, 201, EFZMCP_LEFT_TOP, 598, 205, 1, 378.f },						// 25. 유라 빌리지.
		{ IceField_1 ,650,171, EFZMCP_LEFT_TOP	 ,659, 182,1,0 },					    // 26. 아이스 필드.
		{ Greedy     ,565,268, EFZMCP_LEFT_TOP	 ,565, 267,1,0 },					    // 27. 그리디 호수.
		{ IceField_2 ,725,92, EFZMCP_LEFT_TOP	 ,565, 267,1,0 },					    // 28. 아이스 필드2.
		{ Lost_1	 ,600,533, EFZMCP_LEFT_BOTTOM,596, 533,1,0 },					    // 29. 로스트 아일랜드
		{ Lost_2	 ,597,488, EFZMCP_LEFT_TOP	 ,578, 477,1,0 },					    // 30. 로스트 템플 
};

#else

//여기가 필리핀이셈
static SFullZoomMapInfo		s_FzmInfo[ EFzmNameMax ] = 
{
		{ EPilait, 438, 21, EFZMCP_LEFT_TOP, 440, 32, 1, 508.f },						// 0.  필라이.
		{ Efallfore_1, 399, 84, EFZMCP_LEFT_TOP, 466, 68, 1, 0 },						// 1.  바람이 이끄는 길.
		{ Efallfore_2, 396, 127, EFZMCP_LEFT_BOTTOM, 411, 122, 1, 0 },					// 2.  고요의 계곡.
		{ Efallfore_3, 487, 174, EFZMCP_LEFT_TOP, 480, 191, 1, 0 },						// 3.  황혼의 숲.
		{ EMcavet, 366, 191, EFZMCP_LEFT_TOP, 397, 205, 0, 0 },							// 4.  벌집동굴.
		{ Efallfore_4, 533, 215, EFZMCP_LEFT_TOP, 535, 230, 1, 0 },						// 5.  정령의 숲.
		{ ERuin_3, 451, 265, EFZMCP_LEFT_TOP, 493, 271, 1, 0 },							// 6.  루이넨 빌리지.
		{ Edunt, 544, 288, EFZMCP_LEFT_BOTTOM, 569, 276, 0, 0 },						// 7.  고대 감옥.
		{ ERuin_4, 529, 326, EFZMCP_LEFT_TOP, 535, 341, 1, 0 },							// 8.  저주받은 땅.
		{ ERuin_2, 451, 331, EFZMCP_LEFT_TOP, 461, 350, 1, 0 },							// 9.  폐성곽.
		{ ERuin_1, 470, 390, EFZMCP_LEFT_TOP, 539, 392, 1, 0 },							// 10. 고대인의 안식처.
		{ ETcavet, 363, 379, EFZMCP_LEFT_BOTTOM, 381, 365, 0, 0 },						// 11. 버섯 동굴.
		{ Efore_3, 390, 421, EFZMCP_LEFT_TOP, 380, 438, 1, 0 },							// 12. 아카시아 숲.
		{ Efore_2, 420, 460, EFZMCP_LEFT_BOTTOM, 436, 443, 1, 0 },						// 13. 대나무 숲.
		{ Efore_1, 502, 444, EFZMCP_LEFT_TOP, 505, 463, 1, 0 },							// 14. 자유의 정원.
		{ ERicaT, 494, 510, EFZMCP_LEFT_TOP, 502, 527, 1, 477.f },						// 15. 리카르텐.
		{ Ede_1, 559, 371, EFZMCP_LEFT_TOP, 588, 391, 1, 0 },							// 16. 잊혀진 땅.
		{ Ede_nv, 620, 405, EFZMCP_LEFT_BOTTOM, 640, 382, 1, 0 },						// 17. 네비스코.
		{ Ede_2, 642, 462, EFZMCP_LEFT_TOP, 636, 474, 1, 0 },							// 18. 오아시스.
		{ Ede_3, 675, 364, EFZMCP_LEFT_TOP, 678, 386, 1, 0 },							// 19. 고대인의 전장.
		{ Esint, 692, 417, EFZMCP_LEFT_TOP, 733, 417, 0, 0 },							// 20. 저주받은 신전.
		{ Ede_4, 659, 308, EFZMCP_LEFT_TOP, 710, 314, 1, 0 },							// 21. 금단의 땅.
		{ Eiron_1, 742, 237, EFZMCP_LEFT_TOP, 752, 277, 1, 0 },							// 22. 혼돈의 철로.
		{ EDcavet, 358, 297, EFZMCP_LEFT_TOP, 376, 325, 1, 0 },							// 23. 어둠의 성전.
		{ Eiron_2, 627, 246, EFZMCP_LEFT_TOP, 663, 270, 1, 352.f },						// 24. 폐룸의 심장.
		{ Eyura_1, 591, 201, EFZMCP_LEFT_TOP, 598, 205, 1, 378.f },						// 25. 유라 빌리지.
		{ IceField_1 ,650,171, EFZMCP_LEFT_TOP	 ,659, 182,1,0 },					    // 26. 아이스 필드.
		{ Greedy     ,565,268, EFZMCP_LEFT_TOP	 ,565, 267,1,0 },					    // 27. 그리디 호수.
		{ IceField_2 ,725,92, EFZMCP_LEFT_TOP	 ,712, 97,1,0 },					    // 28. 아이스 필드2.
		{ Lost_1	 ,600,533, EFZMCP_LEFT_BOTTOM,596, 533,1,0 },					    // 29. 로스트 아일랜드
		{ Lost_2	 ,597,488, EFZMCP_LEFT_TOP	 ,578, 477,1,0 },					    // 30. 로스트 템플 
};
#endif
#endif

#ifdef _LANGUAGE_CHINESE
static SFullZoomMapInfo		s_FzmInfo[ EFzmNameMax ] = 
{
		{ EPilait,		447, 20,  EFZMCP_LEFT_TOP,		440, 32, 1, 508.f },				// 0.  필라이.
		{ Efallfore_1,	429, 82,  EFZMCP_LEFT_TOP,		466, 68, 1, 0 },					// 1.  바람이 이끄는 길.
		{ Efallfore_2,	412, 138, EFZMCP_LEFT_BOTTOM,	411, 122, 1, 0 },					// 2.  고요의 계곡.
		{ Efallfore_3,	486, 188, EFZMCP_LEFT_TOP,		480, 191, 1, 0 },					// 3.  황혼의 숲.
		{ EMcavet,		382, 189, EFZMCP_LEFT_TOP,		397, 205, 0, 0 },					// 4.  벌집동굴.
		{ Efallfore_4,	535, 222, EFZMCP_LEFT_TOP,		535, 230, 1, 0 },					// 5.  정령의 숲.
		{ ERuin_3,		441, 269, EFZMCP_LEFT_TOP,		493, 271, 1, 0 },					// 6.  루이넨 빌리지.
		{ Edunt,		535, 292, EFZMCP_LEFT_BOTTOM,	569, 276, 0, 0 },					// 7.  고대 감옥.
		{ ERuin_4,		530, 325, EFZMCP_LEFT_TOP,		535, 341, 1, 0 },					// 8.  저주받은 땅.
		{ ERuin_2,		469, 335, EFZMCP_LEFT_TOP,		461, 350, 1, 0 },					// 9.  폐성곽.
		{ ERuin_1,		470, 394, EFZMCP_LEFT_TOP,		539, 392, 1, 0 },					// 10. 고대인의 안식처.
		{ ETcavet,		364, 385, EFZMCP_LEFT_BOTTOM,	381, 365, 0, 0 },					// 11. 버섯 동굴.
		{ Efore_3,		383, 426, EFZMCP_LEFT_TOP,		380, 438, 1, 0 },					// 12. 아카시아 숲.
		{ Efore_2,		419, 464, EFZMCP_LEFT_BOTTOM,	436, 443, 1, 0 },					// 13. 대나무 숲.
		{ Efore_1,		504, 446, EFZMCP_LEFT_TOP,		505, 463, 1, 0 },					// 14. 자유의 정원.
		{ ERicaT,		496, 510, EFZMCP_LEFT_TOP,		502, 527, 1, 477.f },				// 15. 리카르텐.
		{ Ede_1,		560, 374, EFZMCP_LEFT_TOP,		588, 391, 1, 0 },					// 16. 잊혀진 땅.
		{ Ede_nv,		606, 404, EFZMCP_LEFT_BOTTOM,	640, 382, 1, 0 },					// 17. 네비스코.
		{ Ede_2,		635, 462, EFZMCP_LEFT_TOP,		636, 474, 1, 0 },					// 18. 오아시스.
		{ Ede_3,		678, 367, EFZMCP_LEFT_TOP,		678, 386, 1, 0 },					// 19. 고대인의 전장.
		{ Esint,		691, 420, EFZMCP_LEFT_TOP,		733, 417, 0, 0 },					// 20. 저주받은 신전.
		{ Ede_4,		659, 312, EFZMCP_LEFT_TOP,		710, 314, 1, 0 },					// 21. 금단의 땅.
		{ Eiron_1,		728, 239, EFZMCP_LEFT_TOP,		752, 277, 1, 0 },					// 22. 혼돈의 철로.
		{ EDcavet,		363, 298, EFZMCP_LEFT_TOP,		376, 325, 1, 0 },					// 23. 어둠의 성전.
		{ Eiron_2,		641, 247, EFZMCP_LEFT_TOP,		663, 270, 1, 352.f },				// 24. 폐룸의 심장.
		{ Eyura_1,		562, 194, EFZMCP_LEFT_TOP,		598, 205, 1, 378.f },				// 25. 유라 빌리지.
		{ IceField_1 ,	655, 168, EFZMCP_LEFT_TOP,		659, 182,1,0 },					    // 26. 아이스 필드.
		{ Greedy     ,	568, 263, EFZMCP_LEFT_TOP,		565, 267,1,0 },					    // 27. 그리디 호수.
		{ IceField_2 ,	700, 97 , EFZMCP_LEFT_TOP,		565, 267 ,1,0 },					// 28. 아이스 필드2.
		{ Lost_1	 ,	590,536, EFZMCP_LEFT_BOTTOM,	596, 533,1,0 },					    // 29. 로스트 아일랜드
		{ Lost_2	 ,	574,475, EFZMCP_LEFT_TOP,		578, 477,1,0 },					    // 30. 로스트 템플 
};
#endif

#ifdef	_LANGUAGE_JAPANESE
static SFullZoomMapInfo		s_FzmInfo[ EFzmNameMax ] = 
{
		{ EPilait, 430, 21, EFZMCP_LEFT_TOP, 440, 32, 1, 508.f },						// 0.  필라이.
		{ Efallfore_1, 391, 79, EFZMCP_LEFT_TOP, 466, 68, 1, 0 },						// 1.  바람이 이끄는 길.
		{ Efallfore_2, 394, 130, EFZMCP_LEFT_BOTTOM, 411, 122, 1, 0 },					// 2.  고요의 계곡.
		{ Efallfore_3, 480, 178, EFZMCP_LEFT_TOP, 480, 191, 1, 0 },						// 3.  황혼의 숲.
		{ EMcavet, 384, 185, EFZMCP_LEFT_TOP, 397, 205, 0, 0 },							// 4.  벌집동굴.
		{ Efallfore_4, 530, 215, EFZMCP_LEFT_TOP, 535, 230, 1, 0 },						// 5.  정령의 숲.
		{ ERuin_3, 436, 257, EFZMCP_LEFT_TOP, 493, 271, 1, 0 },							// 6.  루이넨 빌리지.
		{ Edunt, 532, 280, EFZMCP_LEFT_BOTTOM, 569, 276, 0, 0 },						// 7.  고대 감옥.
		{ ERuin_4, 528, 320, EFZMCP_LEFT_TOP, 535, 341, 1, 0 },							// 8.  저주받은 땅.
		{ ERuin_2, 447, 332, EFZMCP_LEFT_TOP, 461, 350, 1, 0 },							// 9.  폐성곽.
		{ ERuin_1, 457, 391, EFZMCP_LEFT_TOP, 539, 392, 1, 0 },							// 10. 고대인의 안식처.
		{ ETcavet, 358, 383, EFZMCP_LEFT_BOTTOM, 381, 365, 0, 0 },						// 11. 버섯 동굴.
		{ Efore_3, 354, 427, EFZMCP_LEFT_TOP, 380, 438, 1, 0 },							// 12. 아카시아 숲.
		{ Efore_2, 420, 461, EFZMCP_LEFT_BOTTOM, 436, 443, 1, 0 },						// 13. 대나무 숲.
		{ Efore_1, 499, 445, EFZMCP_LEFT_TOP, 505, 463, 1, 0 },							// 14. 자유의 정원.
		{ ERicaT, 485, 507, EFZMCP_LEFT_TOP, 502, 527, 1, 477.f },						// 15. 리카르텐.
		{ Ede_1, 558, 365, EFZMCP_LEFT_TOP, 588, 391, 1, 0 },							// 16. 잊혀진 땅.
		{ Ede_nv, 613, 403, EFZMCP_LEFT_BOTTOM, 640, 382, 1, 0 },						// 17. 네비스코.
		{ Ede_2, 628, 460, EFZMCP_LEFT_TOP, 636, 474, 1, 0 },							// 18. 오아시스.
		{ Ede_3, 667, 365, EFZMCP_LEFT_TOP, 678, 386, 1, 0 },							// 19. 고대인의 전장.
		{ Esint, 674, 417, EFZMCP_LEFT_TOP, 733, 417, 0, 0 },							// 20. 저주받은 신전.
		{ Ede_4, 650, 310, EFZMCP_LEFT_TOP, 710, 314, 1, 0 },							// 21. 금단의 땅.
		{ Eiron_1, 715, 236, EFZMCP_LEFT_TOP, 752, 277, 1, 0 },							// 22. 혼돈의 철로.
		{ EDcavet, 362, 298, EFZMCP_LEFT_TOP, 376, 325, 1, 0 },							// 23. 어둠의 성전.
		{ Eiron_2, 620, 263, EFZMCP_LEFT_TOP, 663, 270, 1, 352.f },						// 24. 폐룸의 심장.
		{ Eyura_1, 598, 209, EFZMCP_LEFT_TOP, 598, 205, 1, 378.f },						// 25. 유라 빌리지.
		{ IceField_1 ,652,174, EFZMCP_LEFT_TOP	 ,659, 182,1,0 },					    // 26. 아이스 필드.
		{ Greedy     ,537,261, EFZMCP_LEFT_TOP	 ,565, 267,1,0 },					    // 27. 그리디 호수.
		{ IceField_2 ,700, 97, EFZMCP_LEFT_TOP	 ,712, 97,1,0 },					    // 28. 아이스 필드2.
		{ Lost_1	 ,594,536, EFZMCP_LEFT_BOTTOM,596, 533,1,0 },					    // 29. 로스트 아일랜드
		{ Lost_2	 ,588,481, EFZMCP_LEFT_TOP	 ,578, 477,1,0 },					    // 30. 로스트 템플 
	};
#endif
#ifdef	_LANGUAGE_THAI
static SFullZoomMapInfo		s_FzmInfo[ EFzmNameMax ] = 
{
		{ EPilait, 438, 21, EFZMCP_LEFT_TOP, 440, 32, 1, 508.f },						// 0.  필라이.
		{ Efallfore_1, 399, 84, EFZMCP_LEFT_TOP, 466, 68, 1, 0 },						// 1.  바람이 이끄는 길.
		{ Efallfore_2, 396, 127, EFZMCP_LEFT_BOTTOM, 411, 122, 1, 0 },					// 2.  고요의 계곡.
		{ Efallfore_3, 487, 174, EFZMCP_LEFT_TOP, 480, 191, 1, 0 },						// 3.  황혼의 숲.
		{ EMcavet, 366, 191, EFZMCP_LEFT_TOP, 397, 205, 0, 0 },							// 4.  벌집동굴.
		{ Efallfore_4, 533, 215, EFZMCP_LEFT_TOP, 535, 230, 1, 0 },						// 5.  정령의 숲.
		{ ERuin_3, 451, 265, EFZMCP_LEFT_TOP, 493, 271, 1, 0 },							// 6.  루이넨 빌리지.
		{ Edunt, 544, 278, EFZMCP_LEFT_BOTTOM, 569, 276, 0, 0 },						// 7.  고대 감옥.
		{ ERuin_4, 529, 326, EFZMCP_LEFT_TOP, 535, 341, 1, 0 },							// 8.  저주받은 땅.
		{ ERuin_2, 451, 331, EFZMCP_LEFT_TOP, 461, 350, 1, 0 },							// 9.  폐성곽.
		{ ERuin_1, 470, 390, EFZMCP_LEFT_TOP, 539, 392, 1, 0 },							// 10. 고대인의 안식처.
		{ ETcavet, 363, 379, EFZMCP_LEFT_BOTTOM, 381, 365, 0, 0 },						// 11. 버섯 동굴.
		{ Efore_3, 390, 421, EFZMCP_LEFT_TOP, 380, 438, 1, 0 },							// 12. 아카시아 숲.
		{ Efore_2, 420, 460, EFZMCP_LEFT_BOTTOM, 436, 443, 1, 0 },						// 13. 대나무 숲.
		{ Efore_1, 502, 444, EFZMCP_LEFT_TOP, 505, 463, 1, 0 },							// 14. 자유의 정원.
		{ ERicaT, 494, 510, EFZMCP_LEFT_TOP, 502, 527, 1, 477.f },						// 15. 리카르텐.
		{ Ede_1, 559, 371, EFZMCP_LEFT_TOP, 588, 391, 1, 0 },							// 16. 잊혀진 땅.
		{ Ede_nv, 620, 405, EFZMCP_LEFT_BOTTOM, 640, 382, 1, 0 },						// 17. 네비스코.
		{ Ede_2, 642, 462, EFZMCP_LEFT_TOP, 636, 474, 1, 0 },							// 18. 오아시스.
		{ Ede_3, 675, 364, EFZMCP_LEFT_TOP, 678, 386, 1, 0 },							// 19. 고대인의 전장.
		{ Esint, 692, 417, EFZMCP_LEFT_TOP, 733, 417, 0, 0 },							// 20. 저주받은 신전.
		{ Ede_4, 659, 308, EFZMCP_LEFT_TOP, 710, 314, 1, 0 },							// 21. 금단의 땅.
		{ Eiron_1, 742, 237, EFZMCP_LEFT_TOP, 752, 277, 1, 0 },							// 22. 혼돈의 철로.
		{ EDcavet, 358, 297, EFZMCP_LEFT_TOP, 376, 325, 1, 0 },							// 23. 어둠의 성전.
		{ Eiron_2, 627, 246, EFZMCP_LEFT_TOP, 663, 270, 1, 352.f },						// 24. 폐룸의 심장.
		{ Eyura_1, 591, 201, EFZMCP_LEFT_TOP, 598, 205, 1, 378.f },						// 25. 유라 빌리지.
		{ IceField_1 ,650,171, EFZMCP_LEFT_TOP	 ,659, 182,1,0 },					    // 26. 아이스 필드.
		{ Greedy     ,567,271, EFZMCP_LEFT_TOP	 ,565, 267,1,0 },					    // 27. 그리디 호수.
		{ IceField_2 ,712,97, EFZMCP_LEFT_TOP	 ,565, 267,1,0 },					    // 28. 아이스 필드2.
		{ Lost_1	 ,600,533, EFZMCP_LEFT_BOTTOM,596, 533,1,0 },					    // 29. 로스트 아일랜드
		{ Lost_2	 ,1024,768, EFZMCP_LEFT_TOP	 ,578, 477,1,0 },					    // 30. 로스트 템플 
//		{ Lost_2	 ,578,477, EFZMCP_LEFT_TOP	 ,578, 477,1,0 },					    // 30. 로스트 템플 
	};
#endif
#ifdef _LANGUAGE_BRAZIL	
static SFullZoomMapInfo		s_FzmInfo[ EFzmNameMax ] = 
{
		{ EPilait, 438, 23, EFZMCP_LEFT_TOP, 440, 32, 1, 508.f },						// 0.  필라이.
		{ Efallfore_1, 398, 83, EFZMCP_LEFT_TOP, 466, 68, 1, 0 },						// 1.  바람이 이끄는 길.
		{ Efallfore_2, 390, 143, EFZMCP_LEFT_BOTTOM, 411, 122, 1, 0 },					// 2.  고요의 계곡.
		{ Efallfore_3, 482, 176, EFZMCP_LEFT_TOP, 480, 191, 1, 0 },						// 3.  황혼의 숲.
		{ EMcavet, 376, 190, EFZMCP_LEFT_TOP, 397, 205, 0, 0 },							// 4.  벌집동굴.
		{ Efallfore_4, 524, 218, EFZMCP_LEFT_TOP, 535, 230, 1, 0 },						// 5.  정령의 숲.
		{ ERuin_3, 438, 269, EFZMCP_LEFT_TOP, 493, 271, 1, 0 },							// 6.  루이넨 빌리지.
		{ Edunt, 531, 298, EFZMCP_LEFT_BOTTOM, 569, 276, 0, 0 },						// 7.  고대 감옥.
		{ ERuin_4, 535, 323, EFZMCP_LEFT_TOP, 535, 341, 1, 0 },							// 8.  저주받은 땅.
		{ ERuin_2, 456, 339, EFZMCP_LEFT_TOP, 461, 350, 1, 0 },							// 9.  폐성곽.
		{ ERuin_1, 489, 389, EFZMCP_LEFT_TOP, 539, 392, 1, 0 },							// 10. 고대인의 안식처.
		{ ETcavet, 361, 377, EFZMCP_LEFT_BOTTOM, 381, 365, 0, 0 },						// 11. 버섯 동굴.
		{ Efore_3, 375, 422, EFZMCP_LEFT_TOP, 380, 438, 1, 0 },							// 12. 아카시아 숲.
		{ Efore_2, 419, 455, EFZMCP_LEFT_BOTTOM, 436, 443, 1, 0 },						// 13. 대나무 숲.
		{ Efore_1, 506, 449, EFZMCP_LEFT_TOP, 505, 463, 1, 0 },							// 14. 자유의 정원.
		{ ERicaT, 495, 507, EFZMCP_LEFT_TOP, 502, 527, 1, 477.f },						// 15. 리카르텐.
		{ Ede_1, 551, 366, EFZMCP_LEFT_TOP, 588, 391, 1, 0 },							// 16. 잊혀진 땅.
		{ Ede_nv, 621, 404, EFZMCP_LEFT_BOTTOM, 640, 382, 1, 0 },						// 17. 네비스코.
		{ Ede_2, 640, 460, EFZMCP_LEFT_TOP, 636, 474, 1, 0 },							// 18. 오아시스.
		{ Ede_3, 671, 367, EFZMCP_LEFT_TOP, 678, 386, 1, 0 },							// 19. 고대인의 전장.
		{ Esint, 693, 419, EFZMCP_LEFT_TOP, 733, 417, 0, 0 },							// 20. 저주받은 신전.
		{ Ede_4, 643, 315, EFZMCP_LEFT_TOP, 710, 314, 1, 0 },							// 21. 금단의 땅.
		{ Eiron_1, 743, 239, EFZMCP_LEFT_TOP, 752, 277, 1, 0 },							// 22. 혼돈의 철로.
		{ EDcavet, 363, 297, EFZMCP_LEFT_TOP, 376, 325, 1, 0 },							// 23. 어둠의 성전.
		{ Eiron_2, 621, 256, EFZMCP_LEFT_TOP, 663, 270, 1, 352.f },						// 24. 폐룸의 심장.
		{ Eyura_1, 591, 201, EFZMCP_LEFT_TOP, 598, 205, 1, 378.f },						// 25. 유라 빌리지.
		{ IceField_1 ,654,171, EFZMCP_LEFT_TOP	 ,659, 182,1,0 },					    // 26. 아이스 필드.
		{ Greedy     ,536,276, EFZMCP_LEFT_TOP	 ,565, 267,1,0 },					    // 27. 그리디 호수.
		{ IceField_2 ,714,83, EFZMCP_LEFT_TOP	 ,565, 267,1,0 },					    // 28. 아이스 필드2.
		{ Lost_1	 ,597,532, EFZMCP_LEFT_BOTTOM,596, 533,1,0 },					    // 29. 로스트 아일랜드
		{ Lost_2	 ,598,487, EFZMCP_LEFT_TOP	 ,578, 477,1,0 },					    // 30. 로스트 템플 
};
#endif
#ifdef _LANGUAGE_ARGENTINA
static SFullZoomMapInfo		s_FzmInfo[ EFzmNameMax ] = 
{
		{ EPilait, 438, 22, EFZMCP_LEFT_TOP, 440, 32, 1, 508.f },						// 0.  필라이.
		{ Efallfore_1, 399, 84, EFZMCP_LEFT_TOP, 466, 68, 1, 0 },						// 1.  바람이 이끄는 길.
		{ Efallfore_2, 395, 127, EFZMCP_LEFT_BOTTOM, 411, 122, 1, 0 },					// 2.  고요의 계곡.
		{ Efallfore_3, 487, 174, EFZMCP_LEFT_TOP, 480, 191, 1, 0 },						// 3.  황혼의 숲.
		{ EMcavet, 379, 204, EFZMCP_LEFT_TOP, 397, 205, 0, 0 },							// 4.  벌집동굴.
		{ Efallfore_4, 533, 215, EFZMCP_LEFT_TOP, 535, 230, 1, 0 },						// 5.  정령의 숲.
		{ ERuin_3, 450, 265, EFZMCP_LEFT_TOP, 493, 271, 1, 0 },							// 6.  루이넨 빌리지.
		{ Edunt, 543, 285, EFZMCP_LEFT_BOTTOM, 569, 276, 0, 0 },						// 7.  고대 감옥.
		{ ERuin_4, 530, 325, EFZMCP_LEFT_TOP, 535, 341, 1, 0 },							// 8.  저주받은 땅.
		{ ERuin_2, 451, 331, EFZMCP_LEFT_TOP, 461, 350, 1, 0 },							// 9.  폐성곽.
		{ ERuin_1, 470, 390, EFZMCP_LEFT_TOP, 539, 392, 1, 0 },							// 10. 고대인의 안식처.
		{ ETcavet, 361, 386, EFZMCP_LEFT_BOTTOM, 381, 365, 0, 0 },						// 11. 버섯 동굴.
		{ Efore_3, 380, 430, EFZMCP_LEFT_TOP, 380, 438, 1, 0 },							// 12. 아카시아 숲.
		{ Efore_2, 420, 460, EFZMCP_LEFT_BOTTOM, 436, 443, 1, 0 },						// 13. 대나무 숲.
		{ Efore_1, 502, 444, EFZMCP_LEFT_TOP, 505, 463, 1, 0 },							// 14. 자유의 정원.
		{ ERicaT, 494, 511, EFZMCP_LEFT_TOP, 502, 527, 1, 477.f },						// 15. 리카르텐.
		//{ Ede_1, 659, 308, EFZMCP_LEFT_TOP, 588, 391, 1, 0 },							// 16. 잊혀진 땅.
		{ Ede_1, 559, 370, EFZMCP_LEFT_TOP, 588, 391, 1, 0 },		// pluto 			// 16. 잊혀진 땅.
		{ Ede_nv, 620, 405, EFZMCP_LEFT_BOTTOM, 640, 382, 1, 0 },						// 17. 네비스코.
		{ Ede_2, 642, 462, EFZMCP_LEFT_TOP, 636, 474, 1, 0 },							// 18. 오아시스.
		{ Ede_3, 675, 364, EFZMCP_LEFT_TOP, 678, 386, 1, 0 },							// 19. 고대인의 전장.
		{ Esint, 696, 417, EFZMCP_LEFT_TOP, 733, 417, 0, 0 },							// 20. 저주받은 신전.
		//{ Ede_4, 559, 370, EFZMCP_LEFT_TOP, 710, 314, 1, 0 },							// 21. 금단의 땅.
		{ Ede_4, 659, 308, EFZMCP_LEFT_TOP, 710, 314, 1, 0 },		// pluto			// 21. 금단의 땅.
		{ Eiron_1, 742, 237, EFZMCP_LEFT_TOP, 752, 277, 1, 0 },							// 22. 혼돈의 철로.
		{ EDcavet, 358, 297, EFZMCP_LEFT_TOP, 376, 325, 1, 0 },							// 23. 어둠의 성전.
		{ Eiron_2, 618, 248, EFZMCP_LEFT_TOP, 663, 270, 1, 352.f },						// 24. 폐룸의 심장.
		{ Eyura_1, 592, 201, EFZMCP_LEFT_TOP, 598, 205, 1, 378.f },						// 25. 유라 빌리지.
		{ IceField_1 ,650,171, EFZMCP_LEFT_TOP	 ,659, 182,1,0 },					    // 26. 아이스 필드.
		{ Greedy     ,567,268, EFZMCP_LEFT_TOP	 ,565, 267,1,0 },					    // 27. 그리디 호수.
		{ IceField_2 ,723,92, EFZMCP_LEFT_TOP	 ,565, 267,1,0 },					    // 28. 아이스 필드2.
		{ Lost_1	 ,597,525, EFZMCP_LEFT_BOTTOM,596, 533,1,0 },					    // 29. 로스트 아일랜드
		{ Lost_2	 ,597,489, EFZMCP_LEFT_TOP	 ,578, 477,1,0 },					    // 30. 로스트 템플 
};
#endif
#ifdef _LANGUAGE_VEITNAM
static SFullZoomMapInfo		s_FzmInfo[ EFzmNameMax ] = 
{
		{ EPilait, 435, 24, EFZMCP_LEFT_TOP, 440, 32, 1, 508.f },						// 0.  필라이.
		{ Efallfore_1, 397, 80, EFZMCP_LEFT_TOP, 466, 68, 1, 0 },						// 1.  바람이 이끄는 길.
		{ Efallfore_2, 387, 124, EFZMCP_LEFT_BOTTOM, 411, 122, 1, 0 },					// 2.  고요의 계곡.
		{ Efallfore_3, 491, 181, EFZMCP_LEFT_TOP, 480, 191, 1, 0 },						// 3.  황혼의 숲.
		{ EMcavet, 359, 176, EFZMCP_LEFT_TOP, 397, 205, 0, 0 },							// 4.  벌집동굴.
		{ Efallfore_4, 536, 221, EFZMCP_LEFT_TOP, 535, 230, 1, 0 },						// 5.  정령의 숲.
		{ ERuin_3, 429, 260, EFZMCP_LEFT_TOP, 493, 271, 1, 0 },							// 6.  루이넨 빌리지.
		{ Edunt, 535, 287, EFZMCP_LEFT_BOTTOM, 569, 276, 0, 0 },						// 7.  고대 감옥.
		{ ERuin_4, 535, 321, EFZMCP_LEFT_TOP, 535, 341, 1, 0 },							// 8.  저주받은 땅.
		{ ERuin_2, 434, 328, EFZMCP_LEFT_TOP, 461, 350, 1, 0 },							// 9.  폐성곽.
		{ ERuin_1, 451, 388, EFZMCP_LEFT_TOP, 539, 392, 1, 0 },							// 10. 고대인의 안식처.
		{ ETcavet, 363, 386, EFZMCP_LEFT_BOTTOM, 381, 365, 0, 0 },						// 11. 버섯 동굴.
		{ Efore_3, 372, 425, EFZMCP_LEFT_TOP, 380, 438, 1, 0 },							// 12. 아카시아 숲.
		{ Efore_2, 418, 460, EFZMCP_LEFT_BOTTOM, 436, 443, 1, 0 },						// 13. 대나무 숲.
		{ Efore_1, 502, 446, EFZMCP_LEFT_TOP, 505, 463, 1, 0 },							// 14. 자유의 정원.
		{ ERicaT, 493, 511, EFZMCP_LEFT_TOP, 502, 527, 1, 477.f },						// 15. 리카르텐.
		{ Ede_1, 542, 359, EFZMCP_LEFT_TOP, 588, 391, 1, 0 },							// 16. 잊혀진 땅.
		{ Ede_nv, 620, 405, EFZMCP_LEFT_BOTTOM, 640, 382, 1, 0 },						// 17. 네비스코.
		{ Ede_2, 640, 460, EFZMCP_LEFT_TOP, 636, 474, 1, 0 },							// 18. 오아시스.
		{ Ede_3, 671, 368, EFZMCP_LEFT_TOP, 678, 386, 1, 0 },							// 19. 고대인의 전장.
		{ Esint, 690, 418, EFZMCP_LEFT_TOP, 733, 417, 0, 0 },							// 20. 저주받은 신전.
		{ Ede_4, 645, 309, EFZMCP_LEFT_TOP, 710, 314, 1, 0 },							// 21. 금단의 땅.
		{ Eiron_1, 721, 238, EFZMCP_LEFT_TOP, 752, 277, 1, 0 },							// 22. 혼돈의 철로.
		{ EDcavet, 360, 295, EFZMCP_LEFT_TOP, 376, 325, 1, 0 },							// 23. 어둠의 성전.
		{ Eiron_2, 627, 246, EFZMCP_LEFT_TOP, 663, 270, 1, 352.f },						// 24. 폐룸의 심장.
		{ Eyura_1, 591, 201, EFZMCP_LEFT_TOP, 598, 205, 1, 378.f },						// 25. 유라 빌리지.
		{ IceField_1 ,653,159, EFZMCP_LEFT_TOP	 ,653, 159,1,0 },					    // 26. 아이스 필드.
		{ Greedy     ,564,264, EFZMCP_LEFT_TOP	 ,565, 267,1,0 },					    // 27. 그리디 호수.
		{ IceField_2 ,712,97, EFZMCP_LEFT_TOP	 ,712, 97,1,0 },					    // 28. 아이스 필드2.
		{ Lost_1	 ,586,537, EFZMCP_LEFT_BOTTOM,596, 533,1,0 },					    // 29. 로스트 아일랜드
		{ Lost_2	 ,587,485, EFZMCP_LEFT_TOP	 ,578, 477,1,0 },					    // 30. 로스트 템플 
};
#endif
//
static int s_iFzmFieldCodeInfo[] = 
{
	12, // 아카시아 숲.
	13, // 대나무 숲.
	14, // 자유의 정원.
	15, // 리카르텐.
	10, // 고대인의 안식처.
	9,  // 폐성곽.
	6,  // 루이넨 빌리지.
	8,  // 저주받은 땅.
	16, // 잊혀진 땅.
	17, // 네비스코.
	18, // 오아시스.
	19, // 고대인의 전장.
	21, // 금단의 땅.
	7,  // 고대 감옥. (1층)
	7,  // 고대 감옥. (2층)
	7,  // 고대 감옥. (3층)
	7,  // 장기판.
	5,  // 정령의 숲.
	3,  // 황혼의 숲.
	2,  // 고요의 계곡.
	1,  // 바람이 이끄는 길.
	0,  // 필라이.
	20, // 저주받은 신전. (1층)
	20, // 저주받은 신전. (2층)
	11, // 버섯 동굴.
	4,  // 벌집동굴.
	23, // 어둠의 성전.
	22, // 혼돈의 철로.
	24,	// 폐룸의 심장.
	25, // 유라 빌리지.	
	4,	// SOD
	26, // 아이스 필드
	4,  // 대전장
	4,  // 공성필드
	27, // 그리디 호수
	28, // 아이스 필드2
	// 현재 사용치 않음.	
	4,
	29, // 로스트 아일랜드			
	30, // 로스트 템플
	4,
	30, // pluto 끝없는 탑 1층
	30, // pluto 끝없는 탑 1층
	20, // 박재원 - 저주받은 신전 3층 추가
	30, // 박재원 - 끝없는 탑 3층 추가
};

// Guide map of a field: a moved old map (sFIELD::OriginField) shows today's
// version of it; fields past the table get the same one as SOD.
static int FzmFieldCode( int iFieldCode )
{
	if ( sField[iFieldCode].OriginField>=0 ) iFieldCode = sField[iFieldCode].OriginField;
	if ( iFieldCode>=int(sizeof(s_iFzmFieldCodeInfo)/sizeof(s_iFzmFieldCodeInfo[0])) ) return 4;
	return s_iFzmFieldCodeInfo[ iFieldCode ];
}

//
static char	*s_lpFzmINamePtrBuff[ EFzmNameMax * 2 ] =			//미니맵 이름 ,미니맵 이미지
{
	// 필라이.
	"Pilait.bmp",
	"town_PLX-40Y-36.bmp",

	// 바람이 이끄는 길.
	"fallfore-1t.bmp",
	"fallfore-1X70.bmp",

	// 고요의 계곡.
	"fallfore-2t.bmp",
	"fallfore-2X10.bmp",

	// 황혼의 숲.
	"fallfore-3t.bmp",
	"fallfore-3X5.bmp",

	// 벌집동굴.
	"_Mcavet.bmp",
	"dun.bmp",				// 중복. 재사용 한다.

	// 정령의 숲.
	"fallfore-4t.bmp",
	"fallfore-4X30.bmp",

	// 루이넨 빌리지.
	"Ruin-3t.bmp",
	"Ruin-3Y10.bmp",

	// 고대 감옥.
	"_dunt.bmp",
	"@4",					// "dun.bmp"

	// 저주받은 땅.
	"Ruin-4t.bmp",
	"Ruin-4X40.bmp",

	// 폐성곽.
	"Ruin-2t.bmp",
	"Ruin-2X0.bmp",

	// 고대인의 안식처.
	"Ruin-1t.bmp",
	"Ruin-1X10.bmp",

	// 버섯 동굴.
	"_Tcavet.bmp",
	"@4",					// "dun.bmp"

	// 아카시아 숲.
	"fore-3t.bmp",
	"fore-3X2Y20.bmp",

	// 대나무 숲.
	"fore-2t.bmp",
	"fore-2Y-5.bmp",

	// 자유의 정원.
	"fore-1t.bmp",
	"fore-1X20.bmp",

	// 리카르텐.
	"town_RicaT.bmp",
	"town_RicaX-88Y-77.bmp",

	// 잊혀진 땅.
	"de-1t.bmp",
	"de-1X30.bmp",

	// 네비스코.
	"de-NVt.bmp",
	"de-NVY40.bmp",

	// 오아시스.
	"de-2t.bmp",
	"de-2X30.bmp",

	// 고대인의 전장.
	"de-3t.bmp",
	"de-3Y20.bmp",

	// 저주받은 신전.
	"_sint.bmp",
	"@4",					// "dun.bmp"

	// 금단의 땅.
	"de-4t.bmp",
	"de-4Y30.bmp",

	// 혼돈의 철로.
	"iron-1t.bmp",
	"iron-1X40.bmp",

	// 어둠의 성전.
	"_Dcavet.bmp",
	"@4",					// "dun.bmp"

	// 폐룸의 심장.
	"iron_2t.bmp",
	"iron2X-15Y14.bmp",

	// 유라 빌리지.
	"yura_1t.bmp",
	"yuraX15Y-22.bmp",

	//아이스 필드
	"ice1-x655y166.bmp",
	"ice1_X40Y3.bmp",

	//그리디 호수
	"greedy_x565y267.bmp",
	"greedy_x-15y-28.bmp",

	//아이스필드2
	"ice2-x712y97.bmp",
	"ice2_X0Y0.bmp",
	
	//로스트 아일랜드 
	"Lost-1t.bmp",
	"lost-1t_x0_y0.bmp",
	
	//로스트 템플 
	"lost-2_x579y478.bmp",
	"lost-2_x0_y0.bmp"
	
};

//
static int s_iFzmMapOffset[ EFzmNameMax ][ 2 ] = 
{
	{-40, -36}, // 필라이.
	{ 70,   0}, // 바람이 이끄는 길.
	{ 10,   0}, // 고요의 계곡.
	{  5,   0}, // 황혼의 숲.
	{  0,   0}, // 벌집동굴.
	{ 30,   0}, // 정령의 숲.
	{  0,  10}, // 루이넨 빌리지.
	{  0,   0}, // 고대 감옥.
	{ 40,   0}, // 저주받은 땅.
	{  0,   0}, // 폐성곽.
	{ 10,   0}, // 고대인의 안식처.
	{  0,   0}, // 버섯 동굴.
	{  2,  20}, // 아카시아 숲.
	{  0,  -5}, // 대나무 숲.
	{ 20,   0}, // 자유의 정원.
	{-88, -77}, // 리카르텐.
	{ 30,   0}, // 잊혀진 땅.
	{  0,  40}, // 네비스코.
	{ 30,   0}, // 오아시스.
	{  0,  20}, // 고대인의 전장.
	{  0,   0}, // 저주받은 신전.
	{  0,  30}, // 금단의 땅.
	{ 40,   0}, // 혼돈의 철로.
	{  0,   0}, // 어둠의 성전.
	{-15,  14}, // 폐룸의 심장.
	{ 15, -22}, // 유라 빌리지.
	{ 40,   3}, // 아이스 필드
	{ 15,  -4}, // 그리디 호수
	{ 20,  -4}, // 아이스 필드 2
	{ 30,  18}, // 로스트 아일랜드
	{ 7 ,  38}, // 로스트 템플
};

//
static SFullZoomMapEffectInfo   s_NameEffectDataInfo = 
{
	{  3 },
	{  0,  16, 30, 0,},
	{100, 255, 20, 0,},
	{  0,  12, 24, 0,},
};

//
static SFullZoomMapEffectInfo   s_CharPosEffectDataInfo = 
{
	{  4 },
	{  0,  20,  30,  30, 0, },
	{255, 255, 255, 100, 0, },
	{  0,   6,   6,   6, 0, },
};


//######################################################################################
//
extern float g_fWinSizeRatio_X;
extern float g_fWinSizeRatio_Y;

extern int MatArrow;
extern int psDrawTexImage2( smTEXTUREHANDLE *hTexHandle , float *x, float *y, float fx, float fy, float fw, float fh, int Transp , int specular);

//
static void	CreateFullZoomMap(void);		
static void DestroyFullZoomMap(void);		
static int  LoadZoomInMap( SFullZoomMapInfo  *lpZoomMapInfo );		
static void	SetFzmEffect( EFzmEffectType eEffectType, SFullZoomMapEffectInfo *lpEffectInfo, int EventNum, int ActiveName );
static void	MainFzmEffect(void);
static void	DrawFzmEffect(void);
static void	SetMapStageInfo(void);
static void	SetFzmCharPos(void);

//
static SMapStageInfo			s_MapStageInfoBuff[ FIELD_MAX ];
static SFullZoomMapImage		s_FzmImage[ EFzmNameMax ];

static int						s_iMouseLButtonState[2] = {-1,-1};
static POINT					s_CurPlayerPos;
static int						s_iFzmActiveName = -1;
static int						s_iFzmFocusName  = -1;
static LPDIRECTDRAWSURFACE4		s_lpDDS_FzmActiveMap;

static LPDIRECTDRAWSURFACE4		s_lpDDS_Exit;
static LPDIRECTDRAWSURFACE4		s_lpDDS_ExitImage;
static LPDIRECTDRAWSURFACE4		s_lpDDS_ExitFocusImage;
static POINT					s_ExitButtonPos;

static SFullZoomMapEffect		s_NameEffectBuff[ dMAX_NAME_EFFECT ];
static int						s_iFzmNameEffectMatNum;

static int						s_iFzmCharPosNameEffectTime;

static SFzmCharPos				s_FzmCharPosBuff[ dMAX_FZM_CHAR_POS ];
static int						s_iFzmCharPosNum;

static LPDIRECTDRAWSURFACE4		s_lpDDS_FullMapImage;
static int						s_iFzmUserLargeMatNum;
static int						s_iFzmPartyLargeMatNum;
static int						s_iFzmUserSmallMatNum;

//
int								g_iFzmCursorFocusGame;

int								g_iFzmCursorPosX;
int								g_iFzmCursorPosY;
int								g_FzmMouseButton[3];


//######################################################################################
//작 성 일 : 2004-04-08 오후 3:27:06
//리 턴 값 :
//설    명 :
//작 성 자 : 오 영 석
//######################################################################################
static void SetFzmEffect( EFzmEffectType eEffectType, SFullZoomMapEffectInfo *lpEffectInfo, int EventNum, int ActiveName )
{
	if( ActiveName < 0 )
		return;

	for( int i=0; i < dMAX_NAME_EFFECT; i++ )
	{
		if( s_NameEffectBuff[i].Life == 0 )
		{
			SFullZoomMapInfo  *lpZoomMapInfo  = &s_FzmInfo[ ActiveName ];
			SFullZoomMapImage *lpZoomMapImage = &s_FzmImage[ lpZoomMapInfo->eNameType-1 ];

			::ZeroMemory( &s_NameEffectBuff[i], sizeof(s_NameEffectBuff[i]) );
			s_NameEffectBuff[i].lpEffectInfo = lpEffectInfo;
			s_NameEffectBuff[i].eEffectType  = eEffectType;
			
			s_NameEffectBuff[i].Life      = 1;
			s_NameEffectBuff[i].EventNum  = EventNum;
			s_NameEffectBuff[i].Pos.x     = lpZoomMapInfo->iNamePosX + (lpZoomMapImage->iCheckWideh  >> 1);
			s_NameEffectBuff[i].Pos.y	  = lpZoomMapInfo->iNamePosY + (lpZoomMapImage->iCheckHeight >> 1);
			s_NameEffectBuff[i].HalfSize  = 8;
			s_NameEffectBuff[i].fBlendNum = float(lpEffectInfo->BlendNum[0]);
			s_NameEffectBuff[i].fScaleNum = float(lpEffectInfo->ScaleNum[0]);
			return;
		}
	}
}
//######################################################################################
//작 성 일 : 2004-04-08 오후 3:27:28
//리 턴 값 :
//설    명 :
//작 성 자 : 오 영 석
//######################################################################################
static void MainFzmEffect(void)
{
	//
	static int iFieldCode;
	if( s_iFzmCharPosNameEffectTime > 160 )
	{
		if( lpCurPlayer->OnStageField >= 0 )
		{
			iFieldCode = StageField[ lpCurPlayer->OnStageField ]->FieldCode;
			SetFzmEffect(CHAR_POS_NAME_EFFECT, &s_NameEffectDataInfo, 2, FzmFieldCode( iFieldCode ) );
		}
		
		s_iFzmCharPosNameEffectTime = 0;
	}
	else
		s_iFzmCharPosNameEffectTime++;

	//
	for( int i=0; i < dMAX_NAME_EFFECT; i++ )
	{
		if( s_NameEffectBuff[i].Life )
		{
			SFullZoomMapEffect *lpNameEffect = &s_NameEffectBuff[i];
			lpNameEffect->FrameTime++;

			if( lpNameEffect->FrameTime >= lpNameEffect->lpEffectInfo->DelayTime[ lpNameEffect->FrameNum ] )
			{
				lpNameEffect->FrameNum++;
				if( lpNameEffect->EventNum == lpNameEffect->FrameNum )
				{
					if( lpNameEffect->eEffectType == NAME_EFFECT )
						SetFzmEffect(NAME_EFFECT, &s_NameEffectDataInfo, 0, s_iFzmActiveName);
					else if( lpNameEffect->eEffectType == CHAR_POS_EFFECT )
						SetFzmEffect(CHAR_POS_EFFECT, &s_CharPosEffectDataInfo, 3, s_iFzmActiveName);
					else if( lpNameEffect->eEffectType == CHAR_POS_NAME_EFFECT )
						SetFzmEffect(CHAR_POS_NAME_EFFECT, &s_NameEffectDataInfo, 0, FzmFieldCode( iFieldCode ) );
				}

				//
				if( lpNameEffect->FrameNum >= lpNameEffect->lpEffectInfo->MaxFrameNum )
					lpNameEffect->Life = 0;
				else
				{
					lpNameEffect->FrameTime = 0;
					lpNameEffect->fBlendNum = float( lpNameEffect->lpEffectInfo->BlendNum[ lpNameEffect->FrameNum-1 ] );
					lpNameEffect->fScaleNum = float( lpNameEffect->lpEffectInfo->ScaleNum[ lpNameEffect->FrameNum-1 ] );

					float fDelayTime = float( lpNameEffect->lpEffectInfo->DelayTime[ lpNameEffect->FrameNum ] );
					lpNameEffect->fBlendStep = (lpNameEffect->lpEffectInfo->BlendNum[ lpNameEffect->FrameNum ] - lpNameEffect->fBlendNum) / fDelayTime;
					lpNameEffect->fScaleStep = (lpNameEffect->lpEffectInfo->ScaleNum[ lpNameEffect->FrameNum ] - lpNameEffect->fScaleNum) / fDelayTime;
				}
			}
			else
			{
				lpNameEffect->fBlendNum += lpNameEffect->fBlendStep;
				lpNameEffect->fScaleNum += lpNameEffect->fScaleStep;
			}
		}
	}
}


//######################################################################################
//작 성 일 : 2004-04-08 오후 3:27:48
//리 턴 값 :
//설    명 :
//작 성 자 : 오 영 석
//######################################################################################
static void DrawFzmEffect(void)
{
	for( int i=0; i < dMAX_NAME_EFFECT; i++ )
	{
		if( s_NameEffectBuff[i].Life && s_NameEffectBuff[i].eEffectType != CHAR_POS_EFFECT )
		{
			SFullZoomMapEffect *lpNameEffect = &s_NameEffectBuff[i];
			
			DrawFzmBackToTexture2D( smMaterial[ s_iFzmNameEffectMatNum ].smTexture[0], lpNameEffect->Pos.x, lpNameEffect->Pos.y, 0,
									lpNameEffect->HalfSize + int(lpNameEffect->fScaleNum), int(lpNameEffect->fBlendNum) );
		}
	}
}

//######################################################################################
//작 성 일 : 2004-04-08 오후 3:28:08
//리 턴 값 :
//설    명 :
//작 성 자 : 오 영 석
//######################################################################################
static void SetMapStageInfo(void)
{
	FILE *fp = fopen( "image\\GuideMap\\StageInfo.dat" , "rb" );
	if( fp )
	{
		fread( s_MapStageInfoBuff, sizeof(s_MapStageInfoBuff), 1, fp );
		fclose(fp);
	}
	#ifdef _DEBUG
	else
	{
		smSTAGE3D			 MapStage;
		SFullZoomMapInfo	*lpZoomMapInfo;
		int					 Width, Height;
		float				 fRatio;

		MapStage.Init();
		for( int i=0; i < FieldCount; i++ )
		{
			// 라이트 벡터
			MapStage.VectLight.x = smConfig.MapLightVector.x;
			MapStage.VectLight.y = smConfig.MapLightVector.y;
			MapStage.VectLight.z = smConfig.MapLightVector.z;
			MapStage.Bright		 = smConfig.MapBright;
			MapStage.Contrast	 = smConfig.MapContrast;

			//
			if( smSTAGE3D_ReadASE(sField[i].szName, &MapStage) )
			{
				::CopyMemory( &s_MapStageInfoBuff[i].rcStageMapRect, &MapStage.StageMapRect, sizeof(RECT) );
				s_MapStageInfoBuff[i].iStageWidth  = MapStage.StageMapRect.right  - MapStage.StageMapRect.left;
				s_MapStageInfoBuff[i].iStageHeight = MapStage.StageMapRect.bottom - MapStage.StageMapRect.top;

				Width		  = s_MapStageInfoBuff[i].iStageWidth;
				Height		  = s_MapStageInfoBuff[i].iStageHeight;

				lpZoomMapInfo = &s_FzmInfo[ FzmFieldCode( i ) ];
				if( lpZoomMapInfo->fRatio )
				{
					if( Width > Height )	fRatio = float(Width)  / lpZoomMapInfo->fRatio;
					else					fRatio = float(Height) / lpZoomMapInfo->fRatio;
				}
				else
				{
					if( Width > Height )	fRatio = float(Width)  / 320.f;
					else					fRatio = float(Height) / 320.f;
				}

				s_MapStageInfoBuff[i].fRatio = fRatio;

				//
				MapStage.Close();
				MapStage.Init();
			}
		}

		fp = fopen( "image\\GuideMap\\StageInfo.dat" , "wb" );
		if( fp )
		{			fwrite( s_MapStageInfoBuff, sizeof(s_MapStageInfoBuff), 1, fp );
			fclose(fp);
		}
	}
	#endif
}


//######################################################################################
//작 성 일 : 2004-04-08 오후 3:28:14
//리 턴 값 :
//설    명 :
//작 성 자 : 오 영 석
//######################################################################################
static void SetFzmCharPos(void)
{
	//
	if( s_iFzmActiveName < 0 )
		return;

	s_iFzmCharPosNum = 0;

	//
	SFullZoomMapInfo  *lpZoomMapInfo  = &s_FzmInfo[ s_iFzmActiveName ];
	SFullZoomMapImage *lpZoomMapImage = &s_FzmImage[ lpZoomMapInfo->eNameType-1 ];
	// 박재원 - 끝없는 탑 3층
	if( StageField[ lpCurPlayer->OnStageField ]->FieldCode == 40 || StageField[ lpCurPlayer->OnStageField ]->FieldCode == 41 || StageField[ lpCurPlayer->OnStageField ]->FieldCode == 43)
	{
		return;		// pluto 끝없는 탑에 있을때 맵 화면에서 잃어버린 사원 안나오게 한다
	}
	if( lpZoomMapInfo->iZoomMapIsCharPos <= 0 )
		return;

	//
	int	   cnt;
	float  fRatio = 0;
	RECT  *lpRect = NULL;

	// User ////////////////////////////////////////////////////////////////////////
	if( lpCurPlayer->OnStageField >= 0 )
	{
		sFIELD *lpField = StageField[ lpCurPlayer->OnStageField ];
		int iFieldCode = lpField->FieldCode;
		int iInfo = lpField->OriginField>=0 ? lpField->OriginField : iFieldCode;		// moved old map: today's map, player moved back
		if( s_iFzmActiveName == FzmFieldCode( iFieldCode ) && s_MapStageInfoBuff[ iInfo ].fRatio > 0 )
		{

			lpRect = &s_MapStageInfoBuff[ iInfo ].rcStageMapRect;
			fRatio = s_MapStageInfoBuff[ iInfo ].fRatio;

			s_FzmCharPosBuff[ s_iFzmCharPosNum ].eCharType = USER_CHAR;

			s_FzmCharPosBuff[ s_iFzmCharPosNum ].iXpos     = int( float(lpCurPlayer->pX - lpField->OffsetX*fONE - lpRect->left)   / fRatio);
			s_FzmCharPosBuff[ s_iFzmCharPosNum ].iYpos     = int( float(lpRect->bottom - (lpCurPlayer->pZ - lpField->OffsetZ*fONE)) / fRatio);
			s_iFzmCharPosNum++;
		}
	}
	//////////////////////////////////////////////////////////////////////////
	
	// Party ////////////////////////////////////////////////////////////////////////
	if( InterfaceParty.PartyMemberCount )
	{
		if( s_iFzmCharPosNum <= 0 )
		{
			for( cnt=0; cnt < FieldCount; cnt++ )
			{
				if( s_iFzmActiveName == FzmFieldCode( cnt ) )
				{
					lpRect = &s_MapStageInfoBuff[ cnt ].rcStageMapRect;
					fRatio = s_MapStageInfoBuff[ cnt ].fRatio;
					break;
				}
			}
		}

		if( lpRect == NULL || fRatio <= 0 )
			return;

		for( cnt=0; cnt < InterfaceParty.PartyMemberCount; cnt++ )
		{
			if( InterfaceParty.PartyMember[cnt].ChrCode != lpCurPlayer->dwObjectSerial )
			{
				POINT3D *lpChPos = &InterfaceParty.PartyMember[ cnt ].CharacterPos;
				if( lpChPos->x > lpRect->left && lpChPos->x < lpRect->right &&
					lpChPos->z > lpRect->top  && lpChPos->z < lpRect->bottom )
				{
					if( s_iFzmCharPosNum >= dMAX_FZM_CHAR_POS )
						return;

					s_FzmCharPosBuff[ s_iFzmCharPosNum ].eCharType = PARTY_CHAR;
					s_FzmCharPosBuff[ s_iFzmCharPosNum ].iXpos     = int( float(lpChPos->x - lpRect->left)   / fRatio);
					s_FzmCharPosBuff[ s_iFzmCharPosNum ].iYpos     = int( float(lpRect->bottom - lpChPos->z) / fRatio);
					s_iFzmCharPosNum++;
				}
			}
		}
	}
	//////////////////////////////////////////////////////////////////////////
}


//######################################################################################
//작 성 일 : 2004-04-08 오후 3:31:12
//리 턴 값 :
//설    명 :
//작 성 자 : 오 영 석
//######################################################################################
static void CreateFullZoomMap(void)
{
	if( g_IsDxProjectZoomIn > 0 )
		return;

	//
	char szFullName[128];
	char szDirectory[32];
	::lstrcpy( szDirectory, "image\\GuideMap\\" );

	//
	::CopyMemory( szFullName, szDirectory, 32 );
	::lstrcat( szFullName, "_AFull.bmp" );
	s_lpDDS_FullMapImage = LoadDibSurfaceOffscreen( szFullName );

	//
	::CopyMemory( szFullName, szDirectory, 32 );
	::lstrcat( szFullName, "exit_1.bmp" );
	s_lpDDS_ExitImage = LoadDibSurfaceOffscreen( szFullName );

	s_lpDDS_Exit      = s_lpDDS_ExitImage;
	s_ExitButtonPos.x = 682;
	s_ExitButtonPos.y = 547;

	//
	::CopyMemory( szFullName, szDirectory, 32 );
	::lstrcat( szFullName, "exit_2.bmp" );
	s_lpDDS_ExitFocusImage = LoadDibSurfaceOffscreen( szFullName );	

	//
	for( int i=0; i < EFzmNameMax; i++ )
	{
		//
		::CopyMemory( szFullName, szDirectory, 32 );
		::lstrcat( szFullName, s_lpFzmINamePtrBuff[i*2] );
		s_FzmImage[i].lpDDS_Name   = LoadDibSurfaceOffscreen( szFullName );
		s_FzmImage[i].iCheckWideh  = g_DibLastReadWidth;
		s_FzmImage[i].iCheckHeight = g_DibLastReadHeight;

		//
		s_FzmImage[i].iMapOffset_x = s_iFzmMapOffset[i][0];
		s_FzmImage[i].iMapOffset_y = s_iFzmMapOffset[i][1];

	}

	//
	SetMapStageInfo();
}


//######################################################################################
//작 성 일 : 2004-04-08 오후 3:31:18
//리 턴 값 :
//설    명 :
//작 성 자 : 오 영 석
//######################################################################################
static void DestroyFullZoomMap(void)
{
	//
	if( s_lpDDS_FullMapImage )
	{
		s_lpDDS_FullMapImage->Release();
		s_lpDDS_FullMapImage = NULL;
	}

	//
	if( s_lpDDS_ExitImage )
	{
		s_lpDDS_ExitImage->Release();
		s_lpDDS_ExitImage = NULL;
	}

	//
	if( s_lpDDS_ExitFocusImage )
	{
		s_lpDDS_ExitFocusImage->Release();
		s_lpDDS_ExitFocusImage = NULL;
	}

	//
	for( int i=0; i < EFzmNameMax; i++ )
	{
		if( s_FzmImage[i].lpDDS_Name )
		{
			s_FzmImage[i].lpDDS_Name->Release();
			s_FzmImage[i].lpDDS_Name = NULL;
		}
	}

	//
	if( s_lpDDS_FzmActiveMap )
	{
		s_lpDDS_FzmActiveMap->Release();
		s_lpDDS_FzmActiveMap = NULL;
	}
}


//######################################################################################
//작 성 일 : 2004-04-09 오전 11:40:10
//리 턴 값 :
//설    명 :
//작 성 자 : 오 영 석
//######################################################################################
static int LoadZoomInMap( SFullZoomMapInfo  *lpZoomMapInfo )
{
	if( lpZoomMapInfo == NULL )
		return FALSE;

	char szFullName[128] = {0,};
	::lstrcpy( szFullName, "image\\GuideMap\\" );

	int index = ((lpZoomMapInfo->eNameType-1) << 1) + 1;
	if( *s_lpFzmINamePtrBuff[ index ] != '@' )
		::lstrcat( szFullName, s_lpFzmINamePtrBuff[ index ] );
	else
	{
		index = ::atoi( s_lpFzmINamePtrBuff[ index ] + 1 );
		::lstrcat( szFullName, s_lpFzmINamePtrBuff[ (index << 1) + 1 ] );
	}

	LPDIRECTDRAWSURFACE4 lpDDS_FzmActiveMap = LoadDibSurfaceOffscreen( szFullName );
	if( lpDDS_FzmActiveMap == NULL )
		return FALSE;

	//
	if( s_lpDDS_FzmActiveMap )
	{
		s_lpDDS_FzmActiveMap->Release();
		s_lpDDS_FzmActiveMap = NULL;
	}

	s_lpDDS_FzmActiveMap = lpDDS_FzmActiveMap;
	return TRUE;
}


//######################################################################################
//작 성 일 : 2004-04-09 오전 11:32:21
//리 턴 값 :
//설    명 :
//작 성 자 : 오 영 석
//######################################################################################
void CreateBeforeFullZoomMap(void)
{
	//
	s_iFzmNameEffectMatNum = cSinWarpGate.MatSelect;

	//
	s_iFzmUserLargeMatNum  = MatArrow;
	s_iFzmPartyLargeMatNum = CreateTextureMaterial( "image\\GuideMap\\User_P.tga", 0, 0, 0, 0, SMMAT_BLEND_ALPHA );
	s_iFzmUserSmallMatNum  = CreateTextureMaterial( "image\\GuideMap\\User_PSR.tga", 0, 0, 0, 0, SMMAT_BLEND_ALPHA );
}


//######################################################################################
//작 성 일 : 2004-04-08 오후 3:31:28
//리 턴 값 :
//설    명 :
//작 성 자 : 오 영 석
//######################################################################################
void DrawFzmBackToSurface( int dest_x, int dest_y, LPDIRECTDRAWSURFACE4 lpDDS, int x, int y, int width, int height, int Bltflag )
{
	RECT destRect;	
	destRect.left   = int(dest_x * g_fWinSizeRatio_X);
	destRect.top    = int(dest_y * g_fWinSizeRatio_Y);
	destRect.right  = int((dest_x+width)  * g_fWinSizeRatio_X);
	destRect.bottom = int((dest_y+height) * g_fWinSizeRatio_Y);
	
	RECT srcRect;
	srcRect.left   = x;
	srcRect.top    = y;
	srcRect.right  = width;
	srcRect.bottom = height;

	lpDDSBack->Blt( &destRect, lpDDS, &srcRect, Bltflag, NULL );
}


//######################################################################################
//작 성 일 : 2004-04-08 오후 3:31:36
//리 턴 값 :
//설    명 :
//작 성 자 : 오 영 석
//######################################################################################
void DrawFzmBackToTexture2D( smTEXTUREHANDLE *hTexHandle, int X, int Y, int Angle, int HalfSize, int Blend )
{
	int ix[4], iy[4];
	ix[0] = -HalfSize; iy[0] = -HalfSize;
	ix[1] = -HalfSize; iy[1] =  HalfSize;
	ix[2] =  HalfSize; iy[2] = -HalfSize;
	ix[3] =  HalfSize; iy[3] =  HalfSize;

	int cnt;
	float fx[4], fy[4];	

	if( Angle )
	{
		int sinY = GetSin[ Angle & ANGCLIP ];
		int cosY = GetCos[ (Angle+ANGLE_180) & ANGCLIP ];

		for( cnt=0; cnt < 4; cnt++ )
		{
			fx[cnt] = (float) (( iy[cnt]*sinY + ix[cnt]*cosY )>>8);
			fy[cnt] = (float) (( iy[cnt]*cosY - ix[cnt]*sinY )>>8);
			fx[cnt] /= 256;
			fy[cnt] /= 256;
			fx[cnt] += (float)X;
			fy[cnt] += (float)Y;
		}
	}
	else
	{
		for( cnt=0; cnt < 4; cnt++ )
		{
			fx[cnt] = float(ix[cnt] + X);
			fy[cnt] = float(iy[cnt] + Y);
		}
	}

	//
	for( cnt=0; cnt < 4; cnt++ )
	{
		fx[cnt] *= g_fWinSizeRatio_X;
		fy[cnt] *= g_fWinSizeRatio_Y;
	}

	//
	psDrawTexImage2( hTexHandle, fx, fy, 0, 0, 1, 1, Blend, 0 );
}


//######################################################################################
//작 성 일 : 2004-04-08 오후 3:31:43
//리 턴 값 :
//설    명 :
//작 성 자 : 오 영 석
//######################################################################################
int SetFullZoomMap(int IsSet)
{
	if( IsSet )
	{
		SFullZoomMapInfo *lpZoomMapInfo;
		int				  iFieldCode;
		
		//
		if( lpCurPlayer->OnStageField >= 0 )
		{
			iFieldCode = StageField[ lpCurPlayer->OnStageField ]->FieldCode;

			s_iFzmActiveName = FzmFieldCode( iFieldCode );
			if( iFieldCode == 40 || iFieldCode == 41 || iFieldCode == 43 ) // pluto 끝없는 탑에 있을때 맵 화면에서 잃어버린 사원 안나오게 한다 // 박재원 - 끝없는 탑 3층
			{
				lpZoomMapInfo    = &s_FzmInfo[ 4 ];
			}
			else
			{
				lpZoomMapInfo    = &s_FzmInfo[ s_iFzmActiveName ];
			}

			LoadZoomInMap( lpZoomMapInfo );
		}
		else
		{
			// 에러이다. 임시로 셋팅. 
			s_iFzmActiveName = 0;
			lpZoomMapInfo    = &s_FzmInfo[ s_iFzmActiveName ];

			LoadZoomInMap( lpZoomMapInfo );
		}

		//
		if( g_IsDxProjectZoomIn <= 0 )
		{
			CreateFullZoomMap();
			g_IsDxProjectZoomIn = 1;
			SetFzmEffect(CHAR_POS_EFFECT, &s_CharPosEffectDataInfo, 3, s_iFzmActiveName);
		}

		return 1;
	}
	else
	{
		DestroyFullZoomMap();

		g_IsDxProjectZoomIn = 0;
		s_iFzmCharPosNameEffectTime = 0;

		::ZeroMemory( s_NameEffectBuff, sizeof(s_NameEffectBuff) );
		s_iFzmFocusName  = -1;
		s_iFzmActiveName = -1;
		g_FzmMouseButton[0] = FALSE;
		g_FzmMouseButton[1] = FALSE;
		g_FzmMouseButton[2] = FALSE;
		return 1;
	}
}



//######################################################################################
//작 성 일 : 2004-08-12 오후 2:06:34
//리 턴 값 :
//설    명 :
//작 성 자 : 오 영 석
//######################################################################################
void DrawFullZoomMapCharPos(void)
{
	SFullZoomMapInfo  *lpZoomMapInfo;
	SFullZoomMapImage *lpZoomMapImage;

	//
	if( s_iFzmCharPosNum > 0 && s_iFzmActiveName >= 0 )
	{
		lpZoomMapInfo  = &s_FzmInfo[ s_iFzmActiveName ];
		lpZoomMapImage = &s_FzmImage[ lpZoomMapInfo->eNameType-1 ];

		lpD3DDevice->BeginScene();
		for( int cnt=0; cnt < s_iFzmCharPosNum; cnt++ )
		{
			switch( s_FzmCharPosBuff[ cnt ].eCharType )
			{
			//
			case USER_CHAR:
				for( int i=0; i < dMAX_NAME_EFFECT; i++ )
				{
					if( s_NameEffectBuff[i].Life && s_NameEffectBuff[i].eEffectType == CHAR_POS_EFFECT )
					{
						SFullZoomMapEffect *lpCharPosEffect = &s_NameEffectBuff[i];

						DrawFzmBackToTexture2D( smMaterial[ s_iFzmUserLargeMatNum ].smTexture[0],
												30 + lpZoomMapImage->iMapOffset_x + s_FzmCharPosBuff[ cnt ].iXpos,
												15 + lpZoomMapImage->iMapOffset_y + s_FzmCharPosBuff[ cnt ].iYpos,
												lpCurPlayer->Angle.y, 
												lpCharPosEffect->HalfSize + int(lpCharPosEffect->fScaleNum),
												int(lpCharPosEffect->fBlendNum) );
					}
				}
				break;

			//
			case PARTY_CHAR:
				DrawFzmBackToTexture2D( smMaterial[ s_iFzmPartyLargeMatNum ].smTexture[0],
										30 + lpZoomMapImage->iMapOffset_x + s_FzmCharPosBuff[ cnt ].iXpos,
										15 + lpZoomMapImage->iMapOffset_y + s_FzmCharPosBuff[ cnt ].iYpos,
										0, 8, 255 );
				break;
			}
		}
		lpD3DDevice->EndScene();
	}
}


//######################################################################################
//작 성 일 : 2004-04-08 오후 3:31:58
//리 턴 값 :
//설    명 :
//작 성 자 : 오 영 석
//######################################################################################
void DrawFullZoomMap(void)
{
	SFullZoomMapInfo  *lpZoomMapInfo;
	SFullZoomMapImage *lpZoomMapImage;

	//
	if( s_lpDDS_FzmActiveMap )
		DrawFzmBackToSurface( 30, 15, s_lpDDS_FzmActiveMap, 0, 0, 320, 320, DDBLT_WAIT );	

	//
	DrawFullZoomMapCharPos();

	//
	DrawFzmBackToSurface( 0, 0, s_lpDDS_FullMapImage, 0, 0, 800, 600, DDBLT_WAIT | DDBLT_KEYSRC );

	//
	DrawFzmBackToSurface( s_ExitButtonPos.x, s_ExitButtonPos.y, s_lpDDS_Exit, 0, 0, 64, 34, DDBLT_WAIT | DDBLT_KEYSRC  );

	//
	if( s_iFzmFocusName >= 0 && s_iFzmFocusName != s_iFzmActiveName )
	{
		lpZoomMapInfo  = &s_FzmInfo[ s_iFzmFocusName ];
		lpZoomMapImage = &s_FzmImage[ lpZoomMapInfo->eNameType-1 ];

		DrawFzmBackToSurface( lpZoomMapInfo->iNamePosX-1, lpZoomMapInfo->iNamePosY, lpZoomMapImage->lpDDS_Name,
							  0, 0, lpZoomMapImage->iCheckWideh, lpZoomMapImage->iCheckHeight, DDBLT_WAIT | DDBLT_KEYSRC );
	}

	//
	if( s_iFzmActiveName >= 0 )
	{
		lpZoomMapInfo  = &s_FzmInfo[ s_iFzmActiveName ];
		lpZoomMapImage = &s_FzmImage[ lpZoomMapInfo->eNameType-1 ];

		DrawFzmBackToSurface( lpZoomMapInfo->iNamePosX, lpZoomMapInfo->iNamePosY, lpZoomMapImage->lpDDS_Name,
							  0, 0, lpZoomMapImage->iCheckWideh, lpZoomMapImage->iCheckHeight, DDBLT_WAIT | DDBLT_KEYSRC );
	}

	//
	lpD3DDevice->BeginScene();
	DrawFzmEffect();
	lpD3DDevice->EndScene();
}


//######################################################################################
//작 성 일 : 2004-04-08 오후 3:32:13
//리 턴 값 :
//설    명 :
//작 성 자 : 오 영 석
//######################################################################################
void MainFullZoomMap(void)
{
	//
	int st_x, st_y, end_x, end_y;

	//
	if( lpCurPlayer->OnStageField >= 0 )
	{
		if( lpCurPlayer->pX != s_CurPlayerPos.x || lpCurPlayer->pZ != s_CurPlayerPos.y )
		{
			if( s_iFzmActiveName != FzmFieldCode( StageField[lpCurPlayer->OnStageField]->FieldCode ) )
				SetFullZoomMap(1);
		}

		s_CurPlayerPos.x = lpCurPlayer->pX;
		s_CurPlayerPos.y = lpCurPlayer->pZ;
	}

	//
	MainFzmEffect();

	//
	st_x  = int(682 * g_fWinSizeRatio_X);
	st_y  = int(547 * g_fWinSizeRatio_Y);
	end_x = int((682 + 64) * g_fWinSizeRatio_X);
	end_y = int((547 + 34) * g_fWinSizeRatio_Y);

	if( g_iFzmCursorPosX > st_x && g_iFzmCursorPosX < end_x &&
		g_iFzmCursorPosY > st_y && g_iFzmCursorPosY < end_y )
	{
		if( s_lpDDS_Exit == s_lpDDS_ExitImage )
		{
			s_lpDDS_Exit = s_lpDDS_ExitFocusImage;

			s_iMouseLButtonState[0] = g_FzmMouseButton[0];
			s_iMouseLButtonState[1] = -1;
		}

		if( s_iMouseLButtonState[0] == 0 )
		{
			if( s_iMouseLButtonState[1] == 1 )
			{
				s_ExitButtonPos.x = 682+1;
				s_ExitButtonPos.y = 547+1;

				if( g_FzmMouseButton[0] == 0 )
					SetFullZoomMap(0);
			}
			else
				s_iMouseLButtonState[1] = g_FzmMouseButton[0];
		}
		else
			s_iMouseLButtonState[0] = g_FzmMouseButton[0];
	}
	else
	{
		s_lpDDS_Exit      = s_lpDDS_ExitImage;
		s_ExitButtonPos.x = 682;
		s_ExitButtonPos.y = 547;
	}

	
	//
	SFullZoomMapInfo  *lpZoomMapInfo;
	SFullZoomMapImage *lpZoomMapImage;

	int i; for( i =0; i < EFzmNameMax; i++ )
	{
		lpZoomMapInfo  = &s_FzmInfo[ i ];
		lpZoomMapImage = &s_FzmImage[ lpZoomMapInfo->eNameType-1 ];

		st_x  = int(lpZoomMapInfo->iNamePosX * g_fWinSizeRatio_X);
		st_y  = int(lpZoomMapInfo->iNamePosY * g_fWinSizeRatio_Y);
		end_x = int((lpZoomMapInfo->iNamePosX + lpZoomMapImage->iCheckWideh)  * g_fWinSizeRatio_X);
		end_y = int((lpZoomMapInfo->iNamePosY + lpZoomMapImage->iCheckHeight) * g_fWinSizeRatio_Y);

		if( g_iFzmCursorPosX > st_x && g_iFzmCursorPosX < end_x &&
			g_iFzmCursorPosY > st_y && g_iFzmCursorPosY < end_y )
		{
			if( s_iFzmFocusName == i )
			{
				if( s_iMouseLButtonState[0] == 0 )
				{
					if( s_iMouseLButtonState[1] == 1 )
					{
						if( g_FzmMouseButton[0] == 0 )
						{
							//
							s_iMouseLButtonState[1] = 0;

							//
							LoadZoomInMap( lpZoomMapInfo );
							s_iFzmActiveName	 = i;

							//
							SetFzmEffect(NAME_EFFECT, &s_NameEffectDataInfo, 2, s_iFzmActiveName);		//미니맵 네임  클릭했을때 이벤트 
						}
					}
					else
						s_iMouseLButtonState[1] = g_FzmMouseButton[0];
				}
				else
					s_iMouseLButtonState[0] = g_FzmMouseButton[0];
			}
			else
			{
				s_iFzmFocusName = i;

				s_iMouseLButtonState[0] = g_FzmMouseButton[0];
				s_iMouseLButtonState[1] = -1;
			}

			break;
		}
	}

	if( i >= EFzmNameMax )
		s_iFzmFocusName = -1;

	//
    SetFzmCharPos();
}



//######################################################################################
//작 성 일 : 2004-05-06 오후 5:47:27
//리 턴 값 :
//설    명 :
//작 성 자 : 오 영 석
//######################################################################################
int KeyFullZoomMap( WPARAM wParam )
{
	// VK_F1 ~ VK_F8
	if( wParam >= 0x70 && wParam <= 0x77 )
		return 1;

	// VK_END, VK_HOME, VK_LEFT, VK_UP, VK_RIGHT, VK_DOWN
	if( wParam >= 0x23 && wParam <= 0x28 )
		return 1;

	// 1 key,  2 key,  3 key  
	if( wParam == VK_CONTROL || wParam == 0x31 || wParam == 0x32 || wParam == 0x33 )
		return 1;
	
	return 0;
}

