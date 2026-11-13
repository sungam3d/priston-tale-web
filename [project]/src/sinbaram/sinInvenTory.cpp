
/*----------------------------------------------------------------------------*
*	파일명 :  sinInvenTory.cpp	
*	하는일 :  인벤토리의 모든 설정을 관리한다 
*	작성일 :  최종업데이트 12월
*	적성자 :  박상열 
*-----------------------------------------------------------------------------*/	

#include "sinLinkHeader.h"

#include "..\\tjboy\\clanmenu\\tjclanDEF.h"
#include "..\\tjboy\\clanmenu\\tjclan.h"
#include "..\\field.h"
#include "..\\SrcServer\\onserver.h"
//ReformItem( sITEMINFO *lpItemInfo ); //아이템 인증받기 
//CheckItemForm( sITEMINFO *lpItemInfo ); //아이템 인증확인 

#ifdef _XTRAP_GUARD_4_CLIENT //HEAP TEST MEMORY
#include "..\\XTrapSrcD5\\ClientExt\\XTrap4ClientExt1.h"	//XTrapD5
#endif

//ReformCharForm(); //재인증 
//CheckCharForm(); //인증 
int LbuttonUpCheckFlag = 0; //이벤트 딜레이를 주기위해서 

int sinInterHeight2 = -50; //상점위치에 있는 인터페이스의 위치를 보정한다 

int sinTempDamage2[2] = {0,0}; //디스플레이시에만 보여줄 데미지
int sinTempAbsorb = 0;
int sinTempBlock  = 0;

sITEMINFO TempLinkCoreItem;
POINT UnionCorePosi = {0,0};
int   sinOverWeightCnt = 0;
int sinMyShopItemPass = 0;  


// 장별 - 입력창
int nName = 0;
int nYesButton= 0;
int nNoButton = 0;
int BackUpPosX , BackUpPosY = 0;
int BackUpButton2X , BackUpButton2Y = 0;
int BackUpButton3X , BackUpButton3Y = 0;

/*----------------------------------------------------------------------------*
*								전역변수 			
*-----------------------------------------------------------------------------*/	
cINVENTORY	cInvenTory;

sINVENTORY_EFFECT	sInven_Effect[MAX_INVEN_PARTICLE];	//인벤 파티클 
sINVENTORY_EFFECT   UnionCore_Effect[100];				//유니온 코어 인벤 파티클 
sINVENTORY_EFFECT   UnionCore_Effect_Bone[10];			//유니온 코어 인벤 파티클 
sINVENTORY_EFFECT   UnionCore_Effect_Bomb[50];			//유니온 코어 터지는 이펙트

sINVENTORY_EFFECT   ForceOrb_Effect;  //ForceOrb 이펙트 
int MatForceOrbEffect[8] = {0};
sITEM MyShopMouseItem;

sELEMENT_ATTACK	sElement_Attack;
sUNIONCORE_EFFECT   sUCore_Effect[10];

int StartX;  //인벤 박스의 시작점 X
int StartY;  // Y

int EndX;    //박스크기하나를 뺀 마지막 지점 X
int EndY;    //Y 

int ColorIndex;					//컬러 박스의 인덱스 
RECT ColorRect = {0,0,0,0};		//색이 표시될 영역 

int CrashItemIndex[2] = {0,0};	//아이템이 2개이상 겹치게되면 셋팅을 할수없다 
int AutoSetItemIndex = 0;       //양손무기를 셋팅할경우 자동으로 셋팅될 아이템의 인덱스 

int SelectInvenItemIndex=0;		//선택될 아이템의 인덱스 

int SelectPotionIndex = 0;

int ArrowState[2] = {0,0};

int InvenArrowPosi = 0;

sITEM *pUsePotion = 0;          //사용할 포션의 포인터 

int sinLoadOK=0;				//로드가 되면 1을 준다 

int PotionIndex=0;              //박스 거래를 위해 인덱스를 저장한다

sITEM *sinPosionItem= 0;        //물약 문제해결을 위한 포인터 
int BackUpItemIndex = 0;		//물약 문제해겨을 위한 아이템 인덱스 
int NotPotionFlag = 0;			//물약을 집었는지 아닌지를 채크 	

char sinTestBuff[128];			//테스트를 하기위한 버퍼 

int GhostPotionCheckFlag=0;		// 'E'키로인한 물약 사라지는 버그를 수정하기위한 날치기 플랙 

int sinChangeSetFlag = 0;

/*----------------------------------------------------------------------------*
*	The 3215 costume slot. Its page (Costume-UI.bmp) covers the weapon, armour
*	and shield slots while the costume view is on (the up arrow, or T); the
*	slots and items of the page that is not shown can't be clicked or dropped on.
*-----------------------------------------------------------------------------*/
static LPDIRECTDRAWSURFACE4 lpCostumeUI = 0, lpCostumeArrow = 0, lpCostumeInfo = 0;
extern int sinCostumeView;			// playsub.cpp
void sinToggleCostumeView();
static int InvenSlotHidden( int i )			// sInven index
{
	if ( sinCostumeView ) return i>=0 && i<=2;		// right hand, left hand, armour
	return i==INVENTORY_POS_SLOT_COSTUME;
}
static int InvenItemHidden( sITEM *pItem )
{
	return pItem->ItemPosition && InvenSlotHidden( pItem->ItemPosition-1 );
}

/*----------------------------------------------------------------------------*
*	Who may wear a costume (sinItem_CostumeFit.h, from the 3215 names): its
*	gender, and armour for the armour classes / robe for the robe classes
*	(Magician, Priestess, Shaman). TRUE for anything that is not a costume.
*-----------------------------------------------------------------------------*/
#include "sinItem_CostumeFit.h"
int sinCostumeFits( DWORD dwCode , int JobCode )
{
	for ( int k=0; sinCostumeFit[k][0]; k++ ) {
		if ( sinCostumeFit[k][0]!=dwCode ) continue;
		int female = ( JobCode==JOBCODE_ARCHER || JobCode==JOBCODE_ATALANTA || JobCode==JOBCODE_PRIESTESS ||
					   JobCode==JOBCODE_ASSASSIN || JobCode==JOBCODE_MARTIALARTIST );
		int robe = ( JobCode==JOBCODE_MAGICIAN || JobCode==JOBCODE_PRIESTESS || JobCode==JOBCODE_SHAMAN );
		return (int)sinCostumeFit[k][1]==( female ? 2 : 1 ) && (int)sinCostumeFit[k][2]==robe;
	}
	return TRUE;
}

int sinUsePotionDelayFlag = 0;
int PotionCheckNumJumpFlag = 1;

/////////////// 스킬관련
int sinUndeadAbsorb = 0;

int sinItemPickUpDelayFlag = 0;
float   sinAdd_fMagic_Mastery	= 0;			//마법숙련도 
int sinDamageFunction[3] = {0,0,0};

LPDIRECTDRAWSURFACE4	lpInvenMain;

LPDIRECTDRAWSURFACE4	lpUnionCoreEffect[4];

int MatUnionCoreEffect = 0;

LPDIRECTDRAWSURFACE4   lpGold7=0;
LPDIRECTDRAWSURFACE4   lpExp7=0;


LPDIRECTDRAWSURFACE4   lpSpecialItem[2];
LPDIRECTDRAWSURFACE4   lpSpecialItemInfo[2];

LPDIRECTDRAWSURFACE4   lpSpecialInven;


int IndiShopIconFlag = 0;

//아이템 Make
sWINGQUEST sWingQuest[MAX_WINGQUEST] = {
	#include "sinWingItem.h"
};


int SpecialItemShowFlag = 0;
int NotChangeSetItemFlag = 0;
int PuzzleOkFlag = 0;

int PuzzleOkY = 0;

int sinTestFlag2 = 0;

/*
void haTest(char *buff)
{
	HANDLE hFile;
    DWORD dwAcess =0;
	
    hFile = CreateFile( "ha.log", GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE,
                        NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );

    if( hFile != INVALID_HANDLE_VALUE ) {
        SetFilePointer( hFile , 0 , NULL , FILE_END );
		strcat(buff,"\r\n");
        //데이타 쓰기     버퍼    쓸크기    실제로 쓰여진 바이트수
        int chk = WriteFile( hFile , buff , lstrlen(buff) , &dwAcess , NULL );
        //파일 닫기
        CloseHandle( hFile );
   }
}
*/
///////////캐릭터 능력치 + 아이템 (색을 바꿔주려고 외부선언함 Flag 안쓰려고 음하하하)
/*----------------------------------------------------------------------------*
*   	                      아이템이 셋팅될 영역 
*-----------------------------------------------------------------------------*/	
sINVENTORY sInven[INVENTORY_MAX_POS] = {
	{INVENTORY_POS_RHAND  ,{ 574+1 , 433+1 , 640 , 544}},  //오른손			0
	{INVENTORY_POS_LHAND  ,{ 722+1 , 433+1 , 788 , 544}},  //왼손			1
	{INVENTORY_POS_ARMOR  ,{ 648+1 , 433+1 , 714 , 544}},  //갑옷			2
	{INVENTORY_POS_AMULET ,{ 304+1 , 438+1 , 326 , 460}},  //목걸이			3
	{INVENTORY_POS_RRING  ,{ 356+1 , 438+1 , 378 , 460}},  //오른쪽 링		4
	{INVENTORY_POS_LRING  ,{ 330+1 , 438+1 , 352 , 460}},  //왼쪽 링		5
	{INVENTORY_POS_SHELTOM,{ 382+1 , 438+1 , 404 , 460}},  //쉘텀			6
	{INVENTORY_POS_ARMLET ,{ 411+1 , 438+1 , 455 , 482}},  //암렛			7
	{INVENTORY_POS_GLOVES ,{ 462+1 , 438+1 , 506 , 482}},  //장갑			8
	{INVENTORY_POS_BOOTS  ,{ 513+1 , 438+1 , 557 , 482}},  //신발			9
	{INVENTORY_POS_POTION ,{ 495   , 565   , 495+26 , 565+26}},  //포션 1			10
	{INVENTORY_POS_POTION ,{ 495+26 , 565  , 495+26+26 , 565+26}},  //포션 2			11
	{INVENTORY_POS_POTION ,{ 495+26+26 , 565 , 495+26+26+26 , 565+26}},  //포션 3			12
	// the 3215 inventory (Inven.bmp has them drawn in)
	{INVENTORY_POS_COSTUME,{ 648+1 , 433+1 , 714 , 544}},  //costume: where the armour is, in costume view	13
	{INVENTORY_POS_PRING1 ,{ 512+1 , 500+1 , 534 , 522}},  //premium ring 1			14
	{INVENTORY_POS_PRING2 ,{ 539+1 , 500+1 , 561 , 522}},  //premium ring 2			15
};

//화살표 좌표 
int ArrowPosi[10][4] = {
	{26,572,26+22,591},
	{48,572,48+22,591},
	{524,525,542,543},		// weapon set A/B: where the 3215 Inven.bmp draws the left and right arrows
	{546,525,564,543},
	{266,572,266+22,591},//인벤 닫기 
	{302,467,321,486},//무게 아이콘 
	{120,572,120+20,591},//돈 아이콘 
	{70,572,70+22,591},//퀘스트 아이템
	{98,572,98+22,591},//개인상점
	{502,525,520,543},		// the up arrow: costume view on/off (also T)	(InvenArrowPosi 10)
};


int InvenTextPosi[2][4] = {
	{121,579,209,592}, //돈 
	{322,473,404,486}, //무게 

};
//Force Orb 를 사용한다
int ForceOrbUseLevel[][2] = {												 // 박재원 : 빌링 포스 추가(요구 레벨 50+)
	{1,19},{1,29},{1,39},{1,49},{50,200},{50,200},{50,200},{50,200},{50,200},{50,200},{50,200},{50,200},
};

// 박재원 - 매직 포스(사용 레벨, 요구 레벨)
int MagicForceOrbUseLevel[][2] = {	
	{1,19},{1,29},{1,39},{1,49},{50,200},{50,200},{60,200},{70,200},{80,200},{80,200},{90,200},{90,200},
};

// 박재원 - 빌링 매직 포스 추가(사용 레벨)
int BillingMagicForceOrbUseLevel[][2] = {	// 박재원 - 빌링 매직 포스 추가(요구 레벨)
	{1,200},{1,200},{1,200},
};

int ForceOrbUseTime[] = {500,500,500,500,700,900,1800,2400,3600,3600,3600,3600,3600,3600}; // 박재원 - 일반 포스 유지시간 추가

// 박재원 - 매직 포스 추가 
int MagicForceOrbUseTime[] = {500,500,500,500,700,900,1800,2400,3600,3600,3600,3600,3600,3600}; // 박재원 - 매직 포스 추가(유지시간 추가)

// 박재원 - 빌링 매직 포스 추가
int BillingMagicForceOrbUseTime[] = {3600, 10800, 86400}; // 박재원 - 빌링 매직 포스 유지시간 추가

int ForceOrbDamage[]  = {2,4,7,10,15,25,40,60,80,100,120,140,160,180}; // 박재원 - 빌링 포스 추가 공격력(벨룸, 오르도 포스 추가)
int MagicForceOrbDamage[]  = {2,4,7,10,15,25,40,60,80,100,120,140,160,180}; // 박재원 - 매직 포스 추가 공격력 
int BillingMagicForceOrbDamage[]  = {0,0,0}; // 박재원 - 빌링 매직 포스 추가 공격력 

int BoosterItem_UseTime[] = {3600,10800,86400}; // 박재원 - 부스터 아이템(1시간, 3시간, 1일) 유지시간 추가
int BoosterItem_DataPercent[] = {15,15,15}; // 박재원 - 부스터 아이템(생명력, 기력, 근력) 가중 퍼센트

// 장별 - 스킬 딜레이
int nSkillDelay_UseTime[] = {300,10800,86400};

/*----------------------------------------------------------------------------*
*						(인벤 토리)인벤  컬러 초기화 
*-----------------------------------------------------------------------------*/	
DWORD sinInvenColor[7] = {RGBA_MAKE( 160,235,256,128 ) , RGBA_MAKE( 255,255,255,128 ) ,RGBA_MAKE( 255,0,0,128 ),
					RGBA_MAKE( 255,255,255,128 ) , RGBA_MAKE( 255,200,0,128 ) ,RGBA_MAKE( 0,231,123,128 ),
					RGBA_MAKE( 173,203,247,128 )

};

//일곱가지 크리스탈 보은  크리스탈 코드
DWORD CristalEventCODE[12]={(sinGP1|sin01),(sinGP1|sin02),(sinGP1|sin03),(sinGP1|sin04),(sinGP1|sin05)
					       ,(sinGP1|sin06),(sinGP1|sin07),(sinGP1|sin08),(sinGP1|sin09),
						    (sinGP1|sin11),(sinGP1|sin12),(sinGP1|sin13)};

//박재원 - 수박 모아오기(호박) //박재원 - 밤하늘의 소원 이벤트
DWORD WatermelonEventCODE = (sinSP1|sin26);
// 장별 - 발렌타인 이벤트
DWORD ValentineEventCODE = (sinSP1|sin35);

// 박재원 - 알파벳 조합 이벤트
DWORD PristonAlphabetEventCODE[7] = { (sinSP1|sin27), (sinSP1|sin28), (sinSP1|sin29),
									  (sinSP1|sin30), (sinSP1|sin31), (sinSP1|sin32), (sinSP1|sin33) };

// 장별 - 캔디데이즈
DWORD CandydaysEventCODE = (sinSP1|sin36);

// 장별 - 매지컬그린
DWORD MagicalGreenEventCODE[2] = { (sinSP1|sin37), (sinSP1|sin38) };

// 장별 - 카라의 눈물
DWORD TeatOfKaraEventCODE = (sinSP1|sin39);

// 장별 - 조사원을 찾아라 - 마력연구자
DWORD FindinvestigatorEventCODE[2] = { (sinGF1|sin07), (sinGF1|sin08) };

/*----------------------------------------------------------------------------*
*							클래스 초기, 종료 
*-----------------------------------------------------------------------------*/	
cINVENTORY::cINVENTORY()
{
	//인벤토리 영역 초기화 
	for(int i=0 ; i<INVENTORY_MAXITEM;i++)
		memset(&InvenItem[i],0,sizeof(sITEM));

}
cINVENTORY::~cINVENTORY()
{

}

/*----------------------------------------------------------------------------*
*							     초기화 
*-----------------------------------------------------------------------------*/
void cINVENTORY::Init()
{
	//////////////포션공간을 정한다
	
	StartX = 23;  //인벤 박스의 시작점 X
	StartY = 436; // Y

	EndX   = StartX+((INVENTORY_BOX_ROW) *22); //박스크기하나를 뺀 마지막 지점 X
	EndY   = StartY+((INVENTORY_BOX_COL) *22); //Y 


	//MatInven[0] =  CreateTextureMaterial( "Image\\SinImage\\Inven\\Inven-1.tga" , 0, 0, 0,0, SMMAT_BLEND_ALPHA );
	//MatInven[1] =  CreateTextureMaterial( "Image\\SinImage\\Inven\\Inven-2.tga" , 0, 0, 0,0, SMMAT_BLEND_ALPHA );
	//MatInven[2] =  CreateTextureMaterial( "Image\\SinImage\\Inven\\Inven-3.tga" , 0, 0, 0,0, SMMAT_BLEND_ALPHA );
	//MatInven[3] =  CreateTextureMaterial( "Image\\SinImage\\Inven\\Inven-4.tga" , 0, 0, 0,0, SMMAT_BLEND_ALPHA );
	Load();

	ReformCharForm();
	SearchJobAndSetting();  //Job코드와 이름을 구해온다 그다음에 SetItemToChar()초기화한다 
	CheckWeight();          //무게를 체크한다 
	SetItemToChar();        //초기화 할때 넣어준다 

	sinSetLife(sinChar->Life[1]);
	sinSetMana(sinChar->Mana[1]);
	sinSetStamina(sinChar->Stamina[1]);
	ReformCharForm();       //재인증 

	int i = 0;
	char szBuff[128];
	for(i = 0 ; i < 5 ; i++){
		wsprintf(szBuff,"Image\\SinImage\\Effect\\Particle\\InvenPaticle%d.tga",i);
		MatInvenParticle[i] = CreateTextureMaterial(szBuff, 0, 0, 0,0, SMMAT_BLEND_ALPHA );
		
	}
	MatUnionCoreEffect = CreateTextureMaterial("image\\Sinimage\\Effect\\InterEffect\\UC-hall.tga",0, 0, 0,0, SMMAT_BLEND_ALPHA );
	

	/*
	char *Path = "image\\Sinimage\\Effect\\ForceOrb\\P_0%d.tga";
	if(!MatForceOrbEffect[0]){
		for( i = 0; i < 8; i++){
			wsprintf(szBuff,Path,i+1);
			MatForceOrbEffect[i] = CreateTextureMaterial(szBuff, 0, 0, 0,0, SMMAT_BLEND_ALPHA );
			  
		}
	}
	*/
	LoadCharState(); 
	

}
/*----------------------------------------------------------------------------*
*						    Bmp파일을 로드한다 
*-----------------------------------------------------------------------------*/	
void cINVENTORY::Load()
{
	char szBuff[128];
	for(int  i = 0 ; i < 4 ; i++){
		wsprintf(szBuff,"image\\Sinimage\\Effect\\InterEffect\\UC-0%d.bmp",i+1);
		lpUnionCoreEffect[i] = LoadDibSurfaceOffscreen(szBuff);

	}

	lpIndiShopIcon = LoadDibSurfaceOffscreen( "Image\\SinImage\\indishop\\indishop_icon.bmp" );
	lpMyShopIcon_T = LoadDibSurfaceOffscreen( "Image\\SinImage\\Inven\\InfoBox\\inven-SaleT.bmp" );

	lpMyShopIcon = LoadDibSurfaceOffscreen( "Image\\SinImage\\Inven\\inven-Sale.bmp" );

	lpSpecialItem[0] = LoadDibSurfaceOffscreen( "Image\\SinImage\\Inven\\QuestItem_.bmp" );
	lpSpecialItem[1] = LoadDibSurfaceOffscreen( "Image\\SinImage\\Inven\\QuestItem.bmp" );

	lpSpecialItemInfo[0] = LoadDibSurfaceOffscreen( "Image\\SinImage\\Inven\\InfoBox\\Questitem.bmp" );
	lpSpecialItemInfo[1] = LoadDibSurfaceOffscreen( "Image\\SinImage\\Inven\\InfoBox\\close.bmp" ); 

	lpSpecialInven = LoadDibSurfaceOffscreen( "Image\\SinImage\\Inven\\QuestItembox.bmp" ); 
	
	lpInvenMain = LoadDibSurfaceOffscreen( "Image\\SinImage\\Inven\\Inven.bmp" );
	lpInvenArrow[0] = LoadDibSurfaceOffscreen( "Image\\SinImage\\Inven\\LeftArrow.bmp" );
	lpInvenArrow[1] = LoadDibSurfaceOffscreen( "Image\\SinImage\\Inven\\RightArrow.bmp" );
	// the costume slot (3215): its panel over the weapon/armour/shield slots, the lit up arrow, its label
	lpCostumeUI    = LoadDibSurfaceOffscreen( "Image\\SinImage\\Inven\\Costume-UI.bmp" );
	lpCostumeArrow = LoadDibSurfaceOffscreen( "Image\\SinImage\\Inven\\CArrow.bmp" );
	lpCostumeInfo  = LoadDibSurfaceOffscreen( "Image\\SinImage\\Inven\\InfoBox\\Costume.bmp" );

	lpCoverSmall   = LoadDibSurfaceOffscreen( "Image\\SinImage\\Inven\\CoverSmall.bmp" );
	lpCoverMiddle  = LoadDibSurfaceOffscreen( "Image\\SinImage\\Inven\\CoverMiddle.bmp" );
	lpCoverLarge   = LoadDibSurfaceOffscreen( "Image\\SinImage\\Inven\\CoverLarge.bmp" );

	lpPotionBack   = LoadDibSurfaceOffscreen( "Image\\SinImage\\Inven\\PotionBack.bmp" ); 

	lpChangeABInfo[0] = LoadDibSurfaceOffscreen( "Image\\SinImage\\Inven\\InfoBox\\ButtonA.bmp" ); 
	lpChangeABInfo[1] = LoadDibSurfaceOffscreen( "Image\\SinImage\\Inven\\InfoBox\\ButtonB.bmp" ); 

	lpExitInfo = LoadDibSurfaceOffscreen( "Image\\SinImage\\Inven\\InfoBox\\ButtonExit.bmp" ); 
	lpWeightInfo = LoadDibSurfaceOffscreen( "Image\\SinImage\\Inven\\InfoBox\\ButtonWeight.bmp" ); 

	lpItemABInfo[0] = LoadDibSurfaceOffscreen( "Image\\SinImage\\Inven\\InfoBox\\ButtonItemA.bmp" ); 
	lpItemABInfo[1] = LoadDibSurfaceOffscreen( "Image\\SinImage\\Inven\\InfoBox\\ButtonItemB.bmp" ); 

	lpShowWeaponClass[0] =LoadDibSurfaceOffscreen( "Image\\SinImage\\Items\\Weapon_Onehand.bmp" );
	lpShowWeaponClass[1] =LoadDibSurfaceOffscreen( "Image\\SinImage\\Items\\Weapon_Twohand.bmp" );
	lpShowWeaponClass[2] =LoadDibSurfaceOffscreen( "Image\\SinImage\\Items\\Weapon_Knight.bmp" );
	lpShowWeaponClass[3] =LoadDibSurfaceOffscreen( "Image\\SinImage\\Items\\Weapon_Wizard.bmp" );


	lpPuzzleInven  =LoadDibSurfaceOffscreen( "Image\\SinImage\\Inven\\NYEAR_00.bmp" );
	lpPuzzlewidth  =LoadDibSurfaceOffscreen( "Image\\SinImage\\Inven\\NYEAR_01.bmp" );
	lpPuzzleHeight =LoadDibSurfaceOffscreen( "Image\\SinImage\\Inven\\NYEAR_02.bmp" );

}//아이템을 로드한다 
void cINVENTORY::InvenItemLoad()
{
	char szFilePath[256];
	for(int i = 0 ; i < INVENTORY_MAXITEM ; i++){
		if(InvenItem[i].Flag){
			for(int j=0 ; j < MAX_ITEM ; j++){
				if(InvenItem[i].CODE == sItem[j].CODE){	
					wsprintf(szFilePath ,"Image\\sinImage\\Items\\%s\\it%s.bmp",sItem[j].ItemFilePath,sItem[j].LastCategory);
					if(!sItem[j].lpTempItem)
						sItem[j].lpTempItem = LoadDibSurfaceOffscreen(szFilePath);
					InvenItem[i].lpItem = sItem[j].lpTempItem;
					break;
				}
			}
		}
		if(InvenItemTemp[i].Flag){
			for(int j=0 ; j < MAX_ITEM ; j++){
				if(InvenItemTemp[i].CODE == sItem[j].CODE){	
					wsprintf(szFilePath ,"Image\\sinImage\\Items\\%s\\it%s.bmp",sItem[j].ItemFilePath,sItem[j].LastCategory);
					if(!sItem[j].lpTempItem)
						sItem[j].lpTempItem = LoadDibSurfaceOffscreen(szFilePath);
					InvenItemTemp[i].lpItem = sItem[j].lpTempItem;
					break;
				}
			}
		}
	}
}
//아이템을 Release한다 
void cINVENTORY::InvenItemRelease()
{
	int i = 0;
	for(i = 0 ; i < MAX_ITEM ; i++){
		if(sItem[i].lpTempItem){
			sItem[i].lpTempItem->Release();
			sItem[i].lpTempItem = 0;

		}
	}

	for( i = 0 ; i < INVENTORY_MAXITEM ; i++){
		if(InvenItem[i].Flag){
			if(InvenItem[i].lpItem){
				InvenItem[i].lpItem->Release();
				InvenItem[i].lpItem = 0;
			}

		}
		if(InvenItemTemp[i].Flag){
			if(InvenItemTemp[i].lpItem){
				InvenItemTemp[i].lpItem->Release();
				InvenItemTemp[i].lpItem = 0;
			}

		}

	}

}

/*----------------------------------------------------------------------------*
*						     Release
*-----------------------------------------------------------------------------*/	
void cINVENTORY::Release()
{
	int i;

	if(lpCostumeUI){ lpCostumeUI->Release(); lpCostumeUI = 0; }
	if(lpCostumeArrow){ lpCostumeArrow->Release(); lpCostumeArrow = 0; }
	if(lpCostumeInfo){ lpCostumeInfo->Release(); lpCostumeInfo = 0; }

	for(i = 0 ; i <2 ; i++){
		if(lpInvenArrow[i]){
			lpInvenArrow[i]->Release();
			lpInvenArrow[i] = 0;
		}
		if(lpChangeABInfo[i]){
			lpChangeABInfo[i]->Release();
			lpChangeABInfo[i] = 0;
		}
		if(lpItemABInfo[i]){
			lpItemABInfo[i]->Release();
			lpItemABInfo[i] = 0;
		}
		
	}
	if(lpCoverSmall){
		lpCoverSmall->Release();
		lpCoverSmall = 0;
	}
	if(lpCoverMiddle){
		lpCoverMiddle->Release();
		lpCoverMiddle = 0;
	}
	if(lpCoverLarge){
		lpCoverLarge->Release();
		lpCoverLarge = 0;
	}
	if(lpPotionBack){
		lpPotionBack->Release();
		lpPotionBack = 0;
	}
	if(lpExitInfo){
		lpExitInfo->Release();
		lpExitInfo =0;
	}
	if(lpWeightInfo){
		lpWeightInfo->Release();
		lpWeightInfo = 0;
	}
	if(lpShowWeaponClass[0]){
		lpShowWeaponClass[0]->Release();
		lpShowWeaponClass[0] = 0;

	}

	if(lpShowWeaponClass[1]){
		lpShowWeaponClass[1]->Release();
		lpShowWeaponClass[1] = 0;

	}


}
/*----------------------------------------------------------------------------*
*							     그리기  
*-----------------------------------------------------------------------------*/	
int CoreEffectTime = 0;
int CoreEffectFlag = 0;
void cINVENTORY::Draw()
{

	int i=0;
	/////////////////인벤토리 박스 
	DrawSprite(0,600-sinMoveKindInter[SIN_INVENTORY]+56 , lpInvenMain,0,0,800,200);    
	//dsDrawTexImage( MatInven[0] , 0, 600-sinMoveKindInter[SIN_INVENTORY], 256, 256 , 255 );       
	//dsDrawTexImage( MatInven[1] , 256, 600-sinMoveKindInter[SIN_INVENTORY], 256, 256 , 255 );   
	//dsDrawTexImage( MatInven[2] , 256+256, 600-sinMoveKindInter[SIN_INVENTORY], 256, 256 , 255 );       
	//dsDrawTexImage( MatInven[3] , 256+256+256, 600-sinMoveKindInter[SIN_INVENTORY], 32, 256 , 255 );   

	//퍼즐 칸을 넣는다
	//if()


	// 장별 - 입력창
	if(nName == 1)
	{
			    sinChatEnter = 0; //임의로 글씨가 안보이게한다
				cInterFace.ChatFlag = 0;

		//		BackUpButton2X = 35;    
		//		BackUpButton2Y = 157;
		//		BackUpButton3X = 115;
		//		BackUpButton3Y = 157;

		//		BackUpPosX = BackStartPos.x;
		//		BackUpPosY = BackStartPos.y;

			//	BackStartPos.x = 300;
			//	BackStartPos.y = 100;

				Interface_clanmenu.DrawBox(BackStartPos.x,BackStartPos.y,2,3);	            
				dsDrawTexImage(MatHelpBoxImage[9],BackStartPos.x+7,BackStartPos.y+35,180,15,255);
				
				
				dsDrawTexImage(MatMyShopPRBoxLine,BackStartPos.x+28,BackStartPos.y-120,254,30,255);

				DrawSprite(BackStartPos.x+BackUpButton2X,BackStartPos.y+BackUpButton2Y-200, cMessageBox.lpButtonBox07,  0, 0, 48, 23, 1);
				DrawSprite(BackStartPos.x+BackUpButton3X,BackStartPos.y+BackUpButton3Y-200, cMessageBox.lpButtonBox07,  0, 0, 48, 23, 1);

				// 예 아니오(회색)
			//	DrawSprite(BackStartPos.x+BackUpButton2X+7,BackStartPos.y+BackUpButton2Y-200+3, cMessageBox.lpButtonYesGlay07,  0, 0, 32, 16, 1);
			//	DrawSprite(BackStartPos.x+BackUpButton3X+7,BackStartPos.y+BackUpButton3Y-200+3, cMessageBox.lpButtonNoGlay07,  0, 0, 32, 16, 1);


			//	BackStartPos.x = BackUpPosX;
			//	BackStartPos.y = BackUpPosY;


				// 예 버튼에 마우스 
				if(pCursorPos.x >= BackStartPos.x+BackUpButton2X && pCursorPos.x <= BackStartPos.x+BackUpButton2X + 48 &&
				pCursorPos.y >= BackStartPos.y+BackUpButton2Y - 30   && pCursorPos.y <= BackStartPos.y+BackUpButton2Y- 7)
				{
					nYesButton = 1;

					/*
					nName = 0;
					isDrawClanMenu = 0;
					SetIME_Mode(0);
					cInterFace.ChatFlag = 0;
					sinChatEnter = 0;
					sinMessageBoxShowFlag = 0;
					hFocusWnd = 0;  
					cInterFace.ChatFlag = 0;
					*/
				}
				else nYesButton = 0;

				// 아니오 버튼에 마우스 
				if(pCursorPos.x >= BackStartPos.x+BackUpButton3X && pCursorPos.x <= BackStartPos.x+BackUpButton3X + 48 &&
				pCursorPos.y >= BackStartPos.y+BackUpButton2Y - 30   && pCursorPos.y <= BackStartPos.y+BackUpButton2Y- 7)
				{
					
					nNoButton = 1;
					
					nName = 0;
					isDrawClanMenu = 0;
					SetIME_Mode(0);
					cInterFace.ChatFlag = 0;
					sinChatEnter = 0; 
					sinMessageBoxShowFlag = 0;
					hFocusWnd = 0;  
					cInterFace.ChatFlag = 0;
					
				}
				else nNoButton = 0;

				if( nYesButton == 0 ) DrawSprite(BackStartPos.x+BackUpButton2X+7,BackStartPos.y+BackUpButton2Y-200+3, cMessageBox.lpButtonYesGlay07,  0, 0, 32, 16, 1);
				else DrawSprite(BackStartPos.x+BackUpButton2X+7,BackStartPos.y+BackUpButton2Y-200+3, cMessageBox.lpButtonYes07,  0, 0, 32, 16, 1);

				if( nNoButton == 0 ) DrawSprite(BackStartPos.x+BackUpButton3X+7,BackStartPos.y+BackUpButton3Y-200+3, cMessageBox.lpButtonNoGlay07,  0, 0, 32, 16, 1);
				else DrawSprite(BackStartPos.x+BackUpButton3X+7,BackStartPos.y+BackUpButton3Y-200+3, cMessageBox.lpButtonNo07,  0, 0, 32, 16, 1);
	
				

			}


	////////////////무기 셋팅여역 배경 
	if(sinCostumeView && lpCostumeUI){	// the costume page over the weapon / armour / shield slots
		DrawSprite(sInven[0].Rect.left-3 , sInven[0].Rect.top-3+(256-sinMoveKindInter[SIN_INVENTORY]) , lpCostumeUI ,0 ,0 , 220 , 116);
		i = INVENTORY_POS_SLOT_COSTUME;
		if(sInven[i].ItemIndex && InvenItem[sInven[i].ItemIndex-1].Flag)
			DrawSprite(sInven[i].Rect.left , sInven[i].Rect.top+(256-sinMoveKindInter[SIN_INVENTORY]) , lpCoverLarge ,0 ,0 , 66 , 110);
	}
	else
	for( i=0 ; i<3 ; i++){ //값옷 , 무기 배경
		if(sInven[i].ItemIndex && InvenItem[sInven[i].ItemIndex-1].Flag)
			DrawSprite(sInven[i].Rect.left , sInven[i].Rect.top+(256-sinMoveKindInter[SIN_INVENTORY]) , lpCoverLarge ,0 ,0 , 66 , 110);
	}

	for( i=3 ; i<7 ; i++){    //링 , 아물렛 , 쉘텀
		if(sInven[i].ItemIndex && InvenItem[sInven[i].ItemIndex-1].Flag)
			DrawSprite(sInven[i].Rect.left , sInven[i].Rect.top+(256-sinMoveKindInter[SIN_INVENTORY]) , lpCoverSmall ,0 ,0 , 22 , 22);
	}
	for( i=INVENTORY_POS_SLOT_PRING ; i<INVENTORY_POS_SLOT_PRING+2 ; i++){    // the premium rings
		if(sInven[i].ItemIndex && InvenItem[sInven[i].ItemIndex-1].Flag)
			DrawSprite(sInven[i].Rect.left , sInven[i].Rect.top+(256-sinMoveKindInter[SIN_INVENTORY]) , lpCoverSmall ,0 ,0 , 22 , 22);
	}

	for( i=7 ; i<10 ; i++){    //신발, 암렛, 장갑 배경 
		if(sInven[i].ItemIndex && InvenItem[sInven[i].ItemIndex-1].Flag)
			DrawSprite(sInven[i].Rect.left , sInven[i].Rect.top+(256-sinMoveKindInter[SIN_INVENTORY]) , lpCoverMiddle ,0 ,0 , 44 , 44);
	}

	///////////그림 나오는 순서대루 정열 
	if(ArrowState[0])
		DrawSprite(ArrowPosi[1][0] , ArrowPosi[1][1]+(256-sinMoveKindInter[SIN_INVENTORY]),lpInvenArrow[1],0 ,0 , 20,20);
	else
		DrawSprite(ArrowPosi[0][0] , ArrowPosi[0][1]+(256-sinMoveKindInter[SIN_INVENTORY]),lpInvenArrow[0],0 ,0 , 20,20);

	if(ArrowState[1])
		DrawSprite(ArrowPosi[3][0], ArrowPosi[3][1]+(256-sinMoveKindInter[SIN_INVENTORY]),lpInvenArrow[1],0 ,0 , 20,20);
	else
		DrawSprite(ArrowPosi[2][0], ArrowPosi[2][1]+(256-sinMoveKindInter[SIN_INVENTORY]),lpInvenArrow[0],0 ,0 , 20,20);

	DrawSprite(sInven[10].Rect.left , sInven[10].Rect.top , lpPotionBack ,0,0,77,25);	 	    


	//퀘스트 아이템 버튼
	if(!SpecialItemShowFlag){ 
		DrawSprite(ArrowPosi[7][0], ArrowPosi[7][1]+(256-sinMoveKindInter[SIN_INVENTORY]),lpSpecialItem[0],0 ,0 , 20,20);
	}
	else{
		for(i = 0; i < 6; i++){
			for(int j = 0 ; j < 3 ; j++){
				DrawSprite(StartX+(i*44), StartY+(j*44)+(256-sinMoveKindInter[SIN_INVENTORY]),lpSpecialInven,0 ,0 , 44,44);
			}
		}
		
		DrawSprite(ArrowPosi[7][0], ArrowPosi[7][1]+(256-sinMoveKindInter[SIN_INVENTORY]),lpSpecialItem[1],0 ,0 , 20,20);
	}

	////////////////아이템 배경 색 셋팅 (녹) No셋팅(적) Select(황)...
	switch(ColorIndex){ 
		case SET_ITEM_CHECK_COLOR: //셋팅영역 
			if(MouseItem.Flag){
				dsDrawColorBox( sinInvenColor[0] ,ColorRect.left,ColorRect.top+(256-sinMoveKindInter[SIN_INVENTORY]), ColorRect.right, ColorRect.bottom );
			}
		break;
		case OVERLAP_ITEM_COLOR:   //겹친 아이템 영역 
			if(CrashItemIndex[0]){
				dsDrawColorBox( sinInvenColor[1] ,InvenItem[CrashItemIndex[0]-1].x,
					InvenItem[CrashItemIndex[0]-1].y+(256-sinMoveKindInter[SIN_INVENTORY]), InvenItem[CrashItemIndex[0]-1].w, InvenItem[CrashItemIndex[0]-1].h );	
			}

		break;
		case NOT_SETTING_COLOR:    //세팅할수 없음 영역 
			dsDrawColorBox( sinInvenColor[2] ,ColorRect.left,ColorRect.top+(256-sinMoveKindInter[SIN_INVENTORY]), ColorRect.right, ColorRect.bottom );
		break;
		
		case NOT_USE_COLOR:        //사용할수 없는 아이템 
			dsDrawColorBox( sinInvenColor[2] ,ColorRect.left,ColorRect.top+(256-sinMoveKindInter[SIN_INVENTORY]), ColorRect.right, ColorRect.bottom );
		break;

		case OVERLAP_BOX_COLOR:    
			if(CrashItemIndex[0]){
				dsDrawColorBox( sinInvenColor[1] ,sInven[InvenItem[CrashItemIndex[0]-1].ItemPosition-1].Rect.left ,sInven[InvenItem[CrashItemIndex[0]-1].ItemPosition-1].Rect.top+(256-sinMoveKindInter[SIN_INVENTORY]) ,
					sInven[InvenItem[CrashItemIndex[0]-1].ItemPosition-1].Rect.right  - sInven[InvenItem[CrashItemIndex[0]-1].ItemPosition-1].Rect.left,
					sInven[InvenItem[CrashItemIndex[0]-1].ItemPosition-1].Rect.bottom - sInven[InvenItem[CrashItemIndex[0]-1].ItemPosition-1].Rect.top);
			}

		break;
		case SELECT_ITEM_COLOR:
			if(!SelectInvenItemIndex)break; //표시될 아이템 인덱스가 없으면 break
			if(InvenItem[SelectInvenItemIndex-1].ItemPosition){ //창고가 아닐때 
				dsDrawColorBox( sinInvenColor[4] ,sInven[InvenItem[SelectInvenItemIndex-1].ItemPosition-1].Rect.left ,sInven[InvenItem[SelectInvenItemIndex-1].ItemPosition-1].Rect.top+(256-sinMoveKindInter[SIN_INVENTORY]) ,
					sInven[InvenItem[SelectInvenItemIndex-1].ItemPosition-1].Rect.right  - sInven[InvenItem[SelectInvenItemIndex-1].ItemPosition-1].Rect.left,
					sInven[InvenItem[SelectInvenItemIndex-1].ItemPosition-1].Rect.bottom - sInven[InvenItem[SelectInvenItemIndex-1].ItemPosition-1].Rect.top);
					
				}
				else{
				dsDrawColorBox( sinInvenColor[4] ,InvenItem[SelectInvenItemIndex-1].x,
					InvenItem[SelectInvenItemIndex-1].y+(256-sinMoveKindInter[SIN_INVENTORY]), InvenItem[SelectInvenItemIndex-1].w, InvenItem[SelectInvenItemIndex-1].h );	
				}

		break;

	}

	//유니온 코어 날치기 이펙트 
	for(i = 0 ; i < 10 ; i++){    
		if(sUCore_Effect[i].Flag && sUCore_Effect[i].Time > 0){
			dsDrawTexImage(sUCore_Effect[i].MatNum,sUCore_Effect[i].x,sUCore_Effect[i].y,sUCore_Effect[i].w,
				sUCore_Effect[i].h,sUCore_Effect[i].Alpha);
		}
	}


	for(int cnt = 0 ;cnt < INVENTORY_MAXITEM ; cnt++){ 
		if(InvenItem[cnt].Flag == 1){ //인벤토리에 아이템이 있을때 아이템을 그린다 
			//인벤토리에 퍼즐이 있으면 그린다
			if((InvenItem[cnt].sItemInfo.CODE & sinITEM_MASK2) == sinPZ1 || (InvenItem[cnt].sItemInfo.CODE & sinITEM_MASK2) == sinPZ2){
				DrawSprite(StartX+(22*8),sInven[0].Rect.top+(256-sinMoveKindInter[SIN_INVENTORY])+2 , lpPuzzleInven,0,0,88,88);
			}

			if(InvenItemHidden(&InvenItem[cnt]))continue;		// on the costume / gear page that is not shown
			if(InvenItem[cnt].sItemInfo.NotUseFlag && !MyShopItemIndex[cnt]){
				dsDrawColorBox( sinInvenColor[2] ,InvenItem[cnt].x,InvenItem[cnt].y+(256-sinMoveKindInter[SIN_INVENTORY]), InvenItem[cnt].w, InvenItem[cnt].h );
				if(InvenItem[cnt].ItemPosition != 0){
						dsDrawColorBox( sinInvenColor[2] ,sInven[InvenItem[cnt].ItemPosition-1].Rect.left,sInven[InvenItem[cnt].ItemPosition-1].Rect.top + (256-sinMoveKindInter[SIN_INVENTORY]),
							sInven[InvenItem[cnt].ItemPosition-1].Rect.right - sInven[InvenItem[cnt].ItemPosition-1].Rect.left,
							sInven[InvenItem[cnt].ItemPosition-1].Rect.bottom - sInven[InvenItem[cnt].ItemPosition-1].Rect.top );
				}
			}
		}
	}

	for(i=0 ; i < INVENTORY_MAXITEM ; i++){
		if(InvenItem[i].Flag && !InvenItemHidden(&InvenItem[i])){
			//인터페이스의 물약창이 아닐경우에만 아이템의 높이를 보정한다
			if(InvenItem[i].ItemPosition != 11 && InvenItem[i].ItemPosition != 12 && InvenItem[i].ItemPosition != 13){
				//지금 상점에 등록된 아이템은 이미지를 표시해준다
				if(MyShopItemIndex[i]){ 
					dsDrawColorBox( RGBA_MAKE(45,133,255,180) ,InvenItem[i].x ,InvenItem[i].y+(256-sinMoveKindInter[SIN_INVENTORY]),InvenItem[i].w ,InvenItem[i].h); 
				}

				DrawSprite(InvenItem[i].x , InvenItem[i].y+(256-sinMoveKindInter[SIN_INVENTORY]), InvenItem[i].lpItem ,0,0, InvenItem[i].w , InvenItem[i].h);
				//이미지가 없는 아이템을 그려준다 
				if(!InvenItem[i].lpItem){  
					if(InvenItem[i].x > 10 ){//땜빵 -_-
						dsDrawColorBox( RGBA_MAKE(125+(i*10),125+(i*10),255+(i*10),125+(i*10)) ,InvenItem[i].x ,InvenItem[i].y+(256-sinMoveKindInter[SIN_INVENTORY]),InvenItem[i].w ,InvenItem[i].h); 
					}

				}
				if((InvenItem[i].CODE & sinITEM_MASK2) == sinFO1 ){
					DrawForceOrbEffect(InvenItem[i].x , InvenItem[i].y+(256-sinMoveKindInter[SIN_INVENTORY]));
				}
			}
			else{
				DrawSprite(InvenItem[i].x , InvenItem[i].y, InvenItem[i].lpItem ,0,0, InvenItem[i].w , InvenItem[i].h); 
			}
		}
	}

	//완성된 퍼즐 이펙트
	if(PuzzleOkFlag){    
		dsDrawColorBox( RGBA_MAKE(255,255,0,30) , StartX+(22*8) ,sInven[0].Rect.top+(256-sinMoveKindInter[SIN_INVENTORY])+2,88 ,88); 

		dsDrawColorBox( RGBA_MAKE(255,123,0,180) , StartX+(22*8) ,PuzzleOkY+sInven[0].Rect.top+(256-sinMoveKindInter[SIN_INVENTORY])+4,88 ,2); 
		dsDrawColorBox( RGBA_MAKE(255,123,0,180) , StartX+(22*8) ,(80-PuzzleOkY)+sInven[0].Rect.top+(256-sinMoveKindInter[SIN_INVENTORY])+4,88 ,2); 


	}

	int len = 0;
	if(CoreEffectFlag == 0){ 
		CoreEffectTime++;
	}
	else 
		CoreEffectTime--;

	if(CoreEffectTime > 38 )
		CoreEffectFlag = 1;
	else if(CoreEffectTime < 1)
		CoreEffectFlag = 0;


	for(i=0 ; i < INVENTORY_MAXITEM ; i++){ 
		if(InvenItem[i].Flag){ 
			if(InvenItem[i].sItemInfo.CODE == (sinEC1|sin05)){
				len = lstrlen(InvenItem[i].sItemInfo.ItemName); 
				if(InvenItem[i].sItemInfo.ItemName[len+1] != NULL){ 
					DrawSprite(InvenItem[i].x , InvenItem[i].y+(256-sinMoveKindInter[SIN_INVENTORY]), lpUnionCoreEffect[CoreEffectTime/10],0,0,44,22);

				}
			}
		}
	}
	switch(InvenArrowPosi){
		case 1:
			DrawSprite(ArrowPosi[0][0] , ArrowPosi[0][1]+(256-sinMoveKindInter[SIN_INVENTORY]),lpInvenArrow[0],0 ,0 , 20,20);
			DrawSprite(ArrowPosi[0][0] , ArrowPosi[0][1]-27+(256-sinMoveKindInter[SIN_INVENTORY]),lpItemABInfo[0],0,0,57,27);
			
		break;
		case 2:
			DrawSprite(ArrowPosi[1][0] , ArrowPosi[1][1]+(256-sinMoveKindInter[SIN_INVENTORY]),lpInvenArrow[1],0 ,0 , 20,20);
			DrawSprite(ArrowPosi[1][0] , ArrowPosi[1][1]-27+(256-sinMoveKindInter[SIN_INVENTORY]),lpItemABInfo[1],0,0,57,27);

		break;
		case 3:
			DrawSprite(ArrowPosi[2][0], ArrowPosi[2][1]+(256-sinMoveKindInter[SIN_INVENTORY]),lpInvenArrow[0],0 ,0 , 20,20);
			DrawSprite(ArrowPosi[2][0], ArrowPosi[2][1]-27+(256-sinMoveKindInter[SIN_INVENTORY]),lpChangeABInfo[0],0,0,70,27);
		break;
		case 4:
			DrawSprite(ArrowPosi[3][0], ArrowPosi[3][1]+(256-sinMoveKindInter[SIN_INVENTORY]),lpInvenArrow[1],0 ,0 , 20,20);
			DrawSprite(ArrowPosi[3][0], ArrowPosi[3][1]-27+(256-sinMoveKindInter[SIN_INVENTORY]),lpChangeABInfo[1],0,0,70,27);
		break;

		case 5: 
			DrawSprite(ArrowPosi[4][0], ArrowPosi[4][1]+(256-sinMoveKindInter[SIN_INVENTORY]),cShop.lpExit,0 ,0 , 20,20);
			DrawSprite(ArrowPosi[4][0]-8, ArrowPosi[4][1]-27+(256-sinMoveKindInter[SIN_INVENTORY]),lpExitInfo,0 ,0 , 47,27);
		break;
		case 6:
			//DrawSprite(346, 515-27+(256-sinMoveKindInter[SIN_INVENTORY]),lpWeightInfo,0 ,0 , 70,27);
		break;
		case 8:
			if(!SpecialItemShowFlag){   
				DrawSprite(ArrowPosi[7][0], ArrowPosi[7][1]+(256-sinMoveKindInter[SIN_INVENTORY]),lpSpecialItem[1],0 ,0 , 20,20);
				DrawSprite(ArrowPosi[7][0], ArrowPosi[7][1]-27+(256-sinMoveKindInter[SIN_INVENTORY]),lpSpecialItemInfo[0],0 ,0 , 100,27);
			}
			else{
				DrawSprite(ArrowPosi[7][0], ArrowPosi[7][1]+(256-sinMoveKindInter[SIN_INVENTORY]),lpSpecialItem[1],0 ,0 , 20,20);
				DrawSprite(ArrowPosi[7][0], ArrowPosi[7][1]-27+(256-sinMoveKindInter[SIN_INVENTORY]),lpSpecialItemInfo[1],0 ,0 , 70,27);
			}
		break;
		case 9:
			DrawSprite(ArrowPosi[8][0], ArrowPosi[8][1]+(256-sinMoveKindInter[SIN_INVENTORY]),lpMyShopIcon,0 ,0 , 20,20);
			DrawSprite(ArrowPosi[8][0]-39, ArrowPosi[8][1]-27+(256-sinMoveKindInter[SIN_INVENTORY]),lpMyShopIcon_T,0 ,0 , 100,26);

		break;
		case 10:	// the costume view arrow: lit, with its "Costume" label
			if(lpCostumeArrow) DrawSprite(ArrowPosi[9][0], ArrowPosi[9][1]+(256-sinMoveKindInter[SIN_INVENTORY]),lpCostumeArrow,0 ,0 , 20,20);
			if(lpCostumeInfo) DrawSprite(ArrowPosi[9][0]-30, ArrowPosi[9][1]-27+(256-sinMoveKindInter[SIN_INVENTORY]),lpCostumeInfo,0 ,0 , 80,27);
		break;

	}

	if(sinSkill.pLeftSkill){		 //왼쪽 스킬 
		if(sinSkill.pLeftSkill->UseSkillFlag)
			DrawSprite(sLeftRightSkill[0].BoxRect.left , sLeftRightSkill[0].BoxRect.top , //버튼 
					cSkill.lpSkillButton[sinSkill.pLeftSkill->Position-1],0,0,40,40);
		else
			DrawSprite(sLeftRightSkill[0].BoxRect.left , sLeftRightSkill[0].BoxRect.top , //버튼 
					cSkill.lpSkillButton_Gray[sinSkill.pLeftSkill->Position-1],0,0,40,40);

	}
	if(sinSkill.pRightSkill){		//오른쪽 스킬  
		if(sinSkill.pRightSkill->UseSkillFlag)
			DrawSprite(sLeftRightSkill[1].BoxRect.left , sLeftRightSkill[1].BoxRect.top , //버튼 
					cSkill.lpSkillButton[sinSkill.pRightSkill->Position-1],0,0,40,40);
		else
			DrawSprite(sLeftRightSkill[1].BoxRect.left , sLeftRightSkill[1].BoxRect.top , //버튼 
					cSkill.lpSkillButton_Gray[sinSkill.pRightSkill->Position-1],0,0,40,40);
	}


	//유니온 코어
	//파티클
	for(i = 0 ; i < 100; i++){  
		if(UnionCore_Effect[i].Flag){
			cInvenTory.sinDrawInvenTexImage(&UnionCore_Effect[i].InvenFace);
		}
	}
	
	for( i = 0 ; i < 50 ; i++){ 
		if(UnionCore_Effect_Bomb[i].Flag){
			cInvenTory.sinDrawInvenTexImage(&UnionCore_Effect_Bomb[i].InvenFace);


		}
	}
	//////////개인 상점
	if(IndiShopIconFlag){     
		DrawSprite(225,570,lpIndiShopIcon,0,0,27,25);
		DrawSprite(225-35,570-27,cShop.lpMyShop_T,0,0,100,26);

	}

	
}
/*----------------------------------------------------------------------------*
*							     메인  
*-----------------------------------------------------------------------------*/	
int InvenMainCount = 0;   // 인벤토리 메인 카운트 
int SetDelayFlag = 0;     // 셋팅 딜레이 플렉 
bool bGiftBoxDelay = false;	// pluto 선물상자
DWORD dwUsePotionDelayTime = 0;
DWORD dwCheckHackCharStateTime = 0;
DWORD dwCheckHackCharStateFlag = 0;
DWORD dwCheckPickUpItemTime = 0;
int	  CharToItemFirstFlag = 0;
DWORD CharToItemFirstTime = 0;
DWORD dwUseCristalDelayTime  = 0;
int   sinUseCristalDelayFlag = 0;


DWORD dwCheckAutoMouseTime = 0;

int   CheckPosiFlag = 0;
#define   CHECKAUTOMOUSE_DISTANCE		(200*200)
#define   CHECKAUTOMOUSE_SETITEM_MAX	500
#define   CHECKAUTOMOUSE_TIME			(70*60*30)

//#define   CHECKAUTOMOUSE_SETITEM_MAX	100
//#define   CHECKAUTOMOUSE_TIME			(70*60*10)

int   CheckX = 0 , CheckZ = 0;
int   CheckResultX = 0 , CheckResultZ = 0;

DWORD dwPuzzleOkTime = 0;
int ReversEffectFlag = 0;
DWORD dwForceOrbTime = 0;
int   ForceOrbEffectIndex = 0;
void cINVENTORY::Main()
{		
	dwForceOrbTime++; 
	if(dwForceOrbTime > 8){   
		dwForceOrbTime = 0;
		ForceOrbEffectIndex++;
		if(ForceOrbEffectIndex > 7)
			ForceOrbEffectIndex = 0;
	}

	if(PuzzleOkFlag){    
		dwPuzzleOkTime++;
		if(dwPuzzleOkTime >2){ 
			dwPuzzleOkTime = 0;
			if(PuzzleOkY  > 82)ReversEffectFlag=1;
			if(ReversEffectFlag){
				PuzzleOkY --;
				if(PuzzleOkY <=0 )
					ReversEffectFlag = 0;
			}
			else PuzzleOkY ++;

		}
	}
	else{
		PuzzleOkY   = 0;
		dwPuzzleOkTime = 0;
	}
	//////////테스트
//	sInven[10].Rect.left = 499;   
//	sInven[10].Rect.top  = 575;


	int dist = 0;
	//////////오토마우스를 체크한다
	if(!CheckPosiFlag){
		CheckX = lpCurPlayer->pX;
		CheckZ = lpCurPlayer->pZ;
		CheckPosiFlag = 1;
	}

	dwCheckAutoMouseTime++;
	if(dwCheckAutoMouseTime >= CHECKAUTOMOUSE_TIME){
		if(sinOverWeightCnt > CHECKAUTOMOUSE_SETITEM_MAX){
			CheckResultX = ( lpCurPlayer->pX-CheckX )>>FLOATNS;
			CheckResultZ = ( lpCurPlayer->pZ-CheckZ )>>FLOATNS;
			dist   = CheckResultX*CheckResultX+CheckResultZ*CheckResultZ;
			if(CHECKAUTOMOUSE_DISTANCE >=dist){
				//신고 
				sinOverWeightCnt = 0;
				CheckPosiFlag = 0;
				dwCheckAutoMouseTime = 0;
			}
			else{
				//무게 초과나 공간초과 카운트는 넘었으나 캐릭터 이동거리가 클경우는 리셋한다
				sinOverWeightCnt = 0;
				CheckPosiFlag = 0;
				dwCheckAutoMouseTime = 0;
			}

		}
		else{
			sinOverWeightCnt = 0;
			CheckPosiFlag = 0;
			dwCheckAutoMouseTime = 0;
		}

	}

	if(sinUseCristalDelayFlag){
		dwUseCristalDelayTime++;
		if(dwUseCristalDelayTime >70){
			sinUseCristalDelayFlag = 0;
			dwUseCristalDelayTime  = 0;
		}
	}

	UnionCoreParticleProcess();
	//해킹방지를 위해서 30초마다 한번씩 체크한다
	dwCheckHackCharStateTime++; 
	if(dwCheckHackCharStateTime >= 30*70){
		SetItemToChar(TRUE);
		dwCheckHackCharStateTime = 0;

	}


	if(sinUsePotionDelayFlag){
		dwUsePotionDelayTime++;
		if(dwUsePotionDelayTime > 50){
			dwUsePotionDelayTime = 0;
			sinUsePotionDelayFlag = 0;

		}

	}

	if(SetDelayFlag){ //셋팅할때 Delay시간을 준다 
		InvenMainCount++;	
		if(InvenMainCount >=70){ //1초 딜레이를 준다 
			SetDelayFlag = 0;
			InvenMainCount = 0;
		}

	}


	// 장별 - 입력창
	if(nName){ 
		BackUpButton2X = 35;    
		BackUpButton2Y = 157;
		BackUpButton3X = 115;
		BackUpButton3Y = 157;

		BackUpPosX = BackStartPos.x;
		BackUpPosY = BackStartPos.y;

		// 예 버튼에 마우스 
		if(pCursorPos.x >= BackStartPos.x+BackUpButton2X && pCursorPos.x <= BackStartPos.x+BackUpButton2X + 48 &&
		pCursorPos.y >= BackStartPos.y+BackUpButton2Y - 30   && pCursorPos.y <= BackStartPos.y+BackUpButton2Y- 7)
		{
			nYesButton = 1;
		}

		// 아니오 버튼에 마우스 
		if(pCursorPos.x >= BackStartPos.x+BackUpButton3X && pCursorPos.x <= BackStartPos.x+BackUpButton3X + 48 &&
		pCursorPos.y >= BackStartPos.y+BackUpButton2Y - 30   && pCursorPos.y <= BackStartPos.y+BackUpButton2Y- 7)
		{
			
			nNoButton = 1;
		}

		BackStartPos.x = BackUpPosX;
		BackStartPos.y = BackUpPosY;

	}

	// pluto 선물상자
	if( bGiftBoxDelay )
	{
		InvenMainCount++;
		if(InvenMainCount >= 140 )
		{
			bGiftBoxDelay = false;
			InvenMainCount = 0;
		}
	}
	InvenArrowPosi = 0; //리턴하기전에 값을 초기화해준다 
	int i = 0;
	if(OpenFlag)
		for(i=0;i<10 ;i++)		// 10: the costume view arrow
			if ( ArrowPosi[i][0] < pCursorPos.x  && ArrowPosi[i][2] > pCursorPos.x  &&
				 ArrowPosi[i][1] < pCursorPos.y  && ArrowPosi[i][3] > pCursorPos.y  )
				InvenArrowPosi = i+1;

	//Pet메세지
	for(i=0;i<3 ;i++)
		if ( sInven[i+10].Rect.left < pCursorPos.x  && sInven[i+10].Rect.right > pCursorPos.x  && 
			sInven[i+10].Rect.top < pCursorPos.y  && sInven[i+10].Rect.bottom > pCursorPos.y  ){
				cHelpPet.PetMessage("*Potion_ShortKey",0);

			}
				



	PickUpInvenItem(pCursorPos.x , pCursorPos.y); //집을수 있는 아이템을 표시한다 
	
	int BackdsDrawOffsetArray = 0;
	if(MouseItem.Flag){
		SetInvenItemAreaCheck(&MouseItem); 
		sinItemPickUpDelayFlag =1;

	}
	if(sinItemPickUpDelayFlag){
		dwCheckPickUpItemTime++;
		if(dwCheckPickUpItemTime >  70){
			sinItemPickUpDelayFlag = 0;
			dwCheckPickUpItemTime = 0;
		}

	}
	else{
		sinItemPickUpDelayFlag = 0;
		dwCheckPickUpItemTime = 0;
	}

	if(!PotionCheckNumJumpFlag)
		CheckPotionNumForm();

	//인벤토리 이펙트 즐~
	if(OpenFlag){
		//ShowInvenEffectParticle(); //인벤토리 파티클을 셋팅한다  
		//MoveInvenParticle(); //인벤토르 파티클을 움직인다 
	}

	//캐릭터 능력보정을 위해 시작후 한번 보정해준다 
	if(!CharToItemFirstFlag){
		CharToItemFirstTime++;
		if(CharToItemFirstTime >= 70*3){
			SetItemToChar(0); 
			SetItemToChar(); 
			CharToItemFirstFlag = 1;
		}
	}

}
/*----------------------------------------------------------------------------*
*							     종료 
*-----------------------------------------------------------------------------*/	
void cINVENTORY::Close()
{
	Release();	
}
/*----------------------------------------------------------------------------*
*							    Down
*-----------------------------------------------------------------------------*/	
#include "..\\language.h"
int sinButtonFlag=0;
void cINVENTORY::LButtonDown(int x , int y)
{

	// 장별 - 입력창
	if( nYesButton == 1 )
	{
		lstrcpy(cInvenTory.szSendDoc,cInvenTory.szDoc);
		nName = 0;
		isDrawClanMenu = 0;
		SetIME_Mode(0);
		cInterFace.ChatFlag = 0;
		sinChatEnter = 0;
		sinMessageBoxShowFlag = 0;
		hFocusWnd = 0;  
		cInterFace.ChatFlag = 0;
		SendsServerDoc( cInvenTory.szSendDoc );
	}

	if( nNoButton == 1 )
	{
		nName = 0;
		isDrawClanMenu = 0;
		SetIME_Mode(0);
		cInterFace.ChatFlag = 0;
		sinChatEnter = 0;
		sinMessageBoxShowFlag = 0;
		hFocusWnd = 0;  
		cInterFace.ChatFlag = 0;
	}

	//PetMessage
	int i = 0;
	for(i=0;i<3 ;i++)
		if ( sInven[i+10].Rect.left < pCursorPos.x  && sInven[i+10].Rect.right > pCursorPos.x  && 
			sInven[i+10].Rect.top < pCursorPos.y  && sInven[i+10].Rect.bottom > pCursorPos.y  ){
				cHelpPet.PetMessage("*Potion_ShortKey",1);

			}

//#ifdef _TEST_SERVER
	//개인상점
	if(InvenArrowPosi == 9){
		if(!MouseItem.Flag){
			cInterFace.CheckAllBox(SIN_MYSHOP);
		}

	}
//#endif 
	//if(IndiShopIconFlag){
	//	cInterFace.CheckAllBox(SIN_MYSHOP);
	//} 

	if( bGiftBoxDelay ) return;		// pluto 선물상자
	// the up arrow: the costume view on/off (the T key does the same)
	if(InvenArrowPosi == 10){
		if(!MouseItem.Flag){
			sinToggleCostumeView();
			sinPlaySound(SIN_SOUND_SHOW_INTER);
		}
		return;
	}
	//퀘스트 아이템
	if(InvenArrowPosi == 8){
		if(!cTrade.OpenFlag && !cWareHouse.OpenFlag && !cCraftItem.OpenFlag && !cMyShop.OpenFlag && !cCharShop.OpenFlag && !MyShopSendButton) //에이징은 열리게한다 워쩔수엄따
			ChangeSpecialItem();

	}
	if(InvenArrowPosi == 5){
		OpenFlag = 0;

	}
	if(InvenArrowPosi == 7){ //돈 메세지 박스가 뜬다  
		if(!MouseItem.Flag){ //마우스에 아이템이 없을 경우에만 보여준다 
			//돈을 버리는 창을 제한한다
			if(!cTrade.OpenFlag && !cMyShop.OpenFlag && !MyShopSendButton && !cCharShop.OpenFlag && !sSinWarpGate.ShowFlag) 
				cMessageBox.ShowMessage2(MESSAGE_THROW_MONEY);
			if(cWareHouse.OpenFlag) //창고가 떠있으면 
				cMessageBox.ShowMessage2(MESSAGE_MOVE_MONEY_MOVE);

		}
	}

	if(InvenArrowPosi && InvenArrowPosi < 5){ //셋팅을 바꾼다 
		if(!cTrade.OpenFlag){ //트레이드하고있으면 안됀다 
			if(sinButtonFlag == InvenArrowPosi)return;
				sinButtonFlag = InvenArrowPosi;
				if(InvenArrowPosi <= 2)
					ChangeABItem(InvenArrowPosi);
				if(InvenArrowPosi >= 3)
					sinChangeSetFlag = InvenArrowPosi;
		}

	}
	// pluto 제작 AB체인지 막는다
	if( sinCraftItemStartFlag || StartInterEffectAging2 )
	{
		return;
	}
	else if( ( ManufactureItem.m_OpenFlag || SmeltingItem.OpenFlag ) && MixCancelButtonDelayFlag )
	{
		return;
	}
	if(MouseItem.Flag){
		if(cInvenTory.OpenFlag && MouseItem.sItemInfo.SpecialItemFlag[1] == ITEM_KIND_SPECIAL){
			if(ColorIndex == OVERLAP_ITEM_COLOR || ColorIndex == SET_ITEM_CHECK_COLOR ){
				if(!SpecialItemShowFlag){
						cInvenTory.UnionCoreParticle2(MouseItem.x , MouseItem.y); //이펙트 
						sinPlaySound(SIN_SOUND_EAT_POTION2); //유니온코어 사운드
				}
				SetSpecialItem(&MouseItem); //스페셜아이템박스에 넣는다
				MouseItem.Flag = 0;
				ReFormInvenItem();
				ReformCharForm();//재인증 
				return;
			}				
			
		}

		//퀘스트 아이템 셋팅을 제한한다
		/*
		if(MouseItem.ItemPosition == 2 && MouseItem.Class == ITEM_CLASS_WEAPON_TWO){
			if(sInven[0].ItemIndex && InvenItem[sInven[0].ItemIndex-1].sItemInfo.ItemKindCode == ITEM_KIND_QUEST_WEAPON&&
				InvenItem[sInven[0].ItemIndex-1].sItemInfo.ItemAgingNum[0] <4 )
				return;
		}
		*/
		if(SpecialItemShowFlag)return; //퀘스트아이템창에서는 셋팅할수엄따
		if(SetDelayFlag)return; //딜레이 가 있을때는 실행하지 않는다  
		
		//워프게이트 실행시는 아이템을 버리지않는다
		if(!sSinWarpGate.ShowFlag )
			ThrowInvenItemToField(&MouseItem);//아이템을 버린다
		//if(!MouseItem.Flag)return; //아이템이 버려지면 리턴한다 

		if(ColorIndex == NO_BOX_COLOR || ColorIndex == NOT_SETTING_COLOR)return; //셋팅될수없으면 리턴한다 
		if(!CheckSetOk(&MouseItem)) //셋팅 할수있는지를 체크한다 
			return;
		if(MouseItem.Class == ITEM_CLASS_POTION){ //포션일 경우는 따로 처리한다 
			if(!MouseSetPotion(&MouseItem)){
				if(MouseItem.Flag){
					NoSettingThrowItem(&MouseItem);// 셋팅이 안될때는 바닥에 아이템을 버린다 
				}

			}
			else
				CheckWeight(); //포션을 넣은다음 무게를 체크한다 
			
		}
		else{
			if(!ChangeInvenItem(&MouseItem)){  //아이템을 바꾼다  
				if(MouseItem.ItemPosition ){ //겹치기 방지를 위해서 한번더 체크 
					/*
					if((InvenItem[sInven[MouseItem.ItemPosition-1].ItemIndex-1].sItemInfo.ItemKindCode == ITEM_KIND_QUEST_WEAPON && InvenItem[sInven[MouseItem.ItemPosition-1].ItemIndex-1].sItemInfo.ItemAgingNum[0] < 4)||
						(InvenItem[AutoSetItemIndex-1].sItemInfo.ItemKindCode == ITEM_KIND_QUEST_WEAPON && InvenItem[AutoSetItemIndex-1].sItemInfo.ItemAgingNum[0] < 4)){
						cMessageBox.ShowMessage(MESSAGE_NOT_PICKUP_ITEM); //퀘스트아이템의 셋팅을 바꿀수없다
						return;
					}
					
					*/
					if(sInven[MouseItem.ItemPosition-1].ItemIndex && InvenItem[sInven[MouseItem.ItemPosition-1].ItemIndex-1].Flag){
						return;
					}
					

				}
				if(!LastSetInvenItem(&MouseItem)){ //아이템이 바뀌지 않았으면 그냥 인벤토리에 셋팅한다
					if(MouseItem.Flag){ //버릴수없는 아이템을 체크한다 
						if((MouseItem.sItemInfo.CODE & sinITEM_MASK2) == sinMA1 ||
							(MouseItem.sItemInfo.CODE & sinITEM_MASK2) == sinMA2 ||
							(MouseItem.sItemInfo.CODE & sinITEM_MASK2) == sinQT1 ||
							(MouseItem.sItemInfo.CODE & sinITEM_MASK2) == sinQW1 ){
				
						}
						else{
							NoSettingThrowItem(&MouseItem);// 셋팅이 안될때는 바닥에 아이템을 버린다 
						}
					}
				}
			}
		}
	}
	else{
		//if(SpecialItemShowFlag)return; //퀘스트아이템창에서는 셋팅할수엄따
		////////////모션중이면 아이템을 집을수엄따!! 
		if(lpCurPlayer->MotionInfo->State == CHRMOTION_STATE_ATTACK ||
			lpCurPlayer->MotionInfo->State == CHRMOTION_STATE_SKILL)return;

		pMessageItem = 0;  //포인터 초기화 	
		CheckInvenItemForm(); //아이템을 집기전에 체크한다 
		if(cMyShop.OpenFlag){  
			PickUpInvenItem(pCursorPos.x,pCursorPos.y,2);//등록할 아이템을 집는다
		}
		else{
			if(!MyShopSendButton && !cCharShop.OpenFlag && !sSinWarpGate.ShowFlag){
				PickUpInvenItem(pCursorPos.x,pCursorPos.y,1);//아이템을 집는다 
			}
		}

		ReFormInvenItem();
		if(MouseItem.Flag){
			if(MouseItem.Class != ITEM_CLASS_POTION){
				CopyItemClear(&MouseItem); //복사된 아이템을 없앤다  (자신의 아이템은 마우스로 옮겨져있다)
			}
		}
	}
	if(MouseItem.Flag) //인벤토리에서 아이템을 잡았을경우에는 트레이드 마우스다운을 건너뛴다	
	LbuttonUpCheckFlag = 1; 

			
}
/*----------------------------------------------------------------------------*

*							   LButtonUp
*-----------------------------------------------------------------------------*/	
void cINVENTORY::LButtonUp(int x , int y)
{
	LbuttonUpCheckFlag = 0;
	sinMyShopItemPass  = 0; //이벤트를 한번 건너뛰기위한 플랙
}
/*----------------------------------------------------------------------------*
*							   RButtonDown
*-----------------------------------------------------------------------------*/

DWORD dwBoosterItemCode[] = {sin21,sin22,sin23,sin24,sin25,sin26,sin27,sin28,sin29,0}; // 박재원 - 부스터 아이템
DWORD dwCastlItemCODE[]={sin01,sin02,sin03,sin04,sin05,sin06,sin07,sin08,sin09,sin10,sin11,sin12,sin13,sin14,sin15,sin16,0};
DWORD dwNotUseVillageItemCODE[3]={(sinBI1|sin05),(sinBI1|sin06),(sinBI1|sin07)};
DWORD dwNotUseItemCODE[6][3]={{SCROLL_INVULNERABILITY,(sinBC1|sin01),1},{SCROLL_CRITICAL,(sinBC1|sin02),2},{SCROLL_EVASION,(sinBC1|sin03),3},
							  {SCROLL_P_INVULNERABILITY,(sinBI1|sin05),1},{SCROLL_P_CRITICAL,(sinBI1|sin06),2},{SCROLL_P_EVASION,(sinBI1|sin07),3}};
DWORD dwSkillDelayItemCode[] = {sin30,sin31,sin32, 0}; // 장별 - 스킬 딜레이
void cINVENTORY::RButtonDown(int x , int y)
{
	
	if( MouseItem.Flag || sinItemPickUpDelayFlag)return ;		//우클릭시 계속 사용되는 버그 수정 
	
	//Pet메세지
	int i = 0;
	for(i=0;i<3 ;i++) {
		if ( sInven[i+10].Rect.left < pCursorPos.x  && sInven[i+10].Rect.right > pCursorPos.x  && 
			sInven[i+10].Rect.top < pCursorPos.y  && sInven[i+10].Rect.bottom > pCursorPos.y  ){
				cHelpPet.PetMessage("*Potion_ShortKey",2);
			}
	}
	for(int j=0 ; j<SIN_MAX_HELP_NUMBER ; j++)				//텔레포트 핼프창이 열렸을테 우클릭을 막는다 (성근 추가 )
	{
		if(sSinHelp[j].KindFlag == SIN_HELP_KIND_TELEPORT_SCROLL)
		return ; 
	}
	//
	if( MouseItem.Flag )return ;
	char szBuff[128];
	if(cWareHouse.OpenFlag || cTrade.OpenFlag ||cCraftItem.OpenFlag || cShop.OpenFlag || cMixtureReset.OpenFlag || // 석지용 - 믹스쳐 리셋 추가
		cMyShop.OpenFlag || cCharShop.OpenFlag || SmeltingItem.OpenFlag || ManufactureItem.m_OpenFlag)return; 
	UsePotion(x , y);//포션을 사용한다  
	
	if(SelectInvenItemIndex){ //귀환 아아템 
		cHelpPet.PetMessage("*InvenTory_Item",2);
		//복주머니를 퍼즐로 바꾼다
		if((InvenItem[SelectInvenItemIndex-1].sItemInfo.CODE & sinITEM_MASK2) == sinPZ1 || (InvenItem[SelectInvenItemIndex-1].sItemInfo.CODE & sinITEM_MASK2) == sinPZ2){
			if(InvenItem[SelectInvenItemIndex-1].sItemInfo.PotionCount <= 1){
				if(sinChar->Money >= 1000){
					ChangePuzzle(&InvenItem[SelectInvenItemIndex-1]);
				}
				else
					cMessageBox.ShowMessage(MESSAGE_NOT_ENOUGH_MONEY);
			}
		}
		//Force Orb를 사용한다
		if((InvenItem[SelectInvenItemIndex-1].CODE &sinITEM_MASK2 ) == sinFO1)
		{
			if( ((InvenItem[SelectInvenItemIndex-1].CODE &sinITEM_MASK3 ) >= sin01) && ((InvenItem[SelectInvenItemIndex-1].CODE &sinITEM_MASK3 ) <= sin12) )
				UseForceOrb(&InvenItem[SelectInvenItemIndex-1]);
			else if( ((InvenItem[SelectInvenItemIndex-1].CODE &sinITEM_MASK3 ) >= sin21) && ((InvenItem[SelectInvenItemIndex-1].CODE &sinITEM_MASK3 ) <= sin32) )
				UseMagicForceOrb(&InvenItem[SelectInvenItemIndex-1]); // 박재원 - 매직 포스 추가(매직 포스를 사용한다)
			else if( ((InvenItem[SelectInvenItemIndex-1].CODE &sinITEM_MASK3 ) >= sin35) && ((InvenItem[SelectInvenItemIndex-1].CODE &sinITEM_MASK3 ) <= sin37) )
				UseBillingMagicForceOrb(&InvenItem[SelectInvenItemIndex-1]); // 박재원 - 빌링 매직 포스 추가(빌링 매직 포스를 사용한다.
		}


		if((InvenItem[SelectInvenItemIndex-1].CODE &sinITEM_MASK2 ) == sinEC1){
			UseEcore(&InvenItem[SelectInvenItemIndex-1]);

		}
		if((InvenItem[SelectInvenItemIndex-1].CODE &sinITEM_MASK2 ) == sinSP1){
			// pluto 커플링
			if( InvenItem[ SelectInvenItemIndex - 1 ].CODE == (sinSP1|sin16) )
			{
				return;
			}
			// pluto 선물상자
			if(  InvenItem[ SelectInvenItemIndex - 1 ].CODE == (sinSP1|sin05) || 
				 InvenItem[ SelectInvenItemIndex - 1 ].CODE == (sinSP1|sin06) ||
				 InvenItem[ SelectInvenItemIndex - 1 ].CODE == (sinSP1|sin07) ||
				 InvenItem[ SelectInvenItemIndex - 1 ].CODE == (sinSP1|sin08) ||
				 InvenItem[ SelectInvenItemIndex - 1 ].CODE == (sinSP1|sin09) ||
				 InvenItem[ SelectInvenItemIndex - 1 ].CODE == (sinSP1|sin10) )
			{
				sITEM temp;
				temp = InvenItem[SelectInvenItemIndex-1];
				if( !CheckInvenEmpty( &temp ) )
				{
					cMessageBox.ShowMessage(MESSAGE_OVER_SPACE); //공간부족
					return;
				}
				if( sinChar->Weight[0] + 10 >= sinChar->Weight[1] )
				{
					cMessageBox.ShowMessage(MESSAGE_OVER_WEIGHT); //무게초과
					return;
				}
				bGiftBoxDelay = true;
				if(sinActionPotion()){
					pUsePotion = &InvenItem[SelectInvenItemIndex-1]; //사용할 송편을 백업한다
					sinUsePotionDelayFlag = 1;
				}
			}
			else if( InvenItem[ SelectInvenItemIndex - 1 ].CODE == (sinSP1|sin34) )  // 박재원 - 호랑이 캡슐 사용
			{	
				UseCapsuleItem( &InvenItem[ SelectInvenItemIndex - 1 ] );
			}
			else if( InvenItem[ SelectInvenItemIndex - 1 ].CODE == (sinSP1|sin15) ) // 박재원 - 수박 아이템 사용
			{	
				if(sinActionPotion()){
					pUsePotion = &InvenItem[SelectInvenItemIndex-1];
				}
			}
			else if( InvenItem[ SelectInvenItemIndex - 1 ].CODE == (sinSP1|sin26) ) // 박재원 - 밤하늘의 소원이벤트 // 박재원 - 별조각 아이템 사용
			{
				return;
			}
			else if( InvenItem[ SelectInvenItemIndex - 1 ].CODE == (sinSP1|sin27) || InvenItem[ SelectInvenItemIndex - 1 ].CODE == (sinSP1|sin28) ||
			 	  	 InvenItem[ SelectInvenItemIndex - 1 ].CODE == (sinSP1|sin29) || InvenItem[ SelectInvenItemIndex - 1 ].CODE == (sinSP1|sin30) ||
			 		 InvenItem[ SelectInvenItemIndex - 1 ].CODE == (sinSP1|sin31) || InvenItem[ SelectInvenItemIndex - 1 ].CODE == (sinSP1|sin32) ||
					 InvenItem[ SelectInvenItemIndex - 1 ].CODE == (sinSP1|sin33) ) // 박재원 - 알파벳 조합 이벤트
			{
				return;
			}

			else if( InvenItem[ SelectInvenItemIndex - 1 ].CODE == (sinSP1|sin35) ) // 장별 - 발렌타인 초콜렛 아이템 사용
			{	
				if(sinActionPotion()){
					pUsePotion = &InvenItem[SelectInvenItemIndex-1];
				}
			}

			else if( InvenItem[ SelectInvenItemIndex - 1 ].CODE == (sinSP1|sin36) ) // 장별 - 캔디데이즈 캔디아이템 사용
			{	
				if(sinActionPotion()){
					pUsePotion = &InvenItem[SelectInvenItemIndex-1];
				}
			}

			else if( InvenItem[ SelectInvenItemIndex - 1 ].CODE == (sinSP1|sin37) ||  InvenItem[ SelectInvenItemIndex - 1 ].CODE == (sinSP1|sin38) ) // 장별 - 매지컬그린
			{
				return;
			}

			else if( InvenItem[ SelectInvenItemIndex - 1 ].CODE == (sinSP1|sin39) ) // 장별 - 카라의 눈물
			{
				return;
			}

			// 박재원 - 2010 월드컵 이벤트 - 축구공 포션
			else if( InvenItem[ SelectInvenItemIndex - 1 ].CODE == (sinSP1|sin40) )
			{
				if(lpCurPlayer->smCharInfo.dwEventTime_T) // 박재원 - 축구공 포션 시간이 남아 있으면
				{
					cMessageBox.ShowMessage( MESSAGE_ASATTRIBUTE_USE );
					return;
				}
				UseSoccerBallPotionItem( &InvenItem[ SelectInvenItemIndex - 1 ] );
			}


			else if( InvenItem[ SelectInvenItemIndex - 1 ].CODE == (sinSP1|sin42) ) // 장별 - 수박
			{	
				if(sinActionPotion()){
					pUsePotion = &InvenItem[SelectInvenItemIndex-1];
				}
			}

			else
			{
				//EatSongPyeun(&InvenItem[SelectInvenItemIndex-1]);
				if(sinActionPotion()){
					pUsePotion = &InvenItem[SelectInvenItemIndex-1]; //사용할 송편을 백업한다 

				}
			}
		}
		/////////////몬스터 크리스탈
		if((InvenItem[SelectInvenItemIndex-1].CODE &sinITEM_MASK2 ) == sinGP1){ 
			//몬스터 크리스탈은 딜레이를 준다.
			if(InvenItem[SelectInvenItemIndex-1].sItemInfo.CODE  == (sinGP1|sin17) ||
			   InvenItem[SelectInvenItemIndex-1].sItemInfo.CODE  == (sinGP1|sin18) ||
			   InvenItem[SelectInvenItemIndex-1].sItemInfo.CODE  == (sinGP1|sin19) ||
			   InvenItem[SelectInvenItemIndex-1].sItemInfo.CODE  == (sinGP1|sin20) ){ // 박재원 - 익스트림 크리스탈 추가
				if(chaPremiumitem.haItemUseDelayFlag){
					cMessageBox.ShowMessage(MESSAGE_NOT_DELAYTIME);
					return;
				}
			}
			if(InvenItem[SelectInvenItemIndex-1].sItemInfo.Level > sinChar->Level)return;
	
			//클랜 크리스탈은 클랜원만 사용할수있다 
			if(InvenItem[SelectInvenItemIndex-1].sItemInfo.CODE  == (sinGP1|sin10) ||
				InvenItem[SelectInvenItemIndex-1].sItemInfo.CODE  == (sinGP1|sin22) ){	// pluto 가디안 디바인
				if(!sinChar->ClassClan || cldata.stats[0] != '1'){
					cMessageBox.ShowMessage(MESSAGE_USE_REGULAR_CLAN_ONLY);
					return;
				}
				else{
					if(!sinUseCristalDelayFlag)sinUseCristalDelayFlag = 1;
					else return;
				}
			}
			if(!sinItemPickUpDelayFlag){ //무한으로 사용할수있음을 방지한다
				InvenItem[SelectInvenItemIndex-1].sItemInfo.PotionCount = 100;
				sinThrowItemToFeild(&InvenItem[SelectInvenItemIndex-1]);
				InvenItem[SelectInvenItemIndex-1].Flag = 0;
				cInvenTory.CheckWeight();//무게를 보정해준다
				ReFormInvenItem();
				chaPremiumitem.haItemUseDelayFlag = true;
				
			}
		}


		 // 장별 - 소울스톤
		if((InvenItem[SelectInvenItemIndex-1].CODE &sinITEM_MASK2 ) == sinGP2 )
		{ 

			if(chaPremiumitem.haItemUseDelayFlag)
			{
				cMessageBox.ShowMessage(MESSAGE_NOT_DELAYTIME);
				return;
			}

			if(InvenItem[SelectInvenItemIndex-1].sItemInfo.Level > sinChar->Level)return;
			
			if(!sinItemPickUpDelayFlag){ //무한으로 사용할수있음을 방지한다
			InvenItem[SelectInvenItemIndex-1].sItemInfo.PotionCount = 100;
			sinThrowItemToFeild(&InvenItem[SelectInvenItemIndex-1]);
			InvenItem[SelectInvenItemIndex-1].Flag = 0;
			cInvenTory.CheckWeight();//무게를 보정해준다
			ReFormInvenItem();
			chaPremiumitem.haItemUseDelayFlag = true;
			
		}
	}

		//같이 사용돼면 안돼는 아이템을 체크해서 리턴시킨다.
		for( i=0;i<6;i++){							
			if(cSkill.SearchContiueSkillCODE(dwNotUseItemCODE[i][0])){
				for(int j=0;j<6;j++){
					if(InvenItem[SelectInvenItemIndex-1].CODE == dwNotUseItemCODE[j][1] && dwNotUseItemCODE[j][1] != dwNotUseItemCODE[i][1]){
						if(dwNotUseItemCODE[j][2] == dwNotUseItemCODE[i][2]){
							cMessageBox.ShowMessage(MESSAGE_ASATTRIBUTE_USE);
							return ;
						}
					}
				}
			}
		}
		
		/////////공성전 전용 아이템 
		if((InvenItem[SelectInvenItemIndex-1].CODE &sinITEM_MASK2 ) == sinBC1)
		{ 
			if(lpCurPlayer->OnStageField>=0 && StageField[lpCurPlayer->OnStageField]->FieldCode!=rsCASTLE_FIELD &&
				(InvenItem[SelectInvenItemIndex-1].CODE &sinITEM_MASK3)<=sin16)  // 박재원 - 부스터 아이템은 공성필드외에도 사용할 수 있다.
			{
				return ;
			}
		
			if(InvenItem[SelectInvenItemIndex-1].sItemInfo.CODE==(sinBC1|sin05)){
				return ;
			}
			for(i = 0; i < 16 ; i++){ 
				if((InvenItem[SelectInvenItemIndex-1].CODE&sinITEM_MASK3)== dwCastlItemCODE[i]){
					chaSiege.SetCastleItemSkill(i+1);	
					SetItemToChar();
					break;
				}
			}

			// 박재원 - 부스터 아이템
			if( ((InvenItem[SelectInvenItemIndex-1].CODE &sinITEM_MASK3 ) >= sin21) && ((InvenItem[SelectInvenItemIndex-1].CODE &sinITEM_MASK3 ) <= sin29)) // 장별 - 스킬 딜레이
				cSkill.UseBoosterItem(&InvenItem[SelectInvenItemIndex-1]);

			// 장별 - 스킬 딜레이
			if( ((InvenItem[SelectInvenItemIndex-1].CODE &sinITEM_MASK3 ) >= sin30))
				cSkill.UseSkillDelayItem(&InvenItem[SelectInvenItemIndex-1]);

			if(haCastleSkillUseFlag!=1)return; //사용중인 아이템이다.

			if(!sinItemPickUpDelayFlag){ //무한으로 사용할수있음을 방지한다
				InvenItem[SelectInvenItemIndex-1].sItemInfo.PotionCount = 200;
				sinThrowItemToFeild(&InvenItem[SelectInvenItemIndex-1]);
				InvenItem[SelectInvenItemIndex-1].Flag = 0;
				cInvenTory.CheckWeight();//무게를 보정해준다
				ReFormInvenItem();
			}
			
		}
		if((InvenItem[SelectInvenItemIndex-1].CODE &sinITEM_MASK2 ) == sinBI1 || (InvenItem[SelectInvenItemIndex-1].CODE &sinITEM_MASK2 ) == sinBI2){
			chaPremiumitem.TelePortCore.ItemIndex=0;
			//마을에서 사용돼면 안돼는 아이템--------------------------------------------------------------------------------
			if(lpCurPlayer->OnStageField>= 0 && StageField[ lpCurPlayer->OnStageField ]->State == FIELD_STATE_VILLAGE){ 
				/*if( (InvenItem[SelectInvenItemIndex-1].CODE &sinITEM_MASK2 ) == sinBI2 )  
					return;*/
				for(int i=0;i<3;i++){
					if(InvenItem[SelectInvenItemIndex-1].sItemInfo.CODE == dwNotUseVillageItemCODE[i])
						return;
				}
			}
			//--------------------------------------------------------------------------------------------------------------
		
			//Contiue 초기화
			iEventBuyItemKind =0;
			//블루스톤 사용
			if(InvenItem[SelectInvenItemIndex-1].sItemInfo.CODE==(sinBI1|sin01))
				chaPremiumitem.UsePremiumItem(1);
			//레드스톤
			if(InvenItem[SelectInvenItemIndex-1].sItemInfo.CODE==(sinBI1|sin02))
				chaPremiumitem.UsePremiumItem(2);
			//그린스톤
			if(InvenItem[SelectInvenItemIndex-1].sItemInfo.CODE==(sinBI1|sin03))
				chaPremiumitem.UsePremiumItem(3);
			//부활주문서
			if(InvenItem[SelectInvenItemIndex-1].sItemInfo.CODE==(sinBI1|sin04))
				return;
			//이터널 라이프
			if(InvenItem[SelectInvenItemIndex-1].sItemInfo.CODE==(sinBI1|sin05))
				chaPremiumitem.UsePremiumItem(5);
			//페이틀 에지
			if(InvenItem[SelectInvenItemIndex-1].sItemInfo.CODE==(sinBI1|sin06))
			{
				for( i = 0 ; i < MAX_CONTINUE_SKILL ; i++)			 // pluto 어버트 스크롤 사용중이면 막는다
				{
					if( ContinueSkill[i].CODE == SCROLL_P_EVASION )
					{
						if( ContinueSkill[i].Flag )
						{
							cMessageBox.ShowMessage( MESSAGE_NO_USE_ITEM );
							return;
						}
					}
				}
				chaPremiumitem.UsePremiumItem(6);
			}
			//어버트 스크롤
			if(InvenItem[SelectInvenItemIndex-1].sItemInfo.CODE==(sinBI1|sin07))
			{
				for( i = 0 ; i < MAX_CONTINUE_SKILL ; i++)
				{
					if( ContinueSkill[i].CODE == SCROLL_P_CRITICAL )	// pluto 페이틀 스크롤 사용중이면 막는다
					{
						if( ContinueSkill[i].Flag )
						{
							cMessageBox.ShowMessage( MESSAGE_NO_USE_ITEM );
							return;
						}
					}
				}
				chaPremiumitem.UsePremiumItem(7);
			}
			//텔레포트 코어
			if(InvenItem[SelectInvenItemIndex-1].sItemInfo.CODE==(sinBI1|sin08)){
				chaPremiumitem.TelePortCore.ItemIndex = SelectInvenItemIndex-1;
				chaPremiumitem.UsePremiumItem(8);
			}
			//얼큰이 물약
			if(InvenItem[SelectInvenItemIndex-1].sItemInfo.CODE==(sinBI1|sin09)){
				chaPremiumitem.UsePremiumItem(9);
			}
			//에이징 스톤
			if(InvenItem[SelectInvenItemIndex-1].sItemInfo.CODE==(sinBI1|sin10))
				chaPremiumitem.UsePremiumItem(10);
			//코퍼 오어
			if(InvenItem[SelectInvenItemIndex-1].sItemInfo.CODE==(sinBI1|sin11))
				chaPremiumitem.UsePremiumItem(11);
			//써드 아이즈(1일)
			if(InvenItem[SelectInvenItemIndex-1].sItemInfo.CODE==(sinBI1|sin12))
			{
				if( chaPremiumitem.m_ThirdEyesTime > 0 )
				{
					cMessageBox.ShowMessage(MESSAGE_CLANSKILL_USE);
					return;
				}

				chaPremiumitem.UsePremiumItem(12);
			}
			//경험치 업 아이템(1일)
			if(InvenItem[SelectInvenItemIndex-1].sItemInfo.CODE==(sinBI1|sin13))
			{
				if( chaPremiumitem.m_ExpUpPotionTime > 0 )
				{
					cMessageBox.ShowMessage(MESSAGE_CLANSKILL_USE);
					return;
				}

				chaPremiumitem.UsePremiumItem(13);
			}
			//써드 아이즈(7일)
			if(InvenItem[SelectInvenItemIndex-1].sItemInfo.CODE==(sinBI1|sin14))
			{
				if( chaPremiumitem.m_ThirdEyesTime > 0 )
				{
					cMessageBox.ShowMessage(MESSAGE_CLANSKILL_USE);
					return;
				}

				chaPremiumitem.UsePremiumItem(14);
			}
			//경험치 업 아이템(7일)
			if(InvenItem[SelectInvenItemIndex-1].sItemInfo.CODE==(sinBI1|sin15))
			{
				if( chaPremiumitem.m_ExpUpPotionTime > 0 )
				{
					cMessageBox.ShowMessage(MESSAGE_CLANSKILL_USE);
					return;
				}

				chaPremiumitem.UsePremiumItem(15);
			}
			//헤어틴트 포션(A)
			if(InvenItem[SelectInvenItemIndex-1].sItemInfo.CODE==(sinBI1|sin16))
				chaPremiumitem.UsePremiumItem(16);
			//헤어틴트 포션(B)
			if(InvenItem[SelectInvenItemIndex-1].sItemInfo.CODE==(sinBI1|sin17))
				chaPremiumitem.UsePremiumItem(17);
			//헤어틴트 포션(C)
			if(InvenItem[SelectInvenItemIndex-1].sItemInfo.CODE==(sinBI1|sin18))
				chaPremiumitem.UsePremiumItem(18);
			// pluto 뱀피릭 커스핏 3시간
			if(InvenItem[SelectInvenItemIndex-1].sItemInfo.CODE==(sinBI1|sin19))
			{
				if( chaPremiumitem.m_VampiricCuspidTime > 0 )
				{
					cMessageBox.ShowMessage(MESSAGE_CLANSKILL_USE);
					return;
				}

				chaPremiumitem.UsePremiumItem(19);
			}
			// pluto 뱀피릭 커스핏 1일
			if(InvenItem[SelectInvenItemIndex-1].sItemInfo.CODE==(sinBI1|sin20))
			{
				if( chaPremiumitem.m_VampiricCuspidTime > 0 )
				{
					cMessageBox.ShowMessage(MESSAGE_CLANSKILL_USE);
					return;
				}

				chaPremiumitem.UsePremiumItem(20);
			}
			// pluto 마나 리차징 포션 3시간
			if(InvenItem[SelectInvenItemIndex-1].sItemInfo.CODE==(sinBI1|sin21))
			{
				if( chaPremiumitem.m_ManaRechargingPTime > 0 )
				{
					cMessageBox.ShowMessage(MESSAGE_CLANSKILL_USE);
					return;
				}

				chaPremiumitem.UsePremiumItem(21);
			}
			// pluto 마나 리차징 포션 1일
			if(InvenItem[SelectInvenItemIndex-1].sItemInfo.CODE==(sinBI1|sin22))
			{
				if( chaPremiumitem.m_ManaRechargingPTime > 0 )
				{
					cMessageBox.ShowMessage(MESSAGE_CLANSKILL_USE);
					return;
				}

				chaPremiumitem.UsePremiumItem(22);
			}
			//폭죽
			if(InvenItem[SelectInvenItemIndex-1].sItemInfo.CODE==(sinBI1|sin23))
			{
				// 장별 - 입력창
				nName = 1;

				sinChatEnter = 0; //임의로 글씨가 안보이게한다
				cInterFace.ChatFlag = 0;
			//	chaPremiumitem.UsePremiumItem(23);
			}

		
			//브론즈 패키지 아이템 (3시간 )		//성근추가
			if(InvenItem[SelectInvenItemIndex-1].sItemInfo.CODE==(sinBI2|sin01))
				chaPremiumitem.UsePremiumItem(24);
			//브론즈 패키지 아이템 (1일 )		
			if(InvenItem[SelectInvenItemIndex-1].sItemInfo.CODE==(sinBI2|sin02))
				chaPremiumitem.UsePremiumItem(25);
			//브론즈 패키지 아이템 (7일 )		
			if(InvenItem[SelectInvenItemIndex-1].sItemInfo.CODE==(sinBI2|sin03))
				chaPremiumitem.UsePremiumItem(26);
			//브론즈 패키지 아이템 (30일 )		
			if(InvenItem[SelectInvenItemIndex-1].sItemInfo.CODE==(sinBI2|sin04))
				chaPremiumitem.UsePremiumItem(27);

			//실버 패키지 아이템 (3시간 )		
			if(InvenItem[SelectInvenItemIndex-1].sItemInfo.CODE==(sinBI2|sin05))
				chaPremiumitem.UsePremiumItem(28);
			//실버 패키지 아이템 (1일 )		
			if(InvenItem[SelectInvenItemIndex-1].sItemInfo.CODE==(sinBI2|sin06))
				chaPremiumitem.UsePremiumItem(29);
			//실버 패키지 아이템 (7일 )		
			if(InvenItem[SelectInvenItemIndex-1].sItemInfo.CODE==(sinBI2|sin07))
				chaPremiumitem.UsePremiumItem(30);
			//실버 패키지 아이템 (30일 )		
			if(InvenItem[SelectInvenItemIndex-1].sItemInfo.CODE==(sinBI2|sin08))
				chaPremiumitem.UsePremiumItem(31);

			//골드  패키지 아이템 (3시간 )		
			if(InvenItem[SelectInvenItemIndex-1].sItemInfo.CODE==(sinBI2|sin09))
				chaPremiumitem.UsePremiumItem(32);
			//골드 패키지 아이템 (1일 )		
			if(InvenItem[SelectInvenItemIndex-1].sItemInfo.CODE==(sinBI2|sin10))
				chaPremiumitem.UsePremiumItem(33);
			//골드 패키지 아이템 (7일 )		
			if(InvenItem[SelectInvenItemIndex-1].sItemInfo.CODE==(sinBI2|sin11))
				chaPremiumitem.UsePremiumItem(34);
			//골드 패키지 아이템 (30일 )		
			if(InvenItem[SelectInvenItemIndex-1].sItemInfo.CODE==(sinBI2|sin12))
				chaPremiumitem.UsePremiumItem(35);
			//===================== 2차 헤어틴트 포션 ========= 성근 ===== 060526
			//헤어틴트 포션(D)
			if(InvenItem[SelectInvenItemIndex-1].sItemInfo.CODE==(sinBI2|sin13))
				chaPremiumitem.UsePremiumItem(36);
			//헤어틴트 포션(E)
			if(InvenItem[SelectInvenItemIndex-1].sItemInfo.CODE==(sinBI2|sin14))
				chaPremiumitem.UsePremiumItem(37);
			//헤어틴트 포션(F)
			if(InvenItem[SelectInvenItemIndex-1].sItemInfo.CODE==(sinBI2|sin15))
				chaPremiumitem.UsePremiumItem(38);
			//헤어틴트 포션(G)
			if(InvenItem[SelectInvenItemIndex-1].sItemInfo.CODE==(sinBI2|sin16))
				chaPremiumitem.UsePremiumItem(39);
			//헤어틴트 포션(H)
			if(InvenItem[SelectInvenItemIndex-1].sItemInfo.CODE==(sinBI2|sin17))
				chaPremiumitem.UsePremiumItem(40);

			//===================== 3차 헤어틴트 포션 ========= 성근 ===== 060809
			//헤어틴트 포션(I)
			if(InvenItem[SelectInvenItemIndex-1].sItemInfo.CODE==(sinBI2|sin18))
				chaPremiumitem.UsePremiumItem(41);
			//헤어틴트 포션(J)
			if(InvenItem[SelectInvenItemIndex-1].sItemInfo.CODE==(sinBI2|sin19))
				chaPremiumitem.UsePremiumItem(42);
			//헤어틴트 포션(K)
			if(InvenItem[SelectInvenItemIndex-1].sItemInfo.CODE==(sinBI2|sin20))
				chaPremiumitem.UsePremiumItem(43);
			//헤어틴트 포션(L)
			if(InvenItem[SelectInvenItemIndex-1].sItemInfo.CODE==(sinBI2|sin21))
				chaPremiumitem.UsePremiumItem(44);
			//헤어틴트 포션(M)
			if(InvenItem[SelectInvenItemIndex-1].sItemInfo.CODE==(sinBI2|sin22))
				chaPremiumitem.UsePremiumItem(45);

			//***********************베트남요청 경치 50%============================			
			// 1일 
			if(InvenItem[SelectInvenItemIndex-1].sItemInfo.CODE==(sinBI1|sin24))
				chaPremiumitem.UsePremiumItem(46);
			//7일
			if(InvenItem[SelectInvenItemIndex-1].sItemInfo.CODE==(sinBI1|sin25))
				chaPremiumitem.UsePremiumItem(47);
			

			// pluto 마이트 오브 아웰 1일
			if( InvenItem[ SelectInvenItemIndex - 1 ].sItemInfo.CODE == (sinBI1|sin26) )
			{
				chaPremiumitem.UsePremiumItem( 48 );
			}
			// pluto 마이트 오브 아웰 30일
			if( InvenItem[ SelectInvenItemIndex - 1 ].sItemInfo.CODE == (sinBI1|sin27) )
			{
				chaPremiumitem.UsePremiumItem( 49 );
			}
			// pluto 마나 리듀스 포션 1일
			if( InvenItem[ SelectInvenItemIndex -1 ].sItemInfo.CODE == (sinBI1|sin28) )
			{
				if( chaPremiumitem.m_ManaReducePotiontime > 0 ) return;

				chaPremiumitem.UsePremiumItem( 50 );
			}
			// pluto 마나 리듀스 포션 7일
			if( InvenItem[ SelectInvenItemIndex -1 ].sItemInfo.CODE == (sinBI1|sin29) )
			{
				if( chaPremiumitem.m_ManaReducePotiontime > 0 )
				{
					cMessageBox.ShowMessage(MESSAGE_CLANSKILL_USE);
					return;
				}

				chaPremiumitem.UsePremiumItem( 51 );
			}
			// pluto 브론즈 패키지2 3시간
			if( InvenItem[ SelectInvenItemIndex -1 ].sItemInfo.CODE == (sinBI2|sin26) )
			{
				chaPremiumitem.UsePremiumItem( 52 );
			}
			// pluto 브론즈 패키지2 1일
			if( InvenItem[ SelectInvenItemIndex -1 ].sItemInfo.CODE == (sinBI2|sin27) )
			{
				chaPremiumitem.UsePremiumItem( 53 );
			}
			// pluto 브론즈 패키지2 7일
			if( InvenItem[ SelectInvenItemIndex -1 ].sItemInfo.CODE == (sinBI2|sin28) )
			{
				chaPremiumitem.UsePremiumItem( 54 );
			}
			// pluto 실버 패키지2 3시간
			if( InvenItem[ SelectInvenItemIndex -1 ].sItemInfo.CODE == (sinBI2|sin29) )
			{
				chaPremiumitem.UsePremiumItem( 55 );
			}
			// pluto 실버 패키지2 1일
			if( InvenItem[ SelectInvenItemIndex -1 ].sItemInfo.CODE == (sinBI2|sin30) )
			{
				chaPremiumitem.UsePremiumItem( 56 );
			}
			// pluto 실버 패키지2 7일
			if( InvenItem[ SelectInvenItemIndex -1 ].sItemInfo.CODE == (sinBI2|sin31) )
			{
				chaPremiumitem.UsePremiumItem( 57 );
			}
			// pluto 골드 패키지2 3시간
			if( InvenItem[ SelectInvenItemIndex -1 ].sItemInfo.CODE == (sinBI2|sin32) )
			{
				chaPremiumitem.UsePremiumItem( 58 );
			}
			// pluto 골드 패키지2 1일
			if( InvenItem[ SelectInvenItemIndex -1 ].sItemInfo.CODE == (sinBI2|sin33) )
			{
				chaPremiumitem.UsePremiumItem( 59 );
			}
			// pluto 골드 패키지2 7일
			if( InvenItem[ SelectInvenItemIndex -1 ].sItemInfo.CODE == (sinBI2|sin34) )
			{
				chaPremiumitem.UsePremiumItem( 60 );
			}
			// pluto 슈페리어 패키지2 3시간
			if( InvenItem[ SelectInvenItemIndex -1 ].sItemInfo.CODE == (sinBI2|sin35) )
			{
				chaPremiumitem.UsePremiumItem( 61 );
			}
			// pluto 슈페리어 패키지2 1일
			if( InvenItem[ SelectInvenItemIndex -1 ].sItemInfo.CODE == (sinBI2|sin36) )
			{
				chaPremiumitem.UsePremiumItem( 62 );
			}
			// pluto 슈페리어 패키지2 7일
			if( InvenItem[ SelectInvenItemIndex -1 ].sItemInfo.CODE == (sinBI2|sin37) )
			{
				chaPremiumitem.UsePremiumItem( 63 );
			}
			// pluto 마이트 오브 아웰2 1일
			if( InvenItem[ SelectInvenItemIndex - 1 ].sItemInfo.CODE == (sinBI1|sin30) )
			{
				chaPremiumitem.UsePremiumItem( 64 );
			}
			// pluto 마이트 오브 아웰2 30일
			if( InvenItem[ SelectInvenItemIndex - 1 ].sItemInfo.CODE == (sinBI1|sin31) )
			{
				chaPremiumitem.UsePremiumItem( 65 );
			}
			// pluto 슈페리어 패키지 3시간
			if( InvenItem[ SelectInvenItemIndex - 1 ].sItemInfo.CODE == (sinBI2|sin23) )
			{
				chaPremiumitem.UsePremiumItem( 66 );
			}
			// pluto 슈페리어 패키지 1일
			if( InvenItem[ SelectInvenItemIndex - 1 ].sItemInfo.CODE == (sinBI2|sin24) )
			{
				chaPremiumitem.UsePremiumItem( 67 );
			}
			// pluto 슈페리어 패키지 7일
			if( InvenItem[ SelectInvenItemIndex - 1 ].sItemInfo.CODE == (sinBI2|sin25) )
			{
				chaPremiumitem.UsePremiumItem( 68 );
			}
			// pluto 펫(해외)
			if( InvenItem[ SelectInvenItemIndex -1 ].sItemInfo.CODE == (sinBI1|sin32) )
			{
				chaPremiumitem.UsePremiumItem( 69 );
			}
			// pluto 펫(해외)
			if( InvenItem[ SelectInvenItemIndex -1 ].sItemInfo.CODE == (sinBI1|sin33) )
			{
				chaPremiumitem.UsePremiumItem( 70 );
			}
			// pluto 펫(해외)
			if( InvenItem[ SelectInvenItemIndex -1 ].sItemInfo.CODE == (sinBI1|sin34) )
			{
				chaPremiumitem.UsePremiumItem( 71 );
			}
			// 박재원 - 에이징 마스터(무기)
			if(InvenItem[SelectInvenItemIndex-1].sItemInfo.CODE==(sinBI1|sin36))
			{
				// 무기 에이징 마스터 아이템(양손무기도 왼손 슬롯만 체크하면 된다.)
				if(!cInvenTory.InvenItem[sInven[0].ItemIndex-1].Flag || // 무기를 인벤에 착용하지 않을경우
				   !cInvenTory.InvenItem[sInven[0].ItemIndex-1].sItemInfo.ItemAgingNum[1]) // 왼손무기(에이징된 무기가 아닐경우)
				{
					cMessageBox.ShowMessage(MESSAGE_HAVE_NOT_AGINGITEM); 
					return;
				}
				chaPremiumitem.UsePremiumItem(73);
			}
			// 박재원 - 에이징 마스터(방패, 오브, 비즈)
			if(InvenItem[SelectInvenItemIndex-1].sItemInfo.CODE==(sinBI1|sin37))
			{
				// 방패 오브 비즈 에이징 마스터 아이템
				if(!cInvenTory.InvenItem[sInven[1].ItemIndex-1].Flag || // 무기를 인벤에 착용하지 않을경우
					!cInvenTory.InvenItem[sInven[1].ItemIndex-1].sItemInfo.ItemAgingNum[1]) // 에이징된 아이템이 아닐경우
				{
					cMessageBox.ShowMessage(MESSAGE_HAVE_NOT_AGINGITEM); 
					return;
				}
				chaPremiumitem.UsePremiumItem(74);
			}
			// 박재원 - 에이징 마스터(갑옷, 로브)
			if(InvenItem[SelectInvenItemIndex-1].sItemInfo.CODE==(sinBI1|sin38))
			{
				// 갑옷 에이징 마스터 아이템
				if(!cInvenTory.InvenItem[sInven[2].ItemIndex-1].Flag|| // 갑옷를 인벤에 착용하지 않을경우
					!cInvenTory.InvenItem[sInven[2].ItemIndex-1].sItemInfo.ItemAgingNum[1]) // 갑옷(에이징된 무기가 아닐경우)
				{
					cMessageBox.ShowMessage(MESSAGE_HAVE_NOT_AGINGITEM); 
					return;
				}
				chaPremiumitem.UsePremiumItem(75);
			}
			// 박재원 - 스킬 마스터(1차)(스킬 숙련도를 전직별로 최대치로 한다.)
			if(InvenItem[SelectInvenItemIndex-1].sItemInfo.CODE==(sinBI1|sin39))
			{
				if(cSkill.CheckMaturedSkill(1) == FALSE)
				{
					// 이미 1차 전직 스킬이 모두 숙련됨
					cMessageBox.ShowMessage(MESSAGE_HAVE_NOT_MATURESKILL); 
					return;
				}
				chaPremiumitem.UsePremiumItem(76);

			}
			// 박재원 - 스킬 마스터(2차)(스킬 숙련도를 전직별로 최대치로 한다.)
			if(InvenItem[SelectInvenItemIndex-1].sItemInfo.CODE==(sinBI1|sin40))
			{
				if(cSkill.CheckMaturedSkill(2)==FALSE)
				{
					// 이미 2차 전직 스킬이 모두 숙련됨
					cMessageBox.ShowMessage(MESSAGE_HAVE_NOT_MATURESKILL); 
					return;
				}
				chaPremiumitem.UsePremiumItem(77);

			}
			// 박재원 - 스킬 마스터(3차)(스킬 숙련도를 전직별로 최대치로 한다.)
			if(InvenItem[SelectInvenItemIndex-1].sItemInfo.CODE==(sinBI1|sin41))
			{
				if(cSkill.CheckMaturedSkill(3) == 0)
				{
					// 이미 3차 전직 스킬이 모두 숙련됨
					cMessageBox.ShowMessage(MESSAGE_HAVE_NOT_MATURESKILL); 
					return;
				}
				chaPremiumitem.UsePremiumItem(78);

			}
			// 박재원 - 이동 상점 아이템
			if(InvenItem[SelectInvenItemIndex-1].sItemInfo.CODE==(sinBI1|sin42))
			{
				// lpPlayInfo->Position.Area==rsCASTLE_FIELD
				// StageField[lpCurPlayer->OnStageField]->State==FIELD_STATE_ROOM || // 장기판
				if( StageField[lpCurPlayer->OnStageField]->State==FIELD_STATE_ROOM || // 장기판
				   	StageField[lpCurPlayer->OnStageField]->FieldCode==START_FIELD_NUM || // 리카르텐
					StageField[lpCurPlayer->OnStageField]->FieldCode==START_FIELD_MORYON || // 필라이
					StageField[lpCurPlayer->OnStageField]->FieldCode==rsCASTLE_FIELD || // 공성
					StageField[lpCurPlayer->OnStageField]->FieldCode==rsSOD_FIELD || // 벨라트라
					StageField[lpCurPlayer->OnStageField]->FieldCode==29 ) // 유라 빌리지
				{
					cMessageBox.ShowMessage(MESSAGE_NO_USE_ITEM); 
					return;
				}
				iEventBuyItemKind = 79;
				chaPremiumitem.Using_SelectInvenItemIndex = SelectInvenItemIndex-1;
				cMessageBox.ShowMessage3(MESSAGE_EVENT_ITEMOK, InvenItem[SelectInvenItemIndex-1].sItemInfo.ItemName);
			}
			
			// 박재원 - 경험치증가 포션(100% 1일 추가)
			if(InvenItem[SelectInvenItemIndex-1].sItemInfo.CODE==(sinBI1|sin43))
			{
				if( chaPremiumitem.m_ExpUpPotionTime > 0 )
				{
					cMessageBox.ShowMessage(MESSAGE_CLANSKILL_USE);
					return;
				}

				chaPremiumitem.UsePremiumItem(80);
			}
			
			// 박재원 - 경험치증가 포션(100% 7일 추가)
			if(InvenItem[SelectInvenItemIndex-1].sItemInfo.CODE==(sinBI1|sin44))
			{
				if( chaPremiumitem.m_ExpUpPotionTime > 0 )
				{
					cMessageBox.ShowMessage(MESSAGE_CLANSKILL_USE);
					return;
				}

				chaPremiumitem.UsePremiumItem(81);
			}
			
			// 박재원 - 캐릭터 속성별 스탯 초기화 아이템(5종 - 힘, 정신력, 재능, 민첩성, 건강)
			if(InvenItem[SelectInvenItemIndex-1].sItemInfo.CODE==(sinBI1|sin45)) // 힘
				chaPremiumitem.UsePremiumItem(82);
			if(InvenItem[SelectInvenItemIndex-1].sItemInfo.CODE==(sinBI1|sin46)) // 정신력
				chaPremiumitem.UsePremiumItem(83);
			if(InvenItem[SelectInvenItemIndex-1].sItemInfo.CODE==(sinBI1|sin47)) // 재능
				chaPremiumitem.UsePremiumItem(84);
			if(InvenItem[SelectInvenItemIndex-1].sItemInfo.CODE==(sinBI1|sin48)) // 민첩성
				chaPremiumitem.UsePremiumItem(85);
			if(InvenItem[SelectInvenItemIndex-1].sItemInfo.CODE==(sinBI1|sin49)) // 건강
				chaPremiumitem.UsePremiumItem(86);

			// 박재원 - 경험치증가 포션(100% 30일 추가)
			if(InvenItem[SelectInvenItemIndex-1].sItemInfo.CODE==(sinBI1|sin50))
			{
				if( chaPremiumitem.m_ExpUpPotionTime > 0 )
				{
					cMessageBox.ShowMessage(MESSAGE_CLANSKILL_USE);
					return;
				}

				chaPremiumitem.UsePremiumItem(87);
			}

			// // 박재원 - 픽닉스 펫(30일) 추가
			if( InvenItem[ SelectInvenItemIndex -1 ].sItemInfo.CODE == (sinBI1|sin51) )
			{
				chaPremiumitem.UsePremiumItem( 88 );
			}

			// 박재원 - 엘더 코퍼 오어 추가
			if( InvenItem[ SelectInvenItemIndex -1 ].sItemInfo.CODE == (sinBI1|sin60) )
			{
				chaPremiumitem.UsePremiumItem( 97 );
			}

			// 박재원 - 슈퍼 에이징 스톤 추가
			if( InvenItem[ SelectInvenItemIndex -1 ].sItemInfo.CODE == (sinBI1|sin61) )
			{
				chaPremiumitem.UsePremiumItem( 98 );
			}

			// 박재원 - 빌링 도우미 펫 추가(테리)
			if( InvenItem[ SelectInvenItemIndex -1 ].sItemInfo.CODE == (sinBI1|sin52) )
			{
				if(sinChar->Level < 10)
				{
					cMessageBox.ShowMessage( MESSAGE_PET_USE_LEVEL );
					return;
				}
				chaPremiumitem.UsePremiumItem( 99 );
			}
			// 박재원 - 빌링 도우미 펫 추가(넵시스)
			if( InvenItem[ SelectInvenItemIndex -1 ].sItemInfo.CODE == (sinBI1|sin53) )
			{
				if(sinChar->Level < 10  )
				{
					cMessageBox.ShowMessage( MESSAGE_PET_USE_LEVEL );
					return;
				}
				chaPremiumitem.UsePremiumItem( 100 );
			}
			// 박재원 - 빌링 도우미 펫 추가(이오)
			if( InvenItem[ SelectInvenItemIndex -1 ].sItemInfo.CODE == (sinBI1|sin54) )
			{
				if(sinChar->Level < 10  )
				{
					cMessageBox.ShowMessage( MESSAGE_PET_USE_LEVEL );
					return;
				}
				chaPremiumitem.UsePremiumItem( 101 );
			}
			// 박재원 - 빌링 도우미 펫 추가(무트)
			if( InvenItem[ SelectInvenItemIndex -1 ].sItemInfo.CODE == (sinBI1|sin55) )
			{
				if(sinChar->Level < 10  )
				{
					cMessageBox.ShowMessage( MESSAGE_PET_USE_LEVEL );
					return;
				}
				chaPremiumitem.UsePremiumItem( 102 );
			}


			// 장별 - 빌링 도우미 펫 추가(테리) // 1일
			if( InvenItem[ SelectInvenItemIndex -1 ].sItemInfo.CODE == (sinBI1|sin69) )
			{
				if(sinChar->Level < 10)
				{
					cMessageBox.ShowMessage( MESSAGE_PET_USE_LEVEL );
					return;
				}
				chaPremiumitem.UsePremiumItem( 103 );
			}
			// 장별 - 빌링 도우미 펫 추가(넵시스)
			if( InvenItem[ SelectInvenItemIndex -1 ].sItemInfo.CODE == (sinBI1|sin70) )
			{
				if(sinChar->Level < 10  )
				{
					cMessageBox.ShowMessage( MESSAGE_PET_USE_LEVEL );
					return;
				}
				chaPremiumitem.UsePremiumItem( 104 );
			}
			// 장별 - 빌링 도우미 펫 추가(이오)
			if( InvenItem[ SelectInvenItemIndex -1 ].sItemInfo.CODE == (sinBI1|sin71) )
			{
				if(sinChar->Level < 10  )
				{
					cMessageBox.ShowMessage( MESSAGE_PET_USE_LEVEL );
					return;
				}
				chaPremiumitem.UsePremiumItem( 105 );
			}
			// 장별 - 빌링 도우미 펫 추가(무트)
			if( InvenItem[ SelectInvenItemIndex -1 ].sItemInfo.CODE == (sinBI1|sin72) )
			{
				if(sinChar->Level < 10  )
				{
					cMessageBox.ShowMessage( MESSAGE_PET_USE_LEVEL );
					return;
				}
				chaPremiumitem.UsePremiumItem( 106 );
			}

			// 장별 - 경험치증가 포션(100% 1시간)
			if(InvenItem[SelectInvenItemIndex-1].sItemInfo.CODE==(sinBI1|sin80))
			{
				if( chaPremiumitem.m_ExpUpPotionTime > 0 )
				{
					cMessageBox.ShowMessage(MESSAGE_CLANSKILL_USE);
					return;
				}

				chaPremiumitem.UsePremiumItem(107);
			}

			// 장별 - 피닉스펫(1시간)
			if( InvenItem[ SelectInvenItemIndex -1 ].sItemInfo.CODE == (sinBI1|sin77) )
			{
				chaPremiumitem.UsePremiumItem( 108 );
			}

			// 장별 - 빌링 도우미 펫 추가(테리)
			if( InvenItem[ SelectInvenItemIndex -1 ].sItemInfo.CODE == (sinBI1|sin65) )
			{
				if(sinChar->Level < 10)
				{
					cMessageBox.ShowMessage( MESSAGE_PET_USE_LEVEL );
					return;
				}
				chaPremiumitem.UsePremiumItem( 109 );
			}
			// 장별 - 빌링 도우미 펫 추가(넵시스)
			if( InvenItem[ SelectInvenItemIndex -1 ].sItemInfo.CODE == (sinBI1|sin66) )
			{
				if(sinChar->Level < 10  )
				{
					cMessageBox.ShowMessage( MESSAGE_PET_USE_LEVEL );
					return;
				}
				chaPremiumitem.UsePremiumItem( 110 );
			}
			// 장별 - 빌링 도우미 펫 추가(이오)
			if( InvenItem[ SelectInvenItemIndex -1 ].sItemInfo.CODE == (sinBI1|sin67) )
			{
				if(sinChar->Level < 10  )
				{
					cMessageBox.ShowMessage( MESSAGE_PET_USE_LEVEL );
					return;
				}
				chaPremiumitem.UsePremiumItem( 111 );
			}
			// 장별 - 빌링 도우미 펫 추가(무트)
			if( InvenItem[ SelectInvenItemIndex -1 ].sItemInfo.CODE == (sinBI1|sin68) )
			{
				if(sinChar->Level < 10  )
				{
					cMessageBox.ShowMessage( MESSAGE_PET_USE_LEVEL );
					return;
				}
				chaPremiumitem.UsePremiumItem( 112 );
			}
			// 박재원 - 에이징 마스터(2차) - 암릿
			if(InvenItem[SelectInvenItemIndex-1].sItemInfo.CODE==(sinBI1|sin62))
			{
				// 암릿 에이징 마스터 아이템
				if(!cInvenTory.InvenItem[sInven[7].ItemIndex-1].Flag || // 암릿을 인벤에 착용하지 않을경우
					!cInvenTory.InvenItem[sInven[7].ItemIndex-1].sItemInfo.ItemAgingNum[1]) // 착용한 암릿이 에이징된 것이 아닐경우
				{
					cMessageBox.ShowMessage(MESSAGE_HAVE_NOT_AGINGITEM); 
					return;
				}
				chaPremiumitem.UsePremiumItem(113);
			}
			// 박재원 - 에이징 마스터(2차) - 장갑
			if(InvenItem[SelectInvenItemIndex-1].sItemInfo.CODE==(sinBI1|sin63))
			{
				// 장갑 에이징 마스터 아이템
				if(!cInvenTory.InvenItem[sInven[8].ItemIndex-1].Flag || // 장갑을 인벤에 착용하지 않을경우
					!cInvenTory.InvenItem[sInven[8].ItemIndex-1].sItemInfo.ItemAgingNum[1]) // // 착용한 장갑이 에이징된 것이 아닐경우
				{
					cMessageBox.ShowMessage(MESSAGE_HAVE_NOT_AGINGITEM); 
					return;
				}
				chaPremiumitem.UsePremiumItem(114);
			}
			// 박재원 - 에이징 마스터(2차) - 부츠
			if(InvenItem[SelectInvenItemIndex-1].sItemInfo.CODE==(sinBI1|sin64))
			{
				// 부츠 에이징 마스터 아이템
				if(!cInvenTory.InvenItem[sInven[9].ItemIndex-1].Flag|| // 부츠를 인벤에 착용하지 않을경우
					!cInvenTory.InvenItem[sInven[9].ItemIndex-1].sItemInfo.ItemAgingNum[1]) // // 착용한 부츠가 에이징된 것이 아닐경우
				{
					cMessageBox.ShowMessage(MESSAGE_HAVE_NOT_AGINGITEM); 
					return;
				}
				chaPremiumitem.UsePremiumItem(115);
			}


			// 장별 - 그라비티 스크롤
			if( InvenItem[ SelectInvenItemIndex - 1 ].sItemInfo.CODE == (sinBI1|sin84) )
			{
				chaPremiumitem.UsePremiumItem( 116 );
			}

			// 장별 - 빌링 도우미 펫 추가(테리 1시간)
			if( InvenItem[ SelectInvenItemIndex -1 ].sItemInfo.CODE == (sinBI1|sin73) )
			{
				if(sinChar->Level < 10)
				{
					cMessageBox.ShowMessage( MESSAGE_PET_USE_LEVEL );
					return;
				}
				chaPremiumitem.UsePremiumItem( 117 );


			}
			// 장별 - 빌링 도우미 펫 추가(넵시스 1시간)
			if( InvenItem[ SelectInvenItemIndex -1 ].sItemInfo.CODE == (sinBI1|sin74) )
			{
				if(sinChar->Level < 10  )
				{
					cMessageBox.ShowMessage( MESSAGE_PET_USE_LEVEL );
					return;
				}
				chaPremiumitem.UsePremiumItem( 118 );
			}
			// 장별 - 빌링 도우미 펫 추가(이오 1시간)
			if( InvenItem[ SelectInvenItemIndex -1 ].sItemInfo.CODE == (sinBI1|sin75) )
			{
				if(sinChar->Level < 10  )
				{
					cMessageBox.ShowMessage( MESSAGE_PET_USE_LEVEL );
					return;
				}
				chaPremiumitem.UsePremiumItem( 119 );
			}
			// 장별 - 빌링 도우미 펫 추가(무트 1시간)
			if( InvenItem[ SelectInvenItemIndex -1 ].sItemInfo.CODE == (sinBI1|sin76) )
			{
				if(sinChar->Level < 10  )
				{
					cMessageBox.ShowMessage( MESSAGE_PET_USE_LEVEL );
					return;
				}
				chaPremiumitem.UsePremiumItem( 120 );
			}

			// 장별 - 써드 아이즈(1시간)
			if(InvenItem[SelectInvenItemIndex-1].sItemInfo.CODE==(sinBI1|sin78))
			{
				if( chaPremiumitem.m_ThirdEyesTime > 0 )
				{
					cMessageBox.ShowMessage(MESSAGE_CLANSKILL_USE);
					return;
				}

				chaPremiumitem.UsePremiumItem(121);
			}

			// 장별 - 슈퍼 에이징 스톤 1.5
			if( InvenItem[ SelectInvenItemIndex -1 ].sItemInfo.CODE == (sinBI1|sin85) )
			{
				chaPremiumitem.UsePremiumItem( 122 );
			}

			 // 장별 - 경험치증가포션(1시간)
			if(InvenItem[SelectInvenItemIndex-1].sItemInfo.CODE==(sinBI1|sin79))
			{
				if( chaPremiumitem.m_ExpUpPotionTime > 0 )
				{
					cMessageBox.ShowMessage(MESSAGE_CLANSKILL_USE);
					return;
				}

				chaPremiumitem.UsePremiumItem(123);
			}

			// 장별 - 뱀피릭 커스핏(1시간)
			if(InvenItem[SelectInvenItemIndex-1].sItemInfo.CODE==(sinBI1|sin81))
			{
				if( chaPremiumitem.m_VampiricCuspidTime > 0 )
				{
					cMessageBox.ShowMessage(MESSAGE_CLANSKILL_USE);
					return;
				}

				chaPremiumitem.UsePremiumItem(124);
			}

			// 장별 - 마나 리차징 포션(1시간)
			if(InvenItem[SelectInvenItemIndex-1].sItemInfo.CODE==(sinBI1|sin82))
			{
				if( chaPremiumitem.m_ManaRechargingPTime > 0 )
				{
					cMessageBox.ShowMessage(MESSAGE_CLANSKILL_USE);
					return;
				}

				chaPremiumitem.UsePremiumItem(125);
			}

			// 장별 - 마나 리듀스(1시간)
			if( InvenItem[ SelectInvenItemIndex -1 ].sItemInfo.CODE == (sinBI1|sin83) )
			{
				if( chaPremiumitem.m_ManaReducePotiontime > 0 )
				{
					cMessageBox.ShowMessage(MESSAGE_CLANSKILL_USE);
					return;
				}

				chaPremiumitem.UsePremiumItem( 126 );
			}

			// 장별 - 뱀피릭 커스핏 EX(1시간)
			if(InvenItem[SelectInvenItemIndex-1].sItemInfo.CODE==(sinBI1|sin86))
			{
				if( chaPremiumitem.m_VampiricCuspidEXTime > 0 )
				{
					cMessageBox.ShowMessage(MESSAGE_CLANSKILL_USE);
					return;
				}

				chaPremiumitem.UsePremiumItem(127);
			}

			// 장별 - 뱀피릭 커스핏 EX(3시간)
			if(InvenItem[SelectInvenItemIndex-1].sItemInfo.CODE==(sinBI1|sin87))
			{
				if( chaPremiumitem.m_VampiricCuspidEXTime > 0 )
				{
					cMessageBox.ShowMessage(MESSAGE_CLANSKILL_USE);
					return;
				}

				chaPremiumitem.UsePremiumItem(128);
			}

			// 장별 - 뱀피릭 커스핏 EX(1일)
			if(InvenItem[SelectInvenItemIndex-1].sItemInfo.CODE==(sinBI1|sin88))
			{
				if( chaPremiumitem.m_VampiricCuspidEXTime > 0 )
				{
					cMessageBox.ShowMessage(MESSAGE_CLANSKILL_USE);
					return;
				}

				chaPremiumitem.UsePremiumItem(129);
			}
			// 석지용 - 믹스쳐 리셋 스톤
			if(InvenItem[SelectInvenItemIndex-1].sItemInfo.CODE==(sinBI1|sin89))
				chaPremiumitem.UsePremiumItem(130);
			// 박재원 - 리스펙 스톤
			if(InvenItem[SelectInvenItemIndex-1].sItemInfo.CODE==(sinBI1|sin90))
				chaPremiumitem.UsePremiumItem(131);
			// 박재원 - 근력 리듀스 포션(1일)
			if(InvenItem[SelectInvenItemIndex-1].sItemInfo.CODE==(sinBI1|sin91))
			{
				if( chaPremiumitem.m_StaminaReducePotiontime > 0 ) 
				{
					cMessageBox.ShowMessage(MESSAGE_CLANSKILL_USE);
					return;
				}

				chaPremiumitem.UsePremiumItem(132);
			}
			// 박재원 - 근력 리듀스 포션(7일)
			if(InvenItem[SelectInvenItemIndex-1].sItemInfo.CODE==(sinBI1|sin92))
			{
				if( chaPremiumitem.m_StaminaReducePotiontime > 0 )
				{
					cMessageBox.ShowMessage(MESSAGE_CLANSKILL_USE);
					return;
				}

				chaPremiumitem.UsePremiumItem(133);
			}
			// 석지용 - 필드 코어 1일
			if( InvenItem[ SelectInvenItemIndex-1 ].sItemInfo.CODE == (sinBI1 | sin93 ) )
			{
				if( sinChar->Level > 19 )
				{
					chaPremiumitem.TelePortCore.ItemIndex = SelectInvenItemIndex-1;		// 인벤토리에서의 아이템의 인덱스 얻기
					chaPremiumitem.UsePremiumItem(134);									// 아이템 사용
				}
				else
					cMessageBox.ShowMessage(MESSAGE_NOT_ENOUGH_LEVEL);
			}
			// 석지용 - 필드 코어 7일
			if( InvenItem[ SelectInvenItemIndex-1 ].sItemInfo.CODE == (sinBI1 | sin94 ) )
			{
				if( sinChar->Level > 19 )
				{
					chaPremiumitem.TelePortCore.ItemIndex = SelectInvenItemIndex-1;		// 인벤토리에서의 아이템의 인덱스 얻기
					chaPremiumitem.UsePremiumItem(135);									// 아이템 사용
				}
				else
					cMessageBox.ShowMessage(MESSAGE_NOT_ENOUGH_LEVEL);
			}

			//////////////////////////////////////////////////////////////////////////
					
			if(haCastleSkillUseFlag!=1)return; //사용중인 아이템이다.

			//사용
			if(!sinItemPickUpDelayFlag){ //무한으로 사용할수있음을 방지한다
				InvenItem[SelectInvenItemIndex-1].sItemInfo.PotionCount = 300;
				sinThrowItemToFeild(&InvenItem[SelectInvenItemIndex-1]);
				InvenItem[SelectInvenItemIndex-1].Flag = 0;
				cInvenTory.CheckWeight();//무게를 보정해준다
				ReFormInvenItem();
				SaveGameData();
			}


		}
	

/*
#define	JOBCODE_MECHANICIAN			2
#define	JOBCODE_FIGHTER				1
#define	JOBCODE_PIKEMAN				4
#define	JOBCODE_ARCHER				3

#define	JOBCODE_KNIGHT				6
#define	JOBCODE_ATALANTA			5
#define	JOBCODE_PRIESTESS			8
#define	JOBCODE_MAGICIAN			7
*/
		int UseFlag2 = 0;

		//발렌타인데이 초콜릿 , 사탕
		if((InvenItem[SelectInvenItemIndex-1].CODE &sinITEM_MASK2 ) == sinCH1){

			if(lpCurPlayer->OnStageField>=0 && StageField[ lpCurPlayer->OnStageField ]->State == FIELD_STATE_VILLAGE) return;
			//Contiue 초기화
			for(int i = 0 ; i < MAX_CONTINUE_SKILL ; i++){
				if(ContinueSkill[i].Flag){
					if(ContinueSkill[i].CODE == SKILL_VIRTUAL_LIFE){
						//memset(&ContinueSkill[i],0,sizeof(sSKILL));
						return; //중복시전 불가
					}
				}
			}

#ifdef _LANGUAGE_ENGLISH
			if((InvenItem[SelectInvenItemIndex-1].CODE &sinITEM_MASK3) == sin01) // 블랙 초콜렛
			{
				switch(sinChar->JOB_CODE)
				{
					case JOBCODE_MECHANICIAN:
					case JOBCODE_FIGHTER:
					case JOBCODE_PIKEMAN:
					case JOBCODE_KNIGHT:
					case JOBCODE_MAGICIAN:
						cMessageBox.ShowMessage(MESSAGE_NOT_USE_MALE);

					break;
					case JOBCODE_ARCHER:
					case JOBCODE_ATALANTA:
					case JOBCODE_PRIESTESS:
						UseFlag2 = 1;

					break;
				}
			}
			else if((InvenItem[SelectInvenItemIndex-1].CODE &sinITEM_MASK3) == sin02 ) // 화이트 초콜렛
			{
				switch(sinChar->JOB_CODE)
				{
					case JOBCODE_ARCHER:
					case JOBCODE_ATALANTA:
					case JOBCODE_PRIESTESS:
						cMessageBox.ShowMessage(MESSAGE_NOT_USE_FEMALE);

					break;
					case JOBCODE_MECHANICIAN:
					case JOBCODE_FIGHTER:
					case JOBCODE_PIKEMAN:
					case JOBCODE_KNIGHT:
					case JOBCODE_MAGICIAN:
						UseFlag2 = 1;

					break;
				}
			}
#else
			if((InvenItem[SelectInvenItemIndex-1].CODE &sinITEM_MASK3) == sin01 ||
				(InvenItem[SelectInvenItemIndex-1].CODE &sinITEM_MASK3) == sin02 ){ //초콜릿
					switch(sinChar->JOB_CODE){
						case JOBCODE_ARCHER:
						case JOBCODE_ATALANTA:
						case JOBCODE_PRIESTESS:
							cMessageBox.ShowMessage(MESSAGE_NOT_USE_FEMALE);

						break;
						case JOBCODE_MECHANICIAN:
						case JOBCODE_FIGHTER:
						case JOBCODE_PIKEMAN:
						case JOBCODE_KNIGHT:
						case JOBCODE_MAGICIAN:
							UseFlag2 = 1;

						break;
					}
				}
#endif

			if((InvenItem[SelectInvenItemIndex-1].CODE &sinITEM_MASK3) == sin03 ||
				(InvenItem[SelectInvenItemIndex-1].CODE &sinITEM_MASK3) == sin04 ){ //사탕
					switch(sinChar->JOB_CODE){
						case JOBCODE_ARCHER:
						case JOBCODE_ATALANTA:
						case JOBCODE_PRIESTESS:
							UseFlag2 = 1;
							
						break;
						case JOBCODE_MECHANICIAN:
						case JOBCODE_FIGHTER:
						case JOBCODE_PIKEMAN:
						case JOBCODE_KNIGHT:
						case JOBCODE_MAGICIAN:
							cMessageBox.ShowMessage(MESSAGE_NOT_USE_MALE);
						break;

					}

				}


			if ( cItem.GetItemLimitTime( &InvenItem[SelectInvenItemIndex-1] )==FALSE ) {
				wsprintf(szBuff,"%s : %s",sinChar->szName, sinDeadCandyEat);
				SendChatMessageToServer(szBuff);
				InvenItem[SelectInvenItemIndex-1].Flag = 0;
				sinUsePotionDelayFlag = 1;
				UseFlag2 =0;
				cInvenTory.CheckWeight();
				ReFormInvenItem();
			}

			if(!sinItemPickUpDelayFlag && UseFlag2){ //무한으로 사용할수있음을 방지한다
				InvenItem[SelectInvenItemIndex-1].sItemInfo.PotionCount = 100;
				sinThrowItemToFeild(&InvenItem[SelectInvenItemIndex-1]);
				InvenItem[SelectInvenItemIndex-1].Flag = 0;
				cInvenTory.CheckWeight();//무게를 보정해준다
				ReFormInvenItem();
			}
		}

	}
	ReFormPotionNum();	//포션 갯수를 체크한다
	
}
/*----------------------------------------------------------------------------*
*							   RButtonUp
*-----------------------------------------------------------------------------*/	
void cINVENTORY::RButtonUp(int x , int y)
{

}
/*----------------------------------------------------------------------------*
*							    KeyDown
*-----------------------------------------------------------------------------*/	
void cINVENTORY::KeyDown()
{

	sITEM TempPotion;
	
	memset(&TempPotion,0,sizeof(sITEM));
	int CountTemp  = 0;
	int sArrowPosi = 0;
	char szKeyBuff[3]={'1','2','3'}; //키버퍼

	// T: the costume view on/off, as in the 3215 client (the inventory's up arrow does the same)
	if(sinGetKeyClick('T')){
		if(!MouseItem.Flag) sinToggleCostumeView();
	}

	if(sinGetKeyClick('V')){
		cHelpPet.PetMessage("*InvenTory",3);
		if(cInvenTory.OpenFlag)cInvenTory.OpenFlag = SIN_CLOSE;
		else cInvenTory.OpenFlag = SIN_OPEN;
		cInterFace.CheckAllBox(SIN_INVENTORY); //창을 하나만 띄운다 
		if(cInvenTory.OpenFlag){
			if(!sinFireShow){
				StartMenuFlame(0,350);
				sinPlaySound( 0 );
			}
			else
				sinPlaySound(SIN_SOUND_SHOW_INTER);
			sinFireShow = 1;
		}

	}

	int TempPotionCount = 0;
	
	//Pet 메세지
//	cHelpPet.PetMessage("*Potion_ShortKey",3);

	for(int i=0; i < 3 ; i++){ 
		if(sinGetKeyClick(szKeyBuff[i])){
			if(SelectPotionIndex){ //포션을 움직일때 
				if(VRKeyBuff[VK_SHIFT]){
					if(MouseItem.Flag)break; //겹쳐진 아이템이 있을경우 셋팅을 할수없다 
					if(MyShopSendButton || cMyShop.OpenFlag)break;
					if(InvenItem[SelectPotionIndex-1].sItemInfo.PotionCount <= 0){ //포션 이 0일경우 지워준다
						InvenItem[SelectPotionIndex-1].Flag = 0;
						ReFormInvenItem();
						ReFormPotionNum(); //포션카운트를 갱신해준다
						break;

					}
					if(sInven[i+10].ItemIndex){ //아이템이 있을 경우에 
						if(InvenItem[sInven[i+10].ItemIndex-1].CODE == InvenItem[SelectPotionIndex-1].CODE){
							TempPotionCount = InvenItem[SelectPotionIndex-1].sItemInfo.PotionCount; //카운트를 저장
							for(int p=0; p < TempPotionCount; p++){
								if(sinChar->Potion_Space > InvenItem[sInven[i+10].ItemIndex-1].sItemInfo.PotionCount ){
									InvenItem[sInven[i+10].ItemIndex-1].sItemInfo.PotionCount++;
									InvenItem[SelectPotionIndex-1].sItemInfo.PotionCount--;
									sinPlaySound(InvenItem[SelectPotionIndex-1].SoundIndex); //물약소리를 내준다
									if(InvenItem[SelectPotionIndex-1].sItemInfo.PotionCount <= 0){
										InvenItem[SelectPotionIndex-1].Flag = 0;
										break;
									}
								}
							}
						}
					}
					else{ //아이템이 없을 경우에 
						memcpy(&TempPotion, &InvenItem[SelectPotionIndex-1] , sizeof(sITEM));
						TempPotion.SetX = sInven[i+10].Rect.left+(((sInven[i+10].Rect.right  - sInven[i+10].Rect.left) - InvenItem[SelectPotionIndex-1].w)/2);  //좌표 설정 							 
						TempPotion.SetY = sInven[i+10].Rect.top+(((sInven[i+10].Rect.bottom - sInven[i+10].Rect.top) - InvenItem[SelectPotionIndex-1].h)/2);	

						TempPotion.sItemInfo.PotionCount = 0;
						TempPotion.Flag = 1;
						TempPotion.ItemPosition = i+10+1;
						CountTemp = InvenItem[SelectPotionIndex-1].sItemInfo.PotionCount;
						for(int f=0; f < CountTemp; f++){										
							if(sinChar->Potion_Space > TempPotion.sItemInfo.PotionCount){
								TempPotion.sItemInfo.PotionCount++;
								InvenItem[SelectPotionIndex-1].sItemInfo.PotionCount--;
								if(InvenItem[SelectPotionIndex-1].sItemInfo.PotionCount <=0 ){
									InvenItem[SelectPotionIndex-1].Flag = 0;
									break;
								}

							}

						}
						LastSetInvenItem(&TempPotion,1); //인벤토리에 셋팅을 한다 	
					}
				
				}
		
				else{
					if(VRKeyBuff[VK_SHIFT])break;
					if(cWareHouse.OpenFlag || cTrade.OpenFlag ||cCraftItem.OpenFlag )break; 
					if(sInven[i+10].ItemIndex){
						if(sinActionPotion()){
							pUsePotion = &InvenItem[sInven[i+10].ItemIndex-1];
							break;
						}
					}
				}
				
			}
			else{
				if(VRKeyBuff[VK_SHIFT])break;
				if(cWareHouse.OpenFlag || cTrade.OpenFlag ||cCraftItem.OpenFlag )break; 
				if(sInven[i+10].ItemIndex){
					if(sinActionPotion()){
						cHelpPet.PetMessage("*Potion_ShortKey",3+i);
						pUsePotion = &InvenItem[sInven[i+10].ItemIndex-1];
						break;
					}
				}
			}
		}
	}

	if(!cTrade.OpenFlag && !sinUsePotionDelayFlag){ //트레이드하고있거나 물약을 먹을때는 안된다 
		//무기셋팅 펑션 키 
		if(sinGetKeyClick('E')){
			if(ArrowState[0]==0)
			{	sArrowPosi = 2; }
			else 
			{sArrowPosi = 1; }
			sinButtonFlag = sArrowPosi;
			SelectInvenItemIndex = 0; //에테르 코어남발을 막기위해 초기화
			ChangeABItem(sArrowPosi); // 2보다 작을때 인벤 박스 
		}
		if(sinGetKeyClick('W')){
			if(ArrowState[1]==0)sArrowPosi = 4;
			else sArrowPosi = 3;
			sinButtonFlag = sArrowPosi;
			
			sinChangeSetFlag = sArrowPosi;
			//단축키로 무기 셋팅시 보이는 스텟 재조정
			SetItemToChar();

			//ChangeABItem(sArrowPosi); // 2보다 클때 무기셋팅 
		}
	}


}
/*----------------------------------------------------------------------------*
*							  클래스 함수 
*-----------------------------------------------------------------------------*/	
int cINVENTORY::LoadItemInfo() //아이템정보를 로드한후 저장한다  
{
	int i;

	int TempItemIndex[INVENTORY_MAX_POS+4] = {0,};		// by ItemPosition (1..INVENTORY_MAX_POS)
	int TempItemIndex2[INVENTORY_MAX_POS+4] = {0,};
	
	char szFilePath[256];

	sITEM sinTempItem;
	int TEstFlag = 1;
	int cnt=0;


	for( i=0;i<INVENTORY_MAXITEM ; i++){ //인벤 토리 아이템 
		if(InvenItemTemp[i].Flag){
			sinSetSpecialItemCode(&InvenItemTemp[i].sItemInfo);//Temp도 확인해준다
		}
		if(InvenItem[i].Flag){
			sinSetSpecialItemCode(&InvenItem[i].sItemInfo);//기존에의 Special 아이템을 체크후 코드를 셋팅한다
			for(int j=0 ; j < MAX_ITEM ; j++){
				if(InvenItem[i].sItemInfo.CODE == sItem[j].CODE){
					memcpy(&sinTempItem,&sItem[j],sizeof(sITEM));
					memcpy(&sinTempItem.sItemInfo,&InvenItem[i].sItemInfo,sizeof(sITEMINFO));
					wsprintf(szFilePath ,"Image\\sinImage\\Items\\%s\\it%s.bmp",sItem[j].ItemFilePath,sItem[j].LastCategory);
					//복주 머니 관련
					if(!sItem[j].lpTempItem){ //이미지를 템프로 로드 
						sItem[j].lpTempItem = LoadDibSurfaceOffscreen(szFilePath); 
						if(!sItem[j].lpTempItem){
							TEstFlag = 0;

						}
					}
			       if((InvenItem[i].sItemInfo.CODE & sinITEM_MASK2) == sinPZ1 || (InvenItem[i].sItemInfo.CODE & sinITEM_MASK2) == sinPZ2){
						if(InvenItem[i].sItemInfo.PotionCount <=1){
							//복주머니를 찾는다.
							for(cnt = 0 ; cnt < MAX_ITEM ; cnt++){
								if(sItem[cnt].sItemInfo.CODE == (sinPZ1|sin00)){
									break;
								}
							}
							wsprintf(szFilePath ,"Image\\sinImage\\Items\\%s\\it%s.bmp",sItem[cnt].ItemFilePath,sItem[cnt].LastCategory);
							sItem[j].lpTempItem = LoadDibSurfaceOffscreen(szFilePath); //이미지를 먼저읽어준다
							sinTempItem.w = sItem[cnt].w; //크기 보정 
							sinTempItem.h = sItem[cnt].h;
						}
						else{
							wsprintf(szFilePath ,"Image\\sinImage\\Items\\%s\\it%s.bmp",sItem[j].ItemFilePath,sItem[j].LastCategory);
							sItem[j].lpTempItem = LoadDibSurfaceOffscreen(szFilePath); //이미지를 먼저읽어준다
							sinTempItem.w = sItem[j].w; //크기 보정 
							sinTempItem.h = sItem[j].h;

						}
					}
					
					sinTempItem.lpItem = sItem[j].lpTempItem;
					sinTempItem.x = InvenItem[i].x;
					sinTempItem.y = InvenItem[i].y; 
					sinTempItem.ItemPosition = InvenItem[i].ItemPosition;
					sinTempItem.Flag = 1;
					memcpy(&InvenItem[i],&sinTempItem,sizeof(sITEM));
					if(InvenItem[i].ItemPosition){
						sInven[InvenItem[i].ItemPosition-1].ItemIndex = i+1; //인벤토리 아이템 박스에 아이템 인덱스를 저장한다 
						sinSetCharItem(InvenItem[i].CODE , InvenItem[i].SetModelPosi, TRUE); //무기와 갑옷을 셋팅한다 
					}
					break;
				
				}

			}

		}
	}

	for( i=0 ; i < 84 ; i++){ //Temp아이템 
		if(InvenItemTemp[i].Flag){
			for(int j=0 ; j < MAX_ITEM ; j++){
				if(InvenItemTemp[i].sItemInfo.CODE == sItem[j].CODE){	
					memcpy(&sinTempItem,&sItem[j],sizeof(sITEM));
					memcpy(&sinTempItem.sItemInfo,&InvenItemTemp[i].sItemInfo,sizeof(sITEMINFO));
					wsprintf(szFilePath ,"Image\\sinImage\\Items\\%s\\it%s.bmp",sItem[j].ItemFilePath,sItem[j].LastCategory);
					if(!sItem[j].lpTempItem){
						sItem[j].lpTempItem = LoadDibSurfaceOffscreen(szFilePath);
						if(!sItem[j].lpTempItem){
							TEstFlag = 0;

						}

					}
					//복주 머니 관련
					if((InvenItemTemp[i].sItemInfo.CODE & sinITEM_MASK2) == sinPZ1 || (InvenItemTemp[i].sItemInfo.CODE & sinITEM_MASK2) == sinPZ2){
						if(InvenItemTemp[i].sItemInfo.PotionCount <= 1){
							//복주머니를 찾는다.
							for(cnt = 0 ; cnt < MAX_ITEM ; cnt++){
								if(sItem[cnt].sItemInfo.CODE == (sinPZ1|sin00)){
									break;
								}
							}
							wsprintf(szFilePath ,"Image\\sinImage\\Items\\%s\\it%s.bmp",sItem[cnt].ItemFilePath,sItem[cnt].LastCategory);
							sItem[j].lpTempItem = LoadDibSurfaceOffscreen(szFilePath); //이미지를 먼저읽어준다
							sinTempItem.w = sItem[cnt].w; //크기 보정 
							sinTempItem.h = sItem[cnt].h;
						}
						else{
							wsprintf(szFilePath ,"Image\\sinImage\\Items\\%s\\it%s.bmp",sItem[j].ItemFilePath,sItem[j].LastCategory);
							sItem[j].lpTempItem = LoadDibSurfaceOffscreen(szFilePath); //이미지를 먼저읽어준다
							sinTempItem.w = sItem[j].w; //크기 보정 
							sinTempItem.h = sItem[j].h;

						}

					}
		
					sinTempItem.lpItem = sItem[j].lpTempItem;
					sinTempItem.x = InvenItemTemp[i].x;
					sinTempItem.y = InvenItemTemp[i].y;
					sinTempItem.ItemPosition = InvenItemTemp[i].ItemPosition;
					sinTempItem.Flag = 1;
					memcpy(&InvenItemTemp[i],&sinTempItem,sizeof(sITEM));
					break;
				}
			}
		}
	}

	//잘못된 데이타의 아이템을 날린다 겹친것두 날려버린다 음하하하  
	for( i=0;i<INVENTORY_MAXITEM ; i++){ 
		if(InvenItemTemp[i].Flag){ //뒷쪽에 있는 아이템도 검색해준다 
			cItem.GetItemLimitTime(&InvenItemTemp[i]);
			//물약 정보가 잘못된것을 수정해준다 
			if(InvenItemTemp[i].x < StartX || InvenItemTemp[i].x > 800 || InvenItemTemp[i].y < StartY || InvenItemTemp[i].y > 600 ){
				ReSetPotionXY(&InvenItemTemp[i],1);
			}
			
			if(InvenItemTemp[i].ItemPosition){
				if(!TempItemIndex2[InvenItemTemp[i].ItemPosition]){
					TempItemIndex2[InvenItemTemp[i].ItemPosition] = i+1;
				}
				else{
					InvenItemTemp[i].Flag = 0;
					InvenItemTemp[TempItemIndex2[InvenItemTemp[i].ItemPosition]-1].Flag = 0;
				}
			}
			/////////////////////인증 코드 
			if(InvenItemTemp[i].ItemPosition ==2){
				if(InvenItemTemp[i].Class == ITEM_CLASS_WEAPON_TWO){
					break;
				}
			}

			if(!CheckItemForm(&InvenItemTemp[i].sItemInfo)){
				SendSetHackUser(FALSE); //해킹을 하려고했던 못된 유저를 고발 TRUE 접속 종료 
				InvenItemTemp[i].Flag =0; //치트된 아이템을 없애버린다 
			}

			//if(!InvenItemTemp[i].lpItem)
			//	InvenItemTemp[i].Flag = 0;

		}
	}
	bool ArmorFlag = false;
	for( i=0;i<INVENTORY_MAXITEM ; i++){ 
		if(InvenItem[i].Flag){
			cItem.GetItemLimitTime(&InvenItem[i]);
			if(InvenItem[i].x < StartX || InvenItem[i].x > 800 || InvenItem[i].y < StartY || InvenItem[i].y > 600){
				ReSetPotionXY(&InvenItem[i]);
			}

			if(InvenItem[i].ItemPosition > 0){
				if(InvenItem[i].ItemPosition > INVENTORY_MAX_POS)continue;	// was >= 15: the costume and premium ring slots are 14..16
				//현재 아머창에 아이템이 잇으면 
				if( InvenItem[i].ItemPosition == 3)
						ArmorFlag  = true;	

 				if(!TempItemIndex[InvenItem[i].ItemPosition]){
					TempItemIndex[InvenItem[i].ItemPosition] = i+1;
				}
				else{
					InvenItem[i].Flag = 0;
					InvenItem[TempItemIndex[InvenItem[i].ItemPosition]-1].Flag = 0;
					if(InvenItem[TempItemIndex[InvenItem[i].ItemPosition]-1].Class == ITEM_CLASS_WEAPON_TWO){
						if(InvenItem[TempItemIndex[InvenItem[i].ItemPosition]-1].ItemPosition == 1 ){//오른손 
							if(InvenItem[TempItemIndex[InvenItem[i].ItemPosition]-1].CODE == InvenItem[sInven[1].ItemIndex-1].CODE){
								InvenItem[sInven[1].ItemIndex-1].Flag = 0;
								sInven[1].ItemIndex = 0; //인벤토리 박스 초기화 

							}
						}
						if(InvenItem[TempItemIndex[InvenItem[i].ItemPosition]-1].ItemPosition == 2 ){//왼손 
							if(InvenItem[TempItemIndex[InvenItem[i].ItemPosition]-1].CODE == InvenItem[sInven[0].ItemIndex-1].CODE){
								InvenItem[sInven[0].ItemIndex-1].Flag = 0;
								sInven[0].ItemIndex = 0; //인벤토리 박스 초기화 

							}

						}

					}
					if(InvenItem[TempItemIndex[InvenItem[i].ItemPosition]-1].ItemPosition){
						sInven[TempItemIndex[InvenItem[i].ItemPosition]-1].ItemIndex = 0;
					}

					if(InvenItem[i].SetModelPosi)
						sinSetCharItem(InvenItem[i].CODE , InvenItem[i].SetModelPosi, FALSE); 
					if(InvenItem[TempItemIndex[InvenItem[i].ItemPosition]-1].SetModelPosi)
						sinSetCharItem(InvenItem[TempItemIndex[InvenItem[i].ItemPosition]-1].CODE , InvenItem[TempItemIndex[InvenItem[i].ItemPosition]-1].SetModelPosi, FALSE); 
					if(InvenItem[i].ItemPosition){
						sInven[InvenItem[i].ItemPosition-1].ItemIndex = 0;

					}
					if(InvenItem[i].Class == ITEM_CLASS_WEAPON_TWO){
						if(InvenItem[i].ItemPosition == 1 ){//오른손 
							if(InvenItem[i].CODE == InvenItem[sInven[1].ItemIndex-1].CODE){
								InvenItem[sInven[1].ItemIndex-1].Flag = 0;
								sInven[1].ItemIndex = 0; //인벤토리 박스 초기화 

							}
						}
						if(InvenItem[i].ItemPosition == 2 ){//왼손 
							if(InvenItem[i].CODE == InvenItem[sInven[0].ItemIndex-1].CODE){
								InvenItem[sInven[0].ItemIndex-1].Flag = 0;
								sInven[0].ItemIndex = 0; //인벤토리 박스 초기화 

							}

						}

					}
						 
				}
				//양손아이템이 한손에 있을경우에는 지워준다 
				if(InvenItem[i].Class == ITEM_CLASS_WEAPON_TWO){
					if(InvenItem[i].ItemPosition == 1 ){//오른손 
						if(InvenItem[i].CODE != InvenItem[sInven[1].ItemIndex-1].CODE){
							InvenItem[sInven[1].ItemIndex-1].Flag = 0;
							InvenItem[i].Flag = 0;
							sInven[1].ItemIndex = 0; //인벤토리 박스 초기화 
							sInven[0].ItemIndex = 0; //인벤토리 박스 초기화 
							if(InvenItem[i].SetModelPosi)
								sinSetCharItem(InvenItem[i].CODE , InvenItem[i].SetModelPosi, FALSE); 

						}
					}
					if(InvenItem[i].ItemPosition == 2 ){//왼손 
						if(InvenItem[i].CODE != InvenItem[sInven[0].ItemIndex-1].CODE){
							InvenItem[sInven[0].ItemIndex-1].Flag = 0;
							InvenItem[i].Flag = 0;
							sInven[0].ItemIndex = 0; //인벤토리 박스 초기화 
							sInven[1].ItemIndex = 0; //인벤토리 박스 초기화 
							if(InvenItem[i].SetModelPosi)
								sinSetCharItem(InvenItem[i].CODE , InvenItem[i].SetModelPosi, FALSE); 

						}
					}

				}


			}


			if(!InvenItem[i].lpItem){ //이미지가 없어도 아이템을 지우지않는다 
				cMessageBox.ShowMessage(MESSAGE_NOTEXIT_ITEMIMAGE);
			}
			//	InvenItem[i].Flag = 0;

			/////////////////////인증 코드 
			if(InvenItem[i].ItemPosition == 2){ //오른손 
				if(InvenItem[i].Class == ITEM_CLASS_WEAPON_TWO)
					continue; //인증코드가 없는 투핸드 무기는 체크하지않는다 

			}
		
			if(!CheckItemForm(&InvenItem[i].sItemInfo)){
				SendSetHackUser(FALSE); //해킹을 하려고했던 못된 유저를 고발 TRUE 접속 종료 
				InvenItem[i].Flag = 0;  //치트된 아이템을 없애버힌다 
				if(InvenItem[i].ItemPosition){
					sInven[InvenItem[i].ItemPosition-1].ItemIndex = 0;
					sinSetCharItem(InvenItem[i].CODE , InvenItem[i].SetModelPosi, FALSE); //무기와 갑옷을 셋팅한다 

				}
				if(InvenItem[i].Class == ITEM_CLASS_WEAPON_TWO){
					InvenItem[sInven[1].ItemIndex-1].Flag = 0;
					sInven[1].ItemIndex = 0;
				}
			}
		
		}

	}
	if(!ArmorFlag)
		sinSetCharItem(NULL , INVENTORY_POS_ARMOR, FALSE); //무기와 갑옷을 셋팅한다 


	ReformCharForm();//재인증 
	LoadCharState(); //로딩이 끝난후에 캐릭터 정보를 셋팅한다 

	SearchJobAndSetting();
	ReFormInvenItem(); //아이템의 데이타를 셋팅한다 

	CheckWeight(); //무게를 체크한다 

	cSkill.SearchUseSkill(); //사용할 스킬을 찾고 스킬이미지를 로드한다 
	SetItemToChar(); //초기화 할때 넣어준다 

	ReFormPotionNum(); //포션 갯수 체크 
	PotionCheckNumJumpFlag = 0;
	CheckRequireItem(); //아이템을 사용할수있는지를 체크한다

	/////////마우스 아이템로딩 
	if(MouseItem.Flag){
		for(int j=0 ; j < MAX_ITEM ; j++){
			if(MouseItem.sItemInfo.CODE == sItem[j].CODE){
				memcpy(&sinTempItem,&sItem[j],sizeof(sITEM));
				memcpy(&sinTempItem.sItemInfo,&MouseItem.sItemInfo,sizeof(sITEMINFO));
				wsprintf(szFilePath ,"Image\\sinImage\\Items\\%s\\it%s.bmp",sItem[j].ItemFilePath,sItem[j].LastCategory);
				if(!sItem[j].lpTempItem) //이미지를 템프로 로드 
					sItem[j].lpTempItem = LoadDibSurfaceOffscreen(szFilePath); 
				sinTempItem.lpItem = sItem[j].lpTempItem;
				sinTempItem.x = MouseItem.x;
				sinTempItem.y = MouseItem.y;
				sinTempItem.ItemPosition = MouseItem.ItemPosition;
				sinTempItem.Flag = 1;
				ReFormInvenItem(); //아이템의 데이타를 셋팅한다 
				memcpy(&MouseItem,&sinTempItem,sizeof(sITEM));
				cInvenTory.AutoSetInvenItem(&MouseItem); //마우스아이템을 인벤토리에 셋팅한다 
				break;

			}

		}

	}

	//아이템의 위치를 보정해준다 
	CheckResetInvenItem(); 
	cSkill.InitReformSkillPointForm(); //스킬 포인트정보를 저장한다 
	ReFormMousePotionNum(); //마우스아이템을 ReForm해준다
	CheckSpecialItemPosi(); //퀘스트 아이템 소팅
	ChangeSpecialItem(2); //셋팅을 바꾼다
	CheckChangeJob_QuestItem();
	CheckPuzzle();
	cHelpPet.CheckPetData();
	cPCBANGPet.CheckPetData();
	ReSetQuestItemPosi();
	return TRUE;
}

//특별한 아이템을 퀘스트 인벤토리에 셋팅한다 (해상도 변경으로 인한 좌표변경을 보정해준다)
int cINVENTORY::ReSetQuestItemPosi()
{

	sITEM *pItem=0;
	for(int  i = 0; i < INVENTORY_MAXITEM*2 ; i++){
		if(i<100){
			if(!InvenItem[i].Flag)continue;
			pItem = &InvenItem[i];
		}
		else{
			if(!InvenItemTemp[i-100].Flag)continue;
			pItem = &InvenItemTemp[i-100];
		}
		if(pItem->x >1000 && pItem->x < 2000){
			pItem->x += 9000;
		}
	}
	return TRUE;
}

int cINVENTORY::SetSpecialItem(sITEM *pItem)
{
	if(pItem->sItemInfo.SpecialItemFlag[1] != ITEM_KIND_SPECIAL)return FALSE;

	int cntx,cnty;
	int cx,cy;

	RECT	rect;
	int		flag;

	cx = (44*6)-pItem->w; //아이템이 들어갈수있는 가장 끝 X좌표 
	cy = (44*3)-pItem->h; //아이템이 들어갈수있는 가장 끝 Y좌표

	int TempWidth = 0;
	if(SpecialItemShowFlag){
		TempWidth = 0;
	}
	else
		TempWidth = 10000;

	//코드가 같지 않거나 아이템이 비어있지 않은 곳에 셋팅할때 검색해서 위치를 찾아낸다 
	for( cntx=(StartX+TempWidth) ; cntx <= StartX+TempWidth+cx ; cntx+=44){
		for( cnty=StartY ; cnty <= StartY+cy ; cnty+=44){ 
			flag = 0;
			for(int i=84 ; i<INVENTORY_MAXITEM ; i++){
				if(InvenItem[i].Flag && InvenItem[i].sItemInfo.SpecialItemFlag[1] ==ITEM_KIND_SPECIAL){
					if(InvenItem[i].x < TempWidth){
						rect.left   = InvenItem[i].x+TempWidth;
						rect.right  = InvenItem[i].x+TempWidth+InvenItem[i].w;
					}
					else{
						rect.left   = InvenItem[i].x;
						rect.right  = InvenItem[i].x+InvenItem[i].w;
					}
					rect.top    = InvenItem[i].y;
					rect.bottom = InvenItem[i].y+InvenItem[i].h;

					if( cntx > rect.left-pItem->w && cntx< rect.right && 
						cnty > rect.top-pItem->h && cnty< rect.bottom )
						flag++;
				}
			}
			if ( flag==0 ) {
				pItem->SetX = cntx+((44-pItem->w)/2);
				pItem->SetY = cnty+((44-pItem->h)/2);
				pItem->ItemPosition = 0; 

				for(int j=84 ; j < INVENTORY_MAXITEM ; j++){
					if(InvenItem[j].Flag == 0){
						CheckInvenItemForm();
						memcpy(&InvenItem[j],pItem,sizeof(sITEM));
						InvenItem[j].x = pItem->SetX;
						InvenItem[j].y = pItem->SetY;
						ReFormInvenItem();
						SetItemToChar();
						sinPlaySound(InvenItem[j].SoundIndex);
						CheckWeight();   //무게를 체크 

						//////////////// 요기서 서버로 메세지를 보낸다 
						SendCheckItemToServer(&InvenItem[j].sItemInfo);
						CheckDamage(); //최대데미지가 나올 무기로 계산하여 서버로 보내준다 
						ReFormPotionNum(); //포션카운트를 갱신해준다
						return TRUE;
					}
				}
			}
		}
	}

	return FALSE;
}

//오토 셋팅 
int cINVENTORY::AutoSetInvenItem(sITEM *pItem , int PostBox )
{

	char szBuff[128];
	//Special아이템을 체크한다
	if(SetSpecialItem(pItem))return TRUE; 

	ChangeSpecialItem(2); //무조건 퀘스트인벤토리는 닫는다
	/*
	//Npc를 통해서 받을때는 퀘스트인벤토리를 바꿔준다
	if(PostBox){
		ChangeSpecialItem(2);
	}
	*/
	if(!PostBox){ //아이템을 npc를 통해서 받을때는 체크하지않는다
		if(!CheckSetOk(pItem,1)) //셋팅될수있느지를 체크한다 
			return FALSE;
	}

	//퍼즐을 강제 셋팅한다
	if(PostBox==2){ 
		pItem->SetX = StartX+(22*8);
		pItem->SetY = StartY;
		if(LastSetInvenItem(pItem,1)){  //완전히 셋팅이 됬을경우 TRUE를 리턴한다 
			wsprintf(szBuff,"%s %s",pItem->sItemInfo.ItemName,RecvItemTT);
			cMessageBox.ShowMessageEvent(szBuff);
			return TRUE;
		}
	}
	//받은아이템을 표시만한다
	if(PostBox==3){
		wsprintf(szBuff,"%s %s",pItem->sItemInfo.ItemName,RecvItemTT);
		cMessageBox.ShowMessageEvent(szBuff);
	}

	if(AutoSetPotion(pItem))return TRUE; //포션을 자동으로 셋팅한다 


	//먼저 무기구 박스를 체크한다 
	for(int i=0 ; i < INVENTORY_MAX_POS ; i++){
		if(pItem->Class & sInven[i].Position){
			if(i==INVENTORY_POS_SLOT_COSTUME && !sinCostumeFits(pItem->CODE , sinChar->JOB_CODE))continue;	// not this class's costume: to the bag
			if(!sInven[i].ItemIndex){
				if(sInven[i].ItemIndex)break;
				if(pItem->Class == ITEM_CLASS_WEAPON_TWO) //양손영역에 아이템이 있으면 셋팅할수 없다 
					if(sInven[0].ItemIndex || sInven[1].ItemIndex)break;
				if(pItem->sItemInfo.NotUseFlag)break; //능력치가 부족하면 셋팅할수 없다 
				pItem->SetX = sInven[i].Rect.left + (((sInven[i].Rect.right  - sInven[i].Rect.left) - pItem->w)/2);
				pItem->SetY = sInven[i].Rect.top  + (((sInven[i].Rect.bottom - sInven[i].Rect.top) - pItem->h)/2);
				pItem->ItemPosition = i+1;  //셋팅영역 중에서 아이템이 셋팅될 영역을 저장한다 (오른손 , 왼손 ,갑옷 ...) 
				if(LastSetInvenItem(pItem,1))  //완전히 셋팅이 됬을경우 TRUE를 리턴한다 
					return TRUE;
	
			}
		}
	}
	if(InvenEmptyAearCheck(pItem))return TRUE; //박스의 빈곳을 찾아서 셋팅한다

	cMessageBox.ShowMessage(MESSAGE_OVER_SPACE); //이곳에 오기전에 리턴되지 않으면 공간부족 메세지를 띄워준다 
	sinOverWeightCnt++; //공간부족도 같이쓴다
	return FALSE;
}

//인벤토리에서의 빈곳을 체크한다 
int cINVENTORY::InvenEmptyAearCheck(sITEM *pItem)
{
	int cntx,cnty;
	int cx,cy;

	RECT	rect;
	int		flag;

	if(AutoSetPotionToBox(pItem))return TRUE;

	cx = (22*INVENTORY_BOX_ROW)-pItem->w; //아이템이 들어갈수있는 가장 끝 X좌표 
	cy = (22*INVENTORY_BOX_COL)-pItem->h; //아이템이 들어갈수있는 가장 끝 Y좌표

	//코드가 같지 않거나 아이템이 비어있지 않은 곳에 셋팅할때 검색해서 위치를 찾아낸다 
	for( cntx=StartX ; cntx <= StartX+cx ; cntx+=22){
		for( cnty=StartY ; cnty <= StartY+cy ; cnty+=22){ 
			flag = 0;
			for(int i=0 ; i<INVENTORY_MAXITEM ; i++){
				if(InvenItem[i].Flag){
					rect.left   = InvenItem[i].x;
					rect.right  = InvenItem[i].x+InvenItem[i].w;
					rect.top    = InvenItem[i].y;
					rect.bottom = InvenItem[i].y+InvenItem[i].h;

					if( cntx > rect.left-pItem->w && cntx< rect.right && 
						cnty > rect.top-pItem->h && cnty< rect.bottom )
						flag++;
				}

			}
			if ( flag==0 ) {
				pItem->SetX = cntx;
				pItem->SetY = cnty;
				pItem->ItemPosition = 0; 
				LastSetInvenItem(pItem,1); //오토셋팅일경우에는 인자1을 넘긴다 
					return TRUE;
			}
		}
	}
return FALSE;
}

//인벤토리 아이템으로 복사한다 
int cINVENTORY::LastSetInvenItem(sITEM *pItem , int AutoSet)
{
	int PassFlag=0; //물약 때문에 처리한다 
	if(!AutoSet) //오토셋팅이 아닐경우에만 처리한다 
		if(!InvenNotOpenCheck(pItem))return FALSE; //인벤 로직수행여부를 체크한다 

	for(int j=0 ; j < INVENTORY_MAXITEM ; j++){
		if(InvenItem[j].Flag == 0){
			if(sinPosionItem){ //겹쳐진 물약 처리를 위해 코드 삽입 ㅎㅎ 
				if(sinPosionItem->sItemInfo.ItemHeader.Head == InvenItem[j].sItemInfo.ItemHeader.Head)
					continue;

			}
			CheckInvenItemForm();
			memcpy(&InvenItem[j],pItem,sizeof(sITEM));
			InvenItem[j].x = pItem->SetX;
			InvenItem[j].y = pItem->SetY;
			if(InvenItem[j].ItemPosition){ //셋팅될 포지션이 있으면 값을 셋팅한다 
				SetTwoHandItem(&InvenItem[j]);//투핸드 무기인지를 체크한다 
				sInven[InvenItem[j].ItemPosition-1].ItemIndex = j+1;//셋팅될때 인벤영역에도 아이템 인덱스를 셋팅한다
				sinSetCharItem(InvenItem[j].CODE , pItem->SetModelPosi, TRUE); //무기와 갑옷을 셋팅한다 
			
			}
			pItem->Flag = 0; //초기화 
			ReFormInvenItem();
			SetItemToChar();
			sinPlaySound(InvenItem[j].SoundIndex);
			CheckWeight();   //무게를 체크 
			//////////////물약의 위치를 보정한다
			if(InvenItem[j].x < StartX || InvenItem[j].x > 800 || InvenItem[j].y < StartY || InvenItem[j].y > 600){
				AutoSetInvenItem(&InvenItem[j]);
			}

			//////////////// 요기서 서버로 메세지를 보낸다 
			SendCheckItemToServer(&InvenItem[j].sItemInfo);
			CheckDamage(); //최대데미지가 나올 무기로 계산하여 서버로 보내준다 
			ReFormPotionNum(); //포션카운트를 갱신해준다

			//퍼즐을 체크한다
			CheckPuzzle();
			return TRUE;
			
		}
	}
	ReFormInvenItem();
	return FALSE;
}
//////////////////////////////////
//         *포션 
//////////////////////////////////
int cINVENTORY::UsePotion(int x , int y)
{   //포션을 사용한다 {
	for(int i=0 ; i < INVENTORY_MAXITEM ; i++){
		if(InvenItem[i].Flag){
			if ( InvenItem[i].x < x  && InvenItem[i].x + InvenItem[i].w > x  && 
				InvenItem[i].y < y  && InvenItem[i].y + InvenItem[i].h > y ){
				if(InvenItem[i].ItemPosition == 11 || InvenItem[i].ItemPosition == 12 || InvenItem[i].ItemPosition == 13){
					if(InvenItem[i].Class == ITEM_CLASS_POTION){ //물약일경우는 사용한다 
						if(sinActionPotion()){ //포션을 사용하는 모션을 취한다  모션이 끝난후 물약을 사용한다 
							pUsePotion = &InvenItem[i]; //사용할 포션을 백업한다 
							sinUsePotionDelayFlag = 1;
						}
						
					}

				}
				else{
					if(!OpenFlag)break;
					if(InvenItem[i].Class == ITEM_CLASS_POTION){ //물약일경우는 사용한다 
						if(sinActionPotion()){ //포션을 사용하는 모션을 취한다  모션이 끝난후 물약을 사용한다 
							pUsePotion = &InvenItem[i]; //사용할 포션을 백업한다 
							sinUsePotionDelayFlag = 1;
						}
					}
				}
			}
		}
	}
	ReFormPotionNum();	//포션 갯수를 체크한다
	return FALSE;

}

int cINVENTORY::ReSettingPotion()
{ //포션 공간이 변했을 경우 포션을 셋팅한다 
	sITEM TempPotionItem;
	int i=0,j=0,cnt=0;
	
	memset(&TempPotionItem,0,sizeof(sITEM));
	for(i=10 ; i < 13; i++){ //포션 세팅박스만 검사한다 
		if(sInven[i].ItemIndex){
			if(InvenItem[sInven[i].ItemIndex-1].Flag){
				memcpy(&TempPotionItem,&InvenItem[sInven[i].ItemIndex-1],sizeof(sITEM));
				cnt = TempPotionItem.sItemInfo.PotionCount;
				if(sinChar->Potion_Space < InvenItem[sInven[i].ItemIndex-1].sItemInfo.PotionCount){ //포션 할당 공간보다 포션이 더있으면 
					TempPotionItem.sItemInfo.PotionCount = InvenItem[sInven[i].ItemIndex-1].sItemInfo.PotionCount -sinChar->Potion_Space;
					InvenItem[sInven[i].ItemIndex-1].sItemInfo.PotionCount -= TempPotionItem.sItemInfo.PotionCount;
					if(!InvenEmptyAearCheck(&TempPotionItem)){
						NoSettingThrowItem(&TempPotionItem);// 셋팅이 안될때는 바닥에 아이템을 버린다 
					}

				}
			}
		
		}

	}
	ReFormPotionNum();	//포션 갯수를 체크한다

	return TRUE;
}
int BackUpX,BackUpY,BackUpPosi=0;
int cINVENTORY::MouseSetPotion(sITEM *pItem)
{   //포션을 마우스로 셋팅한다 

	int i=0 , cnt;
	sITEM TempPotionItem;
	sinPosionItem = 0 ; //초기화 

	if(!pItem->Flag)return FALSE;
	BackUpItemIndex = 0;
	if(CrashItemIndex[0]){ //겹쳐진 아이템이 있으면 
		if(pMessageItem){
			BackUpX = pMessageItem->x;
			BackUpY = pMessageItem->y;
			BackUpPosi = pMessageItem->ItemPosition;
			sinPosionItem  = &InvenItem[CrashItemIndex[0]-1];
			BackUpItemIndex = CrashItemIndex[0];

		}
		if(pItem->ItemPosition){ //포션밖스에서 겹칠경우에 
			memcpy(&TempPotionItem,pItem,sizeof(sITEM));
			if(pItem->CODE == InvenItem[CrashItemIndex[0]-1].CODE){ //같은 코드일경우 
				cnt = pItem->sItemInfo.PotionCount;
				for(i = 0 ; i < cnt ; i++){
					if(sinChar->Potion_Space > InvenItem[CrashItemIndex[0]-1].sItemInfo.PotionCount){
						InvenItem[CrashItemIndex[0]-1].sItemInfo.PotionCount++;
						pItem->sItemInfo.PotionCount--;
						if(pItem->sItemInfo.PotionCount <= 0 ){
							pItem->Flag = 0; //인덱스를 다 증가했으면 아이템을 지워준다 
							sinPlaySound(pItem->SoundIndex);
							ReFormPotionNum();	//포션 갯수를 체크한다
							return TRUE; //인덱스를 증가해줄게 없으면 리턴 
						}
					}
					else{ 
						if(InvenEmptyAearCheck(pItem)){ //나머지 포션은 창고로 셋팅 						
							ReFormPotionNum();	//포션 갯수를 체크한다
							return TRUE;
						}
						else{
							ResetPotion2();
							if(sinThrowItemToFeild(pItem)){
								ReformCharForm();//재인증
								cInvenTory.ReFormPotionNum();	//물약 갯수 체크
								return TRUE;
							}
							else{
								ReformCharForm();//재인증
								cInvenTory.ReFormPotionNum();	//물약 갯수 체크
								return FALSE;

							}
						}							

					}
				}

			}
			else{ //다른 코드일 경우 
				memcpy(&TempItem,&InvenItem[CrashItemIndex[0]-1],sizeof(sITEM)); //인벤토리 아이템을 템프로 복사 
				InvenItem[CrashItemIndex[0]-1].Flag = 0;
				if(sinPosionItem)
					sinPosionItem->Flag = 0;
				TempPotionItem.sItemInfo.PotionCount = 0; //포션 카운트 초기화 
				cnt = pItem->sItemInfo.PotionCount;
				for(i = 0 ; i < cnt ; i++){
					if(sinChar->Potion_Space < pItem->sItemInfo.PotionCount){
						pItem->sItemInfo.PotionCount--;
						TempPotionItem.sItemInfo.PotionCount++;
					}
					else{
						ReFormInvenItem();
						if(LastSetInvenItem(pItem)){  //포션셋팅위치에 셋팅 후 (겹쳐졌을때에는 아이템 셋팅을 2번 건너띤후 한다 )
							if(TempPotionItem.sItemInfo.PotionCount > 0){
								if(InvenEmptyAearCheck(&TempPotionItem)){ //나머지 포션은 창고로 셋팅 
									memcpy(pItem,&TempItem,sizeof(sITEM));
									pMessageItem = sinPosionItem;
									pMessageItem->x = BackUpX;
									pMessageItem->y = BackUpY;
									pMessageItem->ItemPosition = BackUpPosi;
									sinPosionItem = 0;
									ReFormPotionNum();	//포션 갯수를 체크한다
									return TRUE;
								}
								else{
									//물약 사라짐 방지
									memcpy(pItem,&TempItem,sizeof(sITEM));
									if(sinPosionItem){
										pMessageItem = sinPosionItem;
										pMessageItem->x = BackUpX;
										pMessageItem->y = BackUpY;
										pMessageItem->ItemPosition = BackUpPosi;
										sinPosionItem = 0;
									}
									sinThrowItemToFeild(&TempPotionItem);					 
									ReFormPotionNum();	//포션 갯수를 체크한다
									return TRUE;
								}
							}
							memcpy(pItem,&TempItem,sizeof(sITEM));
							if(sinPosionItem){
								pMessageItem = sinPosionItem;
								pMessageItem->x = BackUpX;
								pMessageItem->y = BackUpY;
								pMessageItem->ItemPosition = BackUpPosi;
								sinPosionItem = 0;
							}
							ReFormPotionNum();	//포션 갯수를 체크한다
							return TRUE;
						}
						else{
							ReFormPotionNum();	//포션 갯수를 체크한다
							return FALSE;
						}

					}

				}

			}

		}
		else{ //창고에서 겹칠경우에 
			if(pItem->CODE == InvenItem[CrashItemIndex[0]-1].CODE){ //같은 코드일경우 
				/////////////// 물약을 복사하던 놈들을 잡기위해서 함정을판다 
				if(!cInvenTory.OpenFlag){
					SendSetHackUser2(1960,3);
					return TRUE;
				}
				
				InvenItem[CrashItemIndex[0]-1].sItemInfo.PotionCount += pItem->sItemInfo.PotionCount;
				pItem->Flag = 0; //포션 카운트를 더해준다음 초기화한다 
				sinPlaySound(pItem->SoundIndex);
				ReFormPotionNum();	//포션 갯수를 체크한다
				return TRUE;
			}
			else{
				memcpy(&TempItem,&InvenItem[CrashItemIndex[0]-1],sizeof(sITEM)); //마우스 아이템을 템프로 복사 
				InvenItem[CrashItemIndex[0]-1].Flag = 0 ; //복사한후에 초기화

				TempItem.x = pItem->x;
				TempItem.y = pItem->y;
				ReFormInvenItem();
				LastSetInvenItem(pItem); //마우스 아이템을 인벤토리로 복사 
				memcpy(pItem,&TempItem,sizeof(sITEM)); //템프아이템을 마우스로 복사 
				if(sinPosionItem){ //포션일경우에만 포인터에 값을 셋팅 
					pMessageItem = sinPosionItem;
					pMessageItem->x = BackUpX;
					pMessageItem->y = BackUpY;
					pMessageItem->ItemPosition = BackUpPosi;
					sinPosionItem = 0;

				}
				ReFormPotionNum();	//포션 갯수를 체크한다
				return TRUE;
			}
		}
	}
	else{ //아니면 
		if(pItem->ItemPosition){  //포션 박스이면 
			memcpy(&TempPotionItem,pItem,sizeof(sITEM));
			TempPotionItem.sItemInfo.PotionCount = 0; //포션 카운트 초기화 
			cnt = pItem->sItemInfo.PotionCount;
			for(i = 0 ; i < cnt ; i++){
				if(sinChar->Potion_Space < pItem->sItemInfo.PotionCount){
					pItem->sItemInfo.PotionCount--;
					TempPotionItem.sItemInfo.PotionCount++;

				}
				else{
					ReFormInvenItem();
					if(LastSetInvenItem(pItem)){  //포션셋팅위치에 셋팅 후 
						if(TempPotionItem.sItemInfo.PotionCount > 0){
							if(InvenEmptyAearCheck(&TempPotionItem)){ //나머지 포션은 창고로 셋팅 
								ReFormPotionNum();	//포션 갯수를 체크한다
								return TRUE;
							}
							else{
								//물약 사라짐 방지 
								sinThrowItemToFeild(&TempPotionItem);//아이템을 버린다
								ReFormPotionNum();	//포션 갯수를 체크한다
								return FALSE;
							}
						}
						ReFormPotionNum();	//포션 갯수를 체크한다
						return TRUE;
					}
					else{
						ReFormPotionNum();	//포션 갯수를 체크한다
						return FALSE;
					}
				}
			}
		}
		else{ //창고이면 그냥 셋팅한다 
			ReFormInvenItem();
			if(LastSetInvenItem(pItem)){
				ReFormPotionNum();	//포션 갯수를 체크한다
				return TRUE;
			}

		}

	}
	ReFormPotionNum();	//포션 갯수를 체크한다
	return FALSE;
}

int cINVENTORY::AutoSetPotionToBox(sITEM *pItem)
{ //포션을 창고에 세팅한다 
	
	if(pItem->Class != ITEM_CLASS_POTION)return FALSE; //포션이 아니면 리턴한다 
	for(int i=0 ; i < INVENTORY_MAXITEM ; i++){
		if(InvenItem[i].Flag){
			if(pItem->CODE == InvenItem[i].CODE){//같은 포션이면
				if(InvenItem[i].ItemPosition)continue; //포션 박스에 있으면 continue
				InvenItem[i].sItemInfo.PotionCount += pItem->sItemInfo.PotionCount;
				pItem->Flag = 0;
				sinPlaySound(pItem->SoundIndex);
				///////////////////// 포션이 잘못들어가는 경우를 체크해서 보정해준다 
				if(InvenItem[i].x < 3 || InvenItem[i].y < 10)
					ReSetPotionXY(&InvenItem[i]);
				ReformCharForm();//재인증
				cInvenTory.ReFormPotionNum();	//물약 갯수 체크
					
				return TRUE;

			}
		}
	}

	return FALSE;
}
int cINVENTORY::AutoSetPotion(sITEM *pItem)
{	//포션을 자동으로 셋팅한다 


	int i=0,j=0,cnt=0;
	sITEM TempPotion;

	
	if(pItem->Class != ITEM_CLASS_POTION)return FALSE; //포션이 아니면 리턴한다  
	for( i=0 ; i < INVENTORY_MAX_POS ; i++){  
		if(pItem->Class & sInven[i].Position){ //포션이 셋팅될수 있는 영역이면 
			if(sInven[i].ItemIndex){ //현재 포션 박스에 아이템이 있으면 코드를 검색해 아이템을 더한다 
				if(pItem->CODE == InvenItem[sInven[i].ItemIndex-1].CODE){ //같은 코드의 포션일경우 
					if(sinChar->Potion_Space == InvenItem[sInven[i].ItemIndex-1].sItemInfo.PotionCount)continue; //코드는 같지만 공간이 부족하면 
					cnt = pItem->sItemInfo.PotionCount;
					for( j = 0 ; j < cnt ; j++){ //포션의 갯수만큼 루프를 돈다 
						if(sinChar->Potion_Space > InvenItem[sInven[i].ItemIndex-1].sItemInfo.PotionCount){ //물약 보유공간보다 작으면 
							InvenItem[sInven[i].ItemIndex-1].sItemInfo.PotionCount++;
							pItem->sItemInfo.PotionCount--;
							sinPlaySound(pItem->SoundIndex);
							if(pItem->sItemInfo.PotionCount <= 0){
								pItem->Flag = 0 ;
								ReformCharForm();//재인증
								cInvenTory.ReFormPotionNum();	//물약 갯수 체크
								return TRUE; //물약을 다썼으면 리턴 
							}
						}
						else{ //물약 보유공간 만큼 크기가 차게되면 	
							if(InvenEmptyAearCheck(pItem)){ //나머지 포션을 인벤토리 공간에 넣어준다 
								ReformCharForm();//재인증
								cInvenTory.ReFormPotionNum();	//물약 갯수 체크
								return TRUE;
							}
							else{ //일단은 문제가 없나 해본다 
								ResetPotion2();
								if(sinThrowItemToFeild(pItem)){
									ReformCharForm();//재인증
									cInvenTory.ReFormPotionNum();	//물약 갯수 체크
									return TRUE;

								}
							}
						}
					}
				}
				else //코드가 다르면 다시 for루프를 돈다 
					continue;

			}
			else{ //아이템이 없으면 셋팅한다 
				pItem->SetX = sInven[i].Rect.left + (((sInven[i].Rect.right  - sInven[i].Rect.left) - pItem->w)/2);
				pItem->SetY = sInven[i].Rect.top  + (((sInven[i].Rect.bottom - sInven[i].Rect.top) - pItem->h)/2);

				cnt = pItem->sItemInfo.PotionCount;
				//pItem->SetX = sInven[i].Rect.left; //좌표설정 
				//pItem->SetY = sInven[i].Rect.top;
				pItem->ItemPosition = i+1;
				memcpy(&TempPotion,pItem,sizeof(sITEM)); //아이템을 복사한다 (아이템 갯수가 있을경우 2곳으로 나뉘어야하기때문에)
				TempPotion.sItemInfo.PotionCount = 0; //포션 갯수 초기화 
				for(j = 0; j<cnt ; j++){
					if(sinChar->Potion_Space < pItem->sItemInfo.PotionCount){ //포션 아이템이 크면 
						pItem->sItemInfo.PotionCount--;
						TempPotion.sItemInfo.PotionCount++;

					}
					else{ //공간에 맞게 들어가면 
						if(LastSetInvenItem(pItem,1)){//포션셋팅위치에 셋팅 후 
							if(TempPotion.sItemInfo.PotionCount > 0){
								if(InvenEmptyAearCheck(&TempPotion)){ //나머지 포션은 창고로 셋팅 
									ReformCharForm();//재인증
									cInvenTory.ReFormPotionNum();	//물약 갯수 체크
									return TRUE;
								}
							}
							else{
								ReformCharForm();//재인증
								cInvenTory.ReFormPotionNum();	//물약 갯수 체크
								return TRUE; //자동 셋팅이 안돼 더라도 포션위치에 셋팅하면 리턴해준다 
							}
						}

					}
				}
			}
		}
	}
	
	return FALSE;
}
//////////////////////////////////
//         *투핸드
//////////////////////////////////
int cINVENTORY::ClearTwoHandPosiAndItem(sITEM *pItem)
{
	if(pItem->Class == ITEM_CLASS_WEAPON_TWO){
		if(sInven[0].ItemIndex){  //오른손 
			InvenItem[sInven[0].ItemIndex-1].Flag = 0;
			sInven[0].ItemIndex = 0;
			if(InvenItem[sInven[0].ItemIndex-1].ItemPosition){
				if(InvenItem[sInven[0].ItemIndex-1].SetModelPosi){				
					sinSetCharItem(InvenItem[sInven[0].ItemIndex-1].CODE , InvenItem[sInven[0].ItemIndex-1].SetModelPosi, FALSE); //무기와 갑옷을 셋팅한다 
				}

			}

		}
		if(sInven[1].ItemIndex){
			InvenItem[sInven[1].ItemIndex-1].Flag = 0;
			sInven[1].ItemIndex = 0;
			if(InvenItem[sInven[1].ItemIndex-1].ItemPosition){
				if(InvenItem[sInven[1].ItemIndex-1].SetModelPosi){				
					sinSetCharItem(InvenItem[sInven[1].ItemIndex-1].CODE , InvenItem[sInven[1].ItemIndex-1].SetModelPosi, FALSE); //무기와 갑옷을 셋팅한다 
				}
			}
		}
	}
	return TRUE;
}
int cINVENTORY::SetTwoHandItem(sITEM *pItem)
{ //셋팅 

	sITEM TempTwoHandItem;

	memset(&TempTwoHandItem,0,sizeof(sITEM)); //초기화 해준다 

	if(pItem->Class == ITEM_CLASS_WEAPON_TWO){ //투핸드 무기를 체크한다 
		if(pItem->ItemPosition == 2 ){ //왼손이면 
			pItem->ItemPosition = 1;   //아이템의 영역을 오른손으로 바꾼다  좌표를 셋팅한다 
			pItem->x = sInven[0].Rect.left + (((sInven[0].Rect.right - sInven[0].Rect.left)-pItem->w)/2);
			pItem->y = sInven[0].Rect.top  + (((sInven[0].Rect.bottom - sInven[0].Rect.top)-pItem->h)/2);	


		}
		TempTwoHandItem.x = sInven[1].Rect.left + (((sInven[1].Rect.right - sInven[1].Rect.left)-pItem->w)/2);
		TempTwoHandItem.y = sInven[1].Rect.top + (((sInven[1].Rect.bottom - sInven[1].Rect.top)-pItem->h)/2);
		TempTwoHandItem.w = pItem->w; 
		TempTwoHandItem.h = pItem->h;
		TempTwoHandItem.ItemPosition = 2; //아이템의 위치는 왼손 
		TempTwoHandItem.Class = pItem->Class; 
		TempTwoHandItem.sItemInfo.CODE = pItem->CODE; //코드를 넣어준다 (아이템Load시 아이템 구분인자로 적합)
		TempTwoHandItem.sItemInfo.Price = pItem->sItemInfo.Price; //가격을 넣어준다 
		memcpy(&TempTwoHandItem.lpItem,&pItem->lpItem,sizeof(LPDIRECTDRAWSURFACE4));
		TempTwoHandItem.Flag = 1;
		for(int j=0 ; j < INVENTORY_MAXITEM ; j++){
			if(InvenItem[j].Flag == 0){
				memcpy(&InvenItem[j],&TempTwoHandItem,sizeof(sITEM));
				sInven[1].ItemIndex = j+1; //셋팅될때 인벤영역에도 아이템 인덱스를 셋팅한다
				break;
			}
		}

	}

	return TRUE;
}

int cINVENTORY::PickUpTwoHandItem(sITEM *pItem)
{//선택 
	sITEM TempPickItem;
	memset(&TempPickItem,0,sizeof(sITEM));

	if(pItem->ItemPosition){ //무기 박스에서 집었을 경우에만 
		if(sInven[0].ItemIndex && sInven[1].ItemIndex){
			memcpy(&TempPickItem,&InvenItem[sInven[0].ItemIndex-1],sizeof(sITEM)); //오른손의 아이템을 복사한다 
			InvenItem[sInven[0].ItemIndex-1].Flag = 0; //오른손 아이템을 지워준다 
			InvenItem[sInven[1].ItemIndex-1].Flag = 0; //왼손 아이템을 지워준다 
			sInven[0].ItemIndex = 0; //무기셋팅 박스의 아이템 포지션도 지워준다 
			sInven[1].ItemIndex = 0; //무기셋팅 박스의 아이템 포지션도 지워준다 
			memcpy(pItem,&TempPickItem,sizeof(sITEM));
			ReFormInvenItem();
			ReformCharForm();//재인증 
			ReFormPotionNum();
			return TRUE;
		}
	}
	return FALSE;
}

//투핸드 아이템으로 겹치는 경우를 체크한다  (무기셋팅 영역일때만 호출된다 )
int cINVENTORY::OverlapTwoHandItem(sITEM *pItem)
{
	AutoSetItemIndex  = 0; //자동으로 셋팅될 아이템의 인덱스 
	if(pItem->Class == ITEM_CLASS_WEAPON_TWO){      //투핸드 아이템일 경우 
		if(sInven[1].ItemIndex){
			if(InvenItem[sInven[1].ItemIndex-1].Class !=ITEM_CLASS_WEAPON_TWO){ //현재 무기포지션에 있는 아이템이 투핸드 무기가 아닐경우에
				if(pItem->ItemPosition == 1 ){ //오른손 
					if(sInven[1].ItemIndex)
						AutoSetItemIndex = sInven[1].ItemIndex; //왼손 아이템의 인덱스를 저장한다 
				}
				if(pItem->ItemPosition == 2 ){ //왼손 
					if(sInven[0].ItemIndex)
						AutoSetItemIndex = sInven[0].ItemIndex; //오른손 아이템의 인덱스를 저장한다 
				}
			}
		}
	}
	return TRUE;

}

//투핸드 아이템으로 겹치는 경우를 체크한다  (무기셋팅 영역외의 영역 오른손에 아이템을 체크했을때 방패가 인덱스가온다)
int cINVENTORY::OverlapTwoHandSwitch(sITEM *pItem)
{
	if(pItem->Class == ITEM_CLASS_WEAPON_TWO){      //투핸드 아이템일 경우 
		if(pItem->ItemPosition == 1 ) //오른손 
			if(sInven[1].ItemIndex)
				CrashItemIndex[0] = sInven[1].ItemIndex;
		if(pItem->ItemPosition == 2 ) //왼손 
			if(sInven[0].ItemIndex)
				CrashItemIndex[0] = sInven[0].ItemIndex;

	}
	return TRUE;
}


//인벤토리 체크 여부 (물약박스를 체크하지 위해 )
int cINVENTORY::InvenNotOpenCheck(sITEM *pItem )
{
	if(!OpenFlag) //인벤토리가 닫혀있고  물약 포지션이 아니면 리턴한다 
		if(pItem->ItemPosition != 11 && pItem->ItemPosition != 12 && pItem->ItemPosition != 13)
			return FALSE; 
	return TRUE;
}

int cINVENTORY::PickUpInvenItem(int x , int y , int PickUpFlag)
{
	
	NotPotionFlag = 0; //플랙 초기화 
	sITEMPRICE sinItemPriceTemp; //아이템의 가격을 구해온다 
	SelectInvenItemIndex = 0;    //집을 아이템 
	SelectPotionIndex    = 0;    //포션 오토셋 인덱스 
	int kk = 0;
	for(int i=0 ; i < INVENTORY_MAXITEM ; i++){
		if(InvenItem[i].Flag && !InvenItemHidden(&InvenItem[i])){		// not on the page that is not shown
			if ( InvenItem[i].x < x  && InvenItem[i].x + InvenItem[i].w > x  &&
				InvenItem[i].y < y  && InvenItem[i].y + InvenItem[i].h > y ){
				if(!InvenNotOpenCheck(&InvenItem[i]))return FALSE;
				if(PickUpFlag == 1){ 
					if(MyShopItemIndex[i])return FALSE;
					/////////////////// 아이템을 초기화했을경우 여기서 무게관련부분을 체크해준다
					if(InvenItem[i].ItemPosition){
						if(sinChar->Weight[0] > sinChar->Weight[1]){
							cMessageBox.ShowMessage(MESSAGE_OVER_WEIGHT);
							sinOverWeightCnt++;
							return FALSE;
						}
					}
					/////////////////// 이미지가 없는 아이템은 집을수없다 
					if(!InvenItem[i].lpItem){ 
						cMessageBox.ShowMessage(MESSAGE_NOTEXIT_ITEMIMAGE);
						return FALSE;
					}
					/*
					if(InvenItem[i].sItemInfo.ItemKindCode == ITEM_KIND_QUEST_WEAPON &&
						InvenItem[i].ItemPosition && InvenItem[i].sItemInfo.ItemAgingNum[0] < 4){ //퀘스트가 끝나믄 집을수있따
						cMessageBox.ShowMessage(MESSAGE_NOT_PICKUP_ITEM);
						return FALSE;
					}
					*/

					//허상 아이템은 따로 체크한다 (양손무기)
					/*
					if(InvenItem[i].Class  == ITEM_CLASS_WEAPON_TWO ){
						if(sInven[0].ItemIndex){
							if(InvenItem[sInven[0].ItemIndex-1].sItemInfo.ItemKindCode == ITEM_KIND_QUEST_WEAPON &&
								InvenItem[i].ItemPosition && InvenItem[sInven[0].ItemIndex-1].sItemInfo.ItemAgingNum[0] < 4 ){
								cMessageBox.ShowMessage(MESSAGE_NOT_PICKUP_ITEM);
								return FALSE;
							}
						}
					}
					*/

					//============ 텔레포트코어 무한사용을 막는다 ( 성근 060619 )
				
					for(int j=0 ; j<SIN_MAX_HELP_NUMBER ; j++)
					{
						if(sSinHelp[j].KindFlag == SIN_HELP_KIND_TELEPORT_SCROLL   )
							return FALSE; 
					}

					//if(!CheckSpecialItem(&InvenItem[i]))return FALSE; //특별한 아이템일 경우에는 집을 수가없다 
					if(CursorClass == SIN_CURSOR_REPAIR){ //아이템 수리이면 
						sinItemPriceTemp = GetInvenItemPrice(&InvenItem[i]); //아이템의 가격을 구해온다
						if(RepairInvenItem(&InvenItem[i],sinItemPriceTemp.RepairCost)){
							sinPlaySound(SIN_SOUND_REPAIR_ITEM);	
						}
						return TRUE;
					}
					// 아이템의 인벤 복귀를 위해서 포인터에 값을 셋팅한다 
					InvenItem[i].OldX = InvenItem[i].x; //좌표를 저장한다  
					InvenItem[i].OldY = InvenItem[i].y; //
					pMessageItem =	&InvenItem[i]; //포인터로 가지고 있는다
					PotionIndex = i+1; //검사를 위해 인덱스 증가 
					BackUpItemIndex = i+1; 
					
					if(CursorClass == SIN_CURSOR_SELL){
						//투핸드 아이템의 허상은 팔수엄따
						if(!InvenItem[i].sItemInfo.ItemHeader.dwChkSum)return TRUE;

						//등록된 아이템 코드와 마스크 종류는 팔수없다
						for(kk = 0 ; kk < NotSell_Item_CODECnt ; kk++){
							if(NotSell_Item_CODE[kk] == InvenItem[i].sItemInfo.CODE)return TRUE;

						}
						for(kk = 0 ; kk < NotSell_Item_MASKCnt ; kk++){
							if((InvenItem[i].sItemInfo.CODE & sinITEM_MASK2) == NotSell_Item_MASK[kk])return TRUE;

						}
						for(kk = 0 ; kk < NotSell_Item_KINDCnt ; kk++){
							if(NotSell_Item_KIND[kk] == InvenItem[i].sItemInfo.ItemKindCode)return TRUE;

						}
						
						if(InvenItem[i].Class == ITEM_CLASS_POTION || 
							InvenItem[i].sItemInfo.CODE == (sinGF1|sin01) ){ //|| InvenItem[i].sItemInfo.CODE == (sinGF1|sin02)){
							//if(pMessageItem){ //인벤토리에서 집은 아이템일경우에만 
							//	cMessageBox.ShowMessage2(MESSAGE_SELL_ITEM);
								return TRUE;
							//}

						}
						else{
							if(!cShop.SellItemToShop(&InvenItem[i])) //아이템을 판다 (비비 꼬여있음)
								return TRUE;
						}

					}
					else{
						if(InvenItem[i].Class  == ITEM_CLASS_WEAPON_TWO ){ //투핸드 무기이면 
							PickUpTwoHandItem(&InvenItem[i]); //오른손 아이템을 복사하고 왼손은 지워준다 
						}
						memcpy(&MouseItem , &InvenItem[i] , sizeof(sITEM));

					}

					InvenItem[i].Flag = 0; //먼저 아이템을 없애준다
					if(InvenItem[i].ItemPosition){ //착용된 무기일 경우에만 
						sinSetCharItem(InvenItem[i].CODE , InvenItem[i].SetModelPosi, FALSE); //무기와 갑옷을 셋팅한다 
						sInven[InvenItem[i].ItemPosition-1].ItemIndex = 0; //사용되지 않는다 					

					}
					ReFormInvenItem();  //포션을 셋팅해주기전에 체크를 해준다 
					CheckWeight();		//무게 체크 
					SetItemToChar();	//아이템 셋팅 
					sinPlaySound(InvenItem[i].SoundIndex);
					ColorIndex = 0;
					//퍼즐을 체크한다
					CheckPuzzle();
					cHelpPet.PetMessage("*InvenTory_Item",1);
					return TRUE;		
				}
				//개인 상점에 아이템 등록 
				else if(PickUpFlag == 2){  
					if(MyShopSendButton)return FALSE; 
					if(MyShopItemIndex[i])return FALSE;
					if(InvenItem[i].ItemPosition)return FALSE;
					//별상품권은 팔수엄따!
					if(InvenItem[i].sItemInfo.CODE == (sinGF1|sin01))return FALSE;
					//if(InvenItem[i].sItemInfo.CODE == (sinGF1|sin02))return FALSE;

					// 장별 - 하트링은 팔수 없다
					if(InvenItem[i].sItemInfo.CODE == (sinOR2|sin33))return FALSE;

					// 장별 - 눈결정 목걸이는 팔수 없다
					if(InvenItem[i].sItemInfo.CODE == (sinOA1|sin36))return FALSE;

					// 장별 - 캔디데이즈 하트아뮬렛은 팔수 없다
					if(InvenItem[i].sItemInfo.CODE == (sinOA1|sin37))return FALSE;

					//등록된 아이템 코드와 마스크 종류는 팔수없다
					for(kk = 0 ; kk < NotSell_Item_CODECnt ; kk++){
						if(NotSell_Item_CODE[kk] == InvenItem[i].sItemInfo.CODE)return FALSE;

					}
					for(kk = 0 ; kk < NotSell_Item_MASKCnt ; kk++){
						if((InvenItem[i].sItemInfo.CODE & sinITEM_MASK2) == NotSell_Item_MASK[kk])return FALSE;

					}
					for(kk = 0 ; kk < NotSell_Item_KINDCnt ; kk++){
						if(NotSell_Item_KIND[kk] == InvenItem[i].sItemInfo.ItemKindCode)return FALSE;

					}

					memcpy(&MyShopMouseItem , &InvenItem[i] , sizeof(sITEM)); //껍질만 등록한다
					memset(&MyShopMouseItem.sItemInfo,0,sizeof(sITEMINFO));
					

					MyShopMouseItem.sItemInfo.PotionCount = InvenItem[i].sItemInfo.PotionCount; //물약일경우를 생각해 카운트를 카피한다
					MyShopMouseItem.sItemInfo.ItemHeader = InvenItem[i].sItemInfo.ItemHeader; //헤더를 카피한다
					MyShopMouseItem.sItemInfo.Index = i+1;				 //아이템 인덱스 저장
					sinMyShopItemPass = 1; //이벤트를 한번 건너뛰기위한 플랙

				}
				else{
					if(InvenItem[i].Class == ITEM_CLASS_POTION){
						if(!InvenItem[i].ItemPosition)
							SelectPotionIndex = i+1;
					}
					cItem.GetItemLimitTime(&InvenItem[i]); //아이템의 유통기한을 체크한다  
					SelectInvenItemIndex = i+1;
					ColorIndex = SELECT_ITEM_COLOR;
					sinShowItemInfoFlag = 2; //아이템의 정보를 보여준다 
					cHelpPet.PetMessage("*InvenTory_Item",0);
					if(cMyShop.OpenFlag){
						cItem.ShowItemInfo(&InvenItem[i] , 2,i);
					}
					else{
						cItem.ShowItemInfo(&InvenItem[i] , 2);
					}
				}
			}
		}
	}
	return FALSE;
}


//마우스에 아이템이 있을때 아이템이 셋팅될 영역을 체크한다 
int cINVENTORY::SetInvenItemAreaCheck(sITEM *pItem)
{

	int i,j;
	ColorIndex= 0; //초기화 
	CrashItemIndex[0] = 0; //충돌된 아이템 Index 초기화 
	CrashItemIndex[1] = 0;
	AutoSetItemIndex  = 0; //자동으로 셋팅될 아이템의 인덱스 

	///////////무기 영역 &포션 영역 
	if(!cInvenTory.OpenFlag)return FALSE; //인벤토리가 열려있지 않을시에는 아이템을 셋팅할수 없다
	for( i=0 ; i < INVENTORY_MAX_POS ; i++){
		if(InvenSlotHidden(i))continue;		// the costume slot in the gear view, the gear slots in the costume view
		if(CheckInvenPos(pItem->x+11 , pItem->y+11 ,pItem->x +pItem->w -11 ,pItem->y +pItem->h -11 , sInven[i].Rect)){
			ColorRect.left   = sInven[i].Rect.left;     //컬럭 박스의 RECT
			ColorRect.top    = sInven[i].Rect.top;
			ColorRect.right  = sInven[i].Rect.right  - sInven[i].Rect.left;
			ColorRect.bottom = sInven[i].Rect.bottom - sInven[i].Rect.top;
			pItem->SetX = sInven[i].Rect.left + ((ColorRect.right-pItem->w)/2);
			pItem->SetY = ColorRect.top  + ((ColorRect.bottom - pItem->h)/2);
			pItem->ItemPosition = i+1;

			// the premium ring slots take the premium (OR2xx) rings only
			int bPremiumRing = ( i>=INVENTORY_POS_SLOT_PRING && i<INVENTORY_POS_SLOT_PRING+2 &&
								 (pItem->Class & ITEM_CLASS_RING) && (pItem->CODE & sinITEM_MASK2)==sinOR2 );
			if(!(sInven[i].Position & pItem->Class) && !bPremiumRing){ //클래스가 맞지 않는다
				ColorIndex = NOT_SETTING_COLOR;
				return FALSE;
			}
			// a costume for the other gender, or armour for a robe class (and the other way round)
			if(i==INVENTORY_POS_SLOT_COSTUME && !sinCostumeFits(pItem->CODE , sinChar->JOB_CODE)){
				ColorIndex = NOT_SETTING_COLOR;
				return FALSE;
			}
			if(pItem->sItemInfo.NotUseFlag){ //클래스가 맞지 않거나 셋팅이 될수없으면  
				ColorIndex = NOT_USE_COLOR;
				return FALSE;

			}
			/*
			if(CrashItemIndex[0] = CrashInvenItem(ColorRect)){  //겹치는 아이템이 있나 체크 
				if(InvenItem[CrashItemIndex[0]-1].sItemInfo.ItemKindCode == ITEM_KIND_QUEST_WEAPON &&
					InvenItem[CrashItemIndex[0]-1].sItemInfo.ItemAgingNum[0] < 4){
					ColorIndex = NOT_SETTING_COLOR;
					return FALSE;
				}
			}
			*/

			if(CrashItemIndex[0] = CrashInvenItem(ColorRect)){  //겹치는 아이템이 있나 체크 
				OverlapTwoHandItem(pItem); //투핸드 무기로 겹치게 될 아이템을 체크한다
				ColorIndex = OVERLAP_BOX_COLOR;
				return TRUE;
			}
			OverlapTwoHandSwitch(pItem); //투핸드 아이템의 반대쪽 겹치기 체크 
			ColorIndex = SET_ITEM_CHECK_COLOR; //셋팅할영역 
			return TRUE;
		}

	}
	
	///////////박스 영역 
	for(i=pItem->x+11 ; i<pItem->x+pItem->w ; i+=22){
		for( j=pItem->y+11 ; j<pItem->y+pItem->h ; j+=22){
			if(StartX <= i && EndX > i && StartY <=j && EndY > j){ //인벤토리 박스에 들어갔는지를 체크 
				ColorRect.left =  StartX +( ( ( i - StartX) /22)*22) ; 
				ColorRect.top  =  StartY +( ( ( j - StartY) /22)*22) ; 
				ColorRect.right  = pItem->w;
				ColorRect.bottom = pItem->h;
				if(EndX < (ColorRect.left + ColorRect.right)-11 || EndY < (ColorRect.top + ColorRect.bottom)-11){
					InitColorRect(); //컬러 박스 RECT 초기화 (컬러 박스 RECT중 하나라두 0 이면 셋팅할수 없다 
					return FALSE;
				}
				pItem->SetX = ColorRect.left;
				pItem->SetY = ColorRect.top;
				pItem->ItemPosition = 0; //배열의 인덱스를 저장한다 
				ColorIndex = SET_ITEM_CHECK_COLOR; //셋팅할영역


				/*
				if(CrashItemIndex[0] = CrashInvenItem(ColorRect)){  //겹치는 아이템이 있나 체크 
					if(InvenItem[CrashItemIndex[0]-1].sItemInfo.ItemKindCode == ITEM_KIND_QUEST_WEAPON &&
						InvenItem[CrashItemIndex[0]-1].sItemInfo.ItemAgingNum[0] < 4){
						ColorIndex = NOT_SETTING_COLOR;
						return FALSE;
					}
					
				}
				*/
				if(CrashItemIndex[0] = CrashInvenItem(ColorRect)){  //겹치는 아이템이 있나 체크
					/*
					if(!CheckSpecialItem(&InvenItem[CrashItemIndex[0]-1])){ //집을 수없는 특별한 아이템을 체크
						ColorIndex = NOT_SETTING_COLOR;
						return FALSE;
					}
					*/

					if(CrashItemIndex[1] = CrashInvenItem(ColorRect,CrashItemIndex[0])){ //두개 이상 겹치나 체크 
						ColorIndex = NOT_SETTING_COLOR;
						CrashItemIndex[1] = 0; //초기화 
						return FALSE;

					}
					ColorIndex = OVERLAP_ITEM_COLOR;
					return TRUE;
				}
				return TRUE;
			}
				
		}
	
	}

	return TRUE;	
}

//컬러 박스 RECT 초기화 
int cINVENTORY::InitColorRect()
{
	ColorRect.left   = 0;   
	ColorRect.top    = 0;
	ColorRect.right  = 0;
	ColorRect.bottom = 0;
	return TRUE;

}

//아이템 끼리의 충돌영역을 체크한다
int cINVENTORY::CrashInvenItem(RECT &desRect , int PassItemIndex)
{

	RECT rect;

	for(int i=0 ; i<INVENTORY_MAXITEM ; i++){
		if(InvenItem[i].Flag){
			if(InvenItemHidden(&InvenItem[i]))continue;		// the armour under the costume slot, and so on
			if(PassItemIndex != 0)
				if(PassItemIndex == i+1)continue;
			rect.left   = InvenItem[i].x;
			rect.right  = InvenItem[i].x+InvenItem[i].w;
			rect.top    = InvenItem[i].y;
			rect.bottom = InvenItem[i].y+InvenItem[i].h;

			if( desRect.left > rect.left - desRect.right  && desRect.left < rect.right &&
				desRect.top > rect.top - desRect.bottom && desRect.top< rect.bottom ){
					return i+1; //충돌 체크된 아이템 인덱스를 하나더해서 리턴한다 
			}
		}
	}

	return FALSE;

}
//아이템이 놓을 영역 충돌 체크 
int cINVENTORY::CheckInvenPos(int x , int y , int lx ,int ly , RECT &rect)
{
	if(x <= rect.left){
		if(rect.left >= lx)return FALSE;
	}
	else{ 
		if(x >=rect.right)return FALSE;
	}

	if(y <= rect.top ){
		if(rect.top >= ly)return FALSE;
	}
	else {
		if(y >= rect.bottom)return FALSE;
	}

	return TRUE;

}

 

int cINVENTORY::ChangeInvenItem(sITEM *pItem)
{
	
	int BackUpX=0,BackUpY=0,BackUpPosi=0;
	int j; for( j =0 ; j<SIN_MAX_HELP_NUMBER ; j++)				//텔레포트 핼프창이 열렸을때 아템 체인지 막는다 (성근 추가 )
	{
		if(sSinHelp[j].KindFlag == SIN_HELP_KIND_TELEPORT_SCROLL){
			return TRUE;
			if(NoSettingThrowItem(&MouseItem)){
				return FALSE;
			}
		}
		
	}
	
	//퀘스트 아이템은 겹쳐진아이템으로 뺄수엄따
	/*
	if(CrashItemIndex[0]){
		if(InvenItem[CrashItemIndex[0]-1].sItemInfo.ItemKindCode == ITEM_KIND_QUEST_WEAPON &&
			InvenItem[CrashItemIndex[0]-1].sItemInfo.ItemAgingNum[0] < 4){
			cMessageBox.ShowMessage(MESSAGE_NOT_PICKUP_ITEM);
			return FALSE;
		}
	}
	*/

	// pluto 무한 스킬 버그 막는다 급조한 거임 -_- //해외
	if(lpCurPlayer->MotionInfo->State == CHRMOTION_STATE_ATTACK || lpCurPlayer->MotionInfo->State == CHRMOTION_STATE_SKILL)
	{
		//if( pCursorPos.x > 575 && pCursorPos.x < 785 && pCursorPos.y > 435 && pCursorPos.y < 540 ) // 무기 장비창 좌표임 땜~빵~
		//if( pCursorPos.x > 515 && pCursorPos.x < 785 && pCursorPos.y > 345 && pCursorPos.y < 600 ) // EPT유저땜에 다시 무기 장비창 좌표임 땜~빵~
		//다시 오른쪽 아래에서 아이템을 깔아끼어서 , CL버그를 사용하더라. 그래서 아애 스킬 사용중에는 아이템 못갈아끼게함
		{
			return FALSE;
		}
	}
	if(!InvenNotOpenCheck(pItem))return FALSE; //인벤 로직수행여부를 체크한다 
	if(CrashItemIndex[0]){//충돌 된 아이템이 있으면 아이템을 바꿔준다 
		CheckInvenItemForm();
		if(InvenItem[CrashItemIndex[0]-1].Class == ITEM_CLASS_POTION){
			if(pMessageItem){
				BackUpX = pMessageItem->x;
				BackUpY = pMessageItem->y;
				BackUpPosi = pMessageItem->ItemPosition;
				sinPosionItem  = &InvenItem[CrashItemIndex[0]-1];
				BackUpItemIndex = CrashItemIndex[0];
			}
		}
		if(InvenItem[CrashItemIndex[0]-1].Class == ITEM_CLASS_WEAPON_TWO) //양손무기일경우에는 인자구조체에 오른손 아이템을 복사하고 나머지는 지워준다 
			PickUpTwoHandItem(&InvenItem[CrashItemIndex[0]-1]); //양손 무기

		memcpy(&TempItem,&InvenItem[CrashItemIndex[0]-1],sizeof(sITEM)); //마우스 아이템을 템프로 복사 
		if(InvenItem[CrashItemIndex[0]-1].ItemPosition) //장착되 있는 무기일경우에는 무기를 빼준다
			sinSetCharItem(InvenItem[CrashItemIndex[0]-1].CODE , InvenItem[CrashItemIndex[0]-1].SetModelPosi, FALSE); //무기와 갑옷을 셋팅한다 			
		InvenItem[CrashItemIndex[0]-1].Flag = 0 ; //복사한후에 초기화
		TempItem.x = pItem->x;
		TempItem.y = pItem->y;
		if(AutoSetItemIndex){ //오토 셋팅될 아이템이 있으면 
			ReFormInvenItem(); //오토 셋팅된다음에도 체크섬을 갱신한다 
			sinSetCharItem(InvenItem[AutoSetItemIndex-1].CODE , InvenItem[AutoSetItemIndex-1].SetModelPosi , FALSE); //방패를 빼준다 
			if(!InvenEmptyAearCheck(&InvenItem[AutoSetItemIndex-1])){//한쪽의 아이템을 오토셋팅한다 
				if(AutoSetItemIndex)
					NoSettingThrowItem(&InvenItem[AutoSetItemIndex-1],1);//셋팅이 안될때는 버린다 

			}
			
		}
		ReFormInvenItem();
		LastSetInvenItem(pItem); //마우스 아이템을 인벤토리로 복사 
		memcpy(pItem,&TempItem,sizeof(sITEM));
		if(sinPosionItem){ //포션일경우에만 포인터에 값을 셋팅 
			if(InvenItem[CrashItemIndex[0]-1].Class == ITEM_CLASS_POTION){
				pMessageItem = sinPosionItem;
				pMessageItem->x = BackUpX;
				pMessageItem->y = BackUpY;
				pMessageItem->ItemPosition = BackUpPosi;
				sinPosionItem = 0;
			}

		}
		ReFormInvenItem();
		return TRUE;
		
	}

	return FALSE;
}

//셋팅이 안될때에는 바닥에 아이템을 버린다 
int cINVENTORY::NoSettingThrowItem(sITEM *pItem , int Flag)
{
	//패치를 위해 잠시 막아둔다 
	if(!pItem)return FALSE;  
	if(!pItem->Flag)return FALSE;
	if(!Flag){ //겹쳐지는 아이템은 바닦에 버린다 (겹쳐지지않은 상태에서는 상점 및 트레이드 창등이 떠있을경우에는 버릴수없다) // pluto 제련
		if(cTrade.OpenFlag || cWareHouse.OpenFlag || cCraftItem.OpenFlag || cAging.OpenFlag || SmeltingItem.OpenFlag || ManufactureItem.m_OpenFlag || cMixtureReset.OpenFlag)return FALSE; //트레이드나 창고가 떠있으면 아이템을 버리지 않는다 // 석지용 - 믹스쳐 리셋 창 추가
	}
	if(!pItem->sItemInfo.ItemHeader.Head || pItem->sItemInfo.ItemHeader.Head < 0)return FALSE;
	if(pItem->sItemInfo.CODE == ( sinGF1|sin01))return FALSE;
	//if(pItem->sItemInfo.CODE == ( sinGF1|sin02))return FALSE;

	int kk =0;
	//등록된 아이템은 버릴수없다아이템을 버릴수없다 
	for(kk = 0 ; kk < NotDrow_Item_CODECnt ; kk++){
		if(NotDrow_Item_CODE[kk] == pItem->sItemInfo.CODE)return FALSE;

	}
	for(kk = 0 ; kk < NotDrow_Item_MASKCnt ; kk++){
		if((pItem->sItemInfo.CODE & sinITEM_MASK2) == NotDrow_Item_MASK[kk])return FALSE;

	}
	for(kk = 0 ; kk < NotDrow_Item_KINDCnt ; kk++){
		if(NotDrow_Item_KIND[kk] == pItem->sItemInfo.ItemKindCode)return FALSE;

	}

	if(sinThrowItemToFeild(pItem)){
		sinPlaySound(pItem->SoundIndex);
		pItem->Flag = 0;
	}
	else{
		pItem->Flag = 0; //버리지 못해두 셋팅이 안되므로 지워버린다 
		
	}

	return TRUE;
}

//아이템을 버린다 
int cINVENTORY::ThrowInvenItemToField(sITEM *pItem)
{
	///////////이벤트 아이템은 버릴수 없다  팔수도없다 
	//if((pItem->sItemInfo.CODE & sinITEM_MASK2) == sinGP1)return TRUE;
	//if((pItem->sItemInfo.CODE & sinITEM_MASK2) == sinBI2)return TRUE;							//패키지캐쉬아이템은 버릴수 없다.
	if(cTrade.OpenFlag || cWareHouse.OpenFlag || cCraftItem.OpenFlag || cAging.OpenFlag || SmeltingItem.OpenFlag || ManufactureItem.m_OpenFlag || cMixtureReset.OpenFlag)		//트레이드나 창고가 떠있으면 아이템을 버리지 않는다  // 석지용 - 믹스쳐 리셋
		return TRUE;

	if(sMessageBox3[MESSAGE_TELEPORT_CORE].Flag)return TRUE;		// 텔레포트 핼프창이 떠있을때 아이템을 버릴수 없다 (성근추가)
	for(int i=0 ; i<SIN_MAX_HELP_NUMBER ; i++)
	{
		if(sSinHelp[i].KindFlag == SIN_HELP_KIND_TELEPORT_SCROLL)	
		{ return TRUE; }

	}

	int kk =0;
	//등록된 아이템은 버릴수없다아이템을 버릴수없다 
	for(kk = 0 ; kk < NotDrow_Item_CODECnt ; kk++){
		if(NotDrow_Item_CODE[kk] == pItem->sItemInfo.CODE)return TRUE;

	}
	for(kk = 0 ; kk < NotDrow_Item_MASKCnt ; kk++){
		if((pItem->sItemInfo.CODE & sinITEM_MASK2) == NotDrow_Item_MASK[kk])return TRUE;

	}
	for(kk = 0 ; kk < NotDrow_Item_KINDCnt ; kk++){
		if(NotDrow_Item_KIND[kk] == pItem->sItemInfo.ItemKindCode)return TRUE;

	}


	if(cCraftItem.OpenFlag){
		if ( 0   < pCursorPos.x  && 256+128  > pCursorPos.x && 
			 130 < pCursorPos.y  && 130+(22*10) > pCursorPos.y ){
			return TRUE;

		}

	}
	if(pItem->sItemInfo.CODE == (sinGF1|sin01))return TRUE; //상품권은 버릴수엄따
//	if(pItem->sItemInfo.CODE == (sinGF1|sin02))return TRUE; //상품권은 버릴수엄따

	
	if(cShop.OpenFlag){
//		if ( 18+(22*10)  < pCursorPos.x  && 18+(22*10)+100  > pCursorPos.x && 
//			 134 < pCursorPos.y  && 134+(22*10) > pCursorPos.y ){
//			return TRUE;

//		}
		int kk  = 0;
		if ( 18  < pCursorPos.x  && 18+(22*10)  > pCursorPos.x && 
			 134+sinInterHeight2 < pCursorPos.y  && 134+(22*10)+sinInterHeight2 > pCursorPos.y ){
				//등록된 아이템 코드와 마스크 종류는 팔수없다
				for(kk = 0 ; kk < NotSell_Item_CODECnt ; kk++){
					if(NotSell_Item_CODE[kk] == pItem->sItemInfo.CODE)return TRUE;

				}
				for(kk = 0 ; kk < NotSell_Item_MASKCnt ; kk++){
					if((pItem->sItemInfo.CODE & sinITEM_MASK2) == NotSell_Item_MASK[kk])return TRUE;

				}
				for(kk = 0 ; kk < NotSell_Item_KINDCnt ; kk++){
					if(NotSell_Item_KIND[kk] == pItem->sItemInfo.ItemKindCode)return TRUE;

				}

			   if(pItem->Class == ITEM_CLASS_POTION ){
					return TRUE; //포션과 상품권은 팔지 않는다 
				}

				/*
				if(pItem->sItemInfo.PotionCount == 1){ //포션이 한개일 경우에는 그냥 판다 
					cShop.SellItemToShop(pItem);
					return TRUE;

				}
				if(pMessageItem){ //인벤토리에서 집은 아이템일경우에만 
					cMessageBox.ShowMessage2(MESSAGE_SELL_ITEM);
					pItem->Flag=0; //마우스 아이템 초기화
					pMessageItem->Flag = 1; //아이템을 집을때 저장해뒀던 포인터로 플랙을 살린다
					sInven[pMessageItem->ItemPosition-1].ItemIndex = BackUpItemIndex;
					CheckOverlapItem(pMessageItem,BackUpItemIndex);
					ReSettingPotion();      //포션공간 체크 
					CheckWeight();          //무게 보정 
					ReFormPotionNum();		//포션갯수 보정 
					if(pMessageItem->ItemPosition) //아이템 포지션이 있었을경우에는 위치를 살려준다 
						sInven[pMessageItem->ItemPosition-1].ItemIndex = PotionIndex;
					return TRUE;

				}
				*/

				else{
					if(cShop.CheckHighRankItem(pItem)){ 
						CursorClass = 1; //커서를 변경해준다
						pItem->Flag = 0; //커서때문에 마우스 아이템을 잠시 초기화한
						ResetInvenItemCode(); //마우스 플렉을 없앨때 다시 체크해준다
						cMessageBox.ShowMessage3(MESSAGE_SELL_HIGHRANK_ITEM,pItem->sItemInfo.ItemName);

					}
					else cShop.SellItemToShop(pItem);
				}

		}
	}
	else{ //상점이 아니고서는 복사된 아이템과 증정용아이템을 버릴수없다 
		if(pItem->sItemInfo.SpecialItemFlag[0] == CHECK_COPY_ITEM || 
			pItem->sItemInfo.SpecialItemFlag[0] == CHECK_GIVE_ITEM||
			pItem->sItemInfo.ItemKindCode == ITEM_KIND_QUEST  ||
			(pItem->sItemInfo.CODE & sinITEM_MASK2) == sinMA1 ||
			(pItem->sItemInfo.CODE & sinITEM_MASK2) == sinMA2 ||
			(pItem->sItemInfo.CODE & sinITEM_MASK2) == sinQW1 ||
			(pItem->sItemInfo.CODE & sinITEM_MASK2) == sinBI1 || 
			(pItem->sItemInfo.CODE & sinITEM_MASK2) == sinBI2 ||	//성근추가(패키지캐쉬아이템에 대한 드랍 방지)
			//파티복 코스튬 추가
			(pItem->sItemInfo.CODE == (sinDA1|sin31)) || // 박재원 : 송편아뮬렛sin31과 파티복 sin31 아이템이 겹쳐서 구분함
			(pItem->sItemInfo.CODE == (sinDA2|sin31)) || // 박재원 : 송편아뮬렛sin31과 파티복 sin31 아이템이 겹쳐서 구분함
			(pItem->sItemInfo.CODE == (sinDA1|sin32)) || // 박재원 : 산타아뮬렛sin32과 파티복 sin32 아이템이 겹쳐서 구분함 
			(pItem->sItemInfo.CODE == (sinDA2|sin32)) || // 박재원 : 산타아뮬렛sin32과 파티복 sin32 아이템이 겹쳐서 구분함 
			(pItem->sItemInfo.CODE == (sinDA1|sin33)) || // 박재원 : 이벤트아뮬렛 sin33과 파티복 sin33 아이템이 겹쳐서 구분함
			(pItem->sItemInfo.CODE == (sinDA2|sin33)) || // 박재원 : 이벤트아뮬렛 sin33과 파티복 sin33 아이템이 겹쳐서 구분함  
			(pItem->sItemInfo.CODE == (sinDA1|sin34))   || (pItem->sItemInfo.CODE == (sinDA2|sin34))      ||
			(pItem->sItemInfo.CODE == (sinDA1|sin35))   || (pItem->sItemInfo.CODE == (sinDA2|sin35))      ||
			(pItem->sItemInfo.CODE == (sinDA1|sin36))   || (pItem->sItemInfo.CODE == (sinDA2|sin36))      ||
			(pItem->sItemInfo.CODE == (sinDA1|sin37))   || (pItem->sItemInfo.CODE == (sinDA2|sin37))      ||
			(pItem->sItemInfo.CODE == (sinDA1|sin38))   || (pItem->sItemInfo.CODE == (sinDA2|sin38))      ||
			(pItem->sItemInfo.CODE == (sinDA1|sin39))   || (pItem->sItemInfo.CODE == (sinDA2|sin39))      ||
			(pItem->sItemInfo.CODE == (sinDA1|sin40))   || (pItem->sItemInfo.CODE == (sinDA2|sin40))      ||
			(pItem->sItemInfo.CODE == (sinDA1|sin41))   || (pItem->sItemInfo.CODE == (sinDA2|sin41))      ||
			(pItem->sItemInfo.CODE == (sinDA1|sin42))   || (pItem->sItemInfo.CODE == (sinDA2|sin42))      ||
			(pItem->sItemInfo.CODE == (sinDA1|sin43))   || (pItem->sItemInfo.CODE == (sinDA2|sin43))      ||
			(pItem->sItemInfo.CODE == (sinDA1|sin44))   || (pItem->sItemInfo.CODE == (sinDA2|sin44))      ||
			(pItem->sItemInfo.CODE == (sinDA1|sin45))   || (pItem->sItemInfo.CODE == (sinDA2|sin45))      ||
			(pItem->sItemInfo.CODE == (sinDA1|sin46))   || (pItem->sItemInfo.CODE == (sinDA2|sin46))      ||
			(pItem->sItemInfo.CODE == (sinOA2|sin31))   || (pItem->sItemInfo.CODE == (sinOA2|sin32))      || // 박재원 - 슈퍼 암릿(7일, 30일) 추가
			(pItem->sItemInfo.CODE == (sinDB1|sin31))   || (pItem->sItemInfo.CODE == (sinDB1|sin32))      || // 박재원 - 스피드 부츠(7일, 30일) 추가
//			(pItem->sItemInfo.CODE & sinITEM_MASK3) == sin48		//한복 Kyle수정
			(pItem->sItemInfo.CODE == (sinOR2|sin33))	|| // 장별 - 하트링(7일) 추가 
			(pItem->sItemInfo.CODE == (sinOA1|sin36))	|| // 장별 - 눈결정목걸이(7일) 추가	
			(pItem->sItemInfo.CODE == (sinOA1|sin37))	|| // 장별 - 캔디데이즈 하트아뮬렛(7일) 추가	
			(pItem->sItemInfo.CODE == (sinDB1|sin33))	||	// 장별 - 스피드 부츠(1일)
			(pItem->sItemInfo.CODE == (sinOA2|sin33))	||	// 장별 - 슈퍼 암릿(1일)
			(pItem->sItemInfo.CODE == (sinDB1|sin34))	||	// 장별 - 스피드 부츠(1시간)
			(pItem->sItemInfo.CODE == (sinOA2|sin34))	||	// 장별 - 슈퍼 암릿(1시간)
			(pItem->sItemInfo.CODE == (sinBI1|sin84))	||	// 장별 - 그라비티 스크롤
			(pItem->sItemInfo.CODE == (sinDA1|sin54))	|| (pItem->sItemInfo.CODE == (sinDA2|sin54)) || // 박재원 - 수영복 복장 추가(남자) - 드랍 금지
			(pItem->sItemInfo.CODE == (sinDA1|sin55))	|| (pItem->sItemInfo.CODE == (sinDA2|sin55))  ||  // 박재원 - 수영복 복장 추가(여자) - 드랍 금지
			(pItem->sItemInfo.CODE == (sinGF1|sin07))	|| (pItem->sItemInfo.CODE == (sinGF1|sin08))	// 장별 - 조사원을 찾아라
			)
		{
				return TRUE;
				
		}
		//if((pItem->sItemInfo.CODE & sinITEM_MASK2) == sinGP1)return TRUE;
	}


	if(cShop.OpenFlag) //상점 로직을 처리해준후 처리해준다 (상점에서는 아이템을 버리지 않는다 )
		return TRUE;


	if(!pItem->Flag)return FALSE;
	if(OpenFlag){ 
		//pItem->x = pCursorPos.x-(MouseItem.w/2);
		//pItem->y = pCursorPos.y -(MouseItem.h/2);
		if ( pItem->y+pItem->h < (600 ) -180 ){ //인벤토리 영역을 벗어났으면 
			//물약일 경우 
			if(pItem->Class == ITEM_CLASS_POTION){
				if(pItem->sItemInfo.PotionCount == 1){ //포션이 한개일 경우에는 그냥 버린다 
					if(sinThrowItemToFeild(pItem)){
						sinPlaySound(pItem->SoundIndex);
						pItem->Flag = 0;
						ReFormPotionNum();		//포션갯수 보정 
						return TRUE;
					}

				}
				if(pMessageItem){ //인벤토리에서 집은 아이템일경우에만 
					if(GhostPotionCheckFlag){
						if(sinThrowItemToFeild(pItem)){
							sinPlaySound(pItem->SoundIndex);
							pItem->Flag = 0;
							GhostPotionCheckFlag = 0;
							ReFormPotionNum();		//포션갯수 보정 
							return TRUE;

						}

					}
					cMessageBox.ShowMessage2(MESSAGE_THROW_ITEM);
					pItem->Flag=0; //마우스 아이템 초기화
					pMessageItem->Flag = 1; //아이템을 집을때 저장해뒀던 포인터로 플랙을 살린다
					sInven[pMessageItem->ItemPosition-1].ItemIndex = BackUpItemIndex;
					CheckOverlapItem(pMessageItem,BackUpItemIndex);
					ReSettingPotion();      //포션공간 체크 
					CheckWeight();          //무게 보정 
					ReFormPotionNum();		//포션갯수 보정 
					if(pMessageItem->ItemPosition) //아이템 포지션이 있었을경우에는 위치를 살려준다 
						sInven[pMessageItem->ItemPosition-1].ItemIndex = PotionIndex;
					return TRUE;
				}
			}
				if(sinThrowItemToFeild(pItem)){
					sinPlaySound(pItem->SoundIndex);
					pItem->Flag = 0;
				}

		}
	}
	else{
		if ( pItem->y+pItem->h < 540   ){       //인벤토리가 닫혀있을경우 인터페이스 창이 아닐경우 버린다 
			//물약일 경우 
			if(pItem->Class == ITEM_CLASS_POTION){
				if(pItem->sItemInfo.PotionCount == 1){ //포션이 한개일 경우에는 그냥 버린다 
					if(sinThrowItemToFeild(pItem)){
						sinPlaySound(pItem->SoundIndex);
						pItem->Flag = 0;
						return TRUE;
					}
				}

				if(pMessageItem){
					if(GhostPotionCheckFlag){
						if(sinThrowItemToFeild(pItem)){
							sinPlaySound(pItem->SoundIndex);
							pItem->Flag = 0;
							GhostPotionCheckFlag = 0;
							ReFormPotionNum();		//포션갯수 보정 
							return TRUE;
						}
					}
					cMessageBox.ShowMessage2(MESSAGE_THROW_ITEM);
					pItem->Flag=0; //마우스 아이템 초기화
					pMessageItem->Flag = 1; //아이템을 집을때 저장해뒀던 포인터로 플랙을 살린다 
					sInven[pMessageItem->ItemPosition-1].ItemIndex = BackUpItemIndex;
					CheckOverlapItem(pMessageItem,BackUpItemIndex);
					ReSettingPotion();      //포션공간 체크 
					CheckWeight();          //무게 보정 
					ReFormPotionNum();		//포션갯수 보정 
					if(pMessageItem->ItemPosition)
						sInven[pMessageItem->ItemPosition-1].ItemIndex = PotionIndex;
					return TRUE;
				}
			}
			if(sinThrowItemToFeild(pItem)){
				sinPlaySound(pItem->SoundIndex);
				pItem->Flag = 0;
			}

		}
	}
	return TRUE;
}

//자신이 찰수있는 아이템을 착용한다 
int cINVENTORY::CharOnlySetItem(sITEM *pItem)
{
	//파티복 코스튬 착용제
	//여자
	if(sinChar->JOB_CODE == JOBCODE_PRIESTESS || sinChar->JOB_CODE == JOBCODE_ATALANTA || sinChar->JOB_CODE == JOBCODE_ARCHER ||
		sinChar->JOB_CODE == JOBCODE_ASSASSIN || sinChar->JOB_CODE == JOBCODE_MARTIALARTIST){
		if((pItem->sItemInfo.CODE & sinITEM_MASK2) == sinDA1 || (pItem->sItemInfo.CODE & sinITEM_MASK2) == sinDA2 ){
			if( (pItem->sItemInfo.CODE & sinITEM_MASK3) == sin31 || (pItem->sItemInfo.CODE & sinITEM_MASK3) == sin32 ||
				(pItem->sItemInfo.CODE & sinITEM_MASK3) == sin35 || (pItem->sItemInfo.CODE & sinITEM_MASK3) == sin36 ||
				(pItem->sItemInfo.CODE & sinITEM_MASK3) == sin39 || (pItem->sItemInfo.CODE & sinITEM_MASK3) == sin40 ||
				(pItem->sItemInfo.CODE & sinITEM_MASK3) == sin43 || (pItem->sItemInfo.CODE & sinITEM_MASK3) == sin44 ||
				(pItem->sItemInfo.CODE & sinITEM_MASK3) == sin51 ||
				(pItem->sItemInfo.CODE & sinITEM_MASK3) == sin54 )
			{
				pItem->sItemInfo.NotUseFlag = 1; 			
			}
		}		
	}
	else{ // 남자
		if((pItem->sItemInfo.CODE & sinITEM_MASK2) == sinDA1 || (pItem->sItemInfo.CODE & sinITEM_MASK2) == sinDA2 ){
			if( (pItem->sItemInfo.CODE & sinITEM_MASK3) == sin33 || (pItem->sItemInfo.CODE & sinITEM_MASK3) == sin34 ||
				(pItem->sItemInfo.CODE & sinITEM_MASK3) == sin37 || (pItem->sItemInfo.CODE & sinITEM_MASK3) == sin38 ||
				(pItem->sItemInfo.CODE & sinITEM_MASK3) == sin41 || (pItem->sItemInfo.CODE & sinITEM_MASK3) == sin42 ||
				(pItem->sItemInfo.CODE & sinITEM_MASK3) == sin45 || (pItem->sItemInfo.CODE & sinITEM_MASK3) == sin46 ||
				(pItem->sItemInfo.CODE & sinITEM_MASK3) == sin45 || (pItem->sItemInfo.CODE & sinITEM_MASK3) == sin46 ||
				(pItem->sItemInfo.CODE & sinITEM_MASK3) == sin52 ||
				(pItem->sItemInfo.CODE & sinITEM_MASK3) == sin55 )
			{

				pItem->sItemInfo.NotUseFlag = 1; 			
			}
		}
	}

	///////////// 법사 (and the Shaman)
	if(sinChar->JOB_CODE == 7 || sinChar->JOB_CODE == 8 || sinChar->JOB_CODE == JOBCODE_SHAMAN){
		if((pItem->sItemInfo.CODE & sinITEM_MASK2) == sinDA1){
			pItem->sItemInfo.NotUseFlag = 1; //일반 갑옷은 법사가 착용할수엄따

		}
	}
	else{ //전사
		if((pItem->sItemInfo.CODE & sinITEM_MASK2) == sinDA2 ||
			(pItem->sItemInfo.CODE & sinITEM_MASK2) == sinOM1 ){
				pItem->sItemInfo.NotUseFlag = 1; //로브 와 오브는 는 전사가 착용할수엄따

			}
	}

	if ( !CanClassUseWeapon( sinChar->JOB_CODE, pItem->sItemInfo.CODE ) )
		pItem->sItemInfo.NotUseFlag = 1;

	return TRUE;
}

//현재 상태를 체크한다 (레벨 ,힘 ,민첩 ....) 
int cINVENTORY::CheckRequireItem()
{
	int NotUseItemFlag = 0;
	int i;

	for( i=0 ; i < INVENTORY_MAXITEM ; i++){   
		if(InvenItem[i].Flag){
			if(InvenItem[i].sItemInfo.Level     > sinChar->Level)NotUseItemFlag = 1;
			if(InvenItem[i].sItemInfo.Dexterity > sinChar->Dexterity)NotUseItemFlag = 1;
			if(InvenItem[i].sItemInfo.Strength  > sinChar->Strength)NotUseItemFlag = 1;
			if(InvenItem[i].sItemInfo.Talent    > sinChar->Talent)NotUseItemFlag = 1;
			if(InvenItem[i].sItemInfo.Spirit    > sinChar->Spirit)NotUseItemFlag = 1;
			if(InvenItem[i].sItemInfo.Health    > sinChar->Health)NotUseItemFlag = 1;
			if(!sinCostumeFits(InvenItem[i].sItemInfo.CODE , sinChar->JOB_CODE))NotUseItemFlag = 1;	// another class's costume: no stats
			if(DeleteEventItem_TimeOut( &InvenItem[i].sItemInfo)==TRUE) NotUseItemFlag = 1;  //	박재원 : 기간제 아이템 만료되었을때(클랜치프링 등등)
			if(NotUseItemFlag){
				InvenItem[i].sItemInfo.NotUseFlag = 1;
				NotUseItemFlag = 0;
			}
			else
				InvenItem[i].sItemInfo.NotUseFlag = 0;
			CharOnlySetItem(&InvenItem[i]); //캐릭터별 아이템을 체크한다
			if((InvenItem[i].sItemInfo.CODE & sinITEM_MASK2) == sinFO1){
				int j; for( j = 0; j < 14 ; j++){ // 박재원 - 매직 포스 추가
					if((InvenItem[i].sItemInfo.CODE&sinITEM_MASK3)== SheltomCode2[j] || (InvenItem[i].sItemInfo.CODE&sinITEM_MASK3)== MagicSheltomCode[j]){
						break;
					}
				}
				if( (InvenItem[i].sItemInfo.CODE&sinITEM_MASK3) < sin21 ) // 박재원 - 일반 포스
				{
					if(sinChar->Level < ForceOrbUseLevel[j][0] ||
						sinChar->Level > ForceOrbUseLevel[j][1]){
							InvenItem[i].sItemInfo.NotUseFlag = 1;
							NotUseItemFlag = 0;

						}
					else{
						InvenItem[i].sItemInfo.NotUseFlag = 0;

					}
				}
				else if( (InvenItem[i].sItemInfo.CODE&sinITEM_MASK3) < sin35) // 박재원 - 매직 포스
				{
					if(sinChar->Level < MagicForceOrbUseLevel[j][0] ||
						sinChar->Level > MagicForceOrbUseLevel[j][1]){
							InvenItem[i].sItemInfo.NotUseFlag = 1;
							NotUseItemFlag = 0;

						}
					else{
						InvenItem[i].sItemInfo.NotUseFlag = 0;

					}
				}
			}

		}	

	}
	return TRUE;
}

//셋팅할수 있는지를 체크 
int cINVENTORY::CheckRequireItemToSet(sITEM *pItem)
{
	int NotUseItemFlag = 0;

	if(pItem->sItemInfo.Level     > sinChar->Level )NotUseItemFlag = 1;
	if(pItem->sItemInfo.Dexterity > sinChar->Dexterity)NotUseItemFlag = 1;
	if(pItem->sItemInfo.Strength  > sinChar->Strength)NotUseItemFlag = 1;
	if(pItem->sItemInfo.Talent    > sinChar->Talent)NotUseItemFlag = 1;
	if(pItem->sItemInfo.Spirit    > sinChar->Spirit)NotUseItemFlag = 1;
	if(pItem->sItemInfo.Health    > sinChar->Health)NotUseItemFlag = 1;
	if(NotUseItemFlag){
		pItem->sItemInfo.NotUseFlag = 1;
		NotUseItemFlag = 0;
	}
	else{
		pItem->sItemInfo.NotUseFlag = 0;


	}
	CharOnlySetItem(pItem); //캐릭터별 아이템을 체크한다
	return TRUE;

}

//인벤토리 텍스트 
int cINVENTORY::DrawInvenText()
{
	//if(!sinMoveKindInter[SIN_INVENTORY])return FALSE; //캐릭터 스테이터스가 닫혀있으면 리턴한다 

	HDC	hdc;

	char strBuff[128];


	memset(strBuff,0,sizeof(strBuff));
	lpDDSBack->GetDC( &hdc );
	SelectObject( hdc, sinFont);
    SetBkMode( hdc, TRANSPARENT );
	SetTextColor( hdc, RGB(255,255,255) );

	POINT NoImageMessagePosi = {0,0};
	int i = 0;
	
	if(OpenFlag){ 
		NumLineComa(sinChar->Money,strBuff);
		dsTextLineOut(hdc,CheckEditSize(InvenTextPosi[0][0]+26,InvenTextPosi[0][2], strBuff),InvenTextPosi[0][1]+(256-sinMoveKindInter[SIN_INVENTORY]),
				strBuff,lstrlen(strBuff));
	
		wsprintf(strBuff,"%d/%d",sinChar->Weight[0],sinChar->Weight[1]);
		dsTextLineOut(hdc,CheckEditSize(InvenTextPosi[1][0],InvenTextPosi[1][2], strBuff),InvenTextPosi[1][1]+(256-sinMoveKindInter[SIN_INVENTORY]),
				strBuff,lstrlen(strBuff));
	}

	for( i=0;i<INVENTORY_MAXITEM ; i++){    
		if(InvenItem[i].Flag){ 
			///////////////////////// 아이템 이미지가 없을 경우 표시해준다
			if(!InvenItem[i].lpItem){
				if(InvenItem[i].x > 10 ){//땜빵 -_-
					NoImageMessagePosi.x = InvenItem[i].x + (InvenItem[i].w/2)-5;
					NoImageMessagePosi.y = InvenItem[i].y + (InvenItem[i].h/2)-20;
					lstrcpy(strBuff,"NO");
					dsTextLineOut(hdc,NoImageMessagePosi.x , NoImageMessagePosi.y+(256-sinMoveKindInter[SIN_INVENTORY]),strBuff,lstrlen(strBuff));

					NoImageMessagePosi.x = InvenItem[i].x + (InvenItem[i].w/2) -15;
					NoImageMessagePosi.y = InvenItem[i].y + (InvenItem[i].h/2) -13;
					lstrcpy(strBuff,"IMAGE");
					dsTextLineOut(hdc,NoImageMessagePosi.x , NoImageMessagePosi.y+(256-sinMoveKindInter[SIN_INVENTORY])+20,strBuff,lstrlen(strBuff));
				}
			}
			if(InvenItem[i].Class == ITEM_CLASS_POTION){
				wsprintf(strBuff,"%d",InvenItem[i].sItemInfo.PotionCount);
				if(InvenItem[i].ItemPosition == 11 || InvenItem[i].ItemPosition == 12 || InvenItem[i].ItemPosition == 13){
					dsTextLineOut(hdc,InvenItem[i].x , InvenItem[i].y,strBuff,lstrlen(strBuff));
				}
				else{
					if(OpenFlag)
						dsTextLineOut(hdc,InvenItem[i].x , InvenItem[i].y+(256-sinMoveKindInter[SIN_INVENTORY]),strBuff,lstrlen(strBuff));
				}
			}
		}
	}


	int BackUpPosX , BackUpPosY = 0; 
	int Start = 0 , End = 0 ,Conut = 0;
	char szBuff2[128];


	// 장별 - 입력창
	if(nName == 1)
	{   
	//	BackUpPosX = BackStartPos.x;
	//	BackUpPosY = BackStartPos.y; 

//		BackStartPos.x = 300;
//		BackStartPos.y = 100; 

		SelectObject( hdc, sinBoldFont);   
		SetBkMode( hdc, TRANSPARENT ); 
		SetTextColor( hdc, RGB(65,177,240) );
		dsTextLineOut(hdc,BackStartPos.x+60,BackStartPos.y,"Name",lstrlen("Name")); 


		SetIME_Mode(1);
		hFocusWnd = hTextWnd;  
		cInterFace.ChatFlag = 0;
		sinChatEnter = 0;
		sinMessageBoxShowFlag = 1;

		hFocusWnd = hTextWnd;  
		GetWindowText( hFocusWnd , cInvenTory.szDoc , 20 );


		SelectObject( hdc, sinFont);
		SetBkMode( hdc, TRANSPARENT );
		SetTextColor( hdc, RGB(255,255,255) );


		int len = 0;
		int line = 1;
		len = lstrlen(cInvenTory.szDoc);

		line += (len/20);  

		int i; for( i = 0; i < len ; i++){     
			memset(szBuff2,0,sizeof(szBuff2));
			while(1){
				if(cInvenTory.szDoc[End] == NULL)break;
				if(cInvenTory.szDoc[End] & 0x80){ //한글이면..
					End   += 2;
					Conut += 2;
				}
				else{ //영문이면..
					End++;
					Conut++;
				}
				if(Conut > 20)break;
			}
			memcpy(szBuff2 , &cInvenTory.szDoc[Start],End - Start); 

		dsTextLineOut(hdc,BackStartPos.x+32,BackStartPos.y+(i*20)-110,szBuff2,lstrlen(szBuff2));

			if(cInvenTory.szDoc[End] != NULL ){
				if(cInvenTory.szDoc[End] == ' ')
					End += 1;
				Start = End;
				Conut = 0;

			}
			else break;
		}

			
	}

	//테스트를 알수있는 메세지 나중에 지워야함
	/*
	if(sinTestFlag2){
		if(sinTestFlag2 <= 5){
			cMessageBox.ShowMessage(MESSAGE_TEST_DAMAGE_FUC);
		}
	}
	*/

	/////////////Test
	
/*
		wsprintf(strBuff,"%d  %d",BackUpX,BackUpY);
		dsTextLineOut(hdc,400,100,strBuff,lstrlen(strBuff));

		if(sinPosionItem){
			wsprintf(strBuff,"%d  %d",sinPosionItem->x , sinPosionItem->y);
			dsTextLineOut(hdc,200,100,strBuff,lstrlen(strBuff));


		}
		if(pMessageItem){
		wsprintf(strBuff,"%d  %d",pMessageItem->x , pMessageItem->y);
		dsTextLineOut(hdc,200,150,strBuff,lstrlen(strBuff));
		}
	
		wsprintf(strBuff,"%d  %d",InvenItem[sInven[10].ItemIndex-1].x , InvenItem[sInven[10].ItemIndex-1].y);
		dsTextLineOut(hdc,200,200,strBuff,lstrlen(strBuff));
		wsprintf(strBuff,"%d  %d",InvenItem[sInven[11].ItemIndex-1].x , InvenItem[sInven[11].ItemIndex-1].y);
		dsTextLineOut(hdc,200,300,strBuff,lstrlen(strBuff));
		wsprintf(strBuff,"%d  %d",InvenItem[sInven[12].ItemIndex-1].x , InvenItem[sInven[12].ItemIndex-1].y);
		dsTextLineOut(hdc,200,400,strBuff,lstrlen(strBuff));


	/*
	if(MouseItem.Flag){
		wsprintf(strBuff,"%d  %d",MouseItem.SetX , MouseItem.SetY);
		dsTextLineOut(hdc,200,200,strBuff,lstrlen(strBuff));
		wsprintf(strBuff,"%d  %d",MouseItem.OldX , MouseItem.OldY);
		dsTextLineOut(hdc,300,200,strBuff,lstrlen(strBuff));
		wsprintf(strBuff,"%d  %d",MouseItem.x , MouseItem.y);
		dsTextLineOut(hdc,400,200,strBuff,lstrlen(strBuff));

	}
	*/
	lpDDSBack->ReleaseDC( hdc );

	return TRUE;
}

//현재의 직업을 구해온다 
int cINVENTORY::SearchJobAndSetting()
{
	int cnt;

	JobName[0] = 0;

	cnt = 0;
	//난중에 꼭빼야함!
	/*
	if(smConfig.DebugMode){ 
		if(sinChar->ChangeJob > 1)
			sinChar->ChangeJob = 1;
	}
	*/
	while(1) {
		if ( JobDataBase[cnt].JobCode==0 ) break;
		if ( JobDataBase[cnt].JobCode==sinChar->JOB_CODE ) {
			sinJobList = GetJobDataCode(sinChar->JOB_CODE , sinChar->ChangeJob);
			lstrcpy( JobName , sinJobList->szName2 );
			sinChar->JobBitMask = JobDataBase[cnt].JobBitCode;
			sinChar->LifeFunction = JobDataBase[cnt].LifeFunction; //생명력 함수 
			sinChar->ManaFunction = JobDataBase[cnt].ManaFunction; //기력 함수 
			sinChar->StaminaFunction = JobDataBase[cnt].StaminaFunction; //근력함수 
			sinDamageFunction[0] = JobDataBase[cnt].DamageFunction[0];  //근접 공격 
			sinDamageFunction[1] = JobDataBase[cnt].DamageFunction[1];  //원거리 공격 
			sinDamageFunction[2] = JobDataBase[cnt].DamageFunction[2];  //마법 공격

           	break;
		}
		cnt++;
	}
	return TRUE;

}

//캐릭터 능력치 설정 
int cINVENTORY::SetItemToChar(int CheckHackFlag)
{

	//CheckWeight();//무게 보정  //해킹방지에서 문제가 생겨 이곳에서 무게를 한번더 보정해준다(Speed관련)
	memset(&sElement_Attack,0,sizeof(sELEMENT_ATTACK)); //속성 공격 데미지는 이곳에서만 적용한다 
	CheckCharForm();//인증  

	sinTempDamage2[0] = 0;   //보여지기만 하는 데미지 (계산은 서버에서 )
	sinTempDamage2[1] = 0;

	int cnt = 0; 
	int sinAttack_Rating=0;   //개인 명중률 
	int sinAttack_Damage[2] = {0,0}; //공격력 
	int sinCritical = 0;      //크리티컬힛 100분율로 나타낸다 
	int sinDefense = 0;		  //방어력 
	float sinAbsorption = 0;  //흡수력 
	int   sinTempAbsorption = 0 ; //흡수력을 보정한다
	float sinTempAbsorption2 = 0 ; //흡수력을 보정한다
	float sinBlock_Rate = 0;  //블럭률 
	int sinWeight = 0;        //무게 
	float sinMoveSpeed = 0;	  //스피드
	int sinWeaponSpeed = 0;   //무기속도 
	int sinShooting_Range = 0;  //공격거리
	int sinSight = 0;		//시야 (야간 적용)
	int sinResistance[8] = {0,0,0,0,0,0,0,0};
	int sinPotion_Space = 2; //기본 은 2개 까지 가능
	int sinSkillMasteryItem = 0;

	/////////// 스킬 능력 치 
	int sinSkillDamage[2] = {0,0};
	int sinSkillResistance[8] = {0,0,0,0,0,0,0,0};
	int sinSkillWeaponSpeed = 0;

	/////////// 캐릭터 적용 아이템 능력치 
	int sinCharItemDamage = 0;

	////////////특화 아이템 인자 //////////////////////
	float	sinAdd_fAbsorb			= 0;			//흡수력 
	int     sinAdd_Defence			= 0;			//방어력 
	float   sinAdd_fSpeed			= 0;			//이동 속도 
	float   sinAdd_fBlock_Rating	= 0;			//(방패)블럭율 
	int     sinAdd_Attack_Speed		= 0;			//공격속도
	int     sinAdd_Critical_Hit		= 0;			//1.5배 데미지확율
	int     sinAdd_Shooting_Range	= 0;			//사정거리 
	short	sinAdd_Resistance[8]	={0,0,0,0,0,0,0,0};		//원소에대한 저항력 

	////////////////////////////////////////////////

	short	sinLev_Attack_Resistance[8] = {0,0,0,0,0,0,0,0};	//원소에대한 공격력
	int		sinLev_Mana					=0;	//마나회복(최소)(최대)
	int		sinLev_Life					=0;	//라이프회복(최소)(최대) 
	int     sinLev_Attack_Rating		=0; //명중력 
	short	sinLev_Damage[2]			={0,0};//공격력

	float	sinPer_Mana_Regen			= 0;//마나 재생  (계산을위해 float형으로 )
	float	sinPer_Life_Regen			= 0;//라이프 재생 
	float 	sinPer_Stamina_Regen		= 0;//스테미나 재생

	float fstrength, fhealth,  ftalent , fLevel , fMaxWeight , 	fNowWeight ,fSpirit ,fDexterity;
	float fAttack_Rating , fDefense;

	sinAdd_fMagic_Mastery = 0;

	///////////////////////////////////////////////// 

	float sinfIncreLife		= 0; //최대치 증가 
	float sinfIncreMana		= 0;
	float sinfIncreStamina  = 0;

	float sinfRegenLife		= 0; //Regen 관련 
	float sinfRegenMana		= 0;
	float sinfRegenStamina  = 0;

	/////////////////////////////////////////////////
	//이쁘게 다시맹근 스킬
	sinUndeadAbsorb = 0; //언데드 흡수율 초기화 

	//////////////////////////////////////////////////
	int InvenCheckItem[INVENTORY_MAX_POS+4] = {0,};		// by ItemPosition: the costume and premium rings too
	sITEM sinTempItem;
	memset(&sinTempItem,0,sizeof(sITEM));

	int CountAbsorbItem = 0;

	int i; for( i =0; i < INVENTORY_MAXITEM ; i++){
		if(InvenItem[i].Flag ){//셋팅되있는 아이템이면 
			if(InvenItem[i].ItemPosition){
				if(InvenItem[i].sItemInfo.CODE > sinPM1)continue;
				if(InvenItem[i].sItemInfo.NotUseFlag)continue; //셋팅될수 없는 아이템이면 파라메타 셋팅을 하지않는다

				//한번더 체크한다
				if(!CheckRequireItemToSet2(&InvenItem[i]))continue;

				if(InvenItem[i].Class == ITEM_CLASS_WEAPON_TWO)
					if(InvenItem[i].sItemInfo.ItemHeader.dwChkSum == 0 )continue; //양손무기땜시 살짝 땜빵 

				if(!InvenCheckItem[InvenItem[i].ItemPosition]){  //아이템이 겹쳐있을땐 능력치를 반영안한다 
					InvenCheckItem[InvenItem[i].ItemPosition] = i+1;
				}
				else 
					continue;

				////////////////////////치트된 아이템을 날려버린다 
				if(!CheckItemForm(&InvenItem[i].sItemInfo)){
					SendSetHackUser(1); //해킹을 하려고했던 못된 유저를 고발 TRUE 접속 종료 
					InvenItem[i].Flag = 0;  //치트된 아이템을 없애버힌다 
					if(InvenItem[i].ItemPosition){
						sInven[InvenItem[i].ItemPosition-1].ItemIndex = 0;
						sinSetCharItem(InvenItem[i].CODE , InvenItem[i].SetModelPosi, FALSE); //무기와 갑옷을 셋팅한다 

					}
					if(InvenItem[i].Class == ITEM_CLASS_WEAPON_TWO){
						InvenItem[sInven[1].ItemIndex-1].Flag = 0;
						sInven[1].ItemIndex = 0;
					}
					continue;
				}
				////////////////////////

				sinAttack_Rating +=	InvenItem[i].sItemInfo.Attack_Rating; //개인 명중률 				
				sinAttack_Damage[0] += InvenItem[i].sItemInfo.Damage[0];
				sinAttack_Damage[1] += InvenItem[i].sItemInfo.Damage[1];
				sinCritical += InvenItem[i].sItemInfo.Critical_Hit;      
				sinDefense  += InvenItem[i].sItemInfo.Defence;
				//sinTempAbsorption = (int)(InvenItem[i].sItemInfo.fAbsorb*10.000001f);
				sinTempAbsorption = (int)(GetItemAbsorb(&InvenItem[i].sItemInfo)*10.000001f);
				sinTempAbsorption2 = ((float)sinTempAbsorption/10.0f);
				sinTempAbsorption2 += 0.000001f;  //오차보정

				if(InvenItem[i].sItemInfo.fAbsorb){
					CountAbsorbItem++;

				}

				sinAbsorption += sinTempAbsorption2; //뒷부분을 자르고 계산한다
//				sinAbsorption += InvenItem[i].sItemInfo.fAbsorb;

				sinBlock_Rate += InvenItem[i].sItemInfo.fBlock_Rating;
				sinMoveSpeed      += InvenItem[i].sItemInfo.fSpeed;
				
				//퀘스트 아이템때문에 이렇게 한다 괴롭다 -_-
				if(CheckQuestItemDownFlag && InvenItem[i].sItemInfo.ItemKindCode == ITEM_KIND_QUEST_WEAPON){
					sinWeaponSpeed +=InvenItem[i].sItemInfo.Attack_Speed-2;
				}
				else
					sinWeaponSpeed +=InvenItem[i].sItemInfo.Attack_Speed;

				sinShooting_Range +=InvenItem[i].sItemInfo.Shooting_Range;
				sinSight      += InvenItem[i].sItemInfo.Sight;
				sinfRegenLife += InvenItem[i].sItemInfo.fLife_Regen;
				sinfRegenMana += InvenItem[i].sItemInfo.fMana_Regen;
				sinfRegenStamina += InvenItem[i].sItemInfo.fStamina_Regen;
				sinAdd_fMagic_Mastery = InvenItem[i].sItemInfo.fMagic_Mastery;

				sinfIncreLife += InvenItem[i].sItemInfo.fIncrease_Life;
				sinfIncreMana += InvenItem[i].sItemInfo.fIncrease_Mana;
				sinfIncreStamina += InvenItem[i].sItemInfo.fIncrease_Stamina;
				if(InvenItem[i].sItemInfo.Potion_Space) //암렛 착용시 는 암렛의 포션 보유공간으로  설정
				{
					// 박재원 - 슈퍼 암릿 아이템 추가
					 if( DeleteEventItem_TimeOut(&InvenItem[i].sItemInfo) == TRUE )
					 {
						sinPotion_Space = 2;
					 }
					 else
						sinPotion_Space  = InvenItem[i].sItemInfo.Potion_Space;
				}
				///////////////////////캐릭터 특화 능력
				if(sinChar->JobBitMask & InvenItem[i].sItemInfo.JobCodeMask){
					sinTempAbsorption = (int)(InvenItem[i].sItemInfo.JobItem.Add_fAbsorb*10.000001f);
					sinTempAbsorption2 = ((float)sinTempAbsorption/10.0f);
					sinTempAbsorption2 += 0.000001f;  //오차보정
					sinAdd_fAbsorb += sinTempAbsorption2; //뒷부분을 자르고 계산한다

					if(InvenItem[i].sItemInfo.JobItem.Add_fAbsorb){
						CountAbsorbItem++;

					}

					//sinAdd_fAbsorb    += InvenItem[i].sItemInfo.JobItem.Add_fAbsorb;   //흡수력 
					sinAdd_Defence    += InvenItem[i].sItemInfo.JobItem.Add_Defence;   //방어력 
					sinAdd_fSpeed     += InvenItem[i].sItemInfo.JobItem.Add_fSpeed ;	//이동 속도 
					sinAdd_fBlock_Rating  += InvenItem[i].sItemInfo.JobItem.Add_fBlock_Rating;	//(방패)블럭율 
					sinAdd_Attack_Speed   += InvenItem[i].sItemInfo.JobItem.Add_Attack_Speed;	//공격속도
					sinAdd_Critical_Hit   += InvenItem[i].sItemInfo.JobItem.Add_Critical_Hit;	//1.5배 데미지확율
					sinAdd_Shooting_Range += InvenItem[i].sItemInfo.JobItem.Add_Shooting_Range;	//사정거리 
					sinAdd_fMagic_Mastery += InvenItem[i].sItemInfo.JobItem.Add_fMagic_Mastery; //마법숙련도 

					if(InvenItem[i].sItemInfo.JobItem.Lev_Mana ) //마나 최대량 
						sinLev_Mana    += (sinChar->Level/InvenItem[i].sItemInfo.JobItem.Lev_Mana);				//마나회복(최소)(최대)
					if(InvenItem[i].sItemInfo.JobItem.Lev_Life)  //라이프 최대량
						sinLev_Life    += (sinChar->Level/InvenItem[i].sItemInfo.JobItem.Lev_Life);				//라이프회복(최소)(최대) 
					if(InvenItem[i].sItemInfo.JobItem.Lev_Attack_Rating) //어택레이팅 
						sinLev_Attack_Rating += (sinChar->Level /InvenItem[i].sItemInfo.JobItem.Lev_Attack_Rating);		//명중력 
					//if(InvenItem[i].sItemInfo.JobItem.Lev_Damage[0]) //데미지(최소)
					//	sinLev_Damage[0] +=  (sinChar->Level /InvenItem[i].sItemInfo.JobItem.Lev_Damage[0]);			//공격력
					if(InvenItem[i].sItemInfo.JobItem.Lev_Damage[1]) // LV/x 특화 +데미지 sinLev_Damage[0]은 사용하고있지않다
						sinLev_Damage[1] +=  (sinChar->Level /InvenItem[i].sItemInfo.JobItem.Lev_Damage[1]);			//공격력
					if(InvenItem[i].sItemInfo.JobItem.Per_Mana_Regen)//마나리젠
						sinPer_Mana_Regen += (InvenItem[i].sItemInfo.JobItem.Per_Mana_Regen/2.0f);			//마나 재생 
					if(InvenItem[i].sItemInfo.JobItem.Per_Life_Regen)//라이프 리젠
						sinPer_Life_Regen += (InvenItem[i].sItemInfo.JobItem.Per_Life_Regen/2.0f);			//라이프 재생 
					if(	InvenItem[i].sItemInfo.JobItem.Per_Stamina_Regen)//스테미나 리젠
						sinPer_Stamina_Regen += (InvenItem[i].sItemInfo.JobItem.Per_Stamina_Regen/2.0f);		//스테미나 재생
					
					for(int t = 0 ;t < 8 ; t++){ //특화 저항관련  
						sinAdd_Resistance[t]  += (int)InvenItem[i].sItemInfo.JobItem.Add_Resistance[t];	//원소에대한 저항력 
						if(InvenItem[i].sItemInfo.JobItem.Lev_Attack_Resistance[t])
							sinLev_Attack_Resistance[t] +=(int)InvenItem[i].sItemInfo.JobItem.Lev_Attack_Resistance[t] + 
								(sinChar->Level/InvenItem[i].sItemInfo.JobItem.Lev_Attack_Resistance[t]);//원소에대한 공격력
					
						}//if for
					}// 특화능력 
					for(int j=0; j < 8 ; j++) //기본 저항 
						sinResistance[j] +=(int)InvenItem[i].sItemInfo.Resistance[j];
				
			} //if ItemPosition

		} //end if InvenFlag
	}//end for
 
	float TempSkillData=0;
	float TempDamage[2]={0,0};
	int   TempAttack_Rating=0;
	int   TempDefense=0;
	int   TempDefense2=0;
	int   TempWeaponSpeed=0;
	float TempCritical=0;

	if(sInven[0].ItemIndex){  
		float TempDamage2[2] = {cInvenTory.InvenItem[sInven[0].ItemIndex-1].sItemInfo.Damage[0],cInvenTory.InvenItem[sInven[0].ItemIndex-1].sItemInfo.Damage[1]};
		TempAttack_Rating = cInvenTory.InvenItem[sInven[0].ItemIndex-1].sItemInfo.Attack_Rating;
		memcpy(TempDamage,TempDamage2,sizeof(float)*2);
		TempWeaponSpeed =  cInvenTory.InvenItem[sInven[0].ItemIndex-1].sItemInfo.Attack_Speed;
		if(sinChar->JobBitMask & cInvenTory.InvenItem[sInven[0].ItemIndex-1].sItemInfo.JobCodeMask){
			TempWeaponSpeed +=  cInvenTory.InvenItem[sInven[0].ItemIndex-1].sItemInfo.JobItem.Add_Attack_Speed;
			
		}
		TempCritical    = (float)cInvenTory.InvenItem[sInven[0].ItemIndex-1].sItemInfo.Critical_Hit;


	}

	//특화를 넣야한다
	int MetalArmor = 0; 
	if(sInven[2].ItemIndex){
		TempDefense = cInvenTory.InvenItem[sInven[2].ItemIndex-1].sItemInfo.Defence;
		if(sinChar->JobBitMask & cInvenTory.InvenItem[sInven[2].ItemIndex-1].sItemInfo.JobCodeMask){
			TempDefense +=cInvenTory.InvenItem[sInven[2].ItemIndex-1].sItemInfo.JobItem.Add_Defence;
			MetalArmor = 1; //메탈 아머 플랙

		}

	}
	
    if(sInven[1].ItemIndex){
		TempDefense2 = cInvenTory.InvenItem[sInven[1].ItemIndex-1].sItemInfo.Defence;
		if(sinChar->JobBitMask & cInvenTory.InvenItem[sInven[1].ItemIndex-1].sItemInfo.JobCodeMask){
			TempDefense2 +=cInvenTory.InvenItem[sInven[1].ItemIndex-1].sItemInfo.JobItem.Add_Defence;
			

		}

	}

	int sinSkillTimeDamage[2] = {0,0};
	sinTempBlock  = 0;
	sinTempAbsorb = 0;

	int Count2=0;
	int Point2 = 0;
	int p = 0;
	int Flag3 = 0; //스킬에 사용한다 현제는 컨센트레이션만
	int Flag4 = 0; //스킬에 사용한다 현제는 스위프트엑스 

	//PC방용 명중력때문에 이곳에서 초기화해준다
	SkillTextColor[SIN2_ATTACK_RATE] = SIN_TEXT_COLOR_WHITE;
	SkillTextColor[SIN2_LIFE] = SIN_TEXT_COLOR_WHITE;
	SkillTextColor[SIN2_MANA] = SIN_TEXT_COLOR_WHITE; // 박재원 - 부스터 아이템(기력)
	SkillTextColor[SIN2_STAMINA] = SIN_TEXT_COLOR_WHITE; // 박재원 - 부스터 아이템(근력)

	//////////////////////// 스킬 플러스 수치 
	for( i=0 ; i < 10 ; i++){
		if(ContinueSkill[i].Flag){
			{	// the new classes' buffs (NcSkills): stats from the item pass
				sNCSKILL *nc = NC_FindCode( ContinueSkill[i].CODE );
				if ( nc && nc->Kind==NCK_BUFF ) {
					int st[3] = { nc->Stat, nc->Stat2, nc->Stat3 };
					int *va[3] = { nc->Val, nc->Val2, nc->Val3 };
					for( int s=0; s<3; s++ ) {
						int v = NC_Rank( va[s] , ContinueSkill[i].Point );
						switch( st[s] ) {
						case NCS_ACC_FLAT:	sinAttack_Rating += v;	break;
						case NCS_AGI_PCT:	sinAttack_Rating += (int)((v*TempAttack_Rating)/100);	break;
						case NCS_ABSORB:	sinTempAbsorb += v;	SkillTextColor[SIN2_ABSORB] = SIN_TEXT_COLOR_BLUE;	break;
						case NCS_ASPD:		sinSkillWeaponSpeed += (int)((v*TempWeaponSpeed)/100);	break;
						case NCS_STR_PCT:
							sinTempDamage2[0] += (int)((v*TempDamage[0])/100);
							sinTempDamage2[1] += (int)((v*TempDamage[1])/100);
							SkillTextColor[SIN2_DAMAGE] = SIN_TEXT_COLOR_GOLD;
							break;
						}
					}
				}
			}
			switch(ContinueSkill[i].CODE){
			case SKILL_EXTREME_SHIELD:
				sinTempBlock += ContinueSkill[i].PlusState[0]; 
				//sinBlock_Rate += ContinueSkill[i].PlusState[0];
			break;
			case SKILL_PHYSICAL_ABSORB:
				//sinAbsorption += ContinueSkill[i].PlusState[0];
				sinTempAbsorb += ContinueSkill[i].PlusState[0];
			break;
			case SKILL_AUTOMATION:	// (현재 형식의 변환이있었다 잘보면이해됨)
				if(sinWS1 == (cInvenTory.InvenItem[sInven[0].ItemIndex-1].CODE&sinITEM_MASK2) ||
					sinWT1 == (cInvenTory.InvenItem[sInven[0].ItemIndex-1].CODE&sinITEM_MASK2)){
					//데미지는 서버에서 처리한다
					sinTempDamage2[0] += (int)(((float)Automation_Damage[ContinueSkill[i].Point-1]/100)*TempDamage[0]); //디스플레이로 보여주기만한다
					sinTempDamage2[1] += (int)(((float)Automation_Damage[ContinueSkill[i].Point-1]/100)*TempDamage[1]);
					sinSkillWeaponSpeed += ContinueSkill[i].PlusState[0];
					SendProcessSKillToServer( SKILL_PLAY_AUTOMATION ,ContinueSkill[i].Point , 0, 0 );
				}
				else
					SendCancelSkillToServer( SKILL_PLAY_AUTOMATION  ,ContinueSkill[i].Point  , 0 , 0 ); //스킬이 취소될때 서버에 알려준다 

			break;
			case SKILL_MAXIMIZE: //최대 데미지를 상승시킨다 
				SendProcessSKillToServer( SKILL_PLAY_MAXIMIZE ,ContinueSkill[i].Point , 0, 0 );
				sinTempDamage2[1] += (int)(((float)ContinueSkill[i].PlusState[0]/100.0f)*TempDamage[1]);

			break;
			case SKILL_WINDY: //명중력 상승  
				if(sinWT1 == (cInvenTory.InvenItem[sInven[0].ItemIndex-1].CODE&sinITEM_MASK2)){
					sinAttack_Rating  += (Windy_Attack_Rating[ContinueSkill[i].Point-1]*TempAttack_Rating)/100;
					sinShooting_Range += 30; //사정거리 30플러스 
					SkillTextColor[SIN2_ATTACK_RATE] = SIN_TEXT_COLOR_ORANGE;

				}
				else{
					ContinueSkill[i].CheckTime = ContinueSkill[i].UseTime*70; //투창이 아닐경우 취소시킨다 
					SkillTextColor[SIN2_ATTACK_RATE] = SIN_TEXT_COLOR_WHITE;
				}
			break;
			case SKILL_METAL_ARMOR: 
				if(MetalArmor){ //메카니션 특화아머가 있을경우에만 적용된다 
					//디펜스를 올리고 
					sinDefense += (Metal_Armor_Defense[ContinueSkill[i].Point-1]*(TempDefense))/100;
					SkillTextColor[SIN2_DEFENSE] = SIN_TEXT_COLOR_BLUE; 
					//흡수율을 올린다
					Point2 = cSkill.GetSkillPoint(SKILL_PHYSICAL_ABSORB);
					if(Point2){
						sinTempAbsorb += (int)(((P_Absorb[Point2-1][0]+P_Absorb[Point2-1][1])*1.2f)/2);
					}
				}				
			break;
			case SKILL_SPARK_SHIELD:
				if(sInven[1].ItemIndex){
					if(sinDS1 == (cInvenTory.InvenItem[sInven[1].ItemIndex-1].CODE&sinITEM_MASK2)){
						sinDefense += Spark_Shield_Defense[ContinueSkill[i].Point-1];
					}
				}
			break;
			case SKILL_SWIFT_AXE:
				if(sInven[0].ItemIndex){ 
					if(ContinueSkill[i].Skill_Info.UseWeaponCode[0] == (cInvenTory.InvenItem[sInven[0].ItemIndex-1].CODE&sinITEM_MASK2)){
						sinSkillWeaponSpeed  += (int)((Swift_Axe_Speed[ContinueSkill[i].Point-1]*TempWeaponSpeed)/100);
						Flag4 =1;
					}
				}
				//스킬사용무기가 아니면 꺼준다
				//if(!Flag4){
				///	ContinueSkill[i].CheckTime = ContinueSkill[i].UseTime*70; //도끼가 아닐경우는 취소한다
				//}
			break;
			case SKILL_TRIUMPH_OF_VALHALLA:
				//파티원은 반만 적용된다
				sinTempDamage2[1]  += (T_Of_Valhalla_Damage[ContinueSkill[i].Point-1] + (SetT_Of_ValhallaLV/4))/(SetT_Of_ValhallaFlag+1);
				SkillTextColor[SIN2_DAMAGE] = SIN_TEXT_COLOR_GOLD;
			break;
			case SKILL_SPIRIT_ELEMENTAL:
				sinfRegenMana += Spirit_Elemental_RegenMana[ContinueSkill[i].Point-1];

			break;
			case SKILL_GOLDEN_FALCON:
				sinfRegenLife += Golden_Falcon_LifeRegen[ContinueSkill[i].Point-1];
			break;
			case SKILL_VIRTUAL_LIFE:
				SkillTextColor[SIN2_LIFE] = SIN_TEXT_COLOR_PINK;
			break;
			case SKILL_DIVINE_INHALATION:
				sinTempBlock += D_Inhalation_Block[ContinueSkill[i].Point-1];
			break;
			case SKILL_MAGNETIC_SPHERE:
				//sinTempDamage2[0]  += Magnetic_Sphere_AddDamage[ContinueSkill[i].Point-1][0];
				//sinTempDamage2[1]  += Magnetic_Sphere_AddDamage[ContinueSkill[i].Point-1][1];
				//sinTempDamage2[1]  += Magnetic_Sphere_AddDamage[ContinueSkill[i].Point-1];
				//SkillTextColor[SIN2_DAMAGE] = SIN_TEXT_COLOR_GOLD;
			break;
			case SKILL_BERSERKER:
				sinTempDamage2[0]  += Berserker_AddAttack[ContinueSkill[i].Point-1];
				sinTempDamage2[1]  += Berserker_AddAttack[ContinueSkill[i].Point-1];
				sinTempAbsorb += Berserker_SubAbsorb[ContinueSkill[i].Point-1];
				SkillTextColor[SIN2_ABSORB] = SIN_TEXT_COLOR_RED;
			break;
			case SKILL_BLOODY_BERSERKER:		// tier 5
				sinTempDamage2[0]  += Bloody_Berserker_AddAttack[ContinueSkill[i].Point-1];
				sinTempDamage2[1]  += Bloody_Berserker_AddAttack[ContinueSkill[i].Point-1];
				sinTempAbsorb += Bloody_Berserker_SubAbsorb[ContinueSkill[i].Point-1];
				SkillTextColor[SIN2_DAMAGE] = SIN_TEXT_COLOR_GOLD;
				SkillTextColor[SIN2_ABSORB] = SIN_TEXT_COLOR_RED;
			break;
			case SKILL_HYPER_SONIC:			// tier 5 Mechanician: any weapon
				sinSkillWeaponSpeed += (int)((Hyper_Sonic_Speed[ContinueSkill[i].Point-1]*TempWeaponSpeed)/100);
			break;
			case SKILL_POWER_ENHANCEMENT:	// tier 5 Mechanician: attack power +% (the server applies it)
				sinTempDamage2[0] += (int)((Power_Enhancement_Damage[ContinueSkill[i].Point-1]*TempDamage[0])/100);
				sinTempDamage2[1] += (int)((Power_Enhancement_Damage[ContinueSkill[i].Point-1]*TempDamage[1])/100);
				SkillTextColor[SIN2_DAMAGE] = SIN_TEXT_COLOR_GOLD;
			break;
			case SKILL_BLESSING_KRISHNA:	// tier 5 Priestess: absorption +
				sinTempAbsorb += Blessing_Krishna_Absorb[ContinueSkill[i].Point-1];
				SkillTextColor[SIN2_ABSORB] = SIN_TEXT_COLOR_BLUE;
			break;
			case SKILL_AMPLIFIED:			// tier 5 Pikeman: attack power +% (the server applies it), less absorption
				sinTempDamage2[0] += (int)((Amplified_Damage[ContinueSkill[i].Point-1]*TempDamage[0])/100);
				sinTempDamage2[1] += (int)((Amplified_Damage[ContinueSkill[i].Point-1]*TempDamage[1])/100);
				sinTempAbsorb += Amplified_SubAbsorb[ContinueSkill[i].Point-1];
				SkillTextColor[SIN2_DAMAGE] = SIN_TEXT_COLOR_GOLD;
				SkillTextColor[SIN2_ABSORB] = SIN_TEXT_COLOR_RED;
			break;
			case SKILL_ASSASSIN_EYE:
				//sinCritical += Assassin_Eye_AddCritical[ContinueSkill[i].Point-1];
			break;
			case SKILL_FORCE_OF_NATURE: //Flag 본인 1 파티 2
				sinTempDamage2[0]  += Force_Of_Nature_AddDamage[ContinueSkill[i].Point-1]/ContinueSkill[i].Flag; 
				sinTempDamage2[1]  += Force_Of_Nature_AddDamage[ContinueSkill[i].Point-1]/ContinueSkill[i].Flag; 
				sinAttack_Rating   += Force_Of_Nature_AddHit[ContinueSkill[i].Point-1]/ContinueSkill[i].Flag;
				SkillTextColor[SIN2_DAMAGE] = SIN_TEXT_COLOR_GOLD;
				SkillTextColor[SIN2_ATTACK_RATE] = SIN_TEXT_COLOR_ORANGE;
			break;
			case SKILL_GOD_BLESS:
				sinTempDamage2[0]  += God_Bless_AddDamage[ContinueSkill[i].Point-1]; 
				sinTempDamage2[1]  += God_Bless_AddDamage[ContinueSkill[i].Point-1]; 
				SkillTextColor[SIN2_DAMAGE] = SIN_TEXT_COLOR_GOLD;
			break;
			case SKILL_HALL_OF_VALHALLA: //잠시대기
				if(SetT_Of_ValhallaPOINT){
					sinTempDamage2[1] += (T_Of_Valhalla_Damage[SetT_Of_ValhallaPOINT-1] + (SetH_Of_ValhallaLV/4))/ContinueSkill[i].Flag;
					SkillTextColor[SIN2_DAMAGE] = SIN_TEXT_COLOR_GOLD;
				}
			break;
			case SKILL_COMPULSION:
				sinTempAbsorb += Compulsion_AddAbsorb[ContinueSkill[i].Point-1];
				SkillTextColor[SIN2_ABSORB] = SIN_TEXT_COLOR_BLUE; 
			break;
			case SKILL_FROST_JAVELIN: //잠시대기
				if(sinWT1 == (cInvenTory.InvenItem[sInven[0].ItemIndex-1].CODE&sinITEM_MASK2)){
					sinTempDamage2[0]  += Frost_Javelin_IceAddDamage[ContinueSkill[i].Point-1][0]; 
					sinTempDamage2[1]  += Frost_Javelin_IceAddDamage[ContinueSkill[i].Point-1][1]; 
				}
				else{
					ContinueSkill[i].CheckTime = ContinueSkill[i].UseTime*70; //투창이 아닐경우 취소시킨다 
					SkillTextColor[SIN2_ATTACK_RATE] = SIN_TEXT_COLOR_WHITE;
					SendCancelSkillToServer( SKILL_PLAY_FROST_JAVELIN ,0,0,0 );
				}
			break;
			case CLANSKILL_ABSORB:   //흡수력+20 
				sinTempAbsorb+=20;	 // 박재원 - 공성전 수성 클랜 스킬 상향 조정(10 -> 20)
				SkillTextColor[SIN2_ABSORB] = SIN_TEXT_COLOR_BLUE;
			break;
			case SCROLL_INVULNERABILITY:  //무적 스크롤 공격력 1/2
			case SCROLL_P_INVULNERABILITY:
				sinTempDamage2[0]  -= (sinChar->Attack_Damage[0])/2; 
				sinTempDamage2[1]  -= (sinChar->Attack_Damage[1])/2; 
				SkillTextColor[SIN2_DAMAGE] = SIN_TEXT_COLOR_RED;			
			break;
			
			case BOOSTER_ITEM_LIFE: // 박재원 - 부스터 아이템(생명력)
				SkillTextColor[SIN2_LIFE] = SIN_TEXT_COLOR_PINK; // 사용중일때 인터페이스 글자색을 바꿔준다

				if(AddBoosterLife)
				{
					if(!LifeFlag)
					{	
						tempLife[0] = ( (sinChar->Life[1]+AddVirtualLife[0]) * (short)BoosterItem_DataPercent[0]) / 100;
						LifeFlag = 1;
					}
					else if(sinChar->Level != tempLife[1]) // 박재원 - 부스터 아이템 사용중 레벨 업 했을 경우 
					{	
						tempLife[0] = ( (sinChar->Life[1]+AddVirtualLife[0]-tempLife[0]) * (short)BoosterItem_DataPercent[0]) / 100;
						tempLife[1] = sinChar->Level;
					}
					else if(!AddVirtualLife[1] && LifeFlag == 1) // 박재원 - 부스터 아이템(생명력)과 버추얼 라이프(프티 스킬) 중복 사용중 버추얼 라이프 유지시간이 먼저 종료될 때
					{
						sinChar->Life[1] -= tempLife[0];
						tempLife[0] = (sinChar->Life[1] * (short)BoosterItem_DataPercent[0]) / 100;
					}

					sinfIncreLife = sinfIncreLife + tempLife[0];

				}
				else
					tempLife[0] = 0;

				break;
			case BOOSTER_ITEM_MANA: // 박재원 - 부스터 아이템(기력)
				SkillTextColor[SIN2_MANA] = SIN_TEXT_COLOR_PINK; // 사용중일때 인터페이스 글자색을 바꿔준다

				if(AddVirtualMana[1])
				{
					if(!ManaFlag)
					{	
						tempMana[0] = (sinChar->Mana[1] * (short)BoosterItem_DataPercent[0]) / 100;
						ManaFlag = 1;
					}
					else if(sinChar->Level != tempMana[1])
					{	
						tempMana[0] = ( (sinChar->Mana[1]-tempMana[0]) * (short)BoosterItem_DataPercent[0]) / 100;
						tempMana[1] = sinChar->Level;
					}

					sinfIncreMana = sinfIncreMana + tempMana[0];
				}
				else
					tempMana[0] = 0;

				break;
			case BOOSTER_ITEM_STAMINA: // 박재원 - 부스터 아이템(근력)
				SkillTextColor[SIN2_STAMINA] = SIN_TEXT_COLOR_PINK; // 사용중일때 인터페이스 글자색을 바꿔준다

				if(AddVirtualStamina[1])
				{
					if(!StaminaFlag)
					{	
						tempStamina[0] = (sinChar->Stamina[1] * (short)BoosterItem_DataPercent[0]) / 100;
						StaminaFlag = 1;
					}
					else if(sinChar->Level != tempStamina[1])
					{	
						tempStamina[0] = ( (sinChar->Stamina[1]-tempStamina[0]) * (short)BoosterItem_DataPercent[0]) / 100;
						tempStamina[1] = sinChar->Level;
					}	

					sinfIncreStamina = sinfIncreStamina + tempStamina[0];

				}
				else
					tempStamina[0] = 0;

				break;

			}
		}
	}

	//////// 저항력 가중치 
	for(i = 1 ; i < MAX_USESKILL  ; i++){ //노멀 어택을 빼준다 
		if(sinSkill.UseSkill[i].Flag && sinSkill.UseSkill[i].Point){//사용 포인트가 있으면
			switch(sinSkill.UseSkill[i].CODE){
				case SKILL_POISON_ATTRIBUTE:       //데미지 증가 
					sinSkillResistance[sITEMINFO_POISON] = PlusPoison[sinSkill.UseSkill[i].Point-1];

				break;
				case SKILL_ICE_ATTRIBUTE:
					sinSkillResistance[sITEMINFO_ICE] = PlusIce[sinSkill.UseSkill[i].Point-1];

				break;
				case SKILL_FIRE_ATTRIBUTE:
					sinSkillResistance[sITEMINFO_FIRE] = PlusFire[sinSkill.UseSkill[i].Point-1];
	
				break;
			}
		}
	}

	//요기는 능력치 적용되기전 인자를 플러스 해주는곳
	int TempDivide = 1;
	for( i = 0 ; i < MAX_CONTINUE_SKILL ; i++){
		if(ContinueSkill[i].Flag){
			switch(ContinueSkill[i].CODE){
				case SKILL_ZENITH:
					if(ContinueSkill[i].PartyFlag)TempDivide = 2; //파티원일 경우는 50%만 적용한다 
					sinSkillResistance[sITEMINFO_BIONIC] += Zenith_Element[ContinueSkill[i].Point-1]/TempDivide;
					sinSkillResistance[sITEMINFO_FIRE] += Zenith_Element[ContinueSkill[i].Point-1]/TempDivide;
					sinSkillResistance[sITEMINFO_ICE] += Zenith_Element[ContinueSkill[i].Point-1]/TempDivide;
					sinSkillResistance[sITEMINFO_LIGHTING] += Zenith_Element[ContinueSkill[i].Point-1]/TempDivide;
					sinSkillResistance[sITEMINFO_POISON] += Zenith_Element[ContinueSkill[i].Point-1]/TempDivide;
				break;
			}
		}
	}
	/////////////////////////////////////////////
	for(i=0;i<8;i++){ //저항력 
		sinChar->Resistance[i] = sinResistance[i]+sinAdd_Resistance[i]+sinSkillResistance[i];
		
	}

	//////////////////////// 데미지 가중치 
	for(i = 1 ; i < MAX_USESKILL  ; i++){ //노멀 어택을 빼준다 
		if(sinSkill.UseSkill[i].Flag && sinSkill.UseSkill[i].Point){ //사용 포인트가 있으면 
			//////////// 무기에 상관없이 능력치를 조정한다 
			//switch(sinSkill.UseSkill[i].CODE){

			//}
			//////////////같은 계열무기일경우에 해당 
			{	// the new classes' passives (NcSkills): damage, critical and accuracy
				sNCSKILL *nc = NC_FindCode( sinSkill.UseSkill[i].CODE );
				if ( nc && nc->Kind==NCK_PASSIVE ) {
					int wpok = 0, w;
					if ( sInven[0].ItemIndex )
						for( w=0; w<8; w++ )
							if ( sinSkill.UseSkill[i].Skill_Info.UseWeaponCode[w] &&
								sinSkill.UseSkill[i].Skill_Info.UseWeaponCode[w]==(cInvenTory.InvenItem[sInven[0].ItemIndex-1].CODE&sinITEM_MASK2) ) wpok = 1;
					int st[2] = { nc->Stat, nc->Stat2 };
					int *va[2] = { nc->Val, nc->Val2 };
					for( int s=0; s<2; s++ ) {
						int v = NC_Rank( va[s] , sinSkill.UseSkill[i].Point );
						switch( st[s] ) {
						case NCS_DMG_PCT:
							if ( wpok ) {
								sinSkillDamage[0] += (int)((v*TempDamage[0])/100);
								sinSkillDamage[1] += (int)((v*TempDamage[1])/100);
							}
							break;
						case NCS_CRIT:		if ( wpok ) sinCritical += v;	break;
						case NCS_ACC_PCT:	sinAttack_Rating += (int)((v*TempAttack_Rating)/100);	break;
						}
					}
				}
			}
			for(cnt = 0; cnt < 8 ; cnt++){
				switch(sinSkill.UseSkill[i].CODE){
					case SKILL_MELEE_MASTERY:       //데미지 증가 
						if(sInven[0].ItemIndex){
							if(sinSkill.UseSkill[i].Skill_Info.UseWeaponCode[cnt] == (cInvenTory.InvenItem[sInven[0].ItemIndex-1].CODE&sinITEM_MASK2)){
								sinSkillDamage[0] = (int)(((float)Melee_Mastery_DamagePercent[sinSkill.UseSkill[i].Point-1]/100)*TempDamage[0]);
								sinSkillDamage[1] = (int)(((float)Melee_Mastery_DamagePercent[sinSkill.UseSkill[i].Point-1]/100)*TempDamage[1]);
							}
						}
					break;
					case SKILL_SHOOTING_MASTERY:    //활 데미지 증가  
						if(sInven[0].ItemIndex){
							if(sinSkill.UseSkill[i].Skill_Info.UseWeaponCode[cnt] == (cInvenTory.InvenItem[sInven[0].ItemIndex-1].CODE&sinITEM_MASK2)){
								sinSkillDamage[0] = (int)(((float)S_Mastery_DamagePercent[sinSkill.UseSkill[i].Point-1]/100)*TempDamage[0]);
								sinSkillDamage[1] = (int)(((float)S_Mastery_DamagePercent[sinSkill.UseSkill[i].Point-1]/100)*TempDamage[1]);
								
							}
						}
					break;
					case SKILL_WEAPONE_DEFENCE_MASTERY:
						if(sInven[0].ItemIndex){
							if(sinSkill.UseSkill[i].Skill_Info.UseWeaponCode[cnt] == (cInvenTory.InvenItem[sInven[0].ItemIndex-1].CODE&sinITEM_MASK2)){
								if(sInven[1].ItemIndex) //방패가 있으면 브레이크 
									if(  sinDS1 == (cInvenTory.InvenItem[sInven[1].ItemIndex-1].CODE&sinITEM_MASK2)) //방패를 차고있지 않을때만 적용 
										break;

								sinBlock_Rate += W_D_Mastery_Block[sinSkill.UseSkill[i].Point-1];
									
							}
						}
					break;
					case SKILL_DIONS_EYE:
						if(sInven[0].ItemIndex){
							if(sinSkill.UseSkill[i].Skill_Info.UseWeaponCode[cnt] == (cInvenTory.InvenItem[sInven[0].ItemIndex-1].CODE&sinITEM_MASK2))
								sinAttack_Rating  += (D_Eye_Attack_Rate[sinSkill.UseSkill[i].Point-1]*TempAttack_Rating)/100;
						}

					break;

					case SKILL_THROWING_MASTERY:
						if(sInven[0].ItemIndex){
							if(sinSkill.UseSkill[i].Skill_Info.UseWeaponCode[cnt] == (cInvenTory.InvenItem[sInven[0].ItemIndex-1].CODE&sinITEM_MASK2)){
								sinSkillDamage[0] = (int)(((float)T_Mastery_Damage[sinSkill.UseSkill[i].Point-1]/100)*TempDamage[0]);
								sinSkillDamage[1] = (int)(((float)T_Mastery_Damage[sinSkill.UseSkill[i].Point-1]/100)*TempDamage[1]);
							}
						}
					break;
					case SKILL_MECHANIC_WEAPON:
						if(sInven[0].ItemIndex){
							if(sinChar->JobBitMask & InvenItem[sInven[0].ItemIndex-1].sItemInfo.JobCodeMask){
								sinSkillDamage[0] = (int)(((float)M_Weapon_Mastey[sinSkill.UseSkill[i].Point-1]/100)*TempDamage[0]);
								sinSkillDamage[1] = (int)(((float)M_Weapon_Mastey[sinSkill.UseSkill[i].Point-1]/100)*TempDamage[1]);
							}
							//메카니션 유니크 아이템
							if(InvenItem[sInven[0].ItemIndex-1].sItemInfo.UniqueItem == 2){
								sinSkillDamage[0] = (int)(((float)M_Weapon_Mastey[sinSkill.UseSkill[i].Point-1]/100)*TempDamage[0]);
								sinSkillDamage[1] = (int)(((float)M_Weapon_Mastey[sinSkill.UseSkill[i].Point-1]/100)*TempDamage[1]);							
							}

						}
					break;
					case SKILL_CRITICAL_MASTERY:
						if(sInven[0].ItemIndex){
							if(sinSkill.UseSkill[i].Skill_Info.UseWeaponCode[cnt] == (cInvenTory.InvenItem[sInven[0].ItemIndex-1].CODE&sinITEM_MASK2)){
								sinCritical += Critical_Mastery_Critical[sinSkill.UseSkill[i].Point-1];

							}
						}
					break;
					case SKILL_SWORD_MASTERY:
						if(sInven[0].ItemIndex){
							if(sinSkill.UseSkill[i].Skill_Info.UseWeaponCode[cnt] == (cInvenTory.InvenItem[sInven[0].ItemIndex-1].CODE&sinITEM_MASK2)){
								sinSkillDamage[0] = (int)(((float)Sword_Mastery_DamagePercent[sinSkill.UseSkill[i].Point-1]/100)*TempDamage[0]);
								sinSkillDamage[1] = (int)(((float)Sword_Mastery_DamagePercent[sinSkill.UseSkill[i].Point-1]/100)*TempDamage[1]);
								
							}
						}
					break;
					case SKILL_EVASION_MASTERY: //모하는 애일까 -_-?


					break;
				}
			}
		}
	}

	/////////캐릭터에 해당하는 무기사용시 데미지 추가 
	switch(sinChar->JOB_CODE){
		case SIN_CHAR_FIGHTER:
			if(sinWA1==(cInvenTory.InvenItem[sInven[0].ItemIndex-1].CODE&sinITEM_MASK2))
				sinCharItemDamage = sinChar->Level/6;

		break;

		case SIN_CHAR_MECANICIAN:
			if(sInven[0].ItemIndex){
				if(sinChar->JobBitMask & InvenItem[sInven[0].ItemIndex-1].sItemInfo.JobCodeMask)
					sinCharItemDamage = sinChar->Level/6;
				//메카니션 유니크 아이템 주무기군 설정 
				if(InvenItem[sInven[0].ItemIndex-1].sItemInfo.UniqueItem == 2)
					sinCharItemDamage = sinChar->Level/6;				
			}		

		break;

		case SIN_CHAR_ARCHER:
			if(sinWS1==(cInvenTory.InvenItem[sInven[0].ItemIndex-1].CODE&sinITEM_MASK2))
				sinCharItemDamage = sinChar->Level/6;

		break;

		case SIN_CHAR_PIKEMAN:
			if(sinWP1==(cInvenTory.InvenItem[sInven[0].ItemIndex-1].CODE&sinITEM_MASK2))
				sinCharItemDamage = sinChar->Level/6;
		break; 
		case SIN_CHAR_KNIGHT:
			if(sinWS2==(cInvenTory.InvenItem[sInven[0].ItemIndex-1].CODE&sinITEM_MASK2))
				sinCharItemDamage = sinChar->Level/6;

		break;
		case SIN_CHAR_ATALANTA:
			if(sinWT1==(cInvenTory.InvenItem[sInven[0].ItemIndex-1].CODE&sinITEM_MASK2))
				sinCharItemDamage = sinChar->Level/6;

		break;
		case SIN_CHAR_PRIESTESS:
			if(sinWM1==(cInvenTory.InvenItem[sInven[0].ItemIndex-1].CODE&sinITEM_MASK2))
				sinCharItemDamage = sinChar->Level/6;

		break;
		case SIN_CHAR_MAGICIAN:
			if(sinWM1==(cInvenTory.InvenItem[sInven[0].ItemIndex-1].CODE&sinITEM_MASK2))
				sinCharItemDamage = sinChar->Level/6;

		break;


	}
	int TempChangeDamageFlag = 0;
	int TempChangeAbsorbFlag = 0;
	//스킬로 색이바뀌는 부분때문에 이렇게 했따 
	for( i=0 ; i < 10 ; i++){  
		if(ContinueSkill[i].Flag){
			for(cnt = 0; cnt < 8 ; cnt++){
				switch(ContinueSkill[i].CODE){
					case SKILL_AUTOMATION: 
					case SKILL_MAXIMIZE:
						if(ContinueSkill[i].Skill_Info.UseWeaponCode[cnt] ==(cInvenTory.InvenItem[sInven[0].ItemIndex-1].CODE&sinITEM_MASK2)){
							TempChangeDamageFlag =1;
						}
					break;
					case SKILL_METAL_ARMOR:
						if(MetalArmor){ //메카니션 특화아머가 있을경우에만 적용된다 
							TempChangeAbsorbFlag = 1;

						}
					break;
					case SKILL_PHYSICAL_ABSORB:
						TempChangeAbsorbFlag = 1;
					break;
					case SKILL_FORCE_ORB:
						TempChangeDamageFlag =1;
					break;
					case SKILL_COMPULSION:
						TempChangeAbsorbFlag = 1;
					break;
					case CLANSKILL_ABSORB:
						TempChangeAbsorbFlag = 1;
					break;
					case CLANSKILL_ATTACK:
						TempChangeDamageFlag = 1;
					break;
					case SCROLL_INVULNERABILITY:
						TempChangeDamageFlag = 1;
					break;
				}			
			}
		}
	}
	
	// 박재원 - 얼큰이 포션을 사용하면 데미지 텍스트를 바꾼다.
	if(sinChar->dwEventTime_T && sinChar->SizeLevel > 1) // 박재원 - 축구공 포션 사용시에는 공격력 텍스트 색상이 안바뀐다.
	{
		TempChangeDamageFlag = 1;
	}

	//주무기군 데미지 텍스트 색을 표시한다 스킬로 데미지 올라가는 부분이 아닐경우에 표시한다 
	if(sinCharItemDamage)
		SkillTextColor[SIN2_DAMAGE] = SIN_TEXT_COLOR_GOLD;
	else if(TempChangeDamageFlag)
		SkillTextColor[SIN2_DAMAGE] = SIN_TEXT_COLOR_GOLD;
	else SkillTextColor[SIN2_DAMAGE] = SIN_TEXT_COLOR_WHITE;

	//흡수율관련
	if(TempChangeAbsorbFlag)
		SkillTextColor[SIN2_ABSORB] = SIN_TEXT_COLOR_BLUE;
	else SkillTextColor[SIN2_ABSORB] = SIN_TEXT_COLOR_WHITE;

	sinChar->Potion_Space =	sinPotion_Space; //포션을 가질수 있는 한계치 

	fstrength = (float)sinChar->Strength;
	fhealth   = (float)sinChar->Health;
	ftalent   = (float)sinChar->Talent;
	fSpirit   = (float)sinChar->Spirit;
	fDexterity = (float)sinChar->Dexterity;
	fLevel    = (float)sinChar->Level;


	//개인 명중률 
	sinChar->Attack_Rating = (int)((sinAttack_Rating+(fDexterity*3.1)+(fLevel*1.9)+(ftalent*1.5))+sinLev_Attack_Rating);
	/* //피씨방 명중력 업 이벤트
	if((sBiInfo->EventPlay[0] & VIP_USER)!=0){
		sinChar->Attack_Rating += (int)((float)sinChar->Attack_Rating*(15.0f/100.0f));
		SkillTextColor[SIN2_ATTACK_RATE] = SIN_TEXT_COLOR_ORANGE;

	}
	*/
	/*
	
	////주먹 대미지 
	if(sinDamageFunction[0] == 1 && !sInven[0].ItemIndex ){
		sinChar->Attack_Damage[0] = 1+((sinChar->Strength+130)/130) + ((sinChar->Talent + sinChar->Dexterity)/40)+sinLev_Damage[0];
		sinChar->Attack_Damage[1] = 2+((sinChar->Strength+130)/130) + ((sinChar->Talent + sinChar->Dexterity)/35)+sinLev_Damage[1];
	}
	if((sinDamageFunction[0] == 2 || sinDamageFunction[0] == 3 || sinDamageFunction[0] == 4)&& !sInven[0].ItemIndex){
		sinChar->Attack_Damage[0] = 1+((sinChar->Strength+200)/200) + ((sinChar->Talent + sinChar->Dexterity)/50)+sinLev_Damage[0];
		sinChar->Attack_Damage[1] = 2+((sinChar->Strength+200)/200) + ((sinChar->Talent + sinChar->Dexterity)/45)+sinLev_Damage[1];
	}

	////(근접형1)
	if((sinDamageFunction[0] == 1 && (InvenItem[sInven[0].ItemIndex-1].WeaponClass == 1 || InvenItem[sInven[0].ItemIndex-1].WeaponClass == 3))){
		sinChar->Attack_Damage[0] = 1+(sinAttack_Damage[0]*(sinChar->Strength+145)/145) +
									((sinChar->Talent + sinChar->Dexterity)/40)+sinLev_Damage[0]+sinSkillDamage[0]+sinCharItemDamage;
		sinChar->Attack_Damage[1] = 2+(sinAttack_Damage[1]*(sinChar->Strength+145)/145) + 
									((sinChar->Talent + sinChar->Dexterity)/35) +sinLev_Damage[1]+sinSkillDamage[1]+sinCharItemDamage;
	}
	if((sinDamageFunction[0] == 2 && (InvenItem[sInven[0].ItemIndex-1].WeaponClass == 1 || InvenItem[sInven[0].ItemIndex-1].WeaponClass == 3))){
	////(근접형2)
		sinChar->Attack_Damage[0] = 1+(sinAttack_Damage[0]*(sinChar->Strength+160)/160) +
									((sinChar->Talent + sinChar->Dexterity)/45)+sinLev_Damage[0]+sinSkillDamage[0]+sinCharItemDamage;
		sinChar->Attack_Damage[1] = 2+(sinAttack_Damage[1]*(sinChar->Strength+160)/160) +
									((sinChar->Talent + sinChar->Dexterity)/40) +sinLev_Damage[1]+sinSkillDamage[1]+sinCharItemDamage;
	}
	if((sinDamageFunction[0] == 3 && (InvenItem[sInven[0].ItemIndex-1].WeaponClass == 1 || InvenItem[sInven[0].ItemIndex-1].WeaponClass == 3))){
	////(근접형3)
		sinChar->Attack_Damage[0] = 1+(sinAttack_Damage[0]*(sinChar->Strength+200)/200) +
									((sinChar->Talent + sinChar->Dexterity)/50)+sinLev_Damage[0]+sinSkillDamage[0]+sinCharItemDamage;
		sinChar->Attack_Damage[1] = 2+(sinAttack_Damage[1]*(sinChar->Strength+200)/200) +
									((sinChar->Talent + sinChar->Dexterity)/45) +sinLev_Damage[1]+sinSkillDamage[1]+sinCharItemDamage;
	}

	if(sinDamageFunction[1] == 1 && InvenItem[sInven[0].ItemIndex-1].WeaponClass == 2){
	////(발사형1)
		sinChar->Attack_Damage[0] = 1+(sinAttack_Damage[0]*(sinChar->Dexterity+145)/145) + 
									((sinChar->Talent + sinChar->Strength)/40)+sinLev_Damage[0]+sinSkillDamage[0]+sinCharItemDamage;
		sinChar->Attack_Damage[1] = 2+(sinAttack_Damage[1]*(sinChar->Dexterity+145)/145) + 
									((sinChar->Talent + sinChar->Strength)/35)+sinLev_Damage[1]+sinSkillDamage[1]+sinCharItemDamage;
	}
	if(sinDamageFunction[1] == 2 && InvenItem[sInven[0].ItemIndex-1].WeaponClass == 2){
	////(발사형2)
		sinChar->Attack_Damage[0] = 1+(sinAttack_Damage[0]*(sinChar->Dexterity+200)/200) +
									((sinChar->Talent + sinChar->Strength)/50)+sinLev_Damage[0]+sinSkillDamage[0]+sinCharItemDamage;
		sinChar->Attack_Damage[1] = 2+(sinAttack_Damage[1]*(sinChar->Dexterity+200)/200) +
									((sinChar->Talent + sinChar->Strength)/45)+sinLev_Damage[1]+sinSkillDamage[1]+sinCharItemDamage;
	}

	if(sinDamageFunction[2] == 1 && InvenItem[sInven[0].ItemIndex-1].WeaponClass == 3){
	////(법사형1)
		sinChar->Attack_Damage[0] = 1+(sinAttack_Damage[0]*(sinChar->Spirit+160)/160) +
									((sinChar->Talent)/30)+sinLev_Damage[0]+sinSkillDamage[0]+sinCharItemDamage;
		sinChar->Attack_Damage[1] = 2+(sinAttack_Damage[1]*(sinChar->Spirit+160)/160) +
									((sinChar->Talent)/25)+sinLev_Damage[1]+sinSkillDamage[1]+sinCharItemDamage;
	}
	if(sinDamageFunction[2] == 2 && InvenItem[sInven[0].ItemIndex-1].WeaponClass == 3){
	////(법사형2)
		sinChar->Attack_Damage[0] = 1+(sinAttack_Damage[0]*(sinChar->Spirit+180)/180) +
									((sinChar->Talent)/30)+sinLev_Damage[0]+sinSkillDamage[0]+sinCharItemDamage;
		sinChar->Attack_Damage[1] = 2+(sinAttack_Damage[1]*(sinChar->Spirit+180)/180) +
									((sinChar->Talent)/25)+sinLev_Damage[1]+sinSkillDamage[1]+sinCharItemDamage;
	}
	sinChar->Attack_Damage[0] += 1; //추가 데미지 
	sinChar->Attack_Damage[1] += 1;
	
	*/
	//////////////////////////////////  *수정된 데미지 
	////주먹 대미지  **** 생명력함수도 고쳐야함 (주의)
	//sinTestFlag2++;

#ifdef _XTRAP_GUARD_4_CLIENT //HEAP MEMORY TEST
	XTrap_CE1_Func11_Protect( &(sinChar->Attack_Damage), sizeof(sinChar->Attack_Damage) );	//보호영역 무결성 체크
#endif

	//박상열휴가갔네
	if(sinDamageFunction[0] == 1 && !sInven[0].ItemIndex ){
		sinChar->Attack_Damage[0] = 1+((sinChar->Strength+130)/130) + ((sinChar->Talent + sinChar->Dexterity)/40)+sinLev_Damage[0];
		sinChar->Attack_Damage[1] = 2+((sinChar->Strength+130)/130) + ((sinChar->Talent + sinChar->Dexterity)/35)+sinLev_Damage[1];
	}
	if((sinDamageFunction[0] == 2 || sinDamageFunction[0] == 3 || sinDamageFunction[0] == 4)&& !sInven[0].ItemIndex){
		sinChar->Attack_Damage[0] = 1+((sinChar->Strength+200)/200) + ((sinChar->Talent + sinChar->Dexterity)/50)+sinLev_Damage[0];
		sinChar->Attack_Damage[1] = 2+((sinChar->Strength+200)/200) + ((sinChar->Talent + sinChar->Dexterity)/45)+sinLev_Damage[1];
	}

	////(근접형1)
	if((sinDamageFunction[0] == 1 && (InvenItem[sInven[0].ItemIndex-1].WeaponClass == 1 || InvenItem[sInven[0].ItemIndex-1].WeaponClass == 3))){
		sinChar->Attack_Damage[0] = 1+(sinAttack_Damage[0]*(sinChar->Strength+130)/130) +
									((sinChar->Talent + sinChar->Dexterity)/40)+sinLev_Damage[0]+sinSkillDamage[0]+sinCharItemDamage + ((sinAttack_Damage[0]+sinAttack_Damage[1])/16);
		sinChar->Attack_Damage[1] = 3+(sinAttack_Damage[1]*(sinChar->Strength+130)/130) + 
									((sinChar->Talent + sinChar->Dexterity)/40) +sinLev_Damage[1]+sinSkillDamage[1]+sinCharItemDamage;
	}
	if((sinDamageFunction[0] == 2 && (InvenItem[sInven[0].ItemIndex-1].WeaponClass == 1 || InvenItem[sInven[0].ItemIndex-1].WeaponClass == 3))){
	////(근접형2)
		sinChar->Attack_Damage[0] = 1+(sinAttack_Damage[0]*(sinChar->Strength+150)/150) +
									((sinChar->Talent + sinChar->Dexterity)/45)+sinLev_Damage[0]+sinSkillDamage[0]+sinCharItemDamage + ((sinAttack_Damage[0]+sinAttack_Damage[1])/16);
		sinChar->Attack_Damage[1] = 3+(sinAttack_Damage[1]*(sinChar->Strength+150)/150) +
									((sinChar->Talent + sinChar->Dexterity)/45) +sinLev_Damage[1]+sinSkillDamage[1]+sinCharItemDamage;
	}
	if((sinDamageFunction[0] == 3 && (InvenItem[sInven[0].ItemIndex-1].WeaponClass == 1 || InvenItem[sInven[0].ItemIndex-1].WeaponClass == 3))){
	////(근접형3)
		sinChar->Attack_Damage[0] = 1+(sinAttack_Damage[0]*(sinChar->Strength+190)/190) +
									((sinChar->Talent + sinChar->Dexterity)/50)+sinLev_Damage[0]+sinSkillDamage[0]+sinCharItemDamage + ((sinAttack_Damage[0]+sinAttack_Damage[1])/16);
		sinChar->Attack_Damage[1] = 3+(sinAttack_Damage[1]*(sinChar->Strength+190)/190) +
									((sinChar->Talent + sinChar->Dexterity)/50) +sinLev_Damage[1]+sinSkillDamage[1]+sinCharItemDamage;
	}

	if(sinDamageFunction[1] == 1 && InvenItem[sInven[0].ItemIndex-1].WeaponClass == 2){
	////(발사형1)
		sinChar->Attack_Damage[0] = 1+(sinAttack_Damage[0]*(sinChar->Dexterity+130)/130) + 
									((sinChar->Talent + sinChar->Strength)/40)+sinLev_Damage[0]+sinSkillDamage[0]+sinCharItemDamage + ((sinAttack_Damage[0]+sinAttack_Damage[1])/16);
		sinChar->Attack_Damage[1] = 3+(sinAttack_Damage[1]*(sinChar->Dexterity+130)/130) + 
									((sinChar->Talent + sinChar->Strength)/40)+sinLev_Damage[1]+sinSkillDamage[1]+sinCharItemDamage;
	}
	if(sinDamageFunction[1] == 2 && InvenItem[sInven[0].ItemIndex-1].WeaponClass == 2){
	////(발사형2)
		sinChar->Attack_Damage[0] = 1+(sinAttack_Damage[0]*(sinChar->Dexterity+190)/190) +
									((sinChar->Talent + sinChar->Strength)/50)+sinLev_Damage[0]+sinSkillDamage[0]+sinCharItemDamage + ((sinAttack_Damage[0]+sinAttack_Damage[1])/16);
		sinChar->Attack_Damage[1] = 3+(sinAttack_Damage[1]*(sinChar->Dexterity+190)/190) +
									((sinChar->Talent + sinChar->Strength)/50)+sinLev_Damage[1]+sinSkillDamage[1]+sinCharItemDamage;
	}

	if(sinDamageFunction[2] == 1 && InvenItem[sInven[0].ItemIndex-1].WeaponClass == 3){
	////(법사형1)
		sinChar->Attack_Damage[0] = 1+(sinAttack_Damage[0]*(sinChar->Spirit+150)/150) +
									((sinChar->Talent)/30)+sinLev_Damage[0]+sinSkillDamage[0]+sinCharItemDamage + ((sinAttack_Damage[0]+sinAttack_Damage[1])/16);
		sinChar->Attack_Damage[1] = 3+(sinAttack_Damage[1]*(sinChar->Spirit+150)/150) +
									((sinChar->Talent)/30)+sinLev_Damage[1]+sinSkillDamage[1]+sinCharItemDamage;
	}
	if(sinDamageFunction[2] == 2 && InvenItem[sInven[0].ItemIndex-1].WeaponClass == 3){
	////(법사형2)
		sinChar->Attack_Damage[0] = 1+(sinAttack_Damage[0]*(sinChar->Spirit+170)/170) +
									((sinChar->Talent)/30)+sinLev_Damage[0]+sinSkillDamage[0]+sinCharItemDamage + ((sinAttack_Damage[0]+sinAttack_Damage[1])/16);
		sinChar->Attack_Damage[1] = 3+(sinAttack_Damage[1]*(sinChar->Spirit+170)/170) +
									((sinChar->Talent)/30)+sinLev_Damage[1]+sinSkillDamage[1]+sinCharItemDamage;
	}
	sinChar->Attack_Damage[0] += 1; //추가 데미지 
	sinChar->Attack_Damage[1] += 1;
   ////////////////////////// 요기까지 
   
#ifdef _XTRAP_GUARD_4_CLIENT //HEAP MEMORY TEST
	XTrap_CE1_Func12_Protect( &(sinChar->Attack_Damage), sizeof(sinChar->Attack_Damage) );	//보호영역 보호
	XTrap_CE1_Func13_Free( &(sinChar->Attack_Damage), sizeof(sinChar->Attack_Damage) );		//보호영역 해제
#endif

	//데미지가 올라가는 부분을 보여준다
	if(ViewDamagePercent){
		sinTempDamage2[0] += (sinChar->Attack_Damage[0]*ViewDamagePercent)/100; //디스플레이로 보여주기만한다
		sinTempDamage2[1] += (sinChar->Attack_Damage[1]*ViewDamagePercent)/100; //디스플레이로 보여주기만한다
	}
	else if(ViewDamageNum){
		sinTempDamage2[0] += ViewDamageNum;
		sinTempDamage2[1] += ViewDamageNum;
	}

	//크리티컬힛 
	sinChar->Critical_Hit = sinCritical+sinAdd_Critical_Hit;

	//크리티컬힛 최대치 보정 
	if(sinChar->Critical_Hit > 50)sinChar->Critical_Hit = 50;
	fstrength = (float)sinChar->Strength;
	fhealth   = (float)sinChar->Health;
	ftalent   = (float)sinChar->Talent;
	fSpirit   = (float)sinChar->Spirit;

	//방어력 
	sinChar->Defence  = (int)((fDexterity/2)+(ftalent/4)+fLevel*1.4+sinDefense)+sinAdd_Defence;
	fDefense       = (float)sinChar->Defence;	

	//흡수력 
	CountAbsorbItem /= 2;
	if(CountAbsorbItem > 2) CountAbsorbItem = 2;
	sinChar->Absorption = (int)((fDefense/100+(fLevel/10)+sinAbsorption+sinAdd_fAbsorb)+((fstrength+ftalent)/40))+1+CountAbsorbItem; //소숫점때문에 보정치를 더올렸다
	//공격 거리 
	sinChar->Shooting_Range = sinShooting_Range+sinAdd_Shooting_Range;

	//시야 
	sinChar->Sight = sinSight + 180 ;//현재는 그냥 메카니션셋팅으로  적용 (전업및 캐릭터 클래스에 맞게 설정)

	//블럭율
	sinChar->Chance_Block = (int)(sinBlock_Rate+sinAdd_fBlock_Rating);
		
	//무게 최대치 
	// pluto 마이트 오브 아웰  캐쉬템 소지량 증가
	if( chaPremiumitem.m_MightofAwellTime > 0 )
	{
		sinChar->Weight[1] = (short)(((fstrength*2) +(fhealth*1.5) + fLevel*3)+60 + chaPremiumitem.m_MightofAwellWeight);
	}
	else
	{
		sinChar->Weight[1] = (short)(((fstrength*2) +(fhealth*1.5) + fLevel*3)+60);
	}

	fMaxWeight   = (float)sinChar->Weight[1];
	fNowWeight   = (float)sinChar->Weight[0];


	// 장별 - 그라비티 스크롤
	if( sinChar->GravityScroolCheck[0] == 0 ) 
	{
		sinChar->Weight[1] = (short)(((fstrength*2) +(fhealth*1.5) + fLevel*3)+60);
	
	}

	if( sinChar->GravityScroolCheck[0] > 0 )
	{
		sinChar->Weight[1] = (short)(((fstrength*2) +(fhealth*1.5) + fLevel*3)+60) + ( sinChar->GravityScroolCheck[0] )*50;
		
	}
	
	fMaxWeight   = (float)sinChar->Weight[1];
	fNowWeight   = (float)sinChar->Weight[0];



	fAttack_Rating = (float)sinChar->Attack_Rating;
	//sinChar->    sinAdd_fMagic_Mastery

	int Accuracy_Temp;
	while(1){
		if(Accuracy_Table[cnt][0] == 0)break;
		Accuracy_Temp = sinChar->Attack_Rating;
		if( Accuracy_Temp > Accuracy_Table[cnt][0] && Accuracy_Temp < Accuracy_Table[cnt+1][0] ){
			sinChar->Accuracy = Accuracy_Table[cnt+1][1];	
			break;

		}
		cnt++;
	}

	//스피드
	sinChar->Move_Speed = (int)((((((ftalent +fhealth +fLevel+60)/150) - ( fNowWeight/  fMaxWeight)))+sinMoveSpeed)+sinAdd_fSpeed);
	sinChar->Move_Speed +=1;


#ifdef _XTRAP_GUARD_4_CLIENT //HEAP MEMORY TEST
	XTrap_CE1_Func11_Protect( &(sinChar->Life), sizeof(sinChar->Life) );	//보호영역 무결성 체크
#endif

	/////////////// 라이프 ////////
	//sinTestFlag2++;
	switch(sinChar->LifeFunction){
		case 1:
			sinChar->Life[1] = (short)(fLevel * 2.1 +((fhealth*2.4)+(fstrength*0.8))-10);  
			sinChar->Life[1] += (short)(sinLev_Life+sinfIncreLife);  
		break;
		case 2:
			sinChar->Life[1] = (short)(fLevel * 2.1 +((fhealth*2.2)+(fstrength*0.6))-5);
			sinChar->Life[1] += (short)(sinLev_Life+sinfIncreLife);
		break;
		case 3:
			sinChar->Life[1] = (short)(fLevel * 1.8 +((fhealth*2.1)+(fstrength*0.3)));
			sinChar->Life[1] += (short)(sinLev_Life+sinfIncreLife);
		break;
		case 4:
			sinChar->Life[1] = (short)((fLevel *1.5) + (fhealth *2.1));
			sinChar->Life[1] += (short)(sinLev_Life+sinfIncreLife);
		break;
		case 5:
			sinChar->Life[1] = (short)((fLevel *1.5) + (fhealth *1.9));
			sinChar->Life[1] += (short)(sinLev_Life+sinfIncreLife);
		break;

	}
	/*
	/////////////// 라이프 ////////
	switch(sinChar->LifeFunction){
		case 1:
			sinChar->Life[1] = (short)(fLevel * 1.5 +((fhealth*2.5)+(fstrength*0.8))-10);  
			sinChar->Life[1] += (short)(sinLev_Life+sinfIncreLife);  
		break;
		case 2:
			sinChar->Life[1] = (short)(fLevel * 1.5 +((fhealth*2.3)+(fstrength*0.6))-5);
			sinChar->Life[1] += (short)(sinLev_Life+sinfIncreLife);
		break;
		case 3:
			sinChar->Life[1] = (short)((fLevel *1.2) + (fhealth *2.2));
			sinChar->Life[1] += (short)(sinLev_Life+sinfIncreLife);
		break;
		case 4:
			sinChar->Life[1] = (short)((fLevel *1.2) + (fhealth *2.0));
			sinChar->Life[1] += (short)(sinLev_Life+sinfIncreLife);
		break;

	}
	*/
	

	//3차전업을 한경우는 Life 15를 올려준다
	if(!sinQuest_ChangeJob3.CODE && !SearchItemCode(0,ITEM_KIND_QUEST_WEAPON) &&
		sinChar->ChangeJob >=2 && sinChar->Level >=40){
		sinChar->Life[1] += 15;
	}

	//80퀘스트를 수행할경우 HP를 40 올려준다
	if((sinQuest_levelLog & QUESTBIT_LEVEL_90) == QUESTBIT_LEVEL_90){
		if(sinChar->Level >=90){
			sinChar->Life[1] += 40;
		}

	}
#ifdef _XTRAP_GUARD_4_CLIENT //HEAP MEMORY TEST
	XTrap_CE1_Func12_Protect( &(sinChar->Life), sizeof(sinChar->Life) );	//보호영역 보호
	XTrap_CE1_Func13_Free( &(sinChar->Life), sizeof(sinChar->Life) );		//보호영역 해제
#endif

//	FILE	*fp;
//	DWORD testFunc11, testFunc12, testFunc13;

//	fp = fopen("c:\\XTRAPTEST.txt", "a+t");

//	if(fp)
//	{

	
#ifdef _XTRAP_GUARD_4_CLIENT //HEAP MEMORY TEST
	XTrap_CE1_Func11_Protect( &(sinChar->Mana), sizeof(sinChar->Mana) );	//보호영역 무결성 체크
#endif

	/////////////// 마나 //////////		//kyle xtrapHeap
	switch(sinChar->ManaFunction){
		case 1:
			sinChar->Mana[1] = (short)((fLevel * 1.5f+(fSpirit*3.8f)));
			sinChar->Mana[1] += (short)(sinfIncreMana);
		break;
		case 2:
			sinChar->Mana[1] = (short)(sinChar->Level * 0.9f+(sinChar->Spirit*2.7f));
			sinChar->Mana[1] += (short)(sinLev_Mana+sinfIncreMana);
		break;
		case 3:
			sinChar->Mana[1] = (short)(sinChar->Level * 0.6f+(sinChar->Spirit*2.2f));
			sinChar->Mana[1] += (short)(sinLev_Mana+sinfIncreMana);
		break;
		
	}

#ifdef _XTRAP_GUARD_4_CLIENT //HEAP MEMORY TEST
	XTrap_CE1_Func12_Protect( &(sinChar->Mana), sizeof(sinChar->Mana) );	//보호영역 보호
	XTrap_CE1_Func13_Free( &(sinChar->Mana), sizeof(sinChar->Mana) );		//보호영역 해제
#endif


//	FILE *fp;
//	DWORD	Func11, Func12, Func13;
//	fp = fopen("c:\\XTRAP.txt", "a+t");

//	if(fp)
//	{

#ifdef _XTRAP_GUARD_4_CLIENT //HEAP MEMORY TEST
	XTrap_CE1_Func11_Protect( &(sinChar->Stamina), sizeof(sinChar->Stamina) );	//보호영역 무결성 체크
//	Func11 = XTrap_CE1_Func11_Protect( &(sinChar->Stamina[1]), sizeof(sinChar->Stamina[1]) );
#endif

	//스테미나
	sinChar->Stamina[1] = (short)((fhealth*1.4) + (fstrength +  ftalent) / 2 + (fLevel*2.3)+80+fSpirit); 
	sinChar->Stamina[1] += ((short)(sinfIncreStamina)); 

#ifdef _XTRAP_GUARD_4_CLIENT //HEAP MEMORY TEST
	XTrap_CE1_Func12_Protect( &(sinChar->Stamina), sizeof(sinChar->Stamina) );	//보호영역 보호
//	Func12 = XTrap_CE1_Func12_Protect( &(sinChar->Stamina[1]), sizeof(sinChar->Stamina[1]) );
	XTrap_CE1_Func13_Free( &(sinChar->Stamina), sizeof(sinChar->Stamina) );		//보호영역 해제
//	Func13 = XTrap_CE1_Func13_Free( &(sinChar->Stamina[1]), sizeof(sinChar->Stamina[1]) );
#endif

//	fprintf(fp, " Func11 = %d  Func12 = %d  Func13 = %d\n", Func11, Func12, Func13);
//	}
//	fclose(fp);

//	SYSTEMTIME st;
//	GetLocalTime(&st);
//	fprintf(fp, "%d/%d/%d - %d:%d\t", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute);
//		fprintf(fp, "Func11 = %d  Func12 = %d  Func13 = %d\n\n", testFunc11, testFunc12, testFunc13);
//	}
//	fclose(fp);

	//Regen 
	sinChar->Life_Regen		= sinfRegenLife + sinPer_Life_Regen; 
	sinChar->Mana_Regen		= sinfRegenMana + sinPer_Mana_Regen;
	sinChar->Stamina_Regen	= sinfRegenStamina + sinPer_Stamina_Regen;

	//공격속도 
	sinChar->Attack_Speed =	sinWeaponSpeed+sinAdd_Attack_Speed+sinSkillWeaponSpeed; //공격속도는 일단 무기속도로 나타낸다. 
	//if(sinChar->Attack_Speed > 8 )sinChar->Attack_Speed = 8; //8로 강제보정

	 
	for(i = 1 ; i < MAX_USESKILL ; i++){ //노멀 어택을 빼준다 
		if(sinSkill.UseSkill[i].Flag && sinSkill.UseSkill[i].Point){ //사용 포인트가 있으면 
			switch(sinSkill.UseSkill[i].CODE){
				case SKILL_PHYSICAL_TRANING: 
					TempSkillData = (((float)P_Traning_Stamina[sinSkill.UseSkill[i].Point-1])*(float)sinChar->Stamina[1]/100.0f);
					sinChar->Stamina[1] += (short)TempSkillData; //계산공식이 들어가므로 최대스테미나량을 구한다음에 이곳에서 +한다
				break;
				case SKILL_FIRE_JAVELIN: 
					sElement_Attack.Fire[0] += Fire_Javelin_Damage[sinSkill.UseSkill[i].Point-1][0];
					sElement_Attack.Fire[1] += Fire_Javelin_Damage[sinSkill.UseSkill[i].Point-1][1];
				break;
				case SKILL_MEDITATION:
					sinChar->Mana_Regen += Meditation_Regen[sinSkill.UseSkill[i].Point-1];
				break;
				case SKILL_MENTAL_MASTERY:
					TempSkillData = (float)((float)(Mental_Mastery_Mana[sinSkill.UseSkill[i].Point-1])*((float)sinChar->Mana[1]/100.0f));
					sinChar->Mana[1] += (short)TempSkillData; //계산공식이 들어가므로 최대마나량을 구한다음에 이곳에서 +한다
				break;
				case SKILL_BOOST_HEALTH:
					sinChar->Life[1] += (short)Boost_Health_Life[sinSkill.UseSkill[i].Point-1];

				break;
			
			}
		}
	}


	//요기는 능력치 적용된후 다른일을 하는곳
	/////////////////////////////////////////////
	SkillTextColor[SIN2_DEFENSE] = SIN_TEXT_COLOR_WHITE; //글색을 초기화해준다 Drastic_Spirit 땀시 땜빵!

	int OrbDamageTemp2 = 0;
	///////이쁘게 다시 잘만든 스킬 두둥!!!!!!////
	{	// the new classes' passives and buffs on the final stats: defense, life, mana, stamina
		int k, s;
		for( k=0; k<MAX_CONTINUE_SKILL+SIN_MAX_USE_SKILL; k++ ) {
			sNCSKILL *nc;
			int pt;
			if ( k<MAX_CONTINUE_SKILL ) {
				if ( !ContinueSkill[k].Flag ) continue;
				nc = NC_FindCode( ContinueSkill[k].CODE );
				if ( !nc || nc->Kind!=NCK_BUFF ) continue;
				pt = ContinueSkill[k].Point;
			}
			else {
				int u = k-MAX_CONTINUE_SKILL;
				if ( u<1 || !sinSkill.UseSkill[u].Flag || !sinSkill.UseSkill[u].Point ) continue;
				nc = NC_FindCode( sinSkill.UseSkill[u].CODE );
				if ( !nc || nc->Kind!=NCK_PASSIVE ) continue;
				pt = sinSkill.UseSkill[u].Point;
			}
			int st[3] = { nc->Stat, nc->Stat2, nc->Stat3 };
			int *va[3] = { nc->Val, nc->Val2, nc->Val3 };
			for( s=0; s<3; s++ ) {
				int v = NC_Rank( va[s] , pt );
				switch( st[s] ) {
				case NCS_DEF_PCT:	sinChar->Defence += (sinChar->Defence*v)/100;	SkillTextColor[SIN2_DEFENSE] = SIN_TEXT_COLOR_BLUE;	break;
				case NCS_MANA_PCT:	sinChar->Mana[1] += (sinChar->Mana[1]*v)/100;	break;
				case NCS_STAM_PCT:	sinChar->Stamina[1] += (sinChar->Stamina[1]*v)/100;	break;
				case NCS_LIFE_PCT:	sinChar->Life[1] += (sinChar->Life[1]*v)/100;	break;
				case NCS_VIT:		sinChar->Life[1] += v*2;	break;		// vitality: about 2 life a point
				case NCS_SPI_PCT:	sinChar->Mana[1] += (sinChar->Mana[1]*v)/100;	break;
				}
			}
		}
	}

	for(int i = 0 ; i < MAX_CONTINUE_SKILL ; i++){  
		if(ContinueSkill[i].Flag){
			switch(ContinueSkill[i].CODE){
				case SKILL_HOLY_BODY:
					TempSkillData = ((float)HolyBody_Absorb[ContinueSkill[i].Point-1]/100.0f);
					sinUndeadAbsorb = (int)((float)sinChar->Absorption*TempSkillData);
				break;
				case SKILL_DRASTIC_SPIRIT:
					TempSkillData = ((float)Drastic_Spirit_Defense[ContinueSkill[i].Point-1]/100.0f);
					sinChar->Defence += (int)((float)sinChar->Defence * TempSkillData);
					SkillTextColor[SIN2_DEFENSE] = SIN_TEXT_COLOR_BLUE;
				break;
				case SKILL_BLESSING_KRISHNA:	// tier 5 Priestess: defense +%
					sinChar->Defence += (sinChar->Defence*Blessing_Krishna_Defense[ContinueSkill[i].Point-1])/100;
					SkillTextColor[SIN2_DEFENSE] = SIN_TEXT_COLOR_BLUE;
				break;
				case SKILL_THIRD_ANIMA:		// tier 5 Magician: max mana -%
					sinChar->Mana[1] -= (sinChar->Mana[1]*Third_Anima_ManaDown[ContinueSkill[i].Point-1])/100;
					if ( sinChar->Mana[0]>sinChar->Mana[1] ) sinChar->Mana[0] = sinChar->Mana[1];
				break;
				case SKILL_TALARIA:			// tier 5 Atalanta: move speed +
					sinChar->Move_Speed += Talaria_Speed[ContinueSkill[i].Point-1];
				break;
				case SKILL_AMPLIFIED:		// tier 5 Pikeman: attack rating +%
					sinChar->Attack_Rating += (sinChar->Attack_Rating*Amplified_AddHit[ContinueSkill[i].Point-1])/100;
					SkillTextColor[SIN2_ATTACK_RATE] = SIN_TEXT_COLOR_ORANGE;
					SkillTextColor[SIN2_ABSORB] = SIN_TEXT_COLOR_RED;
				break;
				case SKILL_SPARK_SHIELD: //계산은 위에서하고 글자 색만 보정해준다
				case SKILL_METAL_ARMOR: 
					if(MetalArmor){ //메카니션 특화아머가 있을경우에만 적용된다 
						SkillTextColor[SIN2_DEFENSE] = SIN_TEXT_COLOR_BLUE; 
					}
				break;
				case SKILL_CONCENTRATION:  //자신의 명중력이 관련되기때문에 밑에서 계산해준다
					for(p = 0; p < 8 ; p++){ //다른 무기군도 체크된다   
						if(sInven[0].ItemIndex){
							if(ContinueSkill[i].Skill_Info.UseWeaponCode[p] == (cInvenTory.InvenItem[sInven[0].ItemIndex-1].CODE&sinITEM_MASK2)){
								sinChar->Attack_Rating  += Concentration_AttackRate[ContinueSkill[i].Point-1];
								SkillTextColor[SIN2_ATTACK_RATE] = SIN_TEXT_COLOR_ORANGE; 
								Flag3 = 1;
								break;
							}
						}

					}
					if(!Flag3){
						//스킬사용무기가 아니면 적용만안되게한다
					//	ContinueSkill[i].CheckTime = ContinueSkill[i].UseTime*70; 
						SkillTextColor[SIN2_ATTACK_RATE] = SIN_TEXT_COLOR_WHITE;
					}
				break;
				case SKILL_FORCE_ORB:
					// 박재원 - 매직 포스, 빌링 매직 포스 추가시 캐릭터 정보창에 보여지는 공격력(디스플레이시에만 보여줄 데미지)
					if(ContinueSkill[i].ElementIndex == 1000) // 빌링 매직 포스 사용중일때
					{
						// fo135 ~ fo137 // 빌링 매직 포스
						OrbDamageTemp2 = (sinChar->Attack_Damage[0]+ ((sinChar->Attack_Damage[1] - sinChar->Attack_Damage[0])/2))/15;
						sinTempDamage2[0] += OrbDamageTemp2; 
						sinTempDamage2[1] += OrbDamageTemp2;
					}
					else // 일반 매직 포스 사용중일때 // 장별 - 포스 인페르나, 이니그마 포스 수정
					{
						// fo107 ~ fo110 // 머키 포스 ~ 미라지 포스 - 공격력가중 10% 
						if(ContinueSkill[i].Point >= 7 && ContinueSkill[i].Point <= 10)
						{     
							OrbDamageTemp2 = (sinChar->Attack_Damage[0]+ ((sinChar->Attack_Damage[1] - sinChar->Attack_Damage[0])/2))/10;
							sinTempDamage2[0] += OrbDamageTemp2; 
							sinTempDamage2[1] += OrbDamageTemp2;
						}
						else if(ContinueSkill[i].Point == 11) // 인페르나 포스
						{     
							OrbDamageTemp2 =  (((sinChar->Attack_Damage[1] + sinChar->Attack_Damage[0])/2)*15)/100;
							sinTempDamage2[0] += OrbDamageTemp2; 
							sinTempDamage2[1] += OrbDamageTemp2;
						}
						else if(ContinueSkill[i].Point == 12) // 이니그마 포스
						{     
							OrbDamageTemp2 = (((sinChar->Attack_Damage[1] + sinChar->Attack_Damage[0])/2)*20)/100;
							sinTempDamage2[0] += OrbDamageTemp2; 
							sinTempDamage2[1] += OrbDamageTemp2;
						}
						sinTempDamage2[0] += ForceOrbDamage[ContinueSkill[i].Point-1];
						sinTempDamage2[1] += ForceOrbDamage[ContinueSkill[i].Point-1];
					}
				break;
				case SKILL_REGENERATION_FIELD:
					sinChar->Mana_Regen += Regeneration_Field_ManaRegen[ContinueSkill[i].Point-1]/ContinueSkill[i].Flag;
					sinChar->Life_Regen += Regeneration_Field_LifeRegen[ContinueSkill[i].Point-1];

				break;
				case SKILL_BERSERKER:
				case SKILL_BLOODY_BERSERKER:
					SkillTextColor[SIN2_ABSORB] = SIN_TEXT_COLOR_RED;
				break;
				case SCROLL_INVULNERABILITY:  //무적 스크롤 공격력 1/2
				case SCROLL_P_INVULNERABILITY:
					SkillTextColor[SIN2_DAMAGE] = SIN_TEXT_COLOR_RED;			
				break;
				case CLANSKILL_ATTACK:   //공격력 가중 15% // 박재원 - 공성전 수성 클랜 스킬 상향 조정(10% -> 15%)
					sinTempDamage2[0]  +=(int)(sinChar->Attack_Damage[0]*15)/100; 
					sinTempDamage2[1]  +=(int)(sinChar->Attack_Damage[1]*15)/100; 
					SkillTextColor[SIN2_DAMAGE] = SIN_TEXT_COLOR_GOLD;
				break;

			
			}
		}
	}


	// 장별 - 옐로우 드래곤 옵션( 캐릭터 공격력 상승)
	if( chaPremiumitem.m_HelpPetTime > 0 && sinChar->GravityScroolCheck[1] == 2 )
	{
		sinTempDamage2[0]  += (sinTempDamage2[0] * 10) / 100; 
		sinTempDamage2[1]  += (sinTempDamage2[1] * 10) / 100; 
	}

	//90LV 스킬 저주 
	if(sinQuest_Level90_2.CODE && sinQuest_Level90_2.State >= 2 ){
		sinTempDamage2[0] = (sinTempDamage2[0]*70)/100;
		sinTempDamage2[1] = (sinTempDamage2[1]*70)/100;

		sinChar->Attack_Damage[0] = (sinChar->Attack_Damage[0]*70)/100;
		sinChar->Attack_Damage[1] = (sinChar->Attack_Damage[1]*70)/100;

	}

	/////////////////////////////////////////////
	CheckRequireItem(); //착용될수 있는 아이템인지를 체크한다 

	ReformCharForm();   //재인증 

	//물약 수납공간을 체크한다 
	ReSettingPotion();	



	//디스플레이에 카피 
	if(!CheckHackFlag)
		memcpy(&sinCharDisplay , sinChar , sizeof(smCHAR_INFO));
	else //해킹방지
		HackCharStateCheck(sinChar,&sinCharDisplay);

	return TRUE;
}

//캐릭터 정보를 읽어온다 
void cINVENTORY::LoadCharState()
{

	if(!sinChar->ArrowPosi[0])sinChar->ArrowPosi[0] = 1; //인벤 AB아이템 화살표 표시값이 없으면 기본셋팅한다 
	if(!sinChar->ArrowPosi[1])sinChar->ArrowPosi[1] = 3;

	//화살표 위치 보정 
	if(sinChar->ArrowPosi[0] == 1)
		ArrowState[0] = 0;
	if(sinChar->ArrowPosi[0] == 2)
		ArrowState[0] = 1;
	if(sinChar->ArrowPosi[1] == 3)
		ArrowState[1] = 0;
	if(sinChar->ArrowPosi[1] == 4)
		ArrowState[1] = 1;

	OldLevel = sinChar->Level;
	OldLevelTemp = OldLevel+176;
	sinLoadOK = 1;
  
}

//아이템을 바꾼다 
int cINVENTORY::ChangeABItem(int Posi)
{
	SetDelayFlag = 1; //AB 체인지를 하면 딜레이 플랙을 준다 

	//if(cTrade.OpenFlag)return TRUE; //날치기 땜빵!! 
	if(NotChangeSetItemFlag)return TRUE;
	if(cMyShop.OpenFlag)return FALSE;
	if(MyShopSendButton)return FALSE;
	if(ChangeSpecialItem(1))return TRUE; //퀘스트아이템이 켜져있을경우에는 캔슬시킨다
	
	if(cWareHouse.OpenFlag){
		//수정 하대용
		//창고 아이템을 인벤토리로 옮길시에 인벤찰을 바꿀수 없다./
		if(MouseItem.Flag)
			return TRUE;  //창고가 열려있을경우에는 캔슬시킨다
	}
	
	if(sMessageBox3[MESSAGE_SELL_HIGHRANK_ITEM].Flag)return TRUE; //아이템팔기 메세지박스가있을겨우는 리턴
	if(SpecialItemShowFlag)return TRUE;
	
//	if( cSinHelp.sinShowHelp()) 
	CheckInvenItemForm();

	int desPosi;
	int i,j,t;
	//============ 텔레포트코어 무한사용을 막는다 (성근060612)
	//텔레포트핼프창이 떠있을때 인벤 AB 막음
	if(sMessageBox3[MESSAGE_TELEPORT_CORE].Flag)return TRUE;		
	for( i=0 ; i<SIN_MAX_HELP_NUMBER ; i++)
	{
		if(sSinHelp[i].KindFlag == SIN_HELP_KIND_TELEPORT_SCROLL)	
		{ return TRUE; }

	}

	// pluto 제작 AB체인지 막는다
	if( sinCraftItemStartFlag || StartInterEffectAging2 )
	{
		return TRUE;
	}
	else if( ( ManufactureItem.m_OpenFlag || SmeltingItem.OpenFlag ) && MixCancelButtonDelayFlag )
	{
		return TRUE;
	}

	//============== 아템사용후 AB체인지 막는다

	if(Posi <= 2)desPosi =1;
	if(Posi >2)desPosi = 2;

	if(Posi == 1){
		ArrowState[0] = 0;
		sinChar->ArrowPosi[0] = 1;

	}
	if(Posi == 2){
		ArrowState[0] = 1;
		sinChar->ArrowPosi[0] = 2;

	}
	if(Posi == 3){
		ArrowState[1] = 0;
		sinChar->ArrowPosi[1] = 3;

	}
	if(Posi == 4){
		ArrowState[1] = 1;
		sinChar->ArrowPosi[1] = 4;

	}

	switch(desPosi){
	case 1: //Box
		memcpy(&BackUpInvenItemTemp ,&InvenItemTemp ,sizeof(sITEM)*80); //잠시 아이템을 백업한다 
		for(t=0; t<80;t++)InvenItemTemp[t].Flag = 0;
		for(i = 0;i<80;i++){ //Temp로 복사 
			if(InvenItem[i].Flag){
				if(InvenItem[i].ItemPosition == 0){
					for(j=0;j<80;j++){
						if(!InvenItemTemp[j].Flag ){
							memcpy(&InvenItemTemp[j] , &InvenItem[i],sizeof(sITEM));
							InvenItem[i].Flag = 0;
							break;
						}
					}
				}
			}
		}
		for(i=0 ; i< 80 ; i++){ //인벤토리 아이템으로 복사 
			if(BackUpInvenItemTemp[i].Flag){
				for(j = 0;j<80;j++){ 
					if(!InvenItem[j].Flag){
						memcpy(&InvenItem[j],&BackUpInvenItemTemp[i],sizeof(sITEM));
						BackUpInvenItemTemp[i].Flag = 0;
						break;

					}

				}

			}

		}

	break;

	case 2: //무기 포지션 
		ContinueSkillCancelFlag = 1;
		memcpy(&BackUpInvenItemTemp[80] ,&InvenItemTemp[80] ,sizeof(sITEM)*2);
		for(t=80; t<82;t++)InvenItemTemp[t].Flag = 0;
		for(i = 0;i<80;i++){
			if(InvenItem[i].Flag){
				if(InvenItem[i].ItemPosition ==1 || InvenItem[i].ItemPosition ==2 ){
					for(j=80;j<82;j++){
						if(!InvenItemTemp[j].Flag ){
							memcpy(&InvenItemTemp[j] , &InvenItem[i],sizeof(sITEM));
							InvenItem[i].Flag = 0;
							sInven[InvenItem[i].ItemPosition-1].ItemIndex = 0;
							sinSetCharItem(InvenItem[i].CODE , InvenItem[i].SetModelPosi, FALSE);
							break;
						}
					}
				}
			}
		}

		for(i=80 ; i< 82 ; i++){ //인벤토리 아이템으로 복사 
			if(BackUpInvenItemTemp[i].Flag){
				for(j = 0;j<80;j++){ 
					if(!InvenItem[j].Flag){
						memcpy(&InvenItem[j],&BackUpInvenItemTemp[i],sizeof(sITEM));
						BackUpInvenItemTemp[i].Flag = 0;
						sInven[InvenItem[j].ItemPosition-1].ItemIndex = j+1;
						sinSetCharItem(InvenItem[j].CODE , InvenItem[j].SetModelPosi , TRUE); //아이템을 바꿀때 
						if(InvenItem[j].SoundIndex)
							sinPlaySound(InvenItem[j].SoundIndex);
						break;

					}

				}

			}

		}
		SetItemToChar();
	break;
	}

	//양손 아이템이 한손에 있을경우에는 지워준다 
	for( i=0;i<INVENTORY_MAXITEM ; i++){ 
		if(InvenItem[i].Flag){
			if(InvenItem[i].ItemPosition){
				//양손아이템이 한손에 있을경우에는 지워준다 
				if(InvenItem[i].Class == ITEM_CLASS_WEAPON_TWO){
					if(InvenItem[i].ItemPosition == 1 ){//오른손 
						if(InvenItem[i].sItemInfo.CODE != InvenItem[sInven[1].ItemIndex-1].sItemInfo.CODE){
							InvenItem[sInven[1].ItemIndex-1].Flag = 0;
							InvenItem[i].Flag = 0;
							sInven[1].ItemIndex = 0; //인벤토리 박스 초기화 
							sInven[0].ItemIndex = 0; //인벤토리 박스 초기화 
							if(InvenItem[i].SetModelPosi)
								sinSetCharItem(InvenItem[i].CODE , InvenItem[i].SetModelPosi, FALSE); 

						}
					}
					if(InvenItem[i].ItemPosition == 2 ){//왼손 
						if(InvenItem[i].sItemInfo.CODE != InvenItem[sInven[0].ItemIndex-1].sItemInfo.CODE){
							InvenItem[sInven[0].ItemIndex-1].Flag = 0;
							InvenItem[i].Flag = 0;
							sInven[0].ItemIndex = 0; //인벤토리 박스 초기화 
							sInven[1].ItemIndex = 0; //인벤토리 박스 초기화 
							if(InvenItem[i].SetModelPosi)
								sinSetCharItem(InvenItem[i].CODE , InvenItem[i].SetModelPosi, FALSE); 

						}
					}
				}
			}
		}
	}

	///////////'E'키로인해 없어지는 물약을 찾기위한 힘겨운 노력!!!
	if(MouseItem.Flag){
		if(MouseItem.Class == ITEM_CLASS_POTION){
			GhostPotionCheckFlag = 1;

		}
		else
			GhostPotionCheckFlag = 0;

	}
	else
		GhostPotionCheckFlag = 0;

	CheckRequireItem(); //아이템의 정보를 변경한다 (요구치 관련)
	ReFormInvenItem(); //오토 셋팅된다음에도 체크섬을 갱신한다 
	cInvenTory.CheckPuzzle(); //퍼즐 체크
	return TRUE;
}
//아이템의 가격정보를 얻어온다 
sITEMPRICE cINVENTORY::GetInvenItemPrice(sITEM *pItem)
{
	
	sITEMPRICE sinItemPrice;

	if(pItem->Class == ITEM_CLASS_WEAPON_TWO){
		if(pItem->ItemPosition){
			pItem = &InvenItem[sInven[0].ItemIndex-1];

		}
	}

	float Dur[2];		//내구력 
	float Price;		//현재 가격 

	Dur[0] = pItem->sItemInfo.Durability[0];
	Dur[1] = pItem->sItemInfo.Durability[1];
	Price  = (float)pItem->sItemInfo.Price;
	if(Dur[0] == 0)Dur[0] = 1; //값이 0이되는것을 보정한다 
	if(Dur[1] == 0)Dur[1] = 1;

	sinItemPrice.PureSellPrice = (int)((Price*(Dur[0]/Dur[1]))+(Price-(Price*(Dur[0]/Dur[1])))*0.25);
	
	sinItemPrice.SellPrice =  (int)sinItemPrice.PureSellPrice/5;  

	sinItemPrice.RepairCost = (pItem->sItemInfo.Price -sinItemPrice.PureSellPrice)/8; 
/*
	//수표 Kyle

	if( pItem->sItemInfo.CODE == (sinBI1|sin35)	||
		pItem->sItemInfo.CODE == (sinBI1|sin36)	||
		pItem->sItemInfo.CODE == (sinBI1|sin37)	||
		pItem->sItemInfo.CODE == (sinBI1|sin38)
		)
	{
		sinItemPrice.PureSellPrice	= 0;
		sinItemPrice.SellPrice		= (int)Price;
		sinItemPrice.RepairCost		= 0; 
	}
	else
	{
		sinItemPrice.PureSellPrice = (int)((Price*(Dur[0]/Dur[1]))+(Price-(Price*(Dur[0]/Dur[1])))*0.25);
		sinItemPrice.SellPrice =  (int)sinItemPrice.PureSellPrice/5;  
		sinItemPrice.RepairCost = (pItem->sItemInfo.Price -sinItemPrice.PureSellPrice)/8; 
	}
*/
	return sinItemPrice; //구조체를 리턴한다 

}


//아이템을 수리한다 
int cINVENTORY::RepairInvenItem(sITEM *pItem , int RepairCost) 
{
	if(pItem->Class == ITEM_CLASS_WEAPON_TWO){
		if(pItem->ItemPosition){
			pItem = &InvenItem[sInven[0].ItemIndex-1];

		}
	}

	CheckCharForm();//인증 
	//ReformCharForm();//재인증
	if(!pItem->sItemInfo.Durability[1])return FALSE;
	if(sinChar->Money - RepairCost > 0){
		if(pItem->sItemInfo.Durability[0] < pItem->sItemInfo.Durability[1]){
			//sinChar->Money -= RepairCost;
			sinMinusMoney(RepairCost);
			pItem->sItemInfo.Durability[0] = pItem->sItemInfo.Durability[1];
			ReformCharForm();//재인증 
			SendSaveMoney(); //금액 조작을 못하게하기위해 호출한다 
			return TRUE;
		}
	}
	return FALSE;
	
}

//무게를 체크한다 
void cINVENTORY::CheckWeight()
{

	CheckCharForm();//인증
	
	sinChar->Weight[0] = 0; //무게 초기화 후 다시 셋팅 
	for(int i=0; i <INVENTORY_MAXITEM ; i++){
		if(InvenItem[i].Flag){
			if(InvenItem[i].Class == ITEM_CLASS_POTION){
				if( InvenItem[i].sItemInfo.PotionCount < 0 )continue;
				sinChar->Weight[0] += InvenItem[i].sItemInfo.PotionCount;
				continue;

			}
			if(InvenItem[i].sItemInfo.Weight < 0)continue;
			sinChar->Weight[0] += InvenItem[i].sItemInfo.Weight;
		}
	}
	for(int j=0; j <INVENTORY_MAXITEM ; j++){
		if(InvenItemTemp[j].Flag){
			if(InvenItemTemp[j].Class == ITEM_CLASS_POTION){
				if( InvenItemTemp[j].sItemInfo.PotionCount < 0 )continue;
				sinChar->Weight[0] += InvenItemTemp[j].sItemInfo.PotionCount;
				continue;

			}
			if(InvenItemTemp[j].sItemInfo.Weight < 0)continue;
			sinChar->Weight[0] += InvenItemTemp[j].sItemInfo.Weight;
		}
	}

	ReformCharForm();//재인증 

}

// 셋팅할수있는지를 체크한다 
int cINVENTORY::CheckSetOk(sITEM *pItem , int AutoFlag)
{
	if(pItem->Class == ITEM_CLASS_POTION){
		if(sinChar->Weight[0]+pItem->sItemInfo.PotionCount > sinChar->Weight[1]){ //무게를 넘어서면 
			cMessageBox.ShowMessage(MESSAGE_OVER_WEIGHT);
			sinOverWeightCnt++;
			return FALSE;
		}

	}
	else{
		if(sinChar->Weight[0]+pItem->sItemInfo.Weight > sinChar->Weight[1]){ //무게를 넘어서면 
			if(pItem->sItemInfo.ItemKindCode != ITEM_KIND_QUEST_WEAPON){
				cMessageBox.ShowMessage(MESSAGE_OVER_WEIGHT);
				sinOverWeightCnt++;
				return FALSE;
			}
		}
	}
	if(!AutoFlag){ //오토셋팅이 아닐 경우에만 체크한다 
		if(pItem->ItemPosition){
			if(pItem->sItemInfo.NotUseFlag){
				cMessageBox.ShowMessage(MESSAGE_NO_USE_ITEM);
				return FALSE; //메세지만 출력해준다 창고로 오토셋팅은 된다 
			}
		}
	}
	return TRUE;
}


//복사된 아이템을 없앤다  
int cINVENTORY::CopyItemClear(sITEM *pItem)
{ //마우스에 들려있는 아이템과 같은 아이템이 있는지를 검사한다

	int CopyItemCnt = 0;
	for(int i = 0 ; i < INVENTORY_MAXITEM ; i++){ 
		if(InvenItem[i].Flag){		//앞면 인벤토리 
			if(CompareItems(&pItem->sItemInfo ,  &InvenItem[i].sItemInfo)){
				//복사된 아이템을 지워준다 
				InvenItem[i].Flag = 0;
				if(InvenItem[i].ItemPosition){
					sInven[InvenItem[i].ItemPosition-1].ItemIndex = 0;
					sinSetCharItem(InvenItem[i].CODE , InvenItem[i].SetModelPosi, FALSE); //무기와 갑옷을 셋팅한다 
					ClearTwoHandPosiAndItem(&InvenItem[i]);//투핸드 무기이면 투핸드 무기를 지워준다 

				}
				CopyItemCnt++;
				

			}

		}
		if(InvenItemTemp[i].Flag){	//뒷면 인벤토리 
			if(CompareItems(&pItem->sItemInfo ,  &InvenItemTemp[i].sItemInfo)){//복사된 아이템을 지워준다 
				InvenItemTemp[i].Flag = 0;
				if(InvenItemTemp[i].Class == ITEM_CLASS_WEAPON_TWO){
					if(InvenItemTemp[i].ItemPosition == 1 ||InvenItemTemp[i].ItemPosition == 2){
						for(int k = 0 ; k < INVENTORY_MAXITEM ; k++){ 
							if(InvenItemTemp[k].Flag){
								if(InvenItemTemp[k].ItemPosition == 1 ||InvenItemTemp[k].ItemPosition == 2){
									if(InvenItemTemp[i].sItemInfo.CODE == InvenItemTemp[k].sItemInfo.CODE){
										InvenItemTemp[k].Flag = 0;		
									}
								}
							}
						}
					}
				}
				CopyItemCnt++;
			}
		}
	}
	if(CopyItemCnt){
		SendCopyItemUser(CopyItemCnt); //서버에 카피된 아이템의 갯수를 보낸다 
		CopyItemCnt = 0;
	}
	return TRUE;	

}

POINT ResultSize;
//그림의 사이즈를 구해서 리턴한다 
POINT cINVENTORY::CheckImageSize(LPDIRECTDRAWSURFACE4 lpdd)
{
	ResultSize.x = 0; //초기화 
	ResultSize.y = 0;

	DDSURFACEDESC2	ddTemp;
	memset(&ddTemp,0,sizeof(DDSURFACEDESC2));
	ddTemp.dwSize = sizeof(ddTemp);
	lpdd->GetSurfaceDesc(&ddTemp);

	ResultSize.y = ddTemp.dwHeight;
	ResultSize.x = ddTemp.dwWidth;

	return ResultSize;
}

//임시로 아이템 이미지를 체크하기위해서 쓰인다 
int cINVENTORY::ImsiCheckItemImageSize()
{
	sITEM TestItem;
	POINT TestPoint = {0,0};
	char szFilePath[256];
	sinTestBuff[0] = 0;
	for(int j=0 ; j < MAX_ITEM ; j++){
		wsprintf(szFilePath ,"Image\\sinImage\\Items\\%s\\it%s.bmp",sItem[j].ItemFilePath,sItem[j].LastCategory);
		memcpy(&TestItem ,&sItem[j],sizeof(sITEM));  //구조체에 정의된 정보를 저장한다 
		TestItem.lpItem = LoadDibSurfaceOffscreen(szFilePath);
		TestPoint.x = 0;
		TestPoint.y = 0;
		if(TestItem.lpItem){
			TestPoint = CheckImageSize(TestItem.lpItem);
			if(TestPoint.x != sItem[j].w || TestPoint.y != sItem[j].h){
				lstrcpy(sinTestBuff,sItem[j].LastCategory);
			}

		}

	}
	return TRUE;
}


//잘못된 좌표를 보정해준다 
int cINVENTORY::CheckOverlapItem(sITEM *pItem , int PassItemIndex)
{
	int ReSetXYFlag =0 ;
	if(!pItem)return FALSE;
	if(pItem->ItemPosition){
		if(pItem->ItemPosition < 11){ //물약 포지션이 아니면 
			sInven[pItem->ItemPosition-1].ItemIndex = 0; //포지션 초기화 
			pItem->ItemPosition = 0;
			ReSetXYFlag = 1;
		}
		else 
			ReSetXYFlag = 0;
	}

	RECT Rect;
	int  TempPosiFlag = 0;
	Rect.left   = pItem->x; 
	Rect.top    = pItem->y;
	Rect.right  = pItem->w;
	Rect.bottom = pItem->h;

	int cntx,cnty;
	int cx,cy;

	RECT	rect;
	int		flag;

	cx = (22*INVENTORY_BOX_ROW)-pItem->w; //아이템이 들어갈수있는 가장 끝 X좌표 
	cy = (22*INVENTORY_BOX_COL)-pItem->h; //아이템이 들어갈수있는 가장 끝 Y좌표

	//코드가 같지 않거나 아이템이 비어있지 않은 곳에 셋팅할때 검색해서 위치를 찾아낸다 

	if(CrashInvenItem(Rect,PassItemIndex)||ReSetXYFlag){
		for( cntx=StartX ; cntx <= StartX+cx ; cntx+=22){
			for( cnty=StartY ; cnty <= StartY+cy ; cnty+=22){ 
				flag = 0;
				for(int i=0 ; i<INVENTORY_MAXITEM ; i++){
					if(InvenItem[i].Flag){
						rect.left   = InvenItem[i].x;
						rect.right  = InvenItem[i].x+InvenItem[i].w;
						rect.top    = InvenItem[i].y;
						rect.bottom = InvenItem[i].y+InvenItem[i].h;
	
						if( cntx > rect.left-pItem->w && cntx< rect.right && 
							cnty > rect.top-pItem->h && cnty< rect.bottom )
							flag++;
					}
				}
				if ( flag==0 ) {
					pItem->x = cntx;
					pItem->y = cnty;
					sInven[pItem->ItemPosition-1].ItemIndex =0;
					pItem->ItemPosition = 0; 
					return TRUE;
				}
			}
		}
	}
	return FALSE;
}

//임시로 잘못된 좌표를 보정해준다 
int cINVENTORY::ReSetPotionXY(sITEM *pItem,int kind)
{

	RECT Rect;
	Rect.left   = pItem->x; 
	Rect.top    = pItem->y;
	Rect.right  = pItem->w;
	Rect.bottom = pItem->h;

	int cntx,cnty;
	int cx,cy;

	RECT	rect;
	int		flag;

	cx = (22*INVENTORY_BOX_ROW)-pItem->w; //아이템이 들어갈수있는 가장 끝 X좌표 
	cy = (22*INVENTORY_BOX_COL)-pItem->h; //아이템이 들어갈수있는 가장 끝 Y좌표

	//코드가 같지 않거나 아이템이 비어있지 않은 곳에 셋팅할때 검색해서 위치를 찾아낸다 

	if(kind == 1){
		for( cntx=StartX ; cntx <= StartX+cx ; cntx+=22){
			for( cnty=StartY ; cnty <= StartY+cy ; cnty+=22){ 
				flag = 0;
				int i; for( i =0 ; i<INVENTORY_MAXITEM ; i++){
					if(InvenItemTemp[i].Flag){
						rect.left   = InvenItemTemp[i].x;
						rect.right  = InvenItemTemp[i].x+InvenItemTemp[i].w;
						rect.top    = InvenItemTemp[i].y;
						rect.bottom = InvenItemTemp[i].y+InvenItemTemp[i].h;
		
						if( cntx > rect.left-pItem->w && cntx< rect.right && 
							cnty > rect.top-pItem->h && cnty< rect.bottom )
							flag++;
					}
				}
				if ( flag==0 ) {
					pItem->x = cntx;
					pItem->y = cnty;
					//sInven[pItem->ItemPosition-2].ItemIndex =0; //요기는 템프아이템이라서 걍 냅둔다 
					pItem->ItemPosition = 0; 
					return TRUE;
				}

			}
		}
	}
	else{
		for( cntx=StartX ; cntx <= StartX+cx ; cntx+=22){
			for( cnty=StartY ; cnty <= StartY+cy ; cnty+=22){ 
				flag = 0;
				int i; for( i =0 ; i<INVENTORY_MAXITEM ; i++){
					if(InvenItem[i].Flag){
						rect.left   = InvenItem[i].x;
						rect.right  = InvenItem[i].x+InvenItem[i].w;
						rect.top    = InvenItem[i].y;
						rect.bottom = InvenItem[i].y+InvenItem[i].h;
		
						if( cntx > rect.left-pItem->w && cntx< rect.right && 
							cnty > rect.top-pItem->h && cnty< rect.bottom )
							flag++;
					}
				}
				if ( flag==0 ) {
					pItem->x = cntx;
					pItem->y = cnty;
					sInven[pItem->ItemPosition-2].ItemIndex =0;
					pItem->ItemPosition = 0; 
					return TRUE;
				}

			}
		}
	}
	return FALSE;

}

//귀환 아이템을 사용한다 
int cINVENTORY::UseEcore(sITEM *pItem)
{
	char szLinkName[24]; 
	int len = lstrlen(pItem->sItemInfo.ItemName); 
	if(sinItemPickUpDelayFlag)return FALSE; //무한에테르 코어를 막는다 
	// pluto
	if(  pItem->sItemInfo.CODE == (sinEC1|sin01) && StageField[lpCurPlayer->OnStageField]->State==FIELD_STATE_ROOM )		// 에테르 코어 사라지는거 막는거
	{
		cMessageBox.ShowMessage(MESSAGE_NO_USE_ITEM); // 텔레포트 불가
		return FALSE;
	}
	//링크 코어일경우 서버로 아이템을 보낸다
	if(pItem->sItemInfo.CODE == (sinEC1|sin05)){
		if(sinChar->ClassClan){
			//if(cldata.myPosition == 100 || cldata.myPosition == 101 ){
			if(cldata.stats[0] == '1'){ //정식클랜원일 경우에만 코어를 사용할수있다 
				if(pItem->sItemInfo.ItemName[len+1] == NULL){ //링크코어등록 
					//cInvenTory.UnionCoreParticle2(pItem->x , pItem->y); //이펙트 
					//sinPlaySound(SIN_SOUND_EAT_POTION2); //유니온코어 사운드
					UnionCorePosi.x = pItem->x;
					UnionCorePosi.y = pItem->y;
					memset(&TempLinkCoreItem,0,sizeof(sITEMINFO));
					memcpy(&TempLinkCoreItem,&pItem->sItemInfo,sizeof(sITEMINFO));
					cMessageBox.ShowMessage3(MESSAGE_LINKCORE,"");
					
				}
				else{
					lstrcpy(szLinkName,&pItem->sItemInfo.ItemName[len+1]);//자기자신은 링크코어를 사용할수엄따
					if(stricmp(sinChar->szName,szLinkName) != 0){
						//무게는 어서 줄어주남 -_-?
						SendLinkCoreToServer(&pItem->sItemInfo);
					}
				}
			}
			else{
				cMessageBox.ShowMessage(MESSAGE_USE_REGULAR_CLAN_ONLY);
			}
		}
		else{
			cMessageBox.ShowMessage(MESSAGE_USE_CLAN_ONLY);
		}
	}
	else{
		CheckInvenItemForm();
		ActionEtherCore(pItem);
		pItem->Flag = 0;
		CheckWeight();//무게 보정 
		ReFormInvenItem();
	}
	sinUsePotionDelayFlag = 1;
	return TRUE;
}

//송편을 먹는다 
int cINVENTORY::EatSongPyeun(sITEM *pItem)
{
	// pluto 선물상자는 리턴
	if( (pItem->CODE & sinITEM_MASK3) == sin05 || (pItem->CODE & sinITEM_MASK3) == sin06 || (pItem->CODE & sinITEM_MASK3) == sin07 ||
		(pItem->CODE & sinITEM_MASK3) == sin08 || (pItem->CODE & sinITEM_MASK3) == sin09 || (pItem->CODE & sinITEM_MASK3) == sin10 )
	{
		ReFormInvenItem();
		sinUsePotionDelayFlag = 1;
		return TRUE;
	}
	char szBuff[128];

	//////////////유통기한이 지난 송편을 먹었을때 
	if ( cItem.GetItemLimitTime( pItem )==FALSE) {
	//if(pItem->LimitTimeFlag){
		//cMessageBox.ShowMessage(MESSAGE_DEAD_SONGPYUEN);
		wsprintf(szBuff,"%s : %s",sinChar->szName, sinDeadSongPyeunEat);
		SendChatMessageToServer(szBuff);
		ReFormInvenItem();
		sinUsePotionDelayFlag = 1;
		return FALSE;
	}
	int TempRandPotion = 0;
	int ResultPotion = 0;
	int Life=0 , Mana=0 , Stamina=0;

	//CheckInvenItemForm();

	TempRandPotion = pItem->sItemInfo.Life[1] - pItem->sItemInfo.Life[0];
	ResultPotion   = pItem->sItemInfo.Life[0]+(rand()%TempRandPotion);
	sinSetLife((sinGetLife()+ResultPotion));		//라이프 셋팅 
	Life = (sinGetLife()+ResultPotion);

	TempRandPotion = pItem->sItemInfo.Mana[1] - pItem->sItemInfo.Mana[0];
	ResultPotion   = pItem->sItemInfo.Mana[0]+(rand()%TempRandPotion);
	sinSetMana((sinGetMana()+ResultPotion));		//마나 셋팅
	Mana = (sinGetLife()+ResultPotion);

	TempRandPotion = pItem->sItemInfo.Stamina[1] - pItem->sItemInfo.Stamina[0];
	ResultPotion   = pItem->sItemInfo.Stamina[0]+(rand()%TempRandPotion);
	sinSetStamina((sinGetStamina()+ResultPotion));	//스테미나 셋팅
	Stamina = (sinGetLife()+ResultPotion);

	smTRANS_COMMAND	smTransCommand;

	smTransCommand.size = sizeof(smTRANS_COMMAND);
	smTransCommand.code = smTRANSCODE_PARTY_POTION;
	smTransCommand.WParam =	Life;	//생명력
	smTransCommand.LParam =	Mana;	//기력
	smTransCommand.SParam =	Stamina;//근력
	smTransCommand.EParam = lpCurPlayer->dwObjectSerial;
	
	if ( smWsockUserServer && InterfaceParty.PartyMemberCount )
		smWsockUserServer->Send2( (char *)&smTransCommand , smTransCommand.size , TRUE );


	ReFormInvenItem();
	sinUsePotionDelayFlag = 1;


	return TRUE;

}
//마우스에 있는 아이템을 리턴한다 
int cINVENTORY::ResetMouseItem()
{
	if(MouseItem.Flag){
		if(cInvenTory.AutoSetInvenItem(&MouseItem)){
			MouseItem.Flag = 0;
		}
	}

	return TRUE;
}	
//CheckInvenItemForm();
//ReFormInvenItem();
//////////////////////인벤토리 아이템의 조작을 방지한다 
int cINVENTORY::CheckInvenItemForm()
{ //데이타의 값을 더해서 변수에 넣어준다 
	DWORD TempCheckDataSum = 0;

	int i; for( i =0 ; i<INVENTORY_MAXITEM ; i++){
		if(InvenItem[i].Flag){
			if(InvenItem[i].Class != ITEM_CLASS_POTION){
				//TempCheckDataSum += (i+1)*InvenItem[i].x;
				TempCheckDataSum += (i+1)*InvenItem[i].y;
				TempCheckDataSum += (i+1)*InvenItem[i].Class;
				TempCheckDataSum += InvenItem[i].sItemInfo.ItemHeader.dwChkSum;
				TempCheckDataSum += InvenItem[i].sItemInfo.ItemHeader.Head;
			}

		}
		if(InvenItemTemp[i].Flag){
			if(InvenItemTemp[i].Class != ITEM_CLASS_POTION){
				//TempCheckDataSum += (i+1)*InvenItemTemp[i].x;
				TempCheckDataSum += (i+1)*InvenItemTemp[i].y;
				TempCheckDataSum += (i+1)*InvenItemTemp[i].Class;
				TempCheckDataSum += InvenItemTemp[i].sItemInfo.ItemHeader.dwChkSum;
				TempCheckDataSum += InvenItemTemp[i].sItemInfo.ItemHeader.Head;
			}

		}

	}
	if(TempCheckDataSum != InvenDataCheckSum){
		SendSetHackUser(2); //해킹을 하려고했던 못된 유저를 고발 TRUE 접속 종료 
		//TempCheckDataSum = 0;

	}
	return TRUE;
}

//데이타의 값을 셋팅한다 
int cINVENTORY::ReFormInvenItem()
{
	DWORD Temp = 0;
	InvenDataCheckSum = 0;

	int i; for( i =0 ; i<INVENTORY_MAXITEM ; i++){
		if(InvenItem[i].Flag){
			if(InvenItem[i].Class != ITEM_CLASS_POTION){
				//InvenDataCheckSum += (i+1)*InvenItem[i].x;
				InvenDataCheckSum += (i+1)*InvenItem[i].y;
				InvenDataCheckSum += (i+1)*InvenItem[i].Class;
				InvenDataCheckSum += InvenItem[i].sItemInfo.ItemHeader.dwChkSum;
				InvenDataCheckSum += InvenItem[i].sItemInfo.ItemHeader.Head;
			}

		}
		if(InvenItemTemp[i].Flag){
			if(InvenItemTemp[i].Class != ITEM_CLASS_POTION){
				//InvenDataCheckSum += (i+1)*InvenItemTemp[i].x;
				InvenDataCheckSum += (i+1)*InvenItemTemp[i].y;
				InvenDataCheckSum += (i+1)*InvenItemTemp[i].Class;
				InvenDataCheckSum += InvenItemTemp[i].sItemInfo.ItemHeader.dwChkSum;
				InvenDataCheckSum += InvenItemTemp[i].sItemInfo.ItemHeader.Head;
			}

		}

	}
	Temp = InvenDataCheckSum;
	
	return TRUE;

}

//돈을 셋팅할수있는지를 체크한다 
int cINVENTORY::CheckMoneyLimit(int Money)
{

	int CheckMoney = 0;  
	if(sinChar->Level <= 10){
		if((sinChar->Money + Money) <= 200000 )
			return TRUE;

	}
	else{
		switch(sinChar->ChangeJob){
			case 0:
				if((sinChar->Money + Money) <= (sinChar->Level*200000)-1800000) 
					return TRUE;
			break;

			case 1:
				if( ( sinChar->Money + Money) <= 10000000 )return TRUE;
			break;
			case 2:
				if( ( sinChar->Money + Money) <= 50000000 )return TRUE;

			break;
			//4차 전업(임시)
			case 3:
			case 4:		// tier 5: the fifth rank (without this, a rank 5 character could pick up no gold at all)
			default:
				if( ( sinChar->Money + Money) <= 100000000 )return TRUE;
			break;

		}
	}

	cMessageBox.ShowMessage(MESSAGE_OVER_MONEY);
	return FALSE;

/*
	int CheckMoney = 0;  
	if(sinChar->Level <= 20){
		if((sinChar->Money + Money) <= 200000 )
			return TRUE;

	}
	else{
		if((sinChar->Money + Money) <= (sinChar->Level*200000)-1800000) 
			return TRUE;

	}
	
	cMessageBox.ShowMessage(MESSAGE_OVER_MONEY);
	return FALSE;
*/

}

//////포션의 갯수를 더한다 
int sinPotionNum[4] = {0,0,0,0};
int CheckPotionNum[4] = { 0,0,0,0};
int cINVENTORY::ReFormPotionNum()
{
	PotionCheckNumJumpFlag =1;
	
	for(int j=0;j<4;j++){
		sinPotionNum[j] = 0;
	
	}	
	int i; for( i =0;i<INVENTORY_MAXITEM ; i++){ 
		if(InvenItem[i].Flag){
			if(InvenItem[i].Class  == ITEM_CLASS_POTION){
				switch(InvenItem[i].CODE & sinITEM_MASK3){
				case sin01:
					sinPotionNum[0] += InvenItem[i].sItemInfo.PotionCount;

				break;
				case sin02:
					sinPotionNum[1] += InvenItem[i].sItemInfo.PotionCount;

				break;
				case sin03:
					sinPotionNum[2] += InvenItem[i].sItemInfo.PotionCount;

				break;
				case sin04:
					sinPotionNum[3] += InvenItem[i].sItemInfo.PotionCount;

				break;
				}
			}
		}
		if(InvenItemTemp[i].Flag){
			if(InvenItemTemp[i].Class  == ITEM_CLASS_POTION){
				switch(InvenItemTemp[i].CODE & sinITEM_MASK3){
				case sin01:
					sinPotionNum[0] += InvenItemTemp[i].sItemInfo.PotionCount;

				break;
				case sin02:
					sinPotionNum[1] += InvenItemTemp[i].sItemInfo.PotionCount;

				break;
				case sin03:
					sinPotionNum[2] += InvenItemTemp[i].sItemInfo.PotionCount;

				break;
				case sin04:
					sinPotionNum[3] += InvenItemTemp[i].sItemInfo.PotionCount;

				break;
				}
			}
		}
	}
	/*
	if(MouseItem.Flag){
		if(MouseItem.Class  == ITEM_CLASS_POTION){
			switch(MouseItem.sItemInfo.CODE & sinITEM_MASK3){
				case sin01:
					sinPotionNum[0] += MouseItem.sItemInfo.PotionCount;

				break;
				case sin02:
					sinPotionNum[1] += MouseItem.sItemInfo.PotionCount;

				break;
				case sin03:
					sinPotionNum[2] += MouseItem.sItemInfo.PotionCount;

				break;
				case sin04:
					sinPotionNum[3] += MouseItem.sItemInfo.PotionCount;

				break;
			}
		}
	}
	*/
	PotionCheckNumJumpFlag = 0;
	return TRUE;
}

//포션을 체크한다 

DWORD PotionCheckDelayTime = 0;
int cINVENTORY::CheckPotionNumForm()
{
/*	
	PotionCheckDelayTime++;
	if(PotionCheckDelayTime < 70*10)return FALSE;
	PotionCheckDelayTime = 0;
*/	
	
	for(int k=0;k<4;k++){
		CheckPotionNum[k] = 0;
	
	}	
	int i; for( i =0;i<INVENTORY_MAXITEM ; i++){ 
		if(InvenItem[i].Flag){
			if(InvenItem[i].Class  == ITEM_CLASS_POTION){
				switch(InvenItem[i].CODE & sinITEM_MASK3){
				case sin01:
					CheckPotionNum[0] += InvenItem[i].sItemInfo.PotionCount;

				break;
				case sin02:
					CheckPotionNum[1] += InvenItem[i].sItemInfo.PotionCount;

				break;
				case sin03:
					CheckPotionNum[2] += InvenItem[i].sItemInfo.PotionCount;

				break;
				case sin04:
					CheckPotionNum[3] += InvenItem[i].sItemInfo.PotionCount;

				break;
				}
			}
		}
		if(InvenItemTemp[i].Flag){
			if(InvenItemTemp[i].Class  == ITEM_CLASS_POTION){
				switch(InvenItemTemp[i].CODE & sinITEM_MASK3){
				case sin01:
					CheckPotionNum[0] += InvenItemTemp[i].sItemInfo.PotionCount;

				break;
				case sin02:
					CheckPotionNum[1] += InvenItemTemp[i].sItemInfo.PotionCount;

				break;
				case sin03:
					CheckPotionNum[2] += InvenItemTemp[i].sItemInfo.PotionCount;

				break;
				case sin04:
					CheckPotionNum[3] += InvenItemTemp[i].sItemInfo.PotionCount;

				break;
				}
			}
		}
	}
	/*
	if(MouseItem.Flag){
		if(MouseItem.Class  == ITEM_CLASS_POTION){
			switch(MouseItem.sItemInfo.CODE & sinITEM_MASK3){
				case sin01:
					CheckPotionNum[0] += MouseItem.sItemInfo.PotionCount;

				break;
				case sin02:
					CheckPotionNum[1] += MouseItem.sItemInfo.PotionCount;

				break;
				case sin03:
					CheckPotionNum[2] += MouseItem.sItemInfo.PotionCount;

				break;
				case sin04:
					CheckPotionNum[3] += MouseItem.sItemInfo.PotionCount;

				break;
			}
		}
	}
	*/

	for(int j=0;j<4;j++){
		if(CheckPotionNum[j] !=sinPotionNum[j]){
			//SendSetHackUser(TRUE); //해킹을 하려고했던 못된 유저를 고발 TRUE 접속 종료 
			SendSetHackUser2(1510,j); //해킹한 유저를 신고한다
		}
	}
	return TRUE;

}


//////////////// 각종 특별한 아이템을 체크한다 
int cINVENTORY::CheckSpecialItem(sITEM *pItem)
{
	//////퀘스트용 아이템일경우 
	if(pItem->sItemInfo.ItemKindCode == ITEM_KIND_QUEST || pItem->sItemInfo.UniqueItem)
		return FALSE;


	return TRUE;
}


/*----------------------------------------------------------------------------*
*						인벤토리 이펙트 
*-----------------------------------------------------------------------------*/	
//인벤토리 파티클을 보여준다 
int dwInvenParticleShowTime = 0;
int cINVENTORY::ShowInvenEffectParticle()
{
	dwInvenParticleShowTime++; 
	if(dwInvenParticleShowTime <= 5)return FALSE;
	dwInvenParticleShowTime = 0;

	int TempX,TempY;
	
	if(MouseItem.Flag){
		if(MouseItem.sItemInfo.ItemKindCode == ITEM_KIND_QUEST || MouseItem.sItemInfo.UniqueItem){
			TempX = rand()%MouseItem.w;
			TempY = rand()%MouseItem.h;
			SetInvenParticle(MouseItem.x+TempX,MouseItem.y+TempY,SIN_EFFECT_SHOW_INVEN);

		}
	}

	int i; for( i = 0 ; i < INVENTORY_MAXITEM ; i++){  
		if(InvenItem[i].Flag){
			//////// 경품권 이펙트를 살자쿵 넣어봄다 (일단 뺌다)
			/*
			if((InvenItem[i].sItemInfo.CODE & sinITEM_MASK2 ) == sinGP1 ){
				TempX = rand()%InvenItem[i].w;
				TempY = rand()%InvenItem[i].h;
				SetInvenParticle(InvenItem[i].x+TempX,InvenItem[i].y+TempY,SIN_EFFECT_SHOW_GYUNGFUM);

			}
			*/

			if(InvenItem[i].sItemInfo.ItemKindCode == ITEM_KIND_QUEST || InvenItem[i].sItemInfo.UniqueItem){
				TempX = rand()%InvenItem[i].w;
				TempY = rand()%InvenItem[i].h;
				SetInvenParticle(InvenItem[i].x+TempX,InvenItem[i].y+TempY,SIN_EFFECT_SHOW_INVEN);

				if(InvenItem[i].Class == ITEM_CLASS_WEAPON_TWO){
					if(i == sInven[0].ItemIndex-1 ){ //양손무기에서두 이펙트가 나오게 살짝 땜빵을 해봅니다
						TempX = rand()%(sInven[1].Rect.right-10 - sInven[1].Rect.left+10);
						TempY = rand()%(sInven[1].Rect.bottom-10 - sInven[1].Rect.top+10);
						SetInvenParticle(sInven[1].Rect.left+TempX,sInven[1].Rect.top+TempY,SIN_EFFECT_SHOW_INVEN);
					}

				}

			}
		}
	}
	
	//트레이드 창에서 이펙트 보이기 
	for(i = 0 ; i < MAX_TRADE_ITEM ; i++){
		if(sTrade.TradeItem[i].Flag){
			if(sTrade.TradeItem[i].sItemInfo.UniqueItem){
				TempX = rand()%sTrade.TradeItem[i].w;
				TempY = rand()%sTrade.TradeItem[i].h;
				SetInvenParticle(sTrade.TradeItem[i].x+TempX,sTrade.TradeItem[i].y+TempY,SIN_EFFECT_SHOW_TRADE);

			}
		}
		if(sTradeRecv.TradeItem[i].Flag){
			if(sTradeRecv.TradeItem[i].sItemInfo.UniqueItem){
				TempX = rand()%sTradeRecv.TradeItem[i].w;
				TempY = rand()%sTradeRecv.TradeItem[i].h;
				SetInvenParticle(sTradeRecv.TradeItem[i].x+TempX,sTradeRecv.TradeItem[i].y+TempY,SIN_EFFECT_SHOW_TRADE);

			}
		}

	}

	//창고아이템에서 이펙트 보이기
	for(i=0 ; i < 100 ; i++){
		if(sWareHouse.WareHouseItem[i].Flag){
			if(sWareHouse.WareHouseItem[i].sItemInfo.UniqueItem){
				TempX = rand()%sWareHouse.WareHouseItem[i].w;
				TempY = rand()%sWareHouse.WareHouseItem[i].h;
				SetInvenParticle(sWareHouse.WareHouseItem[i].x+TempX,sWareHouse.WareHouseItem[i].y+TempY,SIN_EFFECT_SHOW_WAREHOUSE);
			}
		}
	}
	return TRUE;
}

//이펙트를 셋팅한다 
int cINVENTORY::SetInvenParticle(int x , int y , int Kind)
{
	
	int i , k;
	int TempSize;
	int Count=0; 

	for(i = 0 ; i < MAX_INVEN_PARTICLE ; i++){     
		if(!sInven_Effect[i].Time){
			sInven_Effect[i].Kind = Kind;
			sInven_Effect[i].Time = 1; 
			sInven_Effect[i].Max_Time = rand()%20 +3;

			sInven_Effect[i].InvenFace.x = x;
			sInven_Effect[i].InvenFace.y = y;
			TempSize = rand()%13+1;
			sInven_Effect[i].InvenFace.width  = TempSize; 
			sInven_Effect[i].InvenFace.height = TempSize;

			//sInven_Effect[i].Speed.x = rand()%3+1;
			sInven_Effect[i].Speed.y = rand()%12+3;

			if(Kind == SIN_EFFECT_SHOW_GYUNGFUM){ //요건 경품권  
				sInven_Effect[i].InvenFace.MatNum =	MatInvenParticle[4]; 
	//			sInven_Effect[i].InvenFace.r = 0;//rand()%255;
	//			sInven_Effect[i].InvenFace.g = rand()%255;
	//			sInven_Effect[i].InvenFace.b = rand()%255;
				sInven_Effect[i].InvenFace.r = 255;
				sInven_Effect[i].InvenFace.g = 255;
				sInven_Effect[i].InvenFace.b = 255;
				sInven_Effect[i].InvenFace.Transparency = rand()%55 + 200; 
				sInven_Effect[i].FadeType = 1;
				if(sInven_Effect[i].InvenFace.width > 5 ){
					sInven_Effect[i].InvenFace.width -= 2;
					sInven_Effect[i].InvenFace.height -= 2;

				}
				if(sInven_Effect[i].InvenFace.width < 5 ){
					memset(&sInven_Effect[i],0,sizeof(sINVENTORY_EFFECT));  //스페셜 왕 날치기 -_-;;

				}

			}
			else{
				sInven_Effect[i].InvenFace.MatNum =	MatInvenParticle[0];
	//			sInven_Effect[i].InvenFace.r = 0;//rand()%255;
	//			sInven_Effect[i].InvenFace.g = rand()%255;
	//			sInven_Effect[i].InvenFace.b = rand()%255;
				sInven_Effect[i].InvenFace.r = 255;
				sInven_Effect[i].InvenFace.g = 255;
				sInven_Effect[i].InvenFace.b = 255;
				sInven_Effect[i].InvenFace.Transparency = rand()%55 + 200; 
				sInven_Effect[i].FadeType = 1;
			}
			///////날치기 복사(여러개 겹치면 밝아진다나 음...) 
			if(Kind == 1){ //인벤토리
				for(k = 0 ; k < MAX_INVEN_PARTICLE ; k++){    
					if(!sInven_Effect[k].Time){
						Count++;
						if(Count > 2){
							Count = 0;
							break;

						}
						memcpy(&sInven_Effect[k],&sInven_Effect[i],sizeof(sINVENTORY_EFFECT));
					}
					
				}
			}
			break;
		}

	}
	
	return TRUE;
}

DWORD dwInvenParticleMoveTime =0;
//인벤토리 이펙트를 움직인다
int cINVENTORY::MoveInvenParticle() //앞으로 절때 건들지 말것!
{
	dwInvenParticleMoveTime++;
	if(dwInvenParticleMoveTime <= 5)return FALSE;
	dwInvenParticleMoveTime = 0;

	int i;
	for(i = 0 ; i < MAX_INVEN_PARTICLE ; i++){    
		if(sInven_Effect[i].Time){ 
			sInven_Effect[i].Time++;
			//if(sInven_Effect[i].Time >= sInven_Effect[i].Max_Time)sInven_Effect[i].Time = 0;
			sInven_Effect[i].InvenFace.y -= sInven_Effect[i].Speed.y;

			if(sInven_Effect[i].FadeType == 1) 
				sInven_Effect[i].InvenFace.Transparency += 14;
			if(sInven_Effect[i].FadeType == 2)
				sInven_Effect[i].InvenFace.Transparency -= 30;
			
			if(sInven_Effect[i].InvenFace.Transparency >= 255)
				sInven_Effect[i].FadeType = 2;
			
			if(sInven_Effect[i].InvenFace.Transparency <= 0)
				sInven_Effect[i].Time = 0;

			if(sInven_Effect[i].Kind != SIN_EFFECT_SHOW_GYUNGFUM){ 
				if((sInven_Effect[i].Time%5) == 0){ 
					sInven_Effect[i].AniCount++;
					if(sInven_Effect[i].AniCount > 3)sInven_Effect[i].AniCount = 0;
					sInven_Effect[i].InvenFace.MatNum = MatInvenParticle[sInven_Effect[i].AniCount];

				}
			}

			if(sInven_Effect[i].InvenFace.Transparency <= 0)sInven_Effect[i].Time = 0; 
			
		}

	}

	return TRUE;
}

//이펙트 이미지를 그린다 
int cINVENTORY::sinDrawInvenTexImage(smFACE2D *Face)
{
	
	D3DTLVERTEX tlVertex[4];
	int cnt;
	int x,y;

	x = Face->x;
	y = Face->y;
	dsGetDrawOffset( &x , &y );


	for(cnt=0;cnt<4;cnt++) 
	{ 
		tlVertex[cnt].rhw = 1;
		tlVertex[cnt].color	= RGBA_MAKE(Face->r,Face->g,Face->b,Face->Transparency);
		tlVertex[cnt].specular  =  RGBA_MAKE( 0,0,0,0 );
		tlVertex[cnt].sz = 0;
	}

	tlVertex[0].sx = (float)x;   
	tlVertex[0].sy = (float)y;
	tlVertex[0].tu = 0;
	tlVertex[0].tv = 0;

	tlVertex[1].sx = (float)x+(float)Face->width;
	tlVertex[1].sy = (float)y;
	tlVertex[1].tu = 1;
	tlVertex[1].tv = 0;

	tlVertex[2].sx = (float)x;
	tlVertex[2].sy = (float)y+(float)Face->height;
	tlVertex[2].tu = 0;
	tlVertex[2].tv = 1;

	tlVertex[3].sx = ((float)x+(float)Face->width);
	tlVertex[3].sy = ((float)y+(float)Face->height);
	tlVertex[3].tu = 1;
	tlVertex[3].tv = 1;

	/*
    // Turn on Z-buffering
	//lpD3DDevice->SetRenderState( D3DRENDERSTATE_ZENABLE, FALSE ); (이걸 해주면 화면 캐릭이 사라진다 오호 신기하구나)

	lpD3DDevice->SetRenderState( D3DRENDERSTATE_ZFUNC,   D3DCMP_LESSEQUAL );
	// null out the texture handle
	lpD3DDevice->SetRenderState( D3DRENDERSTATE_TEXTUREHANDLE, 0 );

	// turn on dithering
	lpD3DDevice->SetRenderState( D3DRENDERSTATE_DITHERENABLE, TRUE );

	lpD3DDevice->SetRenderState( D3DRENDERSTATE_TEXTUREADDRESS, D3DTADDRESS_WRAP );
	lpD3DDevice->SetRenderState( D3DRENDERSTATE_TEXTUREPERSPECTIVE, TRUE ); 

	// D3DFILTER_LINEAR
	lpD3DDevice->SetRenderState( D3DRENDERSTATE_TEXTUREMAG, D3DFILTER_LINEAR );
	lpD3DDevice->SetRenderState( D3DRENDERSTATE_TEXTUREMIN, D3DFILTER_LINEAR );

	lpD3DDevice->SetRenderState( D3DRENDERSTATE_SPECULARENABLE, TRUE );
	lpD3DDevice->SetRenderState( D3DRENDERSTATE_COLORKEYENABLE, FALSE );

	lpD3DDevice->SetRenderState( D3DRENDERSTATE_TEXTUREMAPBLEND, D3DTBLEND_MODULATE );

	lpD3DDevice->SetRenderState( D3DRENDERSTATE_ALPHABLENDENABLE, TRUE );
	lpD3DDevice->SetRenderState( D3DRENDERSTATE_DESTBLEND, D3DBLEND_INVSRCALPHA );
	lpD3DDevice->SetRenderState( D3DRENDERSTATE_SRCBLEND, D3DBLEND_SRCALPHA );

	smRender.AlphaTestDepth = 60;


	lpD3DDevice->SetTextureStageState( 0, D3DTSS_MINFILTER, D3DTFN_POINT  );
	lpD3DDevice->SetTextureStageState( 0, D3DTSS_MAGFILTER, D3DTFG_POINT  );

	//lpD3DDevice->SetRenderState( D3DRENDERSTATE_TEXTUREMIN, D3DTFN_POINT );
	//lpD3DDevice->SetRenderState( D3DRENDERSTATE_TEXTUREMAG, D3DTFN_POINT );

	if(smMaterial[Face->MatNum].smTexture[0] != NULL)
	{
		if(smMaterial[Face->MatNum].smTexture[0]->lpD3DTexture != NULL)
		{
			if(FAILED(lpD3DDevice->SetTexture( 0, smMaterial[Face->MatNum].smTexture[0]->lpD3DTexture )))
			{
				OutputDebugString("SetTexture Error");
				return FALSE;
			}

			
			if(FAILED(lpD3DDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP ,
							D3DFVF_TLVERTEX  ,
							tlVertex,
							4,

							NULL)))
			{
				OutputDebugString("DrawPrimitive Error");
				return FALSE;
			}
		}


	}
	*/
	//lpD3DDevice->SetRenderState( D3DRENDERSTATE_CULLMODE , D3DCULL_NONE );
	if(lpD3DDevice != NULL)
	{
		  // Turn on Z-buffering
		lpD3DDevice->SetRenderState( D3DRENDERSTATE_CULLMODE , D3DCULL_NONE );
		lpD3DDevice->SetRenderState( D3DRENDERSTATE_ZENABLE, TRUE );
		lpD3DDevice->SetRenderState( D3DRENDERSTATE_ZWRITEENABLE, TRUE );
		//lpD3DDevice->SetRenderState( D3DRENDERSTATE_ZFUNC,   D3DCMP_LESSEQUAL );
		//	lpD3DDevice->SetRenderState( D3DRENDERSTATE_ZENABLE, D3DZB_USEW );//TRUE );


		// null out the texture handle
		lpD3DDevice->SetRenderState( D3DRENDERSTATE_TEXTUREHANDLE, 0 );

		// turn on dithering
		//D3DDevice->SetRenderState( D3DRENDERSTATE_DITHERENABLE, TRUE );

		lpD3DDevice->SetRenderState( D3DRENDERSTATE_TEXTUREADDRESS, D3DTADDRESS_WRAP);
		//lpD3DDevice->SetRenderState( D3DRENDERSTATE_TEXTUREPERSPECTIVE, TRUE ); 

		// D3DFILTER_LINEAR
		//lpD3DDevice->SetRenderState( D3DRENDERSTATE_TEXTUREMAG, D3DFILTER_LINEAR );
		//lpD3DDevice->SetRenderState( D3DRENDERSTATE_TEXTUREMIN, D3DFILTER_LINEAR );
 
		lpD3DDevice->SetRenderState( D3DRENDERSTATE_SPECULARENABLE, TRUE );
		lpD3DDevice->SetRenderState( D3DRENDERSTATE_COLORKEYENABLE, FALSE );

		lpD3DDevice->SetRenderState( D3DRENDERSTATE_TEXTUREMAPBLEND, D3DTBLEND_MODULATE );

		lpD3DDevice->SetRenderState( D3DRENDERSTATE_ALPHABLENDENABLE, TRUE );
		lpD3DDevice->SetRenderState( D3DRENDERSTATE_DESTBLEND, D3DBLEND_INVSRCALPHA );
		lpD3DDevice->SetRenderState( D3DRENDERSTATE_SRCBLEND, D3DBLEND_SRCALPHA );

		smRender.AlphaTestDepth = 60;

		lpD3DDevice->SetTextureStageState( 0, D3DTSS_MINFILTER, D3DTFN_POINT  );
        lpD3DDevice->SetTextureStageState( 0, D3DTSS_MAGFILTER, D3DTFG_POINT  );

		//lpD3DDevice->SetRenderState( D3DRENDERSTATE_TEXTUREMIN, D3DTFN_POINT );
		//lpD3DDevice->SetRenderState( D3DRENDERSTATE_TEXTUREMAG, D3DTFN_POINT );
		

		if(smMaterial[Face->MatNum].smTexture[0] != NULL)
		{
			if(smMaterial[Face->MatNum].smTexture[0]->lpD3DTexture != NULL)
			{
				if(FAILED(lpD3DDevice->SetTexture( 0, smMaterial[Face->MatNum].smTexture[0]->lpD3DTexture )))
				{
					OutputDebugString("SetTexture Error");
					return FALSE;
				}

				if(FAILED(lpD3DDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP ,
								D3DFVF_TLVERTEX  ,
								tlVertex,
								4,

								NULL)))
				{
					OutputDebugString("DrawPrimitive Error");
					return FALSE;
				}
			}
			
			lpD3DDevice->SetTextureStageState( cnt, D3DTSS_MINFILTER, D3DTFN_LINEAR  );
			lpD3DDevice->SetTextureStageState( cnt, D3DTSS_MAGFILTER, D3DTFG_LINEAR  );
			//D3DDevice->SetRenderState( D3DRENDERSTATE_TEXTUREADDRESS, D3DTADDRESS_WRAP);
			//lpD3DDevice->SetRenderState( D3DRENDERSTATE_TEXTUREMIN, D3DFILTER_LINEAR );
			//lpD3DDevice->SetRenderState( D3DRENDERSTATE_TEXTUREMAG, D3DFILTER_LINEAR );
		}
	}	

	return TRUE;
}


//아이템이 셋팅될수있는 지를 체크 
int cINVENTORY::CheckSetEmptyArea(sITEM *pItem)
{
	int cntx,cnty;
	int cx,cy;

	RECT	rect={0,0,0,0};
	int		flag=0;

	cx = (22*INVENTORY_BOX_ROW)-pItem->w; //아이템이 들어갈수있는 가장 끝 X좌표 
	cy = (22*INVENTORY_BOX_COL)-pItem->h; //아이템이 들어갈수있는 가장 끝 Y좌표

	//코드가 같지 않거나 아이템이 비어있지 않은 곳에 셋팅할때 검색해서 위치를 찾아낸다 
	for( cntx=StartX ; cntx <= StartX+cx ; cntx+=22){
		for( cnty=StartY ; cnty <= StartY+cy ; cnty+=22){ 
			flag = 0;
			for(int i=0 ; i<INVENTORY_MAXITEM ; i++){
				if(InvenItem[i].Flag){
					rect.left   = InvenItem[i].x;
					rect.right  = InvenItem[i].x+InvenItem[i].w;
					rect.top    = InvenItem[i].y;
					rect.bottom = InvenItem[i].y+InvenItem[i].h;

					if( cntx > rect.left-pItem->w && cntx< rect.right && 
						cnty > rect.top-pItem->h && cnty< rect.bottom )
						flag++;
				}
			}
			if ( flag==0 ) {
				pItem->SetX = cntx;
				pItem->SetY = cnty;
				pItem->ItemPosition = 0; 
				return TRUE;

			}
		}
	}

	return FALSE;

}

//아이템 새인증 (서버 메세지) 
int cINVENTORY::ResetInvenItemToServer(DWORD CODE ,DWORD Head , DWORD CheckSum , DWORD Time )
{	
	int i = 0;
	for(i = 0; i < INVENTORY_MAXITEM ; i++){
		if(InvenItem[i].Flag){
			if(InvenItem[i].CODE == CODE && 
				InvenItem[i].sItemInfo.ItemHeader.Head == Head &&
				InvenItem[i].sItemInfo.ItemHeader.dwChkSum == CheckSum ){
					InvenItem[i].sItemInfo.ItemHeader.dwTime = Time;
					return TRUE;

				}

		}
		if(InvenItemTemp[i].Flag){
			if(InvenItemTemp[i].CODE == CODE && 
				InvenItemTemp[i].sItemInfo.ItemHeader.Head == Head &&
				InvenItemTemp[i].sItemInfo.ItemHeader.dwChkSum == CheckSum ){
					InvenItemTemp[i].sItemInfo.ItemHeader.dwTime = Time;
					return TRUE;

				}

		}
	}
	return FALSE;
}

//아이템 삭제 (서버 메세지)
int cINVENTORY::DeleteInvenItemToServer(DWORD CODE ,DWORD Head , DWORD CheckSum)				
{

	//마우스 아이템을 체크해준다 
	if(MouseItem.Flag && MouseItem.CODE == CODE && 
		MouseItem.sItemInfo.ItemHeader.Head == Head &&
		MouseItem.sItemInfo.ItemHeader.dwChkSum == CheckSum ){
			MouseItem.Flag = 0;
	}

	int i = 0;
	for(i = 0; i < INVENTORY_MAXITEM ; i++){
		if(InvenItem[i].Flag){
			if(InvenItem[i].CODE == CODE && 
				InvenItem[i].sItemInfo.ItemHeader.Head == Head &&
				InvenItem[i].sItemInfo.ItemHeader.dwChkSum == CheckSum ){
					InvenItem[i].Flag = 0;
					if(InvenItem[i].ItemPosition){
						//양손아이템이 한손에 있을경우에는 지워준다 
						if(InvenItem[i].Class == ITEM_CLASS_WEAPON_TWO){
							if(InvenItem[i].ItemPosition == 1 ){//오른손 
								if(InvenItem[i].CODE != InvenItem[sInven[1].ItemIndex-1].CODE){
									InvenItem[sInven[1].ItemIndex-1].Flag = 0;
									InvenItem[i].Flag = 0;
									sInven[1].ItemIndex = 0; //인벤토리 박스 초기화 
									sInven[0].ItemIndex = 0; //인벤토리 박스 초기화 
									if(InvenItem[i].SetModelPosi)
										sinSetCharItem(InvenItem[i].CODE , InvenItem[i].SetModelPosi, FALSE); 

								}
							}
							if(InvenItem[i].ItemPosition == 2 ){//왼손 
								if(InvenItem[i].CODE != InvenItem[sInven[0].ItemIndex-1].CODE){
									InvenItem[sInven[0].ItemIndex-1].Flag = 0;
									InvenItem[i].Flag = 0;
									sInven[0].ItemIndex = 0; //인벤토리 박스 초기화 
									sInven[1].ItemIndex = 0; //인벤토리 박스 초기화 
									if(InvenItem[i].SetModelPosi)
										sinSetCharItem(InvenItem[i].CODE , InvenItem[i].SetModelPosi, FALSE); 

								}
							}

						}
						sInven[InvenItem[i].ItemPosition-1].ItemIndex  = 0 ;
							if(InvenItem[i].SetModelPosi)
								sinSetCharItem(InvenItem[i].CODE , InvenItem[i].SetModelPosi, FALSE); 

					}
					ReFormInvenItem();
					ReformCharForm();//재인증 
					ReFormPotionNum();
					SetItemToChar();
					break;

				}

		}
		if(InvenItemTemp[i].Flag){
			if(InvenItemTemp[i].CODE == CODE && 
				InvenItemTemp[i].sItemInfo.ItemHeader.Head == Head &&
				InvenItemTemp[i].sItemInfo.ItemHeader.dwChkSum == CheckSum ){
					InvenItemTemp[i].Flag = 0;
					ReFormInvenItem();
					ReformCharForm();//재인증 
					ReFormPotionNum();
					SetItemToChar();
					break;

				}
		}
	}

	//허상 아이템이 있을경우 체크해서 다시한번 지워준다
	if(!sInven[0].ItemIndex){
		if(sInven[1].ItemIndex){
			if(InvenItem[sInven[1].ItemIndex-1].Flag){
				if(InvenItem[sInven[1].ItemIndex-1].Class == ITEM_CLASS_WEAPON_TWO){
					if(!InvenItem[sInven[1].ItemIndex-1].sItemInfo.ItemHeader.Head){
						InvenItem[sInven[1].ItemIndex-1].Flag = 0;
						sInven[1].ItemIndex = 0;
						ReFormInvenItem();
						ReformCharForm();//재인증 
						ReFormPotionNum();
						SetItemToChar();
					}
				}
			}
		}
	}
	cInvenTory.CheckWeight();
	return TRUE;
}

//캐릭터 파라메터 조작을 방지한다 
int cINVENTORY::HackCharStateCheck(smCHAR_INFO *pChar_Info , smCHAR_INFO *desChar_Info)
{
	int HackFlag = 0;

	if(pChar_Info->Attack_Damage[0] < desChar_Info->Attack_Damage[0]){
		HackFlag = 1;

	}
	if(pChar_Info->Attack_Damage[1] < desChar_Info->Attack_Damage[1]){
		HackFlag = 1;

	}
	if(pChar_Info->Defence < desChar_Info->Defence){
		HackFlag = 1;

	}
	if(pChar_Info->Absorption < desChar_Info->Absorption){
		HackFlag = 1;

	}
	if(pChar_Info->Accuracy < desChar_Info->Accuracy){
		HackFlag = 1;

	}
	if(pChar_Info->Attack_Speed < desChar_Info->Attack_Speed){
		HackFlag = 1;

	}
	if(pChar_Info->Chance_Block < desChar_Info->Chance_Block){
		HackFlag = 1;

	}
	if(pChar_Info->Critical_Hit < desChar_Info->Critical_Hit){
		HackFlag = 1;

	}
	if(pChar_Info->Shooting_Range < desChar_Info->Shooting_Range){
		HackFlag = 1;

	}
	if(HackFlag){
		smTRANS_COMMAND	smTransCommand;
		smTransCommand.size = sizeof(smTRANS_COMMAND);
		smTransCommand.code = smTRANSCODE_FIX_ATTACK;
		smTransCommand.WParam = pChar_Info->Attack_Damage[1];
		smTransCommand.LParam = pChar_Info->Defence;
		smTransCommand.SParam = (int)(pChar_Info->Absorption*256);
		smTransCommand.EParam = 0;
		if ( smWsockDataServer ) 
			smWsockDataServer->Send2( (char *)&smTransCommand , smTransCommand.size , TRUE );

		return FALSE;
	}
	return TRUE;
}
	
//잘못 배치된 아이템을 재배치한다 
int cINVENTORY::CheckResetInvenItem()
{
	RECT rect;
	for(int i = 0; i < INVENTORY_MAXITEM ; i++){
		if(InvenItem[i].Flag){
			if(InvenItem[i].ItemPosition){
				if(!sInven[InvenItem[i].ItemPosition-1].ItemIndex || sInven[InvenItem[i].ItemPosition-1].ItemIndex-1 != i )continue;
				rect.left   = sInven[InvenItem[i].ItemPosition-1].Rect.left;
				rect.top    = sInven[InvenItem[i].ItemPosition-1].Rect.top;
				rect.right  = sInven[InvenItem[i].ItemPosition-1].Rect.right;
				rect.bottom = sInven[InvenItem[i].ItemPosition-1].Rect.bottom;
				
				if(InvenItem[i].x <	rect.left || InvenItem[i].y < rect.top ||
					InvenItem[i].x + InvenItem[i].w > rect.right+2 || InvenItem[i].y + InvenItem[i].h > rect.bottom+2 ){
						
						InvenItem[i].x = rect.left + (((rect.right - rect.left) - InvenItem[i].w)/2);
						InvenItem[i].y = rect.top + (((rect.bottom - rect.top) - InvenItem[i].h)/2);
						ReFormInvenItem();
				}
			}

		}
		if(InvenItemTemp[i].Flag){
			if(InvenItemTemp[i].ItemPosition){
				rect.left   = sInven[InvenItemTemp[i].ItemPosition-1].Rect.left;
				rect.top    = sInven[InvenItemTemp[i].ItemPosition-1].Rect.top;
				rect.right  = sInven[InvenItemTemp[i].ItemPosition-1].Rect.right;
				rect.bottom = sInven[InvenItemTemp[i].ItemPosition-1].Rect.bottom;
				
				if(InvenItemTemp[i].x <	rect.left || InvenItemTemp[i].y < rect.top ||
					InvenItemTemp[i].x + InvenItemTemp[i].w > rect.right+2 || InvenItemTemp[i].y + InvenItemTemp[i].h > rect.bottom+2 ){
						
						InvenItemTemp[i].x = rect.left + (((rect.right - rect.left) - InvenItemTemp[i].w)/2);
						InvenItemTemp[i].y = rect.top + (((rect.bottom - rect.top) - InvenItemTemp[i].h)/2);
						ReFormInvenItem();

				}
			}
		}
	}
	return TRUE;
}

//같은 코드의 아이템이 있는지를 찾는다
int cINVENTORY::SearchItemCode(DWORD CODE , int KindFlag)
{
	int ItemCount=0;
	if(CODE){
		for(int i = 0; i < INVENTORY_MAXITEM ; i++){
			if(InvenItem[i].Flag){
				if(InvenItem[i].CODE == CODE)ItemCount++;
			}
			if(InvenItemTemp[i].Flag){
				if(InvenItemTemp[i].CODE == CODE)ItemCount++;
			}
		}
		return ItemCount;
	}

	if(KindFlag){
		for(int i = 0; i < INVENTORY_MAXITEM ; i++){
			if(InvenItem[i].Flag){
				if(InvenItem[i].sItemInfo.ItemKindCode == KindFlag)ItemCount++;
			}
			if(InvenItemTemp[i].Flag){
				if(InvenItemTemp[i].sItemInfo.ItemKindCode == KindFlag)ItemCount++;
			}
		}
		return ItemCount;
	}
	return FALSE;
}

//최대데미지가 나올수있는 아이템을 체크해서 서버로 보내준다 
int cINVENTORY::CheckDamage()
{
	int Damage[2] = {0,0};
	int Critical = 0;
	int SendDamage[2] = {0,0};
	int SendCritical = 0;
	int RealDamage[2] = {0,0};
	int RealSendCritical = 0;

	int i = 0 , j = 0 , k=0 ,t=0;

	DWORD WeaponCode[7] = {sinWA1,sinWS1,sinWP1,sinWS2,sinWT1,sinWM1,sinWM1};
	int   CharCode[7]   = {SIN_CHAR_FIGHTER,SIN_CHAR_ARCHER,SIN_CHAR_PIKEMAN,SIN_CHAR_KNIGHT,SIN_CHAR_ATALANTA,SIN_CHAR_PRIESTESS,SIN_CHAR_MAGICIAN};

	sITEM *pItem=0;
	pItem = new sITEM;
	for( i = 0; i < INVENTORY_MAXITEM*2 ; i++){
		Damage[0] = 0; Damage[1] = 0; Critical = 0; //초기화 
		if(i<100){
			if(!InvenItem[i].Flag)continue;
			if(InvenItem[i].NotUseFlag)continue;
			memcpy(pItem,&InvenItem[i],sizeof(sITEM));

		}
		else{
			if(!InvenItemTemp[i-100].Flag)continue;
			if(InvenItemTemp[i-100].NotUseFlag)continue;
			memcpy(pItem,&InvenItemTemp[i-100],sizeof(sITEM));
		}
		if(pItem->sItemInfo.Damage[0] || pItem->sItemInfo.Damage[1] || pItem->sItemInfo.Critical_Hit){
			Damage[0] += pItem->sItemInfo.Damage[0];
			Damage[1] += pItem->sItemInfo.Damage[1];
			Critical  += pItem->sItemInfo.Critical_Hit;

			//박상열휴가갔네
			if((sinDamageFunction[0] == 1 && (pItem->WeaponClass == 1 || pItem->WeaponClass == 3))){
				SendDamage[0] = 1+(Damage[0]*(sinChar->Strength+130)/130) +
											((sinChar->Talent + sinChar->Dexterity)/40)+((Damage[0]+Damage[1])/16);
				SendDamage[1] = 3+(Damage[1]*(sinChar->Strength+130)/130) + 
											((sinChar->Talent + sinChar->Dexterity)/40);
			}

			if((sinDamageFunction[0] == 2 && (pItem->WeaponClass == 1 || pItem->WeaponClass == 3))){
			////(근접형2)
				SendDamage[0] = 1+(Damage[0]*(sinChar->Strength+150)/150) +
											((sinChar->Talent + sinChar->Dexterity)/45)+((Damage[0]+Damage[1])/16);
				SendDamage[1] = 3+(Damage[1]*(sinChar->Strength+150)/150) +
											((sinChar->Talent + sinChar->Dexterity)/45);
			}
			if((sinDamageFunction[0] == 3 && (pItem->WeaponClass == 1 || pItem->WeaponClass == 3))){
			////(근접형3)
				SendDamage[0] = 1+(Damage[0]*(sinChar->Strength+190)/190) +
											((sinChar->Talent + sinChar->Dexterity)/50)+((Damage[0]+Damage[1])/16);
				SendDamage[1] = 3+(Damage[1]*(sinChar->Strength+190)/190) +
											((sinChar->Talent + sinChar->Dexterity)/50);
			}
			if(sinDamageFunction[1] == 1 && pItem->WeaponClass == 2){
			////(발사형1)
				SendDamage[0] = 1+(Damage[0]*(sinChar->Dexterity+130)/130) + 
											((sinChar->Talent + sinChar->Strength)/40)+ ((Damage[0]+Damage[1])/16);
				SendDamage[1] = 3+(Damage[1]*(sinChar->Dexterity+130)/130) + 
											((sinChar->Talent + sinChar->Strength)/40);
			}

			if(sinDamageFunction[1] == 2 && pItem->WeaponClass == 2){
			////(발사형2)
				SendDamage[0] = 1+(Damage[0]*(sinChar->Dexterity+190)/190) +
											((sinChar->Talent + sinChar->Strength)/50)+((Damage[0]+Damage[1])/16);
				SendDamage[1] = 3+(Damage[1]*(sinChar->Dexterity+190)/190) +
											((sinChar->Talent + sinChar->Strength)/50);
			}



			if(sinDamageFunction[2] == 1 && pItem->WeaponClass == 3){
			////(법사형1)
				SendDamage[0] = 1+(Damage[0]*(sinChar->Spirit+150)/150) +
											((sinChar->Talent)/30)+((Damage[0]+Damage[1])/16);
				SendDamage[1] = 3+(Damage[1]*(sinChar->Spirit+150)/150) +
											((sinChar->Talent)/30);
			}

			if(sinDamageFunction[2] == 2 && pItem->WeaponClass == 3){
			////(법사형2)
				SendDamage[0] = 1+(Damage[0]*(sinChar->Spirit+180)/180) +
											((sinChar->Talent)/30);
				SendDamage[1] = 2+(Damage[1]*(sinChar->Spirit+180)/180) +
											((sinChar->Talent)/25);
			}
			if(sinDamageFunction[2] == 2 && pItem->WeaponClass == 3){
			////(법사형2)
				SendDamage[0] = 1+(Damage[0]*(sinChar->Spirit+170)/170) +
											((sinChar->Talent)/30)+ ((Damage[0]+Damage[1])/16);
				SendDamage[1] = 3+(Damage[1]*(sinChar->Spirit+170)/170) +
											((sinChar->Talent)/30);
			}

			//주무기군 
			for( j= 0 ; j < 7 ; j++){ 
				if(CharCode[j] == sinChar->JOB_CODE){
					if(WeaponCode[j] == (pItem->sItemInfo.CODE & sinITEM_MASK2)){
						SendDamage[0] += sinChar->Level/6;
						SendDamage[1] += sinChar->Level/6;
					}
				}
			}
			//메카니션은 주무기군이 특화루 설정되어있다 
			if(sinChar->JOB_CODE == SIN_CHAR_MECANICIAN){
				if(sinChar->JobBitMask & pItem->sItemInfo.JobCodeMask){
					SendDamage[0] += sinChar->Level/6;
					SendDamage[1] += sinChar->Level/6;
				}		
			}


			//스킬
			//////////////////////// 데미지 가중치 
			for(k = 1 ; k < MAX_USESKILL  ; k++){ //노멀 어택을 빼준다 
				if(sinSkill.UseSkill[k].Flag && sinSkill.UseSkill[k].Point){ //사용 포인트가 있으면 
					//////////////같은 계열무기일경우에 해당 
					for(t = 0; t < 8 ; t++){
						switch(sinSkill.UseSkill[k].CODE){
							case SKILL_MELEE_MASTERY:       //데미지 증가 
								if(sinSkill.UseSkill[k].Skill_Info.UseWeaponCode[t] == (pItem->sItemInfo.CODE&sinITEM_MASK2)){
									SendDamage[0] += (int)(((float)Melee_Mastery_DamagePercent[sinSkill.UseSkill[k].Point-1]/100)*pItem->sItemInfo.Damage[0]);
									SendDamage[1] += (int)(((float)Melee_Mastery_DamagePercent[sinSkill.UseSkill[k].Point-1]/100)*pItem->sItemInfo.Damage[1]);
								}
							break;
							case SKILL_SHOOTING_MASTERY:    //활 데미지 증가  
								if(sinSkill.UseSkill[k].Skill_Info.UseWeaponCode[t] == (pItem->sItemInfo.CODE&sinITEM_MASK2)){
									SendDamage[0] += (int)(((float)S_Mastery_DamagePercent[sinSkill.UseSkill[k].Point-1]/100)*pItem->sItemInfo.Damage[0]);
									SendDamage[1] += (int)(((float)S_Mastery_DamagePercent[sinSkill.UseSkill[k].Point-1]/100)*pItem->sItemInfo.Damage[1]);
							
								}	
							break;
							case SKILL_THROWING_MASTERY:
								if(sinSkill.UseSkill[k].Skill_Info.UseWeaponCode[t] == (pItem->sItemInfo.CODE&sinITEM_MASK2)){
									SendDamage[0] += (int)(((float)T_Mastery_Damage[sinSkill.UseSkill[k].Point-1]/100)*pItem->sItemInfo.Damage[0]);
									SendDamage[1] += (int)(((float)T_Mastery_Damage[sinSkill.UseSkill[k].Point-1]/100)*pItem->sItemInfo.Damage[1]);
							}
							break;
							case SKILL_CRITICAL_MASTERY:
								if(sinSkill.UseSkill[k].Skill_Info.UseWeaponCode[t] == (pItem->sItemInfo.CODE&sinITEM_MASK2)){
									Critical += Critical_Mastery_Critical[sinSkill.UseSkill[k].Point-1];

								}
							break;
							case SKILL_SWORD_MASTERY:
								if(sinSkill.UseSkill[k].Skill_Info.UseWeaponCode[t] == (pItem->sItemInfo.CODE&sinITEM_MASK2)){
									SendDamage[0] += (int)(((float)Sword_Mastery_DamagePercent[sinSkill.UseSkill[k].Point-1]/100)*pItem->sItemInfo.Damage[0]);
									SendDamage[1] += (int)(((float)Sword_Mastery_DamagePercent[sinSkill.UseSkill[k].Point-1]/100)*pItem->sItemInfo.Damage[1]);
							
								}	
							break;

						}
					}
				}
			}

			//특화 
			if(sinChar->JobBitMask & pItem->sItemInfo.JobCodeMask){
				if(pItem->sItemInfo.JobItem.Lev_Damage[1]) // LV/x 특화 +데미지 sinLev_Damage[0]은 사용하고있지않다
					SendDamage[1] +=  (sinChar->Level /pItem->sItemInfo.JobItem.Lev_Damage[1]);			//공격력
				if(pItem->sItemInfo.JobItem.Add_Critical_Hit)
					Critical  += pItem->sItemInfo.JobItem.Add_Critical_Hit;	//1.5배 데미지확율
			}
			SendCritical = Critical;
			SendDamage[0] += 1; //추가 데미지 
			SendDamage[1] += 1;

			if(RealDamage[0] < SendDamage[0] ){
				RealDamage[0] = SendDamage[0];
			}
			if(RealDamage[1] < SendDamage[1] ){
				RealDamage[1] = SendDamage[1];
			}
			if(RealSendCritical < SendCritical )
				RealSendCritical = SendCritical;
		}
	}

	//요기서 값을 보내준다 SendDamage를 보낸다 
	SendMaxDamageToServer(RealDamage[0],RealDamage[1],RealSendCritical);
	return TRUE;
}


//퀘스트 아이템을 체크해서 날려버린다 
int cINVENTORY::ClearQuestItem()
{
	sITEM *pItem=0;
	for(int i = 0; i < INVENTORY_MAXITEM*2 ; i++){
		if(i<100){
			if(!InvenItem[i].Flag)continue;
			pItem = &InvenItem[i];
		}
		else{
			if(!InvenItemTemp[i-100].Flag)continue;
			pItem = &InvenItemTemp[i-100];
		}
		if((pItem->sItemInfo.CODE & sinITEM_MASK2) == sinMA2){
			pItem->Flag = 0;
		}
	}	
	cInvenTory.ReFormInvenItem();
	ResetInvenItemCode();
	return TRUE;
}

// pluto 커플링
int cINVENTORY::DeleteCoupleRing()
{
	sITEM *pItem=0;
	for(int i = 0; i < INVENTORY_MAXITEM*2 ; i++)
	{
		if(i<100)
		{
			if(!InvenItem[i].Flag)continue;
			pItem = &InvenItem[i];
		}
		else
		{
			if(!InvenItemTemp[i-100].Flag)continue;
			pItem = &InvenItemTemp[i-100];
		}
		if( pItem->sItemInfo.CODE == (sinSP1|sin16) )
		{
			pItem->Flag = 0;
			break;
		}
	}	
	cInvenTory.ReFormInvenItem();
	ResetInvenItemCode();
	return TRUE;
}

//링크코어 데이타를 서버에서 받는다
int cINVENTORY::RecvLinkCore(sITEMINFO *pItemInfo)
{
	sITEMINFO *pItemInfo2=0;
	for(int i = 0; i < INVENTORY_MAXITEM*2 ; i++){
		if(i<100){
			if(!InvenItem[i].Flag)continue;
			pItemInfo2 = &InvenItem[i].sItemInfo;
		}
		else{
			if(!InvenItemTemp[i-100].Flag)continue;
			pItemInfo2 = &InvenItemTemp[i-100].sItemInfo;
		}
		if(pItemInfo->BackUpChkSum == pItemInfo2->BackUpChkSum &&
			pItemInfo->BackUpKey == pItemInfo2->BackUpKey &&
			pItemInfo->CODE == pItemInfo2->CODE){
				memcpy(pItemInfo2,pItemInfo,sizeof(sITEMINFO));
				CheckWeight();//무게 보정 
				ReFormInvenItem();
				return TRUE;

			}
	}
	return FALSE;
}
int sinMousePotionNum[4] = {0,0,0,0};
int ChecMousekPotionNum[4] = { 0,0,0,0};

//마우스의 포션을 체크한다
int cINVENTORY::CheckMousePotionNumForm()
{
	for(int k=0;k<4;k++){
		ChecMousekPotionNum[k] = 0;
	
	}	

	if(MouseItem.Flag){
		if(MouseItem.Class  == ITEM_CLASS_POTION){
			switch(MouseItem.sItemInfo.CODE & sinITEM_MASK3){
				case sin01:
					ChecMousekPotionNum[0] += MouseItem.sItemInfo.PotionCount;

				break;
				case sin02:
					ChecMousekPotionNum[1] += MouseItem.sItemInfo.PotionCount;

				break;
				case sin03:
					ChecMousekPotionNum[2] += MouseItem.sItemInfo.PotionCount;

				break;
				case sin04:
					ChecMousekPotionNum[3] += MouseItem.sItemInfo.PotionCount;

				break;
			}
		}
	}
	for(int j=0;j<4;j++){
		if(ChecMousekPotionNum[j] !=sinMousePotionNum[j]){
			//SendSetHackUser(TRUE); //해킹을 하려고했던 못된 유저를 고발 TRUE 접속 종료 
			SendSetHackUser2(1510,j); //해킹한 유저를 신고한다
			

		}

	}

	return TRUE;
}
//마우스의 포션을 인증한다 
int cINVENTORY::ReFormMousePotionNum()
{
	for(int j=0;j<4;j++){
		sinMousePotionNum[j] = 0;
	
	}	
	if(MouseItem.Flag){
		if(MouseItem.Class  == ITEM_CLASS_POTION){
			switch(MouseItem.sItemInfo.CODE & sinITEM_MASK3){
				case sin01:
					sinMousePotionNum[0] += MouseItem.sItemInfo.PotionCount;

				break;
				case sin02:
					sinMousePotionNum[1] += MouseItem.sItemInfo.PotionCount;

				break;
				case sin03:
					sinMousePotionNum[2] += MouseItem.sItemInfo.PotionCount;

				break;
				case sin04:
					sinMousePotionNum[3] += MouseItem.sItemInfo.PotionCount;

				break;
			}
		}
	}
	return TRUE;
}

//유니온 코어 이펙트를 셋팅한다
int cINVENTORY::SetUnionCoreEffect(int x , int y)
{
	for(int i = 0 ; i < 10 ; i++){    
		if(!UnionCore_Effect_Bone[i].Flag){ 
			UnionCore_Effect_Bone[i].DesPosi.x = x+22; //유니온 코어의 중심 좌표를체크한다
			UnionCore_Effect_Bone[i].DesPosi.y = y;//-500;
			UnionCore_Effect_Bone[i].Flag = 1;
			UnionCore_Effect_Bone[i].Posi.y -= 350;


			UnionCore_Effect_Bone[i].InvenFace.MatNum = *(int *)AssaSearchRes("par2.tga",SMMAT_BLEND_ALPHA);
			//UnionCore_Effect_Bone[i].InvenFace.x = x;
			//UnionCore_Effect_Bone[i].InvenFace.y = y;

			//UnionCore_Effect_Bone[i].InvenFace.x = 200;
			//UnionCore_Effect_Bone[i].InvenFace.y = 200;

			UnionCore_Effect_Bone[i].Max_Time = 100;
			UnionCore_Effect_Bone[i].InvenFace.r = 255;
			UnionCore_Effect_Bone[i].InvenFace.g = 255;
			UnionCore_Effect_Bone[i].InvenFace.b = 255;
			UnionCore_Effect_Bone[i].InvenFace.Transparency = 255;
			UnionCore_Effect_Bone[i].InvenFace.height = 32;
			UnionCore_Effect_Bone[i].InvenFace.width  = 32;
			UnionCore_Effect_Bone[i].InvenFace.TexRect = sinTexRect;
			UnionCore_Effect_Bone[i].Flag = 1;

		}
	}
	return TRUE;
}

int cINVENTORY::UnionCoreParticleProcess()
{
	int i = 0;
	//본
	for(i = 0 ; i < 10 ; i++){    
		if(UnionCore_Effect_Bone[i].Flag){
			UnionCore_Effect_Bone[i].Time += GetRandomPos(1,3);
			UnionCore_Effect_Bone[i].Posi.y +=30;
			UnionCore_Effect_Bone[i].InvenFace.y = UnionCore_Effect_Bone[i].Posi.y;
			UnionCore_Effect_Bone[i].Posi.x = UnionCore_Effect_Bone[i].DesPosi.x-5;
			UnionCore_Effect_Bone[i].InvenFace.x = UnionCore_Effect_Bone[i].Posi.x;
			//if((UnionCore_Effect_Bone[i].Time % 5 )==0)
			if(UnionCore_Effect_Bone[i].Posi.y <= UnionCore_Effect_Bone[i].DesPosi.y - 10)
				UnionCoreParticle(UnionCore_Effect_Bone[i].Posi.x,UnionCore_Effect_Bone[i].Posi.y);

			if(UnionCore_Effect_Bone[i].Posi.y >= UnionCore_Effect_Bone[i].DesPosi.y){
				UnionCoreParticle2(UnionCore_Effect_Bone[i].InvenFace.x,UnionCore_Effect_Bone[i].InvenFace.y);
				memset(&UnionCore_Effect_Bone[i],0,sizeof(sINVENTORY_EFFECT));

			}
		}
	}

	//파티클
	for(i = 0 ; i < 100; i++){        
		if(UnionCore_Effect[i].Flag){  
			UnionCore_Effect[i].Time++;
			if(UnionCore_Effect[i].Direction ==1) 
				UnionCore_Effect[i].InvenFace.x -=GetRandomPos(0,1);
			else UnionCore_Effect[i].InvenFace.x +=GetRandomPos(0,1);
			//UnionCore_Effect[i].InvenFace.Transparency -=5;


			//UnionCore_Effect[i].InvenFace.x +=GetRandomPos(-5,5);
			//UnionCore_Effect[i].InvenFace.y +=GetRandomPos(-5,5);
			if(UnionCore_Effect[i].Time >= UnionCore_Effect[i].Max_Time){
				memset(&UnionCore_Effect[i],0,sizeof(sINVENTORY_EFFECT));
			}
		}
	}
	
	
	for(i = 0 ; i < 50 ; i++){                
		if(UnionCore_Effect_Bomb[i].Flag){
			UnionCore_Effect_Bomb[i].Time++;
			/*
			if(UnionCore_Effect_Bomb[i].Direction == 1){
				if(UnionCore_Effect_Bomb[i].Time < 6){
					UnionCore_Effect_Bomb[i].InvenFace.width += 40;
					UnionCore_Effect_Bomb[i].InvenFace.x -=20;
				}
				else{
					UnionCore_Effect_Bomb[i].InvenFace.width -= 20;
					UnionCore_Effect_Bomb[i].InvenFace.x +=10;
				}
			}
			else{
				if(UnionCore_Effect_Bomb[i].Time < 6){
					UnionCore_Effect_Bomb[i].InvenFace.height +=20;
					UnionCore_Effect_Bomb[i].InvenFace.y -=10;
				}
				else{
					UnionCore_Effect_Bomb[i].InvenFace.height -=10;
					UnionCore_Effect_Bomb[i].InvenFace.y +=5;

				}
			}
			*/

			//UnionCore_Effect_Bomb[i].InvenFace.height += UnionCore_Effect_Bomb[i].Direction*2;

			//UnionCore_Effect_Bomb[i].InvenFace.x -= (UnionCore_Effect_Bomb[i].Direction/2)-2;
			//UnionCore_Effect_Bomb[i].InvenFace.y -= (UnionCore_Effect_Bomb[i].Direction/2)-2;

//			UnionCore_Effect_Bomb[i].InvenFace.Transparency -= 5;

			if(UnionCore_Effect_Bomb[i].Time >= UnionCore_Effect_Bomb[i].Max_Time){
				memset(&UnionCore_Effect_Bomb[i],0,sizeof(sINVENTORY_EFFECT));
			}

		}
	}
	
	for(int i = 0 ; i < 10 ; i++){       
		if(sUCore_Effect[i].Flag){
			sUCore_Effect[i].Time++;
			if(sUCore_Effect[i].Time > 0){
				sUCore_Effect[i].h +=2;
				sUCore_Effect[i].w +=2;
				sUCore_Effect[i].x --;
				sUCore_Effect[i].y --;
				sUCore_Effect[i].Alpha -=6;
			}

			if(sUCore_Effect[i].Time > sUCore_Effect[i].Max_Time){
				memset(&sUCore_Effect[i],0,sizeof(sUNIONCORE_EFFECT));
			}
		}
	}


	

	return TRUE;
}

int cINVENTORY::UnionCoreParticle(int x , int y)
{
	for(int i = 0 ; i < 100 ; i++){      
		if(!UnionCore_Effect[i].Flag){  
			//UnionCore_Effect[i].InvenFace.MatNum = *(int *)AssaSearchRes("star05B_02.bmp",SMMAT_BLEND_LAMP);
			UnionCore_Effect[i].InvenFace.MatNum = *(int *)AssaSearchRes("par2.tga",SMMAT_BLEND_ALPHA);
			UnionCore_Effect[i].InvenFace.x = x+GetRandomPos(-3,3);
			UnionCore_Effect[i].InvenFace.y = y+GetRandomPos(-15,15);
			UnionCore_Effect[i].Max_Time = GetRandomPos(8,15);
			UnionCore_Effect[i].InvenFace.r = 255;
			UnionCore_Effect[i].InvenFace.g = 255;
			UnionCore_Effect[i].InvenFace.b = 255;
			UnionCore_Effect[i].InvenFace.Transparency = 255;
			UnionCore_Effect[i].InvenFace.height = GetRandomPos(10,11);
			UnionCore_Effect[i].InvenFace.width  = UnionCore_Effect[i].InvenFace.height;
			UnionCore_Effect[i].InvenFace.TexRect = sinTexRect;
			UnionCore_Effect[i].Flag = 1;
			if((i%2)==0)
				UnionCore_Effect[i].Direction = 1;
			else UnionCore_Effect[i].Direction = -1;

			break;

		}
	}
	return TRUE;
}

//유니온 코어 터지는 이펙트 
int cINVENTORY::UnionCoreParticle2(int x , int y , int Flag)
{

	int Count = 0;
	for(int i = 0 ; i < 10 ; i++){                   
		if(!sUCore_Effect[i].Flag){  
			if(Flag)
				sUCore_Effect[i].MatNum = 	MatInterEffectAging[0];
			else
				sUCore_Effect[i].MatNum = MatUnionCoreEffect;

			sUCore_Effect[i].x = x+12;
			sUCore_Effect[i].y = y;
			sUCore_Effect[i].Max_Time = 40;
			sUCore_Effect[i].Alpha = 255;
			sUCore_Effect[i].w = 20;
			sUCore_Effect[i].h = 20;
			sUCore_Effect[i].Flag = 1;
			//sUCore_Effect[i].Time = -20*Count;
			Count++;
			if(Count > 1)break;

		}
	}
	

	/*
	int i; for( i =0 ; i < 50 ; i++){   
		if(!UnionCore_Effect_Bomb[i].Flag){
			UnionCore_Effect_Bomb[i].InvenFace.MatNum = MatUnionCoreEffect;
			UnionCore_Effect_Bomb[i].InvenFace.x = x;
			UnionCore_Effect_Bomb[i].InvenFace.y = y-10;
			UnionCore_Effect_Bomb[i].Max_Time = 200;
			UnionCore_Effect_Bomb[i].InvenFace.r = 255;
			UnionCore_Effect_Bomb[i].InvenFace.g = 255;
			UnionCore_Effect_Bomb[i].InvenFace.b = 255;
			UnionCore_Effect_Bomb[i].InvenFace.Transparency = 255;
			UnionCore_Effect_Bomb[i].InvenFace.height = 32;
			UnionCore_Effect_Bomb[i].InvenFace.width  = UnionCore_Effect_Bomb[i].InvenFace.height;
			UnionCore_Effect_Bomb[i].InvenFace.TexRect = sinTexRect;
			UnionCore_Effect_Bomb[i].Flag = 1;
		}
	}
	*/
	return TRUE;
}


//코드로 인벤토리 공간을 체크한다 
int cINVENTORY::CheckInvenEmpty(sITEM *pItem)
{
	//돈과경험치는 공간을 체크하지않는다
	if(pItem->CODE == (sinGG1|sin01)||pItem->CODE == (sinGG1|sin02))return TRUE;

	char szFilePath[256];
	for(int j=0 ; j < MAX_ITEM ; j++){
		if(pItem->CODE == sItem[j].CODE){	
			wsprintf(szFilePath ,"Image\\sinImage\\Items\\%s\\it%s.bmp",sItem[j].ItemFilePath,sItem[j].LastCategory);
			if(!sItem[j].lpTempItem) //이미지가 없으면 로드한다
				sItem[j].lpTempItem = LoadDibSurfaceOffscreen(szFilePath);
			pItem->lpItem = sItem[j].lpTempItem; //템프에 있던 이미지 포인터를 넣어준다 
			pItem->w = sItem[j].w;
			pItem->h = sItem[j].h;
		}

	}

	memcpy(&cTrade.CheckItem,&cInvenTory.InvenItem,sizeof(sITEM)*100); //공간 체크를 위해서 복사 
	if(cTrade.TradeCheckEmptyArea(pItem))
        return TRUE;
	else return FALSE;
}

//돈이미지와 경험치 이미지를 로딩한다 
int cINVENTORY::LoadMoneyExpImage(sITEM *pItem)
{
	//돈
	if(pItem->CODE ==(sinGG1|sin01)){
		if(!lpGold7)lpGold7 = LoadDibSurfaceOffscreen("image\\Sinimage\\MessageBox\\Gold7.bmp");
		pItem->lpItem = lpGold7;

	}
	if(pItem->CODE ==(sinGG1|sin02)){
		if(!lpExp7)lpExp7 = LoadDibSurfaceOffscreen("image\\Sinimage\\MessageBox\\Exp7.bmp");
		pItem->lpItem = lpExp7;

	}
	pItem->w = 44;
	pItem->h = 44;
	return TRUE;
}

//sItemInfo로 아이템을 인벤토리에 셋팅한다
int cINVENTORY::SetInvenToItemInfo(sITEMINFO *pItemInfo)
{

	//돈일 경우
	if(pItemInfo->CODE == (sinGG1|sin01)){
		CheckCharForm();//인증 
		sinPlusMoney(pItemInfo->Money);
		ReformCharForm();//재인증 	
		SendSaveMoney(); //금액 조작을 못하게하기위해 호출한다 
		sinPlaySound(SIN_SOUND_COIN);
		return TRUE;
	}

	//경험치일경우 
	if(pItemInfo->CODE == (sinGG1|sin02)){
		AddExp(pItemInfo->Money );
		sinPlaySound(SIN_SOUND_EAT_POTION2);
		return TRUE;

	}

	//아이템일 경우 
	sITEM TempItem;
	if(LoadItemImage(pItemInfo , &TempItem)){
		if(cInvenTory.CheckRequireItemToSet(&TempItem)){         //곧바로 셋팅될수있는지를 체크한다  
			if(!cInvenTory.AutoSetInvenItem(&TempItem,1)){
				TempItem.Flag = 0;
				return FALSE;
			}
		}
	}
	return TRUE;
}

/*******************************************************************************
*						Wing Item 
********************************************************************************/	
int cINVENTORY::sinWingQuestNpc() //윙아이템 퀘스트 
{
	
	sITEM *pItem=0;  
	int   Index = 0;
	int   i = 0;

	WingItemQuestIndex = 0;//초기화 
	memset(&sWingItem_Send,0,sizeof(sCRAFTITEM_SERVER)); //초기화

	for( i = 0; i < INVENTORY_MAXITEM*2 ; i++){
		if(i<100){
			if(!InvenItem[i].Flag)continue;
			pItem = &InvenItem[i];
		}
		else{
			if(!InvenItemTemp[i-100].Flag)continue;
			pItem = &InvenItemTemp[i-100];
		}
		switch(pItem->sItemInfo.CODE){
			case (sinQW1 | sin01): //메탈윙
				Index = 1;
				memcpy(&sWingItem_Send.DesCraftItem,pItem,sizeof(sITEM));

			break;
			case (sinQW1 | sin02): //실버윙
				Index = 2;
				memcpy(&sWingItem_Send.DesCraftItem,pItem,sizeof(sITEM));

			break;
			case (sinQW1 | sin03): //골드윙
				Index = 3;
				memcpy(&sWingItem_Send.DesCraftItem,pItem,sizeof(sITEM));
			break;
			case (sinQW1 | sin04): //다이아윙
				Index = 4;
				memcpy(&sWingItem_Send.DesCraftItem,pItem,sizeof(sITEM));
			break;
			case (sinQW1 | sin05): //케이아스 윙
				Index = 5;
				memcpy(&sWingItem_Send.DesCraftItem,pItem,sizeof(sITEM));
			break;
			case (sinQW1 | sin06): //익스트림윙 로스트 아일랜드 성근 추가
				if ( smConfig.DebugMode ) 
			 	if(WingItemQuestIndex = 1000)  //받을아이템이 없을 경우에대한 임의 셋팅
				cSinHelp.sinShowHelp(SIN_HELP_KIND_QUEST_WING,QuestMessageBoxPosi2.x,QuestMessageBoxPosi2.y,QuestMessageBoxSize2.x,QuestMessageBoxSize2.y,RGBA_MAKE(0,15,128,125), WingQuestFilePath_b[6]);  
				return FALSE;
				
			break;
		}
	}	
	int CodeCount = 0;
	int ItemCount = 0;

	for(int j = 0 ; j < 6; j++){			
		if(sWingQuest[Index].NeedItemCode[j]){
			CodeCount++;
			for( i = 0; i < INVENTORY_MAXITEM*2 ; i++){
				if(i<100){
					if(!InvenItem[i].Flag)continue;
					pItem = &InvenItem[i];
				}
				else{
					if(!InvenItemTemp[i-100].Flag)continue;
					pItem = &InvenItemTemp[i-100];
				}
				if(sWingQuest[Index].NeedItemCode[j] == pItem->sItemInfo.CODE){
					sWingItem_Send.SheltomCode[ItemCount] = pItem->sItemInfo.CODE;
					sWingItem_Send.Head[ItemCount] = pItem->sItemInfo.ItemHeader.Head;
					sWingItem_Send.CheckSum[ItemCount] = pItem->sItemInfo.ItemHeader.dwChkSum;
					ItemCount++;
					break;
				}
			}
		}
	}


	if(CodeCount == ItemCount && sinChar->Money >= sWingQuest[Index].Money){
		WingItemQuestIndex = Index;
		sWingItem_Send.Index = Index;
		sWingItem_Send.Money = sWingQuest[Index].Money;
		cSinHelp.sinShowHelp(SIN_HELP_KIND_QUEST_WING,QuestMessageBoxPosi2.x,QuestMessageBoxPosi2.y,QuestMessageBoxSize2.x,QuestMessageBoxSize2.y, RGBA_MAKE(0,15,128,125),WingQuestFilePath_b[Index]);  
	}
	else{
		WingItemQuestIndex = 1000; //받을아이템이 없을 경우에대한 임의 셋팅
		cSinHelp.sinShowHelp(SIN_HELP_KIND_QUEST_WING,QuestMessageBoxPosi2.x,QuestMessageBoxPosi2.y,QuestMessageBoxSize2.x,QuestMessageBoxSize2.y, RGBA_MAKE(0,15,128,125),WingQuestFilePath_a[Index]);  
	}

	return TRUE;
}

//착용할수있는 아이템인지를 한번더 체크한다 
int cINVENTORY::CheckRequireItemToSet2(sITEM *pItem)
{
	int NotUseItemFlag = 0;

	if(pItem->sItemInfo.Level     > sinChar->Level )NotUseItemFlag = 1;
	if(pItem->sItemInfo.Dexterity > sinChar->Dexterity)NotUseItemFlag = 1;
	if(pItem->sItemInfo.Strength  > sinChar->Strength)NotUseItemFlag = 1;
	if(pItem->sItemInfo.Talent    > sinChar->Talent)NotUseItemFlag = 1;
	if(pItem->sItemInfo.Spirit    > sinChar->Spirit)NotUseItemFlag = 1;
	if(pItem->sItemInfo.Health    > sinChar->Health)NotUseItemFlag = 1;
	if((pItem->sItemInfo.CODE & sinITEM_MASK2) == sinFO1){
		int i; for( i = 0; i < 12 ; i++){ 
			if((pItem->CODE&sinITEM_MASK3)== SheltomCode2[i]){
				break;
			}
		}
		if(sinChar->Level < ForceOrbUseLevel[i][0] ||
			sinChar->Level > ForceOrbUseLevel[i][1]){
			NotUseItemFlag = 1;
		}
		else{
			NotUseItemFlag = 0;

		}

	}

	if(NotUseItemFlag){
		return FALSE;
	}
	return TRUE;
}

//스페셜아이템 소팅
int cINVENTORY::CheckSpecialItemPosi()
{
	int j = 0;
	for(int i=0 ; i < 84 ; i++){ 
		if(InvenItem[i].Flag){ 
			if(InvenItem[i].sItemInfo.SpecialItemFlag[1] == ITEM_KIND_SPECIAL){
				for( j = 84 ; j < INVENTORY_MAXITEM ; j++){
					if(!InvenItem[j].Flag){
						memcpy(&InvenItem[j],&InvenItem[i],sizeof(sITEM));
						memset(&InvenItem[i],0,sizeof(sITEM));

					}
				}
			}
		}
		if(InvenItemTemp[i].Flag){ 
			if(InvenItemTemp[i].sItemInfo.SpecialItemFlag[1] == ITEM_KIND_SPECIAL){
				for( j = 84 ; j < INVENTORY_MAXITEM ; j++){
					if(!InvenItem[j].Flag){
						memcpy(&InvenItem[j],&InvenItemTemp[i],sizeof(sITEM));
						memset(&InvenItemTemp[i],0,sizeof(sITEM));

					}
				}
			}
		}

	}
	ReFormInvenItem();
	ReformCharForm();//재인증 
	ReFormPotionNum();
	SetItemToChar();
	return TRUE;
}

//스페셜아이템을 보여준다
int cINVENTORY::ChangeSpecialItem(int Flag)
{
	if(Flag==1){ //AB버튼으로 아이템 앞뒤가 바뀔때 스페셜아이템창이 떠있으면 바꿔준다
		if(SpecialItemShowFlag){
			SpecialItemShowFlag = 0;

		}
		else return FALSE;
	}
	else if (Flag ==2){ //아이템을 입수할때나 거래할때 창고를 열때 상점을 열때 등등 창을 바꾼다
		SpecialItemShowFlag = 0;

	}
	else SpecialItemShowFlag ^=1;


	//스페셜아이템의 좌표를 이동시킨다
	for(int i=0 ; i < INVENTORY_MAXITEM ; i++){ 
		if(InvenItem[i].Flag){ 
			if(SpecialItemShowFlag){ //특별한 아이템의 디스플레이를 다르게한다
				if(InvenItem[i].sItemInfo.SpecialItemFlag[1] == ITEM_KIND_SPECIAL){
					if(InvenItem[i].x > 10000){
						InvenItem[i].x -= 10000;
					}
				}
				else {
					if(!InvenItem[i].ItemPosition && InvenItem[i].x < 10000){
							InvenItem[i].x += 10000;
					}
				}
			}
			else{
				if(InvenItem[i].sItemInfo.SpecialItemFlag[1] == ITEM_KIND_SPECIAL){
					if(InvenItem[i].x < 10000){
						InvenItem[i].x += 10000;
					}
				}
				else{
					if(!InvenItem[i].ItemPosition){
						if(InvenItem[i].x > 10000){
							InvenItem[i].x -= 10000;
						}
					}
				}
			}
		}
	}
	return TRUE;
}
//코드나 아이템 종류로 인벤토리 아이템 인덱스를 리턴한다
int cINVENTORY::SearchItemIndex(DWORD CODE , int KindFlag)
{
	int i = 0; 
	if(CODE){
		for(i = 0; i < INVENTORY_MAXITEM ; i++){
			if(InvenItem[i].Flag){
				if(InvenItem[i].CODE == CODE){
					return i+1;

				}
			}
		}
		for(i = 0; i < INVENTORY_MAXITEM ; i++){
			if(InvenItemTemp[i].Flag){
				if(InvenItemTemp[i].CODE == CODE){
					return 100+i+1;
				}
			}
		}
	}

	if(KindFlag){
		for(int i = 0; i < INVENTORY_MAXITEM ; i++){
			if(InvenItem[i].Flag){
				if(InvenItem[i].sItemInfo.ItemKindCode == KindFlag){
					return i+1;

				}
			}
			if(InvenItemTemp[i].Flag){
				if(InvenItemTemp[i].sItemInfo.ItemKindCode == KindFlag){
					return 100+i+1;
					
				}
			}
		}
	}
	return FALSE;
}

//박재원 - 수박 모아오기
int WatermelonEventItemIndex[INVENTORY_MAXITEM*2] = {0,};

//장별 - 발렌타인 초콜렛 모아오기
int ValentineEventItemIndex[INVENTORY_MAXITEM*2] = {0,};

// 장별 - 캔디데이즈
int CandydaysEventItemIndex[INVENTORY_MAXITEM*2] = {0,};

// 장별 - 매지컬그린
int MagicalGreenEventItemIndex[INVENTORY_MAXITEM*2] = {0,}; 
int MagicalGreenEmeraldEventItemIndex[INVENTORY_MAXITEM*2] = {0,}; 

// 장별 - 카라의 눈물
int TearOfKaraEventItemIndex[INVENTORY_MAXITEM*2] = {0,};

// 장별 - 조사원을 찾아라 
int FindinvestigatorNineEventItemIndex[INVENTORY_MAXITEM*2] = {0,}; // 나인 아뮬렛
int FindinvestigatorTaleEventItemIndex[INVENTORY_MAXITEM*2] = {0,}; // 테일 아뮬렛

// 박재원 - 같은 아이템의 갯수를 리턴한다.(인벤에서 수박 갯수 리턴하기)
int cINVENTORY::CheckItemCount(DWORD CODE)
{
	int i = 0; 
	int item_count = 0;
	if(CODE)
	{
		for(i = 0; i < INVENTORY_MAXITEM ; i++)
		{
			if(InvenItem[i].Flag)
			{
				if(InvenItem[i].CODE == CODE)
				{
					item_count++;
					WatermelonEventItemIndex[item_count-1] = i+1;
				}
			}
		}

		for(i = 0; i < INVENTORY_MAXITEM ; i++)
		{
			if(InvenItemTemp[i].Flag)
			{
				if(InvenItemTemp[i].CODE == CODE)
				{
					item_count++;
					WatermelonEventItemIndex[item_count-1] = 100+i+1;
				}
			}
		}
	}

	return item_count;
}


// 장별 - 같은 아이템의 갯수를 리턴한다.(인벤에서 발렌타인 초콜렛 갯수 리턴하기)
int cINVENTORY::CheckValentineItemCount(DWORD CODE) 
{
	int i = 0; 
	int item_count = 0;
	if(CODE)
	{
		for(i = 0; i < INVENTORY_MAXITEM ; i++)
		{
			if(InvenItem[i].Flag)
			{
				if(InvenItem[i].CODE == CODE)
				{
					item_count++;
					ValentineEventItemIndex[item_count-1] = i+1;
				}
			}
		}

		for(i = 0; i < INVENTORY_MAXITEM ; i++)
		{
			if(InvenItemTemp[i].Flag)
			{
				if(InvenItemTemp[i].CODE == CODE)
				{
					item_count++;
					ValentineEventItemIndex[item_count-1] = 100+i+1;
				}
			}
		}
	}

	return item_count;
}


// 장별 - 캔디데이즈 같은 아이템의 갯수를 리턴한다.(인벤에서 캔디 갯수 리턴하기)
int cINVENTORY::CheckCandyItemCount(DWORD CODE) 
{
	int i = 0; 
	int item_count = 0;
	if(CODE)
	{
		for(i = 0; i < INVENTORY_MAXITEM ; i++)
		{
			if(InvenItem[i].Flag)
			{
				if(InvenItem[i].CODE == CODE)
				{
					item_count++;
					CandydaysEventItemIndex[item_count-1] = i+1;
				}
			}
		}

		for(i = 0; i < INVENTORY_MAXITEM ; i++)
		{
			if(InvenItemTemp[i].Flag)
			{
				if(InvenItemTemp[i].CODE == CODE)
				{
					item_count++;
					CandydaysEventItemIndex[item_count-1] = 100+i+1;
				}
			}
		}
	}

	return item_count;
}


// 장별 - 매지컬그린
int cINVENTORY::CheckMagicalGreenItemCount(DWORD CODE) 
{
	int i = 0; 
	int item_count = 0;
	if(CODE)
	{
		for(i = 0; i < INVENTORY_MAXITEM ; i++)
		{
			if(InvenItem[i].Flag)
			{
				if(InvenItem[i].CODE == CODE)
				{
					item_count++;
					MagicalGreenEventItemIndex[item_count-1] = i+1;
				}
			}
		}

		for(i = 0; i < INVENTORY_MAXITEM ; i++)
		{
			if(InvenItemTemp[i].Flag)
			{
				if(InvenItemTemp[i].CODE == CODE)
				{
					item_count++;
					MagicalGreenEventItemIndex[item_count-1] = 100+i+1;
				}
			}
		}
	}

	return item_count;
}

// 장별 - 매지컬그린
int cINVENTORY::CheckMagicalGreenItemEmeraldCount(DWORD CODE) 
{
	int i = 0; 
	int item_count = 0;
	if(CODE)
	{
		for(i = 0; i < INVENTORY_MAXITEM ; i++)
		{
			if(InvenItem[i].Flag)
			{
				if(InvenItem[i].CODE == CODE)
				{
					item_count++;
					MagicalGreenEmeraldEventItemIndex[item_count-1] = i+1;
				}
			}
		}

		for(i = 0; i < INVENTORY_MAXITEM ; i++)
		{
			if(InvenItemTemp[i].Flag)
			{
				if(InvenItemTemp[i].CODE == CODE)
				{
					item_count++;
					MagicalGreenEmeraldEventItemIndex[item_count-1] = 100+i+1;
				}
			}
		}
	}

	return item_count;
}


// 장별 - 카라의 눈물 - 이벤트 아이템 갯수 리턴
int cINVENTORY::CheckTearOfKaraItemCount(DWORD CODE) 
{
	int i = 0; 
	int item_count = 0;
	if(CODE)
	{
		for(i = 0; i < INVENTORY_MAXITEM ; i++)
		{
			if(InvenItem[i].Flag)
			{
				if(InvenItem[i].CODE == CODE)
				{
					item_count++;
					TearOfKaraEventItemIndex[item_count-1] = i+1;
				}
			}
		}

		for(i = 0; i < INVENTORY_MAXITEM ; i++)
		{
			if(InvenItemTemp[i].Flag)
			{
				if(InvenItemTemp[i].CODE == CODE)
				{
					item_count++;
					TearOfKaraEventItemIndex[item_count-1] = 100+i+1;
				}
			}
		}
	}

	return item_count;
}


// 장별 - 조사원을 찾아라 - 나인 아뮬렛 갯수 리턴
int cINVENTORY::CheckFindinvestigatorNineItemCount(DWORD CODE) 
{
	int i = 0; 
	int item_count = 0;
	if(CODE)
	{
		for(i = 0; i < INVENTORY_MAXITEM ; i++)
		{
			if(InvenItem[i].Flag)
			{
				if(InvenItem[i].CODE == CODE)
				{
					item_count++;
					FindinvestigatorNineEventItemIndex[item_count-1] = i+1;
				}
			}
		}

		for(i = 0; i < INVENTORY_MAXITEM ; i++)
		{
			if(InvenItemTemp[i].Flag)
			{
				if(InvenItemTemp[i].CODE == CODE)
				{
					item_count++;
					FindinvestigatorNineEventItemIndex[item_count-1] = 100+i+1;
				}
			}
		}
	}

	return item_count;
}


// 장별 - 조사원을 찾아라 - 테일 아뮬렛 갯수 리턴
int cINVENTORY::CheckFindinvestigatorTaleItemCount(DWORD CODE) 
{
	int i = 0; 
	int item_count = 0;
	if(CODE)
	{
		for(i = 0; i < INVENTORY_MAXITEM ; i++)
		{
			if(InvenItem[i].Flag)
			{
				if(InvenItem[i].CODE == CODE)
				{
					item_count++;
					FindinvestigatorTaleEventItemIndex[item_count-1] = i+1;
				}
			}
		}

		for(i = 0; i < INVENTORY_MAXITEM ; i++)
		{
			if(InvenItemTemp[i].Flag)
			{
				if(InvenItemTemp[i].CODE == CODE)
				{
					item_count++;
					FindinvestigatorTaleEventItemIndex[item_count-1] = 100+i+1;
				}
			}
		}
	}

	return item_count;
}

//복주머니에서 퍼즐로 변경한다
int cINVENTORY::ChangePuzzle(sITEM *pItem)
{
	char szTestBuff2[256];
	szTestBuff2[0] = 0; //버퍼초기화  
	if((pItem->sItemInfo.CODE & sinITEM_MASK2) == sinPZ1 || (pItem->sItemInfo.CODE & sinITEM_MASK2) == sinPZ2){
		if(pItem->sItemInfo.PotionCount <= 1){
			pItem->sItemInfo.PotionCount = 2;
			for(int j=0 ; j < MAX_ITEM ; j++){ 
				if(pItem->sItemInfo.CODE == sItem[j].CODE){	
					CheckInvenItemForm(); 
					//memcpy(pItem,&sItem[j],sizeof(sITEM));
					wsprintf(szTestBuff2 ,"Image\\sinImage\\Items\\%s\\it%s.bmp",sItem[j].ItemFilePath,sItem[j].LastCategory);
					sItem[j].lpTempItem = LoadDibSurfaceOffscreen(szTestBuff2);
					pItem->lpItem = sItem[j].lpTempItem;
					pItem->w = sItem[j].w;
					pItem->h = sItem[j].h;
					pItem->CODE = sItem[j].CODE;   //개인 상점을 이용을 위해 코드도 넘겨준다.
					CheckWeight();//무게 보정 
					ReFormInvenItem();
					sinMinusMoney(1000);
					ReformCharForm();//재인증 
					SendSaveMoney(); //금액 조작을 못하게하기위해 호출한다 
					sinUsePotionDelayFlag = 1;
					cInvenTory.UnionCoreParticle2(pItem->x , pItem->y,1); //이펙트 
					cInvenTory.UnionCoreParticle2(pItem->x , pItem->y,1); //이펙트 
					cInvenTory.UnionCoreParticle2(pItem->x , pItem->y,1); //이펙트 
//					SetPuzzleEffect(pItem->x , pItem->y); //이펙트 
					sinPlaySound(SIN_SOUND_EAT_POTION2); //유니온코어 사운드
					SaveGameData();
					return TRUE;
				}
			}
		}
	}
	return FALSE;
}

DWORD CheckPuzzleIndex[4][4] = {
	{sin01,0,sin02,0},
	{sin03,sin04,0,sin06},
	{0,sin05,0,0},
	{sin07,0,sin08,0},
};

// 박재원 - 일본 퍼즐 이벤트(퍼즐 대형 변경)
DWORD CheckPuzzleIndex2[4][4] = {
	{sin01,0,sin02,0},
	{sin03,sin04,0,sin06},
	{0,sin05,0,0},
	{sin07,0,sin08,0},
};
/*
DWORD CheckPuzzleIndex2[4][4] = {
	{sin01,0,sin03,sin04},
	{sin02,0,0,0},
	{sin05,sin06,sin07,0},
	{0,0,sin08,0},
};
*/
DWORD CheckPuzzleX[4] = {8,9,10,11};
int   PuzzleIndex[8] = {0,};

//퍼즐을 체크한다
int cINVENTORY::CheckPuzzle()
{
	int Count = 0;
	int Count2 = 0;
	int i,j,k;
	PuzzleOkFlag = 0;
	for(int p = 0 ; p < 8 ; p++){
		PuzzleIndex[p] = 0;

	}

	for(i = 0 ; i < INVENTORY_MAXITEM ; i++){  
		if(InvenItem[i].Flag){
			if((InvenItem[i].sItemInfo.CODE & sinITEM_MASK2) == sinPZ1){
				for(j = 0; j < 4 ; j++){
					for(k = 0; k < 4 ; k++){
						if((InvenItem[i].sItemInfo.CODE & sinITEM_MASK3) == CheckPuzzleIndex[j][k]){
							if((StartX+(CheckPuzzleX[k]*22)) == InvenItem[i].x &&
								(StartY+(j*22)) == InvenItem[i].y){
								PuzzleIndex[Count] = i+1;
								Count++;

							}
						}
					}
				}
			}

			if((InvenItem[i].sItemInfo.CODE & sinITEM_MASK2) == sinPZ2){
				for(j = 0; j < 4 ; j++){
					for(k = 0; k < 4 ; k++){
						if((InvenItem[i].sItemInfo.CODE & sinITEM_MASK3) == CheckPuzzleIndex2[j][k]){
							if((StartX+(CheckPuzzleX[k]*22)) == InvenItem[i].x &&
								(StartY+(j*22)) == InvenItem[i].y){
								PuzzleIndex[Count2] = i+1;
								Count2++;

							}
						}
					}
				}
			}
		}
	}
	if(Count == 8 || Count2 == 8){
		//if(!PuzzleOkFlag)
		//	cMessageBox.ShowMessage(MESSAGE_PERPFECT_PUZZLE);
		PuzzleOkFlag = 1;

	}
	return TRUE;
}


//바벨의 뿔을 체크한다
int cINVENTORY::CheckEventItem()
{
	
	int ItemIndex = SearchItemIndex((sinGF1|sin02),0);
	if(ItemIndex && ItemIndex < 100 ){
		cInterFace.CheckAllBox(SIN_INVENTORY);
		cMessageBox.ShowMessage3(MESSAGE_EVENTITEM_SEND,BabelHorn);
		return TRUE;
	}
	else		
		cSinHelp.sinShowHelp(SIN_HELP_KIND_BABEL,QuestMessageBoxPosi2.x,QuestMessageBoxPosi2.y,QuestMessageBoxSize2.x,QuestMessageBoxSize2.y, RGBA_MAKE(0,15,128,125),BabelEventDoc);  
		cMessageBox.ShowMessage3(MESSAGE_FALLGAME,FallGameName);	
	return TRUE;
}

//바벨의 뿔을 보낸다
int cINVENTORY::SendEventItem()
{
	int Count = 0; 
	int ItemIndex = 0;
	sITEM TempItem;
	TempItem.w = 22*2;
	TempItem.h = 22*2;


	memset(&sWingItem_Send,0,sizeof(sCRAFTITEM_SERVER)); //초기화

	ItemIndex = SearchItemIndex((sinGF1|sin02),0);
	if(ItemIndex && ItemIndex < 100 ){
		if(sinChar->Weight[0]+30 > sinChar->Weight[1]){
			cMessageBox.ShowMessage(MESSAGE_OVER_WEIGHT);
			return FALSE;

		}		//여기서 가상의 공간체크를해본다
		if(!cCraftItem.CraftCheckEmptyArea(&TempItem)){
			cMessageBox.ShowMessage(MESSAGE_OVER_SPACE);
			return FALSE;
		}
		sWingItem_Send.SheltomCode[0] = InvenItem[ItemIndex-1].sItemInfo.CODE;
		sWingItem_Send.Head[0] = InvenItem[ItemIndex-1].sItemInfo.ItemHeader.Head;
		sWingItem_Send.CheckSum[0] = InvenItem[ItemIndex-1].sItemInfo.ItemHeader.dwChkSum;
	}

	sWingItem_Send.DocIndex =2;
	return TRUE;	

}

//나인테일 아물렛을 체크한다
int cINVENTORY::CheckFoxEventItem()
{
	
	int ItemIndex = SearchItemIndex((sinGF1|sin03),0);
	int ItemIndex2 = SearchItemIndex((sinGF1|sin04),0);

	if(ItemIndex && ItemIndex < 100 && ItemIndex2 && ItemIndex2 < 100){
		cInterFace.CheckAllBox(SIN_INVENTORY);
		cMessageBox.ShowMessage3(MESSAGE_FOXEVENTITEM_SEND,NineTailFoxItem);
	}
	else
		cSinHelp.sinShowHelp(SIN_HELP_KIND_BABEL,QuestMessageBoxPosi2.x,QuestMessageBoxPosi2.y,QuestMessageBoxSize2.x,QuestMessageBoxSize2.y, RGBA_MAKE(0,15,128,125),NineTailFoxDoc);  
	return TRUE;
}

//나인테일 아물렛을 보낸다
int cINVENTORY::SendFoxEventItem()
{
	int Count = 0; 
	int ItemIndex = 0;
	int ItemIndex2 = 0;
	sITEM TempItem;
	TempItem.w = 22*3;
	TempItem.h = 22*4;


	memset(&sWingItem_Send,0,sizeof(sCRAFTITEM_SERVER)); //초기화

	ItemIndex = SearchItemIndex((sinGF1|sin03),0);
	ItemIndex2 = SearchItemIndex((sinGF1|sin04),0);
	if(ItemIndex && ItemIndex < 100 && ItemIndex2 && ItemIndex2 < 100){
		if(sinChar->Weight[0]+100 > sinChar->Weight[1]){
			cMessageBox.ShowMessage(MESSAGE_OVER_WEIGHT);
			return FALSE;

		}
		//여기서 가상의 공간체크를해본다
		if(!cCraftItem.CraftCheckEmptyArea(&TempItem)){
			cMessageBox.ShowMessage(MESSAGE_OVER_SPACE);
			return FALSE;
		}
		sWingItem_Send.SheltomCode[0] = InvenItem[ItemIndex-1].sItemInfo.CODE;
		sWingItem_Send.Head[0] = InvenItem[ItemIndex-1].sItemInfo.ItemHeader.Head;
		sWingItem_Send.CheckSum[0] = InvenItem[ItemIndex-1].sItemInfo.ItemHeader.dwChkSum;

		sWingItem_Send.SheltomCode[1] = InvenItem[ItemIndex2-1].sItemInfo.CODE;
		sWingItem_Send.Head[1] = InvenItem[ItemIndex2-1].sItemInfo.ItemHeader.Head;
		sWingItem_Send.CheckSum[1] = InvenItem[ItemIndex2-1].sItemInfo.ItemHeader.dwChkSum;

	}
	sWingItem_Send.DocIndex = 2;
	return TRUE;	

}


//반짝반짝 가루를 체크한다
int cINVENTORY::CheckChristMasEventItem()
{
	
	 int ItemIndex = SearchItemIndex((sinGF1|sin05),0);

	 if(ItemIndex && ItemIndex < 100  ){
		 cInterFace.CheckAllBox(SIN_INVENTORY);
//		 cMessageBox.ShowMessage3(MESSAGE_CHRISTMASITEM_SEND, ChristMas );
	 }
	 else
		cSinHelp.sinShowHelp(SIN_HELP_KIND_BABEL,QuestMessageBoxPosi2.x,QuestMessageBoxPosi2.y,QuestMessageBoxSize2.x,QuestMessageBoxSize2.y, RGBA_MAKE(0,15,128,125),StarDustDoc);  
	 return TRUE;
}

//반짝반짝 가루를 보낸다
int cINVENTORY::SendChristmasEventItem()
{
	int Count = 0; 
	int ItemIndex = 0;

	sITEM TempItem;
	TempItem.w = 22*3;
	TempItem.h = 22*4;


	memset(&sWingItem_Send,0,sizeof(sCRAFTITEM_SERVER)); //초기화
	
	ItemIndex = SearchItemIndex((sinGF1|sin05),0);	
	
	if(ItemIndex && ItemIndex < 100 ){
		if(sinChar->Weight[0]+100 > sinChar->Weight[1]){
			cMessageBox.ShowMessage(MESSAGE_OVER_WEIGHT);
			return FALSE;

		}
		//여기서 가상의 공간체크를해본다
		if(!cCraftItem.CraftCheckEmptyArea(&TempItem)){
			cMessageBox.ShowMessage(MESSAGE_OVER_SPACE);
			return FALSE;
		}
		sWingItem_Send.SheltomCode[0] = InvenItem[ItemIndex-1].sItemInfo.CODE;
		sWingItem_Send.Head[0] = InvenItem[ItemIndex-1].sItemInfo.ItemHeader.Head;
		sWingItem_Send.CheckSum[0] = InvenItem[ItemIndex-1].sItemInfo.ItemHeader.dwChkSum;

	}
	sWingItem_Send.DocIndex = 2;
	return TRUE;	

	
}
//퍼즐을 보낸다
int cINVENTORY::CheckSendPuzzle()
{
	int Count = 0; 
	memset(&sWingItem_Send,0,sizeof(sCRAFTITEM_SERVER)); //초기화
	for(int i = 0; i < 8 ; i++){
		if(PuzzleIndex[i] && InvenItem[PuzzleIndex[i]-1].Flag){
			if((InvenItem[PuzzleIndex[i]-1].sItemInfo.CODE & sinITEM_MASK2) == sinPZ1 ||
				(InvenItem[PuzzleIndex[i]-1].sItemInfo.CODE & sinITEM_MASK2) == sinPZ2){
				sWingItem_Send.SheltomCode[i] = InvenItem[PuzzleIndex[i]-1].sItemInfo.CODE;
				sWingItem_Send.Head[i] = InvenItem[PuzzleIndex[i]-1].sItemInfo.ItemHeader.Head;
				sWingItem_Send.CheckSum[i] = InvenItem[PuzzleIndex[i]-1].sItemInfo.ItemHeader.dwChkSum;
				Count++;
			}
		}
	}
	if(Count == 8){
		////////퍼즐 코드 소팅
		DWORD Temp=0;
		for(int j = 0; j < 8 ; j++){
			for(int k = 0; k < 8; k++){
				if(j == k )continue;
				if(sWingItem_Send.SheltomCode[j] == sWingItem_Send.SheltomCode[k]){
					cMessageBox.ShowMessage(MESSAGE_FAILD_PUZZLE);
					return FALSE;

				}
			}
		}
	}
	else{
		cMessageBox.ShowMessage(MESSAGE_FAILD_PUZZLE);
		return FALSE;
	}
	if(sinChar->Weight[0]+120 > sinChar->Weight[1]){		//퍼즐아이템 인벤토리 무게(현재120) (puzzle)
		cMessageBox.ShowMessage(MESSAGE_OVER_WEIGHT);
		return FALSE;

	}
	sWingItem_Send.DocIndex = 1;
	return TRUE;	
}
	
int cINVENTORY::SendPuzzleNpc()
{
	
	if(PuzzleOkFlag){  
		cInterFace.CheckAllBox(SIN_INVENTORY);
		cMessageBox.ShowMessage3(MESSAGE_PUZZLE_SEND,PuzzleEvent5);
	}
	else{
		WingItemQuestIndex = 1000; //윙아이템을 그대로 델꾸왔다 하하핫 
		cSinHelp.sinShowHelp(SIN_HELP_KIND_QUEST_WING,QuestMessageBoxPosi2.x,QuestMessageBoxPosi2.y,QuestMessageBoxSize2.x,QuestMessageBoxSize2.y, RGBA_MAKE(0,15,128,125),WingQuestFilePath_a[6]);  //WingQuestFilePath_a[5]);  //원래값 //해외
	}
	return TRUE;
}

//보여지는 데미지를 조정한다 
int cINVENTORY::IncreViewDamage(int Persent , int Num)
{
	ViewDamagePercent = Persent;
	ViewDamageNum = Num;
	SetItemToChar();
	return TRUE;
}

//ForceOrb의 이펙트를 그린다
int cINVENTORY::DrawForceOrbEffect(int x , int y)
{ 
	//이미지 로드
	
	int i = 0;   
	char *Path = "image\\Sinimage\\Effect\\ForceOrb\\P_0%d.tga";      
	char szBuff[128];
	if(!MatForceOrbEffect[0]){
		for( i = 0; i < 8; i++){
			wsprintf(szBuff,Path,i+1);
			MatForceOrbEffect[i] = CreateTextureMaterial(szBuff, 0, 0, 0,0, SMMAT_BLEND_ALPHA );
			  
		}
		ReadTextures();	//텍스쳐 로딩
	}
	

	//ForceOrbEffectIndex
	ForceOrb_Effect.InvenFace.MatNum = MatForceOrbEffect[ForceOrbEffectIndex];          
	ForceOrb_Effect.InvenFace.x = x-5; 
	ForceOrb_Effect.InvenFace.y = y-5;
	ForceOrb_Effect.InvenFace.r = 255;
	ForceOrb_Effect.InvenFace.g = 255;
	ForceOrb_Effect.InvenFace.b = 255;
	ForceOrb_Effect.InvenFace.Transparency = 255;
	ForceOrb_Effect.InvenFace.height = 32;
	ForceOrb_Effect.InvenFace.width = 32;
	ForceOrb_Effect.InvenFace.TexRect = sinTexRect;

	sinDrawInvenTexImage(&ForceOrb_Effect.InvenFace);
	return TRUE;
}

//sITEM UseForceOrbBak;
int cINVENTORY::UseForceOrb(sITEM *pItem , int Flag)
{

	if(lpCurPlayer->OnStageField>=0 && StageField[ lpCurPlayer->OnStageField ]->State == FIELD_STATE_VILLAGE)
		return FALSE; //마을에서는 못사용한다

	//if(!Flag && sinItemPickUpDelayFlag)return FALSE; //무한에테르 코어를 막는다    
	if(sinItemPickUpDelayFlag)return FALSE; //무한에테르 코어를 막는다    
	int UseFlag = 0;
	sSKILL TempSkill;
	memset(&TempSkill,0,sizeof(sSKILL));

	int i = 0;
	if(!smConfig.DebugMode){
		for(i = 0; i < 12 ; i++){ 
			if((pItem->CODE&sinITEM_MASK3)== SheltomCode2[i]){
				if(sinChar->Level >= ForceOrbUseLevel[i][0] &&
					sinChar->Level <= ForceOrbUseLevel[i][1]){
						UseFlag = i+1;
				}
			}
		}
		
		if(!UseFlag){
			cMessageBox.ShowMessage(MESSAGE_NOT_USE_FORCEORB);
			return FALSE;

		}
	}	
	else{
		for(i = 0; i < 12 ; i++){ 
			if((pItem->CODE&sinITEM_MASK3)== SheltomCode2[i]){
					UseFlag = i+1;

			}
		}
	}

	if(cSkill.SearchContiueSkill(SKILL_FORCE_ORB)){
		cMessageBox.ShowMessage(MESSAGE_NOT_USE_FORCEORB2);
		return FALSE;
	}
	
	/*
	if(!Flag){
		memcpy(&UseForceOrbBak,pItem,sizeof(sITEM));
		cMessageBox.ShowMessage3(MESSAGE_USE_FORCEORB,pItem->sItemInfo.ItemName);
		return TRUE;
	}
	*/

	//텍스쳐 로딩
	char *Path = "image\\Sinimage\\skill\\ForceOrb\\FOS_0%d.tga";   // 박재원 : 빌링 포스(포스 아이콘 10부터 12추가)
	char szBuff[128];
	if(!MatForceOrb[UseFlag-1]){
		wsprintf(szBuff,Path,UseFlag);
		MatForceOrb[UseFlag-1] = CreateTextureMaterial(szBuff, 0, 0, 0,0, SMMAT_BLEND_ALPHA );
		ReadTextures();	//텍스쳐 로딩
	}

	TempSkill.CODE = SKILL_FORCE_ORB;
	TempSkill.Point = UseFlag;
	TempSkill.UseTime = ForceOrbUseTime[UseFlag-1];
	TempSkill.MatIcon = MatForceOrb[UseFlag-1];
	sinContinueSkillSet(&TempSkill);
	pItem->sItemInfo.PotionCount = 100;
	sinThrowItemToFeild(pItem);
	pItem->Flag = 0;

	cInvenTory.SetItemToChar(); //아이템이 셋팅되면 능력치를 셋팅한다 
	cInvenTory.ReFormInvenItem();
	cInvenTory.CheckWeight();   //무게를 셋팅한다 
	sinUsePotionDelayFlag = 1;
	sinPlaySound(SIN_SOUND_EAT_POTION2);//대박 사운드
	return TRUE;
}

// 박재원 - 매직 포스 추가(매직 포스를 사용한다)
int cINVENTORY::UseMagicForceOrb(sITEM *pItem , int Flag)
{
	if(lpCurPlayer->OnStageField>=0 && StageField[ lpCurPlayer->OnStageField ]->State == FIELD_STATE_VILLAGE)
		return FALSE; //마을에서는 못사용한다

	//if(!Flag && sinItemPickUpDelayFlag)return FALSE; //무한에테르 코어를 막는다    
	if(sinItemPickUpDelayFlag)return FALSE; //무한에테르 코어를 막는다    
	int UseFlag = 0;
	sSKILL TempSkill;
	memset(&TempSkill,0,sizeof(sSKILL));

	int i = 0;
	if(!smConfig.DebugMode){
		for(i = 0; i < 12 ; i++){ 
			if((pItem->CODE&sinITEM_MASK3)== MagicSheltomCode[i]){
				if(sinChar->Level >= ForceOrbUseLevel[i][0] &&
					sinChar->Level <= ForceOrbUseLevel[i][1]){
						UseFlag = i+1;
					}
			}
		}

		if(!UseFlag){
			cMessageBox.ShowMessage(MESSAGE_NOT_USE_FORCEORB);
			return FALSE;

		}
	}	
	else{
		for(i = 0; i < 12 ; i++){ 
			if((pItem->CODE&sinITEM_MASK3)== MagicSheltomCode[i]){
				UseFlag = i+1;

			}
		}
	}

	if(cSkill.SearchContiueSkill(SKILL_FORCE_ORB)){
		cMessageBox.ShowMessage(MESSAGE_NOT_USE_FORCEORB2);
		return FALSE;
	}

	//텍스쳐 로딩
	char *Path = "image\\Sinimage\\skill\\ForceOrb\\FOS_0%d.tga";   // 박재원 - 매직 포스(포스 아이콘 21부터 32추가)
	char szBuff[128];
	if(!MatForceOrb[UseFlag-1]){
		wsprintf(szBuff,Path,UseFlag);
		MatForceOrb[UseFlag-1] = CreateTextureMaterial(szBuff, 0, 0, 0,0, SMMAT_BLEND_ALPHA );
		ReadTextures();	//텍스쳐 로딩
	}

	TempSkill.CODE = SKILL_FORCE_ORB;
	TempSkill.Point = UseFlag;
	TempSkill.ElementIndex = 2000; // 박재원 - 매직 포스 구분을 위함.(매직 포스 추가)
	TempSkill.UseTime = MagicForceOrbUseTime[UseFlag-1];
	TempSkill.MatIcon = MatForceOrb[UseFlag-1];
	sinContinueSkillSet(&TempSkill);
	pItem->sItemInfo.PotionCount = 100;
	sinThrowItemToFeild(pItem);
	pItem->Flag = 0;

	cInvenTory.SetItemToChar(); //아이템이 셋팅되면 능력치를 셋팅한다 
	cInvenTory.ReFormInvenItem();
	cInvenTory.CheckWeight();   //무게를 셋팅한다 
	sinUsePotionDelayFlag = 1;
	sinPlaySound(SIN_SOUND_EAT_POTION2);//대박 사운드
	return TRUE;
}

// 박재원 - 빌링 매직 포스 추가(빌링 매직 포스를 사용한다)
int cINVENTORY::UseBillingMagicForceOrb(sITEM *pItem , int Flag)
{
	if(lpCurPlayer->OnStageField>=0 && StageField[ lpCurPlayer->OnStageField ]->State == FIELD_STATE_VILLAGE)
		return FALSE; //마을에서는 못사용한다

	//if(!Flag && sinItemPickUpDelayFlag)return FALSE; //무한에테르 코어를 막는다    
	if(sinItemPickUpDelayFlag)return FALSE; //무한에테르 코어를 막는다    
	int UseFlag = 0;
	sSKILL TempSkill;
	memset(&TempSkill,0,sizeof(sSKILL));

	int i = 0;
	if(!smConfig.DebugMode)
	{
		for(i = 0; i < 3 ; i++)
		{ 
			if((pItem->CODE&sinITEM_MASK3)== BillingMagicSheltomCode[i])
			{
				if(sinChar->Level >= BillingMagicForceOrbUseLevel[i][0] && sinChar->Level <= BillingMagicForceOrbUseLevel[i][1])
				{
					UseFlag = i+1;
				}	
			}
		}

		if(!UseFlag){
			cMessageBox.ShowMessage(MESSAGE_NOT_USE_FORCEORB);
			return FALSE;

		}
	}	
	else{
		for(i = 0; i < 3 ; i++){ 
			if((pItem->CODE&sinITEM_MASK3)== BillingMagicSheltomCode[i]){
				UseFlag = i+1;

			}
		}
	}

	if(cSkill.SearchContiueSkill(SKILL_FORCE_ORB)){
		cMessageBox.ShowMessage(MESSAGE_NOT_USE_FORCEORB2);
		return FALSE;
	}

	//텍스쳐 로딩
	char *Path = "image\\Sinimage\\skill\\ForceOrb\\FOS_0%d.tga";   // 박재원 : 빌링 매직 포스(포스 아이콘 35부터 37추가)
	char szBuff[128];
	if(!MatBillingMagicForceOrb[UseFlag-1]){
		wsprintf(szBuff,Path,UseFlag+34);
		MatBillingMagicForceOrb[UseFlag-1] = CreateTextureMaterial(szBuff, 0, 0, 0,0, SMMAT_BLEND_ALPHA );
		ReadTextures();	//텍스쳐 로딩
	}

	TempSkill.CODE = SKILL_FORCE_ORB;
	TempSkill.Point = UseFlag;
	TempSkill.ElementIndex = 1000; // 빌링 매직 포스 구분을 위함.
	TempSkill.UseTime = BillingMagicForceOrbUseTime[UseFlag-1];
	TempSkill.MatIcon = MatBillingMagicForceOrb[UseFlag-1];
	sinContinueSkillSet(&TempSkill);
	pItem->sItemInfo.PotionCount = 100;
	sinThrowItemToFeild(pItem);
	pItem->Flag = 0;

	cInvenTory.SetItemToChar(); //아이템이 셋팅되면 능력치를 셋팅한다 
	cInvenTory.ReFormInvenItem();
	cInvenTory.CheckWeight();   //무게를 셋팅한다 
	sinUsePotionDelayFlag = 1;
	sinPlaySound(SIN_SOUND_EAT_POTION2);//대박 사운드
	return TRUE;
}

// 박재원 - 매직 포스 추가 (매직 포스를 셋팅한다.)
int cINVENTORY::SetMagicForceOrb(DWORD ItemCODE , int Time)
{
	int UseFlag = 0; 
	sSKILL TempSkill;
	memset(&TempSkill,0,sizeof(sSKILL));

	for(int i = 0; i < 12 ; i++) // 매직 포스
	{ 
		if((ItemCODE&sinITEM_MASK3)== MagicSheltomCode[i]) // 매직 러씨디 포스 ~ 매직 이니그마 포스
		{
			UseFlag = i+1;
		}
	}

	char *Path = "image\\Sinimage\\skill\\ForceOrb\\FOS_0%d.tga";      
	char szBuff[128];
	if(!MatForceOrb[UseFlag-1]){
		wsprintf(szBuff,Path,UseFlag+20);
		MatForceOrb[UseFlag-1] = CreateTextureMaterial(szBuff, 0, 0, 0,0, SMMAT_BLEND_ALPHA );
		ReadTextures();	//텍스쳐 로딩
	}

	TempSkill.CODE = SKILL_FORCE_ORB;
	TempSkill.Point = UseFlag;
	TempSkill.ElementIndex = 2000;
	TempSkill.UseTime = MagicForceOrbUseTime[UseFlag-1];
	TempSkill.CheckTime = (MagicForceOrbUseTime[UseFlag-1]*70) - (Time*70);
	TempSkill.MatIcon = MatForceOrb[UseFlag-1];
	sinContinueSkillSet(&TempSkill);
	return TRUE;

}

// 박재원 - 빌링 매직 포스 추가 (빌링 매직 포스를 셋팅한다.)
int cINVENTORY::SetBillingMagicForceOrb(DWORD ItemCODE , int Time)
{
	int UseFlag = 0; 
	sSKILL TempSkill;
	memset(&TempSkill,0,sizeof(sSKILL));

	for(int i = 0; i < 3 ; i++) // 빌링 매직 포스
	{ 
		if((ItemCODE&sinITEM_MASK3)== BillingMagicSheltomCode[i]) // 빌링 매직 포스는 쉘텀을 사용하지 않지만 코드 번호를 위해 사용한다.
		{
			UseFlag = i+1;
		}
	}

	char *Path = "image\\Sinimage\\skill\\ForceOrb\\FOS_0%d.tga";      
	char szBuff[128];
	if(!MatForceOrb[UseFlag-1]){
		wsprintf(szBuff,Path,UseFlag+34);
		MatForceOrb[UseFlag-1] = CreateTextureMaterial(szBuff, 0, 0, 0,0, SMMAT_BLEND_ALPHA );
		ReadTextures();	//텍스쳐 로딩
	}

	TempSkill.CODE = SKILL_FORCE_ORB;
	TempSkill.Point = UseFlag;
	TempSkill.ElementIndex = 1000;
	TempSkill.UseTime = BillingMagicForceOrbUseTime[UseFlag-1];
	TempSkill.CheckTime = (BillingMagicForceOrbUseTime[UseFlag-1]*70) - (Time*70);
	TempSkill.MatIcon = MatForceOrb[UseFlag-1];
	sinContinueSkillSet(&TempSkill);
	return TRUE;

}

//포스아이템을 셋팅한다(접속종료및,기타등등)
int cINVENTORY::SetForceOrb(DWORD ItemCODE , int Time)
{
	int UseFlag = 0; 
	sSKILL TempSkill;
	memset(&TempSkill,0,sizeof(sSKILL));

	for(int i = 0; i < 12 ; i++){ 
		if((ItemCODE&sinITEM_MASK3)== SheltomCode2[i]){
			UseFlag = i+1;

		}
	}
	char *Path = "image\\Sinimage\\skill\\ForceOrb\\FOS_0%d.tga";      
	char szBuff[128];
	if(!MatForceOrb[UseFlag-1]){
		wsprintf(szBuff,Path,UseFlag);
		MatForceOrb[UseFlag-1] = CreateTextureMaterial(szBuff, 0, 0, 0,0, SMMAT_BLEND_ALPHA );
		ReadTextures();	//텍스쳐 로딩
	}


	TempSkill.CODE = SKILL_FORCE_ORB;
	TempSkill.Point = UseFlag;
	TempSkill.UseTime = ForceOrbUseTime[UseFlag-1];
	TempSkill.CheckTime = (ForceOrbUseTime[UseFlag-1]*70) - (Time*70);
	TempSkill.MatIcon = MatForceOrb[UseFlag-1];
	sinContinueSkillSet(&TempSkill);
	return TRUE;
}

// 박재원 - 캡슐아이템 사용하기(호랑이 캡슐 사용)
int cINVENTORY::UseCapsuleItem(sITEM *pItem)
{
	// 박재원 : 캡슐아이템 보상으로 주어지는 아이템을 위해 공간을 확보한다.
	sITEM TempItem;
	TempItem.w = 22*2;
	TempItem.h = 22*2;

	if( !cCraftItem.CraftCheckEmptyArea(&TempItem) )
	{
		cMessageBox.ShowMessage(MESSAGE_OVER_SPACE); //공간부족
		return FALSE;
	}

	if( pItem->CODE == (sinSP1|sin34) )
	{
		if( sinChar->Weight[0] + 5 >= sinChar->Weight[1] )
		{
			cMessageBox.ShowMessage(MESSAGE_OVER_WEIGHT); //무게초과
			return FALSE;
		}
	}

	bGiftBoxDelay = true; // 캡슐을 까는 동안 다른 행동을 못하도록 딜레이를 준다. 선물상자 딜레이와 동일

	// 캡슐 아이템 사용 모션을 물약사용 모션처럼 동일하게 사용한다.
	if(sinActionPotion())
	{
		pUsePotion = &InvenItem[SelectInvenItemIndex-1]; 
		cInvenTory.ReFormInvenItem();
		cInvenTory.CheckWeight();   //무게를 셋팅한다 
		sinUsePotionDelayFlag = 1;
	}

	return TRUE;
}

int CristalEventItemIndex[7]={0,};

int PristonAlphabetEventItemIndex[7] = {0,}; // 박재원 - 알파벳 조합 이벤트

//<ha>일곱가지의  보은 크리스탈 체크
int cINVENTORY::CheckCristalEventItem()
{
	int i,cnt=0;
	for(i=0;i<7;i++){
		CristalEventItemIndex[i]=0;
	}
	
	//크리스탈을 찾아서 인덱스를 넘긴다.
	for(i=0;i<12;i++){
		if(cnt < 7){
			if(CristalEventItemIndex[cnt]==0 ){
				CristalEventItemIndex[cnt]=SearchItemIndex(CristalEventCODE[i],0);
			}
			if(CristalEventItemIndex[cnt]!=0)
					cnt++;
		}
	}
	//7개를 모았다.
	if(cnt==7){
		cInterFace.CheckAllBox(SIN_INVENTORY);
		cMessageBox.ShowMessage3(MESSAGE_CRISTALEVENTITEM_SEND,CristalItem);
		
	}
	else
		cSinHelp.sinShowHelp(SIN_HELP_KIND_BABEL,QuestMessageBoxPosi2.x,QuestMessageBoxPosi2.y,QuestMessageBoxSize2.x,QuestMessageBoxSize2.y, RGBA_MAKE(0,15,128,125),CristalEventDoc);  

	return TRUE;
}
//<ha>일곱가지의 보은 크리스탈을 서버로 보낸다
int cINVENTORY::SendCristalEventItem()
{

	sITEM TempItem;
	TempItem.w = 22*3;
	TempItem.h = 22*4;

	memset(&sWingItem_Send,0,sizeof(sCRAFTITEM_SERVER)); //초기화

	//무게를 체크한다.
	if(sinChar->Weight[0]+100 > sinChar->Weight[1]){
			cMessageBox.ShowMessage(MESSAGE_OVER_WEIGHT);
			return FALSE;
	}
	//여기서 가상의 공간체크를해본다
	if(!cCraftItem.CraftCheckEmptyArea(&TempItem)){
		cMessageBox.ShowMessage(MESSAGE_OVER_SPACE);
		return FALSE;
	}
	//체크
	for(int i=0;i<7;i++){	
		if(CristalEventItemIndex[i]>100){
			sWingItem_Send.SheltomCode[i] = InvenItemTemp[CristalEventItemIndex[i]-100-1].sItemInfo.CODE;
			sWingItem_Send.Head[i]        = InvenItemTemp[CristalEventItemIndex[i]-100-1].sItemInfo.ItemHeader.Head;
			sWingItem_Send.CheckSum[i]    = InvenItemTemp[CristalEventItemIndex[i]-100-1].sItemInfo.ItemHeader.dwChkSum;
			continue;
		}
		if(CristalEventItemIndex[i]){
			sWingItem_Send.SheltomCode[i] = InvenItem[CristalEventItemIndex[i]-1].sItemInfo.CODE;
			sWingItem_Send.Head[i]        = InvenItem[CristalEventItemIndex[i]-1].sItemInfo.ItemHeader.Head;
			sWingItem_Send.CheckSum[i]    = InvenItem[CristalEventItemIndex[i]-1].sItemInfo.ItemHeader.dwChkSum;
		
		}
	}
	sWingItem_Send.DocIndex = 2; //auto셋팅!!
	return TRUE;	
}

//박재원 - 수박 모아오기 수박 아이템 갯수를 체크한다.
int cINVENTORY::CheckWatermelonEventItem()
{
	int i,cnt=0;
	for(i=0;i<7;i++)
	{
		WatermelonEventItemIndex[i]=0;
	}

	cnt = CheckItemCount(WatermelonEventCODE);

	//아이템 7개이상을 모을경우
	if(cnt>=7)
	{
		cInterFace.CheckAllBox(SIN_INVENTORY);
		cMessageBox.ShowMessage3(MESSAGE_WATERMELON_EVENTITEM,StarItem); // 별조각 //박재원 - 밤하늘의 소원 이벤트  
	//	cMessageBox.ShowMessage3(MESSAGE_WATERMELON_EVENTITEM,PumpkinItem); // 호박
	//	cMessageBox.ShowMessage3(MESSAGE_WATERMELON_EVENTITEM,WatermelonItem); // 수박

	}
	else
		cSinHelp.sinShowHelp(SIN_HELP_KIND_BABEL,QuestMessageBoxPosi2.x,QuestMessageBoxPosi2.y,QuestMessageBoxSize2.x,QuestMessageBoxSize2.y, RGBA_MAKE(0,15,128,125),StarDoc);  // 별조각 //박재원 - 밤하늘의 소원 이벤트  
	//	cSinHelp.sinShowHelp(SIN_HELP_KIND_BABEL,QuestMessageBoxPosi2.x,QuestMessageBoxPosi2.y,QuestMessageBoxSize2.x,QuestMessageBoxSize2.y, RGBA_MAKE(0,15,128,125),PumpkinDoc);  // 호박
	//	cSinHelp.sinShowHelp(SIN_HELP_KIND_BABEL,QuestMessageBoxPosi2.x,QuestMessageBoxPosi2.y,QuestMessageBoxSize2.x,QuestMessageBoxSize2.y, RGBA_MAKE(0,15,128,125),WatermelonDoc);  // 수박

	return TRUE;
}


//장별 - 발렌타인 초콜렛 모아오기 초콜렛 아이템 갯수를 체크한다.
int cINVENTORY::CheckValentineEventItem()
{
	int i,cnt=0;
	for(i=0;i<7;i++)
	{
		ValentineEventItemIndex[i]=0;
	}

	cnt = CheckValentineItemCount(ValentineEventCODE);

	//아이템 7개이상을 모을경우
	if(cnt>=7)
	{
		cInterFace.CheckAllBox(SIN_INVENTORY);
		cMessageBox.ShowMessage3(MESSAGE_VALENTINE_EVENTITEM,ValentineItem); // 장별 - 발렌타인 이벤트  
	}
	else
		cSinHelp.sinShowHelp(SIN_HELP_KIND_BABEL,QuestMessageBoxPosi2.x,QuestMessageBoxPosi2.y,QuestMessageBoxSize2.x,QuestMessageBoxSize2.y, RGBA_MAKE(0,15,128,125),ValentineDoc);  // 장별 - 발렌타인 이벤트  


	return TRUE;
}


// 장별 - 캔디데이즈 캔디 모아오기 캔디 아이템 갯수를 체크한다.
int cINVENTORY::CheckCandydaysEventItem() 
{
	int i,cnt=0;
	for(i=0;i<7;i++)
	{
		CandydaysEventItemIndex[i]=0;
	}

	cnt = CheckCandyItemCount(CandydaysEventCODE);

	//아이템 7개이상을 모을경우
	if(cnt>=7)
	{
		cInterFace.CheckAllBox(SIN_INVENTORY);
		cMessageBox.ShowMessage3(MESSAGE_CANDYDAYS_EVENTITEM,CandydaysItem); // 장별 - 캔디데이즈 
	}
	else
		cSinHelp.sinShowHelp(SIN_HELP_KIND_BABEL,QuestMessageBoxPosi2.x,QuestMessageBoxPosi2.y,QuestMessageBoxSize2.x,QuestMessageBoxSize2.y, RGBA_MAKE(0,15,128,125),CandydaysDoc);  // 장별 - 캔디데이즈


	return TRUE;
}

// 장별 - 매지컬그린  에메랄드, 비취 아이템 갯수를 체크한다.
int cINVENTORY::CheckMagicalGreenEventItem() 
{
	int i,cnt=0, cnt2=0;
	for(i=0;i<7;i++)
	{
		MagicalGreenEventItemIndex[i]=0;
		MagicalGreenEmeraldEventItemIndex[i] = 0;
	}

	cnt = CheckMagicalGreenItemCount(MagicalGreenEventCODE[0]);
	cnt2 = CheckMagicalGreenItemEmeraldCount(MagicalGreenEventCODE[1]);

	//아이템 7개이상을 모을경우
	if(cnt>=7)
	{
		cInterFace.CheckAllBox(SIN_INVENTORY);
		cMessageBox.ShowMessage3(MESSAGE_MAGICALGREEN_EVENTITEM,MagicalGreenJadeItem); 
	}

	else if(cnt2>=7)
	{
		cInterFace.CheckAllBox(SIN_INVENTORY);
		cMessageBox.ShowMessage3(MESSAGE_MAGICALGREEN_EVENTITEM_EMERALD,MagicalGreenEmeraldItem); 
	}

	else
		cSinHelp.sinShowHelp(SIN_HELP_KIND_BABEL,QuestMessageBoxPosi2.x,QuestMessageBoxPosi2.y,QuestMessageBoxSize2.x,QuestMessageBoxSize2.y, RGBA_MAKE(0,15,128,125),MagicalGreenDoc);  


	return TRUE;
}

// 장별 - 카라의 눈물 아이템 갯수를 체크한다.
int cINVENTORY::CheckTearOfKaraEventItem() 
{
	int i,cnt=0;
	for(i=0;i<7;i++)
	{
		TearOfKaraEventItemIndex[i]=0;
	}

	cnt = CheckTearOfKaraItemCount(TeatOfKaraEventCODE);

	//아이템 7개이상을 모을경우
	if(cnt>=7)
	{
		cInterFace.CheckAllBox(SIN_INVENTORY);
		cMessageBox.ShowMessage3(MESSAGE_TEAROFKARA_EVENTITEM,TearOfKaraItem); // 장별 - 캔디데이즈 
	}
	else
		cSinHelp.sinShowHelp(SIN_HELP_KIND_BABEL,QuestMessageBoxPosi2.x,QuestMessageBoxPosi2.y,QuestMessageBoxSize2.x,QuestMessageBoxSize2.y, RGBA_MAKE(0,15,128,125),TearOfKaraDoc); 


	return TRUE;
}


// 장별 - 조사원을 찾아라 아이템 갯수를 체크한다. - 마력 연구가
int cINVENTORY::CheckFindinvestigatorEventItem()
{
	int i,cnt=0, cnt2=0;

	for(i=0;i<1;i++)
	{
		FindinvestigatorNineEventItemIndex[i] = 0;
		FindinvestigatorTaleEventItemIndex[i] = 0;
	}

	cnt = CheckFindinvestigatorNineItemCount(FindinvestigatorEventCODE[0]);
	cnt2 = CheckFindinvestigatorTaleItemCount(FindinvestigatorEventCODE[1]);

	// 나인아뮬렛, 테일아뮬렛 둘다 소지하지 않고 있을때
	if( cnt ==0 && cnt2 == 0 )
	{
		cSinHelp.sinShowHelp(SIN_HELP_KIND_BABEL,QuestMessageBoxPosi2.x,QuestMessageBoxPosi2.y,QuestMessageBoxSize2.x,QuestMessageBoxSize2.y, RGBA_MAKE(0,15,128,125),Findinvestigator1_1Doc);
		cMessageBox.ShowMessage3(MESSAGE_FIND_EVENTITEM,FindinvestigatorItem);
		
	}

	// 나인아뮬렛 소지(테일아율렛 상관 없음)
	else if( cnt == 1 )
	{
		cSinHelp.sinShowHelp(SIN_HELP_KIND_BABEL,QuestMessageBoxPosi2.x,QuestMessageBoxPosi2.y,QuestMessageBoxSize2.x,QuestMessageBoxSize2.y, RGBA_MAKE(0,15,128,125),Findinvestigator1_2Doc);
	}

	// 테일아뮬렛만 소지시
	else if( cnt == 0 && cnt2 == 1 )
	{
		cSinHelp.sinShowHelp(SIN_HELP_KIND_BABEL,QuestMessageBoxPosi2.x,QuestMessageBoxPosi2.y,QuestMessageBoxSize2.x,QuestMessageBoxSize2.y, RGBA_MAKE(0,15,128,125),Findinvestigator1_3Doc);
	}

	return TRUE;
}


// 장별 - 조사원을 찾아라 아이템 갯수를 체크한다. - 카미유
int cINVENTORY::CheckFindinvestigatorKamiuEventItem()
{
	int i,cnt=0;

	for(i=0;i<1;i++)
	{
		FindinvestigatorNineEventItemIndex[i] = 0;
	}

	cnt = CheckFindinvestigatorNineItemCount(FindinvestigatorEventCODE[0]);

	// 나인아뮬렛을 소지
	if( cnt == 1 )
	{
		cSinHelp.sinShowHelp(SIN_HELP_KIND_BABEL,QuestMessageBoxPosi2.x,QuestMessageBoxPosi2.y,QuestMessageBoxSize2.x,QuestMessageBoxSize2.y, RGBA_MAKE(0,15,128,125),Findinvestigator2_1Doc);
		cMessageBox.ShowMessage3(MESSAGE_FINDKAMIU_EVENTITEM,FindinvestigatorNineItem);
	}

	// 나인아뮬렛 소지하지 않을 때
	else if( cnt == 0 )
	{
		cSinHelp.sinShowHelp(SIN_HELP_KIND_BABEL,QuestMessageBoxPosi2.x,QuestMessageBoxPosi2.y,QuestMessageBoxSize2.x,QuestMessageBoxSize2.y, RGBA_MAKE(0,15,128,125),Findinvestigator2_2Doc);
	}

	

	return TRUE;
}


// 장별 - 조사원을 찾아라 아이템 갯수를 체크한다. - 에토
int cINVENTORY::CheckFindinvestigatorEtoEventItem()
{
	int i,cnt=0;

	for(i=0;i<1;i++)
	{
		FindinvestigatorTaleEventItemIndex[i] = 0;
	}

	cnt = CheckFindinvestigatorTaleItemCount(FindinvestigatorEventCODE[1]);

	// 테일아뮬렛을 소지
	if( cnt == 1 )
	{
		cSinHelp.sinShowHelp(SIN_HELP_KIND_BABEL,QuestMessageBoxPosi2.x,QuestMessageBoxPosi2.y,QuestMessageBoxSize2.x,QuestMessageBoxSize2.y, RGBA_MAKE(0,15,128,125),Findinvestigator3_1Doc);
		cMessageBox.ShowMessage3(MESSAGE_FINDETO_EVENTITEM,FindinvestigatorTaleItem);
	}

	// 테일아뮬렛 소지하지 않을 때
	else if( cnt == 0 )
	{
		cSinHelp.sinShowHelp(SIN_HELP_KIND_BABEL,QuestMessageBoxPosi2.x,QuestMessageBoxPosi2.y,QuestMessageBoxSize2.x,QuestMessageBoxSize2.y, RGBA_MAKE(0,15,128,125),Findinvestigator3_2Doc);
	}

	

	return TRUE;
}


//박재원 - 수박 모아오기 수박 아이템을 서버로 보낸다
int cINVENTORY::SendWatermelonEventItem()
{
	sITEM TempItem;
	TempItem.w = 22*3;
	TempItem.h = 22*4;

	memset(&sWingItem_Send,0,sizeof(sCRAFTITEM_SERVER)); //초기화

	//무게를 체크한다.
	if(sinChar->Weight[0]+10 > sinChar->Weight[1]){ // 무게 체크 (수박이벤트=130 / 호박이벤트=30 / 밤하늘의 소원 이벤트=10  ) 
		cMessageBox.ShowMessage(MESSAGE_OVER_WEIGHT);
		return FALSE;
	}
	//여기서 가상의 공간체크를해본다
	if(!cCraftItem.CraftCheckEmptyArea(&TempItem)){
		cMessageBox.ShowMessage(MESSAGE_OVER_SPACE);
		return FALSE;
	}
	//체크
	for(int i=0;i<7;i++){	
		if(WatermelonEventItemIndex[i]>100){
			sWingItem_Send.SheltomCode[i] = InvenItemTemp[WatermelonEventItemIndex[i]-100-1].sItemInfo.CODE;
			sWingItem_Send.Head[i]        = InvenItemTemp[WatermelonEventItemIndex[i]-100-1].sItemInfo.ItemHeader.Head;
			sWingItem_Send.CheckSum[i]    = InvenItemTemp[WatermelonEventItemIndex[i]-100-1].sItemInfo.ItemHeader.dwChkSum;
			continue;
		}
		if(WatermelonEventItemIndex[i]){
			sWingItem_Send.SheltomCode[i] = InvenItem[WatermelonEventItemIndex[i]-1].sItemInfo.CODE;
			sWingItem_Send.Head[i]        = InvenItem[WatermelonEventItemIndex[i]-1].sItemInfo.ItemHeader.Head;
			sWingItem_Send.CheckSum[i]    = InvenItem[WatermelonEventItemIndex[i]-1].sItemInfo.ItemHeader.dwChkSum;

		}
	}
	sWingItem_Send.DocIndex = 2; //auto셋팅!!
	return TRUE;
}




//장별 - 발렌타인 이벤트 // 발렌타인 초콜렛 아이템을 서버로 보낸다
int cINVENTORY::SendValentineEventItem()
{
	sITEM TempItem;
	TempItem.w = 22*3;
	TempItem.h = 22*4;

	memset(&sWingItem_Send,0,sizeof(sCRAFTITEM_SERVER)); //초기화

	//무게를 체크한다.
	if(sinChar->Weight[0]+50 > sinChar->Weight[1]){ // 무게 체크 (수박이벤트=130 / 호박이벤트=30 / 밤하늘의 소원 이벤트=10 / 발렌타인이벤트= 50) 
		cMessageBox.ShowMessage(MESSAGE_OVER_WEIGHT);
		return FALSE;
	}
	//여기서 가상의 공간체크를해본다
	if(!cCraftItem.CraftCheckEmptyArea(&TempItem)){
		cMessageBox.ShowMessage(MESSAGE_OVER_SPACE);
		return FALSE;
	}
	//체크
	for(int i=0;i<7;i++){	
		if(ValentineEventItemIndex[i]>100){
			sWingItem_Send.SheltomCode[i] = InvenItemTemp[ValentineEventItemIndex[i]-100-1].sItemInfo.CODE;
			sWingItem_Send.Head[i]        = InvenItemTemp[ValentineEventItemIndex[i]-100-1].sItemInfo.ItemHeader.Head;
			sWingItem_Send.CheckSum[i]    = InvenItemTemp[ValentineEventItemIndex[i]-100-1].sItemInfo.ItemHeader.dwChkSum;
			continue;
		}
		if(ValentineEventItemIndex[i]){
			sWingItem_Send.SheltomCode[i] = InvenItem[ValentineEventItemIndex[i]-1].sItemInfo.CODE;
			sWingItem_Send.Head[i]        = InvenItem[ValentineEventItemIndex[i]-1].sItemInfo.ItemHeader.Head;
			sWingItem_Send.CheckSum[i]    = InvenItem[ValentineEventItemIndex[i]-1].sItemInfo.ItemHeader.dwChkSum;

		}
	}
	sWingItem_Send.DocIndex = 2; //auto셋팅!!
	return TRUE;
}

// 박재원 - 알파벳 조합 이벤트
int cINVENTORY::CheckPristonAlphabetEventItem()
{
	int i,cnt=0;
	for(i=0;i<7;i++){
		PristonAlphabetEventItemIndex[i]=0;
	}


	//크리스탈을 찾아서 인덱스를 넘긴다.
	for(i=0;i<7;i++){
		if(cnt < 7){
			if(PristonAlphabetEventItemIndex[cnt]==0 ){
				PristonAlphabetEventItemIndex[cnt]=SearchItemIndex(PristonAlphabetEventCODE[i],0);
			}
			if(PristonAlphabetEventItemIndex[cnt]!=0)
				cnt++;
		}
	}


	//7개를 모았다.
	if(cnt==7){
		cInterFace.CheckAllBox(SIN_INVENTORY);
		cMessageBox.ShowMessage3(MESSAGE_PRISTON_ALPHABET_EVENTITEM,PristonAlphabetItem); 	// 박재원 - 알파벳 조합 이벤트

	}
	else
	{
		cSinHelp.sinShowHelp(SIN_HELP_KIND_BABEL,QuestMessageBoxPosi2.x,QuestMessageBoxPosi2.y,QuestMessageBoxSize2.x,QuestMessageBoxSize2.y, RGBA_MAKE(0,15,128,125),PristonAlphabetDoc);  
	}
	return TRUE;
}

// 박재원 - 알파벳 조합 이벤트(알파벳 아이템을 서버로 보낸다)
int cINVENTORY::SendPristonAlphabetEventItem()
{

	sITEM TempItem;
	TempItem.w = 22*3;
	TempItem.h = 22*4;

	memset(&sWingItem_Send,0,sizeof(sCRAFTITEM_SERVER)); //초기화

	//무게를 체크한다.
	if(sinChar->Weight[0]+20 > sinChar->Weight[1]){ // 박재원 - 알파벳 조합 이벤트(무게를 70으로 수정) // 장별 20으로 수정
		cMessageBox.ShowMessage(MESSAGE_OVER_WEIGHT);
		return FALSE;
	}
	//여기서 가상의 공간체크를해본다
	if(!cCraftItem.CraftCheckEmptyArea(&TempItem)){
		cMessageBox.ShowMessage(MESSAGE_OVER_SPACE);
		return FALSE;
	}
	//체크
	for(int i=0;i<7;i++){	
		if(PristonAlphabetEventItemIndex[i]>100){
			sWingItem_Send.SheltomCode[i] = InvenItemTemp[PristonAlphabetEventItemIndex[i]-100-1].sItemInfo.CODE;
			sWingItem_Send.Head[i]        = InvenItemTemp[PristonAlphabetEventItemIndex[i]-100-1].sItemInfo.ItemHeader.Head;
			sWingItem_Send.CheckSum[i]    = InvenItemTemp[PristonAlphabetEventItemIndex[i]-100-1].sItemInfo.ItemHeader.dwChkSum;
			continue;
		}
		if(PristonAlphabetEventItemIndex[i]){
			sWingItem_Send.SheltomCode[i] = InvenItem[PristonAlphabetEventItemIndex[i]-1].sItemInfo.CODE;
			sWingItem_Send.Head[i]        = InvenItem[PristonAlphabetEventItemIndex[i]-1].sItemInfo.ItemHeader.Head;
			sWingItem_Send.CheckSum[i]    = InvenItem[PristonAlphabetEventItemIndex[i]-1].sItemInfo.ItemHeader.dwChkSum;

		}
	}
	sWingItem_Send.DocIndex = 2; //auto셋팅!!
	return TRUE;	
}


// 장별 - 캔디데이즈 // 캔디 아이템을 서버로 보낸다
int cINVENTORY::SendCandydaysEventItem()
{
	sITEM TempItem;
	TempItem.w = 22*3;
	TempItem.h = 22*4;

	memset(&sWingItem_Send,0,sizeof(sCRAFTITEM_SERVER)); //초기화

	//무게를 체크한다.
	if(sinChar->Weight[0]+50 > sinChar->Weight[1]){ // 무게 체크 (수박이벤트=130 / 호박이벤트=30 / 밤하늘의 소원 이벤트=10 / 캔디데이즈 = 50 ) 
		cMessageBox.ShowMessage(MESSAGE_OVER_WEIGHT);
		return FALSE;
	}
	//여기서 가상의 공간체크를해본다
	if(!cCraftItem.CraftCheckEmptyArea(&TempItem)){
		cMessageBox.ShowMessage(MESSAGE_OVER_SPACE);
		return FALSE;
	}
	//체크
	for(int i=0;i<7;i++){	
		if(CandydaysEventItemIndex[i]>100){
			sWingItem_Send.SheltomCode[i] = InvenItemTemp[CandydaysEventItemIndex[i]-100-1].sItemInfo.CODE;
			sWingItem_Send.Head[i]        = InvenItemTemp[CandydaysEventItemIndex[i]-100-1].sItemInfo.ItemHeader.Head;
			sWingItem_Send.CheckSum[i]    = InvenItemTemp[CandydaysEventItemIndex[i]-100-1].sItemInfo.ItemHeader.dwChkSum;
			continue;
		}
		if(CandydaysEventItemIndex[i]){
			sWingItem_Send.SheltomCode[i] = InvenItem[CandydaysEventItemIndex[i]-1].sItemInfo.CODE;
			sWingItem_Send.Head[i]        = InvenItem[CandydaysEventItemIndex[i]-1].sItemInfo.ItemHeader.Head;
			sWingItem_Send.CheckSum[i]    = InvenItem[CandydaysEventItemIndex[i]-1].sItemInfo.ItemHeader.dwChkSum;

		}
	}
	sWingItem_Send.DocIndex = 2; //auto셋팅!!
	return TRUE;
}



// 장별 - 매지컬그린 - 아이템을 서버로 보낸다
int cINVENTORY::SendMagicalGreenEventItem()
{
	sITEM TempItem;
	TempItem.w = 22*3;
	TempItem.h = 22*4;

	memset(&sWingItem_Send,0,sizeof(sCRAFTITEM_SERVER)); //초기화

	//무게를 체크한다.
	if(sinChar->Weight[0]+70 > sinChar->Weight[1]){ // 무게 체크 (캔디데이즈 = 50 / 매지컬그린 = 20 ) 
		cMessageBox.ShowMessage(MESSAGE_OVER_WEIGHT);
		return FALSE;
	}
	//여기서 가상의 공간체크를해본다
	if(!cCraftItem.CraftCheckEmptyArea(&TempItem)){
		cMessageBox.ShowMessage(MESSAGE_OVER_SPACE);
		return FALSE;
	}
	//체크
	for(int i=0;i<7;i++){	
		if(MagicalGreenEventItemIndex[i]>100){
			sWingItem_Send.SheltomCode[i] = InvenItemTemp[MagicalGreenEventItemIndex[i]-100-1].sItemInfo.CODE;
			sWingItem_Send.Head[i]        = InvenItemTemp[MagicalGreenEventItemIndex[i]-100-1].sItemInfo.ItemHeader.Head;
			sWingItem_Send.CheckSum[i]    = InvenItemTemp[MagicalGreenEventItemIndex[i]-100-1].sItemInfo.ItemHeader.dwChkSum;
			continue;
		}
		if(MagicalGreenEventItemIndex[i]){
			sWingItem_Send.SheltomCode[i] = InvenItem[MagicalGreenEventItemIndex[i]-1].sItemInfo.CODE;
			sWingItem_Send.Head[i]        = InvenItem[MagicalGreenEventItemIndex[i]-1].sItemInfo.ItemHeader.Head;
			sWingItem_Send.CheckSum[i]    = InvenItem[MagicalGreenEventItemIndex[i]-1].sItemInfo.ItemHeader.dwChkSum;

		}
	}
	sWingItem_Send.DocIndex = 2; //auto셋팅!!
	return TRUE;
}


// 장별 - 매지컬그린 - 아이템을 서버로 보낸다
int cINVENTORY::SendMagicalGreenEmeraldEventItem()
{
	sITEM TempItem;
	TempItem.w = 22*3;
	TempItem.h = 22*4;

	memset(&sWingItem_Send,0,sizeof(sCRAFTITEM_SERVER)); //초기화

	//무게를 체크한다.
	if(sinChar->Weight[0]+70 > sinChar->Weight[1]){ // 무게 체크 (캔디데이즈 = 50 / 매지컬그린 = 20 ) 
		cMessageBox.ShowMessage(MESSAGE_OVER_WEIGHT);
		return FALSE;
	}
	//여기서 가상의 공간체크를해본다
	if(!cCraftItem.CraftCheckEmptyArea(&TempItem)){
		cMessageBox.ShowMessage(MESSAGE_OVER_SPACE);
		return FALSE;
	}
	//체크
	for(int i=0;i<7;i++){	
		if(MagicalGreenEmeraldEventItemIndex[i]>100){
			sWingItem_Send.SheltomCode[i] = InvenItemTemp[MagicalGreenEmeraldEventItemIndex[i]-100-1].sItemInfo.CODE;
			sWingItem_Send.Head[i]        = InvenItemTemp[MagicalGreenEmeraldEventItemIndex[i]-100-1].sItemInfo.ItemHeader.Head;
			sWingItem_Send.CheckSum[i]    = InvenItemTemp[MagicalGreenEmeraldEventItemIndex[i]-100-1].sItemInfo.ItemHeader.dwChkSum;
			continue;
		}
		if(MagicalGreenEmeraldEventItemIndex[i]){
			sWingItem_Send.SheltomCode[i] = InvenItem[MagicalGreenEmeraldEventItemIndex[i]-1].sItemInfo.CODE;
			sWingItem_Send.Head[i]        = InvenItem[MagicalGreenEmeraldEventItemIndex[i]-1].sItemInfo.ItemHeader.Head;
			sWingItem_Send.CheckSum[i]    = InvenItem[MagicalGreenEmeraldEventItemIndex[i]-1].sItemInfo.ItemHeader.dwChkSum;

		}
	}
	sWingItem_Send.DocIndex = 2; //auto셋팅!!
	return TRUE;
}


// 장별 - 카라의 눈물 // 눈물 아이템을 서버로 보낸다
int cINVENTORY::SendTearOfKaraEventItem()
{
	sITEM TempItem;
	TempItem.w = 22*2;
	TempItem.h = 22*2;

	memset(&sWingItem_Send,0,sizeof(sCRAFTITEM_SERVER)); //초기화

	//무게를 체크한다.
	if(sinChar->Weight[0]+10 > sinChar->Weight[1]){ // 무게 체크 ( 캔디데이즈 = 50 / 카라의 눈물 = 10) 
		cMessageBox.ShowMessage(MESSAGE_OVER_WEIGHT);
		return FALSE;
	}
	//여기서 가상의 공간체크를해본다
	if(!cCraftItem.CraftCheckEmptyArea(&TempItem)){
		cMessageBox.ShowMessage(MESSAGE_OVER_SPACE);
		return FALSE;
	}
	//체크
	for(int i=0;i<7;i++){	
		if(TearOfKaraEventItemIndex[i]>100){
			sWingItem_Send.SheltomCode[i] = InvenItemTemp[TearOfKaraEventItemIndex[i]-100-1].sItemInfo.CODE;
			sWingItem_Send.Head[i]        = InvenItemTemp[TearOfKaraEventItemIndex[i]-100-1].sItemInfo.ItemHeader.Head;
			sWingItem_Send.CheckSum[i]    = InvenItemTemp[TearOfKaraEventItemIndex[i]-100-1].sItemInfo.ItemHeader.dwChkSum;
			continue;
		}
		if(TearOfKaraEventItemIndex[i]){
			sWingItem_Send.SheltomCode[i] = InvenItem[TearOfKaraEventItemIndex[i]-1].sItemInfo.CODE;
			sWingItem_Send.Head[i]        = InvenItem[TearOfKaraEventItemIndex[i]-1].sItemInfo.ItemHeader.Head;
			sWingItem_Send.CheckSum[i]    = InvenItem[TearOfKaraEventItemIndex[i]-1].sItemInfo.ItemHeader.dwChkSum;

		}
	}
	sWingItem_Send.DocIndex = 2; //auto셋팅!!
	return TRUE;
}

// 박재원 - 2010 월드컵 이벤트 - 축구공 포션 사용하기
int cINVENTORY::UseSoccerBallPotionItem(sITEM *pItem)
{
	pItem->sItemInfo.PotionCount = 100;
	sinThrowItemToFeild(pItem);
	pItem->Flag = 0;

	cInvenTory.SetItemToChar(); //아이템이 셋팅되면 능력치를 셋팅한다 
	cInvenTory.ReFormInvenItem();
	cInvenTory.CheckWeight();   //무게를 셋팅한다 
	sinUsePotionDelayFlag = 1;
	sinPlaySound(SIN_SOUND_EAT_POTION2);//대박 사운드

	return TRUE;
}



// 장별 - 조사원을 찾아라 - 마력연구자
int cINVENTORY::SendFindinvestigatorNineEventItem()
{
	sITEM TempItem;
	TempItem.w = 22*2;
	TempItem.h = 22*2;

	memset(&sWingItem_Send,0,sizeof(sCRAFTITEM_SERVER)); //초기화


	//무게를 체크한다.
	if(sinChar->Weight[0]+5 > sinChar->Weight[1]){ // 무게 체크 ( 캔디데이즈 = 50 / 카라의 눈물 = 10) 
		cMessageBox.ShowMessage(MESSAGE_OVER_WEIGHT);
		return FALSE;
	}
	//여기서 가상의 공간체크를해본다
	if(!cCraftItem.CraftCheckEmptyArea(&TempItem)){
		cMessageBox.ShowMessage(MESSAGE_OVER_SPACE);
		return FALSE;
	}
	//체크
	for(int i=0;i<1;i++){	
		if(FindinvestigatorNineEventItemIndex[i]>100){
			sWingItem_Send.SheltomCode[i] = InvenItemTemp[FindinvestigatorNineEventItemIndex[i]-100-1].sItemInfo.CODE;
			sWingItem_Send.Head[i]        = InvenItemTemp[FindinvestigatorNineEventItemIndex[i]-100-1].sItemInfo.ItemHeader.Head;
			sWingItem_Send.CheckSum[i]    = InvenItemTemp[FindinvestigatorNineEventItemIndex[i]-100-1].sItemInfo.ItemHeader.dwChkSum;
			continue;
		}
		if(FindinvestigatorNineEventItemIndex[i]){
			sWingItem_Send.SheltomCode[i] = InvenItem[FindinvestigatorNineEventItemIndex[i]-1].sItemInfo.CODE;
			sWingItem_Send.Head[i]        = InvenItem[FindinvestigatorNineEventItemIndex[i]-1].sItemInfo.ItemHeader.Head;
			sWingItem_Send.CheckSum[i]    = InvenItem[FindinvestigatorNineEventItemIndex[i]-1].sItemInfo.ItemHeader.dwChkSum;

		}
	}
	sWingItem_Send.DocIndex = 2; //auto셋팅!!
	return TRUE;
}


// 장별 - 조사원을 찾아라 - 카미유
int cINVENTORY::SendFindinvestigatorTaleEventItem()
{
	sITEM TempItem;
	TempItem.w = 22*2;
	TempItem.h = 22*2;

	memset(&sWingItem_Send,0,sizeof(sCRAFTITEM_SERVER)); //초기화

	//무게를 체크한다.
	if(sinChar->Weight[0]+5 > sinChar->Weight[1]){ // 무게 체크 ( 캔디데이즈 = 50 / 카라의 눈물 = 10) 
		cMessageBox.ShowMessage(MESSAGE_OVER_WEIGHT);
		return FALSE;
	}
	//여기서 가상의 공간체크를해본다
	if(!cCraftItem.CraftCheckEmptyArea(&TempItem)){
		cMessageBox.ShowMessage(MESSAGE_OVER_SPACE);
		return FALSE;
	}
	//체크
	for(int i=0;i<1;i++){	
		if(FindinvestigatorNineEventItemIndex[i]>100){
			sWingItem_Send.SheltomCode[i] = InvenItemTemp[FindinvestigatorNineEventItemIndex[i]-100-1].sItemInfo.CODE;
			sWingItem_Send.Head[i]        = InvenItemTemp[FindinvestigatorNineEventItemIndex[i]-100-1].sItemInfo.ItemHeader.Head;
			sWingItem_Send.CheckSum[i]    = InvenItemTemp[FindinvestigatorNineEventItemIndex[i]-100-1].sItemInfo.ItemHeader.dwChkSum;
			continue;
		}
		if(FindinvestigatorNineEventItemIndex[i]){
			sWingItem_Send.SheltomCode[i] = InvenItem[FindinvestigatorNineEventItemIndex[i]-1].sItemInfo.CODE;
			sWingItem_Send.Head[i]        = InvenItem[FindinvestigatorNineEventItemIndex[i]-1].sItemInfo.ItemHeader.Head;
			sWingItem_Send.CheckSum[i]    = InvenItem[FindinvestigatorNineEventItemIndex[i]-1].sItemInfo.ItemHeader.dwChkSum;

		}
	}
	sWingItem_Send.DocIndex = 2; //auto셋팅!!
	return TRUE;
}


// 장별 - 조사원을 찾아라 - 에토
int cINVENTORY::SendFindinvestigatorTaleEtoEventItem()
{
	sITEM TempItem;
	TempItem.w = 22*2;
	TempItem.h = 22*2;

	memset(&sWingItem_Send,0,sizeof(sCRAFTITEM_SERVER)); //초기화

	//무게를 체크한다.
	if(sinChar->Weight[0]+10 > sinChar->Weight[1]){ // 무게 체크 ( 캔디데이즈 = 50 / 카라의 눈물 = 10) 
		cMessageBox.ShowMessage(MESSAGE_OVER_WEIGHT);
		return FALSE;
	}
	//여기서 가상의 공간체크를해본다
	if(!cCraftItem.CraftCheckEmptyArea(&TempItem)){
		cMessageBox.ShowMessage(MESSAGE_OVER_SPACE);
		return FALSE;
	}
	//체크
	for(int i=0;i<1;i++){	
		if(FindinvestigatorTaleEventItemIndex[i]>100){
			sWingItem_Send.SheltomCode[i] = InvenItemTemp[FindinvestigatorTaleEventItemIndex[i]-100-1].sItemInfo.CODE;
			sWingItem_Send.Head[i]        = InvenItemTemp[FindinvestigatorTaleEventItemIndex[i]-100-1].sItemInfo.ItemHeader.Head;
			sWingItem_Send.CheckSum[i]    = InvenItemTemp[FindinvestigatorTaleEventItemIndex[i]-100-1].sItemInfo.ItemHeader.dwChkSum;
			continue;
		}
		if(FindinvestigatorTaleEventItemIndex[i]){
			sWingItem_Send.SheltomCode[i] = InvenItem[FindinvestigatorTaleEventItemIndex[i]-1].sItemInfo.CODE;
			sWingItem_Send.Head[i]        = InvenItem[FindinvestigatorTaleEventItemIndex[i]-1].sItemInfo.ItemHeader.Head;
			sWingItem_Send.CheckSum[i]    = InvenItem[FindinvestigatorTaleEventItemIndex[i]-1].sItemInfo.ItemHeader.dwChkSum;

		}
	}
	sWingItem_Send.DocIndex = 2; //auto셋팅!!
	return TRUE;
}