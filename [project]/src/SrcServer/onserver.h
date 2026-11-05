//PK가능한 레벨 
#define	LIMIT_PK_LEVEL	20				// pluto PK허용 필드 전부

#define AUTOPLAYER_MAX		2048
#define AUTOPLAYER_MASK		2047


//유료 아이템 기본 사용시간 설정
#define	PRIME_ITEM_TIME_THIRDEYES		(60*60*24)
#define	PRIME_ITEM_TIME_EXPUP			(60*60*24)

#define	PRIME_ITEM_TIME_THIRDEYESx7		(60*60*24*7)
#define	PRIME_ITEM_TIME_EXPUPx7			(60*60*24*7)

#define	PRIME_ITEM_TIME_VAMPCUSPID		(60*60*3)	// pluto 뱀피릭 커스핏 3시간
#define	PRIME_ITEM_TIME_MANARECHARG		(60*60*3)	// pluto 마나 리차징 포션 3시간

#define	PRIME_ITEM_TIME_VAMPCUSPIDx2	(60*60*24)	// pluto 뱀피릭 커스핏 1일
#define	PRIME_ITEM_TIME_MANARECHARGx2	(60*60*24)	// pluto 마나 리차징 포션 1일

#define	PRIME_ITEM_TIME_PACKAGE1		(60*60*3)
#define	PRIME_ITEM_TIME_PACKAGE2		(60*60*24)
#define	PRIME_ITEM_TIME_PACKAGE3		(60*60*24*7)
#define	PRIME_ITEM_TIME_PACKAGE4		(60*60*24*30)
#define	PRIME_ITEM_TIME_PACKAGE5		(60*60*1) // 장별 - 경험치증가 포션(100% 1시간)


#define	PRIME_ITEM_PACKAGE_NONE			0
#define	PRIME_ITEM_PACKAGE_BRONZE		1
#define	PRIME_ITEM_PACKAGE_SILVER		2
#define	PRIME_ITEM_PACKAGE_GOLD			3
#define	PRIME_ITEM_PACKAGE_ULTRA		4

// pluto 마나 리듀스 포션
#define PRIME_ITEM_MIGHTOFAWELL			5
#define PRIME_ITEM_MANAREDUCE			6


//SOD 관련
#define	rsSOD_FIELD			30
#define	rsSOD_VILLAGE		9
#define	rsBABEL_FIELD		27
#define	rsKELVEZU_FIELD		36
#define	rsMOKOVA_FIELD		38
//공성
#define	rsCASTLE_FIELD	33
#define	rsACTION_FIELD	39

//퀘스트용 대전 격투장
#define	QUEST_ARENA_FIELD		32
#define	QUEST_ARENA_NPC_POS_X	(22091*fONE)
#define	QUEST_ARENA_NPC_POS_Z	(9673*fONE)


#define	WHERE_TRADE				300
#define	WHERE_GIFT_EXPRESS		600
#define	WHERE_AGINGITEM			400
#define	WHERE_CRAFTITEM			420
#define	WHERE_FORECORB			440
#define	WHERE_LEARNSKILL		460
#define	WHERE_WARPGATE			470
#define	WHERE_BUY_ITEM			200
#define	WHERE_BUY_POTION		210
#define	WHERE_SELL_ITEM			220
#define	WHERE_LOAD_PLAYER		100
#define	WHERE_THROW_ITEM		120
#define	WHERE_GET_ITEM			130
#define	WHERE_STAR_POINT		620
#define	WHERE_CLAN_MONEY		630
#define	WHERE_GIVE_MONEY		640
#define	WHERE_PERSONAL_SHOP		320
#define	WHERE_OPEN_WAREHOUES	500


#define STG_START_POINT_MAX			200

struct STG_CHAR_INFO {

	smCHAR_INFO	smCharInfo;					//기본 캐릭터 정보 이름 모델 레벨 등등

	int		PosiState;						//속성 종류 ( 적/NPC )

	int		StartFixed;						//시작 위치 고정이면 TRUE ( FALSE면 수시로 변경)
	POINT	StartPosi;						//시작 위치 ( 고정일 경우 )

};

struct STG_START_POINT {
	int state;
	int x,z;
};

#define STG_ITEM_MAX	1024

struct psITEM	{
//	int		ItemCode;
	int		state;
	int		Serial;
//	char	szName[32];

	sITEMINFO	ItemInfo;
};

struct	STG_ITEMS {
	int		state;
	int		x,y,z;
	psITEM	Item;
	DWORD	dwCreateTime;
	DWORD	dwLoseTime;
	DWORD	Level;
	int		BeginMode;
};

// pluto 제작 룬 조합
struct Rune_Formula
{
	DWORD Recipe;
	DWORD Rune[3];
};



#define FIX_CHAR_MAX	100

#define	STG_MONSTER_MAX		256




class STG_AREA {
public:
	DWORD	Head;

	smSTAGE3D			*lpStage;
	sFIELD				*lpField;

	char				szStartPointFile[64];
	char				szCharFixedFile[64];
	char				szCharMonsterFile[64];

	STG_START_POINT		StartPoint[STG_START_POINT_MAX];
	int					StartPointNearPlay[STG_START_POINT_MAX];
	int					StartPointMonCount[STG_START_POINT_MAX];	//해당 몬스터 카운터
	int					StartPointCnt;
	int					StartPointLastOpenCount;					//최근 열린 시작 포인트 카운터
	DWORD				dwStartPoint_OpenTime[STG_START_POINT_MAX];	//열린 최근 시간 기록

	int					StartPointOpenCount;						//전체 시작점 수
	int					StartPointActiveCount;						//활성화된 시작점 수

	STG_ITEMS			StgItems[STG_ITEM_MAX];					//맵상에 존재하는 아이템

	STG_CHAR_INFO		*lpStageCharInfo;

	smTRNAS_PLAYERINFO	TransCharFixed[FIX_CHAR_MAX];			//고정된 캐릭터 정보 ( NPC외 )

	rsSTG_MONSTER_LIST	rsMonsterList;							//출현 몬스터 테이블

	int					StgCharInfoCnt;

	int					ItemSettingCount;
	int					MonsterSettingCount;

	int					Counter;

	DWORD				FieldNightDay;							//밤 낮의 변화를 감지하기 위한 변수

	smCHAR				*lpCharMonster[STG_MONSTER_MAX];		//출현 몬스터 연결 테이블
	int					MonsterCount;							//몬스터 출현 카운터
	int					MonsterLastPoint;						//몬스터 테이블 마지막 포인트

	int					EvnMonster;								//몬스터 일시적으로 초과 오픈

	DWORD				Event_dwMonsterTime;					//이벤트 몬스터 설정 시간
	smCHAR_INFO			*Event_lpChrMonster;					//이벤트 몬스터 캐릭 포인터
	int					Event_MonsterPercentage;				//이벤트 몬스터 발생 확률

	DWORD				dwActiveMode;							//설정 상태 ( spp , spc 등등 )

	STG_AREA();
//	~STG_AREA();

	int	Close();										//클래스 닫기

	int LoadStage( char *szFile );						//배경 메시 로드 

	smCHAR *OpenMonster( STG_CHAR_INFO *lpStgCharInfo , int gGroup=1 );	//몬스터 출현
	int Main();

	int SendStartPos( rsPLAYINFO *lpPlayInfo );

	int LoadStartPoint( char *szFile );
	int SaveStartPoint( char *szFile );
	int FindStartPoint( int x, int z );					//시작 지점 검색
	int AddStartPoint( int x, int z, int state );		//시작 지점 추가
	int DeleteStartPoint( int x, int z );				//시작 지점 제거
	int SetStartPosChar( smCHAR *lpChar );				//새로운 출현 시작 위치를 찾아 설정 
	int SetStartPosNearChar( smCHAR *lpChar , int cx, int cy, int cz );	//특정 좌표 주변에 캐릭터 위치를 설치함 
	int SetPosNearChar( smCHAR *lpChar , int cx, int cy, int cz );	//특정 좌표 주변에 캐릭터를 이동함 
	int StartPosRandomize();							//출현 시작 위치 랜덤화 한다 

	int SendStartPointFlag();							//시작지점 깃발위치 전송

	smTRNAS_PLAYERINFO *FindCharInfoFixed( smTRNAS_PLAYERINFO *lpTransPlayerInfo );		//고정 캐릭터 정보 찾기
	int	AddCharInfoFixed( smTRNAS_PLAYERINFO *lpTransPlayerInfo );		//고정 캐릭터 정보 추가
	int	DeleteCharInfoFixed( smTRNAS_PLAYERINFO *lpTransPlayerInfo );	//고정 캐릭터 정보 제거
	int OpenNpc( smTRNAS_PLAYERINFO *lpTransPlayerInfo );				//NPC 캐릭터 출현
	int LoadCharInfoFixed( char *szFile );								//고정 캐릭터 정보 읽어오기
	int SaveCharInfoFixed( char *szFile );								//고정 캐릭터 정보 저장

	STG_ITEMS *FindItem( int x, int y, int z );							//아이템을 찾는다
	STG_ITEMS *AddItem( psITEM *lpItem , int x, int y, int z , int BeginMode=0 );	//아이템을 설치한다
	int	DeleteItem( int x, int y, int z );								//해당 위치의 아이템을 제거함
	int DeleteItem( STG_ITEMS *lpStgItem );								//해당 아이템을 제거함
	int SendStgItem( rsPLAYINFO *lpPlayInfo );							//아이템 정보 전송 
	int SendStgItems();													//아이템 정보를 각 유저들에게 전송
	int SendStgItem( rsPLAYINFO *lpPlayInfo , STG_ITEMS *lpStgItem );	//아이템 정보 1개만 전송 
	int SendStgItemToNearUsers( STG_ITEMS *lpStgItem );					//아이템 정보 1개만 전송 
	int SendStgItemToPartyUsers( rsPLAYINFO *lpPlayInfo , STG_ITEMS *lpStgItem );	//아이템 정보 1개를 근처의 파티유저들 한테 보냄 

	int	FindEmptyMonTable();											//빈 몬스터 테이블 번호 찾기
	int	AddMonTable( smCHAR *lpChar );									//몬스터 테이블에 포인터 추가
	int	DeleteMonTable( smCHAR *lpChar );								//몬스터 테이블에 포인터 제거


	int LoadEnemyList( char *szFile );
};

//StgArea 최대 갯수 
#define STAGE_AREA_MAX		128			// was 64: the field table now has the old client's maps too (fields 44+)

extern STG_AREA	*StageArea;				//Stg_Area 선언

extern DWORD		dwPlayServTime;
extern rsSERVER_CONFIG		rsServerConfig;				//서버 설정 구조
extern smCHAR		*chrAutoPlayer;					//[AUTOPLAYER_MAX];
extern rsPLAYINFO	*rsPlayInfo;				//[CONNECTMAX];

extern int Server_SqlReady;
extern HWND	hDebugWnd;
extern CRITICAL_SECTION	cSerSection;
extern int Server_SqlReady;

extern time_t		tServerTime;

extern DWORD		dwBabelPlayTime;			//거대몬스터 바벨 유지시간
extern int			BabelPlayField;				//거대몬스터 바벨 필드

extern DWORD		dwKelvezuPlayTime;			//거대몬스터 켈베쥬 유지시간
extern int			KelvezuPlayField;			//거대몬스터 켈베쥬 필드

///////////////////////////////////////////////////////////////////////

//블레스 캐슬 용병 제한수량 정의
#define srCASLTE_SOLDER_CLAN_MAX	20
#define srCASLTE_SOLDER_MAX			1
#define srCASLTE_SOLDER_KIND_MAX	5

//블레스 캐슬 정보 구조
struct	rsBLESS_CASTLE {

	int		CastleMode;
	DWORD	dwMasterClan;
	int		DefenceLevel;
	DWORD	dwLimitTime;
	int		Counter;

	int		Tax;				//현재 세율
	int		NextTax;			//다음 세율

	DWORD	dwBattleOverTime;	//전투 종료 시간
	int		SoulFountain_PotionCount;		//영혼의 샘터 물약 누적
	DWORD	dwSoulFountainTime;				//영혼의 샘터 물약 발생시간
	DWORD	dwSoulFountainTime2;			//영혼의 샘터 물약 보낼시간
	int		SoulFountain_LastUserCount;

	DWORD	dwStartTime;
	short	TimeSec[2];						//진행시간초[0] 종료시간초[1]	
	int		SelectedSkill;					//선택된 스킬

	int		Temp[10];

	smTRANS_BLESSCASTLE	BlessCastleSetup;
	rsUSER_LIST_TOP10	ClanTop10;
	ATTACK_DAMAGE_LIST	*lpAttackDamageList;
	char	bCrystalSolderCount[4];
	char	bCrystalTowerCount[4];
	smCHAR	*lpChar_ValhallaTower;
	DWORD	dwScoreLogTime;				//공성점수 저장시간

};

extern rsBLESS_CASTLE	rsBlessCastle;				//블래스 캐슬 공성 정보


struct	rsCURRENCY_LOG {	
	//사용된 돈 통화량 기록
	INT64	int64_CraftMoney;
	INT64	int64_AgingMoney;
	INT64	int64_ShopMoney;
	INT64	int64_ShopPotionMoney[3];

	INT64	int64_ForceOrbMoney;
	INT64	int64_WarpgateMoney;
	INT64	int64_LearnSkillMoney;
};

extern	rsCURRENCY_LOG	CurrencyLog;

//////////////////////////////////////////////////////////////////////////

#define	srMONSTER_KILL_QUE_MAX		1024
#define	srMONSTER_KILL_QUE_MASK		1023

struct	rsKILL_MONSTER_INFO {
	DWORD	dwAutoCharCode;
	DWORD	dwCharSoundCode;
	DWORD	dwMonObjectSerial;
	DWORD	dwUserObjectSerial;
	int		x,y,z,Area;
	int		PartyFlag;
	DWORD	dwParam[4];
};

struct	TRANS_KILL_MONSTER_INFO {
	int		size,code;
	DWORD	tCode;
	int		KillInfo_Count;
	int		Temp[4];
};

//////////////////////////////////////////////////////////////////////////

/////////////// 벨라트라 몬스터 배치 구조체 //////////////////////////////
//마지막 카운터
#define	rsHARDCORE_COUNT_END	8	
#define	hCORE_ROUND_MAX		9

struct	_sHCORE_MONSTER {
	char	szName[32];
	smCHAR_INFO	*lpCharInfo;
	int		perCount;
};

struct	sHCORE_MONSTER {

	_sHCORE_MONSTER	hCoreMonster1;
	_sHCORE_MONSTER	hCoreMonster2;
	_sHCORE_MONSTER	hCoreMonster3;
	_sHCORE_MONSTER	hCoreMonster4;
	_sHCORE_MONSTER	hCoreMonsterBoss;

	int		PerCount;
	int		StartHour;
	int		State;
	int		MaxMonster;
	int		DelayTime;			//시작시 딜레이 시간
};

///////////////////////////////////////////////////////////////////////////////

//데이타서버로 경유전송
int rsSendDataServer( smWINSOCK *lpsmSock , void *szData );
//퀘스트 진행 처리
int rsProcessQuest( rsPLAYINFO *lpPlayInfo , smCHAR *lpChar , int QuestCode , int QuestParam );
//상점 아이템 목록 클라이언트에 전송
int SendShopItemList( smWINSOCK *lpsmSock , smCHAR *lpChar );
//스킬 수련 NPC
int SendSkillMenu( smWINSOCK *lpsmSock , smCHAR *lpChar , int Mode );
//창고 열기
int SendOpenWareHouse( smWINSOCK *lpsmSock );
//아이템 조합창 열기
int SendOpenMixItem( smWINSOCK *lpsmSock , int MixFlag );
//모금함 열기
int SendOpenCollectMoney( smWINSOCK *lpsmSock );
//경품이벤트
int SendOpenEventGift( smWINSOCK *lpsmSock );
//에이징 창 열기
int SendOpenAgingItem( smWINSOCK *lpsmSock );
//클랜 메뉴 열기
int SendOpenClanMenu( smWINSOCK *lpsmSock );
//이벤트 NPC
int SendOpenEvent( smCHAR *lpChar , rsPLAYINFO *lpPlayInfo , int EventFlag );
//선물 배달
int SendOpenGiftExpress( rsPLAYINFO *lpPlayInfo , DWORD	dwItemCode , DWORD dwPassCode_Param );
// pluto 아이템 제련창 열기
int SendOpenSmelting( smWINSOCK *lpsmSock );
// pluto 아이템 제작창 열기
int SendOpenManufacture( smWINSOCK *lpsmSock );
//SoD 진행 상황 출력
TRANS_SOD_RESULT *rsSod_GetRealTimeScore( rsPLAYINFO *lpPlayInfo );
// 석지용 - 믹스쳐 리셋 창 열기
int SendOpenMixtureItemReset( smWINSOCK *lpsmSock );

//클라이언트 끈김 ( 크리티칼 사용 안함 )
int DisconnectUser( smWINSOCK *lpsmSock );
//해킹 시도 기록 파일로 남김
int RecordHackLogFile( rsPLAYINFO *lpPlayInfo , void *lpTransCommand );

//범위형 공격 판정
int RecvRangeAttack( rsPLAYINFO *lpPlayInfo , TRANS_SKIL_ATTACKDATA *lpTransSkilAttackData );
//범위형 공격 받음 ( 유저 공격 )
int RecvRangeAttackUser( rsPLAYINFO *lpPlayInfo , TRANS_SKIL_ATTACKDATA *lpTransSkilAttackData );


//해당 필드에서 캐릭터를 찾는다
smCHAR *srFindCharFromSerial( DWORD dwObjectSerial , int AreaNum );
//캐릭터를 찾는다 ( 나중에 빨리 칮을수 있게 코드화 시켜 바꿔 줘야 함 )
smCHAR *srFindCharFromSerial( DWORD dwObjectSerial );
//캐릭터를 찾는다 ( 나중에 빨리 칮을수 있게 코드화 시켜 바꿔 줘야 함 )
rsPLAYINFO *srFindUserFromSerial( DWORD dwObjectSerial );
//그랜드힐링
int rsPlayGrandHealing( rsPLAYINFO *lpPlayInfo , smTRANS_COMMAND *lpTransCommand );
//힐링
int	rsPlayHealing( smTRANS_COMMAND *lpTransCommand );

//빌링 남은 시간 표시
int srDisplayBillingTime( rsPLAYINFO *lpPlayInfo , int Add_BillingTime );

//주변 유저에게 명령 보냄
int	rsSendCommandUser( rsPLAYINFO *lpPlayInfo , smTRANS_COMMAND_EX	*lpTransCommandEx );
//주변 유저에게 명령 보냄
int	rsSendCommandUser2( rsPLAYINFO *lpPlayInfo , smTRANS_COMMAND *lpTransCommand );


//서버를 종료시킨다 ( 종료시키는데 걸리는 시간은 약 6~10분 정도 소요 )
int	rsShutDown();
//아이피 주소를 뒤집어서 변환
DWORD GetSwapIPCode( char *szIP );

//이벤트 로그온 성공처리
int rsEventLogOn_Sucess( rsPLAYINFO *lpPlayInfo );



//SOD 지역번호 구하기
int rsGetSoD_AreaCode( int x, int y, int z ) ;
//루시퍼 일반공격시 스킬도 같이 적용 (범위지정)
int rsSendRangeDamage( smCHAR *lpChar , rsPLAYINFO *lpMainPlayInfo , int Range , int SubDamgePers );
//몬스터 생명력 재생치 구하는 함수
int rsGetMonsterLifeRegen( smCHAR *lpChar );

//클랜 머니 기록
int	rsClanMoneyLog( rsPLAYINFO *lpPlayInfo , int Money , int RestMoney );


//서버 포션에 물약 추가
int rsAddServerPotion( rsPLAYINFO *lpPlayInfo , DWORD dwPotionCode , int PotionCount );
//서버 포션에서 물약 구하기
int rsGetServerPotion( rsPLAYINFO *lpPlayInfo , DWORD dwPotionCode );

//해킹 시도 기록 파일로 남김
int RecordHackLogFile( rsPLAYINFO *lpPlayInfo , void *lpBuff );


//스킬 소환몬스터 출현 시킴
smCHAR *OpenMonsterFromSkill( DWORD SkillCode , int x, int y, int z , rsPLAYINFO *lpPlayInfo , smCHAR *lpEnemyChar );
//소환연결몹 찾기
smCHAR *rsGetLinkChar( smCHAR *lpMasterChar );


//블래스캐슬 설정정보 수신
int rsRecvBlessCastInfo( rsPLAYINFO *lpPlayInfo , void *lpPacket );
//블래스캐슬 설정정보 송신
int rsSendBlessCastInfo( rsPLAYINFO *lpPlayInfo , int Mode );

//클랜 코드 구하기
DWORD GetClanCode( int ClassClanCode );


//다른서버에 업데이트 정보 보냄
int	rsUpdateServerParam( rsPLAYINFO *lpPlayInfo , DWORD dwUpdateCode , int Param1 , int Param2 , int TargetMode );


///////////////////////// 디버그용 출력 //////////////////////
int rsDebugMessageToClient( rsPLAYINFO *lpPlayInfo , char *szDmgMsg );	//디버그 메세지 출력


////////////////////////// 베트남 경험치 시간분배 /////////////////////////////
int rsGetExp_GameTime( rsPLAYINFO *lpPlayInfo );
// pluto 제작 룬 검사 // 장별 - 대장장이의 혼
DWORD ManufactureCheckRune( rsPLAYINFO *lpPlayInfo, DWORD RecipeCode, DWORD *pRuneCode );

////////////////////////// 중국 시간제한 누적시간에 따른 메세지 출력 ///////////////////
int rsDispay_GameTime( rsPLAYINFO *lpPlayInfo , DWORD dwTimeOld );

// 박재원 - 2010 월드컵 이벤트 - 축구공 포션 사용
int	rsUseSoccerBallPotionItem( DWORD dwItemCode , rsPLAYINFO *lpPlayInfo );

// 박재원 - 에이징내맘대로(에이징 아이템 만들기)
int SetAgingItem(sITEMINFO *pItem, int AgingNum);

int rsSendGameServer_PrimeItem2( rsPLAYINFO *lpPlayInfo );

// 박재원 - 빌링 도우미 펫 추가  // 박재원 - 근력 리듀스 포션 추가
// 박재원 - 메인 서버로 시간제 빌링 아이템 정보 보냄
// rsSendGameServer_PrimeItem3 -> (빌링 도우미 펫, 뱀피릭 커스핏 EX, 근력 리듀스 포션)
int rsSendGameServer_PrimeItem3( rsPLAYINFO *lpPlayInfo );