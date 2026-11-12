/*----------------------------------------------------------------------------*
*	파일명 :  sinTrade.h	
*	하는일 :  아이템 교환 
*	작성일 :  최종업데이트 12월
*	적성자 :  박상열 
*-----------------------------------------------------------------------------*/	
#define MAX_TRADE_ITEM			20
#define MAX_CRAFTITEM_INFO		300
#define MAX_AGINGITEM_INFO		100
#define MAX_MAKEITEM_INFO		100

#define T_SET_BOX				0
#define T_MONEY_POSI			1
#define T_MONEY_TEXT_POSI		2
#define T_WEIGHT_TEXT_POSI		3
#define T_CHECK_POSI			4
#define T_CLOSE_POSI			5
// 에이징에 관한 디파인 //성근추가
#define MAX_SHELTOM				14	//에이징에 필요한 쉘텀의 최대치 // 박재원 - 에이징 설정 변경(19차, 20차 추가)
#define MAX_AGING				25	// was 20 (+21..+25 added 2026-11-12)	//에이징의 마지막 차수 // 박재원 - 에이징 설정 변경(19차, 20차 추가)
#define MAX_RUNE				14	// pluto 제련, 제작 룬 // 장별 - 제련 추가 // 장별 - 대장장이의 혼
#define MAX_RECIPE_KIND			11	// pluto 레시피 종류 // 장별 - 제작 // 장별 - 대장장이의 혼( 10 -> 11 )
#define MAX_RECIPE_ITEM			60	// pluto 레시피 최대치
#define MAX_HANDMADEITEM		176 // 장별 - 대장장이의 혼 (제작서 추가시 +16)

#define MAX_SEEL_COUNT			4	// 박재원 - 테이와즈 씰 추가(제라, 니이드, 지푸, 테이와즈)

/*----------------------------------------------------------------------------*
*								트레이드  구조체 
*-----------------------------------------------------------------------------*/
struct sTRADE{
	sITEM TradeItem[MAX_TRADE_ITEM]; //트레이드될 아이템 
	int Money;			//트레이드될 금액 
	int Weight;			//보여질 무게 
	int CheckFlag;		//확인 플랙 
	int CheckMoney;
};

///////창고 구조체 
struct sWAREHOUSE{
	sITEM WareHouseItem[100]; 
	int Money;
	short Weight[2];
	int BuyAreaCount; //창고를 얼마만큼 사게되었나 
	int FakeMoney;


};

//////아이템 조합 구조체 
struct sCRAFTITEM{
	sITEM CraftItem[15];	//조합될 아이템 
	int   SheltomIndex[12];

};

/////서버로 보내질 아이템 구조체 
struct sCRAFTITEM_SERVER{
	int size, code;
	int DocIndex;
	int Result;
	sITEM DesCraftItem;
	DWORD SheltomCode[12];
	DWORD Head[12];
	DWORD CheckSum[12];
	int  Index;
	int  Money;

	DWORD A_StoneItemCode;
	DWORD A_StoneHead;
	DWORD A_StoneCheckSum;
};

///// 크래프트 아이템 
struct sCRAFTITEM_INFO{
	DWORD MainItemCode[8];	//조합할 아이템 코드 
	int AddItem[9];			//추가할 아이템 (쉘텀)
	int PlusElementCode[8];	//추가될 요인 
	float AddElement[8];	//추가된 수치 
	int ElementKind[8];		//수치의 기준 (+ % 소수)
	char Doc[256];			//설명 

};

// 석지용 - 믹스쳐 리셋 아이템 구조체
struct sMIXTURE_RESET_ITEM
{
	sITEM	MixtureResetItem[2];	// 리셋 시킬 아이템(0은 아이템, 1은 스톤)
};

// 석지용 - 믹스쳐 리셋 아이템 구조체 (서버용)
struct sMIXTURE_RESET_ITEM_SERVER
{
	int		size, code;
	int		DocIndex;
	int		Result;
	int		Index;

	sITEM	DesMixtureResetItem;	// 리셋 할 아이템

	DWORD	dwMixtureResetStoneItemCode;	// 믹스쳐 리셋 스톤용
	DWORD	dwMixtureResetStoneHead;		// 믹스쳐 리셋 스톤용
	DWORD	dwMixtureResetStoneCheckSum;	// 믹스쳐 리셋 스톤용
};

///// 에이징 
struct sAGINGITEM{
	sITEM AgingItem[15];	//조합될 아이템 
	int   SheltomIndex[12];	//조할될 쉘텀의 인덱스 
	int   AgingOkFlag;	    //조합이 될수있을까의 플랙 

};

struct sAGINGITEM_INFO{
	DWORD MainItemCode;		//조합할 아이템 코드 
	int PlusElementCode[8];	//추가될 요인 
	float AddElement[8];	//추가된 수치 
	int ElementKind[8];		//수치의 기준 (+ % 소수)
	char Doc[128];			//설명 

};

///// MakeItem
struct sMAKEITEM_INFO{
	DWORD CODE[15];			//조합되는 아이템의 코드
	DWORD Result_CODE;		//조합된 아이템 코드 

};

struct sPOSTBOX_ITEM{
	char	szName[64];
	DWORD	CODE;
};

//아이템 재구성 구조체
struct sRECON_ITEM{
	DWORD  dwCODE;    //..
	int	   iLevel[2]; //min~max
	int	   iCount;    //필요 수량
	int    iPrice;    //가격?
};

// pluto 제련
struct SSmeltingItem
{
	sITEM SmeltingItem[5];	//조합될 아이템 
	int   SheltomIndex[12];
};

// pluto 제련
struct sSMELTINGITEM_SERVER
{
	int size, code;
	int DocIndex;
	int Result;
	sITEM DesSmeltingItem;
	DWORD SmeltingCode[5];
	DWORD Head[5];
	DWORD CheckSum[5];
	int  Index;
	int  Money;

	DWORD A_StoneItemCode;
	DWORD A_StoneHead;
	DWORD A_StoneCheckSum;
};

// pluto 제작
struct SManufactureItem
{
	sITEM	ManufactureItem[4];
	int		RuneIndex[12];
};

// pluto 제작
struct SManufactureItem_Server
{
	int size, code;
	int DocIndex;
	int Result;
	sITEM DesManufactureItem;
	DWORD RuneCode[4];
	DWORD Head[4];
	DWORD CheckSum[4];
	int  Index;
	int  Money;

	DWORD A_StoneItemCode;
	DWORD A_StoneHead;
	DWORD A_StoneCheckSum;
};

// pluto 제작
struct SManufacture_Rune
{
	int RecipeCode;
	int NeedRuneIndex[3];
};

// pluto 제작 룬 조합 공식 맞으면 해당 아이템 코드 발생
struct SManufacture_CreateItemCode
{
	DWORD RecipeKind[2];
	DWORD CheckRuneCode[8][3];
	DWORD CreateItemCode[2][8];
};

// pluto 제작 아이템 무게, 가격 정보
struct SManufacture_ResultItemInfo
{
	DWORD ItemCode;
	int	Weight;
	int Price;
	char ItemName[32];
};

// pluto 제작 아이템 무게, 가격 정보  // 장별 - 제작 160(144) // 장별 - 대장장이의 혼
struct SManufacture_ResultItemInfo_Server
{
	smTRANS_COMMAND		smTransCommand;
	DWORD ItemCode[MAX_HANDMADEITEM];
	int Weight[MAX_HANDMADEITEM];
	int Price[MAX_HANDMADEITEM];
	char ItemName[MAX_HANDMADEITEM][32];
};
/*----------------------------------------------------------------------------*
*							에이징  아이템 클래스 
*-----------------------------------------------------------------------------*/
class cAGING{
public:

	int OpenFlag;
	int AgingItemCheckSum;
	LPDIRECTDRAWSURFACE4	lpGraySheltom[MAX_SHELTOM];

public:
	//에이징 에 사용돼는 종류   //1.에이징스톤 
	int  AginStoneKind;         //2.코퍼 오어
								//박재원 - 3.엘더 코퍼 오어
								//박재원 - 4.슈퍼 에이징 스톤
                                //차후 추가됨
	
public:

	int  AgingItem(int MakeItemFlag =0);		//아이템을 에이징한다 
	int  CancelAgingItem(int Flag=0);	//에이징 아이템을 캔슬한다 
	int  CheckItem(sITEM *pItem);		//에이징할 아이템을 체크한다 
	int  CheckAgingItemOk(); //에이징을 할수있는지를 체크한다 

	int  SetAgingItemAreaCheck(sITEM *pItem );	//에이징 아이템이 셋팅될 공간을 체크한다 
	
	///////////// 에이징 데이터의 조작여부를 가린다 
	int  CheckAgingData();

	/////////// 서버에서 아이템을 에이징한다
	int sinSendAgingItem();
	int sinRecvAgingItem(sCRAFTITEM_SERVER *pCraftItem_Server);
	int sinRecvAgingItemResult(sCRAFTITEM_SERVER *pCraftItem_Server);

};

/*----------------------------------------------------------------------------*
*						석지용 - 믹스쳐 리셋 클래스 
*-----------------------------------------------------------------------------*/
class cMIXTURERESET
{
public:

	int	OpenFlag;					// 열려있는가?
	int	MixtureResetItemCheckSum;	// 검사용
	int	HaveMixtureResetStone;		// 믹스쳐 리셋 스톤이 있는가?

public:

	int	SetMixtureItemResetAreaCheck(sITEM *pItem);		// 믹스쳐 리셋될 아이템의 영역을 체크한다.
	int PickUpMixtureResetItem( int x, int y, int PickUpFlag = 0, int Kind = 0 );	// 믹스쳐 리셋 아이템을 선택한다.
	int LastSetMixtureResetItem( sITEM *pItem, int kind );	// 믹스쳐 리셋에 필요한 아이템 놓는다
	int	CancelMixtureItemReset(int Flag=0);		// 믹스쳐 리셋을 취소하며 창을 닫는다.
	int	CheckMixtureResetItemForm();	// 아이템 체크?
	int	ReformMixtureResetItem();		// 아이템 다시 체크?



	int	MixtureItemReset();	
	int	CheckItem();
	int	CheckMixtureItemResetOk();
	int	CheckMixtureItemData();	



	int	sinSendMixtureItemReset();			// 서버로 믹스쳐리셋할 아이템을 전송
	int	sinRecvMixtureItemReset(sMIXTURE_RESET_ITEM_SERVER *pMixtureResetItem);		// 믹스쳐 리셋된 아이템을 Recv용 변수로 복사
	int	sinRecvMixtureItemResetResult(sMIXTURE_RESET_ITEM_SERVER *pMixtureResetItem_Server);
};

// pluto
/*----------------------------------------------------------------------------*
*								아이템 제련 클래스 
*-----------------------------------------------------------------------------*/
class CSmeltingItem
{
public:
	int OpenFlag;
	int SmeltingItemCheckSum;
	int m_SmeltingPrice;
public:
	int CancelSmeltingItem();
	int SetSmeltingItemAreaCheck( sITEM *pItem, int Kind = 0 );
	int LastSetSmeltingItem( sITEM *pItem, int Kind = 0);
	int CheckSmeltingItemForm();
	int ReFormSmeltingItem();
	int PickUpSmeltingItem( int x, int y, int PickUpFlag = 0, int Kind = 0 );
	int sinSendSmeltingItem(); // 서버에 제련 아이템 보낸다
	int sinRecvSmeltingItem( sSMELTINGITEM_SERVER *pSmeltingItem_Server ); // 제련 아이템 받는다
	int sinRecvSmeltingItemResult( sSMELTINGITEM_SERVER *pSmeltingItem_Server );
	int SmeltingCheckEmptyArea( sITEM *pItem );
	int GetSmeltingCheckEmptyArea();
	int CheckSmeltingPrice();		// 제련 비용 검사
	int GetSheltomIndex( DWORD SmeltingItemCode ); // 제련 회색 이미지 쉘텀
};

/*----------------------------------------------------------------------------*
* pluto							아이템 제작 클래스 
*-----------------------------------------------------------------------------*/

class CManufacture
{
public:
	int m_OpenFlag;
	int m_ManufactureItemCheckSum;
	int m_ManufacturePrice;
	LPDIRECTDRAWSURFACE4 m_lpGrayRune[MAX_SHELTOM];
public:
	void RuneIndexInit();	// 필요한 룬 회색 이미지
	int CancelManufactureItem(); // 제작 취소
	int SetManufactureItemAreaCheck( sITEM *pItem ); // 제작 아이템 들어갈 자리 검사
	int LastSetManufactureItem( sITEM *pItem );		// 제작 아이템 놓는다
	int CheckManufactureItem();
	int ReFormManufactureItem();
	int PickUpManufactureItem( int x, int y, int PickUpFlag = 0 );
	int sinSendManufactureItem();	// 서버로 보낸다
	int sinRecvManufactureItem( SManufactureItem_Server *pManufactureItem_Server );
	int RecvManufactureItemResult( SManufactureItem_Server *pManufactureItem_Server ); // 서버에서 받은 결과
	int ManufactureCheckEmptyArea( sITEM *pItem );
	int GetRecipeIndex( DWORD Code ); // 레시피 인덱스를 가져온다
	int GetManufactureCheckEmptyArea();
	SManufacture_ResultItemInfo ManufactureCheckItem( sITEM *pItemCode, smCHAR_INFO *charInfo );	// 제작될 아이템 코드. 무게 검사 할려고 // 장별 - 대장장이의 혼
};

/*----------------------------------------------------------------------------*
*							크래프트 아이템 클래스 
*-----------------------------------------------------------------------------*/
class cCRAFTITEM{
public:
	int OpenFlag;
	int CraftItemCheckSum;
	int ForceFlag;
	int ForceItemPrice;
	int iReconItemFlag;
public:

	int SetCraftItemAreaCheck(sITEM *pItem,int Kind=0); //아이템을 셋팅한다 
	int LastSetCraftItem(sITEM *pItem,int Kind=0);
	int PickUpCraftItem(int x , int y , int PickUpFlag=0,int Kind=0); //아이템을 집거나 정보를 보여준다 

	//아이템을 조합한다 
	int MixingItem();

	//아이템 공간을 체크한다 
	int CraftCheckEmptyArea(sITEM *pItem);
	int CancelMixItem(); //믹스쳐 아이템을 취소한다 

	int CheckHackCraftItem(); //크래프트 아이템 조작 방지
	int CheckCraftItemForm(); //조작하는 상놈의 자식들을 죽여버리자 퉤!!
	int ReFormCraftItem();    //조작하는 상놈의 자식들을 죽여버리자 퉤!!

	int sinSendCraftItem();	  //서버에 믹스할 아이템을 보낸다 	
	int sinRecvCraftItem(sCRAFTITEM_SERVER *pCraftItem_Server);   //서버에서 믹스할 아이템을 받는다 

	//믹스된 결과물을 받는다 
	int sinRecvCraftItemResult(sCRAFTITEM_SERVER *pCraftItem_Server);

	//Force Orb 아이템의 가격을 체크한다
	int CheckForceOrbPrice();
	int CheckForceOrbPrice(sITEM *pItem);

	//아이템 재구성 시스템
	int haCheckReconItem(); //아이템 재구성 
	int haSendReconItem();  //아이템 재구성 을 서버로 보낸다.
	/*----------------------------------------------------------------------------*
	* 작성자 : 하대용  			
	* Desc   : 추가돼는 함수들  		
	*-----------------------------------------------------------------------------*/

	//인베토리 빈곳을 체크한다.
	int GetCraftCheckEmptyArea();
	//sorting function
	int haCraftSortItem(sCRAFTITEM *pCraftItem);
};



/*----------------------------------------------------------------------------*
*								창고 클래스 
*-----------------------------------------------------------------------------*/

class cWAREHOUSE{
public:
	int OpenFlag;	//함수의 공용화를 위해 어쩔수 없이 클래스 생성 
	int WareHouseCheckSum;

public:

	int SetWareHouseItemAreaCheck(sITEM *pItem); //공간을 체크한다 
	int CrashWareHouseItem(RECT &desRect, int PassItemIndex=0); //아이템이 겹쳐졌는지를 찾는다 
	int ChangeWareHouseItem(sITEM *pItem); //겹쳐진 아이템을 셋팅한다 
	int LastSetWareHouseItem(sITEM *pItem); //아이템을 셋팅한다 
	int PickUpWareHouseItem(int x , int y , int PickUpFlag=0); //아이템을 집거나 정보를 보여준다 

	//무게를 구한다 
	int GetWeight();

	//아이템 이미지를 로드한다
	int LoadWareHouseItemIamge();	
	
	//인벤토리 아이템을 잠시 백업한다 
	int BackUpInvenItem();
	//아이템을 복구해준다 
	int RestoreInvenItem(); 
	//창고를 닫는다 
	int CloseWareHouse();

	//////////////////조작 방지 
	int ReFormWareHouse();
	int CheckWareHouseForm();

	//////////////////복사 아이템에 관한 처리 (아이템을 집을수없다)
	int CopyItemNotPickUp(sITEM *pItem,int JumpIndex);
	int CheckCopyItem(); //복사된 아이템이 있는지를 체크한다 
	int DeleteCopyItem(); //복사 아이템을 없애준다 (서버 이상으로 창고에 복사아이템이 많이 생겼음)

};

/*----------------------------------------------------------------------------*
*								스킬 클래스 
*-----------------------------------------------------------------------------*/
class cTRADE{

public:

	int OpenFlag;
	int MatTrade[4];
	int MatWareHouse[3]; //창고 (이미지하나는 상점에서 가져온다)
	int MatTradebuttonMain;
	int MatTradebuttonCancel;

	LPDIRECTDRAWSURFACE4 lpbuttonCancel;

	LPDIRECTDRAWSURFACE4 lpbuttonCancelInfo;

	LPDIRECTDRAWSURFACE4 lpbuttonOk;
	LPDIRECTDRAWSURFACE4 lpCheck;

	LPDIRECTDRAWSURFACE4 lpCheck_Glay;
	LPDIRECTDRAWSURFACE4 lpCheckInfo;
	LPDIRECTDRAWSURFACE4 lpDelayCheck;

	LPDIRECTDRAWSURFACE4 lpCenterBox;

	sITEM CheckItem[100];
	sITEM CheckRecvItem[MAX_TRADE_ITEM];

	////사용 변수 
	int   TradeRequestFlag;
	DWORD TradeCharCode;
	char  szTradeCharName[64];

	int   TradeItemCheckSum;



public:
	cTRADE();
	~cTRADE();

	void Init(); //클래스 초기화
	void Load();
	void Release();
	void Draw();
	void Close();//클래스 종료 
	void Main();
	void LButtonDown(int x , int y);
	void LButtonUp(int x , int y);
	void RButtonDown(int x , int y);
	void RButtonUp(int x, int y);
	void KeyDown(); 

	void DrawTradeText(); //텍스를 찍어준다 

	///////////// 트레이드 함수 
	int SetTradeItemAreaCheck(sITEM *pItem); //공간을 체크한다 
	int CrashTradeItem(RECT &desRect, int PassItemIndex=0 , int Kind=0); //아이템이 겹쳐졌는지를 찾는다 
	int InitTradeColorRect(); //컬러 박스 초기화 
	int ChangeTradeItem(sITEM *pItem); //겹쳐진 아이템을 셋팅한다 
	int LastSetTradeItem(sITEM *pItem); //아이템을 셋팅한다 
	int PickUpTradeItem(int x , int y , int PickUpFlag=0); //아이템을 집거나 정보를 보여준다 

	//아이템이 셋팅가능한지 알아본다 
	int CheckTradeItemSet(); //셋팅 체크 메인(무게를 체크하고 나머지 함수를 호출한다)
	int TradeCheckEmptyArea(sITEM *pItem); //아이템이 셋팅될수있는지 체크한다
	int ChecketInvenItem(sITEM *pItem); //아이템을 임시로 셋팅한다 

	//아이템을 인벤토리로 셋팅한다 
	int CheckOkTradeItem();			//트레이드 확인 
	int CancelTradeItem();			//트레이드 취소 
	int LoadTradeItemIamge();		//아이템 이미지를 로드한다

	//승인 관련 
	int CheckTradeButtonOk();		//서버에 승인을 얻는다 

	//무게를 구한다 
	int GetWeight();

	////////////////////아이템을 조작하는 상놈의 자식들을 죽여버린다 퉤!!
	int CheckTradeItemForm();	//검사 
	int ReFormTradeItem();		//인증 


};
// pluto 제작 아이템 무게, 가격
int ManufactureItemResultInfo( SManufacture_ResultItemInfo_Server *pResultItemInfo );
///////////// 함수
int CopyMixItemCheckDelete(sITEM *pItem);
int sinMakeItemCheck(); //MakeItem을 체크한다
int sinPosBoxNpc();     //PostBox Npc한테서 메세지를 받는다 
extern SManufacture_ResultItemInfo g_Manufacture_ItemInfo[MAX_HANDMADEITEM]; // 장별 - 제작 // 장별 - 대장장이의 혼
extern cTRADE cTrade;
extern sTRADE sTrade;
extern sTRADE sTradeRecv;

extern int RequestTradeButtonFlag; //거래요구 버튼 클릭 
	   
extern cWAREHOUSE cWareHouse; //창고 클래스 
extern sWAREHOUSE sWareHouse; //창고 구조체 
extern sITEM *BackUpInvenItem2; //인벤아이템 임시 백업 


extern cCRAFTITEM cCraftItem;	//아이템 조합 

extern CSmeltingItem SmeltingItem; // pluto 제련
extern SSmeltingItem S_smeltingItem; // pluto 제련
extern sSMELTINGITEM_SERVER sSmeltingItem_Send; // pluto 제련 서버로 보냄
extern sSMELTINGITEM_SERVER sSmeltingItem_Recv; // pluto 제련 서버에서 받음

extern CManufacture ManufactureItem; // pluto 제작
extern SManufactureItem g_sManufactureItem; // pluto 제작
extern SManufactureItem_Server g_sManufactureItem_Send; // pluto 제련 서버로 보냄
extern SManufactureItem_Server g_sManufactureItem_Recv; // pluto 제련 서버에서 받음
extern SManufacture_Rune g_sManufacture_Rune[MAX_RECIPE_KIND];
//extern SManufacture_ResultItemInfo_Server g_Item_Recv;
extern sCRAFTITEM sCraftItem;
extern sCRAFTITEM_INFO sCraftItem_Info[MAX_CRAFTITEM_INFO]; //아이템 조합 공식

extern sCRAFTITEM_SERVER sCraftItem_Send; //서버로 보내질 아이템 
extern sCRAFTITEM_SERVER sCraftItem_Recv; //서버에서 받을 아이템

extern sCRAFTITEM_SERVER sAging_Send; //서버로 보내질 아이템 
extern sCRAFTITEM_SERVER sAging_Recv; //서버에서 받을 아이템 


extern int ShowItemCraftMessageFlag; //조합된 아이템 인자를 보여준다 
extern int ShowItemAgingMessageFlag; //아이템이 에이징됬을때 메세지를 보여준다 
extern int ShowItemSmeltingMessageFlag; // pluto 아이템이 제련 됬을때
extern int ShowItemManufactureMessageFlag; // pluto 아이템이 제작 됬을때
extern int ShowItemOverSpace;	// pluto 공간부족

extern int TempShowMoney;	  //보여지기만 하는 돈 
extern int ItemPickUpFlag;	  //사기방지를 위해서 딜레이를 준다 	
extern int WareHouseSaveFlag;


extern cAGING	cAging;		  //에이징 
extern sAGINGITEM sAgingItem;
extern sAGINGITEM_INFO sAgingItem_Info[MAX_AGINGITEM_INFO];

extern int AgingLevelAttack[MAX_AGING+1];
extern int AgingLevelCritical[MAX_AGING+1];

extern char szAgingItemBuff[128];
extern POINT CraftItemMessageSize;

extern int AgingCheckSheltomFlag; //에이징 가능 플랙 초기화 
extern int AgingSheltomCnt2;
extern int MixItemNoCopyFlag;
extern int CheckMixItemCopyFlag;
extern sMAKEITEM_INFO sMakeItem_Info[MAX_MAKEITEM_INFO];

// 믹스쳐 리셋 객체 및 변수들 전역화 하기
extern cMIXTURERESET cMixtureReset;						// 석지용 - 믹스쳐 리셋 객체
extern sMIXTURE_RESET_ITEM	sMixtureResetItem;			// 석지용 - 믹스쳐 리셋 아이템 정보 구조체
extern sMIXTURE_RESET_ITEM sMixtureResetItemBackUp;		// 석지용 - 믹스쳐 리셋 아이템 정보 구조체 백업
extern int	MixtureResetCheckFlag;						// 석지용 - 믹스쳐 리셋이 가능한가?
extern sMIXTURE_RESET_ITEM_SERVER	sMixtureReset_Send;	// 석지용 - 믹스쳐 리셋 서버로 전송할 구조체
extern sMIXTURE_RESET_ITEM_SERVER	sMixtureReset_Recv;	// 석지용 - 믹스쳐 리셋 서버에서 받을 구조체

extern char *PostBoxDocFilePath[];

extern sPOSTBOX_ITEM sPostbox_Item[50];
extern int PostBoxIndexCnt;
extern DWORD ExpressItemCode;
extern char  szExpressItemName[64]; //받을 아이템 이름 

extern sCRAFTITEM_SERVER sWingItem_Send; //서버로 보내질 윙아이템
extern sCRAFTITEM_SERVER sWingItem_Recv; //서버에서 받을 아이템 (험 필요한가 멀겠네)

extern int DownTradePosi[6][4];
extern DWORD SheltomCode2[];
extern DWORD MagicSheltomCode[]; // 박재원 - 매직 포스 추가
extern DWORD BillingMagicSheltomCode[]; // 박재원 - 빌링 매직 포스 추가
extern int ForceItemPrice2;
extern sRECON_ITEM sReconItem[MAX_SEEL_COUNT]; // 박재원 - 테이와즈 씰 추가
extern int		MixCancelButtonDelayFlag; // pluto 외부 변수로 바꿨다 -_-