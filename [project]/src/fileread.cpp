#include <stddef.h>
#include "smlib3d\\smd3d.h"
#include "smwsock.h"
#include "character.h"
#include "fileread.h"
#include "field.h"
#include "effectsnd.h"
#include "smreg.h"
#include "playmain.h"
#include "playsub.h"
#include "language.h"

#include "SrcServer\\gamesql.h"
#include "SrcServer\\onserver.h"

// 석지용 - 믹스쳐 리셋 (무특화 아이템을 위해 전역을 하나 만든다 ㅠ_ㅠ)
int	NotItemSpecial = 0;

extern	int	quit;

static char decode[512];
static char line[512];

//######################################################################################
//작 성 자 : 오 영 석
WORD GetMotionItemIndexOfSlot( int ItemSlot );		// character.cpp
void ModelKeyWordEncode( DWORD dwFileCode, smMODELINFO *lpMODELINFO );
int  ModelKeyWordDecode( DWORD dwFileCode, smMODELINFO *lpMODELINFO );
void MotionKeyWordEncode( DWORD dwCode, smMODELINFO *lpMODELINFO );
int	 MotionKeyWordDecode( DWORD dwCode, smMODELINFO *lpMODELINFO );
//######################################################################################


//서버용 NPC 메세지 카운터
#ifdef	_W_SERVER
#define	SVR_NPC_MSG_MAX		1024
char	szSvr_NpcMsgs[SVR_NPC_MSG_MAX][256];
DWORD	dwSvr_NpcMsgCode[SVR_NPC_MSG_MAX];
#else
#define	SVR_NPC_MSG_MAX		32
char	szSvr_NpcMsgs[SVR_NPC_MSG_MAX][128];
DWORD	dwSvr_NpcMsgCode[SVR_NPC_MSG_MAX];
#endif

int		NpcMsgCount = 0;

//단축 글자
char	szShotCutMessage[10][100];



//NPC 용 대사 데이타 버퍼에 저장후 포인터 반환
char	*SetNPCMsg( char *szMsg )
{
	char	*lpMsg;
	DWORD	dwCode;
	int		cnt;

	if ( NpcMsgCount>=SVR_NPC_MSG_MAX ) return NULL;

	dwCode = GetSpeedSum(szMsg);

	for(cnt=0;cnt<NpcMsgCount;cnt++) {
		if ( dwCode==dwSvr_NpcMsgCode[cnt] && lstrcmp( szSvr_NpcMsgs[cnt] , szMsg )==0 ) {
			break;
		}
	}

	if ( cnt<NpcMsgCount ) {
		return szSvr_NpcMsgs[cnt];
	}

	dwSvr_NpcMsgCode[NpcMsgCount] = dwCode;
	lpMsg = szSvr_NpcMsgs[NpcMsgCount++];
	lstrcpy( lpMsg , szMsg );

	return lpMsg;
}


static char *fileread_GetWord(char *q , char *p)
{

//	while ( *p != '\n' ) {
		while ( (*p == 32) || (*p == 9) ) // SPACE or TAB or ':'는 제외 시킴
		{
			p++;
		}
	
		while ( (*p != 32) && (*p != 9) ) // SPACE or TAB
		{
			if ( *p == '\n' || *p=='\r' ) break;
			*q++ = *p++;
		}
		*q++ = 0;	// end of one string, put Null character
//	}
//	*q = '\n';

	return p;
}


char *szIniCommand[] = {
	"*\265\277\300\333\306\304\300\317",
	"*\265\277\300\333\270\360\300\275",
	"*\270\360\276\347\306\304\300\317",
	"*\270\360\276\347\300\314\270\247",
	"*\306\304\300\317\277\254\260\341",
	"*\272\270\301\266\265\277\300\333\306\304\300\317",

	"*\301\244\271\320\270\360\276\347",
	"*\272\270\305\353\270\360\276\347",
	"*\300\372\301\372\270\360\276\347",

	"*XX\260\310\264\302\265\277\300\333",
	"*XX\274\255\300\326\261\342\265\277\300\333",

	"*\302\370\277\353\271\253\261\342",
	"*\300\373\277\353\301\367\276\367",
	"*\307\330\264\347\300\247\304\241",
	"*\300\373\277\353\261\342\274\372",

	//######################################################################################
	//작 성 자 : 오 영 석
	"*\307\245\301\244\306\304\300\317\277\254\260\341",
	"*\307\245\301\244\306\304\300\317",
	"*\307\245\301\244\270\360\300\275",
	//######################################################################################

	""
};

char *szIniRepeat = "\271\335\272\271";
char *szMotionWord = "\265\277\300\333";

char *szRunWord = "\266\331\264\302";
char *szStandWord = "*\274\255\300\326\261\342";
char *szStandWord2 = "*\301\244\301\366";
char *szWalkWord = "*\260\310\264\302";

char *szAttackWord = "\260\370\260\335";
char *szDeadWord = "\301\327\261\342";
char *szDeadWord2 = "\301\327\264\302";
char *szDamageWord = "\305\270\260\335";
char *szDamageWord2 = "\265\245\271\314\301\366";
char *szDamageWord3 = "\270\267\261\342";

char *szWarpWord = "\277\357\261\342";
char *szWarpWord2 = "\277\366\307\301";

char *szSometimeWord = "\260\241\262\373";

char *szFallDownWord = "\266\263\276\356";
char *szFallStandLargeWord = "\305\253\302\370\301\366";
char *szFallStandSmallWord = "\300\333\300\272\302\370\301\366";

char *szRestartWord = "\264\331\275\303\275\303\300\333";
char *szEatWord = "\270\324\261\342";
char *szSkillWord = "\261\342\274\372";
char *szYahooWord = "\270\270\274\274";

//######################################################################################
//작 성 자 : 오 영 석
char *szMotionTalkWord = "\307\245\301\244";

char *szArTalkWord = "\276\306\307\245\301\244";
char *szETalkWord = "\300\314\307\245\301\244";
char *szOhTalkWord = "\277\300\307\245\301\244";
char *szEyeTalkWord = "\264\253\261\364\272\375\307\245\301\244";

char *szBlankTalkWord = "\271\253\307\245\301\244(\300\333\265\277\276\312\307\324)";
char *szSmileTalkWord = "\277\364\264\302\307\245\301\244";
char *szGrumbleTalkWord = "\310\255\263\255\307\245\301\244";
char *szSorrowTalkWord = "\275\275\307\302\307\245\301\244";
char *szStartledTalkWord = "\263\356\266\365\307\245\301\244";
char *szNatureTalkWord = "\260\355\300\257\307\245\301\244";
char *szSpecialTalkWord = "\306\257\272\260\307\245\301\244";
//######################################################################################


JOBNAME_BIT BitMaskJobNames[] = {
	{	"Fighter"		,	0x0001	},
	{	"Mechanician"	,	0x0002	},
	{	"Archer"		,	0x0004	},
	{	"Pikeman"		,	0x0008	},

	{	"Atalanta"		,	0x0010	},
	{	"Knight"		,	0x0020	},
	{	"Magician"		,	0x0040	},
	{	"Priest"		,	0x0080	},

	// the later classes (JOB_CODE 9..11): their motion tables' bits (M6Bip/M7Bip/M8Bip)
	{	"Assassin"		,	0x0100	},
	{	"Shaman"		,	0x0200	},
	{	"MartialArtist"	,	0x0400	},

	{	"" , 0 },

	{	"PureBody"		,	0x0040	},
	{	"PureBody"		,	0x0080	},
	{	"PureBody"		,	0x0100	},
	{	"PureBody"		,	0x0200	},
	{	"PureBody"		,	0x0400	},
	{	"PureBody"		,	0x0800	},
	{	"PureBody"		,	0x1000	}
};


//######################################################################################
//작 성 자 : 오 영 석
//모델정모 동작 설정
int SetIniMotionInfo( smMOTIONINFO *motion, char *p, int *lpRate = NULL )
//######################################################################################
{
	char strBuff[256];
	int EventCount;

	EventCount = 0;

	//*걷는동작
	p = fileread_GetWord( strBuff , p );
	motion->StartFrame = atoi( strBuff );
	p = fileread_GetWord( strBuff , p );
	motion->EndFrame   = atoi( strBuff );

	motion->EventFrame[0] = 0;
	motion->EventFrame[1] = 0;
	motion->EventFrame[2] = 0;
	motion->EventFrame[3] = 0;

	while(1) {
		p = fileread_GetWord( strBuff , p );
		if ( strBuff[0]<'0' || strBuff[0]>'9' ) break;
		if ( EventCount>=4 ) break;

		motion->EventFrame[EventCount] = (atoi( strBuff )-motion->StartFrame) * 160;
		EventCount++;
	}

	//######################################################################################
	//작 성 자 : 오 영 석
	if( lstrcmp( strBuff, szIniRepeat ) == 0 )
	{
		motion->Repeat = TRUE;
	}
	//######################################################################################

	//######################################################################################
	//작 성 자 : 오 영 석
	if( strBuff[0] != '(' && strBuff[0] >= ' ' && strBuff[0] <= 0x7F ) 
		motion->KeyCode = strBuff[0];

	if( lpRate )
	{
		p = fileread_GetWord( strBuff, p );
		
		int  len = strlen(strBuff);
		char NumBuff[10] = {0,};

		if( len > 2 && len < 6 && strBuff[0] == '(' && strBuff[len-1] == ')'  )
		{
			::CopyMemory( NumBuff, &strBuff[1], len-2 );
			*lpRate = atoi( NumBuff );
			if( *lpRate > 100 )
				*lpRate = 100;
		}
	}
	//######################################################################################

	return TRUE;
}

static char *szFileExt = "inx";


//모델 정보 파일을 (Info에저장)
//######################################################################################
//작 성 자 : 오 영 석
int AddModelDecode( char *iniFile, smMODELINFO *Info )
//######################################################################################
{
	FILE	*fp;
	char	*szDirecotry;
	char strBuff[64];
	char *p;
	char *pb;
	int CmdNum;
	int cnt;
	_MODELGROUP	*lpModelGroup;
	int JobCount;

	char *szMotionFileList[128];

	int MotionFileListCnt;
	int MotionFrameList[100];
	smPAT3D	*pat3d;

	int MotionLastPos;

	MotionLastPos = 0;


	MotionFileListCnt = 0;
	ZeroMemory( MotionFrameList , sizeof( int )*100 );

	fp = fopen( iniFile , "rb" );
	if ( fp==NULL )
		return FALSE;

	//파일 이름으로 부터 디렉토리를 찾아 설정
	szDirecotry = SetDirectoryFromFile( iniFile );

	while( !feof( fp ) )//  feof: file end까지 읽어라 
	{
		if( fgets( line, 512, fp ) == NULL)	break;

		p = fileread_GetWord( decode , line);

		CmdNum = 0;

		while(1) {

			if ( lstrcmp( decode, szIniCommand[CmdNum] ) == 0 ) {
				break;
			}

			if ( szIniCommand[CmdNum][0]==NULL ) {
				//확장 모션
				if ( decode[0]=='*' && strstr( decode , szMotionWord ) ) {

					//######################################################################################
					//작 성 자 : 오 영 석
					if( Info->MotionCount < MOTION_INFO_MAX ) 
					{
						SetIniMotionInfo( &Info->MotionInfo[Info->MotionCount], p, 
										  &Info->NpcMotionRate[Info->MotionCount] );
					}
					//######################################################################################


					Info->MotionInfo[Info->MotionCount].MotionFrame = MotionFileListCnt;
					Info->MotionInfo[Info->MotionCount].State = TRUE;
	
					//달리기 모션
					if ( strstr( decode , szRunWord ) ) {
						Info->MotionInfo[Info->MotionCount].State = CHRMOTION_STATE_RUN;
						Info->MotionInfo[Info->MotionCount].Repeat = TRUE;
					}

					//공격 모션
					if ( strstr( decode , szAttackWord ) ) {
						Info->MotionInfo[Info->MotionCount].State = CHRMOTION_STATE_ATTACK;
					}
					//죽는 모션
					if ( strstr( decode , szDeadWord ) ) {
						Info->MotionInfo[Info->MotionCount].State = CHRMOTION_STATE_DEAD;
					}
					//죽는 모션
					if ( strstr( decode , szDeadWord2 ) ) {
						Info->MotionInfo[Info->MotionCount].State = CHRMOTION_STATE_DEAD;
					}

					//타격 모션
					if ( strstr( decode , szDamageWord ) ) {
						Info->MotionInfo[Info->MotionCount].State = CHRMOTION_STATE_DAMAGE;
					}
					//타격 모션
					if ( strstr( decode , szDamageWord2 ) ) {
						Info->MotionInfo[Info->MotionCount].State = CHRMOTION_STATE_DAMAGE;
					}
					//타격 모션
					if ( strstr( decode , szDamageWord3 ) ) {
						Info->MotionInfo[Info->MotionCount].State = CHRMOTION_STATE_DAMAGE;
					}
					//가끔하는 모션
					if ( strstr( decode , szSometimeWord ) ) {
						Info->MotionInfo[Info->MotionCount].State = CHRMOTION_STATE_SOMETIME;
					}

					//떨어지는 모션
					if ( strstr( decode , szFallDownWord ) ) {
						Info->MotionInfo[Info->MotionCount].State = CHRMOTION_STATE_FALLDOWN;
					}
					//착지 모션
					if ( strstr( decode , szFallStandSmallWord ) ) {
						Info->MotionInfo[Info->MotionCount].State = CHRMOTION_STATE_FALLSTAND;
					}
					//충격적인 착지 모션
					if ( strstr( decode , szFallStandLargeWord ) ) {
						Info->MotionInfo[Info->MotionCount].State = CHRMOTION_STATE_FALLDAMAGE;
					}

					//서있는 모션
					if ( strstr( decode , szStandWord ) || strstr( decode , szStandWord2 ) ) {
						Info->MotionInfo[Info->MotionCount].State = CHRMOTION_STATE_STAND;
						Info->MotionInfo[Info->MotionCount].Repeat = TRUE;
					}
					//걷는 모션
					if ( strstr( decode , szWalkWord ) ) {
						Info->MotionInfo[Info->MotionCount].State = CHRMOTION_STATE_WALK;
						Info->MotionInfo[Info->MotionCount].Repeat = TRUE;
					}
					//다시 시작 모션
					if ( strstr( decode , szRestartWord ) ) {
						Info->MotionInfo[Info->MotionCount].State = CHRMOTION_STATE_RESTART;
					}
					//물약먹기
					if ( strstr( decode , szEatWord ) ) {
						Info->MotionInfo[Info->MotionCount].State = CHRMOTION_STATE_EAT;
					}
					//기술동작
					if ( strstr( decode , szSkillWord ) ) {
						Info->MotionInfo[Info->MotionCount].State = CHRMOTION_STATE_SKILL;
						Info->MotionInfo[Info->MotionCount].SkillCodeList[0] = 0;
					}

					//기술동작
					if ( strstr( decode , szWarpWord ) || strstr( decode , szWarpWord2 ) ) {
						Info->MotionInfo[Info->MotionCount].State = CHRMOTION_STATE_WARP;
					}

					//만세동작
					if ( strstr( decode , szYahooWord ) ) {
						Info->MotionInfo[Info->MotionCount].State = CHRMOTION_STATE_YAHOO;
					}

					//죽는 모션 끝 프레임 보정 ( 권호씨가 준 프레임땜 )
					if ( Info->MotionInfo[Info->MotionCount].State==CHRMOTION_STATE_DEAD ) {
						Info->MotionInfo[Info->MotionCount].EndFrame -= 8;
					}

/*
char *szFallDownWord = "떨어";
char *szFallStandLargeWord = "큰착지";
char *szFallStandSmallWord = "작은착지";

#define CHRMOTION_STATE_FALLDOWN	0x140
#define CHRMOTION_STATE_FALLSTAND	0x150
#define CHRMOTION_STATE_FALLDAMAGE	0x160
*/

					MotionLastPos = Info->MotionCount;
					Info->MotionCount++;
				}
				//######################################################################################
				//작 성 자 : 오 영 석
				else if( decode[0]=='*' && strstr(decode, szMotionTalkWord) && MotionFileListCnt )
				{
					if ( Info->TalkMotionCount < TALK_MOTION_INFO_MAX )
					{
						SetIniMotionInfo( &Info->TalkMotionInfo[Info->TalkMotionCount], p,
										  &Info->TalkMotionRate[ Info->TalkMotionCount ] );
					}

					Info->TalkMotionInfo[ Info->TalkMotionCount ].MotionFrame = MotionFileListCnt-1;

					// 아표정
					if( strstr(decode, szArTalkWord) )
						Info->TalkMotionInfo[Info->TalkMotionCount].State = CHRMOTION_STATE_TALK_AR;
					// 이표정
					else if( strstr(decode, szETalkWord) )
						Info->TalkMotionInfo[Info->TalkMotionCount].State = CHRMOTION_STATE_TALK_E;
					// 오표정
					else if( strstr(decode, szOhTalkWord) )
						Info->TalkMotionInfo[Info->TalkMotionCount].State = CHRMOTION_STATE_TALK_OH;
					// 눈깜빡표정
					else if( strstr(decode, szEyeTalkWord) )
						Info->TalkMotionInfo[Info->TalkMotionCount].State = CHRMOTION_STATE_TALK_EYE;
					// 무표정(작동않함)
					else if( strstr(decode, szBlankTalkWord) )
						Info->TalkMotionInfo[Info->TalkMotionCount].State = CHRMOTION_STATE_TALK_BLANK;
					// 웃는표정
					else if( strstr(decode, szSmileTalkWord) )
						Info->TalkMotionInfo[Info->TalkMotionCount].State = CHRMOTION_STATE_TALK_SMILE;
                    // 화난표정
					else if( strstr(decode, szGrumbleTalkWord) )
						Info->TalkMotionInfo[Info->TalkMotionCount].State = CHRMOTION_STATE_TALK_GRUMBLE;
					// 슬픈표정
					else if( strstr(decode, szSorrowTalkWord) )
						Info->TalkMotionInfo[Info->TalkMotionCount].State = CHRMOTION_STATE_TALK_SORROW;
					// 놀란표정
					else if( strstr(decode, szStartledTalkWord) )
						Info->TalkMotionInfo[Info->TalkMotionCount].State = CHRMOTION_STATE_TALK_STARTLED;
					// 고유표정
					else if( strstr(decode, szNatureTalkWord) )
						Info->TalkMotionInfo[Info->TalkMotionCount].State = CHRMOTION_STATE_TALK_NATURE;
					// 특별표정
					else if( strstr(decode, szSpecialTalkWord) )
						Info->TalkMotionInfo[Info->TalkMotionCount].State = CHRMOTION_STATE_TALK_SPECIAL;

					Info->TalkMotionCount++;
				}
				//######################################################################################
				break;
			}

			CmdNum++;
		}



		switch( CmdNum ) {
		case 0:
			//동작 파일
			pb = p;
			p = fileread_GetWord( strBuff , p );
			if ( strBuff[0]==34 ) {	//따옴표 발견
				p = GetString( strBuff , pb );
			}
			lstrcpy( Info->szMotionFile , szDirecotry );
			lstrcat( Info->szMotionFile , strBuff );
			break;

		//######################################################################################
		//작 성 자 : 오 영 석
		case 17:	// 표정모음
		//######################################################################################

		case 1:
			//동작모음
			pb = p;
			p = fileread_GetWord( strBuff , p );
			if ( strBuff[0]==34 ) {	//따옴표 발견
				p = GetString( strBuff , pb );
			}
			szMotionFileList[ MotionFileListCnt ] = new char [64];
			lstrcpy( szMotionFileList[ MotionFileListCnt ] , szDirecotry );
			lstrcat( szMotionFileList[ MotionFileListCnt ] , strBuff );
			MotionFileListCnt++;
			break;

		case 2:
			//모양 파일
			p = fileread_GetWord( strBuff , p );
			lstrcpy( Info->szModelFile , szDirecotry );
			lstrcat( Info->szModelFile , strBuff );
			break;

		case 3:
			//모양 이름
			pb = p;
			p = fileread_GetWord( strBuff , p );
			if ( strBuff[0]==34 ) {	//따옴표 발견
				p = GetString( strBuff , pb );
			}

			//lstrcpy( Info->szModelName , strBuff );
			break;

		case 4:
			//"*동작연결파일",
			p = fileread_GetWord( strBuff , p );
			//######################################################################################
			//작 성 자 : 오 영 석
			lstrcpy( Info->szLinkFile , szDirecotry );
			lstrcat( Info->szLinkFile , strBuff );
			break;
			//######################################################################################

		case 5:
			//"*보조동작파일",
			pb = p;
			p = fileread_GetWord( strBuff , p );
			if ( strBuff[0]==34 ) {	//따옴표 발견
				p = GetString( strBuff , pb );
			}
			
			//######################################################################################
			//작 성 자 : 오 영 석
			lstrcpy( Info->szSubModelFile , szDirecotry );
			lstrcat( Info->szSubModelFile , strBuff );
			//######################################################################################
			break;

		case 6:
		case 7:
		case 8:

			switch(CmdNum) {
			case 6:
				//정밀 모양
				lpModelGroup = &Info->HighModel;
				break;
			case 7:
				lpModelGroup = &Info->DefaultModel;
				break;
			case 8:
				lpModelGroup = &Info->LowModel;
				break;
			}

			for(cnt=0;cnt<4;cnt++) {
				pb = p;
				p = fileread_GetWord( strBuff , p );
				if ( !strBuff[0] ) break;

				if ( strBuff[0]==34 ) {	//따옴표 발견
					p = GetString( strBuff , pb );
				}
				if ( lpModelGroup->ModelNameCnt<4 ) {
					lstrcpy( lpModelGroup->szModelName[ lpModelGroup->ModelNameCnt++ ] , strBuff );
				}
			}
			break;

		case 9:
			//*걷는동작
			//SetIniMotionInfo( &Info->MotionInfo[CHRMOTION_WALK] , p );
			//Info->MotionInfo[CHRMOTION_WALK].MotionFrame = MotionFileListCnt;
			break;
		case 10:
			//*서있기동작
			//SetIniMotionInfo( &Info->MotionInfo[CHRMOTION_STAND] , p );
			//Info->MotionInfo[CHRMOTION_STAND].MotionFrame = MotionFileListCnt;
			break;


/*
		case 11:
			//*점프동작
			SetIniMotionInfo( &Info->MotionInfo[CHRMOTION_JUMP] , p );
			Info->MotionInfo[CHRMOTION_JUMP].MotionFrame = MotionFileListCnt;
			break;
		case 12:
			//*공격동작
			SetIniMotionInfo( &Info->MotionInfo[CHRMOTION_ATTACK] , p );
			Info->MotionInfo[CHRMOTION_ATTACK].MotionFrame = MotionFileListCnt;
			break;
*/

		case 11:
			//"*착용무기",
			while(1) {
				if ( p[0]==0 ) 
					break;
				strBuff[0] = 0;
				p = fileread_GetWord( strBuff , p );
				if ( strBuff[0] ) {
					if ( lstrcmp( strBuff , "\270\360\265\316" )==0 ) {
						Info->MotionInfo[MotionLastPos].ItemCodeCount = 0; break; }

					if ( Info->MotionInfo[MotionLastPos].ItemCodeCount>=MOTION_TOOL_MAX/2 ) break;
					if ( lstrcmp( strBuff , "\276\370\300\275" )==0 ) {
						Info->MotionInfo[MotionLastPos].ItemCodeList[
								Info->MotionInfo[MotionLastPos].ItemCodeCount++] = 0xFFFF;		// bare hands
					}
					else {
						//코드검색 하여 설정 (our item table's index; keys we don't have are skipped)
						for(cnt=0;cnt<MAX_ITEM;cnt++) {
							if ( lstrcmpi( strBuff , sItem[cnt].LastCategory )==0 ) {
								Info->MotionInfo[MotionLastPos].ItemCodeList[
									Info->MotionInfo[MotionLastPos].ItemCodeCount++] = cnt;
								break;
							}
						}
					}

				}
				else
					break;
			}

			break;
		case 12:
			//"*적용직업",
//			if ( Info->MotionInfo[MotionLastPos].State!=CHRMOTION_STATE_SKILL ) {
				Info->MotionInfo[MotionLastPos].dwJobCodeBit = 0;
				while(1) {
					p = fileread_GetWord( strBuff , p );
					if ( strBuff[0]==0 ) break;
					JobCount = 0;
					while(1) {
						if ( !BitMaskJobNames[JobCount].dwBitMask ) break;
						if ( lstrcmpi( strBuff , BitMaskJobNames[JobCount].szName )==0 ) {
							Info->MotionInfo[MotionLastPos].dwJobCodeBit |= BitMaskJobNames[JobCount].dwBitMask;
							break;
						}
						JobCount++;
					}
				}
//			}
			break;
		case 13:
			//"*해당위치",
			p = fileread_GetWord( strBuff , p );
			cnt = 0;

			if ( lstrcmp( strBuff, "\270\266\300\273" )==0 ) {
					cnt |= 1;
			}
			if ( lstrcmp( strBuff, "\307\312\265\345" )==0 ) {
					cnt |= 2;
			}

			p = fileread_GetWord( strBuff , p );
			if ( strBuff[0] ) {
				if ( lstrcmp( strBuff, "\270\266\300\273" )==0 ) {
						cnt |= 1;
				}
				if ( lstrcmp( strBuff, "\307\312\265\345" )==0 ) {
						cnt |= 2;
				}
			}

			if ( cnt==0 || Info->MotionInfo[MotionLastPos].State==CHRMOTION_STATE_SKILL ) 
				cnt=3; 

			Info->MotionInfo[MotionLastPos].MapPosition = cnt;
			break;

		case 14:
			//"*적용기술",
			if ( Info->MotionInfo[MotionLastPos].State==CHRMOTION_STATE_SKILL ) {
				JobCount = 0;
				while(1) {
					p = fileread_GetWord( strBuff , p );
					if ( strBuff[0]==0 ) break;
					cnt = 0;
					while(1) {
						if ( !SkillDataCode[cnt].szSkillName[0] ) 
							break;
						if ( JobCount>=(MOTION_SKIL_MAX-1) ) break;
						if ( lstrcmpi( SkillDataCode[cnt].szSkillName , strBuff )==0 ) {
							Info->MotionInfo[MotionLastPos].SkillCodeList[JobCount++] = cnt;
							break;
						}
						cnt++;
					}
				}
				Info->MotionInfo[MotionLastPos].SkillCodeList[JobCount] = 0;
			}
			break;

		//######################################################################################
		//작 성 자 : 오 영 석
		case 15:	//"*표정파일연결"	(파일연결)
			pb = p;
			p = fileread_GetWord( strBuff, p );
			if( strBuff[0] == 34 )	//따옴표 발견
				p = GetString( strBuff , pb );

			//######################################################################################
			//작 성 자 : 오 영 석
			lstrcpy( Info->szTalkLinkFile, szDirecotry );
			lstrcat( Info->szTalkLinkFile, strBuff );
			//######################################################################################
			break;

		case 16:	//"*표정파일"
			pb = p;
			p = fileread_GetWord( strBuff, p );
			if( strBuff[0] == 34 )	//따옴표 발견
				p = GetString( strBuff , pb );

			//######################################################################################
			//작 성 자 : 오 영 석
			lstrcpy( Info->szTalkMotionFile, szDirecotry );
			lstrcat( Info->szTalkMotionFile, strBuff );
			break;
			//######################################################################################
		//######################################################################################

		}
	}

	fclose(fp);


	//######################################################################################
	//작 성 자 : 오 영 석
	int MaxRateNum, MaxRatePos;

	// NpcMotionRate
	if( Info->NpcMotionRate[ CHRMOTION_EXT ] )
	{
		///////////////////////////////////////////////////////////////
		// 예외 처리. ( 글픽에서 제대로 셋팅 해야 함. )
		MaxRateNum = MaxRatePos = CmdNum = 0;
		for( cnt=CHRMOTION_EXT; cnt < (int)Info->MotionCount; cnt++ )
		{
			if( MaxRateNum < Info->NpcMotionRate[ cnt ] )
			{
				MaxRateNum = Info->NpcMotionRate[ cnt ];
				MaxRatePos = cnt;
			}
			
            CmdNum += Info->NpcMotionRate[ cnt ];
		}
		
		if( CmdNum > 100 )
		{
			MaxRateNum -= (CmdNum - 100);
			if( MaxRateNum <= 0 )
			{
				// 대책 없는 버그 발생인디. ㅡ_ㅡ 
				// 작동 하지 않게끔 셋팅.
                Info->NpcMotionRate[ CHRMOTION_EXT ] = 0;
			}
			else
				Info->NpcMotionRate[ MaxRatePos ] = MaxRateNum;
		}
		else if( CmdNum < 100 )
		{
			MaxRateNum += (100 - CmdNum);
            Info->NpcMotionRate[ MaxRatePos ] = MaxRateNum;
		}
		///////////////////////////////////////////////////////////////

		if( Info->NpcMotionRate[ CHRMOTION_EXT ] )
		{
			MaxRatePos = 0;
			for( cnt=CHRMOTION_EXT; cnt < (int)Info->MotionCount; cnt++ )
			{
				for( CmdNum=0; CmdNum < Info->NpcMotionRate[ cnt ]; CmdNum++ )
				{
					Info->NpcMotionRateCnt[ MaxRatePos ] = cnt;
					MaxRatePos++;
				}
			}
		}
	}

	// TalkMotionRate
	if( Info->TalkMotionRate[ CHRMOTION_EXT ] )
	{
		///////////////////////////////////////////////////////////////
		//
		int MotionCntBuff[ TALK_MOTION_FILE_MAX ][ TALK_MOTION_INFO_MAX ] = {0,};
		int RateBuff[ TALK_MOTION_FILE_MAX ][ TALK_MOTION_INFO_MAX ] = {0,};
		int RateCnt[ TALK_MOTION_FILE_MAX ] = { CHRMOTION_EXT, CHRMOTION_EXT };

		for( cnt=CHRMOTION_EXT; cnt < (int)Info->TalkMotionCount; cnt++ )
		{
			if( Info->TalkMotionInfo[cnt].MotionFrame == TALK_MOTION_FILE )
			{
				MotionCntBuff[ TALK_MOTION_FILE ][ RateCnt[TALK_MOTION_FILE] ] = cnt;
				RateBuff[ TALK_MOTION_FILE ][ RateCnt[TALK_MOTION_FILE] ] = Info->TalkMotionRate[cnt];
				RateCnt[ TALK_MOTION_FILE ]++;
			}
			else if( Info->TalkMotionInfo[cnt].MotionFrame == FACIAL_MOTION_FILE )
			{
				MotionCntBuff[ FACIAL_MOTION_FILE ][ RateCnt[FACIAL_MOTION_FILE] ] = cnt;
				RateBuff[ FACIAL_MOTION_FILE ][ RateCnt[FACIAL_MOTION_FILE] ] = Info->TalkMotionRate[cnt];
				RateCnt[FACIAL_MOTION_FILE]++;
			}
		}
		///////////////////////////////////////////////////////////////

		//
		for( int loop=0; loop < TALK_MOTION_FILE_MAX; loop++ )
		{
			///////////////////////////////////////////////////////////////
			// 예외 처리. ( 글픽에서 제대로 셋팅 해야 함. )
			MaxRateNum = MaxRatePos = CmdNum = 0;

			int *lpRateBuff = RateBuff[ loop ];
			for( cnt=CHRMOTION_EXT; cnt < RateCnt[ loop ]; cnt++ )
			{
				if( MaxRateNum < lpRateBuff[cnt] )
				{
					MaxRateNum = lpRateBuff[cnt];
					MaxRatePos = cnt;
				}
				
				CmdNum += lpRateBuff[ cnt ];
			}

			if( CmdNum > 100 )
			{
				MaxRateNum -= (CmdNum - 100);
				if( MaxRateNum <= 0 )
				{
					// 대책 없는 버그 발생인디. ㅡ_ㅡ 
					// 작동 하지 않게끔 셋팅.
					Info->TalkMotionRate[ CHRMOTION_EXT ] = 0;
				}
				else
					lpRateBuff[ MaxRatePos ] = MaxRateNum;
			}
			else if( CmdNum < 100 )
			{
				MaxRateNum += (100 - CmdNum);
				lpRateBuff[ MaxRatePos ] = MaxRateNum;
			}
			///////////////////////////////////////////////////////////////

			if( Info->TalkMotionRate[ CHRMOTION_EXT ] )
			{
				MaxRatePos = 0;
				for( cnt=CHRMOTION_EXT; cnt < RateCnt[ loop ]; cnt++ )
				{
					for( CmdNum=0; CmdNum < lpRateBuff[ cnt ]; CmdNum++ )
					{
						Info->TalkMotionRateCnt[ loop ][ MaxRatePos ] = MotionCntBuff[ loop ][ cnt ];
						MaxRatePos++;
					}
				}
			}
		}
	}
	//######################################################################################


	//모션 파일 없구 .. 동작 모음만 있으면..모션 생성
	//######################################################################################
	//작 성 자 : 오 영 석
#ifdef PT_WEB
	// The web build never rebuilds a motion file from its parts: the data has the merged
	// .smb (m1..m8.smb), and an empty talk-motion name would otherwise count as "missing"
	// and download and merge every part (tens of MB per character).
	for(cnt=MotionFileListCnt-1;cnt>=0;cnt--)
		delete szMotionFileList[cnt];
	MotionFileListCnt = 0;
#endif
	if( MotionFileListCnt )
	//if( MotionFileListCnt > 1 )
	{
		if( smFindPatFile( Info->szMotionFile, "smb" ) == FALSE )
		{
			pat3d = smASE_MergeBone( Info->szMotionFile, szMotionFileList, MotionFileListCnt );
			if ( pat3d ) delete pat3d;
		}
		else if( smFindPatFile( Info->szTalkMotionFile, "smb" ) == FALSE )
		{
			pat3d = smASE_MergeBone( Info->szTalkMotionFile, szMotionFileList, MotionFileListCnt, 0 );
			if ( pat3d ) delete pat3d;
		}

		for(cnt=MotionFileListCnt-1;cnt>=0;cnt--)
			delete szMotionFileList[cnt];
	}
	//######################################################################################

#ifdef PT_WEB
	// The web build reads the class motion tables (char\tmABCD\M?Bip.in) as text: their
	// weapon keys and skill names resolve against our own item and skill tables, which the
	// v3215 .inx files' numbers do not (they index that client's tables). No compiled copy is
	// written: the page keeps files the game writes, so it would go stale with the data.
	return TRUE;
#endif

#ifndef _W_SERVER
		if ( !smConfig.DebugMode ) {
			quit = TRUE;
			return TRUE;
		}
#endif

	//######################################################################################
	//작 성 자 : 오 영 석
	char  *szFile = ChangeFileExt( iniFile, szFileExt );
	DWORD  dwCode = GetSpeedSum( szFile );
	ModelKeyWordEncode( dwCode, Info );

	//
	if( Info->MotionCount > CHRMOTION_EXT && Info->TalkMotionCount <= CHRMOTION_EXT )
		MotionKeyWordEncode( dwCode, Info );

	//모션 파일 저장
	szFile = ChangeFileExt( iniFile, szFileExt );	//필히 다시 해야 함.!! 버퍼가 하나이기에.!
	fp = fopen( szFile , "wb" );
	if( fp )
	{
		fwrite( Info , sizeof( smMODELINFO ) , 1, fp);
		fclose(fp);
	}

	//
	if( Info->MotionCount > CHRMOTION_EXT && Info->TalkMotionCount <= CHRMOTION_EXT )
		MotionKeyWordDecode( dwCode, Info );
	//######################################################################################

	return TRUE;
}


//모델 정보 파일을 해독하여 돌려준다 (Info에저장)
//######################################################################################
//작 성 자 : 오 영 석
//////////////////////////////////////////////////////////////////////////////////
// .inx files are a raw smMODELINFO. The v3215 data stores 104 item codes per
// motion (MOTION_TOOL_MAX); older files store 52. Convert an old file to the
// current layout: every smMOTIONINFO is shifted by the 52 extra bytes.
//////////////////////////////////////////////////////////////////////////////////
#define MOTIONINFO_LIST_OFS		offsetof( smMOTIONINFO, ItemCodeList )
#define MOTIONINFO_OLD_SIZE		( sizeof(smMOTIONINFO) - (MOTION_TOOL_MAX-MOTION_TOOL_MAX_OLD) )
#define SMMODELINFO_OLD_SIZE	( sizeof(smMODELINFO) - (MOTION_INFO_MAX+TALK_MOTION_INFO_MAX)*(MOTION_TOOL_MAX-MOTION_TOOL_MAX_OLD) )
static_assert( sizeof(smMODELINFO)==95268, "smMODELINFO must match the v3215 .inx size" );
static_assert( SMMODELINFO_OLD_SIZE==67084, "old .inx size" );

// character.cpp: an entry of our item table (sItem) as a v3215 motion item index
WORD GetMotionItemIndexOfSlot( int ItemSlot );

static void MotionInfoFromOld( BYTE *lpOld, smMOTIONINFO *lpNew, int Count )
{
	int headSize = MOTIONINFO_LIST_OFS;								//State ~ ItemCodeCount
	int tailSize = MOTIONINFO_OLD_SIZE - headSize - MOTION_TOOL_MAX_OLD;	//dwJobCodeBit ~ MotionFrame

	for ( int cnt=0; cnt<Count; cnt++ ) {
		BYTE *lpDest = (BYTE *)&lpNew[cnt];
		memcpy( lpDest, lpOld, headSize );
		memcpy( lpDest + MOTIONINFO_LIST_OFS + MOTION_TOOL_MAX, lpOld + headSize + MOTION_TOOL_MAX_OLD, tailSize );

		// the old list: one byte per weapon, our item table's index (0xFF bare hands)
		BYTE *lpList = lpOld + headSize;
		int n = lpNew[cnt].ItemCodeCount;
		if ( n<0 || n>MOTION_TOOL_MAX/2 ) n = 0;
		lpNew[cnt].ItemCodeCount = n;
		for ( int i=0; i<MOTION_TOOL_MAX/2; i++ )
			lpNew[cnt].ItemCodeList[i] = i<n ? ( lpList[i]==0xFF ? 0xFFFF : lpList[i] ) : 0;

		lpOld += MOTIONINFO_OLD_SIZE;
	}
}

static void smModelInfoFromOldFile( BYTE *lpOld, smMODELINFO *lpInfo )
{
	BYTE *lpNew = (BYTE *)lpInfo;
	int ofsMotion = offsetof( smMODELINFO, MotionInfo );
	int ofsMid = offsetof( smMODELINFO, MotionCount );
	int ofsTalk = offsetof( smMODELINFO, TalkMotionInfo );
	int ofsTail = offsetof( smMODELINFO, TalkMotionCount );

	memcpy( lpNew, lpOld, ofsMotion );									//파일 이름, 모델 그룹
	lpOld += ofsMotion;
	MotionInfoFromOld( lpOld, lpInfo->MotionInfo, MOTION_INFO_MAX );
	lpOld += MOTIONINFO_OLD_SIZE*MOTION_INFO_MAX;
	memcpy( lpNew+ofsMid, lpOld, ofsTalk-ofsMid );						//MotionCount ~ szTalkMotionFile
	lpOld += ofsTalk-ofsMid;
	MotionInfoFromOld( lpOld, lpInfo->TalkMotionInfo, TALK_MOTION_INFO_MAX );
	lpOld += MOTIONINFO_OLD_SIZE*TALK_MOTION_INFO_MAX;
	memcpy( lpNew+ofsTail, lpOld, sizeof(smMODELINFO)-ofsTail );		//TalkMotionCount ~ 끝
}

int smModelDecode( char *iniFile , smMODELINFO *Info )
{
	char  szFile[64];
	DWORD dwFileLen;
	char *lpFile;
	FILE *fp;
	int	  len;

	smMODELINFO	smModelInfo;
	ZeroMemory( Info, sizeof( smMODELINFO ) );

	Info->TalkMotionCount = CHRMOTION_EXT;	

	Info->MotionCount = CHRMOTION_EXT;
	Info->HighModel.ModelNameCnt = 0;
	Info->DefaultModel.ModelNameCnt = 0;
	Info->LowModel.ModelNameCnt = 0;

	len = lstrlen( iniFile );

	//ASE 파일을 직접 넣었을 경우
	if ( iniFile[len-4]=='.' &&
		(iniFile[len-3]=='a' || iniFile[len-3]=='A' ) &&
		(iniFile[len-2]=='s' || iniFile[len-2]=='S' ) &&
		(iniFile[len-1]=='e' || iniFile[len-1]=='E' ) ) {

		lstrcpy( Info->szModelFile , iniFile );

		return TRUE;
	}

	//
	for( int i=0; i < 3; i++ )
	{
		switch( i )
		{
		case 0:
			lstrcpy( szFile, iniFile );
			break;

		case 1:
			lstrcpy( szFile, Info->szLinkFile );
			break;

		case 2:
			lstrcpy( szFile, Info->szTalkLinkFile );
			break;
		}

		if( szFile[0] )
		{
			lpFile = smFindFile( szFile , szFileExt , &dwFileLen );
			if( lpFile && ( dwFileLen == sizeof(smMODELINFO) || dwFileLen == SMMODELINFO_OLD_SIZE ) )
			{
				fp = fopen( lpFile, "rb" );
				if( fp )
				{
					ZeroMemory( &smModelInfo, sizeof( smMODELINFO ) );
					if ( dwFileLen == sizeof(smMODELINFO) )
						fread( &smModelInfo, sizeof( smMODELINFO ) , 1, fp );
					else {
						//예전 형식 (.inx 의 동작별 아이템 목록이 52개)
						BYTE *lpOld = new BYTE[ SMMODELINFO_OLD_SIZE ];
						fread( lpOld, SMMODELINFO_OLD_SIZE, 1, fp );
						smModelInfoFromOldFile( lpOld, &smModelInfo );
						delete [] lpOld;
					}
					fclose(fp);

					DWORD dwCode = GetSpeedSum( lpFile );
					if( ModelKeyWordDecode( dwCode, &smModelInfo ) == FALSE )
						return FALSE;

					if( Info->szModelFile[0] && i > 0 )
					{
						if( smModelInfo.MotionCount > CHRMOTION_EXT && i == 1 )
						{
							if( MotionKeyWordDecode(dwCode, &smModelInfo) == FALSE )
								return FALSE;

							//
							lstrcpy( Info->szMotionFile, smModelInfo.szMotionFile );
							CopyMemory( Info->MotionInfo, smModelInfo.MotionInfo, sizeof(smModelInfo.MotionInfo) );
							Info->MotionCount = smModelInfo.MotionCount;

							//
							CopyMemory( Info->NpcMotionRate, smModelInfo.NpcMotionRate, sizeof(smModelInfo.NpcMotionRate) );
							CopyMemory( Info->NpcMotionRateCnt, smModelInfo.NpcMotionRateCnt, sizeof(smModelInfo.NpcMotionRateCnt) );							
						}
						else if( smModelInfo.TalkMotionCount > CHRMOTION_EXT && i == 2 )
						{
							//
							lstrcpy( Info->szTalkLinkFile, smModelInfo.szTalkLinkFile );
							lstrcpy( Info->szTalkMotionFile, smModelInfo.szTalkMotionFile );
							CopyMemory( Info->TalkMotionInfo, smModelInfo.TalkMotionInfo, sizeof(smModelInfo.TalkMotionInfo) );
							Info->TalkMotionCount = smModelInfo.TalkMotionCount;

							//							
							CopyMemory( Info->TalkMotionRate, smModelInfo.TalkMotionRate, sizeof(smModelInfo.TalkMotionRate) );
							CopyMemory( Info->TalkMotionRateCnt, smModelInfo.TalkMotionRateCnt, sizeof(smModelInfo.TalkMotionRateCnt) );
						}		
					}
					else
					{
						if( smModelInfo.MotionCount > CHRMOTION_EXT )
						{
							if( MotionKeyWordDecode(dwCode, &smModelInfo) == FALSE )
								return FALSE;
						}

						memcpy( Info, &smModelInfo, sizeof(smMODELINFO) );
					}
				}
				else
				{
					if( AddModelDecode(szFile, Info) == FALSE )
						return FALSE;
				}
			}
			else
			{
				if( AddModelDecode(szFile, Info) == FALSE )
                    return FALSE;
			}
		}
	}

	return TRUE;
}
//######################################################################################


//화면 및 캐릭터 및 파일등 기본 설정
smCONFIG	smConfig;

//hotuk.ini 에 *MODE / *SCREENSIZE 가 있었는지 (있으면 레지스트리보다 우선)
static BOOL	IniSetWinMode = FALSE;
static BOOL	IniSetScreenSize = FALSE;



#ifndef _LANGUAGE_KOREAN
char *szConfigIniCommand[] = {
	"*MODE",
	"*SCREENSIZE",
	"*COLORMODE",

	"*\271\350\260\346\310\255\270\351",
	"*\270\336\264\272\310\255\270\351",

	"*PLAYERSKIN",
	"*\263\252\273\333\263\360",

	"*\301\366\307\374\270\360\265\250",
	"*\301\366\307\374\271\260\303\274",

	"*\271\350\260\346\271\340\261\342",
	"*\271\350\260\346\274\261\270\355\265\265",
	"*\301\266\270\355\260\242\265\265",
//	"*조명위치",

	"*\274\255\271\366\301\326\274\322",
	"*\274\255\271\366\306\367\306\256",

	"*\310\255\270\351\307\260\301\372",
	"*\260\374\302\373\300\332\270\360\265\345",

	"*\265\245\300\314\305\270\274\255\271\366\301\326\274\322",
	"*\265\245\300\314\305\270\274\255\271\366\306\367\306\256",

	"*\304\263\270\257\305\315\274\255\271\366\301\326\274\322",
	"*\304\263\270\257\305\315\274\255\271\366\306\367\306\256",
	
//	"Copyright-2002",
	//"MFC40.DLL",
#ifdef _LANGUAGE_ENGLISH	
#ifndef _LANGUAGE_PHILIPIN
	"\276\313\267\272\275\272*Bobby&\300\314\275\272\270\266\277\244~-\244\314\244\273",
	"\306\304\276\306\266\365",
#else
	"\263\252\273\333\261\331\271\350Halem$&%Bye\244\273\244\273",
	"\300\314\267\261\232\170",
#endif
#else
//중국
#ifdef _LANGUAGE_CHINESE
	"\307\307\305\315\272\376\276\262\274\323\276\306\270\247\264\331\277\356\306\304\266\373\260\355\276\347\300\314",
	"\264\251\267\261\273\366",
#else
#ifdef _LANGUAGE_THAI
	"\261\327\271\260\266\313\262\277\271\331\301\366\274\323\261\364\300\345\300\341\300\332\270\256",
	"\272\323\300\272\273\366",
#else
#ifdef _LANGUAGE_BRAZIL
	"\276\306\270\363\265\345\302\311\304\332~\277\370\303\362>_<",
	"\303\312\304\332\273\366",
#else
//아르헨티나
#ifdef _LANGUAGE_ARGENTINA
	"\301\244\307\317\301\366$\276\312\276\322\264\331..\232\170\264\365~\244\273",
	"\276\306!\301\367%",
#else
#ifdef _LANGUAGE_JAPANESE
	"\271\314\267\241offline$#!\264\353\276\262\277\344",
	"XD1004%\274\261\271\260\274\274\306\256",
#else
#ifdef _LANGUAGE_VEITNAM
	"\277\326\263\274\260\241\276\306\277\245\300\314\270\246\307\317\263\304%\302\245\301\365\301\366\264\353\267\316",
	"\307\301\305\327\305\365\265\265\301\361\260\334\266\363",
#else
	"\270\301\273\347\266\313\262\277\306\322\306\274\274\323\261\364\300\345\260\263\271\314",
	"\307\316\305\251\273\366",
#endif
#endif
#endif
#endif
#endif
#endif
#endif

	"*MONSTER_KEY",

	//######################################################################################
	//작 성 자 : 오 영 석
	"*\301\366\307\374\276\326\264\317",
	//######################################################################################


	0
};

char *szWordWinMode[5] = { "FULLSCREEN" , "WINDOW" , "SERVER" };
#else
char *szConfigIniCommand[] = {
	"*\310\255\270\351\270\360\265\345",
	"*\310\255\270\351\307\330\273\363\265\265",
	"*\310\255\270\351\273\366\273\363\272\361\306\256",

	"*\271\350\260\346\310\255\270\351",
	"*\270\336\264\272\310\255\270\351",

	"*\304\263\270\257\305\315\275\272\305\262",
	"*\263\252\273\333\263\360",

	"*\301\366\307\374\270\360\265\250",
	"*\301\366\307\374\271\260\303\274",

	"*\271\350\260\346\271\340\261\342",
	"*\271\350\260\346\274\261\270\355\265\265",
	"*\301\266\270\355\260\242\265\265",
//	"*조명위치",

	"*\274\255\271\366\301\326\274\322",
	"*\274\255\271\366\306\367\306\256",

	"*\310\255\270\351\307\260\301\372",
	"*\260\374\302\373\300\332\270\360\265\345",

	"*\265\245\300\314\305\270\274\255\271\366\301\326\274\322",
	"*\265\245\300\314\305\270\274\255\271\366\306\367\306\256",

	"*\304\263\270\257\305\315\274\255\271\366\301\326\274\322",
	"*\304\263\270\257\305\315\274\255\271\366\306\367\306\256",
	
	"\306\367\300\345\264\334\303\274\301\326\271\256\310\257\277\265",
	"(\277\325\303\312\272\270\260\241\300\314\265\345)",

	"*\270\363\275\272\305\315\301\366\301\244",

	//######################################################################################
	//작 성 자 : 오 영 석
	"*\301\366\307\374\276\326\264\317",
	//######################################################################################

	0
};
char *szWordWinMode[5] = { "\300\374\303\274\310\255\270\351" , "\303\242\270\360\265\345" , "\274\255\271\366\270\360\265\345" };
#endif

char *szWordTexQualty[4] = { "\301\301\300\275" , "\272\270\305\353" , "\263\252\273\335" , "\276\306\301\326\263\252\273\335" };
char *InfoRegPath = "SOFTWARE\\Triglow Pictures\\PristonTale";

/////////////////////////////////////////////////////////////////////////
ODBC_CONFIG	Odbc_Config;
TRANS_SERVER_LIST	TransServerList;
rsSERVER_CONFIG	rsServerConfig;
static int	srAdminCmdCnt = 0;
DWORD GetSwapIPCode( char *szIP );			//아이피 주소를 뒤집어서 변환 ( onserver.cpp )

/*
	int		LimitVersion;			//접속 가능한 최하 버전
	char	szAdminPassword[3][64];	//관리자 권한 부여 패스워드
*/

#ifdef _W_SERVER

//서버 설정 서브 루틴
int SetServerConfig( char *decode , char *lpPoint )
{

	int flag;
	char *p = lpPoint;
	char *pb;
	char strBuff[128];
	int cnt;

	flag = 0;

	if ( lstrcmp( decode , "*\274\255\271\366\304\332\265\345\271\370\310\243" )==0 ||
		lstrcmpi( decode , "*SERVER_CODE" )==0 ) {
		pb=p;p=fileread_GetWord(strBuff,p);if(strBuff[0]==34)p=GetString(strBuff,pb);
		rsServerConfig.ServerCode = atoi(strBuff);
		flag++;
	}

	if ( !flag && ( lstrcmp( decode , "*\300\332\265\277\304\263\270\257\305\315\273\347\277\353" )==0 ||
		lstrcmpi( decode , "*SERVER_AUTOPLAY" )==0 ) ) {
		rsServerConfig.AutoPlayer = TRUE;
		flag++;
	}

	if ( !flag && ( lstrcmp( decode , "*\300\372\300\345\271\366\306\333\273\347\277\353" )==0 ||
		lstrcmpi( decode , "*SERVER_RECORD_MEM" )==0 ) ) {
		rsServerConfig.UseRecorMemory = TRUE;
		flag++;
	}


	if ( !flag && lstrcmp( decode , "*\265\277\267\341\261\335\301\366" )==0 ) {
		rsServerConfig.DisableParty = TRUE;
		flag++;
	}
	if ( !flag && ( lstrcmp( decode , "*\301\242\274\323\301\246\307\321\300\316\277\370" )==0 ||
		lstrcmpi( decode , "*MAX_USERS" )==0 )) {
		pb=p;p=fileread_GetWord(strBuff,p);if(strBuff[0]==34)p=GetString(strBuff,pb);
		rsServerConfig.ConnectUserMax = atoi(strBuff);
		flag++;
	}

	if ( !flag && ( lstrcmp( decode , "*\305\327\275\272\306\256\274\255\271\366" )==0 || 
		lstrcmpi( decode , "*TEST_SERVER" )==0 ) ) {
		rsServerConfig.TestSeverMode = TRUE;
		pb=p;p=fileread_GetWord(strBuff,p);if(strBuff[0]==34)p=GetString(strBuff,pb);
		if ( strBuff[0] ) {
			rsServerConfig.TestSeverMode = atoi(strBuff);
		}
		flag++;
	}

	if ( !flag && ( lstrcmp( decode , "*\301\242\274\323\260\241\264\311\304\332\265\345" )==0 ||
		lstrcmpi( decode , "*CONNECT_CLIENT_CODE" )==0 ) ) {
		for( cnt=0;cnt<32;cnt++ ) {
			p=fileread_GetWord(strBuff,p);
			if ( !strBuff[0] ) break;
			//접속가능 클라이언트 첵크섬 목록
			rsServerConfig.dwClientCheckSum[rsServerConfig.ClientCheckSumCount] = atoi(strBuff);
			rsServerConfig.ClientCheckSumCount++;
		}
		flag++;
	}

	if ( !flag && ( lstrcmp( decode , "*\304\332\265\345\301\242\274\323\301\276\267\341" )==0 || 
		lstrcmpi( decode , "*DISCONNECT_CLIENT_BADCODE" )==0 ) ) {
		rsServerConfig.ClientCheckOut = TRUE;
		flag++;
	}

	if ( !flag && ( lstrcmp( decode , "*\263\273\272\316\272\270\310\243\304\332\265\345" )==0 || 
		lstrcmpi( decode , "*CLSAFE_CODE" )==0 ) ) {

		for( cnt=0;cnt<32;cnt++ ) {
			p=fileread_GetWord(strBuff,p);
			if ( !strBuff[0] ) break;
			//내부 보호 코드 목록
			rsServerConfig.dwSafeClientCode[rsServerConfig.SafeClientCodeCount] = atoi(strBuff);
			rsServerConfig.SafeClientCodeCount++;
		}
		flag++;
//		pb=p;p=fileread_GetWord(strBuff,p);if(strBuff[0]==34)p=GetString(strBuff,pb);
//		rsServerConfig.dwSafeClientCode = atoi(strBuff);
//		flag++;
	}
	if ( !flag && ( lstrcmp( decode , "*\272\270\310\243\304\332\265\345\277\300\267\371\301\276\267\341" )==0 || 
		lstrcmpi( decode , "*DISCONNECT_CLSAFE_CODE" )==0 ) ) {
		rsServerConfig.ClientCodeCheckOut = TRUE;
		flag++;
	}


	if ( !flag && ( lstrcmp( decode , "*\301\246\307\321\271\366\300\374" )==0 || 
		lstrcmpi( decode , "*VERSION" )==0 ) ) {
		pb=p;p=fileread_GetWord(strBuff,p);if(strBuff[0]==34)p=GetString(strBuff,pb);
		rsServerConfig.LimitVersion = atoi(strBuff);

		pb=p;p=fileread_GetWord(strBuff,p);if(strBuff[0]==34)p=GetString(strBuff,pb);
		if ( strBuff[0] )
			rsServerConfig.LimitVersionMax = atoi(strBuff);

		flag++;
	}

	if ( !flag && ( lstrcmp( decode , "*\260\374\270\256\261\307\307\321\272\316\277\251" )==0 || 
		lstrcmpi( decode , "*ADMIN_COMMAND" )==0 ) ) {
		pb=p;p=fileread_GetWord(strBuff,p);if(strBuff[0]==34)p=GetString(strBuff,pb);
		lstrcpy( rsServerConfig.szAdminPassword[srAdminCmdCnt] , strBuff );
		srAdminCmdCnt++;
		flag++;
	}

	if ( !flag && ( lstrcmp( decode , "*\260\374\270\256\300\332\300\314\270\247" )==0 || 
		lstrcmpi( decode , "*ADMIN_NAME" )==0 ) ) {
		pb=p;p=fileread_GetWord(strBuff,p);if(strBuff[0]==34)p=GetString(strBuff,pb);
		lstrcpy( rsServerConfig.szAdminName , strBuff );
		flag++;
	}

	if ( !flag && ( lstrcmp( decode , "*\260\374\270\256\300\332IP\301\326\274\322" )==0 || 
		lstrcmpi( decode , "*ADMIN_IP" )==0 ) ) {
		pb=p;p=fileread_GetWord(strBuff,p);if(strBuff[0]==34)p=GetString(strBuff,pb);
		lstrcpy( rsServerConfig.szAdminIP , strBuff );
		flag++;
	}

	if ( !flag && ( lstrcmp( decode , "*\260\374\270\256\300\332\301\242\274\323\276\317\310\243" )==0 || 
		lstrcmpi( decode , "*ADMIN_LOGIN" )==0 ) ) {
		pb=p;p=fileread_GetWord(strBuff,p);if(strBuff[0]==34)p=GetString(strBuff,pb);
		lstrcpy( rsServerConfig.szAdminLogPassword , strBuff );
		flag++;
	}

	if ( !flag && ( lstrcmp( decode , "*\274\255\271\366\300\314\270\247" )==0 || 
		lstrcmpi( decode , "*SERVER_NAME" )==0 ) ) {
		pb=p;p=fileread_GetWord(strBuff,p);if(strBuff[0]==34)p=GetString(strBuff,pb);
		lstrcpy( rsServerConfig.szServerName , strBuff );
		flag++;
	}

	if ( !flag && ( lstrcmp( decode , "*PK\307\343\260\241" )==0 || 
		lstrcmpi( decode , "*ENABLE_PK" )==0 ) ) {

		pb=p;p=fileread_GetWord(strBuff,p);if(strBuff[0]==34)p=GetString(strBuff,pb);
		rsServerConfig.Enable_PK = atoi(strBuff);
		if ( rsServerConfig.Enable_PK==0 ) rsServerConfig.Enable_PK=TRUE;

		flag++;
	}

	if ( !flag && ( lstrcmp( decode , "*PK\307\312\265\345" )==0 || 
		lstrcmpi( decode , "*PK_FIELD" )==0 ) ) {

		pb=p;p=fileread_GetWord(strBuff,p);if(strBuff[0]==34)p=GetString(strBuff,pb);
		rsServerConfig.Enable_PKField = atoi(strBuff);
		if ( rsServerConfig.Enable_PKField==0 ) rsServerConfig.Enable_PKField=34;		//기본 PK필드 34 그리디 호수

		flag++;
	}


	// pluto PK허용 필드 전부
	if( !flag && ( lstrcmp( decode, "*PK\270\360\265\347\307\312\265\345" ) == 0 || lstrcmpi( decode, "*PK_FIELD_ALL" ) == 0 ) )
	{
		pb=p;p=fileread_GetWord(strBuff,p);if(strBuff[0]==34)p=GetString(strBuff,pb);
		rsServerConfig.Enable_PKField_All = atoi(strBuff);
		flag++;
	}


	if ( !flag && ( lstrcmp( decode , "*\271\260\276\340\260\250\275\303" )==0 || 
		lstrcmpi( decode , "*POTION_MONITOR" )==0 ) ) {

		pb=p;p=fileread_GetWord(strBuff,p);if(strBuff[0]==34)p=GetString(strBuff,pb);
		if ( strBuff[0] )
			rsServerConfig.PotionMonitor = atoi(strBuff);
		else
			rsServerConfig.PotionMonitor = 1;

		flag++;
	}

	if ( !flag && ( lstrcmp( decode , "*\272\270\276\310\260\313\273\347" )==0 || 
		lstrcmpi( decode , "*CL_PROTECT" )==0 ) ) {

		rsServerConfig.CheckProtect = TRUE;
		flag++;
	}

	if ( !flag && ( lstrcmp( decode , "*\260\324\300\323\260\241\265\345" )==0 || 
		lstrcmpi( decode , "*GAME_GUARD" )==0 ) ) {

		pb=p;p=fileread_GetWord(strBuff,p);if(strBuff[0]==34)p=GetString(strBuff,pb);
		if ( strBuff[0] )
			rsServerConfig.GameGuard_Auth = atoi(strBuff);
		else
			rsServerConfig.GameGuard_Auth = 2;

		flag++;
	}

	#ifdef LOG_THE_PK
		//PK 로그 남기는거 //해외
		if ( !flag && ( lstrcmpi( decode , "*PK_LOG" )==0 ) )
		{
			rsServerConfig.bPK_LOG = true;
			flag++;
		}
	#endif

	#ifdef _XTRAP_GUARD_4_
		if ( !flag && ( lstrcmp( decode , "*\277\242\275\272\306\256\267\246" )==0 || 
			lstrcmpi( decode , "*XTRAP_GUARD" )==0 ) ) 
		{
			rsServerConfig.XTRAP_Timer = 200;
			rsServerConfig.XTRAP_MaxDealy = 1;




			pb=p;p=fileread_GetWord(strBuff,p);if(strBuff[0]==34)p=GetString(strBuff,pb);
			if ( strBuff[0] ) {
				rsServerConfig.XTRAP_Timer = atoi(strBuff);
				OutputDebugString((strBuff));
				pb=p;p=fileread_GetWord(strBuff,p);if(strBuff[0]==34)p=GetString(strBuff,pb);
				if ( strBuff[0] ) rsServerConfig.XTRAP_MaxDealy = atoi(strBuff);
			}

			flag++;
		}
	#endif

	if ( !flag && ( lstrcmp( decode , "*\272\270\276\310\301\246\260\305" )==0 || 
		lstrcmpi( decode , "*REMOVE_PROTECT_TIME" )==0 ) ) {
		pb=p;p=fileread_GetWord(strBuff,p);if(strBuff[0]==34)p=GetString(strBuff,pb);
		rsServerConfig.RemoveNProtectTime = atoi(strBuff);
		flag++;
	}


	if ( !flag && ( lstrcmp( decode , "*\265\360\271\366\261\327IP" )==0 || 
		lstrcmpi( decode , "*DEBUG_IP" )==0 ) ) {

		for( cnt=0;cnt<10;cnt++ ) {
			p=fileread_GetWord(strBuff,p);
			if ( !strBuff[0] ) break;

			lstrcpy( rsServerConfig.szDebugIP[rsServerConfig.DebugIP_Count] , strBuff );
			rsServerConfig.DebugIP_Count++;
		}
		flag++;
	}

	if ( !flag && ( lstrcmp( decode , "*\265\360\271\366\261\327ID" )==0 || 
		lstrcmpi( decode , "*DEBUG_ID" )==0 ) ) {

		for( cnt=0;cnt<32;cnt++ ) {
			if ( rsServerConfig.DebugID_Count>=32 ) break;
			
			pb=p;p=fileread_GetWord(strBuff,p);if(strBuff[0]==34)p=GetString(strBuff,pb);
			if ( !strBuff[0] ) break;

			lstrcpy( rsServerConfig.szDebugID[rsServerConfig.DebugID_Count] , strBuff );
			rsServerConfig.DebugID_Count++;
		}
		flag++;
	}

	if ( !flag && ( lstrcmp( decode , "*\272\355\267\260\264\334\260\350" )==0 ||
		lstrcmpi( decode , "*BLOCK_LEVEL" )==0 ) ) {
		pb=p;p=fileread_GetWord(strBuff,p);if(strBuff[0]==34)p=GetString(strBuff,pb);
		rsServerConfig.BlockLevel = atoi(strBuff);
		flag++;
	}

	if ( !flag && ( lstrcmp( decode , "*\272\355\267\260\276\306\277\364" )==0 ||
		lstrcmpi( decode , "*BLOCK_LOGOUT" )==0 ) ) {
		rsServerConfig.BlockLevel_LogOut = TRUE;
		flag++;
	}

	if ( !flag && ( lstrcmp( decode , "*\276\306\300\314\305\333\277\300\267\371\301\276\267\341" )==0 ||
		lstrcmpi( decode , "*DISCONNECT_ITEM_ERROR" )==0 ) ) {
		pb=p;p=fileread_GetWord(strBuff,p);if(strBuff[0]==34)p=GetString(strBuff,pb);
		if ( strBuff[0] )
			rsServerConfig.ItemCodeOut = atoi(strBuff);
		else
			rsServerConfig.ItemCodeOut = 1;

		flag++;
	}

	if ( !flag && ( lstrcmp( decode , "*\306\343\274\307\260\313\273\347" )==0 ||
		lstrcmpi( decode , "*CLIENT_CHECK_FUNC" )==0 ) ) {
		rsServerConfig.ClientFuncCheckMode = 1;
		pb=p;p=fileread_GetWord(strBuff,p);if(strBuff[0]==34)p=GetString(strBuff,pb);
		rsServerConfig.ClientFuncCheckMode = atoi(strBuff);
		flag++;
	}

	if ( !flag && ( lstrcmp( decode , "*\274\333\306\355\300\314\272\245\306\256" )==0 ||
		lstrcmpi( decode , "*EVENT_MANDOO" )==0 ) ) {
		rsServerConfig.Event_Mandoo = TRUE;;
		flag++;
	}
	if ( !flag && ( lstrcmp( decode , "*\307\317\265\345\304\332\276\356\300\314\272\245\306\256" )==0 ||
		lstrcmpi( decode , "*EVENT_HARDCORE" )==0 ) ) {

			p=fileread_GetWord(strBuff,p);
			if ( strBuff[0] )
				rsServerConfig.Event_HardCore = atoi(strBuff);
			else
				rsServerConfig.Event_HardCore = 1;

		flag++;
	}
#ifdef	_LANGUAGE_JAPANESE // 장별 - 원래는 필리핀	// 장별 - 몬스터 공격력 할인
	if ( !flag && ( lstrcmp( decode , "*\260\370\260\335\267\302\310\260\300\316\300\314\272\245\306\256" )==0 ||
		lstrcmpi( decode , "*EVENT_DAMAGEOFF" )==0 ) )
	{
		p=fileread_GetWord(strBuff,p);
		if ( strBuff[0] )
			rsServerConfig.Event_DamageOff = atoi( strBuff );
		else
			rsServerConfig.Event_DamageOff = 1;
		flag++;
	}
#else
	if ( !flag && ( lstrcmpi( decode , "*EVENT_DAMAGEOFF1" )==0 ) )
	{
		p=fileread_GetWord(strBuff,p);
		if ( strBuff[0] )
			rsServerConfig.Event_DamageOff1 = atoi( strBuff );
		else
			rsServerConfig.Event_DamageOff1 = 1;
		flag++;
	}
	if ( !flag && ( lstrcmpi( decode , "*EVENT_DAMAGEOFF2" )==0 ) )
	{
		p=fileread_GetWord(strBuff,p);
		if ( strBuff[0] )
			rsServerConfig.Event_DamageOff2 = atoi( strBuff );
		else
			rsServerConfig.Event_DamageOff2 = 1;
		flag++;
	}
#endif
	//해외
	if ( !flag && ( lstrcmp( decode , "*\271\346\276\356\267\302\307\322\300\316\300\314\272\245\306\256" )==0 ||
		lstrcmpi( decode , "*EVENT_DEFENSEOFF" )==0 ) )
	{
		p=fileread_GetWord(strBuff,p);
		if ( strBuff[0] )
			rsServerConfig.Event_DefenseOff = atoi( strBuff );
		else
			rsServerConfig.Event_DefenseOff = 1;
		flag++;
	}
//해외////////////////////////////////////////////////////////////
	if ( !flag && ( lstrcmp( decode , "*\276\356\270\260\300\314\300\314\272\245\306\256" )==0 ||
		lstrcmpi( decode , "*EVENT_CHILD" )==0 ) ) {

			p=fileread_GetWord(strBuff,p);
			if ( strBuff[0] )
				rsServerConfig.Event_Child = atoi(strBuff);
			else
				rsServerConfig.Event_Child = 1;

		flag++;
	}

////////////////////////////////////////////////////////////////////
	if ( !flag && ( lstrcmp( decode , "*\305\251\270\256\275\272\305\273\300\314\272\245\306\256" )==0 ||
		lstrcmpi( decode , "*EVENT_CRISTAL" )==0 ) ) {

			p=fileread_GetWord(strBuff,p);
			if ( strBuff[0] )
				rsServerConfig.Event_Cristal = atoi(strBuff);
			else
				rsServerConfig.Event_Cristal = 1;

		flag++;
	}
	if ( !flag && ( lstrcmp( decode , "*\271\260\276\340\300\314\272\245\306\256" )==0 ||
		lstrcmpi( decode , "*EVENT_POTION" )==0 ) ) {

		p=fileread_GetWord(strBuff,p);
		rsServerConfig.Event_Potion = atoi( strBuff );
		flag++;
	}
	if ( !flag && ( lstrcmp( decode , "*\272\260\306\367\300\316\306\256\300\314\272\245\306\256" )==0 ||
		lstrcmpi( decode , "*EVENT_STARPOINT" )==0 ) ) {

		p=fileread_GetWord(strBuff,p);
		if ( strBuff[0] )
			rsServerConfig.Event_StarPointTicket = atoi( strBuff );
		else
			rsServerConfig.Event_StarPointTicket = 40;

		flag++;
	}

	if ( !flag && ( lstrcmp( decode , "*\273\375\300\317\300\314\272\245\306\256" )==0 ||
		lstrcmpi( decode , "*EVENT_BIRTHDAY" )==0 ) ) {

		p=fileread_GetWord(strBuff,p);
		if ( strBuff[0] )
			rsServerConfig.Event_ComicBirthDay = atoi( strBuff );
		else
			rsServerConfig.Event_ComicBirthDay = 1;

		flag++;
	}

	if ( !flag && ( lstrcmp( decode , "*\306\333\301\361\300\314\272\245\306\256" )==0 ||
		lstrcmpi( decode , "*EVENT_PUZZLE" )==0 ) ) {

		p=fileread_GetWord(strBuff,p);
		if ( strBuff[0] )
			rsServerConfig.Event_Puzzle = atoi( strBuff );
		else
			rsServerConfig.Event_Puzzle = 200;

		flag++;
	}

	if ( !flag && ( lstrcmp( decode , "*\271\331\272\247\273\324\300\314\272\245\306\256" )==0 ||
		lstrcmpi( decode , "*EVENT_BABELHORN" )==0 ) ) {

		p=fileread_GetWord(strBuff,p);
		if ( strBuff[0] )
			rsServerConfig.Event_BabelHorn = atoi( strBuff );
		else
			rsServerConfig.Event_BabelHorn = 4;

		if ( rsServerConfig.Event_BabelHorn>10 ) rsServerConfig.Event_BabelHorn=10;

		flag++;
	}

	if ( !flag && ( lstrcmp( decode , "*\261\270\271\314\310\243\300\314\272\245\306\256" )==0 ||
		lstrcmpi( decode , "*EVENT_NINEFOX" )==0 ) ) {

		p=fileread_GetWord(strBuff,p);
		if ( strBuff[0] )
			rsServerConfig.Event_NineFox = atoi( strBuff );
		else
			rsServerConfig.Event_NineFox = 20;

		flag++;
	}
	
	//크리스마스
	if ( !flag && ( lstrcmp( decode , "*\305\251\270\256\275\272\270\266\275\272\300\314\272\245\306\256" )==0 ||
		lstrcmpi( decode , "*EVENT_CHRISTMAS" )==0 ) ) {
		p=fileread_GetWord(strBuff,p);
		if ( strBuff[0] )
			rsServerConfig.EVENT_Christmas = atoi( strBuff );
		else
			rsServerConfig.EVENT_Christmas = 100;

		flag++;
	}
	if ( !flag && ( lstrcmp( decode , "*\271\337\267\273\305\270\300\316\265\245\300\314_\300\314\272\245\306\256" )==0 ||
		lstrcmpi( decode , "*EVENT_VALENTINEDAY" )==0 ) ) {

		p=fileread_GetWord(strBuff,p);
		if ( strBuff[0] )
			rsServerConfig.Event_ValentineDay = atoi( strBuff );
		else
			rsServerConfig.Event_ValentineDay = 100;

		flag++;
	}

		// pluto 커플링
	if ( !flag && ( lstrcmp( decode , "*\304\277\307\303\270\265_\300\314\272\245\306\256" )==0 ||
		lstrcmpi( decode , "*EVENT_COUPLERING" )==0 ) )
	{
		p=fileread_GetWord(strBuff,p);
		if ( strBuff[0] )
			rsServerConfig.Event_CoupleRing = atoi( strBuff );
		else
			rsServerConfig.Event_CoupleRing = 100;

		flag++;
	}

	// 박재원 - 수박 이벤트
	if ( !flag && ( lstrcmp( decode , "*\274\366\271\332_\300\314\272\245\306\256" )==0 ||
		lstrcmpi( decode , "*EVENT_WATERMELON" )==0 ) )
	{
		p=fileread_GetWord(strBuff,p);
		if ( strBuff[0] )
			rsServerConfig.Event_Watermelon = atoi( strBuff );
		else
			rsServerConfig.Event_Watermelon = 100;

		flag++;
	}

	// 장별 - 발렌타인 이벤트
	if ( !flag && ( lstrcmp( decode , "*\271\337\267\273\305\270\300\316_\300\314\272\245\306\256" )==0 ||
		lstrcmpi( decode , "*EVENT_VALENTINE" )==0 ) )
	{
		p=fileread_GetWord(strBuff,p);
		if ( strBuff[0] )
			rsServerConfig.Event_Valentine = atoi( strBuff );
		else
			rsServerConfig.Event_Valentine = 100;

		flag++;
	}


	// 장별 - 캔디데이즈
	if ( !flag && ( lstrcmp( decode , "*\304\265\265\360\265\245\300\314\301\356_\300\314\272\245\306\256" )==0 ||
		lstrcmpi( decode , "*EVENT_CANDYDAYS" )==0 ) ) {

		p=fileread_GetWord(strBuff,p);
		if ( strBuff[0] )
			rsServerConfig.Event_Candydays = atoi( strBuff );
		else
			rsServerConfig.Event_Candydays = 100;

		flag++;
	}

	// 박재원 - 밤하늘의 소원이벤트
	if ( !flag && ( lstrcmp( decode , "*\271\343\307\317\264\303\274\322\277\370_\300\314\272\245\306\256" )==0 ||
		lstrcmpi( decode , "*EVENT_STAR" )==0 ) )
	{
		p=fileread_GetWord(strBuff,p);
		if ( strBuff[0] )
			rsServerConfig.Event_Star = atoi( strBuff );
		else
			rsServerConfig.Event_Star = 100;

		flag++;
	}

	// 박재원 - 알파벳 조합 이벤트
	if ( !flag && ( lstrcmp( decode , "*\276\313\306\304\272\252\301\266\307\325_\300\314\272\245\306\256" )==0 ||
		lstrcmpi( decode , "*EVENT_PRISTON_ALPHABET" )==0 ) )
	{
		p=fileread_GetWord(strBuff,p);
		if ( strBuff[0] )
			rsServerConfig.Event_PristonAlphabet = atoi( strBuff );
		else
			rsServerConfig.Event_PristonAlphabet = 100;
		flag++;
	}


	// 장별 - 화이트콘트라스트 이벤트
	if ( !flag && ( lstrcmp( decode , "*\310\255\300\314\306\256\304\334\306\256\266\363\275\272\306\256_\300\314\272\245\306\256" )==0 ||
		lstrcmpi( decode , "*EVENT_WHITECONTRAST" )==0 ) )
	{
		p=fileread_GetWord(strBuff,p);
		if ( strBuff[0] )
			rsServerConfig.Event_WhiteContrast = atoi( strBuff );
		else
			rsServerConfig.Event_WhiteContrast = 100;

		flag++;
	}

	// 장별 - 대장장이의 혼
	if ( !flag && ( lstrcmp( decode , "*\264\353\300\345\300\345\300\314\300\307\310\245_\300\314\272\245\306\256" )==0 ||
		lstrcmpi( decode , "*EVENT_BLACKSMITH" )==0 ) )
	{
		p=fileread_GetWord(strBuff,p);
		if ( strBuff[0] )
			rsServerConfig.Event_BlackSmith = atoi( strBuff );
		else
			rsServerConfig.Event_BlackSmith = 100;

		flag++;
	}

	// 장별 - 매지컬그린
	if ( !flag && ( lstrcmp( decode , "*\270\305\301\366\304\303\261\327\270\260_\300\314\272\245\306\256" )==0 ||
		lstrcmpi( decode , "*EVENT_MAGICALGREEN" )==0 ) )
	{
		p=fileread_GetWord(strBuff,p);
		if ( strBuff[0] )
			rsServerConfig.Event_MagicalGreen = atoi( strBuff );
		else
			rsServerConfig.Event_MagicalGreen = 100;

		flag++;
	}

	// 장별 - 카라의 눈물
	if ( !flag && ( lstrcmp( decode , "*\304\253\266\363\300\307\264\253\271\260_\300\314\272\245\306\256" )==0 ||
		lstrcmpi( decode , "*EVENT_TEAROFKARA" )==0 ) ) {

		p=fileread_GetWord(strBuff,p);
		if ( strBuff[0] )
			rsServerConfig.Event_TearOfKara = atoi( strBuff );
		else
			rsServerConfig.Event_TearOfKara = 100;

		flag++;
	}


	if ( !flag && ( lstrcmp( decode , "*\310\255\300\314\306\256\265\245\300\314_\300\314\272\245\306\256" )==0 ||
		lstrcmpi( decode , "*EVENT_WHITEDAY" )==0 ) ) {

		p=fileread_GetWord(strBuff,p);
		if ( strBuff[0] )
			rsServerConfig.Event_WhiteDay = atoi( strBuff );
		else
			rsServerConfig.Event_WhiteDay = 100;

		flag++;
	}

	if ( !flag && ( lstrcmp( decode , "*\270\360\270\256\307\301\300\307\275\251\305\322_\300\314\272\245\306\256" )==0 ||
		lstrcmpi( decode , "*EVENT_MORIFSHELTOM" )==0 ) ) {

		p=fileread_GetWord(strBuff,p);
		if ( strBuff[0] )
			rsServerConfig.Event_MorifSheltom = atoi( strBuff );
		else
			rsServerConfig.Event_MorifSheltom = 1;

		flag++;
	}

	if ( !flag && ( lstrcmp( decode , "*\277\371\265\345\304\305_\300\314\272\245\306\256" )==0 ||
		lstrcmpi( decode , "*EVENT_WORLDCUP" )==0 ) ) {

		p=fileread_GetWord(strBuff,p);
		if ( strBuff[0] )
			rsServerConfig.Event_WorldCup = atoi( strBuff );
		else
			rsServerConfig.Event_WorldCup = 5;

		flag++;
	}

	// 장별 - 수박몬스터 이벤트
	if ( !flag && ( lstrcmp( decode , "*\274\366\271\332\270\363\275\272\305\315_\300\314\272\245\306\256" )==0 ||
	lstrcmpi( decode , "*EVENT_WATERMELONMONSTER" )==0 ) ) {

	p=fileread_GetWord(strBuff,p);
	if ( strBuff[0] )
		rsServerConfig.Event_WarerMelonMonster = atoi( strBuff );
	else
		rsServerConfig.Event_WarerMelonMonster = 1;

	flag++;
	}



	if ( !flag && ( lstrcmp( decode , "*\270\364\270\256\277\315\264\301\264\353_\300\314\272\245\306\256" )==0 ||
		lstrcmpi( decode , "*EVENT_MOLLYWOLF" )==0 ) ) {

		p=fileread_GetWord(strBuff,p);
		if ( strBuff[0] )
			rsServerConfig.Event_MollyWolf = atoi( strBuff );
		else
			rsServerConfig.Event_MollyWolf = 9;

		flag++;
	}

	// 박재원 - 복날 이벤트
	if ( !flag && ( lstrcmp( decode , "*\272\271\263\257_\300\314\272\245\306\256" )==0 || lstrcmpi( decode , "*EVENT_CHICHEN_DAY" )==0 ) ) 
	{
		p=fileread_GetWord(strBuff,p);
		if ( strBuff[0] )
			rsServerConfig.Event_Chichen_Day = atoi( strBuff );
		else
			rsServerConfig.Event_Chichen_Day = 1;

		flag++;
	}

	if ( !flag && ( lstrcmp( decode , "*\260\355\275\272\306\256_\300\314\272\245\306\256" )==0 ||
		lstrcmpi( decode , "*EVENT_GHOST" )==0 ) ) {

		p=fileread_GetWord(strBuff,p);
		if ( strBuff[0] )
			rsServerConfig.Event_Ghost = atoi( strBuff );
		else
			rsServerConfig.Event_Ghost = 2;

		flag++;
	}

	if ( !flag && ( lstrcmp( decode , "*\273\352\305\270\260\355\272\355\270\260_\300\314\272\245\306\256" )==0 ||
		lstrcmpi( decode , "*EVENT_SANTAGOBLIN" )==0 ) ) {

		p=fileread_GetWord(strBuff,p);
		if ( strBuff[0] )
			rsServerConfig.Event_SantaGoblin = atoi( strBuff );
		else
			rsServerConfig.Event_SantaGoblin = 5;

		flag++;
	}

	if ( !flag && ( lstrcmp( decode , "*\273\357\260\350\305\301_\300\314\272\245\306\256" )==0 ||
		lstrcmpi( decode , "*EVENT_SUMMER_CHICKEN" )==0 ) ) {

		p=fileread_GetWord(strBuff,p);
		if ( strBuff[0] )
			rsServerConfig.Event_SummerChicken = atoi( strBuff );
		else
			rsServerConfig.Event_SummerChicken = 5;

		flag++;
	}

	if ( !flag && ( lstrcmp( decode , "*\307\307\275\303\271\346\306\352_\300\314\272\245\306\256" )==0 ||
		lstrcmpi( decode , "*EVENT_PCBANG_PET" )==0 ) ) {

		p=fileread_GetWord(strBuff,p);
		if ( strBuff[0] )
			rsServerConfig.Event_PCBangPet = atoi( strBuff );
		else
			rsServerConfig.Event_PCBangPet = 1;

		flag++;
	}

	if ( !flag && ( lstrcmp( decode , "*\260\346\307\350\304\241\276\367_\300\314\272\245\306\256" )==0 ||
		lstrcmpi( decode , "*EVENT_EXPUP" )==0 ) ) {

		p=fileread_GetWord(strBuff,p);
		if ( strBuff[0] )
			rsServerConfig.Event_ExpUp = atoi( strBuff );
		else
			rsServerConfig.Event_ExpUp = 120;

// 해외

		if		( rsServerConfig.Event_ExpUp >= 100 && rsServerConfig.Event_ExpUp < 200 )	{ rsServerConfig.Event_ExpUp=100; }
		else if ( rsServerConfig.Event_ExpUp >= 200 && rsServerConfig.Event_ExpUp < 300 )	{ rsServerConfig.Event_ExpUp=200; }
		else if ( rsServerConfig.Event_ExpUp >= 300 && rsServerConfig.Event_ExpUp < 400 )	{ rsServerConfig.Event_ExpUp=300; }
		else if ( rsServerConfig.Event_ExpUp >= 400)										{ rsServerConfig.Event_ExpUp=400; }
		else																				{ rsServerConfig.Event_ExpUp=100; }

		flag++;
	}

#ifdef _LANGUAGE_ARGENTINA

		if ( !flag && ( lstrcmp( decode , "*\260\346\307\350\304\241\276\367_\300\314\272\245\306\256_\266\363\306\276" )==0 || lstrcmpi( decode , "*EVENT_EXPUP_LATIN" )==0 ) ) {

		p=fileread_GetWord(strBuff,p);
		if ( strBuff[0] )
		{
			rsServerConfig.Event_ExpUp_latin = atoi( strBuff );
		}
		else
		{
			rsServerConfig.Event_ExpUp_latin = 100;
		}

		flag++;
	}

#endif



	// pluto 선물상자
	if( !flag && ( lstrcmp( decode , "*\274\261\271\260\273\363\300\332_\300\314\272\245\306\256" )==0 ||
		lstrcmpi( decode , "*EVENT_HALLOWEEN" )==0 ) )	//해외용 Kyle
	{
		p=fileread_GetWord(strBuff,p);
		if ( strBuff[0] )
		{
			rsServerConfig.Event_GiftBox = atoi( strBuff );
		}
		else
		{
			rsServerConfig.Event_GiftBox = 0;
		}

		flag++;
	}

//#ifdef _LANGUAGE_VEITNAM
	//베트남 경험치 시간분배
	if ( !flag && ( lstrcmp( decode , "*\275\303\260\243\260\346\307\350\304\241" )==0 ||	lstrcmpi( decode , "*EXP_GAMETIME" )==0 ) ) 
	{
		p=fileread_GetWord(strBuff,p);
		if ( strBuff[0] )
			rsServerConfig.ExpGameTimeMode = atoi( strBuff );
		else
			rsServerConfig.ExpGameTimeMode = 1;
		
		flag++;
	}
//#endif

	if ( !flag && ( lstrcmp( decode , "*\272\355\267\271\275\272\304\263\275\275" )==0 ||
		lstrcmpi( decode , "*EVENT_BLESSCASTLE" )==0 ) ) {

		p=fileread_GetWord(strBuff,p);
		if ( strBuff[0] )
			rsServerConfig.BlessCastleMode = atoi( strBuff );
		else
			rsServerConfig.BlessCastleMode = 1;

		flag++;
	}
	if ( !flag && ( lstrcmp( decode , "*\260\370\274\272\275\303\260\243" )==0 ||
		lstrcmpi( decode , "*EVENT_SIEGE_TIME" )==0 ) ) {

		p=fileread_GetWord(strBuff,p);
		if ( strBuff[0] )
			rsServerConfig.sBlessCastle_StartTime[0] = atoi( strBuff );		//요일
		p=fileread_GetWord(strBuff,p);
		if ( strBuff[0] )
			rsServerConfig.sBlessCastle_StartTime[1] = atoi( strBuff );		//시간

		flag++;
	}

	if ( !flag && ( lstrcmp( decode , "*\300\314\272\245\306\256\272\361\306\256\274\263\301\244" )==0 ||
		lstrcmpi( decode , "*EVENT_BITMASK" )==0 ) ) {

		p=fileread_GetWord(strBuff,p);
		if ( strBuff[0] )
			rsServerConfig.EventPlay_BitMask = atoi( strBuff );

		flag++;
	}

	if ( !flag && ( lstrcmp( decode , "*\275\305\261\324\300\257\300\372\274\261\271\260" )==0 ||
		lstrcmpi( decode , "*DEFAULT_ITEM_USER" )==0 ) ) {

		p=fileread_GetWord(strBuff,p);
		if ( strBuff[0] )
			rsServerConfig.StartUser_PresentItem = atoi( strBuff );
		else
			rsServerConfig.StartUser_PresentItem = TRUE;

		flag++;
	}


	if ( !flag && ( lstrcmp( decode , "*\300\374\276\367\300\332\261\342\267\317" )==0 ||
		lstrcmpi( decode , "*RECORD_JOBCHANGER" )==0 ) ) {
		rsServerConfig.Record_JobChanger = TRUE;;
		flag++;
	}

	if ( !flag && ( lstrcmp( decode , "*\277\265\261\270\272\355\267\260\307\343\260\241" )==0 ||
		lstrcmpi( decode , "*ENABLE_BLOCK_FOREVER" )==0 ) ) {
		rsServerConfig.ForeverBlockMode = TRUE;;
		flag++;
	}

	if ( !flag && ( lstrcmp( decode , "*\260\346\307\350\304\241\277\300\302\367\271\374\300\247" )==0 ||
		lstrcmpi( decode , "*PERMIT_EXP" )==0 ) ) {
		pb=p;p=fileread_GetWord(strBuff,p);if(strBuff[0]==34)p=GetString(strBuff,pb);
		rsServerConfig.Permit_Exp = atoi(strBuff);
		flag++;
	}
	if ( !flag && ( lstrcmp( decode , "*\265\267\277\300\302\367\271\374\300\247" )==0 ||
		lstrcmpi( decode , "*PERMIT_MONEY" )==0 ) ) {
		pb=p;p=fileread_GetWord(strBuff,p);if(strBuff[0]==34)p=GetString(strBuff,pb);
		rsServerConfig.Permit_Money = atoi(strBuff);
		flag++;
	}

	if ( !flag && ( lstrcmp( decode , "*\307\301\267\316\274\274\275\272\275\303\260\243\260\250\275\303" )==0 ||
		lstrcmpi( decode , "*CLIENT_CHECK_PROCESS_TIME" )==0 ) ) {
		pb=p;p=fileread_GetWord(strBuff,p);if(strBuff[0]==34)p=GetString(strBuff,pb);
		rsServerConfig.ClientProcessTimeChk = atoi(strBuff);
		flag++;
	}
	if ( !flag && ( lstrcmp( decode , "*\307\301\267\316\274\274\275\272\275\303\260\243\301\276\267\341" )==0 ||
		lstrcmpi( decode , "*CLIENT_PROCESS_TIME_OUT" )==0 ) ) {
		rsServerConfig.ClientProcessTimeChkOut = TRUE;
		flag++;
	}
	if ( !flag && ( lstrcmp( decode , "*\271\260\276\340\260\271\274\366\310\256\300\316" )==0 ||
		lstrcmpi( decode , "*CHECK_POTION_COUNT" )==0 ) ) {
		pb=p;p=fileread_GetWord(strBuff,p);if(strBuff[0]==34)p=GetString(strBuff,pb);
		rsServerConfig.Dis_PotionError = atoi(strBuff);
		flag++;
	}
	if ( !flag && ( lstrcmp( decode , "*\274\255\271\366\260\346\307\350\304\241\273\347\277\353" )==0 ||
		lstrcmpi( decode , "*ENABLE_SERVER_EXP" )==0 ) ) {
		pb=p;p=fileread_GetWord(strBuff,p);if(strBuff[0]==34)p=GetString(strBuff,pb);

		if ( strBuff[0] ) rsServerConfig.ExpFixMode = atoi(strBuff);
		else rsServerConfig.ExpFixMode = 50;

		flag++;
	}

	if ( !flag && ( lstrcmp( decode , "*\272\355\267\260\300\257\300\372\260\346\267\316" )==0 || 
		lstrcmpi( decode , "*BLOCK_USER_PATH" )==0 ) ) {
		pb=p;p=fileread_GetWord(strBuff,p);if(strBuff[0]==34)p=GetString(strBuff,pb);
		lstrcpy( rsServerConfig.szBlockUserPath , strBuff );
		flag++;
	}

	if ( !flag && ( lstrcmp( decode , "*\273\347\277\353\275\303\260\243\307\245\275\303" )==0 || 
		lstrcmpi( decode , "*DISP_BILLING_TIME" )==0 ) ) {

		pb=p;p=fileread_GetWord(strBuff,p);if(strBuff[0]==34)p=GetString(strBuff,pb);
		if ( strBuff[0] )
			rsServerConfig.DispBillingTime = atoi( strBuff );
		else
			rsServerConfig.DispBillingTime = 1;

		flag++;
	}

	//오프라인 솔로 플레이 (계정 DB 없이 로그인 허용)
	if ( !flag && lstrcmpi( decode , "*OFFLINE_MODE" )==0 ) {
		rsServerConfig.OfflineMode = TRUE;
		flag++;
	}
	if ( !flag && lstrcmpi( decode , "*OFFLINE_PASSWORDS" )==0 ) {
		rsServerConfig.OfflinePasswords = TRUE;
		flag++;
	}
	// test server only: GM gold/exp/level commands without the rest of *TEST_SERVER
	if ( !flag && lstrcmpi( decode , "*GM_CHEATS" )==0 ) {
		rsServerConfig.GmCheats = TRUE;
		flag++;
	}
	// test server: no black list (high-level gear trips the old stat checks: "Hacking Alert ( 100 )")
	if ( !flag && lstrcmpi( decode , "*DISABLE_BLACKLIST" )==0 ) {
		rsServerConfig.NoBlacklist = TRUE;
		flag++;
	}

	if ( !flag && ( lstrcmp( decode , "*\267\316\261\327\300\316\261\335\301\366" )==0 ||
		lstrcmpi( decode , "*DISABLE_LOGIN" )==0 ) ) {

		pb=p;p=fileread_GetWord(strBuff,p);if(strBuff[0]==34)p=GetString(strBuff,pb);
		if ( strBuff[0] )
			rsServerConfig.Disable_LogIn = atoi( strBuff );
		else
			rsServerConfig.Disable_LogIn = 1;
		flag++;
	}
	if ( !flag && ( lstrcmp( decode , "*\267\316\261\327\305\245\261\335\301\366" )==0 || 
		lstrcmpi( decode , "*DISABLE_LOGQUE" )==0 ) ) {
		rsServerConfig.Disable_LogQue = TRUE;
		flag++;
	}
	if ( !flag && ( lstrcmp( decode , "*\267\316\261\327\276\306\277\364\310\256\300\316\261\335\301\366" )==0 || 
		lstrcmpi( decode , "*DISABLE_CHECK_LOGOUT" )==0 ) ) {
		rsServerConfig.Disable_CheckLogOut = TRUE;
		flag++;
	}

	if ( !flag && ( lstrcmp( decode , "*\267\316\261\327\300\316\302\367\264\334" )==0 || 
		lstrcmpi( decode , "*DISABLE_LOGINX" )==0 ) ) {
		rsServerConfig.Disable_LogIn2 = TRUE;
		flag++;
	}

	if ( !flag && ( lstrcmp( decode , "*\307\307\275\303\271\346\261\335\301\366" )==0 || 
		lstrcmpi( decode , "*DISABLE_PCBANG" )==0 ) ) {
		rsServerConfig.Disable_PCBang = TRUE;
		flag++;
	}


	if ( !flag && ( lstrcmp( decode , "*\271\253\267\341\267\271\272\247" )==0 || 
		lstrcmpi( decode , "*FREE_LEVEL" )==0 ) ) {
		pb=p;p=fileread_GetWord(strBuff,p);if(strBuff[0]==34)p=GetString(strBuff,pb);
		rsServerConfig.FreeLevel = atoi(strBuff);					//무료 레벨

		pb=p;p=fileread_GetWord(strBuff,p);if(strBuff[0]==34)p=GetString(strBuff,pb);
		if ( strBuff[0] )
			lstrcpy( rsServerConfig.szFreeGpCode , strBuff );		//무료 GPCODE

		flag++;
	}

	if ( !flag && ( lstrcmp( decode , "*\274\261\271\260\267\271\272\247" )==0 || 
		lstrcmpi( decode , "*PRESENT_LEVEL" )==0 ) ) {
		pb=p;p=fileread_GetWord(strBuff,p);if(strBuff[0]==34)p=GetString(strBuff,pb);
		rsServerConfig.PresentLevel = atoi(strBuff);
		flag++;
	}

	if ( !flag && ( lstrcmp( decode , "*\265\277\267\341\276\306\300\314\305\333\303\342\267\302" )==0 || 
		lstrcmpi( decode , "*ENABLE_PARTYITEM" )==0 ) ) {

		rsServerConfig.Enable_PartyItem = TRUE;
		flag++;
	}

	if ( !flag && ( lstrcmp( decode , "*\307\312\265\345\301\366\307\374\260\313\273\347" )==0 || 
		lstrcmpi( decode , "*CHECK_PLAYFIELD" )==0 ) ) {

		rsServerConfig.CheckPlayField = TRUE;
		flag++;
	}

		//스킨핵 방지
	if ( !flag && ( lstrcmp( decode , "*\270\360\265\250\306\304\300\317\260\313\273\347" )==0 || 
		lstrcmpi( decode , "*CHECK_MODELFILE" )==0 ) ) {

		rsServerConfig.CheckModelFile = TRUE;
		flag++;
	}
		

	if ( !flag && ( lstrcmp( decode , "*\301\242\274\323\301\246\307\321IP" )==0 || 
		lstrcmpi( decode , "*DISABLE_IP" )==0 ) ) {

		if ( rsServerConfig.DisableIP_Count<DISABLE_IP_MAX ) {
			p=fileread_GetWord(strBuff,p);
			rsServerConfig.DisableIP[rsServerConfig.DisableIP_Count][0] = GetSwapIPCode( strBuff );
			p=fileread_GetWord(strBuff,p);
			rsServerConfig.DisableIP[rsServerConfig.DisableIP_Count][1] = GetSwapIPCode( strBuff );
			rsServerConfig.DisableIP_Count++;
		}

		flag++;
	}

	if ( !flag && ( lstrcmp( decode , "*\301\242\274\323\307\343\260\241IP" )==0 || 
		lstrcmpi( decode , "*ENABLE_IP" )==0 ) ) {

		if ( rsServerConfig.EnableIP_Count<ENABLE_IP_MAX ) {
			p=fileread_GetWord(strBuff,p);
			rsServerConfig.EnableIP[rsServerConfig.EnableIP_Count][0] = GetSwapIPCode( strBuff );
			p=fileread_GetWord(strBuff,p);
			rsServerConfig.EnableIP[rsServerConfig.EnableIP_Count][1] = GetSwapIPCode( strBuff );
			rsServerConfig.EnableIP_Count++;
		}

		flag++;
	}

	if ( !flag && ( lstrcmp( decode , "*\260\374\270\256\301\242\274\323IP" )==0 || 
		lstrcmpi( decode , "*ADMINISTRATOR_IP" )==0 ) ) {

		if ( rsServerConfig.AdminIP_Count<ADMIN_IP_MAX ) {
			p=fileread_GetWord(strBuff,p);
			rsServerConfig.AdminIP[rsServerConfig.AdminIP_Count][0] = GetSwapIPCode( strBuff );
			p=fileread_GetWord(strBuff,p);
			rsServerConfig.AdminIP[rsServerConfig.AdminIP_Count][1] = GetSwapIPCode( strBuff );
			rsServerConfig.AdminIP_Count++;
		}

		flag++;
	}

	if ( !flag && ( lstrcmp( decode , "*\275\303\275\272\305\333IP" )==0 || 
		lstrcmpi( decode , "*SYSTEM_IP" )==0 ) ) {

		if ( rsServerConfig.SystemIP_Count<ADMIN_IP_MAX ) {
			p=fileread_GetWord(strBuff,p);
			rsServerConfig.SystemIP[rsServerConfig.SystemIP_Count][0] = GetSwapIPCode( strBuff );
			p=fileread_GetWord(strBuff,p);
			rsServerConfig.SystemIP[rsServerConfig.SystemIP_Count][1] = GetSwapIPCode( strBuff );
			rsServerConfig.SystemIP_Count++;
		}

		flag++;
	}

	if ( !flag && ( lstrcmp( decode , "*\267\316\261\327\300\316\274\255\271\366IP" )==0 || 
		lstrcmpi( decode , "*LOGIN_SERVER_IP" )==0 ) ) {

		if ( rsServerConfig.SystemIP_Count<LOGIN_IP_MAX ) {
			p=fileread_GetWord(strBuff,p);
			rsServerConfig.LoginServerIP[rsServerConfig.LoginServerIP_Count][0] = GetSwapIPCode( strBuff );
			p=fileread_GetWord(strBuff,p);
			rsServerConfig.LoginServerIP[rsServerConfig.LoginServerIP_Count][1] = GetSwapIPCode( strBuff );
			rsServerConfig.LoginServerIP_Count++;
		}

		flag++;
	}

	if ( !flag && ( lstrcmp( decode , "*\274\255\271\366\300\316\272\245\305\344\270\256" )==0 || 
		lstrcmpi( decode , "*SERVER_INVENTORY" )==0 ) ) {

		rsServerConfig.ServerInventoryMode = TRUE;
		flag++;
	}

	if ( !flag && ( lstrcmp( decode , "*\271\351\276\367\306\372\264\365\300\314\270\247" )==0 || 
		lstrcmpi( decode , "*BACKUP_PATH" )==0 ) ) {
		pb=p;p=fileread_GetWord(strBuff,p);if(strBuff[0]==34)p=GetString(strBuff,pb);
		lstrcpy( rsServerConfig.szBackupPath , strBuff );
		flag++;
	}

	if ( !flag && ( lstrcmp( decode , "*\275\272\305\263\306\367\300\316\306\256\260\250\274\322\272\322\260\241" )==0 || 
		lstrcmpi( decode , "*DISABLE_DEC_SKILLPOINT" )==0 ) ) {

		rsServerConfig.Disable_DecSkillPoint = TRUE;
		flag++;
	}

	if ( !flag && ( lstrcmp( decode , "*\304\371\275\272\306\256\261\335\301\366" )==0 || 
		lstrcmpi( decode , "*DISABLE_QUEST" )==0 ) ) {

		pb=p;p=fileread_GetWord(strBuff,p);if(strBuff[0]==34)p=GetString(strBuff,pb);
		rsServerConfig.Disable_Quest = atoi(strBuff);
		flag++;
	}

	if ( !flag && ( lstrcmp( decode , "*\275\303\300\333\307\312\265\345\260\355\301\244" )==0 || 
		lstrcmpi( decode , "*START_FIELD" )==0 ) ) {

		pb=p;p=fileread_GetWord(strBuff,p);if(strBuff[0]==34)p=GetString(strBuff,pb);
		if ( strBuff[0] )
			rsServerConfig.FixedStartField = atoi(strBuff);
		flag++;
	}

	if ( !flag && ( lstrcmp( decode , "*\276\306\300\314\305\333\271\366\270\262\261\335\301\366" )==0 || 
		lstrcmpi( decode , "*DISABLE_THROWITEM" )==0 ) ) {

		pb=p;p=fileread_GetWord(strBuff,p);if(strBuff[0]==34)p=GetString(strBuff,pb);
		if ( strBuff[0] )
			rsServerConfig.DisableThrowItem = atoi(strBuff);
		else
			rsServerConfig.DisableThrowItem = 1;

		flag++;
	}



	if ( !flag && ( lstrcmp( decode , "*\305\251\267\271\300\314\301\366\306\320\305\266" )==0 || 
		lstrcmpi( decode , "*CRAZY_PACKET" )==0 ) ) {

		rsServerConfig.CrazyPacketMode = TRUE;
		flag++;
	}

	if ( !flag && ( lstrcmp( decode , "*\305\353\272\270\301\326\274\322" )==0 || 
		lstrcmpi( decode , "*NOTICE_URL" )==0 ) ) {
		pb=p;p=fileread_GetWord(strBuff,p);if(strBuff[0]==34)p=GetString(strBuff,pb);
		lstrcpy( rsServerConfig.szNoticeURL , strBuff );
		flag++;
	}

	if ( !flag && ( lstrcmp( decode , "*\260\374\270\256\270\355\267\311\261\342\267\317" )==0 || 
		lstrcmpi( decode , "*REC_ADMIN_CMD" )==0 ) ) {
		pb=p;p=fileread_GetWord(strBuff,p);if(strBuff[0]==34)p=GetString(strBuff,pb);
		if ( strBuff[0] )
			rsServerConfig.RecordAdminCommand = atoi( strBuff );
		else
			rsServerConfig.RecordAdminCommand = 2;

		flag++;
	}


	if ( !flag && ( lstrcmp( decode , "*\274\255\271\366\277\254\260\341IP" )==0 || 
		lstrcmpi( decode , "*SERVER_LINK_IP" )==0 ) ) {
		pb=p;p=fileread_GetWord(strBuff,p);if(strBuff[0]==34)p=GetString(strBuff,pb);
		lstrcpy( rsServerConfig.szLinkServerIP , strBuff );
		flag++;
	}


	if ( !flag && ( lstrcmp( decode , "*\301\366\277\252\274\255\271\366" )==0 || 
		lstrcmpi( decode , "*AREA_SERVER" )==0 ) ) {

		for( cnt=0;cnt<32;cnt++ ) {
			if ( rsServerConfig.AreaServer.IP_Count>=SERVER_MAP_MAX ) break;

			pb=p;p=fileread_GetWord(strBuff,p);if(strBuff[0]==34)p=GetString(strBuff,pb);
			if ( !strBuff[0] ) break;

			rsServerConfig.AreaServer.dwIP[ rsServerConfig.AreaServer.IP_Count ] = inet_addr( strBuff );

			rsServerConfig.AreaServer.IP_Count++;
			rsServerConfig.AreaServer.size = sizeof(TRANS_SEVER_MAP)-(sizeof(DWORD)*(SERVER_MAP_MAX-rsServerConfig.AreaServer.IP_Count));
			rsServerConfig.AreaServer.code = smTRANSCODE_AREA_SERVER_MAP;
		}
		flag++;
	}

	if ( !flag && ( lstrcmp( decode , "*\305\344\263\312\270\325\306\256\260\350\301\244\274\255\271\366" )==0 || 
		lstrcmpi( decode , "*TT_DATA_SERVER" )==0 ) ) {

			cnt = rsServerConfig.TT_DataServer_Count;

			if ( cnt>=TT_DATA_SERVER_MAX ) goto skip_t;

			pb=p;p=fileread_GetWord(strBuff,p);if(strBuff[0]==34)p=GetString(strBuff,pb);
			if ( !strBuff[0] ) goto skip_t;
			lstrcpy( rsServerConfig.TT_DataServer[cnt].szServerID , strBuff );

			pb=p;p=fileread_GetWord(strBuff,p);if(strBuff[0]==34)p=GetString(strBuff,pb);
			if ( !strBuff[0] ) goto skip_t;
			lstrcpy( rsServerConfig.TT_DataServer[cnt].szServerPath, strBuff );

			rsServerConfig.TT_DataServer_Count++;
skip_t:
		flag++;
	}


	return flag;
}

#else

//서버 설정 서브 루틴
int SetServerConfig( char *decode , char *lpPoint )
{

	return NULL;
}

#endif

//ODBC 설정 서브 루틴
int SetOdbcConfig( char *decode , char *lpPoint )
{
	int flag;
	char *p = lpPoint;
	char *pb;
	char strBuff[128];

	flag = 0;

	if ( lstrcmp( decode , "*\300\316\301\365\301\242\274\323\274\255\272\361\275\272" )==0 ||
		lstrcmpi( decode , "*ODBC_NAME" )==0 ) {
		pb=p;p=fileread_GetWord(strBuff,p);if(strBuff[0]==34)p=GetString(strBuff,pb);
		lstrcpy( Odbc_Config.Dsn , strBuff );
		flag++;
	}
	if ( !flag && ( lstrcmp( decode , "*\300\316\301\365\301\242\274\323\300\314\270\247" )==0 ||
		lstrcmpi( decode , "*ODBC_ACCOUNT" )==0 ) ) {
		pb=p;p=fileread_GetWord(strBuff,p);if(strBuff[0]==34)p=GetString(strBuff,pb);
		lstrcpy( Odbc_Config.Name , strBuff );
		flag++;
	}
	if ( !flag && ( lstrcmp( decode , "*\300\316\301\365\301\242\274\323\276\317\310\243" )==0 ||
		lstrcmpi( decode , "*ODBC_PASSWORD" )==0 ) ) {
		pb=p;p=fileread_GetWord(strBuff,p);if(strBuff[0]==34)p=GetString(strBuff,pb);
		lstrcpy( Odbc_Config.Password , strBuff );
		flag++;
	}
	if ( !flag && ( lstrcmp( decode , "*\305\327\300\314\272\355\300\314\270\247" )==0 ||
		lstrcmpi( decode , "*ODBC_TABLE" )==0 ) ) {
		pb=p;p=fileread_GetWord(strBuff,p);if(strBuff[0]==34)p=GetString(strBuff,pb);
		lstrcpy( Odbc_Config.Table , strBuff );
		flag++;
	}
	if ( !flag && ( lstrcmp( decode , "*\305\327\300\314\272\355\260\350\301\244" )==0 ||
		lstrcmpi( decode , "*ODBC_TABLE_IDNAME" )==0 ) ) {
		pb=p;p=fileread_GetWord(strBuff,p);if(strBuff[0]==34)p=GetString(strBuff,pb);
		lstrcpy( Odbc_Config.Table_Id , strBuff );
		flag++;
	}

	if ( !flag && ( lstrcmp( decode , "*\305\327\300\314\272\355\276\317\310\243\277\255" )==0 ||
		lstrcmpi( decode , "*ODBC_TABLE_PASSWORD" )==0 ) ) {
		pb=p;p=fileread_GetWord(strBuff,p);if(strBuff[0]==34)p=GetString(strBuff,pb);
		Odbc_Config.Table_Password = atoi(strBuff);
		flag++;
	}
	if ( !flag && ( lstrcmp( decode , "*\305\327\300\314\272\355\307\343\260\241\277\255" )==0 ||
		lstrcmpi( decode , "*ODBC_TABLE_PLAY" )==0 ) ) {
		pb=p;p=fileread_GetWord(strBuff,p);if(strBuff[0]==34)p=GetString(strBuff,pb);
		Odbc_Config.Table_Play = atoi(strBuff);
		flag++;
	}

	if ( !flag && ( lstrcmp( decode , "*\305\327\275\272\306\256\300\374\277\353\300\316\301\365" )==0 ||
		lstrcmp( decode , "*LOGIN_TESTER" )==0 ) ) {
		Odbc_Config.CloseAccount = TRUE;
		flag++;
	}
	
	return flag;
}


//SERVER 목록 설정 서브 루틴
int SetServerSelectList( char *decode , char *lpPoint )
{
	int flag;
	char *p = lpPoint;
	char *pb;
	char strBuff[128];
	int cnt;

	flag = 0;

	if ( TransServerList.ServerCount>=TRANS_SERVER_MAX ) return FALSE;

	if ( lstrcmp( decode , "*\301\242\274\323\274\255\271\366\301\244\272\270" )==0 || 
		lstrcmpi( decode , "*GAME_SERVER" )==0  ) {
		cnt = TransServerList.ServerCount;

		pb=p;p=fileread_GetWord(strBuff,p);if(strBuff[0]==34)p=GetString(strBuff,pb);
		if ( strBuff[0] ) {
			lstrcpy( TransServerList.ServerInfo[cnt].szServerName , strBuff );			//서버 이름
		}

		pb=p;p=fileread_GetWord(strBuff,p);if(strBuff[0]==34)p=GetString(strBuff,pb);
		if ( strBuff[0] ) {
			lstrcpy( TransServerList.ServerInfo[cnt].szServerIp1 , strBuff );			//서버 이름
			TransServerList.ServerInfo[cnt].dwServerPort1 = TCP_SERVPORT;
		}

		pb=p;p=fileread_GetWord(strBuff,p);if(strBuff[0]==34)p=GetString(strBuff,pb);
		if ( strBuff[0] ) {
			lstrcpy( TransServerList.ServerInfo[cnt].szServerIp2 , strBuff );			//서버 이름 ( 예비 )
			TransServerList.ServerInfo[cnt].dwServerPort2 = TCP_SERVPORT;
		}

		pb=p;p=fileread_GetWord(strBuff,p);if(strBuff[0]==34)p=GetString(strBuff,pb);
		if ( strBuff[0] ) {
			lstrcpy( TransServerList.ServerInfo[cnt].szServerIp3 , strBuff );			//서버 이름 ( 예비 )
			TransServerList.ServerInfo[cnt].dwServerPort3 = TCP_SERVPORT;
		}

		TransServerList.ServerCount++;
		flag++;
	}

	if ( !flag && ( lstrcmp( decode , "*\305\254\267\243\274\255\271\366\301\244\272\270" )==0 || 
		lstrcmpi( decode , "*CLAN_SERVER" )==0 ) ) {

		cnt = TransServerList.ServerCount+TransServerList.ClanServerCount;

		lstrcpy( TransServerList.ServerInfo[cnt].szServerName , "Clan" );

		pb=p;p=fileread_GetWord(strBuff,p);if(strBuff[0]==34)p=GetString(strBuff,pb);
		if ( strBuff[0] )
			lstrcpy( TransServerList.ServerInfo[cnt].szServerIp1 , strBuff );			//IP

		pb=p;p=fileread_GetWord(strBuff,p);if(strBuff[0]==34)p=GetString(strBuff,pb);
		if ( strBuff[0] )
			TransServerList.ServerInfo[cnt].dwServerPort1 = atoi(strBuff);				//Port

		pb=p;p=fileread_GetWord(strBuff,p);if(strBuff[0]==34)p=GetString(strBuff,pb);
		if ( strBuff[0] )
			lstrcpy( TransServerList.ServerInfo[cnt].szServerIp2 , strBuff );			//IP

		pb=p;p=fileread_GetWord(strBuff,p);if(strBuff[0]==34)p=GetString(strBuff,pb);
		if ( strBuff[0] )
			TransServerList.ServerInfo[cnt].dwServerPort2 = atoi(strBuff);				//Port

		pb=p;p=fileread_GetWord(strBuff,p);if(strBuff[0]==34)p=GetString(strBuff,pb);
		if ( strBuff[0] )
			lstrcpy( TransServerList.ServerInfo[cnt].szServerIp3 , strBuff );			//IP

		pb=p;p=fileread_GetWord(strBuff,p);if(strBuff[0]==34)p=GetString(strBuff,pb);
		if ( strBuff[0] )
			TransServerList.ServerInfo[cnt].dwServerPort3 = atoi(strBuff);				//Port

		TransServerList.ClanServerCount++;
		flag++;
	}

	
	return flag;
}

/////////////// 레지스트리 설정값 읽어옴 ///////////////
smCONFIG *smConfigReg()
{
	MAKELOG("now entering the smConfigReg() function");

	char *buff;
#ifdef _LANGUAGE_WINDOW
	MAKELOG("#ifdef _LANGUAGE_WINDOW");
	buff = GetRegString( HKEY_LOCAL_MACHINE , InfoRegPath , "FullMode" );
	if ( buff )
	{
		if ( lstrcmpi( buff , "off" )==0 )
		{
			smConfig.WinMode = 1;
			MAKELOG("smConfig.WinMode = 1");
		}
		else
		{
			smConfig.WinMode = 0;
			MAKELOG("smConfig.WinMode = 0");
		}
	}
	else
	{
		MAKELOG("cant find the 'FullMode' registry");
	}
	/*
	else
	{
		#ifndef _W_SERVER
		#ifdef _LANGUAGE_CHINESE//(중국 레지값 임시로 등록(테스트))
				SetRegString( HKEY_LOCAL_MACHINE, InfoRegPath , "FullMode" , "off");
		#endif
		#endif
	}
	*/
#endif

	buff = GetRegString( HKEY_LOCAL_MACHINE , InfoRegPath , "Graphic" );
	if ( buff ) {
		smConfig.TextureQuality = atoi( buff );
	}

	buff = GetRegString( HKEY_LOCAL_MACHINE , InfoRegPath , "ColorBPP" );
	if ( buff ) {
		if ( atoi(buff)==32 ) smConfig.ScreenColorBit = 32;
	}


	buff = GetRegString( HKEY_LOCAL_MACHINE , InfoRegPath , "Network" );
	if ( buff ) {
		smConfig.NetworkQuality = atoi( buff );
	}


	buff = GetRegString( HKEY_LOCAL_MACHINE , InfoRegPath , "CameraInvert" );
	if ( buff ) {
		if ( lstrcmpi( buff , "true" )==0 ) 
			CameraInvRot = 1;
		else
			CameraInvRot = 0;
	}

	buff = GetRegString( HKEY_LOCAL_MACHINE , InfoRegPath , "MotionBlur" );
	if ( buff ) {
		if ( lstrcmpi( buff , "true" )==0 ) 
			g_IsCreateFilterEffect = 1;
		else
			g_IsCreateFilterEffect = 0;
	}

	buff = GetRegString( HKEY_LOCAL_MACHINE , InfoRegPath , "CameraSight" );
	if ( buff ) {
		if ( lstrcmpi( buff , "ON" )==0 ) 
			CameraSight = 1;
		else
			CameraSight = 0;
	}

	buff = GetRegString( HKEY_LOCAL_MACHINE , InfoRegPath , "ScreenSize" );
	if ( buff ) {
		switch( atoi(buff) ) {
			case 0:
			{
				smConfig.ScreenSize.x = 800;
				smConfig.ScreenSize.y = 600;
				MAKELOG(" ScreenSize.x = 800 , ScreenSize.y = 600");
			}break;

			case 1:
			{
				smConfig.ScreenSize.x = 1024;
				smConfig.ScreenSize.y = 768;
				MAKELOG(" ScreenSize.x = 1024 , ScreenSize.y = 768");
			}break;

			case 2:
			{				
				smConfig.ScreenSize.x = 1280;
				smConfig.ScreenSize.y = 1024;
				MAKELOG(" ScreenSize.x = 1280 , ScreenSize.y = 1024");
			}break;
		}
	}
	else
	{
		MAKELOG("cant find the 'ScreenSize' registry");
	}

	buff = GetRegString( HKEY_LOCAL_MACHINE , InfoRegPath , "Sound" );
	if ( buff ) {
		if ( lstrcmpi( buff , "off" )==0 ) 
			smConfig.BGM_Mode = 0;
		else
			smConfig.BGM_Mode = 1;
	}

	// pluto 해외 비
	buff = GetRegString( HKEY_LOCAL_MACHINE , InfoRegPath , "RainMode" );
	if ( buff ) {
		if ( lstrcmpi( buff , "Off" )==0 ) 
			smConfig.WeatherSwitch = 0;
		else
			smConfig.WeatherSwitch = 1;
	}

	buff = GetRegString( HKEY_LOCAL_MACHINE , InfoRegPath , "Server1" );
	if ( buff ) {
		lstrcpy( smConfig.szDataServerIP , buff );

		MAKELOG(" 'Server1' the value of  smConfig.szDataServerIP is ->");
		MAKELOG( buff );
	}
	buff = GetRegString( HKEY_LOCAL_MACHINE , InfoRegPath , "Server2" );
	if ( buff ) {
		lstrcpy( smConfig.szServerIP , buff );
		lstrcpy( smConfig.szExtendServerIP , buff );

		MAKELOG(" 'Server2' the value of  smConfig.szServerIP , szExtendServerIP is ->");
		MAKELOG( buff );
	}
	buff = GetRegString( HKEY_LOCAL_MACHINE , InfoRegPath , "Server3" );
	if ( buff ) {
		lstrcpy( smConfig.szUserServerIP , buff );
		MAKELOG(" 'Server3' the value of  smConfig.szUserServerIP is ->");
		MAKELOG( buff );
	}

	smConfig.dwDataServerPort = TCP_SERVPORT;
	smConfig.dwServerPort = TCP_SERVPORT;
	smConfig.dwUserServerPort = TCP_SERVPORT;
	smConfig.dwExtendServerPort = TCP_SERVPORT;

	//디버그 플랙 백업 ( 해킹 방지 )
	dwDebugXor = GetCurrentTime();
	dwDebugBack = smConfig.DebugMode^dwDebugXor;

	return &smConfig;
}



//정보 파일을 해독하여 돌려준다 (Info에저장)
smCONFIG *smConfigDecode( char *iniFile  )
{
	FILE	*fp = NULL;
	char strBuff[64];
	char *p,*pb;
	int CmdNum;
	int MapLightAngX,MapLightAngY;
	int x,y,z;
	int cnt;

	smCONFIG *Config = &smConfig;

	srAdminCmdCnt = 0;

	IniSetWinMode = FALSE;
	IniSetScreenSize = FALSE;
	ZeroMemory( Config , sizeof( smCONFIG ) );
	ZeroMemory( &Odbc_Config , sizeof( ODBC_CONFIG ) );
	ZeroMemory( &TransServerList , sizeof( TRANS_SERVER_LIST ) );
	ZeroMemory( &rsServerConfig , sizeof( rsSERVER_CONFIG ) );

	//기본 설정
	Config->WinMode = 0;
	Config->ScreenSize.x = 800;
	Config->ScreenSize.y = 600;
	Config->ScreenColorBit = 16;
	Config->StageObjCnt = 0;
	Config->TextureQuality = 0;		//기본 1
	Config->NetworkQuality = 2;		//기본 2
	Config->BGM_Mode = 1;
	Config->WeatherSwitch = 0;	// pluto 해외 비


	Config->MapBright = DEFAULT_BRIGHT;
	Config->MapContrast = DEFAULT_CONTRAST;
	Config->MapLightVector.x = fONE;
	Config->MapLightVector.y = -fONE;
	Config->MapLightVector.z = fONE/2;

	//return smConfigReg();					//레지스트리 설정 적용

	rsServerConfig.Enable_PKField = -1;
	rsServerConfig.Enable_PKField_All = 0;


	////////////////////////// 디버그 모드 설정 //////////////////////


	fp = fopen( iniFile , "rb" );
	
	if( fp != NULL )
	{
		MAKELOG("fp = fopen( iniFile , 'rb' ) : success");
	}
	else
	{
		MAKELOG("fp = fopen( iniFile , 'rb' ) : failure , so call the smConfigReg() function.");
	}

	if ( fp==NULL ) {
		return smConfigReg();					//레지스트리 설정 적용
	}

	while( !feof( fp ) )//  feof: file end까지 읽어라 
	{
		if( fgets( line, 255, fp ) == NULL)	break;

		p = fileread_GetWord( decode , line);

		CmdNum = 0;

		while(1) {

			if ( lstrcmp( decode, szConfigIniCommand[CmdNum] ) == 0 ) {
				break;
			}
			if ( szConfigIniCommand[CmdNum]==NULL ) break;
			CmdNum++;

#ifdef _W_SERVER
			//////////// 서버용 설정 /////////////////////////////////
			if ( SetOdbcConfig( decode , p ) ) break;			// ODBC
			if ( SetServerSelectList( decode , p ) ) break;		// 접속 서버 목록
			if ( SetServerConfig( decode , p ) ) break;			//서버설정
			//////////////////////////////////////////////////////////
#endif
		}

		switch( CmdNum ) {
		case 0:
			//"*화면모드",
			p = fileread_GetWord( strBuff , p );
#ifdef _W_SERVER
			if ( lstrcmpi( strBuff , szWordWinMode[2])==0 ) {				//서버 모드
				Config->WinMode = -1;
				break;
			}
#endif

#ifdef _WINMODE_DEBUG
			if ( lstrcmpi( strBuff , szWordWinMode[1] )==0 ) 
				Config->WinMode = 1;
			else
				Config->WinMode = 0;
#else 
				Config->WinMode = 0;
#endif
			IniSetWinMode = TRUE;
			break;

		case 1:
			//"*화면해상도",
			p = fileread_GetWord( strBuff , p );
			Config->ScreenSize.x = atoi(strBuff);
			p = fileread_GetWord( strBuff , p );
			Config->ScreenSize.y = atoi(strBuff);
			IniSetScreenSize = TRUE;
			break;

		case 2://
			//"*화면색상비트",
			p = fileread_GetWord( strBuff , p );
			//Config->ScreenColorBit = atoi(strBuff);
			break;

		case 3:
			//"*배경화면",
			p = fileread_GetWord( strBuff , p );
			lstrcpy( Config->szFile_BackGround[0] , strBuff );
			p = fileread_GetWord( strBuff , p );
			lstrcpy( Config->szFile_BackGround[1] , strBuff );
			break;

		case 4:
			//"*메뉴화면",
			p = fileread_GetWord( strBuff , p );
			lstrcpy( Config->szFile_Menu , strBuff );
			break;

		case 5:
#ifdef _WINMODE_DEBUG
			//"*주인공",
			p = fileread_GetWord( strBuff , p );
			lstrcpy( Config->szFile_Player , strBuff );
			p = fileread_GetWord( strBuff , p );
			Config->Posi_Player.x = (int)(atof(strBuff) * fONE);
			p = fileread_GetWord( strBuff , p );
			Config->Posi_Player.y = (int)(atof(strBuff) * fONE);
#endif
			break;

		case 6:
#ifdef _WINMODE_DEBUG
			//"*나쁜놈",
			p = fileread_GetWord( strBuff , p );
			lstrcpy( Config->szFile_Enemy , strBuff );
			p = fileread_GetWord( strBuff , p );
			Config->Posi_Enemy.x = (int)(atof(strBuff) * fONE);
			p = fileread_GetWord( strBuff , p );
			Config->Posi_Enemy.y = (int)(atof(strBuff) * fONE);
#endif
			break;

		case 7:
			//"*지형모델",
			p = fileread_GetWord( strBuff , p );
			lstrcpy( Config->szFile_Stage , strBuff );

			p = fileread_GetWord( strBuff , p );
			if ( strBuff[0] )
				Config->Posi_Stage.x = (int)(atof(strBuff) * fONE);

			p = fileread_GetWord( strBuff , p );

			if ( strBuff[0] )
				Config->Posi_Stage.y = (int)(atof(strBuff) * fONE);
			break;

		//######################################################################################
		//작 성 자 : 오 영 석
		case 8:
			//*지형물체			
			p = fileread_GetWord( strBuff, p );
			if( Config->StageObjCnt < FIELD_STAGE_OBJ_MAX )
			{
				lstrcpy( Config->szFile_StageObject[ Config->StageObjCnt ], strBuff );
				Config->IsStageAniObject[ Config->StageObjCnt ] = 0;
				Config->StageObjCnt++;
			}
			break;
		//######################################################################################

		case 9:
			//"*배경밝기",
			//p = fileread_GetWord( strBuff , p );
			//Config->MapBright = atoi(strBuff);
			break;

		case 10:
			//"*배경선명도",
			//p = fileread_GetWord( strBuff , p );
			//Config->MapContrast = atoi(strBuff);

			break;

		case 11:
			//"*조명각도",
			/*
			p = fileread_GetWord( strBuff , p );
			Config->MapLightVector.x = (int)(atof(strBuff)*fONE);
			p = fileread_GetWord( strBuff , p );
			Config->MapLightVector.z = (int)(atof(strBuff)*fONE);
			p = fileread_GetWord( strBuff , p );
			Config->MapLightVector.y = (int)(atof(strBuff)*fONE);
			*/

			/*
			p = fileread_GetWord( strBuff , p );
			MapLightAngY = atoi(strBuff) * 16;
			p = fileread_GetWord( strBuff , p );
			MapLightAngX = (128-atoi(strBuff)) * 16;
			MapLightAngX = (MapLightAngX - ANGLE_90) & ANGCLIP;

			MapLightAngY = 128 * 16;
			MapLightAngX = (128-10) * 16;
			MapLightAngX = (MapLightAngX - ANGLE_90) & ANGCLIP;

			x = 0;
			y = 0;
			z = (int)(1.5*fONE);

			smMATRIX rX , rY , mVect;
			int vx,vy,vz;

			smIdentityMatrix( rX );
			smIdentityMatrix( rY );

			smRotateYMatrix( rY , MapLightAngY );
			smRotateXMatrix( rX , MapLightAngX );
			smMatrixMult( mVect , rX , rY );

			vx = x * mVect._11 +
				 y * mVect._21 +
				 z * mVect._31 ;

			vy = x * mVect._12 +
				 y * mVect._22 +
				 z * mVect._32 ;

			vz = x * mVect._13 +
				 y * mVect._23 +
				 z * mVect._33 ;

			vx>>=FLOATNS;
			vy>>=FLOATNS;
			vz>>=FLOATNS;

			Config->MapLightVector.x = vx;
			Config->MapLightVector.y = vy;
			Config->MapLightVector.z = vz;
			*/

			break;
		case 12:
			//"*서버주소",
			p = fileread_GetWord( strBuff , p );
			lstrcpy( Config->szServerIP , strBuff );
			break;

		case 13:
			//"*서버포트",
			p = fileread_GetWord( strBuff , p );
			Config->dwServerPort = atoi( strBuff );
			break;

		case 14:
			//"*화면품질",
			p = fileread_GetWord( strBuff , p );

			if ( lstrcmp( strBuff , szWordTexQualty[0] )==0 ) {
				//좋음
				Config->TextureQuality = 0;
				break;
			}
			if ( lstrcmp( strBuff , szWordTexQualty[1] )==0 ) {
				//보통
				Config->TextureQuality = 1;
				break;
			}
			if ( lstrcmp( strBuff , szWordTexQualty[2] )==0 ) {
				//나쁨
				Config->TextureQuality = 2;
				break;
			}
			if ( lstrcmp( strBuff , szWordTexQualty[3] )==0 ) {
				//아주나쁨
				Config->TextureQuality = 3;
				break;
			}
			break;

		case 15:
#ifdef _WINMODE_DEBUG
			//"*관찰자모드",
			DebugPlayer = TRUE;
#endif
			break;

		case 16:
			//"*데이타서버주소",
			p = fileread_GetWord( strBuff , p );
			lstrcpy( Config->szDataServerIP , strBuff );
			break;

		case 17:
			//"*데이타서버포트",
			p = fileread_GetWord( strBuff , p );
			Config->dwDataServerPort = atoi( strBuff );
			break;

		case 18:
			//"*캐릭터서버주소",
			p = fileread_GetWord( strBuff , p );
			lstrcpy( Config->szUserServerIP , strBuff );
			break;
		case 19:
			//"*캐릭터서버포트",
			p = fileread_GetWord( strBuff , p );
			Config->dwUserServerPort = atoi( strBuff );
			break;


		case 20:
#ifdef _WINMODE_DEBUG
			//"*디버그모드" ( 내부 IP전용 )
			p = fileread_GetWord( strBuff , p );
			if ( lstrcmp( strBuff, szConfigIniCommand[CmdNum] )!=0 ) break;
			p = fileread_GetWord( strBuff , p );
			if ( lstrcmp( strBuff, szConfigIniCommand[CmdNum] )!=0 ) break;

			p = fileread_GetWord( strBuff , p );
			if ( strBuff[0]=='^' && strBuff[1]=='^' ) 
			{
				MAKELOG(" case 20: if ( strBuff[0]=='^' && strBuff[1]=='^' ) : success");
				Config->DebugMode = TRUE;
			}
#endif
			break;

		case 21:
		{

			#ifdef _WINMODE_DEBUG
			{
				//"*디버그모드" - 숨겨진 최고의 모드
				p = fileread_GetWord( strBuff , p );
				if ( lstrcmp( strBuff, szConfigIniCommand[CmdNum-1] )!=0 ) break;

				p = fileread_GetWord( strBuff , p );
				if ( strBuff[0]=='/' && strBuff[1]=='0' ) 
				{
					MAKELOG(" case 21: if ( strBuff[0]=='/' && strBuff[1]=='0' ) : success");
					Config->DebugMode = 0x100;
				}
			}
			#endif

		}break;

		case 22:
			//몬스터 키보드 설정
			for(cnt=0;cnt<CONFIG_KEY_MONSTER_MAX;cnt++) {
				pb=p;p=fileread_GetWord(strBuff,p);if(strBuff[0]==34)p=GetString(strBuff,pb);
				if ( !strBuff[0] ) break;			
				if ( Config->CmdMonsterCount>=CONFIG_KEY_MONSTER_MAX ) break;
				lstrcpy( Config->szCmdOpenMonster[Config->CmdMonsterCount++] , strBuff );
			}
			break;

		//######################################################################################
		//작 성 자 : 오 영 석
		case 23:
			// *지형애니
			p = fileread_GetWord( strBuff, p );
			if( Config->StageObjCnt < FIELD_STAGE_OBJ_MAX )
			{
				lstrcpy( Config->szFile_StageObject[ Config->StageObjCnt ], strBuff );
				Config->IsStageAniObject[ Config->StageObjCnt ] = 1;
				Config->StageObjCnt++;
			}
			break;
		//######################################################################################

		}
	}

	fclose(fp);

#ifdef	_WINMODE_DEBUG
#ifndef _LANGUAGE_VEITNAM	//윈도우 모드
#ifndef _LANGUAGE_CHINESE	//윈도우 모드
#ifndef _LANGUAGE_PHILIPIN	//윈도우 모드
#ifndef _LANGUAGE_ENGLISH	//윈도우 모드

	if ( !Config->DebugMode && Config->WinMode>=0 )
	{
		//디버그모드 아니면 화면 설정 불가
		Config->WinMode = 0;
		Config->ScreenSize.x = 800;
		Config->ScreenSize.y = 600;
		Config->ScreenColorBit = 16;
	}
#endif
#endif
#endif
#endif
#endif


			//"*조명각도",

			smMATRIX rX , rY , mVect;
			int vx,vy,vz;

			MapLightAngY = 128 * 16;
			MapLightAngX = (128-10) * 16;
			MapLightAngX = (MapLightAngX - ANGLE_90) & ANGCLIP;

			x = 0;
			y = 0;
			z = (int)(1.5*fONE);

			smIdentityMatrix( rX );
			smIdentityMatrix( rY );

			smRotateYMatrix( rY , MapLightAngY );
			smRotateXMatrix( rX , MapLightAngX );
			smMatrixMult( mVect , rX , rY );

			vx = x * mVect._11 +
				 y * mVect._21 +
				 z * mVect._31 ;

			vy = x * mVect._12 +
				 y * mVect._22 +
				 z * mVect._32 ;

			vz = x * mVect._13 +
				 y * mVect._23 +
				 z * mVect._33 ;

			vx>>=FLOATNS;
			vy>>=FLOATNS;
			vz>>=FLOATNS;

			Config->MapLightVector.x = vx;
			Config->MapLightVector.y = vy;
			Config->MapLightVector.z = vz;


			//"*배경밝기",
			Config->MapBright = 130;

			//"*배경선명도",
			Config->MapContrast = 300;


	//hotuk.ini 값이 레지스트리(런처 설정)보다 우선한다: *MODE WINDOW 가 무시되지 않도록
	int		IniWinMode = Config->WinMode;
	POINT	IniScreenSize = Config->ScreenSize;

	smConfigReg();					//레지스트리 설정 적용

	if ( IniSetWinMode )	Config->WinMode = IniWinMode;
	if ( IniSetScreenSize )	Config->ScreenSize = IniScreenSize;

	return &smConfig;
};



//단축 메세지 읽어 오기
int	ReadShotcutMessage( char *szFileName )
{
	FILE	*fp;
	int cnt;
	int	Count = 1;

	for(cnt=0;cnt<10;cnt++) {
		szShotCutMessage[cnt][0] = 0;
	}


	fp = fopen( szFileName , "rb" );
	if ( fp==NULL ) return FALSE;

	while( !feof( fp ) )					//  feof: file end까지 읽어라 
	{
		if( fgets( line, 255, fp ) == NULL)	
			break;

		line[99]=0;

		for(cnt=0;cnt<100;cnt++) {

			if ( line[cnt]=='\\' && ( line[cnt+1]=='r' || line[cnt+1]=='R' ) ) {
				line[cnt] = 0x0D;
				line[cnt+1] = 0;
				break;
			}

			if ( line[cnt]==0x0A || line[cnt]==0x0D || line[cnt]==0 ) {
				line[cnt] = 0;
				break;
			}
		}

		cnt = Count;
		if ( Count==10 ) cnt = 0;
		lstrcpy( szShotCutMessage[cnt] , line );

		Count++;
		if ( Count>10 ) break;
	}

	fclose(fp);

	return TRUE;

}

//파일 이름으로 부터 디렉토리를 찾아 설정
char *GetDirectoryFromFile( char *filename , char *szDirectory )
{
	int len;
	int cnt;

	if ( !szDirectory ) return NULL;

	lstrcpy( szDirectory , filename );

	len = lstrlen( szDirectory );

	for( cnt=len; cnt>=0 ; cnt-- )
		if ( szDirectory[cnt]=='\\' ) break;

	szDirectory[cnt+1] = NULL;

	return szDirectory;
}



//아이템 정보 파일을 해독하여 돌려준다 
int DecodeItemInfo( char *szItemFile , sDEF_ITEMINFO *lpDefItem  )
{
	FILE	*fp;
	char strBuff[128];
	char *p;
	char *pb;
	int cnt;
	int NextFlag;
	sITEMINFO	*lpItem;
	
	lpItem = &lpDefItem->Item;

	ZeroMemory( lpDefItem , sizeof( sDEF_ITEMINFO ) );

	fp = fopen( szItemFile , "rb" );
	if ( fp==NULL ) return FALSE;

	while( !feof( fp ) )					//  feof: file end까지 읽어라 
	{
		if( fgets( line, 255, fp ) == NULL)	
			break;

		p = fileread_GetWord( decode , line);
		NextFlag = 0;

		///////////////// 이름 ///////////////////////
		if ( lstrcmp( decode , "*\300\314\270\247" )==0 ) {
			pb=p;p=fileread_GetWord(strBuff,p);if(strBuff[0]==34)p=GetString(strBuff,pb);	//단어 찾기

			lstrcpy( lpItem->ItemName , strBuff );
			NextFlag = TRUE;
		}

#ifdef _LANGUAGE_CHINESE
		if ( lstrcmpi( decode , "*C_NAME" )==0 ) {
			pb=p;p=fileread_GetWord(strBuff,p);if(strBuff[0]==34)p=GetString(strBuff,pb);	//단어 찾기

			lstrcpy( lpItem->ItemName , strBuff );
			NextFlag = TRUE;
		}
#endif
#ifdef _LANGUAGE_JAPANESE
		if ( lstrcmpi( decode , "*J_NAME" )==0 ) {
			pb=p;p=fileread_GetWord(strBuff,p);if(strBuff[0]==34)p=GetString(strBuff,pb);	//단어 찾기

			lstrcpy( lpItem->ItemName , strBuff );
			NextFlag = TRUE;
		}
#endif
#ifdef _LANGUAGE_TAIWAN
		if ( lstrcmpi( decode , "*T_NAME" )==0 ) {
			pb=p;p=fileread_GetWord(strBuff,p);if(strBuff[0]==34)p=GetString(strBuff,pb);	//단어 찾기

			lstrcpy( lpItem->ItemName , strBuff );
			NextFlag = TRUE;
		}
#endif
#ifdef _LANGUAGE_ENGLISH
		if ( lstrcmpi( decode , "*E_NAME" )==0 ) {
			pb=p;p=fileread_GetWord(strBuff,p);if(strBuff[0]==34)p=GetString(strBuff,pb);	//단어 찾기

			lstrcpy( lpItem->ItemName , strBuff );
			NextFlag = TRUE;
		}
#endif
#ifdef _LANGUAGE_THAI
		if ( lstrcmpi( decode , "*TH_NAME" )==0 ) {
			pb=p;p=fileread_GetWord(strBuff,p);if(strBuff[0]==34)p=GetString(strBuff,pb);	//단어 찾기

			lstrcpy( lpItem->ItemName , strBuff );
			NextFlag = TRUE;
		}
#endif
#ifdef _LANGUAGE_VEITNAM
		if ( lstrcmpi( decode , "*V_NAME" )==0 ) {
			pb=p;p=fileread_GetWord(strBuff,p);if(strBuff[0]==34)p=GetString(strBuff,pb);	//단어 찾기

			lstrcpy( lpItem->ItemName , strBuff );
			NextFlag = TRUE;
		}
#endif
#ifdef _LANGUAGE_BRAZIL
		if ( lstrcmpi( decode , "*B_NAME" )==0 ) {
			pb=p;p=fileread_GetWord(strBuff,p);if(strBuff[0]==34)p=GetString(strBuff,pb);	//단어 찾기

			lstrcpy( lpItem->ItemName , strBuff );
			NextFlag = TRUE;
		}
#endif
//아르헨티나
#ifdef _LANGUAGE_ARGENTINA
		if ( lstrcmpi( decode , "*A_NAME" )==0 ) {
			pb=p;p=fileread_GetWord(strBuff,p);if(strBuff[0]==34)p=GetString(strBuff,pb);	//단어 찾기

			lstrcpy( lpItem->ItemName , strBuff );
			NextFlag = TRUE;
		}
#endif

		///////////////// 코드 ///////////////////////
		if ( !NextFlag && lstrcmp( decode , "*\304\332\265\345" )==0 ) {
			pb=p;p=fileread_GetWord(strBuff,p);if(strBuff[0]==34)p=GetString(strBuff,pb);	//단어 찾기

			for(cnt=0;cnt<MAX_ITEM;cnt++) {
				if ( lstrcmpi( strBuff , sItem[cnt].LastCategory )==0 ) {
					lpItem->CODE = sItem[cnt].CODE; break; }
			}

			NextFlag = TRUE;
		}

/////////////////////	유니크 아이템	 ////////////////////////////
		///////////////// 코드 ///////////////////////
		if ( !NextFlag && lstrcmp( decode , "*\300\257\264\317\305\251" )==0 ) {
			p=fileread_GetWord(strBuff,p);
			if ( strBuff[0] ) 
				lpItem->UniqueItem = atoi(strBuff);
			else
				lpItem->UniqueItem = TRUE;

			NextFlag = TRUE;
		}

		if ( !NextFlag && lstrcmp( decode , "*\300\257\264\317\305\251\273\366\273\363" )==0 ) {
			p=fileread_GetWord(strBuff,p);
			lpItem->EffectColor[0] = atoi(strBuff);			//R
			p=fileread_GetWord(strBuff,p);
			lpItem->EffectColor[1] = atoi(strBuff);			//G
			p=fileread_GetWord(strBuff,p);
			lpItem->EffectColor[2] = atoi(strBuff);			//B
			p=fileread_GetWord(strBuff,p);
			lpItem->EffectColor[3] = atoi(strBuff);			//A
			p=fileread_GetWord(strBuff,p);
			lpItem->EffectBlink[0] = atoi(strBuff);			//Blink
			p=fileread_GetWord(strBuff,p);
			if ( strBuff[0] ) 
				lpItem->ScaleBlink[0] = (int)(atof(strBuff)*fONE);	//Scale

			NextFlag = TRUE;
		}

		if ( !NextFlag && lstrcmp( decode , "*\300\314\306\345\306\256\274\263\301\244" )==0 ) {
			p=fileread_GetWord(strBuff,p);
			lpItem->DispEffect = atoi(strBuff);
			NextFlag = TRUE;
		}


/////////////////////	공통사항		 ////////////////////////////

		///////////////// 내구력 ///////////////////////
		if ( !NextFlag && lstrcmp( decode , "*\263\273\261\270\267\302" )==0 ) {
			p=fileread_GetWord(strBuff,p);
			lpDefItem->sDurability[0] = atoi(strBuff);
			p=fileread_GetWord(strBuff,p);
			lpDefItem->sDurability[1] = atoi(strBuff);
			NextFlag = TRUE;
		}

		///////////////// *무게 ///////////////////////
		if ( !NextFlag && lstrcmp( decode , "*\271\253\260\324" )==0 ) {
			p=fileread_GetWord(strBuff,p);
			lpItem->Weight = atoi(strBuff);
			NextFlag = TRUE;
		}

		///////////////// *가격 ///////////////////////
		if ( !NextFlag && lstrcmp( decode , "*\260\241\260\335" )==0 ) {
			p=fileread_GetWord(strBuff,p);
			lpItem->Price = atoi(strBuff);
			NextFlag = TRUE;
		}
//////////////////////      원소    ///////////////////////

		if ( !NextFlag && lstrcmp( decode , "*\273\375\303\274" )==0 ) {
			p=fileread_GetWord(strBuff,p);
			lpDefItem->sResistance[sITEMINFO_BIONIC][0] = atoi(strBuff);
			p=fileread_GetWord(strBuff,p);
			lpDefItem->sResistance[sITEMINFO_BIONIC][1] = atoi(strBuff);
			NextFlag = TRUE;
		}
		if ( !NextFlag && lstrcmp( decode , "*\264\353\300\332\277\254" )==0 ) {
			p=fileread_GetWord(strBuff,p);
			lpDefItem->sResistance[sITEMINFO_EARTH][0] = atoi(strBuff);
			p=fileread_GetWord(strBuff,p);
			lpDefItem->sResistance[sITEMINFO_EARTH][1] = atoi(strBuff);
			NextFlag = TRUE;
		}
		if ( !NextFlag && lstrcmp( decode , "*\272\322" )==0 ) {
			p=fileread_GetWord(strBuff,p);
			lpDefItem->sResistance[sITEMINFO_FIRE][0] = atoi(strBuff);
			p=fileread_GetWord(strBuff,p);
			lpDefItem->sResistance[sITEMINFO_FIRE][1] = atoi(strBuff);
			NextFlag = TRUE;
		}
		if ( !NextFlag && lstrcmp( decode , "*\263\303\261\342" )==0 ) {
			p=fileread_GetWord(strBuff,p);
			lpDefItem->sResistance[sITEMINFO_ICE][0] = atoi(strBuff);
			p=fileread_GetWord(strBuff,p);
			lpDefItem->sResistance[sITEMINFO_ICE][1] = atoi(strBuff);
			NextFlag = TRUE;
		}
		if ( !NextFlag && lstrcmp( decode , "*\271\370\260\263" )==0 ) {
			p=fileread_GetWord(strBuff,p);
			lpDefItem->sResistance[sITEMINFO_LIGHTING][0] = atoi(strBuff);
			p=fileread_GetWord(strBuff,p);
			lpDefItem->sResistance[sITEMINFO_LIGHTING][1] = atoi(strBuff);
			NextFlag = TRUE;
		}
		if ( !NextFlag && lstrcmp( decode , "*\265\266" )==0 ) {
			p=fileread_GetWord(strBuff,p);
			lpDefItem->sResistance[sITEMINFO_POISON][0] = atoi(strBuff);
			p=fileread_GetWord(strBuff,p);
			lpDefItem->sResistance[sITEMINFO_POISON][1] = atoi(strBuff);
			NextFlag = TRUE;
		}
		if ( !NextFlag && lstrcmp( decode , "*\271\260" )==0 ) {
			p=fileread_GetWord(strBuff,p);
			lpDefItem->sResistance[sITEMINFO_WATER][0] = atoi(strBuff);
			p=fileread_GetWord(strBuff,p);
			lpDefItem->sResistance[sITEMINFO_WATER][1] = atoi(strBuff);
			NextFlag = TRUE;
		}
		if ( !NextFlag && lstrcmp( decode , "*\271\331\266\367" )==0 ) {
			p=fileread_GetWord(strBuff,p);
			lpDefItem->sResistance[sITEMINFO_WIND][0] = atoi(strBuff);
			p=fileread_GetWord(strBuff,p);
			lpDefItem->sResistance[sITEMINFO_WIND][1] = atoi(strBuff);
			NextFlag = TRUE;
		}



///////////////	공격성능		/////////////

		///////////////// 공격력 ///////////////////////
		if ( !NextFlag && lstrcmp( decode , "*\260\370\260\335\267\302" )==0 ) {
			p=fileread_GetWord(strBuff,p);
			lpDefItem->sDamage[0] = atoi(strBuff);
			p=fileread_GetWord(strBuff,p);
			lpDefItem->sDamage[1] = atoi(strBuff);
			p=fileread_GetWord(strBuff,p);
			lpDefItem->sDamage[2] = atoi(strBuff);
			p=fileread_GetWord(strBuff,p);
			lpDefItem->sDamage[3] = atoi(strBuff);
			NextFlag = TRUE;
		}

		///////////////// *사정거리 ///////////////////////
		if ( !NextFlag && lstrcmp( decode , "*\273\347\301\244\260\305\270\256" )==0 ) {
			p=fileread_GetWord(strBuff,p);
			lpItem->Shooting_Range = atoi(strBuff);
			NextFlag = TRUE;
		}
		///////////////// *공격속도 ///////////////////////
		if ( !NextFlag && lstrcmp( decode , "*\260\370\260\335\274\323\265\265" )==0 ) {
			p=fileread_GetWord(strBuff,p);
			lpItem->Attack_Speed = atoi(strBuff);
			NextFlag = TRUE;
		}

		///////////////// *명중력 ///////////////////////
		if ( !NextFlag && lstrcmp( decode , "*\270\355\301\337\267\302" )==0 ) {
			p=fileread_GetWord(strBuff,p);
			lpDefItem->sAttack_Rating[0] = atoi(strBuff);
			p=fileread_GetWord(strBuff,p);
			lpDefItem->sAttack_Rating[1] = atoi(strBuff);
			NextFlag = TRUE;
		}

		///////////////// *크리티컬힛 ///////////////////////
		if ( !NextFlag && lstrcmp( decode , "*\305\251\270\256\306\274\304\303" )==0 ) {
			p=fileread_GetWord(strBuff,p);
			lpItem->Critical_Hit = atoi(strBuff);
			NextFlag = TRUE;
		}

//////////////	방어성능		/////////////


		///////////////// *흡수력 ///////////////////////
		if ( !NextFlag && lstrcmp( decode , "*\310\355\274\366\267\302" )==0 ) {
			p=fileread_GetWord(strBuff,p);
			lpDefItem->fAbsorb[0] = (float)atof(strBuff);
			p=fileread_GetWord(strBuff,p);
			lpDefItem->fAbsorb[1] = (float)atof(strBuff);
			NextFlag = TRUE;
		}

		///////////////// *방어력 ///////////////////////
		if ( !NextFlag && lstrcmp( decode , "*\271\346\276\356\267\302" )==0 ) {
			p=fileread_GetWord(strBuff,p);
			lpDefItem->sDefence[0] = atoi(strBuff);
			p=fileread_GetWord(strBuff,p);
			lpDefItem->sDefence[1] = atoi(strBuff);
			NextFlag = TRUE;
		}

		///////////////// *블럭율 ///////////////////////
		if ( !NextFlag && lstrcmp( decode , "*\272\355\267\260\300\262" )==0 ) {
			p=fileread_GetWord(strBuff,p);
			lpDefItem->fBlock_Rating[0] = (float)atof(strBuff);
			p=fileread_GetWord(strBuff,p);
			lpDefItem->fBlock_Rating[1] = (float)atof(strBuff);
			NextFlag = TRUE;
		}

//////////////	신발성능		////////////
		///////////////// *이동속도 ///////////////////////
		if ( !NextFlag && lstrcmp( decode , "*\300\314\265\277\274\323\265\265" )==0 ) {
			p=fileread_GetWord(strBuff,p);
			lpDefItem->fSpeed[0] = (float)atof(strBuff);
			p=fileread_GetWord(strBuff,p);
			lpDefItem->fSpeed[1] = (float)atof(strBuff);
			NextFlag = TRUE;
		}
//////////////	팔찌성능		/////////////

		///////////////// *보유공간 ///////////////////////
		if ( !NextFlag && lstrcmp( decode , "*\272\270\300\257\260\370\260\243" )==0 ) {
			p=fileread_GetWord(strBuff,p);
			lpItem->Potion_Space = atoi(strBuff);
			NextFlag = TRUE;
		}

//////////////	특수능력		/////////////
		///////////////// *마법숙련도 ///////////////////////
		if ( !NextFlag && lstrcmp( decode , "*\270\266\271\375\274\367\267\303\265\265" )==0 ) {
			p=fileread_GetWord(strBuff,p);
			lpItem->fMagic_Mastery = (float)atof(strBuff);
			NextFlag = TRUE;
		}
		///////////////// *마나재생 ///////////////////////
		if ( !NextFlag && ( lstrcmp( decode , "*\270\266\263\252\300\347\273\375" )==0 || lstrcmp( decode , "*\261\342\267\302\300\347\273\375" )==0 ) ) {
			p=fileread_GetWord(strBuff,p);
			lpDefItem->fMana_Regen[0] = (float)atof(strBuff);
			p=fileread_GetWord(strBuff,p);
			lpDefItem->fMana_Regen[1] = (float)atof(strBuff);
			NextFlag = TRUE;
		}
		///////////////// *라이프재생 ///////////////////////
		if ( !NextFlag && ( lstrcmp( decode , "*\266\363\300\314\307\301\300\347\273\375" )==0 || lstrcmp( decode , "*\273\375\270\355\267\302\300\347\273\375" )==0 )) {
			p=fileread_GetWord(strBuff,p);
			lpDefItem->fLife_Regen[0] = (float)atof(strBuff);
			p=fileread_GetWord(strBuff,p);
			lpDefItem->fLife_Regen[1] = (float)atof(strBuff);
			NextFlag = TRUE;
		}
		///////////////// *스테미나재생 ///////////////////////
		if ( !NextFlag && ( lstrcmp( decode , "*\275\272\305\327\271\314\263\252\300\347\273\375" )==0 || lstrcmp( decode , "*\261\331\267\302\300\347\273\375" )==0 )) {
			p=fileread_GetWord(strBuff,p);
			lpDefItem->fStamina_Regen[0] = (float)atof(strBuff);
			p=fileread_GetWord(strBuff,p);
			lpDefItem->fStamina_Regen[1] = (float)atof(strBuff);
			NextFlag = TRUE;

		}
		///////////////// *마나추가 ///////////////////////
		if ( !NextFlag && ( lstrcmp( decode , "*\270\266\263\252\303\337\260\241" )==0 || lstrcmp( decode , "*\261\342\267\302\303\337\260\241" )==0 )) {
			p=fileread_GetWord(strBuff,p);
			lpDefItem->Increase_Mana[0] = atoi(strBuff);
			p=fileread_GetWord(strBuff,p);
			lpDefItem->Increase_Mana[1] = atoi(strBuff);
			NextFlag = TRUE;
		}
		///////////////// *라이프추가 ///////////////////////
		if ( !NextFlag && ( lstrcmp( decode , "*\266\363\300\314\307\301\303\337\260\241" )==0 || lstrcmp( decode , "*\273\375\270\355\267\302\303\337\260\241" )==0 ) ) {
			p=fileread_GetWord(strBuff,p);
			lpDefItem->Increase_Life[0] = atoi(strBuff);
			p=fileread_GetWord(strBuff,p);
			lpDefItem->Increase_Life[1] = atoi(strBuff);
			NextFlag = TRUE;
		}
		///////////////// *스테미나추가 ///////////////////////
		if ( !NextFlag && ( lstrcmp( decode , "*\275\272\305\327\271\314\263\252\303\337\260\241" )==0 || lstrcmp( decode , "*\261\331\267\302\303\337\260\241" )==0 ) ) {
			p=fileread_GetWord(strBuff,p);
			lpDefItem->Increase_Stamina[0] = atoi(strBuff);
			p=fileread_GetWord(strBuff,p);
			lpDefItem->Increase_Stamina[1] = atoi(strBuff);
			NextFlag = TRUE;
		}


//////////////	요구특성		/////////////

		///////////////// *레벨 ///////////////////////
		if ( !NextFlag && lstrcmp( decode , "*\267\271\272\247" )==0 ) {
			p=fileread_GetWord(strBuff,p);
			lpItem->Level = atoi(strBuff);
			NextFlag = TRUE;
		}

		///////////////// *힘 ///////////////////////
		if ( !NextFlag && lstrcmp( decode , "*\310\373" )==0 ) {
			p=fileread_GetWord(strBuff,p);
			lpItem->Strength = atoi(strBuff);
			NextFlag = TRUE;
		}

		///////////////// *정신력 ///////////////////////
		if ( !NextFlag && lstrcmp( decode , "*\301\244\275\305\267\302" )==0 ) {
			p=fileread_GetWord(strBuff,p);
			lpItem->Spirit = atoi(strBuff);
			NextFlag = TRUE;
		}
		///////////////// *재능 ///////////////////////
		if ( !NextFlag && lstrcmp( decode , "*\300\347\264\311" )==0 ) {
			p=fileread_GetWord(strBuff,p);
			lpItem->Talent = atoi(strBuff);
			NextFlag = TRUE;
		}
		///////////////// *민첩성 ///////////////////////
		if ( !NextFlag && lstrcmp( decode , "*\271\316\303\270\274\272" )==0 ) {
			p=fileread_GetWord(strBuff,p);
			lpItem->Dexterity = atoi(strBuff);
			NextFlag = TRUE;
		}
		///////////////// *건강 ///////////////////////
		if ( !NextFlag && lstrcmp( decode , "*\260\307\260\255" )==0 ) {
			p=fileread_GetWord(strBuff,p);
			lpItem->Health = atoi(strBuff);
			NextFlag = TRUE;
		}


/////////////	회복약		 ////////////

		///////////////// *스테미너상승 ///////////////////////
		if ( !NextFlag && ( lstrcmp( decode , "*\275\272\305\327\271\314\263\312\273\363\275\302" )==0 || lstrcmp( decode , "*\261\331\267\302\273\363\275\302" )==0 ) ) {
			p=fileread_GetWord(strBuff,p);
			lpItem->Stamina[0] = atoi(strBuff);
			p=fileread_GetWord(strBuff,p);
			lpItem->Stamina[1] = atoi(strBuff);
			NextFlag = TRUE;
		}

		///////////////// *마나상승 ///////////////////////
		if ( !NextFlag && ( lstrcmp( decode , "*\270\266\263\252\273\363\275\302" )==0 || lstrcmp( decode , "*\261\342\267\302\273\363\275\302" )==0 )) {
			p=fileread_GetWord(strBuff,p);
			lpItem->Mana[0] = atoi(strBuff);
			p=fileread_GetWord(strBuff,p);
			lpItem->Mana[1] = atoi(strBuff);
			NextFlag = TRUE;
		}

		///////////////// *라이프상승 ///////////////////////
		if ( !NextFlag && ( lstrcmp( decode , "*\266\363\300\314\307\301\273\363\275\302" )==0 || lstrcmp( decode , "*\273\375\270\355\267\302\273\363\275\302" )==0 )) {
			p=fileread_GetWord(strBuff,p);
			lpItem->Life[0] = atoi(strBuff);
			p=fileread_GetWord(strBuff,p);
			lpItem->Life[1] = atoi(strBuff);
			NextFlag = TRUE;
		}

//////////////////////////////////////////////////////////
/////////////////// 직업별 특화능력 //////////////////////
//////////////////////////////////////////////////////////

		///////////////// 특화 직업 설정 ///////////////////////////
		if ( !NextFlag && lstrcmp( decode , "**\306\257\310\255" )==0 ) {
			while(1) {
				p=fileread_GetWord(strBuff,p);
				if ( strBuff[0]==0 ) break;
				cnt = 0;
				while(1) {
					if ( !JobDataBase[cnt].szName[0] ) break;
					if ( lstrcmpi( JobDataBase[cnt].szName  , strBuff )==0 ) {
						//lpItem->JobCodeMask |= JobDataBase[cnt].JobBitCode;
						lpItem->JobCodeMask = JobDataBase[cnt].JobBitCode;
						break;
					}
					cnt++;
				}
			}

			NextFlag = TRUE;
		}

		///////////////// 랜덤특화 직업 설정 ///////////////////////////
		if ( !NextFlag && lstrcmp( decode , "**\306\257\310\255\267\243\264\375" )==0 ) {
			while(1) {
				p=fileread_GetWord(strBuff,p);
				if ( strBuff[0]==0 ) break;
				cnt = 0;
				while(1) {
					if ( !JobDataBase[cnt].szName[0] ) break;
					if ( lstrcmpi( JobDataBase[cnt].szName  , strBuff )==0 ) {
						if ( lpDefItem->JobBitCodeRandomCount>=SPECIAL_JOB_RANDOM_MAX ) break;
						lpDefItem->dwJobBitCode_Random[lpDefItem->JobBitCodeRandomCount++] = JobDataBase[cnt].JobBitCode;
						//lpItem->JobCodeMask |= JobDataBase[cnt].JobBitCode;
						break;
					}
					cnt++;
				}
			}

			NextFlag = TRUE;
		}


		/////////////////////////////////////////////////////
		/////////////////// 특화 추가 ///////////////////////
		/////////////////////////////////////////////////////

		///////////////// *흡수력 ///////////////////////
		if ( !NextFlag && lstrcmp( decode , "**\310\355\274\366\267\302" )==0 ) {
			p=fileread_GetWord(strBuff,p);
			lpDefItem->fSpecial_Absorb[0] = (float)atof(strBuff);
			p=fileread_GetWord(strBuff,p);
			lpDefItem->fSpecial_Absorb[1] = (float)atof(strBuff);			
			//lpItem->JobItem.Add_fAbsorb = (float)atof(strBuff);
			NextFlag = TRUE;
		}
		///////////////// *방어력 ///////////////////////
		if ( !NextFlag && lstrcmp( decode , "**\271\346\276\356\267\302" )==0 ) {
			p=fileread_GetWord(strBuff,p);
			lpDefItem->sSpecial_Defence[0] = atoi(strBuff);
			p=fileread_GetWord(strBuff,p);
			lpDefItem->sSpecial_Defence[1] = atoi(strBuff);
//			p=fileread_GetWord(strBuff,p);
//			lpItem->JobItem.Add_Defence = atoi(strBuff);
			NextFlag = TRUE;
		}
/*
		if ( !NextFlag && lstrcmp( decode , "*방어력" )==0 ) {
			p=fileread_GetWord(strBuff,p);
			lpDefItem->sSpecial_Defence[0] = atoi(strBuff);
			p=fileread_GetWord(strBuff,p);
			lpDefItem->sDefence[1] = atoi(strBuff);
			NextFlag = TRUE;
		}
*/
		///////////////// *이동속도 ///////////////////////
		if ( !NextFlag && lstrcmp( decode , "**\300\314\265\277\274\323\265\265" )==0 ) {
			p=fileread_GetWord(strBuff,p);
			lpDefItem->fSpecial_fSpeed[0] = (float)atof(strBuff);
			p=fileread_GetWord(strBuff,p);
			lpDefItem->fSpecial_fSpeed[1] = (float)atof(strBuff);
			//lpItem->JobItem.Add_fSpeed = (float)atof(strBuff);
			NextFlag = TRUE;
		}
		///////////////// *블럭율 ///////////////////////
		if ( !NextFlag && lstrcmp( decode , "**\272\355\267\260\300\262" )==0 ) {
			p=fileread_GetWord(strBuff,p);
			lpItem->JobItem.Add_fBlock_Rating = (float)atof(strBuff);
			NextFlag = TRUE;
		}
		///////////////// *공격속도 ///////////////////////
		if ( !NextFlag && lstrcmp( decode , "**\260\370\260\335\274\323\265\265" )==0 ) {
			p=fileread_GetWord(strBuff,p);
			lpItem->JobItem.Add_Attack_Speed = atoi(strBuff);
			NextFlag = TRUE;
		}
		///////////////// *크리티컬힛 ///////////////////////
		if ( !NextFlag && lstrcmp( decode , "**\305\251\270\256\306\274\304\303" )==0 ) {
			p=fileread_GetWord(strBuff,p);
			lpItem->JobItem.Add_Critical_Hit = atoi(strBuff);
			NextFlag = TRUE;
		}
		///////////////// *사정거리 ///////////////////////
		if ( !NextFlag && lstrcmp( decode , "**\273\347\301\244\260\305\270\256" )==0 ) {
			p=fileread_GetWord(strBuff,p);
			lpItem->JobItem.Add_Shooting_Range = atoi(strBuff);
			NextFlag = TRUE;
		}
		///////////////// *마법숙련도 ///////////////////////
		if ( !NextFlag && lstrcmp( decode , "**\270\266\271\375\274\367\267\303\265\265" )==0 ) {
			p=fileread_GetWord(strBuff,p);
			//lpItem->JobItem.Add_fMagic_Mastery = (float)atof(strBuff);
			lpDefItem->fSpecial_Magic_Mastery[0] = (float)atof(strBuff);
			p=fileread_GetWord(strBuff,p);
			lpDefItem->fSpecial_Magic_Mastery[1] = (float)atof(strBuff);

			NextFlag = TRUE;
		}


		/////////////////////////////////////////////////////
		/////////////////// 레벨별 상승 /////////////////////
		/////////////////////////////////////////////////////


		///////////////// *마나추가 ///////////////////////
		if ( !NextFlag && ( lstrcmp( decode , "**\270\266\263\252\303\337\260\241" )==0 || lstrcmp( decode , "**\261\342\267\302\303\337\260\241" )==0 ) ) {
			p=fileread_GetWord(strBuff,p);
			lpItem->JobItem.Lev_Mana  = atoi(strBuff);
			NextFlag = TRUE;
		}
		///////////////// *라이프추가 ///////////////////////
		if ( !NextFlag && ( lstrcmp( decode , "**\266\363\300\314\307\301\303\337\260\241" )==0 || lstrcmp( decode , "**\273\375\270\355\267\302\303\337\260\241" )==0 ) ) {
			p=fileread_GetWord(strBuff,p);
			lpItem->JobItem.Lev_Life = atoi(strBuff);
			NextFlag = TRUE;
		}
		///////////////// *명중력 ///////////////////////
		if ( !NextFlag && lstrcmp( decode , "**\270\355\301\337\267\302" )==0 ) {
			//lpItem->JobItem.Lev_Attack_Rating = atoi(strBuff);
			p=fileread_GetWord(strBuff,p);
			lpDefItem->Lev_Attack_Rating[0] = atoi(strBuff);
			p=fileread_GetWord(strBuff,p);
			lpDefItem->Lev_Attack_Rating[1] = atoi(strBuff);
			NextFlag = TRUE;
		}
		///////////////// 공격력 ///////////////////////
		if ( !NextFlag && lstrcmp( decode , "**\260\370\260\335\267\302" )==0 ) {
			p=fileread_GetWord(strBuff,p);
			lpItem->JobItem.Lev_Damage[0] = atoi(strBuff);
			p=fileread_GetWord(strBuff,p);
			lpItem->JobItem.Lev_Damage[1] = atoi(strBuff);
			NextFlag = TRUE;
		}

		/////////////////////////////////////////////////////
		/////////////////// %비율 상승 //////////////////////
		/////////////////////////////////////////////////////
/*
	float	Per_Mana_Regen;			//마나 재생 
	float	Per_Life_Regen;			//라이프 재생 
	float	Per_Stamina_Regen;		//스테미나 재생
*/
		///////////////// *마나재생 ///////////////////////
		if ( !NextFlag && ( lstrcmp( decode , "**\270\266\263\252\300\347\273\375" )==0 || lstrcmp( decode , "**\261\342\267\302\300\347\273\375" )==0 ) ) {
			p=fileread_GetWord(strBuff,p);
			//lpItem->JobItem.Per_Mana_Regen = (float)atof(strBuff);
			lpDefItem->fSpecial_Mana_Regen[0] = (float)atof(strBuff);
			p=fileread_GetWord(strBuff,p);
			lpDefItem->fSpecial_Mana_Regen[1] = (float)atof(strBuff);

			NextFlag = TRUE;
		}
		///////////////// *라이프재생 ///////////////////////
		if ( !NextFlag && ( lstrcmp( decode , "**\266\363\300\314\307\301\300\347\273\375" )==0 || lstrcmp( decode , "**\273\375\270\355\267\302\300\347\273\375" )==0 )) {
			p=fileread_GetWord(strBuff,p);
			lpItem->JobItem.Per_Life_Regen = (float)atof(strBuff);
			NextFlag = TRUE;
		}
		///////////////// *스테미나재생 ///////////////////////
		if ( !NextFlag && ( lstrcmp( decode , "**\275\272\305\327\271\314\263\252\300\347\273\375" )==0 || lstrcmp( decode , "**\261\331\267\302\300\347\273\375" )==0 ) ) {
			p=fileread_GetWord(strBuff,p);
			lpItem->JobItem.Per_Stamina_Regen = (float)atof(strBuff);
			NextFlag = TRUE;
		}

		if ( !NextFlag && lstrcmp( decode , "*\271\337\273\375\301\246\307\321" )==0 ) {
			p=fileread_GetWord(strBuff,p);
			lpDefItem->sGenDay[0] = atoi(strBuff);
			NextFlag = TRUE;
		}

		////////////////// 다른 파일에서 계속 읽어 드림 ///////////////
		if ( !NextFlag && lstrcmp( decode , "*\277\254\260\341\306\304\300\317" )==0 ) {
			pb=p;p=fileread_GetWord(strBuff,p);if(strBuff[0]==34)p=GetString(strBuff,pb);	//단어 찾기

			char szNextFile[128];
			GetDirectoryFromFile( szItemFile , szNextFile );
			lstrcat( szNextFile , strBuff );
			NextFlag = TRUE;
			fclose(fp);

			fp = fopen( szNextFile , "rb" );
			if ( fp==NULL ) break;
		}

	}

	if ( lpItem->EffectBlink[0] ) {
		lpItem->EffectBlink[1] = 
			( lpItem->EffectColor[0]+lpItem->EffectColor[1]+lpItem->EffectColor[2]+lpItem->EffectColor[3]+(short)lpItem->DispEffect );
	}

	if (fp) fclose(fp);
	return TRUE;
}


struct szSTATE_CODE {
	char	*szCodeName;
	DWORD	dwCode;
};

szSTATE_CODE	dwCharSoundCode[] = {
	{	"CYCLOPS"	,	snCHAR_SOUND_CYCLOPS },
	{	"HOBGOBLIN"	,	snCHAR_SOUND_HOBGOBLIN },
	{	"IMP"		,	snCHAR_SOUND_IMP },
	{	"MINIG"		,	snCHAR_SOUND_MINIG },
	{	"PLANT"		,	snCHAR_SOUND_PLANT },
	{	"SKELETON"	,	snCHAR_SOUND_SKELETON },
	{	"ZOMBI"		,	snCHAR_SOUND_ZOMBI },
	{	"OBIT"		,	snCHAR_SOUND_OBIT },

	{	"HOPT"		,	snCHAR_SOUND_HOPT },
	{	"BARGON"	,	snCHAR_SOUND_BARGON },
	{	"LEECH"		,	snCHAR_SOUND_LEECH },
	{	"MUSHROOM"	,	snCHAR_SOUND_MUSHROOM },

	{	"ARMA"		,	snCHAR_SOUND_ARMA },
	{	"ARMADIL"	,	snCHAR_SOUND_ARMA },
	{	"SCORPION"	,	snCHAR_SOUND_SCORPION },

	{	"HEADCUTTER",	snCHAR_SOUND_HEADCUTTER },
	{	"SANDLEM"	,	snCHAR_SOUND_SANDLEM },
	{	"WEB"		,	snCHAR_SOUND_WEB },
	{	"HOPYKING"	,	snCHAR_SOUND_HOPYKING },
	{	"CRIP"		,	snCHAR_SOUND_CRIP },
	{	"BUMA"		,	snCHAR_SOUND_BUMA },
	{	"DECOY"		,	snCHAR_SOUND_DECOY },
	{	"DORAL"		,	snCHAR_SOUND_DORAL },
	{	"FIGON"		,	snCHAR_SOUND_FIGON },
	{	"GOLEM"		,	snCHAR_SOUND_STONEGIANT },
	{	"GREVEN"	,	snCHAR_SOUND_GREVEN },
	{	"ILLUSIONKNIGHT" , snCHAR_SOUND_ILLUSIONKNIGHT },
	{	"SKELETONRANGE" , snCHAR_SOUND_SKELETONRANGE },
	{	"SKELETONMELEE" , snCHAR_SOUND_SKELETONMELEE },
	{	"WOLVERLIN" , snCHAR_SOUND_WOLVERLIN },

	//모라이온 신규 추가 몬스터

	{	"RABIE"			, snCHAR_SOUND_RABIE			},
	{	"MUDY"			, snCHAR_SOUND_MUDY				},
	{	"SEN"			, snCHAR_SOUND_SEN				},
	{	"EGAN"			, snCHAR_SOUND_EGAN				},
	{	"BEEDOG"		, snCHAR_SOUND_BEEDOG			},
	{	"MUTANTPLANT"	, snCHAR_SOUND_MUTANTPLANT		},
	{	"MUTANTRABIE"	, snCHAR_SOUND_MUTANTRABIE		},
	{	"MUTANTTREE"	, snCHAR_SOUND_MUTANTTREE		},
	{	"AVELISK"		, snCHAR_SOUND_AVELISK			},
	{	"NAZ"			, snCHAR_SOUND_NAZ				},
	{	"MUMMY"			, snCHAR_SOUND_MUMMY			},
	{	"HULK"			, snCHAR_SOUND_HULK				},
	{	"HUNGKY"		, snCHAR_SOUND_HUNGKY			},
	{	"SUCCUBUS"		, snCHAR_SOUND_SUCCUBUS			},
	{	"DAWLIN"		, snCHAR_SOUND_DAWLIN			},
	{	"SHADOW"		, snCHAR_SOUND_SHADOW			},
	{	"BERSERKER"		, snCHAR_SOUND_BERSERKER		},
	{	"IRONGUARD"		, snCHAR_SOUND_IRONGUARD		},
	{	"FURY"			, snCHAR_SOUND_FURY				},
	{	"SLIVER"		, snCHAR_SOUND_SLIVER			},

	{	"RATOO"			, snCHAR_SOUND_RATOO			},
	{	"STYGIANLORD"	, snCHAR_SOUND_STYGIANLORD		},
	{	"OMICRON"		, snCHAR_SOUND_OMICRON			},
	{	"D-MACHINE"		, snCHAR_SOUND_DMACHINE			},
	{	"METRON"		, snCHAR_SOUND_METRON			},
	{	"MRGHOST"		, snCHAR_SOUND_MRGHOST			},

	{	"VAMPIRICBAT"	, snCHAR_SOUND_VAMPIRICBAT		},
	{	"MIREKEEPER"	, snCHAR_SOUND_MIREKEEPER		},
	{	"MUFFIN"		, snCHAR_SOUND_MUFFIN			},
	{	"SOLIDSNAIL"	, snCHAR_SOUND_SOLIDSNAIL		},
	{	"BEEVIL"		, snCHAR_SOUND_BEEVIL			},
	{	"NIGHTMARE"		, snCHAR_SOUND_NIGHTMARE		},
	{	"STONEGOLEM"	, snCHAR_SOUND_STONEGOLEM		},
	{	"THORNCRAWLER"	, snCHAR_SOUND_THORNCRAWLER		},
	{	"HEAVYGOBLIN"	, snCHAR_SOUND_HEAVYGOBLIN		},
	{	"EVILPLANT"		, snCHAR_SOUND_EVILPLANT		},
	{	"HAUNTINGPLANT"	, snCHAR_SOUND_HAUNTINGPLANT	},
	{	"DARKKNIGHT"	, snCHAR_SOUND_DARKKNIGHT		},
	{	"GUARDIAN-SAINT", snCHAR_SOUND_GUARDIAN_SAINT	},

	/////////////// 아이언 몬스터 추가 //////////////////

	{	"CHAINGOLEM"	, snCHAR_SOUND_CHAINGOLEM		},
	{	"DEADZONE"		, snCHAR_SOUND_DEADZONE			},
	{	"GROTESQUE"		, snCHAR_SOUND_GROTESQUE		},
	{	"HYPERMACHINE"	, snCHAR_SOUND_HYPERMACHINE		},
	{	"IRONFIST"		, snCHAR_SOUND_IRONFIST			},
	{	"MORGON"		, snCHAR_SOUND_MORGON			},
	{	"MOUNTAIN"		, snCHAR_SOUND_MOUNTAIN			},
	{	"RAMPAGE"		, snCHAR_SOUND_RAMPAGE			},
	{	"RUNICGUARDIAN"	, snCHAR_SOUND_RUNICGUARDIAN	},
	{	"SADNESS"		, snCHAR_SOUND_SADNESS			},
	{	"TOWERGOLEM"	, snCHAR_SOUND_TOWERGOLEM		},
	{	"VAMPIRICBEE"	, snCHAR_SOUND_VAMPRICBEE		},
	{	"VAMPIRICMACHINE", snCHAR_SOUND_VAMPRICMACHINE	},
	{	"AVELINARCHER"	, snCHAR_SOUND_AVELIN_ARCHER	},
	{	"AVELINQUEEN"	, snCHAR_SOUND_AVELIN_QUEEN		},
	{	"BABEL"			, snCHAR_SOUND_BABEL			},

	/////////////// 아이스 몬스터 추가 //////////////////
	{	"MYSTIC"		, snCHAR_SOUND_MYSTIC			},
	{	"ICEGOBLIN"		, snCHAR_SOUND_ICEGOBLIN		},
	{	"COLDEYE"		, snCHAR_SOUND_COLDEYE			},
	{	"FROZEN"		, snCHAR_SOUND_FROZEN			},
	{	"ICEGOLEM"		, snCHAR_SOUND_ICEGOLEM			},
	{	"FROST"			, snCHAR_SOUND_FROST			},
	{	"CHAOSCARA"		, snCHAR_SOUND_CHAOSCARA		},
	{	"DEATHKNIGHT"	, snCHAR_SOUND_DEATHKNIGHT		},


	///////////// 그리디호수 몬스터 추가 ////////////////
	{	"GREATE_GREVEN"	, snCHAR_SOUND_GREATE_GREVEN	},
	{	"LIZARDFOLK"	, snCHAR_SOUND_LIZARDFOLK		},
	{	"M_LORD"		, snCHAR_SOUND_M_LORD			},
	{	"SPIDER"		, snCHAR_SOUND_SPIDER			},
	{	"STINGRAY"		, snCHAR_SOUND_STINGRAY			},
	{	"STRIDER"		, snCHAR_SOUND_STRIDER			},
	{	"OMU"			, snCHAR_SOUND_OMU				},

	///////////// 아이스2 몬스터 추가 ////////////////
	{	"TURTLECANNON"	, snCHAR_SOUND_TURTLE_CANNON	},
	{	"DEVILBIRD"		, snCHAR_SOUND_DEVIL_BIRD		},
	{	"BLIZZARDGIANT"	, snCHAR_SOUND_BLIZZARD_GIANT	},
	{	"KELVEZU"		, snCHAR_SOUND_KELVEZU			},


	///////////// 신규필 몬스터 추가 ////////////////
	{	"DARKPHALANX"	, snCHAR_SOUND_DARKPHALANX		},
	{	"BLOODYKNIGHT"	, snCHAR_SOUND_BLOODYKNIGHT		},
	{	"CHIMERA"		, snCHAR_SOUND_CHIMERA			},
	{	"FIREWORM"		, snCHAR_SOUND_FIREWORM			},
	{	"HELLHOUND"		, snCHAR_SOUND_HELLHOUND		},
	{	"DARKGUARD"		, snCHAR_SOUND_DARKGUARD		},
	{	"DARKMAGE"		, snCHAR_SOUND_DARKMAGE			},

	{	"MONMOKOVA"		, snCHAR_SOUND_MOKOVA			},
	{	"MONTEMPLEGUARD", snCHAR_SOUND_TEMPLEGUARD		},
	{	"MONSETO"		, snCHAR_SOUND_SETO				},
	{	"MONKINGSPIDER"	, snCHAR_SOUND_KINGSPIDER		},
	//////////////	pluto 신규필드 몬스터	//////////////
	{	"D_KN"			, snCHAR_SOUND_REVIVED_KNIGHT	},
	{	"D_MAGI"		, snCHAR_SOUND_REVIVED_MAGICIAN },
	{	"D_ATAL"		, snCHAR_SOUND_REVIVED_ATALANTA },
	{	"D_FI"			, snCHAR_SOUND_REVIVED_FIGTHER	},
	{	"D_AR"			, snCHAR_SOUND_REVIVED_ARCHER	},
	{	"D_MECA"		, snCHAR_SOUND_REVIVED_MECANICIAN },
	{	"D_PA"			, snCHAR_SOUND_REVIVED_PIKEMAN	},
	{	"D_PR"			, snCHAR_SOUND_REVIVED_PRIESTESS},
	{	"DEADHOPT"		, snCHAR_SOUND_DEADHOPT			},
	{	"DEADKINGHOPY"	, snCHAR_SOUND_DEADKINGHOPY		},
	{	"GORGON"		, snCHAR_SOUND_GORGON			},
	{	"HOBOGOLEM"		, snCHAR_SOUND_HOBOGOLEM		},
	
	// 박재원 - 저주받은 신전 3층(신규필드 몬스터)
	{	"NIKEN"			, snCHAR_SOUND_NIKEN			},
	{	"MIMIC"			, snCHAR_SOUND_MIMIC			},
	{	"KINGBAT"		, snCHAR_SOUND_KINGBAT			},
	{	"GOBLINSHAMAN"	, snCHAR_SOUND_GOBLINSHAMAN		}, 
	{	"HEST"			, snCHAR_SOUND_HEST				},

	// 박재원 - 끝없는 탑 3층(신규필드 몬스터)
	{	"RUCA"			, snCHAR_SOUND_RUCA				},
	{	"NAZSENIOR"		, snCHAR_SOUND_NAZSENIOR		},
	{	"IGOLATION"		, snCHAR_SOUND_IGOLATION		},
	{	"KAKOA"			, snCHAR_SOUND_KAKOA			}, 
	{	"SPRIN"			, snCHAR_SOUND_SPRIN			},
	{	"UNDEADMAPLE"	, snCHAR_SOUND_UNDEADMAPLE		},
	{	"XETAN"			, snCHAR_SOUND_XETAN			},


	// 박재원 - 복날 이벤트 몬스터 추가
	{	"BEBECHICK"		, snCHAR_SOUND_BEBECHICK		},	// 아기 꼬꼬
	{	"PAPACHICK"		, snCHAR_SOUND_PAPACHICK		},	// 아빠 꼬꼬

	{	"LORDHOPY"		, snCHAR_SOUND_LORDHOPY			},


	/////////////// 이벤트용 NPC //////////////////
	{	"MORIF"			, snCHAR_SOUND_NPC_MORIF		},
	{	"MOLLYWOLF"		, snCHAR_SOUND_NPC_MOLLYWOLF	},
	{	"SKILLMASTER"	, snCHAR_SOUND_NPC_SKILLMASTER	},
	{	"MAGE"			, snCHAR_SOUND_NPC_MAGICMASTER	},
	{	"WORLDCUP"		, snCHAR_SOUND_WORLDCUP			},
	{	"WATERMELON"		, snCHAR_SOUND_WATERMELON			}, // 장별 - 수박몬스터

	/////////////// 스킬 소환용 //////////////////

	{	"WOLVERIN"			, snCHAR_SOUND_S_WOLVERLIN		},
	{	"METALGOLEM"		, snCHAR_SOUND_S_METALGOLEM		},
	{	"FIREELEMENTAL"		, snCHAR_SOUND_S_F_ELEMENTAL	},


	///////////////// 공성 관련 물체 /////////////////////
	{	"CASTLEDOOR"		, snCHAR_SOUND_CASTLE_DOOR		},
	{	"T_CRYSTAL_R"		, snCHAR_SOUND_CASTLE_CRYSTAL_R	},
	{	"T_CRYSTAL_G"		, snCHAR_SOUND_CASTLE_CRYSTAL_G	},
	{	"T_CRYSTAL_B"		, snCHAR_SOUND_CASTLE_CRYSTAL_B	},
	{	"T_CRYSTAL_N"		, snCHAR_SOUND_CASTLE_CRYSTAL_N	},
	{	"TOWER-B"			, snCHAR_SOUND_CASTLE_TOWER_B	},

	//용병 A,B,C
	{	"SOLDIER_A"			, snCHAR_SOUND_CASTLE_SOLDER_A	},
	{	"SOLDIER_B"			, snCHAR_SOUND_CASTLE_SOLDER_B	},
	{	"SOLDIER_C"			, snCHAR_SOUND_CASTLE_SOLDER_C	},

	// 장별 - 소울스톤
	{	"S_AVELIN"				, snCHAR_SOUND_S_AVELIN	},
	{	"S_BAGON"				, snCHAR_SOUND_S_BAGON	},
	{	"S_BEEDOG"				, snCHAR_SOUND_S_BEEDOG	},
	{	"S_BEEVIL"				, snCHAR_SOUND_S_BEEVIL	},
	{	"S_BERSERKER"			, snCHAR_SOUND_S_BERSERKER	},
	{	"S_BUMA"					, snCHAR_SOUND_S_BUMA	},
	{	"S_COKRIS"				, snCHAR_SOUND_S_COKRIS	},
	{	"S_COLDEYE"			, snCHAR_SOUND_S_COLDEYE	},
	{	"S_CRYPT"				, snCHAR_SOUND_S_CRYPT	},
	{	"S_CYCLOPS"			, snCHAR_SOUND_S_CYCLOPS	},
	{	"S_DEADZONE"			, snCHAR_SOUND_S_DEADZONE	},
	{	"S_DECOY"				, snCHAR_SOUND_S_DECOY	},
	{	"S_D-Machine"			, snCHAR_SOUND_S_DMACHINE	},
	{	"S_EVILSNAIL"			, snCHAR_SOUND_S_EVILSNAIL	},
	{	"S_GREVEN"				, snCHAR_SOUND_S_GREVEN	},
	{	"S_GROTESQUE"		, snCHAR_SOUND_S_GROTESQUE	},
	{	"S_ICEGOBLIN"			, snCHAR_SOUND_S_ICEGOBLIN	},
	{	"S_ICEGOLEM"			, snCHAR_SOUND_S_ICEGOLEM	},
	{	"S_INCUBUS"				, snCHAR_SOUND_S_INCUBUS	},
	{	"S_KINGHOPY"			, snCHAR_SOUND_S_KINGHOPY	},
	{	"S_LEECH"				, snCHAR_SOUND_S_LEECH	},
	{	"S_LIZARDFOLK"		, snCHAR_SOUND_S_LIZARDFOLK	},
	{	"S_MEPHIT"				, snCHAR_SOUND_S_MEPHIT	},
	{	"S_METRON"				, snCHAR_SOUND_S_METRON	},
	{	"S_MUFFIN"				, snCHAR_SOUND_S_MUFFIN	},
	{	"S_MUMMY"				, snCHAR_SOUND_S_MUMMY	},
	{	"S_NAZ"					, snCHAR_SOUND_S_NAZ	},
	{	"S_OMEGA"				, snCHAR_SOUND_S_OMEGA	},
	{	"S_RAMPAGE"			, snCHAR_SOUND_S_RAMPAGE	},
	{	"S_SADNESS"			, snCHAR_SOUND_S_SADNESS	},
	{	"S_SLAUGHTER"			, snCHAR_SOUND_S_SLAUGHTER	},
	{	"S_SLAYON"				, snCHAR_SOUND_S_SLAYON	},
	{	"S_SLIVER"				, snCHAR_SOUND_S_SLIVER	},
	{	"S_TITAN"					, snCHAR_SOUND_S_TITAN	},
	{	"S_TOWERGOLEM"		, snCHAR_SOUND_S_TOWERGOLEM	},
	{	"S_TYPHOON"			, snCHAR_SOUND_S_TYPHOON	},
	{	"S_VAMPIRICBAT"		, snCHAR_SOUND_S_VAMPIRICBAT	},
	{	"S_WITCH"					, snCHAR_SOUND_S_WITCH	},
	{	"S_ZOMBIE"				, snCHAR_SOUND_S_ZOMBIE	},


	{	"" , 0 }
};




char	*szCharSizeCodeName[] = {
	"\274\322\307\374" , "\301\337\307\374" , "\301\337\264\353\307\374" , "\264\353\307\374" , ""
};


//100 분율을 256분율로 변환
int ConvPercent8( int percent100 )
{
	return (percent100*256)/100;
}
//걷는 속도를 실제 속도로 변환
int ConvMoveSpeed( float fSpeed )
{
	int sp;

	sp = (int)( (fSpeed-9)*16 )+fONE;

	return sp;
}

//캐릭터 정보 파일에서 해독하여 설정한다
int smCharDecode( char *szCharFile , smCHAR_INFO *lpCharInfo , smCHAR_MONSTER_INFO	*lpMonInfo , char *lpDialogMessage )
{

	FILE	*fp;
	//char	*szDirecotry;
	char strBuff[256];
	char szFile[128];
	char *p;
	char *pb;
	int NextFlag;
	int cnt;
	int	percent;
	int	DialogFlag;

	DWORD	dwItemCode[32];
	int		ItemCodeCnt;


	lstrcpy( szFile , szCharFile );

	fp = fopen( szFile , "rb" );
	if ( fp==NULL ) return FALSE;

	//파일 이름으로 부터 디렉토리를 찾아 설정
	//szDirecotry = SetDirectoryFromFile( szCharFile );

	lpCharInfo->szModelName2[0] = 0;
	lpCharInfo->wPlayClass[1] = 0;
	lpCharInfo->ArrowPosi[0] = 0;
	lpCharInfo->ArrowPosi[1] = 0;

	if ( lpMonInfo ) {
		ZeroMemory( lpMonInfo , sizeof(smCHAR_MONSTER_INFO) );
		lpMonInfo->MoveRange = 64*fONE;
		lpMonInfo->FallItemCount = 0;
		lpMonInfo->FallItemMax = 0;
		lpMonInfo->FallItemPerMax = 0;

		lpMonInfo->DamageStunPers = 100;			//기본 스턴률 100%
		lpMonInfo->UseEventModel = 0;
	}

	while( !feof( fp ) )//  feof: file end까지 읽어라 
	{
		if( fgets( line, 255, fp ) == NULL)	break;

		p = fileread_GetWord( decode , line);

		NextFlag = 0;

		///////////////// 이름 ///////////////////////
		if ( lstrcmp( decode , "*\300\314\270\247" )==0 ) {
			pb=p;p=fileread_GetWord(strBuff,p);if(strBuff[0]==34)p=GetString(strBuff,pb);	//단어 찾기

			lstrcpy( lpCharInfo->szName , strBuff );
			NextFlag = TRUE;
		}

#ifdef	_LANGUAGE_CHINESE
		if ( lstrcmpi( decode , "*C_NAME" )==0 ) {
			pb=p;p=fileread_GetWord(strBuff,p);if(strBuff[0]==34)p=GetString(strBuff,pb);	//단어 찾기

			if ( lpMonInfo )
				lstrcpy( lpMonInfo->szName , strBuff );
			else
				lstrcpy( lpCharInfo->szName , strBuff );

			NextFlag = TRUE;
		}
#endif
#ifdef	_LANGUAGE_JAPANESE
		if ( lstrcmpi( decode , "*J_NAME" )==0 ) {
			pb=p;p=fileread_GetWord(strBuff,p);if(strBuff[0]==34)p=GetString(strBuff,pb);	//단어 찾기

			if ( lpMonInfo )
				lstrcpy( lpMonInfo->szName , strBuff );
			else
				lstrcpy( lpCharInfo->szName , strBuff );

			NextFlag = TRUE;
		}
#endif
#ifdef	_LANGUAGE_TAIWAN
		if ( lstrcmpi( decode , "*T_NAME" )==0 ) {
			pb=p;p=fileread_GetWord(strBuff,p);if(strBuff[0]==34)p=GetString(strBuff,pb);	//단어 찾기

			if ( lpMonInfo )
				lstrcpy( lpMonInfo->szName , strBuff );
			else
				lstrcpy( lpCharInfo->szName , strBuff );

			NextFlag = TRUE;
		}
#endif
#ifdef	_LANGUAGE_ENGLISH
		if ( lstrcmpi( decode , "*E_NAME" )==0 ) {
			pb=p;p=fileread_GetWord(strBuff,p);if(strBuff[0]==34)p=GetString(strBuff,pb);	//단어 찾기

			if ( lpMonInfo )
				lstrcpy( lpMonInfo->szName , strBuff );
			else
				lstrcpy( lpCharInfo->szName , strBuff );

			NextFlag = TRUE;
		}
#endif
#ifdef	_LANGUAGE_THAI
		if ( lstrcmpi( decode , "*TH_NAME" )==0 ) {
			pb=p;p=fileread_GetWord(strBuff,p);if(strBuff[0]==34)p=GetString(strBuff,pb);	//단어 찾기

			if ( lpMonInfo )
				lstrcpy( lpMonInfo->szName , strBuff );
			else
				lstrcpy( lpCharInfo->szName , strBuff );

			NextFlag = TRUE;
		}
#endif
#ifdef _LANGUAGE_VEITNAM
		if ( lstrcmpi( decode , "*V_NAME" )==0 ) {
			pb=p;p=fileread_GetWord(strBuff,p);if(strBuff[0]==34)p=GetString(strBuff,pb);	//단어 찾기

			if ( lpMonInfo )
				lstrcpy( lpMonInfo->szName , strBuff );
			else
				lstrcpy( lpCharInfo->szName , strBuff );

			NextFlag = TRUE;
		}
#endif
#ifdef _LANGUAGE_BRAZIL
		if ( lstrcmpi( decode , "*B_NAME" )==0 ) {
			pb=p;p=fileread_GetWord(strBuff,p);if(strBuff[0]==34)p=GetString(strBuff,pb);	//단어 찾기

			if ( lpMonInfo )
				lstrcpy( lpMonInfo->szName , strBuff );
			else
				lstrcpy( lpCharInfo->szName , strBuff );

			NextFlag = TRUE;
		}
#endif
#ifdef _LANGUAGE_ARGENTINA
		if ( lstrcmpi( decode , "*A_NAME" )==0 ) {
			pb=p;
			p=fileread_GetWord(strBuff,p);
			if(strBuff[0]==34)
			{
				p=GetString(strBuff,pb);	//단어 찾기
			}

			if ( lpMonInfo )
				lstrcpy( lpMonInfo->szName , strBuff );
			else
				lstrcpy( lpCharInfo->szName , strBuff );

			NextFlag = TRUE;
		}
#endif

		///////////////// 모양파일 ///////////////////////
		if ( !NextFlag && lstrcmp( decode , "*\270\360\276\347\306\304\300\317" )==0 ) {
			pb=p;p=fileread_GetWord(strBuff,p);if(strBuff[0]==34)p=GetString(strBuff,pb);	//단어 찾기

			//lstrcpy( lpCharInfo->szModelName, szDirecotry );
			//lstrcat( lpCharInfo->szModelName, strBuff );
			lstrcpy( lpCharInfo->szModelName, strBuff );
			NextFlag = TRUE;
		}

		///////////////// *대화 ///////////////////////

		DialogFlag = 0;

		if ( !NextFlag && lpDialogMessage && lstrcmp( decode , "*\264\353\310\255" )==0 ) {
			pb=p;p=fileread_GetWord(strBuff,p);if(strBuff[0]==34)p=GetString(strBuff,pb);	//단어 찾기

			lstrcpy( lpDialogMessage , strBuff );
			NextFlag = TRUE;
			DialogFlag++;
		}

#ifdef _LANGUAGE_CHINESE
		if ( lpDialogMessage && lstrcmpi( decode , "*C_CHAT" )==0 ) {
			pb=p;p=fileread_GetWord(strBuff,p);if(strBuff[0]==34)p=GetString(strBuff,pb);	//단어 찾기

			lstrcpy( lpDialogMessage , strBuff );
			NextFlag = TRUE;
			DialogFlag++;
		}
#endif

#ifdef _LANGUAGE_JAPANESE
		if ( lpDialogMessage && lstrcmpi( decode , "*J_CHAT" )==0 ) {
			pb=p;p=fileread_GetWord(strBuff,p);if(strBuff[0]==34)p=GetString(strBuff,pb);	//단어 찾기

			lstrcpy( lpDialogMessage , strBuff );
			NextFlag = TRUE;
			DialogFlag++;
		}
#endif

#ifdef _LANGUAGE_TAIWAN
		if ( lpDialogMessage && lstrcmpi( decode , "*T_CHAT" )==0 ) {
			pb=p;p=fileread_GetWord(strBuff,p);if(strBuff[0]==34)p=GetString(strBuff,pb);	//단어 찾기

			lstrcpy( lpDialogMessage , strBuff );
			NextFlag = TRUE;
			DialogFlag++;
		}
#endif
#ifdef _LANGUAGE_ENGLISH
		if ( lpDialogMessage && lstrcmpi( decode , "*E_CHAT" )==0 ) {
			pb=p;p=fileread_GetWord(strBuff,p);if(strBuff[0]==34)p=GetString(strBuff,pb);	//단어 찾기

			lstrcpy( lpDialogMessage , strBuff );
			NextFlag = TRUE;
			DialogFlag++;
		}
#endif
#ifdef _LANGUAGE_THAI
		if ( lpDialogMessage && lstrcmpi( decode , "*TH_CHAT" )==0 ) {
			pb=p;p=fileread_GetWord(strBuff,p);if(strBuff[0]==34)p=GetString(strBuff,pb);	//단어 찾기

			lstrcpy( lpDialogMessage , strBuff );
			NextFlag = TRUE;
			DialogFlag++;
		}
#endif
#ifdef _LANGUAGE_VEITNAM
		if ( lpDialogMessage && lstrcmpi( decode , "*V_CHAT" )==0 ) {
			pb=p;p=fileread_GetWord(strBuff,p);if(strBuff[0]==34)p=GetString(strBuff,pb);	//단어 찾기

			lstrcpy( lpDialogMessage , strBuff );
			NextFlag = TRUE;
			DialogFlag++;
		}
#endif
#ifdef _LANGUAGE_BRAZIL
		if ( lpDialogMessage && lstrcmpi( decode , "*B_CHAT" )==0 ) {
			pb=p;p=fileread_GetWord(strBuff,p);if(strBuff[0]==34)p=GetString(strBuff,pb);	//단어 찾기

			lstrcpy( lpDialogMessage , strBuff );
			NextFlag = TRUE;
			DialogFlag++;
		}
#endif
#ifdef _LANGUAGE_ARGENTINA
		if ( lpDialogMessage && lstrcmpi( decode , "*A_CHAT" )==0 ) {
			pb=p;p=fileread_GetWord(strBuff,p);if(strBuff[0]==34)p=GetString(strBuff,pb);	//단어 찾기

			lstrcpy( lpDialogMessage , strBuff );
			NextFlag = TRUE;
			DialogFlag++;
		}
#endif
		//대사목록 저장
		if ( DialogFlag && lpMonInfo ) {
			if ( lpMonInfo->NpcMsgCount<NPC_MESSAGE_MAX ) {
/*
#ifdef _LANGUAGE_ARGENTINA		//줄바꿈 kyle
				
				char szMessage[512];
				unsigned int iCount = 0;
				unsigned int nMessageSize = 0;

				ZeroMemory(szMessage, sizeof(szMessage));

				nMessageSize = strlen(lpDialogMessage); //npc 대사내용 길이

				for(unsigned int i=0;i<=nMessageSize;i++)
				{
					int y = 0;
					if( (i!= 0) && (iCount % 35) == 0 )
					{
						// 연결된 문자열 사이의 공백 체크
						if( lpDialogMessage[i-1] != 32	&&		
							lpDialogMessage[i] != 32	&&
							lpDialogMessage[i] != 46	)     // 두번째 문자열이 스페이스거나 '.' 이 아닐경우
						{
							szMessage[iCount] = '-';
							iCount++;
							szMessage[iCount] = ' ';//테스트중
							iCount++;
							szMessage[iCount] = ' ';//테스트중
							iCount++;
							szMessage[iCount] = lpDialogMessage[i];
						}
						else
						{
							szMessage[iCount] = lpDialogMessage[i];
						}
					}
					else
					{
						szMessage[iCount] = lpDialogMessage[i];
					}

					iCount++;	
				}

				lpMonInfo->lpNpcMessage[lpMonInfo->NpcMsgCount++] = SetNPCMsg(szMessage);
#else*/
				lpMonInfo->lpNpcMessage[lpMonInfo->NpcMsgCount++] = SetNPCMsg(lpDialogMessage);
//#endif

				
			}
		}

		///////////////// *속성 ///////////////////////
		if ( !NextFlag && lstrcmp( decode , "*\274\323\274\272" )==0 ) {
			pb=p;p=fileread_GetWord(strBuff,p);if(strBuff[0]==34)p=GetString(strBuff,pb);	//단어 찾기

			if ( lstrcmp( strBuff , "\300\373" )==0 )
				lpCharInfo->State = TRUE;
			else
				lpCharInfo->State = FALSE;

			NextFlag = TRUE;
		}

		///////////////// *크기 ///////////////////////
		if ( !NextFlag && lstrcmp( decode , "*\305\251\261\342" )==0 ) {
			p=fileread_GetWord(strBuff,p);
			cnt = 0;

			lpCharInfo->SizeLevel = -1;

			while(1) {
				if ( szCharSizeCodeName[cnt][0]==0 ) break;
				if ( lstrcmp( szCharSizeCodeName[cnt], strBuff )==0 ) {
					lpCharInfo->SizeLevel = cnt;
				}
				cnt++;
			}
			NextFlag = TRUE;
		}

		///////////////// *소리 ///////////////////////
		if ( !NextFlag && ( lstrcmp( decode , "*\274\322\270\256" )==0 || lstrcmp( decode , "*\310\277\260\372\300\275" )==0 ) ) {
			p=fileread_GetWord(strBuff,p);
			cnt = 0;

			while(1) {
				if ( dwCharSoundCode[cnt].dwCode==0 ) break;
				if ( lstrcmpi( dwCharSoundCode[cnt].szCodeName , strBuff )==0 ) {
					lpCharInfo->dwCharSoundCode = dwCharSoundCode[cnt].dwCode;
				}
				cnt++;
			}
			NextFlag = TRUE;
		}


		///////////////// *레벨 ///////////////////////
		if ( !NextFlag && lstrcmp( decode , "*\267\271\272\247" )==0 ) {
			p=fileread_GetWord(strBuff,p);
			lpCharInfo->Level = atoi( strBuff );
			NextFlag = TRUE;
		}

		///////////////// *계급 ///////////////////////
		if ( !NextFlag && lstrcmp( decode , "*\265\316\270\361" )==0 ) {
			//p=fileread_GetWord(strBuff,p);
			lpCharInfo->wPlayClass[0] = MONSTER_CLASS_BOSS;
			NextFlag = TRUE;
		}

		if ( !NextFlag && lstrcmp( decode , "*\260\350\261\336" )==0 ) {
			p=fileread_GetWord(strBuff,p);
			lpCharInfo->wPlayClass[0] = atoi(strBuff);
			NextFlag = TRUE;
		}

		/////////////// *계급 크기 /////////////////////
		if ( !NextFlag && lstrcmp( decode , "*\270\360\265\250\305\251\261\342" )==0 ) {
			p=fileread_GetWord(strBuff,p);
			lpCharInfo->wPlayClass[1] = (short)(atof(strBuff)*fONE);
			if ( lpCharInfo->wPlayClass[1]==fONE ) lpCharInfo->wPlayClass[1]=0;
			NextFlag = TRUE;
		}


		///////////////// *이동속도 ///////////////////////
		if ( !NextFlag && lstrcmp( decode , "*\300\314\265\277\267\302" )==0 ) {
			p=fileread_GetWord(strBuff,p);
			lpCharInfo->Move_Speed = ConvMoveSpeed( (float)atof( strBuff ) ); 
			NextFlag = TRUE;
		}

		///////////////// *공격력 ///////////////////////
		if ( !NextFlag && lstrcmp( decode , "*\260\370\260\335\267\302" )==0 ) {
			p=fileread_GetWord(strBuff,p);
			lpCharInfo->Attack_Damage[0] = atoi(strBuff);
			p=fileread_GetWord(strBuff,p);
			lpCharInfo->Attack_Damage[1] = atoi(strBuff);

			NextFlag = TRUE;
		}

		///////////////// *공격속도 ///////////////////////
		if ( !NextFlag && lstrcmp( decode , "*\260\370\260\335\274\323\265\265" )==0 ) {
			p=fileread_GetWord(strBuff,p);
			lpCharInfo->Attack_Speed = (int)( atof(strBuff)*fONE );
			p=fileread_GetWord(strBuff,p);
			lpCharInfo->Attack_Speed = (int)( atof(strBuff)*fONE );

			NextFlag = TRUE;
		}

		///////////////// *공격범위 ///////////////////////
		if ( !NextFlag && lstrcmp( decode , "*\260\370\260\335\271\374\300\247" )==0 ) {
			p=fileread_GetWord(strBuff,p);
			lpCharInfo->Shooting_Range = atoi(strBuff)*fONE;
			NextFlag = TRUE;
		}



		///////////////// *명중률 ///////////////////////
		if ( !NextFlag && lstrcmp( decode , "*\270\355\301\337\267\302" )==0 ) {
			p=fileread_GetWord(strBuff,p);
			lpCharInfo->Attack_Rating = atoi(strBuff);

			NextFlag = TRUE;
		}
	
		///////////////// *방어력 ///////////////////////
		if ( !NextFlag && lstrcmp( decode , "*\271\346\276\356\267\302" )==0 ) {
			p=fileread_GetWord(strBuff,p);
			lpCharInfo->Defence = atoi(strBuff);

			NextFlag = TRUE;
		}
		///////////////// *흡수율 ///////////////////////
		if ( !NextFlag && lstrcmp( decode , "*\310\355\274\366\300\262" )==0 ) {
			p=fileread_GetWord(strBuff,p);
			lpCharInfo->Absorption = atoi(strBuff);

			NextFlag = TRUE;
		}
		///////////////// *블럭율 ///////////////////////
		if ( !NextFlag && lstrcmp( decode , "*\272\355\267\260\300\262" )==0 ) {
			p=fileread_GetWord(strBuff,p);
			lpCharInfo->Chance_Block = atoi(strBuff);

			NextFlag = TRUE;
		}
	
		///////////////// *라이프 ///////////////////////
		if ( !NextFlag && ( lstrcmp( decode , "*\273\375\270\355\267\302" )==0 || lstrcmp( decode , "*\266\363\300\314\307\301" )==0 ) ) {
			p=fileread_GetWord(strBuff,p);
			lpCharInfo->Life[0] = atoi(strBuff);
			lpCharInfo->Life[1] = lpCharInfo->Life[0];

			NextFlag = TRUE;
		}
	
		///////////////// *생체 ///////////////////////
		if ( !NextFlag && lstrcmp( decode , "*\273\375\303\274" )==0 ) {
			p=fileread_GetWord(strBuff,p);
			lpCharInfo->Resistance[sITEMINFO_BIONIC] = (short)atoi(strBuff);
			NextFlag = TRUE;
		}
		///////////////// *물 ///////////////////////
		if ( !NextFlag && lstrcmp( decode , "*\271\260" )==0 ) {
			p=fileread_GetWord(strBuff,p);
			lpCharInfo->Resistance[sITEMINFO_WATER] = (short)atoi(strBuff);
			NextFlag = TRUE;
		}
		///////////////// *번개 ///////////////////////
		if ( !NextFlag && lstrcmp( decode , "*\271\370\260\263" )==0 ) {
			p=fileread_GetWord(strBuff,p);
			lpCharInfo->Resistance[sITEMINFO_LIGHTING] = (short)atoi(strBuff);

			NextFlag = TRUE;
		}
		///////////////// *얼음 ///////////////////////
		if ( !NextFlag && lstrcmp( decode , "*\276\363\300\275" )==0 ) {
			p=fileread_GetWord(strBuff,p);
			lpCharInfo->Resistance[sITEMINFO_ICE] = (short)atoi(strBuff);
			NextFlag = TRUE;
		}
		///////////////// *바람 ///////////////////////
		if ( !NextFlag && lstrcmp( decode , "*\271\331\266\367" )==0 ) {
			p=fileread_GetWord(strBuff,p);
			lpCharInfo->Resistance[sITEMINFO_WIND] = (short)atoi(strBuff);
			NextFlag = TRUE;
		}
		///////////////// *지동력 ///////////////////////
		if ( !NextFlag && lstrcmp( decode , "*\301\366\265\277\267\302" )==0 ) {
			p=fileread_GetWord(strBuff,p);
			lpCharInfo->Resistance[sITEMINFO_EARTH] = (short)atoi(strBuff);
			NextFlag = TRUE;
		}
		///////////////// *불 ///////////////////////
		if ( !NextFlag && lstrcmp( decode , "*\272\322" )==0 ) {
			p=fileread_GetWord(strBuff,p);
			lpCharInfo->Resistance[sITEMINFO_FIRE] = (short)atoi(strBuff);
			NextFlag = TRUE;
		}
		///////////////// *독 ///////////////////////
		if ( !NextFlag && lstrcmp( decode , "*\265\266" )==0 ) {
			p=fileread_GetWord(strBuff,p);
			lpCharInfo->Resistance[sITEMINFO_POISON] = (short)atof(strBuff);
			NextFlag = TRUE;
		}





//////////////////////////////// 추가분 //////////////////////////////////

		///////////////// *시야 ///////////////////////
		if ( !NextFlag && lstrcmp( decode , "*\275\303\276\337" )==0 ) {
			p=fileread_GetWord(strBuff,p);
			lpCharInfo->Sight = atoi( strBuff );// * fONE;
			if ( lpMonInfo ) lpMonInfo->Real_Sight = lpCharInfo->Sight;

			lpCharInfo->Sight *= lpCharInfo->Sight;			//몬스터 시야 제곱값
			NextFlag = TRUE;
		}

		///////////////// *미니화면 보정 ///////////////////////
		if ( !NextFlag && lstrcmp( decode , "*\310\255\270\351\272\270\301\244" )==0 ) {
			p=fileread_GetWord(strBuff,p);
			if ( strBuff[0] )
				lpCharInfo->ArrowPosi[0] = atoi( strBuff );

			p=fileread_GetWord(strBuff,p);
			if ( strBuff[0] )
				lpCharInfo->ArrowPosi[1] = atoi( strBuff );

		}

		if ( lpMonInfo && !NextFlag ) {

			///////////////// *예비모델 ///////////////////////
			if ( !NextFlag && lstrcmp( decode , "*\277\271\272\361\270\360\265\250" )==0 ) {
				pb=p;p=fileread_GetWord(strBuff,p);if(strBuff[0]==34)p=GetString(strBuff,pb);	//단어 찾기
				lstrcpy( lpCharInfo->szModelName2+1, strBuff ); 
				lpMonInfo->UseEventModel = TRUE;
				NextFlag = TRUE;
			}

			///////////////// *기술공격력 ///////////////////////
			if ( !NextFlag && lstrcmp( decode , "*\261\342\274\372\260\370\260\335\267\302" )==0 ) {
				p=fileread_GetWord(strBuff,p);
				lpMonInfo->SkillDamage[0] = atoi(strBuff);
				p=fileread_GetWord(strBuff,p);
				lpMonInfo->SkillDamage[1] = atoi(strBuff);

				NextFlag = TRUE;
			}

			///////////////// *기술공격거리 ///////////////////////
			if ( !NextFlag && lstrcmp( decode , "*\261\342\274\372\260\370\260\335\260\305\270\256" )==0 ) {
				p=fileread_GetWord(strBuff,p);
				lpMonInfo->SkillDistance = atoi(strBuff);

				NextFlag = TRUE;
			}


			///////////////// *기술공격범위 ///////////////////////
			if ( !NextFlag && lstrcmp( decode , "*\261\342\274\372\260\370\260\335\271\374\300\247" )==0 ) {
				p=fileread_GetWord(strBuff,p);
				lpMonInfo->SkillRange = atoi(strBuff);

				NextFlag = TRUE;
			}

			///////////////// *기술공격률 ///////////////////////
			if ( !NextFlag && lstrcmp( decode , "*\261\342\274\372\260\370\260\335\267\374" )==0 ) {
				p=fileread_GetWord(strBuff,p);
				lpMonInfo->SkillRating = atoi(strBuff);

				NextFlag = TRUE;
			}

			///////////////// *저주기술설정 ///////////////////////
			if ( !NextFlag && lstrcmp( decode , "*\300\372\301\326\261\342\274\372" )==0 ) {
				p=fileread_GetWord(strBuff,p);
				lpMonInfo->SkillCurse = atoi(strBuff);
				NextFlag = TRUE;
			}

			///////////////// *이동타입 ///////////////////////
			if ( !NextFlag && lstrcmp( decode , "*\300\314\265\277\305\270\300\324" )==0 ) {
				p=fileread_GetWord(strBuff,p);

				NextFlag = TRUE;
			}

			///////////////// *공격속도 ///////////////////////
			if ( !NextFlag && lstrcmp( decode , "*\300\314\265\277\271\374\300\247" )==0 ) {
				p=fileread_GetWord(strBuff,p);
				lpMonInfo->MoveRange = (int)( atof(strBuff)*fONE );
				NextFlag = TRUE;
			}


			///////////////// *활동시간 ///////////////////////
			if ( !NextFlag && lstrcmp( decode , "*\310\260\265\277\275\303\260\243" )==0 ) {
				p=fileread_GetWord(strBuff,p);
				lpMonInfo->ActiveHour = 0;
				if ( lstrcmpi( strBuff , "\263\267" )==0 )
					lpMonInfo->ActiveHour = 1;
				if ( lstrcmpi( strBuff , "\271\343" )==0 )
					lpMonInfo->ActiveHour = -1;
				NextFlag = TRUE;
			}

			///////////////// *조직 ///////////////////////
			if ( !NextFlag && lstrcmp( decode , "*\301\266\301\367" )==0 ) {
				p=fileread_GetWord(strBuff,p);
				lpMonInfo->GenerateGroup[0] = atoi( strBuff );
				p=fileread_GetWord(strBuff,p);
				lpMonInfo->GenerateGroup[1] = atoi( strBuff );
				NextFlag = TRUE;
			}

			///////////////// *지능 ///////////////////////
			if ( !NextFlag && lstrcmp( decode , "*\301\366\264\311" )==0 ) {
				p=fileread_GetWord(strBuff,p);
				lpMonInfo->IQ = atoi( strBuff );
				NextFlag = TRUE;
			}

			///////////////// *구별 코드 //////////////////
			if ( !NextFlag && lstrcmp( decode , "*\261\270\272\260\304\332\265\345" )==0 ) {
				p=fileread_GetWord(strBuff,p);
				lpMonInfo->ClassCode = atoi( strBuff );
				NextFlag = TRUE;
			}

			///////////////// *스턴율 ///////////////////////
			if ( !NextFlag && ( lstrcmp( decode , "*\275\272\305\317\300\262" )==0 || lstrcmp( decode , "*\275\272\305\317\267\374" )==0 )) {
				p=fileread_GetWord(strBuff,p);
				lpMonInfo->DamageStunPers = atoi( strBuff );
				NextFlag = TRUE;
			}

			///////////////// *품성 ///////////////////////
			if ( !NextFlag && lstrcmp( decode , "*\307\260\274\272" )==0 ) {
				lpMonInfo->Nature = smCHAR_MONSTER_NATURAL;
				//lpCharInfo->JOB_CODE = smCHAR_MONSTER_NATURAL;
				
				p=fileread_GetWord(strBuff,p);
				if ( lstrcmpi( strBuff , "good" )==0 ) {
					lpMonInfo->Nature = smCHAR_MONSTER_GOOD;
					//lpCharInfo->JOB_CODE = smCHAR_MONSTER_GOOD;
				}
				if ( lstrcmpi( strBuff , "evil" )==0 ) {
					lpMonInfo->Nature = smCHAR_MONSTER_EVIL;
					//lpCharInfo->JOB_CODE = smCHAR_MONSTER_EVIL;
				}
				NextFlag = TRUE;
			}

			///////////////// *이벤트 ///////////////////////
			if ( !NextFlag && lstrcmp( decode , "*\300\314\272\245\306\256\304\332\265\345" )==0 ) {
				p=fileread_GetWord(strBuff,p);
	
				lpMonInfo->EventCode = atoi( strBuff );
				NextFlag = TRUE;
			}
			if ( !NextFlag && lstrcmp( decode , "*\300\314\272\245\306\256\301\244\272\270" )==0 ) {
				p=fileread_GetWord(strBuff,p);
	
				lpMonInfo->EventInfo = atoi( strBuff );
				NextFlag = TRUE;
			}
			if ( !NextFlag && lstrcmp( decode , "*\300\314\272\245\306\256\276\306\300\314\305\333" )==0 ) {
				p=fileread_GetWord(strBuff,p);

				//코드검색 하여 설정
				for(cnt=0;cnt<MAX_ITEM;cnt++) {
					if ( lstrcmpi( strBuff , sItem[cnt].LastCategory )==0 ) {
						lpMonInfo->dwEvnetItem = sItem[cnt].CODE;
						break;
					}
				}
				NextFlag = TRUE;
			}


			///////////////// *특수공격률 ///////////////////////
			if ( !NextFlag && lstrcmp( decode , "*\306\257\274\366\260\370\260\335\267\374" )==0 ) {
				p=fileread_GetWord(strBuff,p);
				lpMonInfo->SpAttackPercetage = ConvPercent8( atoi( strBuff ) );
				NextFlag = TRUE;
			}
			///////////////// *언데드 ///////////////////////
			if ( !NextFlag && lstrcmp( decode , "*\276\360\265\245\265\345" )==0 ) {
				p=fileread_GetWord(strBuff,p);
				if ( lstrcmp( strBuff , "\300\257" )==0 || lstrcmp( strBuff , "\300\326\300\275" )==0 ) {
					lpMonInfo->Undead = TRUE;
					lpCharInfo->Brood = smCHAR_MONSTER_UNDEAD;
				}
				else {
					lpMonInfo->Undead = FALSE;
					lpCharInfo->Brood = smCHAR_MONSTER_NORMAL;
				}

				NextFlag = TRUE;
			}

			if ( !NextFlag && lstrcmp( decode , "*\270\363\275\272\305\315\301\276\301\267" )==0 ) {
				p=fileread_GetWord(strBuff,p);
				if ( strBuff[0] ) {

					lpCharInfo->Brood = smCHAR_MONSTER_NORMAL;

					if ( lstrcmp( strBuff , "\276\360\265\245\265\345" )==0 ) {
						lpCharInfo->Brood = smCHAR_MONSTER_UNDEAD;
					}
					else if ( lstrcmp( strBuff , "\271\302\305\317\306\256" )==0 ) {
						lpCharInfo->Brood = smCHAR_MONSTER_MUTANT;
					}
					else if ( lstrcmp( strBuff , "\265\360\270\325" )==0 ) {
						lpCharInfo->Brood = smCHAR_MONSTER_DEMON;
					}
					else if ( lstrcmp( strBuff , "\270\336\304\253\264\320" )==0 ) {
						lpCharInfo->Brood = smCHAR_MONSTER_MECHANIC;
					}
				}
			}


			///////////////// *경험치 ///////////////////////
			if ( !NextFlag && lstrcmp( decode , "*\260\346\307\350\304\241" )==0 ) {
				p=fileread_GetWord(strBuff,p);
				lpMonInfo->GetExp = atoi( strBuff );
				lpCharInfo->Exp = lpMonInfo->GetExp;
				NextFlag = TRUE;
			}

			///////////////// *물약 보유 ///////////////////////
			if ( !NextFlag && lstrcmp( decode , "*\271\260\276\340\272\270\300\257\274\366" )==0 ) {
				p=fileread_GetWord(strBuff,p);
				lpMonInfo->PotionCount = atoi( strBuff );
				NextFlag = TRUE;
			}

			///////////////// *물약 보유 ///////////////////////
			if ( !NextFlag && lstrcmp( decode , "*\271\260\276\340\272\270\300\257\267\374" )==0 ) {
				p=fileread_GetWord(strBuff,p);
				lpMonInfo->PotionPercent = atoi( strBuff );
				NextFlag = TRUE;
			}
//	int		PotionCount;		//물약 보유수
//	int		PotionPercent;		//물약 보유률


			if ( !NextFlag && lstrcmp( decode , "*\276\306\300\314\305\333\270\360\265\316" )==0 ) {	//땅에 떨어질때 모두에게 아이템 보임
				lpMonInfo->AllSeeItem = TRUE;
				NextFlag = TRUE;
			}

			if ( !NextFlag && lstrcmp( decode , "*\276\306\300\314\305\333\304\253\277\356\305\315" )==0 ) {
				p = fileread_GetWord( strBuff , p );			//확률
				lpMonInfo->FallItemMax = atoi( strBuff );
				NextFlag = TRUE;
			}

			if ( !NextFlag && lstrcmp( decode , "*\303\337\260\241\276\306\300\314\305\333" )==0 ) {
				p = fileread_GetWord( strBuff , p );			//확률
				lpMonInfo->FallItems_Plus[ lpMonInfo->FallItemPlusCount ].Percentage = atoi(strBuff);

				p = fileread_GetWord( strBuff , p );			//아이템 코드
				if ( strBuff[0] && lpMonInfo->FallItemPlusCount<FALLITEM2_MAX ) {
					for(cnt=0;cnt<MAX_ITEM;cnt++) {
						if ( lstrcmpi( strBuff , sItem[cnt].LastCategory )==0 ) {
							lpMonInfo->FallItems_Plus[ lpMonInfo->FallItemPlusCount ].dwItemCode = sItem[cnt].CODE;
							lpMonInfo->FallItemPlusCount++;
							break;
						}
					}
				}
				NextFlag = TRUE;
			}

			if ( !NextFlag && lstrcmp( decode , "*\276\306\300\314\305\333" )==0 ) {
				p = fileread_GetWord( strBuff , p );			//확률
				percent = atoi( strBuff );

				lpMonInfo->FallItemPerMax += percent;

				p = fileread_GetWord( strBuff , p );			//확률
				if ( lstrcmp( strBuff , "\276\370\300\275" )==0 ) {
					//테스트 서버 안나올 확률 적계
					if ( rsServerConfig.TestSeverMode ) {
						lpMonInfo->FallItemPerMax -= percent;
						percent /= 2;
						lpMonInfo->FallItemPerMax += percent;
					}

					//안떨어질 확률
					lpMonInfo->FallItems[ lpMonInfo->FallItemCount ].dwItemCode = 0;
					lpMonInfo->FallItems[ lpMonInfo->FallItemCount ].Percentage = percent;
					lpMonInfo->FallItemCount ++;
				}
				else {
					if ( lstrcmp( strBuff , "\265\267" )==0 ) {
						//돈 떨어질 확률
						lpMonInfo->FallItems[ lpMonInfo->FallItemCount ].dwItemCode = sinGG1|sin01;
						p=fileread_GetWord( strBuff , p );
						lpMonInfo->FallItems[ lpMonInfo->FallItemCount ].sPrice[0] = atoi(strBuff);
						p=fileread_GetWord( strBuff , p );
						lpMonInfo->FallItems[ lpMonInfo->FallItemCount ].sPrice[1] = atoi(strBuff);
						lpMonInfo->FallItems[ lpMonInfo->FallItemCount ].Percentage = percent;
						lpMonInfo->FallItemCount ++;
					}
					else {
						//아이템 떨어질 확률
						ItemCodeCnt = 0;

						while(1) {
							if ( strBuff[0] ) {
								//코드검색 하여 설정
								for(cnt=0;cnt<MAX_ITEM;cnt++) {
									if ( lstrcmpi( strBuff , sItem[cnt].LastCategory )==0 ) {
										dwItemCode[ItemCodeCnt++] = sItem[cnt].CODE;
										break;
									}
								}
							}
							else 
								{ break; }

							strBuff[0] = 0;
							if ( p[0]==0 ) break;
							p = fileread_GetWord( strBuff , p );
						}

						if ( ItemCodeCnt>0 ) {
							//아이템 테이블에 저장
							percent = percent/ItemCodeCnt;
							for( cnt=0;cnt<ItemCodeCnt;cnt++ ) {
								lpMonInfo->FallItems[ lpMonInfo->FallItemCount ].dwItemCode = dwItemCode[cnt];
								lpMonInfo->FallItems[ lpMonInfo->FallItemCount ].Percentage = percent;
								lpMonInfo->FallItemCount ++;
							}
						}
					}
				}
				NextFlag = TRUE;
			}

			///////////////// NPC 물건 판매 정보 /////////////////////
			if ( !NextFlag && lstrcmp( decode , "*\271\253\261\342\306\307\270\305" )==0 ) {
				while(1) {
					pb=p;p=fileread_GetWord(strBuff,p);if(strBuff[0]==34)p=GetString(strBuff,pb);	//단어 찾기
					if ( strBuff[0]==0 ) break;
					if ( lpMonInfo->SellAttackItemCount>=32 ) break;
					for( cnt=0;cnt<MAX_ITEM;cnt++) {
						if ( lstrcmpi( strBuff , sItem[cnt].LastCategory )==0 ) {
							lpMonInfo->SellAttackItem[lpMonInfo->SellAttackItemCount++] =
								(void *)sItem[cnt].CODE;
							break;
						}
					}
				}
				NextFlag = TRUE;
			}

			if ( !NextFlag && lstrcmp( decode , "*\271\346\276\356\261\270\306\307\270\305" )==0 ) {
				while(1) {
					pb=p;p=fileread_GetWord(strBuff,p);if(strBuff[0]==34)p=GetString(strBuff,pb);	//단어 찾기
					if ( strBuff[0]==0 ) break;
					if ( lpMonInfo->SellDefenceItemCount>=32 ) break;
					for( cnt=0;cnt<MAX_ITEM;cnt++) {
						if ( lstrcmpi( strBuff , sItem[cnt].LastCategory )==0 ) {
							lpMonInfo->SellDefenceItem[lpMonInfo->SellDefenceItemCount++] =
								(void *)sItem[cnt].CODE;
							break;
						}
					}
				}
				NextFlag = TRUE;
			}

			if ( !NextFlag && lstrcmp( decode , "*\300\342\310\255\306\307\270\305" )==0 ) {
				while(1) {
					pb=p;p=fileread_GetWord(strBuff,p);if(strBuff[0]==34)p=GetString(strBuff,pb);	//단어 찾기
					if ( strBuff[0]==0 ) break;
					if ( lpMonInfo->SellEtcItemCount>=32 ) break;
					for( cnt=0;cnt<MAX_ITEM;cnt++) {
						if ( lstrcmpi( strBuff , sItem[cnt].LastCategory )==0 ) {
							lpMonInfo->SellEtcItem[lpMonInfo->SellEtcItemCount++] =
								(void *)sItem[cnt].CODE;
							break;
						}
					}
				}
				NextFlag = TRUE;
			}

			if ( !NextFlag && lstrcmp( decode , "*\275\272\305\263\274\366\267\303" )==0 ) {
				lpMonInfo->SkillMaster = TRUE;
				NextFlag = TRUE;
			}
			if ( !NextFlag && lstrcmp( decode , "*\301\367\276\367\300\374\310\257" )==0 ) {
				lpMonInfo->SkillChangeJob = TRUE;

				p=fileread_GetWord( strBuff , p );
				if ( strBuff[0] )
					lpMonInfo->SkillChangeJob = atoi(strBuff);

				NextFlag = TRUE;
			}

			if ( !NextFlag && lstrcmp( decode , "*\300\314\272\245\306\256\270\305\307\245\274\322" )==0 ) {
				p=fileread_GetWord( strBuff , p );
				if ( strBuff[0] )
					lpMonInfo->EventNPC = atoi(strBuff);
				else
					lpMonInfo->EventNPC = 1;

				NextFlag = TRUE;
			}
			if ( !NextFlag && lstrcmp( decode , "*\276\306\300\314\305\333\272\270\260\374" )==0 ) {
				lpMonInfo->WareHouseMaster = TRUE;
				NextFlag = TRUE;
			}
			if ( !NextFlag && lstrcmp( decode , "*\276\306\300\314\305\333\301\266\307\325" )==0 ) {
				lpMonInfo->ItemMix = TRUE;
				NextFlag = TRUE;
			}
			// pluto 제련
			if( !NextFlag && lstrcmp( decode , "*\276\306\300\314\305\333\301\246\267\303" ) == 0 )
			{
				lpMonInfo->Smelting = TRUE;
				NextFlag = TRUE;
			}
			// pluto 제작
			if( !NextFlag && lstrcmp( decode, "*\276\306\300\314\305\333\301\246\300\333" ) == 0 )
			{
                lpMonInfo->Manufacture = TRUE;
				NextFlag = TRUE;
			}
			if ( !NextFlag && lstrcmp( decode , "*\276\306\300\314\305\333\277\254\261\335" )==0 ) {
				lpMonInfo->ItemMix = 200;
				NextFlag = TRUE;
			}
			if ( !NextFlag && lstrcmp( decode , "*\276\306\300\314\305\333\277\241\300\314\302\241" )==0 ) {
				lpMonInfo->ItemAging = TRUE;
				NextFlag = TRUE;
			}
			// 석지용 - 믹스쳐 리셋
			if( !NextFlag && lstrcmp( decode, "*\271\315\275\272\303\263\270\256\274\302" ) == 0 )
			{
				lpMonInfo->MixtureReset = TRUE;
				NextFlag = TRUE;
			}
			if ( !NextFlag && lstrcmp( decode , "*\270\360\261\335\307\324" )==0 ) {
				lpMonInfo->CollectMoney = TRUE;
				NextFlag = TRUE;
			}
#ifdef _LANGUAGE_VEITNAM
			if ( !NextFlag && lstrcmp( decode , "*\262\316\300\314\301\366\267\325" )==0 ) {
				lpMonInfo->WowEvent = TRUE;
				NextFlag = TRUE;
			}
#endif
			if ( !NextFlag && lstrcmp( decode , "*\260\346\307\260\303\337\303\267" )==0 ) {
				lpMonInfo->EventGift = TRUE;
				NextFlag = TRUE;
			}
			if ( !NextFlag && lstrcmp( decode , "*\305\254\267\243\261\342\264\311" )==0 ) {
				lpMonInfo->ClanNPC = TRUE;
				NextFlag = TRUE;
			}
			if ( !NextFlag && lstrcmp( decode , "*\260\346\307\260\271\350\264\336" )==0 ) {
				lpMonInfo->GiftExpress = TRUE;
				NextFlag = TRUE;
			}

			if ( !NextFlag && lstrcmp( decode , "*\300\256\304\371\275\272\306\256" )==0 ) {
				p=fileread_GetWord( strBuff , p );
				if ( strBuff[0] )
					lpMonInfo->WingQuestNpc = atoi(strBuff);
				else
					lpMonInfo->WingQuestNpc = 1;
				NextFlag = TRUE;
			}

			if ( !NextFlag && lstrcmp( decode , "*\304\371\275\272\306\256\300\314\272\245\306\256" )==0 ) {

				p=fileread_GetWord( strBuff , p );
				if ( strBuff[0] )
					lpMonInfo->WingQuestNpc = atoi(strBuff);
				else
					lpMonInfo->WingQuestNpc = 2;

				NextFlag = TRUE;
			}

			if ( !NextFlag && lstrcmp( decode , "*\272\260\306\367\300\316\306\256\300\373\270\263" )==0 ) {
				p=fileread_GetWord( strBuff , p );
				if ( strBuff[0] )
					lpMonInfo->StarPointNpc = atoi(strBuff);
				else
					lpMonInfo->StarPointNpc = 20;

				NextFlag = TRUE;
			}
			if ( !NextFlag && lstrcmp( decode , "*\261\342\272\316\307\324" )==0 ) {
				lpMonInfo->GiveMoneyNpc = TRUE;
				NextFlag = TRUE;
			}

			if ( !NextFlag && lstrcmp( decode , "*\305\332\267\271\306\367\306\256" )==0 ) {
				p=fileread_GetWord( strBuff , p );
				if ( strBuff[0] )
					lpMonInfo->TelePortNpc = atoi(strBuff);
				else
					lpMonInfo->TelePortNpc = 1;

				NextFlag = TRUE;
			}
			if ( !NextFlag && lstrcmp( decode , "*\272\355\267\271\275\272\304\263\275\275" )==0 ) {
				p=fileread_GetWord( strBuff , p );
				if ( strBuff[0] )
					lpMonInfo->BlessCastleNPC = atoi(strBuff);
				else
					lpMonInfo->BlessCastleNPC = 1;

				NextFlag = TRUE;
			}
			if ( !NextFlag && lstrcmp( decode , "*\274\263\271\256\301\266\273\347" )==0 ) {
				p=fileread_GetWord( strBuff , p );
				if ( strBuff[0] )
					lpMonInfo->PollingNpc = atoi(strBuff);
				else
					lpMonInfo->PollingNpc = 1;

				NextFlag = TRUE;
			}

			if ( !NextFlag && lstrcmp( decode , "*\265\277\277\265\273\363\301\246\270\361" )==0 ) {
				pb=p;p=fileread_GetWord(strBuff,p);if(strBuff[0]==34)p=GetString(strBuff,pb);	//단어 찾기
				if ( strBuff[0] ) {
					lpMonInfo->szMediaPlayNPC_Title = SetNPCMsg(strBuff);
				}
				NextFlag = TRUE;
			}
			if ( !NextFlag && lstrcmp( decode , "*\265\277\277\265\273\363\260\346\267\316" )==0 ) {
				pb=p;p=fileread_GetWord(strBuff,p);if(strBuff[0]==34)p=GetString(strBuff,pb);	//단어 찾기
				if ( strBuff[0] ) {
					lpMonInfo->szMediaPlayNPC_Path = SetNPCMsg(strBuff);
				}
				NextFlag = TRUE;
			}
			
			if ( !NextFlag && lstrcmp( decode , "*\303\342\307\366\260\243\260\335" )==0 ) {
				p=fileread_GetWord( strBuff , p );
				lpMonInfo->OpenCount[0] = atoi(strBuff);		//출현 번호
				p=fileread_GetWord( strBuff , p );
				lpMonInfo->OpenCount[1] = atoi(strBuff);		//랜덤 카운트
				NextFlag = TRUE;
			}
			if ( !NextFlag && lstrcmp( decode , "*\304\371\275\272\306\256\304\332\265\345" )==0 ) {
				p=fileread_GetWord( strBuff , p );
				lpMonInfo->QuestCode = atoi(strBuff);		//퀘스트 코드
				p=fileread_GetWord( strBuff , p );
				lpMonInfo->QuestParam = atoi(strBuff);		//퀘스트 파라메터
				NextFlag = TRUE;
			}
		}

		////////////////// 다른 파일에서 계속 읽어 드림 ///////////////
		if ( !NextFlag && lstrcmp( decode , "*\277\254\260\341\306\304\300\317" )==0 ) {
			pb=p;p=fileread_GetWord(strBuff,p);if(strBuff[0]==34)p=GetString(strBuff,pb);	//단어 찾기

			char szNextFile[128];
			GetDirectoryFromFile( szFile , szNextFile );
			lstrcat( szNextFile , strBuff );
			NextFlag = TRUE;
			fclose(fp);

			fp = fopen( szNextFile , "rb" );
			if ( fp==NULL ) break;
		}

	}

	//몬스터 구조체 연결
	if ( lpMonInfo )
		lpCharInfo->lpMonInfo = lpMonInfo;
	else
		lpCharInfo->lpMonInfo = 0;


	if (fp) fclose(fp);

	return TRUE;
}

//몬스터 출현 정보 파일에서 해독하여 설정한다
int DecodeOpenMonster( char *szCharFile , rsSTG_MONSTER_LIST *lpStgMonList , smCHAR_INFO *lpCharInfoList, int ListCnt )
{

	FILE	*fp;
	char strBuff[64];
	char *p;
	char *pb;
	int NextFlag;
	int cnt,MonCnt,PercentCnt;
	sBOSS_MONSTER *lpBossMonster;

	ZeroMemory( lpStgMonList , sizeof( rsSTG_MONSTER ) * rsSTG_MONSTER_MAX );

	//기본 값 설정
	lpStgMonList->OpenInterval = 0x7F;
	lpStgMonList->OpenLimit = 3;

	fp = fopen( szCharFile , "rb" );
	if ( fp==NULL ) return FALSE;

	MonCnt = 0;
	PercentCnt = 0;

	while( !feof( fp ) )//  feof: file end까지 읽어라 
	{
		if( fgets( line, 255, fp ) == NULL)	break;

		p = fileread_GetWord( decode , line);

		NextFlag = 0;

		///////////////// 이름 ///////////////////////
		if ( lstrcmp( decode , "*\303\342\277\254\300\332" )==0 || lstrcmpi( decode , "*ACTOR" )==0 ) {
			pb=p;p=fileread_GetWord(strBuff,p);if(strBuff[0]==34)p=GetString(strBuff,pb);	//단어 찾기
			lstrcpy( lpStgMonList->rsMonster[MonCnt].szMonsterName , strBuff );
			for(cnt=0;cnt<ListCnt;cnt++) {
				if ( lstrcmp( strBuff , lpCharInfoList[cnt].szName )==0 ) {
					lpStgMonList->rsMonster[MonCnt].lpCharInfo = &lpCharInfoList[cnt];
					//출현빈도
					p=fileread_GetWord(strBuff,p);
					cnt = atoi( strBuff );
					lpStgMonList->rsMonster[MonCnt].NumOpenStart = PercentCnt;
					lpStgMonList->rsMonster[MonCnt].OpenPercentage = cnt;
					PercentCnt += cnt;
					MonCnt ++;
					break;
				}
			}

			NextFlag = TRUE;
		}

		if ( lstrcmp( decode , "*\303\342\277\254\300\332\265\316\270\361" )==0 || lstrcmpi( decode , "*BOSS_ACTOR" )==0 ) {

			lpBossMonster = &lpStgMonList->sBossMonsters[lpStgMonList->BossMonsterCount];

			pb=p;p=fileread_GetWord(strBuff,p);if(strBuff[0]==34)p=GetString(strBuff,pb);	//보스 몬스터 이름
			for(cnt=0;cnt<ListCnt;cnt++) {
				if ( lstrcmp( strBuff , lpCharInfoList[cnt].szName )==0 ) {
					lpBossMonster->MasterMonster = &lpCharInfoList[cnt];
					break;
				}
			}

			pb=p;p=fileread_GetWord(strBuff,p);if(strBuff[0]==34)p=GetString(strBuff,pb);	//보조 몬스터 이름
			for(cnt=0;cnt<ListCnt;cnt++) {
				if ( lstrcmp( strBuff , lpCharInfoList[cnt].szName )==0 ) {
					lpBossMonster->SlaveMonster = &lpCharInfoList[cnt];
					break;
				}
			}

			p=fileread_GetWord(strBuff,p);		// 보조 출연 최대인원
			lpBossMonster->SlaveCount = atoi( strBuff );


			for( cnt=0;cnt<32;cnt++) {
				p=fileread_GetWord(strBuff,p);		// 출연 시간
				if ( !strBuff[0] ) break;
				lpBossMonster->bOpenTime[ lpBossMonster->OpenTimeCount++ ] = atoi( strBuff );
			}

			lpStgMonList->BossMonsterCount++;
			NextFlag = TRUE;
		}


		if ( !NextFlag && ( lstrcmp( decode , "*\303\326\264\353\265\277\275\303\303\342\307\366\274\366" )==0 || lstrcmp( decode , "*MAX_ACTOR_POS" )==0 )) {
			p=fileread_GetWord(strBuff,p);
			lpStgMonList->LimitMax = atoi( strBuff );		
			NextFlag = TRUE;
		}

		if ( !NextFlag && ( lstrcmp( decode , "*\303\342\307\366\260\243\260\335" )==0 || lstrcmp( decode , "*DELAY" )==0 ) ) {
			p=fileread_GetWord(strBuff,p);
			lpStgMonList->OpenInterval = (1<<atoi( strBuff ));
			if (lpStgMonList->OpenInterval>1) lpStgMonList->OpenInterval--;

			p=fileread_GetWord(strBuff,p);
			if ( strBuff[0] )
				lpStgMonList->dwIntervalTime = atoi( strBuff )*1000;

			NextFlag = TRUE;
		}

		if ( !NextFlag && ( lstrcmp( decode , "*\303\342\307\366\274\366" )==0 || lstrcmp( decode , "*MAX_ACTOR" )==0 ) ) {
			p=fileread_GetWord(strBuff,p);
			lpStgMonList->OpenLimit = atoi( strBuff );
			NextFlag = TRUE;
		}


	}
	lpStgMonList->Counter		= MonCnt;
	lpStgMonList->PecetageCount = PercentCnt;


	fclose(fp);

	return TRUE;
}


//최소 최대치에서 랜덤값 산출
// GM /DeckMeOut!: CreateDefItem rolls every range at its best (and specialisation requirement
// changes at their lowest)
int Random_MaxMode = 0;

int GetRandomPos( int Min , int Max )
{
	int sb;
	int rnd;

	if ( Random_MaxMode ) return (Random_MaxMode>0)==(Max>Min) ? Max : Min;	// >0 the best, <0 the least

	sb = (Max+1)-Min;

	if ( sb==0 ) return Max;

	rnd = rand()%sb;
	return Min+rnd;
}

//최소 최대치에서 랜덤값 산출
float GetRandomFloatPos( float Min , float Max )
{
	int sb;
	int rnd;

	if ( Random_MaxMode ) return (Random_MaxMode>0)==(Max>Min) ? Max : Min;	// >0 the best, <0 the least

	sb = (int)(( Max - Min)*100.0f);

	if ( sb==0 ) return Max;

	rnd = rand()%(sb+1);
	return Min+((float)((float)rnd/100.0f));
}

//최소 최대치에서 랜덤값 산출 ( 소수점 1자리 )
float GetRandomFloatPos2( float Min , float Max )
{
	int sb;
	int rnd;

	if ( Random_MaxMode ) return (Random_MaxMode>0)==(Max>Min) ? Max : Min;	// >0 the best, <0 the least

	sb = (int)(( Max - Min)*10.0f);

	if ( sb==0 ) return Max;

	rnd = rand()%(sb+1);
	return Min+((float)((float)rnd/10.0f));
}


//최소 최대치에서 랜덤값 산출
int GetMiddlePos( int Min , int Max )
{
	int sb;

	sb = (Max-Min)/2;

	return Min+sb;
}


///////////////////////////////////////
static int	Random_NormalMode = 0;

//최소 최대치에서 랜덤값 산출
int GetRandomPos_Normal( int inMin , int inMax )
{
	int sb,sc;
	int rnd;
	int	Min,Max;

	if ( Random_MaxMode ) return GetRandomPos( inMin , inMax );

	if ( !Random_NormalMode ) 
		return GetRandomPos( inMin , inMax );

	sc = inMax-inMin;
	sb = sc/4;

	if ( sb==0 && sc>=2 ) sb=1;

	Min = inMin+sb;
	Max = inMax-sb;

	sb = (Max+1)-Min;

	if ( sb==0 ) return Max;

	rnd = rand()%sb;
	return Min+rnd;
}
/*
//최소 최대치에서 랜덤값 산출
float GetRandomFloatPos_Normal( float inMin , float inMax )
{
	int sb;
	int rnd;
	float fsb;
	float Min,Max;

	if ( !Random_NormalMode || inMin==inMax ) 
		return GetRandomFloatPos( inMin , inMax );

	fsb = (inMax-inMin)/4;
	Min = inMin+fsb;
	Max = inMax-fsb;

	sb = (int)(( Max - Min)*100.0f);

	if ( sb==0 ) return Max;

	rnd = rand()%(sb+1);
	return Min+((float)((float)rnd/100.0f));
}
*/
/////////////////////////////////////////////



struct sSPECIAL_REVISION {
	DWORD	dwJobCode;
	short	sLevel[2];
	short	sStrength[2];
	short	sSpirit[2];
	short	sTalent[2];
	short	sDexterity[2];
	short	sHealth[2];
};

#define	sSPECIAL_REVISION_MAX		8
// 특화 지업별  요구 능력치 보정표
sSPECIAL_REVISION	sSpecialRevision[sSPECIAL_REVISION_MAX] = {
	//      직업                레벨     힘			정신       재능			민첩        건강
	{ JOB_CODE_MECHANICIAN,		0,0,	5,15,		-20,-10,	0,0,		-25,-15,	0,0		},
	{ JOB_CODE_FIGHTER,			0,0,	10,15,		-20,-15,	0,0,		-20,-15,	0,0		},
	{ JOB_CODE_PIKEMAN,			0,0,	10,15,		-20,-15,	0,0,		-25,-15,	0,0		},
	{ JOB_CODE_ARCHER,			0,0,	-25,-15,	-20,-10,	0,0,		15,25,		0,0		},

	{ JOB_CODE_KNIGHT,			0,0,	5,15,		-15,-10,	5,10,		-25,-15,	0,0		},
	{ JOB_CODE_ATALANTA,		0,0,	-20,-15,	-20,-10,	0,0,		15,25,		0,0		},
	{ JOB_CODE_PRIEST,			0,0,	-25,-20,	15,20,		-15,-10,	-20,-15,	0,0		},
	{ JOB_CODE_MAGICIAN,		0,0,	-25,-20,	15,25,		-15,-10,	-20,-15,	0,0		}

};

/*
// 특화 지업별  요구 능력치 보정표 (2004년7월20일수정)
sSPECIAL_REVISION	sSpecialRevision[sSPECIAL_REVISION_MAX] = {
	//      직업                레벨     힘			정신       재능			민첩        건강
	{ JOB_CODE_MECHANICIAN,		0,0,	5,15,		-20,-10,	0,0,		-25,-20,	0,0		},
	{ JOB_CODE_FIGHTER,			0,0,	10,15,		-20,-15,	0,0,		-25,-20,	0,0		},
	{ JOB_CODE_PIKEMAN,			0,0,	10,15,		-20,-15,	0,0,		-25,-20,	0,0		},
	{ JOB_CODE_ARCHER,			0,0,	-40,-30,	-20,-10,	0,0,		15,25,		0,0		},

	{ JOB_CODE_KNIGHT,			0,0,	5,15,		-15,-10,	5,10,		-30,-20,	0,0		},
	{ JOB_CODE_ATALANTA,		0,0,	-30,-20,	-20,-10,	0,0,		15,25,		0,0		},
	{ JOB_CODE_PRIEST,			0,0,	-25,-20,	15,20,		-15,-10,	-25,-20,	0,0		},
	{ JOB_CODE_MAGICIAN,		0,0,	-25,-20,	15,25,		-15,-10,	-25,-20,	0,0		}

};
*/

//아이템 생성
int CreateDefItem( sITEMINFO *lpItem , sDEF_ITEMINFO *lpDefItem , int SpJobCode , int ItemNormam )
{
	int is,ie;
	int cnt;
	JOB_DATA_CODE	*lpJobDataCode = 0;

	Random_NormalMode = ItemNormam;

	//--------------------------------------------------------------< _ignore_bug_fix_0x00000000_	//해외
	//ZeroMemory(lpItem,sizeof(sITEMINFO));
	//--------------------------------------------------------------  _ignore_bug_fix_0x00000000_ >
	memcpy( lpItem , &lpDefItem->Item , sizeof(sITEMINFO) );

	//내구력 설정
	if ( lpDefItem->sDurability[0] ) {

		if ( lpDefItem->sDurability[1] )
			lpItem->Durability[1] = GetRandomPos_Normal(lpDefItem->sDurability[0],lpDefItem->sDurability[1]);
		else
			lpItem->Durability[1] = lpDefItem->sDurability[0];

		ie = lpItem->Durability[1];
		is = ie/2;
		lpItem->Durability[0] = GetRandomPos_Normal( is, ie );
	}

	//8개의 원소값 설정
	for( cnt=0;cnt<8;cnt++ ) {
		if ( lpDefItem->sResistance[cnt][1] ) {
			lpItem->Resistance[cnt] = 
				GetRandomPos_Normal( lpDefItem->sResistance[cnt][0] , lpDefItem->sResistance[cnt][1] );
		}
		else
			lpItem->Resistance[cnt] = lpDefItem->sResistance[cnt][0];
	}

	//공격력 설정
	if ( lpDefItem->sDamage[1] || lpDefItem->sDamage[3] ) {
		lpItem->Damage[0] = GetRandomPos_Normal(lpDefItem->sDamage[0],lpDefItem->sDamage[1]);
		lpItem->Damage[1] = GetRandomPos_Normal(lpDefItem->sDamage[2],lpDefItem->sDamage[3]);
	}

	//명중력 설정
	if ( lpDefItem->sAttack_Rating[1] ) 
		lpItem->Attack_Rating = GetRandomPos_Normal(lpDefItem->sAttack_Rating[0],lpDefItem->sAttack_Rating[1]);
	else
		lpItem->Attack_Rating = lpDefItem->sAttack_Rating[0];

	//흡수력 설정
	if ( lpDefItem->fAbsorb[1]!=0 ) 
		lpItem->fAbsorb = GetRandomFloatPos2(lpDefItem->fAbsorb[0],lpDefItem->fAbsorb[1]);
	else
		lpItem->fAbsorb = lpDefItem->fAbsorb[0];

	//방어력 설정
	if ( lpDefItem->sDefence[1] )
		lpItem->Defence = GetRandomPos_Normal(lpDefItem->sDefence[0],lpDefItem->sDefence[1]);
	else
		lpItem->Defence = lpDefItem->sDefence[0];

	//블럭율 설정
	if ( lpDefItem->fBlock_Rating[1]!=0 )
		lpItem->fBlock_Rating = GetRandomFloatPos(lpDefItem->fBlock_Rating[0],lpDefItem->fBlock_Rating[1]);
	else
		lpItem->fBlock_Rating = lpDefItem->fBlock_Rating[0];


	//이동속도 설정
	if ( lpDefItem->fSpeed[1]!=0 )
		lpItem->fSpeed = GetRandomFloatPos2(lpDefItem->fSpeed[0],lpDefItem->fSpeed[1]);
	else
		lpItem->fSpeed = lpDefItem->fSpeed[0];


	//마나재생
	if ( lpDefItem->fMana_Regen[1]!=0 )
		lpItem->fMana_Regen = GetRandomFloatPos2(lpDefItem->fMana_Regen[0],lpDefItem->fMana_Regen[1]);
	else
		lpItem->fMana_Regen = lpDefItem->fMana_Regen[0];

	//라이프재생
	if ( lpDefItem->fLife_Regen[1]!=0 )
		lpItem->fLife_Regen = GetRandomFloatPos2(lpDefItem->fLife_Regen[0],lpDefItem->fLife_Regen[1]);
	else
		lpItem->fLife_Regen = lpDefItem->fLife_Regen[0];

	//스테미나재생
	if ( lpDefItem->fStamina_Regen[1]!=0 )
		lpItem->fStamina_Regen = GetRandomFloatPos2(lpDefItem->fStamina_Regen[0],lpDefItem->fStamina_Regen[1]);
	else
		lpItem->fStamina_Regen = lpDefItem->fStamina_Regen[0];


	//라이프 증가
	if ( lpDefItem->Increase_Life[1] )
		lpItem->fIncrease_Life = (float)GetRandomPos_Normal(lpDefItem->Increase_Life[0],lpDefItem->Increase_Life[1]);
	else
		lpItem->fIncrease_Life = (float)lpDefItem->Increase_Life[0];

	//마나 증가
	if ( lpDefItem->Increase_Mana[1] )
		lpItem->fIncrease_Mana = (float)GetRandomPos_Normal(lpDefItem->Increase_Mana[0],lpDefItem->Increase_Mana[1]);
	else
		lpItem->fIncrease_Mana = (float)lpDefItem->Increase_Mana[0];

	//스테미나 증가
	if ( lpDefItem->Increase_Stamina[1] )
		lpItem->fIncrease_Stamina = (float)GetRandomPos_Normal(lpDefItem->Increase_Stamina[0],lpDefItem->Increase_Stamina[1]);
	else
		lpItem->fIncrease_Stamina = (float)lpDefItem->Increase_Stamina[0];




	DWORD	dwSpecialJob;
	int		RndPos;
	int		Revision;
	int		tempJobCode = 0; // 박재원 - 리스펙 스톤
	
	// 박재원 - 리스펙 스톤
	if(SpJobCode>=100)
	{
		tempJobCode = SpJobCode - 100; // 자신의 직업코드를 얻는다. tempJobCode
		SpJobCode = 0;
	}

	if ( SpJobCode>0 ) {
		lpJobDataCode = GetJobDataCode(	SpJobCode, 0 );

		if ( lpJobDataCode ) {
			dwSpecialJob = lpJobDataCode->JobBitCode;

			for(cnt=0;cnt<lpDefItem->JobBitCodeRandomCount;cnt++) {
				if ( lpDefItem->dwJobBitCode_Random[cnt]==dwSpecialJob ) break;
			}
			if ( cnt>=lpDefItem->JobBitCodeRandomCount ) {
				if ( dwSpecialJob!=lpItem->JobCodeMask ) SpJobCode = 0;		//특화 발생 불가 아이템
			}
		}
	}


	if ( lpItem->JobCodeMask || lpDefItem->JobBitCodeRandomCount || SpJobCode ) {
		//특화 아이템 설정

		//랜덤 특화직업 추가
		cnt = rand()%10;			//특화 추가될 확률 20%

		// GM /DeckMeOut!: the player's class or no specialisation at all
		if ( Random_MaxMode && !SpJobCode ) cnt = 100;

		// 석지용 - 믹스쳐 리셋 (무특화를 위해 완죤 날치기를....;;;)
		if( NotItemSpecial )
		{
			cnt = 100;
			NotItemSpecial = 0;
		}

		if ( cnt<=3 || SpJobCode ) {
			//&& lpDefItem->JobBitCodeRandomCount ) {

			cnt = rand()%10;			//기본 특화 확률 70%

			dwSpecialJob = lpItem->JobCodeMask;

			if ( cnt<3 && lpDefItem->JobBitCodeRandomCount ) {
				lpItem->JobCodeMask = 0;
			}

			if ( !lpItem->JobCodeMask && lpDefItem->JobBitCodeRandomCount ) 
			{
				if(tempJobCode>0) // 박재원 - 리스펙 스톤
				{
					lpJobDataCode = GetJobDataCode(	tempJobCode, 0 );
					if(lpJobDataCode)
					{
						cnt = rand()%lpDefItem->JobBitCodeRandomCount;
						while(lpDefItem->dwJobBitCode_Random[cnt] == lpJobDataCode->JobBitCode) // 박재원 - 내 직업코드와 아이템 직업코드가 같을 경우 다른 직업코드가 나올때까지 반복한다.
						{
							cnt = rand()%lpDefItem->JobBitCodeRandomCount;
							if(lpDefItem->dwJobBitCode_Random[cnt] != lpJobDataCode->JobBitCode) 
							{
								dwSpecialJob = lpDefItem->dwJobBitCode_Random[cnt];
								lpItem->JobCodeMask = dwSpecialJob;			
								break;						
							}	
						}
					}
				}
				else //추가 특화 직업
				{
					cnt = rand()%lpDefItem->JobBitCodeRandomCount;
					dwSpecialJob = lpDefItem->dwJobBitCode_Random[cnt];
					lpItem->JobCodeMask = dwSpecialJob;
				}
			}

			if ( SpJobCode>0 ) {
				//강제 특화 발생
				lpJobDataCode = GetJobDataCode(	SpJobCode, 0 );
				if ( lpJobDataCode ) {
					dwSpecialJob = lpJobDataCode->JobBitCode;
					lpItem->JobCodeMask = dwSpecialJob;
				}
			}

			// 장별 - 리스펙스톤 사용시 가격 버그 수정
			if( lpItem->JobCodeMask) lpItem->Price += (lpItem->Price*2)/10;		//특화 가격 상승 20%
				


			// 특화 지업별  요구능력치 보정 (GM /DeckMeOut!: the smallest requirements)
			int SaveMaxMode = Random_MaxMode;
			if ( Random_MaxMode ) Random_MaxMode = -1;
			for( cnt=0;cnt<sSPECIAL_REVISION_MAX;cnt++ ) {
				if ( sSpecialRevision[cnt].dwJobCode==dwSpecialJob ) {
					//레벨
					if ( lpItem->Level && sSpecialRevision[cnt].sLevel[0] ) {
						RndPos = GetRandomPos_Normal( sSpecialRevision[cnt].sLevel[0] , sSpecialRevision[cnt].sLevel[1] );
						Revision = (lpItem->Level*RndPos)/100;
						lpItem->Level += Revision;
					}
					//힘
					if ( lpItem->Strength && sSpecialRevision[cnt].sStrength[0] ) {
						RndPos = GetRandomPos_Normal( sSpecialRevision[cnt].sStrength[0] , sSpecialRevision[cnt].sStrength[1] );
						Revision = (lpItem->Strength*RndPos)/100;
						lpItem->Strength += Revision;
					}
					//정신
					if ( lpItem->Spirit && sSpecialRevision[cnt].sSpirit[0] ) {
						RndPos = GetRandomPos_Normal( sSpecialRevision[cnt].sSpirit[0] , sSpecialRevision[cnt].sSpirit[1] );
						Revision = (lpItem->Spirit*RndPos)/100;
						lpItem->Spirit += Revision;
					}
					//재능
					if ( lpItem->Talent && sSpecialRevision[cnt].sTalent[0] ) {
						RndPos = GetRandomPos_Normal( sSpecialRevision[cnt].sTalent[0] , sSpecialRevision[cnt].sTalent[1] );
						Revision = (lpItem->Talent*RndPos)/100;
						lpItem->Talent += Revision;
					}
					//민첩
					if ( lpItem->Dexterity && sSpecialRevision[cnt].sDexterity[0] ) {
						RndPos = GetRandomPos_Normal( sSpecialRevision[cnt].sDexterity[0] , sSpecialRevision[cnt].sDexterity[1] );
						Revision = (lpItem->Dexterity*RndPos)/100;
						lpItem->Dexterity += Revision;
					}
					//건강
					if ( lpItem->Health && sSpecialRevision[cnt].sHealth[0] ) {
						RndPos = GetRandomPos_Normal( sSpecialRevision[cnt].sHealth[0] , sSpecialRevision[cnt].sHealth[1] );
						Revision = (lpItem->Health*RndPos)/100;
						lpItem->Health += Revision;
					}
				}
			}
			Random_MaxMode = SaveMaxMode;
/*
			if ( dwSpecialJob==JOB_CODE_ARCHER || dwSpecialJob==JOB_CODE_ATALANTA ) {
				//아쳐 아탈 특화
				if ( (lpItem->CODE&sinITEM_MASK2)==sinDA1 ) {		//갑옷
					lpItem->Dexterity = lpItem->Level + 20;			//민첩추가
				}
			}
*/
			//흡수력
			if ( lpDefItem->fSpecial_Absorb[1]!=0 ) {
				lpItem->JobItem.Add_fAbsorb = GetRandomFloatPos(lpDefItem->fSpecial_Absorb[0],lpDefItem->fSpecial_Absorb[1]);
			}
			else {
				lpItem->JobItem.Add_fAbsorb = lpDefItem->fSpecial_Absorb[0];
			}
		
			//방어력
			if ( lpDefItem->sSpecial_Defence[1] ) {
				lpItem->JobItem.Add_Defence = GetRandomPos_Normal(lpDefItem->sSpecial_Defence[0],lpDefItem->sSpecial_Defence[1]);
			}
			else {
				lpItem->JobItem.Add_Defence = lpDefItem->sSpecial_Defence[0];
			}

			//이동속도
			if ( lpDefItem->fSpecial_fSpeed[1]!=0 ) {
				lpItem->JobItem.Add_fSpeed = GetRandomFloatPos(lpDefItem->fSpecial_fSpeed[0],lpDefItem->fSpecial_fSpeed[1]);
			}
			else {
				lpItem->JobItem.Add_fSpeed = lpDefItem->fSpecial_fSpeed[0];
			}

			//마법숙련도
			if ( lpDefItem->fSpecial_Magic_Mastery[1]!=0 ) {
				lpItem->JobItem.Add_fMagic_Mastery = GetRandomFloatPos(lpDefItem->fSpecial_Magic_Mastery[0],lpDefItem->fSpecial_Magic_Mastery[1]);
			}
			else {
				lpItem->JobItem.Add_fMagic_Mastery = lpDefItem->fSpecial_Magic_Mastery[0];
			}

			//기력 재생
			if ( lpDefItem->fSpecial_Mana_Regen[1]!=0 ) {
				lpItem->JobItem.Per_Mana_Regen = GetRandomFloatPos(lpDefItem->fSpecial_Mana_Regen[0],lpDefItem->fSpecial_Mana_Regen[1]);
			}
			else {
				lpItem->JobItem.Per_Mana_Regen = lpDefItem->fSpecial_Mana_Regen[0];
			}


			//특화 명중력
			if ( lpDefItem->Lev_Attack_Rating[1] ) {
				lpItem->JobItem.Lev_Attack_Rating = GetRandomPos_Normal(lpDefItem->Lev_Attack_Rating[0],lpDefItem->Lev_Attack_Rating[1]);
			}
			else {
				lpItem->JobItem.Lev_Attack_Rating = lpDefItem->Lev_Attack_Rating[0];
			}


		}
		else {
			//특화 없음
			lpItem->JobCodeMask = 0;
		}
	}

	if ( !lpItem->JobCodeMask ) {
		ZeroMemory( &lpItem->JobItem , sizeof(sITEM_SPECIAL) );
	}


/*
		///////////////// *흡수력 ///////////////////////
		if ( !NextFlag && lstrcmp( decode , "**흡수력" )==0 ) {
			p=fileread_GetWord(strBuff,p);
			lpDefItem->fSpecial_Absorb[0] = (float)atof(strBuff);
			p=fileread_GetWord(strBuff,p);
			lpDefItem->fSpecial_Absorb[1] = (float)atof(strBuff);			
			//lpItem->JobItem.Add_fAbsorb = (float)atof(strBuff);
			NextFlag = TRUE;
		}
		///////////////// *방어력 ///////////////////////
		if ( !NextFlag && lstrcmp( decode , "**방어력" )==0 ) {
			p=fileread_GetWord(strBuff,p);
			lpDefItem->sSpecial_Defence[0] = atoi(strBuff);
			p=fileread_GetWord(strBuff,p);
			lpDefItem->sSpecial_Defence[1] = atoi(strBuff);
//			lpItem->JobItem.Add_Defence = atoi(strBuff);
			NextFlag = TRUE;
		}
*/

	//아이템 인증 받기
	ReformItem( lpItem );

	return	TRUE;
}

//아이템 생성
int CreateDefItem( sITEMINFO *lpItem , sDEF_ITEMINFO *lpDefItem , int SpJobCode )
{
	return CreateDefItem( lpItem , lpDefItem , SpJobCode , 0 );
}


//후진 아이템 설정
int SetLowerItem( sDEF_ITEMINFO *lpDefItem  )
{
	int ie;
	int cnt;

	sITEMINFO *lpItem;
	lpItem = &lpDefItem->Item;

	//내구력 설정
	if ( lpDefItem->sDurability[0] ) {
		ie = lpDefItem->sDurability[0];
		lpItem->Durability[0] = ie;
		lpItem->Durability[1] = ie;
	}

	//8개의 원소값 설정
	for( cnt=0;cnt<8;cnt++ ) {
		lpItem->Resistance[cnt] = lpDefItem->sResistance[cnt][0];
	}

	//공격력 설정
	if ( lpDefItem->sDamage[1] || lpDefItem->sDamage[3] ) {
		lpItem->Damage[0] = lpDefItem->sDamage[0];
		lpItem->Damage[1] = lpDefItem->sDamage[2];
	}

	lpItem->Attack_Rating = lpDefItem->sAttack_Rating[0];
	lpItem->fAbsorb = lpDefItem->fAbsorb[0];
	lpItem->Defence = lpDefItem->sDefence[0];
	lpItem->fBlock_Rating = lpDefItem->fBlock_Rating[0];
	lpItem->fSpeed = lpDefItem->fSpeed[0];

	lpItem->fMana_Regen = lpDefItem->fMana_Regen[0];		//마나재생
	lpItem->fLife_Regen = lpDefItem->fLife_Regen[0];		//라이프재생
	lpItem->fStamina_Regen = lpDefItem->fStamina_Regen[0];	//스테미나재생

	lpItem->fIncrease_Life = (float)lpDefItem->Increase_Life[0];
	lpItem->fIncrease_Mana = (float)lpDefItem->Increase_Mana[0];
	lpItem->fIncrease_Stamina = (float)lpDefItem->Increase_Stamina[0];

/*
	if ( lpItem->JobCodeMask ) {
		//특화 아이템 설정
		lpItem->JobItem.Add_fAbsorb = lpDefItem->fSpecial_Absorb[0];
		lpItem->JobItem.Add_Defence = lpDefItem->sSpecial_Defence[0];
		lpItem->JobItem.Add_fSpeed = lpDefItem->fSpecial_fSpeed[0];
	}
*/

	DWORD	dwSpecialJob;
	int		RndPos;
	int		Revision;


	if ( lpItem->JobCodeMask ) {
		//특화 아이템 설정

			dwSpecialJob = lpItem->JobCodeMask;

			// 특화 지업별  요구능력치 보정 ( 능력치는 중간으로 고정 )
			for( cnt=0;cnt<sSPECIAL_REVISION_MAX;cnt++ ) {
				if ( sSpecialRevision[cnt].dwJobCode==dwSpecialJob ) {
					//레벨
					if ( lpItem->Level && sSpecialRevision[cnt].sLevel[0] ) {
						RndPos = GetMiddlePos( sSpecialRevision[cnt].sLevel[0] , sSpecialRevision[cnt].sLevel[1] );
						Revision = (lpItem->Level*RndPos)/100;
						lpItem->Level += Revision;
					}
					//힘
					if ( lpItem->Strength && sSpecialRevision[cnt].sStrength[0] ) {
						RndPos = GetMiddlePos( sSpecialRevision[cnt].sStrength[0] , sSpecialRevision[cnt].sStrength[1] );
						Revision = (lpItem->Strength*RndPos)/100;
						lpItem->Strength += Revision;
					}
					//정신
					if ( lpItem->Spirit && sSpecialRevision[cnt].sSpirit[0] ) {
						RndPos = GetMiddlePos( sSpecialRevision[cnt].sSpirit[0] , sSpecialRevision[cnt].sSpirit[1] );
						Revision = (lpItem->Spirit*RndPos)/100;
						lpItem->Spirit += Revision;
					}
					//재능
					if ( lpItem->Talent && sSpecialRevision[cnt].sTalent[0] ) {
						RndPos = GetMiddlePos( sSpecialRevision[cnt].sTalent[0] , sSpecialRevision[cnt].sTalent[1] );
						Revision = (lpItem->Talent*RndPos)/100;
						lpItem->Talent += Revision;
					}
					//민첩
					if ( lpItem->Dexterity && sSpecialRevision[cnt].sDexterity[0] ) {
						RndPos = GetMiddlePos( sSpecialRevision[cnt].sDexterity[0] , sSpecialRevision[cnt].sDexterity[1] );
						Revision = (lpItem->Dexterity*RndPos)/100;
						lpItem->Dexterity += Revision;
					}
					//건강
					if ( lpItem->Health && sSpecialRevision[cnt].sHealth[0] ) {
						RndPos = GetMiddlePos( sSpecialRevision[cnt].sHealth[0] , sSpecialRevision[cnt].sHealth[1] );
						Revision = (lpItem->Health*RndPos)/100;
						lpItem->Health += Revision;
					}
				}
			}

			//흡수력
			lpItem->JobItem.Add_fAbsorb = lpDefItem->fSpecial_Absorb[0];
		
			//방어력
			lpItem->JobItem.Add_Defence = lpDefItem->sSpecial_Defence[0];

			//이동속도
			lpItem->JobItem.Add_fSpeed = lpDefItem->fSpecial_fSpeed[0];

			//마법숙련도
			lpItem->JobItem.Add_fMagic_Mastery = lpDefItem->fSpecial_Magic_Mastery[0];

			//기력 재생
			lpItem->JobItem.Per_Mana_Regen = lpDefItem->fSpecial_Mana_Regen[0];

			//특화 명중력
			if ( lpDefItem->Lev_Attack_Rating[1] ) 
				lpItem->JobItem.Lev_Attack_Rating = lpDefItem->Lev_Attack_Rating[0]+(lpDefItem->Lev_Attack_Rating[1]-lpDefItem->Lev_Attack_Rating[0])/2;
			else
				lpItem->JobItem.Lev_Attack_Rating = lpDefItem->Lev_Attack_Rating[0];

	}
	else {
		ZeroMemory( &lpItem->JobItem , sizeof(sITEM_SPECIAL) );
	}

	//아이템 인증 받기
	ReformItem( lpItem );

	return	TRUE;
}

//직업 정보 구하기 ( 기본 직업 [0~3] , 전업 횟수 [0~3] )
JOB_DATA_CODE	*GetJobDataCode(	DWORD BaseJob	, int	JobCnt )
{
	int cnt;
	int	JobCount;
	int JobCount2;
	int	bJob;

	cnt = 0;
	JobCount = 0;
	JobCount2 = JobCnt;					// ranks 0..4 (4: the fifth rank, tier 5 skills)
	if ( JobCount2<0 || JobCount2>4 ) JobCount2 = 4;
	bJob = BaseJob&0xFFFF;

	if ( !JobCnt ) {
		JobCount2 = BaseJob>>24;
	}

	while(1) {
		if ( !JobDataBase[cnt].szName[0] ) break;
		if ( bJob==JobDataBase[cnt].JobCode ) {
			if ( JobCount2==JobCount ) 
				return &JobDataBase[cnt];
			JobCount++;
		}
		cnt++;
	}

	return NULL;
}


#ifdef	_LANGUAGE_KOREAN
//한국어
#include "Korean\\k_JobCode.h"
#endif

#ifdef	_LANGUAGE_CHINESE
//중국어
#include "Chinese\\C_JobCode.h"
#endif

#ifdef	_LANGUAGE_JAPANESE
//일본어
#include "Japanese\\J_JobCode.h"
#endif

#ifdef	_LANGUAGE_TAIWAN
//자유중국
#include "Taiwan\\T_JobCode.h"
#endif

#ifdef	_LANGUAGE_ENGLISH
//영국어
#include "English\\E_JobCode.h"
#endif

#ifdef	_LANGUAGE_THAI
//태국어
#include "THAI\\TH_JobCode.h"
#endif

#ifdef _LANGUAGE_VEITNAM
#include "VEITNAM\\V_JobCode.h"
#endif

#ifdef _LANGUAGE_BRAZIL
#include "BRAZIL\\B_JobCode.h"
#endif

#ifdef _LANGUAGE_ARGENTINA
#include "ARGENTINA\\A_JobCode.h"
#endif


#include "sinbaram\\NewClassSkill.h"		// the new classes' cast codes
SKILL_DATA_CODE	SkillDataCode[] = {
	{	"xxxxxxxxxxxxx"			,	0								},		//NULL Code
	//1단계 메카니션
	{	"ExtremeShield"			,	SKILL_PLAY_EXTREME_SHIELD		},
	{	"MechanicBomb"			,	SKILL_PLAY_MECHANIC_BOMB		},
	{	"PhysicalAbsorption"	,	SKILL_PLAY_PHYSICAL_ABSORB		},
	{	"PoisonAttribute"		,	0								},
	//2단계 메카닉마스터
	{	"roundimpact"			,	SKILL_PLAY_GREAT_SMASH			},
	{	"Maximize"				,	SKILL_PLAY_MAXIMIZE				},
	{	"Automation"			,	SKILL_PLAY_AUTOMATION			},
	{	"Spark"					,	SKILL_PLAY_SPARK				},
	//3단계 메탈리더
	{	"MetalArmor"			,	SKILL_PLAY_METAL_ARMOR			},
	{	"GrandSmash"			,	SKILL_PLAY_GRAND_SMASH			},	//10
	{	"SparkShield"			,	SKILL_PLAY_SPARK_SHIELD			},
	//4단계 메탈리온
	{	"IMPULSION"				,	SKILL_PLAY_IMPULSION			},
	{	"COMPULSION"			,	SKILL_PLAY_COMPULSION			},
	{	"MAGNETICSPHERE"		,	SKILL_PLAY_MAGNETIC_SPHERE		},
	{	"METALGOLEM"			,	SKILL_PLAY_METAL_GOLEM			},


	//1단계 파이터
	{	"MeleeMastery"			,	0								},
	{	"FireAttribute"			,	0								},
	{	"Raving"				,	SKILL_PLAY_RAVING				},
	{	"Impact"				,	SKILL_PLAY_IMPACT				},
	//2단계 워리어
	{	"TripleImpact"			,	SKILL_PLAY_TRIPLE_IMPACT		},	//20
	{	"BrutalSwing"			,	SKILL_PLAY_BRUTAL_SWING			},
	{	"Roar"					,	SKILL_PLAY_ROAR					},
	{	"RageofZecram"			,	SKILL_PLAY_RAGEOF_ZECRAM		},
	//3단계 챔피언
	{	"Concentration"			,	SKILL_PLAY_CONCENTRATION		},
	{	"AvangingCrash"			,	SKILL_PLAY_AVANGING_CRASH		},
	{	"SwiftAxe"				,	SKILL_PLAY_SWIFT_AXE			},
	{	"BoneSmash"				,	SKILL_PLAY_BONE_SMASH			},
	//4단계 이모탈워리어
	{	"DESTROYER"				,	SKILL_PLAY_DESTROYER			},
	{	"BERSERKER"				,	SKILL_PLAY_BERSERKER			},
	{	"CYCLONESTRIKE"			,	SKILL_PLAY_CYCLONE_STRIKE		},	//30


	//1단계 파이크맨
	{	"PikeWind"				,	SKILL_PLAY_PIKEWIND				},
	{	"IceAttribute"			,	0								},
	{	"CriticalHit"			,	SKILL_PLAY_CRITICAL_HIT			},
	{	"JumpingCrash"			,	SKILL_PLAY_JUMPING_CRASH		},
	//2단계 컴배턴트
	{	"GroundPike"			,	SKILL_PLAY_GROUND_PIKE			},
	{	"Tornado"				,	SKILL_PLAY_TORNADO				},
	{	"WeaponDefenceMastery"	,	0								},
	{	"Expansion"				,	SKILL_PLAY_EXPANSION			},
	//3단계 랜서
	{	"VenomSpear"			,	SKILL_PLAY_VENOM_SPEAR			},
	//{	"Vanish"				,	SKILL_PLAY_VANISH				},	//BladeOfBlaze 가 변경됬음
	{	"BladeOfBlaze"			,	SKILL_PLAY_BLADE_OF_BLAZE		},	//사용 금지 예정	//40
	{	"ChainLance"			,	SKILL_PLAY_CHAIN_LANCE			},
	//4단계 랜슬롯
	{	"ASSASSINEYE"			,	SKILL_PLAY_ASSASSIN_EYE			},
	{	"CHARGINGSTRIKE"		,	SKILL_PLAY_CHARGING_STRIKE		},
	{	"VAGUE"					,	SKILL_PLAY_VAGUE				},
	{	"SHADOWMASTER"			,	SKILL_PLAY_SHADOW_MASTER		},



	//1단계 아쳐
	{	"ScoutHawk"				,	SKILL_PLAY_SCOUT_HAWK			},
	{	"ShootingMastery"		,	0								},
	{	"WindArrow"				,	SKILL_PLAY_WIND_ARROW			},
	{	"PerfectAim"			,	SKILL_PLAY_PERFECT_AIM			},
	//2단계 헌트리스마스터
	{	"DionsEye"				,	0								},
	{	"Falcon"				,	SKILL_PLAY_FALCON				},
	{	"ArrowofRage"			,	SKILL_PLAY_ARROWOF_RAGE			},
	{	"Avalanche"				,	SKILL_PLAY_AVALANCHE			},
	//3단계 디온의수제자
	{	"ElementalShot"			,	SKILL_PLAY_ELEMENTAL_SHOT		},
	{	"GoldenFalcon"			,	SKILL_PLAY_GOLDEN_FALCON		},
	{	"BombShot"				,	SKILL_PLAY_BOMB_SHOT			},
	{	"Perforation"			,	SKILL_PLAY_PERFORATION			},
	//4단계 새지테리온
	{	"RECALLWOLVERIN"		,	SKILL_PLAY_RECALL_WOLVERIN		},
	{	"PHOENIXSHOT"			,	SKILL_PLAY_PHOENIX_SHOT			},
	{	"FORCEOFNATURE"			,	SKILL_PLAY_FORCE_OF_NATURE		},


	/////////////////////////// 모라이온 //////////////////////////////

	//1단계 나이트
	{	"SwordBlast"			,	SKILL_PLAY_SWORD_BLAST			},
	{	"HolyBody"				,	SKILL_PLAY_HOLY_BODY			},
	{	"PhysicalTraning"		,	0								},
	{	"DoubleCrash"			,	SKILL_PLAY_DOUBLE_CRASH			},
	//2단계	팔라딘
	{	"holyteam"				,	SKILL_PLAY_HOLY_VALOR			},
	{	"Brandish"				,	SKILL_PLAY_BRANDISH				},
	{	"Piercing"				,	SKILL_PLAY_PIERCING				},
	{	"DrasticSpirit"			,	SKILL_PLAY_DRASTIC_SPIRIT		},
	//3단계	홀리나이트
	{	"FlameBrandish"			,	SKILL_PLAY_FLAME_BRANDISH		},
	{	"Divineinhalation"		,	SKILL_PLAY_DIVINE_INHALATION	},
	{	"HolyIncantation"		,	SKILL_PLAY_HOLY_INCANTATION		},
	{	"GrandCross"			,	SKILL_PLAY_GRAND_CROSS			},
	//4단계	세인트나이트
	{	"DIVINEPIERCING"		,	SKILL_PLAY_DIVINE_PIERCING		},
	{	"DIVINEPIERCING2"		,	SKILL_PLAY_DIVINE_PIERCING2		},
	{	"DIVINEPIERCING3"		,	SKILL_PLAY_DIVINE_PIERCING3		},
	{	"GODLYSHIELD"			,	SKILL_PLAY_GODLY_SHIELD			},
	{	"GODSBLESS"				,	SKILL_PLAY_GODS_BLESS			},
	{	"SWORDOFJUSTICE"		,	SKILL_PLAY_SWORD_OF_JUSTICE		},


	//1단계 아탈란타
	{	"ShieldStrike"			,	SKILL_PLAY_SHIELD_STRIKE		},
	{	"Farina"				,	SKILL_PLAY_FARINA				},
	{	"ThrowingMastery"		,	0								},
	{	"VigorSpear"			,	SKILL_PLAY_VIGOR_SPEAR			},
	//2단계 발키리
	{	"Windy"					,	SKILL_PLAY_WINDY				},
	{	"TwistJavelin"			,	SKILL_PLAY_TWIST_JAVELIN		},
	{	"SoulSucker"			,	SKILL_PLAY_SOUL_SUCKER			},
	{	"FireJavelin"			,	SKILL_PLAY_FIRE_JAVELIN			},
	//3단계 Brunhild
	{	"SplitJavelin"			,	SKILL_PLAY_SPLIT_JAVELIN		},
	{	"TriumphOfValhalla"		,	SKILL_PLAY_TRIUMPH_OF_VALHALLA	},
	{	"LightningJavelin"		,	SKILL_PLAY_LIGHTNING_JAVELIN	},
	{	"StormJavelin"			,	SKILL_PLAY_STORM_JAVELIN		},
	//4단계 발할라
	{	"HALLOFVALHALLA"		,	SKILL_PLAY_HALL_OF_VALHALLA		},
	{	"XRAGE"					,	SKILL_PLAY_X_RAGE				},
	{	"FROSTJAVELIN"			,	SKILL_PLAY_FROST_JAVELIN		},
	{	"VENGEANCE"				,	SKILL_PLAY_VENGEANCE			},


	//1단계 프리스티스
	{	"Healing"				,	SKILL_PLAY_HEALING				},
	{	"HolyBolt"				,	SKILL_PLAY_HOLY_BOLT			},
	{	"MultiSpark"			,	SKILL_PLAY_MULTI_SPARK			},
	{	"HolyMind"				,	SKILL_PLAY_HOLY_MIND			},
	//2단계 세인티스
	{	"Meditation"			,	0								},
	{	"DivineLightning"		,	SKILL_PLAY_DIVINE_LIGHTNING		},
	{	"HolyReflection"		,	SKILL_PLAY_HOLY_REFLECTION		},
	{	"GrandHealing"			,	SKILL_PLAY_GREAT_HEALING		},
	//3단계 비숍
	{	"VigorBall"				,	SKILL_PLAY_VIGOR_BALL			},
	{	"Resurrection"			,	SKILL_PLAY_RESURRECTION			},
	{	"Extinction"			,	SKILL_PLAY_EXTINCTION			},
	{	"VirtualLife"			,	SKILL_PLAY_VIRTUAL_LIFE			},
	//4단계 CELESTIAL
	{	"GLACIALSPIKE"			,	SKILL_PLAY_GLACIAL_SPIKE		},
	{	"REGENERATIONFIELD"		,	SKILL_PLAY_REGENERATION_FIELD	},
	{	"CHAINLIGHTNING"		,	SKILL_PLAY_CHAIN_LIGHTNING		},
	{	"SUMMONMUSPELL"			,	SKILL_PLAY_SUMMON_MUSPELL		},


	//1단계 매지션
	{	"Agony"					,	SKILL_PLAY_AGONY				},
	{	"FireBolt"				,	SKILL_PLAY_FIRE_BOLT			},
	{	"Zenith"				,	SKILL_PLAY_ZENITH				},
	{	"FireBall"				,	SKILL_PLAY_FIRE_BALL			},
	//2단계 위자드
	{	"MentalMastery"			,	0								},
	{	"ColumnOfWater"			,	SKILL_PLAY_COLUMN_OF_WATER		},
	{	"EnchantWeapon"			,	SKILL_PLAY_ENCHANT_WEAPON		},
	{	"DeadRay"				,	SKILL_PLAY_DEAD_RAY				},
	//3단계 로얄위저드
	{	"EnergyShield"			,	SKILL_PLAY_ENERGY_SHIELD		},
	{	"Diastrophism"			,	SKILL_PLAY_DIASTROPHISM			},
	{	"SpiritElemental"		,	SKILL_PLAY_SPIRIT_ELEMENTAL		},
	{	"DancingSword"			,	SKILL_PLAY_DANCING_SWORD		},
	//4단계 아크메이지
	{	"FIREELEMENTAL"			,	SKILL_PLAY_FIRE_ELEMENTAL		},
	{	"FLAMEWAVE"				,	SKILL_PLAY_FLAME_WAVE			},
	{	"DISTORTION"			,	SKILL_PLAY_DISTORTION			},
	{	"METEO"					,	SKILL_PLAY_METEO				},

	// tier 5, as the class motion tables (M?Bip.in) name them
	{	"DownHit"				,	SKILL_PLAY_DOWN_HIT				},
	{	"PowerDash"				,	SKILL_PLAY_POWER_DASH			},
	{	"MotalBlow"				,	SKILL_PLAY_MORTAL_BLOW			},
	{	"BloodyBerserker"		,	SKILL_PLAY_BLOODY_BERSERKER		},
	{	"Landmining"			,	SKILL_PLAY_LAND_MINING			},
	{	"HyperSonic"			,	SKILL_PLAY_HYPER_SONIC			},
	{	"RollingSmash"			,	SKILL_PLAY_ROLLING_SMASH		},
	{	"PowerEnhance"			,	SKILL_PLAY_POWER_ENHANCEMENT	},
	{	"DancingReaper"			,	SKILL_PLAY_DANCING_REAPER		},
	{	"FinalSpear"			,	SKILL_PLAY_FINAL_SPEAR			},
	{	"Amplified"				,	SKILL_PLAY_AMPLIFIED			},
	{	"SideSpinAttack"		,	SKILL_PLAY_SIDE_SPIN_ATTACK		},
	{	"EvadeShot"				,	SKILL_PLAY_EVADE_SHOT			},
	{	"ThunderRoop"			,	SKILL_PLAY_THUNDER_ROPE			},
	{	"NatureSplash"			,	SKILL_PLAY_NATURE_SPLASH		},
	{	"CircleTrap"			,	SKILL_PLAY_CIRCLE_TRAP			},
	{	"SoulBreaker"			,	SKILL_PLAY_SOUL_BREAKER			},
	{	"CrescentMoon"			,	SKILL_PLAY_CRESCENT_MOON		},
	{	"SaintBlade"			,	SKILL_PLAY_SAINT_BLADE			},
	{	"HolyBenedic"			,	SKILL_PLAY_HOLY_BENEDIC			},
	{	"Talaria"				,	SKILL_PLAY_TALARIA				},
	{	"GalaxyCoup"			,	SKILL_PLAY_GALAXY_COUP			},
	{	"SummonArcuda"			,	SKILL_PLAY_SUMMON_ARCUDA		},
	{	"SnippyFear"			,	SKILL_PLAY_SNIPPY_FEAR			},
	{	"SpiritImpact"			,	SKILL_PLAY_SPIRIT_IMPACT		},
	{	"PiercingIce"			,	SKILL_PLAY_PIERCING_ICE			},
	{	"SummonRamiel"			,	SKILL_PLAY_SUMMON_RAMIEL		},
	{	"BlessingKrishna"		,	SKILL_PLAY_BLESSING_KRISHNA		},
	{	"Silraphim"				,	SKILL_PLAY_SILRAPHIM			},
	{	"VisTenus"				,	SKILL_PLAY_VIS_TENUS			},
	{	"Primaignis"			,	SKILL_PLAY_PRIMA_IGNIS			},
	{	"ThirdAnima"			,	SKILL_PLAY_THIRD_ANIMA			},
#include "sinbaram\\NewClassSkill_Clips.h"

	{	"",0 }
};



//######################################################################################
//작 성 자 : 오 영 석

// 꼬아서리.!!! 
void ModelKeyWordEncode( DWORD dwFileCode, smMODELINFO *lpMODELINFO )
{
	char *szFile;
	int   dwCode_1, dwCode_2, dwCode_3, dwCode_4;

	if( lpMODELINFO->szLinkFile[0] )
	{
		szFile = ChangeFileExt( lpMODELINFO->szLinkFile, szFileExt );
		DWORD dwLinkCode = GetSpeedSum( szFile );

		dwCode_1 = ( (dwFileCode & 0x00ff0000) << 8 );
		dwCode_2 = ( (dwLinkCode & 0x000000ff) << 16 );
		dwCode_3 = ( (dwFileCode & 0x000000ff) << 8 );
		dwCode_4 = ( (dwLinkCode & 0xff000000) >> 24 );
		lpMODELINFO->FileTypeKeyWord = ( dwCode_1 | dwCode_2 | dwCode_3 | dwCode_4 ); 

		dwCode_1 = ( (dwFileCode & 0xff000000) );
		dwCode_2 = ( (dwLinkCode & 0x0000ff00) << 8 );
		dwCode_3 = ( (dwFileCode & 0x0000ff00) );
		dwCode_4 = ( (dwLinkCode & 0x00ff0000) >> 16 );
		lpMODELINFO->LinkFileKeyWord = ( dwCode_1 | dwCode_2 | dwCode_3 | dwCode_4 ); 
	}
	else
	{
		dwCode_1 = ( (dwFileCode & 0x0000ff00) << 16 );
		dwCode_2 = ( (dwFileCode & 0xff000000) >> 8 );
		dwCode_3 = ( (dwFileCode & 0x000000ff) << 8 );
		dwCode_4 = ( (dwFileCode & 0x00ff0000) >> 16 );
		lpMODELINFO->FileTypeKeyWord = ( dwCode_1 | dwCode_2 | dwCode_3 | dwCode_4 ); 
	}
}

// 풀어서.!!! 
int ModelKeyWordDecode( DWORD dwFileCode, smMODELINFO *lpMODELINFO )
{
	char *szFile;
	int   dwCode_1, dwCode_2, dwCode_3, dwCode_4;

	if( lpMODELINFO->szLinkFile[0] )
	{
		szFile = ChangeFileExt( lpMODELINFO->szLinkFile, szFileExt );
		DWORD dwLinkCode = GetSpeedSum( szFile );

		dwCode_1 = ( (lpMODELINFO->LinkFileKeyWord & 0xff000000) );
		dwCode_2 = ( (lpMODELINFO->FileTypeKeyWord & 0xff000000) >> 8 );
		dwCode_3 = ( (lpMODELINFO->LinkFileKeyWord & 0x0000ff00) );
		dwCode_4 = ( (lpMODELINFO->FileTypeKeyWord & 0x0000ff00) >> 8 );
		if( dwFileCode != (dwCode_1 | dwCode_2 | dwCode_3 | dwCode_4) )
			return FALSE;

		dwCode_1 = ( (lpMODELINFO->FileTypeKeyWord & 0x000000ff) << 24 );
		dwCode_2 = ( (lpMODELINFO->LinkFileKeyWord & 0x000000ff) << 16 );
		dwCode_3 = ( (lpMODELINFO->LinkFileKeyWord & 0x00ff0000) >> 8  );
		dwCode_4 = ( (lpMODELINFO->FileTypeKeyWord & 0x00ff0000) >> 16 );
		if( dwLinkCode != (dwCode_1 | dwCode_2 | dwCode_3 | dwCode_4) )
			return FALSE;
	}
	else
	{
		dwCode_1 = ( (lpMODELINFO->FileTypeKeyWord & 0xff000000) >> 16 );
		dwCode_2 = ( (lpMODELINFO->FileTypeKeyWord & 0x00ff0000) << 8 );
		dwCode_3 = ( (lpMODELINFO->FileTypeKeyWord & 0x0000ff00) >> 8 );
		dwCode_4 = ( (lpMODELINFO->FileTypeKeyWord & 0x000000ff) << 16 );
		if( dwFileCode != (dwCode_2 | dwCode_4 | dwCode_1 | dwCode_3) )
			return FALSE;
	}

	return TRUE;
}

// 꼬아서리.!!! 
void MotionKeyWordEncode( DWORD dwCode, smMODELINFO *lpMODELINFO )
{
	int cnt, i, stFrm, endFrm;
	smMOTIONINFO *lpMotionInfo;

	//
	cnt = lpMODELINFO->MotionCount;
	for( i=CHRMOTION_EXT; i < cnt; i++ )
	{
		lpMotionInfo = &lpMODELINFO->MotionInfo[i];
		if( lpMotionInfo )
		{
			stFrm  = lpMotionInfo->StartFrame;
			endFrm = lpMotionInfo->EndFrame;
			
			//
			if( stFrm )
			{
				lpMotionInfo->MotionKeyWord_1 = (dwCode & 0xff000000) | ( (stFrm & 0x0000ff00)  << 8 ) |
												( (dwCode & 0x00ff0000) >> 8 ) | (stFrm & 0x000000ff);

				lpMotionInfo->StartFrame	  = ( (dwCode & 0x000000ff) << 24 )  | (stFrm & 0x00ff0000) | 
												(dwCode & 0x0000ff00) | ( (stFrm & 0xff000000) >> 24 );
			}

			//
			if( endFrm )
			{
				lpMotionInfo->MotionKeyWord_2 = ( (dwCode & 0x000000ff) << 24 ) | ( (endFrm & 0xff000000) >> 8 ) | 
												(dwCode & 0x0000ff00) | ( (endFrm & 0x00ff0000) >> 16 );

				lpMotionInfo->EndFrame		  = ( (dwCode & 0x00ff0000) << 8 ) | ( (endFrm & 0x0000ff00) << 8 ) | 
												( (dwCode & 0xff000000) >> 16 ) | (endFrm & 0x000000ff);
			}
		}
	}
}

// 풀어서.!!! 
int MotionKeyWordDecode( DWORD dwCode, smMODELINFO *lpMODELINFO )
{
	int cnt, i, KeyWord, Frame;
	smMOTIONINFO *lpMotionInfo;

	//
	cnt = lpMODELINFO->MotionCount;
	for( i=CHRMOTION_EXT; i < cnt; i++ )
	{
		lpMotionInfo = &lpMODELINFO->MotionInfo[i];
		if( lpMotionInfo )
		{
			//
			if( lpMotionInfo->MotionKeyWord_1 || lpMotionInfo->StartFrame )
			{
				KeyWord = (lpMotionInfo->MotionKeyWord_1 & 0xff000000) | ( (lpMotionInfo->MotionKeyWord_1 & 0x0000ff00) << 8 ) |
						  (lpMotionInfo->StartFrame & 0x0000ff00) | ( (lpMotionInfo->StartFrame & 0xff000000) >> 24 );

				Frame   = ( (lpMotionInfo->StartFrame & 0x000000ff) << 24 ) | (lpMotionInfo->StartFrame & 0x00ff0000) |
						  ( (lpMotionInfo->MotionKeyWord_1 & 0x00ff0000) >> 8 ) | (lpMotionInfo->MotionKeyWord_1 & 0x000000ff);

				if( dwCode != KeyWord )
					return FALSE;

				lpMotionInfo->MotionKeyWord_1 = 0;
				lpMotionInfo->StartFrame      = Frame;
			}

			//
			if( lpMotionInfo->MotionKeyWord_2 || lpMotionInfo->EndFrame )
			{
				KeyWord = ( (lpMotionInfo->EndFrame & 0x0000ff00) << 16 ) | ( (lpMotionInfo->EndFrame & 0xff000000) >> 8 ) |
						  (lpMotionInfo->MotionKeyWord_2 & 0x0000ff00) | ( (lpMotionInfo->MotionKeyWord_2 & 0xff000000) >> 24 );

				Frame   = ( (lpMotionInfo->MotionKeyWord_2 & 0x00ff0000) << 8 ) | ( (lpMotionInfo->MotionKeyWord_2 & 0x000000ff) << 16 )|
						  ( (lpMotionInfo->EndFrame & 0x00ff0000) >> 8 ) | (lpMotionInfo->EndFrame & 0x000000ff);

				if( dwCode != KeyWord )
					return FALSE;

				lpMotionInfo->MotionKeyWord_2 = 0;
				lpMotionInfo->EndFrame        = Frame;
			}
		}
	}

	return TRUE;
}

//######################################################################################
//SOD벨라트라 몬스터 설정 불러오기
int	Open_SODConfig( char *szFileName , void *lphCoreMonster , int *SodTax , char *szSodText , int *SodScoreDiv )
{

	FILE	*fp;
	char strBuff[256];
	char *p;
	char *pb;
	int NextFlag;
	int cnt;
	int hcRoundCount = 0;

	int	m_SodTax = 0;
	int m_SodScoreDiv = 0;
	char m_szSodText[256];
	sHCORE_MONSTER	hcMonster[hCORE_ROUND_MAX];
	sHCORE_MONSTER	*lpSod_Monster = (sHCORE_MONSTER *)lphCoreMonster;

	m_szSodText[0] = 0;
	memcpy( hcMonster , lpSod_Monster , sizeof(sHCORE_MONSTER)*hCORE_ROUND_MAX );

	for(cnt=0;cnt<rsHARDCORE_COUNT_END;cnt++) {

		hcMonster[cnt].hCoreMonster1.szName[0] = 0;
		hcMonster[cnt].hCoreMonster1.lpCharInfo = 0;
		hcMonster[cnt].hCoreMonster1.perCount = 0;

		hcMonster[cnt].hCoreMonster2.szName[0] = 0;
		hcMonster[cnt].hCoreMonster2.lpCharInfo = 0;
		hcMonster[cnt].hCoreMonster2.perCount = 0;

		hcMonster[cnt].hCoreMonster3.szName[0] = 0;
		hcMonster[cnt].hCoreMonster3.lpCharInfo = 0;
		hcMonster[cnt].hCoreMonster3.perCount = 0;

		hcMonster[cnt].hCoreMonster4.szName[0] = 0;
		hcMonster[cnt].hCoreMonster4.lpCharInfo = 0;
		hcMonster[cnt].hCoreMonster4.perCount = 0;

		hcMonster[cnt].hCoreMonsterBoss.szName[0] = 0;
		hcMonster[cnt].hCoreMonsterBoss.lpCharInfo = 0;
		hcMonster[cnt].hCoreMonsterBoss.perCount = 0;

		hcMonster[cnt].MaxMonster = 0;
	}


	fp = fopen( szFileName , "rb" );
	if ( fp==NULL ) return FALSE;


	while( !feof( fp ) )//  feof: file end까지 읽어라 
	{
		if( fgets( line, 255, fp ) == NULL)	break;

		p = fileread_GetWord( decode , line);

		NextFlag = 0;

		///////////////// 세율 ///////////////////////
		if ( lstrcmpi( decode , "*BELATRA_TAX" )==0 ) {

			pb=p;p=fileread_GetWord(strBuff,p);if(strBuff[0]==34)p=GetString(strBuff,pb);
			if ( strBuff[0] ) m_SodTax = atoi( strBuff );

			NextFlag = TRUE;
		}


		///////////////// 설명 ///////////////////////
		if ( !NextFlag && lstrcmpi( decode , "*BELATRA_TEXT" )==0 ) {

			pb=p;p=fileread_GetWord(strBuff,p);if(strBuff[0]==34)p=GetString(strBuff,pb);
			lstrcpy( m_szSodText , strBuff );

			NextFlag = TRUE;
		}

		///////////////// 설명 ///////////////////////
		if ( !NextFlag && lstrcmpi( decode , "*BELATRA_DIVSCORE" )==0 ) {

			pb=p;p=fileread_GetWord(strBuff,p);if(strBuff[0]==34)p=GetString(strBuff,pb);
			if ( strBuff[0] ) m_SodScoreDiv = atoi( strBuff );

			NextFlag = TRUE;
		}


		///////////////// 몬스터 ///////////////////////
		if ( !NextFlag && lstrcmpi( decode , "*BELATRA_MONSTER" )==0 ) {
			if ( hcRoundCount<rsHARDCORE_COUNT_END ) {
				pb=p;p=fileread_GetWord(strBuff,p);if(strBuff[0]==34)p=GetString(strBuff,pb);		//1st 몬스터
				lstrcpy( hcMonster[hcRoundCount].hCoreMonster1.szName , strBuff );
				p=fileread_GetWord(strBuff,p);
				hcMonster[hcRoundCount].hCoreMonster1.perCount = atoi(strBuff);

				pb=p;p=fileread_GetWord(strBuff,p);if(strBuff[0]==34)p=GetString(strBuff,pb);		//2nd 몬스터
				lstrcpy( hcMonster[hcRoundCount].hCoreMonster2.szName , strBuff );
				p=fileread_GetWord(strBuff,p);
				hcMonster[hcRoundCount].hCoreMonster2.perCount = atoi(strBuff);

				pb=p;p=fileread_GetWord(strBuff,p);if(strBuff[0]==34)p=GetString(strBuff,pb);		//3rd 몬스터
				lstrcpy( hcMonster[hcRoundCount].hCoreMonster3.szName , strBuff );
				p=fileread_GetWord(strBuff,p);
				hcMonster[hcRoundCount].hCoreMonster3.perCount = atoi(strBuff);

				pb=p;p=fileread_GetWord(strBuff,p);if(strBuff[0]==34)p=GetString(strBuff,pb);		//4th 몬스터
				lstrcpy( hcMonster[hcRoundCount].hCoreMonster4.szName , strBuff );
				p=fileread_GetWord(strBuff,p);
				hcMonster[hcRoundCount].hCoreMonster4.perCount = atoi(strBuff);

				pb=p;p=fileread_GetWord(strBuff,p);if(strBuff[0]==34)p=GetString(strBuff,pb);		//보스 몬스터
				lstrcpy( hcMonster[hcRoundCount].hCoreMonsterBoss.szName , strBuff );
				hcMonster[hcRoundCount].hCoreMonsterBoss.perCount = 0;

				p=fileread_GetWord(strBuff,p);														//출현제한수
				hcMonster[hcRoundCount].MaxMonster = atoi(strBuff);

				hcRoundCount ++;
			}
			NextFlag = TRUE;
		}
	}
	fclose(fp);

	if ( hcRoundCount<rsHARDCORE_COUNT_END ) return 0;

	if ( m_SodTax>=0 && m_SodTax<100 ) 	*SodTax = m_SodTax;
	if ( m_szSodText[0] ) lstrcpy( szSodText , m_szSodText );
	if ( m_SodScoreDiv>0 ) 	*SodScoreDiv = m_SodScoreDiv;

	memcpy( lpSod_Monster , hcMonster , sizeof(sHCORE_MONSTER)*hCORE_ROUND_MAX );

	return 1;		//나중에 갯수 추가된다면 숫자가 그룹만큼 리턴
}




//크랙정보파일을 읽어 온다
int ReadCustomCracker( char *szCrackFile , TRANS_FIND_CRACKER *lpTransFindCrack  )
{
	FILE	*fp;
	char strBuff[64];
	char *p,*pb;
	int cnt;

	//fp = fopen( "FindCrack.ini" , "rb" );
	fp = fopen( szCrackFile , "rb" );
	if ( fp==NULL ) {
		return FALSE;					//레지스트리 설정 적용
	}

	cnt = 0;

	while( !feof( fp ) )//  feof: file end까지 읽어라 
	{

		if( fgets( line, 255, fp ) == NULL)	break;

		p = fileread_GetWord( decode , line);


		if ( lstrcmpi( decode , "*CRACK" )==0 ) {


			pb=p;p=fileread_GetWord(strBuff,p);if(strBuff[0]==34)p=GetString(strBuff,pb);
			lstrcpy( lpTransFindCrack->CrackName[cnt].szName1 , strBuff );

			pb=p;p=fileread_GetWord(strBuff,p);if(strBuff[0]==34)p=GetString(strBuff,pb);
			lstrcpy( lpTransFindCrack->CrackName[cnt].szName2 , strBuff );

			pb=p;p=fileread_GetWord(strBuff,p);if(strBuff[0]==34)p=GetString(strBuff,pb);
			lstrcpy( lpTransFindCrack->CrackName[cnt].szClass , strBuff );

			cnt++;
		}
		if ( cnt>=FIND_CRACK_LIST_MAX ) break;

	}

	lpTransFindCrack->code = smTRANSCODE_FINDCRACK;
	lpTransFindCrack->size = sizeof(_CRACK_NAME)*cnt+16;
	lpTransFindCrack->FindCounter = cnt;

	fclose(fp);

	return cnt;
}


