/*----------------------------------------------------------------------------*
*	파일명 :  sinTrade.cpp
*	하는일 :  아이템 교환 
*	작성일 :  최종업데이트 12월 sinInvenColor
*	적성자 :  박상열	
*-----------------------------------------------------------------------------*/	

#include "sinLinkHeader.h"
//#define  __SIN_AGING_TEST
#define AGING_MONEY_FREE // 박재원 - 에이징 가격 무료 이벤트
/*----------------------------------------------------------------------------*
*								전역변수 			
*-----------------------------------------------------------------------------*/	
cTRADE cTrade;
sTRADE sTrade;	   //보낼 정보 	
sTRADE sTradeRecv; //받은 정보 

cMIXTURERESET cMixtureReset;					// 석지용 - 믹스쳐 리셋 객체
sMIXTURE_RESET_ITEM	sMixtureResetItem;			// 석지용 - 믹스쳐 리셋 아이템 정보 구조체
sMIXTURE_RESET_ITEM sMixtureResetItemBackUp;	// 석지용 - 믹스쳐 리셋 아이템 정보 구조체 백업
int	MixtureResetCheckFlag = 0;					// 석지용 - 믹스쳐 리셋이 가능한가?
sMIXTURE_RESET_ITEM_SERVER	sMixtureReset_Send;	// 석지용 - 믹스쳐 리셋 서버로 전송할 구조체
sMIXTURE_RESET_ITEM_SERVER	sMixtureReset_Recv;	// 석지용 - 믹스쳐 리셋 서버에서 받을 구조체

cAGING	cAging;		//에이징 
sAGINGITEM sAgingItem;
sAGINGITEM sAgingItemBackUp;


sCRAFTITEM_SERVER sAging_Send; //서버로 보내질 아이템 
sCRAFTITEM_SERVER sAging_Recv; //서버에서 받을 아이템 

cWAREHOUSE cWareHouse;	//창고 
sWAREHOUSE sWareHouse;

int WareHouseSaveFlag = 0;
int CloseWareHouseCheckFlag2 = 0;
int CloseWareHouseCheckFlag3 = 0;

cCRAFTITEM cCraftItem;	//아이템 조합 
sCRAFTITEM sCraftItem;  //조합될 아이템 
sCRAFTITEM sCraftItemBackUp;  //조합될 아이템 

// pluto 제련
CSmeltingItem SmeltingItem;
SSmeltingItem S_smeltingItem;
SSmeltingItem sSmeltingItemBackUp;
sSMELTINGITEM_SERVER sSmeltingItem_Send;
sSMELTINGITEM_SERVER sSmeltingItem_Recv;

// pluto 제작
CManufacture ManufactureItem;
SManufactureItem g_sManufactureItem;
SManufactureItem g_sManufactureItemBackUp;
SManufactureItem_Server g_sManufactureItem_Send;
SManufactureItem_Server g_sManufactureItem_Recv;
SManufacture_Rune g_sManufacture_Rune[MAX_RECIPE_KIND];
SManufacture_ResultItemInfo g_Manufacture_ItemInfo[MAX_HANDMADEITEM]; // 장별 - 제작 // 장별 - 대장장이의 혼
//SManufacture_ResultItemInfo_Server g_Item_Recv;

sCRAFTITEM_SERVER sCraftItem_Send; //서버로 보내질 아이템 
sCRAFTITEM_SERVER sCraftItem_Recv; //서버에서 받을 아이템 

sCRAFTITEM_SERVER sWingItem_Send; //서버로 보내질 윙아이템
sCRAFTITEM_SERVER sWingItem_Recv; //서버에서 받을 아이템 

int MixItemNoCopyFlag = 0;


int CraftItemButtonIndex = 0;
int CraftItemSendServerIndex = 0;

/////////////// 믹스쳐 복사 방지를 위해서 
int CheckMixItemCopyFlag = 0;

int ShowItemCraftMessageFlag = 0; //아이템이 조합됬을때 메세지를 보여준다 
int ShowItemAgingMessageFlag = 0; //아이템이 에이징됬을때 메세지를 보여준다
int ShowItemSmeltingMessageFlag = 0;	// pluto 아이템이 제련 됬을때
int ShowItemManufactureMessageFlag = 0; // pluto 아이템이 제작 됬을때
int ShowItemOverSpace = 0; // pluto 공간부족

char szAgingItemBuff[128];
char szCraftItemBuff[128];
POINT CraftItemMessageBoxPosi = {0,0}; //박스 위치 
POINT CraftItemMessageSize = {0,0};    //박스 사이즈 


sITEM *BackUpInvenItem2;
sITEM *BackUpInvenTempItem2;
sINVENTORY *BackUpsInven;
int   InvenGold = 0;
int   RealGold  = 0;

int TradeCheckFlag = 0;

int CopyItemIndex7[10] = {0,0,0,0,0,0,0,0,0,0}; //복사된 아이템을 찾기 편하게 하기위한 플랙 

int TradeColorIndex = 0;			//배경 컬러 인덱스 
int	TradeCrashItemIndex[2] = {0,0}; //겹쳐질 아이템 인덱스 
RECT TradeColorRect = {0,0,0,0};
int  SelectTradeItemIndex=0;		//선택아이템 배경 
int RequestTradeButtonFlag = 0;		//버튼 플랙 

int ItemKindFlag = 0; //1은 내꺼  2는 거래자꺼 

/////////////////////// 승인 관련 
int CheckDelayFlag = 0;
int sinCancelFlag  = 0; //취소만 할수있다 

/////////////////////// 해킹 방지 
int TempShowMoney = 0;

///////////////////////
/*        창고        */
int CloseWareHouseFlag = 0;

///////////////////////

/////////////////////// 사기방지 
int ItemPickUpFlag = 0;

//우체국에서 아이템을 받는다
sPOSTBOX_ITEM sPostbox_Item[50];
int PostBoxIndexCnt = 0;
DWORD ExpressItemCode = 0; //서버에서 받을 아이템코드
char  szExpressItemName[64]; //받을 아이템 이름 

///////////////////////
char *PostBoxDocFilePath[] ={
	"image\\Sinimage\\shopall\\PostBox.sin",
	
};
//haGoon아이템 재구성 시스템 
sRECON_ITEM sReconItem[MAX_SEEL_COUNT]={
     //CODE         level    count price
	{(sinSE1|sin01) ,{0,105} ,2   ,100000},  // 박재원 : 아이템 재구성할때 필요한 씰 개수 2개로 모두 통일시킴 // 장별 - 사용 가능 레벨 102->105로 변경
	{(sinSE1|sin02) ,{40,79} ,2   ,50000 },  // 박재원 : 니이드 사용레벨(40~79)로 수정 {(sinSE1|sin02) ,{40,69} ,4   ,50000 },  
	{(sinSE1|sin03) ,{0,39}  ,2   ,2000  },  // {(sinSE1|sin03) ,{0,39}  ,8   ,2000  },
	{(sinSE1|sin04) ,{0,80}  ,2   ,70000 }  // 박재원 - 테이와즈 씰 추가
};

#define T_REVISION_Y			22*5


//////거래요구창 좌표 
#define TRADEBUTTONMAIN_X		513
#define TRADEBUTTONMAIN_Y		3

#include "..\\nettype.hpp"

//아이템 크래프트
sCRAFTITEM_INFO sCraftItem_Info[MAX_CRAFTITEM_INFO] = {

#ifdef _W_SERVER
#include "sinCraftItem.h"
#else
#include "_sinCraftItem.h"
#endif
};

//아이템 Make
sMAKEITEM_INFO sMakeItem_Info[MAX_MAKEITEM_INFO] = {
	#include "sinMakeItem.h"
};


DWORD sinTradeColor = RGBA_MAKE( 0,200,255,80 );

/*
	{21,136,21+(22*9),136+(22*4)},	//셋팅박스 
	{221,163-3-20,240,182-3-20},	//돈버튼 좌표 
	{244,163-3-20,307,180-3-20},	//돈표시 좌표 
	{248,191-3-20,308,209-3-20},	//무게 
	{251,220,251+26,220+26}	//체크 
*/
int UpTradePosi[5][4] = {
	{21,136+sinInterHeight2,21+(22*9),136+(22*4)+sinInterHeight2},	//셋팅박스 
	{231,170+sinInterHeight2,250,189+sinInterHeight2},	//돈버튼
	{251,120,326,137},	//돈표시 좌표 
	{251,118,326,162},	//무게  (왜 이렇게 이상하게 되있는지 나두 멀겠따 -_-
	{252,221+sinInterHeight2,252+26,221+26+sinInterHeight2}	//체크 

};

int DownTradePosi[6][4] = {
	{21,224+21+sinInterHeight2,21+(22*9),224+21+(22*4)+sinInterHeight2},	//셋팅박스 
	{225,205,250,226},	//돈버튼 좌표 
	{251,206,326,224},	//돈표시 좌표 
	{251,232,326,250},	//무게 
	{252+1,310+sinInterHeight2,252+26,310+26+sinInterHeight2},//체크 
	{305,314+sinInterHeight2,304+20,314+20+sinInterHeight2},	//닫기 

};

//좌표셋팅 
int TradeStartX=DownTradePosi[T_SET_BOX][0], TradeStartY =DownTradePosi[T_SET_BOX][1], TradeEndX=DownTradePosi[T_SET_BOX][2], TradeEndY =DownTradePosi[T_SET_BOX][3];

char szAgingMsgBuff[128];
char *SheltomName[MAX_SHELTOM] = {"\267\257\276\276\265\360","\275\352\267\271\263\327\277\300","\306\304\265\245\277\300","\275\272\306\304\305\260","\267\271\300\314\264\370\306\256","\306\256\267\243\275\272\306\320\267\316","\270\323\305\260","\265\245\272\361\263\327","\275\354\267\271\275\272\305\344","\271\314\266\363\301\366","\300\316\306\344\270\243\263\252","\300\314\264\317\261\327\270\266","\272\247\267\353"};

/////임시적으로 에이징에 관련된 쉘텀을 셋팅한다 

//에이징에 쓰일 쉘텀 

#ifdef __SIN_AGING_TEST

//공격횟수 
int AgingLevelAttack[21] = {2,4,6,8,10,11,12,13,14,15,10,10,10,10,10,10,10,10,10,10,10};

//크리티컬 횟수 
int AgingLevelCritical[21] = {2,4,6,8,10,11,12,13,14,15,10,10,10,10,10,10,10,10,10,10,10};

//블럭횟수
int AgingLevelBlock[21] = {2,4,6,8,10,11,12,13,14,15,10,10,10,10,10,10,10,10,10,10,10};

//타격횟수 
int AgingLevelHit[21] = {2,4,6,8,10,11,12,13,14,15,10,10,10,10,10,10,10,10,10,10,10};

int AgingLevelSheltom[20][12] = {
	{1,0,0,0,0,0,0,0,0,0,0,0},
	{1,0,0,0,0,0,0,0,0,0,0,0},
	{1,0,0,0,0,0,0,0,0,0,0,0},
	{1,0,0,0,0,0,0,0,0,0,0,0},
	{1,0,0,0,0,0,0,0,0,0,0,0},
	{1,0,0,0,0,0,0,0,0,0,0,0},
	{1,0,0,0,0,0,0,0,0,0,0,0},
	{1,0,0,0,0,0,0,0,0,0,0,0},
	{1,0,0,0,0,0,0,0,0,0,0,0},
	{1,0,0,0,0,0,0,0,0,0,0,0},

	//추가
	{1,0,0,0,0,0,0,0,0,0,0,0},
	{1,0,0,0,0,0,0,0,0,0,0,0},
	{1,0,0,0,0,0,0,0,0,0,0,0},
	{1,0,0,0,0,0,0,0,0,0,0,0},
	// 16부터
	{1,0,0,0,0,0,0,0,0,0,0,0},
	{1,0,0,0,0,0,0,0,0,0,0,0},
	{1,0,0,0,0,0,0,0,0,0,0,0},
	{1,0,0,0,0,0,0,0,0,0,0,0},
	{1,0,0,0,0,0,0,0,0,0,0,0},
	{1,0,0,0,0,0,0,0,0,0,0,0},

};
int AgingOkPercent[20]              = {0,5,15,30,50,60,70,80,90,95,10,10,10,10,10,10,10,10,10,10};


#else 

/////////////////////에이징 레벨별 타격횟수 
#ifdef _W_SERVER

////////////// 진짜 공격횟수 
/*
//공격횟수 
int AgingLevelAttack[21] = {100,140,196,274,384,538,753,1054,1476,2066,2893,4050,5669,7937,11112,15557,21779,30489,42683,59755};

//크리티컬 횟수 
int AgingLevelCritical[21] = {12,17,24,33,46,65,90,126,177,248,347,486,680,952,1333,1867,2613,3658,5121,7169};

//블럭횟수
int AgingLevelBlock[21] = {15,21,29,41,58,81,113,158,221,310,434,607,850,1191,1667,2334,3267,4573,6402,8962};

//타격횟수 
int AgingLevelHit[21] = {45,63,88,123,173,242,339,474,664,930,1302,1822,2551,3572,5000,7001,9801,13721,19209,26892};
*/

//박재원 : 에이징 레벨상승 예상숙련치 수정
//공격횟수 
int AgingLevelAttack[21] = {100,130,169,219,284,369,479,622,808,1049,1362,1769,2297,2983,3874,5031,6534,8486,11021,14313};

//크리티컬 횟수 
int AgingLevelCritical[21] = {12,16,21,27,35,45,58,75,97,126,164,213,277,360,468,608,790,1026,1332,1730};

//블럭횟수
int AgingLevelBlock[21] = {15,19,25,32,42,55,71,92,119,155,201,261,339,440,571,742,964,1252,1626,2112};

//타격횟수 
int AgingLevelHit[21] = {45,58,75,97,126,164,213,277,360,468,608,790,1026,1332,1730,2247,2918,3790,4922,6392};


///////////////
//int AgingOkPercent[20]                     = {0,0,0,15,20,25,30,35,40,45,50,55,60,70,80,90,95,95,95,95};
//int AgingStoneAgingOkPercent[20]           = {0,0,0,0,15,20,25,30,36,41,46,52,57,67,78,88,96,96,96,96};

//박재원 : 에이징 실패 확률 조정 08.07.31 // 장별 - 10.08.03
//int AgingOkPercent[20]                     = {0, 0, 0, 0, 0, 15, 20, 25, 30, 35, 40, 45, 50, 55, 60, 70, 80, 90, 95, 95}; // 에이징 실패 확률
//int AgingStoneAgingOkPercent[20]           = {0, 0, 0, 0, 0, 10, 15, 20, 25, 30, 35, 40, 45, 50, 55, 65, 75, 85, 96, 96}; // 에이징 스톤 실패 확률


int AgingOkPercent[20]                     = {0, 0, 0, 0, 0, 5, 10, 15, 20, 30, 35, 40, 45, 50, 55, 65, 75, 85, 95, 95};

// 장별 - 에이징스톤 실패 확률 수정 2010.03.05 장별 - 10.08.03
//int AgingStoneAgingOkPercent[20]           = {0, 0, 0, 0, 0, 8, 11, 14, 18, 21, 25, 29, 33, 38, 43, 54, 67, 82, 90, 90}; // 에이징 스톤 실패 확률
int AgingStoneAgingOkPercent[20]           = {0, 0, 0, 0, 0, 3, 5, 8, 11, 18, 21, 25, 29, 33, 38, 48, 60, 74, 90, 90}; // 에이징 스톤 실패 확률

// 장별 - 슈퍼 에이징 스톤 실패 확률
int SuperAgingStoneAgingFailPercent[20] = {0, 0, 0, 0, 0, 3, 5, 8, 11, 18, 21, 25, 29, 33, 38, 48, 60, 74, 90, 90};

// 장별 - 슈퍼 에이징 스톤 1.5 실패 확률
int SuperAgingStone1_5AgingFailPercent[20] = {0, 0, 0, 0, 0, 3, 7, 11, 14, 22, 26, 31, 35, 40, 45, 55, 67, 79, 93, 93};


#else
//야매
//공격횟수 
int AgingLevelAttack[21] = {200,400,800,1500,3000,5000,10000,15000,25000,50000,10,100,10,10,10,10,10,10,10,10,10};

//크리티컬 횟수 
int AgingLevelCritical[21] = {40,80,160,300,600,1000,2000,3000,5000,10000,10,100,10,10,10,10,10,10,10,10,10};

//블럭횟수
int AgingLevelBlock[21] = {40,80,160,300,600,1000,2000,3000,5000,10000,10,100,10,10,10,10,10,10,10,10,10};

//타격횟수 
int AgingLevelHit[21] = {100,200,300,750,1500,2500,5000,7500,12500,25000,10,100,10,10,10,10,10,10,10,10,10};

int AgingOkPercent[20]                     = {0, 0, 0, 0, 0, 5, 10, 15, 20, 30, 35, 40, 45, 50, 55, 65, 75, 85, 95, 95};
int AgingStoneAgingOkPercent[20]           = {0, 0, 0, 0, 0, 3, 5, 8, 11, 18, 21, 25, 29, 33, 38, 48, 60, 74, 90, 90};
int SuperAgingStoneAgingFailPercent[20] = {0, 0, 0, 0, 0, 3, 5, 8, 11, 18, 21, 25, 29, 33, 38, 48, 60, 74, 90, 90};
// 장별 - 슈퍼 에이징 스톤 1.5 실패 확률
int SuperAgingStone1_5AgingFailPercent[20] = {0, 0, 0, 0, 0, 3, 7, 11, 14, 22, 26, 31, 35, 40, 45, 55, 67, 79, 93, 93};


#endif


int AgingLevelSheltom[20][12] = {
	{3,3,4,4,5,0,0,0,0,0,0,0},
	{3,3,4,4,5,5,0,0,0,0,0,0},
	{3,3,4,4,5,5,6,0,0,0,0,0},
	{3,3,4,4,5,5,6,6,0,0,0,0},
	{3,3,4,4,5,5,6,6,7,0,0,0},
	{3,3,4,4,5,5,6,6,7,7,0,0},
	{3,3,4,4,5,5,6,6,7,7,8,0},
	{3,3,4,4,5,5,6,6,7,7,8,8},
	{4,4,5,5,6,6,7,7,8,8,9,0},
	{4,4,5,5,6,6,7,7,8,8,9,9},    //10

	{5,5,6,6,7,7,8 ,8 ,9 ,9 ,10,0 },
	{5,5,6,6,7,7,8 ,8 ,9 ,9 ,10,10},
	{6,6,7,7,8,8,9 ,9 ,10,10,11,0 },
	{6,6,7,7,8,8,9 ,9 ,10,10,11,11},
	{7,7,8,8,9,9,10,10,11,11,12,0 },
	{7,7,8,8,9,9,10,10,11,11,12,12},
	//16차부터 추가 
	{8,8,9,9,10,10,11,11,12,12,13,0},
	{8,8,9,9,10,10,11,11,12,12,13,13},
	{9,9,10,10,11,11,12,12,13,13,14,0},
	{9,9,10,10,11,11,12,12,13,13,14,14},
	//여기서부터 추가 
	//기존
    //여기서 추가 보류
//	{4,4,5,5,6,6,7,7,8,8,9,9},
//	{4,4,5,5,6,6,7,7,8,8,9,9},
//	{4,4,5,5,6,6,7,7,8,8,9,9},


};


///////////////////

#endif

int AgingLevelSheltomIndex[4][3] = {
	0,0,0,
	0,0,0,
	0,0,0,
	0,0,0
};


int AgingCheckSheltomFlag = 0;	//에이징이 될수있는 플렉 
int AgingSheltomCnt2 = 0;


//에이징레벨당  들어간는돈
#define SIN_AGING_MASK	0x00101011

// 박재원 - 에이징 설정 변경(19차, 20차 에이징 비용 추가)
int AgingLevelMoney[20] = {	1000^SIN_AGING_MASK,2000^SIN_AGING_MASK,3000^SIN_AGING_MASK,4000^SIN_AGING_MASK,5000^SIN_AGING_MASK,
							6000^SIN_AGING_MASK,7000^SIN_AGING_MASK,8000^SIN_AGING_MASK,9000^SIN_AGING_MASK,10000^SIN_AGING_MASK,
							11000^SIN_AGING_MASK,12000^SIN_AGING_MASK,13000^SIN_AGING_MASK,14000^SIN_AGING_MASK,15000^SIN_AGING_MASK,
							16000^SIN_AGING_MASK,17000^SIN_AGING_MASK,18000^SIN_AGING_MASK,19000^SIN_AGING_MASK,20000^SIN_AGING_MASK};

POINT AgingLevelSheltomXY[4][3] = {
	{{218,193+sinInterHeight2}			,{218+22,193+sinInterHeight2}		  ,{218+22+22,193+sinInterHeight2}},
	{{218,193+22+sinInterHeight2}		,{218+22,193+22+sinInterHeight2}	  ,{218+22+22,193+22+sinInterHeight2}},
	{{218,193+22+22+sinInterHeight2}	,{218+22,193+22+22+sinInterHeight2}   ,{218+22+22,193+22+22+sinInterHeight2}},
	{{218,193+22+22+22+sinInterHeight2} ,{218+22,193+22+22+22+sinInterHeight2},{218+22+22,193+22+22+22+sinInterHeight2}}

};

// Force Orb
int ForceItemPrice2 = 0;
LPDIRECTDRAWSURFACE4	lpForceMain;
LPDIRECTDRAWSURFACE4	lpForceButtonInfo;
LPDIRECTDRAWSURFACE4	lpForceTitle;


/*----------------------------------------------------------------------------*
*							클래스 초기, 종료 
*-----------------------------------------------------------------------------*/	
cTRADE::cTRADE()
{

}
cTRADE::~cTRADE()
{

}
/*----------------------------------------------------------------------------*
*							     초기화 
*-----------------------------------------------------------------------------*/	
void cTRADE::Init()
{

	//믹스쳐에 사용될 구조체 초기화 
	memset(&sCraftItem,0,sizeof(sCRAFTITEM)); //구조체 초기화 
	memset(&sCraftItem_Recv,0,sizeof(sCRAFTITEM_SERVER)); //받은 믹스쳐 구조체 초기화 

 
	MatTradebuttonMain		= CreateTextureMaterial( "Image\\SinImage\\Shopall\\TradeButton\\TradeButtonMain.tga" , 0, 0, 0,0, SMMAT_BLEND_ALPHA );
	MatTradebuttonCancel	= CreateTextureMaterial( "Image\\SinImage\\Shopall\\TradeButton\\ButtonCancel.tga" , 0, 0, 0,0, SMMAT_BLEND_ALPHA );
	sWareHouse.Money += 2023;	//값을 넣어준다 
	sWareHouse.Weight[0] += 197;
	sWareHouse.Weight[1] += 196;

	ManufactureItem.RuneIndexInit();
	Load();

}
/*----------------------------------------------------------------------------*
*						    Bmp파일을 로드한다 
*-----------------------------------------------------------------------------*/	
void cTRADE::Load()
{
	
	//lpWareHouseMain = LoadDibSurfaceOffscreen("Image\\SinImage\\shopall\\Trade_Check.bmp"); 
	char szTemp[256];

	lpCheck = LoadDibSurfaceOffscreen("Image\\SinImage\\shopall\\Trade_Check.bmp"); 
	lpCheckInfo = LoadDibSurfaceOffscreen("Image\\SinImage\\shopall\\Trade_Check_Info.bmp"); 	
	lpDelayCheck = LoadDibSurfaceOffscreen("Image\\SinImage\\shopall\\Trade_Check_Delay.bmp"); 
	
	lpbuttonOk = LoadDibSurfaceOffscreen("Image\\SinImage\\shopall\\TradeButton\\ButtonOk.bmp"); 

	lpbuttonCancel		=LoadDibSurfaceOffscreen("Image\\SinImage\\shopall\\Trade_Cancel.bmp"); 
	lpbuttonCancelInfo	=LoadDibSurfaceOffscreen("Image\\SinImage\\shopall\\Trade_CancelInfo.bmp"); 

	lpCheck_Glay = LoadDibSurfaceOffscreen("Image\\SinImage\\shopall\\Trade_Check_Glay.bmp"); 
	lpCenterBox = LoadDibSurfaceOffscreen("Image\\SinImage\\shopall\\Trade_0.bmp"); 

	for(int i=0; i < MAX_SHELTOM ; i++){
		wsprintf(szTemp,"Image\\SinImage\\shopall\\Aging\\itos%d_.bmp",100+i+1);
		cAging.lpGraySheltom[i] = LoadDibSurfaceOffscreen(szTemp);
	}
	// pluto 제작 아이템 제작 필요한 룬
	/*
	for( int j = 0; j < MAX_SHELTOM; j++ )	// 모양이 같아서 안쓴다
	{
		wsprintf( szTemp, "Image\\SinImage\\shopall\\Manufacture\\itos%d_.bmp",100+j+1 );
		ManufactureItem.m_lpGrayRune[j] = LoadDibSurfaceOffscreen( szTemp );
	}
	*/
}
/*----------------------------------------------------------------------------*
*						     Release
*-----------------------------------------------------------------------------*/	
void cTRADE::Release()
{
	if(lpCheck){
		lpCheck->Release();
		lpCheck = 0;
	}
	if(lpCheckInfo){
		lpCheckInfo->Release();
		lpCheckInfo =0;

	}
	if(lpDelayCheck){
		lpDelayCheck->Release();
		lpDelayCheck = 0;

	}
	if(lpbuttonOk){
		lpbuttonOk->Release();
		lpbuttonOk = 0;
	}
	if(lpbuttonCancel){
		lpbuttonCancel->Release();
		lpbuttonCancel = 0;

	}
	if(lpbuttonCancelInfo){
		lpbuttonCancelInfo->Release();
		lpbuttonCancelInfo = 0;

	}
	if(lpCheck_Glay){
		lpCheck_Glay->Release();
		lpCheck_Glay = 0;
	}

}
/*----------------------------------------------------------------------------*
*							     그리기  
*-----------------------------------------------------------------------------*/	
DWORD TradeSpotTimer=0;

void cTRADE::Draw()
{
	sinInterHeight2 = -50; //잘될꺼야!

	int i=0 , j = 0;
	////////// 요기는 창고 
	//dsDrawTexImage( cShop.MatShop[0] , 0 - (256+128-sinMoveKindInter[SIN_WAREHOUSE]),  96+sinInterHeight2 , 256, 256 , 255 );       
	//dsDrawTexImage( cShop.MatShop[2] , 256-(256+128-sinMoveKindInter[SIN_WAREHOUSE])-25, 96+sinInterHeight2 ,128 , 256 , 255 );   

	DrawSprite(0-(256+128-sinMoveKindInter[SIN_WAREHOUSE]),90+sinInterHeight2,cShop.lpShopMain,0,0,344,264);	       


	//////////요기는 트레이드 
	//dsDrawTexImage( cShop.MatShop[0] , 0 - (256+128-sinMoveKindInter[SIN_TRADE]),  96+sinInterHeight2 , 256, 256 , 255 );       
	//dsDrawTexImage( cShop.MatShop[1] , 256-(256+128-sinMoveKindInter[SIN_TRADE])-25, 96+sinInterHeight2 ,128 , 256 , 255 );   
	DrawSprite(0-(256+128-sinMoveKindInter[SIN_TRADE]),90+sinInterHeight2,cShop.lpShopMain,0,0,344,264);	       
	DrawSprite(225-(256+128-sinMoveKindInter[SIN_TRADE]),47,cShop.lpTitle_Trade,0,0,111,244);	

	//////////요기는 아이템 조합 

	DrawSprite(0-(256+128-sinMoveKindInter[SIN_CRAFTITEM]),90+sinInterHeight2,cShop.lpShopMain,0,0,344,264);	       
	DrawSprite(225-(256+128-sinMoveKindInter[SIN_CRAFTITEM]),47,cShop.lpTitle_CraftItem,0,0,111,32);	
	DrawSprite(18-(256+128-sinMoveKindInter[SIN_CRAFTITEM]),135+sinInterHeight2,cShop.lpCraftItemMain , 0,0,320,208);

	DrawSprite(200-(256+128-sinMoveKindInter[SIN_CRAFTITEM]),314+sinInterHeight2,cShop.lpGoldEdit , 0,0,98,21);

	if(cCraftItem.ForceFlag){   
		if(!lpForceMain){
			lpForceMain = LoadDibSurfaceOffscreen("Image\\SinImage\\shopall\\shop_alchemy.bmp"); 
			lpForceButtonInfo = LoadDibSurfaceOffscreen("Image\\SinImage\\shopall\\Bt_alchemy.bmp"); 
			lpForceTitle = LoadDibSurfaceOffscreen("Image\\SinImage\\shopall\\title_alchemy.bmp"); 
		}
		DrawSprite(17-(256+128-sinMoveKindInter[SIN_CRAFTITEM]),134+sinInterHeight2,lpForceMain , 0,0,217,208);
		DrawSprite(225-(256+128-sinMoveKindInter[SIN_CRAFTITEM]),47,lpForceTitle,0,0,111,32);	
	}

	//////////요기는 아이템 에이징 

	DrawSprite(0-(256+128-sinMoveKindInter[SIN_AGING]),90+sinInterHeight2,cShop.lpShopMain,0,0,344,264);
	DrawSprite(225-(256+128-sinMoveKindInter[SIN_AGING]),47,cShop.lpTitle_Aging,0,0,111,32);	
	DrawSprite(18-(256+128-sinMoveKindInter[SIN_AGING]),135+sinInterHeight2,cShop.lpCraftItemMain , 0,0,320,208);
	DrawSprite(200-(256+128-sinMoveKindInter[SIN_AGING]),314+sinInterHeight2,cShop.lpGoldEdit,0,0,98,21);

	//////////요기는 아이템 제련
	DrawSprite( 0-(256+128-sinMoveKindInter[SIN_SMELTING]), 90+sinInterHeight2, cShop.lpShopMain, 0, 0, 344, 264 );
	DrawSprite( 225-(256+128-sinMoveKindInter[SIN_SMELTING]), 47, cShop.lpTitle_SmeltingItem, 0, 0, 111, 32);
	DrawSprite( 18-(256+128-sinMoveKindInter[SIN_SMELTING]), 135+sinInterHeight2, cShop.lpSmeltingItemMain, 0, 0, 320, 208 );
	DrawSprite( 200-(256+128-sinMoveKindInter[SIN_SMELTING]), 314+sinInterHeight2, cShop.lpGoldEdit, 0, 0, 98, 21 );

	//////////요기는 아이템 제작
	DrawSprite( 0-(256+128-sinMoveKindInter[SIN_MANUFACTURE]), 90+sinInterHeight2, cShop.lpShopMain, 0, 0, 344, 264 );
	DrawSprite( 225-(256+128-sinMoveKindInter[SIN_MANUFACTURE]), 47, cShop.lpTitle_ManufactureItem, 0, 0, 111, 32);
	DrawSprite( 18-(256+128-sinMoveKindInter[SIN_MANUFACTURE]), 135+sinInterHeight2, cShop.lpManufactureItemMain, 0, 0, 320, 208 );
	DrawSprite( 200-(256+128-sinMoveKindInter[SIN_MANUFACTURE]), 314+sinInterHeight2, cShop.lpGoldEdit, 0, 0, 98, 21 );

	// 석지용 - 믹스쳐 리셋 창을 그리도록 한다.
	DrawSprite( 0-(256+128-sinMoveKindInter[SIN_MIXTURE_RESET]), 90+sinInterHeight2, cShop.lpShopMain, 0, 0, 344, 264 );
	DrawSprite( 225-(256+128-sinMoveKindInter[SIN_MIXTURE_RESET]), 47, cShop.lpMResetTitle, 0, 0, 111, 32 );
	DrawSprite( 18-(256+128-sinMoveKindInter[SIN_MIXTURE_RESET]), 135+sinInterHeight2, cShop.lpMResetMain, 0, 0, 320, 208 );

	for(i = 0 ; i < 9 ; i++)
		DrawSprite(21+(i*22)-(256+128-sinMoveKindInter[SIN_TRADE]),224+sinInterHeight2,lpCenterBox,0,0,22,21);

	if(sTrade.CheckFlag){ //체크이면 (내꺼 )
		if(!CheckDelayFlag)
			DrawSprite(DownTradePosi[T_CHECK_POSI][0]-(256+128-sinMoveKindInter[SIN_TRADE]),DownTradePosi[T_CHECK_POSI][1],lpCheck,0,0,26,26);
	}
	if(sTradeRecv.CheckFlag){ //상대편 
		DrawSprite(UpTradePosi[T_CHECK_POSI][0]-(256+128-sinMoveKindInter[SIN_TRADE]),UpTradePosi[T_CHECK_POSI][1],lpCheck,0,0,26,26);

	}
	if(TradeCheckFlag == T_CHECK_POSI ){  //툴팁 
		DrawSprite(DownTradePosi[T_CHECK_POSI][0]-(256+128-sinMoveKindInter[SIN_TRADE]),DownTradePosi[T_CHECK_POSI][1]-27,lpCheckInfo,0 ,0 , 47,27);

	}
	if(TradeCheckFlag  == T_CLOSE_POSI){ //닫기이면 
		DrawSprite(DownTradePosi[T_CLOSE_POSI][0]-(256+128-sinMoveKindInter[SIN_TRADE]),DownTradePosi[T_CLOSE_POSI][1],lpbuttonCancel,0,0,20,20);
		DrawSprite(DownTradePosi[T_CLOSE_POSI][0]-27-(256+128-sinMoveKindInter[SIN_TRADE]),DownTradePosi[T_CLOSE_POSI][1]-27,lpbuttonCancelInfo,0 ,0 , 77,27);

	}

	/////////// 사기방지 
	if(	CheckDelayFlag){
		//내꺼 
		DrawSprite(DownTradePosi[T_CHECK_POSI][0]-(256+128-sinMoveKindInter[SIN_TRADE]),DownTradePosi[T_CHECK_POSI][1],lpCheck_Glay,0,0,26,26);
		//남에꺼 
		//DrawSprite(UpTradePosi[T_CHECK_POSI][0]-(256+128-sinMoveKindInter[SIN_TRADE]),UpTradePosi[T_CHECK_POSI][1],lpCheck_Glay,0,0,26,26);

	}
		
	if(	CloseWareHouseFlag){
		DrawSprite(304-(256+128-sinMoveKindInter[SIN_WAREHOUSE]),313+sinInterHeight2,cShop.lpExit,0,0,20,20);	
		DrawSprite(304-(256+128-sinMoveKindInter[SIN_WAREHOUSE]),313-27+sinInterHeight2,cInvenTory.lpExitInfo,0 ,0 , 47,27);

	}

	/////////////////쉘텀 인덱스 그림을 먼저 보여준다 
	if(cAging.OpenFlag){
		for(i = 0 ;i < 4 ; i++){
			for(int t = 0 ; t < 3 ; t++)
				if(AgingLevelSheltomIndex[i][t]){
					DrawSprite(AgingLevelSheltomXY[i][t].x , AgingLevelSheltomXY[i][t].y ,
						cAging.lpGraySheltom[AgingLevelSheltomIndex[i][t]-1],
						0,0,22,22);
			}
		}
	}

	// pluto 제련 쉘텀 인덱스 그림
	if( SmeltingItem.OpenFlag )
	{
		int SheltomIndex = 0;
		if( S_smeltingItem.SmeltingItem[0].Flag )
		{
			SheltomIndex = SmeltingItem.GetSheltomIndex( S_smeltingItem.SmeltingItem[0].CODE );
			if( SheltomIndex )
			{
				// 쉘텀 한개만 나온다
				DrawSprite( AgingLevelSheltomXY[0][0].x + 22, AgingLevelSheltomXY[0][0].y + 30,  // 좌표는 같아서 걍 썻다 
					cAging.lpGraySheltom[SheltomIndex-1], 0, 0, 22, 22); // 같은 그림이라 걍 썻다
			}
		}
	}
	// pluto 제작 룬 인덱스 그림
/*	if( ManufactureItem.m_OpenFlag )
	{
		int RecipeIndex = 0;
		if( g_sManufactureItem.ManufactureItem[0].Flag )
		{
			RecipeIndex = ManufactureItem.GetRecipeIndex( g_sManufactureItem.ManufactureItem[0].CODE & sinITEM_MASK3 );
			for( i = 0; i < 4; i++ )
			{
				for( int a = 0; a < 3; a++ )
				{
					for( int r = 0; r < MAX_RECIPE_KIND; r++ )
					{
						if( g_sManufacture_Rune[r].RecipeCode == RecipeIndex)
						{
							if( g_sManufacture_Rune[r].NeedRuneIndex[i][a] )
							{
								DrawSprite( AgingLevelSheltomXY[i][a].x, AgingLevelSheltomXY[i][a].y,  // 좌표는 같아서 걍 썻다
									ManufactureItem.m_lpGrayRune[ g_sManufacture_Rune[RecipeIndex].NeedRuneIndex[i][a]-1 ], 0, 0, 22, 22);
							}
						}
					}
				}
			}
		}

	}*/

	////////////////아이템 배경 색 셋팅 (녹) No셋팅(적) Select(황)...
	switch(TradeColorIndex){
		case SET_ITEM_CHECK_COLOR: //셋팅영역 
			if(MouseItem.Flag){
				dsDrawColorBox( sinInvenColor[0] ,TradeColorRect.left-(256+128-sinMoveKindInter[SIN_TRADE]),TradeColorRect.top, TradeColorRect.right, TradeColorRect.bottom );
				dsDrawColorBox( sinInvenColor[0] ,TradeColorRect.left-(256+128-sinMoveKindInter[SIN_WAREHOUSE]),TradeColorRect.top, TradeColorRect.right, TradeColorRect.bottom );
				dsDrawColorBox( sinInvenColor[0] ,TradeColorRect.left-(256+128-sinMoveKindInter[SIN_CRAFTITEM]),TradeColorRect.top, TradeColorRect.right, TradeColorRect.bottom );
				dsDrawColorBox( sinInvenColor[0] ,TradeColorRect.left-(256+128-sinMoveKindInter[SIN_AGING]),TradeColorRect.top, TradeColorRect.right, TradeColorRect.bottom );
				// pluto 제련
				dsDrawColorBox( sinInvenColor[0], TradeColorRect.left-(256+128-sinMoveKindInter[SIN_SMELTING]), TradeColorRect.top, TradeColorRect.right, TradeColorRect.bottom );
				// pluto 제작
				dsDrawColorBox( sinInvenColor[0], TradeColorRect.left-(256+128-sinMoveKindInter[SIN_MANUFACTURE]), TradeColorRect.top, TradeColorRect.right, TradeColorRect.bottom );
				// 석지용 - 믹스쳐 리셋 
				dsDrawColorBox( sinInvenColor[0], TradeColorRect.left-(256+128-sinMoveKindInter[SIN_MIXTURE_RESET]), TradeColorRect.top, TradeColorRect.right, TradeColorRect.bottom );
			}
		break;
		case OVERLAP_ITEM_COLOR:   //겹친 아이템 영역 
			if(TradeCrashItemIndex[0]){
				dsDrawColorBox( sinInvenColor[1] ,sWareHouse.WareHouseItem[TradeCrashItemIndex[0]-1].x-(256+128-sinMoveKindInter[SIN_WAREHOUSE]),
					sWareHouse.WareHouseItem[TradeCrashItemIndex[0]-1].y, sWareHouse.WareHouseItem[TradeCrashItemIndex[0]-1].w, sWareHouse.WareHouseItem[TradeCrashItemIndex[0]-1].h );	
			}

		break;
		case NOT_SETTING_COLOR:    //세팅할수 없음 영역 
			dsDrawColorBox( sinInvenColor[2] ,TradeColorRect.left-(256+128-sinMoveKindInter[SIN_TRADE]),TradeColorRect.top, TradeColorRect.right, TradeColorRect.bottom );
			dsDrawColorBox( sinInvenColor[2] ,TradeColorRect.left-(256+128-sinMoveKindInter[SIN_WAREHOUSE]),TradeColorRect.top, TradeColorRect.right, TradeColorRect.bottom );
			dsDrawColorBox( sinInvenColor[2] ,TradeColorRect.left-(256+128-sinMoveKindInter[SIN_CRAFTITEM]),TradeColorRect.top, TradeColorRect.right, TradeColorRect.bottom );
			dsDrawColorBox( sinInvenColor[2] ,TradeColorRect.left-(256+128-sinMoveKindInter[SIN_AGING]),TradeColorRect.top, TradeColorRect.right, TradeColorRect.bottom );
			dsDrawColorBox( sinInvenColor[2] ,TradeColorRect.left-(256+128-sinMoveKindInter[SIN_SMELTING]),TradeColorRect.top, TradeColorRect.right, TradeColorRect.bottom );
			dsDrawColorBox( sinInvenColor[2] ,TradeColorRect.left-(256+128-sinMoveKindInter[SIN_MANUFACTURE]),TradeColorRect.top, TradeColorRect.right, TradeColorRect.bottom );
			dsDrawColorBox( sinInvenColor[2], TradeColorRect.left-(256+128-sinMoveKindInter[SIN_MIXTURE_RESET]), TradeColorRect.top, TradeColorRect.right, TradeColorRect.bottom ); // 석지용 - 믹스쳐 리셋 
		break;

		case NOT_AGING_ITEM:	//에이징 할수없는 아이템 (에이징 할때만 쓰인다 )
			dsDrawColorBox( sinInvenColor[2] ,TradeColorRect.left-(256+128-sinMoveKindInter[SIN_AGING]),TradeColorRect.top, TradeColorRect.right, TradeColorRect.bottom );

		case NOT_USE_COLOR:        //사용할수 없는 아이템 
			dsDrawColorBox( sinInvenColor[2] ,TradeColorRect.left-(256+128-sinMoveKindInter[SIN_TRADE]),TradeColorRect.top, TradeColorRect.right, TradeColorRect.bottom );
		break;

		case SELECT_ITEM_COLOR:
			for(i = 0 ; i < 10 ; i++){ //복사된 아이템을 표시해준다 
				if(CopyItemIndex7[i]){
					dsDrawColorBox( sinInvenColor[4] ,sWareHouse.WareHouseItem[CopyItemIndex7[i]-1].x-(256+128-sinMoveKindInter[SIN_WAREHOUSE]),
						sWareHouse.WareHouseItem[CopyItemIndex7[i]-1].y, sWareHouse.WareHouseItem[CopyItemIndex7[i]-1].w, sWareHouse.WareHouseItem[CopyItemIndex7[i]-1].h );	
				}
			}
			if(!SelectTradeItemIndex)break; //표시될 아이템 인덱스가 없으면 break
			if(ItemKindFlag == 1){ //내꺼
				dsDrawColorBox( sinInvenColor[4] ,sTrade.TradeItem[SelectTradeItemIndex-1].x-(256+128-sinMoveKindInter[SIN_TRADE]),
					sTrade.TradeItem[SelectTradeItemIndex-1].y, sTrade.TradeItem[SelectTradeItemIndex-1].w, sTrade.TradeItem[SelectTradeItemIndex-1].h );	
			}

			if(ItemKindFlag == 2){ //거래자꺼 
				dsDrawColorBox( sinInvenColor[4] ,sTradeRecv.TradeItem[SelectTradeItemIndex-1].x-(256+128-sinMoveKindInter[SIN_TRADE]),
					sTradeRecv.TradeItem[SelectTradeItemIndex-1].y-T_REVISION_Y, sTradeRecv.TradeItem[SelectTradeItemIndex-1].w, sTradeRecv.TradeItem[SelectTradeItemIndex-1].h );	
			}
			if(sWareHouse.WareHouseItem[SelectTradeItemIndex-1].Flag)
				dsDrawColorBox( sinInvenColor[4] ,sWareHouse.WareHouseItem[SelectTradeItemIndex-1].x-(256+128-sinMoveKindInter[SIN_WAREHOUSE]),
					sWareHouse.WareHouseItem[SelectTradeItemIndex-1].y, sWareHouse.WareHouseItem[SelectTradeItemIndex-1].w, sWareHouse.WareHouseItem[SelectTradeItemIndex-1].h );	
			if(sCraftItem.CraftItem[SelectTradeItemIndex-1].Flag)
				dsDrawColorBox( sinInvenColor[4] ,sCraftItem.CraftItem[SelectTradeItemIndex-1].x-(256+128-sinMoveKindInter[SIN_CRAFTITEM]),
					sCraftItem.CraftItem[SelectTradeItemIndex-1].y, sCraftItem.CraftItem[SelectTradeItemIndex-1].w, sCraftItem.CraftItem[SelectTradeItemIndex-1].h );	
			if(sAgingItem.AgingItem[SelectTradeItemIndex-1].Flag)
				dsDrawColorBox( sinInvenColor[4] ,sAgingItem.AgingItem[SelectTradeItemIndex-1].x-(256+128-sinMoveKindInter[SIN_AGING]),
					sAgingItem.AgingItem[SelectTradeItemIndex-1].y, sAgingItem.AgingItem[SelectTradeItemIndex-1].w, sAgingItem.AgingItem[SelectTradeItemIndex-1].h );
			// pluto 제련
			if( S_smeltingItem.SmeltingItem[SelectTradeItemIndex-1].Flag )
			{
				dsDrawColorBox( sinInvenColor[4], S_smeltingItem.SmeltingItem[SelectTradeItemIndex-1].x-(256+128-sinMoveKindInter[SIN_SMELTING]),
					S_smeltingItem.SmeltingItem[SelectTradeItemIndex-1].y, S_smeltingItem.SmeltingItem[SelectTradeItemIndex-1].w, S_smeltingItem.SmeltingItem[SelectTradeItemIndex-1].h );
			}
			// pluto 제작
			if( g_sManufactureItem.ManufactureItem[SelectTradeItemIndex-1].Flag )
			{
				dsDrawColorBox( sinInvenColor[4], g_sManufactureItem.ManufactureItem[SelectTradeItemIndex-1].x-(256+128-sinMoveKindInter[SIN_MANUFACTURE]),
					g_sManufactureItem.ManufactureItem[SelectTradeItemIndex-1].y, g_sManufactureItem.ManufactureItem[SelectTradeItemIndex-1].w, g_sManufactureItem.ManufactureItem[SelectTradeItemIndex-1].h );
			}
			// 석지용 - 믹스쳐 리셋
			if( sMixtureResetItem.MixtureResetItem[SelectTradeItemIndex-1].Flag )
			{
				dsDrawColorBox( sinInvenColor[4], sMixtureResetItem.MixtureResetItem[SelectTradeItemIndex-1].x-(256+128-sinMoveKindInter[SIN_MIXTURE_RESET]),
					sMixtureResetItem.MixtureResetItem[SelectTradeItemIndex-1].y, sMixtureResetItem.MixtureResetItem[SelectTradeItemIndex-1].w, sMixtureResetItem.MixtureResetItem[SelectTradeItemIndex-1].h );
			}

		break;

	}
	for(i=0 ; i < MAX_TRADE_ITEM ; i++){ //내아이템 
		if(sTrade.TradeItem[i].Flag){
			DrawSprite(sTrade.TradeItem[i].x-(256+128-sinMoveKindInter[SIN_TRADE]) , sTrade.TradeItem[i].y, sTrade.TradeItem[i].lpItem ,0,0, sTrade.TradeItem[i].w , sTrade.TradeItem[i].h);
			if((sTrade.TradeItem[i].CODE & sinITEM_MASK2) == sinFO1 ){
				cInvenTory.DrawForceOrbEffect(sTrade.TradeItem[i].x-(256+128-sinMoveKindInter[SIN_TRADE]) , sTrade.TradeItem[i].y);
			}

		}
		if(sTradeRecv.TradeItem[i].Flag){ //받은 아이템 
			DrawSprite(sTradeRecv.TradeItem[i].x-(256+128-sinMoveKindInter[SIN_TRADE]) , sTradeRecv.TradeItem[i].y-T_REVISION_Y, sTradeRecv.TradeItem[i].lpItem ,0,0, sTradeRecv.TradeItem[i].w ,							sTradeRecv.TradeItem[i].h);
			if((sTradeRecv.TradeItem[i].CODE & sinITEM_MASK2) == sinFO1 ){
				cInvenTory.DrawForceOrbEffect(sTradeRecv.TradeItem[i].x-(256+128-sinMoveKindInter[SIN_TRADE]) , sTradeRecv.TradeItem[i].y-T_REVISION_Y);
			}

		}
	}
	if(TradeRequestFlag){
		TradeSpotTimer++;
		if(TradeSpotTimer > 15){
			dsDrawColorBox( sinTradeColor ,TRADEBUTTONMAIN_X+5,TRADEBUTTONMAIN_Y+5, 160, 53 ); //깜빡인다 
			if(TradeSpotTimer >30)
				TradeSpotTimer = 0;

		}

		dsDrawTexImage(MatTradebuttonMain,TRADEBUTTONMAIN_X,TRADEBUTTONMAIN_Y,256,64,255);
		if(RequestTradeButtonFlag == 1) //거래 
			DrawSprite(TRADEBUTTONMAIN_X+140 , TRADEBUTTONMAIN_Y+11, lpbuttonOk ,0,0, 23,23);
		if(RequestTradeButtonFlag == 2) //캔슬 
			dsDrawTexImage(MatTradebuttonCancel,TRADEBUTTONMAIN_X+144,TRADEBUTTONMAIN_Y+37,16,16,255);

	}

	////////////////////창고 
	for(i=0 ; i < 100 ; i++){ //창고 아이템   
		if(sWareHouse.WareHouseItem[i].Flag){
			DrawSprite(sWareHouse.WareHouseItem[i].x-(256+128-sinMoveKindInter[SIN_WAREHOUSE]) , sWareHouse.WareHouseItem[i].y, sWareHouse.WareHouseItem[i].lpItem ,0,0, sWareHouse.WareHouseItem[i].w						, sWareHouse.WareHouseItem[i].h);
			if((sWareHouse.WareHouseItem[i].CODE & sinITEM_MASK2) == sinFO1 ){
				cInvenTory.DrawForceOrbEffect(sWareHouse.WareHouseItem[i].x-(256+128-sinMoveKindInter[SIN_WAREHOUSE]), sWareHouse.WareHouseItem[i].y);
			}

		}
	}

	DrawSprite(231-(256+128-sinMoveKindInter[SIN_WAREHOUSE]) , 243+sinInterHeight2, cShop.lpGoldEdit ,0,0, 99 , 24);
	DrawSprite(231-(256+128-sinMoveKindInter[SIN_WAREHOUSE]) , 241+sinInterHeight2, cShop.lpWeightEdit ,0,0, 20 , 20);
	DrawSprite(243-(256+128-sinMoveKindInter[SIN_WAREHOUSE]) , 100, cShop.lpTitle_WareHouse ,0,0, 79 ,40);
	//DrawSprite(225-(256+128-sinMoveKindInter[SIN_WAREHOUSE]) , 98+sinInterHeight2, cShop.lpTitle_WareHouse ,0,0, 111 , 32);

	//////에이징이 될수있으면 불이 들어온다 
	if(AgingCheckSheltomFlag){
		//dsDrawColorBox(sinInvenColor[4],158-(256+128-sinMoveKindInter[SIN_AGING]),222,26,26);
		DrawSprite(158-(256+128-sinMoveKindInter[SIN_AGING]),222+sinInterHeight2,cShop.lpCraftItemButton,0,0,26,26);	

	}

	/////////////////// 아이템 조합 
	if(CraftItemButtonIndex == 1){  
		DrawSprite(158-(256+128-sinMoveKindInter[SIN_CRAFTITEM]),222+sinInterHeight2,cShop.lpCraftItemButton,0,0,26,26);	
		if(cCraftItem.ForceFlag){
			DrawSprite(148-(256+128-sinMoveKindInter[SIN_CRAFTITEM])-27,222-27+sinInterHeight2,lpForceButtonInfo,0 ,0 , 100,27);
		}
		else{
			DrawSprite(158-(256+128-sinMoveKindInter[SIN_CRAFTITEM])-27,222-27+sinInterHeight2,cShop.lpCraftItemButtonInfo,0 ,0 , 80,27);
		}
		// pluto 제련
		if ( SmeltingItem.OpenFlag )
		{
			DrawSprite(158-(256+128-sinMoveKindInter[SIN_SMELTING]),222+sinInterHeight2,cShop.lpCraftItemButton,0,0,26,26);	
			DrawSprite(158-(256+128-sinMoveKindInter[SIN_SMELTING])-27,222-27+sinInterHeight2,cShop.lpSmeltingItemButtonInfo,0 ,0 , 80,27);
		}
		// pluto 제작
		if( ManufactureItem.m_OpenFlag )
		{
			DrawSprite(158-(256+128-sinMoveKindInter[SIN_MANUFACTURE]),222+sinInterHeight2,cShop.lpCraftItemButton,0,0,26,26);	
			DrawSprite(158-(256+128-sinMoveKindInter[SIN_MANUFACTURE])-27,222-27+sinInterHeight2,cShop.lpManufactureItemButtonInfo,0 ,0 , 80,27);
		}

	}

	// 석지용 - 믹스쳐 리셋이 가능한 상태면 +버튼이 활성화된다.
	if( MixtureResetCheckFlag )		
		DrawSprite(158-(256+128-sinMoveKindInter[SIN_MIXTURE_RESET]),221+sinInterHeight2,cShop.lpMResetButton,0,0,26,26);	

	// 석지용 - 조합버튼 위에 마우스가 위치하고, 믹스쳐 리셋이 가능한 상태라면 조합 메시지 띄우기
	if( CraftItemButtonIndex == 1 && MixtureResetCheckFlag )
	{		
		//DrawSprite(158-(256+128-sinMoveKindInter[SIN_MIXTURE_RESET]),222+sinInterHeight2,cShop.lpCraftItemButton,0,0,26,26);	
		DrawSprite(158-(256+128-sinMoveKindInter[SIN_MIXTURE_RESET])-27,222-27+sinInterHeight2,cShop.lpMResetButtonInfo,0 ,0 , 80,27);
	}

	if(CraftItemButtonIndex == 1 && AgingCheckSheltomFlag){ 
		DrawSprite(158-(256+128-sinMoveKindInter[SIN_AGING]),222+sinInterHeight2,cShop.lpCraftItemButton,0,0,26,26);	
		DrawSprite(158-(256+128-sinMoveKindInter[SIN_AGING])-27,222-27+sinInterHeight2,cShop.lpAging_Info,0 ,0 , 80,27);

	}

	if(	CraftItemButtonIndex == 2){ 
		DrawSprite(304-(256+128-sinMoveKindInter[SIN_CRAFTITEM])+2-2,313+1+sinInterHeight2,cShop.lpExit,0,0,20,20);	
		DrawSprite(304-(256+128-sinMoveKindInter[SIN_CRAFTITEM])-2,313-27+sinInterHeight2,cInvenTory.lpExitInfo,0 ,0 , 47,27);
		DrawSprite(304-(256+128-sinMoveKindInter[SIN_AGING])+2-2,313+1+sinInterHeight2,cShop.lpExit,0,0,20,20);	
		DrawSprite(304-(256+128-sinMoveKindInter[SIN_AGING])-2,313-27+sinInterHeight2,cInvenTory.lpExitInfo,0 ,0 , 47,27);
		//pluto 제련
		DrawSprite( 304-(256+128-sinMoveKindInter[SIN_SMELTING])+2-2, 313+1+sinInterHeight2, cShop.lpExit, 0, 0, 20, 20 );
		DrawSprite( 304-(256+128-sinMoveKindInter[SIN_SMELTING])-2, 313-27+sinInterHeight2, cInvenTory.lpExitInfo, 0, 0, 47, 27);
		// pluto 제작
		DrawSprite( 304-(256+128-sinMoveKindInter[SIN_MANUFACTURE])+2-2, 313+1+sinInterHeight2, cShop.lpExit, 0, 0, 20, 20 );
		DrawSprite( 304-(256+128-sinMoveKindInter[SIN_MANUFACTURE])-2, 313-27+sinInterHeight2, cInvenTory.lpExitInfo, 0, 0, 47, 27);
		// 석지용 - 믹스쳐 리셋 닫기 버튼 활성화
		DrawSprite(304-(256+128-sinMoveKindInter[SIN_MIXTURE_RESET])+2-2,313+1+sinInterHeight2,cShop.lpExit,0,0,20,20);	
		DrawSprite(304-(256+128-sinMoveKindInter[SIN_MIXTURE_RESET])-2,313-27+sinInterHeight2,cInvenTory.lpExitInfo,0 ,0 , 47,27);
	}

	for(i=0;i<15;i++){ //크래프트 아이템  
		if(sCraftItem.CraftItem[i].Flag){
			DrawSprite(sCraftItem.CraftItem[i].x-(256+128-sinMoveKindInter[SIN_CRAFTITEM]) , sCraftItem.CraftItem[i].y, sCraftItem.CraftItem[i].lpItem ,0,0, sCraftItem.CraftItem[i].w , sCraftItem.CraftItem[i].h);

		}
	}

	for(i=0;i<15;i++){ //에이징 아이템   
		if(sAgingItem.AgingItem[i].Flag){
			DrawSprite(sAgingItem.AgingItem[i].x-(256+128-sinMoveKindInter[SIN_AGING]) , sAgingItem.AgingItem[i].y, sAgingItem.AgingItem[i].lpItem ,0,0, sAgingItem.AgingItem[i].w , sAgingItem.AgingItem[i].h);

		}
	}

	// 석지용 - 믹스쳐 리셋 아이템 그리기??
	for(i=0; i<2; i++)
	{
		if( sMixtureResetItem.MixtureResetItem[i].Flag )
		{
			DrawSprite( sMixtureResetItem.MixtureResetItem[i].x-(256+128-sinMoveKindInter[SIN_MIXTURE_RESET]), sMixtureResetItem.MixtureResetItem[i].y,
				sMixtureResetItem.MixtureResetItem[i].lpItem, 0, 0, sMixtureResetItem.MixtureResetItem[i].w, sMixtureResetItem.MixtureResetItem[i].h );
		}
	}

	// pluto 제련
	for( i = 0; i < 5; i++ )
	{
		if( S_smeltingItem.SmeltingItem[i].Flag )
		{
			DrawSprite( S_smeltingItem.SmeltingItem[i].x-(256+128-sinMoveKindInter[SIN_SMELTING]), S_smeltingItem.SmeltingItem[i].y, S_smeltingItem.SmeltingItem[i].lpItem, 0, 0, S_smeltingItem.SmeltingItem[i].w, S_smeltingItem.SmeltingItem[i].h);
		}
	}
	// pluto 제작
	for( i = 0; i < 4; i++ )
	{
		if( g_sManufactureItem.ManufactureItem[i].Flag )
		{
			DrawSprite( g_sManufactureItem.ManufactureItem[i].x-(256+128-sinMoveKindInter[SIN_MANUFACTURE]), g_sManufactureItem.ManufactureItem[i].y, g_sManufactureItem.ManufactureItem[i].lpItem, 0, 0, g_sManufactureItem.ManufactureItem[i].w, g_sManufactureItem.ManufactureItem[i].h);
		}
	}


/*	
	CraftItemMessageSize.x = 15; //임시 
	CraftItemMessageSize.y = 5;
*/
	CraftItemMessageBoxPosi.x = 230;
	CraftItemMessageBoxPosi.y = 200;

	if(ShowItemCraftMessageFlag || ShowItemAgingMessageFlag || ShowItemSmeltingMessageFlag || ShowItemManufactureMessageFlag || ShowItemOverSpace){    //아이템이 조합됬을때 메세지를 보여준다 
		for(i = 0; i < CraftItemMessageSize.x ; i++){
			for(int j = 0; j< CraftItemMessageSize.y ; j++){
				if(i == 0 && j == 0 ) //좌측상단 
					dsDrawTexImage( cItem.MatItemInfoBox_TopLeft , CraftItemMessageBoxPosi.x+(i*16) , CraftItemMessageBoxPosi.y+(j*16), 16, 16 , 255 );
				if(j == 0 && i !=0 && i+1 < CraftItemMessageSize.x ) //가운데
					dsDrawTexImage( cItem.MatItemInfoBox_TopCenter , CraftItemMessageBoxPosi.x+(i*16) , CraftItemMessageBoxPosi.y+(j*16), 16, 16 , 255 );
				if(j == 0 && i+1 == CraftItemMessageSize.x) //우측상단
					dsDrawTexImage( cItem.MatItemInfoBox_TopRight , CraftItemMessageBoxPosi.x+(i*16) , CraftItemMessageBoxPosi.y+(j*16), 16, 16 , 255 );

				if(i == 0 && j != 0 && j+1 != CraftItemMessageSize.y) //좌측 줄거리 
					dsDrawTexImage( cItem.MatItemInfoBox_Left , CraftItemMessageBoxPosi.x+(i*16) , CraftItemMessageBoxPosi.y+(j*16), 16, 16 , 255 );
				if(i != 0 && j != 0 && i+1 !=CraftItemMessageSize.x && j+1 !=CraftItemMessageSize.y) //가운데 토막
					dsDrawTexImage( cItem.MatItemInfoBox_Center , CraftItemMessageBoxPosi.x+(i*16) , CraftItemMessageBoxPosi.y+(j*16), 16, 16 , 255 );
				if(i+1 == CraftItemMessageSize.x && j != 0 && j+1 != CraftItemMessageSize.y) //우측 줄거리 
					dsDrawTexImage( cItem.MatItemInfoBox_Right , CraftItemMessageBoxPosi.x+(i*16) , CraftItemMessageBoxPosi.y+(j*16), 16, 16 , 255 );
				
				if(i == 0 && j+1 == CraftItemMessageSize.y) //밑바닥 왼쪽
					dsDrawTexImage( cItem.MatItemInfoBox_BottomLeft , CraftItemMessageBoxPosi.x+(i*16) , CraftItemMessageBoxPosi.y+(j*16), 16, 16 , 255 );
				if(i != 0 && j+1 == CraftItemMessageSize.y && i+1 !=CraftItemMessageSize.x)//밑바닥 가운데
					dsDrawTexImage( cItem.MatItemInfoBox_BottomCenter , CraftItemMessageBoxPosi.x+(i*16) , CraftItemMessageBoxPosi.y+(j*16), 16, 16 , 255 );
				if(j+1 == CraftItemMessageSize.y && i+1 ==CraftItemMessageSize.x)//밑바닥 오른쪽 
					dsDrawTexImage( cItem.MatItemInfoBox_BottomRight , CraftItemMessageBoxPosi.x+(i*16) , CraftItemMessageBoxPosi.y+(j*16), 16, 16 , 255 );
			}
		}
	}
}
/*----------------------------------------------------------------------------*
*							     메인  
*-----------------------------------------------------------------------------*/	
DWORD	dwTradeDelayTime = 0;
DWORD	dwTradeDelayTime2 = 0;
DWORD	dwTradeDelayTime3 = 0;
DWORD	dwTradeDelayTime4 = 0;

DWORD   dwCraftItemInfoDelayTime = 0;
DWORD   dwMixDelayTime = 0;
DWORD	dwMixItemColyTime = 0;
int		MixCancelButtonDelayFlag = 0;
DWORD   dwForceDelayTime = 0;

void cTRADE::Main()
{


	//ForceOrb 응답이 없을경우 닫아준다
	if(cCraftItem.ForceFlag){
		if(MixItemNoCopyFlag){
			dwForceDelayTime++;
			if(dwForceDelayTime > 70*5){
				if(MixItemNoCopyFlag)MixItemNoCopyFlag = 0;
				dwForceDelayTime = 0;
			}
		}
		else dwForceDelayTime = 0;
	}

	//sCraftItem_Info[0].ElementKind[7] = 1; 테스트용
	///////////믹스 복사 방지 플렉
	if(CheckMixItemCopyFlag){
		dwMixItemColyTime++;
		if(dwMixItemColyTime > 70*60*2){ //2분동안 서버에서 응답이 없으면 체크를 풀어준다 
			dwMixItemColyTime = 0;
			CheckMixItemCopyFlag = 0;
		}
	}
	else
		dwMixItemColyTime = 0;

	///////////////// 믹스와 에이징 이펙트시 취소를 못하게 하기위해서
	if(MixCancelButtonDelayFlag){
		dwMixDelayTime++;
		if(dwMixDelayTime > 70*10){
			MixCancelButtonDelayFlag = 0;
			dwMixDelayTime = 0;
		}
	}
	else{
		dwMixDelayTime = 0;

	}

	//cCraftItem.CheckHackCraftItem(); //믹스쳐 아이템의 해킹 방지 (서버에서 하는방식으로 바뀌어서 의미가없다)
	cAging.CheckAgingData(); //에이징 아이템의 해킹 방지 
	int i,j=0;
	CraftItemButtonIndex = 0;
	RequestTradeButtonFlag = 0;
	CloseWareHouseFlag = 0;
    TradeCheckFlag = 0; //초기화 
	if(TradeRequestFlag){ //거래 요구 
		if(pRealCursorPos.x >= TRADEBUTTONMAIN_X+138 && pRealCursorPos.x <= TRADEBUTTONMAIN_X+138+16 &&
			pRealCursorPos.y >= TRADEBUTTONMAIN_Y+12 && pRealCursorPos.y <= TRADEBUTTONMAIN_Y+12+16){
			RequestTradeButtonFlag = 1;

		}
		if(pRealCursorPos.x >= TRADEBUTTONMAIN_X+138 && pRealCursorPos.x <= TRADEBUTTONMAIN_X+138+16 &&
			pRealCursorPos.y >= TRADEBUTTONMAIN_Y+33 && pRealCursorPos.y <= TRADEBUTTONMAIN_Y+33+16){
			RequestTradeButtonFlag =2;
		}
		dwTradeDelayTime3++;
		if(dwTradeDelayTime3 >= 70*10){
			TradeRequestFlag = 0;	//10초후 창을 닫아준다 
			dwTradeDelayTime3 = 0;

		}
	}

	if(OpenFlag){	//체크 		
		//무게를 셋팅 

		//쳌크 
		if(pCursorPos.x >= DownTradePosi[T_CHECK_POSI][0] && pCursorPos.x <= DownTradePosi[T_CHECK_POSI][2] &&
			pCursorPos.y >= DownTradePosi[T_CHECK_POSI][1] && pCursorPos.y <= DownTradePosi[T_CHECK_POSI][3]){
			TradeCheckFlag  = T_CHECK_POSI;
		}
		//닫기 
		if(pCursorPos.x >= DownTradePosi[T_CLOSE_POSI][0] && pCursorPos.x <= DownTradePosi[T_CLOSE_POSI][2] &&
			pCursorPos.y >= DownTradePosi[T_CLOSE_POSI][1] && pCursorPos.y <= DownTradePosi[T_CLOSE_POSI][3]){
			TradeCheckFlag  = T_CLOSE_POSI;
		}
		//돈을 인벤으로 이동 
		if(pCursorPos.x >= DownTradePosi[T_MONEY_POSI][0]+10 && pCursorPos.x <= DownTradePosi[T_MONEY_POSI][2]+10 &&
			pCursorPos.y >= DownTradePosi[T_MONEY_POSI][1] && pCursorPos.y <= DownTradePosi[T_MONEY_POSI][3]){
			TradeCheckFlag  = T_MONEY_POSI;
		}
		
		PickUpTradeItem(pCursorPos.x , pCursorPos.y); //집을수 있는 아이템을 표시한다 
		if(MouseItem.Flag)
			SetTradeItemAreaCheck(&MouseItem); //셋팅될 공간을 찾는다
		
		/////////////// 승인을 위해 딜레이를 준다 
		if(CheckDelayFlag){	
			dwTradeDelayTime++;
			if(dwTradeDelayTime >=70*2){ 
				CheckDelayFlag = 0;
				dwTradeDelayTime = 0;

			}
			

		}
		
		if(sTrade.CheckFlag && sTradeRecv.CheckFlag){ //트레이드가 5초동안 안될경우에는 취소할수있게해준다
			dwTradeDelayTime2++;
			if(dwTradeDelayTime2 >= 70*5){
				dwTradeDelayTime2 = 0;
				sinCancelFlag = 1;

			}

		}
		else								 
			sinCancelFlag = 0; //체크를 하고 트레이드가 안됬을때 5초가 안되면 값을 셋팅하지않는다 
 
		if(ItemPickUpFlag){ //사기 방지를 위한 딜레이 
			dwTradeDelayTime4++;
			if(dwTradeDelayTime4 > 70*2){
				dwTradeDelayTime4 = 0;
				ItemPickUpFlag = 0;

			}
		}

	}

	////////////////창고 
	if(cWareHouse.OpenFlag){
		//창고 닫기 
		if(pCursorPos.x >= DownTradePosi[T_CLOSE_POSI][0] && pCursorPos.x <= DownTradePosi[T_CLOSE_POSI][2] &&
			pCursorPos.y >= DownTradePosi[T_CLOSE_POSI][1] && pCursorPos.y <= DownTradePosi[T_CLOSE_POSI][3]){
				CloseWareHouseFlag  = 1;
		}
		cWareHouse.PickUpWareHouseItem(pCursorPos.x , pCursorPos.y); //집을수 있는 아이템을 표시한다 
		if(MouseItem.Flag){
			cWareHouse.SetWareHouseItemAreaCheck(&MouseItem); //셋팅될 공간을 찾는다

		}
		sWareHouse.BuyAreaCount = 10; // 박재원 - 기존 창고 무게 400에서 1000으로 조정함.
		sWareHouse.Weight[1] = (sWareHouse.BuyAreaCount * 100)+196; //임시로 조정한다 
	
		///////// 복사된 아이템을 표시해준다 
		if(smConfig.DebugMode){
			for(i = 0 ; i < 10 ; i++){ //초기화 
				CopyItemIndex7[i] = 0;
	
			}
		
			for(i=0 ; i < 100 ; i++){
				if(sWareHouse.WareHouseItem[i].Flag){
					for(j = 0; j < 100 ; j++){
						if(sWareHouse.WareHouseItem[j].Flag){
							if(i == j)continue; //자신의 아이템은 찾지않는다
							if(CompareItems(&sWareHouse.WareHouseItem[i].sItemInfo ,  &sWareHouse.WareHouseItem[j].sItemInfo)){
								for(int k = 0 ; k < 10 ; k++){
									if(!CopyItemIndex7[k]){
										CopyItemIndex7[k] = i+1;
										CopyItemIndex7[k+1] = j+1;
										break;
									}
								}
							}
						}
					}
				}
			}
		}
	}
	/////////////// 아이템 조합 
	if(cCraftItem.OpenFlag || cAging.OpenFlag || SmeltingItem.OpenFlag || ManufactureItem.m_OpenFlag || cMixtureReset.OpenFlag){ // 석지용 - 믹스쳐 리셋 추가
		if(!MouseItem.Flag){
			if(pCursorPos.x >= 158 && pCursorPos.x <= 158+26 &&
				pCursorPos.y >= 222+sinInterHeight2 && pCursorPos.y <= 222+26+sinInterHeight2){
					if(!MixCancelButtonDelayFlag){
						CraftItemButtonIndex  = 1; //조합
					}
			}

			if(pCursorPos.x >= DownTradePosi[T_CLOSE_POSI][0] && pCursorPos.x <= DownTradePosi[T_CLOSE_POSI][2] &&
				pCursorPos.y >= DownTradePosi[T_CLOSE_POSI][1] && pCursorPos.y <= DownTradePosi[T_CLOSE_POSI][3]){
					if(!MixCancelButtonDelayFlag){
						CraftItemButtonIndex  = 2; //닫기 
					}
			}
			cCraftItem.PickUpCraftItem(pCursorPos.x , pCursorPos.y);
		}
		if(MouseItem.Flag){
			if(cCraftItem.OpenFlag)
				cCraftItem.SetCraftItemAreaCheck(&MouseItem);

		}

	}
	////////////// 에이징 아이템 
	if(cAging.OpenFlag){
		if(MouseItem.Flag){
			cAging.SetAgingItemAreaCheck(&MouseItem);
		}
		else
			cCraftItem.PickUpCraftItem(pCursorPos.x , pCursorPos.y,0,1);//아이템을 집는다 

	}
	// pluto 제련
	if( SmeltingItem.OpenFlag )
	{
		if( MouseItem.Flag )
		{
			if( (MouseItem.sItemInfo.CODE & sinITEM_MASK2) == sinPR1 || (MouseItem.sItemInfo.CODE & sinITEM_MASK2) == sinPR2 ) // 같은 광석만 올릴수 있다
			{
				if( S_smeltingItem.SmeltingItem[0].Flag && S_smeltingItem.SmeltingItem[0].CODE != MouseItem.CODE )
				{
					return;
				}

				// 장별 - 제련 시스템 수정
				if( S_smeltingItem.SmeltingItem[0].Flag && S_smeltingItem.SmeltingItem[1].Flag && S_smeltingItem.SmeltingItem[2].Flag ) return;

			}
			SmeltingItem.SetSmeltingItemAreaCheck( &MouseItem );
		}
		else
		{
			SmeltingItem.PickUpSmeltingItem( pCursorPos.x, pCursorPos.y );
		}
	}
	// pluto 제작
	if( ManufactureItem.m_OpenFlag )
	{
		if( MouseItem.Flag )
		{
			ManufactureItem.SetManufactureItemAreaCheck( &MouseItem );
		}
		else
		{
			ManufactureItem.PickUpManufactureItem( pCursorPos.x, pCursorPos.y );
		}
	}

	// 석지용 - 믹스쳐 리셋 창이 열려있을때 처리
	if( cMixtureReset.OpenFlag )
	{
		if( MouseItem.Flag )	// 아이템을 들었을 때
			cMixtureReset.SetMixtureItemResetAreaCheck( &MouseItem );
		else	// 아이템을 들고있지 않을 때
			cMixtureReset.PickUpMixtureResetItem( pCursorPos.x, pCursorPos.y );
	}

	if(ShowItemCraftMessageFlag || ShowItemAgingMessageFlag || ShowItemSmeltingMessageFlag || ShowItemManufactureMessageFlag || ShowItemOverSpace){ //정보를 10초여준다 
		dwCraftItemInfoDelayTime++;
		if(dwCraftItemInfoDelayTime >= 70*10){
			ShowItemCraftMessageFlag = 0;
			ShowItemAgingMessageFlag = 0;
			dwCraftItemInfoDelayTime = 0;
			ShowItemSmeltingMessageFlag = 0;	// pluto 아이템이 제련 됬을때
			ShowItemManufactureMessageFlag = 0; // pluto 아이템이 제작 됬을때
			ShowItemOverSpace = 0;	// pluto 공간부족

		}

	}
	///////////날치기로 대충 막아놨다 
	if(cWareHouse.OpenFlag)
		CloseWareHouseCheckFlag2 = 1;


	if(!CloseWareHouseCheckFlag3 && !cWareHouse.OpenFlag && CloseWareHouseCheckFlag2 == 1){
		cWareHouse.RestoreInvenItem();
		CloseWareHouseCheckFlag3 = 0;
		CloseWareHouseCheckFlag2 = 0;

	}

}
/*----------------------------------------------------------------------------*
*							     종료 
*-----------------------------------------------------------------------------*/	
void cTRADE::Close()
{

}
/*----------------------------------------------------------------------------*
*							   LButtonDown
*-----------------------------------------------------------------------------*/	
void cTRADE::LButtonDown(int x , int y)
{
	if(LbuttonUpCheckFlag)return; //인벤토리에서 아이템을 잡았을 경우에는 메세지를 건너뛴다  

	int i,TempFlag = 0;
	if(TradeCheckFlag  == T_CHECK_POSI){ //거래 확인
		if(!MouseItem.Flag){
			if(!CheckDelayFlag){ //딜레이가 있을때는 확인 버튼을 누를수없다 
				if(!ItemPickUpFlag){
					if(CheckTradeItemSet()){
						sTrade.CheckFlag = 1;
						SendTradeItem(TradeCharCode); //서버에 메세지를 보낸다 
						CheckTradeButtonOk(); //키값을 보낸다 
					}
				}
			}
		}
	}
	if(TradeCheckFlag  == T_CLOSE_POSI){
		if(!dwTradeMaskTime || sinCancelFlag ){
			SendRequestTrade(TradeCharCode,3); //서버에 메세지를 보낸다 
			cTrade.OpenFlag = 0;
			cInvenTory.OpenFlag =0;
			CancelTradeItem();
			sinCancelFlag = 0;
		}
	
	}

	if(RequestTradeButtonFlag == 2){//거래 취소 
		//취소 메세지 보내는거 잠시 대기 
		TradeRequestFlag = 0;
		TradeCharCode = 0;
		dwTradeDelayTime3 = 0; //타이머 초기화 

	}

	if(RequestTradeButtonFlag == 1){//거래 승인 
		if(!MouseItem.Flag){
			//교환 거래 거리 확인
			if ( GetTradeDistanceFromCode( TradeCharCode )==TRUE )
				SendRequestTrade(TradeCharCode,1); //서버에 메세지를 보낸다 
			TradeRequestFlag = 0;
			dwTradeDelayTime3 = 0; //타이머 초기화
		}
	}
	////////아이템 관련 
	
	if(cTrade.OpenFlag){
		if(MouseItem.Flag && !sTrade.CheckFlag){
			cInvenTory.CheckRequireItemToSet(&MouseItem); //셋팅될수있는 아이템인지 값을 넣어준다 
			if(TradeColorIndex == NO_BOX_COLOR || TradeColorIndex == NOT_SETTING_COLOR)return; //셋팅될수없으면 리턴한다 
			if(!LastSetTradeItem(&MouseItem)){ //아이템이 바뀌지 않았으면 그냥 인벤토리에 셋팅한다
			}
		}
		else{
			if(!TradeItemSucessFlag &&  !TradeSendSucessFlag && !sTrade.CheckFlag){ //아이템 복사를 막기위해 플랙값이 있을때는 아이템을 집을수없다 
				PickUpTradeItem(pCursorPos.x,pCursorPos.y,1);//아이템을 집는다 
				if(TradeCheckFlag  == T_MONEY_POSI && !sTrade.CheckFlag)
					cMessageBox.ShowMessage2(MESSAGE_MOVE_MONEY_TRADE);
				
				}
				
			}

	}

	///////////////////////// 창고 
	if(CloseWareHouseFlag){
		cWareHouse.CloseWareHouse();

	}
	if(cWareHouse.OpenFlag){
		if(MouseItem.Flag){
		
			cInvenTory.CheckRequireItemToSet(&MouseItem); //셋팅될수있는 아이템인지 값을 넣어준다 
			if(TradeColorIndex == NO_BOX_COLOR || TradeColorIndex == NOT_SETTING_COLOR)return; //셋팅될수없으면 리턴한다 
			if(!cWareHouse.ChangeWareHouseItem(&MouseItem))
				cWareHouse.LastSetWareHouseItem(&MouseItem);

		}
		else
			cWareHouse.PickUpWareHouseItem(pCursorPos.x,pCursorPos.y,1);//아이템을 집는다
	
		if(pCursorPos.x >= 231 && pCursorPos.x <= 231+20 &&
			pCursorPos.y >= 213+sinInterHeight2 && pCursorPos.y <= 213+20+sinInterHeight2){
				cMessageBox.ShowMessage2(MESSAGE_MOVE_MONEY_RECV);
		}


	}
	//////////////////////// 아이템 조합 (믹스쳐)
	int CloseButtonFlag = 0;
	int SheltomCount = 0;    //쉘텀이나 씰아이템을  한번 더 체크한다.
	if(cCraftItem.OpenFlag)
	{ 
		if(MouseItem.Flag)
		{
			//물약은 올리지 못한다. (패키지 아이템도 올리지 못한다
		 	if((MouseItem.sItemInfo.CODE & sinITEM_MASK2)==sinPM1 || (MouseItem.sItemInfo.CODE & sinITEM_MASK2)==sinPL1 || 
			   (MouseItem.sItemInfo.CODE & sinITEM_MASK2)==sinPS1 || (MouseItem.sItemInfo.CODE & sinITEM_MASK2)==sinBI1 ||
			   (MouseItem.sItemInfo.CODE & sinITEM_MASK2)==sinBI2 || (MouseItem.sItemInfo.CODE & sinITEM_MASK2)==sinWR1 || // 박재원 - 무기 제작서도 올리지 못한다.
			   (MouseItem.sItemInfo.CODE & sinITEM_MASK2)==sinDR1 || (MouseItem.sItemInfo.CODE & sinITEM_MASK2)==sinGP1 || // 박재원 : 방어구 제작서, 크리스탈도 올리지 못한다.
			   (MouseItem.sItemInfo.CODE & sinITEM_MASK2)==sinCH1 || // 박재원 : 초콜릿, 사탕 조합금지 
				//파티복 코스튬
				//박재원 : 송편아뮬렛sin31과 파티복 sin31 아이템이 겹쳐서 구분함
				//박재원 : 산타아뮬렛sin32과 파티복 sin32 아이템이 겹쳐서 구분함
				//박재원 : 이벤트아뮬렛sin33과 파티복 sin33 아이템이 겹쳐서 구분함
			   (MouseItem.sItemInfo.CODE == (sinDA1|sin31))      || (MouseItem.sItemInfo.CODE == (sinDA2|sin31))      ||
			   (MouseItem.sItemInfo.CODE == (sinDA1|sin32))      || (MouseItem.sItemInfo.CODE == (sinDA2|sin32))      ||
			   (MouseItem.sItemInfo.CODE == (sinDA1|sin33))      || (MouseItem.sItemInfo.CODE == (sinDA2|sin33))      ||
			   (MouseItem.sItemInfo.CODE == (sinDA1|sin34))      || (MouseItem.sItemInfo.CODE == (sinDA2|sin34))      ||
			   (MouseItem.sItemInfo.CODE == (sinDA1|sin35))      || (MouseItem.sItemInfo.CODE == (sinDA2|sin35))      ||
			   (MouseItem.sItemInfo.CODE == (sinDA1|sin36))      || (MouseItem.sItemInfo.CODE == (sinDA2|sin36))      ||
			   (MouseItem.sItemInfo.CODE == (sinDA1|sin37))      || (MouseItem.sItemInfo.CODE == (sinDA2|sin37))      ||
			   (MouseItem.sItemInfo.CODE == (sinDA1|sin38))      || (MouseItem.sItemInfo.CODE == (sinDA2|sin38))      ||
			   (MouseItem.sItemInfo.CODE == (sinDA1|sin39))      || (MouseItem.sItemInfo.CODE == (sinDA2|sin39))      ||
			   (MouseItem.sItemInfo.CODE == (sinDA1|sin40))      || (MouseItem.sItemInfo.CODE == (sinDA2|sin40))      ||
			   (MouseItem.sItemInfo.CODE == (sinDA1|sin41))      || (MouseItem.sItemInfo.CODE == (sinDA2|sin41))      ||
			   (MouseItem.sItemInfo.CODE == (sinDA1|sin42))      || (MouseItem.sItemInfo.CODE == (sinDA2|sin42))      ||
			   (MouseItem.sItemInfo.CODE == (sinDA1|sin43))      || (MouseItem.sItemInfo.CODE == (sinDA2|sin43))      ||
			   (MouseItem.sItemInfo.CODE == (sinDA1|sin44))      || (MouseItem.sItemInfo.CODE == (sinDA2|sin44))      ||
			   (MouseItem.sItemInfo.CODE == (sinDA1|sin45))      || (MouseItem.sItemInfo.CODE == (sinDA2|sin45))      ||
			   (MouseItem.sItemInfo.CODE == (sinDA1|sin46))      || (MouseItem.sItemInfo.CODE == (sinDA2|sin46))      ||
			   (MouseItem.sItemInfo.CODE == (sinDA1|sin48))      || (MouseItem.sItemInfo.CODE == (sinDA2|sin48))      || //한복 Kyle수정
			   (MouseItem.sItemInfo.CODE == (sinOA2|sin31))      || (MouseItem.sItemInfo.CODE == (sinOA2|sin32))      || // 박재원 - 슈퍼 암릿(7일, 30일) 추가
			   (MouseItem.sItemInfo.CODE == (sinDB1|sin31))      || (MouseItem.sItemInfo.CODE == (sinDB1|sin32))      ||  // 박재원 - 스피드 부츠(7일, 30일) 추가
			   (MouseItem.sItemInfo.CODE == (sinDA1|sin54))      || (MouseItem.sItemInfo.CODE == (sinDA2|sin54))      ||  // 박재원 - 수영복 복장 추가 - 조합금지
			   (MouseItem.sItemInfo.CODE == (sinDA1|sin55))      || (MouseItem.sItemInfo.CODE == (sinDA2|sin55))      ||  // 박재원 - 수영복 복장 추가 - 조합금지
			   (MouseItem.sItemInfo.CODE == (sinDB1|sin33))	    || // 장별 - 스피드 부츠(1일) 추가
			   (MouseItem.sItemInfo.CODE == (sinOA2|sin33))		||		// 장별 - 슈퍼 암릿(1일)
			   (MouseItem.sItemInfo.CODE == (sinDB1|sin34))		|| // 장별 - 스피드 부츠(1시간) 추가
		       (MouseItem.sItemInfo.CODE == (sinOA2|sin34))) 		// 장별 - 슈퍼 암릿(1시간)

			   {
					TradeColorIndex = NOT_SETTING_COLOR;	
			   }
			//작성일 : 08.05.14
			//작성자 : 박재원
			//내  용 : 클랜치프링 아이템 조합 금지
			if( (MouseItem.sItemInfo.CODE & sinITEM_MASK2)==sinOR2 )
			{
				if( (MouseItem.sItemInfo.CODE & sinITEM_MASK3)==sin06 || (MouseItem.sItemInfo.CODE & sinITEM_MASK3)==sin07 ||
				    (MouseItem.sItemInfo.CODE & sinITEM_MASK3)==sin08 || (MouseItem.sItemInfo.CODE & sinITEM_MASK3)==sin09 ||
				    (MouseItem.sItemInfo.CODE & sinITEM_MASK3)==sin10 || (MouseItem.sItemInfo.CODE & sinITEM_MASK3)==sin11 ||
				    (MouseItem.sItemInfo.CODE & sinITEM_MASK3)==sin12 || (MouseItem.sItemInfo.CODE & sinITEM_MASK3)==sin13 ||
				    (MouseItem.sItemInfo.CODE & sinITEM_MASK3)==sin14 || (MouseItem.sItemInfo.CODE & sinITEM_MASK3)==sin15 ||
				    (MouseItem.sItemInfo.CODE & sinITEM_MASK3)==sin16 || (MouseItem.sItemInfo.CODE & sinITEM_MASK3)==sin17 ||
				    (MouseItem.sItemInfo.CODE & sinITEM_MASK3)==sin18 || (MouseItem.sItemInfo.CODE & sinITEM_MASK3)==sin19 ||
				    (MouseItem.sItemInfo.CODE & sinITEM_MASK3)==sin20 || (MouseItem.sItemInfo.CODE & sinITEM_MASK3)==sin21 ||
				    (MouseItem.sItemInfo.CODE & sinITEM_MASK3)==sin22 || (MouseItem.sItemInfo.CODE & sinITEM_MASK3)==sin23 ||
				    (MouseItem.sItemInfo.CODE & sinITEM_MASK3)==sin24 || (MouseItem.sItemInfo.CODE & sinITEM_MASK3)==sin25 ||
					(MouseItem.sItemInfo.CODE & sinITEM_MASK3)==sin27 || (MouseItem.sItemInfo.CODE & sinITEM_MASK3)==sin28 || // 박재원 - 이벤트링(7일), 산타링도 추가
					(MouseItem.sItemInfo.CODE & sinITEM_MASK3)==sin29 || (MouseItem.sItemInfo.CODE & sinITEM_MASK3)==sin30 ||  // 박재원 - 이벤트링(1시간), 이벤트링(1일)도 조합금지
					(MouseItem.sItemInfo.CODE & sinITEM_MASK3)==sin31 || (MouseItem.sItemInfo.CODE & sinITEM_MASK3)==sin32 || // 박재원 - 보스 몬스터 링 추가(바벨, 퓨리)
					(MouseItem.sItemInfo.CODE & sinITEM_MASK3)==sin34 || (MouseItem.sItemInfo.CODE & sinITEM_MASK3)==sin35 ) // 장별 - 복날 이벤트 아뮬렛, 링 추가

				   TradeColorIndex = NOT_SETTING_COLOR;
			}

			// 박재원 - 이벤트 아뮬렛, 산타 아뮬렛 조합 금지 // 장별 - 복날 이벤트 아뮬렛, 링 추가
			if( (MouseItem.sItemInfo.CODE & sinITEM_MASK2)==sinOA1 )
			{
				if( (MouseItem.sItemInfo.CODE & sinITEM_MASK3)==sin32 || (MouseItem.sItemInfo.CODE & sinITEM_MASK3)==sin33 ||
					(MouseItem.sItemInfo.CODE & sinITEM_MASK3)==sin34 || (MouseItem.sItemInfo.CODE & sinITEM_MASK3)==sin35 || // // 박재원 - 이벤트아뮬렛(1시간), 이벤트아뮬렛(1일)도 조합금지
					(MouseItem.sItemInfo.CODE & sinITEM_MASK3)==sin38 )
				{
					TradeColorIndex = NOT_SETTING_COLOR;
				}
			}

			// 박재원 - 캡슐 아이템도 조합 금지(호랑이 캡슐 사용)
			if( (MouseItem.sItemInfo.CODE & sinITEM_MASK2)==sinSP1 )
			{
				if( (MouseItem.sItemInfo.CODE & sinITEM_MASK3)==sin34 ) 
				{
					TradeColorIndex = NOT_SETTING_COLOR;
				}
			}

			// 박재원 - 수박 아이템도 조합 금지
			if( (MouseItem.sItemInfo.CODE & sinITEM_MASK2)==sinSP1 )
			{
				if( (MouseItem.sItemInfo.CODE & sinITEM_MASK3)==sin15) 
					TradeColorIndex = NOT_SETTING_COLOR;
			}

			// 장별 - 발렌타인 초콜릿 아이템도 조합 금지
			if( (MouseItem.sItemInfo.CODE & sinITEM_MASK2)==sinSP1 )
			{
				if( (MouseItem.sItemInfo.CODE & sinITEM_MASK3)==sin35) 
					TradeColorIndex = NOT_SETTING_COLOR;
			}

			// 장별 - 하트링도 조합 금지
			if( (MouseItem.sItemInfo.CODE & sinITEM_MASK2)==sinOR2 )
			{
				if( (MouseItem.sItemInfo.CODE & sinITEM_MASK3)==sin33) 
					TradeColorIndex = NOT_SETTING_COLOR;
			}
			
			// 박재원 - 알파벳 조합 이벤트도 조합 금지
			if( (MouseItem.sItemInfo.CODE & sinITEM_MASK2)==sinSP1 )
			{
				if( (MouseItem.sItemInfo.CODE & sinITEM_MASK3)==sin27 || (MouseItem.sItemInfo.CODE & sinITEM_MASK3)==sin28 ||
					(MouseItem.sItemInfo.CODE & sinITEM_MASK3)==sin29 || (MouseItem.sItemInfo.CODE & sinITEM_MASK3)==sin30 ||
					(MouseItem.sItemInfo.CODE & sinITEM_MASK3)==sin31 || (MouseItem.sItemInfo.CODE & sinITEM_MASK3)==sin32 ||
					(MouseItem.sItemInfo.CODE & sinITEM_MASK3)==sin33 )
				{
					TradeColorIndex = NOT_SETTING_COLOR;
				}
			}

			// 박재원 - 매직 쉘텀 조합 금지
			if(!cCraftItem.ForceFlag)
			{
				if( (MouseItem.sItemInfo.CODE & sinITEM_MASK2)==sinOS1 )
				{
					if( (MouseItem.sItemInfo.CODE & sinITEM_MASK3)>=sin21 && (MouseItem.sItemInfo.CODE & sinITEM_MASK3)<=sin34 )
					{
						TradeColorIndex = NOT_SETTING_COLOR;
					}
				}
			}

			// 장별 - 눈결정 목걸이도 조합 금지
			if( (MouseItem.sItemInfo.CODE & sinITEM_MASK2)==sinOA1 )
			{
				if( (MouseItem.sItemInfo.CODE & sinITEM_MASK3)==sin36) 
					TradeColorIndex = NOT_SETTING_COLOR;
			}


			// 장별 - 캔디데이즈 캔디 아이템도 조합 금지
			if( (MouseItem.sItemInfo.CODE & sinITEM_MASK2)==sinSP1 )
			{
				if( (MouseItem.sItemInfo.CODE & sinITEM_MASK3)==sin36) 
					TradeColorIndex = NOT_SETTING_COLOR;
			}

			// 장별 - 캔디데이즈 하트아뮬렛도 조합 금지
			if( (MouseItem.sItemInfo.CODE & sinITEM_MASK2)==sinOA1 )
			{
				if( (MouseItem.sItemInfo.CODE & sinITEM_MASK3)==sin37) 
					TradeColorIndex = NOT_SETTING_COLOR;
			}

			// 장별 - 매지컬그린 에메랄드, 비취 아이템도 조합 금지
			if( (MouseItem.sItemInfo.CODE & sinITEM_MASK2)==sinSP1 )
			{
				if( (MouseItem.sItemInfo.CODE & sinITEM_MASK3)==sin37 || (MouseItem.sItemInfo.CODE & sinITEM_MASK3)==sin38 ) 
					TradeColorIndex = NOT_SETTING_COLOR;
			}

			// 장별 - 카라의 눈물 아이템도 조합 금지 장별 - 수박 아이템 조합 금지
			if( (MouseItem.sItemInfo.CODE & sinITEM_MASK2)==sinSP1 )
			{
				if( (MouseItem.sItemInfo.CODE & sinITEM_MASK3)==sin39 || (MouseItem.sItemInfo.CODE & sinITEM_MASK3)==sin42 ) 
					TradeColorIndex = NOT_SETTING_COLOR;
			}

			// 장별 - 조사원을 찾아라 조합 금지
			if( (MouseItem.sItemInfo.CODE & sinITEM_MASK2)==sinGF1 )
			{
				if( (MouseItem.sItemInfo.CODE & sinITEM_MASK3)==sin07 || (MouseItem.sItemInfo.CODE & sinITEM_MASK3)==sin08 ) 
					TradeColorIndex = NOT_SETTING_COLOR;
			}



			//아이템이 없으면 씰아이템 은 못올린다.
			if(!sCraftItem.CraftItem[0].Flag){	
			    if((MouseItem.sItemInfo.CODE & sinITEM_MASK2)==sinSE1){
					cMessageBox.ShowMessage(MESSAGE_NOT_RECONITEM);
					TradeColorIndex = NOT_SETTING_COLOR;
				}
			}		
			if(TradeColorIndex != NO_BOX_COLOR || TradeColorIndex != NOT_SETTING_COLOR){
				if(MouseItem.sItemInfo.ItemKindCode == ITEM_KIND_AGING ||  MouseItem.sItemInfo.ItemKindCode == ITEM_KIND_CRAFT || MouseItem.sItemInfo.UniqueItem){ //크래프트된 아이템과					유니크는 올려놓을수 없다 
					if(!cCraftItem.ForceFlag){
						cMessageBox.ShowMessage(MESSAGE_NOT_CRAFT_ITEM);
					}
				}
				else{
					//에이징을 할수있는 금액이 있는 가를 체크 
					if(TradeColorIndex == NOT_ENOUGH_MONEY){
						cMessageBox.ShowMessage(MESSAGE_NOT_ENOUGH_MONEY);
						return;
					
					}
					cInvenTory.CheckRequireItemToSet(&MouseItem); //셋팅될수있는 아이템인지 값을 넣어준다 
					// 박재원 - 리스펙 스톤
					if( MouseItem.sItemInfo.CODE == (sinBI1|sin90) )
					{
						if(159+11<=MouseItem.x+22 && 159+11>MouseItem.x && 268+11+sinInterHeight2 < MouseItem.y+22 && 268+11+sinInterHeight2 > MouseItem.y)
						{
							TradeColorIndex = SET_ITEM_CHECK_COLOR;
						}
					}
					if(TradeColorIndex == NO_BOX_COLOR || TradeColorIndex == NOT_SETTING_COLOR)return; //셋팅될수없으면 리턴한다 
						cCraftItem.LastSetCraftItem(&MouseItem);
				}
			}
		}
		else{
			if(	!StartInterEffectAging2 && !sinCraftItemStartFlag && !CheckMixItemCopyFlag){ //이펙트가 실행되지않을때 캔슬할수있다
				if(!sinCraftItemStartFlag2 && !MixItemNoCopyFlag){  //이펙트가 실행중이 아닐경우에만 아이템을 집을수있다 
					cCraftItem.PickUpCraftItem(pCursorPos.x , pCursorPos.y,1);//아이템을 집는다 			
				}
				if(CraftItemButtonIndex == 2){
					/*----------------------------------------------------------------------------*
					*Desc  : 추가수정 아이템 회수 관련 체크 부분을 추가한다..
						     복잡하지만..어쩔수없다...그냥간다....ㅠㅠ
					*Date  : 2006.1.17
					*Author: 하대용
					*-----------------------------------------------------------------------------*/					
					SheltomCount = 0;
					//무기군 
					if(sCraftItem.CraftItem[0].Flag){
						for( i = 0 ; i < sCraftItem.CraftItem[0].h/22 ; i++ ){
							SheltomCount+=sCraftItem.CraftItem[0].w/22;
						}
						if(!cCraftItem.CraftCheckEmptyArea(&sCraftItem.CraftItem[0]) )
							SheltomCount = 100;
	
					}
					for(i = 1 ; i < 15 ; i++){
						if(sCraftItem.CraftItem[i].Flag){
							if(cCraftItem.CraftCheckEmptyArea(&sCraftItem.CraftItem[i])){
								CloseButtonFlag = 0;								
							}
							else{
								CloseButtonFlag = 1;
								break;
							}
							SheltomCount++;
						}
					}	
					//=------------------------------------------------------------------------------
					if(!CloseButtonFlag && SheltomCount <= cCraftItem.GetCraftCheckEmptyArea()){
						cCraftItem.CancelMixItem();
					}
					else{
						cMessageBox.ShowMessage(MESSAGE_OVER_SPACE); //공간 초과  
					}
				}
				if(CraftItemButtonIndex == 1){ //아이템 조합 
					if(cCraftItem.ForceFlag){
						SheltomCount = 0;
						for( i = 1 ; i < 15 ; i++){
							if(sCraftItem.CraftItem[i].Flag)             //아이템이 있을겨우에보낸다
								SheltomCount++;
						}
						if( SheltomCount <= cCraftItem.GetCraftCheckEmptyArea() ){
							cCraftItem.sinSendCraftItem();               //서버로 믹스를 보낸다 
						}
						else
							cMessageBox.ShowMessage(MESSAGE_OVER_SPACE); //공간 초과  
					}
					//haGoon아이템 재구성----------------------------------------------------------------------
					else if(cCraftItem.haCheckReconItem()){
						if(sCraftItem.CraftItem[0].Flag && !sCraftItem.CraftItem[0].sItemInfo.ItemKindCode ){
							for( i=1 ; i < 15 ; i++){
								if(sCraftItem.CraftItem[i].Flag){
									CopyMixItemCheckDelete(&sCraftItem.CraftItem[i-1]);
								}
							}
							//서버로 보내자
							if(cCraftItem.CraftCheckEmptyArea(&sCraftItem.CraftItem[0])){
								if(!sinCraftItemStartFlag2){ //현재 이펙트가 실행중이 아닐경우에만
									sinCraftItemStartFlag = 1;
									SetInterEffectAging(); //이펙트를 실행 

								}
							}
							else{
								cMessageBox.ShowMessage(MESSAGE_OVER_SPACE); //공간 초과
							}

						}
					}
					//-------------------------------------------------------------------------------------------
					else{
					if(sCraftItem.CraftItem[0].Flag && !sCraftItem.CraftItem[0].sItemInfo.ItemKindCode ){
						/////////믹스에서 복사된 아이템을 제거한다
						for( i=1 ; i < 15 ; i++){
							if(sCraftItem.CraftItem[i].Flag){
								CopyMixItemCheckDelete(&sCraftItem.CraftItem[i-1]);
							}
						}
						for(i = 1 ; i < 15 ; i++){
							if(sCraftItem.CraftItem[i].Flag)
							{
								if(cCraftItem.CraftCheckEmptyArea(&sCraftItem.CraftItem[0]) && !sCraftItem.CraftItem[14].Flag) // 박재원 - 리스펙 스톤 슬롯에 스톤이없을 경우 믹스쳐가 가능하다 
								{
									//cCraftItem.MixingItem();
									if(!sinCraftItemStartFlag2){ //현재 이펙트가 실행중이 아닐경우에만
										sinCraftItemStartFlag = 1;
										SetInterEffectAging(); //이펙트를 실행 
										TempFlag = 1;	
										MixCancelButtonDelayFlag = 1;
										CheckMixItemCopyFlag = 1; //믹스 아이템 복사를 방지하기위한 플렉

									}

								}
								else
								{
									if(sCraftItem.CraftItem[14].Flag) // 박재원 - 리스펙 스톤이 슬롯에 장착되어 있지만 씰, 쉘텀이 장착되지 않았을경우 
									{
										cMessageBox.ShowMessage(MESSAGE_CANNOT_RECONITEM);
									}
									else
										cMessageBox.ShowMessage(MESSAGE_OVER_SPACE); //공간 초과  

									TempFlag = 1;
								}

							 }
						 }
						 if(!TempFlag)
							cMessageBox.ShowMessage(MESSAGE_NOT_CRAFT_ITEM);

					 }
					 else{
						cMessageBox.ShowMessage(MESSAGE_NOT_CRAFT_ITEM);
					  }
					} //ForceOrb땀시 
				}
			}
		}
	}
	
	// 석지용 - 믹스쳐 리셋창이 열려있다면...
	if( cMixtureReset.OpenFlag )
	{
		if( MouseItem.Flag )	// 마우스에 아이템이 있다면...
		{
			// 아이템이 없으면 스톤을 못올린다.
			if( !sMixtureResetItem.MixtureResetItem[0].Flag )
			{	
				if( (MouseItem.sItemInfo.CODE & sinITEM_MASK3) == sin89 )
				{
					cMessageBox.ShowMessage(MESSAGE_NOT_MIXTURE_RESET_STONE);
					TradeColorIndex = NOT_SETTING_COLOR;
				}
			}		

			// 세팅이 가능한 아이템이면...
			if( TradeColorIndex != NO_BOX_COLOR && TradeColorIndex != NOT_SETTING_COLOR )
			{
				if( (MouseItem.CODE & sinITEM_MASK2) == sinBI1 && (MouseItem.sItemInfo.CODE & sinITEM_MASK3) == sin89 )
				{
					cMixtureReset.LastSetMixtureResetItem( &MouseItem, 1 );		// 1은 믹스쳐 리셋 스톤
					MixtureResetCheckFlag = 1;		// 믹스쳐 리셋이 가능함!!
				}
				else if( (MouseItem.sItemInfo.ItemKindCode == ITEM_KIND_CRAFT ) )
					cMixtureReset.LastSetMixtureResetItem( &MouseItem, 2 );		// 2는 믹스쳐된 아이템				
			}
		}
		else	// 마우스에 아이템이 없는 상태라면...
		{
			if(	!StartInterEffectAging2 && !sinCraftItemStartFlag )	//이펙트가 실행되지않을때 캔슬할수있다
			{
				if( !sinCraftItemStartFlag2	)	 //현재 이펙트가 실행중이 아닐경우에만 아이템을 집는다 
					cMixtureReset.PickUpMixtureResetItem( pCursorPos.x, pCursorPos.y, 1);

				if(CraftItemButtonIndex == 2 )	// 마우스가 닫기 버튼 영역에 있을때
				{
					// 스톤이 올려져 있는지 검사
					CloseButtonFlag = 0;
					if( sMixtureResetItem.MixtureResetItem[1].Flag )
						CloseButtonFlag = 1;

					if( CloseButtonFlag == 1 )	// 스톤이 올려져 있다면...
						cMessageBox.ShowMessage(MESSAGE_NOT_CANCEL_RESET);	// 창 취소 불가!!
					else
					{
						if( sMixtureResetItem.MixtureResetItem[0].Flag )	// 아이템이 있다면...
						{
							if( !cCraftItem.CraftCheckEmptyArea( &sMixtureResetItem.MixtureResetItem[0] ) )
							{
								// pluto 공간부족
								int LineCount = 0, Temp;
								ShowItemOverSpace = 1;
								lstrcpy( szCraftItemBuff, "\260\370\260\243\300\273 \310\256\272\270\307\330 \301\326\274\274\277\344" );

								strcat(szCraftItemBuff, "\r");
								Temp = lstrlen(szCraftItemBuff);
								for( i = 0 ; i < Temp ; i++)
								{
									if(szCraftItemBuff[i] == '\r')
									{
										LineCount++;
									}
								}
								CraftItemMessageSize.x = 14;           //박스 사이즈 
								CraftItemMessageSize.y = ((LineCount)*2)+2;  //박스 사이즈
								sinPlaySound(SIN_SOUND_FAILD_MIXITEM);
							}
							else
								cMixtureReset.CancelMixtureItemReset();	// 믹스쳐리셋 창 닫기
						}
						else
							cMixtureReset.CancelMixtureItemReset();		// 믹스쳐리셋 창 닫기
					}
				}

				// 믹스쳐 리셋 창이 열린 상태에서 마우스가 조합위치에 있고 스톤이 올려져 있는 상태라면
				if( CraftItemButtonIndex == 1 && sMixtureResetItem.MixtureResetItem[1].Flag )
				{
					if( cCraftItem.CraftCheckEmptyArea( &sMixtureResetItem.MixtureResetItem[0] ) )
					{
						if(!sinCraftItemStartFlag2)	//현재 이펙트가 실행중이 아닐경우에만	
						{							
							sinCraftItemStartFlag = 1;		
							SetInterEffectMixtureReset();	//이펙트를 실행 
							MixCancelButtonDelayFlag = 1;	
							MixtureResetCheckFlag = 0;
						}
					}
					else
						cMessageBox.ShowMessage(MESSAGE_OVER_SPACE); //공간 초과  		
				}
			}
		}
	}

	if(cAging.OpenFlag){		//아이템 에이징 	
		if(MouseItem.Flag){
			if(TradeColorIndex == NOT_AGING_ITEM){
				cMessageBox.ShowMessage(MESSAGE_NOT_AGING_ITEM);
			}
			else{
				//에이징을 할수있는 금액이 있는 가를 체크 
				if(TradeColorIndex == NOT_ENOUGH_MONEY){
					cMessageBox.ShowMessage(MESSAGE_NOT_ENOUGH_MONEY);
					
				}
				if(TradeColorIndex != NO_BOX_COLOR && TradeColorIndex != NOT_SETTING_COLOR && TradeColorIndex != NOT_ENOUGH_MONEY){
					if((MouseItem.sItemInfo.CODE & sinITEM_MASK2) == sinMA1 ||
						(MouseItem.sItemInfo.CODE & sinITEM_MASK2) == sinMA2 ){
							cCraftItem.LastSetCraftItem(&MouseItem,2); //MakeItem 2
							sinMakeItemCheck(); //아이템을체크한다

					}
					else if(MouseItem.sItemInfo.CODE == (sinBI1|sin10) || MouseItem.sItemInfo.CODE == (sinBI1|sin11) ||
						    MouseItem.sItemInfo.CODE == (sinBI1|sin60) || MouseItem.sItemInfo.CODE == (sinBI1|sin61) || // 박재원 - 엘더 코퍼 오어, 슈퍼 에이징 스톤 추가
							MouseItem.sItemInfo.CODE == (sinBI1|sin85) ) // 장별 - 슈퍼 에이징 스톤 1.5
					{
						if(	!StartInterEffectAging2 && !sinCraftItemStartFlag)
							cCraftItem.LastSetCraftItem(&MouseItem,3); //MakeItem 2
					}
					else{
						cCraftItem.LastSetCraftItem(&MouseItem,1); //에이징은 1
						cAging.CheckItem(&MouseItem);//아이템을 체크한다 
					}
				}
			}
		}
		else{
			if(	!StartInterEffectAging2 && !sinCraftItemStartFlag){ //이펙트가 실행되지않을때 캔슬할수있다
				if(!sinCraftItemStartFlag2){ //현재 이펙트가 실행중이 아닐경우에만	
					//임시 작업
					cCraftItem.PickUpCraftItem(pCursorPos.x , pCursorPos.y,1,1);//아이템을 집는다 
					
				}
				if(CraftItemButtonIndex == 2 ){
					//2005년 12월 30일 (2005년 마지막 근무..샤방샤방 롯떼월드감..)
					//무기군만 체크하고 쉘텀이 올라가잇으면 닫혀지지않는다.
					CloseButtonFlag = 0;
					for( int i = 1 ; i < 15 ; i++){
						if(sAgingItem.AgingItem[i].Flag){
							CloseButtonFlag = 1;
							break;
						}							
					}
					if( CloseButtonFlag == 1 ){
						cMessageBox.ShowMessage(MESSAGE_NOT_AGINGSHELTOM); //  

					}
					else{
						if(sAgingItem.AgingItem[0].Flag){
							if(!cCraftItem.CraftCheckEmptyArea(&sAgingItem.AgingItem[0])){
//								cMessageBox.ShowMessage(MESSAGE_OVER_SPACE); //공간 초과  			
								// pluto 공간부족
								int LineCount = 0, Temp;
								ShowItemOverSpace = 1;
								lstrcpy( szCraftItemBuff, InvenSpaceSecure );

								strcat(szCraftItemBuff, "\r");
								Temp = lstrlen(szCraftItemBuff);
								for( int j = 0 ; j < Temp ; j++)
								{
									if(szCraftItemBuff[j] == '\r')
									{
										LineCount++;
									}
								}
								CraftItemMessageSize.x = 14;           //박스 사이즈 
								CraftItemMessageSize.y = ((LineCount)*2)+2;  //박스 사이즈
								sinPlaySound(SIN_SOUND_FAILD_MIXITEM);
							}
							else{
								cAging.CancelAgingItem();	//에이징 아이템을 캔슬한다 						
							}
						}
						else
							cAging.CancelAgingItem();	//에이징 아이템을 캔슬한다 						

					}
				}

/*				if(CraftItemButtonIndex == 1){ //아이템 조합 
					cAging.sinSendAgingItem(); //에이징 아이템을 서버로 보낸다									
				}
*/

				if(CraftItemButtonIndex == 1&& AgingCheckSheltomFlag){ //아이템 조합 
					if(cCraftItem.CraftCheckEmptyArea(&sAgingItem.AgingItem[0])){
						if(!sinCraftItemStartFlag2){ //현재 이펙트가 실행중이 아닐경우에만					
							//cAging.AgingItem();	//아이템을 에이징한다 
							sinCraftItemStartFlag = 1;
							SetInterEffectAging(); //이펙트를 실행 
							MixCancelButtonDelayFlag = 1;
						}
					}
					else
						cMessageBox.ShowMessage(MESSAGE_OVER_SPACE); //공간 초과  		
				}
				
			}
		}

	}

	// pluto 제련
	if( SmeltingItem.OpenFlag )
	{
		if( MouseItem.Flag )
		{
			if(TradeColorIndex == NO_BOX_COLOR || TradeColorIndex == NOT_SETTING_COLOR)
			{
				return; //셋팅될수없으면 리턴한다 
			}
			if( (MouseItem.sItemInfo.CODE & sinITEM_MASK2) == sinPR1 || (MouseItem.sItemInfo.CODE & sinITEM_MASK2) == sinPR2 ) // 같은 광석만 올릴수 있다
			{
				if( S_smeltingItem.SmeltingItem[0].Flag && S_smeltingItem.SmeltingItem[0].CODE != MouseItem.CODE )
				{
					cMessageBox.ShowMessage(MESSAGE_NOT_SAME_SMELTINGITEM);
					return;
				}

				// 장별 - 제련 시스템 수정
				if( S_smeltingItem.SmeltingItem[0].Flag && S_smeltingItem.SmeltingItem[1].Flag && S_smeltingItem.SmeltingItem[2].Flag ) return;

			}
			if(TradeColorIndex == NOT_ENOUGH_MONEY)
			{
				cMessageBox.ShowMessage(MESSAGE_NOT_ENOUGH_MONEY);
				return;

			}
			SmeltingItem.LastSetSmeltingItem( &MouseItem);
		}
		else
		{
			if( !StartInterEffectAging2 && !sinCraftItemStartFlag && !CheckMixItemCopyFlag )
			{
				if( !sinCraftItemStartFlag2 && !MixCancelButtonDelayFlag )
				{
					SmeltingItem.PickUpSmeltingItem( pCursorPos.x, pCursorPos.y, 1 );
				}

				if( CraftItemButtonIndex == 2 )
				{
					for( i = 0; i < 5; i++ )
					{
						if( S_smeltingItem.SmeltingItem[i].Flag )
						{
							if( SmeltingItem.SmeltingCheckEmptyArea( &S_smeltingItem.SmeltingItem[i]) )
							{
								CloseButtonFlag = 0;
							}
							else
							{
								CloseButtonFlag = 1;
								break;
							}
							SheltomCount++;
						}
						
					}
					if( !CloseButtonFlag && SheltomCount <= SmeltingItem.GetSmeltingCheckEmptyArea() )
					{
						SmeltingItem.CancelSmeltingItem();
					}
					else
					{
						// pluto 공간부족
						int LineCount = 0, Temp;
						ShowItemOverSpace = 1;
						lstrcpy( szCraftItemBuff, InvenSpaceSecure );

						strcat(szCraftItemBuff, "\r");
						Temp = lstrlen(szCraftItemBuff);
						for( i = 0 ; i < Temp ; i++)
						{
							if(szCraftItemBuff[i] == '\r')
							{
								LineCount++;
							}
						}
						CraftItemMessageSize.x = 14;           //박스 사이즈 
						CraftItemMessageSize.y = ((LineCount)*2)+2;  //박스 사이즈
						sinPlaySound(SIN_SOUND_FAILD_MIXITEM);
					}
				}

				// 장별 - 제련 시스템 수정
				else if( CraftItemButtonIndex == 1)
				{
					if( S_smeltingItem.SmeltingItem[0].Flag && !S_smeltingItem.SmeltingItem[0].sItemInfo.ItemKindCode &&
						S_smeltingItem.SmeltingItem[1].Flag && !S_smeltingItem.SmeltingItem[1].sItemInfo.ItemKindCode &&
						S_smeltingItem.SmeltingItem[2].Flag && !S_smeltingItem.SmeltingItem[2].sItemInfo.ItemKindCode )
					{
						// pluto 공간 부족 메세지 필요
						sITEM temp;
						for( int i = 0; i < MAX_ITEM; i++ )
						{
							if( sItem[i].CODE == (sinPR1|sin01) )
							{
								temp = sItem[i];
								if( cInvenTory.CheckInvenEmpty(&temp) )
								{
									if( S_smeltingItem.SmeltingItem[4].Flag )
									{
										if( !sinCraftItemStartFlag2 )
										{
											sinCraftItemStartFlag = 1;
											SetInterEffectSmelting();
											MixCancelButtonDelayFlag = 1;
										}
									}
								}
								else
								{
									cMessageBox.ShowMessage(MESSAGE_OVER_SPACE); //공간부족
								}
								break;
							}
						}
					}
				}
			}
		}
	}

	// pluto 제작
	if( ManufactureItem.m_OpenFlag )
	{
		if( MouseItem.Flag )
		{
			if( TradeColorIndex == NOT_SETTING_COLOR )
			{
				return; //셋팅될수없으면 리턴한다 
			}
			else if( TradeColorIndex == NO_BOX_COLOR )
			{
				cMessageBox.ShowMessage( MESSAGE_NOT_NEED_RUNE );
				return;
			}
			else if( TradeColorIndex == NOT_ENOUGH_MONEY )
			{
				cMessageBox.ShowMessage( MESSAGE_NOT_ENOUGH_MONEY );
				return;
			}
			ManufactureItem.LastSetManufactureItem( &MouseItem );
		}
		else
		{
			if( !StartInterEffectAging2 && !sinCraftItemStartFlag && !CheckMixItemCopyFlag )
			{
				if( !sinCraftItemStartFlag2 && !MixCancelButtonDelayFlag )
				{
					ManufactureItem.PickUpManufactureItem( pCursorPos.x, pCursorPos.y, 1 );
				}

				if( CraftItemButtonIndex == 2 )
				{
					CloseButtonFlag = 0;
					for( i = 0; i < 4; i++ )
					{
						if( g_sManufactureItem.ManufactureItem[i].Flag )
						{
							if( ManufactureItem.ManufactureCheckEmptyArea( &g_sManufactureItem.ManufactureItem[i] ) )
							{
								CloseButtonFlag = 0;
							}
							else
							{
								CloseButtonFlag = 1;
								break;
							}
							SheltomCount++;
						}
					}
					if( !CloseButtonFlag && SheltomCount <= ManufactureItem.GetManufactureCheckEmptyArea() )
					{
						ManufactureItem.CancelManufactureItem();
					}
					else
					{
						// pluto 공간부족
						int LineCount = 0, Temp;
						ShowItemOverSpace = 1;
						lstrcpy( szCraftItemBuff, InvenSpaceSecure );

						strcat(szCraftItemBuff, "\r");
						Temp = lstrlen(szCraftItemBuff);
						for( i = 0 ; i < Temp ; i++)
						{
							if(szCraftItemBuff[i] == '\r')
							{
								LineCount++;
							}
						}
						CraftItemMessageSize.x = 14;           //박스 사이즈 
						CraftItemMessageSize.y = ((LineCount)*2)+2;  //박스 사이즈
						sinPlaySound(SIN_SOUND_FAILD_MIXITEM);
					}
				}
				else if( CraftItemButtonIndex == 1 )
				{
					if( g_sManufactureItem.ManufactureItem[0].Flag && !g_sManufactureItem.ManufactureItem[0].sItemInfo.ItemKindCode )
					{
						if( g_sManufactureItem.ManufactureItem[1].Flag && g_sManufactureItem.ManufactureItem[2].Flag && g_sManufactureItem.ManufactureItem[3].Flag )
						{
							SManufacture_ResultItemInfo ResultItem;
							memset(&ResultItem,0,sizeof(SManufacture_ResultItemInfo));
							sITEM temp;
							ResultItem = ManufactureItem.ManufactureCheckItem(g_sManufactureItem.ManufactureItem, sinChar); // 장별 - 대장장이의 혼
							for(int cnt=0; cnt < MAX_ITEM; cnt++)
							{
								if( sItem[cnt].CODE == ResultItem.ItemCode )
								{
									temp = sItem[cnt];
									if(	cInvenTory.CheckInvenEmpty(&temp))
									{
										if( sinChar->Weight[0] + ResultItem.Weight <= sinChar->Weight[1] )
										{
											if( ForceItemPrice2 <= sinChar->Money )
											{
												if( !sinCraftItemStartFlag2 )
												{
													cMessageBox.ShowMessage3(MESSAGE_MANUFACTURE, ResultItem.ItemName);
													//sinCraftItemStartFlag = 1;
													//SetInterEffectAging();
												}
											}
											else
											{
												cMessageBox.ShowMessage( MESSAGE_NOT_ENOUGH_MONEY );
											}
										}
										else
										{
											cMessageBox.ShowMessage(MESSAGE_OVER_WEIGHT); //무게초과
											//ManufactureItem.CancelManufactureItem();
										}
									}
									else
									{
										cMessageBox.ShowMessage(MESSAGE_OVER_SPACE); //공간부족
										//ManufactureItem.CancelManufactureItem();
									}
									break;
								}
							}
						}
					}
				}
			}
		}
	}

}
/*----------------------------------------------------------------------------*
*							   LButtonUp
*-----------------------------------------------------------------------------*/	
void cTRADE::LButtonUp(int x , int y)
{

}
/*----------------------------------------------------------------------------*
*							   RButtonDown
*-----------------------------------------------------------------------------*/	
void cTRADE::RButtonDown(int x , int y)
{

}
/*----------------------------------------------------------------------------*
*							   RButtonUp
*-----------------------------------------------------------------------------*/	
void cTRADE::RButtonUp(int x , int y)
{

}
/*----------------------------------------------------------------------------*
*							    KeyDown
*-----------------------------------------------------------------------------*/	
void cTRADE::KeyDown()
{
/*
	if(sinGetKeyClick('M')){
		lstrcpy(szCraftItemBuff,"모든속성 +5\r기력 +10\r명중력 +50\r 최대공격력+5\r"); //아이템 정보를 보여준다 
		CraftItemMessageSize.x = 14;           //박스 사이즈 
		CraftItemMessageSize.y = ((3)*2)+2;  //박스 사이즈 


	}
*/
}


void cTRADE::DrawTradeText()
{

	if(ShowItemAgingMessageFlag || ShowItemCraftMessageFlag || TradeRequestFlag || ShowItemSmeltingMessageFlag || ShowItemManufactureMessageFlag || ShowItemOverSpace || sinMoveKindInter[SIN_TRADE] || 
		sinMoveKindInter[SIN_CRAFTITEM] || sinMoveKindInter[SIN_WAREHOUSE] ||	sinMoveKindInter[SIN_AGING] || sinMoveKindInter[SIN_SMELTING] || sinMoveKindInter[SIN_MANUFACTURE]){


	int i;
	HDC	hdc;

	char strBuff[128];
	
	memset(strBuff,0,sizeof(strBuff));
	lpDDSBack->GetDC( &hdc );
	SelectObject( hdc, sinFont);
    SetBkMode( hdc, TRANSPARENT );
	SetTextColor( hdc, RGB(255,255,255) );

	if(TradeRequestFlag){ //거래 요청 
		if(cTrade.szTradeCharName[0]){
			lstrcpy(strBuff,cTrade.szTradeCharName);
			dsTextLineOut(hdc,TRADEBUTTONMAIN_X+38,TRADEBUTTONMAIN_Y+17,strBuff,lstrlen(strBuff));

		}

	}

	if(cWareHouse.OpenFlag){ //창고 
		memset(strBuff,0,sizeof(strBuff));
		NumLineComa(sWareHouse.Money-2023,strBuff);
		//wsprintf(strBuff,"%d",sWareHouse.Money-2023); //창고 돈 
		dsTextLineOut(hdc,CheckEditSize(ShopGoldEdit[0][0],ShopGoldEdit[0][2], strBuff)-(256+128-sinMoveKindInter[SIN_WAREHOUSE]),ShopGoldEdit[0][1],
				strBuff,lstrlen(strBuff));
		
		memset(strBuff,0,sizeof(strBuff));
		wsprintf(strBuff,"%d/%d",sWareHouse.Weight[0]-197,sWareHouse.Weight[1]-196); //무게
		dsTextLineOut(hdc,CheckEditSize(ShopGoldEdit[1][0],ShopGoldEdit[1][2], strBuff)-(256+128-sinMoveKindInter[SIN_WAREHOUSE]),ShopGoldEdit[1][1],
					strBuff,lstrlen(strBuff));
		
		for(i=0;i<100;i++){
			if(sWareHouse.WareHouseItem[i].Flag){
				if(sWareHouse.WareHouseItem[i].Class == ITEM_CLASS_POTION){
					wsprintf(strBuff,"%d",sWareHouse.WareHouseItem[i].sItemInfo.PotionCount);
					dsTextLineOut(hdc,sWareHouse.WareHouseItem[i].x -(256+128-sinMoveKindInter[SIN_WAREHOUSE]), sWareHouse.WareHouseItem[i].y,strBuff,lstrlen(strBuff));
				}
			}
		}
	}

	if(OpenFlag){
		///////////// 거래 위 
		lstrcpy(strBuff,cTrade.szTradeCharName); 
		dsTextLineOut(hdc,CheckEditSize(229,332, strBuff)-(256+128-sinMoveKindInter[SIN_TRADE]),
			94,strBuff,lstrlen(strBuff));

		if(sTradeRecv.Weight > 0){
			wsprintf(strBuff,"%d",sTradeRecv.Weight);
			dsTextLineOut(hdc,CheckEditSize(251,326,strBuff)-(256+128-sinMoveKindInter[SIN_TRADE]),
				152,strBuff,lstrlen(strBuff));
		}

		if(sTradeRecv.Money >0){
			memset(strBuff,0,sizeof(strBuff));
			NumLineComa(sTradeRecv.Money-193,strBuff);
			//wsprintf(strBuff,"%d",sTradeRecv.Money-193);
			dsTextLineOut(hdc,CheckEditSize(251,326, strBuff)-(256+128-sinMoveKindInter[SIN_TRADE]),
				127,strBuff,lstrlen(strBuff));
		}


		/*
		lstrcpy(strBuff,cTrade.szTradeCharName);
		dsTextLineOut(hdc,CheckEditSize(UpTradePosi[T_MONEY_TEXT_POSI][0],UpTradePosi[T_MONEY_TEXT_POSI][2], strBuff)-2-(256+128-sinMoveKindInter[SIN_TRADE]),
			UpTradePosi[T_MONEY_TEXT_POSI][1]+6,strBuff,lstrlen(strBuff));

		
		wsprintf(strBuff,"%d",sTradeRecv.Weight);
		dsTextLineOut(hdc,CheckEditSize(UpTradePosi[T_WEIGHT_TEXT_POSI][0],UpTradePosi[T_WEIGHT_TEXT_POSI][2], strBuff)-2-(256+128-sinMoveKindInter[SIN_TRADE]),
			UpTradePosi[T_WEIGHT_TEXT_POSI][1]+5+2+28,strBuff,lstrlen(strBuff));
		


		if(sTradeRecv.Money >0){
			wsprintf(strBuff,"%d",sTradeRecv.Money-193);
			dsTextLineOut(hdc,CheckEditSize(UpTradePosi[T_MONEY_TEXT_POSI][0],UpTradePosi[T_MONEY_TEXT_POSI][2], strBuff)-2-(256+128-sinMoveKindInter[SIN_TRADE]),
				UpTradePosi[T_MONEY_TEXT_POSI][1]+5+2+30,strBuff,lstrlen(strBuff));
		}
		*/		
		///////////// 거래 아래 
		memset(strBuff,0,sizeof(strBuff));
		wsprintf(strBuff,"%d",sTrade.Weight);
		dsTextLineOut(hdc,CheckEditSize(DownTradePosi[T_WEIGHT_TEXT_POSI][0],DownTradePosi[T_WEIGHT_TEXT_POSI][2], strBuff)-2-(256+128-sinMoveKindInter[SIN_TRADE]),
			DownTradePosi[T_WEIGHT_TEXT_POSI][1]+5+2,strBuff,lstrlen(strBuff));

		memset(strBuff,0,sizeof(strBuff));
		NumLineComa(TempShowMoney,strBuff);
		//wsprintf(strBuff,"%d",TempShowMoney);
		dsTextLineOut(hdc,CheckEditSize(DownTradePosi[T_MONEY_TEXT_POSI][0],DownTradePosi[T_MONEY_TEXT_POSI][2], strBuff)-2-(256+128-sinMoveKindInter[SIN_TRADE]),
			DownTradePosi[T_MONEY_TEXT_POSI][1]+5+2+2,strBuff,lstrlen(strBuff));


		//물약 Text
		for(int i=0 ; i < MAX_TRADE_ITEM ; i++){
			if(sTrade.TradeItem[i].Flag){ //내꺼 
				if(sTrade.TradeItem[i].Class == ITEM_CLASS_POTION){
					wsprintf(strBuff,"%d",sTrade.TradeItem[i].sItemInfo.PotionCount);
					dsTextLineOut(hdc,sTrade.TradeItem[i].x-(256+128-sinMoveKindInter[SIN_TRADE]) , sTrade.TradeItem[i].y,strBuff,lstrlen(strBuff));

				}
			}
			if(sTradeRecv.TradeItem[i].Flag){ //거래자꺼 
				if(sTradeRecv.TradeItem[i].Class == ITEM_CLASS_POTION){
					wsprintf(strBuff,"%d",sTradeRecv.TradeItem[i].sItemInfo.PotionCount);
					dsTextLineOut(hdc,sTradeRecv.TradeItem[i].x-(256+128-sinMoveKindInter[SIN_TRADE]) , sTradeRecv.TradeItem[i].y-T_REVISION_Y,strBuff,lstrlen(strBuff));

				}
			}
		}
	}

	//에이징 아이템의 가격을 나타낸다 
	char szMoneyTemp[256];

	if(sAgingItem.AgingItem[0].Flag){ 
		//--------------------------------------------------------------------------//
		// 박재원 - 에이징 가격 무료 이벤트
		#ifdef AGING_MONEY_FREE
			#ifdef HASIEGE_MODE
			//공성전 세율적용 에이징 가격을 표시한다.
			wsprintf(szMoneyTemp,"%d",0); // 에이징 가격 무료
			#else
			wsprintf(szMoneyTemp,"%d",0); // 에이징 가격 무료
			#endif
		#else
			#ifdef HASIEGE_MODE
			//공성전 세율적용 에이징 가격을 표시한다.
			wsprintf(szMoneyTemp,"%d",cShop.haShopItemPrice((sAgingItem.AgingItem[0].sItemInfo.Price+(sAgingItem.AgingItem[0].sItemInfo.Price*sAgingItem.AgingItem[0].sItemInfo.ItemAgingNum[0]))/2));
			#else
			wsprintf(szMoneyTemp,"%d",(sAgingItem.AgingItem[0].sItemInfo.Price+(sAgingItem.AgingItem[0].sItemInfo.Price*sAgingItem.AgingItem[0].sItemInfo.ItemAgingNum[0]))/2);
			#endif
		#endif	
		//---------------------------------------------------------------------------//
		dsTextLineOut(hdc,CheckEditSize(200, 275 , szMoneyTemp),323+sinInterHeight2,szMoneyTemp,lstrlen(szMoneyTemp));

	}
	
	//믹스쳐에 아이템의 가격을 나타낸다 
	if(sCraftItem.CraftItem[0].Flag){   
		//--------------------------------------------------------------------------//
		#ifdef HASIEGE_MODE
			//공성전 세율적용 믹스쳐 가격을 표시한다.
			wsprintf(szMoneyTemp,"%d",cShop.haShopItemPrice(sCraftItem.CraftItem[0].sItemInfo.Price));
		#else
			wsprintf(szMoneyTemp,"%d",sCraftItem.CraftItem[0].sItemInfo.Price);
		#endif
		//---------------------------------------------------------------------------//
		dsTextLineOut(hdc,CheckEditSize(200, 325 , szMoneyTemp),322+sinInterHeight2,szMoneyTemp,lstrlen(szMoneyTemp));

	}

	//Force Orb 아이템의 가격을 나타낸다 
	if(cCraftItem.ForceFlag){ 
		//--------------------------------------------------------------------------//
		#ifdef HASIEGE_MODE
			//공성전 세율에 적용 포스오브 가격을 세팅한다.
		wsprintf(szMoneyTemp,"%d",cShop.haShopItemPrice(ForceItemPrice2));
		#else
			wsprintf(szMoneyTemp,"%d",ForceItemPrice2);
		#endif
		//---------------------------------------------------------------------------//
		dsTextLineOut(hdc,CheckEditSize(200, 325 , szMoneyTemp)-(256+128-sinMoveKindInter[SIN_CRAFTITEM]),322+sinInterHeight2,szMoneyTemp,lstrlen(szMoneyTemp));

	}

	// pluto 제련 아이템의 가격
	if( S_smeltingItem.SmeltingItem[0].Flag )
	{
		//#ifdef HASIEGE_MODE
		//	wsprintf( szMoneyTemp, "%d", cShop.haShopItemPrice( ForceItemPrice2 ) );
		//#else
			wsprintf( szMoneyTemp, "%d", ForceItemPrice2 );
		//#endif
			dsTextLineOut( hdc, CheckEditSize( 200, 325, szMoneyTemp), 322+sinInterHeight2, szMoneyTemp, lstrlen(szMoneyTemp) );
	}
	// pluto 제작 아이템의 가격
	if( g_sManufactureItem.ManufactureItem[0].Flag )
	{
		//#ifdef HASIEGE_MODE
		//		wsprintf( szMoneyTemp, "%d", cShop.haShopItemPrice( ForceItemPrice2 ) );
		//#else
				wsprintf( szMoneyTemp, "%d", ForceItemPrice2 );
		//#endif
				dsTextLineOut( hdc, CheckEditSize( 200, 325, szMoneyTemp), 322+sinInterHeight2, szMoneyTemp, lstrlen(szMoneyTemp) );
	}
	//(200-(256+128-sinMoveKindInter[SIN_AGING]),314,


	char *pCraftItemInfo[10];

	int len, Templen=0 ,Textlen=0 ;
	int Count = 0;
	int SetTextXposi = 0;
	char TempszCraftItemBuff[128];


	lstrcpy(TempszCraftItemBuff ,szCraftItemBuff);

	if(ShowItemCraftMessageFlag || ShowItemAgingMessageFlag || ShowItemSmeltingMessageFlag || ShowItemManufactureMessageFlag || ShowItemOverSpace){    //아이템이 조합됬을때 메세지를 보여준다 
		SetTextColor( hdc, RGB(222,231,255) );
		SelectObject( hdc, sinBoldFont);
		SetBkMode( hdc, TRANSPARENT );
		if(ShowItemCraftMessageFlag)
			dsTextLineOut(hdc,CraftItemMessageBoxPosi.x+53,CraftItemMessageBoxPosi.y+13, MixResultTitle , lstrlen(MixResultTitle));

		// pluto 아이템이 제련 됬을때
		if( ShowItemSmeltingMessageFlag )
		{
			dsTextLineOut(hdc,CraftItemMessageBoxPosi.x+53,CraftItemMessageBoxPosi.y+13, SmeltingResultTitle , lstrlen(SmeltingResultTitle));
		}
		// pluto 아이템이 제작 됬을때
		if( ShowItemManufactureMessageFlag )
		{
			dsTextLineOut(hdc,CraftItemMessageBoxPosi.x+53,CraftItemMessageBoxPosi.y+13, ManufactureResultTitle , lstrlen(ManufactureResultTitle));
		}
		// pluto 공간부족
		if( ShowItemOverSpace )
		{
			dsTextLineOut(hdc,CraftItemMessageBoxPosi.x+53,CraftItemMessageBoxPosi.y+13, InventoryFull , lstrlen("InventoryFull"));
		}
		SelectObject( hdc, sinFont);
		SetBkMode( hdc, TRANSPARENT );
		SetTextColor( hdc, RGB(255,255,255) );

		len = lstrlen(TempszCraftItemBuff);
		for( i = 0 ; i < len ; i++){
			if(TempszCraftItemBuff[i] == '\r'){
				pCraftItemInfo[Count] = &TempszCraftItemBuff[Templen];
				TempszCraftItemBuff[i] = 0;
				Templen = i+1;
				Textlen = lstrlen(pCraftItemInfo[Count]);
				//SetTextXposi = CraftItemMessageBoxPosi.x + ((CraftItemMessageSize.x*16) - ((Textlen)*11));
				SetTextXposi = ((CraftItemMessageBoxPosi.x+((CraftItemMessageSize.x*16)/2))  - ((Textlen/2)*11));
				if(Textlen <= 12)
					SetTextXposi -=6;

				dsTextLineOut(hdc,SetTextXposi+27,CraftItemMessageBoxPosi.y+38+((Count)*18), pCraftItemInfo[Count] , Textlen );
				Count++;
			}

		}
		/*
		if(CraftItemMessageSize.y == 4){
			SetTextXposi = (CraftItemMessageBoxPosi.x + 135) - ((len/2)*11);
			dsTextLineOut(hdc,SetTextXposi-3,CraftItemMessageBoxPosi.y+35, szCraftItemBuff , len );
		}
		*/

	}
	

	lpDDSBack->ReleaseDC( hdc );
	}

}

/////////////// 트레이드 관련 함수 

//셋팅할 영역을 찾는다 
int cTRADE::SetTradeItemAreaCheck(sITEM *pItem)
{

	int i,j;
	TradeColorIndex= 0; //초기화   (트레이드 창과 창고가 같이 쓴다)
	TradeCrashItemIndex[0] = 0; //충돌된 아이템 Index 초기화 
	InitTradeColorRect(); //초기화

	TradeStartX=DownTradePosi[T_SET_BOX][0];
	TradeStartY =DownTradePosi[T_SET_BOX][1];
	TradeEndX=DownTradePosi[T_SET_BOX][2];
	TradeEndY =DownTradePosi[T_SET_BOX][3];
	
	for(i=pItem->x+11 ; i<pItem->x+pItem->w ; i+=22){
		for( j=pItem->y+11 ; j<pItem->y+pItem->h ; j+=22){
			if(TradeStartX <= i && TradeEndX > i && TradeStartY <=j && TradeEndY > j){ //인벤토리 박스에 들어갔는지를 체크 
				TradeColorRect.left =  TradeStartX +( ( ( i - TradeStartX) /22)*22) ; 
				TradeColorRect.top  =  TradeStartY +( ( ( j - TradeStartY) /22)*22) ; 
				TradeColorRect.right  = pItem->w;
				TradeColorRect.bottom = pItem->h;
				if(TradeEndX < (TradeColorRect.left + TradeColorRect.right)-11 || TradeEndY < (TradeColorRect.top + TradeColorRect.bottom)-11){
					InitTradeColorRect(); //컬러 박스 RECT 초기화 (컬러 박스 RECT중 하나라두 0 이면 셋팅할수 없다 
					return FALSE;
				}

				pItem->SetX = TradeColorRect.left;
				pItem->SetY = TradeColorRect.top;
				pItem->ItemPosition = 0; //배열의 인덱스를 저장한다 
				TradeColorIndex = SET_ITEM_CHECK_COLOR; //셋팅할영역

				if(TradeCrashItemIndex[0] = CrashTradeItem(TradeColorRect)){  //겹치는 아이템이 있나 체크 
					TradeColorIndex = NOT_SETTING_COLOR;
						return FALSE;
				}
				// 박재원 - 거래 관련 아이템 설정
				//증정용 아이템이나 복사아이템은 트레이드 할수없다  (포지션이 있는 아이템도 트레이드할수없다)
				if(pItem->sItemInfo.SpecialItemFlag[0] == CHECK_COPY_ITEM || 
					pItem->sItemInfo.SpecialItemFlag[0] == CHECK_GIVE_ITEM ||
					pItem->sItemInfo.ItemKindCode == ITEM_KIND_QUEST  ||
					(pItem->sItemInfo.CODE & sinITEM_MASK2) == sinMA1 ||
					(pItem->sItemInfo.CODE & sinITEM_MASK2) == sinMA2 ||
					(pItem->sItemInfo.CODE & sinITEM_MASK2) == sinQW1 ||
					(pItem->sItemInfo.CODE & sinITEM_MASK2) == sinBI1 ||
					(pItem->sItemInfo.CODE & sinITEM_MASK2) == sinBI2 ||		//패키지캐쉬 아이템도 트레이드 할수없다.
					//파티복 코스튬 
					(pItem->sItemInfo.CODE == (sinDA1|sin31))	|| //박재원 : 송편아뮬렛sin31과 파티복 sin31 아이템이 겹쳐서 구분함
					(pItem->sItemInfo.CODE == (sinDA2|sin31))	|| //박재원 : 송편아뮬렛sin31과 파티복 sin31 아이템이 겹쳐서 구분함
					(pItem->sItemInfo.CODE == (sinDA1|sin32))	|| //박재원 : 산타아뮬렛sin32과 파티복 sin32 아이템이 겹쳐서 구분함					
					(pItem->sItemInfo.CODE == (sinDA2|sin32))	|| //박재원 : 산타아뮬렛sin32과 파티복 sin32 아이템이 겹쳐서 구분함
					(pItem->sItemInfo.CODE == (sinDA1|sin33))	|| //박재원 : 이벤트아뮬렛sin33과 파티복 sin33 아이템이 겹쳐서 구분함					
					(pItem->sItemInfo.CODE == (sinDA2|sin33))	|| //박재원 : 이벤트아뮬렛sin33과 파티복 sin33 아이템이 겹쳐서 구분함	
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
					(pItem->sItemInfo.CODE == (sinDA1|sin54))   || (pItem->sItemInfo.CODE == (sinDA2|sin54))      || // 박재원 - 수영복 복장 추가 - 거래 금지(거래금지)
					(pItem->sItemInfo.CODE == (sinDA1|sin55))   || (pItem->sItemInfo.CODE == (sinDA2|sin55))      || // 박재원 - 수영복 복장 추가 - 거래 금지(거래금지)
//					(pItem->sItemInfo.CODE & sinITEM_MASK3) == sin48 ||	//한복 Kyle수정
					pItem->sItemInfo.CODE == (sinGF1|sin01)		||  (pItem->sItemInfo.CODE == (sinOR2|sin33))	  || // 장별 - 하트링(7일) 추가
					//pItem->sItemInfo.CODE == (sinGF1|sin02) ||
					(pItem->sItemInfo.CODE == (sinOA1|sin36))	||	 // 장별 - 눈꽃 목걸이(7일) 추가 
					(pItem->sItemInfo.CODE == (sinOA1|sin37))	||   // 장별 - 캔디데이즈 하트아뮬렛(7일) 추가
					(pItem->sItemInfo.CODE == (sinDB1|sin33))	||	 // 장별 - 스피드 부츠(1일) 추가
					(pItem->sItemInfo.CODE == (sinOA2|sin33))	||   // 장별 - 슈퍼 암릿(1일)
					(pItem->sItemInfo.CODE == (sinBI1|sin84))	||	 // 장별 - 그라비티 스크롤
					(pItem->sItemInfo.CODE == (sinDB1|sin34))	||	 // 장별 - 스피드 부츠(1시간) 추가
					(pItem->sItemInfo.CODE == (sinOA2|sin34))	||   // 장별 - 슈퍼 암릿(1시간)
					pItem->sItemInfo.CODE == (sinGF1|sin07)		||	// 장별 - 조사원을 찾아라
					pItem->sItemInfo.CODE == (sinGF1|sin08)		||	// 장별 - 조사원을 찾아라
					pItem->ItemPosition){
					TradeColorIndex = NOT_SETTING_COLOR;
						return FALSE;

				}

				int kk =0;
				//등록된 아이템은 트레이드 할수없다
				for(kk = 0 ; kk < NotSet_Item_CODECnt ; kk++){
					if(NotSet_Item_CODE[kk] == pItem->sItemInfo.CODE){
						TradeColorIndex = NOT_SETTING_COLOR;
						return FALSE;
					}

				}
				for(kk = 0 ; kk < NotSet_Item_MASKCnt ; kk++){
					if((pItem->sItemInfo.CODE & sinITEM_MASK2) == NotSet_Item_MASK[kk]){
						TradeColorIndex = NOT_SETTING_COLOR;
						return FALSE;
					}
				}
				for(kk = 0 ; kk < NotSet_Item_KINDCnt ; kk++){
					if(NotSet_Item_KIND[kk] == pItem->sItemInfo.ItemKindCode){
						TradeColorIndex = NOT_SETTING_COLOR;
						return FALSE;
					}
				}

				//이벤트 아이템은 트레이드할수없다 
				/*
				if((pItem->sItemInfo.CODE & sinITEM_MASK2) == sinGP1){
					TradeColorIndex = NOT_SETTING_COLOR;
					return FALSE;
				}
				*/

				return TRUE;
			}
		
		}
	
	}

	return TRUE;
}
//박스 초기화 
int cTRADE::InitTradeColorRect()
{
	TradeColorRect.left   = 0;   
	TradeColorRect.top    = 0;
	TradeColorRect.right  = 0;
	TradeColorRect.bottom = 0;
	return TRUE;

}
//아이템이 겹쳐졌는지를 찾는다 
int cTRADE::CrashTradeItem(RECT &desRect ,int PassItemIndex , int Kind)
{

	RECT rect;
	int i;
	if(Kind == 0){
		for(i=0 ; i<MAX_TRADE_ITEM ; i++){
			if(sTrade.TradeItem[i].Flag){
				if(PassItemIndex != 0)
					if(PassItemIndex == i+1)continue;
				rect.left   = sTrade.TradeItem[i].x;
				rect.right  = sTrade.TradeItem[i].x+sTrade.TradeItem[i].w;
				rect.top    = sTrade.TradeItem[i].y;
				rect.bottom = sTrade.TradeItem[i].y+sTrade.TradeItem[i].h;

				if( desRect.left > rect.left - desRect.right  && desRect.left < rect.right &&
					desRect.top > rect.top - desRect.bottom && desRect.top< rect.bottom ){
					return i+1; //충돌 체크된 아이템 인덱스를 하나더해서 리턴한다 
				}
			}
		}
	}
	if(Kind == 1){
		for(i=0 ; i<100 ; i++){
			if(sWareHouse.WareHouseItem[i].Flag){
				if(PassItemIndex != 0)
					if(PassItemIndex == i+1)continue;
				rect.left   = sWareHouse.WareHouseItem[i].x;
				rect.right  = sWareHouse.WareHouseItem[i].x+sWareHouse.WareHouseItem[i].w;
				rect.top    = sWareHouse.WareHouseItem[i].y;
				rect.bottom = sWareHouse.WareHouseItem[i].y+sWareHouse.WareHouseItem[i].h;
	
				if( desRect.left > rect.left - desRect.right  && desRect.left < rect.right &&
				desRect.top > rect.top - desRect.bottom && desRect.top< rect.bottom ){
					return i+1; //충돌 체크된 아이템 인덱스를 하나더해서 리턴한다 
				}
			}
		}

	}
	if(Kind == 2){	//크래프트 
		for(i=0 ; i<15 ; i++){
			if(sCraftItem.CraftItem[i].Flag){
				if(PassItemIndex != 0)
					if(PassItemIndex == i+1)continue;
				rect.left   = sCraftItem.CraftItem[i].x;
				rect.right  = sCraftItem.CraftItem[i].x+sCraftItem.CraftItem[i].w;
				rect.top    = sCraftItem.CraftItem[i].y;
				rect.bottom = sCraftItem.CraftItem[i].y+sCraftItem.CraftItem[i].h;
	
				if( desRect.left > rect.left - desRect.right  && desRect.left < rect.right &&
				desRect.top > rect.top - desRect.bottom && desRect.top< rect.bottom ){
					return i+1; //충돌 체크된 아이템 인덱스를 하나더해서 리턴한다 
				}
			}
		}
	}
	if(Kind == 3){ //에이징 
		for(i=0 ; i<15 ; i++){
			if(sAgingItem.AgingItem[i].Flag){
				if(PassItemIndex != 0)
					if(PassItemIndex == i+1)continue;
				rect.left   = sAgingItem.AgingItem[i].x;
				rect.right  = sAgingItem.AgingItem[i].x+sAgingItem.AgingItem[i].w;
				rect.top    = sAgingItem.AgingItem[i].y;
				rect.bottom = sAgingItem.AgingItem[i].y+sAgingItem.AgingItem[i].h;
	
				if( desRect.left > rect.left - desRect.right  && desRect.left < rect.right &&
				desRect.top > rect.top - desRect.bottom && desRect.top< rect.bottom ){
					return i+1; //충돌 체크된 아이템 인덱스를 하나더해서 리턴한다 
				}
			}
		}
	}
	// pluto 제련
	if( Kind == 4 )
	{
		for( i = 0; i < 5; i++ )
		{
			if( S_smeltingItem.SmeltingItem[i].Flag )
			{
				if( PassItemIndex != 0 )
				{
					if( PassItemIndex == i+1 ) continue;
				}
				rect.left	= S_smeltingItem.SmeltingItem[i].x;
				rect.right	= S_smeltingItem.SmeltingItem[i].x+S_smeltingItem.SmeltingItem[i].w;
				rect.top	= S_smeltingItem.SmeltingItem[i].y;
				rect.bottom = S_smeltingItem.SmeltingItem[i].y+S_smeltingItem.SmeltingItem[i].h;

				if( desRect.left > rect.left - desRect.right && desRect.left < rect.right &&
					desRect.top > rect.top - desRect.bottom && desRect.top < rect.bottom )
				{
					return i + 1;
				}
			}
		}
	}
	// pluto 제작
	if( Kind == 5 )
	{
		for( i = 0; i < 4; i++ )
		{
			if( g_sManufactureItem.ManufactureItem[i].Flag )
			{
				if( PassItemIndex != 0 )
				{
					if( PassItemIndex == i+1 ) continue;
				}
				rect.left	= g_sManufactureItem.ManufactureItem[i].x;
				rect.right	= g_sManufactureItem.ManufactureItem[i].x + g_sManufactureItem.ManufactureItem[i].w;
				rect.top	= g_sManufactureItem.ManufactureItem[i].y;
				rect.bottom	= g_sManufactureItem.ManufactureItem[i].y + g_sManufactureItem.ManufactureItem[i].h;

				if( desRect.left > rect.left - desRect.right && desRect.left < rect.right &&
					desRect.top > rect.top - desRect.bottom && desRect.top < rect.bottom )
				{
					return i + 1;
				}
			}
		}
	}

	// 석지용 - 믹스쳐 리셋
	if( Kind == 6 )
	{
		for( i = 0; i < 2; i++ )
		{
			if( sMixtureResetItem.MixtureResetItem[i].Flag )
			{
				if( PassItemIndex != 0 )
				{
					if( PassItemIndex == i+1 )
						continue;
				}

				// 해당 아이템의 좌표를 얻는다.
				rect.left	= sMixtureResetItem.MixtureResetItem[i].x;
				rect.right	= sMixtureResetItem.MixtureResetItem[i].x + sMixtureResetItem.MixtureResetItem[i].w;
				rect.top	= sMixtureResetItem.MixtureResetItem[i].y;
				rect.bottom	= sMixtureResetItem.MixtureResetItem[i].y + sMixtureResetItem.MixtureResetItem[i].h;

				if( desRect.left > rect.left - desRect.right && desRect.left < rect.right &&
					desRect.top > rect.top - desRect.bottom && desRect.top < rect.bottom )
				{
					return i + 1;
				}
			}
		}
	}

	return FALSE;

}

//겹친 아이템을 셋팅한다  
/*
int cTRADE::ChangeTradeItem(sITEM *pItem)
{

	if(TradeCrashItemIndex[0]){//충돌 된 아이템이 있으면 아이템을 바꿔준다 
		memcpy(&TempItem,&sTrade.TradeItem[TradeCrashItemIndex[0]-1],sizeof(sITEM)); //마우스 아이템을 템프로 복사 
		sTrade.TradeItem[TradeCrashItemIndex[0]-1].Flag = 0 ; //복사한후에 초기화
		TempItem.x = pItem->x;
		TempItem.y = pItem->y;
		LastSetTradeItem(pItem); //마우스 아이템을 인벤토리로 복사 
		memcpy(pItem,&TempItem,sizeof(sITEM));
		SendTradeItem(TradeCharCode);
		return TRUE;
		
	}
	return FALSE;
}
*/
//트레이드창으로  아이템으로 복사한다 
int cTRADE::LastSetTradeItem(sITEM *pItem)
{
	CheckTradeItemForm();

	for(int j=0 ; j < MAX_TRADE_ITEM ; j++){
		if(!sTrade.TradeItem[j].Flag){
			if(!CheckTrade((void *)&sTrade)) //잘못된 데이타 이면 거래창을 닫는다
				CancelTradeItem();

			memcpy(&sTrade.TradeItem[j],pItem,sizeof(sITEM));
			sTrade.TradeItem[j].x = pItem->SetX;
			sTrade.TradeItem[j].y = pItem->SetY;
			pItem->Flag = 0; //초기화 
			sTrade.CheckFlag = 0; //아이템이 셋팅되면 플랙을 초기화한다 
			sinPlaySound(sTrade.TradeItem[j].SoundIndex);
			ReformTrade((void *)&sTrade);
			GetWeight(); //무게를 체크한다 
			SendTradeItem(TradeCharCode);
			ReFormTradeItem();
			return TRUE;
			
		}
	}
	cMessageBox.ShowMessage(MESSAGE_TRADE_OVER_NUM);
	ReFormTradeItem();	
	return FALSE;
}

int cTRADE::PickUpTradeItem(int x , int y , int PickUpFlag)
{
	int i=0;
	SelectTradeItemIndex = 0;    //집을 아이템 
	ItemKindFlag = 0;			 //아이템 배경을 나타내기 위해서 보정해준다 
	for(i=0 ; i < MAX_TRADE_ITEM ; i++){
		if(sTrade.TradeItem[i].Flag){
			if ( sTrade.TradeItem[i].x < x  && sTrade.TradeItem[i].x + sTrade.TradeItem[i].w > x  && 
				sTrade.TradeItem[i].y < y  && sTrade.TradeItem[i].y + sTrade.TradeItem[i].h > y ){
				if(PickUpFlag){
					CheckTradeItemForm();
					if(!CheckTrade((void *)&sTrade)) //잘못된 데이타 이면 거래창을 닫는다
						CancelTradeItem();
					memcpy(&MouseItem , &sTrade.TradeItem[i] , sizeof(sITEM));
					sTrade.TradeItem[i].Flag = 0;
					sinPlaySound(sTrade.TradeItem[i].SoundIndex);
					TradeColorIndex = 0;
					CheckDelayFlag =1;
					sTrade.CheckFlag =0;
					ReformTrade((void *)&sTrade);
					GetWeight(); //무게를 체크한다 
					SendTradeItem(TradeCharCode);
					ReFormTradeItem();
					return TRUE;
				}
				else{
					SelectTradeItemIndex = i+1;
					TradeColorIndex = SELECT_ITEM_COLOR;
					ItemKindFlag = 1;
					sinShowItemInfoFlag = 1; //아이템의 정보를 보여준다 
					cItem.ShowItemInfo(&sTrade.TradeItem[i] , 2);
				}
			}
		}
	}
	for(i=0 ; i < MAX_TRADE_ITEM ; i++){
		if(sTradeRecv.TradeItem[i].Flag){
			if ( sTradeRecv.TradeItem[i].x < x  && sTradeRecv.TradeItem[i].x + sTradeRecv.TradeItem[i].w > x  && 
				sTradeRecv.TradeItem[i].y-T_REVISION_Y < y  && sTradeRecv.TradeItem[i].y-T_REVISION_Y + sTradeRecv.TradeItem[i].h > y ){
				SelectTradeItemIndex = i+1;
				TradeColorIndex = SELECT_ITEM_COLOR;
				ItemKindFlag = 2;
				sinShowItemInfoFlag = 1; //아이템의 정보를 보여준다 
				sTradeRecv.TradeItem[i].y -= T_REVISION_Y;
				cItem.ShowItemInfo(&sTradeRecv.TradeItem[i] , 2);
				sTradeRecv.TradeItem[i].y += T_REVISION_Y;
				return TRUE;

			}
		}
	}

	return FALSE;
}


/* 셋팅 */
int cTRADE::CheckTradeItemSet()
{ 
	int i=0;
	int TempWeight=0;

	memcpy(&CheckItem,&cInvenTory.InvenItem,sizeof(sITEM)*100); //공간 체크를 위해서 복사 
	memcpy(&CheckRecvItem,&sTradeRecv.TradeItem,sizeof(sITEM)*MAX_TRADE_ITEM); //체크를 위해 복사 

	////무게 관련 
	for(i=0;i<MAX_TRADE_ITEM;i++){
		if(CheckRecvItem[i].Flag){
			if(CheckRecvItem[i].Class == ITEM_CLASS_POTION)//물약일경우는 포션 카운트를 정해준다 
				TempWeight += CheckRecvItem[i].sItemInfo.PotionCount;
			else
				TempWeight +=CheckRecvItem[i].sItemInfo.Weight;
		}
	}
	if(TempWeight+sinChar->Weight[0] > sinChar->Weight[1]){
		cMessageBox.ShowMessage(MESSAGE_OVER_WEIGHT); //무게 초과 
		return FALSE;

	}
	for(i=0;i<MAX_TRADE_ITEM;i++){
		if(CheckRecvItem[i].Flag){
			if(!TradeCheckEmptyArea(&CheckRecvItem[i])){//아이템이 셋팅될수있는지를 체크한다
				cMessageBox.ShowMessage(MESSAGE_OVER_SPACE); //공간 초과  
				return FALSE;
			}
		}
	}

	//소지할수있는 돈을 초과했을경우 
	if(sTradeRecv.Money > 0){
		if(!cInvenTory.CheckMoneyLimit(sTradeRecv.Money-193))
			return FALSE;

	}

	return TRUE;
}

//임시 셋팅 
int cTRADE::TradeCheckEmptyArea(sITEM *pItem)
{
	int cntx,cnty;
	int cx,cy;

	RECT	rect={0,0,0,0};
	int		flag=0;

	int StartX = 23;  //인벤 박스의 시작점 X (트레이드 박스가 아님!!!!)
	int StartY = 436; // Y

	cx = (22*INVENTORY_BOX_ROW)-pItem->w; //아이템이 들어갈수있는 가장 끝 X좌표 
	cy = (22*INVENTORY_BOX_COL)-pItem->h; //아이템이 들어갈수있는 가장 끝 Y좌표

	//코드가 같지 않거나 아이템이 비어있지 않은 곳에 셋팅할때 검색해서 위치를 찾아낸다 
	for( cntx=StartX ; cntx <= StartX+cx ; cntx+=22){
		for( cnty=StartY ; cnty <= StartY+cy ; cnty+=22){ 
			flag = 0;
			for(int i=0 ; i<INVENTORY_MAXITEM ; i++){
				if(CheckItem[i].Flag){
					rect.left   = CheckItem[i].x;
					rect.right  = CheckItem[i].x+CheckItem[i].w;
					rect.top    = CheckItem[i].y;
					rect.bottom = CheckItem[i].y+CheckItem[i].h;

					if( cntx > rect.left-pItem->w && cntx< rect.right && 
						cnty > rect.top-pItem->h && cnty< rect.bottom )
						flag++;
				}
			}
			if ( flag==0 ) {
				pItem->SetX = cntx;
				pItem->SetY = cnty;
				pItem->ItemPosition = 0; 
				if(ChecketInvenItem(pItem)) //오토셋팅일경우에는 인자1을 넘긴다 
					return TRUE;

			}
		}
	}

	return FALSE;

}

int cTRADE::ChecketInvenItem(sITEM *pItem)
{
	for(int j=0 ; j < INVENTORY_MAXITEM ; j++)
		if(CheckItem[j].Flag == 0){
			memcpy(&CheckItem[j],pItem,sizeof(sITEM));
			CheckItem[j].x = pItem->SetX;
			CheckItem[j].y = pItem->SetY;
			pItem->Flag = 0; //초기화 
			return TRUE;
			
		}

	return FALSE;
}

//인벤토리로 아이템을 셋팅한다 
int cTRADE::CheckOkTradeItem()
{
	int i=0;
	for(i=0;i<MAX_TRADE_ITEM;i++){
		if(sTradeRecv.TradeItem[i].Flag){
			if(cInvenTory.CheckRequireItemToSet(&sTradeRecv.TradeItem[i])){         //곧바로 셋팅될수있는지를 체크한다  
				if(!cInvenTory.AutoSetInvenItem(&sTradeRecv.TradeItem[i])){
					if(sinThrowItemToFeild(&sTradeRecv.TradeItem[i])){//아이템을 버린다 
					sTradeRecv.TradeItem[i].Flag = 0;
					}

				}
			}
		}
	}
	if(sTradeRecv.Money > 0){
		CheckCharForm();//인증 
		//sinChar->Money += sTradeRecv.Money-193;
		sinPlusMoney(sTradeRecv.Money-193);
		ReformCharForm();//재인증 	
		SendSaveMoney(); //금액 조작을 못하게하기위해 호출한다 
		sinPlaySound(SIN_SOUND_COIN);
	}
	cTrade.OpenFlag = FALSE;				//트레이드 성공시 창을 닫는다 
	TempShowMoney = 0;
	ItemPickUpFlag = 0; //플랙 초기화 
	
	cInvenTory.CheckWeight();	//인벤토리 의 무게를 보정한다 

	return TRUE;
}


//트레이드 취소시 아이템을  인벤토리로 셋팅한다 
int cTRADE::CancelTradeItem()
{
	int sArrowPosi = 0;

	if(TradeItemSucessFlag){
		memset(&sTrade,0,sizeof(sTRADE));		//비정상적인 트레이드 종료시 아이템을 날린다 
		memset(&sTradeRecv,0,sizeof(sTRADE));	

	}
	int i=0;
	for(i=0;i<MAX_TRADE_ITEM;i++){
		if(sTrade.TradeItem[i].Flag){
			if(cInvenTory.CheckRequireItemToSet(&sTrade.TradeItem[i])){ //곧바로 셋팅될수있는지를 체크한다  
				if(!cInvenTory.AutoSetInvenItem(&sTrade.TradeItem[i])){
					//아이템이 꽉차있을경우는 인벤토리를 뒤집어서 셋팅한다
					if(ArrowState[0]==0)sArrowPosi = 2; //요기는 인벤창고 
					else sArrowPosi = 1;
					sinButtonFlag = sArrowPosi;
					SelectInvenItemIndex = 0; //에테르 코어남발을 막기위해 초기화
					cInvenTory.ChangeABItem(sArrowPosi); // 2보다 작을때 인벤 박스 

					if(!cInvenTory.AutoSetInvenItem(&sTrade.TradeItem[i])){ //그래두 안되면 버린다 팔자려니 해라
						if(sinThrowItemToFeild(&sTrade.TradeItem[i])){//아이템을 버린다 
							sTrade.TradeItem[i].Flag = 0;
						}
					}

					/*
					//요기는 무기포지션(현제 사용하고있지않다)
					if(ArrowState[1]==0)sArrowPosi = 4;
					else sArrowPosi = 3;
					sinButtonFlag = sArrowPosi;
					sinChangeSetFlag = sArrowPosi;
					*/

					if(sinThrowItemToFeild(&sTrade.TradeItem[i])){//아이템을 버린다 
						sTrade.TradeItem[i].Flag = 0;
					}
				}
			}
		}
	}

	if(sTrade.Money > 0){
		CheckCharForm();//인증 
		//sinChar->Money += sTrade.Money-193;
		sinPlusMoney(sTrade.Money-193);
		ReformCharForm();//재인증 	
		SendSaveMoney(); //금액 조작을 못하게하기위해 호출한다 
		sinPlaySound(SIN_SOUND_COIN);
	}


	//마우스에 아이템이 있을경우에는 마우스 아이템도 인벤에 곧바로 셋팅한다 
	if(MouseItem.Flag){
		if(cInvenTory.CheckRequireItemToSet(&MouseItem)){         //곧바로 셋팅될수있는지를 체크한다  
			if(!cInvenTory.AutoSetInvenItem(&MouseItem)){
				if(sinThrowItemToFeild(&MouseItem)){//아이템을 버린다 
					MouseItem.Flag = 0;
				}
			}
		}

	}

	//메세지 박스 관련은 잠시 대기 

	if(sinMessageBoxShowFlag){ //메세지 박스가 있으면 냅다 닫는다 
		if(sMessageBox2[MESSAGE_MOVE_MONEY_TRADE].Flag){
			sMessageBox2[MESSAGE_MOVE_MONEY_TRADE].Flag = 0;
			sinMessageBoxShowFlag =0;
		}
	}

	//트레이드 취소시 창을 닫아준다
	cTrade.OpenFlag = FALSE;
	cMessageBox.ShowMessage( MESSAGE_TRADE_CANCEL );

	memset(&sTrade,0,sizeof(sTRADE));		//셋팅된후 값들을 초기화해준다 
	memset(&sTradeRecv,0,sizeof(sTRADE));	//셋팅된후 값들을 초기화해준다 
	TempShowMoney = 0;
	ItemPickUpFlag = 0; //플랙 초기화 

	CloseEachPlayer();
	cInvenTory.CheckWeight();	//인벤토리 의 무게를 보정한다
	return TRUE;
}

//아이템 이미지를 로드한다 
int cTRADE::LoadTradeItemIamge() 
{
	
	char szFilePath[256];
	int  cnt=0;
	for(int i=0;i<MAX_TRADE_ITEM;i++){
		if(sTradeRecv.TradeItem[i].Flag){
			for(int j=0 ; j < MAX_ITEM ; j++){
				if(sTradeRecv.TradeItem[i].sItemInfo.CODE == sItem[j].CODE){	
					wsprintf(szFilePath ,"Image\\sinImage\\Items\\%s\\it%s.bmp",sItem[j].ItemFilePath,sItem[j].LastCategory);

					if(!sItem[j].lpTempItem)
						sItem[j].lpTempItem = LoadDibSurfaceOffscreen(szFilePath);

					//복주 머니 관련
					if((sTradeRecv.TradeItem[i].sItemInfo.CODE & sinITEM_MASK2) == sinPZ1 || (sTradeRecv.TradeItem[i].sItemInfo.CODE & sinITEM_MASK2) == sinPZ2){
						sItem[j].lpTempItem = LoadDibSurfaceOffscreen(szFilePath); //퍼즐은 로드한다
						if(sTradeRecv.TradeItem[i].sItemInfo.PotionCount <= 1){
							//복주머니를 찾는다.
							for(cnt = 0 ; cnt < MAX_ITEM ; cnt++){
								if(sItem[cnt].sItemInfo.CODE == (sinPZ1|sin00)){
									break;
								}
							}
							wsprintf(szFilePath ,"Image\\sinImage\\Items\\%s\\it%s.bmp",sItem[cnt].ItemFilePath,sItem[cnt].LastCategory);
							sItem[j].lpTempItem = LoadDibSurfaceOffscreen(szFilePath); //이미지를 먼저읽어준다
						}
					}
					sTradeRecv.TradeItem[i].lpItem = sItem[j].lpTempItem;
					break;
				}
			}
		}
	}
	return TRUE;
}

int cTRADE::CheckTradeButtonOk()
{
	///////////////트레이드 체크 
	if(!TradeSendSucessFlag && sTrade.CheckFlag && sTradeRecv.CheckFlag){ //교환! 
		SendTradeCheckItem(TradeCharCode);
		TradeSendSucessFlag = TRUE;
	}

	return TRUE;
}

//무게를 얻어온다 
int cTRADE::GetWeight()
{
	sTrade.Weight = 0;
	for(int i=0;i<MAX_TRADE_ITEM;i++){
		if(sTrade.TradeItem[i].Flag){
			if(sTrade.TradeItem[i].Class == ITEM_CLASS_POTION)
				sTrade.Weight += sTrade.TradeItem[i].sItemInfo.PotionCount;
			else
				sTrade.Weight +=sTrade.TradeItem[i].sItemInfo.Weight;

		}

	}

	return TRUE;
}

/*******************************************************************************************/
/*							여기서 부터는 창고 
/*******************************************************************************************/

//아이템이 셋팅될 공간을 찾는다 
int cWAREHOUSE::SetWareHouseItemAreaCheck(sITEM *pItem)
{
	int i,j;
	TradeColorIndex= 0; //초기화
	TradeCrashItemIndex[0] = 0; //충돌된 아이템 Index 초기화 
	cTrade.InitTradeColorRect(); //초기화
	
	TradeStartX = 21;  
	TradeStartY = 136+sinInterHeight2;
	TradeEndX   = TradeStartX + (22*9);
	TradeEndY   = TradeStartY + (22*9);

	for(i=pItem->x+11 ; i<pItem->x+pItem->w ; i+=22){
		for( j=pItem->y+11 ; j<pItem->y+pItem->h ; j+=22){
			if(TradeStartX <= i && TradeEndX > i && TradeStartY <=j && TradeEndY > j){ //인벤토리 박스에 들어갔는지를 체크 
				TradeColorRect.left =  TradeStartX +( ( ( i - TradeStartX) /22)*22) ; 
				TradeColorRect.top  =  TradeStartY +( ( ( j - TradeStartY) /22)*22) ; 
				TradeColorRect.right  = pItem->w;
				TradeColorRect.bottom = pItem->h;
				if(TradeEndX < (TradeColorRect.left + TradeColorRect.right)-11 || TradeEndY < (TradeColorRect.top + TradeColorRect.bottom)-11){
					cTrade.InitTradeColorRect(); //컬러 박스 RECT 초기화 (컬러 박스 RECT중 하나라두 0 이면 셋팅할수 없다 
					return FALSE;
				}
				//퀘스트 아이템일경우에는 아이템을 창고에 넣을수엄따 
				if(pItem->sItemInfo.ItemKindCode == ITEM_KIND_QUEST || 
					(pItem->sItemInfo.CODE & sinITEM_MASK2) == sinMA1 ||
					(pItem->sItemInfo.CODE & sinITEM_MASK2) == sinMA2 ||
					(pItem->sItemInfo.CODE & sinITEM_MASK2) == sinQW1 ||
					pItem->sItemInfo.CODE == (sinGF1|sin01)){
					//pItem->sItemInfo.CODE == (sinGF1|sin02)){

					TradeColorIndex = NOT_SETTING_COLOR;
					return FALSE;
				}
				int kk =0;
				//등록된 아이템은 창고에 넣을수없다
				for(kk = 0 ; kk < NotSet_Item_CODECnt ; kk++){
					if(NotSet_Item_CODE[kk] == pItem->sItemInfo.CODE){
						TradeColorIndex = NOT_SETTING_COLOR;
						return FALSE;
					}

				}
				for(kk = 0 ; kk < NotSet_Item_MASKCnt ; kk++){
					if((pItem->sItemInfo.CODE & sinITEM_MASK2) == NotSet_Item_MASK[kk]){
						TradeColorIndex = NOT_SETTING_COLOR;
						return FALSE;
					}

				}
				for(kk = 0 ; kk < NotSet_Item_KINDCnt ; kk++){
					if(NotSet_Item_KIND[kk] == pItem->sItemInfo.ItemKindCode){
						TradeColorIndex = NOT_SETTING_COLOR;
						return FALSE;
					}
				}
				//////////// 이벤트 송편
				if(pItem->sItemInfo.ItemKindCode == ITEM_KIND_EVENT){
					TradeColorIndex = NOT_SETTING_COLOR;
					return FALSE;

				}
				if((pItem->sItemInfo.CODE & sinITEM_MASK2) == sinCH1){
					TradeColorIndex = NOT_SETTING_COLOR;
					return FALSE;
				}

				//if((pItem->sItemInfo.CODE & sinITEM_MASK2) == sinGP1)return FALSE;

				pItem->SetX = TradeColorRect.left;
				pItem->SetY = TradeColorRect.top;
				pItem->ItemPosition = 0; //배열의 인덱스를 저장한다 
				TradeColorIndex = SET_ITEM_CHECK_COLOR; //셋팅할영역

				if(TradeCrashItemIndex[0] = cTrade.CrashTradeItem(TradeColorRect,0,1)){  //겹치는 아이템이 있나 체크 
					//요기서 복사아이템을 체크한다 
					if(!CopyItemNotPickUp(&sWareHouse.WareHouseItem[TradeCrashItemIndex[0]-1],TradeCrashItemIndex[0]-1)){
						TradeColorIndex = NOT_SETTING_COLOR;
						TradeCrashItemIndex[0] = 0; //초기화 
						TradeCrashItemIndex[1] = 0; //초기화 
						return FALSE;

					}
					if(TradeCrashItemIndex[1] = cTrade.CrashTradeItem(TradeColorRect,TradeCrashItemIndex[0],1)){  //겹치는 아이템이 있나 체크 
						TradeColorIndex = NOT_SETTING_COLOR;
						TradeCrashItemIndex[1] = 0; //초기화 
						return FALSE;
					}
					TradeColorIndex = OVERLAP_ITEM_COLOR;

				}

				return TRUE;
			}
		
		}
	
	}

	return TRUE;
}


//아이템을 셋팅한다 
int cWAREHOUSE::LastSetWareHouseItem(sITEM *pItem)
{
	CheckWareHouseForm();
	
	for(int j=0 ; j < 100 ; j++){
		if(!sWareHouse.WareHouseItem[j].Flag){
			memcpy(&sWareHouse.WareHouseItem[j],pItem,sizeof(sITEM));
			sWareHouse.WareHouseItem[j].x = pItem->SetX;
			sWareHouse.WareHouseItem[j].y = pItem->SetY;
			pItem->Flag = 0; //초기화 
			sinPlaySound(sWareHouse.WareHouseItem[j].SoundIndex);
			GetWeight(); //무게를 체크한다 
			ReFormWareHouse();
			return TRUE;
			
		}
	}
	ReFormWareHouse();
	return FALSE;
}

//창고를 닫는다 
int cWAREHOUSE::CloseWareHouse()
{
	int RestoreFlag = 1;

	CloseWareHouseCheckFlag3 = 1;
	cWareHouse.OpenFlag = 0; //창고를 닫는다 
	if(WareHouseSaveFlag){
		if(!SaveWareHouse(&sWareHouse)){ //아이템을 저장시킨다 
			cWareHouse.RestoreInvenItem(); //저장이 되지않으면 복구해준다 
			cMessageBox.ShowMessage(MESSAGE_OVER_ITEM_NUM);
			RestoreFlag = 0;
		}
		else{
			if(BackUpInvenItem2){
				delete BackUpInvenItem2;
				BackUpInvenItem2 = 0;

			}
			if(BackUpInvenTempItem2){
				delete BackUpInvenTempItem2;
				BackUpInvenTempItem2 = 0;

			}
			if(BackUpsInven){
				delete BackUpsInven;
				BackUpsInven = 0;

			}

		}

	}
	WareHouseSaveFlag = 0;//플랙 초기화 
	ResetInvenItemCode();		//이벤토리 아이템 코드 초기화 ********************
	ResetInvenMoney();			//이벤토리 돈 검사 초기화
	return RestoreFlag;
}



//아이템을 집는다 
int cWAREHOUSE::PickUpWareHouseItem(int x , int y , int PickUpFlag)
{
	int i=0;
	SelectTradeItemIndex = 0;    //집을 아이템 
	for(i=0 ; i < 100 ; i++){
		if(sWareHouse.WareHouseItem[i].Flag){
			if ( sWareHouse.WareHouseItem[i].x < x  && sWareHouse.WareHouseItem[i].x + sWareHouse.WareHouseItem[i].w > x  && 
				sWareHouse.WareHouseItem[i].y < y  && sWareHouse.WareHouseItem[i].y + sWareHouse.WareHouseItem[i].h > y ){
				if(PickUpFlag){					
					
					//수정 하대용
					//창고이용시 인벤토리 빈공간이 없을경우는 이용이 불가능하다/
					//가상의 공간 체크용 임시 아이템
					sITEM TempItem;
					TempItem.w = ITEMSIZE*2;
					TempItem.h = ITEMSIZE*4;
					TempItem.Flag = 1;
					if(cInvenTory.CheckSetEmptyArea(&TempItem)){
						////////////요기서 복사된 아이템은 들수가없다 
						if(CopyItemNotPickUp(&sWareHouse.WareHouseItem[i],i)){ 
							CheckWareHouseForm();
							memcpy(&MouseItem , &sWareHouse.WareHouseItem[i] , sizeof(sITEM));
							sWareHouse.WareHouseItem[i].Flag = 0;
							sinPlaySound(sWareHouse.WareHouseItem[i].SoundIndex);
							TradeColorIndex = 0;
							GetWeight(); //무게를 체크한다 
							ReFormWareHouse();
							return TRUE;
						}
					}
					else{
						cMessageBox.ShowMessage(MESSAGE_OVER_SPACE);
						break;
					}
							
					////////////요기서 복사된 아이템은 들수가없다 
					if(CopyItemNotPickUp(&sWareHouse.WareHouseItem[i],i)){ 
						CheckWareHouseForm();
						memcpy(&MouseItem , &sWareHouse.WareHouseItem[i] , sizeof(sITEM));
						sWareHouse.WareHouseItem[i].Flag = 0;
						sinPlaySound(sWareHouse.WareHouseItem[i].SoundIndex);
						TradeColorIndex = 0;
						GetWeight(); //무게를 체크한다 
						ReFormWareHouse();
						return TRUE;
					}
					
				}
				else{
					SelectTradeItemIndex = i+1;
					TradeColorIndex = SELECT_ITEM_COLOR;
					sinShowItemInfoFlag = 1; //아이템의 정보를 보여준다 
					cItem.ShowItemInfo(&sWareHouse.WareHouseItem[i] , 2);
				}
			}
		}
	}
	return FALSE;
}

//겹친 아이템을 교환한다 
int cWAREHOUSE::ChangeWareHouseItem(sITEM *pItem)
{
	if(pItem->Class == ITEM_CLASS_POTION){	//물약은 창고에 보관할수없다 
		cMessageBox.ShowMessage(MESSAGE_POTION_NOT_SETTING);
		return TRUE;

	}
	if(pItem->sItemInfo.Weight + (sWareHouse.Weight[0]-197) > sWareHouse.Weight[1]-196 ){
		cMessageBox.ShowMessage(MESSAGE_OVER_WEIGHT);
		return TRUE;

	}

	CheckWareHouseForm();
	
	if(TradeCrashItemIndex[0]){//충돌 된 아이템이 있으면 아이템을 바꿔준다 
		if(sWareHouse.WareHouseItem[TradeCrashItemIndex[0]-1].Class == ITEM_CLASS_POTION){
			if(sWareHouse.WareHouseItem[TradeCrashItemIndex[0]-1].CODE == pItem->CODE){
				sWareHouse.WareHouseItem[TradeCrashItemIndex[0]-1].sItemInfo.PotionCount += pItem->sItemInfo.PotionCount;
				pItem->Flag = 0;
				GetWeight();
				ReFormWareHouse();
				return TRUE;

			}
		}
		memcpy(&TempItem,&sWareHouse.WareHouseItem[TradeCrashItemIndex[0]-1],sizeof(sITEM)); //마우스 아이템을 템프로 복사 
		sWareHouse.WareHouseItem[TradeCrashItemIndex[0]-1].Flag = 0 ; //복사한후에 초기화
		TempItem.x = pItem->x;
		TempItem.y = pItem->y;
		ReFormWareHouse();
		LastSetWareHouseItem(pItem); //마우스 아이템을 인벤토리로 복사 
		memcpy(pItem,&TempItem,sizeof(sITEM));
		GetWeight();
		ReFormWareHouse();
		return TRUE;
		
	}
	ReFormWareHouse();
	return FALSE;
}

//무게를 구한다 
int cWAREHOUSE::GetWeight()
{
	sWareHouse.Weight[0] = 197;
	for(int i=0;i<100;i++){
		if(sWareHouse.WareHouseItem[i].Flag){
			if(sWareHouse.WareHouseItem[i].Class == ITEM_CLASS_POTION)
				sWareHouse.Weight[0] += sWareHouse.WareHouseItem[i].sItemInfo.PotionCount;
			else
				sWareHouse.Weight[0] +=sWareHouse.WareHouseItem[i].sItemInfo.Weight;

		}

	}
	WareHouseSaveFlag = 1; //아이템이 바뀌게 되면 값을 셋팅해준다 	

	return TRUE;
}

//아이템의 이미지와 아이템 크기를 셋팅한다 
int cWAREHOUSE::LoadWareHouseItemIamge() 
{
	
	char szFilePath[256];
	int  cnt=0;
	for(int i=0;i<100;i++){
		if(sWareHouse.WareHouseItem[i].Flag){	 
			for(int j=0 ; j < MAX_ITEM ; j++){ 
				if(sWareHouse.WareHouseItem[i].sItemInfo.CODE == sItem[j].CODE){	
					sWareHouse.WareHouseItem[i].w = sItem[j].w; //크기 보정 
					sWareHouse.WareHouseItem[i].h = sItem[j].h;
					wsprintf(szFilePath ,"Image\\sinImage\\Items\\%s\\it%s.bmp",sItem[j].ItemFilePath,sItem[j].LastCategory);
					if(!sItem[j].lpTempItem)
						sItem[j].lpTempItem = LoadDibSurfaceOffscreen(szFilePath);
					//복주 머니 관련
					if((sWareHouse.WareHouseItem[i].sItemInfo.CODE & sinITEM_MASK2) == sinPZ1 || (sWareHouse.WareHouseItem[i].sItemInfo.CODE & sinITEM_MASK2) == sinPZ2){
						if(sWareHouse.WareHouseItem[i].sItemInfo.PotionCount == 2){
							wsprintf(szFilePath ,"Image\\sinImage\\Items\\%s\\it%s.bmp",sItem[j].ItemFilePath,sItem[j].LastCategory);
							sItem[j].lpTempItem = LoadDibSurfaceOffscreen(szFilePath); //이미지를 먼저읽어준다
							sWareHouse.WareHouseItem[i].w = sItem[j].w; //크기 보정 
							sWareHouse.WareHouseItem[i].h = sItem[j].h;

						}
						else{
							//복주머니를 찾는다.
							for(cnt = 0 ; cnt < MAX_ITEM ; cnt++){
								if(sItem[cnt].sItemInfo.CODE == (sinPZ1|sin00)){
									break;
								}
							}
							wsprintf(szFilePath ,"Image\\sinImage\\Items\\%s\\it%s.bmp",sItem[cnt].ItemFilePath,sItem[cnt].LastCategory);
							sItem[j].lpTempItem = LoadDibSurfaceOffscreen(szFilePath); //이미지를 먼저읽어준다
							sWareHouse.WareHouseItem[i].w = sItem[cnt].w; //크기 보정 
							sWareHouse.WareHouseItem[i].h = sItem[cnt].h;
						}
						
					}
					sWareHouse.WareHouseItem[i].lpItem = sItem[j].lpTempItem;
					break;
				}
			}
		}
	}
	
	cInterFace.CheckAllBox(SIN_WAREHOUSE);
//	sWareHouse.Money -=2023;
//	sWareHouse.Weight[0] -=197;
//	sWareHouse.Weight[1] -=196;
	

	return TRUE;
}

//인벤토리 아이템을 잠시 백업한다 
int cWAREHOUSE::BackUpInvenItem() 
{

	BackUpInvenItem2 = 0;
	BackUpInvenTempItem2 = 0;
	BackUpsInven = 0;
	
	BackUpInvenItem2 = new sITEM[100];
	BackUpInvenTempItem2 = new sITEM[100];
	BackUpsInven = new sINVENTORY[INVENTORY_MAX_POS];

	memcpy(BackUpInvenItem2,&cInvenTory.InvenItem,sizeof(sITEM)*100);
	memcpy(BackUpInvenTempItem2,&cInvenTory.InvenItemTemp,sizeof(sITEM)*100);
	memcpy(BackUpsInven,&sInven,sizeof(sINVENTORY)*INVENTORY_MAX_POS);

	InvenGold = sinChar->Money+2023;

	RealGold = InvenGold*3;

	return TRUE;
}


//아이템을 복구해준다 
int cWAREHOUSE::RestoreInvenItem() 
{
	//복구하기전에 현재 착용중인 아이템을 벗는다 (안해두 되는데 걍 한다 )
	sinSetCharItem(cInvenTory.InvenItem[sInven[0].ItemIndex-1].CODE , cInvenTory.InvenItem[sInven[0].ItemIndex-1].SetModelPosi, FALSE);
	sinSetCharItem(cInvenTory.InvenItem[sInven[1].ItemIndex-1].CODE , cInvenTory.InvenItem[sInven[1].ItemIndex-1].SetModelPosi, FALSE);
	sinSetCharItem(cInvenTory.InvenItem[sInven[2].ItemIndex-1].CODE , cInvenTory.InvenItem[sInven[2].ItemIndex-1].SetModelPosi, FALSE);
	
	memcpy(&cInvenTory.InvenItem,BackUpInvenItem2,sizeof(sITEM)*100);
	memcpy(&cInvenTory.InvenItemTemp,BackUpInvenTempItem2,sizeof(sITEM)*100);
	memcpy(&sInven,BackUpsInven,sizeof(sINVENTORY)*INVENTORY_MAX_POS); 
	
	//마우스 아이템을 없애준다
	if(MouseItem.Flag)
		MouseItem.Flag = 0;

	if(BackUpInvenItem2){
		delete BackUpInvenItem2;
		BackUpInvenItem2 = 0;

	}
	if(BackUpInvenTempItem2){
		delete BackUpInvenTempItem2;
		BackUpInvenTempItem2 = 0;

	}
	if(BackUpsInven){
		delete BackUpsInven;
		BackUpsInven = 0;

	}

	if(sInven[0].ItemIndex){ //무기 
		sinSetCharItem(cInvenTory.InvenItem[sInven[0].ItemIndex-1].CODE , cInvenTory.InvenItem[sInven[0].ItemIndex-1].SetModelPosi, TRUE);

	}

	if(sInven[1].ItemIndex){ //방패 
		sinSetCharItem(cInvenTory.InvenItem[sInven[1].ItemIndex-1].CODE , cInvenTory.InvenItem[sInven[1].ItemIndex-1].SetModelPosi, TRUE);

	}

	if(sInven[2].ItemIndex){ //갑옷 
		sinSetCharItem(cInvenTory.InvenItem[sInven[2].ItemIndex-1].CODE , cInvenTory.InvenItem[sInven[2].ItemIndex-1].SetModelPosi, TRUE);

	}
	if((InvenGold-2023 ) == (RealGold/3)-2023){
		CheckCharForm();//인증 
		sinChar->Money = InvenGold-2023;
		ReformCharForm();//재인증 	

	}
	else{
		SendSetHackUser(6); //해킹을 하려고했던 못된 유저를 고발 TRUE 접속 종료 

	}
	InvenGold = 0;
	RealGold = 0;

	cInvenTory.SetItemToChar(); //파라메터 셋팅 
	cInvenTory.ReFormInvenItem(); //아이템 인증을 다시 받는다 
	cInvenTory.CheckWeight();	  //무게 셋팅 	

	return TRUE;
}

/*****************************************************************************/
/*							아이템 조합										 */
/*****************************************************************************/
//아이템이 들어갈수있는지를 체크한다 
int cCRAFTITEM::SetCraftItemAreaCheck(sITEM *pItem , int Kind)
{

	int i,j;
	TradeColorIndex= 0; //초기화   (트레이드 창과 창고가 같이 쓴다) 
	TradeCrashItemIndex[0] = 0; //충돌된 아이템 Index 초기화 
	cTrade.InitTradeColorRect(); //초기화
	
	// 박재원 - 리스펙 스톤(리스펙(아이템 재구성)창에서 리스펙 스톤 자리에 아이템을 넣을 때 마우스에 달려있는 상태)
	if( pItem->sItemInfo.CODE == (sinBI1|sin90) )
	{
		int ReconStoneX=159+11,ReconStoneY=268+11+sinInterHeight2;
		TradeColorIndex= 0; //초기화   (트레이드 창과 창고가 같이 쓴다)
		if(ReconStoneX <= pItem->x+22 && ReconStoneX > pItem->x && ReconStoneY < pItem->y+22 && ReconStoneY > pItem->y)
		{		
			pItem->SetX = ReconStoneX +( ( ( pItem->x+22 - ReconStoneX) /22)*22)-11;
			pItem->SetY = ReconStoneY +( ( ( pItem->y+22 - ReconStoneY) /22)*22)-11;
			pItem->ItemPosition = 0; //배열의 인덱스를 저장한다 
			TradeColorIndex = SET_ITEM_CHECK_COLOR; //셋팅할영역
			if(TradeEndX < (TradeColorRect.left + TradeColorRect.right)-11 || TradeEndY < (TradeColorRect.top + TradeColorRect.bottom)-11){
				cTrade.InitTradeColorRect(); //컬러 박스 RECT 초기화 (컬러 박스 RECT중 하나라두 0 이면 셋팅할수 없다 
				return FALSE;
			}
			if(TradeCrashItemIndex[0] = cTrade.CrashTradeItem(TradeColorRect,0,2)){  //겹치는 아이템이 있나 체크 
				TradeColorIndex = NOT_SETTING_COLOR;
				return FALSE;
			}
			return TRUE;
		}
	}

	if((pItem->CODE & sinITEM_MASK2)  == sinOS1 || (pItem->CODE & sinITEM_MASK2)  == sinSE1){ //쉘텀일 경우 || 아이템 재구성 씰 
		TradeStartX = 218;
		TradeStartY = 193+sinInterHeight2;
		TradeEndX = TradeStartX+(3*22);
		TradeEndY = TradeStartY+(4*22);

		for(i=pItem->x+11 ; i<pItem->x+pItem->w ; i+=22){
			for( j=pItem->y+11 ; j<pItem->y+pItem->h ; j+=22){
				if(TradeStartX <= i && TradeEndX > i && TradeStartY <=j && TradeEndY > j){ //인벤토리 박스에 들어갔는지를 체크 
					TradeColorRect.left =  TradeStartX +( ( ( i - TradeStartX) /22)*22) ; 
					TradeColorRect.top  =  TradeStartY +( ( ( j - TradeStartY) /22)*22) ; 
					TradeColorRect.right  = pItem->w;
					TradeColorRect.bottom = pItem->h;
					if(TradeEndX < (TradeColorRect.left + TradeColorRect.right)-11 || TradeEndY < (TradeColorRect.top + TradeColorRect.bottom)-11){
						cTrade.InitTradeColorRect(); //컬러 박스 RECT 초기화 (컬러 박스 RECT중 하나라두 0 이면 셋팅할수 없다 
						return FALSE;
					}

					pItem->SetX = TradeColorRect.left;
					pItem->SetY = TradeColorRect.top;
					pItem->ItemPosition = 0; //배열의 인덱스를 저장한다 
					TradeColorIndex = SET_ITEM_CHECK_COLOR; //셋팅할영역

					//포스 오브
					if(cCraftItem.ForceFlag){      
						//돈이 부족할 경우 
						//--------------------------------------------------------------------------//
						#ifdef HASIEGE_MODE
							//공성전 세율적용 포스 오브머니를 체크한다.
							if(cShop.haBuyMoneyCheck(cCraftItem.CheckForceOrbPrice(pItem))==0){
						#else
							if(sinChar->Money < cCraftItem.CheckForceOrbPrice(pItem)){
						#endif
						//---------------------------------------------------------------------------//
							TradeColorIndex = NOT_ENOUGH_MONEY;
							return FALSE;				
						}
						
						#ifdef	_LANGUAGE_JAPANESE
						// 박재원  일본은 미라지까지 사용한다. 일반유저들은 
						// os101 ~ os110까지만 포스마스터한테 포스를 제작할 수 있다.
						if(!smConfig.DebugMode)
						{
							// 박재원 - 매직 포스는 호떡파일 상관없이 무조건 다 제작이 가능함
							// 일반 포스는 현재 인페르나 이니그마 벨룸, 오르도는 유저모드에서는 제작 불가능
							if( ((pItem->sItemInfo.CODE & sinITEM_MASK3) >= sin11) && ((pItem->sItemInfo.CODE & sinITEM_MASK3) <= sin14) )
							{
								TradeColorIndex = NOT_SETTING_COLOR;
								return FALSE;
							}
						}
						#else
						// Kyle 이니그마까지 사용
						if(!smConfig.DebugMode){
							if((pItem->sItemInfo.CODE & sinITEM_MASK3) >= sin12){
								TradeColorIndex = NOT_SETTING_COLOR;
								return FALSE;
							}
						}
						#endif

						/*
						// pluto 셀레스토까지 사용
						if(!smConfig.DebugMode){
							if((pItem->sItemInfo.CODE & sinITEM_MASK3) >= sin10){
								TradeColorIndex = NOT_SETTING_COLOR;
								return FALSE;
							}
						}
						*/
						/*
						//데비네까지 사용  
						if(!smConfig.DebugMode)
						{//gm모드가 되는건 gm모드만 되게한거다. 유저모드는 안되는거다!
							if((pItem->sItemInfo.CODE & sinITEM_MASK3) >= sin09)
							{
								TradeColorIndex = NOT_SETTING_COLOR;
								return FALSE;
							}
						}
						*/
					}

					//퀘스트 아이템일 경우에는 아이템을 셋팅할수 엄따 
					if(pItem->sItemInfo.ItemKindCode == ITEM_KIND_QUEST ||
						(pItem->sItemInfo.CODE & sinITEM_MASK2) == sinMA1 ||
						(pItem->sItemInfo.CODE & sinITEM_MASK2) == sinMA2 ||
						(pItem->sItemInfo.CODE & sinITEM_MASK2) == sinQW1 ||
						(pItem->sItemInfo.CODE & sinITEM_MASK2) == sinPZ1 || //퍼즐도 올릴수엄따
						(pItem->sItemInfo.CODE & sinITEM_MASK2) == sinPZ2 ||
						(pItem->sItemInfo.CODE & sinITEM_MASK2) == sinFO1 ||
						(pItem->sItemInfo.CODE & sinITEM_MASK2) == sinSP1 || // pluto 선물상자 송편... 계열
						(pItem->sItemInfo.CODE & sinITEM_MASK2) == sinWR1 || // 박재원 - 무기 제작서 계열
						(pItem->sItemInfo.CODE & sinITEM_MASK2) == sinDR1 || // 박재원 - 방어구 제작서 계열
						(pItem->sItemInfo.CODE & sinITEM_MASK2) == sinGP1 || // 박재원 - 크리스탈 계열
						(pItem->sItemInfo.CODE & sinITEM_MASK2) == sinCH1 || // 박재원 - 초콜릿, 사탕 계열
						( pItem->sItemInfo.CODE >= (sinOR2|sin06) && pItem->sItemInfo.CODE <= (sinOR2|sin25) ) || // 박재원 : 클랜치프링 계열
						pItem->sItemInfo.SpecialItemFlag[0] == CHECK_GIVE_ITEM ||
						pItem->sItemInfo.CODE == (sinGF1|sin01) ||
						pItem->sItemInfo.CODE == (sinGF1|sin02) ||
						pItem->sItemInfo.CODE == (sinGF1|sin03) ||
						pItem->sItemInfo.CODE == (sinGF1|sin04) ||
						pItem->sItemInfo.CODE == (sinGF1|sin05) ||
						pItem->sItemInfo.CODE == (sinGF1|sin06) ||
						pItem->sItemInfo.CODE == (sinOR2|sin27) || // 박재원 - 산타 링 추가
						pItem->sItemInfo.CODE == (sinOA1|sin32) || // 박재원 - 산타 아뮬렛 추가
						pItem->sItemInfo.CODE == (sinOR2|sin28) || // 박재원 - 이벤트 링(7일) 추가
						pItem->sItemInfo.CODE == (sinOA1|sin33) || // 박재원 - 이벤트 아뮬렛(7일) 추가
						pItem->sItemInfo.CODE == (sinOR2|sin29) || // 박재원 - 이벤트 링(1시간) 추가
						pItem->sItemInfo.CODE == (sinOA1|sin34) || // 박재원 - 이벤트 아뮬렛(1시간) 추가
						pItem->sItemInfo.CODE == (sinOR2|sin30) || // 박재원 - 이벤트 링(1일) 추가
						pItem->sItemInfo.CODE == (sinOA1|sin35) || // 박재원 - 이벤트 아뮬렛(1일) 추가
						pItem->sItemInfo.CODE == (sinOA2|sin31) || // 박재원 - 슈퍼 암릿(7일) 추가
						pItem->sItemInfo.CODE == (sinOA2|sin32) || // 박재원 - 슈퍼 암릿(30일) 추가
						pItem->sItemInfo.CODE == (sinDB1|sin31) || // 박재원 - 스피드 부츠(7일) 추가
						pItem->sItemInfo.CODE == (sinDB1|sin32) || // 박재원 - 스피드 부츠(30일) 추가
						pItem->sItemInfo.CODE == (sinSP1|sin15) || // 박재원 - 수박 아이템 추가
						pItem->sItemInfo.CODE == (sinSP1|sin27) || // 박재원 - 알파벳 조합 이벤트 추가
						pItem->sItemInfo.CODE == (sinSP1|sin28) || // 박재원 - 알파벳 조합 이벤트 추가
						pItem->sItemInfo.CODE == (sinSP1|sin29) || // 박재원 - 알파벳 조합 이벤트 추가
						pItem->sItemInfo.CODE == (sinSP1|sin30) || // 박재원 - 알파벳 조합 이벤트 추가
						pItem->sItemInfo.CODE == (sinSP1|sin31) || // 박재원 - 알파벳 조합 이벤트 추가
						pItem->sItemInfo.CODE == (sinSP1|sin32) || // 박재원 - 알파벳 조합 이벤트 추가
						pItem->sItemInfo.CODE == (sinSP1|sin33) || // 박재원 - 알파벳 조합 이벤트 추가
						pItem->sItemInfo.CODE == (sinOR2|sin31) || // 박재원 - 보스 몬스터 링 추가(바벨)
						pItem->sItemInfo.CODE == (sinOR2|sin32) ||  // 박재원 - 보스 몬스터 링 추가(퓨리)
						pItem->sItemInfo.CODE == (sinSP1|sin35) || // 장별 - 발렌타인 초콜릿 아이템 추가
						pItem->sItemInfo.CODE == (sinOR2|sin33) ||   // 장별 - 하트링(7일) 아이템 추가
						pItem->sItemInfo.CODE == (sinOA1|sin36)	|| // 장별 - 눈꽃 목걸이(7일) 아이템 추가
						pItem->sItemInfo.CODE == (sinSP1|sin36) || // 장별 - 캔디데이즈 캔디 아이템 추가
						pItem->sItemInfo.CODE == (sinOA1|sin37)	||   // 장별 - 하트아뮬렛(7일) 아이템 추가
						pItem->sItemInfo.CODE == (sinOA2|sin33) || // 장별 - 슈퍼 암릿(1일)
						pItem->sItemInfo.CODE == (sinDB1|sin33) || // 장별 - 스피드 부츠(1일) 추가
						pItem->sItemInfo.CODE == (sinSP1|sin34) || // 박재원 - 호랑이 캡슐 추가
						pItem->sItemInfo.CODE == (sinSP1|sin37) || // 장별 - 매지컬그린 비취 아이템 추가
						pItem->sItemInfo.CODE == (sinSP1|sin38) || // 장별 - 매지컬그린 에메랄드 아이템 추가
						pItem->sItemInfo.CODE == (sinBI1|sin84)  || // 장별 - 그라비티 스크롤 추가
						pItem->sItemInfo.CODE == (sinDB1|sin34) || // 장별 - 스피드 부츠(1시간) 추가
						pItem->sItemInfo.CODE == (sinOA2|sin34) || // 장별 - 슈퍼 암릿(1시간)
						pItem->sItemInfo.CODE == (sinSP1|sin39) || // 장별 - 카라의 눈물 아이템 추가
						pItem->sItemInfo.CODE == (sinGF1|sin07) || // 장별 - 조사원을 찾아라
						pItem->sItemInfo.CODE == (sinGF1|sin08) ||	// 장별 - 조사원을 찾아라
						pItem->sItemInfo.CODE == (sinOR2|sin36)	|| // 장별 - 소울스톤
						pItem->sItemInfo.CODE == (sinOR2|sin37)	||
						pItem->sItemInfo.CODE == (sinOR2|sin38)	||
						pItem->sItemInfo.CODE == (sinOR2|sin39)	||
						pItem->sItemInfo.CODE == (sinOR2|sin40)	||
						pItem->sItemInfo.CODE == (sinOA1|sin39)	||
						pItem->sItemInfo.CODE == (sinOA1|sin40)	||
						pItem->sItemInfo.CODE == (sinOA1|sin41)	||
						pItem->sItemInfo.CODE == (sinOA1|sin42)	||
						pItem->sItemInfo.CODE == (sinOA1|sin38)	|| // 장별 - 복날 이벤트 아뮬렛, 링 
						pItem->sItemInfo.CODE == (sinOR2|sin34)	||
						pItem->sItemInfo.CODE == (sinOR2|sin35)	)


					{
						TradeColorIndex = NOT_SETTING_COLOR;
						return FALSE;

					}
					
					// 박재원 - 매직 쉘텀 조합 금지
					if(!cCraftItem.ForceFlag)
					{
						if( (pItem->sItemInfo.CODE & sinITEM_MASK2) == sinOS1 )
						{
							if( (pItem->sItemInfo.CODE & sinITEM_MASK3)>=sin21 && (pItem->sItemInfo.CODE & sinITEM_MASK3)<=sin34 )
							{
								TradeColorIndex = NOT_SETTING_COLOR;
								return FALSE;
							}
						}
					}

					int kk =0;
					//등록된 아이템은 믹스할수없다
					for(kk = 0 ; kk < NotSet_Item_CODECnt ; kk++){
						if(NotSet_Item_CODE[kk] == pItem->sItemInfo.CODE){
							TradeColorIndex = NOT_SETTING_COLOR;
							return FALSE;
						}

					}
					for(kk = 0 ; kk < NotSet_Item_MASKCnt ; kk++){
						if((pItem->sItemInfo.CODE & sinITEM_MASK2) == NotSet_Item_MASK[kk]){
							TradeColorIndex = NOT_SETTING_COLOR;
							return FALSE;
						}

					}
					for(kk = 0 ; kk < NotSet_Item_KINDCnt ; kk++){
						if(NotSet_Item_KIND[kk] == pItem->sItemInfo.ItemKindCode){
							TradeColorIndex = NOT_SETTING_COLOR;
							return FALSE;
						}
					}

					//if((pItem->sItemInfo.CODE & sinITEM_MASK2) == sinGP1)return FALSE;

					if(Kind == 0){ //믹스쳐 
						if(TradeCrashItemIndex[0] = cTrade.CrashTradeItem(TradeColorRect,0,2)){  //겹치는 아이템이 있나 체크 
							TradeColorIndex = NOT_SETTING_COLOR;
								return FALSE;
	
						}
						return TRUE;

					}
					if(Kind == 1){ //에이징 
						if(TradeCrashItemIndex[0] = cTrade.CrashTradeItem(TradeColorRect,0,3)){  //겹치는 아이템이 있나 체크 
							TradeColorIndex = NOT_SETTING_COLOR;
								return FALSE;
	
						}
						return TRUE;

					}
				}
			
			}

		}
	}
	else{
		//포스 오브
		if(cCraftItem.ForceFlag)return FALSE;
		TradeColorIndex= 0; //초기화   (트레이드 창과 창고가 같이 쓴다)
		TradeCrashItemIndex[0] = 0; //충돌된 아이템 Index 초기화 
		cTrade.InitTradeColorRect(); //초기화

		TradeStartX = 58;
		TradeStartY = 193+sinInterHeight2;
		TradeEndX = TradeStartX+(3*22);
		TradeEndY = TradeStartY+(4*22);
		if(TradeStartX <= pCursorPos.x && TradeEndX > pCursorPos.x && TradeStartY <=pCursorPos.y && TradeEndY > pCursorPos.y){ //인벤토리 박스에 들어갔는지를 체크 
			TradeColorRect.left =  TradeStartX;
			TradeColorRect.top  =  TradeStartY;
			TradeColorRect.right  = TradeEndX - TradeStartX;
			TradeColorRect.bottom = TradeEndY - TradeStartY;


			pItem->SetX = TradeStartX+((TradeEndX - TradeStartX)-pItem->w)/2;
			pItem->SetY = TradeStartY+((TradeEndY - TradeStartY)-pItem->h)/2;
			pItem->ItemPosition = 0; //배열의 인덱스를 저장한다 
			TradeColorIndex = SET_ITEM_CHECK_COLOR; //셋팅할영역

			if(TradeCrashItemIndex[0] = cTrade.CrashTradeItem(TradeColorRect,0,2)){  //겹치는 아이템이 있나 체크 
				TradeColorIndex = NOT_SETTING_COLOR;
				return FALSE;

			}

			//돈이 부족할 경우 
			//--------------------------------------------------------------------------//
			#ifdef HASIEGE_MODE
					//공성전 세율적용 믹스비용을 체크한다.
				if(cShop.haBuyMoneyCheck(pItem->sItemInfo.Price)==0){ 
			#else
				if(sinChar->Money < pItem->sItemInfo.Price){
			#endif
			//---------------------------------------------------------------------------//
				TradeColorIndex = NOT_ENOUGH_MONEY;
				return FALSE;
			}
			//복사되 아이템은 믹스쳐와 에이징을 할수없다 MakeItem은 믹스할수없다 
			if(pItem->sItemInfo.SpecialItemFlag[0] == CHECK_COPY_ITEM ||
				pItem->sItemInfo.ItemKindCode == ITEM_KIND_QUEST  ||
				(pItem->sItemInfo.CODE & sinITEM_MASK2) == sinMA1 ||
				(pItem->sItemInfo.CODE & sinITEM_MASK2) == sinMA2 ||
				(pItem->sItemInfo.CODE & sinITEM_MASK2) == sinQW1 ||
				(pItem->sItemInfo.CODE & sinITEM_MASK2) == sinPZ1 || //퍼즐도 올릴수엄따 
				(pItem->sItemInfo.CODE & sinITEM_MASK2) == sinPZ2 ||
				(pItem->sItemInfo.CODE & sinITEM_MASK2) == sinFO1 || //포스도안된다
				(pItem->sItemInfo.CODE & sinITEM_MASK2) == sinSP1 || // pluto 선물상자 송편... 계열
				(pItem->sItemInfo.CODE & sinITEM_MASK2) == sinWR1 || // 박재원 - 무기 제작서 계열
				(pItem->sItemInfo.CODE & sinITEM_MASK2) == sinDR1 || // 박재원 - 방어구 제작서 계열
				(pItem->sItemInfo.CODE & sinITEM_MASK2) == sinGP1 || // 박재원 - 크리스탈 계열
				(pItem->sItemInfo.CODE & sinITEM_MASK2) == sinCH1 || // 박재원 - 초콜릿, 사탕 계열
				( pItem->sItemInfo.CODE >= (sinOR2|sin06) && pItem->sItemInfo.CODE <= (sinOR2|sin25) ) || // 박재원 : 클랜치프링 계열
				pItem->sItemInfo.CODE == (sinGF1|sin01) ||
				pItem->sItemInfo.CODE == (sinGF1|sin02) ||
			//	pItem->sItemInfo.CODE == (sinGF1|sin03) || 
			//	pItem->sItemInfo.CODE == (sinGF1|sin04) ||
				pItem->sItemInfo.CODE == (sinGF1|sin05) ||
				pItem->sItemInfo.CODE == (sinGF1|sin06) || 
				pItem->sItemInfo.CODE == (sinOR2|sin27) || // 박재원 - 산타 링 추가
				pItem->sItemInfo.CODE == (sinOA1|sin32) || // 박재원 - 산타 아뮬렛 추가 
				pItem->sItemInfo.CODE == (sinOR2|sin28) || // 박재원 - 이벤트 링 추가(7일)
				pItem->sItemInfo.CODE == (sinOA1|sin33) || // 박재원 - 이벤트 아뮬렛 추가(7일) 
				pItem->sItemInfo.CODE == (sinOR2|sin29) || // 박재원 - 이벤트 링 추가(1시간)
				pItem->sItemInfo.CODE == (sinOA1|sin34) || // 박재원 - 이벤트 아뮬렛 추가(1시간) 
				pItem->sItemInfo.CODE == (sinOR2|sin30) || // 박재원 - 이벤트 링 추가(1일)
				pItem->sItemInfo.CODE == (sinOA1|sin35) || // 박재원 - 이벤트 아뮬렛 추가(1일) 
				pItem->sItemInfo.CODE == (sinOA2|sin31) || // 박재원 - 슈퍼 암릿(7일) 추가
				pItem->sItemInfo.CODE == (sinOA2|sin32) || // 박재원 - 슈퍼 암릿(30일) 추가
				pItem->sItemInfo.CODE == (sinDB1|sin31) || // 박재원 - 스피드 부츠(7일) 추가
				pItem->sItemInfo.CODE == (sinDB1|sin32) || // 박재원 - 스피드 부츠(30일) 추가
				pItem->sItemInfo.CODE == (sinSP1|sin15) || // 박재원 - 수박 아이템 추가
				pItem->sItemInfo.CODE == (sinSP1|sin27) || // 박재원 - 알파벳 조합 이벤트 추가
				pItem->sItemInfo.CODE == (sinSP1|sin28) || // 박재원 - 알파벳 조합 이벤트 추가
				pItem->sItemInfo.CODE == (sinSP1|sin29) || // 박재원 - 알파벳 조합 이벤트 추가
				pItem->sItemInfo.CODE == (sinSP1|sin30) || // 박재원 - 알파벳 조합 이벤트 추가
				pItem->sItemInfo.CODE == (sinSP1|sin31) || // 박재원 - 알파벳 조합 이벤트 추가
				pItem->sItemInfo.CODE == (sinSP1|sin32) || // 박재원 - 알파벳 조합 이벤트 추가
				pItem->sItemInfo.CODE == (sinSP1|sin33) || // 박재원 - 알파벳 조합 이벤트 추가
				pItem->sItemInfo.CODE == (sinSP1|sin35) || // 장별 - 발렌타인 초콜릿 아이템 추가
				pItem->sItemInfo.CODE == (sinOR2|sin33) || // 장별 - 하트링 아이템 추가
				pItem->sItemInfo.CODE == (sinOR2|sin31) || // 박재원 - 보스 몬스터 링 추가(바벨)
				pItem->sItemInfo.CODE == (sinOR2|sin32) || // 박재원 - 보스 몬스터 링 추가(퓨리)
				pItem->sItemInfo.CODE == (sinOA1|sin36)	||   // 장별 - 눈꽃 목걸이(7일) 아이템 추가
				pItem->sItemInfo.CODE == (sinSP1|sin36) || // 장별 - 캔디데이즈 캔디 아이템 추가
				pItem->sItemInfo.CODE == (sinOA1|sin37)	||   // 장별 - 하트아뮬렛 아이템 추가
				pItem->sItemInfo.CODE == (sinOA2|sin33) || // 장별 - 슈퍼 암릿(1일)
				pItem->sItemInfo.CODE == (sinDB1|sin33) || // 장별 - 스피드 부츠(1일) 추가
				pItem->sItemInfo.CODE == (sinSP1|sin34) || // 박재원 - 호랑이 캡슐 추가 
				pItem->sItemInfo.CODE == (sinSP1|sin37) || // 장별 - 매지컬그린 비취 아이템 추가
				pItem->sItemInfo.CODE == (sinSP1|sin38) || // 장별 - 매지컬그린 에메랄드 아이템 추가
				pItem->sItemInfo.CODE == (sinBI1|sin84)	|| // 장별 - 그라비티 스크롤 추가
				pItem->sItemInfo.CODE == (sinDB1|sin34) || // 장별 - 스피드 부츠(1시간) 추가
				pItem->sItemInfo.CODE == (sinOA2|sin34) || // 장별 - 슈퍼 암릿(1시간)
				pItem->sItemInfo.CODE == (sinSP1|sin39) || // 장별 - 카라의 눈물 아이템 추가
				pItem->sItemInfo.CODE == (sinDA1|sin54) || pItem->sItemInfo.CODE == (sinDA2|sin54) ||// 박재원 - 수영복 복장 추가 - 조합금지
				pItem->sItemInfo.CODE == (sinDA1|sin55) || pItem->sItemInfo.CODE == (sinDA2|sin55)  ||  // 박재원 - 수영복 복장 추가 - 조합금지
				pItem->sItemInfo.CODE == (sinOR2|sin36)	|| // 장별 - 소울스톤
				pItem->sItemInfo.CODE == (sinOR2|sin37)	||
				pItem->sItemInfo.CODE == (sinOR2|sin38)	||
				pItem->sItemInfo.CODE == (sinOR2|sin39)	||
				pItem->sItemInfo.CODE == (sinOR2|sin40)	||
				pItem->sItemInfo.CODE == (sinOA1|sin39)	||
				pItem->sItemInfo.CODE == (sinOA1|sin40)	||
				pItem->sItemInfo.CODE == (sinOA1|sin41)	||
				pItem->sItemInfo.CODE == (sinOA1|sin42)	||
				pItem->sItemInfo.CODE == (sinOA1|sin38)	|| // 장별 - 복날 이벤트 아뮬렛, 링
				pItem->sItemInfo.CODE == (sinOR2|sin34)	||
				pItem->sItemInfo.CODE == (sinOR2|sin35)	
				)

			{ 

				TradeColorIndex = NOT_SETTING_COLOR;
				return FALSE;
			}

			// 박재원 - 매직 쉘텀 조합 금지
			if(!cCraftItem.ForceFlag)
			{
				if( (pItem->sItemInfo.CODE & sinITEM_MASK2) == sinOS1 )
				{
					if( (pItem->sItemInfo.CODE & sinITEM_MASK3)>=sin21 && (pItem->sItemInfo.CODE & sinITEM_MASK3)<=sin34 )
					{
						TradeColorIndex = NOT_SETTING_COLOR;
						return FALSE;
					}
				}
			}

			int kk =0;
			//등록된 아이템은 믹스할수없다
			for(kk = 0 ; kk < NotSet_Item_CODECnt ; kk++){
				if(NotSet_Item_CODE[kk] == pItem->sItemInfo.CODE){
					TradeColorIndex = NOT_SETTING_COLOR;
						return FALSE;
				}

			}
			for(kk = 0 ; kk < NotSet_Item_MASKCnt ; kk++){
				if((pItem->sItemInfo.CODE & sinITEM_MASK2) == NotSet_Item_MASK[kk]){
					TradeColorIndex = NOT_SETTING_COLOR;
						return FALSE;
				}

			}
			for(kk = 0 ; kk < NotSet_Item_KINDCnt ; kk++){
				if(NotSet_Item_KIND[kk] == pItem->sItemInfo.ItemKindCode){
					TradeColorIndex = NOT_SETTING_COLOR;
						return FALSE;
				}
					
			}

			/*
			//이벤트 아이템은 셋팅할수없다 
			if((pItem->sItemInfo.CODE & sinITEM_MASK2) == sinGP1){ 
				TradeColorIndex = NOT_SETTING_COLOR;
				return FALSE;
			}
			*/

			return TRUE;
		}

	}
	return TRUE;
}

//아이템을 셋팅한다 
// 박재원 - 포스 오브 제작 비용 수정
int ForceOrbPriceIndex[] = {200,500,1000,2000,4000,5000,10000,20000,30000,40000,50000,60000,70000,80000};  // 박재원 - 벨룸, 오르도 포스 비용 추가
// 박재원 - 매직 포스 추가(매직 포스 오프 제작 비용)
int MagicForceOrbPriceIndex[] = {200,500,1000,2000,4000,5000,10000,20000,30000,40000,50000,60000,70000,80000};

DWORD SheltomCode2[] = {sin01,sin02,sin03,sin04,sin05,sin06,sin07,sin08,sin09,sin10,sin11,sin12,sin13,sin14}; // 박재원 - 벨룸, 오르도 추가 

// 박재원 - 매직 포스 추가
DWORD MagicSheltomCode[] = {sin21,sin22,sin23,sin24,sin25,sin26,sin27,sin28,sin29,sin30,sin31,sin32, sin33, sin34}; 

// 박재원 - 빌링 매직 포스 추가
DWORD BillingMagicSheltomCode[] = {sin35, sin36, sin37};

int cCRAFTITEM::CheckForceOrbPrice()
{
	ForceItemPrice = 0;
	for(int j=1 ; j < 15 ; j++){
		if(sCraftItem.CraftItem[j].Flag){
			for(int t = 0; t < 14 ; t++) // 박재원 - 벨룸, 오르도 포스 추가
			{
				if( (sCraftItem.CraftItem[j].sItemInfo.CODE & sinITEM_MASK3) <= sin20 )
				{
					if((sCraftItem.CraftItem[j].sItemInfo.CODE & sinITEM_MASK3) == SheltomCode2[t])
					{
						ForceItemPrice += ForceOrbPriceIndex[t];
						break;
					}
				}
				else // 쉘텀 코드 sin21 ~
				{ // 박재원 - 매직 포스 추가
					if((sCraftItem.CraftItem[j].sItemInfo.CODE & sinITEM_MASK3) == MagicSheltomCode[t])
					{
						ForceItemPrice += MagicForceOrbPriceIndex[t];
						break;
					}
				}

			}
		}
	}
	ForceItemPrice2 = ForceItemPrice;
	return TRUE;
}
int cCRAFTITEM::CheckForceOrbPrice(sITEM *pItem)
{

	int TempPrice = 0;
	for(int j=1 ; j < 15 ; j++){
		if(sCraftItem.CraftItem[j].Flag){
			for(int t = 0; t < 12 ; t++){
				if((sCraftItem.CraftItem[j].sItemInfo.CODE & sinITEM_MASK3) == SheltomCode2[t]){
					TempPrice += ForceOrbPriceIndex[t];
					break;
				}
				// 박재원 - 매직 포스 추가
				else if((sCraftItem.CraftItem[j].sItemInfo.CODE & sinITEM_MASK3) == MagicSheltomCode[t]){
					TempPrice += MagicForceOrbPriceIndex[t];
					break;
				}
			}
		}
	}

	for(int t = 0; t < 12 ; t++){
		if((pItem->sItemInfo.CODE & sinITEM_MASK3) == SheltomCode2[t]){
			return TempPrice+ForceOrbPriceIndex[t];
			
		}
		// 박재원 - 매직 포스 추가
		if((pItem->sItemInfo.CODE & sinITEM_MASK3) == MagicSheltomCode[t]){
			return TempPrice+MagicForceOrbPriceIndex[t];
		}
	}
	return FALSE;
}
int cCRAFTITEM::LastSetCraftItem(sITEM *pItem,int Kind)
{
	int TempLevel=0;
	int TempLevel2=0;
	if(Kind == 0){ //크래프트 아이템 
		CheckCraftItemForm(); //조작하는 상놈의 자식들을 죽여버리자 퉤!!
		if((pItem->CODE & sinITEM_MASK2)  == sinOS1){ //쉘텀일 경우 
			for(int j=1 ; j < 15 ; j++){
				if(!sCraftItem.CraftItem[j].Flag){
					memcpy(&sCraftItem.CraftItem[j],pItem,sizeof(sITEM));
					sCraftItem.CraftItem[j].x = pItem->SetX;
					sCraftItem.CraftItem[j].y = pItem->SetY;
					pItem->Flag = 0; //초기화 
					sinPlaySound(sCraftItem.CraftItem[j].SoundIndex);
					ReFormCraftItem();
					CheckForceOrbPrice();
					return TRUE;
				}
			}
		}
		//HaGoon아이템 재구성 ===================================================================================================
		else if((pItem->CODE & sinITEM_MASK2)  == sinSE1){
			if(sCraftItem.CraftItem[0].Flag){
				TempLevel  = sCraftItem.CraftItem[0].sItemInfo.Level;
				for(int k=0;k<MAX_SEEL_COUNT;k++){ // 박재원 - 테이와즈 씰 추가 (3종류 -> 4종류)
					if(pItem->CODE  == sReconItem[k].dwCODE){
						if(!(TempLevel>= sReconItem[k].iLevel[0] &&TempLevel <= sReconItem[k].iLevel[1])){
								cMessageBox.ShowMessage(MESSAGE_NOT_RECONITEM);
								return FALSE;
						}
					}
				}
			}
			for(int j=1 ; j < 15 ; j++){
				if(!sCraftItem.CraftItem[j].Flag){
					memcpy(&sCraftItem.CraftItem[j],pItem,sizeof(sITEM));
					sCraftItem.CraftItem[j].x = pItem->SetX;
					sCraftItem.CraftItem[j].y = pItem->SetY;
					pItem->Flag = 0; //초기화 
					sinPlaySound(sCraftItem.CraftItem[j].SoundIndex);
					ReFormCraftItem();
					return TRUE;
				}
			}

		}
		//=======================================================================================================================
		// 박재원 - 리스펙 스톤
		else if((pItem->CODE & sinITEM_MASK2) == sinBI1)
		{
			if((pItem->CODE & sinITEM_MASK3) == sin90)
			{
				if(!sCraftItem.CraftItem[14].Flag){
					memcpy(&sCraftItem.CraftItem[14],pItem,sizeof(sITEM));
					sCraftItem.CraftItem[14].x = pItem->SetX;
					sCraftItem.CraftItem[14].y = pItem->SetY;
					pItem->Flag = 0; //초기화 
					sinPlaySound(sCraftItem.CraftItem[14].SoundIndex);
					return TRUE;
				}
			}
		}
		else{
			if(!sCraftItem.CraftItem[0].Flag){
				memcpy(&sCraftItem.CraftItem[0],pItem,sizeof(sITEM));
				sCraftItem.CraftItem[0].x = pItem->SetX;
				sCraftItem.CraftItem[0].y = pItem->SetY;
				pItem->Flag = 0; //초기화 
				sinPlaySound(sCraftItem.CraftItem[0].SoundIndex);
				ReFormCraftItem();
				return TRUE;
			}

		}
	}		

	if(Kind == 1){	//아이템 에이징 
		//CheckCraftItemForm(); //조작하는 상놈의 자식들을 죽여버리자 퉤!!
		if((pItem->CODE & sinITEM_MASK2)  == sinOS1){ //쉘텀일 경우 
			for(int j=1 ; j < 15 ; j++){
				if(!sAgingItem.AgingItem[j].Flag){
					memcpy(&sAgingItem.AgingItem[j],pItem,sizeof(sITEM));
					sAgingItem.AgingItem[j].x = pItem->SetX;
					sAgingItem.AgingItem[j].y = pItem->SetY;
					pItem->Flag = 0; //초기화 
					sinPlaySound(sAgingItem.AgingItem[j].SoundIndex);
		//			ReFormCraftItem();
					return TRUE;
				}
			}
		}
		else{
			if(!sAgingItem.AgingItem[0].Flag){
				memcpy(&sAgingItem.AgingItem[0],pItem,sizeof(sITEM));
				sAgingItem.AgingItem[0].x = pItem->SetX;
				sAgingItem.AgingItem[0].y = pItem->SetY;
				pItem->Flag = 0; //초기화 
				sinPlaySound(sAgingItem.AgingItem[0].SoundIndex);
		//		ReFormCraftItem();
				return TRUE;
			}

		}

	}
	if(Kind == 2){	//아이템 Make
		for(int j=0 ; j < 15 ; j++){
			if(!sAgingItem.AgingItem[j].Flag){
				memcpy(&sAgingItem.AgingItem[j],pItem,sizeof(sITEM));
				sAgingItem.AgingItem[j].x = pItem->SetX;
				sAgingItem.AgingItem[j].y = pItem->SetY;
				pItem->Flag = 0; //초기화 
				sinPlaySound(sAgingItem.AgingItem[j].SoundIndex);
				return TRUE;
			}
		}
	}
	if(Kind == 3){  //에이징 스톤 // 코퍼 오어 // 박재원 - 엘더 코퍼 오어, 슈퍼 에이징 스톤   // 장별 - 슈퍼 에이징 스톤 1.5
		if(!sAgingItem.AgingItem[14].Flag){
				memcpy(&sAgingItem.AgingItem[14],pItem,sizeof(sITEM));
				sAgingItem.AgingItem[14].x = pItem->SetX;
				sAgingItem.AgingItem[14].y = pItem->SetY;
				pItem->Flag = 0; //초기화 
				sinPlaySound(sAgingItem.AgingItem[14].SoundIndex);
				return TRUE;
		}
	}

	return TRUE;

}

//아이템을 선택한다 
int cCRAFTITEM::PickUpCraftItem(int x , int y , int PickUpFlag,int Kind)
{
	
	int i=0;
	SelectTradeItemIndex = 0;    //집을 아이템 
	if(Kind == 0){
		for(i=0 ; i < 15 ; i++){
			if(sCraftItem.CraftItem[i].Flag){
				if ( sCraftItem.CraftItem[i].x < x  && sCraftItem.CraftItem[i].x + sCraftItem.CraftItem[i].w > x  && 
					sCraftItem.CraftItem[i].y < y  && sCraftItem.CraftItem[i].y + sCraftItem.CraftItem[i].h > y ){
					if(PickUpFlag){
						//쉘텀 조합 상태에서 에이징을 캔슬하지 못한다.======================================================
						int cnt = 0;
						for( cnt = 1 ; cnt < 15 ; cnt++){
					 		if(i==0 &&  sCraftItem.CraftItem[cnt].Flag){
								cMessageBox.ShowMessage(MESSAGE_NOT_AGINGPICKUP); //  
								cnt = 100;
								break;
							}
						}
						if( cnt == 100) break;
						//=================================================================================================	
						CheckCraftItemForm();
						memcpy(&MouseItem , &sCraftItem.CraftItem[i] , sizeof(sITEM));
						sCraftItem.CraftItem[i].Flag = 0;
						haCraftSortItem(&sCraftItem);  //아이템 재구성 때문에 배열 이동
						//memset(&sCraftItem.CraftItem[i],0,sizeof(sITEM));  //서버 해킹 에러 때문에 임시 방편이다.

						sinPlaySound(sCraftItem.CraftItem[i].SoundIndex);
						TradeColorIndex = 0;
						ReFormCraftItem();
						if(ForceFlag)
							CheckForceOrbPrice(); // Forece Orb 플랙있을경우엔 포스가격을 표시

						if(i==0){  //씰아이템 때문에 
							for(int i=1;i<15;i++){
								if(sCraftItem.CraftItem[i].Flag){
									if((sCraftItem.CraftItem[i].CODE & sinITEM_MASK2)  == sinSE1){
										if(!cInvenTory.AutoSetInvenItem(&sCraftItem.CraftItem[i])){
											sinThrowItemToFeild(&sCraftItem.CraftItem[i]);
											sCraftItem.CraftItem[i].Flag = 0;
											memset(&sCraftItem.CraftItem[i],0,sizeof(sCRAFTITEM)); //아이템을 다 날려버린다 
										}
									}
								}
							}   
						}
						return TRUE;
					}
					else{
						SelectTradeItemIndex = i+1;
						TradeColorIndex = SELECT_ITEM_COLOR;
						sinShowItemInfoFlag = 1; //아이템의 정보를 보여준다 
						cItem.ShowItemInfo(&sCraftItem.CraftItem[i] , 2);
					}
				}
			}
		}
	}
	if(Kind == 1){ 
		for(i=0 ; i < 15 ; i++){
			if(sAgingItem.AgingItem[i].Flag){
				if ( sAgingItem.AgingItem[i].x < x  && sAgingItem.AgingItem[i].x + sAgingItem.AgingItem[i].w > x  && 
					sAgingItem.AgingItem[i].y < y  && sAgingItem.AgingItem[i].y + sAgingItem.AgingItem[i].h > y ){
					if(PickUpFlag){
						//쉘텀 조합 상태에서 에이징을 캔슬하지 못한다.======================================================
						int cnt = 0;
						for( cnt = 1 ; cnt < 15 ; cnt++){
					 		if(i==0 && sAgingItem.AgingItem[cnt].Flag){
								cMessageBox.ShowMessage(MESSAGE_NOT_AGINGPICKUP); //  
								cnt = 100;
								break;
							}
						}
						if( cnt == 100) break;
						//===================================================================================================

						memcpy(&MouseItem , &sAgingItem.AgingItem[i] , sizeof(sITEM));
						sAgingItem.AgingItem[i].Flag = 0;
						sinPlaySound(sAgingItem.AgingItem[i].SoundIndex);
						//에이징 스톤 // 코퍼 오어
						if( sAgingItem.AgingItem[i].sItemInfo.CODE == (sinBI1|sin10) || 
							sAgingItem.AgingItem[i].sItemInfo.CODE == (sinBI1|sin11) ||
							// 박재원 - 엘더 코퍼 오어, 슈퍼 에이징 스톤 추가
							// 장별 - 슈퍼 에이징 스톤 1.5
							sAgingItem.AgingItem[i].sItemInfo.CODE == (sinBI1|sin60) ||
							sAgingItem.AgingItem[i].sItemInfo.CODE == (sinBI1|sin61) ||
							sAgingItem.AgingItem[i].sItemInfo.CODE == (sinBI1|sin85) ) break;
						TradeColorIndex = 0;
						//Make Item   (플렉없이 그냥 KindCode만 체크한다 )플렉은 위에서 지웠다 -_-;
						if((MouseItem.sItemInfo.CODE & sinITEM_MASK2) == sinMA1 || 
							(MouseItem.sItemInfo.CODE & sinITEM_MASK2) == sinMA2 ){
							sinMakeItemCheck();

						}
						else{
							if(i == 0){
								for(int y=0;y<4;y++){ //쉘텀 인덱스 초기화 
									for(int h= 0 ;h < 3  ;h ++){
										AgingLevelSheltomIndex[y][h] = 0;
										AgingCheckSheltomFlag = 0;
										AgingSheltomCnt2 = 0;
										cAging.CancelAgingItem(1);
									}
								}
							}
							else{
								if(AgingSheltomCnt2){
									AgingSheltomCnt2--;
									AgingCheckSheltomFlag = 0;
								}
							}
						}
						return TRUE;
					}
					else{
						SelectTradeItemIndex = i+1;
						TradeColorIndex = SELECT_ITEM_COLOR;
						sinShowItemInfoFlag = 1; //아이템의 정보를 보여준다 
						cItem.ShowItemInfo(&sAgingItem.AgingItem[i] , 2);
					}
				}
			}
		}
	}
	return FALSE;
}

DWORD SheltomCode[9] = {sin01,sin02,sin03,sin04,sin05,sin06,sin07,sin08,sin09};
//아이템을 조합한다 
int cCRAFTITEM::MixingItem()
{
 	int i,j,t,k,CheckFlag = 0 , Temp=0 , LineCount = 0;
	DWORD TempCode2 = 0;
	int  ReConCnt = 0;
	if(sCraftItem_Recv.DocIndex == 2000){ //아이템 재구성
		for(i=0;i<12;i++){                 
			if(sCraftItem_Recv.SheltomCode[i]){
				TempCode2 = sCraftItem_Recv.SheltomCode[i];
				break;
			}
		}
		for(j = 0 ; j < MAX_SEEL_COUNT ; j++){ // 박재원 - 테이와즈 씰 추가 (3종류 -> 4종류)
			if(sReconItem[j].dwCODE == TempCode2){
				ReConCnt = sReconItem[j].iCount;
			}
		}
		if(!ReConCnt){  //코드가 틀리면 실패
			sCraftItem_Recv.Result = FALSE;
			return FALSE;
		}
		for(i = 0 ; i < ReConCnt ; i++){ //갯수가 안맞으면 실패 
			if(sCraftItem_Recv.SheltomCode[i]){
				if(TempCode2 != sCraftItem_Recv.SheltomCode[i]){
					sCraftItem_Recv.Result = FALSE;
					return FALSE;
				}
			}
		}
		sCraftItem_Recv.Result = TRUE;
		return 2000;
	}

	//씰아이템 아이템 믹스쳐 방지
	for(j = 0; j < 12 ; j++){
		if((sCraftItem_Recv.SheltomCode[j] & sinITEM_MASK2)==sinSE1){
			sCraftItem_Recv.Result = FALSE;
			return FALSE;
		}
	}


	//sITEM TempItem;

	for(j = 0; j < 12 ; j++) //초기화 
		sCraftItem.SheltomIndex[j] = 0;


	/////////////쉘텀을 검색한다 
	for(i = 0 ; i < 12 ; i++){ //1부터는 쉘텀
		if(sCraftItem_Recv.SheltomCode[i]&sinITEM_MASK2){
			for(j = 0; j < 9 ; j++){
				if((sCraftItem_Recv.SheltomCode[i] & sinITEM_MASK3) == SheltomCode[j]){
					sCraftItem.SheltomIndex[j]++;
					break;
				}
			}
		}
	}



	for(i = 0 ; i < MAX_CRAFTITEM_INFO ; i++){
		for(j = 0;j < 8 ; j++){ //무기 코드 검사 
			if(sCraftItem_Recv.DesCraftItem.Flag){ 
				//방패와 갑옷도 이젠 믹스가된다 +_+ 띠요요요요용
				/*
				if(!smConfig.DebugMode){ 
					if((sCraftItem_Recv.DesCraftItem.CODE & sinITEM_MASK2) == sinDA1 ||
						(sCraftItem_Recv.DesCraftItem.CODE & sinITEM_MASK2) == sinDS1 )
						break;

				}
				*/

				if((sCraftItem_Info[i].MainItemCode[j] & sinITEM_MASK2) ==
					(sCraftItem_Recv.DesCraftItem.sItemInfo.CODE & sinITEM_MASK2)){ //조합할수있는 무기이면 
					for(t = 0 ; t < 9 ; t++ ){ //쉘텀의 종류와 갯수를 체크한다 
						if(sCraftItem_Info[i].AddItem[t] != sCraftItem.SheltomIndex[t]){
							CheckFlag = 1;
							break;
						}
						CheckFlag = 0;

					}

					////검색 조건에 맞으면 
					if(!CheckFlag){
						/*
						if(!CheckItemForm(&sCraftItem_Recv.DesCraftItem.sItemInfo)){ //아이템 인증받기 
							SendSetHackUser(7); //해킹을 하려고했던 못된 유저를 고발 TRUE 접속 종료 
							return FALSE;

						}
						*/
						for(k = 0 ; k < 9 ; k++ ){ //능력치를 더한다 
							//사용된 최상위쉘텀을 확인한다
							if(sCraftItem.SheltomIndex[k])
								sCraftItem_Recv.DesCraftItem.sItemInfo.ItemAgingNum[0] = k+1;

							if(sCraftItem_Info[i].PlusElementCode[k]){ //플러스될 수치가 있을경우에
								switch(sCraftItem_Info[i].PlusElementCode[k]){
								case SIN_ADD_FIRE:
									sCraftItem_Recv.DesCraftItem.sItemInfo.Resistance[sITEMINFO_FIRE] += (int)sCraftItem_Info[i].AddElement[k];
									sCraftItem_Recv.DesCraftItem.sItemInfo.ItemKindMask |= SIN_ADD_FIRE;

								break;
								case SIN_ADD_ICE:
									sCraftItem_Recv.DesCraftItem.sItemInfo.Resistance[sITEMINFO_ICE] += (int)sCraftItem_Info[i].AddElement[k];
									sCraftItem_Recv.DesCraftItem.sItemInfo.ItemKindMask |= SIN_ADD_ICE;
								break;
								case SIN_ADD_LIGHTNING:
									sCraftItem_Recv.DesCraftItem.sItemInfo.Resistance[sITEMINFO_LIGHTING] += (int)sCraftItem_Info[i].AddElement[k];
									sCraftItem_Recv.DesCraftItem.sItemInfo.ItemKindMask |= SIN_ADD_LIGHTNING;
								break;
										
								case SIN_ADD_POISON:
									sCraftItem_Recv.DesCraftItem.sItemInfo.Resistance[sITEMINFO_POISON] += (int)sCraftItem_Info[i].AddElement[k];
									sCraftItem_Recv.DesCraftItem.sItemInfo.ItemKindMask |= SIN_ADD_POISON;
								break;
								case SIN_ADD_BIO:
									sCraftItem_Recv.DesCraftItem.sItemInfo.Resistance[sITEMINFO_BIONIC] += (int)sCraftItem_Info[i].AddElement[k];
									sCraftItem_Recv.DesCraftItem.sItemInfo.ItemKindMask |= SIN_ADD_BIO;
								break;
								case SIN_ADD_CRITICAL:		//크리티컬 
									if(sCraftItem_Info[i].ElementKind[k] ==  SIN_ADD_NUM)
										sCraftItem_Recv.DesCraftItem.sItemInfo.Critical_Hit += (int)sCraftItem_Info[i].AddElement[k];
									else
									sCraftItem_Recv.DesCraftItem.sItemInfo.Critical_Hit +=  (sCraftItem_Recv.DesCraftItem.sItemInfo.																							Critical_Hit*((int)sCraftItem_Info[i].AddElement[k])) / 100;
									//sCraftItem_Recv.DesCraftItem.sItemInfo.Critical_Hit +=  (int)((float)sCraftItem_Recv.DesCraftItem.sItemInfo.Critical_Hit*(sCraftItem_Info[i].AddElement									[k] / 100));
									sCraftItem_Recv.DesCraftItem.sItemInfo.ItemKindMask |= SIN_ADD_CRITICAL;
								break;
										case SIN_ADD_ATTACK_RATE:	//명중력 
									if(sCraftItem_Info[i].ElementKind[k] ==  SIN_ADD_NUM)
										sCraftItem_Recv.DesCraftItem.sItemInfo.Attack_Rating += (int)sCraftItem_Info[i].AddElement[k];
									else
										sCraftItem_Recv.DesCraftItem.sItemInfo.Attack_Rating +=  (sCraftItem_Recv.DesCraftItem.sItemInfo.Attack_Rating*((int)sCraftItem_Info[i].AddElement											[k]))/100;
										//sCraftItem_Recv.DesCraftItem.sItemInfo.Attack_Rating +=  (int)((float)sCraftItem_Recv.DesCraftItem.sItemInfo.Attack_Rating*(sCraftItem_Info[i].											AddElement[k]/100));
									sCraftItem_Recv.DesCraftItem.sItemInfo.ItemKindMask |= SIN_ADD_ATTACK_RATE;
								break;
								case SIN_ADD_DAMAGE_MIN:	//최소대미지
									if(sCraftItem_Info[i].ElementKind[k] ==  SIN_ADD_NUM)
										sCraftItem_Recv.DesCraftItem.sItemInfo.Damage[0] += (int)sCraftItem_Info[i].AddElement[k];
									else
										sCraftItem_Recv.DesCraftItem.sItemInfo.Damage[0] +=  (sCraftItem_Recv.DesCraftItem.sItemInfo.Damage[0]*((int)sCraftItem_Info[i].AddElement[k]))/100;
										//sCraftItem_Recv.DesCraftItem.sItemInfo.Damage[0] +=  (int)((float)sCraftItem_Recv.DesCraftItem.sItemInfo.Damage[0]*(sCraftItem_Info[i].AddElement[k]/100));
									sCraftItem_Recv.DesCraftItem.sItemInfo.ItemKindMask |= SIN_ADD_DAMAGE_MIN;
								break;

								case SIN_ADD_DAMAGE_MAX:	//최대대미지
									if(sCraftItem_Info[i].ElementKind[k] ==  SIN_ADD_NUM)
										sCraftItem_Recv.DesCraftItem.sItemInfo.Damage[1] += (int)sCraftItem_Info[i].AddElement[k];
									else
										sCraftItem_Recv.DesCraftItem.sItemInfo.Damage[1] +=  (sCraftItem_Recv.DesCraftItem.sItemInfo.Damage[1]*((int)sCraftItem_Info[i].AddElement[k]))/100;
										//sCraftItem_Recv.DesCraftItem.sItemInfo.Damage[1] +=  (int)((float)sCraftItem_Recv.DesCraftItem.sItemInfo.Damage[1]*(sCraftItem_Info[i].AddElement[k]/100));
									sCraftItem_Recv.DesCraftItem.sItemInfo.ItemKindMask |= SIN_ADD_DAMAGE_MAX;
								break;
								case SIN_ADD_ATTACK_SPEED:	//공격속도
									if(sCraftItem_Info[i].ElementKind[k] ==  SIN_ADD_NUM)
										sCraftItem_Recv.DesCraftItem.sItemInfo.Attack_Speed += (int)sCraftItem_Info[i].AddElement[k];
									else
										sCraftItem_Recv.DesCraftItem.sItemInfo.Attack_Speed +=  (sCraftItem_Recv.DesCraftItem.sItemInfo.Attack_Speed*((int)sCraftItem_Info[i].AddElement[k]))/100;
										//sCraftItem_Recv.DesCraftItem.sItemInfo.Attack_Speed += (int)((float)sCraftItem_Recv.DesCraftItem.sItemInfo.Attack_Speed*(sCraftItem_Info[i].AddElement[k]/											100));
									sCraftItem_Recv.DesCraftItem.sItemInfo.ItemKindMask |= SIN_ADD_ATTACK_SPEED;
								break;
								case SIN_ADD_ABSORB:	//흡수력 
									if(sCraftItem_Info[i].ElementKind[k] ==  SIN_ADD_NUM)
										sCraftItem_Recv.DesCraftItem.sItemInfo.fAbsorb += sCraftItem_Info[i].AddElement[k];
									else
										sCraftItem_Recv.DesCraftItem.sItemInfo.fAbsorb += (GetItemAbsorb(&sCraftItem_Recv.DesCraftItem.sItemInfo)*(sCraftItem_Info[i].AddElement[k]/100));
									sCraftItem_Recv.DesCraftItem.sItemInfo.ItemKindMask |= SIN_ADD_ABSORB;
								break;
								case SIN_ADD_DEFENCE:	//방어력 
									if(sCraftItem_Info[i].ElementKind[k] ==  SIN_ADD_NUM)
										sCraftItem_Recv.DesCraftItem.sItemInfo.Defence += (int)sCraftItem_Info[i].AddElement[k];
									else
										sCraftItem_Recv.DesCraftItem.sItemInfo.Defence += (sCraftItem_Recv.DesCraftItem.sItemInfo.Defence*((int)sCraftItem_Info[i].AddElement[k]))/100;
										//sCraftItem_Recv.DesCraftItem.sItemInfo.Defence += (int)((float)sCraftItem_Recv.DesCraftItem.sItemInfo.Defence*(sCraftItem_Info[i].AddElement[k]/100));
									sCraftItem_Recv.DesCraftItem.sItemInfo.ItemKindMask |= SIN_ADD_DEFENCE;
								break;
								case SIN_ADD_BLOCK_RATE: //블럭율 
									if(sCraftItem_Info[i].ElementKind[k] ==  SIN_ADD_NUM)
										sCraftItem_Recv.DesCraftItem.sItemInfo.fBlock_Rating += sCraftItem_Info[i].AddElement[k];
									else
										sCraftItem_Recv.DesCraftItem.sItemInfo.fBlock_Rating += (sCraftItem_Recv.DesCraftItem.sItemInfo.fBlock_Rating*(sCraftItem_Info[i].AddElement[k]/100));
									sCraftItem_Recv.DesCraftItem.sItemInfo.ItemKindMask |= SIN_ADD_BLOCK_RATE;
								break;
								case SIN_ADD_MOVE_SPEED: //이동속도 
									if(sCraftItem_Info[i].ElementKind[k] ==  SIN_ADD_NUM)
										sCraftItem_Recv.DesCraftItem.sItemInfo.fSpeed += sCraftItem_Info[i].AddElement[k];
									else
										sCraftItem_Recv.DesCraftItem.sItemInfo.fSpeed += (sCraftItem_Recv.DesCraftItem.sItemInfo.fSpeed*(sCraftItem_Info[i].AddElement[k]/100));								
									sCraftItem_Recv.DesCraftItem.sItemInfo.ItemKindMask |= SIN_ADD_MOVE_SPEED;
								break;

								case SIN_ADD_LIFE: //최대 생명력 
									if(sCraftItem_Info[i].ElementKind[k] ==  SIN_ADD_NUM)
										sCraftItem_Recv.DesCraftItem.sItemInfo.fIncrease_Life += sCraftItem_Info[i].AddElement[k];
									else
										sCraftItem_Recv.DesCraftItem.sItemInfo.fIncrease_Life += (sCraftItem_Recv.DesCraftItem.sItemInfo.fIncrease_Life*(sCraftItem_Info[i].AddElement[k]/100));								
									sCraftItem_Recv.DesCraftItem.sItemInfo.ItemKindMask |= SIN_ADD_LIFE;
								break;

								case SIN_ADD_MANA: //최대 기력
									if(sCraftItem_Info[i].ElementKind[k] ==  SIN_ADD_NUM)
										sCraftItem_Recv.DesCraftItem.sItemInfo.fIncrease_Mana += sCraftItem_Info[i].AddElement[k];
									else
										sCraftItem_Recv.DesCraftItem.sItemInfo.fIncrease_Mana += (sCraftItem_Recv.DesCraftItem.sItemInfo.fIncrease_Mana*(sCraftItem_Info[i].AddElement[k]/100));								
									sCraftItem_Recv.DesCraftItem.sItemInfo.ItemKindMask |= SIN_ADD_MANA;
								break;
								case SIN_ADD_STAMINA: //최대 근력
									if(sCraftItem_Info[i].ElementKind[k] ==  SIN_ADD_NUM)
										sCraftItem_Recv.DesCraftItem.sItemInfo.fIncrease_Stamina += sCraftItem_Info[i].AddElement[k];
									else
										sCraftItem_Recv.DesCraftItem.sItemInfo.fIncrease_Stamina += (sCraftItem_Recv.DesCraftItem.sItemInfo.fIncrease_Stamina*(sCraftItem_Info[i].AddElement[k]/100));								
									sCraftItem_Recv.DesCraftItem.sItemInfo.ItemKindMask |= SIN_ADD_STAMINA;
								break;
								case SIN_ADD_LIFEREGEN: //생명력재생
									if(sCraftItem_Info[i].ElementKind[k] ==  SIN_ADD_NUM)
										sCraftItem_Recv.DesCraftItem.sItemInfo.fLife_Regen += sCraftItem_Info[i].AddElement[k];
									else
										sCraftItem_Recv.DesCraftItem.sItemInfo.fLife_Regen +=(sCraftItem_Recv.DesCraftItem.sItemInfo.fLife_Regen*(sCraftItem_Info[i].AddElement[k]/100));
									sCraftItem_Recv.DesCraftItem.sItemInfo.ItemKindMask |= SIN_ADD_LIFEREGEN;
								break;
								case SIN_ADD_MANAREGEN: //기력재생
									if(sCraftItem_Info[i].ElementKind[k] ==  SIN_ADD_NUM)
										sCraftItem_Recv.DesCraftItem.sItemInfo.fMana_Regen += sCraftItem_Info[i].AddElement[k];
									else
										sCraftItem_Recv.DesCraftItem.sItemInfo.fMana_Regen +=(sCraftItem_Recv.DesCraftItem.sItemInfo.fMana_Regen*(sCraftItem_Info[i].AddElement[k]/100));
									sCraftItem_Recv.DesCraftItem.sItemInfo.ItemKindMask |= SIN_ADD_MANAREGEN;

								break;
								case SIN_ADD_STAMINAREGEN: //근력재생
									if(sCraftItem_Info[i].ElementKind[k] ==  SIN_ADD_NUM)
										sCraftItem_Recv.DesCraftItem.sItemInfo.fStamina_Regen += sCraftItem_Info[i].AddElement[k];
									else
										sCraftItem_Recv.DesCraftItem.sItemInfo.fStamina_Regen +=(sCraftItem_Recv.DesCraftItem.sItemInfo.fStamina_Regen*(sCraftItem_Info[i].AddElement[k]/100));
									sCraftItem_Recv.DesCraftItem.sItemInfo.ItemKindMask |= SIN_ADD_STAMINAREGEN;

								break;

								}
							}
						}
						
						////////수치적용 끝 
						ReformItem( &sCraftItem_Recv.DesCraftItem.sItemInfo);  //아이템 인증 받기 
						
						//아이템 인증 그 두번째 
						if(!CheckMixItem( &sCraftItem_Recv.DesCraftItem.sItemInfo )){
							SendSetHackUser(50); //해킹을 하려고했던 못된 유저를 고발 TRUE 접속 종료 
							return FALSE;
						}
						

						sCraftItem_Recv.DesCraftItem.sItemInfo.ItemKindCode = ITEM_KIND_CRAFT;
						ReformMixItem( &sCraftItem_Recv.DesCraftItem.sItemInfo  ); //재인증 

						sCraftItem_Recv.DocIndex = i;
						sCraftItem_Recv.Result = TRUE;

						//믹스쳐의 결과를 서버로 보낸다 
						//sinSendCraftItemResult(&sCraftItem_Recv.DesCraftItem , i, TRUE);

						/*
						ShowItemCraftMessageFlag = 1; //아이템이 조합됬을때 메세지를 보여준다 
						lstrcpy(szCraftItemBuff,sCraftItem_Info[i].Doc); //아이템 정보를 보여준다 
						
						Temp = lstrlen(szCraftItemBuff);
						for( i = 0 ; i < Temp ; i++){
							if(szCraftItemBuff[i] == '\r'){
								LineCount++;
							}
						}

						CraftItemMessageSize.x = 14;           //박스 사이즈 
						CraftItemMessageSize.y = ((LineCount)*2)+2;  //박스 사이즈 

						
						sinAgingSuccessFlag2 = 1;

						CheckCharForm(); //인증 
						//sinChar->Money -= sCraftItem_Recv.DesCraftItem.sItemInfo.Price;
						sinMinusMoney(sCraftItem_Recv.DesCraftItem.sItemInfo.Price);
						ReformCharForm(); //재인증 
						SendSaveMoney(); //금액 조작을 못하게하기위해 호출한다 

						
						//실패든 성공이든간에 아이템을 원상 복귀 시킨다 
						cCraftItem.OpenFlag = 0; //창을 닫는다
						cInvenTory.AutoSetInvenItem(&sCraftItem_Recv.DesCraftItem); //아이템을 셋팅한다 
						for(int y=1;y<15;y++){ //날라간 쉘텀을 서버에 알려준다 
							if(sCraftItem.CraftItem[y].Flag)
								SendSaveThrowItem(&sCraftItem.CraftItem[y].sItemInfo);

						}
						memset(&sCraftItem,0,sizeof(sCRAFTITEM)); //구조체 초기화 
						//믹스쳐 할때 돈을 뺀다 
						
						*/	
						return TRUE;

					}
				}
			}
		}
	}

	
	sCraftItem_Recv.Result = FALSE;

	//sinSendCraftItemResult(&sCraftItem_Recv.DesCraftItem , 0, FALSE);

	/*

	//믹스쳐 할때 돈을 뺀다 
	CheckCharForm(); //인증 
	//sinChar->Money -= sCraftItem_Recv.DesCraftItem.sItemInfo.Price;
	sinMinusMoney(sCraftItem_Recv.DesCraftItem.sItemInfo.Price);
	ReformCharForm(); //재인증 
	SendSaveMoney(); //금액 조작을 못하게하기위해 호출한다 

	//memcpy(&TempItem,&sCraftItem_Recv.DesCraftItem,sizeof(sITEM));
	cInvenTory.AutoSetInvenItem(&sCraftItem_Recv.DesCraftItem); //아이템을 셋팅한다 
	for(int y=1;y<15;y++){ //날라간 쉘텀을 서버에 알려준다 
		if(sCraftItem.CraftItem[y].Flag)
			SendSaveThrowItem(&sCraftItem.CraftItem[y].sItemInfo);

	}
	memset(&sCraftItem,0,sizeof(sCRAFTITEM)); //구조체 초기화 
	cCraftItem.OpenFlag = 0; //창을 닫는다 
	cMessageBox.ShowMessage(MESSAGE_FAILD_CRAFT_ITEM);
	sinPlaySound(SIN_SOUND_FAILD_MIXITEM);
	//memset(&sCraftItem_Recv.DesCraftItem,&TempItem,sizeof(sITEM)); //아이템 복구 

	*/


	return FALSE;
}

//아이템이 들어갈수있는지 체크한다 
int cCRAFTITEM::CraftCheckEmptyArea(sITEM *pItem)
{
	int cntx,cnty;
	int cx,cy;

	RECT	rect;
	int		flag;
	int StartX = 23;  //인벤 박스의 시작점 X
	int StartY = 436; // Y

	cx = (22*INVENTORY_BOX_ROW)-pItem->w; //아이템이 들어갈수있는 가장 끝 X좌표 
	cy = (22*INVENTORY_BOX_COL)-pItem->h; //아이템이 들어갈수있는 가장 끝 Y좌표

	//코드가 같지 않거나 아이템이 비어있지 않은 곳에 셋팅할때 검색해서 위치를 찾아낸다 
	for( cntx=StartX ; cntx <= StartX+cx ; cntx+=22){
		for( cnty=StartY ; cnty <= StartY+cy ; cnty+=22){ 
			flag = 0;
			for(int i=0 ; i<INVENTORY_MAXITEM ; i++){
				if(cInvenTory.InvenItem[i].Flag){
					rect.left   = cInvenTory.InvenItem[i].x;
					rect.right  = cInvenTory.InvenItem[i].x+cInvenTory.InvenItem[i].w;
					rect.top    = cInvenTory.InvenItem[i].y;
					rect.bottom = cInvenTory.InvenItem[i].y+cInvenTory.InvenItem[i].h;

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

//믹스쳐 아이템을 취소한다 
int cCRAFTITEM::CancelMixItem()
{
	if(MixItemNoCopyFlag)return FALSE; 
	for(int i=0;i<15;i++){
		if(sCraftItem.CraftItem[i].Flag){
			if(!cInvenTory.AutoSetInvenItem(&sCraftItem.CraftItem[i])){
				sinThrowItemToFeild(&sCraftItem.CraftItem[i]);
				sCraftItem.CraftItem[i].Flag = 0;
			}
		}
	}

	cCraftItem.OpenFlag = 0 ; //창을 닫는다 
	memset(&sCraftItem,0,sizeof(sCRAFTITEM)); //아이템을 다 날려버린다 
	ResetInvenItemCode();				//이벤 아이템 코드 초기화

	memset(&sCraftItem_Recv,0,sizeof(sCRAFTITEM_SERVER)); //받은 믹스쳐 구조체 초기화  (일단 같이해준다)
	memset(&sCraftItem_Send,0,sizeof(sCRAFTITEM_SERVER)); //보낸 믹스쳐 구조체 초기화 

	//Force Orb 초기화
	ForceFlag = 0; 
	ForceItemPrice = 0;
	ForceItemPrice2 = 0;
	return TRUE;
}

int CraftItemCheckDelayTime = 0;

//크래프트 아이템을 조작하는 망할 유저를 병신 만든다 
int cCRAFTITEM::CheckHackCraftItem()
{
	//현제 서버에서 하도록 되어있어 적용이 안된다 
	CraftItemCheckDelayTime++;
	if(CraftItemCheckDelayTime < 70*10)return FALSE;
	CraftItemCheckDelayTime = 0;


	unsigned int AddItemCheckSum = 0;
	unsigned int CheckAddItemData = 2774094026;

	for(int i=0 ;i < MAX_CRAFTITEM_INFO ; i++){
		for(int j=0; j < 8 ;j++){
			AddItemCheckSum += (int)((j+1)*sCraftItem_Info[i].AddElement[j]);
			AddItemCheckSum += (j+1)*sCraftItem_Info[i].AddItem[j];
			AddItemCheckSum += (j+1)*sCraftItem_Info[i].ElementKind[j];
			AddItemCheckSum += (j+1)*sCraftItem_Info[i].PlusElementCode[j];
			AddItemCheckSum += (j+1)*sCraftItem_Info[i].MainItemCode[j];

		}

	}
	if( CheckAddItemData != AddItemCheckSum){
		SendSetHackUser(8); //해킹을 하려고했던 못된 유저를 고발 TRUE 접속 종료 

	}
	 
	return TRUE;
}

//////////믹스쳐 아이템 조작 방지를 위한 힘겨운 노력 
int cCRAFTITEM::CheckCraftItemForm()
{ //체크  
	int TempCheckDataSum = 0;

	if(!cCraftItem.ForceFlag){
		for(int i=0;i<15;i++){
			if(sCraftItem.CraftItem[i].Flag){
				TempCheckDataSum += (i+1)*sCraftItem.CraftItem[i].x;
				TempCheckDataSum += (i+1)*sCraftItem.CraftItem[i].y;
				TempCheckDataSum += sCraftItem.CraftItem[i].sItemInfo.ItemHeader.dwChkSum;
				TempCheckDataSum += sCraftItem.CraftItem[i].sItemInfo.ItemHeader.Head;

			}
		}
		//if(TempCheckDataSum != CraftItemCheckSum) //필요없느거 같서리 뺏다 하핫
			//SendSetHackUser(9); //해킹을 하려고했던 못된 유저를 고발 TRUE 접속 종료 
	}
	return TRUE;

}

int cCRAFTITEM::ReFormCraftItem()
{ //인증 

	CraftItemCheckSum = 0;
	for(int i=0;i<15;i++){
		if(sCraftItem.CraftItem[i].Flag){
			CraftItemCheckSum += (i+1)*sCraftItem.CraftItem[i].x;
			CraftItemCheckSum += (i+1)*sCraftItem.CraftItem[i].y;
			CraftItemCheckSum += sCraftItem.CraftItem[i].sItemInfo.ItemHeader.dwChkSum;
			CraftItemCheckSum += sCraftItem.CraftItem[i].sItemInfo.ItemHeader.Head;

		}
	}

	return TRUE;
}

//////////트레이드  아이템 조작 방지를 위한 힘겨운 노력 헥헥헥
//ReFormTradeItem();
//CheckTradeItemForm();
int cTRADE::CheckTradeItemForm()
{ //체크  
	int TempCheckDataSum = 0;
	for(int i=0 ; i<MAX_TRADE_ITEM ; i++){
		if(sTrade.TradeItem[i].Flag){
			TempCheckDataSum += (i+1)*sTrade.TradeItem[i].x;
			TempCheckDataSum += (i+1)*sTrade.TradeItem[i].y;
			TempCheckDataSum += sTrade.TradeItem[i].sItemInfo.ItemHeader.dwChkSum;
			TempCheckDataSum += sTrade.TradeItem[i].sItemInfo.ItemHeader.Head;

		}
	}
	if(TempCheckDataSum != TradeItemCheckSum)
		SendSetHackUser(10); //해킹을 하려고했던 못된 유저를 고발 TRUE 접속 종료 

	return TRUE;

}

int cTRADE::ReFormTradeItem()
{ //인증 

	TradeItemCheckSum = 0;
	for(int i=0 ; i<MAX_TRADE_ITEM ; i++){
		if(sTrade.TradeItem[i].Flag){
			TradeItemCheckSum += (i+1)*sTrade.TradeItem[i].x;
			TradeItemCheckSum += (i+1)*sTrade.TradeItem[i].y;
			TradeItemCheckSum += sTrade.TradeItem[i].sItemInfo.ItemHeader.dwChkSum;
			TradeItemCheckSum += sTrade.TradeItem[i].sItemInfo.ItemHeader.Head;

		}
	}
	return TRUE;
}

//CheckWareHouseForm();
//ReFormWareHouse();
////////////////////창고를 조작하려는 나쁜 자식들을 벌주기 위한 힘겨운 노력 !!!
int cWAREHOUSE::CheckWareHouseForm()
{ //체크  
	int TempCheckDataSum = 0;
	for(int i=0 ; i<100 ; i++){
		if(sWareHouse.WareHouseItem[i].Flag){
			TempCheckDataSum += (i+1)*sWareHouse.WareHouseItem[i].x;
			TempCheckDataSum += (i+1)*sWareHouse.WareHouseItem[i].y;
			TempCheckDataSum += sWareHouse.WareHouseItem[i].sItemInfo.ItemHeader.dwChkSum;
			TempCheckDataSum += sWareHouse.WareHouseItem[i].sItemInfo.ItemHeader.Head;

		}
	}
	if(TempCheckDataSum != WareHouseCheckSum)
		SendSetHackUser(11); //해킹을 하려고했던 못된 유저를 고발 TRUE 접속 종료 

	return TRUE;

}

int cWAREHOUSE::ReFormWareHouse()
{ //인증 

	WareHouseCheckSum = 0;
	for(int i=0 ; i<100 ; i++){
		if(sWareHouse.WareHouseItem[i].Flag){
			WareHouseCheckSum += (i+1)*sWareHouse.WareHouseItem[i].x;
			WareHouseCheckSum += (i+1)*sWareHouse.WareHouseItem[i].y;
			WareHouseCheckSum += sWareHouse.WareHouseItem[i].sItemInfo.ItemHeader.dwChkSum;
			WareHouseCheckSum += sWareHouse.WareHouseItem[i].sItemInfo.ItemHeader.Head;

		}
	}
	return TRUE;
}

////////////////////////////////////// 에이징

//아이템을 에이징한다 
int cAGING::AgingItem(int MakeItemFlag)
{
	
	int ResultCount =0;
	int ResultCountCheck =0;
	DWORD TempCODE[16] = {0,};
	int  i = 0;
	//코드 복사

	if(sAging_Recv.A_StoneItemCode == (sinBI1|sin10))      //에이징 스톤
		cAging.AginStoneKind = 1;
	else if(sAging_Recv.A_StoneItemCode == (sinBI1|sin11)) //코퍼 오어
		cAging.AginStoneKind = 2;
	else if(sAging_Recv.A_StoneItemCode == (sinBI1|sin60)) // 박재원 - 엘더 코퍼 오어
		cAging.AginStoneKind = 3;
	else if(sAging_Recv.A_StoneItemCode == (sinBI1|sin61)) // 박재원 - 슈퍼 에이징 스톤
		cAging.AginStoneKind = 4;
	else if(sAging_Recv.A_StoneItemCode == (sinBI1|sin85)) // 장별 - 슈퍼 에이징 스톤 1.5
		cAging.AginStoneKind = 5;
	else
		cAging.AginStoneKind = 0;

	if(sAging_Recv.DesCraftItem.sItemInfo.ItemKindCode == ITEM_KIND_QUEST_WEAPON){ //퀘스트 아이템일경우
		if(SetChangeJob3QuestItem(&sAging_Recv.DesCraftItem.sItemInfo)){
			ReformMixItem( &sAging_Recv.DesCraftItem.sItemInfo  ); //재인증 (아이템을 인벤토리에 셋팅하기전에 ...)
			sAging_Recv.Result = 100+sAging_Recv.DesCraftItem.sItemInfo.ItemAgingNum[0];
			return TRUE;
		}
		sAging_Recv.Result = FALSE;
		return FALSE;
	}
	
	for(int  i = 0 ; i < MAX_MAKEITEM_INFO ; i++){
		//코드를 한번씩 초기화해준다 
		TempCODE[0] = sAging_Recv.DesCraftItem.sItemInfo.CODE; //아이템 코드 
		for(int p = 0 ; p < 15 ; p++){
			if(sAging_Recv.SheltomCode[p])
				TempCODE[p+1] = sAging_Recv.SheltomCode[p]; //그외에 조합될 코드 (ex 꿀,기름)
		}
		for(int j = 0 ; j < 15; j++){
			if(sMakeItem_Info[i].CODE[j]){
				ResultCount++;
				for(int k = 0 ; k < 15 ; k++){
					if(sAging_Recv.SheltomCode[k] && TempCODE[k]){
						if(TempCODE[k] == sMakeItem_Info[i].CODE[j] ){
							TempCODE[k] = -1;
							ResultCountCheck++;
						}
					}
				}
				if(sAging_Recv.DesCraftItem.sItemInfo.CODE == sMakeItem_Info[i].CODE[j]){
					ResultCountCheck++;

				}
			}
		}
		if(ResultCount && ResultCountCheck &&ResultCount == ResultCountCheck){
			sAging_Recv.DesCraftItem.sItemInfo.CODE = sMakeItem_Info[i].Result_CODE;
			sAging_Recv.Result = TRUE;
			//요기서 아이템을 만들어서 클라이언트로 보내준다 
			//sMakeItem_Info[i].Result_CODE;  요건 만들어질 아이템의 코드이다 
			return TRUE;
		}
		else AgingCheckSheltomFlag = 0;
		ResultCount = 0;
		ResultCountCheck = 0;

	}


	if( MakeItemFlag > 0 )return FALSE;
	int DestroyPersent = 0;

	//돈을 뺀다 
	/*
	CheckCharForm(); //인증 
	//sinChar->Money -= sAgingItem.AgingItem[0].sItemInfo.Price+(sAgingItem.AgingItem[0].sItemInfo.Price*sAgingItem.AgingItem[0].sItemInfo.ItemAgingNum[0]);
	sinMinusMoney(sAgingItem.AgingItem[0].sItemInfo.Price+(sAgingItem.AgingItem[0].sItemInfo.Price*sAgingItem.AgingItem[0].sItemInfo.ItemAgingNum[0]));
	ReformCharForm(); //재인증 
	SendSaveMoney(); //금액 조작을 못하게하기위해 호출한다 
	*/

	///////////쉘텀이 맞는지를 확인한다 
	// 박재원 - 에이징 설정 변경(오르도 쉘텀 추가)
	DWORD SheltomCODE[MAX_SHELTOM] = {sinOS1 | sin01,sinOS1 |sin02,sinOS1 |sin03,sinOS1 |sin04,sinOS1 |sin05,sinOS1 |sin06,sinOS1 |sin07,sinOS1 |sin08,sinOS1 |sin09,sinOS1 |sin10,sinOS1 |sin11,sinOS1 |sin12,sinOS1 |sin13, sinOS1 |sin14};
	DWORD CheckSheltomCODE[12] = {0,};
	DWORD CheckSheltomCODE2[12] = {0,};


	DWORD ItemCode[] = {sinOA1,sinOR1,sinOS1}; // 박재원 - 에이징 아이템 추가(장갑, 부츠, 암릿)
	int Cnt = 0;
	while(1){
		if(!ItemCode[Cnt])break;
		if((sAging_Recv.DesCraftItem.sItemInfo.CODE & sinITEM_MASK2) == ItemCode[Cnt]){
			sAging_Recv.Result = FALSE;
			return FALSE;
		}
		Cnt++;
	}


	int   AgingLevel = sAging_Recv.DesCraftItem.sItemInfo.ItemAgingNum[0];

	for(int g = 0 ; g < 12 ; g++){
		if(AgingLevelSheltom[AgingLevel][g]){
			CheckSheltomCODE[g] = SheltomCODE[AgingLevelSheltom[AgingLevel][g]-1];
			CheckSheltomCODE2[g] = sAging_Recv.SheltomCode[g];

		}
	}

	////////쉘텀코드 소팅 
	DWORD Temp=0;
	for(int i = 11; i > 0; i--){
		for(int j = 0; j < i; j++){
			if(CheckSheltomCODE[j] < CheckSheltomCODE[j+1]){
				Temp = CheckSheltomCODE[j+1];
				CheckSheltomCODE[j+1] = CheckSheltomCODE[j];
				CheckSheltomCODE[j] = Temp;
			}
			if(CheckSheltomCODE2[j] < CheckSheltomCODE2[j+1]){
				Temp = CheckSheltomCODE2[j+1];
				CheckSheltomCODE2[j+1] = CheckSheltomCODE2[j];
				CheckSheltomCODE2[j] = Temp;
			}

		}
	}
////////////////////////////////////////////////////////////////////////////
	for(int i = 0; i < 12 ; i++){
		//if(sAging_Recv.SheltomCode[i]){ //받은 쉘텀코드가 아닌 현제 쉘텀창의 쉘텀으로 체크를한다 
		if(CheckSheltomCODE[i]){		  //<============== 여기여기
			if(CheckSheltomCODE2[i] != CheckSheltomCODE[i]){
				return FALSE;
			}
		}
	}
/////////////////////////////////////////////////////////////////////////

	sITEM *pItem = &sAging_Recv.DesCraftItem;

	if(!sinCheckAgingItemHack(pItem)){ //에이징 아이템을 해킹한경우 아이템을 날려버린다뇨!!! 
		sAging_Recv.Result = FALSE;
		return FALSE;
	}

	if(MakeItemFlag == -1)
		DestroyPersent = 100;
	else
		DestroyPersent = GetRandomPos(0,100);
 
	int AgingKind = 0;


	//무기 파괴 
	//int AgingOkPercent[10] = {0,5,15,30,50,60,70,80,90,95};
//	int   PlusAgingPercent[20]          = { 20,10,10,5,5,5,5,5,5,0,0,0,0,0,0,0,0,0,0,0}; //에이징 2배

	// 박재원 : 에이징 2배 확률 조정 08.07.31
	int   PlusAgingPercent[20]          = { 30, 25, 20, 10, 10, 10, 10, 5, 5, 5, 5, 4, 4, 3, 3, 2, 2, 2, 0 ,0}; //에이징 2배

	//0 파괴 1 다운 2 픽스 3 성공 4 따따블
	int AgingItemFaildNum = 3; 

	int DownPersent = GetRandomPos(0,100);    
	int PlusPersent = GetRandomPos(0,100);

//임시 테스트용
#ifdef __SIN_AGING_TEST
	//DestroyPersent = 70; //나중에 꼭빼야함
		//실패 
	if(80 < DestroyPersent){
		switch(sAging_Recv.DesCraftItem.sItemInfo.ItemAgingNum[0]){	
			case 4:
				if(DownPersent < 90)
					AgingItemFaildNum = 1;
				else AgingItemFaildNum = 2;
			break;
			case 5:
				if(DownPersent < 90)
					AgingItemFaildNum = 1;
				else AgingItemFaildNum = 2;
			break;
			case 6:
				if(DownPersent < 90)
					AgingItemFaildNum = 1;
				else if(DownPersent >= 90 && DownPersent < 90)
					AgingItemFaildNum = 2;
				else AgingItemFaildNum = 1; 
			break;
			case 7:
				if(DownPersent < 90)
					AgingItemFaildNum = 1;
				else if(DownPersent >= 90 && DownPersent < 90)
					AgingItemFaildNum = 2;
				else AgingItemFaildNum = 1; 
			break;
			case 8:
				if(DownPersent < 90)
					AgingItemFaildNum = 1;
				else if(DownPersent >= 90 && DownPersent < 90)
					AgingItemFaildNum = 2;
				else AgingItemFaildNum = 0; 
			break;
			case 9:
				if(DownPersent < 90)
					AgingItemFaildNum = 1;
				else if(DownPersent >= 90 && DownPersent < 90)
					AgingItemFaildNum = 2;
				else AgingItemFaildNum = 1; 
			break;
			//에이징 레벨 10이상 (90%로 아이템 소멸)
			case 10:
			case 11:
			case 12:
			case 13:
			case 14:
			case 15:
			case 16:
			case 17:
			case 18:
				if(DownPersent < 100)
					AgingItemFaildNum = 1;
				else AgingItemFaildNum = 1; 
			break;
		}
	}
	else{ //에이징 2배
		if(sAging_Recv.DesCraftItem.sItemInfo.ItemAgingNum[0]<10){  //에이징 10레벨 이상은 +2가없다.
			if(PlusAgingPercent[sAging_Recv.DesCraftItem.sItemInfo.ItemAgingNum[0]] > PlusPersent){
				sinSetAgingItemIncreState(&sAging_Recv.DesCraftItem.sItemInfo);
				AgingItemFaildNum = 4;
			}
		}
	}
#else
	//에이징 스톤 사용시 테이블 변경
	if(cAging.AginStoneKind == 1 ){ 
		//실패    박재원 - AgingStoneAgingOkPercent(에이징 스톤을 사용했을 경우 실패확률) 에이징 스톤을 사용하면 실패확률을 낮춰준다.
		if(AgingStoneAgingOkPercent[sAging_Recv.DesCraftItem.sItemInfo.ItemAgingNum[0]] > DestroyPersent){
			switch(sAging_Recv.DesCraftItem.sItemInfo.ItemAgingNum[0]){
				// 박재원 - 에이징 스톤 사용유무 상관없이 실패했을 때 각각의 -1, -2, 소멸(파괴) 될 확률은 동일하다. 
				case 6:
					if(DownPersent < 90)
						AgingItemFaildNum = 1;
					else // 장별 - 에이징 6,7차 소멸 없음
						AgingItemFaildNum = 2;
					break;
				case 7:
					if(DownPersent < 80)
						AgingItemFaildNum = 1;
					else // 장별 - 에이징 6,7차 소멸 없음
						AgingItemFaildNum = 2;
					break;
				case 8:
					if(DownPersent < 70)
						AgingItemFaildNum = 1;
					else if(DownPersent >= 70 && DownPersent < 85)
						AgingItemFaildNum = 2;
					else AgingItemFaildNum = 0; 
					break;
				case 9:
					if(DownPersent < 60)
						AgingItemFaildNum = 1;
					else if(DownPersent >= 60 && DownPersent < 80)
						AgingItemFaildNum = 2;
					else AgingItemFaildNum = 0; 
					break;
				case 10:
					if(DownPersent < 50)
						AgingItemFaildNum = 1;
					else if(DownPersent >= 50 && DownPersent < 70)
						AgingItemFaildNum = 2;
					else AgingItemFaildNum = 0; 
					break;
				case 11:
					if(DownPersent < 40)
						AgingItemFaildNum = 1;
					else if(DownPersent >= 40 && DownPersent < 60)
						AgingItemFaildNum = 2;
					else AgingItemFaildNum = 0; 
					break;
				case 12:
					if(DownPersent < 30)
						AgingItemFaildNum = 1;
					else if(DownPersent >= 30 && DownPersent < 50)
						AgingItemFaildNum = 2;
					else AgingItemFaildNum = 0; 
					break;	
				case 13:
					if(DownPersent < 30)
						AgingItemFaildNum = 1;
					else if(DownPersent >= 30 && DownPersent < 40)
						AgingItemFaildNum = 2;
					else AgingItemFaildNum = 0; 
					break;
				case 14:
					if(DownPersent < 20)
						AgingItemFaildNum = 1;
					else if(DownPersent >= 20 && DownPersent < 30)
						AgingItemFaildNum = 2;
					else AgingItemFaildNum = 0; 
					break;
				case 15:
				case 16:
				case 17:
				case 18:
				case 19: // 박재원 - 에이징 설정 변경(19차, 20차 에이징 확률 추가)
				case 20: // 박재원 - 에이징 설정 변경(19차, 20차 에이징 확률 추가)
					if(DownPersent < 10)
						AgingItemFaildNum = 1;
					else if(DownPersent >= 10 && DownPersent < 20)
						AgingItemFaildNum = 2;
					else AgingItemFaildNum = 0; 
					break;
			}
		}
		else{ //에이징 2배
			if(sAging_Recv.DesCraftItem.sItemInfo.ItemAgingNum[0]<10){  //에이징 10레벨 이상은 +2가없다.
				if((PlusAgingPercent[sAging_Recv.DesCraftItem.sItemInfo.ItemAgingNum[0]]*2) > PlusPersent){
					sinSetAgingItemIncreState(&sAging_Recv.DesCraftItem.sItemInfo);
					AgingItemFaildNum = 4;
				}
			}
		}	
	}


	// 장별 - 슈퍼 에이징 스톤  
	else if( cAging.AginStoneKind == 4 )
	{
		if( SuperAgingStoneAgingFailPercent[sAging_Recv.DesCraftItem.sItemInfo.ItemAgingNum[0]] > DestroyPersent )
		{
			switch(sAging_Recv.DesCraftItem.sItemInfo.ItemAgingNum[0])
			{

				case 6:
					if(DownPersent < 90)
						AgingItemFaildNum = 1;
					else // 장별 - 에이징 6,7차 소멸 없음
						AgingItemFaildNum = 2;
					break;
				case 7:
					if(DownPersent < 80)
						AgingItemFaildNum = 1;
					else // 장별 - 에이징 6,7차 소멸 없음
						AgingItemFaildNum = 2;
					break;
				case 8:
					if(DownPersent < 70)
						AgingItemFaildNum = 1;
					else if(DownPersent >= 70 && DownPersent < 85)
						AgingItemFaildNum = 2;
					else AgingItemFaildNum = 0; 
					break;
				case 9:
					if(DownPersent < 60)
						AgingItemFaildNum = 1;
					else if(DownPersent >= 60 && DownPersent < 80)
						AgingItemFaildNum = 2;
					else AgingItemFaildNum = 0; 
					break;
				case 10:
					if(DownPersent < 50)
						AgingItemFaildNum = 1;
					else if(DownPersent >= 50 && DownPersent < 70)
						AgingItemFaildNum = 2;
					else AgingItemFaildNum = 0; 
					break;
				case 11:
					if(DownPersent < 40)
						AgingItemFaildNum = 1;
					else if(DownPersent >= 40 && DownPersent < 60)
						AgingItemFaildNum = 2;
					else AgingItemFaildNum = 0; 
					break;
				case 12:
					if(DownPersent < 30)
						AgingItemFaildNum = 1;
					else if(DownPersent >= 30 && DownPersent < 50)
						AgingItemFaildNum = 2;
					else AgingItemFaildNum = 0; 
					break;	
				case 13:
					if(DownPersent < 30)
						AgingItemFaildNum = 1;
					else if(DownPersent >= 30 && DownPersent < 40)
						AgingItemFaildNum = 2;
					else AgingItemFaildNum = 0; 
					break;
				case 14:
					if(DownPersent < 20)
						AgingItemFaildNum = 1;
					else if(DownPersent >= 20 && DownPersent < 30)
						AgingItemFaildNum = 2;
					else AgingItemFaildNum = 0; 
					break;
				case 15:
				case 16:
				case 17:
				case 18:
				case 19: // 박재원 - 에이징 설정 변경(19차, 20차 에이징 확률 추가)
				case 20: // 박재원 - 에이징 설정 변경(19차, 20차 에이징 확률 추가)
					if(DownPersent < 10)
						AgingItemFaildNum = 1;
					else if(DownPersent >= 10 && DownPersent < 20)
						AgingItemFaildNum = 2;
					else AgingItemFaildNum = 0; 
					break;
			}
		}
	}

	// 장별 - 슈퍼 에이징 스톤 1.5    
	else if( cAging.AginStoneKind == 5 )
	{
		if( SuperAgingStone1_5AgingFailPercent[sAging_Recv.DesCraftItem.sItemInfo.ItemAgingNum[0]] > DestroyPersent )
		{
			switch(sAging_Recv.DesCraftItem.sItemInfo.ItemAgingNum[0])
			{

				case 6:
					if(DownPersent < 90)
						AgingItemFaildNum = 1;
					else // 장별 - 에이징 6,7차 소멸 없음
						AgingItemFaildNum = 2;
					break;
				case 7:
					if(DownPersent < 80)
						AgingItemFaildNum = 1;
					else // 장별 - 에이징 6,7차 소멸 없음
						AgingItemFaildNum = 2;
					break;
				case 8:
					if(DownPersent < 70)
						AgingItemFaildNum = 1;
					else if(DownPersent >= 70 && DownPersent < 85)
						AgingItemFaildNum = 2;
					else AgingItemFaildNum = 0; 
					break;
				case 9:
					if(DownPersent < 60)
						AgingItemFaildNum = 1;
					else if(DownPersent >= 60 && DownPersent < 80)
						AgingItemFaildNum = 2;
					else AgingItemFaildNum = 0; 
					break;
				case 10:
					if(DownPersent < 50)
						AgingItemFaildNum = 1;
					else if(DownPersent >= 50 && DownPersent < 70)
						AgingItemFaildNum = 2;
					else AgingItemFaildNum = 0; 
					break;
				case 11:
					if(DownPersent < 40)
						AgingItemFaildNum = 1;
					else if(DownPersent >= 40 && DownPersent < 60)
						AgingItemFaildNum = 2;
					else AgingItemFaildNum = 0; 
					break;
				case 12:
					if(DownPersent < 30)
						AgingItemFaildNum = 1;
					else if(DownPersent >= 30 && DownPersent < 50)
						AgingItemFaildNum = 2;
					else AgingItemFaildNum = 0; 
					break;	
				case 13:
					if(DownPersent < 30)
						AgingItemFaildNum = 1;
					else if(DownPersent >= 30 && DownPersent < 40)
						AgingItemFaildNum = 2;
					else AgingItemFaildNum = 0; 
					break;
				case 14:
					if(DownPersent < 20)
						AgingItemFaildNum = 1;
					else if(DownPersent >= 20 && DownPersent < 30)
						AgingItemFaildNum = 2;
					else AgingItemFaildNum = 0; 
					break;
				case 15:
				case 16:
				case 17:
				case 18:
				case 19: // 박재원 - 에이징 설정 변경(19차, 20차 에이징 확률 추가)
				case 20: // 박재원 - 에이징 설정 변경(19차, 20차 에이징 확률 추가)
					if(DownPersent < 10)
						AgingItemFaildNum = 1;
					else if(DownPersent >= 10 && DownPersent < 20)
						AgingItemFaildNum = 2;
					else AgingItemFaildNum = 0; 
					break;
			}
		}
	}

	else
	{
		// 박재원 : 에이징 레벨별 실패 확률 08.07.31
		if(AgingOkPercent[sAging_Recv.DesCraftItem.sItemInfo.ItemAgingNum[0]] > DestroyPersent){
			switch(sAging_Recv.DesCraftItem.sItemInfo.ItemAgingNum[0]){	
				case 6:
					if(DownPersent < 90)
						AgingItemFaildNum = 1;
					else // 장별 - 에이징 6,7차 소멸 없음
						AgingItemFaildNum = 2;
				break;
				case 7:
					if(DownPersent < 80)
						AgingItemFaildNum = 1;
					else // 장별 - 에이징 6,7차 소멸 없음
						AgingItemFaildNum = 2;
				break;
				case 8:
					if(DownPersent < 70)
						AgingItemFaildNum = 1;
					else if(DownPersent >= 70 && DownPersent < 85)
						AgingItemFaildNum = 2;
					else AgingItemFaildNum = 0; 
				break;
				case 9:
					if(DownPersent < 60)
						AgingItemFaildNum = 1;
					else if(DownPersent >= 60 && DownPersent < 80)
						AgingItemFaildNum = 2;
					else AgingItemFaildNum = 0; 
				break;
				case 10:
					if(DownPersent < 50)
						AgingItemFaildNum = 1;
					else if(DownPersent >= 50 && DownPersent < 70)
						AgingItemFaildNum = 2;
					else AgingItemFaildNum = 0; 
				break;
				case 11:
					if(DownPersent < 40)
						AgingItemFaildNum = 1;
					else if(DownPersent >= 40 && DownPersent < 60)
						AgingItemFaildNum = 2;
					else AgingItemFaildNum = 0; 
				break;
				case 12:
					if(DownPersent < 30)
						AgingItemFaildNum = 1;
					else if(DownPersent >= 30 && DownPersent < 50)
						AgingItemFaildNum = 2;
					else AgingItemFaildNum = 0; 
				break;	
				case 13:
					if(DownPersent < 30)
						AgingItemFaildNum = 1;
					else if(DownPersent >= 30 && DownPersent < 40)
						AgingItemFaildNum = 2;
					else AgingItemFaildNum = 0; 
				break;
				case 14:
					if(DownPersent < 20)
						AgingItemFaildNum = 1;
					else if(DownPersent >= 20 && DownPersent < 30)
						AgingItemFaildNum = 2;
					else AgingItemFaildNum = 0; 
				break;
				case 15:
				case 16:
				case 17:
				case 18:
				case 19: // 박재원 - 에이징 설정 변경(19차, 20차 에이징 확률 추가)
				case 20: // 박재원 - 에이징 설정 변경(19차, 20차 에이징 확률 추가)
					if(DownPersent < 10)
						AgingItemFaildNum = 1;
					else if(DownPersent >= 10 && DownPersent < 20)
						AgingItemFaildNum = 2;
					else AgingItemFaildNum = 0; 
				break;
			}
		}
		else{ //에이징 2배
			if(sAging_Recv.DesCraftItem.sItemInfo.ItemAgingNum[0]<10){  //에이징 10레벨 이상은 +2가없다.
				if(PlusAgingPercent[sAging_Recv.DesCraftItem.sItemInfo.ItemAgingNum[0]] > PlusPersent){
					sinSetAgingItemIncreState(&sAging_Recv.DesCraftItem.sItemInfo);
					AgingItemFaildNum = 4;
				}
			}
		}
		
		if( cAging.AginStoneKind == 2 || //코퍼  오어 
			cAging.AginStoneKind == 3 || // 박재원 - 엘더 코퍼 오어 사용시에도 무기 파괴를 방지한다.(무기 소멸 방지)
			cAging.AginStoneKind == 4 || // 박재원 - 슈퍼 에이징 스톤 사용시에도 무기 파괴를 방지한다.(무기 소멸 방지)
			cAging.AginStoneKind == 5 ) // 장별 - 슈퍼 에이징 스톤 1.5 
		{ 
			if(AgingItemFaildNum == 0)
			{   //무기가 파괴되지 않게한다.			
				if(rand()%100 < 81) // 박재원 : 코퍼 오어 장착시 에이징하면 파괴안되고 대신 차수를 낮춘다.
					AgingItemFaildNum = 1;
				else
					AgingItemFaildNum = 2;				
			}
		}
	}
#endif
	////////////////////////////// 나중에 꼭빼야함(-1강제 세팅시 사)
	//AgingItemFaildNum = 1;

	// 장별 - 슈퍼 에이징스톤 수정
	if(AgingItemFaildNum == 0)
	{
		if( cAging.AginStoneKind == 2 || //코퍼  오어 
			cAging.AginStoneKind == 3 || // 엘더 코퍼 오어
			cAging.AginStoneKind == 4 || // 슈퍼 에이징 스톤
			cAging.AginStoneKind == 5 ) // 장별 - 슈퍼 에이징 스톤 1.5 

		{
			if(rand()%100 < 81) 
					AgingItemFaildNum = 1;
				else
					AgingItemFaildNum = 2;		
		}
	}
	
	if(!AgingItemFaildNum){
		sAging_Recv.Result = FALSE;
		return FALSE;
	}

	/*
	if(AgingOkPercent[sAging_Recv.DesCraftItem.sItemInfo.ItemAgingNum[0]] > DestroyPersent){
		if(sAging_Recv.DesCraftItem.sItemInfo.ItemAgingNum[0] ==3 || 
			sAging_Recv.DesCraftItem.sItemInfo.ItemAgingNum[0] ==4 )
		{
			AgingItemFaildNum = 1;

		}
		else if( sAging_Recv.DesCraftItem.sItemInfo.ItemAgingNum[0] ==5){
			AgingItemFaildNum = GetRandomPos(1,2);
		}
		else{
			AgingItemFaildNum = GetRandomPos(0,2);
		}

	}
	if(!AgingItemFaildNum){
		sAging_Recv.Result = FALSE;
		return FALSE;
	}
	*/

	if(AgingItemFaildNum ==1 || AgingItemFaildNum ==2 ){
		sinAgingItemInit(&pItem->sItemInfo,AgingItemFaildNum);//능력치 다운

	}


	//크리티컬 
	if( (pItem->CODE & sinITEM_MASK2) == sinWC1 ||
		(pItem->CODE & sinITEM_MASK2) == sinWS2 ||
		(pItem->CODE & sinITEM_MASK2) == sinWP1 ||
		(pItem->CODE & sinITEM_MASK2) == sinWD1 ||		// daggers as swords
		(pItem->CODE & sinITEM_MASK2) == sinWV1 ) {		// vambraces as claws
		pItem->sItemInfo.ItemAgingCount[1] = AgingLevelCritical[pItem->sItemInfo.ItemAgingNum[0]];
		if(AgingItemFaildNum <= 2)
			pItem->sItemInfo.ItemAgingCount[0] = pItem->sItemInfo.ItemAgingCount[1]/2;

		}

	//공격 
	if( (pItem->CODE & sinITEM_MASK2) == sinWA1 || 
		(pItem->CODE & sinITEM_MASK2) == sinWH1 ||
		(pItem->CODE & sinITEM_MASK2) == sinWM1 ||
		(pItem->CODE & sinITEM_MASK2) == sinWS1 ||
		(pItem->CODE & sinITEM_MASK2) == sinWT1 ||
		(pItem->CODE & sinITEM_MASK2) == sinWN1 ){		// phantoms as staves
		pItem->sItemInfo.ItemAgingCount[1] = AgingLevelAttack[pItem->sItemInfo.ItemAgingNum[0]];
		if(AgingItemFaildNum <= 2)
			pItem->sItemInfo.ItemAgingCount[0] = pItem->sItemInfo.ItemAgingCount[1]/2;

	}



	//블럭 
	if( (pItem->CODE & sinITEM_MASK2) == sinDS1){
		pItem->sItemInfo.ItemAgingCount[1] = AgingLevelBlock[pItem->sItemInfo.ItemAgingNum[0]];
		if(AgingItemFaildNum <= 2)
			pItem->sItemInfo.ItemAgingCount[0] = pItem->sItemInfo.ItemAgingCount[1]/2;

	}


	//타격(받음)
	if( (pItem->CODE & sinITEM_MASK2) == sinDA1 ||
		(pItem->CODE & sinITEM_MASK2) == sinDA2 ||
		(pItem->CODE & sinITEM_MASK2) == sinOM1 ||
		(pItem->CODE & sinITEM_MASK2) == sinDB1 ||  // 박재원 - 에이징 아이템 추가(부츠)
		(pItem->CODE & sinITEM_MASK2) == sinDG1 ||  // 박재원 - 에이징 아이템 추가(장갑)
		(pItem->CODE & sinITEM_MASK2) == sinOA2 ){  // 박재원 - 에이징 아이템 추가(암릿)
		pItem->sItemInfo.ItemAgingCount[1] = AgingLevelHit[pItem->sItemInfo.ItemAgingNum[0]];
		if(AgingItemFaildNum <= 2)
			pItem->sItemInfo.ItemAgingCount[0] = pItem->sItemInfo.ItemAgingCount[1]/2;

	}

	if(pItem->CODE == (sinOR2|sin01)){
		pItem->sItemInfo.ItemAgingCount[1] = 60*20; //20분으로 한다

	}
	
	pItem->sItemInfo.ItemAgingNum[1] = 1;	//에이징 시작 
	pItem->sItemInfo.ItemKindCode = ITEM_KIND_AGING;
	if(AgingItemFaildNum >= 3)
		pItem->sItemInfo.ItemAgingCount[0] = 0; //에이징 카운트 초기화



	///////////////////보안을 위해 아이템을 증정용으로 표시한다 
	//pItem->sItemInfo.CurseItemFlag[0] = CHECK_GIVE_ITEM;


	//////////////////에이징 가능 플랙 설정 
//	short   ItemAgingNum[2];	//0은 에이징+몇 1은 에이징이 되고있나 아닌가?
//	short   ItemAgingCount[2];	//0아이템 숙련 카운트 1숙련치 최대 


	memcpy(&sAging_Recv.DesCraftItem ,pItem,sizeof(sITEM));
	ReformMixItem( &sAging_Recv.DesCraftItem.sItemInfo  ); //재인증 (아이템을 인벤토리에 셋팅하기전에 ...)
	sAging_Recv.Result = AgingItemFaildNum;

/*
	
	for(int i=0;i<15;i++){
		if(sAgingItem.AgingItem[i].Flag){ //쉘텀은 날린다 
			sAgingItem.AgingItem[i].Flag = 0;

		}

	}
	
	cAging.OpenFlag = 0; //에이징 인터페이스를 닫는다 
	AgingCheckSheltomFlag = 0; //에이징 가능 플랙 초기화 
	AgingSheltomCnt2 = 0;
*/
	return TRUE;
}

//에이징 아이템을 취소한다 
int cAGING::CancelAgingItem(int Flag)
{
	if(MixItemNoCopyFlag)return FALSE;    
	for(int i=0;i<15;i++){
		if(sAgingItem.AgingItem[i].Flag){ 
			if(!cInvenTory.AutoSetInvenItem(&sAgingItem.AgingItem[i])){
				sinThrowItemToFeild(&sAgingItem.AgingItem[i]);
				
			}
			sAgingItem.AgingItem[i].Flag = 0;
		}
	}
	
	for(int y=0;y<4;y++){ //쉘텀 인덱스 초기화 
		for(int h= 0 ;h < 3  ;h ++){
			AgingLevelSheltomIndex[y][h] = 0;
		}

	}

	AgingCheckSheltomFlag = 0; //에이징 가능 플랙 초기화 
	AgingSheltomCnt2 = 0;
	if(!Flag)
		cAging.OpenFlag = 0 ; //창을 닫는다 

	ResetInvenItemCode();				//이벤 아이템 코드 초기화
	return TRUE;
}

//에이징할수있는 아이템인지를 체크한다 
int cAGING::CheckItem(sITEM *pItem)
{
	int k = 0;
	int SheltomCnt = 0;
	if((pItem->CODE & sinITEM_MASK2) == sinOS1 ){
		AgingSheltomCnt2++;

	}
	else{
		for(int i = 0 ; i < 4 ;i ++){
			for(int j=0 ; j < 3 ; j ++){
				AgingLevelSheltomIndex[i][j] = AgingLevelSheltom[pItem->sItemInfo.ItemAgingNum[0]][k++]; //쉘텀의 종류와 인덱스를 넣는다 

			}

		}

	}
	if(AgingSheltomCnt2){
		if(sAgingItem.AgingItem[0].Flag){
			for( int h = 0 ; h < 12 ; h ++){
				if(AgingLevelSheltom[sAgingItem.AgingItem[0].sItemInfo.ItemAgingNum[0]][h])
					SheltomCnt++;
			}

		}
		if(AgingSheltomCnt2 == SheltomCnt)
			AgingCheckSheltomFlag = 1;
		else
			AgingCheckSheltomFlag = 0;

	}

	return FALSE;

}

// 박재원 - 에이징 아이템 추가(장갑, 부츠, 암릿)
#define AGING_ITEM_CODE3_MAX	18		// + the daggers, phantoms and vambraces
DWORD AgingItemCode3[AGING_ITEM_CODE3_MAX] = {sinWA1,sinWC1,sinWH1,sinWM1,sinWP1,sinWS1,sinWS2,sinWT1,sinDA1,sinDS1,sinOM1,sinDA2,sinDG1,sinDB1,sinOA2,sinWD1,sinWN1,sinWV1};
//에이징할 아이템의 공간을 체크한다 
int cAGING::SetAgingItemAreaCheck(sITEM *pItem)
{
	// 박재원 - 에이징 설정 변경(오르도 추가)
	DWORD SheltomCODE[MAX_SHELTOM] = {sin01,sin02,sin03,sin04,sin05,sin06,sin07,sin08,sin09,sin10,sin11,sin12,sin13,sin14}; 
	int SheltomIndex2 = 0;
	int i,j;
	//에이징 스톤/ 코퍼 오어-------------------------------------------------------------
	if( pItem->sItemInfo.CODE == (sinBI1|sin10) || pItem->sItemInfo.CODE == (sinBI1|sin11) ||
		pItem->sItemInfo.CODE == (sinBI1|sin60) || pItem->sItemInfo.CODE == (sinBI1|sin61) || // 박재원 - 엘더 코퍼 오어, 슈퍼 에이징 스톤 추가
		pItem->sItemInfo.CODE == (sinBI1|sin85) ) // 장별 - 슈퍼 에이징 스톤 1.5
	{
		cAging.AginStoneKind = 0;
		int AgingStoneX=159+11,AgingStoneY=268+11+sinInterHeight2;
		TradeColorIndex= 0; //초기화   (트레이드 창과 창고가 같이 쓴다)
		cTrade.InitTradeColorRect(); //초기화
		if(AgingStoneX<=pItem->x+22 && AgingStoneX >pItem->x &&AgingStoneY< pItem->y+22 && AgingStoneY > pItem->y){		
			pItem->SetX = AgingStoneX +( ( ( pItem->x+22 - AgingStoneX) /22)*22)-11;
			pItem->SetY = AgingStoneY +( ( ( pItem->y+22 - AgingStoneY) /22)*22)-11;
			pItem->ItemPosition = 0; //배열의 인덱스를 저장한다 
			TradeColorIndex = SET_ITEM_CHECK_COLOR; //셋팅할영역
			if(TradeEndX < (TradeColorRect.left + TradeColorRect.right)-11 || TradeEndY < (TradeColorRect.top + TradeColorRect.bottom)-11){
					cTrade.InitTradeColorRect(); //컬러 박스 RECT 초기화 (컬러 박스 RECT중 하나라두 0 이면 셋팅할수 없다 
					return FALSE;
			}
			if(TradeCrashItemIndex[0] = cTrade.CrashTradeItem(TradeColorRect,0,3)){  //겹치는 아이템이 있나 체크 
					TradeColorIndex = NOT_SETTING_COLOR;
					return FALSE;
			}
			return TRUE;
		}
	}

	if( pItem->sItemInfo.CODE == (sinBI1|sin61) ) // 박재원 - 엘더 코퍼 오어, 슈퍼 에이징 스톤 추가
	{
		cAging.AginStoneKind = 4;
		int AgingStoneX=159+11,AgingStoneY=268+11+sinInterHeight2;
		TradeColorIndex= 0; //초기화   (트레이드 창과 창고가 같이 쓴다)
		cTrade.InitTradeColorRect(); //초기화
		if(AgingStoneX<=pItem->x+22 && AgingStoneX >pItem->x &&AgingStoneY< pItem->y+22 && AgingStoneY > pItem->y){		
			pItem->SetX = AgingStoneX +( ( ( pItem->x+22 - AgingStoneX) /22)*22)-11;
			pItem->SetY = AgingStoneY +( ( ( pItem->y+22 - AgingStoneY) /22)*22)-11;
			pItem->ItemPosition = 0; //배열의 인덱스를 저장한다 
			TradeColorIndex = SET_ITEM_CHECK_COLOR; //셋팅할영역
			if(TradeEndX < (TradeColorRect.left + TradeColorRect.right)-11 || TradeEndY < (TradeColorRect.top + TradeColorRect.bottom)-11){
					cTrade.InitTradeColorRect(); //컬러 박스 RECT 초기화 (컬러 박스 RECT중 하나라두 0 이면 셋팅할수 없다 
					return FALSE;
			}
			if(TradeCrashItemIndex[0] = cTrade.CrashTradeItem(TradeColorRect,0,3)){  //겹치는 아이템이 있나 체크 
					TradeColorIndex = NOT_SETTING_COLOR;
					return FALSE;
			}
			return TRUE;
		}
	}


	// 장별 - 슈퍼 에이징 스톤 1.5
	if( pItem->sItemInfo.CODE == (sinBI1|sin85) ) 
	{
		cAging.AginStoneKind = 5;
		int AgingStoneX=159+11,AgingStoneY=268+11+sinInterHeight2;
		TradeColorIndex= 0; //초기화   (트레이드 창과 창고가 같이 쓴다)
		cTrade.InitTradeColorRect(); //초기화
		if(AgingStoneX<=pItem->x+22 && AgingStoneX >pItem->x &&AgingStoneY< pItem->y+22 && AgingStoneY > pItem->y){		
			pItem->SetX = AgingStoneX +( ( ( pItem->x+22 - AgingStoneX) /22)*22)-11;
			pItem->SetY = AgingStoneY +( ( ( pItem->y+22 - AgingStoneY) /22)*22)-11;
			pItem->ItemPosition = 0; //배열의 인덱스를 저장한다 
			TradeColorIndex = SET_ITEM_CHECK_COLOR; //셋팅할영역
			if(TradeEndX < (TradeColorRect.left + TradeColorRect.right)-11 || TradeEndY < (TradeColorRect.top + TradeColorRect.bottom)-11){
					cTrade.InitTradeColorRect(); //컬러 박스 RECT 초기화 (컬러 박스 RECT중 하나라두 0 이면 셋팅할수 없다 
					return FALSE;
			}
			if(TradeCrashItemIndex[0] = cTrade.CrashTradeItem(TradeColorRect,0,3)){  //겹치는 아이템이 있나 체크 
					TradeColorIndex = NOT_SETTING_COLOR;
					return FALSE;
			}
			return TRUE;
		}
	}


	//-----------------------------------------------------------------------------------
	if((pItem->sItemInfo.CODE  & sinITEM_MASK2)== sinMA2) {
		if(sAgingItem.AgingItem[0].Flag && (sAgingItem.AgingItem[0].sItemInfo.CODE & sinITEM_MASK2) == sinMA1){
			TradeColorIndex= 0; //초기화   (트레이드 창과 창고가 같이 쓴다)
			TradeCrashItemIndex[0] = 0; //충돌된 아이템 Index 초기화 
			cTrade.InitTradeColorRect(); //초기화
			TradeStartX = 218;
			TradeStartY = 193+sinInterHeight2;
			TradeEndX = TradeStartX+(3*22);
			TradeEndY = TradeStartY+(4*22);

			for(i=pItem->x+11 ; i<pItem->x+pItem->w ; i+=22){
				for( j=pItem->y+11 ; j<pItem->y+pItem->h ; j+=22){
					if(TradeStartX <= i && TradeEndX > i && TradeStartY <=j && TradeEndY > j){ //인벤토리 박스에 들어갔는지를 체크 
						TradeColorRect.left =  TradeStartX +( ( ( i - TradeStartX) /22)*22) ; 
						TradeColorRect.top  =  TradeStartY +( ( ( j - TradeStartY) /22)*22) ; 
						TradeColorRect.right  = pItem->w;
						TradeColorRect.bottom = pItem->h;
						if(TradeEndX < (TradeColorRect.left + TradeColorRect.right)-11 || TradeEndY < (TradeColorRect.top + TradeColorRect.bottom)-11){
							cTrade.InitTradeColorRect(); //컬러 박스 RECT 초기화 (컬러 박스 RECT중 하나라두 0 이면 셋팅할수 없다 
							return FALSE;
						}

						pItem->SetX = TradeColorRect.left;
						pItem->SetY = TradeColorRect.top;
						pItem->ItemPosition = 0; //배열의 인덱스를 저장한다 
						TradeColorIndex = SET_ITEM_CHECK_COLOR; //셋팅할영역

						if(TradeCrashItemIndex[0] = cTrade.CrashTradeItem(TradeColorRect,0,3)){  //겹치는 아이템이 있나 체크 
							TradeColorIndex = NOT_SETTING_COLOR;
							return FALSE;
						}
						return TRUE;

					}
				
				}

			}
		}

	}

	if((pItem->sItemInfo.CODE  & sinITEM_MASK2)== sinMA1){
		TradeColorIndex= 0; //초기화   (트레이드 창과 창고가 같이 쓴다)
		TradeCrashItemIndex[0] = 0; //충돌된 아이템 Index 초기화 
		cTrade.InitTradeColorRect(); //초기화

		TradeStartX = 58;
		TradeStartY = 193+sinInterHeight2;
		TradeEndX = TradeStartX+(3*22);
		TradeEndY = TradeStartY+(4*22);
		if(TradeStartX <= pCursorPos.x && TradeEndX > pCursorPos.x && TradeStartY <=pCursorPos.y && TradeEndY > pCursorPos.y){ //인벤토리 박스에 들어갔는지를 체크 
			TradeColorRect.left =  TradeStartX;
			TradeColorRect.top  =  TradeStartY;
			TradeColorRect.right  = TradeEndX - TradeStartX;
			TradeColorRect.bottom = TradeEndY - TradeStartY;


			pItem->SetX = TradeStartX+((TradeEndX - TradeStartX)-pItem->w)/2;
			pItem->SetY = TradeStartY+((TradeEndY - TradeStartY)-pItem->h)/2;
			pItem->ItemPosition = 0; //배열의 인덱스를 저장한다 
			TradeColorIndex = SET_ITEM_CHECK_COLOR; //셋팅할영역
			if(TradeCrashItemIndex[0] = cTrade.CrashTradeItem(TradeColorRect,0,3)){  //겹치는 아이템이 있나 체크 
				TradeColorIndex = NOT_SETTING_COLOR;
				return FALSE;

			}
			return TRUE;
		}
	}
	
	/////////여기는 기본에이징 
	for(int y = 0 ;y < MAX_SHELTOM ; y++){
		if(SheltomCODE[y] ==( pItem->sItemInfo.CODE & sinITEM_MASK3)){
			SheltomIndex2 = y+1;
			break;
		}
	}
	
	int AgingOkWeaponGroupFlag = 0;	//에이징을 할수있는 아이템 군인가?
	TradeColorIndex= 0; //초기화   (트레이드 창과 창고가 같이 쓴다)
	TradeCrashItemIndex[0] = 0; //충돌된 아이템 Index 초기화 
	cTrade.InitTradeColorRect(); //초기화

	if((pItem->CODE & sinITEM_MASK2)  == sinOS1){ //쉘텀일 경우 
//		TradeStartX = 218;
//		TradeStartY = 193;
//		TradeEndX = TradeStartX+(3*22);
//		TradeEndY = TradeStartY+(4*22);

		for(int p = 0 ; p < 4 ; p++){
			for(int m=0; m < 3 ; m++){
				if(AgingLevelSheltomIndex[p][m] == SheltomIndex2){
					TradeStartX = AgingLevelSheltomXY[p][m].x;
					TradeStartY = AgingLevelSheltomXY[p][m].y;
					TradeEndX = TradeStartX+22;
					TradeEndY = TradeStartY+22;
					for(i=pItem->x+11 ; i<pItem->x+pItem->w ; i+=22){
						for( j=pItem->y+11 ; j<pItem->y+pItem->h ; j+=22){
							if(TradeStartX <= i && TradeEndX > i && TradeStartY <=j && TradeEndY > j){ //인벤토리 박스에 들어갔는지를 체크 
								TradeColorRect.left =  TradeStartX +( ( ( i - TradeStartX) /22)*22) ; 
								TradeColorRect.top  =  TradeStartY +( ( ( j - TradeStartY) /22)*22) ; 
								TradeColorRect.right  = pItem->w;
								TradeColorRect.bottom = pItem->h;
								if(TradeEndX < (TradeColorRect.left + TradeColorRect.right)-11 || TradeEndY < (TradeColorRect.top + TradeColorRect.bottom)-11){
									cTrade.InitTradeColorRect(); //컬러 박스 RECT 초기화 (컬러 박스 RECT중 하나라두 0 이면 셋팅할수 없다 
									return FALSE;
								}

								pItem->SetX = TradeColorRect.left;
								pItem->SetY = TradeColorRect.top;
								pItem->ItemPosition = 0; //배열의 인덱스를 저장한다 
								if( (pItem->CODE & sinITEM_MASK3) > sin20) // 박재원 - 매직 포스(에이징할때 매직 쉘텀은 올려놓지 못하도록 한다)
								{
									TradeColorIndex = NOT_SETTING_COLOR; 
								}
								else
									TradeColorIndex = SET_ITEM_CHECK_COLOR; //셋팅할영역
								
								if(TradeCrashItemIndex[0] = cTrade.CrashTradeItem(TradeColorRect,0,3)){  //겹치는 아이템이 있나 체크 
									TradeColorIndex = NOT_SETTING_COLOR;
										return FALSE;
				
								}
								return TRUE;
							}
						}
					}
				}
			}
		}
	}
	else{
		TradeColorIndex= 0; //초기화   (트레이드 창과 창고가 같이 쓴다)
		TradeCrashItemIndex[0] = 0; //충돌된 아이템 Index 초기화 
		cTrade.InitTradeColorRect(); //초기화

		TradeStartX = 58;
		TradeStartY = 193+sinInterHeight2;
		TradeEndX = TradeStartX+(3*22);
		TradeEndY = TradeStartY+(4*22);
		if(TradeStartX <= pCursorPos.x && TradeEndX > pCursorPos.x && TradeStartY <=pCursorPos.y && TradeEndY > pCursorPos.y){ //인벤토리 박스에 들어갔는지를 체크 
			TradeColorRect.left =  TradeStartX;
			TradeColorRect.top  =  TradeStartY;
			TradeColorRect.right  = TradeEndX - TradeStartX;
			TradeColorRect.bottom = TradeEndY - TradeStartY;


			pItem->SetX = TradeStartX+((TradeEndX - TradeStartX)-pItem->w)/2;
			pItem->SetY = TradeStartY+((TradeEndY - TradeStartY)-pItem->h)/2;
			pItem->ItemPosition = 0; //배열의 인덱스를 저장한다 
			TradeColorIndex = SET_ITEM_CHECK_COLOR; //셋팅할영역
			
			for(int i =0 ; i < AGING_ITEM_CODE3_MAX ; i++){
				if((pItem->CODE &  sinITEM_MASK2) == AgingItemCode3[i]){
					AgingOkWeaponGroupFlag = 1;

				}
			}

			//부활의 반지도 에이징할수있다 
			if( pItem->sItemInfo.CODE == (sinOR2 | sin01))AgingOkWeaponGroupFlag = 1;

			//에이징을 할수없는 아이템군  일경우에는 
			if(!AgingOkWeaponGroupFlag || pItem->sItemInfo.ItemAgingNum[0] >= MAX_AGING ){  //에이징 +18
				TradeColorIndex = NOT_AGING_ITEM;
				return FALSE;
			}

			//에이징 할수없는 아이템일 경우 
			if(pItem->sItemInfo.UniqueItem || pItem->sItemInfo.ItemKindCode == ITEM_KIND_CRAFT || MouseItem.sItemInfo.ItemAgingNum[1] == 1 ||
				pItem->sItemInfo.ItemKindCode == ITEM_KIND_QUEST ||
				pItem->sItemInfo.SpecialItemFlag[0] == CHECK_GIVE_ITEM ||
				pItem->sItemInfo.CODE == (sinGF1|sin01) ||
				pItem->sItemInfo.CODE == (sinGF1|sin02) ||
				pItem->sItemInfo.CODE == (sinGF1|sin07) ||	// 장별 - 조사원을 찾아라
				pItem->sItemInfo.CODE == (sinGF1|sin08) ||	// 장별 - 조사원을 찾아라
				pItem->sItemInfo.CODE == (sinGF1|sin05) ||
				pItem->sItemInfo.CODE == (sinGF1|sin06) ||
				//파티복 코스튬
				( (pItem->sItemInfo.CODE == (sinDA1|sin31))		  )	|| //박재원 : 송편아뮬렛sin31과 파티복 sin31 아이템이 겹쳐서 구분함
				( (pItem->sItemInfo.CODE == (sinDA2|sin31))	      ) || //박재원 : 송편아뮬렛sin31과 파티복 sin31 아이템이 겹쳐서 구분함
				( (pItem->sItemInfo.CODE == (sinDA1|sin32))		  ) || //박재원 : 산타아뮬렛sin32과 파티복 sin32 아이템이 겹쳐서 구분함					
				( (pItem->sItemInfo.CODE == (sinDA2|sin32))		  ) || //박재원 : 산타아뮬렛sin32과 파티복 sin32 아이템이 겹쳐서 구분함
				( (pItem->sItemInfo.CODE == (sinDA1|sin33))		  ) || //박재원 : 이벤트아뮬렛sin33과 파티복 sin33 아이템이 겹쳐서 구분함					
				( (pItem->sItemInfo.CODE == (sinDA2|sin33))		  ) || //박재원 : 이벤트아뮬렛sin33과 파티복 sin33 아이템이 겹쳐서 구분함
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
				(pItem->sItemInfo.CODE == (sinDA1|sin48))   || (pItem->sItemInfo.CODE == (sinDA2|sin48))      || //한복 Kyle수정
				(pItem->sItemInfo.CODE == (sinDA1|sin54))	|| (pItem->sItemInfo.CODE == (sinDA2|sin54))	  || // 박재원 - 수영복 복장 추가 - 에이징 금지
				(pItem->sItemInfo.CODE == (sinDA1|sin55))	|| (pItem->sItemInfo.CODE == (sinDA2|sin55))	  || // 박재원 - 수영복 복장 추가 - 에이징 금지
				(pItem->sItemInfo.CODE == (sinOA2|sin31))   || (pItem->sItemInfo.CODE == (sinOA2|sin32))      || // 박재원 - 슈퍼 암릿(7일, 30일) 추가
				(pItem->sItemInfo.CODE == (sinDB1|sin31))   || (pItem->sItemInfo.CODE == (sinDB1|sin32))      ||  // 박재원 - 스피드 부츠(7일, 30일) 추가
				(pItem->sItemInfo.CODE == (sinOR2|sin33))   || (pItem->sItemInfo.CODE == (sinSP1|sin35))	  || // 장별 - 하트링, 초콜릿 에이징 금지
				(pItem->sItemInfo.CODE == (sinOA1|sin36))	||	// 장별 - 눈꽃 목걸이 에이징 금지
				(pItem->sItemInfo.CODE == (sinOA1|sin37))   || (pItem->sItemInfo.CODE == (sinSP1|sin36))	  || // 장별 - 캔디데이즈 하트아뮬렛, 캔디 에이징 금지
				(pItem->sItemInfo.CODE == (sinOA2|sin33))	|| // 장별 - 슈퍼 암릿(1일)
				(pItem->sItemInfo.CODE == (sinDB1|sin33))	|| // 장별 - 스피드 부츠(1일) 추가
				(pItem->sItemInfo.CODE == (sinSP1|sin37))   || (pItem->sItemInfo.CODE == (sinSP1|sin38))	  || // 장별 - 매지컬그린 에메랄드, 비취 에이징 금지
				(pItem->sItemInfo.CODE == (sinBI1|sin84))		|| // 장별 - 그라비티 스크롤
				(pItem->sItemInfo.CODE == (sinDB1|sin34))	|| // 장별 - 스피드 부츠(1시간) 추가
				(pItem->sItemInfo.CODE == (sinOA2|sin34))	 ||// 장별 - 슈퍼 암릿(1시간)
				(pItem->sItemInfo.CODE == (sinSP1|sin42)) // 장별 - 수박

				){
				TradeColorIndex = NOT_AGING_ITEM;
				return FALSE;
			}
			int kk =0;
			//등록된 아이템은 에이징할수없다
			for(kk = 0 ; kk < NotSet_Item_CODECnt ; kk++){
				if(NotSet_Item_CODE[kk] == pItem->sItemInfo.CODE){
					TradeColorIndex = NOT_AGING_ITEM;
					return FALSE;
				}

			}
			for(kk = 0 ; kk < NotSet_Item_MASKCnt ; kk++){
				if((pItem->sItemInfo.CODE & sinITEM_MASK2) == NotSet_Item_MASK[kk]){
					TradeColorIndex = NOT_AGING_ITEM;
					return FALSE;
				}

			}
			for(kk = 0 ; kk < NotSet_Item_KINDCnt ; kk++){
				if(NotSet_Item_KIND[kk] == pItem->sItemInfo.ItemKindCode){
					TradeColorIndex = NOT_AGING_ITEM;
					return FALSE;
				}

			}

			
			//아이템이 겹칠경우 
			if(TradeCrashItemIndex[0] = cTrade.CrashTradeItem(TradeColorRect,0,3)){  //겹치는 아이템이 있나 체크 
				TradeColorIndex = NOT_SETTING_COLOR;
				return FALSE;

			}
			//--------------------------------------------------------------------------//
			#ifdef AGING_MONEY_FREE // 박재원 - 에이징 가격 무료 이벤트

			#else
				#ifdef HASIEGE_MODE
				//공성전 세율적용 에이징 가격을 체크한다.
					if(cShop.haBuyMoneyCheck((pItem->sItemInfo.Price+(pItem->sItemInfo.Price*pItem->sItemInfo.ItemAgingNum[0]))/2)==0){
				#else
					if(sinChar->Money < (pItem->sItemInfo.Price+(pItem->sItemInfo.Price*pItem->sItemInfo.ItemAgingNum[0]))/2){	
				#endif
				//---------------------------------------------------------------------------//	
				TradeColorIndex = NOT_ENOUGH_MONEY;
				return FALSE;
			}
			#endif
			return TRUE;
		}

	}


	return TRUE;
}
/*
int CraftItemCheckDelayTime = 0;

//크래프트 아이템을 조작하는 망할 유저를 병신 만든다  (서버에서 하는관계로 의미가 없긴하나 혹시나해서 주석처리함)
int cCRAFTITEM::CheckHackCraftItem()
{
	CraftItemCheckDelayTime++;
	if(CraftItemCheckDelayTime < 70*10)return FALSE;
	CraftItemCheckDelayTime = 0;


	unsigned int AddItemCheckSum = 0;
	unsigned int CheckAddItemData = 750604968;

	for(int i=0 ;i < MAX_CRAFTITEM_INFO ; i++){
		for(int j=0; j < 8 ;j++){
			AddItemCheckSum += (int)((j+1)*sCraftItem_Info[i].AddElement[j]);
			AddItemCheckSum += (j+1)*sCraftItem_Info[i].AddItem[j];
			AddItemCheckSum += (j+1)*sCraftItem_Info[i].ElementKind[j];
			AddItemCheckSum += (j+1)*sCraftItem_Info[i].PlusElementCode[j];
			AddItemCheckSum += (j+1)*sCraftItem_Info[i].MainItemCode[j];

		}

	}
	if( CheckAddItemData != AddItemCheckSum){
		SendSetHackUser(8); //해킹을 하려고했던 못된 유저를 고발 TRUE 접속 종료 

	}
	 
	return TRUE;
}

*/

int AgingItemCheckDelayTime = 0;

//에이징 데이타의 조작여부를 조정한다 
int cAGING::CheckAgingData()
{
	/*
	#ifdef __SIN_AGING_TEST


	#else 
	AgingItemCheckDelayTime++;
	if(AgingItemCheckDelayTime < 70*10)return FALSE;
	AgingItemCheckDelayTime = 0;

	unsigned int AddItemCheckSum = 0;
	const unsigned int CheckAddItemData = 1848296; //이렇게 해놓으면 읽기 전용으로만 된다네... 음하하하 


	int i,j;
	for(i=0; i < 10 ; i++){
		AddItemCheckSum += (i+1)*AgingLevelAttack[i];
		AddItemCheckSum += (i+1)*AgingLevelCritical[i];
		AddItemCheckSum += (i+1)*AgingLevelBlock[i];
		AddItemCheckSum += (i+1)*AgingLevelHit[i];
		AddItemCheckSum += (i+1)*AgingOkPercent[i];
		for(j = 0; j < 12 ; j++){
			AddItemCheckSum += (i+1)*AgingLevelSheltom[i][j];


		}

	}
	if( CheckAddItemData != AddItemCheckSum){
		SendSetHackUser(107); //해킹을 하려고했던 못된 유저를 고발 TRUE 접속 종료 

	}
#endif 
	*/
	
	return TRUE;
}

////////////창고에 복사된아이템이 있는지를 찾는다 
int cWAREHOUSE::CopyItemNotPickUp(sITEM *pItem,int JumpIndex)
{
	int i;
	if(smConfig.DebugMode)return TRUE; //디버그 모드일때는 들수있따 크크크
	for(i=0 ; i < 100 ; i++){
		if(sWareHouse.WareHouseItem[i].Flag){
			if(JumpIndex == i)continue; //자신의 아이템은 찾지않는다
			if(CompareItems(&pItem->sItemInfo ,  &sWareHouse.WareHouseItem[i].sItemInfo)){
				CopyItemIndex7[i] = i+1;
				return FALSE;

			}

		}

	}

	return TRUE;
}

int SendServerFlag7 = 0;
//복사된 아이템이 있는지를 체크한다 
int cWAREHOUSE::CheckCopyItem()
{
	int i,j;
	if(SendServerFlag7)return FALSE;
	for(i=0 ; i < 100 ; i++){
		if(sWareHouse.WareHouseItem[i].Flag){
			for(j=0 ; j < 100 ; j++){
				if(i != j && sWareHouse.WareHouseItem[j].Flag){
					if(CompareItems(&sWareHouse.WareHouseItem[i].sItemInfo ,  &sWareHouse.WareHouseItem[j].sItemInfo)){
						SendSetHackUser2(1010,sWareHouse.WareHouseItem[i].CODE); //해킹한 유저를 신고한다
						SendServerFlag7 = 1; //한번만 체크하도록한다 
						break;
					}
				}
			}
		}
	}
	/*
	//덩달아 돈고도 체크 (음... 찝찝하다)
	if(8000000 < sWareHouse.Money-2023-1000000){
			SendSetHackUser2(1110,sWareHouse.Money-2023); //해킹한 유저를 신고한다

	}
	*/


	return TRUE;
}
int CheckCraftMoney = 0;

///////// 믹스쳐 아이템을 서버에 보낸다 
int cCRAFTITEM::sinSendCraftItem()
{

	if(MixItemNoCopyFlag)return FALSE;
	int i = 0;
	

	//복사를 체크하기위한 백업본을 만든다
	memcpy(&sCraftItemBackUp,&sCraftItem,sizeof(sCRAFTITEM)); 

	MixItemNoCopyFlag = 1; //아이템이 복사되는걸 방지하기위해 플렉을준다
	
	if(ForceFlag){ //Force Orb 이면 
		memset(&sCraftItem_Send.DesCraftItem,0,sizeof(sITEM));
		memset(&sCraftItemBackUp,0,sizeof(sCRAFTITEM)); 
		sCraftItem_Send.DesCraftItem.CODE = 1000;
		sCraftItem_Send.Money = ForceItemPrice;

	}
	//haGoon아이템 재구성 을 서버로 보낸다.---------------------------------------------------------------------------
	if(cCraftItem.iReconItemFlag){
		memset(&sCraftItem_Send.DesCraftItem,0,sizeof(sITEM));
		memset(&sCraftItemBackUp,0,sizeof(sCRAFTITEM)); 
		sCraftItem_Send.DocIndex = 2000;		

	}
	//----------------------------------------------------------------------------------------------------------------
	if(sCraftItem.CraftItem[0].Flag){
		sCraftItem_Send.DesCraftItem = sCraftItem.CraftItem[0];
		sCraftItem_Send.Money = sCraftItem.CraftItem[0].sItemInfo.Price;
	}
	for( i = 0 ; i < 12 ; i++){
		if(sCraftItem.CraftItem[i+1].Flag){
			sCraftItem_Send.SheltomCode[i] = sCraftItem.CraftItem[i+1].CODE;
			sCraftItem_Send.CheckSum[i]    = sCraftItem.CraftItem[i+1].sItemInfo.ItemHeader.dwChkSum;
			sCraftItem_Send.Head[i]        = sCraftItem.CraftItem[i+1].sItemInfo.ItemHeader.Head;
		}
	}

	// 박재원 - 리스펙 스톤도 서버로 보낸다.
	if(sCraftItem.CraftItem[14].Flag){
		sCraftItem_Send.A_StoneItemCode = sCraftItem.CraftItem[14].CODE;
		sCraftItem_Send.A_StoneCheckSum = sCraftItem.CraftItem[14].sItemInfo.ItemHeader.dwChkSum;
		sCraftItem_Send.A_StoneHead     = sCraftItem.CraftItem[14].sItemInfo.ItemHeader.Head;
	}
	
	CheckCraftMoney = sCraftItem_Send.Money;
	
	CraftItemSendServerIndex++; //인덱스를 보낸다 
	sCraftItem_Send.Index = CraftItemSendServerIndex;
	//저장된 구조체를 서버에 보낸다 
	cCraftItem.iReconItemFlag = 0;
	
	if(ForceFlag){ //포스 오브는 윙으로 보낸다
		SendWingItemToServer( &sCraftItem_Send );
	}
	else{
		SendCraftItemToServer( &sCraftItem_Send );
	}

	return TRUE;
}
///////// 에이징 아이템을 서버에 보낸다 
int cAGING::sinSendAgingItem()
{
	int i = 0; 
	//아이템을 보낼때 복사를 방지하기위한 백업본을 만든다
	memcpy(&sAgingItemBackUp,&sAgingItem,sizeof(sAGINGITEM)); 

	MixItemNoCopyFlag = 1;
	if(sAgingItem.AgingItem[0].Flag){
		sAging_Send.DesCraftItem = sAgingItem.AgingItem[0];
	}
	for( i = 0 ; i < 12 ; i++){
		if(sAgingItem.AgingItem[i+1].Flag){
			sAging_Send.SheltomCode[i] = sAgingItem.AgingItem[i+1].CODE;
			sAging_Send.CheckSum[i]    = sAgingItem.AgingItem[i+1].sItemInfo.ItemHeader.dwChkSum;
			sAging_Send.Head[i]        = sAgingItem.AgingItem[i+1].sItemInfo.ItemHeader.Head;
		}
	}
	//에이징 스톤도 서버로 보낸다.
	if(sAgingItem.AgingItem[14].Flag){
		sAging_Send.A_StoneItemCode = sAgingItem.AgingItem[14].CODE;
		sAging_Send.A_StoneCheckSum = sAgingItem.AgingItem[14].sItemInfo.ItemHeader.dwChkSum;
		sAging_Send.A_StoneHead     = sAgingItem.AgingItem[14].sItemInfo.ItemHeader.Head;
	}

#ifdef AGING_MONEY_FREE // 박재원 - 에이징 가격 무료 이벤트
	sAging_Send.Money = 0;
	CheckCraftMoney = sAging_Send.Money;
#else
	sAging_Send.Money = sAgingItem.AgingItem[0].sItemInfo.Price;
	CheckCraftMoney = sAging_Send.Money;
#endif
	
	CraftItemSendServerIndex++; //인덱스를 보낸다 
	sAging_Send.Index = CraftItemSendServerIndex;

	//저장된 구조체를 서버에 보낸다 
	SendAgingItemToServer( &sAging_Send );


	//////아이템을 보내면서 돈을 뺀다
#ifdef AGING_MONEY_FREE // 박재원 - 에이징 가격 무료 이벤트
	int Price = 0; // 무료
#else
	int Price = ((sAgingItem.AgingItem[0].sItemInfo.ItemAgingNum[0]+1)*sAgingItem.AgingItem[0].sItemInfo.Price)/2;
#endif

		
	CheckCharForm();		//인증 
	sinMinusMoney(Price,1);			
	ReformCharForm();		//재인증 
	SendSaveMoney();		//금액 조작을 못하게하기위해 호출한다 
	return TRUE;
}
///////// 서버에서 믹스쳐 아이템의 구조를 받는다 
int cCRAFTITEM::sinRecvCraftItem(sCRAFTITEM_SERVER *pCraftItem_Server)
{
	memcpy(&sCraftItem_Recv , pCraftItem_Server , sizeof(sCRAFTITEM_SERVER));
	
	return TRUE;
}

///////// 서버에서 에이징 아이템의 구조를 받는다
int cAGING::sinRecvAgingItem(sCRAFTITEM_SERVER *pCraftItem_Server)
{
	memcpy(&sAging_Recv , pCraftItem_Server , sizeof(sCRAFTITEM_SERVER));
	
	return TRUE;
}

//에이징 아이템의 결과를 서버에서 받는다 
int cAGING::sinRecvAgingItemResult(sCRAFTITEM_SERVER *pCraftItem_Server)
{
 
	//퀘스트 아이템일경우
	if(pCraftItem_Server->DesCraftItem.sItemInfo.ItemKindCode == ITEM_KIND_QUEST_WEAPON){ 
		if(pCraftItem_Server->Result){
			cInvenTory.DeleteInvenItemToServer(TempQuestItem.sItemInfo.CODE, TempQuestItem.sItemInfo.ItemHeader.Head,TempQuestItem.sItemInfo.ItemHeader.dwChkSum);
			memset(&TempQuestItem,0,sizeof(sITEM));
			//카운트 갱신
			pCraftItem_Server->DesCraftItem.sItemInfo.ItemAgingCount[0] = pCraftItem_Server->DesCraftItem.sItemInfo.ItemAgingCount[1];
			if(cInvenTory.AutoSetInvenItem(&pCraftItem_Server->DesCraftItem)){
				cMessageBox.ShowMessage(MESSAGE_QUEST_ITEM_AGING);
				sinPlaySound(SIN_SOUND_EAT_POTION2);//대박 사운드
				StartEffect(lpCurPlayer->pX, lpCurPlayer->pY, lpCurPlayer->pZ, EFFECT_AGING); //에이징이펙트
				sinQuest_ChangeJob3.Monster[0] = pCraftItem_Server->DesCraftItem.sItemInfo.ItemAgingNum[0];
				StartQuest_Code(sinQuest_ChangeJob3.CODE);

			}
			//퀘스트 메세지를 보여준다
			if(pCraftItem_Server->DesCraftItem.sItemInfo.ItemAgingNum[0] == 4){
				sinQuest_ChangeJob3.State = 4;
			}
		}
		else{
			//지웠던 아이템을 복구해준다
			cInvenTory.AutoSetInvenItem(&TempQuestItem);

		}
		memset(&sAging_Recv,0,sizeof(sCRAFTITEM_SERVER)); //받은 믹스쳐 구조체 초기화 
		memset(&sAging_Send,0,sizeof(sCRAFTITEM_SERVER)); //보낸 믹스쳐 구조체 초기화 
		NotChangeSetItemFlag = 0; //무기셋팅 체인지 제한을 초기화해준다
		return TRUE;

	}

	if(!cAging.OpenFlag){
		AgingCheckSheltomFlag = 0; //에이징 가능 플랙 초기화 
		AgingSheltomCnt2 = 0;
		MixItemNoCopyFlag = 0; //복사방지 플렉 초기화
		memset(&sCraftItem_Recv,0,sizeof(sCRAFTITEM_SERVER)); //받은 믹스쳐 구조체 초기화 
		memset(&sCraftItem_Send,0,sizeof(sCRAFTITEM_SERVER)); //보낸 믹스쳐 구조체 초기화 
		MixCancelButtonDelayFlag = 0;
		return FALSE;
	}
	
	if(pCraftItem_Server->Result){
		if((pCraftItem_Server->DesCraftItem.sItemInfo.CODE & sinITEM_MASK2) == sinQT1){
			sinSetQuestItem(&pCraftItem_Server->DesCraftItem.sItemInfo);
		}
		else{
			if(!cInvenTory.AutoSetInvenItem(&pCraftItem_Server->DesCraftItem)){
				sinThrowItemToFeild(&pCraftItem_Server->DesCraftItem);

			}
			CheckCharForm(); //인증
			switch(pCraftItem_Server->Result){
				case 1: //에이징 -1
					cMessageBox.ShowMessage(MESSAGE_AGING_LEVEL_DOWN1);
					sinPlaySound(SIN_SOUND_FAILD_MIXITEM); //실패 사운드 
				break;
				case 2: //에이징 -2
					cMessageBox.ShowMessage(MESSAGE_AGING_LEVEL_DOWN2);
					sinPlaySound(SIN_SOUND_FAILD_MIXITEM); //실패 사운드 
				break;
				case 3: //성공
					cMessageBox.ShowMessage(MESSAGE_AGING_LEVEL_PLUS);
					sinPlaySound(SIN_SOUND_EAT_POTION2);//대박 사운드
				break;
				case 4: //2배 
					cMessageBox.ShowMessage(MESSAGE_AGING_LEVEL_PLUS2);
					sinPlaySound(SIN_SOUND_EAT_POTION2);//대박 사운드
				break;
			}
			SendSaveMoney(); //금액 조작을 못하게하기위해 호출한다 
			ReformCharForm(); //재인증 
		}
	}
	else{
		sinPlaySound(SIN_SOUND_FAILD_MIXITEM); //실패 사운드 
		cMessageBox.ShowMessage(MESSAGE_FAILD_AGING_ITEM);

	}

	for(int y=0;y<4;y++){ //쉘텀 인덱스 초기화 
		for(int h= 0 ;h < 3  ;h ++){
			AgingLevelSheltomIndex[y][h] = 0;
		}

	}

	for(int i=0;i<15;i++){ //에이징 아이템을 날린다
		if(sAgingItemBackUp.AgingItem[i].Flag){
			cInvenTory.DeleteInvenItemToServer(sAgingItemBackUp.AgingItem[i].sItemInfo.CODE,
				sAgingItemBackUp.AgingItem[i].sItemInfo.ItemHeader.Head,
				sAgingItemBackUp.AgingItem[i].sItemInfo.ItemHeader.dwChkSum);
		}

	}


	memset(&sAgingItem,0,sizeof(sAGINGITEM));
	memset(&sAgingItemBackUp,0,sizeof(sAGINGITEM));
	cAging.OpenFlag = 0; //에이징 인터페이스를 닫는다 
	AgingCheckSheltomFlag = 0; //에이징 가능 플랙 초기화 
	AgingSheltomCnt2 = 0;
	MixItemNoCopyFlag = 0; //복사방지 플렉 초기화
	MixCancelButtonDelayFlag = 0;
	memset(&sAging_Recv,0,sizeof(sCRAFTITEM_SERVER)); //받은 믹스쳐 구조체 초기화 
	memset(&sAging_Send,0,sizeof(sCRAFTITEM_SERVER)); //보낸 믹스쳐 구조체 초기화 
	cInvenTory.CheckDamage(); //최대데미지가 나올 무기로 계산하여 서버로 보내준다 
	return TRUE;
}

//크래프트 아이템의 결과물을 서버에서 받는다 
int cCRAFTITEM::sinRecvCraftItemResult(sCRAFTITEM_SERVER *pCraftItem_Server)
{
	CheckMixItemCopyFlag = 0; //복사방지 플렉  
	if(!cCraftItem.OpenFlag){
		MixItemNoCopyFlag = 0; //복사방지 플렉 초기화
		memset(&sCraftItem,0,sizeof(sCRAFTITEM)); //구조체 초기화 
		memset(&sCraftItemBackUp,0,sizeof(sCRAFTITEM)); //구조체 초기화 
		memset(&sCraftItem_Recv,0,sizeof(sCRAFTITEM_SERVER)); //받은 믹스쳐 구조체 초기화 
		memset(&sCraftItem_Send,0,sizeof(sCRAFTITEM_SERVER)); //보낸 믹스쳐 구조체 초기화 
		MixCancelButtonDelayFlag = 0;
		return FALSE;

	}

	int i , LineCount=0 ,Temp;

	/*   돈관련 부분은 뺀다 
	if(CheckCraftMoney != pCraftItem_Server->Money) //돈이 맞지 않으면 실패로 간주한다 
		pCraftItem_Server->Result = 0;
	*/

	if(pCraftItem_Server->Result){
		if(pCraftItem_Server->DocIndex == 2000){ //아이템 재구성시스템 
			//아이템일 경우 
			sITEM TempItem;
			if(LoadItemImage(&pCraftItem_Server->DesCraftItem.sItemInfo , &TempItem)){
				if(cInvenTory.CheckRequireItemToSet(&TempItem)){         //곧바로 셋팅될수있는지를 체크한다  
					//haGoon수정================================================================================
					if(!cInvenTory.AutoSetInvenItem(&TempItem,1)){
						sinThrowItemToFeild(&pCraftItem_Server->DesCraftItem);
						memset(&sCraftItem,0,sizeof(sCRAFTITEM)); //구조체 초기화 
						cCraftItem.OpenFlag =0;
						return FALSE;
					}
					//===========================================================================================
				}
				
			}
			cMessageBox.ShowMessage(MESSAGE_RECONITEM_OK);
		}
		else{
			ShowItemCraftMessageFlag = 1; //아이템이 조합됬을때 메세지를 보여준다 
			lstrcpy(szCraftItemBuff,sCraftItem_Info[pCraftItem_Server->DocIndex].Doc); //아이템 정보를 보여준다 
			
			Temp = lstrlen(szCraftItemBuff);
			for( i = 0 ; i < Temp ; i++){
				if(szCraftItemBuff[i] == '\r'){
					LineCount++;
				}
			}

			CraftItemMessageSize.x = 14;           //박스 사이즈 
			CraftItemMessageSize.y = ((LineCount)*2)+2;  //박스 사이즈 

			
			sinAgingSuccessFlag2 = 1;

			//CheckCharForm(); //인증 
			//sinChar->Money -= sCraftItem_Recv.DesCraftItem.sItemInfo.Price;
			//sinMinusMoney(pCraftItem_Server->DesCraftItem.sItemInfo.Price,1);
			//ReformCharForm(); //재인증 
			//SendSaveMoney(); //금액 조작을 못하게하기위해 호출한다 
			//실패든 성공이든간에 아이템을 원상 복귀 시킨다 
			//cCraftItem.OpenFlag = 0; //창을 닫는다
			//cInvenTory.AutoSetInvenItem(&pCraftItem_Server->DesCraftItem); //아이템을 셋팅한다 
			
			//haGoon수정 ============================================================================
			if(!cInvenTory.AutoSetInvenItem(&pCraftItem_Server->DesCraftItem)){
				sinThrowItemToFeild(&pCraftItem_Server->DesCraftItem);
			}
			//=======================================================================================

			//무기가 복사되었으면 무기를 지운다
			if(sCraftItemBackUp.CraftItem[0].Flag){
				cInvenTory.DeleteInvenItemToServer(sCraftItemBackUp.CraftItem[0].sItemInfo.CODE,
					sCraftItemBackUp.CraftItem[0].sItemInfo.ItemHeader.Head,
					sCraftItemBackUp.CraftItem[0].sItemInfo.ItemHeader.dwChkSum);
			}

			/*
			for(int y=1;y<15;y++){ //날라간 쉘텀을 서버에 알려준다 
				if(sCraftItem.CraftItem[y].Flag)
					SendSaveThrowItem(&sCraftItem.CraftItem[y].sItemInfo);

			}
			*/
		}
		cCraftItem.OpenFlag = 0; //창을 닫는다
		CheckCharForm(); //인증 
		sinMinusMoney(sCraftItem.CraftItem[0].sItemInfo.Price,1);
		ReformCharForm(); //재인증 
		SendSaveMoney(); //금액 조작을 못하게하기위해 호출한다 
		SaveGameData();
	}
	else{


		//haGoon수정============================================================================
		if(!cInvenTory.AutoSetInvenItem(&sCraftItem.CraftItem[0])){ //아이템을 셋팅한다 
			sinThrowItemToFeild(&sCraftItem.CraftItem[0]);
		}
		//=======================================================================================
		/*
		for(int y=1;y<15;y++){ //날라간 쉘텀을 서버에 알려준다 
			if(sCraftItem.CraftItem[y].Flag)
				SendSaveThrowItem(&sCraftItem.CraftItem[y].sItemInfo);

		}
		*/
	
		//믹스쳐 할때 돈을 뺀다 
		CheckCharForm(); //인증 
		sinMinusMoney(sCraftItem.CraftItem[0].sItemInfo.Price,1);
		ReformCharForm(); //재인증 
		SendSaveMoney(); //금액 조작을 못하게하기위해 호출한다 

		cCraftItem.OpenFlag = 0; //창을 닫는다 
		cMessageBox.ShowMessage(MESSAGE_FAILD_CRAFT_ITEM);
		sinPlaySound(SIN_SOUND_FAILD_MIXITEM);
		//memset(&sCraftItem_Recv.DesCraftItem,&TempItem,sizeof(sITEM)); //아이템 복구 

	}

	//복사가되었거나한 아이템을 지운다 (쉘텀만지운다)
	for( i=1; i < 15 ; i++){
		if(sCraftItemBackUp.CraftItem[i].Flag){
			cInvenTory.DeleteInvenItemToServer(sCraftItemBackUp.CraftItem[i].sItemInfo.CODE,
				sCraftItemBackUp.CraftItem[i].sItemInfo.ItemHeader.Head,
				sCraftItemBackUp.CraftItem[i].sItemInfo.ItemHeader.dwChkSum);
		}

	}
	memset(&sCraftItem,0,sizeof(sCRAFTITEM)); //구조체 초기화 
	memset(&sCraftItemBackUp,0,sizeof(sCRAFTITEM)); //구조체 초기화 
	memset(&sCraftItem_Recv,0,sizeof(sCRAFTITEM_SERVER)); //받은 믹스쳐 구조체 초기화 
	memset(&sCraftItem_Send,0,sizeof(sCRAFTITEM_SERVER)); //보낸 믹스쳐 구조체 초기화 

	MixItemNoCopyFlag = 0; //복사방지 플렉 초기화
	MixCancelButtonDelayFlag = 0;
	return TRUE;
}

////////복사 아이템을 없애준다 
int cWAREHOUSE::DeleteCopyItem()
{
	int i,j;
	for(i=0 ; i < 100 ; i++){
		if(sWareHouse.WareHouseItem[i].Flag){
			for(j=0 ; j < 100 ; j++){
				if(i != j && sWareHouse.WareHouseItem[j].Flag){
					if(CompareItems(&sWareHouse.WareHouseItem[i].sItemInfo ,  &sWareHouse.WareHouseItem[j].sItemInfo)){
						//SendSetHackUser2(1010,sWareHouse.WareHouseItem[i].CODE); //해킹한 유저를 신고한다
						//SendServerFlag7 = 1; //한번만 체크하도록한다 
						sWareHouse.WareHouseItem[j].Flag = 0; //복사 아이템을 날린다 
						cWareHouse.ReFormWareHouse();
						break;
					}
				}
			}
		}
	}	
	return TRUE;
}

////////믹스에서 복사된 아이템을 없애준다 
int CopyMixItemCheckDelete(sITEM *pItem)
{
	for(int i=0 ; i<INVENTORY_MAXITEM ; i++){
		if(cInvenTory.InvenItem[i].Flag){
			if(CompareItems(&cInvenTory.InvenItem[i].sItemInfo,&pItem->sItemInfo)){
				cInvenTory.InvenItem[i].Flag = 0; //복사된 아이템을 지운다 
			}
		}
		if(cInvenTory.InvenItemTemp[i].Flag){
			if(CompareItems(&cInvenTory.InvenItemTemp[i].sItemInfo,&pItem->sItemInfo)){
				cInvenTory.InvenItemTemp[i].Flag = 0; //복사된 아이템을 지운다 
			}
		}
	}
	return TRUE;
}

//Make할 아이템이 맞는가 코드를 확인한다
int sinMakeItemCheck()
{
	int ResultCount =0;
	int ResultCountCheck =0;
	DWORD TempCODE[15] = {0,};
	int  i = 0;
	//코드 복사 
	for(i = 0 ; i < MAX_MAKEITEM_INFO ; i++){  
		for(int p = 0 ; p < 15 ; p++){  //코드를 한번씩 초기화해준다 
			if(sAgingItem.AgingItem[p].Flag)
				TempCODE[p] = sAgingItem.AgingItem[p].CODE;
		}

		for(int j = 0 ; j < 15; j++){
			if(sMakeItem_Info[i].CODE[j]){
				ResultCount++;
				for(int k = 0; k < 15 ; k++){
					if(sAgingItem.AgingItem[k].Flag && TempCODE[k]){
						if(TempCODE[k] == sMakeItem_Info[i].CODE[j] ){ 
							ResultCountCheck++;
							TempCODE[k] = -1;
						}
					}
				}
			}
		}
		if(ResultCount && ResultCountCheck && (ResultCount == ResultCountCheck)){
			AgingCheckSheltomFlag = 1;
			return TRUE;
		}
		else AgingCheckSheltomFlag = 0;
		ResultCount = 0;
		ResultCountCheck = 0;

	}

	return TRUE;
}
/*****************************************************************************/
/*							PostBox 시스템
/*****************************************************************************/
int sinPosBoxNpc()
{ 
	//전혀 하는일 없는 백수 함수 -_-;
	//초기화
	//memset(sPostbox_Item,0,sizeof(sPOSTBOX_ITEM));
	//PostBoxIndexCnt = 0; 
	
	//메세지를 띄운다 
	HelpBoxKindIndex = SIN_POST_BOX; 
	cSinHelp.sinShowHelp(SIN_HELP_KIND_POSTBOX,QuestMessageBoxPosi2.x,QuestMessageBoxPosi2.y,QuestMessageBoxSize2.x,QuestMessageBoxSize2.y, RGBA_MAKE(0,15,128,125),PostBoxDocFilePath[0]);  
	cInvenTory.OpenFlag = 1;
	cInterFace.CheckAllBox(SIN_INVENTORY); //인벤토리 창을 띄운다 
	return TRUE;
}
//haGoon아이템 재구성을 체크한다.----------------------------------------------------------------------------
int cCRAFTITEM::haCheckReconItem()
{
	int cnt[MAX_SEEL_COUNT]={0}; // 박재원 - 테이와즈 씰 추가 (3종류 -> 4종류)
	int CheckNotReconItem=0;
	int CheckBlockCount=0;
	cCraftItem.iReconItemFlag = 0;

	//씰아이템이 아닌것은 다 체크한다.
	for(int k=1;k<14;k++){ // 박재원 - 리스펙 스톤(sCraftItem.CraftItem[14]는 리스펙 스톤이므로 체크에서 제외한다) for(int k=1;k<15;k++) -> for(int k=1;k<14;k++)
		if(sCraftItem.CraftItem[k].Flag){
			if((sCraftItem.CraftItem[k].CODE & sinITEM_MASK2)  == sinSE1)
				CheckBlockCount++;   //씰 아이템 체크
			else
				CheckNotReconItem++; //씰외아이템 체크
		}
		else
			memset(&sCraftItem.CraftItem[k],0,sizeof(sITEM));  //서버 해킹 에러 때문에 임시 방편이다.
	}
	//씰을 체크한다.
	for(int i=1;i<14;i++){ // 박재원 - 리스펙 스톤(sCraftItem.CraftItem[14]는 리스펙 스톤이므로 체크에서 제외한다) for(int k=1;k<15;k++) -> for(int k=1;k<14;k++)
		if((sCraftItem.CraftItem[i].CODE & sinITEM_MASK2)  == sinSE1 && sCraftItem.CraftItem[i].Flag){
			for(int k=0;k<MAX_SEEL_COUNT;k++){ // 박재원 - 테이와즈 씰 추가 (3종류 -> 4종류)
				if(sCraftItem.CraftItem[i].CODE==sReconItem[k].dwCODE){
					cnt[k]++;
					if(sReconItem[k].iCount == cnt[k] && CheckNotReconItem < 1 && CheckBlockCount == cnt[k]){
						cCraftItem.iReconItemFlag = 1;
						return TRUE;
					}
				}
			}
		}	
	}
	return FALSE;
}
/*----------------------------------------------------------------------------*
*Desc: 빈배열을 당겨준다.
*-----------------------------------------------------------------------------*/
int cCRAFTITEM::haCraftSortItem(sCRAFTITEM *pCraftItem)
{
	//빈 아이템 배열을 당겨준다.
	int i;
	for(i=1;i<13;i++){
		if(!pCraftItem->CraftItem[i].Flag){
			pCraftItem->CraftItem[i] = pCraftItem->CraftItem[i+1];
			pCraftItem->CraftItem[i+1].Flag = 0;
		}
	}
	return TRUE;
}
/*----------------------------------------------------------------------------*
*Desc: 인벤토리의 22*22크기의 빈칸을 구해 리턴한다.   		
*-----------------------------------------------------------------------------*/
int cCRAFTITEM::GetCraftCheckEmptyArea()
{
	int cntx,cnty;
	int cx,cy;

	RECT	rect;
	int		flag;
	int StartX = 23;  //인벤 박스의 시작점 X
	int StartY = 436; // Y

	cx = (22*INVENTORY_BOX_ROW)-22; //아이템이 들어갈수있는 가장 끝 X좌표 
	cy = (22*INVENTORY_BOX_COL)-22; //아이템이 들어갈수있는 가장 끝 Y좌표

	int count = 0;
	//코드가 같지 않거나 아이템이 비어있지 않은 곳에 셋팅할때 검색해서 위치를 찾아낸다 
	for( cntx=StartX ; cntx <= StartX+cx ; cntx+=22){
		for( cnty=StartY ; cnty <= StartY+cy ; cnty+=22){ 
			flag = 0;
			for(int i=0 ; i<INVENTORY_MAXITEM ; i++){
				if(cInvenTory.InvenItem[i].Flag){
					rect.left   = cInvenTory.InvenItem[i].x;
					rect.right  = cInvenTory.InvenItem[i].x+cInvenTory.InvenItem[i].w;
					rect.top    = cInvenTory.InvenItem[i].y;
					rect.bottom = cInvenTory.InvenItem[i].y+cInvenTory.InvenItem[i].h;

					if( cntx > rect.left-22 && cntx< rect.right && 
						cnty > rect.top-22 && cnty< rect.bottom )
						flag++;
				}
			}
			if ( flag==0 ) {
				count++;
			}
		}
	}
	if( count )
		return count;
	else
		return 0;
}

// 석지용 - 믹스쳐 리셋 아이템이 들어갈 공간을 체크하는 함수 구현
int cMIXTURERESET::SetMixtureItemResetAreaCheck(sITEM *pItem)
{
	TradeColorIndex = 0;			// 초기화
	TradeCrashItemIndex[0] = 0;		// 충돌된 아이템 Index 초기화 
	cTrade.InitTradeColorRect();	// 영역 초기화

	if( (pItem->CODE & sinITEM_MASK2) == sinBI1 )		// 빌링 아이템이고...
	{	
		// 믹스쳐 리셋 스톤이 아니면 리턴
		if( (pItem->sItemInfo.CODE & sinITEM_MASK3) != sin89 )
			return FALSE;

		// 스톤이 위치할 좌표를 계산해 낸다.
		TradeStartX = AgingLevelSheltomXY[0][0].x + 23;
		TradeStartY = AgingLevelSheltomXY[0][0].y + 31;
		TradeEndX = TradeStartX+22;
		TradeEndY = TradeStartY+22;

		int i, j;
		for( i=pItem->x+11; i<pItem->x+pItem->w; i+=22 )
		{
			for( j=pItem->y+11; j<pItem->y+pItem->h; j+=22 )
			{
				if(TradeStartX <= i && TradeEndX > i && TradeStartY <=j && TradeEndY > j)
				{ 
					//인벤토리 박스에 들어갔는지를 체크 
					TradeColorRect.left =  TradeStartX + ( ( ( i - TradeStartX ) / 22 ) * 22 ) ; 
					TradeColorRect.top  =  TradeStartY + ( ( ( j - TradeStartY ) / 22 ) * 22 ) ; 
					TradeColorRect.right  = pItem->w;
					TradeColorRect.bottom = pItem->h;

					if(TradeEndX < (TradeColorRect.left + TradeColorRect.right)-11 || TradeEndY < (TradeColorRect.top + TradeColorRect.bottom)-11)
					{
						cTrade.InitTradeColorRect(); //컬러 박스 RECT 초기화 (컬러 박스 RECT중 하나라두 0 이면 셋팅할수 없다 
						return FALSE;
					}

					// 아이템의 위치를 지정한다.
					pItem->SetX = TradeColorRect.left;
					pItem->SetY = TradeColorRect.top;
					pItem->ItemPosition = 0; //배열의 인덱스를 저장한다

					TradeColorIndex = SET_ITEM_CHECK_COLOR;		//셋팅할영역

					// 겹치는 아이템이 있나 체크 
					if(TradeCrashItemIndex[0] = cTrade.CrashTradeItem(TradeColorRect,0,6))
					{  
						TradeColorIndex = NOT_SETTING_COLOR;
						return FALSE;

					}

					return TRUE;
				}
			}
		}
	}
	else
	{
		if( pItem->sItemInfo.ItemKindCode == ITEM_KIND_CRAFT )
		{
			TradeColorIndex= 0;				//초기화   (트레이드 창과 창고가 같이 쓴다)
			TradeCrashItemIndex[0] = 0;		//충돌된 아이템 Index 초기화 
			cTrade.InitTradeColorRect();	//초기화

			// 믹스쳐가 수행된 아이템이 들어갈 자리의 좌표를 구함
			TradeStartX = 58;
			TradeStartY = 193+sinInterHeight2;
			TradeEndX = TradeStartX+(3*22);
			TradeEndY = TradeStartY+(4*22);

			//인벤토리 박스에 들어갔는지를 체크 
			if(TradeStartX <= pCursorPos.x && TradeEndX > pCursorPos.x && TradeStartY <=pCursorPos.y && TradeEndY > pCursorPos.y)
			{
				TradeColorRect.left =  TradeStartX;
				TradeColorRect.top  =  TradeStartY;
				TradeColorRect.right  = TradeEndX - TradeStartX;
				TradeColorRect.bottom = TradeEndY - TradeStartY;


				pItem->SetX = TradeStartX+((TradeEndX - TradeStartX)-pItem->w)/2;
				pItem->SetY = TradeStartY+((TradeEndY - TradeStartY)-pItem->h)/2;
				pItem->ItemPosition = 0; //배열의 인덱스를 저장한다 
				TradeColorIndex = SET_ITEM_CHECK_COLOR; //셋팅할영역

				//아이템이 겹칠경우 
				if(TradeCrashItemIndex[0] = cTrade.CrashTradeItem( TradeColorRect, 0, 6))
				{
					TradeColorIndex = NOT_SETTING_COLOR;
					return FALSE;

				}

				return TRUE;
			}
		}
		else
		{
			TradeStartX = 58;
			TradeStartY = 193+sinInterHeight2;
			TradeEndX = TradeStartX+(3*22);
			TradeEndY = TradeStartY+(4*22);
			if(TradeStartX <= pCursorPos.x && TradeEndX > pCursorPos.x && TradeStartY <=pCursorPos.y && TradeEndY > pCursorPos.y)
			{ //인벤토리 박스에 들어갔는지를 체크 
				TradeColorRect.left =  TradeStartX;
				TradeColorRect.top  =  TradeStartY;
				TradeColorRect.right  = TradeEndX - TradeStartX;
				TradeColorRect.bottom = TradeEndY - TradeStartY;


				pItem->SetX = TradeStartX+((TradeEndX - TradeStartX)-pItem->w)/2;
				pItem->SetY = TradeStartY+((TradeEndY - TradeStartY)-pItem->h)/2;
				pItem->ItemPosition = 0; //배열의 인덱스를 저장한다 
				TradeColorIndex = SET_ITEM_CHECK_COLOR; //셋팅할영역

				if(	pItem->sItemInfo.ItemKindCode != ITEM_KIND_CRAFT )
				{
					TradeColorIndex = NOT_SETTING_COLOR;
					return FALSE;
				}

			}
			//	TradeColorIndex = NOT_SETTING_COLOR;
			//	return FALSE;
		}
	}	

	return TRUE;
}

// 석지용 - 믹스쳐 리셋 아이템의 정보 체크
int cMIXTURERESET::CheckMixtureResetItemForm()
{
	int TempCheckDataSum = 0;

	for( int i = 0; i < 2; i++ )
	{
		if( sMixtureResetItem.MixtureResetItem[i].Flag )
		{
			TempCheckDataSum += (i+1) * sMixtureResetItem.MixtureResetItem[i].x;
			TempCheckDataSum += (i+1) * sMixtureResetItem.MixtureResetItem[i].y;
			TempCheckDataSum += sMixtureResetItem.MixtureResetItem[i].sItemInfo.ItemHeader.dwChkSum;
			TempCheckDataSum += sMixtureResetItem.MixtureResetItem[i].sItemInfo.ItemHeader.Head;

		}
	}
	return TRUE;
}

// 석지용 - 믹스쳐 리셋 아이템의 정보 다시 체크
int cMIXTURERESET::ReformMixtureResetItem()
{
	MixtureResetItemCheckSum = 0;

	for( int i = 0; i < 2; i++ )
	{
		if( sMixtureResetItem.MixtureResetItem[i].Flag )
		{
			MixtureResetItemCheckSum += (i+1) * sMixtureResetItem.MixtureResetItem[i].x;
			MixtureResetItemCheckSum += (i+1) * sMixtureResetItem.MixtureResetItem[i].y;
			MixtureResetItemCheckSum += sMixtureResetItem.MixtureResetItem[i].sItemInfo.ItemHeader.dwChkSum;
			MixtureResetItemCheckSum += sMixtureResetItem.MixtureResetItem[i].sItemInfo.ItemHeader.Head;

		}
	}

	return TRUE;
}

// 석지용 - 믹스쳐 리셋 아이템을 집는 함수
int cMIXTURERESET::PickUpMixtureResetItem(int x, int y, int PickUpFlag, int Kind)
{
	int i=0;
	SelectTradeItemIndex = 0;    //집을 아이템 

	if(Kind == 0)
	{
		for(i=0 ; i < 2 ; i++)
		{
			if( sMixtureResetItem.MixtureResetItem[i].Flag )
			{
				if( sMixtureResetItem.MixtureResetItem[i].x < x  && sMixtureResetItem.MixtureResetItem[i].x + sMixtureResetItem.MixtureResetItem[i].w > x  && 
					sMixtureResetItem.MixtureResetItem[i].y < y  && sMixtureResetItem.MixtureResetItem[i].y + sMixtureResetItem.MixtureResetItem[i].h > y )
				{
					if(PickUpFlag)
					{
						if( i == 1)
							MixtureResetCheckFlag = 0;	// 조합 버튼 비활성화

						// 스톤이 있는 상태에선 믹스쳐 된 아이템 들기 불가
						if( sMixtureResetItem.MixtureResetItem[1].Flag && i == 0 )
						{
							cMessageBox.ShowMessage(MESSAGE_NOT_MIXTURE_RESET_ITEM);
							return FALSE;
						}	

						CheckMixtureResetItemForm();
						memcpy( &MouseItem, &sMixtureResetItem.MixtureResetItem[i], sizeof(sITEM) );
						sMixtureResetItem.MixtureResetItem[i].Flag = 0;
						memset( &sMixtureResetItem.MixtureResetItem[i], 0, sizeof(sITEM) );  //서버 해킹 에러 때문에 임시 방편이다.

						sinPlaySound( sMixtureResetItem.MixtureResetItem[i].SoundIndex );
						TradeColorIndex = 0;
						ReformMixtureResetItem();

						return TRUE;
					}
					else
					{
						SelectTradeItemIndex = i+1;
						TradeColorIndex = SELECT_ITEM_COLOR;
						sinShowItemInfoFlag = 1; //아이템의 정보를 보여준다 
						cItem.ShowItemInfo( &sMixtureResetItem.MixtureResetItem[i], 2 );
					}
				}
			}
		}
	}

	return FALSE;	
}

// 석지용 - 믹스쳐 리셋 아이템을 창에 위치시키도록 하는 함수
int cMIXTURERESET::LastSetMixtureResetItem(sITEM *pItem, int kind)
{
	//CheckMixtureResetItemForm();

	if( kind == 1 )
	{
		// 믹스쳐된 아이템은 올려져 있고, 스톤은 놓여있지 않다면...
		if( sMixtureResetItem.MixtureResetItem[0].Flag && !sMixtureResetItem.MixtureResetItem[1].Flag )
		{
			memcpy( &sMixtureResetItem.MixtureResetItem[1], pItem, sizeof(sITEM));
			sMixtureResetItem.MixtureResetItem[1].x = pItem->SetX;
			sMixtureResetItem.MixtureResetItem[1].y = pItem->SetY;
			pItem->Flag = 0; //초기화 
			sinPlaySound( sMixtureResetItem.MixtureResetItem[1].SoundIndex );
			//ReformMixtureResetItem();
			return TRUE;
		}
	}
	else if( kind == 2 )		// 믹스쳐된 아이템이면...
	{
		// 믹스쳐된 아이템이 올려져 있지 않다면...
		if( !sMixtureResetItem.MixtureResetItem[0].Flag )
		{
			memcpy( &sMixtureResetItem.MixtureResetItem[0], pItem, sizeof(sITEM));
			sMixtureResetItem.MixtureResetItem[0].x = pItem->SetX;
			sMixtureResetItem.MixtureResetItem[0].y = pItem->SetY;
			pItem->Flag = 0; //초기화 
			sinPlaySound( sMixtureResetItem.MixtureResetItem[0].SoundIndex );
			//ReformMixtureResetItem();
			return TRUE;
		}
	}

	return TRUE;
}

// 석지용 - 믹스쳐 리셋을 취소한다.
int cMIXTURERESET::CancelMixtureItemReset(int Flag)
{
	// 복사 방지 플랙이 활성화 되어 있다면....
	if( MixItemNoCopyFlag )
		return FALSE;

	// 믹스쳐 리셋에 올렸던 아이템이 있다면 인벤의 빈 공간으로 자동 셋팅하고, 지운다.
	if( sMixtureResetItem.MixtureResetItem[0].Flag )
	{
		if( !cInvenTory.AutoSetInvenItem( &sMixtureResetItem.MixtureResetItem[0] ) )
			sinThrowItemToFeild( &sMixtureResetItem.MixtureResetItem[0] );

		sMixtureResetItem.MixtureResetItem[0].Flag = 0;
	}

	MixtureResetCheckFlag = 0;	// 믹스쳐 리셋 불가능

	// 믹스쳐 리셋 창을 닫는다.
	if( !Flag )
		cMixtureReset.OpenFlag = 0;

	ResetInvenItemCode();	// 인벤 아이템 코드 초기화

	return TRUE;
}

// 석지용 - 믹스쳐 리셋될 아이템을 서버로 전송한다.
int cMIXTURERESET::sinSendMixtureItemReset()
{
	if( MixItemNoCopyFlag )
		return FALSE;

	memcpy( &sMixtureResetItemBackUp, &sMixtureResetItem, sizeof(sMIXTURE_RESET_ITEM) );	// 복사를 체크하기위한 백업본을 만든다
	MixItemNoCopyFlag = 1;	//아이템이 복사되는걸 방지하기위해 플렉을준다
	memset( &sMixtureReset_Send, 0, sizeof(sMIXTURE_RESET_ITEM_SERVER) );	// 서버에 보낼 구조체 초기화

	// 믹스쳐 리셋 시킬 아이템을 저장
	if( sMixtureResetItem.MixtureResetItem[0].Flag )
		sMixtureReset_Send.DesMixtureResetItem = sMixtureResetItem.MixtureResetItem[0];

	// 믹스쳐 리셋 스톤을 저장
	if( sMixtureResetItem.MixtureResetItem[1].Flag )
	{
		sMixtureReset_Send.dwMixtureResetStoneItemCode	= sMixtureResetItem.MixtureResetItem[1].CODE;
		sMixtureReset_Send.dwMixtureResetStoneCheckSum	= sMixtureResetItem.MixtureResetItem[1].sItemInfo.ItemHeader.dwChkSum;
		sMixtureReset_Send.dwMixtureResetStoneHead		= sMixtureResetItem.MixtureResetItem[1].sItemInfo.ItemHeader.Head;
	}

	// 인덱스를 보낸다 
	CraftItemSendServerIndex++; 
	sMixtureReset_Send.Index = CraftItemSendServerIndex;

	// 저장된 구조체를 서버에 보낸다 
	SendMixtureResetItemToServer( &sMixtureReset_Send );


	return TRUE;
}

// 석지용 - 믹스쳐 리셋된 아이템을 Recv용 변수로 복사
int	cMIXTURERESET::sinRecvMixtureItemReset(sMIXTURE_RESET_ITEM_SERVER *pMixtureResetItem)
{
	memcpy(&sMixtureReset_Recv ,pMixtureResetItem, sizeof(sMIXTURE_RESET_ITEM_SERVER));

	return TRUE;
}

// 석지용 - 믹스쳐 리셋 아이템 최종 결과 처리
int cMIXTURERESET::sinRecvMixtureItemResetResult(sMIXTURE_RESET_ITEM_SERVER *pMixtureResetItem_Server)
{
	CheckMixItemCopyFlag = 0; //복사방지 플렉  

	// 믹스쳐리셋 창이 열려있지 않다면 모두 초기화
	if( !cMixtureReset.OpenFlag )
	{
		MixItemNoCopyFlag = 0; //복사방지 플렉 초기화
		memset(&sMixtureResetItem, 0, sizeof(sMIXTURE_RESET_ITEM));			//구조체 초기화 
		memset(&sMixtureResetItemBackUp, 0, sizeof(sMIXTURE_RESET_ITEM));	//구조체 초기화 
		memset(&sMixtureReset_Recv, 0, sizeof(sMIXTURE_RESET_ITEM_SERVER)); //받은 믹스쳐리셋 구조체 초기화 
		memset(&sMixtureReset_Send, 0, sizeof(sMIXTURE_RESET_ITEM_SERVER)); //보낸 믹스쳐리셋 구조체 초기화 
		MixCancelButtonDelayFlag = 0;

		return FALSE;
	}

	int i, LineCount=0, Temp;

	if( pMixtureResetItem_Server->Result )
	{
		sITEM TempItem;
		if( LoadItemImage( &pMixtureResetItem_Server->DesMixtureResetItem.sItemInfo, &TempItem ) )
		{
			lstrcpy( szCraftItemBuff, pMixtureResetItem_Server->DesMixtureResetItem.sItemInfo.ItemName );
			strcat(szCraftItemBuff, "\r");

			Temp = lstrlen(szCraftItemBuff);
			for( i = 0 ; i < Temp ; i++)
			{
				if(szCraftItemBuff[i] == '\r')
				{
					LineCount++;
				}
			}

			CraftItemMessageSize.x = 14;				//박스 사이즈 
			CraftItemMessageSize.y = ((LineCount)*2)+2; //박스 사이즈 

			if( cInvenTory.CheckRequireItemToSet( &TempItem ) )
			{
				//곧바로 셋팅될수있는지를 체크한다  
				if( !cInvenTory.AutoSetInvenItem( &TempItem, 1 ) )
				{
					sinThrowItemToFeild( &pMixtureResetItem_Server->DesMixtureResetItem );
				}
			}

		}

		cMessageBox.ShowMessage( MESSAGE_OK_MIXTURE_RESET_ITEM );

		CheckCharForm();	//인증 
		ReformCharForm();	//재인증 
		SendSaveMoney();	//금액 조작을 못하게하기위해 호출한다 
		SaveGameData();
		cMixtureReset.OpenFlag = 0; //창을 닫는다
	}
	else
	{
		CheckCharForm();	//인증 
		ReformCharForm();	//재인증 
		SendSaveMoney();	//금액 조작을 못하게하기위해 호출한다 
		cMixtureReset.OpenFlag = 0; //창을 닫는다
		cMessageBox.ShowMessage( MESSAGE_FAIL_MIXTURE_RESET_ITEM );
	}

	//복사가되었거나한 아이템을 지운다
	for( i=0; i<2 ; i++)
	{
		if( sMixtureResetItemBackUp.MixtureResetItem[i].Flag )
		{
			cInvenTory.DeleteInvenItemToServer( sMixtureResetItemBackUp.MixtureResetItem[i].sItemInfo.CODE,
				sMixtureResetItemBackUp.MixtureResetItem[i].sItemInfo.ItemHeader.Head,
				sMixtureResetItemBackUp.MixtureResetItem[i].sItemInfo.ItemHeader.dwChkSum);
		}

	}

	memset( &sMixtureResetItem, 0, sizeof(sMIXTURE_RESET_ITEM));			//구조체 초기화 
	memset( &sMixtureResetItemBackUp, 0, sizeof(sMIXTURE_RESET_ITEM));		//구조체 초기화 
	memset( &sMixtureReset_Recv, 0, sizeof(sMIXTURE_RESET_ITEM_SERVER));	//받은 믹스쳐리셋 구조체 초기화 
	memset( &sMixtureReset_Send, 0, sizeof(sMIXTURE_RESET_ITEM_SERVER));	//보낸 믹스쳐리셋 구조체 초기화 

	MixItemNoCopyFlag = 0; //복사방지 플렉 초기화
	MixCancelButtonDelayFlag = 0;


	return TRUE;
}

// pluto 제련
int CSmeltingItem::CancelSmeltingItem()
{
	
	for( int i = 0; i < 5; i++ )
	{
		if( S_smeltingItem.SmeltingItem[i].Flag)
		{
			if( !cInvenTory.AutoSetInvenItem( &S_smeltingItem.SmeltingItem[i]) )
			{
				sinThrowItemToFeild( &S_smeltingItem.SmeltingItem[i] );
				S_smeltingItem.SmeltingItem[i].Flag = 0;
			}
		}
	}

	SmeltingItem.OpenFlag = 0;
	m_SmeltingPrice = 0;
	ForceItemPrice2 = 0;
	memset(&S_smeltingItem,0,sizeof(SSmeltingItem)); //아이템을 다 날려버린다 
	ResetInvenItemCode();				//인벤 아이템 코드 초기화

	memset( &sSmeltingItem_Recv,0,sizeof( sSMELTINGITEM_SERVER ) ); //받은 믹스쳐 구조체 초기화  (일단 같이해준다)
	memset( &sSmeltingItem_Send,0,sizeof( sSMELTINGITEM_SERVER ) ); //보낸 믹스쳐 구조체 초기화 

	return TRUE;
}

// pluto 제련
int CSmeltingItem::SetSmeltingItemAreaCheck( sITEM *pItem, int Kind )
{
						//광석	  케이아스 이프리아 펜타나  바루나 도루네아 사티쿠아 메루카바
	int SmeltingPriceIndexOre = 25000;

	//수정    케이아스 이프리아 펜타나  바루나 도루네아 사티쿠아 메루카바 눈결정 스노우플라워 하얀눈물
	int SmeltingPriceIndexCrystal = 50000;
//	int SmeltingPriceIndexCrystal = 10000; // 장별 - 일본 요청으로 임시로 만원
	// 장별 - 대장장이의 혼
	DWORD SmeltingCode[] = { sin01, sin02, sin03, sin04, sin05, sin06, sin07, sin08, sin09, sin10, sin11, sin12, sin13, sin14 };

	int i,j;
	TradeColorIndex = 0; // 초기화
	TradeCrashItemIndex[0] = 0; //충돌된 아이템 Index 초기화 
	cTrade.InitTradeColorRect(); //초기화

	if((pItem->CODE & sinITEM_MASK2)  == sinOS1 )
	{ 
		//TradeStartX = 218;
		//TradeStartY = 193+sinInterHeight2;
		//TradeEndX = TradeStartX+(3*22);
		//TradeEndY = TradeStartY+(4*22);

		//for(i=pItem->x+11 ; i<pItem->x+pItem->w ; i+=22)
		//{
		//	for( j=pItem->y+11 ; j<pItem->y+pItem->h ; j+=22)
		//	{
		//		if(TradeStartX <= i && TradeEndX > i && TradeStartY <=j && TradeEndY > j)
		//		{ //인벤토리 박스에 들어갔는지를 체크 
		//			TradeColorRect.left =  TradeStartX +( ( ( i - TradeStartX) /22)*22) ; 
		//			TradeColorRect.top  =  TradeStartY +( ( ( j - TradeStartY) /22)*22) ; 
		//			TradeColorRect.right  = pItem->w;
		//			TradeColorRect.bottom = pItem->h;
		//			if(TradeEndX < (TradeColorRect.left + TradeColorRect.right)-11 || TradeEndY < (TradeColorRect.top + TradeColorRect.bottom)-11)
		//			{
		//				cTrade.InitTradeColorRect(); //컬러 박스 RECT 초기화 (컬러 박스 RECT중 하나라두 0 이면 셋팅할수 없다 
		//				return FALSE;
		//			}

		//			pItem->SetX = TradeColorRect.left;
		//			pItem->SetY = TradeColorRect.top;
		//			pItem->ItemPosition = 0; //배열의 인덱스를 저장한다 
		//			TradeColorIndex = SET_ITEM_CHECK_COLOR; //셋팅할영역
		//			if( !S_smeltingItem.SmeltingItem[0].Flag )
		//			{
		//				TradeColorIndex = NOT_SETTING_COLOR;
		//				return FALSE;
		//			}
		//			if(Kind == 0)
		//			{ //믹스쳐 
		//				if(TradeCrashItemIndex[0] = cTrade.CrashTradeItem(TradeColorRect,0,4))
		//				{  //겹치는 아이템이 있나 체크 
		//					TradeColorIndex = NOT_SETTING_COLOR;
		//					return FALSE;

		//				}
		//				return TRUE;
		//			}
		//		}

		//	}

		//}


		//int SmeltingSheltomAray[4][3] = { 0, 0, 0,		// 쉘텀 갯수 늘어 나면 쓰자
		//								  0, 0, 0,
		//								  0, 0, 0,
		//								  0, 0, 0 };

		int SmeltingSheltomIndex = 0;
		SmeltingSheltomIndex = GetSheltomIndex( S_smeltingItem.SmeltingItem[0].CODE );
		int Smelting_SheltomIndex = 0;
		DWORD SheltomAray[MAX_SHELTOM] = { sin01, sin02, sin03, sin04, sin05, sin06, sin07, sin08, sin09, sin10, sin11, sin12, sin13};

		for( int q = 0; q < MAX_SHELTOM; q++ )
		{
			if( SheltomAray[q] == (pItem->sItemInfo.CODE & sinITEM_MASK3) )
			{
				Smelting_SheltomIndex = q+1;
				break;
			}
		}
		//for(int p = 0 ; p < 4 ; p++)			// 쉘텀 갯수 늘어 나면 쓰자
		{
			//for(int m=0; m < 3 ; m++)
			{
				if( SmeltingSheltomIndex == Smelting_SheltomIndex )
				{
					TradeStartX = AgingLevelSheltomXY[0][0].x + 22;			// 좌표는 같아서 걍 썻음
					TradeStartY = AgingLevelSheltomXY[0][0].y + 30;
					TradeEndX = TradeStartX+22;
					TradeEndY = TradeStartY+22;
					for(i=pItem->x+11 ; i<pItem->x+pItem->w ; i+=22)
					{
						for( j=pItem->y+11 ; j<pItem->y+pItem->h ; j+=22)
						{
							if(TradeStartX <= i && TradeEndX > i && TradeStartY <=j && TradeEndY > j)
							{ //인벤토리 박스에 들어갔는지를 체크 
								TradeColorRect.left =  TradeStartX +( ( ( i - TradeStartX) /22)*22) ; 
								TradeColorRect.top  =  TradeStartY +( ( ( j - TradeStartY) /22)*22) ; 
								TradeColorRect.right  = pItem->w;
								TradeColorRect.bottom = pItem->h;
								if(TradeEndX < (TradeColorRect.left + TradeColorRect.right)-11 || TradeEndY < (TradeColorRect.top + TradeColorRect.bottom)-11)
								{
									cTrade.InitTradeColorRect(); //컬러 박스 RECT 초기화 (컬러 박스 RECT중 하나라두 0 이면 셋팅할수 없다 
									return FALSE;
								}

								pItem->SetX = TradeColorRect.left;
								pItem->SetY = TradeColorRect.top;
								pItem->ItemPosition = 0; //배열의 인덱스를 저장한다

								TradeColorIndex = SET_ITEM_CHECK_COLOR; //셋팅할영역
								if( !S_smeltingItem.SmeltingItem[0].Flag )
								{
									TradeColorIndex = NOT_SETTING_COLOR;
									return FALSE;
								}

								if(TradeCrashItemIndex[0] = cTrade.CrashTradeItem(TradeColorRect,0,4))
								{  //겹치는 아이템이 있나 체크 
									TradeColorIndex = NOT_SETTING_COLOR;
									return FALSE;

								}
								return TRUE;
							}
						}
					}
				}
			}
		}
	}
	else if( (pItem->CODE & sinITEM_MASK2) == sinPR1 || (pItem->CODE & sinITEM_MASK2) == sinPR2 )
	{
		//char *p = pItem->LastCategory;
		//if( strcmp( p, "SE101") )
		//{
		//	return false;
		//}
		TradeColorIndex= 0; //초기화   (트레이드 창과 창고가 같이 쓴다)
		TradeCrashItemIndex[0] = 0; //충돌된 아이템 Index 초기화 
		cTrade.InitTradeColorRect(); //초기화

		//TradeStartX = 58;
		//TradeStartY = 193+sinInterHeight2;
		//TradeEndX = TradeStartX+(3*22);
		//TradeEndY = TradeStartY+(3*22);

		TradeStartX = 69;
		TradeStartY = 212+sinInterHeight2;
		TradeEndX = TradeStartX+(2*18);
		TradeEndY = TradeStartY+(2*18);
		for(i=pItem->x+11 ; i<pItem->x+pItem->w ; i+=22)
		{
			for( j=pItem->y+11 ; j<pItem->y+pItem->h ; j+=22)
			{
				if(TradeStartX <= pCursorPos.x && TradeEndX > pCursorPos.x && TradeStartY <=pCursorPos.y && TradeEndY > pCursorPos.y)
				{ //인벤토리 박스에 들어갔는지를 체크 // 하는중
					if( !( 80 <= pCursorPos.x && 101 >= pCursorPos.x && 143 <= pCursorPos.y && 164 >= pCursorPos.y ) )
					{
						TradeColorRect.left =  TradeStartX +( ( ( i - TradeStartX) /22)*22); 
						TradeColorRect.top  =  TradeStartY +( ( ( j - TradeStartY) /22)*22); 
						TradeColorRect.right  = pItem->w;
						TradeColorRect.bottom = pItem->h;
						if(TradeEndX < (TradeColorRect.left + TradeColorRect.right)-11 || TradeEndY < (TradeColorRect.top + TradeColorRect.bottom)-11)
						{
							cTrade.InitTradeColorRect(); //컬러 박스 RECT 초기화 (컬러 박스 RECT중 하나라두 0 이면 셋팅할수 없다 
							return FALSE;
						}

						pItem->SetX = TradeColorRect.left;
						pItem->SetY = TradeColorRect.top;
						pItem->ItemPosition = 0; //배열의 인덱스를 저장한다 
						TradeColorIndex = SET_ITEM_CHECK_COLOR; //셋팅할영역

						if(TradeCrashItemIndex[0] = cTrade.CrashTradeItem(TradeColorRect,0,4))
						{  //겹치는 아이템이 있나 체크 
							TradeColorIndex = NOT_SETTING_COLOR;
							return FALSE;
						}

						// 제련 할수 있는 돈이 있나 검사
						for( int i = 0; i < 7; i++ )
						{
							// 광석
							if( (pItem->CODE & sinITEM_MASK2) == sinPR1 )
							{
								if( ( pItem->sItemInfo.CODE & sinITEM_MASK3) == SmeltingCode[i] )
								{
									if( sinChar->Money < SmeltingPriceIndexOre)
									{
										TradeColorIndex = NOT_ENOUGH_MONEY;
										return FALSE;
									}
								}
							}
						}

						for( int i = 0; i < 14; i++ ) // 장별 - 제련 // 장별 - 대장장이의 혼
						{
							// 수정
							if( (pItem->CODE & sinITEM_MASK2) == sinPR2 )
							{
								if( (pItem->sItemInfo.CODE & sinITEM_MASK3) == SmeltingCode[i] )
								{
									if( sinChar->Money < SmeltingPriceIndexCrystal )
									{
										TradeColorIndex = NOT_ENOUGH_MONEY;
										return FALSE;
									}
								}
							}
						}

						////복사되 아이템은 믹스쳐와 에이징을 할수없다 MakeItem은 믹스할수없다 
						if(pItem->sItemInfo.SpecialItemFlag[0] == CHECK_COPY_ITEM ||
							pItem->sItemInfo.ItemKindCode == ITEM_KIND_QUEST  ||
							(pItem->sItemInfo.CODE & sinITEM_MASK2) == sinMA1 ||
							(pItem->sItemInfo.CODE & sinITEM_MASK2) == sinMA2 ||
							(pItem->sItemInfo.CODE & sinITEM_MASK2) == sinQW1 ||
							(pItem->sItemInfo.CODE & sinITEM_MASK2) == sinPZ1 || //퍼즐도 올릴수엄따 
							(pItem->sItemInfo.CODE & sinITEM_MASK2) == sinPZ2 ||
							(pItem->sItemInfo.CODE & sinITEM_MASK2) == sinFO1 || //포스도안된다
							pItem->sItemInfo.CODE == (sinGF1|sin01) ||
							pItem->sItemInfo.CODE == (sinGF1|sin02) ||
							pItem->sItemInfo.CODE == (sinGF1|sin03) || 
							pItem->sItemInfo.CODE == (sinGF1|sin04) ||
							pItem->sItemInfo.CODE == (sinGF1|sin05) ||
							pItem->sItemInfo.CODE == (sinGF1|sin06) ||
							pItem->sItemInfo.CODE == (sinGF1|sin07) ||	// 장별 - 조사원을 찾아라
							pItem->sItemInfo.CODE == (sinGF1|sin08) )
						{ 

							TradeColorIndex = NOT_SETTING_COLOR;
							return FALSE;
						}
						return TRUE;
					}

				}
			}
		}
	}
	return TRUE;
}

// pluto 제련
int CSmeltingItem::LastSetSmeltingItem( sITEM *pItem, int Kind )
{
	if(Kind == 0)
	{
		CheckSmeltingItemForm();
		if((pItem->CODE & sinITEM_MASK2)  == sinOS1)
		{ //쉘텀일 경우
			//for(int j=12 ; j < 24 ; j++)
			//{
			//	if(!S_smeltingItem.SmeltingItem[j].Flag)
			//	{
			//		memcpy(&S_smeltingItem.SmeltingItem[j],pItem,sizeof(sITEM));
			//		S_smeltingItem.SmeltingItem[j].x = pItem->SetX;
			//		S_smeltingItem.SmeltingItem[j].y = pItem->SetY;
			//		pItem->Flag = 0; //초기화 
			//		sinPlaySound(S_smeltingItem.SmeltingItem[j].SoundIndex);
			//		ReFormSmeltingItem();
			//		return TRUE;
			//	}
			//}
			if( !S_smeltingItem.SmeltingItem[4].Flag)	// pluto 쉘텀 한개만 놓는다 
			{
				memcpy(&S_smeltingItem.SmeltingItem[4],pItem,sizeof(sITEM));
				S_smeltingItem.SmeltingItem[4].x = pItem->SetX;
				S_smeltingItem.SmeltingItem[4].y = pItem->SetY;
				pItem->Flag = 0; //초기화 
				sinPlaySound(S_smeltingItem.SmeltingItem[4].SoundIndex);
				ReFormSmeltingItem();
				return TRUE;
			}

		}
		else
		{
			int k = 3; // 장별 - 제련 시스템 수정
			//if( (pItem->CODE & sinITEM_MASK2) == sinPR1 )
			//{
			//	k = 5;		// 광석
			//}
			//else if( (pItem->CODE & sinITEM_MASK2) == sinPR2 )
			//{
			//	k = 3;		// 수정
			//}

			for( int i = 0; i < k; i++ )
			{
				if(!S_smeltingItem.SmeltingItem[i].Flag)		// 광석, 수정
				{
					memcpy(&S_smeltingItem.SmeltingItem[i],pItem,sizeof(sITEM));
					S_smeltingItem.SmeltingItem[i].x = pItem->SetX;
					S_smeltingItem.SmeltingItem[i].y = pItem->SetY;
					pItem->Flag = 0; //초기화 
					sinPlaySound(sCraftItem.CraftItem[i].SoundIndex);
					ReFormSmeltingItem();
					CheckSmeltingPrice();
					return TRUE;
				}
			}
		}
	}
	return TRUE;
}

// pluto 제련
int CSmeltingItem::CheckSmeltingItemForm()
{ //체크  
	int TempCheckDataSum = 0;



	for(int i=0;i<5;i++)
	{
		if( S_smeltingItem.SmeltingItem[i].Flag)
		{
			TempCheckDataSum += (i+1)*S_smeltingItem.SmeltingItem[i].x;
			TempCheckDataSum += (i+1)*S_smeltingItem.SmeltingItem[i].y;
			TempCheckDataSum += S_smeltingItem.SmeltingItem[i].sItemInfo.ItemHeader.dwChkSum;
			TempCheckDataSum += S_smeltingItem.SmeltingItem[i].sItemInfo.ItemHeader.Head;

		}
	}
	return TRUE;
}

// pluto 제련
int CSmeltingItem::ReFormSmeltingItem()
{ //인증 

	SmeltingItemCheckSum = 0;
	for(int i=0;i<5;i++)
	{
		if(S_smeltingItem.SmeltingItem[i].Flag)
		{
			SmeltingItemCheckSum += (i+1)*S_smeltingItem.SmeltingItem[i].x;
			SmeltingItemCheckSum += (i+1)*S_smeltingItem.SmeltingItem[i].y;
			SmeltingItemCheckSum += S_smeltingItem.SmeltingItem[i].sItemInfo.ItemHeader.dwChkSum;
			SmeltingItemCheckSum += S_smeltingItem.SmeltingItem[i].sItemInfo.ItemHeader.Head;

		}
	}

	return TRUE;
}

// pluto 제련 아이템 집기
int CSmeltingItem::PickUpSmeltingItem(int x , int y , int PickUpFlag,int Kind)
{
	int i=0;
	SelectTradeItemIndex = 0;    //집을 아이템 
	if(Kind == 0)
	{
		for(i=0 ; i < 5 ; i++)
		{
			if( S_smeltingItem.SmeltingItem[i].Flag)
			{
				if( S_smeltingItem.SmeltingItem[i].x < x  && S_smeltingItem.SmeltingItem[i].x + S_smeltingItem.SmeltingItem[i].w > x  && 
					S_smeltingItem.SmeltingItem[i].y < y  && S_smeltingItem.SmeltingItem[i].y + S_smeltingItem.SmeltingItem[i].h > y )
				{
					if(PickUpFlag)
					{
                        if( i < 4 && S_smeltingItem.SmeltingItem[4].Flag )
						{
							cMessageBox.ShowMessage(MESSAGE_NOT_SMELTING);
							return FALSE;
						}
						CheckSmeltingItemForm();
						memcpy(&MouseItem , &S_smeltingItem.SmeltingItem[i] , sizeof(sITEM));
						S_smeltingItem.SmeltingItem[i].Flag = 0;
						memset(&S_smeltingItem.SmeltingItem[i],0,sizeof(sITEM));  //서버 해킹 에러 때문에 임시 방편이다.

						sinPlaySound(S_smeltingItem.SmeltingItem[i].SoundIndex);
						TradeColorIndex = 0;
						ReFormSmeltingItem();
						return TRUE;
					}
					else
					{
						SelectTradeItemIndex = i+1;
						TradeColorIndex = SELECT_ITEM_COLOR;
						sinShowItemInfoFlag = 1; //아이템의 정보를 보여준다 
						cItem.ShowItemInfo(&S_smeltingItem.SmeltingItem[i] , 2);
					}
				}
			}
		}
	}
	return FALSE;
}

// pluto 제련
int CSmeltingItem::sinSendSmeltingItem()
{
	if(MixItemNoCopyFlag)
	{
		return FALSE;
	}

	int i = 0;

	//복사를 체크하기위한 백업본을 만든다
	memcpy( &sSmeltingItemBackUp, &S_smeltingItem, sizeof( SSmeltingItem ));
	MixItemNoCopyFlag = 1; //아이템이 복사되는걸 방지하기위해 플렉을준다
	memset( &sSmeltingItem_Send.DesSmeltingItem, 0, sizeof(sITEM));

	sSmeltingItem_Send.Money = m_SmeltingPrice;

	for( i = 0 ; i < 5 ; i++)
	{
		if( S_smeltingItem.SmeltingItem[i].Flag)
		{
			sSmeltingItem_Send.SmeltingCode[i]	= S_smeltingItem.SmeltingItem[i].CODE;
			sSmeltingItem_Send.CheckSum[i]		= S_smeltingItem.SmeltingItem[i].sItemInfo.ItemHeader.dwChkSum;
			sSmeltingItem_Send.Head[i]			= S_smeltingItem.SmeltingItem[i].sItemInfo.ItemHeader.Head;
		}
	}

	CheckCraftMoney = sSmeltingItem_Send.Money;

	CraftItemSendServerIndex++; //인덱스를 보낸다 
	sSmeltingItem_Send.Index = CraftItemSendServerIndex;
	//저장된 구조체를 서버에 보낸다 


	SendSmeltingItemToServer( &sSmeltingItem_Send );

	return TRUE;
}

int CSmeltingItem::sinRecvSmeltingItem( sSMELTINGITEM_SERVER *pSmeltingItem_Server )
{
	memcpy(&sSmeltingItem_Recv ,pSmeltingItem_Server, sizeof(sSMELTINGITEM_SERVER));
    
	return TRUE;
}

// pluto 제련 아이템의 결과물을 서버에서 받는다 
int CSmeltingItem::sinRecvSmeltingItemResult( sSMELTINGITEM_SERVER *pSmeltingItem_Server )
{
	CheckMixItemCopyFlag = 0; //복사방지 플렉  
	if( !SmeltingItem.OpenFlag )
	{
		MixItemNoCopyFlag = 0; //복사방지 플렉 초기화
		memset(&S_smeltingItem,0,sizeof(SSmeltingItem)); //구조체 초기화 
		memset(&sSmeltingItemBackUp,0,sizeof(SSmeltingItem)); //구조체 초기화 
		memset(&sSmeltingItem_Recv,0,sizeof(sSMELTINGITEM_SERVER)); //받은 제련 구조체 초기화 
		memset(&sSmeltingItem_Send,0,sizeof(sSMELTINGITEM_SERVER)); //보낸 제련 구조체 초기화 
		MixCancelButtonDelayFlag = 0;
		return FALSE;

	}

	int i , LineCount=0 ,Temp;


	if(pSmeltingItem_Server->Result)
	{
		ShowItemSmeltingMessageFlag = 1; // 아이템이 제련됬을때 메세지를 보여준다

		sITEM TempItem;
		if(LoadItemImage(&pSmeltingItem_Server->DesSmeltingItem.sItemInfo , &TempItem))
		{
			lstrcpy( szCraftItemBuff, pSmeltingItem_Server->DesSmeltingItem.sItemInfo.ItemName );
			
			strcat(szCraftItemBuff, "\r");
			Temp = lstrlen(szCraftItemBuff);
			for( i = 0 ; i < Temp ; i++)
			{
				if(szCraftItemBuff[i] == '\r')
				{
					LineCount++;
				}
			}
			CraftItemMessageSize.x = 14;           //박스 사이즈 
			CraftItemMessageSize.y = ((LineCount)*2)+2;  //박스 사이즈 

			if(cInvenTory.CheckRequireItemToSet(&TempItem))
			{
				//곧바로 셋팅될수있는지를 체크한다  
				if(!cInvenTory.AutoSetInvenItem(&TempItem,1))
				{
					sinThrowItemToFeild(&pSmeltingItem_Server->DesSmeltingItem);
				}
			}

		}
	
		CheckCharForm(); //인증 
		sinMinusMoney( m_SmeltingPrice );
		ReformCharForm(); //재인증 
		SendSaveMoney(); //금액 조작을 못하게하기위해 호출한다 
		SaveGameData();
		SmeltingItem.OpenFlag = 0; //창을 닫는다
	}
	else
	{
		CheckCharForm(); //인증 
		sinMinusMoney( m_SmeltingPrice );
		ReformCharForm(); //재인증 
		SendSaveMoney(); //금액 조작을 못하게하기위해 호출한다 
		SmeltingItem.OpenFlag = 0; //창을 닫는다 
		cMessageBox.ShowMessage(MESSAGE_FAILD_SMELTING_ITEM);
	}

	//복사가되었거나한 아이템을 지운다
	for( i=0; i < 5 ; i++)
	{
		if(sSmeltingItemBackUp.SmeltingItem[i].Flag)
		{
			cInvenTory.DeleteInvenItemToServer(sSmeltingItemBackUp.SmeltingItem[i].sItemInfo.CODE,
				sSmeltingItemBackUp.SmeltingItem[i].sItemInfo.ItemHeader.Head,
				sSmeltingItemBackUp.SmeltingItem[i].sItemInfo.ItemHeader.dwChkSum);
		}

	}
	memset(&S_smeltingItem,0,sizeof(SSmeltingItem)); //구조체 초기화 
	memset(&sSmeltingItemBackUp,0,sizeof(SSmeltingItem)); //구조체 초기화 
	memset(&sSmeltingItem_Recv,0,sizeof(sSMELTINGITEM_SERVER)); //받은 제련 구조체 초기화 
	memset(&sSmeltingItem_Send,0,sizeof(sSMELTINGITEM_SERVER)); //보낸 제련 구조체 초기화 

	MixItemNoCopyFlag = 0; //복사방지 플렉 초기화
	MixCancelButtonDelayFlag = 0;
	m_SmeltingPrice = 0;
	ForceItemPrice2 = 0;
	return TRUE;
}

// pluto 제련
int CSmeltingItem::SmeltingCheckEmptyArea( sITEM *pItem )
{
	int cntx,cnty;
	int cx,cy;

	RECT	rect;
	int		flag;
	int StartX = 23;  //인벤 박스의 시작점 X
	int StartY = 436; // Y

	cx = (22*INVENTORY_BOX_ROW)-pItem->w; //아이템이 들어갈수있는 가장 끝 X좌표 
	cy = (22*INVENTORY_BOX_COL)-pItem->h; //아이템이 들어갈수있는 가장 끝 Y좌표

	//코드가 같지 않거나 아이템이 비어있지 않은 곳에 셋팅할때 검색해서 위치를 찾아낸다 
	for( cntx=StartX ; cntx <= StartX+cx ; cntx+=22){
		for( cnty=StartY ; cnty <= StartY+cy ; cnty+=22){ 
			flag = 0;
			for(int i=0 ; i<INVENTORY_MAXITEM ; i++){
				if(cInvenTory.InvenItem[i].Flag){
					rect.left   = cInvenTory.InvenItem[i].x;
					rect.right  = cInvenTory.InvenItem[i].x+cInvenTory.InvenItem[i].w;
					rect.top    = cInvenTory.InvenItem[i].y;
					rect.bottom = cInvenTory.InvenItem[i].y+cInvenTory.InvenItem[i].h;

					if( cntx > rect.left-pItem->w && cntx< rect.right && 
						cnty > rect.top-pItem->h && cnty< rect.bottom )
						flag++;
				}
			}
			if ( flag==1 ) { // 장별 - 제련 시스템 수정
				pItem->SetX = cntx;
				pItem->SetY = cnty;
				pItem->ItemPosition = 0; 
				return TRUE;
			}
		}
	}

	return FALSE;
}

int CSmeltingItem::GetSmeltingCheckEmptyArea()
{
	int cntx,cnty;
	int cx,cy;

	RECT	rect;
	int		flag;
	int StartX = 23;  //인벤 박스의 시작점 X
	int StartY = 436; // Y

	cx = (22*INVENTORY_BOX_ROW)-22; //아이템이 들어갈수있는 가장 끝 X좌표 
	cy = (22*INVENTORY_BOX_COL)-22; //아이템이 들어갈수있는 가장 끝 Y좌표

	int count = 0;
	//코드가 같지 않거나 아이템이 비어있지 않은 곳에 셋팅할때 검색해서 위치를 찾아낸다 
	for( cntx=StartX ; cntx <= StartX+cx ; cntx+=22){
		for( cnty=StartY ; cnty <= StartY+cy ; cnty+=22){ 
			flag = 0;
			for(int i=0 ; i<INVENTORY_MAXITEM ; i++){
				if(cInvenTory.InvenItem[i].Flag){
					rect.left   = cInvenTory.InvenItem[i].x;
					rect.right  = cInvenTory.InvenItem[i].x+cInvenTory.InvenItem[i].w;
					rect.top    = cInvenTory.InvenItem[i].y;
					rect.bottom = cInvenTory.InvenItem[i].y+cInvenTory.InvenItem[i].h;

					if( cntx > rect.left-22 && cntx< rect.right && 
						cnty > rect.top-22 && cnty< rect.bottom )
						flag++;
				}
			}
			if ( flag==1 ) { // 장별 - 제련 시스템 수정
				count++;
			}
		}
	}
	if( count )
		return count;
	else
		return 0;
}

// pluto 광석, 수정 제련 비용
int CSmeltingItem::CheckSmeltingPrice()
{						//광석	  케이아스 이프리아 펜타나  바루나 도루네아 사티쿠아 메루카바
	//int SmeltingPriceIndexOre[] = { 100000, 200000, 300000, 400000, 500000, 600000, 700000 };
	int SmeltingPriceIndexOre = 25000;

							//수정    케이아스 이프리아 펜타나  바루나 도루네아 사티쿠아 메루카바
	//int SmeltingPriceIndexCrystal[] = { 200000, 400000, 600000, 800000, 1000000, 1200000, 1400000 };
	int SmeltingPriceIndexCrystal = 50000;

//	int SmeltingPriceIndexCrystal = 10000; // 장별 - 일본 요청으로 임시로 만원

	//DWORD SmeltingCode[] = { sin01, sin02, sin03, sin04, sin05, sin06, sin07, sin08 };
	m_SmeltingPrice = 0;
	if( S_smeltingItem.SmeltingItem[0].Flag )
	{
		if( (S_smeltingItem.SmeltingItem[0].sItemInfo.CODE & sinITEM_MASK2) == sinPR1 ) // 광석
		{
			/*
			for( int i = 0; i < 7; i++ )
			{
				if( (S_smeltingItem.SmeltingItem[0].CODE & sinITEM_MASK3) == SmeltingCode[i] )
				{
					m_SmeltingPrice = SmeltingPriceIndexOre;
					ForceItemPrice2 = m_SmeltingPrice;
					return TRUE;
				}
			}
			*/
			m_SmeltingPrice = SmeltingPriceIndexOre;
			ForceItemPrice2 = m_SmeltingPrice;
			return TRUE;
		}
		else if( (S_smeltingItem.SmeltingItem[0].sItemInfo.CODE & sinITEM_MASK2) == sinPR2  ) // 수정
		{
			/*
			for( int i = 0; i < 7; i++ )
			{
				if( (S_smeltingItem.SmeltingItem[0].CODE & sinITEM_MASK3) == SmeltingCode[i] )
				{
					m_SmeltingPrice = SmeltingPriceIndexCrystal[i];
					ForceItemPrice2 = m_SmeltingPrice;
					return TRUE;
				}
			}
			*/
			m_SmeltingPrice = SmeltingPriceIndexCrystal;
			ForceItemPrice2 = m_SmeltingPrice;
			return TRUE;
		}
	}
	return TRUE;
}

// pluto 제작 취소
int CManufacture::CancelManufactureItem()
{
	
	for( int i = 0; i < 4; i++ )
	{
		if( g_sManufactureItem.ManufactureItem[i].Flag )
		{
			if( !cInvenTory.AutoSetInvenItem( &g_sManufactureItem.ManufactureItem[i]) )
			{
				sinThrowItemToFeild( &g_sManufactureItem.ManufactureItem[i] );
				g_sManufactureItem.ManufactureItem[i].Flag = 0;
			}
		}
	}

	ManufactureItem.m_OpenFlag = 0;
	m_ManufacturePrice = 0;
	ForceItemPrice2 = 0;
	memset(&g_sManufactureItem,0,sizeof( SManufactureItem )); //아이템을 다 날려버린다 
	ResetInvenItemCode();				//인벤 아이템 코드 초기화

	memset( &g_sManufactureItem_Recv,0,sizeof( SManufactureItem_Server ) ); //받은 제작 구조체 초기화
	memset( &g_sManufactureItem_Send,0,sizeof( SManufactureItem_Server ) ); //보낸 제작 구조체 초기화 

	return TRUE;
}

// pluto 제작 아이템 들어갈 자리 검사
int CManufacture::SetManufactureItemAreaCheck( sITEM *pItem )
{
	////광석	  케이아스 이프리아 펜타나  바루나 도루네아 사티쿠아 메루카바
	//int SmeltingPriceIndexOre[] = { 100000, 200000, 300000, 400000, 500000, 600000, 700000 };

	////수정    케이아스 이프리아 펜타나  바루나 도루네아 사티쿠아 메루카바
	//int SmeltingPriceIndexCrystal[] = { 200000, 400000, 600000, 800000, 1000000, 1200000, 1400000 };
	//DWORD SmeltingCode[] = { sin01, sin02, sin03, sin04, sin05, sin06, sin07 };

	POINT ManufactureRuneXY[3] = { {240, 159},  {215, 190}, {265, 190} };

	int i,j;
	TradeColorIndex = 0; // 초기화
	TradeCrashItemIndex[0] = 0; //충돌된 아이템 Index 초기화 
	cTrade.InitTradeColorRect(); //초기화

	if( (pItem->CODE & sinITEM_MASK2)  == sinPR3 || (pItem->CODE & sinITEM_MASK2)  == sinPR4 )
	{ 

		//TradeStartX = 218;
		//TradeStartY = 193+sinInterHeight2;
		//TradeEndX = TradeStartX+(3*22);
		//TradeEndY = TradeStartY+(4*22);

		//for(i=pItem->x+11 ; i<pItem->x+pItem->w ; i+=22)
		//{
		//	for( j=pItem->y+11 ; j<pItem->y+pItem->h ; j+=22)
		//	{
		//		if(TradeStartX <= i && TradeEndX > i && TradeStartY <=j && TradeEndY > j)
		//		{ //인벤토리 박스에 들어갔는지를 체크 
		//			TradeColorRect.left =  TradeStartX +( ( ( i - TradeStartX) /22)*22) ; 
		//			TradeColorRect.top  =  TradeStartY +( ( ( j - TradeStartY) /22)*22) ; 
		//			TradeColorRect.right  = pItem->w;
		//			TradeColorRect.bottom = pItem->h;
		//			if(TradeEndX < (TradeColorRect.left + TradeColorRect.right)-11 || TradeEndY < (TradeColorRect.top + TradeColorRect.bottom)-11)
		//			{
		//				cTrade.InitTradeColorRect(); //컬러 박스 RECT 초기화 (컬러 박스 RECT중 하나라두 0 이면 셋팅할수 없다 
		//				return FALSE;
		//			}

		//			pItem->SetX = TradeColorRect.left;
		//			pItem->SetY = TradeColorRect.top;
		//			pItem->ItemPosition = 0; //배열의 인덱스를 저장한다 
		//			TradeColorIndex = SET_ITEM_CHECK_COLOR; //셋팅할영역

		//			if(TradeCrashItemIndex[0] = cTrade.CrashTradeItem(TradeColorRect,0,5))
		//			{  //겹치는 아이템이 있나 체크 
		//				TradeColorIndex = NOT_SETTING_COLOR;
		//				return FALSE;
		//			}
		//			return TRUE;
		//		}

		//	}

		//}

		// 조합 가능한 룬만 놓는다 // 장별 - 룬 추가 11까지 // 장별 - 대장장이의 혼 11->14
		int RuneItemIndex = 0;
		DWORD RuneAray[MAX_RUNE] = { sin01, sin02, sin03, sin04 ,sin05, sin06, sin07, sin08, sin09, sin10, sin11, sin12, sin13, sin14 };
		for( int q = 0; q < MAX_RUNE; q++ )
		{
			if( RuneAray[q] == (pItem->sItemInfo.CODE & sinITEM_MASK3))
			{
				RuneItemIndex = q+1;
				break;
			}
		}
		int RecipeIndex = 0;
		RecipeIndex = ManufactureItem.GetRecipeIndex( g_sManufactureItem.ManufactureItem[0].CODE & sinITEM_MASK3 );
		for(int p = 0 ; p < 3 ; p++)
		{
			//for(int m=0; m < 3 ; m++)
			{
				for( int r = 0; r < MAX_RECIPE_KIND; r++ )
				{
					if( g_sManufacture_Rune[r].RecipeCode == RecipeIndex)
					{
						if( g_sManufacture_Rune[r].NeedRuneIndex[p] == RuneItemIndex )
						{
							TradeStartX = ManufactureRuneXY[p].x;			// 좌표는 같아서 걍 썻음
							TradeStartY = ManufactureRuneXY[p].y;
							TradeEndX = TradeStartX+22;
							TradeEndY = TradeStartY+22;
							for(i=pItem->x+11 ; i<pItem->x+pItem->w ; i+=22)
							{
								for( j=pItem->y+11 ; j<pItem->y+pItem->h ; j+=22)
								{
									if(TradeStartX <= i && TradeEndX > i && TradeStartY <=j && TradeEndY > j)
									{ //인벤토리 박스에 들어갔는지를 체크 
										TradeColorRect.left =  TradeStartX +( ( ( i - TradeStartX) /22)*22) ; 
										TradeColorRect.top  =  TradeStartY +( ( ( j - TradeStartY) /22)*22) ; 
										TradeColorRect.right  = pItem->w;
										TradeColorRect.bottom = pItem->h;
										if(TradeEndX < (TradeColorRect.left + TradeColorRect.right)-11 || TradeEndY < (TradeColorRect.top + TradeColorRect.bottom)-11)
										{
											cTrade.InitTradeColorRect(); //컬러 박스 RECT 초기화 (컬러 박스 RECT중 하나라두 0 이면 셋팅할수 없다 
											return FALSE;
										}

										pItem->SetX = TradeColorRect.left;
										pItem->SetY = TradeColorRect.top;
										pItem->ItemPosition = 0; //배열의 인덱스를 저장한다

										TradeColorIndex = SET_ITEM_CHECK_COLOR; //셋팅할영역

										if(TradeCrashItemIndex[0] = cTrade.CrashTradeItem(TradeColorRect,0,5))
										{  //겹치는 아이템이 있나 체크 
											TradeColorIndex = NOT_SETTING_COLOR;
											return FALSE;
										}

										return TRUE;
									}
								}
							}
						}
					}
				}
			}
		}
	}
	else if( (pItem->CODE & sinITEM_MASK2) == sinWR1 || (pItem->CODE & sinITEM_MASK2) == sinDR1 )
	{
		TradeColorIndex= 0; //초기화   (트레이드 창과 창고가 같이 쓴다)
		TradeCrashItemIndex[0] = 0; //충돌된 아이템 Index 초기화 
		cTrade.InitTradeColorRect(); //초기화

		TradeStartX = 58;
		TradeStartY = 193+sinInterHeight2;
		TradeEndX = TradeStartX+(3*22);
		TradeEndY = TradeStartY+(4*22);
		if(TradeStartX <= pCursorPos.x && TradeEndX > pCursorPos.x && TradeStartY <=pCursorPos.y && TradeEndY > pCursorPos.y)
		{ //인벤토리 박스에 들어갔는지를 체크 
			TradeColorRect.left =  TradeStartX;
			TradeColorRect.top  =  TradeStartY;
			TradeColorRect.right  = TradeEndX - TradeStartX;
			TradeColorRect.bottom = TradeEndY - TradeStartY;


			pItem->SetX = TradeStartX+((TradeEndX - TradeStartX)-pItem->w)/2;
			pItem->SetY = TradeStartY+((TradeEndY - TradeStartY)-pItem->h)/2;
			pItem->ItemPosition = 0; //배열의 인덱스를 저장한다 
			TradeColorIndex = SET_ITEM_CHECK_COLOR; //셋팅할영역

			if(TradeCrashItemIndex[0] = cTrade.CrashTradeItem(TradeColorRect,0,5))
			{  //겹치는 아이템이 있나 체크 
				TradeColorIndex = NOT_SETTING_COLOR;
				return FALSE;

			}

			//돈이 부족할 경우 
			//--------------------------------------------------------------------------//

			if(sinChar->Money < pItem->sItemInfo.Price)
			{
				TradeColorIndex = NOT_ENOUGH_MONEY;
				return FALSE;
			}
			//복사되 아이템은 믹스쳐와 에이징을 할수없다 MakeItem은 믹스할수없다 
			if(pItem->sItemInfo.SpecialItemFlag[0] == CHECK_COPY_ITEM ||
				pItem->sItemInfo.ItemKindCode == ITEM_KIND_QUEST  ||
				(pItem->sItemInfo.CODE & sinITEM_MASK2) == sinMA1 ||
				(pItem->sItemInfo.CODE & sinITEM_MASK2) == sinMA2 ||
				(pItem->sItemInfo.CODE & sinITEM_MASK2) == sinQW1 ||
				(pItem->sItemInfo.CODE & sinITEM_MASK2) == sinPZ1 || //퍼즐도 올릴수엄따 
				(pItem->sItemInfo.CODE & sinITEM_MASK2) == sinPZ2 ||
				(pItem->sItemInfo.CODE & sinITEM_MASK2) == sinFO1 || //포스도안된다
				pItem->sItemInfo.CODE == (sinGF1|sin01) ||
				pItem->sItemInfo.CODE == (sinGF1|sin02) ||
				pItem->sItemInfo.CODE == (sinGF1|sin03) || 
				pItem->sItemInfo.CODE == (sinGF1|sin04) ||
				pItem->sItemInfo.CODE == (sinGF1|sin05) ||
				pItem->sItemInfo.CODE == (sinGF1|sin06) ||
				pItem->sItemInfo.CODE == (sinGF1|sin07) || // 장별 - 조사원을 찾아라
				pItem->sItemInfo.CODE == (sinGF1|sin08) )
			{ 

					TradeColorIndex = NOT_SETTING_COLOR;
					return FALSE;
				
			}
				return TRUE;
		}
	}
	return TRUE;
}

// pluto 제작 아이템 놓는다
int CManufacture::LastSetManufactureItem( sITEM *pItem )
{
	CheckManufactureItem();
	if( (pItem->CODE & sinITEM_MASK2)  == sinPR3 || (pItem->CODE & sinITEM_MASK2)  == sinPR4 )
	{ 
		// 룬일 경우
		for(int j = 1 ; j < 4 ; j++)
		{
			if( !g_sManufactureItem.ManufactureItem[j].Flag )
			{
				memcpy(&g_sManufactureItem.ManufactureItem[j],pItem,sizeof(sITEM));
				g_sManufactureItem.ManufactureItem[j].x = pItem->SetX;
				g_sManufactureItem.ManufactureItem[j].y = pItem->SetY;
				pItem->Flag = 0; //초기화 
				sinPlaySound(g_sManufactureItem.ManufactureItem[j].SoundIndex);
				ReFormManufactureItem();
				return TRUE;
			}
		}
	}
	else
	{
		if( !g_sManufactureItem.ManufactureItem[0].Flag )
		{
			memcpy( &g_sManufactureItem.ManufactureItem[0],pItem,sizeof(sITEM) );
			g_sManufactureItem.ManufactureItem[0].x = pItem->SetX;
			g_sManufactureItem.ManufactureItem[0].y = pItem->SetY;
			pItem->Flag = 0; //초기화 
			sinPlaySound(g_sManufactureItem.ManufactureItem[0].SoundIndex);
			ReFormManufactureItem();
			return TRUE;
		}
	}
	return TRUE;

}

// pluto 제작
int CManufacture::CheckManufactureItem()
{ //체크  
	int TempCheckDataSum = 0;

	for(int i=0;i<4;i++)
	{
		if( g_sManufactureItem.ManufactureItem[i].Flag)
		{
			TempCheckDataSum += (i+1)*g_sManufactureItem.ManufactureItem[i].x;
			TempCheckDataSum += (i+1)*g_sManufactureItem.ManufactureItem[i].y;
			TempCheckDataSum += g_sManufactureItem.ManufactureItem[i].sItemInfo.ItemHeader.dwChkSum;
			TempCheckDataSum += g_sManufactureItem.ManufactureItem[i].sItemInfo.ItemHeader.Head;
		}
	}
	return TRUE;
}

// pluto 제작
int CManufacture::ReFormManufactureItem()
{ 
	//인증 
	m_ManufactureItemCheckSum = 0;
	for(int i=0; i<4 ;i++)
	{
		if(g_sManufactureItem.ManufactureItem[i].Flag)
		{
			m_ManufactureItemCheckSum += (i+1)*g_sManufactureItem.ManufactureItem[i].x;
			m_ManufactureItemCheckSum += (i+1)*g_sManufactureItem.ManufactureItem[i].y;
			m_ManufactureItemCheckSum += g_sManufactureItem.ManufactureItem[i].sItemInfo.ItemHeader.dwChkSum;
			m_ManufactureItemCheckSum += g_sManufactureItem.ManufactureItem[i].sItemInfo.ItemHeader.Head;
		}
	}
	return TRUE;
}


int CManufacture::PickUpManufactureItem( int x, int y, int PickUpFlag )
{
	int i=0;
	SelectTradeItemIndex = 0;    //집을 아이템 

	for(i=0 ; i < 4 ; i++)
	{
		if( g_sManufactureItem.ManufactureItem[i].Flag )
		{
			if ( g_sManufactureItem.ManufactureItem[i].x < x  && g_sManufactureItem.ManufactureItem[i].x + g_sManufactureItem.ManufactureItem[i].w > x  && 
				g_sManufactureItem.ManufactureItem[i].y < y  && g_sManufactureItem.ManufactureItem[i].y + g_sManufactureItem.ManufactureItem[i].h > y )
			{
				if(PickUpFlag)
				{
					int cnt = 0;
					for( cnt = 1 ; cnt < 4 ; cnt++ )
					{
						if( i==0 &&  g_sManufactureItem.ManufactureItem[cnt].Flag )
						{
							cMessageBox.ShowMessage(MESSAGE_NOT_MANUFACTURE);
							return FALSE;
						}
					}

					CheckManufactureItem();
					memcpy(&MouseItem , &g_sManufactureItem.ManufactureItem[i] , sizeof(sITEM));
					g_sManufactureItem.ManufactureItem[i].Flag = 0;
					memset(&g_sManufactureItem.ManufactureItem[i],0,sizeof(sITEM));  //서버 해킹 에러 때문에 임시 방편이다.

					sinPlaySound(g_sManufactureItem.ManufactureItem[i].SoundIndex);
					TradeColorIndex = 0;
					ReFormManufactureItem();
					return TRUE;
				}
				else
				{
					SelectTradeItemIndex = i+1;
					TradeColorIndex = SELECT_ITEM_COLOR;
					sinShowItemInfoFlag = 1; //아이템의 정보를 보여준다 
					cItem.ShowItemInfo(&g_sManufactureItem.ManufactureItem[i] , 2);
				}
			}
		}
	}
	return FALSE;
}


// pluto 제작
int CManufacture::ManufactureCheckEmptyArea( sITEM *pItem )
{
	int cntx,cnty;
	int cx,cy;

	RECT	rect;
	int		flag;
	int StartX = 23;  //인벤 박스의 시작점 X
	int StartY = 436; // Y

	cx = (22*INVENTORY_BOX_ROW)-pItem->w; //아이템이 들어갈수있는 가장 끝 X좌표 
	cy = (22*INVENTORY_BOX_COL)-pItem->h; //아이템이 들어갈수있는 가장 끝 Y좌표

	//코드가 같지 않거나 아이템이 비어있지 않은 곳에 셋팅할때 검색해서 위치를 찾아낸다 
	for( cntx=StartX ; cntx <= StartX+cx ; cntx+=22){
		for( cnty=StartY ; cnty <= StartY+cy ; cnty+=22){ 
			flag = 0;
			for(int i=0 ; i<INVENTORY_MAXITEM ; i++){
				if(cInvenTory.InvenItem[i].Flag){
					rect.left   = cInvenTory.InvenItem[i].x;
					rect.right  = cInvenTory.InvenItem[i].x+cInvenTory.InvenItem[i].w;
					rect.top    = cInvenTory.InvenItem[i].y;
					rect.bottom = cInvenTory.InvenItem[i].y+cInvenTory.InvenItem[i].h;

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

int CManufacture::sinSendManufactureItem()
{
	if(MixItemNoCopyFlag)
	{
		return FALSE;
	}

	int i = 0;

	//복사를 체크하기위한 백업본을 만든다
	memcpy( &g_sManufactureItemBackUp, &g_sManufactureItem, sizeof( SManufactureItem ));
	MixItemNoCopyFlag = 1; //아이템이 복사되는걸 방지하기위해 플렉을준다
	memset( &g_sManufactureItem_Send.DesManufactureItem, 0, sizeof(sITEM));

	g_sManufactureItem_Send.Money = m_ManufacturePrice;
	//if( g_sManufactureItem.ManufactureItem[0].Flag )
	//{
	//	g_sManufactureItem_Send.DesManufactureItem = g_sManufactureItem.ManufactureItem[0];
	//}
	for( i = 0 ; i < 4 ; i++)
	{
		if( g_sManufactureItem.ManufactureItem[i].Flag)
		{
			g_sManufactureItem_Send.RuneCode[i]		= g_sManufactureItem.ManufactureItem[i].CODE;
			g_sManufactureItem_Send.CheckSum[i]		= g_sManufactureItem.ManufactureItem[i].sItemInfo.ItemHeader.dwChkSum;
			g_sManufactureItem_Send.Head[i]			= g_sManufactureItem.ManufactureItem[i].sItemInfo.ItemHeader.Head;
		}
	}

	CheckCraftMoney = g_sManufactureItem_Send.Money;

	CraftItemSendServerIndex++; //인덱스를 보낸다 
	g_sManufactureItem_Send.Index = CraftItemSendServerIndex;
	
	//저장된 구조체를 서버에 보낸다 
	SendManufactureItemToServer( &g_sManufactureItem_Send );

	return TRUE;
}

// pluto 제작
int CManufacture::sinRecvManufactureItem( SManufactureItem_Server *pManufactureItem_Server )
{
	memcpy(&g_sManufactureItem_Recv, pManufactureItem_Server, sizeof(SManufactureItem_Server));
	return TRUE;
}

// pluto 제작 // 장별 - 제작
void CManufacture::RuneIndexInit()
{
	// pluto 제작 할때 필요한 룬 보여주는거
	for( int i = 0; i < MAX_RECIPE_KIND; i++ )
	{
		memset(&g_sManufacture_Rune[i], 0, sizeof(SManufacture_Rune));
	}

	g_sManufacture_Rune[0].RecipeCode = EXTREME_RECIPE; // 잊혀진 레시피
	g_sManufacture_Rune[0].NeedRuneIndex[0] = 1;
	g_sManufacture_Rune[0].NeedRuneIndex[1] = 2;
	g_sManufacture_Rune[0].NeedRuneIndex[2] = 3;

	////////////////////////////////////////////////////

	g_sManufacture_Rune[1].RecipeCode = ANCIENT_RECIPE; // 고대의 레시피
	g_sManufacture_Rune[1].NeedRuneIndex[0] = 2;
	g_sManufacture_Rune[1].NeedRuneIndex[1] = 3;
	g_sManufacture_Rune[1].NeedRuneIndex[2] = 4;

	////////////////////////////////////////////////////

	g_sManufacture_Rune[2].RecipeCode = MINOTAUR_RECIPE; // 대지의 레시피
	g_sManufacture_Rune[2].NeedRuneIndex[0] = 3;
	g_sManufacture_Rune[2].NeedRuneIndex[1] = 4;
	g_sManufacture_Rune[2].NeedRuneIndex[2] = 5;

	////////////////////////////////////////////////////

	g_sManufacture_Rune[3].RecipeCode = DOOM_RECIPE; // 어둠의 레시피
	g_sManufacture_Rune[3].NeedRuneIndex[0] = 3;
	g_sManufacture_Rune[3].NeedRuneIndex[1] = 4;
	g_sManufacture_Rune[3].NeedRuneIndex[2] = 5;

	////////////////////////////////////////////////////

	g_sManufacture_Rune[4].RecipeCode = SALAMANDER_RECIPE; // 화염의 레시피
	g_sManufacture_Rune[4].NeedRuneIndex[0] = 3;
	g_sManufacture_Rune[4].NeedRuneIndex[1] = 4;
	g_sManufacture_Rune[4].NeedRuneIndex[2] = 5;

	////////////////////////////////////////////////////

	g_sManufacture_Rune[5].RecipeCode = WYVERN_RECIPE; // 바람의 레시피
	g_sManufacture_Rune[5].NeedRuneIndex[0] = 3;
	g_sManufacture_Rune[5].NeedRuneIndex[1] = 4;
	g_sManufacture_Rune[5].NeedRuneIndex[2] = 5;

	////////////////////////////////////////////////////

	g_sManufacture_Rune[6].RecipeCode = PHOENIX_RECIPE; // 태양의 레시피
	g_sManufacture_Rune[6].NeedRuneIndex[0] = 4;
	g_sManufacture_Rune[6].NeedRuneIndex[1] = 5;
	g_sManufacture_Rune[6].NeedRuneIndex[2] = 6;

	////////////////////////////////////////////////////

	g_sManufacture_Rune[7].RecipeCode = FRENZY_RECIPE; // 광포한 레시피
	g_sManufacture_Rune[7].NeedRuneIndex[0] = 5;
	g_sManufacture_Rune[7].NeedRuneIndex[1] = 6;
	g_sManufacture_Rune[7].NeedRuneIndex[2] = 7;

	////////////////////////////////////////////////////

	g_sManufacture_Rune[8].RecipeCode = HEAVENS_RECIPE; // 천상의 레시피
	g_sManufacture_Rune[8].NeedRuneIndex[0] = 6;
	g_sManufacture_Rune[8].NeedRuneIndex[1] = 7;
	g_sManufacture_Rune[8].NeedRuneIndex[2] = 8;

	////////////////////////////////////////////////////

	g_sManufacture_Rune[9].RecipeCode = SNOW_RECIPE; // 포설의 레시피 // 장별 - 제작
	g_sManufacture_Rune[9].NeedRuneIndex[0] = 9;
	g_sManufacture_Rune[9].NeedRuneIndex[1] = 10;
	g_sManufacture_Rune[9].NeedRuneIndex[2] = 11;

	////////////////////////////////////////////////////

	g_sManufacture_Rune[10].RecipeCode = MEMORY_RECIPE; // 기억의 레시피 // 장별 - 대장장이의 혼
	g_sManufacture_Rune[10].NeedRuneIndex[0] = 12;
	g_sManufacture_Rune[10].NeedRuneIndex[1] = 13;
	g_sManufacture_Rune[10].NeedRuneIndex[2] = 14;


}

// pluto 제작 레시피 인덱스다 필요한 룬 그림 셋팅 할라구 // 장별 - 제작 // 장별 - 대장장이의 혼
int CManufacture::GetRecipeIndex( DWORD Code )
{
	int RecipeIndex = -1;

	switch( Code )
	{
		case sin01:
            RecipeIndex = EXTREME_RECIPE;
			break;
		case sin02:
			RecipeIndex = ANCIENT_RECIPE;
			break;
		case sin03:
			RecipeIndex = MINOTAUR_RECIPE;
			break;
		case sin04:
			RecipeIndex = DOOM_RECIPE;
			break;
		case sin05:
			RecipeIndex = SALAMANDER_RECIPE;
			break;
		case sin06:
			RecipeIndex = WYVERN_RECIPE;
			break;
		case sin07:
			RecipeIndex = PHOENIX_RECIPE;
			break;
		case sin08:
			RecipeIndex = FRENZY_RECIPE; 
			break;
		case sin09:
			RecipeIndex = HEAVENS_RECIPE;
			break;

		case sin10:
			RecipeIndex = SNOW_RECIPE; // 장별 - 제작 : 포설의 제작서
		break;

		case sin11:
			RecipeIndex = MEMORY_RECIPE; //  기억의 제작서	// 장별 - 대장장이의 혼
		break;
	}
	/*
	if( Code == sin01 )
	{
		RecipeIndex = OLD_RECIPE;
		return RecipeIndex;
	}
	else if( Code == sin02 )
	{
		RecipeIndex = DISCOLORED_RECIPE;
		return RecipeIndex;
	}
	else if( Code == sin03 )
	{
		RecipeIndex = ANCIENT_RECIPE;
		return RecipeIndex;
	}
	else if( Code == sin04 )
	{
		RecipeIndex = DISMAL_RECIPE;
		return RecipeIndex;
	}
	else
	{
		return RecipeIndex;
	}
	*/
	return RecipeIndex;
}

// pluto 제작 결과
int CManufacture::RecvManufactureItemResult( SManufactureItem_Server *pManufactureItem_Server )
{
	CheckMixItemCopyFlag = 0; //복사방지 플렉  
	if( !ManufactureItem.m_OpenFlag )
	{
		MixItemNoCopyFlag = 0; //복사방지 플렉 초기화
		memset(&g_sManufactureItem,0,sizeof(SManufactureItem)); //구조체 초기화 
		memset(&g_sManufactureItemBackUp,0,sizeof(SManufactureItem)); //구조체 초기화 
		memset(&g_sManufactureItem_Recv,0,sizeof(SManufactureItem_Server)); //받은 제작 구조체 초기화 
		memset(&g_sManufactureItem_Send,0,sizeof(SManufactureItem_Server)); //보낸 제작 구조체 초기화 
		MixCancelButtonDelayFlag = 0;
		return FALSE;

	}

	int i , LineCount=0 ,Temp;


	if(pManufactureItem_Server->Result)
	{
		ShowItemManufactureMessageFlag = 1; // 아이템이 제작됬을때 메세지를 보여준다

		sITEM TempItem;
		if(LoadItemImage(&pManufactureItem_Server->DesManufactureItem.sItemInfo, &TempItem))
		{
			lstrcpy( szCraftItemBuff, pManufactureItem_Server->DesManufactureItem.sItemInfo.ItemName );

			strcat(szCraftItemBuff, "\r");
			Temp = lstrlen(szCraftItemBuff);
			for( i = 0 ; i < Temp ; i++)
			{
				if(szCraftItemBuff[i] == '\r')
				{
					LineCount++;
				}
			}
			CraftItemMessageSize.x = 14;           //박스 사이즈 
			CraftItemMessageSize.y = ((LineCount)*2)+2;  //박스 사이즈 

			if(cInvenTory.CheckRequireItemToSet(&TempItem))
			{
				//곧바로 셋팅될수있는지를 체크한다  
				if(!cInvenTory.AutoSetInvenItem(&TempItem,1))
				{
					sinThrowItemToFeild(&pManufactureItem_Server->DesManufactureItem);
				}
			}

		}

		CheckCharForm(); //인증 
		sinMinusMoney( m_ManufacturePrice );
		ReformCharForm(); //재인증 
		SendSaveMoney(); //금액 조작을 못하게하기위해 호출한다 
		SaveGameData();
		ManufactureItem.m_OpenFlag = 0; //창을 닫는다
	}
	else
	{
		CheckCharForm(); //인증 
		sinMinusMoney( m_ManufacturePrice );
		ReformCharForm(); //재인증 
		SendSaveMoney(); //금액 조작을 못하게하기위해 호출한다 
		ManufactureItem.m_OpenFlag = 0; //창을 닫는다 
		cMessageBox.ShowMessage( MESSAGE_FAILD_MANUFACTURE_ITEM );
	}

	//복사가되었거나한 아이템을 지운다
	for( i=0; i < 4 ; i++)
	{
		if(g_sManufactureItemBackUp.ManufactureItem[i].Flag)
		{
			cInvenTory.DeleteInvenItemToServer(g_sManufactureItemBackUp.ManufactureItem[i].sItemInfo.CODE,
				g_sManufactureItemBackUp.ManufactureItem[i].sItemInfo.ItemHeader.Head,
				g_sManufactureItemBackUp.ManufactureItem[i].sItemInfo.ItemHeader.dwChkSum);
		}

	}
	memset(&g_sManufactureItem,0,sizeof(SManufactureItem)); //구조체 초기화 
	memset(&g_sManufactureItemBackUp,0,sizeof(SManufactureItem)); //구조체 초기화 
	memset(&g_sManufactureItem_Recv,0,sizeof(SManufactureItem_Server)); //받은 제작 구조체 초기화 
	memset(&g_sManufactureItem_Send,0,sizeof(SManufactureItem_Server)); //보낸 제작 구조체 초기화 

	MixItemNoCopyFlag = 0; //복사방지 플렉 초기화
	MixCancelButtonDelayFlag = 0;
	m_ManufacturePrice = 0;
	ForceItemPrice2 = 0;
	return TRUE;
}

int CManufacture::GetManufactureCheckEmptyArea()
{
	int cntx,cnty;
	int cx,cy;

	RECT	rect;
	int		flag;
	int StartX = 23;  //인벤 박스의 시작점 X
	int StartY = 436; // Y

	cx = (22*INVENTORY_BOX_ROW)-22; //아이템이 들어갈수있는 가장 끝 X좌표 
	cy = (22*INVENTORY_BOX_COL)-22; //아이템이 들어갈수있는 가장 끝 Y좌표

	int count = 0;
	//코드가 같지 않거나 아이템이 비어있지 않은 곳에 셋팅할때 검색해서 위치를 찾아낸다 
	for( cntx=StartX ; cntx <= StartX+cx ; cntx+=22)
	{
		for( cnty=StartY ; cnty <= StartY+cy ; cnty+=22)
		{ 
			flag = 0;
			for(int i=0 ; i<INVENTORY_MAXITEM ; i++)
			{
				if(cInvenTory.InvenItem[i].Flag)
				{
					rect.left   = cInvenTory.InvenItem[i].x;
					rect.right  = cInvenTory.InvenItem[i].x+cInvenTory.InvenItem[i].w;
					rect.top    = cInvenTory.InvenItem[i].y;
					rect.bottom = cInvenTory.InvenItem[i].y+cInvenTory.InvenItem[i].h;

					if( cntx > rect.left-22 && cntx< rect.right && 
						cnty > rect.top-22 && cnty< rect.bottom )
						flag++;
				}
			}
			if ( flag==0 )
			{
				count++;
			}
		}
	}
	if( count )
		return count;
	else
		return 0;
}

// pluto 제련 회색 이미지 쉘텀 // 장별 - 제련 // 장별 - 대장장이의 혼
int CSmeltingItem::GetSheltomIndex( DWORD SmeltingItemCode )
{
	DWORD SmeltingItemAray[] = { (sinPR1|sin01), (sinPR1|sin02), (sinPR1|sin03), (sinPR1|sin04), (sinPR1|sin05), (sinPR1|sin06), (sinPR1|sin07), (sinPR1|sin08),
								 (sinPR2|sin01), (sinPR2|sin02), (sinPR2|sin03), (sinPR2|sin04), (sinPR2|sin05), (sinPR2|sin06), (sinPR2|sin07), (sinPR2|sin08),
								 (sinPR2|sin09), (sinPR2|sin10), (sinPR2|sin11), (sinPR2|sin12), (sinPR2|sin13), (sinPR2|sin14) };
	int SheltomAray[] = { 6, 7, 8, 9, 10, 11, 12, 13,
						  7, 8, 9, 10, 11, 12, 13, 14,
							3, 4, 5, 3, 4, 5}; // 광석이나 수정 번호
	for( int i = 0; i < 22 ; i++ )
	{
		if( SmeltingItemCode == SmeltingItemAray[i] )
		{
			return SheltomAray[i];
		}
	}
	return FALSE;
}

// pluto 제작될 아이템 코드. 무게 검사 할려고 // 장별 - 제작
SManufacture_ResultItemInfo CManufacture::ManufactureCheckItem( sITEM *pItemCode, smCHAR_INFO *charInfo ) // 장별 - 대장장이의 혼
{
	int RuneCnt = 0;
	SManufacture_ResultItemInfo Result;
	memset(&Result,0,sizeof(SManufacture_ResultItemInfo));
	DWORD Arm_Armor_Code[MAX_RECIPE_KIND] = { sin01, sin02, sin03, sin04, sin05, sin06, sin07, sin08, sin09, sin10, sin11 }; // 레시피 종류 // 장별 - 대장장이의 혼
	DWORD CheckRuneCode[MAX_HANDMADEITEM][8][3] = { {	{ (sinPR3|sin01), (sinPR3|sin02), (sinPR3|sin03) },  // 룬 조합식이다
										{ (sinPR3|sin01), (sinPR3|sin02), (sinPR4|sin03) },
										{ (sinPR3|sin01), (sinPR4|sin02), (sinPR3|sin03) },
										{ (sinPR3|sin01), (sinPR4|sin02), (sinPR4|sin03) },
										{ (sinPR4|sin01), (sinPR3|sin02), (sinPR3|sin03) },
										{ (sinPR4|sin01), (sinPR3|sin02), (sinPR4|sin03) },
										{ (sinPR4|sin01), (sinPR4|sin02), (sinPR3|sin03) },
										{ (sinPR4|sin01), (sinPR4|sin02), (sinPR4|sin03) } },
										////////////////////////////////////////////////////////////////////////////////////////////	잊혀진 제작서
									 {  { (sinPR3|sin02), (sinPR3|sin03), (sinPR3|sin04) },
										{ (sinPR3|sin02), (sinPR3|sin03), (sinPR4|sin04) },
										{ (sinPR3|sin02), (sinPR4|sin03), (sinPR3|sin04) },
										{ (sinPR3|sin02), (sinPR4|sin03), (sinPR4|sin04) },
										{ (sinPR4|sin02), (sinPR3|sin03), (sinPR3|sin04) },
										{ (sinPR4|sin02), (sinPR3|sin03), (sinPR4|sin04) },
										{ (sinPR4|sin02), (sinPR4|sin03), (sinPR3|sin04) },
										{ (sinPR4|sin02), (sinPR4|sin03), (sinPR4|sin04) } },
										////////////////////////////////////////////////////////////////////////////////////////////	고대의 제작서
									 {  { (sinPR3|sin03), (sinPR3|sin04), (sinPR3|sin05) },
										{ (sinPR3|sin03), (sinPR3|sin04), (sinPR4|sin05) },
										{ (sinPR3|sin03), (sinPR4|sin04), (sinPR3|sin05) },
										{ (sinPR3|sin03), (sinPR4|sin04), (sinPR4|sin05) },
										{ (sinPR4|sin03), (sinPR3|sin04), (sinPR3|sin05) },
										{ (sinPR4|sin03), (sinPR3|sin04), (sinPR4|sin05) },
										{ (sinPR4|sin03), (sinPR4|sin04), (sinPR3|sin05) },
										{ (sinPR4|sin03), (sinPR4|sin04), (sinPR4|sin05) } },
										////////////////////////////////////////////////////////////////////////////////////////////	대지의 제작서
									 {  { (sinPR3|sin03), (sinPR3|sin04), (sinPR3|sin05) },
										{ (sinPR3|sin03), (sinPR3|sin04), (sinPR4|sin05) },
										{ (sinPR3|sin03), (sinPR4|sin04), (sinPR3|sin05) },
										{ (sinPR3|sin03), (sinPR4|sin04), (sinPR4|sin05) },
										{ (sinPR4|sin03), (sinPR3|sin04), (sinPR3|sin05) },
										{ (sinPR4|sin03), (sinPR3|sin04), (sinPR4|sin05) },
										{ (sinPR4|sin03), (sinPR4|sin04), (sinPR3|sin05) },
										{ (sinPR4|sin03), (sinPR4|sin04), (sinPR4|sin05) } },
										////////////////////////////////////////////////////////////////////////////////////////////	어둠의 제작서
									 {	{ (sinPR3|sin03), (sinPR3|sin04), (sinPR3|sin05) },
										{ (sinPR3|sin03), (sinPR3|sin04), (sinPR4|sin05) },
										{ (sinPR3|sin03), (sinPR4|sin04), (sinPR3|sin05) },
										{ (sinPR3|sin03), (sinPR4|sin04), (sinPR4|sin05) },
										{ (sinPR4|sin03), (sinPR3|sin04), (sinPR3|sin05) },
										{ (sinPR4|sin03), (sinPR3|sin04), (sinPR4|sin05) },
										{ (sinPR4|sin03), (sinPR4|sin04), (sinPR3|sin05) },
										{ (sinPR4|sin03), (sinPR4|sin04), (sinPR4|sin05) } },
										////////////////////////////////////////////////////////////////////////////////////////////	화염의 제작서
									 {  { (sinPR3|sin03), (sinPR3|sin04), (sinPR3|sin05) },
										{ (sinPR3|sin03), (sinPR3|sin04), (sinPR4|sin05) },
										{ (sinPR3|sin03), (sinPR4|sin04), (sinPR3|sin05) },
										{ (sinPR3|sin03), (sinPR4|sin04), (sinPR4|sin05) },
										{ (sinPR4|sin03), (sinPR3|sin04), (sinPR3|sin05) },
										{ (sinPR4|sin03), (sinPR3|sin04), (sinPR4|sin05) },
										{ (sinPR4|sin03), (sinPR4|sin04), (sinPR3|sin05) },
										{ (sinPR4|sin03), (sinPR4|sin04), (sinPR4|sin05) } },
										////////////////////////////////////////////////////////////////////////////////////////////	바람의 제작서
									 {  { (sinPR3|sin04), (sinPR3|sin05), (sinPR3|sin06) },
										{ (sinPR3|sin04), (sinPR3|sin05), (sinPR4|sin06) },
										{ (sinPR3|sin04), (sinPR4|sin05), (sinPR3|sin06) },
										{ (sinPR3|sin04), (sinPR4|sin05), (sinPR4|sin06) },
										{ (sinPR4|sin04), (sinPR3|sin05), (sinPR3|sin06) },
										{ (sinPR4|sin04), (sinPR3|sin05), (sinPR4|sin06) },
										{ (sinPR4|sin04), (sinPR4|sin05), (sinPR3|sin06) },
										{ (sinPR4|sin04), (sinPR4|sin05), (sinPR4|sin06) } },
										////////////////////////////////////////////////////////////////////////////////////////////	태양의 제작서
									 {  { (sinPR3|sin05), (sinPR3|sin06), (sinPR3|sin07) },
										{ (sinPR3|sin05), (sinPR3|sin06), (sinPR4|sin07) },
										{ (sinPR3|sin05), (sinPR4|sin06), (sinPR3|sin07) },
										{ (sinPR3|sin05), (sinPR4|sin06), (sinPR4|sin07) },
										{ (sinPR4|sin05), (sinPR3|sin06), (sinPR3|sin07) },
										{ (sinPR4|sin05), (sinPR3|sin06), (sinPR4|sin07) },
										{ (sinPR4|sin05), (sinPR4|sin06), (sinPR3|sin07) },
										{ (sinPR4|sin05), (sinPR4|sin06), (sinPR4|sin07) } },
										////////////////////////////////////////////////////////////////////////////////////////////	광포한 제작서
									 {  { (sinPR3|sin06), (sinPR3|sin07), (sinPR3|sin08) },
										{ (sinPR3|sin06), (sinPR3|sin07), (sinPR4|sin08) },
										{ (sinPR3|sin06), (sinPR4|sin07), (sinPR3|sin08) },
										{ (sinPR3|sin06), (sinPR4|sin07), (sinPR4|sin08) },
										{ (sinPR4|sin06), (sinPR3|sin07), (sinPR3|sin08) },
										{ (sinPR4|sin06), (sinPR3|sin07), (sinPR4|sin08) },
										{ (sinPR4|sin06), (sinPR4|sin07), (sinPR3|sin08) },
										{ (sinPR4|sin06), (sinPR4|sin07), (sinPR4|sin08) } },
										////////////////////////////////////////////////////////////////////////////////////////////	천상의 제작서

									 {  { (sinPR3|sin09), (sinPR4|sin10), (sinPR3|sin11) },
										{ 0, 0, 0 },
										{ 0, 0, 0 },
										{ 0, 0, 0 },
										{ 0, 0, 0 },
										{ 0, 0, 0 },
										{ 0, 0, 0 },
										{ 0, 0, 0 } },
										////////////////////////////////////////////////////////////////////////////////////////////	포설의 제작서 //장별

										{  { (sinPR3|sin12), (sinPR3|sin13), (sinPR3|sin14) },
										{ 0, 0, 0 },
										{ 0, 0, 0 },
										{ 0, 0, 0 },
										{ 0, 0, 0 },
										{ 0, 0, 0 },
										{ 0, 0, 0 },
										{ 0, 0, 0 } },
										////////////////////////////////////////////////////////////////////////////////////////////	기억의 제작서 // 장별 - 대장장이의 혼
	};

	// 장별 - 대장장이의 혼
	DWORD	dwItem = 0;

	// 프리스티스 일 경우
	if(charInfo->JOB_CODE == JOBCODE_PRIESTESS) dwItem = (sinDA2|sin52);
	// 메지션 일 경우
	else if(charInfo->JOB_CODE == JOBCODE_MAGICIAN) dwItem = (sinDA2|sin51);
	// 아처, 아틀란타 일 경우
	else if(charInfo->JOB_CODE == JOBCODE_ARCHER || charInfo->JOB_CODE == JOBCODE_ATALANTA ) dwItem = (sinDA1|sin52);
	// 파이터, 파이크맨, 메카니션, 나이트 일 경우
	else dwItem = (sinDA1|sin51);


	// 장별 배열 숫자 [a][b][c] a = 무구 or 갑주 / b = 레시피 종류 / c = 생성될 아이템 종류(고정)
	// 레시피와 룬조합에 맞는 발생 아이템
	DWORD CreateItemCode[2][MAX_HANDMADEITEM][8] = {	{	{ (sinWA1|sin14), (sinWC1|sin14), (sinWH1|sin15), (sinWP1|sin15), (sinWS1|sin16), (sinWS2|sin17), (sinWT1|sin15), (sinWM1|sin15) }, // 잊혀진 레시피 무구
											{ (sinWA1|sin15), (sinWC1|sin15), (sinWH1|sin16), (sinWP1|sin16), (sinWS1|sin17), (sinWS2|sin18), (sinWT1|sin16), (sinWM1|sin16) }, // 고대의 레시피 무구
											{ (sinWA1|sin16), (sinWC1|sin16), (sinWH1|sin17), (sinWP1|sin17), (sinWS1|sin18), (sinWS2|sin19), (sinWT1|sin17), (sinWM1|sin17) }, // 대지의 무구
											{ (sinWA1|sin17), (sinWC1|sin17), (sinWH1|sin18), (sinWP1|sin18), (sinWS1|sin19), (sinWS2|sin20), (sinWT1|sin18), (sinWM1|sin18) }, // 어둠의 레시피 무구
											{ (sinWA1|sin18), (sinWC1|sin18), (sinWH1|sin19), (sinWP1|sin19), (sinWS1|sin20), (sinWS2|sin21), (sinWT1|sin19), (sinWM1|sin19) }, // 화염의 레시피 무구
											{ (sinWA1|sin19), (sinWC1|sin19), (sinWH1|sin20), (sinWP1|sin20), (sinWS1|sin21), (sinWS2|sin22), (sinWT1|sin20), (sinWM1|sin20) }, // 바람의 레시피 무구
											{ (sinWA1|sin21), (sinWC1|sin21), (sinWH1|sin22), (sinWP1|sin22), (sinWS1|sin23), (sinWS2|sin24), (sinWT1|sin22), (sinWM1|sin22) }, // 태양의 레시피 무구
											{ (sinWA1|sin22), (sinWC1|sin22), (sinWH1|sin23), (sinWP1|sin23), (sinWS1|sin24), (sinWS2|sin25), (sinWT1|sin23), (sinWM1|sin23) }, // 광포한 레시피 무구
											{ 0, 0, 0, 0, 0, 0, 0, 0 } , // 천상의 레시피 무구
											{ 0, 0, 0, 0, 0, 0, 0, 0 } , // 포설의 레시피 // 장별
											{ 0, 0, 0, 0, 0, 0, 0, 0 } }, // 기억의 레시피 // 장별 - 대장장이의 혼
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
										{	{ (sinDA1|sin16), (sinDA2|sin16), (sinDS1|sin14), (sinOM1|sin15), (sinDG1|sin14), (sinOA2|sin14), (sinDB1|sin14), 0 }, // 잊혀진 레시피 갑주
											{ (sinDA1|sin17), (sinDA2|sin17), (sinDS1|sin15), (sinOM1|sin16), (sinDG1|sin15), (sinOA2|sin15), (sinDB1|sin15), 0 }, // 고대의 레시피 갑주
											{ (sinDA1|sin18), (sinDA2|sin18), (sinDS1|sin16), (sinOM1|sin17), (sinDG1|sin16), (sinOA2|sin16), (sinDB1|sin16), 0 }, // 대지의 레시피 갑주
											{ (sinDA1|sin19), (sinDA2|sin19), (sinDS1|sin17), (sinOM1|sin18), (sinDG1|sin17), (sinOA2|sin17), (sinDB1|sin17), 0 }, // 어둠의 레시피 갑주
											{ (sinDA1|sin20), (sinDA2|sin20), (sinDS1|sin18), (sinOM1|sin19), (sinDG1|sin18), (sinOA2|sin18), (sinDB1|sin18), 0 }, // 화염의 레시피 갑주
											{ (sinDA1|sin21), (sinDA2|sin21), (sinDS1|sin19), (sinOM1|sin20), (sinDG1|sin19), (sinOA2|sin19), (sinDB1|sin19), 0 }, // 바람의 레시피 갑주
											{ (sinDA1|sin23), (sinDA2|sin23), (sinDS1|sin21), (sinOM1|sin22), (sinDG1|sin21), (sinOA2|sin21), (sinDB1|sin21), 0 }, // 태양의 레시피 갑주
											{ (sinDA1|sin24), (sinDA2|sin24), (sinDS1|sin22), (sinOM1|sin23), (sinDG1|sin22), (sinOA2|sin22), (sinDB1|sin22), 0 }, // 광포한 레시피 갑주
											{ 0, 0, 0, 0, 0, 0, 0, 0 } , // 천상의 레시피 갑주
											{ (sinOA1|sin36), 0, 0, 0, 0, 0, 0, 0 } , // 포설의 레시피 //장별
											{ dwItem, 0, 0, 0, 0, 0, 0, 0 } } // 기억의 레시피 // 장별 - 대장장이의 혼
	};

	if( (pItemCode[0].CODE & sinITEM_MASK2) == sinWR1 )		// 무구
	{
		for( int i = 0; i < MAX_RECIPE_KIND; i++ )
		{
			if( Arm_Armor_Code[i] == (pItemCode[0].CODE & sinITEM_MASK3) )
			{
				for( int j = 0; j < MAX_RUNE; j++ )   // 룬 조합 
				{
					RuneCnt = 0;
					for( int x = 0; x < 3; x++ ) // 필요한 룬
					{
						for( int y = 0; y < 3; y++ )
						{
							if( CheckRuneCode[i][j][x] == pItemCode[y+1].CODE )
							{
								RuneCnt++;
								break;
							}
						}
						if( RuneCnt == 3 )
						{
							for( int c = 0; c < MAX_HANDMADEITEM; c++ ) // 장별 - 대장장이의 혼
							{
								if( CreateItemCode[0][i][j] == g_Manufacture_ItemInfo[c].ItemCode )
								{
									Result.ItemCode = g_Manufacture_ItemInfo[c].ItemCode;
									lstrcpy( Result.ItemName, g_Manufacture_ItemInfo[c].ItemName );
									Result.Weight = g_Manufacture_ItemInfo[c].Weight;
									Result.Price = g_Manufacture_ItemInfo[c].Price / 3;
									ForceItemPrice2 = Result.Price;
									m_ManufacturePrice = Result.Price;
								}
							}
							return Result;
						}
					}
				}
			}
		}
	}
	else if( (pItemCode[0].CODE & sinITEM_MASK2) == sinDR1 )		// 갑주
	{
		for( int i = 0; i < MAX_RECIPE_KIND; i++ )
		{
			if( Arm_Armor_Code[i] == (pItemCode[0].CODE & sinITEM_MASK3) )
			{
				for( int j = 0; j < MAX_RUNE; j++ )
				{
					RuneCnt = 0;
					for( int x = 0; x < 3; x++ )
					{
						for( int y = 0; y < 3; y++ )
						{
							if( CheckRuneCode[i][j][x] == pItemCode[y+1].CODE )
							{
								RuneCnt++;
								break;
							}
						}
						if( RuneCnt == 3 )
						{
							for( int c = 0; c < MAX_HANDMADEITEM; c++ ) // 장별 - 대장장이의 혼
							{
								if( CreateItemCode[1][i][j] == g_Manufacture_ItemInfo[c].ItemCode )
								{
									Result.ItemCode = g_Manufacture_ItemInfo[c].ItemCode;
									lstrcpy( Result.ItemName, g_Manufacture_ItemInfo[c].ItemName );
									Result.Weight = g_Manufacture_ItemInfo[c].Weight;
									Result.Price = g_Manufacture_ItemInfo[c].Price / 3;
									ForceItemPrice2 = Result.Price;
									m_ManufacturePrice = Result.Price;
								}
							}
							if( j == 7 )
							{
								for( int c = 0; c < MAX_HANDMADEITEM; c++ ) // 장별 - 대장장이의 혼
								{
									if( CreateItemCode[1][i][0] == g_Manufacture_ItemInfo[c].ItemCode )
									{
										Result.ItemCode = g_Manufacture_ItemInfo[c].ItemCode;
										lstrcpy( Result.ItemName, ManufacturingTitle );	//fuck pluto//해외
										Result.Weight = g_Manufacture_ItemInfo[c].Weight;
										Result.Price = g_Manufacture_ItemInfo[c].Price / 3;
										ForceItemPrice2 = Result.Price;
										m_ManufacturePrice = Result.Price;
									}
								}
								return Result;
							}
							return Result;
						}
					}
				}
			}
		}
	}
	return Result;
}

// pluto 제작 아이템 무게, 가격 정보
int ManufactureItemResultInfo( SManufacture_ResultItemInfo_Server *pResultItemInfo )
{
	for( int i = 0; i < MAX_HANDMADEITEM ; i++ ) // 장별 - 제작 MAX_HANDMADEITEM = 제작 할 수 있는 아이템 갯수(레시피가 추가되면 증가) // 장별 - 대장장이의 혼
	{
		g_Manufacture_ItemInfo[i].ItemCode = pResultItemInfo->ItemCode[i];
		lstrcpy( g_Manufacture_ItemInfo[i].ItemName, pResultItemInfo->ItemName[i] );
		g_Manufacture_ItemInfo[i].Weight = pResultItemInfo->Weight[i];
		g_Manufacture_ItemInfo[i].Price = pResultItemInfo->Price[i];
	}
	return TRUE;
}