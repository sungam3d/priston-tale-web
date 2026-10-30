#include "smlib3d\\smd3d.h"
#ifdef PT_GFX_GL
#include "ptgfx\\ptgfx.h"			// real-time shadows replace the round ones (DrawShadow)
#endif
#include "smwsock.h"

#include "character.h"
#include "srcsound\\dxwav.h"
#include "particle.h"
#include "fileread.h"
#include "playmain.h"
#include "drawsub.h"
#include "netplay.h"
#include "field.h"
#include "hobaram\\holinkheader.h"
#include "sinbaram\\sinlinkheader.h"

#include "effectsnd.h"
#include "playsub.h"
#include "skillsub.h"
#include "language.h"

#include "tjboy\\clanmenu\\tjclanDEF.h"
#include "tjboy\\clanmenu\\tjclan.h"
#include "tjboy\\clanmenu\\cE_CViewClanInfo.h"

#include "damage.h"
#include "srcserver\\svr_Damage.h"
#include "srcserver\\onserver.h"
#include "FullZoomMap.h"		// pluto 죽었을때 지도 못 열게 하려고

//추가 회피 데미지 출력 옵션
#define DISP_DAMAGE

#ifdef	DISP_DAMAGE
int rsDisplayDamgeToClient( rsPLAYINFO *lpPlayInfo , char *szDmgMsg );
#endif

#define		SUCCUBUS_CURSE_TIME		10

//PK시 40% 흡수력 적용
#define		PKMODE_ABSORB			30

extern BYTE VRKeyBuff[256];
extern int MouseButton[3];
extern DWORD dwPlayServTime;					//서버모드일때 시간

static int MatMBlur;							//메트리얼 번호

static smTEXLINK smTexLinkMotionBlur[128];		//텍스쳐 좌표
static smVERTEX MbVertex[64];
static smFACE MbFace[64];

static smTEXLINK smTexLinkMotionBlur2[64];		//텍스쳐 좌표
static smVERTEX MbVertex2[32];
static smFACE MbFace2[32];

#define	AGING_SUB_LEVEL	10

int	SkillChargingFlag = 0;			//스킬 차징 플랙
int	SkillEventDamageCount;			//스킬 데미지 카운터

TRANS_SKIL_ATTACKDATA	Trans_SplashCharList;		//팅겨서 맞는 공격 데이타 임시저장

//크리티컬 섹션
CRITICAL_SECTION	cLoadSection;

int DrawMotionBlur();

/////////////////////// 서버측 함수 불러다 쓴다 //////////////////////
//이벤트 지역인지 첵크
int rsCheckHardCoreEventArea( int x, int y, int z ) ;
extern int	rsHardCoreEventInterLineZ;
extern int	rsHardCoreEventInterLineZ2;
extern int	rsHardCoreEventPosY;
extern int	rsHardCoreEventOutLineZ;
extern rsSERVER_CONFIG		rsServerConfig;

//특정위치에 보조몬스터 강제 출현 시킴
smCHAR *rsOpenSubMonster( smCHAR *lpMainChar , DWORD dwCharSoundCode , int x, int y, int z , int MaxFlag );
//해당 몬스터 출신 깃발 의 활성화 몬스터 수
int rsGetMonCountFlag( smCHAR *lpChar );


//상대 캐릭터에 기술 공격 가함 
int rsSendTransSkillAttack( smCHAR *lpChar , rsPLAYINFO *lpPlayInfo );
//서버 - 홀리 리플렉션 공격 처리
int rsProcessAttack_SkillHolyReflection( rsPLAYINFO *lpPlayInfo , smCHAR *lpChar );
//주변 몬스터를 찾는다
smCHAR *rsFindAroundMonster( smCHAR *lpChar , int RoundDist);
//주변 몬스터를 찾아 데미지 공격
smCHAR *rsSkillDamage_AroundMonster( smCHAR *lpChar );
//주변 몬스터를 찾아 스킬 ( 라투 주변 몬스터 스턴제거 )
smCHAR *rsSkillMonster_AroundMonster( smCHAR *lpChar , int RoundDist );

//////////////////////////////////////////////////////////////////////

int GetProjPosi( int &x3d , int &y3d, int &z3d , POINT3D *CameraPosi , POINT3D *CameraAngle );
//상대 캐릭터에 기술 공격 가함 
int rsSendTransSkillAttack( smCHAR *lpChar , rsPLAYINFO *lpPlayInfo );

//쉐도우 마스타 연타 타격 점
int	SkillShadowMasterEvent[9][7] = {
	{ 0,0,0,0,0,0,0 },
	{ 0,0,0,0,0,0,0 },
	{ 0,0,0,0,0,0,0 },

	{ 1,3,5,0,0,0,0 },		//3 번 때림
	{ 1,3,5,6,0,0,0 },
	{ 1,2,3,4,6,0,0 },
	{ 1,2,3,4,6,7,0 },
	{ 1,2,3,4,5,6,7 },		//7 번 때림

	{ 1,2,3,4,5,6,7 }
};

//공성전 필드 좌표 정보
int CastleBattleZone_LineZ = -24043*fONE;
int	CastleBattleZone_DoorLineZ = -25545*fONE;								//성문 입구
int	CastleBattleZone_DoorLineX[2] = { 34407*fONE , 34695*fONE };			//성문 입구
int	CastleBattleZone_BridgeZ = -25258*fONE;									//다리 입구
int CastleSoulFountain_Area[4] = {	37040*fONE , 37158*fONE , -30286*fONE, -30102*fONE };	//영혼의샘터



//저항 속성 계산
int GetResistancePower( int power , int ResCode )
{
	int rs,result;

	rs = lpCurPlayer->smCharInfo.Resistance[ResCode];

	if ( rs<0 ) rs = 0;
	if ( rs>=100 ) rs=100;
	rs = 100-rs;

	result = (power*rs)/100;

	if ( result<=0 ) result=4;

	return result;
}


//######################################################################################
//작 성 자 : 오 영 석
int			Debug_TalkZoomMode;
int			Debug_TalkRepeatMode;
int			Debug_TalkRepeatModeType;

void DrawTalkZoom( smCHAR *lpChar, smPAT3D *lpPattern, float fx, float fy )
{
	if( lpChar == NULL )
		return;

	POINT3D TalkCameraPos;

	smOBJ3D	*lpObj = lpPattern->GetObjectFromName("Head");
	if( lpObj == NULL )
	{
		TalkCameraPos.y = lpChar->PatHeight - 10*fONE;
		TalkCameraPos.x = 0;
		TalkCameraPos.z = 0;
	}
	else
	{
		smMATRIX *mWorld;
		POINT3D angle;

		ZeroMemory( &angle, sizeof( POINT3D ) );
		angle.y = (-angle.y+ANGLE_180)&ANGCLIP;

		mWorld = &lpObj->mWorld;

		AnimObjectTree( lpObj, lpChar->frame, angle.x, angle.y, angle.z );

		TalkCameraPos.x = mWorld->_41;
		TalkCameraPos.z = mWorld->_42;
		TalkCameraPos.y = mWorld->_43;
	}

	if( lpChar->smCharInfo.State == smCHAR_STATE_ENEMY )
	{
		TalkCameraPos.y += lpChar->smCharInfo.ArrowPosi[0]*fONE;
		TalkCameraPos.z += lpChar->smCharInfo.ArrowPosi[1]*fONE;
	}

	//
	int x,y,z,w,h;
	int mx,my,he;
	POINT3D angle;
	int dist;
	int	ObjSizeBack;

	mx = MidX;
	my = MidY;

	MidX = (int)((float)WinSizeX * fx );
	MidY = (int)((float)WinSizeY * fy );

	int chrHeight;	
	if ( lpChar->smCharInfo.State <= TRUE )  
		chrHeight = lpChar->PatHeight - 8*fONE;
	else
		chrHeight = lpChar->PatHeight;

	w = (int)((float)WinSizeX/4);
	h = (int)((float)WinSizeY/2);

	w += 20;
	h += 20;

	smRender.SMSHIFT_PERSPECTIVE_WIDTH = RENDCLIP_DEFAULT_SHIFT_PERSPECTIVE_WIDTH*3;
	smRender.SMMULT_PERSPECTIVE_HEIGHT = RENDCLIP_DEFAULT_MULT_PERSPECTIVE_HEIGHT*2;

	dist = 30*fONE;
	he = -1*fONE;

	dist += TalkCameraPos.y;

	extern int	MatEachBox;
	if( MatEachBox >= 0 )
	{
		lpD3DDevice->BeginScene();
		dsDrawTexImage( MatEachBox , MidX-(w>>1), MidY-(h>>1), w,h, 255, 0 );
		lpD3DDevice->EndScene();
	}

	lpD3DViewport->Clear2( 1UL, &D3DRect, D3DCLEAR_ZBUFFER , smBackColor, 1, 0L );
	smCHAR_SetCameraPosi( TalkCameraPos.x , TalkCameraPos.y+he, dist+TalkCameraPos.z, 0, ANGLE_180,0 );

	x = lpChar->pX;
	y = lpChar->pY;
	z = lpChar->pZ;
	memcpy( &angle, &lpChar->Angle, sizeof( POINT3D ) );

	lpChar->pX = 0;
	lpChar->pY = 0;
	lpChar->pZ = 0;
	ZeroMemory( &lpChar->Angle, sizeof( POINT3D ) );

	ObjSizeBack = lpChar->smCharInfo.wPlayClass[1];
	lpChar->smCharInfo.wPlayClass[1] = 0;
	
	lpChar->Draw();

	lpChar->smCharInfo.wPlayClass[1] = ObjSizeBack;

	lpChar->pX = x;
	lpChar->pY = y;
	lpChar->pZ = z;
	memcpy( &lpChar->Angle, &angle, sizeof( POINT3D ) );
	
	MidX = mx;
	MidY = my;

	smRender.SMSHIFT_PERSPECTIVE_WIDTH = RENDCLIP_DEFAULT_SHIFT_PERSPECTIVE_WIDTH;
	smRender.SMMULT_PERSPECTIVE_HEIGHT = RENDCLIP_DEFAULT_MULT_PERSPECTIVE_HEIGHT;
}
//######################################################################################


//도구 위치 포인트 구하기
int smCHAR::GetToolBipPoint( smCHARTOOL *ChrTool, int *nX, int *nY, int *nZ )
{

	smMATRIX *mWorld;
	POINT3D angle;
	int tx,ty,tz;
	smOBJ3D	*lpObj;
	int pframe;
	int rx,ry,rz;

	if ( !DisplayFlag ) return FALSE;

	tx = 0;
	ty = 0;
	tz = 0;

	memcpy( &angle, &Angle , sizeof( POINT3D ) );
	angle.y = (-angle.y+ANGLE_180)&ANGCLIP;
	lpObj = ChrTool->ObjBip;

	if ( lpObj ) {
			mWorld = &lpObj->mWorld;
			pframe = frame;

			AnimObjectTree( lpObj , pframe , angle.x , angle.y , angle.z );

			rx = tx * mWorld->_11 +
				 ty * mWorld->_21 +
				 tz * mWorld->_31 ;

			ry = tx * mWorld->_12 +
				 ty * mWorld->_22 +
				 tz * mWorld->_32 ;

			rz = tx * mWorld->_13 +
				 ty * mWorld->_23 +
				 tz * mWorld->_33 ;

			*nX = pX + (rx>>FLOATNS) + mWorld->_41;
			*nZ = pZ + (ry>>FLOATNS) + mWorld->_42;
			*nY = pY + (rz>>FLOATNS) + mWorld->_43;

			return TRUE;
	}

	return FALSE;
}



//공격 포인트 좌표를 구한다
int smCHAR::GetAttackPoint( int *nX, int *nY, int *nZ )
{

	smMATRIX *mWorld;
	POINT3D angle;
	int tx,ty,tz;
	smCHARTOOL *ChrTool = 0;
	smOBJ3D	*lpObj;
	int pframe;
	int rx,ry,rz;
/*
	if ( HvLeftHand.PatTool ) 
		ChrTool = &HvLeftHand;

	if ( HvRightHand.PatTool ) 
*/
	ChrTool = &HvRightHand;

	if ( !DisplayFlag ) return FALSE;

	tx = 0;
	ty = 0;

	if ( ChrTool->PatTool )
		tz = ChrTool->SizeMax/2;
	else
		tz = 0;

	memcpy( &angle, &Angle , sizeof( POINT3D ) );
	angle.y = (-angle.y+ANGLE_180)&ANGCLIP;

	if ( AttackObjBip && !tz && !ShootingMode )
		lpObj = AttackObjBip;
	else
		lpObj = ChrTool->ObjBip;

	if ( lpObj ) {
			mWorld = &lpObj->mWorld;
			pframe = frame;

			AnimObjectTree( lpObj , pframe , angle.x , angle.y , angle.z );

			rx = tx * mWorld->_11 +
				 ty * mWorld->_21 +
				 tz * mWorld->_31 ;

			ry = tx * mWorld->_12 +
				 ty * mWorld->_22 +
				 tz * mWorld->_32 ;

			rz = tx * mWorld->_13 +
				 ty * mWorld->_23 +
				 tz * mWorld->_33 ;

			*nX = pX + (rx>>FLOATNS) + mWorld->_41;
			*nZ = pZ + (ry>>FLOATNS) + mWorld->_42;
			*nY = pY + (rz>>FLOATNS) + mWorld->_43;

			return TRUE;
	}

	return FALSE;
}



//아이템 번호 찾기 코드로
int GetSinItemNumFromCode( DWORD CODE )
{
	for(int j=0 ; j < MAX_ITEM ; j++){
			if(sItem[j].CODE == CODE){	
				return j;
			}

	}
	return -1;
}

//2점의 거리를 구한다 ( 반환은 거리의 제곱값 )
int GetDistanceDbl( int x1, int z1 , int x2, int z2 )
{
	int ax,az;

	ax = (x1 - x2 ) >>FLOATNS;
	az = (z1 - z2 ) >>FLOATNS;
	ax *=ax;
	az *=az;
	return ax+az;
}

//위치를 검사하여 일정크기 박스크기로 확인
int CheckLocateBox( int x1, int y1 , int z1, int x2, int y2, int z2 , int size )
{
	if ( abs(x1-x2)<size && abs(y1-y2)<size && abs(z1-z2)<size )
		return TRUE;

	return FALSE;
}


//특정 오브젝트의 트리를 추적하여 애니메이션 시킴
int AnimObjectTree( smOBJ3D *tObj , int frame , int ax, int ay, int az )
{
	smOBJ3D *ObjList[32];
	smOBJ3D *obj;
	int ObjCnt;
	int cnt;

	ObjCnt = 0;
	obj = tObj;

	while(1) {
		ObjList[ ObjCnt++ ] = obj;
		if ( !obj->pParent ) break;
		obj = obj->pParent;
	}

	for( cnt=ObjCnt-1 ; cnt>=0; cnt-- ) {
		ObjList[ cnt ]->TmAnimation( frame , ax , ay , az );
	}

	return TRUE;
}


char *szBipName_RightHand = "Bip weapon01";//"Bip01 R Hand";//
char *szBipName_LeftHand = "Bip01 L Hand";
char *szBipName_Shield = "Bip01 L Forearm";
char *szBipName_Shield2 = "Bip-shield";
char *szBipName_BackSpine = "Bip in01";
char *szBipName_BackSpineCross = "Bip in-bow";
char *szBipName_BackSpineBow = "Bip in-cro";
char *szBipName_RFoot = "Bip01 R Foot";
char *szBipName_LFoot = "Bip01 L Foot";
char *szBipName_Head = "Bip01 Head";


//char *szBipName_BackSpine = "Bip01 Neck";

int smCHAR_CameraX;
int smCHAR_CameraY;
int smCHAR_CameraZ;

int smCHAR_CameraAngX;
int smCHAR_CameraAngY;
int smCHAR_CameraAngZ;


// 캐릭터에 사용할 카메라 위치를 설정한다
int smCHAR_SetCameraPosi( int x,int y,int z, int angX,int angY,int angZ )
{
	smCHAR_CameraX = x;
	smCHAR_CameraY = y;
	smCHAR_CameraZ = z;

	smCHAR_CameraAngX = angX;
	smCHAR_CameraAngY = angY;
	smCHAR_CameraAngZ = angZ;

	return TRUE;
}

////////////////////////////////////////////////////////////////////


//고속 첵크용 코드 제작
DWORD GetSpeedSum( char *szName )
{
	DWORD cnt;
	DWORD Sum1,Sum2;
	BYTE ch;
	DWORD  dwSum;

	Sum2 = 0;
	dwSum = 0;

	cnt = 0;

	while(1) {
		ch = (BYTE)szName[cnt];
		if ( ch==0 ) break;
		if ( ch>='a' && ch<='z' ) {//대문자 소문자로
			Sum2 += (ch-0x20)*(cnt+1);
			dwSum += (ch-0x20)*(cnt*cnt);
		}
		else {
			Sum2 += (ch*(cnt+1));
			dwSum += ch*(cnt*cnt);
		}
		cnt++;
	}

	Sum1 = cnt;

	return (dwSum<<24)|(Sum1<<16)|Sum2;
}

//######################################################################################
// 작 성 자 : 오 영 석	
smPATTERN::smPATTERN(void)
{
	Init();
}


smPATTERN::~smPATTERN(void)
{
	Close();
}
//######################################################################################


//초기화
void smPATTERN::Init()
{
	int cnt;

	for(cnt=0;cnt<DPAT_MAX;cnt++) 
		ZeroMemory( &DinaPat[cnt] , sizeof( smDPAT ) );

	BipPattern = 0;
}

//말기화
int smPATTERN::Close()
{
	int cnt;

	//smMODELINFO

	for(cnt=0;cnt<DPAT_MAX;cnt++) {
		if ( DinaPat[cnt].Pat ) {
			delete DinaPat[cnt].Pat;
			DinaPat[cnt].Pat = 0;
			if ( DinaPat[cnt].lpModelInfo ) {
				delete DinaPat[cnt].lpModelInfo;
				DinaPat[cnt].lpModelInfo = 0;
			}
		}
	}

	return TRUE;
}


//이릉으로 패턴번호를 찾음
int smPATTERN::FindFromName( char *szName )
{
	int cnt;

	for( cnt=0;cnt<DPAT_MAX;cnt++) {
		if ( DinaPat[cnt].Pat ) {
			if ( lstrcmpi( szName , DinaPat[cnt].szPatName )==0 ) {
				return cnt;
			}
		}
	}

	return -1;
}

//이릉으로 패턴번호를 찾음
int smPATTERN::FindFromCode( DWORD dwCode )
{
	int cnt;

	for( cnt=0;cnt<DPAT_MAX;cnt++) {
		if ( DinaPat[cnt].Pat && dwCode==DinaPat[cnt].dwSpeedFindSum ) 
			return cnt;
	}

	return -1;
}

//새로운 패턴 위치를 얻음
int smPATTERN::GetNew()
{
	int cnt;

	for( cnt=0;cnt<DPAT_MAX;cnt++) {
		if ( DinaPat[cnt].Pat==0 ) 
			return cnt;
	}

	return -1;
}

//뼉따귀 모션 데이타를 불러와 등록
smDPAT *smPATTERN::LoadBipPattern( char *szFileName ) 
{
	int Num;
	DWORD dwCode;


	//Num = FindFromName( szFileName );
	dwCode = GetSpeedSum(szFileName);
	Num = FindFromCode(dwCode);

	if ( Num<0 ) {
		//AddChatBuff( szFileName );

		Num = GetNew();
		if ( Num>=0 ) {
			DinaPat[Num].Pat =smASE_ReadBone( szFileName );		// 애니메이션 읽기
			if ( DinaPat[Num].Pat ) {
				DinaPat[Num].smDinaLink = 0;
				lstrcpy( DinaPat[Num].szPatName , szFileName );
				DinaPat[Num].dwSpeedFindSum = dwCode;
				DinaPat[Num].UseCount =1;
				return &DinaPat[Num];
			}
		}
		else
			return NULL;
	}

	DinaPat[Num].UseCount ++;

	return &DinaPat[Num];
}

//껍데기 데이타를 불러와 등록
smDPAT *smPATTERN::LoadCharactor( char *szFileName ) 
{
	int Num,sframe;
	DWORD cnt;
	smDPAT *dMotionPat;
	smPAT3D	*AnimPat;
	DWORD	dwCode;

	smMODELINFO	modelInfo;


	//Num = FindFromName( szFileName );
	dwCode = GetSpeedSum(szFileName);
	Num = FindFromCode( dwCode );//GetSpeedSum(szFileName) );

	if ( Num<0 ) {

		if ( smModelDecode( szFileName , &modelInfo )==NULL ) 
			return NULL;

		//AddChatBuff( szFileName );

		smASE_SetPhysique( 0 );

		Num = GetNew();

		if ( Num<0 ) return NULL;

		if ( BipPattern && modelInfo.szMotionFile[0] ) {
			dMotionPat = BipPattern->LoadBipPattern(modelInfo.szMotionFile);
			if ( dMotionPat ) {
				smASE_SetPhysique( dMotionPat->Pat );
				DinaPat[ Num ].smDinaLink = dMotionPat;
			}
			else return NULL;
		}
		else
			DinaPat[ Num ].smDinaLink = 0;

		//######################################################################################
		//작 성 자 : 오 영 석
		if( BipPattern && modelInfo.szTalkMotionFile[0] )
		{
			smDPAT *lpTalkLink = BipPattern->LoadBipPattern( modelInfo.szTalkMotionFile );
			if( lpTalkLink )
				DinaPat[ Num ].lpTalkLink = lpTalkLink;
			else
				return NULL;
		}
		else
			DinaPat[ Num ].lpTalkLink = NULL;
		//######################################################################################

		DinaPat[Num].Pat = smASE_Read( modelInfo.szModelFile );// , modelInfo.szModelName );
		if ( !DinaPat[Num].Pat ) {
			if ( smConfig.DebugMode ) {
				//로딩 에러 메세지 출력
				char	szErrorMsg[128];
				wsprintf( szErrorMsg , "Loading Error , ( %s )" , modelInfo.szModelFile );
				AddChatBuff( szErrorMsg );
			}
			return NULL;
		}

		if ( DinaPat[ Num ].smDinaLink )
			AnimPat = DinaPat[ Num ].smDinaLink->Pat;
		else
			AnimPat = DinaPat[Num].Pat;

		//에니메이션 프레임 보정
		if ( AnimPat && AnimPat->TmFrameCnt>0 ) {
			for(cnt=0;cnt<modelInfo.MotionCount;cnt++) {
				sframe = AnimPat->TmFrame[ modelInfo.MotionInfo[cnt].MotionFrame-1 ].StartFrame/160;
				modelInfo.MotionInfo[cnt].StartFrame += sframe;
				modelInfo.MotionInfo[cnt].EndFrame  += sframe;
			}
		}

		DinaPat[Num].lpModelInfo = new smMODELINFO;
		memcpy( DinaPat[Num].lpModelInfo , &modelInfo , sizeof(smMODELINFO) );
		lstrcpy( DinaPat[Num].szPatName , szFileName );
		DinaPat[Num].dwSpeedFindSum = dwCode;
		DinaPat[Num].UseCount =1;
		return &DinaPat[Num];
	}

	DinaPat[Num].UseCount ++;
	return &DinaPat[Num];
}


extern smPAT3D *PatKnife;
char *szDefaultCharName = "char\\woman.ini";

//모델 로더 초기화
int InitLoader();
//모델 로딩을 위한 쓰레드 생성
int CreateLoaderThread();
DWORD WINAPI smLoaderThreadProc( void *pInfo );

struct smDATALOADER	{
	smSTAGE3D	*lpStage;
	smCHAR		*lpChar;
	scITEM		*lpItem;


	char		szLoadName[64];
	char		szLoadName2[64];

	int			Result;
	int			Cancel;				//취소 요구
};

#define DATALOADER_MAX		64
#define DATALOADER_MASK		63

smDATALOADER	smLoader[DATALOADER_MAX];
int	smLoaderCnt;
int smLoaderEnd;
int	ThreadLoaderStats;				//쓰레드 상태

smPATTERN smBipPattern;
smPATTERN smPattern;

//배경을 새 쓰레드에 통하여 로드 한다
int AddLoaderStage( smSTAGE3D *lpStage , char *szName )
{
	int cnt;

	cnt = smLoaderEnd & DATALOADER_MASK;

	smLoaderEnd++;

	smLoader[cnt].lpStage		= lpStage;
	smLoader[cnt].lpChar		= 0;
	smLoader[cnt].lpItem		= 0;
	smLoader[cnt].Cancel		= 0;
	smLoader[cnt].Result 		= 0;
	lstrcpy( smLoader[cnt].szLoadName , szName );

	//모델 로딩을 위한 쓰레드 생성
	CreateLoaderThread();

	return smLoaderEnd;
}

//아이템을 새 쓰레드에 통하여 로드 한다
int	AddLoaderPattern( smCHAR *lpChar , char *szName , char *szName2 )
{
	int cnt;

	cnt = smLoaderEnd & DATALOADER_MASK;

	smLoaderEnd++;

	smLoader[cnt].lpStage		= 0;
	smLoader[cnt].lpChar		= lpChar;
	smLoader[cnt].lpItem		= 0;
	smLoader[cnt].Cancel		= 0;
	smLoader[cnt].Result 		= 0;
	lstrcpy( smLoader[cnt].szLoadName , szName );

	if ( szName2 ) 
		lstrcpy( smLoader[cnt].szLoadName2 , szName2 );
	else
		smLoader[cnt].szLoadName2[0] = 0;

	smLoader[cnt].lpChar->PatLoading = TRUE;

	//모델 로딩을 위한 쓰레드 생성
	CreateLoaderThread();

	return TRUE;
}




//캐릭터를 새 쓰레드에 통하여 로드 한다
int	AddLoaderItem( scITEM *lpItem , char *szName )
{
	int cnt;

	cnt = smLoaderEnd & DATALOADER_MASK;

	smLoaderEnd++;

	smLoader[cnt].lpStage		= 0;
	smLoader[cnt].lpChar		= 0;
	smLoader[cnt].lpItem		= lpItem;
	smLoader[cnt].Cancel		= 0;
	smLoader[cnt].Result 		= 0;
	lstrcpy( smLoader[cnt].szLoadName , szName );

	smLoader[cnt].lpItem->PatLoading = TRUE;

	//모델 로딩을 위한 쓰레드 생성
	CreateLoaderThread();

	return TRUE;
}


DWORD WINAPI smLoaderThreadProc( void *pInfo )
{
//	HANDLE	hThread;

	int	cnt;
	smDPAT	*smDPat;
	smDPAT  *smDPat2;
//	int Result;

	smDPAT *lpPatOld;
	smDPAT *lpPatOld2;

//	char szDebugPrint[256];

	//크리티칼 섹션 선언
	EnterCriticalSection( &cLoadSection );

//	while( smLoaderCnt<smLoaderEnd ) {
		cnt = smLoaderCnt & DATALOADER_MASK;

		//배경 로드
		if ( smLoader[cnt].lpStage ) {
			//로딩중 화면 표시
			OpenDispLoading();

			if ( smSTAGE3D_ReadASE( smLoader[cnt].szLoadName , smLoader[cnt].lpStage ) ) {
				if ( lpD3DDevice ) {
					//######################################################################################
					//작 성 자 : 오 영 석
					//텍스쳐 로딩
					smLoader[cnt].lpStage->smMaterialGroup->ReadTextures(1);
					//######################################################################################

					//스왑 텍스쳐 확인
					smLoader[cnt].lpStage->smMaterialGroup->CheckMatreialTextureSwap();	
				}
				smLoader[cnt].Result = TRUE;
				smLoader[cnt].lpStage->Head = TRUE;
			}
			//로딩 표시종료
			CloseDispLoading();
		}

		//아이템 로드
		if ( smLoader[cnt].lpItem ) {
			smLoader[cnt].lpItem->PatLoading = TRUE;
			//AddChatBuff( smLoader[cnt].szLoadName );

			lpPatOld = smLoader[cnt].lpItem->lpDinaPattern;
			smDPat = smPattern.LoadCharactor( smLoader[cnt].szLoadName );

			if ( smDPat ) {
				if ( lpD3DDevice && smDPat->UseCount==1 ) {

					//wsprintf( szDebugPrint , "TEX_READ:%s",smLoader[cnt].szLoadName );
					//AddChatBuff( szDebugPrint );

					//ReadTextures();
					//스왑 텍스쳐 확인
					if ( smDPat && smDPat->Pat ) {
						smDPat->Pat->smMaterialGroup->ReadTextures();
						smDPat->Pat->smMaterialGroup->CheckMatreialTextureSwap();	
					}
				}

				//캐릭터 스킨 확인 초기화
				//smLoader[cnt].lpChar->Init();

				smLoader[cnt].lpItem->DisplayFlag = 0;

				smLoader[cnt].lpItem->SetPattern( smDPat->Pat );
				smLoader[cnt].lpItem->lpDinaPattern = smDPat;
				lstrcpy( smLoader[cnt].lpItem->szModelName , smDPat->szPatName );
				smLoader[cnt].lpItem->DisplayFlag = TRUE;

			}
			smLoader[cnt].Result = TRUE;

			if ( smDPat && lpPatOld )
				smDPAT_Delete( lpPatOld );

			smLoader[cnt].lpItem->PatLoading = FALSE;
		}



		//캐릭터 로드
		if ( smLoader[cnt].lpChar ) {
			smDPat = 0;
			smDPat2 = 0;

			smLoader[cnt].lpChar->PatLoading = TRUE;
			//AddChatBuff( smLoader[cnt].szLoadName );

			lpPatOld = smLoader[cnt].lpChar->lpDinaPattern;
			lpPatOld2 = smLoader[cnt].lpChar->lpDinaPattern2;

			smDPat = smPattern.LoadCharactor( smLoader[cnt].szLoadName );

			if ( smDPat ) {
				//캐릭터 스킨 확인 초기화
				//smLoader[cnt].lpChar->Init();

				smLoader[cnt].lpChar->DisplayFlag = 0;

				smLoader[cnt].lpChar->SetDinaPattern( smDPat );

				smLoader[cnt].lpChar->SetMotionFromCode( CHRMOTION_STATE_STAND );

//				smLoader[cnt].lpChar->SetTool( PatKnife ,  hvPOSI_RHAND );
				//smLoader[cnt].lpChar->ActionPattern = 99;
				smLoader[cnt].lpChar->Flag = 1;
				lstrcpy( smLoader[cnt].lpChar->smCharInfo.szModelName , smDPat->szPatName );


				smLoader[cnt].lpChar->smCharInfo.szModelName2[0] = 0;

				if ( smDPat->lpModelInfo->szSubModelFile[0] ) {
					//보조 모델 가져오기 ( 메인모델에 보조 모델 파일이 존재할경우 )
					smDPat2 = smPattern.LoadCharactor( smDPat->lpModelInfo->szSubModelFile );
				}
				else {
					//보조 모델 로딩이 설정되 있는 경우 
					if ( smLoader[cnt].szLoadName2[0] ) {
						smDPat2 = smPattern.LoadCharactor( smLoader[cnt].szLoadName2 );
						lstrcpy( smLoader[cnt].lpChar->smCharInfo.szModelName2 , smDPat2->szPatName );
					}
				}

				smLoader[cnt].lpChar->SetDinaPattern2( smDPat2 );

				//테스쳐 로딩
				if ( lpD3DDevice && ( smDPat->UseCount==1 || (smDPat2 && smDPat2->UseCount==1 ) ) ) {
					//wsprintf( szDebugPrint , "TEX_READ:%s",smLoader[cnt].szLoadName );
					//AddChatBuff( szDebugPrint );

					//ReadTextures();



					//스왑 텍스쳐 확인
					if ( smDPat && smDPat->Pat ) {
						smDPat->Pat->smMaterialGroup->ReadTextures();
						smDPat->Pat->smMaterialGroup->CheckMatreialTextureSwap();	
					}
					if ( smDPat2 && smDPat2->Pat ) {
						smDPat2->Pat->smMaterialGroup->ReadTextures();	
						smDPat2->Pat->smMaterialGroup->CheckMatreialTextureSwap();	
					}
				}

				smLoader[cnt].lpChar->DisplayFlag = TRUE;

			}
			else {
				/*
				//현재 하드에 없는 캐릭터 스킨
				smDPat = smPattern.LoadCharactor( szDefaultCharName );

				if ( lpD3DDevice && smDPat->UseCount==1 ) {
					//wsprintf( szDebugPrint , "TEX_READ:%s",szDefaultCharName );
					//AddChatBuff( szDebugPrint );
					ReadTextures();
					//스왑 텍스쳐 확인
					if ( smDPat && smDPat->Pat ) smDPat->Pat->smMaterialGroup->CheckMatreialTextureSwap();	
				}

				smLoader[cnt].lpChar->DisplayFlag = 0;

				//캐릭터 스킨 확인 초기화
				//smLoader[cnt].lpChar->Init();
				smLoader[cnt].lpChar->SetDinaPattern( smDPat );
				smLoader[cnt].lpChar->SetMotionFromCode( CHRMOTION_STATE_STAND );

//				smLoader[cnt].lpChar->SetTool( PatKnife ,  hvPOSI_RHAND );
				//smLoader[cnt].lpChar->ActionPattern = 99;
				smLoader[cnt].lpChar->Flag = 1;
				lstrcpy( smLoader[cnt].lpChar->smCharInfo.szModelName , smDPat->szPatName );
				smLoader[cnt].lpChar->DisplayFlag = TRUE;		
				*/
				DisconnectFlag = dwPlayTime;
			}
			smLoader[cnt].Result = TRUE;

			if ( lpPatOld ) smDPAT_Delete( lpPatOld );
			if ( lpPatOld2 ) smDPAT_Delete( lpPatOld2 );
/*
			if ( smDPat && lpPatOld ) {
				if ( smDPat2 && lpPatOld2 ) smDPAT_Delete( lpPatOld2 );
				smDPAT_Delete( lpPatOld );
			}
*/

			smLoader[cnt].lpChar->PatLoading = FALSE;
		}
		smLoaderCnt++;
//	}

	//크리티칼 섹션 해제
	LeaveCriticalSection( &cLoadSection );

	ThreadLoaderStats = FALSE;				//쓰레드 상태
	//ExitThread( TRUE );


	return TRUE;
}

//모델 로딩을 위한 쓰레드 생성
int CreateLoaderThread()
{
//	DWORD dwMsgId;
//	HANDLE	handle;
/*
//	if ( ThreadLoaderStats==FALSE ) {
		handle = CreateThread( NULL , 0, smLoaderThreadProc , 0 , 0, &dwMsgId );
//	}

	SetThreadPriority( handle , THREAD_PRIORITY_HIGHEST );		//클래스를 높게
*/

	smLoaderThreadProc( 0 );

	return FALSE;
}
//모델 로더 초기화
int InitLoader()
{
	smLoaderCnt = 0;
	smLoaderEnd = 0;

	return TRUE;
}

//패턴버퍼를 초기화
int InitPatterns()
{
	smBipPattern.Init();
	smPattern.Init();

	smPattern.BipPattern = &smBipPattern;

	//모델 로더 초기화
	InitLoader();


	//크리티컬 섹션 초기화
	InitializeCriticalSection( &cLoadSection );

//	CRITICAL_SECTION	cPatSection;


	return TRUE;
}

//패턴버퍼를 말기화
int ClosePatterns()
{
	smBipPattern.Close();
	smPattern.Close();

	return TRUE;
}

//제거
int smDPAT_Delete( smDPAT *dPat )
{
	if ( dPat->UseCount>0 ) {
		dPat->UseCount--;

		//######################################################################################
		//작 성 자 : 오 영 석
		if( dPat->lpTalkLink && dPat->UseCount == 0 )
			smDPAT_Delete( dPat->lpTalkLink );
		//######################################################################################

		if ( dPat->smDinaLink && dPat->UseCount==0 )
			smDPAT_Delete( dPat->smDinaLink );

		if ( dPat->UseCount<=0 ) {
			delete dPat->Pat;
			dPat->Pat = 0;
			if ( dPat->lpModelInfo ) {
				delete dPat->lpModelInfo;
				dPat->lpModelInfo = 0;
			}
		}
	}
	return NULL;
}


//캐릭터에 패턴을 설정
int SetLoadPattern( smCHAR *smChar , char *szName , char *szName2 , POINT3D *Posi , POINT3D *Angle )
{
	smDPAT	*smDPat;
	smDPAT	*smDPat2;

	smDPat = 0;
	smDPat2 = 0;

	smDPat = smPattern.LoadCharactor( szName );

	if ( smDPat ) {
		smChar->Init();
		if ( Posi && Angle ) 
			smChar->SetPosi( Posi->x, Posi->y, Posi->z , Angle->x, Angle->y, Angle->z );

		smChar->SetDinaPattern( smDPat );
		smChar->ChangeMotion( 0 );
		smChar->DisplayFlag = TRUE;
		lstrcpy( smChar->smCharInfo.szModelName , smDPat->szPatName );

		smChar->smCharInfo.szModelName2[0] = 0;

		if ( smDPat->lpModelInfo->szSubModelFile[0] ) {
			//보조 모델 가져오기 ( 메인모델에 보조 모델 파일이 존재할경우 )
			smDPat2 = smPattern.LoadCharactor( smDPat->lpModelInfo->szSubModelFile );
		}
		else {
			//보조 모델 로딩이 설정되 있는 경우 
			if ( szName2 && szName2[0] ) { 
				smDPat2 = smPattern.LoadCharactor( szName2 );
				lstrcpy( smChar->smCharInfo.szModelName2 , smDPat2->szPatName );
			}
		}

		//if ( smDPat2 ) {
			smChar->SetDinaPattern2( smDPat2 );
		//}

		return TRUE;
	}

	return FALSE;
}

//캐릭터에 패턴을 설정
int SetLoadPattern( smCHAR *smChar , char *szName , POINT3D *Posi , POINT3D *Angle )
{

	return SetLoadPattern( smChar , szName , 0, Posi , Angle );
}




////////////////////////// smCHAR 함수 /////////////////////////////
smCHAR::smCHAR()
{
	Pattern = 0;
	Pattern2 = 0;

	Init();
}

smCHAR::~smCHAR()
{

}

void smCHAR::Init()
{

	//######################################################################################
	//작 성 자 : 오 영 석
	TalkPattern = NULL;	
	TalkFrame = TalkSoundSum= 0;
	lpTalkModelParent = NULL;
	lpTalkMotionInfo = NULL;

	::ZeroMemory( &FrameInfo, sizeof(SMotionStEndInfo) );
	//######################################################################################

	frame = 0;
	action = 0;
	Flag = 0;
	pX=pY=pZ= 0;
	Angle.x = Angle.y = Angle.z = 0;
	MoveFlag = FALSE;
	FrameCnt = 0;
	Counter = 0;

	frame = 0;

	ZeroMemory( PlayBuff , sizeof( smPLAYBUFF ) * PLAYBUFF_SIZE );
	PlayBuffCnt = 0;
	PlayBuffPosi_End = 0;

	TransMinCnt = 8;
	TransLevel  = 0;
	TransDelayMax = PLAYCOUNT_PER_SECOND * 5;

	lpDinaPattern = 0;
	lpDinaPattern2 = 0;
	PatLoading = 0;

	lpDinaLeftPattern = 0;
	lpDinaRightPattern = 0;

	HvLeftHand.PatTool = 0;
	HvRightHand.PatTool = 0;
	HvLeftHand.dwItemCode = 0;
	HvRightHand.dwItemCode = 0;

	HvLeftHand.ColorBlink = 0;
	HvRightHand.ColorBlink = 0;

//	int		ColorBlink;
//	short	sColors[4];
	/*
	HvRightHand.sColors[SMC_R] = 0;
	HvRightHand.sColors[SMC_G] = 0;
	HvRightHand.sColors[SMC_B] = 190;
	HvRightHand.sColors[SMC_A] = 96;

	HvLeftHand.sColors[SMC_R] = 0;
	HvLeftHand.sColors[SMC_G] = 0;
	HvLeftHand.sColors[SMC_B] = 0;
	HvLeftHand.sColors[SMC_A] = -180;
	*/


	wStickItems[0] = -1;
	wStickItems[1] = -1;
	wStickItems[2] = -1;
	wStickItems[3] = -1;

	DisplayFlag = 0;
	AnimDispMode = 0;
	PatDispMode = DISP_MODE_PATMAIN;
	MotionSelectFrame = 0;

	smMotionInfo = 0;

	//######################################################################################
	//작 성 자 : 오 영 석
	smMotionInfo2 = 0;
	//######################################################################################


	DisplayTools = TRUE;

	dwTarget	= FALSE;

	if ( smStage ) lpStage = smStage;

	//움직일 다음 목적 좌표
	ptNextTarget.x = 0;
	ptNextTarget.y = 0;
	ptNextTarget.z = 0;

	//목적 캐릭터 ~~ 
	lpTargetPlayInfo =0;		//목표 캐릭터 (서버용)
	lpTargetChar     =0;		//목표 캐릭터 (클라이언트용)

	lpMasterPlayInfo = 0;		//주인 캐릭터 ( 서버용 )

	//체력 임시 설정
	smCharInfo.Life[0] = 100;
	smCharInfo.Life[1] = 100;
	smCharInfo.State = 0;
	//smCharInfo.Brood = 0;

	dwLastTransTime = dwPlayTime;

	FrameCounter = 0;

	PatWidth = 0;
	PatHeight = 0;
	PatSizeLevel = 0;

	FrameStep = 80;
	MoveSpeed = 250;		//이동 움직임 값
	AttackSpeed = fONE;		//공격 속도

	lpExt1 = 0;
	lpExt2 = 0;
	lpExt3 = 0;
	lpExt4 = 0;

	//채팅 문자열 초기화
	dwChatMessageTimer = dwPlayTime;
	//전송 버퍼 초기화
	srTransBuffSize = 0;

	dwActionItemCode = 0;		//기본 액셕 아이템 

	ShootingMode = FALSE;
	ShootingFlag = FALSE;
	chrAttackTarget = 0;

	AttackToolRange = 0;
	AttackAnger = 0;
	AttackIce = 0;
	OnStageField = -1;
	OnStageFieldState = 0;
	OldMotionState = 0;

	OverLapPosi = 0;
	OverLapPosi2 = 0;
	lpCharOverLap = 0;			//겹친 캐릭터의 포인트

	TargetMoveCount = 0;		//목표 이동좌표 카운터

	AttackObjBip = 0;

	NearPlayCount = 256;		//근처에 유저 확인 카운터 ( 서버에서 사용 )
	AttackCritcal = 0;			//크리티컬 힛 설정

	AutoMoveStep = 0;

	LevelLog	= 0;

	dwNextMotionCode = 0;
	PosBeginCount = 0;

	EnableStateBar = 0;
	DisplayAlpha = 0;			//반투명도
	RendAlpha = 0;				//반투명 캐릭
	AttackSkil = 0;				//스킬공격
	AttackEffect = 0;			//공격용 효과 초기화

	dwDispAppSkill = 0;
	dwDispAppSkillMask = 0;

	HoSkillCode = 0;
	PartyFlag = 0;
	dwAttackPlayTime = 0;
	
	PlaySlowCount = 0;
	PlaySlowSpeed = 0;
	PlayStunCount = 0;
	PlayStopCount = 0;
	ServerCode = 0;

	PlayHolyMind[1] =0;			//데미지 약화 기능 ( [0] 감소된 데미지% [1] 유지시간 )
	PlayHolyPower[1] =0;		//데미지 강화 기능 ( [0] 강화된 데미지% [1] 유지시간 )
	PlayPoison[1] = 0;			//독 공격 받음
	DispPoison = 0;				//독 표시
	PlayHolyIncantation[1] = 0;	//몬스터 현혹
	PlayVanish = 0;				//배니쉬 스킬 적용상태
	PlayCurseQuest = 0;			//저주 퀘스트
	PlayVague = 0;
	PlayDistortion = 0;
	PlayInvincible = 0;
//	Bless_HealLimit = 0;

	dwForceOfNatureTime = 0;	//포스오브 네이쳐 유지시간
	dwHallOfValhallaTime = 0;	//홀오브발할라 유지시간


	AttackSkillRange = 0;

	RecvExp = 0;				//이캐릭으로 부터 받은 경험치 

	sAddColorEffect[0] = 0;
	sAddColorEffect[1] = 0;
	sAddColorEffect[2] = 0;
	sAddColorEffect[3] = 0;

	((DWORD *)smCharInfo.wPlayClass)[0] = 0;

	RenderLatter = FALSE;
	dwUpdateCharInfoTime = 0;

	dwTradeMsgCode = 0;
	EventAttackCount = 0;

	dwClanManageBit = 0;

	lpAttackDamageList = 0;
	lpAttackDamageList_BlessCastle = 0;

	dwForceOrbCode = 0;		//포스오브 적용 코드
	dwForceOrbTime = 0;		//포스오브 유지시간

	// 박재원 - 데미지 부스터(생명력)
	dwLifeBoosterCode = 0; // 박재원 - 부스터 아이템(생명력) 코드
	dwLifeBoosterTime = 0; // 박재원 - 부스터 아이템(생명력) 유지시간
	// 박재원 - 데미지 부스터(기력)
	dwManaBoosterCode = 0; // 박재원 - 부스터 아이템(기력) 코드
	dwManaBoosterTime = 0; // 박재원 - 부스터 아이템(기력) 유지시간
	// 박재원 - 데미지 부스터(근력)
	dwStaminaBoosterCode = 0; // 박재원 - 부스터 아이템(근력) 코드
	dwStaminaBoosterTime = 0; // 박재원 - 부스터 아이템(근력) 유지시간

	// 장별 - 스킬 딜레이
	dwSkillDelayCode = 0;
	dwSkillDelayTime = 0;

	dwLinkObjectCode = 0;
	lpLinkPlayInfo = 0;
	lpLinkChar = 0;

	PkMode_CharState = 0;
	dwClanCode = 0;
	UseObject_VirtualLife = 0;


#ifndef _W_SERVER
	//( 클라이언트 에서만 사용 )
	//무기용 이펙트 
	WeaponEffect = 0;
	dwWeaponEffectTime = 0;

	ClanInfoNum = -1;					//클랜 정보 초기화
	Clan_CastleMasterFlag = 0;				//성주 클랜
	dwClanInfoTime = 0;
	TryAttackCount = 0;

	((DWORD *)sBlessCastle_Damage)[0] = 0;	//공성 개인별 데미지 추가치


#else
	dwTargetLockTime = 0;

	lpCompulsionPlayInfo = 0;	//시선끌기 캐릭터
	dwCompulsionTime = 0;		//시선끌기 시간

	dwAssassinEyeTime = 0;		//어세신아이 적용 몬스터
	AssassinEyeParam = 0;		//어세신아이 적용 몬스터

	//몬스터 이동 스텝 구성
	sMoveStepCount[0] = 0;
	sMoveStepCount[1] = 0;

	DontMoveFlag = 0;

	Clan_CastleMasterFlag = 0;				//빌링 몬스터 플랙 ( 서버에서 서로 공격안하도록 하기위한 구분용 )


#endif

}


int smCHAR::Close()
{
	Flag = 0;

	if ( lpDinaPattern2 ) {
		AddRemoveDynPattern( lpDinaPattern2 );
		//smDPAT_Delete( lpDinaPattern2 );
		lpDinaPattern2 = 0;
	}



	if ( lpDinaPattern ) {
		AddRemoveDynPattern( lpDinaPattern );
		//smDPAT_Delete( lpDinaPattern );
		lpDinaPattern = 0;
	}

	if ( lpDinaLeftPattern ) {
		AddRemoveDynPattern( lpDinaLeftPattern );
		//smDPAT_Delete( lpDinaLeftPattern );
		lpDinaLeftPattern = 0;
	}

	if ( lpDinaRightPattern ) {
		AddRemoveDynPattern( lpDinaRightPattern );
		//smDPAT_Delete( lpDinaRightPattern );
		lpDinaRightPattern = 0;
	}


	Pattern = 0;
	Pattern2 = 0;

	HvLeftHand.PatTool = 0;
	HvRightHand.PatTool = 0;

	smMotionInfo = 0;
	smMotionInfo2 = 0;

	dwDispAppSkill = 0;

	if ( lpAttackDamageList ) {
		delete	lpAttackDamageList;
		lpAttackDamageList = 0;
	}

	return TRUE;
}


//패턴설정
int smCHAR::SetPattern( smPAT3D *pat )
{
	smPAT3D *AnimPat;

	Pattern = pat;

	if ( pat->TmParent ) AnimPat = pat->TmParent;
	else AnimPat = pat;

	HvRightHand.ObjBip = AnimPat->GetObjectFromName(szBipName_RightHand) ;

	//왼쪽 도구
	if ( (HvLeftHand.dwItemCode&sinITEM_MASK2)==sinDS1 )
		HvLeftHand.ObjBip = AnimPat->GetObjectFromName(szBipName_Shield) ;		//방패
	else 
		HvLeftHand.ObjBip = AnimPat->GetObjectFromName(szBipName_LeftHand) ;	//기타 활등


	BackObjBip[0] = AnimPat->GetObjectFromName( szBipName_BackSpine );
	BackObjBip[1] = AnimPat->GetObjectFromName( szBipName_BackSpineBow );
	BackObjBip[2] = AnimPat->GetObjectFromName( szBipName_BackSpineCross );
	ShieldObjBip = AnimPat->GetObjectFromName( szBipName_Shield2 );

//	if ( !BackObjBip[1] ) BackObjBip[1]=BackObjBip[0];
//	if ( !BackObjBip[2] ) BackObjBip[2]=BackObjBip[0];

	AttackObjBip = 0;

	//여자 기본 공격 발차기 ( 아쳐 , 아탈란타 )
	if ( smCharInfo.JOB_CODE==3 || smCharInfo.JOB_CODE==5 )
		AttackObjBip = AnimPat->GetObjectFromName( szBipName_LFoot );

	if ( smCharInfo.JOB_CODE==7 ) //메지션 박치기
		AttackObjBip = AnimPat->GetObjectFromName( szBipName_Head );

	AnimPattern = AnimPat;

	PatHeight = pat->SizeHeight;
	PatWidth = (pat->SizeWidth*160)>>FLOATNS;

//	PatSizeLevel

	return TRUE;
}

//패턴머리 설정
int smCHAR::SetPattern2( smPAT3D *pat )
{
//	smPAT3D *AnimPat;

	Pattern2 = pat;

	return TRUE;
}

//마을에서 들고 다니는 아이템 무기
DWORD	dwItemCodeFromVillage[] ={
	sinWC1|sin01 , sinWC1|sin02 , sinWC1|sin03 , sinWC1|sin04 , sinWC1|sin05 ,
	sinWC1|sin06 , sinWC1|sin07 , sinWC1|sin08 , sinWC1|sin09 , sinWC1|sin10 ,
	sinWC1|sin11 , sinWC1|sin12 , sinWC1|sin13 , sinWC1|sin14 , sinWC1|sin15 ,
	sinWC1|sin16 , sinWC1|sin17 , sinWC1|sin18 , sinWC1|sin19 , sinWC1|sin20 ,
	sinWC1|sin21 , sinWC1|sin22 , sinWC1|sin23 ,

	sinWM1|sin01 , sinWM1|sin02 , sinWM1|sin03 , sinWM1|sin06 , 
	sinWS2|sin01 , sinWS2|sin02 , sinWS2|sin03 , 

	sinWS1|sin02, sinWS1|sin03,  
	sinWS1|sin09,   
	0
};

//등에 붙이는 활 계열 무기
DWORD	dwItemCodeFromVillage_Bow[] ={
	sinWS1|sin04,sinWS1|sin08,sinWS1|sin10,sinWS1|sin13,sinWS1|sin17,
	0
};

//등에 붙이는 석궁 계열 무기
DWORD	dwItemCodeFromVillage_Cross[] ={
	sinWS1|sin01,sinWS1|sin05,sinWS1|sin06, sinWS1|sin07,
	sinWS1|sin11, sinWS1|sin12,sinWS1|sin14, sinWS1|sin15,sinWS1|sin16,

	0
};



int smCHAR::SetTool( DWORD dwItemCode , int hvPosi  )
{
	char szToolMeshName[64];
	int cnt,szX,szN;

	szX = szN = 0;

	if ( smCharInfo.State!=smCHAR_STATE_USER && PkMode_CharState!=smCHAR_STATE_USER ) return FALSE;

	if ( dwItemCode )
		sinGetItemInfo( dwItemCode , szToolMeshName );

	switch( hvPosi ) {
	case hvPOSI_RHAND:
		if ( HvRightHand.ObjBip ) {
			if ( lpDinaRightPattern ) {
				AddRemoveDynPattern(lpDinaRightPattern);
				//smDPAT_Delete( lpDinaRightPattern );
				lpDinaRightPattern = 0;
			}
			if ( dwItemCode ) {
				lpDinaRightPattern = smPattern.LoadCharactor( szToolMeshName );
				if ( lpDinaRightPattern ) {
					HvRightHand.PatTool = lpDinaRightPattern->Pat;
					for( cnt=0;cnt<lpDinaRightPattern->Pat->nObj3d;cnt++) {
						if ( szX<lpDinaRightPattern->Pat->obj3d[cnt]->maxY ) 
							szX=lpDinaRightPattern->Pat->obj3d[cnt]->maxY;
						if ( szN<lpDinaRightPattern->Pat->obj3d[cnt]->minY ) 
							szN=lpDinaRightPattern->Pat->obj3d[cnt]->minY;
					}
					HvRightHand.SizeMax = szX;
					HvRightHand.SizeMin = szN;
					HvRightHand.dwItemCode = dwItemCode;
					//오른쪽 도구는 손에 붙임
					HvRightHand.ObjBip = AnimPattern->GetObjectFromName(szBipName_RightHand) ;
					dwActionItemCode = dwItemCode;

					//wStickItems[smPLAYBUFF_ITEM_ACTION] = GetSinItemNumFromCode( dwActionItemCode );
					wStickItems[smPLAYBUFF_ITEM_RIGHT] = GetSinItemNumFromCode( dwItemCode );
				}
				else {
					HvRightHand.PatTool = 0;
					HvRightHand.dwItemCode =0;
					wStickItems[smPLAYBUFF_ITEM_RIGHT] = -1;
				}
			}
			else {
				HvRightHand.PatTool = 0;
				HvRightHand.dwItemCode =0;
				wStickItems[smPLAYBUFF_ITEM_RIGHT] = -1;
			}
		}
		else 
			return FALSE;
		break;

	case hvPOSI_LHAND:
		if ( HvLeftHand.ObjBip ) {
			if ( lpDinaLeftPattern ) {
				AddRemoveDynPattern(lpDinaLeftPattern);
				//smDPAT_Delete( lpDinaLeftPattern );
				lpDinaLeftPattern = 0;
			}
			if ( dwItemCode ) {
//				lpDinaLeftPattern = smPattern.LoadCharactor( "Shield\\Ds101.ASE" );
				lpDinaLeftPattern = smPattern.LoadCharactor( szToolMeshName );
				if ( lpDinaLeftPattern ) {
					HvLeftHand.PatTool = lpDinaLeftPattern->Pat;
					for( cnt=0;cnt<lpDinaLeftPattern->Pat->nObj3d;cnt++) {
						if ( szX<lpDinaLeftPattern->Pat->obj3d[cnt]->maxY ) 
							szX=lpDinaLeftPattern->Pat->obj3d[cnt]->maxY;
						if ( szN<lpDinaLeftPattern->Pat->obj3d[cnt]->minY ) 
							szN=lpDinaLeftPattern->Pat->obj3d[cnt]->minY;
					}
					HvLeftHand.SizeMax = szX;
					HvLeftHand.SizeMin = szN;
					HvLeftHand.dwItemCode = dwItemCode;
					wStickItems[smPLAYBUFF_ITEM_LEFT] = GetSinItemNumFromCode( dwItemCode );

					//왼쪽 도구
					if ( (dwItemCode&sinITEM_MASK2)==sinDS1 || (dwItemCode&sinITEM_MASK2)==sinOM1 ) {
						if ( (dwItemCode&sinITEM_MASK2)==sinDS1 ) {
							//방패
							HvLeftHand.ObjBip = AnimPattern->GetObjectFromName(szBipName_Shield) ;		//방패
						}
						else {
							//메지컬 스터프
							HvLeftHand.PatTool = 0;
							HvLeftHand.dwItemCode = 0;
							wStickItems[smPLAYBUFF_ITEM_LEFT] = -1;
						}
					}
					else {
						HvLeftHand.ObjBip = AnimPattern->GetObjectFromName(szBipName_LeftHand) ;	//기타 활등
						dwActionItemCode = dwItemCode;
						//wStickItems[smPLAYBUFF_ITEM_ACTION] = GetSinItemNumFromCode( dwActionItemCode );
					}
				}
				else {
					HvLeftHand.PatTool = 0;
					HvLeftHand.dwItemCode = 0;
					wStickItems[smPLAYBUFF_ITEM_LEFT] = -1;
				}
			}
			else {
				HvLeftHand.PatTool = 0;
				HvLeftHand.dwItemCode = 0;
				wStickItems[smPLAYBUFF_ITEM_LEFT] = -1;
			}

		}
		else 
			return FALSE;
		break;
	}

	if ( dwActionItemCode ) {
		if ( dwActionItemCode!=HvLeftHand.dwItemCode && dwActionItemCode!=HvRightHand.dwItemCode ) {
			dwActionItemCode = 0;
			//wStickItems[smPLAYBUFF_ITEM_ACTION] = -1;
		}

		//공격 사정거리 설정
		if ( dwActionItemCode==HvLeftHand.dwItemCode ) {
			AttackToolRange = (HvLeftHand.SizeMax * 200)>>8;
		}
		if ( dwActionItemCode==HvRightHand.dwItemCode ) {
			AttackToolRange = (HvRightHand.SizeMax * 200)>>8;
		}

		cnt = 0;
		dwItemSetting = 1;
		
		while(1) {
			if ( dwItemCodeFromVillage[cnt]==0 ) break;
			if ( dwItemCodeFromVillage[cnt]==dwActionItemCode ) {
				dwItemSetting = 0;
				break;
			}
			cnt++;
		}

		cnt = 0;
		while(1) {
			if ( dwItemCodeFromVillage_Bow[cnt]==0 ) break;
			if ( dwItemCodeFromVillage_Bow[cnt]==dwActionItemCode ) {
				dwItemSetting = 2;
				break;
			}
			cnt++;
		}

		cnt = 0;
		while(1) {
			if ( dwItemCodeFromVillage_Cross[cnt]==0 ) break;
			if ( dwItemCodeFromVillage_Cross[cnt]==dwActionItemCode ) {
				dwItemSetting = 3;
				break;
			}
			cnt++;
		}
		
	}
	else
		AttackToolRange = 0;


	if ( (dwActionItemCode&sinITEM_MASK2)==sinWS1 || (dwActionItemCode&sinITEM_MASK2)==sinWT1 ||
		( (dwActionItemCode&sinITEM_MASK2)==sinWM1 && ( smCharInfo.JOB_CODE==7 || smCharInfo.JOB_CODE==8 ) ) ) {
		//활 , 마법 지팡이
		ShootingMode = TRUE;
	}
	else
		ShootingMode = FALSE;

	if ( (smCharInfo.State==smCHAR_STATE_USER || PkMode_CharState==smCHAR_STATE_USER ) && dwItemCode==dwActionItemCode ) {
		if ( MotionInfo->State!=CHRMOTION_STATE_DEAD && MotionInfo->State!=CHRMOTION_STATE_DAMAGE &&
			MotionInfo->State!=CHRMOTION_STATE_RESTART ) {
			SetMotionFromCode( MotionInfo->State );
			AttackCritcal = -1;
		}

		if ( dwItemCode ) {
			//양손 무기 첵크
			for( cnt=0;cnt<MAX_ITEM;cnt++) {
				if ( sItem[cnt].CODE==dwItemCode ) {
					if ( sItem[cnt].Class==ITEM_CLASS_WEAPON_TWO && !ShootingMode )
						dwActionItemTwoHand = TRUE;
					else
						dwActionItemTwoHand = FALSE;

					break;
				}
			}
		}
	}

/*
	if ( lpD3DDevice ) {
		ReadTextures();
		if ( HvRightHand.PatTool ) HvRightHand.PatTool->smMaterialGroup->CheckMatreialTextureSwap();
		if ( HvLeftHand.PatTool ) HvLeftHand.PatTool->smMaterialGroup->CheckMatreialTextureSwap();
	}
*/
	DisplayTools = TRUE;
	return TRUE;
}



int smCHAR::SetPosi( int x, int y, int z , int angX, int angY, int angZ )
{
	int sy;

	pX = x;
	pZ = z;
	pY = y;

	if ( y==0 && lpStage ) 
		pY = lpStage->GetHeight( pX , pZ );

	if ( pY==0 ) {

		if ( smGameStage[0] )
			pY = smGameStage[0]->GetHeight( pX , pZ );

		if ( smGameStage[1] ) {
			sy = smGameStage[1]->GetHeight( pX , pZ );
			if ( pY<sy ) pY = sy;
		}
	}


	pY -= 2*fONE;

	PHeight = pY;

	Angle.x = angX;
	Angle.y = angY;
	Angle.z = angZ;

	return TRUE;
}

int smCHAR::SetTargetPosi( int x, int z )
{

	tx = x<<FLOATNS;
	tz = z<<FLOATNS;
	MoveFlag = TRUE;

	Angle.y = GetRadian2D( pX ,pZ, tx, tz );

	if ( ActionPattern==0) 	{
		if ( MoveMode )
			SetMotionFromCode( CHRMOTION_STATE_RUN );
		else
			SetMotionFromCode( CHRMOTION_STATE_WALK );						
	}

	return Angle.y;
}

int smCHAR::SetTargetPosi2( int x, int z )
{

	tx = x<<FLOATNS;
	tz = z<<FLOATNS;

	Angle.y = GetRadian2D( pX ,pZ, tx, tz );

	return Angle.y;
}


//다음 목적 좌표 설정
int smCHAR::SetNextTarget( int x, int y, int z )
{
	ptNextTarget.x = x;
	ptNextTarget.y = y;
	ptNextTarget.z = z;

	return TRUE;
}

int smCHAR::SetAction( int nAction )
{
//			if ( CheckLocateBox( pX, pZ , tx, tz , 16*fONE )==TRUE )

	if ( nAction==0 ) MoveFlag = FALSE;

	return TRUE;

}

//먼지 생성
int	smCHAR::SetSmoking( int level )
{

	GetMoveLocation( 4*fONE , 0, 0 , 0, Angle.y , 0 );
	StartEffect( pX+GeoResult_X,pY+8*fONE,pZ+GeoResult_Z ,20,20, EFFECT_DUST1 );

	GetMoveLocation( -4*fONE , 0, 0 , 0, Angle.y , 0 );
	StartEffect( pX+GeoResult_X,pY+8*fONE,pZ+GeoResult_Z ,20,20, EFFECT_DUST1 );

	return TRUE;


}

//이동 ( 정밀 )
int smCHAR::MoveAngle2( int step )
{
	int x,z,y;
	int he,hy;
	int OldStageField;
	POINT3D MovePosi;
	int MovLevel[2];

	WaterHeight = CLIP_OUT;

#ifndef _W_SERVER

	if ( smConfig.DebugMode && VRKeyBuff['N'] ) {

		x = pX + (( (GetSin[Angle.y&ANGCLIP]>>FLOATNS) * step )>>FLOATNS);
		z = pZ + (( (GetCos[Angle.y&ANGCLIP]>>FLOATNS) * step )>>FLOATNS);
		y = pY;

		//y = smStage->GetHeight( x , z );
		//if ( abs( pY-y )>(16*fONE) ) return FALSE;

		pY=y;
		pX=x; 
		pZ=z;

		return TRUE;
	}
#else
	if ( DontMoveFlag ) return FALSE;
#endif

	POINT3D Posi;

	he = 0;

	Posi.x = x = pX;
	Posi.y = y = pY;
	Posi.z = z = pZ;

	OldStageField = OnStageField;
	OnStageField = -1;

	if ( step==0 || PHeight<y  ) {
		if ( lpStage ) {
			he = lpStage->GetFloorHeight( x,y,z , Pattern->SizeHeight );
		}
		else {
			if ( smGameStage[0] ) {
				he = smGameStage[0]->GetFloorHeight( x,y,z , Pattern->SizeHeight );	
				OnStageField = 0;
			}

			if ( smGameStage[1] ) {
				hy = smGameStage[1]->GetFloorHeight( x,y,z , Pattern->SizeHeight );	
				if ( he<hy ) {
					he = hy;
					OnStageField = 1;
				}
			}
		}
		if ( he!=CLIP_OUT )
			y = he;
		else
			y = -80*fONE;

		if ( (pY-y)>8*fONE ) {
			if ( MotionInfo->State!=CHRMOTION_STATE_FALLDOWN && (pY-y)>32*fONE ) {
				FallHeight = pY-y;
				SetMotionFromCode( CHRMOTION_STATE_FALLDOWN );
			}
			PHeight = y;
			pY -= 8*fONE;
		}
		else {
			pY = y;
			PHeight = y;
			if ( MotionInfo->State==CHRMOTION_STATE_FALLDOWN ) {
				if ( FallHeight>200*fONE )
					SetMotionFromCode( CHRMOTION_STATE_FALLDAMAGE );
				else
					SetMotionFromCode( CHRMOTION_STATE_FALLSTAND );

				SetSmoking(0);

			}
		}
		return TRUE;
	}
	else {
		if ( lpStage && lpStage->CheckNextMove( &Posi, &Angle , &MovePosi, step , Pattern->SizeWidth , Pattern->SizeHeight , OverLapPosi2 )>0 ) {
			x = MovePosi.x;
			y = MovePosi.y;
			z = MovePosi.z;
		}
		else {
			MovLevel[0] = 0;
			MovLevel[1] = 0;

			if ( smGameStage[0] && (MovLevel[0]=smGameStage[0]->CheckNextMove( &Posi, &Angle , &MovePosi , step , Pattern->SizeWidth , Pattern->SizeHeight, OverLapPosi2 ))>0 ) {

				if ( smStage_WaterChk!=CLIP_OUT ) 
					WaterHeight = smStage_WaterChk;


				if ( StageField[0]->LimitLevel<=smCharInfo.Level ) {

					OnStageField = 0;

					he = MovePosi.y;
					x = MovePosi.x;
					y = MovePosi.y;
					z = MovePosi.z;
				}

			}
			if ( smGameStage[1] && (MovLevel[1]=smGameStage[1]->CheckNextMove( &Posi, &Angle , &MovePosi, step , Pattern->SizeWidth , Pattern->SizeHeight,OverLapPosi2 )>0) ) {

				if ( smStage_WaterChk!=CLIP_OUT ) 
					WaterHeight = smStage_WaterChk;

				if ( StageField[1]->LimitLevel<=smCharInfo.Level ) {
					if ( MovLevel[0] && abs(he-MovePosi.y)<fONE*8 ) {
						if ( MovLevel[0]>MovLevel[1] ) {
							x = MovePosi.x;
							y = MovePosi.y;
							z = MovePosi.z;
							OnStageField = 1;
						}
					}
					else {
						if ( he<MovePosi.y ) {//Posi.y ) {
							x = MovePosi.x;
							y = MovePosi.y;
							z = MovePosi.z;
							OnStageField = 1;
						}
					}
				}
			}
		}
	}

	pX = x;
	pZ = z;

	if ( (pY-y)>8*fONE ) {

		if ( MotionInfo->State!=CHRMOTION_STATE_FALLDOWN  && (pY-y)>32*fONE ) {
			FallHeight = pY-y;
			SetMotionFromCode( CHRMOTION_STATE_FALLDOWN );
		}

		PHeight = y;
		pY -= 8*fONE;
	}
	else {
		pY = y;
		PHeight = y;
		if ( MotionInfo->State==CHRMOTION_STATE_FALLDOWN ) {
			if ( FallHeight>200*fONE ) 
				SetMotionFromCode( CHRMOTION_STATE_FALLDAMAGE );
			else
				SetMotionFromCode( CHRMOTION_STATE_FALLSTAND );

			SetSmoking(0);

		}
	}

	if ( OnStageField>=0 && OldStageField!=OnStageField ) {
		OnStageFieldState = StageField[ OnStageField ]->State;
		if ( OldStageField<0 || OnStageFieldState!=StageField[ OldStageField ]->State ) {
			SetMotionFromCode( MotionInfo->State );
		}
	}
	if ( OnStageField<0 )
		OnStageField = OldStageField;

	return TRUE;
}


//이동
int smCHAR::MoveAngle( int step )
{
	return	MoveAngle2( step*fONE );
}

int smCHAR::SetDinaPattern( smDPAT	*lpDPat )
{

	//######################################################################################
	//작 성 자 : 오 영 석
	TalkPattern = NULL;	
	TalkFrame = TalkSoundSum = 0;
	lpTalkModelParent = NULL;
	lpTalkMotionInfo = NULL;

	if( smConfig.DebugMode )
	{
		Debug_TalkZoomMode = 0;
		Debug_TalkRepeatMode = 0;
		Debug_TalkRepeatModeType = -1;
	}
	//######################################################################################

	lpDinaPattern = lpDPat;
	smMotionInfo = lpDinaPattern->lpModelInfo;
	SetPattern( lpDPat->Pat );

	SetMotionFromCode( CHRMOTION_STATE_STAND );	

	return TRUE;
}

int smCHAR::SetDinaPattern2( smDPAT	*lpDPat )
{
	lpDinaPattern2 = lpDPat;

	if ( lpDinaPattern2 ) {
		smMotionInfo2 = lpDinaPattern2->lpModelInfo;
		SetPattern2( lpDPat->Pat );

		if ( Pattern->TmParent==Pattern2->TmParent ) {
			AnimDispMode = 0;				//같이 출력
			PatDispMode =DISP_MODE_PATMAIN|DISP_MODE_PATSUB;
		}
		else {
			AnimDispMode = 1;				//하나씩 출력
			PatDispMode =DISP_MODE_PATMAIN;
		}
	}
	else {
		AnimDispMode = 0;				//같이 출력
		PatDispMode =DISP_MODE_PATMAIN;
		SetPattern2( 0 );
	}

	//######################################################################################
	//작 성 자 : 오 영 석
	if( lpDinaPattern->lpTalkLink )
	{
        PatDispMode |= DISP_MODE_PATTALK;
		TalkPattern = lpDinaPattern->lpTalkLink->Pat;
	}
	else if( lpDinaPattern2 && lpDinaPattern2->lpTalkLink )
	{
		PatDispMode |= DISP_MODE_PATTALK;
		TalkPattern = lpDinaPattern2->lpTalkLink->Pat;
	}

	if( TalkPattern && (PatDispMode & DISP_MODE_PATTALK) )
	{
		if( smMotionInfo->szTalkLinkFile[0] )
			lpTalkModelParent = smMotionInfo;
		else if( smMotionInfo2 && smMotionInfo2->szTalkLinkFile[0] )
			lpTalkModelParent = smMotionInfo2;
		else
		{
			// 에러 처리. 작동되지 않게끔.
			PatDispMode &= (~DISP_MODE_PATTALK);
			TalkPattern = NULL;
		}
	}
	//######################################################################################

	return TRUE;
}

//움직임 동작 설정
int smCHAR::ChangeMotion( int Motion , int DpMode )
{

	if ( DpMode ) {
		//다중 모델일 경우 움직임 선택
		if ( MotionSelectFrame ) {
			MotionInfo = &smMotionInfo2->MotionInfo[ Motion ];
			PatDispMode = DISP_MODE_PATSUB;
		}
		else {
			MotionInfo = &smMotionInfo->MotionInfo[ Motion ];
			PatDispMode = DISP_MODE_PATMAIN;
		}
	}
	else {
		//기본 모델 움직임으로 설정
		if ( AnimDispMode )
			PatDispMode = DISP_MODE_PATMAIN;

		MotionInfo = &smMotionInfo->MotionInfo[ Motion ];
		MotionSelectFrame = 0;
	}

	frame = MotionInfo->StartFrame*160;
	action	=	Motion;
	FrameCounter = 0;

	AttackSkil = 0;
	HideWeapon = 0;
	MotionLoop = 0;

	dwEventFrameExt[0] = 0;
	dwEventFrameExt[1] = 0;
	dwEventFrameExt[2] = 0;
	dwEventFrameExt[3] = 0;

	//######################################################################################
	//작 성 자 : 오 영 석
	FrameInfo.StartFrame = frame;
	FrameInfo.EndFrame = MotionInfo->EndFrame*160;
	//######################################################################################

	return TRUE;
}


//######################################################################################
//작 성 자 : 오 영 석
int smCHAR::AutoChangeTalkMotion( int TalkModeType )
{
	if( TalkPattern == NULL )
		return 0;

	int TalkCnt = 0;
	if( lpTalkModelParent->TalkMotionRate[ CHRMOTION_EXT ] )
	{
		TalkCnt = rand() % 100;
		TalkCnt = lpTalkModelParent->TalkMotionRateCnt[ TalkModeType ][ TalkCnt ];
	}
	else
		TalkCnt = rand() % lpTalkModelParent->TalkMotionCount;

	lpTalkMotionInfo = &lpTalkModelParent->TalkMotionInfo[ TalkCnt ];
	TalkFrame = lpTalkMotionInfo->StartFrame * 160;

	return 1;
}
//######################################################################################


//해당 모션의 갯수를 구한다
int smCHAR::FindMotionCountFromCode( DWORD MotionCode )
{
	int cnt;
	int FindCnt =0;

	if ( !smMotionInfo ) return 0;		//모델을 불러오지 못한 경우

	for(cnt=0;cnt<(int)smMotionInfo->MotionCount;cnt++) {
		if ( smMotionInfo->MotionInfo[ cnt ].State == MotionCode ) {
			FindCnt++;
		}
	}

	return FindCnt;
}

//모션 분류 코드로 모션을 바꾼다
int smCHAR::SetMotionFromCode( DWORD MotionCode )
{
	int cnt,cnt2,itcnt;
	int FoundNum[64];
	int FindCnt;

	int FoundNum2[64];
	int FindCnt2;
	int StageVillage;
	int	dwActItemCode;
	int	atSkill = 0;
	int	skillCode;
	BYTE	bSkill;

	DWORD	dwJobCodeMask;

	DWORD dwFindCode;

	smMODELINFO	*lpMotionInfo;

	if ( smCharInfo.JOB_CODE && smCharInfo.JOB_CODE<16 )
		dwJobCodeMask =	BitMaskJobNames[ smCharInfo.JOB_CODE-1 ].dwBitMask;
	else
		dwJobCodeMask =	0;


	FindCnt = 0;
	FindCnt2 = 0;
	MotionSelectFrame = 0;

	dwActItemCode = dwActionItemCode;

	if ( OnStageField>=0 ) {
		if ( StageField[OnStageField]->State==FIELD_STATE_VILLAGE ) {
			StageVillage = 1;
			dwActItemCode = 0;
		}
		else
			StageVillage = 2;
	}
	else {
#ifdef _W_SERVER
		StageVillage = 3;
#else
		if ( GameMode==1 ) StageVillage=1;
		else StageVillage=3; 
#endif
	}


	lpMotionInfo = smMotionInfo;

	if ( MotionCode==CHRMOTION_STATE_SKILL && AttackSkil ) {
		atSkill = AttackSkil;
	}

	for(cnt=0;cnt<(int)smMotionInfo->MotionCount;cnt++) {
		//각종 조건에 맞는 모션을 찾는다 ( 동작/맵위치/직업 등을 고려 )
		if ( smMotionInfo->MotionInfo[ cnt ].State == MotionCode && 
			(!smMotionInfo->MotionInfo[ cnt ].MapPosition || (smMotionInfo->MotionInfo[ cnt ].MapPosition&StageVillage)) &&
			(!smMotionInfo->MotionInfo[ cnt ].dwJobCodeBit || !dwJobCodeMask ||
			( smMotionInfo->MotionInfo[ cnt ].dwJobCodeBit&dwJobCodeMask )) ) {

			if ( atSkill ) {
				//스킬 모션일 경우
				cnt2 = 0;
				skillCode = atSkill&0xFF;
				while(1) {
					//스킬 코드 검색
					bSkill = smMotionInfo->MotionInfo[ cnt ].SkillCodeList[cnt2];
					if ( bSkill==0 ) break;
					if ( SkillDataCode[bSkill].dwPlaySkillCode==skillCode ) {
						break;
					}
					cnt2++;
				}
				if ( bSkill ) {
					//스킬 검색 성공
					FoundNum[FindCnt++] = cnt;
					if ( FindCnt>=32 ) break;
				}
			}
			else {
				FoundNum[FindCnt++] = cnt;
				if ( FindCnt>=32 ) break;
			}
		}
	}


	if ( FindCnt==0 && AnimDispMode && lpDinaPattern2 ) {
		//2차 모델에서 정보를 찾음
		for(cnt=0;cnt<(int)smMotionInfo2->MotionCount;cnt++) {
			if ( smMotionInfo2->MotionInfo[ cnt ].State == MotionCode && 
				(!smMotionInfo2->MotionInfo[ cnt ].MapPosition || (smMotionInfo2->MotionInfo[ cnt ].MapPosition&StageVillage)) ) {
				FoundNum[FindCnt++] = cnt;
				if ( FindCnt>=32 ) break;
			}
		}

		if ( FindCnt ) {
			MotionSelectFrame = 1;
			lpMotionInfo = smMotionInfo2;
		}
	}

	for( cnt=0;cnt<FindCnt;cnt++) {
		itcnt = lpMotionInfo->MotionInfo[ FoundNum[cnt] ].ItemCodeCount;
		if ( itcnt==0 ) {
			FoundNum2[ FindCnt2++ ] = FoundNum[cnt];
		}
		else {
			if ( dwActItemCode ) {
				for ( cnt2=0;cnt2<itcnt;cnt2++) {
					dwFindCode = lpMotionInfo->MotionInfo[ FoundNum[cnt] ].ItemCodeList[cnt2];
					if ( dwActItemCode==(int)sItem[ dwFindCode ].CODE ) {
						FoundNum2[ FindCnt2++ ] = FoundNum[cnt];
						break;
					}
				}
			}
			else {
				for ( cnt2=0;cnt2<itcnt;cnt2++) {
					dwFindCode = lpMotionInfo->MotionInfo[ FoundNum[cnt] ].ItemCodeList[cnt2];
					if ( dwFindCode==0xFF ) {
						FoundNum2[ FindCnt2++ ] = FoundNum[cnt];
						break;
					}
				}
			}
		}
	}

/*
#define MOTION_LIST_MAX			32
#define MOTION_INFO_MAX			512
#define	MOTION_TOOL_MAX			20
#define	MOTION_JOB_MAX			12


struct smMOTIONINFO {
	DWORD	State;							//상태 TRUE면 유효
	DWORD	StartFrame;						//시작 프레임
	DWORD	EndFrame;						//종료 프레임
	DWORD	EventFrame[4];					//이벤트 생성 프레임

	int		ItemCodeCount;					//해당 아이템 리스트 카운터 ( 0 없음  -1 전체 해당 )
	DWORD	ItemCodeList[MOTION_TOOL_MAX];	//해당 아이템 코드 리스트 
	BYTE	JobCodeList[MOTION_JOB_MAX];	//해당 직업 코드 리스트

	int		MapPosition;					//해당 맵 적용 ( 0-관계없음 마을 1 - 필드 2 )

	DWORD	Repeat;							//반복 여부
	CHAR	KeyCode;						//동작 시작 키
	int		MotionFrame;					//모션 연결 파일 번호
};
*/

	if ( FindCnt2==0 ) {
		if ( FindCnt==1 ) 
			ChangeMotion( FoundNum[0] , AnimDispMode );
		else
			if ( FindCnt ) ChangeMotion( FoundNum[(rand()%FindCnt)] , AnimDispMode );

		if ( atSkill ) AttackSkil = atSkill;

		return NULL;
	}

	//FoundNum2[0] = 115;FindCnt2=1;

	if ( FindCnt2==1 ) ChangeMotion( FoundNum2[0] , AnimDispMode );
	else
		if ( FindCnt2 ) ChangeMotion( FoundNum2[(rand()%FindCnt2)] , AnimDispMode );


	if ( atSkill ) AttackSkil = atSkill;

	return FindCnt;
}


//프레임 번호로 동작을 찾는다
int smCHAR::FindActionFromFrame( int sframe )
{
	DWORD cnt;
	DWORD compFrame;

	compFrame = sframe / 160;

	for( cnt=0;cnt<smMotionInfo->MotionCount;cnt++) {
		if ( smMotionInfo->MotionInfo[ cnt ].StartFrame<compFrame &&
			smMotionInfo->MotionInfo[ cnt ].EndFrame>compFrame ) 
			return cnt;
	}

	return -1;
}

int smCHAR::SavePlayBuff()
{

	PlayBuffCnt = (PlayBuffCnt+1)&PLAYBUFF_MASK;

	PlayBuff[ PlayBuffCnt ].angle[0] = Angle.x;
	PlayBuff[ PlayBuffCnt ].angle[1] = Angle.y;
	PlayBuff[ PlayBuffCnt ].angle[2] = Angle.z;
	PlayBuff[ PlayBuffCnt ].angle[3] = action;

	PlayBuff[ PlayBuffCnt ].x = pX;
	PlayBuff[ PlayBuffCnt ].y = pY;
	PlayBuff[ PlayBuffCnt ].z = pZ;

	PlayBuff[ PlayBuffCnt ].frame = (MotionSelectFrame<<CHAR_FRAME_SELSHIFT)|frame;

	if ( chrAttackTarget )
		PlayBuff[ PlayBuffCnt ].dwTargetSerial = chrAttackTarget->dwObjectSerial;
	else
		PlayBuff[ PlayBuffCnt ].dwTargetSerial = NULL;

	memcpy( PlayBuff[ PlayBuffCnt ].wStickItems , wStickItems , sizeof( short ) *4 );

	return PlayBuffCnt;
}

//몬스터 및 자동캐릭터용 프레임 저장 (서버용)
int smCHAR::SavePlayBuff2()
{

	PlayBuffCnt = (PlayBuffCnt+1)&PLAYBUFF_MASK;

	PlayBuff[ PlayBuffCnt ].angle[0] = Angle.x;
	PlayBuff[ PlayBuffCnt ].angle[1] = Angle.y;
	PlayBuff[ PlayBuffCnt ].angle[2] = Angle.z;
	PlayBuff[ PlayBuffCnt ].angle[3] = action;

	PlayBuff[ PlayBuffCnt ].x = pX;
	PlayBuff[ PlayBuffCnt ].y = pY;
	PlayBuff[ PlayBuffCnt ].z = pZ;

	PlayBuff[ PlayBuffCnt ].frame = (MotionSelectFrame<<CHAR_FRAME_SELSHIFT)|frame;

	PlayBuff[ PlayBuffCnt ].dwTargetSerial = NULL;

	if ( lpTargetChar )
		PlayBuff[ PlayBuffCnt ].dwTargetSerial = lpTargetChar->dwObjectSerial;

	if ( lpTargetPlayInfo )
		PlayBuff[ PlayBuffCnt ].dwTargetSerial = lpTargetPlayInfo->dwObjectSerial;

	memcpy( PlayBuff[ PlayBuffCnt ].wStickItems , wStickItems , sizeof( short ) *4 );

	return PlayBuffCnt;
}



//이동 모드 변경
int smCHAR::ChangeMoveMode()
{
//CHRMOTION_STATE_RUN
/*
							if ( !SetMotionFromCode( CHRMOTION_STATE_SOMETIME ) ) {
								Angle.y = (cnt<<4)&ANGCLIP;
								ChangeMotion( CHRMOTION_WALK );						
							}						
						}
						else {
							if ( !SetMotionFromCode( TRUE ) ) {
								Angle.y = (cnt<<4)&ANGCLIP;
								ChangeMotion( CHRMOTION_WALK );						
*/
	if ( MoveMode==0 ) MoveMode = 1;
	else MoveMode = 0;

	if ( MoveFlag ) {
		if ( MoveMode )
			SetMotionFromCode( CHRMOTION_STATE_RUN );
		else
			SetMotionFromCode( CHRMOTION_STATE_WALK );						
	}

	return MoveMode;
}

//공격받은 시간 검사하여 원거리 공격 탐지
int smCHAR::CheckShootingTest_LastAttackTime( smCHAR *lpChar )
{
	if ( dwLastRecvAttackTime<dwPlayTime && (dwLastRecvAttackTime+2000)>dwPlayTime ) 
		return TRUE;

	return CheckShootingTest(lpChar);
}

//발사형 무기 메인함수
int smCHAR::CheckShootingTest(smCHAR *lpChrTarget)
{
//	POINT3D Posi;
	int x,y,z;
	int he;//,hy;
	int step;
	int cnt;

	int angX,angY;
	int	SucessCnt;

	if ( lpChrTarget->smCharInfo.dwCharSoundCode==snCHAR_SOUND_CASTLE_TOWER_B ) 	//발할라타워 제외
		return TRUE;

	GetRadian3D( pX,pY,pZ , lpChrTarget->pX,lpChrTarget->pY,lpChrTarget->pZ );
	angX = GeoResult_X;
	angY = GeoResult_Y;

	ShootingPosi.x = pX;
	ShootingPosi.y = pY+34*fONE;
	ShootingPosi.z = pZ;
	
	ShootingAngle.x = angX;
	ShootingAngle.y = angY;
	ShootingAngle.z = 0;

	SucessCnt = 0;

	step = 16*fONE;

	GetMoveLocation( 0, 0, step, angX, angY, 0 );

	for ( cnt=0;cnt<40;cnt++ ) {
		ShootingPosi.x += GeoResult_X;
		ShootingPosi.y += GeoResult_Y;
		ShootingPosi.z += GeoResult_Z;

		he = TRUE;

		//지정된 방향의 거리를 잇는 선분에 폴리건이 충돌하는지 확인 
		
		if ( lpStage ) {
			he = lpStage->CheckVecImpact( &ShootingPosi, &ShootingAngle , step );
		}
		else {
			if ( smGameStage[0] ) {
				he = smGameStage[0]->CheckVecImpact( &ShootingPosi, &ShootingAngle , step );	
			}

			if ( he==TRUE && smGameStage[1] ) {
				he = smGameStage[1]->CheckVecImpact( &ShootingPosi, &ShootingAngle , step );
			}
		}

		if ( chrAttackTarget ) {

			x = abs( ShootingPosi.x-lpChrTarget->pX );
			z = abs( ShootingPosi.z-lpChrTarget->pZ );
			y = abs( ShootingPosi.y-lpChrTarget->pY );

			if ( x<48*fONE && z<48*fONE && y<64*fONE && !SucessCnt ) {
				//return TRUE;
				SucessCnt = cnt+2;
			}
		}

		if ( SucessCnt && SucessCnt<cnt ) 
			return TRUE;


		if ( he==FALSE ) 
			return FALSE;


	}

	return TRUE;


}



//발사형 무기 메인함수
int smCHAR::ShootingMain()
{

	POINT3D Posi,pos;
	int x,y,z;
	int he;//,hy;
	int ShootSize;
	int step;
	int r,g,b;
	int EffectFlag;

	he = CLIP_OUT;
	EffectFlag = 0;

	ShootSize = 512;

//	step = 1*fONE;
	if ( (dwActionItemCode&sinITEM_MASK2)==sinWS1 )
		step = 8*fONE;				//화살
	else
		step = 6*fONE;				//창이나 기타 던지는 무기

	Posi.x = x = ShootingPosi.x;
	Posi.y = y = ShootingPosi.y;
	Posi.z = z = ShootingPosi.z;

	GetMoveLocation( 0, 0, step, ShootingAngle.x, ShootingAngle.y, ShootingAngle.z );
	ShootingPosi.x += GeoResult_X;
	ShootingPosi.y += GeoResult_Y;
	ShootingPosi.z += GeoResult_Z;

	if ( smCharInfo.State==smCHAR_STATE_USER ) {

		if (dwActionItemCode==HvRightHand.dwItemCode ) {
			if ( HvRightHand.PatTool && HvRightHand.ColorBlink && !HvRightHand.EffectKind ) {

				r = (HvRightHand.sColors[SMC_R]>>1)+128;
				g = (HvRightHand.sColors[SMC_G]>>1)+128;
				b = (HvRightHand.sColors[SMC_B]>>1)+128;

				StartEffect( ShootingPosi.x,ShootingPosi.y,ShootingPosi.z,
					r,g,b,255, EFFECT_PARTICLE_BOW1 );

				EffectFlag++;
			}
		}
		if (dwActionItemCode==HvLeftHand.dwItemCode ) {
			if ( HvLeftHand.PatTool && HvLeftHand.ColorBlink && !HvLeftHand.EffectKind ) {

				r = (HvLeftHand.sColors[SMC_R]>>1)+128;
				g = (HvLeftHand.sColors[SMC_G]>>1)+128;
				b = (HvLeftHand.sColors[SMC_B]>>1)+128;

				StartEffect( ShootingPosi.x,ShootingPosi.y,ShootingPosi.z,
					r,g,b,255, EFFECT_PARTICLE_BOW1 );

				EffectFlag++;
			}
		}

		switch( ShootingKind ) {
		case SKILL_PLAY_FARINA:
			//파리나 이팩트
			sinEffect_Farina_Particle(&ShootingPosi);
			break;
		}
	}

//	y += fONE* ;
/*
		he = TRUE;

		//지정된 방향의 거리를 잇는 선분에 폴리건이 충돌하는지 확인 
		
		if ( lpStage ) {
			he = lpStage->CheckVecImpact( &ShootingPosi, &ShootingAngle , step );
		}
		else {
			if ( smGameStage[0] ) {
				he = smGameStage[0]->CheckVecImpact( &ShootingPosi, &ShootingAngle , step );	
			}

			if ( he==TRUE && smGameStage[1] ) {
				he = smGameStage[1]->CheckVecImpact( &ShootingPosi, &ShootingAngle , step );
			}
		}
*/
		if ( ShootingCount>60*2 ) { //|| he==FALSE  ) {
			ShootingFlag=FALSE;
		}

	ShootingCount ++;

	if ( ShootingFlag>1 && chrAttackTarget ) {
		ShootingFlag++;
		if ( ShootingFlag>8 ) ShootingFlag = FALSE;

		if ( EffectFlag ) {
			StartEffect( chrAttackTarget->pX,ShootingPosi.y,chrAttackTarget->pZ,
				r,g,b,255, EFFECT_PARTICLE_BOW1 );
		}
		return TRUE;
	}

	if ( ShootingFlag==1 && chrAttackTarget && AttackCritcal>=0 ) {

		x = abs( ShootingPosi.x-chrAttackTarget->pX );
		z = abs( ShootingPosi.z-chrAttackTarget->pZ );
		y = abs( ShootingPosi.y-chrAttackTarget->pY );

		if ( x<64*fONE && z<64*fONE && y<80*fONE ) {
			ShootingFlag++;


			if ( ShootingKind==SKILL_PLAY_FARINA ) {
				//파리나 이팩트
				Posi.x = chrAttackTarget->pX;
				Posi.y = chrAttackTarget->pY+22*fONE;
				Posi.z = chrAttackTarget->pZ;
				sinEffect_SkillHit( SKILL_FARINA ,  &Posi );
				SetDynLight( chrAttackTarget->pX,chrAttackTarget->pY+22*fONE,chrAttackTarget->pZ , 100, 0,0,0, 400 );
				PlayWaponSoundDirect( chrAttackTarget->pX,chrAttackTarget->pY,chrAttackTarget->pZ , 16 );	//크리티컬 
			}
			else {
				if ( WeaponEffect ) {
					pos.x = chrAttackTarget->pX;
					pos.y = chrAttackTarget->pY+22*fONE;
					pos.z = chrAttackTarget->pZ;

					switch( WeaponEffect ) {
						case (sITEMINFO_LIGHTING+1):
							AssaParticle_LightHit1( &pos );
							SetDynLight( x,y,z , 0,0,100,0, 400 );
							if ( (rand()%2)==0 ) 
								SkillPlaySound( SKILL_SOUND_SKILL_ENCHANTWEAPON_LIGHTNING1	 , pX,pY,pZ );
							else
								SkillPlaySound( SKILL_SOUND_SKILL_ENCHANTWEAPON_LIGHTNING2	 , pX,pY,pZ );
							break;
						case (sITEMINFO_FIRE+1):
							AssaParticle_FireHit1( &pos );
							SetDynLight( x,y,z , 100, 0,0,0, 400 );
							if ( (rand()%2)==0 ) 
								SkillPlaySound( SKILL_SOUND_SKILL_ENCHANTWEAPON_FIRE1	 , pX,pY,pZ );
							else
								SkillPlaySound( SKILL_SOUND_SKILL_ENCHANTWEAPON_FIRE2	 , pX,pY,pZ );
							break;
						case (sITEMINFO_ICE+1):
							AssaParticle_IceHit1( &pos );
							SetDynLight( x,y,z , 0,0,100,0, 400 );
							if ( (rand()%2)==0 ) 
								SkillPlaySound( SKILL_SOUND_SKILL_ENCHANTWEAPON_ICE1	 , pX,pY,pZ );
							else
								SkillPlaySound( SKILL_SOUND_SKILL_ENCHANTWEAPON_ICE2	 , pX,pY,pZ );
							break;
					}
				}
				else {
					// 도구 공격
					if ( AttackCritcal ) {
						StartEffect( chrAttackTarget->pX,chrAttackTarget->pY+22*fONE,chrAttackTarget->pZ , EFFECT_CRITICAL_HIT1 );
						SetDynLight( chrAttackTarget->pX,chrAttackTarget->pY+22*fONE,chrAttackTarget->pZ , 100, 0,0,0, 400 );
						PlayWaponSoundDirect( chrAttackTarget->pX,chrAttackTarget->pY,chrAttackTarget->pZ , 16 );	//크리티컬 
					}
					else {
						StartEffect( chrAttackTarget->pX,chrAttackTarget->pY+22*fONE,chrAttackTarget->pZ , EFFECT_NORMAL_HIT1 );
						SetDynLight( chrAttackTarget->pX,chrAttackTarget->pY+22*fONE,chrAttackTarget->pZ , 40, 0,0,0, 400 );
					}

					if ( dwForceOrbTime>dwPlayTime ) {		//포스오브 공격 이펙트
						pos.x = chrAttackTarget->pX;
						pos.y = chrAttackTarget->pY+22*fONE;
						pos.z = chrAttackTarget->pZ;

						AssaParticle_ShelltomWeaponHit(&pos);
					}

				}
			}
			//공격소리
			//PlayWav( 9 , 0 , 300 , 2205 );
			PlayWaponSoundDirect( chrAttackTarget->pX,chrAttackTarget->pY,chrAttackTarget->pZ , 14 );	//펀치소리

			//경험치 일부 상승
			if ( this==lpCurPlayer && AttackExp>0 ) {
				//AddExp( AttackExp );
				//SetTotalAddExp( AttackExp );			//경험치 증가 기록
				AttackExp = 0;
			}

		}

	}

	return TRUE;
}

//스킬 차징
int smCHAR::ChargingSkill()
{
	DWORD dwFrame;

	if ( this!=lpCurPlayer ) return FALSE;

	dwFrame = frame-MotionInfo->StartFrame*160;

	if ( SkillChargingFlag && MouseButton[1]==0 ) SkillChargingFlag=0;

	switch ( AttackSkil&0xFF ) {
	case SKILL_PLAY_PHOENIX_SHOT:
		if ( dwFrame>=MotionInfo->EventFrame[0] && dwFrame<MotionInfo->EventFrame[1] ) {
			if ( SkillChargingFlag==0 ) {
				frame = MotionInfo->StartFrame*160+MotionInfo->EventFrame[1];
				ChargingFlag = 1;
			}
		}
		break;
	case SKILL_PLAY_CHARGING_STRIKE:
		if ( dwFrame>=MotionInfo->EventFrame[0] && dwFrame<MotionInfo->EventFrame[1] ) {
			if ( SkillChargingFlag==0 ) {
				frame = MotionInfo->StartFrame*160+MotionInfo->EventFrame[1];
				ChargingFlag = 1;
			}
		}
		break;
	}

	return TRUE;
}

//공격 이벤트
int smCHAR::EventAttack( int Flag )
{

	int shootFlag;
	DWORD compFrame;
	int x,y,z;
	int wp;
	DWORD NearFrame;
	POINT3D	pos,pos2;

	int	cnt,ChrCnt;
	smCHAR *lpCharList[32];

//	if ( MotionInfo->State<0x100 ) return FALSE;

	ChargingSkill();

	if ( AttackSkil>0 || (smCharInfo.State==smCHAR_STATE_ENEMY && MotionInfo->State==CHRMOTION_STATE_SKILL ) ) {
		if ( Flag || ((MotionInfo->State==CHRMOTION_STATE_ATTACK || MotionInfo->State==CHRMOTION_STATE_SKILL ) && MotionInfo->EventFrame[0]) ) {
			compFrame = frame-(MotionInfo->StartFrame*160);
			//if ( MotionInfo->EventFrame[0]>(compFrame-FrameStep) && MotionInfo->EventFrame[0]<=compFrame ) {
			if ( (MotionInfo->EventFrame[0]>(compFrame-FrameStep) && MotionInfo->EventFrame[0]<=compFrame) ||
				(MotionInfo->EventFrame[1] && MotionInfo->EventFrame[1]>(compFrame-FrameStep) && MotionInfo->EventFrame[1]<=compFrame) ||
				(MotionInfo->EventFrame[2] && MotionInfo->EventFrame[2]>(compFrame-FrameStep) && MotionInfo->EventFrame[2]<=compFrame) ||
				(MotionInfo->EventFrame[3] && MotionInfo->EventFrame[3]>(compFrame-FrameStep) && MotionInfo->EventFrame[3]<=compFrame) ||

				(dwEventFrameExt[0] && dwEventFrameExt[0]>(frame-FrameStep) && dwEventFrameExt[0]<=frame) ||
				(dwEventFrameExt[1] && dwEventFrameExt[1]>(frame-FrameStep) && dwEventFrameExt[1]<=frame) ||
				(dwEventFrameExt[2] && dwEventFrameExt[2]>(frame-FrameStep) && dwEventFrameExt[2]<=frame) ||
				(dwEventFrameExt[3] && dwEventFrameExt[3]>(frame-FrameStep) && dwEventFrameExt[3]<=frame) ||

				( MotionEvent==0 && MotionInfo->EventFrame[0]<=compFrame ) ) {

				//스킬이벤트
				MotionEvent++;

				//경험치 일부 상승 ( 주인공 만 )
				if ( this==lpCurPlayer ) {//&& AttackExp>0 ) { 
					//AddExp( AttackExp );
					//SetTotalAddExp( AttackExp );			//경험치 증가 기록
					AttackExp=0;

					if ( chrAttackTarget && abs(chrAttackTarget->smCharInfo.Level-smCharInfo.Level)<=AGING_SUB_LEVEL ) {
						//레벨 7차이 이하는 에이징 적용
						if ( AttackCritcal==TRUE )
							sinCheckAgingLevel(SIN_AGING_CRITICAL, false);
						else
							sinCheckAgingLevel(SIN_AGING_ATTACK, false);
					}
				}

				if ( EventSkill()==TRUE )
					return TRUE;
			}
		}
	}


	if ( MotionInfo->State!=CHRMOTION_STATE_ATTACK && MotionInfo->State!=CHRMOTION_STATE_SKILL ) return FALSE;

	if ( ActionPattern )
		shootFlag = 0;
	else {
		shootFlag = ShootingFlag;
	}

	if ( MotionInfo->EventFrame[0] ) {
			compFrame = frame-(MotionInfo->StartFrame*160);
			NearFrame = compFrame-FrameStep;

			if ( Flag || (MotionInfo->EventFrame[0]>NearFrame && MotionInfo->EventFrame[0]<=compFrame) ||
				(MotionInfo->EventFrame[1] && MotionInfo->EventFrame[1]>NearFrame && MotionInfo->EventFrame[1]<=compFrame) ) {

				MotionEvent++;

				if ( smCharInfo.State==smCHAR_STATE_ENEMY ) {
					CharPlaySound( this );
				}
				else {
					WeaponPlaySound( this );
					if ( AttackCritcal<0 ) return FALSE;			//공격 실패시
				}

				//경험치 일부 상승 ( 주인공 만 )
				if ( this==lpCurPlayer && AttackExp>0 ) { 

					//AttackExp = 500000;
	
					//AddExp( AttackExp );
					//SetTotalAddExp( AttackExp );			//경험치 증가 기록
					AttackExp=0;

					//SaveGameData();					

					if ( chrAttackTarget && abs(chrAttackTarget->smCharInfo.Level-smCharInfo.Level)<=AGING_SUB_LEVEL ) {
						//레벨 7차이 이하는 에이징 적용
						if ( AttackCritcal==TRUE )
							sinCheckAgingLevel(SIN_AGING_CRITICAL, false);
						else
							sinCheckAgingLevel(SIN_AGING_ATTACK, false);
					}

				}

				//주인공을 향한 공격
				if (chrAttackTarget==lpCurPlayer ) {
					//공격 받은 모션 카운터
					AttackMotionCountUp();
				}

				switch( smCharInfo.dwCharSoundCode ) {
				case snCHAR_SOUND_MUSHROOM:
					//버섯 독가스 발사
					StartEffect( pX,pY+(24*fONE),pZ , EFFECT_GAS1 );
					break;

				case snCHAR_SOUND_IMP:
					//임프 매핏
					if (chrAttackTarget) {
						if ( CheckShootingTest_LastAttackTime(chrAttackTarget) ||
							CheckLocateBox( pX,pY,pZ, chrAttackTarget->pX, chrAttackTarget->pY, chrAttackTarget->pZ, 80*fONE )) {

							GetMoveLocation( 0 , 0, 10*fONE , 0, Angle.y , 0 );
							x = pX+GeoResult_X;
							y = pY+8*fONE;
							z = pZ+GeoResult_Z;

							if ( smCharInfo.Level<15 ) {
								//임프
								StartEffectMonsterDest( x,y,z, 
									chrAttackTarget->pX , chrAttackTarget->pY+24*fONE , chrAttackTarget->pZ ,
									MONSTER_IMP_SHOT1 );
							}
							else {
								//메핏
								StartEffectMonsterDest( x,y,z, 
									chrAttackTarget->pX , chrAttackTarget->pY+24*fONE , chrAttackTarget->pZ ,
									MONSTER_MEPHIT_SHOT1 );
							}
						}
						else {
							ResetAttackTrans();		//AttackTrans.code = 0;
						}
					}
					break;

				case snCHAR_SOUND_SHADOW:
					//쉐도우 공격
					if (chrAttackTarget) {
						if ( CheckShootingTest_LastAttackTime(chrAttackTarget) ||
							CheckLocateBox( pX,pY,pZ, chrAttackTarget->pX, chrAttackTarget->pY, chrAttackTarget->pZ, 80*fONE )) {

							StartEffectMonsterDest( pX,pY+32*fONE,pZ, 
								chrAttackTarget->pX , chrAttackTarget->pY+24*fONE , chrAttackTarget->pZ ,
								MONSTER_IMP_SHOT3 );
						}
						else {
							ResetAttackTrans();		//AttackTrans.code = 0;
						}
					}
					break;

				case snCHAR_SOUND_FIGON:
					//파이곤 공격
					if (chrAttackTarget) {
						if ( CheckShootingTest_LastAttackTime(chrAttackTarget) ||
							CheckLocateBox( pX,pY,pZ, chrAttackTarget->pX, chrAttackTarget->pY, chrAttackTarget->pZ, 80*fONE )) {

							StartEffectMonsterDest( pX,pY+32*fONE,pZ, 
								chrAttackTarget->pX , chrAttackTarget->pY+24*fONE , chrAttackTarget->pZ ,
								MONSTER_PIGON_SHOT1 );
						}
						else {
							ResetAttackTrans();		//AttackTrans.code = 0;
						}
					}
					break;

				case snCHAR_SOUND_WEB:
					//눈튀어나온 거미
					if (chrAttackTarget) {
						if ( CheckShootingTest_LastAttackTime(chrAttackTarget) ||
							CheckLocateBox( pX,pY,pZ, chrAttackTarget->pX, chrAttackTarget->pY, chrAttackTarget->pZ, 80*fONE ) ) {

							GetMoveLocation( 0 , 0, 32*fONE , 0, Angle.y , 0 );
							StartEffectMonsterDest( pX+GeoResult_X,pY+12*fONE,pZ+GeoResult_Z, 
								chrAttackTarget->pX , chrAttackTarget->pY+24*fONE , chrAttackTarget->pZ ,
								MONSTER_WEB_SHOT1 );
						}
						else {
							ResetAttackTrans();		//AttackTrans.code = 0;
						}
					}
					break;

				case snCHAR_SOUND_SKELETONRANGE:
					//활쏘는 해골
					if (chrAttackTarget) {
						if ( CheckShootingTest_LastAttackTime(chrAttackTarget) ) {
							ShootingPosi.x = pX;
							ShootingPosi.y = pY+28*fONE;
							ShootingPosi.z = pZ;
							GetRadian3D( pX,pY+28*fONE,pZ, chrAttackTarget->pX,chrAttackTarget->pY+24*fONE,chrAttackTarget->pZ );
							ShootingAngle.x = GeoResult_X;
							ShootingAngle.y = GeoResult_Y;
							ShootingAngle.z = Angle.z;
							ShootingFlag = TRUE;
							ShootingCount = 0;
							dwActionItemCode = sinWS1;
						}
						else {
							ResetAttackTrans();		//AttackTrans.code = 0;
						}
					}
					break;

				case snCHAR_SOUND_WOLVERLIN:
					//을버린
 					if (chrAttackTarget) { 
						GetMoveLocation( 0,0,50*fONE , 0,Angle.y,0 );
						x = pX+GeoResult_X;
						z = pZ+GeoResult_Z;
						y = pY+18*fONE;
						//StartEffect( x,y,z, EFFECT_FIRE_HIT2 );
 				 		//StartEffect( x,y,z , EFFECT_NORMAL_HIT1 );
 						StartEffectMonster(x,y,z, MONSTER_PIGON_PARTICLE1);
						StartEffectMonster(x,y,z, MONSTER_PIGON_PARTICLE1);
						StartEffectMonster(x,y,z, MONSTER_PIGON_PARTICLE1);
						StartEffectMonster(x,y,z, MONSTER_PIGON_PARTICLE1);

					}
					break;

				case snCHAR_SOUND_EGAN:
					//이간
					if (chrAttackTarget) {
						if ( CheckShootingTest_LastAttackTime(chrAttackTarget) ) {
							GetMoveLocation( 0 , 0, 10*fONE , 0, Angle.y , 0 );
							x = pX+GeoResult_X;
							y = pY+8*fONE;
							z = pZ+GeoResult_Z;

							StartEffectMonsterDest( x,y,z, 
								chrAttackTarget->pX , chrAttackTarget->pY+24*fONE , chrAttackTarget->pZ ,
								MONSTER_IMP_SHOT1 );
						}
						else {
							ResetAttackTrans();		//AttackTrans.code = 0;
						}
					}
					break;

				case snCHAR_SOUND_MUTANTTREE:
					//뮤턴트 트리
					if (chrAttackTarget) {
						if ( CheckShootingTest_LastAttackTime(chrAttackTarget) ) {
							GetMoveLocation( 0 , 0, 24*fONE , 0, Angle.y , 0 );
							x = pX+GeoResult_X;
							y = pY+8*fONE;
							z = pZ+GeoResult_Z;

							sinEffect_MutentTree_Attack( this , chrAttackTarget );
						}
						else {
							ResetAttackTrans();		//AttackTrans.code = 0;
						}
					}
					break;

				case snCHAR_SOUND_HULK:
					//헐크
					if (chrAttackTarget) {
						GetMoveLocation( 0 , 0, 54*fONE , 0, Angle.y , 0 );
						StartEffectMonster( pX+GeoResult_X, pY+48*fONE,pZ+GeoResult_Z, MONSTER_HULK_HIT1);
					}
					break;


				case snCHAR_SOUND_DMACHINE:
					//디머신
					if (chrAttackTarget) {
						GetMoveLocation( 0 , 30*fONE, 10*fONE , 0, Angle.y , 0 );
						StartEffectMonsterDest(pX+GeoResult_X, pY+GeoResult_Y,pZ+GeoResult_Z, 
							chrAttackTarget->pX , chrAttackTarget->pY+24*fONE , chrAttackTarget->pZ ,
							MONSTER_DMACHINE_MISSILE1);
					}
					break;

				case snCHAR_SOUND_TURTLE_CANNON:
					//터틀캐논
					if (chrAttackTarget) {
						ParkAssaParticle_MonsterTurtleCanonNormal( this , chrAttackTarget );
					}
					break;

				case snCHAR_SOUND_SUCCUBUS:
					//써큐버스
					if (chrAttackTarget) {

						if ( smCharInfo.Level<=75 )
							wp = MONSTER_SERQBUS_SHOT1;	//휘치
						else if ( smCharInfo.Level<80 )
							wp = MONSTER_SERQBUS_SHOT2;	//써큐버스
						else
							wp = MONSTER_SERQBUS_SHOT3;	//인큐버스

						GetMoveLocation( 0 , 20*fONE, 20*fONE , 0, Angle.y , 0 );
						StartEffectMonsterDest(pX+GeoResult_X, pY+GeoResult_Y,pZ+GeoResult_Z, 
							chrAttackTarget->pX , chrAttackTarget->pY+24*fONE , chrAttackTarget->pZ ,
							wp );
					}
					break;

				case snCHAR_SOUND_STYGIANLORD:
					//스티지언로드
					if (chrAttackTarget) {
						//GetMoveLocation( 0 , 20*fONE, 20*fONE , 0, Angle.y , 0 );
						//StartEffectMonsterDest(pX+GeoResult_X, pY+GeoResult_Y,pZ+GeoResult_Z, 
						StartEffectMonsterDest(pX, pY, pZ, 
							chrAttackTarget->pX , chrAttackTarget->pY+24*fONE , chrAttackTarget->pZ ,
							0,Angle.y,0, MONSTER_STYGIANLORD_SHOT1 );
					}
					break;

				case snCHAR_SOUND_MOKOVA:				//모코바
				case snCHAR_SOUND_OMICRON:
					//헐크 꺼랑 동일
					if (chrAttackTarget) {
						GetMoveLocation( 0 , 0, 68*fONE , 0, Angle.y , 0 );
						StartEffectMonster( pX+GeoResult_X, pY+22*fONE,pZ+GeoResult_Z, MONSTER_HULK_HIT1);
					}
					break;

				case snCHAR_SOUND_GUARDIAN_SAINT:
					//가디안 - 세인트
					AssaParticleClanMonsterHit2( this );
					break;

				case snCHAR_SOUND_HYPERMACHINE:		//하이퍼 머신
					if (chrAttackTarget) {
						pos.x = chrAttackTarget->pX;
						pos.y = chrAttackTarget->pY+20*fONE;
						pos.z = chrAttackTarget->pZ;
						AssaParticle_MonsterIronHyperHit1( this , &pos );
					}
					break;

				case snCHAR_SOUND_MOUNTAIN:			//마운틴
					AssaParticle_MonsterIronMountainHit1( this );
					break;

				case snCHAR_SOUND_TOWERGOLEM:
					AssaParticle_MonsterIronTowerGolemHit1( this );
					break;

				case snCHAR_SOUND_OMU:
					AssaParticle_MonsterIronOmuHit1( this );
					break;

				case snCHAR_SOUND_CHAINGOLEM:
				case snCHAR_SOUND_DEADZONE:
				case snCHAR_SOUND_IRONFIST:
				case snCHAR_SOUND_MORGON:
				case snCHAR_SOUND_RAMPAGE:
				case snCHAR_SOUND_VAMPRICBEE:
				case snCHAR_SOUND_IRONGUARD:
				case snCHAR_SOUND_GORGON:	// pluto 추가 몬스터 고르곤
				case snCHAR_SOUND_HOBOGOLEM: // pluto 추가 몬스터 호보고렘

					GetMoveLocation( 0 , 0, 54*fONE , 0, Angle.y , 0 );
					pos.x = pX+GeoResult_X;
					pos.y = pY+GeoResult_Y+32*fONE;
					pos.z = pZ+GeoResult_Z;
					AssaParticle_MonsterIronHit1( &pos );
					break;
				
				case snCHAR_SOUND_GOBLINSHAMAN: // 박재원 - 저주받은 신전 3층(신규필드 몬스터) - 고블린샤먼
					if(chrAttackTarget)
					{
						ParkAssaParticle_MummyShot( this , chrAttackTarget );	// 머미로드 통상공격
					}
					break;

				case snCHAR_SOUND_GROTESQUE:
					GetMoveLocation( 0 , 0, 75*fONE , 0, Angle.y , 0 );
					pos.x = pX+GeoResult_X;
					pos.y = pY+GeoResult_Y+32*fONE;
					pos.z = pZ+GeoResult_Z;
					AssaParticle_MonsterIronHit1( &pos );
					break;


				case snCHAR_SOUND_AVELIN_ARCHER:
					if (chrAttackTarget) {
						AssaParticle_MonsterAvelArcher(this, chrAttackTarget); //아벨리스크 아처
					}
					break;

				case snCHAR_SOUND_AVELIN_QUEEN:
					if (chrAttackTarget) {
						AssaParticle_MonsterAvelQueen(this, chrAttackTarget); //아벨리스크 아처
					}
					break;

				case snCHAR_SOUND_RUNICGUARDIAN:
					if (chrAttackTarget) {
						AssaParticle_MonsterRunicGuardianShot( this , chrAttackTarget );	//루닉가디안
					}
					break;

				case snCHAR_SOUND_BABEL:				//바벨 (루닉가디안 보스)

					if (chrAttackTarget) {
						if ( Trans_SplashCharList.code==smTRANSCODE_ATTACK_SLASH_LIST && Trans_SplashCharList.TargetCount>1 ) {
							ChrCnt = 0;
							for(cnt=0;cnt<Trans_SplashCharList.TargetCount;cnt++) {
								if ( cnt>=32 ) break;
								lpCharList[ChrCnt] = FindChrPlayer( Trans_SplashCharList.dwTarObjectSerial[cnt] );
								if ( lpCharList[ChrCnt] ) ChrCnt++;
							}
							AssaParticle_MonsterRunicGuardianBossHit( this , chrAttackTarget , lpCharList , ChrCnt );	//바벨
							Trans_SplashCharList.code = 0;
						}
						else
							AssaParticle_MonsterRunicGuardianBossHit( this , chrAttackTarget );	//바벨
					}
					break;

				case snCHAR_SOUND_SADNESS:			//새드니스
					if (chrAttackTarget) {
						AssaParticle_MonsterIronSadNessHit1( this );
					}
					break;

				// 장별 - 수박몬스터 일반공격동작
				case snCHAR_SOUND_WATERMELON: 
					if(chrAttackTarget)
					{
						GetMoveLocation( 0 , 0, 54*fONE , 0, Angle.y , 0 );
						pos.x = pX+GeoResult_X;
						pos.y = pY+GeoResult_Y+32*fONE;
						pos.z = pZ+GeoResult_Z;
						AssaParticle_MonsterIronHit1( &pos );
					}
				break;


				////////////////// 아이스 몬스터 /////////////////////
				case snCHAR_SOUND_COLDEYE:
					if (chrAttackTarget) {
						GetMoveLocation( 0 , 0, 24*fONE , 0, Angle.y , 0 );
						pos.x = pX+GeoResult_X;
						pos.y = pY+64*fONE;
						pos.z = pZ+GeoResult_Z;
						pos2.x = chrAttackTarget->pX;
						pos2.y = chrAttackTarget->pY+64*fONE;
						pos2.z = chrAttackTarget->pZ;
						ParkAssaParticle_ColdEye( &pos , &pos2 );
					}
					break;

				case snCHAR_SOUND_FROZEN:
					if (chrAttackTarget) {
						ParkAssaParticle_MistIceBolt( this , chrAttackTarget);
					}
					break;

				case snCHAR_SOUND_FROST:
					if (chrAttackTarget && MotionInfo->KeyCode=='U' ) {
						ParksinSkillEffect_AcientPiercing(this);
					}
					break;

				case snCHAR_SOUND_CHAOSCARA:
					if (chrAttackTarget) {
						ParkAssaParticle_ChaosKara1( chrAttackTarget );
					}
					break;

				case snCHAR_SOUND_DEATHKNIGHT:			//발렌토
					if ( MotionInfo->KeyCode=='P' ) {
						if ( MotionEvent==1 )
							ParkAssaParticle_Valento1( this );
						else
							ParkAssaParticle_Valento3_1( this );
					}
					else if ( MotionInfo->KeyCode=='L' ) {
						if ( MotionEvent==1 )
							ParkAssaParticle_Valento2_1( this );
						else
							ParkAssaParticle_Valento2_2( this );
					}
					break;


				////////////////// 그리디호수 몬스터 /////////////////////
				case snCHAR_SOUND_GREATE_GREVEN:
				case snCHAR_SOUND_LIZARDFOLK:
				case snCHAR_SOUND_SPIDER:
				case snCHAR_SOUND_STRIDER:
					if (chrAttackTarget) {
						GetMoveLocation( 0 , 0, 68*fONE , 0, Angle.y , 0 );
						StartEffectMonster( pX+GeoResult_X, pY+22*fONE,pZ+GeoResult_Z, MONSTER_HULK_HIT1);
					}
					break;

				case snCHAR_SOUND_M_LORD:
					if (chrAttackTarget) {
						ParkAssaParticle_MummyShot( this , chrAttackTarget );	// 머미로드 통상공격
					}
					break;


				////////////////// // 박재원 - 끝없는 탑 3층(신규필드 몬스터) /////////////////////
				case snCHAR_SOUND_IGOLATION: // 이골라티온 : 일반 공격
					if (chrAttackTarget && MotionInfo->KeyCode=='H') 
					{
						GetMoveLocation( 0 , 0, 68*fONE , 0, Angle.y , 0 );
						StartEffectMonster( pX+GeoResult_X, pY+22*fONE,pZ+GeoResult_Z, MONSTER_HULK_HIT1);
					}
					break;
				case snCHAR_SOUND_NIKEN:  // 니켄 : 일반 공격
					if (chrAttackTarget)
					{	
						pos.x = chrAttackTarget->pX;
						pos.y = chrAttackTarget->pY;
						pos.z = chrAttackTarget->pZ;

						AssaParticle_SnailShot( this , &pos );
					}
					break;
				case snCHAR_SOUND_XETAN: // 제탄 : 일반 공격
					if (chrAttackTarget && MotionInfo->KeyCode=='G')
					{	
						GetMoveLocation( 0 , 40*fONE, 20*fONE , 0, Angle.y , 0 );
						StartEffectMonsterDest(pX+GeoResult_X, pY+GeoResult_Y,pZ+GeoResult_Z, 
							chrAttackTarget->pX , chrAttackTarget->pY+24*fONE , chrAttackTarget->pZ ,
							MONSTER_SERQBUS_SHOT2 ); 
					}
					break;
				case snCHAR_SOUND_SPRIN: // 스프린 : 일반 공격
					if (chrAttackTarget && (MotionInfo->KeyCode=='G' || MotionInfo->KeyCode=='H') )
					{
						if ( CheckShootingTest_LastAttackTime(chrAttackTarget) ||
							CheckLocateBox( pX,pY,pZ, chrAttackTarget->pX, chrAttackTarget->pY, chrAttackTarget->pZ, 80*fONE ) ) {

								GetMoveLocation( 0 , 0, 32*fONE , 0, Angle.y , 0 );
								StartEffectMonsterDest( pX+GeoResult_X,pY+12*fONE,pZ+GeoResult_Z, 
									chrAttackTarget->pX , chrAttackTarget->pY+24*fONE , chrAttackTarget->pZ ,
									MONSTER_WEB_SHOT1 );
							}
						else {
							ResetAttackTrans();
						}
					}
					break;


			////////////////// // 박재원 - 복날 이벤트 몬스터 추가 /////////////////////

				case snCHAR_SOUND_BEBECHICK:	// 아기 꼬꼬
					if(chrAttackTarget && (MotionInfo->KeyCode=='H' || MotionInfo->KeyCode=='J') )
					{
						//  이펙트 정의 안함.(모션만 설정)
					}
					break;
				case snCHAR_SOUND_PAPACHICK:	// 아빠 꼬꼬
					if(chrAttackTarget && MotionInfo->KeyCode=='H')
					{
						//  이펙트 정의 안함.(모션만 설정)
					}
					break;

			/////////////////////////////////////////////////////////////////////////////////////

				case snCHAR_SOUND_KELVEZU:
					if ( chrAttackTarget ) {

						if ( MotionInfo->KeyCode!='P' ) {
							ParkKelvezuNormal2( this );
							ParkKelvezuHit(chrAttackTarget);
						}
						else {
							ParkKelvezuNormal1(chrAttackTarget);
						}
					}
					break;

				////////////////////////////////////////////////////
/*
					if (chrAttackTarget) {
						GetMoveLocation( 0 , 0, 54*fONE , 0, Angle.y , 0 );
						StartEffectMonster( pX+GeoResult_X, pY+48*fONE,pZ+GeoResult_Z, MONSTER_HULK_HIT1);
					}
*/
				case snCHAR_SOUND_DARKPHALANX:
					if (chrAttackTarget) {
						GetMoveLocation( 0 , 0, 70*fONE , 0, Angle.y , 0 );
						StartEffectMonster( pX+GeoResult_X, pY+42*fONE,pZ+GeoResult_Z, MONSTER_HULK_HIT1);
					}
					break;
				case snCHAR_SOUND_BLOODYKNIGHT:
					if (chrAttackTarget) {
						GetMoveLocation( 0 , 0, 70*fONE , 0, Angle.y , 0 );
						StartEffectMonster( pX+GeoResult_X, pY+42*fONE,pZ+GeoResult_Z, MONSTER_HULK_HIT1);
					}
					break;

				case snCHAR_SOUND_CHIMERA:
					if (chrAttackTarget) {
						GetMoveLocation( 0 , 0, 70*fONE , 0, Angle.y , 0 );
						StartEffectMonster( pX+GeoResult_X, pY+42*fONE,pZ+GeoResult_Z, MONSTER_HULK_HIT1);
					}
					break;

				case snCHAR_SOUND_FIREWORM:
					if ( chrAttackTarget ) {
						ParkAssaParticle_FireWorm( chrAttackTarget , 0 );
					}
					break;
				case snCHAR_SOUND_HELLHOUND:
					if ( chrAttackTarget ) {
						ParkAssaParticle_HellHoundNormal( chrAttackTarget );

						GetMoveLocation( 0 , 0, 70*fONE , 0, Angle.y , 0 );
						StartEffectMonster( pX+GeoResult_X, pY+42*fONE,pZ+GeoResult_Z, MONSTER_HULK_HIT1);
					}
					break;
				case snCHAR_SOUND_DARKMAGE:
					if ( chrAttackTarget ) {
						ParkAssaParticle_DarkMageNormal( this , chrAttackTarget );
					}
					break;
				case snCHAR_SOUND_DARKGUARD:
					//활쏘는 다크가드
					if (chrAttackTarget) {
						if ( CheckShootingTest_LastAttackTime(chrAttackTarget) ) {
							ShootingPosi.x = pX;
							ShootingPosi.y = pY+38*fONE;
							ShootingPosi.z = pZ;
							GetRadian3D( pX,pY+38*fONE,pZ, chrAttackTarget->pX,chrAttackTarget->pY+24*fONE,chrAttackTarget->pZ );
							ShootingAngle.x = GeoResult_X;
							ShootingAngle.y = GeoResult_Y;
							ShootingAngle.z = Angle.z;
							ShootingFlag = TRUE;
							ShootingCount = 0;
							dwActionItemCode = sinWS1;
						}
						else {
							ResetAttackTrans();		//AttackTrans.code = 0;
						}
					}
					break;

				case snCHAR_SOUND_TEMPLEGUARD:
					if ( MotionInfo->KeyCode!='I' ) {
						AssaParticle_MonsterIronMorgonHit(this);
					}
					else {
						GetMoveLocation( 0 , 0, 54*fONE , 0, Angle.y , 0 );
						pos.x = pX+GeoResult_X;
						pos.y = pY+GeoResult_Y+32*fONE;
						pos.z = pZ+GeoResult_Z;
						AssaParticle_MonsterIronHit1( &pos );
					}
					break;

				case snCHAR_SOUND_SETO:
					if ( chrAttackTarget ) {
						GetMoveLocation( 0 , 24*fONE, 24*fONE , 0, Angle.y , 0 );
						StartEffectMonsterDest( pX+GeoResult_X,pY+GeoResult_Y,pZ+GeoResult_Z,
							chrAttackTarget->pX , chrAttackTarget->pY+20*fONE , chrAttackTarget->pZ ,
							MONSTER_PIGON_SHOT1 );

						if ( ((dwPlayTime>>4)&1)==0 ) 
							SkillPlaySound( SKILL_SOUND_SKILL_FIREBALL1	 , pX,pY,pZ );
						else
							SkillPlaySound( SKILL_SOUND_SKILL_FIREBALL2	 , pX,pY,pZ );
					}
					break;

				case snCHAR_SOUND_KINGSPIDER:
					if (chrAttackTarget) {
						GetMoveLocation( 0 , 0, 70*fONE , 0, Angle.y , 0 );
						StartEffectMonster( pX+GeoResult_X, pY+42*fONE,pZ+GeoResult_Z, MONSTER_HULK_HIT1);
					}
					break;

				////////////////////////////////////////////////////

				case snCHAR_SOUND_NPC_MORIF:		//믹스장인 모리프
					if (chrAttackTarget) {

						GetMoveLocation( 0 , 0, 25*fONE , 0, Angle.y , 0 );
						pos.x = pX+GeoResult_X;
						pos.y = pY+GeoResult_Y+32*fONE;
						pos.z = pZ+GeoResult_Z;

						pos2.x = chrAttackTarget->pX;
						pos2.y = chrAttackTarget->pY+12*fONE;
						pos2.z = chrAttackTarget->pZ;

						AssaParticle_ShelltomShot( &pos, &pos2 , rand()%3 );
					}
					break;

				case snCHAR_SOUND_NPC_MOLLYWOLF:		//송편훔친늑대
					if (chrAttackTarget) {

						GetMoveLocation( 0 , 0, 25*fONE , 0, Angle.y , 0 );
						pos.x = pX+GeoResult_X;
						pos.y = pY+GeoResult_Y+32*fONE;
						pos.z = pZ+GeoResult_Z;

						pos2.x = chrAttackTarget->pX;
						pos2.y = chrAttackTarget->pY+12*fONE;
						pos2.z = chrAttackTarget->pZ;

						AssaParticle_BamShot( &pos, &pos2 );
					}
					break;

				case snCHAR_SOUND_WORLDCUP:				//월드컵 몬스터
					if (chrAttackTarget) {
						ParkAssaParticle_WorldCupMonsterAttack( this , chrAttackTarget );
					}
					break;

				case snCHAR_SOUND_S_F_ELEMENTAL:		//소환몹 화이어엘리멘탈
					if (chrAttackTarget) {
						GetMoveLocation( 0 , 0, 25*fONE , 0, Angle.y , 0 );
						pos.x = pX+GeoResult_X;
						pos.y = pY+GeoResult_Y+32*fONE;
						pos.z = pZ+GeoResult_Z;

						pos2.x = chrAttackTarget->pX;
						pos2.y = chrAttackTarget->pY+12*fONE;
						pos2.z = chrAttackTarget->pZ;

						SkillArchMageFireElementalShot(&pos, &pos2);
					}
					break;

				case snCHAR_SOUND_S_METALGOLEM:		//소환몹 메탈고렘
					if (chrAttackTarget) {
						if ( !smCharInfo.wPlayClass[1] )
							GetMoveLocation( 0 , 0, 54*fONE , 0, Angle.y , 0 );
						else
							GetMoveLocation( 0 , 0, 54*smCharInfo.wPlayClass[1] , 0, Angle.y , 0 );

						StartEffectMonster( pX+GeoResult_X, pY+48*fONE,pZ+GeoResult_Z, MONSTER_HULK_HIT1);
					}
					break;

				case snCHAR_SOUND_S_WOLVERLIN:		//소환몹 울버린
					if (chrAttackTarget) {
						if ( !smCharInfo.wPlayClass[1] )
							GetMoveLocation( 0,0,50*fONE , 0,Angle.y,0 );
						else
							GetMoveLocation( 0,0,50*smCharInfo.wPlayClass[1] , 0, Angle.y , 0 );

						x = pX+GeoResult_X;
						z = pZ+GeoResult_Z;
						y = pY+18*fONE;
 						StartEffectMonster(x,y,z, MONSTER_PIGON_PARTICLE1);
						StartEffectMonster(x,y,z, MONSTER_PIGON_PARTICLE1);
						StartEffectMonster(x,y,z, MONSTER_PIGON_PARTICLE1);
						StartEffectMonster(x,y,z, MONSTER_PIGON_PARTICLE1);
					}
					break;

				case snCHAR_SOUND_NPC_SKILLMASTER:
					if (chrAttackTarget && GetAttackPoint( &x, &y, &z )==TRUE ) {
						StartEffect( x,y,z , EFFECT_CRITICAL_HIT1 );
						SetDynLight( x,y,z , 40, 0,0,0, 400 );
					}
					break;

				case snCHAR_SOUND_NPC_MAGICMASTER:
					if (chrAttackTarget ) {
						GetMoveLocation( 0 , 0, 25*fONE , 0, Angle.y , 0 );
						StartEffectMonsterDest( pX+GeoResult_X,pY+32*fONE,pZ+GeoResult_Z, 
							chrAttackTarget->pX , chrAttackTarget->pY+24*fONE , chrAttackTarget->pZ ,
							MONSTER_IMP_SHOT1 );
					}
					break;

				case snCHAR_SOUND_CASTLE_CRYSTAL_R:	//빨간 크리스탈 타워
					if (chrAttackTarget ) {
						y = GetRadian2D( pX,pZ , chrAttackTarget->pX, chrAttackTarget->pZ );
						GetMoveLocation( 0 , 0, 16*fONE , 0, y , 0 );
						pos.x = pX+GeoResult_X;
						pos.y = pY+GeoResult_Y+136*fONE;
						pos.z = pZ+GeoResult_Z;
						pos2.x = chrAttackTarget->pX;
						pos2.y = chrAttackTarget->pY+12*fONE;
						pos2.z = chrAttackTarget->pZ;
						AssaParticle_FireShot(&pos, &pos2);

						if ( (rand()%2)==0 ) 
							SkillPlaySound( SKILL_SOUND_SKILL_FIREBOLT1 , pos.x, pos.y, pos.z);			//스킬 효과음
						else
							SkillPlaySound( SKILL_SOUND_SKILL_FIREBOLT2 , pos.x, pos.y, pos.z);			//스킬 효과음

					}
					break;

				case snCHAR_SOUND_CASTLE_CRYSTAL_G:	//그린 크리스탈 타워
					if (chrAttackTarget ) {
						y = GetRadian2D( pX,pZ , chrAttackTarget->pX, chrAttackTarget->pZ );
						GetMoveLocation( 0 , 0, 16*fONE , 0, y , 0 );
						pos.x = pX+GeoResult_X;
						pos.y = pY+GeoResult_Y+136*fONE;
						pos.z = pZ+GeoResult_Z;
						pos2.x = chrAttackTarget->pX;
						pos2.y = chrAttackTarget->pY+12*fONE;
						pos2.z = chrAttackTarget->pZ;
						AssaParticle_LightShot(&pos, &pos2);

						if ( (rand()%2)==0 ) 
							SkillPlaySound( SKILL_SOUND_SKILL_ENCHANTWEAPON_MAGIC_LIGHTNING1 , pos.x, pos.y, pos.z);			//스킬 효과음
						else
							SkillPlaySound( SKILL_SOUND_SKILL_ENCHANTWEAPON_MAGIC_LIGHTNING2 , pos.x, pos.y, pos.z);			//스킬 효과음
					}
					break;

				case snCHAR_SOUND_CASTLE_CRYSTAL_B:	//파랑 크리스탈 타워
					if (chrAttackTarget ) {
						y = GetRadian2D( pX,pZ , chrAttackTarget->pX, chrAttackTarget->pZ );
						GetMoveLocation( 0 , 0, 16*fONE , 0, y , 0 );
						pos.x = pX+GeoResult_X;
						pos.y = pY+GeoResult_Y+136*fONE;
						pos.z = pZ+GeoResult_Z;
						pos2.x = chrAttackTarget->pX;
						pos2.y = chrAttackTarget->pY+12*fONE;
						pos2.z = chrAttackTarget->pZ;
						AssaParticle_IceShot(&pos, &pos2);

						if ( (rand()%2)==0 ) 
							SkillPlaySound( SKILL_SOUND_SKILL_ENCHANTWEAPON_MAGIC_ICE1 , pos.x, pos.y, pos.z);			//스킬 효과음
						else
							SkillPlaySound( SKILL_SOUND_SKILL_ENCHANTWEAPON_MAGIC_ICE2 , pos.x, pos.y, pos.z);			//스킬 효과음

					}
					break;

				case snCHAR_SOUND_CASTLE_SOLDER_A:
				case snCHAR_SOUND_CASTLE_SOLDER_B:
				case snCHAR_SOUND_CASTLE_SOLDER_C:
					if (chrAttackTarget && GetAttackPoint( &x, &y, &z )==TRUE ) {
						StartEffect( x,y,z , EFFECT_CRITICAL_HIT1 );
						SetDynLight( x,y,z , 40, 0,0,0, 400 );
					}
					break;
				case snCHAR_SOUND_REVIVED_ARCHER:
					if (chrAttackTarget)
					{
						if ( CheckShootingTest_LastAttackTime(chrAttackTarget) )
						{
							ShootingPosi.x = pX;
							ShootingPosi.y = pY+38*fONE;
							ShootingPosi.z = pZ;
							GetRadian3D( pX,pY+38*fONE,pZ, chrAttackTarget->pX,chrAttackTarget->pY+24*fONE,chrAttackTarget->pZ );
							ShootingAngle.x = GeoResult_X;
							ShootingAngle.y = GeoResult_Y;
							ShootingAngle.z = Angle.z;
							ShootingFlag = TRUE;
							ShootingCount = 0;
							dwActionItemCode = sinWS1;
						}
						else
						{
							ResetAttackTrans();		//AttackTrans.code = 0;
						}
					}
					break;

				}

				if ( this!=lpCurPlayer ) {
					PlayAttack();									//공격 적용
				}
				if ( smCharInfo.State!=smCHAR_STATE_USER && PkMode_CharState!=smCHAR_STATE_USER ) return TRUE;

				if ( !ShootingMode ) {
					if ( GetAttackPoint( &x,&y,&z )==TRUE ) {
						if ( WeaponEffect ) {
							pos.x = x;
							pos.y = y;
							pos.z = z;

							switch( WeaponEffect ) {
								case (sITEMINFO_LIGHTING+1):
									AssaParticle_LightHit1( &pos );
									SetDynLight( x,y,z , 0,0,100,0, 200 );
									if ( (rand()%2)==0 ) 
										SkillPlaySound( SKILL_SOUND_SKILL_ENCHANTWEAPON_LIGHTNING1	 , pX,pY,pZ );
									else
										SkillPlaySound( SKILL_SOUND_SKILL_ENCHANTWEAPON_LIGHTNING2	 , pX,pY,pZ );

									break;
								case (sITEMINFO_FIRE+1):
									AssaParticle_FireHit1( &pos );
									SetDynLight( x,y,z , 100, 0,0,0, 200 );
									if ( (rand()%2)==0 ) 
										SkillPlaySound( SKILL_SOUND_SKILL_ENCHANTWEAPON_FIRE1	 , pX,pY,pZ );
									else
										SkillPlaySound( SKILL_SOUND_SKILL_ENCHANTWEAPON_FIRE2	 , pX,pY,pZ );
									break;
								case (sITEMINFO_ICE+1):
									AssaParticle_IceHit1( &pos );
									SetDynLight( x,y,z , 0,0,100,0, 200 );
									if ( (rand()%2)==0 ) 
										SkillPlaySound( SKILL_SOUND_SKILL_ENCHANTWEAPON_ICE1	 , pX,pY,pZ );
									else
										SkillPlaySound( SKILL_SOUND_SKILL_ENCHANTWEAPON_ICE2	 , pX,pY,pZ );
									break;
							}
						}
						else {
							if ( dwActionItemCode ) {		//HvRightHand.PatTool ) {
								// 도구 공격
								if ( AttackCritcal==TRUE || AttackEffect ) {
									StartEffect( x,y,z , EFFECT_CRITICAL_HIT1 );
									SetDynLight( x,y,z , 100, 0,0,0, 400 );
								}
								else {
									StartEffect( x,y,z , EFFECT_NORMAL_HIT1 );
									SetDynLight( x,y,z , 40, 0,0,0, 400 );
								}

							}
							else {
								StartEffect( x,y,z , EFFECT_NORMAL_HIT1 );
							}

							if ( dwForceOrbTime>dwPlayTime ) {		//포스오브 공격 이펙트
								pos.x = x;
								pos.y = y;
								pos.z = z;

								AssaParticle_ShelltomWeaponHit(&pos);
							}
						}
					}
				}
				else {
					//발사형 공격
					wp = (dwActionItemCode&sinITEM_MASK2);
					if ( !shootFlag && chrAttackTarget ) {
						GetAttackPoint( &x,&y,&z );

						if ( wp==sinWS1 || wp==sinWT1 ) {


							if ( smCharInfo.JOB_CODE==JOBCODE_ATALANTA &&
								(dwDispAppSkill&SKILL_APPMASK_GODLY_SHIELD) && wp==sinWT1 ) {
									//프로스트 제블린 스킬 이펙트 (아탈란타)
									pos.x = x;
									pos.y = y;
									pos.z = z;
									pos2.x = chrAttackTarget->pX;
									pos2.y = chrAttackTarget->pY+24*fONE;
									pos2.z = chrAttackTarget->pZ;
									SkillFrostJavelin( &pos, &pos2 );

									if ( (rand()%2)==0 ) 
										SkillPlaySound( SKILL_SOUND_SKILL_ENCHANTWEAPON_ICE1 , pX,pY,pZ );			//스킬 효과음
									else
										SkillPlaySound( SKILL_SOUND_SKILL_ENCHANTWEAPON_ICE2 , pX,pY,pZ );			//스킬 효과음

									return TRUE;
								}

							ShootingPosi.x = x;
							ShootingPosi.y = y;
							ShootingPosi.z = z;

							if (chrAttackTarget) {
								GetRadian3D( x,y,z, chrAttackTarget->pX,chrAttackTarget->pY+24*fONE,chrAttackTarget->pZ );
								ShootingAngle.x = GeoResult_X;
								ShootingAngle.y = GeoResult_Y;
								ShootingAngle.z = Angle.z;
							}
							else {
								ShootingAngle.x = Angle.x;
								ShootingAngle.y = Angle.y;
								ShootingAngle.z = Angle.z;
							}

							ShootingCount = 0;
							ShootingFlag = TRUE;

							if ( AttackSkil )
								ShootingKind = AttackSkil&0xFF;
							else
								ShootingKind = TRUE;

						}

						if ( wp==sinWM1 ) {
							//마법 지팡이

							if ( WeaponEffect ) {
								switch( WeaponEffect ) {
									case (sITEMINFO_LIGHTING+1):
										pos.x = x;
										pos.y = y;
										pos.z = z;
										pos2.x = chrAttackTarget->pX;
										pos2.y = chrAttackTarget->pY+24*fONE;
										pos2.z = chrAttackTarget->pZ;
										AssaParticle_LightShot( &pos, &pos2 );
										if ( (rand()%2)==0 ) 
											SkillPlaySound( SKILL_SOUND_SKILL_ENCHANTWEAPON_MAGIC_LIGHTNING1 , pX,pY,pZ );			//스킬 효과음
										else
											SkillPlaySound( SKILL_SOUND_SKILL_ENCHANTWEAPON_MAGIC_LIGHTNING2 , pX,pY,pZ );			//스킬 효과음
										break;
									case (sITEMINFO_FIRE+1):
										pos.x = x;
										pos.y = y;
										pos.z = z;
										pos2.x = chrAttackTarget->pX;
										pos2.y = chrAttackTarget->pY+24*fONE;
										pos2.z = chrAttackTarget->pZ;
										AssaParticle_FireShot( &pos, &pos2 );
										if ( (rand()%2)==0 ) 
											SkillPlaySound( SKILL_SOUND_SKILL_FIREBOLT1 , pX,pY,pZ );			//스킬 효과음
										else
											SkillPlaySound( SKILL_SOUND_SKILL_FIREBOLT2 , pX,pY,pZ );			//스킬 효과음
										break;
									case (sITEMINFO_ICE+1):
										pos.x = x;
										pos.y = y;
										pos.z = z;
										pos2.x = chrAttackTarget->pX;
										pos2.y = chrAttackTarget->pY+24*fONE;
										pos2.z = chrAttackTarget->pZ;
										AssaParticle_IceShot( &pos, &pos2 );
										if ( (rand()%2)==0 ) 
											SkillPlaySound( SKILL_SOUND_SKILL_ENCHANTWEAPON_MAGIC_ICE1 , pX,pY,pZ );			//스킬 효과음
										else
											SkillPlaySound( SKILL_SOUND_SKILL_ENCHANTWEAPON_MAGIC_ICE2 , pX,pY,pZ );			//스킬 효과음
										break;
								}
							}
							else {
								//임프 화이어볼
								StartEffectMonsterDest( x,y,z, 
									chrAttackTarget->pX , chrAttackTarget->pY+24*fONE , chrAttackTarget->pZ ,
									MONSTER_IMP_SHOT1 );
							}
						}

					}
				}
			}
	}


	return TRUE;

}

extern int MatLight[2];

//손에 효과 주기 ( 파이곤 용 )
int smCHAR::StartHandEffect( int Mode )
{
	POINT3D angle;
	POINT3D posi;
	smMATRIX *mWorld;
	int x,y,z;

		if ( smCharInfo.dwCharSoundCode==snCHAR_SOUND_FIGON ) {

			memcpy( &angle, &Angle , sizeof( POINT3D ) );
			angle.y = (-angle.y+ANGLE_180)&ANGCLIP;

			AnimObjectTree( HvLeftHand.ObjBip , frame , angle.x , angle.y , angle.z );
			mWorld = &HvLeftHand.ObjBip->mWorld;
			x = pX + mWorld->_41;
			z = pZ + mWorld->_42;
			y = pY + mWorld->_43;
			StartEffectMonster( x,y,z, MONSTER_PIGON_POWER1 );


			AnimObjectTree( HvRightHand.ObjBip , frame , angle.x , angle.y , angle.z );
			mWorld = &HvRightHand.ObjBip->mWorld;
			x = pX + mWorld->_41;
			z = pZ + mWorld->_42;
			y = pY + mWorld->_43;
			StartEffectMonster( x,y,z, MONSTER_PIGON_POWER1 );

			//파이곤 손에 불 붙이기
			//StartEffectMonster( 
			//HvLeftHand.ObjBip = AnimPat->GetObjectFromName( "Bip01 L Hand" );
			//HvRightHand.ObjBip = AnimPat->GetObjectFromName( "Bip01 R Hand" );
		}
		if ( smCharInfo.dwCharSoundCode==snCHAR_SOUND_SEN ) {
			if ( MotionInfo->State!=CHRMOTION_STATE_DEAD ) {
				memcpy( &angle, &Angle , sizeof( POINT3D ) );
				angle.y = (-angle.y+ANGLE_180)&ANGCLIP;

				AnimObjectTree( HvLeftHand.ObjBip , frame , angle.x , angle.y , angle.z );
				mWorld = &HvLeftHand.ObjBip->mWorld;
				x = pX + mWorld->_41;
				z = pZ + mWorld->_42;
				y = pY + mWorld->_43;

				posi.x = x;
				posi.y = y;
				posi.z = z;
				sinEffect_Sen( &posi );
				//임시로 없앴음
				//StartEffectMonster( x,y,z, MONSTER_PIGON_POWER1 );
			}
		}

		if ( smCharInfo.dwCharSoundCode==snCHAR_SOUND_MUTANTTREE ) {
			if ( MotionInfo->State!=CHRMOTION_STATE_DEAD ) {
				memcpy( &angle, &Angle , sizeof( POINT3D ) );
				angle.y = (-angle.y+ANGLE_180)&ANGCLIP;

				AnimObjectTree( HvLeftHand.ObjBip , frame , angle.x , angle.y , angle.z );
				mWorld = &HvLeftHand.ObjBip->mWorld;
				x = pX + mWorld->_41;
				z = pZ + mWorld->_42;
				y = pY + mWorld->_43;

				posi.x = x;
				posi.y = y+5*fONE;
				posi.z = z;
				sinEffect_MutentTree_Smoke( &posi ); 
			}
		}

		if ( smCharInfo.dwCharSoundCode==snCHAR_SOUND_RATOO ) {
			if ( MotionInfo->State!=CHRMOTION_STATE_DEAD ) {
				sinEffect_Ratoo_Aura2(this);
			}
		}
/*
		if ( smCharInfo.dwCharSoundCode==snCHAR_SOUND_SHADOW ) {

			memcpy( &angle, &Angle , sizeof( POINT3D ) );
			angle.y = (-angle.y+ANGLE_180)&ANGCLIP;

			AnimObjectTree( HvLeftHand.ObjBip , frame , angle.x , angle.y , angle.z );
			mWorld = &HvLeftHand.ObjBip->mWorld;
			x = pX + mWorld->_41;
			z = pZ + mWorld->_42;
			y = pY + mWorld->_43;

			posi.x = x;
			posi.y = y+5*fONE;
			posi.z = z;
			sinEffect_MutentTree_Smoke( &posi ); 

			//StartEffectMonster( x,y,z, MONSTER_PIGON_POWER2 );


			AnimObjectTree( HvRightHand.ObjBip , frame , angle.x , angle.y , angle.z );
			mWorld = &HvRightHand.ObjBip->mWorld;
			x = pX + mWorld->_41;
			z = pZ + mWorld->_42;
			y = pY + mWorld->_43;

			posi.x = x;
			posi.y = y+5*fONE;
			posi.z = z;
			sinEffect_MutentTree_Smoke( &posi ); 

			//StartEffectMonster( x,y,z, MONSTER_PIGON_POWER2 );

		}
*/
		
		return TRUE;

}

#define SERVER_CHAR_FRAME_STEP			320
#define SERVER_CHAR_SKIP				4

int smCHAR::Main()
{

	int x,z,y;
	int keycode;
	int cnt;
	DWORD compFrame;
	smPLAYBUFF	*playBuff;
	int OldAction;
	int dist;
//	int FallMode;
	POINT3D OldPos;
//	int step1,step2;
	int	frameStep , moveStep;
	smTRANS_COMMAND	smTransCommand;
	smCHAR *lpChar;
	int EvtFrame;
	int dm1,dm2;
	int UserSlowSpeed=128;



	if ( !Pattern || PatLoading || !MotionInfo ) {
		//PlayBuffCnt++;
		return FALSE;
	}

	//모델데이타 삭제된 오류
	if ( lpDinaPattern->UseCount<=0 ) {
		Close();
		if ( smConfig.DebugMode ) AddChatBuff( "\270\360\265\250\301\246\260\305 \277\300\267\371" );
		return FALSE;
	}


	switch( ActionPattern ) {
	case 0:
		if ( MotionInfo->State && MotionInfo->State<0x100 ) {

			//무기바꾸기 예약 실행
			if ( sinChangeSetFlag ) {
				cInvenTory.ChangeABItem( sinChangeSetFlag );
				sinChangeSetFlag = 0;
			}

			if ( sinGetLife()==0 ) {
				//생명력 보정
				sinSetLife( smCharInfo.Life[1]/2 );
				ResetEnergyGraph( 0 );						//에너지 그래프 첵크 초기화

			}

			if ( PlaySlowCount ) {
				if ( PlaySlowSpeed ) {
					UserSlowSpeed = PlaySlowSpeed;
				}
				else
					UserSlowSpeed = 128;

				if ( PlaySlowSpeed>200 || PlaySlowSpeed<0 ) UserSlowSpeed = 128;
			}

			if ( MoveFlag==TRUE ) {

				cnt = smCharInfo.Move_Speed-1;
				if ( cnt<0 ) cnt=0;
				if ( cnt>8 ) cnt=8;
				// 박재원 - 모코바 부츠 속도 적용
				// 박재원 - 스피드 부츠 속도 적용
				// 박재원 - 앞으로 추가될 속도 향상 빌링 부츠는 여기에 추가한다.
				if( cInvenTory.InvenItem[ sInven[9].ItemIndex-1 ].sItemInfo.CODE == (sinDB1|sin30) ||
					cInvenTory.InvenItem[ sInven[9].ItemIndex-1 ].sItemInfo.CODE == (sinDB1|sin31) || // 박재원 - 스피드 부츠 추가(7일)
					cInvenTory.InvenItem[ sInven[9].ItemIndex-1 ].sItemInfo.CODE == (sinDB1|sin32) ||  // 박재원 - 스피드 부츠 추가(30일)
					cInvenTory.InvenItem[ sInven[9].ItemIndex-1 ].sItemInfo.CODE == (sinDB1|sin33) || // 장별 - 스피드 부츠(1일)
					cInvenTory.InvenItem[ sInven[9].ItemIndex-1 ].sItemInfo.CODE == (sinDB1|sin34) )	// 장별 - 스피드 부츠(1시간)
				{
					if( DeleteEventItem_TimeOut(&cInvenTory.InvenItem[ sInven[9].ItemIndex-1 ].sItemInfo)==TRUE  ) // 사용기간이 만료될 경우
						cnt = 1;
					else
						cnt = smCharInfo.Move_Speed;
				}
				MoveSpeed = 250+10*cnt;

				if ( PlaySlowCount )		//느려지는 모드
					MoveSpeed=(MoveSpeed*UserSlowSpeed)>>8;			//70%로 느리게


				if ( MoveCnt>0 ) {
					MoveCnt--;
					if ( MoveCnt<=0 ) 
						MoveFlag=FALSE;
				}

				OldPos.x = pX;
				OldPos.y = pY;
				OldPos.z = pZ;

				if ( !ShootingFlag ) chrAttackTarget = 0;

				if ( !smConfig.DebugMode && !Pattern2 ) MoveMode = 0;			//머리없는 캐릭은 뛰지 못함
				if ( smCharInfo.Weight[0]>smCharInfo.Weight[1] ) MoveMode = 0;	//무계초과 캐릭터 뛰지 못함

				if ( MoveMode==0 || ShootingFlag || smCharInfo.Stamina[0]==0 ) {
					//걷기
					if ( MotionInfo->State!=CHRMOTION_STATE_FALLDOWN && 
						MotionInfo->State!=CHRMOTION_STATE_WALK ) 
						SetMotionFromCode( CHRMOTION_STATE_WALK );

					if ( OnStageFieldState!=FIELD_STATE_VILLAGE )
						MoveAngle2( (MoveSpeed*180)>>8 );				//필드에서 걷기
					else
						MoveAngle2( MoveSpeed );				//마을에서 걷기

					dwLastCharMoveTime = dwPlayTime;
				}
				else {
					//뛰기
					if ( MotionInfo->State!=CHRMOTION_STATE_FALLDOWN && MotionInfo->State!=CHRMOTION_STATE_RUN )
							SetMotionFromCode( CHRMOTION_STATE_RUN );

#ifdef _WINMODE_DEBUG
					if ( smConfig.DebugMode && VRKeyBuff[ VK_SHIFT ] )
						MoveAngle2( 3000 );
					else
#endif
						MoveAngle2( (MoveSpeed*460)>>8 );

					dwLastCharMoveTime = dwPlayTime;

					//달릴때 생기는 먼지
					if ( (Counter&0x7)==0 && WeatherMode<2) {
						if ( OnStageField>=0 && StageField[OnStageField]->FieldCode!=rsSOD_FIELD &&
							(StageField[OnStageField]->State==FIELD_STATE_RUIN || 
							StageField[OnStageField]->State==FIELD_STATE_DESERT) ) {

							if ( (Counter&8)==0 )
								GetMoveLocation( 4*fONE , 0, 0 , 0, Angle.y , 0 );
							else
								GetMoveLocation( -4*fONE , 0, 0 , 0, Angle.y , 0 );

							cnt = 10+( (dwPlayTime>>2)&0xF );
							StartEffect( pX+GeoResult_X,pY+12*fONE,pZ+GeoResult_Z , cnt,cnt , EFFECT_DUST1 );
						}
					}
				}


				if ( OverLapPosi || pX!=OldPos.x || pZ!=OldPos.z ) {
					lpChar = CheckOtherPlayPosi( pX , pY, pZ );
					if ( lpChar ) {
						if ( !OverLapPosi ) {
							pX = OldPos.x;
							pZ = OldPos.z;
							if ( ActionGameMode ) {
								if ( lpChar->smCharInfo.State==smCHAR_STATE_USER ) {
									if ( chrEachMaster!=lpChar ) {
										if (chrEachMaster) CloseEachPlayer();
										OpenEachPlayer( lpChar );
									}
									if ( chrEachMaster )
										EachTradeButton = TRUE;

								}
							}
						}

						if ( CheckOtherPlayPosi( pX , pY, pZ ) ) {
							OverLapPosi = TRUE;
							OverLapPosi2 = FALSE;
						}
						else {
							OverLapPosi = FALSE;
							OverLapPosi2 = TRUE;
						}

					}
					else {
						OverLapPosi = FALSE;
						OverLapPosi2 = FALSE;
					}
				}

				if ( smCharInfo.wPlayerKilling[0]>0 ) {
					//감옥 좌표 고정
					if ( pX<PrisonRect.left || pX>PrisonRect.right || pZ<PrisonRect.top || pZ>PrisonRect.bottom ) {
						pX = OldPos.x;
						pY = OldPos.y;
						pZ = OldPos.z;
					}
				}
/*
				if ( OnStageField>=0 && StageField[OnStageField]->FieldEvent==FIELD_EVENT_NIGHTMARE ) {
					//페허마을 하드몬스터 이벤트 지역 막음
					if ( rsHardCoreEventPosY<pY && rsHardCoreEventInterLineZ>pZ && rsHardCoreEventOutLineZ<pZ ) {
						pX = OldPos.x;
						pY = OldPos.y;
						pZ = OldPos.z;
					}

				}
*/
				if ( OnStageField>=0 && StageField[OnStageField]->FieldCode==rsCASTLE_FIELD ) {
					//블래스 캐슬 성문 입구
					if ( pZ<CastleBattleZone_DoorLineZ+16*fONE && pZ>CastleBattleZone_DoorLineZ-48*fONE &&
						pX>CastleBattleZone_DoorLineX[0] && pX<CastleBattleZone_DoorLineX[1] ) 
					{
						if ( rsBlessCastle.CastleMode && rsBlessCastle.DefenceLevel==0 ) {
							pX = OldPos.x;
							pY = OldPos.y;
							pZ = OldPos.z;
						}
					}
					else {
						//클랜원 외에 출입 금지
						//if ( !smConfig.DebugMode && pZ<CastleBattleZone_BridgeZ &&  ( !rsBlessCastle.CastleMode || rsBlessCastle.DefenceLevel ) ) {
						if ( !smConfig.DebugMode && pZ<CastleBattleZone_BridgeZ && !rsBlessCastle.CastleMode ) {
							if ( !smCharInfo.ClassClan || GetClanCode(smCharInfo.ClassClan)!=rsBlessCastle.dwMasterClan ) {
								pX = OldPos.x;
								pY = OldPos.y;
								pZ = OldPos.z;
							}
						}
					}
				}

				if ( pX==OldPos.x && pY==OldPos.y && pZ==OldPos.z ) {
					if ( TraceMode_DblClick ) {
						MoveFlag = 0;
						TraceMode_DblClick = 0;
					}
				}
/*
int CastleBattleZone_LineZ = -24043*fONE;
int	CastleBattleZone_DoorLineZ = -25545*fONE;								//성문 입구
int	CastleBattleZone_DoorLineX[2] = { 34407*fONE , 34695*fONE };			//성문 입구
int	CastleBattleZone_BridgeZ = -25258*fONE;									//다리 입구
int CastleSoulFountain_Area[4] = {	37040*fONE , 37158*fONE , -30286*fONE, -30102*fONE };	//영혼의샘터
*/

				// 박재원 - 모코바 부츠 속도 적용 후 FrameStep(스텝스피드) 재조정
				int limit_FrameStep = 0;
				if(MoveSpeed>350)
				{
					limit_FrameStep = 350;
					FrameStep = (80*limit_FrameStep)/300;
				}
				else
					FrameStep = (80*MoveSpeed)/300;

			}
			else {
				if ( MotionInfo->State!=CHRMOTION_STATE_FALLDOWN && MotionInfo->State!=CHRMOTION_STATE_STAND ) SetMotionFromCode( CHRMOTION_STATE_STAND );
				if ( PHeight<pY ) MoveAngle2(0);

				FrameStep = 80;

				if ( FrameCounter==0 ) {
					//캐릭터 끼리 끼여서 못움직이는 거 방지 ( 섰다 다시 가면 지나가도록 배려 )
					if ( CheckOtherPlayPosi( pX , pY, pZ ) ) {
						OverLapPosi = TRUE;
						OverLapPosi2 = FALSE;
					}
				}

			}


			//######################################################################################
			//작 성 자 : 오 영 석
			if( smConfig.DebugMode )
			{
				if( VRKeyBuff[VK_SHIFT] && VRKeyBuff[VK_CONTROL] && TalkPattern )
				{
					if( VRKeyBuff[ VK_F1 ] )
						Debug_TalkZoomMode = TRUE;
					else if( VRKeyBuff[ VK_F2 ] )
						Debug_TalkZoomMode = FALSE;
					else if( VRKeyBuff[ VK_F3 ] )
					{
						Debug_TalkRepeatMode = TRUE;
						Debug_TalkRepeatModeType = TALK_MOTION_FILE;
					}
					else if( VRKeyBuff[ VK_F4 ] )
					{
                        Debug_TalkRepeatMode = FALSE;
						Debug_TalkRepeatModeType = -1;
					}
					else if( VRKeyBuff[ VK_F5 ] )
					{
						Debug_TalkRepeatMode = TRUE;
						Debug_TalkRepeatModeType = FACIAL_MOTION_FILE;
					}
					else if( VRKeyBuff[ VK_F6 ] )
					{
						Debug_TalkRepeatMode = FALSE;
						Debug_TalkRepeatModeType = -1;
					}

					if( Debug_TalkRepeatMode == FALSE )
					{
						for( cnt=0; cnt < (int)lpTalkModelParent->TalkMotionCount; cnt++ )
						{
							keycode = lpTalkModelParent->TalkMotionInfo[ cnt ].KeyCode;
							if( keycode )
							{
								if( VRKeyBuff[ keycode ] )
								{
									lpTalkMotionInfo = &lpTalkModelParent->TalkMotionInfo[ cnt ];
									TalkFrame = lpTalkMotionInfo->StartFrame * 160;
									break;
								}
							}
						}
					}
				}
				else
				{
					for( cnt=0;cnt<(int)smMotionInfo->MotionCount;cnt++)
					{
						keycode = smMotionInfo->MotionInfo[cnt].KeyCode;
						if( keycode && keycode>='A' )
						{
							if ( VRKeyBuff[ keycode ] )
							{
								ChangeMotion( cnt );
								break;
							}
						}
					}					
				}
			}
			//######################################################################################			
		}

		if ( MotionInfo->State==CHRMOTION_STATE_ATTACK || MotionInfo->State==CHRMOTION_STATE_SKILL ) {
			if ( !MotionLoop ) {
				FrameStep = (80*AttackSpeed)>>FLOATNS;

				//프리스티스 공격속도 보정 ( 양손무기 공격동작 프레임이 적어서 느리게 설정 )
				if ( smCharInfo.JOB_CODE==8 && 
					MotionInfo->ItemCodeCount && sItem[MotionInfo->ItemCodeList[0]].CODE==(sinWM1|sin04) ) {
						FrameStep -= 16;
					}
			}
			else
				FrameStep = MotionLoopSpeed;

			if ( PlaySlowCount )		//느려지는 모드
				FrameStep=(FrameStep*UserSlowSpeed)>>8;
		}
		else {
			if ( MotionInfo->State==CHRMOTION_STATE_DAMAGE || MotionInfo->State==CHRMOTION_STATE_EAT ) {
				FrameStep = 92;		//15%	

				if ( PlaySlowCount )		//느려지는 모드
					FrameStep=(FrameStep*UserSlowSpeed)>>8;
			}
		}

		if ( PlaySlowCount>0 ) {
			PlaySlowCount--;
			if ( PlaySlowCount==0 && AttackIce!=0 ) {
				AttackIce = -AttackIce;
			}
		}
		if ( PlayDistortion>0 ) PlayDistortion--;
		if ( PlayVanish>0 ) {
			PlayVanish--;			//배니쉬
			if ( PlayVanish<=0 && dwM_BlurTime ) dwM_BlurTime = dwPlayTime;
		}
		if ( PlayStunCount>0 )		//중립 상태 유지 모드로 사용 (로어 공격 해당 )
			PlayStunCount--;

		if ( AttackIce!=0 ) {
			//얼음 공격
			if ( AttackIce>0 && AttackIce<160 ) AttackIce ++;
			if ( AttackIce<0 ) AttackIce++;
			if ( MotionInfo->State==CHRMOTION_STATE_DEAD ) AttackIce = 0;
		}

		if ( PlayInvincible>0 )	PlayInvincible--;		//무적 아이템 사용중

		////////////////// 독 관련 처리 //////////////////////
		if ( PlayPoison[1]>0 ) {
			if ( DispPoison<=0 ) DispPoison = 1;
			PlayPoison[1]--;

			if ( PlayPoison[0] && (PlayPoison[1]&1)==0 && MotionInfo->State!=CHRMOTION_STATE_DEAD )  {
				sinSetLife( sinGetLife()-PlayPoison[0] );
				if ( sinGetLife()<=0 ) {
					sinSetLife( 1 );
				}
			}
		}
		else {
			if ( DispPoison>0 ) DispPoison = -DispPoison;
		}

		if ( DispPoison!=0 ) {
			//독에 감염
			if ( DispPoison>0 && DispPoison<100 ) DispPoison ++;
			if ( DispPoison<0 ) DispPoison++;
			if ( MotionInfo->State==CHRMOTION_STATE_DEAD ) DispPoison = 0;
		}
		//////////////////////////////////////////////////////

		//######################################################################################
		//작 성 자 : 오 영 석
		if( TalkFrame )
		{
			if( TalkFrame >= lpTalkMotionInfo->EndFrame * 160 )
				TalkFrame = 0;
			else
				TalkFrame += 80;
		}
		
		if( smConfig.DebugMode )
		{
			if( Debug_TalkRepeatMode == TRUE && TalkFrame == 0 )
			{
				int TalkCnt = 0;
				if( lpTalkModelParent->TalkMotionRate[ CHRMOTION_EXT ] )
				{
					TalkCnt = rand() % 100;
					TalkCnt = lpTalkModelParent->TalkMotionRateCnt[ Debug_TalkRepeatModeType ][ TalkCnt ];
				}
				else
					TalkCnt = rand() % lpTalkModelParent->TalkMotionCount;

				lpTalkMotionInfo = &lpTalkModelParent->TalkMotionInfo[ TalkCnt ];
				TalkFrame = lpTalkMotionInfo->StartFrame * 160;
			}
		}
		//######################################################################################


		frame += FrameStep;

		EventAttack();			//공격 이벤트

		//임시로 소리 삽입
		if( MoveFlag==TRUE ) {
			//######################################################################################
			//작 성 자 : 오 영 석
			int IsStateIce = 0;
			//######################################################################################

			if ( OnStageField>=0 ) {
				if ( StageField[OnStageField]->FieldCode!=0 && StageField[OnStageField]->FieldCode!=1 &&
					StageField[OnStageField]->FieldCode!=19 && StageField[OnStageField]->FieldCode!=18 &&
					StageField[OnStageField]->FieldCode!=24 && StageField[OnStageField]->FieldCode!=26 &&
					StageField[OnStageField]->FieldCode!=30 && StageField[OnStageField]->FieldCode!=33 &&
					StageField[OnStageField]->FieldCode!=34 )

					WaterHeight=CLIP_OUT;		//물이 없는 지역

				//######################################################################################
				//작 성 자 : 오 영 석
				if( lpCurPlayer == this && StageField[OnStageField]->State == FIELD_STATE_ICE )
                    IsStateIce = 1;
				//######################################################################################

			}


			if ( MotionInfo->EventFrame[0] ) {
				compFrame = frame-(MotionInfo->StartFrame*160);
				if ( MotionInfo->EventFrame[0]>(compFrame-FrameStep) && MotionInfo->EventFrame[0]<=compFrame ) {
					PlayFootStep( this );

					if ( WaterHeight!=CLIP_OUT && WaterHeight>pY && WaterHeight<(pY+32*fONE) && OnStageField>=0 ) {
						GetMoveLocation( 0 , 0, 8*fONE , 0, Angle.y , 0 );
						StartTerrainEffect( pX+GeoResult_X, WaterHeight+128, pZ+GeoResult_Z, EFFECT_TERRAIN_WAVE);
					}

					//######################################################################################
					//작 성 자 : 오 영 석					
					else if( IsStateIce )
					{
						GetMoveLocation( 3*fONE, 0, 5*fONE, 0, Angle.y, 0 );
						SetIceFoot( 0, dwPlayTime, GeoResult_X, GeoResult_Z );
					}
					//######################################################################################
				}

			}

			if ( MotionInfo->EventFrame[1] ) {
				compFrame = frame-(MotionInfo->StartFrame*160);
				if ( MotionInfo->EventFrame[1]>(compFrame-FrameStep) && MotionInfo->EventFrame[1]<=compFrame ) {
					PlayFootStep( this );

					if ( WaterHeight!=CLIP_OUT && WaterHeight>pY && WaterHeight<(pY+32*fONE) && OnStageField>=0 ) { 
						GetMoveLocation( 0 , 0, 8*fONE , 0, Angle.y , 0 );
						StartTerrainEffect( pX+GeoResult_X, WaterHeight+128, pZ+GeoResult_Z, EFFECT_TERRAIN_WAVE);
					}

					//######################################################################################
					//작 성 자 : 오 영 석					
					else if( IsStateIce )
					{
						GetMoveLocation( -3*fONE, 0, 5*fONE, 0, Angle.y, 0 );
						SetIceFoot( 1, dwPlayTime, GeoResult_X, GeoResult_Z );
					}
					//######################################################################################
				}
			}
		}


/*
UseEtherCoreCode
#define START_FIELD_NUM			3
#define START_FIELD_NEBISCO		9
//필드 시작 ( 귀환용 )
int WarpStartField( int FieldNum , int *mx, int *mz )
*/

		if ( MotionInfo->State==CHRMOTION_STATE_EAT ) {
			//물약 먹기 동작
			if ( MotionInfo->EventFrame[0] ) {
				compFrame = frame-(MotionInfo->StartFrame*160);
				if ( MotionInfo->EventFrame[0]>(compFrame-FrameStep) && MotionInfo->EventFrame[0]<=compFrame ) {
					//GetAttackPoint( &x,&y,&z );
					if ( UseEtherCoreCode ) {
						//귀환 아이템
						//StartEffect( pX,pY+48*fONE,pZ, EFFECT_RETURN1 );
						PotionLog = POTION_ETHERCORE;
					}
					else {
						if ( pUsePotion && pUsePotion->sItemInfo.CODE ) {
							//사용한 아이템 코드 서버로 보냄
							SendUseItemCodeToServer( &pUsePotion->sItemInfo );
						}

						switch( sinUsePotion() ) {
						case 1:
							StartEffect( pX,pY+48*fONE,pZ, EFFECT_POTION1 );
							PotionLog = 1;
							ResetEnergyGraph( 0 );
							break;
						case 2:
							StartEffect( pX,pY+48*fONE,pZ, EFFECT_POTION2 );
							PotionLog = 2;
							ResetEnergyGraph( 1 );
							break;
						case 3:
							StartEffect( pX,pY+48*fONE,pZ, EFFECT_POTION3 );
							PotionLog = 3;
							ResetEnergyGraph( 2 );
							break;
						case 4:
							StartEffect( pX,pY+48*fONE,pZ, EFFECT_POTION3 );
							StartEffect( pX,pY+48*fONE,pZ, EFFECT_POTION2 );
							StartEffect( pX,pY+48*fONE,pZ, EFFECT_POTION1 );
							PotionLog = 4;
							ResetEnergyGraph( 3 );

							for( cnt=0;cnt<OTHER_PLAYER_MAX;cnt++) {
								if ( chrOtherPlayer[cnt].Flag && chrOtherPlayer[cnt].dwObjectSerial && chrOtherPlayer[cnt].PartyFlag &&
									chrOtherPlayer[cnt].MotionInfo->State!=CHRMOTION_STATE_DEAD && chrOtherPlayer[cnt].RendSucess ) {

									x = chrOtherPlayer[cnt].pX;
									y = chrOtherPlayer[cnt].pY+48*fONE;
									z = chrOtherPlayer[cnt].pZ;

									StartEffect( x,y,z, EFFECT_POTION1 );
									StartEffect( x,y,z, EFFECT_POTION2 );
									StartEffect( x,y,z, EFFECT_POTION3 );
									
								}
							}
							break;
						case 5:
								//상한 음식 먹었다
								SetMotionFromCode( CHRMOTION_STATE_DAMAGE );		//아픈동작
								CharPlaySound( lpCurPlayer );
								break;
						case 6:	// pluto 선물상자
								//StartEffect( pX,pY+48*fONE,pZ, EFFECT_POTION1 );
								break;
						case 7:
								SetMotionFromCode(CHRMOTION_STATE_YAHOO); // 박재원 - 캡슐아이템 사용하면 야호동작(호랑이 캡슐 사용)
								break;
						}

					}
					SetDynLight( pX,pY+48*fONE,pZ, 50, 100,70,0, 250 , 1 );
				}
			}
		}
/*
		if ( sinChangeSetFlag ) {
			cInvenTory.ChangeABItem( sinChangeSetFlag );
			sinChangeSetFlag = 0;
		}
*/

		if ( frame>=MotionInfo->EndFrame*160 ) {
			//동작의 끝
/*
			if ( MotionInfo->State == CHRMOTION_STATE_DEAD ) {
				if ( FrameCounter>400 ) {
*/
			if ( MotionInfo->State == CHRMOTION_STATE_DEAD ) {
				if( g_IsDxProjectZoomIn )	// pluto 죽었을때 지도 못 열게 하려고
				{
					SetFullZoomMap(0);
				}
				if ( ReStartOptionIndex ) {

					if ( OnStageField>=0 && ReStartOptionIndex==RESTART_FEILD ) {
						//필드에서 재시작
						if ( StageField[OnStageField]->FieldCode==rsSOD_FIELD ) 
							WarpFieldNearPos( rsSOD_VILLAGE , pX, pZ , &x, &z );
						else 
							WarpFieldNearPos( StageField[OnStageField]->FieldCode , pX, pZ , &x, &z );

						if ( !PlayUsed_Resurrection ) {
							ContGameExp( this );		//부활주문서 없으면 경험치 하락
						}
						PlayUsed_Resurrection = 0;
						RestartPlayCount = 700;		//10초 동안 무적
					}

					if ( ReStartOptionIndex==RESTART_TOWN ) {
						//마을에서 재시작

						//공성전
						if ( OnStageField>=0 && StageField[OnStageField]->State==FIELD_STATE_CASTLE && 
							smCharInfo.ClassClan && GetClanCode(smCharInfo.ClassClan)==rsBlessCastle.dwMasterClan ) 
						{
							//워프 블레스 캐슬 필드
							WarpCastleField( 1 , &x, &z );
						}
						else
							WarpStartField( &x,&z );

						RestartPlayCount = 700;		//10초 동안 무적
						PlayUsed_Resurrection = 0;
					}

					if ( ReStartOptionIndex==RESTART_EXIT ) {
						//게임 종료
						CheckCharForm();
						//플레이어 죽음
						sinSetLife( 0 );
						ResetEnergyGraph( 0 );						//에너지 그래프 첵크 초기화

						quit = TRUE;
						SaveGameData();				//게임 저장하고 나가기
						QuitSave = TRUE;
						PlayUsed_Resurrection = 0;
					}
					else {
						SetPosi( x,0, z ,0,0,0 );
						TraceCameraPosi.x = pX;
						TraceCameraPosi.y = pY;
						TraceCameraPosi.z = pZ;
						TraceTargetPosi.x = pX;
						TraceTargetPosi.y = pY;
						TraceTargetPosi.z = pZ;


						if ( smCharInfo.State==smCHAR_STATE_ENEMY ) {
							//몬스터 시작 모션 없음
							SetMotionFromCode( CHRMOTION_STATE_STAND );		//서있기 동작
						}
						else {
							SetMotionFromCode( CHRMOTION_STATE_RESTART );		//시작모션
							StartEffect( pX,pY,pZ, EFFECT_GAME_START1 );
							SetDynLight( pX,pY,pZ, 100, 100,100,0, 200 );
							CharPlaySound( this );
						}
						dwBattleTime = 0;
						dwBattleQuitTime = 0;

						//바닥 확인
						OnStageField = -1;
						if ( smGameStage[0] ) {
							cnt = smGameStage[0]->GetFloorHeight( pX,pY,pZ ,Pattern->SizeHeight );	
							lpCurPlayer->OnStageField = 0;
						}
						if ( cnt==CLIP_OUT && smGameStage[1] ) {
							lpCurPlayer->OnStageField = 1;
						}
					}

					CheckCharForm();
					//플레이어 죽음
					sinSetLife( smCharInfo.Life[1]/2 );
					ResetEnergyGraph( 0 );						//에너지 그래프 첵크 초기화
					ReformCharForm();					//캐릭터 정보 인증
					MoveFlag = FALSE;

					ReStartOptionIndex = 0;
					ReStartFlag = 0;
					//cInterFace.ReleaseReStartImage();

				}
				else {
					frame = MotionInfo->EndFrame*160-80;
				}

				//죽었을때 상점 거래 안됨 취소 시킴
				if ( cTrade.OpenFlag && !dwTradeMaskTime ) {
					SendRequestTrade( cTrade.TradeCharCode , 3 );			//거래 취소 요구
					cTrade.CancelTradeItem();								//거래창 닫기
				}

				// pluto 부활주문서 있어도 마을로 가는 버그 FrameCounter== 이렇게 있던거 >= 요걸로 땜빵 ㅋㅋㅋ
				if ( FrameCounter>=70*10 && lpCurPlayer->OnStageField>=0 ) {
					//인벤토리의 지정한 아이템을 찾아 검색하여 찾는다
					sITEMINFO *lpItemInfo = 0;

					if ( StageField[ lpCurPlayer->OnStageField ]->State==FIELD_STATE_CASTLE ) 
						lpItemInfo = FindItemFromCode(sinBC1|sin05);

//					if ( !lpItemInfo ) 
//						lpItemInfo = FindItemFromCode(sinBI1|sin04);		//부활 주문서

					if ( lpItemInfo || PlayUsed_Resurrection ) {
						if ( lpItemInfo ) {
							SendUseItemCodeToServer( lpItemInfo );
							cInvenTory.DeleteInvenItemToServer(lpItemInfo->CODE  , lpItemInfo->ItemHeader.Head , lpItemInfo->ItemHeader.dwChkSum );
						}

						lpCurPlayer->SetMotionFromCode( CHRMOTION_STATE_RESTART );		//시작모션
						StartEffect( lpCurPlayer->pX,lpCurPlayer->pY,lpCurPlayer->pZ, EFFECT_GAME_START1 );
						SetDynLight( lpCurPlayer->pX,lpCurPlayer->pY,lpCurPlayer->pZ, 100, 100,100,0, 200 );
						CharPlaySound( lpCurPlayer );

						sinSetLife( lpCurPlayer->smCharInfo.Life[1]/2 );
						ResetEnergyGraph( 4 );								//에너지 그래프 첵크 초기화
						lpCurPlayer->MoveFlag = FALSE;
						RestartPlayCount = 350;								//5초 동안 무적
						ReStartFlag = 0;
						PlayUsed_Resurrection = 0;

					}
				}

			}
			else {

				if ( MotionLoop ) MotionLoop--;

				if ( MotionInfo->State==CHRMOTION_STATE_YAHOO )				//야호 동작
					SendClanYahooMotion();

				if ( MotionInfo->State==CHRMOTION_STATE_EAT ) {
					if ( UseEtherCoreCode ) {
						if ( OnStageField<0 || 
							( StageField[OnStageField]->FieldCode!=rsSOD_FIELD && StageField[OnStageField]->FieldCode!=QUEST_ARENA_FIELD ) ) {

							//필드 시작 ( 귀환용 )
							switch( UseEtherCoreCode ) {
							case (sinEC1|sin01):
								SaveGameData();
								WarpStartField( START_FIELD_NUM , &pX, &pZ );			//초보마을
								SetPosi( pX,0, pZ ,0,0,0 );
								break;
							case (sinEC1|sin02):
								SaveGameData();
								WarpStartField( START_FIELD_NEBISCO , &pX, &pZ );		//페허마을
								SetPosi( pX,0, pZ ,0,0,0 );
								break;
							case (sinEC1|sin04):
								SaveGameData();
								WarpStartField( START_FIELD_MORYON , &pX, &pZ );		//필리아마을
								SetPosi( pX,0, pZ ,0,0,0 );
								break;
							}

							TraceCameraPosi.x = pX;
							TraceCameraPosi.y = pY;
							TraceCameraPosi.z = pZ;
							TraceTargetPosi.x = pX;
							TraceTargetPosi.y = pY;
							TraceTargetPosi.z = pZ;

							StartEffect( pX,pY+48*fONE,pZ, EFFECT_RETURN1 );
							SkillPlaySound( SKILL_SOUND_LEARN , pX,pY,pZ );			//스킬 효과음
							RestartPlayCount = 700;		//10초 동안 무적
						}
						MoveFlag = 0;
						MouseButton[0]=0;
						dwNextMotionCode = 0;
						UseEtherCoreCode = 0;
					}
					else {
						//물약갯수 비교
						ComparePotion();
					}

				}

				if ( lpTransVirtualPotion ) {
					//가상 포션 실행
					x = (smCharInfo.Life[1]*40)/100; //총 HP의 40% 까지만 적용
					if ( lpTransVirtualPotion->WParam<x ) x=lpTransVirtualPotion->WParam;
					sinSetLife( sinGetLife()+x );

					sinSetMana( sinGetMana()+lpTransVirtualPotion->LParam );
					sinSetStamina( sinGetStamina()+lpTransVirtualPotion->SParam );
					ResetEnergyGraph( 3 );	//에너지 그래프 첵크 초기화

					//메모리 제거 ( 흔적을 없애자 )
					ZeroMemory(lpTransVirtualPotion,sizeof(smTRANS_COMMAND) );
					delete lpTransVirtualPotion;
					lpTransVirtualPotion = 0;
				}

				//무기바꾸기 예약 실행
				if ( sinChangeSetFlag && !MotionLoop ) {
					cInvenTory.ChangeABItem( sinChangeSetFlag );
					sinChangeSetFlag = 0;
				}

				if ( dwNextMotionCode && !MotionLoop ) {

					//다음 동작 예약 실행
					if ( SetMotionFromCode( dwNextMotionCode )==FALSE ) {
						SetMotionFromCode( CHRMOTION_STATE_STAND );
					}
					else {
						if ( UseEtherCoreCode && dwNextMotionCode==CHRMOTION_STATE_EAT ) {
							//귀환 아이템
							StartEffect( pX,pY+48*fONE,pZ, EFFECT_RETURN1 );
							SkillPlaySound( SKILL_SOUND_LEARN , pX,pY,pZ );			//스킬 효과음
						}
					}
					dwNextMotionCode = 0;
				}
				else {
					if ( MotionInfo->State == CHRMOTION_STATE_ATTACK || MotionInfo->State == CHRMOTION_STATE_SKILL ) {

						if ( MotionInfo->State==CHRMOTION_STATE_SKILL && AttackSkil )				//스킬동작 종료
							EndSkill();

						if ( AttackSkil && MotionLoop ) {
							frame = MotionInfo->StartFrame*160;
							//스킬 공격을 시도
							PlaySkillLoop( AttackSkil , this );
						}
						else {
							if ( !AttAutoCont && MouseButton[0]==0 && MouseButton[1]==0 && lpCharMsTrace && lpCharMsTrace==chrAttackTarget ) {
								//공격 취소
								if ( CancelAttack()==TRUE )
									MoveFlag = FALSE;
							}
							if ( MoveFlag && !MsTraceMode ) {
								Angle.y = GetPlayMouseAngle();
							}
						}
					}
					else
						if ( MotionInfo->State == CHRMOTION_STATE_EAT ) {
							if ( !AttAutoCont && MouseButton[0]==0 && MouseButton[1]==0 && lpCharMsTrace && lpCharMsTrace==chrAttackTarget ) {
								//공격 취소
								if ( CancelAttack()==TRUE )
									MoveFlag = FALSE;
							}
						}

					if ( MotionInfo->Repeat == TRUE ) {
						frame = (MotionInfo->StartFrame*160)+(frame - MotionInfo->EndFrame*160);
					}
					else {
						if ( !MotionLoop )	{//반복 모션
							if ( dwNextMotionCode ) {
								//다음 동작 예약 실행
								if ( SetMotionFromCode( dwNextMotionCode )==FALSE ) {
									SetMotionFromCode( CHRMOTION_STATE_STAND );
								}
								dwNextMotionCode = 0;
							}
							else {
								SetMotionFromCode( CHRMOTION_STATE_STAND );
							}
						}
					}
				}
			}

			////////////// 공격 속도 설정 ///////////////////////////////////
			AttackSpeed = GetAttackSpeedMainFrame( smCharInfo.Attack_Speed );

			if ( dwActionItemTwoHand )	
				AttackSpeed+=16;

			for(cnt=0;cnt<10;cnt++) {
				if ( ContinueSkill[cnt].Flag ) {
					if ( ContinueSkill[cnt].CODE==SKILL_SWIFT_AXE ) {
						if ( (dwActionItemCode&sinITEM_MASK2)==sinWA1 ) {
							//스위프트액스 공격속도 가중
							x = ContinueSkill[cnt].Point-1;
							if ( x>=0 && x<10 ) {
								AttackSpeed += (AttackSpeed*Swift_Axe_Speed[x])/100;
					//		if ( AttackSpeed>336 ) AttackSpeed=336;		//약 8.5	// pluto 공속 제한 해제 // 장별 - 스위프트액스 공속 제한 진짜 해제
							}
							break;
						}
					}
				}
			}
			//////////////////////////////////////////////////////////////////


			if ( !MotionLoop ) {
				//공격 이펙트 초기화
				AttackEffect = 0;
			}

			MotionEvent = 0;
		}

		if ( AttackAnger>0 )
			AttackAnger--;

		//무기 속성 이펙트
		if ( dwWeaponEffectTime ) {
			if ( dwWeaponEffectTime<dwPlayTime ) {
				dwWeaponEffectTime = 0;
				WeaponEffect = 0;
			}
		}

		wStickItems[ smPLAYBUFF_ITEM_SKIL ] = AttackSkil;


		if ( WaterHeight!=CLIP_OUT && WaterHeight>pY && WaterHeight<(pY+32*fONE) && OnStageField>=0 ) {

			dist = 0;
			cnt = 0x3F;
			switch( MotionInfo->State ) {
			case CHRMOTION_STATE_WALK:
				cnt = 0x1F;
				dist = 8;
				break;
			case CHRMOTION_STATE_RUN:
				dist = 14;
			case CHRMOTION_STATE_ATTACK:
			case CHRMOTION_STATE_SKILL:
				cnt = 0xF;
				break;				
			}

			if ( (Counter&cnt)==0 ) {
				GetMoveLocation( 0 , 0, dist<<FLOATNS , 0, Angle.y , 0 );
				StartTerrainEffect( pX+GeoResult_X, WaterHeight+128, pZ+GeoResult_Z, EFFECT_TERRAIN_WAVE);
/*
				GetMoveLocation( ((rand()%40)-20)<<FLOATNS , 0, (dist+((rand()%40)-20))<<FLOATNS , 0, Angle.y , 0 );
				StartTerrainEffect( pX+GeoResult_X, WaterHeight, pZ+GeoResult_Z, EFFECT_TERRAIN_WAVE);

				GetMoveLocation( ((rand()%40)-20)<<FLOATNS , 0, (dist+((rand()%40)-20))<<FLOATNS , 0, Angle.y , 0 );
				StartTerrainEffect( pX+GeoResult_X, WaterHeight, pZ+GeoResult_Z, EFFECT_TERRAIN_WAVE);
*/
			}
		}

		//PlayBuff에 동작 저장
		SavePlayBuff();
		break;

	case 1:	//펭귄
		break;

	case 5:	//몬스터

		if ( PlaySlowCount ) {
			if ( PlaySlowSpeed ) {
				moveStep = (SERVER_CHAR_SKIP*PlaySlowSpeed)>>8;
				frameStep = (SERVER_CHAR_FRAME_STEP*PlaySlowSpeed)>>8;
			}
			else {
				moveStep = SERVER_CHAR_SKIP>>1;
				frameStep = SERVER_CHAR_FRAME_STEP>>1;
			}
		}
		else {
			moveStep = SERVER_CHAR_SKIP;
			frameStep = SERVER_CHAR_FRAME_STEP;
		}

		if ( MotionInfo->State == CHRMOTION_STATE_WARP ) {
			//워프 동작
			frame += frameStep;

			if ( frame>MotionInfo->EndFrame*160 ) {
				Close();
				return TRUE;
			}
			SavePlayBuff2();
			FrameCounter+=3;
			break;
		}

		//뱀피릭 머신 움직이지 않도록
		//if ( smCharInfo.dwCharSoundCode==snCHAR_SOUND_VAMPRICMACHINE || smCharInfo.dwCharSoundCode==snCHAR_SOUND_CASTLE_DOOR )
		if ( DontMoveFlag )	moveStep = 0;


		//바벨 출현 유지중
		if ( smCharInfo.dwCharSoundCode==snCHAR_SOUND_BABEL ) {
			if ( smCharInfo.Life[0]>0 )	dwBabelPlayTime = dwPlayServTime+5000;
		}
/*
		if ( smCharInfo.dwCharSoundCode==snCHAR_SOUND_KELVEZU ) {
			if ( smCharInfo.Life[0]>0 )	dwKelvezuPlayTime = dwPlayServTime+5000;
		}
*/
		if ( MotionInfo->State && MotionInfo->State<0x100 && moveStep ) {
			if ( MotionInfo->State==CHRMOTION_STATE_WALK || MotionInfo->State==CHRMOTION_STATE_RUN ) {
				OldPos.x = pX;
				OldPos.y = pY;
				OldPos.z = pZ;

				if ( sMoveStepCount[0] ) {
					//이동 스텝 수동 설정
					if ( sMoveStepCount[1] ) moveStep *= sMoveStepCount[1];

					for(cnt=0;cnt<sMoveStepCount[0];cnt++) {
						MoveAngle( moveStep );
						if ( MotionInfo->State==CHRMOTION_STATE_RUN ) 
							MoveAngle( moveStep );
					}
				}
				else {

					if ( smCharInfo.wPlayClass[0]==MONSTER_CLASS_BOSS && smCharInfo.dwCharSoundCode==snCHAR_SOUND_DARKKNIGHT ) {
						//둠로드
						MoveAngle( moveStep*2 );
					}
					else
						MoveAngle( moveStep );

					if ( MotionInfo->State==CHRMOTION_STATE_RUN ) {
						if ( smCharInfo.dwCharSoundCode==snCHAR_SOUND_GREVEN ) {
							//공룡 달리기 무지 빨리
							if ( lpTargetPlayInfo && TragetTraceMode ) {
								SetTargetPosi2( lpTargetPlayInfo->Position.x>>FLOATNS , 
									lpTargetPlayInfo->Position.z>>FLOATNS);
							}
							MoveAngle( moveStep*2 );
							MoveAngle( moveStep*2 );
						}
						else {
							if ( smCharInfo.dwCharSoundCode==snCHAR_SOUND_WOLVERLIN ) {
								//울버린 달리기
								if ( lpTargetPlayInfo && TragetTraceMode ) {
									SetTargetPosi2( lpTargetPlayInfo->Position.x>>FLOATNS , 
										lpTargetPlayInfo->Position.z>>FLOATNS);
								}
								MoveAngle( moveStep*2 );
							}
							else 
								MoveAngle( moveStep );
						}
					}
				}

				if ( MotionInfo->State!=CHRMOTION_STATE_WALK && MotionInfo->State!=CHRMOTION_STATE_RUN ) 
					SetMotionFromCode( CHRMOTION_STATE_WALK );


				
				if ( OverLapPosi || pX!=OldPos.x || pZ!=OldPos.z ) {
						lpCharOverLap = CheckOtherPlayPosi( pX , pY, pZ );
						if ( lpCharOverLap ) {
							if ( !OverLapPosi ) {
								pX = OldPos.x;
								pZ = OldPos.z;
								lpCharOverLap = CheckOtherPlayPosi( pX , pY, pZ );
							}

							if ( lpCharOverLap ) {
								OverLapPosi = TRUE;
								OverLapPosi2 = FALSE;
							}
							else {
								OverLapPosi = FALSE;
								OverLapPosi2 = TRUE;
							}

						}
						else {
							OverLapPosi = FALSE;
							OverLapPosi2 = FALSE;
						}
				}
				if ( !OverLapPosi ) lpCharOverLap=0;
			}
			else {
				if ( MotionInfo->State!=CHRMOTION_STATE_STAND ) SetMotionFromCode( CHRMOTION_STATE_STAND );
			}
		}

		frame += frameStep;

		if ( frame>MotionInfo->EndFrame*160 ) {

			if ( MotionInfo->State == CHRMOTION_STATE_DEAD ) {
				frame = MotionInfo->EndFrame*160;
			}
			else {

				if ( MotionInfo->State==CHRMOTION_STATE_EAT && PotionLog==1 ) {
					//물약 사용 성공
					smCharInfo.Life[0] += (smCharInfo.Life[1]>>1);
					if ( smCharInfo.Life[0]>smCharInfo.Life[1] )
						smCharInfo.Life[0] = smCharInfo.Life[1];
				}

				if ( smMonsterInfo.SkillCurse && smCharInfo.Brood!=smCHAR_MONSTER_USER ) {
					if ( MotionInfo->State==CHRMOTION_STATE_ATTACK ) {
						lpTargetPlayInfo = 0;
					}

					if ( lpTargetChar && MotionInfo->State==CHRMOTION_STATE_SKILL)  {
						lpTargetChar->PlayHolyPower[0] = GetRandomPos( 10,20 );
						lpTargetChar->PlayHolyPower[1] = 16*20;
						lpTargetChar = 0;
					}
				}

				if ( (lpTargetPlayInfo || TargetMoveCount ) && MotionInfo->State<0x100 )
					PosBeginCount ++;

				if ( MotionInfo->Repeat == TRUE ) {
					frame = (MotionInfo->StartFrame*160)+(frame - MotionInfo->EndFrame*160);
				}
				else {
					if ( MotionInfo->State == CHRMOTION_STATE_DAMAGE && !PlayStunCount && !PlayStopCount && 
						SetMotionFromCode( CHRMOTION_STATE_WALK ) ) 
					{
						TargetMoveCount = 2;
					}
					else
						SetMotionFromCode( CHRMOTION_STATE_STAND );
					AutoMoveStep = 1;
				}

				if ( PosBeginCount>4 ) {
					x = (pX-PosBeginMove.x)>>FLOATNS;
					z = (pZ-PosBeginMove.z)>>FLOATNS;
					dist = x*x+z*z;

					PosBeginMove.x = pX;
					PosBeginMove.z = pZ;
					PosBeginCount = 0;

					if ( dist<(32*32) ) {
						SetTargetPosi( PosiAround.x>>FLOATNS , PosiAround.z>>FLOATNS );
						TragetTraceMode = 0;

						y = (rand()%ANGLE_90)-ANGLE_45;
						Angle.y = (Angle.y+y)&ANGCLIP;

						TargetMoveCount = 10;
						OverLapPosi2 = FALSE;

						ptNextTarget.x = 0;
						ptNextTarget.y = 0;
						ptNextTarget.z = 0;

						if ( !SetMotionFromCode( CHRMOTION_STATE_RUN ) ) {
							SetMotionFromCode( CHRMOTION_STATE_WALK );
						}

						//lpTargetPlayInfo = 0;
						goto Skip_5;
					}
				}

				if ( TargetMoveCount>0 ) {
					TargetMoveCount--;
					if ( TargetMoveCount<=0 ) SetMotionFromCode( CHRMOTION_STATE_STAND );
				}

				if ( DistAroundDbl && ( MotionInfo->State==CHRMOTION_STATE_WALK ||
					MotionInfo->State==CHRMOTION_STATE_RUN ) ) {
					//거리를 벗어난경우 처리
					x = (pX-PosiAround.x)>>FLOATNS;
					z = (pZ-PosiAround.z)>>FLOATNS;
					dist = x*x+z*z;
					if ( dist>DistAroundDbl ) {
						SetTargetPosi( PosiAround.x>>FLOATNS , PosiAround.z>>FLOATNS );
						ptNextTarget.x = ptNextTarget.z = 0;
						TargetMoveCount = 6;
						TragetTraceMode = 0;
					}
				}

				if ( !TargetMoveCount && ptNextTarget.x && ptNextTarget.z ) {

					SetTargetPosi( ptNextTarget.x>>FLOATNS , ptNextTarget.z>>FLOATNS);
					TragetTraceMode = TRUE;

					if ( OverLapPosi && lpCharOverLap ) {
						//캐릭터가 겹쳐 있음 ( 떨어지도록 노력 함 )
						Angle.y = GetRadian2D( lpCharOverLap->pX>>FLOATNS , lpCharOverLap->pZ>>FLOATNS ,
							pX>>FLOATNS , pZ>>FLOATNS );

						SetMotionFromCode( CHRMOTION_STATE_WALK );
						OverLapPosi2 = FALSE;
						AutoMoveStep = 0;
					}
					else {
						if ( AutoMoveStep<=0 ) {
							if ( MotionInfo->State==CHRMOTION_STATE_STAND ) {
								//지능에 비례한 이동
								AutoMoveStep = smMonsterInfo.IQ - 5;
								if ( AutoMoveStep<0 ) AutoMoveStep=0;

								x = (pX-ptNextTarget.x)>>FLOATNS;
								z = (pZ-ptNextTarget.z)>>FLOATNS;
								dist = x*x+z*z;

								if ( smMonsterInfo.IQ>=6 && dist<(64*8*64*8) ) {
									if ( SetMotionFromCode( CHRMOTION_STATE_RUN ) ) {
										AutoMoveStep<<=2;			//달리기는 좀더 오래
									}
									else
										SetMotionFromCode( CHRMOTION_STATE_WALK );
								}
								else
									SetMotionFromCode( CHRMOTION_STATE_WALK );
							}
							else
								SetMotionFromCode( CHRMOTION_STATE_STAND );
						}
						else {
							SetTargetPosi2( ptNextTarget.x>>FLOATNS , ptNextTarget.z>>FLOATNS);
							AutoMoveStep--;
						}
					}

					ptNextTarget.x = 0;
					ptNextTarget.y = 0;
					ptNextTarget.z = 0;
				}

				//몬스터 물약 사용
				if ( smMonsterInfo.PotionCount ) {
					cnt = (smCharInfo.Life[0]*100)/smCharInfo.Life[1];
					if ( cnt<30 ) {
						if ( smCharInfo.dwCharSoundCode==snCHAR_SOUND_NPC_MAGICMASTER ) {
							//매직마스터는 물약대신 힐링
							smTransCommand.code = smTRANSCODE_PROCESS_SKILL2;
							smTransCommand.size = sizeof(smTRANS_COMMAND);
							smTransCommand.WParam = SKILL_PLAY_HEALING;
							smTransCommand.LParam = dwObjectSerial;
							smTransCommand.SParam = 0;
							smTransCommand.EParam = 0;
							if ( lpLinkPlayInfo && lpLinkPlayInfo->lpsmSock ) 
								lpLinkPlayInfo->lpsmSock->Send2( (char *)&smTransCommand , smTransCommand.size , TRUE );

							smCharInfo.Life[0] += (smCharInfo.Life[1]>>2);
							if ( smCharInfo.Life[0]>smCharInfo.Life[1] )
								smCharInfo.Life[0] = smCharInfo.Life[1];

						}
						else {	//일반 몬스터는 물약 사용
							SetMotionFromCode( CHRMOTION_STATE_EAT );
							PotionLog = 1;
						}

						smMonsterInfo.PotionCount--;
					}
				}

				if ( dwNextMotionCode ) {
					//다음 동작 예약 실행
					if ( SetMotionFromCode( dwNextMotionCode )==FALSE ) {
						SetMotionFromCode( CHRMOTION_STATE_STAND );
					}
					dwNextMotionCode = 0;
				}
				if ( smCharInfo.dwCharSoundCode==snCHAR_SOUND_HUNGKY ) {
					//변신
					cnt = (smCharInfo.Life[0]*100)/smCharInfo.Life[1];
					if ( cnt<30 || AttackAnger>80 ) {
						SetMotionFromCode( CHRMOTION_STATE_WARP );
						lpTargetPlayInfo = 0;
					}
				}

				// 박재원 - 복날 이벤트
				if ( smCharInfo.dwCharSoundCode==snCHAR_SOUND_BEBECHICK ) // 아기 꼬꼬 -> 아빠 꼬꼬 로 변신
				{
					//변신
					cnt = (smCharInfo.Life[0]*100)/smCharInfo.Life[1];
					if ( cnt<30 || AttackAnger>80 ) {
						SetMotionFromCode( CHRMOTION_STATE_WARP );
						lpTargetPlayInfo = 0;
					}
				}

				if( smCharInfo.dwCharSoundCode == snCHAR_SOUND_DEADHOPT )	// pluto 언데드 킹호피 변신
				{
					cnt = (smCharInfo.Life[0]*100) / smCharInfo.Life[1];
                    if( cnt < 20 || AttackAnger > 80 )
					{
						SetMotionFromCode( CHRMOTION_STATE_WARP );
						lpTargetPlayInfo = 0;
					}
				}

			}
		}
Skip_5:
		if ( DontMoveFlag==2 ) {
			//각도 고정
			Angle.x = 0;
			Angle.y = 0;
			Angle.z = 0;
		}
		SavePlayBuff2();

		if ( lpMasterPlayInfo && HoSkillCode && lpMasterPlayInfo->lpsmSock && smCharInfo.Life[0]>0 ) {

			if ( lpMasterPlayInfo->lpLinkChar!=this ) {
				smCharInfo.Life[0]=0;
				SetMotionFromCode( CHRMOTION_STATE_DEAD );
			}
			else {
				//소환 몹 거리 조절
				x = (lpMasterPlayInfo->Position.x - pX)>>FLOATNS;
				z = (lpMasterPlayInfo->Position.z - pZ)>>FLOATNS;
				dist = x*x+z*z;
				if ( dist>=(500*500) ) {
					if ( lpStage ) {
						y = lpStage->GetHeight( lpMasterPlayInfo->Position.x,lpMasterPlayInfo->Position.z );
						if ( y<0 ) {
							smCharInfo.Life[0]=0;
							SetMotionFromCode( CHRMOTION_STATE_DEAD );
						}
					}
					if ( smCharInfo.Life[0]>0 ) {
						pX = lpMasterPlayInfo->Position.x;
						pY = lpMasterPlayInfo->Position.y;
						pZ = lpMasterPlayInfo->Position.z;

						ptNextTarget.x = ptNextTarget.z = 0;
						TargetMoveCount = 0;
						TragetTraceMode = 0;

						lpTargetChar = 0;

					}
				}
				else if ( dist>=(300*300) || (!lpTargetChar && dist>=(150*150)) ) {
					SetTargetPosi( lpMasterPlayInfo->Position.x>>FLOATNS , lpMasterPlayInfo->Position.z>>FLOATNS );
					TargetMoveCount = 2;
					TragetTraceMode = 0;
					lpTargetChar = 0;
					if ( MotionInfo->State!=CHRMOTION_STATE_DEAD && ( MotionInfo->State>=0x100 || MotionInfo->State==CHRMOTION_STATE_STAND ) ) {
						if ( !SetMotionFromCode( CHRMOTION_STATE_RUN ) )
							SetMotionFromCode( CHRMOTION_STATE_WALK );
					}
				}
			}
		}

		if ( dwLinkObjectCode ) {
			switch( smCharInfo.dwCharSoundCode ) {
				case snCHAR_SOUND_NPC_SKILLMASTER:
				case snCHAR_SOUND_NPC_MAGICMASTER:

					if ( smCharInfo.Life[0]<=(smCharInfo.Life[1]>>4) ) {
						//스킬마스터를 이겼다!
						smCharInfo.Life[0] = (smCharInfo.Life[1]>>4);
						lpTargetPlayInfo = 0;
						lpTargetChar = 0;
						SetMotionFromCode( CHRMOTION_STATE_STAND );
						//smCharInfo.State=smCHAR_STATE_NPC;
						ActionPattern = 12;	
						PosiAround.x = pX;
						PosiAround.y = pY;
						PosiAround.z = pZ;
						DistAroundDbl =  256*256;
						if ( lpLinkPlayInfo && lpLinkPlayInfo->dwObjectSerial==dwLinkObjectCode && lpLinkPlayInfo->lpsmSock ) {
							rsProcessQuest( lpLinkPlayInfo , this , SIN_QUEST_CODE_CHANGEJOB4  , dwObjectSerial );
						}
						lpChar = rsGetLinkChar( this );		//연결캐릭 삭제
						if ( lpChar ) {
							lpChar->SetMotionFromCode(CHRMOTION_STATE_DEAD);
							lpChar->smCharInfo.Life[0]=0;
						}
						/*
						else {
							chrAutoPlayer[cnt].Close();
							((STG_AREA *)chrAutoPlayer[cnt].lpExt1)->MonsterCount--;
							((STG_AREA *)chrAutoPlayer[cnt].lpExt1)->DeleteMonTable( &chrAutoPlayer[cnt] );
						}
						*/
					}
					else {
						if ( smCharInfo.dwCharSoundCode==snCHAR_SOUND_NPC_MAGICMASTER ) {
							//레이몬 순간이동 회피
							if ( (rand()%100)<3 && MotionInfo->State!=CHRMOTION_STATE_SKILL && MotionInfo->State!=CHRMOTION_STATE_EAT ) {
								if ( lpLinkPlayInfo ) {
									x = (pX-lpLinkPlayInfo->Position.x)>>FLOATNS;
									z = (pZ-lpLinkPlayInfo->Position.z)>>FLOATNS;
									dist = x*x+z*z;
									if ( dist<(300*300) ) {
										pX = QUEST_ARENA_NPC_POS_X + ((rand()%600)-300)*fONE;
										pZ = QUEST_ARENA_NPC_POS_Z + ((rand()%600)-300)*fONE;
									}
								}
							}
						}
					}
					break;

				case snCHAR_SOUND_FURY:

					if ( smCharInfo.Life[0]<=0 && !smMonsterInfo.PotionCount && smCharInfo.Life[1]>1 ) {
						//스킬마스터를 이겼다!
						if ( lpLinkPlayInfo && lpLinkPlayInfo->dwObjectSerial==dwLinkObjectCode && lpLinkPlayInfo->lpsmSock ) {
							rsProcessQuest( lpLinkPlayInfo , this , HAQUEST_CODE_FURYOFPHANTOM  , dwObjectSerial );
							smCharInfo.Life[1] = 1;
						}
					}
					break;
			}
		}

		if ( lpTargetChar ) {
			if ( smMonsterInfo.SkillCurse && smCharInfo.Brood!=smCHAR_MONSTER_USER && lpTargetChar->smCharInfo.Brood!=smCHAR_MONSTER_USER ) {
				//몬스터 공격력 강화용
				if ( MotionInfo->State<0x100 ) {
					TargetMoveCount = 1;
					AutoMoveStep = 1;
					lpTargetChar = 0;
				}
			}
			else if ( lpTargetChar->Flag && !PlayStunCount && !PlayStopCount ) {

				dist = smCharInfo.Shooting_Range+32;			//슈팅 거리가 몬스터의 공격 거리
				if ( dist<80 ) dist = 80;						//거리 보정

				if ( MotionInfo->State!=CHRMOTION_STATE_DAMAGE && 
					MotionInfo->State!=CHRMOTION_STATE_DEAD &&
					MotionInfo->State!=CHRMOTION_STATE_EAT ) {

					if ( MotionInfo->State!=CHRMOTION_STATE_ATTACK &&
						MotionInfo->State!=CHRMOTION_STATE_SKILL && MotionInfo->State!=CHRMOTION_STATE_DEAD ) {

							cnt = GetDistanceDbl( pX , pZ , lpTargetChar->pX, lpTargetChar->pZ );

							dist = dist>>FLOATNS;
							dist *= dist;

							if ( cnt<dist && abs(pY-lpTargetChar->pY)<64*fONE ) {

								PosBeginMove.x = 0;
								PosBeginMove.z = 0;	
								PosBeginCount = 0;

								dwLastTransTime = dwPlayServTime;		//최근 활성화된 시간

								SetMotionFromCode( CHRMOTION_STATE_ATTACK );

								if ( smMonsterInfo.SkillRating>0 && MotionInfo->State==CHRMOTION_STATE_ATTACK ) {
									//몬스터 스킬 공격
									if ( (rand()%100)<smMonsterInfo.SkillRating )
										SetMotionFromCode( CHRMOTION_STATE_SKILL );
								}
	
								Angle.y = GetRadian2D( pX,pZ , lpTargetChar->pX, lpTargetChar->pZ );
								AttackX = lpTargetChar->pX;
								AttackY = lpTargetChar->pY;
								AttackZ = lpTargetChar->pZ;

							}
							else {
								if ( cnt>DIST_TRANSLEVEL_LOW ) {
									lpTargetChar=0;
								}
							}
						}
				}

				if ( smCharInfo.Brood!=smCHAR_MONSTER_USER ) {
					if ( FrameCounter==80 && smCharInfo.dwCharSoundCode==snCHAR_SOUND_BABEL ) {		//보스 바벨
						AttackSkillRange = 	smMonsterInfo.SkillRange*smMonsterInfo.SkillRange;

						//몬스터 생명력 재생치 구하는 함수
						smCharInfo.Life[0] += rsGetMonsterLifeRegen( this );

						if ( smCharInfo.Life[0]>smCharInfo.Life[1] || smCharInfo.Life[0]<0 )
							smCharInfo.Life[0]=smCharInfo.Life[1];
					}
				}


				if ( FrameCounter==40 && lpTargetChar ) {

					if ( MotionInfo->State==CHRMOTION_STATE_ATTACK || MotionInfo->State==CHRMOTION_STATE_SKILL ) {
						//SendTransAttack( 0 , lpTargetPlayInfo->lpsmSock , 0);
						AttackUserFlag = TRUE;
						AttackAnger++;				//공격 횟수 카운터
						cnt = sinGetPVPAccuracy( smCharInfo.Level , smCharInfo.Attack_Rating , lpTargetChar->smCharInfo.Level , lpTargetChar->smCharInfo.Defence );
						if ( (rand()%100)<cnt && lpTargetChar->smCharInfo.Life[0]>0) {
							//명중
							if ( !lpTargetChar->lpTargetPlayInfo ) {
								lpTargetChar->lpTargetChar = this;
							}
							else {
								dist = GetDistanceDbl( pX , pZ , lpTargetChar->pX, lpTargetChar->pZ );
								if ( GetDistanceDbl( pX , pZ , lpTargetChar->lpTargetPlayInfo->Position.x, lpTargetChar->lpTargetPlayInfo->Position.z )>dist ) {
									lpTargetChar->lpTargetChar = this;
									lpTargetChar->lpTargetPlayInfo = 0;
								}
							}

							// smCharInfo.Attack_Damage[0]  미스틱 처리

							if ( MotionInfo->State==CHRMOTION_STATE_SKILL && smCharInfo.Attack_Damage[0] ) {
								cnt = GetRandomPos( smMonsterInfo.SkillDamage[0] , smMonsterInfo.SkillDamage[1] );

								switch( smCharInfo.dwCharSoundCode ) {
									case snCHAR_SOUND_GUARDIAN_SAINT:
										//가디언 세인트 ( 범위 공격 )
										rsSkillDamage_AroundMonster( this );
										cnt = 0;
										break;
								}
							}
							else {
								cnt = GetRandomPos( smCharInfo.Attack_Damage[0] , smCharInfo.Attack_Damage[1] );
							}

							//소환몬스터 공격시 x8 데미지
							if ( smCharInfo.dwCharSoundCode==snCHAR_SOUND_BABEL || 
								smCharInfo.dwCharSoundCode==snCHAR_SOUND_CHAOSCARA || 
								smCharInfo.dwCharSoundCode==snCHAR_SOUND_DEATHKNIGHT ||
								smCharInfo.dwCharSoundCode==snCHAR_SOUND_MOKOVA ||
								smCharInfo.dwCharSoundCode==snCHAR_SOUND_KELVEZU ||
								smCharInfo.dwCharSoundCode==snCHAR_SOUND_MOKOVA ) 
							{		//보스
								cnt *= 8;
								if ( smCharInfo.dwCharSoundCode==snCHAR_SOUND_KELVEZU ) {
									lpTargetChar->smCharInfo.Life[0] = 0;
									cnt = 1;
								}
							}


							if ( lpTargetChar )
								cnt -= (cnt*lpTargetChar->smCharInfo.Absorption)/100;
							else
								cnt = 0;

							if ( cnt>0 ) {
								lpTargetChar->smCharInfo.Life[0] -= cnt;

								if ( smCharInfo.dwCharSoundCode==snCHAR_SOUND_WEB )	{	//소환 웹의 공격 받으면 느려진다
									if (lpTargetChar->smCharInfo.dwCharSoundCode!=snCHAR_SOUND_KELVEZU )	//켈베쥬제외
										lpTargetChar->PlaySlowCount = -36;
								}


								if ( lpTargetChar->smCharInfo.Life[0]<=0 ) {
									lpTargetChar->smCharInfo.Life[0]=0;
									lpTargetChar->SetMotionFromCode(CHRMOTION_STATE_DEAD);
									if ( lpMasterPlayInfo && lpMasterPlayInfo->lpsmSock && lpMasterPlayInfo->dwObjectSerial && lpMasterPlayInfo->dwObjectSerial==smCharInfo.Next_Exp ) {
										lpTargetChar->lpExt2 = lpMasterPlayInfo;
										//lpTargetChar->smMonsterInfo.GetExp>>=1;				//경험치 절반
										lpTargetChar->lpExt3 = this;
									}
									else {
										if ( lpMasterPlayInfo ) {
											//플레이어 연결 종료 또는 위치가 바뀜
											smCharInfo.Life[0] = 0;
											SetMotionFromCode(CHRMOTION_STATE_DEAD);
										}
										else {
											lpTargetChar = 0;
										}
									}
								}
								else {
									if ( lpTargetChar->MotionInfo->State<0x100 ) {
										lpTargetChar->SetMotionFromCode(CHRMOTION_STATE_DAMAGE);
									}
								}
							}
						}
					}
				}
			}
			else {
				SetNextTarget( 0,0,0 );
			}

			if ( lpTargetChar && (!lpTargetChar->Flag || lpTargetChar->smCharInfo.Life[0]<=0) ) {
				lpTargetChar=0;
				if ( MotionInfo->State<0x100 ) {
					TargetMoveCount = 1;
					AutoMoveStep = 1;
				}
			}

			//if ( (smCharInfo.dwCharSoundCode==snCHAR_SOUND_DEATHKNIGHT || smCharInfo.dwCharSoundCode==snCHAR_SOUND_KELVEZU ) && lpTargetChar ) 
			if ( smCharInfo.dwCharSoundCode==snCHAR_SOUND_DEATHKNIGHT && lpTargetChar ) 
			{		//발렌토 얍삽이 상대 검사
				if ( GetDistanceDbl( pX, pZ , lpTargetChar->pX , lpTargetChar->pZ )>120*120 ) {
					lpTargetChar = 0;			//압삽이 상대 안함
					SetNextTarget( 0,0,0 );
				}
			}

		}


		cnt = TRUE;

		switch( smCharInfo.dwCharSoundCode ) {		//뱀피릭 머신 공격 제한 (깃발내 몬스터수 확인)
			case snCHAR_SOUND_VAMPRICMACHINE:
				//해당 몬스터 출신 깃발 의 활성화 몬스터 수
				if ( rsGetMonCountFlag( this )>3 ) cnt = FALSE;
				break;

			case snCHAR_SOUND_MYSTIC:				//죽을때 미스틱 자폭
				if ( MotionInfo->State==CHRMOTION_STATE_DEAD && FrameCounter<16 ) {
					AttackSkillRange = 	smMonsterInfo.SkillRange*smMonsterInfo.SkillRange;
					AttackUserFlag = TRUE;
					FrameCounter = 16;
				}
				if ( MotionInfo->State!=CHRMOTION_STATE_DEAD ) {
					if ( Counter>16*60*2 ) {
						smCharInfo.Life[0]  = 0;
						SetMotionFromCode(CHRMOTION_STATE_DEAD);
						AttackSkillRange = 	smMonsterInfo.SkillRange*smMonsterInfo.SkillRange;
						AttackUserFlag = TRUE;
						FrameCounter = 16;
					}
				}
				if ( lpTargetChar && MotionInfo->State==CHRMOTION_STATE_DEAD ) {
					if ( FrameCounter==16 && lpTargetChar->MotionInfo->State!=CHRMOTION_STATE_DEAD && lpTargetChar->smCharInfo.Life[0]>0 ) {
						lpTargetChar->smCharInfo.Life[0] -= GetRandomPos( smMonsterInfo.SkillDamage[0] , smMonsterInfo.SkillDamage[1] ) * 2;
						if ( lpTargetChar->smCharInfo.Life[0]<=0 ) {
							lpTargetChar->smCharInfo.Life[0]  = 0;
							lpTargetChar->SetMotionFromCode(CHRMOTION_STATE_DEAD);
						}
					}
					lpTargetChar = 0;
				}
				break;

			case snCHAR_SOUND_CHAOSCARA:
				if ( MotionInfo->State==CHRMOTION_STATE_SKILL && MotionInfo->KeyCode!='J' ) {
					smMonsterInfo.SkillRangeRect.left = 0;
					smMonsterInfo.SkillRangeRect.right = 0;
					smMonsterInfo.SkillRangeRect.top = 0;
					smMonsterInfo.SkillRangeRect.bottom = 0;
				}
				break;
		}

		


		//서버용 처리 ( 목표 캐릭을 추적하여 공격 하도록 설정 )
		if ( !lpTargetChar && lpTargetPlayInfo && cnt ) {
			if ( lpTargetPlayInfo->lpsmSock && !PlayStunCount && !PlayStopCount ) {

				dist = smCharInfo.Shooting_Range;			//슈팅 거리가 몬스터의 공격 거리

				if ( MotionInfo->State!=CHRMOTION_STATE_DAMAGE && 
					MotionInfo->State!=CHRMOTION_STATE_DEAD &&
					MotionInfo->State!=CHRMOTION_STATE_EAT && dist>=0 ) {
					if ( PlayAttackFromPosi( 
							lpTargetPlayInfo->Position.x ,
							lpTargetPlayInfo->Position.y ,
							lpTargetPlayInfo->Position.z ,
							dist )==TRUE ) {
						
						PosBeginMove.x = 0;
						PosBeginMove.z = 0;	
						PosBeginCount = 0;

						dwLastTransTime = dwPlayServTime;		//최근 활성화된 시간

						if ( smMonsterInfo.SkillRating>0 && MotionInfo->State==CHRMOTION_STATE_ATTACK ) {
							//몬스터 스킬 공격
							if ( (rand()%100)<smMonsterInfo.SkillRating ) { 
								if ( smCharInfo.dwCharSoundCode==snCHAR_SOUND_NPC_MAGICMASTER && (rand()%100)<20 ) {
									//매직 마스터 기력 물약먹기
									SetMotionFromCode( CHRMOTION_STATE_EAT );
									PotionLog = 2;
								}
								else if ( SetMotionFromCode( CHRMOTION_STATE_SKILL )>0 ) {
									if ( smMonsterInfo.SkillCurse ) {
										//저주 걸기 기술
										switch( smMonsterInfo.SkillCurse ) {
											case 102:	//다른 몬스터 공격력 강화 기술
												lpChar = rsFindAroundMonster( this , 200 );
												if ( lpChar ) {
													lpTargetChar = lpChar;
													lpTargetPlayInfo = 0;
													Angle.y = GetRadian2D( pX,pZ , lpTargetChar->pX, lpTargetChar->pZ );
												}
												else {
													//저주 기술 실패
													if ( SetMotionFromCode( CHRMOTION_STATE_ATTACK )==FALSE ) 
														SetMotionFromCode( CHRMOTION_STATE_STAND );
												}
												break;

											case 103:	//유저에게 저주걸기 ( 공격력 약화 )
												lpTargetPlayInfo->dwCurse_Attack_Time = dwPlayServTime+SUCCUBUS_CURSE_TIME*1000;
												lpTargetPlayInfo->dwCurse_Attack_Param = GetRandomPos( 10,15 );
												break;

											case 101:	//유저에게 저주걸기 ( 흡수력 약화 )
												lpTargetPlayInfo->dwCurse_Defence_Time = dwPlayServTime+SUCCUBUS_CURSE_TIME*1000;
												lpTargetPlayInfo->dwCurse_Defence_Param = GetRandomPos( 2,6 );

												if ( smCharInfo.dwCharSoundCode==snCHAR_SOUND_MYSTIC )
													lpTargetPlayInfo->dwCurse_Defence_Param = GetRandomPos( 10,16 );

												break;

										}
									}


									if ( smMonsterInfo.SkillRange>0 ) {
										//범위형
										//AttackSkillRange = smMonsterInfo.SkillRange*smMonsterInfo.SkillRange;
										AttackSkillRange = 0;

										if ( smMonsterInfo.SkillDistance ) {

											GetMoveLocation( 0,0, smMonsterInfo.SkillDistance*fONE , 0, Angle.y , 0 );
											x = pX+GeoResult_X;
											z = pZ+GeoResult_Z;

											dist = smMonsterInfo.SkillRange<<FLOATNS;

                                            smMonsterInfo.SkillRangeRect.left = x-dist;
                                            smMonsterInfo.SkillRangeRect.right = x+dist;
                                            smMonsterInfo.SkillRangeRect.top = z-dist;
                                            smMonsterInfo.SkillRangeRect.bottom = z+dist;
										}
									}

									switch ( smCharInfo.dwCharSoundCode ) {
										case snCHAR_SOUND_RATOO:
											//주변 몬스터를 찾아 스킬 ( 라투 주변 몬스터 스턴제거 )
											rsSkillMonster_AroundMonster( this , 200 );
											break;
										case snCHAR_SOUND_NPC_SKILLMASTER:
											cnt = rand()%100;
											if ( cnt<3 && !rsGetLinkChar(this) && lpLinkPlayInfo ) {
												if ( (cnt&1)==0 ) {
													lpChar = OpenMonsterFromSkill( SKILL_PLAY_METAL_GOLEM , pX,pY,pZ, lpLinkPlayInfo , this );
													if ( lpChar ) {
														lpChar->smCharInfo.wPlayClass[1] = fONE+5*15;		//크기 확대
														lpChar->sMoveStepCount[0] = 1;		//이동 속도 2x2
														lpChar->sMoveStepCount[1] = 2;
														//lpChar->smCharInfo.Attack_Damage[0]/=2;
														//lpChar->smCharInfo.Attack_Damage[1]/=2;
														//lpChar->smMonsterInfo.SkillDamage[0]/=2;
														//lpChar->smMonsterInfo.SkillDamage[1]/=2;
													}
												}
												else {
													lpChar = OpenMonsterFromSkill( SKILL_PLAY_RECALL_WOLVERIN , pX,pY,pZ, lpLinkPlayInfo , this );
													if ( lpChar ) {
														lpChar->smCharInfo.wPlayClass[1] = (fONE/2)+5*12;		//크기 확대
														lpChar->sMoveStepCount[0] = 1;		//이동 속도 2x2
														lpChar->sMoveStepCount[1] = 2;
														//lpChar->smCharInfo.Attack_Damage[0]/=2;
														//lpChar->smCharInfo.Attack_Damage[1]/=2;
														//lpChar->smMonsterInfo.SkillDamage[0]/=2;
														//lpChar->smMonsterInfo.SkillDamage[1]/=2;
													}
												}
											}
											break;
										case snCHAR_SOUND_NPC_MAGICMASTER:
											cnt = rand()%100;
											if ( cnt<3 && !rsGetLinkChar(this) && lpLinkPlayInfo ) {
												lpChar = OpenMonsterFromSkill( SKILL_PLAY_FIRE_ELEMENTAL , pX,pY,pZ, lpLinkPlayInfo , this );
												if ( lpChar ) {
													lpChar->sMoveStepCount[0] = 1;		//이동 속도 2x2
													lpChar->sMoveStepCount[1] = 2;
													//lpChar->smCharInfo.Attack_Damage[0]/=2;
													//lpChar->smCharInfo.Attack_Damage[1]/=2;
													//lpChar->smMonsterInfo.SkillDamage[0]/=2;
													//lpChar->smMonsterInfo.SkillDamage[1]/=2;
												}
											}
											break;
									}
								}
							}
						}
					}
				}

				EvtFrame = 0;
				if ( smCharInfo.dwCharSoundCode==snCHAR_SOUND_BABEL ) {		//보스 바벨
					if ( FrameCounter==40 || FrameCounter==80 ) 
						EvtFrame = 1;
				}


				if ( (!EvtFrame && FrameCounter==40) || EvtFrame ) {
					if ( MotionInfo->State==CHRMOTION_STATE_ATTACK && 
						rsProcessAttack_SkillHolyReflection( lpTargetPlayInfo , this )==TRUE ) {

						if ( smCharInfo.dwCharSoundCode==snCHAR_SOUND_VAMPRICMACHINE ) {		//뱀피릭 머신 벌발사
							cnt = 0;
							//특정위치에 보조몬스터 강제 출현 시킴
							rsOpenSubMonster( this , snCHAR_SOUND_VAMPRICBEE , pX, pY, pZ , 4 );
						}
						else {
							if ( smCharInfo.dwCharSoundCode==snCHAR_SOUND_BABEL && (rand()%100)<60 ) {	//60%확률
								//바벨 일반공격시 스킬도 같이 적용 (범위지정)
								rsSendRangeDamage( this , lpTargetPlayInfo , 100 , fONE/2 );

								x = (lpTargetPlayInfo->Position.x - pX)>>FLOATNS;
								z = (lpTargetPlayInfo->Position.z - pZ)>>FLOATNS;
								dist = x*x+z*z;
								if ( dist>(200*200) ) {
									//원거리 70%데미지
									dm1 = smCharInfo.Attack_Damage[0];
									dm2 = smCharInfo.Attack_Damage[1];

									smCharInfo.Attack_Damage[0] = (smCharInfo.Attack_Damage[0]*70)/100;
									smCharInfo.Attack_Damage[1] = (smCharInfo.Attack_Damage[1]*70)/100;
									cnt = SendTransAttack( 0 , lpTargetPlayInfo->lpsmSock , 0);

									smCharInfo.Attack_Damage[0] = dm1;
									smCharInfo.Attack_Damage[1] = dm2;
								}
								else {
									//근거리 100% 데미지
									cnt = SendTransAttack( 0 , lpTargetPlayInfo->lpsmSock , 0);
								}

							}
							else {
								cnt = SendTransAttack( 0 , lpTargetPlayInfo->lpsmSock , 0);
							}
						}

						if ( cnt ) {
							//공격 데이타 기록
							lpTargetPlayInfo->Send_AttackCount++;
							lpTargetPlayInfo->Send_AttackDamage+=cnt;

//해외//////////////////////////////////////////////////////////////////////////////////////////////////
/*							if ( rsServerConfig.Event_Child==1 && PartyFlag!=rsHARDCORE_EVENT_FLAG )
							{
								//어린이 이벤트
								if ( smCharInfo.dwCharSoundCode==snCHAR_SOUND_BUMA || 
									smCharInfo.dwCharSoundCode==snCHAR_SOUND_SEN ||
									smCharInfo.dwCharSoundCode==snCHAR_SOUND_BEEDOG ||
									smCharInfo.dwCharSoundCode==snCHAR_SOUND_MINIG ||
									smCharInfo.dwCharSoundCode==snCHAR_SOUND_DECOY ||
									smCharInfo.dwCharSoundCode==snCHAR_SOUND_MUMMY ||
									smCharInfo.dwCharSoundCode==snCHAR_SOUND_HULK ||
									smCharInfo.dwCharSoundCode==snCHAR_SOUND_SANDLEM ) {

									if ( (rand()%70)==1 ) {
										smTransCommand.size = sizeof(smTRANS_COMMAND);
										smTransCommand.code = smTRANSCODE_COMIC_SKIN;
										smTransCommand.WParam = (rand()%2)+1;
										smTransCommand.LParam = 60*10;
										smTransCommand.SParam = 0;
										smTransCommand.EParam = 0;
										lpTargetPlayInfo->lpsmSock->Send2( (char *)&smTransCommand , smTransCommand.size , TRUE );
									}
								}
							}
*/   /////////////////////////////////////////////////////////////////////////////////////////////////////
							//뽕망치 이벤트
							if ( smCharInfo.wPlayClass[0]==MONSTER_CLASS_HAMMER &&
								lpTargetPlayInfo->smCharInfo.SizeLevel<=0x1000 || lpTargetPlayInfo->smCharInfo.SizeLevel>0x1002 ) {

								if ( dwEventAttackParam!=lpTargetPlayInfo->dwObjectSerial ) {

									EventAttackCount = 0;
									dwEventAttackParam = lpTargetPlayInfo->dwObjectSerial;
								}

								EventAttackCount++;

								if ( EventAttackCount>=10 ) {

									smTransCommand.size = sizeof(smTRANS_COMMAND);
									smTransCommand.code = smTRANSCODE_COMIC_SKIN;
									smTransCommand.WParam = (rand()%2)+1;
									smTransCommand.LParam = 60*10;
									smTransCommand.SParam = 0;
									smTransCommand.EParam = 0;
									lpTargetPlayInfo->lpsmSock->Send2( (char *)&smTransCommand , smTransCommand.size , TRUE );

									if ( smMonsterInfo.UseEventModel && smCharInfo.szModelName2[1] ) {
										//이벤트 모델 사용중

										Close();

										lstrcpy( smCharInfo.szModelName , smCharInfo.szModelName2+1 );
										SetLoadPattern( smCharInfo.szModelName );
										smCharInfo.bUpdateInfo[0]++;
										SetMotionFromCode(CHRMOTION_STATE_STAND);

									}

									smCharInfo.wPlayClass[0]=MONSTER_CLASS_NORMAL;
									smMonsterInfo.UseEventModel = 0;
								}

								TRANS_CHATMESSAGE	TransChatMessage;
								wsprintf( TransChatMessage.szMessage , "%s: %d " , smCharInfo.szName , EventAttackCount );
								TransChatMessage.code = smTRANSCODE_WHISPERMESSAGE;
								TransChatMessage.size = 32+lstrlen(TransChatMessage.szMessage);
								TransChatMessage.dwIP = 0;
								TransChatMessage.dwObjectSerial = dwObjectSerial;

								rsSendCommandUser2( lpTargetPlayInfo , (smTRANS_COMMAND *)&TransChatMessage );
							}
						}

						AttackUserFlag = TRUE;
						AttackAnger++;				//공격 횟수 카운터

						if ( lpTargetPlayInfo->BadPlayer==4 && lpTargetPlayInfo->lpsmSock ) {
							if ( (smCharInfo.Level+6)>lpTargetPlayInfo->smCharInfo.Level && (rand()%100)>92 ) {
								//배드 플레이어 확인 강제로 죽이기
								smTransCommand.code = smTRANSCODE_DEAD_PLAYER;
								smTransCommand.size = sizeof(smTRANS_COMMAND);
								smTransCommand.WParam = 0;
								smTransCommand.LParam = 0;
								smTransCommand.SParam = 0;
								smTransCommand.EParam = 0;
								lpTargetPlayInfo->lpsmSock->Send2( (char *)&smTransCommand , smTransCommand.size , TRUE );
								lpTargetPlayInfo->BadPlayer=2;		//죽인후 해제
							}
						}
					}
				}

				EvtFrame = 32;

				if ( smCharInfo.dwCharSoundCode==snCHAR_SOUND_BABEL || smCharInfo.dwCharSoundCode==snCHAR_SOUND_DEATHKNIGHT ) {		//보스 바벨
					EvtFrame = 120;
					if ( FrameCounter==EvtFrame ) {							//피 채워준다
						//몬스터 생명력 재생치 구하는 함수
						smCharInfo.Life[0] += rsGetMonsterLifeRegen( this );

						if ( smCharInfo.Life[0]>smCharInfo.Life[1] || smCharInfo.Life[0]<0 )
							smCharInfo.Life[0]=smCharInfo.Life[1];

					}
				}

				if ( FrameCounter==EvtFrame ) {
					if ( MotionInfo->State==CHRMOTION_STATE_SKILL ) {
						if ( smCharInfo.dwCharSoundCode!=snCHAR_SOUND_MYSTIC ) {				//미스틱 저주 데미지 없음
							AttackSkillRange = 	smMonsterInfo.SkillRange*smMonsterInfo.SkillRange;
							if ( !AttackSkillRange && lpTargetPlayInfo ) {
								//상대 캐릭터에 기술 공격 가함 
								rsSendTransSkillAttack( this , lpTargetPlayInfo );
							}
							AttackUserFlag = TRUE;
						}
					}
				}

			}
			else {
				SetNextTarget( 0,0,0 );
			}
		}


		if ( PlaySlowCount>0 ) 		//몬스터는 저속 상태 유지 모드로 사용 
			PlaySlowCount--;

		if ( PlaySlowCount<0 ) 		//몬스터는 저속 상태 유지 모드로 사용 
			PlaySlowCount++;

		if ( PlayStunCount>0 )		//몬스터는 중립 상태 유지 모드로 사용 (로어 공격 해당 )
			PlayStunCount--;

		if ( PlayStopCount>0 )		//몬스터는 움직임이 멈춰 있다
			PlayStopCount--;

		if ( PlayHolyMind[1]>0 )	//데미지 약화
			PlayHolyMind[1]--;

		if ( PlayHolyPower[1]>0 )	//데미지 강화
			PlayHolyPower[1]--;

		if ( PlayPoison[1]>0 ) {
			PlayPoison[1]--;			//독 감염
			if ( (Counter&0xF)==0 ) {
				if ( smCharInfo.Life[0]>0 && MotionInfo->State!=CHRMOTION_STATE_DEAD ) {
					smCharInfo.Life[0]-= PlayPoison[0];			//데미지 감소
					if ( smCharInfo.Life[0]<1 ) {				//죽음
						smCharInfo.Life[0]=1;
						//SetMotionFromCode( CHRMOTION_STATE_DEAD );
						PlayPoison[1] = 0;
					}
				}
			}
		}

		if ( PlayHolyIncantation[1]>0 ) {
			PlayHolyIncantation[1]--;
			if ( PlayHolyIncantation[1]==0 && smCharInfo.Brood==smCHAR_MONSTER_USER ) {
				smCharInfo.Brood=PlayHolyIncantation[0];	//smCHAR_MONSTER_USER
				smCharInfo.bUpdateInfo[0]++;
				lpMasterPlayInfo = 0;
			}
		}

		if ( UseObject_VirtualLife ) {
			//가상의 생명력
			if ( sObject_VirtualLife[0]>0 ) {
				cnt = (smCharInfo.Life[0]*100)/smCharInfo.Life[1];
				if ( cnt<=10 ) { //10%이하 재생
					sObject_VirtualLife[0] --;
					smCharInfo.Life[0] = smCharInfo.Life[1];

					if ( smCharInfo.dwCharSoundCode==snCHAR_SOUND_KELVEZU && (sObject_VirtualLife[0]&0x1)==0 ) {
						//켈베쥬 (카오스 호출)
						rsOpenSubMonster( this , snCHAR_SOUND_CHAOSCARA , pX+((rand()%(200*fONE))-100*fONE), 0, pZ+((rand()%(200*fONE))-100*fONE) , 8 );
						rsOpenSubMonster( this , snCHAR_SOUND_CHAOSCARA , pX+((rand()%(200*fONE))-100*fONE), 0, pZ+((rand()%(200*fONE))-100*fONE) , 8 );
					}
					else if ( smCharInfo.dwCharSoundCode==snCHAR_SOUND_MOKOVA && (sObject_VirtualLife[0]&0x1)==0 ) {
						//모코바 (다크가드 호출)
						rsOpenSubMonster( this , snCHAR_SOUND_DARKGUARD , pX+((rand()%(200*fONE))-100*fONE), 0, pZ+((rand()%(200*fONE))-100*fONE) , 8 );
						rsOpenSubMonster( this , snCHAR_SOUND_DARKGUARD , pX+((rand()%(200*fONE))-100*fONE), 0, pZ+((rand()%(200*fONE))-100*fONE) , 8 );
					}

				}
			}

			//출력용 생명력
			cnt = (smCharInfo.Life[0]*100)/smCharInfo.Life[1];
			sObject_DisplayLife[0] = sObject_VirtualLife[0]*100;
			sObject_DisplayLife[0] += cnt;
			sObject_DisplayLife[1] = (sObject_VirtualLife[1]+1)*100;

			if ( sObject_DisplayLife[0]<=0 && smCharInfo.Life[0]>0 ) {
				sObject_DisplayLife[0] = 1;
			}

			if ( sObject_DisplayLife[0]>sObject_DisplayLife[1] )
				sObject_DisplayLife[0] = sObject_DisplayLife[1];

		}

		FrameCounter+=3;
		break;


	case 10:	//동물
	case 11:	//NPC
		if ( MotionInfo->State && MotionInfo->State<0x100 ) {
			if ( MotionInfo->State==CHRMOTION_STATE_WALK ) {
				MoveAngle( SERVER_CHAR_SKIP );
				if ( MotionInfo->State!=CHRMOTION_STATE_WALK ) SetMotionFromCode( CHRMOTION_STATE_WALK );
			}
			else {
				if ( MotionInfo->State!=CHRMOTION_STATE_STAND ) SetMotionFromCode( CHRMOTION_STATE_STAND );
			}

		}

		frame += SERVER_CHAR_FRAME_STEP;

		if ( frame>MotionInfo->EndFrame*160 ) {

			if ( MotionInfo->State == CHRMOTION_STATE_DEAD ) {
				frame = MotionInfo->EndFrame*160;
			}
			else {

				if ( MotionInfo->Repeat == TRUE ) {
					frame = (MotionInfo->StartFrame*160)+(frame - MotionInfo->EndFrame*160);
				}
				else SetMotionFromCode( CHRMOTION_STATE_STAND );



				if ( MotionInfo->State==CHRMOTION_STATE_STAND ) {
					cnt = rand()%ANGCLIP;
					if ( ((cnt>>2)&3)==0 ) {
						//때때로 하는 동작
						if ( ActionPattern==10 ) {
							if ( !SetMotionFromCode( CHRMOTION_STATE_SOMETIME ) ) {
								Angle.y = cnt&ANGCLIP;
								SetMotionFromCode( CHRMOTION_STATE_WALK );						
							}						
						}
						else {
							if ( !SetMotionFromCode( TRUE ) ) {
								Angle.y = cnt&ANGCLIP;
								SetMotionFromCode( CHRMOTION_STATE_WALK );
							}						
						}
					}
					else {
						Angle.y = cnt&ANGCLIP;
						SetMotionFromCode( CHRMOTION_STATE_WALK );
					}

					if ( MotionInfo->State==CHRMOTION_STATE_WALK ) {
						//거리를 벗어난경우 처리
						x = (pX-PosiAround.x)>>FLOATNS;
						z = (pZ-PosiAround.z)>>FLOATNS;
						dist = x*x+z*z;
						if ( dist>DistAroundDbl ) {
							SetTargetPosi( PosiAround.x>>FLOATNS , PosiAround.z>>FLOATNS );
						}
					}
				}
				else SetMotionFromCode( CHRMOTION_STATE_STAND );

				if ( OverLapPosi && lpCharOverLap ) {
					//캐릭터가 겹쳐 있음 ( 떨어지도록 노력 함 )
					Angle.y = GetRadian2D( lpCharOverLap->pX>>FLOATNS , lpCharOverLap->pZ>>FLOATNS ,
						pX>>FLOATNS , pZ>>FLOATNS );

					SetMotionFromCode( CHRMOTION_STATE_WALK );
					OverLapPosi2 = FALSE;
				}
			}
		}
		SavePlayBuff();
		FrameCounter+=3;
		break;

	case 12:	//NPC
		//######################################################################################
		// 작 성 자 : 오 영 석	 ( 서버 코드에 추가 )
		// NPC 는 smMotionInfo2 사용치 않음. NPC 는 고정이다.		
		if( smMotionInfo->NpcMotionRate[ CHRMOTION_EXT ] )
		{
			///////////////////////////////////////////////////////////////////
			if( MotionInfo->State == CHRMOTION_STATE_WARP )
			{
				//워프 동작
				frame += SERVER_CHAR_FRAME_STEP;
				if( frame > MotionInfo->EndFrame * 160 )
				{
					Close();
					return TRUE;
				}
				SavePlayBuff();
				FrameCounter+=3;
				break;
			}
			///////////////////////////////////////////////////////////////////

			int TalkCnt = 0;
			if( MotionInfo->State == CHRMOTION_STATE_WALK )
			{
				x=pX; z=pZ;
				MoveAngle( SERVER_CHAR_SKIP );

				if( x == pX && z == pZ )
				{
					TalkCnt = rand() % 100;
					TalkCnt = smMotionInfo->NpcMotionRateCnt[ TalkCnt ];

					if( TalkCnt == action && smMotionInfo->MotionInfo[ TalkCnt ].State == CHRMOTION_STATE_WALK )
					{
						//거리를 벗어난경우 처리
						x = (pX-PosiAround.x)>>FLOATNS;
						z = (pZ-PosiAround.z)>>FLOATNS;
						dist = x*x+z*z;
						if( dist > DistAroundDbl )
							SetTargetPosi( PosiAround.x>>FLOATNS, PosiAround.z>>FLOATNS );
					}
					else
					{
						frame = MotionInfo->EndFrame * 160;
                        MotionInfo->State = CHRMOTION_STATE_STAND;
					}
				}
			}

			frame += SERVER_CHAR_FRAME_STEP;
			if( frame > MotionInfo->EndFrame * 160 )
			{
				if( MotionInfo->State == CHRMOTION_STATE_WALK )
                    frame = (MotionInfo->StartFrame * 160) + (frame - MotionInfo->EndFrame * 160);
				else if( smCharInfo.dwCharSoundCode != snCHAR_SOUND_FURY )	// 퓨리는 걷지 않게
				{
					if( TalkCnt == 0 )
                        TalkCnt = rand() % 100;

					ChangeMotion( smMotionInfo->NpcMotionRateCnt[ TalkCnt ] );
					if( MotionInfo->State == CHRMOTION_STATE_WALK )
					{
						//거리를 벗어난경우 처리
						x = (pX-PosiAround.x)>>FLOATNS;
						z = (pZ-PosiAround.z)>>FLOATNS;
						dist = x*x+z*z;
						if( dist > DistAroundDbl )
							SetTargetPosi( PosiAround.x>>FLOATNS, PosiAround.z>>FLOATNS );
					}
					else
						Angle.y = rand() % ANGCLIP;
				}
				else // 퓨리 전용.
					SetMotionFromCode( CHRMOTION_STATE_STAND );
			}
		}
		//######################################################################################
		else // NpcMotionRate 값이 없다. (기존 NPC )
		{
			if ( MotionInfo->State == CHRMOTION_STATE_WARP ) {
				//워프 동작
				frame += SERVER_CHAR_FRAME_STEP;

				if ( frame>MotionInfo->EndFrame*160 ) {
					Close();
					return TRUE;
				}
				SavePlayBuff();
				FrameCounter+=3;
				break;
			}


			if ( MotionInfo->State && MotionInfo->State<0x100 ) {
				if ( MotionInfo->State==CHRMOTION_STATE_WALK ) {
					x=pX; z=pZ;

					MoveAngle( SERVER_CHAR_SKIP );

					if ( x==pX && z==pZ ) {
						Angle.y = rand()%ANGCLIP;
					}

					if ( MotionInfo->State!=CHRMOTION_STATE_WALK ) SetMotionFromCode( CHRMOTION_STATE_WALK );
				}
				else {
					if ( MotionInfo->State!=CHRMOTION_STATE_STAND ) SetMotionFromCode( CHRMOTION_STATE_STAND );
				}
			}

			frame += SERVER_CHAR_FRAME_STEP;

			if ( frame>MotionInfo->EndFrame*160 ) {

				if ( MotionInfo->State == CHRMOTION_STATE_DEAD ) {
					frame = MotionInfo->EndFrame*160;
				}
				else {

					if ( MotionInfo->Repeat == TRUE ) {
						frame = (MotionInfo->StartFrame*160)+(frame - MotionInfo->EndFrame*160);
					}
					else SetMotionFromCode( CHRMOTION_STATE_STAND );



					if ( MotionInfo->State==CHRMOTION_STATE_STAND && smCharInfo.dwCharSoundCode!=snCHAR_SOUND_FURY ) {	//퓨리는 걷지 않게
						cnt = rand()%ANGCLIP;
						if ( ((cnt>>2)&3)==0 ) {
							//때때로 하는 동작
								if ( !SetMotionFromCode( TRUE ) ) {
									if ( MoveSpeed ) Angle.y = cnt&ANGCLIP;

									if ( DistAroundDbl )
										SetMotionFromCode( CHRMOTION_STATE_WALK );
									else
										SetMotionFromCode( CHRMOTION_STATE_STAND );
								}
						}
						else {
							SetMotionFromCode( CHRMOTION_STATE_STAND );
						}

						if ( MotionInfo->State==CHRMOTION_STATE_WALK ) {
							//거리를 벗어난경우 처리
							x = (pX-PosiAround.x)>>FLOATNS;
							z = (pZ-PosiAround.z)>>FLOATNS;
							dist = x*x+z*z;
							if ( dist>DistAroundDbl ) {
								SetTargetPosi( PosiAround.x>>FLOATNS , PosiAround.z>>FLOATNS );
							}
						}
					}
					else SetMotionFromCode( CHRMOTION_STATE_STAND );
	/*
					if ( OverLapPosi && lpCharOverLap ) {
						//캐릭터가 겹쳐 있음 ( 떨어지도록 노력 함 )
						Angle.y = GetRadian2D( lpCharOverLap->pX>>FLOATNS , lpCharOverLap->pZ>>FLOATNS ,
							pX>>FLOATNS , pZ>>FLOATNS );

						SetMotionFromCode( CHRMOTION_STATE_WALK );
						OverLapPosi2 = FALSE;
					}
	*/
				}
			}
		}

		SavePlayBuff();
		FrameCounter+=3;
		break;

		/*
	case 12:	//NPC
		if ( MotionInfo->State == CHRMOTION_STATE_WARP ) {
			//워프 동작
			frame += SERVER_CHAR_FRAME_STEP;

			if ( frame>MotionInfo->EndFrame*160 ) {
				Close();
				return TRUE;
			}
			SavePlayBuff();
			FrameCounter+=3;
			break;
		}


		if ( MotionInfo->State && MotionInfo->State<0x100 ) {
			if ( MotionInfo->State==CHRMOTION_STATE_WALK ) {
				x=pX; z=pZ;

				MoveAngle( SERVER_CHAR_SKIP );

				if ( x==pX && z==pZ ) {
					Angle.y = rand()%ANGCLIP;
				}

				if ( MotionInfo->State!=CHRMOTION_STATE_WALK ) SetMotionFromCode( CHRMOTION_STATE_WALK );
			}
			else {
				if ( MotionInfo->State!=CHRMOTION_STATE_STAND ) SetMotionFromCode( CHRMOTION_STATE_STAND );
			}
		}

		frame += SERVER_CHAR_FRAME_STEP;

		if ( frame>MotionInfo->EndFrame*160 ) {

			if ( MotionInfo->State == CHRMOTION_STATE_DEAD ) {
				frame = MotionInfo->EndFrame*160;
			}
			else {

				if ( MotionInfo->Repeat == TRUE ) {
					frame = (MotionInfo->StartFrame*160)+(frame - MotionInfo->EndFrame*160);
				}
				else SetMotionFromCode( CHRMOTION_STATE_STAND );



				if ( MotionInfo->State==CHRMOTION_STATE_STAND && smCharInfo.dwCharSoundCode!=snCHAR_SOUND_FURY ) {	//퓨리는 걷지 않게
					cnt = rand()%ANGCLIP;
					if ( ((cnt>>2)&3)==0 ) {
						//때때로 하는 동작
							if ( !SetMotionFromCode( TRUE ) ) {
								if ( MoveSpeed ) Angle.y = cnt&ANGCLIP;

								if ( DistAroundDbl )
									SetMotionFromCode( CHRMOTION_STATE_WALK );
								else
									SetMotionFromCode( CHRMOTION_STATE_STAND );
							}
					}
					else {
						SetMotionFromCode( CHRMOTION_STATE_STAND );
					}

					if ( MotionInfo->State==CHRMOTION_STATE_WALK ) {
						//거리를 벗어난경우 처리
						x = (pX-PosiAround.x)>>FLOATNS;
						z = (pZ-PosiAround.z)>>FLOATNS;
						dist = x*x+z*z;
						if ( dist>DistAroundDbl ) {
							SetTargetPosi( PosiAround.x>>FLOATNS , PosiAround.z>>FLOATNS );
						}
					}
				}
				else SetMotionFromCode( CHRMOTION_STATE_STAND );
			}
		}
		SavePlayBuff();
		FrameCounter+=3;
		break;
		*/



	case 99:

		if ( PlayBuffCnt==PlayBuffPosi_End ) {

			if ( MotionInfo->State!=CHRMOTION_STATE_STAND && MotionInfo->State!=CHRMOTION_STATE_DEAD && MotionInfo->State!=CHRMOTION_STATE_WARP &&
				MotionInfo->State!=CHRMOTION_STATE_ATTACK && MotionInfo->State!=CHRMOTION_STATE_SKILL )
				SetMotionFromCode( CHRMOTION_STATE_STAND );

			frame += 80;
			compFrame = frame / 160;

			if ( compFrame>=MotionInfo->EndFrame ) {
				if ( MotionInfo->Repeat ) {
					ChangeMotion( action , AnimDispMode );
				}
				else 
					frame = (MotionInfo->EndFrame)*160;

				if ( MotionInfo->State==CHRMOTION_STATE_ATTACK || MotionInfo->State==CHRMOTION_STATE_SKILL )
					SetMotionFromCode( CHRMOTION_STATE_STAND );

			}
			TransDelayCheck();

		}
		else {
			playBuff = &PlayBuff[ PlayBuffCnt & PLAYBUFF_MASK ];
			OldAction = action;

			Angle.x = playBuff->angle[0];
			Angle.y = playBuff->angle[1];
			Angle.z = playBuff->angle[2];
			action  = playBuff->angle[3];

			if ( !action ) {
				action = OldAction;

				x = abs(pX - playBuff->x);
				y = abs(pY - playBuff->y);
				z = abs(pZ - playBuff->z);

				cnt = x+y+z;

				if ( cnt>32 ) {
					if ( MotionInfo->State!=CHRMOTION_STATE_WALK && MotionInfo->State!=CHRMOTION_STATE_RUN ) {

						if (cnt<512 ) {
							//걷기
							SetMotionFromCode( CHRMOTION_STATE_WALK );
						}
						else {
							//뛰기
							if ( !SetMotionFromCode( CHRMOTION_STATE_RUN ) )
								SetMotionFromCode( CHRMOTION_STATE_WALK );
						}
					}
				}
				else {
					if ( MotionInfo->State!=CHRMOTION_STATE_STAND ) {
						SetMotionFromCode( CHRMOTION_STATE_STAND );
					}
				}
			}

			pX = playBuff->x;
			pY = playBuff->y;
			pZ = playBuff->z;

			PHeight = pY;


			if ( !playBuff->dwTargetSerial ) chrAttackTarget=0;
			else {
				if ( chrAttackTarget && chrAttackTarget->dwObjectSerial==playBuff->dwTargetSerial ) {

				}
				else 
					chrAttackTarget = FindChrPlayer( playBuff->dwTargetSerial );
			}

			if ( playBuff->frame<smPLAYBUFF_FRAME_NONE ) {
//				if ( OldAction!=action ) {// || !MotionInfo->State || MotionInfo->State>=0x100 ) {

				//프레임 설정
				frame = playBuff->frame & CHAR_FRAME_MASK;

				//해당 프레임의 모델선택
				MotionSelectFrame = playBuff->frame>>CHAR_FRAME_SELSHIFT;
				if ( AnimDispMode ) {
					if ( MotionSelectFrame )
						PatDispMode = DISP_MODE_PATSUB;			//보조 모델
					else {
						PatDispMode = DISP_MODE_PATMAIN;		//주 모델
					}
				}

				if ( action ) {

					if ( PatDispMode == DISP_MODE_PATSUB )
						MotionInfo = &smMotionInfo2->MotionInfo[action];
					else
						MotionInfo = &smMotionInfo->MotionInfo[action];

					//######################################################################################
					//작 성 자 : 오 영 석
					FrameInfo.StartFrame = MotionInfo->StartFrame*160;
					FrameInfo.EndFrame   = MotionInfo->EndFrame*160;
					//######################################################################################
				}

				if ( OldMotionState!=MotionInfo->State ) {

					if ( MotionInfo->State>=0x100 && MotionInfo->State!=CHRMOTION_STATE_ATTACK && MotionInfo->State!=CHRMOTION_STATE_SKILL ) {
						//if ( MotionInfo->State!=CHRMOTION_STATE_DEAD || (OnStageField>=0 && StageField[OnStageField]->State!=FIELD_STATE_VILLAGE) ) {
							//캐릭터 사운드
							CharPlaySound( this );
						//}
					}
					else {
						//가끔 내는 몬스터 소리
						if ( smCharInfo.State==smCHAR_STATE_ENEMY && MotionInfo->State==CHRMOTION_STATE_STAND && (rand()%4)==0 )
							CharPlaySound( this );
					}

/*
					//다시 시작 이펙트
					if ( MotionInfo->State==CHRMOTION_STATE_DAMAGE )
						//피튀기는 파티클
						StartEffect( pX, pY+18*fONE, pZ, 50,50 , EFFECT_DAMAGE1 );
*/
					//if ( AttackSkil && !MotionEvent && (OldMotionState==CHRMOTION_STATE_ATTACK || OldMotionState==CHRMOTION_STATE_SKILL) ) {
					if ( !MotionEvent && (OldMotionState==CHRMOTION_STATE_ATTACK || OldMotionState==CHRMOTION_STATE_SKILL) ) {
						if ( MotionInfo->EventFrame[0] ) {
							EventAttack();				//스킵된 이벤트 실행
							//PlayAttack(TRUE);									//공격 적용
							dwAttackPlayTime = 0;		//원거리 캐릭의 공격은 제외
							ResetAttackTrans();		//AttackTrans.code = 0;
						}
						PlayAttack();									//공격 적용
					}
					MotionEvent = 0;

					if ( playBuff->wStickItems[smPLAYBUFF_ITEM_SKIL]>=0 ) {
						//스킬시작
						BeginSkill( playBuff->wStickItems[smPLAYBUFF_ITEM_SKIL] , 0 , chrAttackTarget , 0,0,0 );
						//AttackSkil = playBuff->wStickItems[smPLAYBUFF_ITEM_SKIL];
					}

					if ( smCharInfo.State==smCHAR_STATE_ENEMY ) {
						if ( MotionInfo->State==CHRMOTION_STATE_SKILL )
							BeginSkill( 0 , 0 , chrAttackTarget , 0,0,0 );

						if ( MotionInfo->State==CHRMOTION_STATE_ATTACK )
							BeginAttack_Monster();

						if ( OldMotionState==CHRMOTION_STATE_SKILL ) {
							PlayAttack();									//공격 적용
						}

					}

					switch(MotionInfo->State) {
						case CHRMOTION_STATE_RESTART:
							StartEffect( pX,pY,pZ,EFFECT_GAME_START1 );
							SetDynLight( pX,pY,pZ, 100, 100,100,0, 200 );
							CharPlaySound( this );
							break;
						case CHRMOTION_STATE_WARP:
							if ( smCharInfo.dwCharSoundCode==snCHAR_SOUND_WOLVERLIN ) {
								//순간 이동 이펙트
								StartEffect( pX,pY+48*fONE,pZ, EFFECT_RETURN1 );
								SkillPlaySound( SKILL_SOUND_LEARN , pX,pY,pZ );			//스킬 효과음
								SetDynLight( pX,pY,pZ, 50, 100,70,0, 300 , 1 );
								CharPlaySound( this );
							}
							break;

						case CHRMOTION_STATE_DEAD:			//캐릭 죽음동작 시작
							FrameCounter = 0;
							switch( smCharInfo.dwCharSoundCode ) 
							{
							case snCHAR_SOUND_DEADZONE:
								AssaParticle_MonsterDeadZoneDead(this);
								break;
							case snCHAR_SOUND_HYPERMACHINE:
								AssaParticle_MonsterHyperDead(this);
								break;
							case snCHAR_SOUND_MYSTIC:
								ParkAssaParticle_MisticSpinel(this);
								break;
							case snCHAR_SOUND_DARKMAGE:
								ParkAssaParticle_DarkMageDead(this);
								break;
							}
							break;
					}
				}
			}
			else {
				frame += playBuff->frame&0xFFFF;
				//frame += 80;
			}

/*
			if ( PatDispMode==DISP_MODE_PATSUB && MotionInfo->State<0x100  ) {

				compFrame = frame / 160;

			}
*/

			AttackSkil = playBuff->wStickItems[smPLAYBUFF_ITEM_SKIL];
			EventAttack();			//공격 이벤트

			if( smCharInfo.State!=smCHAR_STATE_USER && 
				( MotionInfo->State==CHRMOTION_STATE_WALK ||  MotionInfo->State==CHRMOTION_STATE_RUN ) ) {
				//발자국 소리 ( 바곤 및 거대 몬스터 )
				if ( MotionInfo->EventFrame[0] ) {
					compFrame = frame-(MotionInfo->StartFrame*160);
					if ( MotionInfo->EventFrame[0]>(compFrame-80) && MotionInfo->EventFrame[0]<=compFrame ) {
						CharPlaySound( this );
					}
				}

				if ( MotionInfo->EventFrame[1] ) {
					compFrame = frame-(MotionInfo->StartFrame*160);
					if ( MotionInfo->EventFrame[1]>(compFrame-80) && MotionInfo->EventFrame[1]<=compFrame ) {
						CharPlaySound( this );
					}
				}
			}

			FrameStep = 80;

			if ( MotionInfo->State==CHRMOTION_STATE_EAT ) {
				//물약 먹기 동작
				if ( MotionInfo->EventFrame[0] ) {
					compFrame = frame-(MotionInfo->StartFrame*160);
					if ( MotionInfo->EventFrame[0]>(compFrame-FrameStep) && MotionInfo->EventFrame[0]<=compFrame ) {
						if ( smCharInfo.State==smCHAR_STATE_USER ) 
							cnt=pY+48*fONE;
						else
							cnt=pY+PatHeight;

						switch( PotionLog ) {
						case 1:
							StartEffect( pX,cnt,pZ, EFFECT_POTION1 );
							sinPlaySound(SIN_SOUND_EAT_POTION,GetDistVolume(pX,cnt,pZ) );			//물약먹는 소리
							break;
						case 2:
							StartEffect( pX,cnt,pZ, EFFECT_POTION2 );
							sinPlaySound(SIN_SOUND_EAT_POTION,GetDistVolume(pX,cnt,pZ) );			//물약먹는 소리
							break;
						case 3:
							StartEffect( pX,cnt,pZ, EFFECT_POTION3 );
							sinPlaySound(SIN_SOUND_EAT_POTION,GetDistVolume(pX,cnt,pZ) );			//물약먹는 소리
							break;
						case 4:
							StartEffect( pX,cnt,pZ, EFFECT_POTION3 );
							StartEffect( pX,cnt,pZ, EFFECT_POTION2 );
							StartEffect( pX,cnt,pZ, EFFECT_POTION1 );
							sinPlaySound(SIN_SOUND_EAT_POTION,GetDistVolume(pX,cnt,pZ) );			//물약먹는 소리
							break;

						case POTION_ETHERCORE:
							StartEffect( pX,pY+48*fONE,pZ, EFFECT_RETURN1 );
							SkillPlaySound( SKILL_SOUND_LEARN , pX,cnt,pZ );			//스킬 효과음
						}
						SetDynLight( pX,cnt,pZ, 50, 100,70,0, 250 , 1 );
					}
				}
			}

			compFrame = frame / 160;

			if ( compFrame<MotionInfo->StartFrame )
				frame = MotionInfo->StartFrame*160;


			if ( compFrame>=MotionInfo->EndFrame ) {
				if ( MotionInfo->Repeat ) {
					ChangeMotion( action , AnimDispMode );
				}
				else 
					frame = (MotionInfo->EndFrame)*160;

				AttackEffect = 0;
			}

/*
			if ( compFrame>=smMotionInfo->MotionInfo[action].EndFrame ) {
				if ( smMotionInfo->MotionInfo[action].Repeat ) {
					ChangeMotion( action );
				}
				else 
					frame = (smMotionInfo->MotionInfo[action].EndFrame)*160;
			}
*/

			////// 장착한 아이템 코드 확인하여 변경 사항 적용 ////////////
			if ( ( smCharInfo.State==smCHAR_STATE_USER || PkMode_CharState==smCHAR_STATE_USER ) && DisplayFlag ) {

				// 왼쪽 장착 아이템
				if ( playBuff->wStickItems[smPLAYBUFF_ITEM_LEFT]!=wStickItems[smPLAYBUFF_ITEM_LEFT] ) {
					if ( playBuff->wStickItems[smPLAYBUFF_ITEM_LEFT]>=0 ) {
						SetTool( sItem[playBuff->wStickItems[smPLAYBUFF_ITEM_LEFT]].CODE , hvPOSI_LHAND );
						HvLeftHand.ColorBlink = 0;
						HvLeftHand.DispEffect = 0;
					}
					else
						SetTool(  0 , hvPOSI_LHAND );
				}
				// 오른쪽 장착 아이템
				if ( playBuff->wStickItems[smPLAYBUFF_ITEM_RIGHT]!=wStickItems[smPLAYBUFF_ITEM_RIGHT] ) {
					if ( playBuff->wStickItems[smPLAYBUFF_ITEM_RIGHT]>=0 ) {
						SetTool( sItem[playBuff->wStickItems[smPLAYBUFF_ITEM_RIGHT]].CODE , hvPOSI_RHAND );
						HvRightHand.ColorBlink = 0;
						HvRightHand.DispEffect = 0;
					}
					else
						SetTool(  0 , hvPOSI_RHAND );
				}
				// 몸 장착 아이템( 갑옷 등 )
				if ( playBuff->wStickItems[smPLAYBUFF_ITEM_BODY]!=wStickItems[smPLAYBUFF_ITEM_BODY] ) {
					SetChrArmor( this , playBuff->wStickItems[smPLAYBUFF_ITEM_BODY] );
					wStickItems[smPLAYBUFF_ITEM_BODY] = playBuff->wStickItems[smPLAYBUFF_ITEM_BODY];
				}
			}

			

			PlayBuffCnt++;
			MoveCnt = 0;

			//분노치 변화
			//if ( (( MotionInfo->State==CHRMOTION_STATE_ATTACK || MotionInfo->State==CHRMOTION_STATE_DAMAGE )&& 
			//	PatDispMode==DISP_MODE_PATSUB) ) {
			if ( (( MotionInfo->State==CHRMOTION_STATE_ATTACK || MotionInfo->State==CHRMOTION_STATE_DAMAGE )&& 
				smCharInfo.dwCharSoundCode==snCHAR_SOUND_MINIG) ) {
				AttackAnger+=2;
				if ( AttackAnger>100 ) AttackAnger=100;
			}
			else {
				AttackAnger-=1;
				if ( AttackAnger<=0 ) AttackAnger=0;
			}

			if ( AttackIce!=0 ) {
				//얼음 공격
				if ( AttackIce>0 && AttackIce<160 ) AttackIce ++;
				if ( AttackIce<0 ) AttackIce++;
				if ( MotionInfo->State==CHRMOTION_STATE_DEAD ) AttackIce = 0;
			}

			if ( DispPoison!=0 ) {
				//독에 감염
				if ( DispPoison>0 && DispPoison<100 ) DispPoison ++;
				if ( DispPoison<0 ) DispPoison++;
				if ( MotionInfo->State==CHRMOTION_STATE_DEAD ) DispPoison = 0;
			}

			if ( PlayDistortion>0 ) {
				//왜곡 기능
				PlayDistortion --;
			}
				
			//바닥 확인용
			if ( (PlayBuffCnt&0x3F)==0 && smCharInfo.State!=TRUE ) {
				OnStageField = -1;
				if ( smGameStage[0] ) {
					cnt = smGameStage[0]->GetFloorHeight( pX,pY,pZ , Pattern->SizeHeight );	
					OnStageField = 0;
				}
				if ( cnt==CLIP_OUT && smGameStage[1] ) {
					OnStageField = 1;
				}
			}

			if ( dwAttackPlayTime && dwAttackPlayTime<dwPlayTime ) {
				//공격 시간예약 적용
				PlayAttack( TRUE );
				dwAttackPlayTime = 0;
			}


			OldMotionState = MotionInfo->State;


			//게임 데이타 전송이 처음 되었을때 캐릭터 화면에 표시
			DisplayFlag = TRUE;		
		}

		if ( smCharInfo.dwCharSoundCode==snCHAR_SOUND_WOLVERLIN && smCharInfo.State==smCHAR_STATE_ENEMY ) {
			if ( (Counter&0x1)==0 && MotionInfo->State!=CHRMOTION_STATE_DEAD && MotionInfo->State==CHRMOTION_STATE_RUN ) {
				GetMoveLocation( 0 , 0, ((rand()%48)-24)*fONE , 0, Angle.y , 0 );
				sinSetEffect( SIN_PARTICLE_SPOUT ,  pX+GeoResult_X,pY+30*fONE,pZ+GeoResult_Z , 1000 , SIN_PARTICLE_COLOR_GOLD );
			}
			if ( MotionInfo->State==CHRMOTION_STATE_DEAD ) {
				if ( sAddColorEffect[SMC_R]>0 ) sAddColorEffect[SMC_R]--;
				if ( sAddColorEffect[SMC_G]>0 ) sAddColorEffect[SMC_G]--;
				if ( sAddColorEffect[SMC_B]>0 ) sAddColorEffect[SMC_B]--;
			}
		}

		if ( smCharInfo.wPlayClass[0]==MONSTER_CLASS_BOSS ) {
			//보스몬스터 죽음
			if ( MotionInfo->State==CHRMOTION_STATE_DEAD ) {
				if ( sAddColorEffect[SMC_R]>0 ) sAddColorEffect[SMC_R]--;
				if ( sAddColorEffect[SMC_G]>0 ) sAddColorEffect[SMC_G]--;
				if ( sAddColorEffect[SMC_B]>0 ) sAddColorEffect[SMC_B]--;
			}
			/*
			else {
				if ( RendSucess && RendPoint.z>0 && RendPoint.z<64*14*fONE && (Counter&1)==0 ) {
					sinSetEffect( SIN_PARTICLE_ROTATE_RISE ,  pX,pY ,pZ , 1000 , 0 , 1500+(smCharInfo.SizeLevel*500) );
				}
			}
			*/
		}

		if ( smCharInfo.wPlayClass[1] ) {
			if ( smCharInfo.dwCharSoundCode==snCHAR_SOUND_HOBGOBLIN && smCharInfo.Level==60 ) {
				//풍선같은 산타 고블린 (맞을수록 커진다)
				cnt = ((smCharInfo.Life[1]-smCharInfo.Life[0])*460)/smCharInfo.Life[1];
				cnt += 120;
				smCharInfo.wPlayClass[1] = cnt;
			}
		}
		
		// 박재원 - 미리 모리프 이벤트(모리프를 사냥하면 모리프가 점점 커지고 미니 모리프가 나온다)
		if ( smCharInfo.dwCharSoundCode==snCHAR_SOUND_NPC_MORIF && smCharInfo.Level==60 ) {
			//풍선같이 점점 커지는 모리프 (맞을수록 커진다)
			cnt = ((smCharInfo.Life[1]-smCharInfo.Life[0])*700)/smCharInfo.Life[1];
			cnt += 300;
			smCharInfo.wPlayClass[1] = cnt;
		}

		if ( smCharInfo.Life[0]!=DispLifeBar ) {
			if ( DispLifeBar<smCharInfo.Life[0] ) DispLifeBar++;
			else DispLifeBar--;
		}

		if ( MotionInfo->State == CHRMOTION_STATE_WARP ) {
			if ( FrameCounter>10 && smCharInfo.dwCharSoundCode==snCHAR_SOUND_WOLVERLIN ) {
				if ( sAddColorEffect[SMC_A]>-256 ) sAddColorEffect[SMC_A]-=8;
				if ( sAddColorEffect[SMC_R]>0 ) sAddColorEffect[SMC_R]-=8;
				if ( sAddColorEffect[SMC_G]>0 ) sAddColorEffect[SMC_G]-=8;
				if ( sAddColorEffect[SMC_B]>0 ) sAddColorEffect[SMC_B]-=8;
				DisplayAlpha = -16;
			}
		}

		//무기 속성 이펙트
		if ( dwWeaponEffectTime ) {
			if ( dwWeaponEffectTime<dwPlayTime ) {
				dwWeaponEffectTime = 0;
				WeaponEffect = 0;
			}
		}

		if ( PlayVanish>0 ) PlayVanish--;
		if ( PlayInvincible>0 )	PlayInvincible--;		//무적 아이템 사용중

		//클랜 정보 업데이트 확인
		if ( smCharInfo.ClassClan && ClanInfoNum<0 && dwClanInfoTime && (dwClanInfoTime+60*1000)<dwPlayTime ) {
			//클랜원인데 클랜정보가 업데이트 되지 않은경우 1 분 마다 재 시도
			ClanInfoNum = ReadClanInfo( smCharInfo.ClassClan );
			dwClanInfoTime = dwPlayTime;
		}
		break;

	case 101:		//확대 캐릭터

			frame += 80;
			compFrame = frame / 160;

			if ( compFrame>=MotionInfo->EndFrame ) {
				SetMotionFromCode( CHRMOTION_STATE_STAND );

//				ChangeMotion( action , AnimDispMode );
/*
				if ( MotionInfo->Repeat ) {
					ChangeMotion( action , AnimDispMode );
				}
				else 
					frame = (MotionInfo->EndFrame)*160;
*/
			}

			//######################################################################################
			//작 성 자 : 오 영 석
			if( TalkFrame )
			{
				if( TalkFrame >= lpTalkMotionInfo->EndFrame * 160 )
					TalkFrame = 0;
				else
					TalkFrame += 80;
			}
			//######################################################################################
			break;

	}

	if ( ShootingFlag ) {
		//발사형 무기 메인함수
		ShootingMain();
	}

	if ( DisplayAlpha<0 ) {
		DisplayAlpha+=4;
		if (DisplayAlpha>0) DisplayAlpha=0;
	}
	if ( DisplayAlpha>0 ) {
		DisplayAlpha-=4;
		if (DisplayAlpha<0) DisplayAlpha=0;
	}


	Counter++;
	FrameCounter++;

	return TRUE;
}

#define VIEW_HIGH_DIST	36864			//(64*3)		
#define VIEW_MID_DIST	102400			//(64*5)		


int bb_r, bb_g,bb_b,bb_a;

int SetRenderBlinkColor( smCHARTOOL *smCharTool )
{
	int BlinkMsk;
	int BlinkTime;
	int bTime;
	int Blink,r,g,b,a;

	bb_r = smRender.Color_R;
	bb_g = smRender.Color_G;
	bb_b = smRender.Color_B;
	bb_a = smRender.Color_A;

	if ( smCharTool->ColorBlink==0 ) return NULL;

	Blink = smCharTool->ColorBlink;
	r = smCharTool->sColors[SMC_R];
	g = smCharTool->sColors[SMC_G];
	b = smCharTool->sColors[SMC_B];
	a = smCharTool->sColors[SMC_A];

	if ( Blink<0 ) {
		smRender.Color_R += r;
		smRender.Color_G += g;
		smRender.Color_B += b;
		smRender.Color_A += a;

		return NULL;
	}

	BlinkTime = (1<<Blink);
	BlinkMsk = BlinkTime-1;

	bTime = dwPlayTime&BlinkMsk;

	if ( dwPlayTime&BlinkTime ) {
		smRender.Color_R += (bTime*r)>>Blink;
		smRender.Color_G += (bTime*g)>>Blink;
		smRender.Color_B += (bTime*b)>>Blink;
		if ( a<0 ) smRender.Color_A += (bTime*a)>>Blink;
	}
	else {
		bTime = BlinkMsk-bTime;
		smRender.Color_R += (bTime*r)>>Blink;
		smRender.Color_G += (bTime*g)>>Blink;
		smRender.Color_B += (bTime*b)>>Blink;
		if ( a<0 ) smRender.Color_A += (bTime*a)>>Blink;
	}

	if ( a>0 ) 
		smRender.Color_A -= a;

	bTime = (bTime*fONE)>>Blink;
	if ( !bTime ) bTime =1;

	if ( smCharTool->TexMixCode>=0 ) {
		SetItem2PassTexture( smCharTool->TexMixCode , smCharTool->TexScroll );
	}

	return  bTime;
}

int RestoreRenderBlinkColor()
{
	smRender.Color_R = bb_r;
	smRender.Color_G = bb_g;
	smRender.Color_B = bb_b;
	smRender.Color_A = bb_a;

	SetItem2PassTexture( -1 );

	return TRUE;
}

//가상으로 그린다 ( 실제로는 그리지 않고 값만 세팅 )
int smCHAR::VirtualDraw()
{

	POINT	sPos;
	POINT3D CameraPosi , CameraAngle;
	int		He;

	//패턴 유무 체크 ( 통신시 파일 모델수신이 안된경우 )
	if ( !DisplayFlag || !MotionInfo ) return FALSE;

	//수신시간 확인하여 3초 초과 캐릭터 화면 비표시
	if 	( ActionPattern==99 ) {
		if ( LastPlayDataType ) {
			//원거리 교신 ( 데이타의 정확도가 떨어짐 )
			if ((dwLastTransTime+DISPLAY_TRANS_TIME_OVER_FAR)<dwPlayTime ) {
				dwDispAppSkill = 0;
				return TRUE;
			}
		}
		else {
			//근거리 교신
			if ((dwLastTransTime+DISPLAY_TRANS_TIME_OVER)<dwPlayTime ) {
				dwDispAppSkill = 0;
				return TRUE;
			}
		}
	}
	CameraPosi.x = smCHAR_CameraX;
	CameraPosi.y = smCHAR_CameraY;
	CameraPosi.z = smCHAR_CameraZ;

	CameraAngle.x = smCHAR_CameraAngX;
	CameraAngle.y = smCHAR_CameraAngY;
	CameraAngle.z = smCHAR_CameraAngZ;

	He = PatHeight;
	if ( He<44*fONE ) He=44*fONE;

	smRender.SetCameraPosi( CameraPosi.x,CameraPosi.y,CameraPosi.z,CameraAngle.x,CameraAngle.y,CameraAngle.z );
	RendPoint.z = smRender.GetRect2D( pX , pY+(He>>1), pZ , 44*fONE , He , &RendRect2D , &sPos );
	RendPoint.x = sPos.x;
	RendPoint.y = sPos.y;

	if ( RendPoint.z>0 && sPos.x>0 && sPos.x<WinSizeX && sPos.y>0 && sPos.y<WinSizeY ) {
		RendSucess = TRUE;
		return TRUE;
	}

	return FALSE;
}

int smCHAR::Draw()
{

	POINT3D CameraPosi , CameraAngle;
	POINT3D angle;
	smPAT3D *pat;
	POINT	sPos;
	POINT3D	sPos3;
	int		He;

	int dx,dy,dz , dDist;
	_MODELGROUP	*lpModelGroup;

	int	BlinkFlag = 0;
	smMATRIX	Mat;
	int		BlinkLevel;
	int		BlinkScale;


	RendSucess = 0;
	FlagShow = 0;

	//패턴 유무 체크 ( 통신시 파일 모델수신이 안된경우 )
	if ( !DisplayFlag || !MotionInfo ) return FALSE;

	//모델데이타 삭제된 오류
	if ( lpDinaPattern->UseCount<=0 ) {
		Close();
		if ( smConfig.DebugMode ) AddChatBuff( "\270\360\265\250\301\246\260\305 \277\300\267\371" );
		return FALSE;
	}


	//수신시간 확인하여 3초 초과 캐릭터 화면 비표시
	if 	( ActionPattern==99 ) {
		if ( LastPlayDataType ) {
			//원거리 교신 ( 데이타의 정확도가 떨어짐 )
			if ((dwLastTransTime+DISPLAY_TRANS_TIME_OVER_FAR)<dwPlayTime ) {
				dwDispAppSkill = 0;
				return TRUE;
			}
		}
		else {
			//근거리 교신
			if ((dwLastTransTime+DISPLAY_TRANS_TIME_OVER)<dwPlayTime ) {
				dwDispAppSkill = 0;
				return TRUE;
			}
		}
	}

	FlagShow = TRUE;

	dx = (smCHAR_CameraX - pX)>>FLOATNS;
	dy = (smCHAR_CameraY - pY)>>FLOATNS;
	dz = (smCHAR_CameraZ - pZ)>>FLOATNS;

	dDist = dx*dx+dy*dy+dz*dz;

	CameraPosi.x = smCHAR_CameraX;
	CameraPosi.y = smCHAR_CameraY;
	CameraPosi.z = smCHAR_CameraZ;

	CameraAngle.x = smCHAR_CameraAngX;
	CameraAngle.y = smCHAR_CameraAngY;
	CameraAngle.z = smCHAR_CameraAngZ;

	/*
		if ( GetProjPosi( x,y,z , &CameraPosi , &CameraAngle )==TRUE ) {
			smSayBox.DrawText( 0 , x , y , z );
		}
	*/

	memcpy( &angle, &Angle , sizeof( POINT3D ) );
	angle.y = (-angle.y+ANGLE_180)&ANGCLIP;

	if ( ActionGameMode ) {
		if ( angle.y>ANGLE_180 )
			angle.y = ANGLE_270;
		else
			angle.y = ANGLE_90;
	}

	Posi.x = pX;
	Posi.y = pY;
	Posi.z = pZ;


	Pattern->SetPosi( &Posi , &angle );

	//프레임 확인 ( 없는 캐릭터의 프레임 오류 처리를 위해 )
	if ( Pattern->TmParent && Pattern->TmParent->MaxFrame>(int)frame ) 
		Pattern->Frame = frame;
	else
		Pattern->Frame = 80;			//프레임 오류

	char *ModelNameList[4];

	int	cr,cg,cb,ca,cnt,sR,sG,sB,sA;
	int nl=0;

	cr = smRender.Color_R;
	cg = smRender.Color_G;
	cb = smRender.Color_B;
	ca = smRender.Color_A;
/*
		if ( smCharInfo.wPlayClass[0] ) {
			//보스 몬스터
			if ( smCharInfo.State==smCHAR_STATE_ENEMY ) {
				sAddColorEffect[SMC_R] = 100;
				sAddColorEffect[SMC_G] = 50;
				sAddColorEffect[SMC_B] = 70;
			}
		}
*/

	//일반 몬스터
	sR = sAddColorEffect[SMC_R];
	sG = sAddColorEffect[SMC_G];
	sB = sAddColorEffect[SMC_B];
	sA = sAddColorEffect[SMC_A];

	if ( smCharInfo.wPlayClass[0]==MONSTER_CLASS_BOSS ) {
		//보스 몬스터
		cnt = Counter&0x1F;
		if ( (Counter&0x20)==0 ) {
			cnt = 0x1F-cnt;
		}
		sR = (sR*cnt)>>5;
		sG = (sG*cnt)>>5;
		sB = (sB*cnt)>>5;
	}


	smRender.Color_R = cr+AttackAnger+sR;
	smRender.Color_G = cg-(AttackAnger>>1)+sG+abs(DispPoison);
	smRender.Color_B = cb+abs(AttackIce)+sB;
	smRender.Color_A = ca+DisplayAlpha-(abs(AttackIce)>>1)+sA;

	if ( PlayCurseQuest ) {
		//퀘스트 저주
		smRender.Color_R += 100;
		smRender.Color_B += 100;
	}
	if ( PlayDistortion && MotionInfo->State!=CHRMOTION_STATE_DEAD ) {
		//디스토션 걸리면 색깔 점멸
		//cnt = dwPlayTime>>7;
		smRender.DistortionMesh = TRUE;
		smRender.Color_R += 80;
		smRender.Color_G += 80;
	}


	if ( smCharInfo.wPlayClass[1] ) {
		//크기 키우기
		smRender.ObjectVertexScale = smCharInfo.wPlayClass[1];
	}

	if ( DarkLevel>0 && OnStageField>=0 && StageField[OnStageField]->State==FIELD_STATE_VILLAGE ) {
		//마을에서는 동적조명 적용 안되도록 강제 설정
		nl = DarkLevel>>1;
		smRender.Color_R += nl;
		smRender.Color_G += nl;
		smRender.Color_B += nl;

		if ( smRender.nRendLight>0 && smRender.smRendLight[0].Range==DarkLightRange ) {
			smRender.smRendLight[0].Range = 0;
			nl = 1;
		}
		else
			nl = 0;
	}


	if ( MotionInfo->State==CHRMOTION_STATE_DEAD ) {
		if ( FrameCounter>400 ) {
			cnt = 100;
			smRender.Color_R -= cnt;
			smRender.Color_G -= cnt;
			smRender.Color_B -= cnt;
		}
		else {
			if ( FrameCounter>200 ) {
				cnt = (FrameCounter-200)>>1;
				smRender.Color_R -= cnt;
				smRender.Color_G -= cnt;
				smRender.Color_B -= cnt;

			}
		}

		if ( smCharInfo.dwCharSoundCode==snCHAR_SOUND_FIGON || smCharInfo.dwCharSoundCode==snCHAR_SOUND_STYGIANLORD || 
			smCharInfo.dwCharSoundCode==snCHAR_SOUND_GUARDIAN_SAINT || smCharInfo.dwCharSoundCode==snCHAR_SOUND_S_F_ELEMENTAL ||
			smCharInfo.dwCharSoundCode==snCHAR_SOUND_CHAOSCARA ) {

			//파이곤 죽을때 서서히 사라짐
			smRender.Color_A = ca-(FrameCounter<<1);
			if ( smRender.Color_A<-400 ) goto rendSkip;
			DisplayAlpha = 100;
		}
	}


	//거리별 메쉬 정밀도 변경해 가며 렌더링
	lpModelGroup = &smMotionInfo->DefaultModel;

	if ( dDist<VIEW_HIGH_DIST && smMotionInfo->HighModel.ModelNameCnt  ) 
		lpModelGroup = &smMotionInfo->HighModel;

	if ( dDist>VIEW_MID_DIST && smMotionInfo->LowModel.ModelNameCnt  ) 
		lpModelGroup = &smMotionInfo->LowModel;
/*
	//프레임 범위 벗어나는지 확인하기 ( 디버그용 )
	if ( frame<(MotionInfo->StartFrame*160) || frame>(MotionInfo->EndFrame*160) ) {
		cnt = cnt;
	}
*/
	//######################################################################################
	//작 성 자 : 오 영 석
	if( TalkFrame ) // && (PatDispMode & DISP_MODE_PATTALK) )
	{
		if( TalkPattern->MaxFrame > (int)TalkFrame )
			TalkPattern->Frame = TalkFrame;
		else
			TalkFrame = 0;	// 프레임 오류
	}
	//######################################################################################

	if ( PatDispMode&DISP_MODE_PATMAIN ) {

		//프레임 캐시 초기화 ( 안그러면 캐릭터 죽을때 찌그러짐 )
		if ( Pattern->TmParent ) Pattern->TmParent->TmLastFrame = -1;
		else Pattern->TmLastFrame = -1;

		if ( lpModelGroup->ModelNameCnt ) {
			ModelNameList[0] = lpModelGroup->szModelName[0];
			ModelNameList[1] = lpModelGroup->szModelName[1];
			ModelNameList[2] = lpModelGroup->szModelName[2];
			ModelNameList[3] = lpModelGroup->szModelName[3];
			//######################################################################################
			//작 성 자 : 오 영 석
			if( TalkFrame && lpTalkMotionInfo->State )
				RendSucess += Pattern->ChoiseRenderD3D( &CameraPosi, &CameraAngle, ModelNameList, lpModelGroup->ModelNameCnt, TalkPattern, &FrameInfo );
			else
                RendSucess += Pattern->ChoiseRenderD3D( &CameraPosi, &CameraAngle, ModelNameList, lpModelGroup->ModelNameCnt, &FrameInfo );
			//######################################################################################			
		}
		//######################################################################################
		//작 성 자 : 오 영 석
		else
		{
            if( TalkFrame && lpTalkMotionInfo->State )
				RendSucess += Pattern->RenderD3D( &CameraPosi, &CameraAngle, TalkPattern, &FrameInfo );
			else
				RendSucess += Pattern->RenderD3D( &CameraPosi, &CameraAngle, &FrameInfo );
		}
		//######################################################################################
	}

	//머리및 보조 패턴 출력
	if ( lpDinaPattern2 && Pattern2 && PatDispMode&DISP_MODE_PATSUB ) {

		lpModelGroup = &lpDinaPattern2->lpModelInfo->DefaultModel;

		if ( dDist<VIEW_HIGH_DIST && lpDinaPattern2->lpModelInfo->HighModel.ModelNameCnt  ) 
			lpModelGroup = &lpDinaPattern2->lpModelInfo->HighModel;

		if ( dDist>VIEW_MID_DIST && lpDinaPattern2->lpModelInfo->LowModel.ModelNameCnt  ) 
			lpModelGroup = &lpDinaPattern2->lpModelInfo->LowModel;


		Pattern2->SetPosi( &Posi , &angle );
		if ( Pattern2->TmParent && Pattern2->TmParent->MaxFrame>(int)frame ) 
			Pattern2->Frame = frame;

		//프레임 캐시 초기화 ( 안그러면 캐릭터 죽을때 찌그러짐 )
		if ( Pattern2->TmParent ) Pattern2->TmParent->TmLastFrame = -1;
		else Pattern2->TmLastFrame = -1;

		if ( lpModelGroup->ModelNameCnt ) {
			ModelNameList[0] = lpModelGroup->szModelName[0];
			ModelNameList[1] = lpModelGroup->szModelName[1];
			ModelNameList[2] = lpModelGroup->szModelName[2];
			ModelNameList[3] = lpModelGroup->szModelName[3];
			//######################################################################################
			//작 성 자 : 오 영 석
			if( TalkFrame && lpTalkMotionInfo->State )
				RendSucess += Pattern2->ChoiseRenderD3D( &CameraPosi, &CameraAngle, ModelNameList, lpModelGroup->ModelNameCnt, TalkPattern, &FrameInfo );
			else
                RendSucess += Pattern2->ChoiseRenderD3D( &CameraPosi, &CameraAngle, ModelNameList, lpModelGroup->ModelNameCnt, &FrameInfo );
			//######################################################################################			
		}
		//######################################################################################
		//작 성 자 : 오 영 석
		else
		{
            if( TalkFrame && lpTalkMotionInfo->State )
				RendSucess += Pattern2->RenderD3D( &CameraPosi, &CameraAngle, TalkPattern, &FrameInfo );
			else
				RendSucess += Pattern2->RenderD3D( &CameraPosi, &CameraAngle, &FrameInfo );
		}
		//######################################################################################
	}

	smRender.ObjectVertexScale = 0;

	if ( RendSucess ) {

		int We = 44*fONE;
		He = PatHeight;

		if ( smCharInfo.wPlayClass[1] ) {
			//크기 키우기
			He = (He*smCharInfo.wPlayClass[1])>>FLOATNS;
		}

		if ( smCharInfo.dwCharSoundCode==snCHAR_SOUND_KELVEZU ) {
			We = He*2;
		}

		if ( He<44*fONE ) He=44*fONE;

		smRender.SetCameraPosi( CameraPosi.x,CameraPosi.y,CameraPosi.z,CameraAngle.x,CameraAngle.y,CameraAngle.z );
		//RendPoint.z = smRender.GetRect2D( pX , pY+(24*fONE), pZ , 32*fONE , 40*fONE , &RendRect2D , &sPos );
		RendPoint.z = smRender.GetRect2D( pX , pY+(He>>1), pZ , We , He , &RendRect2D , &sPos );
		RendPoint.x = sPos.x;
		RendPoint.y = sPos.y;
	}


	if ( !Pattern->TmParent || !DisplayTools ) {
		smRender.Color_R = cr;
		smRender.Color_G = cg;
		smRender.Color_B = cb;
		if ( nl ) smRender.smRendLight[0].Range=DarkLightRange;
		return TRUE;
	}


	//무기및 부착 물품 출력

//  if ( !HideWeapon ) {

	if ( MotionInfo->State==CHRMOTION_STATE_ATTACK && dwPlayTime & 0x20 ) 
		BlinkFlag = TRUE;

	if ( BlinkFlag && !ShootingMode ) {
		smRender.Color_R +=80;
		smRender.Color_G +=80;
		smRender.Color_B +=80;
	}

	Rend_HvLeftHand = TRUE;			//무기 렌더링 유무
	Rend_HvRightHand = TRUE;		//무기 렌더링 유무

  if ( smCharInfo.State==smCHAR_STATE_USER || PkMode_CharState==smCHAR_STATE_USER ) {

	if ( dwItemSetting==0 || (OnStageField>=0 && StageField[OnStageField]->State!=FIELD_STATE_VILLAGE && MotionInfo->State!=CHRMOTION_STATE_YAHOO ) ) {
		if ( HvLeftHand.PatTool ) { //&& !HideWeapon ) {
			pat = HvLeftHand.PatTool;

			if ( (HvLeftHand.dwItemCode&sinITEM_MASK2)==sinDS1 ) {
				if ( (dwDispAppSkill&SKILL_APPMASK_SPARK_SHIELD) && PatSparkShield && smCharInfo.JOB_CODE==JOBCODE_MECHANICIAN) {
					if ( MotionInfo->State==CHRMOTION_STATE_SKILL && (AttackSkil&0xFF)==SKILL_PLAY_SPARK_SHIELD && MotionEvent==0 )
						pat = HvLeftHand.PatTool;
					else
						pat = PatSparkShield;
				}
				if ( (dwDispAppSkill&SKILL_APPMASK_SPARK_SHIELD) && PatDivineShield && smCharInfo.JOB_CODE==JOBCODE_KNIGHT) {
					if ( MotionInfo->State==CHRMOTION_STATE_SKILL && (AttackSkil&0xFF)==SKILL_PLAY_DIVINE_INHALATION && MotionEvent==0 )
						pat = HvLeftHand.PatTool;
					else
						pat = PatDivineShield;
				}
				if ( (dwDispAppSkill&SKILL_APPMASK_GODLY_SHIELD) && PatGodlyShield && smCharInfo.JOB_CODE==JOBCODE_KNIGHT) {
					if ( MotionInfo->State==CHRMOTION_STATE_SKILL && (AttackSkil&0xFF)==SKILL_PLAY_GODLY_SHIELD && MotionEvent==0 )
						pat = HvLeftHand.PatTool;
					else
						pat = PatGodlyShield;
				}
			}


			if ( MotionInfo->State==CHRMOTION_STATE_SKILL && 
				((AttackSkil&0xFF)==SKILL_PLAY_SHIELD_STRIKE || (AttackSkil&0xFF)==SKILL_PLAY_SOUL_SUCKER) &&
				ShieldObjBip ) {

				pat->LinkParentObject(  AnimPattern , ShieldObjBip );

				cr = smRender.Color_R;
				cg = smRender.Color_G;
				cb = smRender.Color_B;

				smRender.Color_R+=100;
				smRender.Color_G+=100;
				smRender.Color_B+=80;

			}
			else
				pat->LinkParentObject(  AnimPattern , HvLeftHand.ObjBip );

			pat->SetPosi( &Posi , &angle );
			pat->Frame = frame;
			//행렬 복사
			memcpy( &HvLeftHand_Matrix , &HvLeftHand.ObjBip->mWorld , sizeof(smMATRIX) );

			if ( !HvLeftHand.ColorBlink || !HvLeftHand.sColors[SMC_A] ) {
				//점멸색상 설정
				BlinkLevel = SetRenderBlinkColor( &HvLeftHand );

				//스케일 점멸
				if ( HvLeftHand.BlinkScale && BlinkLevel ) {
					smIdentityMatrix( Mat );
					if ( HvLeftHand.BlinkScale<0 ) 
						BlinkScale = -HvLeftHand.BlinkScale;
					else
						BlinkScale = (BlinkLevel*HvLeftHand.BlinkScale)>>FLOATNS;
					Mat._11 = BlinkScale;
					Mat._22 = BlinkScale;
					Mat._33 = BlinkScale;
					pat->Frame =  -1;
					memcpy( &pat->obj3d[0]->mWorld , &HvLeftHand.ObjBip->mWorld , sizeof(smMATRIX) );
					smMatrixMult( pat->obj3d[0]->mWorld , Mat , pat->obj3d[0]->mWorld );
				}

				pat->RenderD3D( &CameraPosi , &CameraAngle );
				RestoreRenderBlinkColor();		//색상복구

				//쉴드 스트라이크 방패에 붙이는 이펙트
				if ( MotionInfo->State==CHRMOTION_STATE_SKILL && (AttackSkil&0xFF)==SKILL_PLAY_SHIELD_STRIKE && ShieldObjBip ) {
//				((AttackSkil&0xFF)==SKILL_PLAY_SHIELD_STRIKE || (AttackSkil&0xFF)==SKILL_PLAY_SOUL_SUCKER) &&

					smRender.Color_R = cr;
					smRender.Color_G = cg;
					smRender.Color_B = cb;

					sPos3.x = pat->obj3d[0]->mWorld._41+pX;
					sPos3.z = pat->obj3d[0]->mWorld._42+pZ;
					sPos3.y = pat->obj3d[0]->mWorld._43+pY;
					sinEffect_Shield_Strike(&sPos3);
				}
			}
			else
				Rend_HvLeftHand = FALSE;

			pat->LinkParentObject( 0,0 );
			pat->TmLastFrame = -1;
		}

		//HideSkillWeapon
		//&& AttackSkil != SKILL_PLAY_MECHANIC_BOM 
		if ( HvRightHand.PatTool && !HideWeapon ) {
			if ( (dwActionItemCode&sinITEM_MASK2)==sinWT1 && ShootingFlag  ) {

			}
			else {
				//if ( MotionInfo->State!=CHRMOTION_STATE_EAT ) {	//물약 화면 깨짐 땜방
					pat = HvRightHand.PatTool;
					pat->LinkParentObject(  AnimPattern , HvRightHand.ObjBip );
					pat->SetPosi( &Posi , &angle );
					pat->Frame = frame;

					//행렬 복사
					memcpy( &HvRightHand_Matrix , &HvRightHand.ObjBip->mWorld , sizeof(smMATRIX) );

					if ( !HvRightHand.ColorBlink || !HvRightHand.sColors[SMC_A] ) {
						//점멸색상 설정
						BlinkLevel = SetRenderBlinkColor( &HvRightHand );

						//스케일 점멸
						if ( HvRightHand.BlinkScale && BlinkLevel ) {
							smIdentityMatrix( Mat );
							if ( HvRightHand.BlinkScale<0 ) 
								BlinkScale = -HvRightHand.BlinkScale;
							else
								BlinkScale = (BlinkLevel*HvRightHand.BlinkScale)>>FLOATNS;

							Mat._11 = BlinkScale;
							Mat._22 = BlinkScale;
							Mat._33 = BlinkScale;
							pat->Frame =  -1;
							memcpy( &pat->obj3d[0]->mWorld , &HvRightHand.ObjBip->mWorld , sizeof(smMATRIX) );
							smMatrixMult( pat->obj3d[0]->mWorld , Mat , pat->obj3d[0]->mWorld );
						}

						pat->RenderD3D( &CameraPosi , &CameraAngle );
						RestoreRenderBlinkColor();		//색상복구
					}
					else
						Rend_HvRightHand = FALSE;

					pat->LinkParentObject( 0,0 );
					pat->TmLastFrame = -1;
				//}
			}
		}
	}
	else {
		if ( dwActionItemCode ) {

			BlinkScale = 0;

			//등에 장착
			if ( HvLeftHand.dwItemCode==dwActionItemCode ) {
				pat = HvLeftHand.PatTool;
				if ( !HvLeftHand.ColorBlink || !HvLeftHand.sColors[SMC_A] ) {
					//점멸색상 설정
					BlinkLevel = SetRenderBlinkColor( &HvLeftHand );

					if ( HvLeftHand.BlinkScale && BlinkLevel ) {
						if ( HvLeftHand.BlinkScale<0 ) 
							BlinkScale = -HvLeftHand.BlinkScale;
						else
							BlinkScale = (BlinkLevel*HvLeftHand.BlinkScale)>>FLOATNS;
						memcpy( &pat->obj3d[0]->mWorld , &HvLeftHand.ObjBip->mWorld , sizeof(smMATRIX) );
					}
				}
				else
					Rend_HvLeftHand = FALSE;
			}
			if ( HvRightHand.dwItemCode==dwActionItemCode ) {
				pat = HvRightHand.PatTool;
				if ( !HvRightHand.ColorBlink || !HvRightHand.sColors[SMC_A] ) {
					//점멸색상 설정
					BlinkLevel = SetRenderBlinkColor( &HvRightHand );

					if ( HvRightHand.BlinkScale && BlinkLevel ) {
						if ( HvRightHand.BlinkScale<0 ) 
							BlinkScale = -HvRightHand.BlinkScale;
						else
							BlinkScale = (BlinkLevel*HvRightHand.BlinkScale)>>FLOATNS;

						memcpy( &pat->obj3d[0]->mWorld , &HvRightHand.ObjBip->mWorld , sizeof(smMATRIX) );
					}
				}
				else
					Rend_HvRightHand = FALSE;
			}

			pat->LinkParentObject( AnimPattern , BackObjBip[dwItemSetting-1] );
			pat->SetPosi( &Posi , &angle );
			pat->Frame = frame;

			//스케일 점멸
			if (BlinkScale ) {
				smIdentityMatrix( Mat );
				Mat._11 = BlinkScale;
				Mat._22 = BlinkScale;
				Mat._33 = BlinkScale;
				pat->Frame =  -1;
				smMatrixMult( pat->obj3d[0]->mWorld , Mat , pat->obj3d[0]->mWorld );
			}

			//행렬 복사
			if ( HvLeftHand.dwItemCode==dwActionItemCode ) {
				memcpy( &HvLeftHand_Matrix , &BackObjBip[dwItemSetting-1]->mWorld , sizeof(smMATRIX) );
				if ( Rend_HvLeftHand ) {
					pat->RenderD3D( &CameraPosi , &CameraAngle );
					RestoreRenderBlinkColor();
				}
			}
			else {
				memcpy( &HvRightHand_Matrix , &BackObjBip[dwItemSetting-1]->mWorld , sizeof(smMATRIX) );
				if ( Rend_HvRightHand ) {
					pat->RenderD3D( &CameraPosi , &CameraAngle );
					RestoreRenderBlinkColor();
				}
			}

			
			pat->LinkParentObject( 0,0 );
			pat->TmLastFrame = -1;
		}

		if ( HvLeftHand.PatTool &&  dwActionItemCode!=HvLeftHand.dwItemCode ) {
			pat = HvLeftHand.PatTool;
			pat->LinkParentObject(  AnimPattern , HvLeftHand.ObjBip );
			pat->SetPosi( &Posi , &angle );
			pat->Frame = frame;
			//행렬 복사
			memcpy( &HvLeftHand_Matrix , &HvLeftHand.ObjBip->mWorld , sizeof(smMATRIX) );

			if ( !HvLeftHand.ColorBlink || !HvLeftHand.sColors[SMC_A] ) {
				//점멸색상 설정
				BlinkLevel = SetRenderBlinkColor( &HvLeftHand );

				//스케일 점멸
				if ( HvLeftHand.BlinkScale && BlinkLevel ) {
					smIdentityMatrix( Mat );
					if ( HvLeftHand.BlinkScale<0 ) 
						BlinkScale = -HvLeftHand.BlinkScale;
					else
						BlinkScale = (BlinkLevel*HvLeftHand.BlinkScale)>>FLOATNS;

					Mat._11 = BlinkScale;
					Mat._22 = BlinkScale;
					Mat._33 = BlinkScale;
					pat->Frame =  -1;
					memcpy( &pat->obj3d[0]->mWorld , &HvLeftHand.ObjBip->mWorld , sizeof(smMATRIX) );
					smMatrixMult( pat->obj3d[0]->mWorld , Mat , pat->obj3d[0]->mWorld );
				}
				pat->RenderD3D( &CameraPosi , &CameraAngle );
				RestoreRenderBlinkColor();		//색상복구
			}
			else
				Rend_HvLeftHand = FALSE;

			pat->LinkParentObject( 0,0 );
			pat->TmLastFrame = -1;
		}

	}

	//화살 시위 당기기
	if ( ( MotionInfo->State==CHRMOTION_STATE_ATTACK || MotionInfo->State==CHRMOTION_STATE_SKILL )
		&& (dwActionItemCode&sinITEM_MASK2)==sinWS1 
		&& !HvRightHand.PatTool && !ShootingFlag && !HideWeapon) {
		pat = PatArrow;
		pat->LinkParentObject(  AnimPattern , HvRightHand.ObjBip );
		pat->SetPosi( &Posi , &angle );
		pat->Frame = frame;
		pat->RenderD3D( &CameraPosi , &CameraAngle );
		pat->LinkParentObject( 0,0 );
		pat->TmLastFrame = -1;
	}
   }
   
   // pluto 되살아난 아처 화살 시위 당기기
   //if( MotionInfo->State == CHRMOTION_STATE_ATTACK || MotionInfo->State == CHRMOTION_STATE_SKILL && smCharInfo.State==smCHAR_STATE_ENEMY && 
	  // smCharInfo.dwCharSoundCode == snCHAR_SOUND_REVIVED_ARCHER && !HvRightHand.PatTool && !ShootingFlag && !HideWeapon )
   //{
	  // pat = PatArrow;
	  // pat->LinkParentObject(  AnimPattern , HvRightHand.ObjBip );
	  // pat->SetPosi( &Posi , &angle );
	  // pat->Frame = frame;
	  // pat->RenderD3D( &CameraPosi , &CameraAngle );
	  // pat->LinkParentObject( 0,0 );
	  // pat->TmLastFrame = -1;
   //}

	//화살 날아가기
	if ( ShootingFlag ) {
		memcpy( &angle, &ShootingAngle , sizeof( POINT3D ) );
		angle.y = (-angle.y+ANGLE_180)&ANGCLIP;

		if ( (dwActionItemCode&sinITEM_MASK2)==sinWS1 ) {
			pat = PatArrow;
		}
		else {
			pat = HvRightHand.PatTool;
			angle.x = (angle.x-ANGLE_90)&ANGCLIP;
		}

		//angle.y = (-angle.y+ANGLE_90)&ANGCLIP;
		//angle.x = (angle.x+ANGLE_90)&ANGCLIP;
		//angle.x = (angle.x+ANGLE_90)&ANGCLIP;


		pat->SetPosi( &ShootingPosi , &angle );
		pat->Frame = 0;
		pat->RenderD3D( &CameraPosi , &CameraAngle );
	}
//  }

rendSkip:

	smRender.Color_R = cr;
	smRender.Color_G = cg;
	smRender.Color_B = cb;
	smRender.Color_A = ca;
	if ( nl ) smRender.smRendLight[0].Range=DarkLightRange;
	smRender.DistortionMesh = FALSE;

	return TRUE;
}

//그림자 그리기
int smCHAR::DrawShadow()
{
	int a;
	POINT3D	pos;
	POINT3D angle;
	POINT3D CameraPosi , CameraAngle;

	if ( PatSizeLevel<0 || !MotionInfo ) return FALSE;
#ifdef PT_GFX_GL
	if ( PtGfx_ShadowActive() ) return FALSE;			// real-time shadows drawn instead (ptgfx_shadow.cpp)
#endif

	CameraPosi.x = smCHAR_CameraX;
	CameraPosi.y = smCHAR_CameraY;
	CameraPosi.z = smCHAR_CameraZ;

	CameraAngle.x = smCHAR_CameraAngX;
	CameraAngle.y = smCHAR_CameraAngY;
	CameraAngle.z = smCHAR_CameraAngZ;

	if ( Pattern && PatShadow[PatSizeLevel] && MotionInfo->State!=CHRMOTION_STATE_RESTART &&
		RendSucess && RendPoint.z<12*64*fONE && PlayVanish<=0 ) {

		pos.x = pX;
		pos.y = PHeight+(1*fONE);	//pY;
		pos.z = pZ;


		angle.x = Angle.x;
		angle.y = (-Angle.y+ANGLE_180)&ANGCLIP;
		angle.z = Angle.z;

		a = smRender.Color_A;

		if ( MotionInfo->State==CHRMOTION_STATE_DEAD )
			smRender.Color_A = a-(FrameCounter<<1);			//죽을때 그림자 서서히 사라짐

		smRender.Color_A -=160;

		if ( smRender.Color_A>-240 ) {
		    lpD3DDevice->SetRenderState( D3DRENDERSTATE_ZWRITEENABLE, FALSE );
			smRender.SetMaterialGroup( smMaterialGroup );
			PatShadow[PatSizeLevel]->SetPosi( &pos, &angle );
			PatShadow[PatSizeLevel]->RenderD3D( &CameraPosi, &CameraAngle );
		    lpD3DDevice->SetRenderState( D3DRENDERSTATE_ZWRITEENABLE, TRUE );
		}

		smRender.Color_A = a;

	}

	//테스트
	smPAT3D *pat;
	smMATRIX Mat;
	int BlinkLevel , BlinkScale;



	if ( RendSucess && !HideWeapon ) {
		if (!Rend_HvRightHand) {
			if ( HvRightHand.ColorBlink ) {
				pat = HvRightHand.PatTool;
				pat->SetPosi( &Posi , &angle );
				pat->Frame = -1;
				//점멸색상 설정
				BlinkLevel = SetRenderBlinkColor( &HvRightHand );
				memcpy( &pat->obj3d[0]->mWorld , &HvRightHand_Matrix , sizeof(smMATRIX) );
				//스케일 점멸
				if ( HvRightHand.BlinkScale && BlinkLevel ) {
					smIdentityMatrix( Mat );

					if ( HvRightHand.BlinkScale<0 ) 
						BlinkScale = -HvRightHand.BlinkScale;
					else
						BlinkScale = (BlinkLevel*HvRightHand.BlinkScale)>>FLOATNS;

					Mat._11 = BlinkScale;
					Mat._22 = BlinkScale;
					Mat._33 = BlinkScale;
					smMatrixMult( pat->obj3d[0]->mWorld , Mat , pat->obj3d[0]->mWorld );
				}

				pat->RenderD3D( &CameraPosi , &CameraAngle );
				RestoreRenderBlinkColor();		//색상복구
			}
		}
		if (!Rend_HvLeftHand) {
			if ( HvLeftHand.ColorBlink ) {
				pat = HvLeftHand.PatTool;
				pat->SetPosi( &Posi , &angle );
				pat->Frame = -1;
				//점멸색상 설정
				BlinkLevel = SetRenderBlinkColor( &HvLeftHand );
				memcpy( &pat->obj3d[0]->mWorld , &HvLeftHand_Matrix , sizeof(smMATRIX) );
				//스케일 점멸
				if ( HvLeftHand.BlinkScale && BlinkLevel ) {
					smIdentityMatrix( Mat );

					if ( HvLeftHand.BlinkScale<0 ) 
						BlinkScale = -HvLeftHand.BlinkScale;
					else
						BlinkScale = (BlinkLevel*HvLeftHand.BlinkScale)>>FLOATNS;

					Mat._11 = BlinkScale;
					Mat._22 = BlinkScale;
					Mat._33 = BlinkScale;
					smMatrixMult( pat->obj3d[0]->mWorld , Mat , pat->obj3d[0]->mWorld );
				}
				pat->RenderD3D( &CameraPosi , &CameraAngle );
				RestoreRenderBlinkColor();		//색상복구
			}
		}
/*
				BlinkLevel = SetRenderBlinkColor( &HvLeftHand );

				//스케일 점멸
				if ( HvLeftHand.BlinkScale && BlinkLevel ) {
					smIdentityMatrix( Mat );
					BlinkScale = (BlinkLevel*HvLeftHand.BlinkScale)>>FLOATNS;
					Mat._11 = fONE+BlinkScale;
					Mat._22 = fONE+BlinkScale;
					Mat._33 = fONE+BlinkScale;
					pat->Frame =  -1;
					memcpy( &pat->obj3d[0]->mWorld , &HvLeftHand.ObjBip->mWorld , sizeof(smMATRIX) );
					smMatrixMult( pat->obj3d[0]->mWorld , Mat , pat->obj3d[0]->mWorld );
				}

*/

	}

	int r,g,b,c,d;
	int cnt;

//#define	SKILL_APPMASK_EXTREME_SHIELD		0x0001
//	DWORD			dwDispAppSkill;			//스킬 적용표시

//	for(cnt=0;cnt<10;cnt++) {
//		if ( ContinueSkill[cnt].Flag && ContinueSkill[cnt].CODE==SKILL_EXTREME_SHIELD ) {

	pat = 0;

	if ( (dwDispAppSkill&SKILL_APPMASK_EXTREME_SHIELD) && 
		( MotionInfo->State!=CHRMOTION_STATE_SKILL || (AttackSkil&0xFF)!=SKILL_PLAY_EXTREME_SHIELD ) ) {
			pat = PatEffectShield;
		}
/*
	if ( (dwDispAppSkill&SKILL_APPMASK_SPARK_SHIELD) && 
		( MotionInfo->State!=CHRMOTION_STATE_SKILL || (AttackSkil&0xFF)!=SKILL_PLAY_SPARK_SHIELD ) ) {
			pat = PatSparkShield;
		}
*/
	if ( pat ) {
			//익스트림 쉴드 표시
			if ( RendSucess && HvLeftHand.PatTool && 
				(HvLeftHand.dwItemCode&sinITEM_MASK2)==sinDS1 ) {

				memcpy( &angle, &Angle , sizeof( POINT3D ) );
				angle.y = (-angle.y+ANGLE_180)&ANGCLIP;

				Posi.x = pX;
				Posi.y = pY;
				Posi.z = pZ;
				a = smRender.Color_A;
				r = smRender.Color_R;
				g = smRender.Color_G;
				b = smRender.Color_B;
				smRender.Color_B = +128;

				//PatSparkShield
				//pat = PatEffectShield;//HvLeftHand.PatTool;;//PatEffectShield;

				pat->SetPosi( &Posi , &angle );

				d = dwPlayTime>>4;
				c = d&0x3F;
				if ( (d&0x40) ) {
					c = 0x40-c;
				}

				c += (int)(fONE*0.9f)+c;

				smIdentityMatrix( Mat );
				if ( PatEffectShield==pat ) {
					Mat._11 = c;
					Mat._22 = c;
					Mat._33 = c;
				}

				for( cnt=0;cnt<pat->nObj3d;cnt++) {
					memcpy( &pat->obj3d[cnt]->mWorld , &HvLeftHand_Matrix , sizeof(smMATRIX) );
					smMatrixMult( pat->obj3d[cnt]->mWorld , Mat , pat->obj3d[cnt]->mWorld );
				}

				smRender.Color_R = 0;
				smRender.Color_G = 0;
				smRender.Color_B = 0;
				smRender.Color_A = 0;

				pat->Frame = -1;
				pat->RenderD3D( &CameraPosi , &CameraAngle );

				smRender.Color_A = a;
				smRender.Color_R = r;
				smRender.Color_G = g;
				smRender.Color_B = b;
/*
				HvLeftHand.sColors[SMC_R] = 0;
				HvLeftHand.sColors[SMC_G] = 0;
				HvLeftHand.sColors[SMC_B] = 0;
				HvLeftHand.sColors[SMC_A] = -160;
*/
			}
	}


	int s,sz,maxSize;

	//익스팬션 스킬
	if ( (AttackSkil&0xFF)==SKILL_PLAY_EXPANSION && RendSucess && HvRightHand.PatTool && MotionInfo->State==CHRMOTION_STATE_ATTACK ) {

		sz = Expansion_Size[(AttackSkil>>8)-1];

		memcpy( &angle, &Angle , sizeof( POINT3D ) );
		angle.y = (-angle.y+ANGLE_180)&ANGCLIP;

		Posi.x = pX;
		Posi.y = pY;
		Posi.z = pZ;
		a = smRender.Color_A;
		r = smRender.Color_R;
		smRender.Color_A = -96-(FrameCounter<<1);
		smRender.Color_R = +128;
		if ( smRender.Color_A<-192 ) smRender.Color_A=-192;

		pat = HvRightHand.PatTool;
		pat->SetPosi( &Posi , &angle );
		pat->Frame = -1;

		smIdentityMatrix( Mat );
		//Mat._11 = (int)(fONE*1.05f);
		//Mat._22 = (int)(fONE*1.05f);
		//Mat._33 = (int)(fONE*1.05f);
		s = FrameCounter<<3;
		maxSize = (sz*fONE)/100;
		if ( s>maxSize ) s=maxSize;
		s += (fONE/4);

		Mat._11 = fONE+s;
		Mat._22 = fONE+s;
		Mat._33 = fONE+s;

		memcpy( &pat->obj3d[0]->mWorld , &HvRightHand_Matrix , sizeof(smMATRIX) );
		smMatrixMult( pat->obj3d[0]->mWorld , Mat , pat->obj3d[0]->mWorld );
		pat->RenderD3D( &CameraPosi , &CameraAngle );

		smRender.Color_A = a;
		smRender.Color_R = r;
	}

	//블레스 캐슬 왕관
	if ( Clan_CastleMasterFlag && smCharInfo.ClassClan && PatCrown && RendSucess && RendPoint.z<12*64*fONE && PlayVanish<=0 ) {

		GetMoveLocation( 0 , 0, 0*fONE , 0, Angle.y , 0 );

		pos.x = pX+GeoResult_X;
		pos.y = pY+PatHeight+10*fONE;	//pY;
		pos.z = pZ+GeoResult_Z;

		cnt = (dwPlayTime>>2)&0xFF;
		if ( (cnt&0x80) ) {
			pos.y+= (cnt&0x7F);
		}
		else {
			pos.y+= 0x80-(cnt&0x7F);
		}

		angle.x = 0;
		angle.y = (dwPlayTime>>2)&ANGCLIP;
		angle.z = 0;

		//a = smRender.Color_A;
		//smRender.Color_A -=160;

	    lpD3DDevice->SetRenderState( D3DRENDERSTATE_ZWRITEENABLE, FALSE );
		smRender.SetMaterialGroup( smMaterialGroup );
		PatCrown->SetPosi( &pos, &angle );
		PatCrown->RenderD3D( &CameraPosi, &CameraAngle );
	    lpD3DDevice->SetRenderState( D3DRENDERSTATE_ZWRITEENABLE, TRUE );

		//smRender.Color_A = a;
	}

	return TRUE;

}


//######################################################################################
//작 성 자 : 오 영 석
void smCHAR::SetIceFoot( int TypeNum, DWORD dwPlayTime, int x, int z )
{
	if( smGameStage[OnStageField] == NULL )
		return;

	int loop; for( loop =0; loop < ICE_FOOT_COUNT_MAX; loop++ )
	{
		if( m_IceFootInfo[loop].Life == 0 )
			break;
	}

	if( loop >= ICE_FOOT_COUNT_MAX )
		return;

	POINT3D FootPos;
	FootPos.x = pX + x;
	//FootPos.y = PHeight;
	FootPos.y = pY + (10*fONE);
	FootPos.z = pZ + z;

	POINT3D FaceAngle;
	if( smGameStage[OnStageField]->CheckFaceIceFoot( &FootPos, &FaceAngle, 32*fONE ) == 0 )
		return;

	m_IceFootInfo[loop].Life       = 1;
	m_IceFootInfo[loop].dwPlayTime = dwPlayTime;
	m_IceFootInfo[loop].TypeNum    = TypeNum;
	m_IceFootInfo[loop].FootPos.x  = FootPos.x;
	m_IceFootInfo[loop].FootPos.y  = FootPos.y;
	m_IceFootInfo[loop].FootPos.z  = FootPos.z;
	m_IceFootInfo[loop].FootAngle.x = -(FaceAngle.x-ANGLE_180) & ANGCLIP;
	m_IceFootInfo[loop].FootAngle.y = (-Angle.y) & ANGCLIP;	
	m_IceFootInfo[loop].FootAngle.z = (FaceAngle.z-ANGLE_180) & ANGCLIP;
}

//
int smCHAR::DrawIceFoot(void)
{
	if( MotionInfo == NULL || Pattern == NULL || MotionInfo->State == CHRMOTION_STATE_RESTART || RendSucess == 0 )
		return FALSE;

	POINT3D CameraPosi, CameraAngle;

	CameraPosi.x = smCHAR_CameraX;
	CameraPosi.y = smCHAR_CameraY;
	CameraPosi.z = smCHAR_CameraZ;

	CameraAngle.x = smCHAR_CameraAngX;
	CameraAngle.y = smCHAR_CameraAngY;
	CameraAngle.z = smCHAR_CameraAngZ;

	int b_r, b_g, b_b, b_a;
	b_r = smRender.Color_R;
	b_g = smRender.Color_G;
	b_b = smRender.Color_B;
	b_a = smRender.Color_A;

	smRender.Color_R  = 0;
	smRender.Color_G  = 0;
	smRender.Color_B  = 0;
	smRender.Color_A  = 0;

	smRender.m_dwRenderType = smRENDER_TYPE_ICE;

	smRender.SetMaterialGroup( ::smMaterialGroup );
	lpD3DDevice->SetRenderState( D3DRENDERSTATE_ZWRITEENABLE, FALSE );
	for( int loop=0; loop < ICE_FOOT_COUNT_MAX; loop++ )
	{
		if( m_IceFootInfo[loop].Life )
		{
			float Alpha = float(dwPlayTime - m_IceFootInfo[loop].dwPlayTime) * ICE_FOOT_ALPHA_STEP;
			if( Alpha >= 200 )
				m_IceFootInfo[loop].Life = 0;
			else
			{
				smRender.Color_A = -int(Alpha);
				PatIceFoot[ m_IceFootInfo[loop].TypeNum ]->SetPosi( &m_IceFootInfo[loop].FootPos, &m_IceFootInfo[loop].FootAngle );
				PatIceFoot[ m_IceFootInfo[loop].TypeNum ]->RenderD3D( &CameraPosi, &CameraAngle );
			}
		}
	}
	lpD3DDevice->SetRenderState( D3DRENDERSTATE_ZWRITEENABLE, TRUE );

	smRender.Color_R = b_r;
	smRender.Color_G = b_g;
	smRender.Color_B = b_b;
	smRender.Color_A = b_a;

	smRender.m_dwRenderType = 0;
	return TRUE;
}
//######################################################################################




#define STATE_BAR_WIDTH		100
#define STATE_BAR_HEIGHT	16
#define STATE_BAR_SIZE		92


// 체력바 그리기
int smCHAR::DrawStateBar( int x, int y )
{
	int Percent;
	int	Percent2;
	int	Percent3;

	DWORD dwColor;

	//바벨 에너지바 설정
	if ( !EnableStateBar && smCharInfo.State==smCHAR_STATE_ENEMY ) {
		if ( smCharInfo.dwCharSoundCode==snCHAR_SOUND_BABEL && TryAttackCount>=10 ) {
			EnableStateBar = TRUE;
		}
		else if ( smCharInfo.dwCharSoundCode==snCHAR_SOUND_CASTLE_DOOR ||
				smCharInfo.dwCharSoundCode==snCHAR_SOUND_CASTLE_TOWER_B ||
				smCharInfo.dwCharSoundCode==snCHAR_SOUND_CASTLE_CRYSTAL_R ||
				smCharInfo.dwCharSoundCode==snCHAR_SOUND_CASTLE_CRYSTAL_G ||
				smCharInfo.dwCharSoundCode==snCHAR_SOUND_CASTLE_CRYSTAL_B ||
				smCharInfo.dwCharSoundCode==snCHAR_SOUND_CASTLE_CRYSTAL_N ) 
		{
			EnableStateBar = TRUE;
		}
	}

	if ( smCharInfo.Life[1]==0 || ( !DisplayDebug && !EnableStateBar ) ) return NULL;
	if ( x<0 || (x+STATE_BAR_WIDTH)>=smScreenWidth ) return NULL;
	if ( y<0 || (y+STATE_BAR_HEIGHT)>=smScreenHeight ) return NULL;

	psSetDefaultRenderState();
	lpD3DDevice->SetRenderState( D3DRENDERSTATE_ALPHABLENDENABLE, FALSE );

	Percent = (smCharInfo.Life[0]*STATE_BAR_SIZE)/smCharInfo.Life[1];
	Percent2 = (smCharInfo.Life[0]*511)/smCharInfo.Life[1];
	Percent2 = (Percent2*Percent2)/512;
	Percent3 = Percent2&0xFF;

	if ( Percent2<256 )
		dwColor = RGBA_MAKE( 255 , Percent3 , 0, 255 );
	else
		dwColor = RGBA_MAKE( 255-Percent3 , 255 , 0, 255 );

	//Percent = (DispLifeBar*STATE_BAR_SIZE)/smCharInfo.Life[1];
/*
	if( Percent==STATE_BAR_SIZE ) {
		dsDrawTexImage(MatEnergyBox[1] , x , y , STATE_BAR_WIDTH , STATE_BAR_HEIGHT,
			128 , 16 , 0 , 0, STATE_BAR_WIDTH , 16, 255);
	}
	else {
		dsDrawTexImage(MatEnergyBox[1] , x , y , Percent+3 , STATE_BAR_HEIGHT,
			128 , 16 , 0 , 0,(float)Percent+3 , 16, 255);

		dsDrawTexImage(MatEnergyBox[0] , x+Percent+3 , y , 100-Percent+3 , STATE_BAR_HEIGHT,
			128 , 16 , (float)Percent+4 , 0, 101 , 16, 255);
	}
*/
	float fSize;

	fSize = (float)WinSizeX/800.0f;


	dsDrawTexImageFloat(MatEnergyBox[0] , (float)x , (float)y , STATE_BAR_WIDTH*fSize , STATE_BAR_HEIGHT*fSize, 128 , 16 , 0 , 0, STATE_BAR_WIDTH , 15, 255);

	if ( smCharInfo.Life[0]>0 )
		dsDrawTexImageFloatColor(MatEnergyBox[1] , (float)x , (float)y , (float)(Percent+3)*fSize , STATE_BAR_HEIGHT*fSize, 128 , 16 , 0 , 0,(float)Percent+3 , 15, dwColor );

	return TRUE;
}


// 체력바 그리기
int smCHAR::DrawStateBar2( int x, int y )
{
	int Percent;
	int	Percent2;
	int	Percent3;

	DWORD dwColor;

	if ( smCharInfo.Life[1]==0 ) return NULL;
	if ( x<0 || (x+(STATE_BAR_WIDTH/2))>=smScreenWidth ) return NULL;
	if ( y<0 || (y+(STATE_BAR_HEIGHT/2))>=smScreenHeight ) return NULL;

	psSetDefaultRenderState();
	lpD3DDevice->SetRenderState( D3DRENDERSTATE_ALPHABLENDENABLE, FALSE );

	Percent = (smCharInfo.Life[0]*STATE_BAR_SIZE)/smCharInfo.Life[1];
	Percent2 = (smCharInfo.Life[0]*511)/smCharInfo.Life[1];
	Percent2 = (Percent2*Percent2)/512;
	Percent3 = Percent2&0xFF;

	if ( Percent2<256 )
		dwColor = RGBA_MAKE( 255 , Percent3 , 0, 255 );
	else
		dwColor = RGBA_MAKE( 255-Percent3 , 255 , 0, 255 );
/*
	if( Percent==STATE_BAR_SIZE ) {
		dsDrawTexImageFloat(MatEnergyBox[1] , (float)x , (float)y , STATE_BAR_WIDTH*0.6f , STATE_BAR_HEIGHT*0.7f,
			128 , 16 , 0 , 0, STATE_BAR_WIDTH , 15, 255);
	}
	else {
		dsDrawTexImageFloat(MatEnergyBox[1] , (float)x , (float)y , ((float)Percent+3)*0.6f , STATE_BAR_HEIGHT*0.7f,
			128 , 16 , 0 , 0,(float)Percent+3 , 15, 255);

		dsDrawTexImageFloat(MatEnergyBox[0] , (float)x+((float)Percent+3)*0.6f , (float)y , (100-(float)Percent+3)*0.6f , STATE_BAR_HEIGHT*0.7f,
			128 , 16 , (float)Percent+4 , 0, 101 , 15, 255);
	}
*/

	dsDrawTexImageFloat(MatEnergyBox[0] , (float)x , (float)y , STATE_BAR_WIDTH*0.6f , STATE_BAR_HEIGHT*0.7f, 128 , 16 , 0 , 0, STATE_BAR_WIDTH , 15, 255);
	dsDrawTexImageFloatColor(MatEnergyBox[1] , (float)x , (float)y , ((float)Percent+3)*0.6f , STATE_BAR_HEIGHT*0.7f, 128 , 16 , 0 , 0,(float)Percent+3 , 15, dwColor );

	return TRUE;
}


// 차징 게이지 그리기
int smCHAR::DrawChargingBar( int x, int y , int Charging , int ChargingMax )
{
	int	Percent;

	DWORD dwColor;

	if ( smCharInfo.Life[1]==0 ) return NULL;
	if ( x<0 || (x+(STATE_BAR_WIDTH/2))>=smScreenWidth ) return NULL;
	if ( y<0 || (y+(STATE_BAR_HEIGHT/2))>=smScreenHeight ) return NULL;

	psSetDefaultRenderState();
	lpD3DDevice->SetRenderState( D3DRENDERSTATE_ALPHABLENDENABLE, FALSE );

	Percent = (Charging*STATE_BAR_SIZE)/ChargingMax;
	dwColor = RGBA_MAKE( 160 , 160 , 255, 255 );

	dsDrawTexImageFloat(MatEnergyBox[0] , (float)x , (float)y , STATE_BAR_WIDTH*0.6f , STATE_BAR_HEIGHT*0.7f, 128 , 16 , 0 , 0, STATE_BAR_WIDTH , 15, 255);
	dsDrawTexImageFloatColor(MatEnergyBox[1] , (float)x , (float)y , ((float)Percent+3)*0.6f , STATE_BAR_HEIGHT*0.7f, 128 , 16 , 0 , 0,(float)Percent+3 , 15, dwColor );

	return TRUE;
}

//움직임에 잔상 남기기
int smCHAR::DrawMotionBlur()
{

	if ( HideWeapon || !DisplayFlag || !RendSucess || !smMotionInfo ) return FALSE;

	if ( RendPoint.z<24*64*fONE ) {
		//손에 효과 주기 ( 파이곤 용 )
		StartHandEffect( 0 );
	}

	if ( ( smCharInfo.State!=smCHAR_STATE_USER && PkMode_CharState!=smCHAR_STATE_USER ) || RendPoint.z>10*64*fONE ) return FALSE;

	if ( ShootingMode || (smMotionInfo->MotionInfo[action].State!=CHRMOTION_STATE_ATTACK && smMotionInfo->MotionInfo[action].State!=CHRMOTION_STATE_SKILL ) )
		return FALSE;

	if ( HvLeftHand.PatTool ) 
		DrawMotionBlurTool( &HvLeftHand );

	if ( HvRightHand.PatTool ) 
		DrawMotionBlurTool( &HvRightHand );

	return TRUE;
}



//무기 움직임에 잔상 남기기
int smCHAR::DrawMotionBlurTool( smCHARTOOL *ChrTool )
{
	DWORD ClipStates;
	smVERTEX *vp1,*vp2;
	smVERTEX *vp3,*vp4;

	int cnt,vcnt;
	int	sin,cos;
	DWORD pframe;
	int rx,ry,rz;
	smMATRIX *mWorld;
	POINT3D angle;
	POINT3D pTop , pBot;
	int szLevel;
	int mLevel;
	int mFrames;
	int mLevel2;
	smVERTEX	*lpVertex;
	smFACE		*lpFace;

	int r,g,b;


	if ( !DisplayFlag || !dwActionItemCode || (dwActionItemCode&sinITEM_MASK2)==sinDS1 ) return FALSE;//|| ChrTool->SizeMax<fONE*8 ) return FALSE;


	if ( ActionPattern==0 ) {
		//주인공은 32단계
		mLevel = 32;
		mFrames = 40;
		mLevel2 = mLevel<<1;
		lpVertex = MbVertex;
		lpFace = MbFace;
	}
	else {
		//다른 캐릭은 16단계
		mLevel = 16;
		mFrames = 80;
		mLevel2 = mLevel<<1;
		lpVertex = MbVertex2;
		lpFace = MbFace2;
	}

	pTop.x = 0;
	pTop.y = 0;
	pTop.z = 0;
	//pTop.z = ChrTool->SizeMin;

	pBot.x = 0;
	pBot.y = 0;
	pBot.z = ChrTool->SizeMax;	//fONE*20;

	szLevel = pBot.z/(mLevel<<1);

	if ( dwActionItemTwoHand ) {
		cnt = (-ANGLE_45/5)&ANGCLIP;
		//양손무기는 5도 기울어짐
		/*
		pTop.y = (pTop.z*(GetSin[ ANGLE_45/9 ]>>8))>>8;
		pBot.y = (pBot.z*(GetSin[ ANGLE_45/9 ]>>8))>>8;
		*/
		sin = GetSin[cnt]>>8;
		cos = GetCos[cnt]>>8;

		pTop.y = -( pTop.z*sin )>>8;
		pTop.z = ( pTop.z*cos )>>8;

		pBot.y = -( pBot.z*sin )>>8;
		pBot.z = ( pBot.z*cos )>>8;
	}

	memcpy( &angle, &Angle , sizeof( POINT3D ) );
	angle.y = (-angle.y+ANGLE_180)&ANGCLIP;

	if ( ActionGameMode ) {
		if ( angle.y>ANGLE_180 )
			angle.y = ANGLE_270;
		else
			angle.y = ANGLE_90;
	}


	mWorld = &ChrTool->ObjBip->mWorld;

	if ( ChrTool->PatTool && ChrTool->ObjBip ) {
		for( cnt=0;cnt<mLevel;cnt++) {
			pframe = frame-(cnt*mFrames);
			if ( pframe<MotionInfo->StartFrame ) pframe = MotionInfo->StartFrame;

			AnimObjectTree( ChrTool->ObjBip , pframe , angle.x , angle.y , angle.z );

			vcnt = cnt<<1;

			rx = pTop.x * mWorld->_11 +
				 pTop.y * mWorld->_21 +
				 pTop.z * mWorld->_31 ;

			ry = pTop.x * mWorld->_12 +
				 pTop.y * mWorld->_22 +
				 pTop.z * mWorld->_32 ;

			rz = pTop.x * mWorld->_13 +
				 pTop.y * mWorld->_23 +
				 pTop.z * mWorld->_33 ;

			lpVertex[vcnt].x = pX + (rx>>FLOATNS) + mWorld->_41;
			lpVertex[vcnt].z = pZ + (ry>>FLOATNS) + mWorld->_42;
			lpVertex[vcnt].y = pY + (rz>>FLOATNS) + mWorld->_43;

			vcnt++;

			rx = pBot.x * mWorld->_11 +
				 pBot.y * mWorld->_21 +
				 pBot.z * mWorld->_31 ;

			ry = pBot.x * mWorld->_12 +
				 pBot.y * mWorld->_22 +
				 pBot.z * mWorld->_32 ;

			rz = pBot.x * mWorld->_13 +
				 pBot.y * mWorld->_23 +
				 pBot.z * mWorld->_33 ;

			lpVertex[vcnt].x = pX + (rx>>FLOATNS) + mWorld->_41;
			lpVertex[vcnt].z = pZ + (ry>>FLOATNS) + mWorld->_42;
			lpVertex[vcnt].y = pY + (rz>>FLOATNS) + mWorld->_43;

			pTop.z += szLevel;
			pBot.z -= szLevel;
		}
	}

	//클리핑 모드값
	ClipStates = ( SMCLIP_NEARZ | SMCLIP_FARZ | SMCLIP_LEFT | SMCLIP_RIGHT | SMCLIP_TOP | SMCLIP_BOTTOM );

	smRender.Init();
	smRender.SetCameraPosi( smCHAR_CameraX,smCHAR_CameraY,smCHAR_CameraZ, 
		smCHAR_CameraAngX,smCHAR_CameraAngY,smCHAR_CameraAngZ );


	for (cnt = 0; cnt<mLevel2; cnt++) 
		smRender.AddObjectVertex( &lpVertex[cnt] );

	for (cnt = 0; cnt<(mLevel2-2); cnt+=2) {
		vp1 = &lpVertex[cnt];
		vp2 = &lpVertex[cnt+1];
		vp3 = &lpVertex[cnt+2];
		vp4 = &lpVertex[cnt+3];

		if ( vp1->x!=vp3->x || vp1->y!=vp3->y || vp1->z!=vp3->z ||
			 vp2->x!=vp4->x || vp2->y!=vp4->y || vp2->z!=vp4->z ) {
			smRender.AddObjectFace( &lpFace[cnt] );
			smRender.AddObjectFace( &lpFace[cnt+1] );
		}
	}
/*
				lpCurPlayer->HvRightHand.ColorBlink = 9;
				lpCurPlayer->HvRightHand.sColors[0] = 220;
				lpCurPlayer->HvRightHand.sColors[1] = 80;
				lpCurPlayer->HvRightHand.sColors[2] = 0;
				lpCurPlayer->HvRightHand.sColors[3] = 0;
*/

//		int r,g,b;

	r = smRender.Color_R;
	g = smRender.Color_G;
	b = smRender.Color_B;

	//스킬 색상 적용 ( 우선 )
	if ( AttackSkil ) {
		cnt = SetSkillMotionBlurColor( AttackSkil );
	}

	//무기 색상 적용
	if ( !cnt && ChrTool->ColorBlink ) {
		smRender.Color_R += ChrTool->sColors[SMC_R]>>1;
		smRender.Color_G += ChrTool->sColors[SMC_G]>>1;
		smRender.Color_B += ChrTool->sColors[SMC_B]>>1;
	}


	if ( smRender.nRendFace ) {
		smRender.SetClipStates( ClipStates );				// 클리핑 선택
		smRender.ClipRendFace();							// 전체 렌더링 페이스를 클리핑
		smRender.GeomVertex2D( FALSE );				// 버텍스를 2D좌표로 변환  
		smRender.RenderD3D();
	}

	smRender.Color_R = r;
	smRender.Color_G = g;
	smRender.Color_B = b;

	return TRUE;
}





//캐릭터에 패턴을 설정
int smCHAR::SetLoadPattern( char *szName )
{
	Pattern = 0;
	Flag = 1;
	return AddLoaderPattern( (smCHAR *)&Head , szName );

}

//받은 데미지 암화화 코드
DWORD smCHAR::GetAttackTrans_XorCode()
{
	return dwObjectSerial^lpCurPlayer->smCharInfo.dwObjectSerial^(((DWORD)this)>>8);
}

//받은데미지 수치 초기화
int smCHAR::ResetAttackTrans()
{

	AttackTrans.code = GetAttackTrans_XorCode();
	return TRUE;
}

//받은데미지 수치 암호화 반전
int smCHAR::XorAttackTrans()
{
	int cnt,len;
	DWORD dwCode;

	len = (sizeof(TRANS_ATTACKDATA)/sizeof(DWORD));//+2;
	DWORD *lpdwBuff = (DWORD *)&AttackTrans;

	dwCode = GetAttackTrans_XorCode();

	lpdwBuff[0] ^= dwCode;
	lpdwBuff[1] ^= dwCode;

	for( cnt=2;cnt<len;cnt++ )  {
		lpdwBuff[cnt] ^= dwCode;
		dwCode += dwCode;
	}

	return TRUE;
}


//공격 실행
int smCHAR::PlayAttack( int EventFlag )
{
	int cnt,len,act;
	int block,ChanceBlock;
	smCHAR *lpChar;
	int	HolyBody_Undead = 0;
	int x,y,z;
	POINT3D	Pos1;//,Pos2;
	int temp;
	int Param1,Param2;
	int	bStunCount;


	TRANS_ATTACKDATA	*lpTransAttackData;

		lpTransAttackData = &AttackTrans;

		XorAttackTrans();

		if ( !lpTransAttackData->code || ( RestartPlayCount>0 && RestartPlayCount<1000 ) || 
			( UseEtherCoreCode && lpCurPlayer->MotionInfo->State==CHRMOTION_STATE_EAT )) {
				//ResetAttackTrans();//lpTransAttackData->code = 0;
				AttackTrans.code = GetAttackTrans_XorCode();
				if ( RestartPlayCount ) Ptect_IncAttackCount(0);
				return FALSE;
			}

		if ( lpTransAttackData->dwDamageChkSum!=dm_GetDamgeChkSum_S2V(lpTransAttackData) ) {	//코드 검사 오류
			ResetAttackTrans();
			SendSetHackUser3( 8580 , lpTransAttackData->Power , 0 );
			return FALSE;
		}

		Ptect_IncAttackCount( -1 );

		if ( lpCurPlayer->MotionInfo->State != CHRMOTION_STATE_DEAD && DisplayFlag ) {
			len = GetDistanceDbl( lpCurPlayer->pX , lpCurPlayer->pZ , pX, pZ );
			cnt = GetDistanceDbl( lpCurPlayer->pX , lpCurPlayer->pZ , lpTransAttackData->x, lpTransAttackData->z );

			if ( smCharInfo.State==smCHAR_STATE_USER || PkMode_CharState==smCHAR_STATE_USER ) {
				len=0;			//PK 모드
				cnt = 0;
			}

			if ( smCharInfo.dwCharSoundCode==snCHAR_SOUND_FIGON || smCharInfo.dwCharSoundCode==snCHAR_SOUND_WEB ||
				smCharInfo.dwCharSoundCode==snCHAR_SOUND_SKELETONRANGE || smCharInfo.dwCharSoundCode==snCHAR_SOUND_IMP || 
				smCharInfo.dwCharSoundCode==snCHAR_SOUND_SCORPION ||
				
				smCharInfo.dwCharSoundCode==snCHAR_SOUND_NAZ ||
				smCharInfo.dwCharSoundCode==snCHAR_SOUND_NAZSENIOR || // 박재원 - 끝없는 탑 3층(신규필드 몬스터)
				smCharInfo.dwCharSoundCode==snCHAR_SOUND_SHADOW ||
				smCharInfo.dwCharSoundCode==snCHAR_SOUND_BEEDOG ||
				smCharInfo.dwCharSoundCode==snCHAR_SOUND_MUTANTTREE ||
				smCharInfo.dwCharSoundCode==snCHAR_SOUND_SUCCUBUS ||
				smCharInfo.dwCharSoundCode==snCHAR_SOUND_BERSERKER ||
				smCharInfo.dwCharSoundCode==snCHAR_SOUND_STYGIANLORD ||	

				smCharInfo.dwCharSoundCode==snCHAR_SOUND_AVELIN_ARCHER ||
				smCharInfo.dwCharSoundCode==snCHAR_SOUND_AVELIN_QUEEN ||
				smCharInfo.dwCharSoundCode==snCHAR_SOUND_DMACHINE ||
				smCharInfo.dwCharSoundCode==snCHAR_SOUND_SADNESS ||
				smCharInfo.dwCharSoundCode==snCHAR_SOUND_RUNICGUARDIAN ||
				smCharInfo.dwCharSoundCode==snCHAR_SOUND_BABEL ||
				smCharInfo.dwCharSoundCode==snCHAR_SOUND_HYPERMACHINE ||
				smCharInfo.dwCharSoundCode==snCHAR_SOUND_TURTLE_CANNON ||


				smCharInfo.dwCharSoundCode==snCHAR_SOUND_FROZEN ||
				smCharInfo.dwCharSoundCode==snCHAR_SOUND_CHAOSCARA ||
				smCharInfo.dwCharSoundCode==snCHAR_SOUND_COLDEYE ||
				smCharInfo.dwCharSoundCode==snCHAR_SOUND_M_LORD ||
				smCharInfo.dwCharSoundCode==snCHAR_SOUND_KELVEZU ||

				smCharInfo.dwCharSoundCode==snCHAR_SOUND_DARKGUARD ||
				smCharInfo.dwCharSoundCode==snCHAR_SOUND_DARKMAGE ||
				smCharInfo.dwCharSoundCode==snCHAR_SOUND_FIREWORM ||
				smCharInfo.dwCharSoundCode==snCHAR_SOUND_SETO ||

				smCharInfo.dwCharSoundCode==snCHAR_SOUND_NPC_MAGICMASTER ||
				smCharInfo.dwCharSoundCode==snCHAR_SOUND_NPC_SKILLMASTER ||

				smCharInfo.dwCharSoundCode==snCHAR_SOUND_CASTLE_CRYSTAL_R ||
				smCharInfo.dwCharSoundCode==snCHAR_SOUND_CASTLE_CRYSTAL_G ||
				smCharInfo.dwCharSoundCode==snCHAR_SOUND_CASTLE_CRYSTAL_B ||
				
				smCharInfo.dwCharSoundCode==snCHAR_SOUND_GOBLINSHAMAN || // 박재원 - 저주받은 신전 3층(신규필드 몬스터) - 고블린샤먼

				lpTransAttackData->AttackState==0x80						//스킬공격
				) {
				//원거리 공격
				if ( len<(460*460) ) len = 0;

				cnt = 0;

				if ( smCharInfo.dwCharSoundCode==snCHAR_SOUND_WEB ) {
					lpCurPlayer->PlaySlowCount = 70*2;			//2초동안 느리게 움직임
					lpCurPlayer->PlaySlowSpeed = 180;
				}
				if ( !EventFlag ) { //&& !dwAttackPlayTime ) {	
					dwAttackPlayTime = dwPlayTime+1100;
					XorAttackTrans();
					return TRUE;
				}
			}

			if ( lpTransAttackData->sRating[0]<0 ) {
				Param1 = -lpTransAttackData->sRating[0];
				Param2 = lpTransAttackData->sRating[1];
				lpTransAttackData->sRating[0]= 0;
				lpTransAttackData->sRating[1]= 0;
			}
			else {
				Param1 = 0;
				Param2 = 0;
			}


			//주인공 몬스터 스킨일 경우
			if ( lpCurPlayer->smCharInfo.State==smCHAR_STATE_ENEMY ) act = 1;
			else act = 0;

			if ( lpTransAttackData->AttackState==0x80 ) {
				x = (lpTransAttackData->x-lpCurPlayer->pX)>>FLOATNS;
				y = (lpTransAttackData->y-lpCurPlayer->pY)>>FLOATNS;
				z = (lpTransAttackData->z-lpCurPlayer->pZ)>>FLOATNS;
				if ( (x*x+y*y+z*z)>(140*140) ) {
					cnt = lpTransAttackData->AttackSize;
				}
				act=1;			//몬스터의 스킬공격 ( 명중 100% )
			}

			Record_RecvDamageCount++;			//데미지 카운터

			if ( cnt<lpTransAttackData->AttackSize && sinGetLife()>0 && ( len<(330*330) || act ) && DisplayAlpha==0 ) {
/*
				lpChar = 0;

				//공격한 상대방을 찾아 방향을 바꾼다
				for( cnt=0;cnt<OTHER_PLAYER_MAX;cnt++ ) {
					if ( chrOtherPlayer[cnt].Flag && chrOtherPlayer[cnt].dwObjectSerial==lpTransAttackData->dwDestObjectSerial ) {
						lpChar = &chrOtherPlayer[cnt];
						break;
					}
				}
*/

				lpChar = this;

				block = FALSE;

				if ( lpChar ) {

					if ( lpCurPlayer->smCharInfo.JOB_CODE==JOBCODE_ARCHER ) {			//아처
						cnt = cSkill.GetSkillPoint( SKILL_EVASION_MASTERY );			//추가 회피 마스터리
						if ( cnt>0 && cnt<=10 ) {
							cnt = Evasion_Mastery_AddPercent[ cnt-1 ];
							if ( cnt<=26 ) { // 장별 - 스킬밸런스 조정(10.08.10)
								if ( (rand()%100)<=cnt ) {
									//추가회피 성공
									cInterFace.SetStringEffect(HA_EVASION);
									block = 100;
									Record_BlockCount++;
								}
							}
						}
					}



					if ( lpCurPlayer->smCharInfo.Chance_Block ) {
						ChanceBlock = sinGetBlockRating( lpChar->smCharInfo.Level );

						if ( (lpCurPlayer->HvLeftHand.dwItemCode&sinITEM_MASK2)==sinDS1 ) {
							if ( lpCurPlayer->smCharInfo.JOB_CODE==JOBCODE_MECHANICIAN && (lpCurPlayer->dwDispAppSkill&SKILL_APPMASK_EXTREME_SHIELD)!=0 ) {		//익스트림 쉴드
								ChanceBlock+=(lpTransAttackData->AttackState>>16);
							}
							if ( lpCurPlayer->smCharInfo.JOB_CODE==JOBCODE_KNIGHT && (lpCurPlayer->dwDispAppSkill&SKILL_APPMASK_SPARK_SHIELD)!=0 ) {		//익스트림 쉴드
								ChanceBlock+=(lpTransAttackData->AttackState>>(16+8));
							}
							if ( lpCurPlayer->smCharInfo.JOB_CODE==JOBCODE_KNIGHT && (lpCurPlayer->dwDispAppSkill&SKILL_APPMASK_GODLY_SHIELD)!=0 ) {		//가들리 쉴드
								cnt = cSkill.GetSkillPoint( SKILL_DIVINE_INHALATION );		//디바인 쉴드 인계
								if ( cnt>0 && D_Inhalation_Block[cnt-1]<=10 ) {
									ChanceBlock+=D_Inhalation_Block[cnt-1];
								}
							}
							if ( ChanceBlock>50 ) ChanceBlock=50;
						}
					}
					else 
						ChanceBlock = 0;

					if ( (rand()%100)<ChanceBlock ) {
						//블럭성공
						cInterFace.SetStringEffect(HA_BLOCK);
						block = TRUE;
						Record_BlockCount++;

						if ( abs(lpChar->smCharInfo.Level-lpCurPlayer->smCharInfo.Level)<=AGING_SUB_LEVEL ) {
							//레벨 7차이 이하는 에이징 적용
							sinCheckAgingLevel( SIN_AGING_BLOCK , false);
						}

						if ( (lpCurPlayer->dwDispAppSkill&SKILL_APPMASK_SPARK_SHIELD) && lpChar ) {

							if ( lpCurPlayer->smCharInfo.JOB_CODE==JOBCODE_MECHANICIAN ) {
								//스파크 쉴드 적용
								dm_SendTransDamage( lpChar , 0,0,0,0, SKILL_PLAY_SPARK_SHIELD|0x100 , FALSE );
								/*
								//스파크 쉴드
								if ( lpCurPlayer->HvLeftHand.PatTool ) {
									if ( lpCurPlayer->GetToolBipPoint( &lpCurPlayer->HvLeftHand , &x,&y,&z ) ) {

										Pos1.x = x;//lpCurPlayer->pX;
										Pos1.y = y;//lpCurPlayer->pY;
										Pos1.z = z;//lpCurPlayer->pZ;

										Pos2.x = lpChar->pX;
										Pos2.y = lpChar->pY+24*fONE;
										Pos2.z = lpChar->pZ;

										AssaParticle_SparkShieldDefence( &Pos1 , &Pos2 );
										SkillPlaySound( SKILL_SOUND_SKILL_SPARK2	 , lpCurPlayer->pX,lpCurPlayer->pY,lpCurPlayer->pZ );
									}
								}
								*/
							}
							//디바인 인홀레이션
							if ( lpCurPlayer->smCharInfo.JOB_CODE==JOBCODE_KNIGHT ) {
								cnt = (lpTransAttackData->AttackState>>16)&0xFF;
								if ( cnt>0 ) {
									sinSetLife((sinGetLife()+cnt));
									ResetEnergyGraph( 0 );						//에너지 그래프 첵크 초기화

									if ( lpCurPlayer->HvLeftHand.PatTool ) {
										if ( lpCurPlayer->GetToolBipPoint( &lpCurPlayer->HvLeftHand , &x,&y,&z ) ) {
											Pos1.x = x;
											Pos1.y = y;
											Pos1.z = z;

											AssaParticle_DivineInhalationDefence( &Pos1 );
										}
									}
								}
							}
						}
					}
				}

				//블럭율 계산
				if ( lpChar && ( !block || act ) ) { 

					//방어율 계산
					//cnt = GetPlayDefence( &smCharInfo , &lpCurPlayer->smCharInfo );
					if ( sizeof(TRANS_ATTACKDATA)==lpTransAttackData->size )
						cnt = sinGetMonsterAccuracy( lpTransAttackData->sRating[0] , lpTransAttackData->sRating[1] );
					else
						cnt = sinGetMonsterAccuracy( smCharInfo.Level , smCharInfo.Attack_Rating );

					if ( dmAttackCount<2 ) cnt<<=2;			//공격을 안할시에는 방어률 절반

					//cnt = 100;
					if ( (rand()%100)<=cnt || act ) {	

						cnt = lpTransAttackData->Power;

						if ( (lpCurPlayer->dwDispAppSkill&SKILL_APPMASK_HOLY_BODY) ) {
							if ( smCharInfo.Brood==smCHAR_MONSTER_UNDEAD ) {
								//홀리 바디 스킬 적용 ( 언데드만 적용 )
								//cnt -= (cnt*HolyBody_Absorb[DispApp_SkillPoint-1])/100;
								HolyBody_Undead = TRUE;
							}
						}

						if ( lpCurPlayer->smCharInfo.JOB_CODE==JOBCODE_MAGICIAN ) {
							//에너지 쉴드 적용 확인
							temp = lpTransAttackData->AttackState>>16;
							if ( temp>0 ) {
								AssaParticle_EnergyShieldDefence( lpCurPlayer );
								temp = sinGetMana()-temp;
								if ( temp<0 ) temp = 0;
								sinSetMana( temp );
								ResetEnergyGraph( 1 );						//에너지 그래프 첵크 초기화
							}
						}

						if ( lpCurPlayer->smCharInfo.JOB_CODE==JOBCODE_PRIESTESS ) {
							//서몬 무스펠
							if ( lpChar->smCharInfo.Brood==smCHAR_MONSTER_UNDEAD && (lpCurPlayer->dwDispAppSkill&SKILL_APPMASK_GODLY_SHIELD)!=0 ) {
								temp = lpTransAttackData->AttackState>>16;
								if ( temp>0 ) {
									temp = sinGetLife()+temp;
									sinSetLife( temp );
									ResetEnergyGraph( 0 );						//에너지 그래프 첵크 초기화
								}
							}
						}

						bStunCount = lpCurPlayer->PlayStunCount;
						lpCurPlayer->PlayStunCount = 0;

						/////////////////// PKMODE ///////////////////
						switch( Param1 ) {
							case SKILL_PLAY_ROAR:
							case SKILL_PLAY_SHIELD_STRIKE:
								if ( !bStunCount ) {
									lpCurPlayer->PlayStunCount = GetResistancePower( Param2*70 , sITEMINFO_BIONIC );
									AssaParticle_Stune( lpCurPlayer , Param2+1 );
								}
								break;
							case SKILL_PLAY_GROUND_PIKE:
								lpCurPlayer->PlaySlowCount = GetResistancePower( Param2*70 , sITEMINFO_ICE );
								lpCurPlayer->PlaySlowSpeed = 180;
								lpCurPlayer->AttackIce = 1;
								break;
							case SKILL_PLAY_GLACIAL_SPIKE:
							case SKILL_PLAY_ICE_CRYSTAL:		//공성 얼음탑
								lpCurPlayer->PlaySlowCount = GetResistancePower( Param2*70 , sITEMINFO_ICE );
								lpCurPlayer->PlaySlowSpeed = 200;
								lpCurPlayer->AttackIce = 1;
								break;
							case SKILL_PLAY_DISTORTION:
								lpCurPlayer->PlaySlowCount = Param2*70;
								lpCurPlayer->PlaySlowSpeed = 210;
								lpCurPlayer->PlayDistortion = lpCurPlayer->PlaySlowCount;
								lpCurPlayer->AttackIce = 0;
								break;
							case SKILL_PLAY_ENCHANT_WEAPON:
								lpCurPlayer->PlaySlowCount = GetResistancePower( 100 , sITEMINFO_ICE );
								lpCurPlayer->PlaySlowSpeed = Param2*70;
								lpCurPlayer->AttackIce = 1;
								break;
							case SKILL_PLAY_DANCING_SWORD:
								lpCurPlayer->PlaySlowCount = GetResistancePower( 140 , sITEMINFO_ICE );
								lpCurPlayer->PlaySlowSpeed = Param2*70;
								lpCurPlayer->AttackIce = 1;
								break;
							case SKILL_PLAY_FROST_JAVELIN:
								lpCurPlayer->PlaySlowCount = GetResistancePower( 210 , sITEMINFO_ICE );
								lpCurPlayer->PlaySlowSpeed = Param2*70;
								lpCurPlayer->AttackIce = 1;
								break;
							case SKILL_PLAY_FIRE_CRYSTAL:		//공성 불탑
								cnt = GetResistancePower( cnt , sITEMINFO_FIRE );
								break;
							case SKILL_PLAY_LIGHTNING_CRYSTAL:	//공성 전기탑
								if ( !bStunCount ) {
									lpCurPlayer->PlayStunCount = GetResistancePower( Param2*70 , sITEMINFO_LIGHTING );
									AssaParticle_Stune( lpCurPlayer , Param2+1 );
								}
								break;
						}


						switch( smCharInfo.dwCharSoundCode ) {
						case snCHAR_SOUND_COLDEYE:
						case snCHAR_SOUND_FROZEN:
						//case snCHAR_SOUND_ICEGOLEM:
							if ( (lpTransAttackData->AttackState&0xFF)==0x80 )						//스킬공격
								lpCurPlayer->PlaySlowCount = GetResistancePower( 70*4 , sITEMINFO_ICE );
								lpCurPlayer->PlaySlowSpeed = 200;
								lpCurPlayer->AttackIce = 1;
							break;
						case snCHAR_SOUND_FROST:
						case snCHAR_SOUND_DEATHKNIGHT:
							if ( (lpTransAttackData->AttackState&0xFF)==0x80 )						//스킬공격
								lpCurPlayer->PlaySlowCount = GetResistancePower( 70*6 , sITEMINFO_ICE );
								lpCurPlayer->PlaySlowSpeed = 200;
								lpCurPlayer->AttackIce = 1;
							break;

						case snCHAR_SOUND_CHAOSCARA:
							//카오스카라 스킬
							if ( Param1==SKILL_PLAY_CHAOSCARA_VAMP && LastSkillParam ) {
								sinSetMana( sinGetMana()-cnt );
							}
							break;

						case snCHAR_SOUND_STINGRAY:
							if (chrAttackTarget && (lpTransAttackData->AttackState&0xFF)==0x80 )	{					//스킬공격
								ParkAssaParticle_Posion( chrAttackTarget , STINGRAY_POSION_TIME );		// 독걸린거 표현	(캐릭터)
								if ( chrAttackTarget==lpCurPlayer ) {
									len = GetResistancePower( 70*STINGRAY_POSION_TIME , sITEMINFO_POISON );
									lpCurPlayer->PlayPoison[0] = 1;
									lpCurPlayer->PlayPoison[1] = len;
								}
							}
							break;

						case snCHAR_SOUND_KINGBAT: // 박재원 - 저주받은 신전 3층(신규필드 몬스터) - 킹뱃
							ParkAssaParticle_Posion( chrAttackTarget , STINGRAY_POSION_TIME );		// 독걸린거 표현	(캐릭터)
							if ( chrAttackTarget==lpCurPlayer ) {
								len = GetResistancePower( 70*STINGRAY_POSION_TIME , sITEMINFO_POISON );
								lpCurPlayer->PlayPoison[0] = 1;
								lpCurPlayer->PlayPoison[1] = len;
							}
							break;

						case snCHAR_SOUND_DEVIL_BIRD:
							if (chrAttackTarget ) {
								ParkDevilBirdAttackNormal( chrAttackTarget );
							}
							break;

							/////////////////////////

						case snCHAR_SOUND_BLOODYKNIGHT:
							if (chrAttackTarget && (lpTransAttackData->AttackState&0xFF)==0x80 )	{					//스킬공격
								ParkAssaParticle_BloodySkillStun(chrAttackTarget);
								lpCurPlayer->PlayStunCount = 70*3;
							}
							break;
						case snCHAR_SOUND_HELLHOUND:
							if (chrAttackTarget && (lpTransAttackData->AttackState&0xFF)==0x80 )	{					//스킬공격
								ParkAssaParticle_HellHoundSkillTaget(chrAttackTarget);
							}
							break;
						case snCHAR_SOUND_CHIMERA:
							if (chrAttackTarget && (lpTransAttackData->AttackState&0xFF)==0x80 )	{					//스킬공격
								lpCurPlayer->PlayStunCount = 70*3;
							}
							break;


						}


						//////////////////////////////////////////////

						//흡수율 계산
						if ( PkMode_CharState==smCHAR_STATE_USER )
							cnt = (cnt - ((lpCurPlayer->smCharInfo.Absorption*PKMODE_ABSORB)/100));
						else
							cnt = (cnt - lpCurPlayer->smCharInfo.Absorption);
/*
						if ( (lpCurPlayer->dwDispAppSkill&SKILL_APPMASK_HOLY_BODY) ) {
							if ( smCharInfo.Brood==smCHAR_MONSTER_UNDEAD ) {
								//홀리 바디 스킬 적용 ( 언데드만 적용 )
								cnt -= HolyBody_Absorb[DispApp_SkillPoint-1];
								HolyBody_Undead = TRUE;
							}
						}
*/
						//if ( smCharInfo.State==smCHAR_STATE_USER )	cnt = (cnt*80)/100;		//PK모드 생명감소 반으로 줄임

						if ( cnt<=0 ) cnt = 1;

							if ( abs(lpChar->smCharInfo.Level-lpCurPlayer->smCharInfo.Level)<=AGING_SUB_LEVEL ) {
								//레벨 7차이 이하는 에이징 적용
							//	sinCheckAgingLevel( SIN_AGING_DEFENSE , false);
								sinCheckAgingLevel( SIN_AGING_DEFENSE_ARMOR , false);
								sinCheckAgingLevel( SIN_AGING_DEFENSE_ORB , false);
								sinCheckAgingLevel( SIN_AGING_DEFENSE_BOOTS , false);   // 박재원 - 에이징 아이템 추가(부츠)
								sinCheckAgingLevel( SIN_AGING_DEFENSE_GLOVES , false);  // 박재원 - 에이징 아이템 추가(장갑)
								sinCheckAgingLevel( SIN_AGING_DEFENSE_ARMLETS , false); // 박재원 - 에이징 아이템 추가(암릿)
							}

							CheckCharForm();

							sinSetLife( sinGetLife()-cnt );
							//명중 ( 공격 성공 )
							//lpCurPlayer->smCharInfo.Life[0] -= cnt;

							if ( smCharInfo.dwCharSoundCode==snCHAR_SOUND_SUCCUBUS ) {
								if ( lpTransAttackData->sRating[0]>75 ) {
									if ( lpTransAttackData->sRating[0]<80 )	//서큐버스
										sinSetStamina( sinGetStamina()-((lpCurPlayer->smCharInfo.Stamina[1]*6)/100) );
									else 	//인큐버스
										sinSetMana( sinGetMana()-((lpCurPlayer->smCharInfo.Mana[1]*6)/100) );
								}
							}

							//피튀기는 파티클
							StartEffect( lpCurPlayer->pX, lpCurPlayer->pY+16*fONE, lpCurPlayer->pZ, EFFECT_DAMAGE1 );
							SetDynLight( lpCurPlayer->pX, lpCurPlayer->pY+16*fONE, lpCurPlayer->pZ, 160, 0,0,0, 180 );

							if ( ( (lpCurPlayer->dwDispAppSkill&SKILL_APPMASK_PHYSICAL_ABSORB) && (lpCurPlayer->dwDispAppSkill&SKILL_APPMASK_HOLY_BODY)==0 ) || HolyBody_Undead ) {
								//피지클업솝 적용이펙트
								StartEffect( lpCurPlayer->pX, lpCurPlayer->pY+24*fONE , lpCurPlayer->pZ , EFFECT_PHYSICAL_ABSORB_DAMAGE );
							}

							if ( lpCurPlayer->MotionInfo->State != CHRMOTION_STATE_DAMAGE && lpCurPlayer->MotionInfo->State!=CHRMOTION_STATE_EAT && lpCurPlayer->MotionInfo->State!=CHRMOTION_STATE_ATTACK &&
								lpCurPlayer->MotionInfo->State!=CHRMOTION_STATE_SKILL && cnt>1 ) {

								lpCurPlayer->SetMotionFromCode( CHRMOTION_STATE_DAMAGE );
								CharPlaySound( lpCurPlayer );
							}

							//임시 처리
							if ( sinGetLife()<=0 ) {

								if ( sinQuest_ChangeJob4.CODE==SIN_QUEST_CODE_CHANGEJOB4 && 
									lpCurPlayer->OnStageField>=0 && StageField[ lpCurPlayer->OnStageField ]->FieldCode==QUEST_ARENA_FIELD ) {
										//퀘스트 대전장
						 				OpenEachPlayer( this );
										sinBattleNpcResult( 0 );		//3차 전업 마스터 한테 졌다
										sinSetLife( 0 );
										lpCurPlayer->SetMotionFromCode( CHRMOTION_STATE_DEAD );
										CharPlaySound( lpCurPlayer );
										ResetAttackTrans();
										return TRUE;
									}
								lpCurPlayer->SetMotionFromCode( CHRMOTION_STATE_DEAD );
								sinSetLife( 0 );
								CharPlaySound( lpCurPlayer );

								SendCommand_AreaServer( smTRANSCODE_DEAD_USER , dwObjectSerial ,0,0,0 );
								SendCommand_DataServer( smTRANSCODE_DEAD_USER , dwObjectSerial ,1,0,0 );
								WeaponEffect = 0;
								dwWeaponEffectTime = 0;
				
								#ifdef LOG_THE_PK
								if ( smCharInfo.State!=smCHAR_STATE_USER && PkMode_CharState!=smCHAR_STATE_USER )
								#else
								if ( smCharInfo.State!=smCHAR_STATE_USER )
								#endif
								{//해외
									DeadPlayerExp( lpCurPlayer );				//죽어서 경험치 깍임 (PK당했을때는 제외)
								}

								sinCheckQuest85Die();		//퀘스트 수행중 죽었을 경우

								//게임을 저장
								SaveGameData();
								
								#ifdef LOG_THE_PK
								if ( smCharInfo.State==smCHAR_STATE_USER || PkMode_CharState==smCHAR_STATE_USER ) 
								#else
								if ( smCharInfo.State==smCHAR_STATE_USER ) 
								#endif
								{//해외
									//PK당해서 죽음
									PlayerKilling( dwObjectSerial );
								}
								ResetAttackTrans();	//lpTransAttackData->code = 0;
								SetFullZoomMap(0);	// pluto 죽었을때 지도 못 열게 하려고
								return TRUE;
							}
							ReformCharForm();					//캐릭터 정보 인증

					}
					else {
						//회피(방어) 성공
						cInterFace.SetStringEffect(HA_DEFANCE);
						Record_DefenceCount++;
					}
				}
				dwBattleTime = dwPlayTime;			//전투 시간 기록
			}
		}

		ResetAttackTrans();	//lpTransAttackData->code = 0;


	return TRUE;
}


//위치를 받아 예측데이타를 만들어 넣는다
int smCHAR::MakePlayBuffFromPosi( int px, int py, int pz , int anX,int anY, int anZ , int act )
{
	int len;
	int sx,sy,sz;
	int x,y,z;
	int mx,my,mz;
	int PlayBuffPosi;
	int cnt;
	int aY;
	int dist;
	//smMOTIONINFO	*lpMotionInfo;
	//int SelFrame;

	if ( !smMotionInfo ) return FALSE;

	len=0x40;


	if (PlayBuffPosi_End<=0) {
		sx = px;
		sy = py;
		sz = pz;
	}
	else {
		PlayBuffPosi = (PlayBuffPosi_End-1) & PLAYBUFF_MASK;
		sx = PlayBuff[ PlayBuffPosi ].x;
		sy = PlayBuff[ PlayBuffPosi ].y;
		sz = PlayBuff[ PlayBuffPosi ].z;
	}

	if ( sx==px && sy==py && sz==pz )
		aY = anY;
	else
		aY = GetRadian2D( sx,sz, px, pz );

	x = (px-sx)>>FLOATNS;
	z = (py-sy)>>FLOATNS;

	dist = x*x+z*z;
	if ( dist>(110*110) ) {
		//예측 데이타 간격이 너무 긴 경우

		PlayBuffPosi = PlayBuffPosi_End & PLAYBUFF_MASK;

		PlayBuff[ PlayBuffPosi ].x = px;
		PlayBuff[ PlayBuffPosi ].y = py;
		PlayBuff[ PlayBuffPosi ].z = pz;

		PlayBuff[ PlayBuffPosi ].angle[0] = 0;
		PlayBuff[ PlayBuffPosi ].angle[1] = aY;
		PlayBuff[ PlayBuffPosi ].angle[2] = 0;
		PlayBuff[ PlayBuffPosi ].angle[3] = 0;

		PlayBuff[ PlayBuffPosi ].frame = smPLAYBUFF_FRAME_NONE|80;
		PlayBuff[ PlayBuffPosi ].dwTargetSerial = 0;

		memcpy( PlayBuff[ PlayBuffPosi ].wStickItems , wStickItems , sizeof( short )*4 );

		PlayBuffPosi_End++;
		return TRUE;
	}

	mx = (( px - sx )<<8) / len;
	my = (( py - sy )<<8) / len;
	mz = (( pz - sz )<<8) / len;

	x=0;	y=0;	z=0;

	for(cnt=0;cnt<len;cnt++) {
		PlayBuffPosi = (PlayBuffPosi_End+cnt) & PLAYBUFF_MASK;

		PlayBuff[ PlayBuffPosi ].x = sx+(x>>8);
		PlayBuff[ PlayBuffPosi ].y = sy+(y>>8);
		PlayBuff[ PlayBuffPosi ].z = sz+(z>>8);

		PlayBuff[ PlayBuffPosi ].angle[0] = 0;
		PlayBuff[ PlayBuffPosi ].angle[1] = aY;
		PlayBuff[ PlayBuffPosi ].angle[2] = 0;
		PlayBuff[ PlayBuffPosi ].angle[3] = 0;

		PlayBuff[ PlayBuffPosi ].frame = smPLAYBUFF_FRAME_NONE|80;
		PlayBuff[ PlayBuffPosi ].dwTargetSerial = 0;

		memcpy( PlayBuff[ PlayBuffPosi ].wStickItems , wStickItems , sizeof( short )*4 );

		x += mx;
		y += my;
		z += mz;
	}
	PlayBuffPosi_End = PlayBuffPosi_End + len;

	return TRUE;
}

//전송 받은 데이타로 PLAYBUFF에 예측 데이타를 만들어 넣는다
int smCHAR::MakePlayBuffFromRecvData( smPLAYBUFF *StartBuff , smPLAYBUFF *EndBuff , int len )
{
	int cnt;
	int x,y,z;
	int px,py,pz;
	int mx,my,mz;
	int PlayBuffPosi;
	int aX,aY,aZ;
	int saX,saY,saZ;
	int taX,taY,taZ;
	int frameState;
	int frameStep;

	if ( !smMotionInfo || len<=1 ) return FALSE;

	if ( !EndBuff ) {
		PlayBuffPosi = PlayBuffPosi_End & PLAYBUFF_MASK;

		PlayBuff[ PlayBuffPosi ].x = StartBuff->x;
		PlayBuff[ PlayBuffPosi ].y = StartBuff->y;
		PlayBuff[ PlayBuffPosi ].z = StartBuff->z;

		PlayBuff[ PlayBuffPosi ].angle[0] = StartBuff->angle[0] & ANGCLIP;
		PlayBuff[ PlayBuffPosi ].angle[1] = StartBuff->angle[1] & ANGCLIP;
		PlayBuff[ PlayBuffPosi ].angle[2] = StartBuff->angle[2] & ANGCLIP;
		PlayBuff[ PlayBuffPosi ].angle[3] = StartBuff->angle[3];

		PlayBuff[ PlayBuffPosi ].frame = StartBuff->frame;
		PlayBuff[ PlayBuffPosi ].dwTargetSerial = StartBuff->dwTargetSerial;

		memcpy( PlayBuff[ PlayBuffPosi ].wStickItems , StartBuff->wStickItems , sizeof( short )*4 );
		PlayBuffPosi_End ++;
		return TRUE;
	}

	px = StartBuff->x;
	py = StartBuff->y;
	pz = StartBuff->z;

	mx = (( EndBuff->x - px )<<8) / len;
	my = (( EndBuff->y - py )<<8) / len;
	mz = (( EndBuff->z - pz )<<8) / len;

	aX = StartBuff->angle[0];
	aY = StartBuff->angle[1];
	aZ = StartBuff->angle[2];

	taX = ANGLE_180 - EndBuff->angle[0];
	taY = ANGLE_180 - EndBuff->angle[1];
	taZ = ANGLE_180 - EndBuff->angle[2];

	saX = (aX + taX) & ANGCLIP;
	saY = (aY + taY) & ANGCLIP;
	saZ = (aZ + taZ) & ANGCLIP;

	saX = ((saX-ANGLE_180)<<8) / len;
	saY = ((saY-ANGLE_180)<<8) / len;
	saZ = ((saZ-ANGLE_180)<<8) / len;

	if ( StartBuff->frame<CHAR_FRAME_MASK ) {
		frameState = smMotionInfo->MotionInfo[ StartBuff->angle[3] ].State;
		if ( frameState==CHRMOTION_STATE_WALK || frameState==CHRMOTION_STATE_RUN ) {
			aY = GetRadian2D( px,pz, EndBuff->x, EndBuff->z );
			saY = 0;
		}
	}


	x=0;
	y=0;
	z=0;

	taX = 0;
	taY = 0;
	taZ = 0;

	cnt = len-1;
	if ( cnt<=0 ) {
		frameStep = 80;
	}
	else {
		frameStep = ( EndBuff->frame-StartBuff->frame )/cnt;
	}

	if ( frameStep>=4096 || frameStep<20 ) {
		frameStep = 80;
	}

	for(cnt=0;cnt<len;cnt++) {
		PlayBuffPosi = (PlayBuffPosi_End+cnt) & PLAYBUFF_MASK;

		PlayBuff[ PlayBuffPosi ].x = px+(x>>8);
		PlayBuff[ PlayBuffPosi ].y = py+(y>>8);
		PlayBuff[ PlayBuffPosi ].z = pz+(z>>8);
/*
		PlayBuff[ PlayBuffPosi ].angle[0] = (aX+(taX>>8)) & ANGCLIP;
		PlayBuff[ PlayBuffPosi ].angle[1] = (aY+(taY>>8)) & ANGCLIP;
		PlayBuff[ PlayBuffPosi ].angle[2] = (aZ+(taZ>>8)) & ANGCLIP;
*/
		PlayBuff[ PlayBuffPosi ].angle[0] = aX & ANGCLIP;
		PlayBuff[ PlayBuffPosi ].angle[1] = aY & ANGCLIP;
		PlayBuff[ PlayBuffPosi ].angle[2] = aZ & ANGCLIP;

		PlayBuff[ PlayBuffPosi ].angle[3] = StartBuff->angle[3];

		PlayBuff[ PlayBuffPosi ].frame = smPLAYBUFF_FRAME_NONE|frameStep;

		PlayBuff[ PlayBuffPosi ].dwTargetSerial = StartBuff->dwTargetSerial;

		memcpy( PlayBuff[ PlayBuffPosi ].wStickItems , StartBuff->wStickItems , sizeof( short )*4 );

		x += mx;
		y += my;
		z += mz;

		taX += saX;
		taY += saY;
		taZ += saZ;
	}

	PlayBuff[ PlayBuffPosi_End&PLAYBUFF_MASK ].frame = StartBuff->frame;
	PlayBuff[ (PlayBuffPosi_End+len-1)&PLAYBUFF_MASK ].frame = EndBuff->frame;

	PlayBuffPosi_End = PlayBuffPosi_End + len;

	return TRUE;
}


//통신 오류 체크하여 보정
int smCHAR::TransDelayCheck()
{
	int Delay;

	if ( TransSock && TransSock->ConnectDirection ) {
		Delay = PlayCounter-TransLastSendCnt;

		if ( Delay>TransDelayMax ) {
			TransSendWait = 0;
			TransLastSendCnt = PlayCounter-16;
			return TRUE;
		}
	}

	return FALSE;
}

//거리별 전송 시간차를 조절 한다
int smCHAR::SetDistSendCnt( smCHAR *player )
{
	int x,y,z;
	int dist;

	x = (pX-player->pX)>>FLOATNS;
	y = (pY-player->pY)>>FLOATNS;
	z = (pZ-player->pZ)>>FLOATNS;

	dist = x*x+y*y+z*z;

	if ( dist<DIST_TRANSLEVEL_HIGH || !DisplayFlag ) {
		TransMinCnt = 8;
		TransLevel = 0;
		TransDelayMax = PLAYCOUNT_PER_SECOND * 4;		//4초
		return TRUE;
	}
	if ( dist<DIST_TRANSLEVEL_MID ) {
		TransMinCnt = PLAYCOUNT_PER_SECOND;
		TransLevel = 1;
		TransDelayMax = PLAYCOUNT_PER_SECOND * 6;		//8초
		return TRUE;
	}
	if ( dist<DIST_TRANSLEVEL_LOW ) {
		TransMinCnt = PLAYCOUNT_PER_SECOND * 5;
		TransLevel = 2;
		TransDelayMax = PLAYCOUNT_PER_SECOND * 10;		//5초
		return TRUE;
	}

	TransMinCnt = PLAYCOUNT_PER_SECOND * 15;		//15초
	TransLevel = 3;
	TransDelayMax = PLAYCOUNT_PER_SECOND * 25;		//25초

	return TRUE;
}

//상대 플레이어의 데이타 초기 설정
int smCHAR::FormSetPlayerInfo()
{

	int cnt=0;

	if ( !Pattern ) {
		//패턴 오류 경고 - 로그에 기록
		char szBuff[128];

		wsprintf( szBuff , "Mesh Pattern Error [ %s ] ( %s %s )" , smCharInfo.szName , smCharInfo.szModelName , smCharInfo.szModelName2 );
		Record_ClinetLogFile( szBuff );
	}


		PHeight = pY;

		smCharInfo.bUpdateInfo[1] = 0xFF;			//스킬포인트 초기화

		if ( this!=lpCurPlayer && lpCurPlayer->MotionInfo->State==CHRMOTION_STATE_STAND && 
			abs(lpCurPlayer->pX-pX)<32*fONE && abs(lpCurPlayer->pZ-pZ)<32*fONE ) {

			if ( lpCurPlayer->OnStageField>=0 ) {
				if ( StageField[lpCurPlayer->OnStageField]->CheckStartPoint( lpCurPlayer->pX , lpCurPlayer->pZ )==TRUE ) {
						lpCurPlayer->MoveFlag = TRUE;
						lpCurPlayer->MoveCnt = 100;
				}
			}
		}

		//신규 입력 캐릭터 바닥 확인
		if ( smCharInfo.State!=smCHAR_STATE_ENEMY ) {
			OnStageField = -1;
			if ( smGameStage[0] && Pattern ) {
				cnt = smGameStage[0]->GetFloorHeight( pX,pY,pZ , Pattern->SizeHeight );	
				OnStageField = 0;
			}
			if ( cnt==CLIP_OUT && smGameStage[1] ) {
				OnStageField = 1;
			}
		}

		PatSizeLevel = smCharInfo.SizeLevel;
		if ( PatSizeLevel>=0x1000 ) PatSizeLevel = 1;

		if ( TransSock && TransSock->ConnectDirection ) 	TransSendWait = 0;
		else TransSendWait = 1;

		TransLastSendCnt = PlayCounter;

		TransMinCnt = 8;
		TransLevel = 0;
		TransDelayMax = PLAYCOUNT_PER_SECOND * 4;		//4초
	
		PlayBuffPosi_End = 0;
		PlayBuffCnt = 0;

		DisplayAlpha = -256;
		DispLifeBar = 0;

		switch( smCharInfo.dwCharSoundCode ) {
		case snCHAR_SOUND_FIGON:
			//파이곤 손에 불 붙이기
			if ( Pattern && Pattern->TmParent ) {
				HvLeftHand.ObjBip = Pattern->TmParent->GetObjectFromName( "Bip01 L Finger1" );
				HvRightHand.ObjBip = Pattern->TmParent->GetObjectFromName( "Bip01 R Finger1" );
			}
			else {
				HvLeftHand.ObjBip = 0;
				HvRightHand.ObjBip = 0;
			}
			break;

		case snCHAR_SOUND_ILLUSIONKNIGHT:
			//일루전 나이트 - 반투명
			RendAlpha = -180;
			break;

		case snCHAR_SOUND_MUDY:
			//머디
			RenderLatter = TRUE;
			break;

		case snCHAR_SOUND_WOLVERLIN:
			//울버린 색상
			if ( smCharInfo.State==smCHAR_STATE_ENEMY ) {
				sAddColorEffect[SMC_R] = 100;
				sAddColorEffect[SMC_G] = 100;
			}
			break;

		case snCHAR_SOUND_SEN:
			//센 조명
			if ( Pattern && Pattern->TmParent ) {
				HvLeftHand.ObjBip = Pattern->TmParent->GetObjectFromName( "Bip01" );
			}
			else {
				HvLeftHand.ObjBip = 0;
				HvRightHand.ObjBip = 0;
			}
			break;

		case snCHAR_SOUND_MUTANTTREE:
			//뮤턴트 트리 연기
			if ( Pattern && Pattern->TmParent ) {
				HvLeftHand.ObjBip = Pattern->TmParent->GetObjectFromName( "Bip05" );
			}
			else {
				HvLeftHand.ObjBip = 0;
				HvRightHand.ObjBip = 0;
			}
			break;

		case snCHAR_SOUND_SHADOW:
			//쉐도우
			if ( Pattern && Pattern->TmParent ) {
				HvLeftHand.ObjBip = Pattern->TmParent->GetObjectFromName( "Bip01 L UpperArm" );
				HvRightHand.ObjBip = Pattern->TmParent->GetObjectFromName( "Bip01 R UpperArm" );
			}
			else {
				HvLeftHand.ObjBip = 0;
				HvRightHand.ObjBip = 0;
			}
			break;

		case snCHAR_SOUND_MRGHOST:
			RendAlpha = -80;
			break;

		case snCHAR_SOUND_FURY:
			AssaParticle_Fury_Eye(this);			//퓨리 눈빛

			switch( smCharInfo.Level )				//퀘스트용 퓨리에 색상 설정
			{
			case 118:
				sAddColorEffect[SMC_R] = 100;
				sAddColorEffect[SMC_G] = 100;
				RendAlpha = -80;
				break;
			case 119:
				sAddColorEffect[SMC_R] = 100;
				RendAlpha = -80;
				break;
			case 120:
				sAddColorEffect[SMC_B] = 100;
				RendAlpha = -80;
				break;
			}
			break;

		case snCHAR_SOUND_BEEVIL:							//벌
			RenderLatter = TRUE;
			break;

		case snCHAR_SOUND_GUARDIAN_SAINT:		//가디언-세인트 ( 몸에서 빛이 점멸 )
			sAddColorEffect[SMC_R] = 80;
			sAddColorEffect[SMC_G] = 80;
			sAddColorEffect[SMC_B] = 20;

			AssaParticleClanMonster(this);
			break;

		case  snCHAR_SOUND_CHAINGOLEM:			//체인골렘
			AssaParticle_MonsterIronChainGolem(this);
			break;

		case snCHAR_SOUND_RUNICGUARDIAN:		//루닉 가디안
			AssaParticle_MonsterRunicGuardianHand( this );
			break;

		case snCHAR_SOUND_BABEL:				//바벨 (루닉가디안 보스)
			AssaParticle_MonsterRunicGuardianBossWeapon(this);
			break;

		case snCHAR_SOUND_CHAOSCARA:
			ParkAssaParticle_MonsterIceChaosKara(this);
			break;

		case snCHAR_SOUND_DEATHKNIGHT:			//발렌토
			ParkAssaParticle_MonsterIceValentoBody(this);
			break;

		case snCHAR_SOUND_S_F_ELEMENTAL:		//소환몹 화이어엘리멘탈
			SkillArchMageFireElementalHand( this );
			break;

		case snCHAR_SOUND_NPC_SKILLMASTER:		//스킬마스터
			if ( Pattern && Pattern->TmParent )
				AttackObjBip = Pattern->TmParent->GetObjectFromName( "bip01 weapon" );
			break;

		case snCHAR_SOUND_NPC_MAGICMASTER:		//매직마스터
			if ( Pattern && Pattern->TmParent )
				AttackObjBip = Pattern->TmParent->GetObjectFromName( "Bip01 staff02" );
			break;

		case snCHAR_SOUND_CASTLE_SOLDER_A:
		case snCHAR_SOUND_CASTLE_SOLDER_B:
		case snCHAR_SOUND_CASTLE_SOLDER_C:
			if ( Pattern && Pattern->TmParent )
				AttackObjBip = Pattern->TmParent->GetObjectFromName( "Bip01 Effect" );
			break;

		case snCHAR_SOUND_KELVEZU:
			ParkKelvezuJin( this );
			break;
		case snCHAR_SOUND_REVIVED_KNIGHT:	// pluto 신규필드 몬스터 되살아난 나이트
			if( Pattern && Pattern->TmParent )
			{
				HvRightHand.ObjBip = Pattern->TmParent->GetObjectFromName( "Bip01 R Hand" );
			}
			break;
		case snCHAR_SOUND_REVIVED_PIKEMAN:
			if( Pattern && Pattern->TmParent )
			{
				AttackObjBip = Pattern->TmParent->GetObjectFromName( "bip01 wea" );
			}
			break;
		case snCHAR_SOUND_REVIVED_ARCHER:
			if( Pattern && Pattern->TmParent )
			{
				HvRightHand.ObjBip = Pattern->TmParent->GetObjectFromName( "Bip01 R Hand" );
			}
			break;
		}



		if ( smCharInfo.wPlayClass[0]==MONSTER_CLASS_BOSS ) {
			//보스 몬스터
			if ( smCharInfo.State==smCHAR_STATE_ENEMY ) {
				sAddColorEffect[SMC_R] = 130;
				sAddColorEffect[SMC_G] = 50;
				sAddColorEffect[SMC_B] = 70;
			}
		}

	return TRUE;
}

//수신 받은 상대 플레이어의 데이타 설정
int smCHAR::SetTransPlayerInfo( smTRNAS_PLAYERINFO *lpTransPlayerInfo )
{

	memcpy( &smCharInfo , &lpTransPlayerInfo->smCharInfo , sizeof( smCHAR_INFO ) );
	ResetAttackTrans();

	if ( !PatLoading )
		AddLoaderPattern( (smCHAR *)&Head , smCharInfo.szModelName , smCharInfo.szModelName2 );

	pX = lpTransPlayerInfo->x;
	pY = lpTransPlayerInfo->y;
	pZ = lpTransPlayerInfo->z;

	FormSetPlayerInfo();

	if ( smCharInfo.ClassClan ) {
		//클랜 정보
		ClanInfoNum = ReadClanInfo( smCharInfo.ClassClan );
		dwClanInfoTime = dwPlayTime;

		if ( GetClanCode(smCharInfo.ClassClan)==rsBlessCastle.dwMasterClan )	//캐슬 마스터 클랜
			Clan_CastleMasterFlag = TRUE;
	}	

	PkMode_CharState = 0;

	return TRUE;
}

//수신 받은 상대 플레이어의 데이타 설정
int smCHAR::SetTransPlayerInfoQuick( smTRNAS_PLAYERINFO_QUICK *lpTransPlayerInfo )
{
	int len;

	len = sizeof(smTRNAS_PLAYERINFO_QUICK) - lpTransPlayerInfo->size;

	ZeroMemory( &smCharInfo , sizeof(smCHAR_INFO) );
	smCharInfo.Life[0] = 10;
	smCharInfo.Life[1] = 50;

	memcpy( &smCharInfo , &lpTransPlayerInfo->smCharInfo , sizeof( smCHAR_INFO )-len );
	ResetAttackTrans();

	if ( !PatLoading )
		AddLoaderPattern( (smCHAR *)&Head , smCharInfo.szModelName , smCharInfo.szModelName2 );

	pX = lpTransPlayerInfo->x;
	pY = lpTransPlayerInfo->y;
	pZ = lpTransPlayerInfo->z;

	FormSetPlayerInfo();

	if ( smCharInfo.ClassClan ) {
		//클랜 정보
		ClanInfoNum = ReadClanInfo( smCharInfo.ClassClan );
		dwClanInfoTime = dwPlayTime;

		if ( GetClanCode(smCharInfo.ClassClan)==rsBlessCastle.dwMasterClan )	//캐슬 마스터 클랜
			Clan_CastleMasterFlag = TRUE;
	}
	
	PkMode_CharState = 0;

	return TRUE;
}

//데이타 입수 ( 그룹 처리용 )
int smCHAR::RecvPlayData2( char *lpData )
{
	smPLAYDATA *lpPlayData;
	int RecvCode;
	int cnt;
	int len;
	int LastBuffPosi_End;
	int act;
	char *lpBuff;
	int	Code,Size;
	smCHAR	*lpChar;

	RecvCode = ((int *)lpData)[1];

	switch( RecvCode ) {
	case smTRANSCODE_PLAYDATA1:
		//게임 진행 수신
		lpPlayData = (smPLAYDATA *)lpData;

		act = lpPlayData->angle[3];
		//위치를 받아 예측데이타를 만들어 넣는다
		MakePlayBuffFromPosi( lpPlayData->x , lpPlayData->y , lpPlayData->z ,lpPlayData->angle[0] , lpPlayData->angle[1], lpPlayData->angle[2], act );
		MoveCnt = 0;

/*
		if ( PlayBuffCnt==PlayBuffPosi_End ) {

			pX = lpPlayData->x;
			pY = lpPlayData->y;
			pZ = lpPlayData->z;

			Angle.x = lpPlayData->angle[0] & ANGCLIP;
			Angle.y = lpPlayData->angle[1] & ANGCLIP;
			Angle.z = lpPlayData->angle[2] & ANGCLIP;
			act = lpPlayData->angle[3];

			if ( act!=action ) {//|| ( act!=CHRMOTION_WALK && act!=CHRMOTION_STAND ) )
				if ( Pattern )
					ChangeMotion( act );
				else
					act = 0;
			}

			action = act;

			MoveCnt = 0;
			dwTarget = 0;
			//PlayBuffCnt = PlayBuffPosi_End;
		}
*/
		LastPlayDataType = 1;			//원거리 교신
		smCharInfo.bUpdateInfo[1] = 0xFF;
		break;

	case smTRANSCODE_TRANSPLAYDATA:
		//게임 진행 수신

		smTRANS_PLAYDATA	*lpTransPlayData;
		smTRANS_PLAYBUFF	*lpTransPlayBuff;
		smTRANS_PLAYBUFF	*lpTransPlayBuff2;

		LastPlayDataType = 0;			//근거리 교신

		lpTransPlayData = (smTRANS_PLAYDATA *)lpData;
		lpTransPlayBuff = (smTRANS_PLAYBUFF *)(lpData + sizeof(smTRANS_PLAYDATA) );

		LastBuffPosi_End = PlayBuffPosi_End;

		smCharInfo.Life[0] = lpTransPlayData->Hp[0];		//체력
		smCharInfo.Life[1] = lpTransPlayData->Hp[1];	//체력최대치

		if ( LevelLog ) {
			if ( smCharInfo.State==smCHAR_STATE_USER && lpTransPlayData->bEventInfo[TRANS_PLAYDATA_EVENT_LEVEL]>LevelLog ) {
				//상대방 레벨업
				StartEffect( pX,pY+32*fONE,pZ , EFFECT_LEVELUP1 );
				esPlaySound( 7, GetDistVolume( pX , pY , pZ ) );
			}
		}

		LevelLog = lpTransPlayData->bEventInfo[TRANS_PLAYDATA_EVENT_LEVEL];
		PotionLog = lpTransPlayData->bEventInfo[TRANS_PLAYDATA_EVENT_POTION]&0x0F;
		CriticalLog = lpTransPlayData->bEventInfo[TRANS_PLAYDATA_EVENT_POTION]>>4;
		dwDispAppSkill = lpTransPlayData->bEventInfo[TRANS_PLAYDATA_EVENT_SKIL];

/*
		if ( dwDispAppSkill&SKILL_APPMASK_ICE ) {
			//얼음 공격받음
			if ( AttackIce<=0 ) AttackIce = 1;
		}
		else {
			if ( AttackIce>0 ) AttackIce = -AttackIce;
		}
*/

		if (CriticalLog>=8) CriticalLog = -1;
		AttackCritcal = CriticalLog;

		if ( lpTransPlayData->PlayBuffCnt>1 ) {

			for(cnt=0;cnt<lpTransPlayData->PlayBuffCnt-1;cnt++) {
				lpTransPlayBuff2 = lpTransPlayBuff+1;

				len = lpTransPlayBuff2->BuffCount - lpTransPlayBuff->BuffCount;
				MakePlayBuffFromRecvData( &lpTransPlayBuff->PlayBuff , &lpTransPlayBuff2->PlayBuff , len );

				lpTransPlayBuff++;
			}

			dwTarget = lpTransPlayData->dwTarget;
/*
			if ( lpTransPlayData->bUpdateInfo[0]!=smCharInfo.bUpdateInfo[0]) {
				//내용 정보가 바뀜
				//Close();				//캐릭터 닫기 
				//smCharInfo.bUpdateInfo[1] = lpTransPlayData->bUpdateInfo[0];


				//smCharInfo.bUpdateInfo[0] = lpTransPlayData->bUpdateInfo[0];

				if ( !dwUpdateCharInfoTime || (dwUpdateCharInfoTime+1000)<dwPlayTime ) {
					Send_GetCharInfo( dwObjectSerial , ServerCode );
				
					if ( smCharInfo.dwCharSoundCode==snCHAR_SOUND_HUNGKY && !dwUpdateCharInfoTime) { 
						//이펙트 ( 헐크 변신 )
						StartEffect( pX,pY+48*fONE,pZ, EFFECT_RETURN1 );
						SetDynLight( pX,pY,pZ, 50, 100,70,0, 300 , 1 );

						//변신 소리
						cnt = MotionInfo->State;
						MotionInfo->State=CHRMOTION_STATE_WARP;
						CharPlaySound( this );
						MotionInfo->State = cnt;
					}
					dwUpdateCharInfoTime = dwPlayTime;
				}
			}
*/
			if ( smCharInfo.bUpdateInfo[1]==0xFF ) {
				//스킬포인터 오래되서 새로 갱신
				smCharInfo.bUpdateInfo[1] = lpTransPlayData->bUpdateInfo[1];
			}
			else {
				if ( lpTransPlayData->bUpdateInfo[1]!=smCharInfo.bUpdateInfo[1] ) {
					//스킬 포인트 변화
					if ( lpTransPlayData->bUpdateInfo[1]<smCharInfo.bUpdateInfo[1] )
						LearnSkillEffect( this );			//스킬 습득 이펙트

					smCharInfo.bUpdateInfo[1] = lpTransPlayData->bUpdateInfo[1];
				}
			}


			lpTransPlayBuff++;
		}
		else {		//서있는 동작 연속
			MakePlayBuffFromRecvData( &lpTransPlayBuff->PlayBuff , 0 , 0 );
			if ( PlayBuffCnt==PlayBuffPosi_End ) {

				if ( DisplayFlag ) {

					pX = lpTransPlayBuff->PlayBuff.x;
					pY = lpTransPlayBuff->PlayBuff.y;
					pZ = lpTransPlayBuff->PlayBuff.z;

					// 왼쪽 장착 아이템
					if ( lpTransPlayBuff->PlayBuff.wStickItems[smPLAYBUFF_ITEM_LEFT]!=wStickItems[smPLAYBUFF_ITEM_LEFT] ) {
						if ( lpTransPlayBuff->PlayBuff.wStickItems[smPLAYBUFF_ITEM_LEFT]>=0 ) {
							SetTool( sItem[lpTransPlayBuff->PlayBuff.wStickItems[smPLAYBUFF_ITEM_LEFT]].CODE , hvPOSI_LHAND );
							HvLeftHand.ColorBlink = 0;
							HvLeftHand.DispEffect = 0;
						}
						else
							SetTool(  0 , hvPOSI_LHAND );
					}

					// 오른쪽 장착 아이템
					if ( lpTransPlayBuff->PlayBuff.wStickItems[smPLAYBUFF_ITEM_RIGHT]!=wStickItems[smPLAYBUFF_ITEM_RIGHT] ) {
						if ( lpTransPlayBuff->PlayBuff.wStickItems[smPLAYBUFF_ITEM_RIGHT]>=0 ) {
							SetTool( sItem[lpTransPlayBuff->PlayBuff.wStickItems[smPLAYBUFF_ITEM_RIGHT]].CODE , hvPOSI_RHAND );
							HvRightHand.ColorBlink = 0;
							HvRightHand.DispEffect = 0;
						}
						else
							SetTool(  0 , hvPOSI_RHAND );
					}

					// 몸 장착 아이템( 갑옷 등 )
					if ( lpTransPlayBuff->PlayBuff.wStickItems[smPLAYBUFF_ITEM_BODY]!=wStickItems[smPLAYBUFF_ITEM_BODY] ) {
						SetChrArmor( this , lpTransPlayBuff->PlayBuff.wStickItems[smPLAYBUFF_ITEM_BODY] );
						wStickItems[smPLAYBUFF_ITEM_BODY] = lpTransPlayBuff->PlayBuff.wStickItems[smPLAYBUFF_ITEM_BODY];
					}
				}
			}

			lpTransPlayBuff++;
		}

		if ( lpTransPlayData->bUpdateInfo[0]!=smCharInfo.bUpdateInfo[0]) {

			if ( !dwUpdateCharInfoTime || (dwUpdateCharInfoTime+1000)<dwPlayTime ) {
				Send_GetCharInfo( dwObjectSerial , ServerCode );
				
				if ( smCharInfo.dwCharSoundCode==snCHAR_SOUND_HUNGKY && !dwUpdateCharInfoTime) { 
					//이펙트 ( 헐크 변신 )
					StartEffect( pX,pY+48*fONE,pZ, EFFECT_RETURN1 );
					SetDynLight( pX,pY,pZ, 50, 100,70,0, 300 , 1 );

					//변신 소리
					if ( MotionInfo ) {
						cnt = MotionInfo->State;
						MotionInfo->State=CHRMOTION_STATE_WARP;
						CharPlaySound( this );
						MotionInfo->State = cnt;
					}
				}
				// pluto 언데드 킹호피 변신
				if( smCharInfo.dwCharSoundCode == snCHAR_SOUND_DEADHOPT && !dwUpdateCharInfoTime )
				{
					StartEffect( pX,pY+48*fONE,pZ, EFFECT_RETURN1 );
					SetDynLight( pX,pY,pZ, 50, 100,70,0, 300 , 1 );

					if( MotionInfo )
					{
						cnt = MotionInfo->State;
						MotionInfo->State = CHRMOTION_STATE_WARP;
						CharPlaySound(this);
						MotionInfo->State = cnt;
					}
				}

				// 박재원 - 복날 이벤트
				if( smCharInfo.dwCharSoundCode == snCHAR_SOUND_BEBECHICK && !dwUpdateCharInfoTime )
				{
					StartEffect( pX,pY+48*fONE,pZ, EFFECT_RETURN1 );
					SetDynLight( pX,pY,pZ, 50, 100,70,0, 300 , 1 );

					if( MotionInfo )
					{
						cnt = MotionInfo->State;
						MotionInfo->State = CHRMOTION_STATE_WARP;
						CharPlaySound(this);
						MotionInfo->State = cnt;
					}
				}


				dwUpdateCharInfoTime = dwPlayTime;
			}
		}


		////////////////// 공성전 필드 확인 ////////////////////

		if ( lpCurPlayer->OnStageField>=0 && 
			(
			(StageField[ lpCurPlayer->OnStageField ]->State==FIELD_STATE_CASTLE && CastleBattleZone_LineZ>lpCurPlayer->pZ) ||
			(StageField[ lpCurPlayer->OnStageField ]->FieldCode==PK_FieldState.FieldCode && PK_FieldState.dwPlay_FieldTime>dwPlayTime && !PartyFlag ) 
			)	) {

			//주인공 공성 모드
			if ( PkMode_CharState==0 && smCharInfo.State==smCHAR_STATE_USER && 
				(smCharInfo.ClassClan==0 || smCharInfo.ClassClan!=lpCurPlayer->smCharInfo.ClassClan )) 
			{

				if ( StageField[ lpCurPlayer->OnStageField ]->FieldCode!=PK_FieldState.FieldCode || !PartyFlag ) {
					PkMode_CharState = smCharInfo.State;
					smCharInfo.State = smCHAR_STATE_ENEMY;
				}
			}
		}
		else {
			if ( PkMode_CharState ) {
				//공성모드 해제
				smCharInfo.State = PkMode_CharState;
				PkMode_CharState =0;
			}
		}

		int ChkIceFlag = 0;
		int ChkPoisonFlag = 0;
		DWORD	TradeMsgCode = 0;
		//DWORD	dwMsgCode;

		PlayVanish = 0;
		TradeMsgCode = dwTradeMsgCode;
		dwTradeMsgCode = 0;
		PlayCurseQuest = 0;

		if ( lpTransPlayData->bEventInfo[TRANS_PLAYDATA_EVENT_EXT] ) {
			lpBuff = (char *)lpTransPlayBuff;

			for( cnt=0;cnt<lpTransPlayData->bEventInfo[TRANS_PLAYDATA_EVENT_EXT];cnt++) {
			
				Code = ((int *)lpBuff)[1];
				Size = ((int *)lpBuff)[0];

				switch(Code) {
				case smTRANSCODE_EFFECT_ITEM:
					//착용 무기 이펙트 적용
					SetTransEffectItems( (smEFFECT_ITEM *)lpBuff );
					break;

				case smTRANSCODE_EFFECT_WEAPON:
					//수신된 데이타의 장착 무기에 속성 이펙트 적용 
					SetTransEffectWeapon( (smEFFECT_ITEM *)lpBuff );
					break;

				case smTRANSCODE_EFFECT_STUN:
					//캐릭터가 스턴 상태
					if ( smCharInfo.Life[0]>0 )
						AssaParticle_Stune( this , 4 );//(((smEFFECT_ITEM *)lpBuff)->dwItemCode>>4)+4 );
					break;

				case smTRANSCODE_EFFECT_CURSE:
					//써큐버스 저주 ( HolyPower 적용중 )
					if ( smCharInfo.Life[0]>0 )
						StartEffect_Serqbus_Skill2( this , 4 );
					break;

				case smTRANSCODE_EFFECT_ICE:
					//꽁꽁 얼었음 
					if ( ((smEFFECT_ITEM *)lpBuff)->ColorBlink ) {
						PlayDistortion = 120;//((smEFFECT_ITEM *)lpBuff)->ColorBlink*4;
					}
					else {
						PlayDistortion = 0;	//디스토션후 얼음공격일때 디스토션 효과 유지 버그 수정( 2005년 7월14일 ) 
						if ( AttackIce<=0 ) AttackIce = 1;
						ChkIceFlag = TRUE;
					}
					break;

				case smTRANSCODE_EFFECT_POISION:
					if (DispPoison<=0 ) DispPoison = 1;
					ChkPoisonFlag = TRUE;
					break;

				case smTRANSCODE_EFFECT_CURSE_QUEST:
					PlayCurseQuest = 1;
					break;

				case smTRANSCODE_EFFECT_VANISH:
					PlayVanish = ((smEFFECT_ITEM *)lpBuff)->dwItemCode;
					PlayVague = ((smEFFECT_ITEM *)lpBuff)->ColorBlink;
					break;

				case smTRANSCODE_EFFECT_KEEPSKILL:
					//스킬 이펙트 플랙 정보
					switch( ((smEFFECT_ITEM *)lpBuff)->dwItemCode ) {
						case SKILL_PLAY_FORCE_OF_NATURE:
							SkillSagittarionForceOfNature2( this , 5.0f );		//포스오브네이쳐 이펙트
							break;
						case SKILL_PLAY_HALL_OF_VALHALLA:	
							SkillValhallaHallOfValhallaHandEffect( this , 5.0f );	//홀 오브 발할라 이펙트
							break;
						case SKILL_PLAY_BLESS_SIEGE_ITEM:
							PlayInvincible = 70*5;					//무적 아이템
							break;
					}
					break;

				case smTRANSCODE_EFFECT_PERSHOP:		//개인상점 오픈
					dwTradeMsgCode = ((smEFFECT_ITEM *)lpBuff)->dwItemCode;

					if ( TradeMsgCode!=dwTradeMsgCode ) {
						/*
						if ( szTradeMessage[0] )
							dwMsgCode = GetSpeedSum( szTradeMessage );
						else
							dwMsgCode = 0;
						*/

						//코드 변경
						wsprintf( szTradeMessage , "%s :" , smCharInfo.szName ); 
						//지역서버에 명령 코드 전송
						SendCommand_AreaServer( smTRANSCODE_MYSHOP_MESSAGE , 0 , dwObjectSerial , dwTradeMsgCode , 0 );
					}
					break;

				case smTRANSCODE_EFFECT_LOWPET:		//저레벨 도움미 펫

					if ( ((smEFFECT_ITEM *)lpBuff)->ColorBlink ) 
						lpChar = FindChrPlayer( ((smEFFECT_ITEM *)lpBuff)->ColorBlink );
					else
						lpChar = 0;

					switch( ((smEFFECT_ITEM *)lpBuff)->dwItemCode ) {
						case 1:
							g_LowLevelAddIdlePet( this, PET_TERRY , lpChar );
							break;
						case 2:
							g_LowLevelAddIdlePet( this, PET_NEPSYS , lpChar );
							break;
						case 3:
							g_LowLevelAddIdlePet( this, PET_IO  , lpChar );
							break;
						case 4:
							g_LowLevelAddIdlePet( this, PET_MUTE , lpChar );
							break;

					}
					break;

				case smTRANSCODE_EFFECT_PCBANGPET:		//PC방 펫

					if ( ((smEFFECT_ITEM *)lpBuff)->ColorBlink ) 
						lpChar = FindChrPlayer( ((smEFFECT_ITEM *)lpBuff)->ColorBlink );
					else
						lpChar = 0;

					switch( ((smEFFECT_ITEM *)lpBuff)->dwItemCode ) {
						case 1:
							g_HoPCBangAddIdlePet( this, PCBANG_PET_BURSTP , lpChar );
						break;
						case 2:
							g_HoPCBangAddIdlePet( this, PCBANG_PET_BURSTS , lpChar );
						break;
						case 3:
							g_HoPCBangAddIdlePet( this, PCBANG_PET_BURSTL  , lpChar );
						break;
						case 4:
							g_HoPCBangAddIdlePet( this, PCBANG_PET_BURSTD , lpChar );
						break;

					}
					break;

				case smTRANSCODE_EFFECT_CLANMGR:		//순위권 클랜 표시
					dwClanManageBit = ((smEFFECT_ITEM *)lpBuff)->dwItemCode;
					break;

				case smTRANSCODE_EFFECT_FORCEORB:		//포스 오브 시전중
					dwForceOrbCode = ((smEFFECT_ITEM *)lpBuff)->dwItemCode;
					dwForceOrbTime = ((smEFFECT_ITEM *)lpBuff)->ColorBlink;
					dwForceOrbTime += dwPlayTime;

					AssaParticle_ShelltomWeapon( this ,  70*5, ((dwForceOrbCode&0xFFFF)>>8)-1 );
					break;

				case smTRANSCODE_EFFECT_BLESS_SCORE:
					//블래스 캐슬 득점 정보 입수
					sBlessCastle_Damage[0] = ((smEFFECT_ITEM *)lpBuff)->sColors[0];
					sBlessCastle_Damage[1] = ((smEFFECT_ITEM *)lpBuff)->sColors[1];
					break;
				}
				lpBuff += Size;
			}
		}
		if ( !ChkIceFlag && AttackIce>0 ) AttackIce = -AttackIce;
		if ( !ChkPoisonFlag && DispPoison>0 ) DispPoison = -DispPoison;

	}


	TransSendWait = 0;
	dwLastTransTime = dwPlayTime;

	return TRUE;
}

//데이타를 받음
int smCHAR::RecvPlayData( smTHREADSOCK *pData )
{
	int act;
	smPLAYDATA PlayData;
	smPLAYDATA2 PlayData2;
	smPLAYDATA3 PlayData3;
	smTRNAS_PLAYERINFO	*lpTransPlayerInfo;
	int RecvCode;
	int cnt;
	int len;
	int LastBuffPosi_End;
	//smCHAR	*lpChar;

	RecvCode = ((int *)pData->Buff)[1];

	switch( RecvCode ) {
	case smTRANSCODE_PLAYERINFO:
		//상대 플레이어의 데이타 수신 받음
		lpTransPlayerInfo = ( smTRNAS_PLAYERINFO * )pData->Buff;
		SetTransPlayerInfo( lpTransPlayerInfo );
		return TRUE;

	case smTRANSCODE_PLAYERINFO_Q:
		//상대 플레이어의 데이타 수신 받음 ( 패킷양 적은 데이타 )
		SetTransPlayerInfoQuick( (smTRNAS_PLAYERINFO_QUICK *)pData->Buff );
		break;

	case smTRANSCODE_PLAYDATA1:
		//게임 진행 수신
		memcpy( &PlayData , pData->Buff , sizeof( smPLAYDATA ) );

		pX = PlayData.x;
		pY = PlayData.y;
		pZ = PlayData.z;

		Angle.x = PlayData.angle[0] & ANGCLIP;
		Angle.y = PlayData.angle[1] & ANGCLIP;
		Angle.z = PlayData.angle[2] & ANGCLIP;
		act = PlayData.angle[3];

		if ( act!=action )//|| ( act!=CHRMOTION_WALK && act!=CHRMOTION_STAND ) )
			frame = PlayData.frame;

		action = act;

		PlayBuffCnt = PlayBuffPosi_End;
		MoveCnt = 0;

		dwTarget = 0;
		break;

	case smTRANSCODE_PLAYDATA2:
		//게임 진행 수신
		LastBuffPosi_End = PlayBuffPosi_End;
		memcpy( &PlayData2 , pData->Buff , sizeof( smPLAYDATA2 ) );
		len = PlayData2.BuffCount[1] - PlayData2.BuffCount[0];
		MakePlayBuffFromRecvData( &PlayData2.PlayBuff[0] , &PlayData2.PlayBuff[1] , len );
		if ( (PlayBuffPosi_End-PlayBuffCnt)>32 ) PlayBuffCnt = LastBuffPosi_End;

		dwTarget = PlayData2.dwTarget;
		break;

	case smTRANSCODE_PLAYDATA3:
		//게임 진행 수신
		LastBuffPosi_End = PlayBuffPosi_End;
		memcpy( &PlayData3 , pData->Buff , sizeof( smPLAYDATA3 ) );
		len = PlayData3.BuffCount[2] - PlayData3.BuffCount[0];
		MakePlayBuffFromRecvData( &PlayData3.PlayBuff[0] , &PlayData3.PlayBuff[2] , len );
		len = PlayData3.BuffCount[1] - PlayData3.BuffCount[2];
		MakePlayBuffFromRecvData( &PlayData3.PlayBuff[2] , &PlayData3.PlayBuff[1] , len );
		if ( (PlayBuffPosi_End-PlayBuffCnt)>32 ) PlayBuffCnt = LastBuffPosi_End;

		dwTarget = PlayData3.dwTarget;
		break;

	case smTRANSCODE_TRANSPLAYDATA:
		//게임 진행 수신

		smTRANS_PLAYDATA	*lpTransPlayData;
		smTRANS_PLAYBUFF	*lpTransPlayBuff;
		smTRANS_PLAYBUFF	*lpTransPlayBuff2;

		lpTransPlayData = (smTRANS_PLAYDATA *)pData->Buff;
		lpTransPlayBuff = (smTRANS_PLAYBUFF *)(pData->Buff + sizeof(smTRANS_PLAYDATA) );

		LastBuffPosi_End = PlayBuffPosi_End;

		smCharInfo.Life[0] = lpTransPlayData->Hp[0];		//체력

		for(cnt=0;cnt<lpTransPlayData->PlayBuffCnt-1;cnt++) {
			lpTransPlayBuff2 = lpTransPlayBuff+1;

			len = lpTransPlayBuff2->BuffCount - lpTransPlayBuff->BuffCount;
			MakePlayBuffFromRecvData( &lpTransPlayBuff->PlayBuff , &lpTransPlayBuff2->PlayBuff , len );

			lpTransPlayBuff++;
		}

		dwTarget = lpTransPlayData->dwTarget;
		break;

	case smTRANSCODE_ATTACKDATA2:
	case smTRANSCODE_ATTACKDATA:
		//상대방한테 공격 함
		AttackRecvCountUp();

		Record_TotalRecvDamageCount++;

		if ( ((TRANS_ATTACKDATA *)pData->Buff)->AttackState==0x80 ) {
			memcpy( &AttackTrans , (TRANS_ATTACKDATA *)pData->Buff , sizeof(TRANS_ATTACKDATA) );
			XorAttackTrans();
			if ( PkMode_CharState || ( lpCurPlayer->OnStageField>=0 && StageField[ lpCurPlayer->OnStageField ]->FieldCode==rsCASTLE_FIELD ) )
				PlayAttack(TRUE);								//밀린 공격 적용
			else
				PlayAttack();									//밀린 공격 적용
			break;
		}

		if ( AttackTrans.code^GetAttackTrans_XorCode() ) {
			PlayAttack( TRUE );									//밀린 공격 적용
		}

		memcpy( &AttackTrans , (TRANS_ATTACKDATA *)pData->Buff , sizeof(TRANS_ATTACKDATA) );
		XorAttackTrans();

/*
#ifdef	_S2C_DAMAGE_PACKET
#ifdef _LANGUAGE_CHINESE
		PlayAttack( TRUE );									//밀린 공격 적용
#endif
#endif
*/
		/*
		TRANS_ATTACKDATA	*lpTransAttackData;

		lpTransAttackData = (TRANS_ATTACKDATA *)pData->Buff;

		if ( lpCurPlayer->MotionInfo->State != CHRMOTION_STATE_DAMAGE && lpCurPlayer->MotionInfo->State != CHRMOTION_STATE_DEAD && DisplayFlag ) {

			len = GetDistanceDbl( lpCurPlayer->pX , lpCurPlayer->pZ , pX, pZ );
			cnt = GetDistanceDbl( lpCurPlayer->pX , lpCurPlayer->pZ , lpTransAttackData->x, lpTransAttackData->z );

			if ( smCharInfo.dwCharSoundCode==snCHAR_SOUND_FIGON || smCharInfo.dwCharSoundCode==snCHAR_SOUND_WEB ) {
				//원거리 공격
				len = 0;
				cnt = 0;
			}

			//주인공 몬스터 스킨일 경우
			if ( lpCurPlayer->smCharInfo.State==smCHAR_STATE_ENEMY ) act = 1;
			else act = 0;

			if ( cnt<lpTransAttackData->AttackSize && lpCurPlayer->smCharInfo.Life[0]>0 && ( len<(128*128) || act ) && DisplayAlpha==0 ) {

				lpChar = 0;

				//공격한 상대방을 찾아 방향을 바꾼다
				for( cnt=0;cnt<OTHER_PLAYER_MAX;cnt++ ) {
					if ( chrOtherPlayer[cnt].Flag && chrOtherPlayer[cnt].dwObjectSerial==lpTransAttackData->dwDestObjectSerial ) {
						lpChar = &chrOtherPlayer[cnt];
						break;
					}
				}

				//블럭율 계산
				if ( lpChar && ( (rand()%100)>=sinGetBlockRating( lpChar->smCharInfo.Level ) || act ) ) { 

					//방어율 계산
					//cnt = GetPlayDefence( &smCharInfo , &lpCurPlayer->smCharInfo );
					cnt = sinGetMonsterAccuracy( smCharInfo.Level , smCharInfo.Attack_Rating );
					//cnt = 100;
					if ( (rand()%100)<=cnt || act ) {	

						//흡수율 계산
						cnt = (lpTransAttackData->Power - lpCurPlayer->smCharInfo.Absorption);

						if ( cnt<=0 ) cnt = 1;

							CheckCharForm();
							//명중 ( 공격 성공 )
							lpCurPlayer->smCharInfo.Life[0] -= cnt;

							//피튀기는 파티클
							StartEffect( lpCurPlayer->pX, lpCurPlayer->pY+16*fONE, lpCurPlayer->pZ, EFFECT_DAMAGE1 );
							SetDynLight( lpCurPlayer->pX, lpCurPlayer->pY+16*fONE, lpCurPlayer->pZ, 160, 0,0,0, 180 );

							if ( (lpCurPlayer->dwDispAppSkill&SKILL_APPMASK_PHYSICAL_ABSORB) ) {
								//피지클업솝 적용이펙트
								StartEffect( lpCurPlayer->pX, lpCurPlayer->pY+24*fONE , lpCurPlayer->pZ , EFFECT_PHYSICAL_ABSORB_DAMAGE );
							}

							if ( lpCurPlayer->MotionInfo->State!=CHRMOTION_STATE_EAT && lpCurPlayer->MotionInfo->State!=CHRMOTION_STATE_ATTACK &&
								lpCurPlayer->MotionInfo->State!=CHRMOTION_STATE_SKILL && cnt>1 ) {
								lpCurPlayer->SetMotionFromCode( CHRMOTION_STATE_DAMAGE );
								CharPlaySound( lpCurPlayer );
							}

							//임시 처리
							if ( lpCurPlayer->smCharInfo.Life[0]<=0 ) {
								lpCurPlayer->SetMotionFromCode( CHRMOTION_STATE_DEAD );
								lpCurPlayer->smCharInfo.Life[0] = 0;
								ReformCharForm();					//캐릭터 정보 인증
								CharPlaySound( lpCurPlayer );
								DeadPlayerExp( lpCurPlayer );
								//게임을 저장
								SaveGameData();
								break;
							}
							ReformCharForm();					//캐릭터 정보 인증

					}
				}
				dwBattleTime = dwPlayTime;			//전투 시간 기록
				//공격한 상대방을 찾아 방향을 바꾼다
				//lpCurPlayer->Angle.y = GetRadian2D( lpCurPlayer->pX , lpCurPlayer->pZ ,lpChar->pX, lpChar->pZ );
			}
		}
		*/
		break;
	}

	TransSendWait = 0;
	dwLastTransTime = dwPlayTime;

	//게임 데이타 전송이 처음 되었을때 캐릭터 화면에 표시
	//if ( Pattern ) DisplayFlag = TRUE;		

	return TRUE;
}

//데이타를 보냄
int smCHAR::SendPlayData( smCHAR *player )
{
	int SendTime;
	smPLAYDATA PlayData;
	smPLAYDATA2 PlayData2;
	smPLAYDATA3 PlayData3;
	int PlayerBuffCnt;
	int cnt;
	int StartPosi;
	int BuffPosi;
	int mAction;
	int SendMode;

	//if ( AutoPlayer ) return NULL;

	SendTime = PlayCounter-TransLastSendCnt;

	if ( TransSendWait==0 && SendTime>=TransMinCnt ) {

		//목표를 겨냥한 동작일 경우~~ 목표점 표시한다
		if ( lpCharMsTrace && lpCharMsTrace==this ) {
			PlayData2.dwTarget = TRUE;
			PlayData3.dwTarget = TRUE;
		}
		else {
			PlayData2.dwTarget = FALSE;
			PlayData3.dwTarget = FALSE;
		}

		if ( TransLevel>1 ) {
			//버퍼링 없는 데이타 전송 ( 통신의 중요도가 낮음 )
			PlayData.size = sizeof( smPLAYDATA );
			PlayData.code = smTRANSCODE_PLAYDATA1;
			PlayData.x = player->pX;
			PlayData.y = player->pY;
			PlayData.z = player->pZ;

			PlayData.angle[0] = player->Angle.x;
			PlayData.angle[1] = player->Angle.y;
			PlayData.angle[2] = player->Angle.z;
			PlayData.angle[3] = player->action;

			PlayData.frame = player->frame;

			if ( TransSock->Send( (char *)&PlayData , sizeof( smPLAYDATA ) )  ) {
				TransSendWait = 1;
				TransLastSendCnt = PlayCounter;
				TransLastSendTime = SendTime;
				SetDistSendCnt( player );				//통신 시간차 조절
			}
		}
		else {
			//버퍼링 데이타 전송 ( 고품질의 데이타 전송 )
			SendMode = 0;
			PlayerBuffCnt = player->PlayBuffCnt&PLAYBUFF_MASK;
			StartPosi = ( PlayerBuffCnt - SendTime) & PLAYBUFF_MASK;

			mAction = player->PlayBuff[ PlayerBuffCnt ].angle[3];

			for( cnt=SendTime; cnt>=0; cnt-- ) {
				BuffPosi = (StartPosi + cnt ) & PLAYBUFF_MASK;
				if ( player->PlayBuff[BuffPosi].angle[3]!=mAction ) {
					SendMode = 1;
					memcpy( &PlayData3.PlayBuff[2] , &player->PlayBuff[BuffPosi] , sizeof( smPLAYBUFF ) );
					PlayData3.BuffCount[2] = cnt;
					break;
				}
			}

			if ( !SendMode ) {
				memcpy( &PlayData2.PlayBuff[0] , &player->PlayBuff[ StartPosi ] , sizeof( smPLAYBUFF ) );
				memcpy( &PlayData2.PlayBuff[1] , &player->PlayBuff[ PlayerBuffCnt ] , sizeof( smPLAYBUFF ) );
				PlayData2.code = smTRANSCODE_PLAYDATA2;
				PlayData2.size = sizeof( smPLAYDATA2 );
				PlayData2.BuffCount[0] = 0;
				PlayData2.BuffCount[1] = SendTime;

				if ( TransSock->Send( (char *)&PlayData2 , sizeof( smPLAYDATA2 ) ) ) {
					TransSendWait = 1;
					TransLastSendCnt = PlayCounter;
					TransLastSendTime = SendTime;
					SetDistSendCnt( player );				//통신 시간차 조절
				}
			}
			else {
				memcpy( &PlayData3.PlayBuff[0] , &player->PlayBuff[ StartPosi ] , sizeof( smPLAYBUFF ) );
				memcpy( &PlayData3.PlayBuff[1] , &player->PlayBuff[ PlayerBuffCnt ] , sizeof( smPLAYBUFF ) );
				PlayData3.code = smTRANSCODE_PLAYDATA3;
				PlayData3.size = sizeof( smPLAYDATA3 );
				PlayData3.BuffCount[0] = 0;
				PlayData3.BuffCount[1] = SendTime;

				if ( TransSock->Send( (char *)&PlayData3 , sizeof( smPLAYDATA3 ) ) ) {
					TransSendWait = 1;
					TransLastSendCnt = PlayCounter;
					TransLastSendTime = SendTime;
					SetDistSendCnt( player );				//통신 시간차 조절
				}
			}
		}
	}

	return TRUE;
}
//통신용 플레이 데이타를 지정한 버퍼에 작성 
int smCHAR::MakeTransPlayData( char *lpTargetBuff , int SendTime , int pBuffStep )
{
	smTRANS_PLAYDATA	*lpTransPlayData;
	smTRANS_PLAYBUFF	*lpTransPlayBuff;
	smEFFECT_ITEM		*lpEffectItem;

	int StartPosi;
	int BuffPosi;
	int BuffPosi_Old;
	int mAction;
	int mFrameSelect;
	DWORD mFrame;
	int	Life;

//	int PlayerBuffCnt;

	int	buffCnt;
	int cnt;
	int LastCnt;
	int	SameAction;

	lpTransPlayData = (smTRANS_PLAYDATA *)lpTargetBuff;
	lpTransPlayBuff = (smTRANS_PLAYBUFF *)(lpTargetBuff + sizeof( smTRANS_PLAYDATA ));

	lpTransPlayData->code = smTRANSCODE_TRANSPLAYDATA;
	lpTransPlayData->dwObjectSerial = dwObjectSerial;
	lpTransPlayData->dwTarget = 0;
	lpTransPlayData->StartPosi = PlayBuffCnt - SendTime;
	if ( this==lpCurPlayer ) {
		if ( MotionInfo && MotionInfo->State==CHRMOTION_STATE_DEAD )
			lpTransPlayData->Hp[0] = 0;
		else
			lpTransPlayData->Hp[0] = sinGetLife();
	}
	else {
		lpTransPlayData->Hp[0] = smCharInfo.Life[0];
	}
	lpTransPlayData->Hp[1] = smCharInfo.Life[1];

	if ( UseObject_VirtualLife ) {		//가상 생명력 사용 (확장)
		lpTransPlayData->Hp[0] = sObject_DisplayLife[0];
		lpTransPlayData->Hp[1] = sObject_DisplayLife[1];
	}



	if ( smCharInfo.State==smCHAR_STATE_ENEMY )
		lpTransPlayData->dwAutoCharCode = smMonsterInfo.dwAutoCharCode;
	else
		lpTransPlayData->dwAutoCharCode = 0;

	smCharInfo.bUpdateInfo[1] = sinSkill.SkillPoint;				//스킬 포인트 저장

	lpTransPlayData->bUpdateInfo[0] = smCharInfo.bUpdateInfo[0];
	lpTransPlayData->bUpdateInfo[1] = smCharInfo.bUpdateInfo[1];
	lpTransPlayData->bUpdateInfo[2] = 0;
	lpTransPlayData->bUpdateInfo[3] = 0;


	int	MainArea , SubArea;

	if ( lpCurPlayer && lpCurPlayer->OnStageField>=0 ) {
		MainArea = (lpCurPlayer->OnStageField)&1;
		SubArea = (lpCurPlayer->OnStageField+1)&1;
	}
	else {
		MainArea = 0;
		SubArea = 0;
	}

	//로딩된 해당 지역 코드 삽입
	if ( StageField[0] )
		lpTransPlayData->wArea[0] = StageField[MainArea]->FieldCode;
	else
		lpTransPlayData->wArea[0] = -1;

	if ( StageField[1] )
		lpTransPlayData->wArea[1] = StageField[SubArea]->FieldCode;
	else
		lpTransPlayData->wArea[1] = -1;

	lpTransPlayData->bEventInfo[TRANS_PLAYDATA_EVENT_LEVEL] = smCharInfo.Level;
	lpTransPlayData->bEventInfo[TRANS_PLAYDATA_EVENT_POTION] = (PotionLog&0x0F)|((AttackCritcal&0xF)<<4);
	lpTransPlayData->bEventInfo[TRANS_PLAYDATA_EVENT_EXT] = 0;
	lpTransPlayData->bEventInfo[TRANS_PLAYDATA_EVENT_SKIL] = (BYTE)dwDispAppSkill;

/*
	if ( smCharInfo.State!=smCHAR_STATE_USER ) {
		//몬스터
		if ( PlaySlowCount>0 ) lpTransPlayData->bEventInfo[TRANS_PLAYDATA_EVENT_SKIL] |= SKILL_APPMASK_ICE;
	}
*/
	//lpCurPlayer->dwDispAppSkill = 0;

//	BYTE	bEventInfo[4];		//모션 이벤트 정보 ( 0-Level 1-Potion사용 2-스킬사용 3-예비 )


	StartPosi = ( lpTransPlayData->StartPosi ) & PLAYBUFF_MASK;

	buffCnt = 0;

	//시작 정보
	mAction = PlayBuff[ StartPosi ].angle[3];
	mFrame = PlayBuff[ StartPosi ].frame;
	memcpy( &lpTransPlayBuff->PlayBuff , &PlayBuff[ StartPosi ] , sizeof( smPLAYBUFF ) );
	lpTransPlayBuff->BuffCount = 0;
	lpTransPlayBuff++;
	buffCnt++;
	LastCnt = 0;
	BuffPosi_Old = StartPosi;
	mFrameSelect = PlayBuff[ StartPosi ].frame>>CHAR_FRAME_SELSHIFT;

	SameAction = TRUE;

	for( cnt=1; cnt<SendTime-1;cnt++ ) {
		BuffPosi = (StartPosi + cnt ) & PLAYBUFF_MASK;

		if ( PlayBuff[BuffPosi].angle[3]!=mAction ) SameAction=FALSE;

		if ( PlayBuff[BuffPosi].frame<mFrame || PlayBuff[BuffPosi].angle[3]!=mAction || mFrameSelect!=(int)(PlayBuff[ BuffPosi ].frame>>CHAR_FRAME_SELSHIFT) ) {
			//if ( (cnt-LastCnt)>4 ) {
				//바뀐 모션 이벤트 기록
				mAction = PlayBuff[ BuffPosi_Old ].angle[3];
				memcpy( &lpTransPlayBuff->PlayBuff , &PlayBuff[ BuffPosi_Old ] , sizeof( smPLAYBUFF ) );
				lpTransPlayBuff->BuffCount = (cnt*pBuffStep)-1;
				lpTransPlayBuff++;
				buffCnt++;
			//}

			//바뀐 모션 이벤트 기록
			mAction = PlayBuff[ BuffPosi ].angle[3];
			mFrame = PlayBuff[ BuffPosi ].frame;
			memcpy( &lpTransPlayBuff->PlayBuff , &PlayBuff[ BuffPosi ] , sizeof( smPLAYBUFF ) );
			lpTransPlayBuff->BuffCount = cnt*pBuffStep;
			lpTransPlayBuff++;
			buffCnt++;
			LastCnt = cnt;
			mFrameSelect = PlayBuff[ BuffPosi ].frame>>CHAR_FRAME_SELSHIFT;
		}

		BuffPosi_Old = BuffPosi;
	}

	BuffPosi = (StartPosi + SendTime-1 ) & PLAYBUFF_MASK;

	//마지막 기록
	mAction = PlayBuff[ BuffPosi ].angle[3];
	memcpy( &lpTransPlayBuff->PlayBuff , &PlayBuff[ BuffPosi ] , sizeof( smPLAYBUFF ) );
	lpTransPlayBuff->BuffCount = (SendTime*pBuffStep)-1;
	lpTransPlayBuff++;
	buffCnt++;

	lpTransPlayData->StartPosi = (lpTransPlayData->StartPosi*pBuffStep)&PLAYBUFF_MASK;

	if ( SameAction==TRUE && smCharInfo.State==smCHAR_STATE_USER && MotionInfo->State==CHRMOTION_STATE_STAND ) 
		buffCnt = 1;			//가만히 서있는 동작의 연속

	lpTransPlayData->PlayBuffCnt = buffCnt;
	lpTransPlayData->size = sizeof(smTRANS_PLAYDATA) + ( sizeof(smTRANS_PLAYBUFF)*buffCnt );

	/////////////////// 추가 정보 뒤에 기록 /////////////////////


	MotionRecordCount ++;


//	if ( (MotionRecordCount&0x7)==0 ) {

		//추가 이펙트 정보가 있으면 주기적으로 전송
		lpEffectItem = (smEFFECT_ITEM *)(lpTargetBuff+lpTransPlayData->size);

		if ( HvLeftHand.PatTool && HvLeftHand.dwItemCode && (HvLeftHand.ColorBlink || HvLeftHand.DispEffect) ) {

			lpEffectItem->code =smTRANSCODE_EFFECT_ITEM;
			lpEffectItem->size = sizeof(smEFFECT_ITEM);
			lpEffectItem->dwItemCode = HvLeftHand.dwItemCode;
			lpEffectItem->ColorBlink = HvLeftHand.ColorBlink;
			memcpy( lpEffectItem->sColors , HvLeftHand.sColors , sizeof( short ) *4 );
			lpEffectItem->DispEffect = HvLeftHand.DispEffect;
			lpEffectItem->BlinkScale = HvLeftHand.BlinkScale;


			lpTransPlayData->size += lpEffectItem->size;
			lpTransPlayData->bEventInfo[TRANS_PLAYDATA_EVENT_EXT] ++;
			lpEffectItem++;
		}

		if ( HvRightHand.PatTool && HvRightHand.dwItemCode && (HvRightHand.ColorBlink || HvRightHand.DispEffect) ) {

			lpEffectItem->code =smTRANSCODE_EFFECT_ITEM;
			lpEffectItem->size = sizeof(smEFFECT_ITEM);
			lpEffectItem->dwItemCode = HvRightHand.dwItemCode;
			lpEffectItem->ColorBlink = HvRightHand.ColorBlink;
			memcpy( lpEffectItem->sColors , HvRightHand.sColors , sizeof( short ) *4 );
			lpEffectItem->DispEffect = HvRightHand.DispEffect;
			lpEffectItem->BlinkScale = HvRightHand.BlinkScale;

			lpTransPlayData->size += lpEffectItem->size;
			lpTransPlayData->bEventInfo[TRANS_PLAYDATA_EVENT_EXT] ++;
			lpEffectItem++;
		}

		if ( dwWeaponEffectTime ) {
			//속성 이펙트
			lpEffectItem->code =smTRANSCODE_EFFECT_WEAPON;
			lpEffectItem->size = sizeof(smEFFECT_ITEM);
			lpEffectItem->dwItemCode = WeaponEffect;
			lpEffectItem->ColorBlink = (dwWeaponEffectTime-dwPlayTime)+500;
			ZeroMemory( lpEffectItem->sColors , sizeof( short ) *4 );
			lpEffectItem->DispEffect = EnchantEffect_Point;
			lpEffectItem->BlinkScale = 0;

			lpTransPlayData->size += lpEffectItem->size;
			lpTransPlayData->bEventInfo[TRANS_PLAYDATA_EVENT_EXT] ++;
			lpEffectItem++;
		}

		if ( this==lpCurPlayer )
			Life = sinGetLife();
		else
			Life = smCharInfo.Life[0];

		if ( PlayStunCount && Life>0 ) {
			lpEffectItem->code =smTRANSCODE_EFFECT_STUN;
			lpEffectItem->size = sizeof(smEFFECT_ITEM);
			lpEffectItem->dwItemCode = PlayStunCount;
			lpEffectItem->ColorBlink = 0;
			ZeroMemory( lpEffectItem->sColors , sizeof( short ) *4 );
			lpEffectItem->DispEffect = 0;
			lpEffectItem->BlinkScale = 0;

			lpTransPlayData->size += lpEffectItem->size;
			lpTransPlayData->bEventInfo[TRANS_PLAYDATA_EVENT_EXT] ++;
			lpEffectItem++;
		}

		if ( PlayHolyPower[1] && Life>0 ) {
			lpEffectItem->code =smTRANSCODE_EFFECT_CURSE;
			lpEffectItem->size = sizeof(smEFFECT_ITEM);
			lpEffectItem->dwItemCode =  PlayHolyPower[1];
			lpEffectItem->ColorBlink = 0;
			ZeroMemory( lpEffectItem->sColors , sizeof( short ) *4 );
			lpEffectItem->DispEffect = 0;
			lpEffectItem->BlinkScale = 0;

			lpTransPlayData->size += lpEffectItem->size;
			lpTransPlayData->bEventInfo[TRANS_PLAYDATA_EVENT_EXT] ++;
			lpEffectItem++;
		}
/*
	if ( smCharInfo.State!=smCHAR_STATE_USER ) {
		//몬스터
		if ( PlaySlowCount>0 ) lpTransPlayData->bEventInfo[TRANS_PLAYDATA_EVENT_SKIL] |= SKILL_APPMASK_ICE;
	}
*/
		if ( PlaySlowCount>0 && Life>0 ) {
			if ( smCharInfo.State!=smCHAR_STATE_USER || AttackIce || PlayDistortion ) {
				lpEffectItem->code =smTRANSCODE_EFFECT_ICE;
				lpEffectItem->size = sizeof(smEFFECT_ITEM);
				lpEffectItem->dwItemCode =  PlaySlowCount;
				lpEffectItem->ColorBlink = PlayDistortion;
				ZeroMemory( lpEffectItem->sColors , sizeof( short ) *4 );
				lpEffectItem->DispEffect = 0;
				lpEffectItem->BlinkScale = 0;

				lpTransPlayData->size += lpEffectItem->size;
				lpTransPlayData->bEventInfo[TRANS_PLAYDATA_EVENT_EXT] ++;
				lpEffectItem++;
			}
		}

		if ( PlayPoison[1] && Life>0 ) {
			lpEffectItem->code =smTRANSCODE_EFFECT_POISION;
			lpEffectItem->size = sizeof(smEFFECT_ITEM);
			lpEffectItem->dwItemCode =  PlayPoison[0];
			lpEffectItem->ColorBlink = 0;
			ZeroMemory( lpEffectItem->sColors , sizeof( short ) *4 );
			lpEffectItem->DispEffect = 0;
			lpEffectItem->BlinkScale = 0;

			lpTransPlayData->size += lpEffectItem->size;
			lpTransPlayData->bEventInfo[TRANS_PLAYDATA_EVENT_EXT] ++;
			lpEffectItem++;
		}

		if ( PlayCurseQuest && Life>0 ) {
			lpEffectItem->code =smTRANSCODE_EFFECT_CURSE_QUEST;
			lpEffectItem->size = sizeof(smEFFECT_ITEM);
			lpEffectItem->dwItemCode =  PlayCurseQuest;
			lpEffectItem->ColorBlink = 0;
			ZeroMemory( lpEffectItem->sColors , sizeof( short ) *4 );
			lpEffectItem->DispEffect = 0;
			lpEffectItem->BlinkScale = 0;

			lpTransPlayData->size += lpEffectItem->size;
			lpTransPlayData->bEventInfo[TRANS_PLAYDATA_EVENT_EXT] ++;
			lpEffectItem++;
		}

		if ( PlayVanish>0 && Life>0 ) {
			lpEffectItem->code =smTRANSCODE_EFFECT_VANISH;
			lpEffectItem->size = sizeof(smEFFECT_ITEM);
			lpEffectItem->dwItemCode = PlayVanish;
			lpEffectItem->ColorBlink = PlayVague;
			ZeroMemory( lpEffectItem->sColors , sizeof( short ) *4 );
			lpEffectItem->DispEffect = 0;
			lpEffectItem->BlinkScale = 0;

			lpTransPlayData->size += lpEffectItem->size;
			lpTransPlayData->bEventInfo[TRANS_PLAYDATA_EVENT_EXT] ++;
			lpEffectItem++;
		}

		if ( dwForceOfNatureTime>dwPlayTime ) {
			lpEffectItem->code =smTRANSCODE_EFFECT_KEEPSKILL;
			lpEffectItem->size = sizeof(smEFFECT_ITEM);
			lpEffectItem->dwItemCode = SKILL_PLAY_FORCE_OF_NATURE;
			lpEffectItem->ColorBlink = dwForceOfNatureTime-dwPlayTime;
			ZeroMemory( lpEffectItem->sColors , sizeof( short ) *4 );
			lpEffectItem->DispEffect = 0;
			lpEffectItem->BlinkScale = 0;

			lpTransPlayData->size += lpEffectItem->size;
			lpTransPlayData->bEventInfo[TRANS_PLAYDATA_EVENT_EXT] ++;
			lpEffectItem++;
		}

		if ( dwHallOfValhallaTime>dwPlayTime ) {
			lpEffectItem->code =smTRANSCODE_EFFECT_KEEPSKILL;
			lpEffectItem->size = sizeof(smEFFECT_ITEM);
			lpEffectItem->dwItemCode = SKILL_PLAY_HALL_OF_VALHALLA;
			lpEffectItem->ColorBlink = dwHallOfValhallaTime-dwPlayTime;
			ZeroMemory( lpEffectItem->sColors , sizeof( short ) *4 );
			lpEffectItem->DispEffect = 0;
			lpEffectItem->BlinkScale = 0;

			lpTransPlayData->size += lpEffectItem->size;
			lpTransPlayData->bEventInfo[TRANS_PLAYDATA_EVENT_EXT] ++;
			lpEffectItem++;
		}

		if ( PlayInvincible>0 ) {
			lpEffectItem->code =smTRANSCODE_EFFECT_KEEPSKILL;
			lpEffectItem->size = sizeof(smEFFECT_ITEM);
			lpEffectItem->dwItemCode = SKILL_PLAY_BLESS_SIEGE_ITEM;
			lpEffectItem->ColorBlink = PlayInvincible;
			ZeroMemory( lpEffectItem->sColors , sizeof( short ) *4 );
			lpEffectItem->DispEffect = 0;
			lpEffectItem->BlinkScale = 0;

			lpTransPlayData->size += lpEffectItem->size;
			lpTransPlayData->bEventInfo[TRANS_PLAYDATA_EVENT_EXT] ++;
			lpEffectItem++;
		}


		if ( dwTradeMsgCode ) {
			lpEffectItem->code =smTRANSCODE_EFFECT_PERSHOP;
			lpEffectItem->size = sizeof(smEFFECT_ITEM);
			lpEffectItem->dwItemCode = dwTradeMsgCode;
			lpEffectItem->ColorBlink = 0;
			ZeroMemory( lpEffectItem->sColors , sizeof( short ) *4 );
			lpEffectItem->DispEffect = 0;
			lpEffectItem->BlinkScale = 0;

			lpTransPlayData->size += lpEffectItem->size;
			lpTransPlayData->bEventInfo[TRANS_PLAYDATA_EVENT_EXT] ++;
			lpEffectItem++;
		}

		if ( dwForceOrbTime && dwForceOrbTime>dwPlayTime ) {
			lpEffectItem->code =smTRANSCODE_EFFECT_FORCEORB;
			lpEffectItem->size = sizeof(smEFFECT_ITEM);
			lpEffectItem->dwItemCode = dwForceOrbCode;
			lpEffectItem->ColorBlink = dwPlayTime-dwForceOrbTime;
			ZeroMemory( lpEffectItem->sColors , sizeof( short ) *4 );
			lpEffectItem->DispEffect = 0;
			lpEffectItem->BlinkScale = 0;

			lpTransPlayData->size += lpEffectItem->size;
			lpTransPlayData->bEventInfo[TRANS_PLAYDATA_EVENT_EXT] ++;
			lpEffectItem++;
		}

		if ( cHelpPet.PetShow ) {// && smCharInfo.Level<=10 ) { // 박재원 - 빌링 도우미 펫 추가(타유저가 펫을 사용했을때 보이게 함)
			lpEffectItem->code =smTRANSCODE_EFFECT_LOWPET;
			lpEffectItem->size = sizeof(smEFFECT_ITEM);
			lpEffectItem->dwItemCode = cHelpPet.PetKind;
			lpEffectItem->ColorBlink = g_LowLevelPetAttackSerial;
			ZeroMemory( lpEffectItem->sColors , sizeof( short ) *4 );
			lpEffectItem->DispEffect = 0;
			lpEffectItem->BlinkScale = 0;

			lpTransPlayData->size += lpEffectItem->size;
			lpTransPlayData->bEventInfo[TRANS_PLAYDATA_EVENT_EXT] ++;
			lpEffectItem++;

			if ( g_LowLevelPetAttackSerial ) g_LowLevelPetAttackSerial = 0;
		}

		//박철호 : 2005-11-17 오후 8:43:19
		//pc 방 이펙트 
		if ( cPCBANGPet.PetShow ) { // 뒤에 pc방아이피가 맞는지 확인하는 체크루틴
			lpEffectItem->code =smTRANSCODE_EFFECT_PCBANGPET;
			lpEffectItem->size = sizeof(smEFFECT_ITEM);
			lpEffectItem->dwItemCode = cPCBANGPet.PetKind;
			lpEffectItem->ColorBlink = g_HoPCBangPetAttackSerial;
			ZeroMemory( lpEffectItem->sColors , sizeof( short ) *4 );
			lpEffectItem->DispEffect = 0;
			lpEffectItem->BlinkScale = 0;

			lpTransPlayData->size += lpEffectItem->size;
			lpTransPlayData->bEventInfo[TRANS_PLAYDATA_EVENT_EXT] ++;
			lpEffectItem++;

			if ( g_HoPCBangPetAttackSerial ) g_HoPCBangPetAttackSerial = 0;
		}

		if ( dwClanManageBit ) {
			lpEffectItem->code =smTRANSCODE_EFFECT_CLANMGR;
			lpEffectItem->size = sizeof(smEFFECT_ITEM);
			lpEffectItem->dwItemCode = dwClanManageBit;
			lpEffectItem->ColorBlink = 0;
			ZeroMemory( lpEffectItem->sColors , sizeof( short ) *4 );
			lpEffectItem->DispEffect = 0;
			lpEffectItem->BlinkScale = 0;

			lpTransPlayData->size += lpEffectItem->size;
			lpTransPlayData->bEventInfo[TRANS_PLAYDATA_EVENT_EXT] ++;
			lpEffectItem++;
		}

#ifndef _W_SERVER
		if ( OnStageField>=0 && StageField[OnStageField]->State==FIELD_STATE_CASTLE ) {
			//블래스 캐슬 개인 정보
			if ( sBlessCastle_Damage[0] || sBlessCastle_Damage[1] ) {
				lpEffectItem->code =smTRANSCODE_EFFECT_BLESS_SCORE;
				lpEffectItem->size = sizeof(smEFFECT_ITEM);
				lpEffectItem->dwItemCode = 0;
				lpEffectItem->ColorBlink = 0;
				lpEffectItem->sColors[0] = sBlessCastle_Damage[0];
				lpEffectItem->sColors[1] = sBlessCastle_Damage[1];
				lpEffectItem->sColors[2] = 0;
				lpEffectItem->sColors[3] = 0;
				lpEffectItem->DispEffect = 0;
				lpEffectItem->BlinkScale = 0;

				lpTransPlayData->size += lpEffectItem->size;
				lpTransPlayData->bEventInfo[TRANS_PLAYDATA_EVENT_EXT] ++;
				lpEffectItem++;
			}
		}

#endif

	srTransBuffSize = lpTransPlayData->size;

	return TRUE;
}

//수신된 데이타의 장착 아이템에 이펙트 적용 
int smCHAR::SetTransEffectItems(smEFFECT_ITEM *lpEffectItem)
{
	if ( HvRightHand.PatTool && HvRightHand.dwItemCode==lpEffectItem->dwItemCode ) {
		HvRightHand.ColorBlink = lpEffectItem->ColorBlink;
		memcpy( HvRightHand.sColors , lpEffectItem->sColors , sizeof( short ) *4 );
		HvRightHand.DispEffect = lpEffectItem->DispEffect;
		HvRightHand.BlinkScale = lpEffectItem->BlinkScale;

		//색상으로 아이템의 상태를 판별한다
		GetItemKindFromBliankColor( &HvRightHand );
		if ( HvRightHand.DispEffect ) AssaParticle_ClassUpWeapon(this);

	}

	if ( HvLeftHand.PatTool && HvLeftHand.dwItemCode==lpEffectItem->dwItemCode ) {
		HvLeftHand.ColorBlink = lpEffectItem->ColorBlink;
		memcpy( HvLeftHand.sColors , lpEffectItem->sColors , sizeof( short ) *4 );
		HvLeftHand.DispEffect = lpEffectItem->DispEffect;
		HvLeftHand.BlinkScale = lpEffectItem->BlinkScale;

		//색상으로 아이템의 상태를 판별한다
		GetItemKindFromBliankColor( &HvLeftHand );
		if ( HvLeftHand.DispEffect ) AssaParticle_ClassUpWeapon(this);
	}

	return TRUE;
}

//수신된 데이타의 장착 무기에 속성 이펙트 적용 
int smCHAR::SetTransEffectWeapon(smEFFECT_ITEM *lpEffectItem)
{
	int	time;

	if ( dwWeaponEffectTime && WeaponEffect==lpEffectItem->dwItemCode ) {
		time = abs(dwWeaponEffectTime-dwPlayTime);		//이펙트 남은시간
		if ( abs(time-lpEffectItem->ColorBlink)>6000 ) {
			switch( lpEffectItem->dwItemCode ) {
				case (sITEMINFO_FIRE+1):
					AssaParticle_EnchantWeaponFireMember( this , lpEffectItem->ColorBlink/1000 );
					break;
				case (sITEMINFO_ICE+1):
					AssaParticle_EnchantWeaponIceMember( this , lpEffectItem->ColorBlink/1000 );
					break;
				case (sITEMINFO_LIGHTING+1):
					AssaParticle_EnchantWeaponLightMember( this , lpEffectItem->ColorBlink/1000 );
					break;
			}
			dwWeaponEffectTime = dwPlayTime+lpEffectItem->ColorBlink;
			EnchantEffect_Point = lpEffectItem->DispEffect;
		}
	}
	else {
		switch( lpEffectItem->dwItemCode ) {
			case (sITEMINFO_FIRE+1):
				AssaParticle_EnchantWeaponFireMember( this , lpEffectItem->ColorBlink/1000 );
				break;
			case (sITEMINFO_ICE+1):
				AssaParticle_EnchantWeaponIceMember( this , lpEffectItem->ColorBlink/1000 );
				break;
			case (sITEMINFO_LIGHTING+1):
				AssaParticle_EnchantWeaponLightMember( this , lpEffectItem->ColorBlink/1000 );
				break;
		}
		WeaponEffect = lpEffectItem->dwItemCode;
		dwWeaponEffectTime = dwPlayTime+lpEffectItem->ColorBlink;
		EnchantEffect_Point = lpEffectItem->DispEffect;
	}

	return TRUE;
}

//자신의 데이타를 소켓을 통하여 다른 곳으로 전송 ( 서버에서 클라이언트로 전송용 )
int smCHAR::SendPlayDataSock( smWINSOCK *lpsmsock , char *lpTransBuff , int ex, int ey, int ez )
{
	int SendTime;
	smPLAYDATA PlayData;
	int dist;
	int x,y,z;


//	TransLastSendCnt++;

	if ( (PlayBuffCnt&0x3F)!=0 ) return FALSE;

	SendTime = 0x40;

	x = (pX-ex)>>FLOATNS;
	y = (pY-ey)>>FLOATNS;
	z = (pZ-ez)>>FLOATNS;

	dist = x*x+y*y+z*z;

	TransLevel = 3;

	if ( dist<DIST_TRANSLEVEL_HIGH || !DisplayFlag ) {
		TransLevel = 0;
	}
	else {
		if ( dist<DIST_TRANSLEVEL_MID ) {
			TransLevel = 1;
		}
		else {
			if ( dist<DIST_TRANSLEVEL_LOW ) {
				TransLevel = 2;
			}
		}
	}

		if ( TransLevel>1 ) {
			//버퍼링 없는 데이타 전송 ( 통신의 중요도가 낮음 )
			if ( (PlayBuffCnt&0xFF)!=0 ) return FALSE;

			PlayData.size = sizeof( smPLAYDATA );
			PlayData.code = smTRANSCODE_PLAYDATA1;
			PlayData.x = pX;
			PlayData.y = pY;
			PlayData.z = pZ;

			PlayData.angle[0] = Angle.x;
			PlayData.angle[1] = Angle.y;
			PlayData.angle[2] = Angle.z;
			PlayData.angle[3] = action;

			PlayData.frame = frame;
			PlayData.dwObjectSerial = dwObjectSerial;

			if ( lpsmsock->Send( (char *)&PlayData , sizeof( smPLAYDATA ) , TRUE ) ) {
				//SetDistSendCnt( player );				//통신 시간차 조절
			}
		}
		else {
			/*
			//버퍼링 데이타 전송 ( 고품질의 데이타 전송 )
			SendMode = 0;
			PlayerBuffCnt = PlayBuffCnt&PLAYBUFF_MASK;
			StartPosi = ( PlayerBuffCnt - SendTime) & PLAYBUFF_MASK;

			mAction = PlayBuff[ PlayerBuffCnt ].angle[3];


			for( cnt=SendTime; cnt>=0; cnt-- ) {
				BuffPosi = (StartPosi + cnt ) & PLAYBUFF_MASK;
				if ( PlayBuff[BuffPosi].angle[3]!=mAction ) {
					SendMode = 1;
					memcpy( &PlayData3.PlayBuff[2] , &PlayBuff[BuffPosi] , sizeof( smPLAYBUFF ) );
					PlayData3.BuffCount[2] = cnt;
					break;
				}
			}

			if ( !SendMode ) {
				memcpy( &PlayData2.PlayBuff[0] , &PlayBuff[ StartPosi ] , sizeof( smPLAYBUFF ) );
				memcpy( &PlayData2.PlayBuff[1] , &PlayBuff[ PlayerBuffCnt ] , sizeof( smPLAYBUFF ) );
				PlayData2.code = smTRANSCODE_PLAYDATA2;
				PlayData2.size = sizeof( smPLAYDATA2 );
				PlayData2.BuffCount[0] = 0;
				PlayData2.BuffCount[1] = SendTime;
				PlayData2.dwObjectSerial = dwObjectSerial;

				if ( lpsmsock->Send( (char *)&PlayData2 , sizeof( smPLAYDATA2 ) ) ) {
					//TransSendWait = 1;
					//TransLastSendCnt = PlayCounter;
					//TransLastSendTime = SendTime;
					//SetDistSendCnt( player );				//통신 시간차 조절
				}
			}
			else {
				memcpy( &PlayData3.PlayBuff[0] , &PlayBuff[ StartPosi ] , sizeof( smPLAYBUFF ) );
				memcpy( &PlayData3.PlayBuff[1] , &PlayBuff[ PlayerBuffCnt ] , sizeof( smPLAYBUFF ) );
				PlayData3.code = smTRANSCODE_PLAYDATA3;
				PlayData3.size = sizeof( smPLAYDATA3 );
				PlayData3.BuffCount[0] = 0;
				PlayData3.BuffCount[1] = SendTime;
				PlayData3.dwObjectSerial = dwObjectSerial;

				if ( lpsmsock->Send( (char *)&PlayData3 , sizeof( smPLAYDATA3 ) ) ) {
					//TransSendWait = 1;
					//TransLastSendCnt = PlayCounter;
					//TransLastSendTime = SendTime;
					//SetDistSendCnt( player );				//통신 시간차 조절
				}
			}
			*/

//					lpCurPlayer->SetMotionFromCode( CHRMOTION_STATE_DEAD );
//					if ( MotionInfo->State==CHRMOTION_STATE_ATTACK )


			if ( lpsmsock->Send( lpTransBuff , ((smTRANS_PLAYDATA *)lpTransBuff)->size) , TRUE  ) {
					//TransSendWait = 1;
					//TransLastSendCnt = PlayCounter;
					//TransLastSendTime = SendTime;
					//SetDistSendCnt( player );				//통신 시간차 조절
			}
			
		}
//	}

	return TRUE;
}



//캐릭터의 포괄적 정보를 소켓으로 상대방에 전송
int smCHAR::SendCharInfo( smWINSOCK *lpsmsock )
{

	smTRNAS_PLAYERINFO	TransPlayerInfo;

	memcpy( &TransPlayerInfo.smCharInfo , &smCharInfo , sizeof( smCHAR_INFO ) );
	TransPlayerInfo.size = sizeof( smTRNAS_PLAYERINFO );
	TransPlayerInfo.code = smTRANSCODE_PLAYERINFO;
	TransPlayerInfo.dwObjectSerial = dwObjectSerial;

	TransPlayerInfo.x = pX;
	TransPlayerInfo.y = pY;
	TransPlayerInfo.z = pZ;

	return lpsmsock->Send( (char *)&TransPlayerInfo , TransPlayerInfo.size , TRUE );
}

//스킬시작
int smCHAR::BeginSkill( int SkilCode , int Level , smCHAR *lpTarChar , int x, int y, int z )
{
	int point;
	int param;
	POINT3D pos;
	int cnt;
	POINT3D	Pos1,Pos2;

	if ( smCharInfo.State==smCHAR_STATE_ENEMY && PkMode_CharState!=smCHAR_STATE_USER ) {
		return BeginSkill_Monster();
	}

	AttackEffect = 0;
	//point = SkilCode>>8;
	point = (SkilCode>>8)&0xF;
	param = (SkilCode>>12)&0xF;

	switch( SkilCode&0xFF )
	{

	case SKILL_PLAY_EXTREME_SHIELD:
		SkillPlaySound( SKILL_SOUND_EXTREME_SHIELD , pX,pY,pZ );			//스킬 효과음
		break;

	case SKILL_PLAY_MECHANIC_BOMB:
		//메카닉 붐
//		StartSkillDest(pX,pY+12*fONE,pZ , lpTarChar->pX,lpTarChar->pY,lpTarChar->pZ, SKILL_MECHANIC_BOMB , 1 );
//		Skil_RangeAttack( lpTarChar->pX, lpTarChar->pY, lpTarChar->pZ, 100*fONE , 20 , 0 );
		HideWeapon = TRUE;
		break;

	case SKILL_PLAY_PHYSICAL_ABSORB:
		//픽지클 옵져브
		dwDispAppSkill |= SKILL_APPMASK_PHYSICAL_ABSORB;
		StartSkill( pX,pY,pZ , 0, Angle.y , 0, SKILL_PHYSICAL_ABSORB );
		StartSkill( pX,pY,pZ , 0, Angle.y , 0, this , SKILL_SHIELD1 , point );
		SkillPlaySound( SKILL_SOUND_PHYSICAL_ABSORB1 , pX,pY,pZ );			//스킬 효과음
		//SkillPlaySound( SKILL_SOUND_PHYSICAL_ABSORB2 , pX,pY,pZ );			//스킬 효과음
		break;

	case SKILL_PLAY_POISON_ATTRIBUTE:
		//포이즌 어트리븃
		StartSkill( pX,pY,pZ , 0, Angle.y , 0, SKILL_UP1 );
		SetMotionFromCode( CHRMOTION_STATE_ATTACK );
		break;


	case SKILL_PLAY_RAVING:
	case SKILL_PLAY_IMPACT:
		//임팩트
		AttackEffect = TRUE;					//타격시 효과
		break;

	case SKILL_PLAY_PIKEWIND:
		//파이크 윈드
		break;

	case SKILL_PLAY_JUMPING_CRASH:
		//점핑 크래시
		AttackEffect = TRUE;
		SkillPlaySound( SKILL_SOUND_JUMPING_CRASH1 , pX,pY,pZ );			//스킬 효과음
		break;

	case SKILL_PLAY_SCOUT_HAWK:
		//스카우트 호크
		if ( HoSkillCode ) {
			//기존 스킬 이펙트 강제로 종료
			HoSkillCode = 0;
			EffectMgr.Main();
		}

		StartSkill( pX,pY,pZ , 0, Angle.y , 0, this , SKILL_SCOUT_HAWK , point );
		SkillPlaySound( SKILL_SOUND_SCOUT_HAWK1 , pX,pY,pZ );			//스킬 효과음
		SkillPlaySound( SKILL_SOUND_SCOUT_HAWK2 , pX,pY,pZ );			//스킬 효과음
		HideWeapon = TRUE;
		break;

	case SKILL_PLAY_WIND_ARROW:
		break;

	case SKILL_PLAY_PERFECT_AIM:
		break;

	case SKILL_PLAY_MAXIMIZE:
		//맥시마이즈
		dwDispAppSkill |= SKILL_APPMASK_AUTOMATION;
		StartSkill( pX,pY,pZ, Angle.x , Angle.y , Angle.z , SKILL_MAXIMIZE ); 
		StartSkill( pX,pY,pZ , 0, Angle.y , 0, this , SKILL_SHIELD2 , point );
		SkillPlaySound( SKILL_SOUND_SKILL_MAXIMIZE1	 , pX,pY,pZ );
		break;

	case SKILL_PLAY_AUTOMATION:
		//오토메이션
		dwDispAppSkill |= SKILL_APPMASK_AUTOMATION;
		StartSkill( pX,pY,pZ, Angle.x , Angle.y , Angle.z , SKILL_AUTOMATION ); 
		StartSkill( pX,pY,pZ , 0, Angle.y , 0, this , SKILL_SHIELD2 , point );
		SkillPlaySound( SKILL_SOUND_SKILL_AUTOMATION1	 , pX,pY,pZ );
		break;

	case SKILL_PLAY_SPARK:
		//스파크
		HideWeapon = TRUE;
		SkillPlaySound( SKILL_SOUND_SKILL_SPARK1	 , pX,pY,pZ );
		break;


	case SKILL_PLAY_METAL_ARMOR:
		//메탈아머
		AssaParticle_MetalArmor( this, Metal_Armor_Time[point-1] );
		SkillPlaySound( SKILL_SOUND_SKILL_METAL_ARMOR	 , pX,pY,pZ );
		break;

	case SKILL_PLAY_SPARK_SHIELD:
		//스파크쉴드
		SkillPlaySound( SKILL_SOUND_SKILL_SPARK1	 , pX,pY,pZ );
		break;

//	case SKILL_PLAY_GROUND_PIKE:
//		break;

/*
	case SKILL_PLAY_SPARK:
		//스파크
		StartSkillDest( pX,pY,pZ, Angle.x , Angle.y , Angle.z , SKILL_AUTOMATION ); 
		break;
*/

	case SKILL_PLAY_GROUND_PIKE:
		//그라운드 파이크
		//if( chrAttackTarget ) {
		//StartSkill( pX,pY,pZ, 0, Angle.y,0, SKILL_GROUND_PIKE , point );
		//}
		break;

	case SKILL_PLAY_TORNADO:
		//토네이도
		//StartSkillDest( pX,pY,pZ, lpTarChar->pX,lpTarChar->pY,lpTarChar->pZ, SKILL_TORNADO , point );
		break;

	case SKILL_PLAY_EXPANSION:
		SkillPlaySound( SKILL_SOUND_SKILL_EXPANSION1	 , pX,pY,pZ );
		break;

	case SKILL_PLAY_VENOM_SPEAR:
		AssaParticle_VeonmSpearPike( this );
		SkillPlaySound( SKILL_SOUND_JUMPING_CRASH1 , pX,pY,pZ );			//스킬 효과음
		break;

	case SKILL_PLAY_CHAIN_LANCE:
		//SkillPlaySound( SKILL_SOUND_SKILL_CHAIN_LANCE1 , pX,pY,pZ );			//스킬 효과음
		break;

	case SKILL_PLAY_VANISH:
		AssaParticle_Vanish(this);
		SkillPlaySound( SKILL_SOUND_SKILL_VANISH1	 , pX,pY,pZ );
		break;


	case SKILL_PLAY_FALCON:
		//팔콘
		if ( HoSkillCode ) {
			//기존 스킬 이펙트 강제로 종료
			HoSkillCode = 0;
			EffectMgr.Main();
		}

		StartSkill( pX,pY,pZ, 0,0,0, this , SKILL_FALCON , point-1 );
		HideWeapon = TRUE;
		SkillPlaySound( SKILL_SOUND_SKILL_FALCON	 , pX,pY,pZ );
		break;

	case SKILL_PLAY_GOLDEN_FALCON:
		//골든 팔콘
		if ( HoSkillCode ) {
			//기존 스킬 이펙트 강제로 종료
			HoSkillCode = 0;
			EffectMgr.Main();
		}

		AssaParticle_GoldenFalcon( this , Golden_Falcon_Time[point-1] );
		HideWeapon = TRUE;
		SkillPlaySound( SKILL_SOUND_SKILL_GOLDEN_FALCON	 , pX,pY,pZ );
		break;

	case SKILL_PLAY_RAGEOF_ZECRAM:
		StartSkill( pX,pY+24*fONE,pZ, 0,0,0, SKILL_RAGE_OF_ZECRAM_POWER1 );
		SkillPlaySound( SKILL_SOUND_SKILL_RAGE_OF_ZECRAM3 , pX,pY,pZ );			//스킬 효과음
		break;

	case SKILL_PLAY_ROAR:
		//로어
		StartSkill( pX,pY,pZ, 0,0,0, SKILL_ROAR, point );

		if ( (dwPlayTime&1)==0 )
			SkillPlaySound( SKILL_SOUND_SKILL_ROAR1	 , pX,pY,pZ );
		else
			SkillPlaySound( SKILL_SOUND_SKILL_ROAR2	 , pX,pY,pZ );

		break;

	case SKILL_PLAY_CONCENTRATION:
		AssaParticle_Concentration( this , Concentration_Time[point-1] );
		SkillPlaySound( SKILL_SOUND_SKILL_CONCENTRATION1 , pX,pY,pZ );
		break;

	case SKILL_PLAY_SWIFT_AXE:
		AssaParticle_SwiftAxe( this , Swift_Axe_Time[point-1] );
		SkillPlaySound( SKILL_SOUND_SKILL_SWIFT_AXE1 , pX,pY,pZ );
		break;

/*
	case SKILL_PLAY_METEO:
		//메테오
		if ( lpTarChar ) {
			Skil_RangeAttack( lpTarChar->pX, lpTarChar->pY, lpTarChar->pZ, 160*fONE , 20 , 0 );
			SetMotionFromCode( CHRMOTION_STATE_ATTACK );
			StartSkillDest( pX,pY,pZ , lpTarChar->pX,lpTarChar->pY,lpTarChar->pZ , SKILL_METEO );
		}
		else
			return FALSE;
*/
		break;



		//////////////////////////// 나이트 ///////////////////////////

	case SKILL_PLAY_SWORD_BLAST:
		SkillPlaySound( SKILL_SOUND_SKILL_SWORDBLAST1	 , pX,pY,pZ );
		break;

	case SKILL_PLAY_BRANDISH:
		//sinSkillEffect_Brandish( this );
		SkillPlaySound( SKILL_SOUND_SKILL_BLANDISH1			 , pX,pY,pZ );
		break;

	case SKILL_PLAY_HOLY_BODY:
		//홀리바디
		dwDispAppSkill |= SKILL_APPMASK_PHYSICAL_ABSORB;
		sinEffect_Holy_Body( this );
		StartSkill( pX,pY,pZ , 0, Angle.y , 0, this , SKILL_SHIELD1 , point );
		SkillPlaySound( SKILL_SOUND_SKILL_HOLYBODY			 , pX,pY,pZ );
		break;

	case SKILL_PLAY_HOLY_VALOR:
		AssaParticle_HolyValor_Jang( this , Holy_Valor_Time[ point-1 ] );
		SkillPlaySound( SKILL_SOUND_SKILL_HOLY_VALOR1			 , pX,pY,pZ );
		break;

	case SKILL_PLAY_PIERCING:
		//피어싱
		sinSkillEffect_Piercing( this );
		SkillPlaySound( SKILL_SOUND_SKILL_PIERCING			 , pX,pY,pZ );
		break;

	case SKILL_PLAY_FLAME_BRANDISH:
		//프레임 브랜디시
		SkillPlaySound( SKILL_SOUND_SKILL_BLANDISH1			 , pX,pY,pZ );
		break;

	case SKILL_PLAY_DRASTIC_SPIRIT:
		//드래스틱스피리트
		AssaParticle_DrasticSpirit_Jang( this , Drastic_Spirit_Time[ point-1 ] );
		SkillPlaySound( SKILL_SOUND_SKILL_DRASTICSPIRIT1			 , pX,pY,pZ );
		break;

	case SKILL_PLAY_DIVINE_INHALATION:
		//디바인인홀레이션
		SkillPlaySound( SKILL_SOUND_SKILL_DIVINE_INHALATION	 , pX,pY,pZ );
		break;

	case SKILL_PLAY_HOLY_INCANTATION:
		AssaParticle_HolyIncantationHand( this );
		break;

		////////////////////////// 아탈란타 ////////////////////////////
	case SKILL_PLAY_WINDY:
		//윈디
		sinSkillEffect_Windy( this );
		AssaParticle_Windy( this , Windy_Time[ point-1 ] );
		SkillPlaySound( SKILL_SOUND_SKILL_WINDY1			 , pX,pY,pZ );
		break;

	case SKILL_PLAY_SPLIT_JAVELIN:
		if ( chrAttackTarget && AttackCritcal>=0 && GetAttackPoint( &x,&y,&z )==TRUE ) {
			Pos1.x = x;
			Pos1.y = y;
			Pos1.z = z;
			Pos2.x = chrAttackTarget->pX;
			Pos2.y = chrAttackTarget->pY+28*fONE;
			Pos2.z = chrAttackTarget->pZ;

			AssaParticle_SplitJavelin( &Pos1, &Pos2 , param );
		}

		SkillPlaySound( SKILL_SOUND_SKILL_SPLIT_JAVELIN1	 , pX,pY,pZ );
		break;

	case SKILL_PLAY_TRIUMPH_OF_VALHALLA:
		SkillPlaySound( SKILL_SOUND_SKILL_TRIUMPH_OF_VALHALLA1 , pX,pY,pZ );
		break;

		////////////////////////// 프리스티스 ///////////////////////////

	case SKILL_PLAY_HEALING:
		if ( !lpTarChar && this==lpCurPlayer ) {
			sinEffect_Healing2( this );		//힐링 이펙트
			SkillPlaySound( SKILL_SOUND_SKILL_HEALING	 , pX,pY,pZ );		//힐링 사운드
		}
		else {
			if ( lpTarChar ) {
				sinEffect_Healing2( lpTarChar );		//힐링 이펙트
				SkillPlaySound( SKILL_SOUND_SKILL_HEALING	 , lpTarChar->pX,lpTarChar->pY,lpTarChar->pZ );		//힐링 사운드
			}
		}

		SkillPlaySound( SKILL_SOUND_SKILL_CASTING_PRIEST	 , pX,pY,pZ );		//마법 소리

		pos.x = pX;	pos.y = pY;	pos.z = pZ;
		sinEffect_StartMagic( &pos , 1 );			//마법진 이펙트
		break;

	case SKILL_PLAY_HOLY_MIND:
		if ( lpTarChar ) {
			cnt = lpTarChar->smCharInfo.Resistance[sITEMINFO_BIONIC];
			if ( cnt<0 ) cnt=0;
			if ( cnt>100 ) cnt=100;
			cnt = (15*(100-cnt))/100;
			if ( cnt>0 )
				AssaParticle_HolyMind_Attack( lpTarChar , cnt );
		}

		SkillPlaySound( SKILL_SOUND_SKILL_HOLYMIND			 , pX,pY,pZ );		//홀리 마인드소리
		SkillPlaySound( SKILL_SOUND_SKILL_CASTING_PRIEST	 , pX,pY,pZ );		//마법 소리

		pos.x = pX;	pos.y = pY;	pos.z = pZ;
		sinEffect_StartMagic( &pos , 1 );			//마법진 이펙트
		break;

	case SKILL_PLAY_HOLY_BOLT:
	case SKILL_PLAY_MULTI_SPARK:
		SkillPlaySound( SKILL_SOUND_SKILL_CASTING_PRIEST	 , pX,pY,pZ );		//마법 소리

		pos.x = pX;	pos.y = pY;	pos.z = pZ;
		sinEffect_StartMagic( &pos , 1 );			//마법진 이펙트
		break;

	case SKILL_PLAY_DIVINE_LIGHTNING:
		pos.x = pX;	pos.y = pY;	pos.z = pZ;
		sinEffect_StartMagic( &pos , 1 );			//마법진 이펙트
		SkillPlaySound( SKILL_SOUND_SKILL_CASTING_PRIEST	 , pX,pY,pZ );		//마법 소리
		break;
	case SKILL_PLAY_HOLY_REFLECTION:
		sinSkillEffect_Holy_Reflection( this , Holy_Reflection_Time[point-1] );
		pos.x = pX;	pos.y = pY;	pos.z = pZ;
		sinEffect_StartMagic( &pos , 1 );			//마법진 이펙트
		SkillPlaySound( SKILL_SOUND_SKILL_CASTING_PRIEST	 , pX,pY,pZ );		//마법 소리
		SkillPlaySound( SKILL_SOUND_SKILL_HOLYREFLECTION1	 , pX,pY,pZ );
		break;
	case SKILL_PLAY_GREAT_HEALING:
		sinSkillEffect_Great_Healing( this );
		pos.x = pX;	pos.y = pY;	pos.z = pZ;
		sinEffect_StartMagic( &pos , 1 );			//마법진 이펙트
		SkillPlaySound( SKILL_SOUND_SKILL_CASTING_PRIEST	 , pX,pY,pZ );		//마법 소리
		SkillPlaySound( SKILL_SOUND_SKILL_GRANDHEALING	 , pX,pY,pZ );
		break;

	case SKILL_PLAY_VIGOR_BALL:
	case SKILL_PLAY_RESURRECTION:
	case SKILL_PLAY_EXTINCTION:
	case SKILL_PLAY_VIRTUAL_LIFE:
		SkillPlaySound( SKILL_SOUND_SKILL_CASTING_PRIEST	 , pX,pY,pZ );		//마법 소리

		pos.x = pX;	pos.y = pY;	pos.z = pZ;
		sinEffect_StartMagic( &pos , 1 );			//마법진 이펙트
		break;


		//////////////////////////// 매지션 /////////////////////////////////////

	case SKILL_PLAY_AGONY:
		sinEffect_Agony( this );
		SkillPlaySound( SKILL_SOUND_SKILL_CASTING_MAGICIAN	 , pX,pY,pZ );		//마법 소리
		SkillPlaySound( SKILL_SOUND_SKILL_AGONY	 , pX,pY,pZ );

		pos.x = pX;	pos.y = pY;	pos.z = pZ;
		sinEffect_StartMagic( &pos , 2 );			//마법진 이펙트
		break;
	case SKILL_PLAY_ZENITH:
		sinEffect_Zenith( this , Zenith_Time[ point-1 ] );
		SkillPlaySound( SKILL_SOUND_SKILL_CASTING_MAGICIAN	 , pX,pY,pZ );		//마법 소리
		SkillPlaySound( SKILL_SOUND_SKILL_ZENITH	 , pX,pY,pZ );

		pos.x = pX;	pos.y = pY;	pos.z = pZ;
		sinEffect_StartMagic( &pos , 2 );			//마법진 이펙트
		break;

	case SKILL_PLAY_FIRE_BALL:
	case SKILL_PLAY_FIRE_BOLT:
		SkillPlaySound( SKILL_SOUND_SKILL_CASTING_MAGICIAN	 , pX,pY,pZ );		//마법 소리

		pos.x = pX;	pos.y = pY;	pos.z = pZ;
		sinEffect_StartMagic( &pos , 2 );			//마법진 이펙트
		break;

	case SKILL_PLAY_COLUMN_OF_WATER:
		SkillPlaySound( SKILL_SOUND_SKILL_CASTING_MAGICIAN	 , pX,pY,pZ );		//마법 소리
		pos.x = pX;	pos.y = pY;	pos.z = pZ;
		sinEffect_StartMagic( &pos , 2 );			//마법진 이펙트
		break;
	case SKILL_PLAY_ENCHANT_WEAPON:
		SkillPlaySound( SKILL_SOUND_SKILL_CASTING_MAGICIAN	 , pX,pY,pZ );		//마법 소리
		pos.x = pX;	pos.y = pY;	pos.z = pZ;
		sinEffect_StartMagic( &pos , 2 );			//마법진 이펙트

		pos.x = pX;
		pos.y = pY;
		pos.z = pZ;

		if ( this==lpCurPlayer ) {
			if ( !chrAttackTarget ) {
				cnt = Enchant_Weapon_Time[point-1];
				EnchantEffect_Point = point;
				switch( (param) ) {
					case 0:
						AssaParticle_EnchantWeaponIceJang( this , cnt );
						WeaponEffect = sITEMINFO_ICE+1;
						dwWeaponEffectTime = dwPlayTime + cnt*1000;
						break;
					case 1:
						AssaParticle_EnchantWeaponLightJang( this , cnt );
						WeaponEffect = sITEMINFO_LIGHTING+1;
						dwWeaponEffectTime = dwPlayTime + cnt*1000;
						break;
					case 2:
						AssaParticle_EnchantWeaponFireJang( this , cnt );
						WeaponEffect = sITEMINFO_FIRE+1;
						dwWeaponEffectTime = dwPlayTime + cnt*1000;
						break;
				}
			}
		}
		SkillPlaySound( SKILL_SOUND_SKILL_ENCHANTWEAPON_CAST	 , pX,pY,pZ );
		break;

	case SKILL_PLAY_DEAD_RAY:
		SkillPlaySound( SKILL_SOUND_SKILL_CASTING_MAGICIAN	 , pX,pY,pZ );		//마법 소리
		pos.x = pX;	pos.y = pY;	pos.z = pZ;
		sinEffect_StartMagic( &pos , 2 );			//마법진 이펙트
		break;

	case SKILL_PLAY_ENERGY_SHIELD:
	case SKILL_PLAY_DIASTROPHISM:
	case SKILL_PLAY_SPIRIT_ELEMENTAL:
	case SKILL_PLAY_DANCING_SWORD:

		SkillPlaySound( SKILL_SOUND_SKILL_CASTING_MAGICIAN	 , pX,pY,pZ );		//마법 소리
		pos.x = pX;	pos.y = pY;	pos.z = pZ;
		sinEffect_StartMagic( &pos , 2 );			//마법진 이펙트
		break;


	case SKILL_PLAY_COMPULSION:
		SkillPlaySound( SKILL_SOUND_SKILL_COMPULSION1	 , pX,pY,pZ );
		break;

	case SKILL_PLAY_IMPULSION:
		SkillImpulsionLight( this , 2 );
		break;

	case SKILL_PLAY_MAGNETIC_SPHERE:
		SkillPlaySound( SKILL_SOUND_SKILL_SPARK1	 , pX,pY,pZ );
		break;

	case SKILL_PLAY_METAL_GOLEM:
		SkillPlaySound( SKILL_SOUND_SKILL_METAL_GOLEM	 , pX,pY,pZ );
		break;

	case SKILL_PLAY_DESTROYER:
		SkillWarriorDestroyerBlade(this, 3.0f );          //잔상
		break;

	case SKILL_PLAY_BERSERKER:
		SkillPlaySound( SKILL_SOUND_SKILL_BERSERKER	 , pX,pY,pZ );
		break;

	case SKILL_PLAY_CYCLONE_STRIKE:
		SkillWarriorCycloneStrikeBlade( this , 3 );
		SkillPlaySound( SKILL_SOUND_SKILL_CYCLONE_STRIKE , pX,pY,pZ );
		break;

	case SKILL_PLAY_ASSASSIN_EYE:
		SkillPlaySound( SKILL_SOUND_SKILL_ASSASSIN_EYE	 , pX,pY,pZ );
		break;

	case SKILL_PLAY_CHARGING_STRIKE:
	case SKILL_PLAY_SHADOW_MASTER:
		if ( this==lpCurPlayer ) SkillEventDamageCount = 0;
		break;

	case SKILL_PLAY_PHOENIX_SHOT:
		//SkillPlaySound( SKILL_SOUND_SKILL_CHARGING	 , pX,pY,pZ );
		break;

	case SKILL_PLAY_VAGUE:
		SkillPlaySound( SKILL_SOUND_SKILL_VAGUE	 , pX,pY,pZ );
		break;

	case SKILL_PLAY_GODS_BLESS:
		SkillSaintGodBless( this , (float)God_Bless_Time[point-1] );
		SkillPlaySound( SKILL_SOUND_SKILL_GODS_BLESS	 , pX,pY,pZ );
		break;

	case SKILL_PLAY_RECALL_WOLVERIN:
		break;

	case SKILL_PLAY_FORCE_OF_NATURE:
		SkillPlaySound( SKILL_SOUND_SKILL_FORCE_OF_NATURE , pX,pY,pZ );
		break;

	case SKILL_PLAY_GODLY_SHIELD:
		SkillPlaySound( SKILL_SOUND_SKILL_GODLY_SHIELD	 , pX,pY,pZ );
		break;

	case SKILL_PLAY_SWORD_OF_JUSTICE:
		SkillPlaySound( SKILL_SOUND_SKILL_SWORD_OF_JUSTICE	 , pX,pY,pZ );
		break;

	case SKILL_PLAY_HALL_OF_VALHALLA:
		SkillPlaySound( SKILL_SOUND_SKILL_HALL_OF_VALHALLA	 , pX,pY,pZ );
		break;
	case SKILL_PLAY_FROST_JAVELIN:
		SkillPlaySound( SKILL_SOUND_SKILL_FROST_JAVELIN	 , pX,pY,pZ );
		break;

	case SKILL_PLAY_GLACIAL_SPIKE:
		SkillPlaySound( SKILL_SOUND_SKILL_CASTING_PRIEST	 , pX,pY,pZ );		//마법 소리
		pos.x = pX;	pos.y = pY;	pos.z = pZ;
		sinEffect_StartMagic( &pos , 1 );			//마법진 이펙트
		break;

	case SKILL_PLAY_REGENERATION_FIELD:
		SkillPlaySound( SKILL_SOUND_SKILL_REGEN_FIELD2	 , pX,pY,pZ );
		pos.x = pX;	pos.y = pY;	pos.z = pZ;
		sinEffect_StartMagic( &pos , 1 );			//마법진 이펙트
		break;

	case SKILL_PLAY_CHAIN_LIGHTNING:
		SkillPlaySound( SKILL_SOUND_SKILL_CASTING_PRIEST	 , pX,pY,pZ );		//마법 소리

		pos.x = pX;	pos.y = pY;	pos.z = pZ;
		sinEffect_StartMagic( &pos , 1 );			//마법진 이펙트
		break;

	case SKILL_PLAY_SUMMON_MUSPELL:
		SkillPlaySound( SKILL_SOUND_SKILL_MUSPELL1	 , pX,pY,pZ );		//마법 소리

		pos.x = pX;	pos.y = pY;	pos.z = pZ;
		sinEffect_StartMagic( &pos , 1 );			//마법진 이펙트
		break;

	case SKILL_PLAY_FIRE_ELEMENTAL:
		SkillPlaySound( SKILL_SOUND_SKILL_FIRE_ELEMENTAL , pX,pY,pZ );
		//SkillPlaySound( SKILL_SOUND_SKILL_CASTING_MAGICIAN	 , pX,pY,pZ );		//마법 소리
		pos.x = pX;	pos.y = pY;	pos.z = pZ;
		sinEffect_StartMagic( &pos , 2 );			//마법진 이펙트
		break;

	case SKILL_PLAY_FLAME_WAVE:
		SkillPlaySound( SKILL_SOUND_SKILL_FLAME_WAVE3 , pX,pY,pZ );
		//SkillPlaySound( SKILL_SOUND_SKILL_CASTING_MAGICIAN	 , pX,pY,pZ );		//마법 소리
		pos.x = pX;	pos.y = pY;	pos.z = pZ;
		sinEffect_StartMagic( &pos , 2 );			//마법진 이펙트
		break;

	case SKILL_PLAY_DISTORTION:
		SkillPlaySound( SKILL_SOUND_SKILL_DISTORTION , pX,pY,pZ );
		//SkillPlaySound( SKILL_SOUND_SKILL_CASTING_MAGICIAN	 , pX,pY,pZ );		//마법 소리
		pos.x = pX;	pos.y = pY;	pos.z = pZ;
		sinEffect_StartMagic( &pos , 2 );			//마법진 이펙트
		break;

	case SKILL_PLAY_METEO:
		SkillPlaySound( SKILL_SOUND_SKILL_CASTING_MAGICIAN	 , pX,pY,pZ );		//마법 소리
		pos.x = pX;	pos.y = pY;	pos.z = pZ;
		sinEffect_StartMagic( &pos , 2 );			//마법진 이펙트
		break;

	}

	//AttackSkil = SkilCode;

	return TRUE;
}

//몬스터 일반 공격 시작
int smCHAR::BeginAttack_Monster()
{
	switch( smCharInfo.dwCharSoundCode ) {
		case snCHAR_SOUND_DARKKNIGHT:
			//둠 가드
			if ( smCharInfo.Level>=65 )
				AssaParticle_DoomGuardHit1( this );
			break;

		case snCHAR_SOUND_GUARDIAN_SAINT:
			//가디안 - 세인트
			//AssaParticleClanMonsterHit2( this );
			break;

		case snCHAR_SOUND_DEATHKNIGHT:			//발렌토
			if ( MotionInfo->KeyCode!='L' ) 
				ParkValentSwordShow( this, 70 );
			break;

		case snCHAR_SOUND_DEVIL_BIRD:			//데블버드
			if ( MotionInfo->KeyCode!='B' ) 
				ParkDevilBirdAttackBlur( this );
			break;


		case snCHAR_SOUND_KELVEZU:
			if ( MotionInfo->KeyCode=='P' ) {
				ParkKelvezuWingShow( this, 70 );
			}
			else {
				ParkKelvezuFingerShow( this, 70 );
			}

			break;

		case snCHAR_SOUND_CHIMERA:
			ParkAssaParticle_ChimeraNormal( this );
			break;
		case snCHAR_SOUND_REVIVED_ARCHER:
			//화살 시위 당기기
			break;
	}

	return TRUE;
}

//몬스터 스킬 시작
int smCHAR::BeginSkill_Monster()
{
	POINT3D	pos;

	switch( smCharInfo.dwCharSoundCode ) {
		case snCHAR_SOUND_NAZ:
			GetMoveLocation( 0 , 0, 100*fONE , 0, Angle.y , 0 );
			StartSkillDest( pX,pY,pZ, pX+GeoResult_X,pY,pZ+GeoResult_Z, SKILL_TORNADO2 , 1 );
			break;

		case snCHAR_SOUND_FURY:
			StartEffectMonster(pX,pY,pZ, 0, Angle.y, 0, MONSTER_FURY_MAGIC1);
			break;

		case snCHAR_SOUND_DARKKNIGHT:
			//둠 가드
			if ( smCharInfo.Level>=65 )
				AssaParticle_DoomGuardHit2( this );
			break;

		case snCHAR_SOUND_HEAVYGOBLIN:
			//헤비 고블린
			AssaParticle_HeavyGoblinHit1( this );
			break;

		case snCHAR_SOUND_ICEGOBLIN:			//아이스 고블린
			ParkAssaParticle_IceGoblinHit1(this);
			break;

		case snCHAR_SOUND_COLDEYE:
			ParkParticle_ColdEyeSkill(this);
			break;

		case snCHAR_SOUND_CHAOSCARA:
			if ( MotionInfo->KeyCode=='J' ) {
				if (chrAttackTarget) {
					ParkAssaParticle_ChaosKara2(chrAttackTarget);
				}
				LastSkillParam = 0;
			}
			else {
				ParkAssaParticle_ChaosKaraSkill_Monster(this);
				//몬스터 스킬 이펙트 실현 ( 이펙트 실현 )
				SkillPlay_Monster_Effect( this , SKILL_PLAY_CHAOSCARA_VAMP, 220 );
				LastSkillParam = 1;
			}
			break;

		case snCHAR_SOUND_DEATHKNIGHT:			//발렌토
			ParkValentSwordShow( this, 70 );
			break;

		case snCHAR_SOUND_DEVIL_BIRD:
			DevilBird_Skill(this);
			break;

		case snCHAR_SOUND_KELVEZU:
			if ( MotionInfo->KeyCode=='I' ) {
				ParkKelvezuWingShow( this , 200 );
			}
			else {
				ParkKelvezuTaleShow( this , 90 );
				ParkKelvezuSkill2( this );
			}
			break;

		case snCHAR_SOUND_NPC_SKILLMASTER:
			switch( MotionInfo->KeyCode ) {
				case 'A':	//스파크
					break;
				case 'B':	//본 크래쉬
				case 'N':	//체인 랜스
				case 'Y':	//디스트로이어
					AssaMotionBlur( this , "Bip01 R Hand","bip01 weapon", 80 );
					break;
				case 'M':	//골든팰컨
					break;

				case 'L':	//차징스트라이크
					SkillLancelotChargingStrike( this );
					break;

			}
			break;

		case snCHAR_SOUND_NPC_MAGICMASTER:
			SkillPlaySound( SKILL_SOUND_SKILL_CASTING_MAGICIAN	 , pX,pY,pZ );		//마법 소리
			pos.x = pX;	pos.y = pY;	pos.z = pZ;
			sinEffect_StartMagic( &pos , 2 );			//마법진 이펙트
			break;


		case snCHAR_SOUND_CASTLE_SOLDER_A:
			AssaMotionBlur( this , "Bip01_w","Bip01 Effect", 80 );
			break;
		case snCHAR_SOUND_CASTLE_SOLDER_B:
			AssaMotionBlur( this , "Bip01 waraxe","Bip01 Effect", 80 );
			break;
		case snCHAR_SOUND_CASTLE_SOLDER_C:
			AssaMotionBlur( this , "Bip01 Sword","Bip01 Effect", 80 );
			break;
		case snCHAR_SOUND_REVIVED_KNIGHT:	// pluto 신규필드 몬스터 되살아난 나이트
			if( chrAttackTarget )
			{
				if( MotionInfo->KeyCode == 'H')
				{
					//피어싱
					//sinSkillEffect_Piercing( this );
					SkillPlaySound( SKILL_SOUND_SKILL_PIERCING			 , pX,pY,pZ );
				}
			}
			break;
		case snCHAR_SOUND_REVIVED_PRIESTESS:	// pluto 신규필드 몬스터 되살아난 프리스티스
			SkillPlaySound( SKILL_SOUND_SKILL_CASTING_MAGICIAN	 , pX,pY,pZ );		//마법 소리
			pos.x = pX;	pos.y = pY;	pos.z = pZ;
			sinEffect_StartMagic( &pos , 2 );			//마법진 이펙트
			break;
		case snCHAR_SOUND_REVIVED_PIKEMAN:		// pluto 신규필드 몬스터 되살아난 파이크맨
			if( chrAttackTarget )
			{
				switch( MotionInfo->KeyCode )
				{
					case 'Z':	//체인 랜스
						AssaMotionBlur( this , "Bip01 R Hand","bip01 wea", 80 );
						break;
				}
			}
			break;
	}

	return TRUE;
}

//몬스터 스킬 이벤트
int smCHAR::EventSkill_Monster()
{
	POINT3D	pos,pos2;
	RECT rect;
	int x,y,z;

	switch( smCharInfo.dwCharSoundCode ) {
		case snCHAR_SOUND_NAZ:

			break;

		case snCHAR_SOUND_HEADCUTTER:
			GetMoveLocation( 0 , 0, 60*fONE , 0, Angle.y , 0 );
			StartEffectMonster(pX+GeoResult_X,pY+GeoResult_Y,pZ+GeoResult_Z, 0, Angle.y, 0, MONSTER_HEADER_CUTTER_HIT1);
			break;

		case snCHAR_SOUND_TEMPLEGUARD:
		case snCHAR_SOUND_DMACHINE:
		case snCHAR_SOUND_TURTLE_CANNON:
			//디머신
			if (chrAttackTarget) {
				GetMoveLocation( 0 , 40*fONE, 0 , 0, Angle.y , 0 );
				StartEffectMonsterDest(pX+GeoResult_X, pY+GeoResult_Y,pZ+GeoResult_Z, 
					chrAttackTarget->pX , chrAttackTarget->pY+24*fONE , chrAttackTarget->pZ ,
					MONSTER_DMACHINE_MISSILE2);
			}
			break;
		case snCHAR_SOUND_REVIVED_KNIGHT:	// pluto 신규필드 몬스터 되살아난 나이트
			switch( MotionInfo->KeyCode )
			{
				case 'H':		// 피어싱
					if( chrAttackTarget )
					{
						sinSkillEffect_Piercing_Attack( this );
					}
					break;
				case 'Z':		// 그랜드크로스
					if ( chrAttackTarget && GetAttackPoint( &x,&y,&z )==TRUE )
					{
						pos.x = x;
						pos.y = y;
						pos.z = z;
						if ( MotionEvent==1 )
						{
							AssaParticle_GrandCrossHorz( &pos, Angle.y );
							SkillPlaySound( SKILL_SOUND_SKILL_SWING2	 , pX,pY,pZ );
						}
						else
						{
							AssaParticle_GrandCrossVert( &pos, Angle.y );
							SkillPlaySound( SKILL_SOUND_SKILL_GRAND_CROSS	 , pX,pY,pZ );
						}
					}
					else
					{
						//미스 소리
						//무기 소리 직접 연주
						PlayWaponSoundDirect( pX,pY,pZ , 13 );
					}
					break;

			}
			break;

		case snCHAR_SOUND_OMICRON:
			//오미크론
			if (chrAttackTarget) {
				GetMoveLocation( 0 , 20*fONE, 20*fONE , 0, Angle.y , 0 );
				StartEffectMonster(pX+GeoResult_X, pY+GeoResult_Y,pZ+GeoResult_Z, 
					0,Angle.y , 0, MONSTER_OMICRON_HIT1);
			}
			break;

		case snCHAR_SOUND_RATOO:
			//라투
			if (chrAttackTarget) {
				AssaParticle_RatooHit1( this );
			}
			break;

		case snCHAR_SOUND_MOKOVA:			//모코바
		case snCHAR_SOUND_STYGIANLORD:
			//스티지언로드
			if (chrAttackTarget) {
				StartEffectMonster(pX, pY,pZ, MONSTER_STYGIANLORD_MAGIC1);
			}
			break;

		case snCHAR_SOUND_SHADOW:
			//쉐도우 스킬
			if (chrAttackTarget) {
				GetMoveLocation( 0 , 140*fONE, 32*fONE , 0, Angle.y , 0 );
				StartEffectMonsterDest(pX+GeoResult_X, pY+GeoResult_Y,pZ+GeoResult_Z, 
					chrAttackTarget->pX , chrAttackTarget->pY+24*fONE , chrAttackTarget->pZ ,
					Angle.x , Angle.y , Angle.z , MONSTER_SHADOW_SHOT1 );
			}
			break;

		case snCHAR_SOUND_SUCCUBUS:
			//써큐버스
			if (chrAttackTarget ) {
			
				if ( smCharInfo.Level<=75 ) {	//휘치
					StartEffectMonster( pX,pY,pZ, MONSTER_SERQBUS_MAGIC1);
					StartEffect_Serqbus_Skill1( chrAttackTarget , SUCCUBUS_CURSE_TIME );
					StartEffectMonster(chrAttackTarget->pX, chrAttackTarget->pY+chrAttackTarget->PatHeight, chrAttackTarget->pZ, MONSTER_SERQBUS_MAGIC_ATTACK1);

					if ( chrAttackTarget==lpCurPlayer ) {		//흡수 약화 출력
						if ( !sDisplayState[0].Absorb )
							sDisplayState[0].Absorb = -GetRandomPos( 2,6 );
						sDisplayState[0].MaxTime = dwPlayTime+SUCCUBUS_CURSE_TIME*1000;
					}

				}
				else if ( smCharInfo.Level<80 ) {	//서큐버스
					if ( chrAttackTarget->smCharInfo.State!=smCHAR_STATE_USER ) {
						StartEffectMonster( pX,pY,pZ, MONSTER_SERQBUS_MAGIC2 );
						StartEffectMonster(chrAttackTarget->pX, chrAttackTarget->pY+chrAttackTarget->PatHeight, chrAttackTarget->pZ, MONSTER_SERQBUS_MAGIC_ATTACK2);
					}
				}
				else if ( smCharInfo.Level<90 ){	//인큐버스
					StartEffectMonster( pX,pY,pZ, MONSTER_SERQBUS_MAGIC3);
					StartEffect_Serqbus_Skill3( chrAttackTarget , SUCCUBUS_CURSE_TIME );
					StartEffectMonster(chrAttackTarget->pX, chrAttackTarget->pY+chrAttackTarget->PatHeight, chrAttackTarget->pZ, MONSTER_SERQBUS_MAGIC_ATTACK3);

					if ( chrAttackTarget==lpCurPlayer ) {		//공격 약화 출력
						if ( !sDisplayState[1].PercentDamage[0] ) {
							sDisplayState[1].PercentDamage[0] = -GetRandomPos( 10,15 );
							sDisplayState[1].PercentDamage[1] = sDisplayState[1].PercentDamage[0];
						}
						sDisplayState[1].MaxTime = dwPlayTime+SUCCUBUS_CURSE_TIME*1000;
					}
				}
				else {	//인큐버스 서머너
					ParkIncubusSummonerSkill( this );
				}
			}
			break;


		case snCHAR_SOUND_EVILPLANT:
			//이블 플랜트 메이플
			if ( smCharInfo.Level>=60 ) {
				//이블 플랜트
				AssaParticle_EvilPlantShot( this );
			}
			else {
				//이블 메이플
				AssaParticle_EvilMapleShot( this );
			}
			break;

		case snCHAR_SOUND_STONEGOLEM:
			//스톤 고렘
			AssaParticle_StoneGolemShot( this );
			break;


		case snCHAR_SOUND_SOLIDSNAIL:
			//나이트 메어
			if ( chrAttackTarget ) {
				pos.x = chrAttackTarget->pX;
				pos.y = chrAttackTarget->pY;
				pos.z = chrAttackTarget->pZ;

				AssaParticle_SnailShot( this , &pos );
			}

		case snCHAR_SOUND_SCORPION:
			//이블 스네일
			if ( chrAttackTarget && smCharInfo.Level==50 ) {
				pos.x = chrAttackTarget->pX;
				pos.y = chrAttackTarget->pY;
				pos.z = chrAttackTarget->pZ;

				AssaParticle_SnailShot( this , &pos );
			}
			break;

		case snCHAR_SOUND_THORNCRAWLER:
			//숀 크로울러
			if ( chrAttackTarget ) {
				pos.x = chrAttackTarget->pX;
				pos.y = chrAttackTarget->pY;
				pos.z = chrAttackTarget->pZ;

				AssaParticle_ThornCrawlerShot( this , &pos );
			}
			break;

		case snCHAR_SOUND_NIGHTMARE:
		case snCHAR_SOUND_HEST: // 박재원 - 저주받은 신전 3층(신규필드 몬스터) - 헤스트
			//나이트 메어
			AssaParticle_NightMare( this );
			break;

		case snCHAR_SOUND_GUARDIAN_SAINT:
			//가디안 - 세인트
			AssaParticleClanMonsterHit1( this );
			/*
			if ( smConfig.DebugMode ) {
				AddChatBuff( "Guardian Power" , 0 );
			}
			*/
			break;

		case snCHAR_SOUND_HYPERMACHINE:
			//하이퍼머신
			if ( chrAttackTarget ) {
				pos.x = chrAttackTarget->pX;
				pos.y = chrAttackTarget->pY;
				pos.z = chrAttackTarget->pZ;
				AssaParticle_MonsterIronHyperSkill1( this , &pos );
			}
			break;

		case snCHAR_SOUND_RUNICGUARDIAN:		//루닉가디안
			if ( chrAttackTarget ) {
				AssaParticle_MonsterIronRunicGuardian( this , chrAttackTarget );
			}
			break;

		case snCHAR_SOUND_BABEL:				//바벨 (루닉가디안 보스)
			if ( chrAttackTarget ) {
				AssaParticle_MonsterRunicGrardianBossSkill( this );
			}
			break;


		case snCHAR_SOUND_MOUNTAIN:			//마운틴
			AssaParticle_MonsterIronMountainSkill1( this  );
			break;

		case snCHAR_SOUND_TOWERGOLEM:		//타워고렘
			AssaParticle_MonsterIronTowerGolemSkill1( this  );
			break;

		case snCHAR_SOUND_SADNESS:			//새드니스
			AssaParticle_MonsterIronSadNessSkill1( this );
			break;

		case snCHAR_SOUND_IRONFIST:
			AssaParticle_MonsterIronFist(this);
			break;

		case snCHAR_SOUND_MORGON:
		case snCHAR_SOUND_GORGON: // pluto 추가 몬스터 고르곤
			AssaParticle_MonsterIronMorgonHit(this);
			break;

		///////////////// 아이스 몬스터 ///////////////

		case snCHAR_SOUND_ICEGOLEM:
			ParkAssaParticle_IceGolemShot( this );
			break;
		case snCHAR_SOUND_FROST:
			ParkAssaParticle_AcientDia( this );
			break;
		case snCHAR_SOUND_FROZEN:
			if ( chrAttackTarget ) {
				ParkAssaParticle_MistIceBall(this , chrAttackTarget);
			}
			break;

		case snCHAR_SOUND_MYSTIC:
			if ( chrAttackTarget ) {
				StartEffectMonster( pX,pY,pZ, MONSTER_SERQBUS_MAGIC2 );
				StartEffect_Serqbus_Skill1( chrAttackTarget , SUCCUBUS_CURSE_TIME );
				StartEffectMonster(chrAttackTarget->pX, chrAttackTarget->pY+chrAttackTarget->PatHeight, chrAttackTarget->pZ, MONSTER_SERQBUS_MAGIC_ATTACK1);

				if ( chrAttackTarget==lpCurPlayer ) {		//흡수 약화 출력
					if ( !sDisplayState[0].Absorb )
						sDisplayState[0].Absorb = -GetRandomPos( 10,16 );
					sDisplayState[0].MaxTime = dwPlayTime+SUCCUBUS_CURSE_TIME*1000;
				}
			}
			break;

		case snCHAR_SOUND_DEATHKNIGHT:			//발렌토
			if ( MotionInfo->KeyCode=='N' ) 
				ParkAssaParticle_Valento1( this );
			break;


		////////////////// 그리디호수 몬스터 /////////////////////
		case snCHAR_SOUND_SPIDER:
			ParkAssaParticle_SpiderTrooperHit1(this);
			break;

		case snCHAR_SOUND_STINGRAY:
			ParkAssaParticle_StingRay(this);
			break;

		case snCHAR_SOUND_M_LORD:
			SkillPlay_MummyLord_Effect( this , 200 );
			break;

		case snCHAR_SOUND_GOBLINSHAMAN:	// 박재원 - 저주받은 신전 3층(신규필드 몬스터) - 고블린샤먼
			SkillPlay_MummyLord_Effect( this , 200 );
			break;


		///////////////////////////////////////////////

		case snCHAR_SOUND_BLIZZARD_GIANT:
			if ( chrAttackTarget ) {
				ParkBlizzardGiantSkillToTarget( chrAttackTarget );
			}
			break;
		case snCHAR_SOUND_DEVIL_BIRD:
			if ( chrAttackTarget ) {
				ParkDevilBirdSkillToTarget( chrAttackTarget );
			}
			break;

		case snCHAR_SOUND_KELVEZU:
			if ( chrAttackTarget ) {
				if ( MotionInfo->KeyCode=='I' ) {
					ParkKelvezuSkillHit( chrAttackTarget );
					ParkKelvezuSkill1( this );
				}
				else {
					ParkKelvezuSkillHit( chrAttackTarget );
				}
			}
			break;

		///////////////////////////////////////////////

		case snCHAR_SOUND_DARKPHALANX:
			ParkAssaParticle_DarkPhalanx( this );
			break;
		case snCHAR_SOUND_FIREWORM:
			if ( chrAttackTarget ) {
				ParkAssaParticle_FireWorm( chrAttackTarget , 1 );
			}
			break;

		case snCHAR_SOUND_CHIMERA:
			if ( chrAttackTarget ) {
				ParkAssaParticle_ChimeraSkill( chrAttackTarget );
			}
			break;

		case snCHAR_SOUND_BLOODYKNIGHT:
			ParkAssaParticle_BloodySkill( this );
			break;
		case snCHAR_SOUND_HELLHOUND:
			ParkAssaParticle_HellHoundSkillAttack( this );
			break;
		case snCHAR_SOUND_DARKGUARD:
			if ( chrAttackTarget ) {
				ParkAssaParticle_DarkGuardSkill( this , chrAttackTarget );
			}
			break;
		case snCHAR_SOUND_DARKMAGE:
			if ( chrAttackTarget ) {
				ParkAssaParticle_DarkMageSkill( chrAttackTarget );
			}
			break;

		case snCHAR_SOUND_SETO:
			SkillArchMageFlameWave(this);
			break;

		case snCHAR_SOUND_KINGSPIDER:
			//헐크 꺼랑 동일
			if (chrAttackTarget) {
				GetMoveLocation( 0 , 0, 68*fONE , 0, Angle.y , 0 );
				StartEffectMonster( pX+GeoResult_X, pY+22*fONE,pZ+GeoResult_Z, MONSTER_HULK_HIT1);
			}
			break;
		///////////////////////////////////////////////


		case snCHAR_SOUND_S_METALGOLEM:		//소환몹 메탈고렘
			if (chrAttackTarget) {
				if ( !smCharInfo.wPlayClass[1] )
					GetMoveLocation( 0 , 0, 54*fONE , 0, Angle.y , 0 );
				else
					GetMoveLocation( 0 , 0, 54*smCharInfo.wPlayClass[1] , 0, Angle.y , 0 );

				StartEffectMonster( pX+GeoResult_X, pY+48*fONE,pZ+GeoResult_Z, MONSTER_HULK_HIT1);
			}
			break;

		case snCHAR_SOUND_S_WOLVERLIN:		//소환몹 울버린
			if (chrAttackTarget) {
				if ( !smCharInfo.wPlayClass[1] )
					GetMoveLocation( 0,0,50*fONE , 0,Angle.y,0 );
				else
					GetMoveLocation( 0,0,50*smCharInfo.wPlayClass[1] , 0, Angle.y , 0 );

				x = pX+GeoResult_X;
				z = pZ+GeoResult_Z;
				y = pY+18*fONE;
				StartEffectMonster(x,y,z, MONSTER_PIGON_PARTICLE1);
				StartEffectMonster(x,y,z, MONSTER_PIGON_PARTICLE1);
				StartEffectMonster(x,y,z, MONSTER_PIGON_PARTICLE1);
				StartEffectMonster(x,y,z, MONSTER_PIGON_PARTICLE1);
			}
			break;

		case snCHAR_SOUND_NPC_SKILLMASTER:
			if ( chrAttackTarget ) {
				switch( MotionInfo->KeyCode ) {
					case 'A':	//스파크
						StartSkillDest( pX,pY+24*fONE,pZ , chrAttackTarget->pX,chrAttackTarget->pY+24*fONE,chrAttackTarget->pZ, SKILL_SPARK , 8 );
						SkillPlaySound( SKILL_SOUND_SKILL_SPARK2	 , pX,pY,pZ);
						break;
					case 'B':	//본 크래쉬
						if ( GetAttackPoint( &x,&y,&z )==TRUE ) {
							StartSkill( x,y,z, 0,0,0, SKILL_TRIPLE_IMPACT );
							SetDynLight( x,y,z , 100, 0,0,0, 300 );
							SkillPlaySound( SKILL_SOUND_SKILL_AVANGING_CRASH , pX,pY,pZ );
						}
						break;
					case 'N':	//체인 랜스
						if ( GetAttackPoint( &x,&y,&z )==TRUE ) {
							pos.x = x;
							pos.y = y;
							pos.z = z;
							AssaParticle_ChainLance( &pos );
						}
						break;
					case 'M':	//골든팰컨
						GetMoveLocation( 0 , 24*fONE, 24*fONE , 0, Angle.y , 0 );
						pos.x = pX+GeoResult_X;
						pos.y = pY+GeoResult_Y;
						pos.z = pZ+GeoResult_Z;
						pos2.x = chrAttackTarget->pX;
						pos2.y = chrAttackTarget->pY+24*fONE;
						pos2.z = chrAttackTarget->pZ;
						SkillSagittarionPhoneixShot( this, &pos , &pos2 );
						SkillPlaySound( SKILL_SOUND_SKILL_PHOENIX_SHOT	 , pX,pY,pZ );
						break;
					case 'Y':	//디스트로이어
						if ( GetAttackPoint( &pos.x,&pos.y,&pos.z )==TRUE ) {
							SkillWarriorDestroyerHit(&pos);                       //타격
							if ( MotionEvent<3 )
								SkillPlaySound( SKILL_SOUND_SKILL_AVANGING_CRASH , pX,pY,pZ );
							else
								SkillPlaySound( SKILL_SOUND_SKILL_EXPANSION2	 , pX,pY,pZ );
						}
						break;
					case 'L':	//차징스트라이크
						if ( GetAttackPoint( &x,&y,&z )==TRUE ) {
							StartSkill( x,y,z, 0,0,0, SKILL_TRIPLE_IMPACT );
							SetDynLight( x,y,z , 100, 0,0,0, 400 );
							switch( MotionEvent ) {
								case 1:
									SkillPlaySound( SKILL_SOUND_SKILL_CHAIN_LANCE2	 , pX,pY,pZ );
									break;
								case 2:
									SkillPlaySound( SKILL_SOUND_SKILL_AVANGING_CRASH , pX,pY,pZ );
									break;
								case 3:
								case 4:
									SkillPlaySound( SKILL_SOUND_SKILL_CHARGING_STRIKE	 , pX,pY,pZ );
									break;
							}
						}
						break;
				}
			}
			break;
		case snCHAR_SOUND_NPC_MAGICMASTER:
			if ( chrAttackTarget ) {
				switch( MotionInfo->KeyCode ) {
					case 'R':	//에너지 쉴드
						AssaParticle_EnergyShield( this , 5 );
						SkillPlaySound( SKILL_SOUND_SKILL_ENERGY_SHIELD	 , pX,pY,pZ );
						break;
					case 'E':	//멀티 스파크
						sinEffect_MultiSpark( this , chrAttackTarget , 5 );
						SkillPlaySound( SKILL_SOUND_SKILL_MULTISPARK	 , pX,pY,pZ );
						break;
					case 'U':	//데스 레이
						GetMoveLocation( 0 , 24*fONE, 24*fONE , 0, Angle.y , 0 );
						pos.x = pX+GeoResult_X;
						pos.y = pY+GeoResult_Y;
						pos.z = pZ+GeoResult_Z;
						pos2.x = chrAttackTarget->pX;
						pos2.y = chrAttackTarget->pY+32*fONE;
						pos2.z = chrAttackTarget->pZ;

						AssaParticle_DeadLay( &pos, &pos2 );
						SkillPlaySound( SKILL_SOUND_SKILL_DEATHRAY	 , pX,pY,pZ );
						break;
					case 'I':	//파이어 볼
						GetMoveLocation( 0 , 24*fONE, 24*fONE , 0, Angle.y , 0 );
						StartEffectMonsterDest( pX+GeoResult_X,pY+GeoResult_Y,pZ+GeoResult_Z,
							chrAttackTarget->pX , chrAttackTarget->pY+20*fONE , chrAttackTarget->pZ ,
							MONSTER_PIGON_SHOT1 );

						if ( ((dwPlayTime>>4)&1)==0 ) 
							SkillPlaySound( SKILL_SOUND_SKILL_FIREBALL1	 , pX,pY,pZ );
						else
							SkillPlaySound( SKILL_SOUND_SKILL_FIREBALL2	 , pX,pY,pZ );
						break;

					case 'H':	//글래셜 스파이크
						Angle.y = GetRadian2D( pX ,pZ, chrAttackTarget->pX, chrAttackTarget->pZ );
						SkillCelestialGlacialSpike( this );

						GetMoveLocation( 0 , 0, 64*fONE , 0, Angle.y , 0 );
 						SetDynLight( pX+GeoResult_X,pY,pZ+GeoResult_Z, 0, 0,100,0, 700 );

						if ( rand()%2 ) 
							SkillPlaySound( SKILL_SOUND_SKILL_GLACIAL_SPIKE1	 , pX,pY,pZ );
						else
							SkillPlaySound( SKILL_SOUND_SKILL_GLACIAL_SPIKE2	 , pX,pY,pZ );

						if ( chrAttackTarget==lpCurPlayer ) {
							lpCurPlayer->PlaySlowCount = 90;			//1초동안 느리게 움직임
							lpCurPlayer->AttackIce -= 160;
						}

						break;

					case 'G':	//미티어
						pos.x = chrAttackTarget->pX;
						pos.y = chrAttackTarget->pY;
						pos.z = chrAttackTarget->pZ;
						SkillArchMageMeteo( &pos );
						SkillPlaySound( SKILL_SOUND_SKILL_METEO1 , pX,pY,pZ );
						break;

				}
			}
			break;

		case snCHAR_SOUND_CASTLE_SOLDER_A:
		case snCHAR_SOUND_CASTLE_SOLDER_B:
		case snCHAR_SOUND_CASTLE_SOLDER_C:
			if ( chrAttackTarget ) {
				if ( GetAttackPoint( &x,&y,&z )==TRUE ) {
					StartSkill( x,y,z, 0,0,0, SKILL_TRIPLE_IMPACT );
					SetDynLight( x,y,z , 100, 0,0,0, 300 );
					SkillPlaySound( SKILL_SOUND_SKILL_AVANGING_CRASH , pX,pY,pZ );
				}
			}
			break;

		case snCHAR_SOUND_CASTLE_CRYSTAL_R:
			AssaParticle_CastleRed( this );
			break;

		case snCHAR_SOUND_CASTLE_CRYSTAL_G:
			AssaParticle_CastleGreen( this );
			break;

		case snCHAR_SOUND_CASTLE_CRYSTAL_B:
			AssaParticle_CastleBlue( this );
			break;
		case snCHAR_SOUND_REVIVED_MAGICIAN:		// pluto 신규필드 몬스터 되살아난 매지션
			if( chrAttackTarget )
			{
				switch( MotionInfo->KeyCode )
				{
					case 'O':	//데스 레이
						GetMoveLocation( 0 , 24*fONE, 24*fONE , 0, Angle.y , 0 );
						pos.x = pX+GeoResult_X;
						pos.y = pY+GeoResult_Y;
						pos.z = pZ+GeoResult_Z;
						pos2.x = chrAttackTarget->pX;
						pos2.y = chrAttackTarget->pY+32*fONE;
						pos2.z = chrAttackTarget->pZ;

						AssaParticle_DeadLay( &pos, &pos2 );
						SkillPlaySound( SKILL_SOUND_SKILL_DEATHRAY	 , pX,pY,pZ );
						break;
					case 'Q':	// 플레임 웨이브
						SkillArchMageFlameWave(this);

						if ( rand()%2 )
							SkillPlaySound( SKILL_SOUND_SKILL_FLAME_WAVE1 , pX,pY,pZ );
						else
							SkillPlaySound( SKILL_SOUND_SKILL_FLAME_WAVE2 , pX,pY,pZ );
						break;
					case 'Z':	//미티어
						pos.x = chrAttackTarget->pX;
						pos.y = chrAttackTarget->pY;
						pos.z = chrAttackTarget->pZ;
						SkillArchMageMeteo( &pos );
						SkillPlaySound( SKILL_SOUND_SKILL_METEO1 , pX,pY,pZ );
						break;
				}
			}
			break;
		case snCHAR_SOUND_REVIVED_ARCHER:		// pluto 신규필드 몬스터 되살아난 아처
			if ( chrAttackTarget )
			{
				switch( MotionInfo->KeyCode )
				{
					case 'O':            //윈드 애로우
						GetAttackPoint( &x,&y,&z );
						StartSkillDest( x,y,z , chrAttackTarget->pX,chrAttackTarget->pY+(24*fONE),chrAttackTarget->pZ , SKILL_WIND_ARROW );
						SkillPlaySound( SKILL_SOUND_WIND_ARROW , pX,pY,pZ );			//스킬 효과음
						break;
					case 'Z':			// 피닉스 샷
						int cnt = frame-MotionInfo->StartFrame*80;
						if ( MotionInfo->EventFrame[1]<=(DWORD)cnt )
						{
							GetMoveLocation( 0 , 24*fONE, 24*fONE , 0, Angle.y , 0 );
							pos.x = pX+GeoResult_X;
							pos.y = pY+GeoResult_Y;
							pos.z = pZ+GeoResult_Z;
							pos2.x = chrAttackTarget->pX;
							pos2.y = chrAttackTarget->pY+24*fONE;
							pos2.z = chrAttackTarget->pZ;
							SkillSagittarionPhoneixShot( this, &pos , &pos2 );
							SkillPlaySound( SKILL_SOUND_SKILL_PHOENIX_SHOT	 , pX,pY,pZ );
						}
						break;
				}
			}
			break;
		case snCHAR_SOUND_REVIVED_ATALANTA:		// pluto 신규필드 몬스터 되살아난 아탈란타
			switch( MotionInfo->KeyCode )
			{
				case 'O':	// 스톰 재블린
					if( chrAttackTarget && GetAttackPoint( &x,&y,&z )==TRUE )
					{
						pos.x = x;
						pos.y = y;
						pos.z = z;
						pos2.x = chrAttackTarget->pX;
						pos2.y = chrAttackTarget->pY+28*fONE;
						pos2.z = chrAttackTarget->pZ;

						AssaParticle_StormJavelin( &pos, &pos2 );

						rect.left = -30;
						rect.right = 30;
						rect.top = 0;
						rect.bottom=240;
						rect.bottom+=20;
						rect.bottom += 20;

						for(int cnt=0;cnt<dmSelected_CharCnt;cnt++)
						{
							if ( lpSelected_Char[cnt] )
							{
								AssaParticle_StormJavelinHit( lpSelected_Char[cnt] , this , 0.5f );
							}
						}
						switch( rand()%2 )
						{
						case 0:
							SkillPlaySound( SKILL_SOUND_SKILL_STORM_JAVELIN1	 , pX,pY,pZ );
							break;
						case 1:
							SkillPlaySound( SKILL_SOUND_SKILL_STORM_JAVELIN2	 , pX,pY,pZ );
							break;
						}
					}
					break;
				case 'Z':	// 벤젼스
					if( chrAttackTarget )
					{
						GetMoveLocation( 0 , 24*fONE, 24*fONE , 0, Angle.y , 0 );
						pos.x = pX+GeoResult_X;
						pos.y = pY+GeoResult_Y;
						pos.z = pZ+GeoResult_Z;
						pos2.x = chrAttackTarget->pX;
						pos2.y = chrAttackTarget->pY+40*fONE;
						pos2.z = chrAttackTarget->pZ;
						//if ( AttackCritcal>=0 )
							SkillVengeance( &pos , &pos2 );

						//if ( this==lpCurPlayer && MotionEvent==1 )
						//{
						//	cnt = dm_SendTransDamage( chrAttackTarget , 0,0 , 0, 0, AttackSkil , TRUE );
						//	if ( !cnt )
						//		AttackCritcal = -1;				//빗나감
						//}
						if ( MotionEvent==1 )
							SkillPlaySound( SKILL_SOUND_SKILL_VENGEANCE1	 , pX,pY,pZ );
						else
							SkillPlaySound( SKILL_SOUND_SKILL_VENGEANCE2	 , pX,pY,pZ );
					}
					break;
			}
			break;
		case snCHAR_SOUND_REVIVED_FIGTHER:		// pluto 신규필드 몬스터 되살아난 파이터
			if( chrAttackTarget )
			{
				switch( MotionInfo->KeyCode )
				{
					case 'G':
						//어벤징 크래쉬
						AssaParticle_AvangingCrash( this );
						SkillPlaySound( SKILL_SOUND_SKILL_AVANGING_CRASH , pX,pY,pZ );
						break;
					case 'Z':	// 디스트로이어
						if ( GetAttackPoint( &pos.x,&pos.y,&pos.z )==TRUE )
						{
							SkillWarriorDestroyerHit(&pos);                       //타격
							if ( MotionEvent<3 )
								SkillPlaySound( SKILL_SOUND_SKILL_AVANGING_CRASH , pX,pY,pZ );
							else
								SkillPlaySound( SKILL_SOUND_SKILL_EXPANSION2	 , pX,pY,pZ );
						}
						break;
				}
			}
			break;
		case snCHAR_SOUND_REVIVED_MECANICIAN:	// pluto 신규필드 몬스터 되살아난 메카니션
			if( chrAttackTarget )
			{
				switch( MotionInfo->KeyCode )
				{
					case 'O':	// 스파크
						StartSkillDest( pX,pY+24*fONE,pZ , chrAttackTarget->pX,chrAttackTarget->pY+24*fONE,chrAttackTarget->pZ, SKILL_SPARK , 8 );
						SkillPlaySound( SKILL_SOUND_SKILL_SPARK2	 , pX,pY,pZ);
						break;
					case 'Z':	// 그랜드 스매쉬
						if ( GetAttackPoint( &x,&y,&z )==TRUE )
						{
							// 도구 공격
							StartSkill( x,y,z, 0,Angle.y,0, SKILL_GREAT_SMASH );
							SkillPlaySound( SKILL_SOUND_SKILL_SWING1	 , pX,pY,pZ );
						}
						break;
				}
			}
			break;
		case snCHAR_SOUND_REVIVED_PIKEMAN:		// pluto 신규필드 몬스터 되살아난 파이크맨
			if( chrAttackTarget )
			{
				switch( MotionInfo->KeyCode )
				{
					case 'G':	// 점핑 크래쉬
						if( GetAttackPoint( &x,&y,&z )==TRUE )
						{
							StartEffect( x,y,z , EFFECT_CRITICAL_HIT1 );
							SetDynLight( x,y,z , 100, 0,0,0, 400 );
							SkillPlaySound( SKILL_SOUND_JUMPING_CRASH2 , pX,pY,pZ );			//스킬 효과음
						}
						break;
					case 'Z':	// 체인 랜스
						if ( GetAttackPoint( &x,&y,&z )==TRUE )
						{
							pos.x = x;
							pos.y = y;
							pos.z = z;
							AssaParticle_ChainLance( &pos );
							switch(MotionEvent)
							{
								case 1:
									SkillPlaySound( SKILL_SOUND_SKILL_CHAIN_LANCE1 , pX,pY,pZ );			//스킬 효과음
									break;
								case 2:
									SkillPlaySound( SKILL_SOUND_SKILL_CHAIN_LANCE2 , pX,pY,pZ );			//스킬 효과음
									break;
								case 3:
									SkillPlaySound( SKILL_SOUND_SKILL_CHAIN_LANCE3 , pX,pY,pZ );			//스킬 효과음
									break;
							}
						}
						break;
				}
			}
			break;
		case snCHAR_SOUND_REVIVED_PRIESTESS:	// pluto 신규필드 몬스터 되살아난 프리스티스
			if( chrAttackTarget )
			{
				switch( MotionInfo->KeyCode )
				{
					case 'O':	// 멀티 스파크
						sinEffect_MultiSpark( this , chrAttackTarget , 5 );
						SkillPlaySound( SKILL_SOUND_SKILL_MULTISPARK	 , pX,pY,pZ );
						break;
					case 'H':	// 비거볼
						AssaParticle_VigorBall( this , chrAttackTarget );
						switch( rand()%2 )
						{
							case 0:
								SkillPlaySound( SKILL_SOUND_SKILL_VIGOR_BALL1	 , pX,pY,pZ );
								break;
							case 1:
								SkillPlaySound( SKILL_SOUND_SKILL_VIGOR_BALL2	 , pX,pY,pZ );
								break;
						}
						break;
					case 'Z':	// 글래셜 스파이크
						Angle.y = GetRadian2D( pX ,pZ, chrAttackTarget->pX, chrAttackTarget->pZ );
						SkillCelestialGlacialSpike( this );
						GetMoveLocation( 0 , 0, 64*fONE , 0, Angle.y , 0 );
						SetDynLight( pX+GeoResult_X,pY,pZ+GeoResult_Z, 0, 0,100,0, 700 );
						if( rand()%2 ) 
							SkillPlaySound( SKILL_SOUND_SKILL_GLACIAL_SPIKE1	 , pX,pY,pZ );
						else
							SkillPlaySound( SKILL_SOUND_SKILL_GLACIAL_SPIKE2	 , pX,pY,pZ );

						if( chrAttackTarget==lpCurPlayer )
						{
							lpCurPlayer->PlaySlowCount = 90;			//1초동안 느리게 움직임
							lpCurPlayer->AttackIce -= 160;
						}
						break;
				}
			}
			break;
		case snCHAR_SOUND_HOBOGOLEM:	// pluto 추가 몬스터 호보고렘
            if (chrAttackTarget)
			{
				GetMoveLocation( 7000 , 30*fONE, 50*fONE , 0, Angle.y , 0 );
				StartEffectMonsterDest(pX+GeoResult_X, pY+GeoResult_Y,pZ+GeoResult_Z, 
					chrAttackTarget->pX , chrAttackTarget->pY+24*fONE , chrAttackTarget->pZ ,
					MONSTER_DMACHINE_MISSILE1);
			}
            break;

		////////////////// // 박재원 - 끝없는 탑 3층(신규필드 몬스터) /////////////////////
		case snCHAR_SOUND_SPRIN: // 스프린 : 스킬 공격
			if (chrAttackTarget && MotionInfo->KeyCode=='J')
			{
				ParkAssaParticle_HellHoundSkillAttack( this );
			}
			break;
		case snCHAR_SOUND_XETAN: // 제탄 : 스킬 공격
			if(chrAttackTarget && MotionInfo->KeyCode=='J') 
			{
				ParkAssaParticle_DarkMageSkill( chrAttackTarget );
			}
			break;
		case snCHAR_SOUND_RUCA: // 루카 : 스킬 공격
			if(chrAttackTarget && MotionInfo->KeyCode=='J') 
			{
				GetMoveLocation( 0 , 0, 64*fONE , 0, Angle.y , 0 );
				pos.x = pX+GeoResult_X;
				pos.y = pY+GeoResult_Y+32*fONE;
				pos.z = pZ+GeoResult_Z;
				AssaParticle_MonsterIronHit1( &pos );
			}
			break;
		case snCHAR_SOUND_NAZSENIOR: // 나즈 시니어 : 스킬 공격
			if (chrAttackTarget && MotionInfo->KeyCode=='k')
			{
				GetMoveLocation( 0 , 0, 100*fONE , 0, Angle.y , 0 );
				StartSkillDest( pX,pY,pZ, pX+GeoResult_X,pY,pZ+GeoResult_Z, SKILL_TORNADO2 , 1 );
			}
			break;
		case snCHAR_SOUND_UNDEADMAPLE: // 언데드 메이플 : 스킬 공격
			if (chrAttackTarget && MotionInfo->KeyCode=='J') 
			{
				AssaParticle_EvilPlantShot( this );
			}
			break;
			

		// 장별 - 수박몬스터 기술동작
		case snCHAR_SOUND_WATERMELON:	

			pos.x = pX;
			pos.y = pY;
			pos.z = pZ;
			pos2.x = chrAttackTarget->pX;
			pos2.y = chrAttackTarget->pY+32*fONE;
			pos2.z = chrAttackTarget->pZ;

			AssaParticle_IceShot( &pos, &pos2 );
		break;
		/////////////////////////////////////////////////////////////////////////////////////
	}

	PlayAttack();									//공격 적용
	CharPlaySound( this );

	return TRUE;
}

//스킬동작종료
int smCHAR::EndSkill()
{
	int	point;
	int	param;

	point = (AttackSkil>>8)&0xF;
	param = (AttackSkil>>12)&0xF;

	switch( AttackSkil&0xFF )
	{
	case SKILL_PLAY_DIASTROPHISM:
		if ( this==lpCurPlayer && MotionEvent<3 ) {
			//범위형 공격 범위 선정 (원)
			dm_SelectRange( pX, pY, pZ, Diastrophism_Area[point-1] , FALSE );

			//범위공격 설정후 전송
			dm_SendRangeDamage( pX, pY, pZ, 0,
				cInvenTory.InvenItem[ sInven[0].ItemIndex-1 ].sItemInfo.Damage[0] ,
				cInvenTory.InvenItem[ sInven[0].ItemIndex-1 ].sItemInfo.Damage[1] ,
				0, 0, AttackSkil );
		}
		break;
/*
	case SKILL_PLAY_SHADOW_MASTER:
		if ( lpCurPlayer==this && MotionEvent<Shadow_Master_ShadowNum[point-1] && chrAttackTarget ) {
			cnt = dm_SendTransDamage( chrAttackTarget , 0,0 , 0, 0, lpCurPlayer->AttackSkil , TRUE );
			if ( !cnt )	
				lpCurPlayer->AttackCritcal = -1;				//빗나감
			else
				lpCurPlayer->AttackCritcal = 0;
		}
		break;
*/
	case SKILL_PLAY_METEO:
		if ( chrAttackTarget && point ) {
			if ( this==lpCurPlayer && MotionEvent<4 ) {
				//범위형 공격 범위 선정 (원)
				dm_SelectRange( chrAttackTarget->pX, chrAttackTarget->pY, chrAttackTarget->pZ, Meteo_Area[point-1] , FALSE );
				//범위공격 설정후 전송
				dm_SendRangeDamage( chrAttackTarget->pX, chrAttackTarget->pY, chrAttackTarget->pZ,  0 , 0 , 0 ,0,0, lpCurPlayer->AttackSkil );
			}
		}
		break;
	}

	MotionEvent++;

	return TRUE;
}

//스킬이벤트
int smCHAR::EventSkill()
{
	int x,y,z;
	int	point;
	int	param;
	int	damage;
	int	temp;
	int cnt;
	POINT3D	Pos1,Pos2;
	RECT rect;
	smCHAR *lpChar;

	if ( smCharInfo.State==smCHAR_STATE_ENEMY && PkMode_CharState!=smCHAR_STATE_USER ) {
		return EventSkill_Monster();
	}

	point = (AttackSkil>>8)&0xF;
	param = (AttackSkil>>12)&0xF;


	switch( AttackSkil&0xFF )
	{
	case SKILL_PLAY_EXTREME_SHIELD:
		//익스트림 쉴드
		if ( HvLeftHand.PatTool ) {
			if ( GetToolBipPoint( &HvLeftHand , &x,&y,&z ) ) {
				StartSkill( x,y,z, 0,0,0, SKILL_EXTREME_SHIELD );
			}
		}
		break;

	case SKILL_PLAY_MECHANIC_BOMB:
		//메카닉붐
		if( chrAttackTarget ) {
			if ( point ) {
				damage = GetRandomPos( Mechanic_Bomb_Damage[point-1][0] , Mechanic_Bomb_Damage[point-1][1] )/2;
				StartSkillDest(pX,pY+30*fONE,pZ , chrAttackTarget->pX,chrAttackTarget->pY,chrAttackTarget->pZ, SKILL_MECHANIC_BOMB , point );
				SkillPlaySound( SKILL_SOUND_MECHANIC_BOMB1 , pX,pY,pZ );			//스킬 효과음

				if ( this==lpCurPlayer ) {
					/*
					Skil_RangeAttack( chrAttackTarget->pX, chrAttackTarget->pY, chrAttackTarget->pZ, (80*fONE)+(point*5*fONE) , damage , 0 );
					//상대 캐릭터에 공격 가함 
					QuickSendTransAttack( chrAttackTarget , damage );
					*/
					//범위형 공격 범위 선정 (원)
					dm_SelectRange( chrAttackTarget->pX, chrAttackTarget->pY, chrAttackTarget->pZ, 80+(point*5) , FALSE );
					//범위공격 설정후 전송
					dm_SendRangeDamage( chrAttackTarget->pX, chrAttackTarget->pY, chrAttackTarget->pZ, chrAttackTarget ,0, 0, 0, 0, lpCurPlayer->AttackSkil );
				}
			}
		}
		break;
	case SKILL_PLAY_PHYSICAL_ABSORB:
		//픽지클 옵져브
		break;

	case SKILL_PLAY_RAVING:
		//스킬 공격
		return FALSE;

	case SKILL_PLAY_IMPACT:
		//파이터 임팩트
		if ( this==lpCurPlayer && chrAttackTarget && MotionEvent<2 ) {
			if ( AttackCritcal>=0 ) {
				temp = smCharInfo.Attack_Rating;
				smCharInfo.Attack_Rating += Impact_Attack_Rating[point-1];
				//공격 전송
				//cnt = SendTransAttack( chrAttackTarget , smWsockServer , 0 , Impact_Damage[point-1] );
				cnt = dm_SendTransDamage( chrAttackTarget , 0,0 , 0, 0, lpCurPlayer->AttackSkil , TRUE );
				smCharInfo.Attack_Rating = temp;

				if ( cnt>0 ) {
					DamageExp( chrAttackTarget , cnt );					//데미지에 따른 경험치 상승
					if ( AttackExp ) {
						//AddExp( AttackExp );
						//SetTotalAddExp( AttackExp );			//경험치 증가 기록
						AttackExp=0;
					}
				}
			}
		}
		return FALSE;

	case SKILL_PLAY_PIKEWIND:
		//파이크 윈드
		if ( point ) {
			StartSkill( pX,pY,pZ, 0,0,0, SKILL_PIKE_WIND , point );
			SkillPlaySound( SKILL_SOUND_PIKE_WIND , pX,pY,pZ );			//스킬 효과음
		}
		break;

	case SKILL_PLAY_CRITICAL_HIT:
		//크리티컬 힛
		return FALSE;

	case SKILL_PLAY_JUMPING_CRASH:
		//스킬 공격
		if ( AttackSkil>=0 )
			SkillPlaySound( SKILL_SOUND_JUMPING_CRASH2 , pX,pY,pZ );			//스킬 효과음
		return FALSE;


	case SKILL_PLAY_WIND_ARROW:
		//윈드 애로우
		if ( chrAttackTarget ) {
			GetAttackPoint( &x,&y,&z );
			StartSkillDest( x,y,z , chrAttackTarget->pX,chrAttackTarget->pY+(24*fONE),chrAttackTarget->pZ , SKILL_WIND_ARROW );
			SkillPlaySound( SKILL_SOUND_WIND_ARROW , pX,pY,pZ );			//스킬 효과음
		}
		break;

	case SKILL_PLAY_PERFECT_AIM:
		//퍼펙트 암
		if ( chrAttackTarget ) {
			GetAttackPoint( &x,&y,&z );
			StartSkillDest( x,y,z , chrAttackTarget->pX,chrAttackTarget->pY+(24*fONE),chrAttackTarget->pZ , SKILL_PERFECT_AIM );
			SkillPlaySound( SKILL_SOUND_PERFECT_AIM , pX,pY,pZ );			//스킬 효과음
		}
		break;

	////////////////////////////////// 2차 스킬 ///////////////////////////////
	case SKILL_PLAY_GREAT_SMASH:
		//그레이트 스매시
		if ( GetAttackPoint( &x,&y,&z )==TRUE ) {
			if ( dwActionItemCode ) {		//HvRightHand.PatTool ) {
				// 도구 공격
				StartSkill( x,y,z, 0,Angle.y,0, SKILL_GREAT_SMASH );
				SkillPlaySound( SKILL_SOUND_SKILL_SWING1	 , pX,pY,pZ );
			}
		}
		break;
/*
	case SKILL_PLAY_MAXIMIZE:
		//맥시마이즈
		SkillPlaySound( SKILL_SOUND_SKILL_MAXIMIZE2 , pX,pY,pZ );
		break;

	case SKILL_PLAY_AUTOMATION:
		//오토매이션
		SkillPlaySound( SKILL_SOUND_SKILL_AUTOMATION3 , pX,pY,pZ );
		break;
*/
	case SKILL_PLAY_SPARK:
		//스파크
		if( chrAttackTarget ) {
			if ( point ) {
				//스파크
				StartSkillDest( pX,pY+24*fONE,pZ , chrAttackTarget->pX,chrAttackTarget->pY+24*fONE,chrAttackTarget->pZ, SKILL_SPARK , point );
				//SkillPlaySound( SKILL_SOUND_MECHANIC_BOMB1 , pX,pY,pZ );			//스킬 효과음

				if ( this==lpCurPlayer ) {
					/*
					damage = GetRandomPos( Spark_Damage[point-1][0] , Spark_Damage[point-1][1] )/2;
					Skil_RangeAttack( chrAttackTarget->pX, chrAttackTarget->pY, chrAttackTarget->pZ, Spark_Area[point-1]*fONE , damage , 0 );
					//상대 캐릭터에 공격 가함 
					QuickSendTransAttack( chrAttackTarget , damage );
					*/
					//범위형 공격 범위 선정 (원)
					dm_SelectRange( chrAttackTarget->pX, chrAttackTarget->pY, chrAttackTarget->pZ, 80+(point*5) , FALSE );
					//범위공격 설정후 전송
					dm_SendRangeDamage( chrAttackTarget->pX, chrAttackTarget->pY, chrAttackTarget->pZ, 0 ,0, 0, 0, 0, lpCurPlayer->AttackSkil );
					//dm_SendRangeDamage( chrAttackTarget->pX, chrAttackTarget->pY, chrAttackTarget->pZ, chrAttackTarget ,0, 0, 0, 0, lpCurPlayer->AttackSkil );
				}
				SkillPlaySound( SKILL_SOUND_SKILL_SPARK2	 , pX,pY,pZ);
			}
		}
		break;

	case SKILL_PLAY_GRAND_SMASH:
		//그랜드 스매쉬
		//if ( GetAttackPoint( &x,&y,&z )==TRUE ) {
			if ( dwActionItemCode && AttackCritcal>=0 ) {
				// 도구 공격
				AssaParticle_GrandSmash( this );
				SkillPlaySound( SKILL_SOUND_SKILL_SWING2	 , pX,pY,pZ );
			}
			else {
				//미스 소리
				//무기 소리 직접 연주
				PlayWaponSoundDirect( pX,pY,pZ , 13 );
			}
		//}

		if ( this==lpCurPlayer && chrAttackTarget && MotionEvent==1 ) {
			temp = smCharInfo.Attack_Rating;
			smCharInfo.Attack_Rating += Grand_Smash_AttackRate[point-1];
			//공격 전송
			cnt = dm_SendTransDamage( chrAttackTarget , 0,0 , 0, 0, lpCurPlayer->AttackSkil , TRUE );
			smCharInfo.Attack_Rating = temp;

			if ( !cnt )
				AttackCritcal = -1;				//빗나감
			else
				AttackCritcal = 0;					//명중
		}

		break;

	case SKILL_PLAY_SPARK_SHIELD:
		//스파크 쉴드
		if ( HvLeftHand.PatTool ) {
			/*
			if ( GetToolBipPoint( &HvLeftHand , &x,&y,&z ) ) {
				Pos1.x = x;
				Pos1.y = y;
				Pos1.z = z;
				*/
				AssaParticle_SparkShieldStart( this , Spark_Shield_Time[point-1] );
				SkillPlaySound( SKILL_SOUND_SKILL_SPARK3	 , pX,pY,pZ );
			//}
		}
		break;


	case SKILL_PLAY_GROUND_PIKE:
		//그라운드 파이크
 		if ( point ) {
			GetMoveLocation( 0 , 0, 32*fONE , 0, Angle.y , 0 );
			StartSkill( pX+GeoResult_X,pY,pZ+GeoResult_Z, 0, Angle.y,0, SKILL_GROUND_PIKE , point );
 			SetDynLight( pX,pY,pZ, 0, 0,100,0, 700 );

			SkillPlaySound( SKILL_SOUND_SKILL_GROUND_PIKE	 , pX,pY,pZ );
		}
		break;

	case SKILL_PLAY_EXPANSION:
		//익스팬션
		SkillPlaySound( SKILL_SOUND_SKILL_EXPANSION2	 , pX,pY,pZ );
		if ( GetAttackPoint( &x,&y,&z )==TRUE ) {
			if ( dwActionItemCode ) {
				StartSkill( x,y,z, 0,Angle.y,0, SKILL_GREAT_SMASH );
				//StartEffect( x,y,z , EFFECT_CRITICAL_HIT1 );
				SetDynLight( x,y,z , 100, 0,0,0, 400 );
			}
		}
		return FALSE;


	case SKILL_PLAY_TORNADO:
		//토네이도
		if ( chrAttackTarget ) {
			x = (pX-chrAttackTarget->pX)>>FLOATNS;
			z = (pZ-chrAttackTarget->pZ)>>FLOATNS;

			if ( (x*x+z*z)<(50*50) ) {
				y = GetRadian2D( pX , pZ , chrAttackTarget->pX , chrAttackTarget->pZ );
				
				GetMoveLocation( 0,0,50*fONE , 0,y,0 );
				x = pX+GeoResult_X;
				z = pZ+GeoResult_Z;
				y = chrAttackTarget->pY;
				StartSkillDest( pX,pY,pZ, x,y,z, SKILL_TORNADO , point );

			}
			else {
				StartSkillDest( pX,pY,pZ, chrAttackTarget->pX,chrAttackTarget->pY,chrAttackTarget->pZ, SKILL_TORNADO , point );
			}

			SkillPlaySound( SKILL_SOUND_SKILL_TORNADO1	 , pX,pY,pZ );
		}
		break;

	case SKILL_PLAY_VENOM_SPEAR:
		//베놈 스피어


		if ( this==lpCurPlayer ) {
			//범위형 공격 범위 선정 (디바인 라이트닝)
			dm_SelectDamageCount( this , VenomSpear_Num[ point-1 ] , VenomSpear_Range[ point-1 ] );
			//범위공격 설정후 전송
			dm_SendRangeDamage( pX,pY,pZ, 0 ,smCharInfo.Attack_Damage[0], smCharInfo.Attack_Damage[1], 0, 0, AttackSkil );
		}
		SkillPlaySound( SKILL_SOUND_SKILL_VENOM_SPEAR1	 , pX,pY,pZ );

		if ( SkillPlay_VenomSpear_Effect( this , point )>0 ) {			
			SkillPlaySound( SKILL_SOUND_SKILL_VENOM_SPEAR2	 , pX,pY,pZ );
		}
		break;

	case SKILL_PLAY_VANISH:
		//블레이드 오브 블레이즈
		//AssaParticle_BladeOfBlaze( this , 10 );
		SkillPlaySound( SKILL_SOUND_SKILL_VANISH2	 , pX,pY,pZ );

		if ( lpCurPlayer==this ) {
			PlayVanish = Vanish_Time[point-1]*70;
			PlayVague = 0;
			SendProcessSKillToServer( SKILL_PLAY_VANISH , point , 0, 0 );

			if( IsCreateNewRenderTarget() ) {
				dwM_BlurTime=dwPlayTime+Vanish_Time[point-1]*1000;
				SetFilterEffect( FILTER_EFFECT_SET_MOTION_BLUR, 80 );
			}
		}
		break;

	case SKILL_PLAY_CHAIN_LANCE:
		//체인랜스
		if ( chrAttackTarget && GetAttackPoint( &x,&y,&z )==TRUE ) {
			Pos1.x = x;
			Pos1.y = y;
			Pos1.z = z;
			if ( lpCurPlayer->AttackCritcal>=0 ) {
				AssaParticle_ChainLance( &Pos1 );

				switch(MotionEvent) {
				case 1:
					SkillPlaySound( SKILL_SOUND_SKILL_CHAIN_LANCE1 , pX,pY,pZ );			//스킬 효과음
					break;
				case 2:
					SkillPlaySound( SKILL_SOUND_SKILL_CHAIN_LANCE2 , pX,pY,pZ );			//스킬 효과음
					break;
				case 3:
					SkillPlaySound( SKILL_SOUND_SKILL_CHAIN_LANCE3 , pX,pY,pZ );			//스킬 효과음
					break;
				}
			}
			else {
				//미스 소리
				//무기 소리 직접 연주
				PlayWaponSoundDirect( pX,pY,pZ , 13 );
			}
		}

		if ( this==lpCurPlayer && MotionEvent<3 && chrAttackTarget ) {
			//명중력 보강
			//x = smCharInfo.Attack_Rating;
			//smCharInfo.Attack_Rating += Chain_Lance_AttackRate[point-1];

			cnt = dm_SendTransDamage( chrAttackTarget , 0,0 , 0, 0, AttackSkil , TRUE );
			if ( !cnt )
				lpCurPlayer->AttackCritcal = -1;				//빗나감
			else
				lpCurPlayer->AttackCritcal = 0;					//명중

			//smCharInfo.Attack_Rating = x;
		}	
		break;

	case SKILL_PLAY_FALCON:
		//팔콘
		break;

	case SKILL_PLAY_ARROWOF_RAGE:
		//엘로우오브레이지
		if ( chrAttackTarget && GetAttackPoint( &x,&y,&z )==TRUE ) {
			StartSkillDest( x,y,z, chrAttackTarget->pX,chrAttackTarget->pY,chrAttackTarget->pZ, SKILL_ARROW_OF_RAGE , point );

/*
//에로우 오브 레이지 
int A_Rage_ArrowNum[10]   = {2,3,4,5,6,7,8,9,10,11};
int A_Rage_Damage[10]     = {4,4,4,4,3,3,3,3,2,2};
int A_Rage_Area[10]	  = {60,65,70,75,80,85,90,95,100,110}; 	
*/
			if ( this==lpCurPlayer && chrAttackTarget ) {
				//공격 전송
				/*
				cnt = GetRandomPos( smCharInfo.Attack_Damage[0] , smCharInfo.Attack_Damage[1] );
				cnt += smCharInfo.Level/A_Rage_Damage[ point-1 ];
				Skil_RangeAttack( chrAttackTarget->pX, chrAttackTarget->pY, chrAttackTarget->pZ, A_Rage_Area[point-1]*fONE , cnt , 0 );
				*/
				//범위형 공격 범위 선정 (원)
				dm_SelectRange( chrAttackTarget->pX, chrAttackTarget->pY, chrAttackTarget->pZ, A_Rage_Area[point-1] , FALSE );
				//범위공격 설정후 전송
				dm_SendRangeDamage( chrAttackTarget->pX, chrAttackTarget->pY, chrAttackTarget->pZ, 0 ,0, 0, 0, 0, lpCurPlayer->AttackSkil );

			}
			SkillPlaySound( SKILL_SOUND_WIND_ARROW , pX,pY,pZ );			//스킬 효과음
		}
		break;

	case SKILL_PLAY_AVALANCHE:
		//아발란체
		if ( chrAttackTarget && GetAttackPoint( &x,&y,&z )==TRUE ) {

			if ( AttackCritcal>=0 ) {
				StartSkillDest( x,y,z, chrAttackTarget->pX,chrAttackTarget->pY+(24*fONE),chrAttackTarget->pZ, SKILL_AVALANCHE );

				switch( rand()%3 ) {
				case 0:
					SkillPlaySound( SKILL_SOUND_SKILL_AVALANCHE1	 , pX,pY,pZ );
					break;
				case 1:
					SkillPlaySound( SKILL_SOUND_SKILL_AVALANCHE2	 , pX,pY,pZ );
					break;
				case 2:
					SkillPlaySound( SKILL_SOUND_SKILL_AVALANCHE3	 , pX,pY,pZ );
					break;
				}
			}
			else 
				WeaponPlaySound( this );

		}
		break;

	case SKILL_PLAY_ELEMENTAL_SHOT:
		//엘리멘탈 샷
		if ( chrAttackTarget && GetAttackPoint( &x,&y,&z )==TRUE ) {
			if ( AttackCritcal>=0 ) {

				Pos1.x = x;
				Pos1.y = y;
				Pos1.z = z;

				Pos2.x = chrAttackTarget->pX;
				Pos2.y = chrAttackTarget->pY+(24*fONE);
				Pos2.z = chrAttackTarget->pZ;

				if ( AttackCritcal>=0 ) {
					switch( param ) {
					case 0:
						AssaParticle_ElementalShotFire( &Pos1 , &Pos2 );

/*
						//호동씨가 넣자
							if ( (rand()%2)==0 ) 
								SkillPlaySound( SKILL_SOUND_SKILL_ENCHANTWEAPON_LIGHTNING1	 , pX,pY,pZ );
							else
								SkillPlaySound( SKILL_SOUND_SKILL_ENCHANTWEAPON_LIGHTNING2	 , pX,pY,pZ );
							break;
						case (sITEMINFO_FIRE+1):
							AssaParticle_FireHit1( &pos );
							SetDynLight( x,y,z , 100, 0,0,0, 400 );
							if ( (rand()%2)==0 ) 
								SkillPlaySound( SKILL_SOUND_SKILL_ENCHANTWEAPON_FIRE1	 , pX,pY,pZ );
							else
								SkillPlaySound( SKILL_SOUND_SKILL_ENCHANTWEAPON_FIRE2	 , pX,pY,pZ );
							break;
*/
						break;
					case 1:
						AssaParticle_ElementalShotLighting( &Pos1 , &Pos2 );
						break;
					}
					SkillPlaySound( SKILL_SOUND_SKILL_ELEMENTAL_SHOT	 , pX,pY,pZ );
				}
			}
		}
		break;

	case SKILL_PLAY_BOMB_SHOT:
		//범샷
		if ( chrAttackTarget && GetAttackPoint( &x,&y,&z )==TRUE ) {
			if ( AttackCritcal>=0 ) {
				Pos1.x = x;
				Pos1.y = y;
				Pos1.z = z;

				Pos2.x = chrAttackTarget->pX;
				Pos2.y = chrAttackTarget->pY+(24*fONE);
				Pos2.z = chrAttackTarget->pZ;

				AssaParticle_BombShot( &Pos1, &Pos2 );

				SkillPlaySound( SKILL_SOUND_SKILL_BOMB_SHOT1	 , pX,pY,pZ );

			}
		}
		break;

	case SKILL_PLAY_PERFORATION:
		//퍼포레이션
		if ( chrAttackTarget && GetAttackPoint( &x,&y,&z )==TRUE ) {
			if ( AttackCritcal>=0 ) {
				Pos1.x = x;
				Pos1.y = y;
				Pos1.z = z;

				Pos2.x = chrAttackTarget->pX;
				Pos2.y = chrAttackTarget->pY+(24*fONE);
				Pos2.z = chrAttackTarget->pZ;

				AssaParticle_Perforation( &Pos1, &Pos2 );
				SkillPlaySound( SKILL_SOUND_SKILL_PERFORATION	 , pX,pY,pZ );

				rect.left = -40;
				rect.right = 40;
				rect.top = 0;
				rect.bottom=240;
				rect.bottom+=20;
				rect.bottom += 20;

				//범위형 공격 범위 선정 (박스)
				dm_SelectRangeBox( this , &rect , FALSE );

				for(cnt=0;cnt<dmSelected_CharCnt;cnt++) {
					if ( lpSelected_Char[cnt] ) {
						AssaParticle_PerforationHit( lpSelected_Char[cnt] , this , 0.5f );
					}
				}
			}
		}
		break;

	case SKILL_PLAY_TRIPLE_IMPACT:
		//트리플 임팩트
		if ( AttackCritcal>=0 ) {
			if ( GetAttackPoint( &x,&y,&z )==TRUE ) {
				StartSkill( x,y,z, 0,0,0, SKILL_TRIPLE_IMPACT );
				SetDynLight( x,y,z , 100, 0,0,0, 400 );
			}

			AttackCritcal = 1;
		}
		WeaponPlaySound( this );

		if ( this==lpCurPlayer && chrAttackTarget && MotionEvent<T_Impact_Hit[point-1] ) {
			if ( AttackCritcal>=0 ) {
/*
				if ( (rand()%100)<sinGetCritical( chrAttackTarget->smCharInfo.Level ) )
					AttackCritcal = TRUE;			//크리티컬 공격 성공
				else
					AttackCritcal = FALSE;

				if ( AttackCritcal==FALSE ) {

					if ( MotionEvent==1 )
						AttackCritcal = TRUE;

					//공격 전송
					cnt = SendTransAttack( chrAttackTarget , smWsockServer , 0 , T_Impact_Damage[point-1] );

					AttackCritcal = FALSE;
				}
				else {
					//공격 전송
					cnt = SendTransAttack( chrAttackTarget , smWsockServer , 0 , T_Impact_Damage[point-1] );
				}
*/
				cnt = dm_SendTransDamage( chrAttackTarget , 0,0 , 0, 0, lpCurPlayer->AttackSkil , TRUE );

				if ( cnt>0 ) {
					DamageExp( chrAttackTarget , cnt );					//데미지에 따른 경험치 상승
					if ( AttackExp ) {
						//AddExp( AttackExp );
						//SetTotalAddExp( AttackExp );			//경험치 증가 기록
						AttackExp=0;
					}
				}
			}
		}
		break;

	case SKILL_PLAY_BRUTAL_SWING:
		//브루탈 스윙
		if ( AttackCritcal>=0 ) {
			if ( GetAttackPoint( &x,&y,&z )==TRUE ) {
				StartSkill( x,y,z, 0,0,0, SKILL_BRUTAL_SWING );
				SetDynLight( x,y,z , 100, 0,0,0, 400 );
				SkillPlaySound( SKILL_SOUND_SKILL_SWING1 , pX,pY,pZ );
			}
		}
		else	//미스
			WeaponPlaySound( this );
		break;

	case SKILL_PLAY_RAGEOF_ZECRAM:
		if ( GetAttackPoint( &x,&y,&z )==TRUE ) {
			if ( MotionEvent==1 ) {
				StartSkill( x,y,z, 0,0,0, SKILL_RAGE_OF_ZECRAM_HIT1 );
				//이펙트 예약 추가 ( 0.6초후 )
				AddTimeEffect( SKILL_RAGE_OF_ZECRAM_HIT2 , dwPlayTime+600 , x,y,z );

				if ( this==lpCurPlayer && chrAttackTarget ) {
					//마지막 공격 미리 보냄
					/*
					cnt = GetRandomPos( smCharInfo.Attack_Damage[0] , smCharInfo.Attack_Damage[1] );
					cnt += (cnt*R_Zecram_Damage[point-1])/100;
					cnt >>=1;
					Skil_RangeAttack( lpCurPlayer->pX, lpCurPlayer->pY, lpCurPlayer->pZ, 70*fONE , cnt , 100 );	//100은 불 속성
					*/
					//범위형 공격 범위 선정 (원)
					dm_SelectRange( lpCurPlayer->pX, lpCurPlayer->pY, lpCurPlayer->pZ, 70 , FALSE );
					//범위공격 설정후 전송
					dm_SendRangeDamage( lpCurPlayer->pX, lpCurPlayer->pY, lpCurPlayer->pZ, 0 ,0, 0, 0, 0, lpCurPlayer->AttackSkil );

					SkillPlaySound( SKILL_SOUND_SKILL_RAGE_OF_ZECRAM1 , pX,pY,pZ, FALSE );
					SetDynLight( x,y,z , 100, 0,0,0, 400 );
					
				}
			}
		}
		break;
	case SKILL_PLAY_ROAR:
		//로어
		//Skil_RangeEffect( this , Roar_Range[point-1] , SKILL_PLAY_ROAR , 0 , point );
		break;

	case SKILL_PLAY_AVANGING_CRASH:
		//어벤징 크래쉬
		if ( GetAttackPoint( &x,&y,&z )==TRUE ) {
			Pos1.x = x;
			Pos1.y = y;
			Pos1.z = z;

			if ( AttackCritcal>=0 )	{
				AssaParticle_AvangingCrash( this );
				SkillPlaySound( SKILL_SOUND_SKILL_AVANGING_CRASH , pX,pY,pZ );
			}
			else {
				//미스 소리
				//무기 소리 직접 연주
				PlayWaponSoundDirect( pX,pY,pZ , 13 );
			}
		}

		if ( this==lpCurPlayer && MotionEvent==1 && chrAttackTarget ) {
			//명중력 증가
			x = lpCurPlayer->smCharInfo.Attack_Rating;

			smCharInfo.Attack_Rating += (smCharInfo.Attack_Rating * A_Crash_AttackRate[point-1])/100;

			cnt = dm_SendTransDamage( chrAttackTarget , cSkill.GetSkillPoint(SKILL_BRUTAL_SWING)  ,0 , 0, 0, AttackSkil , TRUE );

			if ( !cnt )
				AttackCritcal = -1;				//빗나감
			else {
				//데미지에 따른 경험치 상승
				DamageExp( chrAttackTarget , cnt );
				AttackCritcal = 0;
			}
			smCharInfo.Attack_Rating = x;
		}

		break;

	case SKILL_PLAY_BONE_SMASH:				
		//본스매쉬
		/*
		if ( GetAttackPoint( &x,&y,&z )==TRUE ) {
			Pos1.x = x;
			Pos1.y = y;
			Pos1.z = z;
			*/
		if ( AttackCritcal>=0 )	{
			AssaParticle_BoneSmash1( this );
			SkillPlaySound( SKILL_SOUND_SKILL_BONE_CRASH1 , pX,pY,pZ, FALSE );
		}
		else {
			//미스 소리
			//무기 소리 직접 연주
			PlayWaponSoundDirect( pX,pY,pZ , 13 );
		}

		//}

		if ( this==lpCurPlayer && MotionEvent==1 && chrAttackTarget ) {
			//공격 
			cnt = dm_SendTransDamage( chrAttackTarget , 0 ,0 , 0, 0, AttackSkil , TRUE );

			if ( !cnt )
				AttackCritcal = -1;				//빗나감
			else {
				//데미지에 따른 경험치 상승
				DamageExp( chrAttackTarget , cnt );
				AttackCritcal = 0;
			}
		}

		break;

		//////////////////////////////// 나이트 ///////////////////////////////
	case SKILL_PLAY_HOLY_BODY:
		break;

	case SKILL_PLAY_SWORD_BLAST:

		if ( chrAttackTarget ) {
			sinEffect_Sword_Blast( this , chrAttackTarget );

			SkillPlaySound( SKILL_SOUND_SKILL_SWORDBLAST2	 ,pX,pY,pZ  );
		}
		break;

	case SKILL_PLAY_DOUBLE_CRASH:
		if ( AttackCritcal>=0 ) {
			if ( GetAttackPoint( &x,&y,&z )==TRUE ) {
				if ( AttackCritcal==0 ) {
					StartEffect( x,y,z , EFFECT_NORMAL_HIT1 );
					SetDynLight( x,y,z , 80, 0,0,0, 300 );
				}
				else {
					StartEffect( x,y,z , EFFECT_CRITICAL_HIT1 );
					SetDynLight( x,y,z , 100, 0,0,0, 400 );
				}
			}
		}
		WeaponPlaySound( this );

		if ( this==lpCurPlayer && chrAttackTarget && MotionEvent<2 ) {
/*
			if ( (rand()%100)<(sinGetCritical( chrAttackTarget->smCharInfo.Level )+D_Crash_Critical[point-1] ))
				lpCurPlayer->AttackCritcal = TRUE;			//크리티컬 공격 성공
			else
				lpCurPlayer->AttackCritcal = FALSE;

			cnt = lpCurPlayer->SendTransAttack( chrAttackTarget , smWsockServer , 0 , D_Crash_Damage[point-1] );
*/

			cnt = dm_SendTransDamage( chrAttackTarget , 0,0 , 0, 0, lpCurPlayer->AttackSkil , TRUE );

			if ( cnt>0 ) {
				lpCurPlayer->AttackCritcal = 0;
				DamageExp( chrAttackTarget , cnt );					//데미지에 따른 경험치 상승
				if ( AttackExp ) {
					//AddExp( AttackExp );
					//SetTotalAddExp( AttackExp );			//경험치 증가 기록
					AttackExp=0;
				}
			}
			else
				lpCurPlayer->AttackCritcal = -1;
		}
		break;

		///////////////////////////// 2차 ///////////////////////////
	case SKILL_PLAY_HOLY_VALOR:
		if ( this==lpCurPlayer ) {
			//파티원에게 스킬 사용 정보 보내기
			SendPartySkillToServer( SKILL_PLAY_HOLY_VALOR , point , 200 , 0 , 0 , 0 , 0 );
		}
		break;

	case SKILL_PLAY_BRANDISH:
		sinSkillEffect_Brandish( this );
		SkillPlaySound( SKILL_SOUND_SKILL_BLANDISH2			 , pX,pY,pZ );
		break;

	case SKILL_PLAY_PIERCING:
		sinSkillEffect_Piercing_Attack( this );
		break;

	case SKILL_PLAY_FLAME_BRANDISH:
		AssaParticle_FlameBrandish( this );
		SkillPlaySound( SKILL_SOUND_SKILL_FLAME_BRANDISH ,pX,pY,pZ  );

		if ( chrAttackTarget ) {
			//범위형 공격 범위 선정 (원)
			dm_SelectRange( chrAttackTarget->pX, chrAttackTarget->pY, chrAttackTarget->pZ, 80 , FALSE );
			for(cnt=0;cnt<dmSelected_CharCnt;cnt++) {
				if ( lpSelected_Char[cnt] ) {
					AssaParticle_FlameBrandishHit1( lpSelected_Char[cnt] );
				}
			}
		}
		break;

	case SKILL_PLAY_DIVINE_INHALATION:
		//디바인인홀레이션
		AssaParticle_DivineInhalationStart( this , D_Inhalation_Time[ point-1 ] );
		break;

	case SKILL_PLAY_HOLY_INCANTATION:
		if ( chrAttackTarget ) {
			AssaParticle_HolyIncantationShot( chrAttackTarget );

			SkillPlaySound( SKILL_SOUND_SKILL_HOLY_INCANTATION	 , pX,pY,pZ );

			if ( this==lpCurPlayer )
				dm_SendTransDamage( chrAttackTarget , 0,0,0,0, lpCurPlayer->AttackSkil , FALSE );	
		}
		break;

	case SKILL_PLAY_GRAND_CROSS:
		if ( chrAttackTarget && AttackCritcal>=0 && GetAttackPoint( &x,&y,&z )==TRUE ) {
			Pos1.x = x;
			Pos1.y = y;
			Pos1.z = z;
			if ( MotionEvent==1 ) {
				AssaParticle_GrandCrossHorz( &Pos1 , Angle.y );
				SkillPlaySound( SKILL_SOUND_SKILL_SWING2	 , pX,pY,pZ );
			}
			else {
				AssaParticle_GrandCrossVert( &Pos1 , Angle.y );
				SkillPlaySound( SKILL_SOUND_SKILL_GRAND_CROSS	 , pX,pY,pZ );
			}
		}
		else {
			//미스 소리
			//무기 소리 직접 연주
			PlayWaponSoundDirect( pX,pY,pZ , 13 );
		}

		if ( this==lpCurPlayer && chrAttackTarget && MotionEvent==1 ) {

			x = smCharInfo.Attack_Rating;

			smCharInfo.Attack_Rating += (smCharInfo.Attack_Rating*Grand_Cross_AttackRate[point-1])/100;

			cnt = dm_SendTransDamage( chrAttackTarget , 0,0 , 0, 0, lpCurPlayer->AttackSkil , TRUE );

			if ( cnt>0 ) 
				lpCurPlayer->AttackCritcal = 0;
			else
				lpCurPlayer->AttackCritcal = -1;

			smCharInfo.Attack_Rating = x;
		}

		break;



		//////////////////////////////// 아탈란타 ///////////////////////////////
	case SKILL_PLAY_SHIELD_STRIKE:
		SkillPlaySound( SKILL_SOUND_SKILL_SHIELDSTRIKE	 , pX,pY,pZ );
		break;

	case SKILL_PLAY_VIGOR_SPEAR:
		if ( chrAttackTarget && GetAttackPoint( &x,&y,&z )==TRUE ) {
			sinEffect_VigorSpear( this , chrAttackTarget );

			SkillPlaySound( SKILL_SOUND_SKILL_VIGORSPEAR	 , pX,pY,pZ );
		}
		break;

	case SKILL_PLAY_FARINA:
		if ( chrAttackTarget && AttackCritcal>=0 ) 
			SkillPlaySound( SKILL_SOUND_SKILL_FARINA	 , pX,pY,pZ );

		return FALSE;

	case SKILL_PLAY_TWIST_JAVELIN:
		if ( chrAttackTarget && AttackCritcal>=0 && GetAttackPoint( &x,&y,&z )==TRUE ) {

			Pos1.x = x;
			Pos1.y = y;
			Pos1.z = z;
			Pos2.x = chrAttackTarget->pX;
			Pos2.y = chrAttackTarget->pY+28*fONE;
			Pos2.z = chrAttackTarget->pZ;

			AssaParticle_TwistJavelinShot( &Pos1, &Pos2 );
			SkillPlaySound( SKILL_SOUND_SKILL_TWISTJAVELIN			 , pX,pY,pZ );
		}
		return FALSE;
		//break;
	case SKILL_PLAY_SOUL_SUCKER:
		//범위형 공격 이펙트 연출
		rect.left = -30;
		rect.right = 30;
		rect.top = 0;
		rect.bottom = 200;
		if ( Skil_RangeBoxEffect( this , &rect , SKILL_PLAY_SOUL_SUCKER , 0 )>0 )
			SkillPlaySound( SKILL_SOUND_SKILL_SOULSUCKER2			 , pX,pY,pZ );
		else
			SkillPlaySound( SKILL_SOUND_SKILL_SHIELDSTRIKE	 , pX,pY,pZ );
		break;

	case SKILL_PLAY_FIRE_JAVELIN:
		if ( chrAttackTarget && AttackCritcal>=0 && GetAttackPoint( &x,&y,&z )==TRUE ) {

			Pos1.x = x;
			Pos1.y = y;
			Pos1.z = z;
			Pos2.x = chrAttackTarget->pX;
			Pos2.y = chrAttackTarget->pY+28*fONE;
			Pos2.z = chrAttackTarget->pZ;

			AssaParticle_FireJavelin( &Pos1, &Pos2 );
			SkillPlaySound( SKILL_SOUND_SKILL_FIREJAVELIN			 , pX,pY,pZ );
		}
		//break;
		return FALSE;

	case SKILL_PLAY_SPLIT_JAVELIN:
		if ( chrAttackTarget && AttackCritcal>=0 && GetAttackPoint( &x,&y,&z )==TRUE ) {
			/*
			Pos1.x = x;
			Pos1.y = y;
			Pos1.z = z;
			Pos2.x = chrAttackTarget->pX;
			Pos2.y = chrAttackTarget->pY+28*fONE;
			Pos2.z = chrAttackTarget->pZ;
			AssaParticle_SplitJavelin( &Pos1, &Pos2 , Split_Javelin_AttackNum[point-1] );
			*/
			SkillPlaySound( SKILL_SOUND_SKILL_SPLIT_JAVELIN2	 , pX,pY,pZ );

		}
		return TRUE;

	case SKILL_PLAY_TRIUMPH_OF_VALHALLA:
		if ( chrAttackTarget ) {
			if ( chrAttackTarget->smCharInfo.State==smCHAR_STATE_USER ) {
				AssaParticle_TriumphOfValhallaMember( this , chrAttackTarget , T_Of_Valhalla_Time[ point-1 ] );
				if ( this==lpCurPlayer ) {
					SendProcessSKillToServer( SKILL_PLAY_TRIUMPH_OF_VALHALLA , point , chrAttackTarget->dwObjectSerial, smCharInfo.Level );
				}
			}
		}
		else {
			AssaParticle_TriumphOfValhalla( this , T_Of_Valhalla_Time[ point-1 ] );
		}
		break;

	case SKILL_PLAY_LIGHTNING_JAVELIN:
		if ( chrAttackTarget && GetAttackPoint( &x,&y,&z )==TRUE ) {
			Pos1.x = x;
			Pos1.y = y;
			Pos1.z = z;
			Pos2.x = chrAttackTarget->pX;
			Pos2.y = chrAttackTarget->pY+28*fONE;
			Pos2.z = chrAttackTarget->pZ;

			AssaParticle_LightingJavelin( &Pos1, &Pos2 );
			switch( rand()%2 ) {
				case 0:
					SkillPlaySound( SKILL_SOUND_SKILL_LIGHTNING_JAVELIN1	 , pX,pY,pZ );
					break;
				case 1:
					SkillPlaySound( SKILL_SOUND_SKILL_LIGHTNING_JAVELIN2	 , pX,pY,pZ );
					break;
			}
		}
		break;

	case SKILL_PLAY_STORM_JAVELIN:
		if ( chrAttackTarget && AttackCritcal>=0 && GetAttackPoint( &x,&y,&z )==TRUE ) {
			Pos1.x = x;
			Pos1.y = y;
			Pos1.z = z;
			Pos2.x = chrAttackTarget->pX;
			Pos2.y = chrAttackTarget->pY+28*fONE;
			Pos2.z = chrAttackTarget->pZ;

			AssaParticle_StormJavelin( &Pos1, &Pos2 );

			rect.left = -30;
			rect.right = 30;
			rect.top = 0;
			rect.bottom=240;
			rect.bottom+=20;
			rect.bottom += 20;

			//범위형 공격 범위 선정 (박스)
			dm_SelectRangeBox( this , &rect , FALSE );

			for(cnt=0;cnt<dmSelected_CharCnt;cnt++) {
				if ( lpSelected_Char[cnt] ) {
					AssaParticle_StormJavelinHit( lpSelected_Char[cnt] , this , 0.5f );
				}
			}

			switch( rand()%2 ) {
				case 0:
					SkillPlaySound( SKILL_SOUND_SKILL_STORM_JAVELIN1	 , pX,pY,pZ );
					break;
				case 1:
					SkillPlaySound( SKILL_SOUND_SKILL_STORM_JAVELIN2	 , pX,pY,pZ );
					break;
			}
		}

		return TRUE;


		//////////////////////////////// 프리스티스 ///////////////////////////////

	case SKILL_PLAY_HOLY_BOLT:
		if ( chrAttackTarget  ) {
			GetMoveLocation( 0 , 24*fONE, 24*fONE , 0, Angle.y , 0 );

			StartEffectMonsterDest( pX+GeoResult_X,pY+GeoResult_Y,pZ+GeoResult_Z, 
				chrAttackTarget->pX , chrAttackTarget->pY+20*fONE , chrAttackTarget->pZ ,
				MONSTER_MEPHIT_SHOT2 );

			SkillPlaySound( SKILL_SOUND_SKILL_HOLYBOLT	 , pX,pY,pZ );
		}
		break;

	case SKILL_PLAY_MULTI_SPARK:
		if ( chrAttackTarget  ) {
			sinEffect_MultiSpark( this , chrAttackTarget , param );

			SkillPlaySound( SKILL_SOUND_SKILL_MULTISPARK	 , pX,pY,pZ );
		}
		break;

	case SKILL_PLAY_DIVINE_LIGHTNING:
		//SkillPlay_DivineLightning_Select( this , point );

		if ( this==lpCurPlayer ) {
			//범위형 공격 범위 선정 (디바인 라이트닝)
			dm_SelectDamageCount( this , Divine_Lightning_Num[ point-1 ] , 180 );
			//범위공격 설정후 전송
			dm_SendRangeDamage( pX,pY,pZ, 0 ,
				cInvenTory.InvenItem[ sInven[0].ItemIndex-1 ].sItemInfo.Damage[0],
				cInvenTory.InvenItem[ sInven[0].ItemIndex-1 ].sItemInfo.Damage[1],
				0, 0, AttackSkil );
		}

		if ( SkillPlay_DivineLightning_Effect( this , point )>0 ) {
			switch( rand()%3 ) {
				case 0:
					SkillPlaySound( SKILL_SOUND_SKILL_DIVINELIGHTNING1	 , pX,pY,pZ );
					break;
				case 1:
					SkillPlaySound( SKILL_SOUND_SKILL_DIVINELIGHTNING2	 , pX,pY,pZ );
					break;
				case 2:
					SkillPlaySound( SKILL_SOUND_SKILL_DIVINELIGHTNING3	 , pX,pY,pZ );
					break;
			}
		}
		break;

	case SKILL_PLAY_HOLY_REFLECTION:
		if ( this==lpCurPlayer ) {
			SendProcessSKillToServer( SKILL_PLAY_HOLY_REFLECTION , point , 0, 0 );
		}
		break;
	case SKILL_PLAY_GREAT_HEALING:
		if ( this==lpCurPlayer ) {
			/*
			CheckCharForm();
			sinSetLife( sinGetLife()+GetRandomPos(Grand_Healing[point-1][0] , Grand_Healing[point-1][1] ));
			ResetEnergyGraph( 0 );						//에너지 그래프 첵크 초기화
			ReformCharForm();
			*/
			int dist , ddist;

			ddist = Grand_Healing_Range[point-1];
			ddist *= ddist;

			//파티원들 힐링 이펙트
			for( cnt=0;cnt<OTHER_PLAYER_MAX;cnt++ ) {
				if ( chrOtherPlayer[cnt].Flag && chrOtherPlayer[cnt].dwObjectSerial && 
					chrOtherPlayer[cnt].smCharInfo.State==smCHAR_STATE_USER &&
					chrOtherPlayer[cnt].PartyFlag && 
					chrOtherPlayer[cnt].MotionInfo->State!=CHRMOTION_STATE_DEAD ) {

					x = (chrOtherPlayer[cnt].pX - lpCurPlayer->pX)>>FLOATNS;
					y = (chrOtherPlayer[cnt].pY - lpCurPlayer->pY)>>FLOATNS;
					z = (chrOtherPlayer[cnt].pZ - lpCurPlayer->pZ)>>FLOATNS;
					dist = x*x+y*y+z*z;

					if ( dist<ddist ) {
						sinSkillEffect_Great_Healing_Party( &chrOtherPlayer[cnt] );
					}
				}
			}

		}
		break;


	case SKILL_PLAY_VIGOR_BALL:
		if ( chrAttackTarget  ) {
			if ( this==lpCurPlayer && MotionEvent<3 ) { 
				cnt = dm_SendTransDamage( chrAttackTarget , 
					cInvenTory.InvenItem[ sInven[0].ItemIndex-1 ].sItemInfo.Damage[0],
					cInvenTory.InvenItem[ sInven[0].ItemIndex-1 ].sItemInfo.Damage[1],
					0, 0, AttackSkil , FALSE );
			}

			AssaParticle_VigorBall( this , chrAttackTarget );

			switch( rand()%2 ) {
				case 0:
					SkillPlaySound( SKILL_SOUND_SKILL_VIGOR_BALL1	 , pX,pY,pZ );
					break;
				case 1:
					SkillPlaySound( SKILL_SOUND_SKILL_VIGOR_BALL2	 , pX,pY,pZ );
					break;
			}
		}
		break;

	case SKILL_PLAY_RESURRECTION:		//죽은자 살리기
		//AssaParticle_ResurrectionShot( this );
		//SkillPlaySound( SKILL_SOUND_SKILL_RESURRECTION	 , pX,pY,pZ );

		if ( this==lpCurPlayer ) {
			lpChar = FindDeadPartyUser();
			if ( lpChar ) {
				SendProcessSKillToServer( SKILL_PLAY_RESURRECTION , point , lpChar->dwObjectSerial , 0 );
				chrAttackTarget = lpChar;
			}
			else
				chrAttackTarget = 0;
		}

		if ( chrAttackTarget )
			AssaParticle_ResurrectionShot( chrAttackTarget );

		break;

	case SKILL_PLAY_EXTINCTION:
		//AssaParticle_Extinction( this );
		break;

	case SKILL_PLAY_VIRTUAL_LIFE:
		if ( chrAttackTarget ) {
			AssaParticle_VirtualLife( chrAttackTarget , Virtual_Life_Time[ point-1 ] );
		}
		else {
			AssaParticle_VirtualLife( this , Virtual_Life_Time[ point-1 ] );
		}
		SkillPlaySound( SKILL_SOUND_SKILL_VIRTUAL_LIFE	 , pX,pY,pZ );
		break;

		//////////////////////////// 매지션 /////////////////////////////////////

	case SKILL_PLAY_FIRE_BOLT:
		if ( chrAttackTarget ) {

			GetMoveLocation( 0 , 24*fONE, 24*fONE , 0, Angle.y , 0 );

			StartEffectMonsterDest( pX+GeoResult_X,pY+GeoResult_Y,pZ+GeoResult_Z, 
				chrAttackTarget->pX , chrAttackTarget->pY+20*fONE , chrAttackTarget->pZ ,
				MONSTER_IMP_SHOT2 );

			if ( ((dwPlayTime>>4)&1)==0 ) 
				SkillPlaySound( SKILL_SOUND_SKILL_FIREBOLT1	 , pX,pY,pZ );
			else
				SkillPlaySound( SKILL_SOUND_SKILL_FIREBOLT2	 , pX,pY,pZ );

		}
		break;

	case SKILL_PLAY_FIRE_BALL:
		if ( chrAttackTarget ) {
			GetMoveLocation( 0 , 24*fONE, 24*fONE , 0, Angle.y , 0 );
			StartEffectMonsterDest( pX+GeoResult_X,pY+GeoResult_Y,pZ+GeoResult_Z,
				chrAttackTarget->pX , chrAttackTarget->pY+20*fONE , chrAttackTarget->pZ ,
				MONSTER_PIGON_SHOT1 );

			if ( ((dwPlayTime>>4)&1)==0 ) 
				SkillPlaySound( SKILL_SOUND_SKILL_FIREBALL1	 , pX,pY,pZ );
			else
				SkillPlaySound( SKILL_SOUND_SKILL_FIREBALL2	 , pX,pY,pZ );
		}
		break;

	case SKILL_PLAY_COLUMN_OF_WATER:
		if ( chrAttackTarget ) {
			Pos1.x = pX;
			Pos1.y = pY;
			Pos1.z = pZ;
			Pos2.x = chrAttackTarget->pX;
			Pos2.y = chrAttackTarget->pY;
			Pos2.z = chrAttackTarget->pZ;

			AssaParticle_ColumnOfWater( &Pos1 , &Pos2 , point );

			switch( rand()%2 ) {
				case 0:
					SkillPlaySound( SKILL_SOUND_SKILL_WATORNADO1	 , pX,pY,pZ );
					break;
				case 1:
					SkillPlaySound( SKILL_SOUND_SKILL_WATORNADO2	 , pX,pY,pZ );
					break;
			}
		}
		break;

	case SKILL_PLAY_ENCHANT_WEAPON:
		if (chrAttackTarget ) {
			AssaParticle_EnchantWeaponJinMember(chrAttackTarget); //Member에게 쓰는 효과 이펙트
/*
			//시현시 해당 캐릭에 적용 시작 이펙트 출력
			//cnt = Enchant_Weapon_Time[(point&0xF)-1];
			switch( (point>>4) ) {
				case 0:
					//AssaParticle_EnchantWeaponIceJang( lpTarChar , cnt );
					break;
				case 1:
					//AssaParticle_EnchantWeaponLightJang( lpTarChar , cnt );
					break;
				case 2:
					//AssaParticle_EnchantWeaponFireJang( lpTarChar , cnt );
					break;
			}
*/
			SendProcessSKillToServer( SKILL_PLAY_ENCHANT_WEAPON , point, chrAttackTarget->dwObjectSerial, param );
		}
		else {
			if ( this==lpCurPlayer ) {
				SendProcessSKillToServer( SKILL_PLAY_ENCHANT_WEAPON , point , lpCurPlayer->dwObjectSerial, param );
				cSkill.SetEnchant_Weapon( Enchant_Weapon_Time[point-1],point);
			}
		}
		break;

	case SKILL_PLAY_DEAD_RAY:
		if ( chrAttackTarget ) {
			GetMoveLocation( 0 , 24*fONE, 24*fONE , 0, Angle.y , 0 );
			Pos1.x = pX+GeoResult_X;
			Pos1.y = pY+GeoResult_Y;
			Pos1.z = pZ+GeoResult_Z;
			Pos2.x = chrAttackTarget->pX;
			Pos2.y = chrAttackTarget->pY+32*fONE;
			Pos2.z = chrAttackTarget->pZ;

			AssaParticle_DeadLay( &Pos1, &Pos2 );
			SkillPlaySound( SKILL_SOUND_SKILL_DEATHRAY	 , pX,pY,pZ );
		}
		break;


	case SKILL_PLAY_ENERGY_SHIELD:
		AssaParticle_EnergyShield( this , Energy_Shield_Time[point-1] );
		SkillPlaySound( SKILL_SOUND_SKILL_ENERGY_SHIELD	 , pX,pY,pZ );
		break;

	case SKILL_PLAY_DIASTROPHISM:
		AssaParticle_Diastrophism( this );
		SkillPlaySound( SKILL_SOUND_SKILL_DIASTROPHISM	 , pX,pY,pZ );

		if ( this==lpCurPlayer && MotionEvent<2 ) {
			//범위형 공격 범위 선정 (원)
			dm_SelectRange( pX, pY, pZ, Diastrophism_Area[point-1] , FALSE );

			//범위공격 설정후 전송
			dm_SendRangeDamage( pX, pY, pZ, 0,
				cInvenTory.InvenItem[ sInven[0].ItemIndex-1 ].sItemInfo.Damage[0] ,
				cInvenTory.InvenItem[ sInven[0].ItemIndex-1 ].sItemInfo.Damage[1] ,
				0, 0, AttackSkil );
		}
		break;

	case SKILL_PLAY_SPIRIT_ELEMENTAL:
		AssaParticle_SpiritElemental( this , Spirit_Elemental_Time[point-1] );
		SkillPlaySound( SKILL_SOUND_SKILL_SPIRIT_ELEMENTAL	 , pX,pY,pZ );
		break;

	case SKILL_PLAY_DANCING_SWORD:
		switch( param ) {
		case 0:		//얼음
			AssaParticle_DancingSwordIce( this , Dancing_Sword_Time[ point-1 ] , point );
			break;

		case 1:		//불
			AssaParticle_DancingSwordFire( this , Dancing_Sword_Time[ point-1 ] , point );
			break;
		}
		SkillPlaySound( SKILL_SOUND_SKILL_DANCING_SWORD	 , pX,pY,pZ );
		break;


	/////////////////// 4차스킬 ///////////////////
	case SKILL_PLAY_IMPULSION:
 		if ( point && chrAttackTarget ) {

			if ( GetAttackPoint( &Pos2.x,&Pos2.y,&Pos2.z )==TRUE ) {
				SkillImpulsionLightHit(&Pos2);                     //타격 이펙트
			}

			if ( lpCurPlayer!=this ) {
				//범위형 공격 범위 선정 (원)
				dm_SelectRange( chrAttackTarget->pX, chrAttackTarget->pY, chrAttackTarget->pZ, Impulsion_Range[point-1] , TRUE , Impulsion_LightingNum[point-1] );
			}
			else {
				if ( MotionEvent==1 )
					dm_SendTransDamage( chrAttackTarget , 0,0 ,0, 0, lpCurPlayer->AttackSkil , FALSE );
			}

			SkillMetalionImpulsionHit( lpSelected_Char , dmSelected_CharCnt );

			if ( dmSelected_CharCnt )
				SkillPlaySound( SKILL_SOUND_SKILL_IMPULSION	 , pX,pY,pZ );

			SkillPlaySound( SKILL_SOUND_SKILL_CHAIN_LANCE2	 , pX,pY,pZ );
		}
		break;

	case SKILL_PLAY_COMPULSION:
 		if ( point ) {
			SkillMetalionCompulsion( this );
		}
		break;

	case SKILL_PLAY_MAGNETIC_SPHERE:
 		if ( point ) {
			SkillMetalionSphere( this , (float)Magnetic_Sphere_Time[point-1] , (float)Magnetic_Sphere_AttackDelay[point-1] );

			SkillPlaySound( SKILL_SOUND_SKILL_SPARK3	 , pX,pY,pZ );
		}
		break;

	case SKILL_PLAY_METAL_GOLEM:
 		if ( point ) {
			SkillMetalionGolem( this );

		}
		break;

	case SKILL_PLAY_DESTROYER:

		if ( chrAttackTarget ) {
			if ( AttackCritcal>=0 ) {
				GetAttackPoint( &Pos2.x,&Pos2.y,&Pos2.z );
				SkillWarriorDestroyerHit(&Pos2);                       //타격
				SetDynLight( Pos2.x,Pos2.y,Pos2.z , 100, 0,0,0, 400 );

				if ( MotionEvent<3 )
					SkillPlaySound( SKILL_SOUND_SKILL_AVANGING_CRASH , pX,pY,pZ );
				else
					SkillPlaySound( SKILL_SOUND_SKILL_EXPANSION2	 , pX,pY,pZ );
			}
			else {
				//미스 소리
				//무기 소리 직접 연주
				PlayWaponSoundDirect( pX,pY,pZ , 13 );
			}


			if ( this==lpCurPlayer && MotionEvent<3 ) {
				cnt = dm_SendTransDamage( chrAttackTarget , 0,0 , 0, 0, lpCurPlayer->AttackSkil , TRUE );
				if ( !cnt )
					lpCurPlayer->AttackCritcal = -1;				//빗나감
				else
					lpCurPlayer->AttackCritcal = 0;					//명중

			}
		}


		break;

	case SKILL_PLAY_BERSERKER:
 		if ( point ) {
			SkillWarriorBetserker( this , (float)Berserker_Time[point-1] );
			AttackAnger = 200;
		}
		break;

	case SKILL_PLAY_CYCLONE_STRIKE:
		if ( lpCurPlayer!=this ) {
			dm_SelectRange( pX, pY, pZ, Cyclone_Strike_Area[point-1] , TRUE , Cyclone_Strike_AttackNum[point-1] );
			dm_AddRangeDamage( chrAttackTarget );			//선택된 범위공격 목록에 캐릭 강제 추가
		}

		for(cnt=0;cnt<dmSelected_CharCnt;cnt++) {
			StartEffect( lpSelected_Char[cnt]->pX,lpSelected_Char[cnt]->pY+16*fONE,lpSelected_Char[cnt]->pZ , EFFECT_NORMAL_HIT1 );
		}
		break;

	//파이크맨
	case SKILL_PLAY_ASSASSIN_EYE:
 		if ( point && chrAttackTarget ) {
			SkillLancelotAssassinEye( chrAttackTarget , (float)Assassin_Eye_Time[point-1] );
		}
		break;

	case SKILL_PLAY_CHARGING_STRIKE:
		if ( chrAttackTarget && point ) {
			cnt = frame-MotionInfo->StartFrame*160;

			if ( MotionInfo->EventFrame[1]<=(DWORD)cnt ) {
				SkillLancelotChargingStrike( this );

				if ( MotionInfo->EventFrame[2]<=(DWORD)cnt && GetAttackPoint( &x,&y,&z )==TRUE ) {
					if ( AttackCritcal>=0 ) {
						StartSkill( x,y,z, 0,0,0, SKILL_TRIPLE_IMPACT );
						SetDynLight( x,y,z , 100, 0,0,0, 400 );

						switch( MotionEvent ) {
							case 2:
								SkillPlaySound( SKILL_SOUND_SKILL_CHAIN_LANCE2	 , pX,pY,pZ );
								break;
							case 3:
								SkillPlaySound( SKILL_SOUND_SKILL_AVANGING_CRASH , pX,pY,pZ );
								break;
							case 4:
							case 5:
								SkillPlaySound( SKILL_SOUND_SKILL_CHARGING_STRIKE	 , pX,pY,pZ );
								break;
						}
					}
					else {
						//미스 소리
						//무기 소리 직접 연주
						PlayWaponSoundDirect( pX,pY,pZ , 13 );
					}
				}

				if ( this==lpCurPlayer && SkillEventDamageCount<3 ) {

					if ( SkillEventDamageCount==0 )
						SkillStopSound(SKILL_SOUND_SKILL_CHARGING);

					temp = dm_SendTransDamage( chrAttackTarget , ChargingFlag ,0 , 0, 0, lpCurPlayer->AttackSkil , TRUE );

					if ( !temp )
						AttackCritcal = -1;				//빗나감
					else
						AttackCritcal = 0;					//명중

					SkillEventDamageCount++;
					MotionLoopSpeed = GetAttackSpeedFrame( 7 );

					MouseButton[1] = 0;
					CancelAttack();
				}
			}
			else {
				if ( this==lpCurPlayer ) ChargingFlag = 0;
			}

			cnt = (MotionInfo->EventFrame[3]/160)-MotionInfo->StartFrame;

			/*
			//모션 이벤트 확장 설정
			switch( cnt ) {
				case 226:
					dwEventFrameExt[0] = (MotionInfo->StartFrame+240)*160;
					break;
				case 396:
					dwEventFrameExt[0] = (MotionInfo->StartFrame+410)*160;
					break;
				case 576:
					dwEventFrameExt[0] = (MotionInfo->StartFrame+590)*160;
					break;
			}
			*/
			dwEventFrameExt[0] = (MotionInfo->StartFrame+140)*160;	//시작 프레임에서 140프레임 후에 강제로 삽입
		}


		break;

	case SKILL_PLAY_VAGUE:
		if ( lpCurPlayer==this ) {
			PlayVanish = Vague_Time[point-1]*70;
			PlayVague = point;
			SendProcessSKillToServer( SKILL_PLAY_VAGUE , point , 0, 0 );

			if( IsCreateNewRenderTarget() ) {
				dwM_BlurTime=dwPlayTime+Vanish_Time[point-1]*1000;
				SetFilterEffect( FILTER_EFFECT_SET_MOTION_BLUR, 80 );
			}
		}
		break;

	case SKILL_PLAY_SHADOW_MASTER:

		dwEventFrameExt[0] = (793-710+MotionInfo->StartFrame)*160;
		dwEventFrameExt[1] = (794-710+MotionInfo->StartFrame)*160;
		dwEventFrameExt[2] = (795-710+MotionInfo->StartFrame)*160;

		//793	794	795
		if ( chrAttackTarget && point ) {
			for(cnt=0;cnt<7;cnt++) {
				if ( SkillShadowMasterEvent[ Shadow_Master_ShadowNum[point-1] ][cnt]==MotionEvent ) {

					if ( AttackCritcal>=0 ) {
						if ( GetAttackPoint( &x,&y,&z )==TRUE ) {
							StartSkill( x,y,z, 0,0,0, SKILL_TRIPLE_IMPACT );
							SetDynLight( x,y,z , 100, 0,0,0, 400 );

							switch( rand()%3 ) {
								case 0:
									SkillPlaySound( SKILL_SOUND_SKILL_SHADOW_STRIKE1	 , pX,pY,pZ );
									break;
								case 1:
									SkillPlaySound( SKILL_SOUND_SKILL_SHADOW_STRIKE2	 , pX,pY,pZ );
									break;
								case 2:
									SkillPlaySound( SKILL_SOUND_SKILL_SHADOW_STRIKE3	 , pX,pY,pZ );
									break;
							}
						}
						AttackCritcal = 1;
					}
					else {
						//미스 소리
						PlayWaponSoundDirect( pX,pY,pZ , 13 );
						//WeaponPlaySound( this );
					}

					if ( lpCurPlayer==this && (SkillEventDamageCount+1)<Shadow_Master_ShadowNum[point-1] ) {
						x = dm_SendTransDamage( chrAttackTarget , 0,0 , 0, 0, lpCurPlayer->AttackSkil , TRUE );
						if ( !x )	
							lpCurPlayer->AttackCritcal = -1;				//빗나감
						else
							lpCurPlayer->AttackCritcal = 0;

						SkillEventDamageCount++;
					}
					break;
				}
			}
		}
		break;

	//아처
	case SKILL_PLAY_RECALL_WOLVERIN:
		SkillPlaySound( SKILL_SOUND_SKILL_RECALL_WOLVERIN , pX,pY,pZ );
		break;

	case SKILL_PLAY_PHOENIX_SHOT:
		if ( chrAttackTarget && point ) {

			cnt = frame-MotionInfo->StartFrame*160;

			if ( MotionInfo->EventFrame[1]<=(DWORD)cnt ) {
				GetMoveLocation( 0 , 24*fONE, 24*fONE , 0, Angle.y , 0 );
				Pos1.x = pX+GeoResult_X;
				Pos1.y = pY+GeoResult_Y;
				Pos1.z = pZ+GeoResult_Z;
				Pos2.x = chrAttackTarget->pX;
				Pos2.y = chrAttackTarget->pY+24*fONE;
				Pos2.z = chrAttackTarget->pZ;
				SkillSagittarionPhoneixShot( this, &Pos1 , &Pos2 );

				//ChargingFlag

				if ( this==lpCurPlayer ) {

					SkillStopSound(SKILL_SOUND_SKILL_CHARGING);

					if ( ChargingFlag!=0 ) {
						dm_SendTransDamage( chrAttackTarget , 0,0 , 0, 0, lpCurPlayer->AttackSkil , FALSE );
					}
					else {
						rect.left = -50;
						rect.right = 50;
						rect.top = 0;
						rect.bottom=300;
						rect.bottom+=20;
						rect.bottom += 20;

						//범위형 공격 범위 선정 (박스)
						dm_SelectRangeBox( lpCurPlayer , &rect , FALSE );
						//범위공격 설정후 전송
						dm_SendRangeDamage( lpCurPlayer->pX, lpCurPlayer->pY, lpCurPlayer->pZ, 0 ,0, 0, 0, 0, lpCurPlayer->AttackSkil );
					}
					lpCurPlayer->MotionLoopSpeed =		GetAttackSpeedFrame( 7 );
					lpCurPlayer->MotionLoop = 1;

					MouseButton[1] = 0;
					CancelAttack();
				}
				SkillPlaySound( SKILL_SOUND_SKILL_PHOENIX_SHOT	 , pX,pY,pZ );

			}
			else {
				if ( this==lpCurPlayer ) ChargingFlag = 0;
			}
		}
		break;

	case SKILL_PLAY_FORCE_OF_NATURE:
 		if ( point ) {
			SkillSagittarionForceOfNature1( this , (float)Force_Of_Nature_Time[point-1] );
		}
		break;
	//나이트
	case SKILL_PLAY_DIVINE_PIERCING:
 		if ( chrAttackTarget && point ) {
			if ( AttackCritcal>=0 )	SkillSaintDivinePiercing( this );


			if ( Divine_Piercing_AttackNum[point-1]>=7 ) {
				dwEventFrameExt[0] = (81-10+MotionInfo->StartFrame)*160;
				dwEventFrameExt[1] = (102-10+MotionInfo->StartFrame)*160;
				dwEventFrameExt[2] = (120-10+MotionInfo->StartFrame)*160;
			}
			else if ( Divine_Piercing_AttackNum[point-1]>=5 ) {
				dwEventFrameExt[0] = (227-150+MotionInfo->StartFrame)*160;
			}


			if ( MotionEvent==Divine_Piercing_AttackNum[point-1] && AttackCritcal>=0 )
				SkillPlaySound( SKILL_SOUND_SKILL_DVINE_PIERCING	 , pX,pY,pZ );
			else
				WeaponPlaySound( this );


			if ( lpCurPlayer==this && MotionEvent<Divine_Piercing_AttackNum[point-1] ) {
				//temp = lpCurPlayer->smCharInfo.Attack_Rating;
				//lpCurPlayer->smCharInfo.Attack_Rating += Divine_Piercing_AddHit[point-1];				//명중력 보강
				cnt = dm_SendTransDamage( chrAttackTarget , 0,0 , 0, 0, lpCurPlayer->AttackSkil , TRUE );
				//lpCurPlayer->smCharInfo.Attack_Rating = temp;

				if ( !cnt )	
					lpCurPlayer->AttackCritcal = -1;				//빗나감
				else
					lpCurPlayer->AttackCritcal = 0;
			}

		}
		break;
	case SKILL_PLAY_GODLY_SHIELD:
 		if ( point ) {
			SkillSaintGodlyShield( this , (float)Godly_Shield_Time[point-1] );
		}
		break;
	case SKILL_PLAY_GODS_BLESS:
		break;

	case SKILL_PLAY_SWORD_OF_JUSTICE:
 		if ( point ) {
			SkillSwordOfUstice( this );
		}
		break;

	//아탈란타
	case SKILL_PLAY_HALL_OF_VALHALLA:
 		if ( point ) {
			SkillValhallaHallOfValhalla1( this );
			SkillValhallaHallOfValhallaHandEffect(this,(float)Hall_Of_Valhalla_Time[point-1]);
		}
		break;
	case SKILL_PLAY_X_RAGE:
		if ( chrAttackTarget && point ) {
			GetMoveLocation( 0 , 0, 24*fONE , 0, Angle.y , 0 );
			Pos1.x = pX+GeoResult_X;
			Pos1.y = pY+GeoResult_Y;
			Pos1.z = pZ+GeoResult_Z;
			Pos2.x = chrAttackTarget->pX;
			Pos2.y = chrAttackTarget->pY+40*fONE;
			Pos2.z = chrAttackTarget->pZ;
			SkillValhallaXRage( &Pos1 , &Pos2 );

			switch ( rand()%3 ) {
			case 0:
				SkillPlaySound( SKILL_SOUND_SKILL_EXTREM_RAGE1	 , pX,pY,pZ );
				break;
			case 1:
				SkillPlaySound( SKILL_SOUND_SKILL_EXTREM_RAGE2	 , pX,pY,pZ );
				break;
			case 2:
				SkillPlaySound( SKILL_SOUND_SKILL_EXTREM_RAGE3	 , pX,pY,pZ );
				break;
			}
		}
		break;
	case SKILL_PLAY_FROST_JAVELIN:
		if ( point ) {
			SkillParticle_FrostJavelin( this , (float)Frost_Javelin_Time[point-1] );
/*
			GetMoveLocation( 0 , 24*fONE, 24*fONE , 0, Angle.y , 0 );
			Pos1.x = pX+GeoResult_X;
			Pos1.y = pY+GeoResult_Y;
			Pos1.z = pZ+GeoResult_Z;
			Pos2.x = chrAttackTarget->pX;
			Pos2.y = chrAttackTarget->pY+40*fONE;
			Pos2.z = chrAttackTarget->pZ;
			SkillFrostJavelin( &Pos1 , &Pos2 );
*/
		}
		break;
	case SKILL_PLAY_VENGEANCE:
		if ( chrAttackTarget && point ) {
			GetMoveLocation( 0 , 24*fONE, 24*fONE , 0, Angle.y , 0 );
			Pos1.x = pX+GeoResult_X;
			Pos1.y = pY+GeoResult_Y;
			Pos1.z = pZ+GeoResult_Z;
			Pos2.x = chrAttackTarget->pX;
			Pos2.y = chrAttackTarget->pY+40*fONE;
			Pos2.z = chrAttackTarget->pZ;
			if ( AttackCritcal>=0 ) SkillVengeance( &Pos1 , &Pos2 );

			if ( this==lpCurPlayer && MotionEvent==1 ) {
				cnt = dm_SendTransDamage( chrAttackTarget , 0,0 , 0, 0, AttackSkil , TRUE );
				if ( !cnt )
					AttackCritcal = -1;				//빗나감
			}
			if ( MotionEvent==1 )
				SkillPlaySound( SKILL_SOUND_SKILL_VENGEANCE1	 , pX,pY,pZ );
			else
				SkillPlaySound( SKILL_SOUND_SKILL_VENGEANCE2	 , pX,pY,pZ );
		}
		break;

	//프리스티스
	case SKILL_PLAY_GLACIAL_SPIKE:
 		if ( point && MotionEvent==1 ) {
			SkillCelestialGlacialSpike( this );

			GetMoveLocation( 0 , 0, 64*fONE , 0, Angle.y , 0 );
			SetDynLight( pX+GeoResult_X,pY,pZ+GeoResult_Z, 0, 0,100,0, 700 );

			if ( this==lpCurPlayer ) {
				rect.left = -50;
				rect.right = 50;
				rect.top = 0;
				rect.bottom=300;
				rect.bottom+=20;
				rect.bottom += 20;

				//범위형 공격 범위 선정 (박스)
				dm_SelectRangeBox( lpCurPlayer , &rect , FALSE );
				//범위공격 설정후 전송
				dm_SendRangeDamage( lpCurPlayer->pX, lpCurPlayer->pY, lpCurPlayer->pZ, 0 ,
					cInvenTory.InvenItem[ sInven[0].ItemIndex-1 ].sItemInfo.Damage[0],
					cInvenTory.InvenItem[ sInven[0].ItemIndex-1 ].sItemInfo.Damage[1],
					0, 0, lpCurPlayer->AttackSkil );
			}

			if ( rand()%2 ) 
				SkillPlaySound( SKILL_SOUND_SKILL_GLACIAL_SPIKE1	 , pX,pY,pZ );
			else
				SkillPlaySound( SKILL_SOUND_SKILL_GLACIAL_SPIKE2	 , pX,pY,pZ );

		}
		break;
	case SKILL_PLAY_REGENERATION_FIELD:
 		if ( point  ) {
			SkillCelestialRegnerationField( this , 2.0f );

			if ( this==lpCurPlayer ) {
				//cnt = frame-MotionInfo->StartFrame*160;
				//if ( MotionInfo->EventFrame[1]<=(DWORD)cnt ) {
				if ( MouseButton[1] && cSkill.SearchContiueSkill(SKILL_REGENERATION_FIELD)  ) {
					//마우스 버튼 누르고 있으면 동작 반복
					frame = MotionInfo->StartFrame*160+MotionInfo->EventFrame[0];
				}
				else {
					cSkill.CancelContinueSkill( SKILL_REGENERATION_FIELD );
				}
			}
			else {
				cnt = Regeneration_Field_Area[point-1];
				cnt *= cnt;
				if ( GetDistanceDbl( pX , pZ , lpCurPlayer->pX , lpCurPlayer->pZ )<cnt && abs( lpCurPlayer->pY-pY )<16*fONE ) {
					cSkill.Regeneration_Field( 5 , point , 1 );		//재생력 추가
				}
			}
		}
		break;
	case SKILL_PLAY_CHAIN_LIGHTNING:
		if ( chrAttackTarget && point ) {

			//범위형 공격 범위 선정 (체인라이트닝)
			//dm_SelectDamageCount( this , chrAttackTarget , Chain_Lightning_Num[ point-1 ] , Chain_Lightning_Range[point-1] );
			dm_SelectDamageChainCount( this , chrAttackTarget , Chain_Lightning_Num[ point-1 ] , Chain_Lightning_Range[point-1] );

			if ( dmSelected_CharCnt>0 ) {
				if ( this==lpCurPlayer ) {
					//범위공격 설정후 전송
					dm_SendRangeDamage( pX,pY,pZ, 0 ,
						cInvenTory.InvenItem[ sInven[0].ItemIndex-1 ].sItemInfo.Damage[0],
						cInvenTory.InvenItem[ sInven[0].ItemIndex-1 ].sItemInfo.Damage[1],
						0, 0, AttackSkil );
				}

				SkillCelestialChainLighting( lpSelected_Char , dmSelected_CharCnt );
				
				if ( rand()%2 )
					SkillPlaySound( SKILL_SOUND_SKILL_CHAIN_LIGHTNING1	 , pX,pY,pZ );		//마법 소리
				else
					SkillPlaySound( SKILL_SOUND_SKILL_CHAIN_LIGHTNING2	 , pX,pY,pZ );		//마법 소리
			}
		}
		break;

	case SKILL_PLAY_SUMMON_MUSPELL:
 		if ( point ) {
			SkillCelestialMusPel(this,(float)Summon_Muspell_Time[point-1],(float)Summon_Muspell_Attack_Delay[point-1]);
			SkillPlaySound( SKILL_SOUND_SKILL_MUSPELL2	 , pX,pY,pZ );
		}
		break;

	//메지션
	case SKILL_PLAY_FIRE_ELEMENTAL:
 		if ( point ) {

		}
		break;
	case SKILL_PLAY_FLAME_WAVE:
		if ( point ) {
			SkillArchMageFlameWave(this);

			if ( rand()%2 )
				SkillPlaySound( SKILL_SOUND_SKILL_FLAME_WAVE1 , pX,pY,pZ );
			else
				SkillPlaySound( SKILL_SOUND_SKILL_FLAME_WAVE2 , pX,pY,pZ );

			if ( this==lpCurPlayer && MotionEvent==1 ) {

				rect.left = -60;
				rect.right = 60;
				rect.top = 0;
				rect.bottom=300;
				rect.bottom+=20;
				rect.bottom += 20;

				//범위형 공격 범위 선정 (박스)
				dm_SelectRangeBox( lpCurPlayer , &rect , FALSE );
				//범위공격 설정후 전송
				dm_SendRangeDamage( lpCurPlayer->pX, lpCurPlayer->pY, lpCurPlayer->pZ, 0 ,
					cInvenTory.InvenItem[ sInven[0].ItemIndex-1 ].sItemInfo.Damage[0],
					cInvenTory.InvenItem[ sInven[0].ItemIndex-1 ].sItemInfo.Damage[1],
					0, 0, lpCurPlayer->AttackSkil );

			}

		}
		break;
	case SKILL_PLAY_DISTORTION:
		if ( point ) {

		}
		break;

	case SKILL_PLAY_METEO:
		//메테오
		if ( chrAttackTarget && point ) {
			Pos2.x = chrAttackTarget->pX;
			Pos2.y = chrAttackTarget->pY;
			Pos2.z = chrAttackTarget->pZ;
			SkillArchMageMeteo(  &Pos2 );

			if ( this==lpCurPlayer && MotionEvent<3 ) {
				//범위형 공격 범위 선정 (원)
				dm_SelectRange( chrAttackTarget->pX, chrAttackTarget->pY, chrAttackTarget->pZ, Meteo_Area[point-1] , FALSE );
				//범위공격 설정후 전송
				dm_SendRangeDamage( chrAttackTarget->pX, chrAttackTarget->pY, chrAttackTarget->pZ,  0 , 
					cInvenTory.InvenItem[ sInven[0].ItemIndex-1 ].sItemInfo.Damage[0],
					cInvenTory.InvenItem[ sInven[0].ItemIndex-1 ].sItemInfo.Damage[1],
					0,0, lpCurPlayer->AttackSkil );
			}

			SkillPlaySound( SKILL_SOUND_SKILL_METEO1 , pX,pY,pZ );
		}
		break;
	}
	//★이름

	return TRUE;
}

/*
SkillPlaySound( SKILL_SOUND_SKILL_FIREBOLT1	 , x,y,z );
#define	SKILL_SOUND_SKILL_CASTING_MAGICIAN	0x1800
#define	SKILL_SOUND_SKILL_CASTING_PRIEST	0x1700

#define	SKILL_SOUND_SKILL_SWORDBLAST1		0x1510
#define	SKILL_SOUND_SKILL_SWORDBLAST2		0x1511
#define	SKILL_SOUND_SKILL_HOLYBODY			0x1520

#define	SKILL_SOUND_SKILL_SHIELDSTRIKE		0x1610
#define	SKILL_SOUND_SKILL_FARINA			0x1620
#define	SKILL_SOUND_SKILL_VIGORSPEAR		0x1640

#define	SKILL_SOUND_SKILL_HEALING			0x1710
#define	SKILL_SOUND_SKILL_HOLYBOLT			0x1720
#define	SKILL_SOUND_SKILL_MULTISPARK		0x1730
#define	SKILL_SOUND_SKILL_HOLYMIND			0x1740

#define	SKILL_SOUND_SKILL_AGONY				0x1810
#define	SKILL_SOUND_SKILL_FIREBOLT1			0x1820
#define	SKILL_SOUND_SKILL_FIREBOLT2			0x1821
#define	SKILL_SOUND_SKILL_ZENITH			0x1830
#define	SKILL_SOUND_SKILL_FIREBALL1			0x1840
#define	SKILL_SOUND_SKILL_FIREBALL2			0x1841
*/



//특정 상대에게 공격을 가한다 ( 상대방의 소켓값 또는 캐릭터 포인터 [둘중 선택] )
int smCHAR::SendTransAttack( smCHAR *lpChar , smWINSOCK *lpsmsock , int AttackCode , int Add_Damage ,int Resistance )
{
	TRANS_ATTACKDATA	TransAttackData;
	smWINSOCK	*lpsmSendSock;
	rsPLAYINFO *lpPlayInfo = 0;
	char szDispDamage[128];
	smTRANS_COMMAND	smTransCommand;

	int	pow;

	int temp;
	int	accuracy;

#ifndef _W_SERVER
	//클라이언트 로그 파일 기록
	Record_ClinetLogFile( "\261\270\271\366\300\374 \260\370\260\335\306\320\305\266 - ::SendTransAttack " );
	quit = 1;
	return TRUE;
#endif
/*
	if ( this==lpCurPlayer && 
		(abs(dwPlayTime-dwLastMouseMoveTime)>1024*60*3 ||
		 abs(dwPlayTime-dwLastCharMoveTime)>1024*60*3 || 
		 abs(dwFuncChkTime-dwPlayTime)>14*60*1024 ) ||
		 RestartPlayCount!=0 ) {
		//2~3분 이상 마우스 움직임 없을땐 공격 무효 ( 보츠 플레이 자동 방어 )
		//2~3분 이상 캐릭터 제자리에 서있을때 공격 무효 ( 보츠 플레이 자동 방어 )
		return FALSE;
	}
*/
/*
	if ( smConfig.DebugMode ) {
		//스킬 테스트
		if ( lpChar && VRKeyBuff[ VK_CONTROL ] && MouseButton[1] ) {
			//범위형 공격
			//BeginSkill( SKILL_PLAY_METEO , 0 , lpChar ,0,0,0 );
			AddChatBuff( "메테오 사용불가~ 메롱" );
			return TRUE;
		}
	}
*/

	if ( lpsmsock ) {
		lpPlayInfo = (rsPLAYINFO *)lpsmsock->ExtData1;
	}

	TransAttackData.code = smTRANSCODE_ATTACKDATA;
	TransAttackData.size = sizeof(TRANS_ATTACKDATA);

	TransAttackData.x = pX;
	TransAttackData.y = pY;
	TransAttackData.z = pZ;

	TransAttackData.AttackState = 1;
	TransAttackData.AttackSize  = 48*fONE;
	TransAttackData.Power		= GetRandomPos( smCharInfo.Attack_Damage[0] , smCharInfo.Attack_Damage[1] );

	TransAttackData.sRating[0] = smCharInfo.Level;
	TransAttackData.sRating[1] = smCharInfo.Attack_Rating;

	if ( PlayHolyMind[1]>0 )		//공격력 감소 - 홀리마인드
		TransAttackData.Power -= ((TransAttackData.Power*PlayHolyMind[0])/100);

	if ( !lpPlayInfo ) return FALSE;


	//종성 아이템 시간종료 확인
	if (  lpPlayInfo->dwSiegeItem_Scroll_Time && lpPlayInfo->dwSiegeItem_Scroll_Time<dwPlayServTime ) {
		lpPlayInfo->dwSiegeItem_Scroll_Code = 0;
		lpPlayInfo->dwSiegeItem_Scroll_Time = 0;
	}
	if (  lpPlayInfo->dwSiegeItem_Stone1_Time && lpPlayInfo->dwSiegeItem_Stone1_Time<dwPlayServTime ) {
		lpPlayInfo->dwSiegeItem_Stone1_Code = 0;
		lpPlayInfo->dwSiegeItem_Stone1_Time = 0;
	}

	if ( smCharInfo.dwCharSoundCode==snCHAR_SOUND_CASTLE_CRYSTAL_B ) {
		//공성 타워크리스탈 (아이스)
		TransAttackData.sRating[0] = -SKILL_PLAY_ICE_CRYSTAL;
		TransAttackData.sRating[1] = 3;
	}
	else if ( smCharInfo.dwCharSoundCode==snCHAR_SOUND_CASTLE_CRYSTAL_G ) 
	{
		//공성 타워크리스탈 (전기)
		TransAttackData.sRating[0] = -SKILL_PLAY_LIGHTNING_CRYSTAL;
		TransAttackData.sRating[1] = 2;
	}
	else if ( smCharInfo.dwCharSoundCode==snCHAR_SOUND_CASTLE_CRYSTAL_R ) {
		//공성 타워크리스탈 (불)
		TransAttackData.sRating[0] = -SKILL_PLAY_FIRE_CRYSTAL;
		TransAttackData.sRating[1] = 1;
	}

	if ( lpPlayInfo->dwSiegeItem_Stone1_Code && lpPlayInfo->dwSiegeItem_Stone1_Code==smCharInfo.dwCharSoundCode ) {
		//크리스탈 저항석
		TransAttackData.Power /= 2;		//공격력 절반
		TransAttackData.sRating[0] = smCharInfo.Level;
		TransAttackData.sRating[1] = smCharInfo.Attack_Rating;
	}

	if ( lpPlayInfo->dwSiegeItem_Scroll_Code==(sinBC1|sin01) ) {	//무적스크롤
		if ( lpPlayInfo->Position.Area==rsCASTLE_FIELD ) return FALSE;
	}
	if ( lpPlayInfo->dwSiegeItem_Scroll_Code==(sinBI1|sin05) )		//무적스크롤
		return FALSE;


	if ( lpPlayInfo->dwSiegeItem_Scroll_Code==(sinBC1|sin03) ) {	//추가 회피
		if ( lpPlayInfo->Position.Area==rsCASTLE_FIELD ) {
			if ( (rand()%100)<(int)lpPlayInfo->dwSiegeItem_Scroll_Param ) {
#ifdef DISP_DAMAGE
				if ( lpPlayInfo->AdminMode>2 ) {
					wsprintf( szDispDamage , ">%s \260\370\260\335 \303\337\260\241\310\270\307\307 ( Evade Scroll[%d] )" , smCharInfo.szName , lpPlayInfo->dwSiegeItem_Scroll_Param );
					rsDisplayDamgeToClient( lpPlayInfo , szDispDamage );
				}
#endif
				//추가회피 정보 클라리언트에 출력
				smTransCommand.code = smTRANSCODE_SUCCESS_EVATION;
				smTransCommand.size = sizeof( smTRANS_COMMAND );
				smTransCommand.WParam = 0;
				smTransCommand.LParam = lpPlayInfo->dwSiegeItem_Scroll_Code;
				smTransCommand.SParam = dwObjectSerial;
				smTransCommand.EParam = 0;
				lpPlayInfo->lpsmSock->Send( (char *)&smTransCommand , smTransCommand.size , TRUE );
				return FALSE;
			}
		}
	}

	if ( lpPlayInfo->dwSiegeItem_Scroll_Code==(sinBI1|sin07) ) {	//추가 회피
		if ( (rand()%100)<(int)lpPlayInfo->dwSiegeItem_Scroll_Param ) {
#ifdef DISP_DAMAGE
			if ( lpPlayInfo->AdminMode>2 ) {
				wsprintf( szDispDamage , ">%s \260\370\260\335 \303\337\260\241\310\270\307\307 ( Evade Scroll[%d] )" , smCharInfo.szName , lpPlayInfo->dwSiegeItem_Scroll_Param );
				rsDisplayDamgeToClient( lpPlayInfo , szDispDamage );
			}
#endif
			//추가회피 정보 클라리언트에 출력
			smTransCommand.code = smTRANSCODE_SUCCESS_EVATION;
			smTransCommand.size = sizeof( smTRANS_COMMAND );
			smTransCommand.WParam = 0;
			smTransCommand.LParam = lpPlayInfo->dwSiegeItem_Scroll_Code;
			smTransCommand.SParam = dwObjectSerial;
			smTransCommand.EParam = 0;
			lpPlayInfo->lpsmSock->Send( (char *)&smTransCommand , smTransCommand.size , TRUE );
			return FALSE;
		}
	}

	if ( lpPlayInfo->dwSkill_PhysicalAbsorb_Time ) {	//공격력 감소 (흡수력 미리적용)- 피지컬 사용
		if ( lpPlayInfo->dwSkill_PhysicalAbsorb_Time>dwPlayServTime ) {
			TransAttackData.Power -= lpPlayInfo->dwSkill_PhysicalAbsorb_Param;
			if ( TransAttackData.Power<0 ) return FALSE;
		}
		else {
			lpPlayInfo->dwSkill_PhysicalAbsorb_Time = 0;
			lpPlayInfo->dwSkill_PhysicalAbsorb_Param = 0;
		}
	}

	if ( lpPlayInfo->dwSkill_MetalArmor_Time ) {	//공격력 감소 (흡수력 미리적용)- 메탈아머 사용
		if ( lpPlayInfo->dwSkill_MetalArmor_Time>dwPlayServTime ) {
			TransAttackData.Power -= lpPlayInfo->dwSkill_MetalArmor_Param;
			if ( TransAttackData.Power<0 ) return FALSE;
		}
		else {
			lpPlayInfo->dwSkill_MetalArmor_Time = 0;
			lpPlayInfo->dwSkill_MetalArmor_Param = 0;
		}
	}


	if ( lpPlayInfo->dwSkill_HolyBody_Time ) {	//언데드 x% 공격력 감소 (흡수력 미리적용)- 홀리바디 사용
		if ( lpPlayInfo->dwSkill_HolyBody_Time>dwPlayServTime ) {
			if ( smCharInfo.Brood==smCHAR_MONSTER_UNDEAD ) {
				TransAttackData.Power -= (TransAttackData.Power*lpPlayInfo->dwSkill_HolyBody_Param)/100;
				if ( TransAttackData.Power<0 ) return FALSE;
			}
		}
		else {
			lpPlayInfo->dwSkill_HolyBody_Time = 0;
			lpPlayInfo->dwSkill_HolyBody_Param = 0;
		}
	}

	if ( lpPlayInfo->dwSkill_ExtreamShield_Time ) {	//블럭율증가 - 익스트림 쉴드
		if ( lpPlayInfo->dwSkill_ExtreamShield_Time>dwPlayServTime ) {
			TransAttackData.AttackState |= (lpPlayInfo->dwSkill_ExtreamShield_Param<<16);
			if ( TransAttackData.Power<0 ) return FALSE;
		}
		else {
			lpPlayInfo->dwSkill_ExtreamShield_Time = 0;
			lpPlayInfo->dwSkill_ExtreamShield_Param = 0;
		}
	}


	if ( lpPlayInfo->dwSkill_DivineInhalation_Time && smCharInfo.Brood==smCHAR_MONSTER_UNDEAD ) {	//블럭시 생명력
		if ( lpPlayInfo->dwSkill_DivineInhalation_Time>dwPlayServTime ) {

			temp = smCharInfo.Resistance[sITEMINFO_BIONIC];
			if ( temp ) {
				if ( temp>=100 ) temp=100;
				if ( temp<=-100 ) temp=-100;
				temp = ( D_Inhalation_Life[lpPlayInfo->dwSkill_DivineInhalation_Param]*temp)/100;
				temp &= 0xFF;
				temp |= D_Inhalation_Block[lpPlayInfo->dwSkill_DivineInhalation_Param]<<8;
				if ( temp ) {
					TransAttackData.AttackState |= (temp<<16);
				}
			}
			if ( TransAttackData.Power<0 ) return FALSE;
		}
		else {
			lpPlayInfo->dwSkill_DivineInhalation_Time = 0;
			lpPlayInfo->dwSkill_DivineInhalation_Param = 0;
		}
	}

	if ( lpPlayInfo->dwSkill_VirtualLife_Time ) {	//가상 생명력을 높여서 공격력 감소
		if ( lpPlayInfo->dwSkill_VirtualLife_Time>dwPlayServTime ) {
			TransAttackData.Power -= (TransAttackData.Power*lpPlayInfo->dwSkill_VirtualLife_Param)/100;
			if ( TransAttackData.Power<0 ) return FALSE;
		}
		else {
			lpPlayInfo->dwSkill_VirtualLife_Time = 0;
			lpPlayInfo->dwSkill_VirtualLife_Param = 0;
		}
	}

	if ( lpPlayInfo->dwSkill_EnergyShield_Time ) {	//에너지쉴드 공격력 감소
		if ( lpPlayInfo->dwSkill_EnergyShield_Time>dwPlayServTime ) {
			temp = (TransAttackData.Power*lpPlayInfo->dwSkill_EnergyShield_Param)/100;
			TransAttackData.Power -= temp;
			TransAttackData.AttackState |= (temp<<16);
			if ( TransAttackData.Power<0 ) return FALSE;
		}
		else {
			lpPlayInfo->dwSkill_EnergyShield_Time = 0;
			lpPlayInfo->dwSkill_EnergyShield_Param = 0;
		}
	}

	if ( lpPlayInfo->dwSkill_Compulsion_Time ) {	//Compulsion 흡수력 추가
		if ( lpPlayInfo->dwSkill_Compulsion_Time>dwPlayServTime ) {
			TransAttackData.Power -= lpPlayInfo->dwSkill_Compulsion_Param;
			if ( TransAttackData.Power<0 ) return FALSE;
		}
		else {
			lpPlayInfo->dwSkill_Compulsion_Time = 0;
			lpPlayInfo->dwSkill_Compulsion_Param = 0;
		}
	}

	if ( lpPlayInfo->dwSkill_Berserker_Time ) {	//버서커 흡수력 감소
		if ( lpPlayInfo->dwSkill_Berserker_Time>dwPlayServTime ) {
			TransAttackData.Power -= Berserker_SubAbsorb[lpPlayInfo->dwSkill_Berserker_Param];
			if ( TransAttackData.Power<0 ) return FALSE;
		}
		else {
			lpPlayInfo->dwSkill_Berserker_Time = 0;
			lpPlayInfo->dwSkill_Berserker_Param = 0;
		}
	}
	

	//베이그 추가회피
	if ( lpPlayInfo->dwSkill_Vague_Time ) {
		if ( lpPlayInfo->dwSkill_Vague_Time>dwPlayServTime ) {
			if ( (rand()%100)<Vague_EvasionPercent[lpPlayInfo->dwSkill_Vague_Param] ) {

#ifdef DISP_DAMAGE
				if ( lpPlayInfo->AdminMode>2 ) {
					wsprintf( szDispDamage , ">%s \260\370\260\335 \303\337\260\241\310\270\307\307 ( Vague[%d] )" , smCharInfo.szName , Vague_EvasionPercent[lpPlayInfo->dwSkill_Vague_Param] );
					rsDisplayDamgeToClient( lpPlayInfo , szDispDamage );
				}
#endif

				//추가회피 정보 클라리언트에 출력
				smTransCommand.code = smTRANSCODE_SUCCESS_EVATION;
				smTransCommand.size = sizeof( smTRANS_COMMAND );
				smTransCommand.WParam = 0;
				smTransCommand.LParam = SKILL_PLAY_VAGUE;
				smTransCommand.SParam = dwObjectSerial;
				smTransCommand.EParam = 0;
				lpPlayInfo->lpsmSock->Send( (char *)&smTransCommand , smTransCommand.size , TRUE );

				return FALSE;		//추가 회피 판정
			}
		}
		else {
			lpPlayInfo->dwSkill_Vague_Time = 0;
			lpPlayInfo->dwSkill_Vague_Param = 0;
		}
	}

	if ( lpPlayInfo->dwSkill_GodlyShied_Time ) {	//공격력 감소 (흡수력 미리적용)- 가들리쉴드사용
		if ( lpPlayInfo->dwSkill_GodlyShied_Time>dwPlayServTime ) {
			TransAttackData.Power -= (TransAttackData.Power*Godly_Shield_AbsorbPercent[lpPlayInfo->dwSkill_GodlyShied_Param])/100;
			if ( TransAttackData.Power<0 ) return FALSE;
		}
		else {
			lpPlayInfo->dwSkill_GodlyShied_Time = 0;
			lpPlayInfo->dwSkill_GodlyShied_Param = 0;
		}
	}


	//홀오브발할라 추가회피
	if ( lpPlayInfo->dwSkill_HallOfValhalla_Time ) {
		if ( lpPlayInfo->dwSkill_HallOfValhalla_Time>dwPlayServTime ) {
			if ( (rand()%100)<(int)lpPlayInfo->wSkill_HallOfValhalla_Param[0] ) {
#ifdef DISP_DAMAGE
				if ( lpPlayInfo->AdminMode>2 ) {
					wsprintf( szDispDamage , ">%s \260\370\260\335 \303\337\260\241\310\270\307\307 ( Hall of Valhalla [%d] )" , smCharInfo.szName , lpPlayInfo->wSkill_HallOfValhalla_Param[0] );
					rsDisplayDamgeToClient( lpPlayInfo , szDispDamage );
				}
#endif
				//추가회피 정보 클라리언트에 출력
				smTransCommand.code = smTRANSCODE_SUCCESS_EVATION;
				smTransCommand.size = sizeof( smTRANS_COMMAND );
				smTransCommand.WParam = 0;
				smTransCommand.LParam = SKILL_PLAY_HALL_OF_VALHALLA;
				smTransCommand.SParam = dwObjectSerial;
				smTransCommand.EParam = 0;
				lpPlayInfo->lpsmSock->Send( (char *)&smTransCommand , smTransCommand.size , TRUE );

				return FALSE;		//추가 회피 판정
			}
		}
		else {
			lpPlayInfo->dwSkill_HallOfValhalla_Time = 0;
		}
	}

	if ( lpPlayInfo->dwSkill_SummonMuspell_Time ) {	//서몬 무스펠
		if ( lpPlayInfo->dwSkill_SummonMuspell_Time>dwPlayServTime ) {

			if ( (rand()%100)<Summon_Muspell_BlockPercent[ lpPlayInfo->dwSkill_SummonMuspell_Param ] ) {
#ifdef DISP_DAMAGE
				if ( lpPlayInfo->AdminMode>2 ) {
					wsprintf( szDispDamage , ">%s \260\370\260\335 \303\337\260\241 \310\270\307\307 ( Summon Muspell [%d] )" , smCharInfo.szName , Summon_Muspell_BlockPercent[ lpPlayInfo->dwSkill_SummonMuspell_Param ] );
					rsDisplayDamgeToClient( lpPlayInfo , szDispDamage );
				}
#endif
				//추가회피 정보 클라리언트에 출력
				smTransCommand.code = smTRANSCODE_SUCCESS_EVATION;
				smTransCommand.size = sizeof( smTRANS_COMMAND );
				smTransCommand.WParam = 0;
				smTransCommand.LParam = SKILL_PLAY_SUMMON_MUSPELL;
				smTransCommand.SParam = dwObjectSerial;
				smTransCommand.EParam = 0;
				lpPlayInfo->lpsmSock->Send( (char *)&smTransCommand , smTransCommand.size , TRUE );

				return FALSE;
			}
			if ( smCharInfo.Brood==smCHAR_MONSTER_UNDEAD ) {
				//언데드 공격력 흡수
				temp = (TransAttackData.Power*Summon_Muspell_UndeadAbsorbPercent[lpPlayInfo->dwSkill_SummonMuspell_Param])/100;
				TransAttackData.AttackState |= (temp<<16);
				if ( TransAttackData.Power<0 ) return FALSE;
			}
		}
		else {
			lpPlayInfo->dwSkill_SummonMuspell_Time = 0;
			lpPlayInfo->dwSkill_SummonMuspell_Param = 0;
		}
	}


	if ( lpPlayInfo->dwBlessSkill_Code ) {
		if ( lpPlayInfo->dwBlessSkill_Code==SKILL_PLAY_BLESS_ABSORB ) {	//클랜스킬 (흡수)
			if ( lpPlayInfo->dwBlessSkill_Time>dwPlayServTime ) {
				TransAttackData.Power -= lpPlayInfo->dwBlessSkill_Param;
				if ( TransAttackData.Power<0 ) return FALSE;
			}
			else {
				lpPlayInfo->dwBlessSkill_Code = 0;
				lpPlayInfo->dwBlessSkill_Time = 0;
			}
		}
		else if ( lpPlayInfo->dwBlessSkill_Code==SKILL_PLAY_BLESS_EVADE ) 
		{	//클랜스킬 (이베이드)
			if ( lpPlayInfo->dwBlessSkill_Time>dwPlayServTime ) {

				if ( (rand()%100)<(int)lpPlayInfo->dwBlessSkill_Param ) {
#ifdef DISP_DAMAGE
					if ( lpPlayInfo->AdminMode>2 ) {
						wsprintf( szDispDamage , ">%s \260\370\260\335 \303\337\260\241 \310\270\307\307 ( Clan Skill [%d] )" , smCharInfo.szName , lpPlayInfo->dwBlessSkill_Param );
						rsDisplayDamgeToClient( lpPlayInfo , szDispDamage );
					}
#endif
					//추가회피 정보 클라리언트에 출력
					smTransCommand.code = smTRANSCODE_SUCCESS_EVATION;
					smTransCommand.size = sizeof( smTRANS_COMMAND );
					smTransCommand.WParam = 0;
					smTransCommand.LParam = SKILL_PLAY_BLESS_EVADE;
					smTransCommand.SParam = dwObjectSerial;
					smTransCommand.EParam = 0;
					lpPlayInfo->lpsmSock->Send( (char *)&smTransCommand , smTransCommand.size , TRUE );

					return FALSE;
				}
			}
			else {
				lpPlayInfo->dwBlessSkill_Code = 0;
				lpPlayInfo->dwBlessSkill_Time = 0;
			}
		}
	}



	//해당 플레이어 흡수력 저주 걸렸음 ( 몬스터 공격력 증가 )
	if ( lpPlayInfo->dwCurse_Defence_Time ) {
		if ( lpPlayInfo->dwCurse_Defence_Time>dwPlayServTime ) 
			TransAttackData.Power += lpPlayInfo->dwCurse_Defence_Param;
		else
			lpPlayInfo->dwCurse_Defence_Time = 0;
	}

	//공격력 강화
	if ( PlayHolyPower[1] ) {
		TransAttackData.Power += (TransAttackData.Power*PlayHolyPower[0])/100;
	}


	//추가 데미지
	if ( Add_Damage ) {
		TransAttackData.Power += (TransAttackData.Power*Add_Damage)/100;
	}

	if ( AttackCritcal ) {
		//크리티컬 힛 ( 공격 파워를 1.5배로 )
		TransAttackData.Power = (TransAttackData.Power*170)/100;
		TransAttackData.AttackState = 2;			//크리티컬 데미지
	}

	//공격 효과
	if ( AttackEffect )
		TransAttackData.AttackState = 2;

	if ( TransAttackData.Power<1 ) TransAttackData.Power = 1;

	if ( this==lpCurPlayer && smConfig.DebugMode && smConfig.szFile_Player[0] ) {
		//몬스터 스킬 공격력 업
		if ( VRKeyBuff[VK_CONTROL] )
			TransAttackData.Power = 200;
		else
			TransAttackData.Power = 30;
	}
	else {
		if ( lpChar ) {
			switch( lpChar->smCharInfo.State ) {
				case smCHAR_STATE_ENEMY:
					//명중률을 리턴한다 
					dwBattleTime = dwPlayTime;						//전투 시간 기록


					temp = smCharInfo.Attack_Rating;
					//스카우트호크 ( 명중율 증가 )
					if ( HoSkillCode ) {
						switch( HoSkillCode ) {
						case SKILL_SCOUT_HAWK:
							pow = GetSkillPower( this , lpChar , 0,0,0 );
							if ( pow ) {
								smCharInfo.Attack_Rating += (smCharInfo.Attack_Rating*pow)/100;
							}
							break;
						}
					}
					accuracy = sinGetAccuracy( lpChar->smCharInfo.Level , lpChar->smCharInfo.Defence );
					smCharInfo.Attack_Rating = temp;
					if ( (rand()%100)>accuracy ) 
						return FALSE;		//방어율 실패
					break;

				case smCHAR_STATE_NPC:
					TransAttackData.AttackState = Resistance;
					Resistance = 0;
					break;
			}
		}
	}

	if ( Resistance ) {
		TransAttackData.AttackState |= (Resistance<<16);
		pow = Resistance>>4;
		if ( pow>0 && pow<512 ) {
			TransAttackData.Power += pow;			//속성 공격력 추가 ( 서버에서 추가된 공격력 따로 계산 )
		}
	}

	LastAttackDamage = TransAttackData.Power;			//최신 공격 데미지 기록
	TransAttackData.dwDestObjectSerial = dwObjectSerial;
	TransAttackData.dwTarObjectSerial = 0;
	TransAttackData.dwChkSum = TransAttackData.Power*2002 + (TransAttackData.x * TransAttackData.y * TransAttackData.z);// 조작 방지 코드
	lpsmSendSock = lpsmsock;


	if ( lpChar ) {
		if ( lpChar->TransSock )
			lpsmSendSock = lpChar->TransSock;

		if ( lpChar->ServerCode ) {
			lpsmSendSock = GetServerSock(lpChar->ServerCode);		//서버 코드 구하기
		}

		TransAttackData.dwTarObjectSerial = lpChar->dwObjectSerial;
		
		//처음 공격하는 캐릭한테만 체력바 출력
		if ( lpChar->smCharInfo.State==smCHAR_STATE_ENEMY && lpChar->smCharInfo.Life[1] && lpChar->smCharInfo.Life[0]==lpChar->smCharInfo.Life[1] ) {
			lpChar->EnableStateBar = TRUE;
		}
	}


	if ( lpsmSendSock ) {
		if ( !ServerMode ) {
			//보낸 공격력 기록
			Record_SendDamage( lpsmSendSock , TransAttackData.Power );
		}

		pow = TransAttackData.Power;

		if ( lpsmsock ) {
			if ( rsServerConfig.CrazyPacketMode && lpPlayInfo && lpPlayInfo->dwDecPacketCode &&
				lpPlayInfo->dwDecPacketTime2<dwPlayServTime ) {					//크레이지 패킷 모드

				//S2C 공격 첵크섬 ( 크레이지 패킷 )
				TransAttackData.code = smTRANSCODE_ATTACKDATA2;
				TransAttackData.dwDamageChkSum = dm_GetDamgeChkSum_S2V( &TransAttackData );
				rsEncodeDamagePacket( lpPlayInfo , &TransAttackData );
			}
			else
				TransAttackData.dwDamageChkSum = dm_GetDamgeChkSum_S2V( &TransAttackData );
		}

		if ( lpsmSendSock->Send2( (char *)&TransAttackData , TransAttackData.size , TRUE ) ) {
			return pow;
		}
	}


	return FALSE;
}



//공격 사정거리에 들어왔으면 공격 ( 좌표 와 사정거리 )
int smCHAR::PlayAttackFromPosi( int ex, int ey, int ez, int Dist , int attack )
{
	int AtDist;
	int	dDist;
	int ay;

	ay = abs(pY-ey);

	if ( OnStageField>=0 && StageField[ OnStageField ]->FieldCode==rsCASTLE_FIELD ) {
		//공성전 필드 공격 설정 보정
		if ( ShootingMode ) {
			if ( ay>80*fONE ) return NULL;
		}
		else {
			if ( ay>52*fONE ) return NULL;
		}

		//성문 경유해서 공격 금지
		if ( (pZ<CastleBattleZone_DoorLineZ && ez>CastleBattleZone_DoorLineZ) ||
			(pZ>CastleBattleZone_DoorLineZ && ez<CastleBattleZone_DoorLineZ) )
		{
			if ( rsBlessCastle.CastleMode && rsBlessCastle.DefenceLevel==0 ) {
				return NULL;
			}
		}
	}
	else
		if ( ay>64*fONE ) return NULL;

	AtDist = GetDistanceDbl( pX , pZ , ex, ez );
	dDist = (Dist>>FLOATNS);
	dDist *= dDist;

	if ( AtDist<dDist ) {
		if ( MotionInfo->State!=CHRMOTION_STATE_ATTACK &&
			MotionInfo->State!=CHRMOTION_STATE_SKILL && MotionInfo->State!=CHRMOTION_STATE_DEAD ) {

			if ( attack ) {
				//마을에선 공격 불가
				if ( smCharInfo.State!=smCHAR_STATE_USER || (OnStageField>=0 && StageField[ OnStageField ]->State!=FIELD_STATE_VILLAGE) ) {	
					SetMotionFromCode( CHRMOTION_STATE_ATTACK );
					Angle.y = GetRadian2D( pX,pZ , ex,ez );
					AttackX = ex;
					AttackY = ey;
					AttackZ = ez;
				}
			}

			return TRUE;
		}
	}

	return NULL;
}



//모션부려 초기화
int InitMotionBlur()
{
	int cnt;
	int TlCnt;
	float fVCnt,fVCnt2;
	float fBlurLevel;
	int MbCnt,MvCnt;
	

	MatMBlur =  CreateTextureMaterial( "image\\MotionMask.bmp" ,NULL , TRUE , FALSE ,FALSE  , SMMAT_BLEND_ADDCOLOR );//ALPHA );

	///////////////////////// 32단계 /////////////////////////

	fBlurLevel = 1.0f/32;
	TlCnt = 0;
	fVCnt = 0;
	fVCnt2 = 0+ fBlurLevel;

	for( cnt=0; cnt<32; cnt++ ) {

		smTexLinkMotionBlur[TlCnt].NextTex  = 0;
		smTexLinkMotionBlur[TlCnt].hTexture = 0;
		smTexLinkMotionBlur[TlCnt].v[0] = fVCnt;
		smTexLinkMotionBlur[TlCnt].v[1] = fVCnt;
		smTexLinkMotionBlur[TlCnt].v[2] = fVCnt2;
		smTexLinkMotionBlur[TlCnt].u[0] = 0;
		smTexLinkMotionBlur[TlCnt].u[1] = 1;
		smTexLinkMotionBlur[TlCnt].u[2] = 0;

		TlCnt++;

		smTexLinkMotionBlur[TlCnt].NextTex  = 0;
		smTexLinkMotionBlur[TlCnt].hTexture = 0;
		smTexLinkMotionBlur[TlCnt].v[0] = fVCnt2;
		smTexLinkMotionBlur[TlCnt].v[1] = fVCnt;
		smTexLinkMotionBlur[TlCnt].v[2] = fVCnt2;
		smTexLinkMotionBlur[TlCnt].u[0] = 0;
		smTexLinkMotionBlur[TlCnt].u[1] = 1;
		smTexLinkMotionBlur[TlCnt].u[2] = 1;

		TlCnt++;

		fVCnt = fVCnt2;
		fVCnt2 += fBlurLevel;

	}

	MbCnt = 0;
	MvCnt = 0;

	ZeroMemory( MbVertex , sizeof( smVERTEX ) * 64 );

	for( cnt=0; cnt<32; cnt++) {

		MbFace[ MbCnt ].lpTexLink = &smTexLinkMotionBlur[MbCnt];
		MbFace[ MbCnt ].v[0] = MvCnt;
		MbFace[ MbCnt ].v[1] = MvCnt+1;
		MbFace[ MbCnt ].v[2] = MvCnt+2;
		MbFace[ MbCnt ].v[3] = MatMBlur;
		MbCnt++;

		MbFace[ MbCnt ].lpTexLink = &smTexLinkMotionBlur[MbCnt];
		MbFace[ MbCnt ].v[0] = MvCnt+2;
		MbFace[ MbCnt ].v[1] = MvCnt+1;
		MbFace[ MbCnt ].v[2] = MvCnt+3;
		MbFace[ MbCnt ].v[3] = MatMBlur;
		MbCnt++;
		MvCnt+=2;

	}


	///////////////////////// 16단계 /////////////////////////


	fBlurLevel = 1.0f/16;
	TlCnt = 0;
	fVCnt = 0;
	fVCnt2 = 0+ fBlurLevel;

	for( cnt=0; cnt<16; cnt++ ) {

		smTexLinkMotionBlur2[TlCnt].NextTex  = 0;
		smTexLinkMotionBlur2[TlCnt].hTexture = 0;
		smTexLinkMotionBlur2[TlCnt].v[0] = fVCnt;
		smTexLinkMotionBlur2[TlCnt].v[1] = fVCnt;
		smTexLinkMotionBlur2[TlCnt].v[2] = fVCnt2;
		smTexLinkMotionBlur2[TlCnt].u[0] = 0;
		smTexLinkMotionBlur2[TlCnt].u[1] = 1;
		smTexLinkMotionBlur2[TlCnt].u[2] = 0;

		TlCnt++;

		smTexLinkMotionBlur2[TlCnt].NextTex  = 0;
		smTexLinkMotionBlur2[TlCnt].hTexture = 0;
		smTexLinkMotionBlur2[TlCnt].v[0] = fVCnt2;
		smTexLinkMotionBlur2[TlCnt].v[1] = fVCnt;
		smTexLinkMotionBlur2[TlCnt].v[2] = fVCnt2;
		smTexLinkMotionBlur2[TlCnt].u[0] = 0;
		smTexLinkMotionBlur2[TlCnt].u[1] = 1;
		smTexLinkMotionBlur2[TlCnt].u[2] = 1;

		TlCnt++;

		fVCnt = fVCnt2;
		fVCnt2 += fBlurLevel;

	}

	MbCnt = 0;
	MvCnt = 0;

	ZeroMemory( MbVertex , sizeof( smVERTEX ) * 32 );

	for( cnt=0; cnt<16; cnt++) {

		MbFace2[ MbCnt ].lpTexLink = &smTexLinkMotionBlur2[MbCnt];
		MbFace2[ MbCnt ].v[0] = MvCnt;
		MbFace2[ MbCnt ].v[1] = MvCnt+1;
		MbFace2[ MbCnt ].v[2] = MvCnt+2;
		MbFace2[ MbCnt ].v[3] = MatMBlur;
		MbCnt++;

		MbFace2[ MbCnt ].lpTexLink = &smTexLinkMotionBlur2[MbCnt];
		MbFace2[ MbCnt ].v[0] = MvCnt+2;
		MbFace2[ MbCnt ].v[1] = MvCnt+1;
		MbFace2[ MbCnt ].v[2] = MvCnt+3;
		MbFace2[ MbCnt ].v[3] = MatMBlur;
		MbCnt++;
		MvCnt+=2;

	}

	return TRUE;
}








int GetProjPosi( int &x3d , int &y3d, int &z3d , POINT3D *CameraPosi , POINT3D *CameraAngle )
{

	int x2d,y2d;
	smMATRIX	trans,rX,rY,rZ,rm;
//	smPOINT3D	*pV;
	int sx,sy,sz;
	int cx,cy,cz;

	cx = x3d - CameraPosi->x;
	cy = y3d - CameraPosi->y;
	cz = z3d - CameraPosi->z;

	smIdentityMatrix( rX );
	smIdentityMatrix( rY );
	smIdentityMatrix( rZ );
	smIdentityMatrix( trans );
	smZeroMatrix( rm );

	smRotateXMatrix( rX , (-CameraAngle->x)&ANGCLIP );
	smRotateYMatrix( rY , (-CameraAngle->y)&ANGCLIP );
	smRotateZMatrix( rZ , (-CameraAngle->z)&ANGCLIP );
	smTranslateMatrix( trans , cx,cy,cz  );

	smMatrixMult( rm , trans, rY );
	smMatrixMult( rm , rm, rX );
	smMatrixMult( rm , rm, rZ );


		sx = x3d * rm._11 +
			 y3d * rm._21 +
			 z3d * rm._31;


		sy = x3d * rm._12 +
			 y3d * rm._22 +
			 z3d * rm._32 ;

		sz = x3d * rm._13 +
			 y3d * rm._23 +
			 z3d * rm._33 ;

		x3d = (sx>>FLOATNS)+ rm._41 ;
		y3d = (sy>>FLOATNS)+ rm._42 ;
		z3d = (sz>>FLOATNS)+ rm._43 ;


    if (z3d > 0)
    {
		x3d = MidX + ((x3d * viewdistZ) / z3d);
		y3d = MidY - ((y3d * viewdistZ) / z3d);
		z3d >>=FLOATNS;
		return TRUE;

    }
    else
    {
		x2d = CLIP_OUT;
		y2d = CLIP_OUT;
		return FALSE;
    }

}




scITEM::scITEM()
{
	Init();
}

scITEM::~scITEM()
{
	Close();
}

void scITEM::Init()
{
	ZeroMemory( &Head , sizeof( scITEM ) );
}

int scITEM::Close()
{

	Flag = 0;

	if ( lpDinaPattern ) {
		AddRemoveDynPattern( lpDinaPattern );
		//smDPAT_Delete( lpDinaPattern );
		lpDinaPattern = 0;
	}

	Pattern = 0;

	return TRUE;
}

//패턴설정
int scITEM::SetPattern( smPAT3D *pat )
{
	Pattern = pat;
	return TRUE;
}


//위치설정
int scITEM::SetPosi( int x, int y, int z )
{
//	int py1;
	int py2 = 0;

	pX = x;
	pZ = z;
	pY = y;

	if ( pY==0 ) {

		if ( smGameStage[0] )
			pY = smGameStage[0]->GetHeight( pX , pZ );

		if ( smGameStage[1] )
			py2 = smGameStage[1]->GetHeight( pX , pZ );

		if ( py2>pY ) pY = py2;
	}

	return TRUE;
}


//수신된 아이템 설정
int scITEM::SetTransItem( TRANS_ITEM *lpTransItem )
{
	dwLastTransTime = dwPlayTime;

//	state = lpTransItem->state

	Flag = TRUE;

	lstrcpy( szName , lpTransItem->szName );
	//lstrcpy( szModelName , lpTransItem->szModelName );
	SetPosi( lpTransItem->x, lpTransItem->y , lpTransItem->z );
	lpStgArea = lpTransItem->lpStgArea;

	ItemCode = lpTransItem->ItemCode;


	Angle.y = ((pX+pZ)>>2)&ANGCLIP;

	if ( sinGetItemInfo( ItemCode , szModelName )==FALSE || ItemCode==0 ) {
		lstrcpy( szModelName , "char\\flag\\flag.ini" );
	}

/*
	switch ( ItemCode ) {
	case 0:
		lstrcpy( szModelName , "char\\flag\\flag.ini" );
		break;
	case 1:
		lstrcpy( szModelName , "DropItem\\DRcoin.ASE" );
		break;
	case 2:
		lstrcpy( szModelName , "weapons\\Weapon_low\\Iw07.ASE" );
		break;
	}
*/

	if ( !PatLoading )
		AddLoaderItem( this , szModelName );

	return TRUE;
}




#define ITEM_TRANS_LIMIT_TIME		10000

int scITEM::Draw()
{

	POINT3D CameraPosi , CameraAngle;
	POINT3D angle;
//	smPAT3D *pat;
	int r,g,b;
	int scale;

	if( ItemCode==0 && !DisplayDebug ) return FALSE;

	//패턴 유무 체크 ( 통신시 파일 모델수신이 안된경우 )
	if ( !DisplayFlag || !Pattern ) return FALSE;

	//요구된 업데이트 시간 초과시 아이템 제거
	if ( dwPlayTime>(dwLastTransTime+ITEM_TRANS_LIMIT_TIME) ) {
		Close();
		return FALSE;
	}

	CameraPosi.x = smCHAR_CameraX;
	CameraPosi.y = smCHAR_CameraY;
	CameraPosi.z = smCHAR_CameraZ;

	CameraAngle.x = smCHAR_CameraAngX;
	CameraAngle.y = smCHAR_CameraAngY;
	CameraAngle.z = smCHAR_CameraAngZ;

	memcpy( &angle, &Angle , sizeof( POINT3D ) );

	Posi.x = pX;
	Posi.y = pY+6*fONE;
	Posi.z = pZ;
	
	int	BlinkFlag;

	BlinkFlag = FALSE;

	if ( (ItemCode&sinITEM_MASK1)==sinITEM_WEAPON )	{
		angle.x = ANGLE_90;
	}

	if ( ItemCode && (dwPlayTime & 0x80) )
		BlinkFlag = TRUE;


	//SOD 필드의 특정 아이템의 경우 크기변형으로 강조
	if ( lpCurPlayer->OnStageField>=0 && StageField[ lpCurPlayer->OnStageField ]->FieldCode==rsSOD_FIELD ) {
		if ( (ItemCode&sinITEM_MASK2)==sinPM1 || (ItemCode&sinITEM_MASK2)==sinSD2 ) {
			
			scale = (dwPlayTime>>2)&0x1FF;

			if ( (scale&0x100)!=0 ) {
				scale = fONE-(scale&0xFF);
			}
			if ( (ItemCode&sinITEM_MASK2)==sinPM1 )
				scale *= 3;

			smRender.ObjectVertexScale = scale+fONE;
		}
	}



	if ( BlinkFlag ) {

		r = smRender.Color_R;
		g = smRender.Color_G;
		b = smRender.Color_B;

		smRender.Color_R +=100;
		smRender.Color_G +=100;
		smRender.Color_B +=100;

		Pattern->SetPosi( &Posi , &angle );
		Pattern->Frame = 0;
		Pattern->RenderD3D( &CameraPosi , &CameraAngle );

		smRender.Color_R =r;
		smRender.Color_G =g;
		smRender.Color_B =b;
	}
	else {
		Pattern->SetPosi( &Posi , &angle );
		Pattern->Frame = 0;
		Pattern->RenderD3D( &CameraPosi , &CameraAngle );
	}

	smRender.ObjectVertexScale = 0;

	return TRUE;
}


//////////////////////////////// 무적모드 악용 대비 감시함수 ////////////////////////////

DWORD	Ptect_RestartCounter = 0;
DWORD	Ptect_RestartMask = 0;

//공격 마크 보안 카운터 증가
int	Ptect_IncAttackCount( int Flag )
{
	DWORD	rc;
	smTRANS_COMMAND	smTransCommand;

	rc = Ptect_RestartCounter^Ptect_RestartMask;
	Ptect_RestartMask = dwPlayTime;

	switch( Flag ) {
		case -1:
			rc = 0;
			break;
		case 0:
			rc ++;
			break;
	}

	if ( rc>160 ) {
		//서버에 경고 보냄
		smTransCommand.code = smTRANSCODE_WARNING_BLINK_ATT;
		smTransCommand.size = sizeof(smTRANS_COMMAND);
		smTransCommand.WParam = rc;
		smTransCommand.LParam = 0;
		smTransCommand.SParam = 0;
		smTransCommand.EParam = 0;
		if ( smWsockDataServer ) smWsockDataServer->Send2( (char *)&smTransCommand , smTransCommand.size , TRUE );
		rc = 0;
	}

	rc = rc^Ptect_RestartMask;
	Ptect_RestartCounter = rc;

	return TRUE;
}

//공격 마크 보안 카운터 얻기
DWORD Ptect_GetAttackCount()
{
	DWORD	rc;
	rc = Ptect_RestartCounter^Ptect_RestartMask;
	Ptect_RestartMask = dwPlayTime;
	Ptect_RestartCounter = rc^Ptect_RestartMask;

	return rc;
}



//캐슬 마스터 업데이트
int UpdateCastleMasterClan( DWORD dwMasterClan )
{
	int cnt;

	SetClanMaster_Player(dwMasterClan);

	for( cnt=0;cnt<OTHER_PLAYER_MAX;cnt++ ) {
		if ( chrOtherPlayer[cnt].Flag && chrOtherPlayer[cnt].dwObjectSerial ) {
			if ( chrOtherPlayer[cnt].smCharInfo.ClassClan && GetClanCode(chrOtherPlayer[cnt].smCharInfo.ClassClan)==dwMasterClan )
				lpCurPlayer->Clan_CastleMasterFlag = TRUE;
			else
				lpCurPlayer->Clan_CastleMasterFlag = FALSE;
		}
	}

	return TRUE;
}

//플레이어 클랜 마스터 확인
int SetClanMaster_Player( DWORD dwMasterClan )
{
	if ( lpCurPlayer->smCharInfo.ClassClan && GetClanCode(lpCurPlayer->smCharInfo.ClassClan)==dwMasterClan )
		lpCurPlayer->Clan_CastleMasterFlag = TRUE;
	else
		lpCurPlayer->Clan_CastleMasterFlag = FALSE;

	return lpCurPlayer->Clan_CastleMasterFlag;

}