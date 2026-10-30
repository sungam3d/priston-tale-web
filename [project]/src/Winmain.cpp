#define _SINBARAM

#include <windows.h>
#include <math.h>
#include <stdio.h>
#include <ZMOUSE.H>
#include <process.h>

#include "smlib3d\\smd3d.h"
#ifdef PT_GFX_GL
#include "ptgfx\\ptgfx.h"			// real-time shadows (PtGfx_Shadow*)
#include <dsound.h>
#include "ptsound\\ptsound.h"		// master music / sound effect volumes
#include "weather.h"				// rain, storms, fog, snow
#endif

#ifdef PT_WEB
#include <emscripten.h>
// Page settings (web\settings.js): Module.ptOpts = { v: version, music, effects
// (0 = off .. 1), view (draw distance in map cells, 0 = the game's own), weather
// (weather.h PTWX_SET_*) }. Read
// once per frame (PlayD3D), and only when the version changed - the page never
// calls into the game itself.
static int s_WebViewCells = 0;
static void PtWeb_PollOptions()
{
	static int s_Version = -1;
	int v = EM_ASM_INT({ var o = Module['ptOpts']; return o ? (o.v|0) : 0; });
	if ( v==s_Version ) return;
	s_Version = v;
	double music = EM_ASM_DOUBLE({ var o = Module['ptOpts']; return o && typeof o.music=='number' ? o.music : 1; });
	double effects = EM_ASM_DOUBLE({ var o = Module['ptOpts']; return o && typeof o.effects=='number' ? o.effects : 1; });
	s_WebViewCells = EM_ASM_INT({ var o = Module['ptOpts']; return o && o.view ? (o.view|0) : 0; });
	if ( s_WebViewCells>110 ) s_WebViewCells = 110;		// the map table is 256 cells across
	PtWeather_Setting = EM_ASM_INT({ var o = Module['ptOpts']; return o && o.weather ? (o.weather|0) : 0; });
	PtSound_SetMasterVolume( (float)music, (float)effects );
}
#endif
#include "smwsock.h"
#include "smreg.h"

#include "character.h"
#include "playmain.h"
#include "srcsound\\dxwav.h"
#include "effectsnd.h"
#include "fileread.h"
#include "netplay.h"
#include "makeshadow.h"
//#include "cSelect.h"
#include "particle.h"
#include "field.h"
#include "hoBaram\\holinkheader.h"

#include "tjboy\\clanmenu\\tjclanDEF.h"
#include "tjboy\\clanmenu\\tjclan.h"
#include "tjboy\\clanmenu\\cE_CViewClanInfo.h"

#ifdef _SINBARAM
#include "sinbaram\\sinlinkheader.h"
#endif

#include "playsub.h"
#include "cracker.h"
#include "SkillSub.h"

#include "resource.h"

//#include "HanDes.h"			//한게임 암호화 모듈 //해외블럭

#include "TextMessage.h"		//문자 메세지 헤더
#include "language.h"
#include "srcLang\\jts.h"		//일본어 코드 첵크

#include "CurseFilter.h"		//욕설필터
#include "damage.h"
#include "AreaServer.h"

#include "BellatraFontEffect.h"

//------------------------------------< _ignore_xtrap_
#ifdef _LANGUAGE_VEITNAM
#ifdef _XTRAP_GUARD_4_CLIENT
	#include ".\\XTrapSrcD5\\Launcher\\XTrap4Launcher.h"
#endif
#endif
//------------------------------------  _ignore_xtrap_ >

//XTRAP 라이브러리 적용
#ifdef _XTRAP_GUARD_4_CLIENT
#include "XTrapSrcD5\\Client\\XTrap4Client.h"	
#pragma comment( lib,"XTrapSrcD5\\Client\\XTrap4Client_mt.lib")
#endif

#ifdef _XTRAP_GUARD_4_CLIENT // HEAP MEMERY TEST
#include "XTrapSrcD5\\ClientExt\\XTrap4ClientExt1.h"	
#pragma comment( lib,"XTrapSrcD5\\ClientExt\\XTrap4ClientExt1_st.lib")
#pragma comment( lib,"XTrapSrcD5\\ClientExt\\XTrap4ClientExt1_mt.lib")
#pragma comment( lib,"XTrapSrcD5\\ClientExt\\XTrap4ClientExt1_mtdll.lib")
#endif

//박재원 - XignCode
#ifdef _XIGNCODE_CLIENT
#include "Xigncode\Client\zwave_sdk_client.h"
#endif

//키크립트
/* temp by 2neo
#ifndef	_nProtect_For_Debug
#include "nprotect\\PT_KeyCrypt.h"
#endif
*/
//중국 xTrap
#ifdef _xTrap_GUARD
#include "XtrapSrc/xtrapgameclient.h"
#endif

#include "actiongame.h"

/* temp by 2neo
#ifdef _npGAME_GUARD
#include "nprotect\\PT_KeyCrypt.h"
#endif
*/

//######################################################################################
//작 성 자 : 박철호
#define WM_CALLMEMMAP				WM_USER+3   //메모리맵에 메세지가 있다가 통보

#ifdef PARKMENU
#include "TJBOY\\\\Park\\MICVOL\\MicVolume.h"
extern BOOL bMic;
extern int vrunRuning;
extern int micInit;
#endif

//######################################################################################

//######################################################################################
//작 성 자 : 오 영 석
#include "WinInt\\WinIntThread.h"
#include "FullZoomMap.h"
#include "FontImage.h"
//######################################################################################


//char szAppName[]="프리스톤 테일";
HWND hwnd;
HWND hTextWnd;
HWND hFocusWnd;

extern int sinChatDisplayMode; //{ 0- 아무것도 안그리기  1-보통  2-상점 }
extern int sinChatInputMode;   //{ 0- 아무것도 안그리기  1-보통  2-상점 }

HDC hdc;
int quit = 0;
int QuitSave = 0;
int WinSizeX = 640;
int WinSizeY = 480;
int	WinColBit = 16;

//######################################################################################
//작 성 자 : 오 영 석
float g_fWinSizeRatio_X;
float g_fWinSizeRatio_Y;
//######################################################################################


int TextMode;

int BrCtrl = 0;

DWORD	dwM_BlurTime = 0;


int MouseButton[3] = { 0,0,0 };
extern int TJwheel; //ktj
extern int keydownEnt;


//IME 관리 라이브러리 추가
#pragma comment( lib, "imm32.lib" )


#define DLLEXPORT __declspec(dllexport)

extern "C" {
	DLLEXPORT BOOL _stdcall smSetCurrentWindows( HWND wnd );
	DLLEXPORT int _stdcall InitD3D(HWND hWnd);
	DLLEXPORT void _stdcall CloseD3d();
	DLLEXPORT void _stdcall smPlayD3D( int x, int y, int z, int ax, int ay, int az );
	DLLEXPORT int _stdcall smSetMode( HWND hWnd , DWORD Width, DWORD Height, DWORD BPP );
}


extern "C" BOOL WINAPI DllMain(
  HINSTANCE hinstDLL,  // handle to DLL module
  DWORD fdwReason,     // reason for calling function
  LPVOID lpvReserved   // reserved
);
///////////////////////// IME 관련 //////////////////////////////

#include "ime.h"

ImeClass IMETest;
CStrClass sz;
LRESULT CALLBACK EditWndProc01(HWND,UINT,WPARAM,LPARAM);
WNDPROC		OldEditProc01;
char szIME_Buff[10][64];


//디버그 플랙 백업
DWORD	dwDebugBack;
DWORD	dwDebugXor;

///////////////////////// 게임에 필요한 선언들 ///////////////////
int MouseX , MouseY;
int MousemX, MousemY;
int angX = 0;
int angY = 0;
DWORD	dwLastMouseMoveTime;
DWORD	dwLastCharMoveTime;

int xPos,yPos;

POINT3D TraceCameraPosi;
POINT3D TraceTargetPosi;
int	PlayCameraMode = 1;
int AutoCameraFlag = FALSE;
int	CameraInvRot = 0;					//카메라 회전 방향
int	CameraSight = 0;					//카메라 시야 조절

//smPAT3D *pat3d;
//LPDIRECT3DTEXTURE2 hOBJ3DTexture;
LPDIRECTDRAWSURFACE4 lpDDSMenu;

//ccsSELECT	*lpPlaySelect = 0;

int DisplayDebug = 0;
int DispInterface = TRUE;			//인터페이스 그리기
int	DebugPlayer = FALSE;				//디버깅 카메라 플레이어
int	LastAttackDamage = 0;			//마지막 공격 데미지

int	HoMsgBoxMode = 0;				//호동 메세지 박스 프레임 표시 비표시 토글

//sinTrade.cpp 에 디파인 되 있도다.
#define TRADEBUTTONMAIN_X		513
#define TRADEBUTTONMAIN_Y		3

POINT pHoPartyMsgBox = { TRADEBUTTONMAIN_X, TRADEBUTTONMAIN_Y };	//호동 파티신청 메세지 박스


//운영자용 IP
char *szOperationIP[5] = {
	"211.61.248.221",
	"211.108.45.",
	"211.44.231.",
	"221.148.123.",
	0
};

char szExitInfo_URL[128]={ 0, };		//게임 종료시 표시하는 URL 경로


#define ANX_NONE	-32768

int pX;
int pY;
int pZ;
int	dpX=0;
int dpY=0;
int dpZ=0;

int whAnx = 64;
int anx = 64;
int	ViewAnx = 64;
int	ViewDist = 100;
int	PlayFloor = 0;
int any = 0;
int anz = 0;
int dist = 100;
int CaTarDist = 0;
int	CaMovAngle = 0;

int tX = 0;
int tY = 0;
int tZ = 0;


int	ImeChatLen = 0;		//IME 관련 채팅 문자길이
POINT	ImePositon;		//IME 위치 저장

int SetMousePlay( int flag );

//호동동 추가(카메라 이펙트)
int WaveCameraMode = TRUE;
POINT3D WaveCameraPosi;									
int	WaveCameraFactor = 0; // 10*fONE;		    //흔드는 크기
int	WaveCameraTimeCount = 0;
int	WaveCameraDelay = 0;
BOOL	WaveCameraFlag = FALSE;



//게임 초기화
int GameInit();
//게임 닫기
int GameClose();
//게임 선택
int SetGameMode( int mode );

//채팅창 문자 설정및 활성화
int	SetChatingLine( char *szMessage );

int GameMode = 0;

//그리기 크리티컬 섹션
CRITICAL_SECTION	cDrawSection;

//해외용 함수
int HoInstallFont();	//폰트 등록하기
int HoUninstallFont();	//폰트 삭제하기

HINSTANCE hinst; 
BOOL WINAPI DllMain(	HINSTANCE hinstDLL,  // handle to DLL module
						DWORD fdwReason,     // reason for calling function
						LPVOID lpvReserved   // reserved
						) {

	hinst = hinstDLL;

	return TRUE;

}

#define IDC_TEXT	101

BYTE VRKeyBuff[256];


LONG APIENTRY WndProc(HWND,UINT,WPARAM,LPARAM);
void PlayD3D();
//코멘드 라인 분석 설정
int DecodeCmdLine( char *lpCmdLine );
//불법 소프트 웨어 단속에 대비한 레지스트리 조작
int HaejukReg();


//폰트 생성
HFONT	hFont = 0;
int SetCreateFont();

//크리티컬 섹션 선언
CRITICAL_SECTION	cSection;

//IME Context
HIMC hImc;
HIMC hImcEdit;

extern rsRECORD_DBASE	rsRecorder;					//서버에 게임데이타 기록장치
extern INT WINAPI ServerWinMain(HINSTANCE hInst, HINSTANCE hPreInst, LPSTR lpCmdLine,INT nCmdShow);

//int AllUserDataSearch();
char	szCmdLine[128];			//커맨드라인 문자열


//클로즈 베타 테스터 초기화
extern int	InitCloseBetaUser();
//서버에 기록된 전체 데이타를 확인하여 의심가는 유저를 찾는다
extern int	CheckServerRecordData();

//캐릭터 정보 파일에서 해독하여 설정한다
extern int RestoreBackupData( char *szListFile , char *BackupPath );

#ifdef	_LANGUAGE_JAPANESE

static char *StripArg0(LPSTR cmdline)
{
    char *ptr;

    ptr = cmdline;
    while (*ptr && *ptr == ' ' || *ptr == '\t')
        ptr++;

    if (*ptr == '"') {
        ptr++;
        while (*ptr && *ptr != '"')
            ptr++;
        if (*ptr == '"') {
            ptr++;
            while (*ptr && *ptr == ' ')
                ptr++;
            return ptr;
        }
    }
    else {
while (*ptr && *ptr != ' ')
            ptr++;
        if (*ptr == ' ')
            return ptr+1;  
    }

    return ("");
}
#endif

/*
#include "srcserver\dllserver.h"

typedef int (*LPFN_LogITEMRecord)( char *lpszPID, LogITEM *LogItem );
typedef int (*LPFN_LogITEMTradeRecord)( char *lpszPID, LogITEM_TRADE *LogItem );
typedef int (*LPFN_LogITEMPostRecord)(char *lpszPID, LogGiftITEM *GiftItem);
*/

#ifdef _W_SERVER

#include "mini_dump.h"
#include "OfflineServer.h"
#ifdef PT_WEB
int PtWeb_IsServer();		// web\ptwin\src\ptwin_main.cpp
#endif
#ifdef PT_USE_SDL
#include "ptplatform\ptplatform.h"
#endif
Mini_Dump CMiniDump;

#endif


#ifdef _DEBUG
#include <crtdbg.h>
//Debug 빌드: 힙 손상을 찾기 위한 진단
// The debug C runtime checks the heap every 128 allocations. Any problem it
// reports is appended to heap_report.txt and the program then crashes on
// purpose so mini_dump writes a .dmp showing where it was detected.
// Set the environment variable PT_BREAK_ALLOC=<n> to stop when allocation
// number n (from the report) is made, to see who allocated the damaged block.
static int CrtReportToFile( int reportType, char *message, int *returnValue )
{
	FILE *fp = fopen( "heap_report.txt", "at" );
	if ( fp ) {
		fputs( message, fp );
		fclose( fp );
	}
	if ( reportType != _CRT_WARN )
		*(volatile int *)0 = 0;
	return TRUE;
}

static void InitDebugHeapChecks()
{
	_CrtSetDbgFlag( _CrtSetDbgFlag( _CRTDBG_REPORT_FLAG ) | _CRTDBG_ALLOC_MEM_DF | _CRTDBG_CHECK_EVERY_128_DF );
	if ( GetEnvironmentVariable( "PT_HEAP_CHECK_ALWAYS", NULL, 0 ) )		//slow: check on every allocation
		_CrtSetDbgFlag( ( _CrtSetDbgFlag( _CRTDBG_REPORT_FLAG ) & 0xFFFF ) | _CRTDBG_CHECK_ALWAYS_DF );
	_CrtSetReportHook( CrtReportToFile );
	char szBreak[32];
	if ( GetEnvironmentVariable( "PT_BREAK_ALLOC", szBreak, sizeof(szBreak) ) )
		_CrtSetBreakAlloc( atol( szBreak ) );
}
#endif

INT WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPreInst, LPSTR lpCmdLine,INT nCmdShow)
{
#ifdef _DEBUG
	InitDebugHeapChecks();
#endif

	//------------------------------------< _ignore_xtrap_
	#ifdef _LANGUAGE_VEITNAM
	#ifdef _XTRAP_GUARD_4_CLIENT
	{
		#pragma comment(lib,".\\XTrapSrcD5\\Launcher\\XTrap4Launcher_mt.lib")
		#pragma comment( lib, "Urlmon.lib")
		#pragma comment( lib, "Wininet.lib")
	    XTrap_L_Patch(XTRAP_CONFIG_ARG, NULL, 60);
	}
	#endif
	#endif
	//------------------------------------  _ignore_xtrap_ >


	MAKELOG(" ------------ WINMAIN ---------------- ");
	MSG msg;
	WNDCLASS wndclass;
	hinst = hInst;
	int	sx,sy;
/*
	TRANS_RECORD_DATA	TransRecordData;
	TRANS_RECORD_DATA	TransRecordDataBack;
	FILE *fp;

	fp=fopen("DataServer\\tae.dat","rb");
	if ( fp ) {
		fread( &TransRecordData , sizeof(TRANS_RECORD_DATA) , 1, fp );
		fclose(fp);
	}
	fp=fopen("DataServer\\newsam.dat","rb");
	if ( fp ) {
		fread( &TransRecordDataBack , sizeof(TRANS_RECORD_DATA) , 1, fp );
		fclose(fp);
	}

	fp = 0;
*/

	//CheckDLL_Module();
/*
	sx = 100;
	sy = 200;
	dsGetDrawOffset( &sx , &sy );
*/

/*
	struct	tm	tm;
	time_t		ttm,ttm2;

	tm.tm_year = 2004 - 1900;	// 2003년
	tm.tm_mon = 2-1;			// 10월
	tm.tm_mday = 16;			// 29일
	tm.tm_hour = 17;			// 9시
	tm.tm_min = 58;				// 0분
	tm.tm_sec = 45;				// 0초

	ttm = mktime( &tm );

	::time( &ttm2 );			//시간 구하기

	::time( &ttm2 );			//시간 구하기
*/

/*
	HMODULE	hLogSqlDLL;
	hLogSqlDLL = LoadLibrary("SQL");		//Sql.Dll
	LogITEM_TRADE	LogItem;

	LPFN_LogITEMTradeRecord	LogITEMTradeRecord	=(LPFN_LogITEMTradeRecord)GetProcAddress(hLogSqlDLL,"LogITEMTradeRecord");
	LPFN_LogITEMPostRecord	LogITEMPostRecord	=(LPFN_LogITEMPostRecord)GetProcAddress(hLogSqlDLL,"LogITEMPostRecord");
	LPFN_LogITEMRecord		LogITEMRecord		=(LPFN_LogITEMRecord)GetProcAddress(hLogSqlDLL,"LogITEMRecord");


	ZeroMemory( &LogItem , sizeof(LogITEM_TRADE) );

	LogItem.Flag = 8;
	lstrcpy( LogItem.UserID , "lpPlayInfo->szID" );
	lstrcpy( LogItem.CharName , "lpPlayInfo->szName" );
	lstrcpy( LogItem.TUserID  , "lpPlayInfo2->szID" );
	lstrcpy( LogItem.TCharName , "lpPlayInfo2->szName" );
	LogItem.IP = 123;
	LogItem.TIP = 456;

	LogItem.Money = 0;

	LogItem.Item[0].dwCode   = 999;
	LogItem.Item[0].dwINo[0] = 888;
	LogItem.Item[0].dwINo[1] = 777;
	LogItem.ItemCount ++;
	LogItem.size = sizeof(LogITEM_TRADE)-( sizeof(_LogITEM)*(32-LogItem.ItemCount) );	//구조체의 사용된 크기


	LogITEMTradeRecord( "testID" , &LogItem );

	return FALSE;
*/
/*
	sITEMINFO	TestItem;
	FILE *fp;
	fp = fopen( "test.itm" , "rb" );
	if ( fp ) {
		fread( &TestItem , sizeof( sITEMINFO ) , 1, fp );
		fclose( fp );
		return TRUE;
	}
*/

	//EncryptFunc( "show me the money", szCmdLine, 66 , 'a' );
	//EncryptFunc( "/ID=battery /PWD=asdfgh", szCmdLine, 66 , 'a' );

	//클로즈 베타 테스터 초기화
	//InitCloseBetaUser();
	//CheckServerRecordData();
	//return TRUE;

	//캐릭터 정보 파일에서 해독하여 설정한다
	//return RestoreBackupData( "복구명단.txt" , "d:\\backup\\3월21일\\" );


/*
	char szBuff1[256];
	EncryptFunc( "/ID=battery /PWD=asdfgh", szBuff1, 66 , 'a' );
	wsprintf( szCmdLine, "/IP=211.50.44.144 /ENC=%s /HANGAME" , szBuff1 );
	// /IP=211.50.44.144 /ENC=023Ba49195 /HANGAME
*/


	lstrcpy( szCmdLine , lpCmdLine );
//	HoCommandLineDecoder();		//해외 블럭(Handes 관련)

	initSinCos();		//삼각함수 초기화
	//기본 설정 불러옴
	//if ( smConfigDecode( "default.ini"  )==FALSE ) return FALSE;
#ifdef PT_SERVER_EXE		//server.exe 빌드 (_W_SERVER 는 NetType.hpp 에서 항상 켜져 있음)
	smConfigDecode( "server.ini"  );		//server.exe 는 자기 설정 파일을 쓴다 (클라이언트와 같은 폴더)
#elif defined(PT_WEB)
	smConfigDecode( (char *)( PtWeb_IsServer() ? "server.ini" : "hotuk.ini" ) );		// browser: one game.wasm runs as game and as server
#else
	smConfigDecode( "hotuk.ini"  );
#endif
	ReadShotcutMessage( "ShortCut.ini" );		//단축 메세지 읽어 오기
	DecodeCmdLine( lpCmdLine );

#ifndef PT_SERVER_EXE
	//오프라인 솔로 플레이: server.exe + server.ini 가 있으면 로컬 서버를 띄우고 접속한다
#ifdef PT_WEB
	if ( !PtWeb_IsServer() )
#endif
	{
		WORD wPort = (WORD)( smConfig.dwServerPort ? smConfig.dwServerPort : TCP_SERVPORT );
		int OfflineResult = StartOfflineServer( wPort );

		if ( OfflineResult==OFFLINE_SERVER_FAILED ) {
			MessageBox( NULL, "server.exe did not start.\nCheck server.ini and the game data next to game.exe.",
				"Priston Tale - offline", MB_OK|MB_ICONERROR );
			return FALSE;
		}
#ifdef PT_WEB
		int PtWeb_RemoteServer();
		BOOL bShared = PtWeb_RemoteServer();		// shared server.exe: every connection goes through the site's bridge
#else
		BOOL bShared = FALSE;
#endif
		if ( OfflineResult==OFFLINE_SERVER_READY || bShared ) {
			if ( !bShared ) atexit( StopOfflineServer );
			//모든 서버 주소를 로컬 서버로 (로그인 화면은 DataServer 주소로 접속한다)
			lstrcpy( smConfig.szServerIP, "127.0.0.1" );
			lstrcpy( smConfig.szDataServerIP, "127.0.0.1" );
			lstrcpy( smConfig.szUserServerIP, "127.0.0.1" );
			lstrcpy( smConfig.szExtendServerIP, "127.0.0.1" );
			smConfig.dwServerPort = wPort;
			smConfig.dwDataServerPort = wPort;
			smConfig.dwUserServerPort = wPort;
			smConfig.dwExtendServerPort = wPort;
		}
	}
#endif



	//SetCurrentDirectory( "E:\\UpdateTest" );

	//크리티칼 섹션 초기화
	InitializeCriticalSection( &cSection );
	InitializeCriticalSection( &cDrawSection );
	InitializeCriticalSection( &cSection_Main );
	

	srand( GetCurrentTime() );		//랜덤 초기화

#ifdef _W_SERVER
	if ( smConfig.WinMode<0 ) {
		return ServerWinMain( hInst,  hPreInst, lpCmdLine, nCmdShow);
	}
#endif
/*
#ifdef	_LANGUAGE_JAPANESE

	LPSTR lpCmdLine2 = StripArg0(GetCommandLine());
	if (lpCmdLine2[0] == 0 && !smConfig.DebugMode )
	{
		if ( strstr( lpCmdLine2 , "twimnet" )==0 )	return TRUE;
		if ( strstr( lpCmdLine2 , "TWINNET" )==0 )	return TRUE;
	}
#endif
*/

	// pluto 비스타에서 채팅 때문에 윈도우 생성전에 사운드 만든다
	InitDirectSound();

	//레지스트리 이름 조작 ..(불법 소프트웨어 단속)
	HaejukReg();

	WinSizeX = smConfig.ScreenSize.x;
	WinSizeY = smConfig.ScreenSize.y;
	WinColBit = smConfig.ScreenColorBit;
	WindowMode = smConfig.WinMode ;

	//if ( WindowMode ) {
		sx = WinSizeX;
		sx = WinSizeY;
/*
	}
	else {
		sx = GetSystemMetrics(SM_CXSCREEN);
		sy = GetSystemMetrics(SM_CYSCREEN);
	}
*/
	//WindowMode = 01;

	//그림자 만들기 테스트
	//mksInit();
	//return TRUE;

//XTrap 시동
#ifdef _XTRAP_GUARD_4_CLIENT
{
//	UINT	ulFuncErr, ulLib, ulErrCode = 0;

	XTrap_C_Start(XTRAP_CONFIG_ARG,NULL);	//XTrapD5
	XTrap_CE1_Func10_Init();				//XTrap Heap 메모리변조 방지

/////////////
//	XTrap_CE1_Func10_Init_D(&ulFuncErr, &ulLib, &ulErrCode);
//	DWORD testFunc10;
//	testFunc10 = XTrap_CE1_Func10_Init_D(&ulFuncErr, &ulLib, &ulErrCode);

//	FILE *fp;
//	fp = fopen("c:\\XTRAPTEST-Func10.txt", "a+t");
//	if(fp)
//	{
//		SYSTEMTIME st;
//		GetLocalTime(&st);
//		fprintf(fp, "%d/%d/%d - %d:%d\t", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute);
//		fprintf(fp, "      Func10 = %d  ulFuncErr = %d  ulLib = %d  ulErrCode = %d\n", testFunc10, ulFuncErr, ulLib, ulErrCode);
//	}
//	fclose(fp);
}
#endif

/*
#ifdef __USE_XTRAP_MANAGER_LOG
		g_XTrap.XTrap_StartUp("game.exe");
#else
		g_XTrap.XTrap_StartUp("game.exe", "127.0.0.1");
#endif
*/

//XTrap v4,0
#ifndef _XTRAP_GUARD_4_
#ifndef _xTrap_GUARD
	//nProtect 실행 //디버그시 동작못하게 막음 kyle
/*
	if ( Start_nProtect(smConfig.DebugMode)==FALSE ) {

		if ( szExitInfo_URL[0] )
			ShellExecute(NULL, NULL, szExitInfo_URL, NULL, NULL, NULL);

		return FALSE;
	}
*/
	sx = WinSizeX;
	sy = WinSizeY;
#else
	if ( WindowMode ) {
		sx = WinSizeX;
		sy = WinSizeY;
	}
	else {
		sx = GetSystemMetrics(SM_CXSCREEN);
		sy = GetSystemMetrics(SM_CYSCREEN);
	}
#endif
#endif

	if (!hPreInst) {

#ifdef PT_GFX_GL
		// OpenGL needs a private device context: with the shared (class) DC the
		// game's own GetDC/ReleaseDC calls on the window recycle the DC the
		// renderer presents through, and the window stays black.
		wndclass.style			=	CS_HREDRAW|CS_VREDRAW|CS_DBLCLKS|CS_OWNDC;
#else
		wndclass.style			=	CS_HREDRAW|CS_VREDRAW|CS_DBLCLKS;
#endif
		wndclass.lpfnWndProc	=	WndProc;
		wndclass.cbClsExtra		=	0;
		wndclass.cbWndExtra		=	0;
		wndclass.hInstance		=	hInst;
		wndclass.hIcon			=	LoadIcon (hInst, MAKEINTRESOURCE(IDI_DEFAULT_ICON) );
//		wndclass.hIcon			=	LoadIcon(NULL, 0);
		wndclass.hCursor		=	LoadCursor(NULL,IDC_ARROW);
#ifdef PT_GFX_GL
		wndclass.hbrBackground	=	NULL;			// OpenGL paints the whole window; no white erase flicker
#else
		wndclass.hbrBackground	=	(HBRUSH)GetStockObject(WHITE_BRUSH);
#endif
		wndclass.lpszMenuName	=	NULL;
		wndclass.lpszClassName	=	szAppName;
		RegisterClass(&wndclass);
	}

#ifdef _WINMODE_DEBUG
	if ( WindowMode ) {
		hwnd=CreateWindow(szAppName,szAppName,
				  WS_OVERLAPPEDWINDOW| WS_POPUP|WS_EX_TOPMOST,CW_USEDEFAULT,
				  CW_USEDEFAULT,sx,sy,NULL,NULL,
				  hInst,NULL);
	}
	else {
		hwnd=CreateWindow(szAppName,szAppName,
				  WS_VISIBLE| WS_POPUP|WS_EX_TOPMOST,CW_USEDEFAULT,
				  CW_USEDEFAULT,sx,sy,NULL,NULL,
				  hInst,NULL);
	}
#else

#ifdef	_LANGUAGE_TAIWAN
	hwnd=CreateWindow(szAppName,szAppName,
			  WS_VISIBLE| WS_POPUP|WS_EX_TOPMOST,CW_USEDEFAULT,
			  CW_USEDEFAULT,sx,sy,NULL,NULL,
			  hInst,NULL);
#endif
/*
#ifdef	_LANGUAGE_CHINESE //중국 전체모드만
	hwnd=CreateWindow(szAppName,szAppName,
			  WS_VISIBLE| WS_POPUP|WS_EX_TOPMOST,CW_USEDEFAULT,
			  CW_USEDEFAULT,sx,sy,NULL,NULL,
			  hInst,NULL);
#endif
*/
//중국 창모드 지원
#ifdef	_LANGUAGE_CHINESE	//윈도우 모드
	if ( WindowMode )
	{
		hwnd=CreateWindow(szAppName,szAppName,
				  WS_OVERLAPPEDWINDOW| WS_POPUP|WS_EX_TOPMOST,CW_USEDEFAULT,
				  CW_USEDEFAULT,sx,sy,NULL,NULL,
				  hInst,NULL);
	}
	else
	{
		hwnd=CreateWindow(szAppName,szAppName,
				  WS_VISIBLE| WS_POPUP|WS_EX_TOPMOST,CW_USEDEFAULT,
				  CW_USEDEFAULT,sx,sy,NULL,NULL,
				  hInst,NULL);
	}
#endif
#ifdef	_LANGUAGE_ENGLISH	//윈도우 모드
	if ( WindowMode ) {
		hwnd=CreateWindow(szAppName,szAppName,
				  WS_OVERLAPPEDWINDOW| WS_POPUP|WS_EX_TOPMOST,CW_USEDEFAULT,
				  CW_USEDEFAULT,sx,sy,NULL,NULL,
				  hInst,NULL);
	}
	else {
		hwnd=CreateWindow(szAppName,szAppName,
				  WS_VISIBLE| WS_POPUP|WS_EX_TOPMOST,CW_USEDEFAULT,
				  CW_USEDEFAULT,sx,sy,NULL,NULL,
				  hInst,NULL);
	}
#endif
#ifdef	_LANGUAGE_THAI
	hwnd=CreateWindow(szAppName,szAppName,
			  WS_VISIBLE| WS_POPUP|WS_EX_TOPMOST,CW_USEDEFAULT,
			  CW_USEDEFAULT,sx,sy,NULL,NULL,
			  hInst,NULL);
#endif

#ifdef _LANGUAGE_BRAZIL	
	hwnd=CreateWindow(szAppName,szAppName,
				  WS_VISIBLE| WS_POPUP|WS_EX_TOPMOST,CW_USEDEFAULT,
				  CW_USEDEFAULT,sx,sy,NULL,NULL,
				  hInst,NULL);
/*
	if ( WindowMode )
	{
		hwnd=CreateWindow(szAppName,szAppName,
				  WS_OVERLAPPEDWINDOW| WS_POPUP|WS_EX_TOPMOST,CW_USEDEFAULT,
				  CW_USEDEFAULT,sx,sy,NULL,NULL,
				  hInst,NULL);
	}
	else
	{
		hwnd=CreateWindow(szAppName,szAppName,
				  WS_VISIBLE| WS_POPUP|WS_EX_TOPMOST,CW_USEDEFAULT,
				  CW_USEDEFAULT,sx,sy,NULL,NULL,
				  hInst,NULL);
	}
*/
#endif
#ifdef _LANGUAGE_ARGENTINA
	hwnd=CreateWindow(szAppName,szAppName,
				  WS_VISIBLE| WS_POPUP|WS_EX_TOPMOST,CW_USEDEFAULT,
				  CW_USEDEFAULT,sx,sy,NULL,NULL,
				  hInst,NULL);
/*
	if ( WindowMode )
	{
		hwnd=CreateWindow(szAppName,szAppName,
				  WS_OVERLAPPEDWINDOW| WS_POPUP|WS_EX_TOPMOST,CW_USEDEFAULT,
				  CW_USEDEFAULT,sx,sy,NULL,NULL,
				  hInst,NULL);
	}
	else
	{
		hwnd=CreateWindow(szAppName,szAppName,
				  WS_VISIBLE| WS_POPUP|WS_EX_TOPMOST,CW_USEDEFAULT,
				  CW_USEDEFAULT,sx,sy,NULL,NULL,
				  hInst,NULL);
	}
*/
#endif
#ifdef	_LANGUAGE_KOREAN
	if ( WindowMode ) {
		hwnd=CreateWindow(szAppName,szAppName,
				  WS_OVERLAPPEDWINDOW| WS_POPUP|WS_EX_TOPMOST,CW_USEDEFAULT,
				  CW_USEDEFAULT,sx,sy,NULL,NULL,
				  hInst,NULL);
	}
	else {
		hwnd=CreateWindow(szAppName,szAppName,
				  WS_VISIBLE| WS_POPUP|WS_EX_TOPMOST,CW_USEDEFAULT,
				  CW_USEDEFAULT,sx,sy,NULL,NULL,
				  hInst,NULL);
	}
#endif

#ifdef	_LANGUAGE_JAPANESE
	if ( WindowMode ) {
		hwnd=CreateWindow(szAppName,szAppName,
				  WS_OVERLAPPEDWINDOW| WS_POPUP|WS_EX_TOPMOST,CW_USEDEFAULT,
				  CW_USEDEFAULT,sx,sy,NULL,NULL,
				  hInst,NULL);
	}
	else {
		hwnd=CreateWindow(szAppName,szAppName,
				  WS_VISIBLE| WS_POPUP|WS_EX_TOPMOST,CW_USEDEFAULT,
				  CW_USEDEFAULT,sx,sy,NULL,NULL,
				  hInst,NULL);
	}
#endif
#ifdef _LANGUAGE_VEITNAM	//윈도우 모드
	if ( WindowMode ) {
		hwnd=CreateWindow(szAppName,szAppName,
				  WS_OVERLAPPEDWINDOW| WS_POPUP|WS_EX_TOPMOST,CW_USEDEFAULT,
				  CW_USEDEFAULT,sx,sy,NULL,NULL,
				  hInst,NULL);
	}
	else {
		hwnd=CreateWindow(szAppName,szAppName,
				  WS_VISIBLE| WS_POPUP|WS_EX_TOPMOST,CW_USEDEFAULT,
				  CW_USEDEFAULT,sx,sy,NULL,NULL,
				  hInst,NULL);
	}
#endif

#endif


#ifdef _npGAME_GUARD
	//ShowWindow(hwnd,SW_HIDE);

	//nProtect 실행 //디버그시 동작못하게 막음 kyle
/*
	if ( Start_nProtect(smConfig.DebugMode)==FALSE ) {
		if ( szExitInfo_URL[0] )
			ShellExecute(NULL, NULL, szExitInfo_URL, NULL, NULL, NULL);
		return FALSE;
	}
*/

	while (1) {
	    if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))  {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
	    }
		else
			break;
	}

	MoveWindow( hwnd , 0,0, WinSizeX , WinSizeY , FALSE );

#endif

//박재원 - XignCode // 장별 - 싸인코드 디버깅 하려면 ZSOPT_USESYSGUARD 빼준다
#ifdef _XIGNCODE_CLIENT
//	if(!ZCWAVE_SysEnter(("_97cpcxcIB3A"),"Xigncode", ZSOPT_USESYSGUARD | ZSOPT_SHOWSPLASH | ZSOPT_SHOWTRAY)) // 장별 - 싸인코드3
	if(!ZCWAVE_SysEnter(("_97cpcxcIB3A"),_T("Xigncode"), ZSOPT_USESYSGUARD )) // 장별 - 싸인코드3
//	if(!ZCWAVE_SysEnter(("_97cpcxcIB3A"),"Xigncode", ZSOPT_SHOWSPLASH | ZSOPT_SHOWTRAY))
	{
		quit = 1;	
	}
#endif


#ifdef _XTRAP_GUARD_4_CLIENT
	XTrap_C_KeepAlive();	//XTrapD5
#endif

#ifdef PT_USE_SDL
	// *PLATFORM SDL in hotuk.ini: SDL takes over the GL context and input (ptplatform.cpp)
	PtPlat_Attach( hwnd, (WNDPROC)WndProc );
#endif

	ShowWindow(hwnd,nCmdShow);
	UpdateWindow(hwnd);

	ShowCursor( FALSE );

	//v3wshook.dll
	//v3wsui.dll
	//FreeLibrary();
/*
	HMODULE	hMod;
	hMod = GetModuleHandle( "v3wshook.dll" );
	FreeLibrary( hMod );
*/

#ifdef _LANGUAGE_WINDOW
	if ( WindowMode )
	{
		HWND		hImsi1=GetDesktopWindow();
		RECT		rImsi1;
		GetWindowRect(hImsi1,&rImsi1);
		//바탕화면이 작을 때만 800x600 으로 (예전 코드는 .top 을 비교해서 항상 800x600 이었음)
		if( (rImsi1.bottom-rImsi1.top)<=768 )
		{
			WinSizeX = 800;
			WinSizeY = 600;
		}
	}
#endif

	if ( InitD3D( hwnd )==NULL ) return FALSE;

	//Check_PlaySubReleaseFunc();
	//nProtect 실행

	//로딩램프
	InitLoadingLamp( hwnd );

	hTextWnd=CreateWindow( "EDIT", "",
			  WS_CHILD|WS_BORDER|ES_LEFT|ES_MULTILINE|ES_WANTRETURN,
			  10,400,500,40,
			  hwnd , (HMENU)IDC_TEXT,
			  hInst,NULL);

	OldEditProc01 = (WNDPROC)SetWindowLong(hTextWnd,GWL_WNDPROC,(LONG)EditWndProc01);
	IMETest.SetHWND(hTextWnd);

	hImc = ImmGetContext( hwnd );
	hImcEdit = ImmGetContext( hTextWnd );

	SetIME_Mode( 0 );		//IME 모드 전환

	//폰트 생성
	SetCreateFont();


	TextMode = 0;

	InitGameSocket();
	//InitBindSock( TCP_GAMEPORT );

	//내부자 IP검사
	char *szIP;
	int cnt;
	int ChkIp;

	if ( smConfig.DebugMode ) {
		szIP = smGetMyIp();
		if ( szIP ) {
			cnt = 0;
			ChkIp = 0;
			while(1) {
				//운영자용 IP 검사
				if ( !szOperationIP[cnt] ) break;
				if ( strstr( szIP , szOperationIP[cnt] )!=0 ) {
					ChkIp++;
					break;
				}
				cnt++;
			}
			if ( !ChkIp && smConfig.DebugMode==TRUE ) {
				smConfig.DebugMode=0;
			}
		}
		if ( smConfig.DebugMode>1 ) smConfig.DebugMode=TRUE;
	}
	else {
		DeleteFile( "ddd.txt" );			//클랜 디버그 파일 삭제
	}

	SetGameMode( 1 );
	CharacterName1[0] = 0;
	CharacterName2[0] = 0;

	//타이머 인터럽트 가동 ( 실제 20초가 게임에서의 1분 )
	SetTimer( hwnd , 0 , GAME_WORLDTIME_MIN/4 , NULL );

	//nProtect 확인
	//if ( Check_nProtectDLL()==FALSE ) quit = TRUE;

#ifdef _LANGUAGE_WINDOW
	if ( WindowMode )
	{
		//창 크기를 게임 해상도에 맞추고 화면 가운데에 둔다 (예전에는 바탕화면 전체를 덮었음)
		RECT		rImsi = { 0, 0, WinSizeX, WinSizeY };
		AdjustWindowRect( &rImsi, GetWindowLong( hwnd, GWL_STYLE ), FALSE );
		int			winW = rImsi.right-rImsi.left;
		int			winH = rImsi.bottom-rImsi.top;
		int			winX = ( GetSystemMetrics(SM_CXSCREEN)-winW )/2;
		int			winY = ( GetSystemMetrics(SM_CYSCREEN)-winH )/2;
		if ( winX<0 ) winX = 0;
		if ( winY<0 ) winY = 0;
		SetWindowPos(hwnd,HWND_TOP,winX,winY,winW,winH,SWP_SHOWWINDOW);
		//SetWindowPos(hwnd,HWND_BOTTOM,0,0,WinSizeX+25,WinSizeY+25,SWP_SHOWWINDOW);
	}
#endif

#ifndef _WINMODE_DEBUG

#ifdef	_LANGUAGE_TAIWAN
	if ( WindowMode ) {
		return FALSE;
	}
#endif
//중국
/*
#ifdef	_LANGUAGE_CHINESE	//윈도우 모드
	if ( WindowMode )
	{
		return FALSE;
	}

#endif
*/
//태국
#ifdef	_LANGUAGE_THAI
	if ( WindowMode ) {
		return FALSE;
	}
#endif
//영어, 필리핀	//윈도우 모드
	/*
#ifdef	_LANGUAGE_ENGLISH
#ifndef _LANGUAGE_PHILIPIN
	if ( WindowMode ) {
		return FALSE;
	}
#endif
#endif
	*/
//브라질
#ifdef	_LANGUAGE_BRAZIL
	if ( WindowMode ) {
		return FALSE;
	}
#endif
//아르헨티나
#ifdef	_LANGUAGE_ARGENTINA
	if ( WindowMode ) {
		return FALSE;
	}
#endif
/*
#ifdef	_LANGUAGE_VEITNAM	//윈도우 모드
	if ( WindowMode ) {
		return FALSE;
	}
#endif
*/

#endif

	//욕설 필터링
#ifdef	_LANGUAGE_KOREAN
	if ( LoadCurses("Image\\Curse.cht")!=-81004 ) return FALSE;
#endif
#ifdef	_LANGUAGE_THAI
	if ( LoadCurses("Image\\thaicurse.cht")!=98085 ) return FALSE;
#endif
#ifdef	_LANGUAGE_TAIWAN
	if ( LoadCurses("Image\\taiwan_curse.cht")!=-19406 ) return FALSE;
	if ( LoadCursesID("Image\\taiwan_BadID.cht")!=-33585 ) return FALSE;
#endif
#ifdef	_LANGUAGE_BRAZIL
	if ( LoadCurses("Image\\brazil_curse.cht")!=240 ) return FALSE;
	if ( LoadCursesID("Image\\brazil_curse.cht")!=240 ) return FALSE;
#endif
#ifdef	_LANGUAGE_ARGENTINA
//	if ( LoadCurses("Image\\argentina_curse.cht")!=240 ) return FALSE;
//	if ( LoadCursesID("Image\\argentina_curse.cht")!=240 ) return FALSE;
#endif
#ifdef	_LANGUAGE_CHINESE
	if ( LoadCurses("Image\\chinacurse.cht")!=-92461 ) return FALSE;
	if ( LoadCursesID("Image\\chinacurse.cht")!=-92461 ) return FALSE;
#endif
#ifdef	_LANGUAGE_JAPANESE
	if ( LoadCurses("Image\\japancurse.cht")!=232284 ) return FALSE;
	if ( LoadCursesID("Image\\japancurse.cht")!=232284 ) return FALSE;
#endif


//					if ( wParam == VK_NEXT && VRKeyBuff[wParam]==0 ) {
//					if ( wParam == VK_PRIOR && VRKeyBuff[wParam]==0 ) {
	/*
      // Simulate a key press
         keybd_event( VK_NEXT,
                      0x45,
                      KEYEVENTF_EXTENDEDKEY | 0,
                      0 );

      // Simulate a key release
         keybd_event( VK_NEXT,
                      0x45,
                      KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP,
                      0);

*/
#ifdef _LANGUAGE_WINDOW
		SetForegroundWindow( hwnd );
#endif


	while (1) 
	  {

		//EnterCriticalSection( &cSection_Main );			//크리티칼 섹션 선언

#ifdef PT_USE_SDL
		if ( PtPlat_IsSDL() ) {
			// SDL: pump everything (Win32 messages are dispatched inside), then one frame
			PtPlat_PumpEvents();
			EnterCriticalSection( &cSection_Main );
			PlayD3D();
			PlayRecvMessageQue();
			LeaveCriticalSection( &cSection_Main );
		}
		else
#endif
	    if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
	    {
	      TranslateMessage(&msg);

		  if ( msg.message==WM_SYSKEYDOWN || msg.message==WM_SYSKEYUP ) {
			  if ( msg.wParam!=0xF4 && msg.wParam!=VK_F10 ) continue;		//일본어 가나키 제외
		  }

	      DispatchMessage(&msg);
	    }
		else {
//			if ( msg.message == WM_QUIT ) break;

			EnterCriticalSection( &cSection_Main );			//크리티칼 섹션 선언

			PlayD3D();
			//메세지 큐의 메세지를 처리
			PlayRecvMessageQue();

			LeaveCriticalSection( &cSection_Main );			//크리티칼 섹션 해제
		}

		//LeaveCriticalSection( &cSection_Main );			//크리티칼 섹션 해제

		if (quit!=0 && !dwTradeMaskTime )  {
			if ( GameMode!=2 ) break;
	
			if ( smWsockServer && !QuitSave ) {
				if ( cTrade.OpenFlag ) {
					//거래중이면 거래 종료
					SendRequestTrade( cTrade.TradeCharCode , 3 );	//거래 취소 요구
					cTrade.CancelTradeItem();						//거래창 닫기
				}

				if ( cWareHouse.OpenFlag ) {
					//창고를 닫는다 
					cWareHouse.RestoreInvenItem(); //인벤토리 복구
					//cWareHouse.OpenFlag = FALSE;
				}

				cInvenTory.ResetMouseItem();	//들고 있는 아이템 복구

				SaveGameData();				//게임 저장하고 나가기
				QuitSave = TRUE;
			}
			else {
				if ( !smWsockServer || GetSaveResult()==TRUE ) break;
				if (  (dwPlayTime - rsRecorder.dwRecordTime)>15000 ) break;
			}
		}
	}

	SetGameMode( 0 );

	//폰트 제거
	DeleteObject( hFont );
	//로딩램프 제거
	CloseLoadingLamp();

	CloseBindSock();

	CloseD3d();

	if ( fpNetLog ) fclose( fpNetLog );

	//nProtect 제거
	Remove_nProtect();

	if ( szExitInfo_URL[0] )
		ShellExecute(NULL, NULL, szExitInfo_URL, NULL, NULL, NULL);

	return msg.wParam;
}

int msX=0,msY=0;
int msXo=0,msYo=0;
int xPs;
int yPs;

POINT	pCursorPos;
POINT	pRealCursorPos;
//게임에서의 윈도 메세지 처리
DWORD GameWindowMessage( HWND hWnd,UINT messg,WPARAM wParam,LPARAM lParam );
DWORD dwTimerCount =0;

/*
BOOL cbutton=true;


BOOL FoundKeyboard(HWND hWnd,int mode)
{
	LPTSTR check;
	//char szDesc[128];	
	//char szKLID[128];	
	int kk=0;
	HWND	hOldWnd;

	//UINT uiRet;
	HKL hkl;

	hOldWnd = GetFocus();
	if ( hWnd && hOldWnd!=hWnd ) SetFocus(hWnd);

	//찾고 싶은 언어 스크립트 추가...
	//if(mode==0) check = ""; 영문 
	if(mode==1) check = "한글";
	else if(mode==2) check = "Japanese";
	else if(mode==3) check = "入法";

	//memset(szKLID,0x00,128);
	//memset(szDesc,0x00,128);
	//GetKeyboardLayoutName(szKLID);
	//hkl = LoadKeyboardLayout(szKLID,KLF_ACTIVATE);	
	//ImmGetDescription(hkl,szDesc,256);

	//if((mode!=0)&&(strstr(szDesc,check)!=0)) {
	//	if ( hWnd!=hOldWnd ) SetFocus(hOldWnd);
	//	return 1;
	//}
	//else if(mode==0&&szDesc[0]==0&&szDesc[0]==0&&szDesc[0]==0&&szDesc[0]==0) {
	//	if ( hWnd!=hOldWnd ) SetFocus(hOldWnd);
	//	return 1; 
	//}
	hkl = ActivateKeyboardLayout((HKL)HKL_PREV,KLF_REORDER);
	hkl = ActivateKeyboardLayout((HKL)HKL_NEXT,KLF_REORDER);
	if ( hWnd!=hOldWnd ) SetFocus(hOldWnd);
	return 0;
}

//중문 모드에서 영문/중문 입력 선택
void z_SetImeMode(HWND hWnd,BOOL mode)
{	

#ifdef _LANGUAGE_CHINESE
	HIMC  hIMC;
	DWORD dwConv, dwSent;
	DWORD dwTemp;

	hIMC = ImmGetContext(hWnd);
	ImmGetConversionStatus(hIMC,&dwConv,&dwSent);

	dwTemp = dwConv & ~IME_CMODE_LANGUAGE;
	if(!mode) dwTemp |= (IME_CMODE_NATIVE | IME_CMODE_KATAKANA);	
	dwConv = dwTemp;	
	ImmSetConversionStatus(hIMC,dwConv,dwSent);
			
	//현재 입력 모드가 무엇인가 
	//if((IME_CMODE_LANGUAGE&dwConv) == IME_CMODE_ALPHANUMERIC) SetDlgItemText(z_hWnd,ID_BUTTON1,"영문"); 
	//if((IME_CMODE_LANGUAGE&dwConv) == (IME_CMODE_NATIVE | IME_CMODE_KATAKANA)) SetDlgItemText(z_hWnd,ID_BUTTON1,"한글중문");
	//EnableWindow(GetDlgItem(z_hWnd,ID_BUTTON2),FALSE);  //영문모드 		
	ImmReleaseContext(hWnd,hIMC);

#endif

}

//IME 상태창 위치
void z_SetImeStatusPos(HWND hWnd,int x,int y)
{
	HIMC  hIMC;
	hIMC = ImmGetContext(hWnd);
	POINT pos;
	pos.x = x;
	pos.y = y;
	ImmSetStatusWindowPos(hIMC,&pos);
}
//IME 상황에서 입력창의 위치
void z_SetImeStatusPos(HWND hWnd,int mode,int x,int y,int top,int left,int botton,int right)
{
	HIMC  hIMC;
	COMPOSITIONFORM compos;
	RECT  rt;
	POINT pos;

	if(mode<=1) compos.dwStyle = CFS_DEFAULT;
	else if(mode==2) compos.dwStyle = CFS_FORCE_POSITION;
	else if(mode==3) compos.dwStyle = CFS_POINT;
	else if(mode>=4) compos.dwStyle = CFS_RECT;
		
	pos.x = x;
	pos.y = y;

	rt.top = top;
	rt.left = left;
	rt.bottom = botton;
	rt.right = right;

	compos.ptCurrentPos = pos;
	compos.rcArea = rt;

	hIMC = ImmGetContext(hWnd);
	ImmSetCompositionWindow(hIMC,&compos);
}
//IME상황에서 내용창의 위치
void z_SetImeCandidatePos(HWND hWnd,DWORD page,BOOL mode,int x,int y,int top,int left,int botton,int right)
{
	HIMC  hIMC;	
	CANDIDATEFORM candidat;
	RECT  rt;
	POINT pos;

	candidat.dwIndex = page;

	if(mode) candidat.dwStyle = CFS_CANDIDATEPOS;
	else candidat.dwStyle = CFS_EXCLUDE;

	pos.x = x;
	pos.y = y;

	rt.bottom = botton;
	rt.left = left;
	rt.right = right;
	rt.top = top;

	candidat.ptCurrentPos = pos;
	candidat.rcArea = rt;

	hIMC = ImmGetContext(hWnd);
	ImmSetCandidateWindow(hIMC,&candidat);
}
*/

//IME 모드 전환
BOOL DisplayIME=FALSE;
int SetIME_Mode( BOOL mode )
{
	if ( mode ) {
		DisplayIME=TRUE;
		ImmSetOpenStatus(hImc,TRUE);
#ifdef	_LANGUAGE_JAPANESE
/*

		DWORD	conv , sent;
		//일본판
		ImmGetConversionStatus( hImc , &conv , &sent );
		ImmSetConversionStatus( hImc , IME_CMODE_NATIVE|IME_CMODE_FULLSHAPE,sent );

		ImmGetConversionStatus( hImcEdit , &conv , &sent );
		ImmSetConversionStatus( hImcEdit , IME_CMODE_NATIVE|IME_CMODE_FULLSHAPE,sent );
*/
		//일본판
		ImmSetConversionStatus( hImc , IME_CMODE_NATIVE|IME_CMODE_FULLSHAPE,IME_SMODE_PHRASEPREDICT );
		ImmSetConversionStatus( hImcEdit , IME_CMODE_NATIVE|IME_CMODE_FULLSHAPE,IME_SMODE_PHRASEPREDICT );


#else
		//한국판
		ImmSetConversionStatus( hImc , IME_CMODE_NATIVE,IME_CMODE_NATIVE );
		ImmSetConversionStatus( hImcEdit , IME_CMODE_NATIVE,IME_CMODE_NATIVE );
#endif

	}
	else {
		ImmSetConversionStatus( hImc , IME_CMODE_ALPHANUMERIC,IME_CMODE_ALPHANUMERIC );
		ImmSetConversionStatus( hImcEdit , IME_CMODE_ALPHANUMERIC,IME_CMODE_ALPHANUMERIC );

		ImmSetOpenStatus(hImc,FALSE);
		DisplayIME=FALSE;
	}


	return TRUE;
}

//2바이트 코드 확인
int CheckCode_2Byte( char *Str )
{
#ifdef _LANGUAGE_ENGLISH
	//영문판 - 무조건 1

	return 1;
#endif

#ifdef _LANGUAGE_THAI
	if(CheckTHAI_ptr(Str,0)==1) return 1;
    else if(CheckTHAI_ptr(Str,0)==2) return 2;
	return 1;
#endif

#ifdef _LANGUAGE_JAPANESE

	//일본코드 확인

	if ( CheckJTS_ptr(Str,0)==2 ) {
		return 2;
	}
/*

	BYTE	ch;

	ch = (BYTE)Str[0];

	if ( ch>=0x81 && ch<=0x9F ) return 2;
	if ( ch>=0xE0 && ch<=0xEF ) return 2;
*/
#else
	//한국 중국 코드
	if ( Str[0]<0 ) {
		return 2;
	}
	
#endif

	return 1;
}



LRESULT CALLBACK EditWndProc01(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	wmId    = LOWORD(wParam); 
	wmEvent = HIWORD(wParam); 			

	switch (message) 
	{
		case WM_KEYDOWN:
			//SetWindowText( hWnd , "" );

#ifdef	_IME_ACTIVE
			IMETest.GetInfoPerTime();
#endif

/*
			for(int cnt=0;cnt<10;cnt++) {
				lstrcpy( szIME_Buff[cnt] , IMETest.m_szCandList[cnt].GetBuffer( IMETest.m_szCandList[cnt].GetLength() ) );
			}
*/
			break;
   }
   return CallWindowProc(OldEditProc01,hWnd,message,wParam,lParam);
}



//////////////////////// [ 일본어 IME 처리 프로시저 ] ///////////////////////////////
#ifdef	_LANGUAGE_JAPANESE

char g_bufEdit[256];		
char g_bufFixed[256];

int WndProc_Japanese(HWND hWnd,UINT messg,WPARAM wParam,LPARAM lParam)
{
	HIMC	imc;
	static DWORD conv = IME_CMODE_NATIVE | IME_CMODE_FULLSHAPE | IME_CMODE_ROMAN;
	static DWORD sent = IME_SMODE_PHRASEPREDICT;
	static BOOL setFont = FALSE;

	switch (messg)
	{

		case WM_IME_COMPOSITION:

			if ((imc = ImmGetContext(hWnd))) {
				if (lParam & GCS_RESULTSTR) {
					ZeroMemory(g_bufFixed, 256);
					ImmGetCompositionString(imc, GCS_RESULTSTR, g_bufFixed, 256);
				}
				else if (lParam & GCS_COMPSTR) {
					ZeroMemory(g_bufEdit, 256);
					ImmGetCompositionString(imc, GCS_COMPSTR, g_bufEdit, 256);
				}
				ImmReleaseContext(hWnd, imc);
			}
			break;

		case WM_IME_NOTIFY:
			switch (wParam) {
			case IMN_SETCONVERSIONMODE:
			case IMN_SETSENTENCEMODE:
				if ((imc = ImmGetContext(hWnd))) {
					ImmGetConversionStatus(imc, &conv, &sent);
					ImmReleaseContext(hWnd, imc);
				}
				break;

			case IMN_SETOPENSTATUS:
				if ((imc = ImmGetContext(hWnd))) {
					if (ImmGetOpenStatus(imc)) {
						ImmSetConversionStatus(imc, conv, sent);
					}
					else {
						ImmGetConversionStatus(imc, &conv, &sent);
					}
					ImmReleaseContext(hWnd, imc);
				}

				if (!setFont && (imc = ImmGetContext(hWnd))) {
					LOGFONT lf;
					ZeroMemory(&lf, sizeof(LOGFONT));
					lf.lfHeight = 12;//-MulDiv(10, GetDeviceCaps(hDC, LOGPIXELSY), 72);
					lf.lfItalic = FALSE;
					lf.lfUnderline = FALSE;
					lf.lfStrikeOut = FALSE;
					lf.lfCharSet = OEM_CHARSET;//SHIFTJIS_CHARSET;
					lf.lfOutPrecision = OUT_TT_ONLY_PRECIS;
					lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
					lf.lfQuality = DEFAULT_QUALITY;
					lf.lfPitchAndFamily = VARIABLE_PITCH | FF_ROMAN;
					strcpy(lf.lfFaceName, _T("\202\154\202\162 \202\157\203\123\203\126\203\142\203\116"));
					ImmSetCompositionFont(imc, &lf);
					ImmReleaseContext(hWnd, imc);
					setFont = TRUE;
				}
				break;

			case IMN_OPENCANDIDATE:
			case IMN_CHANGECANDIDATE:
				if ((imc = ImmGetContext(hWnd))) {
					CANDIDATEFORM cf;
					ZeroMemory(&cf, sizeof(CANDIDATEFORM));
					cf.dwStyle = CFS_CANDIDATEPOS;
					cf.ptCurrentPos.x = 100000;//GetSystemMetrics(SM_CXSCREEN);
					cf.ptCurrentPos.y = 100000;//GetSystemMetrics(SM_CYSCREEN);
					ImmSetCandidateWindow(imc, &cf);
					ImmReleaseContext(hWnd, imc);
				}
				break;
			}
			break;
	}

	return TRUE;
}
#endif

//ktj 
extern void clan_Mouse(int msg,WPARAM wParam);


LONG APIENTRY WndProc(HWND hWnd,UINT messg,WPARAM wParam,LPARAM lParam)
{

//	PAINTSTRUCT ps;
//	HDC	hdc;
	short zDelta;
	char strBuff[256];
	char strBuff2[256];
	char *lpString;
	int	len,CrCode;
/*
#ifdef	_LANGUAGE_JAPANESE
	//if ( messg==WM_IME_COMPOSITION || messg==WM_IME_NOTIFY ) {
		WndProc_Japanese( hWnd, messg, wParam, lParam);		//일본어 처리 프로시저
	//}
#endif
*/

	switch (messg)
	{
		//박철호 : voice채팅용 ==================================================
		case WM_CALLMEMMAP:    //vrun.dat 메모리맵에 메세지
		void PacketParsing();	//process.cpp
		PacketParsing();
		break;

		case WM_CREATE:
			break;

		case WM_CLOSE:
			//게임 종료
			QuitGame();
			break;

		case WSA_ACCEPT:
			WSAMessage_Accept( wParam , lParam );
			break;   

		case WSA_READ:
			WSAMessage_Read( wParam , lParam );
			break;      

		case SWM_RECVSUCCESS:
//				SendMessage( hwnd , SWM_RECVSUCCESS , (WPARAM)SockInfo , 0 );
//			SockInfo = (smTHREADSOCK *)wParam;
			RecvPlayData( (smTHREADSOCK *)wParam );
			break;

		case SWM_MCONNECT:
			//메세지 쓰레드에서 대신 연결
			ConnectOtherPlayer( wParam );
			break;

		case SWM_MDISCONNECT:
			//메세지 쓰레드에서 대신 끈음
			((smWINSOCK *)wParam)->CloseSocket();
			break;

		//case WM_IME_KEYDOWN:
		case WM_KEYDOWN :

			//######################################################################################
			//작 성 자 : 오 영 석
			if( g_IsDxProjectZoomIn > 0 )
			{
				if( KeyFullZoomMap(wParam) )
				{
					VRKeyBuff[ wParam ] = 1;
                    break;
				}
				else
				{
					SetFullZoomMap(0);
					pCursorPos.x = g_iFzmCursorPosX;
					pCursorPos.y = g_iFzmCursorPosY;

					if( wParam == VK_ESCAPE )
						break;
				}
			}
			//######################################################################################


		
			//////////////박철호
#ifdef PARKMENU
			/////////펑션키  Key Down
			//if ( GameMode==2 )
			if( wParam == 0x47 )
			{
				if(vrunRuning==TRUE)
				{
					if(bMic==FALSE)
					{
						if(micInit==0)
						{
							micInit = cmicvol.Init();
						}
						bMic = TRUE;
						void ParkSetMicOnOFF();
						ParkSetMicOnOFF();
					}
				}
			}
			//}
#endif
			//XTRAP HEAP MEMORY TEST
//			if(wParam == 0x6c || wParam == 0x4b)
//			{
//				sinChar->Life[0] -= 100;
//				ReformCharForm();
//			}
//			if(wParam == 0x70 || wParam == 0x50)
//			{
//				sinChar->Stamina[0] -= 100;
//				ReformCharForm();
//			}


			//ktj
			if(wParam == VK_RETURN )keydownEnt=1;

			if ( wParam==VK_CONTROL ) VRKeyBuff[ wParam ] = 1;

			if ( !hFocusWnd ) {
				//######################################################################################
				//작 성 자 : 오 영 석
				//if ( VRKeyBuff[VK_CONTROL] && !sinMessageBoxShowFlag && VRKeyBuff[wParam]==0 ) {
				if ( ! VRKeyBuff[VK_SHIFT] && VRKeyBuff[VK_CONTROL] && !sinMessageBoxShowFlag && VRKeyBuff[wParam]==0 ) {
				//######################################################################################
					if ( wParam==VK_F1 || wParam==VK_F2 || wParam==VK_F3 || wParam==VK_F4 || wParam==VK_F5 || 
						wParam==VK_F6 || wParam==VK_F7 || wParam==VK_F8 || wParam==VK_F9 || wParam==VK_F10 ) {

							SetChatingLine( "" );

						}
				}
			}

			if ( hFocusWnd ) { 
				lpString = 0;
				CrCode = 0;

				//######################################################################################
				//작 성 자 : 오 영 석
				//if ( VRKeyBuff[VK_CONTROL] && !sinMessageBoxShowFlag ) {
				if ( ! VRKeyBuff[VK_SHIFT] && VRKeyBuff[VK_CONTROL] && !sinMessageBoxShowFlag ) {
				//######################################################################################
					if ( wParam==VK_F1  && szShotCutMessage[1][0] ) lpString=szShotCutMessage[1];
					if ( wParam==VK_F2  && szShotCutMessage[2][0] ) lpString=szShotCutMessage[2];
					if ( wParam==VK_F3  && szShotCutMessage[3][0] ) lpString=szShotCutMessage[3];
					if ( wParam==VK_F4  && szShotCutMessage[4][0] ) lpString=szShotCutMessage[4];
					if ( wParam==VK_F5  && szShotCutMessage[5][0] ) lpString=szShotCutMessage[5];
					if ( wParam==VK_F6  && szShotCutMessage[6][0] ) lpString=szShotCutMessage[6];
					if ( wParam==VK_F7  && szShotCutMessage[7][0] ) lpString=szShotCutMessage[7];
					if ( wParam==VK_F8  && szShotCutMessage[8][0] ) lpString=szShotCutMessage[8];
					if ( wParam==VK_F9  && szShotCutMessage[9][0] ) lpString=szShotCutMessage[9];
					if ( wParam==VK_F10 && szShotCutMessage[0][0] ) lpString=szShotCutMessage[0];

					if ( lpString ) {
#ifdef	_LANGUAGE_VEITNAM
						GetWindowText( hFocusWnd , strBuff2 , 80 );
#else
						GetWindowText( hFocusWnd , strBuff2 , 90 );
#endif
						len = lstrlen(strBuff2)+lstrlen(lpString);
						if ( len<80 ) {
							lstrcat( strBuff2 , lpString );
							if ( strBuff2[len-1]==0x0D ) {
								len--;
								strBuff2[len] = 0;
								CrCode = TRUE;
							}
							SetWindowText( hFocusWnd , strBuff2 );
							SendMessage( hFocusWnd , EM_SETSEL , len , len );
						}
					}
				}
				if ( !sinMessageBoxShowFlag &&
					((wParam == VK_RETURN && VRKeyBuff[wParam]==0 && !VRKeyBuff[VK_CONTROL]) || CrCode==TRUE ) ) {

#ifdef	_LANGUAGE_VEITNAM
					GetWindowText( hFocusWnd , strBuff2 , 80 );
#else
					GetWindowText( hFocusWnd , strBuff2 , 90 );
#endif
					cHelpPet.PetOnOff(strBuff2);   //<==== 요거추가해주세요

					if ( strBuff2[0] ) {
						if ( strBuff2[0]=='/' || (smConfig.DebugMode && strBuff2[0]=='~') || strBuff2[0]=='`' || strBuff2[0]=='@' )
							wsprintf( strBuff , "%s" , strBuff2 );
						else {
							switch( DispChatMode ) {
							case 0:		//일반
							case 4:		//귓말
#ifdef	_LANGUAGE_VEITNAM
     						    wsprintf( strBuff , "%s: %s" , PlayerName , strBuff2 );
#else
								wsprintf( strBuff , "%s: %s" , PlayerName , strBuff2 );
#endif
								break;
							case 1:		//클랜
								wsprintf( strBuff , "/CLAN> %s", strBuff2 );
								break;
							case 2:		//동료
								wsprintf( strBuff , "@%s", strBuff2 );
								break;
							case 3:		//거래
								wsprintf( strBuff , "/TRADE> %s", strBuff2 );
								break;
							}
						}
/*
						//######################################################################################
						//작 성 자 : 오 영 석
						if( smConfig.DebugMode )
						{
							int   Alpha;
							char  szBuff[64] = {0,};
							char *pos = GetWord( szBuff, strBuff );

							if( smRender.m_FogIsRend )
							{
								if( stricmp( szBuff, "/포그모드") == 0 )
								{
									pos = GetWord( szBuff, pos );
									smRender.m_FogMode = atoi( szBuff );
								}
								else if( smRender.m_FogMode )
								{
									if( stricmp( szBuff, "/포그시작") == 0 )
									{
										pos = GetWord( szBuff, pos );
										smRender.m_fFogStNum = (float)atof( szBuff );
										lpD3DDevice->SetRenderState( D3DRENDERSTATE_FOGTABLESTART, *((DWORD*)(&smRender.m_fFogStNum)) );
									}
									else if( stricmp( szBuff, "/포그끝") == 0 )
									{
										pos = GetWord( szBuff, pos );
										smRender.m_fFogEndNum = (float)atof( szBuff );
										lpD3DDevice->SetRenderState( D3DRENDERSTATE_FOGTABLEEND, *((DWORD*)(&smRender.m_fFogEndNum)) );
									}
									else if( stricmp( szBuff, "/포그칼라") == 0 )
									{
										int r,g,b,a;
										pos = GetWord( szBuff, pos );
										r = atoi( szBuff );
										pos = GetWord( szBuff, pos );
										g = atoi( szBuff );
										pos = GetWord( szBuff, pos );
										b = atoi( szBuff );
										pos = GetWord( szBuff, pos );
										a = atoi( szBuff );

										smRender.m_dwFogColor = RGBA_MAKE(r,g,b,a);
										lpD3DDevice->SetRenderState( D3DRENDERSTATE_FOGCOLOR, smRender.m_dwFogColor );
									}
								}
							}

							if( IsCreateNewRenderTarget() )
							{
								if( stricmp( szBuff, "/정상모드") == 0 )
									SetFilterEffect( FILTER_EFFECT_NULL, 0 );
								else if( stricmp( szBuff, "/모션블러") == 0 )
								{
									pos = GetWord( szBuff, pos ); 
									Alpha = atoi(szBuff);
									if( Alpha < 80 || Alpha > 200 )		Alpha = 80;
								}
								else if( stricmp( szBuff, "/뽀샤시") == 0 )
								{
									pos = GetWord( szBuff, pos );
									Alpha = atoi(szBuff);
									if( Alpha < 160 || Alpha > 200 )	Alpha = 180;
									SetFilterEffect( FILTER_EFFECT_SET_BRIGHT_CONTRAST, Alpha );
								}
							}
						}
						//######################################################################################
*/

						if ( GameMode==2 ) {
							if ( !usehFocus )
								SendChatMessageToServer( strBuff );	//int chatlistSPEAKERflag();

							if ( chatlistSPEAKERflag() && lstrlen(strBuff)>LastWhisperLen ) {
								SetClanChatting();				//클랜 채팅 모드
								if ( szLastWhisperName[0] ) {

									szLastWhisperName[0] = 0; //귓말 초기화
								}
							}
							else
								if ( szLastWhisperName[0] && lstrlen(strBuff)>LastWhisperLen ) {
									//파티원 귓말 보내기
									WhisperPartyPlayer( szLastWhisperName );
								}
								else {
									SetWindowText( hFocusWnd , "" );
									szLastWhisperName[0] = 0;
									LastWhisperLen = 0;
									InterfaceParty.chat_WhisperPartyPlayer_close();
									chatlistSPEAKERflagChg( 0 );		//클랜 채팅 종료
								}
						}
						else {
							hFocusWnd = 0;
						}
					}
					else {
						hFocusWnd = 0;
						if ( GameMode==2 ) {
							szLastWhisperName[0] = 0;
							LastWhisperLen = 0;
							InterfaceParty.chat_WhisperPartyPlayer_close();
							chatlistSPEAKERflagChg( 0 );		//클랜 채팅 종료
						}
					}
				}

				if ( GameMode==2 ) {
					if ( wParam == VK_TAB && VRKeyBuff[wParam]==0 ) {
						IsClanChatting();
						break;
					}

					if ( wParam == VK_UP && VRKeyBuff[wParam]==0 && !LastWhisperLen ) {
						//채팅 저장 코맨드 복구 
						RecallSavedChatCommand( hFocusWnd , 1 );
						//VRKeyBuff[ wParam ] = 1;
						break;
					}
					if ( wParam == VK_DOWN && VRKeyBuff[wParam]==0 && !LastWhisperLen ) {
						RecallSavedChatCommand( hFocusWnd , 0 );
						//VRKeyBuff[ wParam ] = 1;
						break;
					}
					if ( wParam==VK_ESCAPE && VRKeyBuff[wParam]==0 ) {
						if ( SendMessage( hTextWnd , EM_GETLIMITTEXT , 78 , 0 )>=78 ) {
							hFocusWnd = 0;
							cInterFace.ChatFlag = 0;
							szLastWhisperName[0]= 0;
							LastWhisperLen = 0;
							InterfaceParty.chat_WhisperPartyPlayer_close();
							chatlistSPEAKERflagChg( 0 );		//클랜 채팅 종료
						}
//							SendMessage( hTextWnd , EM_SETLIMITTEXT , 78 , 0 );
					}

					//채팅창 스크롤
					if ( wParam == VK_NEXT && VRKeyBuff[wParam]==0 ) {
						ChatScrollPoint-=DispChatMsgHeight;//6;
					}
					if ( wParam == VK_PRIOR && VRKeyBuff[wParam]==0 ) {
						ChatScrollPoint+=DispChatMsgHeight;//6;
					}

					if ( !hFocusWnd ) {
						//ImmSimulateHotKey(hwnd,IME_CHOTKEY_IME_NONIME_TOGGLE);
						//ImmSimulateHotKey(hTextWnd,IME_CHOTKEY_IME_NONIME_TOGGLE);
						SetIME_Mode( 0 );		//IME 모드 전환
						ChatScrollPoint = 0;
						//ImmSetConversionStatus( hImc , IME_CMODE_ALPHANUMERIC,IME_CMODE_ALPHANUMERIC );
						//ImmSetConversionStatus( hImcEdit , IME_CMODE_ALPHANUMERIC,IME_CMODE_ALPHANUMERIC );
					}
				}
/*
				IMETest.SetHWND(hFocusWnd);
				IMETest.GetInfoPerTime();
				for(int cnt=0;cnt<10;cnt++) {
					lstrcpy( szIME_Buff[cnt] , IMETest.m_szCandList[cnt].GetBuffer( IMETest.m_szCandList[cnt].GetLength() ) );
				}
*/
				SendMessage( hFocusWnd, messg , wParam , lParam );
				break;
			}

			if ( wParam==VK_ESCAPE && VRKeyBuff[wParam]==0 ) {
				if ( VRKeyBuff[VK_SHIFT] )  
					QuitGame();
				else {
					if ( cInterFace.ChatFlag ) 
						cInterFace.ChatFlag = 0;
					else {
						if ( cInvenTory.OpenFlag || cCharStatus.OpenFlag || cSkill.OpenFlag || ExitButtonClick ) {
							cInterFace.CheckAllBox( SIN_ALL_CLOSE );
						}
						else {
							if ( InterfaceParty.PartyPosState==PARTY_PROCESS ) {
								InterfaceParty.PartyPosState=PARTY_END;
								InterfaceParty.qN_Pressf_CLEAR();
							}
							else {
								ExitButtonClick = 1;
							}
						}
					}
				}
			}


			if ( GameMode==2 ) {
				if ( GameWindowMessage( hWnd, messg, wParam , lParam )==FALSE ) break;
			}
/*
			else {
				if ( smConfig.DebugMode && wParam == VK_F8 && VRKeyBuff[ wParam ]==0 ) {
					SetGameMode( 2 );
				}
			}
*/
			if ( GameMode==2 && lpCurPlayer->OnStageField>=0 && StageField[lpCurPlayer->OnStageField] && 
				StageField[lpCurPlayer->OnStageField]->State==FIELD_STATE_ROOM )
			{
				//알까기 대국장에서는 화면 캡쳐 금지

			}
			else
			{
#ifdef _LANGUAGE_ENGLISH	//해외
				if(lpCurPlayer->OnStageField >= 0 && StageField[lpCurPlayer->OnStageField] &&
					StageField[lpCurPlayer->OnStageField]->State == FIELD_STATE_CASTLE)
				{
					//필리핀 공성필드에서 캡쳐금지
				}
				else
				{
					if ( wParam==VK_HOME && VRKeyBuff[wParam]==0 && VRKeyBuff[VK_CONTROL] )
					{
						Capture(hwnd,CAPTURE_JPG);
					}
					//화면 캡쳐2
					if ( wParam==VK_END && VRKeyBuff[wParam]==0 && VRKeyBuff[VK_CONTROL] )
					{
						Capture(hwnd,CAPTURE_BMP);
					}
				}
#else
				//화면 캡쳐
				if ( wParam==VK_HOME && VRKeyBuff[wParam]==0 && VRKeyBuff[VK_CONTROL] )
				{
					Capture(hwnd,CAPTURE_JPG);
				}
				//화면 캡쳐2
				if ( wParam==VK_END && VRKeyBuff[wParam]==0 && VRKeyBuff[VK_CONTROL] )
				{
					Capture(hwnd,CAPTURE_BMP);
				}
#endif
			}

			VRKeyBuff[ wParam ] = 1;
			clan_Mouse(WM_KEYDOWN,wParam);

			break;

		//case WM_IME_KEYUP:
		case WM_KEYUP :
			//######################################################################################
			//작 성 자 : 오 영 석
			if( g_IsDxProjectZoomIn > 0 )
			{
				if( KeyFullZoomMap(wParam) )
				{
					VRKeyBuff[ wParam ] = 0;
                    break;
				}
			}
			//######################################################################################


#ifdef PARKMENU

			/////////펑션키	 Key UP
			//if ( GameMode==2 )
			//{
				if( wParam == 0x47 )
				{
					if(vrunRuning==TRUE)
					{
						if(bMic==TRUE)
						{
							bMic = FALSE;
							void ParkSetMicOnOFF();
							ParkSetMicOnOFF();
						}
					}
				}
			//}
#endif

			if ( hFocusWnd ) {
				SendMessage( hFocusWnd, messg , wParam , lParam );
			}
			VRKeyBuff[ wParam ] = 0;
			clan_Mouse(WM_KEYUP,wParam);
			break;

		case WM_SYSKEYDOWN:
			if ( wParam==VK_F10 ) {
				PostMessage( hWnd , WM_KEYDOWN , wParam , lParam );
				return FALSE;
			}
			if ( hFocusWnd ) 
				SendMessage( hFocusWnd, messg , wParam , lParam );
			break;

		case WM_SYSKEYUP:
			if ( wParam==VK_F10 ) {
				PostMessage( hWnd , WM_KEYUP , wParam , lParam );
				return FALSE;
			}
			if ( hFocusWnd ) 
				SendMessage( hFocusWnd, messg , wParam , lParam );
			break;

		case WM_MOUSEMOVE:
			//######################################################################################
			//작 성 자 : 오 영 석
			if( g_IsDxProjectZoomIn )
			{
				g_iFzmCursorPosX = LOWORD(lParam);  // horizontal position of cursor 
				g_iFzmCursorPosY = HIWORD(lParam);  // vertical position of cursor 

				if( g_iFzmCursorPosX > 30  * g_fWinSizeRatio_X && g_iFzmCursorPosX < 350 * g_fWinSizeRatio_X &&
					g_iFzmCursorPosY > 338 * g_fWinSizeRatio_Y && g_iFzmCursorPosY < 578 * g_fWinSizeRatio_Y )
				{
					if( g_iFzmCursorFocusGame == 0 )
					{
						g_iFzmCursorFocusGame = 1;
						
						xPs = pCursorPos.x = msXo = int( ((float)g_iFzmCursorPosX - g_fZoomInAdd_x) * g_fZoomInDiv_x );
						yPs = pCursorPos.y = msYo = int( ((float)g_iFzmCursorPosY - g_fZoomInAdd_y) * g_fZoomInDiv_y );
					}
					
					xPs = int( ((float)g_iFzmCursorPosX - g_fZoomInAdd_x) * g_fZoomInDiv_x );
					yPs = int( ((float)g_iFzmCursorPosY - g_fZoomInAdd_y) * g_fZoomInDiv_y );

					AutoMouse_Distance += abs(pCursorPos.x-xPs);
					AutoMouse_Distance += abs(pCursorPos.y-yPs);

					pCursorPos.x  = xPs;
					pCursorPos.y  = yPs;

					msX = msXo - xPs;
					msY = msYo - yPs;

					msXo = xPs;
					msYo = yPs;

					if( (wParam & MK_MBUTTON) )
						SetMousePlay( 3 );
					else
					{
						if( wParam == MK_LBUTTON || TraceMode_DblClick ) 
							SetMousePlay( 2 );
						else if( wParam == MK_RBUTTON )
							SetMousePlay( 4 );
					}
					dwLastMouseMoveTime = dwPlayTime;
				}
				else
					g_iFzmCursorFocusGame = 0;
			}
			else
			{
				xPs = LOWORD(lParam);  // horizontal position of cursor 
				yPs = HIWORD(lParam);  // vertical position of cursor 

				AutoMouse_Distance += abs(pCursorPos.x-xPs);
				AutoMouse_Distance += abs(pCursorPos.y-yPs);

				//pCursorPos.x  = xPs;
				//pCursorPos.y  = yPs;
				g_iFzmCursorPosX = pCursorPos.x  = xPs;
				g_iFzmCursorPosY = pCursorPos.y  = yPs;
				msX = msXo - xPs;
				msY = msYo - yPs;

				msXo = xPs;
				msYo = yPs;

				if ( (wParam&MK_MBUTTON) )
					SetMousePlay( 3 );
				else {
					if ( wParam==MK_LBUTTON || TraceMode_DblClick ) 
						SetMousePlay( 2 );
					else
						if ( wParam==MK_RBUTTON )
							SetMousePlay( 4 );
				}

	/*
				if ( wParam==MK_LBUTTON ) {
					angY += MousemX/4;
					angX -= MousemY/4;
				}
	*/
			}
			//######################################################################################

			dwLastMouseMoveTime = dwPlayTime;
			break;

		case WM_LBUTTONDBLCLK:
			MouseDblClick = TRUE;

			if ( GameMode==2 ) {
				//안내지도 띄우기
				if ( GetFieldMapCursor()==TRUE && cInterFace.sInterFlags.MapOnFlag ) {
					sinCheck_ShowHelpMap();
				}
				sinDoubleClick();

				dsMenuCursorPos( &pCursorPos , 2 );
				if ( StopCharMotion(pCursorPos.x,pCursorPos.y)!=TRUE ) {
					Moving_DblClick = TRUE;			//마우스 이동 첵크 더블 클릭
					MousePos_DblClick.x = xPs;
					MousePos_DblClick.y = yPs;
					MousePos_DblClick.z = 0;
				}
				dsMenuCursorPos( &pCursorPos , 0 );
			}
			break;

		case WM_LBUTTONDOWN:
			//######################################################################################
			//작 성 자 : 오 영 석
			if( g_IsDxProjectZoomIn <= 0 || g_iFzmCursorFocusGame )
			{
				SetMousePlay( 1 );
				sinLButtonDown();
				MouseButton[0] = TRUE;

				clan_Mouse(WM_LBUTTONDOWN,wParam);
			}
			else
				g_FzmMouseButton[0] = TRUE;
			//######################################################################################

			if ( GameMode==2 && DisconnectFlag ) quit = TRUE;
			break;

		case WM_LBUTTONUP:
			//######################################################################################
			//작 성 자 : 오 영 석
			if( g_IsDxProjectZoomIn <= 0 || g_iFzmCursorFocusGame || MouseButton[0] )
			{
				SetMousePlay( 0 );
				sinLButtonUp();
				
				MouseButton[0] = FALSE;
				MouseDblClick = FALSE;
				//ktj
				clan_Mouse(WM_LBUTTONUP,wParam);
			}
			else {
				MouseDblClick = FALSE;
				g_FzmMouseButton[0] = FALSE;
			}
			//######################################################################################

			break;

		case WM_RBUTTONDOWN:
			//######################################################################################
			//작 성 자 : 오 영 석
			if( g_IsDxProjectZoomIn <= 0 || g_iFzmCursorFocusGame )
			{
				SetMousePlay( 11 );
				MouseButton[1] = TRUE;
				//ktj
				clan_Mouse(WM_RBUTTONDOWN,wParam);
			}
			else
				g_FzmMouseButton[1] = TRUE;
			//######################################################################################

			break;

		case WM_RBUTTONUP:
			//######################################################################################
			//작 성 자 : 오 영 석
			if( g_IsDxProjectZoomIn <= 0 || g_iFzmCursorFocusGame )
			{
				SetMousePlay( 10 );
				sinRButtonUp();
				MouseButton[1] = FALSE;
				//ktj
				clan_Mouse(WM_RBUTTONUP,wParam);
			}
			else
				g_FzmMouseButton[1] = FALSE;
			//######################################################################################

			break;

		case WM_MBUTTONDOWN:
			//######################################################################################
			//작 성 자 : 오 영 석
			if( g_IsDxProjectZoomIn <= 0 || g_iFzmCursorFocusGame )
			{
				MouseButton[2] = TRUE;
				//ktj
				clan_Mouse(WM_MBUTTONDOWN,wParam);
			}
			else
				g_FzmMouseButton[2] = TRUE;
			//######################################################################################
			break;

		case WM_MBUTTONUP:
			//######################################################################################
			//작 성 자 : 오 영 석
			if( g_IsDxProjectZoomIn <= 0 || g_iFzmCursorFocusGame )
			{
				MouseButton[2] = FALSE;
				//ktj
				clan_Mouse(WM_MBUTTONUP,wParam);
			}
			else
				g_FzmMouseButton[2] = FALSE;
			//######################################################################################

			break;

		case WM_MOUSEWHEEL:
			//######################################################################################
			//작 성 자 : 오 영 석
			if( g_IsDxProjectZoomIn <= 0 || g_iFzmCursorFocusGame )
			{
				zDelta = (short) HIWORD(wParam);    // wheel rotation

				if ( cSinHelp.sinGetScrollMove(-zDelta/16)==TRUE ) break;
				if ( TJBscrollWheel( zDelta )==TRUE ) break;

				if ( whAnx==ANX_NONE ) whAnx = anx+zDelta;
				else whAnx +=zDelta;
				AutoCameraFlag = FALSE;
			}
			//######################################################################################

			break;

		case WM_CHAR:
			if ( wParam==0x0D || wParam==0x0A || ( GameMode==2 && wParam==VK_TAB ) ) return TRUE;
			if ( !smConfig.DebugMode && wParam==22 && lpCurPlayer->szChatMessage[0] ) return TRUE;		//도배 금지 CONTROL-V 막음
	
		case WM_SYSCHAR:
		case WM_IME_CHAR:
		case WM_IME_COMPOSITION:
/*
		case WM_IME_CONTROL:
		case WM_IME_COMPOSITIONFULL:
		case WM_IME_ENDCOMPOSITION:
		case WM_IME_KEYDOWN:
		case WM_IME_KEYUP:
		case WM_IME_NOTIFY:
		case WM_IME_REQUEST:
		case WM_IME_SELECT:
		case WM_IME_SETCONTEXT:
		case WM_IME_STARTCOMPOSITION:
*/
			//######################################################################################
			//작 성 자 : 오 영 석
			if( g_IsDxProjectZoomIn > 0 ) break;
			//######################################################################################
			if ( hFocusWnd ) 
				SendMessage( hFocusWnd, messg , wParam , lParam );
			break;

		case WM_KILLFOCUS:
			SetFocus( hWnd );
			return TRUE;

		case WM_COMMAND:
			break;

		case WM_DESTROY:
			PostQuitMessage (0);
			break;


		case WM_ACTIVATE: 
			break;

		case WM_NCACTIVATE: 
#ifdef _WINMODE_DEBUG
			if ( !smConfig.DebugMode )
#endif
#ifndef _LANGUAGE_WINDOW
				SetForegroundWindow( hwnd );
#endif

			//여기에 해킹 툴 삽입
			if ( GameMode==2 && lpCurPlayer->dwObjectSerial ) {
				//오프라인 크랙 검색
				CheckCracker();
				//크랙을 다시 찾는다
				CheckCracker( NULL ); //(온라인 검색)
				//캐릭터 정보 인증 확인
				CheckCharForm();
			}

			break;

		case WM_TIMER:
			if ( (dwTimerCount&3)==0 ) {
				dwGameWorldTime ++;			//게임의 월드시간

				dwGameHour = dwGameWorldTime+dwGameTimeOffset;
				dwGameHour = dwGameHour/60;
				dwGameHour = dwGameHour-(((int)(dwGameHour/24))*24);			//게임내부에서의 시
				dwGameMin = dwGameWorldTime-(((int)(dwGameWorldTime/60))*60);		//게임내부에서의 분

				//dwGameHour = 1;		//밤 만들기
			}

			if ( smWsockServer && smWsockDataServer && smWsockUserServer ) {
				smCheckWaitMessage();
			}

			dwTimerCount++;
/*
			if ( (dwTimerCount&0x1F)==0 ) {
				//nProtect 확인
				if ( Check_nProtect()==FALSE ) {
					quit = TRUE;
				}
			}
*/
			break;

		case WM_NPROTECT_EXIT_TWO:
			//nProtect 종료 메세지
			DisconnectServerCode = 4;
			DisconnectFlag = GetCurrentTime();
			break;

		case WM_NPROTECT_SPEEDHACK:
		case WM_NPROTECT_SH_ERROR:
		case WM_NPROTECT_SOFTICE:
			DisconnectServerCode = 4;
			DisconnectFlag = GetCurrentTime();
			quit = 1;
			break;

		default: 
			return DefWindowProc(hWnd,messg,wParam,lParam);
			break;
	}
	return 0L;

}




int MoveAngle( int step , int angY )
{
	int x,z;

	x = ((pX<<16) + GetSin[angY&ANGLE_MASK] * step)>>16;
	z = ((pZ<<16) + GetCos[angY&ANGLE_MASK] * step)>>16;
/*
	y = smStage->GetHeight( x,pY,z );
	if ( (y-pY)>64 ) return FALSE;
	y = smStage->GetHeight( x-24,pY,z-24 );
	if ( (y-pY)>64 ) return FALSE;
	y = smStage->GetHeight( x+24,pY,z-24 );
	if ( (y-pY)>64 ) return FALSE;
	y = smStage->GetHeight( x-24,pY,z+24 );
	if ( (y-pY)>64 ) return FALSE;
	y = smStage->GetHeight( x+24,pY,z+24 );
	if ( (y-pY)>64 ) return FALSE;
*/

	pX=x; 
	pZ=z;

	return TRUE;
}

#define CAMERA_MOVE_STEP		(8*fONE)

int TraceCameraMain()
{

	if ( TraceCameraPosi.x<TraceTargetPosi.x ) {
		TraceCameraPosi.x+=CAMERA_MOVE_STEP;
		if ( TraceCameraPosi.x>TraceTargetPosi.x )
			TraceCameraPosi.x = TraceTargetPosi.x;
	}
	if ( TraceCameraPosi.x>TraceTargetPosi.x ) {
		TraceCameraPosi.x-=CAMERA_MOVE_STEP;
		if ( TraceCameraPosi.x<TraceTargetPosi.x )
			TraceCameraPosi.x = TraceTargetPosi.x;
	}

	if ( TraceCameraPosi.y<TraceTargetPosi.y ) {
		TraceCameraPosi.y+=CAMERA_MOVE_STEP;
		if ( TraceCameraPosi.y>TraceTargetPosi.y )
			TraceCameraPosi.y = TraceTargetPosi.y;
	}
	if ( TraceCameraPosi.y>TraceTargetPosi.y ) {
		TraceCameraPosi.y-=CAMERA_MOVE_STEP/2;
		if ( TraceCameraPosi.y<TraceTargetPosi.y )
			TraceCameraPosi.y = TraceTargetPosi.y;
	}


	if ( TraceCameraPosi.z<TraceTargetPosi.z ) {
		TraceCameraPosi.z+=CAMERA_MOVE_STEP;
		if ( TraceCameraPosi.z>TraceTargetPosi.z )
			TraceCameraPosi.z = TraceTargetPosi.z;
	}
	if ( TraceCameraPosi.z>TraceTargetPosi.z ) {
		TraceCameraPosi.z-=CAMERA_MOVE_STEP;
		if ( TraceCameraPosi.z<TraceTargetPosi.z )
			TraceCameraPosi.z = TraceTargetPosi.z;
	}

	return TRUE;
}


int RendSightPos = 0;
int RendSightOffset = 0;

//////////////////// 렌더링 뷰 거리 설정 ////////////////////
int SetRendSight()
{
#ifdef PT_WEB
	if ( s_WebViewCells>0 ) {					// page draw distance setting (map cells)
		ViewPointLen = s_WebViewCells * SizeMAPCELL;
		ccDistZMin   = ViewPointLen - (20 + s_WebViewCells/8) * SizeMAPCELL;	// far colour fade starts here
		if ( ccDistZMin<ViewPointLen/3 ) ccDistZMin = ViewPointLen/3;
		return TRUE;
	}
#endif
	if( smRender.m_GameFieldView ) {
		ViewPointLen = 38 * SizeMAPCELL + RendSightPos ;
		ccDistZMin   = ViewPointLen - (20*SizeMAPCELL+RendSightPos/4);

		return TRUE;
	}

	return FALSE;
}

int RendSightSub( int flag )
{
	if ( lpCurPlayer->OnStageField>=0 && StageField[ lpCurPlayer->OnStageField ]->FieldSight ) {
		RendSightOffset = StageField[ lpCurPlayer->OnStageField ]->FieldSight * SizeMAPCELL;
	}
	else {
		RendSightOffset = smRender.m_GameFieldViewStep * SizeMAPCELL;
		if ( flag ) RendSightPos = RendSightOffset;
	}

	if ( RendSightPos<RendSightOffset ) RendSightPos+=4;
	else 
		if ( RendSightPos>RendSightOffset ) RendSightPos-=4;


	return TRUE;
}
/////////////////////////////////////////////////////////////////////




#define	FPS_TIME		(1000/70)

DWORD FrameSkipTimer = 0;
int fps	= 70;
int FpsTime;

#define AC_MOVE_STEP	4
#define AC_MOVE_MIN		256

//int aMoveStep = 0;
int dsCameraRotation = 0;			//던전시즈 카메라 회전값

void PlayMain()
{
	int mv;

	if ( ActionGameMode ) {
		//액션게임 키조작
		ActionGameMain();
	}
	else if ( cSinHelp.sinGetHelpShowFlag()!=TRUE ) {

		if ( CameraInvRot ) {
			if ( VRKeyBuff[VK_RIGHT] ) { any+=16; AutoCameraFlag=FALSE; }
			if ( VRKeyBuff[VK_LEFT] ) { any-=16; AutoCameraFlag=FALSE; }
		}
		else {
			if ( VRKeyBuff[VK_RIGHT] ) { any-=16; AutoCameraFlag=FALSE; }
			if ( VRKeyBuff[VK_LEFT] ) { any+=16; AutoCameraFlag=FALSE; }
		}

		if ( VRKeyBuff[VK_CONTROL] ) {
			if ( VRKeyBuff[VK_UP] ) { anx -= 16; AutoCameraFlag=FALSE; }
			if ( VRKeyBuff[VK_DOWN] ) { anx +=16; AutoCameraFlag=FALSE; }
		}
		else {
			//######################################################################################
			//작 성 자 : 오 영 석
			if ( VRKeyBuff[VK_UP] )
			{
				dist -= 8;
				if( dist < 100 )
				{
					if( anx <= 40 )
					{
						if( dist < 40 )
							dist = 40;
					}
					else
						dist = 100;

					CaTarDist = 0;
				}
			}

			if ( VRKeyBuff[VK_DOWN] ) 
			{
				dist +=8;
				if( dist > 440 ) { dist=440; CaTarDist=0; }
			}
			//######################################################################################
		}

		if ( VRKeyBuff[VK_NEXT] ) { anx -= 16; AutoCameraFlag=FALSE; }
		if ( VRKeyBuff[VK_PRIOR] ) { anx +=16; AutoCameraFlag=FALSE; }
	}

	if ( cInterFace.sInterFlags.CameraAutoFlag!=2 ) {
		//던전시즈 카메라
		if ( pCursorPos.x>=8 && pCursorPos.x<=WinSizeX-8 ) {
			dsCameraRotation = 0;
		}

		if ( pCursorPos.x<8 ) {
			if ( !dsCameraRotation ) dsCameraRotation = -512;
			mv = dsCameraRotation/16;
			if ( mv<-16 ) mv=-16;
			any+=mv;
			dsCameraRotation++;
			if ( dsCameraRotation>=0 ) {
				dsCameraRotation = 0;
				pCursorPos.x = 8;
			}
			AutoCameraFlag=FALSE;
		}

		if ( pCursorPos.x>WinSizeX-8 ) { 
			if ( !dsCameraRotation ) dsCameraRotation = 512;
			mv=dsCameraRotation/16;
			if ( mv>16 ) mv=16;
			any+=mv;
			dsCameraRotation--;
			if ( dsCameraRotation<=0 ) {
				dsCameraRotation = 0;
				pCursorPos.x = WinSizeX-8;
			}
			AutoCameraFlag=FALSE;
		}


		//if ( pCursorPos.y<16 ) { anx-=8; AutoCameraFlag=FALSE; }
		//if ( pCursorPos.y>WinSizeY-16 ) { anx+=8; AutoCameraFlag=FALSE; }
	}

	if ( CaTarDist ) {
		if ( dist<CaTarDist ) {
			dist+=12;
			if ( dist>=CaTarDist ) { dist=CaTarDist;CaTarDist = 0; }
		}
		else 
			if ( dist>CaTarDist ) {
				dist-=12;
				if ( dist<=CaTarDist ) { dist=CaTarDist;CaTarDist = 0; }
			}
	}

	if ( CaMovAngle ) {
		mv = CaMovAngle>>4;
		if ( CaMovAngle>0 ) {
			if ( mv>32 ) mv = 32;
			any += mv; CaMovAngle-=mv;
			if ( CaMovAngle<0 ) CaMovAngle = 0;
		}
		else {
			if ( mv<-32 ) mv = -32;
			any += mv; CaMovAngle-=mv;
			if ( CaMovAngle>0 ) CaMovAngle = 0;
		}
		if ( mv==0 ) CaMovAngle=0;

		if ( CaMovAngle==0 )
			AutoCameraFlag=TRUE;
		else
			AutoCameraFlag=FALSE;
	}

	if ( whAnx!=ANX_NONE ) {
		//휠에 의한 각도 변경
		if ( anx<whAnx ) {
			anx+=8;
			if ( anx>=whAnx ) {
				whAnx = ANX_NONE;
				AutoCameraFlag = TRUE;
			}
		}
		else {
			anx-=8;
			if ( anx<=whAnx ) {
				whAnx = ANX_NONE;
				AutoCameraFlag = TRUE;
			}
		}
	}

	if ( PlayFloor>0 ){
		if ( ViewAnx<500 )
			ViewAnx += 8;

		if ( ViewDist>250 )
			ViewDist -= 8;

		PlayFloor--;
	}
	else {
		if ( ViewAnx<anx ) {
			ViewAnx += 8;
			if ( ViewAnx>anx ) ViewAnx = anx;
		}
		if ( ViewAnx>anx ) {
			ViewAnx -= 8;
			if ( ViewAnx<anx ) ViewAnx = anx;
		}


		if ( CaTarDist ) 
			mv=100;
		else 
			mv=8;

		if ( ViewDist<dist ) {
			ViewDist += mv;
			if ( ViewDist>dist ) ViewDist = dist;
		}
		if ( ViewDist>dist ) {
			ViewDist -= mv;
			if ( ViewDist<dist ) ViewDist = dist;
		}
	}


	if ( DebugPlayer ) {
		if ( dpX==0 && dpZ==0 ) {
			dpX = lpCurPlayer->pX;
			dpY = lpCurPlayer->pY;
			dpZ = lpCurPlayer->pZ;
		}

		if ( VRKeyBuff[ VK_SPACE ] ) {
			GetMoveLocation( 0 , 0, fONE*2, anx, any, 0 );
			dpX += GeoResult_X;
			dpY += GeoResult_Y;
			dpZ += GeoResult_Z;
		}

		if ( VRKeyBuff[ 'Z' ]  ) anz -=8;
		if ( VRKeyBuff[ 'X' ]  ) anz +=8;

		lpCurPlayer->pX = dpX;
		lpCurPlayer->pZ = dpZ;

		anx &=ANGCLIP;

		if ( anx>=ANGLE_90 && anx<ANGLE_270) {
			if ( anx<ANGLE_180 )
				anx = ANGLE_90-32;
			else
				anx = ANGLE_270+32;
		}
		ViewAnx = anx;
		whAnx = ANX_NONE;

	}
	else {
		if ( anx>=(ANGLE_90-48) ) {
			anx = (ANGLE_90-48);
			whAnx = ANX_NONE;
		}
		if ( anx<40 ) {
			anx = 40;
			whAnx = ANX_NONE;
		}
	}

	//######################################################################################
	//작 성 자 : 오 영 석
	if( anx > 40 && dist < 100 ) { dist=100; CaTarDist=0; }
	if( dist > 440 )			 { dist=440; CaTarDist=0; }
	//######################################################################################

	int ay;
	int astep;

	if ( PlayCameraMode!=cInterFace.sInterFlags.CameraAutoFlag ) {
		if ( cInterFace.sInterFlags.CameraAutoFlag==2 ) any=ANGLE_45;
		PlayCameraMode = cInterFace.sInterFlags.CameraAutoFlag;

		if( !dwM_BlurTime && IsCreateNewRenderTarget() ) {
			dwM_BlurTime=dwPlayTime+600;
			SetFilterEffect( FILTER_EFFECT_SET_MOTION_BLUR, 80 );
		}
	}

	if ( PlayCameraMode==1 && AutoCameraFlag && any!=lpCurPlayer->Angle.y && lpCurPlayer->MoveFlag ) {
		ay = (any-lpCurPlayer->Angle.y) & ANGCLIP;

		if ( ay>=ANGLE_180 ) {
			ay=ay-ANGLE_360;
		}
/*
		if ( abs(ay)>AC_MOVE_MIN && abs(ay)<(ANGLE_90+180) ) {
			astep = ay>>6;
			if ( ay<0 ) {
				if ( astep<-AC_MOVE_STEP ) astep = -AC_MOVE_STEP;
//				astep = -AC_MOVE_STEP;
			}
			else {
				if ( astep>AC_MOVE_STEP ) astep = AC_MOVE_STEP;
//				astep = AC_MOVE_STEP;
			}
//			aMoveStep += astep;
			any = ( any-astep ) & ANGCLIP;
		}
*/


		if ( abs(ay)<(ANGLE_90+180) ) {

			if ( ay<0 ) {
				astep = abs( ay )>>6;
				if ( astep<AC_MOVE_STEP ) astep = AC_MOVE_STEP;

				if ( ay<-AC_MOVE_MIN ) {
					ay += astep;
					if ( ay>0 ) {
						any = lpCurPlayer->Angle.y;
					}
					else
						any = ( any+astep ) & ANGCLIP;
				}
			}
			else {
				astep = abs( ay )>>6;
				if ( astep<AC_MOVE_STEP ) astep = AC_MOVE_STEP;
	
				if ( ay>AC_MOVE_MIN ) {
					ay -= astep;
					if ( ay<0 ) {
						any = lpCurPlayer->Angle.y;
					}
					else
						any = ( any-astep ) & ANGCLIP;
				}
			}

		}

	}
/*
	if ( aMoveStep>0 ) {
		aMoveStep -= 1;
		if ( aMoveStep<0 ) aMoveStep=0;
		any = ( any-aMoveStep ) & ANGCLIP;
	}
	if ( aMoveStep<0 ) {
		aMoveStep += 1;
		if ( aMoveStep>0 ) aMoveStep=0;
		any = ( any-aMoveStep ) & ANGCLIP;
	}
*/

	NetWorkPlay();
	PlayPat3D();

	MainEffect();							//이펙트 메인

	//######################################################################################
	//작 성 자 : 오 영 석
	if( g_IsDxProjectZoomIn )
		MainFullZoomMap();
	//######################################################################################

	dwMemError = dwMemError^dwPlayTime;
	dwPlayTime = GetCurrentTime();			//시간 기록
	dwMemError = dwMemError^dwPlayTime;

	TraceCameraMain();

	RendSightSub(0);				//렌더링 시야 변경
}

POINT3D TargetPosi = { 0,0,0 };
POINT3D GeoResult = { 0,0,0 };

int timeg = 0;
int RenderCnt=0;
int DispRender;
int	DispMainLoop;

DWORD	dwLastPlayTime =0;
DWORD	dwLastOverSpeedCount = 0;
int		PlayTimerMax = 0;
int		MainLoopCount = 0;
DWORD	dwLastRenderTime = 0;

void PlayD3D()
{
	DWORD dwTime;

#ifdef PT_WEB
	PtWeb_PollOptions();			// page settings: sound volumes, draw distance
#endif
	dwTime = GetCurrentTime();
	if ( dwLastPlayTime ) {
		//스피드핵 경고
		if ( dwTime<dwLastPlayTime && dwLastOverSpeedCount>1000 ) {
			//해킹 시도한 유저 자동 신고
			SendSetHackUser2( 1200 , dwLastPlayTime-dwTime );
			dwLastOverSpeedCount = 0;
		}

		//프레임 타이머 차를 기록 ( 최대치를 나중에 확인하여 프로세스 디버그가 사용됐는지 검사 )
		//SubTime = dwTime-dwLastPlayTime;
		//if ( SubTime>PlayTimerMax ) dwLastOverSpeedCount = SubTime;

	}
	dwLastPlayTime = dwTime;

	if ( FrameSkipTimer==0 ) { 
		FrameSkipTimer = dwTime;
		FpsTime = 1000/fps;
	}

	if ( ((int)(dwTime-FrameSkipTimer))>5000 ) {
		FrameSkipTimer = dwTime;//-5000;
	}

	if ( FrameSkipTimer>dwTime ) {
		Sleep( FrameSkipTimer-dwTime );
		dwTime = GetCurrentTime();
		dwLastPlayTime = dwTime;
	}

	pRealCursorPos.x = pCursorPos.x;
	pRealCursorPos.y = pCursorPos.y;

	while(1) {
		if ( FrameSkipTimer>=dwTime ) break;

		FrameSkipTimer += FPS_TIME;

		switch( GameMode ) {
		case 1:
			//lpPlaySelect->Main();
			if ( MainOpening()==TRUE ) {
				SetGameMode(2);
			}
			break;
		case 2:
			PlayMain();
			//캐릭터 정보 인증 확인
#ifdef _SINBARAM
			//######################################################################################
			//작 성 자 : 오 영 석
			if( g_IsDxProjectZoomIn <= 0 ) {
				dsMenuCursorPos( &pCursorPos , 1 );				//메뉴커서 시물레이션
                sinMain();
				dsMenuCursorPos( &pCursorPos , 0 );				//메뉴커서 시물레이션 복구

				MainInterfaceParty( pHoPartyMsgBox.x , pHoPartyMsgBox.y );		//파티권유 메뉴
			}
			else
				MainSub();
			//######################################################################################
#endif
			if ( BellatraEffectInitFlag )		//벨라트라 이펙트
				MainBellatraFontEffect();

			break;
		}

		MainLoopCount++;
		dwLastOverSpeedCount++;
	}

	if ( ParkPlayMode ) {
		if ( ParkPlayMode<0 ) {
			if ((smFlipCount&1)==0)		//서페이스 면 첵크
				return;
		}
		else {
			if ( ParkPlayMode<=1000 ) {
				if ( (dwLastRenderTime+ParkPlayMode)>dwTime )
					return;
			}
		}

	}
	else {
		if ( GetForegroundWindow()!=hwnd )
			Sleep(200);
	}

	dwLastRenderTime = dwTime;

	int i;
//	HDC hdc;
//	char strBuff[256];

	i = GetCurrentTime();
	if ( timeg<i ) {
		timeg = i+1000;
		DispRender = RenderCnt;
		RenderCnt = 0;
		DispMainLoop = MainLoopCount;
		MainLoopCount = 0;
	}
	RenderCnt++;


	switch( GameMode ) {
	case 1:
		/*
		lpPlaySelect->Draw();

		lpDDSBack->GetDC( &hdc );
		SelectObject( hdc , hFont );
	    SetBkMode( hdc, TRANSPARENT );
		wsprintf( strBuff , "%d fps", DispRender );
		SetTextColor( hdc, RGB(0, 0, 0) );
		dsTextLineOut( hdc , 11,11, strBuff , lstrlen( strBuff ) );
		SetTextColor( hdc, RGB(255, 255, 255) );
		dsTextLineOut( hdc , 10,10, strBuff , lstrlen( strBuff ) );
		lpDDSBack->ReleaseDC( hdc );

		*/
		DrawOpening();
		if ( Flip()==FALSE ) {
			quit = 1;	
		}
		return;
	}
	

	//카메라 고정 시점
	if ( PlayCameraMode==2 ) {
		dist = 400;
		anx = ANGLE_45-128;
		ViewAnx = anx;
		ViewDist = dist;
		//any = ANGLE_45;
	}

	any &=ANGCLIP;
	anx &=ANGCLIP;


	//홍호동 추가(카메라 이펙트)      아래부분 수정입니다.
	if( WaveCameraFlag && WaveCameraMode ) // && g_IsCreateFilterEffect )
	{
		WaveCameraTimeCount++;
			
			if((WaveCameraTimeCount>WaveCameraDelay))
		{
			WaveCameraTimeCount = 0;
			if(WaveCameraDelay > 1 && WaveCameraFactor < 40)
                		WaveCameraFactor = -int((float)WaveCameraFactor/10.f*9.f);
			else
				WaveCameraFactor = -int((float)WaveCameraFactor/10.f*8.f);
			ViewDist += WaveCameraFactor;
		}

  		if(abs(WaveCameraFactor) < 1)
		{
			WaveCameraFlag = FALSE;
			WaveCameraTimeCount = 0;
		}
	}
	else {
		WaveCameraFlag = FALSE;
	}

	GetMoveLocation( 0 , 0, -(ViewDist<<FLOATNS), ViewAnx&ANGCLIP, any, 0 );
	//GetMoveLocation( 0 , 0, -(dist<<FLOATNS), anx, any, 0 );
	if ( GeoResult_X==GeoResult.x && GeoResult_Y==GeoResult.y && GeoResult_Z==GeoResult.z ) {
		pX += lpCurPlayer->pX - TargetPosi.x;
		pY += lpCurPlayer->pY - TargetPosi.y;
		pZ += lpCurPlayer->pZ - TargetPosi.z;
	}
	else {
		pX = lpCurPlayer->pX;
		pY = lpCurPlayer->pY;
		pZ = lpCurPlayer->pZ;

		pX +=GeoResult_X;
		pZ +=GeoResult_Z;
		pY +=GeoResult_Y;

		GeoResult.x = GeoResult_X;
		GeoResult.y = GeoResult_Y;
		GeoResult.z = GeoResult_Z;
	}

	TargetPosi.x = lpCurPlayer->pX;
	TargetPosi.y = lpCurPlayer->pY;
	TargetPosi.z = lpCurPlayer->pZ;


	if ( DebugPlayer ) {

		lpCurPlayer->Angle.y = any;

		pX = dpX;
		pY = dpY;
		pZ = dpZ;

		//anx = lpCurPlayer->Angle.x;
		any = lpCurPlayer->Angle.y;
	}
	else
		anz = 0;



/*
	if ( pX<tX ) tX-=512;
	if ( pX>tX ) tX+=512;
	if ( pY<tY ) tY-=512;
	if ( pY>tY ) tY+=512;
	if ( pZ<tZ ) tZ-=512;
	if ( pZ>tZ ) tZ+=512;
*/
/*
	tX = pX>>FLOATNS;
	tY = pY>>FLOATNS;
	tZ = pZ>>FLOATNS;
*/

	if ( smConfig.DebugMode && VRKeyBuff[VK_SHIFT] ) {
		if ( VRKeyBuff[VK_HOME] )
			DarkLevel=220;
		if ( VRKeyBuff[VK_END] )
			DarkLevel=0;
	}

	//속도 변수 Lock
	LockSpeedProtect( lpCurPlayer );

	//크리티칼 섹션 선언
	EnterCriticalSection( &cDrawSection );
	smEnterTextureCriticalSection();


	//스킬 포인트 숙련도 렌더링중 바뀌면 해킹
	DWORD	dwSkilChkSum=0;
	DWORD	dwChkSum;
	DWORD	dwLevelQuestSum;
	int		cnt,k;

	for(cnt=0;cnt<SIN_MAX_USE_SKILL;cnt++) {
		k=(cnt+5)<<2;
		dwSkilChkSum += sinSkill.UseSkill[cnt].UseSkillCount*k;
		dwSkilChkSum += sinSkill.UseSkill[cnt].Point*k;
		dwSkilChkSum += sinSkill.UseSkill[cnt].Mastery*k;
		dwSkilChkSum += sinSkill.UseSkill[cnt].GageLength*k;
		dwSkilChkSum += sinSkill.UseSkill[cnt].Skill_Info.UseStamina[0]*k;
		dwSkilChkSum += sinSkill.UseSkill[cnt].Skill_Info.UseStamina[1]*k;
	}

	dwLevelQuestSum = sinQuest_levelLog&0x576321cc;

	//화면 렌더링 함수
	smPlayD3D( pX, pY, pZ, (ViewAnx&ANGCLIP), (any&ANGCLIP), (anz&ANGCLIP) );

	dwChkSum = 0;
	for(cnt=0;cnt<SIN_MAX_USE_SKILL;cnt++) {
		k=(cnt+5)<<2;
		dwChkSum += sinSkill.UseSkill[cnt].UseSkillCount*k;
		dwChkSum += sinSkill.UseSkill[cnt].Point*k;
		dwChkSum += sinSkill.UseSkill[cnt].Mastery*k;
		dwChkSum += sinSkill.UseSkill[cnt].GageLength*k;
		dwChkSum += sinSkill.UseSkill[cnt].Skill_Info.UseStamina[0]*k;
		dwChkSum += sinSkill.UseSkill[cnt].Skill_Info.UseStamina[1]*k;
	}

	if ( dwSkilChkSum!=dwChkSum ) {
		//스킬 변경 됐음 , 수상한점 발견
		SendSetHackUser3( 8540 , dwSkilChkSum , dwChkSum );
	}

	if ( dwLevelQuestSum!=(sinQuest_levelLog&0x576321cc) ) {
		//퀘스트 코드변경 , 수상한점 발견
		SendSetHackUser3( 8820 , (dwLevelQuestSum&0x576321cc) , sinQuest_levelLog );
	}


	int mapY;
	int x,z,y;


	GetMoveLocation( 0 , 0, -(dist<<FLOATNS), anx, any, 0 );

	x = lpCurPlayer->pX+GeoResult_X;
	y = lpCurPlayer->pY+GeoResult_Y;
	z = lpCurPlayer->pZ+GeoResult_Z;

	PlayFloor=0;
	y=0;

	if ( !DebugPlayer ) {
/*
		if 	( lpCurPlayer->OnStageField>=0 && StageField[ lpCurPlayer->OnStageField ]->FieldCode<16 ) {

			if ( smGameStage[0] ) {
				mapY = (smGameStage[0]->GetHeight( x , z  ));
				if ( mapY>CLIP_OUT ) y++;
			}
			if ( smGameStage[1] ) {
				mapY = (smGameStage[1]->GetHeight( x , z  ));
				if ( mapY>CLIP_OUT ) y++;
			}
		}
		else {
*/
			if ( smGameStage[0] ) {
				mapY = (smGameStage[0]->GetHeight2( x , z  ));
				if ( mapY>CLIP_OUT ) y++;
			}
			if ( smGameStage[1] ) {
				mapY = (smGameStage[1]->GetHeight2( x , z  ));
				if ( mapY>CLIP_OUT ) y++;
			}
//		}
		if ( !y ) PlayFloor = 64;
	}


	//크리티칼 섹션 해제
	smLeaveTextureCriticalSection();
	LeaveCriticalSection( &cDrawSection );

	//속도 변수 UnLock
	UnlockSpeedProtect( lpCurPlayer );

}



extern int DispPolyCnt;			// 실재 출력된 폴리곤 카운터 (통계용)
extern int GetMouseMapPoint( int x, int y );

extern int xframe;

int GetPlayMouseAngle()
{
	int ax,az,ay;

	ax = xPs - (WinSizeX>>1);
	az = yPs - (WinSizeY>>1);
	ay = GetRadian2D( 0,0, ax, -az );

	return ay+any;
}

int GetActionGame_PlayMouseAngle()
{
	int ax,az,ay;

	ax = xPs - lpCurPlayer->RendPoint.x;
	az = yPs - lpCurPlayer->RendPoint.x;
	ay = GetRadian2D( 0,0, ax, -az );

	return ay;;
}


int SetMousePlay( int flag )
{
	int ax,az,ay;
	char szBuff[256];
	int	cnt;
	//POINT	pmCursor;

	if ( GameMode!=2 ) return TRUE;

	if ( !lpCurPlayer->MotionInfo || dwNextWarpDelay || lpCurPlayer->PlayStunCount ) return FALSE;

	if ( lpCurPlayer->MotionInfo->State == CHRMOTION_STATE_DEAD ) return FALSE;


	#ifdef _SINBARAM
	//######################################################################################
	//작 성 자 : 오 영 석
	if( g_IsDxProjectZoomIn <= 0 )
	{
		//pmCursor.x = xPs; 
		//pmCursor.y = yPs; 
		dsMenuCursorPos( &pCursorPos , 2 );
		if ( StopCharMotion(pCursorPos.x,pCursorPos.y)==TRUE ) {
			if ( lpCurPlayer->MotionInfo->State<0x100 && lpCurPlayer->MotionInfo->State!=CHRMOTION_STATE_STAND ) {
				lpCurPlayer->SetMotionFromCode( CHRMOTION_STATE_STAND );
				lpCurPlayer->MoveFlag = FALSE;
			}
			flag = 0;
			//return TRUE;
		}
		dsMenuCursorPos( &pCursorPos , 0 );
	}
	//######################################################################################
	#endif

	switch( flag ) {
	case 1:

#ifdef	_NEW_PARTRADE

		if ( EachTradeButton && chrEachMaster ) {
			cnt = GetPartyTradeButtonPos( xPs , yPs );
			if ( cnt>=0 ) {
				EachTradeButton = 0;
				switch( cnt ) {
				case 0:
					//교환 거래 거리 확인
					if ( GetTradeDistanceFromCode( chrEachMaster->dwObjectSerial )==TRUE ) {
						//아이템 교환 신청
						SendRequestTrade( chrEachMaster->dwObjectSerial , 0 );
						wsprintf( szBuff , mgRequestTrade , chrEachMaster->smCharInfo.szName );
					}
					else {
						wsprintf( szBuff , mgRequestTrade2 , chrEachMaster->smCharInfo.szName );
					}
					return TRUE;

				case 1:
					//동료 신청
					wsprintf( szBuff , "//party %s" , chrEachMaster->smCharInfo.szName );
					SendChatMessageToServer( szBuff );
					return TRUE;

				case 2:
					//친구 등록
					InterfaceParty.latest_Insert(chrEachMaster->smCharInfo.szName);	//귓속말한사람의 리스트 추가.
					InterfaceParty.friend_Insert(chrEachMaster->smCharInfo.szName);		//친구목록

					InterfaceParty.Main_menuSet(2);	//메인메뉴 퀘스트, 동료, 친구를 바꿈.
					InterfaceParty.chat_changeMENU(1);	//친구중 최근목록(0), 친구목록(1), 거부자목록을바꿈.
					if ( InterfaceParty.PartyPosState==PARTY_NONE )	ShowParty();
					return TRUE;

				case 3:
					//클랜 메세지 보내기
					//SendClanJoinService( 1 , chrEachMaster );
					g_IsCheckClanMember(chrEachMaster);
					return TRUE;

				}
			}
		}


#else
		if ( EachTradeButton==2 && chrEachMaster ) {
			EachTradeButton = 0;

			//교환 거래 거리 확인
			if ( GetTradeDistanceFromCode( chrEachMaster->dwObjectSerial )==TRUE ) {
				//아이템 교환 신청
				SendRequestTrade( chrEachMaster->dwObjectSerial , 0 );
				//wsprintf( szBuff , "%s님께 거래를 신청했습니다" , chrEachMaster->smCharInfo.szName );
				wsprintf( szBuff , mgRequestTrade , chrEachMaster->smCharInfo.szName );
			}
			else {
//				wsprintf( szBuff , "%s님은 거리가 멀어서 거래 신청을 할수 없습니다" , chrEachMaster->smCharInfo.szName );
				wsprintf( szBuff , mgRequestTrade2 , chrEachMaster->smCharInfo.szName );
			}

			AddChatBuff( szBuff );
			break;
		}
#endif
		//안내지도 띄우기
		if ( GetFieldMapCursor()==TRUE && cInterFace.sInterFlags.MapOnFlag ) {
			if ( lpCurPlayer->smCharInfo.Level<20 ) {
				sinCheck_ShowHelpMap();
			}
			break;
		}

		if ( !ActionGameMode ) {

			//마우스 버튼 누름
			if ( lpCharSelPlayer || lpSelItem ) {
				SelMouseButton = 1;				//왼쪽 버튼
				TraceAttackPlay();
				AutoCameraFlag = FALSE;
			}
			else {
				if ( MsTraceMode && !lpCharSelPlayer && !lpSelItem ) {
					if ( lpCurPlayer->MotionInfo->State != CHRMOTION_STATE_ATTACK && 
						lpCurPlayer->MotionInfo->State!=CHRMOTION_STATE_SKILL)
						lpCurPlayer->SetTargetPosi( 0,0 );
					CancelAttack();
				}
				AutoCameraFlag = TRUE;
			}
		}
		else {
			ActionMouseClick[0] = 1;
		}

		AutoMouse_WM_Count++;				//자동 마우스 감지용
		//TraceMode_DblClick = 0;

		if ( SkillMasterFlag ) {
			//스킬 배우기를 마친다 
			sinSkillMasterClose();
		}

		lpCurPlayer->MoveFlag = TRUE;
		DispEachMode = 0;

		if ( hFocusWnd ) {
			GetWindowText( hFocusWnd , szBuff , 240 );
			if ( szBuff[0]==0 ) {
				hFocusWnd = 0;
				szLastWhisperName[0]= 0;
				LastWhisperLen = 0;
				InterfaceParty.chat_WhisperPartyPlayer_close();
				chatlistSPEAKERflagChg( 0 );		//클랜 채팅 종료
				SetIME_Mode( 0 );		//IME 모드 전환
				ChatScrollPoint = 0;
				//ImmSetConversionStatus( hImc , IME_CMODE_ALPHANUMERIC,IME_CMODE_ALPHANUMERIC );
				//ImmSetConversionStatus( hImcEdit , IME_CMODE_ALPHANUMERIC,IME_CMODE_ALPHANUMERIC );
			}
		}
		break;

	case 0:
		//마우스 버튼 놓음
		//if ( !lpCharSelPlayer && !lpSelItem ) {
		if ( Moving_DblClick ) {
			TraceMode_DblClick = TRUE;
			Moving_DblClick = 0;
			lpCurPlayer->MoveFlag = TRUE;			
			ActionMouseClick[0] = 2;

		}
		else {
			if ( !lpCharMsTrace && !lpMsTraceItem ) {
				lpCurPlayer->SetAction( 0 );
			}
		}
		return TRUE;

	case 11:
		if ( hFocusWnd ) {
			GetWindowText( hFocusWnd , szBuff , 240 );
			if ( szBuff[0]==0 ) {
				hFocusWnd = 0;
				SetIME_Mode( 0 );		//IME 모드 전환
				ChatScrollPoint = 0;
				//ImmSetConversionStatus( hImc , IME_CMODE_ALPHANUMERIC,IME_CMODE_ALPHANUMERIC );
				//ImmSetConversionStatus( hImcEdit , IME_CMODE_ALPHANUMERIC,IME_CMODE_ALPHANUMERIC );
			}
		}
		//마우스 오르쪽 버튼 누름
		if ( sinSkill.pRightSkill && lpCurPlayer->MotionInfo->State != CHRMOTION_STATE_ATTACK && 
			lpCurPlayer->MotionInfo->State!=CHRMOTION_STATE_SKILL ) {

			if ( lpCurPlayer->MotionInfo->State!=CHRMOTION_STATE_EAT) {
				//스킬 사용
				if ( OpenPlaySkill( sinSkill.pRightSkill ) ) 
					break;
			}
		}

		if ( !ActionGameMode ) {
			if ( lpCharSelPlayer ) { //|| lpSelItem ) {
				SelMouseButton = 2;				//오른 버튼
				TraceAttackPlay();
				AutoCameraFlag = FALSE;
				lpCurPlayer->MoveFlag = TRUE;
				DispEachMode = 0;
			}
		}
		else {
			ActionMouseClick[1] = TRUE;
		}


		AutoMouse_WM_Count++;				//자동 마우스 감지용
		break;
	case 10:
		//마우스 오르쪽 버튼 놓음
		if ( !lpCharMsTrace && !lpMsTraceItem ) {
			lpCurPlayer->SetAction( 0 );
		}
		break;

	case 3:
		//가운데 버튼 누른 상태에서 이동
		//	msX = xPs-msXo;
		//msY = yPs-msXo;

		ay = msY*4;//>>1;
		ax = msX*8;

		if ( ay ) {
			if ( !CaTarDist ) CaTarDist=dist;
			CaTarDist -= ay;
			//######################################################################################
			//작 성 자 : 오 영 석
			if( CaTarDist < 100 )
			{
				if( anx <= 40 )
				{
					if( CaTarDist < 40 )
						CaTarDist = 40;
				}
				else
					CaTarDist = 100;
			}

			if( CaTarDist > 440 ) { CaTarDist = 440; }
			//######################################################################################
		}

		if ( ax ) {
			if ( CameraInvRot ) ax = -ax;

			az = ANGLE_45>>1;
			if ( ax<-az ) ax=-az;
			if ( ax>az ) ax=az;

			CaMovAngle += ax;
		}

		return TRUE;

	case 4:
		if ( DebugPlayer ) {
				/*
				msXo = WinSizeX>>1;
				msYo = WinSizeY>>1;

				SetCursorPos( msXo, msYo );
				*/

				ay = msY*2;
				ax = msX*2;

				any += ax;
				anx += ay;

				any &=ANGCLIP;
				anx &=ANGCLIP;

		}
		return TRUE;
	}

	if ( lpCurPlayer->MotionInfo->State==CHRMOTION_STATE_ATTACK ||
		lpCurPlayer->MotionInfo->State==CHRMOTION_STATE_SKILL ||
		lpCurPlayer->MotionInfo->State==CHRMOTION_STATE_YAHOO 
		) return FALSE;


	if ( lpCurPlayer->MoveFlag ) {

		if ( MsTraceMode ) {
			lpCurPlayer->Angle.y = GetMouseSelAngle();
			//if ( ay>=0) lpCurPlayer->Angle.y = ay;
		}
		else {
			if ( ActionGameMode )
				lpCurPlayer->Angle.y = GetActionGame_PlayMouseAngle();
			else
				lpCurPlayer->Angle.y = GetPlayMouseAngle();
		}
	}

	return TRUE;
}

extern int TestTraceMatrix();

int _stdcall InitD3D(HWND hWnd)
{

	hwnd = hWnd;

	//smSetMeshReload( FALSE );
#ifndef _WINMODE_DEBUG

#ifdef	_LANGUAGE_TAIWAN
	WindowMode = FALSE;
#endif
/*
#ifdef	_LANGUAGE_CHINESE	//윈도우 모드
	WindowMode = FALSE;
#endif
*/
#ifdef	_LANGUAGE_THAI
	WindowMode = FALSE;
#endif
/*
#ifdef	_LANGUAGE_ENGLISH
#ifndef _LANGUAGE_PHILIPIN
	WindowMode = FALSE;
#endif	
#endif
*/
#ifdef	_LANGUAGE_BRAZIL
	WindowMode = FALSE;
#endif
#ifdef	_LANGUAGE_ARGENTINA
	WindowMode = FALSE;
#endif

/*
#ifdef	_LANGUAGE_VEITNAM	//윈도우 모드
	WindowMode = FALSE;
#endif
*/


#endif

	MidX = WinSizeX	/2;
	MidY = WinSizeY	/2;
	MidY -= 59;
	viewdistZ = WinSizeX;


    // Direct3D 생성
    if ( !CreateD3D() )
        return FALSE;

    // 비디오 모드를 640x480 16비트 모드로 설정
//    if ( !SetDisplayMode( hWnd, WinSizeX, WinSizeY, 16 ) )
    if ( !SetDisplayMode( hWnd, WinSizeX, WinSizeY, WinColBit ) )
        return FALSE;
/*
#ifdef _npGAME_GUARD

	//nProtect 실행 //디버그시 동작못하게 막음 kyle
	if ( Start_nProtect(smConfig.DebugMode)==FALSE ) {
		if ( szExitInfo_URL[0] )
			ShellExecute(NULL, NULL, szExitInfo_URL, NULL, NULL, NULL);
		return FALSE;
	}
	Sleep(1000);

#endif
*/
    // Viewport 생성
   if ( !CreateViewport() )
       return FALSE;

	TestTraceMatrix();

    // 랜더링 초기화
    InitRender();
	InitTexture();

	//텍스쳐 품질 모드 설정 ( 0 - 3 )
	smSetTextureLevel( smConfig.TextureQuality );

	//######################################################################################
	//작 성 자 : 오 영 석
	LPD3DPRIMCAPS lpD3dCaps = &lpD3DDeviceDesc->Desc.dpcTriCaps;
	if( TextureSwapMode == FALSE && lpD3DDeviceDesc->bIsHardware )
	{
		if( lpD3dCaps->dwTextureFilterCaps & D3DPTFILTERCAPS_LINEARMIPLINEAR )
		{
            MipMapModeCreate = NOSQUARE_MODE;
            if( lpD3dCaps->dwTextureCaps & D3DPTEXTURECAPS_SQUAREONLY )
                MipMapModeCreate = SQUARE_MODE;
		}
	}
	
	if( lpD3dCaps->dwRasterCaps & D3DPRASTERCAPS_FOGTABLE ) // && (lpD3dCaps->dwRasterCaps & D3DPRASTERCAPS_WFOG) 
	{
        smRender.m_FogIsRend = TRUE;

		D3DMATRIX matProj;
		D3D_SetProjectionMatrix( matProj, (g_PI/4.4f), (float(WinSizeX) / float(WinSizeY)), 20.f, 4000.f );
		lpD3DDevice->SetTransform( D3DTRANSFORMSTATE_PROJECTION, &matProj );

		lpD3DDevice->SetRenderState( D3DRENDERSTATE_FOGTABLEMODE, D3DFOG_LINEAR );
	}
	//######################################################################################

	HBITMAP hBitmap;
	HDC	hdcBitmap;

	//////////////////// 로딩 화면 출력 ///////////////////////
	hdc = GetDC( hwnd );
	// 말 상자 그림 로드 
#ifdef _LANGUAGE_KOREAN
	hBitmap = LoadBitmap( hinst, "IDB_LOADING" );
#else
#ifdef _LANGUAGE_THAI
	hBitmap = LoadBitmap( hinst, "IDB_THAI" );
#else
#ifdef _LANGUAGE_PHILIPIN
	hBitmap = LoadBitmap( hinst, "IDB_LOGOPHIL" );
#else
#ifdef _LANGUAGE_VEITNAM
	hBitmap = LoadBitmap( hinst, "IDB_VIETNAM" );
#else
#ifdef _LANGUAGE_BRAZIL
	hBitmap = LoadBitmap( hinst, "IDB_LOADING_BRAZIL" );	//브라질 로고
#else
#ifdef _LANGUAGE_ARGENTINA
	hBitmap = LoadBitmap( hinst, "IDB_LOADINGA" );		//아르헨티나 로고
#else
#ifdef	_LANGUAGE_CHINESE
	hBitmap = LoadBitmap( hinst, "IDB_LOGOCHINA" );
#else
#ifdef _LANGUAGE_ENGLISH
	hBitmap = LoadBitmap( hinst, "IDB_LOGOSUBA" );
#else
#ifdef _LANGUAGE_JAPANESE
	hBitmap = LoadBitmap( hinst, "IDB_LOADING_JAPAN" );
#else
	hBitmap = LoadBitmap( hinst, "IDB_LOADING" );
#endif
#endif
#endif
#endif
#endif
#endif
#endif	
#endif
#endif
	hdcBitmap = CreateCompatibleDC(hdc);
	SelectObject( hdcBitmap, hBitmap );
	
#ifdef _LANGUAGE_JAPANESE
	//RECT Client_rect;
	//GetWindowRect(::GetDesktopWindow(), &Client_rect);
	//int width = Client_rect.right - Client_rect.left;
	//int height = Client_rect.bottom - Client_rect.top;
	//hBitmap = ::CreateCompatibleBitmap(hdc, width, height);

/*
	LPDIRECTDRAWSURFACE4	LogoImage;
	LogoImage = LoadDibSurfaceOffscreen( "arario.bmp");

	if(LogoImage)
	{
		LogoImage->SetColorKey( DDCKEY_COLORSPACE , 0 );
		
		// 랜드링 시작
		BeginRender();

		DrawSprite( WinSizeX/2-400 , WinSizeY/2-300, LogoImage, 0, 0, 800, 600);
		
		Flip();

		Sleep(2000);	// 2seconds

		LogoImage->Release();
	}
*/

	BitBlt( hdc , WinSizeX/2-400 , WinSizeY/2-300, 800, 600 , hdcBitmap , 0,0, SRCCOPY );	
	Sleep(3000);	// 3seconds
	
//	BitBlt( hdc , 0, 0, smConfig.ScreenSize.x, smConfig.ScreenSize.y , hdcBitmap , 0,0, SRCCOPY );	
//	int nx = smConfig.ScreenSize.x / 800;
//	int ny = smConfig.ScreenSize.y / 600;
//	StretchBlt(hdc, 0, 0, RECT. , smConfig.ScreenSize.y*2, hdcBitmap, 0, 0, smConfig.ScreenSize.x*nx,smConfig.ScreenSize.y, SRCCOPY);
	
#else
	BitBlt( hdc , WinSizeX/2-265 , WinSizeY/2-180, 498,300 , hdcBitmap , 0,0, SRCCOPY );
#endif
	
#ifdef _LANGUAGE_ARGENTINA
	Sleep(3000);	// 3seconds
#endif

	DeleteDC( hdcBitmap );
	DeleteObject( hBitmap );
	ReleaseDC( hwnd ,hdc );

	Check_CodeSafe((DWORD)GameInit );

	///////////////////////////////////////////////////////////


//	Flip();

	//////////////////////////////////////

//	InitMap();

//	smInitColorZClip( 0*256+255 , 6*256+255 ,138*256+255 );
//	smInitColorZClip( 214*256+255 , 255*256+255 ,239*256+255 );
//	smInitColorZClip( 0*256+255 , 0*256+255 ,255*256+255 );

	InitSoundEffect( hwnd );	//사운드 초기화
	InitPatterns();

	//주인공 구조체 초기화
	lpCurPlayer = InitRotPlayer();

	return Code_VRamBuffOpen();			//코드버퍼 초기화

}

void _stdcall CloseD3d()
{

	//TermDX_Wav2();			   //말기화
	CloseRotPlayer();			//주인공 구조체 포인터 종료

	CloseSoundEffect();

	//CloseMaterial();
	CloseTexture();

//	delete pat3d;
	
//	if ( hOBJ3DTexture )
//		hOBJ3DTexture->Release();

//	CloseMap();
    ReleaseD3D();
}


//게임 초기화
int GameInit()
{
	//######################################################################################
	//작 성 자 : 오 영 석
	g_IsReadTextures = 1;
	//######################################################################################

	//서버에 권한 설정
	if ( smConfig.DebugMode ) SendAdminMode( TRUE );

	npSetUserID( UserAccount );		//게임가드에 ID 통보

	dwPlayTime = GetCurrentTime();
	dwMemError = dwMemError^dwPlayTime;
	Check_nProtect();					//nProtect 확인

	// 메트리얼 초기화
	InitMaterial();
	smRender.SetMaterialGroup( smMaterialGroup );			//기본 메트리얼 그룹

	//ZeroMemory( &lpCurPlayer->smCharInfo , sizeof( smCHAR_INFO ) );
	ReformCharForm();

	InitEffect();			//호 이펙트 초기화

	InitMotionBlur();		//모션 부려 초기화
	InitBackGround();		//배경 초기화

	Check_CodeSafe( (DWORD)CloseD3d );	//코드 보호 실행

	InitStage();
	InitPat3D();

#ifdef _XTRAP_GUARD_4_CLIENT
	XTrap_C_SetUserInfo(UserAccount,szConnServerName,lpCurPlayer->smCharInfo.szName , "class name" ,1);	//XTrapD5
#endif

	//######################################################################################
	//작 성 자 : 오 영 석
	CreateItem2PassTexture();
	//######################################################################################

	CheckCharForm();

	//메모리 첵크 초기화
	//InitKeepMemFunc();

	//메모리 전체 첵크
	//CheckKeepMemFull();


#ifdef _SINBARAM
	lpDDSMenu = 0;
	sinInit();
#else
	lpDDSMenu =  LoadDibSurfaceOffscreen( smConfig.szFile_Menu );
#endif

	//######################################################################################
	//작 성 자 : 오 영 석
	g_fWinSizeRatio_X = float(WinSizeX) / 800.f;
	g_fWinSizeRatio_Y = float(WinSizeY) / 600.f;

	CreateBeforeFullZoomMap();
	CreateFontImage();
	//######################################################################################

	TraceCameraPosi.x = lpCurPlayer->pX;
	TraceCameraPosi.y = lpCurPlayer->pY;
	TraceCameraPosi.z = lpCurPlayer->pZ;

	TraceTargetPosi.x = lpCurPlayer->pX;
	TraceTargetPosi.y = lpCurPlayer->pY;
	TraceTargetPosi.z = lpCurPlayer->pZ;

	InitMessageBox();

	//렌더링 기본 값
	smRender.SMMULT_PERSPECTIVE_HEIGHT = RENDCLIP_DEFAULT_MULT_PERSPECTIVE_HEIGHT;
	MidX = WinSizeX	/2;
	MidY = WinSizeY	/2;
	viewdistZ = WinSizeX;

	//######################################################################################
	//작 성 자 : 오 영 석
	g_IsReadTextures = 1;
	//######################################################################################

	ReadTextures();

	CheckOftenMeshTextureSwap();	//자주쓸 메시 텍스쳐 스왑첵크

	//음악 연주
	if ( smConfig.BGM_Mode ) {
		if ( StageField[0] )
			PlayBGM_Direct( StageField[0]->BackMusicCode  );
		else {
			OpenBGM("wav\\bgm\\Field - Desert - Pilgrim.bgm");
			PlayBGM();
		}
	}
	CharPlaySound( lpCurPlayer );
	StartEffect( lpCurPlayer->pX,lpCurPlayer->pY,lpCurPlayer->pZ, EFFECT_GAME_START1 );
	RestartPlayCount = 700;		//10초 동안 무적


	hFocusWnd = 0;
	szLastWhisperName[0]= 0;
	LastWhisperLen = 0;
	InterfaceParty.chat_WhisperPartyPlayer_close();
	chatlistSPEAKERflagChg( 0 );		//클랜 채팅 종료
	SendMessage( hTextWnd , EM_SETLIMITTEXT , 78 , 0 );			//채팅 80글자 제한

	MouseButton[0] = 0;
	MouseButton[1] = 0;
	MouseButton[2] = 0;

	//주인공 캐릭터 포인터 변경		//kyle xtrapHeap

#ifdef _XTRAP_GUARD_4_CLIENT //HEAP MEMORY TEST
	XTrap_CE1_Func11_Protect( &sinChar, sizeof(sinChar) );	//보호영역 무결성 체크
#endif

#ifdef _XIGNCODE_CLIENT
	// 박재원 - XignCode
	Xigncode_Client_Start();
#endif

	smCHAR	*lpTempChar;
	lpTempChar =  SelectRotPlayer( lpCurPlayer );
	if ( lpTempChar ) {
		lpCurPlayer = lpTempChar;
		sinChar = &lpCurPlayer->smCharInfo;
	}
	lpTempChar =  SelectRotPlayer( lpCurPlayer );
	if ( lpTempChar ) {
		lpCurPlayer = lpTempChar;
		sinChar = &lpCurPlayer->smCharInfo;
	}

#ifdef _XTRAP_GUARD_4_CLIENT //HEAP MEMORY TEST
	XTrap_CE1_Func12_Protect( &sinChar, sizeof(sinChar) );	//보호영역 보호
	XTrap_CE1_Func13_Free( &sinChar, sizeof(sinChar) );		//보호영역 해제
#endif

	SetIME_Mode( 0 );		//IME 모드 전환

	//스킬보호값 전부 초기화
	ReformSkillInfo();


	HoMsgBoxMode = 1;
	SetMessageFrameSelect( HoMsgBoxMode );		//호메세지창 프레임 모드

	//######################################################################################
	//작 성 자 : 오 영 석
	CreateWinIntThread();
	//######################################################################################

	return TRUE;
}

//게임 닫기
int GameClose()
{

	#ifdef _XIGNCODE_CLIENT
	//박재원 - XignCode
	ZCWAVE_Cleanup();
	ZCWAVE_SysExit();
	#endif

	if ( lpDDSMenu ) lpDDSMenu->Release();

	//메모리 첵크 종료
	//CloseKeepMem();

	ClosePat3D();
	CloseBackGround();
	CloseStage();

#ifdef _SINBARAM
	sinClose();
#endif
	CloseMaterial();

	//######################################################################################
	//작 성 자 : 오 영 석
	DestroyWinIntThread();
	//######################################################################################

	if ( BellatraEffectInitFlag ) {
		DestroyBellatraFontEffect();
	}

	return TRUE;
}

//로고가 있으면 로고를 화면에 보여준다 (0.8초)
int DIsplayLogoImage()
{
	LPDIRECTDRAWSURFACE4	LogoImage;
	LPDIRECTDRAWSURFACE4	LogoImage2;

	LogoImage = LoadDibSurfaceOffscreen( "Image\\Logo.bmp");
	LogoImage2 = LoadDibSurfaceOffscreen( "Image\\Logo2.bmp");
	if ( LogoImage ) {
		LogoImage->SetColorKey( DDCKEY_COLORSPACE , 0 );
#ifdef _LANGUAGE_KOREAN
		if ( LogoImage2 )
			LogoImage2->SetColorKey( DDCKEY_COLORSPACE , 0 );
#endif

		// 랜드링 시작
		BeginRender();
#ifdef _LANGUAGE_BRAZIL
		DrawSprite( WinSizeX/2-400 , WinSizeY/2-300, LogoImage, 0, 0, 800, 600);
#else
#ifdef _LANGUAGE_ARGENTINA
		DrawSprite( WinSizeX/2-250 , WinSizeY/2-150, LogoImage, 0, 0, 500, 300);
#else
#ifdef _LANGUAGE_CHINESE
		DrawSprite( WinSizeX/2-150 , WinSizeY/2-125, LogoImage, 0, 0, 300, 250);
#else
		DrawSprite( WinSizeX/2-250 , WinSizeY/2-150, LogoImage, 0, 0, 500, 300);
#endif
#endif
#endif

		//DrawSprite( 150, 150, LogoImage, 0, 0, 500, 300);
#ifdef _LANGUAGE_KOREAN
		if ( LogoImage2 )
			DrawSprite( WinSizeX-100, 16, LogoImage2, 0, 0, 80, 45);
#endif
		Flip();

#ifdef	_LANGUAGE_CHINESE
		Sleep(1000);
#else
		Sleep(600);
#endif
		LogoImage->Release();
	}
	return TRUE;
}


//게임 선택
int SetGameMode( int mode )
{
	int OldMode;
	
	OldMode = GameMode;
	GameMode = mode;
	//신바람에 캐릭터 포인터 설정

#ifdef _XTRAP_GUARD_4_CLIENT //HEAP MEMORY TEST
	XTrap_CE1_Func11_Protect( &sinChar, sizeof(sinChar) );	//보호영역 무결성 체크
#endif

	sinChar = &lpCurPlayer->smCharInfo;

#ifdef _XTRAP_GUARD_4_CLIENT //HEAP MEMORY TEST
	XTrap_CE1_Func12_Protect( &sinChar, sizeof(sinChar) );	//보호영역 보호
	XTrap_CE1_Func13_Free( &sinChar, sizeof(sinChar) );		//보호영역 해제
#endif

	switch( OldMode ) {
	case 1:
		/*
		lpPlaySelect->Close();
		if (lpPlaySelect) { delete lpPlaySelect; lpPlaySelect=0; }
		*/
//국문용
//#ifdef _npGAME_GUARD
//		KeyCrypt.OnExitKeyCrypt();
//#endif

/* temp by 2neo
#ifndef _WINMODE_DEBUG
#ifndef _nProtect_For_Debug
		KeyCrypt.OnExitKeyCrypt();		//키크립트
#endif
#endif
*/

		CloseOpening();
		CloseMaterial();
		break;

	case 2:
		GameClose();
		break;

	}

	switch( GameMode ) {
	case 1:
		//######################################################################################
		//작 성 자 : 오 영 석
		SetDxProjection( (g_PI/4.4f), WinSizeX, WinSizeY, 20.f, 4000.f );
		//######################################################################################

		//######################################################################################
		//작 성 자 : 오 영 석
		smRender.CreateRenderBuff(CameraSight);
		//######################################################################################


		//lpPlaySelect = new ccsSELECT();
		// 메트리얼 초기화
		InitMaterial();
		smRender.SetMaterialGroup( smMaterialGroup );			//기본 메트리얼 그룹
		InitEffect();			//호 이펙트 초기화

		DIsplayLogoImage();			//로고를 보여준다
//국문용
//#ifdef _npGAME_GUARD
//		KeyCrypt.OnLoadKeyCrypt();
//#endif
		//키크립트

/* temp by 2neo
#ifndef _WINMODE_DEBUG
#ifndef	_nProtect_For_Debug
		KeyCrypt.OnLoadKeyCrypt();
#endif
#endif
*/


		InitOpening();
		//렌더링 와드클리핑 모드
		smRender.SMMULT_PERSPECTIVE_HEIGHT = RENDCLIP_WIDE_MULT_PERSPECTIVE_HEIGHT;
		MidX = WinSizeX	/2;
		MidY = WinSizeY	/2;

		// 실행중인 프로세스를 검사한다.
		if ( CheckCrackProcess(TRUE) ) quit = 1;

		if( IsCreateNewRenderTarget() )
			SetFilterEffect( FILTER_EFFECT_SET_BRIGHT_CONTRAST, 180 );

		dwM_BlurTime = 0;

		//Init_ParkPlayer();
		//Play_ParkPlayer( "mms://hapkido.re.kr/hapkido/8833/movie_158.asf" , 0,0, 400,400 , -1 );
		//Play_ParkPlayer( "mms://hapkido.re.kr/hapkido/8833/movie_158.asf" , 200,200, 300,300 , 150 );

//국문용
//#ifdef _npGAME_GUARD
//		KeyCrypt.OnRegisterwindow(hwnd);
//		SetHwnd_nProtect(hwnd);
//#endif
//키크립트

/* temp by 2neo
#ifndef _WINMODE_DEBUG
#ifndef _nProtect_For_Debug
		KeyCrypt.OnExitKeyCrypt();		//키크립트
#endif
#endif
*/


		break;

	case 2:
		if( IsCreateNewRenderTarget() )
			SetFilterEffect( FILTER_EFFECT_SET_BRIGHT_CONTRAST, 160 );

		//######################################################################################
		//작 성 자 : 오 영 석
		if( smRender.m_GameFieldView )
		{
			smRender.m_GameFieldViewStep = 22;
			smRender.SetGameFieldViewStep();
		}
		//######################################################################################

		GameInit();

		// 실행중인 프로세스를 검사한다.
		if ( CheckCrackProcess() ) quit = 1;
		break;
	}

	WaveCameraFlag = FALSE;

	return TRUE;
}


float xr=0;

int jcnt = 0;

float brt = 1;
float bs = 0;


int ox=0,oy=0,oz=0;

	int GoText = 0;

	char strBuff[240];
	char strBuff2[256];

int RestoreFlag = 0;

char *szRestore = "\301\366\261\335 \261\327\270\262 \265\245\300\314\305\270\270\246 \276\306\301\326 \277\255\275\311\310\367 \267\316\265\345\307\317\261\270 \300\326\275\300\264\317\264\331. \302\311\261\335\270\270 \261\342\264\331\270\256\274\274\277\344 !";

int RestoreAll()
{
	HDC hdc;

    // Back Surface 지우기
    DDBLTFX bltFx;
    ZeroMemory( &bltFx, sizeof(DDBLTFX) );
    bltFx.dwSize        = sizeof(DDBLTFX);
    bltFx.dwFillColor   = 0;
    lpDDSPrimary->Blt( NULL, NULL, NULL, DDBLT_WAIT | DDBLT_COLORFILL, &bltFx );

	VramTotal = 0;
	
	lpDDSPrimary->GetDC( &hdc );
	SetTextColor( hdc, RGB(255, 255, 255) );
	SetBkMode( hdc, TRANSPARENT );
	dsTextLineOut( hdc , 16,WinSizeY-32, szRestore , lstrlen( szRestore ) );
	lpDDSPrimary->ReleaseDC( hdc );

	RestoreTextures();
	if ( lpDDSMenu ) {
		lpDDSMenu->Release();
		lpDDSMenu =  LoadDibSurfaceOffscreen( smConfig.szFile_Menu );
	}

	sinReload();

	return TRUE;

}


//반복 확인 배경 카우터
//extern int DEBUG_ReCheckedCnt;

int NumPoly;
int Disp_tx,Disp_ty;
int Disp_rx,Disp_ry;
//채팅창 소팅용 박스
smCHAR	*lpCharMsgSort[OTHER_PLAYER_MAX];
int ChatMsgSortCnt;

extern int Debug_RecvCount1;
extern int Debug_RecvCount2;
extern int Debug_SendCount;

//텍스쳐 옵션을 기본값으로
int RestoreInterfaceTexture()
{
	int cnt;

	smRender.Color_R  = 0;
	smRender.Color_G  = 0;
	smRender.Color_B  = 0;
	smRender.Color_A  = 0;

	cnt = 0;

	lpD3DDevice->SetTextureStageState( cnt, D3DTSS_COLOROP,   D3DTOP_MODULATE    );
	lpD3DDevice->SetTextureStageState( cnt, D3DTSS_COLORARG1, D3DTA_TEXTURE );
	lpD3DDevice->SetTextureStageState( cnt, D3DTSS_COLORARG2, D3DTA_CURRENT ); 

	lpD3DDevice->SetTextureStageState( cnt, D3DTSS_ALPHAOP,   D3DTOP_MODULATE );
	lpD3DDevice->SetTextureStageState( cnt, D3DTSS_ALPHAARG2, D3DTA_CURRENT );
	lpD3DDevice->SetTextureStageState( cnt, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );

	lpD3DDevice->SetTexture( cnt, 0 );

	for( cnt=1; cnt < 8; cnt++ ) {
		lpD3DDevice->SetTextureStageState( cnt, D3DTSS_COLOROP, D3DTOP_DISABLE );
		lpD3DDevice->SetTextureStageState( cnt, D3DTSS_ALPHAOP,   D3DTOP_DISABLE );
		lpD3DDevice->SetTexture( cnt, 0 );
	}

	return TRUE;
}

//######################################################################################
//작 성 자 : 오 영 석
void VirtualDrawGameState(void)
{
	smRender.ClearLight();

	int BackDarkLevel;

	BackDarkLevel = DarkLevel;
	DarkLevel = 0;

	///////////////////////////////////////////////////////////////////////
	//텍스쳐 옵션을 기본값으로	
	RestoreInterfaceTexture();
	///////////////////////////////////////////////////////////////////////

	if( DisconnectFlag )
	{
		//서버와의 연결 끊어짐
		if ( DisconnectServerCode==0 ) {
			DrawMessage( MidX-64 ,  MidY, mgDiconnect ,36, BOX_ONE );
		}
		if ( DisconnectServerCode==1 ) {
			DrawMessage( MidX-64 ,  MidY, mgDiconnect1 ,36, BOX_ONE );
		}
		if ( DisconnectServerCode==2 ) {
			DrawMessage( MidX-64 ,  MidY, mgDiconnect2 ,36, BOX_ONE );
		}
		if ( DisconnectServerCode==3 ) {
			DrawMessage( MidX-64 ,  MidY, mgDiconnect3 ,36, BOX_ONE );
		}
		if ( DisconnectServerCode==4 ) {
			DrawMessage( MidX-64 ,  MidY, mgDiconnect4 ,36, BOX_ONE );
		}

#ifdef	_WINMODE_DEBUG
		if ( !smConfig.DebugMode && !quit && ((DWORD)DisconnectFlag+5000)<dwPlayTime ) quit=TRUE;
#else
		if ( !quit && ((DWORD)DisconnectFlag+5000)<dwPlayTime ) quit=TRUE;
#endif

	}
	else
	{
		if ( quit )
			DrawMessage( MidX-40 ,  MidY, mgCloseGame ,36, BOX_ONE );
		else {
			if ( dwCloseBoxTime && dwCloseBoxTime>dwPlayTime ) {
				DrawMessage( MidX-100 ,  MidY, mgCloseWindow ,36, BOX_ONE );
			}
			else {
				if ( dwBattleQuitTime ) {
					if ( (dwBattleQuitTime+5000)>dwPlayTime )
						DrawMessage( MidX-40 ,  MidY, mgCloseBattle ,36, BOX_ONE );
					else
						dwBattleQuitTime = 0;
				}
			}
		}
	}

	//
	if( lpD3DDevice )
	{ 
		lpD3DDevice->BeginScene();
		DrawCursor();
		lpD3DDevice->EndScene();
	}

	DarkLevel = BackDarkLevel;
	if ( Flip()==FALSE ) {
		DisconnectFlag = dwPlayTime;
		quit = 1;
	}
}
//######################################################################################


int DrawGameState()
{
	int i;
	HDC	hdc;
	int cnt,cnt2;//,len;
//	int x,y;
	int	y,DispBar,DispMaster;
//	POINT	ppoint;
	smCHAR	*lpChar;
	scITEM	*lpItem;
	RECT	ddRect;
	int BackDarkLevel;
	DWORD	dwColor;

/*
	ddRect.left   = 0;
	ddRect.right  = 640;
	ddRect.top    = 340;
	ddRect.bottom = 480;
*/
	ddRect.left   = 0;
	ddRect.right  = 800;
	ddRect.top    = 0;
	ddRect.bottom = 150;

//	lpDDSBack->BltFast(0,WinSizeY-120,lpDDSMenu,&ddRect,DDBLTFAST_WAIT|DDBLTFAST_SRCCOLORKEY);

	if ( lpDDSMenu ) i = lpDDSBack->BltFast(0,WinSizeY-150,lpDDSMenu,&ddRect,DDBLTFAST_WAIT|DDBLTFAST_SRCCOLORKEY);

	smRender.ClearLight();

	BackDarkLevel =DarkLevel;
	DarkLevel = 0;

	///////////////////////////////////////////////////////////////////////
	//텍스쳐 옵션을 기본값으로
	RestoreInterfaceTexture();

	if ( lpCharMsTrace && lpCharMsTrace->RendSucess ) {
		Disp_tx = lpCharMsTrace->RendPoint.x - 32;
		Disp_ty = lpCharMsTrace->RendPoint.y - 12;
//		Disp_ty = lpCharMsTrace->RendRect2D.top+16;
	}

	if ( lpCharSelPlayer && lpCharMsTrace!=lpCharSelPlayer ) {
		if ( lpCharSelPlayer->RendSucess ) {
			Disp_rx = lpCharSelPlayer->RendPoint.x - 32;
			Disp_ry = lpCharSelPlayer->RendPoint.y - 12;
//			Disp_ry = lpCharSelPlayer->RendRect2D.top+16;
		}
	}
/*
	for( cnt=0;cnt<OTHER_PLAYER_MAX;cnt++) {
		if ( chrOtherPlayer[cnt].Flag && chrOtherPlayer[cnt].RendSucess &&
			( chrOtherPlayer[cnt].PartyFlag || 
			( chrOtherPlayer[cnt].smCharInfo.State==smCHAR_STATE_ENEMY && chrOtherPlayer[cnt].smCharInfo.Brood==smCHAR_MONSTER_USER && chrOtherPlayer[cnt].smCharInfo.Next_Exp==lpCurPlayer->dwObjectSerial && chrOtherPlayer[cnt].smCharInfo.Life[0]>0 ) ) ) {

			i = 0;

			if ( lpCharMsTrace==&chrOtherPlayer[cnt] ) {
				x = Disp_tx;
				y = Disp_ty;
			}
			else {
				if ( lpCharSelPlayer==&chrOtherPlayer[cnt] ) {
					x = Disp_rx;
					y = Disp_ry;
				}
				else {
					x = chrOtherPlayer[cnt].RendPoint.x - 32;
					y = chrOtherPlayer[cnt].RendPoint.y - 12;
					i = 1;
				}
			}


			wsprintf( strBuff , "%s",chrOtherPlayer[cnt].smCharInfo.szName );
			len = lstrlen(strBuff);

		    lpD3DDevice->EndScene();
			chrOtherPlayer[cnt].DrawStateBar2( x+8,y+14 );
		    lpD3DDevice->BeginScene();

			x = (x+32)-(len>>1)*6;

			if ( i ) {
				lpDDSBack->GetDC( &hdc );
				SelectObject( hdc , hFont );
			    SetBkMode( hdc, TRANSPARENT );


				if ( chrOtherPlayer[cnt].smCharInfo.State==smCHAR_STATE_ENEMY && chrOtherPlayer[cnt].smCharInfo.Brood==smCHAR_MONSTER_USER && chrOtherPlayer[cnt].smCharInfo.Next_Exp==lpCurPlayer->dwObjectSerial ) {
					SetTextColor( hdc, RGB(0,0,0) );
					dsTextLineOut( hdc , x-1 , y-1 , strBuff , len );
					SetTextColor( hdc, RGB(255, 200, 160) );
					dsTextLineOut( hdc , x , y , strBuff , len );
				}
				else {
					SetTextColor( hdc, RGB(0,0,0) );
					dsTextLineOut( hdc , x-1 , y-1 , strBuff , len );
					SetTextColor( hdc, RGB(200, 255, 160) );
					dsTextLineOut( hdc , x , y , strBuff , len );
				}

				lpDDSBack->ReleaseDC( hdc );
			}
		}
	}
*/
	////////////////////////////////////////////////////////////////////////////
	lpChar = 0;
	lpItem =0;
/*
	lpDDSBack->GetDC( &hdc );
	SelectObject( hdc , hFont );
    SetBkMode( hdc, TRANSPARENT );

*/
	if ( lpMsTraceItem ) 
		lpItem = lpMsTraceItem;
	else
		lpItem = lpSelItem;
/*
	if ( lpCharMsTrace ) {
		lpChar = lpCharMsTrace;

		if ( lpChar->RendSucess ) {
			//Disp_tx = lpChar->RendPoint.x - 32;
			//Disp_ty = lpChar->RendPoint.y;

			//wsprintf( strBuff , "%s" , lpChar->smCharInfo.szName );

			len = lstrlen(strBuff);
			x = (Disp_tx+32)-(len>>1)*6;
			y = Disp_ty;

//			SetTextColor( hdc, RGB(0, 0, 0) );
//			dsTextLineOut( hdc , x-1 , y-1 , strBuff , len );


			if ( lpChar->smCharInfo.State==smCHAR_STATE_ENEMY )
				dwColor = RGB(255, 180, 180);
			else
				dwColor = RGB(180, 180, 255);

//			dsTextLineOut( hdc , x , y , strBuff , len );


			if ( !lpChar->szChatMessage[0] ) {
				DrawCharacterMessage( lpChar->RendPoint.x, lpChar->RendRect2D.top,
					lpChar->smCharInfo.szName , 0 , 0, 0 , dwColor  );
			}

			if ( lpChar->smCharInfo.Brood==smCHAR_MONSTER_USER && lpChar->smCharInfo.Next_Exp!=lpCurPlayer->dwObjectSerial ) {
				wsprintf( strBuff , "(%s)" , lpChar->smCharInfo.szModelName2+1 );
				len = lstrlen(strBuff);
				x = (Disp_tx+32)-(len>>1)*6;
				y = Disp_ty;

				SetTextColor( hdc, RGB(0, 0, 0) );
				dsTextLineOut( hdc , x-1 , y+11 , strBuff , len );
				SetTextColor( hdc, RGB(255, 220, 160) );
				dsTextLineOut( hdc , x , y+12 , strBuff , len );
			}
			else if ( lpChar->smCharInfo.ClassClan && lpChar->ClanInfoNum>=0 ) {
				//클랜 이름 표시 임시
				wsprintf( strBuff , "[%s]" , ClanInfo[lpChar->ClanInfoNum].ClanInfoHeader.ClanName );
				len = lstrlen(strBuff);
				x = (Disp_tx+32)-(len>>1)*6;
				y = Disp_ty;

				SetTextColor( hdc, RGB(0, 0, 0) );
				dsTextLineOut( hdc , x-1 , y+11 , strBuff , len );
				SetTextColor( hdc, RGB(160, 220, 220) );
				dsTextLineOut( hdc , x , y+12 , strBuff , len );

			}

		}
	}

	if ( lpCharSelPlayer && lpCharMsTrace!=lpCharSelPlayer ) {
		lpChar = lpCharSelPlayer;
		if ( lpChar->RendSucess ) {
			//Disp_tx = lpChar->RendPoint.x - 32;
			//Disp_ty = lpChar->RendPoint.y;

			//wsprintf( strBuff , "%s" , lpChar->smCharInfo.szName );

			len = lstrlen(strBuff);
			x = (Disp_rx+32)-(len>>1)*6;
			y = Disp_ry;

//			SetTextColor( hdc, RGB(0, 0, 0) );
//			dsTextLineOut( hdc , x-1 , y-1 , strBuff , len );
//			SetTextColor( hdc, RGB(255, 255, 255) );
//			dsTextLineOut( hdc , x , y , strBuff , len );

			if ( !lpChar->szChatMessage[0] ) {
				DrawCharacterMessage( lpChar->RendPoint.x, lpChar->RendRect2D.top,
					lpChar->smCharInfo.szName , 0 , 0, 0 , RGB(255, 255, 255)  );
			}

			if ( lpChar->smCharInfo.State==smCHAR_STATE_ENEMY && lpChar->smCharInfo.Brood==smCHAR_MONSTER_USER && lpChar->smCharInfo.Next_Exp!=lpCurPlayer->dwObjectSerial ) {
				wsprintf( strBuff , "(%s)" , lpChar->smCharInfo.szModelName2+1 );
				len = lstrlen(strBuff);
				x = (Disp_rx+32)-(len>>1)*6;
				y = Disp_ry;

				SetTextColor( hdc, RGB(0, 0, 0) );
				dsTextLineOut( hdc , x-1 , y+11 , strBuff , len );
				SetTextColor( hdc, RGB(255, 220, 160) );
				dsTextLineOut( hdc , x , y+12 , strBuff , len );
			}
			else if ( lpChar->smCharInfo.ClassClan && lpChar->ClanInfoNum>=0 ) {
				//클랜 이름 표시 임시
				wsprintf( strBuff , "[%s]" , ClanInfo[lpChar->ClanInfoNum].ClanInfoHeader.ClanName );
				len = lstrlen(strBuff);
				x = (Disp_rx+32)-(len>>1)*6;
				y = Disp_ry;

				SetTextColor( hdc, RGB(0, 0, 0) );
				dsTextLineOut( hdc , x-1 , y+11 , strBuff , len );
				SetTextColor( hdc, RGB(160, 220, 220) );
				dsTextLineOut( hdc , x , y+12 , strBuff , len );
			}

		}
	}
*/

	if ( VRKeyBuff[ 'A' ] ) {
		//바닥 아이템 이름: 글자는 모아서 한 번에 찍는다 (FPS 개선)
		BeginLabelTextBatch();
		for(cnt=0;cnt<DISP_ITEM_MAX;cnt++) {
			if ( scItems[cnt].Flag && scItems[cnt].ItemCode!=0 && lpSelItem!=&scItems[cnt] ) {

				if ( scItems[cnt].RendPoint.z>=32*fONE && scItems[cnt].RendPoint.z<64*12*fONE ) {

					DrawCharacterMessage( scItems[cnt].RendPoint.x, scItems[cnt].RendPoint.y,
						scItems[cnt].szName , 0 , 0, 0 , RGB(110, 110, 110)  );
				}
			}
		}
		EndLabelTextBatch();
	}

	if ( lpSelItem && !lpCharSelPlayer && !lpCharMsTrace ) {
		//아이템 이름 표시
		Disp_tx = MsSelPos.x;
		Disp_ty = MsSelPos.y;

		DrawCharacterMessage( Disp_tx, Disp_ty,
			lpSelItem->szName , 0 , 0, 0 , RGB(180, 180, 180)  );

	}


//	lpDDSBack->ReleaseDC( hdc );

////////////////////////////////////////////////////////////////////////////////


	int	SelFlag;

	ChatMsgSortCnt = 0;

	for( cnt=0;cnt<OTHER_PLAYER_MAX;cnt++) {
		if ( chrOtherPlayer[cnt].Flag && chrOtherPlayer[cnt].RendSucess && chrOtherPlayer[cnt].smCharInfo.szName[0] ) {
			SelFlag = 0;
			if ( chrOtherPlayer[cnt].RendPoint.z<12*64*fONE ) {
				if ( chrOtherPlayer[cnt].szChatMessage[0] ) {
					//채팅 창 검사
					if ( chrOtherPlayer[cnt].dwChatMessageTimer<dwPlayTime ) 
						chrOtherPlayer[cnt].szChatMessage[0] = 0;
					else {
						if ( chrOtherPlayer[cnt].smCharInfo.State ) {
							lpCharMsgSort[ChatMsgSortCnt++] = &chrOtherPlayer[cnt];
							SelFlag++;
						}
					}
				}

				if ( chrOtherPlayer[cnt].dwTradeMsgCode && !SelFlag ) {	//개인상점 오픈
					lpCharMsgSort[ChatMsgSortCnt++] = &chrOtherPlayer[cnt];
					SelFlag++;
				}
			}



			if ( !SelFlag ) {
				if ( lpCharMsTrace==&chrOtherPlayer[cnt] ||
					lpCharSelPlayer==&chrOtherPlayer[cnt] || 
					( chrOtherPlayer[cnt].smCharInfo.State==smCHAR_STATE_NPC && chrOtherPlayer[cnt].RendPoint.z<12*64*fONE ) || 
					( chrOtherPlayer[cnt].smCharInfo.Life[0]>0 && chrOtherPlayer[cnt].smCharInfo.State==smCHAR_STATE_ENEMY && chrOtherPlayer[cnt].smCharInfo.Brood==smCHAR_MONSTER_USER ) ||
					( chrOtherPlayer[cnt].smCharInfo.ClassClan && chrOtherPlayer[cnt].smCharInfo.ClassClan==lpCurPlayer->smCharInfo.ClassClan )  ||
					( HoMsgBoxMode && chrOtherPlayer[cnt].dwClanManageBit ) ||
					chrOtherPlayer[cnt].PartyFlag ) {

					lpCharMsgSort[ChatMsgSortCnt++] = &chrOtherPlayer[cnt];
				}
			}

		}
	}
	if ( lpCurPlayer->szChatMessage[0] ) {
		if ( lpCurPlayer->dwChatMessageTimer<dwPlayTime ) 
			lpCurPlayer->szChatMessage[0] = 0;
		else 
			lpCharMsgSort[ChatMsgSortCnt++] = lpCurPlayer ;
	}
	else {
		if ( lpCurPlayer->dwTradeMsgCode ) {
			lpCharMsgSort[ChatMsgSortCnt++] = lpCurPlayer ;
		}
	}

	//lpCharMsgSort[ChatMsgSortCnt++] = &chrOtherPlayer[cnt];

	//메세지 박스 소팅
	if ( ChatMsgSortCnt ) {
		for( cnt=0;cnt<ChatMsgSortCnt;cnt++) {
			for( cnt2=cnt+1;cnt2<ChatMsgSortCnt;cnt2++) {
				if ( lpCharMsgSort[cnt]->RendPoint.z<lpCharMsgSort[cnt2]->RendPoint.z ||
					lpCharMsgSort[cnt]==lpCharMsTrace || lpCharMsgSort[cnt]==lpCharSelPlayer ) {

					lpChar = lpCharMsgSort[cnt];
					lpCharMsgSort[cnt] = lpCharMsgSort[cnt2];
					lpCharMsgSort[cnt2] = lpChar;
				}
			}
		}
	}
/*

//x, y 찍을 위치 이 위치에서 좌우로 크기 정열해서 찍음..
// message: 일반이름 이나 채팅 메시지를 넣어주세요 (일반이름과 채팅 메시지는 다르게 알아서 찍힘) //NULL 일 경우 채팅 메시지 찍히지 않음.
// lineSize: 글이 길경우 가로 문자열 크기 제한..
// clanMark: clanMark 이미지 NULL 일경우는 찍히지 않음..
// clanName: 클랜 이름 NULL일 경우는 찍히지 않음..
// 포인터 값중 쓰지 않는 값은 NULL을 넣어 주세요..
DrawCharacterMessage(int x, int y, char *message, int lineSize, LPDIRECTDRAWSURFACE4 clanMark, char *clanName)
*/

	LPDIRECTDRAWSURFACE4 lpDDS_clanMark;
	char *szClanName;
	char *szBoxMsg;
	BOOL  selectedBox;


	y = 8+(ViewAnx>>6);

	//순차 정렬된 채팅 박스 그리기
	for( cnt=0;cnt<ChatMsgSortCnt;cnt++) {
/*
#ifdef _LANGUAGE_ENGLISH
		DrawChatMessage( lpCharMsgSort[cnt]->RendPoint.x, lpCharMsgSort[cnt]->RendRect2D.top,//+24, 
			lpCharMsgSort[cnt]->szChatMessage ,32);
#else
*/
		/*
		DrawChatMessage( lpCharMsgSort[cnt]->RendPoint.x, lpCharMsgSort[cnt]->RendRect2D.top,//+24, 
			lpCharMsgSort[cnt]->szChatMessage ,30);
		*/
		/*
			( chrOtherPlayer[cnt].PartyFlag || 
			( chrOtherPlayer[cnt].smCharInfo.State==smCHAR_STATE_ENEMY && chrOtherPlayer[cnt].smCharInfo.Brood==smCHAR_MONSTER_USER && chrOtherPlayer[cnt].smCharInfo.Next_Exp==lpCurPlayer->dwObjectSerial && chrOtherPlayer[cnt].smCharInfo.Life[0]>0 ) ) ) {

				dwColor = RGB(255, 180, 180);
			else
				dwColor = RGB(180, 180, 255);

			*/

		dwColor = RGB(255, 255, 200);

/*
		if ( lpCharMsgSort[cnt]==lpCharMsTrace ) {
			dwColor = RGB(255, 255, 255);
		}
		else if ( lpCharMsgSort[cnt]==lpCharSelPlayer ) {
			dwColor = RGB(255, 255, 255);
		}
*/
		DispBar = FALSE;
		DispMaster = FALSE;
		selectedBox = FALSE;

		if ( lpCharMsgSort[cnt]->PartyFlag ) {
			dwColor = RGB(220, 255, 160);
			DispBar = TRUE;
		}
		else if ( lpCharMsgSort[cnt]->smCharInfo.State==smCHAR_STATE_ENEMY ) {
			if (  lpCharMsgSort[cnt]->smCharInfo.Brood==smCHAR_MONSTER_USER ) {
				dwColor = RGB(220, 255, 160);

				if ( lpCharMsgSort[cnt]->smCharInfo.Next_Exp==lpCurPlayer->dwObjectSerial || lpCharMsgSort[cnt]->smCharInfo.ClassClan )
					DispBar = TRUE;
				else
					DispMaster = TRUE;
			}
			else
				dwColor = RGB(255, 210, 210);
		}
		else if ( lpCharMsgSort[cnt]->smCharInfo.State==smCHAR_STATE_NPC ) {
			dwColor = RGB(180, 180, 255);
		}

		if ( lpCharMsgSort[cnt]==lpCharMsTrace ) {
			if ( lpCharMsgSort[cnt]->smCharInfo.State==smCHAR_STATE_ENEMY && lpCharMsgSort[cnt]->smCharInfo.Brood!=smCHAR_MONSTER_USER )
				dwColor = RGB(255, 230, 200);
			else
				dwColor = RGB(255, 255, 255);

			selectedBox = TRUE;
		}
		else {
			if ( lpCharMsgSort[cnt]==lpCharSelPlayer && lpCharMsgSort[cnt]->szChatMessage[0] ) selectedBox = TRUE;
		}

/*
LPDIRECTDRAWSURFACE4 lpDDS_clanMark;
char *szClanName;
			else if ( lpChar->smCharInfo.ClassClan && lpChar->ClanInfoNum>=0 ) {
				//클랜 이름 표시 임시
				wsprintf( strBuff , "[%s]" , ClanInfo[lpChar->ClanInfoNum].ClanInfoHeader.ClanName );
*/
		lpDDS_clanMark = 0;
		szClanName = 0;
		DWORD	dwClanMgrBit;

		if ( lpCharMsgSort[cnt]->smCharInfo.ClassClan ) {
			if ( lpCharMsgSort[cnt]==lpCurPlayer  ) {
				lpDDS_clanMark =  cldata.hClanMark16;
				szClanName	   =  cldata.name;
			}
			else if ( lpCharMsgSort[cnt]->ClanInfoNum>=0 ) {
				lpDDS_clanMark = ClanInfo[lpCharMsgSort[cnt]->ClanInfoNum].hClanMark;
				szClanName = ClanInfo[lpCharMsgSort[cnt]->ClanInfoNum].ClanInfoHeader.ClanName;
			}
		}


		if ( ( ( lpCharMsgSort[cnt]->szChatMessage[0] || lpCharMsgSort[cnt]->dwTradeMsgCode ) && lpCharMsgSort[cnt]->smCharInfo.State && lpCharMsgSort[cnt]->RendPoint.z<12*64*fONE ) || 
			lpCharMsgSort[cnt]==lpCurPlayer ) {

			if ( szClanName && lpCharMsgSort[cnt]!=lpCharMsTrace && lpCharMsgSort[cnt]!=lpCharSelPlayer ) {
				if ( !lpCurPlayer->smCharInfo.ClassClan || lpCharMsgSort[cnt]->smCharInfo.ClassClan!=lpCurPlayer->smCharInfo.ClassClan )
					szClanName = 0;
			}

			if ( lpCharMsgSort[cnt]->szChatMessage[0] ) {
				DrawCharacterMessage( lpCharMsgSort[cnt]->RendPoint.x, lpCharMsgSort[cnt]->RendRect2D.top+y,
					lpCharMsgSort[cnt]->szChatMessage ,30 , lpDDS_clanMark, szClanName ,dwColor , selectedBox );
			}
			else {
				if ( lpCharMsgSort[cnt]->szTradeMessage[0] ) {
					DrawCharacterShopByeMessage( lpCharMsgSort[cnt]->RendPoint.x, lpCharMsgSort[cnt]->RendRect2D.top+y,
						lpCharMsgSort[cnt]->szTradeMessage ,30 , lpDDS_clanMark, dwColor , selectedBox );
				}
			}
		}
		else {
			if ( DispMaster && !lpCharMsgSort[cnt]->smCharInfo.ClassClan ) {

				wsprintf( strBuff , "(%s)" , lpCharMsgSort[cnt]->smCharInfo.szModelName2+1 );
				DrawTwoLineMessage( lpCharMsgSort[cnt]->RendPoint.x, lpCharMsgSort[cnt]->RendRect2D.top+y,
					lpCharMsgSort[cnt]->smCharInfo.szName , strBuff , dwColor , RGB(255, 255, 200) ,0 , selectedBox );

			}
			else {
				/*
				if ( lpDDS_clanMark && lpCharMsgSort[cnt]!=lpCharSelPlayer && lpCharMsgSort[cnt]!=lpCharMsTrace && !DispBar ) {
					szClanName = 0;
				}*/

				if ( HoMsgBoxMode )
					dwClanMgrBit = lpCharMsgSort[cnt]->dwClanManageBit;
				else
					dwClanMgrBit = 0;

				if ( szClanName ) {
					if ( smConfig.DebugMode && VRKeyBuff[VK_CONTROL] )
						wsprintf( strBuff , "%d/%d" , lpCharMsgSort[cnt]->smCharInfo.ClassClan , lpCharMsgSort[cnt]->ClanInfoNum );
					else
						wsprintf( strBuff , "[%s]" , szClanName );

					DrawTwoLineMessage( lpCharMsgSort[cnt]->RendPoint.x, lpCharMsgSort[cnt]->RendRect2D.top+y,
						strBuff , lpCharMsgSort[cnt]->smCharInfo.szName , RGB(150, 255, 200) ,dwColor ,lpDDS_clanMark , selectedBox , dwClanMgrBit );
				}
				else {
					szBoxMsg = lpCharMsgSort[cnt]->smCharInfo.szName;
					DrawCharacterMessage( lpCharMsgSort[cnt]->RendPoint.x, lpCharMsgSort[cnt]->RendRect2D.top+y,
						szBoxMsg ,30 , lpDDS_clanMark, szClanName ,dwColor , selectedBox , dwClanMgrBit );
				}
			}
		}

		if ( DispBar ) {
		    lpD3DDevice->EndScene();
			lpCharMsgSort[cnt]->DrawStateBar2( lpCharMsgSort[cnt]->RendPoint.x-30, lpCharMsgSort[cnt]->RendRect2D.top+y-14 );
		    lpD3DDevice->BeginScene();
		}


//#endif
	}

	//스킬 차징 게이지
	if ( lpCurPlayer->AttackSkil ) {
		switch( lpCurPlayer->AttackSkil&0xFF ) {
			case SKILL_PLAY_CHARGING_STRIKE:

				cnt = lpCurPlayer->frame-lpCurPlayer->MotionInfo->StartFrame*160;
				if ( lpCurPlayer->MotionInfo->EventFrame[0]<(DWORD)cnt && lpCurPlayer->MotionInfo->EventFrame[1]>(DWORD)cnt ) {

					cnt -= (int)lpCurPlayer->MotionInfo->EventFrame[0];
					cnt2 = (int)(lpCurPlayer->MotionInfo->EventFrame[1]-lpCurPlayer->MotionInfo->EventFrame[0]);

					lpD3DDevice->EndScene();
					lpCurPlayer->DrawChargingBar( lpCurPlayer->RendPoint.x-30,lpCurPlayer->RendRect2D.bottom, cnt ,cnt2 );
					lpD3DDevice->BeginScene();
				}
				break;
			case SKILL_PLAY_PHOENIX_SHOT:

				cnt = lpCurPlayer->frame-lpCurPlayer->MotionInfo->StartFrame*160;
				if ( lpCurPlayer->MotionInfo->EventFrame[0]<(DWORD)cnt && lpCurPlayer->MotionInfo->EventFrame[1]>(DWORD)cnt ) {

					cnt -= (int)lpCurPlayer->MotionInfo->EventFrame[0];
					cnt2 = (int)(lpCurPlayer->MotionInfo->EventFrame[1]-lpCurPlayer->MotionInfo->EventFrame[0]);

					lpD3DDevice->EndScene();
					lpCurPlayer->DrawChargingBar( lpCurPlayer->RendPoint.x-30,lpCurPlayer->RendRect2D.bottom, cnt ,cnt2 );
					lpD3DDevice->BeginScene();
				}
				break;
		}
	}

	if ( DispInterface ) {
		if ( DispEachMode ) {
			//DrawEachPlayer( 0.74f , 0.34f , 1 );					//확대 캐릭터 그리기
			DrawEachPlayer( 0.74f , 0.32f , 1 );					//확대 캐릭터 그리기
		}
		else {
			DrawEachPlayer( 0.92f , 0.1f , 0 );					//확대 캐릭터 그리기

			dsDrawOffset_X = WinSizeX-800;
			dsDrawOffset_Y = WinSizeY-600;
			dsDrawOffsetArray = dsARRAY_RIGHT|dsARRAY_BOTTOM;
			dsMenuCursorPos( &pCursorPos , 3 );				//메뉴커서 시물레이션
			//파티 메뉴 그리기
			DrawInterfaceParty();
			dsMenuCursorPos( &pCursorPos , 0 );				//메뉴커서 시물레이션 복구
			dsDrawOffsetArray = dsARRAY_TOP;
			dsDrawOffset_X = 0;
			dsDrawOffset_Y = 0;

			DrawInterfaceParty( pHoPartyMsgBox.x , pHoPartyMsgBox.y );		//파티권유 메뉴
		}
		//######################################################################################
		//작 성 자 : 오 영 석
		if( smConfig.DebugMode )
		{
			extern int Debug_TalkZoomMode;
			if( Debug_TalkZoomMode && lpCurPlayer->TalkPattern )
			{
				extern void DrawTalkZoom( smCHAR *lpChar, smPAT3D *lpPattern, float fx, float fy );
				DrawTalkZoom( lpCurPlayer, lpCurPlayer->TalkPattern, 0.2f, 0.32f );
			}
		}
		//######################################################################################

	}

//////////////////////////////
#ifdef	_NEW_PARTRADE
	if ( EachTradeButton && chrEachMaster ) {
		//파티 트레이드 신청 버트 클릭
		DisplayPartyTradeButton();
	}
#endif


	//Sod점수 표시 함수
	DisplaySodScore();

#ifdef _SINBARAM
	///////////////////////////////////////////////////////////////////////
	//텍스쳐 옵션을 기본값으로
	RestoreInterfaceTexture();

	dsDrawOffset_X = WinSizeX-800;
	dsDrawOffset_Y = WinSizeY-600;

	if ( DispInterface ) sinDraw();

	dsDrawOffset_X = 0;
	dsDrawOffset_Y = 0;
#endif

	if ( BellatraEffectInitFlag )		//벨라트라 이펙트
		DrawBellatraFontEffect();

	lpDDSBack->GetDC( &hdc );

	SelectObject( hdc , hFont );

//			RecvPopCount = 0;			//강제로 받기 시도 카운트
//		RecvPopErrorCount = 0;		//강제로 받기 시도 실패 카운트

#ifdef	_IME_ACTIVE

	////////////////////////// IME 표시 //////////////////////////
	char *ImeStr;
	//int Num,ImeY;
	int ImeY;
	//if ( hFocusWnd ) {
	if ( DisplayIME ) {

		SetBkColor( hdc , RGB( 255,255,200 ) );
		SetTextColor( hdc, RGB(0, 0, 0) );

		if (sinChatDisplayMode==1) 
			ImeY = 442;
		else 
			ImeY = 340;

		//ImeStr = IMETest.m_szCOMPOSTR.GetBuffer( IMETest.m_szCOMPOSTR.GetLength() );
		//if ( ImeStr[0] ) 
		//	dsTextLineOut( hdc , 8,ImeY-12, ImeStr , lstrlen( ImeStr ) );
		IMETest.GetImeInfo();
		ImeStr = IMETest.m_szTitle.GetBuffer( IMETest.m_szTitle.GetLength() );
		if ( ImeStr[0] )
			//dsTextLineOut( hdc , 100,ImeY-12, ImeStr , lstrlen( ImeStr ) );
			dsTextLineOut( hdc , 8,ImeY-12, ImeStr , lstrlen( ImeStr ) );

		//wsprintf( strBuff , "%s %s", 
		//	IMETest.m_szConvMode.GetBuffer( IMETest.m_szConvMode.GetLength() ) ,
		//	IMETest.m_szHalfMode.GetBuffer( IMETest.m_szHalfMode.GetLength() ) );

		//dsTextLineOut( hdc , 300,ImeY-12, strBuff , lstrlen( strBuff ) );

		SetBkColor( hdc , RGB( 255,255,255 ) );
		/*
		for( cnt=0;cnt<IMETest.nCandListCount;cnt++ ) {
	
			ImeStr = IMETest.m_szCandList[cnt].GetBuffer( IMETest.m_szCandList[cnt].GetLength() );

			if ( cnt>=9 ) Num=0;
			else Num = cnt+1;

			wsprintf( strBuff , " %d: %s ", Num , ImeStr );

			dsTextLineOut( hdc , 344,ImeY+cnt*12, strBuff , lstrlen( strBuff ) );
		}
		*/
	}

#endif

    SetBkMode( hdc, TRANSPARENT );

#ifdef _WINMODE_DEBUG

	int	rcv1,rcv2,snd1,snd2,arcv1,brcv1,arcv2,brcv2;
	int LineY;

	if (DisplayDebug) {
		//wsprintf( strBuff , "%d  %d  x=%d z=%d y=%d  VRAM=%d Rcv1=%d Rcv2=%d", DispRender , DispPolyCnt , lpCurPlayer->pX>>(FLOATNS) , lpCurPlayer->pZ>>(FLOATNS) , lpCurPlayer->pY>>(FLOATNS), VramTotal , Debug_RecvCount1 , Debug_RecvCount2  );

		rcv1=0;
		rcv2=0;
		snd1=0;
		snd2=0;
		arcv1=0;
		brcv1=0;
		arcv2=0;
		brcv2=0;

		if ( smWsockServer ) {
			rcv1= smWsockServer->RecvPacketCount;
			snd1= smWsockServer->SendPacketCount;
			arcv1 = smWsockServer->RecvPopCount;
			brcv1 = smWsockServer->RecvPopErrorCount;
		}
		else { rcv1=0;snd1=0; }

		if ( smWsockUserServer ) {
			rcv2= smWsockUserServer->RecvPacketCount;
			snd2= smWsockUserServer->SendPacketCount;
			arcv2 = smWsockUserServer->RecvPopCount;
			brcv2 = smWsockUserServer->RecvPopErrorCount;
		}
		else { rcv2=0;snd2=0; }

/*
		wsprintf( strBuff , "%d(%d) %d x=%d z=%d y=%d VRAM=%d R1=%d(%d) R2=%d(%d) S1=%d S2=%d (%d/%d) (%d/%d) Recon( %d %d )", DispRender ,DispMainLoop , DispPolyCnt , lpCurPlayer->pX>>(FLOATNS) , lpCurPlayer->pZ>>(FLOATNS) , lpCurPlayer->pY>>(FLOATNS), (VramTotal/(1024*1024)) , 
			rcv1,Debug_RecvCount1,rcv2,Debug_RecvCount2,snd1,snd2 , brcv1,arcv1, brcv2,arcv2 , ReconnServer ,ReconnDataServer );
*/
		wsprintf( strBuff , "%d(%d) %d x=%d z=%d y=%d VRAM=%d R1=%d(%d) R2=%d(%d) S1=%d S2=%d (%d/%d) (%d/%d) RcvTurb( %d )", DispRender ,DispMainLoop , DispPolyCnt , lpCurPlayer->pX>>(FLOATNS) , lpCurPlayer->pZ>>(FLOATNS) , lpCurPlayer->pY>>(FLOATNS), (VramTotal/(1024*1024)) , 
			rcv1,Debug_RecvCount1,rcv2,Debug_RecvCount2,snd1,snd2 , brcv1,arcv1, brcv2,arcv2 , smTransTurbRcvMode );	//ReconnServer ,ReconnDataServer );

		SetTextColor( hdc, RGB(0, 0, 0) );
		dsTextLineOut( hdc , 11,11, strBuff , lstrlen( strBuff ) );
		SetTextColor( hdc, RGB(255, 255, 255) );
		dsTextLineOut( hdc , 10,10, strBuff , lstrlen( strBuff ) );

		wsprintf( strBuff , "Stage : %s , %s", szGameStageName[0] ,szGameStageName[1] );
		SetTextColor( hdc, RGB(0, 0, 0) );
		dsTextLineOut( hdc , 11,31, strBuff , lstrlen( strBuff ) );
		SetTextColor( hdc, RGB(255, 255, 255) );
		dsTextLineOut( hdc , 10,30, strBuff , lstrlen( strBuff ) );

		wsprintf( strBuff , "World Time (%d:%d)", dwGameHour ,dwGameMin );
		SetTextColor( hdc, RGB(0, 0, 0) );
		dsTextLineOut( hdc , (WinSizeX>>1)+1,31, strBuff , lstrlen( strBuff ) );
		SetTextColor( hdc, RGB(255, 255, 255) );
		dsTextLineOut( hdc , (WinSizeX>>1),30, strBuff , lstrlen( strBuff ) );


		if ( lpCurPlayer->PatLoading==FALSE ) {
			if ( AdminCharMode ) 
				wsprintf( strBuff , "%s" , lpCurPlayer->smCharInfo.szName );
			else
				wsprintf( strBuff , "%s" , lpCurPlayer->lpDinaPattern->szPatName );

			SetTextColor( hdc, RGB(0, 0, 0) );
			dsTextLineOut( hdc , (WinSizeX>>1)+1+120 , 31 , strBuff , lstrlen( strBuff ) );
			SetTextColor( hdc, RGB(255, 255, 255) );
			dsTextLineOut( hdc , (WinSizeX>>1)+120 , 30 , strBuff , lstrlen( strBuff ) );

			wsprintf( strBuff , "Damage : %d" , LastAttackDamage );
			SetTextColor( hdc, RGB(0, 0, 0) );
			dsTextLineOut( hdc , 9 , 48 , strBuff , lstrlen( strBuff ) );
			SetTextColor( hdc, RGB(255, 255, 255) );
			dsTextLineOut( hdc , 8 , 47 , strBuff , lstrlen( strBuff ) );

			wsprintf( strBuff , "RcvDamage:(%d) [%d] %d %d", Record_TotalRecvDamageCount , Record_RecvDamageCount,Record_DefenceCount,Record_BlockCount );
			SetTextColor( hdc, RGB(0, 0, 0) );
			dsTextLineOut( hdc , 9 , 62 , strBuff , lstrlen( strBuff ) );
			SetTextColor( hdc, RGB(255, 255, 255) );
			dsTextLineOut( hdc , 8 , 61 , strBuff , lstrlen( strBuff ) );

			if ( AreaServerMode ) {
				if ( lpWSockServer_Area[0] ) {
					wsprintf( strBuff , "Area Server[0]: (%s)",lpWSockServer_Area[0]->szIPAddr );
					SetTextColor( hdc, RGB(0, 0, 0) );
					dsTextLineOut( hdc , 9 , 82 , strBuff , lstrlen( strBuff ) );
					SetTextColor( hdc, RGB(255, 255, 255) );
					dsTextLineOut( hdc , 8 , 81 , strBuff , lstrlen( strBuff ) );

				}
				if ( lpWSockServer_Area[1] ) {
					wsprintf( strBuff , "Area Server[1]: (%s)",lpWSockServer_Area[1]->szIPAddr );
					SetTextColor( hdc, RGB(0, 0, 0) );
					dsTextLineOut( hdc , 9 , 96 , strBuff , lstrlen( strBuff ) );
					SetTextColor( hdc, RGB(255, 255, 255) );
					dsTextLineOut( hdc , 8 , 95 , strBuff , lstrlen( strBuff ) );

				}

				if ( lpWSockServer_DispArea[0] ) {
					wsprintf( strBuff , "Area Server[0]: (%s)",lpWSockServer_DispArea[0]->szIPAddr );
					SetTextColor( hdc, RGB(0, 0, 0) );
					dsTextLineOut( hdc , 209 , 82 , strBuff , lstrlen( strBuff ) );
					SetTextColor( hdc, RGB(255, 255, 255) );
					dsTextLineOut( hdc , 208 , 81 , strBuff , lstrlen( strBuff ) );

				}
				if ( lpWSockServer_DispArea[1] ) {
					wsprintf( strBuff , "Area Server[1]: (%s)",lpWSockServer_DispArea[1]->szIPAddr );
					SetTextColor( hdc, RGB(0, 0, 0) );
					dsTextLineOut( hdc , 209 , 96 , strBuff , lstrlen( strBuff ) );
					SetTextColor( hdc, RGB(255, 255, 255) );
					dsTextLineOut( hdc , 208 , 95 , strBuff , lstrlen( strBuff ) );

				}


				wsprintf( strBuff , "AreaCount(%d) AreaConn(%d) AreaIP( [%d] %d %d ) Step(%d)" , 
					dwDebugAreaCount , dwDebugAreaConnCount ,
					dwDebugAreaIP[0],dwDebugAreaIP[1],dwDebugAreaIP[2] , dwDebugAreaStep );

				SetTextColor( hdc, RGB(0, 0, 0) );
				dsTextLineOut( hdc , 9 , 108 , strBuff , lstrlen( strBuff ) );
				SetTextColor( hdc, RGB(255, 255, 255) );
				dsTextLineOut( hdc , 8 , 108 , strBuff , lstrlen( strBuff ) );

			}

		}


		SetTextColor( hdc, RGB(255, 255, 255) );
		//DisplayError( hdc , 0, 48 );

/*
	wsprintf( strBuff , "ReChecked=%d" ,DEBUG_ReCheckedCnt ) ;
	SetTextColor( hdc, RGB(0, 0, 0) );
	dsTextLineOut( hdc , 11,31, strBuff , lstrlen( strBuff ) );
	SetTextColor( hdc, RGB(255, 255, 255) );
	dsTextLineOut( hdc , 10,30, strBuff , lstrlen( strBuff ) );
	DEBUG_ReCheckedCnt=0;
*/	
		LineY=48;
		for( i=0;i<OTHER_PLAYER_MAX;i++) {
			if ( chrOtherPlayer[i].Flag && chrOtherPlayer[i].Pattern && chrOtherPlayer[i].smCharInfo.State==smCHAR_STATE_USER ) {
				wsprintf( strBuff , "%s", chrOtherPlayer[i].smCharInfo.szName );//, chrOtherPlayer[i].TransSock->szIPAddr , chrOtherPlayer[i].TransLastSendTime );
				SetTextColor( hdc, RGB(0, 0, 0) );
				dsTextLineOut( hdc , WinSizeX-220,LineY+1, strBuff , lstrlen( strBuff ) );
				SetTextColor( hdc, RGB(255, 255, 255) );
				dsTextLineOut( hdc , WinSizeX-221,LineY, strBuff , lstrlen( strBuff ) );
				LineY += 16;
				if ( LineY>460 ) break;
			}
		}

		int	MonCharBuff[OTHER_PLAYER_MAX];
		int MonCharCnt = 0;
		int	MonCnt;

		for( i=0;i<OTHER_PLAYER_MAX;i++) {
			if ( chrOtherPlayer[i].Flag && chrOtherPlayer[i].Pattern && chrOtherPlayer[i].smCharInfo.State!=smCHAR_STATE_USER ) {
				MonCharBuff[MonCharCnt++] = i;
			}
		}

		LineY=48;
		for( i=0;i<MonCharCnt;i++) {
			if ( MonCharBuff[i]>=0 ) {
				MonCnt = 1;
				for( cnt=i+1;cnt<MonCharCnt;cnt++ ) {
					if ( MonCharBuff[cnt]>=0 && 
						lstrcmp( chrOtherPlayer[MonCharBuff[cnt]].smCharInfo.szName , 
						chrOtherPlayer[MonCharBuff[i]].smCharInfo.szName )==0 ) {
						MonCharBuff[cnt] = -1;
						MonCnt++;
					}
				}

				wsprintf( strBuff , "%s x %d", chrOtherPlayer[MonCharBuff[i]].smCharInfo.szName , MonCnt );//, chrOtherPlayer[i].TransSock->szIPAddr , chrOtherPlayer[i].TransLastSendTime );
				SetTextColor( hdc, RGB(0, 0, 0) );
				dsTextLineOut( hdc , WinSizeX-380,LineY+1, strBuff , lstrlen( strBuff ) );
				if ( chrOtherPlayer[MonCharBuff[i]].smCharInfo.State==smCHAR_STATE_NPC )
					SetTextColor( hdc, RGB(192, 192, 255) );
				else
					SetTextColor( hdc, RGB(255, 192, 192) );
				dsTextLineOut( hdc , WinSizeX-381,LineY, strBuff , lstrlen( strBuff ) );
				LineY += 16;
				if ( LineY>460 ) break;

			}
		}

	}
#endif

//	smCHAR	*lpChar;


//extern int sinChatDisplayMode; //{ 0- 아무것도 안그리기  1-보통  2-상점 }
//extern int sinChatInputMode;   //{ 0- 아무것도 안그리기  1-보통  2-상점 }
/*

	char *lpInput;

	if ( hFocusWnd && sinChatInputMode ) {
		GetWindowText( hTextWnd , strBuff2 , 240 );
		strBuff2[242]=0x0D;
		len = lstrlen( strBuff2 );

		SetBkMode( hdc, TRANSPARENT );

		wsprintf( strBuff, "▶ %s" , strBuff2 );

		len=lstrlen( strBuff );
		if ( len>=52 ) {
			lpInput = &strBuff[len-52];
			len=52;
		}
		else 
			lpInput = strBuff;

		if ( sinChatInputMode==1 ) {
			x = 16 ; y = 582;
		}
		else {
			x = 16 ; y = 432;
		}

		SetTextColor( hdc, RGB(0, 0, 0) );
		dsTextLineOut( hdc , x+1, y+1 , lpInput , len );
		SetTextColor( hdc, RGB(255, 255, 255) );
		dsTextLineOut( hdc , x , y , lpInput , len );
	}

	if ( sinChatDisplayMode ) {
		if ( sinChatDisplayMode==1 ) {
			x = 16 ; y = 562; i = 10;
		}
		else {
			x = 16 ; y = 412; i = 6;
		}
		//채팅 정보 화면에 표시
		DisplayChatMessage2( hdc , x , y , 52 , i );
	}
*/
	lpDDSBack->ReleaseDC( hdc );

/*
char *mgDiconnect = "서버와의 연결이 끊어 졌습니다";
char *mgDiconnect1 = "서버와의 연결이 끊어 졌습니다 (1)";
char *mgDiconnect2 = "서버와의 연결이 끊어 졌습니다 (2)";
char *mgDiconnect3 = "서버와의 연결이 끊어 졌습니다 (3)";
char *mgDiconnect4 = "서버와의 연결이 끊어 졌습니다 (4)";

char *mgCloseGame = "게임을 종료 합니다";
char *mgCloseWindow = "사용중인 창을 닫고 종료 하십시오";
char *mgCloseBattle = "전투중에는 종료할수 없습니다";
*/

	if ( DisconnectFlag ) {
		//서버와의 연결 끊어짐
		if ( DisconnectServerCode==0 ) {
			DrawMessage( MidX-64 ,  MidY, mgDiconnect ,36, BOX_ONE );
		}
		if ( DisconnectServerCode==1 ) {
			DrawMessage( MidX-64 ,  MidY, mgDiconnect1 ,36, BOX_ONE );
		}
		if ( DisconnectServerCode==2 ) {
			DrawMessage( MidX-64 ,  MidY, mgDiconnect2 ,36, BOX_ONE );
		}
		if ( DisconnectServerCode==3 ) {
			DrawMessage( MidX-64 ,  MidY, mgDiconnect3 ,36, BOX_ONE );
		}
		if ( DisconnectServerCode==4 ) {
			DrawMessage( MidX-64 ,  MidY, mgDiconnect4 ,36, BOX_ONE );
		}

#ifdef	_WINMODE_DEBUG
		if ( !smConfig.DebugMode && !quit && ((DWORD)DisconnectFlag+5000)<dwPlayTime ) quit=TRUE;
#else
		if ( !quit && ((DWORD)DisconnectFlag+5000)<dwPlayTime ) quit=TRUE;
#endif

	}
	else {
		if ( quit )
			DrawMessage( MidX-40 ,  MidY, mgCloseGame ,36, BOX_ONE );
		else {
			if ( dwCloseBoxTime && dwCloseBoxTime>dwPlayTime ) {
				DrawMessage( MidX-100 ,  MidY, mgCloseWindow ,36, BOX_ONE );
			}
			else {
				if ( dwBattleQuitTime ) {
					if ( (dwBattleQuitTime+5000)>dwPlayTime )
						DrawMessage( MidX-40 ,  MidY, mgCloseBattle ,36, BOX_ONE );
					else
						dwBattleQuitTime = 0;
				}
			}
		}
	}


//int CloseEachPlayer();					//확대 캐릭터 닫기
/*
    lpD3DDevice->BeginScene();
	smRender.ClearLight();
	smRender.Color_A = 0;
	smRender.Color_R = 0;
	smRender.Color_G = 0;
	smRender.Color_B = 0;
	DrawEffect(0,0,0, 0,0,0 );					//호이펙트 그리기
    lpD3DDevice->EndScene();
*/
	DarkLevel = BackDarkLevel;

	if ( Flip()==FALSE ) {
		DisconnectFlag = dwPlayTime;
		quit = 1;
	}

	return TRUE;
}




void _stdcall smPlayD3D( int x, int y, int z, int ax, int ay, int az )
{

	eCAMERA_TRACE	eTrace;
	int	ap;

//	xr+=.001;


	DispPolyCnt = 0;

	if ( lpDDSBack->IsLost()==DDERR_SURFACELOST || lpDDSPrimary->IsLost()==DDERR_SURFACELOST ) {
		//서페이스 로스트 연결 종료
		DisconnectFlag = dwPlayTime;
		quit = TRUE;

		lpDDSPrimary->Restore();
		lpDDSBack->Restore();
		RestoreFlag = TRUE;
		return;
	}
	else {
		if ( RestoreFlag ) {
			RestoreAll();
			RestoreFlag = FALSE;
		}
	}

	//////////////////////////////////////////////////////////////////////
	//메모리 첵크
	//CheckKeepMem( 70 );
	//////////////////////////////////////////////////////////////////////

	SetRendSight();		//렌더링 시야 설정

	//######################################################################################
	//작 성 자 : 오 영 석
	if( g_IsDxProjectZoomIn <= 0 )
        DrawGameState();
	else
		VirtualDrawGameState();
	//######################################################################################

	if ( (dwDebugBack^dwDebugXor)==0 ) {
		if ( smConfig.DebugMode ) {
			//해킹 시도한 유저 자동 신고
			SendSetHackUser( TRUE );
			smConfig.DebugMode = 0;
		}
	}


	int Mapfl;
	int cy;
	int mapY;

	Mapfl = 0;
	cy = y+16*fONE;

	if ( !DebugPlayer ) {
		if ( smGameStage[0] ) {
			mapY = (smGameStage[0]->GetHeight( x , z ));
			if ( y<mapY ) {
				y = mapY;
				y +=8<<FLOATNS;
			}
			if ( mapY>CLIP_OUT ) Mapfl++;
		}
		if ( smGameStage[1] ) {
			mapY = (smGameStage[1]->GetHeight( x , z ));
			if ( y<mapY ) {
				y = mapY;
				y +=8<<FLOATNS;
			}
			if ( mapY>CLIP_OUT ) Mapfl++;
		}
	}

	Mix_CodeVram();		//코드 버퍼 믹서

	smRender.Color_R = BrCtrl;
	smRender.Color_G = BrCtrl;
	smRender.Color_B = BrCtrl;
	smRender.Color_A = BrCtrl;


	//SetDynLight( lpCurPlayer->pX,lpCurPlayer->pY+32*fONE,lpCurPlayer->pZ , 100, 100,100,0, 200 );
	smRender.Color_R = -DarkLevel+BackColor_R;
	smRender.Color_G = -DarkLevel+BackColor_G;
	smRender.Color_B = -DarkLevel+BackColor_B;


	if( dwM_BlurTime && IsCreateNewRenderTarget() ) {
		if ( dwM_BlurTime<dwPlayTime ) {
			dwM_BlurTime = 0;
			SetFilterEffect( FILTER_EFFECT_SET_BRIGHT_CONTRAST, 160 );
			SetFilterEffect( FILTER_EFFECT_DEL_MOTION_BLUR, 0 );
		}
	}



	//######################################################################################
	//작 성 자 : 오 영 석
	if( GetFilterEffect() )
	{
		BeginRender();
        ChangeRenderTarget( NEW_TARGET_BACK );
	}
	//######################################################################################


	BeginRender();
/*
	//######################################################################################
	//작 성 자 : 오 영 석
	if( g_IsDxProjectZoomIn )
	{
		DrawFullZoomMap();
		lpD3DViewport->Clear2( 1UL, &D3DRect, D3DCLEAR_ZBUFFER, smBackColor, 1, 0L );
	}
	//######################################################################################
*/
	//######################################################################################
	//작 성 자 : 오 영 석
	y += (32<<FLOATNS);

	if( anx <= 40 && dist <= 100 )
		y -= ((110-dist) << FLOATNS);
	//######################################################################################

	TraceTargetPosi.x = x;
	TraceTargetPosi.y = y;
	TraceTargetPosi.z = z;

	if ( AutoCameraFlag ) {

		TraceCameraMain();

		x = TraceCameraPosi.x;
		y = TraceCameraPosi.y;
		z = TraceCameraPosi.z;
	}

	//######################################################################################
	//작 성 자 : 오 영 석
	int ey = lpCurPlayer->pY + (32*fONE);
	if( anx <= 40 && dist <= 100 ) 
		ey += ((100-dist) * fONE);


	ActionGameMode = FALSE;
/*
		x = lpCurPlayer->pX;
		y = StageField[ lpCurPlayer->OnStageField ]->ActionCamera.FixPos.y*fONE;
		z = StageField[ lpCurPlayer->OnStageField ]->ActionCamera.FixPos.z*fONE;

		if ( x<StageField[ lpCurPlayer->OnStageField ]->ActionCamera.LeftX*fONE ) x= StageField[ lpCurPlayer->OnStageField ]->ActionCamera.LeftX*fONE;
		else if ( x>StageField[ lpCurPlayer->OnStageField ]->ActionCamera.RightX*fONE ) x=StageField[ lpCurPlayer->OnStageField ]->ActionCamera.RightX*fONE;

		MakeTraceMatrix( &eTrace, x,y,z, lpCurPlayer->pX, lpCurPlayer->pY , (StageField[ lpCurPlayer->OnStageField ]->ActionCamera.FixPos.z+290)*fONE );
*/


	if ( lpCurPlayer && lpCurPlayer->OnStageField>=0 && StageField[ lpCurPlayer->OnStageField ]->State==FIELD_STATE_ACTION ) {
		//횡 스크롤 액션 모드
		x = lpCurPlayer->pX;
		y = StageField[ lpCurPlayer->OnStageField ]->ActionCamera.FixPos.y+80*fONE;
		z = StageField[ lpCurPlayer->OnStageField ]->ActionCamera.FixPos.z*fONE;

		if ( x<StageField[ lpCurPlayer->OnStageField ]->ActionCamera.LeftX*fONE ) x= StageField[ lpCurPlayer->OnStageField ]->ActionCamera.LeftX*fONE;
		else if ( x>StageField[ lpCurPlayer->OnStageField ]->ActionCamera.RightX*fONE ) x=StageField[ lpCurPlayer->OnStageField ]->ActionCamera.RightX*fONE;

		MakeTraceMatrix( &eTrace, x,y,z, x, lpCurPlayer->pY , 326711*fONE );

/*
		x = lpCurPlayer->pX;
		y = 213140+80*fONE;
		z = (36711-290)*fONE;

		MakeTraceMatrix( &eTrace, x,y,z, lpCurPlayer->pX, lpCurPlayer->pY , 326711*fONE );
*/
		ax = eTrace.AngX;
		ay = eTrace.AngY;
		smRender.OpenCameraPosi( x,y,z, &eTrace.eRotMatrix );

		ActionGameMode = TRUE;
	}
	else if ( !DebugPlayer )
	{
		MakeTraceMatrix( &eTrace, x,y,z, lpCurPlayer->pX, ey, lpCurPlayer->pZ );

		ax = eTrace.AngX;
		ay = eTrace.AngY;
		smRender.OpenCameraPosi( x,y,z, &eTrace.eRotMatrix );
	}
	//######################################################################################




	smRender.ClearLight();

	if ( DarkLevel>0 ) {

		ap = DarkLevel+(DarkLevel>>2);

		if ( StageField[lpCurPlayer->OnStageField]->State==FIELD_STATE_DUNGEON ) {
			DarkLightRange = 400*fONE;		//320
			ap = DarkLevel;
		}
		else
			DarkLightRange = 260*fONE;		//220
			//DarkLightRange = 180*fONE;


		if ( lpCurPlayer->OnStageField>=0 && StageField[lpCurPlayer->OnStageField]->State==FIELD_STATE_VILLAGE )  {
			//ap = (ap*190)>>8;
			//smRender.AddDynamicLight( lpCurPlayer->pX,lpCurPlayer->pY+32*fONE,lpCurPlayer->pZ, ap,ap,ap,0, DarkLightRange );
		}
		else
			smRender.AddDynamicLight( lpCurPlayer->pX,lpCurPlayer->pY+32*fONE,lpCurPlayer->pZ, ap,ap,ap,0, DarkLightRange );
	}

	DynLightApply();	//동적 광원 렌더러에 적용


#ifdef PT_GFX_GL
	// weather (weather.cpp): the sky, the map and the characters get its look
	PtWeather_Frame( FALSE );
	PtGfx_WeatherWorld( TRUE );
#endif
	//DrawBG( x,  y,  z,  ax, ay, az );
	DrawSky( x,  y,  z,  ax, ay, az );

	smRender.DeviceRendMode = FALSE;
	lpD3DDevice->BeginScene();

	//######################################################################################
	//작 성 자 : 오 영 석
	if( smRender.m_FogIsRend && smRender.m_FogMode )
		lpD3DDevice->SetRenderState( D3DRENDERSTATE_FOGENABLE, TRUE );
	//######################################################################################

	//######################################################################################
	//작 성 자 : 오 영 석
#ifdef PT_GFX_GL
	// Real-time shadows (ptgfx_shadow.cpp): the characters drawn by DrawPat3D, the
	// loaded maps' meshes and the map's objects (a frame late) cast, the map
	// receives. Not in dungeons (no sun there). The shadowed square is centred on
	// the player.
	{
		extern smSTAGE3D *smGameStage[2];
		smStage_ShadowMesh( smGameStage[0], 0 );
		smStage_ShadowMesh( smGameStage[1], 1 );
	}
	PtGfx_ShadowFrame( lpCurPlayer && lpCurPlayer->OnStageField>=0 && StageField[lpCurPlayer->OnStageField] &&
		StageField[lpCurPlayer->OnStageField]->State!=FIELD_STATE_DUNGEON,
		(double)x/fONE, (double)y/fONE, (double)z/fONE,
		(double)lpCurPlayer->pX/fONE, (double)lpCurPlayer->pY/fONE, (double)lpCurPlayer->pZ/fONE );
#endif
	smRender.bApplyRendObjLight = TRUE;
	DrawPat3D( x, y, z, ax, ay, az );

#ifdef PT_GFX_GL
	PtGfx_ShadowRole( PTSHADOW_RECEIVER );
#endif
	smRender.bApplyRendObjLight = FALSE;
	NumPoly = DisplayStage( x ,  y,  z, ax, ay, az );
#ifdef PT_GFX_GL
	PtGfx_ShadowRole( PTSHADOW_OFF );
#endif

	smRender.bApplyRendObjLight = TRUE;
	DrawPat3D_Alpha();
#ifdef PT_GFX_GL
	PtGfx_WeatherWorld( FALSE );
#endif

	smRender.ClearObjLight();
	//######################################################################################

	//######################################################################################
	//작 성 자 : 오 영 석
	if( smRender.m_FogIsRend && smRender.m_FogMode )
		lpD3DDevice->SetRenderState( D3DRENDERSTATE_FOGENABLE, FALSE );
	//######################################################################################

	smRender.ClearLight();
	smRender.Color_A = 0;
	smRender.Color_R = 0;
	smRender.Color_G = 0;
	smRender.Color_B = 0;

	//그림자 그리기
	DrawPatShadow( x, y, z, ax, ay, az );

	lpD3DDevice->EndScene();
	smRender.DeviceRendMode = TRUE;

	//텍스쳐 옵션을 기본값으로
	RestoreInterfaceTexture();

	DrawEffect(x, y, z, ax,ay,az);					//호이펙트 그리기
	cSin3D.Draw( x,y,z,ax,ay,az );
	DrawPat2D( x, y, z, ax, ay, az );


	//######################################################################################
	//작 성 자 : 오 영 석
	if( GetFilterEffect() && GetRenderTarget() == NEW_TARGET_BACK )
	{
		SetNewTargetTextureState();
		lpD3DDevice->BeginScene();
		RenderFilterEffect();
		lpD3DDevice->EndScene();
		DefaultsNewTargetTextureState();
	}
	//######################################################################################

	//######################################################################################
	//작 성 자 : 오 영 석
	if( g_IsDxProjectZoomIn )
	{
		DrawFullZoomMap();
	}
	else
	{
		if ( cInterFace.sInterFlags.MapOnFlag )
		{
			//필드 맵 그리기
			if ( DispInterface ) DrawFieldMap();
		}
	}
	//######################################################################################

	if ( (!MsTraceMode && MouseButton[0]==0) || 
		lpCurPlayer->smCharInfo.Stamina[0]>(lpCurPlayer->smCharInfo.Stamina[1]>>2) ) {
		//걷기 뛰기 모드
		lpCurPlayer->MoveMode = cInterFace.sInterFlags.RunFlag;

		if ( ActionGameMode ) {
			lpCurPlayer->MoveMode = ActionDashMode;
		}

	}
	else {
		if ( lpCurPlayer->smCharInfo.Stamina[0]==0 )
			lpCurPlayer->MoveMode = FALSE;
	}

/*
	if ( lpCurPlayer->smCharInfo.Stamina[0] ) {
		//걷기 뛰기 모드
		lpCurPlayer->MoveMode = cInterFace.sInterFlags.RunFlag;
	}
	else {
		lpCurPlayer->MoveMode = FALSE;
	}
*/
	smRender.CloseCameraPosi();

/*
	if ( GoText ) {
		TestSetNewText( strBuff2 ) ;
		GoText =0;
	}
*/

//	smRender.RenderD3D();

	Disp_tx = MsSelPos.x-32;
	Disp_ty = MsSelPos.y;


	EndRender();
	//텍스쳐 프레임 증가
	IncTextureFrame();


}


char *CompCmdStr( char *strCmdLine , char *strword )
{
	int len,len2;
	int cnt,cnt2;

	len = lstrlen( strCmdLine );
	len2 = lstrlen( strword );

	for(cnt=0;cnt<len-len2;cnt++) {
		for(cnt2=0;cnt2<len2;cnt2++) {
			if ( strword[cnt2]!=strCmdLine[cnt+cnt2] ) break;
		}
		if ( cnt2==len2 ) 
			return &strCmdLine[cnt+cnt2];
	}

	return NULL;
};

//코멘드 라인 분석 설정
int DecodeCmdLine( char *lpCmdLine )
{
	char *lpChar;
	int	cnt;
//	char lpBuff[32];

	lpChar = CompCmdStr( lpCmdLine , "/reload=" );
	if ( lpChar ) {
		for(cnt=0;cnt<16;cnt++) {
			if ( lpChar[cnt]==' ' || lpChar[cnt]=='&' || lpChar[cnt]==0 ) break;
		}
		if ( atoi( lpChar )==0 ) {
			smSetMeshReload( 0 , 1 );
		}
	}

	return TRUE;
}



//szAppName
//sCraftItem_Info



char *RegPath_3DMax = "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\3D Studio MAX R3.1L";
char *RegPath_3DMax2 = "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\3D Studio MAX R3.1";
char *RegPath_Photoshop = "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Adobe Photoshop 6.0";
char *RegPath_ACDSee = "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\ACDSee";

//불법 소프트 웨어 단속에 대비한 레지스트리 조작
int HaejukReg()
{
/*
	char *RegStr;
	RegStr = GetRegString( HKEY_LOCAL_MACHINE , RegPath_3DMax , "DisplayName" );
	if ( RegStr ) {
		SetRegString( HKEY_LOCAL_MACHINE , RegPath_3DMax , "DisplayName" , "DeluxPaint 2000" );
		
	}

	RegStr = GetRegString( HKEY_LOCAL_MACHINE , RegPath_3DMax2 , "DisplayName" );
	if ( RegStr ) {
		SetRegString( HKEY_LOCAL_MACHINE , RegPath_3DMax2 , "DisplayName" , "DeluxPaint 2000" );		
	}

	RegStr = GetRegString( HKEY_LOCAL_MACHINE , RegPath_Photoshop , "DisplayName" );
	if ( RegStr ) {
		SetRegString( HKEY_LOCAL_MACHINE , RegPath_Photoshop , "DisplayName" , "Paint School 1.0" );		
	}

	RegStr = GetRegString( HKEY_LOCAL_MACHINE , RegPath_ACDSee , "DisplayName" );
	if ( RegStr ) {
		SetRegString( HKEY_LOCAL_MACHINE , RegPath_ACDSee , "DisplayName" , "ABC 메신저" );		
	}
	
*/	


	return TRUE;
}


//폰트 생성
int SetCreateFont()
{

#ifdef _LANGUAGE_ENGLISH
#ifdef	_CUSTOM_FONT
	hFont = PT_CreateFont( 16,
                        0,
                        0,
                        0,
						FW_NORMAL,
                        FALSE,
                        FALSE,
                        FALSE,
                        DEFAULT_CHARSET,
                        OUT_DEFAULT_PRECIS,
                        CLIP_DEFAULT_PRECIS,
                        DEFAULT_QUALITY,
                        VARIABLE_PITCH,
                        "Comic Sans MS" );
#else
	hFont = PT_CreateFont( 16,
                        0,
                        0,
                        0,
						FW_NORMAL,
                        FALSE,
                        FALSE,
                        FALSE,
						DEFAULT_CHARSET,
                        OUT_DEFAULT_PRECIS,
                        CLIP_DEFAULT_PRECIS,
                        DEFAULT_QUALITY,
                        VARIABLE_PITCH,
						"Comic Sans MS" );

#endif
#endif

#ifdef _LANGUAGE_TAIWAN
	hFont = PT_CreateFont( 12,
                        0,
                        0,
                        0,
						FW_NORMAL,
                        FALSE,
                        FALSE,
                        FALSE,
						CHINESEBIG5_CHARSET,
                        OUT_DEFAULT_PRECIS,
                        CLIP_DEFAULT_PRECIS,
                        DEFAULT_QUALITY,
                        VARIABLE_PITCH,
						"MingLiu" );
#endif


#ifdef _LANGUAGE_JAPANESE

#include "Japanese\\j_font.h"

	hFont = PT_CreateFont( 12,
                        0,
                        0,
                        0,
						FW_NORMAL,
                        FALSE,
                        FALSE,
                        FALSE,
						SHIFTJIS_CHARSET,
                        OUT_DEFAULT_PRECIS,
                        CLIP_DEFAULT_PRECIS,
                        DEFAULT_QUALITY,
                        FIXED_PITCH | FF_MODERN,
						j_font );
#endif

#ifdef _LANGUAGE_CHINESE

	hFont = PT_CreateFont( 12,
                        0,
                        0,
                        0,
						FW_NORMAL,
                        FALSE,
                        FALSE,
                        FALSE,
						GB2312_CHARSET,
                        OUT_DEFAULT_PRECIS,
                        CLIP_DEFAULT_PRECIS,
                        DEFAULT_QUALITY,
                        VARIABLE_PITCH,
						"SimSun" );
#endif

#ifdef _LANGUAGE_THAI
#ifdef _CUSTOM_FONT
/*#ifdef	_ZHOON_TEST
	hFont = PT_CreateFont(z_fonttest.setdata[0].z_nHeight,
						z_fonttest.setdata[0].z_nWidth,
						z_fonttest.setdata[0].z_nEscapement,
						z_fonttest.setdata[0].z_nOrientation,
						z_fonttest.setdata[0].z_nWeight,
						z_fonttest.setdata[0].z_bItalic,
						z_fonttest.setdata[0].z_bUnderline,
						z_fonttest.setdata[0].z_cStrikeOut,
						z_fonttest.setdata[0].z_nCharSet,
						z_fonttest.setdata[0].z_nOutPrecision,
						z_fonttest.setdata[0].z_nClipPrecision,
						z_fonttest.setdata[0].z_nQuality,
                      z_fonttest.setdata[0].z_nPitchAndFamily,
						z_fonttest.setdata[0].z_lpszFacename);
#else*/
	hFont = PT_CreateFont(13,
						0,
						0,
						0,
						FW_THIN,
						FALSE,
						FALSE,
						FALSE,
						THAI_CHARSET,
						OUT_TT_PRECIS,
						CLIP_DEFAULT_PRECIS,
						ANTIALIASED_QUALITY,
                      DEFAULT_PITCH | FF_DONTCARE,
						"MS Sans Serif");
//#endif

#else
	hFont = PT_CreateFont(13,
						0,
						0,
						0,
						FW_THIN,
						FALSE,
						FALSE,
						FALSE,
						THAI_CHARSET,
						OUT_TT_PRECIS,
						CLIP_DEFAULT_PRECIS,
						ANTIALIASED_QUALITY,
                      FIXED_PITCH|FF_MODERN,
						"FreesiaUPC");
#endif					
#endif
#ifdef _LANGUAGE_BRAZIL	
	hFont = PT_CreateFont(16,
						0,
						0,
						0,
						FW_THIN,
						FALSE,
						FALSE,
						FALSE,
						ANSI_CHARSET|FS_LATIN1,
						OUT_TT_PRECIS,
						CLIP_DEFAULT_PRECIS,
						ANTIALIASED_QUALITY,
						FIXED_PITCH|FF_MODERN,
						"Comic Sans MS");
#endif					

#ifdef _LANGUAGE_ARGENTINA
	hFont = PT_CreateFont(16,
						0,
						0,
						0,
						FW_THIN,
						FALSE,
						FALSE,
						FALSE,
						EASTEUROPE_CHARSET,
						OUT_TT_PRECIS,
						CLIP_DEFAULT_PRECIS,
						ANTIALIASED_QUALITY,
						FIXED_PITCH|FF_MODERN,
						"Tahoma");
#endif

#ifdef _LANGUAGE_KOREAN

	hFont = PT_CreateFont( 12,
                        0,
                        0,
                        0,
						FW_NORMAL,
                        FALSE,
                        FALSE,
                        FALSE,
                        HANGEUL_CHARSET,
                        OUT_DEFAULT_PRECIS,
                        CLIP_DEFAULT_PRECIS,
                        DEFAULT_QUALITY,
                        VARIABLE_PITCH,
                        "\261\274\270\262\303\274" );
#endif
#ifdef _LANGUAGE_VEITNAM

	hFont = PT_CreateFont( 14,
                        0,
                        0,
                        0,
						FW_NORMAL,
                        FALSE,
                        FALSE,
                        FALSE,
						VIETNAMESE_CHARSET,
                        OUT_DEFAULT_PRECIS,
                        CLIP_DEFAULT_PRECIS,
                        DEFAULT_QUALITY,
                        VARIABLE_PITCH,
						"Tahoma" );
#endif

	return TRUE;

}

//채팅창 문자 설정및 활성화
int	SetChatingLine( char *szMessage )
{

	cInterFace.ChatFlag = TRUE;
	hFocusWnd = hTextWnd;
	SetWindowText( hFocusWnd, szMessage );
	SetIME_Mode( 1 );		//IME 모드 전환

	//ImmSetConversionStatus( hImc , IME_CMODE_NATIVE,IME_CMODE_NATIVE );
	//ImmSetConversionStatus( hImcEdit , IME_CMODE_NATIVE,IME_CMODE_NATIVE );

	return TRUE;
}

//게임에서의 윈도 메세지 처리
DWORD GameWindowMessage( HWND hWnd,UINT messg,WPARAM wParam,LPARAM lParam )
{
	int stm,cnt;

	switch( messg ) {

	//case WM_IME_KEYDOWN:
	case WM_KEYDOWN:

			if ( GameMode==2 ) {

				if ( wParam==VK_RETURN && VRKeyBuff[wParam]==0 && !sinMessageBoxShowFlag &&
					!VRKeyBuff[VK_CONTROL] ) {

					SetChatingLine( "" );
				}

				if ( wParam==VK_BACK && VRKeyBuff[wParam]==0 ) {
					if ( HoMsgBoxMode ) 
						HoMsgBoxMode = 0;
					else 
						HoMsgBoxMode = 1;

					SetMessageFrameSelect( HoMsgBoxMode );
				}

				if ( wParam=='M' && VRKeyBuff[wParam]==0 ) {
					//안내지도 띄우기
					sinCheck_ShowHelpMap();
				}

				if ( dwYahooTime>dwPlayTime && dwYahooTime<(dwPlayTime+60*1000) ) {
					if ( wParam==VK_SPACE && VRKeyBuff[wParam]==0 ) {
						if ( lpCurPlayer && lpCurPlayer->MotionInfo ) {
							stm = sinGetStamina();
							cnt = (lpCurPlayer->smCharInfo.Stamina[1]*3)/100;
							if ( lpCurPlayer->MotionInfo->State<0x100 && stm>cnt ) {
								if ( lpCurPlayer->SetMotionFromCode( CHRMOTION_STATE_YAHOO ) ) {
									//lpCurPlayer->HideWeapon = TRUE;
									if ( cnt>0 ) sinSetStamina( stm-cnt );
								}
							}
						}
						VRKeyBuff[wParam] = 1;
						return FALSE;
					}
				}

				//######################################################################################
				//작 성 자 : 오 영 석
				if( smRender.m_GameFieldView && !VRKeyBuff[wParam] )
				{
					if( wParam == VK_ADD )
					{
						if( smRender.m_GameFieldViewStep < 22 )
						{
							smRender.m_GameFieldViewStep++;
							//smRender.SetGameFieldViewStep();
							RendSightSub( 1 );
						}
					}
					else if( wParam == VK_SUBTRACT )
					{
						if( smRender.m_GameFieldViewStep > 1 )
						{
							smRender.m_GameFieldViewStep--;
							//smRender.SetGameFieldViewStep();
							RendSightSub( 1 );
						}
					}
				}
				//######################################################################################



#ifdef _WINMODE_DEBUG

				if (smConfig.DebugMode ) {
					//디버깅용 기능
					if ( wParam == VK_F9 && VRKeyBuff[VK_F9]==0 ) {
						if ( DisplayDebug ) DisplayDebug = FALSE;
						else DisplayDebug = TRUE;
					}
/*
	E_BL_LODING		   = 0,
	E_BL_CHANGE_TRUE   = 1,
	E_BL_CHANGE_FALSE  = 2,
	E_BL_FONT_ROUND    = 3,
	E_BL_FONT_COUNT_1  = 4,
	E_BL_FONT_COUNT_2  = 5,
	E_BL_FONT_COUNT_3  = 6,
	E_BL_FONT_COUNT_4  = 7,
	E_BL_FONT_COUNT_5  = 8,
	E_BL_FONT_COUNT_6  = 9,
	E_BL_FONT_COUNT_7  = 10,
	E_BL_FONT_START    = 11,	
	E_BL_FONT_STAGE    = 12,
	E_BL_FONT_COMPLETE = 13,
	E_BL_FONT_FAIL	   = 14,
*/
					if ( VRKeyBuff[VK_CONTROL] && wParam == 'E' && VRKeyBuff['E']==0 ) {
						//연결 효과음 삽입
						//PlayBGM_Direct( BGM_CODE_SOD1 );
/*
						if ( !BellatraEffectInitFlag ) {
							CreateBellatraFontEffect();
							BellatraEffectInitFlag = TRUE;
						}

						SetBellatraFontEffect(E_BL_CHANGE_TRUE);
*/
/*
extern char	szSOD_String[64];
extern BYTE	bSOD_StringColor[4];
extern int	SOD_StringCount;
*/
						lstrcpy( szSOD_String , "Score up 5000 pts" );
						bSOD_StringColor[0] = 128;
						bSOD_StringColor[1] = 255;
						bSOD_StringColor[2] = 128;
						SOD_StringCount = 256;
					}


					//몬스터 생성
					//int SendOpenMonster( int State )
					//if ( lpCurPlayer->smCharInfo.State!=smCHAR_STATE_USER && VRKeyBuff[wParam]==0 ) {
/*
					if ( VRKeyBuff[VK_CONTROL] && VRKeyBuff[wParam]==0 ) {
						//몬스터 생성
						if ( wParam == '6' ){
								SendOpenMonster( -1 );				//랜덤
						}
						//몬스터 생성
						if ( wParam == '7' ){
								SendOpenMonster( 0 );
						}
						//몬스터 생성
						if ( wParam == '8' ) {
								SendOpenMonster( 1 );
						}
						//몬스터 생성
						if ( wParam == '9' ) {
								SendOpenMonster( 2 );
						}
						//몬스터 생성
						if ( wParam == '0' ) {
								SendOpenMonster( 3 );
						}
						//몬스터 생성
						if ( wParam == 189 ) {//'-' ) {
								SendOpenMonster( 4 );
						}
					}
*/
					//######################################################################################
					//작 성 자 : 오 영 석
					//if ( VRKeyBuff[VK_SHIFT] ) {
					if ( VRKeyBuff[VK_SHIFT] && ! VRKeyBuff[VK_CONTROL] ) {
					//######################################################################################					
						if ( wParam == VK_F5 && VRKeyBuff[VK_F5]==0 ) {
							SkipNextField = 1;
						}
						if ( wParam == VK_F6 && VRKeyBuff[VK_F6]==0 ) {
							SkipNextField = 2;
						}

						//안보이는 벽 표시 비표시
						if ( wParam == VK_F7 && VRKeyBuff[VK_F7]==0 ) {
							if ( smRender.dwMatDispMask ) 
								smRender.dwMatDispMask = 0;
							else
								smRender.dwMatDispMask = sMATS_SCRIPT_NOTVIEW;
						}
/*
						if ( wParam==VK_F11 && VRKeyBuff[wParam]==0 ) {
							if ( DispInterface==0 ) DispInterface=TRUE;
							else DispInterface=FALSE;
						}
*/

						if ( wParam == VK_F8 && VRKeyBuff[wParam]==0 ) {
							//클랜 정보 표시
							if ( ktj_imsiDRAWinfo ) ktj_imsiDRAWinfo=0;
							else ktj_imsiDRAWinfo=1;
						}
					}

						if ( wParam==VK_F11 && VRKeyBuff[wParam]==0 ) {
							if ( DispInterface==0 ) DispInterface=TRUE;
							else DispInterface=FALSE;
						}



					if ( AdminCharMode ) {
						if ( wParam==VK_INSERT && VRKeyBuff[wParam]==0 && VRKeyBuff[VK_CONTROL] ) {
							//시작 지점 추가
							SendAdd_Npc( lpCurPlayer , 0 );
						}
						if ( wParam==VK_DELETE && VRKeyBuff[wParam]==0 && VRKeyBuff[VK_CONTROL] ) {
							//시작 지점 추가
							if ( lpCharSelPlayer ) {
								//NPC 캐릭터 제거
								SendDelete_Npc( lpCharSelPlayer );
							}
						}
					}
					else {
						if ( wParam==VK_INSERT && VRKeyBuff[wParam]==0 && VRKeyBuff[VK_CONTROL] ) {
							//시작 지점 추가
							SendAddStartPoint( lpCurPlayer->pX , lpCurPlayer->pZ );
						}

						if ( wParam==VK_DELETE && VRKeyBuff[wParam]==0 && VRKeyBuff[VK_CONTROL] ) {
							//시작 지점 추가
							if ( lpSelItem ) {
								SendDeleteStartPoint( lpSelItem->pX , lpSelItem->pZ );
							}
						}
					}
				}
#endif

			}
			break;
	}

	return TRUE;
}



//퀘스트 메세지 보드 설정 하는곳
#include "sinbaram\\HaQuestBoard.h"




//퀘스트 시작
int	StartQuest_Code( DWORD wCode )
{
	SetQuestBoard();


	if (InterfaceParty.PartyPosState==PARTY_NONE) {
		//퀘스트 창을 보여준다 
		ShowQuest();
		InterfaceParty.quest_Sel_Progress();	//퀘스트진행버튼누른걸로 셋팅함.
	}

	return TRUE;
}

//퀘스트 완료
int EndQuest_Code( DWORD wCode )
{
	//종료된 퀘스트 기록 추가
	Record_LastQuest( (WORD)wCode );
	SetQuestBoard();

	return TRUE;
}

//해외용 함수
int HoInstallFont()
{

	//폰트를 등록한다
#ifdef	_LANGUAGE_ENGLISH		//C7
	AddFontResource("ptz.ttf");
#endif

#ifdef	_LANGUAGE_THAI		//C7
/*#ifdef	_ZHOON_TEST
	AddFontResource(z_fonttest.z_FontFileName);
#else*/
	AddFontResource("ssee874.fon");
//#endif	
#endif
	
	

	//레지스트리에 등록한다.
	OSVERSIONINFO vi;
	vi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	char szKey[255];
	GetVersionEx(&vi);
	if (vi.dwPlatformId == VER_PLATFORM_WIN32_NT)
		strcpy(szKey, "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Fonts");
	else
		strcpy(szKey, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Fonts");

#ifdef	_LANGUAGE_ENGLISH		//C7
	SetRegString(HKEY_LOCAL_MACHINE, szKey, "ptz", "ptz.ttf");
#endif
#ifdef	_LANGUAGE_THAI	
/*#ifdef	_ZHOON_TEST
	SetRegString(HKEY_LOCAL_MACHINE, szKey, z_fonttest.z_FontName,z_fonttest.z_FontFileName);
#else*/
	SetRegString(HKEY_LOCAL_MACHINE, szKey, "MS Sans Serif", "ssee874.fon");
//#endif	
#endif

	
	return TRUE;
}

//해외용 함수
//폰트 삭제하기
int HoUninstallFont()
{
	//폰트를 제거한다.
#ifdef	_LANGUAGE_ENGLISH		//C7
	RemoveFontResource("ptz.ttf");
#endif
#ifdef	_LANGUAGE_THAI		//C7
/*#ifdef	_ZHOON_TEST
	RemoveFontResource(z_fonttest.z_FontFileName);	
#else*/
	RemoveFontResource("ssee874.fon");
//#endif
#endif	

	//레지스트리 값을 지운다.
	OSVERSIONINFO vi;
	vi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	char szKey[255];
	GetVersionEx(&vi);
	if (vi.dwPlatformId == VER_PLATFORM_WIN32_NT)
		strcpy(szKey, "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Fonts");
	else
		strcpy(szKey, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Fonts");
	HKEY key;
	DWORD dwDisp;
	RegCreateKeyEx(HKEY_LOCAL_MACHINE, szKey, 0, NULL,
		REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &key, &dwDisp);

#ifdef	_LANGUAGE_ENGLISH		//C7
	RegDeleteValue(key,"ptz");
#endif
#ifdef	_LANGUAGE_THAI		//C7
/*#ifdef	_ZHOON_TEST	
	RegDeleteValue(key,z_fonttest.z_FontName);
#else*/
	RegDeleteValue(key,"MS Sans Serif");
//#endif	
#endif		

	RegCloseKey(key);

	return TRUE;
}
