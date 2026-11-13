#include "smlib3d\\smd3d.h"

#include "smwsock.h"
#include "character.h"
#include "avictrl.h"
#include "playmain.h"
#include "srcsound\\dxwav.h"
#include "fileread.h"

#include "particle.h"
#include "netplay.h"

#include "sinbaram\\sinlinkheader.h"
#include "hobaram\\holinkheader.h"

#include "field.h"
#include "effectsnd.h"

#include "record.h"
#include "playsub.h"
#include "resource.h"
#include "drawsub.h"
#include "language.h"

#include "tjboy\\clanmenu\\tjclanDEF.h"
#include "tjboy\\clanmenu\\tjclan.h"
#include "tjboy\\clanmenu\\cE_CViewClanInfo.h"

#include "srcserver\\onserver.h"

#include "BellatraFontEffect.h"
#include "TextMessage.h"

//######################################################################################
//작 성 자 : 오 영 석
#include "WinInt\\WinIntThread.h"
#include "FullZoomMap.h"
#include "FontImage.h"
//######################################################################################

//박재원 - XignCode
#ifdef _XIGNCODE_CLIENT
#include "Xigncode\Client\zwave_sdk_client.h"
#endif

extern HINSTANCE hinst; 

/////////////////////// 서버측 함수 불러다 쓴다 //////////////////////
//이벤트 지역인지 첵크
int rsCheckHardCoreEventArea( int x, int y, int z ) ;
extern POINT3D	rsHardCoreEventOpenPos[7][3];

//////////////////////////////////////////////////////////////////////

int	CodeXorExp  = 0;				//경험치 숨김코드_
int	CodeXorExp_High  = 0;			//경험치 숨김코드
int	CodeXorLife = 0;				//생명력 숨김코드
int	CodeXorMana = 0;				//기력 숨김코드
int	CodeXorStamina = 0;				//근력 숨김코드
int	CodeXorMoney  = 0;				//돈 숨김코드

//이쪽 모듈 보호 첵크섬 코드
#define	PLAYSUB_CHECKRESULT			                                                               1884018394

DWORD	dwPlaySub_CheckResult = PLAYSUB_CHECKRESULT;

static char *szArmorDirectory = "char\\tmABCD\\";
static char *szNpcDirectory = "char\\npc\\";


#ifdef _PLAY_MORYON
#define MAX_JOBNUM	11		// 8 + the Assassin (9), the Shaman (10), the Martial Artist (11)
#else
#define MAX_JOBNUM	4
#endif

#include "playmodel.h"

// class -> armour row of szArmorNumFile (rows 8..10: e / me / f bodies)
int ArmorJobNum[12] = { 0, 1, 0, 3, 2, 5, 4, 7, 6, 8, 9, 10 };

// a class's head table (playmodel.h), NULL for none
static char *(*GetClassFaceTable( int jobCode ))[HEAREPOTION_MAX]
{
	switch( jobCode ) {
		case 1:		return szModel_FighterFaceName;
		case 2:		return szModel_MechFaceName;
		case 3:		return szModel_ArcherFaceName;
		case 4:		return szModel_PikeFaceName;
		case 5:		return szModel_AtalantaFaceName;
		case 6:		return szModel_KnightFaceName;
		case 7:		return szModel_Magican;
		case 8:		return szModel_PristessFaceName;
		case JOBCODE_ASSASSIN:		return szModel_AssassinFaceName;
		case JOBCODE_SHAMAN:		return szModel_ShamanFaceName;
		case JOBCODE_MARTIALARTIST:	return szModel_MartialFaceName;
	}
	return NULL;
}

int	PlayerArmor = 0;

#define TEMP_DYNPAT_MAX		2048

smDPAT	*DynTempPat[TEMP_DYNPAT_MAX];

int DynTempPatCnt = 0;

int	PlayUsed_Resurrection = 0;		//부활 아이템 사용


DWORD	dwSafeCodeSum;
//0xE9
BYTE	dwAsmJmpCode = 0xE0;

DWORD	dwStartFuncPoint=0;
DWORD	dwFuncLen=0;
DWORD	dwFuncSafeMemCount;
DWORD	dwFuncSafeMemCode;

//////////////////////////////////////////
DWORD	dwFuncChkTime = 0;
DWORD	dwFuncSafeCode = 0;
//////////////////////////////////////////

char	*lpFuncBackupBuff = 0;



LPDIRECTDRAWSURFACE4	lpDDS_CodeSaveBuff =0;

extern char *szBangImage;

DWORD	*lpCodeVram = 0;

//시작 펑션
int	PlaySub_StartFunc()
{
	return FALSE;
}

//VRAM에 데이타 저장
int Code_VRamBuffOpen()
{

	lpCodeVram = new DWORD[256];

	return (DWORD)lpCodeVram;

}

int Code_VramClear()
{
	lpCodeVram[0] = 777;
	lpCodeVram[1] = 0;
	lpCodeVram[2] = 777;
	lpCodeVram[3] = 0;

	return TRUE;
}


int Code_VramSave( DWORD Param1, DWORD Param2 , DWORD Param3 )
{
	DWORD	dwTime;

	if ( lpCodeVram ) {
		dwTime = GetCurrentTime();

		lpCodeVram[3] = dwTime;
		lpCodeVram[0] = Param1^dwTime;
		lpCodeVram[1] = Param2^dwTime;
		lpCodeVram[2] = Param3^dwTime;
	}

	return TRUE;
}

int Code_VramLoad( DWORD *Param1, DWORD *Param2 , DWORD *Param3 )
{
	*Param1 = lpCodeVram[0]^lpCodeVram[3];
	*Param2 = lpCodeVram[1]^lpCodeVram[3];
	*Param3 = lpCodeVram[2]^lpCodeVram[3];

	return TRUE;
}

int	Mix_CodeVram()
{
	DWORD	dwTime;
	DWORD	dwNewTime;


	dwTime = (dwPlayTime>>4)&0xFF;
	dwNewTime = (dwTime<<24)|(dwTime<<16)|(dwTime<<8)|dwTime;

	dwTime = lpCodeVram[3];

	lpCodeVram[0] = (lpCodeVram[0]^dwTime)^dwNewTime;
	lpCodeVram[1] = (lpCodeVram[1]^dwTime)^dwNewTime;
	lpCodeVram[2] = (lpCodeVram[2]^dwTime)^dwNewTime;
	lpCodeVram[3] = dwNewTime;

	return TRUE;
}



//////////////// 프로그래밍 코드 보호 /////////////
/////////////// ( 디스어셈블 조작방지 ) ///////////
//코드보호
DWORD Check_CodeSafe( DWORD dwStartFunc )
{
	BYTE	*lpFunc;
	//int		cnt;
	DWORD	PointMin,PointMax;
	//DWORD	dwFunc;
	DWORD	dwCode,dwCnt;
	DWORD	dwLen;
	DWORD	*lpdwFunc;
	BYTE	JmpCode = 9;
	DWORD	dwCodeSeg;
	DWORD	dwCodeAddr;
	DWORD	dwHeaderPos = 0x30;

	JmpCode+=dwAsmJmpCode;

	dwStartFunc = 0x4c11cc;

	lpFunc = (BYTE *)dwStartFunc;
	dwCodeSeg = dwStartFunc&0xFFF00000;
	dwCodeAddr = dwStartFunc&0x000FFFFF;

	lpdwFunc = (DWORD *)(dwCodeSeg+dwHeaderPos+0xC);
	lpdwFunc = (DWORD *)(lpdwFunc[0]+dwCodeSeg);
	PointMax = lpdwFunc[0x4B]+dwCodeSeg;
	PointMin = dwCodeSeg+0x1000;

/*
	dwFunc = ((DWORD *)(lpFunc+1))[0];
	dwFunc += (DWORD)(lpFunc+5);

	PointMin = dwFunc;
	PointMax = dwFunc;

	cnt = 0;
	while(1) {
		if ( lpFunc[cnt]!=JmpCode ) break;
		dwFunc = ((DWORD *)(lpFunc+cnt+1))[0];
		dwCode = (DWORD)(lpFunc+cnt);
		dwCode += dwFunc+5;

		if ( dwCode>PointMax ) PointMax = dwCode;
		if ( dwCode<PointMin ) PointMin = dwCode;
		cnt-=5;
	}

	cnt = 0;
	while(1) {
		if ( lpFunc[cnt]!=JmpCode ) break;
		dwFunc = ((DWORD *)(lpFunc+cnt+1))[0];
		dwCode = (DWORD)(lpFunc+cnt);
		dwCode += dwFunc+5;

		if ( dwCode>PointMax ) PointMax = dwCode;
		if ( dwCode<PointMin ) PointMin = dwCode;
		cnt+=5;
	}
*/

	dwCode = 0;
	dwLen = (PointMax-PointMin)>>2;
	if ( dwLen>200000 ) dwLen = 200000;

	lpdwFunc = (DWORD *)PointMin;

	dwStartFuncPoint = PointMin^(dwLen<<8);
	dwFuncLen = dwLen;

	for(dwCnt=0;dwCnt<dwLen;dwCnt++) {
		dwCode = dwCode+(lpdwFunc[dwCnt]*((dwCnt&0x3)+1));
	}
/*
	FILE *fp;
	char szBuff[256];
	fp=fopen("memcode.txt","wb");
	if ( fp ) {
		wsprintf( szBuff , "%d %d ( %d ) (Code: %d ) ( %d )",PointMin , PointMax , dwStartFunc ,  dwCode , (DWORD)GetSafeCode );
		fwrite( szBuff , lstrlen(szBuff),1,fp );
		fclose(fp);
	}
*/

	DWORD FnCode;

	dwFuncChkTime = GetCurrentTime();
	FnCode = dwCode ^ ( (dwFuncChkTime<<16)|(dwFuncChkTime&0xFFFF) );

	dwFuncSafeCode = FnCode;
	dwFuncSafeMemCount = 0;
	dwFuncSafeMemCode = 0;

	Code_VramSave( FnCode, dwFuncChkTime , dwCode );			//VRAM 에 저장

	lpFunc = 0;
	lpdwFunc = 0;	

	return dwCode;
}

//코드보호2
DWORD Check_CodeSafe2()
{
	//DWORD	dwCode,dwCnt;
	DWORD	*lpdwFunc;
	DWORD	dwCode;
	int cnt;


	lpdwFunc = (DWORD *)(dwStartFuncPoint^(dwFuncLen<<8));

	if ( dwFuncLen>200000 ) return FALSE;

	for( cnt=0;cnt<32;cnt++) {
	//for( cnt=0;cnt<7;cnt++) {

		dwFuncSafeMemCode = dwFuncSafeMemCode+(lpdwFunc[dwFuncSafeMemCount++] * ((dwFuncSafeMemCount&0x3)+1));

		if ( dwFuncSafeMemCount>=dwFuncLen ) {
			dwFuncChkTime = GetCurrentTime();
			dwCode = dwFuncSafeMemCode ^ ( (dwFuncChkTime<<16)|(dwFuncChkTime&0xFFFF) );
			dwFuncSafeCode = dwCode;

			Code_VramSave( dwCode, dwFuncChkTime , dwFuncSafeMemCode );			//VRAM 에 저장

/*
	FILE *fp;
	char szBuff[256];
	fp=fopen("memcode2.txt","wb");
	if ( fp ) {
		wsprintf( szBuff , "%d %d ( %d ) (Code: %d ) ( %d )",(DWORD)lpdwFunc , dwFuncLen , 0 ,  dwFuncSafeMemCode , (DWORD)GetSafeCode );
		fwrite( szBuff , lstrlen(szBuff),1,fp );
		fclose(fp);
	}
*/
			dwFuncSafeMemCount = 0;
			dwFuncSafeMemCode = 0;
			lpdwFunc = 0;

			return TRUE;
		}
	}

	lpdwFunc = 0;
	return FALSE;
}




//보호 코드 구하기
DWORD	GetSafeCode()
{
	return dwFuncSafeCode ^ ( (dwFuncChkTime<<16)|(dwFuncChkTime&0xFFFF) );
}

//보호 코드 구하기
DWORD	GetSafeCode2()
{
	DWORD	dwCode , dwFnChkTime , dwFnSafeCode;
	DWORD	dwOriginal;

	Code_VramLoad( &dwCode, &dwFnChkTime , &dwFnSafeCode );			//VRAM 에 로딩

	dwOriginal = dwCode ^ ( (dwFnChkTime<<16)|(dwFnChkTime&0xFFFF) );

	if ( (dwOriginal^dwFnSafeCode)==0 ) return dwOriginal;
	return FALSE;
}


//자주쓰는 메시 로드
int	LoadOftenMeshPattern( char *szFileName )
{

	DynTempPat[DynTempPatCnt] = smPattern.LoadCharactor( szFileName );
	if ( DynTempPat[DynTempPatCnt] )
	{
		DynTempPat[DynTempPatCnt]->UseCount = 99;
		DynTempPatCnt++;
		return TRUE;
	}

	return FALSE;
}

#ifdef _CHECK_MESH
//자주쓰는 메시 로드 ( 아이템메쉬 조작검사 기능 추가 )
static DWORD dwOftPatSum = 0;

int	LoadOftenMeshPatternCheckSum( char *szFileName )
{
	smPAT3D *lpPat3D;

	DynTempPat[DynTempPatCnt] = smPattern.LoadCharactor( szFileName );
	if ( DynTempPat[DynTempPatCnt] ) {
		lpPat3D = DynTempPat[DynTempPatCnt]->Pat;
		int szX=0;int szN=0;
		for( int cnt=0;cnt<lpPat3D->nObj3d;cnt++) {
			if ( szX<lpPat3D->obj3d[cnt]->maxY ) 
				szX=lpPat3D->obj3d[cnt]->maxY;
			if ( szN<lpPat3D->obj3d[cnt]->minY ) 
				szN=lpPat3D->obj3d[cnt]->minY;
		}
		DynTempPat[DynTempPatCnt]->UseCount = 99;
		DynTempPatCnt++;

		DWORD dwSpdSum = GetSpeedSum(szFileName);
		DWORD dwSizSum = (szX<<4) | (szN<<1) ;
		dwOftPatSum += dwSpdSum + (dwSpdSum * dwSizSum ) + dwSizSum;
		return TRUE;
	}

	return FALSE;
}

//아이템메쉬 조작검사 결과값 전송
int Send_OftPatSumToDataServer()
{

	smTRANS_COMMAND	smTransCommand;
	smTransCommand.code = smTRANSCODE_CHECK_ITEMMESH;
	smTransCommand.size = sizeof(smTRANS_COMMAND);
	smTransCommand.WParam = dwOftPatSum;
	smTransCommand.SParam = 0;
	smTransCommand.LParam = 0;
	smTransCommand.EParam = 0;

	dwOftPatSum = 0;

	if ( smWsockDataServer ) 
		return smWsockDataServer->Send2( (char *)&smTransCommand , smTransCommand.size , TRUE );

	return FALSE;
}
#endif

int	LoadOftenFullPattern()
{
	int cnt,cnt2;
	int	ErrorCount;
	char	szBuff[128];

	ErrorCount = 0;

	for(cnt=0;cnt<8;cnt++) {
		for(cnt2=0;cnt2<ARMOR_FILE_MAX;cnt2++) {
			lstrcpy( szBuff , szArmorDirectory );
			lstrcat( szBuff, szArmorNumFile[cnt][cnt2] );

			if ( LoadOftenMeshPattern( szBuff )==FALSE ) {
				ErrorCount++;
			}
		}
	}

	for(cnt=0;cnt<8;cnt++) {
		for(cnt2=0;cnt2<ARMOR_FILE_MAX;cnt2++) {
			lstrcpy( szBuff , szArmorDirectory );
			lstrcat( szBuff, szArmorNumFile_Comic[cnt][cnt2] );

			if ( LoadOftenMeshPattern( szBuff )==FALSE ) {
				ErrorCount++;
			}
		}
	}


	for(cnt=0;cnt<FACE_MODEL_MAX;cnt++) {
		for(cnt2=0;cnt2<HEAREPOTION_MAX;cnt2++) {		//0,3 머리 미리 올림
			if ( LoadOftenMeshPattern( szModel_FighterFaceName[cnt][cnt2] )==FALSE )
				ErrorCount++;
			if ( LoadOftenMeshPattern( szModel_MechFaceName[cnt][cnt2] )==FALSE )
				ErrorCount++;
			if ( LoadOftenMeshPattern( szModel_ArcherFaceName[cnt][cnt2] )==FALSE )
				ErrorCount++;
			if ( LoadOftenMeshPattern( szModel_PikeFaceName[cnt][cnt2] )==FALSE )
				ErrorCount++;
			if ( LoadOftenMeshPattern( szModel_KnightFaceName[cnt][cnt2] )==FALSE )
				ErrorCount++;
			if ( LoadOftenMeshPattern( szModel_AtalantaFaceName[cnt][cnt2] )==FALSE )
				ErrorCount++;
			if ( LoadOftenMeshPattern( szModel_PristessFaceName[cnt][cnt2] )==FALSE )
				ErrorCount++;
			if ( LoadOftenMeshPattern( szModel_Magican[cnt][cnt2] )==FALSE )
				ErrorCount++;
		}
	}




	//////////////////// NPC 폴더 ////////////////////
	HANDLE hFindHandle;
	WIN32_FIND_DATA	fd;

	char	szFindPath[64];
	char	szFilePath[64];

	lstrcpy( szFindPath , szNpcDirectory );
	lstrcat( szFindPath , "*.ini" );

	hFindHandle = FindFirstFile( szFindPath , &fd );
	if ( hFindHandle!=INVALID_HANDLE_VALUE ) {
		while(1) {
			lstrcpy( szFilePath , szNpcDirectory );
			lstrcat( szFilePath, fd.cFileName );

			LoadOftenMeshPattern(szFilePath);

			//다음 파일 찾음
			if ( FindNextFile( hFindHandle , &fd )==FALSE ) break;
		}
	}
	FindClose( hFindHandle );


	return ErrorCount;
}



//자주사용하는 모델들 로드
int LoadOftenModels()
{

	return TRUE;
}

char *szOftenNPC[] = {
/*
	"char\\npc\\npc-gard-sword.INI",
	"char\\npc\\npc-gard-spear.INI",
	"char\\npc\\npc-01.ini",
	"char\\npc\\npc-02.INI",
	"char\\npc\\npc-04.ini",
	"char\\npc\\npc-06.ini",
	"char\\npc\\npc-09.ini",
*/
	0
};

//폴더 내의 ini 파일 찿아 읽기
int LoadFolder_Ini( char *szPath , char *szWildCard , char *szChgExt , int OftSave );

int Make_CharAll_SMD()
{
	LoadOftenFullPattern();
	LoadFolder_Ini( "Char\\Npc" , "*.ini" , 0, 0 );
	LoadFolder_Ini( "Char\\Monster" , "*.ini" , 0, 0 );

	return TRUE;
}


//NPC 미리 로딩
int LoadOftenNpc()
{
//537개
	//Make_CharAll_SMD();

#ifdef _W_SERVER
/*
	HANDLE hFindHandle;
	WIN32_FIND_DATA	fd;

	char	*lpPath;
	char	szFindPath[64];
	char	szFilePath[64];
	char	szPath[64];


	lstrcpy( szFindPath , szNpcDirectory );
	lstrcat( szFindPath , "*.inx" );

	hFindHandle = FindFirstFile( szFindPath , &fd );
	if ( hFindHandle!=INVALID_HANDLE_VALUE ) {
		while(1) {
			lstrcpy( szFilePath , szNpcDirectory );
			lstrcat( szFilePath, fd.cFileName );

			lpPath = ChangeFileExt( szFilePath , "ini" );
			lstrcpy( szPath , lpPath );

			LoadOftenMeshPattern(szPath);

			//다음 파일 찾음
			if ( FindNextFile( hFindHandle , &fd )==FALSE ) break;
		}
	}
	FindClose( hFindHandle );
*/
	LoadFolder_Ini( "Char\\Npc" , "*.inx" , "ini" , TRUE );

#else

	int cnt;

	cnt=0;
	while(1) {
		if ( !szOftenNPC[cnt] ) break;
		LoadOftenMeshPattern(szOftenNPC[cnt]);
		cnt++;
	}

#endif

	return TRUE;

}


//얼굴파일 미리 로딩
int LoadOftenFace()
{
/*
	HANDLE hFindHandle;
	WIN32_FIND_DATA	fd;

	char	szFindPath[64];
	char	szFilePath[64];

	lstrcpy( szFindPath , szArmorDirectory );
	lstrcat( szFindPath , "*.inf" );

	hFindHandle = FindFirstFile( szFindPath , &fd );
	if ( hFindHandle!=INVALID_HANDLE_VALUE ) {
		while(1) {
			lstrcpy( szFilePath , szArmorDirectory );
			lstrcat( szFilePath, fd.cFileName );

			LoadOftenMeshPattern(szFilePath);

			//다음 파일 찾음
			if ( FindNextFile( hFindHandle , &fd )==FALSE ) break;
		}
	}
	FindClose( hFindHandle );
*/
	int cnt,cnt2;
	int ErrorCount = 0;

	for(cnt=0;cnt<6;cnt++) {
		//for(cnt2=0;cnt2<3;cnt2++) {					//0,3 머리 미리 올림
		for(cnt2=0;cnt2<HEAREPOTION_MAX;cnt2++) {		//0,3 머리 미리 올림
 			if ( LoadOftenMeshPattern( szModel_FighterFaceName[cnt][cnt2] )==FALSE )
				ErrorCount++;
			if ( LoadOftenMeshPattern( szModel_MechFaceName[cnt][cnt2] )==FALSE )
				ErrorCount++;
			if ( LoadOftenMeshPattern( szModel_ArcherFaceName[cnt][cnt2] )==FALSE )
				ErrorCount++;
			if ( LoadOftenMeshPattern( szModel_PikeFaceName[cnt][cnt2] )==FALSE )
				ErrorCount++;
			if ( LoadOftenMeshPattern( szModel_KnightFaceName[cnt][cnt2] )==FALSE )
				ErrorCount++;
			if ( LoadOftenMeshPattern( szModel_AtalantaFaceName[cnt][cnt2] )==FALSE )
				ErrorCount++;
			if ( LoadOftenMeshPattern( szModel_PristessFaceName[cnt][cnt2] )==FALSE )
				ErrorCount++;
			if ( LoadOftenMeshPattern( szModel_Magican[cnt][cnt2] )==FALSE )
				ErrorCount++;
		}
	}

	if ( ErrorCount ) 
		return FALSE;

	return TRUE;

}


//자주쓰는 갑옷 미리 로딩
int LoadOftenArmor()
{
	int cntW,cntH;
	char szName[128];

	for( cntH=0;cntH<MAX_JOBNUM; cntH++ ) {
		for( cntW=0;cntW<18; cntW++ ) {
			if ( cntW==0 || cntW>=10 ) {
				lstrcpy( szName , szArmorDirectory );
				lstrcat( szName , szArmorNumFile[cntH][cntW] );
				LoadOftenMeshPattern( szName  );
			}
		}
	}

	return TRUE;
}

/*----------------------------------------------------------------------------*
*					    자주쓰는 아이템 미리 로딩 
*-----------------------------------------------------------------------------*/
int  LoadOftenItem()
{
	int cnt;
	char szBuff[256];

	for( cnt=0 ; cnt< MAX_ITEM ; cnt++){
		if( sItem[cnt].DorpItem == NULL ) continue;
		wsprintf(szBuff ,"Image\\sinImage\\Items\\DropItem\\it%s.ASE",sItem[cnt].DorpItem);

#ifdef _CHECK_MESH
		LoadOftenMeshPatternCheckSum( szBuff  );			//자주쓰는 메시 로드 ( 아이템메쉬 조작검사 기능 추가 )
/*
		FILE  *fp = NULL;
		fp=fopen("c:\\Itemlist.txt","a+t");

		if(fp)
		{
			fprintf(fp,"%s\n",szBuff);
		}
		fclose(fp);
*/
#else
		LoadOftenMeshPattern( szBuff  );
#endif
		if ( sItem[cnt].CODE==(sinGG1|sin01) ) break;		//돈 그림이 마지막
	}

#ifdef _CHECK_MESH
	Send_OftPatSumToDataServer();							//아이템메쉬 조작검사 결과값 전송
#endif

	return TRUE;	
}

//자주쓸 메시 텍스쳐 스왑첵크
int CheckOftenMeshTextureSwap()
{

	//######################################################################################
	//작 성 자 : 오 영 석
    return TRUE;
	//######################################################################################

	int cnt;

	for( cnt=0;cnt<DynTempPatCnt;cnt++ ) {
		if ( DynTempPat[cnt] && DynTempPat[cnt]->Pat && DynTempPat[cnt]->Pat->smMaterialGroup ) {
			DynTempPat[cnt]->Pat->smMaterialGroup->CheckMatreialTextureSwap();
		}
	}
	return TRUE;
}


/*----------------------------------------------------------------------------*
*	The body model of the armour and robes past Highlander/Archangel (the gear
*	of levels 103-145): by their item key, as their codes do not follow the old
*	"byte, or byte-17" rule. DA126..DA129 / DA226..DA229 -> model 025..028,
*	DA301..DA310 / DA401..DA410 -> 029..038 (KPT's numbering: a025 is Magic Armor,
*	a026 Cronus). Index into szArmorNumFile (playmodel.h), or -1 for the old rule.
*-----------------------------------------------------------------------------*/
static int NewArmorModel( DWORD dwCode )
{
	for ( int cnt=0; cnt<MAX_ITEM; cnt++ ) {
		if ( sItem[cnt].CODE!=dwCode ) continue;
		const char *k = sItem[cnt].LastCategory;
		if ( (k[0]!='D' && k[0]!='d') || (k[1]!='A' && k[1]!='a') || lstrlen(k)!=5 ) return -1;
		int num = atoi( k+3 ), model = 0;
		if ( (k[2]=='1' || k[2]=='2') && num>=26 && num<=29 ) model = num-1;
		else if ( (k[2]=='3' || k[2]=='4') && num>=1 && num<=10 ) model = 28+num;
		if ( !model ) return -1;
		return 54 + (model-25);			// "X025.ini" is entry 54
	}
	return -1;
}

// a costume's worn model: COSTUME_ARMOR_BASE + its place in szCostumeArmorFile (playmodel_costume.h).
// CA1xx/CA2xx: table 1 at the item number (CA131 -> 31); CA5xx/CA6xx: table 2 at number-1. -1: not one
static int CostumeArmorIndex( DWORD dwCode )
{
	DWORD dwFam = dwCode & sinITEM_MASK2;
	if ( dwFam!=sinCA1 && dwFam!=sinCA2 && dwFam!=sinCA5 && dwFam!=sinCA6 ) return -1;
	for ( int cnt=0; cnt<MAX_ITEM; cnt++ ) {
		if ( sItem[cnt].CODE!=dwCode ) continue;
		int num = lstrlen( sItem[cnt].LastCategory )==5 ? atoi( sItem[cnt].LastCategory+3 ) : 0;
		if ( num<=0 ) return -1;
		int k = ( dwFam==sinCA1 || dwFam==sinCA2 ) ? num : 97 + num-1;
		if ( k<0 || k>=COSTUME_ARMOR_MAX ) return -1;
		return COSTUME_ARMOR_BASE + k;
	}
	return -1;
}

static int ArmorIndexOfCode( DWORD dwCode )
{
	int n = CostumeArmorIndex( dwCode );
	if ( n>=0 ) return n;
	n = NewArmorModel( dwCode );
	if ( n>=0 ) return n;
	//작성자 하 대 용 : 아이템 코드 수정(sin25 이후엔 -16을 해준다.)
	if ( ((dwCode&sinITEM_MASK3)>>8) > 25 ) return ((dwCode&sinITEM_MASK3)>>8)-17;
	return (dwCode&sinITEM_MASK3)>>8;
}

//갑옷 모양 바꾸기
int SetPlayerArmor( int armor )
{
	char szbuff[256];
	int jobNum;

	if ( PlayerArmor==armor ) return FALSE;
	if ( lpCurPlayer->PatLoading ) return FALSE;
	if ( !lpCurPlayer->smCharInfo.szModelName2[0] ) return FALSE;		//머리 없는 캐릭터 값옷 설정 금지 // 얼큰이 오류 수정
	if ( armor == -1) armor=0; 
	
	jobNum = lpCurPlayer->smCharInfo.JOB_CODE;
	if ( jobNum>MAX_JOBNUM ) jobNum=0;
	if ( armor>=COSTUME_ARMOR_BASE && armor<COSTUME_ARMOR_BASE+COSTUME_ARMOR_MAX ) {	// a costume
		lstrcpy( szbuff, szArmorDirectory );
		lstrcat( szbuff, szCostumeArmorFile[ArmorJobNum[jobNum]][ armor-COSTUME_ARMOR_BASE ] );
		lpCurPlayer->PatLoading = TRUE;
		AddLoaderPattern( lpCurPlayer , szbuff , lpCurPlayer->smCharInfo.szModelName2 );
		PlayerArmor = armor;
		return TRUE;
	}
	if ( armor>ARMOR_FILE_MAX-1 ) armor=ARMOR_FILE_MAX-1;

	lstrcpy( szbuff, szArmorDirectory );

	if ( lpCurPlayer->smCharInfo.SizeLevel>0x1000 && lpCurPlayer->smCharInfo.SizeLevel<=0x1002 )
		lstrcat( szbuff, szArmorNumFile_Comic[ArmorJobNum[jobNum]][ armor ] );
	else
		lstrcat( szbuff, szArmorNumFile[ArmorJobNum[jobNum]][ armor ] );

	lpCurPlayer->PatLoading = TRUE;
	AddLoaderPattern( lpCurPlayer , szbuff , lpCurPlayer->smCharInfo.szModelName2 );

	PlayerArmor = armor;

	return TRUE;
}

//갑옷 모양 바꾸기
int SetChrArmor( smCHAR *lpChar , int ItemNum )
{
	char szbuff[256];
	int armor;
	int jobNum;

//	if ( lpChar->PlayerArmor==armor ) return FALSE;
//	if ( lpCurPlayer->PatLoading ) return FALSE;

	if ( !lpChar->DisplayFlag || lpChar->wStickItems[smPLAYBUFF_ITEM_BODY]==ItemNum ) return FALSE;
	if ( lpChar->PatLoading ) return FALSE; 
	if ( !lpChar->smCharInfo.szModelName2[0] ) return FALSE;		//머리 없는 캐릭터 값옷 설정 금지 // 얼큰이 오류 수정

	if ( ItemNum>=0 ){
		armor = ArmorIndexOfCode( sItem[ItemNum].CODE );		// the old rule, or the new armour's model
	}
	else
		armor = 0;

	jobNum = lpChar->smCharInfo.JOB_CODE;
	if ( jobNum>MAX_JOBNUM ) jobNum=0;
	if ( armor>=COSTUME_ARMOR_BASE && armor<COSTUME_ARMOR_BASE+COSTUME_ARMOR_MAX ) {	// another player's costume
		lstrcpy( szbuff, szArmorDirectory );
		lstrcat( szbuff, szCostumeArmorFile[ArmorJobNum[jobNum]][ armor-COSTUME_ARMOR_BASE ] );
		lpChar->PatLoading = TRUE;
		AddLoaderPattern( lpChar , szbuff , lpChar->smCharInfo.szModelName2 );
		return TRUE;
	}
	if ( armor>ARMOR_FILE_MAX-1 ) armor=ARMOR_FILE_MAX-1;

	lstrcpy( szbuff, szArmorDirectory );

	if ( lpChar->smCharInfo.SizeLevel>0x1000 && lpChar->smCharInfo.SizeLevel<=0x1002 )
		lstrcat( szbuff, szArmorNumFile_Comic[ArmorJobNum[jobNum]][ armor ] );
	else
		lstrcat( szbuff, szArmorNumFile[ArmorJobNum[jobNum]][ armor ] );

	lpChar->PatLoading = TRUE;
	AddLoaderPattern( lpChar , szbuff , lpChar->smCharInfo.szModelName2 );

	return TRUE;
}

//얼굴 복구용 코드 생성 // 얼큰이 오류 수정
DWORD GetHairSkinCode( int FaceCode , int HairCode )
{
	return ((FaceCode&0xFF)<<8) | (HairCode&0xFF) | ((((FaceCode+1)+(HairCode+1)*3)&0xFF)<<16);
}

//얼굴 복구 코드 정상인지 확인 // 얼큰이 오류 수정
int CheckHairSkinCode( DWORD dwModelCode )
{
	if ( dwModelCode ) 
	{
		int FaceCode = (dwModelCode>>8)&0xFF;
		int HairCode = dwModelCode&0xFF;
		int Sum = (dwModelCode>>16)&0xFF;
		if ( (((FaceCode+1)+(HairCode+1)*3)&0xFF)==Sum ) return TRUE;
	}
	return FALSE;
}

//얼굴 복구 // 얼큰이 오류 수정
char *GetFaceHairSkin( smCHAR *lpChar )
{

	DWORD dwModelCode = lpChar->smCharInfo.ModelNameCode2;
	int FaceCode = (dwModelCode>>8)&0xFF;
	int HairCode = dwModelCode&0xFF;

	if ( CheckHairSkinCode(dwModelCode)==FALSE ) return NULL;

	char *szHeadName = 0;

	switch( lpChar->smCharInfo.JOB_CODE ) {
		case 1:		//파이터
			szHeadName = szModel_FighterFaceName[FaceCode][HairCode];
			break;
		case 2:		//메카니션
			szHeadName = szModel_MechFaceName[FaceCode][HairCode];
			break;
		case 3:		//아쳐
			szHeadName = szModel_ArcherFaceName[FaceCode][HairCode];
			break;
		case 4:		//파이크맨
			szHeadName = szModel_PikeFaceName[FaceCode][HairCode];
			break;
		case 5:		//아탈란타
			szHeadName = szModel_AtalantaFaceName[FaceCode][HairCode];
			break;
		case 6:		//나이트
			szHeadName = szModel_KnightFaceName[FaceCode][HairCode];
			break;
		case 7:		//매지션
			szHeadName = szModel_Magican[FaceCode][HairCode];
			break;
		case 8:		//프리스티스
			szHeadName = szModel_PristessFaceName[FaceCode][HairCode];
			break;
		case JOBCODE_ASSASSIN:
		case JOBCODE_SHAMAN:
		case JOBCODE_MARTIALARTIST:
			if ( FaceCode<FACE_MODEL_MAX && HairCode<HEAREPOTION_MAX )
				szHeadName = GetClassFaceTable( lpChar->smCharInfo.JOB_CODE )[FaceCode][HairCode];
			break;
	}

	return szHeadName;
}

// 얼큰이 오류 수정
int FindFaceSkin( smCHAR *lpChar , char *lpHeadName , char ***lpSelFace , char **lpSelFaceName )
{

	int cnt,cnt2;
	int FindCount = 0;
	int FaceCode;
	int HairCode;

	switch( lpChar->smCharInfo.JOB_CODE ) {
		case 1:		//파이터
			for( cnt=0;cnt<FACE_MODEL_MAX;cnt++ ) {
				for( cnt2=0;cnt2<HEAREPOTION_MAX;cnt2++ ) {
					if ( lstrcmpi(szModel_FighterFaceName[cnt][cnt2] , lpHeadName )==0 ) 
					{
						*lpSelFace = szModel_FighterFaceName[cnt];
						*lpSelFaceName = szModel_FighterFaceName[cnt][cnt2];
						FaceCode = cnt; HairCode= cnt2 ; FindCount++;
						break;
					}
				}
				if ( cnt2<HEAREPOTION_MAX ) break;
			}
			break;
		case 2:		//메카니션
			for( cnt=0;cnt<FACE_MODEL_MAX;cnt++ ) {
				for( cnt2=0;cnt2<HEAREPOTION_MAX;cnt2++ ) {
					if ( lstrcmpi(szModel_MechFaceName[cnt][cnt2] , lpHeadName )==0 ) 
					{
						*lpSelFace = szModel_MechFaceName[cnt];
						*lpSelFaceName = szModel_MechFaceName[cnt][cnt2];
						FaceCode = cnt; HairCode= cnt2 ; FindCount++;
						break;
					}
				}
				if ( cnt2<HEAREPOTION_MAX ) break;
			}
			break;
		case 3:		//아쳐
			for( cnt=0;cnt<FACE_MODEL_MAX;cnt++ ) {
				for( cnt2=0;cnt2<HEAREPOTION_MAX;cnt2++ ) {
					if ( lstrcmpi(szModel_ArcherFaceName[cnt][cnt2] , lpHeadName )==0 ) 
					{
						*lpSelFace = szModel_ArcherFaceName[cnt];
						*lpSelFaceName = szModel_ArcherFaceName[cnt][cnt2];
						FaceCode = cnt; HairCode= cnt2 ; FindCount++;
						break;
					}
				}
				if ( cnt2<HEAREPOTION_MAX ) break;
			}
			break;
		case 4:		//파이크맨
			for( cnt=0;cnt<FACE_MODEL_MAX;cnt++ ) {
				for( cnt2=0;cnt2<HEAREPOTION_MAX;cnt2++ ) {
					if ( lstrcmpi(szModel_PikeFaceName[cnt][cnt2] , lpHeadName )==0 ) 
					{
						*lpSelFace = szModel_PikeFaceName[cnt];
						*lpSelFaceName = szModel_PikeFaceName[cnt][cnt2];
						FaceCode = cnt; HairCode= cnt2 ; FindCount++;
						break;
					}
				}
				if ( cnt2<HEAREPOTION_MAX ) break;

			}
			break;
		case 5:		//아탈란타
			for( cnt=0;cnt<FACE_MODEL_MAX;cnt++ ) {
				for( cnt2=0;cnt2<HEAREPOTION_MAX;cnt2++ ) {
					if ( lstrcmpi(szModel_AtalantaFaceName[cnt][cnt2] , lpHeadName )==0 ) 
					{
						*lpSelFace = szModel_AtalantaFaceName[cnt];
						*lpSelFaceName = szModel_AtalantaFaceName[cnt][cnt2];
						FaceCode = cnt; HairCode= cnt2 ; FindCount++;
						break;
					}
				}
				if ( cnt2<HEAREPOTION_MAX ) break;
			}
			break;
		case 6:		//나이트
			for( cnt=0;cnt<FACE_MODEL_MAX;cnt++ ) {
				for( cnt2=0;cnt2<HEAREPOTION_MAX;cnt2++ ) {
					if ( lstrcmpi(szModel_KnightFaceName[cnt][cnt2] , lpHeadName )==0 ) 
					{
						*lpSelFace = szModel_KnightFaceName[cnt];
						*lpSelFaceName = szModel_KnightFaceName[cnt][cnt2];
						FaceCode = cnt; HairCode= cnt2 ; FindCount++;
						break;
					}
				}
				if ( cnt2<HEAREPOTION_MAX ) break;
			}
			break;
		case 7:		//매지션
			for( cnt=0;cnt<FACE_MODEL_MAX;cnt++ ) {
				for( cnt2=0;cnt2<HEAREPOTION_MAX;cnt2++ ) {
					if ( lstrcmpi(szModel_Magican[cnt][cnt2] , lpHeadName )==0 ) 
					{
						*lpSelFace = szModel_Magican[cnt];
						*lpSelFaceName = szModel_Magican[cnt][cnt2];
						FaceCode = cnt; HairCode= cnt2 ; FindCount++;
						break;
					}
				}
				if ( cnt2<HEAREPOTION_MAX ) break;
			}
			break;
		case 8:		//프리스티스
			for( cnt=0;cnt<FACE_MODEL_MAX;cnt++ ) {
				for( cnt2=0;cnt2<HEAREPOTION_MAX;cnt2++ ) {
					if ( lstrcmpi(szModel_PristessFaceName[cnt][cnt2] , lpHeadName )==0 )
					{
						*lpSelFace = szModel_PristessFaceName[cnt];
						*lpSelFaceName = szModel_PristessFaceName[cnt][cnt2];
						FaceCode = cnt; HairCode= cnt2 ; FindCount++;
						break;
					}
				}
				if ( cnt2<HEAREPOTION_MAX ) break;
			}
			break;
		case JOBCODE_ASSASSIN:
		case JOBCODE_SHAMAN:
		case JOBCODE_MARTIALARTIST:
			{
				char *(*lpTable)[HEAREPOTION_MAX] = GetClassFaceTable( lpChar->smCharInfo.JOB_CODE );
				for( cnt=0;cnt<FACE_MODEL_MAX;cnt++ ) {
					for( cnt2=0;cnt2<HEAREPOTION_MAX;cnt2++ ) {
						if ( lstrcmpi(lpTable[cnt][cnt2] , lpHeadName )==0 )
						{
							*lpSelFace = lpTable[cnt];
							*lpSelFaceName = lpTable[cnt][cnt2];
							FaceCode = cnt; HairCode= cnt2 ; FindCount++;
							break;
						}
					}
					if ( cnt2<HEAREPOTION_MAX ) break;
				}
			}
			break;
	}

	if ( !FindCount ) {
		// a fifth-rank head ("...d.inf", not in the tables): find it as the fourth rank's ("c")
		// or the first rank's ("a") head of the same hair style. This also takes the bad
		// "tmhC-01d.inf" an earlier build saved on Pikemen.
		int len = lpHeadName ? lstrlen( lpHeadName ) : 0;
		if ( len>5 && len<120 && lstrcmpi( lpHeadName+len-5 , "d.inf" )==0 ) {
			char szAlt[128];
			const char cAlt[2] = { 'c', 'a' };
			for ( int k=0; k<2; k++ ) {
				lstrcpy( szAlt , lpHeadName );
				szAlt[len-5] = cAlt[k];
				int r = FindFaceSkin( lpChar , szAlt , lpSelFace , lpSelFaceName );
				if ( r>=0 ) return r;
			}
		}
		return -1;
	}


	return (FaceCode<<8)|HairCode;

}

//모델 스킨 변경
int	ChangeModelSkin( smCHAR *lpChar , int ComicArmor , int ComicHeadNum , char *szArmor , char *szHead )
{
	char szArmorName[32];
	char szHeadName[32];
	char szbuff1[256];
	char szbuff2[256];

	int len,cnt; // 얼큰이 오류 수정
	int jobNum;
	int SelCount;
	int FindArmor , FindHead;
	char *lpArmorFile;
	char *lpHeadFile;

	// 얼큰이 오류 수정
	char **lpSelFace = 0;
	char *lpSelFaceName = 0;

	jobNum = lpChar->smCharInfo.JOB_CODE;
	if ( jobNum>MAX_JOBNUM ) jobNum=0;

	if ( szArmor ) lpArmorFile = szArmor;
	else lpArmorFile = lpChar->smCharInfo.szModelName;

	if ( szHead ) lpHeadFile = szHead;
	else lpHeadFile = lpChar->smCharInfo.szModelName2;

	szArmorName[0] = 0;
	szHeadName[0] = 0;

	//원본 모델 추출
	len = lstrlen( lpArmorFile );
	for( cnt=len-1;cnt>=0;cnt-- ) {
		if ( lpArmorFile[cnt]=='\\' ) {
			lstrcpy( szArmorName , &lpArmorFile[cnt+1] );
			break;
		}
	}
/*
	len = lstrlen( lpHeadFile );
	for( cnt=len-1;cnt>=0;cnt-- ) {
		if ( lpHeadFile[cnt]=='\\' ) {
			lstrcpy( szHeadName , &lpHeadFile[cnt+1] );
			break;
		}
	}
*/
	lstrcpy( szHeadName , lpHeadFile );


	if ( !szArmorName[0] || !szHeadName[0] ) return FALSE;


	SelCount = ComicHeadNum;
	int bRank5Head = 0;
	if ( SelCount==JOB_FACE_RANK5 ) {		// tier 5: the fourth rank's row, then its tier 5 file
		SelCount = 3;
		bRank5Head = 1;
	}
	if ( SelCount<0 || SelCount>=FACE_MODEL_MAX ) SelCount = 0;

	FindArmor = 0;
	FindHead = 0;

	for( cnt=0;cnt<ARMOR_FILE_MAX;cnt++ ) {
		if ( lstrcmpi( szArmorName, szArmorNumFile[ArmorJobNum[jobNum]][cnt] )==0 ) {
			if ( ComicArmor )
				lstrcpy( szArmorName , szArmorNumFile_Comic[ArmorJobNum[jobNum]][cnt] );
			else
				lstrcpy( szArmorName , szArmorNumFile[ArmorJobNum[jobNum]][cnt] );

			FindArmor++;
			break;
		}
	}
	if ( !FindArmor ) {
		for( cnt=0;cnt<ARMOR_FILE_MAX;cnt++ ) {
			if ( lstrcmpi( szArmorName, szArmorNumFile_Comic[ArmorJobNum[jobNum]][cnt] )==0 ) {
				if ( ComicArmor )
					lstrcpy( szArmorName , szArmorNumFile_Comic[ArmorJobNum[jobNum]][cnt] );
				else
					lstrcpy( szArmorName , szArmorNumFile[ArmorJobNum[jobNum]][cnt] );

				FindArmor++;
				break;
			}
		}
	}

	// 얼큰이 오류 수정
	int FindFaceValue = FindFaceSkin( lpChar , szHeadName , &lpSelFace , &lpSelFaceName );
	if ( FindFaceValue<0 ) return FALSE;
	int HairCode = FindFaceValue&0xFF;
	int FaceCode = FindFaceValue>>8;

	//복구용 얼굴 코드 최초 생성
	if ( CheckHairSkinCode(lpChar->smCharInfo.ModelNameCode2)==FALSE ) {
		lpChar->smCharInfo.ModelNameCode2 = GetHairSkinCode( FaceCode , HairCode );
	}

	switch( jobNum ) 
	{
		case 1:		//파이터
			lstrcpy( szHeadName , szModel_FighterFaceName[SelCount][HairCode] );
			break;
		case 2:		//메카니션
			lstrcpy( szHeadName , szModel_MechFaceName[SelCount][HairCode] );
			break;
		case 3:		//아쳐
			lstrcpy( szHeadName , szModel_ArcherFaceName[SelCount][HairCode] );
			break;
		case 4:		//파이크맨
			lstrcpy( szHeadName , szModel_PikeFaceName[SelCount][HairCode] );
			break;
		case 5:		//아탈란타
			lstrcpy( szHeadName , szModel_AtalantaFaceName[SelCount][HairCode] );
			break;
		case 6:		//나이트
			lstrcpy( szHeadName , szModel_KnightFaceName[SelCount][HairCode] );
			break;
		case 7:		//매지션
			lstrcpy( szHeadName , szModel_Magican[SelCount][HairCode] );
			break;
		case 8:		//프리스티스
			lstrcpy( szHeadName , szModel_PristessFaceName[SelCount][HairCode] );
			break;
		case JOBCODE_ASSASSIN:
		case JOBCODE_SHAMAN:
		case JOBCODE_MARTIALARTIST:
			lstrcpy( szHeadName , GetClassFaceTable( jobNum )[SelCount][HairCode] );
			break;
	}

	// tier 5: the fifth rank's head is the fourth rank's "...c.inf" as "...d.inf" (heads go
	// none/a/b/c/d by rank; "t" is the big-head event head). Hair styles 01..03 have one in every
	// class, loaded from its .inx (no .inf ships); the other styles keep the fourth rank's head.
	// Named from the first rank's head ("...01a.inf" -> "...01d.inf"): the fourth rank's names are
	// not always "c.inf" (the Pikeman's are "tmhC-01c", so the old c->d swap gave "tmhC-01d", which
	// does not exist, and the head failed to load, which also stopped armour showing). Every hair
	// style that has one gets it (KPT has all 13 for most classes); otherwise the fourth rank's head.
	if ( bRank5Head ) {
		char *(*lpFaces)[HEAREPOTION_MAX] = GetClassFaceTable( jobNum );
		if ( lpFaces && HairCode>=0 && HairCode<HEAREPOTION_MAX && lpFaces[1][HairCode] ) {
			char szRank5[128], szInx[128];
			lstrcpy( szRank5 , lpFaces[1][HairCode] );
			len = lstrlen( szRank5 );
			if ( len>5 && lstrcmpi( szRank5+len-5 , "a.inf" )==0 ) {
				szRank5[len-5] = 'd';
				lstrcpy( szInx , szRank5 );
				lstrcpy( szInx+lstrlen(szInx)-4 , ".inx" );
				if ( GetFileAttributes( szRank5 )!=INVALID_FILE_ATTRIBUTES || GetFileAttributes( szInx )!=INVALID_FILE_ATTRIBUTES )
					lstrcpy( szHeadName , szRank5 );
			}
		}
	}


	lstrcpy( szbuff1, szArmorDirectory );
	lstrcat( szbuff1, szArmorName );
	lstrcpy( szbuff2, szHeadName );

	lpChar->PatLoading = TRUE;
	AddLoaderPattern( lpChar , szbuff1 , szbuff2 );

	return TRUE;
}


//머리모양 바꿔버리기  // 얼큰이 오류 수정
int ChangeHairModel( int HairCode )
{

	smCHAR *lpChar = lpCurPlayer;
	int jobNum = lpChar->smCharInfo.JOB_CODE;
	char *lpHeadName = lpChar->smCharInfo.szModelName2;
	char **lpSelFace = 0;
	char *lpSelFaceName = 0;

	if ( HairCode<0 || HairCode >= HEAREPOTION_MAX ) return FALSE;

	int FindFaceValue = FindFaceSkin( lpChar , lpHeadName , &lpSelFace , &lpSelFaceName );

	if ( FindFaceValue<0 || lpSelFace==0 ) return FALSE;

	int FaceCode = FindFaceValue>>8;

	if ( lstrcmpi( lpChar->smCharInfo.szModelName2 , lpSelFace[HairCode] ) == 0 ) 
		return FALSE;

	lstrcpy( lpChar->smCharInfo.szModelName2 , lpSelFace[HairCode] );
	lpChar->smCharInfo.ModelNameCode2 = GetHairSkinCode( FaceCode , HairCode);		//머리변경 코드 저장 (나중에 복구하기 위해서....)

	lpChar->PatLoading = TRUE;
	AddLoaderPattern( lpChar , lpChar->smCharInfo.szModelName , lpChar->smCharInfo.szModelName2 );

	lpCurPlayer->smCharInfo.bUpdateInfo[0]++;
	ReformCharForm();
	SendSetObjectSerial( lpCurPlayer->dwObjectSerial );
	SaveGameData();

	StartSkill( lpCurPlayer->pX,lpCurPlayer->pY,lpCurPlayer->pZ, 0,0,0, SKILL_UP1 );
	SkillPlaySound( SKILL_SOUND_LEARN , lpCurPlayer->pX,lpCurPlayer->pY,lpCurPlayer->pZ );			//스킬 효과음
 

	return TRUE;
}


/*
static short	AgingBlinkColor[8][4] = {
	{ 100,	 60,	 90,	0	},		//약한보라
	{ 140,	120,	  0,	0	},		//노랑
	{	0,	130,	 40,	0	},		//초록
	{	0,	 80,	180,	0	},		//파랑
	{ 100,	  0,	160,	0	},		//보라
	{ 200,	 80,	  0,	0	},		//주황
	{ 180,	 20,	  0,	0	},		//빨강
	{ 255,	 80,	 30,	0	}		//밝은빨강ee
};
*/
/*----------------------------------------------------------------------------*
*Desc: 에이징 차수에 따른 무기 컬러값  		
*-----------------------------------------------------------------------------*/
const int  CONST_AGINGBCOLOR_MAX = 22;		// ages 4..25 (was 20: rows for 4..20)

static short	AgingBlinkColor[CONST_AGINGBCOLOR_MAX][6] =
{
	{ 100,	 40,	 90,	0,	-1,	0			},		//약한보라	
	{ 120,	100,	 10,	0,	-1,	0			},		//노랑
	{   0,	110,	 30,	0,	-1,	0			},		//초록
	{   0,	 50,	140,	0,	 0,	smTEXSTATE_FS_SCROLL5	},		//파랑
	{ 100,	  0,	 90,	0,	 1,	smTEXSTATE_FS_SCROLL6	},		//보라
	{ 150,	 60,	  0,	0,	 2,	smTEXSTATE_FS_SCROLL6	},		//주황
	{ 150,	 10,	  0,	0,	 3,	smTEXSTATE_FS_SCROLL8	},		//빨강
	{  20,	220,	190,	0,	 4,	smTEXSTATE_FS_SCROLL8	},		//청록
	{  10,	220,	 30,	0,	 4,	smTEXSTATE_FS_SCROLL8	},		//연두
	{ 170,	 40,	170,	0,	 4,	smTEXSTATE_FS_SCROLL9	},		//보라
	{ 250,	 30,	160,	0,	 4,	smTEXSTATE_FS_SCROLL9	},		//진한분홍
	{  30,	190,	255,	0,	 4,	smTEXSTATE_FS_SCROLL9	},		//하늘
	{ 250,	130,	 30,	0,	 4,	smTEXSTATE_FS_SCROLL9	},		//연한주황
	{ 120,	 30,	 30,	0,	 5,	smTEXSTATE_FS_SCROLL10	},		//고동
	{ 130,	  0,	255,	0,	 5,	smTEXSTATE_FS_SCROLL10	},		//청보라
	{ 220,	240,	 70,	0,	 5,	smTEXSTATE_FS_SCROLL10	},		//연한노랑
	{ 240,	240,	240,	0,	 5,	smTEXSTATE_FS_SCROLL10	},		// 박재원 - 에이징 설정 변경(20차 에이징 무기 컬러값 수정)
	// +21..+25: +20's white (each a shade apart, so other clients can tell the age) and a light orb
	// (AssaAgingOrb). Keep these last: GetAgingOrbLevel counts from row 17.
	{ 240,	240,	241,	0,	 5,	smTEXSTATE_FS_SCROLL10	},		// +21
	{ 240,	240,	242,	0,	 5,	smTEXSTATE_FS_SCROLL10	},		// +22
	{ 240,	240,	243,	0,	 5,	smTEXSTATE_FS_SCROLL10	},		// +23
	{ 240,	240,	244,	0,	 5,	smTEXSTATE_FS_SCROLL10	},		// +24
	{ 240,	240,	245,	0,	 5,	smTEXSTATE_FS_SCROLL10	},		// +25
};

static short	CraftBlinkColor[6][6] =
{
	{   13,	  0,	  5,	0,	 9,	smTEXSTATE_FS_SCROLL4	},		//클래식
	{	13,	  0,	  6,	0,	 5,	smTEXSTATE_FS_SCROLL4	},		//트랜
	{	13,	  0,	  7,	0,	 6,	smTEXSTATE_FS_SCROLL5	},		//머키
	{	13,	  0,	  8,	0,	 7,	smTEXSTATE_FS_SCROLL5	},		//데비
	{	13,	  0,	  9,	0,	 8,	smTEXSTATE_FS_SCROLL5	},		//셀레
	{   13,	  0,	  10,	0,	 9,	smTEXSTATE_FS_SCROLL5	}		//???
};


//색상으로 아이템의 상태를 판별한다
// aging +21..+25 -> 1..5 (the light orb's level), 0 otherwise, from the tool's blink colours
int GetAgingOrbLevel( smCHARTOOL *lpCharTool )
{
	if ( !lpCharTool || !lpCharTool->ColorBlink ) return 0;
	for( int cnt=17; cnt<CONST_AGINGBCOLOR_MAX; cnt++ ) {
		if ( AgingBlinkColor[cnt][0]==lpCharTool->sColors[SMC_R] &&
			AgingBlinkColor[cnt][1]==lpCharTool->sColors[SMC_G] &&
			AgingBlinkColor[cnt][2]==lpCharTool->sColors[SMC_B] &&
			AgingBlinkColor[cnt][3]==lpCharTool->sColors[SMC_A] )
			return cnt-16;
	}
	return 0;
}

int GetItemKindFromBliankColor( smCHARTOOL	*lpCharTool )
{
	int cnt;

	lpCharTool->TexMixCode = -1;
	lpCharTool->TexScroll  = 0;

	for( cnt=0;cnt<CONST_AGINGBCOLOR_MAX;cnt++ ) {
		if ( AgingBlinkColor[cnt][0]==lpCharTool->sColors[SMC_R] && 
			AgingBlinkColor[cnt][1]==lpCharTool->sColors[SMC_G] && 
			AgingBlinkColor[cnt][2]==lpCharTool->sColors[SMC_B] && 
			AgingBlinkColor[cnt][3]==lpCharTool->sColors[SMC_A] ) {

			lpCharTool->TexMixCode = AgingBlinkColor[cnt][4];
			lpCharTool->TexScroll  = AgingBlinkColor[cnt][5];
			return TRUE;
		}
	}

	for( cnt=0;cnt<6;cnt++ ) {
		if ( CraftBlinkColor[cnt][0]==lpCharTool->sColors[SMC_R] && 
			CraftBlinkColor[cnt][1]==lpCharTool->sColors[SMC_G] && 
			CraftBlinkColor[cnt][2]==lpCharTool->sColors[SMC_B] && 
			CraftBlinkColor[cnt][3]==lpCharTool->sColors[SMC_A] ) {

			lpCharTool->TexMixCode = CraftBlinkColor[cnt][4];
			lpCharTool->TexScroll  = CraftBlinkColor[cnt][5];
			return TRUE;
		}
	}


	return NULL;

}

extern DWORD sinWornArmorCode, sinWornCostumeCode;
void sinRefreshBodyLook();

/////////////// 신바람 인벤토리 변경 호출 함수 ///////////////
int sinSetCharItem(DWORD CODE , DWORD Posi , BOOL SetFlag )
{
	DWORD	dwCode;
	sITEMINFO *lpItemInfo;
	DWORD	dwMotionCode;
	int		FrameCounter;
	int		cnt;

/*
InvenItem[sInven[0].ItemIndex-1] //오른손
InvenItem[sInven[1].ItemIndex-1] //왼손
InvenItem[sInven[2].ItemIndex-1] //갑옷

	int		ColorBlink;
	short	sColors[4];
	DWORD	DispEffect;

*/
	switch( Posi ) {
		case INVENTORY_POS_LHAND:
			if ( SetFlag ) {
				lpCurPlayer->SetTool( CODE , hvPOSI_LHAND );

				lpItemInfo =  &cInvenTory.InvenItem[sInven[1].ItemIndex-1].sItemInfo;
				if ( !lpItemInfo->ItemName[0] )		//이름 없으면 양손무기
					lpItemInfo =  &cInvenTory.InvenItem[sInven[0].ItemIndex-1].sItemInfo;

				//아이템 이펙트 설정
				lpCurPlayer->HvLeftHand.ColorBlink = lpItemInfo->EffectBlink[0];
				memcpy( lpCurPlayer->HvLeftHand.sColors , lpItemInfo->EffectColor , sizeof(short)*4);
				lpCurPlayer->HvLeftHand.DispEffect = lpItemInfo->DispEffect;
				lpCurPlayer->HvLeftHand.BlinkScale = lpItemInfo->ScaleBlink[0];
				lpCurPlayer->HvLeftHand.EffectKind = 0;

				/////////////////////////////////////////////////////////////////
				//에이징 아이템 색상 점멸 이펙트
				if ( lpItemInfo->ItemKindCode==ITEM_KIND_AGING ) {
					cnt = lpItemInfo->ItemAgingNum[0];
					if ( cnt>=4 && cnt-4<CONST_AGINGBCOLOR_MAX ) {
						cnt -= 4;
						lpCurPlayer->HvLeftHand.ColorBlink = 9;
						lpCurPlayer->HvLeftHand.sColors[SMC_R] = AgingBlinkColor[cnt][0];
						lpCurPlayer->HvLeftHand.sColors[SMC_G] = AgingBlinkColor[cnt][1];
						lpCurPlayer->HvLeftHand.sColors[SMC_B] = AgingBlinkColor[cnt][2];
						lpCurPlayer->HvLeftHand.sColors[SMC_A] = AgingBlinkColor[cnt][3];
					}
				}
				//믹스 아이템 색상 점멸 이펙트
				if ( lpItemInfo->ItemKindCode==ITEM_KIND_CRAFT ) {
					cnt = lpItemInfo->ItemAgingNum[0];
					if ( cnt>=6 && cnt<=10 ) {
						cnt -= 5;
					}
					else {
						if ( cnt==0 ) cnt=0;			//클래식 믹스쳐
						else cnt=-1;
					}
					if ( cnt>=0 ) {
						lpCurPlayer->HvLeftHand.ColorBlink = 9;
						lpCurPlayer->HvLeftHand.sColors[SMC_R] = CraftBlinkColor[cnt][0];
						lpCurPlayer->HvLeftHand.sColors[SMC_G] = CraftBlinkColor[cnt][1];
						lpCurPlayer->HvLeftHand.sColors[SMC_B] = CraftBlinkColor[cnt][2];
						lpCurPlayer->HvLeftHand.sColors[SMC_A] = CraftBlinkColor[cnt][3];
					}
				}

				//색상으로 아이템의 상태를 판별한다
				GetItemKindFromBliankColor( &lpCurPlayer->HvLeftHand );
				AssaParticle_AgingOrb( lpCurPlayer );		// aging +21..+25
				/////////////////////////////////////////////////////////////////

				if ( lpCurPlayer->HvLeftHand.DispEffect ) AssaParticle_ClassUpWeapon(lpCurPlayer);

			}
			else
				lpCurPlayer->SetTool( 0 , hvPOSI_LHAND );
		break;

		case INVENTORY_POS_RHAND:
			if ( SetFlag ) {
				lpCurPlayer->SetTool( CODE , hvPOSI_RHAND );

				lpItemInfo =  &cInvenTory.InvenItem[sInven[0].ItemIndex-1].sItemInfo;
				//아이템 이펙트 설정
				lpCurPlayer->HvRightHand.ColorBlink = lpItemInfo->EffectBlink[0];
				memcpy( lpCurPlayer->HvRightHand.sColors , lpItemInfo->EffectColor , sizeof(short)*4);
				lpCurPlayer->HvRightHand.DispEffect = lpItemInfo->DispEffect;
				lpCurPlayer->HvRightHand.BlinkScale = lpItemInfo->ScaleBlink[0];
				lpCurPlayer->HvRightHand.EffectKind = 0;

				/////////////////////////////////////////////////////////////////
				//에이징 아이템 색상 점멸 이펙트
				if ( lpItemInfo->ItemKindCode==ITEM_KIND_AGING ) {
					cnt = lpItemInfo->ItemAgingNum[0];
					if ( cnt>=4 && cnt-4<CONST_AGINGBCOLOR_MAX ) {
						cnt -= 4;
						lpCurPlayer->HvRightHand.ColorBlink = 9;
						lpCurPlayer->HvRightHand.sColors[SMC_R] = AgingBlinkColor[cnt][0];
						lpCurPlayer->HvRightHand.sColors[SMC_G] = AgingBlinkColor[cnt][1];
						lpCurPlayer->HvRightHand.sColors[SMC_B] = AgingBlinkColor[cnt][2];
						lpCurPlayer->HvRightHand.sColors[SMC_A] = AgingBlinkColor[cnt][3];
					}
				}
				//믹스 아이템 색상 점멸 이펙트
				if ( lpItemInfo->ItemKindCode==ITEM_KIND_CRAFT ) {
					/*
					cnt = lpItemInfo->ItemAgingNum[0];
					if ( cnt>=6 && cnt<=10 ) {
						cnt -= 6;
						lpCurPlayer->HvRightHand.ColorBlink = 9;
						lpCurPlayer->HvRightHand.sColors[SMC_R] = CraftBlinkColor[cnt][0];
						lpCurPlayer->HvRightHand.sColors[SMC_G] = CraftBlinkColor[cnt][1];
						lpCurPlayer->HvRightHand.sColors[SMC_B] = CraftBlinkColor[cnt][2];
						lpCurPlayer->HvRightHand.sColors[SMC_A] = CraftBlinkColor[cnt][3];
					}
					*/
					cnt = lpItemInfo->ItemAgingNum[0];
					if ( cnt>=6 && cnt<=10 ) {
						cnt -= 5;
					}
					else {
						if ( cnt==0 ) cnt=0;			//클래식 믹스쳐
						else cnt=-1;
					}
					if ( cnt>=0 ) {
						lpCurPlayer->HvRightHand.ColorBlink = 9;
						lpCurPlayer->HvRightHand.sColors[SMC_R] = CraftBlinkColor[cnt][0];
						lpCurPlayer->HvRightHand.sColors[SMC_G] = CraftBlinkColor[cnt][1];
						lpCurPlayer->HvRightHand.sColors[SMC_B] = CraftBlinkColor[cnt][2];
						lpCurPlayer->HvRightHand.sColors[SMC_A] = CraftBlinkColor[cnt][3];
					}
				}

				//색상으로 아이템의 상태를 판별한다
				GetItemKindFromBliankColor( &lpCurPlayer->HvRightHand );
				AssaParticle_AgingOrb( lpCurPlayer );		// aging +21..+25

				//////////////////////////////////////////////////////////////////
				if ( lpCurPlayer->HvRightHand.DispEffect ) AssaParticle_ClassUpWeapon(lpCurPlayer);

			}
			else
				lpCurPlayer->SetTool( 0 , hvPOSI_RHAND );
		break;

		case INVENTORY_POS_ARMOR:
			//옷 갑아 입기
			sinWornArmorCode = ( CODE!=NULL && SetFlag ) ? CODE : 0;
			sinRefreshBodyLook();
		break;

		case INVENTORY_POS_COSTUME:		// the costume slot: shown instead of the armour in costume view
			sinWornCostumeCode = ( CODE!=NULL && SetFlag ) ? CODE : 0;
			sinRefreshBodyLook();
		break;
	}
	return TRUE;
}

/*----------------------------------------------------------------------------*
*	The costume slot (3215 inventory): the body shows the costume while the
*	costume view is on (the inventory's up arrow, or T) and one is worn, else
*	the armour. Others see the same (wStickItems body = the item shown). The
*	costume's stats count either way (it is an item in a slot: SetItemToChar).
*-----------------------------------------------------------------------------*/
DWORD sinWornArmorCode = 0;		// what the armour slot holds (0: nothing)
DWORD sinWornCostumeCode = 0;	// what the costume slot holds
int sinCostumeView = 0;			// 1: the costume page and look (when one is worn), 0: the gear (at login)

void sinRefreshBodyLook()
{
	extern int sinCostumeFits( DWORD dwCode , int JobCode );		// sinInvenTory.cpp: gender, armour or robe
	DWORD dwShow = ( sinCostumeView && sinWornCostumeCode && sinCostumeFits( sinWornCostumeCode , lpCurPlayer->smCharInfo.JOB_CODE ) )
		? sinWornCostumeCode : sinWornArmorCode;
	DWORD dwMotionCode = lpCurPlayer->MotionInfo->State;
	int FrameCounter = lpCurPlayer->FrameCounter;

	if ( dwShow ) {
		SetPlayerArmor( ArmorIndexOfCode( dwShow ) );		// the old rule, the new armour's model, or a costume
		lpCurPlayer->wStickItems[smPLAYBUFF_ITEM_BODY] = GetSinItemNumFromCode( dwShow );
	}
	else {
		SetPlayerArmor( 0 );
		lpCurPlayer->wStickItems[smPLAYBUFF_ITEM_BODY] = -2;
	}
	if ( dwMotionCode==CHRMOTION_STATE_DEAD ) {
		//죽었을때 옷 갈아입어도 소용 없다
		lpCurPlayer->SetMotionFromCode( dwMotionCode );
		lpCurPlayer->frame = (lpCurPlayer->MotionInfo->EndFrame-1)*160;
		lpCurPlayer->FrameCounter = FrameCounter;
	}
}

void sinToggleCostumeView()
{
	sinCostumeView = !sinCostumeView;
	sinRefreshBodyLook();
}

//아이템 버리기 함수
int sinThrowItemToFeild(sITEM *pItem)
{

	sITEM	sThrowItem;
	int		ThrowItemBackup = 0;

	if( (pItem->sItemInfo.CODE & sinITEM_MASK2) == sinBI1 || (pItem->sItemInfo.CODE & sinITEM_MASK2) == sinBI2 ){   //패키지 아이템도 버릴수없다
		if(pItem->sItemInfo.PotionCount == 0)
			return FALSE;
	}


	//창고가 열려있을경우에 아이템이 버려질경우를 체크한다
	if(cWareHouse.OpenFlag){
		if(!cWareHouse.CloseWareHouse()){
			pItem->Flag = 0;
			return FALSE;
		}

	}

	if ( lpCurPlayer->OnStageField>=0 && StageField[ lpCurPlayer->OnStageField ]->FieldCode==QUEST_ARENA_FIELD ) {
		//경기장 필드 (포스와 초코렛은 사용가능)
		if ( (pItem->sItemInfo.CODE&sinITEM_MASK2)!=sinFO1 && (pItem->sItemInfo.CODE&sinITEM_MASK2)!=sinCH1 )
			return FALSE;	//아이템 버려지지 않게

		if ( pItem->sItemInfo.PotionCount!=100 )
			return FALSE;

	}

	if ( pItem->sItemInfo.CODE!=(sinGG1|sin01) && !pItem->Flag ) return FALSE;

	if ( pItem->sItemInfo.PotionCount==100 ) {

		//공성 용병 크리스탈 첵크중
		int BlessCristalCode = -1;
		int BlessSolderTotal;

		switch( pItem->sItemInfo.CODE ) {
		case sinGP1|sin14:	
			BlessCristalCode = 0;
			break;
		case sinGP1|sin15:	
			BlessCristalCode = 1;
			break;
		case sinGP1|sin16:	
			BlessCristalCode = 2;
			break;
		}
		if ( BlessCristalCode>=0 ) {
			if ( !rsBlessCastle.CastleMode || 
				(lpCurPlayer->OnStageField>=0 && sField[lpCurPlayer->OnStageField].FieldCode!=rsCASTLE_FIELD) &&
				!lpCurPlayer->Clan_CastleMasterFlag ) 
			{
				//공성모드 아니거다 사용할수 없는 위치 또는 성주클랜이 아님 ( 사용불가 )
				cMessageBox.ShowMessageEvent(mgBlessCrystal_03);
				return FALSE;
			}

			BlessSolderTotal =	rsBlessCastle.bCrystalSolderCount[0]+
								rsBlessCastle.bCrystalSolderCount[1]+
								rsBlessCastle.bCrystalSolderCount[2]+
								rsBlessCastle.bCrystalSolderCount[3];

			if ( rsBlessCastle.bCrystalSolderCount[BlessCristalCode]>=srCASLTE_SOLDER_KIND_MAX ||
				BlessSolderTotal>=srCASLTE_SOLDER_CLAN_MAX ) {
					//발생수량 초과
					cMessageBox.ShowMessageEvent(mgBlessCrystal_01);
					return FALSE;
				}
		}
	}



	//아이템 인증 확인
	if ( !CheckItemForm( &pItem->sItemInfo ) ) {
		SendSetHackUser( 80 );
		return TRUE;
	}

	//무계 초과시 버려지 못하게
	//if ( lpCurPlayer->smCharInfo.Weight[0]>lpCurPlayer->smCharInfo.Weight[1] ) return FALSE;

	// 석지용 - 믹스쳐 리셋 플랙 추가
	if ( cShop.OpenFlag || SkillMasterFlag || cCraftItem.OpenFlag || cAging.OpenFlag || SmeltingItem.OpenFlag || ManufactureItem.m_OpenFlag || cMixtureReset.OpenFlag)  {
		if ( pItem->sItemInfo.CODE==(sinGG1|sin01) ) return FALSE;
	}

	if ( (pItem->sItemInfo.CODE&sinITEM_MASK1)==(sinPM1&sinITEM_MASK1) ) {
		//물약 재인증
		ThrowItemBackup = 1;
		memcpy( &sThrowItem , pItem , sizeof(sITEM) );
		ReformItem( &pItem->sItemInfo );
	}

	if ( ThrowPutItem( pItem , lpCurPlayer->pX , lpCurPlayer->pY , lpCurPlayer->pZ )==TRUE ) {
		SubInvenItemCode( &pItem->sItemInfo );
		pItem->Flag = 0;
//		ResetInvenItemCode();				//이벤 아이템 코드 초기화
/*
		//몬스터 소환 동작 ( 공격 동작으로 대신 사용 )
		if ( (pItem->sItemInfo.CODE&sinITEM_MASK2)==sinGP1 && pItem->sItemInfo.PotionCount==100 ) {
			if ( lpCurPlayer->MotionInfo->State<0x100 && lpCurPlayer->OnStageField>=0 && StageField[ lpCurPlayer->OnStageField ]->State!=FIELD_STATE_VILLAGE ) {
				lpCurPlayer->SetMotionFromCode( CHRMOTION_STATE_ATTACK );
			}
		}
*/

		return TRUE;
	}

	if ( ThrowItemBackup )
		memcpy( pItem, &sThrowItem , sizeof(sITEM) );

	return FALSE;
}

//물약 먹기
int sinActionPotion()
{
	if ( sinGetLife()>0 && lpCurPlayer->MotionInfo->State!=CHRMOTION_STATE_EAT 
		&& lpCurPlayer->MotionInfo->State!=CHRMOTION_STATE_DEAD ) {
		if ( lpCurPlayer->MotionInfo->State==CHRMOTION_STATE_ATTACK || lpCurPlayer->MotionInfo->State==CHRMOTION_STATE_SKILL ) 
			lpCurPlayer->dwNextMotionCode = CHRMOTION_STATE_EAT;
		else
			lpCurPlayer->SetMotionFromCode( CHRMOTION_STATE_EAT );
		return TRUE;
	}

	return FALSE;
}

//귀환아이템 사용
int	ActionEtherCore( sITEM	*lpItem )	//DWORD dwCode )
{

	DWORD	dwCode = lpItem->sItemInfo.CODE;

	if ( lpCurPlayer->OnStageField>=0 && StageField[lpCurPlayer->OnStageField]->State==FIELD_STATE_ROOM ) {
		//방에서는 귀환아이템 사용 불가
		return TRUE;
	}
	if ( lpCurPlayer->smCharInfo.wPlayerKilling[0] ) {
		//감옥에서는 귀환아이템 사용 불가
		return TRUE;
	}

	if ( sinGetLife()>0 && lpCurPlayer->MotionInfo->State!=CHRMOTION_STATE_DEAD ) {

		if ( lpCurPlayer->MotionInfo->State==CHRMOTION_STATE_ATTACK || lpCurPlayer->MotionInfo->State==CHRMOTION_STATE_SKILL ) 
			lpCurPlayer->dwNextMotionCode = CHRMOTION_STATE_EAT;
		else {
			lpCurPlayer->SetMotionFromCode( CHRMOTION_STATE_EAT );
			StartEffect( lpCurPlayer->pX,lpCurPlayer->pY+48*fONE,lpCurPlayer->pZ, EFFECT_RETURN1 );
			SkillPlaySound( SKILL_SOUND_LEARN , lpCurPlayer->pX,lpCurPlayer->pY,lpCurPlayer->pZ );			//스킬 효과음
		}

		UseEtherCoreCode = dwCode;

		//lpItem = &cInvenTory.InvenItem[SelectInvenItemIndex-1];

		if ( (lpItem->sItemInfo.CODE&sinITEM_MASK2 )==sinEC1 ) {
			//사용한 아이템 코드 서버로 보냄
			SendUseItemCodeToServer( &lpItem->sItemInfo );
		}

		return TRUE;
	}

	return FALSE;
}


//죽는경우 패널티 값
int GetDeathPenalty() 
{
	sITEMINFO	*lpItemInfo;

	int	Penalty = 0;

	if(sInven[4].ItemIndex){
		lpItemInfo = &cInvenTory.InvenItem[sInven[4].ItemIndex-1].sItemInfo;
		if( lpItemInfo->CODE==(sinOR2|sin01) && lpItemInfo->ItemKindCode==ITEM_KIND_AGING ) {
			if ( lpItemInfo->ItemAgingNum[0]>=Penalty ) Penalty=lpItemInfo->ItemAgingNum[0];
		}
	}

	if(sInven[5].ItemIndex){
		lpItemInfo = &cInvenTory.InvenItem[sInven[5].ItemIndex-1].sItemInfo;
		if( lpItemInfo->CODE==(sinOR2|sin01) && lpItemInfo->ItemKindCode==ITEM_KIND_AGING ) {
			if ( lpItemInfo->ItemAgingNum[0]>=Penalty ) Penalty=lpItemInfo->ItemAgingNum[0];
		}
	}

	//공성필드 경험치 하락 없음
	if ( lpCurPlayer->OnStageField>=0 && StageField[lpCurPlayer->OnStageField]->State==FIELD_STATE_CASTLE &&
		StageField[lpCurPlayer->OnStageField]->FieldCode==rsCASTLE_FIELD )
	{
		return 0;
	}

	//부활 주문서
	lpItemInfo = FindItemFromCode(sinBI1|sin04);
	if ( lpItemInfo ) {
		SendUseItemCodeToServer( lpItemInfo );
		cInvenTory.DeleteInvenItemToServer(lpItemInfo->CODE  , lpItemInfo->ItemHeader.Head , lpItemInfo->ItemHeader.dwChkSum );
		PlayUsed_Resurrection ++;
		return 0;								
	}

#ifndef _DEATH_PANALTY
	if ( sBiInfo && (sBiInfo->EventPlay[0]&BIMASK_VIP_USER)!=0 ) return 100;			//활인 VIP PC방 IP
#endif

	//태국수정
#ifdef _LANGUAGE_THAI_RING
	if ( Penalty>=8 ) return 1000;														//지존반지
	if ( Penalty>=7 ) return 700;
	if ( Penalty>=6 ) return 600;
	if ( Penalty>=5 ) return 500;														//지존반지
	if ( Penalty>=4 ) return 400;
	if ( Penalty>=3 ) return 300;														//지존반지
	if ( Penalty>=2 ) return 200;
	if ( Penalty>=1 ) return 100;
#endif
	if ( Penalty>=8 ) return 100;														//지존반지
	if ( Penalty>=6 ) return 50;

	if ( (sinQuest_levelLog&QUESTBIT_LEVEL_85)!=0 && lpCurPlayer->smCharInfo.Level>=85 ) {
		//90레벨 퀘스트 완료
#ifdef _DEATH_PANALTY //데쓰페널티 이벤트시 0.5%
	return 200;
#endif
		return 50;
	}
#ifdef _DEATH_PANALTY //데쓰페널티 이벤트시 1.0%
	return 110;
#endif


	return 33;
}


INT64 ExpLevelTable[] = {
#include "LevelTable.h"
	-1 };

//int LevelTableSum = 1793134039;
//INT64 LevelTableSum = -2044845887;
//INT64 LevelTableSum = 23422506775620;
//INT64 LevelTableSum = 1025409293731028;	// levels 1..120
INT64 LevelTableSum = 506871725618431790;	// levels 1..200 (LevelTable.h)


//경험치 데이타를 반전
INT64 CodeXorCharInfo_Exp()
{
	INT64 exp1,exp2;

	lpCurPlayer->smCharInfo.Exp ^= CodeXorExp;
	lpCurPlayer->smCharInfo.Exp_High ^= CodeXorExp_High;

	exp1 = lpCurPlayer->smCharInfo.Exp;
	exp2 = lpCurPlayer->smCharInfo.Exp_High;

	return (exp2<<32)|(exp1&0xFFFFFFFF);
}


//경험치 테이블 첵크
INT64 CheckLevelTable()
{
	INT64 cnt = 0;
	INT64 ChkSum = 0;

	while(1) {
		if (ExpLevelTable[cnt]==-1) break;
		ChkSum += ExpLevelTable[cnt]*(cnt+1);
		cnt++;
	}

	return ChkSum;
}

//레벨과 경험치가 맞는지 확인
int	CheckLevelExp( int Level , INT64 Exp )
{
	INT64 xp;
	INT64 np;

	if ( Level>0 && Level<=CHAR_LEVEL_MAX ) {
		xp = ExpLevelTable[Level-1];
		np = ExpLevelTable[Level];

		if ( Exp>=xp && Exp<=np ) {
			return TRUE;
		}

	}
	return FALSE;
}

//경험치로 레벨 추산
int	GetLevelFromExp( INT64 Exp )
{
	int	cnt;

	for(cnt=0;cnt<CHAR_LEVEL_MAX;cnt++) {
		if ( Exp>=ExpLevelTable[cnt] && Exp<ExpLevelTable[cnt+1] ) {
			return cnt+1;
		}
	}

	return NULL;
}

//64비트 경험치 반환
INT64 GetExp64( smCHAR_INFO *smCharInfo )
{
	INT64 exp1,exp2;
	exp1 = (INT64)smCharInfo->Exp;

#ifdef _MODE_EXP64
	exp2 = (INT64)smCharInfo->Exp_High;
#else
	exp2 = 0;
#endif

	return (exp2<<32)|(exp1&0xFFFFFFFF);
}

//64비트 경험치 반환
INT64 GetXorExp64( smCHAR_INFO *smCharInfo )
{
	INT64 exp1,exp2;

	exp1 = (INT64)(smCharInfo->Exp^CodeXorExp);
	exp2 = (INT64)(smCharInfo->Exp_High^CodeXorExp_High);

	return (exp2<<32)|(exp1&0xFFFFFFFF);
}


//64비트 경험치 설정
int	SetExp64( smCHAR_INFO *smCharInfo , INT64 Exp64 )
{
#ifdef _MODE_EXP64
	smCharInfo->Exp_High = (int)(Exp64>>32);
#endif
	smCharInfo->Exp = (int)(Exp64&0xFFFFFFFF);

	return smCharInfo->Exp;
}


//경험치 추가
int AddExp( int Exp )
{
	int Level;
	INT64 iexp;
	INT64 ChkSum;
	INT64 exp64;
	int mlevel;

	//경험치 데이타를 반전
	exp64 = CodeXorCharInfo_Exp();
/*
#ifndef	_LANGUAGE_KOREAN
	//해와판 레벨 제한 (79까지)
	if ( exp64<1878000000 )	exp64 += Exp;
#else
*/
		mlevel = CHAR_LEVEL_MAX-2;		//98

	if ( exp64<ExpLevelTable[mlevel] ) exp64 += Exp;

	//나중에 퀘스트 추가 ( 퀘스트를 수행하지 못했을 경우 경험치 증가 못함 )
	if ( exp64>ExpLevelTable[99] && lpCurPlayer->smCharInfo.ChangeJob<3 ) {
		CodeXorCharInfo_Exp();
		return TRUE;
	}


	//64비트 경험치 설정
	SetExp64( &lpCurPlayer->smCharInfo , exp64 );		//lpCurPlayer->smCharInfo.Exp += Exp;

	CodeXorCharInfo_Exp();

	//캐릭터 정보 인증 받기
	ReformCharForm();

	Level = lpCurPlayer->smCharInfo.Level;
	iexp = ExpLevelTable[Level];

	if ( iexp>=0 && exp64>=iexp && lpCurPlayer->smCharInfo.Level<CHAR_LEVEL_MAX ) {
		//레벨 상승
		ChkSum = CheckLevelTable();				//(첵크섬 오류 , 레벨테이블 이상)
		if (LevelTableSum!=ChkSum) return TRUE;

		lpCurPlayer->smCharInfo.Level++;
		lpCurPlayer->smCharInfo.Next_Exp = (int)ExpLevelTable[Level+1];
		StartEffect( lpCurPlayer->pX,lpCurPlayer->pY+32*fONE,lpCurPlayer->pZ , EFFECT_LEVELUP1 );
		esPlaySound( 7, 400 );
		//캐릭터 정보 인증 받기
		ReformCharForm();

		//서버에 변경된 캐릭터 정보 보냄 ( 레벨 변경 )
		SendPlayUpdateInfo();
		SaveGameData();			//저장
	}

	return TRUE;
}

//필드 재시작 가능한지 확인
int CheckFieldContExp()
{
	int Level;
	INT64 LowExp;
	INT64 HighExp;
	INT64 exp;
	INT64 exp64;

	Level = lpCurPlayer->smCharInfo.Level;
	if ( Level>=11 ) {
		LowExp = ExpLevelTable[Level-1];
		HighExp = ExpLevelTable[Level];
		exp = (HighExp - LowExp)/100;			//약 1%

		exp64 = GetXorExp64( &lpCurPlayer->smCharInfo );

		exp = exp64 - exp;
		if ( exp<LowExp ) {
			return FALSE;
		}
	}
	else {
		if ( lpCurPlayer->OnStageField>=0 && StageField[lpCurPlayer->OnStageField]->State!=FIELD_STATE_FOREST ) {
			//저랩은 숲에서만 재시작 가능
			return FALSE;
		}
	}

	return TRUE;
}


//다음 경험치 구하기
INT64 GetNextExp( int Level )
{
	return ExpLevelTable[Level];
}

//죽어서 경험치 깍임
int DeadPlayerExp( smCHAR *lpChar )
{
	int Level;
	INT64 LowExp;
	INT64 HighExp;
	INT64 exp;
	INT64 ep;
	INT64 exp64;
	int Money;
	int ExpLower = 0;
	int	Penalty = GetDeathPenalty();

	Level = lpChar->smCharInfo.Level;
	if ( Level>=11 ) {
		LowExp = ExpLevelTable[Level-1];
		HighExp = ExpLevelTable[Level];
		if ( Penalty )
			exp = (HighExp - LowExp)/Penalty;			//약 3%
		else
			exp = 0;

		exp64 = CodeXorCharInfo_Exp();
		if ( exp64>=LowExp ) {
			ep = exp64;
			exp64 -= exp;
			if ( exp64<LowExp ) {
				exp64=LowExp;
				ExpLower = TRUE;
			}
			SetExp64( &lpCurPlayer->smCharInfo , exp64 );
			ep = ep-exp64;
			SetTotalSubExp( (int)ep );		//경험치 증감 기록
		}
		CodeXorCharInfo_Exp();				//경험치 코드반전

		//Money = lpChar->smCharInfo.Money/25;	//돈 4% 없어짐
		if ( Penalty )
			Money = lpChar->smCharInfo.Money/Penalty;	//돈 3% 없어짐
		else
			Money = 0;

		lpChar->smCharInfo.Money -= Money;
		if ( lpChar->smCharInfo.Money<0 ) {
			Money = -Money;
			lpChar->smCharInfo.Money = 0;
		}
		SetTotalSubMoney( Money );	//돈 감소 기록

		//캐릭터 정보 인증 받기
		ReformCharForm();
		ResetInvenMoney();		//이벤토리 돈 검사 초기화
	}

	return ExpLower;
}

//필드에서 재시작 캐릭터 경험치 추가 깍임
int ContGameExp( smCHAR *lpChar )
{
	int Level;
	INT64 LowExp;
	INT64 HighExp;
	INT64 exp;
	INT64 ep;
	INT64 exp64;
	int Money;

	Level = lpChar->smCharInfo.Level;
	if ( Level>=11 ) {
		LowExp = ExpLevelTable[Level-1];
		HighExp = ExpLevelTable[Level];
		exp = (HighExp - LowExp)/100;			//약 1%

		//lpChar->smCharInfo.Exp ^= CodeXorExp;		//경험치 코드반전
		exp64 = CodeXorCharInfo_Exp();

		if ( exp64>LowExp ) {
			ep = exp64;
			exp64 -= exp;
			if ( exp64<LowExp ) 
				exp64=LowExp;

			SetExp64( &lpCurPlayer->smCharInfo , exp64 );

			ep = ep - exp64;
			SetTotalSubExp( (int)ep );		//경험치 증감 기록
		}

		CodeXorCharInfo_Exp();
		//lpChar->smCharInfo.Exp ^= CodeXorExp;		//경험치 코드반전

		Money = lpChar->smCharInfo.Money/100;	//돈 1% 없어짐
		lpChar->smCharInfo.Money -= Money;
		if ( lpChar->smCharInfo.Money<0 ) {
			Money = -Money;
			lpChar->smCharInfo.Money = 0;
		}

		SetTotalSubMoney( Money );	//돈 감소 기록

		//캐릭터 정보 인증 받기
		ReformCharForm();
		ResetInvenMoney();		//이벤토리 돈 검사 초기화
	}

	return TRUE;
}



//방어율 계산
int GetPlayDefence( smCHAR_INFO *lpAttackCharInfo , smCHAR_INFO *lpDefenceCharInfo )
{
	int defence;

	float ar,def,dLv,aLv;
	int weight;

	//방어율 계산
	ar = (float)lpAttackCharInfo->Attack_Rating;
	aLv = (float)lpAttackCharInfo->Level;
	def = (float)lpDefenceCharInfo->Defence;
	dLv = (float)lpDefenceCharInfo->Level;
	weight = lpDefenceCharInfo->Weight[0];
	defence = (int) (((((ar/(ar+(def/2))) - ((dLv-aLv)/50)))  * 100)) - (weight/10000) ;

	if ( defence>95 ) defence=95;

	return defence;
}


int RainMaterial[4][8];


int InitWeatherSystem()
{
	if ( smConfig.TextureQuality>=2 ) return FALSE;

	RainMaterial[0][0] =  CreateTextureMaterial( "rain\\ll.bmp" , 0, TRUE ,FALSE , FALSE , SMMAT_BLEND_ADDCOLOR );

	RainMaterial[1][0] =  CreateTextureMaterial( "rain\\45rain01.tga" ,0 , TRUE ,FALSE , FALSE , SMMAT_BLEND_ALPHA );
	RainMaterial[1][1] =  CreateTextureMaterial( "rain\\45rain02.tga" ,0 , TRUE ,FALSE , FALSE , SMMAT_BLEND_ALPHA );
	RainMaterial[1][2] =  CreateTextureMaterial( "rain\\45rain03.tga" ,0 , TRUE ,FALSE , FALSE , SMMAT_BLEND_ALPHA );
	RainMaterial[1][3] =  CreateTextureMaterial( "rain\\45rain04.tga" ,0 , TRUE ,FALSE , FALSE , SMMAT_BLEND_ALPHA );
	RainMaterial[1][4] =  CreateTextureMaterial( "rain\\45rain05.tga" ,0 , TRUE ,FALSE , FALSE , SMMAT_BLEND_ALPHA );
	RainMaterial[1][5] =  CreateTextureMaterial( "rain\\45rain06.tga" ,0 , TRUE ,FALSE , FALSE , SMMAT_BLEND_ALPHA );

	RainMaterial[2][0] =  CreateTextureMaterial( "rain\\80rain01.tga" ,0 , TRUE ,FALSE , FALSE , SMMAT_BLEND_ALPHA );
	RainMaterial[2][1] =  CreateTextureMaterial( "rain\\80rain02.tga" ,0 , TRUE ,FALSE , FALSE , SMMAT_BLEND_ALPHA );
	RainMaterial[2][2] =  CreateTextureMaterial( "rain\\80rain03.tga" ,0 , TRUE ,FALSE , FALSE , SMMAT_BLEND_ALPHA );
	RainMaterial[2][3] =  CreateTextureMaterial( "rain\\80rain04.tga" ,0 , TRUE ,FALSE , FALSE , SMMAT_BLEND_ALPHA );
	RainMaterial[2][4] =  CreateTextureMaterial( "rain\\80rain05.tga" ,0 , TRUE ,FALSE , FALSE , SMMAT_BLEND_ALPHA );
	RainMaterial[2][5] =  CreateTextureMaterial( "rain\\80rain06.tga" ,0 , TRUE ,FALSE , FALSE , SMMAT_BLEND_ALPHA );

	RainMaterial[3][0] =  CreateTextureMaterial( "rain\\90rain01.tga" ,0 , TRUE ,FALSE , FALSE , SMMAT_BLEND_ALPHA );
	RainMaterial[3][1] =  CreateTextureMaterial( "rain\\90rain02.tga" ,0 , TRUE ,FALSE , FALSE , SMMAT_BLEND_ALPHA );
	RainMaterial[3][2] =  CreateTextureMaterial( "rain\\90rain03.tga" ,0 , TRUE ,FALSE , FALSE , SMMAT_BLEND_ALPHA );
	RainMaterial[3][3] =  CreateTextureMaterial( "rain\\90rain04.tga" ,0 , TRUE ,FALSE , FALSE , SMMAT_BLEND_ALPHA );
	RainMaterial[3][4] =  CreateTextureMaterial( "rain\\90rain05.tga" ,0 , TRUE ,FALSE , FALSE , SMMAT_BLEND_ALPHA );
	RainMaterial[3][5] =  CreateTextureMaterial( "rain\\90rain06.tga" ,0 , TRUE ,FALSE , FALSE , SMMAT_BLEND_ALPHA );

	return TRUE;
}


extern int WeatherMode;

int DrawWeather( int ax )
{
/*
	if ( WeatherMode==2 ) {
		if ( Thunder==0 && (Draw2DCounter&0x1FF)==0x80 ) {
			Thunder = timeGetTime();
			if ( ((Thunder>>4)&1 )==0 ) {
				PlayWav2( 2 , 0 , 400 , 2205 );
			}
			else 
				Thunder=0;
		}
	}
*/

	if ( smConfig.TextureQuality>=2 ) return FALSE;

	smFACE2D face2d;

	int tm;
	float fx;
	int RainMode;

	if ( ax<ANGLE_45-256 ) RainMode = 0;
	else {
		if ( ax>ANGLE_45+256 ) RainMode = 3;
		else {
			if ( ax>ANGLE_45 ) RainMode = 2;
			else RainMode = 1;
		}
	}

	if ( WeatherMode==2 ) {

		if ( lpCurPlayer->OnStageField>=0 && 
			( StageField[lpCurPlayer->OnStageField]->State==FIELD_STATE_DUNGEON ||
			StageField[lpCurPlayer->OnStageField]->State==FIELD_STATE_ROOM ) )
			return TRUE;
//	if ( WeatherMode<0 ) {
//	if ( WeatherMode>=0 ) {
		//비오는 날씨 그림
/*
	if ( Thunder ) {
		tm = timeGetTime();
		if ( (Thunder+1000)<tm ) Thunder = 0;

		tm = ( tm>>1 )&0x7F;
		smRender.Color_R = tm;
		smRender.Color_G = tm;
		smRender.Color_B = tm;

	}
	else {
		smRender.Color_R = -32;
		smRender.Color_G = -32;
		smRender.Color_B = -32;
	}
*/


	smRender.Init();

	switch( RainMode ) {
	case 0:
		tm = timeGetTime();
		tm = ( tm>>2 )&0x3FF;
		fx = ((float)tm)/256;
/*
		face2d.TexRect.left = fx;
		face2d.TexRect.right = fx+4;
		face2d.TexRect.top = fx;
		face2d.TexRect.bottom = fx+0.8;
		face2d.width  = WinSizeX;
		face2d.height = WinSizeY;
		face2d.Transparency = 255;
		face2d.r = 255; face2d.g = 255; face2d.b = 255;
		face2d.x = 0;
		face2d.z = 64*6;
		face2d.y = 0;
		face2d.MatNum = RainMaterial[RainMode][0];
		smRender.AddFace2D_Direct ( &face2d );
*/
		tm = timeGetTime();
		tm = ( tm )&0x3FF;
		fx = ((float)tm)/256;

		face2d.TexRect.left = fx;
		face2d.TexRect.right = fx+3;
		face2d.TexRect.top = fx;
		face2d.TexRect.bottom = fx+0.5f;
		face2d.width  = WinSizeX;
		face2d.height = WinSizeY;
		face2d.Transparency = 255;
		face2d.r = 255; face2d.g = 255; face2d.b = 255;
		face2d.x = 0;
		face2d.z = 0;
		face2d.y = 0;
		face2d.MatNum = RainMaterial[RainMode][0];
		smRender.AddFace2D_Direct ( &face2d );

		break;

	case 1:
		tm = timeGetTime()>>6;

		face2d.TexRect.left = 1;
		face2d.TexRect.right = 0;
		face2d.TexRect.top = 0;
		face2d.TexRect.bottom = 1;
		face2d.width  = WinSizeX/2;
		face2d.height = WinSizeY;
		face2d.Transparency = 160;
		face2d.r = 255; face2d.g = 255; face2d.b = 255;
		face2d.x = 0;
		face2d.z = 0;
		face2d.y = 0;
		face2d.MatNum = RainMaterial[RainMode][tm%6];
		smRender.AddFace2D_Direct ( &face2d );

		face2d.TexRect.left = 0;
		face2d.TexRect.right = 1;
		face2d.TexRect.top = 0;
		face2d.TexRect.bottom = 1;
		face2d.width  = WinSizeX/2;
		face2d.height = WinSizeY;
		face2d.Transparency = 160;
		face2d.r = 255; face2d.g = 255; face2d.b = 255;
		face2d.x = WinSizeX/2;
		face2d.z = 0;
		face2d.y = 0;
		face2d.MatNum = RainMaterial[RainMode][(tm+3)%6];
		smRender.AddFace2D_Direct ( &face2d );
		break;

	case 2:

		tm = timeGetTime()>>6;

		face2d.TexRect.left = 1;
		face2d.TexRect.right = 0;
		face2d.TexRect.top = 0;
		face2d.TexRect.bottom = 1;
		face2d.width  = WinSizeX/2;
		face2d.height = WinSizeY;
		face2d.Transparency = 128;
		face2d.r = 255; face2d.g = 255; face2d.b = 255;
		face2d.x = 0;
		face2d.z = 0;
		face2d.y = 0;
		face2d.MatNum = RainMaterial[RainMode][tm%6];
		smRender.AddFace2D_Direct ( &face2d );

		face2d.TexRect.left = 0;
		face2d.TexRect.right = 1;
		face2d.TexRect.top = 0;
		face2d.TexRect.bottom = 1;
		face2d.width  = WinSizeX/2;
		face2d.height = WinSizeY;
		face2d.Transparency = 128;
		face2d.r = 255; face2d.g = 255; face2d.b = 255;
		face2d.x = WinSizeX/2;
		face2d.z = 0;
		face2d.y = 0;
		face2d.MatNum = RainMaterial[RainMode][(tm+3)%6];
		smRender.AddFace2D_Direct ( &face2d );
		break;

	case 3:
		tm = timeGetTime()>>6;

		face2d.TexRect.left = 0;
		face2d.TexRect.right = 1;
		face2d.TexRect.top = 0;
		face2d.TexRect.bottom = 1;
		face2d.width  = WinSizeX/2;
		face2d.height = WinSizeY/2;
		face2d.Transparency = 128;
		face2d.r = 255; face2d.g = 255; face2d.b = 255;
		face2d.x = 0;
		face2d.z = 0;
		face2d.y = 0;
		face2d.MatNum = RainMaterial[RainMode][tm%6];
		smRender.AddFace2D_Direct ( &face2d );

		face2d.TexRect.left = 1;
		face2d.TexRect.right = 0;
		face2d.TexRect.top = 0;
		face2d.TexRect.bottom = 1;
		face2d.width  = WinSizeX/2;
		face2d.height = WinSizeY/2;
		face2d.Transparency = 128;
		face2d.r = 255; face2d.g = 255; face2d.b = 255;
		face2d.x = WinSizeX/2;
		face2d.z = 0;
		face2d.y = 0;
		face2d.MatNum = RainMaterial[RainMode][(tm+3)%6];
		smRender.AddFace2D_Direct ( &face2d );

		/////////////////////////////////////////////////
		face2d.TexRect.left = 0;
		face2d.TexRect.right = 1;
		face2d.TexRect.top = 1;
		face2d.TexRect.bottom = 0;
		face2d.width  = WinSizeX/2;
		face2d.height = WinSizeY/2;
		face2d.Transparency = 128;
		face2d.r = 255; face2d.g = 255; face2d.b = 255;
		face2d.x = 0;
		face2d.z = 0;
		face2d.y = WinSizeY>>1;
		face2d.MatNum = RainMaterial[RainMode][(tm+1)%6];
		smRender.AddFace2D_Direct ( &face2d );

		face2d.TexRect.left = 1;
		face2d.TexRect.right = 0;
		face2d.TexRect.top = 1;
		face2d.TexRect.bottom = 0;
		face2d.width  = WinSizeX/2;
		face2d.height = WinSizeY/2;
		face2d.Transparency = 128;
		face2d.r = 255; face2d.g = 255; face2d.b = 255;
		face2d.x = WinSizeX/2;
		face2d.z = 0;
		face2d.y = WinSizeY>>1;
		face2d.MatNum = RainMaterial[RainMode][(tm+4)%6];
		smRender.AddFace2D_Direct ( &face2d );

	}

		smRender.SetClipStates( 0 );
		smRender.ClipRendFace();							// 전체 렌더링 페이스를 클리핑
		smRender.RenderD3D();
	}

	return TRUE;
}



HANDLE	hLoadingThread = 0;
HBITMAP hBitmapLamp = 0;
HDC	hdcBitmapLamp = 0;
HBITMAP hBitmapLoadImage = 0;
HBITMAP hBitmapLoadImage2 = 0;
HDC	hdcBitmapLoadImage = 0;

int DispLoading;

DWORD WINAPI DispLoadingThreadProc( void *pInfo )
{
	HANDLE	hThread;
	HDC	hdc;
	int Counter = 0;
	char *szLoading[5] = {
		"   -Loading-   ",
		"  - Loading -  ",
		" -  Loading  - ",
		"-   Loading   -",
		"----Loading----"
	};
	int	DispLoadingCount = 0;

    RECT                destRect;
	POINT				pt;

	DispLoading = 0;
	hThread = GetCurrentThread();

	while(1) {
		if ( quit ) break;

		if ( !DispLoading ) {
			SuspendThread(hThread);
			DispLoadingCount++;
		}

		//크리티칼 섹션 선언
		EnterCriticalSection( &cDrawSection );

		GetClientRect(hwnd,&destRect);

		pt.x=pt.y=0;
	    ClientToScreen(hwnd,&pt);
	    OffsetRect(&destRect,pt.x,pt.y);

		lpDDSPrimary->GetDC( &hdc );
/*
		if ( (Counter&1)==0 )
			BitBlt( hdc , 395, 544, 10, 9 , hdcBitmapLamp , 0,0 ,SRCCOPY );
		else
			BitBlt( hdc , 395, 544, 10, 9 , hdcBitmapLamp , 10,0 ,SRCCOPY );
*/
		if ( DispLoadingCount<2 ) {
			if ( (Counter&1)==0 )
				BitBlt( hdc , 362+(WinSizeX-800)/2, 520+(WinSizeY-600)/2, 73, 20 , hdcBitmapLoadImage , 0,0 ,SRCCOPY );
			else
				BitBlt( hdc , 362+(WinSizeX-800)/2, 520+(WinSizeY-600)/2, 73, 20 , hdcBitmapLoadImage , 0,20 ,SRCCOPY );
		}
		else {
			if ( (Counter&1)==0 )
				BitBlt( hdc , 362, 520+(WinSizeY-600), 73, 20 , hdcBitmapLoadImage , 0,0 ,SRCCOPY );
			else
				BitBlt( hdc , 362, 520+(WinSizeY-600), 73, 20 , hdcBitmapLoadImage , 0,20 ,SRCCOPY );
		}



/*

		SelectObject( hdc , hFont );

		SetBkMode( hdc, TRANSPARENT );
		SetTextColor( hdc, RGB( 0,0,0 ) );
		dsTextLineOut( hdc , destRect.left + 350,528, szLoading[4] , lstrlen( szLoading[4] ) );
		SetTextColor( hdc, RGB(255, 255, 255) );
		dsTextLineOut( hdc , destRect.left + 350,528, szLoading[Counter&3] , lstrlen( szLoading[Counter&3] ) );
*/
		lpDDSPrimary->ReleaseDC( hdc );
		//크리티칼 섹션 해제
		LeaveCriticalSection( &cDrawSection );

		Counter++;
		Sleep( 250 );
	}

	ExitThread( TRUE );
	return TRUE;
}


//로딩램프
int InitLoadingLamp( HWND hWnd )
{
	HDC hdc;

	hdc = GetDC( hWnd );
	// 말 상자 그림 로드 
	hBitmapLamp = LoadBitmap( hinst, "IDB_LOADINGLAMP" );
	hdcBitmapLamp = CreateCompatibleDC(hdc);
	SelectObject( hdcBitmapLamp, hBitmapLamp );

	hBitmapLoadImage = LoadBitmap( hinst, "IDB_LOADIMG" );
	hdcBitmapLoadImage = CreateCompatibleDC(hdc);
	SelectObject( hdcBitmapLoadImage, hBitmapLoadImage );

	ReleaseDC( hWnd ,hdc );

	return TRUE;
}
//HBITMAP hBitmapLoadImage = 0;
//HDC	hdcBitmapLoadImage = 0;

//로딩 램프 제거
int CloseLoadingLamp()
{
	if ( hdcBitmapLoadImage ) DeleteDC( hdcBitmapLoadImage );
	if ( hBitmapLoadImage ) DeleteObject( hBitmapLoadImage );

	if ( hdcBitmapLamp ) DeleteDC( hdcBitmapLamp );
	if ( hBitmapLamp ) DeleteObject( hBitmapLamp );

	hdcBitmapLamp=0;hBitmapLamp=0;
	hdcBitmapLoadImage=0;hBitmapLoadImage=0;

	return TRUE;
}

DWORD dwLampThreadId;
DWORD dwCurrentThreadId;

//로딩중 화면 표시
int OpenDispLoading()
{

	if ( !lpDDSPrimary ) return FALSE;

	if ( !hLoadingThread ) {
		hLoadingThread = CreateThread( NULL , 0, DispLoadingThreadProc , 0 , 0, &dwLampThreadId );
		dwCurrentThreadId = GetCurrentThreadId();
	}
	else {
		DispLoading = TRUE;
		ResumeThread( hLoadingThread );
	}

	return TRUE;
}
//로딩 표시종료
int CloseDispLoading()
{

	DispLoading = FALSE;
	return TRUE;
}



#define DYN_REMOVER_MAX		64
#define DYN_REMOVER_MASK	(DYN_REMOVER_MAX-1)

smDPAT *DynRemoveList[DYN_REMOVER_MAX];
int DynRemoveCnt = 0;
int DynRemovePush = 0;

HANDLE	hRemoverThread=0;

DWORD WINAPI RemoveDynaPatternThreadProc( void *pInfo )
{
	HANDLE	hThread;
//	HDC	hdc;
	int Counter = 0;
//	int cnt;

	int RemoveCnt;

	hThread = GetCurrentThread();

	while(1) {
		if ( quit ) break;

		if ( (DynRemovePush-DynRemoveCnt)>32 ) {
			//크리티칼 섹션 선언
			EnterCriticalSection( &cLoadSection );
			EnterCriticalSection( &cDrawSection );

			while(1) {
				RemoveCnt = DynRemoveCnt&DYN_REMOVER_MASK;
				smDPAT_Delete( DynRemoveList[RemoveCnt] );
				DynRemoveCnt++;
				if ( DynRemovePush<=DynRemoveCnt ) break;
			}

			//크리티칼 섹션 해제
			LeaveCriticalSection( &cDrawSection );
			LeaveCriticalSection( &cLoadSection );
		}
		else
			SuspendThread(hThread);

		Counter++;
	}

	ExitThread( TRUE );
	return TRUE;
}

int RemoveDynPatSub()
{
	int RemoveCnt;

	while(1) {
		if ( (DynRemovePush-DynRemoveCnt)>32 ) {
			while(1) {
				RemoveCnt = DynRemoveCnt&DYN_REMOVER_MASK;
				smDPAT_Delete( DynRemoveList[RemoveCnt] );
				DynRemoveCnt++;
				if ( DynRemovePush<=DynRemoveCnt ) break;
			}
		}
		else
			break;
	}

	return TRUE;
}


//나이나믹 패턴 삭제 추가
int AddRemoveDynPattern( smDPAT *lpDinaPattern )
{

	//DWORD dwMsgId;

	if ( ServerMode ) {
		if ( lpDinaPattern->UseCount>1 )
			smDPAT_Delete( lpDinaPattern );

		return TRUE;
	}
/*
	if ( !hRemoverThread ) 
		hRemoverThread = CreateThread( NULL , 0, RemoveDynaPatternThreadProc , 0 , 0, &dwMsgId );
	else {
		DynRemoveList[DynRemovePush&DYN_REMOVER_MASK] = lpDinaPattern;
		DynRemovePush++;
		ResumeThread( hRemoverThread );
	}
*/

	DynRemoveList[DynRemovePush&DYN_REMOVER_MASK] = lpDinaPattern;
	DynRemovePush++;
	RemoveDynPatSub();

	return DynRemovePush;
}


//		smDPAT_Delete( lpDinaPattern2 );
//		lpDinaPattern2 = 0;

struct	sCOMPACT_MAP {
	smTEXTUREHANDLE	*hTexHandle;
	smTEXTUREHANDLE	*hTexTitleHandle;
	RECT	StageRect;
	sFIELD	*lpField;
	int		rx,ry;

};

sCOMPACT_MAP sCompactMap[4];

int MatArrow;
int MatMapBox;
int	MatPartyPos;
int	MatNpcPos;

//필드 맵 초기화
int InitFieldMap()
{
	ZeroMemory( sCompactMap , sizeof(sCOMPACT_MAP) * 4 );
	MatArrow = CreateTextureMaterial( "Image\\ARROW.tga" , 0, 0, 0,0, SMMAT_BLEND_ALPHA );
	MatMapBox = CreateTextureMaterial( "Image\\MapBox.tga" , 0, 0, 0,0, SMMAT_BLEND_ALPHA );
	MatPartyPos = CreateTextureMaterial( "Image\\party.tga" , 0, 0, 0,0, SMMAT_BLEND_ALPHA );
	MatNpcPos = CreateTextureMaterial( "Image\\npc.tga" , 0, 0, 0,0, SMMAT_BLEND_ALPHA );

	return TRUE;
}

//필드맵 로딩
int LoadFieldMap( int PosNum , sFIELD *lpField , smSTAGE3D *lpStage )
{
	int ReadTexFlag = 0;

	if ( !lpField || !lpStage || sCompactMap[PosNum].lpField==lpField ) 
		return TRUE;

	sCompactMap[PosNum].lpField = lpField;

	memcpy( &sCompactMap[PosNum].StageRect , &lpStage->StageMapRect , sizeof( RECT ) );

	if (  sCompactMap[PosNum].hTexHandle ) {
		smTexture.Delete( sCompactMap[PosNum].hTexHandle );
		sCompactMap[PosNum].hTexHandle = 0;
	}
	if (  sCompactMap[PosNum].hTexTitleHandle ) {
		smTexture.Delete( sCompactMap[PosNum].hTexTitleHandle );
		sCompactMap[PosNum].hTexTitleHandle = 0;
	}


	if ( lpField->szNameMap[0] ) {
		sCompactMap[PosNum].hTexHandle = smTexture.Add( lpField->szNameMap, 0 );
		sCompactMap[PosNum].hTexHandle->TexSwapMode = FALSE;

		sCompactMap[PosNum].rx = (lpStage->StageMapRect.right - lpStage->StageMapRect.left)>>FLOATNS;
		sCompactMap[PosNum].ry = (lpStage->StageMapRect.bottom - lpStage->StageMapRect.top)>>FLOATNS;

		ReadTexFlag = TRUE;
	}
	else {
		sCompactMap[PosNum].hTexHandle = 0;
		return FALSE;
	}

	if ( lpField->szNameTitle[0] ) {
		sCompactMap[PosNum].hTexTitleHandle = smTexture.Add( lpField->szNameTitle, 0 );
		sCompactMap[PosNum].hTexTitleHandle->TexSwapMode = FALSE;

		ReadTexFlag = TRUE;
	}
	else {
		sCompactMap[PosNum].hTexTitleHandle = 0;
		return FALSE;
	}

	if ( ReadTexFlag )	ReadTextures();

	return TRUE;
}
//그림 출력
int psDrawTexImage( smTEXTUREHANDLE *hTexHandle , float x, float y, float w, float h ,
				   float fx, float fy, float fw, float fh, int Transp , int specular);//그림 출력

int psDrawTexImage_Point( smTEXTUREHANDLE *hTexHandle , float x, float y, float w, float h ,
				   float fx, float fy, float fw, float fh, int Transp , int specular);

int psDrawTexImage2( smTEXTUREHANDLE *hTexHandle , float *x, float *y, 
					float fx, float fy, float fw, float fh, int Transp , int specular);

int DrawMapArrow( int x, int y, int Angle );
int psSetDefaultRenderState();

//필드 맵 그리기
int DrawFieldMap( sCOMPACT_MAP *lpCompactMap , int px, int py , int size );

int FieldMapDrawMode = 2;

int FieldMapDrawSize[3] = { 8 , 16 , 24 };
int FieldMapViewSize1 = 18*64*fONE;
int FieldMapViewSize2 = 8*64*fONE;

//필드 맵 그리기
int MainFieldMap()
{
	if ( FieldMapViewSize1>FieldMapViewSize2 ) {
		FieldMapViewSize2 += 2048;
		if ( FieldMapViewSize1<FieldMapViewSize2 )
			FieldMapViewSize2 = FieldMapViewSize1;
	}

	if ( FieldMapViewSize1<FieldMapViewSize2 ) {
		FieldMapViewSize2 -= 2048;
		if ( FieldMapViewSize1>FieldMapViewSize2 )
			FieldMapViewSize2 = FieldMapViewSize1;
	}


	return TRUE;
}

extern int MatEachBox;
//미니맵에 파티 유저 표시
int	DrawMapPartyPlayer( int px, int py );
//미니맵에 NPC 표시
int	DrawMapNPC( int px, int py );

//필드미니맵의 마우스 위치 확인
int	GetFieldMapCursor()
{
	int px=656+(WinSizeX-800);
	int py=426+(WinSizeY-600);

	if ( WinSizeX<=800 && cSkill.OpenFlag ) return FALSE;

	if ( pCursorPos.x>px && pCursorPos.x<px+128 && pCursorPos.y>py && pCursorPos.y<py+128 && lpCurPlayer->dwTradeMsgCode==0 ) {

		if ( lpCurPlayer->OnStageField>=0 && 
			(StageField[lpCurPlayer->OnStageField]->FieldCode==rsSOD_FIELD || StageField[lpCurPlayer->OnStageField]->FieldCode==QUEST_ARENA_FIELD ) )  {

				return FALSE;
			}

		return TRUE;
	}

	return FALSE;
}

//필드 맵 그리기
int DrawFieldMap()
{
	int px=656+(WinSizeX-800);
	int py=426+(WinSizeY-600);
	int sz;
	int flCode;
	int	dSpe;
/*
	if ( VRKeyBuff['1'] ) FieldMapDrawMode=0;
	if ( VRKeyBuff['2'] ) FieldMapDrawMode=1;
	if ( VRKeyBuff['3'] ) FieldMapDrawMode=2;
*/
	if ( lpCurPlayer->dwTradeMsgCode )		//개인상점 오픈중
		return FALSE;

	if ( lpCurPlayer->OnStageField>=0 && 
		( StageField[lpCurPlayer->OnStageField]->FieldCode==rsSOD_FIELD || 
		StageField[lpCurPlayer->OnStageField]->FieldCode==QUEST_ARENA_FIELD ||
		StageField[lpCurPlayer->OnStageField]->State==FIELD_STATE_ACTION )  ||
				//작성자 : 하 대 용
		//보스존 36번 필드는 미니맵을 활성화 시키지 않는다.
		StageField[lpCurPlayer->OnStageField]->FieldCode == 36 ) 
	{
		return FALSE;
	}


	if ( lpCurPlayer->OnStageField>=0 && StageField[lpCurPlayer->OnStageField]->State==FIELD_STATE_DUNGEON ) {
		FieldMapDrawMode = 1;//0;
	}
	else {

		FieldMapDrawMode = 2;
	}



	lpD3DDevice->BeginScene();

	FieldMapViewSize1 = FieldMapDrawSize[FieldMapDrawMode]*64*fONE;
	sz = FieldMapViewSize2;

	psSetDefaultRenderState();

	//색깔있는 상자 그린다
	dsDrawColorBox( RGBA_MAKE(0,0,0,128) , px , py, 128,128 );

	//필드 맵 그리기
	DrawFieldMap( &sCompactMap[1], px+1, py+1 , sz );
	//필드 맵 그리기
	DrawFieldMap( &sCompactMap[0], px+1, py+1 , sz );

	//미니맵에 NPC 표시
	DrawMapNPC( px+64,py+64 );

	//파티원 위치 표시
	if ( InterfaceParty.PartyMemberCount )
		DrawMapPartyPlayer( px+64,py+64 );


	//화살표 그리기
	DrawMapArrow( px+64,py+64, lpCurPlayer->Angle.y );


	if ( GetFieldMapCursor()==TRUE )
		dSpe = 96;
	else
		dSpe = 0;

	if ( MatMapBox>=0 )	{
		dsDrawTexImage( MatMapBox , px,py,128,128 ,255, dSpe );
	}

	flCode = 0;
	if ( lpCurPlayer->OnStageField>=0 ) {
		if ( StageField[lpCurPlayer->OnStageField]==sCompactMap[0].lpField ) flCode = 0;
		if ( StageField[lpCurPlayer->OnStageField]==sCompactMap[1].lpField ) flCode = 1;
	}

	if ( sCompactMap[flCode].hTexTitleHandle ) {
		//dsDrawTexImage( sCompactMap[flCode].hTexTitleHandle  , px,py-16,128,16 ,255, 0 );
		psDrawTexImage_Point( sCompactMap[flCode].hTexTitleHandle , (float)px,(float)py-16,128,16 , 0,0,1.0f,1.0f,255, dSpe>>2 );
	}

	lpD3DDevice->EndScene();

	return TRUE;
}


//미니맵에 파티 유저 표시
int	DrawMapPartyPlayer( int px, int py )
{
	int cnt;
	int	x,y;
	int	size;
	float sz;
	float sx,sy;
	float fx,fy;
	int dist;

	size = FieldMapViewSize2;
	sz = (float)(FieldMapViewSize2/64);
	fx = (float)px;
	fy = (float)py;

	for(cnt=0;cnt<InterfaceParty.PartyMemberCount;cnt++) {
		if ( InterfaceParty.PartyMember[cnt].ChrCode!=lpCurPlayer->dwObjectSerial ) {
			x = InterfaceParty.PartyMember[cnt].CharacterPos.x-lpCurPlayer->pX;
			y = InterfaceParty.PartyMember[cnt].CharacterPos.z-lpCurPlayer->pZ;
			if ( abs(x)<size && abs(y)<size ) {
				sx = fx+(((float)x)/sz);
				sy = fy-(((float)y)/sz);

				x>>=FLOATNS;
				y>>=FLOATNS;
				dist = x*x+y*y;
				if ( dist<PARTY_GETTING_DIST2 )
					dsDrawTexImageFloat( MatPartyPos , sx-3,sy-3,8,8 ,255, 0 );
				else
					dsDrawTexImageFloat( MatPartyPos , sx-3,sy-3,8,8 ,RGBA_MAKE(255,0,0,255), 0 );

			}
		}
	}

	return TRUE;
}


//미니맵에 NPC 표시
int	DrawMapNPC( int px, int py )
{
	int cnt;
	int	x,y;
	int	size;
	float sz;
	float sx,sy;
	float fx,fy;
	int TransNum;

	size = FieldMapViewSize2;
	sz = (float)(FieldMapViewSize2/64);
	fx = (float)px;
	fy = (float)py;

	TransNum = 127;
/*
	TransNum = dwPlayTime>>3;
	if ( (TransNum&0x80)==0 ) {
		TransNum &= 0x7F;
	}
	else {
		TransNum = 127-(TransNum&0x7F);
	}
*/

	for(cnt=0;cnt<OTHER_PLAYER_MAX;cnt++) {
		if ( chrOtherPlayer[cnt].Flag && chrOtherPlayer[cnt].smCharInfo.State==smCHAR_STATE_NPC ) {
			x = chrOtherPlayer[cnt].pX-lpCurPlayer->pX;
			y = chrOtherPlayer[cnt].pZ-lpCurPlayer->pZ;
			if ( abs(x)<size && abs(y)<size ) {
				//화살표 그리기
				sx = fx+(((float)x)/sz);
				sy = fy-(((float)y)/sz);
				dsDrawTexImageFloat( MatNpcPos , sx-3,sy-3,8,8 ,TransNum+128, 0 );
			}
		}
	}

	return TRUE;
}




int DrawMapArrow( int x, int y, int Angle )
{
	float fx[4],fy[4];
	int sinY,cosY;
	int dx[4],dy[4];
	int cnt;

	sinY = GetSin[ Angle&ANGCLIP ];
	cosY = GetCos[ (Angle+ANGLE_180)&ANGCLIP ];

	dx[0] = -8; dy[0] = -8;
	dx[1] = -8; dy[1] =  8;
	dx[2] =  8; dy[2] = -8;
	dx[3] =  8; dy[3] =  8;


	for( cnt=0;cnt<4;cnt++) {
		fx[cnt] = (float) (( dy[cnt]*sinY + dx[cnt]*cosY )>>8);
		fy[cnt] = (float) (( dy[cnt]*cosY - dx[cnt]*sinY )>>8);
		fx[cnt] /= 256;
		fy[cnt] /= 256;
		fx[cnt] += (float)x;
		fy[cnt] += (float)y;
	}


	psDrawTexImage2( smMaterial[MatArrow].smTexture[0] , fx , fy, 0,0,1,1,255,0 );
//	psDrawTexImage2( 0 , fx , fy, 0,0,1,1,255,0 );


	return TRUE;
}

//필드 맵 그리기
int DrawFieldMap( sCOMPACT_MAP *lpCompactMap , int px, int py , int size )
{

	int x,z;
	int w,h;
	int left,right,bottom,top;
	float fx,fy,fw,fh;

	float dx,dy,dw,dh;
	float sc;

	if ( !lpCompactMap->lpField || !lpCompactMap->hTexHandle ) return FALSE;

	w = lpCompactMap->rx;
	h = lpCompactMap->ry;
	x = (lpCurPlayer->pX-lpCompactMap->StageRect.left);
//	z = (lpCurPlayer->pZ-lpCompactMap->StageRect.top);
	z = (lpCompactMap->StageRect.bottom-lpCurPlayer->pZ);

	left = x-size;
	right = x+size;
	top = z-size;
	bottom = z+size;
/*
	fx = ((float)(left/w))/fONE;
	fw = ((float)(right/w))/fONE;
	fy = ((float)(top/h))/fONE;
	fh = ((float)(bottom/h))/fONE;
*/
	dx = (float)px;
	dy = (float)py;
	dw = 126;
	dh = 126;

	float fpx;
	float fpy;
	float fmx,fmy;

	fpx = ((float)((size>>FLOATNS))) / (float)w ;
	fpy = ((float)((size>>FLOATNS))) / (float)h ;

	fmx = ((float)(x>>FLOATNS))/(float)w;
	fmy = ((float)(z>>FLOATNS))/(float)h;

	fx = fmx-fpx;
	fw = fmx+fpx;

	fy = fmy-fpy;
	fh = fmy+fpy;

	if ( (fx<0 && fw<0) || (fx>1 && fw>1 ) ) return FALSE;
	if ( (fy<0 && fh<0) || (fy>1 && fh>1 ) ) return FALSE;

	if ( fx<0 ) {
		sc = (-fx*(64/fpx));
		dx = dx+sc;
		dw = dw-sc;
		fx = 0;
	}
	if ( fw>1 ) {
		sc = ((fw-1)*(64/fpx));
		dw = dw-sc;
		fw = 1;
	}

	if ( fy<0 ) {
		sc = (-fy*(64/fpy));
		dy = dy+sc;
		dh = dh-sc;
		fy = 0;
	}
	if ( fh>1 ) {
		sc = ((fh-1)*(64/fpy));
		dh = dh-sc;
		fh = 1;
	}

	psDrawTexImage( lpCompactMap->hTexHandle , dx,dy,dw,dh , fx,fy,fw,fh,255, 0 );

	return TRUE;
}

int psSetDefaultRenderState()
{

		  // Turn on Z-buffering

		lpD3DDevice->SetRenderState( D3DRENDERSTATE_ZENABLE, TRUE );
		lpD3DDevice->SetRenderState( D3DRENDERSTATE_ZWRITEENABLE, TRUE );
		lpD3DDevice->SetRenderState( D3DRENDERSTATE_ZFUNC,   D3DCMP_LESSEQUAL );
		//	lpD3DDevice->SetRenderState( D3DRENDERSTATE_ZENABLE, D3DZB_USEW );//TRUE );


		// null out the texture handle
		lpD3DDevice->SetRenderState( D3DRENDERSTATE_TEXTUREHANDLE, 0 );

		// turn on dithering
		lpD3DDevice->SetRenderState( D3DRENDERSTATE_DITHERENABLE, TRUE );

		lpD3DDevice->SetRenderState( D3DRENDERSTATE_TEXTUREADDRESS, D3DTADDRESS_WRAP );
		lpD3DDevice->SetRenderState( D3DRENDERSTATE_TEXTUREPERSPECTIVE, TRUE ); 

		// D3DFILTER_LINEAR
		lpD3DDevice->SetRenderState( D3DRENDERSTATE_TEXTUREMAG, D3DFILTER_LINEAR );
		lpD3DDevice->SetRenderState( D3DRENDERSTATE_TEXTUREMIN, D3DFILTER_LINEAR );
 
		lpD3DDevice->SetRenderState( D3DRENDERSTATE_SPECULARENABLE, TRUE );
		lpD3DDevice->SetRenderState( D3DRENDERSTATE_COLORKEYENABLE, FALSE );

		lpD3DDevice->SetRenderState( D3DRENDERSTATE_TEXTUREMAPBLEND, D3DTBLEND_MODULATE );

		lpD3DDevice->SetRenderState( D3DRENDERSTATE_ALPHABLENDENABLE, TRUE );
		lpD3DDevice->SetRenderState( D3DRENDERSTATE_DESTBLEND, D3DBLEND_INVSRCALPHA );
		lpD3DDevice->SetRenderState( D3DRENDERSTATE_SRCBLEND, D3DBLEND_SRCALPHA );

		smRender.AlphaTestDepth = 60;

		lpD3DDevice->SetTextureStageState( 0, D3DTSS_MINFILTER, D3DTFN_LINEAR  );
        lpD3DDevice->SetTextureStageState( 0, D3DTSS_MAGFILTER, D3DTFN_LINEAR  );

//		lpD3DDevice->SetTextureStageState( 0, D3DTSS_MINFILTER, D3DTFN_POINT  );
//      lpD3DDevice->SetTextureStageState( 0, D3DTSS_MAGFILTER, D3DTFG_POINT  );

		//lpD3DDevice->SetRenderState( D3DRENDERSTATE_TEXTUREMIN, D3DTFN_POINT );
		//lpD3DDevice->SetRenderState( D3DRENDERSTATE_TEXTUREMAG, D3DTFN_POINT );
		
		return TRUE;
}

int psSetDefaultRenderState_PointFilter()
{

		  // Turn on Z-buffering

		lpD3DDevice->SetRenderState( D3DRENDERSTATE_ZENABLE, TRUE );
		lpD3DDevice->SetRenderState( D3DRENDERSTATE_ZWRITEENABLE, TRUE );
		lpD3DDevice->SetRenderState( D3DRENDERSTATE_ZFUNC,   D3DCMP_LESSEQUAL );
		//	lpD3DDevice->SetRenderState( D3DRENDERSTATE_ZENABLE, D3DZB_USEW );//TRUE );


		// null out the texture handle
		lpD3DDevice->SetRenderState( D3DRENDERSTATE_TEXTUREHANDLE, 0 );

		// turn on dithering
		lpD3DDevice->SetRenderState( D3DRENDERSTATE_DITHERENABLE, TRUE );

		lpD3DDevice->SetRenderState( D3DRENDERSTATE_TEXTUREADDRESS, D3DTADDRESS_WRAP );
		lpD3DDevice->SetRenderState( D3DRENDERSTATE_TEXTUREPERSPECTIVE, TRUE ); 

		// D3DFILTER_LINEAR
		lpD3DDevice->SetRenderState( D3DRENDERSTATE_TEXTUREMAG, D3DFILTER_LINEAR );
		lpD3DDevice->SetRenderState( D3DRENDERSTATE_TEXTUREMIN, D3DFILTER_LINEAR );
 
		lpD3DDevice->SetRenderState( D3DRENDERSTATE_SPECULARENABLE, TRUE );
		lpD3DDevice->SetRenderState( D3DRENDERSTATE_COLORKEYENABLE, FALSE );

		lpD3DDevice->SetRenderState( D3DRENDERSTATE_TEXTUREMAPBLEND, D3DTBLEND_MODULATE );

		lpD3DDevice->SetRenderState( D3DRENDERSTATE_ALPHABLENDENABLE, TRUE );
		lpD3DDevice->SetRenderState( D3DRENDERSTATE_DESTBLEND, D3DBLEND_INVSRCALPHA );
		lpD3DDevice->SetRenderState( D3DRENDERSTATE_SRCBLEND, D3DBLEND_SRCALPHA );

		smRender.AlphaTestDepth = 60;

		lpD3DDevice->SetTextureStageState( 0, D3DTSS_MINFILTER, D3DTFN_POINT  );
	    lpD3DDevice->SetTextureStageState( 0, D3DTSS_MAGFILTER, D3DTFG_POINT  );
		
		return TRUE;
}

//그림 출력
int psDrawTexImage( smTEXTUREHANDLE *hTexHandle , float x, float y, float w, float h ,
				   float fx, float fy, float fw, float fh, int Transp , int specular)
{
	
	D3DTLVERTEX tlVertex[4];
	int cnt;

	for(cnt=0;cnt<4;cnt++)
	{
		tlVertex[cnt].rhw = 1;
		tlVertex[cnt].color	= RGBA_MAKE( 255,255,255,Transp );
		tlVertex[cnt].specular  =  RGBA_MAKE( specular,specular,specular,0 );
		tlVertex[cnt].sz = 0;
	}

	tlVertex[0].sx = x;
	tlVertex[0].sy = y;
	tlVertex[0].tu = fx;
	tlVertex[0].tv = fy;

	tlVertex[1].sx = x;
	tlVertex[1].sy = y+h;
	tlVertex[1].tu = fx;
	tlVertex[1].tv = fh;

	tlVertex[2].sx = x+w;
	tlVertex[2].sy = y;
	tlVertex[2].tu = fw;
	tlVertex[2].tv = fy;

	tlVertex[3].sx = x+w;
	tlVertex[3].sy = y+h;
	tlVertex[3].tu = fw;
	tlVertex[3].tv = fh;
	
	//뒤집힌면 그리기
	if(lpD3DDevice != NULL)
	{

		psSetDefaultRenderState();

		lpD3DDevice->SetTexture( 0, hTexHandle->lpD3DTexture );

		lpD3DDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP ,
								D3DFVF_TLVERTEX  ,
								tlVertex,
								4,
								NULL);

	}
	return TRUE;
}

//그림 출력
int psDrawTexImage_Point( smTEXTUREHANDLE *hTexHandle , float x, float y, float w, float h ,
				   float fx, float fy, float fw, float fh, int Transp , int specular)
{
	
	D3DTLVERTEX tlVertex[4];
	int cnt;

	for(cnt=0;cnt<4;cnt++)
	{
		tlVertex[cnt].rhw = 1;
		tlVertex[cnt].color	= RGBA_MAKE( 255,255,255,Transp );
		tlVertex[cnt].specular  =  RGBA_MAKE( specular,specular,specular,0 );
		tlVertex[cnt].sz = 0;
	}

	tlVertex[0].sx = x;
	tlVertex[0].sy = y;
	tlVertex[0].tu = fx;
	tlVertex[0].tv = fy;

	tlVertex[1].sx = x;
	tlVertex[1].sy = y+h;
	tlVertex[1].tu = fx;
	tlVertex[1].tv = fh;

	tlVertex[2].sx = x+w;
	tlVertex[2].sy = y;
	tlVertex[2].tu = fw;
	tlVertex[2].tv = fy;

	tlVertex[3].sx = x+w;
	tlVertex[3].sy = y+h;
	tlVertex[3].tu = fw;
	tlVertex[3].tv = fh;
	
	//뒤집힌면 그리기
	if(lpD3DDevice != NULL)
	{

		psSetDefaultRenderState_PointFilter();

		lpD3DDevice->SetTexture( 0, hTexHandle->lpD3DTexture );

		lpD3DDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP ,
								D3DFVF_TLVERTEX  ,
								tlVertex,
								4,
								NULL);

	}
	return TRUE;
}

//그림 출력
int psDrawTexImage2( smTEXTUREHANDLE *hTexHandle , float *x, float *y, float fx, float fy, float fw, float fh, int Transp , int specular)
{
	
	D3DTLVERTEX tlVertex[4];
	int cnt;

	for(cnt=0;cnt<4;cnt++)
	{
		tlVertex[cnt].rhw = 1;
		tlVertex[cnt].color	= RGBA_MAKE( 255,255,255,Transp );
		tlVertex[cnt].specular  =  RGBA_MAKE( specular,specular,specular,0 );
		tlVertex[cnt].sz = 0;
	}

	tlVertex[0].sx = x[0];
	tlVertex[0].sy = y[0];
	tlVertex[0].tu = fx;
	tlVertex[0].tv = fy;

	tlVertex[1].sx = x[1];
	tlVertex[1].sy = y[1];
	tlVertex[1].tu = fx;
	tlVertex[1].tv = fh;

	tlVertex[2].sx = x[2];
	tlVertex[2].sy = y[2];
	tlVertex[2].tu = fw;
	tlVertex[2].tv = fy;

	tlVertex[3].sx = x[3];
	tlVertex[3].sy = y[3];
	tlVertex[3].tu = fw;
	tlVertex[3].tv = fh;




////////////////////////////

/*
	tlVertex[0].sx = x[0];
	tlVertex[0].sy = y[0];
	tlVertex[0].tu = fx;
	tlVertex[0].tv = fy;

	tlVertex[1].sx = x[0];
	tlVertex[1].sy = y[0]+16;
	tlVertex[1].tu = fx;
	tlVertex[1].tv = fh;

	tlVertex[2].sx = x[0]+16;
	tlVertex[2].sy = y[0];
	tlVertex[2].tu = fw;
	tlVertex[2].tv = fy;

	tlVertex[3].sx = x[0]+16;
	tlVertex[3].sy = y[0]+16;
	tlVertex[3].tu = fw;
	tlVertex[3].tv = fh;
*/
	
	if(lpD3DDevice != NULL)
	{

		psSetDefaultRenderState();

		lpD3DDevice->SetTexture( 0, hTexHandle->lpD3DTexture );

		lpD3DDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP ,
								D3DFVF_TLVERTEX  ,
								tlVertex,
								4,
								NULL);
	}
	return TRUE;
}


//데이타 압축 ( Z/NZ 방식 )
int EecodeCompress( BYTE *lpSrcData , BYTE *lpDestData , int size , int DestSize )
{
	int EncCounter;
	int cnt;
	int Counter;
	int pSet;
	BYTE c1,c2;
	int lsize;

	Counter = 0;
	EncCounter = 4;
	lsize = size-1;

	while(1) {		
		if ( Counter>=size ) break;
		if ( EncCounter>=DestSize ) break;

		c1 = lpSrcData[Counter];
		if ( Counter<lsize ) c2 = lpSrcData[Counter+1];
		else c2 = c1;

		if ( c1==0 && c2==0 ) {
			// 0 반복 횟수 구하여 압축
			for(cnt=0;cnt<0x7F;cnt++) {
				if ( Counter>=size ) break;
				if ( lpSrcData[Counter]!=0 ) break;
				Counter++;
			}
			if ( EncCounter>=DestSize ) break;
			lpDestData[EncCounter++]=cnt|0x80;
		}
		else {
			// 0이 아닌 데이타 반복 횟수 구하여 압축
			pSet = EncCounter;
			EncCounter++;

			for(cnt=0;cnt<0x7F;cnt++) {

				c1 = lpSrcData[Counter];
				if ( Counter<lsize ) c2 = lpSrcData[Counter+1];
				else c2 = c1;

				if ( Counter>=size ) break;
				if ( EncCounter>=DestSize ) break;
				if ( c1==0 && c2==0 ) break;
				lpDestData[EncCounter++]=lpSrcData[Counter++];
			}
			lpDestData[pSet]=cnt;
		}
	}

	//맨처음 DWORD는 압축된 길이
	((int *)lpDestData)[0] = EncCounter;

	return EncCounter;
}



//압축데이타 서버 아이템 해독용 ( Z/NZ 방식 )
int DecodeCompress_ItemPotion( rsPLAYINFO *lpPlayInfo , BYTE *lpSrcData , BYTE *lpDestData , int deCount , void *lpTransRecdata )
{

	int DecCounter;
	int cnt;
	int Counter;
	int size;
	int len;
	int dCount = deCount;
	int pCnt;
	TRANS_RECORD_DATA	*lpTransRecordData = (TRANS_RECORD_DATA *)lpTransRecdata;

	sRECORD_ITEM	*lpRecordItem = (sRECORD_ITEM *)lpDestData;

	size = ((int *)lpSrcData)[0];

	Counter = 4;
	DecCounter = 0;

	while(1) {		
		if ( Counter>=size ) break;
		if ( lpSrcData[Counter]&0x80 ) {
			//0 반복 데이타
			len = lpSrcData[Counter++]&0x7F;
			for(cnt=0;cnt<len;cnt++) {
				if ( DecCounter>=dCount ) {
					if ( dCount>=sizeof(sRECORD_ITEM) ) goto Skip_Decmp;
					if ( (lpRecordItem->sItemInfo.CODE&sinITEM_MASK1)!=(sinPM1&sinITEM_MASK1) ) 
						return Counter;

					dCount = sizeof(sRECORD_ITEM);
				}
				lpDestData[DecCounter++] = 0;
			}

		}
		else {
			len = lpSrcData[Counter++]&0x7F;
			for(cnt=0;cnt<len;cnt++) {
				if ( DecCounter>=dCount ) {
					if ( dCount>=sizeof(sRECORD_ITEM) ) goto Skip_Decmp;
					if ( (lpRecordItem->sItemInfo.CODE&sinITEM_MASK1)!=(sinPM1&sinITEM_MASK1) ) 
						return Counter;

					dCount = sizeof(sRECORD_ITEM);
				}
				lpDestData[DecCounter++] = lpSrcData[Counter++];
			}
		}
	}

Skip_Decmp:

	if ( lpTransRecordData ) {
		//버려진 아이템 확인
		for( cnt=0;cnt<lpTransRecordData->ThrowItemCount;cnt++ ) {

			if ( lpTransRecordData->ThrowItemInfo[cnt].dwCode==lpRecordItem->sItemInfo.CODE &&
				lpTransRecordData->ThrowItemInfo[cnt].dwKey==lpRecordItem->sItemInfo.ItemHeader.Head &&
				lpTransRecordData->ThrowItemInfo[cnt].dwSum==lpRecordItem->sItemInfo.ItemHeader.dwChkSum ) {

					//제거될 아이템 이다. (등록취소하고 리턴한다)
					return Counter;

				}
		}
	}

	pCnt = lpRecordItem->sItemInfo.PotionCount;
	if ( pCnt==0 ) pCnt=1;

	rsAddServerPotion( lpPlayInfo , lpRecordItem->sItemInfo.CODE , pCnt );
	return Counter;
}


//압축데이타 해독 ( Z/NZ 방식 )
int DecodeCompress( BYTE *lpSrcData , BYTE *lpDestData , int dCount )
{

	int DecCounter;
	int cnt;
	int Counter;
	int size;
	int len;

	size = ((int *)lpSrcData)[0];

	Counter = 4;
	DecCounter = 0;

	while(1) {		
		if ( Counter>=size ) break;
		if ( lpSrcData[Counter]&0x80 ) {
			//0 반복 데이타
			len = lpSrcData[Counter++]&0x7F;
			for(cnt=0;cnt<len;cnt++) {
				if ( DecCounter>=dCount ) return Counter;
				lpDestData[DecCounter++] = 0;
			}

		}
		else {
			len = lpSrcData[Counter++]&0x7F;
			for(cnt=0;cnt<len;cnt++) {
				if ( DecCounter>=dCount ) return Counter;
				lpDestData[DecCounter++] = lpSrcData[Counter++];
			}
		}
	}

	return Counter;
}


//압축데이타 해독 ( Z/NZ 방식 )
int DecodeCompress( BYTE *lpSrcData , BYTE *lpDestData )
{
	return DecodeCompress(lpSrcData, lpDestData , 0x7FFFF );
}
//데이타 압축 ( Z/NZ 방식 )
int EecodeCompress( BYTE *lpSrcData , BYTE *lpDestData , int size )
{
	return EecodeCompress( lpSrcData , lpDestData , size , 0x7FFFF );
}

//다른 캐릭터와의 위치 겹치는지 확인
smCHAR *CheckOtherPlayPosi( int x, int y, int z )
{
	int cnt;
	int dist;
	int dx,dz;


	if ( ServerMode )
		return Serv_CheckOtherPlayPosi( x,y,z );

	for( cnt=0;cnt<OTHER_PLAYER_MAX;cnt++) {
		if ( chrOtherPlayer[cnt].Flag && chrOtherPlayer[cnt].smCharInfo.Life[0]>0 && 
			(chrOtherPlayer[cnt].dwLastTransTime+DISPLAY_TRANS_TIME_OVER)>dwPlayTime ) {
			if ( (y-16*fONE)<chrOtherPlayer[cnt].pY && (y+16*fONE)>chrOtherPlayer[cnt].pY ) {

				dx = (chrOtherPlayer[cnt].pX-x)>>FLOATNS;
				dz = (chrOtherPlayer[cnt].pZ-z)>>FLOATNS;
				dist =dx*dx+dz*dz;
				if ( dist<(16*16) ) return &chrOtherPlayer[cnt];
			}
		}
	}

	return NULL;
}



#ifdef PT_USE_SDL
#include "ptplatform\ptplatform.h"
#endif

//로딩중 메세지 처리
int MessageLoading = 0;

//윈도 메세지 처리 함수
int smPlayWindowMessage()
{
	MSG msg;

	MessageLoading = TRUE;

	while (1) {
	    if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {

			TranslateMessage(&msg);
  			 if ( msg.message==WM_SYSKEYDOWN || msg.message==WM_SYSKEYUP ) continue;
			DispatchMessage(&msg);
	    }
		else 
			break;
	}
#ifdef PT_USE_SDL
	if ( PtPlat_IsSDL() ) PtPlat_PumpEvents();
#endif

	MessageLoading = FALSE;
	return TRUE;
}

/*
struct sITEMINFO{

	DWORD	dwSize;						//아이템 구조 크기
	
	sITEM_CREATE	ItemHeader;			//아이템 생성 정보

	short	Durability[2];				//내구력

	DWORD	CODE;				//아이템 코드 	
	char	ItemName[32];		//아이템 이름 	

	int     Weight;				//무게 		
	int     Price;				//가격

	/////////////////////////////////////////
	int		Index;				//인덱스
	int		PotionCount;		//포션카운터
	////////////////////////////////////////

	short	Resistance[8];		//원소에대한 저항력 

	int		Sight;				//시야

	DWORD	Temp0;

	short	Damage[2];			//공격력 
	int     Shooting_Range;		//사정거리 
	int     Attack_Speed;		//공격속도 
	int     Attack_Rating;		//명중력 
	int     Critical_Hit;		//1.5배 데미지확율  


	float	fAbsorb;				//흡수력 
  	int     Defence;			//방어력 
	float   fBlock_Rating;		//(방패)블럭율 

	float     fSpeed;				//이동 속도 


	int		Potion_Space;		//소켓 공간 할당 
	
	float   fMagic_Mastery;     //마법숙련도 
	float   fMana_Regen;		//마나 재생 
	float   fLife_Regen;		//라이프 재생 
	float   fStamina_Regen;	    //스테미나 재생 
	float   fIncrease_Life;     //라이프 최대치상승 
	float   fIncrease_Mana;     //마나 최대치상승
	float   fIncrease_Stamina;  //스테미나 최대치상승

	int     Level;				//레벨 
	int     Strength;			//힘 	
	int     Spirit;				//정신력 
	int     Talent;				//재능 
	int     Dexterity;			//민첩성 
	int     Health;				//건강 

	short   Mana[2];            //마나회복(최소)(최대)
	short   Life[2];            //라이프회복(최소)(최대) 
	short   Stamina[2];         //스테미너회복(최소)(최대)
	

	int		Money;				//보유한 돈
	int     NotUseFlag;         //사용불가능 플랙


	DWORD	dwTemp1[7];			//예비

	DWORD	DispEffect;			//아이템 표시 이펙트 (예정)


	DWORD			JobCodeMask;		//특화 직업	(직업별 비트 마스크)
	sITEM_SPECIAL	JobItem;			//특화 아이템 성능

	DWORD	dwTemp[16];			//예비
};
*/
static DWORD iFormCode;
static DWORD iFromCnt;

static DWORD ReformStateCode( char *lpState , int size )
{
	int cnt;

	for(cnt=0;cnt<size;cnt++) {
		iFormCode += ((DWORD)lpState[cnt])*((iFromCnt&0x1FFF)+1);
		iFromCnt++;
	}

	return iFormCode;
}

static DWORD CompareStateCode( char *lpState1 , char *lpState2 , int size )
{
	int cnt;

	for(cnt=0;cnt<size;cnt++) {
		if ( lpState1[cnt]!=lpState2[cnt] )
			return 1;
	}

	return 0;
}


//구버전 인증 처리
static DWORD GetRefromItemCode_Old( sITEMINFO *lpItemInfo , int Key ) {

	iFormCode = 0;
	iFromCnt = Key;

	ReformStateCode( (char *)&lpItemInfo->CODE , sizeof(DWORD) );
	ReformStateCode( (char *)lpItemInfo->ItemName , 32 );

	ReformStateCode( (char *)&lpItemInfo->Weight , sizeof(int) );
	ReformStateCode( (char *)&lpItemInfo->Price , sizeof(int) );

	ReformStateCode( (char *)lpItemInfo->Resistance , sizeof(short)*8 );
	ReformStateCode( (char *)lpItemInfo->Damage , sizeof(short)*2 );

	//ReformStateCode( (char *)&lpItemInfo->Defence , sizeof(int) );

	ReformStateCode( (char *)&lpItemInfo->Shooting_Range , sizeof(int) );
	ReformStateCode( (char *)&lpItemInfo->Attack_Speed , sizeof(int) );

	ReformStateCode( (char *)&lpItemInfo->Attack_Rating , sizeof(int) );
	ReformStateCode( (char *)&lpItemInfo->Critical_Hit , sizeof(int) );

	ReformStateCode( (char *)&lpItemInfo->fSpeed , sizeof(float) );
	ReformStateCode( (char *)&lpItemInfo->Potion_Space , sizeof(int) );

	ReformStateCode( (char *)&lpItemInfo->fMagic_Mastery , sizeof(float) );
	ReformStateCode( (char *)&lpItemInfo->fMana_Regen , sizeof(float) );
	ReformStateCode( (char *)&lpItemInfo->fLife_Regen , sizeof(float) );
	ReformStateCode( (char *)&lpItemInfo->fStamina_Regen , sizeof(float) );
	ReformStateCode( (char *)&lpItemInfo->fIncrease_Life , sizeof(float) );
	ReformStateCode( (char *)&lpItemInfo->fIncrease_Mana , sizeof(float) );
	ReformStateCode( (char *)&lpItemInfo->fIncrease_Stamina , sizeof(float) );

	ReformStateCode( (char *)&lpItemInfo->Level , sizeof(int) );
	ReformStateCode( (char *)&lpItemInfo->Strength , sizeof(int) );
	ReformStateCode( (char *)&lpItemInfo->Spirit , sizeof(int) );
	ReformStateCode( (char *)&lpItemInfo->Talent , sizeof(int) );
	ReformStateCode( (char *)&lpItemInfo->Dexterity , sizeof(int) );
	ReformStateCode( (char *)&lpItemInfo->Health , sizeof(int) );

	ReformStateCode( (char *)lpItemInfo->Mana , sizeof(short)*2 );
	ReformStateCode( (char *)lpItemInfo->Life , sizeof(short)*2 );
	ReformStateCode( (char *)lpItemInfo->Stamina , sizeof(short)*2 );
	
	ReformStateCode( (char *)&lpItemInfo->Money , sizeof(int) );

	ReformStateCode( (char *)&lpItemInfo->JobCodeMask , sizeof(DWORD) );

	ReformStateCode( (char *)&lpItemInfo->JobItem , sizeof(sITEM_SPECIAL) );

	return iFormCode;
}



static DWORD GetRefromItemCode_V10( sITEMINFO *lpItemInfo , int Key ) {

	iFormCode = 0;
	iFromCnt = Key;

	ReformStateCode( (char *)&lpItemInfo->CODE , sizeof(DWORD) );
	ReformStateCode( (char *)lpItemInfo->ItemName , 32 );

	ReformStateCode( (char *)&lpItemInfo->Weight , sizeof(int) );
	ReformStateCode( (char *)&lpItemInfo->Price , sizeof(int) );

	ReformStateCode( (char *)lpItemInfo->Resistance , sizeof(short)*8 );
	ReformStateCode( (char *)lpItemInfo->Damage , sizeof(short)*2 );

	ReformStateCode( (char *)&lpItemInfo->Defence , sizeof(int) );

	ReformStateCode( (char *)&lpItemInfo->Shooting_Range , sizeof(int) );
	ReformStateCode( (char *)&lpItemInfo->Attack_Speed , sizeof(int) );

	ReformStateCode( (char *)&lpItemInfo->Attack_Rating , sizeof(int) );
	ReformStateCode( (char *)&lpItemInfo->Critical_Hit , sizeof(int) );

	ReformStateCode( (char *)&lpItemInfo->fSpeed , sizeof(float) );
	ReformStateCode( (char *)&lpItemInfo->Potion_Space , sizeof(int) );

	ReformStateCode( (char *)&lpItemInfo->fMagic_Mastery , sizeof(float) );
	ReformStateCode( (char *)&lpItemInfo->fMana_Regen , sizeof(float) );
	ReformStateCode( (char *)&lpItemInfo->fLife_Regen , sizeof(float) );
	ReformStateCode( (char *)&lpItemInfo->fStamina_Regen , sizeof(float) );
	ReformStateCode( (char *)&lpItemInfo->fIncrease_Life , sizeof(float) );
	ReformStateCode( (char *)&lpItemInfo->fIncrease_Mana , sizeof(float) );
	ReformStateCode( (char *)&lpItemInfo->fIncrease_Stamina , sizeof(float) );

	ReformStateCode( (char *)&lpItemInfo->Level , sizeof(int) );
	ReformStateCode( (char *)&lpItemInfo->Strength , sizeof(int) );
	ReformStateCode( (char *)&lpItemInfo->Spirit , sizeof(int) );
	ReformStateCode( (char *)&lpItemInfo->Talent , sizeof(int) );
	ReformStateCode( (char *)&lpItemInfo->Dexterity , sizeof(int) );
	ReformStateCode( (char *)&lpItemInfo->Health , sizeof(int) );

	ReformStateCode( (char *)lpItemInfo->Mana , sizeof(short)*2 );
	ReformStateCode( (char *)lpItemInfo->Life , sizeof(short)*2 );
	ReformStateCode( (char *)lpItemInfo->Stamina , sizeof(short)*2 );
	
	ReformStateCode( (char *)&lpItemInfo->Money , sizeof(int) );

	ReformStateCode( (char *)&lpItemInfo->JobCodeMask , sizeof(DWORD) );

	ReformStateCode( (char *)&lpItemInfo->JobItem , sizeof(sITEM_SPECIAL) );

	return iFormCode;
}

//static DWORD CompareStateCode( char *lpState1 , char *lpState2 , int size )

static int CompareReformItemCode_V10( sITEMINFO *lpItemInfo1 , sITEMINFO *lpItemInfo2 ) 
{

	int code;

	code = 0;

	code += CompareStateCode( (char *)&lpItemInfo1->CODE , (char *)&lpItemInfo2->CODE , sizeof(DWORD) );
	code += CompareStateCode( (char *)lpItemInfo1->ItemName , (char *)lpItemInfo2->ItemName, 32 );

	code += CompareStateCode( (char *)&lpItemInfo1->Weight , (char *)&lpItemInfo2->Weight , sizeof(int) );
	code += CompareStateCode( (char *)&lpItemInfo1->Price , (char *)&lpItemInfo2->Price , sizeof(int) );

	code += CompareStateCode( (char *)lpItemInfo1->Resistance , (char *)lpItemInfo2->Resistance , sizeof(short)*8 );
	code += CompareStateCode( (char *)lpItemInfo1->Damage , (char *)lpItemInfo2->Damage , sizeof(short)*2 );

	code += CompareStateCode( (char *)&lpItemInfo1->Defence , (char *)&lpItemInfo2->Defence , sizeof(int) );

	code += CompareStateCode( (char *)&lpItemInfo1->Shooting_Range , (char *)&lpItemInfo2->Shooting_Range , sizeof(int) );
	code += CompareStateCode( (char *)&lpItemInfo1->Attack_Speed , (char *)&lpItemInfo2->Attack_Speed , sizeof(int) );

	code += CompareStateCode( (char *)&lpItemInfo1->Attack_Rating , (char *)&lpItemInfo2->Attack_Rating , sizeof(int) );
	code += CompareStateCode( (char *)&lpItemInfo1->Critical_Hit , (char *)&lpItemInfo2->Critical_Hit , sizeof(int) );

	code += CompareStateCode( (char *)&lpItemInfo1->fSpeed , (char *)&lpItemInfo2->fSpeed , sizeof(float) );
	code += CompareStateCode( (char *)&lpItemInfo1->Potion_Space , (char *)&lpItemInfo2->Potion_Space , sizeof(int) );

	code += CompareStateCode( (char *)&lpItemInfo1->fMagic_Mastery , (char *)&lpItemInfo2->fMagic_Mastery , sizeof(float) );
	code += CompareStateCode( (char *)&lpItemInfo1->fMana_Regen , (char *)&lpItemInfo2->fMana_Regen , sizeof(float) );
	code += CompareStateCode( (char *)&lpItemInfo1->fLife_Regen , (char *)&lpItemInfo2->fLife_Regen , sizeof(float) );
	code += CompareStateCode( (char *)&lpItemInfo1->fStamina_Regen , (char *)&lpItemInfo2->fStamina_Regen , sizeof(float) );
	code += CompareStateCode( (char *)&lpItemInfo1->fIncrease_Life , (char *)&lpItemInfo2->fIncrease_Life , sizeof(float) );
	code += CompareStateCode( (char *)&lpItemInfo1->fIncrease_Mana , (char *)&lpItemInfo2->fIncrease_Mana , sizeof(float) );
	code += CompareStateCode( (char *)&lpItemInfo1->fIncrease_Stamina , (char *)&lpItemInfo2->fIncrease_Stamina , sizeof(float) );

	code += CompareStateCode( (char *)&lpItemInfo1->Level , (char *)&lpItemInfo2->Level , sizeof(int) );
	code += CompareStateCode( (char *)&lpItemInfo1->Strength , (char *)&lpItemInfo2->Strength , sizeof(int) );
	code += CompareStateCode( (char *)&lpItemInfo1->Spirit , (char *)&lpItemInfo2->Spirit , sizeof(int) );
	code += CompareStateCode( (char *)&lpItemInfo1->Talent , (char *)&lpItemInfo2->Talent , sizeof(int) );
	code += CompareStateCode( (char *)&lpItemInfo1->Dexterity , (char *)&lpItemInfo2->Dexterity , sizeof(int) );
	code += CompareStateCode( (char *)&lpItemInfo1->Health , (char *)&lpItemInfo2->Health , sizeof(int) );

	code += CompareStateCode( (char *)lpItemInfo1->Mana , (char *)lpItemInfo2->Mana , sizeof(short)*2 );
	code += CompareStateCode( (char *)lpItemInfo1->Life , (char *)lpItemInfo2->Life , sizeof(short)*2 );
	code += CompareStateCode( (char *)lpItemInfo1->Stamina , (char *)lpItemInfo2->Stamina , sizeof(short)*2 );
	
	code += CompareStateCode( (char *)&lpItemInfo1->Money , (char *)&lpItemInfo2->Money , sizeof(int) );

	code += CompareStateCode( (char *)&lpItemInfo1->JobCodeMask , (char *)&lpItemInfo2->JobCodeMask , sizeof(DWORD) );

	code += CompareStateCode( (char *)&lpItemInfo1->JobItem , (char *)&lpItemInfo2->JobItem , sizeof(sITEM_SPECIAL) );

	return code;
}

static DWORD GetRefromItemCode( sITEMINFO *lpItemInfo , int Key ) {

	iFormCode = 0;
	iFromCnt = Key;

	ReformStateCode( (char *)&lpItemInfo->CODE , sizeof(DWORD) );
	ReformStateCode( (char *)lpItemInfo->ItemName , 32 );

	ReformStateCode( (char *)&lpItemInfo->Weight , sizeof(int) );
	ReformStateCode( (char *)&lpItemInfo->Price , sizeof(int) );

	ReformStateCode( (char *)lpItemInfo->Resistance , sizeof(short)*8 );
	ReformStateCode( (char *)lpItemInfo->Damage , sizeof(short)*2 );

	ReformStateCode( (char *)&lpItemInfo->fAbsorb , sizeof(float) );
	ReformStateCode( (char *)&lpItemInfo->Defence , sizeof(int) );
	ReformStateCode( (char *)&lpItemInfo->fBlock_Rating , sizeof(float) );

	ReformStateCode( (char *)&lpItemInfo->Shooting_Range , sizeof(int) );
	ReformStateCode( (char *)&lpItemInfo->Attack_Speed , sizeof(int) );

	ReformStateCode( (char *)&lpItemInfo->Attack_Rating , sizeof(int) );
	ReformStateCode( (char *)&lpItemInfo->Critical_Hit , sizeof(int) );

	ReformStateCode( (char *)&lpItemInfo->fSpeed , sizeof(float) );
	ReformStateCode( (char *)&lpItemInfo->Potion_Space , sizeof(int) );

	ReformStateCode( (char *)&lpItemInfo->fMagic_Mastery , sizeof(float) );
	ReformStateCode( (char *)&lpItemInfo->fMana_Regen , sizeof(float) );
	ReformStateCode( (char *)&lpItemInfo->fLife_Regen , sizeof(float) );
	ReformStateCode( (char *)&lpItemInfo->fStamina_Regen , sizeof(float) );
	ReformStateCode( (char *)&lpItemInfo->fIncrease_Life , sizeof(float) );
	ReformStateCode( (char *)&lpItemInfo->fIncrease_Mana , sizeof(float) );
	ReformStateCode( (char *)&lpItemInfo->fIncrease_Stamina , sizeof(float) );

	ReformStateCode( (char *)&lpItemInfo->Level , sizeof(int) );
	ReformStateCode( (char *)&lpItemInfo->Strength , sizeof(int) );
	ReformStateCode( (char *)&lpItemInfo->Spirit , sizeof(int) );
	ReformStateCode( (char *)&lpItemInfo->Talent , sizeof(int) );
	ReformStateCode( (char *)&lpItemInfo->Dexterity , sizeof(int) );
	ReformStateCode( (char *)&lpItemInfo->Health , sizeof(int) );

	ReformStateCode( (char *)lpItemInfo->Mana , sizeof(short)*2 );
	ReformStateCode( (char *)lpItemInfo->Life , sizeof(short)*2 );
	ReformStateCode( (char *)lpItemInfo->Stamina , sizeof(short)*2 );
	
	ReformStateCode( (char *)&lpItemInfo->Money , sizeof(int) );
	ReformStateCode( (char *)&lpItemInfo->JobCodeMask , sizeof(DWORD) );
	ReformStateCode( (char *)&lpItemInfo->JobItem , sizeof(sITEM_SPECIAL) );

	ReformStateCode( (char *)&lpItemInfo->UniqueItem , sizeof(DWORD) );
	ReformStateCode( (char *)lpItemInfo->EffectBlink , sizeof(short)*2 );
	ReformStateCode( (char *)lpItemInfo->EffectColor , sizeof(short)*4 );
	ReformStateCode( (char *)lpItemInfo->ScaleBlink , sizeof(short)*2 );
	ReformStateCode( (char *)&lpItemInfo->DispEffect , sizeof(DWORD) );

	return iFormCode;
}


///////////////////// 서버용 /////////////////////////////
#ifdef _W_SERVER

static DWORD rsReformStateCode( char *lpState , int size )
{
	int cnt;

	for(cnt=0;cnt<size;cnt++) {
		iFormCode += ((DWORD)lpState[cnt])*((iFromCnt&0x7FF)+1);
		iFromCnt++;
	}

	return iFormCode;
}


static DWORD rsGetRefromItemCode( sITEMINFO *lpItemInfo , int Key ) {

	iFormCode = 0;
	iFromCnt = Key;

	rsReformStateCode( (char *)&lpItemInfo->CODE , sizeof(DWORD) );
	rsReformStateCode( (char *)lpItemInfo->ItemName , 32 );

	rsReformStateCode( (char *)&lpItemInfo->Weight , sizeof(int) );
	rsReformStateCode( (char *)&lpItemInfo->Price , sizeof(int) );

	rsReformStateCode( (char *)lpItemInfo->Resistance , sizeof(short)*8 );
	rsReformStateCode( (char *)lpItemInfo->Damage , sizeof(short)*2 );

	rsReformStateCode( (char *)&lpItemInfo->fAbsorb , sizeof(float) );
	rsReformStateCode( (char *)&lpItemInfo->Defence , sizeof(int) );
	rsReformStateCode( (char *)&lpItemInfo->fBlock_Rating , sizeof(float) );

	rsReformStateCode( (char *)&lpItemInfo->Shooting_Range , sizeof(int) );
	rsReformStateCode( (char *)&lpItemInfo->Attack_Speed , sizeof(int) );

	rsReformStateCode( (char *)&lpItemInfo->Attack_Rating , sizeof(int) );
	rsReformStateCode( (char *)&lpItemInfo->Critical_Hit , sizeof(int) );

	rsReformStateCode( (char *)&lpItemInfo->fSpeed , sizeof(float) );
	rsReformStateCode( (char *)&lpItemInfo->Potion_Space , sizeof(int) );

	rsReformStateCode( (char *)&lpItemInfo->fMagic_Mastery , sizeof(float) );
	rsReformStateCode( (char *)&lpItemInfo->fMana_Regen , sizeof(float) );
	rsReformStateCode( (char *)&lpItemInfo->fLife_Regen , sizeof(float) );
	rsReformStateCode( (char *)&lpItemInfo->fStamina_Regen , sizeof(float) );
	rsReformStateCode( (char *)&lpItemInfo->fIncrease_Life , sizeof(float) );
	rsReformStateCode( (char *)&lpItemInfo->fIncrease_Mana , sizeof(float) );
	rsReformStateCode( (char *)&lpItemInfo->fIncrease_Stamina , sizeof(float) );

	rsReformStateCode( (char *)&lpItemInfo->Level , sizeof(int) );
	rsReformStateCode( (char *)&lpItemInfo->Strength , sizeof(int) );
	rsReformStateCode( (char *)&lpItemInfo->Spirit , sizeof(int) );
	rsReformStateCode( (char *)&lpItemInfo->Talent , sizeof(int) );
	rsReformStateCode( (char *)&lpItemInfo->Dexterity , sizeof(int) );
	rsReformStateCode( (char *)&lpItemInfo->Health , sizeof(int) );

	rsReformStateCode( (char *)lpItemInfo->Mana , sizeof(short)*2 );
	rsReformStateCode( (char *)lpItemInfo->Life , sizeof(short)*2 );
	rsReformStateCode( (char *)lpItemInfo->Stamina , sizeof(short)*2 );
	
	rsReformStateCode( (char *)&lpItemInfo->Money , sizeof(int) );
	rsReformStateCode( (char *)&lpItemInfo->JobCodeMask , sizeof(DWORD) );
	rsReformStateCode( (char *)&lpItemInfo->JobItem , sizeof(sITEM_SPECIAL) );

	rsReformStateCode( (char *)&lpItemInfo->UniqueItem , sizeof(DWORD) );
	rsReformStateCode( (char *)lpItemInfo->EffectBlink , sizeof(short)*2 );
	rsReformStateCode( (char *)lpItemInfo->EffectColor , sizeof(short)*4 );
	rsReformStateCode( (char *)lpItemInfo->ScaleBlink , sizeof(short)*2 );
	ReformStateCode( (char *)&lpItemInfo->DispEffect , sizeof(DWORD) );

	return iFormCode;
}

#ifdef _NEW_ITEM_FORMCODE3
#include "ItemForm2.h"
#else
#include "ItemForm.h"
#endif

//아이템 서버 인증값 얻기
int	rsGetItemForm_Server( sITEMINFO *lpItemInfo )
{

	return rsGetItem_ServerForm( lpItemInfo );
/*
	DWORD dwCode;

#ifdef	_LANGUAGE_CHINESE
	//중국판 새 인증방법

	//서버용 재 인증 아이템 ( 신버전 인증 )
	dwCode = rsGetRefromItemCode2( lpItemInfo , lpItemInfo->ItemHeader.dwChkSum );
	dwCode = dwCode ^ ((lpItemInfo->ItemAgingProtect[0]*7)^(lpItemInfo->ItemAgingProtect[1]*9));	//추가 (믹스/에이징) 프로텍트
	dwCode+= (lpItemInfo->dwCreateTime&0xFFFF)<<2;
	dwCode = dwCode ^ (lpItemInfo->ItemHeader.Head^((lpItemInfo->ItemHeader.dwChkSum+dwCode)<<15));

#else

	//서버용 재 인증 아이템
	dwCode = rsGetRefromItemCode( lpItemInfo , lpItemInfo->ItemHeader.dwChkSum );

#ifdef	_NEW_ITEM_FORMCODE
	dwCode = dwCode ^ ((lpItemInfo->ItemAgingProtect[0]*7)^(lpItemInfo->ItemAgingProtect[1]*9));	//추가 (믹스/에이징) 프로텍트
	dwCode+= lpItemInfo->dwCreateTime>>8;
#endif

	dwCode = dwCode ^ lpItemInfo->ItemHeader.Head;

#endif

	return dwCode;
*/
}


//아이템 서버 인증만 받기
int	rsReformItem_Server( sITEMINFO *lpItemInfo )
{
	DWORD dwCode;

	//서버용 재 인증 아이템
	dwCode = rsGetItemForm_Server( lpItemInfo );

	lpItemInfo->ItemHeader.dwTime = dwCode;

	return TRUE;
}
#else
int	rsGetItemForm_Server( sITEMINFO *lpItemInfo )
{

	return TRUE;
}

int	rsReformItem_Server( sITEMINFO *lpItemInfo )
{

	return TRUE;
}


#endif

//static DWORD CompareStateCode( char *lpState1 , char *lpState2 , int size )

static int CompareReformItemCode( sITEMINFO *lpItemInfo1 , sITEMINFO *lpItemInfo2 ) 
{

	int code;

	code = 0;

	code += CompareStateCode( (char *)&lpItemInfo1->CODE , (char *)&lpItemInfo2->CODE , sizeof(DWORD) );
	code += CompareStateCode( (char *)lpItemInfo1->ItemName , (char *)lpItemInfo2->ItemName, 32 );

	code += CompareStateCode( (char *)&lpItemInfo1->Weight , (char *)&lpItemInfo2->Weight , sizeof(int) );
	code += CompareStateCode( (char *)&lpItemInfo1->Price , (char *)&lpItemInfo2->Price , sizeof(int) );

	code += CompareStateCode( (char *)lpItemInfo1->Resistance , (char *)lpItemInfo2->Resistance , sizeof(short)*8 );
	code += CompareStateCode( (char *)lpItemInfo1->Damage , (char *)lpItemInfo2->Damage , sizeof(short)*2 );

	code += CompareStateCode( (char *)&lpItemInfo1->fAbsorb , (char *)&lpItemInfo2->fAbsorb , sizeof(float) );
	code += CompareStateCode( (char *)&lpItemInfo1->Defence , (char *)&lpItemInfo2->Defence , sizeof(int) );
	code += CompareStateCode( (char *)&lpItemInfo1->fBlock_Rating , (char *)&lpItemInfo2->fBlock_Rating , sizeof(float) );

	code += CompareStateCode( (char *)&lpItemInfo1->Shooting_Range , (char *)&lpItemInfo2->Shooting_Range , sizeof(int) );
	code += CompareStateCode( (char *)&lpItemInfo1->Attack_Speed , (char *)&lpItemInfo2->Attack_Speed , sizeof(int) );

	code += CompareStateCode( (char *)&lpItemInfo1->Attack_Rating , (char *)&lpItemInfo2->Attack_Rating , sizeof(int) );
	code += CompareStateCode( (char *)&lpItemInfo1->Critical_Hit , (char *)&lpItemInfo2->Critical_Hit , sizeof(int) );

	code += CompareStateCode( (char *)&lpItemInfo1->fSpeed , (char *)&lpItemInfo2->fSpeed , sizeof(float) );
	code += CompareStateCode( (char *)&lpItemInfo1->Potion_Space , (char *)&lpItemInfo2->Potion_Space , sizeof(int) );

	code += CompareStateCode( (char *)&lpItemInfo1->fMagic_Mastery , (char *)&lpItemInfo2->fMagic_Mastery , sizeof(float) );
	code += CompareStateCode( (char *)&lpItemInfo1->fMana_Regen , (char *)&lpItemInfo2->fMana_Regen , sizeof(float) );
	code += CompareStateCode( (char *)&lpItemInfo1->fLife_Regen , (char *)&lpItemInfo2->fLife_Regen , sizeof(float) );
	code += CompareStateCode( (char *)&lpItemInfo1->fStamina_Regen , (char *)&lpItemInfo2->fStamina_Regen , sizeof(float) );
	code += CompareStateCode( (char *)&lpItemInfo1->fIncrease_Life , (char *)&lpItemInfo2->fIncrease_Life , sizeof(float) );
	code += CompareStateCode( (char *)&lpItemInfo1->fIncrease_Mana , (char *)&lpItemInfo2->fIncrease_Mana , sizeof(float) );
	code += CompareStateCode( (char *)&lpItemInfo1->fIncrease_Stamina , (char *)&lpItemInfo2->fIncrease_Stamina , sizeof(float) );

	code += CompareStateCode( (char *)&lpItemInfo1->Level , (char *)&lpItemInfo2->Level , sizeof(int) );
	code += CompareStateCode( (char *)&lpItemInfo1->Strength , (char *)&lpItemInfo2->Strength , sizeof(int) );
	code += CompareStateCode( (char *)&lpItemInfo1->Spirit , (char *)&lpItemInfo2->Spirit , sizeof(int) );
	code += CompareStateCode( (char *)&lpItemInfo1->Talent , (char *)&lpItemInfo2->Talent , sizeof(int) );
	code += CompareStateCode( (char *)&lpItemInfo1->Dexterity , (char *)&lpItemInfo2->Dexterity , sizeof(int) );
	code += CompareStateCode( (char *)&lpItemInfo1->Health , (char *)&lpItemInfo2->Health , sizeof(int) );

	code += CompareStateCode( (char *)lpItemInfo1->Mana , (char *)lpItemInfo2->Mana , sizeof(short)*2 );
	code += CompareStateCode( (char *)lpItemInfo1->Life , (char *)lpItemInfo2->Life , sizeof(short)*2 );
	code += CompareStateCode( (char *)lpItemInfo1->Stamina , (char *)lpItemInfo2->Stamina , sizeof(short)*2 );
	
	code += CompareStateCode( (char *)&lpItemInfo1->Money , (char *)&lpItemInfo2->Money , sizeof(int) );
	code += CompareStateCode( (char *)&lpItemInfo1->JobCodeMask , (char *)&lpItemInfo2->JobCodeMask , sizeof(DWORD) );
	code += CompareStateCode( (char *)&lpItemInfo1->JobItem , (char *)&lpItemInfo2->JobItem , sizeof(sITEM_SPECIAL) );

	code += CompareStateCode( (char *)&lpItemInfo1->UniqueItem , (char *)&lpItemInfo2->UniqueItem , sizeof(DWORD) );
	code += CompareStateCode( (char *)lpItemInfo1->EffectBlink , (char *)lpItemInfo2->EffectBlink , sizeof(short)*2 );
	code += CompareStateCode( (char *)lpItemInfo1->EffectColor , (char *)lpItemInfo2->EffectColor , sizeof(short)*4 );
	code += CompareStateCode( (char *)lpItemInfo1->ScaleBlink , (char *)lpItemInfo2->ScaleBlink , sizeof(short)*2 );
	code += CompareStateCode( (char *)&lpItemInfo1->DispEffect , (char *)&lpItemInfo2->DispEffect , sizeof(DWORD) );

	return code;
}



//아이템 교환창 내용 인증
static DWORD GetRefromTradeCode( sTRADE *lpTrade , int Key ) {

	int cnt;

	iFormCode = 0;
	iFromCnt = Key;

	for(cnt=0;cnt<MAX_TRADE_ITEM;cnt++) {
		if ( lpTrade->TradeItem[cnt].Flag ) {
			ReformStateCode( (char *)&lpTrade->TradeItem[cnt].sItemInfo.CODE  , sizeof(DWORD) );
			ReformStateCode( (char *)&lpTrade->TradeItem[cnt].sItemInfo.ItemHeader.dwChkSum , sizeof(DWORD) );
		}
	}
	ReformStateCode( (char *)&lpTrade->Money , sizeof(int) );

	return iFormCode;
}


static DWORD GetRefromCharState( smCHAR_INFO *lpCharInfo , int Key ) {

	iFormCode = 0;
	iFromCnt = Key;

	/*
	int   Level;			//레벨 
	int   Strength;			//힘 
	int   Spirit;			//정신력 
	int   Talent;			//재능 
	int   Dexterity;		//민첩성 
	int   Health;			//건강 

	short	Resistance[8];			//원소에대한 저항력 
	short	Attack_Resistance[8];	//원소에대한 공격력 
	short   Life[2];			//0 현재 1 최대치 
	short   Mana[2];			//0 현재 1 최대치 
	short   Stamina[2];			//0 현재 1 최대치
	int		Exp;
	int		Next_Exp;
	int		Money;				//보유한 돈
	int		StatePoint;			//능력치 포인트
	*/

	ReformStateCode( (char *)&lpCharInfo->szModelName2 , 64 );

	ReformStateCode( (char *)&lpCharInfo->Level , sizeof(int) );
	ReformStateCode( (char *)&lpCharInfo->Strength , sizeof(int) );
	ReformStateCode( (char *)&lpCharInfo->Spirit , sizeof(int) );
	ReformStateCode( (char *)&lpCharInfo->Talent , sizeof(int) );
	ReformStateCode( (char *)&lpCharInfo->Dexterity , sizeof(int) );
	ReformStateCode( (char *)&lpCharInfo->Health , sizeof(int) );
	ReformStateCode( (char *)&lpCharInfo->Level , sizeof(int) );


	ReformStateCode( (char *)&lpCharInfo->Accuracy , sizeof(int) );
	ReformStateCode( (char *)&lpCharInfo->Attack_Rating , sizeof(int) );
	ReformStateCode( (char *)lpCharInfo->Attack_Damage , sizeof(int)*2 );
	ReformStateCode( (char *)&lpCharInfo->Attack_Speed , sizeof(int) );
	ReformStateCode( (char *)&lpCharInfo->Shooting_Range , sizeof(int) );
	ReformStateCode( (char *)&lpCharInfo->Critical_Hit , sizeof(int) );

	ReformStateCode( (char *)&lpCharInfo->Defence , sizeof(int) );
	ReformStateCode( (char *)&lpCharInfo->Chance_Block , sizeof(int) );
	ReformStateCode( (char *)&lpCharInfo->Absorption , sizeof(int) );
	ReformStateCode( (char *)&lpCharInfo->Move_Speed , sizeof(int) );
	ReformStateCode( (char *)&lpCharInfo->Sight , sizeof(int) );
	ReformStateCode( (char *)lpCharInfo->Weight , sizeof(short)*2 );

	ReformStateCode( (char *)lpCharInfo->Resistance , sizeof(short)*8 );
	ReformStateCode( (char *)lpCharInfo->Attack_Resistance , sizeof(short)*8 );

	ReformStateCode( (char *)lpCharInfo->Life , sizeof(short)*2 );
	ReformStateCode( (char *)lpCharInfo->Mana , sizeof(short)*2 );
	ReformStateCode( (char *)lpCharInfo->Stamina , sizeof(short)*2 );

	ReformStateCode( (char *)&lpCharInfo->Exp , sizeof(int) );
	ReformStateCode( (char *)&lpCharInfo->Next_Exp , sizeof(int) );
	ReformStateCode( (char *)&lpCharInfo->Money , sizeof(int) );
	ReformStateCode( (char *)&lpCharInfo->StatePoint , sizeof(int) );

#ifdef _MODE_EXP64
	ReformStateCode( (char *)&lpCharInfo->Exp_High , sizeof(int) );
#endif

	return iFormCode;
}


static int ItemFormVersion = 12;
static int ItemFormKey = 0;

//아이템 인증 받기
int ReformItem( sITEMINFO *lpItemInfo )
{

	//신규 인증
	DWORD dwCode;

	// Only server.exe may issue an item a new serial. NetType.hpp defines _W_SERVER for the game
	// too, so the game used to re-serial every item it re-checked (a bought item, for one): the
	// server then no longer knew the item, refused to let it be dropped and stripped it on save.
#ifdef PT_SERVER_EXE
	// 서버측 인증
extern time_t		tServerTime;

	lpItemInfo->ItemHeader.Head = ItemFormKey+GetCurrentTime();

	dwCode = GetRefromItemCode( lpItemInfo , lpItemInfo->ItemHeader.Head );

	lpItemInfo->ItemHeader.dwChkSum = dwCode;
	lpItemInfo->ItemHeader.dwVersion = ItemFormVersion;
	lpItemInfo->dwCreateTime = tServerTime;

	if ( !lpItemInfo->BackUpKey || !lpItemInfo->BackUpChkSum ) {
		lpItemInfo->BackUpKey = lpItemInfo->ItemHeader.Head;
		lpItemInfo->BackUpChkSum = lpItemInfo->ItemHeader.dwChkSum;
	}

	ReformMixItem( lpItemInfo );			//아이템 에이징 인증 받기
	rsReformItem_Server( lpItemInfo );		//아이템 서버 인증만 받기

	lpItemInfo->Temp0 = lpItemInfo->ItemHeader.dwChkSum+lpItemInfo->CODE;

#else
	// 클라이언트측 인증 ( 부분인증 )
	if ( !lpItemInfo->CODE || lpItemInfo->CODE==(sinGG1|sin01) || (lpItemInfo->CODE&sinITEM_MASK1)==(sinPM1&sinITEM_MASK1) ) {
		lpItemInfo->ItemHeader.Head = ItemFormKey+GetCurrentTime();

		dwCode = GetRefromItemCode( lpItemInfo , lpItemInfo->ItemHeader.Head );

		lpItemInfo->ItemHeader.dwChkSum = dwCode;
		lpItemInfo->ItemHeader.dwVersion = ItemFormVersion;
	}
#endif

	ItemFormKey++;

	return TRUE;
}

//아이템 인증 받기 ( 생성시간 보정 )
int ReformItem_AdjustTime( sITEMINFO *lpItemInfo , int TimeAdjust )
{

#ifdef PT_SERVER_EXE		// server.exe only: see ReformItem

	//신규 인증
	DWORD dwCode;

	// 서버측 인증 
extern time_t		tServerTime;

	lpItemInfo->ItemHeader.Head = ItemFormKey+GetCurrentTime();

	dwCode = GetRefromItemCode( lpItemInfo , lpItemInfo->ItemHeader.Head );

	lpItemInfo->ItemHeader.dwChkSum = dwCode;
	lpItemInfo->ItemHeader.dwVersion = ItemFormVersion;
	lpItemInfo->dwCreateTime = tServerTime+TimeAdjust;

	if ( !lpItemInfo->BackUpKey || !lpItemInfo->BackUpChkSum ) {
		lpItemInfo->BackUpKey = lpItemInfo->ItemHeader.Head;
		lpItemInfo->BackUpChkSum = lpItemInfo->ItemHeader.dwChkSum;
	}

	ReformMixItem( lpItemInfo );			//아이템 에이징 인증 받기
	rsReformItem_Server( lpItemInfo );		//아이템 서버 인증만 받기

	lpItemInfo->Temp0 = lpItemInfo->ItemHeader.dwChkSum+lpItemInfo->CODE;

#endif

	ItemFormKey++;

	return TRUE;
}



#ifdef _W_SERVER
//아이템 인증 확인
int CheckItemForm( sITEMINFO *lpItemInfo )
{
	DWORD dwCode;
	//DWORD dwChkCode;
/*
	if ( !lpWareHouse->WareHouseItem[cnt].ItemHeader.dwTime ) {
		//코드 위장
		sRecordItem.sItemInfo.ItemHeader.dwTime = 
			(sRecordItem.sItemInfo.ItemHeader.Head>>2)^(sRecordItem.sItemInfo.ItemHeader.dwChkSum<<2);
	}
*/

	//돈또는 물약일 경우 일반 인증 첵크 사용
	if ( lpItemInfo->CODE==(sinGG1|sin01) || (lpItemInfo->CODE&sinITEM_MASK1)==(sinPM1&sinITEM_MASK1) ) {
		dwCode = GetRefromItemCode( lpItemInfo , lpItemInfo->ItemHeader.Head );
		if ( lpItemInfo->ItemHeader.dwChkSum==dwCode ) return TRUE;
		return FALSE;
	}
/*
	dwChkCode = (lpItemInfo->ItemHeader.Head>>2)^(lpItemInfo->ItemHeader.dwChkSum<<2);
	if ( dwChkCode==lpItemInfo->ItemHeader.dwTime ) {
		//위장코드일때 새로 인증 받기
		dwCode = GetRefromItemCode( lpItemInfo , lpItemInfo->ItemHeader.Head );
		if ( lpItemInfo->ItemHeader.dwChkSum!=dwCode ) return FALSE;

		//서버용 재 인증 아이템
		dwCode = rsGetRefromItemCode( lpItemInfo , lpItemInfo->ItemHeader.dwChkSum );
		dwCode = dwCode ^ ((lpItemInfo->ItemAgingProtect[0]*7)^(lpItemInfo->ItemAgingProtect[1]*9));	//추가 (믹스/에이징) 프로텍트
		dwCode+= lpItemInfo->dwCreateTime>>8;
		dwCode = dwCode ^ lpItemInfo->ItemHeader.Head;


		lpItemInfo->ItemHeader.dwTime = dwCode;

		return 0x100;
	}
*/

	//아이템 서버 인증값 얻기
	dwCode = rsGetItemForm_Server( lpItemInfo );

	if ( lpItemInfo->ItemHeader.dwTime==dwCode ) return TRUE;

	return FALSE;
}

#else
//아이템 인증 확인
int CheckItemForm( sITEMINFO *lpItemInfo )
{
	DWORD dwCode;
/*
	if ( lpItemInfo->ItemHeader.dwVersion==0 ) {
		//구버전 인증 확인 후 새버전으로 재 인증
		dwCode = GetRefromItemCode_Old( lpItemInfo , lpItemInfo->ItemHeader.Head );
		if ( lpItemInfo->ItemHeader.dwChkSum==dwCode ) {
			//구버전 조작 아이템 제거
			if ( lpItemInfo->fAbsorb>=20 || lpItemInfo->Defence>=200 || lpItemInfo->Damage[1]>=40 ) {
				return FALSE;
			}
			//구버전 인증 확인 / 새 인증 발급 
			ReformItem( lpItemInfo );
			return TRUE;
		}
		return FALSE;
	}

	if ( lpItemInfo->ItemHeader.dwVersion==10 ) {
		//구버전 인증 확인 후 새버전으로 재 인증
		dwCode = GetRefromItemCode_V10( lpItemInfo , lpItemInfo->ItemHeader.Head );
		if ( lpItemInfo->ItemHeader.dwChkSum==dwCode ) {
			//구버전 조작 아이템 제거
			if ( lpItemInfo->fAbsorb>=20 || lpItemInfo->Defence>=200 || lpItemInfo->Damage[1]>40 ) {
				return FALSE;
			}
			//구버전 인증 확인 / 새 인증 발급 
			ReformItem( lpItemInfo );
			return TRUE;
		}
		return FALSE;
	}
*/
	dwCode = GetRefromItemCode( lpItemInfo , lpItemInfo->ItemHeader.Head );

	if ( lpItemInfo->ItemHeader.dwChkSum==dwCode ) return TRUE;

	return FALSE;
}
#endif

//아이템 2개를 비교한다
int CompareItems( sITEMINFO *lpItemInfo1 , sITEMINFO *lpItemInfo2 )
{

	if ( lpItemInfo1->ItemHeader.dwVersion!=(DWORD)ItemFormVersion ) CheckItemForm(lpItemInfo1);
	if ( lpItemInfo2->ItemHeader.dwVersion!=(DWORD)ItemFormVersion ) CheckItemForm(lpItemInfo2);

	if ( lpItemInfo1->CODE!=lpItemInfo2->CODE ) return FALSE;
	if ( lpItemInfo1->ItemHeader.Head!=lpItemInfo2->ItemHeader.Head ) return FALSE;
	if ( lpItemInfo1->ItemHeader.dwChkSum!=lpItemInfo2->ItemHeader.dwChkSum ) return FALSE;

	if ( CompareReformItemCode( lpItemInfo1 , lpItemInfo2 )==0 ) 
		return TRUE;

	return FALSE;
}




//캐릭터 정보 인증 받기
int ReformCharForm( smCHAR_INFO *lpCharInfo )
{
	DWORD dwCode;

	dwCode = GetRefromCharState( lpCharInfo , 10 );

	lpCharInfo->RefomCode = dwCode;

	ItemFormKey++;

	return TRUE;
}

//캐릭터 정보 인증 확인
int CheckCharForm( smCHAR_INFO *lpCharInfo )
{
	DWORD dwCode;

	dwCode = GetRefromCharState( lpCharInfo , 10 );

	if ( lpCharInfo->RefomCode==dwCode ) return TRUE;

	return FALSE;
}


//캐릭터 정보 인증 받기
int ReformCharForm()
{
	//lpCurPlayer->smCharInfo.Attack_Damage[1] = 300;
	//lpCurPlayer->smCharInfo.Attack_Damage[0] = 300;

	return ReformCharForm( &lpCurPlayer->smCharInfo );

}

//캐릭터 정보 인증 확인
int CheckCharForm()
{
	int val;

	val = CheckCharForm( &lpCurPlayer->smCharInfo );

	if ( val==FALSE ) {//|| ( VRKeyBuff['0'] && DisconnectFlag==0 )) {

		//해킹 시도 알림
		SendSetHackUser( 81 );

		DisconnectServerCode = 4;
		DisconnectFlag = GetCurrentTime();

	}

	return val;
}

//트레이드 정보 인증 받기
int ReformTrade( void *lpTrade )
{
	DWORD dwCode;

	dwCode = GetRefromTradeCode( (sTRADE *)lpTrade , 84 );

	((sTRADE *)lpTrade)->CheckMoney = dwCode;

	ItemFormKey++;

	return TRUE;
}

//트레이드 정보 인증 확인
int CheckTrade( void *lpTrade )
{
	DWORD dwCode;

	dwCode = GetRefromTradeCode( (sTRADE *)lpTrade , 84 );

	if ( ((sTRADE *)lpTrade)->CheckMoney==(int)dwCode ) return TRUE;

	//해킹 시도한 유저 자동 신고 ( 트레이드 인증 오류 서버에 알림 )
	SendSetHackUser2( 4000 , 0 );

	return FALSE;
}

/*
	short   ItemAgingNum[2];	//0은 에이징+몇 1은 에이징이 되고있나 아닌가?
	short   ItemAgingCount[2];	//0아이템 숙련 카운트 1숙련치 최대 
	short	ItemAgingProtect[2];

//ItemAgingProtect
*/

//아이템 에이징 인증코드
short GetMixItemForm( sITEMINFO *lpItemInfo )
{
	short sCode=0;

#ifdef _W_SERVER

	DWORD dwCode;
	DWORD dwItemKey;

	if ( lpItemInfo->ItemKindCode ) {
		dwItemKey = lpItemInfo->ItemHeader.dwChkSum;
/*
		dwCode = lpItemInfo->ItemKindCode*(
			lpItemInfo->ItemAgingNum[0]*3+lpItemInfo->ItemAgingNum[1]*7+
			lpItemInfo->ItemAgingCount[0]*15+lpItemInfo->ItemAgingCount[1]+17 );
*/

		//에이징 카운터(ItemAgingCount) [1]은 서버측 보안 , [0]은 클라이언트 보안으로 변경
		dwCode = lpItemInfo->ItemKindCode*(
			lpItemInfo->ItemAgingNum[0]*3+lpItemInfo->ItemAgingNum[1]*7+lpItemInfo->ItemAgingCount[1]*17+17 );

		dwCode= dwCode^dwItemKey;

		sCode = (short)((dwCode>>16)^(dwCode&0xFFFF));
	}
#endif

	return sCode;
}

//아이템 에이징 인증 받기
int ReformMixItem( sITEMINFO *lpItemInfo )
{
#ifdef _W_SERVER
	lpItemInfo->ItemAgingProtect[0] = GetMixItemForm( lpItemInfo );
#endif
	return TRUE;
}

//아이템 에이징 인증 확인
int CheckMixItem( sITEMINFO *lpItemInfo )
{

#ifdef _W_SERVER
	if ( GetMixItemForm( lpItemInfo )==lpItemInfo->ItemAgingProtect[0] ) {
		return TRUE;
	}
#else
	return TRUE;
#endif

	return FALSE;
}

//흡수력 보정하여 구하기
float GetItemAbsorb( sITEMINFO *lpItemInfo )
{
	float	fAborb;
	DWORD	dwCode;


	dwCode = lpItemInfo->CODE&sinITEM_MASK2;
	fAborb = lpItemInfo->fAbsorb;

//	if ( dwCode!=sinDA1 && dwCode!=sinDA2 ) 
//		return fAborb;

	if ( dwCode==sinDA1 ) {
		switch( lpItemInfo->CODE ) {
			////////////////// DA1xx ////////////////
			case (sinDA1|sin02):
				fAborb+=0.3f;
				break;
			case (sinDA1|sin03):
				fAborb+=0.6f;
				break;
			case (sinDA1|sin04):
				fAborb+=1.0f;
				break;
			case (sinDA1|sin05):
				fAborb+=1.3f;
				break;

			case (sinDA1|sin06):
				fAborb+=1.6f;
				break;
			case (sinDA1|sin07):
				fAborb+=1.9f;
				break;
			case (sinDA1|sin08):
				fAborb+=2.3f;
				break;
			case (sinDA1|sin09):
				fAborb+=2.6f;
				break;
			case (sinDA1|sin10):
				fAborb+=3.0f;
				break;

			case (sinDA1|sin11):
				fAborb+=3.4f;
				break;
			case (sinDA1|sin12):
				fAborb+=3.7f;
				break;
			case (sinDA1|sin13):
				fAborb+=4.0f;
				break;
			case (sinDA1|sin14):
				fAborb+=4.2f;
				break;
			case (sinDA1|sin15):
				fAborb+=4.5f;
				break;

			case (sinDA1|sin16):
				fAborb+=4.8f;
				break;
			case (sinDA1|sin17):
				fAborb+=5.3f;
				break;
			case (sinDA1|sin18):
				fAborb+=6.0f;
				break;
			case (sinDA1|sin19):
				fAborb+=7.0f;
				break;
			case (sinDA1|sin20):
				fAborb+=8.4f;
				break;
/*
			case (sinDA1|sin21):
				fAborb+=10.0f;
				break;
			case (sinDA1|sin22):
				fAborb+=12.0f;
				break;
*/
		}
	}
	else if ( dwCode==sinDA2 ) {
		switch( lpItemInfo->CODE ) {
			////////////////// DA2xx ////////////////
			case (sinDA2|sin02):
				fAborb+=0.3f;
				break;
			case (sinDA2|sin03):
				fAborb+=0.6f;
				break;
			case (sinDA2|sin04):
				fAborb+=0.9f;
				break;
			case (sinDA2|sin05):
				fAborb+=1.2f;
				break;

			case (sinDA2|sin06):
				fAborb+=1.5f;
				break;
			case (sinDA2|sin07):
				fAborb+=1.8f;
				break;
			case (sinDA2|sin08):
				fAborb+=2.1f;
				break;
			case (sinDA2|sin09):
				fAborb+=2.4f;
				break;
			case (sinDA2|sin10):
				fAborb+=2.7f;
				break;

			case (sinDA2|sin11):
				fAborb+=3.0f;
				break;
			case (sinDA2|sin12):
				fAborb+=3.3f;
				break;
			case (sinDA2|sin13):
				fAborb+=3.6f;
				break;
			case (sinDA2|sin14):
				fAborb+=3.9f;
				break;
			case (sinDA2|sin15):
				fAborb+=4.2f;
				break;

			case (sinDA2|sin16):
				fAborb+=4.5f;
				break;
			case (sinDA2|sin17):
				fAborb+=4.9f;
				break;
			case (sinDA2|sin18):
				fAborb+=5.5f;
				break;
			case (sinDA2|sin19):
				fAborb+=6.8f;
				break;
			case (sinDA2|sin20):
				fAborb+=8.0f;
				break;
/*	// 흡수력 수정
			case (sinDA2|sin21):
				fAborb+=9.5f;
				break;
			case (sinDA2|sin22):
				fAborb+=11.4f;
				break;
*/

		}
	}
//#ifdef _TEST_SERVER
#ifdef _LANGUAGE_ENGLISH_NEXTUP

	else if ( dwCode==sinDB1 ) {// && smConfig.DebugMode ) {
		switch( lpItemInfo->CODE ) {
			////////////////// DB1xx ////////////////
			case (sinDB1|sin01):
				fAborb+=0.0f;
				break;
			case (sinDB1|sin02):
				fAborb+=0.0f;
				break;
			case (sinDB1|sin03):
				fAborb+=0.2f;
				break;
			case (sinDB1|sin04):
				fAborb+=0.3f;
				break;
			case (sinDB1|sin05):
				fAborb+=0.4f;
				break;

			case (sinDB1|sin06):
				fAborb+=0.5f;
				break;
			case (sinDB1|sin07):
				fAborb+=0.6f;
				break;
			case (sinDB1|sin08):
				fAborb+=0.7f;
				break;
			case (sinDB1|sin09):
				fAborb+=0.8f;
				break;
			case (sinDB1|sin10):
				fAborb+=1.1f;
				break;

			case (sinDB1|sin11):
				fAborb+=1.3f;
				break;
			case (sinDB1|sin12):
				fAborb+=1.5f;
				break;
			case (sinDB1|sin13):
				fAborb+=1.7f;
				break;
			case (sinDB1|sin14):
				fAborb+=1.9f;
				break;
			case (sinDB1|sin15):
				fAborb+=2.1f;
				break;

			case (sinDB1|sin16):
				fAborb+=2.3f;
				break;
			case (sinDB1|sin17):
				fAborb+=2.5f;
				break;
			case (sinDB1|sin18):
				fAborb+=2.7f;
				break;
			case (sinDB1|sin19):
				fAborb+=2.9f;
				break;
			case (sinDB1|sin20):
				fAborb+=3.1f;
				break;
/*
			case (sinDB1|sin21):
				fAborb+=3.2f;
				break;
			case (sinDB1|sin22):
				fAborb+=3.3f;
				break;
*/
		}
	}
	else if ( dwCode==sinDG1 ) {//&& smConfig.DebugMode ) {
		switch( lpItemInfo->CODE ) {
			////////////////// DG1xx ////////////////
			case (sinDG1|sin01):
				fAborb+=0.0f;
				break;
			case (sinDG1|sin02):
				fAborb+=0.0f;
				break;
			case (sinDG1|sin03):
				fAborb+=0.1f;
				break;
			case (sinDG1|sin04):
				fAborb+=0.2f;
				break;
			case (sinDG1|sin05):
				fAborb+=0.3f;
				break;

			case (sinDG1|sin06):
				fAborb+=0.4f;
				break;
			case (sinDG1|sin07):
				fAborb+=0.5f;
				break;
			case (sinDG1|sin08):
				fAborb+=0.6f;
				break;
			case (sinDG1|sin09):
				fAborb+=0.7f;
				break;
			case (sinDG1|sin10):
				fAborb+=0.9f;
				break;

			case (sinDG1|sin11):
				fAborb+=1.1f;
				break;
			case (sinDG1|sin12):
				fAborb+=1.3f;
				break;
			case (sinDG1|sin13):
				fAborb+=1.5f;
				break;
			case (sinDG1|sin14):
				fAborb+=1.7f;
				break;
			case (sinDG1|sin15):
				fAborb+=1.9f;
				break;

			case (sinDG1|sin16):
				fAborb+=2.1f;
				break;
			case (sinDG1|sin17):
				fAborb+=2.3f;
				break;
			case (sinDG1|sin18):
				fAborb+=2.5f;
				break;
			case (sinDG1|sin19):
				fAborb+=2.7f;
				break;
			case (sinDG1|sin20):
				fAborb+=2.9f;
				break;
/*
			case (sinDG1|sin21):
				fAborb+=3.2f;
				break;
			case (sinDG1|sin22):
				fAborb+=3.3f;
				break;
*/
		}
	}
	else if ( dwCode==sinDS1 ) {//&& smConfig.DebugMode ) {
		switch( lpItemInfo->CODE ) {
			////////////////// DS1xx ////////////////
			case (sinDS1|sin01):
				fAborb+=0.5f;
				break;
			case (sinDS1|sin02):
				fAborb+=0.6f;
				break;
			case (sinDS1|sin03):
				fAborb+=0.7f;
				break;
			case (sinDS1|sin04):
				fAborb+=0.8f;
				break;
			case (sinDS1|sin05):
				fAborb+=0.9f;
				break;

			case (sinDS1|sin06):
				fAborb+=1.0f;
				break;
			case (sinDS1|sin07):
				fAborb+=1.0f;
				break;
			case (sinDS1|sin08):
				fAborb+=1.1f;
				break;
			case (sinDS1|sin09):
				fAborb+=1.3f;
				break;
			case (sinDS1|sin10):
				fAborb+=1.4f;
				break;

			case (sinDS1|sin11):
				fAborb+=1.5f;
				break;
			case (sinDS1|sin12):
				fAborb+=1.6f;
				break;
			case (sinDS1|sin13):
				fAborb+=1.7f;
				break;
			case (sinDS1|sin14):
				fAborb+=1.8f;
				break;
			case (sinDS1|sin15):
				fAborb+=1.9f;
				break;

			case (sinDS1|sin16):
				fAborb+=2.0f;
				break;
			case (sinDS1|sin17):
				fAborb+=2.1f;
				break;
			case (sinDS1|sin18):
				fAborb+=2.2f;
				break;
			case (sinDS1|sin19):
				fAborb+=2.3f;
				break;
			case (sinDS1|sin20):
				fAborb+=2.4f;
				break;
/*
			case (sinDS1|sin21):
				fAborb+=3.2f;
				break;
			case (sinDS1|sin22):
				fAborb+=3.3f;
				break;
*/
		}
	}
//#endif
#endif
	return	fAborb;
}



#define WAIT_BATTLE_QUIT_TIME		10000

//게임 종료
int QuitGame()
{
	#ifdef _XIGNCODE_CLIENT
	//박재원 - XignCode
	ZCWAVE_Cleanup();
	ZCWAVE_SysExit();
	#endif

	if ( quit ) return TRUE;

	if ( dwBattleTime ) {
		if ( (dwBattleTime+WAIT_BATTLE_QUIT_TIME)>dwPlayTime ) {
			dwBattleQuitTime = dwPlayTime;
			return FALSE;
		}
	}

	// 장별 - 제련창, 제작창 종료 // 석지용 - 믹스쳐 리셋 창 추가
	if ( cWareHouse.OpenFlag || cCraftItem.OpenFlag || cTrade.OpenFlag || cAging.OpenFlag || SmeltingItem.OpenFlag || ManufactureItem.m_OpenFlag || cMixtureReset.OpenFlag) 
	{ 
		dwCloseBoxTime = dwPlayTime+2000;
		return FALSE;
	}

	quit = 1;

	return TRUE;
}

#define CHARINFO_CACHE_MAX		200

smTRNAS_PLAYERINFO	CharInfo_Cache[CHARINFO_CACHE_MAX];
DWORD	dwCharInfo_CacheCode[CHARINFO_CACHE_MAX];
int	CharInfoCacheCnt = 0;

//캐릭터 정보를 캐시데이타로 부터 얻는다
smTRNAS_PLAYERINFO	*GetCharInfoCache( char *lpData )
{
	DWORD	dwChkCode;
	int cnt;
	smPLAYDATA *lpPlayData;
	smTRANS_PLAYDATA	*lpTransPlayData;
	smTRANS_PLAYBUFF	*lpTransPlayBuff;
	int	RecvCode;
	smTRNAS_PLAYERINFO	*lpTransPlayInfo;

	RecvCode = ((int *)lpData)[1];

	dwChkCode = 0;

	switch( RecvCode ) {
	case smTRANSCODE_PLAYDATA1:
		lpPlayData = (smPLAYDATA *)lpData;
		dwChkCode = lpPlayData->dwAutoCharCode;
		break;
	case smTRANSCODE_TRANSPLAYDATA:
		lpTransPlayData = (smTRANS_PLAYDATA *)lpData;
		lpTransPlayBuff = (smTRANS_PLAYBUFF *)(lpData + sizeof(smTRANS_PLAYDATA) );
		if ( lpTransPlayData->PlayBuffCnt>0 )
			dwChkCode = lpTransPlayData->dwAutoCharCode;
		break;
	}

	if ( !dwChkCode ) return NULL;

	for(cnt=0;cnt<CharInfoCacheCnt;cnt++) {
		if ( dwChkCode==dwCharInfo_CacheCode[cnt] ) {
			//캐시 데이타 찾음
			lpTransPlayInfo = &CharInfo_Cache[cnt];

			switch( RecvCode ) {
			case smTRANSCODE_PLAYDATA1:
				//게임 진행 수신 ( 저질 데이타 )
				lpTransPlayInfo->x = lpPlayData->x;
				lpTransPlayInfo->y = lpPlayData->y;
				lpTransPlayInfo->z = lpPlayData->z;
				lpTransPlayInfo->ax = lpPlayData->x;
				lpTransPlayInfo->ay = lpPlayData->y;
				lpTransPlayInfo->az = lpPlayData->z;
				lpTransPlayInfo->dwObjectSerial = lpPlayData->dwObjectSerial;
				lpTransPlayInfo->smCharInfo.dwObjectSerial = lpPlayData->dwObjectSerial;
				break;

			case smTRANSCODE_TRANSPLAYDATA:
				//게임 진행 수신 ( 정밀 데이타 )
				lpTransPlayBuff = (smTRANS_PLAYBUFF *) ( ((char *)lpTransPlayData)+sizeof(smTRANS_PLAYDATA) );
				lpTransPlayBuff += lpTransPlayData->PlayBuffCnt-1;

				lpTransPlayInfo->x = lpTransPlayBuff->PlayBuff.x;
				lpTransPlayInfo->y = lpTransPlayBuff->PlayBuff.y;
				lpTransPlayInfo->z = lpTransPlayBuff->PlayBuff.z;

				lpTransPlayInfo->ax = lpTransPlayBuff->PlayBuff.angle[0];
				lpTransPlayInfo->ay = lpTransPlayBuff->PlayBuff.angle[1];
				lpTransPlayInfo->az = lpTransPlayBuff->PlayBuff.angle[2];
				lpTransPlayInfo->dwObjectSerial = lpTransPlayData->dwObjectSerial;
				lpTransPlayInfo->smCharInfo.dwObjectSerial = lpTransPlayData->dwObjectSerial;
				break;
			}

			lpTransPlayInfo->smCharInfo.Life[0] = lpTransPlayInfo->smCharInfo.Life[1];
			return lpTransPlayInfo;
		}
	}

	return NULL;
}

//캐릭터 정보를 캐시데이타에 저장
int SaveCharInfoCache( smTRNAS_PLAYERINFO *lpTransPlayInfo )
{
	int cnt;
	DWORD	dwCode;

	dwCode = GetSpeedSum( lpTransPlayInfo->smCharInfo.szName );

	//기존 데이타 비교
	for(cnt=0;cnt<CharInfoCacheCnt;cnt++) {
		if ( dwCode==dwCharInfo_CacheCode[cnt] ) return NULL;
	}

	//새로 저장
	dwCharInfo_CacheCode[CharInfoCacheCnt] = dwCode;
	memcpy( &CharInfo_Cache[CharInfoCacheCnt] , lpTransPlayInfo , sizeof(smTRNAS_PLAYERINFO) );
	CharInfoCacheCnt++;

	return TRUE;
}

//파티 가입여부 확인
int	RequestPartyPlayers( smCHAR	*lpChar )
{
	int cnt,cnt2;


	if ( lpChar ) {
		lpChar->PartyFlag = FALSE;
		//지정된 캐릭터 검사 하여 적용
		for(cnt=0;cnt<InterfaceParty.PartyMemberCount;cnt++) {
			if ( InterfaceParty.PartyMember[cnt].ChrCode==lpChar->dwObjectSerial ) {
				lpChar->PartyFlag = TRUE;
				return TRUE;
			}
		}
	}
	else {
		//전체 캐릭을 검사하여 적용
		for( cnt2=0;cnt2<OTHER_PLAYER_MAX;cnt2++) {
			if ( chrOtherPlayer[cnt2].Flag && chrOtherPlayer[cnt2].dwObjectSerial ) {
				chrOtherPlayer[cnt2].PartyFlag = FALSE;
				for(cnt=0;cnt<InterfaceParty.PartyMemberCount;cnt++) {
					if ( InterfaceParty.PartyMember[cnt].ChrCode==chrOtherPlayer[cnt2].dwObjectSerial ) {
						chrOtherPlayer[cnt2].PartyFlag = TRUE;
						break;
					}
				}
			}
		}
	}

	return FALSE;
}


smCHAR	chrPartyPlayer;

//파티 플레이어 얼굴 오픈
int OpenPartyPlayer( char *szName , char *szModelFile1 , char *szModelFile2 )
{
	//아이템을 새 쓰레드에 통하여 로드 한다
	AddLoaderPattern( &chrPartyPlayer , szModelFile1 , szModelFile2 );

	chrPartyPlayer.SetMotionFromCode( CHRMOTION_STATE_STAND );
	chrPartyPlayer.ActionPattern = 101;
	chrPartyPlayer.Flag = TRUE;
	chrPartyPlayer.lpStage = 0;
	chrPartyPlayer.OnStageField = lpCurPlayer->OnStageField;

	lstrcpy( chrPartyPlayer.smCharInfo.szName , szName );

	OpenEachPlayer( &chrPartyPlayer );

	return TRUE;
}

//extern HIMC hImc;

char	szLastWhisperName[32] = { 0 };
int		LastWhisperLen = 0;

//파티원 귓말 보내기
int	WhisperPartyPlayer( char *szName )
{
	char szBuff[128];
	int len;

	lstrcpy( szLastWhisperName , szName );

	chatlistSPEAKERflagChg( FALSE );		//클랜 채팅 버튼 끄기
	cInterFace.ChatFlag = TRUE;
	hFocusWnd = hTextWnd;
	wsprintf( szBuff, "/:%s " , szName );
	len = lstrlen(szBuff);
	LastWhisperLen = len;
	SetWindowText( hFocusWnd, szBuff );
	SendMessage( hFocusWnd , EM_SETSEL , len , len );
	//ImmSetConversionStatus( hImc , IME_CMODE_NATIVE,IME_CMODE_NATIVE );

	SetIME_Mode( TRUE );

	return TRUE;
}

//클랜원 채팅 보내기
int	SetClanChatting()
{
	char szBuff[128];
	int len;

	szLastWhisperName[0] = 0;
	hFocusWnd = hTextWnd;
	cInterFace.ChatFlag = TRUE;
	wsprintf( szBuff, "/CLAN> " );
	len = lstrlen(szBuff);
	LastWhisperLen = len;
	SetWindowText( hFocusWnd, szBuff );
	SendMessage( hFocusWnd , EM_SETSEL , len , len );

	SetIME_Mode( TRUE );

	return TRUE;
}

//클랜 채팅 모드 설정
int IsClanChatting()
{
	if ( lpCurPlayer->smCharInfo.ClassClan ) {
		if ( !chatlistSPEAKERflag() ) {
			chatlistSPEAKERflagChg( TRUE );		//클랜 채팅 버튼 켜기
			SetClanChatting();
			return TRUE;
		}
		else {
			chatlistSPEAKERflagChg( FALSE );		//클랜 채팅 버튼 켜기
			SetWindowText( hFocusWnd, "" );
			return TRUE;
		}
		szLastWhisperName[0] = 0;
	}

	return FALSE;
}


int	PlayAttackCount = 0;			//공격 받은 카운터
int	MotionAttackCount = 0;			//나를 공격한 모션들 카운터


//공격 받은 패킷 카운터
int	AttackRecvCountUp()
{
	PlayAttackCount++;
	return PlayAttackCount;
}

//공격 받은 모션 카운터
int AttackMotionCountUp()
{
	MotionAttackCount++;
	if ( MotionAttackCount>30 ) {
		//공격 비율 첵크
		CheckPlayAttackCount();
	}

	return MotionAttackCount;
}

//공격 비율 첵크
int	CheckPlayAttackCount()
{
	//30번 공격 받는 동안 공격 패킷이 3 이하일 경우 문제있다고 판단함
	if ( MotionAttackCount>30 && PlayAttackCount<=3 ) {
		//해킹 시도한 유저 자동 신고
		//SendSetHackUser2( 1500 , MotionAttackCount-PlayAttackCount );
	}

	MotionAttackCount = 0;
	PlayAttackCount = 0;

	return TRUE;
}

struct	sCOPY_ITEM_INFO	{
	DWORD	dwItemCode;
	int		ChkSum;
	int		Header;
};

sCOPY_ITEM_INFO	sCopyItemList[] = {
/*
#include "SrcCopyItems\\공병부코드.h"
#include "SrcCopyItems\\마천사코드.h"
#include "SrcCopyItems\\Dragon_신검 코드.h"

	//클레이머 [파이터특 13-24,46명중 [믹스] - 그리브 아이템중 - 복사본 ( 당진경찰서 )
	{	sinWS2|sin11,	481819,		1127932		},
*/
	{	0,0,0	}

};

//등록된 불량 아이템 검사
int	CheckCopyItem( sITEMINFO *lpItem )
{
	int cnt;

	cnt = 0;

	if ( !lpItem->ItemName[0] )	return FALSE;			//껍데기 아이템 (양손무기)
	if ( lpItem->SpecialItemFlag[0] )	return FALSE;		//이미 등록된 값이 있음

	if ( !lpItem->BackUpKey || !lpItem->BackUpChkSum ) {
		lpItem->BackUpKey = lpItem->ItemHeader.Head;
		lpItem->BackUpChkSum = lpItem->ItemHeader.dwChkSum;
	}

	while(1) {
		if ( sCopyItemList[cnt].dwItemCode==0 ) break;


		if ( sCopyItemList[cnt].dwItemCode==lpItem->CODE ) {
			if ( sCopyItemList[cnt].ChkSum==(int)lpItem->ItemHeader.dwChkSum &&
				sCopyItemList[cnt].Header==(int)lpItem->ItemHeader.Head ) {

				lpItem->SpecialItemFlag[0] = CHECK_COPY_ITEM;
				return TRUE;
			}
			if ( sCopyItemList[cnt].ChkSum==(int)lpItem->BackUpChkSum &&
				sCopyItemList[cnt].Header==(int)lpItem->BackUpKey ) {

				lpItem->SpecialItemFlag[0] = CHECK_COPY_ITEM;
				return TRUE;
			}
		}
		cnt++;
	}


	return FALSE;
}


//얼굴 변경한거 적용
int SetJobFace( char *szModelName )
{

	lstrcpy( lpCurPlayer->smCharInfo.szModelName2 , szModelName );
	AddLoaderPattern( lpCurPlayer , lpCurPlayer->smCharInfo.szModelName , lpCurPlayer->smCharInfo.szModelName2 );
	lpCurPlayer->smCharInfo.bUpdateInfo[0]++;
	ReformCharForm();	//캐릭터 정보 인증 받기

	//서버에 변경데이타 송부
	SendSetObjectSerial( lpCurPlayer->dwObjectSerial );

	return TRUE;
}


//char *szTempPikeFaceName[2][3] = {
/*
char **szFaceName[5] = {
	szTempFighterFaceName,
	szTempFighterFaceName,
	szTempMechFaceName,
	szTempArcherFaceName,
	szTempPikeFaceName
};
*/

#define	_CHANGE_JOB_COUNT_MAX		FACE_MODEL_MAX
#define	_FACE_COUNT_MAX				HEAREPOTION_MAX

//모델이 맞는지 확인
int	CheckUserModel( smCHAR_INFO *lpCharInfo )
{
	int cnt,cnt2,len;
	char	szBuff[64];
	char	*lpBuff;


	lpCharInfo->szModelName[31] = 0;
	lpCharInfo->szModelName2[31] = 0;

	len = lstrlen( lpCharInfo->szModelName );
	lstrcpy( szBuff , lpCharInfo->szModelName );
	for(cnt=len-1;cnt>=0;cnt--) {
		if ( szBuff[cnt]=='\\' ) {
			szBuff[cnt+1] = 0;
			break;
		}
	}

	if ( lstrcmpi( szBuff , "char\\tmABCD\\" )!=0 ) return FALSE;
	lpBuff = &lpCharInfo->szModelName[len-3];
	if ( lstrcmpi( lpBuff , "ini" )!=0 ) return FALSE;

	//직업별 얼굴 확인: the head must be one of the class's heads (playmodel.h). A fifth rank
	// head is its fourth rank one with "c.inf" made "d.inf" (ChangeModelSkin), so check that.
	{
		char *(*lpTable)[HEAREPOTION_MAX] = GetClassFaceTable( lpCharInfo->JOB_CODE );
		char szHead[64];

		if ( !lpTable ) return FALSE;

		if ( !lpCharInfo->szModelName2[0] ) {
			//얼굴없음 아무얼굴이나 설정해주자!
			lstrcpy( lpCharInfo->szModelName2 , lpTable[0][0] );
			return TRUE;
		}

		lstrcpyn( szHead , lpCharInfo->szModelName2 , sizeof(szHead) );
		len = lstrlen( szHead );
		if ( len>5 && lstrcmpi( szHead+len-5 , "d.inf" )==0 )
			szHead[len-5] = 'c';

		for(cnt=0;cnt<_CHANGE_JOB_COUNT_MAX;cnt++) {
			for(cnt2=0;cnt2<_FACE_COUNT_MAX;cnt2++) {
				if ( lstrcmpi( lpTable[cnt][cnt2] , szHead )==0 )
					return TRUE;
			}
		}
		return FALSE;
	}

#if 0	// the per-class version of the above (eight classes, no fifth rank heads)
	switch( lpCharInfo->JOB_CODE ) {
	case 1:
		for(cnt=0;cnt<_CHANGE_JOB_COUNT_MAX;cnt++) {
			for(cnt2=0;cnt2<_FACE_COUNT_MAX;cnt2++) {
				if ( lstrcmpi( szModel_FighterFaceName[cnt][cnt2] , lpCharInfo->szModelName2 )==0 ) {
					return TRUE;
				}
			}
		}
		if ( !lpCharInfo->szModelName2[0] ) {
			//얼굴없음 아무얼굴이나 설정해주자!
			lstrcpy( lpCharInfo->szModelName2 , szModel_FighterFaceName[0][0] );
			return TRUE;
		}
		break;
	case 2:
		for(cnt=0;cnt<_CHANGE_JOB_COUNT_MAX;cnt++) {
			for(cnt2=0;cnt2<_FACE_COUNT_MAX;cnt2++) {
				if ( lstrcmpi( szModel_MechFaceName[cnt][cnt2] , lpCharInfo->szModelName2 )==0 ) {
					return TRUE;
				}
			}
		}
		if ( !lpCharInfo->szModelName2[0] ) {
			//얼굴없음 아무얼굴이나 설정해주자!
			lstrcpy( lpCharInfo->szModelName2 , szModel_MechFaceName[0][0] );
			return TRUE;
		}
		break;
	case 3:
		for(cnt=0;cnt<_CHANGE_JOB_COUNT_MAX;cnt++) {
			for(cnt2=0;cnt2<_FACE_COUNT_MAX;cnt2++) {
				if ( lstrcmpi( szModel_ArcherFaceName[cnt][cnt2] , lpCharInfo->szModelName2 )==0 ) {
					return TRUE;
				}
			}
		}
		if ( !lpCharInfo->szModelName2[0] ) {
			//얼굴없음 아무얼굴이나 설정해주자!
			lstrcpy( lpCharInfo->szModelName2 , szModel_ArcherFaceName[0][0] );
			return TRUE;
		}

		break;
	case 4:
		for(cnt=0;cnt<_CHANGE_JOB_COUNT_MAX;cnt++) {
			for(cnt2=0;cnt2<_FACE_COUNT_MAX;cnt2++) {
				if ( lstrcmpi( szModel_PikeFaceName[cnt][cnt2] , lpCharInfo->szModelName2 )==0 ) {
					return TRUE;
				}
			}
		}
		if ( !lpCharInfo->szModelName2[0] ) {
			//얼굴없음 아무얼굴이나 설정해주자!
			lstrcpy( lpCharInfo->szModelName2 , szModel_PikeFaceName[0][0] );
			return TRUE;
		}
		break;

	case 0:
		return TRUE;
	case 5:	//아탈란타
		for(cnt=0;cnt<_CHANGE_JOB_COUNT_MAX;cnt++) {
			for(cnt2=0;cnt2<_FACE_COUNT_MAX;cnt2++) {
				if ( lstrcmpi( szModel_AtalantaFaceName[cnt][cnt2] , lpCharInfo->szModelName2 )==0 ) {
					return TRUE;
				}
			}
		}
		if ( !lpCharInfo->szModelName2[0] ) {
			//얼굴없음 아무얼굴이나 설정해주자!
			lstrcpy( lpCharInfo->szModelName2 , szModel_AtalantaFaceName[0][0] );
			return TRUE;
		}
		break;
	case 6:	//나이트
		for(cnt=0;cnt<_CHANGE_JOB_COUNT_MAX;cnt++) {
			for(cnt2=0;cnt2<_FACE_COUNT_MAX;cnt2++) {
				if ( lstrcmpi( szModel_KnightFaceName[cnt][cnt2] , lpCharInfo->szModelName2 )==0 ) {
					return TRUE;
				}
			}
		}
		if ( !lpCharInfo->szModelName2[0] ) {
			//얼굴없음 아무얼굴이나 설정해주자!
			lstrcpy( lpCharInfo->szModelName2 , szModel_KnightFaceName[0][0] );
			return TRUE;
		}
		break;
	case 7:	//메지션
		for(cnt=0;cnt<_CHANGE_JOB_COUNT_MAX;cnt++) {
			for(cnt2=0;cnt2<_FACE_COUNT_MAX;cnt2++) {
				if ( lstrcmpi( szModel_Magican[cnt][cnt2] , lpCharInfo->szModelName2 )==0 ) {
					return TRUE;
				}
			}
		}
		if ( !lpCharInfo->szModelName2[0] ) {
			//얼굴없음 아무얼굴이나 설정해주자!
			lstrcpy( lpCharInfo->szModelName2 , szModel_Magican[0][0] );
			return TRUE;
		}
		break;
	case 8:	//프리스티스
		for(cnt=0;cnt<_CHANGE_JOB_COUNT_MAX;cnt++) {
			for(cnt2=0;cnt2<_FACE_COUNT_MAX;cnt2++) {
				if ( lstrcmpi( szModel_PristessFaceName[cnt][cnt2] , lpCharInfo->szModelName2 )==0 ) {
					return TRUE;
				}
			}
		}
		if ( !lpCharInfo->szModelName2[0] ) {
			//얼굴없음 아무얼굴이나 설정해주자!
			lstrcpy( lpCharInfo->szModelName2 , szModel_PristessFaceName[0][0] );
			return TRUE;
		}
		break;
	}

	return FALSE;
#endif
}


//집업 변경 얼굴 변화
int ChangeJobFace()
{
	int cnt1;
	//int cnt2;

	cnt1 = 0;

	StartSkill( lpCurPlayer->pX,lpCurPlayer->pY,lpCurPlayer->pZ, 0,0,0, SKILL_UP1 );
	SkillPlaySound( SKILL_SOUND_LEARN , lpCurPlayer->pX,lpCurPlayer->pY,lpCurPlayer->pZ );			//스킬 효과음

	CheckCharForm();
/*
	for( cnt2=0;cnt2<3;cnt2++) {
		if ( lstrcmpi(	lpCurPlayer->smCharInfo.szModelName2 , szTempPikeFaceName[cnt1][cnt2] )==0 ) {
			cnt1++;
			SetJobFace( szTempPikeFaceName[cnt1][cnt2] );
			return TRUE;
		}
		if ( lstrcmpi(	lpCurPlayer->smCharInfo.szModelName2 , szTempArcherFaceName[cnt1][cnt2] )==0 ) {
			cnt1++;
			SetJobFace( szTempArcherFaceName[cnt1][cnt2] );
			return TRUE;
		}
		if ( lstrcmpi(	lpCurPlayer->smCharInfo.szModelName2 , szTempMechFaceName[cnt1][cnt2] )==0 ) {
			cnt1++;
			SetJobFace( szTempMechFaceName[cnt1][cnt2] );
			return TRUE;
		}
		if ( lstrcmpi(	lpCurPlayer->smCharInfo.szModelName2 , szTempFighterFaceName[cnt1][cnt2] )==0 ) {
			cnt1++;
			SetJobFace( szTempFighterFaceName[cnt1][cnt2] );
			return TRUE;
		}
	}
*/

	ChangeModelSkin( lpCurPlayer , 0 , JOB_FACE(lpCurPlayer->smCharInfo.ChangeJob) , 0 , 0 );			//전업얼굴
	lpCurPlayer->smCharInfo.bUpdateInfo[0]++;
	ReformCharForm();
	SendSetObjectSerial( lpCurPlayer->dwObjectSerial );

	return FALSE;
}


////////////////////////////// 에펙트 예약 시스템 ///////////////////////////////

#define	TIME_EFFECT_BUFF_MAX	16

struct sTIME_EFFECT_BUFF {
	DWORD	dwCode;
	DWORD	dwTime;
	int		x,y,z;
	smCHAR	*lpChar;
};

sTIME_EFFECT_BUFF TimeEffectBuff[TIME_EFFECT_BUFF_MAX];

//예약 이펙트 초기화
int InitTimeEffect()
{
	int cnt;
	for(cnt=0;cnt<TIME_EFFECT_BUFF_MAX;cnt++) {
		TimeEffectBuff[cnt].dwCode = 0;
	}
	
	return TRUE;
}

//이펙트 예약 추가
int AddTimeEffect( DWORD EffectCode , DWORD dwTime , int x, int y, int z , smCHAR *lpChar )
{
	int cnt;
	for(cnt=0;cnt<TIME_EFFECT_BUFF_MAX;cnt++) {
		if ( !TimeEffectBuff[cnt].dwCode ) {
			TimeEffectBuff[cnt].dwCode = EffectCode;
			TimeEffectBuff[cnt].dwTime = dwTime;
			TimeEffectBuff[cnt].x = x;
			TimeEffectBuff[cnt].y = y;
			TimeEffectBuff[cnt].z = z;
			TimeEffectBuff[cnt].lpChar = lpChar;
			return TRUE;
		}
	}
	return FALSE;
}

int AddTimeEffect( DWORD EffectCode , DWORD dwTime , int x, int y, int z  )
{
	return AddTimeEffect(EffectCode,dwTime,x,y,z,0);
}

//예약 이펙트 실행
int TimeEffectMain()
{
	int cnt;
	for(cnt=0;cnt<TIME_EFFECT_BUFF_MAX;cnt++) {
		if ( TimeEffectBuff[cnt].dwCode && TimeEffectBuff[cnt].dwTime<dwPlayTime ) {

			switch( TimeEffectBuff[cnt].dwCode ) {
				case SKILL_RAGE_OF_ZECRAM_HIT2:
					//레이지 오브 제크람 이펙트
					StartSkill( TimeEffectBuff[cnt].x,TimeEffectBuff[cnt].y,TimeEffectBuff[cnt].z, 0,0,0, SKILL_RAGE_OF_ZECRAM_HIT2 );
					SkillPlaySound( SKILL_SOUND_SKILL_RAGE_OF_ZECRAM2 , TimeEffectBuff[cnt].x,TimeEffectBuff[cnt].y,TimeEffectBuff[cnt].z );
					SetDynLight( TimeEffectBuff[cnt].x,TimeEffectBuff[cnt].y,TimeEffectBuff[cnt].z , 100, 0,0,0, 500 );
					break;

				case smTRANSCODE_SUCCESS_EVATION:
					//추가회피 클라이언트에 시간차 출력
					cInterFace.SetStringEffect(HA_EVASION);
					break;

				case SKILL_EFFECT_FIREFLOWER:
					if ( TimeEffectBuff[cnt].lpChar && TimeEffectBuff[cnt].lpChar->Flag ) {
						ParkAssaParticle_FireCracker( TimeEffectBuff[cnt].lpChar , TimeEffectBuff[cnt].x );
						esPlaySound( 23+(rand()%3) , GetDistVolume(TimeEffectBuff[cnt].lpChar->pX, TimeEffectBuff[cnt].lpChar->pY, TimeEffectBuff[cnt].lpChar->pZ) );
					}
					break;
			}

			TimeEffectBuff[cnt].dwCode = 0;
		}
	}
	return TRUE;
}

#define		ROT_PLAYER_MAX		128
#define		ROT_PLAYER_MIN		80

smCHAR	*lpChrPlayers[ROT_PLAYER_MAX];


//회전 플레이어 포인터 초기화
smCHAR *InitRotPlayer()
{
	int cnt;
	for(cnt=0;cnt<ROT_PLAYER_MAX;cnt++)
		lpChrPlayers[cnt] = 0;

	return SelectRotPlayer( 0 );
}

//회전 플레이어 포인터 종료
int	CloseRotPlayer()
{
	int cnt;
	for(cnt=0;cnt<ROT_PLAYER_MAX;cnt++)
		if ( lpChrPlayers[cnt] ) {
			delete	lpChrPlayers[cnt];
			lpChrPlayers[cnt] = 0;
		}

	return TRUE;
}

//회전 플레이어 포인터 선택
smCHAR *SelectRotPlayer( smCHAR *lpCurrentChar )
{
	int rnd;
	smCHAR *lpChar;

	rnd = rand()%ROT_PLAYER_MIN;
	lpChar = lpChrPlayers[rnd];
	if ( !lpChar ) {
		lpChar = new smCHAR;
		lpChrPlayers[rnd] = lpChar;
	}

	if ( !lpChar ) return NULL;


	if ( lpCurrentChar && lpCurrentChar!=lpChar ) {
		memcpy( lpChar , lpCurrentChar , sizeof(smCHAR) );
	}

	if ( !lpCurrentChar && lpChar ) {
		ZeroMemory( lpChar , sizeof(smCHAR) );
	}

	return lpChar;
}


///////////////////////////// 메모리 펑션 2차 감시 //////////////////////////////////
/*
DWORD	dwKeepMemStartPoint;
DWORD	dwKeepMemCount;
DWORD	dwKeepMemSrcPoint;
DWORD	dwKeepLen;

int	InitKeepMemFunc()
{
	BYTE	*lpFunc;
	DWORD	cnt;
	DWORD	PointMin,PointMax;
	//DWORD	dwCode,dwCnt;
	DWORD	dwLen;
	DWORD	*lpdwFunc;
	BYTE	JmpCode = 9;
	DWORD	dwHeaderPos = 0x30;
	DWORD	dwCodeSeg;
	DWORD	dwCodeAddr;

	DWORD dwStartFunc = (DWORD)InitKeepMemFunc; 

	JmpCode+=dwAsmJmpCode;

	lpFunc = (BYTE *)dwStartFunc;

	dwCodeSeg = dwStartFunc&0xFFF00000;
	dwCodeAddr = dwStartFunc&0x000FFFFF;

	lpdwFunc = (DWORD *)(dwCodeSeg+dwHeaderPos+0xC);
	lpdwFunc = (DWORD *)(lpdwFunc[0]+dwCodeSeg);
	PointMax = lpdwFunc[0x4B]+dwCodeSeg;
	PointMin = dwCodeSeg+0x1000;

	dwLen = PointMax-PointMin;

	lpFuncBackupBuff = new char[dwLen];
	memcpy( lpFuncBackupBuff , (void *)PointMin , dwLen );

	dwLen>>=2;
	dwKeepLen = dwLen;

	dwKeepMemCount = 0xcccccccc;

	for(cnt=0;cnt<dwLen;cnt++) {
		((DWORD *)lpFuncBackupBuff)[cnt] ^= dwKeepMemCount;
	}

	dwKeepMemStartPoint = PointMin ^dwKeepMemCount;
	dwKeepMemSrcPoint = (DWORD)lpFuncBackupBuff;
	dwKeepMemSrcPoint ^= dwKeepMemCount;

	lpFuncBackupBuff = (char *)dwAsmJmpCode;

	return TRUE;
}

//메모리 첵크 종료
int CloseKeepMem()
{
	if ( lpFuncBackupBuff ) {
		lpFuncBackupBuff = (char *)(dwKeepMemSrcPoint^0xcccccccc);
		delete lpFuncBackupBuff;
		lpFuncBackupBuff = 0;
	}

	return TRUE;
}

//메모리 첵크
int	CheckKeepMem(DWORD ChkCount )
{
	DWORD	*lpFunc;
	DWORD	*lpFunc2;
	DWORD	dwCnt;
	DWORD	cnt;
	DWORD	ccCode = 0xcccccccc;
	DWORD	RoleCount;
	DWORD	dwCode;
	DWORD	dwTotal;
	DWORD	dwChkCnt;

	lpFunc = (DWORD *)(dwKeepMemStartPoint^ccCode);
	dwCnt = dwKeepMemCount^ccCode;

	lpFunc2 = (DWORD *)(dwKeepMemSrcPoint^ccCode);


	RoleCount = 0;

	if ( !ChkCount ) {
		dwChkCnt = 0xFFFFFFFF;
	}
	else {
		dwChkCnt = ChkCount;
	}

	dwTotal = 0;

	if ( ChkCount ) {
		for( cnt=dwCnt;cnt<dwKeepLen;cnt++ ) {
			dwCode = lpFunc2[cnt]^ccCode;
			if ( lpFunc[cnt]!=dwCode ) {
				Code_VramClear();
				return FALSE;
			}
			if ( RoleCount>=dwChkCnt ) break;
			RoleCount++;
		}
	}
	else {
		for( cnt=dwCnt;cnt<dwKeepLen;cnt++ ) {
			dwCode = lpFunc2[cnt]^ccCode;
			if ( lpFunc[cnt]!=dwCode ) {
				dwMemError = 111;
				dwMemError ^= dwPlayTime;
				Code_VramClear();
				return FALSE;
			}
			dwTotal += (dwCode*((cnt&0x3)+1));
			if ( RoleCount>=dwChkCnt ) break;
			RoleCount++;
		}
	}

	if ( cnt==dwKeepLen ) {
		dwCnt = 0;
		RoleCount = 0;
	}

	dwKeepMemCount = (dwCnt+RoleCount)^ccCode;

	if ( !ChkCount )
		return dwTotal;

	return TRUE;
}


//메모리 전체 첵크
int	CheckKeepMemFull()
{
	DWORD dwCode1, dwFnTimer , Param3;
	DWORD cnt;
	DWORD dwCode;
	smTRANS_COMMAND	TransCommand;

	dwCode = CheckKeepMem( 0 );

	Code_VramLoad( &dwCode1 , &dwFnTimer , &Param3 );

	cnt = dwCode1 ^ ( (dwFnTimer<<16)|(dwFnTimer&0xFFFF) );

	if ( smWsockDataServer ) {
		TransCommand.code = smTRANSCODE_CHECK_NETSTATE2;
		TransCommand.size = sizeof( smTRANS_COMMAND );
		TransCommand.WParam = 0;
		TransCommand.LParam = 0;
		TransCommand.SParam = cnt;
		TransCommand.EParam = 0;
		smWsockDataServer->Send2( (char *)&TransCommand , TransCommand.size , TRUE );
	}

	return TRUE;
}
*/

//시작 펑션
int	PlaySub_EndFunc()
{


	return TRUE;
}

//DWORD	dwPlaySub_CheckResult = 1834343222;


int	Check_PlaySubReleaseFunc()
{
	DWORD	dwStartFunc;
	DWORD	dwEndFunc;
	DWORD	*lpStartFunc;
	int		cnt,len;
	DWORD	dwChkSum;

	dwStartFunc = (DWORD)Code_VRamBuffOpen;
	dwEndFunc = (DWORD)Check_PlaySubReleaseFunc;
	len = (dwEndFunc-dwStartFunc)>>2;

	dwChkSum = 0;

	lpStartFunc = (DWORD *)dwStartFunc;

	if ( len<0 ) return FALSE;

	for( cnt=0;cnt<len;cnt++ ) {
		dwChkSum+= (lpStartFunc[cnt]*((cnt&0x3FF)+1));
	}

	dwPlaySub_CheckResult = dwChkSum;

	if ( PLAYSUB_CHECKRESULT!=dwChkSum ) {
/*
		FILE	*fp;
		char	szBuff[256];

		fp = fopen( "playsub_chk.txt" , "wb" );
		if ( fp ) {
			wsprintf( szBuff,  "시작( %d ) 끝( %d )  결과 ( %d )\r\n" , dwStartFunc , dwEndFunc , dwChkSum );
			fwrite( szBuff , lstrlen(szBuff),1,fp);


			wsprintf( szBuff,  "Code_VRamBuffOpen - %d\r\n" , Code_VRamBuffOpen );
			fwrite( szBuff , lstrlen(szBuff),1,fp);
			wsprintf( szBuff,  "Check_CodeSafe - %d\r\n" , Check_CodeSafe );
			fwrite( szBuff , lstrlen(szBuff),1,fp);
			wsprintf( szBuff,  "CheckTrade - %d\r\n" , CheckTrade );
			fwrite( szBuff , lstrlen(szBuff),1,fp);
			wsprintf( szBuff,  "CheckKeepMemFull - %d\r\n" , CheckKeepMemFull );
			fwrite( szBuff , lstrlen(szBuff),1,fp);
			wsprintf( szBuff,  "Check_PlaySubReleaseFunc - %d\r\n" , Check_PlaySubReleaseFunc );
			fwrite( szBuff , lstrlen(szBuff),1,fp);

			fclose(fp);
		}
*/
		return dwChkSum;
	}

	return NULL;
}



sCHK_POTION_COUNT	sPotionTotalCount;
int	tXorPotionCount = 0;

//물약갯수 기록 반전
int	XorCheckPotion( sCHK_POTION_COUNT *ChkPotion )
{
	ChkPotion->Pl101 ^= tXorPotionCount;
	ChkPotion->Pl102 ^= tXorPotionCount;
	ChkPotion->Pl103 ^= tXorPotionCount;
	ChkPotion->Pl104 ^= tXorPotionCount;

	ChkPotion->Pm101 ^= tXorPotionCount;
	ChkPotion->Pm102 ^= tXorPotionCount;
	ChkPotion->Pm103 ^= tXorPotionCount;
	ChkPotion->Pm104 ^= tXorPotionCount;

	ChkPotion->Ps101 ^= tXorPotionCount;
	ChkPotion->Ps102 ^= tXorPotionCount;
	ChkPotion->Ps103 ^= tXorPotionCount;
	ChkPotion->Ps104 ^= tXorPotionCount;

	return TRUE;
}

//물약갯수 비교
int	ComparePotion()
{
	sCHK_POTION_COUNT	ChkPotionCount;
	int	CompareError = 0;

	XorCheckPotion( &sPotionTotalCount );

	GetInvenPotionCount( &ChkPotionCount );

	if ( ChkPotionCount.Pl101>sPotionTotalCount.Pl101 )
		CompareError ++;
	if ( ChkPotionCount.Pl102>sPotionTotalCount.Pl102 )
		CompareError ++;
	if ( ChkPotionCount.Pl103>sPotionTotalCount.Pl103 )
		CompareError ++;
	if ( ChkPotionCount.Pl104>sPotionTotalCount.Pl104 )
		CompareError ++;

	if ( ChkPotionCount.Pm101>sPotionTotalCount.Pm101 )
		CompareError ++;
	if ( ChkPotionCount.Pm102>sPotionTotalCount.Pm102 )
		CompareError ++;
	if ( ChkPotionCount.Pm103>sPotionTotalCount.Pm103 )
		CompareError ++;
	if ( ChkPotionCount.Pm104>sPotionTotalCount.Pm104 )
		CompareError ++;

	if ( ChkPotionCount.Ps101>sPotionTotalCount.Ps101 )
		CompareError ++;
	if ( ChkPotionCount.Ps102>sPotionTotalCount.Ps102 )
		CompareError ++;
	if ( ChkPotionCount.Ps103>sPotionTotalCount.Ps103 )
		CompareError ++;
	if ( ChkPotionCount.Ps104>sPotionTotalCount.Ps104 )
		CompareError ++;

		int cnt1,cnt2;
		cnt1 = ChkPotionCount.Pl101+ChkPotionCount.Pl102+ChkPotionCount.Pl103+ChkPotionCount.Pl104+
			ChkPotionCount.Pm101+ChkPotionCount.Pm102+ChkPotionCount.Pm103+ChkPotionCount.Pm104+
			ChkPotionCount.Ps101+ChkPotionCount.Ps102+ChkPotionCount.Ps103+ChkPotionCount.Ps104;

		cnt2 = sPotionTotalCount.Pl101+sPotionTotalCount.Pl102+sPotionTotalCount.Pl103+sPotionTotalCount.Pl104+
			sPotionTotalCount.Pm101+sPotionTotalCount.Pm102+sPotionTotalCount.Pm103+sPotionTotalCount.Pm104+
			sPotionTotalCount.Ps101+sPotionTotalCount.Ps102+sPotionTotalCount.Ps103+sPotionTotalCount.Ps104;

	if ( CompareError ) {
		//물약 오류 발견
		//해킹 시도한 유저 자동 신고
		SendSetHackUser3( 1530 , CompareError , cnt1-cnt2  );
	}

	tXorPotionCount = dwPlayTime&0xFFFFFFF;
	memcpy( &sPotionTotalCount , &ChkPotionCount , sizeof(sCHK_POTION_COUNT) );
	XorCheckPotion( &sPotionTotalCount );

	return CompareError;
}



//물약 갯수 새로 설정
int	ResetPotion()
{
	GetInvenPotionCount( &sPotionTotalCount );
	tXorPotionCount = dwPlayTime&0xFFFFFFF;
	XorCheckPotion( &sPotionTotalCount );

	return TRUE;
}

//물약 갯수 새로 설정
int	ResetPotion2()
{
	int cnt1,cnt2;
	sCHK_POTION_COUNT	ChkPotionCount;

	XorCheckPotion( &sPotionTotalCount );
	GetInvenPotionCount( &ChkPotionCount );

	//아이템 물약 입수
	cnt1 = ChkPotionCount.Pl101+ChkPotionCount.Pl102+ChkPotionCount.Pl103+ChkPotionCount.Pl104+
			ChkPotionCount.Pm101+ChkPotionCount.Pm102+ChkPotionCount.Pm103+ChkPotionCount.Pm104+
			ChkPotionCount.Ps101+ChkPotionCount.Ps102+ChkPotionCount.Ps103+ChkPotionCount.Ps104;

	cnt2 = sPotionTotalCount.Pl101+sPotionTotalCount.Pl102+sPotionTotalCount.Pl103+sPotionTotalCount.Pl104+
			sPotionTotalCount.Pm101+sPotionTotalCount.Pm102+sPotionTotalCount.Pm103+sPotionTotalCount.Pm104+
			sPotionTotalCount.Ps101+sPotionTotalCount.Ps102+sPotionTotalCount.Ps103+sPotionTotalCount.Ps104;

	if ( cnt1>(cnt2+TransRecvItem.Item.PotionCount) )
		return TRUE;

	memcpy( &sPotionTotalCount , &ChkPotionCount , sizeof(sCHK_POTION_COUNT) );
	tXorPotionCount = dwPlayTime&0xFFFFFFF;
	XorCheckPotion( &sPotionTotalCount );

	return FALSE;
}

//아이템이 물약인 경우 기록
int	IncPotionCount( sCHK_POTION_COUNT *ChkPotion , sITEMINFO *lpItem ) 
{
	int	PotionCount;

	PotionCount = lpItem->PotionCount;

	if ( !PotionCount ) return FALSE;

	if(lpItem->CODE==(sinPL1|sin01) ) 
		ChkPotion->Pl101+=PotionCount;
	if(lpItem->CODE==(sinPL1|sin02) ) 
		ChkPotion->Pl102+=PotionCount;
	if(lpItem->CODE==(sinPL1|sin03) ) 
		ChkPotion->Pl103+=PotionCount;
	if(lpItem->CODE==(sinPL1|sin04) ) 
		ChkPotion->Pl104+=PotionCount;


	if(lpItem->CODE==(sinPM1|sin01) ) 
		ChkPotion->Pm101+=PotionCount;
	if(lpItem->CODE==(sinPM1|sin02) ) 
		ChkPotion->Pm102+=PotionCount;
	if(lpItem->CODE==(sinPM1|sin03) ) 
		ChkPotion->Pm103+=PotionCount;
	if(lpItem->CODE==(sinPM1|sin04) ) 
		ChkPotion->Pm104+=PotionCount;


	if(lpItem->CODE==(sinPS1|sin01) ) 
		ChkPotion->Ps101+=PotionCount;
	if(lpItem->CODE==(sinPS1|sin02) ) 
		ChkPotion->Ps102+=PotionCount;
	if(lpItem->CODE==(sinPS1|sin03) ) 
		ChkPotion->Ps103+=PotionCount;
	if(lpItem->CODE==(sinPS1|sin04) ) 
		ChkPotion->Ps104+=PotionCount;

	return TRUE;
}


//인벤토리와 거래창에서 물약을 찾어 갯수 기록
int GetInvenPotionCount( sCHK_POTION_COUNT *ChkPotion )
{
	int cnt;

	ZeroMemory( ChkPotion , sizeof(sCHK_POTION_COUNT) );

	for( cnt=0;cnt<INVENTORY_MAXITEM ; cnt++){ 
		if(cInvenTory.InvenItem[cnt].Flag){
			IncPotionCount(  ChkPotion , &cInvenTory.InvenItem[cnt].sItemInfo );
		}
	}

	for( cnt=0;cnt<INVENTORY_MAXITEM ; cnt++){ 
		if(cInvenTory.InvenItemTemp[cnt].Flag){
			IncPotionCount(  ChkPotion , &cInvenTory.InvenItemTemp[cnt].sItemInfo );
		}
	}

	if ( cTrade.OpenFlag ) {
		for( cnt=0;cnt<MAX_TRADE_ITEM;cnt++) {
			if ( sTrade.TradeItem[cnt].Flag ) {
				IncPotionCount(  ChkPotion , &sTrade.TradeItem[cnt].sItemInfo );
			}
		}
	}

	if ( MouseItem.Flag )
		IncPotionCount( ChkPotion , &MouseItem.sItemInfo );

	return TRUE;
}

int	GetItemCheckCode( sITEMINFO *lpItem )
{
	if ( lpItem->CODE<sinPM1 && lpItem->ItemName[0] ) {
		return lpItem->ItemHeader.dwChkSum;
	}
	return 0;
}


//인벤토리의 모든 아이템을 확인하여 코드를 반환
DWORD GetInvenItemCode()
{
	int cnt;

	DWORD	dwCode;


	dwCode = 0;

	for( cnt=0;cnt<INVENTORY_MAXITEM ; cnt++){ 
		if( cInvenTory.InvenItem[cnt].Flag ){
			dwCode+=GetItemCheckCode( &cInvenTory.InvenItem[cnt].sItemInfo );
		}
	}

	for( cnt=0;cnt<INVENTORY_MAXITEM ; cnt++){ 
		if(cInvenTory.InvenItemTemp[cnt].Flag ){
			dwCode+=GetItemCheckCode(&cInvenTory.InvenItemTemp[cnt].sItemInfo);
		}
	}

	if ( cTrade.OpenFlag ) {
		for( cnt=0;cnt<MAX_TRADE_ITEM;cnt++) {
			if ( sTrade.TradeItem[cnt].Flag ) {
				dwCode+=GetItemCheckCode( &sTrade.TradeItem[cnt].sItemInfo );
			}
		}
	}


	if ( MouseItem.Flag ) {
		dwCode +=  GetItemCheckCode( &MouseItem.sItemInfo );
	}

	if ( cWareHouse.OpenFlag ) {
		for( cnt=0;cnt<100;cnt++) {
			if ( sWareHouse.WareHouseItem[cnt].Flag ) {
				dwCode +=  GetItemCheckCode( &sWareHouse.WareHouseItem[cnt].sItemInfo );
			}
		}
	}

	if ( cCraftItem.OpenFlag ) {
		for( cnt=0;cnt<15;cnt++) {
			if ( sCraftItem.CraftItem[cnt].Flag ) {
				dwCode += GetItemCheckCode( &sCraftItem.CraftItem[cnt].sItemInfo );
			}
		}
	}

	if ( cAging.OpenFlag ) {
		for( cnt=0;cnt<15;cnt++) {
			if ( sAgingItem.AgingItem[cnt].Flag ) {
				dwCode += GetItemCheckCode( &sAgingItem.AgingItem[cnt].sItemInfo );
			}
		}
	}

	// pluto 제련
	if ( SmeltingItem.OpenFlag )
	{
		for( cnt=0;cnt<5;cnt++)
		{
			if ( S_smeltingItem.SmeltingItem[cnt].Flag )
			{
				dwCode += GetItemCheckCode( &S_smeltingItem.SmeltingItem[cnt].sItemInfo );
			}
		}
	}

	// pluto 제작
	if( ManufactureItem.m_OpenFlag )
	{
		for( cnt = 0; cnt < 4; cnt++ )
		{
			if( g_sManufactureItem.ManufactureItem[cnt].Flag )
			{
				dwCode += GetItemCheckCode( &g_sManufactureItem.ManufactureItem[cnt].sItemInfo );
			}
		}
	}
	//무기를 팔때 , 믹스쳐 에이징 종료시

	// 석지용 - 믹스쳐 리셋 창이 열려있을시...
	if( cMixtureReset.OpenFlag )
	{
		for( cnt=0; cnt<2; cnt++ )
		{
			if( sMixtureResetItem.MixtureResetItem[cnt].Flag )
			{
				dwCode += GetItemCheckCode( &sMixtureResetItem.MixtureResetItem[cnt].sItemInfo );
			}
		}
	}

	return dwCode;
}


DWORD	dwTotal_InvenItemCode;
DWORD	dwTotal_InvenItemPlayTime;


//이벤토리 아이템 코드 초기화
int	ResetInvenItemCode()
{
	//인벤토리의 모든 아이템을 확인하여 코드를 반환
	dwTotal_InvenItemCode = GetInvenItemCode();
	dwTotal_InvenItemCode = dwPlayTime^dwTotal_InvenItemCode;
	dwTotal_InvenItemPlayTime = dwPlayTime;

	return TRUE;
}

//이벤토리 아이템 코드에서 특정 아이템 제거
int SubInvenItemCode( sITEMINFO *lpItem )
{
	DWORD	dwCode;

	dwCode = dwTotal_InvenItemCode^dwTotal_InvenItemPlayTime;
	dwTotal_InvenItemCode = dwCode - GetItemCheckCode( lpItem );

	dwTotal_InvenItemCode = dwPlayTime^dwTotal_InvenItemCode;
	dwTotal_InvenItemPlayTime = dwPlayTime;

	return TRUE;
}


//인벤토리 아이템 코드 확인
int	CheckInvenItemCode()
{
	DWORD dwCode;
	smTRANS_COMMAND	smTransCommand;

	dwCode = GetInvenItemCode();

	if ( dwCode!=(dwTotal_InvenItemCode^dwTotal_InvenItemPlayTime) ) {
		smTransCommand.size = sizeof( smTRANS_COMMAND );
		smTransCommand.code = smTRANSCODE_SET_BLACKLIST3;
		smTransCommand.WParam = 7010;
		smTransCommand.LParam = dwTotal_InvenItemCode^dwTotal_InvenItemPlayTime;
		smTransCommand.SParam = dwCode;
		if ( smWsockDataServer ) {
			smWsockDataServer->Send2( (char *)&smTransCommand , sizeof(smTRANS_COMMAND) , TRUE );
		}
	}

	dwTotal_InvenItemCode = dwPlayTime^dwCode;
	dwTotal_InvenItemPlayTime = dwPlayTime;

	return TRUE;
}


void CheckPlaySubFunc2()
{
	if ( dwPlaySub_CheckResult!=PLAYSUB_CHECKRESULT ) {
		SendSetHackUser2( 1120 , dwPlaySub_CheckResult );
		DisconnectFlag = dwPlayTime;
	}
}

short	sChkLife[2];
short	sChkMana[2];
short	sChkStamina[2];
int		EnergyGraphErrorCount[4] = { 0,0,0,0 };


//에너지 그래프 오류 첵크
int CheckEnergyGraphError()
{
	int Life,Mana,Stamina;
	smTRANS_COMMAND	smTransCommand;


	Life = EnergyGraphErrorCount[0] - EnergyGraphErrorCount[3];
	Mana = EnergyGraphErrorCount[1] - EnergyGraphErrorCount[3];
	Stamina = EnergyGraphErrorCount[2] - EnergyGraphErrorCount[3];

	//if ( Life>3 || Mana>3 || Stamina>3 ) {

	if ( Life>0 || Mana>0 || Stamina>0 ) {
		smTransCommand.size = sizeof(smTRANS_COMMAND);
		smTransCommand.code = smTRANSCODE_CLIENT_ERROR;
		smTransCommand.WParam = 0;

		smTransCommand.LParam = Life;
		smTransCommand.SParam = Mana;
		smTransCommand.EParam = Stamina;

		if ( smWsockDataServer )
			smWsockDataServer->Send2( (char *)&smTransCommand , smTransCommand.size , TRUE );
	}

	EnergyGraphErrorCount[3] = lpCurPlayer->pX;
	EnergyGraphErrorCount[0] = EnergyGraphErrorCount[3]+Life;
	EnergyGraphErrorCount[1] = EnergyGraphErrorCount[3]+Life;
	EnergyGraphErrorCount[2] = EnergyGraphErrorCount[3]+Life;

	return TRUE;
}

//에너지 그래프 첵크 초기화
int	ResetEnergyGraph( int Num )
{
	int	Pos;

	Pos = lpCurPlayer->pZ;

	switch( Num ) {
		case 0:
			//LIFE
			sChkLife[0] = Pos;
			sChkLife[1] = Pos^(-1);
			break;

		case 1:
			//MANA
			sChkMana[0] = Pos;
			sChkMana[1] = Pos^(-1);
			break;

		case 2:
			//STAMINA
			sChkStamina[0] = Pos;
			sChkStamina[1] = Pos^(-1);
			break;

		case 3:
		case 4:
			//ALL
			sChkLife[0] = Pos;
			sChkLife[1] = Pos^(-1);
			sChkMana[0] = Pos;
			sChkMana[1] = Pos^(-1);
			sChkStamina[0] = Pos;
			sChkStamina[1] = Pos^(-1);
			break;

	}

	return TRUE;
}

//에너지 그래프의 오류 첵크
int CheckEnergyGraph()
{
	int	Pos;
	int	Grp;
	int	Last_Grp;
	int cnt;

	// GM /IDDQD: keep life, mana and stamina full (anything that went round the setters)
	{
		extern int sinGM_GodMode;
		if ( sinGM_GodMode && lpCurPlayer && lpCurPlayer->MotionInfo &&
			lpCurPlayer->MotionInfo->State!=CHRMOTION_STATE_DEAD ) {
			if ( sinGetLife()<lpCurPlayer->smCharInfo.Life[1] ) sinSetLife( lpCurPlayer->smCharInfo.Life[1] );
			if ( sinGetMana()<lpCurPlayer->smCharInfo.Mana[1] ) sinSetMana( lpCurPlayer->smCharInfo.Mana[1] );
			if ( sinGetStamina()<lpCurPlayer->smCharInfo.Stamina[1] ) sinSetStamina( lpCurPlayer->smCharInfo.Stamina[1] );
			ResetEnergyGraph( 4 );
			return TRUE;
		}
	}

	Pos = lpCurPlayer->pZ;

	Last_Grp = sChkLife[0]^sChkLife[1];
	Grp = sinGetLife();
	if ( Last_Grp!=-1 ) {
		cnt = Grp-Last_Grp;
		if ( cnt>15 ) {
			EnergyGraphErrorCount[0] ++;
		}
	}
	sChkLife[0] = Pos;
	sChkLife[1] = Pos^Grp;

	Last_Grp = sChkMana[0]^sChkMana[1];
	Grp = sinGetMana();
	if ( Last_Grp!=-1 ) {
		cnt = Grp-Last_Grp;
		if ( cnt>15 ) {
			EnergyGraphErrorCount[1] ++;
		}
	}
	sChkMana[0] = Pos;
	sChkMana[1] = Pos^Grp;

	Last_Grp = sChkStamina[0]^sChkStamina[1];
	Grp = sinGetStamina();
	if ( Last_Grp!=-1 ) {
		cnt = Grp-Last_Grp;
		if ( cnt>50 ) {
			EnergyGraphErrorCount[2] ++;
		}
	}
	sChkStamina[0] = Pos;
	sChkStamina[1] = Pos^Grp;

	return TRUE;
}


//인벤토리의 지정한 아이템을 찾아 검색하여 찾는다
sITEMINFO *FindInvenItem( DWORD dwItemCode , DWORD dwHead , DWORD dwChkSum )
{
	int cnt;
	sITEMINFO *lpItem;

	if ( (dwItemCode&sinITEM_MASK1)==(sinPM1&sinITEM_MASK1) ) return NULL;

	for( cnt=0;cnt<INVENTORY_MAXITEM ; cnt++){ 
		if( cInvenTory.InvenItem[cnt].Flag ){
			lpItem = &cInvenTory.InvenItem[cnt].sItemInfo;
			if ( lpItem->CODE==dwItemCode && lpItem->ItemHeader.Head==dwHead && lpItem->ItemHeader.dwChkSum==dwChkSum ) { 
				return lpItem;
			}
		}
	}

	for( cnt=0;cnt<INVENTORY_MAXITEM ; cnt++){ 
		if(cInvenTory.InvenItemTemp[cnt].Flag ){
			lpItem = &cInvenTory.InvenItemTemp[cnt].sItemInfo;
			if ( lpItem->CODE==dwItemCode && lpItem->ItemHeader.Head==dwHead && lpItem->ItemHeader.dwChkSum==dwChkSum ) { 
				return lpItem;
			}
		}
	}

	if ( cTrade.OpenFlag ) {
		for( cnt=0;cnt<MAX_TRADE_ITEM;cnt++) {
			if ( sTrade.TradeItem[cnt].Flag ) {
				lpItem = &sTrade.TradeItem[cnt].sItemInfo;
				if ( lpItem->CODE==dwItemCode && lpItem->ItemHeader.Head==dwHead && lpItem->ItemHeader.dwChkSum==dwChkSum ) { 
					return lpItem;
				}
			}
		}
	}

	if ( MouseItem.Flag ) {
		lpItem = &MouseItem.sItemInfo;
		if ( lpItem->CODE==dwItemCode && lpItem->ItemHeader.Head==dwHead && lpItem->ItemHeader.dwChkSum==dwChkSum ) { 
			return lpItem;
		}
	}

	if ( cWareHouse.OpenFlag ) {
		for( cnt=0;cnt<100;cnt++) {
			if ( sWareHouse.WareHouseItem[cnt].Flag ) {
				lpItem = &sWareHouse.WareHouseItem[cnt].sItemInfo;
				if ( lpItem->CODE==dwItemCode && lpItem->ItemHeader.Head==dwHead && lpItem->ItemHeader.dwChkSum==dwChkSum ) { 
					return lpItem;
				}
			}
		}
	}

	if ( cCraftItem.OpenFlag ) {
		for( cnt=0;cnt<15;cnt++) {
			if ( sCraftItem.CraftItem[cnt].Flag ) {
				lpItem = &sCraftItem.CraftItem[cnt].sItemInfo;
				if ( lpItem->CODE==dwItemCode && lpItem->ItemHeader.Head==dwHead && lpItem->ItemHeader.dwChkSum==dwChkSum ) { 
					return lpItem;
				}
			}
		}
	}

	if ( cAging.OpenFlag ) {
		for( cnt=0;cnt<15;cnt++) {
			if ( sAgingItem.AgingItem[cnt].Flag ) {
				lpItem = &sAgingItem.AgingItem[cnt].sItemInfo;
				if ( lpItem->CODE==dwItemCode && lpItem->ItemHeader.Head==dwHead && lpItem->ItemHeader.dwChkSum==dwChkSum ) { 
					return lpItem;
				}
			}
		}
	}
	// pluto 제련
	if( SmeltingItem.OpenFlag )
	{
		for( cnt = 0; cnt < 5; cnt++ )
		{
			if( S_smeltingItem.SmeltingItem[cnt].Flag)
			{
				lpItem = &S_smeltingItem.SmeltingItem[cnt].sItemInfo;
				if( lpItem->CODE == dwItemCode && lpItem->ItemHeader.Head == dwHead && lpItem->ItemHeader.dwChkSum == dwChkSum )
				{
					return lpItem;
				}
			}
		}
	}

	// pluto 제작
	if( ManufactureItem.m_OpenFlag )
	{
		for( cnt = 0; cnt < 4; cnt++ )
		{
			if( g_sManufactureItem.ManufactureItem[cnt].Flag )
			{
				lpItem = &g_sManufactureItem.ManufactureItem[cnt].sItemInfo;
				if( lpItem->CODE == dwItemCode && lpItem->ItemHeader.Head == dwHead && lpItem->ItemHeader.dwChkSum == dwChkSum )
				{
					return lpItem;
				}
			}
		}
	}

	// 석지용 - 믹스쳐 리셋 처리
	if( cMixtureReset.OpenFlag )
	{
		for( cnt=0; cnt<2; cnt++ )
		{
			if( sMixtureResetItem.MixtureResetItem[cnt].Flag )
			{
				lpItem = &sMixtureResetItem.MixtureResetItem[cnt].sItemInfo;
				if( lpItem->CODE == dwItemCode && lpItem->ItemHeader.Head == dwHead && lpItem->ItemHeader.dwChkSum == dwChkSum )
					return lpItem;
			}
		}
	}

	return NULL;
}

//인벤토리 아이템 복사 오류
int SendInvenItemError( DWORD dwKind , DWORD dwItemCode , DWORD dwHead , DWORD dwChkSum )
{
	smTRANS_COMMAND	smTransCommand;

	smTransCommand.size = sizeof(smTRANS_COMMAND);
	smTransCommand.code = smTRANSCODE_INVEN_ERR_ITEM;
	smTransCommand.WParam = dwKind;

	smTransCommand.LParam = dwItemCode;
	smTransCommand.SParam = dwHead;
	smTransCommand.EParam = dwChkSum;

	if ( smWsockDataServer )
		smWsockDataServer->Send2( (char *)&smTransCommand , smTransCommand.size , TRUE );

	return TRUE;
}

int	GetTotalInvenMoney()
{
	int Money;

	Money = lpCurPlayer->smCharInfo.Money;

	if ( cTrade.OpenFlag && sTrade.Money ) 
		Money += sTrade.Money-193;

	if ( cWareHouse.OpenFlag ) 
		Money += sWareHouse.Money -2023;

	return Money;
}

DWORD	dwTotal_CheckMoney = 0;
DWORD	dwTotal_CheckMoneyTime = 0;
#define	TOTAL_CHECK_MONEY_MASK	0x0018cccc
int		NChkMode = 0;


//이벤토리 돈 검사 초기화
int	ResetInvenMoney()
{
	dwTotal_CheckMoney = GetTotalInvenMoney();
	dwTotal_CheckMoney = dwPlayTime^dwTotal_CheckMoney;
	dwTotal_CheckMoneyTime = dwPlayTime^TOTAL_CHECK_MONEY_MASK;

	return TRUE;
}


//인벤토리 검사용돈 추가
int	AddInvenMoney( int Money )
{
	int	m;

	m = dwTotal_CheckMoney^(dwTotal_CheckMoneyTime^TOTAL_CHECK_MONEY_MASK);
	m += Money;
	if ( m<0 ) m=0;

	dwTotal_CheckMoney = dwPlayTime^m;
	dwTotal_CheckMoneyTime = dwPlayTime^TOTAL_CHECK_MONEY_MASK;

	return TRUE;
}

//인벤토리 돈 검사
int	CheckInvenMoney()
{
	DWORD dwCode;
	smTRANS_COMMAND	smTransCommand;

	if ( cShop.OpenFlag || SkillMasterFlag || cCraftItem.OpenFlag || cAging.OpenFlag || SmeltingItem.OpenFlag || ManufactureItem.m_OpenFlag || cMixtureReset.OpenFlag)  // 석지용 - 믹스쳐 리셋 창 추가
	{
		NChkMode = TRUE;
		return TRUE;
	}

	if ( NChkMode ) {
		NChkMode = 0;
		ResetInvenMoney();
		if ( NChkMode==0 ) return TRUE;
		dwCode = 0;
	}
	else {
		dwCode = GetTotalInvenMoney();
	}

	if ( dwCode!=(dwTotal_CheckMoney^(dwTotal_CheckMoneyTime^TOTAL_CHECK_MONEY_MASK)) ) {
		smTransCommand.size = sizeof( smTRANS_COMMAND );
		smTransCommand.code = smTRANSCODE_INVEN_ERR_MONEY;
		smTransCommand.WParam = 0;
		smTransCommand.LParam = dwCode;
		smTransCommand.SParam = (dwTotal_CheckMoney^(dwTotal_CheckMoneyTime^TOTAL_CHECK_MONEY_MASK));
		smTransCommand.EParam = 0;
		if ( smWsockDataServer ) {
			smWsockDataServer->Send2( (char *)&smTransCommand , sizeof(smTRANS_COMMAND) , TRUE );
		}
	}

	dwTotal_CheckMoney = dwPlayTime^dwCode;
	dwTotal_CheckMoneyTime = dwPlayTime^TOTAL_CHECK_MONEY_MASK;

	return TRUE;
}



//인벤토리의 장착 아이템 정보 서버에 보내기
int SendUsingItemListToServer()
{

	int cnt;
	sITEMINFO *lpItem;

	TRANS_USINGITEM	TransUsingItem;

	ZeroMemory( &TransUsingItem , sizeof(TRANS_USINGITEM) );

	TransUsingItem.size = sizeof(TRANS_USINGITEM);
	TransUsingItem.code = smTRANSCODE_INVEN_POSITION;

	for( cnt=0;cnt<INVENTORY_MAXITEM ; cnt++){ 
		if( cInvenTory.InvenItem[cnt].Flag && cInvenTory.InvenItem[cnt].ItemPosition ) {
			lpItem = &cInvenTory.InvenItem[cnt].sItemInfo;
			if ( lpItem->ItemName[0] ) {
				if ( TransUsingItem.ItemListCount>=16 ) break;
				TransUsingItem.ItemList[TransUsingItem.ItemListCount].dwCode = lpItem->CODE;
				TransUsingItem.ItemList[TransUsingItem.ItemListCount].Performance[0] = cInvenTory.InvenItem[cnt].ItemPosition;
				TransUsingItem.ItemList[TransUsingItem.ItemListCount].Performance[1] = lpItem->Damage[1];
				TransUsingItem.ItemList[TransUsingItem.ItemListCount].Performance[2] = lpItem->Defence;
				TransUsingItem.ItemList[TransUsingItem.ItemListCount].Performance[3] = (short)lpItem->fBlock_Rating;
				TransUsingItem.ItemList[TransUsingItem.ItemListCount].Performance[4] = (short)lpItem->fAbsorb;
				TransUsingItem.ItemList[TransUsingItem.ItemListCount].Performance[5] = lpItem->Attack_Rating;
				TransUsingItem.ItemListCount++;
			}
		}
	}

	for( cnt=0;cnt<INVENTORY_MAXITEM ; cnt++){ 
		if(cInvenTory.InvenItemTemp[cnt].Flag && cInvenTory.InvenItemTemp[cnt].ItemPosition ){
			lpItem = &cInvenTory.InvenItemTemp[cnt].sItemInfo;
			if ( lpItem->ItemName[0] ) {
				if ( TransUsingItem.ItemListCount>=16 ) break;
				TransUsingItem.ItemList[TransUsingItem.ItemListCount].dwCode = lpItem->CODE;
				TransUsingItem.ItemList[TransUsingItem.ItemListCount].Performance[0] = cInvenTory.InvenItemTemp[cnt].ItemPosition|0x100;
				TransUsingItem.ItemList[TransUsingItem.ItemListCount].Performance[1] = lpItem->Damage[1];
				TransUsingItem.ItemList[TransUsingItem.ItemListCount].Performance[2] = lpItem->Defence;
				TransUsingItem.ItemList[TransUsingItem.ItemListCount].Performance[3] = (short)lpItem->fBlock_Rating;
				TransUsingItem.ItemList[TransUsingItem.ItemListCount].Performance[4] = (short)lpItem->fAbsorb;
				TransUsingItem.ItemList[TransUsingItem.ItemListCount].Performance[5] = lpItem->Attack_Rating;
				TransUsingItem.ItemListCount++;
			}
		}
	}

	smWINSOCK	*lpsmSendSock;

	lpsmSendSock = GetAreaServerSock();		//보낼 서버구하기
	if ( lpsmSendSock ) {
		return lpsmSendSock->Send2( (char *)&TransUsingItem , TransUsingItem.size , TRUE );
	}

	return NULL;
}

//인벤토리의 장착 아이템 정보 서버에 보내기
int SendUsingItemListToServer2()
{

	int cnt;
	sITEMINFO *lpItem;

	TRANS_USINGITEM	TransUsingItem;

	ZeroMemory( &TransUsingItem , sizeof(TRANS_USINGITEM) );

	TransUsingItem.size = sizeof(TRANS_USINGITEM);
	TransUsingItem.code = smTRANSCODE_INVEN_POSITION2;

	for( cnt=0;cnt<INVENTORY_MAXITEM ; cnt++){ 
		if( cInvenTory.InvenItem[cnt].Flag && cInvenTory.InvenItem[cnt].ItemPosition ) {
			lpItem = &cInvenTory.InvenItem[cnt].sItemInfo;
			if ( lpItem->ItemName[0] ) {
				if ( TransUsingItem.ItemListCount>=16 ) break;
				TransUsingItem.ItemList[TransUsingItem.ItemListCount].dwCode = lpItem->CODE;
				TransUsingItem.ItemList[TransUsingItem.ItemListCount].Performance[0] = cInvenTory.InvenItem[cnt].ItemPosition;
				TransUsingItem.ItemList[TransUsingItem.ItemListCount].Performance[1] = lpItem->Damage[1];
				TransUsingItem.ItemList[TransUsingItem.ItemListCount].Performance[2] = lpItem->Defence;
				TransUsingItem.ItemList[TransUsingItem.ItemListCount].Performance[3] = (short)lpItem->fBlock_Rating;
				TransUsingItem.ItemList[TransUsingItem.ItemListCount].Performance[4] = (short)lpItem->fAbsorb;
				TransUsingItem.ItemList[TransUsingItem.ItemListCount].Performance[5] = lpItem->Attack_Rating;
				TransUsingItem.ItemListCount++;
			}
		}
	}

	smWINSOCK	*lpsmSendSock;

	lpsmSendSock = GetAreaServerSock();		//보낼 서버구하기
	if ( lpsmSendSock ) {
		return lpsmSendSock->Send2( (char *)&TransUsingItem , TransUsingItem.size , TRUE );
	}

	return NULL;
}


//시간 구하기 (time_t)
DWORD	GetPlayTime_T()
{
	DWORD dwTime;

	if ( dwTime_ServerT ) {
		dwTime = (dwPlayTime-dwTime_ConnectMS)/1000;
		dwTime = dwTime_ServerT+dwTime;				//현재의 Time_T
		return dwTime;
	}

	return NULL;
}


//머리가 커지는 모드 설정 
int	ChangeBigHeadMode( int Mode , int Time )
{
	DWORD	dwTime;

	if ( Mode ) 
	{
		if(Mode == 3) // 박재원 - 2010 월드컵 이벤트 - 축구공 포션 사용
		{
			lpCurPlayer->smCharInfo.SizeLevel = 1; // 정상 얼굴
			ChangeModelSkin( lpCurPlayer , 0 , 6 , 0 , 0 );	// 축구공 쓴 얼굴
		}
		else
		{
			lpCurPlayer->smCharInfo.SizeLevel = Mode+0x1000;
			ChangeModelSkin( lpCurPlayer , 1 , lpCurPlayer->smCharInfo.SizeLevel+3-0x1000 , 0 , 0 );	//큰얼굴
		}
		dwTime=GetPlayTime_T();

		if ( dwTime ) 
			lpCurPlayer->smCharInfo.dwEventTime_T = dwTime + Time;	//모델제한 시간
		else
			lpCurPlayer->smCharInfo.dwEventTime_T  = 0;				//시간제한 없음 ( 재접 할때까지 )
	}
	else {
		lpCurPlayer->smCharInfo.SizeLevel = 1;
		lpCurPlayer->smCharInfo.dwEventTime_T = 0;
		ChangeModelSkin( lpCurPlayer , 0 , JOB_FACE(lpCurPlayer->smCharInfo.ChangeJob) , 0 , GetFaceHairSkin(lpCurPlayer) );				//정상 얼굴 // 얼큰이 오류 수정
	}

	lpCurPlayer->smCharInfo.bUpdateInfo[0]++;
	ReformCharForm();
	SendSetObjectSerial( lpCurPlayer->dwObjectSerial );

	if ( RestartPlayCount<350 ) RestartPlayCount=350;		//잠시동안 무적 (3초)

	return TRUE;
}

//공격속도를 프레임속도로 변환
int GetAttackSpeedMainFrame( int AttackSpeed )
{
	int cnt;

	cnt = AttackSpeed-6;
	if ( cnt<0 ) cnt=0;
	if ( cnt>6 ) cnt=6;

	return fONE + 32*cnt;
}

//공격속도를 프레임속도로 변환
int	GetAttackSpeedFrame( int AttackSpeed , int AddSpeed )
{
	int cnt;

	cnt = GetAttackSpeedMainFrame( AttackSpeed );
	if ( AddSpeed<6 && AddSpeed>0 )
		cnt += AddSpeed*32;

	return (80*cnt)>>FLOATNS;
}

//공격속도를 프레임속도로 변환
int	GetAttackSpeedFrame( int AttackSpeed )
{
	int cnt;

	cnt = GetAttackSpeedMainFrame( AttackSpeed );

	return (80*cnt)>>FLOATNS;
}







//폴더 내의 ini 파일 찿아 읽기
int LoadFolder_Ini( char *szPath , char *szWildCard , char *szChgExt , int OftSave )
{

	HANDLE hFindHandle;
	WIN32_FIND_DATA	fd;
	char	szFindPath[64];
	char	szFilePath[64];
	char	*lpPath;

	HANDLE hFindHandle2;
	WIN32_FIND_DATA	fd2;
	char	szFindPath2[64];

	smDPAT	*lpDPatTest[65536];
	int		DPatCount = 0;
	int		cnt;

	lstrcpy( szFindPath , szPath );
	lstrcat( szFindPath , "\\*.*" );


	hFindHandle = FindFirstFile( szFindPath , &fd );
	if ( hFindHandle!=INVALID_HANDLE_VALUE ) {
		while(1) {
			if ( (fd.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)!=0 ) {


				wsprintf( szFindPath2 , "%s\\%s\\%s" , szPath , fd.cFileName , szWildCard );

				hFindHandle2 = FindFirstFile( szFindPath2 , &fd2 );
				if ( hFindHandle2!=INVALID_HANDLE_VALUE ) {
					while(1) {
						wsprintf( szFilePath , "%s\\%s\\%s" , szPath , fd.cFileName , fd2.cFileName );


						if ( OftSave ) {
							//자주 쓰기 기록

							if ( szChgExt )
								lpPath = ChangeFileExt( szFilePath , szChgExt );
							else
								lpPath = 0;
							
							if ( !lpPath ) lpPath = szFilePath;

							lstrcpy( szFilePath , lpPath );

							LoadOftenMeshPattern(szFilePath);
						}
						else {
							//단순 테스트
							lpDPatTest[DPatCount] = smPattern.LoadCharactor( szFilePath );
							if ( lpDPatTest[DPatCount] ) DPatCount++;
							else {
								DPatCount = DPatCount;
							}
						}

						if ( FindNextFile( hFindHandle2 , &fd2 )==FALSE ) break;
					}
					FindClose( hFindHandle2 );
				}
			}

			//다음 파일 찾음
			if ( FindNextFile( hFindHandle , &fd )==FALSE ) break;
		}
		FindClose( hFindHandle );
	}

	for(cnt=0;cnt<DPatCount;cnt++) {
		smDPAT_Delete( lpDPatTest[cnt] );
	}

	return TRUE;
}

sSOD_SCORE	sSodScore;
int			SodScoreFlag = 0;
extern SBL_SetFontEffect SoD_SetFontEffect;	//SoD 용 폰트 이펙트

//sSodScore.dwSoD_NextStageTime

//SoD 라운드 이펙트
int	Sod_SodEffect( int Round )
{

	if ( !BellatraEffectInitFlag ) return FALSE;

	SetBellatraFontEffect( E_BL_FONT_ROUND );

	switch( Round ) {
		case 1:
			SetBellatraFontEffect( E_BL_FONT_COUNT_1 );
			break;
		case 2:
			SetBellatraFontEffect( E_BL_FONT_COUNT_2 );
			break;
		case 3:
			SetBellatraFontEffect( E_BL_FONT_COUNT_3 );
			break;
		case 4:
			SetBellatraFontEffect( E_BL_FONT_COUNT_4 );
			break;
		case 5:
			SetBellatraFontEffect( E_BL_FONT_COUNT_5 );
			break;
		case 6:
			SetBellatraFontEffect( E_BL_FONT_COUNT_6 );
			break;
		case 7:
			SetBellatraFontEffect( E_BL_FONT_COUNT_7 );
			break;
		case 8:
			SetBellatraFontEffect( E_BL_FONT_COUNT_8 );
			break;
	}
	SetBellatraFontEffect( E_BL_FONT_START );

	esPlayContSound( Round );
	//esPlayContSound( 0 );	//Start


	return TRUE;
}



SBL_SetFontEffect SoD_SetFontEffect_CloseScreen;	//SoD 용 폰트 이펙트
char	szSOD_String[64] = { 0, };
BYTE	bSOD_StringColor[4];
int		SOD_StringCount = 0;
int		TeamScoreSort[4];
int		TeamMarkSort[4] = { 3 , 0 , 2 , 1 };
int		SoDGateFlag;


/*
struct  SBL_EventScore
{
	int  TeamNum;
	int  TeamMarkBuff[4];
	char szTeamPointBuff[4][32];

	int  EntryTeamNum;
	int  EntryTeamMarkBuff[2];
};
*/

//SOD 점수 출력표 작성
int	SetSodScoreResult()
{
	int cnt;

	g_EventScore.TeamNum = 0;
	g_EventScore.EntryTeamNum = 0;

	for(cnt=0;cnt<4;cnt++) {
		g_EventScore.TeamMarkBuff[cnt]=TeamMarkSort[TeamScoreSort[cnt]];
		wsprintf( g_EventScore.szTeamPointBuff[cnt] , "%d" , sSodScore.TeamScore[TeamScoreSort[cnt]] );
	}
	for(cnt=0;cnt<2;cnt++) {
		g_EventScore.EntryTeamMarkBuff[cnt] = g_EventScore.TeamMarkBuff[cnt];
	}

	if ( sSodScore.Round>0 && sSodScore.Round<=4 ) {
		g_EventScore.TeamNum = 4;
		g_EventScore.EntryTeamNum = 2;
	}
	if ( sSodScore.Round>4 && sSodScore.Round<=6 ) {
		g_EventScore.TeamNum = 2;
		g_EventScore.EntryTeamNum = 1;
	}

	if ( !g_EventScore.TeamNum ) return FALSE;

	return TRUE;
}


//SoD 연출
int	SodScoreMain()
{
	int dx,dy,dz;
	int x,z,cnt;
//	char	szBuff[256];
/*
	g_EventScore.TeamNum = 4;
	g_EventScore.TeamMarkBuff[0]=1;
	g_EventScore.TeamMarkBuff[1]=0;
	g_EventScore.TeamMarkBuff[2]=2;
	g_EventScore.TeamMarkBuff[3]=3;

	lstrcpy( g_EventScore.szTeamPointBuff[0] , "100" );
	lstrcpy( g_EventScore.szTeamPointBuff[1] , "100" );
	lstrcpy( g_EventScore.szTeamPointBuff[2] , "100" );
	lstrcpy( g_EventScore.szTeamPointBuff[3] , "100" );

	g_EventScore.EntryTeamNum = 2;
	g_EventScore.EntryTeamMarkBuff[0] = 1;
	g_EventScore.EntryTeamMarkBuff[1] = 2;

	SetBellatraFontEffect( E_BL_QUAKE_SEAL );
	SetBellatraFontEffect( E_BL_FONT_QUAKE_SEAL );

	E_BL_QUAKE_SEAL	   = 18,
	E_BL_STUN_SEAL	   = 19,
	E_BL_FREEZE_SEAL   = 20,
	E_BL_RABBIE_SEAL   = 21,
	E_BL_STYGIAN_SEAL  = 22,
	E_BL_GUARDIAN_SEAL = 23,
	E_BL_POINT_SEAL	   = 24,
	E_BL_FONT_QUAKE_SEAL	= 25,
	E_BL_FONT_STUN_SEAL	    = 26,
	E_BL_FONT_FREEZE_SEAL   = 27,
	E_BL_FONT_RABBIE_SEAL   = 28,
	E_BL_FONT_STYGIAN_SEAL  = 29,
	E_BL_FONT_GUARDIAN_SEAL = 30,
	E_BL_FONT_POINT_SEAL	= 31,

*/


	if ( !SodScoreFlag ) {
		SodScoreFlag = TRUE;
		ZeroMemory( &sSodScore , sizeof(sSOD_SCORE) );
	}

	if ( SOD_StringCount ) SOD_StringCount--;


	if ( sSodScore.dwSoD_NextStageTime ) {

		if ( sSodScore.dwSoD_CloseStageTime && sSodScore.dwSoD_CloseStageTime<dwPlayTime ) {

			if ( SoD_SetFontEffect_CloseScreen.eBL_Type!=E_BL_CHANGE_TRUE ) {

				if ( SoDGateFlag==TRUE && SetSodScoreResult()==TRUE )
					SetBellatraFontEffect( E_BL_CHANGE_TRUE , &SoD_SetFontEffect_CloseScreen , E_BL_EVENT_SCORE );		//닫는 화면 시작
				else
					SetBellatraFontEffect( E_BL_CHANGE_TRUE , &SoD_SetFontEffect_CloseScreen );		//닫는 화면 시작


				esPlaySound( 18 , 400 );
			}

			sSodScore.dwSoD_CloseStageTime = 0;
		}

		if ( sSodScore.dwSoD_NextStageTime<dwPlayTime ) {

			if ( SoD_SetFontEffect_CloseScreen.eBL_Type==E_BL_CHANGE_TRUE ) {
				CheckBellatraFontEffect( &SoD_SetFontEffect_CloseScreen , TRUE );					//닫는 화면 제거
				SetBellatraFontEffect( E_BL_CHANGE_FALSE , &SoD_SetFontEffect_CloseScreen );		//여는 화면 시작
				esPlaySound( 18 , 400 );
			}

			if ( sSodScore.SodNextStageNum>=0 && lpCurPlayer->MotionInfo->State!=CHRMOTION_STATE_DEAD ) {
				if ( rsCheckHardCoreEventArea( lpCurPlayer->pX , lpCurPlayer->pY , lpCurPlayer->pZ )==TRUE && sSodScore.SodNextStageNum<100 ) {


					dx = rsHardCoreEventOpenPos[sSodScore.SodNextStageNum][2].x*fONE;
					dy = rsHardCoreEventOpenPos[sSodScore.SodNextStageNum][2].y*fONE;
					dz = rsHardCoreEventOpenPos[sSodScore.SodNextStageNum][2].z*fONE;

					CloseEachPlayer();

					WarpFieldNearPos( rsSOD_FIELD,dx,dz , &x, &z );

					lpCurPlayer->SetPosi(dx,0, dz  ,0,0,0 );
					TraceCameraPosi.x = lpCurPlayer->pX;
					TraceCameraPosi.y = lpCurPlayer->pY;
					TraceCameraPosi.z = lpCurPlayer->pZ;
					TraceTargetPosi.x = lpCurPlayer->pX;
					TraceTargetPosi.y = lpCurPlayer->pY;
					TraceTargetPosi.z = lpCurPlayer->pZ;
			
					//바닥 확인
					lpCurPlayer->OnStageField = -1;
					if ( smGameStage[0] ) {
						cnt = smGameStage[0]->GetFloorHeight( lpCurPlayer->pX,lpCurPlayer->pY,lpCurPlayer->pZ ,lpCurPlayer->Pattern->SizeHeight );	
						lpCurPlayer->OnStageField = 0;
					}
					if ( cnt==CLIP_OUT && smGameStage[1] ) {
						lpCurPlayer->OnStageField = 1;
					}

					sSodScore.EffectStartCount = 70*1;
					sSodScore.EffectCode = sSodScore.NextRound;
				}
			}
			else if ( sSodScore.SodNextStageNum==-1 ) {
				//실패 탈락
				dx = sField[rsSOD_VILLAGE].cX*fONE;
				dz = sField[rsSOD_VILLAGE].cZ*fONE;

				CloseEachPlayer();

				WarpFieldNearPos( rsSOD_VILLAGE,dx,dz , &x, &z );

				lpCurPlayer->SetPosi(dx,0, dz  ,0,0,0 );
				TraceCameraPosi.x = lpCurPlayer->pX;
				TraceCameraPosi.y = lpCurPlayer->pY;
				TraceCameraPosi.z = lpCurPlayer->pZ;
				TraceTargetPosi.x = lpCurPlayer->pX;
				TraceTargetPosi.y = lpCurPlayer->pY;
				TraceTargetPosi.z = lpCurPlayer->pZ;
		
				//바닥 확인
				lpCurPlayer->OnStageField = -1;
				if ( smGameStage[0] ) {
					cnt = smGameStage[0]->GetFloorHeight( lpCurPlayer->pX,lpCurPlayer->pY,lpCurPlayer->pZ ,lpCurPlayer->Pattern->SizeHeight );	
					lpCurPlayer->OnStageField = 0;
				}
				if ( cnt==CLIP_OUT && smGameStage[1] ) {
					lpCurPlayer->OnStageField = 1;
				}
			}
			else if ( sSodScore.SodNextStageNum==-2 ) {
				//로딩 화면 종료
				if ( SoD_SetFontEffect.eBL_Type==E_BL_LODING ) {
					CheckBellatraFontEffect( &SoD_SetFontEffect , TRUE );	//로딩 종료
				}
			}

			lpCurPlayer->SetMotionFromCode( CHRMOTION_STATE_STAND );
			lpCurPlayer->dwNextMotionCode = 0;
			sSodScore.dwSoD_NextStageTime = 0;
		}
		else {
			if ( sSodScore.SodNextStageNum>=0 )
				lpCurPlayer->dwNextMotionCode = CHRMOTION_STATE_YAHOO;
		}

	}

	if ( sSodScore.EffectStartCount ) {
		sSodScore.EffectStartCount--;
		if ( sSodScore.EffectStartCount==0 ) {
			switch( sSodScore.EffectCode ) {
				case 1:
				case 2:
				case 3:
				case 4:
				case 5:
				case 6:
				case 7:
				case 8:
					//SoD 라운드 이펙트
					Sod_SodEffect( sSodScore.EffectCode );
					break;
			}
		}
	}


	if ( sSodScore.ScoreEffectCount ) {
		sSodScore.ScoreEffectCount -=8;
		if ( sSodScore.ScoreEffectCount<0 )
			sSodScore.ScoreEffectCount = 0;
	}

	//공성전 BlessCastle 승리 야호하기
	if ( rsBlessCastle.CastleMode==3 ) {
		if ( rsBlessCastle.dwMasterClan && rsBlessCastle.dwMasterClan==GetClanCode(lpCurPlayer->smCharInfo.ClassClan) ) {
			if ( !lpCurPlayer->dwNextMotionCode ) {
				lpCurPlayer->dwNextMotionCode = CHRMOTION_STATE_YAHOO;
			}
		}
	}


	return TRUE;

}



//Sod 팀순위 설정
int SodTeamSort()
{
	int cnt,cnt1;
	int r1,r2;

	for(cnt=0;cnt<4;cnt++) TeamScoreSort[cnt] = cnt;

	//sSodScore.TeamScore[cnt]

	for( cnt=0;cnt<4;cnt++) {
		for(cnt1=cnt;cnt1<4;cnt1++) {
			r1 = TeamScoreSort[cnt];
			r2 = TeamScoreSort[cnt1];

			if ( sSodScore.TeamScore[r1]<sSodScore.TeamScore[r2] ) {
				TeamScoreSort[cnt] = r2;
				TeamScoreSort[cnt1] = r1;
			}
		}
	}

	return TRUE;
}

//Sod점수 표시 함수
int	DisplaySodScore()
{
	char szBuff[256];
	int x,y;
	int cnt,cnt2;
	DWORD	dwColor;
	DWORD tM,tS;
	int len;
	int RnkCnt=0;

	if ( !SodScoreFlag ) {
		SodScoreFlag = TRUE;
		ZeroMemory( &sSodScore , sizeof(sSOD_SCORE) );
	}

	psSetDefaultRenderState();

	if ( SOD_StringCount ) {
		len = lstrlen( szSOD_String );
		if ( (SOD_StringCount&0x8)==0 ) {
			DrawFontImage( szSOD_String, MidX-len*16, 330, 
				RGBA_MAKE(bSOD_StringColor[0],bSOD_StringColor[1],bSOD_StringColor[2],255 ) , 2.0f );
		}
	}

	if ( sSodScore.dwDispTime>dwPlayTime && rsCheckHardCoreEventArea( lpCurPlayer->pX , lpCurPlayer->pY , lpCurPlayer->pZ )==TRUE ) {

		x=(WinSizeX/2)-80; y=8;

		wsprintf( szBuff , "ROUND %d" , sSodScore.Round );
		DrawFontImage( szBuff, x-40, y, RGBA_MAKE( 80,255,40,255 ) , 0.9f );

		if ( sSodScore.dwPlayTime ) {
			tM = (dwPlayTime - sSodScore.dwPlayTime)/1000;
			tS = tM%60;
			tM = tM/60;
			if ( tM>99 || tM<0 ) tM = 99;
		}
		else {
			tM = 0;
			tS = 0;
		}

		wsprintf( szBuff , "PlayTime %d:%d" , tM,tS );
		DrawFontImage( szBuff, x+160, y+2, RGBA_MAKE( 80,160,255,255 ) ,0.7f );

		y+= 24;

		wsprintf( szBuff , "Score %d" , sSodScore.Score );

		if ( sSodScore.ScoreEffectCount ) 
			DrawFontImage( szBuff, x-40, y+8, RGBA_MAKE( 255,128,64,255-sSodScore.ScoreEffectCount ) , 0.9f );
		else
			DrawFontImage( szBuff, x-40, y+8, RGBA_MAKE( 255,255,0,255 ) , 0.9f  );


		SodTeamSort();

		for(cnt=0;cnt<4;cnt++) {
			cnt2 = TeamScoreSort[cnt];

			if ( sSodScore.MyTeam==cnt2 )
				dwColor = RGBA_MAKE( 255, 255, 0, 255 );
			else
				dwColor = RGBA_MAKE( 255, 128, 0, 255 );

			DrawSpecialFontImage( TeamMarkSort[cnt2] , x+160, y , RGBA_MAKE( 255,255,255,255 ) , 0.7f );

			wsprintf( szBuff , "Team %d" ,  sSodScore.TeamScore[cnt2]  );
			DrawFontImage( szBuff, x+190, y, dwColor , 0.7f );
			y+=18;
		}
	}

	return FALSE;
}






