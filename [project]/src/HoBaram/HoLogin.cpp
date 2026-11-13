#include "..\\sinbaram\\SinLinkHeader.h"
#include "..\\HoBaram\\HoLinkHeader.h"
#include "..\\HoBaram\\HoTextFile.h"
#include "..\\HoBaram\\HoMinMax.h"
#include "..\\HoBaram\\HoNewParticle.h"
#include "..\\HoBaram\\HoNewParticleMgr.h"

#include "..\\tjboy\\clanmenu\\tjclanDEF.h"
#include "..\\tjboy\\clanmenu\\tjclan.h"
#include "..\\CurseFilter.h"		//IsCurse 함수(해외)

//#define NOTUSE_MOR		//모라이언을 사용하지않을때는 define을 사용할것.



int  CharacterPosX = 0;
int  CharacterPosZ = 0;

int  MouseDblClick;

char SelectCharacterName1[3][500];
char SelectCharacterName2[3][500];

char JobSelectCharacterName1[4][500];
char JobSelectCharacterName2[4][500];


// where an account's characters stand on the select screen: a shallow arc, two to a row
static int UserCharacterPos[CHARACTER_USER_MAX][2] =
{
	{ -25 ,-110 },
	{  25 ,-110 },
	{ -65 ,-90 },
	{  65 ,-90 },
	{ -100 ,-60 },
	{ 100 ,-60 }
};

// where the job screen's classes stand: a shallow arc, set for the race's class count by
// SetJobCharacterLayout() (four classes give the original -75/-25/25/75 and -320/-330)
static int JobCharacterPos[CHARACTER_JOB_MAX][2] = {
	{ -75, -320},
	{ -25, -330},
	{  25, -330},
	{  75, -320}
};

static void SetJobCharacterLayout( int count )
{
	int step = count<=4 ? 25 : 21;			// half the spacing
	for( int cnt=0; cnt<CHARACTER_JOB_MAX; cnt++ ) {
		int d = 2*cnt-(count-1);				// -(count-1) .. count-1, in half spaces
		if ( d<0 ) d = -d;
		JobCharacterPos[cnt][0] = (2*cnt-(count-1))*step;
		JobCharacterPos[cnt][1] = -330 + (d*d-1)*10/8;
	}
}

static int CharacterInfoPosi[5] = {362, 243, 488, 154, 588 };
static char *szFileSpark[] = {
	"cSelect\\001.tga",
	"cSelect\\002.tga",
	"cSelect\\003.tga",
	"cSelect\\004.tga",
	"cSelect\\005.tga",
	"cSelect\\006.tga",
	"cSelect\\007.tga",
	"cSelect\\008.tga"
};
static char *szFileFlame[] = {
	"cSelect\\flame1.tga",
	"cSelect\\flame2.tga",
	"cSelect\\flame3.tga",
	"cSelect\\flame4.tga",
	"cSelect\\flame5.tga",
	"cSelect\\flame6.tga",
	"cSelect\\flame7.tga",
	"cSelect\\flame8.tga"
};







//Tempscron 4개 직업의 몸통 이름...
static char *szTempFighterBodyName[] = {
	"char\\tmABCD\\b001.ini",
	"char\\tmABCD\\b002.ini",
	"char\\tmABCD\\b003.ini",
	"char\\tmABCD\\b004.ini",
	"char\\tmABCD\\b005.ini",
	"char\\tmABCD\\b006.ini",
	"char\\tmABCD\\b007.ini"
};
static char *szTempMechBodyName[] = {
	"char\\tmABCD\\a001.ini",
	"char\\tmABCD\\a002.ini",
	"char\\tmABCD\\a003.ini",
	"char\\tmABCD\\a004.ini",
	"char\\tmABCD\\a005.ini",
	"char\\tmABCD\\a006.ini",
	"char\\tmABCD\\a007.ini"
};
static char *szTempArcherBodyName[] = {
	"char\\tmABCD\\d001.ini",
	"char\\tmABCD\\d002.ini",
	"char\\tmABCD\\d003.ini",
	"char\\tmABCD\\d004.ini",
	"char\\tmABCD\\d005.ini",
	"char\\tmABCD\\d006.ini",
	"char\\tmABCD\\d007.ini"
};
static char *szTempPikeBodyName[] = {
	"char\\tmABCD\\c001.ini",
	"char\\tmABCD\\c002.ini",
	"char\\tmABCD\\c003.ini",
	"char\\tmABCD\\c004.ini",
	"char\\tmABCD\\c005.ini",
	"char\\tmABCD\\c006.ini",
	"char\\tmABCD\\c007.ini"
};

//Tempscron 4개 직업별 얼굴...
char *szTempFighterFaceName[2][3] = {
	{
		"char\\tmABCD\\tmh-b01.inf",
		"char\\tmABCD\\tmh-b02.inf",
		"char\\tmABCD\\tmh-b03.inf"
	},
	{
		"char\\tmABCD\\tmh-b01a.inf",
		"char\\tmABCD\\tmh-b02a.inf",
		"char\\tmABCD\\tmh-b03a.inf"
	}

};

char *szTempMechFaceName[2][3] = {
	{
		"char\\tmABCD\\tmh-a01.inf",
		"char\\tmABCD\\tmh-a02.inf",
		"char\\tmABCD\\tmh-a03.inf"
	},
	{
		"char\\tmABCD\\tmh-a01a.inf",
		"char\\tmABCD\\tmh-a02a.inf",
		"char\\tmABCD\\tmh-a03a.inf"
	}
};

char *szTempArcherFaceName[2][3] = {
	{	"char\\tmABCD\\tfh-D01.inf",
		"char\\tmABCD\\tfh-d02.inf",
		"char\\tmABCD\\tfh-d03.inf"
	},
	{
		"char\\tmABCD\\tfh-D01a.inf",
		"char\\tmABCD\\tfh-d02a.inf",
		"char\\tmABCD\\tfh-d03a.inf"
	}
};

char *szTempPikeFaceName[2][3] = {
	{
		"char\\tmABCD\\tmh-c01.inf",
		"char\\tmABCD\\tmh-c02.inf",
		"char\\tmABCD\\tmh-c03.inf"
	},
	{
		"char\\tmABCD\\tmh-c01a.inf",
		"char\\tmABCD\\tmh-c02a.inf",
		"char\\tmABCD\\tmh-c03a.inf"
	}
};

#ifndef NOTUSE_MOR	//=============================================================
//ktj : 새로만듦. 모라이언 애니메인션 용임. start 
//Tempscron 4개 직업의 몸통 이름...

static char *szMorMechBodyName[] = {
	"char\\tmABCD\\ma001.ini",
	"char\\tmABCD\\ma002.ini",
	"char\\tmABCD\\ma003.ini",
	"char\\tmABCD\\ma004.ini",
	"char\\tmABCD\\ma005.ini",
	"char\\tmABCD\\ma006.ini",
	"char\\tmABCD\\ma007.ini"
};

static char *szMorFighterBodyName[] = {
	"char\\tmABCD\\mb001.ini",
	//"char\\tmABCD\\ma001.ini",
	
	
	"char\\tmABCD\\mb002.ini",
	"char\\tmABCD\\mb003.ini",
	"char\\tmABCD\\mb004.ini",
	"char\\tmABCD\\mb005.ini",
	"char\\tmABCD\\mb006.ini",
	"char\\tmABCD\\mb007.ini"
};

static char *szMorPikeBodyName[] = {
	"char\\tmABCD\\mc001.ini",
	"char\\tmABCD\\mc002.ini",
	"char\\tmABCD\\mc003.ini",
	"char\\tmABCD\\mc004.ini",
	"char\\tmABCD\\mc005.ini",
	"char\\tmABCD\\mc006.ini",
	"char\\tmABCD\\mc007.ini"
};

static char *szMorArcherBodyName[] = {
	"char\\tmABCD\\md001.ini",
	"char\\tmABCD\\md002.ini",
	"char\\tmABCD\\md003.ini",
	"char\\tmABCD\\md004.ini",
	"char\\tmABCD\\md005.ini",
	"char\\tmABCD\\md006.ini",
	"char\\tmABCD\\md007.ini"
};



char *szMorMechFaceName[2][3] = {
	{
		"char\\tmABCD\\Mmh-A01.inf",	//ktj : 아직 안들어와있음.
		//"char\\tmABCD\\mmh-a02.inf",
		//"char\\tmABCD\\mmh-a03.inf"
		"char\\tmABCD\\Mmh-A02.inf",
		"char\\tmABCD\\Mmh-A03.inf"
	},
	{
		//"char\\tmABCD\\mmh-a01a.inf",
		//"char\\tmABCD\\mmh-a02a.inf",
		//"char\\tmABCD\\mmh-a03a.inf"

		"char\\tmABCD\\Mmh-A01.inf",	//ktj : 아직 안들어와있음.
		"char\\tmABCD\\Mmh-A02.inf",
		"char\\tmABCD\\Mmh-A03.inf"
	}
};

//Tempscron 4개 직업별 얼굴...
char *szMorFighterFaceName[2][3] = {
	/*								//ktj : 아직 안들어와있음. 일단 임시
	{
		"char\\tmABCD\\mmh-b01.inf",
		"char\\tmABCD\\mmh-b02.inf",
		"char\\tmABCD\\mmh-b03.inf"
	},
	{
		"char\\tmABCD\\mmh-b01a.inf",
		"char\\tmABCD\\mmh-b02a.inf",
		"char\\tmABCD\\mmh-b03a.inf"
	}
	*/
	{
		"char\\tmABCD\\Mfh-B01.inf",
		"char\\tmABCD\\Mfh-B02.inf",
		"char\\tmABCD\\Mfh-B03.inf"
	},
	{
		"char\\tmABCD\\Mfh-B01.inf",
		"char\\tmABCD\\Mfh-B02.inf",
		"char\\tmABCD\\Mfh-B03.inf"
	}
	
	/*
	{
		"char\\tmABCD\\Mmh-a01.inf",
		"char\\tmABCD\\Mmh-a02.inf",
		"char\\tmABCD\\Mmh-a03.inf"
	},
	{
		"char\\tmABCD\\Mmh-a01.inf",
		"char\\tmABCD\\Mmh-a02.inf",
		"char\\tmABCD\\Mmh-a03.inf"
	}
	*/
	

};


char *szMorPikeFaceName[2][3] = {
	/*
	{
		"char\\tmABCD\\mmh-c01.inf",
		"char\\tmABCD\\mmh-c02.inf",
		"char\\tmABCD\\mmh-c03.inf"
	},
	{
		"char\\tmABCD\\mmh-c01a.inf",
		"char\\tmABCD\\mmh-c02a.inf",
		"char\\tmABCD\\mmh-c03a.inf"
	}
	*/
	{
		"char\\tmABCD\\Mfh-C01.inf",
		"char\\tmABCD\\Mfh-C02.inf",
		"char\\tmABCD\\Mfh-C03.inf"
	},
	{
		"char\\tmABCD\\Mfh-C01.inf",
		"char\\tmABCD\\Mfh-C02.inf",
		"char\\tmABCD\\Mfh-C03.inf"
	}
};

char *szMorArcherFaceName[2][3] = {
	/*
	{	"char\\tmABCD\\mfh-D01.inf",
		"char\\tmABCD\\mfh-d02.inf",
		"char\\tmABCD\\mfh-d03.inf"
	},
	{
		"char\\tmABCD\\mfh-D01a.inf",
		"char\\tmABCD\\mfh-d02a.inf",
		"char\\tmABCD\\mfh-d03a.inf"
	}
	*/
	{	"char\\tmABCD\\Mmh-D01.inf",
		"char\\tmABCD\\Mmh-D02.inf",
		"char\\tmABCD\\Mmh-D03.inf"
	},
	{
		"char\\tmABCD\\Mmh-D01.inf",
		"char\\tmABCD\\Mmh-D02.inf",
		"char\\tmABCD\\Mmh-D03.inf"
	}
};



int MorNewCharacterInit[NEW_CHAR_JOB_MAX][6] =
{
	//JobCode, Strentch, Sprit, Talent, Defence, Health
	{6,        26,       13,     17,     19,      24}, //Knight
	{5,        23,       15,     19,     19,      23}, //Atalanta
	{8,        15,       28,     21,     15,      20}, //프리티스
	{7,        16,      29,     19,     14,      21}, //매지션
	{10,       15,       27,     20,     15,      22}, //Shaman
	{0,         0,        0,      0,      0,       0}  //(none: Morion has five classes)
};

// the Shaman (Morion male E): body me001-me030, heads Mmh-E01..13 (a/b/c/d/t for 01..03)
static char *szMorShamanBodyName[] = {
	"char\\tmABCD\\me001.ini"
};
char *szMorShamanFaceName[2][3] = {
	{
		"char\\tmABCD\\Mmh-E01.inf",
		"char\\tmABCD\\Mmh-E02.inf",
		"char\\tmABCD\\Mmh-E03.inf"
	},
	{
		"char\\tmABCD\\Mmh-E01a.inf",
		"char\\tmABCD\\Mmh-E02a.inf",
		"char\\tmABCD\\Mmh-E03a.inf"
	}
};

//ktj : 새로만듦. end 
#endif		//=============================================================












static int TempFaceCount[CHARACTER_JOB_MAX] =
{ 3, 3, 3, 3, 3, 3 };

static int	TempFacePos[CHARACTER_JOB_MAX][4] =
{
	{400-64-128,    400-64,   400-64+128,           -1},
	{400-64-128,	400-64,   400-64+128,           -1},
	{400-64-128,    400-64,   400-64+128,			-1},
	{400-64-128,    400-64,   400-64+128,			-1},
	{400-64-128,    400-64,   400-64+128,			-1},
	{400-64-128,    400-64,   400-64+128,			-1}
};

int TempNewCharacterInit[NEW_CHAR_JOB_MAX][6] =
{
	//JobCode, Strentch, Sprit, Talent, Defence, Health
	{2,        24,       8,     25,     18,      24}, //Mech
	{1,        28,       6,     21,     17,      27}, //Fighter
	{4,        26,       9,     20,     19,      25}, //PikeMan
	{3,        17,      11,     21,     27,      23}, //Archer
	{9,        25,      10,     22,     20,      22}, //Assassin
	{11,       26,       9,     20,     20,      24}  //Martial Artist
};

// the Assassin (Tempskron female E): body e001-e030, heads Tfh-E01..13 (a/b/c/d/t for 01..03)
static char *szTempAssassinBodyName[] = {
	"char\\tmABCD\\e001.ini"
};
char *szTempAssassinFaceName[2][3] = {
	{
		"char\\tmABCD\\Tfh-E01.inf",
		"char\\tmABCD\\Tfh-E02.inf",
		"char\\tmABCD\\Tfh-E03.inf"
	},
	{
		"char\\tmABCD\\Tfh-E01a.inf",
		"char\\tmABCD\\Tfh-E02a.inf",
		"char\\tmABCD\\Tfh-E03a.inf"
	}
};

// the Martial Artist (Tempskron female F): body f001-f031, heads tfh-f01..13 (a/b/c/d/t for 01..03)
static char *szTempMartialBodyName[] = {
	"char\\tmABCD\\f001.ini"
};
char *szTempMartialFaceName[2][3] = {
	{
		"char\\tmABCD\\tfh-f01.inf",
		"char\\tmABCD\\tfh-f02.inf",
		"char\\tmABCD\\tfh-f03.inf"
	},
	{
		"char\\tmABCD\\tfh-f01a.inf",
		"char\\tmABCD\\tfh-f02a.inf",
		"char\\tmABCD\\tfh-f03a.inf"
	}
};

// The job screen's classes, in the order they stand, per race: their body, heads and new
// character stats (rows of TempNewCharacterInit / MorNewCharacterInit, in the same order).
static int LoginJobCount( int tribe )
{
	return tribe==TRIBE_Moryon ? 5 : 6;
}

static char *LoginBodyName( int tribe, int job )
{
	static char **temp[6] = { szTempMechBodyName, szTempFighterBodyName, szTempPikeBodyName, szTempArcherBodyName, szTempAssassinBodyName, szTempMartialBodyName };
	static char **mor[5]  = { szMorMechBodyName, szMorFighterBodyName, szMorPikeBodyName, szMorArcherBodyName, szMorShamanBodyName };
	if ( job<0 || job>=LoginJobCount(tribe) ) job = 0;
	return tribe==TRIBE_Moryon ? mor[job][0] : temp[job][0];
}

static char *LoginFaceName( int tribe, int job, int face )
{
	static char *(*temp[6])[3] = { szTempMechFaceName, szTempFighterFaceName, szTempPikeFaceName, szTempArcherFaceName, szTempAssassinFaceName, szTempMartialFaceName };
	static char *(*mor[5])[3]  = { szMorMechFaceName, szMorFighterFaceName, szMorPikeFaceName, szMorArcherFaceName, szMorShamanFaceName };
	if ( job<0 || job>=LoginJobCount(tribe) ) job = 0;
	if ( face<0 || face>=3 ) face = 0;
	return tribe==TRIBE_Moryon ? mor[job][0][face] : temp[job][0][face];
}

static int *LoginNewCharInit( int tribe, int job )
{
	if ( job<0 || job>=LoginJobCount(tribe) ) job = 0;
	return tribe==TRIBE_Moryon ? MorNewCharacterInit[job] : TempNewCharacterInit[job];
}

HoNewCharInfo	NewCharInfo;
//마우스가 캐릭터가 위에 있는지 확인 
static int GetSelRectChar( smCHAR *lpChar);

//마우스가 Job캐릭터 위에 잇는지 확인
static int GetSelRectJobChar( smCHAR *lpChar );
static int CameraZPos[5] = { fONE*300, fONE*300+fONE*70, fONE*300+fONE*250, fONE*300+fONE*420, fONE*300+fONE*400};

HoLogin::HoLogin()
{
	

}
HoLogin::HoLogin(int sw)
{
	ErrorSelect = ERROR_NONE;
	SelectCurrentState = CHARACTER_SELECT_STATE;
	SelectNextState = CHARACTER_SELECT_STATE;

	//사용하는 Character들 Count
	UserCharacterCount = 0;
	JobCharacterCount = 0;
	
//	CharacterSelectImage = NULL;
//	RaceSelectImage = NULL;
//	JobSelectImage = NULL;
	FaceSelectImage = NULL;
//	NameSelectImage = NULL;
	
	TempscronMarkImage = -1;
	TempscronLogoImage = -1;
	
	GameMapImage = NULL;



#ifndef NOTUSE_MOR 
	
	//ktj : 새로만듦. start
	MoryonMarkImage = -1;	//MoryonMark Image 번호.
	MoryonLogoImage = -1;	//MoryonLogo Image 번호.
	Init_UseMor(sw);

	//ktj : 새로만듦. end
		
#endif



//	ManageCharacterFlag = FALSE;
	//Init();
	Load();

}

HoLogin::~HoLogin()
{
	Close();
}



void HoLogin::Init_UseMor(int flag)
{
	USE_MOR = flag;
	if(USE_MOR ==0) {

		//RECT menuPos[2];		//메뉴셋팅.
		menuPos[0].left = 336;//270;//336,
		menuPos[0].right= menuPos[0].left+128,
		menuPos[0].top = 166,
		menuPos[0].bottom = menuPos[0].top+128;

		menuPos[1].left = -1;
		menuPos[1].right= -1;
		menuPos[1].top = -1;
		menuPos[1].bottom = -1;

	}
	else {

		//RECT menuPos[2];		//메뉴셋팅.
		menuPos[0].left = 270;//336,
		menuPos[0].right= menuPos[0].left+128,
		menuPos[0].top = 166,
		menuPos[0].bottom = menuPos[0].top+128;

		menuPos[1].left = 430;//150,
		menuPos[1].right= menuPos[1].left+128,
		menuPos[1].top = 166,
		menuPos[1].bottom = menuPos[1].top+128;
		//ktj : 새로만듦. end
	}

}



extern smCHAR chrPlayer;
int HoLogin::Init()
{
	StartErrorDelayFlag = FALSE;
	ClanMarkUpdateSuccessFlag = FALSE;
	g_NewParticleMgr.Init();
	InitAssaEffect();
	POINT3D pos;
	
	pos.x = -70*fONE;
	pos.y = 55*fONE;
	pos.z = -156*fONE;
	
	g_NewParticleMgr.Start("Fire", pos);
	g_NewParticleMgr.Start("Fire", pos);
	
	pos.x = -77*fONE;
	pos.y = 60*fONE;
	pos.z = -143*fONE;
	g_NewParticleMgr.Start("Fire", pos);

	pos.x = -81*fONE;
	pos.y = 65*fONE;
	pos.z = -143*fONE;
	g_NewParticleMgr.Start("Fire", pos);
	
	
	pos.x = 70*fONE;
	pos.y = 56*fONE;
	pos.z = -155*fONE;
	g_NewParticleMgr.Start("Fire", pos);
	
	pos.x = 75*fONE;
	pos.y = 60*fONE;
	pos.z = -152*fONE;
	g_NewParticleMgr.Start("Fire", pos);

	pos.x = 81*fONE;
	pos.y = 66*fONE;
	pos.z = -142*fONE;
	g_NewParticleMgr.Start("Fire", pos);
	
	/*
	pos.x = 0;
	pos.y = 10*fONE;
	pos.z = 0;
	g_NewParticleMgr.Start("NewCharacter", pos);
	*/
	
	FocusWndFlag = FALSE;
	CharacterStopFlag = FALSE;
	CharacterReturnFlag = FALSE;
	


	NewCharacterImageBlendCount = 50;
	NewCharacterImageBlendFlag = false;
	
	
	
	MapBlendCount = 0;
	MapBlendFlag = FALSE;
	memset(UserCharacterNewName, 0, sizeof(UserCharacterNewName));
	
	FadeCurrentState = FADE_OUT;
	FadeNextState    = FADE_OUT;
	
	SelectCurrentState = CHARACTER_SELECT_STATE;
	SelectNextState    = CHARACTER_SELECT_STATE;
	SelectSubState     = SUB_SELECT_NONE;

	//Stage관련 초기화.
	TarDist = CameraZPos[SelectCurrentState];
	CaAngle.x = 90;
	CaAngle.y = 0;
	CaAngle.z = 0;
	TarPosi.x = 0;
	TarPosi.y = 16*fONE;
	TarPosi.z = 0;
	
	smRender.SMMULT_PERSPECTIVE_HEIGHT = RENDCLIP_DEFAULT_MULT_PERSPECTIVE_HEIGHT;
	MidX = WinSizeX	/2;
	MidY = WinSizeY	/2;
		
	MouseDownCharacterNum	= -1;
	MouseMoveCharacterNum	= -1;
	JobBodySelectNum		= -1;
	JobFaceSelectNum		= -1;
	
	RaceMarkBlendCount = 0;
	
	FadeCurrentState = FADE_OUT;
	FadeNextState = FADE_OUT;
	
	
	StopBGM();
	OpenBGM("StartImage\\Login\\CharacterSelect.wav");
	PlayBGM();
	
	PlayWav2(9-1, 1);
	PlayWav2(10-1, 1);
	PlayWav2(11-1, 1);

	SetVolumeBGM(0);
	PlayWav_VolChange(9-1, 0);
	PlayWav_VolChange(10-1, 0);
	PlayWav_VolChange(11-1, 0);

	SetFade();
	smInitColorZClip(0, 0, 0);
	smRender.Color_R = 0;
	smRender.Color_G = 0;
	smRender.Color_B = 0;
	smRender.Color_A = 0;
	
	//크랜 마크 초기화
	void InitSelectCha();
	InitSelectCha();

	//ktj : 웹db용 클랜설정함수.  ============================== start
	void web_DB_start();
	web_DB_start();		//안전

	//ktj : 
	void clanMENU_INIT(int setN);
	clanMENU_INIT(0);
	//ktj : 웹db용 임시설정함수.  ============================== end
	
	
	
	void ClearSelectCha();
	ClearSelectCha();



	int HONG_GetChaClanName(char* id, char* chaname);
	if(DownUserCharacterCount > 0)
	{
		//UserCharacter
		for(int index = 0; index < DownUserCharacterCount; index++)
		{
			AddUserCharacter( DownUserCharacterInfo[index].BodyName, DownUserCharacterInfo[index].FaceName, DownUserCharacterInfo[index].Job);
	 		HONG_GetChaClanName(UserAccount, DownUserCharacterInfo[index].Name);
		}
		ReadTextures();
	}
	
 	m_ScreenHalf = smScreenWidth/2;
	return TRUE;
}

extern int MatEachBox;
//초기화 캐릭터 선택
int HoLogin::Load()
{

#ifdef _LANGUAGE_ENGLISH
#ifdef	_CUSTOM_FONT
	hFont = PT_CreateFont( 16,
                        0,
                        0,
                        0,
						FW_NORMAL,
                        FALSE,
                        FALSE,
                        FALSE,
                        DEFAULT_CHARSET,
                        OUT_DEFAULT_PRECIS,
                        CLIP_DEFAULT_PRECIS,
                        DEFAULT_QUALITY,
                        VARIABLE_PITCH,
                        "Comic Sans MS" );
#else
	Font = PT_CreateFont( 16,
                    7,
                    0,
                    0,
					FW_NORMAL,
                    FALSE,
                    FALSE,
                    FALSE,
					DEFAULT_CHARSET,
                    OUT_DEFAULT_PRECIS,
                    CLIP_DEFAULT_PRECIS,
                    DEFAULT_QUALITY,
                    VARIABLE_PITCH,
	   	            "Comic Sans MS" );
#endif
#endif

#ifdef _LANGUAGE_TAIWAN
	Font = PT_CreateFont( 12,
                    0,
                    0,
                    0,
					FW_NORMAL,
                    FALSE,
                    FALSE,
                    FALSE,
					CHINESEBIG5_CHARSET,
                    OUT_DEFAULT_PRECIS,
                    CLIP_DEFAULT_PRECIS,
                    DEFAULT_QUALITY,
                    VARIABLE_PITCH,
	   	            "MingLiu" );
#endif


#ifdef _LANGUAGE_JAPANESE
	Font = PT_CreateFont( 12,
                    0,
                    0,
                    0,
					FW_NORMAL,
                    FALSE,
                    FALSE,
                    FALSE,
					SHIFTJIS_CHARSET,
                    OUT_DEFAULT_PRECIS,
                    CLIP_DEFAULT_PRECIS,
                    DEFAULT_QUALITY,
                    VARIABLE_PITCH,
	   	            "MS GOTHIC" );
#endif


#ifdef _LANGUAGE_CHINESE
	Font = PT_CreateFont( 12,
                    0,
                    0,
                    0,
					FW_NORMAL,
                    FALSE,
                    FALSE,
                    FALSE,
					GB2312_CHARSET,
                    OUT_DEFAULT_PRECIS,
                    CLIP_DEFAULT_PRECIS,
                    DEFAULT_QUALITY,
                    VARIABLE_PITCH,
	   	            "SimSun" );
#endif
#ifdef _LANGUAGE_VEITNAM
	Font = PT_CreateFont( 16,
                    0,
                    0,
                    0,
					FW_NORMAL,
                    FALSE,
                    FALSE,
                    FALSE,
					VIETNAMESE_CHARSET,
                    OUT_DEFAULT_PRECIS,
                    CLIP_DEFAULT_PRECIS,
                    DEFAULT_QUALITY,
                    VARIABLE_PITCH,
	   	            "Tahoma" );
#endif
#ifdef _LANGUAGE_BRAZIL
	Font = PT_CreateFont( 16,
                    0,
                    0,
                    0,
					FW_NORMAL,
                    FALSE,
                    FALSE,
                    FALSE,
					ANSI_CHARSET|FS_LATIN1,
                    OUT_DEFAULT_PRECIS,
                    CLIP_DEFAULT_PRECIS,
                    DEFAULT_QUALITY,
                    VARIABLE_PITCH,
	   	            "Comic Sans MS" );
#endif
#ifdef _LANGUAGE_ARGENTINA
	Font = PT_CreateFont( 16,
                    0,
                    0,
                    0,
					FW_NORMAL,
                    FALSE,
                    FALSE,
                    FALSE,
					DEFAULT_CHARSET|FS_LATIN2,//EASTEUROPE_CHARSET,
					//ANSI_CHARSET|FS_LATIN1,
                    OUT_DEFAULT_PRECIS,
                    CLIP_DEFAULT_PRECIS,
                    DEFAULT_QUALITY,
                    VARIABLE_PITCH,
	   	            "Tahoma" );
#endif
#ifdef _LANGUAGE_THAI
#ifdef _CUSTOM_FONT
/*#ifdef	_ZHOON_TEST
	Font = PT_CreateFont(z_fonttest.setdata[1].z_nHeight,
						z_fonttest.setdata[1].z_nWidth,
						z_fonttest.setdata[1].z_nEscapement,
						z_fonttest.setdata[1].z_nOrientation,
						z_fonttest.setdata[1].z_nWeight,
						z_fonttest.setdata[1].z_bItalic,
						z_fonttest.setdata[1].z_bUnderline,
						z_fonttest.setdata[1].z_cStrikeOut,
						z_fonttest.setdata[1].z_nCharSet,
						z_fonttest.setdata[1].z_nOutPrecision,
						z_fonttest.setdata[1].z_nClipPrecision,
						z_fonttest.setdata[1].z_nQuality,
                      z_fonttest.setdata[1].z_nPitchAndFamily,
						z_fonttest.setdata[1].z_lpszFacename);
#else*/

	Font = PT_CreateFont( 13,
                    0,
                    0,
                    0,
					FW_THIN,
                    FALSE,
                    FALSE,
                    FALSE,
					THAI_CHARSET,
						OUT_TT_PRECIS,
						CLIP_DEFAULT_PRECIS,
						ANTIALIASED_QUALITY,
                        DEFAULT_PITCH | FF_DONTCARE,
					 "MS Sans Serif");
//#endif
#else
	Font = PT_CreateFont( 20,
                    0,
                    0,
                    0,
					FW_NORMAL,
                    FALSE,
                    FALSE,
                    FALSE,
					THAI_CHARSET,
						OUT_TT_PRECIS,
						CLIP_DEFAULT_PRECIS,
						DRAFT_QUALITY,
                        VARIABLE_PITCH|FF_SWISS,
					 "FreesiaUPC");
#endif
#endif


#ifdef _LANGUAGE_KOREAN
	Font = PT_CreateFont( 12,
                    0,
                    0,
                    0,
					FW_NORMAL,
                    FALSE,
                    FALSE,
                    FALSE,
                    HANGEUL_CHARSET,
                    OUT_DEFAULT_PRECIS,
                    CLIP_DEFAULT_PRECIS,
                    DEFAULT_QUALITY,
                    VARIABLE_PITCH,
                    "\261\274\270\262\303\274" );
#endif

	
	Stage = new smSTAGE3D();

	// 라이트 벡터
	Stage->VectLight.x = smConfig.MapLightVector.x;
	Stage->VectLight.y = smConfig.MapLightVector.y;
	Stage->VectLight.z = smConfig.MapLightVector.z;

	Stage->Bright		= smConfig.MapBright;
	Stage->Contrast	= smConfig.MapContrast;

	Stage = smSTAGE3D_ReadASE( "cSelect\\Login.ASE" , smStage );
	if ( Stage )
	{
		Stage->Head = TRUE;
		//smStage->StageObject->AddObjectFile( "cSelect\\Select.ASE" );
		
		//smStage->StageObject->AddObjectFile( "EffectImage\\Test.ase");
		//smPAT3D *Pat = smASE_Read( "StartImage\\Login\\StandPlane.ASE");
		//smStage->StageObject->AddObject( Pat, JobCharacterPos[0][0]*fONE, -500, JobCharacterPos[0][1]*fONE, 0, 0, 0);
	}

	
	/*
	MatNoise = smStage->smMaterialGroup->FindMaterialFromTextureName( "cSelect\\RI.tga" );
	if ( MatNoise )
	{
		lstrcpy( MatNoise->smTexture[0]->Name , "cSelect\\RI01.bmp" );
		MatNoise->BlendType = SMMAT_BLEND_COLOR;
	}
	
	
	smMATERIAL *MatNoise=0;
	MatNoise = smStage->smMaterialGroup->FindMaterialFromTextureName( "cSelect\\001.bmp" );
	if ( MatNoise )
	{
		smStage->smMaterialGroup->ChangeMaterialToAnimation( 
				MatNoise , szFileSpark , 8 , SMMAT_BLEND_COLOR , TRUE );
	}

	MatNoise = smStage->smMaterialGroup->FindMaterialFromTextureName( "cSelect\\flame1.tga" );
	if ( MatNoise )
	{
		smStage->smMaterialGroup->ChangeMaterialToAnimation( 
				MatNoise , szFileFlame , 8 , SMMAT_BLEND_COLOR , TRUE );
	}
	*/

	ReadTextures();
	if(Stage)
	{
		Stage->smMaterialGroup->CheckMatreialTextureSwap();
	}

	//Button Image읽기...

	//OkButton.Init("StartImage\\Login\\ButtonOk.tga", "StartImage\\Login\\ButtonBall.tga");
	//BackButton.Init("StartImage\\Login\\ButtonBack.tga", "StartImage\\Login\\ButtonBall.tga");
	//QuitButton.Init("StartImage\\Login\\ButtonQuit.tga", "StartImage\\Login\\ButtonBall.tga");
	//DeleteButton.Init("StartImage\\Login\\ButtonDelete.tga", "StartImage\\Login\\ButtonBall.tga");
	//NewCharButton.Init("StartImage\\Login\\ButtonNewChr.tga", "StartImage\\Login\\ButtonBall.tga");

	
	OkButton.Init("StartImage\\Login\\CharSelect\\Bt_select.tga", 128, 32);
	OkButton.InitCrashRect(10,0,110,32);

	BackButton.Init("StartImage\\Login\\CharSelect\\Bt_back.tga", 128, 32);
	BackButton.InitCrashRect(20,0,103,32);

	NewCharButton.Init("StartImage\\Login\\CharSelect\\bt-NewChar.tga", 128, 32);
	NewCharButton.InitCrashRect(10,0,110,32);

	QuitButton.Init("StartImage\\Login\\CharSelect\\Bt_exit.tga", 128, 32);
	QuitButton.InitCrashRect(15,0,108,32);

	DeleteButton.Init("StartImage\\Login\\CharSelect\\Bt_delet.tga", 128, 32);
	DeleteButton.InitCrashRect(10,0,108,32);
	
	//Character Select(캐릭터 선택)
//	CharacterSelectImage	= LoadDibSurfaceOffscreen( "StartImage\\Login\\SelectCharacter.bmp" );
	
	UserCharacterInfoBackImage  = CreateTextureMaterial( "StartImage\\Login\\UserCharacterInfo.tga" , 0, 0, 0, 0, SMMAT_BLEND_ALPHA );
	UserCharacterInfoImage      = CreateTextureMaterial( "StartImage\\Login\\CharacterInfo.tga" , 0, 0, 0, 0, SMMAT_BLEND_ALPHA );
	//Race Select(종족 선택)
//	RaceSelectImage			= LoadDibSurfaceOffscreen( "StartImage\\Login\\SelectRace.bmp" );
	
	TempscronMarkImage		= CreateTextureMaterial( "StartImage\\Login\\CharSelect\\mark_T.tga" , 0, 0, 0,0, SMMAT_BLEND_ALPHA );
	TempscronLogoImage		= CreateTextureMaterial( "StartImage\\Login\\CharSelect\\T-tempscron.tga" , 0, 0, 0,0, SMMAT_BLEND_ALPHA );


#ifndef NOTUSE_MOR 

	if(USE_MOR) {	//기본은 1이며 Init에서 셋팅됨.
	
	//ktj :Moryon 관련 새로만듦. start
	MoryonMarkImage = CreateTextureMaterial( "StartImage\\Login\\CharSelect\\mark_M.tga" , 0, 0, 0,0, SMMAT_BLEND_ALPHA );
	MoryonLogoImage = CreateTextureMaterial( "StartImage\\Login\\CharSelect\\T-Morion.tga" , 0, 0, 0,0, SMMAT_BLEND_ALPHA );
	//ktj :Moryon 관련  새로만듦. end
	}

#endif
	//Job Select(직업 선택)
//	JobSelectImage			= LoadDibSurfaceOffscreen( "StartImage\\Login\\SelectJob.bmp");
	
//	TempscronJobLogoImage[0]= CreateTextureMaterial( "StartImage\\Login\\LogoMechanician.tga" , 0, 0, 0,0, SMMAT_BLEND_ALPHA);
	//TempscronJobLogoImage[1]= CreateTextureMaterial( "StartImage\\Login\\LogoFighter.tga" , 0, 0, 0,0, SMMAT_BLEND_ALPHA);
	//TempscronJobLogoImage[2]= CreateTextureMaterial( "StartImage\\Login\\LogoPikeman.tga" , 0, 0, 0,0, SMMAT_BLEND_ALPHA);
	//TempscronJobLogoImage[3]= CreateTextureMaterial( "StartImage\\Login\\LogoArcher.tga" , 0, 0, 0,0, SMMAT_BLEND_ALPHA);

	
#ifndef NOTUSE_MOR 
	if(USE_MOR) 
	{

	//ktj :Moryon 관련  새로만듦. start
	MoryonJobLogoImage[0]= CreateTextureMaterial( "StartImage\\Login\\Moryon\\LogoNight.tga" , 0, 0, 0,0, SMMAT_BLEND_ALPHA);
	MoryonJobLogoImage[1]= CreateTextureMaterial( "StartImage\\Login\\Moryon\\LogoAt.tga" , 0, 0, 0,0, SMMAT_BLEND_ALPHA);
	MoryonJobLogoImage[2]= CreateTextureMaterial( "StartImage\\Login\\Moryon\\LogoPristess.tga" , 0, 0, 0,0, SMMAT_BLEND_ALPHA);
	MoryonJobLogoImage[3]= CreateTextureMaterial( "StartImage\\Login\\Moryon\\LogoMegi.tga" , 0, 0, 0,0, SMMAT_BLEND_ALPHA);
	//ktj :Moryon 관련  새로만듦. end

	}
#endif
	//Face Select(얼굴 선택)
	FaceSelectImage			= LoadDibSurfaceOffscreen( "StartImage\\Login\\SelectFace.bmp");
	
	//Mech
	TempFaceImage[0][0][0]	= CreateTextureMaterial( "StartImage\\Login\\TmA01-01.tga" , 0, 0, 0,0, SMMAT_BLEND_ALPHA);
	TempFaceImage[0][0][1]	= CreateTextureMaterial( "StartImage\\Login\\TmA01-02.tga" , 0, 0, 0,0, SMMAT_BLEND_ALPHA);
	TempFaceImage[0][1][0]	= CreateTextureMaterial( "StartImage\\Login\\TmA02-01.tga" , 0, 0, 0,0, SMMAT_BLEND_ALPHA);
	TempFaceImage[0][1][1]	= CreateTextureMaterial( "StartImage\\Login\\TmA02-02.tga" , 0, 0, 0,0, SMMAT_BLEND_ALPHA);
	TempFaceImage[0][2][0]	= CreateTextureMaterial( "StartImage\\Login\\TmA03-01.tga" , 0, 0, 0,0, SMMAT_BLEND_ALPHA);
	TempFaceImage[0][2][1]	= CreateTextureMaterial( "StartImage\\Login\\TmA03-02.tga" , 0, 0, 0,0, SMMAT_BLEND_ALPHA);


	//Fighter
	TempFaceImage[1][0][0]	= CreateTextureMaterial( "StartImage\\Login\\TmB01-01.tga" , 0, 0, 0,0, SMMAT_BLEND_ALPHA);
	TempFaceImage[1][0][1]	= CreateTextureMaterial( "StartImage\\Login\\TmB01-02.tga" , 0, 0, 0,0, SMMAT_BLEND_ALPHA);
	TempFaceImage[1][1][0]	= CreateTextureMaterial( "StartImage\\Login\\TmB02-01.tga" , 0, 0, 0,0, SMMAT_BLEND_ALPHA);
	TempFaceImage[1][1][1]	= CreateTextureMaterial( "StartImage\\Login\\TmB02-02.tga" , 0, 0, 0,0, SMMAT_BLEND_ALPHA);
	TempFaceImage[1][2][0]	= CreateTextureMaterial( "StartImage\\Login\\TmB03-01.tga" , 0, 0, 0,0, SMMAT_BLEND_ALPHA);
	TempFaceImage[1][2][1]	= CreateTextureMaterial( "StartImage\\Login\\TmB03-02.tga" , 0, 0, 0,0, SMMAT_BLEND_ALPHA);
	
	//Pike
	TempFaceImage[2][0][0]	= CreateTextureMaterial( "StartImage\\Login\\TmC01-01.tga" , 0, 0, 0,0, SMMAT_BLEND_ALPHA);
	TempFaceImage[2][0][1]	= CreateTextureMaterial( "StartImage\\Login\\TmC01-02.tga" , 0, 0, 0,0, SMMAT_BLEND_ALPHA);
	TempFaceImage[2][1][0]	= CreateTextureMaterial( "StartImage\\Login\\TmC02-01.tga" , 0, 0, 0,0, SMMAT_BLEND_ALPHA);
	TempFaceImage[2][1][1]	= CreateTextureMaterial( "StartImage\\Login\\TmC02-02.tga" , 0, 0, 0,0, SMMAT_BLEND_ALPHA);
	TempFaceImage[2][2][0]	= CreateTextureMaterial( "StartImage\\Login\\TmC03-01.tga" , 0, 0, 0,0, SMMAT_BLEND_ALPHA);
	TempFaceImage[2][2][1]	= CreateTextureMaterial( "StartImage\\Login\\TmC03-02.tga" , 0, 0, 0,0, SMMAT_BLEND_ALPHA);

	//Archer      
	TempFaceImage[3][0][0]	= CreateTextureMaterial( "StartImage\\Login\\TfD01-01.tga" , 0, 0, 0,0, SMMAT_BLEND_ALPHA);
	TempFaceImage[3][0][1]	= CreateTextureMaterial( "StartImage\\Login\\TfD01-02.tga" , 0, 0, 0,0, SMMAT_BLEND_ALPHA);
	TempFaceImage[3][1][0]	= CreateTextureMaterial( "StartImage\\Login\\TfD02-01.tga" , 0, 0, 0,0, SMMAT_BLEND_ALPHA);
	TempFaceImage[3][1][1]	= CreateTextureMaterial( "StartImage\\Login\\TfD02-02.tga" , 0, 0, 0,0, SMMAT_BLEND_ALPHA);
	TempFaceImage[3][2][0]	= CreateTextureMaterial( "StartImage\\Login\\TfD03-01.tga" , 0, 0, 0,0, SMMAT_BLEND_ALPHA);
	TempFaceImage[3][2][1]	= CreateTextureMaterial( "StartImage\\Login\\TfD03-02.tga" , 0, 0, 0,0, SMMAT_BLEND_ALPHA);

	//Assassin (its pictures are named -a/-b like Morion's)
	TempFaceImage[4][0][0]	= CreateTextureMaterial( "StartImage\\Login\\TfE01-a.tga" , 0, 0, 0,0, SMMAT_BLEND_ALPHA);
	TempFaceImage[4][0][1]	= CreateTextureMaterial( "StartImage\\Login\\TfE01-b.tga" , 0, 0, 0,0, SMMAT_BLEND_ALPHA);
	TempFaceImage[4][1][0]	= CreateTextureMaterial( "StartImage\\Login\\TfE02-a.tga" , 0, 0, 0,0, SMMAT_BLEND_ALPHA);
	TempFaceImage[4][1][1]	= CreateTextureMaterial( "StartImage\\Login\\TfE02-b.tga" , 0, 0, 0,0, SMMAT_BLEND_ALPHA);
	TempFaceImage[4][2][0]	= CreateTextureMaterial( "StartImage\\Login\\TfE03-a.tga" , 0, 0, 0,0, SMMAT_BLEND_ALPHA);
	TempFaceImage[4][2][1]	= CreateTextureMaterial( "StartImage\\Login\\TfE03-b.tga" , 0, 0, 0,0, SMMAT_BLEND_ALPHA);

	//Martial Artist
	TempFaceImage[5][0][0]	= CreateTextureMaterial( "StartImage\\Login\\TfF01-a.tga" , 0, 0, 0,0, SMMAT_BLEND_ALPHA);
	TempFaceImage[5][0][1]	= CreateTextureMaterial( "StartImage\\Login\\TfF01-b.tga" , 0, 0, 0,0, SMMAT_BLEND_ALPHA);
	TempFaceImage[5][1][0]	= CreateTextureMaterial( "StartImage\\Login\\TfF02-a.tga" , 0, 0, 0,0, SMMAT_BLEND_ALPHA);
	TempFaceImage[5][1][1]	= CreateTextureMaterial( "StartImage\\Login\\TfF02-b.tga" , 0, 0, 0,0, SMMAT_BLEND_ALPHA);
	TempFaceImage[5][2][0]	= CreateTextureMaterial( "StartImage\\Login\\TfF03-a.tga" , 0, 0, 0,0, SMMAT_BLEND_ALPHA);
	TempFaceImage[5][2][1]	= CreateTextureMaterial( "StartImage\\Login\\TfF03-b.tga" , 0, 0, 0,0, SMMAT_BLEND_ALPHA);






#ifndef NOTUSE_MOR 

	if(USE_MOR) 
	{

	//ktj :Moryon 관련  2d얼굴 그림. 새로만듦. start

	//Mech
	MorFaceImage[0][0][0]	= CreateTextureMaterial( "StartImage\\Login\\Moryon\\MmA01-a.tga" , 0, 0, 0,0, SMMAT_BLEND_ALPHA);
	MorFaceImage[0][0][1]	= CreateTextureMaterial( "StartImage\\Login\\Moryon\\MmA01-b.tga" , 0, 0, 0,0, SMMAT_BLEND_ALPHA);
	MorFaceImage[0][1][0]	= CreateTextureMaterial( "StartImage\\Login\\Moryon\\MmA02-a.tga" , 0, 0, 0,0, SMMAT_BLEND_ALPHA);
	MorFaceImage[0][1][1]	= CreateTextureMaterial( "StartImage\\Login\\Moryon\\MmA02-b.tga" , 0, 0, 0,0, SMMAT_BLEND_ALPHA);
	MorFaceImage[0][2][0]	= CreateTextureMaterial( "StartImage\\Login\\Moryon\\MmA03-a.tga" , 0, 0, 0,0, SMMAT_BLEND_ALPHA);
	MorFaceImage[0][2][1]	= CreateTextureMaterial( "StartImage\\Login\\Moryon\\MmA03-b.tga" , 0, 0, 0,0, SMMAT_BLEND_ALPHA);


	//Fighter
	MorFaceImage[1][0][0]	= CreateTextureMaterial( "StartImage\\Login\\Moryon\\MfB02-a.tga" , 0, 0, 0,0, SMMAT_BLEND_ALPHA);
	MorFaceImage[1][0][1]	= CreateTextureMaterial( "StartImage\\Login\\Moryon\\MfB02-b.tga" , 0, 0, 0,0, SMMAT_BLEND_ALPHA);
	MorFaceImage[1][1][0]	= CreateTextureMaterial( "StartImage\\Login\\Moryon\\MfB01-a.tga" , 0, 0, 0,0, SMMAT_BLEND_ALPHA);
	MorFaceImage[1][1][1]	= CreateTextureMaterial( "StartImage\\Login\\Moryon\\MfB01-b.tga" , 0, 0, 0,0, SMMAT_BLEND_ALPHA);
	MorFaceImage[1][2][0]	= CreateTextureMaterial( "StartImage\\Login\\Moryon\\MfB03-a.tga" , 0, 0, 0,0, SMMAT_BLEND_ALPHA);
	MorFaceImage[1][2][1]	= CreateTextureMaterial( "StartImage\\Login\\Moryon\\MfB03-b.tga" , 0, 0, 0,0, SMMAT_BLEND_ALPHA);
	
	//Pike
	MorFaceImage[2][0][0]	= CreateTextureMaterial( "StartImage\\Login\\Moryon\\Mfc01-a.tga" , 0, 0, 0,0, SMMAT_BLEND_ALPHA);
	MorFaceImage[2][0][1]	= CreateTextureMaterial( "StartImage\\Login\\Moryon\\Mfc01-b.tga" , 0, 0, 0,0, SMMAT_BLEND_ALPHA);
	MorFaceImage[2][1][0]	= CreateTextureMaterial( "StartImage\\Login\\Moryon\\Mfc02-a.tga" , 0, 0, 0,0, SMMAT_BLEND_ALPHA);
	MorFaceImage[2][1][1]	= CreateTextureMaterial( "StartImage\\Login\\Moryon\\Mfc02-b.tga" , 0, 0, 0,0, SMMAT_BLEND_ALPHA);
	MorFaceImage[2][2][0]	= CreateTextureMaterial( "StartImage\\Login\\Moryon\\Mfc03-a.tga" , 0, 0, 0,0, SMMAT_BLEND_ALPHA);
	MorFaceImage[2][2][1]	= CreateTextureMaterial( "StartImage\\Login\\Moryon\\Mfc03-b.tga" , 0, 0, 0,0, SMMAT_BLEND_ALPHA);

	//Archer      
	MorFaceImage[3][0][0]	= CreateTextureMaterial( "StartImage\\Login\\Moryon\\Mmd01-a.tga" , 0, 0, 0,0, SMMAT_BLEND_ALPHA);
	MorFaceImage[3][0][1]	= CreateTextureMaterial( "StartImage\\Login\\Moryon\\Mmd01-b.tga" , 0, 0, 0,0, SMMAT_BLEND_ALPHA);
	MorFaceImage[3][1][0]	= CreateTextureMaterial( "StartImage\\Login\\Moryon\\Mmd02-a.tga" , 0, 0, 0,0, SMMAT_BLEND_ALPHA);
	MorFaceImage[3][1][1]	= CreateTextureMaterial( "StartImage\\Login\\Moryon\\Mmd02-b.tga" , 0, 0, 0,0, SMMAT_BLEND_ALPHA);
	MorFaceImage[3][2][0]	= CreateTextureMaterial( "StartImage\\Login\\Moryon\\Mmd03-a.tga" , 0, 0, 0,0, SMMAT_BLEND_ALPHA);
	MorFaceImage[3][2][1]	= CreateTextureMaterial( "StartImage\\Login\\Moryon\\Mmd03-b.tga" , 0, 0, 0,0, SMMAT_BLEND_ALPHA);

	//Shaman
	MorFaceImage[4][0][0]	= CreateTextureMaterial( "StartImage\\Login\\Moryon\\MmE01-a.tga" , 0, 0, 0,0, SMMAT_BLEND_ALPHA);
	MorFaceImage[4][0][1]	= CreateTextureMaterial( "StartImage\\Login\\Moryon\\MmE01-b.tga" , 0, 0, 0,0, SMMAT_BLEND_ALPHA);
	MorFaceImage[4][1][0]	= CreateTextureMaterial( "StartImage\\Login\\Moryon\\MmE02-a.tga" , 0, 0, 0,0, SMMAT_BLEND_ALPHA);
	MorFaceImage[4][1][1]	= CreateTextureMaterial( "StartImage\\Login\\Moryon\\MmE02-b.tga" , 0, 0, 0,0, SMMAT_BLEND_ALPHA);
	MorFaceImage[4][2][0]	= CreateTextureMaterial( "StartImage\\Login\\Moryon\\MmE03-a.tga" , 0, 0, 0,0, SMMAT_BLEND_ALPHA);
	MorFaceImage[4][2][1]	= CreateTextureMaterial( "StartImage\\Login\\Moryon\\MmE03-b.tga" , 0, 0, 0,0, SMMAT_BLEND_ALPHA);

	}

#endif

	//ktj :Moryon 관련  새로만듦. end
	//Name Select(이름 입력)
//	NameSelectImage			= LoadDibSurfaceOffscreen( "StartImage\\Login\\SelectName.bmp");
	//Game 창틀 일기...
//	GameFrameImage[0] = CreateTextureMaterial( "StartImage\\Login\\FrameUp0.tga" , 0, 0, 0,0, SMMAT_BLEND_ALPHA);
//	GameFrameImage[1] = CreateTextureMaterial( "StartImage\\Login\\FrameUp1.tga" , 0, 0, 0,0, SMMAT_BLEND_ALPHA);
//	GameFrameImage[2] = CreateTextureMaterial( "StartImage\\Login\\FrameUp2.tga" , 0, 0, 0,0, SMMAT_BLEND_ALPHA);
//	GameFrameImage[3] = CreateTextureMaterial( "StartImage\\Login\\FrameDown0.tga" , 0, 0, 0,0, SMMAT_BLEND_ALPHA);
//	GameFrameImage[4] = CreateTextureMaterial( "StartImage\\Login\\FrameDown1.tga" , 0, 0, 0,0, SMMAT_BLEND_ALPHA);
//	GameFrameImage[5] = CreateTextureMaterial( "StartImage\\Login\\FrameDown2.tga" , 0, 0, 0,0, SMMAT_BLEND_ALPHA);
	
	//테두리얼굴 박스
	//MatEachBox = CreateTextureMaterial( "image\\cw.tga" ,0 , TRUE ,FALSE , FALSE , SMMAT_BLEND_ALPHA );
	MatEachBox = -1;
//	ZoomFaceFrameImage[0] = CreateTextureMaterial( "StartImage\\Login\\FaceFrame0.tga" ,0 , TRUE ,FALSE , FALSE , SMMAT_BLEND_ALPHA );
//	ZoomFaceFrameImage[1] = CreateTextureMaterial( "StartImage\\Login\\FaceFrame1.tga" ,0 , TRUE ,FALSE , FALSE , SMMAT_BLEND_ALPHA );
//	ZoomFaceFrameImage[2] = CreateTextureMaterial( "StartImage\\Login\\FaceFrame2.tga" ,0 , TRUE ,FALSE , FALSE , SMMAT_BLEND_ALPHA );

	BlackBoxImage = CreateTextureMaterial( "StartImage\\Login\\BlackBox.tga" ,0 , TRUE ,FALSE , FALSE , SMMAT_BLEND_ALPHA );

	SYSTEMTIME	st;
	GetLocalTime( &st );

	
	/*
	if ( st.wMonth==5 && st.wDay<=5 )
		GameMapImage = LoadDibSurfaceOffscreen( "StartImage\\Login\\Map0505.bmp");
	else
		GameMapImage = LoadDibSurfaceOffscreen( "StartImage\\Login\\Map.bmp");
	*/
	
//	FaceBoxImage = CreateTextureMaterial( "StartImage\\Login\\FaceBoxImage.tga" ,0 , TRUE ,FALSE , FALSE , SMMAT_BLEND_ALPHA );
	Message.Init();
	MessageBox.Init();
	InitMessageBox();	//CharMessageBox 초기화..
	
	
	//새로운 이미지 박스 프레임 읽기
	MainBoxFrameImage1[0] = CreateTextureMaterial( "StartImage\\Login\\CharSelect\\mainbox_L.tga" ,0 , TRUE ,FALSE , FALSE , SMMAT_BLEND_ALPHA );
    MainBoxFrameImage1[1] = CreateTextureMaterial( "StartImage\\Login\\CharSelect\\mainbox_T.tga" ,0 , TRUE ,FALSE , FALSE , SMMAT_BLEND_ALPHA );
	MainBoxFrameImage1[2] = CreateTextureMaterial( "StartImage\\Login\\CharSelect\\mainbox_R.tga" ,0 , TRUE ,FALSE , FALSE , SMMAT_BLEND_ALPHA );
	MainBoxFrameImage1[3] = CreateTextureMaterial( "StartImage\\Login\\CharSelect\\mainbox_D.tga" ,0 , TRUE ,FALSE , FALSE , SMMAT_BLEND_ALPHA );
	
    MainBoxFrameImage2[0] = CreateTextureMaterial( "StartImage\\Login\\CharSelect\\mainbox_CN1.tga" ,0 , TRUE ,FALSE , FALSE , SMMAT_BLEND_ALPHA );
    MainBoxFrameImage2[1] = CreateTextureMaterial( "StartImage\\Login\\CharSelect\\mainbox_CN2.tga" ,0 , TRUE ,FALSE , FALSE , SMMAT_BLEND_ALPHA );
	MainBoxFrameImage2[2] = CreateTextureMaterial( "StartImage\\Login\\CharSelect\\mainbox_CN3.tga" ,0 , TRUE ,FALSE , FALSE , SMMAT_BLEND_ALPHA );
	MainBoxFrameImage2[3] = CreateTextureMaterial( "StartImage\\Login\\CharSelect\\mainbox_CN4.tga" ,0 , TRUE ,FALSE , FALSE , SMMAT_BLEND_ALPHA );

	TitleBoxFrameImage = CreateTextureMaterial( "StartImage\\Login\\CharSelect\\title-box.tga" ,0 , TRUE ,FALSE , FALSE , SMMAT_BLEND_ALPHA );
    TitleNameImage     = CreateTextureMaterial( "StartImage\\Login\\CharSelect\\title-txt30.tga" ,0 , TRUE ,FALSE , FALSE , SMMAT_BLEND_ALPHA );
	
	TempscronTextImage = CreateTextureMaterial( "StartImage\\Login\\CharSelect\\T-ex_tscr.tga" ,0 , TRUE ,FALSE , FALSE , SMMAT_BLEND_ALPHA );
	MoryonTextImage    = CreateTextureMaterial( "StartImage\\Login\\CharSelect\\T-ex_mrin.tga" ,0 , TRUE ,FALSE , FALSE , SMMAT_BLEND_ALPHA );

	RaceTextBoxImage[0] = CreateTextureMaterial( "StartImage\\Login\\CharSelect\\mark_box1.tga" ,0 , TRUE ,FALSE , FALSE , SMMAT_BLEND_ALPHA );
	RaceTextBoxImage[1] = CreateTextureMaterial( "StartImage\\Login\\CharSelect\\mark_box2.tga" ,0 , TRUE ,FALSE , FALSE , SMMAT_BLEND_ALPHA );


	JobTextBoxImage[0]  = CreateTextureMaterial( "StartImage\\Login\\CharSelect\\T_cls-box1.tga" ,0 , TRUE ,FALSE , FALSE , SMMAT_BLEND_ALPHA );
	JobTextBoxImage[1]  = CreateTextureMaterial( "StartImage\\Login\\CharSelect\\T_cls-box2.tga" ,0 , TRUE ,FALSE , FALSE , SMMAT_BLEND_ALPHA );
	
	TempscronJobImage[0][0] = CreateTextureMaterial( "StartImage\\Login\\CharSelect\\T_cls-meca_.tga" , 0, 0, 0,0, SMMAT_BLEND_ALPHA);
    TempscronJobImage[1][0] = CreateTextureMaterial( "StartImage\\Login\\CharSelect\\T_cls-ft_.tga" , 0, 0, 0,0, SMMAT_BLEND_ALPHA);
	TempscronJobImage[2][0] = CreateTextureMaterial( "StartImage\\Login\\CharSelect\\T_cls-pkm_.tga" , 0, 0, 0,0, SMMAT_BLEND_ALPHA);
	TempscronJobImage[3][0] = CreateTextureMaterial( "StartImage\\Login\\CharSelect\\T_cls-ac_.tga" , 0, 0, 0,0, SMMAT_BLEND_ALPHA);

    TempscronJobImage[0][1] = CreateTextureMaterial( "StartImage\\Login\\CharSelect\\T_cls-meca.tga" , 0, 0, 0,0, SMMAT_BLEND_ALPHA);
    TempscronJobImage[1][1] = CreateTextureMaterial( "StartImage\\Login\\CharSelect\\T_cls-ft.tga" , 0, 0, 0,0, SMMAT_BLEND_ALPHA);
	TempscronJobImage[2][1] = CreateTextureMaterial( "StartImage\\Login\\CharSelect\\T_cls-pkm.tga" , 0, 0, 0,0, SMMAT_BLEND_ALPHA);
	TempscronJobImage[3][1] = CreateTextureMaterial( "StartImage\\Login\\CharSelect\\T_cls-ac.tga" , 0, 0, 0,0, SMMAT_BLEND_ALPHA);

	TempscronJobImage[4][0] = CreateTextureMaterial( "StartImage\\Login\\CharSelect\\T_cls-assa_.tga" , 0, 0, 0,0, SMMAT_BLEND_ALPHA);
	TempscronJobImage[4][1] = CreateTextureMaterial( "StartImage\\Login\\CharSelect\\T_cls-assa.tga" , 0, 0, 0,0, SMMAT_BLEND_ALPHA);
	TempscronJobImage[5][0] = CreateTextureMaterial( "StartImage\\Login\\CharSelect\\T_cls-ma_.tga" , 0, 0, 0,0, SMMAT_BLEND_ALPHA);
	TempscronJobImage[5][1] = CreateTextureMaterial( "StartImage\\Login\\CharSelect\\T_cls-ma.tga" , 0, 0, 0,0, SMMAT_BLEND_ALPHA);

    MoryonJobImage[0][0] = CreateTextureMaterial( "StartImage\\Login\\CharSelect\\T-cls-knt_.tga" , 0, 0, 0,0, SMMAT_BLEND_ALPHA);
    MoryonJobImage[1][0] = CreateTextureMaterial( "StartImage\\Login\\CharSelect\\T_cls-atlt_.tga" , 0, 0, 0,0, SMMAT_BLEND_ALPHA);
	MoryonJobImage[2][0] = CreateTextureMaterial( "StartImage\\Login\\CharSelect\\T_cls-prts_.tga" , 0, 0, 0,0, SMMAT_BLEND_ALPHA);
	MoryonJobImage[3][0] = CreateTextureMaterial( "StartImage\\Login\\CharSelect\\T_cls-mgc_.tga" , 0, 0, 0,0, SMMAT_BLEND_ALPHA);
    
	MoryonJobImage[0][1] = CreateTextureMaterial( "StartImage\\Login\\CharSelect\\T-cls-knt.tga" , 0, 0, 0,0, SMMAT_BLEND_ALPHA);
    MoryonJobImage[1][1] = CreateTextureMaterial( "StartImage\\Login\\CharSelect\\T_cls-atlt.tga" , 0, 0, 0,0, SMMAT_BLEND_ALPHA);
	MoryonJobImage[2][1] = CreateTextureMaterial( "StartImage\\Login\\CharSelect\\T_cls-prts.tga" , 0, 0, 0,0, SMMAT_BLEND_ALPHA);
	MoryonJobImage[3][1] = CreateTextureMaterial( "StartImage\\Login\\CharSelect\\T_cls-mgc.tga" , 0, 0, 0,0, SMMAT_BLEND_ALPHA);

	MoryonJobImage[4][0] = CreateTextureMaterial( "StartImage\\Login\\CharSelect\\T_cls-sha_.tga" , 0, 0, 0,0, SMMAT_BLEND_ALPHA);
	MoryonJobImage[4][1] = CreateTextureMaterial( "StartImage\\Login\\CharSelect\\T_cls-sha.tga" , 0, 0, 0,0, SMMAT_BLEND_ALPHA);
	MoryonJobImage[5][0] = MoryonJobImage[5][1] = -1;		// Morion has five classes
	
	TempscronJobTextImage[0] = CreateTextureMaterial( "StartImage\\Login\\CharSelect\\T_chr-meca.tga" , 0, 0, 0,0, SMMAT_BLEND_ALPHA);
	TempscronJobTextImage[1] = CreateTextureMaterial( "StartImage\\Login\\CharSelect\\T_chr-ft.tga" , 0, 0, 0,0, SMMAT_BLEND_ALPHA);
	TempscronJobTextImage[2] = CreateTextureMaterial( "StartImage\\Login\\CharSelect\\T_chr-pkm.tga" , 0, 0, 0,0, SMMAT_BLEND_ALPHA);
	TempscronJobTextImage[3] = CreateTextureMaterial( "StartImage\\Login\\CharSelect\\T_chr-ac.tga" , 0, 0, 0,0, SMMAT_BLEND_ALPHA);
	TempscronJobTextImage[4] = CreateTextureMaterial( "StartImage\\Login\\CharSelect\\T_chr-assa.tga" , 0, 0, 0,0, SMMAT_BLEND_ALPHA);
	TempscronJobTextImage[5] = CreateTextureMaterial( "StartImage\\Login\\CharSelect\\T_chr-ma.tga" , 0, 0, 0,0, SMMAT_BLEND_ALPHA);

	MoryonJobTextImage[0] = CreateTextureMaterial( "StartImage\\Login\\CharSelect\\T_chr-knt.tga" , 0, 0, 0,0, SMMAT_BLEND_ALPHA);
	MoryonJobTextImage[1] = CreateTextureMaterial( "StartImage\\Login\\CharSelect\\T_chr-atlt.tga" , 0, 0, 0,0, SMMAT_BLEND_ALPHA);
	MoryonJobTextImage[2] = CreateTextureMaterial( "StartImage\\Login\\CharSelect\\T_chr-prts.tga" , 0, 0, 0,0, SMMAT_BLEND_ALPHA);
	MoryonJobTextImage[3] = CreateTextureMaterial( "StartImage\\Login\\CharSelect\\T_chr-mgc.tga" , 0, 0, 0,0, SMMAT_BLEND_ALPHA);
	MoryonJobTextImage[4] = CreateTextureMaterial( "StartImage\\Login\\CharSelect\\T_chr-sha.tga" , 0, 0, 0,0, SMMAT_BLEND_ALPHA);
	MoryonJobTextImage[5] = -1;

	FaceBoxBigImage[0] = CreateTextureMaterial( "StartImage\\Login\\CharSelect\\face_bigbox1.tga" , 0, 0, 0,0, SMMAT_BLEND_ALPHA);
	FaceBoxBigImage[1] = CreateTextureMaterial( "StartImage\\Login\\CharSelect\\face_bigbox2.tga" , 0, 0, 0,0, SMMAT_BLEND_ALPHA);
	
	FaceBoxSmallImage[0] = CreateTextureMaterial( "StartImage\\Login\\CharSelect\\face_box_.tga" , 0, 0, 0,0, SMMAT_BLEND_ALPHA);
	FaceBoxSmallImage[1] = CreateTextureMaterial( "StartImage\\Login\\CharSelect\\face_box.tga" , 0, 0, 0,0, SMMAT_BLEND_ALPHA);
	FaceBoxSmallImage[2] = CreateTextureMaterial( "StartImage\\Login\\CharSelect\\face_box2.tga" , 0, 0, 0,0, SMMAT_BLEND_ALPHA);

	NewCharacterImage		= CreateTextureMaterial( "StartImage\\Login\\CharSelect\\New_char.tga" , 0, 0, 0, 0, SMMAT_BLEND_ALPHA );

	ReadTextures();
	
	return TRUE;
}

static char *LoadingImageName[MAX_LOADING_IMAGE] =
{
#ifdef _LANGUAGE_BRAZIL
	"StartImage\\Login\\Loading\\Loading1.bmp",
	"StartImage\\Login\\Loading\\Loading2.bmp",
	"StartImage\\Login\\Loading\\Loading3.bmp",
	"StartImage\\Login\\Loading\\Loading4.bmp",
	"StartImage\\Login\\Loading\\Loading5.bmp",
	"StartImage\\Login\\Loading\\Loading6.bmp",
#else
#ifdef _LANGUAGE_ARGENTINA
	"StartImage\\Login\\Loading\\Map.bmp",
	"StartImage\\Login\\Loading\\Ventas.bmp",
	"StartImage\\Login\\Loading\\Ventas2.bmp",
	"StartImage\\Login\\Loading\\Ventas3.bmp",
#else
	"StartImage\\Login\\Loading\\Map.bmp",
	"StartImage\\Login\\Loading\\map-all-all.bmp",
	"StartImage\\Login\\Loading\\map-all-mrin.bmp",
	"StartImage\\Login\\Loading\\map-all-tscr.bmp",
	"StartImage\\Login\\Loading\\map-ch-ac.bmp",
	"StartImage\\Login\\Loading\\map-ch-at.bmp",
	"StartImage\\Login\\Loading\\map-ch-ft.bmp",
	"StartImage\\Login\\Loading\\map-ch-knight.bmp",
	"StartImage\\Login\\Loading\\map-ch-meca.bmp",
	"StartImage\\Login\\Loading\\map-ch-mg.bmp",
	"StartImage\\Login\\Loading\\map-ch-pk.bmp",
	"StartImage\\Login\\Loading\\map-ch-pt.bmp",
	"StartImage\\Login\\Loading\\map-logo-RNS2.bmp",
	"StartImage\\Login\\Loading\\map-logo-RNS.bmp",
	"StartImage\\Login\\Loading\\map-SOD.bmp",
	"StartImage\\Login\\Loading\\map-tw-nvsc.bmp",
	"StartImage\\Login\\Loading\\map-tw-pillai.bmp",
	"StartImage\\Login\\Loading\\map-tw-rica.bmp",
	"StartImage\\Login\\Loading\\map-user_001.bmp"
#endif
#endif
};

void HoLogin::LoadLoadingImage(int level)
{
#ifdef _LANGUAGE_BRAZIL
	GameMapImage = LoadDibSurfaceOffscreen( LoadingImageName[rand()%MAX_LOADING_IMAGE]);
#else
	if(level <= 10)
		GameMapImage = LoadDibSurfaceOffscreen( LoadingImageName[0]);
	else
	{
		GameMapImage = LoadDibSurfaceOffscreen( LoadingImageName[rand()%MAX_LOADING_IMAGE]);
	}
#endif
}

int	HoLogin::AddUserCharacter(char *chrFile1, char *chrFile2, int jobCode)
{
	int index; for( index = 0; index < CHARACTER_USER_MAX; index++)
	{
		if(UserCharacter[index].DisplayFlag == FALSE)
		{
			UserCharacter[index].Init();
			UserCharacter[index].lpStage = Stage;
			
			SetLoadPattern( &UserCharacter[index], chrFile1, chrFile2, 0, 0);
			UserCharacter[index].SetPosi( UserCharacterPos[index][0]*fONE, 
				Stage->GetHeight( UserCharacterPos[index][0]*fONE, UserCharacterPos[index][1]*fONE)
				, UserCharacterPos[index][1]*fONE, 0, ANGLE_180, 0);
			UserCharacter[index].ActionPattern = 0;

			
			UserCharacter[index].ChangeMotion(10);
			
			//if(jobCode == 4)
				//UserCharacter[index].ChangeMotion(11);

			UserCharacterCount++;
			ReadTextures();
			
			UserCharacter[index].lpDinaPattern->Pat->smMaterialGroup->CheckMatreialTextureSwap();
			if ( UserCharacter[index].lpDinaPattern2 )
				UserCharacter[index].lpDinaPattern2->Pat->smMaterialGroup->CheckMatreialTextureSwap();
			break;
		}
	}
	
	
	return TRUE;
}


int HoLogin::AddJobCharacter(char *chrFile1, char *chrFile2, int index)
{
	if(index == -1)
	{
		JobCharacter[JobCharacterCount].Init();
		JobCharacter[JobCharacterCount].lpStage = Stage;
		JobCharacter[JobCharacterCount].smCharInfo.wPlayerKilling[0] = 0;
	

		//Player 결정...
		SetLoadPattern( &JobCharacter[JobCharacterCount], chrFile1, chrFile2, 0, 0);
		
		JobCharacter[JobCharacterCount].SetPosi( JobCharacterPos[JobCharacterCount][0]*fONE, 
			Stage->GetHeight(JobCharacterPos[JobCharacterCount][0]*fONE, JobCharacterPos[JobCharacterCount][1]*fONE), 
			JobCharacterPos[JobCharacterCount][1]*fONE, 0, ANGLE_180, 0);
		JobCharacter[JobCharacterCount].ActionPattern = 0;
		memset(JobCharacter[JobCharacterCount].szChatMessage, 0, sizeof(JobCharacter[JobCharacterCount].szChatMessage));
		memset(JobCharacter[JobCharacterCount].smCharInfo.szName, 0, sizeof(JobCharacter[JobCharacterCount].smCharInfo.szName));
	
		JobCharacter[JobCharacterCount].ChangeMotion(10);
		
		//if(JobCharacterCount == 2)
		//JobCharacter[JobCharacterCount].ChangeMotion(11);
		
		JobCharacterCount++;
		ReadTextures();
		JobCharacter[JobCharacterCount-1].lpDinaPattern->Pat->smMaterialGroup->CheckMatreialTextureSwap();
		JobCharacter[JobCharacterCount-1].lpDinaPattern2->Pat->smMaterialGroup->CheckMatreialTextureSwap();

		// the one just added (this wrote one past it: past the array for the last class)
		JobCharacter[JobCharacterCount-1].smCharInfo.Stamina[0] = 100;
		JobCharacter[JobCharacterCount-1].smCharInfo.Stamina[1] = 100;

	}
	else
	{
		if(index >= 0 && index < CHARACTER_JOB_MAX)
		{
			JobCharacter[index].Init();
			JobCharacter[index].lpStage = Stage;
			JobCharacter[index].smCharInfo.wPlayerKilling[0] = 0;
		
			//Player 결정...
			SetLoadPattern( &JobCharacter[index], chrFile1, chrFile2, 0, 0);
			JobCharacter[index].SetPosi( JobCharacterPos[index][0]*fONE,
				Stage->GetHeight( JobCharacterPos[index][0]*fONE, JobCharacterPos[index][1]*fONE)
				, JobCharacterPos[index][1]*fONE, 0, ANGLE_180, 0);
			JobCharacter[index].ActionPattern = 0;
			memset(JobCharacter[index].szChatMessage, 0, sizeof(JobCharacter[JobCharacterCount].szChatMessage));
			memset(JobCharacter[index].smCharInfo.szName, 0, sizeof(JobCharacter[JobCharacterCount].smCharInfo.szName));
			
			ReadTextures();
			JobCharacter[index].lpDinaPattern->Pat->smMaterialGroup->CheckMatreialTextureSwap();
			JobCharacter[index].lpDinaPattern2->Pat->smMaterialGroup->CheckMatreialTextureSwap();

			JobCharacter[index].smCharInfo.Stamina[0] = 100;
			JobCharacter[index].smCharInfo.Stamina[1] = 100;

		}
		else
			return FALSE;
	}


	return TRUE;
}

// The race's classes on the job screen, from the LoginBodyName/LoginFaceName/LoginNewCharInit
// tables (Tempskron: Mechanician, Fighter, Pikeman, Archer, Assassin, Martial Artist;
// Morion: Knight, Atalanta, Priestess, Magician, Shaman).
void HoLogin::SetupJobCharacters( int tribe )
{
	int cnt, count;

	tribeNum = tribe;
	for( cnt=0; cnt<CHARACTER_JOB_MAX; cnt++ )
		JobCharacter[cnt].Close();

	count = LoginJobCount( tribe );
	SetJobCharacterLayout( count );

	JobCharacterCount = 0;
	for( cnt=0; cnt<count; cnt++ ) {
		JobCharacter[JobCharacterCount].smCharInfo.JOB_CODE = LoginNewCharInit( tribe, cnt )[0];
		AddJobCharacter( LoginBodyName( tribe, cnt ), LoginFaceName( tribe, cnt, 0 ) );
	}
	ReadTextures();
}

//선택된 캐릭터를 찾는다.
//return 1: 마우스가 캐릭터 위에 있는 경우.
//return 2: 마우스로 캐릭터를 선택한 경우.(MouseDown)
int HoLogin::GetSelectCharacter( smCHAR *lpChar )
{
	if(lpChar == NULL)
		return FALSE;
	if(lpChar->DisplayFlag == FALSE)
		return FALSE;

	RECT rect;
	POINT	sPos;
	int sez;
	int SelZ = 65536*16384;
	

	if ( lpChar )
	{
		int x,y,z;
		int anx,any;
		
		anx = CaAngle.x;
		any = CaAngle.y;

		any &=ANGCLIP;
		anx &=ANGCLIP;

		GetMoveLocation( 0 , 0, -TarDist, anx, any, 0 );
		x = TarPosi.x + GeoResult_X;
		y = TarPosi.y + GeoResult_Y;
		z = TarPosi.z + GeoResult_Z;
		smRender.SetCameraPosi( x,y,z, anx , any , 0  );
		
		sez = smRender.GetRect2D( lpChar->pX , lpChar->pY+(32*fONE) , lpChar->pZ ,
					24*fONE , 32*fONE , &rect , &sPos );


		if ( sez>0 && sez<SelZ && 
			rect.left<pCursorPos.x && rect.right>pCursorPos.x &&
			rect.top<pCursorPos.y && rect.bottom>pCursorPos.y )
		{

			if(MouseButton[0])
				return 2;
			
			return 1;
		}
	}
	return FALSE;
}

int HoLogin::SetChangeJobFace(char *charFile1, char *chrFile2, int jobIndex)
{
	int PosX,PosZ;
	PosX = JobCharacter[jobIndex].pX;
	PosZ = JobCharacter[jobIndex].pZ;
	AddJobCharacter( charFile1, chrFile2, jobIndex);
	JobCharacter[jobIndex].SetPosi( PosX, 10, PosZ, 0, ANGLE_180, 0);
	ReadTextures();
	JobCharacter[jobIndex].lpDinaPattern2->Pat->smMaterialGroup->CheckMatreialTextureSwap();
	return TRUE;
}

//종료 캐릭터 선택
int HoLogin::Close()
{
	//폰트 제거
	if(Font != NULL)
		DeleteObject( Font );

	CloseEachPlayer();

	if( Stage)
	{
		delete Stage;
		Stage = NULL;
	}
	
	int index; for( index = 0; index < UserCharacterCount; index++)
		UserCharacter[index].Init();
    
	for(index = 0; index < JobCharacterCount; index++)
		JobCharacter[index].Init();
	

	if(GameMapImage)
	{
		GameMapImage->Release();
		GameMapImage = NULL;
	}

	void ClearSelectCha();
	ClearSelectCha();
	return TRUE;
}

extern smCHAR *chrEachMaster;
//메인함수 캐릭터 선택

int HoLogin::CameraUpDown()
{
	//Camera의 아래, 위 흔들기...
	static int	cameraUpDown = 0;
	if(cameraUpDown < 80)
	{
		TarPosi.y += 4;
	}

	if(cameraUpDown >= 80 && cameraUpDown < 160)
	{
		TarPosi.y -= 4;
	}

	cameraUpDown++;
	if(cameraUpDown >= 160)
		cameraUpDown = 0;
	
	if(SelectCurrentState == CHARACTER_SELECT_STATE)
	{
		if(NewCharacterImageBlendFlag)
		{
			NewCharacterImageBlendCount -= 1.25f;
		}
		else
		{
			NewCharacterImageBlendCount += 1.25f;
		}

		if(NewCharacterImageBlendCount >= 255.f)
		{
			NewCharacterImageBlendFlag = true;
			NewCharacterImageBlendCount = 255.f;
			
		}

		
		if(NewCharacterImageBlendCount < 100.f)
		{
			NewCharacterImageBlendFlag = false;
			NewCharacterImageBlendCount = 100.f;
		}
		
	}

	return TRUE;
}

//ktj : 새로운캐릭을 만드는등의 일을 여기서 한다.
int HoLogin::Main()
{
	if(ErrorSelect != ERROR_NONE)
	{
  		if(StartErrorDelayFlag == FALSE)
		{
			StartErrorDelayFlag = TRUE;
			ErrorDelayCount = 0;
		}
		else
		{
			ErrorDelayCount++;
		}

		if(ErrorDelayCount > 100)
		{
			if(ErrorSelect == ERROR_NONAME_NEWPLAYER)
			{
				hFocusWnd = hTextWnd;
				FocusWndFlag = TRUE;
#ifdef	_LANGUAGE_JAPANESE		//C7
				SetIME_Mode( 1 );
#endif
			}
			ErrorSelect = ERROR_NONE;
			MouseButton[0] = FALSE;
			MouseDblClick = FALSE;

			StartErrorDelayFlag = FALSE;
		}
	}

	//ktj : 오프닝 전용 클랜메뉴. ====================================start
	
	void ktj_imsi_clanMENU_Main();
	ktj_imsi_clanMENU_Main();
	
	//ktj : 오프닝 전용 클랜메뉴. ====================================end/
	
	//클랜이름 마크 루프..
	int  main_chaC();

 	if(main_chaC() && ClanMarkUpdateSuccessFlag == FALSE)
	{
		for(int m = 0; m < 5; m++)
		{
			if(strlen(chacldata[m].ChaName) >= 1)
			{
				for(int n = 0; n < DownUserCharacterCount; n++)
				{
					if(stricmp(chacldata[m].ChaName, DownUserCharacterInfo[n].Name) == 0 && 
						DownUserCharacterInfo[n].ClanFlag == FALSE)
					{
						strcpy(DownUserCharacterInfo[n].ClanName, chacldata[m].name);
						DownUserCharacterInfo[n].ClanFlag = TRUE;
						if(chacldata[m].hClanMark16)
						DownUserCharacterInfo[n].ClanMark = chacldata[m].hClanMark16;
					}
				}
			}
		}
		ClanMarkUpdateSuccessFlag = TRUE;
	}
 	
	MainEffect();

	CameraUpDown(); //Camera 아래 위 흔들기
	
	if(SelectCurrentState == START_GAME_STATE && FadeCurrentState == FADE_IN)
	{
		switch(SelectSubState)
		{
			case SUB_SELECT_CHARACTER_USER:	//기본 캐릭터를 선택한 경우.
			{
				if(MouseDownCharacterNum != -1)
				{
					memcpy(CharacterName1, DownUserCharacterInfo[MouseDownCharacterNum].BodyName, sizeof(CharacterName1));
					memcpy(CharacterName2, DownUserCharacterInfo[MouseDownCharacterNum].FaceName, sizeof(CharacterName2));
					memset(PlayerName, 0, sizeof(PlayerName));
					memcpy(PlayerName, DownUserCharacterInfo[MouseDownCharacterNum].Name, sizeof(PlayerName));
					CharacterJob   = DownUserCharacterInfo[MouseDownCharacterNum].Job;
					GameStartField = DownUserCharacterInfo[MouseDownCharacterNum].StartField;
					
					CharacterPosX = DownUserCharacterInfo[MouseDownCharacterNum].PosX;
					CharacterPosZ = DownUserCharacterInfo[MouseDownCharacterNum].PosZ;
				}

				SelectSubState = SUB_SELECT_NONE;
				return 3;
				break;
			}
			
			case SUB_SELECT_CHARACTER_NEW_SERVER: //새로운 캐릭터를 생성한 경우.
			{
				SelectSubState = SUB_SELECT_NONE;
				memset(CharacterName1, 0, sizeof(CharacterName1));
				memset(CharacterName2, 0, sizeof(CharacterName2));









#ifdef NOTUSE_MOR

				//ktj : 템스크론만 선택할수있는경우.
				if(JobBodySelectNum == 0)//Mech
				{
					memcpy(CharacterName1, szTempMechBodyName[0], sizeof(CharacterName1));
					memcpy(CharacterName2, szTempMechFaceName[0][JobFaceSelectNum], sizeof(CharacterName2));
				}
				else if(JobBodySelectNum == 1)//Fighter
				{
					memcpy(CharacterName1, szTempFighterBodyName[0], sizeof(CharacterName1));
					memcpy(CharacterName2, szTempFighterFaceName[0][JobFaceSelectNum], sizeof(CharacterName2));
				}
				else if(JobBodySelectNum == 2)//Pike
				{
					memcpy(CharacterName1, szTempPikeBodyName[0], sizeof(CharacterName1));
					memcpy(CharacterName2, szTempPikeFaceName[0][JobFaceSelectNum], sizeof(CharacterName2));
				}
				else if(JobBodySelectNum == 3)//Archer
				{
					memcpy(CharacterName1, szTempArcherBodyName[0], sizeof(CharacterName1));
					memcpy(CharacterName2, szTempArcherFaceName[0][JobFaceSelectNum], sizeof(CharacterName2));
				}
				NewCharInfo.JobCode   = TempNewCharacterInit[JobBodySelectNum][0];
				NewCharInfo.Strength  = TempNewCharacterInit[JobBodySelectNum][1];			//힘 
				NewCharInfo.Spirit    = TempNewCharacterInit[JobBodySelectNum][2];			//정신력 
				NewCharInfo.Talent    = TempNewCharacterInit[JobBodySelectNum][3];			//재능 
				NewCharInfo.Dexterity = TempNewCharacterInit[JobBodySelectNum][4];		    //민첩성
				NewCharInfo.Health    = TempNewCharacterInit[JobBodySelectNum][5];			//건강.

#endif






#ifndef NOTUSE_MOR	//=============================================================
			//ktj : 종족추가에 따라 새로 넣음. (각캐릭터의 이름이 써진 2d 그림임)
			{
				// the chosen class's body and head, and its starting stats (either race)
				int *lpInit = LoginNewCharInit( tribeNum, JobBodySelectNum );
				lstrcpyn( CharacterName1, LoginBodyName( tribeNum, JobBodySelectNum ), sizeof(CharacterName1) );
				lstrcpyn( CharacterName2, LoginFaceName( tribeNum, JobBodySelectNum, JobFaceSelectNum ), sizeof(CharacterName2) );
				NewCharInfo.JobCode   = lpInit[0];
				NewCharInfo.Strength  = lpInit[1];			//힘
				NewCharInfo.Spirit    = lpInit[2];			//정신력
				NewCharInfo.Talent    = lpInit[3];			//재능
				NewCharInfo.Dexterity = lpInit[4];		    //민첩성
				NewCharInfo.Health    = lpInit[5];			//건강.
			}





#endif		//=============================================================

				return 3;
				break;
			}

		}
	}
	
	if(SelectCurrentState != COMPLETE_SELECT_STATE && SelectCurrentState != START_GAME_STATE)
	{
		//Stage에 올라 오는 케릭터들 처리...
		int index; for( index = 0; index < UserCharacterCount; index++)
		{
			if(UserCharacter[index].DisplayFlag)
			UserCharacter[index].Main();
		}
		//Stage에 올라 오는 Job 캐릭터들...
		for(index = 0; index < JobCharacterCount; index++)
			JobCharacter[index].Main();
	}
	
	//상태 변화가 완료 됐을 경우.
	if(SelectCurrentState == SelectNextState)
	{
		if(SelectCurrentState == RACE_SELECT_STATE)
		{
			//종족 Mark Blend
			if(RaceMarkBlendCount < 200)
				RaceMarkBlendCount+= 4;
		}
		else if(SelectCurrentState == JOB_SELECT_STATE)
		{
			
			//임시...(캐릭터 제자리로 돌아가기) (이름 입력 상태에서 직업 상태로 왔을 때 딱 한번만 돼야함)
			if(CharacterReturnFlag)//CharacterStopFlag)
			{
				CharacterStopFlag = FALSE;
				CharacterReturnFlag = FALSE;
				int index; for( index =0;index<JobCharacterCount;index++)
				{
					JobCharacter[index].SetPosi( JobCharacterPos[index][0]*fONE,
						Stage->GetHeight(JobCharacterPos[index][0]*fONE, JobCharacterPos[index][1]*fONE) 
						, JobCharacterPos[index][1]*fONE, 0,ANGLE_180,0 );
					JobCharacter[index].ChangeMotion(10);
				}
			}
			
		}
		else if(SelectCurrentState == FACE_SELECT_STATE)
		{
			if(chrEachMaster != NULL)
				chrEachMaster->Flag = 1;
			EachPlayerMain();
		}
		else if(SelectCurrentState == COMPLETE_SELECT_STATE)
		{
			if(FadeCurrentState == FADE_OUT)
			{
				SelectNextState = START_GAME_STATE;
				SetFade();
			}
			if(MouseButton[0] || MouseDblClick || MapBlendCount >= 255)
				SelectNextState = START_GAME_STATE;
		}
	}
	
	//FadeOut이 완료된 경우..
	if(FadeCurrentState == FADE_OUT && FadeNextState == FADE_OUT && SelectNextState == RETURN_OPENING_STATE)
	{
		if(smWsockDataServer != NULL)
		{
			DisconnectServerFull();
			//DisconnectServer_GameMain();
		}
		InitUserCharacter();
		return 2;
	}
	
	//상태 이동중...
	if(SelectCurrentState != SelectNextState)
	{
		if(SelectCurrentState != COMPLETE_SELECT_STATE && SelectNextState != COMPLETE_SELECT_STATE &&
		   SelectCurrentState != START_GAME_STATE && SelectNextState != START_GAME_STATE)
		{
			//RaceSelect -> JobSelect 로 이동중...
			if( (SelectCurrentState == RACE_SELECT_STATE) && (SelectNextState == JOB_SELECT_STATE))
				RaceMarkScaleSize--;
			
			MouseDownCharacterNum = -1;
			MouseMoveCharacterNum = -1;
			RaceMarkBlendCount = 0;
			
			//상태변화에(상태 변화 도중) 따른 카메라의 이동..
			static int step = 1000;
			if( SelectNextState >= 0 && SelectNextState < COMPLETE_SELECT_STATE &&
				SelectCurrentState >= 0 && SelectCurrentState < COMPLETE_SELECT_STATE)
			{
				if( SelectNextState > SelectCurrentState) //Camera가 뒤로 빠지는 경우..
				{
					
					TarDist += step;
					if(TarDist > CameraZPos[SelectNextState]+800)
					{
						TarDist = CameraZPos[SelectNextState];
						SelectCurrentState = SelectNextState;
						step = 1000;
					}

					if( (CameraZPos[SelectNextState]-CameraZPos[SelectCurrentState])/16*15+CameraZPos[SelectCurrentState] < TarDist)
						step = 100;
				}
				else if(SelectNextState < SelectCurrentState) //Camera가 앞으로 가는 경우.
				{
					TarDist -= step;
					if(TarDist < CameraZPos[SelectNextState]-800)
					{
						TarDist = CameraZPos[SelectNextState];
						SelectCurrentState = SelectNextState;
						step = 1000;
					}

					if( (CameraZPos[SelectCurrentState]-CameraZPos[SelectNextState])/16+CameraZPos[SelectNextState] > TarDist)
						step = 100;
				}
			}
		}

		if(SelectNextState == COMPLETE_SELECT_STATE)
		{
			if(FadeCurrentState == FadeNextState)
			{
				SelectCurrentState = COMPLETE_SELECT_STATE;
			}
		}
		
		if(SelectNextState == START_GAME_STATE)
		{
			SelectCurrentState = START_GAME_STATE;
		}
		
		if(SelectCurrentState == FACE_SELECT_STATE && SelectNextState == JOB_SELECT_STATE)
		{
			JobBodySelectNum = -1;
			JobFaceSelectNum = -1;
			memset(UserCharacterNewName, 0, sizeof(UserCharacterNewName));
			SetWindowText(hTextWnd, 0);
			hFocusWnd = NULL;
		}
		
		
		if(SelectCurrentState == JOB_SELECT_STATE && SelectNextState == FACE_SELECT_STATE)
		{
 			if(JobFaceSelectNum == -1)
			{
				JobFaceSelectNum = 1;
				hFocusWnd    = hTextWnd;
				FocusWndFlag = TRUE;
#ifdef	_LANGUAGE_JAPANESE		//C7
				SetIME_Mode( 1 );
#endif

				FaceSelectFirstFlag = FALSE;
			}

				/*
				switch(tribeNum)
				{
				case TRIBE_Tempscron:	
					if(JobBodySelectNum == 0) //Mech
						SetChangeJobFace(szTempMechBodyName[0], szTempMechFaceName[0][JobFaceSelectNum], JobBodySelectNum);
					else if(JobBodySelectNum == 1) // Fighter
						SetChangeJobFace(szTempFighterBodyName[0], szTempFighterFaceName[0][JobFaceSelectNum], JobBodySelectNum);
					else if(JobBodySelectNum == 2) // Pike
						SetChangeJobFace(szTempPikeBodyName[0], szTempPikeFaceName[0][JobFaceSelectNum], JobBodySelectNum);
					else if(JobBodySelectNum == 3) // Archer
						SetChangeJobFace(szTempArcherBodyName[0], szTempArcherFaceName[0][JobFaceSelectNum], JobBodySelectNum);

					break;

				case TRIBE_Moryon:

					if(JobBodySelectNum == 0) //Mech
						SetChangeJobFace(szMorMechBodyName[0], szMorMechFaceName[0][JobFaceSelectNum], JobBodySelectNum);
					else if(JobBodySelectNum == 1) // Fighter
						SetChangeJobFace(szMorFighterBodyName[0], szMorFighterFaceName[0][JobFaceSelectNum], JobBodySelectNum);
					else if(JobBodySelectNum == 2) // Pike
						SetChangeJobFace(szMorPikeBodyName[0], szMorPikeFaceName[0][JobFaceSelectNum], JobBodySelectNum);
					else if(JobBodySelectNum == 3) // Archer
						SetChangeJobFace(szMorArcherBodyName[0], szMorArcherFaceName[0][JobFaceSelectNum], JobBodySelectNum);

					break;
				}
				*/

			//}
		}

	}
	
	//상태 변화 중이거나, 변화중이 아닐 때 아무때나...
 	if(SelectNextState == FACE_SELECT_STATE)// Character Name을 쓸 때..
	{
		
   		if(JobCharacter[JobBodySelectNum].pZ < -415*fONE && CharacterStopFlag==FALSE) //JobCharacter에서 고른 캐릭터가 어느정도 앞으로 나오면 서있게 한다.
		{
			JobCharacter[JobBodySelectNum].MoveFlag = FALSE;
			JobCharacter[JobBodySelectNum].Angle.x = 0;
			JobCharacter[JobBodySelectNum].Angle.y = ANGLE_180;
			JobCharacter[JobBodySelectNum].Angle.z = 0;
			JobCharacter[JobBodySelectNum].ActionPattern = 0;
			JobCharacter[JobBodySelectNum].pZ = -429*fONE;

			CharacterStopFlag = TRUE;
		}
	}
	
	return TRUE;

}

int HoLogin::MainDefaultButton()
{
	if(SelectSubState != SUB_SELECT_NONE || ErrorSelect != ERROR_NONE)
	   return FALSE;
	
	if(SelectCurrentState == SelectNextState && SelectCurrentState == CHARACTER_SELECT_STATE)
	{
		if(MouseDownCharacterNum != -1)
		{
			//Character를 삭제할때.
			if(DeleteButton.Main(pCursorPos.x, pCursorPos.y, MouseButton[0]))
			{
				SelectSubState = SUB_SELECT_CHARACTER_MANAGE;
				MessageBox.Flag = TRUE;
				PlayWav2(4-1);
				

			}
		}

		int userCount = 0;
		int index; for( index = 0; index < DownUserCharacterCount; index++)
		{
			if(UserCharacter[index].DisplayFlag)
				userCount++;
		}
		
		if(userCount == 0)
		{
			if(NewCharButton.Main(pCursorPos.x, pCursorPos.y, MouseButton[0]))
			{
				if(userCount == 0)
				{
					SelectNextState = RACE_SELECT_STATE;
					PlayWav2(5-1);
				}
			}
		}
	}

	
	if(OkButton.Main(pCursorPos.x, pCursorPos.y, MouseButton[0]))
	{
		PlayWav(4-1);
		//User Character를 선택한 경우...
		if(SelectCurrentState == CHARACTER_SELECT_STATE)
		{
			if(MouseDownCharacterNum != -1)
			{
				MessageBox.Flag = TRUE;
				SelectSubState = SUB_SELECT_CHARACTER_USER;
			}

			if(MouseDownCharacterNum == -1)
				ErrorSelect = ERROR_SELECT_CHAR;
		}
		else if(SelectCurrentState == RACE_SELECT_STATE)
		{
			ErrorSelect = ERROR_SELECT_RACE;
		}
		else if(SelectCurrentState == JOB_SELECT_STATE)
		{
			if(MouseDownCharacterNum != -1)
			{
				SelectNextState = FACE_SELECT_STATE;
				JobCharacter[JobBodySelectNum].SetTargetPosi(0, (-420*fONE)>>FLOATNS);
				JobCharacter[JobBodySelectNum].MoveMode = TRUE;
				
				JobCharacter[JobBodySelectNum].smCharInfo.Weight[0] = 0;
				JobCharacter[JobBodySelectNum].smCharInfo.Weight[1] = 1000;
				JobCharacter[JobBodySelectNum].smCharInfo.Stamina[0] = 1000;
				
				JobCharacter[JobBodySelectNum].smCharInfo.ArrowPosi[0] = 0;
				JobCharacter[JobBodySelectNum].smCharInfo.ArrowPosi[1] = 0;

				JobCharacter[JobBodySelectNum].Flag = 1;
				OpenEachPlayer(&JobCharacter[JobBodySelectNum]);
				MouseDownCharacterNum = -1;
				CharacterReturnFlag = TRUE;
			}

			if(JobBodySelectNum == -1)
				ErrorSelect = ERROR_SELECT_JOB;
			
		}
		else if(SelectCurrentState == FACE_SELECT_STATE)
		{
			//NewCharacter 게임을 들어갈 경우..
			if(JobBodySelectNum != -1 && JobFaceSelectNum != -1)
			{
				hFocusWnd = NULL;
				if(strlen(UserCharacterNewName) > 0)//New Player의 이름이 있으면..
				{
					MessageBox.Flag = TRUE;
					SelectSubState = SUB_SELECT_CHARACTER_NEW;
				}
				else
					ErrorSelect = ERROR_NONAME_NEWPLAYER;
			}

			if(JobFaceSelectNum == -1)
			{
				hFocusWnd = NULL;
				ErrorSelect = ERROR_SELECT_FACE;
			}
		}
		
	}
	
	//Button 처리 
	if(QuitButton.Main(pCursorPos.x, pCursorPos.y, MouseButton[0]))
	{
		PlayWav(4-1);
		SelectSubState = SUB_SELECT_BACK;
	}
	
	if(BackButton.Main(pCursorPos.x, pCursorPos.y, MouseButton[0]) && SelectCurrentState != CHARACTER_SELECT_STATE)
	{
		PlayWav(7-1);
		SelectNextState = SelectCurrentState-1;
	}
	
	return TRUE;
}


int HoLogin::InitUserCharacter()
{
	int index; for( index = 0; index < CHARACTER_USER_MAX; index++)
	{
		if(UserCharacter[index].DisplayFlag != FALSE)
			UserCharacter[index].Init();
	}
	UserCharacterCount =0;
	
	return TRUE;
}
int HoLogin::DrawJobCharacter(int x, int y, int z, int ax, int ay, int az)
{	
	
	smCHAR_SetCameraPosi( x,y,z, ax , ay , az );
	int index;
	BOOL mouseDownFlag = FALSE;
	for(index=0; index < JobCharacterCount; index++)
	{
		if(SelectCurrentState == JOB_SELECT_STATE 
			&& ErrorSelect == ERROR_NONE
			&& SelectCurrentState == SelectNextState)
		{
			smRender.Color_R = 0;
			smRender.Color_G = 0;
			smRender.Color_B = 0;
			
			
			if(GetSelectCharacter(&JobCharacter[index]) == 1) //마우스가 캐릭터 위에 있을 경우.
			{
				MouseMoveCharacterNum = index;
				smRender.Color_R = 50;
				smRender.Color_G = 50;
				smRender.Color_B = 50;
				
			}
			else if(GetSelectCharacter(&JobCharacter[index]) == 2 ) //마우스로 캐릭터를 누른 경우,
			{
				PlayWav(8-1);
				MouseDownCharacterNum = index;
				mouseDownFlag = TRUE;
				//JobCharacter[index].ChangeMotion(11);
			}
			
			if(mouseDownFlag == FALSE && MouseButton[0]&& pCursorPos.x > 0 && pCursorPos.x < smScreenWidth && pCursorPos.y > 70 && pCursorPos.y < (smScreenHeight-50)) 
				MouseDownCharacterNum = -1;

			if(MouseDownCharacterNum == index )
			{
				JobBodySelectNum = index;
				smRender.Color_R = 120;
				smRender.Color_G = 120;
				smRender.Color_B = 120;
			
				
				//Job캐릭터를 선택했을 경우...
				if(MouseDblClick)
				{
					SelectNextState = FACE_SELECT_STATE;
					
					JobCharacter[JobBodySelectNum].smCharInfo.Stamina[0] = 100;
					JobCharacter[JobBodySelectNum].smCharInfo.Stamina[1] = 100;
					
					JobCharacter[JobBodySelectNum].SetTargetPosi(0, (-420*fONE)>>FLOATNS);
					JobCharacter[JobBodySelectNum].MoveMode = TRUE;
					CharacterReturnFlag = TRUE;
					
					JobCharacter[JobBodySelectNum].smCharInfo.Weight[0] = 0;
					JobCharacter[JobBodySelectNum].smCharInfo.Weight[1] = 1000;
					JobCharacter[JobBodySelectNum].smCharInfo.Stamina[0] = 1000;
					//JobCharacter[JobBodySelectNum].smCharInfo.State = smCHAR_STATE_NPC;
					JobCharacter[JobBodySelectNum].smCharInfo.ArrowPosi[0] = 0;
					JobCharacter[JobBodySelectNum].smCharInfo.ArrowPosi[1] = 0;

					JobCharacter[JobBodySelectNum].Flag = 1;

					OpenEachPlayer(&JobCharacter[JobBodySelectNum]);
					MouseDblClick = FALSE;
					MouseButton[0] = FALSE;
				}
			}
		}
		JobCharacter[index].Draw();
		smRender.Color_R = 0;
		smRender.Color_G = 0;
		smRender.Color_B = 0;
	}

	return TRUE;
}
int HoLogin::DrawUserCharacter(int x, int y, int z, int ax, int ay, int az)
{
	smCHAR_SetCameraPosi( x,y,z, ax , ay , az );
	
	BOOL mouseDownFlag = FALSE;
	MouseMoveCharacterNum = -1;
	
	int index; for( index =0; index < CHARACTER_USER_MAX; index++)
	{
		if( SelectCurrentState == CHARACTER_SELECT_STATE &&
			ErrorSelect == ERROR_NONE)
		{
			smRender.Color_R = 0;
			smRender.Color_G = 0;
			smRender.Color_B = 0;
			
			if(SelectSubState == SUB_SELECT_NONE)
			{
				if(GetSelectCharacter(&UserCharacter[index]) == 1 && 
					SelectNextState == CHARACTER_SELECT_STATE) //마우스가 캐릭터 위에 있을 경우.
				{
					MouseMoveCharacterNum = index;
					smRender.Color_R = 50;
					smRender.Color_G = 50;
					smRender.Color_B = 50;
				}
				else if(GetSelectCharacter(&UserCharacter[index]) == 2 &&
					SelectNextState == CHARACTER_SELECT_STATE) //마우스로 캐릭터를 누른 경우,
				{
					PlayWav(8-1);
					MouseMoveCharacterNum = index;
					mouseDownFlag = TRUE;
					MouseDownCharacterNum = index;
				}
				
				if(mouseDownFlag == FALSE && MouseButton[0] && 
					pCursorPos.x > 0 && pCursorPos.x < smScreenWidth &&
					pCursorPos.y > 60 && pCursorPos.y < (smScreenHeight-60)) //마우스를 눌렀는데, 누른 위치에 캐릭터가 없으면 값을 초기화 한다.
				{
					MouseDownCharacterNum = -1;
				}
			}

 			
			if(MouseDownCharacterNum == index) 
			{
				smRender.Color_R = 120;
				smRender.Color_G = 120;
				smRender.Color_B = 120;
			}
			
		}
		/*
		if(MouseDownCharacterNum == index)
		{
			smRender.Color_R = 120;
			smRender.Color_G = 120;
			smRender.Color_B = 120;
		}
		*/

		if(UserCharacter[index].DisplayFlag)
			UserCharacter[index].Draw();
		
		smRender.Color_R = 0;
		smRender.Color_G = 0;
		smRender.Color_B = 0;
	}
	
	if(SelectCurrentState == CHARACTER_SELECT_STATE)
	{
		if(MouseDownCharacterNum != -1 && MouseDblClick && pCursorPos.y < smScreenHeight-50)
		{
			MessageBox.Flag = TRUE;
			SelectSubState = SUB_SELECT_CHARACTER_USER;
		}
	}
	return TRUE;
}

int HoLogin::DrawTitle(int state)
{
	//타이틀 박스
     dsDrawTexImage( TitleBoxFrameImage, smScreenWidth/2-256/2, 0, 256, 128, 255);
 	if(state == CHARACTER_SELECT_STATE)
		HoDrawTexImage( TitleNameImage, float(smScreenWidth/2-64), 53.f, 128.f, 30.f, 0.f, 0.f, 128.f, 30.f, 255);
	else if(state == RACE_SELECT_STATE)
		HoDrawTexImage( TitleNameImage, float(smScreenWidth/2-64), 53.f, 128.f, 30.f, 0.f, 30.f, 128.f, 30.f, 255);
	else if(state == JOB_SELECT_STATE)
		HoDrawTexImage( TitleNameImage, float(smScreenWidth/2-64), 53.f, 128.f, 30.f, 0.f, 60.f, 128.f, 30.f, 255);
	else if(state == FACE_SELECT_STATE)
		HoDrawTexImage( TitleNameImage, float(smScreenWidth/2-64), 53.f, 128.f, 30.f, 0.f, 90.f, 128.f, 30.f, 255);

	return TRUE;
}


//게임 창틀 출력...
int HoLogin::DrawGameFrame()
{
	//아래위 검은 부분과 테두리(게임 창틀)
	if(SelectNextState != START_GAME_STATE)
	{
     	dsDrawTexImage( BlackBoxImage, 0, 0,                 smScreenWidth, 70, 150);
    	dsDrawTexImage( BlackBoxImage, 0, smScreenHeight-50, smScreenWidth, 70, 150);
		
   		int frameViewportWidth  = smScreenWidth;
 	 	int frameViewportHeight = smScreenHeight-140;

   		int lineWidthCount   = frameViewportWidth/64;
		int lineWidthRemain  = frameViewportWidth%64;
		int lineHeightCount  = frameViewportHeight/64;
		int lineHeightRemain = frameViewportHeight%64;
		
 		//가로 라인 긋기.
       	int index; for( index = 0; index < lineWidthCount; index++)
		{
	 		dsDrawTexImage( MainBoxFrameImage1[1], 4+64*index,                68,     64, 8, 255);
  			dsDrawTexImage( MainBoxFrameImage1[3], 4+64*index, smScreenHeight-54,     64, 8, 255);
		}
	
 		HoDrawTexImage( MainBoxFrameImage1[1], float(4+(index*64)), 68.f, (float)lineWidthRemain, 8.f, 
                     			0.f, 0.f, (float)lineWidthRemain, 8.f, 255);
		HoDrawTexImage( MainBoxFrameImage1[3], float(4+(index*64)), float(smScreenHeight-74), (float)lineWidthRemain, 8.f, 
                    			0.f, 0.f, (float)lineWidthRemain, 8.f, 255);

		//세로 라인 긋기
      	for(index = 0; index < lineHeightCount; index++)
		{
  			dsDrawTexImage( MainBoxFrameImage1[0], 0,               70+64*index, 8, 64, 255);
  			dsDrawTexImage( MainBoxFrameImage1[2], smScreenWidth-8, 70+64*index, 8, 64, 255);
		}
    		if(lineHeightRemain > 0)
		{
			HoDrawTexImage( MainBoxFrameImage1[0], 0.f,                    float(70+(index*64)), 8.f, (float)lineHeightRemain, 
              			0.f, 0.f, 8.f, (float)lineHeightRemain, 255);
			HoDrawTexImage( MainBoxFrameImage1[2], float(smScreenWidth-8), float(70+(index*64)), 8.f, (float)lineHeightRemain, 
             			0.f, 0.f, 8.f, (float)lineHeightRemain, 255);
		}
		//장식 두르기
		 dsDrawTexImage( MainBoxFrameImage2[0], 0,                 68, 130, 64, 255);
        dsDrawTexImage( MainBoxFrameImage2[1], smScreenWidth-130, 68, 130, 64, 255);
		dsDrawTexImage( MainBoxFrameImage2[2], 0, smScreenHeight-(64+48), 130, 64, 255);
		dsDrawTexImage( MainBoxFrameImage2[3], smScreenWidth-130, smScreenHeight-(64+48), 130, 64, 255);
	}

	return TRUE;
}

int HoLogin::DrawLight()
{
	//등불아래 광원..
	static BOOL flag = TRUE;
	static int tt = 0;
	smRender.ClearLight();
	
	if(FadeCurrentState == FadeNextState && SelectCurrentState != COMPLETE_SELECT_STATE)
	{
		smRender.AddDynamicLight( -200*fONE+tt*fONE, 0, -300*fONE, 80, 30, 0, 255, 40000+tt*fONE);
		smRender.AddDynamicLight( 200*fONE-tt*fONE, 0, -300*fONE, 80, 30, 0, 255, 40000+tt*fONE);
		
		if(SelectCurrentState == FACE_SELECT_STATE)
			smRender.AddDynamicLight( 0, 0, -430*fONE, 110, 40, 0, 255, 10000+tt*fONE);
		
		if(tt == 40)
			flag = TRUE;
		else if(tt == 0)
			flag = FALSE;
		if(flag)
			tt--;
		else tt++;
	}

	return TRUE;
}

int HoLogin::DrawMap(int alpha)
{
	lpD3DDevice->BeginScene();
	dsDrawTexImage(BlackBoxImage,0, 0, smScreenWidth, smScreenHeight, 255);
	lpD3DDevice->EndScene();
	
	if(GameMapImage)
	{
		DrawSprite((smScreenWidth/2-800/2), (smScreenHeight/2-600/2), GameMapImage, 0, 0,800, 600);
	}
	return TRUE;
}

int HoLogin::DrawCharacterInfo()
{
	if(ErrorSelect != ERROR_NONE)
		return FALSE;
	
  	lpD3DDevice->BeginScene();
	if(SelectCurrentState == CHARACTER_SELECT_STATE && MouseMoveCharacterNum != -1 && SelectCurrentState == SelectNextState
 		&& FadeCurrentState == FadeNextState && ErrorSelect == ERROR_NONE)
	{
    	dsDrawTexImage( UserCharacterInfoBackImage, smScreenWidth/2-128, smScreenHeight/2+smScreenHeight/5, 256, 128, 255);
  		if(DownUserCharacterInfo[MouseMoveCharacterNum].ClanFlag)
			dsDrawTexImage( UserCharacterInfoImage, smScreenWidth/2-128, smScreenHeight/2+smScreenHeight/5, 256, 128, 255);
		else
			HoDrawTexImage( UserCharacterInfoImage, smScreenWidth/2-128, smScreenHeight/2+smScreenHeight/5+13, 0, 33, 256, 128-33, 255);
	}
   	lpD3DDevice->EndScene();
#ifdef _LANGUAGE_VEITNAM
   	int textX = smScreenWidth/2-128+102+13;
	int textY = smScreenHeight/2+smScreenHeight/5-5;
#else
   	int textX = smScreenWidth/2-128+102;
	int textY = smScreenHeight/2+smScreenHeight/5;
#endif
   	if(DownUserCharacterInfo[MouseMoveCharacterNum].ClanFlag == TRUE)
		textY += 20;
 	
	HDC hdc;
	char *szJobName;
  	lpDDSBack->GetDC( &hdc );
	SetBkMode( hdc, TRANSPARENT );
	SelectObject( hdc , Font );

	
	if(DownUserCharacterInfo[MouseMoveCharacterNum].ClanFlag) //클랜 이름찍기..
	{
		SetTextColor( hdc, RGB(230, 240, 255) );
 		dsTextLineOut(hdc, textX+17, textY, DownUserCharacterInfo[MouseMoveCharacterNum].ClanName, strlen(DownUserCharacterInfo[MouseMoveCharacterNum].ClanName));
	}
	
	SetTextColor( hdc, RGB(255, 220, 100) );
	char buffer[256];
	memset(buffer, 0, sizeof(buffer));
	wsprintf(buffer, "%d", DownUserCharacterInfo[MouseMoveCharacterNum].Level);
	dsTextLineOut(hdc, textX, textY+40, buffer, strlen(buffer));

  	SetTextColor( hdc, RGB(255, 250, 235) );
	dsTextLineOut(hdc, textX, textY+20, DownUserCharacterInfo[MouseMoveCharacterNum].Name, strlen(DownUserCharacterInfo[MouseMoveCharacterNum].Name));
	
	int jobCode = DownUserCharacterInfo[MouseMoveCharacterNum].Job;
	jobCode = jobCode & 0xFFFF;
	memset(buffer, 0, sizeof(buffer));
	if( JOB_IS_TEMPSKRON(jobCode) ) {
		wsprintf(buffer, "%s", HoTextTempskron );			//"템스크론"
	}
	else if(jobCode <= JOBCODE_MAX) {
		wsprintf(buffer, "%s", HoTextMoryon );			//"템스크론"
	}
	dsTextLineOut(hdc, textX, textY+60, buffer, strlen(buffer));

	{
		JOB_DATA_CODE *lpJobData = GetJobDataCode( DownUserCharacterInfo[MouseMoveCharacterNum].Job , 0 );
		szJobName = lpJobData ? lpJobData->szName2 : (char *)"";
	}
	memset(buffer, 0, sizeof(buffer));
	wsprintf(buffer, "%s", szJobName );//"파이터");
	dsTextLineOut(hdc, textX, textY+80, buffer, strlen(buffer));
	

 	lpDDSBack->ReleaseDC( hdc );
	
	
	//마크 찍기..
 	if(DownUserCharacterInfo[MouseMoveCharacterNum].ClanFlag)
	{
		if(DownUserCharacterInfo[MouseMoveCharacterNum].ClanMark != NULL)
		{
			RECT tempRect;
			tempRect.left = 0;
			tempRect.top  = 0;
			tempRect.right = tempRect.left + 16;
			tempRect.bottom = tempRect.top + 16;
			
			lpDDSBack->BltFast(textX-3,textY-3,DownUserCharacterInfo[MouseMoveCharacterNum].ClanMark
 				,&tempRect,DDBLTFAST_WAIT|DDBLTFAST_SRCCOLORKEY);


			POINT ResultSize;
			ResultSize.x = 0; //초기화 
			ResultSize.y = 0;

			DDSURFACEDESC2	ddTemp;
			memset(&ddTemp,0,sizeof(DDSURFACEDESC2));
			ddTemp.dwSize = sizeof(ddTemp);
			DownUserCharacterInfo[MouseMoveCharacterNum].ClanMark->GetSurfaceDesc(&ddTemp);

			ResultSize.y = ddTemp.dwHeight;
			ResultSize.x = ddTemp.dwWidth;

			memset(&ddTemp,0,sizeof(DDSURFACEDESC2));
			ddTemp.dwSize = sizeof(ddTemp);

			if( chacldata[0].hClanMark16 != NULL ) //2007.03.27 EPT에서 디버깅모드와 클라이언트배포판에서 에러남
			{
				chacldata[0].hClanMark16->GetSurfaceDesc(&ddTemp);
			}
			else
			{
				// 버그다!!
				int kkk=0;	//브레이킹포인트를 위한 쓸모없는 변수선언
			}

			ResultSize.y = ddTemp.dwHeight;
			ResultSize.x = ddTemp.dwWidth;


		}
	}
	
	return TRUE;
}

//Sub 상태 관리..
int HoLogin::DrawSelectSubState()
{
	if(SelectNextState == COMPLETE_SELECT_STATE || 
	   SelectNextState == START_GAME_STATE)
		return FALSE;

	int flag = 0;
	switch(SelectSubState)
	{
		case SUB_SELECT_CHARACTER_MANAGE:
		{
			flag = MessageBox.Main(pCursorPos.x, pCursorPos.y, MouseButton[0]); //TRUE
			if(flag == 1) //Ok(삭제 할 경우)
			{
				PlayWav2(4-1);
				//ktj : 캐릭터 삭제시 클랜에 등록된 사람인지의 여부첵크해 클랜장인경우 못지움.
				int isDELETE_character(char *charName) ;
				int chk = isDELETE_character(DownUserCharacterInfo[MouseDownCharacterNum].Name);
				//chk = 103;
 				if(chk == 102 || chk == 1) //클랜 가입안된 캐릭..
				{
					PlayWav2(6-1);
					UserCharacter[MouseDownCharacterNum].Init();
					UserCharacterCount--;
					
					//지울 캐릭터를 서버에 보낸다.
					if(smWsockDataServer)
					{
						TransUserCommand(smTRANSCODE_DELRECORDDATA, UserAccount, DownUserCharacterInfo[MouseDownCharacterNum].Name);
					}
					MouseDownCharacterNum = -1;
					SelectSubState = SUB_SELECT_NONE;
				}
				else if(chk == 101 || chk == 100) //클랜장, 일반 클랜
				{
					ErrorSelect = ERROR_DELETE_CLAN_MEMBER;
					SelectSubState = SUB_SELECT_NONE;
				}
				else if(chk == 103) //클랜 서버 접속 불가.
				{
					ErrorSelect = ERROR_CLAN_SERVER_NONE;
					SelectSubState = SUB_SELECT_NONE;
				}
				else
				{
					ErrorSelect = ERROR_CLAN_SERVER_NONE;
					SelectSubState = SUB_SELECT_NONE;
				}

 			}
			else if(flag == 2) //Cancel(삭제 안할 경우..)
			{
				PlayWav2(4-1);
				SelectSubState = SUB_SELECT_NONE;
			}
			
			break;
		}
		
		case SUB_SELECT_CHARACTER_USER:
		{
			flag = MessageBox.Main(pCursorPos.x, pCursorPos.y, MouseButton[0]); //TRUE
			if(flag == 1 || VRKeyBuff[VK_RETURN]) //OK
			{
				PlayWav2(12-1);
				SelectNextState = COMPLETE_SELECT_STATE;
				MessageBox.Flag = FALSE;

				LoadLoadingImage(DownUserCharacterInfo[MouseDownCharacterNum].Level);
				SetFade();
			}
			else if(flag == 2) //Cancel
			{
				PlayWav2(4-1);
				SelectSubState = SUB_SELECT_NONE;
				MessageBox.Flag = FALSE;
			}
			break;
		}

		case SUB_SELECT_CHARACTER_NEW:
		{
			flag = MessageBox.Main(pCursorPos.x, pCursorPos.y, MouseButton[0]); //TRUE
			if(flag == 1 || VRKeyBuff[VK_RETURN]) //OK
			{
				PlayWav2(12-1);
				ServerReturn = NULL;
				TransUserCommand(smTRANSCODE_INSRECORDDATA, UserAccount, UserCharacterNewName);
				SelectSubState = SUB_SELECT_CHARACTER_NEW_SERVER;
				LoadLoadingImage(0);
			}
			else if(flag == 2) //Cancel
			{
				PlayWav2(4-1);
				SelectSubState = SUB_SELECT_NONE;
				hFocusWnd = hTextWnd;
				FocusWndFlag = TRUE;
#ifdef	_LANGUAGE_JAPANESE		//C7
				SetIME_Mode( 1 );
#endif
			}
			break;
		}

		case SUB_SELECT_CHARACTER_NEW_SERVER:
		{
			if(ServerReturn != NULL)
			{
				if(ServerReturn->WParam == 1)
				{
					SelectNextState = COMPLETE_SELECT_STATE;
					SetFade();
					memset(PlayerName, 0, sizeof(PlayerName));
					memcpy(PlayerName, UserCharacterNewName, sizeof(PlayerName));
				}
				else
				{
					memset(UserCharacterNewName, 0, sizeof(UserCharacterNewName));
					ErrorSelect = ERROR_EXHIST_NEWPLAYER;
					SelectSubState = SUB_SELECT_NONE;
				}
			}
			break;
		}
		
		case SUB_SELECT_BACK:
		{

			SetFade();
			SelectSubState = SUB_SELECT_NONE;
			SelectNextState = RETURN_OPENING_STATE;
			break;
		}
	}
	
	return TRUE;
}

#include "..\\language.h"
#include "..\\SrcLang\\jts.h"

int HoLogin::KeyFilter()
{
	
	int start;
	int result=0;

	if(hFocusWnd != NULL && JobFaceSelectNum != -1 && hTextWnd != NULL)
	{
		
		SendMessage(hTextWnd, EM_SETLIMITTEXT, 15, 0);
		memset(UserCharacterNewName, 0, sizeof(UserCharacterNewName));
		GetWindowText(hFocusWnd, UserCharacterNewName, sizeof(UserCharacterNewName));


		//KeyFilter
		int index; for( index = 0; index < (int)lstrlen(UserCharacterNewName); index++)
		{
			/*
			if(UserCharacterNewName[index] == '@' ||
			   UserCharacterNewName[index] == '~' ||
			   UserCharacterNewName[index] == '`' ||
			   UserCharacterNewName[index] == '!' ||
			   UserCharacterNewName[index] == '#' ||
			   UserCharacterNewName[index] == '$' ||
			   UserCharacterNewName[index] == '%' ||
			   UserCharacterNewName[index] == '&' ||
			   UserCharacterNewName[index] == '*' ||
			   UserCharacterNewName[index] == '+' ||
			   UserCharacterNewName[index] == '=' ||
			   UserCharacterNewName[index] == '|' ||
			   UserCharacterNewName[index] == '\\'||
			   UserCharacterNewName[index] == '{' ||
			   UserCharacterNewName[index] == '}' ||
			   UserCharacterNewName[index] == ':' ||
			   UserCharacterNewName[index] == ';' ||
			   UserCharacterNewName[index] == '"' ||
			   UserCharacterNewName[index] == '\'' ||
			   UserCharacterNewName[index] == ',' ||
			   UserCharacterNewName[index] == '.' ||
			   UserCharacterNewName[index] == '?' ||
			   UserCharacterNewName[index] == '/' ||
			   UserCharacterNewName[index] == '\t' ||
			   UserCharacterNewName[index] == ' ' ||
			   UserCharacterNewName[index] == '<' ||
			   UserCharacterNewName[index] == '>' ||
			   UserCharacterNewName[index] == '|')
			{
			*/

#ifdef	_LANGUAGE_VEITNAM
//			unsigned char UC = memcpy(&UC,&UserCharacterNewName[index],sizeof(UC));
			if(
			   (UserCharacterNewName[index] <    0)												||
			   (UserCharacterNewName[index] >=   0 && UserCharacterNewName[index] <=  47)		||
			   (UserCharacterNewName[index] >=  58 && UserCharacterNewName[index] <=  64)		||
			   (UserCharacterNewName[index] >=  91 && UserCharacterNewName[index] <=  96)		||
			   (UserCharacterNewName[index] >= 123 && UserCharacterNewName[index] <= 127)		||
			   (UserCharacterNewName[index] == 0x09 )											||	//탭키 방지
			   (UserCharacterNewName[index] == 0xff || UserCharacterNewName[index] == 0x20) 		// 스페이스 블럭
			   )
			{
				UserCharacterNewName[index] = 0;
				SetWindowText(hFocusWnd, 0);
				SetWindowText(hFocusWnd, UserCharacterNewName);
				SendMessage(hFocusWnd, EM_SETSEL, lstrlen(UserCharacterNewName), lstrlen(UserCharacterNewName)+1);
				break;
			}
#else
//중국
#ifdef _LANGUAGE_CHINESE
			//cursefilter.cht 파일을 읽어들여 해당 텍스트를 지워버린다.
			if(IsCurseID(UserCharacterNewName)==TRUE)
				{
					UserCharacterNewName[index] = 0;
					SetWindowText(hFocusWnd, 0);
					SetWindowText(hFocusWnd, UserCharacterNewName);
					SendMessage(hFocusWnd, EM_SETSEL, lstrlen(UserCharacterNewName), lstrlen(UserCharacterNewName)+1);
					break;
				}
			if(
			   (UserCharacterNewName[index] == 86 || UserCharacterNewName[index] == 118 )	||
			   (UserCharacterNewName[index] <  48 && UserCharacterNewName[index] > 0)	||
			   (UserCharacterNewName[index] >= 58 && UserCharacterNewName[index] <= 64) ||
			   (UserCharacterNewName[index] >= 91 && UserCharacterNewName[index] <= 96) ||
			   (UserCharacterNewName[index] >= 123 && UserCharacterNewName[index] <= 127) ||
			   (UserCharacterNewName[index] == 0x09 ) ||										//탭키 방지
			   (UserCharacterNewName[index] == ' ') ||										    //빈공백을 메꿔야 하는데 우선 임시로..
			   (UserCharacterNewName[index] == 0xff || UserCharacterNewName[index] == 0x20) 		// 스페이스 블럭
			   )
			{
				UserCharacterNewName[index] = 0;
				SetWindowText(hFocusWnd, 0);
				SetWindowText(hFocusWnd, UserCharacterNewName);
				SendMessage(hFocusWnd, EM_SETSEL, lstrlen(UserCharacterNewName), lstrlen(UserCharacterNewName)+1);
				break;
			}
			//전각금지, 반각허용

			if(UserCharacterNewName[index] == -93)
			{																							//Unicode(중국어 전각)
				if( UserCharacterNewName[index+1] >= -95 && UserCharacterNewName[index+1] <= -2  )
				{
					UserCharacterNewName[index] = 0;
					SetWindowText(hFocusWnd, 0);
					SetWindowText(hFocusWnd, UserCharacterNewName);
					SendMessage(hFocusWnd, EM_SETSEL, lstrlen(UserCharacterNewName), lstrlen(UserCharacterNewName)+1);
					break;
				}
			}
			
#else
#ifdef _LANGUAGE_JAPANESE
			if(IsCurseID(UserCharacterNewName)==TRUE)
			{
				UserCharacterNewName[index] = 0;
				SetWindowText(hFocusWnd, 0);
				SetWindowText(hFocusWnd, UserCharacterNewName);
				SendMessage(hFocusWnd, EM_SETSEL, lstrlen(UserCharacterNewName), lstrlen(UserCharacterNewName)+1);
				break;
			}
			if(index>0)
			{
				if((((UserCharacterNewName[index-1]&0xff) >= 0x81) && ((UserCharacterNewName[index-1]&0xff) <= 0x9F))
					||  (((UserCharacterNewName[index-1]&0xff) >= 0xE0) && ((UserCharacterNewName[index-1]&0xff) <= 0xEF)))
				{
						index = index;
				}
				else if( (UserCharacterNewName[index] >= 0x1f && UserCharacterNewName[index] <= 0x2f) ||
					(UserCharacterNewName[index] >= 0x3a && UserCharacterNewName[index] <= 0x40) ||
					(UserCharacterNewName[index] == 0x5c ) ||
					(UserCharacterNewName[index] == 0x5e ) ||
					(UserCharacterNewName[index] == 0x5f ) ||
					(UserCharacterNewName[index] == 0x60 ) ||
					(UserCharacterNewName[index] == 0x09 ) ||
//					(UserCharacterNewName[index] >= 0x80 || UserCharacterNewName[index] >= 0xff)
					(UserCharacterNewName[index] >= 0x7b && UserCharacterNewName[index] <= 0xff) ) //0x7f) )
				{
					UserCharacterNewName[index] = 0;
					SetWindowText(hFocusWnd, 0);
					SetWindowText(hFocusWnd, UserCharacterNewName);
					SendMessage(hFocusWnd, EM_SETSEL, lstrlen(UserCharacterNewName), lstrlen(UserCharacterNewName)+1);
					break;
				}
			}
			else
			{
				if( (UserCharacterNewName[index] >= 0x1f && UserCharacterNewName[index] <= 0x2f) ||
					(UserCharacterNewName[index] >= 0x3a && UserCharacterNewName[index] <= 0x40) ||
					(UserCharacterNewName[index] == 0x5c ) ||
					(UserCharacterNewName[index] == 0x5e ) ||
					(UserCharacterNewName[index] == 0x5f ) ||
					(UserCharacterNewName[index] == 0x60 ) ||
					(UserCharacterNewName[index] == 0x09 ) ||
//					(UserCharacterNewName[index] >= 0x80 || UserCharacterNewName[index] >= 0xff)
					(UserCharacterNewName[index] >= 0x7b && UserCharacterNewName[index] <= 0xff) ) //0x7f) )
				{
					UserCharacterNewName[index] = 0;
					SetWindowText(hFocusWnd, 0);
					SetWindowText(hFocusWnd, UserCharacterNewName);
					SendMessage(hFocusWnd, EM_SETSEL, lstrlen(UserCharacterNewName), lstrlen(UserCharacterNewName)+1);
					break;
				}
			}
#else
#ifdef _LANGUAGE_ARGENTINA
				if(
					(UserCharacterNewName[index] >= 0x41 && UserCharacterNewName[index] <= 0x5A ) ||	// Alphabet A to Z
					(UserCharacterNewName[index] >= 0x61 && UserCharacterNewName[index] <= 0x7A ) ||	// Alphabet a to z
		#ifdef	_WINMODE_DEBUG
					(UserCharacterNewName[index] == '[') ||	//193) || A
					(UserCharacterNewName[index] == ']') ||	//193) || A
		#endif
					(UserCharacterNewName[index] == 0xC1) ||	//193) || A
					(UserCharacterNewName[index] == 0xC9) ||	//201) || E
					(UserCharacterNewName[index] == 0xCD) ||	//205) || I
					(UserCharacterNewName[index] == 0xD1) ||	//209) || N
					(UserCharacterNewName[index] == 0xD3) ||	//211) || O
					(UserCharacterNewName[index] == 0xDA) ||	//218) || U
					(UserCharacterNewName[index] == 0xDC) ||	//220) || U
					(UserCharacterNewName[index] == 0xE1) ||	//225) || a
					(UserCharacterNewName[index] == 0xE9) ||	//233) || e
					(UserCharacterNewName[index] == 0xED) ||	//237) || i
					(UserCharacterNewName[index] == 0xF1) ||	//241) || n
					(UserCharacterNewName[index] == 0xF3) ||	//243) || o
					(UserCharacterNewName[index] == 0xFA) ||	//250) || u
					(UserCharacterNewName[index] == 0xFC)    )	//252	) u
				{
				}
				else
				{
					UserCharacterNewName[index] = 0;
					SetWindowText(hFocusWnd, 0);
					SetWindowText(hFocusWnd, UserCharacterNewName);
					SendMessage(hFocusWnd, EM_SETSEL, lstrlen(UserCharacterNewName), lstrlen(UserCharacterNewName)+1);
				}
		#ifndef	_WINMODE_DEBUG
				if(
					(UserCharacterNewName[index] == 'G' && UserCharacterNewName[index+1] == 'M') ||
					(UserCharacterNewName[index] == '[') ||	// [
					(UserCharacterNewName[index] == ']') 	// ]
				  )
				{
					UserCharacterNewName[index] = 0;
					SetWindowText(hFocusWnd, 0);
					SetWindowText(hFocusWnd, UserCharacterNewName);
					SendMessage(hFocusWnd, EM_SETSEL, lstrlen(UserCharacterNewName), lstrlen(UserCharacterNewName)+1);
				}
		#endif
#else
/*extern BYTE  VRKeyBuff[256];    //키입력 버퍼 
			if (VRKeyBuff[VK_CONTROL] || VRKeyBuff[VK_SHIFT])
			{
				if(VRKeyBuff[v] || VRKeyBuff[VK_INSERT])
				{
					UserCharacterNewName[index] = 0;
					SetWindowText(hFocusWnd, 0);
					SetWindowText(hFocusWnd, UserCharacterNewName);
				}
			}
*/
			
			if(     (UserCharacterNewName[index] >= 0x1f && UserCharacterNewName[index] <= 0x2f) ||
					(UserCharacterNewName[index] >= 0x3a && UserCharacterNewName[index] <= 0x40) ||
					(UserCharacterNewName[index] == 0x5c ) ||
					(UserCharacterNewName[index] == 0x5e ) ||
					(UserCharacterNewName[index] == 0x5f ) ||
					(UserCharacterNewName[index] == 0x60 ) ||
					//캐릭 이름에 탭키 막기
					(UserCharacterNewName[index] == 0x09 ) ||
					(UserCharacterNewName[index] >= 0x0e && UserCharacterNewName[index] <= 0x0f ) ||
					(UserCharacterNewName[index] == 0x16 ) ||
					(UserCharacterNewName[index] == 0x20 ) ||
					(UserCharacterNewName[index] == 0xff ) ||
					(UserCharacterNewName[index] >= 0x7b && UserCharacterNewName[index] <= 0x7f) )
			{
				UserCharacterNewName[index] = 0;
				SetWindowText(hFocusWnd, 0);
				SetWindowText(hFocusWnd, UserCharacterNewName);
				SendMessage(hFocusWnd, EM_SETSEL, lstrlen(UserCharacterNewName), lstrlen(UserCharacterNewName)+1);
				break;
			}
#endif
#endif
#endif
#endif
		}
		
		if(lstrlen(UserCharacterNewName) >= 1)
		{
			for(index = 0; index < (int)lstrlen(UserCharacterNewName); index++)
			{
#ifdef _LANGUAGE_BRAZIL
				if( (unsigned char)UserCharacterNewName[index] == 255 || (unsigned char)UserCharacterNewName[index] == 160)
				{
					UserCharacterNewName[index] = 0;
					SetWindowText(hFocusWnd, 0);
					SetWindowText(hFocusWnd, UserCharacterNewName);
					SendMessage(hFocusWnd, EM_SETSEL, lstrlen(UserCharacterNewName), lstrlen(UserCharacterNewName)+1);
				}
#endif


				//한글일 경우..
				if((BYTE)UserCharacterNewName[index] >= 0x80)
				{
/*
#ifdef _LANGUAGE_ARGENTINA
				if( ((BYTE)UserCharacterNewName[index] == 0xC1) ||	//193) || A
					((BYTE)UserCharacterNewName[index] == 0xC9) ||	//201) || E
					((BYTE)UserCharacterNewName[index] == 0xCD) ||	//205) || I
					((BYTE)UserCharacterNewName[index] == 0xD1) ||	//209) || N
					((BYTE)UserCharacterNewName[index] == 0xD3) ||	//211) || O
					((BYTE)UserCharacterNewName[index] == 0xDA) ||	//218) || U
					((BYTE)UserCharacterNewName[index] == 0xDC) ||	//220) || U
					((BYTE)UserCharacterNewName[index] == 0xE1) ||	//225) || a
					((BYTE)UserCharacterNewName[index] == 0xE9) ||	//233) || e
					((BYTE)UserCharacterNewName[index] == 0xED) ||	//237) || i
					((BYTE)UserCharacterNewName[index] == 0xF1) ||	//241) || n
					((BYTE)UserCharacterNewName[index] == 0xF3) ||	//243) || o
					((BYTE)UserCharacterNewName[index] == 0xFA) ||	//250) || u
					((BYTE)UserCharacterNewName[index] == 0xFC)    )//252	) u
				{
				}
				else
				{
					UserCharacterNewName[index] = 0;
					SetWindowText(hFocusWnd, 0);
					SetWindowText(hFocusWnd, UserCharacterNewName);
					SendMessage(hFocusWnd, EM_SETSEL, lstrlen(UserCharacterNewName), lstrlen(UserCharacterNewName)+1);
				}
#endif
*/

#ifdef _LANGUAGE_CHINESE

					//중국어 필터 ( GB2312 )
					int fstByte = (BYTE)UserCharacterNewName[index];
					int sndByte = (BYTE)UserCharacterNewName[index+1]; 

					if ( fstByte<0xA1 || fstByte>0xF7 || sndByte<0xA1 || sndByte>0xFE ) //2바이트인데 중국어코드 아님
					{
						UserCharacterNewName[index] = 0;
						SetWindowText(hFocusWnd, 0);
						SetWindowText(hFocusWnd, UserCharacterNewName);
						SendMessage(hFocusWnd, EM_SETSEL, lstrlen(UserCharacterNewName), lstrlen(UserCharacterNewName)+1);
						break;
					}
					else
						index++;

#endif

#ifdef _LANGUAGE_KOREAN
					//한글 필터
					int fstByte = (BYTE)UserCharacterNewName[index]; 
					int sndByte = (BYTE)UserCharacterNewName[index+1]; 

					result = (fstByte-0xb0)*94 + sndByte-0xa1; 

					if (result < 0 || result >= 2350) //2바이트인데 한글아님.
					{
						UserCharacterNewName[index] = 0;
						SetWindowText(hFocusWnd, 0);
						SetWindowText(hFocusWnd, UserCharacterNewName);
						SendMessage(hFocusWnd, EM_SETSEL, lstrlen(UserCharacterNewName), lstrlen(UserCharacterNewName)+1);
						break;
					}
					else
						index++;
#endif

				}
			}
		}

#ifdef _LANGUAGE_JAPANESE
		//IsJTS ( 일본 문자 )
		result = IsJTS(UserCharacterNewName);
		if ( result>=0 ) {
			UserCharacterNewName[result] = 0;

			SetWindowText(hFocusWnd, 0);
			SetWindowText(hFocusWnd, UserCharacterNewName);
			SendMessage(hFocusWnd, EM_SETSEL, lstrlen(UserCharacterNewName), lstrlen(UserCharacterNewName)+1);
		}

#endif
//영어 관련 처리를 넣어야 한다.
#ifdef _LANGUAGE_ENGLISH
		result = IsENG(UserCharacterNewName);
		if (result>=0)
		{
			UserCharacterNewName[result] = 0;

			SetWindowText(hFocusWnd, 0);
			SetWindowText(hFocusWnd, UserCharacterNewName);
			SendMessage(hFocusWnd, EM_SETSEL, lstrlen(UserCharacterNewName), lstrlen(UserCharacterNewName)+1);
		}
		/*
		else 
		{
			SetWindowText(hFocusWnd,"");
			ZeroMemory(UserCharacterNewName,sizeof(char)*256);
		}*/
#endif
//태국어 관련 처리를 넣어야 한다.
#ifdef _LANGUAGE_THAI
		result = IsTHAI(UserCharacterNewName);
		if ((result>=0)&&(IsCurse(UserCharacterNewName)==0))
		{
			UserCharacterNewName[result] = 0;

			SetWindowText(hFocusWnd, 0);
			SetWindowText(hFocusWnd, UserCharacterNewName);
			SendMessage(hFocusWnd, EM_SETSEL, lstrlen(UserCharacterNewName), lstrlen(UserCharacterNewName)+1);  
		}
		else if(IsCurse(UserCharacterNewName)!=0)
		{
			SetWindowText(hFocusWnd,"");
			ZeroMemory(UserCharacterNewName,sizeof(char)*256);
		}
#endif
#ifdef _LANGUAGE_TAIWAN
		//자유중국 문자 ( 번체 )
		result = IsTaiwan(UserCharacterNewName);
		if ( result>=0 ) {
			UserCharacterNewName[result] = 0;

			SetWindowText(hFocusWnd, 0);
			SetWindowText(hFocusWnd, UserCharacterNewName);
			SendMessage(hFocusWnd, EM_SETSEL, lstrlen(UserCharacterNewName), lstrlen(UserCharacterNewName)+1);
		}
#endif

		SendMessage(hFocusWnd, EM_GETSEL, (WPARAM)&start, NULL);
		HDC hdc;
		lpDDSBack->GetDC( &hdc );

		SetBkMode( hdc, TRANSPARENT );
		SetTextColor( hdc, RGB(255, 255, 255) );
		SelectObject( hdc , Font );

		
		static int i = 0;
		if( i < 6)
		{
#ifdef	_LANGUAGE_VEITNAM
			int newTexPosX  = int(((float)smScreenWidth*356.f)/800.f+13);
#else
			int newTexPosX  = int(((float)smScreenWidth*356.f)/800.f);
#endif
			int newTexPosY  = int(((float)smScreenHeight*397.f)/600.f);

			dsTextLineOut(hdc, newTexPosX+start*6, newTexPosY, "|", 1);
		}

		i++;
		if(i > 12)
			i = 0;
		//dsTextLineOut(hdc, 358, 398, UserCharacterNewName, strlen(UserCharacterNewName));
		lpDDSBack->ReleaseDC( hdc );
	}
	return TRUE;
}

//그리기 함수 캐릭터 선택
int HoLogin::Draw()
{
	MainFade();
	
	//에러 메시지 출력시.. 없애는 경우...
	if(ErrorSelect != ERROR_NONE)
	{
		if(MouseButton[0] || MouseDblClick)
		{
			if(ErrorSelect == ERROR_NONAME_NEWPLAYER)
			{
				hFocusWnd = hTextWnd;
				FocusWndFlag = TRUE;
			}
			ErrorSelect = ERROR_NONE;
			MouseButton[0] = FALSE;
			MouseDblClick = FALSE;
#ifdef	_LANGUAGE_JAPANESE		//C7
				SetIME_Mode( 1 );
#endif

			StartErrorDelayFlag = FALSE;
		}
		
		for(int index = 0; index < 256; index++)
		{
			if(VRKeyBuff[index] == TRUE)
			{
				if(ErrorSelect == ERROR_NONAME_NEWPLAYER)
				{
					hFocusWnd = hTextWnd;
					FocusWndFlag = TRUE;
#ifdef	_LANGUAGE_JAPANESE		//C7
				SetIME_Mode( 1 );
#endif
				}
				
				ErrorSelect = ERROR_NONE;
				StartErrorDelayFlag = FALSE;
				break;
			}
		}
	}
	
	DrawSelectSubState();
	//카메라 각도 계산...
	int x,y,z;
	int anx,any;
	//int cnt;

	anx = CaAngle.x;
	any = CaAngle.y;

	any &=ANGCLIP;
	anx &=ANGCLIP;

	GetMoveLocation( 0 , 0, -TarDist, anx, any, 0 );
	x = TarPosi.x + GeoResult_X;
	y = TarPosi.y + GeoResult_Y;
	z = TarPosi.z + GeoResult_Z;
	
	
	//######################################################################################
	//작 성 자 : 오 영 석
	if( GetFilterEffect() == FILTER_EFFECT_SET_BRIGHT_CONTRAST )
	{
		BeginRender();
        ChangeRenderTarget( NEW_TARGET_BACK );
	}
	//######################################################################################
	

	BeginRender();
	smRender.SetCameraPosi( x,y,z, anx , any , 0  );
	
	
	//등불 아래 광원..
	//ktj : 일단 삭제함. : 로긴화면바뀌면서 필요없게됨.
	//DrawLight();
	
	//Stage를 찍는다..
	if(Stage)
		Stage->DrawStage( x,y,z, anx , any , 0 );
	
	//Character를 찍는다.
	DrawUserCharacter(x, y, z, anx, any, 0);
	DrawJobCharacter(x, y, z, anx, any, 0);
	
	
	DrawEffect(x, y, z, anx, any, 0);

	
	//######################################################################################
	//작 성 자 : 오 영 석
	if( GetFilterEffect() == FILTER_EFFECT_SET_BRIGHT_CONTRAST && GetRenderTarget() == NEW_TARGET_BACK )
	{
		SetNewTargetTextureState();
		lpD3DDevice->BeginScene();
		RenderFilterEffect();
		lpD3DDevice->EndScene();
		DefaultsNewTargetTextureState();
	}
	//######################################################################################
	

	lpD3DDevice->BeginScene();
	
	//Game 틀을 찍는다..
	DrawGameFrame();
	DrawTitle(SelectCurrentState);
	
	int twoButtonStartPos   = m_ScreenHalf-120;
	int threeButtonStartPos = m_ScreenHalf-150;
	int screenWidthHalf  = smScreenWidth/2;
	int screenHeightHalf = smScreenHeight/2; 
	//상태 이동이 완전히 됐을 경우. (BeginRender(), EndRender() 사이에 들어가는 루틴)
	if(SelectCurrentState == SelectNextState && FadeCurrentState == FadeNextState)
	{
		MainDefaultButton();

		//버튼 출력...
		if(SelectCurrentState == CHARACTER_SELECT_STATE) //Character Select State
		{
          	int userCount = 0;
			for(int index = 0; index < DownUserCharacterCount; index++)
			{
				if(UserCharacter[index].DisplayFlag)
					userCount++;
			}

			if( pCursorPos.x > (screenWidthHalf-128) && 
				pCursorPos.x < (screenWidthHalf+128) && 
 				pCursorPos.y > (screenHeightHalf/2-80)&& 
				pCursorPos.y < (screenHeightHalf/2+130)&& 
				SelectSubState == SUB_SELECT_NONE && ErrorSelect == ERROR_NONE)
			{
				//ktj : 위의 동그란그림에 마우스가 들어가면 //캐릭터만들기 라는 이미지가 뜨는곳임.
  				dsDrawTexImage( NewCharacterImage, (m_ScreenHalf)-128, 150, 256, 64, 255);
		
				if(MouseButton[0]) //New Character 선택시..
				{
					//캐릭터 갯수 보다 많이 캐릭터를 만들 경우...
					if(UserCharacterCount < CHARACTER_USER_MAX)
					{
						SelectNextState = RACE_SELECT_STATE;
						PlayWav2(5-1);
					}
					else
					{
						MouseButton[0] = FALSE;
						MouseDblClick = FALSE;
						ErrorSelect = ERROR_MAX_NEWPLAYER;
					}
				}
			}
			else
			{
				if(userCount == 0)
				{
					
 					dsDrawTexImage( NewCharacterImage, (m_ScreenHalf)-128, 150, 256, 64, (int)NewCharacterImageBlendCount);
				}
			}
			
          	if(MouseDownCharacterNum != -1)
				DeleteButton.Draw(0, smScreenHeight-40);
			
			if(userCount == 0)
 				NewCharButton.Draw(twoButtonStartPos, smScreenHeight-40);
			else
				OkButton.Draw(twoButtonStartPos, smScreenHeight-40);
 			QuitButton.Draw(twoButtonStartPos+110, smScreenHeight-40);
		}
		else if(SelectCurrentState == RACE_SELECT_STATE) //Race Select State
		{
			OkButton.Draw(threeButtonStartPos,       smScreenHeight-40);
			BackButton.Draw(threeButtonStartPos+100, smScreenHeight-40);
			QuitButton.Draw(threeButtonStartPos+200, smScreenHeight-40);	
			
			if(USE_MOR==0) //템스크론만 선택시사용됨.
			{
				//종족 마크를 화면에 표시
				if(menuPos[0].left < pCursorPos.x  && pCursorPos.x < menuPos[0].right && 
				   menuPos[0].top < pCursorPos.y && pCursorPos.y < menuPos[0].bottom 
				   && RaceMarkBlendCount>=200 && ErrorSelect == ERROR_NONE)
				{
					//ktj : 바꿈.
					dsDrawTexImage( TempscronLogoImage, smScreenWidth/2-128, smScreenHeight/4, 128, 64, 255);
					dsDrawTexImage( TempscronMarkImage, smScreenWidth/2,     smScreenHeight/4, 128, 128, 255);
					if(MouseButton[0])
					{
						PlayWav2(5-1);
						SelectNextState = JOB_SELECT_STATE;
						RaceMarkScaleSize = 128;
						SetupJobCharacters( TRIBE_Tempscron );
					}
				}
				else
				{
					dsDrawTexImage( TempscronLogoImage, smScreenWidth/2-128, smScreenHeight/4, 128, 64, RaceMarkBlendCount);
					dsDrawTexImage( TempscronMarkImage, smScreenWidth/2,     smScreenHeight/4, 128, 128, RaceMarkBlendCount);
				}
			}
			else 
			{
			    if(pCursorPos.x > (smScreenWidth/2-140) &&
				   pCursorPos.x < (smScreenWidth/2-140+130) && 
			       pCursorPos.y > (smScreenHeight/5) && 
			       pCursorPos.y < (smScreenHeight/5+130) 
 			       && RaceMarkBlendCount>=200 && ErrorSelect == ERROR_NONE)
			    {
					//ktj : 바꿈.
					//마우스가 올라 갔을 경우..
         			dsDrawTexImage( TempscronMarkImage, smScreenWidth/2-140,   smScreenHeight/5,     128, 128, 255);
					dsDrawTexImage( TempscronLogoImage, smScreenWidth/2-140-6, smScreenHeight/5+118, 128, 32,  255);
	 				
         			dsDrawTexImage( RaceTextBoxImage[0], smScreenWidth/2-140-175,     smScreenHeight/5+150, 256, 256, 255);
  					dsDrawTexImage( RaceTextBoxImage[1], smScreenWidth/2-140+256-175, smScreenHeight/5+150, 32,  256, 255);
					dsDrawTexImage( TempscronTextImage,  smScreenWidth/2-140-175+19,  smScreenHeight/5+150+12, 256, 128, 255);


 					if(MouseButton[0])
					{
						PlayWav2(5-1);
						SelectNextState = JOB_SELECT_STATE;
						RaceMarkScaleSize = 128;
						SetupJobCharacters( TRIBE_Tempscron );		// with the Assassin and the Martial Artist
					}
				}
				else
				{
    				dsDrawTexImage( TempscronMarkImage, smScreenWidth/2-140,   smScreenHeight/5,     128, 128, RaceMarkBlendCount);
					dsDrawTexImage( TempscronLogoImage, smScreenWidth/2-140-6, smScreenHeight/5+118, 128, 32,  RaceMarkBlendCount);
 				}
				//ktj : 새로넣은 모라이온 ================================= start
				if((smScreenWidth/2+12) < pCursorPos.x  && pCursorPos.x < (smScreenWidth/2+12+130) && 
				(smScreenHeight/5) < pCursorPos.y  && pCursorPos.y < (smScreenHeight/5+130) 
         				&& RaceMarkBlendCount>=200 && ErrorSelect == ERROR_NONE)
				{
					//ktj : 모라이온 마크
 					dsDrawTexImage( MoryonMarkImage, smScreenWidth/2+12,   smScreenHeight/5,     128, 128, 255);
					dsDrawTexImage( MoryonLogoImage, smScreenWidth/2+12-6, smScreenHeight/5+118, 128, 32,  255);

 					dsDrawTexImage( RaceTextBoxImage[0], smScreenWidth/2+12, smScreenHeight/5+150, 256, 256, 255);
 					dsDrawTexImage( RaceTextBoxImage[1], smScreenWidth/2+12+256,  smScreenHeight/5+150, 32, 256, 255);
					dsDrawTexImage( MoryonTextImage, smScreenWidth/2+12+19,smScreenHeight/5+150+12, 256, 256, 255);
				
					if(MouseButton[0])
					{
						PlayWav2(5-1);
						SelectNextState = JOB_SELECT_STATE;
						RaceMarkScaleSize = 128;
						SetupJobCharacters( TRIBE_Moryon );		// with the Shaman
					}
				}
				else
				{
					dsDrawTexImage( MoryonMarkImage, smScreenWidth/2+12,   smScreenHeight/5,     128, 128, RaceMarkBlendCount);
					dsDrawTexImage( MoryonLogoImage, smScreenWidth/2+12-6, smScreenHeight/5+118, 128, 32,  RaceMarkBlendCount);
				}

			//ktj : 새로넣은 모라이온 ================================= end
			}
		}
		else if(SelectCurrentState == JOB_SELECT_STATE) //Job Select State
		{
  			OkButton.Draw(threeButtonStartPos,       smScreenHeight-40);
			BackButton.Draw(threeButtonStartPos+100, smScreenHeight-40);
			QuitButton.Draw(threeButtonStartPos+200, smScreenHeight-40);	

			//임시로 캐릭터의 위치를 2d로 찾기 고쳐야 함...
			RECT rect;
			POINT	sPos;
			smRender.SetCameraPosi( x,y,z, anx , any , 0  );
			
			for(int index = 0; index < JobCharacterCount; index++)
			{
				smRender.GetRect2D( JobCharacter[index].pX , JobCharacter[index].pY+(32*fONE) , JobCharacter[index].pZ ,
 						32*fONE , 32*fONE , &rect , &sPos );
			
				
#ifdef NOTUSE_MOR 
				dsDrawTexImage( TempscronJobLogoImage[index], rect.left+30, rect.top-10, 64, 32, 255);

#else
   				

				//ktj : 종족추가에 따라 새로 넣음. (각캐릭터의 이름이 써진 2d 그림임)
    				switch(tribeNum){
				case TRIBE_Tempscron:
  					if(MouseMoveCharacterNum == index || MouseDownCharacterNum == index) 
					{
    					dsDrawTexImage( TempscronJobImage[index][1], rect.left-10, rect.top-15, 128, 32, 255);
 						
   						if(MouseDownCharacterNum != index)
						{
 							//캐릭터 정보 출력
							dsDrawTexImage( JobTextBoxImage[0], smScreenWidth/2-140,     smScreenHeight/5, 256, 256, 255);
 							dsDrawTexImage( JobTextBoxImage[1], smScreenWidth/2-140+256, smScreenHeight/5, 32, 256, 255);
							dsDrawTexImage( TempscronJobTextImage[index], smScreenWidth/2-140+10, smScreenHeight/5+4, 256, 256, 255);
						}
     					
					}
					else
						dsDrawTexImage( TempscronJobImage[index][0], rect.left-10, rect.top-15, 128, 32, 255);
					break;
				case TRIBE_Moryon:
					if(MouseMoveCharacterNum == index || MouseDownCharacterNum == index)
					{
						dsDrawTexImage( MoryonJobImage[index][1], rect.left-10, rect.top-15, 128, 32, 255);
						if(MouseDownCharacterNum != index)
						{
							dsDrawTexImage( JobTextBoxImage[0], smScreenWidth/2-140,     smScreenHeight/5, 256, 256, 255);
 							dsDrawTexImage( JobTextBoxImage[1], smScreenWidth/2-140+256, smScreenHeight/5, 32, 256, 255);

		 					dsDrawTexImage( MoryonJobTextImage[index], smScreenWidth/2-140+10, smScreenHeight/5+4, 256, 256, 255);
						}
					}
					else
						dsDrawTexImage( MoryonJobImage[index][0], rect.left-10, rect.top-15, 128, 32, 255);
					break;

				}
#endif

			}
		}
		else if(SelectCurrentState == FACE_SELECT_STATE)
		{
  			OkButton.Draw(threeButtonStartPos,   smScreenHeight-40);
			BackButton.Draw(threeButtonStartPos+100, smScreenHeight-40);
			QuitButton.Draw(threeButtonStartPos+200, smScreenHeight-40);	

			dsDrawTexImage( BlackBoxImage, 150, 410, 500, 128+10, 20);
		}
		
	}
	
	//상태 이동중..
	//RaceSelect -> JobSelect 로 이동중...(마크 서서히 사라짐)
 	if( (SelectCurrentState == RACE_SELECT_STATE) && (SelectNextState == JOB_SELECT_STATE))
	{
		if(tribeNum == TRIBE_Tempscron)
		dsDrawTexImage( TempscronMarkImage, smScreenWidth/2-RaceMarkScaleSize/2, smScreenHeight/3-RaceMarkScaleSize/2-40, RaceMarkScaleSize, RaceMarkScaleSize, RaceMarkScaleSize);
#ifndef NOTUSE_MOR
		if(tribeNum == TRIBE_Moryon)
		dsDrawTexImage( MoryonMarkImage, smScreenWidth/2-RaceMarkScaleSize/2, smScreenHeight/3-RaceMarkScaleSize/2-40, RaceMarkScaleSize, RaceMarkScaleSize, RaceMarkScaleSize);
#endif

	}
	lpD3DDevice->EndScene();
	EndRender();

	IncTextureFrame();
	
	//상태 이동이 완전히 됐을 경우. BeginRender,EndRender 사이에 루틴을 쓰지 못하는 경우(예:GDI(Message, MessageBox))
	if(SelectCurrentState == SelectNextState && FadeCurrentState == FadeNextState)
	{
		//캐릭터 선택일 경우..
		if(SelectCurrentState == CHARACTER_SELECT_STATE)
		{
			//User의 캐릭터 정보 출력..
			if(MouseMoveCharacterNum != -1)
				DrawCharacterInfo();

			if(SelectSubState == SUB_SELECT_CHARACTER_MANAGE)
				//MessageBox.Draw(290, 450, "캐릭터를 삭제 하시겠습니까?", RGB(255, 0, 0));
				MessageBox.Draw(smScreenWidth/2-128, smScreenHeight/2+smScreenHeight/6, HoTextSelectMessage[0], RGB(255, 0, 0));
			if(SelectSubState == SUB_SELECT_CHARACTER_USER)
				//MessageBox.Draw(290, 450, "게임을 시작합니다");
				MessageBox.Draw(smScreenWidth/2-128, smScreenHeight/2+smScreenHeight/6, HoTextSelectMessage[1]);
			if(ErrorSelect == ERROR_MAX_NEWPLAYER)
				//Message.Draw(295, 450,    "캐릭터를 더이상 생성 못합니다.");
				Message.Draw(smScreenWidth/2-128, smScreenHeight/2+smScreenHeight/6,    HoTextSelectMessage[2]);
		}
		else if(SelectCurrentState == RACE_SELECT_STATE && ErrorSelect == ERROR_NONE)
		{
		}
		else if(SelectCurrentState == JOB_SELECT_STATE && ErrorSelect == ERROR_NONE)
		{
		}
		else if(SelectCurrentState == FACE_SELECT_STATE)
		{
 			if(FaceSelectFirstFlag == FALSE && JobFaceSelectNum != -1)
			{
				FaceSelectFirstFlag = TRUE;
				SetChangeJobFace( LoginBodyName( tribeNum, JobBodySelectNum ), LoginFaceName( tribeNum, JobBodySelectNum, JobFaceSelectNum ), JobBodySelectNum );
				JobCharacter[JobBodySelectNum].Flag = 1;
				OpenEachPlayer(&JobCharacter[JobBodySelectNum]);
			}
			int addX = smScreenWidth/2-800/2;

   			
			int newPosX   = int(((float)smScreenWidth*272.f)/800.f);
			int newPosY   = int(((float)smScreenHeight*82.f)/600.f);
			if(smScreenWidth == 1280 && smScreenHeight == 1024)
				newPosY   = int( (float)smScreenHeight* (100.f /600.f)  );
			int newWidth  = int(((float)smScreenWidth*256.f)/800.f);
			int newHeight = int(((float)smScreenHeight*128.f)/600.f);
			
   			//int newPosX   = int( (float)smScreenWidth * (272.f/800.f) );
			//int newPosY   = int( (float)smScreenHeight* (82.f /600.f)  );
  			//if(smScreenWidth == 1280 && smScreenHeight == 1024)
				//newPosY   = int( (float)smScreenHeight* (100.f /600.f)  );

			//int newWidth  = int( (float)smScreenWidth * (256.f/800.f)  );
 			//int newHeight = int( (float)smScreenHeight* (128.f/600.f) );

			int addY = smScreenHeight/2-600/2;
			//확대 화면 찍기..
   			lpD3DDevice->BeginScene();
			//dsDrawTexImage(FaceBoxBigImage[0], 272+addX, 82+addY,     256, 256, 255);
            //dsDrawTexImage(FaceBoxBigImage[1], 272+addX, 82+256+addY, 256, 128, 255);
    		dsDrawTexImage(FaceBoxBigImage[0], newPosX, newPosY,     newWidth, newWidth, 255);
            dsDrawTexImage(FaceBoxBigImage[1], newPosX, newPosY+newWidth, newWidth, newHeight, 255);

			lpD3DDevice->EndScene();

			DrawEachPlayer(0.5f, 0.4f, 1);
			
			int    mouseMoveFace = -1;
			bool   selectFaceFlag = false;
			
			//얼굴을 선택한다.	
			if( JobBodySelectNum >= 0)
			{
 				lpD3DDevice->BeginScene();
				for(int index = 0; index < TempFaceCount[JobBodySelectNum]; index++)
				{
					if( pCursorPos.x > (TempFacePos[JobBodySelectNum][index]+addX) &&
						pCursorPos.x < (TempFacePos[JobBodySelectNum][index]+128+addX) &&
						pCursorPos.y > (smScreenHeight-(70+128)) &&
						pCursorPos.y < (smScreenHeight-(70+128)+128) && ErrorSelect == ERROR_NONE && SelectSubState == SUB_SELECT_NONE)
					{
						
						mouseMoveFace = index;
						if(MouseButton[0])//마우스 버튼을 클릭 했을 경우..
						{	
							selectFaceFlag = true;
							if(JobFaceSelectNum != index)
							{

#ifdef NOTUSE_MOR 
								//ktj : 원본
								if(JobBodySelectNum == 0) //Mech
									SetChangeJobFace(szTempMechBodyName[0], szTempMechFaceName[0][index], JobBodySelectNum);
								else if(JobBodySelectNum == 1) // Fighter
									SetChangeJobFace(szTempFighterBodyName[0], szTempFighterFaceName[0][index], JobBodySelectNum);
								else if(JobBodySelectNum == 2) // Pike
									SetChangeJobFace(szTempPikeBodyName[0], szTempPikeFaceName[0][index], JobBodySelectNum);
								else if(JobBodySelectNum == 3) // Archer
									SetChangeJobFace(szTempArcherBodyName[0], szTempArcherFaceName[0][index], JobBodySelectNum);
								

#else



								
							//ktj : 종족추가에 따라 새로 넣음. (각캐릭터의 이름이 써진 그림임)
							SetChangeJobFace( LoginBodyName( tribeNum, JobBodySelectNum ), LoginFaceName( tribeNum, JobBodySelectNum, index ), JobBodySelectNum );
							
#endif


								JobFaceSelectNum = index;

								JobCharacter[JobBodySelectNum].Flag = 1;
								OpenEachPlayer(&JobCharacter[JobBodySelectNum]);
							}
							if(JobBodySelectNum != -1)
							{
								if(SelectSubState == SUB_SELECT_NONE)
								{
									hFocusWnd    = hTextWnd;
									FocusWndFlag = TRUE;
#ifdef	_LANGUAGE_JAPANESE		//C7
				SetIME_Mode( 1 );
#endif
								}
							}
						}
					}
					
					
 					//선택된 얼굴 표시..
 					if(JobFaceSelectNum == index)
					{
    					
 						dsDrawTexImage( FaceBoxSmallImage[1], TempFacePos[JobBodySelectNum][JobFaceSelectNum]+addX,smScreenHeight-(50+128), 128, 128, 255);
#ifdef NOTUSE_MOR 
						dsDrawTexImage( TempFaceImage[JobBodySelectNum][JobFaceSelectNum][1], TempFacePos[JobBodySelectNum][JobFaceSelectNum]+addX,smScreenHeight-(50+128), 128, 128, 255);
#else
						//ktj : 얼굴그림 새로 넣음.
 						switch(tribeNum){
						case TRIBE_Tempscron:	
  						 	dsDrawTexImage( TempFaceImage[JobBodySelectNum][JobFaceSelectNum][1], TempFacePos[JobBodySelectNum][JobFaceSelectNum]+addX,smScreenHeight-(50+128)+4, 128, 128, 255);
									break;
						case TRIBE_Moryon:
							dsDrawTexImage( MorFaceImage[JobBodySelectNum][JobFaceSelectNum][1], TempFacePos[JobBodySelectNum][JobFaceSelectNum]+addX,smScreenHeight-(50+128), 128, 128, 255);
							break;
						}

#endif
					}
					else //얼굴이 선택되지 않았을 때..
					{
						if(mouseMoveFace != -1 && mouseMoveFace == index)
						{
							//dsDrawTexImage( FaceBoxImage, TempFacePos[JobBodySelectNum][index],420, 128, 128, 200);
     						dsDrawTexImage( FaceBoxSmallImage[1], TempFacePos[JobBodySelectNum][index]+addX,smScreenHeight-(50+128), 128, 128, 180);


#ifdef NOTUSE_MOR 
							dsDrawTexImage( TempFaceImage[JobBodySelectNum][index][1], TempFacePos[JobBodySelectNum][index]+addX,smScreenHeight-(50+128), 128, 128, 200);
#else
	 						//ktj : 얼굴그림 새로 넣음.
 							switch(tribeNum){
							case TRIBE_Tempscron:	
								dsDrawTexImage( TempFaceImage[JobBodySelectNum][index][0], TempFacePos[JobBodySelectNum][index]+addX,smScreenHeight-(50+128)+4, 128, 128, 200);
									break;
							case TRIBE_Moryon:
								dsDrawTexImage( MorFaceImage[JobBodySelectNum][index][0], TempFacePos[JobBodySelectNum][index]+addX,smScreenHeight-(50+128), 128, 128, 200);
								break;
							}
#endif

							
						}
						else
						{
							//dsDrawTexImage( FaceBoxImage, TempFacePos[JobBodySelectNum][index],420, 128, 128, 130); 
     						dsDrawTexImage( FaceBoxSmallImage[0], TempFacePos[JobBodySelectNum][index]+addX,smScreenHeight-(50+128), 128, 128, 255);
							
#ifdef NOTUSE_MOR 
							dsDrawTexImage( TempFaceImage[JobBodySelectNum][index][0], TempFacePos[JobBodySelectNum][index]+addX,smScreenHeight-(50+128), 128, 128, 150);
#else
							//ktj : 얼굴그림 새로 넣음.
  							switch(tribeNum){
							case TRIBE_Tempscron:	
  								 dsDrawTexImage( TempFaceImage[JobBodySelectNum][index][0], TempFacePos[JobBodySelectNum][index]+addX,smScreenHeight-(50+128)+4, 128, 128, 150);
								break;
							case TRIBE_Moryon:
								dsDrawTexImage( MorFaceImage[JobBodySelectNum][index][0], TempFacePos[JobBodySelectNum][index]+addX,smScreenHeight-(50+128), 128, 128, 150);
								break;
							}

#endif
						}
					}
				}
				
                 dsDrawTexImage( FaceBoxSmallImage[2], 305+addX,smScreenHeight-(66), 64, 16, 255);
				dsDrawTexImage( FaceBoxSmallImage[2], 305+128+addX,smScreenHeight-(66), 64, 16, 255);
 
				lpD3DDevice->EndScene();
 			}
			
			
			if( selectFaceFlag == false && MouseButton[0] && SelectSubState == SUB_SELECT_NONE &&
				pCursorPos.x > 0 && pCursorPos.x < 800 && pCursorPos.y > 0 && pCursorPos.y < 546)
				JobFaceSelectNum = -1;
			
			if(JobBodySelectNum != -1 && JobFaceSelectNum != -1 && SelectSubState == SUB_SELECT_NONE &&
				ErrorSelect == ERROR_NONE)
			{
				//hFocusWnd가 NULL 이면 Enter Key가 눌린거 하구 같다.
				if(MouseDblClick || (hFocusWnd == NULL && FocusWndFlag))
				{
					FocusWndFlag = FALSE;
					hFocusWnd = NULL;
#ifdef	_LANGUAGE_JAPANESE		//C7
				SetIME_Mode( 0 );
#endif

					if(strlen(UserCharacterNewName) >0)
					{
						MessageBox.Flag = TRUE;
						SelectSubState = SUB_SELECT_CHARACTER_NEW;
					}
					else
					{
						//Mouse 이벤트 보정..
						MouseDblClick = FALSE;
						MouseButton[0] = FALSE;
						ErrorSelect = ERROR_NONAME_NEWPLAYER;	
					}
				}
			}
			
			HDC hdc;
 			lpDDSBack->GetDC( &hdc );
			SetBkMode( hdc, TRANSPARENT );
			SetTextColor( hdc, RGB(255, 255, 255) );
			SelectObject( hdc , Font );
#ifdef	_LANGUAGE_VEITNAM
 			int newTexPosX  = int(((float)smScreenWidth*358.f)/800.f+13);
#else
 			int newTexPosX  = int(((float)smScreenWidth*358.f)/800.f);
#endif
			int newTexPosY  = int(((float)smScreenHeight*398.f)/600.f);

			//dsTextLineOut(hdc, 358, 398, UserCharacterNewName, strlen(UserCharacterNewName));
    		dsTextLineOut(hdc, newTexPosX, newTexPosY, UserCharacterNewName, strlen(UserCharacterNewName));

			lpDDSBack->ReleaseDC( hdc );
			
			if(JobFaceSelectNum >= 0)
			{
//// KYLE ///// 일본어IME /////////////////////////////////////////////////////////////////////////////Begin//
				int nLength = strlen(UserCharacterNewName);
#ifdef _LANGUAGE_JAPANESE
#include <imm.h>
			
				HIMC hImc = ImmGetContext(hwnd);
				COMPOSITIONFORM	cf;

				if(hImc)
				{
					cf.dwStyle = CFS_FORCE_POSITION;

					if(smConfig.ScreenSize.x == 800)
					{
						cf.ptCurrentPos.x = (smConfig.ScreenSize.x/2 - 40) + nLength*7;
						cf.ptCurrentPos.y = (smConfig.ScreenSize.y/2) + 94;
					}
					if(smConfig.ScreenSize.x == 1024)
					{
						cf.ptCurrentPos.x = (smConfig.ScreenSize.x/2 - 52) + nLength*7;
						cf.ptCurrentPos.y = (smConfig.ScreenSize.y/2) + 120;
					}
					else if(smConfig.ScreenSize.x == 1280)
					{
						cf.ptCurrentPos.x = (smConfig.ScreenSize.x/2 - 70) + nLength*7;
						cf.ptCurrentPos.y = (smConfig.ScreenSize.y/2) + 166;
					}
					else //600 X 480인듯(GM모드)
					{
						cf.ptCurrentPos.x = (smConfig.ScreenSize.x/2 - 47) + nLength*7;
						cf.ptCurrentPos.y = (smConfig.ScreenSize.y/2) + 70;
					}
					ImmSetCompositionWindow(hImc, &cf);	//IME 포지션 저장
					ImePositon.x = cf.ptCurrentPos.x;
					ImePositon.y = cf.ptCurrentPos.y;
					ImeChatLen = nLength;
				}
#endif
			}	//if(JobFaceSelectNum >= 0) - was inside the #ifdef above, which broke non-Japanese builds
//// KYLE ///// 일본어IME /////////////////////////////////////////////////////////////////////////////End////
				KeyFilter();

//해외 (임시로 복사하는 키 방지했다.)/////////////////////////////////////////////////////////
			if ( (GetKeyState(VK_CONTROL) < 0) )
			{
				if((GetKeyState(118) < 0) || (GetKeyState(86) < 0))
				{
					for(int i=0; i<=255; i++)
					{
						UserCharacterNewName[i] = 0;
						SetWindowText(hFocusWnd, 0);
						SetWindowText(hFocusWnd, UserCharacterNewName);
					}
				}
			}
			if ( GetKeyState(VK_SHIFT) < 0 )
			{
				if ( GetKeyState(VK_INSERT) < 0 )
				{
					for(int i=0; i<=255; i++)
					{
						UserCharacterNewName[i] = 0;
						SetWindowText(hFocusWnd, 0);
						SetWindowText(hFocusWnd, UserCharacterNewName);
					}
				}
			}
////////////////////////////////////////////////////////////////////////////////////////////////
		}
	}
	
 
	if(SelectSubState == SUB_SELECT_CHARACTER_NEW)
		MessageBox.Draw(smScreenWidth/2-150, smScreenHeight/2+smScreenHeight/6, HoTextSelectMessage[3]);//"캐릭터 생성을 마쳤습니다 위 설정이 맞습니까?");
	if(ErrorSelect == ERROR_NONAME_NEWPLAYER)
		Message.Draw(smScreenWidth/2-100, smScreenHeight/2+smScreenHeight/6,    HoTextSelectMessage[4]);//"캐릭터의 이름을 입력하세요");
	if(ErrorSelect == ERROR_EXHIST_NEWPLAYER)
		Message.Draw(smScreenWidth/2-128, smScreenHeight/2+smScreenHeight/6,    HoTextSelectMessage[5]);//"캐릭터 이름이 존재합니다");
			
	if(ErrorSelect == ERROR_SELECT_CHAR)
		Message.Draw(smScreenWidth/2-100, smScreenHeight/2+smScreenHeight/6,    HoTextSelectMessage[6]);//"  캐릭터를 선택하세요  ");
	if(ErrorSelect == ERROR_SELECT_RACE)
		Message.Draw(smScreenWidth/2-100, smScreenHeight/2+smScreenHeight/6,    HoTextSelectMessage[7]);//" 종족마크를 선택하세요 ");
	if(ErrorSelect == ERROR_SELECT_JOB)
		Message.Draw(smScreenWidth/2-100, smScreenHeight/2+smScreenHeight/6,    HoTextSelectMessage[8]);//"  캐릭터를 선택하세요  ");
	if(ErrorSelect == ERROR_SELECT_FACE)
		Message.Draw(smScreenWidth/2-100, smScreenHeight/2+smScreenHeight/6,    HoTextSelectMessage[9]); //"캐릭터 얼굴을 선택하세요");
	if(ErrorSelect == ERROR_DELETE_CLAN_MEMBER)
 		Message.Draw(smScreenWidth/2-128, smScreenHeight/2+smScreenHeight/6,    HoTextSelectMessage[10]); //"클랜에 가입되어 있는 캐릭터는 삭제할수 없습니다
 	if(ErrorSelect == ERROR_CLAN_SERVER_NONE)
		Message.Draw(smScreenWidth/2-128, smScreenHeight/2+smScreenHeight/6,    HoTextSelectMessage[11]); //클랜 정보를 확인할수 없습니다

	if(MessageBox.Flag)
	{
     	lpD3DDevice->BeginScene();
 		dsDrawTexImage( BlackBoxImage, 0, smScreenHeight-47,  smScreenWidth, smScreenHeight, 120);
		lpD3DDevice->EndScene();
	}

	if( SelectCurrentState == COMPLETE_SELECT_STATE || 
		SelectCurrentState == START_GAME_STATE)
		DrawMap(MapBlendCount);
	DrawFade();
	
	
	return TRUE;
}




int HoLogin::DrawFade()
{
	if(FadeCurrentState == FADE_IN && FadeNextState == FADE_IN)
		return FALSE;

	bool fastBlendFlag = false;
	for(int index = 0; index < 256; index++)
	{
		if(VRKeyBuff[index] == TRUE)
		{
			fastBlendFlag = true;
			break;
		}
	}
	
	if(MouseButton[0])
	{
		fastBlendFlag = true;
		MouseButton[0] = FALSE;
	}
	
	if(fastBlendFlag)
	{
		if(FadeNextState == FADE_IN)
		{
			BlendValue = 0;
			FadeCurrentState = FADE_IN;
			LoginVolume = 350;
		}
		else if(FadeNextState == FADE_OUT)
		{
			BlendValue =  255;
			FadeCurrentState = FADE_OUT;
			LoginVolume = 0;
		}
	}

	if(SelectCurrentState != COMPLETE_SELECT_STATE && SelectCurrentState != START_GAME_STATE)
	{
		SetVolumeBGM(LoginVolume);
		PlayWav_VolChange(9-1, LoginVolume);
		PlayWav_VolChange(10-1, LoginVolume+50);
		PlayWav_VolChange(11-1, LoginVolume+50);
	}
	
	if(BlendValue > 0 && BlendValue <= 255)
	{
		lpD3DDevice->BeginScene();
		dsDrawTexImage(BlackBoxImage,0, 0, smScreenWidth, smScreenHeight, BlendValue);
		lpD3DDevice->EndScene();
	}

	return TRUE;
}

int HoLogin::MainFade()
{
	//FadeIn FadeOut 관련..
	if(FadeCurrentState != FadeNextState)
	{
		if(FadeNextState == FADE_IN)
			MainFadeIn();
		else
			MainFadeOut();
	}
	
	return TRUE;
}

int HoLogin::MainFadeIn()
{
	//나누는 250.0f는 단계다...
	if(BlendValue > 0)
	{
		BlendValue -= 6;
		if(SelectCurrentState != COMPLETE_SELECT_STATE &&
			SelectCurrentState != START_GAME_STATE)
		{
			LoginVolume += 10;
			if(LoginVolume > 350)
				LoginVolume = 350;
		}
	}
	
	if(BlendValue <= 0)
	{
		BlendValue = 0;
		FadeCurrentState = FADE_IN;
		FadeNextState = FADE_IN;
	}
	return TRUE;
}

int HoLogin::MainFadeOut()
{
	if(BlendValue < 255)
	{
		BlendValue += 8;
		
		if(SelectCurrentState != COMPLETE_SELECT_STATE &&
			SelectCurrentState != START_GAME_STATE)
		{
			LoginVolume -= 10;
			if(LoginVolume < 0)
				LoginVolume = 0;
		}
	}
	
	if(BlendValue >= 255)
	{
		BlendValue = 255;
		LoginVolume = 0;
		FadeCurrentState = FADE_OUT;
		FadeNextState = FADE_OUT;
	}
	return TRUE;
}

void HoLogin::SetFade()
{
	//Fade In을 해야 할경우.... 점점 밝아 지는 경우..

	if(FadeCurrentState == FADE_OUT)
	{
		FadeNextState = FADE_IN;
		BlendValue = 255;
		LoginVolume = 0;
	}
	else//Fade Out을 해야 할 경우...
	{
		FadeNextState = FADE_OUT;
		BlendValue = 0;
		LoginVolume = 350;
	}
}

int HoLogin::ReLoad()
{

//	if(CharacterSelectImage)
	//{
		//CharacterSelectImage->Release();
		//CharacterSelectImage = NULL;
		//CharacterSelectImage	= LoadDibSurfaceOffscreen( "StartImage\\Login\\SelectCharacter.bmp"); 

	//}
	//if(RaceSelectImage)
	//{
		//RaceSelectImage->Release();
		//RaceSelectImage = NULL;
		//RaceSelectImage			= LoadDibSurfaceOffscreen( "StartImage\\Login\\SelectRace.bmp" );
		
	//}
	//if(JobSelectImage)
	//{
		//JobSelectImage->Release();
		//JobSelectImage = NULL;
		//JobSelectImage			= LoadDibSurfaceOffscreen( "StartImage\\Login\\SelectJob.bmp");
	//}

	//if(FaceSelectImage)
	//{
		//FaceSelectImage->Release();
		//FaceSelectImage			= LoadDibSurfaceOffscreen( "StartImage\\Login\\SelectFace.bmp");
	//}

	//if(NameSelectImage)
//	{
		//NameSelectImage->Release();
		//NameSelectImage = NULL;
		//NameSelectImage			= LoadDibSurfaceOffscreen( "StartImage\\Login\\SelectName.bmp");
	//


	if(GameMapImage)
	{
		GameMapImage->Release();
		GameMapImage = NULL;
		GameMapImage = LoadDibSurfaceOffscreen( "StartImage\\Login\\Map.bmp");
	}

	//ReadTextures();
	return TRUE;
}

//마우스가 캐릭터가 위에 있는지 확인
//마우스가 캐릭터 위에 있을 경우: return 1
//마우스가 캐릭터 선택을 했을 경우: return 2 
int GetSelRectChar( smCHAR *lpChar)
{
	RECT rect;
	POINT	sPos;
	int sez;
	int SelZ = 65536*16384;

	if ( lpChar ) {

		//smRender.SetCameraPosi( x,y,z,ax,ay,az );

		sez = smRender.GetRect2D( lpChar->pX , lpChar->pY+(32*fONE) , lpChar->pZ ,
					24*fONE , 32*fONE , &rect , &sPos );


		if ( sez>0 && sez<SelZ && 
			rect.left<pCursorPos.x && rect.right>pCursorPos.x &&
			rect.top<pCursorPos.y && rect.bottom>pCursorPos.y )
		{

			if(MouseButton[0])
				return 2;
			
			
			return 1;

		}
	}

	return FALSE;
}


int GetSelRectJobChar( smCHAR *lpChar )
{
	
	RECT rect;
	POINT	sPos;
	int sez;
	int SelZ = 65536*16384;

	if ( lpChar )
	{
		//smRender.SetCameraPosi( x,y,z,ax,ay,az );
		sez = smRender.GetRect2D( lpChar->pX , lpChar->pY+(32*fONE) , lpChar->pZ ,
					24*fONE , 32*fONE , &rect , &sPos );
		if ( sez>0 && sez<SelZ && 
			rect.left<pCursorPos.x && rect.right>pCursorPos.x &&
			rect.top<pCursorPos.y && rect.bottom>pCursorPos.y )
		{
			
			if(MouseButton[0])
				return 2;

			return 1;

		}
	}
	
	return FALSE;
}