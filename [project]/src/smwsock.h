#ifndef	_SMWSOCK_HEADER

#define	_SMWSOCK_HEADER

#include "nettype.hpp"


#define _PS_TALE

#ifdef _PS_TALE
//프리스톤 테일 네트워크
#define TCP_GAMEPORT		8180
//#define TCP_SERVPORT		9110
#include "language.h"

#ifdef	_LANGUAGE_KOREAN				//한국어

#ifdef	_TEST_SERVER
//테스트 버전
#define TCP_SERVPORT		10009
#else
#define TCP_SERVPORT		10003
#endif

#endif

#ifdef	_LANGUAGE_CHINESE				//중국어
#define TCP_SERVPORT		10004
#endif

#ifdef	_LANGUAGE_TAIWAN				//중화민국어
#define TCP_SERVPORT		10005
#endif

#ifdef	_LANGUAGE_JAPANESE				//일본어
#define TCP_SERVPORT		10006
#endif

#ifdef	_LANGUAGE_ENGLISH				//영어

#if defined(PT_SERVER_PORT)				// a test build beside the live server (web\build_web.py, PT_DEFINES)
#define TCP_SERVPORT		PT_SERVER_PORT
#define PT_PORT_STR2(p)		#p
#define PT_PORT_STR(p)		PT_PORT_STR2(p)
#define PT_SERVER_PORT_TEXT	PT_PORT_STR(PT_SERVER_PORT)
#elif defined(_LANGUAGE_PHILIPIN)		//영어
#define TCP_SERVPORT		10010
#else
#define TCP_SERVPORT		10007
#endif
#endif

#ifdef _LANGUAGE_VEITNAM				//베트남
#define TCP_SERVPORT		10011
#endif

#ifdef _LANGUAGE_BRAZIL					//브라질
#define TCP_SERVPORT		10012
#endif

#ifdef _LANGUAGE_ARGENTINA				//아르헨티나
#define TCP_SERVPORT		10014
#endif

#ifdef	_LANGUAGE_THAI					//태국
#define TCP_SERVPORT		10008
#endif


//최대 연결 소켓수 ( 100 개 - 서버용 )


#ifdef _W_SERVER

#ifdef _W_DATA_SERVER
#define CONNECTMAX			4096
#define	CONNECTMAX_MASK		4095
#else
//서버
#define CONNECTMAX			1024
#define	CONNECTMAX_MASK		1023
#endif


#else
//클라이언트
#define CONNECTMAX			64
#define	CONNECTMAX_MASK		63

#endif


#else
//익사이트 게임서버
//최대 연결 소켓수 ( 2000 개 - 서버용 )
#define CONNECTMAX			1024
#define	CONNECTMAX_MASK		1023

#endif


//최대 패킷 크기 ( 8K Byte )
//#define smSOCKBUFF_SIZE	4096
#define smSOCKBUFF_SIZE	8192

extern int		smTransThreadMode;
extern int		smTransTurbRcvMode;

class smWINSOCK ;

struct smTHREADSOCK {
	smWINSOCK *smMySock;

	SOCKET Sock;

	int RecvLen;
	int RecvCode;

	int BuffCnt;

	int NewData;
	int Sucess;
	int Sleep;
	int SendReady;

	DWORD dwExitThread;
	DWORD dwAcessTime;

	char Buff[smSOCKBUFF_SIZE+256];
};


struct smTHREAD_ROUTE
{
	smTHREAD_ROUTE	*MyPoint;
	int Sleep;
	int TimeOut;
	smTHREADSOCK	*smThreadSock;
	DWORD dwSleepTime;

	HANDLE	hThread;
};

#define	ENC_PACKET_REC_MAX		16
#define	ENC_PACKET_REC_MASK		15

class smWINSOCK {
public:
	smWINSOCK *MyPoint;
	DWORD	MyNum;

	SOCKET sock;

	void *ExtData1;					//확장 데이타 포인터 1
	void *ExtData2;					//확장 데이타 포인터 2
	void *ExtData3;					//확장 데이타 포인터 3
	void *ExtData4;					//확장 데이타 포인터 4

	HANDLE hRecvThread;
	HANDLE hSendThread;

	smTHREADSOCK smRecvState;
	smTHREADSOCK smSendState;

	smTHREAD_ROUTE	*lpsmRecvRoute;		//외부 쓰레드 사용
	smTHREAD_ROUTE	*lpsmSendRoute;		//외부 쓰레드 사용

	int online;
	int status;
	char szBuff[128];
	char szIPAddr[16];
	DWORD	dwPort;

	DWORD	dwLastRecvTime;			//최근 수신 시간
	DWORD	dwDeadLockTime;			//데드락 첵크 시간

	int	WaitReadMessage;			//Read 메세지 대기 카운터
	int	PostReadMessage;			//Read 메세지 추가 카운터

	int	PacketSendQueCnt;			//보내기 패킷큐에 저장된 데이타

	int	EncPacketSendCount;			//암호화 패킷 송신 카운터
	int	EncPacketRecvCount;			//암호화 패킷 수신 카운터

	int	EncRecvPackets[ENC_PACKET_REC_MAX];			//최근 들어온 암호 패킷 16개를 기억
	int	EncRecvPacketCnt;							//최근 암호 패킷 수

	int ConnectDirection;			//연결 방향	( 0<-연결됨 1-연결함 )

	SOCKADDR_IN local_sin;  /* Local socket - internet style */
	SOCKADDR_IN acc_sin;    /* Accept socket address - internet style */
	int acc_sin_len;        /* Accept socket address length */

	DWORD dwRecvSusCount;

	int	RecvPacketCount;		//받은 패킷 카운터
	int	SendPacketCount;		//보낸 패킷 카운터
	int	RecvPopCount;			//강제로 받기 시도 카운트
	int	RecvPopErrorCount;		//강제로 받기 시도 실패 카운트
	int SendPacketErrorCount;	//보낸 패킷 오류 카운터
	int RecvPacketErrorCount;	//받은 패킷 오류 카운터

	DWORD	dwEncPacketCode;	//패킷 동적암호화 코드
	BYTE	bEncXor;			//패킷 동적암호화 Xor
	BYTE	bDecXor;			//패킷 동적암호화 Xor


public:
	smWINSOCK();
	~smWINSOCK();

	int mesAccept( WPARAM wParam , LPARAM lParam );
	int mesReceve( LPARAM lParam );

	int Send( char *SendData , int len , int ImpMode=0 );
	int Send2( char *SendData , int len , int ImpMode=0 );		//패킷을 암호화 하여 보냄
	int Send3( char *SendData , int len , int ImpMode=0 );		//패킷을 암호화 하여 보냄

	int	CheckEncRecvPacket( int RecvCount );					//최근 패킷과 겹쳐지는지 확인
	int	PushEncRecvPacket( int RecvCount );						//최근 패킷번호 기록

	//소켓 닫음
	int CloseSocket();
	//소켓을 재연결 시킨다
	int ReconnectSock();
	//연결
	int ConnectSock( char *szIP , WORD port );
	//주고 받을 쓰레드 생성
	int CreateTransThread( int Mode );



};


extern smWINSOCK	smWSock[CONNECTMAX];

#define WSA_ACCEPT			(WM_USER + 0)
#define WSA_READ			(WM_USER + 1)
#define SWM_SENDSUCCESS     (WM_USER + 10)
#define SWM_RECVSUCCESS     (WM_USER + 11)
#define SWM_MCONNECT		(WM_USER + 12)
#define SWM_MDISCONNECT		(WM_USER + 13)


//통신 오류 체크 카운터
extern	int	smTransRecvErrorCnt;
extern	int	smTransSendErrorCnt;
extern	int	smTransRecvErrorDisCnt;
extern	int	smTransSendErrorDisCnt;
extern	int smTransClearErrorCnt;
extern	int smRcvBuffOverCount;

//소켓 바인딩
void InitBindSock( int port );
//게임 소켓 초기화
void InitGameSocket( int ThreadMode=0 );
//바인딩 소켓 닫음
void CloseBindSock();
//자신의 IP주소 구함
char *smGetMyIp();
//자신의 IP주소 구함
DWORD smGetMyIp2();
//dwIP를 string 으로 변환
char *ConvStrIP( DWORD dwIP );

//WSA메세제 처리
int WSAMessage_Accept( DWORD wParam , DWORD lParam );
int WSAMessage_Read( DWORD wParam , DWORD lParam );

//소켓을 연결 시킨다
smWINSOCK *smConnectSock( char *szIP , WORD port );
//소켓을 연결 시킨다 ( 크리티컬 없음 )
smWINSOCK *smConnectSock3( char *szIP , WORD port );

//소켓을 연결 시킨다 ( IP를 확인하여 기존의 같은 IP가 존재하면 기존 IP 접속 불가 )
smWINSOCK *smConnectSock2( char *szIP , WORD port );
//IP로 소켓 찾기
smWINSOCK *smFindSocket( char *szIP );
//32비트 IP로 소켓을 찾음
smWINSOCK *smFindSocket2( DWORD dwIP );
//소켓을 끈고 다시 연결 시킨다
int smReconnectSock( smWINSOCK *lpsmSock );

//보내는 통신 대기큐에 소켓을 대기 시킨다 
int PushTransSendSocket( smWINSOCK *lpsmsock );
//보내는 통신 대기큐에서 대기 소켓을 꺼내온다 
smWINSOCK *PopTransSendSocket();
//받는 통신 대기큐에 소켓을 대기 시킨다 
int PushTransRecvSocket( smWINSOCK *lpsmsock );
//받는 통신 대기큐에서 대기 소켓을 꺼내온다 
smWINSOCK *PopTransRecvSocket();
//큐에 대기중인 소켓을 찾아 제거한다
int ClearTransQueSocket( smWINSOCK *lpsmsock );
//큐에 있는 소켓 내용 확인 하여 모두 지움 ( 연결이 끈어질 경우 )
int ClearPacketSendQue( smWINSOCK *lpsmSock );

//이상 있는 쓰레드를 찾아 처리 한다
int CheckLostTransThread();
//보내기 쓰레드의 사용 상태 구함
int GetUseingSendThreadCount();
//받기 쓰레드의 사용 상태 구함
int GetUseingRecvThreadCount();

//소켓용 크리티칼 섹션 선언
void smEnterCSectionSocket();
//소켓용 크리티칼 섹션 해제
void smLeaveCSectionSocket();

//대지중인 메세지가 있는지 확인하여 처리한다
int smCheckWaitMessage();
int smCheckWaitMessage( smWINSOCK *lpsmSock );

//암호화패킷 을 복구한다
DWORD DecodePacket( BYTE *lpPacket , BYTE *lpDecBuff );
//패킷을 암호화 한다
int	EncodePacket( DWORD dwKey , DWORD PacketCnt , BYTE *lpPacket , BYTE *lpEncBuff );

//패킷을 암호화 한다
int	EncodePacket2( DWORD dwKey , DWORD PacketCnt , BYTE *lpPacket , BYTE *lpEncBuff , BYTE bEncXor );
//암호화패킷 을 복구한다
DWORD DecodePacket2( BYTE *lpPacket , BYTE *lpDecBuff , BYTE bDecXor );



//패킷 동적 암호화 관련
typedef DWORD (*LPFN_EncodePacket)( DWORD dwKey , DWORD PacketCnt , BYTE *lpPacket , BYTE *lpEncBuff );
typedef DWORD (*LPFN_DecodePacket)( BYTE *lpPacket , BYTE *lpDecBuff );

extern LPFN_EncodePacket	fnEncodePacket;
extern LPFN_DecodePacket	fnDecodePacket;

#define	POS_ENC_START	0x0
#define	POS_ENC_PACCODE 0xB
#define POS_ENC_XOR		0x74

#define	POS_DEC_START	0xA0
#define	POS_DEC_XOR		0x11A

#define	ENC_PACKET_ASM_SIZE	0x150

extern BYTE smwsock_EncodePacket_AsmCode[ENC_PACKET_ASM_SIZE];


#endif