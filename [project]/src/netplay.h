
#ifndef _NET_PLAY_HEADER

#define	_NET_PLAY_HEADER

extern smWINSOCK	*smWsockServer;
extern smWINSOCK	*smWsockDataServer;

extern int ChatScrollPoint;			//채팅 스크롤 포인트
extern int DispChatMsgHeight;			//최근 표시된 채팅창 줄 높이
extern int DispChatMode;			//채팅 출력 모드

extern int BellatraEffectInitFlag;	//벨라트라 이펙트 초기화 플랙

extern char	szConnServerName[16];	//접속한 서버이름

extern int	Version_WareHouse;						//창고 저장데이타 버전
extern int	Version_CharInfo;						//캐릭데이타 저장데이타 버전

extern smTRANS_COMMAND	*lpTransVirtualPotion;		//가상 포션

extern DWORD	dwTime_ServerT ;		//접속시 시간 (서버측 Time_T )
extern DWORD	dwTime_ConnectMS ;		//접속시 시간 (클라이언트ms)

extern int		InitClanMode;			//클랜 초기화 모드

extern DWORD	dwYahooTime;			//야호 유지시간


//디바인 라이트닝 스킬선택 변수
extern DWORD	dwSkill_DivineLightning_Target[8];
extern int		DivineLightning_Target_Count;
extern int		DivineLightning_FindCount;


//자동 플레이어를 찾는다
smCHAR *FindAutoPlayer( DWORD dwObjectSerial );
// 유저번호로 찾는다
smCHAR *FindChrPlayer( DWORD dwObjectSerial );
//죽은파티유저를 찾는다
smCHAR *FindDeadPartyUser();
//최근접 몬스터를 찾는다
smCHAR *FindNearMonster( smCHAR *lpCurChar );


//몬스터 코드 큐에 관리
int ClearAttMonsterCode();
int	AddAttMonsterCode( DWORD dwCode );
int	CheckAttMonsterCode( DWORD dwCode );


//서버 코드로 소켓을 찾음
smWINSOCK *GetServerSock( int ServerCode );
//지역서버 소켓을 찾음
smWINSOCK *GetAreaServerSock();

//데이타 입수
int RecvPlayData( smTHREADSOCK *pData );
//메세지 큐의 메세지를 처리
int PlayRecvMessageQue();

//채팅창 필터링
int	SetChatMsgFilter( int mode );
//채팅 정보 화면에 표시
int DisplayChatMessage( HDC hdc , int x, int y , int MaxLine );
//채팅 정보 화면에 표시
int DisplayChatMessage2( HDC hdc , int x, int y , int StrMax , int LineMax );
int DisplayChatMessage3( HDC hdc , int x, int y , int StrMax , int LineMax , int Mode );
//채팅 저장 코맨드 복구 
int RecallSavedChatCommand( HWND hChatWnd , int Arrow );

//채팅 문자를 서버로 전송
int SendChatMessageToServer( char *szMessage );
//다른 유저와 연결 시킨다
int ConnectOtherPlayer( DWORD dwIP );
//넷 플레이 메인
int NetWorkPlay();
//채팅창에 문자 삽입
int AddChatBuff( char *szMsg , DWORD dwIP );
//채팅창에 문자 삽입
int AddChatBuff( char *szMsg );
void ChatBar_Draw();				// the chat's scroll bar (netplay.cpp)
int ChatBar_LButtonDown();
int ChatBar_Wheel( int zDelta );
//코드 암호 첵크섬 만들기
DWORD EncodeParam( DWORD Param );

//아이템 버리기
int ThrowPutItem( sITEM *lpItem , int x, int y , int z );
//아이템 버리기
int ThrowPutItem2( sITEMINFO *lpItem , int x, int y , int z );

//몬스터 시작지점 제거
int SendDeleteStartPoint( int x, int z );
//몬스터 시작 지점 추가
int SendAddStartPoint( int x, int z );
//NPC 캐릭터 추가
int SendAdd_Npc( smCHAR *lpCharSample , int state );
//NPC 캐릭터 제거
int SendDelete_Npc( smCHAR *lpCharSample );
//저장된 캐릭터 데이타 불러오기 요구
int SendGetRecordData( char *szName );


//게임을 저장
int	SaveGameData();
//게임 저장 상태 ( FALSE 저장중 ( 서버로부터 결과를 받지 못함 ) )
int GetSaveResult();

//유저 정보 관련 통신 코멘드
int	TransUserCommand ( DWORD dwCode , char *szID , char *szName );

//캐릭터 정보 전송 유구
int	Send_GetCharInfo( DWORD	dwObjectSerial , int ServerCode );
//데드락 첵크 ( 메세지가 돌아 오지 않으면 네트웍 다운 )
int	TransCheckNetwork();

//서버에 관리자 모드 설정
int SendAdminMode( DWORD AdminNum );
//서버에 관리자 모드 설정
int SendAdminMode2( DWORD AdminNum );
//서버에 관리자 모드 설정
int SendAdminMode( DWORD AdminNum , smWINSOCK *lpsmSock );


//서버에 네트워크 풀질 모드 설정
int SendNetworkQuality( DWORD NetworkQuality );
//IP접속 주소를 군 서버에 보낸다
int SendCheckIP( int Code , smWINSOCK *lpsmSock );

//서버에 오브젝트 번호 설정
int SendSetObjectSerial( DWORD dwObjectSerial );
//서버에 오브젝트 번호 설정 ( 처음 한번만 )
int SendSetObjectSerial_First( DWORD dwObjectSerial );

//데이타 서버에 아이템 저장 요구 ( 복사 아이템 방지 )
int SendSaveThrowItem( sITEMINFO *lpItemInfo );
//데이타 서버에 아이템 저장 요구 ( 복사 아이템 방지 )
int SendSaveThrowItem2( sITEMINFO *lpItemInfo );
//데이타 서버에 돈 저장 요구 ( 복사 아이템 방지 )
int SendSaveMoney();
//서버에 재접속 시도
int SendContinueServer( DWORD dwObjectSerial , smWINSOCK *lpsmSock );
//크랙 첵크 신고
int CheckCracker();
//크랙 첵크
int CheckCracker(TRANS_FIND_CRACKER *lpTransFindCrack);
//해킹 시도한 유저 자동 신고
int SendSetHackUser( int StopGame );
//해킹 시도한 유저 자동 신고
int SendSetHackUser2( int Type , int LParam );
//해킹 시도한 유저 자동 신고
int SendSetHackUser3( int Type , int LParam , int SParam );
//복사 아이템을 소지한 사람 신고 
int SendCopyItemUser( int CopyItemCount );
//다른 캐릭터 카피 전송 요구
int SendCopyOtherUser( char *szName , int Mode );
//서버에 변경된 캐릭터 정보 보냄
int SendPlayUpdateInfo();
//서버에 변경된 클랜 정보 보냄
int SendUpdateClanInfo();
//몬스터 생성
int SendOpenMonster( int State );

//아이템 교환 신청
int SendRequestTrade( int dwObjectSerial , int Flag );
//교환창 아이템 송신
int SendTradeItem( DWORD dwSender );
//교환창 아이템 수신
int RecvTradeItems( TRANS_TRADEITEMS *lpTransTradeItems );
//아이템 교환 인증키 전송
int SendTradeSucessKey( DWORD dwSender);
//아이템 교환 내용 확인
int SendTradeCheckItem( DWORD dwSender );
//교환 거래 거리 확인
int GetTradeDistanceFromCode( DWORD dwObjectSerial );
//교환 거래 거리 확인
int GetTradeDistance( smCHAR *lpChar );

//범위형 공격
int Skil_RangeAttack( int x, int y, int z , int range , int power , int TargetState );
//범위형 공격( 유저만 공격 )
int Skil_RangeAttack_User( int x, int y, int z , int range , int power , int TargetState );
//범위형 공격
int Skil_RangeBoxAttack( smCHAR *lpCurChar , RECT *lpRect , int power , int TargetState , int UseAttackRating );
//범위형 공격 이펙트 연출
int Skil_RangeBoxEffect( smCHAR *lpCurChar , RECT *lpRect , DWORD dwSkillCode , int TargetState );
//범위형 공격 이펙트 연출
int Skil_RangeEffect( smCHAR *lpCurChar , int Range , DWORD dwSkillCode , int TargetState , int point );


//상대 캐릭터에 공격 가함 
int QuickSendTransAttack( smCHAR *lpChar , int power , int AttackState , int Resistance );
int QuickSendTransAttack( smCHAR *lpChar , int power );
int QuickSendTransAttack( smCHAR *lpChar , int power , int Resistance );



////////////////////////// 파티 관련 /////////////////////////
//파티 허락 
int SendJoinPartyUser( DWORD dwObjectCode );


///////////////////////// 창고 //////////////////////////////
//창고 저장
struct sWAREHOUSE;
int	SaveWareHouse( sWAREHOUSE *lpWareHouse , TRANS_WAREHOUSE *lpTransWareHouse );
int	SaveWareHouse( sWAREHOUSE *lpWareHouse  );
//창고 가져오기
int	LoadWareHouse( TRANS_WAREHOUSE *lpTransWareHouse , sWAREHOUSE *lpWareHouse , int flag );
int	LoadWareHouse( TRANS_WAREHOUSE *lpTransWareHouse , sWAREHOUSE *lpWareHouse );

//////////////////////// 기부금 //////////////////////////////
int SendCollectMoney( int Money );				//서버에 기부한 돈 보냄

/////////////////////// PK ////////////////////////////////////
//PK당해서 죽음
int PlayerKilling( DWORD dwObjectSerial );

//서버 접속
smWINSOCK *ConnectServer_Main();
//서버와의 연결 모두 끊음
int DisconnectServerFull();

//정보 서버 접속
smWINSOCK *ConnectServer_InfoMain();
//게임 서버 접속
smWINSOCK *ConnectServer_GameMain( char *szIP1, DWORD dwPort1, char *szIP2, DWORD dwPort2 );
//게임 서버 접속
smWINSOCK *ConnectServer_GameMain( char *szIP1, DWORD dwPort1, char *szIP2, DWORD dwPort2 , char *szIP3, DWORD dwPort3 );
//게임 서버 연결 끊음
int DisconnectServer_GameMain();


extern smWINSOCK	*smWsockServer ;					//NPC-몬스터-아이템
extern smWINSOCK	*smWsockDataServer ;				//데이타 저장 서버
extern smWINSOCK	*smWsockUserServer ;				//유저 플레이 데이타 교신 서버
extern smWINSOCK	*smWsockExtendServer ;				//필드확장 교신 서버

//서버 재연결 횟수
extern	int	ReconnDataServer;
extern	int	ReconnServer;

//서버와의 연결 끊어짐
// DisconnectFlag: non-zero = drop the connection soon (holds the time it was set).
// Wrapped so every place that sets it is logged to disconnect.log with the
// caller's address (look it up in game.map) - finds why the player gets kicked.
struct DISCONNECT_FLAG {
	int Value;
	DISCONNECT_FLAG &operator=( int v );
	operator int() const { return Value; }
};
extern DISCONNECT_FLAG	DisconnectFlag;
extern int	DisconnectServerCode;
//해킹 경고
extern int	WarningHack;

//트레이드 시도 플랙
extern int TradeSendSucessFlag;
extern DWORD	dwTradeMaskTime;
extern int	TradeItemSucessFlag;

extern	DWORD	dwLastRecvGameServerTime;		//최근 서버로 부터 패킷을 입수한 시간
extern	DWORD	dwLastRecvGameServerTime2;		//최근 서버로 부터 패킷을 입수한 시간
extern	DWORD	dwLastRecvGameServerTime3;		//최근 서버로 부터 패킷을 입수한 시간
extern	DWORD	dwLastRecvGameServerTime4;		//최근 서버로 부터 패킷을 입수한 시간


//////////////// PK 필드 정보 //////////////////
struct	PK_FIELD_STATE {
	DWORD	dwPlay_FieldTime;
	int		FieldCode;
};
extern PK_FIELD_STATE	PK_FieldState;		//Pk 필드 구조 정보체


//수신 시간 초과 캐릭터 종료
#define DIS_TIME_OVER		30000
//수신 시간 초과 캐릭터 비표시
#define DISPLAY_TRANS_TIME_OVER		7000
#define DISPLAY_TRANS_TIME_OVER_FAR	4000

//서버 구분 코드
#define PLAY_SERVER_CODE_MAIN		1
#define PLAY_SERVER_CODE_USER		2
#define PLAY_SERVER_CODE_EXTEND		3

#define PLAY_SERVER_CODE_AREA1		11
#define PLAY_SERVER_CODE_AREA2		12


extern char *szNetLogFile;
extern FILE *fpNetLog;

extern TRANS_SKIL_ATTACKDATA	Back_TransSkilAttackData;

//최근 받은 아이템
extern TRANS_ITEMINFO	TransRecvItem;


//경험치와 돈 거래 금액을 토탈에 기록한다
int	SetTotalAddMoney( int Money );
int	SetTotalSubMoney( int Money );
int	SetTotalAddExp( int Exp );
int	SetTotalSubExp( int Exp );
int GetTotalExp();
int	GetTotalMoney();
int GetTotalSubExp();


// pluto 제련 데이타를 서버로 보냄
int SendSmeltingItemToServer( void *lpsSmeltingItem_Send );
// pluto 제작 데이타를 서버로 보냄
int SendManufactureItemToServer( void *lpsManufactureItem_Send );
// 석지용 - 믹스쳐 리셋 아이템을 서버로 전송한다.
int SendMixtureResetItemToServer( void *lpsMixtureResetItem_Send );

//믹스쳐 데이타를 서버로 보냄
int SendCraftItemToServer( void *lpsCraftItem_Send );
//에이징 데이타를 서버로 보냄
int SendAgingItemToServer( void *lpsCraftItem_Send );
//에이징 완료된 아이템을 서버로 보냄
int SendAgingUpgradeItemToServer( sITEMINFO *lpItem );
//링크코어 등록 보냄
int SendLinkCoreToServer( sITEMINFO *lpItem );
//위프용 윙 아이템 데이타를 서버로 보냄
int SendWingItemToServer( void *lpsCraftItem_Send );

//판매할 아이템을 서버로 보냄
int	SellItemToServer( sITEMINFO *lpItem , int Count );

extern DWORD	dwLastCheckItemTime;
//아이템을 서버에 보내서 확인
int	SendCheckItemToServer( sITEMINFO *lpItem );
//인벤토리의 장착된 아이템을 서버로 보내서 검사
int CheckInvenItemToServer();


//서버에서 메모리 값을 정해서 가져간다
DWORD funcCheckMemSum( DWORD FuncPoint , int count );

//임시저장된 아이템 큐에 아이템 대기중인지 검사
int CheckRecvTrnsItemQue();
//사용한 아이템 코드 서버로 보냄
int SendUseItemCodeToServer( sITEMINFO *lpItem );

//주변 캐릭터에 이벤트 명령 코드 보내기
int	SendCommandUser( int wParam , int lParam , int sParam , int eParam );
//주변 캐릭터용 이벤트 명령 코드
int RecvCommandUser( smTRANS_COMMAND_EX *lpTransCommandEx );

//지역서버에 명령 코드 전송
int	SendCommand_AreaServer( DWORD dwCode , int wParam , int lParam , int sParam , int eParam );
//데이타서버에 명령 코드 전송
int	SendCommand_DataServer( DWORD dwCode , int wParam , int lParam , int sParam , int eParam );

//공격력 기록 포기화
int	Init_RecordDamage();
//보낸 공격력 기록
int	Record_SendDamage( smWINSOCK *lpsmSock , int Damage );
//받은 공격력 기록
int	Record_RecvDamage( smWINSOCK *lpsmSock , int Damage );

//파티원에게 스킬 사용 정보 보내기
int	SendPartySkillToServer( DWORD dwSkillCode , int SkillLevel , int Around , int wParam , int lParam , int sParam , int eParam );
//스킬 취소
int SendCancelSkillToServer( DWORD dwSkillCode , int lParam , int sParam , int eParam );
//서버에 스킬수행 보냄
int SendProcessSKillToServer( DWORD dwSkillCode , int point , int Param1, int Param2 );
//스킬 변경
int SendUpdateSkillToServer( DWORD dwSkillCode , int lParam , int sParam , int eParam );


//클랜 메세지 받기
int RecvClanJoinService( smTRANS_CHAR_COMMAND2 *lpTransCharCommand );
//클랜 메세지 보내기
int SendClanJoinService( DWORD dwMsgCode , smCHAR *lpChar  );
//클랜 가입 승인 메세지 보내기
int SendJoinClan();

//디바인 라이트닝 스킬 연출 ( 준비함수 - 주인공만 )
int	SkillPlay_DivineLightning_Select( smCHAR *lpChar , int SkillPoint );
//디바인 라이트닝 스킬 연출 ( 이펙트 실현 )
int SkillPlay_DivineLightning_Effect( smCHAR *lpChar , int SkillPoint );
//베놈 스피어 스킬 연출 ( 이펙트 실현 )
int SkillPlay_VenomSpear_Effect( smCHAR *lpChar , int SkillPoint );
//몬스터 스킬 이펙트 실현 ( 이펙트 실현 )
int SkillPlay_Monster_Effect( smCHAR *lpChar , int EffectKind , int Range );
//디바인 라이트닝 스킬 연출 ( 머미로드 )
int SkillPlay_MummyLord_Effect( smCHAR *lpChar , int Range );


//시간 구하기 (time_t)
DWORD	GetPlayTime_T();
//머리가 커지는 모드 설정
int	ChangeBigHeadMode( int Mode , int Time );

//클랜원 정보 해독
int	RecvClanCommand( TRANS_CLAN_COMMAND_USER *lpTransClanUser , _CLAN_USER_INFO *ClanUserInfo );

//팔곤 데미지 보내기
int SendFalconDamage( smCHAR *lpChar );
//댄싱소드 데미지 보내기
int SendDancingSwordDamage( smCHAR *lpChar );
//저랩용 펫 데미지
int SendLowLevelPetDamage( smCHAR *lpChar , int petType );
//PC방용 펫 데미지
int SendPCBangPetDamage( smCHAR *lpChar , int petType );

//메그네틱 스피어 데미지 보내기
int SendMegneticSphereDamage( smCHAR *lpChar );
//머스펠 데미지 보내기
int SendMuspellDamage( smCHAR *lpChar );


//최대 데미지 확인
int	SendMaxDamageToServer( WORD MinDamage , WORD MaxDamage , WORD Critical );
//최대 데미지 정보구조체에 설정
int SetMaxDamage( smCHAR_INFO *lpCharInfo );

//스킬보호값 전부 초기화
int ReformSkillInfo();
//스킬값 전부 확인
int SaveCheckSkill();

//아이템 택배 서비스 수신 요구
int	SendItemExpress( DWORD	dwItemCode , char *szPassCode );
//아이템 택배 서비스 수신 요구
int	SendItemExpress( DWORD	dwItemCode );

//야호 모션 서버로 보내기
int SendClanYahooMotion();
//별 포인트 정보를 서버에 보낸다
int	SendStarPointToServer( int Price , int Cash );
//기부한돈을 서버에 보낸다
int	SendGiveMoneyToServer( int Money );


#define	CLANMONEY_KIND_SOD		0
#define	CLANMONEY_KIND_CASTLE	1

//클랜머니 정보를 서버에 보낸다
int	SendClanMoneyToServer( int Money , int Flag , int Kind );
int	SendClanMoneyToServer( int Money , int Flag );
//이용요금 정보를 서버에 보낸다
int	SendPaymentMoneyToServer( int Money , int Flag );


//////////// 개인 상점 관련 함수 /////////////////
int SendOpenPersonalTrade( char *szTradeMsg , void *lpPersTrade );	//개인 상점 오픈
int SendClosePersonalTrade();										//상점 닫기
int	Send_ShowMyShopItem( smCHAR *lpChar );							//개인상점 아이템 내용 요구
int	Send_MyShopItemList( smTRANS_COMMAND	*lpTransCommand );		//개인상점 아이템 내용 상대방에 보내기
int	Recv_MyShopItemList( TRANS_TRADEITEMS *lpTransTradeItems );		//개인상점 아이템 내용 받음
int	Send_PersonalShopItem( DWORD dwCharCode , void *lpShopItem );	//개인 상점 물건 구입
int	UpdateMyShopList( void *lpMyShop );								//개인 상점 아이템 정보 업데이트

// 장별 - 입력창
int SendsServerDoc(char *szTradeMsg );



//날짜 확인 제거 아이템
int DeleteEventItem_TimeOut( sITEMINFO	*lpsItem );			//이벤트 종료하여 아이템 삭제

//퀘스트 명령 보내기
int SendQuestCommandToServer( DWORD dwQuestCode , int Param1, int Param2, int Param3 );

//대전 퀘스트 실행
int Start_QuestArena( DWORD dwQuestCode , int Param1, int Param2 );

//블래스캐슬 설정정보 서버에 보냄
int	SendBlessCastleToServer( smTRANS_BLESSCASTLE *lpBlessCastleSetup , int Mode );
//블래스캐슬 설정정보 수신
int RecvBlessCastInfo( void *lpPacket );
//블래스캐슬 세율 정보 서버에 요구
int	Send_GetBlessCastleTax();
//블레스 캐슬 마스터 클랜 설정
int SetBlessCastleMaster( DWORD dwClanCode , int Mode );

//속성정보 서버로 전송
int	SendResistanceToServer();


//인벤토리의 아이템 찾기 (코드)
sITEMINFO *FindItemFromCode( DWORD dwItemCode );

//설문조사 결과 서버에 보내기
int	SendPublicPollingToServer( int PollCode , int PollCnt , BYTE *bPollingData );

//핵툴 첵크용 NPC기능 구현시 서버에 검사통보
int HackTrap_CheckOpenFlag();
//핵툴 첵크용 함정
int	SendHackTrapToServer( DWORD dwTrapCode , int Param );

#ifdef _XIGNCODE_CLIENT
// 박재원 - XignCode
int Xigncode_Client_Recv(smWINSOCK *lpsmSock, XIGNCODE_PACKET *pack);
int Xigncode_Client_Start();
#endif

#endif