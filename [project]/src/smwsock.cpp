#define _WIN32_WINNT    0x0601  /* Get new material from the headers */
#define _WIN32_WINDOWS  0x040a

//#define	_W_SERVER

#include "nettype.hpp"

#ifdef _W_SERVER
//최대 소켓 갯수 ( 서버 )
#define	FD_SETSIZE 8192
#endif

#include <windows.h>
#include <io.h>
#include <stdio.h>         /* for sprintf                           */
#include <string.h>        /* for strlen                            */
#include <memory.h>
#include <process.h>       /* for _beginthread                      */
//#include <winsock2.h>
//#include <mswsock.h>
#include "smwsock.h"

#pragma comment( lib, "wsock32.lib" )

extern HWND hwnd;

//메세지 윈도핸들
static HWND	hMsgWnd;

#define TRANSMODE_RECV	0x0001
#define TRANSMODE_SEND	0x0002
#define TRANSMODE_DUAL	0x0003

CRITICAL_SECTION	cTransSection;
CRITICAL_SECTION	cSocketSection;
CRITICAL_SECTION	cPacketQueSection;
CRITICAL_SECTION	cSendingSection;

static char szIP[16];

//#define TRANS_THREAD_MAX		6

//암호화된 패킷
#define smTRANSCODE_ENCODE_PACKET	0x80010000


#if defined(_W_SERVER) && !defined(PT_WEB)

#define TRANS_THREAD_SEND_MAX		400
#define TRANS_THREAD_RECV_MAX		200
#define TRANS_WAIT_QUE_MAX			1024
#define TRANS_WAIT_QUE_MASK			1023
//// 보내기 패킷 저장 큐 ////
#define PACKET_SEND_QUE_MAX			1024
#define PACKET_SEND_QUE_MASK		(PACKET_SEND_QUE_MAX-1)

#elif defined(PT_WEB)
// browser: every thread is a Web Worker, and the offline server has one player
#define TRANS_THREAD_SEND_MAX		4
#define TRANS_THREAD_RECV_MAX		4
#define TRANS_WAIT_QUE_MAX			512
#define TRANS_WAIT_QUE_MASK			511
#define PACKET_SEND_QUE_MAX			256
#define PACKET_SEND_QUE_MASK		(PACKET_SEND_QUE_MAX-1)

#else

#define TRANS_THREAD_SEND_MAX		30
#define TRANS_THREAD_RECV_MAX		10
#define TRANS_WAIT_QUE_MAX			512
#define TRANS_WAIT_QUE_MASK			511
//// 보내기 패킷 저장 큐 ////
#define PACKET_SEND_QUE_MAX			256
#define PACKET_SEND_QUE_MASK		(PACKET_SEND_QUE_MAX-1)

#endif

smTHREAD_ROUTE smTransRecvThreads[ TRANS_THREAD_RECV_MAX ];
smTHREAD_ROUTE smTransSendThreads[ TRANS_THREAD_SEND_MAX ];

smWINSOCK *smWaitSendSocketQue[ TRANS_WAIT_QUE_MAX ];
smWINSOCK *smWaitRecvSocketQue[ TRANS_WAIT_QUE_MAX ];
int	smWaitSendQuePop;
int	smWaitSendQuePush;
int	smWaitRecvQuePop;
int	smWaitRecvQuePush;

int		smTransThreadMode = 0;
int		smTransTurbRcvMode = 0;

//통신 오류 체크
int		smTransRecvErrorCnt = 0;
int		smTransSendErrorCnt = 0;
int		smTransRecvErrorDisCnt = 0;
int		smTransSendErrorDisCnt = 0;

int		smTransClearErrorCnt = 0;
int		smRcvBuffOverCount = 0;

//소켓 선언
smWINSOCK	smWSock[CONNECTMAX];


#define _DEBUG_SOCKET

#ifdef _DEBUG_SOCKET

#include "smlib3d\\smd3d.h"
extern smCONFIG smConfig;
extern int AddChatBuff( char *szMsg );

#endif


struct PACKET_SEND_QUE {

	smWINSOCK	*lpsmSock;
	char	PacketQue[ smSOCKBUFF_SIZE ];
	int		Len;

};

PACKET_SEND_QUE	PacketSendQue[PACKET_SEND_QUE_MAX];

int	PacketSendQuePush;

//보내기 패킷 큐를 초기화
int	InitPacketSendQue()
{
	int cnt;

	//크리티칼 섹션 초기화
	InitializeCriticalSection( &cPacketQueSection );

	PacketSendQuePush = 0;

	for( cnt=0;cnt<PACKET_SEND_QUE_MAX;cnt++) 
		PacketSendQue[cnt].lpsmSock = 0;


	return TRUE;
}


//보내기 패킷 큐를 종료
int	ClosePacketSendQue()
{
	DeleteCriticalSection(&cPacketQueSection);
	return TRUE;
}

//보내기 패킷 큐에 데이타 삽입
int PushPacketSendQue( smWINSOCK *lpsmSock, char *lpData , int len )
{
	int QuePos;

	EnterCriticalSection( &cPacketQueSection );	//크리티칼 섹션 선언

	QuePos = PacketSendQuePush & PACKET_SEND_QUE_MASK;

	//아직도 안가구 멈춰 있는 데이타 발견하면 확인하여 밀어버림
	if ( PacketSendQue[QuePos].lpsmSock ) {
		PacketSendQue[QuePos].lpsmSock->PacketSendQueCnt --;
	}

	//큐에 저장
	memcpy( PacketSendQue[QuePos].PacketQue	, lpData , len );
	PacketSendQue[QuePos].lpsmSock = lpsmSock;
	PacketSendQue[QuePos].Len = len;

	PacketSendQuePush++;
	lpsmSock->PacketSendQueCnt++;

	LeaveCriticalSection( &cPacketQueSection );	//크리티칼 섹션 해제

	return TRUE;
}

//큐에 있는 소켓 내용 확인 하여 모두 지움 ( 연결이 끈어질 경우 )
int ClearPacketSendQue( smWINSOCK *lpsmSock )
{
	int cnt;
	int StartQuePos;
	int QuePos;

	EnterCriticalSection( &cPacketQueSection );	//크리티칼 섹션 선언

	StartQuePos=PacketSendQuePush-PACKET_SEND_QUE_MAX;
	if ( StartQuePos<0 ) StartQuePos=0;

	for( cnt=StartQuePos;cnt<PacketSendQuePush;cnt++) {
		QuePos = cnt & PACKET_SEND_QUE_MASK;
		if ( PacketSendQue[QuePos].lpsmSock==lpsmSock ) {
			PacketSendQue[QuePos].lpsmSock = 0;
			lpsmSock->PacketSendQueCnt --;
		}
	}

	LeaveCriticalSection( &cPacketQueSection );	//크리티칼 섹션 해제

	return TRUE;
}

//보내기 패킷 큐에서 데이타를 찾아 재 전송
int PopSendPacketQue( smWINSOCK *lpsmSock )
{
	int cnt;
	int StartQuePos;
	int QuePos;

	EnterCriticalSection( &cPacketQueSection );	//크리티칼 섹션 선언

	StartQuePos=PacketSendQuePush-PACKET_SEND_QUE_MAX;
	if ( StartQuePos<0 ) StartQuePos=0;

	for( cnt=StartQuePos;cnt<PacketSendQuePush;cnt++) {
		QuePos = cnt & PACKET_SEND_QUE_MASK;
		if ( PacketSendQue[QuePos].lpsmSock==lpsmSock ) {
			if ( lpsmSock->Send( PacketSendQue[QuePos].PacketQue , PacketSendQue[QuePos].Len )==TRUE ) {
				//보내기 성공
				PacketSendQue[QuePos].lpsmSock = 0;
				lpsmSock->PacketSendQueCnt--;
				LeaveCriticalSection( &cPacketQueSection );	//크리티칼 섹션 해제
				return TRUE;
			}
			break;
		}
	}

	LeaveCriticalSection( &cPacketQueSection );	//크리티칼 섹션 해제

	return FALSE;
}



//쓰레드에서 보내기 성공
int SucessPacketSending( smTHREADSOCK *SockInfo )
{

	//큐에서 다음 보낼 패킷이 있는지 확인하여 보냄
	if ( SockInfo->smMySock->PacketSendQueCnt ) {
		PopSendPacketQue( SockInfo->smMySock );
	}
	else
		//패킷 전송 완료 메세지 
		SendMessage( hMsgWnd , SWM_SENDSUCCESS , (WPARAM)SockInfo , 0 );

	return TRUE;
}


//TCP 통신 받는쓰레드
DWORD WINAPI smTransRecvThreadProc( void *pInfo )
{
	smTHREAD_ROUTE *ThreadRoute;
	smTHREADSOCK *SockInfo;
	smWINSOCK	*lpsmsock;
	int len;
	char szBuff[smSOCKBUFF_SIZE];
	char *Buff;
	int RecvLen;
	int BuffCnt;
	int BuffSize;
	int RecvCode;
	HANDLE	hThread;

	ThreadRoute = (smTHREAD_ROUTE *)pInfo;
	SockInfo = ThreadRoute->smThreadSock;
	RecvLen = 0;
	BuffCnt = 0;
	BuffSize = smSOCKBUFF_SIZE;
	hThread = GetCurrentThread();
	ThreadRoute->TimeOut = FALSE;
	int result;

	while(1) {
StartPos:
			if ( (SockInfo && SockInfo->NewData==0) || !SockInfo ) {
				ThreadRoute->dwSleepTime = GetCurrentTime();	//슬립 시간 기록
				ThreadRoute->Sleep = TRUE;
				if ( SockInfo ) SockInfo->Sleep = TRUE;

				SuspendThread( hThread );

				SockInfo = ThreadRoute->smThreadSock;
				ThreadRoute->Sleep = FALSE;

				if ( !ThreadRoute->TimeOut ) {
					SockInfo->Sucess = FALSE;
					SockInfo->Sleep = TRUE;
					//억세스 시간 구함
					SockInfo->dwAcessTime = GetCurrentTime();
				}
			}

			if ( ThreadRoute->TimeOut || !SockInfo->smMySock->sock ||
				SockInfo->smMySock->lpsmRecvRoute!=ThreadRoute->MyPoint ) {
				//타임 아웃~~
				RecvLen = 0;
				RecvCode = 0;
				BuffCnt = 0;
				//BuffSize = smSOCKBUFF_SIZE;

				EnterCriticalSection( &cTransSection );	//크리티칼 섹션 선언

				if ( SockInfo->smMySock->lpsmRecvRoute==ThreadRoute->MyPoint ) {
					SockInfo->smMySock->lpsmRecvRoute = 0;
					SockInfo->dwAcessTime = 0;
				}
				ThreadRoute->smThreadSock = 0;
				SockInfo = 0;
				ThreadRoute->TimeOut = FALSE;

				smTransRecvErrorCnt ++;					//에레 체크

				LeaveCriticalSection( &cTransSection );	//크리티칼 섹션 해제

				goto StartPos;
			}

			Buff = szBuff + BuffCnt;
			BuffSize = smSOCKBUFF_SIZE - BuffCnt;

			len = recv( SockInfo->Sock , Buff , BuffSize , 0 );

			if ( BuffCnt<8 ) RecvLen = smSOCKBUFF_SIZE-32;

			if ( BuffCnt<8 && (BuffCnt+len)>=8 ) {
				//RecvLen = 1000;
				RecvLen = (((int *)szBuff)[0]);
				RecvCode = (((int *)szBuff)[1]);
			}

			if ( len>0 ) {

				BuffCnt += len;
				/*
				BuffSize = smSOCKBUFF_SIZE - BuffCnt;

				if ( BuffSize <= 0 )  {
					BuffSize = 0;
				}
				*/
				SockInfo->NewData --;
				SockInfo->smMySock->RecvPacketCount++;		//받은 패킷 카운터
			}
			else {

				SockInfo->NewData = 0;

				if ( SockInfo->smMySock->WaitReadMessage>0 )
					SockInfo->smMySock->RecvPopErrorCount++;	//강제 받기 시도 오류 기록
				else
					SockInfo->smMySock->RecvPacketErrorCount++;		//받기 시도 오류 기록


				if ( len==0 ) {
					//연결이 끊어진 걸로 추측
					SockInfo->smMySock->WaitReadMessage = -1;
				}
				else 
					SockInfo->smMySock->WaitReadMessage = 0;


				if ( BuffCnt==0 ) {

						RecvLen = 0;
						BuffCnt = 0;

						EnterCriticalSection( &cTransSection );	//크리티칼 섹션 선언

						if ( SockInfo->smMySock->lpsmRecvRoute==ThreadRoute->MyPoint ) {
							SockInfo->smMySock->lpsmRecvRoute = 0;
							SockInfo->dwAcessTime = 0;
						}
						ThreadRoute->smThreadSock = 0;
						SockInfo = 0;

						ThreadRoute->TimeOut = FALSE;

						lpsmsock = PopTransRecvSocket();
						if ( lpsmsock ) {
							lpsmsock->lpsmRecvRoute = ThreadRoute->MyPoint;
							ThreadRoute->smThreadSock = &lpsmsock->smRecvState;
							SockInfo = &lpsmsock->smRecvState;
						}

						LeaveCriticalSection( &cTransSection );	//크리티칼 섹션 해제

						goto StartPos;
				}
			}

			//예상치 않은 오류 ~~ 연결을 끈어버림
			if ( BuffCnt>smSOCKBUFF_SIZE || RecvLen<0 || RecvLen>smSOCKBUFF_SIZE ) {
				//오류 ~~
				RecvLen = 0;
				BuffCnt = 0;
				//소켓용 크리티칼 섹션 선언
				smEnterCSectionSocket();
				//연결을 끈는다
				result = SockInfo->smMySock->CloseSocket();
				//소켓용 크리티칼 섹션 해제
				smLeaveCSectionSocket();
/*
				if ( result ) {
					DisconnectPlayer( SockInfo->smMySock );
				}
*/	
				smTransRecvErrorDisCnt ++;					//에레 체크 ( 연결 강제 종료 )

				SockInfo = 0;
				goto StartPos;
			}

			while(1) {
				if ( RecvLen<=0 ) break;

				if ( BuffCnt>=RecvLen ) {
					//패킷 수신 완료 
					memcpy( SockInfo->Buff , szBuff , RecvLen );
					SockInfo->Buff[ BuffCnt ] = 0;
					SockInfo->RecvLen = RecvLen;
					SockInfo->RecvCode = RecvCode;
					SockInfo->BuffCnt = BuffCnt;
					SockInfo->Sucess = TRUE;

					SockInfo->smMySock->dwLastRecvTime = GetCurrentTime();
					SendMessage( hMsgWnd , SWM_RECVSUCCESS , (WPARAM)SockInfo , 0 );

					if ( RecvLen<BuffCnt ) {
						//받은 길이가 패킷 수신 길이보다 클경우 
						//..(붙어서 다음 패킷까지 온 경우 )
						BuffCnt -= RecvLen;
						memmove( szBuff , szBuff+RecvLen , BuffCnt );		// overlapping: memmove (memcpy's result is undefined)

						if ( BuffCnt>=8 ) {
							RecvLen = (((int *)szBuff)[0]);
							RecvCode = (((int *)szBuff)[1]);
							BuffSize = smSOCKBUFF_SIZE - BuffCnt;
						}
						else{
							//패킷 길이가 짧음 ( 다음 패킷 수신을 기다림 )
							RecvLen = 0;
							RecvCode = 0;
							/*
							//BuffCnt = 0;
							//BuffSize = smSOCKBUFF_SIZE - BuffCnt;

							EnterCriticalSection( &cTransSection );	//크리티칼 섹션 선언

							if ( SockInfo->smMySock->lpsmRecvRoute==ThreadRoute->MyPoint ) {
								SockInfo->smMySock->lpsmRecvRoute = 0;
								SockInfo->dwAcessTime = 0;
							}
							ThreadRoute->smThreadSock = 0;
							SockInfo = 0;

							ThreadRoute->TimeOut = FALSE;

							lpsmsock = PopTransRecvSocket();
							if ( lpsmsock ) {
								lpsmsock->lpsmRecvRoute = ThreadRoute->MyPoint;
								ThreadRoute->smThreadSock = &lpsmsock->smRecvState;
								SockInfo = &lpsmsock->smRecvState;
							}
							LeaveCriticalSection( &cTransSection );	//크리티칼 섹션 해제
							*/
							break;
						}
					}
					else {
						RecvLen = 0;
						BuffCnt = 0;
						//BuffSize = smSOCKBUFF_SIZE;

						if ( SockInfo->smMySock->WaitReadMessage>0 ) {
							SockInfo->NewData = TRUE;
							SockInfo->smMySock->WaitReadMessage --;
							break;
						}

						EnterCriticalSection( &cTransSection );	//크리티칼 섹션 선언

						if ( SockInfo->smMySock->lpsmRecvRoute==ThreadRoute->MyPoint ) {
							SockInfo->smMySock->lpsmRecvRoute = 0;
							SockInfo->dwAcessTime = 0;
						}
						ThreadRoute->smThreadSock = 0;
						SockInfo = 0;


						ThreadRoute->TimeOut = FALSE;

						lpsmsock = PopTransRecvSocket();
						if ( lpsmsock ) {
							lpsmsock->lpsmRecvRoute = ThreadRoute->MyPoint;
							ThreadRoute->smThreadSock = &lpsmsock->smRecvState;
							SockInfo = &lpsmsock->smRecvState;
						}
						LeaveCriticalSection( &cTransSection );	//크리티칼 섹션 해제
						break;
					}
				}
				else
					break;
			}

			//소켓이 닫히거나 해서 에러가 난 경우
			if ( SockInfo && !SockInfo->smMySock->sock ){
					RecvLen = 0;
					BuffCnt = 0;
					//BuffSize = smSOCKBUFF_SIZE;

					EnterCriticalSection( &cTransSection );	//크리티칼 섹션 선언

					if ( SockInfo->smMySock->lpsmRecvRoute==ThreadRoute->MyPoint ) {
						SockInfo->smMySock->lpsmRecvRoute = 0;
						SockInfo->dwAcessTime = 0;
					}
					ThreadRoute->smThreadSock = 0;
					SockInfo = 0;
					ThreadRoute->TimeOut = FALSE;

					smTransRecvErrorCnt ++;					//에레 체크

					LeaveCriticalSection( &cTransSection );	//크리티칼 섹션 해제
			}
	}

	ExitThread( TRUE );
	return TRUE;
}

DWORD WINAPI smTransSendThreadProc( void *pInfo )
{
	smTHREAD_ROUTE *ThreadRoute;

	smTHREADSOCK *SockInfo;
	smWINSOCK	*lpsmsock;
	char *szBuff;//[smSOCKBUFF_SIZE];
	int BuffCnt;
	int stats;
	int BuffLen;

	HANDLE	hThread;

	ThreadRoute = (smTHREAD_ROUTE *)pInfo;
	SockInfo = ThreadRoute->smThreadSock;
	hThread = GetCurrentThread();
	ThreadRoute->TimeOut = FALSE;

	while(1) {
StartPos:
			if ( (SockInfo && SockInfo->NewData==0) || !SockInfo ) {
				ThreadRoute->dwSleepTime = GetCurrentTime();	//슬립 시간 기록
				ThreadRoute->Sleep = TRUE;
				if ( SockInfo ) SockInfo->Sleep = TRUE;

				SuspendThread( hThread );

				SockInfo = ThreadRoute->smThreadSock;
				ThreadRoute->Sleep = FALSE;

				if ( !SockInfo ) continue;

				if ( !ThreadRoute->TimeOut ) {
					SockInfo->Sleep = TRUE;
					SockInfo->Sucess = FALSE;
					//억세스 시간 구함
					SockInfo->dwAcessTime = GetCurrentTime();
				}
			}

			if ( ThreadRoute->TimeOut || !SockInfo->smMySock->sock ||
				SockInfo->smMySock->lpsmSendRoute!=ThreadRoute->MyPoint ) {

				EnterCriticalSection( &cTransSection );	//크리티칼 섹션 선언

				if ( SockInfo->smMySock->lpsmSendRoute==ThreadRoute->MyPoint ) {
					SockInfo->smMySock->lpsmSendRoute = 0;
					SockInfo->dwAcessTime = 0;
				}
				ThreadRoute->smThreadSock = 0;
				SockInfo = 0;
				ThreadRoute->TimeOut = FALSE;

				smTransSendErrorCnt ++;					//에레 체크

				LeaveCriticalSection( &cTransSection );	//크리티칼 섹션 해제

				goto StartPos;
			}

			if ( SockInfo->NewData ) {
				BuffLen = SockInfo->BuffCnt;
				BuffCnt = 0;
				SockInfo->NewData = FALSE;
			}

			szBuff = SockInfo->Buff + BuffCnt;

			stats = send( SockInfo->Sock , szBuff , BuffLen , 0 );

			if ( stats != SOCKET_ERROR ) {

				SockInfo->smMySock->SendPacketCount++;		//보낸 패킷 카운터

				if ( stats!=BuffLen ) {
					stats = stats;
				}

				BuffCnt +=stats;
				BuffLen -=stats;
				if ( BuffLen <= 0 ) {
					SockInfo->Sucess = TRUE;
					//SendMessage( hMsgWnd , SWM_SENDSUCCESS , (WPARAM)SockInfo , 0 );
					//쓰레드에서 보내기 성공
					SucessPacketSending( SockInfo );

					EnterCriticalSection( &cTransSection );	//크리티칼 섹션 선언

					if ( SockInfo->smMySock->lpsmSendRoute==ThreadRoute->MyPoint ) {
						SockInfo->smMySock->lpsmSendRoute = 0;
						SockInfo->dwAcessTime = 0;
					}
					ThreadRoute->smThreadSock = 0;
					SockInfo = 0;
					ThreadRoute->TimeOut = FALSE;

					lpsmsock = PopTransSendSocket();
					if ( lpsmsock ) {
						lpsmsock->lpsmSendRoute = ThreadRoute->MyPoint;
						ThreadRoute->smThreadSock = &lpsmsock->smSendState;
						SockInfo = &lpsmsock->smSendState;
					}
					LeaveCriticalSection( &cTransSection );	//크리티칼 섹션 해제
				}
			}
			else {
				SockInfo->smMySock->SendPacketErrorCount++;		//보내기 시도 오류 기록
			}

			//소켓이 닫히거나 해서 에러가 난 경우
			if ( SockInfo && !SockInfo->smMySock->sock ){

					EnterCriticalSection( &cTransSection );	//크리티칼 섹션 선언

					if ( SockInfo->smMySock->lpsmSendRoute==ThreadRoute->MyPoint ) {
						SockInfo->smMySock->lpsmSendRoute = 0;
						SockInfo->dwAcessTime = 0;
					}
					ThreadRoute->smThreadSock = 0;
					SockInfo = 0;
					ThreadRoute->TimeOut = FALSE;

					smTransSendErrorCnt ++;					//에레 체크

					LeaveCriticalSection( &cTransSection );	//크리티칼 섹션 해제
			}
	}

	ExitThread( TRUE );
	return TRUE;
}



//통신 쓰레드 초기화
void InitTransThread()
{
	int cnt;
	DWORD dwRecvMsgId;
	DWORD dwSendMsgId;

	//크리티칼 섹션 초기화
	InitializeCriticalSection( &cTransSection );

	smWaitSendQuePop = 0;
	smWaitSendQuePush = 0;
	smWaitRecvQuePop = 0;
	smWaitRecvQuePush = 0;


	for( cnt=0;cnt<TRANS_THREAD_SEND_MAX;cnt++) {
		smTransSendThreads[cnt].smThreadSock = 0;
		smTransSendThreads[cnt].MyPoint = &smTransSendThreads[cnt];

		smTransSendThreads[cnt].hThread = CreateThread( NULL , 0, smTransSendThreadProc , &smTransSendThreads[cnt] , 0, &dwRecvMsgId );
		//SetThreadPriority( smTransSendThreads[cnt].hThread , THREAD_PRIORITY_ABOVE_NORMAL );
	}

	for( cnt=0;cnt<TRANS_THREAD_RECV_MAX;cnt++) {
		smTransRecvThreads[cnt].smThreadSock = 0;
		smTransRecvThreads[cnt].MyPoint = &smTransRecvThreads[cnt];

		smTransRecvThreads[cnt].hThread = CreateThread( NULL , 0, smTransRecvThreadProc , &smTransRecvThreads[cnt] , 0, &dwSendMsgId );
		//SetThreadPriority( smTransRecvThreads[cnt].hThread , THREAD_PRIORITY_ABOVE_NORMAL );
	}

}

//보내는 통신 대기큐에 소켓을 대기 시킨다 
int PushTransSendSocket( smWINSOCK *lpsmsock )
{
	//크리티칼 섹션 선언
	//EnterCriticalSection( &cTransSection );

	smWaitSendSocketQue[ smWaitSendQuePush&TRANS_WAIT_QUE_MASK ] = lpsmsock;
	smWaitSendQuePush++;

	//크리티칼 섹션 해제
	//LeaveCriticalSection( &cTransSection );

	return smWaitSendQuePush;
}



//보내는 통신 대기큐에서 대기 소켓을 꺼내온다 
smWINSOCK *PopTransSendSocket()
{
	smWINSOCK *lpsmsock;

	//크리티칼 섹션 선언
//	EnterCriticalSection( &cTransSection );

	lpsmsock = NULL;

	while(1) {
		if ( smWaitSendQuePop>=smWaitSendQuePush ) break;

		lpsmsock = smWaitSendSocketQue[ smWaitSendQuePop&TRANS_WAIT_QUE_MASK ];
		smWaitSendQuePop++;

		if ( lpsmsock ) break;
	}

	//크리티칼 섹션 해제
//	LeaveCriticalSection( &cTransSection );

	return lpsmsock;
}



//받는 통신 대기큐에 소켓을 대기 시킨다 
int PushTransRecvSocket( smWINSOCK *lpsmsock )
{
	//크리티칼 섹션 선언
	//EnterCriticalSection( &cTransSection );

	smWaitRecvSocketQue[ smWaitRecvQuePush&TRANS_WAIT_QUE_MASK ] = lpsmsock;
	smWaitRecvQuePush++;

	//크리티칼 섹션 해제
	//LeaveCriticalSection( &cTransSection );

	return smWaitRecvQuePush;
}


//받는 통신 대기큐에서 대기 소켓을 꺼내온다 
smWINSOCK *PopTransRecvSocket()
{
	smWINSOCK *lpsmsock;

	//크리티칼 섹션 선언
//	EnterCriticalSection( &cTransSection );

	lpsmsock = NULL;

	while(1) {
		if ( smWaitRecvQuePop>=smWaitRecvQuePush ) break;

		lpsmsock = smWaitRecvSocketQue[ smWaitRecvQuePop&TRANS_WAIT_QUE_MASK ];
		smWaitRecvQuePop++;

		if ( lpsmsock ) break;
	}

	//크리티칼 섹션 해제
//	LeaveCriticalSection( &cTransSection );

	return lpsmsock;
}

//보내기 쓰레드의 사용 상태 구함
int GetUseingSendThreadCount()
{
	int cnt;
	int Count = 0;

	for( cnt=0;cnt<TRANS_THREAD_SEND_MAX;cnt++) {
		if ( smTransSendThreads[cnt].smThreadSock ) {
			Count++;
		}
	}

	return Count;
}

//받기 쓰레드의 사용 상태 구함
int GetUseingRecvThreadCount()
{
	int cnt;
	int Count = 0;

	for( cnt=0;cnt<TRANS_THREAD_RECV_MAX;cnt++) {
		if ( smTransRecvThreads[cnt].smThreadSock ) {
			Count++;
		}
	}

	return Count;
}


//큐에 대기중인 소켓을 찾아 제거한다
int ClearTransQueSocket( smWINSOCK *lpsmsock )
{
	int cnt;
	int mCnt;

	//크리티칼 섹션 선언
	EnterCriticalSection( &cTransSection );

	for(cnt=smWaitRecvQuePop;cnt<smWaitRecvQuePush;cnt++) {
		mCnt = cnt & TRANS_WAIT_QUE_MASK;

		if ( smWaitRecvSocketQue[mCnt]==lpsmsock ) {
			smWaitRecvSocketQue[mCnt]=0;
		}
	}

	for(cnt=smWaitSendQuePop;cnt<smWaitSendQuePush;cnt++) {
		mCnt = cnt & TRANS_WAIT_QUE_MASK;

		if ( smWaitSendSocketQue[mCnt]==lpsmsock ) {
			smWaitSendSocketQue[mCnt]=0;
		}
	}

	//사용중인 라우트 있으면 닫음 ( 받기 )
	if ( lpsmsock->lpsmRecvRoute ) {

		//유저의 것과 일치 하는지 확인
		if ( lpsmsock->lpsmRecvRoute->smThreadSock && 
			lpsmsock->lpsmRecvRoute->smThreadSock->smMySock==lpsmsock ) {

			lpsmsock->lpsmRecvRoute->TimeOut = TRUE;
			ResumeThread( lpsmsock->lpsmRecvRoute->hThread );
		}
		else {
			lpsmsock->lpsmRecvRoute = 0;
			smTransClearErrorCnt++;					//통신라우트의 불일치 확인
		}
	}

	//사용중인 라우트 있으면 닫음 ( 보내기 )
	if ( lpsmsock->lpsmSendRoute ) {

		//유저의 것과 일치 하는지 확인
		if ( lpsmsock->lpsmSendRoute->smThreadSock && 
			lpsmsock->lpsmSendRoute->smThreadSock->smMySock==lpsmsock ) {

			lpsmsock->lpsmSendRoute->TimeOut = TRUE;
			ResumeThread( lpsmsock->lpsmSendRoute->hThread );
		}
		else {
			lpsmsock->lpsmSendRoute = 0;
			smTransClearErrorCnt++;					//통신라우트의 불일치 확인
		}
	}

/*
	if ( lpsmsock->lpsmRecvRoute ) {
		lpsmsock->lpsmRecvRoute->TimeOut = TRUE;
		ResumeThread( lpsmsock->lpsmRecvRoute->hThread );
	}

	if ( lpsmsock->lpsmSendRoute ) {
		lpsmsock->lpsmSendRoute->TimeOut = TRUE;
		ResumeThread( lpsmsock->lpsmSendRoute->hThread );
	}
*/
	//크리티칼 섹션 해제
	LeaveCriticalSection( &cTransSection );

	return TRUE;
}

//10초
#define TRANS_LOST_TIME		15000

//이상 있는 쓰레드를 찾아 처리 한다
int CheckLostTransThread()
{
	int cnt;
	DWORD dwTime;
	DWORD dwAcessTime;
	smWINSOCK *lpLostSock[TRANS_THREAD_SEND_MAX+TRANS_THREAD_RECV_MAX];
	int LostCount = 0;

	//크리티칼 섹션 선언
	EnterCriticalSection( &cTransSection );

	dwTime = GetCurrentTime();

	for( cnt=0;cnt<TRANS_THREAD_RECV_MAX;cnt++) {
		if ( smTransRecvThreads[cnt].smThreadSock ) {

			if ( smTransRecvThreads[cnt].TimeOut && smTransRecvThreads[cnt].Sleep ) {
				ResumeThread( smTransRecvThreads[cnt].hThread );
			}

			dwAcessTime = smTransRecvThreads[cnt].smThreadSock->dwAcessTime;
			if ( dwAcessTime && (dwAcessTime+TRANS_LOST_TIME)<dwTime ) {
				//억세스 타임 아웃
				//smTransRecvThreads[cnt].TimeOut = TRUE;
				//ResumeThread( smTransRecvThreads[cnt].hThread );

				if ( smTransRecvThreads[cnt].smThreadSock->smMySock ) {
					//smTransRecvThreads[cnt].smThreadSock->smMySock->CloseSocket();
					lpLostSock[LostCount++] = smTransRecvThreads[cnt].smThreadSock->smMySock;
					//break;
				}
			}
			else {
				//슬립 시간이 30 초 이상이면서 라우트가 일치 하지 않는 쓰레드 초기화 시킴
				if ( smTransRecvThreads[cnt].Sleep && 
					smTransRecvThreads[cnt].smThreadSock->smMySock &&
					(&smTransRecvThreads[cnt])!=smTransRecvThreads[cnt].smThreadSock->smMySock->lpsmRecvRoute ) {
					if ( dwTime>(smTransRecvThreads[cnt].dwSleepTime+(TRANS_LOST_TIME*2) ) ) {

						smTransRecvThreads[cnt].TimeOut = TRUE;
						ResumeThread( smTransRecvThreads[cnt].hThread );
						//smTransRecvThreads[cnt].smThreadSock = 0;
					}
				}
			}
		}
	}

	for( cnt=0;cnt<TRANS_THREAD_SEND_MAX;cnt++) {
		if ( smTransSendThreads[cnt].smThreadSock ) {

			if ( smTransSendThreads[cnt].TimeOut && smTransSendThreads[cnt].Sleep ) {
				ResumeThread( smTransSendThreads[cnt].hThread );
			}

			dwAcessTime = smTransSendThreads[cnt].smThreadSock->dwAcessTime;
			if ( dwAcessTime && (dwAcessTime+TRANS_LOST_TIME)<dwTime ) {
				//억세스 타임 아웃
				//smTransSendThreads[cnt].TimeOut = TRUE;
				//ResumeThread( smTransSendThreads[cnt].hThread );

				if ( smTransSendThreads[cnt].smThreadSock->smMySock ) {
					lpLostSock[LostCount++] = smTransSendThreads[cnt].smThreadSock->smMySock;
					//break;
				}

			}
			else {
				//슬립 시간이 30 초 이상이면서 라우트가 일치 하지 않는 쓰레드 초기화 시킴
				if ( smTransSendThreads[cnt].Sleep && 
					smTransSendThreads[cnt].smThreadSock->smMySock &&
					(&smTransSendThreads[cnt])!=smTransSendThreads[cnt].smThreadSock->smMySock->lpsmSendRoute ) {
					if ( dwTime>(smTransSendThreads[cnt].dwSleepTime+(TRANS_LOST_TIME*2) ) ) {

						smTransSendThreads[cnt].TimeOut = TRUE;
						ResumeThread( smTransSendThreads[cnt].hThread );
						//smTransSendThreads[cnt].smThreadSock = 0;
					}
				}
			}
		}
	}
	//크리티칼 섹션 해제
	LeaveCriticalSection( &cTransSection );

	for( cnt=0;cnt<LostCount;cnt++ ) {
		if ( lpLostSock[cnt]->sock )
			lpLostSock[cnt]->CloseSocket();
	}
/*
	//문제 있는 소켓 닫기
	if ( lpLostSock ) {
		cnt = lpLostSock->CloseSocket();
		//if ( cnt ) DisconnectPlayer( lpLostSock );
	}
*/
	return LostCount;
}


//사용하지 않는 통신 쓰레드를 찾는다
smTHREAD_ROUTE *FindTransThread( int SRMode )	// TRUE->Send FALSE->Recv
{
	int cnt;

	if ( SRMode ) {
		for( cnt=0;cnt<TRANS_THREAD_SEND_MAX;cnt++) {
			if ( !smTransSendThreads[cnt].smThreadSock && smTransSendThreads[cnt].Sleep ) {
				return &smTransSendThreads[cnt];
			}
		}
	}
	else {
		for( cnt=0;cnt<TRANS_THREAD_RECV_MAX;cnt++) {
			if( !smTransRecvThreads[cnt].smThreadSock && smTransRecvThreads[cnt].Sleep ) {
				return &smTransRecvThreads[cnt];
			}
		}
	}

	return FALSE;
}

smTHREAD_ROUTE *RecvThreadRoute( smWINSOCK *lpsmsock )
{
	smTHREAD_ROUTE *lpRoute=0;

	//크리티칼 섹션 선언
	EnterCriticalSection( &cTransSection );

	if ( lpsmsock->lpsmRecvRoute ) {
		if ( lpsmsock->lpsmRecvRoute->Sleep )
			ResumeThread( lpsmsock->lpsmRecvRoute->hThread );
		else {
			lpsmsock->WaitReadMessage = 1;
			ResumeThread( lpsmsock->lpsmRecvRoute->hThread );
		}
			//lpsmsock->WaitReadMessage++;
	}
	else {

		lpRoute = FindTransThread( FALSE );

		if ( lpRoute ) {
			lpRoute->smThreadSock = &lpsmsock->smRecvState;
			lpRoute->TimeOut = FALSE;
			lpsmsock->lpsmRecvRoute = lpRoute;

			ResumeThread( lpRoute->hThread );
		}
		else {
			PushTransRecvSocket( lpsmsock );
		}
	}

	//크리티칼 섹션 해제
	LeaveCriticalSection( &cTransSection );

	return lpRoute;
}

smTHREAD_ROUTE *SendThreadRoute( smWINSOCK *lpsmsock )
{
	smTHREAD_ROUTE *lpRoute=0;

	//크리티칼 섹션 선언
	EnterCriticalSection( &cTransSection );

	lpRoute = FindTransThread( TRUE );
	if ( lpRoute ) {
		lpRoute->smThreadSock = &lpsmsock->smSendState;
		lpRoute->TimeOut = FALSE;
		lpsmsock->lpsmSendRoute = lpRoute;

		ResumeThread( lpRoute->hThread );
	}
	else {
		PushTransSendSocket( lpsmsock );
	}

	//크리티칼 섹션 해제
	LeaveCriticalSection( &cTransSection );

	return lpRoute;
}



//dwIP를 string 으로 변환
char *ConvStrIP( DWORD dwIP )
{
	in_addr  ia;

	ia.S_un.S_addr = dwIP;
	lstrcpy( szIP , inet_ntoa (ia) );

	return szIP;
}

//자신의 IP주소 구함
char *smGetMyIp()
{

	PHOSTENT phe;
	in_addr  ia;

	// 자신 주소 구함
	phe = gethostbyname("");
	if ( phe==NULL ) {
		//DNS 이상
		return FALSE;
	}
	ia.S_un.S_addr = ((unsigned long *)phe->h_addr)[0];
	lstrcpy( szIP , inet_ntoa (ia) );

	return szIP;
}

//자신의 IP주소 구함
DWORD smGetMyIp2()
{

	PHOSTENT phe;

	// 자신 주소 구함
	phe = gethostbyname("");
	if ( phe==NULL ) {
		//DNS 이상
		return FALSE;
	}

	return ((DWORD *)phe->h_addr)[0];
}







smWINSOCK::smWINSOCK()
{

}
smWINSOCK::~smWINSOCK()
{

}

//새 플레이어와 연결
extern int ConnectPlayer( smWINSOCK *lpsmSock );
//플레이어 연결 끈어짐
extern int DisconnectPlayer( smWINSOCK *lpsmSock );

//소켓을 억셉트 하여 소켓에 연결 시킴 ( 윈도 메세지로 처리됨 )
int smWINSOCK::mesAccept( WPARAM wParam , LPARAM lParam )
{
	
	HWND hWnd = hMsgWnd;

	if (WSAGETSELECTERROR( lParam ) == 0) {   /* Success */
	    /*
	    *   Accept the incoming connection.
	    */
	    acc_sin_len = sizeof( acc_sin );
	    sock = accept( (SOCKET)wParam,(struct sockaddr FAR *) &acc_sin, (int FAR *) &acc_sin_len );

	    if (sock < 0) {
			//sprintf(szBuff, "%d is the error", WSAGetLastError());
			//WinPrint( hWnd , szBuff );
			sock = 0;
	        return -1;
		}
         
		/*
		*   Send main window a WSA_READ when either data is pending on
		*   the socket (FD_READ) or the connection is closed (FD_CLOSE)
		*/
		if ((status = WSAAsyncSelect( sock, hWnd, WSA_READ, FD_READ | FD_WRITE | FD_CLOSE )) > 0) {
			//wsprintf(szBuff, "%d (0x%x)", status, status);
		    //MessageBox( hWnd, "Error on WSAAsyncSelect()", szBuff, MB_OK);
		    closesocket( sock );
			sock = 0;
			return -3;
		}

		lstrcpy( szIPAddr , inet_ntoa(acc_sin.sin_addr) );
		ConnectDirection = FALSE;			//연결 받음

		PacketSendQueCnt = 0;		//보내기 패킷 큐 카운터 초기화
		WaitReadMessage = 0;
		PostReadMessage = 0;
		RecvPacketCount = 0;		//받은 패킷 카운터
		SendPacketCount = 0;		//보낸 패킷 카운터
		RecvPacketErrorCount = 0;	//받은 패킷 오류
		SendPacketErrorCount = 0;	//보낸 패킷 에러
		RecvPopCount = 0;			//강제로 받기 시도 카운트
		RecvPopErrorCount = 0;		//강제로 받기 시도 실패 카운트
		dwLastRecvTime = GetCurrentTime();				//마지막 수신 시간
		dwDeadLockTime = dwLastRecvTime;

		EncPacketSendCount = 0x0010;		//보낸 암호패킷 카운터
		EncPacketRecvCount = 0;				//받은 암호 패킷 카운터
		EncRecvPacketCnt = 0;				//받은 패킷 기억 카운터

		dwEncPacketCode = smTRANSCODE_ENCODE_PACKET;

#ifdef _PACKET_PASS_XOR
		bEncXor = _PACKET_PASS_XOR;
		bDecXor = _PACKET_PASS_XOR;
#else
		bEncXor = 0;
		bDecXor = 0;
#endif

		CreateTransThread( TRANSMODE_DUAL );

		ConnectPlayer( MyPoint );

		int sockBufferSize;
		//int sockOptSize;

		sockBufferSize = 65536;

		setsockopt( sock , SOL_SOCKET , SO_RCVBUF , (char *)&sockBufferSize , sizeof(sockBufferSize));
		setsockopt( sock , SOL_SOCKET , SO_SNDBUF , (char *)&sockBufferSize , sizeof(sockBufferSize));

		return TRUE;

	}
	else {
         //MessageBox(hWnd, "accept()", "Error occured!", MB_OK);
         /*
         *   Cancel any further notifications.
         */
         //WSAAsyncSelect( sock, hWnd, 0, 0);
         //SetWindowText( hWnd, "Async Listen call canceled");
		 return -2;
    }
	
	return TRUE;
}

//소켓을 재연결 시킨다
int smWINSOCK::ReconnectSock()
{

	SOCKADDR_IN dest_sin;
	SOCKET	msock;
	int status;

	if ( !sock ) return FALSE;

	closesocket( sock );
	sock = 0;
	Sleep(100);

	msock = socket( AF_INET, SOCK_STREAM, 0);
	if (msock == INVALID_SOCKET) {
        return FALSE;
	}

    dest_sin.sin_family = AF_INET ;
	dest_sin.sin_addr.S_un.S_addr = inet_addr ( szIPAddr );
	dest_sin.sin_port = htons((u_short) dwPort ) ;

	if (connect( msock, (PSOCKADDR) &dest_sin, sizeof( dest_sin)) ==SOCKET_ERROR) {
		CloseSocket();
		return FALSE;
	}

	ConnectDirection = TRUE;			//연결함

	status = WSAAsyncSelect( msock, hMsgWnd, WSA_READ, FD_READ | FD_WRITE | FD_CLOSE ); //) > 0) {

	sock = msock;
	acc_sin.sin_addr.S_un.S_addr = inet_addr ( szIPAddr );

	ClearPacketSendQue( this );
	PacketSendQueCnt = 0;				//보내기 패킷 큐 카운터 초기화
	WaitReadMessage = 0;
	PostReadMessage = 0;
	RecvPacketCount = 0;		//받은 패킷 카운터
	SendPacketCount = 0;		//보낸 패킷 카운터
	RecvPacketErrorCount = 0;	//받은 패킷 오류
	SendPacketErrorCount = 0;	//보낸 패킷 에러
	RecvPopCount = 0;			//강제로 받기 시도 카운트
	RecvPopErrorCount = 0;		//강제로 받기 시도 실패 카운트
	dwLastRecvTime = GetCurrentTime();			//마지막 수신 시간
	dwDeadLockTime = dwLastRecvTime;

	EncPacketSendCount = 0x0010;		//보낸 암호패킷 카운터
	EncPacketRecvCount = 0;				//받은 암호 패킷 카운터
	EncRecvPacketCnt = 0;				//받은 패킷 기억 카운터
	
	return TRUE;
}



//소켓을 연결 시킨다
int smWINSOCK::ConnectSock( char *szIP , WORD port )
{

	SOCKADDR_IN dest_sin;
	SOCKET	msock;
	int status;


	msock = socket( AF_INET, SOCK_STREAM, 0);
	if (msock == INVALID_SOCKET) {
        return FALSE;
	}


    dest_sin.sin_family = AF_INET ;
	dest_sin.sin_addr.S_un.S_addr = inet_addr ( szIP );
	dest_sin.sin_port = htons((u_short) port ) ;


	if (connect( msock, (PSOCKADDR) &dest_sin, sizeof( dest_sin)) ==SOCKET_ERROR) {
		closesocket( msock );
		return FALSE;
	}


	lstrcpy( szIPAddr , szIP );
	ConnectDirection = TRUE;			//연결함
	dwPort = port;

	status = WSAAsyncSelect( msock, hMsgWnd, WSA_READ, FD_READ | FD_WRITE | FD_CLOSE ); //) > 0) {
//	status = WSAAsyncSelect( msock, hwnd, WSA_READ, FD_READ | FD_CLOSE ); //) > 0) {

	sock = msock;
	acc_sin.sin_addr.S_un.S_addr = inet_addr ( szIP );

	PacketSendQueCnt = 0;				//보내기 패킷 큐 카운터 초기화
	WaitReadMessage = 0;
	PostReadMessage = 0;
	RecvPacketCount = 0;		//받은 패킷 카운터
	SendPacketCount = 0;		//보낸 패킷 카운터
	RecvPacketErrorCount = 0;	//받은 패킷 오류
	SendPacketErrorCount = 0;	//보낸 패킷 에러
	RecvPopCount = 0;			//강제로 받기 시도 카운트
	RecvPopErrorCount = 0;		//강제로 받기 시도 실패 카운트
	dwLastRecvTime = GetCurrentTime();			//마지막 수신 시간
	dwDeadLockTime = dwLastRecvTime;

	EncPacketSendCount = 0x0010;		//보낸 암호패킷 카운터
	EncPacketRecvCount = 0;				//받은 암호 패킷 카운터
	EncRecvPacketCnt = 0;				//받은 패킷 기억 카운터


	dwEncPacketCode = smTRANSCODE_ENCODE_PACKET;

#ifdef _PACKET_PASS_XOR
	bEncXor = _PACKET_PASS_XOR;
	bDecXor = _PACKET_PASS_XOR;
#else
	bEncXor = 0;
	bDecXor = 0;
#endif

	CreateTransThread( TRANSMODE_DUAL );

	int sockBufferSizeRecv;
	int sockBufferSizeSend;
	//int sockOptSize;
#ifdef _W_SERVER
	sockBufferSizeRecv = 65536;
	sockBufferSizeSend = 65536;
#else
	sockBufferSizeRecv = 16384;
	sockBufferSizeSend = 32768;
#endif
	setsockopt( sock , SOL_SOCKET , SO_RCVBUF , (char *)&sockBufferSizeRecv , sizeof(sockBufferSizeRecv));
	setsockopt( sock , SOL_SOCKET , SO_SNDBUF , (char *)&sockBufferSizeSend , sizeof(sockBufferSizeSend));

/*
	getsockopt( sock , SOL_SOCKET , SO_RCVBUF , (char *)&sockBufferSize , &sockBuffLen );
	getsockopt( sock , SOL_SOCKET , SO_SNDBUF , (char *)&sockBufferSize , &sockBuffLen );
*/

/*
	sockOptSize = sizeof( sockBufferSize );
	sockOptSize = getsockopt( sock , SOL_SOCKET , SO_SNDBUF , (char *)&sockBufferSize , &sockOptSize );
*/

	return TRUE;
}

int smWINSOCK::CloseSocket()
{
	DWORD	dwExitCode;

	DisconnectPlayer( MyPoint );


//		if ( smSendState.Sleep==TRUE && smSendState.Sucess==FALSE )

	if ( !smTransThreadMode ) {
		smSendState.dwExitThread = TRUE;
		ResumeThread( hSendThread );

		smRecvState.dwExitThread = TRUE;
		ResumeThread( hRecvThread );

		//hSendThread 종료 대기
		WaitForSingleObject( hSendThread , 500 );
		GetExitCodeThread( hSendThread , &dwExitCode );

		if ( dwExitCode==STILL_ACTIVE ) {
			smSendState.dwExitThread = TRUE;
			ResumeThread( hSendThread );

			WaitForSingleObject( hSendThread , 500 );
			GetExitCodeThread( hSendThread , &dwExitCode );

			if ( dwExitCode==STILL_ACTIVE ) {
				TerminateThread( hSendThread , 1 );
			}
		}

		//hRecvThread 종료 대기
		WaitForSingleObject( hRecvThread , 500 );
		GetExitCodeThread( hRecvThread , &dwExitCode );

		if ( dwExitCode==STILL_ACTIVE ) {
			smRecvState.dwExitThread = TRUE;
			ResumeThread( hRecvThread );

			WaitForSingleObject( hRecvThread , 500 );
			GetExitCodeThread( hRecvThread , &dwExitCode );

			if ( dwExitCode==STILL_ACTIVE ) {
				TerminateThread( hRecvThread , 1 );
			}
		}
	}
	else {
		ClearTransQueSocket( MyPoint );
	}
 
	ClearPacketSendQue( this );

	closesocket( sock );
	sock = 0;
	smRecvState.Sock = 0;
	smSendState.Sock = 0;


	//TerminateThread( hSendThread , 1 );
	//TerminateThread( hRecvThread , 1 );

	return TRUE;
}

int smWINSOCK::mesReceve( LPARAM lParam )
{
	HWND	hWnd = hMsgWnd;

	switch ( WSAGETSELECTEVENT(lParam) )
	{

	case FD_READ:
		smRecvState.NewData ++;

		if ( !smTransThreadMode ) { 
/*
			if ( !smRecvState.Sleep ) {
				WaitReadMessage++;
				PostReadMessage++;
			}
			else {
				dwRecvSusCount = ResumeThread( hRecvThread );
			}
*/
			if ( !smTransTurbRcvMode ) WaitReadMessage++;
			PostReadMessage++;
			dwRecvSusCount = ResumeThread( hRecvThread );

		}
		else {
			RecvThreadRoute( MyPoint );
/*
			if ( lpsmSendRoute ) 
				ResumeThread( lpsmSendRoute->hThread );
			else {
				if ( !RecvThreadRoute( MyPoint ) ) {
					//쓰레드 로스 처리 추후 필요 예상
				}
			}
*/
		}

		break;

	case FD_WRITE:
		if ( smTransThreadMode && lpsmSendRoute && smSendState.Sucess==FALSE ) {
			ResumeThread( lpsmSendRoute->hThread );
		}

		if ( !smTransThreadMode && smSendState.Sleep==TRUE && smSendState.Sucess==FALSE ) {
			if ( hSendThread ) ResumeThread( hSendThread );
		}

		smSendState.SendReady = TRUE;
		break;

	case FD_CLOSE:
		CloseSocket();
		break;
	}

	return TRUE;
}

extern char *szNetLogFile;
extern FILE *fpNetLog;

int smWINSOCK::Send( char *SendData , int len , int ImpMode )
{

//	if ( smSendState.SendReady==TRUE && smSendState.Sucess==TRUE ) {

/*
	if ( fpNetLog ) {
		char szLogMsg[128];
		wsprintf( szLogMsg , "Send %x %d\r\n" , ((DWORD *)SendData)[1] , len );
		fwrite( szLogMsg , lstrlen( szLogMsg ) , 1, fpNetLog);
	}
*/

	//크리티칼 섹션 선언
	EnterCriticalSection( &cSendingSection );

	if ( smSendState.Sucess==TRUE ) {

		memcpy( smSendState.Buff , SendData , len );
		smSendState.BuffCnt = len;
		smSendState.Sucess = FALSE;
		smSendState.NewData = TRUE;
		//크리티칼 섹션 해제
		LeaveCriticalSection( &cSendingSection );


		if ( smTransThreadMode ) {
			SendThreadRoute( MyPoint );
			/*
			if ( !SendThreadRoute( MyPoint ) ) {
				return FALSE;
			}
			*/
		}
		else {
			ResumeThread( hSendThread );
		}

		return TRUE;
	}
	else {
		if ( smTransThreadMode ) {
			if ( lpsmSendRoute && lpsmSendRoute->Sleep ) {
				ResumeThread( lpsmSendRoute->hThread );
			}
		}
		else {
			if ( smSendState.Sleep ) {
				ResumeThread( hSendThread );
			}
		}
	}

	//크리티칼 섹션 해제
	LeaveCriticalSection( &cSendingSection );

	if ( ImpMode ) {
		PushPacketSendQue( this , SendData , len );
		return TRUE;
	}

	return FALSE;
}


//패킷을 암호화 하여 보냄
int smWINSOCK::Send2( char *SendData , int len , int ImpMode )
{

#ifdef	__W_SERVER
	return Send( SendData , len , ImpMode );
#else

	char EncData[smSOCKBUFF_SIZE];
	DWORD dwKey;

	if ( len<(smSOCKBUFF_SIZE-16) ) {

		dwKey = ((acc_sin.sin_addr.S_un.S_addr)<<8)&0xFFFF;
		dwKey |= rand()%255;

		if ( fnEncodePacket ) {
			fnEncodePacket( dwKey , EncPacketSendCount , (BYTE *)SendData , (BYTE *)EncData );
		}
		else {
			dwKey |= dwEncPacketCode;
			//패킷을 암호화 한다
			EncodePacket2( dwKey , EncPacketSendCount , (BYTE *)SendData , (BYTE *)EncData , bEncXor );
		}

		EncPacketSendCount++;

		//Send( EncData , len+8 , ImpMode );			//테스트 하기위함
		return Send( EncData , len+8 , ImpMode );
	}
	else {
		return Send( SendData , len , ImpMode );
	}
	return FALSE;
#endif

}

//패킷을 암호화 하여 보냄
int smWINSOCK::Send3( char *SendData , int len , int ImpMode )
{

#ifdef	__W_SERVER
	return Send( SendData , len , ImpMode );
#else

	char EncData[smSOCKBUFF_SIZE];
	DWORD dwKey;

	if ( len<(smSOCKBUFF_SIZE-16) ) {

		dwKey = ((acc_sin.sin_addr.S_un.S_addr)<<8)&0xFFFF;
		dwKey |= rand()%255;
		dwKey |= smTRANSCODE_ENCODE_PACKET;
		//패킷을 암호화 한다

		EncodePacket( dwKey , EncPacketSendCount , (BYTE *)SendData , (BYTE *)EncData );
		EncPacketSendCount++;

		//Send( EncData , len+8 , ImpMode );			//테스트 하기위함
		return Send( EncData , len+8 , ImpMode );
	}
	else {
		return Send( SendData , len , ImpMode );
	}
	return FALSE;
#endif

}



//최근 패킷과 겹쳐지는지 확인
int	smWINSOCK::CheckEncRecvPacket( int RecvCount )
{
	int cnt;
	int mCnt;

	for( cnt=EncRecvPacketCnt-ENC_PACKET_REC_MAX;cnt<EncRecvPacketCnt;cnt++ ) {
		if ( cnt>=0 ) {
			mCnt = cnt&ENC_PACKET_REC_MASK;
			if ( EncRecvPackets[mCnt]==RecvCount ) {
				return TRUE;
			}
		}
	}

	return FALSE;
}

//최근 패킷번호 기록
int	smWINSOCK::PushEncRecvPacket( int RecvCount )
{
	int mCnt;

	mCnt = EncRecvPacketCnt&ENC_PACKET_REC_MASK;
	EncRecvPackets[mCnt]=RecvCount;
	EncRecvPacketCnt++;

	if ( EncPacketRecvCount<RecvCount ) 
		EncPacketRecvCount = RecvCount;

	return EncRecvPacketCnt;
}


struct smPLAYDATA {
	int	size;
	int code;

	int x,y,z;
	int ax,ay,az;

	int	frame;
};



DWORD WINAPI smWinsockRecvThreadProc( void *pInfo )
{
	smTHREADSOCK *SockInfo;
	int len;
	char szBuff[smSOCKBUFF_SIZE];
	char *Buff;
	int RecvLen;
	int BuffCnt;
	int BuffSize;
	int RecvCode;
	HANDLE	hThread;
	int	BuffOver;
	DWORD dwRcvSize;

	SockInfo = (smTHREADSOCK *) pInfo;
	RecvLen = 0;
	BuffCnt = 0;
	BuffOver = 0;
	BuffSize = smSOCKBUFF_SIZE;

	SockInfo->Sucess = FALSE;
	SockInfo->dwExitThread = FALSE;
	hThread = GetCurrentThread();

	while(1) {
			if ( SockInfo->dwExitThread ) {
				//쓰레드 종료
				ExitThread( TRUE );
				return TRUE;
			}

			if ( SockInfo->NewData==0 && !BuffOver ) {
				SockInfo->Sleep = TRUE;

				if ( smTransTurbRcvMode ) {
					Sleep(smTransTurbRcvMode);
				}
				else
					SuspendThread( hThread );

				SockInfo->Sleep = FALSE;
			}

			if ( SockInfo->dwExitThread ) {
				//쓰레드 종료
				ExitThread( TRUE );
				return TRUE;
			}

			dwRcvSize = 0;
			ioctlsocket( SockInfo->Sock , FIONREAD , &dwRcvSize );
			if ( dwRcvSize<=0 ) {
				SockInfo->NewData = 0;
				BuffOver = 0;
				SockInfo->smMySock->WaitReadMessage = 0;
				continue;
			}


			BuffOver = 0;

			if ( SockInfo->smMySock && SockInfo->smMySock->PostReadMessage>0 ) {
				SockInfo->smMySock->PostReadMessage--;
				BuffOver ++;
			}

			Buff = szBuff + BuffCnt;
			BuffSize = smSOCKBUFF_SIZE - BuffCnt;

			len = recv( SockInfo->Sock , Buff , BuffSize , 0 );

			if ( len>0 ) {
				if ( BuffSize<=len ) smRcvBuffOverCount ++;
				BuffOver ++;
			}

/*
			if ( (BuffSize<=len ) {
				smRcvBuffOverCount ++;
				BuffOver ++;
			}
*/

			if ( BuffCnt<8 ) RecvLen = smSOCKBUFF_SIZE-32;

			if ( BuffCnt<8 && (BuffCnt+len)>=8 ) {
				//RecvLen = 1000;
				RecvLen = (((int *)szBuff)[0]);
				RecvCode = (((int *)szBuff)[1]);
			}

			if ( len>0 ) {
				SockInfo->smMySock->RecvPacketCount++;		//받은 패킷 카운터

				BuffCnt += len;
				SockInfo->NewData--;

/*
				BuffSize = smSOCKBUFF_SIZE - BuffCnt;

				if ( BuffSize <= 0 )  {
					BuffSize = 0;
				}
*/
			}
			else {
				if ( SockInfo->smMySock->WaitReadMessage>0 )
					SockInfo->smMySock->RecvPopErrorCount++;		//강제 받기 시도 오류 기록
				else
					SockInfo->smMySock->RecvPacketErrorCount++;		//받기 시도 오류 기록

				if ( len==0 ) {
					//연결이 끊어진 걸로 추측
					SockInfo->smMySock->WaitReadMessage = -1;
				}
				else 
					SockInfo->smMySock->WaitReadMessage = 0;

				SockInfo->NewData = 0;

#ifdef _DEBUG_SOCKET
				if (smConfig.DebugMode) {
					char TempBuff[128];
					wsprintf(TempBuff, "Recv %d is the error", WSAGetLastError());
					//AddChatBuff(TempBuff);
				}
#endif

				continue;
			}

			while(1) {
				if ( BuffCnt>=RecvLen ) {
					memcpy( SockInfo->Buff , szBuff , RecvLen );
					SockInfo->Buff[ BuffCnt ] = 0;
					SockInfo->RecvLen = RecvLen;
					SockInfo->RecvCode = RecvCode;
					SockInfo->BuffCnt = BuffCnt;
					SockInfo->Sucess = TRUE;

					SockInfo->smMySock->dwLastRecvTime = GetCurrentTime();
					SendMessage( hMsgWnd , SWM_RECVSUCCESS , (WPARAM)SockInfo , 0 );

					if ( RecvLen<BuffCnt ) {
						BuffCnt -= RecvLen;
						memmove( szBuff , szBuff+RecvLen , BuffCnt );		// overlapping: memmove (memcpy's result is undefined)
						if ( BuffCnt>=8 ) {
							RecvLen = (((int *)szBuff)[0]);
							RecvCode = (((int *)szBuff)[1]);
							BuffSize = smSOCKBUFF_SIZE - BuffCnt;
						}
						else{
							RecvLen = 0;
							RecvCode = 0;
							break;
						}
					}
					else {
						RecvLen = 0;
						BuffCnt = 0;

						if ( SockInfo->smMySock->WaitReadMessage>0 ) {
							SockInfo->NewData = TRUE;
							SockInfo->smMySock->WaitReadMessage --;
							break;
						}
						break;
					}
				}
				else
					break;
			}

	}

	ExitThread( TRUE );
	return TRUE;
}

DWORD WINAPI smWinsockSendThreadProc( void *pInfo )
{

	smTHREADSOCK *SockInfo;
	char *szBuff;//[smSOCKBUFF_SIZE];
	int BuffCnt;
	int stats;
	int BuffLen;

	HANDLE	hThread;

	SockInfo = (smTHREADSOCK *) pInfo;
	hThread = GetCurrentThread();
	SockInfo->dwExitThread = FALSE;

	while(1) {
			if ( SockInfo->dwExitThread ) {
				//쓰레드 종료
				ExitThread( TRUE );
				return TRUE;
			}

			if ( SockInfo->NewData==FALSE ) {
				SockInfo->Sleep = TRUE;
				SuspendThread( hThread );
				SockInfo->Sleep = FALSE;
			}

			if ( SockInfo->dwExitThread ) {
				//쓰레드 종료
				ExitThread( TRUE );
				return TRUE;
			}

			if ( SockInfo->NewData ) {
				BuffLen = SockInfo->BuffCnt;
				BuffCnt = 0;
				SockInfo->NewData = FALSE;
			}

			szBuff = SockInfo->Buff + BuffCnt;

			stats = send( SockInfo->Sock , szBuff , BuffLen , 0 );

			if ( stats != SOCKET_ERROR ) {
				SockInfo->smMySock->SendPacketCount++;		//보낸 패킷 카운터

				if ( stats!=BuffLen ) {
					stats = stats;
				}

				BuffCnt +=stats;
				BuffLen -=stats;
				if ( BuffLen <= 0 ) {
					SockInfo->Sucess = TRUE;
					//쓰레드에서 보내기 성공
					SucessPacketSending( SockInfo );
//					SendMessage( hMsgWnd , SWM_SENDSUCCESS , (WPARAM)SockInfo , 0 );
				}
			}
			else {
				SockInfo->smMySock->SendPacketErrorCount++;		//보낸 패킷 카운터
#ifdef _DEBUG_SOCKET
				if (smConfig.DebugMode) {
					char TempBuff[128];
					wsprintf(TempBuff, "Send %d is the error", WSAGetLastError());
					AddChatBuff(TempBuff);
				}
#endif
			}
	}

	ExitThread( TRUE );
	return TRUE;
}


//ID 인증및 설정
int smWINSOCK::CreateTransThread( int Mode )
{
	DWORD dwRecvMsgId;
	DWORD dwSendMsgId;

	hRecvThread = 0;
	hSendThread = 0;

	smRecvState.Sock = sock;
	smSendState.Sock = sock;

	smSendState.Sucess = TRUE;
	smSendState.SendReady = TRUE;
	smSendState.Sleep = FALSE;


	if ( !smTransThreadMode ) {

		if ( Mode&TRANSMODE_RECV ) {
			hRecvThread = CreateThread( NULL , 0, smWinsockRecvThreadProc , &smRecvState , 0, &dwRecvMsgId );
			SetThreadPriority( hRecvThread , THREAD_PRIORITY_ABOVE_NORMAL );
		}
		if ( Mode&TRANSMODE_SEND ) {
			hSendThread = CreateThread( NULL , 0, smWinsockSendThreadProc , &smSendState , 0, &dwSendMsgId );
			SetThreadPriority( hSendThread , THREAD_PRIORITY_ABOVE_NORMAL );
		}

	}

	ExtData1 = 0;
	ExtData2 = 0;
	ExtData3 = 0;
	ExtData4 = 0;

	return TRUE;
}







//게임 소켓 초기화
void InitGameSocket( int ThreadMode )
{
	int cnt;
	WSADATA WSAData;
	char szTemp[80];
	int status;

	//메세지 받는 윈도 설정
	hMsgWnd = hwnd;

	if ((status = WSAStartup(MAKEWORD(1,1), &WSAData)) != 0) {
		sprintf(szTemp, "%d is the err", status);
		MessageBox( hMsgWnd, szTemp, "Error", MB_OK);
	}

	for(cnt=0;cnt<CONNECTMAX;cnt++) {
		smWSock[cnt].sock = 0;
		smWSock[cnt].MyPoint = &smWSock[cnt];
		smWSock[cnt].MyNum = cnt;
		smWSock[cnt].smRecvState.smMySock = &smWSock[cnt];
		smWSock[cnt].smSendState.smMySock = &smWSock[cnt];
		smWSock[cnt].lpsmRecvRoute = 0;
		smWSock[cnt].lpsmSendRoute = 0;
	}

	if ( ThreadMode ) {
		smTransThreadMode = ThreadMode;
		InitTransThread();
	}
	else {
		smTransThreadMode = 0;
	}

	//크리티칼 섹션 초기화
	InitializeCriticalSection( &cSocketSection );
	//크리티칼 섹션 초기화
	InitializeCriticalSection( &cSendingSection );

	//보내기 패킷 큐를 초기화
	InitPacketSendQue();

}


//소켓을 연결 시킨다
smWINSOCK *smConnectSock( char *szIP , WORD port )
{
	int cnt;
	smWINSOCK *lpsmSock = 0;


	//크리티칼 섹션 선언
	EnterCriticalSection( &cSocketSection );

	if ( smTransThreadMode ) {
		for(cnt=0;cnt<CONNECTMAX;cnt++) {
			if ( smWSock[cnt].sock==0 && !smWSock[cnt].lpsmRecvRoute && !smWSock[cnt].lpsmSendRoute) {
				if ( smWSock[cnt].ConnectSock( szIP, port )!=FALSE )
					lpsmSock = &smWSock[cnt];
				break;
			}
		}
	}
	else {
		for(cnt=0;cnt<CONNECTMAX;cnt++) {
			if ( smWSock[cnt].sock==0 ) {
				if ( smWSock[cnt].ConnectSock( szIP, port )!=FALSE )
					lpsmSock = &smWSock[cnt];
				break;
			}
		}
	}

	//크리티칼 섹션 해제
	LeaveCriticalSection( &cSocketSection );

	return lpsmSock;
}

//소켓을 연결 시킨다 ( 크리티컬 없음 )
smWINSOCK *smConnectSock3( char *szIP , WORD port )
{
	int cnt;
	smWINSOCK *lpsmSock = 0;


	if ( smTransThreadMode ) {
		for(cnt=CONNECTMAX-1;cnt>=0;cnt--) {
			if ( smWSock[cnt].sock==0 && !smWSock[cnt].lpsmRecvRoute && !smWSock[cnt].lpsmSendRoute) {
				if ( smWSock[cnt].ConnectSock( szIP, port )!=FALSE )
					lpsmSock = &smWSock[cnt];
				break;
			}
		}
	}
	else {
		for(cnt=CONNECTMAX-1;cnt>=0;cnt--) {
			if ( smWSock[cnt].sock==0 ) {
				if ( smWSock[cnt].ConnectSock( szIP, port )!=FALSE )
					lpsmSock = &smWSock[cnt];
				break;
			}
		}
	}

	return lpsmSock;
}


static int MsgRetryCount = 0;

int smCheckWaitMessage( smWINSOCK *lpsmSock )
{

	//크리티칼 섹션 선언
	EnterCriticalSection( &cSocketSection );

	if ( lpsmSock && lpsmSock->WaitReadMessage>0 && lpsmSock->smRecvState.Sleep ) {
		lpsmSock->WaitReadMessage--;
		lpsmSock->mesReceve( FD_READ );
		lpsmSock->RecvPopCount ++;
	}

	//크리티칼 섹션 해제
	LeaveCriticalSection( &cSocketSection );

	return TRUE;
}

//대지중인 메세지가 있는지 확인하여 처리한다
int smCheckWaitMessage()
{
	int cnt;
	int MsgCnt;


	MsgCnt = 0;

	//크리티칼 섹션 선언
	EnterCriticalSection( &cSocketSection );

	if ( smTransThreadMode ) {
		for(cnt=0;cnt<CONNECTMAX;cnt++) {
			if ( smWSock[cnt].sock ) {
				if ( smWSock[cnt].WaitReadMessage>0 ) {
					//if ( !smWSock[cnt].lpsmRecvRoute ) {
						//PostMessage( hwnd , WSA_READ , smWSock[cnt].sock , 1 );
						smWSock[cnt].WaitReadMessage--;
						smWSock[cnt].mesReceve( 1 );
						smWSock[cnt].RecvPopCount ++;

						MsgRetryCount++;
						MsgCnt++;
					//}
					//else {
					//	if ( smWSock[cnt].lpsmRecvRoute->Sleep )
					//		ResumeThread( smWSock[cnt].lpsmRecvRoute->hThread );
					//}
				}

				//보내기 큐에 걸려있는 데이타 전송
				if ( smWSock[cnt].PacketSendQueCnt && !smWSock[cnt].lpsmSendRoute ) {
					PopSendPacketQue( &smWSock[cnt] );
				}

				//보내기 오류 재시도
				if ( !smWSock[cnt].smSendState.Sucess ) {
					if ( smWSock[cnt].lpsmSendRoute && smWSock[cnt].lpsmSendRoute->Sleep ) {
						ResumeThread( smWSock[cnt].lpsmSendRoute->hThread );
					}
				}

			}
		}
	}
	else {
		for(cnt=0;cnt<CONNECTMAX;cnt++) {
			if ( smWSock[cnt].sock ) {
				if ( smWSock[cnt].WaitReadMessage>0 && smWSock[cnt].smRecvState.Sleep ) {
					//PostMessage( hwnd , WSA_READ , smWSock[cnt].sock , 1 );
					smWSock[cnt].WaitReadMessage--;
					smWSock[cnt].mesReceve( 1 );
					smWSock[cnt].RecvPopCount ++;

#ifdef _DEBUG_SOCKET
					char TempBuff[128];
					if ( smConfig.DebugMode ) {
						wsprintf(TempBuff, "%s Recv TimeOut Message", smWSock[cnt].szIPAddr);
						AddChatBuff(TempBuff);
					}
#endif
					MsgRetryCount++;
					MsgCnt++;
				}

				//보내기 큐에 걸려있는 데이타 전송
				if ( smWSock[cnt].PacketSendQueCnt && smWSock[cnt].smSendState.Sleep ) {
					PopSendPacketQue( &smWSock[cnt] );
				}

				//보내기 오류 재시도
				if ( !smWSock[cnt].smSendState.Sucess ) {
					if ( smWSock[cnt].smSendState.Sleep ) {
						ResumeThread( smWSock[cnt].hSendThread );
					}
				}

			}
		}
	}
	//크리티칼 섹션 해제
	LeaveCriticalSection( &cSocketSection );

	return MsgCnt;
}


//소켓을 끈고 다시 연결 시킨다
int smReconnectSock( smWINSOCK *lpsmSock )
{
	int res;

	//크리티칼 섹션 선언
	EnterCriticalSection( &cSocketSection );

	res = lpsmSock->ReconnectSock();

	//크리티칼 섹션 해제
	LeaveCriticalSection( &cSocketSection );

	return res;
}


//소켓을 연결 시킨다 ( IP를 확인하여 기존의 같은 IP가 존재하면 기존 IP 접속 불가 )
smWINSOCK *smConnectSock2( char *szIP , WORD port )
{
	int cnt;
	smWINSOCK *lpsmSock = 0;
	smWINSOCK *lpFindsmSock;
	DWORD	dwIP;

	//크리티칼 섹션 선언
	EnterCriticalSection( &cSocketSection );

	dwIP = inet_addr ( szIP );
	lpFindsmSock = smFindSocket2( dwIP );

	if ( !lpsmSock ) {
		if ( smTransThreadMode ) {
			for(cnt=0;cnt<CONNECTMAX;cnt++) {
				if ( smWSock[cnt].sock==0 && !smWSock[cnt].lpsmRecvRoute && !smWSock[cnt].lpsmSendRoute) {
					lpsmSock = &smWSock[cnt];
				}
			}
		}
		else {
			for(cnt=0;cnt<CONNECTMAX;cnt++) {
				if ( smWSock[cnt].sock==0 ) {
					if ( smWSock[cnt].ConnectSock( szIP, port )!=FALSE )
						lpsmSock = &smWSock[cnt];
					break;
				}
			}
		}
	}

	//크리티칼 섹션 해제
	LeaveCriticalSection( &cSocketSection );

	return lpsmSock;
}


//문자열 IP로 소켓을 찾음
smWINSOCK *smFindSocket( char *szIP )
{
	int cnt;

	for(cnt=0;cnt<CONNECTMAX;cnt++) {
		if ( smWSock[cnt].sock && lstrcmpi( smWSock[cnt].szIPAddr , szIP )==0 )
			return &smWSock[cnt];
	}

	return NULL;
}

//32비트 IP로 소켓을 찾음
smWINSOCK *smFindSocket2( DWORD dwIP )
{
	int cnt;

	for(cnt=0;cnt<CONNECTMAX;cnt++) {
		if ( smWSock[cnt].sock && smWSock[cnt].acc_sin.sin_addr.S_un.S_addr==dwIP)
			return &smWSock[cnt];
	}

	return NULL;
}



int WSAMessage_Accept( DWORD wParam , DWORD lParam )
{
	int cnt;
	int Result;

	Result = FALSE;

	//크리티칼 섹션 선언
	EnterCriticalSection( &cSocketSection );

	if ( smTransThreadMode ) {
		for(cnt=0;cnt<CONNECTMAX;cnt++) {
			if ( smWSock[cnt].sock==0 && !smWSock[cnt].lpsmRecvRoute && !smWSock[cnt].lpsmSendRoute) {
				Result = smWSock[cnt].mesAccept( wParam, lParam );
				break;
			}
		}
	}
	else {
		for(cnt=0;cnt<CONNECTMAX;cnt++) {
			if ( smWSock[cnt].sock==0 ) {
				Result = smWSock[cnt].mesAccept( wParam, lParam );
				break;
			}
		}
	}

	if ( Result==FALSE ) {
		SOCKADDR_IN acc_sin;    /* Accept socket address - internet style */
		int acc_sin_len;        /* Accept socket address length */
		SOCKET sock;

	    acc_sin_len = sizeof( acc_sin );
	    sock = accept( (SOCKET)wParam,(struct sockaddr FAR *) &acc_sin, (int FAR *) &acc_sin_len );

		//if ((status = WSAAsyncSelect( sock, hWnd, WSA_READ, FD_READ | FD_WRITE | FD_CLOSE )) > 0) {
	    closesocket( sock );
	}


	//크리티칼 섹션 해제
	LeaveCriticalSection( &cSocketSection );
/*
	if ( Result ) {
		ConnectPlayer( &smWSock[cnt] );
	}
*/
	return Result;
}

//소켓용 크리티칼 섹션 선언
void smEnterCSectionSocket()
{
	//크리티칼 섹션 선언
	EnterCriticalSection( &cSocketSection );
}

//소켓용 크리티칼 섹션 해제
void smLeaveCSectionSocket()
{
	//크리티칼 섹션 해제
	LeaveCriticalSection( &cSocketSection );
}

int WSAMessage_Read( DWORD wParam , DWORD lParam )
{
	int cnt;
	int Result;
	
	Result = FALSE;

	//크리티칼 섹션 선언
	EnterCriticalSection( &cSocketSection );

	for(cnt=0;cnt<CONNECTMAX;cnt++) {
		if ( smWSock[cnt].sock==(SOCKET)wParam && smWSock[cnt].WaitReadMessage>=0 ) {
			Result = smWSock[cnt].mesReceve( lParam );
			break;
		}
	}
#ifdef PT_WEB
	extern int g_PtNetDebug;
	if ( g_PtNetDebug ) {
		int i = 0;
		while ( i<CONNECTMAX && smWSock[i].sock!=(SOCKET)wParam ) i++;
		printf( "[smwsock] WSA_READ sock %d event %X -> %s\n", (int)wParam, (int)lParam,
			cnt<CONNECTMAX ? "handled" : i<CONNECTMAX ? "socket waits (WaitReadMessage<0)" : "unknown socket" );
	}
#endif

	//크리티칼 섹션 해제
	LeaveCriticalSection( &cSocketSection );

	return Result;	
}



static int status;
static char szBuff[128];
static SOCKADDR_IN local_sin;  
static SOCKADDR_IN acc_sin;    
static int acc_sin_len;

//#define MAX_PENDING_CONNECTS 4  /* The backlog allowed for listen() */
#define MAX_PENDING_CONNECTS 32  /* The backlog allowed for listen() */
#define NO_FLAGS_SET         0  /* Used with recv()/send()          */
#define MY_MSG_LENGTH       80  /* msg buffer sent back and forth   */

SOCKET BindSock;

u_short BindPort = 23;            /* Which tcp port are we going to use?   */

BOOL FillAddr( HWND hWnd, PSOCKADDR_IN psin, BOOL bClient );


void InitBindSock( int port )
{
	HWND	hWnd = hwnd;
	SOCKET	sock;

	BindPort = port;

	sock = socket( AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) {
        MessageBox(hWnd, "socket() failed", "Error", MB_OK);
        return;
    }

    if (!FillAddr( hWnd, &local_sin, FALSE)) {
        closesocket( sock );
        return;
    }


         if (bind( sock, (struct sockaddr FAR *) &local_sin, sizeof(local_sin)) == SOCKET_ERROR) {
            sprintf(szBuff, "%d is the error", WSAGetLastError());

            MessageBox(hWnd, szBuff, "bind(sock) failed", MB_OK);
            closesocket( sock );
            return;
         }

         if (listen( sock, MAX_PENDING_CONNECTS ) < 0) {
            sprintf(szBuff, "%d is the error", WSAGetLastError());

            MessageBox(hWnd, szBuff, "listen(sock) failed", MB_OK);
            return;
         }

         /*
         *   Send window a WSA_ACCEPT when something is trying to connect.
         */
         if ((status = WSAAsyncSelect( sock, hWnd, WSA_ACCEPT, FD_ACCEPT)) > 0) {
            wsprintf( szBuff, "%d (0x%x)");
            MessageBox( hWnd, "Error on WSAAsyncSelect()", szBuff, MB_OK);
            SetWindowText( hWnd, "Async listen cancelled");
            closesocket( sock );
         }

		 BindSock = sock;
}

void CloseBindSock()
{
	closesocket( BindSock );
}


/****************************************************************************\
*
*    FUNCTION:  FillAddr(HWND, PSOCKADDR_IN, BOOL)
*
*    PURPOSE:  Retrieves the IP address and port number.
*
*    COMMENTS:
*        This function is called in two conditions.
*            1.) When a client is preparing to call connect(), or
*            2.) When a server host is going to call bind(), listen() and
*                accept().
*        In both situations, a SOCKADDR_IN structure is filled.
*        However, different fields are filled depending on the condition.
*
*   ASSUMPTION:
*      szBuff is a global variable that contains the remote host name or NULL
*      if local.
*      bClient determines if this is being called by a client ( will be
*         performing a connect ) or a server ( will be listening )
*
*
*\***************************************************************************/

BOOL FillAddr(
        HWND hWnd,
        PSOCKADDR_IN psin,
        BOOL bClient)
{
   DWORD dwSize;
   PHOSTENT phe;
   char szTemp[200];


   psin->sin_family = AF_INET;
   psin->sin_port = htons(BindPort);        /* Convert to network ordering */

   /*
   *   If we are setting up for a listen() call (bConnect = FALSE),
   *   fill servent with our address.
   */
   if (bClient) {
      phe = gethostbyname(szBuff);
      if (phe == NULL) {
         sprintf(szTemp, "%d is the error. Make sure '%s' is listed in the hosts file.", WSAGetLastError(), szBuff);

         MessageBox(hWnd, szTemp, "gethostbyname() failed.", MB_OK);
         return FALSE;
      }
      memcpy((char FAR *)&(psin->sin_addr), phe->h_addr,
         phe->h_length);

      }
   else { // server

      /*
      *   Retrieve my ip address.  Assuming the hosts file in
      *   in %systemroot%/system/drivers/etc/hosts contains my computer name.
      */

      dwSize = sizeof(szBuff);
      gethostname(szBuff, dwSize);

      psin->sin_addr.s_addr = INADDR_ANY;
      }

   return TRUE;
}
/*
#ifdef	_PACKET_PASS_XOR
//패킷을 암호화 한다
int	EncodePacket( DWORD dwKey , DWORD PacketCnt , BYTE *lpPacket , BYTE *lpEncBuff )
{
	int cnt;
	BYTE ch;
	BYTE bNear;
	DWORD rKey,sKey;
	int	len;
	WORD wTotal;

	sKey = (dwKey<<16)|(PacketCnt&0xFFFF);

	len = ((int *)lpPacket)[0];
	((int *)lpEncBuff)[0] = len+8;
	((int *)lpEncBuff)[2] = sKey;

	rKey = sKey>>4;
	rKey += len;

	bNear = (BYTE)(PacketCnt+dwKey);
	wTotal = 0;

	for( cnt=4;cnt<len;cnt++ ) {
		wTotal += lpPacket[cnt];
		ch = lpPacket[cnt] ^ ((BYTE)rKey) ^ bNear;
		lpEncBuff[cnt+8] = ch;
		rKey += cnt*(cnt>>1);
		bNear = ch^_PACKET_PASS_XOR;
	}

	((int *)lpEncBuff)[1] = (dwKey&0xFFFF0000)|wTotal;



	return TRUE;
}

//암호화패킷 을 복구한다
DWORD DecodePacket( BYTE *lpPacket , BYTE *lpDecBuff )
{
	int cnt;
	BYTE ch;
	BYTE ch2;
	BYTE bNear;
	DWORD rKey,sKey;
	int	len;
	DWORD dwKey;
	DWORD PacketCnt;
	WORD wTotal;


	len = ((int *)lpPacket)[0]-8;
	sKey = ((int *)lpPacket)[2];

	dwKey = sKey>>16;
	PacketCnt = sKey&0xFFFF;

	rKey = sKey>>4;
	rKey += len;

	bNear = (BYTE)(PacketCnt+dwKey);
	wTotal = 0;

	((int *)lpDecBuff)[0] = len;

	for( cnt=4;cnt<len;cnt++ ) {
		ch = lpPacket[cnt+8];
		ch2= ch;
		ch ^=((BYTE)rKey)^bNear;
		lpDecBuff[cnt] = ch;
		rKey += cnt*(cnt>>1);
		bNear = ch2^_PACKET_PASS_XOR;
		wTotal += ch;
	}

	if ( (((int *)lpPacket)[1]&0xFFFF)!=wTotal ) 
		return NULL;

	return sKey;
}
#else
*/

//패킷을 암호화 한다
int	EncodePacket( DWORD dwKey , DWORD PacketCnt , BYTE *lpPacket , BYTE *lpEncBuff )
{
	int cnt;
	BYTE ch;
	BYTE bNear;
	DWORD rKey,sKey;
	int	len;
	WORD wTotal;

	sKey = (dwKey<<16)|(PacketCnt&0xFFFF);

	len = ((int *)lpPacket)[0];
	((int *)lpEncBuff)[0] = len+8;
	((int *)lpEncBuff)[2] = sKey;

	rKey = sKey>>4;
	rKey += len;

	bNear = (BYTE)(PacketCnt+dwKey);
	wTotal = 0;

	for( cnt=4;cnt<len;cnt++ ) {
		wTotal += lpPacket[cnt];
		ch = lpPacket[cnt] ^ ((BYTE)rKey) ^ bNear;
		lpEncBuff[cnt+8] = ch;
		rKey += cnt*(cnt>>1);
#ifdef	_PACKET_PASS_XOR
		bNear = ch^_PACKET_PASS_XOR;
#else
		bNear = ch;
#endif
	}

	((int *)lpEncBuff)[1] = (dwKey&0xFFFF0000)|wTotal;



	return TRUE;
}

//암호화패킷 을 복구한다
DWORD DecodePacket( BYTE *lpPacket , BYTE *lpDecBuff )
{
	int cnt;
	BYTE ch;
	BYTE ch2;
	BYTE bNear;
	DWORD rKey,sKey;
	int	len;
	DWORD dwKey;
	DWORD PacketCnt;
	WORD wTotal;


	len = ((int *)lpPacket)[0]-8;
	sKey = ((int *)lpPacket)[2];

	dwKey = sKey>>16;
	PacketCnt = sKey&0xFFFF;

	rKey = sKey>>4;
	rKey += len;

	bNear = (BYTE)(PacketCnt+dwKey);
	wTotal = 0;

	((int *)lpDecBuff)[0] = len;

	for( cnt=4;cnt<len;cnt++ ) {
		ch = lpPacket[cnt+8];
		ch2= ch;
		ch ^=((BYTE)rKey)^bNear;
		lpDecBuff[cnt] = ch;
		rKey += cnt*(cnt>>1);
#ifdef	_PACKET_PASS_XOR
		bNear = ch2^_PACKET_PASS_XOR;
#else
		bNear = ch2;
#endif
		wTotal += ch;
	}

	if ( (((int *)lpPacket)[1]&0xFFFF)!=wTotal ) 
		return NULL;

	return sKey;
}
//#endif



//패킷을 암호화 한다
int	EncodePacket2( DWORD dwKey , DWORD PacketCnt , BYTE *lpPacket , BYTE *lpEncBuff , BYTE bEncXor )
{
	int cnt;
	BYTE ch;
	BYTE bNear;
	DWORD rKey,sKey;
	int	len;
	WORD wTotal;

	dwKey |=smTRANSCODE_ENCODE_PACKET;
	sKey = (dwKey<<16)|(PacketCnt&0xFFFF);

	len = ((int *)lpPacket)[0];
	((int *)lpEncBuff)[0] = len+8;
	((int *)lpEncBuff)[2] = sKey;

	rKey = sKey>>4;
	rKey += len;

	bNear = (BYTE)(PacketCnt+dwKey);
	wTotal = 0;

	for( cnt=4;cnt<len;cnt++ ) {
		wTotal += lpPacket[cnt];
		ch = lpPacket[cnt] ^ ((BYTE)rKey) ^ bNear;
		lpEncBuff[cnt+8] = ch;
		rKey += cnt*(cnt>>1);
		bNear = ch^bEncXor;
	}

	((int *)lpEncBuff)[1] = (dwKey&0xFFFF0000)|wTotal;



	return TRUE;
}

//암호화패킷 을 복구한다
DWORD DecodePacket2( BYTE *lpPacket , BYTE *lpDecBuff , BYTE bDecXor )
{
	int cnt;
	BYTE ch;
	BYTE ch2;
	BYTE bNear;
	DWORD rKey,sKey;
	int	len;
	DWORD dwKey;
	DWORD PacketCnt;
	WORD wTotal;


	len = ((int *)lpPacket)[0]-8;
	sKey = ((int *)lpPacket)[2];

	dwKey = sKey>>16;
	PacketCnt = sKey&0xFFFF;

	rKey = sKey>>4;
	rKey += len;

	bNear = (BYTE)(PacketCnt+dwKey);
	wTotal = 0;

	((int *)lpDecBuff)[0] = len;

	for( cnt=4;cnt<len;cnt++ ) {
		ch = lpPacket[cnt+8];
		ch2= ch;
		ch ^=((BYTE)rKey)^bNear;
		lpDecBuff[cnt] = ch;
		rKey += cnt*(cnt>>1);
		bNear = ch2^bDecXor;
		wTotal += ch;
	}

	if ( (((int *)lpPacket)[1]&0xFFFF)!=wTotal ) 
		return NULL;

	return sKey;
}



/*
typedef DWORD (*LPFN_EncodePacket)( DWORD dwKey , DWORD PacketCnt , BYTE *lpPacket , BYTE *lpEncBuff );
typedef DWORD (*LPFN_DecodePacket)( BYTE *lpPacket , BYTE *lpDecBuff );
#define	POS_ENC_START	0x0
#define	POS_ENC_PACCODE 0xB
#define POS_ENC_XOR		0x74
#define	POS_DEC_START	0xA0
#define	POS_DEC_XOR		0x11A
#define	ENC_PACKET_ASM_SIZE	0x150
*/

LPFN_EncodePacket	fnEncodePacket =0;
LPFN_DecodePacket	fnDecodePacket =0;

#ifdef _W_SERVER
BYTE smwsock_EncodePacket_AsmCode[ENC_PACKET_ASM_SIZE] = {
//d8ab 80010000
//d914 33
//D9BA 33
//int	zEncodePacket( DWORD dwKey , DWORD PacketCnt , BYTE *lpPacket , BYTE *lpEncBuff )
 0x8b,0x4c,0x24,0x04,0x53,0x8b,0x5c,0x24,0x0c,0x81,0xc9,0x00,0x00,0x01,0x80,0x55,  //00
 0x8b,0xd3,0x81,0xe2,0xff,0xff,0x00,0x00,0x8b,0xc1,0xc1,0xe0,0x10,0x0b,0xc2,0x8b,  //10
 0x54,0x24,0x14,0x56,0x8b,0x32,0x57,0x8b,0x7c,0x24,0x20,0x89,0x47,0x08,0xc1,0xe8,  //20
 0x04,0x8d,0x56,0x08,0x03,0xc6,0x89,0x17,0x8b,0xd0,0xb8,0x04,0x00,0x00,0x00,0x02,  //30
 0xd9,0x33,0xed,0x3b,0xf0,0x89,0x4c,0x24,0x14,0x7e,0x35,0xeb,0x03,0x8d,0x49,0x00,  //40
 0x8b,0x4c,0x24,0x1c,0x8a,0x0c,0x08,0x66,0x0f,0xb6,0xf9,0x32,0xca,0x32,0xcb,0x8b,  //50
 0xd8,0xd1,0xfb,0x0f,0xaf,0xd8,0x03,0xef,0x8b,0x7c,0x24,0x20,0x88,0x4c,0x07,0x08,  //60
 0x03,0xd3,0x80,0xf1,0x33,0x40,0x3b,0xc6,0x8a,0xd9,0x7c,0xd4,0x8b,0x4c,0x24,0x14,  //70
 0x0f,0xb7,0xd5,0x81,0xe1,0x00,0x00,0xff,0xff,0x0b,0xca,0x89,0x4f,0x04,0x5f,0x5e,  //80
 0x5d,0xb8,0x01,0x00,0x00,0x00,0x5b,0xc3,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,  //90

//DWORD zDecodePacket( BYTE *lpPacket , BYTE *lpDecBuff )
 0x83,0xec,0x0c,0x56,0x57,0x8b,0x7c,0x24,0x18,0x8b,0x57,0x08,0x8b,0x37,0x8b,0xc2,  //A0
 0xc1,0xe8,0x10,0x02,0xc2,0x83,0xee,0x08,0x88,0x44,0x24,0x18,0x8b,0x44,0x24,0x1c,  //B0
 0x8b,0xca,0xc1,0xe9,0x04,0x89,0x30,0x03,0xce,0xb8,0x04,0x00,0x00,0x00,0x3b,0xf0,  //C0
 0x89,0x54,0x24,0x10,0x89,0x4c,0x24,0x08,0xc7,0x44,0x24,0x0c,0x00,0x00,0x00,0x00,  //D0
 0x7e,0x52,0x53,0x55,0xeb,0x0a,0x8b,0x4c,0x24,0x10,0x8d,0x9b,0x00,0x00,0x00,0x00,  //E0
 0x8a,0x54,0x07,0x08,0x8a,0x5c,0x24,0x20,0x8b,0x6c,0x24,0x10,0x32,0xca,0x32,0xcb,  //F0
 0x8b,0x5c,0x24,0x24,0x88,0x0c,0x18,0x8b,0xd8,0xd1,0xfb,0x0f,0xaf,0xd8,0x66,0x0f,  //100
 0xb6,0xc9,0x03,0xeb,0x8b,0x5c,0x24,0x14,0x80,0xf2,0x33,0x89,0x6c,0x24,0x10,0x88,  //110
 0x54,0x24,0x20,0x03,0xd9,0x40,0x3b,0xc6,0x89,0x5c,0x24,0x14,0x7c,0xb8,0x8b,0x54,  //120
 0x24,0x18,0x5d,0x5b,0x66,0x8b,0x4c,0x24,0x0c,0x33,0xc0,0x66,0x39,0x4f,0x04,0x5f,  //130
 0x0f,0x95,0xc0,0x5e,0x48,0x23,0xc2,0x83,0xc4,0x0c,0xc3,0x90,0x90,0x90,0x90,0x90   //140
};
#else
BYTE smwsock_EncodePacket_AsmCode[ENC_PACKET_ASM_SIZE];
#endif

/*

#define _PACKET_PASS_XOR	0x33

//패킷을 암호화 한다
int	zEncodePacket( DWORD dwKey , DWORD PacketCnt , BYTE *lpPacket , BYTE *lpEncBuff )
{
	int cnt;
	BYTE ch;
	BYTE bNear;
	DWORD rKey,sKey;
	int	len;
	WORD wTotal;

	dwKey |=smTRANSCODE_ENCODE_PACKET;
	sKey = (dwKey<<16)|(PacketCnt&0xFFFF);

	len = ((int *)lpPacket)[0];
	((int *)lpEncBuff)[0] = len+8;
	((int *)lpEncBuff)[2] = sKey;

	rKey = sKey>>4;
	rKey += len;

	bNear = (BYTE)(PacketCnt+dwKey);
	wTotal = 0;

	for( cnt=4;cnt<len;cnt++ ) {
		wTotal += lpPacket[cnt];
		ch = lpPacket[cnt] ^ ((BYTE)rKey) ^ bNear;
		lpEncBuff[cnt+8] = ch;
		rKey += cnt*(cnt>>1);
		bNear = ch^_PACKET_PASS_XOR;
	}

	((int *)lpEncBuff)[1] = (dwKey&0xFFFF0000)|wTotal;



	return TRUE;
}

//암호화패킷 을 복구한다
DWORD zDecodePacket( BYTE *lpPacket , BYTE *lpDecBuff )
{
	int cnt;
	BYTE ch;
	BYTE ch2;
	BYTE bNear;
	DWORD rKey,sKey;
	int	len;
	DWORD dwKey;
	DWORD PacketCnt;
	WORD wTotal;


	len = ((int *)lpPacket)[0]-8;
	sKey = ((int *)lpPacket)[2];

	dwKey = sKey>>16;
	PacketCnt = sKey&0xFFFF;

	rKey = sKey>>4;
	rKey += len;

	bNear = (BYTE)(PacketCnt+dwKey);
	wTotal = 0;

	((int *)lpDecBuff)[0] = len;

	for( cnt=4;cnt<len;cnt++ ) {
		ch = lpPacket[cnt+8];
		ch2= ch;
		ch ^=((BYTE)rKey)^bNear;
		lpDecBuff[cnt] = ch;
		rKey += cnt*(cnt>>1);
		bNear = ch2^_PACKET_PASS_XOR;
		wTotal += ch;
	}

	if ( (((int *)lpPacket)[1]&0xFFFF)!=wTotal ) 
		return NULL;

	return sKey;
}
*/

