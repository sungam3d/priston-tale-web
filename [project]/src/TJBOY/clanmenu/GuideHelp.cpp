//---------------------------------------------------------------------------
// GuideHelp.cpp : 캐릭터가 레벨업이 되었을때 해당레벨에 맞는 가이드를 자동
//                 으로 보여주는 오토가이드의 구현 부분이다.
//
// latest modify : 2004. 03. 04 by teze(Tae-Jin Kim.)
// Original Developer : Gwang-Yul Yoo
//
//
// (주)프리스톤 소스코드 제한 사항
//
// 이 소스 코드는 (주)트라이글로우픽쳐스 내부에서만 사용이 유효합니다. 
// (주)트라이글로우픽쳐스 외에서 사용할 경우 그에 합당한 법적 조치가
// 취해질수 있습니다. 
//
// 프리스톤 테일에 관한 모든 권리는 (주)프리스톤 에 있습니다.
//
//
//      Copyright (c) 2003 PRISTON.
//      All Rights Reserved.
//
// (주)PRISTON Seoul South. Korea
// http://kr.pristontale.com
//
// teze에 의해 작성된 주석은 명확한 확인을 거친 것은 아니다. 그리고 나름대로
// 개선해야할 방안에 대해 조금씩 적어 두었다. teze가 작성한 주선은 by teze라
// 는 키워드를 달아 두었다. 언제든 삭제해도 된다.
//---------------------------------------------------------------------------

// extern 클래스의 선언을 위한 정의다. 존재하지 않을 경우 이클래스는 extern으로 선언된다.by teze
#define GUIDEHELP_CPP

//public headers
#include <windows.h>
#include <tchar.h>
#include <math.h>
#include <stdio.h>


//custom headers
#include "tjclanDEF.h"

#ifdef USE_PROSTONTALE		//=========== 프리스턴소스인경우
	#include "..\\..\\sinbaram\\SinLinkHeader.h"
	#include "..\\..\\HoBaram\\HoLinkHeader.h"
	#include "..\\..\\language.h"
	#include "..\\..\\HoBaram\\HoTextFile.h"
	#include "cE_CViewClanInfo.h"
#endif

#include "tjscroll.h"
#include "tjclan.h"
#include "cE_Notice.h"
#include "GuideHelp.h"

//======================================================================================================
// Static Variabel declare
// : 이런 부분들은 맴버 속성으로의 변환이 필요하다 by teze

extern int GuideHelpButton[3]; //가이드 도움말
extern BOOL IsDrawGuideHelp;

//int g_LevelLabel[]  0 1 2 3 4 5 6 7 8 9  10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27
int g_LevelLabel[] = {1,2,3,4,5,6,7,8,9,10,11,12,15,17,20,22,25,30,39,40,50,55,60,65,70,75,79,80};
int g_MorianLabel[]= {1,2,3,4,5,6,7,8,9,10,11,12,15,17,20,22,25,30,39,40,50,55,60,65,70,75,79,80};
//int g_LevelLabel[] = {1,2,3,4,5,6,7,8,9,10,12,15,17,20,22,25,30,39,40,50,55,60,65,70,80};//temp comment
//int g_MorianLabel[]= {1,2,3,4,5,6,7,8,9,10,12,15,17,20,22,25,30,39,40,50,55,60,65,70,80};//temp comment

//int nStrLen[] = {19,11,10,6,12,8,11,6,12,24,18,24,13,33,22,12,12,19,12,16,14,14};


//가이드의실제 내용이 출력될 메뉴의 크기를 결정하기 위해서 한라인의 최종 크기를 결정하기위한 상수인듯
//근데 실제 가이드 갯수랑 차이가 나는데 실제로 왜 쓰는지는 모르겠음. 하나의 코드부분에서 씀(Line : 883)
//by teze

//StrLenth Index   0   1  2  3   4  5   6  7   8   9  10  11  12  13  14  15  16  17  18  19  20  21
int nStrLen[] = {178,102,92,56,108,72,102,56,204,216,168,216,118,246,196,112,224,174,112,148,128,128};

POINT ScrollSavePoint;

extern BOOL bKeyDown;
extern int nKeyDown;
extern BOOL bKeyUp;
extern int nKeyUp;			
extern BOOL bLButtonDown;

// end of declare
//=======================================================================================================


CGuideHelp::CGuideHelp()
{
	m_GuideHelpN = -1;
	nQuetion_Icon =0,nQuetion_Icon1 =0;
	nHelp = nHelpSee = nQuideHelp = nDuru = nDuru1 =0;
	hOff = NULL;
	hOn = NULL;
	bIsOn = TRUE;
	nLevelHelpCount =0;
	ZeroMemory(LevelHelp,sizeof(LevelHelp));	
	ZeroMemory(bHelpIcon,sizeof(bHelpIcon));
	bDoing = FALSE;
	nHelpCountNamugi = 0;

	GuideImage = new smMATERIAL_GROUP(1);
	nGuideImageNum = 0;
	
}

CGuideHelp::~CGuideHelp()
{
	delete GuideImage;
	Clear();
}

void CGuideHelp::ClearImage()
{
	GuideImage->Close();
}

void CGuideHelp::LoadGuideImage(char* file)
{
	ClearImage();
	GuideImage->Init(1);
	nGuideImageNum = GuideImage->CreateTextureMaterial(file,0, 0, 128,128, SMMAT_BLEND_ALPHA );
	GuideImage->ReadTextures();

}

int CGuideHelp::Init(int Life)
{
	life = Life;
	
	return 1;
}

int CGuideHelp::Clear()
{
	if(hOff) 
	{
		hOff->Release();
		hOff = NULL;
	}
	if(hOn) 
	{
		hOn->Release();
		hOn = NULL;
	}
	return 1;
}


int nPos=0;
void CGuideHelp::Main()
{
	int i;
	
	if(!bDoing) return;
	switch(m_GuideHelpN)
	{
	case GUIDEHELP_LIST:
		i = chkeckMenuN();
		switch(i) {
		case 0:				
			menuMouseOver[0] = 1;
			break;
		case 2:
			menuMouseOver[2] = 1;
			break;
		case 5:
			menuMouseOver[2] = 1;
			break;
		case 12:
			case 13:
			case 14:
			case 15:
			case 16:  //5
			case 17:
			case 18:
			case 19:
			case 20:
			case 21: //10
			case 22:
			case 23:
				if(!bHelpIcon[i+nScl_Bar-12])
				{
					ZeroMemory(bHelpIcon,sizeof(bHelpIcon));
					bHelpIcon[i+nScl_Bar-12] =!bHelpIcon[i+nScl_Bar-12];
				}				
				break;
			default:
				ZeroMemory(bHelpIcon,sizeof(bHelpIcon));
				memset(menuMouseOver,-1,sizeof(int)*30);
				break;
		}
		if(TJwheel != 0) {			//ktj : wheel>0 휠 앞으로 굴리기  wheel<0 휠 뒤로 굴리기
			scrIconPos.top = tscr_Guide.ret_WHEELpoint(TJwheel , scrIconPos.top);
			nScl_Bar = tscr_Guide.topScrollLineNum(scrIconPos.top);
			TJwheel = 0;
		}
		//if(MOUSE0())			
		if(GuideHelpButton[0])
		{	
			//MOUSE0()=0;
			
			if(bKeyUp||bKeyDown)
			{
				i = 11;
			}
			switch(i) 
			{
			case 11:
				if(bKeyDown)
				{
					ScrollSavePoint.y+=10;
					if(ScrollSavePoint.y > menuPos[11].top + 246)
					{
						ScrollSavePoint.y = menuPos[11].top + 246;
					}
					nScl_Bar = tscr_Guide.topScrollLineNum(ScrollSavePoint.y);
					if(nHelpCountNamugi == nScl_Bar)
					{
						//nHelpCountNamugi = nScl_Bar;
						scrIconPos.top = menuPos[11].top + 246;
						ScrollSavePoint.y = scrIconPos.top;
					}
					else
					{
						//ktj : 스크롤 아이콘 위치재설정
						scrIconPos.top =  ScrollSavePoint.y-8;//BackStartPos.y + SCROOBAR_TOP;
					}
					GuideHelpButton[0] = 0;
					break;
				}
				if(bKeyUp)
				{
					ScrollSavePoint.y-=10; 
					if(ScrollSavePoint.y < menuPos[11].top)
					{
						ScrollSavePoint.y = menuPos[11].top;
					}
					nScl_Bar = tscr_Guide.topScrollLineNum(ScrollSavePoint.y);
					//ktj : 스크롤 아이콘 위치재설정
					scrIconPos.top =  ScrollSavePoint.y-8;//BackStartPos.y + SCROOBAR_TOP;			
					GuideHelpButton[0] = 0;
					
					break;
				}
				bLButtonDown = TRUE;
				
				ScrollSavePoint = pCursorPos;				
				nScl_Bar = tscr_Guide.topScrollLineNum(pCursorPos.y);
				
				if(nHelpCountNamugi == nScl_Bar)
				{
					scrIconPos.top = menuPos[11].top + 246;
					ScrollSavePoint.y = scrIconPos.top;
				}
				if(pCursorPos.y > menuPos[11].top + 246)
				{
					scrIconPos.top = menuPos[11].top + 246;
				}
				else if(pCursorPos.y < menuPos[11].top)
				{
					scrIconPos.top = menuPos[11].top;
				}
				else
				{
					//ktj : 스크롤 아이콘 위치재설정
					scrIconPos.top =  pCursorPos.y;//BackStartPos.y + SCROOBAR_TOP;				
				}
				
				break;
			
			case 0: //확인
				IsDrawGuideHelp = FALSE;
				m_GuideHelpN = -1;
				bDoing = FALSE;
				isDrawClanMenu =0;
				GuideHelpButton[0] = 0;
				TJBscrollWheelClear();
				TJBscrollWheelClear_Guide();
				break;
			case 5:
			case 2: //상열씨 도움말을띄워준다
				IsDrawGuideHelp = FALSE;
				m_GuideHelpN = -1;
				isDrawClanMenu =0;
				TJBscrollWheelClear();
				TJBscrollWheelClear_Guide();
				int sinShowHelp();
				sinShowHelp();
				GuideHelpButton[0] = 0;
				break;
			case 9: //레벨업 했을때 도움말 띄울것인이 유무 체크 관련 부분
				bIsOn = !bIsOn;
				GuideHelpButton[0] = 0;
				break;
			
			case 12:
			case 13:
			case 14:
			case 15:
			case 16:  //5
			case 17:
			case 18:
			case 19:
			case 20:
			case 21: //10
			case 22:
			case 23:			
				if(!bLButtonDown)
				{
					nPos = (i+nScl_Bar-12);
					if(JOB_IS_TEMPSKRON(sinChar->JOB_CODE))
					{
						CheckLevel(g_LevelLabel[nLevelHelpCount-nPos-1],sinChar->JOB_CODE);
					}
					else
					{
						//CheckLevel(g_MorianLabel[i+nScl_Bar-12],sinChar->JOB_CODE);
						CheckLevel(g_MorianLabel[nLevelHelpCount-nPos-1],sinChar->JOB_CODE);
					}

					m_GuideHelpN = GUIDEHELP_CONTENT;
					menuInit(m_GuideHelpN);
					GuideHelpButton[0] = 0;
				}
				break;
			default:
				if(bLButtonDown)
				{
					//수정 2003.10.30
					if(pCursorPos.y < menuPos[11].top)
					{
						scrIconPos.top = menuPos[11].top;
						nScl_Bar = tscr_Guide.topScrollLineNum(scrIconPos.top);
					}
					else if(pCursorPos.y > menuPos[11].top + 246)
					{
						scrIconPos.top = menuPos[11].top + 246;
						nScl_Bar = tscr_Guide.topScrollLineNum(scrIconPos.top);
					}
					else
					{
						scrIconPos.top = pCursorPos.y;
						nScl_Bar = tscr_Guide.topScrollLineNum(scrIconPos.top);
					}
					//수정전
					/*if(pCursorPos.y < menuPos[11].top)
					{
						pCursorPos.y = menuPos[11].top;
					}
					if(pCursorPos.y > menuPos[11].top + 246)
					{
						pCursorPos.y = menuPos[11].top + 246;
					}
					ScrollSavePoint = pCursorPos;
					nScl_Bar = tscr_Guide.topScrollLineNum(pCursorPos.y);
					if(nHelpCountNamugi == nScl_Bar)
					{
						scrIconPos.top = menuPos[11].top + 246;
						ScrollSavePoint.y = scrIconPos.top;
					}
					//ktj : 스크롤 아이콘 위치재설정
					scrIconPos.top =  pCursorPos.y-8;//BackStartPos.y + SCROOBAR_TOP;					
					*/
				}
				else
					GuideHelpButton[0] = 0;
				break;
			}
		}
	
		break;
	case GUIDEHELP_CONTENT:
		i = chkeckMenuN();
		switch(i) {
		case 0:				
			menuMouseOver[0] = 1;
			break;
		case 2:
			menuMouseOver[2] = 1;
			break;
		case 5:
			menuMouseOver[2] = 1;
			break;
		case 10:
		case 11:
			bHelpIcon[10] = 1;
			break;		
		default:			
			ZeroMemory(bHelpIcon,sizeof(bHelpIcon));
			memset(menuMouseOver,-1,sizeof(int)*30);
			break;
		}

		if(TJwheel != 0) {			//ktj : wheel>0 휠 앞으로 굴리기  wheel<0 휠 뒤로 굴리기
			scrIconPos.top = tscr_Guide.ret_WHEELpoint(TJwheel , scrIconPos.top);
			nScl_Bar = tscr_Guide.topScrollLineNum(scrIconPos.top);
			TJwheel = 0;
		}
		
		//if(MOUSE0())			
		if(GuideHelpButton[0])
		{	
			//MOUSE0()=0;
			//GuideHelpButton[0]=0;
			if(bKeyUp||bKeyDown)
			{
				i = 15;
			}
			switch(i) 
			{
			case 0:
				IsDrawGuideHelp = FALSE;
				m_GuideHelpN = -1;
				bDoing = FALSE;
				isDrawClanMenu = 0;
				TJBscrollWheelClear();
				TJBscrollWheelClear_Guide();
				GuideHelpButton[0] = 0;
				break;
			case 5:
			case 2: //상열씨 도움말을띄워준다
				IsDrawGuideHelp = FALSE;
				m_GuideHelpN = -1;
				isDrawClanMenu =0;
				TJBscrollWheelClear();
				TJBscrollWheelClear_Guide();
				int sinShowHelp();
				sinShowHelp();
				GuideHelpButton[0]=0;
				break;
			case 9: //레벨업 했을때 도움말 띄울것인이 유무 체크 관련 부분
				bIsOn = !bIsOn;
				GuideHelpButton[0]=0;
				break;
			case 6:
			case 7:
			case 11:
			case 10:
				SetDoing(TRUE);				
				CheckLevel(sinChar->Level);				
				SetHelpN(GUIDEHELP_LIST);
				menuInit(GUIDEHELP_LIST);
				IsDrawGuideHelp = TRUE;
				GuideHelpButton[0]=0;
				break;			
			case 15:
				if(bKeyDown)
				{
					//pCursorPos.y = scrIconPos.top;
					scrIconPos.top+=10;
					if(scrIconPos.top > menuPos[15].top + 246)
					{
						scrIconPos.top = menuPos[15].top + 246;
					}
					nScl_Bar = tscr_Guide.topScrollLineNum(scrIconPos.top);
					//scrIconPos.top = pCursorPos.y;
					/*if(nHelpCountNamugi == nScl_Bar)
					{
						//nHelpCountNamugi = nScl_Bar;
						scrIconPos.top = menuPos[11].top + 246;
						ScrollSavePoint.y = scrIconPos.top;
					}
					else
					{
						//ktj : 스크롤 아이콘 위치재설정
						scrIconPos.top =  ScrollSavePoint.y-8;//BackStartPos.y + SCROOBAR_TOP;
					}*/
					GuideHelpButton[0] = 0;
					break;
				}
				if(bKeyUp)
				{
					//ScrollSavePoint.y-=10; 
					scrIconPos.top-=10;
					if(scrIconPos.top < menuPos[15].top)
					{
						scrIconPos.top = menuPos[15].top;
					}
					nScl_Bar = tscr_Guide.topScrollLineNum(scrIconPos.top);
					//ktj : 스크롤 아이콘 위치재설정
					//scrIconPos.top =  ScrollSavePoint.y-8;//BackStartPos.y + SCROOBAR_TOP;			
					GuideHelpButton[0] = 0;
					
					break;
				}
				bLButtonDown = TRUE;
				
				//ScrollSavePoint = pCursorPos.y;				
				//nScl_Bar = tscr_Guide.topScrollLineNum(pCursorPos.y);

				if(pCursorPos.y > menuPos[15].top + 246)
				{
					scrIconPos.top = menuPos[15].top + 246; 
					nScl_Bar = tscr_Guide.topScrollLineNum(scrIconPos.top);
				}
				else if(pCursorPos.y < menuPos[15].top)
				{
					scrIconPos.top = menuPos[15].top;
					nScl_Bar = tscr_Guide.topScrollLineNum(scrIconPos.top);
				}
				else
				{
					nScl_Bar = tscr_Guide.topScrollLineNum(pCursorPos.y);
					scrIconPos.top = pCursorPos.y;
				}


				
				
				/*if(nHelpCountNamugi == nScl_Bar)
				{
					scrIconPos.top = menuPos[15].top + 246;
					ScrollSavePoint.y = scrIconPos.top;
				}
				if(pCursorPos.y > menuPos[15].top + 246)
				{
					scrIconPos.top = menuPos[15].top + 246;
				}
				else if(pCursorPos.y < menuPos[15].top)
				{
					scrIconPos.top = menuPos[15].top;
				}
				else
				{
					//ktj : 스크롤 아이콘 위치재설정
					scrIconPos.top =  pCursorPos.y;//BackStartPos.y + SCROOBAR_TOP;				
				}*/
				/*nScl_Bar = tscr_Guide.topScrollLineNum(pCursorPos.y);
				//ktj : 스크롤 아이콘 위치재설정
				if(pCursorPos.y > menuPos[15].top + 246)
				{
					scrIconPos.top = menuPos[15].top + 246;
				}
				else
					scrIconPos.top =  pCursorPos.y-8;//BackStartPos	
					*/
				
				break;
			default:
				if(bLButtonDown)
				{
					if(pCursorPos.y < menuPos[15].top)
					{
						scrIconPos.top = menuPos[15].top;
						nScl_Bar = tscr_Guide.topScrollLineNum(scrIconPos.top);
					}
					else if(pCursorPos.y > menuPos[15].top + 246)
					{
						scrIconPos.top = menuPos[15].top + 246;
						nScl_Bar = tscr_Guide.topScrollLineNum(scrIconPos.top);
					}
					else
					{
						scrIconPos.top = pCursorPos.y;
						nScl_Bar = tscr_Guide.topScrollLineNum(scrIconPos.top);
					}

					//ScrollSavePoint = pCursorPos;
					
					/*if(nHelpCountNamugi == nScl_Bar)
					{
						scrIconPos.top = menuPos[11].top + 246;
						ScrollSavePoint.y = scrIconPos.top;
					}*/
					//ktj : 스크롤 아이콘 위치재설정
					//scrIconPos.top =  pCursorPos.y-8;//BackStartPos.y + SCROOBAR_TOP;					
				}
				else
					GuideHelpButton[0] = 0;
				break;
			}
		}

		break;
	}
}



void CGuideHelp::Draw()
{
	int i;
	int MenuPos=0;
	int nCap =0;
	if(!bDoing) return;
	switch(m_GuideHelpN)
	{
	case GUIDEHELP_LIST:
		DrawSprite(menuPos[1].left,menuPos[1].top, hButton_Box,  0, 0, 48, 23, 1);		
		if(menuMouseOver[0]>0) DrawSprite(menuPos[0].left,menuPos[0].top, hOk_Yellow,  0, 0, 32, 16, 1);
		else	DrawSprite(menuPos[0].left,menuPos[0].top, hOk_Gray,  0, 0, 32, 16, 1);

		DrawSprite(menuPos[4].left,menuPos[4].top, hClanName,  0, 0, CLANNAME_WIDTH, CLANNAME_HEIGHT, 1);


		lpD3DDevice->BeginScene();
		dsDrawTexImage( nLine, menuPos[3].left,menuPos[3].top,  403, 10,       255);
		if(menuMouseOver[2]>0)
		{
			dsDrawTexImage( nQuetion_Icon, menuPos[2].left,menuPos[2].top,     32, 32,       255);
		}
		else
		{
			dsDrawTexImage( nQuetion_Icon1, menuPos[2].left,menuPos[2].top,     32, 32,       255);
		}
		dsDrawTexImage( nHelp, menuPos[5].left,menuPos[5].top,     64, 32,       255);
		dsDrawTexImage( nDuru, menuPos[6].left,menuPos[6].top,     32, 32,       255);
		dsDrawTexImage( nQuideHelp, menuPos[7].left,menuPos[7].top,     64, 32,       255);
		dsDrawTexImage( nHelpSee, menuPos[8].left,menuPos[8].top,     64, 32,       255);
		lpD3DDevice->EndScene();

//		Draw_Title_Text("가   이   드", menuPos[10].left, menuPos[10].top);
#ifdef _LANGUAGE_JAPANESE
		Draw_Title_Text("\203\113   \203\103   \203\150", menuPos[10].left, menuPos[10].top);
#else
		Draw_Title_Text("G  U  I  D  E", menuPos[10].left, menuPos[10].top);
#endif
		//Draw_Title_Text("가이드 도 움 말", menuPos[10].left + 51-5, menuPos[10].top + 11);
		//Draw_C_Text("가이드 도 움 말", menuPos[10].left + 50, menuPos[10].top + 11);
		//Draw_C_Text("가이드 도 움 말", menuPos[10].left + 51, menuPos[10].top + 11);

		MenuPos = 12;		
		for(i=nScl_Bar; i<nScl_Bar+LEVELHELP_COUNT_MAX; i++)
		{
			//if(i >= LEVELHELP_COUNT_MAX) break;
 			if(i >= nLevelHelpCount) break;
			menuPos[MenuPos].right = menuPos[MenuPos].left + 300;//(6*lstrlen(LevelHelp[i]) + 30);
			lpD3DDevice->BeginScene();
			if(bHelpIcon[i])
			{
				
				sinDrawTexImage( nDuru, (float)menuPos[MenuPos].left, (float)menuPos[MenuPos].top , 24, 24);
			}
			else
			{
				sinDrawTexImage( nDuru1, (float)menuPos[MenuPos].left, (float)menuPos[MenuPos].top , 24, 24);
				
			}
			lpD3DDevice->EndScene();
			if(bHelpIcon[i])
			{
				//
				//Draw_C_Text(LevelHelp[nLevelHelpCount-i-1], menuPos[MenuPos].left+30, menuPos[MenuPos].top+5);
				Draw_Help_Text2(LevelHelp[nLevelHelpCount-i-1], menuPos[MenuPos].left+30, menuPos[MenuPos].top+5);
				
			}
			else
			{
				//Draw_C_Text(LevelHelp[nLevelHelpCount-i-1], menuPos[MenuPos].left+30, menuPos[MenuPos].top+5);
				Draw_Help_Text(LevelHelp[nLevelHelpCount-i-1], menuPos[MenuPos].left+30, menuPos[MenuPos].top+5);
			}
			MenuPos++;			
		}

		MenuPos = 11;

		if(nLevelHelpCount> LEVELHELP_COUNT_MAX)
		{
			lpD3DDevice->BeginScene();			
			sinDrawTexImage(nScl_Bar_tj,  (float)menuPos[MenuPos].left, (float)menuPos[MenuPos].top, 8, 256);
			sinDrawTexImage( nScl_icon_tj , (float)scrIconPos.left, (float)scrIconPos.top, 16, 16);
			lpD3DDevice->EndScene();
		}

		

		if(bIsOn)
		{
			DrawSprite(menuPos[9].left,menuPos[9].top, hOn,  0, 0, 36, 24, 1);
		}
		else
		{
			DrawSprite(menuPos[9].left,menuPos[9].top, hOff,  0, 0, 36, 24, 1);
		}
		
		break;
	case GUIDEHELP_CONTENT:
 		DrawSprite(menuPos[1].left,menuPos[1].top, hButton_Box,  0, 0, 48, 23, 1);		
		if(menuMouseOver[0]>0) DrawSprite(menuPos[0].left,menuPos[0].top, hOk_Yellow,  0, 0, 32, 16, 1);
		else	DrawSprite(menuPos[0].left,menuPos[0].top, hOk_Gray,  0, 0, 32, 16, 1);

		DrawSprite(menuPos[4].left,menuPos[4].top, hClanName,  0, 0, CLANNAME_WIDTH, CLANNAME_HEIGHT, 1);


		lpD3DDevice->BeginScene();
		dsDrawTexImage( nLine, menuPos[3].left,menuPos[3].top,  403, 10,       255);
		if(menuMouseOver[2]>0)
		{
			dsDrawTexImage( nQuetion_Icon, menuPos[2].left,menuPos[2].top,     32, 32,       255);
		}
		else
		{
			dsDrawTexImage( nQuetion_Icon1, menuPos[2].left,menuPos[2].top,     32, 32,       255);
		}

		dsDrawTexImage( nHelp, menuPos[5].left,menuPos[5].top,     64, 32,       255);
		dsDrawTexImage( nDuru, menuPos[6].left,menuPos[6].top,     32, 32,       255);
		dsDrawTexImage( nQuideHelp, menuPos[7].left,menuPos[7].top,     64, 32,       255);
		dsDrawTexImage( nHelpSee, menuPos[8].left,menuPos[8].top,     64, 32,       255);
		lpD3DDevice->EndScene();

//		Draw_Title_Text("가   이   드", menuPos[12].left, menuPos[12].top);
#ifdef _LANGUAGE_JAPANESE
		Draw_Title_Text("\203\113   \203\103   \203\150", menuPos[12].left, menuPos[12].top);
#else
		Draw_Title_Text("G  U  I  D  E", menuPos[12].left, menuPos[12].top);
#endif
		//Draw_C_Text("가이드 도 움 말", menuPos[12].left + 51-5, menuPos[12].top + 11);

		lpD3DDevice->BeginScene();
		//전 단계로 넘겨주는 이미지 부분
		if(bHelpIcon[10])
		{
			sinDrawTexImage( nDuru, (float)menuPos[10].left, (float)menuPos[10].top , 24, 24);
		}
		else
		{
			sinDrawTexImage( nDuru1, (float)menuPos[10].left, (float)menuPos[10].top , 24, 24);
			
		}		
		//전 단계로 넘겨주는 글씨 부분
		dsDrawTexImage( nGuideList, menuPos[11].left,menuPos[11].top,     64, 32,       255);
		
		//내용안에 들어갈 이미지
		if((g_LevelLabel[nLevelHelpCount-nPos-1]!= 9) && (g_LevelLabel[nLevelHelpCount-nPos-1]!= 39))
		{
			dsDrawTexImageFloat( nGuideImageNum, (float)menuPos[14].left,(float)menuPos[14].top,     128, 128,       255);
		}
		//dsDrawTexImage( nGuideImageNum, menuPos[14].left,menuPos[14].top,     128, 128,       255);
		

		lpD3DDevice->EndScene();

		if(bIsOn)
		{
			DrawSprite(menuPos[9].left,menuPos[9].top, hOn,  0, 0, 36, 24, 1);
		}
		else
		{
			DrawSprite(menuPos[9].left,menuPos[9].top, hOff,  0, 0, 36, 24, 1);
		}

		MenuPos = 15;
		if(nHeight_Count > GUIDEHLEP_CONTENT_HEIGHT_MAX)
		{
			lpD3DDevice->BeginScene();			
			sinDrawTexImage(nScl_Bar_tj,  (float)menuPos[MenuPos].left, (float)menuPos[MenuPos].top, 8, 256);
			sinDrawTexImage( nScl_icon_tj , (float)scrIconPos.left, (float)scrIconPos.top, 16, 16);
			lpD3DDevice->EndScene();
		}

		//nCap = 0;
		MenuPos = 16;
		for(i=nScl_Bar; i<nScl_Bar + GUIDEHLEP_CONTENT_HEIGHT_MAX; i++)
		{
			if(i >=nHeight_Count) break; 
			if(nHeight_Count >  GUIDEHLEP_CONTENT_HEIGHT_MAX)
				Draw_C_Text(m_HelpContent[i], menuPos[MenuPos].left, menuPos[MenuPos].top);
			else
				Draw_C_Text(m_HelpContent[i], menuPos[MenuPos].left + 20, menuPos[MenuPos].top);
			MenuPos++;
		}

		break;
	}
}



void CGuideHelp::Load()
{
	nQuetion_Icon = CreateTextureMaterial( "image\\Sinimage\\Help\\help_icon01.tga", 0, 0, 64,64, SMMAT_BLEND_ALPHA );
	nQuetion_Icon1 = CreateTextureMaterial( "image\\Sinimage\\Help\\help_icon01_.tga", 0, 0, 64,64, SMMAT_BLEND_ALPHA );
	nHelp = CreateTextureMaterial( "image\\Sinimage\\Help\\help_icon01t.tga", 0, 0, 64,64, SMMAT_BLEND_ALPHA );
	nHelpSee = CreateTextureMaterial( "image\\Sinimage\\Help\\help_icon00.tga", 0, 0, 64,64, SMMAT_BLEND_ALPHA );
	nQuideHelp = CreateTextureMaterial( "image\\Sinimage\\Help\\help_icon02t.tga", 0, 0, 64,64, SMMAT_BLEND_ALPHA );
	nDuru = CreateTextureMaterial( "image\\Sinimage\\Help\\help_icon02.tga", 0, 0, 64,64, SMMAT_BLEND_ALPHA );
	nDuru1 = CreateTextureMaterial( "image\\Sinimage\\Help\\help_icon02_.tga", 0, 0, 64,64, SMMAT_BLEND_ALPHA );
	nLine = CreateTextureMaterial( "image\\Sinimage\\Help\\Box_Line.tga", 0, 0, 0,0, SMMAT_BLEND_ALPHA );

	hOn = LoadDibSurfaceOffscreen("image\\Sinimage\\Help\\G_help-On.bmp");
	hOff = LoadDibSurfaceOffscreen("image\\Sinimage\\Help\\G_help-Off.bmp");

	nScl_icon_tj = CreateTextureMaterial( "image\\Sinimage\\Help\\scl_icon.tga", 0, 0, 0,0, SMMAT_BLEND_ALPHA );
	
	nScl_Bar_tj= CreateTextureMaterial( "image\\Sinimage\\Help\\scl_bar.tga", 0, 0, 0,0, SMMAT_BLEND_ALPHA );

	nGuideList = CreateTextureMaterial( "image\\Sinimage\\Help\\help_Glist.tga", 0, 0, 64,32, SMMAT_BLEND_ALPHA );


}




void CGuideHelp::menuInit(int num)
{
	int nScrollPos = 0;
	int i =0;
	int nCap =0;

	

	switch(num)
	{
		case GUIDEHELP_LIST: //레벨업 관련 도움말 목차
	 	menuSu  = 25;					
		
 		menuPos[1].left   = 392;//NOTICE_X + 185 ;		//박스
		menuPos[1].top    = 411;//NOTICE_Y + 381;
		menuPos[1].right  = menuPos[0].left + 64;
		menuPos[1].bottom = menuPos[0].top + 16;
		
		menuPos[0].left   = 392+9;//NOTICE_X + 185 + 9; //확인
		menuPos[0].top    = 411+4;//NOTICE_Y + 381 + 4; //NOTICE_Y + 381 + 4;
		menuPos[0].right  = menuPos[1].left + 32;
		menuPos[0].bottom = menuPos[1].top + 16;

		menuPos[2].left   = 208+25;                    //도움말 물음표 이미지 위치
		menuPos[2].top    = 74;
		menuPos[2].right  = menuPos[2].left + 32;
		menuPos[2].bottom = menuPos[2].top + 32;

		menuPos[3].left   = 208+8;                      //라인
		menuPos[3].top    = 61+48;
		menuPos[3].right  = menuPos[3].left + 64;
		menuPos[3].bottom = menuPos[3].top + 16;

		menuPos[4].left   = 340;//NOTICE_X + 115 + 18;  //타이틀
		menuPos[4].top    = 75;
		menuPos[4].right  = menuPos[4].left + 64;
		menuPos[4].bottom = menuPos[4].top + 32;

		

		menuPos[5].left   = 208+55;                    //도움말 글씨 위치
		menuPos[5].top    = 74;
		menuPos[5].right  = menuPos[5].left + 64;
		menuPos[5].bottom = menuPos[5].top + 32;

		menuPos[6].left   = 208+300+5;                 //가이드 도움말 이미지
		menuPos[6].top    = 75;
		menuPos[6].right  = menuPos[6].left + 32;
		menuPos[6].bottom = menuPos[6].top + 32;

		menuPos[7].left   = 208+340;                   //가이드 도움말 글씨
		menuPos[7].top    = 75;
		menuPos[7].right  = menuPos[7].left+64;
		menuPos[7].bottom = menuPos[7].top + 32;

		menuPos[8].left   = NOTICE_X + 185 +100;       //도움말 보기 이미지
		menuPos[8].top    = 409;
		menuPos[8].right  = menuPos[8].left + 64;
		menuPos[8].bottom = menuPos[8].top + 16;

		menuPos[9].left   = menuPos[8].left+64 ;		    //온오프
		menuPos[9].top    = 411;
		menuPos[9].right  = menuPos[9].left + 64;
		menuPos[9].bottom = menuPos[9].top + 16;

		menuPos[10].left  = menuPos[4].left + 35;//115 + 18;		타이틀 제목
		menuPos[10].top   = menuPos[4].top + 9;
		
		//by woodongdong
 		for(int index = 0; index <= 10; index++)
		{
			menuPos[index].top    -= 50;
			menuPos[index].bottom -= 50;
		}

		//스크롤이있을경우
		nScrollPos = 11;
		//================스크롤 바if(nLevelHelpCount> LEVELHELP_COUNT_MAX)
		{			

			menuPos[nScrollPos].left = 602;//BackStartPos.x + NOTICE_SCROLLBAR_LEFT;
			menuPos[nScrollPos].top =  125;//BackStartPos.y + 10;//SCROOBAR_TOP ;
			menuPos[nScrollPos].right = menuPos[nScrollPos].left + 8;
			menuPos[nScrollPos].bottom = menuPos[nScrollPos].top + 290;
			
			//by woodongdong
			menuPos[nScrollPos].top    -= 50;
			menuPos[nScrollPos].bottom -= 50;
			
			//ktj 스크롤관련
			nScl_Bar = 0;
			tscr_Guide.Init(1);
			tscr_Guide.Init(	menuPos[nScrollPos].top,//BackStartPos.y + SCROOBAR_TOP, 
					menuPos[nScrollPos].top + 256,//290,
					nLevelHelpCount, LEVELHELP_COUNT_MAX);
					
			scrIconPos.left  = 602-4;//SCROLLBAR_LEFT-4;
			scrIconPos.top =  menuPos[nScrollPos].top  ;
			ScrollSavePoint.y = scrIconPos.top;
		}
		
 		nScrollPos = 12;
		for(i =0; i <LEVELHELP_COUNT_MAX; i++)
		{
			if(i>=nLevelHelpCount) break;
			menuPos[nScrollPos].left = menuPos[5].left;//NOTICE_X + 130;
			menuPos[nScrollPos].top  = NOTICE_Y + 80 + nCap;
			//by woodongdong
			menuPos[nScrollPos].top -= 50;
			
			//StrLen을 쓰는 유일한 부분. 삭제해도 되는지에 대한 고찰이 필요함 by teze
   	 		menuPos[nScrollPos].right  = menuPos[nScrollPos].left + nStrLen[i];//(6*lstrlen(LevelHelp[i]) + 30);//24;
			menuPos[nScrollPos].bottom = menuPos[nScrollPos].top + 24;
			
			nScrollPos ++;
			nCap+=24;
		}		
		break;

	case GUIDEHELP_CONTENT:
 		nScl_Bar = 0;
		menuSu = 16;
		menuPos[1].left = 392;//NOTICE_X + 185 ;		    //박스
		menuPos[1].top = 411;//NOTICE_Y + 381;
		menuPos[1].right = menuPos[0].left + 64;
		menuPos[1].bottom = menuPos[0].top + 16;

	
		
		menuPos[0].left = 392+9;//NOTICE_X + 185 + 9;		//확인
		menuPos[0].top = 411+4;//NOTICE_Y + 381 + 4;//NOTICE_Y + 381 + 4;
		menuPos[0].right = menuPos[1].left + 32;
		menuPos[0].bottom = menuPos[1].top + 16;

		menuPos[3].left = 208+8;           //라인
		menuPos[3].top = 61+48;
		menuPos[3].right = menuPos[3].left + 64;
		menuPos[3].bottom = menuPos[3].top + 16;

		menuPos[4].left = 340;//OTICE_X + 115 + 18;     //타이틀
		menuPos[4].top =  75;
		menuPos[4].right = menuPos[4].left + 64;
		menuPos[4].bottom = menuPos[4].top + 32;

		menuPos[2].left = 208+25;          //도움말 물음표 이미지 위치
		menuPos[2].top =  74;
		menuPos[2].right = menuPos[2].left + 32;
		menuPos[2].bottom = menuPos[2].top + 32;

		menuPos[5].left = 208+55;     //도움말 글씨 위치
		menuPos[5].top =  74;
		menuPos[5].right = menuPos[5].left + 64;
		menuPos[5].bottom = menuPos[5].top + 32;

		menuPos[6].left = 208+300+5;     //가이드 도움말 이미지
		menuPos[6].top =  75;
		menuPos[6].right = menuPos[6].left + 32;
		menuPos[6].bottom = menuPos[6].top + 32;

		menuPos[7].left = 208+340;     //가이드 도움말 글씨
		menuPos[7].top =  75;
		menuPos[7].right = menuPos[7].left+64;
		menuPos[7].bottom = menuPos[7].top + 32;

		menuPos[8].left = NOTICE_X + 185 +100;		    //도움말 보기 이미지
		menuPos[8].top = 409;
		menuPos[8].right = menuPos[8].left + 64;
		menuPos[8].bottom = menuPos[8].top + 16;

		menuPos[9].left = menuPos[8].left+64 ;		    //온오프
		menuPos[9].top = 411;
		menuPos[9].right = menuPos[9].left + 64;
		menuPos[9].bottom = menuPos[9].top + 16;

		menuPos[10].left = NOTICE_X + 18 +8;		    //가이드 도움말 두루마기 이미지
		menuPos[10].top = 411;
		menuPos[10].right = menuPos[10].left + 24;
		menuPos[10].bottom = menuPos[10].top + 16;

		menuPos[11].left = menuPos[10].right-3;		    //가이드 도움말
		menuPos[11].top = 409;
		menuPos[11].right = menuPos[11].left + 64;
		menuPos[11].bottom = menuPos[11].top + 32;

		menuPos[12].left = menuPos[4].left + 35;//115 + 18;		타이틀 제목
		menuPos[12].top =  menuPos[4].top+9;

		menuPos[13].left = NOTICE_X + 18+18 ;;//115 + 18;		헬프 내용 위치
		menuPos[13].top =  NOTICE_Y + 75;

		menuPos[14].left = 430;		    //가이드 이미지
		menuPos[14].top = 270;
		menuPos[14].right = menuPos[14].left + 64;
		menuPos[14].bottom = menuPos[14].top + 32;

		//by woodongdong
		for(int index = 0; index <= 14; index++)
			menuPos[index].top -= 50;
		

		//================스크롤 바

		nScrollPos = 15;
		if(nHeight_Count> GUIDEHLEP_CONTENT_HEIGHT_MAX)
		{

			menuPos[nScrollPos].left = 602;//BackStartPos.x + NOTICE_SCROLLBAR_LEFT;
			menuPos[nScrollPos].top =  125;//BackStartPos.y + 10;//SCROOBAR_TOP ;
			menuPos[nScrollPos].right = menuPos[nScrollPos].left + 8;
			menuPos[nScrollPos].bottom = menuPos[nScrollPos].top + 290;

			//ktj 스크롤관련
			nScl_Bar = 0;
			tscr_Guide.Init(1);
			tscr_Guide.Init(	menuPos[nScrollPos].top,//BackStartPos.y + SCROOBAR_TOP, 
					menuPos[nScrollPos].top + 256,//290,
					nHeight_Count, GUIDEHLEP_CONTENT_HEIGHT_MAX);
					
			scrIconPos.left  = 602-4;//SCROLLBAR_LEFT-4;
			scrIconPos.top =  menuPos[nScrollPos].top  ;
			ScrollSavePoint.y = scrIconPos.top;
		}

		nScrollPos = 16;
		nCap = 0;
		for(int i = 0; i < GUIDEHLEP_CONTENT_HEIGHT_MAX; i++)
		{
			menuPos[nScrollPos].left = menuPos[13].left - 20;//BackStartPos.x + NOTICE_SCROLLBAR_LEFT;
			menuPos[nScrollPos].top =  menuPos[13].top + nCap;//BackStartPos.y + 10;//SCROOBAR_TOP ;
			nCap+=20;
			nScrollPos++;
		}

		break;
	}
}

void CGuideHelp::SetHelpN(int num)
{
	m_GuideHelpN = num;
}

void CGuideHelp::SetStartPos(int x, int y)
{
	m_Start.x = x;
	m_Start.y = y;
}

void CGuideHelp::InitLevelHelpLabel()
{	
//해외 용(GetPrivateProfileString 이 부분 전부다)
#define LEVELLABEL "./image/clanImage/Help/AutoHelp/LevelSubj.ini"

	if(JOB_IS_TEMPSKRON(sinChar->JOB_CODE)) // 템스크론과 모라이언의 구분인것 같음 by teze
	{
		GetPrivateProfileString("Level_1","Subj","Error",LevelHelp[0],1024,LEVELLABEL);
		GetPrivateProfileString("Level_2","Subj","Error",LevelHelp[1],1024,LEVELLABEL);
		GetPrivateProfileString("Level_3","Subj","Error",LevelHelp[2],1024,LEVELLABEL);
		GetPrivateProfileString("Level_4","Subj","Error",LevelHelp[3],1024,LEVELLABEL);
		GetPrivateProfileString("Level_5","Subj","Error",LevelHelp[4],1024,LEVELLABEL);
		GetPrivateProfileString("Level_6","Subj","Error",LevelHelp[5],1024,LEVELLABEL);
		GetPrivateProfileString("Level_7","Subj","Error",LevelHelp[6],1024,LEVELLABEL);
		GetPrivateProfileString("Level_8","Subj","Error",LevelHelp[7],1024,LEVELLABEL);
		GetPrivateProfileString("Level_9","Subj","Error",LevelHelp[8],1024,LEVELLABEL);
		GetPrivateProfileString("Level_10","Subj","Error",LevelHelp[9],1024,LEVELLABEL);
		GetPrivateProfileString("Level_11","Subj","Error",LevelHelp[10],1024,LEVELLABEL);
		GetPrivateProfileString("Level_12","Subj","Error",LevelHelp[11],1024,LEVELLABEL);		
		GetPrivateProfileString("Level_15","Subj","Error",LevelHelp[12],1024,LEVELLABEL);		
		GetPrivateProfileString("Level_17","Subj","Error",LevelHelp[13],1024,LEVELLABEL);
		GetPrivateProfileString("Level_20","Subj","Error",LevelHelp[14],1024,LEVELLABEL);
		GetPrivateProfileString("Level_22","Subj","Error",LevelHelp[15],1024,LEVELLABEL);
		GetPrivateProfileString("Level_25","Subj","Error",LevelHelp[16],1024,LEVELLABEL);
		GetPrivateProfileString("Level_30","Subj","Error",LevelHelp[17],1024,LEVELLABEL);
		GetPrivateProfileString("Level_39","Subj","Error",LevelHelp[18],1024,LEVELLABEL);
		GetPrivateProfileString("Level_40","Subj","Error",LevelHelp[19],1024,LEVELLABEL);
		GetPrivateProfileString("Level_50","Subj","Error",LevelHelp[20],1024,LEVELLABEL);
		GetPrivateProfileString("Level_55","Subj","Error",LevelHelp[21],1024,LEVELLABEL);
		GetPrivateProfileString("Level_60","Subj","Error",LevelHelp[22],1024,LEVELLABEL);
		GetPrivateProfileString("Level_65","Subj","Error",LevelHelp[23],1024,LEVELLABEL);
		GetPrivateProfileString("Level_70","Subj","Error",LevelHelp[24],1024,LEVELLABEL);
		GetPrivateProfileString("Level_75","Subj","Error",LevelHelp[25],1024,LEVELLABEL);
		GetPrivateProfileString("Level_79","Subj","Error",LevelHelp[26],1024,LEVELLABEL);
		GetPrivateProfileString("Level_80","Subj","Error",LevelHelp[27],1024,LEVELLABEL);		
	}
	else
	{
		
		GetPrivateProfileString("Level_1","Subj","Error",LevelHelp[0],1024,LEVELLABEL);
		GetPrivateProfileString("Level_2","Subj","Error",LevelHelp[1],1024,LEVELLABEL);
		GetPrivateProfileString("Level_3","Subj","Error",LevelHelp[2],1024,LEVELLABEL);
		GetPrivateProfileString("Level_4","Subj","Error",LevelHelp[3],1024,LEVELLABEL);
		GetPrivateProfileString("Level_5","Subj","Error",LevelHelp[4],1024,LEVELLABEL);
		GetPrivateProfileString("Level_6","Subj","Error",LevelHelp[5],1024,LEVELLABEL);
		GetPrivateProfileString("Level_7","Subj","Error",LevelHelp[6],1024,LEVELLABEL);
		GetPrivateProfileString("Level_8","Subj","Error",LevelHelp[7],1024,LEVELLABEL);
		GetPrivateProfileString("Level_9","Subj","Error",LevelHelp[8],1024,LEVELLABEL);
		GetPrivateProfileString("Level_10","Subj","Error",LevelHelp[9],1024,LEVELLABEL);
		GetPrivateProfileString("Level_11","Subj","Error",LevelHelp[10],1024,LEVELLABEL);
		GetPrivateProfileString("Level_12","Subj","Error",LevelHelp[11],1024,LEVELLABEL);		
		GetPrivateProfileString("Level_15","Subj","Error",LevelHelp[12],1024,LEVELLABEL);		
		GetPrivateProfileString("Level_17","Subj","Error",LevelHelp[13],1024,LEVELLABEL);
		GetPrivateProfileString("Level_20","Subj","Error",LevelHelp[14],1024,LEVELLABEL);
		GetPrivateProfileString("Level_22","Subj","Error",LevelHelp[15],1024,LEVELLABEL);
		GetPrivateProfileString("Level_25","Subj","Error",LevelHelp[16],1024,LEVELLABEL);
		GetPrivateProfileString("Level_30","Subj","Error",LevelHelp[17],1024,LEVELLABEL);
		GetPrivateProfileString("Level_39","Subj","Error",LevelHelp[18],1024,LEVELLABEL);
		GetPrivateProfileString("Level_40","Subj","Error",LevelHelp[19],1024,LEVELLABEL);
		GetPrivateProfileString("Level_50","Subj","Error",LevelHelp[20],1024,LEVELLABEL);
		GetPrivateProfileString("Level_55","Subj","Error",LevelHelp[21],1024,LEVELLABEL);
		GetPrivateProfileString("Level_60","Subj","Error",LevelHelp[22],1024,LEVELLABEL);
		GetPrivateProfileString("Level_65","Subj","Error",LevelHelp[23],1024,LEVELLABEL);
		GetPrivateProfileString("Level_70","Subj","Error",LevelHelp[24],1024,LEVELLABEL);
		GetPrivateProfileString("Level_75","Subj","Error",LevelHelp[25],1024,LEVELLABEL);
		GetPrivateProfileString("Level_79","Subj","Error",LevelHelp[26],1024,LEVELLABEL);
		GetPrivateProfileString("Level_80","Subj","Error",LevelHelp[27],1024,LEVELLABEL);		
	}
/*
	if(JOB_IS_TEMPSKRON(sinChar->JOB_CODE)) // 템스크론과 모라이언의 구분인것 같음 by teze
	{		
		strcpy(LevelHelp[0],"프리스톤 테일에 오신것을 환영합니다");
		strcpy(LevelHelp[1],"포인트 분배는 이렇게");
		strcpy(LevelHelp[2],"물약 쉽게 사용하기");
		strcpy(LevelHelp[3],"미니맵 보기");
		strcpy(LevelHelp[4],"2가지의 장비 착용하기");
		strcpy(LevelHelp[5],"동료 하는 방법");
		strcpy(LevelHelp[6],"자동 공격");
		strcpy(LevelHelp[7],"워프 게이트 이용방법");
		strcpy(LevelHelp[8],"특화아이템");
		strcpy(LevelHelp[9],"스킬 배우기");		
		strcpy(LevelHelp[10],"포스 오브 시스템");//HoDong //11레벨
		strcpy(LevelHelp[11],"조합으로 강력해지는 아이템,믹스처 시스템"); //12레벨
		strcpy(LevelHelp[12],"가공을 통한 아이템 성능 향상 에이징 시스템");
		strcpy(LevelHelp[13],"모두를 하나로 묶어주는 클랜시스템");	
		strcpy(LevelHelp[14],"보다 강력한 스킬을 배울 수 있는 전업시스템");
		strcpy(LevelHelp[15],"저주 받은 도시 네비스코");
		strcpy(LevelHelp[16],"시련의 탑(벨라트라)에 참여해보자");
		strcpy(LevelHelp[17],"강한 전사만이 갈 수 있는 레벨 제한구역");
		strcpy(LevelHelp[18],"레벨 40은 이렇다.");
		strcpy(LevelHelp[19],"고대감옥 1층을 가보고 클랜을 만들어보자");
		strcpy(LevelHelp[20],"고대감옥 2층도 가보자");
		strcpy(LevelHelp[21],"더욱 깊숙한 사막과 동굴을 탐험하자");
		strcpy(LevelHelp[22],"고대감옥 3층을 가보자");
		strcpy(LevelHelp[23],"어둠의 성전의 입구가 열렸다");
		strcpy(LevelHelp[24],"어둠의 신전 1층을 가보자");
		strcpy(LevelHelp[25],"어둠의 신전 2층을 가보자");
		strcpy(LevelHelp[26],"가까워진 혼돈의 철로");//HoDong
		strcpy(LevelHelp[27],"혼돈의 철로로, 그리고 바벨을 만나보자");//by teze
	}
	else
	{
		
		strcpy(LevelHelp[0],"프리스톤 테일에 오신것을 환영합니다");
		strcpy(LevelHelp[1],"포인트 분배는 이렇게");
		strcpy(LevelHelp[2],"물약 쉽게 사용하기");
		strcpy(LevelHelp[3],"미니맵 보기");
		strcpy(LevelHelp[4],"2가지의 장비 착용하기");
		strcpy(LevelHelp[5],"동료 하는 방법");
		strcpy(LevelHelp[6],"자동 공격");
		strcpy(LevelHelp[7],"워프 게이트 이용방법");
		strcpy(LevelHelp[8],"특화아이템");
		strcpy(LevelHelp[9],"스킬 배우기");		
		strcpy(LevelHelp[10],"포스 오브 시스템");//HoDong //11레벨
		strcpy(LevelHelp[11],"조합으로 강력해지는 아이템,믹스처 시스템"); //12레벨
		strcpy(LevelHelp[12],"가공을 통한 아이템 성능 향상 에이징 시스템");
		strcpy(LevelHelp[13],"모두를 하나로 묶어주는 클랜시스템");	
		strcpy(LevelHelp[14],"보다 강력한 스킬을 배울 수 있는 전업시스템");
		strcpy(LevelHelp[15],"저주 받은 도시 네비스코");
		strcpy(LevelHelp[16],"시련의 탑(벨라트라)에 참여해보자");
		strcpy(LevelHelp[17],"강한 전사만이 갈 수 있는 레벨 제한구역");
		strcpy(LevelHelp[18],"레벨 40은 이렇다.");
		strcpy(LevelHelp[19],"고대감옥 1층을 가보고 클랜을 만들어보자");
		strcpy(LevelHelp[20],"고대감옥 2층도 가보자");
		strcpy(LevelHelp[21],"더욱 깊숙한 사막과 동굴을 탐험하자");
		strcpy(LevelHelp[22],"고대감옥 3층을 가보자");
		strcpy(LevelHelp[23],"어둠의 성전의 입구가 열렸다");
		strcpy(LevelHelp[24],"어둠의 신전 1층을 가보자");
		strcpy(LevelHelp[25],"어둠의 신전 2층을 가보자");
		strcpy(LevelHelp[26],"가까워진 혼돈의 철로");//HoDong
		strcpy(LevelHelp[27],"혼돈의 철로로, 그리고 바벨을 만나보자");//by teze
	}
*/
}



void CGuideHelp::CheckLevel(int level)
{
	if(level == 1)
		nLevelHelpCount = 1;
	
	if(level == 2)
		nLevelHelpCount = 2;
	
	if(level == 3)
		nLevelHelpCount = 3;
	
	if(level == 4)
		nLevelHelpCount = 4;
	
	if(level == 5)
		nLevelHelpCount = 5;
	
	if(level == 6)
		nLevelHelpCount = 6;
	
	if(level == 7)
		nLevelHelpCount = 7;
	
	if((level == 8)||(level==9))
		nLevelHelpCount = 8;
	
	if(level == 9)
		nLevelHelpCount = 9;
	
	if(level == 10)
		nLevelHelpCount = 10;
	
	if(level == 11)
		nLevelHelpCount = 11;

	if((level >=12) &&(level < 15))
		nLevelHelpCount = 12;
	
	if((level == 15) ||(level==16))
		nLevelHelpCount = 13;

	if((level >=17) &&(level < 20))
		nLevelHelpCount = 14;

	if((level == 20) ||(level==21))
	{
		//if(JOB_IS_TEMPSKRON(sinChar->JOB_CODE))
		nLevelHelpCount = 15;
	}
	if((level >=22) &&(level < 25))
		nLevelHelpCount = 16;		
	
	if((level >=25) &&(level < 30))
	{
		if(JOB_IS_TEMPSKRON(sinChar->JOB_CODE))
		{
			nLevelHelpCount = 17;
		}
		else
		{
			nLevelHelpCount = 17;
		}
	}
	if((level >=30) &&(level < 40))
	{
		if(JOB_IS_TEMPSKRON(sinChar->JOB_CODE))
		{
			nLevelHelpCount = 18;
		}
		else
		{
			nLevelHelpCount = 18;
		}
	}

	if((level >=39))
		nLevelHelpCount = 19;
	
	if((level >=40) &&(level < 50))
	{
		if(JOB_IS_TEMPSKRON(sinChar->JOB_CODE))
		{
			nLevelHelpCount = 20;
		}
		else
		{
			nLevelHelpCount = 20;
		}
	}
	if((level >=50) &&(level < 55))
	{
		if(JOB_IS_TEMPSKRON(sinChar->JOB_CODE))
		{
			nLevelHelpCount = 21;
		}
		else
		{
			nLevelHelpCount = 21;
		}
	}
	if((level >=55) &&(level < 60))
	{
		if(JOB_IS_TEMPSKRON(sinChar->JOB_CODE))
		{
			nLevelHelpCount = 22;
		}
		else
		{
			nLevelHelpCount = 22;
		}
	}
	if((level >=60) &&(level < 65))
	{
		if(JOB_IS_TEMPSKRON(sinChar->JOB_CODE))
		{
			nLevelHelpCount = 23;
		}
		else
		{
			nLevelHelpCount = 23;
		}
	}
	if((level >=65) &&(level < 70))
	{
		if(JOB_IS_TEMPSKRON(sinChar->JOB_CODE))
		{
			nLevelHelpCount = 24;
		}
		else
		{
			nLevelHelpCount = 24;
		}
	}
	/*
	if((level >=70) &&(level < 80))
	{
		if(JOB_IS_TEMPSKRON(sinChar->JOB_CODE))
		{
			nLevelHelpCount = 24;
		}
		else
		{
			nLevelHelpCount = 24;
		}
	}*/
	//by teze
	if((level >=70) &&(level < 75))
	{
		if(JOB_IS_TEMPSKRON(sinChar->JOB_CODE))
		{
			nLevelHelpCount = 25;
		}
		else
		{
			nLevelHelpCount = 25;
		}
	}
	//by teze
	if((level >= 75) &&(level < 79))
	{
		if(JOB_IS_TEMPSKRON(sinChar->JOB_CODE))
		{
			nLevelHelpCount = 26;
		}
		else
		{
			nLevelHelpCount = 26;
		}
	}

	if(level == 79)
		nLevelHelpCount = 27;

	if(level >= 80)
	{
		if(JOB_IS_TEMPSKRON(sinChar->JOB_CODE))
		{
			nLevelHelpCount = 28;
		}
		else
		{
			nLevelHelpCount = 28;
		}
	}

	//예외가 발생시 무시해버리는 경우임. 조금 위험한 디버깅 기법
	if(nLevelHelpCount > LEVELHELP_COUNT_MAX)
	{
		nHelpCountNamugi = nLevelHelpCount - LEVELHELP_COUNT_MAX;
	}
}

void CGuideHelp::GetHelpContent(char* section,char* key,char* ImageKey)
{
	DWORD dwSaveSize = 100;
	char  szSaveStr[100];
	char  szImageDir[100];
	DWORD dwReturn,dwFileSize=0,dwRead = 0;
	HANDLE hFile = NULL;
	char *HelpContent;

	ZeroMemory(m_HelpContent,sizeof(m_HelpContent));
	ZeroMemory(szImageDir,sizeof(szImageDir));

	dwReturn = GetPrivateProfileString(section,key,"Error",szSaveStr,1024,HELP_CONTENT);

	if(dwReturn == 0) return;	
	dwReturn = GetPrivateProfileString(section,ImageKey,"Error",szImageDir,1024,HELP_CONTENT);
	if(dwReturn == 0) return;

	LoadGuideImage(szImageDir);

	hFile = CreateFile( szSaveStr, GENERIC_READ, FILE_SHARE_READ,
                        NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL , NULL );

	if(hFile == INVALID_HANDLE_VALUE) return ;

	dwFileSize = GetFileSize(hFile,NULL);

	HelpContent = new char[dwFileSize];
	ZeroMemory(HelpContent,sizeof(char)*dwFileSize);

	ReadFile(hFile,HelpContent,dwFileSize,&dwRead,NULL);
	CloseHandle(hFile);

	Parsing(HelpContent);

	delete[] HelpContent;	

}

void CGuideHelp::Parsing(char* str)
{
	int nCnt,nCnt2,nCnt3;
	nCnt=nCnt2=nCnt3 = 0;
	nHeight_Count = 0;
	while(1)
	{
		if(str[nCnt3] == 0) break;
		
		if(str[nCnt3] == 0x0d)
		{
			nCnt3++;
			if(str[nCnt3] == 0) break;
			if(str[nCnt3] ==0x0a)
			{
				if(str[nCnt3+1] == 0x0d) break;
				else
				{
					nCnt3++;
					nCnt++;
					nCnt2 = 0;
				}
			}
			else
			{
				nCnt++;
				nCnt2 = 0;
				m_HelpContent[nCnt][nCnt2] = str[nCnt3];
			}
			
			continue;
		}
		else
		{
			m_HelpContent[nCnt][nCnt2] = str[nCnt3];
		}
		nCnt3++;
		nCnt2++;
	}

	nHeight_Count = nCnt+1;
	if(nHeight_Count > GUIDEHLEP_CONTENT_HEIGHT_MAX)
		nSaveNamugi = nHeight_Count - GUIDEHLEP_CONTENT_HEIGHT_MAX;

	
	
}

void CGuideHelp::CheckLevel(int level,int type)
{	

	switch(level)
	{
	case 1: //1~4  템스크론, 5~8 모라이언
		if(type <5)
		{
			//템스크론에서 캐릭터 처음 생성할때
			GetHelpContent("CreateCha_Temscron","Key0","Guide1");
		}
		else
		{
			//모라이언에서 캐릭터 처음 생성할때
			GetHelpContent("CreateCha_Moraion","Key1","Guide2");
		}
		break;
	case 2: //레벨2
		GetHelpContent("Level_2","Key2","Guide3");
		
		break;
	case 3:
		GetHelpContent("Level_3","Key3","Guide4");
		
		break;
	case 4:
		GetHelpContent("Level_4","Key4","Guide5");
		break;
	case 5:
		GetHelpContent("Level_5","Key5","Guide6");
		break;
	case 6:
		GetHelpContent("Level_6","Key6","Guide7");
		
		break;
	case 7:
		GetHelpContent("Level_7","Key24","Guide26");
		
		break;
	case 8:
		GetHelpContent("Level_8","Key7","Guide8");
		break;
	case 9:
		GetHelpContent("Level_9","Key25","Guide27");
		break;
	case 10:
		
		if(type <5)
		{
			//템스크론
			GetHelpContent("Level_10_Temscron","Key8","Guide9");
		}
		else
		{
			//모라이언
			GetHelpContent("Level_10_Moraion","Key9","Guide10");
		}
		break;

	//by HoDong
	case 11:
		GetHelpContent("Level_11","Key11","Guide11");
		break;

	case 12:
		
		GetHelpContent("Level_12","Key10","Guide11");
		break;
	case 15:
		
		GetHelpContent("Level_15","Key11","Guide12");
		break;
	case 17:
		GetHelpContent("Level_17","Key12","Guide13");
		break;
	case 20:
		
		if(type<5)
		{		
			GetHelpContent("Level_20_Temscron","Key13","Guide14");
		}
		else
		{
			GetHelpContent("Level_20_Moraion","Key24","Guide15");
		}
		break;
	case 22:
		
		GetHelpContent("Level_22","Key14","Guide16");
		break;
	case 25:
		
		GetHelpContent("Level_25","Key15","Guide17");
		break;
	case 30:
		
		GetHelpContent("Level_30","Key16","Guide18");
		break;
	case 39:
		GetHelpContent("Level_39","Key26","Guide26");
		break;
	case 40:
		
		GetHelpContent("Level_40","Key17","Guide19");
		break;
	case 50:
		
		GetHelpContent("Level_50","Key18","Guide20");
		break;
	case 55:
		GetHelpContent("Level_55","Key19","Guide21");
		break;
	case 60:
		GetHelpContent("Level_60","Key20","Guide22");
		break;
	case 65:
		
		GetHelpContent("Level_65","Key21","Guide23");
		break;
	case 70:
		
		GetHelpContent("Level_70","Key22","Guide24");
		break;
	//by teze
	case 75:
		GetHelpContent("Level_75","Key28","Guide24");
		break;
	//by HoDong
	case 79:
		GetHelpContent("Level_79","Key79","Guide79");
		break;

	case 80:
		GetHelpContent("Level_80","Key23","Guide25");
		break;
	}
	
}



void CGuideHelp::Draw_Help_Text(char *str, int x, int y, int sw)
{

	char strbuf[1024];

	HDC hdc;
	HFONT oldFont;
	lpDDSBack->GetDC(&hdc);
	SetBkMode( hdc, TRANSPARENT );

	if(hFont != NULL)
		oldFont = (HFONT)SelectObject( hdc , hFont );
	//if(sinBoldFont!= NULL)
	//	oldFont = (HFONT)SelectObject( hdc , sinBoldFont );


	ZeroMemory(strbuf, sizeof(strbuf));

	int strL = strlen(str);
	int cnt=0;
	int i=0;

	while(1) {
		

		strbuf[i] = str[cnt];
		cnt++;
		if(strbuf[i] == '\n')  {
			strbuf[i] =0;
			i=0;
jmp1:


			
			if(sw==2){
				SetTextColor( hdc, RGB(100, 100, 100));	//그림자용 글자색
				//SetTextColor( hdc, RGB(255, 255,255));	//그림자용 글자색
				dsTextLineOut(hdc, x+1, y+1, strbuf, lstrlen(strbuf));

				SetTextColor( hdc, RGB(255, 240, 200));	//일반글자색
				dsTextLineOut(hdc, x, y, strbuf, lstrlen(strbuf));
			}
			else     {
				SetTextColor( hdc, RGB(255, 240, 200));	//일반글자색
				dsTextLineOut(hdc, x, y, strbuf, lstrlen(strbuf));
			}
			
			/*
			if(sw==2){
				SetTextColor( hdc, RGB(255, 255, 255));	//클랜이름입력시 흰색				
			}
			else {
				SetTextColor( hdc, RGB(230, 255, 240));	//일반글자색
			}
			dsTextLineOut(hdc, x, y, strbuf, lstrlen(strbuf));
			*/
			

			ZeroMemory(strbuf, sizeof(strbuf));
			y+=16;

			if(cnt>=strL) break;
		}
		else if(cnt>=strL)
		{
			goto jmp1;
		}
		else {
			i++;
		}
	}
	

	if(sw==1) 
	{
		static int cnt=0;
		cnt++;
		if(cnt<20) {
			SetTextColor( hdc, RGB(255, 255, 255));
			dsTextLineOut(hdc, x + (lstrlen(str)*6) , y-16, "|", 1);
		}
		if(cnt>=40) cnt=0;
	}

	if(oldFont)
		SelectObject( hdc, oldFont);
	lpDDSBack->ReleaseDC(hdc);

}

void CGuideHelp::Draw_Help_Text2(char *str, int x, int y, int sw)
{

	char strbuf[1024];

	HDC hdc;
	HFONT oldFont;
	lpDDSBack->GetDC(&hdc);
	SetBkMode( hdc, TRANSPARENT );

	if(hFont != NULL)
		oldFont = (HFONT)SelectObject( hdc , hFont );
	//if(sinBoldFont!= NULL)
	//	oldFont = (HFONT)SelectObject( hdc , sinBoldFont );


	ZeroMemory(strbuf, sizeof(strbuf));

	int strL = strlen(str);
	int cnt=0;
	int i=0;

	while(1) {
		

		strbuf[i] = str[cnt];
		cnt++;
		if(strbuf[i] == '\n')  {
			strbuf[i] =0;
			i=0;
jmp1:


			
			if(sw==2){
				SetTextColor( hdc, RGB(100, 100, 100));	//그림자용 글자색
				//SetTextColor( hdc, RGB(255, 255,255));	//그림자용 글자색
				dsTextLineOut(hdc, x+1, y+1, strbuf, lstrlen(strbuf));

				SetTextColor( hdc, RGB(255, 200, 100));	//일반글자색
				dsTextLineOut(hdc, x, y, strbuf, lstrlen(strbuf));
			}
			else     {
				SetTextColor( hdc, RGB(255, 200, 100));	//일반글자색
				dsTextLineOut(hdc, x, y, strbuf, lstrlen(strbuf));
			}
			
			/*
			if(sw==2){
				SetTextColor( hdc, RGB(255, 255, 255));	//클랜이름입력시 흰색				
			}
			else {
				SetTextColor( hdc, RGB(230, 255, 240));	//일반글자색
			}
			dsTextLineOut(hdc, x, y, strbuf, lstrlen(strbuf));
			*/
			

			ZeroMemory(strbuf, sizeof(strbuf));
			y+=16;

			if(cnt>=strL) break;
		}
		else if(cnt>=strL)
		{
			goto jmp1;
		}
		else {
			i++;
		}
	}
	

	if(sw==1) 
	{
		static int cnt=0;
		cnt++;
		if(cnt<20) {
			SetTextColor( hdc, RGB(255, 255, 255));
			dsTextLineOut(hdc, x + (lstrlen(str)*6) , y-16, "|", 1);
		}
		if(cnt>=40) cnt=0;
	}

	if(oldFont)
		SelectObject( hdc, oldFont);
	lpDDSBack->ReleaseDC(hdc);

}

void CGuideHelp::Draw_Title_Text(char *str, int x, int y, int sw)
{

	char strbuf[1024];

	HDC hdc;
	HFONT oldFont;
	lpDDSBack->GetDC(&hdc);
	SetBkMode( hdc, TRANSPARENT );

	//if(hFont != NULL)
		//oldFont = (HFONT)SelectObject( hdc , hFont );
	if(sinBoldFont!= NULL)
		oldFont = (HFONT)SelectObject( hdc , sinBoldFont );


	ZeroMemory(strbuf, sizeof(strbuf));

	int strL = strlen(str);
	int cnt=0;
	int i=0;

	while(1) {
		

		strbuf[i] = str[cnt];
		cnt++;
		if(strbuf[i] == '\n')  {
			strbuf[i] =0;
			i=0;
jmp1:


			
			if(sw==2){
				SetTextColor( hdc, RGB(100, 100, 100));	//그림자용 글자색
				//SetTextColor( hdc, RGB(255, 255,255));	//그림자용 글자색
				dsTextLineOut(hdc, x+1, y+1, strbuf, lstrlen(strbuf));

				SetTextColor( hdc, RGB(255, 240, 200));	//일반글자색
				dsTextLineOut(hdc, x, y, strbuf, lstrlen(strbuf));
			}
			else     {
				SetTextColor( hdc, RGB(255, 240, 200));	//일반글자색
				dsTextLineOut(hdc, x, y, strbuf, lstrlen(strbuf));
			}
			
			/*
			if(sw==2){
				SetTextColor( hdc, RGB(255, 255, 255));	//클랜이름입력시 흰색				
			}
			else {
				SetTextColor( hdc, RGB(230, 255, 240));	//일반글자색
			}
			dsTextLineOut(hdc, x, y, strbuf, lstrlen(strbuf));
			*/
			

			ZeroMemory(strbuf, sizeof(strbuf));
			y+=16;

			if(cnt>=strL) break;
		}
		else if(cnt>=strL)
		{
			goto jmp1;
		}
		else {
			i++;
		}
	}
	

	if(sw==1) 
	{
		static int cnt=0;
		cnt++;
		if(cnt<20) {
			SetTextColor( hdc, RGB(255, 255, 255));
			dsTextLineOut(hdc, x + (lstrlen(str)*6) , y-16, "|", 1);
		}
		if(cnt>=40) cnt=0;
	}

	if(oldFont)
		SelectObject( hdc, oldFont);
	lpDDSBack->ReleaseDC(hdc);

}




int CGuideHelp::dsDrawTexImageFloat( int Mat , float x, float y, float w, float h , int Transp , int specular )
{
	int clipLeft;
	int clipTop;
	int clipRight;
	int clipBottom;
	
	clipLeft = 0;
	clipTop = 0;
	clipRight = WinSizeX;
	clipBottom = WinSizeY;
	
	//Clipping 영역을 완전히 벗어나면...
	if ((x >= clipRight) || (y >= clipBottom) || ((x + w) <= clipLeft) || ((y+h) <= clipTop))
		return FALSE;
	
	//원래 사각 영역의 크기는 0, 0, 127, 127 이지만 이렇게 찍을 경우. 한 픽셀이 빠져서 0, 0, 128, 128로 보정한다.
	float x1 = x;
	float y1 = y;
	float x2 = (x+w);
	float y2 = (y+h);
	
	/*
	if(x1 < clipLeft)
		x1 = clipLeft;
	if(y1 < clipTop)
		y1 = clipTop;

	if(x2 >= clipRight)
		x2 = clipRight-1;

	if(y2 >= clipBottom)
		y2 = clipBottom-1;
	*/

	//x1, y1, x2, y2 는 클립된 사각영역이다..
	D3DTLVERTEX tlVertex[4];
	int cnt;

	for(cnt=0;cnt<4;cnt++)
	{
		tlVertex[cnt].rhw = 1.f;
		tlVertex[cnt].color	= RGBA_MAKE( 255,255,255,Transp );
		tlVertex[cnt].specular  =  RGBA_MAKE( specular,specular,specular,0 );
		tlVertex[cnt].sz = 0;
	}
	/*
	tlVertex[0].sx = x1;
	tlVertex[0].sy = y1;
	tlVertex[0].tu = 1.0f*( (x1-(float)x)/w);
	tlVertex[0].tv = 1.0f*( (y1-(float)y)/h);
	
	tlVertex[1].sx = x1;
	tlVertex[1].sy = y2;
	tlVertex[1].tu = 1.0f*( (x1-(float)x)/w);
	tlVertex[1].tv = 1.0f*( (y2-(float)y)/h);
	
	tlVertex[2].sx = x2;
	tlVertex[2].sy = y1;
	tlVertex[2].tu = 1.0f*( (x2-(float)x)/w);
	tlVertex[2].tv = 1.0f*( (y1-(float)y)/h);
	
	tlVertex[3].sx = x2;
	tlVertex[3].sy = y2;
	tlVertex[3].tu = 1.0f*( (x2-(float)x)/w);
	tlVertex[3].tv = 1.0f*( (y2-(float)y)/h);
	*/
	
	//이게 진짜...
	/*
	tlVertex[0].sx = x1;
	tlVertex[0].sy = y1;
	tlVertex[0].tu = 1.0f*(float)( (x1-(float)x)/(float)w);
	tlVertex[0].tv = 1.0f*(float)( (y1-(float)y)/(float)h);
	
	tlVertex[1].sx = x1;
	tlVertex[1].sy = y2;
	tlVertex[1].tu = 1.0f*(float)( (x1-(float)x)/(float)w);
	tlVertex[1].tv = 1.0f*(float)( (y2-(float)y)/(float)h);
	
	tlVertex[2].sx = x2;
	tlVertex[2].sy = y1;
	tlVertex[2].tu = 1.0f*(float)( (x2-(float)x)/(float)w);
	tlVertex[2].tv = 1.0f*(float)( (y1-(float)y)/(float)h);
	
	tlVertex[3].sx = x2;
	tlVertex[3].sy = y2;
	tlVertex[3].tu = 1.0f*(float)( (x2-(float)x)/(float)w);
	tlVertex[3].tv = 1.0f*(float)( (y2-(float)y)/(float)h);
	*/

	
	tlVertex[0].sx = x1;
	tlVertex[0].sy = y1;
	tlVertex[0].tu = 0.0f;
	tlVertex[0].tv = 0.0f;
	
	tlVertex[1].sx = x1;
	tlVertex[1].sy = y2;
	tlVertex[1].tu = 0.0f;
	tlVertex[1].tv = 1.0f;
	
	tlVertex[2].sx = x2;
	tlVertex[2].sy = y1;
	tlVertex[2].tu = 1.0f;
	tlVertex[2].tv = 0.0f;
	
	tlVertex[3].sx = x2;
	tlVertex[3].sy = y2;
	tlVertex[3].tu = 1.0f;
	tlVertex[3].tv = 1.0f;
	
	/*
	tlVertex[0].sx = x;
	tlVertex[0].sy = y;
	tlVertex[0].tu = 0;
	tlVertex[0].tv = 0;

	tlVertex[1].sx = x;
	tlVertex[1].sy = y+h;
	tlVertex[1].tu = 0;
	tlVertex[1].tv = 1;

	tlVertex[2].sx = x+w;
	tlVertex[2].sy = y;
	tlVertex[2].tu = 1;
	tlVertex[2].tv = 0;

	tlVertex[3].sx = x+w;
	tlVertex[3].sy = y+h;
	tlVertex[3].tu = 1;
	tlVertex[3].tv = 1;
	*/

	//뒤집힌면 그리기
	//lpD3DDevice->SetRenderState( D3DRENDERSTATE_CULLMODE , D3DCULL_NONE );
	if(lpD3DDevice != NULL)
	{
		  // Turn on Z-buffering
		lpD3DDevice->SetRenderState( D3DRENDERSTATE_CULLMODE , D3DCULL_NONE );
		lpD3DDevice->SetRenderState( D3DRENDERSTATE_ZENABLE, TRUE );
		lpD3DDevice->SetRenderState( D3DRENDERSTATE_ZWRITEENABLE, TRUE );
		//lpD3DDevice->SetRenderState( D3DRENDERSTATE_ZFUNC,   D3DCMP_LESSEQUAL );
		//	lpD3DDevice->SetRenderState( D3DRENDERSTATE_ZENABLE, D3DZB_USEW );//TRUE );


		// null out the texture handle
		lpD3DDevice->SetRenderState( D3DRENDERSTATE_TEXTUREHANDLE, 0 );

		// turn on dithering
		//D3DDevice->SetRenderState( D3DRENDERSTATE_DITHERENABLE, TRUE );

		lpD3DDevice->SetRenderState( D3DRENDERSTATE_TEXTUREADDRESS, D3DTADDRESS_WRAP);
		//lpD3DDevice->SetRenderState( D3DRENDERSTATE_TEXTUREPERSPECTIVE, TRUE ); 

		// D3DFILTER_LINEAR
		//lpD3DDevice->SetRenderState( D3DRENDERSTATE_TEXTUREMAG, D3DFILTER_LINEAR );
		//lpD3DDevice->SetRenderState( D3DRENDERSTATE_TEXTUREMIN, D3DFILTER_LINEAR );
 
		lpD3DDevice->SetRenderState( D3DRENDERSTATE_SPECULARENABLE, TRUE );
		lpD3DDevice->SetRenderState( D3DRENDERSTATE_COLORKEYENABLE, FALSE );

		lpD3DDevice->SetRenderState( D3DRENDERSTATE_TEXTUREMAPBLEND, D3DTBLEND_MODULATE );

		lpD3DDevice->SetRenderState( D3DRENDERSTATE_ALPHABLENDENABLE, TRUE );
		lpD3DDevice->SetRenderState( D3DRENDERSTATE_DESTBLEND, D3DBLEND_INVSRCALPHA );
		lpD3DDevice->SetRenderState( D3DRENDERSTATE_SRCBLEND, D3DBLEND_SRCALPHA );

		smRender.AlphaTestDepth = 60;

		lpD3DDevice->SetTextureStageState( 0, D3DTSS_MINFILTER, D3DTFN_POINT  );
        lpD3DDevice->SetTextureStageState( 0, D3DTSS_MAGFILTER, D3DTFG_POINT  );

		//lpD3DDevice->SetRenderState( D3DRENDERSTATE_TEXTUREMIN, D3DTFN_POINT );
		//lpD3DDevice->SetRenderState( D3DRENDERSTATE_TEXTUREMAG, D3DTFN_POINT );
		

		if(GuideImage->smMaterial[Mat].smTexture[0] != NULL)
		{
			if(GuideImage->smMaterial[Mat].smTexture[0]->lpD3DTexture != NULL)
			{
				if(FAILED(lpD3DDevice->SetTexture( 0, GuideImage->smMaterial[Mat].smTexture[0]->lpD3DTexture )))
				{
					OutputDebugString("SetTexture Error");
					return FALSE;
				}

				
				if(FAILED(lpD3DDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP ,
								D3DFVF_TLVERTEX  ,
								tlVertex,
								4,
								NULL)))
				{
					OutputDebugString("DrawPrimitive Error");
					return FALSE;
				}
			}
		}
		
		lpD3DDevice->SetTextureStageState( cnt, D3DTSS_MINFILTER, D3DTFN_LINEAR  );
        lpD3DDevice->SetTextureStageState( cnt, D3DTSS_MAGFILTER, D3DTFG_LINEAR  );
		//D3DDevice->SetRenderState( D3DRENDERSTATE_TEXTUREADDRESS, D3DTADDRESS_WRAP);
		//lpD3DDevice->SetRenderState( D3DRENDERSTATE_TEXTUREMIN, D3DFILTER_LINEAR );
		//lpD3DDevice->SetRenderState( D3DRENDERSTATE_TEXTUREMAG, D3DFILTER_LINEAR );
	}	
	//lpD3DDevice->SetRenderState( D3DRENDERSTATE_CULLMODE , D3DCULL_CW );

	return TRUE;
}

void CGuideHelp::GuidHelpEnd()
{
	IsDrawGuideHelp = FALSE;
	m_GuideHelpN = -1;
	bDoing = FALSE;
	isDrawClanMenu = 0;
	TJBscrollWheelClear();
	TJBscrollWheelClear_Guide();
}


