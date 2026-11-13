/*----------------------------------------------------------------------------*
*	파일명 :  sinClassInven.h
*	하는일 :  인벤토리의 모든 설정을 관리한다 
*	작성일 :  최종업데이트 12월 
*	적성자 :  박상열 
*-----------------------------------------------------------------------------*/	
#define INVENTORY_MAXITEM		100 //인벤토리가 가질수 있는 최대아이템 
#define INVENTORY_BOX_SIZE		22  //아이템 박스의 크기 
#define INVENTORY_BOX_COL		6   //아이템 박스의 갯수 (세로)
#define INVENTORY_BOX_ROW		12  //아이템 박스의 갯수 (가로)

// 13 + the 3215 slots: 14 costume, 15/16 premium rings (sinInvenTory.cpp sInven)
#define INVENTORY_MAX_POS		16    //창고를 제외한 10곳의 아이템셋팅 포지션 뒤의 3은 임시이다 RECT만 사용한다
#define INVENTORY_POS_SLOT_COSTUME	13	// sInven index of the costume slot (ItemPosition 14)
#define INVENTORY_POS_SLOT_PRING	14	// the two premium ring slots: 14, 15 (ItemPosition 15, 16)
#define MAX_WINGQUEST			10  //윙 종류가 설마 10개 넘겠어 --;


#define		NO_BOX_COLOR				0
#define		SET_ITEM_CHECK_COLOR		1
#define		OVERLAP_ITEM_COLOR			2
#define		NOT_SETTING_COLOR			3
#define		OVERLAP_BOX_COLOR           4
#define		SELECT_ITEM_COLOR			5
#define     NOT_USE_COLOR				6
#define		NOT_AGING_ITEM				7
#define     NOT_ENOUGH_MONEY			8	//돈부족 


#define     MAX_USESKILL                17 

struct sINVENTORY{
	DWORD  Position;    //셋팅될 위치 
	RECT   Rect;        //위치 좌표 
	int    ItemIndex;   //인벤 포지션에 저장되어있는 아이템 인덱스 ItemIndex가 있으면 사용중 없으면 미사용 
	int    WeaponClass; // 현제 사용무기가 근접인지 발사형인지 

};

#define		MAX_INVEN_PARTICLE		200

#define		SIN_EFFECT_SHOW_INVEN		1
#define		SIN_EFFECT_SHOW_WAREHOUSE	2
#define		SIN_EFFECT_SHOW_TRADE		3
#define		SIN_EFFECT_SHOW_GYUNGFUM	10

/*
#define   sITEMINFO_BIONIC			0	//생체 
#define   sITEMINFO_EARTH			1	//대자연 
#define   sITEMINFO_FIRE			2	//불 
#define	  sITEMINFO_ICE				3	//냉기 
#define   sITEMINFO_LIGHTING		4	//번개 
#define   sITEMINFO_POISON			5	//독 
#define   sITEMINFO_WATER			6	//물 
#define   sITEMINFO_WIND			7	//바람 
#define	  sITEMINFO_NONE			15	//속성 무시
*/


////////속성 데미지 구조체
struct sELEMENT_ATTACK{
	int Bio[2];
	int Fire[2];
	int Ice[2];
	int Lighting[2];
	int Posion[2];
};

////////인벤토리 이펙트(파티클) 구조체
struct sINVENTORY_EFFECT{
	smFACE2D InvenFace;
	int Time;
	int Max_Time;
	POINT Speed;
	int Direction;
	int AniCount;
	int FadeType;
	int Kind;		//어디에서 나올 이펙트인가 
	int Flag;
	POINT DesPosi;
	POINT Posi;

};

//유니온 코어 등록 이펙트 구조체 
struct sUNIONCORE_EFFECT{
	int Flag;
	int Time;
	int Max_Time;
	int MatNum;
	int x;
	int y;
	int Alpha;
	int w;
	int h;

};

struct sWINGQUEST{
	DWORD NeedItemCode[6];
	int   Money;
	DWORD ChangeItemCode;

};

class cINVENTORY{

public:

	int OpenFlag;       //포인터 로 접근하기 때문에 위치가 절대루 바뀌면 안된다 

	sITEM InvenItem[INVENTORY_MAXITEM];

	sITEM InvenItemTemp[INVENTORY_MAXITEM];        //박스에 있는 아이템을 잠시 보관한다 (0-79 Box 80-83 Posi) 
	sITEM BackUpInvenItemTemp[INVENTORY_MAXITEM];  //박스에 있는 아이템을 잠시 보관한다 (0-79 Box 80-83 Posi) 


	char JobName[32];

	LPDIRECTDRAWSURFACE4	lpInvenArrow[2];
	LPDIRECTDRAWSURFACE4	lpCoverSmall;
	LPDIRECTDRAWSURFACE4	lpCoverMiddle;
	LPDIRECTDRAWSURFACE4	lpCoverLarge;
	LPDIRECTDRAWSURFACE4	lpMyShopIcon;
	LPDIRECTDRAWSURFACE4	lpMyShopIcon_T;



	LPDIRECTDRAWSURFACE4	lpPotionBack;
	LPDIRECTDRAWSURFACE4	lpChangeABInfo[2];
	LPDIRECTDRAWSURFACE4	lpExitInfo;
	LPDIRECTDRAWSURFACE4	lpWeightInfo;
	LPDIRECTDRAWSURFACE4	lpItemABInfo[2];

	LPDIRECTDRAWSURFACE4	lpShowWeaponClass[4];

	LPDIRECTDRAWSURFACE4	lpIndiShopIcon;


	LPDIRECTDRAWSURFACE4   lpPuzzleInven;     //퍼즐 칸
	LPDIRECTDRAWSURFACE4   lpPuzzlewidth;     //가로퍼즐 잡을때 마우스이미지 
	LPDIRECTDRAWSURFACE4   lpPuzzleHeight;    //세로퍼즐 잡을때 마우스이미지 




	int MatInven[4];   
	int MatInvenParticle[5];

	DWORD InvenDataCheckSum;	//데이타에 대한 체크섬 

	/////////////// 보여지는 데미지 올려주는 변수
	int ViewDamagePercent;
	int ViewDamageNum;

	//
	int iEventBuyItemKind;
	
public:
	cINVENTORY();
	~cINVENTORY();

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
	void KeyDown(); //키 체크 

	///////////////////////////
	int LoadItemInfo(); //아이템 정보를 로드한후 저장한다  
	int AutoSetInvenItem(sITEM *pItem , int PostBox=0); //아이템을 자동 셋팅한다 

	int InvenEmptyAearCheck(sITEM *pItem);//무기영역이 아닌 박스 영역에 셋팅한다 

	int SetItemToChar(int CheckHackFlag=0); //아이템의 능력치를 캐릭터에 셋팅한다 
	int LastSetInvenItem(sITEM *pItem, int AutoSet=0); //아이템을 인벤토리에 셋팅한다 

	int PickUpInvenItem(int x , int y, int PickUpFlag=0); //아이템을 집는다 

	int ChangeInvenItem(sITEM *pItem); //아이템을 바꾼다 

	int CrashInvenItem(RECT &desRect ,int PassItemIndex=0 ); //아이템끼리의 충돌영역을 체크한다
														   //PassItemIndex로 두개이상 아이템이 겹쳐있는지를 체크한다

	////////////////////////// 아이템 Load & Release
	void InvenItemLoad();    //아이템을 Load 한다 
	void InvenItemRelease(); //아이템을 Release 한다  


	////////////////////////// 체크 
	int CheckRequireItem();  //아이템이 사용가능한지여부를 체크한다 
	void CheckWeight();      //무게를 체크한다 
	int CheckRequireItemToSet(sITEM *pItem); //셋팅할수 있는지를 체크한다 

	int InvenNotOpenCheck(sITEM *pItem); //인벤토리에 로직의 수행여부 


	//////////////////////////TwoHand 무기 관련 
	int SetTwoHandItem(sITEM *pItem);     //양손   무기를 셋팅한다 
	int PickUpTwoHandItem(sITEM *pItem);  //투핸드 무기를 집는다 
	int OverlapTwoHandItem(sITEM *pItem); //투핸드 무기로 겹치는 아이템을 체크한다 
	int OverlapTwoHandSwitch(sITEM *pItem);//오른손에 있을때 왼손을 체크 왼손은 오른손 체크  
	int ClearTwoHandPosiAndItem(sITEM *pItem); //투핸드 아이템을 지워준다 

	
	//////////////////////////Potion  관련 
	int AutoSetPotion(sITEM *pItem);      //포션 자동셋팅 
	int MouseSetPotion(sITEM *pItem);     //마우스로 포션을 셋팅한다 
	int AutoSetPotionToBox(sITEM *pItem); //포션을 창고에 셋팅한다 
	int cINVENTORY::ReSettingPotion();    //포션공간이 부족할경우에는 나머지 포션을 창고에 셋팅한다 
	int UsePotion(int x , int y);         //포션을 사용한다 
	int CheckOverlapItem(sITEM *pItem, int PassItemIndex);   //겹쳐진 아이템을 빈곳을 찾아 셋팅한다

	/////////////////////////캐릭터 관련 
	int SearchJobAndSetting(); //현재의 직업을 찾아서 저장한다 
	void LoadCharState();      //캐릭터의 상태를 로드한다 

	////////////////////////AB 포지션 관련 
	int ChangeABItem(int Posi);


	int DrawInvenText(); //인벤토리 Text를 표시한다 

	int SetInvenItemAreaCheck(sITEM *pItem); //아이템이 들어갈 영역을 체크한다
	int InitColorRect(); //컬럭 박스 RECT 초기화 
	int CheckInvenPos(int x , int y , int lx ,int ly , RECT &rect); //아이템이 놓일 영역 충돌체크 


	/////////////////////////
	int ThrowInvenItemToField(sITEM *pItem); //아이템을 버린다 
	int NoSettingThrowItem(sITEM *pItem ,int Flag= 0);    //셋팅실패시 아이템을 버린다 
	
	/////////////////////////아이템 수리관련 
	sITEMPRICE GetInvenItemPrice(sITEM *pItem);//아이템의 가격정보를 얻어온다 
	int RepairInvenItem(sITEM *pItem , int RepairCost);//아이템을 수리한다 
	
	int CheckSetOk(sITEM *pItem,int AutoFlag=0); //셋팅할수있는지를 체크한다 

	//////////////////////// 아이템 Cheat 관련 
	int CopyItemClear(sITEM *pItem);//복사된 아이템을 없앤다  
	POINT CheckImageSize(LPDIRECTDRAWSURFACE4 lpdd); //그림의 사이즈를 구해서 리턴한다 
	int ImsiCheckItemImageSize();  //임시로 아이템의 사이즈를 체크한다 

	/////////////////////// 잘못된 좌표를 보정해준다 
	int ReSetPotionXY(sITEM *pItem,int kind=0);

	////////////////////// 귀환 아아템 
	int UseEcore(sITEM *pItem);

	////////////////////// 마우스 아이템 복귀
	int ResetMouseItem();

	//////////////////////아이템 조작을 방지하기위해 보안을 강화한다
	int CheckInvenItemForm();   //아이템의 데이타가 맞는지 체크 
	int ReFormInvenItem();		//맞으면 데이타를 다시 셋팅 	

	////////////////////// 돈에 대한 무개 제한
	int CheckMoneyLimit(int Money);
	
	////////////////////// 포션 갯수 조작방지
	int CheckPotionNumForm();
	int ReFormPotionNum();

	////////////////////// 집을수 없거 그외의 특별한 아이템을 체크한다 
	int CheckSpecialItem(sITEM *pItem);

	////////////////////// InvenTory 이펙트 
	int ShowInvenEffectParticle();
	int SetInvenParticle(int x , int y , int Kind );
	int sinDrawInvenTexImage(smFACE2D *Face);
	int MoveInvenParticle(); //파티클을 움직인다 


	////////////////////// 인벤토리에 아이템을 셋팅할수있는 공간이 있나를 체크한다 
	int CheckSetEmptyArea(sITEM *pItem);

	////////////////////// 아이템 새로운 인증
	int ResetInvenItemToServer(DWORD CODE ,DWORD Head , DWORD CheckSum , DWORD Time );  //인증
	int DeleteInvenItemToServer(DWORD CODE ,DWORD Head , DWORD CheckSum);				//삭제 


	///////////// 이벤트 (송편을 먹는다 )
	int EatSongPyeun(sITEM *pItem);


	//////////// 직업별 아이템을 구분한다 
	int CharOnlySetItem(sITEM *pItem);

	//////////// 캐릭터 스텟 조작을 검사한후 서버로 보낸다 
	int HackCharStateCheck(smCHAR_INFO *pChar_Info , smCHAR_INFO *desChar_Info);

	//// 잘못배치된 아이템을 재배치한다
	int CheckResetInvenItem();

	//같은 아이템이 있나없나 찾아본다 
	int SearchItemCode(DWORD CODE , int KindFlag=0);
	
	//최대데미지와 크리티컬을 체크해서 서버로보내준다  
	int CheckDamage();

	//퀘스트아이템을 체크해서 Clear한다
	int ClearQuestItem();

	//링크코어를 서버에서 받는다
	int RecvLinkCore(sITEMINFO *pItem);

	//마우스 포션을 체크한다
	int CheckMousePotionNumForm();
	
	//마우스의 포션을 인증한다 
	int ReFormMousePotionNum();

	//유니온 코어 이펙트를 셋팅한다 
	int SetUnionCoreEffect(int x , int y);
	int UnionCoreParticleProcess();
	int UnionCoreParticle(int x , int y);
	int UnionCoreParticle2(int x , int y , int Flag = 0); //터지는 이펙트 

	//인벤토리에 셋팅될수있는지 체크한다
	int CheckInvenEmpty(sITEM *pItem);

	//sItemInfo로 아이템을 인벤토리에 셋팅한다
	int SetInvenToItemInfo(sITEMINFO *pItemInfo);

	//돈과 경험치의 이미지를 로드한다
	int LoadMoneyExpImage(sITEM *pItem);

	//윙아이템 퀘스트 
	int sinWingQuestNpc(); 

	int SendWingQuestItem(int Index); //윙아이템 퀘스트 

	//한번더 사용가능한 아이템인지를 체크한다
	int CheckRequireItemToSet2(sITEM *pItem);

	//특별한 아이템을 퀘스트 인벤토리에 셋팅한다
	int SetSpecialItem(sITEM *pItem);

	//Special 아이템을 보여준다
	int ChangeSpecialItem(int Flag = 0);

	//Special 아이템 소팅
	int CheckSpecialItemPosi();

	//코드나 아이템 종류로 인벤토리 아이템 인덱스를 리턴한다
	int SearchItemIndex(DWORD CODE , int KindFlag);
	
	//복주머니를 퍼즐로 바꾼다
	int ChangePuzzle(sITEM *pItem);

	//퍼즐을 체크한다
	int CheckPuzzle();

	//퍼즐을 보낸다
	int CheckSendPuzzle();

	//퍼즐 NPC
	int SendPuzzleNpc();

	//보여지는 데미지를 조정한다 
	int IncreViewDamage(int Persent , int Num=0);

	//바벨의 뿔을 체크해서 서버로 보낸다
	int SendEventItem();

	//바벨의 뿔을 체크한다
	int CheckEventItem();

	//ForceOrb의 이펙트를 그린다
	int DrawForceOrbEffect(int x , int y);

	//ForceOrb를 사용한다
	int UseForceOrb(sITEM *pItem , int Flag = 0);

	// 박재원 - 매직 포스 추가(매직 포스를 사용한다)
	int UseMagicForceOrb(sITEM *pItem , int Flag = 0);

	// 박재원 - 빌링 매직 포스 추가(빌링 매직 포스를 사용한다)
	int UseBillingMagicForceOrb(sITEM *pItem , int Flag = 0);

	//ForceOrb를 셋팅한다(접속종료및 끊어짐)
	int SetForceOrb(DWORD ItemCODE , int Time);

	// 박재원 - 매직 포스 추가 (매직 포스를 셋팅한다.)
	int SetMagicForceOrb(DWORD ItemCODE , int Time);

	// 박재원 - 빌링 매직 포스 추가 (빌링 매직 포스를 셋팅한다.)
	int SetBillingMagicForceOrb(DWORD ItemCODE , int Time);
	
	//박재원 - 캡슐 아이템을 사용한다.(호랑이 캡슐 사용)
	int UseCapsuleItem(sITEM *pItem);



	/*----------------------------------------------------------------------------*
	* 작성자 : 하대용  			
	* Desc   : 추가돼는 함수들  		
	*-----------------------------------------------------------------------------*/
	//Fox아이템을 체크한다
	int CheckFoxEventItem();
	//나인테일 아물렛을 보낸다
	int SendFoxEventItem();
	//반짝반짝 가루 체크한다
	int CheckChristMasEventItem();
	//반짝반짝 가루를 보낸다
	int SendChristmasEventItem();
	//퀘스트 아이템의 좌표를 바꾼다
	int ReSetQuestItemPosi();
	//일곱가지 크리스탈의 보은 체크한다
	int CheckCristalEventItem();
	//일곱가지 크리스탈을 서버로 보낸다.
	int SendCristalEventItem();

	//박재원 - 수박 모아오기 수박 아이템 갯수를 체크한다.
	int CheckWatermelonEventItem();
	//박재원 - 수박 모아오기 수박 아이템을 서버로 보낸다
	int SendWatermelonEventItem();
	// 박재원 - 같은 아이템의 갯수를 리턴한다.(인벤에서 수박 갯수 리턴하기)
	int CheckItemCount(DWORD CODE);

	//장별 - 발렌타인 초콜렛 모아오기 수박 아이템 갯수를 체크한다.
	int CheckValentineEventItem();
	//장별 - 발렌타인 이벤트 // 발렌타인 초콜렛 아이템을 서버로 보낸다
	int SendValentineEventItem();
	// 장별 - 같은 아이템의 갯수를 리턴한다.(인벤에서 발렌타인 초콜렛 갯수 리턴하기)
	int CheckValentineItemCount(DWORD CODE);

	// 박재원 - 알파벳 조합 이벤트(알파벳의 개수를 체크한다.)
	int CheckPristonAlphabetEventItem();
	// 박재원 - 알파벳 조합 이벤트(알파벳 아이템을 서버로 보낸다)
	int SendPristonAlphabetEventItem();

	// 장별 - 캔디데이즈 캔디 모아오기 캔디 아이템 갯수를 체크한다.
	int CheckCandydaysEventItem();
	// 장별 - 캔디데이즈 이벤트 // 캔디데이즈 캔디 아이템을 서버로 보낸다
	int SendCandydaysEventItem();
	// 장별 - 같은 아이템의 갯수를 리턴한다.(인벤에서 캔디 갯수 리턴하기)
	int CheckCandyItemCount(DWORD CODE);

	// 장별 - 매지컬그린 - 아이템 갯수를 체크한다.
	int CheckMagicalGreenEventItem();
	// 장별 - 매지컬그린 - 아이템을 서버로 보낸다
	int SendMagicalGreenEventItem();
	int SendMagicalGreenEmeraldEventItem();
	// 장별 - 매지컬그린 - 같은 아이템의 갯수를 리턴한다.
	int CheckMagicalGreenItemCount(DWORD CODE);
	int CheckMagicalGreenItemEmeraldCount(DWORD CODE);

	// 장별 - 카라의 눈물 갯수를 체크한다.
	int CheckTearOfKaraEventItem();
	// 장별 - 카라의 눈물을 서버로 보낸다
	int SendTearOfKaraEventItem();
	// 장별 - 카라의 눈물 갯수 리턴하기
	int CheckTearOfKaraItemCount(DWORD CODE);

	// 박재원 - 2010 월드컵 이벤트 - 축구공 포션 사용하기
	int UseSoccerBallPotionItem(sITEM *pItem);

	// 장별 - 조사원을 찾아라
	int CheckFindinvestigatorEventItem();
	int CheckFindinvestigatorKamiuEventItem();
	int CheckFindinvestigatorEtoEventItem();
	int SendFindinvestigatorNineEventItem();
	int SendFindinvestigatorTaleEventItem();
	int SendFindinvestigatorTaleEtoEventItem();
	int CheckFindinvestigatorNineItemCount(DWORD CODE);
	int CheckFindinvestigatorTaleItemCount(DWORD CODE);




	int DeleteCoupleRing();		// pluto 커플링

	// 장별 - 입력창
	char szDoc[128];     // 입력받은 글
	char szSendDoc[128]; //서버로 보낼 글


};

extern cINVENTORY	cInvenTory;

extern sINVENTORY sInven[INVENTORY_MAX_POS];

extern DWORD sinInvenColor[7];

extern sITEM *pUsePotion; //사용할 포션의 포인터 

extern int sinLoadOK; //로딩 완료시 값을 셋팅 

extern int LbuttonUpCheckFlag;

extern int sinChangeSetFlag; //무기 셋팅 체인지 플랙 

extern int SelectInvenItemIndex;

extern int CrashItemIndex[2];

extern int 	AutoSetItemIndex;

extern int SelectPotionIndex;

extern int ArrowState[2];

extern int InvenArrowPosi;

extern int sinButtonFlag;

extern sINVENTORY_EFFECT	sInven_Effect[MAX_INVEN_PARTICLE];	//인벤 파티클 

extern int ArrowState[2];

extern int InvenArrowPosi;

extern int sinUndeadAbsorb;

extern float   sinAdd_fMagic_Mastery;
extern int	   sinInterHeight2;

extern sELEMENT_ATTACK	sElement_Attack;
extern int sinTempDamage2[2];
extern int sinTempAbsorb;
extern int sinTempBlock;
///////////임시 Extern 나중에 지울것
extern int SendCheckItemToServer(sITEMINFO *lpItem);

extern sITEMINFO TempLinkCoreItem;

extern POINT UnionCorePosi;

extern LPDIRECTDRAWSURFACE4   lpGold7;
extern LPDIRECTDRAWSURFACE4   lpExp7;

extern sWINGQUEST sWingQuest[MAX_WINGQUEST];
extern int   sinOverWeightCnt;

extern int SpecialItemShowFlag;
extern sITEM MyShopMouseItem;
extern int sinMyShopItemPass;
extern int NotChangeSetItemFlag;
extern int MatInterEffectAging[5];
extern int MatInterEffectAging2[4];
extern int MatSuccessEffectAging;
extern int ForceOrbUseTime[];
extern int ForceOrbDamage[];
extern int MagicForceOrbDamage[]; // 박재원 - 매직 포스 추가 공격력 
extern int BillingMagicForceOrbDamage[];  // 박재원 - 빌링 매직 포스 추가 공격력 
extern int MagicForceOrbUseTime[]; // 박재원 - 매직 포스 추가
extern int BillingMagicForceOrbUseTime[]; // 박재원 - 빌링 매직 포스 추가
extern int ForceOrbUseLevel[][2];
extern int MagicForceOrbUseLevel[][2]; // 박재원 - 매직 포스(사용 레벨)
extern int BillingMagicForceOrbUseLevel[][2]; // 박재원 - 빌링 매직 포스 추가

extern int BoosterItem_UseTime[]; // 박재원 - 부스터 아이템(생명력, 기력, 근력) 
extern int BoosterItem_DataPercent[]; // 박재원 - 부스터 아이템(생명력, 기력, 근력) 가중 퍼센트

extern int nSkillDelay_UseTime[]; // 장별 - 스킬 딜레이

// 장별 - 입력창
extern int nName;

//일곱개의 보은
extern DWORD CristalEventCODE[12];

//박재원 - 수박 모아오기
extern DWORD WatermelonEventCODE;

//장별 - 발렌타인 초콜릿 모아오기
extern DWORD ValentineEventCODE;

// 박재원 - 알파벳 조합 이벤트
extern DWORD PristonAlphabetEventCODE[7];

// 장별 - 캔디데이즈 캔디 모아오기
extern DWORD CandydaysEventCODE;

// 장별 - 매지컬그린
extern DWORD MagicalGreenEventCODE[2];

// 장별 - 카라의 눈물 모아오기
extern DWORD TeatOfKaraEventCODE;

// 장별 - 조사원을 찾아라 
extern DWORD FindinvestigatorEventCODE[2];

//공성전 아이템 
extern DWORD dwCastlItemCODE[];
extern DWORD dwPremiumItemCode[];

// 박재원 - 부스터 아이템
extern DWORD dwBoosterItemCode[];

// 장별 - 스킬 딜레이
extern DWORD dwSkillDelayItemCode[];
