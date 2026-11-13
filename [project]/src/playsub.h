
//레벨 제한 120
//레벨 제한 110
#ifdef _LANGUAGE_VEITNAM
#define	CHAR_LEVEL_MAX	120
#else
#ifdef _LANGUAGE_BRAZIL
#define	CHAR_LEVEL_MAX	120
#else
#ifdef _LANGUAGE_ARGENTINA
#define	CHAR_LEVEL_MAX	110
#else
#ifdef _LANGUAGE_THAI
#define	CHAR_LEVEL_MAX	110
#else
#ifdef _LANGUAGE_CHINESE
#define	CHAR_LEVEL_MAX	120
#else
#ifdef	_LANGUAGE_JAPANESE
#define	CHAR_LEVEL_MAX	120
#else
#ifdef _LANGUAGE_PHILIPIN
#define	CHAR_LEVEL_MAX	120
#else
#ifdef _LANGUAGE_ENGLISH
#define	CHAR_LEVEL_MAX	200		// was 120 (raised 2026-11-12; LevelTable.h goes to 200)
#else
#define	CHAR_LEVEL_MAX	100
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif

#define sinNPC_SHOP				0x00000001	//상점 NPC
#define sinNPC_MIX				0x00000002	//조합 NPC
#define sinNPC_AGING			0x00000004	//에이징 NPC
#define sinNPC_WARE				0x00000008	//창고 NPC
#define sinNPC_SKILL			0x00000010	//스킬 NPC
#define sinNPC_FORCE			0x00000020	//포스 NPC
#define sinNPC_SMELTING			0x00000030	// pluto 제련 NPC
#define sinNPC_MANUFACTURE		0x00000040	// pluto 제작 NPC
#define sinNPC_MIXTURE_RESET	0x00000080	// 석지용 - 믹스쳐 리셋 NPC




////////////////////// 보호코드 //////////////////////////

extern DWORD	dwFuncChkTime;

DWORD Check_CodeSafe( DWORD dwStartFunc );		//코드보호
DWORD Check_CodeSafe2();						//코드보호2
DWORD	GetSafeCode();							//보호 코드 구하기
DWORD	GetSafeCode2();
int Code_VRamBuffOpen();
int	Mix_CodeVram();

extern char	szLastWhisperName[32];				//마지막 귓말전송한 사람
extern int	LastWhisperLen;
extern int	PlayerArmor;

extern int	PlayUsed_Resurrection;			//부활 아이템 사용

//갑옷 모양 바꾸기
int SetPlayerArmor( int armor );

char *GetFaceHairSkin( smCHAR *lpChar );		//얼굴 복구 // 얼큰이 오류 수정

/*
//////////////////////////////////////////////////////////////////////
//메모리 전체 첵크
int	CheckKeepMemFull();
//메모리 첵크
int	CheckKeepMem(DWORD ChkCount );
//메모리 첵크 종료
int CloseKeepMem();
//메모리 첵크 초기화
int	InitKeepMemFunc();
//////////////////////////////////////////////////////////////////////
*/
//////////////////////////// 물약 첵크 ////////////////////////////////


//인벤토리와 거래창에서 물약을 찾어 갯수 기록
int GetInvenPotionCount( sCHK_POTION_COUNT *ChkPotion );
//물약 갯수 새로 설정
int	ResetPotion();
//물약 갯수 새로 설정
int	ResetPotion2();
//물약갯수 비교
int	ComparePotion();
//물약갯수 기록 반전
int	XorCheckPotion( sCHK_POTION_COUNT *ChkPotion );
///////////////////////////////////////////////////////////////////////

//파티원 귓말 보내기
int	WhisperPartyPlayer( char *szName );
//클랜원 채팅 보내기
int	SetClanChatting();
//클랜 채팅 모드 설정
int IsClanChatting();

//경험치 추가
int AddExp( int Exp );
//다음 경험치 구하기
INT64 GetNextExp( int Level );
//레벨과 경험치가 맞는지 확인
int	CheckLevelExp( int Level , INT64 Exp );
//경험치로 레벨 추산
int	GetLevelFromExp( INT64 Exp );
//64비트 경험치 설정
int	SetExp64( smCHAR_INFO *smCharInfo , INT64 Exp64 );
//64비트 경험치 반환
INT64 GetExp64( smCHAR_INFO *smCharInfo );
//64비트 경험치 반환
INT64 GetXorExp64( smCHAR_INFO *smCharInfo );
//경험치 데이타를 반전
INT64 CodeXorCharInfo_Exp();

//필드 재시작 가능한지 확인
int CheckFieldContExp();


//귀환아이템 사용
int	ActionEtherCore( sITEM	*lpItem );

//죽어서 경험치 깍임
int DeadPlayerExp( smCHAR *lpChar );
//필드에서 재시작 캐릭터 경험치 추가 깍임
int ContGameExp( smCHAR *lpChar );
//방어율 계산
int GetPlayDefence( smCHAR_INFO *lpAttackCharInfo , smCHAR_INFO *lpDefenceCharInfo );

//날씨
int DrawWeather( int ax );
int InitWeatherSystem();
//로딩중 화면 표시
int OpenDispLoading();
//로딩 표시종료
int CloseDispLoading();
//나이나믹 패턴 삭제 추가
int AddRemoveDynPattern( smDPAT *lpDinaPattern );

//로딩 램프 제거
int CloseLoadingLamp();
//로딩램프
int InitLoadingLamp( HWND hWnd );

//자주사용하는 모델들 로드
int LoadOftenModels();

//자주쓰는 메시 로드
int	LoadOftenMeshPattern( char *szFileName );
//자주쓰는 갑옷 미리 로딩
int LoadOftenArmor();
//자주쓰는 아이템 미리 로딩
int LoadOftenItem();
//NPC 미리 로딩
int LoadOftenNpc();
//얼굴파일 미리 로딩
int LoadOftenFace();
//자주쓸 메시 텍스쳐 스왑첵크
int CheckOftenMeshTextureSwap();
//모델 스킨 변경
int	ChangeModelSkin( smCHAR *lpChar , int ComicArmor , int ComicHeadNum , char *szArmor , char *szHead );
//머리모양 바꿔버리기
int ChangeHairModel( int HairCode );


//필드 맵 초기화
int InitFieldMap();

//필드 맵 그리기
int MainFieldMap();
//필드 맵 그리기
int DrawFieldMap();

int GetFieldMapCursor();		//미니맵 위치에 마우스 유무

int psSetDefaultRenderState();


//데이타 압축 ( Z/NZ 방식 )
int EecodeCompress( BYTE *lpSrcData , BYTE *lpDestData , int size );
//압축데이타 해독 ( Z/NZ 방식 )
int DecodeCompress( BYTE *lpSrcData , BYTE *lpDestData );
//압축데이타 해독 ( Z/NZ 방식 )
int DecodeCompress( BYTE *lpSrcData , BYTE *lpDestData , int dCount );
//데이타 압축 ( Z/NZ 방식 )
int EecodeCompress( BYTE *lpSrcData , BYTE *lpDestData , int size , int DestSize );

//압축데이타 서버 아이템 해독용 ( Z/NZ 방식 )
int DecodeCompress_ItemPotion( rsPLAYINFO *lpPlayInfo , BYTE *lpSrcData , BYTE *lpDestData , int deCount , void *lpTransRecdata );


//다른 캐릭터와의 위치 겹치는지 확인
smCHAR *CheckOtherPlayPosi( int x, int y, int z );

//아이템 인증 받기
int ReformItem( sITEMINFO *lpItemInfo );
//아이템 인증 받기 ( 생성시간 보정 )
int ReformItem_AdjustTime( sITEMINFO *lpItemInfo , int TimeAdjust );

//아이템 인증 확인
int CheckItemForm( sITEMINFO *lpItemInfo );
//아이템 2개를 비교한다
int CompareItems( sITEMINFO *lpItemInfo1 , sITEMINFO *lpItemInfo2 );
//아이템 서버 인증만 받기
int	rsReformItem_Server( sITEMINFO *lpItemInfo );

//캐릭터 정보 인증 확인
int CheckCharForm();
//캐릭터 정보 인증 받기
int ReformCharForm();
//캐릭터 정보 인증 받기
int ReformCharForm( smCHAR_INFO *lpCharInfo );
//캐릭터 정보 인증 확인
int CheckCharForm( smCHAR_INFO *lpCharInfo );
//트레이드 정보 인증 받기
int ReformTrade( void *lpTrade );
//트레이드터 정보 인증 확인
int CheckTrade( void *lpTrade );

//아이템 에이징 인증 받기
int ReformMixItem( sITEMINFO *lpItemInfo );
//아이템 에이징 인증 확인
int CheckMixItem( sITEMINFO *lpItemInfo );

//흡수력 보정하여 구하기
float GetItemAbsorb( sITEMINFO *lpItemInfo );

//파티 가입여부 확인
int	RequestPartyPlayers( smCHAR	*lpChar );

int psSetDefaultRenderState();

//캐릭터 정보를 캐시데이타로 부터 얻는다
smTRNAS_PLAYERINFO	*GetCharInfoCache( char *lpData );
//캐릭터 정보를 캐시데이타에 저장
int SaveCharInfoCache( smTRNAS_PLAYERINFO *lpTransPlayInfo );

extern smCHAR	chrPartyPlayer;
//파티 플레이어 얼굴 오픈
int OpenPartyPlayer( char *szName , char *szModelFile1 , char *szModelFile2 );


//공격 비율 첵크
int	CheckPlayAttackCount();
//공격 받은 모션 카운터
int AttackMotionCountUp();
//공격 받은 패킷 카운터
int	AttackRecvCountUp();
//등록된 불량 아이템 검사
int	CheckCopyItem( sITEMINFO *lpItem );
//집업 변경 얼굴 변화
int ChangeJobFace();
// the costume slot (playsub.cpp): what the armour and costume slots hold, and which one the body shows
extern DWORD sinWornArmorCode, sinWornCostumeCode;
extern int sinCostumeView;
void sinRefreshBodyLook();
void sinToggleCostumeView();
// The face of a rank (ChangeJob): faces 0..3 are the first four ranks and 4..6 the big-head
// event faces (playmodel.h). The fifth rank is JOB_FACE_RANK5: ChangeModelSkin gives it the
// fifth rank's heads ("...d.inf", hair styles 01..03) and the fourth rank's head for the others.
#define JOB_FACE_RANK5			7
#define JOB_FACE( ChangeJob )	( (ChangeJob)>3 ? JOB_FACE_RANK5 : (ChangeJob) )


//예약 이펙트 초기화
int InitTimeEffect();
//이펙트 예약 추가
int AddTimeEffect( DWORD EffectCode , DWORD dwTime , int x, int y, int z );
int AddTimeEffect( DWORD EffectCode , DWORD dwTime , int x, int y, int z , smCHAR *lpChar );
//예약 이펙트 실행
int TimeEffectMain();


//회전 플레이어 포인터 선택
smCHAR *SelectRotPlayer( smCHAR *lpCurrentChar );
//회전 플레이어 포인터 종료
int	CloseRotPlayer();
//회전 플레이어 포인터 초기화
smCHAR *InitRotPlayer();

//PLAYSUB 펑션루틴 첵크
int Check_PlaySubReleaseFunc();

//얼굴 변경한거 적용
int SetJobFace( char *szModelName );
//모델이 맞는지 확인
int	CheckUserModel( smCHAR_INFO *lpCharInfo );

//이벤토리 아이템 코드 초기화
int	ResetInvenItemCode();
//이벤토리 아이템 코드에서 특정 아이템 제거
int SubInvenItemCode( sITEMINFO *lpItem );
//인벤토리 아이템 코드 확인
int	CheckInvenItemCode();

//모듈 첵크 2
void CheckPlaySubFunc2();



//에너지 그래프 첵크 초기화
int	ResetEnergyGraph( int Num );
//에너지 그래프 오류 첵크
int CheckEnergyGraphError();
//에너지 그래프의 오류 첵크
int CheckEnergyGraph();

//인벤토리의 지정한 아이템을 찾아 검색하여 찾는다
sITEMINFO *FindInvenItem( DWORD dwItemCode , DWORD dwHead , DWORD dwChkSum );
//인벤토리 아이템 복사 오류
int SendInvenItemError( DWORD dwKind , DWORD dwItemCode , DWORD dwHead , DWORD dwChkSum );
//인벤토리의 장착 아이템 정보 서버에 보내기
int SendUsingItemListToServer();
//인벤토리의 장착 아이템 정보 서버에 보내기
int SendUsingItemListToServer2();


//이벤토리 돈 검사 초기화
int	ResetInvenMoney();
//인벤토리 검사용돈 추가
int	AddInvenMoney( int Money );
//인벤토리 돈 검사
int	CheckInvenMoney();


//공격속도를 프레임속도로 변환
int GetAttackSpeedMainFrame( int AttackSpeed );
int	GetAttackSpeedFrame( int AttackSpeed , int AddSpeed );
int	GetAttackSpeedFrame( int AttackSpeed );


extern int DispLoading;			//로딩 상태 표시 유무
extern int MessageLoading;		//로딩중 메세지 처리

//에티르코어 포션 로그 코드
#define POTION_ETHERCORE		0x40

extern int	CodeXorExp;					//경험치 숨김코드
extern int	CodeXorExp_High;			//경험치 숨김코드
extern int	CodeXorLife;				//생명력 숨김코드
extern int	CodeXorMana;				//기력 숨김코드
extern int	CodeXorStamina;				//근력 숨김코드
extern int	CodeXorMoney;				//돈 숨김코드


//색상으로 아이템의 상태를 판별한다
int GetItemKindFromBliankColor( smCHARTOOL	*lpCharTool );


extern char	szSOD_String[64];
extern BYTE	bSOD_StringColor[4];
extern int	SOD_StringCount;

extern sSOD_SCORE	sSodScore;
extern int		SoDGateFlag;

int	DisplaySodScore();	//Sod점수 표시 함수
int	SodScoreMain();		//SoD 연출
int	Sod_SodEffect( int Round );			//SoD 라운드 이펙트


