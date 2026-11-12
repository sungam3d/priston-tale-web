#ifndef	_SMPACKET_HEADER

#define _SMPACKET_HEADER

#include	"sinbaram\\sinitem.h"
#include	<atltime.h>

//게임가드 보안 Lib
#ifdef	_npGAME_GUARD_AUTH
#include "nProtect\\CSAuth.h"
#pragma comment(lib,"nprotect\\CSAuth.lib")
#endif

#define	smTRANSCODE_OPENLIVE		0x38000000

#define	smTRANSCODE_SYSTEM			0x48400000

#define	smTRANSCODE_CONNECTED		0x48470080
#define smTRANSCODE_RECORDDATA		0x48470081
#define smTRANSCODE_GETRECORDDATA	0x48470082
#define smTRANSCODE_FAILRECORDDATA	0x48470083
#define smTRANSCODE_RECORDRESULT	0x48470084

//ID로 저장된 캐릭터 정보 입수
#define smTRANSCODE_ID_GETUSERINFO	0x48470085
#define smTRANSCODE_ID_SETUSERINFO	0x48470086
#define smTRANSCODE_DELRECORDDATA	0x48470087
#define smTRANSCODE_INSRECORDDATA	0x48470088
#define smTRANSCODE_ISRECORDDATA	0x48470089

//버전 확인 메세지
#define smTRANSCODE_VERSION			0x4847008A
//네트워크 첵크 메세지
#define smTRANSCODE_CHECK_NETSTATE	0x4847008B
//관리자 모드 설정
#define smTRANSCODE_ADMINMODE		0x4847008C
//오브젝트 서버에 번호 설정 
#define smTRANSCODE_SET_OBJSERIAL	0x4847008D
//네트워크 첵크 메세지(보조 서버용)
#define smTRANSCODE_CHECK_NETSTATE2	0x4847008E
#define smTRANSCODE_CHECK_NETSTATE3	0x4847008F
#define smTRANSCODE_CHECK_NETSTATE4	0x48470090
#define smTRANSCODE_CHECK_NETSTATE5	0x48470091

#define smTRANSCODE_IPLIST			0x48471000
#define smTRANSCODE_CHATMESSAGE		0x48471001
#define smTRANSCODE_POSITION		0x48471002
#define smTRANSCODE_PLAYLIST		0x48471003
#define smTRANSCODE_CONTINUEDATA	0x48471004
#define smTRANSCODE_WHISPERMESSAGE	0x48471005
#define smTRANSCODE_COUPLERING		0x48471006			// pluto 커플링

#define smTRANSCODE_PLAYDATA1		0x48470010
#define smTRANSCODE_PLAYDATA2		0x48470011
#define smTRANSCODE_PLAYDATA3		0x48470012
#define smTRANSCODE_TRANSPLAYDATA	0x48470013
#define smTRANSCODE_PLAYDATAGROUP	0x48470014
#define smTRANSCODE_PLAY_COMMAND	0x48470018

#define smTRANSCODE_PARTY_CHATMSG	0x48471015

#define smTRANSCODE_ADDEXP_PARTY	0x48470029
#define smTRANSCODE_ATTACKDATA		0x48470030
#define smTRANSCODE_ADDEXP			0x48470031
#define smTRANSCODE_PLAYERINFO2		0x48470032
#define smTRANSCODE_SET_BLACKLIST	0x48470033
#define smTRANSCODE_SET_ITEMCOPYLIST 0x48470034
#define smTRANSCODE_SKIL_ATTACKDATA	0x48470035
#define smTRANSCODE_FINDCRACK		0x48470036
#define smTRANSCODE_MODULE_WARNIG	0x48470037
#define smTRANSCODE_UPDATE_CINFO	0x48470038
#define smTRANSCODE_OPENMONSTER		0x48470039
#define smTRANSCODE_WARPFIELD		0x48470040
#define smTRANSCODE_TRADEITEMS		0x48470041
#define smTRANSCODE_TRADE_REQUST	0x48470042
#define smTRANSCODE_TRADE_ITEMKEY	0x48470043
#define smTRANSCODE_TRADE_READYITEM	0x48470044
#define smTRANSCODE_SKILL_MENU		0x48470045
#define smTRANSCODE_FINDCRACK2		0x48470046
#define smTRANSCODE_WAREHOUSE		0x48470047

#define smTRANSCODE_OPEN_WAREHOUSE	0x48470048
#define smTRANSCODE_OPEN_MIXITEM	0x48470049
#define smTRANSCODE_OPEN_COLLECT	0x4847004A
#define smTRANSCODE_COLLECTMONEY	0x4847004B
#define	smTRANSCODE_PLAYERINFO_Q	0x4847004C
#define smTRANSCODE_OPEN_AGING		0x4847004D

#define smTRANSCODE_SKIL_ATTACKUSER	0x4847004E

#define smTRANSCODE_OPEN_EVENTGIFT	0x4847004F

#define smTRANSCODE_PLAYERINFO		0x48470020
#define smTRANSCODE_GETPLAYERINFO	0x48470021
#define smTRANSCODE_FAILCONNECT		0x48470023


#define smTRANSCODE_PLAYITEM		0x48470050
#define smTRANSCODE_DELITEM			0x48470051
#define smTRANSCODE_PUTITEM			0x48470052
#define smTRANSCODE_THROWITEM		0x48470053
#define smTRANSCODE_SHOP_ITEMLIST	0x48470054
#define smTRANSCODE_SAVETHROWITEM	0x48470055
#define smTRANSCODE_SAVETHROWMONEY	0x48470056
#define smTRANSCODE_SAVETHROWITEM2	0x48470057
#define smTRANSCODE_SAVETHROWITEMS	0x48470058
#define smTRANSCODE_PLAYERKILLING	0x48470059
#define smTRANSCODE_GETITEM			0x4847005A
#define smTRANSCODE_USEITEM_CODE	0x4847005B
#define smTRANSCODE_COMMAND_USER	0x4847005C
#define smTRANSCODE_WING_NPC		0x4847005D
#define smTRANSCODE_PAYMENT_MONEY	0x4847005E
#define smTRANSCODE_MEDIA_NPC		0x4847005F

#define smTRANSCODE_ADD_STARTPOSI	0x48470060
#define smTRANSCODE_DEL_STARTPOSI	0x48470061
#define smTRANSCODE_ADD_NPC			0x48470070
#define smTRANSCODE_DEL_NPC			0x48470071
#define smTRANSCODE_ADMIN_COPYCHAR	0x48470062

#define	smTRANSCODE_PARTY_UPDATE		0x484700A0
#define	smTRANSCODE_PARTY_REQUEST		0x484700A1
#define	smTRANSCODE_PARTY_JOIN			0x484700A2
#define	smTRANSCODE_PARTY_PLAYUPDATE	0x484700A3
#define	smTRANSCODE_PARTY_RELEASE		0x484700A4
#define	smTRANSCODE_PARTY_COMMAND		0x484700A5
#define smTRANSCODE_OPEN_MIXTURE_RESET	0x484700A6	// 석지용 - 믹스쳐 리셋 메시지 정의

#define smTRANSCODE_PCBANG_PET			0x484700B0
#define smTRANSCODE_PHENIX_PET			0x484700B1	// pluto 펫(해외) 수정
#define smTRANSCODE_HELP_PET			0x484700B2	// 박재원 - 빌링 도우미 펫 추가

#define	smTRANSCODE_SERVER_INFO			0x484700C0

#define	smTRANSCODE_PARTY_POTION		0x484700D0
//힐링
#define	smTRANSCODE_HEALING				0x484700D2
#define	smTRANSCODE_HOLYMIND			0x484700D3
#define	smTRANSCODE_GRAND_HEALING		0x484700D4

#define	smTRANSCODE_VAMPRIC_CUSPID		0x484700D6
#define	smTRANSCODE_VAMPRIC_CUSPID_EX		0x484700D7		// 장별 - 뱀피릭 커스핏 EX
#define	smTRANSCODE_MANA_RECHARGING		0x484700D8

//아이템 거래 보고
#define	smTRANSCODE_RECORD_TRADE		0x48480010


//네트워크 품질 모드 설정
#define smTRANSCODE_NETWORK_QUALITY	0x484700E0
#define smTRANSCODE_PROCESSINFO		0x484700E4
#define	smTRANSCODE_CLOSECLIENT		0x484700E6
#define smTRANSCODE_CHECKIP			0x484700E7

//클라이언트에 데이타 저장 요구
#define	smTRANSCODE_SAVECLIENT		0x484700E8


#define smTRANSCODE_EFFECT_ITEM		0x48478000
#define smTRANSCODE_EFFECT_WEAPON	0x48478002
#define smTRANSCODE_EFFECT_STUN		0x48478004
#define smTRANSCODE_EFFECT_CURSE	0x48478006
#define smTRANSCODE_EFFECT_ICE		0x48478008
#define smTRANSCODE_EFFECT_POISION	0x4847800A
#define smTRANSCODE_EFFECT_VANISH	0x4847800C
#define smTRANSCODE_EFFECT_PERSHOP	0x4847800E
#define smTRANSCODE_EFFECT_LOWPET	0x4847800F
#define smTRANSCODE_EFFECT_CLANMGR	0x48479000
#define smTRANSCODE_EFFECT_FORCEORB	0x48479010
#define smTRANSCODE_EFFECT_CURSE_QUEST	0x48479011
#define smTRANSCODE_EFFECT_BLESS_SCORE	0x48479012
#define smTRANSCODE_EFFECT_KEEPSKILL	0x48479020


//오브젝트 서버에 번호 설정 
#define smTRANSCODE_CONT_OBJSERIAL	0x48478010

//메세지박스 출력
#define smTRANSCODE_MESSAGEBOX		0x48478100

//펑션 에러난것을 감지
#define	smTRANSCODE_FUNCERROR		0x48478200

//클라이언트 펑션메모리 전송
#define smTRANSCODE_FUNCTION_MEM2	0x48478300

//해킹유저 등록 2
#define	smTRANSCODE_SET_BLACKLIST2	0x48478400
//해킹유저 등록 3
#define smTRANSCODE_SET_BLACKLIST3	0x48478500

//불량 유저 설정
#define	smTRANSCODE_BAD_PLAYER		0x48478600

//잘못된 공수 수치 보정 값
#define	smTRANSCODE_FIX_ATTACK		0x48478700
//캐릭터 강제로 죽이기
#define	smTRANSCODE_DEAD_PLAYER		0x48478800

//클라이언트 수치 오류
#define smTRANSCODE_CLIENT_ERROR	0x48478900

//인벤토리 아이템 오류
#define	smTRANSCODE_INVEN_ERR_ITEM	0x48478910
//인벤토리 돈 오류
#define	smTRANSCODE_INVEN_ERR_MONEY	0x48478920

//인벤토리 아이템 위치 정보
#define	smTRANSCODE_INVEN_POSITION	0x48478930
#define	smTRANSCODE_INVEN_POSITION2	0x48478931


//암호화된 패킷
#define smTRANSCODE_ENCODE_PACKET	0x80010000
#define smTRANSCODE_ENCODE_PACKET2	0x90010000

//클랜 메뉴
#define smTRANSCODE_OPEN_CLANMENU	0x48478A00
//별 포인트 관련
#define smTRANSCODE_OPEN_STARPOINT	0x48478A10
#define smTRANSCODE_STARPOINT		0x48478A12
//돈기부 패킷
#define smTRANSCODE_OPEN_GIVEMONEY	0x48478A14
#define smTRANSCODE_GIVEMONEY		0x48478A16
//클랜머니 관련패킷
#define smTRANSCODE_CLANMONEY		0x48478A18


//아이템 배달
#define smTRANSCODE_ITEM_EXPRESS	0x48478A80

//개인상점 오픈
#define smTRANSCODE_OPEN_MYSHOP		0x48478A90
//개인상점 부르기
#define smTRANSCODE_CALL_MYSHOP		0x48478AA0
//개인상점 아이템
#define	smTRANSCODE_MYSHOP_ITEM		0x48478AB0
//개인상점 메세지 얻기
#define	smTRANSCODE_MYSHOP_MESSAGE	0x48478AC0
//개인상점 물건 구입
#define	smTRANSCODE_MYSHOP_TRADE	0x48478AD0

//박철호 : 2005-11-17 오후 8:41:39
//PC방 펫 공격이펙트
#define smTRANSCODE_EFFECT_PCBANGPET	0x48478AE0


//관리자 모드 설정
#define smTRANSCODE_ADMINMODE2			0x5047108C
//네트웍 첵크
#define smTRANSCODE_CHECK_NETSTATE20	0x5047008E

//경험치 돈 받은데이타 확인
#define smTRANSCODE_CHECK_EXPMONEY		0x50320010
//경험치 데이타 확인
#define smTRANSCODE_CHECK_EXPDATA		0x50320020
//포스오브 설정 데이타 설정교신
#define smTRANSCODE_FORCEORB_DATA		0x50320030
//빌링아이템 설정 데이타 설정교신
#define smTRANSCODE_PRIMEITEM_DATA		0x50320031

// pluto 마나 리듀스
// pluto 빌링아이템 설정 데이타 설정교신
#define smTRANSCODE_PRIMEITEM_DATA2	0x50320032

// 박재원 - 빌링 도우미 펫, 뱀피릭 커스핏 EX, 근력 리듀스 포션
#define smTRANSCODE_PRIMEITEM_DATA3	0x50320033

// 장별 - 뱀피릭 커스핏 EX
#define smTRANSCODE_PRIMEITEM_DATA4	0x50320036

// 박재원 - 부스터 아이템(생명력, 기력, 체력)
#define smTRANSCODE_BOOSTER_DATA		0x50320034

// 장별 - 에이징 치트 서버로 전송
#define smTRANSCODE_AGING_DATA			0x50320035

// 장별 - 스킬 딜레이
#define smTRANSCODE_SKILLDELAY_DATA			0x50320036

//데이타 서버로 클라이언트 경유 전송
#define smTRANSCODE_SEND_DATASERVER		0x50320100
//게임 서버로 클라이언트 경유 전송
#define smTRANSCODE_SEND_GAMESERVER		0x50320110
//지역 서버로 클라이언트 경유 전송
#define smTRANSCODE_SEND_AREASERVER		0x50320120
//지역/게임 서버로 클라이언트 경유 전송
#define smTRANSCODE_SEND_AGAME_SERVER	0x50320130

//pluto 마나 리듀스
//지역/게임 서버로 클라이언트 경유 전송
#define smTRANSCODE_SEND_AGAME_PRIMEITEM_DATA2	0x50320140

// 박재원 - 빌링 도우미 펫, 뱀피릭 커스핏 EX, 근력 리듀스 포션 아이템 추가시 사용함.
// 지역/게임 서버로 클라이언트 경유 전송
#define smTRANSCODE_SEND_AGAME_PRIMEITEM_DATA3	0x50320150

// 장별 - 뱀피릭 커스핏 EX
#define smTRANSCODE_SEND_AGAME_PRIMEITEM_DATA4	0x50320160


//믹스쳐 데이타 보냄
#define smTRANSCODE_CRAFTITEM			0x50320200
//에이징 데이타 보냄
#define smTRANSCODE_AGINGITEM			0x50320202
//에이징 업그레이드
#define	smTRANSCODE_AGING_UPGRADE		0x50320204
//링크코어 등록
#define	smTRANSCODE_MAKE_LINKCORE		0x50320205
//링크코어 사용
#define	smTRANSCODE_USE_LINKCORE		0x50320206
//윙 아이템 데이타 보냄
#define smTRANSCODE_WINGITEM			0x50320208

//상점 데이타 구입
#define smTRANSCODE_SHOPTITEM			0x50320210
//상점 데이타 판매
#define smTRANSCODE_SHOP_SELLITEM		0x50320212

//포스오브 아이템 생성
#define	smTRANSCODE_ITEM_FORCEORB		0x50320214

//퀘스트 명령
#define	smTRANSCODE_QUEST_COMMAND		0x50320220
//종료시 Web URL 오픈
#define	smTRANSCODE_EXITINFO_URL		0x50320230


//아이템 확인
#define smTRANSCODE_CHECKITEM			0x50320300
//아이템 오류 확인
#define smTRANSCODE_ERRORITEM			0x50320310
//물약 갯수 오류 확인 - 물약 겹친수를 1로 초기화 
#define smTRANSCODE_CLEARPOTION			0x50320320

#ifdef _CHECK_MESH
//아이템메쉬 조작검사 첵크섬값
#define smTRANSCODE_CHECK_ITEMMESH		0x50320330
#endif

//클라이언트 펑션포지션 보고
#define smTRANSCODE_CLIENT_FUNCPOS		0x50320400
//클라이언트 펑션포지션 값을 요구 ( 초기설정을 위함 )
#define smTRANSCODE_GETCLIENT_FUNCPOS	0x50320410

//클라이언트 펑션메모리 전송
#define smTRANSCODE_FUNCTION_MEM		0x50320420
//클라이언트 펑션포지션 결과 값
#define smTRANSCODE_FUNCTON_RESULT		0x50320430

//클라이언트 프로세스 타임 최대치
#define smTRANSCODE_PROCESS_TIMEMAX		0x50320500

//nSprite 중국판 보안프로그램
#define smTRANSCODE_NSPRITE				0x50320600
//nProtect
#define smTRANSCODE_NPROTECT			0x50320700
#define smTRANSCODE_GAMEGUARD_AUTH		0x50320710


//공격 기록 비교 요구
#define	smTRANSCODE_REC_DAMAGEDATA		0x50320800

//SoD 결과 보고
#define	smTRANSCODE_SOD_RESULT			0x50320900
#define	smTRANSCODE_SOD_RESULT2			0x50320901

//스킬 실행 동료
#define	smTRANSCODE_PARTY_SKILL			0x50320A00


//스킬 실행 취소
#define	smTRANSCODE_CANCEL_SKILL		0x50320A10
//스킬 실행 일반
#define	smTRANSCODE_PROCESS_SKILL		0x50320A20
#define smTRANSCODE_PROCESS_SKILL2		0x50320A30
#define smTRANSCODE_PROCESS_CLAN_SKILL	0x50320A35
//스킬 변경 업데이트
#define	smTRANSCODE_UPDATEL_SKILL		0x50320A40
//추가회피 성공정보
#define	smTRANSCODE_SUCCESS_EVATION		0x50320A50
//속성정보
#define	smTRANSCODE_RESISTANCE			0x50320A60

//설문조사
#define	smTRANSCODE_PUBLIC_POLLING		0x50320A70
//설명공지 박스
#define	smTRANSCODE_NOTICE_BOX			0x50320A80
//핵킹용 함정
#define	smTRANSCODE_HACKTRAP			0x50320A90


//생명,기력,근력 증가
#define	smTRANSCODE_VIRTURAL_POTION		0x50320B00

//클랜 가입 메세지
#define	smTRANSCODE_CLAN_SERVICE		0x50320C00
//클랜 업데이트 요구 메세지
#define	smTRANSCODE_CLAN_UPDATE			0x50320C10

//퀘스트 처리 요구 메세지
#define	smTRANSCODE_QUEST_MESSAGE		0x50320D00

//캐릭터 죽다
#define	smTRANSCODE_DEAD_USER			0x50320E00
//야호 동작 실행
#define	smTRANSCODE_YAHOO_MOTION		0x50320E10
//이벤트 정보 변경
#define	smTRANSCODE_CHANGE_EVENT		0x50320F00

//클라이언트 정보
#define	smTRANSCODE_CLIENTINFO			0x50321000

//무적 모드 경고
#define	smTRANSCODE_WARNING_BLINK_ATT	0x50321010


//새 공격 패킷
#define smTRANSCODE_ATTACK_DAMAGE		0x50322000
#define smTRANSCODE_RANGE_DAMAGE		0x50322010
#define smTRANSCODE_ATTACK_RESULT		0x50322020
#define smTRANSCODE_DISP_DAMAGE			0x50322030
#define smTRANSCODE_ATTACKDATA2			0x50322040

#define smTRANSCODE_LIMIT_DAMAGE		0x50322050

//클라이언트 문제 경고
#define smTRANSCODE_WARNING_CLIENT		0x50322060
#define smTRANSCODE_WARNING_SKILL		0x50322080
//얼굴 커지는 패킷
#define smTRANSCODE_COMIC_SKIN			0x50322070
//관리자 명령 기록
#define	smTRANSCODE_REC_ADMIN_COMMAND	0x50322090

//관리자 모드 설정 확인
#define smTRANSCODE_CHECK_ADMINMODE		0x503220A0

//SoD정보 보내기
#define smTRANSCODE_SOD_INFOMATION		0x503220B0

//블레스캐슬 설정정보
#define smTRANSCODE_BLESSCASTLE_INFO	0x503220C0
#define smTRANSCODE_BLESSCASTLE_TAX		0x503220D0

//몬스터의 범위스킬 주변 캐릭터 정보
#define smTRANSCODE_ATTACK_SLASH_LIST	0x503220E0

//다른서버에 의해 변경된 값 보냄
#define smTRANSCODE_UPDATE_SERVER_PARAM	0x503220F0
#define smUPDATE_PARAM_LOWUSER_PRESENT	0x100

//메인서버에 계정정보 보냄 (베트남)
#define smTRANSCODE_USER_ID				0x50322100



//데미지 패킷 암호화 모듈
#define smTRANSCODE_DAMAGE_ENCODE_MEM	0x50322F00
#define smTRANSCODE_DAMAGE_ENCODE_MEM2	0x50322EC0

//패킷 동적 암호화 펑션
#define	smTRANSCODE_PACKET_DYNFUNC		0x50323000


//서버 매핑 정보
#define	smTRANSCODE_AREA_SERVER_MAP		0x50324000
// pluto 아이템 제련
#define smTRANSCODE_OPEN_SMELTING		0x50325000
#define smTRANSCODE_SMELTINGITEM		0x50325001
// pluto 아이템 제작
#define smTRANSCODE_OPEN_MANUFACTURE	0x50326000
#define smTRANSCODE_MANUFACTURE			0x50326001
// pluto 제작 아이템 가격, 무게 정보 가져 올려고
#define smTRANSCODE_MANUFACTURE_WEIGHT_PRICE 0x50326002

// pluto 선물상자
#define smTRANSCODE_GIFTBOX_MONEY		0x50326003

//박재원 - 빌링 아이템 초기화 치트
#define smTRANSCODE_PREMIUMITEM_INIT	0x50326004

// 석지용 - 믹스쳐 리셋 처리
#define smTRANSCODE_MIXTURE_RESET_ITEM	0x50326006

//이벤트 플랙모드
#define	rsHARDCORE_EVENT_FLAG		0x100

#define smCODE_SOD_EFFECT		100			//주문 시스템 추가 - SOD 이펙트 재사용

// 장별 - 재분배 이벤트
#define smTRANSCODE_EVENT_GAME		0x50326007
#define smCODE_EVENT_STATE_INIT		100

// 장별 - 입력창
#define smTRANSCODE_ITEMDOC			0x50326008

// GM /MakeMeLevel: server -> client, WParam = the new level
#define smTRANSCODE_GM_SETLEVEL		0x50326010


#define TRANS_BUFF_SIZE		4096

#define	smCHAR_STATE_NPC		0
#define	smCHAR_STATE_ENEMY		1
#define	smCHAR_STATE_USER		0x80

#define	smCHAR_MONSTER_NATURAL	0x80
#define	smCHAR_MONSTER_GOOD		0x81
#define	smCHAR_MONSTER_EVIL		0x82

#define	smCHAR_MONSTER_NORMAL	0x00
#define	smCHAR_MONSTER_UNDEAD	0x90
#define	smCHAR_MONSTER_MUTANT	0x91
#define	smCHAR_MONSTER_DEMON	0x92
#define	smCHAR_MONSTER_MECHANIC	0x93

#define	smCHAR_MONSTER_USER		0xA0

#define	smCOMMNAD_USER_WARP		0x100
#define	smCOMMNAD_USER_AGINGUP	0x110
#define	smCOMMNAD_USER_HOLY_INCANTATION	0x120
#define	smCOMMNAD_USER_RESURRECTION	0x130
#define	smCOMMNAD_USER_FORCEORB	0x140
#define	smCOMMNAD_USER_VALENTO	0x150
#define	smCOMMNAD_USER_FIREFLOWER	0x160

#define	smCOMMAND_PLAY_BABEL		0x200
#define	smCOMMAND_PLAY_BELSS_CASTLE	0x210
#define	smCOMMAND_PLAY_PK_FIELD		0x220
#define	smCOMMAND_PLAY_KELVEZU		0x230

class	smCHAR;

class	rsRECORD_DBASE;

#define	FALLITEM_MAX			200
#define	FALLITEM2_MAX			3

struct _FALLITEM{
	DWORD	dwItemCode;
	int		Percentage;
	short	sPrice[2];
};

#define	NPC_MESSAGE_MAX		20

/*----------------------------------------------------------------------------*
*				(몬스터) 정보 구조체
*-----------------------------------------------------------------------------*/
struct smCHAR_MONSTER_INFO {
	char	szName[32];			//이름

	short	SkillDamage[2];		//스킬 공격력
	int		SkillRange;			//스킬 적용 범위
	int		SkillRating;		//스킬 공격률
	int		SkillDistance;		//적용 스킬의 거리 
	RECT	SkillRangeRect;		//범위 스킬적용시 위치
	int		SkillCurse;			//저주 걸기

	int		AttackPattern;		//공격 패턴
	int		ActiveHour;			//활동시간
	int		IQ;					//지능지수
	int		Nature;				//품성
	int		Undead;				//언데드 유무
	int		GenerateGroup[2];	//동시에 출현수 ( 최소 / 최대 )
	int		GetExp;				//죽었을때 경험치 부여량
	int		SpAttackPercetage;	//특수 공격 사용 확률
	int		MoveRange;			//이동범위

	int		DamageStunPers;		//공격받을시 스턴확률

	int		PotionCount;		//물약 보유수
	int		PotionPercent;		//물약 보유률

	int		AllSeeItem;			//떨어질떄 다보인다
	int		FallItemCount;		//떨어뜨릴 아이템들의 카운터
	int		FallItemPerMax;		//아이템 생성 확률 최대치
	int		FallItemMax;		//동시에 떨어뜨릴 아이템 최대치
	int		FallItemPlusCount;	//떨어뜨릴 아이템들의 카운터 (추가)

	int		ClassCode;			//몬스터 구별 코드

	int		UseEventModel;		//이벤트 모델 저장유무

	int		Real_Sight;			//실제 시야 거리 ( 제곱근 아님 )

	_FALLITEM	FallItems[FALLITEM_MAX];
	_FALLITEM	FallItems_Plus[FALLITEM2_MAX];		//추가아이템


	//NPC 상점 판매 정보
	int		SellAttackItemCount;
	void	*SellAttackItem[32];	//공격 아이템
	int		SellDefenceItemCount;
	void	*SellDefenceItem[32];	//방어 아이템
	int		SellEtcItemCount;
	void	*SellEtcItem[32];		//기타 아이템

	int		SkillMaster;			//스킬 수련 NPC
	int		SkillChangeJob;			//전직 가능 NPC
	int		WareHouseMaster;		//아이템 보관 NPC
	int		ItemMix;				//아이템 조합 NPC
	int		ItemAging;				//아이템 에이징
	int		CollectMoney;			//돈모금 NPC
	int		EventGift;				//경품 이벤트 NPC
	int		Smelting;				// pluto	제련 NPC
	int		Manufacture;			// pluto	제작 NPC
	int		MixtureReset;			// 석지용 - 믹스쳐 리셋 NPC


#ifdef _LANGUAGE_VEITNAM
	int		WowEvent;
#endif

	int		EventCode;				//이벤트 코드
	int		EventInfo;				//이벤트 정보
	DWORD	dwEvnetItem;			//이벤트 아이템
	int		EventNPC;				//이벤트 NPC 정보
	int		ClanNPC;				//클렌 NPC
	int		GiftExpress;			//경품 배달
	int		WingQuestNpc;			//윙 퀘스트 NPC
	int		StarPointNpc;			//별 포인트 NPC
	int		GiveMoneyNpc;			//돈 모금 NPC
	int		TelePortNpc;			//텔레포트 NPC
	int		BlessCastleNPC;			//블레스 캐슬 NPC
	int		PollingNpc;				//설문조사 NPC

	char	*szMediaPlayNPC_Title;		//동영상 NPC
	char	*szMediaPlayNPC_Path;		//동영상 NPC

	int		QuestCode;				//퀘스트 처리코드
	int		QuestParam;				//퀘스트 파라메터

	short	OpenCount[2];			//출현 간격 번호

	DWORD	dwAutoCharCode;			//자동캐릭터 구분 코드 (SpeedSum 값)

	char	*lpNpcMessage[NPC_MESSAGE_MAX];	//NPC 대화 메세지
	int		NpcMsgCount;					//NPC 대화 카운터

};

/*----------------------------------------------------------------------------*
*				(캐릭터 스테이터스)캐릭터 정보 구조체
*-----------------------------------------------------------------------------*/

struct smCHAR_INFO {
	char	szName[32];				//이름
	char	szModelName[64];		//모델 이름
	char	szModelName2[60];		//모델 이름2  // szModelName2[64] -> szModelName2[60] 얼큰이 오류 수정
	DWORD	ModelNameCode2;			//모델 코드 추가 (장순목) // 얼큰이 오류 수정

	DWORD	dwObjectSerial;			//객체의 고유값

	//int		AutoPlayer;				//자동 속성
	int		ClassClan;				//소속클랜
	int		State;					//객체의 속성 ( smCHAR_STATE_  NPC / ENEMY / USER )
	int		SizeLevel;				//크기 단계 ( 0-소형 ~ 3-대형 )
	DWORD	dwCharSoundCode;		//사운드 캐릭터 코드

	/*-------------------------*
	*			능력치 
	*--------------------------*/
	DWORD JOB_CODE;			//직업 코드 
	int   Level;			//레벨 
	int   Strength;			//힘 
	int   Spirit;			//정신력 
	int   Talent;			//재능 
	int   Dexterity;		//민첩성 
	int   Health;			//건강 

	/*-------------------------*
	*			전투관련
	*--------------------------*/
	int	  Accuracy;			//상대적 명중률
	int   Attack_Rating;	//명중률
	int   Attack_Damage[2];	//타격수치 ( 최소 / 최대 )
	int	  Attack_Speed;		//공격속도
	int	  Shooting_Range;	//사정거리 ( 발사형무기 )
	int	  Critical_Hit;		//크리티컬힛 ( 1.5배 데미지 확률 )

	int   Defence;			//방어율 			
	int   Chance_Block;		//블럭율 
	int   Absorption;		//흡수율 

	int   Move_Speed;		//이동 속도 
	int   Sight;			//시야
	short Weight[2];			//운반 능력 

	/*-------------------------*
	*			저항력 
	*--------------------------*/
	short	Resistance[8];			//원소에대한 저항력 
	short	Attack_Resistance[8];	//원소에대한 공격력 

	/*-------------------------*
	*			...
	*--------------------------*/
	short   Life[2];			//0 현재 1 최대치 
	short   Mana[2];			//0 현재 1 최대치 
	short   Stamina[2];			//0 현재 1 최대치

	float	Life_Regen;			//재생능력 
	float	Mana_Regen;			//재생능력 
	float	Stamina_Regen;		//재생능력 

	int		Exp;
	int		Next_Exp;

	int		Money;				//보유한 돈

	smCHAR_MONSTER_INFO	*lpMonInfo;	//몬스터 구조체 포인터

	DWORD	Brood;				//종족

	int		StatePoint;			//능력치 포인트
	BYTE	bUpdateInfo[4];		//내용 변경 카운터
	short	ArrowPosi[2];		//인벤토리 화살표 위치
	int		Potion_Space;		//포션 주머니 크기

	int     LifeFunction;		//생명력 함수 
	int     ManaFunction;		//기력 함수 
	int     StaminaFunction;	//근력 함수 
	short   DamageFunction[2];  //0 근접 1 원거리   

	DWORD	RefomCode;			//리폼 코드

	DWORD	ChangeJob;
	DWORD	JobBitMask;

	WORD	wPlayerKilling[2];	//감옥갇혀 있음
	WORD	wPlayClass[2];		//계급 ( 몬스터의 보스설정 )

	int		Exp_High;			//경험치 상위4비트
	DWORD	dwEventTime_T;		//이벤트 제한 시간 - ( 얼큰이 시간 )
	short	sEventParam[2];		//이벤트 파라메터용 [0] 이벤트코드 [1] 이벤트 파라메터

	short	sPresentItem[2];	//우연을 가장한 필연적인 아이템 지급

	// 장별 - 그라비티 스크롤
	short GravityScroolCheck[2];

	DWORD	dwTemp[11];			//예비 버퍼 // 장별 - 그라비티 스크롤(12->11)

	DWORD	dwLoginServerIP;		//로그인한 서버의 IP
	DWORD	dwLoginServerSafeKey;	//로그인한 서버에서 보낸 보안키

	WORD	wVersion[2];		//이 구조체의 버전

	// 장별 - 예비버퍼
//	DWORD	dwTemp2[2000];
};

//캐릭터 정보 단축 크기
#define	smCHAR_INFO_QUICK_SIZE		192
#define	smCHAR_INFO_QUICK_SIZE2		352

#define	THROW_ITEM_INFO_MAX		64
#define	THROW_ITEM_INFO_MASK	63
#define	INVEN_ITEM_INFO_MAX		300

//버려진 아이템 정보 저장 ( 아이템 복사 방지 )
struct	sTHROW_ITEM_INFO {

	DWORD	dwCode;
	DWORD	dwKey;
	DWORD	dwSum;
};


//캐릭터 좌표 구조
struct rsPLAYPOS {
	DWORD	Area;		// Low->Wolrd High->Area	: 현재 존재하고 있는 지역의 위치 코드
	int		x,y,z;		// 좌표
};

//내용 : 자신의 위치및 정보를 서버에 전송
//대상 : 클라이언트 -> 서버
//코드 : smTRANSCODE_POSITION
//10초 간격으로 서버에 보내지는 플레이어 위치 정보
struct TRANS_PLAYPOS {
	int size,code;
	
	rsPLAYPOS	rsPlayPos;
	short Hp[2];
};



//내용 : 상대한테 공격 정보를 보낸다
//대상 : 클라이언트 -> 서버  클라이언트 -> 클라이언트
//코드 : smTRANSCODE_ATTACKDATA
struct TRANS_ATTACKDATA {
	int size,code;

	DWORD	dwDestObjectSerial;	//공격하는 캐릭터 고유번호
	DWORD	dwTarObjectSerial;	//공격대상 캐릭터 고유번호

	int x,y,z;			// 좌표
	int AttackState;	// 속성
	int AttackSize;		// 공격 범위
	int Power;			// 공격 파워
	DWORD	dwChkSum;

	short	sRating[2];	//[0]-레벨 [1]-명중력
	DWORD	dwDamageChkSum;
};

struct TRANS_ATTACKDATA2 {
	int size,code;

	DWORD	dwChkSum;

	DWORD	dwDestObjectSerial;	//공격하는 캐릭터 고유번호
	DWORD	dwTarObjectSerial;	//공격대상 캐릭터 고유번호

	int		x,y,z;			// 좌표
	int		AttackState;	// 속성
	int		AttackSize;		// 공격 범위
	short	Power[2];		// 공격 파워
	short	Power2[2];		// 공격 파워 ( 추가 공격 )
	short	Critical[2];	// 크리티컬
	int		SkillCode;		// 스킬코드
	DWORD	dwTime;			// 클라이언트 시간
	int		AttackCount;	// 공격 카운터
	short	MotionCount[2];	// 공격 모션 카운터
	DWORD	dwWeaponCode;	// 주 공격 무기
	short	Area[2];		// 지역
	int		Temp[4];		// 예비
};


// 장별 - 범위형 스킬( 32 -> 42 )
#define SKIL_ATTACK_CHAR_MAX		42

struct TRANS_SKIL_ATTACKDATA {
	int size,code;

	DWORD	dwDestObjectSerial;	//공격하는 캐릭터 고유번호

	int x,y,z;			// 좌표
	int AttackState;	// 속성
	int AttackSize;		// 공격 범위
	int Power;			// 공격 파워

	DWORD	dwChkSum;	//조작 방지 코드

	int	TargetCount;
	DWORD	dwTarObjectSerial[SKIL_ATTACK_CHAR_MAX];	//공격대상 캐릭터 고유번호
};

struct TRANS_SKIL_ATTACKDATA2 {
	int size,code;

	DWORD	dwDestObjectSerial;	//공격하는 캐릭터 고유번호

	DWORD	dwChkSum;

	int		x,y,z;			// 좌표
	int		AttackState;	// 속성
	int		AttackSize;		// 공격 범위
	short	Power[2];		// 공격 파워
	short	Power2[2];		// 공격 파워 ( 추가 공격 )
	short	Critical[2];	// 크리티컬
	int		SkillCode;		// 스킬코드
	DWORD	dwTime;			// 클라이언트 시간
	int		AttackCount;	// 공격 카운터
	short	MotionCount[2];	// 공격 모션 카운터
	DWORD	dwWeaponCode;	// 주 공격 무기
	short	Area[2];		// 지역
	int		Temp[4];		// 예비

	DWORD	dwMainTargetObject;		//주 타겟

	int	TargetCount;
	DWORD	dwTarObjectSerial[SKIL_ATTACK_CHAR_MAX];	//공격대상 캐릭터 고유번호
};



//내용 : 지역에 존재하는 다른 플레이어 정보 목록 전송
//대상 : 서버 -> 클라이언트
//코드 : smTRANSCODE_IPLIST
struct TRANS_AREAINFO_HEADER {
	int		size;
	int		code;
	DWORD	dwIP;
	int		Stats;					//전송 완료면 TRUE 아직 더 보낼 데이타가 있으면 FALSE
	int		TransListCnt;			//전송하는 TRANS_IPLIST 갯수
};
struct TRANS_IPLIST {
	DWORD	dwIP;
	rsPLAYPOS	Pos;
};

/////////////// 아이템 택배 서비스 /////////////////
#define	POST_ITEM_MAX				500

struct _POST_BOX_ITEM {
	int		Flag;
	char	szCharName[32];
	char	szItemCode[32];
	char	szSpeJob[32];
	char	szDoc[128];
	char	szFormCode[64];
	char	szPassCode[16];

	DWORD	dwItemCode;
	DWORD	dwJobCode;
	DWORD	dwFormCode;
	DWORD	dwPassCode;
	DWORD	dwParam[4];
};

struct	rsPOST_BOX_ITEM {
	DWORD	dwHead;
	int		ItemCounter;

	_POST_BOX_ITEM	PostItem[POST_ITEM_MAX];
};

struct	TRANS_POST_ITEM {
	int		size,code;

	char	szItemName[32];
	char	szDoc[128];
	DWORD	dwItemCode;
	DWORD	dwItemJobCode;
	DWORD	dwParam[4];

	DWORD	dwItemFlag;
	int		Weight;			// pluto 해외 빌링 아이템 무게
};
//	sITEM_INFO	sItemInfo;


///////////////////////////////////////////////////////


#define	srPLAYBUFF_MAX		4
#define	srPLAYBUFF_MASK		3

#define	PARTY_PLAYER_MAX		6

// 장별 - 예비버퍼
#define	srRECORD_DEFAULT_SIZE	16384
//#define	srRECORD_DEFAULT_SIZE	32768
#define	srRECORD_MEMORY_SIZE	24576
//#define	srRECORD_MEMORY_SIZE	49152

#define SKILL_POINT_COLUM_MAX	16

struct	TRANS_PARTY_PLAYINFO;

struct	npGAME_GUARD_AUTH_INFO {
	 DWORD	Auth_Value;		//게임가드 확인코드 ( 값을 리턴 받음 )
	 DWORD	Auth_Time;		//게임가드 확인시간 ( 다음처리 시간 )
	 DWORD	Auth_Count;		//게임가드 확인카운터 ( 총 보낸 횟수 )
	 DWORD	Auth_Retry;		//게임가드 확인 재시도 ( 응답이 60초 안에 안오면 몇번까지 재시도 )
};

//아이템 구매 카운터
struct	sMYSHOP_SELL_INFO {
	DWORD	dwCode;
	DWORD	dwHead;
	DWORD	dwChkSum;
	int		Price;
};

#define	MYSHOP_LIST_MAX		32
#define	MYSHOP_LIST_MASK	31


struct	sCASTLE_MASTER {
	int		Flag;
	DWORD	dwCheckTime;
	int		SelectSkill;
};

//데이타 에러 비트 플랙
#define	rsDATA_ERROR_WAREHOUSE			1


//간단한 코멘드 전송용 구조
struct smTRANS_COMMAND
{
	int	size;
	int code;

	int LParam;
	int WParam;
	int SParam;
	int EParam;
};

//박재원 - XignCode
#define PACKET_SIZE 512


//박재원 - XignCode
typedef struct _XIGNCODE_PACKET {
	int size;
	int code;
	BYTE data[PACKET_SIZE]; //박재원 - XignCode
} XIGNCODE_PACKET;

typedef struct _XIGNCODE_TempData{
	BOOL m_type;
	BOOL m_ceComplete;
	DWORD	dwWaitTime_NC;
	DWORD	dwWaitTime_CE;
	BOOL   dwClientReady;
	HANDLE xigncode_handle;
	int m_pcount;
} XIGNCODE_TempData;

//박재원 - XignCode
#define smTRANSCODE_XIGNCODE_PACKET		0x48470400
#define smTRANSCODE_XIGNCODE_START		0x48470500


typedef struct _WSABUF {
	u_long      len;     
	char FAR *  buf;     
} WSABUF, FAR * LPWSABUF;



//////////////////// 엑스트랩 ////////////////////
#ifdef _xTrap_GUARD

#include "XTrapSvrSrc/XTrapCC.h"
//smPacket.h
#define	smTRANSCODE_XTRAP_PACKET		0x48470300
#define	smXTRAP_COMMAND_LOGIN			0x00001000
#define	smXTRAP_COMMAND_SECURITY		0x00001010

struct	smTRANS_XTRAP_ADDR {
	smTRANS_COMMAND	smTransCommand;
	char Addr[528];				/* Client CREATEKF_IN_SIZE == Server XC_ADDR_SIZE */

};
struct	smTRANS_XTRAP_SECURITY {

	smTRANS_COMMAND	smTransCommand;
	char serialkey[1088];		/* Client CREATEKF_OUT_SIZE == Server XC_CLIENTBUF_SIZE */

};

struct	smXTRAP_PLAYINFO {
	//XC_CMF				Cmf[2];
	XC_KEYVALUE			KeyValue[2];
	DWORD				dwKeyGenTime;
	int					KeyGenWaitCount;
};

#ifdef _W_SERVER
#define _xTrap_GUARD_SERVER
#endif

#endif
////////////////////////////////////////////////////

//////////////////// 엑스트랩 ////////////////////
#ifdef _XTRAP_GUARD_4_

#ifdef _W_SERVER

#define _XTRAP_GUARD_4_SERVER
#include "XTrapSrcD5\\Server\\XTrap4Server.h"	//XTrapD5

#else

#define _XTRAP_GUARD_4_CLIENT

#endif

#include "XTrapSrcD5\\Server\\MessageProtocol.h"
//smPacket.h
#define	smTRANSCODE_XTRAP_PACKET		0x48470300
#define	smXTRAP_COMMAND_LOGIN			0x00001000
#define	smXTRAP_COMMAND_SECURITY		0x00001010


struct	smTRANS_XTRAP_ADDR {
	smTRANS_COMMAND	smTransCommand;
	char PacketBuf[XTRAP_CC4_BUFFERSIZE_COMMUNICATION_BASE];
};


#endif
////////////////////////////////////////////////////


/////////////////////// 서버에서의 플레이어 정보 구조체 ////////////////////////////////
class rsPLAYINFO {
public:

	char			szServerID[32];			//서버ID

	char			szID[32];				//유저 ID
	char			szName[32];				//유저 캐릭터 이름
	DWORD			dwCode_ID;				//ID를 빨리 찾기위한 코드
	DWORD			dwCode_Name;			//Name을 빨리 찾기 위한 코드

	int				AdminOperMode;			//관리자 복구 로딩중

	DWORD			dwDataError;			//데이타 에러 비트 플랙

	int				TransInfoFlag;			//통신 플랙 ( 0-준비중 1-사용가능 )
	int				ChatBuffCnt;

	int				SystemLine;				//시스템 모드 - 다른 서버에서 연결됨

	int				CharLoadingCount;		//캐릭터 로딩 시도 카운터 (한게임 돈복사 조사중)

	rsPLAYPOS		Position;
	rsPLAYINFO		**lpAreaPlayInfo;		//지역별 플레이어 정보 링크포인터 

	int				SendListCnt;

	DWORD			dwDecPacketCode;		//패킷 암호 코드
	BYTE			bPacketXor[2];			//패킷 암호 해독용 값
	DWORD			dwDecPacketTime;		//패킷 암호 적용 시간
	DWORD			dwDecPacketTime2;		//패킷 암호 적용 보류 시간

	smCHAR_INFO		smCharInfo;				//캐릭터 정보

	int				CharLevelMax;			//캐릭터 최고 레벨 ( 현재 보유한 계정에 있는 캐릭터중 최고레벨 )
	DWORD			dwNoticeCount;			//공지사항 메세지 카운터

	char			srPlayBuff[srPLAYBUFF_MAX+1][TRANS_BUFF_SIZE];		//전송할 데이타 대기용 버퍼
	int				srPlayBuffSize[srPLAYBUFF_MAX+1];					//전송할 데이타 대기용 버퍼 크기

	int				PlayBuffSleep;									//전송 데이타 슬립

	sTHROW_ITEM_INFO	ThrowItemInfo[THROW_ITEM_INFO_MAX];			//버려진 아이템 정보

	sTHROW_ITEM_INFO	InvenItemInfo[INVEN_ITEM_INFO_MAX+16];			//인벤토리 아이템 정보
	sTHROW_ITEM_INFO	WareHouseItemInfo[120];						//창고 아이템 정보
	int				OpenWarehouseInfoFlag;								//창고 아이템 정보 유무
	//sTHROW_ITEM_INFO	*lpWareHouseItemInfo;						//창고 아이템 내용 정보

	DWORD			dwTradePotionInfo[16];							//물약 거래 정보 임시저장
	int				TradePotionInfoCount;							//물약 거래 정보 저장 카운토

	sMYSHOP_SELL_INFO	sMyShopList[ MYSHOP_LIST_MAX ];					//개인 상점 거래 목록 버퍼
	int				MyShopListCount;								//개인 상점 거래 시도 목록 카운터
	DWORD			dwMyShopTradeTime;								//개인상점 거래 대기 시간

	int				ThrowItemCount;											//버려진 아이템 카운터

	int				UnsaveMoney;									//저장되지 않은 돈

	int				ServerMoney;									//서버에세 실제 계산한 돈
	int				WareHouseMoney;									//창고 금액

	DWORD			dwRecvTimePlayBuff;								//전송받은 시간
	DWORD			srSendPlayBuffCnt;								//보낸 버퍼 카운터
	DWORD			srRecvPlayBuffCnt;								//받은 버퍼 카운터

	DWORD			dwObjectSerial;	//객체의 고유값

	int				ChatMsgMode;			//채팅 메세지 모드
	char			szChatMessage[256];		//채팅 메세지

	//상점
	char			szTradeMessage[128];	//상점 메세지 문자열
	DWORD			dwTradeMsgCode;			//상점 메세지 코드

	smWINSOCK		*lpsmSock;

	DWORD			dwLastRecvTime;
	DWORD			dwLastMotionRecvTime;

	int				UpdateInfo_WaitCounter;	//업데이트 정보 대기 카운터
	int				UpdateInfo_WarnigCounter;	//업데이트 정보 경고 (누적되면 연결 종료 )

	rsRECORD_DBASE	*lpRecordBase;			//저장 클래스 포인터
	rsPOST_BOX_ITEM	*lpPostBoxItem;			//아이템 배달 구조체
	int				PostPassFailCount;		//아이템 배달 비밀번호 실패 횟수

	int				AdminMode;				//관리자 모드
	int				HideMode;				//투명 모드 ( 관리자가 남에게 안보이게 다닐때 )
	int				AdminIP;				//관리자 아이피 주소로 접속

	DWORD			dwConnectTime;			//접속한 시간

	int				LastTransPlayUserCount;	//마지막으로 전송된 다른 플레이어 번호
	int				LastTransAutoPlayCount;	//마지막으로 전송된 다른 자동 캐릭터 번호

	DWORD			dwConnectCount;			//전체 연결 카운터

	int				frame;
	DWORD			SendPlayGroupCount;		//보낸 데이타 그룹 카운터

	DWORD			MonTranCount;			//몬스터/NPC 전송 준비
	DWORD			CharTransCount;			//캐리터 전송 준비

	int				HitMonsterCount;		//공격한 몬스터 카운터
	smCHAR			*lpChrTarget;			//유저가 최근 공격한 몬스터 기록(우선순위 전송)
	DWORD			dwTarObjSerial;			//유저가 최근 공격한 몬스터 고유번호

	int				UserPacketLoop;			//유저 1루핑시 전송된 패킷 수치 ( 전송통계 수치 )
	int				BefUserPacketLoop;		//이전 1루핑시 전송된 패킷 수치 ( 전송통계 수치 )

	int				AttackCount;			//공격 카운터			
	DWORD			dwAttackTime;			//공격 시간
	DWORD			dwLastAttackTime;		//최근 공격시간
	int				DamagePacketCount;		//공격 패킷 카운터
	DWORD			dwDamagePacketTime;		//데미지 패킷을 보낸 클라이언트 측 시간

	int				MonsterKillCounter;		//몬스트 죽인 카운터
	DWORD			dwLastTradeChksum;		//마지막 거래시 첵크섬
	DWORD			dwLastTradeTime;		//마지막 거래시 시간

	//////////// 저레벨 선물 ///////////////
	short			sLowLevel_PresentItem[2];	//저레벨 아이템 선물

	DWORD			dwStartUser_PresentItem_Time;	//시작유저 아이템 선물
	DWORD			dwStartUser_PresentItem_Count;	//시작유저 아이템 선물



	////////////////////////////////////////////////
	// 공격 송수신 데이타

	int				Send_AttackCount;
	int				Send_AttackDamage;
	int				Recv_AttackCount;
	int				Recv_AttackDamage;

	/////////////// 매크로 감지 ////////////////////

	DWORD			dwMacroTime;
	int				Macro_PosX;
	int				Macro_PosZ;
	int				Macro_AngY;
	DWORD			dwMacroAngTime;
	DWORD			dwMacroChkTime;


	/////////////// 기타 ///////////////////////////
	DWORD			dwSendFrames[CONNECTMAX];	//다른 캐릭터들 데이타 전송 프레임

	int				CrackWarning;				//크랙 사용 유저
	DWORD			dwTime_Disconnect;			//연결끈기 예약
	int				BadPlayer;					//나쁜 플레이어 ( 모든 공격 무효 )
	DWORD			dwPacketAttackCount;		//과도한 패킷으로 인한 공격 감지 
	DWORD			dwPacketAttackTime;			//과도한 패키공격 감지시작 시간
	int				WarningCount;				//경고 카운트

	char			*lpRecordDataBuff;			//레코드데이타 버퍼 ( 게임저장 메모리버퍼 )
	DWORD			dwRecordDataTime;			//최근 기록시간

	int				EventMode;					//이벤트 진행
	int				EventScore;					//이벤트 점수
	int				EventCount;					//득점 카운트
	DWORD			dwEventTime;				//이벤트 관련 시간
	short			sEventParam[2];				//이벤트 진행 파라메터

	int				AreaOutCount;				//영역침범 카운터

	int				BlockTime;					//계정 블럭시킬 시간

	int				WhisperMode;				//귓말 모드

	int				Event_StarTicketLevel;		//별 상품권 발생 레벨


	////////////// 보완 관련 수치 통계 //////////////////////

	INT64			spExp_Start;				//시작시 경험치
	int				spMoney_Start;				//시작시 돈
	int				spLevel_Start;				//시작시 레벨

	int				spExp_InOut_Total;			//경험치 출력 토탈
	int				spMoney_In_Total;			//돈 입금 토탈
	int				spMoney_InOut_Total;		//돈 출금 토탈

	int				spEXP_Out;					//경험치 출력
	int				spMoney_In;					//돈 입금
	int				spMoney_Out;				//돈 출금

	DWORD			dwLastExpMoneyCheckTime;	//마지막으로 경험치 금액 비교한 시간
	DWORD			dwLastExpMoneyTransServer;	//군서버로 경험치 금액 수치 전송한 시간

	DWORD			dwHopeDisconnectTime;		//강제적으로 설정된 시간이 되면 연결 종료

	int				Client_Version;				//사용중인 클라이언트의 버전 정보

	DWORD			dwLastSaveTime;				//마지막으로 저장한 시간

	DWORD			dwGameServerExp[8];			//게임서버 경험치 저장

	DWORD			dwNSpriteTime;				//nSprite 검사한 시간

	//클라이언트 펑션 첵크
	DWORD			dwClientFuncChk_1;			//첵크한 펑션1
	DWORD			dwClientFuncChk_2;			//켁크한 펑션2
	int				FuncChkFailCount1;			//실패
	int				FuncChkFailCount2;			//실패
	int				FuncChkSucessCount1;		//성공
	int				FuncChkSucessCount2;		//성공
	DWORD			dwFuncChkErrorFlag;			//에러
	int				ClientFunChk1_Count;
	int				ClientFunChk2_Count;

	DWORD			dwClientFunChk_NextTime;	//다음 첵크할 시간

	int				RecordWarningCount;			//게임 저장기록 경고 카운터
	int				RecordWarningExp;			//경고 받은 경험치

	DWORD			dwClentOS;					//클라이언트 OS
	WORD			wClentOSVersion[2];			//클라이언트 버전 [0]-버전 [1]-빌드번호
	DWORD			dwClentModulChkSum;			//클라이언트 모듈 첵크섬
	int				ClentModuleCount;			//클라이언트 모튤 갯수

	DWORD			dwDamagePacketKey[2];		//데미지 패킷 암호용 키 ( 수신용 )
	DWORD			dwDamagePacketKey2[2];		//데미지 패킷 암호용 키 ( 송신용 )


	////////////// 파티 관련 정보 ///////////////////
	DWORD			PartyUpdateFlag;					//파티 정보 업데이트 요구
	DWORD			dwPartyInfo;						//파티 정보
	rsPLAYINFO		*lpPartyMaster;						//파티장 포인트
	DWORD			PartyMasterObjectCode;				//파티장 오브젝트 코드
	rsPLAYINFO		*lpPartyPlayers[PARTY_PLAYER_MAX];	//파티원 포인트
	DWORD			dwPartyObjectCode[PARTY_PLAYER_MAX];//파티원의 오브젝트 코드
	DWORD			PartyLevelAverage;					//파티원 레벨 평균
	int				PartyUserCount;						//파티원 카운트

	///////////// 클랜 관련 정보 ////////////////////
	DWORD			dwClanCode;							//가입한 클랜 코드
	int				ClanTicketCode;						//클랜 티켓 코드 ( Clan.DLL 에서 발급 )
	int				SucessCheck_ClanCode;				//클랜 코드 확인 성공


	//////////////////// 빌링 관련 변수 /////////////////////////

	char	szGuid[128];			//로그인 식별자
	char	szPwd[32];			//패스워드
	char	szGPCode[16];		//상품코드
	time_t	DisuseDay;			//종료 날짜
	int		UsePeriod;			//남은시간 초
	int		GpCode;				//상품코드
	int		GpType;				//상품타입 ( 개인 / PC방 )
	DWORD	dwPeriodTime;		//종료 시간
	int		Bl_RNo;				//PC방 사용 유무
	int		Bl_Meter;			//종량제
	DWORD	dwPeriodTime2;		//숨겨진 종료시간 ( 태국미성년자 등등 )


	////////////////// 아이템 송수신 보안 코드 /////////////////////

	int		SndItemSecCodeCount;			//아이템 보안 생성 코드 카운터
	int		RcvItemSecCodeBuff[8][16];		//받은 아이템 보안 카운터 임지 저장 버퍼

	/////////////////////////////////////////////////////////////////

	short	sLimitDamage[2];				//공격력 제한
	short	sLimitCritical[2];				//크리티컬 제한

	/////////////////////////////////////////////////////////////////

	int		ServerPotion[3][4];				//물약갯수 확인버퍼

	//////////////////////////// 스킬 처리 //////////////////////////////

	//오토메이션
	DWORD	dwSkill_AutoMation_Time;
	int		dwSkill_AutoMation_Param;
	//맥시마이즈
	DWORD	dwSkill_Maximize_Time;
	int		dwSkill_Maximize_Param;
	//피지컬
	DWORD	dwSkill_PhysicalAbsorb_Time;
	int		dwSkill_PhysicalAbsorb_Param;
	//익스트림 쉴드
	DWORD	dwSkill_ExtreamShield_Time;
	int		dwSkill_ExtreamShield_Param;

	//홀리 바디
	DWORD	dwSkill_HolyBody_Time;
	int		dwSkill_HolyBody_Param;
	//홀리 밸러
	DWORD	dwSkill_HolyValor_Time;
	int		dwSkill_HolyValor_Param;
	//홀리 리플렉션
	DWORD	dwSkill_HolyReflection_Time;
	int		dwSkill_HolyReflection_Param;
	//인첸트 워폰
	DWORD	dwSkill_EnchantWeapon_Time;
	int		dwSkill_EnchantWeapon_Param;


	//메탈아머
	DWORD	dwSkill_MetalArmor_Time;
	int		dwSkill_MetalArmor_Param;

	//스파크 쉴드
	DWORD	dwSkill_SparkShield_Time;
	int		dwSkill_SparkShield_Param;

	//팰컨 시리즈
	DWORD	dwSKill_Falcon_Time;
	short	dwSKill_Falcon_Damage[2];

	//디바인 인홀레이션
	DWORD	dwSkill_DivineInhalation_Time;
	DWORD	dwSkill_DivineInhalation_Param;

	//트라이엄프 발할라
	DWORD	dwSkill_TriumphOfValhalla_Time;
	DWORD	dwSkill_TriumphOfValhalla_Param;

	//버츄얼 라이프
	DWORD	dwSkill_VirtualLife_Time;
	DWORD	dwSkill_VirtualLife_Param;

	//에너지쉴드
	DWORD	dwSkill_EnergyShield_Time;
	DWORD	dwSkill_EnergyShield_Param;

	//스피릿엘리멘탈
	DWORD	dwSkill_SpiritElemental_Time;
	DWORD	dwSkill_SpiritElemental_Param;

	//댄싱소드
	DWORD	dwSkill_DancingSword_Time;
	DWORD	dwSkill_DancingSword_Param;
	DWORD	dwSkill_DancingSword_DelayTime;

	//배니쉬
	DWORD	dwSkill_Vanish_Time;
	DWORD	dwSkill_Vanish_Param;

	//배니쉬
	DWORD	dwSkill_Compulsion_Time;
	DWORD	dwSkill_Compulsion_Param;

	//마그네틱스피어
	DWORD	dwSkill_MagneticSphere_Time;
	DWORD	dwSkill_MagneticSphere_Param;

	//버서커
	DWORD	dwSkill_Berserker_Time;
	DWORD	dwSkill_Berserker_Param;

	//어세신아이
	DWORD	dwSkill_AssassinEye_Time;
	DWORD	dwSkill_AssassinEye_Param;

	//보그
	DWORD	dwSkill_Vague_Time;
	DWORD	dwSkill_Vague_Param;

	//포스오브네이쳐
	DWORD	dwSkill_ForceOfNature_Time;
	WORD	wSkill_ForceOfNature_Param[2];

	//가들리 쉴드
	DWORD	dwSkill_GodlyShied_Time;
	DWORD	dwSkill_GodlyShied_Param;

	//갓스블레스
	DWORD	dwSkill_GodsBless_Time;
	DWORD	dwSkill_GodsBless_Param;

	//홀오브발할라
	DWORD	dwSkill_HallOfValhalla_Time;
	WORD	wSkill_HallOfValhalla_Param[2];

	//프로스트재블린
	DWORD	dwSkill_FrostJavelin_Time;
	DWORD	dwSkill_FrostJavelin_Param;

	//리제너럴필드
	DWORD	dwSkill_RegenerationField_Time;

	//서몬 무스펠
	DWORD	dwSkill_SummonMuspell_Time;
	DWORD	dwSkill_SummonMuspell_Param;


	//////////////////////////// 저주 처리 //////////////////////////////
	//서큐버스 시리즈 저주
	DWORD	dwCurse_Attack_Time;		//공격 저주
	DWORD	dwCurse_Attack_Param;		//공격 저주

	DWORD	dwCurse_Defence_Time;		//방어 저주
	DWORD	dwCurse_Defence_Param;		//방어 저주

	////////////////////////// 스킬 포인트 감시 //////////////////////

	BYTE	bSkillPoint[SKILL_POINT_COLUM_MAX];

	////////////////////////////////////////////////////////////////


	//////////////////////////// 포스오브 ////////////////////////////

	DWORD	dwForceOrb_Code;			//포스오브 아이템 실행코드
	DWORD	dwForceOrb_Time;			//포스오브 유지시간
	DWORD	dwForceOrb_Damage;			//포스오브 데미지

	DWORD	dwForceOrb_SaveCode;		//포스오브 군서버 저장용
	DWORD	dwForceOrb_SaveTime;		//포스오브 군서버 저장용
	DWORD	dwForceOrb_SaveDamage;		//포스오브 군서버 저장용
	DWORD	dwForceOrb_NextSetTime;		//포스오브 사용정보 보내기 예약시간

	// 박재원 - 부스터 아이템(생명력)
	DWORD	dwLifeBooster_Code;			//부스터 아이템(생명력) 실행코드
	DWORD	dwLifeBooster_Time;			//부스터 아이템(생명력) 유지시간
	DWORD	dwLifeBooster_Data;			//부스터 아이템(생명력) 가중 퍼센트

	DWORD	dwLifeBooster_SaveCode;		//부스터 아이템(생명력) 군서버 저장용
	DWORD	dwLifeBooster_SaveTime;		//부스터 아이템(생명력) 군서버 저장용
	DWORD	dwLifeBooster_SaveData;		//부스터 아이템(생명력) 군서버 저장용 가중 퍼센트
	DWORD	dwLifeBooster_NextSetTime;	//부스터 아이템(생명력) 사용정보 보내기 예약시간

	// 박재원 - 부스터 아이템(기력)
	DWORD	dwManaBooster_Code;			//부스터 아이템(기력) 실행코드
	DWORD	dwManaBooster_Time;			//부스터 아이템(기력) 유지시간
	DWORD	dwManaBooster_Data;			//부스터 아이템(기력) 가중 퍼센트

	DWORD	dwManaBooster_SaveCode;		//부스터 아이템(기력) 군서버 저장용
	DWORD	dwManaBooster_SaveTime;		//부스터 아이템(기력) 군서버 저장용
	DWORD	dwManaBooster_SaveData;		//부스터 아이템(기력) 군서버 저장용 가중 퍼센트
	DWORD	dwManaBooster_NextSetTime;	//부스터 아이템(기력) 사용정보 보내기 예약시간

	// 박재원 - 부스터 아이템(체력)
	DWORD	dwStaminaBooster_Code;			//부스터 아이템(근력) 실행코드
	DWORD	dwStaminaBooster_Time;			//부스터 아이템(근력) 유지시간
	DWORD	dwStaminaBooster_Data;			//부스터 아이템(근력) 가중 퍼센트

	DWORD	dwStaminaBooster_SaveCode;		//부스터 아이템(근력) 군서버 저장용
	DWORD	dwStaminaBooster_SaveTime;		//부스터 아이템(근력) 군서버 저장용
	DWORD	dwStaminaBooster_SaveData;		//부스터 아이템(근력) 군서버 저장용 가중 퍼센트
	DWORD	dwStaminaBooster_NextSetTime;	//부스터 아이템(근력) 사용정보 보내기 예약시간


	//시간제 빌링 아이템 설정
	DWORD	dwPrimeItem_NextSetTime;	//시간제 빌링아이템 메인서버통보 예약시간
	DWORD	dwTime_PrimeItem_X2;		//아이템 2배 드롭
	DWORD	dwTime_PrimeItem_ExpUp;		//경험치업 포션
	DWORD	dwPrimeItem_PackageCode;	//패키지아이템 코드

	DWORD	dwTime_PrimeItem_VampCuspid;	//뱀피릭커스핏
	DWORD	dwTime_PrimeItem_ManaRecharg;	//마나리차징
	DWORD	dwTime_PrimeItem_MightofAwell;	// pluto 마이트 오브 아웰
	DWORD	dwTime_PrimeItem_MightofAwell2;	// pluto 마이트 오브 아웰
	DWORD	dwTime_PrimeItem_ManaReduce;	// pluto 마나 리듀스
	DWORD	dwTime_PrimeItem_PhenixPet;		// pluto 펫(해외)
	DWORD	dwTime_PrimeItem_HelpPet;	// 박재원 - 빌링 도우미 펫 추가
	DWORD	dwTime_PrimeItem_VampCuspid_EX;	// 장별 - 뱀피릭 커스핏 EX
	DWORD	dwTime_PrimeItem_StaminaReduce;	// 박재원 - 근력 리듀스 포션

	//pC방 펫
	DWORD	dwPCBang_Pet_AttackTime;	//PC방 펫 공격타임
	DWORD	dwPCBang_Pet_NextSetTime;	//PC방 펫 지역 서버통보 예약시간

	// 박재원 - 빌링 도우미 펫 추가
	DWORD	dwHelp_Pet_AttackTime;	//빌링 도우미 펫 공격타임
	DWORD	dwHelp_Pet_NextSetTime;	//빌링 도우미 펫 지역 서버통보 예약시간

	// 장별 - 스킬 딜레이
	DWORD	dwSkillDelay_Code;			
	DWORD	dwSkillDelay_Time;			
	DWORD	dwSkillDelay_Data;			

	DWORD	dwSkillDelay_SaveCode;		
	DWORD	dwSkillDelay_SaveTime;		
	DWORD	dwSkillDelay_SaveData;		
	DWORD	dwSkillDelay_NextSetTime;



	//////////////////////////// 소환몹 관리 ///////////////////////
	smCHAR	*lpLinkChar;
	DWORD	dwLinkCharCode;

	/////////////////////////// 공성 관련 //////////////////////////
	short	sBlessCastle_Damage[2];		//0-Damage 1-Killing Count
	DWORD	dwCheck_MasterClanTime;		//성주 클랜 확인 시간


	sCASTLE_MASTER	CastleMaster;		//성주 설정 정보
	DWORD	dwBlessSkill_Code;			//스킬 설정 코드
	DWORD	dwBlessSkill_Time;			//스킬 유지 시간
	DWORD	dwBlessSkill_Param;			//스킬 설정 파라메터

	DWORD	dwSiegeItem_Scroll_Time;	//공성 아이템 스크롤
	DWORD	dwSiegeItem_Scroll_Code;
	DWORD	dwSiegeItem_Scroll_Param;

	DWORD	dwSiegeItem_Stone1_Time;		//공성 아이템 저항석
	DWORD	dwSiegeItem_Stone1_Code;
	DWORD	dwSiegeItem_Stone1_Param;

	DWORD	dwSiegeItem_Stone2_Time;		//공성 아이템 강화석
	DWORD	dwSiegeItem_Stone2_Code;
	DWORD	dwSiegeItem_Stone2_Param;

	short	sCheckPlayField_Count[2];		//유저 필드 위치 확인 카운터


	//////////// Quest 진행 코드 보안용 //////////////

	DWORD	dwHaQuestItemCount;					//하퀘스트 아이템 지급 카운트


	///////////// 베트남 경험치 분배 관련 /////////////////////
	char	szUserExpTimeID[32];		//유저 ID(베트남 메인서버)
	char	szUserExpTimeName[32];		//유저 Name(베트남 메인서버)
	DWORD	dwExpGameTime;				//플레이시간
	DWORD	dwExpGameTimeCheck;			//플레이시간첵크용



//게임가드 보안 Lib
#ifdef	_npGAME_GUARD_AUTH
	 CCSAuth csa;
	npGAME_GUARD_AUTH_INFO	npGameGuard;
#endif

#ifdef _xTrap_GUARD
	smXTRAP_PLAYINFO	smXTrapInfo;
#endif

#ifdef _XTRAP_GUARD_4_SERVER
	//smXTRAP_PLAYINFO	smXTrapInfo;
	XTRAPCS_SESSION_BUF		XTrapCS_Session_Buf;
#endif

// 박재원 - XignCode
#ifdef _XIGNCODE_SERVER
	XIGNCODE_TempData	Xigncode_tempdata;
	int CE_code_count;
#endif


	////////////////////////////////////////////////////////////////


	//파티장
	int	CreateParty( rsPLAYINFO *lpPlayInfo );			//파티생성 - ( 초기 파티원 정보 )
	int	JoinPartyPlayer( rsPLAYINFO *lpPlayInfo );		//파티에 가입
	int	DeletePartyPlayer( rsPLAYINFO *lpPlayInfo );	//파티에서 탈퇴
	int ChangePartyPlayer( rsPLAYINFO *lpOldPlayInfo , rsPLAYINFO *lpNewPlayInfo );	//파티원 변경
	int	UpdatePartyPlayer();							//파티원 업데이트
	int	UpdatePartyPlayInfo();							//파티플레이 업데이트
	int GetPartyMoney( sITEMINFO *lpDefItemInfo , rsPLAYINFO *lpPlayInfo );	//파티원 돈 획득
	int GetPartyExp( smCHAR *lpChar , rsPLAYINFO *lpPlayInfo );				//파티원 경험치 획득
	int PartyChatting( rsPLAYINFO *lpPlayInfo );			//파티원 채팅메세지 보냄

	int	UpdateServerPartyPlayer( TRANS_PARTY_PLAYINFO *lpTransPartyPlayInfo );	//메인 서버에 파티원 정보 업데이트 /smTRANSCODE_PARTY_PLAYUPDATE


	int	LeavePartyMaster( rsPLAYINFO *lpPlayInfo );		//파티장 이임
	int	ReleaseParty();									//파티 해산

	//파티원에게 메세지 보냄
	int SendPartyMessage( char *szMessage , DWORD dwCode );		//파티전원에게 보내는 메세지
	//파티원에게 데이타 보냄
	int SendPartyData( rsPLAYINFO *lpPlayInfo_From , char *szData  );

	//파티원
	int	JoinParty( rsPLAYINFO *lpPlayMaster );			//파티 가입 신청
	int	SecedeParty ();									//파티 탈퇴
	int	DisconnectPartyUser();							//파티원 연결 끈어짐


	int	AddServerMoney( int Money , int WhereParam=0 );		//서버 머니 추가
	int	SubServerMoney( int Money , int WhereParam=0 );		//서버 머니 감소
	int	SetServerMoney( int Money , int WhereParam=0 );		//서버 머니 설정

	int	CallBack_ClanMarkNum( int ClanMarkNum );			//클랜마크 번호 알려줌


};
/////////////////////////////// 빌링 선언 ///////////////////////////////


// 리턴값: 	-1 : 게임상품 에러
//		 0 : 정액		- 날짜
//		 1 : 정량(시간)       	- 남은시간
//		 2 : 무료
//		 3 : 종량		- 분당 얼마 ( 포탈 - 예비 )
//	PG :   "0" : 개인
// 	       "1" : PC방

#define BILLING_GPTYPE_DAY		0
#define BILLING_GPTYPE_TIME		1
#define BILLING_GPTYPE_FREE		2
#define BILLING_GPTYPE_POTAL	3

#define BILLING_PGTYPE_PERSONAL	0
#define BILLING_PGTYPE_PCROOM	0




/////////////////////////////////////////////////////////////////////////////////////////

//내용 : 채팅 문자열 전송 코드
//대상 : 클라이언트 <-> 서버
//코드 : smTRANSCODE_CHATMESSAGE
struct TRANS_CHATBUFF {
	int		size;
	int		code;
	DWORD	dwIP;
};
struct CHATBUFF {
	DWORD	dwIP;
	char szMessage[256];
};

struct TRANS_CHATMESSAGE {
	int		size;
	int		code;
	DWORD	dwIP;
	DWORD	dwObjectSerial;	//객체의 고유값

	char szMessage[256];
};


struct smPLAYDATA_GROUP {
	int		size,code;
	int		PlayDataCnt;
	DWORD	dwSendCode;
};


#define smPLAYBUFF_FRAME_NONE			0xFFFF0000

#define	smPLAYBUFF_ITEM_BODY			0
#define	smPLAYBUFF_ITEM_LEFT			1
#define	smPLAYBUFF_ITEM_RIGHT			2
#define	smPLAYBUFF_ITEM_SKIL			3

struct smPLAYBUFF {
	int		x,y,z;
	DWORD	frame;
	short	angle[4];

	short	wStickItems[4];				//장착 아이템 번호
	DWORD	dwTargetSerial;				//목표 오브젝트 코드
};


//오른쪽 무기 / 왼쪽 무기 / 목표오브젝트

struct smPLAYDATA {
	int	size;
	int code;

	DWORD	dwObjectSerial;	//객체의 고유값

	int x,y,z;
	short angle[4];		//012->xyz 3->action code

	DWORD	frame;
	DWORD	dwAutoCharCode;		//오토 캐릭터 코드

};

struct smPLAYDATA2 {
	int	size;
	int code;

	DWORD	dwObjectSerial;	//객체의 고유값

	short	BuffCount[2];			//PlayBuff Time Posi
	smPLAYBUFF	PlayBuff[2];		//0-Begin 1-last
	DWORD	dwTarget;				//목표 점에 대한 정보

};

struct smPLAYDATA3 {
	int	size;
	int code;

	DWORD	dwObjectSerial;	//객체의 고유값

	short	BuffCount[3];			//PlayBuff Time Posi
	smPLAYBUFF	PlayBuff[3];		//0-Begin 1-last 2-change
	DWORD	dwTarget;				//목표 점에 대한 정보
};


//움직임 정보 값 전송의 실체
struct smTRANS_PLAYBUFF {
	int		BuffCount;
	smPLAYBUFF	PlayBuff;
};

#define TRANS_PLAYDATA_EVENT_LEVEL		0
#define TRANS_PLAYDATA_EVENT_POTION		1
#define TRANS_PLAYDATA_EVENT_SKIL		2
#define TRANS_PLAYDATA_EVENT_EXT		3

//움직임 정보 값 전송
struct smTRANS_PLAYDATA
{
	int	size;
	int code;

	DWORD	dwObjectSerial;		//객체의 고유값
	DWORD	dwTarget;			//목표 점에 대한 정보

	int		PlayBuffCnt;		//뒤에 연결된 smTRANS_PLAYBUFF 갯수
	int		StartPosi;			//PlayBuff  시작 위치

	short	Hp[2];				//체력값 
	DWORD	dwAutoCharCode;		//오토 캐릭터 코드
	short	wArea[2];			//지역 코드 //int		Etc2;
	BYTE	bUpdateInfo[4];		//내용 변경 카운터
	BYTE	bEventInfo[4];		//모션 이벤트 정보 ( 0-Level 1-Potion사용 2-스킬사용 3-예비 )
};

//아이템의 이펙트 정보
struct smEFFECT_ITEM {
	int size,code;

	DWORD	dwItemCode;

	int		ColorBlink;
	short	sColors[4];
	DWORD	DispEffect;
	DWORD	BlinkScale;
};


//캐릭터 전송요구
struct smTRANS_CHAR_COMMAND
{
	int	size;
	int code;

	int LParam;
	int WParam;
	int SParam;

	char	szName[32];
};



//캐릭터 전송요구
struct smTRANS_CHAR_COMMAND2
{
	int	size;
	int code;

	int LParam;
	int WParam;
	int SParam;

	char	szId[32];
	char	szName[32];
};


//내용 : 다른 플레이어와의 접속후 상대방 정보를 주고 받음
//대상 : 클라이언트 <-> 클라이언트
//코드 : smTRANSCODE_PLAYERINFO
struct smTRNAS_PLAYERINFO
{
	int	size;
	int code;

	smCHAR_INFO	smCharInfo;

	DWORD	dwObjectSerial;

	int	x,y,z;				//최초 좌표
	int ax,ay,az;			//최초 각도
	int state;				//속성
};

struct smTRNAS_PLAYERINFO_QUICK
{
	int	size;
	int code;

	DWORD	dwObjectSerial;

	int	x,y,z;				//최초 좌표
	int ax,ay,az;			//최초 각도
	int state;				//속성

	smCHAR_INFO	smCharInfo;
};


//간단한 코멘드 전송용 구조
struct smTRANS_COMMAND_DWORD
{
	int	size;
	int code;

	DWORD LParam;
	DWORD WParam;
	DWORD SParam;
	DWORD EParam;
};

struct smTRANS_COMMAND_EX
{
	int	size;
	int code;

	int LParam;
	int WParam;
	int SParam;
	int EParam;

	int LxParam;
	int WxParam;
	int SxParam;
	int ExParam;

};


struct smTRANS_COMMAND_BUFF {
	smTRANS_COMMAND		smTransCommand;
	char	*Buff[256];
};

struct smTRANS_COMMAND_SOD {
	smTRANS_COMMAND		smTransCommand;
	int		MyScore;
	int		MyTeam;
	int		TeamScore[4];
	int		Temp[4];
};

struct smTRANS_COMMAND_POLLING {
	smTRANS_COMMAND		smTransCommand;
	BYTE	bPolling[10];
};



//속성 호출
struct smTRANS_RESISTANCE {
	int	size;
	int code;
	int	Param;
	short Resistance[8];
	int	Absorb;
};


//공성
struct smTRANS_BLESSCASTLE {
	smTRANS_COMMAND		smTransCommand;
	int				TaxRate;
	short			Tower[12];		//타워설정	
	unsigned char	MercenaryNum[4];	//int		MercenaryType;	//용병설정
	int				ClanSkill;		//클랜스킬
	int				Price;
	DWORD			dwMasterClan;	//성주클랜
	int				Temp[4];
};

//공격순위 톱10
struct	rsUSER_LIST_TOP10 {
	//SYSTEM HEADER
	int		size,code;

	int		tCode;
	DWORD	dwObjectSerial;
	DWORD	dwNameCode;
	int		x,y,z;

	//BODY

	DWORD	dwCharCode;
	DWORD	dwUpdateTime;
	DWORD	dwTotalDamage;
	char	bCrystalTowerCount[4];
	DWORD	dwTemp[2];

	DWORD	dwUserCode[10];
	int		Damage[10];
	int		Counter[10];
};

/////////////// 아이템을 그룹으로 묶어서 전송 /////////////////////

/*
/////서버로 보내질 아이템 구조체 
struct sCRAFTITEM_SERVER{
	int size, code;
	int DocIndex;
	int Result;
	sITEM DesCraftItem;
	DWORD SheltomCode[12];
	DWORD Head[12];
	DWORD CheckSum[12];
	int Index;
	int Money;
};
*/


//아이템 그룹으로 전송
#define	TRANS_GROUPITEM_MAX		16
#define	TRANS_GROUPITEM_HEADER_SIZE		(14*4)

//압축을 푼경우
struct	TRANS_ITEMINFO_GROUP {
	int	size,code;

	int Index;
	int Result;
	int Money;
	int	ItemCount;

	DWORD	dwTemp[8];

	sITEMINFO	sItemInfo[TRANS_GROUPITEM_MAX];
};

//압축포맷
struct	TRANS_ITEMINFO_GROUP2 {
	int	size,code;

	int Index;
	int Result;
	int Money;
	int	ItemCount;

	DWORD	dwTemp[8];

	char	szBuff[ sizeof(sITEMINFO) * TRANS_GROUPITEM_MAX ];
};


//단일 아이템 전송
struct TRANS_ITEMINFO {
	//------------------------------------------------------------------< _ignore_bug_fix_0x00000000_ //해외
	/*
	TRANS_ITEMINFO()  // 생성자
	{
		size = 0,code = 0;x=0,y=0,z=0,
		ZeroMemory(&Item,sizeof(Item));
		for(int i=0;i<4;i++)
		{
			dwSeCode[i] = 0;
		}
		
	};
	*/
	//------------------------------------------------------------------  _ignore_bug_fix_0x00000000_ >
	int	size,code;

	//char	szItemName[32];
	//DWORD	ItemCode;

	sITEMINFO	Item;

	int x,y,z;

	DWORD	dwSeCode[4];		//보안 코드
};


struct	TRANS_ITEM_CODE {
	int size,code;

	DWORD	dwFlag;

	DWORD	dwItemCode;
	DWORD	dwHead;
	DWORD	dwChkSum;
	DWORD	dwTime;
};


struct TRANS_ITEM {
	int			state;

	int			type;
	int			ItemCode;

	int			x,y,z;
	char		szName[64];
	void		*lpStgArea;
	//char szModelName[64];
};


struct _TRANS_THROW_ITEM {
	DWORD	dwCode;
	DWORD	dwHead;
	DWORD	dwChkSum;
};

#define	THROW_ITEMS_MAX				32
//구조체 헤더 사이즈
#define	TRANS_THROW_ITEMS_DEFSIZE	(sizeof(int)*4)

struct TRANS_THROW_ITEMS {
	int		size,code;
	int		ThrowItemCount;
	int		Temp;

	_TRANS_THROW_ITEM	ThrowItems[THROW_ITEMS_MAX];
};



//거래후 아이템 보고
struct _TRANS_RECORD_TRADE_ITEM {
	DWORD	dwCode;
	DWORD	dwHead;
	DWORD	dwChkSum;
	int		State;
};

//sinTrade.h 에서 재정의
#define _MAX_TRADE_ITEM			20

struct TRANS_TRADE_ITEMKEY	{
	int size,code;
	DWORD	dwSender;
	DWORD	dwRecver;
	DWORD	dwSum;

	DWORD	dwItemCode[_MAX_TRADE_ITEM];
	DWORD	dwItemKey[_MAX_TRADE_ITEM];
	DWORD	dwItemSum[_MAX_TRADE_ITEM];

	int		Money;
};


#define	TRADE_RECV_ITEMS_MAX				32

//구조체 헤더 사이즈
#define	TRANS_RECORD_TRADE_DEFSIZE		(sizeof(int)*16)

struct TRANS_RECORD_TRADE_ITEMS {
	int		size,code;
	int		Money;
	int		ThrowItemCount;
	char	szTradeUserName[32];			//상대방 캐릭명
	int		Temp[4];

	_TRANS_RECORD_TRADE_ITEM	Items[TRADE_RECV_ITEMS_MAX];
};


#define TRANS_ITEM_MAX			16

//아이템 정보
struct smTRANS_ITEMS
{
	int size,code;

	int state;
	int Count;

	TRANS_ITEM	Items[ TRANS_ITEM_MAX ];
};

//아이템 정보
struct smTRANS_ACITON_ITEM
{
	int size,code;

	int state;

	int			x,y,z;
	void		*lpStgArea;
};
//상점 아이템 정보
struct smTRANS_SELL_ITEM {
	int size,code;
	int	ItemKind;					//공격 / 방어 /기타 등등의 구분
	int	ItemCounter;				//판매 아이템의 수
	int	Ext[4];						//예비
};

//아이템 교환 창
#define TRANS_TRADE_BUFF_SIZE		4000
struct TRANS_TRADEITEMS {
	int size,code;
	DWORD	dwSender;
	DWORD	dwRecver;
	DWORD	dwSum;
	DWORD	Temp[4];

	BYTE	TradeBuff[TRANS_TRADE_BUFF_SIZE];
};

#define	SOD_RESULT_MAX	32

struct	_SOD_RESULT {
	char		szName[32];
	rsPLAYINFO	*lpPlayInfo;
	int			JobCode;
	int			Class;
	int			Score;
	int			Monster;
	DWORD		dwTemp[4];
};

//SoD 순위 정보
struct	TRANS_SOD_RESULT {
	int	size,code;

	DWORD	dwGameCode;
	DWORD	dwGameTime;
	DWORD	dwDisplayMode;
	short	TeamRound[2];

	DWORD	dwTemp[15];

	int		ResultCount;

	_SOD_RESULT	Result[SOD_RESULT_MAX];
};



//////// 서버와 클라이언트의 돈,경험치 수치 조회 //////////
struct	sSERVER_MONEY {
	int	InputMoney;
	int	Counter;
	int	TotalX;
	int	TotalY;
	int	TotalZ;
};

struct sSERVER_EXP {
	int	Counter;
	int	InputExp;
	int	Total1;
	int	Total2;
};

struct TRANS_TOTAL_EXPMONEY {
	int	size,code;

	sSERVER_MONEY	sServerMoney;
	sSERVER_EXP		sServerExp;
};

//데이타 서버로 클라이언트 경유 전송
struct	TRANS_SEND_DATASERVER {
	int size,code;
	char	szBuff[1024];
};


//아이템 구입용 구조체
struct	TRANS_BUY_SHOPITEM {
	int	size,code;

	sITEM	sItem;			//구입하는 아이템
	int		ItemCount;		//구입하는 아이템 갯수
};



//거래시 아이템 보낸후 보낸 아이템이 인벤에 존재 하는지 확인
struct	_TRANS_USINGITEM {
	DWORD	dwCode;
	short	Performance[8];
};

struct	TRANS_USINGITEM {
	int	size,code;
	int	ItemListCount;
	_TRANS_USINGITEM	ItemList[16];
};



//트레이드 아이템 키 구조체

//크랙윈도우 서치 구조체
#define	FIND_CRACK_LIST_MAX		64

struct	_CRACK_NAME {
	char	szName1[12];
	char	szName2[12];
	char	szClass[12];
};

struct	TRANS_FIND_CRACKER {
	int		size,code;

	int		FindCounter;

	_CRACK_NAME	CrackName[FIND_CRACK_LIST_MAX];

	char	szTemp[128];
};


#define	PARTY_STATE_NONE		0
#define	PARTY_STATE_MASTER		1
#define	PARTY_STATE_MEMBER		2


struct _PARTY_USER {
	char	szName[32];				//이름
	char	szModelName[64];		//모델 이름
	char	szModelName2[64];		//모델 이름2

	DWORD	dwObjectSerial;			//객체 고유번호
	DWORD	Level;
	short	Life[2];				//생명력
	int		x,z;
};

struct TRANS_PARTY_USERS {
	int		size,code;

	DWORD	dwObjectSerial;
	DWORD	dwTemp[8];

	int		PartyUserCount;

	_PARTY_USER		PartyUser[PARTY_PLAYER_MAX];
};

struct _PARTY_PLAYINFO {
	DWORD	dwObjectSerial;			//객체 고유번호
	DWORD	Level;					//레벨
	short	Life[2];				//생명력
	int		x,z;
};

struct	TRANS_PARTY_PLAYINFO {
	int		size,code;

	DWORD	dwObjectSerial;
	DWORD	dwTemp;

	int		PartyUserCount;

	_PARTY_PLAYINFO	PlayInfo[PARTY_PLAYER_MAX];
};

struct	TRANS_PARTY_SKILL {
	int		size,code;
	DWORD	dwSkillCode;
	int		Point;
	int		wParam;
	int		lParam;
	int		sParam;
	int		eParam;
	int		PartyCount;
	DWORD	dwPartyUser[8];
};

struct TRANS_CLAN_COMMAND_USER {
	int size,code;
	int	Param[4];
	int	UserCount;

	char	szUserBuff[2048];
};

#define	CLAN_USER_MAX	64

struct _CLAN_USER_INFO {
	char		szName[32];
	DWORD		dwSpeedSum;
	DWORD		dwObjectSerial;
	rsPLAYINFO	*lpPlayInfo;
};

struct	rsCLAN_INFOMATION {
	DWORD	dwClanCode;
	DWORD	dwTemp[32];

	_CLAN_USER_INFO	ClanUserInfo[CLAN_USER_MAX];
};


//창고 저장 구조 
struct	TRANS_WAREHOUSE {
	int size,code;

	DWORD	dwChkSum;
	WORD	wVersion[2];
	int		WareHouseMoney;
	int		UserMoney;
	DWORD	dwTemp[5];

	int		DataSize;
	BYTE	Data[ sizeof( sITEM )*100+256 ];
};

#define	TRANS_SERVER_MAX		8

//서버 정보
struct _TRANS_SERVER_INFO {
	char	szServerName[32];
	char	szServerIp1[20];
	char	szServerIp2[20];
	char	szServerIp3[20];

	DWORD	dwServerPort1;
	DWORD	dwServerPort2;
	DWORD	dwServerPort3;

	DWORD	ConnState;
};


//서버 선택 목록 
struct TRANS_SERVER_LIST {
	int size,code;

	char	szServerName[16];

	DWORD	dwServerTime_t;
	int		ClanTicket;
	int	dwTemp[1];
	int	ClanServerCount;
	int	ServerCount;
	_TRANS_SERVER_INFO ServerInfo[TRANS_SERVER_MAX];
};

#define	SERVER_MAP_MAX		64

struct	TRANS_SEVER_MAP {
	int size,code;

	DWORD	dwTemp[30];

	int		IP_Count;
	DWORD	dwIP[SERVER_MAP_MAX];
};

struct	TT_DATA_SERVER_INFO {
	char	szServerID[16];
	char	szServerPath[128];
};


#define	SYSTEM_IP_MAX			32
#define	LOGIN_IP_MAX			16
#define	DISABLE_IP_MAX			500
#define	ENABLE_IP_MAX			100
#define	ADMIN_IP_MAX			16
#define CLAN_SERVER_MAX			3
#define ENABLE_MODULE_CHECK_MAX	64	
#define	TT_DATA_SERVER_MAX		16

//서버 설정 구조
struct rsSERVER_CONFIG	{

	DWORD	ServerCode;
	DWORD	AutoPlayer;
	DWORD	DisableParty;		//동료금지
	DWORD	EnablePlayerKill;	//PK허가
	DWORD	ConnectUserMax;		//최대 접속 허가 인원

	int		TestSeverMode;		//테스트 서버용

	int		UseRecorMemory;		//저장데이타 메모리 버퍼사용
	int		PotionMonitor;		//물약 감시

	int		ClientCheckSumCount;	//클라이언트 체크섬 카운터
	int		ClientCheckOut;			//문제 클라이언트 접속 종료
	DWORD	dwClientCheckSum[32];	//접속 가능한 체크섬 목록

	int		SafeClientCodeCount;	//클라이언트의 펑션첵크 코드카운트
	int		ClientCodeCheckOut;		//펑션문제 클라이언트 접속 종료
	DWORD	dwSafeClientCode[32];	//클라이언트의 펑션첵크 코드

	int		LimitVersion;			//접속 가능한 최하 버전
	int		LimitVersionMax;		//접속 가능한 최상 버전
	char	szAdminPassword[3][32];	//관리자 권한 부여 패스워드
	char	szAdminName[32];		//관리자 이름

	char	szAdminIP[16];			//관리자 IP
	char	szAdminLogPassword[32];	//관리자 Password

	char	szServerName[64];		//서버 이름
	int		Enable_PK;				//PK허가
	int		CheckProtect;			//nProtect 수시로 검사유무
	DWORD	RemoveNProtectTime;		//게임중에 nProtect제거 시간

	char	szDebugIP[10][32];		//디비깅용 IP
	int		DebugIP_Count;			//디버깅 IP 카운터

	char	szDebugID[32][32];		//디버깅용 ID
	int		DebugID_Count;			//디버깅용 ID 카운터

	int		BlockLevel;				//ID 임시 블럭의 레벨
	int		BlockLevel_LogOut;		//로그 아웃시에만 적용( 중국측 요구 )
	int		ItemCodeOut;			//아이템 코드 오류시 종료

	int		ClientFuncCheckMode;	//클라이언트 펑션 검사 모드
	int		Enable_PartyItem;		//동료가 획득한 아이템 출력

	
	int		Event_Mandoo;			//만두(송편) 이벤트
	int		Event_HardCore;			//하드코어 이벤트
	int		Event_DamageOff;		//공격력 활인 이벤트
//해외버전용/////////////////////////////////////////////////////////////////////////////
	//해외
//#ifdef _LANGUAGE_PHILIPIN
	int		Event_DamageOff1;		//몬스터 공격력 할인 이벤트1 (필리핀용)
	int		Event_DamageOff2;		//몬스터 공격력 할인 이벤트2 (필리핀용)
//#endif

	int		Event_DefenseOff;		//몬스터 방어력 약화 이벤트
/////////////////////////////////////////////////////////////////////////////////////////
	int		Event_Child;			//어린이 이벤트
	int		Event_Cristal;			//크리스탈 이벤트 ( 특정 시간대에 크리스탈 몬스터 강화 )
	int		Event_Potion;			//물약 이벤트 ( 물약 동시에 떨어지는 갯수 증가 )
	int		Event_StarPointTicket;	//별포인트 이벤트 ( 별상품권 발생 )
	int		Event_ValentineDay;		//발렌타인 데이 이벤트
	int		Event_WhiteDay;			//화이트 데이 이벤트
	int		Event_MorifSheltom;		//모리프의 쉘텀 탈취 이벤트
	int		Event_MollyWolf;		//몰리와늑대 이벤트
	int		Event_Ghost;			//고스트 이벤트
	int		Event_SantaGoblin;		//산타고블린 이벤트
	int		Event_WorldCup;			//월드컵 이벤트
	int		Event_ExpUp;			//경험치업 이벤트
	int		Event_CoupleRing;		// pluto 커플링 이벤트
	int		Event_Watermelon;		// 박재원 - 수박 이벤트
	int		Event_Star;				// 박재원 - 밤하늘의 소원이벤트
	int		Event_Valentine;		// 장별 - 발렌타인 이벤트
	int		Event_PristonAlphabet;	// 박재원 - 알파벳 조합 이벤트
	int		Event_WhiteContrast;	// 장별 - 화이트콘트라스트 이벤트
	int		Event_Candydays;		// 장별 - 캔디데이즈
	int		Event_BlackSmith;		// 장별 - 대장장이의 혼
	int		Event_MagicalGreen;	// 장별 - 매지컬그린
	int		Event_TearOfKara;		// 장별 - 카라의 눈물
	int		Event_WarerMelonMonster;		// 장별 - 수박몬스터 이벤트
	int		Event_Chichen_Day;		// 박재원 - 복날 이벤트(아기 꼬고 몬스터가 나타난다)
	int		Event_MovieBloodyBattle; // 장별 - 영화 혈투


#ifdef _LANGUAGE_ARGENTINA
	int		Event_ExpUp_latin;		//경험치업 이벤트(아르헨티나) //해외 Kyle
#endif

	int		Event_SummerChicken;	//삼계탕 이벤트
	int		Event_GiftBox;			// pluto 선물상자 이벤트

	int		BlessCastleMode;			//블레스캐슬 설정
	short	sBlessCastle_StartTime[2];	//블레스캐슬 시작 시간 예약 [0]요일 [1]시간

	int		Record_JobChanger;		//전업한자 기록
	int		ForeverBlockMode;		//영원한 블럭

	int		Permit_Exp;				//허용오차 경험치
	int		Permit_Money;			//허용오차 돈

	int		ClientProcessTimeChk;	//클라이언트 프로세스 시간감시 최대치
	int		ClientProcessTimeChkOut;//클라이언트 프로세스 시간 초과시 연결종료
	int		Dis_PotionError;		//포션 에러나면 연결 끈음

	int		ExpFixMode;				//오류 경험치 보정

	char	szBlockUserPath[64];	//유저 블럭 데이타 경로

	int		DispBillingTime;		//빌링 남은시간 표시

	int		Disable_LogIn;			//로그인 금지
	int		Disable_LogQue;			//로그큐 처리 금지
	int		Disable_CheckLogOut;	//로그아웃 큐검사 금지

	int		Disable_LogIn2;			//로그인 금지2번째 신규 로그인만 금지
	int		Disable_PCBang;			//PC방 금지 모드

	int		ExpGameTimeMode;		//경험치 시간별 분배(베트남)

	int		ServerInventoryMode;	//서버의 가상인벤토리로 아이템 관리

	DWORD	DisableIP[DISABLE_IP_MAX][2];		//접근 제한 IP
	int		DisableIP_Count;					//접근 제한 IP 설정 수

	DWORD	EnableIP[ENABLE_IP_MAX][2];			//접근 제한 IP중 허가되는 IP
	int		EnableIP_Count;						//접근 제한 IP 설정 수

	DWORD	AdminIP[ADMIN_IP_MAX][2];			//관리자 IP 목록
	int		AdminIP_Count;						//관리자 IP 설정 수

	DWORD	SystemIP[SYSTEM_IP_MAX][2];			//시스템 IP 목록
	int		SystemIP_Count;						//시스템 IP 설정 수

	DWORD	LoginServerIP[LOGIN_IP_MAX][2];		//로긴서버 IP 목록
	int		LoginServerIP_Count;				//로긴서버 IP 설정 수

	char	szBackupPath[80];					//*백업폴더이름

	char	szClanServerIP[CLAN_SERVER_MAX][20];	//클랜 서버	주소
	int		ClanServerPort[CLAN_SERVER_MAX];		//클랜 서버 포트
	int		ClanServerCount;


	DWORD	Enable_ClientModule[ENABLE_MODULE_CHECK_MAX];	//클라이언트 모튤첵크섬
	int		Enable_ClientModuleCount;						//클라이언트 모듈 첵크섬 카운터

	int		Disable_DecSkillPoint;							//스킬 포인트 감소 첵크 처리

	int		Disable_Quest;									//금지 퀘스트 설정

	int		CrazyPacketMode;							//보안 패킷 모드

	char	szNoticeURL[256];							//Notice URL 설정


	TRANS_SEVER_MAP	AreaServer;							//지역별 서버 매핑

	char	szLinkServerIP[32];							//링크된 서버 IP

	int		RecordAdminCommand;							//운영자 명령 기록

	int		FreeLevel;									//무료 레벨
	char	szFreeGpCode[32];							//무료사용 GPCODE
	int		PresentLevel;								//선물주는 레벨
	int		Event_ComicBirthDay;						//생일선물 얼큰이
	int		Event_Puzzle;								//퍼즐 맞추기 이벤트
	int		Event_BabelHorn;							//바벨뿔이벤트
	int		Event_NineFox;								//구미호이벤트
	int		EVENT_Christmas;							//크리스마스 이벤트
	int		EventPlay_BitMask;							//이벤트 플레이 비트마스크
	int		Event_PCBangPet;							//PC방 펫 이벤트

	int		StartUser_PresentItem;						//시작유저에게 선물 주기

	int		GameGuard_Auth;								//게임가드 보안 설정

#ifdef _XTRAP_GUARD_4_
	int		XTRAP_Timer;								//XTRAP 검사주기		XTRAP_GUARD
	int		XTRAP_MaxDealy;								//XTRAP 검사지체 제한치  XTRAP_GUARD
#endif

	//토너먼트서버 계정 관리
	int		TT_DataServer_Count;									//토너먼트서버 데이타 경로 갯수
	TT_DATA_SERVER_INFO		TT_DataServer[TT_DATA_SERVER_MAX];		//토너먼트서버 데이타 정보

	int		FixedStartField;							//시작 필드 강제 고정시키기
	int		DisableThrowItem;							//아이템 땅에 못버림

	int		CheckPlayField;								//클라이언트 와 서버의  필드 지형 비교 검사

	int		Enable_PKField;								// PK허용 필드 번호

	int		Enable_PKField_All;							// pluto PK허용 필드 전부

	//스킨핵 방지
	int		CheckModelFile;								//모델파일 확인 옵션

	//오프라인 솔로 플레이: DB 없이 모든 계정 로그인 허용 (*OFFLINE_MODE)
	int		OfflineMode;
	// ...with a password per ID: the first login sets it (*OFFLINE_PASSWORDS, shared web server)
	int		OfflinePasswords;
	// GM gold/exp/level commands (/GiveMeMoney!, /GiveMeExp!, /MakeMeLevel) for any GM (*GM_CHEATS, test server only)
	int		GmCheats;


	#ifdef LOG_THE_PK
		//PK_LOG관련 //해외
		bool	bPK_LOG;
		rsSERVER_CONFIG()
		{
			bPK_LOG = false;
		};
	#endif

};

//서버 기동 
extern int	ServerMode;

//서버측 연결/끈김 함수
//새 클라이언트 접속
int Serv_ConnectPlayer( smWINSOCK *lpsmSock );
//클라이언트 끈김
int Serv_DisconnectPlayer( smWINSOCK *lpsmSock );
//다른 캐릭터와의 위치 겹치는지 확인
smCHAR *Serv_CheckOtherPlayPosi( int x, int y, int z );

//경험치와 돈의 결과 한계치 ( 이한계를 넘어가면 저장이 안된다 )
#define	TOTAL_CHECK_MONEY_MAX		52000
#define	TOTAL_CHECK_EXP_MAX			100000




/////////////////// 클라이언트 펑션 위치 구조체 ////////////////////////
#define	dwFUNC_VALUE_MAX	64

struct	sFUNC_VALUE {
	DWORD	dwFunc;
	DWORD	dwLen;
	DWORD	dwChkSum;
};

struct	TRANS_CLIENT_FUNPOS {
	int	size,code;

	int	ClientVersion;

	DWORD	dwFunc_CheckMemSum;
	int		FuncCount;

	sFUNC_VALUE	dwFuncValue[dwFUNC_VALUE_MAX];
};


struct	TRANS_FUNC_MEMORY	{
	int		size,code;
	int		Param[8];
	char	szData[6000];
};


struct	sCHK_POTION_COUNT  {
	int	Ps101;
	int	Ps102;
	int	Ps103;
	int	Ps104;

	int	Pm101;
	int	Pm102;
	int	Pm103;
	int	Pm104;

	int	Pl101;
	int	Pl102;
	int	Pl103;
	int	Pl104;
};

//SOD 점수 출력
struct	sSOD_SCORE {
	int		Flag;
	DWORD	dwDispTime;
	int		Score;
	int		MyTeam;
	int		TeamScore[4];
	int		Round;
	DWORD	dwPlayTime;

	/////// SOD 연출 이벤트 /////////

	DWORD	dwSoD_NextStageTime;		//다음 스테이지 넘어가는 시간
	int		SodNextStageNum;			//넘어갈 스테이지 번호
	int		NextRound;
	DWORD	dwSoD_CloseStageTime;		//화면은 닫는 이펙트 적용시간

	///////// SOD 예약 연출 //////////////
	int		EffectStartCount;			//이펙트 시작까지의 카운터
	int		EffectCode;					//이펙트 코드

	//////// SOD 득점 효과 /////////////
	int		ScoreEffectCount;				//득점 효과

};


#define	JOBCODE_MECHANICIAN			2
#define	JOBCODE_FIGHTER				1
#define	JOBCODE_PIKEMAN				4
#define	JOBCODE_ARCHER				3

#define	JOBCODE_KNIGHT				6
#define	JOBCODE_ATALANTA			5
#define	JOBCODE_PRIESTESS			8
#define	JOBCODE_MAGICIAN			7

// the three later classes (tier 5 project). Their numbers are the ones the data's motion
// tables use: M6Bip (Assassin) tags its clips 0x100, M7Bip (Shaman) 0x200, M8Bip (Martial
// Artist) 0x400, i.e. 1<<(class-1)
#define	JOBCODE_ASSASSIN			9
#define	JOBCODE_SHAMAN				10
#define	JOBCODE_MARTIALARTIST		11
#define	JOBCODE_MAX					11
#ifndef NEW_CHAR_JOB_MAX
#define NEW_CHAR_JOB_MAX	6		// rows of TempNewCharacterInit / MorNewCharacterInit (HoLogin.cpp)
#endif

// Tempskron or Morion. The old code tells them apart with "class<=4"; the Assassin and the
// Martial Artist are Tempskron, the Shaman Morion
#ifndef JOB_IS_TEMPSKRON
#define	JOB_IS_TEMPSKRON(job)		( ((job)&0xFFFF)<=4 || ((job)&0xFFFF)==9 || ((job)&0xFFFF)==11 )
#endif


#define	CHAR_EVENT_STARPOINT		300

#define	ITEM_UNIQUE_CODE_NORMAL		1
#define	ITEM_UNIQUE_CODE_QUEST		2


#define BIMASK_PRORATE_USER			1
#define BIMASK_FREE_USER			2
#define BIMASK_VIP_USER				4
#define BIMASK_BIRTHDAY_USER		8
#define BIMASK_TIMELEVEL_USER		16
#define BIMASK_ADULT_USER			32


#define	MONSTER_CLASS_NORMAL		0
#define	MONSTER_CLASS_BOSS			1
#define	MONSTER_CLASS_HAMMER		200
#define	MONSTER_CLASS_GHOST			300

#endif