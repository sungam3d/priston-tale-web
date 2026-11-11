/*----------------------------------------------------------------------------*
*	파일명 :  sinInterFace.cpp	
*	하는일 :  인터페이스에 관한 모든걸 관리한다 
*	작성일 :  최종업데이트 12월
*	적성자 :  박상열 
*-----------------------------------------------------------------------------*/	

#include "sinLinkHeader.h"
#include "..\\tjboy\\clanmenu\\tjclan.h"
#include "..\\field.h"
/*----------------------------------------------------------------------------*
*								전역변수 			
*-----------------------------------------------------------------------------*/	
cINTERFACE	cInterFace;
sSHADOWSTATE sShadowState[3];

int sinBmpLength[5] = {0,0,0,0,0}; //라이프 마나 스테미나 경험치 

////////////////채팅관련 Flag
int sinChatEnter = 0;         //채팅 플랙 (엔터키를 입력하면 채팅 창이 나온다 )
int SubChatHeight = 0;	      //채팅창 변경 위치 좌표 값 	
int sinChatDisplayMode = 0;   //현재 채팅의 모드 
int sinChatInputMode = 0;     //채팅 입력 모드 
////////////////

/////////////Test Key Msg
int TestKeyAniFlag = 0;

////////////////버튼관련 Flag
int ExitButtonClick = 0;
int HelpButtonClick = 0;
int MouseButtonPosi=0;
int sinMenuBottonPosi = 0;
int sinRunCameraMapPosi = 0; //걷기 카메라 맵 
int SunandMoonFlag[2] = {0,0}; //0 해 1 달 
int InterButtonDown = 0;  //인터페이스 버튼 다운 플랙 
int sinFireShow = 1;      //불을 보여준다 

////////////////스킬 게이지 (현재는 사용되고 있지 않다 )
int ReSizelpGage = 0;     //게이지 그림을 조정한다 


////////////////
int InitStateFlag = 0;    //초기화 플랙 


//////////////// 백업 포인터 
char szInfoMsgBuff5[512];  //생명력 기력 근력 경험치 의 수치 기록 버퍼 
char szInfoMsgBuff6[128];  //생명력 기력 근력 경험치 의 수치 기록 버퍼 
POINT InfoMessageBoxPos;  //텍스를 표시할 좌표 


//////////////// 전사 (다시시작 )
int ReStartFlag = 0;  //값이 들어오면 다시시작 인터페이스를 보여준다 
POINT ReStartMainXY = {0,0}; //다시시작 좌표 
int ReStartIndex = 0;
POINT ReStartTextXY = {0,0};
char szReStartMsg[128];
char szReStartMsg2[128];

int RestartCheckExp = 0;
int ReStartOptionIndex = 0;
int ShowExpPercentFlag = 0;

//////////////// 스킬 전업 종료  (다시시작 )

int SkillNpcFlag = 0;  //값이 들어오면 다시시작 인터페이스를 보여준다 
POINT SkillMasterMainXY = {0,0}; //다시시작 좌표 
int SkillMasterIndex = 0;
POINT SkillMasterTextXY = {0,0};
char szSkillMasterMsg[128];
char szSkillMasterMsg2[128];
int SkillMasterOptionIndex = 0;

////////스킬 사용 불충족 이펙트 
int NotUseSkillEffect[3] = {0,0,0};

int SkillMouseButtonPosi[2] = {0,0};

/////////// 낮 밤
int DayOrNightFlag = 0;
int sinGageTime2 = 0;


int ReStartButtonRect[3][4] = {
	{21,22,21+44,22+44},			//필드에서 시작 
	{72,22,72+44,22+44},			//마을에서 시작 
	{123,22,123+44,22+44},			//게임 종료 
	
};

int ButtonRect[6][5]= {   //박스 오픈 버튼 
	{648,560,648+25,560+27},
	{673,560,673+25,560+27},
	{698,560,698+25,560+27},
	{723,560,723+25,560+27},
	{748,560,748+25,560+27},
	{772,560,772+25,560+27},

};
/*
int ButtonRect[6][5]= {   //박스 오픈 버튼 
	{655,566,678,598},
	{678,566,701,598},
	{701,566,724,598},
	{724,566,747,598},
	{747,566,770,598},
	{770,566,792,598},

};
*/
int sinMenuRect[3][4] = {  //게임 종료 ....
	{692,492,784,511},
	{692,517,784,536},
	{692,542,784,561}, //종료 


};

int sinRunCameraMap[3][4] = { //걷기 , 카메라 , 맵 
	{569,555,569+26,555+26},
	{569+26,555,569+26+26,555+26},
	{569+26+26,555,569+26+26+26,555+26},

};


// 전업 관련 
int ChangeJobButtonclick = 0;


//PK관련
int sinPKWarningExpFlag = 0;
int sinPKWarnigExpTime = 0;
LPDIRECTDRAWSURFACE4 lpGrowInterLeft;
LPDIRECTDRAWSURFACE4 lpGrowInterMiddle;
LPDIRECTDRAWSURFACE4 lpGrowInterRight;


//글자 이펙트관련 ===================================================================
struct HAEFFECT{
	int   Flag;
	int   Time;
	POINT Posi;
	POINT Size;
	int   Mat;
	int   Alpha;
};
char *HaEffectFilePath[] = {
	0,
	"Image\\SinImage\\Inter\\defense.tga", //1
	"Image\\SinImage\\Inter\\block.tga",   //2
	"Image\\SinImage\\Inter\\Evade.tga",   //3
	0,
};
#define  MAX_HAEFFECTNUM 10 //총갯수

int HA_EffectIndex[4]={0,}; //

HAEFFECT HaEffect[MAX_HAEFFECTNUM];
//=====================================================================================

/*----------------------------------------------------------------------------*
*							클래스 초기, 종료 
*-----------------------------------------------------------------------------*/	
cINTERFACE::cINTERFACE()
{
	memset(&HaEffect,0,sizeof(HAEFFECT)); //글자이펙트  초기화 
}
cINTERFACE::~cINTERFACE()
{


}
/*----------------------------------------------------------------------------*
*							     초기화 
*-----------------------------------------------------------------------------*/	
void cINTERFACE::Init()
{

	//ReStart메인 의 좌표를 구한다 
	ReStartMainXY.x = (WinSizeX/2) - (187/2);
	ReStartMainXY.y = (WinSizeY/2) - (127/2)-100;

	ReStartTextXY.x = ReStartMainXY.x+23;
	ReStartTextXY.y = ReStartMainXY.y+73;

	SkillMasterMainXY.x = (WinSizeX/2) - (187/2)-30;
	SkillMasterMainXY.y = (WinSizeY/2) - (127/2)-100;

	SkillMasterTextXY.x = SkillMasterMainXY.x+23;
	SkillMasterTextXY.y = SkillMasterMainXY.y+73;



	InitState(); //생명력 , 기력 , 근력을 초기화한다 

	//MatMain = CreateTextureMaterial( "Image\\SinImage\\Inter\\Inter.tga" , 0, 0, 0,0, SMMAT_BLEND_ALPHA );

	MatMenu[0] = CreateTextureMaterial( "Image\\SinImage\\Inter\\Menu-1.tga" , 0, 0, 0,0, SMMAT_BLEND_ALPHA );
	MatMenu[1] = CreateTextureMaterial( "Image\\SinImage\\Inter\\Menu-2.tga" , 0, 0, 0,0, SMMAT_BLEND_ALPHA );

	MatChatDefault[0] = CreateTextureMaterial( "Image\\SinImage\\Inter\\Chat\\ChatDefault-1.tga" , 0, 0, 0,0, SMMAT_BLEND_ALPHA );//채팅기본 
	MatChatDefault[1] = CreateTextureMaterial( "Image\\SinImage\\Inter\\Chat\\ChatDefault-2.tga" , 0, 0, 0,0, SMMAT_BLEND_ALPHA );//채팅기본 
	
	MatChatSmall[0] = CreateTextureMaterial( "Image\\SinImage\\Inter\\Chat\\ChatSmall-1.tga" , 0, 0, 0,0, SMMAT_BLEND_ALPHA );  //새끼 채팅창
	MatChatSmall[1] = CreateTextureMaterial( "Image\\SinImage\\Inter\\Chat\\ChatSmall-2.tga" , 0, 0, 0,0, SMMAT_BLEND_ALPHA );  //새끼 채팅창				

	MatChatDoc[0] =	CreateTextureMaterial( "Image\\SinImage\\Inter\\Chat\\ChatDoc-1.tga" , 0, 0, 0,0, SMMAT_BLEND_ALPHA );  //내용 창 
	MatChatDoc[1] = CreateTextureMaterial( "Image\\SinImage\\Inter\\Chat\\ChatDoc-2.tga" , 0, 0, 0,0, SMMAT_BLEND_ALPHA );  //내용 창 

	
	MatExitMenu = CreateTextureMaterial( "Image\\SinImage\\Inter\\ExitMenu.tga" , 0, 0, 0,0, SMMAT_BLEND_ALPHA );  //내용 창 


	MatClock        = CreateTextureMaterial("Image\\SinImage\\Inter\\Clock.tga", 0, 0, 0,0, SMMAT_BLEND_ALPHA); //시계 
	MatComPass      = CreateTextureMaterial("Image\\SinImage\\Inter\\Compass.tga", 0, 0, 0,0, SMMAT_BLEND_ALPHA); //나침반 

	MatLife			= CreateTextureMaterial("Image\\SinImage\\Inter\\ShadowLife.tga", 0, 0, 0,0, SMMAT_BLEND_ALPHA); //시계 
	MatMana			= CreateTextureMaterial("Image\\SinImage\\Inter\\ShadowMana.tga", 0, 0, 0,0, SMMAT_BLEND_ALPHA); //시계 
	MatStamina		= CreateTextureMaterial("Image\\SinImage\\Inter\\ShadowStamina.tga", 0, 0, 0,0, SMMAT_BLEND_ALPHA); //시계 


	Load();

	
}
/*----------------------------------------------------------------------------*
*						    Bmp파일을 로드한다 
*-----------------------------------------------------------------------------*/	
void cINTERFACE::Load()
{
	lpLIFE   =  LoadDibSurfaceOffscreen( "Image\\SinImage\\Inter\\Bar_Life.bmp" );
	lpMP     =  LoadDibSurfaceOffscreen( "Image\\SinImage\\Inter\\Bar_Mana.bmp" );
	lpSTM    =  LoadDibSurfaceOffscreen( "Image\\SinImage\\Inter\\Bar_Stamina.bmp" );

	lpButton[0] = LoadDibSurfaceOffscreen( "Image\\SinImage\\Inter\\bStatus.bmp" ); //무브 이미지 
	lpButton[1] = LoadDibSurfaceOffscreen( "Image\\SinImage\\Inter\\bInvenTory.bmp" );
	lpButton[2] = LoadDibSurfaceOffscreen( "Image\\SinImage\\Inter\\bSkill.bmp" );
	lpButton[3] = LoadDibSurfaceOffscreen( "Image\\SinImage\\Inter\\bParty.bmp" );
	lpButton[4] = LoadDibSurfaceOffscreen( "Image\\SinImage\\Inter\\bQuest.bmp" );
	lpButton[5] = LoadDibSurfaceOffscreen( "Image\\SinImage\\Inter\\bSystem.bmp" );

	lpButton2[0] = LoadDibSurfaceOffscreen( "Image\\SinImage\\Inter\\bStatus-1.bmp" ); //클릭 이미지 
	lpButton2[1] = LoadDibSurfaceOffscreen( "Image\\SinImage\\Inter\\bInvenTory-1.bmp" );
	lpButton2[2] = LoadDibSurfaceOffscreen( "Image\\SinImage\\Inter\\bSkill-1.bmp" );
	lpButton2[3] = LoadDibSurfaceOffscreen( "Image\\SinImage\\Inter\\bParty-1.bmp" );
	lpButton2[4] = LoadDibSurfaceOffscreen( "Image\\SinImage\\Inter\\bQuest-1.bmp" );
	lpButton2[5] = LoadDibSurfaceOffscreen( "Image\\SinImage\\Inter\\bSystem-1.bmp" );

	lpInfoBox[0] = LoadDibSurfaceOffscreen( "Image\\SinImage\\Inter\\ButtonInfo\\StatusInfo.bmp" ); //풍선 도움말
	lpInfoBox[1] = LoadDibSurfaceOffscreen( "Image\\SinImage\\Inter\\ButtonInfo\\InvenInfo.bmp" ); //풍선 도움말
	lpInfoBox[2] = LoadDibSurfaceOffscreen( "Image\\SinImage\\Inter\\ButtonInfo\\SkillInfo.bmp" ); //풍선 도움말
	lpInfoBox[3] = LoadDibSurfaceOffscreen( "Image\\SinImage\\Inter\\ButtonInfo\\PartyInfo.bmp" ); //풍선 도움말
	lpInfoBox[4] = LoadDibSurfaceOffscreen( "Image\\SinImage\\Inter\\ButtonInfo\\QuestInfo.bmp" ); //풍선 도움말
	lpInfoBox[5] = LoadDibSurfaceOffscreen( "Image\\SinImage\\Inter\\ButtonInfo\\SystemInfo.bmp" ); //풍선 도움말


	lpMenuButton[0] = LoadDibSurfaceOffscreen( "Image\\SinImage\\Inter\\Option.bmp" ); //옵션 
	lpMenuButton[1] = LoadDibSurfaceOffscreen( "Image\\SinImage\\Inter\\Replay.bmp" ); //다시시작 
	lpMenuButton[2] = LoadDibSurfaceOffscreen( "Image\\SinImage\\Inter\\Exit.bmp" ); //나가기


	lpWalk = LoadDibSurfaceOffscreen( "Image\\SinImage\\Inter\\Button\\Walk.bmp" ); //걷기 

	lpChatShowButton = LoadDibSurfaceOffscreen( "Image\\SinImage\\Inter\\Chat\\ChatDownButton.bmp" ); //채팅버튼  
	

	lpSun  = LoadDibSurfaceOffscreen( "Image\\SinImage\\Inter\\Flash\\Sun.bmp" ); //해 
	lpMoon = LoadDibSurfaceOffscreen( "Image\\SinImage\\Inter\\Flash\\Moon.bmp" ); //달 

	lpGage[0] = LoadDibSurfaceOffscreen( "Image\\SinImage\\Skill\\P-skill.bmp" );  //왼쪽  스킬게이지
	lpGage[1] = LoadDibSurfaceOffscreen( "Image\\SinImage\\Skill\\P-skill2.bmp" );  //오른쪽 스킬게이지

	lpBar_Anger		= LoadDibSurfaceOffscreen( "Image\\SinImage\\Inter\\sinGage\\Bar_Anger.bmp" );  //게이지 전체  
	lpBar_Time		= LoadDibSurfaceOffscreen( "Image\\SinImage\\Inter\\sinGage\\Bar_Time.bmp" );  //게이지 전체  
	lpBar_Exp		= LoadDibSurfaceOffscreen( "Image\\SinImage\\Inter\\sinGage\\Bar_Exp.bmp" );  //게이지 전체  
	lpButton_Anger	= LoadDibSurfaceOffscreen( "Image\\SinImage\\Inter\\sinGage\\Button_Anger.bmp" );  //게이지 전체   
	
	lpRunInfo		= LoadDibSurfaceOffscreen( "Image\\SinImage\\Inter\\ButtonInfo\\Run.bmp");  //뛰기풍선 도움말 
	lpWalkInfo      = LoadDibSurfaceOffscreen( "Image\\SinImage\\Inter\\ButtonInfo\\Walk.bmp"); //걷기풍선 도움말 
	
	lpCameraHandInfo= LoadDibSurfaceOffscreen( "Image\\SinImage\\Inter\\ButtonInfo\\Camera_Hand.bmp");   //수동 카메라 풍선 도움말 
	lpCameraFixInfo = LoadDibSurfaceOffscreen( "Image\\SinImage\\Inter\\ButtonInfo\\Camera_Fix.bmp");   //수동 카메라 풍선 도움말 
	lpCameraAutoInfo= LoadDibSurfaceOffscreen( "Image\\SinImage\\Inter\\ButtonInfo\\Camera_Auto.bmp");   //자동 카메라 풍선 도움말 
	
	lpMapOnInfo		= LoadDibSurfaceOffscreen( "Image\\SinImage\\Inter\\ButtonInfo\\MapOn.bmp");   //자동 카메라 풍선 도움말 
	lpMapOffInfo	= LoadDibSurfaceOffscreen( "Image\\SinImage\\Inter\\ButtonInfo\\MapOff.bmp");   //수동 카메라 풍선 도움말 

	lpMapOnImage		= LoadDibSurfaceOffscreen( "Image\\SinImage\\Inter\\Button\\MapOnImage.bmp");  //맵표시 온 이미지 
	lpCameraImage[0]	= LoadDibSurfaceOffscreen( "Image\\SinImage\\Inter\\Button\\AutoCameraImage.bmp"); //카메라 Auto이미지 
	lpCameraImage[1]	= LoadDibSurfaceOffscreen( "Image\\SinImage\\Inter\\Button\\PixCameraImage.bmp"); //카메라 고정 이미지

    lpLIFESpot		= LoadDibSurfaceOffscreen( "Image\\SinImage\\Inter\\Bar_LifeSpot.bmp");        //라이프 
    lpSTMSpot		= LoadDibSurfaceOffscreen( "Image\\SinImage\\Inter\\Bar_StaminaSpot.bmp");        //스테미나 
    lpMANASpot		= LoadDibSurfaceOffscreen( "Image\\SinImage\\Inter\\Bar_ManaSpot.bmp");        //분노게이지 

	lpReStartMain		= LoadDibSurfaceOffscreen( "Image\\SinImage\\Inter\\ReStart\\ReStartMain.bmp"); //다시시작 메인 
	lpReStartButton[0]	= LoadDibSurfaceOffscreen( "Image\\SinImage\\Inter\\ReStart\\ReStart_Field.bmp"); //다시시작 버튼 
	lpReStartButton[1]	= LoadDibSurfaceOffscreen( "Image\\SinImage\\Inter\\ReStart\\ReStart_Town.bmp"); //다시시작 버튼 
	lpReStartButton[2]	= LoadDibSurfaceOffscreen( "Image\\SinImage\\Inter\\ReStart\\ReStart_Exit.bmp"); //다시시작 버튼 


	lpSelectSkill_Main[0]     = LoadDibSurfaceOffscreen( "Image\\SinImage\\Inter\\SkillIcon\\G_Select_Skill.bmp"); //스킬 배우기 0 그레이 1 활성 
	lpSelectSkill_Main[1]     = LoadDibSurfaceOffscreen( "Image\\SinImage\\Inter\\SkillIcon\\Select_Skill.bmp"); //스킬 배우기 0 그레이 1 활성 
	
	lpSelectChangeJob[0] = LoadDibSurfaceOffscreen( "Image\\SinImage\\Inter\\SkillIcon\\G_Select_ChangeJob.bmp"); //전업        0 그레이 2 활성 
	lpSelectChangeJob[1] = LoadDibSurfaceOffscreen( "Image\\SinImage\\Inter\\SkillIcon\\Select_ChangeJob.bmp"); //전업        0 그레이 2 활성 

	lpTime7 = LoadDibSurfaceOffscreen( "Image\\SinImage\\Inter\\SkillIcon\\Select_ChangeJob.bmp"); //시간 게이지

	lpGrowInterLeft = LoadDibSurfaceOffscreen( "Image\\SinImage\\Inter\\inter_01.bmp"); //확장 인터페이스
	lpGrowInterMiddle = LoadDibSurfaceOffscreen( "Image\\SinImage\\Inter\\inter_02.bmp"); 
	lpGrowInterRight = LoadDibSurfaceOffscreen( "Image\\SinImage\\Inter\\inter_03.bmp"); 


}
/*----------------------------------------------------------------------------*
*						     Release
*-----------------------------------------------------------------------------*/	
void cINTERFACE::Release()
{
	int i;

	if(lpLIFE){
		lpLIFE->Release();
		lpLIFE = 0;

	}
	if(lpMP){
		lpMP->Release();
		lpMP = 0;
	}
	if(lpSTM){
		lpSTM->Release();
		lpSTM = 0;
	}

	for(i = 0 ; i < 6 ; i++){
		if(lpButton[i]){
			lpButton[i]->Release();
			lpButton[i] = 0;

		}
		if(lpButton2[i]){
			lpButton2[i]->Release();
			lpButton2[i] = 0;
		}
		if(lpInfoBox[i]){
			lpInfoBox[i]->Release();
			lpInfoBox[i] = 0;
		}
	}
	for(i=0 ; i < 3 ; i++){
		if(lpMenuButton[i]){
			lpMenuButton[i]->Release();
			lpMenuButton[i] = 0;
		}
	}

	if(lpWalk){
		lpWalk->Release();
		lpWalk = 0;	
	}
	if(lpChatShowButton){
		lpChatShowButton->Release();
		lpChatShowButton = 0;
	}	
	if(lpSun){
		lpSun->Release();
		lpSun = 0;

	}
	if(lpMoon){
		lpMoon->Release();
		lpMoon = 0;
	}
	if(lpGage[0]){
		lpGage[0]->Release();
		lpGage[0] = 0;
	}
	if(lpBar_Anger){
		lpBar_Anger->Release();
		lpBar_Anger = 0;
	}
	if(lpBar_Time){
		lpBar_Time->Release();
		lpBar_Time = 0;
	}
	if(lpBar_Exp){
		lpBar_Exp->Release();
		lpBar_Exp = 0;
	}
	if(lpButton_Anger){
		lpButton_Anger->Release();
		lpButton_Anger = 0;
	}

	if(lpRunInfo){
		lpRunInfo->Release();
		lpRunInfo = 0;
	}
	if(lpWalkInfo){
		lpWalkInfo->Release();
		lpWalkInfo = 0;
	}
	
	if(lpCameraAutoInfo){
		lpCameraAutoInfo->Release();
		lpCameraAutoInfo  = 0;
	}
	if(lpCameraHandInfo){
		lpCameraHandInfo->Release();
		lpCameraHandInfo = 0;
	}
	if(lpCameraFixInfo){
		lpCameraFixInfo->Release();
		lpCameraFixInfo = 0;
	}

	if(lpMapOnInfo){
		lpMapOnInfo->Release();
		lpMapOnInfo = 0;
	}
	if(lpMapOffInfo){
		lpMapOffInfo->Release();
		lpMapOffInfo = 0;
	}
	if(lpMapOnImage){
		lpMapOnImage->Release();
		lpMapOnImage = 0;
	}
	if(lpCameraImage[0]){
		lpCameraImage[0]->Release();
		lpCameraImage[0] = 0;
	}
	if(lpCameraImage[1]){
		lpCameraImage[1]->Release();
		lpCameraImage[1] = 0;
	}

	if(lpLIFESpot){
		lpLIFESpot->Release();
		lpLIFESpot = 0;
	}
    if(lpSTMSpot){
		lpSTMSpot->Release();
		lpSTMSpot = 0;
	}
    if(lpMANASpot){
		lpMANASpot->Release();
		lpMANASpot = 0;
	}
	if(lpReStartMain){
		lpReStartMain->Release();
		lpReStartMain = 0;

	}

	for(i=0; i < 3 ; i++){
		if(lpReStartButton[i]){
			lpReStartButton[i]->Release();
			lpReStartButton[i] = 0;
		}
	}
	if(lpReStartMain){
		lpReStartMain->Release();
		lpReStartMain = 0;
		
	}
	for(i=0; i < 3 ; i++){	
		if(lpReStartButton[i]){
			lpReStartButton[i]->Release();
			lpReStartButton[i] = 0;
			
		}
	}
	for(i=0; i < 2 ; i++){	
		if(lpSelectSkill_Main[i]){
			lpSelectSkill_Main[i]->Release();
			lpSelectSkill_Main[i] = 0;

		}

		if(lpSelectChangeJob[i] ){
			lpSelectChangeJob[i]->Release();
			lpSelectChangeJob[i] = 0;
	
		}
	}
}		
/*----------------------------------------------------------------------------*
*							     그리기  
*-----------------------------------------------------------------------------*/	
int sinWinSizeX[] = {1024,1280,1600};
int sinWinSizeXindex[] = {2,6,11};
int nCheckSkillDraw = 0;		//해외
void cINTERFACE::Draw()
{

	int i = 0; 
	int j = 0;
	//dsDrawTexImage( MatMain , 0,  0, 256, 128 , 255 );  //인터페이스 메인 

	dsDrawTexImage( cInterFace.MatMenu[0] , 800-256-256, 600-128, 256, 128 , 255 );  //인터페이스 메인      
	dsDrawTexImage( cInterFace.MatMenu[1] , 800-256, 600-64, 256, 64 , 255 );      //인터페이스 메인 

	//확장 인터페이스 
	DrawSprite(800,WinSizeY-64,lpGrowInterLeft,0,0,66,64,777);  // 확장인터페이스 플렉         

	for(i = 0; i < 3; i++){
		if(sinWinSizeX[i] == WinSizeX){
			for(j = 0; j < sinWinSizeXindex[i] ; j++){
				DrawSprite(800+66+(64*j),WinSizeY-64,lpGrowInterMiddle,0,0,64,64,777);
			}
		}
	}

	DrawSprite(800+66+(64*j),WinSizeY-64,lpGrowInterRight,0,0,40,64,777);


	//DrawSprite(100,200,lpGrowInterLeft,0,0,80,80);

	///////////////////////잔상효과 
	for(i = 0 ; i < 3 ; i++){
		if(sShadowState[i].Flag){
			dsDrawColorBox( sShadowState[i].Color ,sShadowState[i].Posi.x,sShadowState[i].Posi.y,
				sShadowState[i].Size.x, sShadowState[i].Size.y );

		}
	}

	DrawSprite(319,500+(94-sinBmpLength[0]),lpLIFE,0,(94-sinBmpLength[0]),16,94 ); //라이프        
	DrawSprite(465,500+(94-sinBmpLength[1]),lpMP  ,0,(94-sinBmpLength[1]), 16,94); //마나   
	DrawSprite(303,518+(76-sinBmpLength[2]),lpSTM ,0,(76-sinBmpLength[2]), 8,76);  //스테미나 


//	DrawSprite(319,500+(94-sinBmpLength[0]),lpLIFE,0,0,16, sinBmpLength[0]); //라이프   
//	DrawSprite(465,500+(94-sinBmpLength[1]),lpMP  ,0,0, 16,sinBmpLength[1]); //마나   
//	DrawSprite(303,518+(76-sinBmpLength[2]),lpSTM ,0,0, 8,sinBmpLength[2]);  //스테미나 

	/*
	if(sinBmpLength[0] <= 123 && sinBmpLength[0]  !=0) //라이프 
		DrawSprite(111+sinBmpLength[0]-4,4 ,lpLIFESpot,0,0, 4,17); //라이프 꼭다리 
	*/
/*
	if(sinChatEnter){
		dsDrawTexImage( MatChatDoc[0] , 0, 600-32-SubChatHeight, 256, 32 , 255 );    //채팅창내용  
		dsDrawTexImage( MatChatDoc[1] , 256, 600-32-SubChatHeight, 128, 32 , 255 );  
	}

	if(ChatFlag){	
		if(SubChatHeight){
				dsDrawTexImage( MatChatSmall[0] , 0, 600-152-128-21, 256, 128 , 255 );     //새끼  채팅창 
				dsDrawTexImage( MatChatSmall[1] , 256, 600-152-128-21, 128, 128 , 255 );   //새끼  채팅창 
			}
			else{
				dsDrawTexImage( MatChatDefault[0] , 0, 600-256, 256, 256 , 255 );   //기본 채팅창 
				dsDrawTexImage( MatChatDefault[1] , 256, 600-256, 128, 256 , 255 ); //기본 채팅창 
			}
		
	}
*/ 
	if(ExitButtonClick){    
		dsDrawTexImage( MatExitMenu , 698, 504, 128, 64 , 255 );     //나가기 창   

		if(sinMenuBottonPosi ==1 )
			DrawSprite(698,504,lpMenuButton[0],0,0,96,19); //헬프  

		if(sinMenuBottonPosi ==2 )
			DrawSprite(692,517,lpMenuButton[1],0,0,93,20); //다시시작  

		if(sinMenuBottonPosi ==3 )
			DrawSprite(698,535,lpMenuButton[2],0,0,96,19); //나가기 


	}

	//채팅커서 표시 
	if ( hFocusWnd && sinChatInputMode && !usehFocus) {   
	//if ( hFocusWnd && sinChatInputMode ) {  
		DrawSprite(7,580-SubChatHeight,lpChatShowButton,0,0,22,19); //채팅
	}
//	if(sinChatEnter){
//		DrawSprite(7,579-SubChatHeight,lpChatShowButton,0,0,22,19); //채팅
//	}

	if(DayOrNightFlag ==1){ //낮     
		DrawSprite(363,589,lpSun,0,0,13,13); //해  
		DrawSprite(375,593,lpBar_Time,0,0,sinGageTime2,5); //게이지 
	}
	else{	//밤
		DrawSprite(426,589,lpMoon,0,0,13,13); //달 
		DrawSprite(375+(50-sinGageTime2),593,lpBar_Time,(50-sinGageTime2),0,50,5); //게이지 

	}

	/*
	//해와달 
	if(SunandMoonFlag[0]) 
		DrawSprite(12,62,lpSun,0,0,12,12); //해 

	if(SunandMoonFlag[1])
		DrawSprite(57,7,lpMoon,0,0,12,12); //달
	*/


	int TempGage = 0;

	/////////////스킬 게이지 
	if(sinSkill.pLeftSkill){		 //왼쪽 스킬 
		TempGage = (int)((41.0f*(float)sinSkill.pLeftSkill->GageLength)/35.0f);
		DrawSprite(338 ,542+(41-TempGage), lpGage[0],0,41-TempGage,16,41); 
	}

	if(sinSkill.pRightSkill){		//오른쪽 스킬 
		TempGage = (int)((41.0f*(float)sinSkill.pRightSkill->GageLength)/35.0f);
		DrawSprite(446 ,542+(41-TempGage), lpGage[1],0,41-TempGage,16,41); 

	}

	/*
	if(SelectInterSkill){  //인터페이스 스킬 선택 
		DrawSprite(sLeftRightSkill[SelectInterSkill-1].BoxRect.left-3,
			sLeftRightSkill[SelectInterSkill-1].BoxRect.top-3,cSkill.lpSelectSkill,0,0,44,44);

	}
	*/

	
	//분노 게이지 (임시)
	//DrawSprite(82,18,lpBar_Anger,0,0,27,40-ReSizelpGage); //게이지 

	//분노 버튼 
	//DrawSprite(82,18,lpButton_Anger,0,0,27,40-ReSizelpGage); //게이지 

	//경험치 바를 깜박거린다 
	if(sinPKWarningExpFlag){    
		if(sinPKWarnigExpTime <= 15)
			DrawSprite(485,508+(86-sinBmpLength[3]),lpBar_Exp,0,(86-sinBmpLength[3]),6,86 ); //경험치        


	}
	else
		DrawSprite(485,508+(86-sinBmpLength[3]),lpBar_Exp,0,(86-sinBmpLength[3]),6,86 ); //경험치        


	//시계
	//DrawSprite(505,588,lpBar_Time,0,0,sinBmpLength[4],5); //게이지 

	if(!sInterFlags.RunFlag) //달리기가 아니면 
		DrawSprite(575,565,lpWalk,0,0,24,25); //걷기 
	
	if(sInterFlags.CameraAutoFlag == 1) //카메라 자동이면 
		DrawSprite(575+24,565,lpCameraImage[0],0,0,24,25);

	if(sInterFlags.CameraAutoFlag == 2) //카메라 고정이면
		DrawSprite(575+24,565,lpCameraImage[1],0,0,24,25);


	if(sInterFlags.MapOnFlag)
	{
		DrawSprite(575+24+24,565,lpMapOnImage,0,0,24,25);
	}

	switch(sinRunCameraMapPosi){
	case 1:
		if(sInterFlags.RunFlag){//달리기면 
			DrawSprite(575+12-(76/2),565-29,lpRunInfo,0,0,77,27);
		}
		else{ //걷기이면 
			DrawSprite(575+12-(76/2),565-29,lpWalkInfo,0,0,77,27);
			
		}
	break;
	case 2:
		if(sInterFlags.CameraAutoFlag == 0){ //카메라 수동 
			DrawSprite(575+24+12-(76/2),565-29,lpCameraHandInfo,0,0,77,27);
			cHelpPet.PetMessage("*Camera_Hand",0);
		}
		if(sInterFlags.CameraAutoFlag == 2){ //카메라 고정 
			DrawSprite(575+24+12-(76/2),565-29,lpCameraFixInfo,0,0,77,27);
			cHelpPet.PetMessage("*Camera_Fix",0);

		}
		if(sInterFlags.CameraAutoFlag ==1 ){//카메라 자동 
			DrawSprite(575+26+13-(76/2),565-29,lpCameraAutoInfo,0,0,77,27);
			cHelpPet.PetMessage("*Camera_Auto",0);
			
		}
	break;
	case 3:
		if(sInterFlags.MapOnFlag){ //맵보여주기 
			DrawSprite(575+24+24+12-(76/2),565-29,lpMapOffInfo,0,0,77,27);
			
		}
		else{  //맵숨기기 
			DrawSprite(575+24+24+12-(76/2),565-29,lpMapOnInfo,0,0,77,27);

		}
	break;

	}

	for(int t = 0 ; t < 6 ; t++){  
		if(MouseButtonPosi == t+1){
			if(InterButtonDown){
				DrawSprite(648+(t*25),560,lpButton2[t],0,0,25,27); //캐릭터 스테이터스

			}
			else{
				DrawSprite(648+(t*25),560,lpButton[t],0,0,25,27); //캐릭터 스테이터스
				DrawSprite(648-(77-23-1)+(t*25),560-27,lpInfoBox[t],0,0,77,27); //캐릭터 스테이터스
			}
		}
	}
/*

	switch(MouseButtonPosi){
	case 1:
		if(InterButtonDown)
			DrawSprite(651,567,lpButton2[0],0,0,24,25); //캐릭터 스테이터스
		else{
			DrawSprite(651,567,lpButton[0],0,0,24,25); //캐릭터 스테이터스
			DrawSprite(651-(77-23-1),567-27,lpInfoBox[0],0,0,77,27); //캐릭터 스테이터스
			
		}
	break;

	case 2:
		if(InterButtonDown)
			DrawSprite(678+1,576+1,lpButton2[1],0,0,,24,25); //인벤토리 
		else {
			DrawSprite(678+1,576+1,lpButton[1],0,0,24,25); //인벤토리 
			DrawSprite(678+1-(77-23-1),576+1-27,lpInfoBox[1],0,0,77,27); //인벤토리 
		}
	break;

	case 3:
		if(InterButtonDown)
			DrawSprite(701+1,576+1,lpButton2[2],0,0,24,25); //스킬  
		else {
			DrawSprite(701+1,576+1,lpButton[2],0,0,24,25); //스킬  
			DrawSprite(701+1-(77-23-2),576+1-27,lpInfoBox[2],0,0,77,27); //스킬  
		}
	break;

	case 4:
		if(InterButtonDown)
			DrawSprite(724+1,576+1,lpButton2[3],0,0,24,25); //파티
		else{
			DrawSprite(724+1,576+1,lpButton[3],0,0,24,25); //파티
			DrawSprite(724+1-(77-23-2),576+1-27,lpInfoBox[3],0,0,77,27); //파티
		}
	break;

	case 5:
		if(InterButtonDown)
			DrawSprite(747+1,576+1,lpButton2[4],0,0,24,25); //퀘스트
		else{ 
			DrawSprite(747+1,576+1,lpButton[4],0,0,24,25); //퀘스트
			DrawSprite(747+1-(77-23-1),576+1-27,lpInfoBox[4],0,0,77,27); //퀘스트
		}
	break;

	case 6:
		if(InterButtonDown)
			DrawSprite(770+1,576+1,lpButton2[5],0,0,24,25); //시스템 

		else{
			DrawSprite(770+1,576+1,lpButton[5],0,0,24,25); //시스템 
			DrawSprite(770+1-(77-23-1),576+1-27,lpInfoBox[5],0,0,77,27); //시스템 
		}
	break;

	}
*/

	//DrawClockArrow(61,57,1000);                   //시계바늘을 그린다 
	//DrawCompass(63,52,lpCurPlayer->Angle.y&ANGCLIP);  //나침반을 그린다 


	if(SkillNpcFlag){
		DrawSprite(SkillMasterMainXY.x,SkillMasterMainXY.y,lpReStartMain,0,0,188,128); //스킬 전업 메인 
		DrawSprite(SkillMasterMainXY.x+ReStartButtonRect[0][0],SkillMasterMainXY.y+ReStartButtonRect[0][1],lpSelectSkill_Main[0],0,0,44,44); 
		DrawSprite(SkillMasterMainXY.x+ReStartButtonRect[1][0],SkillMasterMainXY.y+ReStartButtonRect[1][1],lpSelectChangeJob[0],0,0,44,44); 
		if(SkillMasterIndex){
			if(SkillMasterIndex == 1 && SkillMasterOkFlag[0])//스킬 
				DrawSprite(SkillMasterMainXY.x+ReStartButtonRect[SkillMasterIndex-1][0],SkillMasterMainXY.y+ReStartButtonRect[SkillMasterIndex-1][1],lpSelectSkill_Main[1],0,0,44,44); 
			if(SkillMasterIndex == 2 && SkillMasterOkFlag[1])//전업 
				DrawSprite(SkillMasterMainXY.x+ReStartButtonRect[SkillMasterIndex-1][0],SkillMasterMainXY.y+ReStartButtonRect[SkillMasterIndex-1][1],lpSelectChangeJob[1],0,0,44,44); 
			if(SkillMasterIndex == 3)//종료 
				DrawSprite(SkillMasterMainXY.x+ReStartButtonRect[SkillMasterIndex-1][0],SkillMasterMainXY.y+ReStartButtonRect[SkillMasterIndex-1][1],lpReStartButton[2],0,0,44,44); 

		}
	}
	
	////////스킬버튼 닫기 (인터페이스에 가려있는 관계로 이곳으로 이동)
	if(ShowSkillClose){ //닫기 버튼 
		DrawSprite(269,571+(256-sinMoveKindInter[SIN_SKILL]),cShop.lpExit,0,0,20,20);
		DrawSprite(269,571-27+(256-sinMoveKindInter[SIN_SKILL]),cInvenTory.lpExitInfo,0 ,0 , 47,27);
	}
	//이펙트글자를 보여준다.=====================================================================
	
	for(int k=0;k<10;k++){
		if(HaEffect[k].Flag){
			dsDrawTexImage(HaEffect[k].Mat, HaEffect[k].Posi.x,HaEffect[k].Posi.y,HaEffect[k].Size.x,
				HaEffect[k].Size.y,HaEffect[k].Alpha );  //인터페이스 메인   
		}
	}
	//============================================================================================



	/////////////////////////////////// 채팅 관련 ////////
	lpD3DDevice->EndScene();  
	HDC hdc;

	char strBuff2[256];
	char strBuff[256];
	int len=0;
	int x=0,y=0;

	lpDDSBack->GetDC( &hdc );

	SelectObject( hdc , hFont ); 
    SetBkMode( hdc, TRANSPARENT );

	char *lpInput;


	if ( hFocusWnd && sinChatInputMode && !usehFocus) {     
	//if ( hFocusWnd && sinChatInputMode ) { 
		//DrawSprite(7,582-SubChatHeight,lpChatShowButton,0,0,22,19); //채팅

		GetWindowText( hTextWnd , strBuff2 , 240 );
		strBuff2[242]=0x0D;
		len = lstrlen( strBuff2 );

		SetBkMode( hdc, TRANSPARENT );
		
		//wsprintf( strBuff, "▶ %s" , strBuff2 );
		wsprintf( strBuff, "  %s" , strBuff2 );

		len=lstrlen( strBuff );       
		if ( len>=44 ) {
			lpInput = &strBuff[len-44];
			len=44;
		}
		else 
			lpInput = strBuff;

		if ( sinChatInputMode==1 ) {
			x = 16 ; y = 584-SubChatHeight;
		}
		else {
			x = 16 ; y = 584-SubChatHeight;
		}
		
		SetTextColor( hdc, RGB(0, 0, 0) );

#ifdef _LANGUAGE_VEITNAM
		dsTextLineOut( hdc , x+6, y , lpInput , len );
		SetTextColor( hdc, RGB(255, 255, 255) );
		dsTextLineOut( hdc , x+5 , y-1 , lpInput , len );
#else
		dsTextLineOut( hdc , x+1, y+1 , lpInput , len );
		SetTextColor( hdc, RGB(255, 255, 255) );
		dsTextLineOut( hdc , x , y , lpInput , len );
#endif
		
	}
	
#include "..\\language.h"
#ifdef _LANGUAGE_JAPANESE
#include <imm.h>

		if ( ImeChatLen!=len || ImePositon.x!=len || ImePositon.y!=y ) {
			HIMC	h=ImmGetContext(hwnd);
			COMPOSITIONFORM	cf;

			int addPoint = 0;

			if(smConfig.ScreenSize.y == 600) addPoint = 0;
			else if(smConfig.ScreenSize.y == 768) addPoint = 163;
			else if(smConfig.ScreenSize.y == 1024) addPoint = 420;

			if ( h ) {
				cf.dwStyle = CFS_FORCE_POSITION;
				cf.ptCurrentPos.x = x+6*len;
				cf.ptCurrentPos.y = y + addPoint;
				ImmSetCompositionWindow( h, &cf );

				ImePositon.x = len;
				ImePositon.y = y;
				ImeChatLen = len;
			}
		}
#endif


	if ( sinChatDisplayMode ) {                   
		if ( sinChatDisplayMode==1 ) {
			if(NoticeChatIndex == 1){ //공지채팅말

				//공지채팅
				x = 16 ; y = 570-SubChatHeight-125; i = 3;
				DisplayChatMessage3( hdc , x , y , 44 , i , 5 );

				//일반채팅
				x = 16 ; y = 570-SubChatHeight-20; i = 8;
				DisplayChatMessage2( hdc , x , y , 44 , i );

			}
			else if(NoticeChatIndex == 2){
				x = 16 ; y = 570-SubChatHeight-20; i = 12;
				DisplayChatMessage3( hdc , x , y , 44 , i , 5 );
			}
			else{
				x = 16 ; y = 570-SubChatHeight-20; i = 12;
				DisplayChatMessage2( hdc , x , y , 44 , i );
			}
		}
		else {
			if(NoticeChatIndex == 2){
				x = 16 ; y = 570-SubChatHeight-5; i = 8;
				DisplayChatMessage3( hdc , x , y , 44 , i , 5 );
			}
			else{
				x = 16 ; y = 570-SubChatHeight-5; i = 8;
				DisplayChatMessage2( hdc , x , y , 44 , i );
			}
		}
		//채팅 정보 화면에 표시
		//if(NoticeChatIndex != 2 )
		//	DisplayChatMessage2( hdc , x , y , 44 , i );
	}

	lpDDSBack->ReleaseDC( hdc );

	int dsDrawOffsetArrayBackup = dsDrawOffsetArray;

	dsDrawOffsetArray = dsARRAY_TOP;

////////////////////// 스킬 
	if(SkillMasterFlag == 1 ){ 
		for(i = 0; i < SkillUpInfo.x ; i++){
			for(int j = 0; j< SkillUpInfo.y ; j++){
				if(i == 0 && j == 0 ) //좌측상단 
					dsDrawTexImage( cItem.MatItemInfoBox_TopLeft , SkillUpInfoPosi.x+(i*16) , SkillUpInfoPosi.y+(j*16), 16, 16 , 255 );
				if(j == 0 && i !=0 && i+1 < SkillUpInfo.x ) //가운데
					dsDrawTexImage( cItem.MatItemInfoBox_TopCenter , SkillUpInfoPosi.x+(i*16) , SkillUpInfoPosi.y+(j*16), 16, 16 , 255 );
				if(j == 0 && i+1 == SkillUpInfo.x) //우측상단
					dsDrawTexImage( cItem.MatItemInfoBox_TopRight , SkillUpInfoPosi.x+(i*16) , SkillUpInfoPosi.y+(j*16), 16, 16 , 255 );

				if(i == 0 && j != 0 && j+1 != SkillUpInfo.y) //좌측 줄거리 
					dsDrawTexImage( cItem.MatItemInfoBox_Left , SkillUpInfoPosi.x+(i*16) , SkillUpInfoPosi.y+(j*16), 16, 16 , 255 );
				if(i != 0 && j != 0 && i+1 !=SkillUpInfo.x && j+1 !=SkillUpInfo.y) //가운데 토막
					dsDrawTexImage( cItem.MatItemInfoBox_Center , SkillUpInfoPosi.x+(i*16) , SkillUpInfoPosi.y+(j*16), 16, 16 , 255 );
				if(i+1 == SkillUpInfo.x && j != 0 && j+1 != SkillUpInfo.y) //우측 줄거리 
					dsDrawTexImage( cItem.MatItemInfoBox_Right , SkillUpInfoPosi.x+(i*16) , SkillUpInfoPosi.y+(j*16), 16, 16 , 255 );
				
				if(i == 0 && j+1 == SkillUpInfo.y) //밑바닥 왼쪽
					dsDrawTexImage( cItem.MatItemInfoBox_BottomLeft , SkillUpInfoPosi.x+(i*16) , SkillUpInfoPosi.y+(j*16), 16, 16 , 255 );
				if(i != 0 && j+1 == SkillUpInfo.y && i+1 !=SkillUpInfo.x)//밑바닥 가운데
					dsDrawTexImage( cItem.MatItemInfoBox_BottomCenter , SkillUpInfoPosi.x+(i*16) , SkillUpInfoPosi.y+(j*16), 16, 16 , 255 );
				if(j+1 == SkillUpInfo.y && i+1 ==SkillUpInfo.x)//밑바닥 오른쪽 
					dsDrawTexImage( cItem.MatItemInfoBox_BottomRight , SkillUpInfoPosi.x+(i*16) , SkillUpInfoPosi.y+(j*16), 16, 16 , 255 );


			}
		}
		
		if(ShowSkillUpInfo){
			for( i = 0 ; i < 10 ; i ++ ) {
				if(	SkillIconIndex[i]){
					DrawSprite(SkillUpInfoPosi.x+SkillIconPosi.x+(i*18)-10,SkillUpInfoPosi.y+SkillIconPosi.y,cSkill.lpWeaponIcon[SkillIconIndex[i]],0,0,18,24); 
				}
			}

		}
		
		lpD3DDevice->EndScene();

		HDC	hdc;

		int BoldFlag = 0;
		int MasteryLen = 0;
		int MasteryShowX = 0;

		char strBuff[128];
		memset(strBuff,0,sizeof(strBuff));
		lpDDSBack->GetDC( &hdc );
		SelectObject( hdc, sinFont);
		SetBkMode( hdc, TRANSPARENT );
		SetTextColor( hdc, RGB(255,255,255) );

		if(SkillMasterFlag == 1 && !ShowSkillUpInfo){
			SelectObject( hdc, sinBoldFont);
			SetBkMode( hdc, TRANSPARENT );
			SetTextColor( hdc, RGB(222,231,255) );
			dsTextLineOut(hdc,100,225-80,SkillUseInfoText[0]   , lstrlen(SkillUseInfoText[0]) );
			
			//스킬포인트를 찍어준다.
			if(sinSkill.SkillPoint||sinSkill.SkillPoint4){
				SelectObject( hdc, sinFont);
				SetBkMode( hdc, TRANSPARENT );
				SetTextColor( hdc, RGB(255,255,255) );

				dsTextLineOut(hdc,100,260-80,SkillUseInfoText[1], lstrlen(SkillUseInfoText[1]) );
				dsTextLineOut(hdc,100,280-80,SkillUseInfoText[2], lstrlen(SkillUseInfoText[2]) );
				wsprintf(strBuff,sinMsgSkillPoint,sinSkill.SkillPoint);
				SelectObject( hdc, sinBoldFont);
				SetBkMode( hdc, TRANSPARENT );
				SetTextColor( hdc, RGB(247,243,193) );
				dsTextLineOut(hdc,100,310-80,strBuff   , lstrlen(strBuff) );

				wsprintf(strBuff,sinMsgEliteSkillPoint,sinSkill.SkillPoint4);  
				SelectObject( hdc, sinBoldFont);
				SetBkMode( hdc, TRANSPARENT );
				SetTextColor( hdc, RGB(147,243,193) );
				dsTextLineOut(hdc,103,310-55,strBuff   , lstrlen(strBuff) );
			}
			else{
				SelectObject( hdc, sinFont);
				SetBkMode( hdc, TRANSPARENT );
				SetTextColor( hdc, RGB(255,64,64) );

				dsTextLineOut(hdc,100,260-80,SkillUseInfoText[3], lstrlen(SkillUseInfoText[3]) );
				dsTextLineOut(hdc,100,280-80,SkillUseInfoText[4], lstrlen(SkillUseInfoText[4]) );

			}


		}
		lpDDSBack->ReleaseDC( hdc );
		lpD3DDevice->BeginScene();
	}

	//전업 버튼
	if(SkillMasterFlag && !ShowSkillUpInfo && (CheckQuestButtonShow1 ||CheckQuestButtonShow2 ||CheckQuestButtonShow3) )    //스킬정보를 보여주는 옵션이 아닐경우에만 전업 버튼을 띄운다 
	{
		if(sinChangeJobButtonShow || sinChangeJobButtonShow2 || sinChangeJobButtonShow3 || sinChangeJobButtonShow4 )
		{
			DrawSprite(240,380,lpSelectChangeJob[1],0,0,44,44); 
			nCheckSkillDraw = 1;
		}
		else
		{
			DrawSprite(240,380,lpSelectChangeJob[0],0,0,44,44); 
			nCheckSkillDraw = 2;
		}
	}

	if(SkillMasterFlag && !ShowSkillUpInfo && (nCheckSkillDraw == 1))
	{
		DrawSprite(240,380,lpSelectChangeJob[1],0,0,44,44); 
	}

	if(SkillMasterFlag && !ShowSkillUpInfo && (nCheckSkillDraw == 2))
	{
		DrawSprite(240,380,lpSelectChangeJob[0],0,0,44,44); 
	}

	dsDrawOffsetArray = dsDrawOffsetArrayBackup;

	/////////////////////스킬 정보 박스 
	if(SkillInfoShowFlag && !ShowSkillUpInfo){ 
		if(SkillInfoShowFlag == 2)
			dsDrawOffsetArray = dsARRAY_TOP;
		for(i = 0; i < SkillBoxSize.x ; i++){
			for(int j = 0; j< SkillBoxSize.y ; j++){
				if(i == 0 && j == 0 ) //좌측상단 
					dsDrawTexImage( cItem.MatItemInfoBox_TopLeft , SkillInfoBoxPosi.x+(i*16) , SkillInfoBoxPosi.y+(j*16), 16, 16 , 255 );
				if(j == 0 && i !=0 && i+1 < SkillBoxSize.x ) //가운데
					dsDrawTexImage( cItem.MatItemInfoBox_TopCenter , SkillInfoBoxPosi.x+(i*16) , SkillInfoBoxPosi.y+(j*16), 16, 16 , 255 );
				if(j == 0 && i+1 == SkillBoxSize.x) //우측상단

					dsDrawTexImage( cItem.MatItemInfoBox_TopRight , SkillInfoBoxPosi.x+(i*16) , SkillInfoBoxPosi.y+(j*16), 16, 16 , 255 );

				if(i == 0 && j != 0 && j+1 != SkillBoxSize.y) //좌측 줄거리 
					dsDrawTexImage( cItem.MatItemInfoBox_Left , SkillInfoBoxPosi.x+(i*16) , SkillInfoBoxPosi.y+(j*16), 16, 16 , 255 );
				if(i != 0 && j != 0 && i+1 !=SkillBoxSize.x && j+1 !=SkillBoxSize.y) //가운데 토막
					dsDrawTexImage( cItem.MatItemInfoBox_Center , SkillInfoBoxPosi.x+(i*16) , SkillInfoBoxPosi.y+(j*16), 16, 16 , 255 );
				if(i+1 == SkillBoxSize.x && j != 0 && j+1 != SkillBoxSize.y) //우측 줄거리 
					dsDrawTexImage( cItem.MatItemInfoBox_Right , SkillInfoBoxPosi.x+(i*16) , SkillInfoBoxPosi.y+(j*16), 16, 16 , 255 );
				
				if(i == 0 && j+1 == SkillBoxSize.y) //밑바닥 왼쪽
					dsDrawTexImage( cItem.MatItemInfoBox_BottomLeft , SkillInfoBoxPosi.x+(i*16) , SkillInfoBoxPosi.y+(j*16), 16, 16 , 255 );
				if(i != 0 && j+1 == SkillBoxSize.y && i+1 !=SkillBoxSize.x)//밑바닥 가운데
					dsDrawTexImage( cItem.MatItemInfoBox_BottomCenter , SkillInfoBoxPosi.x+(i*16) , SkillInfoBoxPosi.y+(j*16), 16, 16 , 255 );
				if(j+1 == SkillBoxSize.y && i+1 ==SkillBoxSize.x)//밑바닥 오른쪽 
					dsDrawTexImage( cItem.MatItemInfoBox_BottomRight , SkillInfoBoxPosi.x+(i*16) , SkillInfoBoxPosi.y+(j*16), 16, 16 , 255 );


			}
		}
		for( i = 0 ; i < 10 ; i ++ ) {
			if(	SkillIconIndex[i]){
				DrawSprite(SkillInfoBoxPosi.x+SkillIconPosi.x+(i*18),SkillInfoBoxPosi.y+SkillIconPosi.y,cSkill.lpWeaponIcon[SkillIconIndex[i]],0,0,18,24); 

			}
		}
	}
	///////// 생명력, 경험치 등등의 글씨를 잘보이게해줄 배경 
	if(InfoMessageBoxPos.x != 0 || InfoMessageBoxPos.y != 0 ){ //표시위치에 갔을때만 표시한다   
		dsDrawTexImage( cItem.MatItemInfoBox_Center , InfoMessageBoxPos.x-5,InfoMessageBoxPos.y-1, lstrlen(szInfoMsgBuff5)*6, 13 , 255 );
	}

	//경험치 뒷배경은 이제 즐~ (인터페이스 바뀜)
	//if(ShowExpPercentFlag)  
	//	dsDrawTexImage( cItem.MatItemInfoBox_Center , 489-5,543-1, lstrlen(szInfoMsgBuff6)*6, 13 , 255 );

 
	//스킬 초기화 박스를 띄운다 
	cSkill.ShowClearSkillPointButton(SkillInfoBoxPosi.x + (SkillBoxSize.x*16) , SkillInfoBoxPosi.y + (SkillBoxSize.y*16));

    lpD3DDevice->BeginScene();	

}
/*----------------------------------------------------------------------------*
*							     메인  
*-----------------------------------------------------------------------------*/	
DWORD dwCheckTime; //시간을 체크한다 
int   CheckNotUseSkillTime[3] = {0,0,0};
int   CheckNotUseSkillCount[3] = {0,0,0};
int   CheckTime2[3] ={0,0,0};
int   CheckExpFlag = 0;
int   CheckExpFlag2 = 0;
int   CheckExpTime = 0;

DWORD dwTestKeyTime = 0;
int   TeskKeyMessageCnt = 0;
char *szTestKeyMsg[6] = {"\263\252!  &(^_^)&    ",
						 "\263\353!  /(^_^)/   ",
						 "\263\252!  *(^_^)*  ",
						 "",
						 "",
						 "",};
					
void cINTERFACE::Main()
{
	if ( GetFieldMapCursor()==TRUE && cInterFace.sInterFlags.MapOnFlag ) {
		cHelpPet.PetMessage("*Mini_Map",0);
	}


	/////////////////// 채팅 (이미지 때문에 최상위로 빼다 음하하) 
	if(sinChatEnter){       

		/*
		if(SubChatHeight){                 
			dsDrawTexImage( cInterFace.MatChatDoc[0] , 4, 576-SubChatHeight, 242, 32 , 255 );    //채팅창내용  
			dsDrawTexImage( cInterFace.MatChatDoc[1] , 246, 576-SubChatHeight, 164, 32 , 255 );    //채팅창내용  
		}
		else
			dsDrawTexImage( cInterFace.MatChatDoc[0] , 4, 576-SubChatHeight, 296, 32 , 255 );    //채팅창내용  
		*/

	}

	

	if(cInterFace.ChatFlag){	         
		if(SubChatHeight){  
			if (0< pCursorPos.x  && 300 > pCursorPos.x  && 
					294 < pCursorPos.y  && 294+116 > pCursorPos.y  ){
						if(!ChatKindIndexNum){
							cHelpPet.PetMessage("*ChatBox",0);
						}

			}
		}
		else{
			if (0< pCursorPos.x  && 305 > pCursorPos.x  && 
					600-192 < pCursorPos.y  && 600-192+210 > pCursorPos.y  ){
						if( !ChatKindIndexNum){
							cHelpPet.PetMessage("*ChatBox",0);
						}

			}
		}
	}

	//Pet 메세지
	if (365< pCursorPos.x  && 435 > pCursorPos.x  && 
			589 < pCursorPos.y  && 596 > pCursorPos.y  ){
				cHelpPet.PetMessage("*DayandNight",0);

	}


	GetWordTimeDisplay(); //시계를 그린다 
	//PK시 경험치 경고를 준다 
	
	if(sinPKWarningExpFlag){ 
		sinPKWarnigExpTime++;
		if(sinPKWarnigExpTime > 30)
			sinPKWarnigExpTime = 0;

	}
	else
		sinPKWarnigExpTime = 0;


	//////////////테스트 키관련
	if(TestKeyAniFlag){
		dwTestKeyTime++;
		if(dwTestKeyTime > 30){
			dwTestKeyTime = 0;
			SendChatMessageToServer(szTestKeyMsg[TeskKeyMessageCnt]);
			TeskKeyMessageCnt++;
			if(TeskKeyMessageCnt >= 6){
				TestKeyAniFlag = 0;
				dwTestKeyTime  = 0;
				TeskKeyMessageCnt = 0;

			}
		}
	}


	sinMenuBottonPosi = 0;
	
	if ( lpCurPlayer->MotionInfo->State == CHRMOTION_STATE_DEAD && lpCurPlayer->FrameCounter>100 ) {
		ReStartFlag =1;
		if(!CheckExpFlag && !CheckExpFlag2){
			if(!CheckFieldContExp())
				RestartCheckExp = 1;

			CheckExpFlag = 1;
		}

	}
	if(CheckExpFlag2){ //땜빵 
		CheckExpTime++;
		if(CheckExpTime > 100){
			CheckExpFlag2 = 0;
			CheckExpTime = 0;

		}
	}

	if(dwGameHour >= 8 && dwGameHour <=18){
		SunandMoonFlag[0] = 1;
		SunandMoonFlag[1] = 0;
	}
	else{
		SunandMoonFlag[0] = 0;
		SunandMoonFlag[1] = 1;

	}
	if(sinLoadOK){
		if(!InitStateFlag)InitState();
		CheckingState();
	}

	MouseButtonPosi = 0;
	int i; for( i =0;i<6;i++){
		if ( ButtonRect[i][0] < pCursorPos.x  && ButtonRect[i][2] > pCursorPos.x  && 
			ButtonRect[i][1] < pCursorPos.y  && ButtonRect[i][3] > pCursorPos.y  ){
			MouseButtonPosi = i+1;
			switch(MouseButtonPosi){
				case 1:
					cHelpPet.PetMessage("*CarStatus",0);
				break;
				case 2:
					cHelpPet.PetMessage("*InvenTory",0);
				break;
				case 3:
					cHelpPet.PetMessage("*Skill",0);
				break;
				case 4:
					cHelpPet.PetMessage("*Party",0);
				break;
				case 5:
					cHelpPet.PetMessage("*Quest",0);
				break;
				case 6:
					cHelpPet.PetMessage("*System",0);
				break;

			}
		}
	}
	sinRunCameraMapPosi = 0; //초기화 
	for(i=0 ;i<3 ; i++){
		if ( sinRunCameraMap[i][0] < pCursorPos.x  && sinRunCameraMap[i][2] > pCursorPos.x  && 
			sinRunCameraMap[i][1] < pCursorPos.y  && sinRunCameraMap[i][3] > pCursorPos.y  ){
			sinRunCameraMapPosi = i+1;
			switch(sinRunCameraMapPosi){
				case 1:
					cHelpPet.PetMessage("*RunAndWalk",0);
				break;
				case 2:
					//cHelpPet.PetMessage("*RunAndWalk",0);
				break;
				case 3:
					//cHelpPet.PetMessage("*Mini_Map",0);
				break;

			}
		}
	}

	if(ExitButtonClick){
		if ( 699 < pCursorPos.x  && 795 > pCursorPos.x  && 
			 535 < pCursorPos.y  && 555 > pCursorPos.y  )
			 sinMenuBottonPosi = 3;

		if ( 698 < pCursorPos.x  && 795 > pCursorPos.x  && 
			 504 < pCursorPos.y  && 524 > pCursorPos.y  )
			 sinMenuBottonPosi = 1;
		
		

	

		///////////////옵션및  다시시작이 없는관계로 날치기 땜빵 
		
		/*
		for(i=0 ;i<3 ; i++)
			if ( sinMenuRect[i][0] < pCursorPos.x  && sinMenuRect[i][2] > pCursorPos.x  && 
				 sinMenuRect[i][1] < pCursorPos.y  && sinMenuRect[i][3] > pCursorPos.y  )
				sinMenuBottonPosi = i+1;
		*/
			
	}

	////////////////////  채팅관련 (복잡하니까 안건드린다)
	if(cInvenTory.OpenFlag || cCharStatus.OpenFlag || cSkill.OpenFlag){
		SubChatHeight = 170;
		sinChatInputMode =0;
		sinChatDisplayMode = 0;
		if(SubChatHeight>=170){
			SubChatHeight = 170;

		}

	}
	else
		SubChatHeight =0;

	if(sinChatEnter){
		if(SubChatHeight >=170)
			sinChatInputMode = 2;
		else 
			sinChatInputMode = 1;

	}
	else
		sinChatInputMode = 0;

	if(ChatFlag){
		if(SubChatHeight >=170)
			sinChatDisplayMode = 2;
		else 
			sinChatDisplayMode = 1;

	}
	else
		sinChatDisplayMode = 0;

	//////////////////////채팅관련 Flag 끝 

	CheckingState(); // 생명력 ,기력 , 근력의 변화치를 체크해서 현재치만큼 Bmp를 보여준다 
	ShowParaState(); // 생명력 기력 근력 경험치 수치를 표시한다 
	ShowExpPercent(); //경험치는 계속적으로 보여준다


	///////////////////// 전사 (게임 다시시작 필드 , 마을 , 종료 )
	ReStartIndex = 0; //초기화 
	memset(szReStartMsg,0,lstrlen(szReStartMsg)); //초기화 
	memset(szReStartMsg2,0,lstrlen(szReStartMsg2)); //초기화 

	if(ReStartFlag){
		sinMenuBottonPosi = 0;
		for(i=0 ;i<3 ; i++){
			if ( ReStartMainXY.x+ReStartButtonRect[i][0] < pRealCursorPos.x  && ReStartMainXY.x+ReStartButtonRect[i][2] > pRealCursorPos.x  && 
				ReStartMainXY.y+ReStartButtonRect[i][1] < pRealCursorPos.y  && ReStartMainXY.y+ReStartButtonRect[i][3] > pRealCursorPos.y  ){
				if(RestartCheckExp){ //경험치가 부족하여 필드에서 시작할수없는 겨우 
					if( i ==0 ){
						ReStartIndex = i+1;
						lstrcpy(szReStartMsg,ReStartMsg[3]);
						lstrcpy(szReStartMsg2,ReStartMsg2[3]);

					}
					else{
						ReStartIndex = i+1;
						lstrcpy(szReStartMsg,ReStartMsg[i]);
						lstrcpy(szReStartMsg2,ReStartMsg2[i]);
					}

				}
				else if(i ==0&&lpCurPlayer->OnStageField>=0 && StageField[lpCurPlayer->OnStageField]->State==FIELD_STATE_CASTLE){
					ReStartIndex=0;
				}
				else{
					ReStartIndex = i+1;
					lstrcpy(szReStartMsg,ReStartMsg[i]);
					lstrcpy(szReStartMsg2,ReStartMsg2[i]);
				}

			}
		}
		dwCheckTime++;
		if(dwCheckTime > 70*20){ //10초가 넘어가면 
			ReStartFlag = 0; //플랙 초기화 
			ReStartOptionIndex = RESTART_TOWN; //타운에서 시작한다 
			CheckExpFlag = 0;
			CheckExpFlag2 = 1;
			RestartCheckExp = 0; //플랙들을 초기화한다
		}
	
	}
	else
		dwCheckTime = 0; //플랙이 없으면 초기화한다 
	///////////////////// 스킬 , 전업 
	SkillMasterIndex = 0; //초기화 
	memset(szSkillMasterMsg,0,lstrlen(szSkillMasterMsg)); //초기화 
	memset(szSkillMasterMsg2,0,lstrlen(szSkillMasterMsg2)); //초기화 

	/////////키워드 스킬 
	if(SkillNpcFlag){
		
		sinMenuBottonPosi = 0;
		for(i=0 ;i<3 ; i++){
			if ( SkillMasterMainXY.x+ReStartButtonRect[i][0] < pCursorPos.x  && SkillMasterMainXY.x+ReStartButtonRect[i][2] > pCursorPos.x  && 
				SkillMasterMainXY.y+ReStartButtonRect[i][1] < pCursorPos.y  && SkillMasterMainXY.y+ReStartButtonRect[i][3] > pCursorPos.y  ){
				SkillMasterIndex = i+1;
				lstrcpy(szSkillMasterMsg,SkillMaster[i]);
				lstrcpy(szSkillMasterMsg2,SkillMaster2[i]);

			}
		}
	}
	//스킬 충족요건 부족시 효과 
	for(int t = 0; t < 3 ; t++){
		if(NotUseSkillElement[t]){
			CheckNotUseSkillTime[t]++;
			if(CheckNotUseSkillTime[t] > 12 ){
				if(NotUseSkillEffect[t])NotUseSkillEffect[t] = 0;
				else NotUseSkillEffect[t] = 1;
				CheckNotUseSkillTime[t] = 0;
				CheckNotUseSkillCount[t]++;
				if(CheckNotUseSkillCount[t] > 4){
					CheckNotUseSkillCount[t] = 0;
					CheckNotUseSkillTime[t] = 0;
					NotUseSkillEffect[t] = 0;
					NotUseSkillElement[t] = 0;
					SkillMouseButtonPosi[0] = 0;
					SkillMouseButtonPosi[1] = 0;


				}

			}
		}
		if(NotUseSkillEffect[t]){
			CheckTime2[t]++;	//게이지가 안보인다고 해서 보정해줬다 (날치기로)

		}
		else
			CheckTime2[t] = 0;
		if(CheckTime2[t] > 20) //여기서 보정해준다 
			NotUseSkillEffect[t] = 0;


	}
	
	//이펙트글자를 보여준다====================================================
	for(int k = 0 ; k < MAX_HAEFFECTNUM ; k++){      
		if(HaEffect[k].Flag){ 
			if(HaEffect[k].Time<3000)
				HaEffect[k].Time+=20;
			
			switch(HaEffect[k].Time/255){
				case 0:
					HaEffect[k].Alpha=255;
				break;
				case 1:
					HaEffect[k].Alpha=255;
				break;
				case 2:	
				case 3:
				case 4:
				case 5:
				case 6:
					//글자가 사라지는 효과
					HaEffect[k].Alpha-=10;
					HaEffect[k].Posi.x-=1;
					HaEffect[k].Posi.y-=1; 
					HaEffect[k].Size.x+=3;
					HaEffect[k].Size.y+=3;
					if(HaEffect[k].Alpha<=0){
						HaEffect[k].Alpha=0;
						HaEffect[k].Time=0;
						HaEffect[k].Flag=0;
					}
				break;
			};
		}
	}
	//===========================================================================

}
/*----------------------------------------------------------------------------*
*							     종료 
*-----------------------------------------------------------------------------*/	
void cINTERFACE::Close()
{
	Release();
}
/*----------------------------------------------------------------------------*
*							   LButtonDown
*-----------------------------------------------------------------------------*/	
void cINTERFACE::LButtonDown(int x , int y)
{
	
	switch(sinRunCameraMapPosi){
		case 1:
			//cHelpPet.PetMessage("*RunAndWalk",1);
		break;
		case 3:
			//cHelpPet.PetMessage("*Mini_Map",1);
		break;
	}
	/*

	if(sInterFlags.CameraAutoFlag == 0){ //카메라 수동 
		cHelpPet.PetMessage("*Camera_Hand",1);
	}
	if(sInterFlags.CameraAutoFlag == 2){ //카메라 고정 
		cHelpPet.PetMessage("*Camera_Fix",1);
	}
	if(sInterFlags.CameraAutoFlag ==1 ){//카메라 자동 
		cHelpPet.PetMessage("*Camera_Auto",1);
	}

	*/

	//Pet 메세지
	//PetMessage
	if (365< pCursorPos.x  && 435 > pCursorPos.x  &&  
			589 < pCursorPos.y  && 596 > pCursorPos.y  ){
				cHelpPet.PetMessage("*DayandNight",1);

	}

	//Pet 메세지
	if(pCursorPos.x > 315 && pCursorPos.x < 337 && pCursorPos.y > 500 && pCursorPos.y < 594){//라이프 
		cHelpPet.PetMessage("*Bar_Life",1); 
	}
	if(pCursorPos.x > 463 && pCursorPos.x < 483 && pCursorPos.y > 498 && pCursorPos.y < 595){//기력
		cHelpPet.PetMessage("*Bar_Mana",1);
	}
	if(pCursorPos.x > 300 && pCursorPos.x < 313 && pCursorPos.y > 513 && pCursorPos.y < 595){//근력
		cHelpPet.PetMessage("*Bar_Stamina",1);
	}	
	if(pCursorPos.x > 483 && pCursorPos.x < 483+10 && pCursorPos.y > 513 && pCursorPos.y < 595){//경험치
			cHelpPet.PetMessage("*Bar_Exp",1);
	}	
	if(pCursorPos.x > 500 && pCursorPos.x < 570 && pCursorPos.y > 540 && pCursorPos.y < 594){//경험치 
		cHelpPet.PetMessage("*Exp_Num",1);
	}
	if ( GetFieldMapCursor()==TRUE && cInterFace.sInterFlags.MapOnFlag ) {
		cHelpPet.PetMessage("*Mini_Map",1); 

	}




	if(cInterFace.ChatFlag){	         
		if(SubChatHeight){ 
			if (0< pCursorPos.x  && 300 > pCursorPos.x  && 
					294 < pCursorPos.y  && 294+116 > pCursorPos.y  ){
						if(!ChatKindIndexNum){
							cHelpPet.PetMessage("*ChatBox",1);
						}

			}
		}
		else{
			if (0< pCursorPos.x  && 305 > pCursorPos.x  && 
					600-192 < pCursorPos.y  && 600-192+210 > pCursorPos.y  ){
						if(!ChatKindIndexNum){
							cHelpPet.PetMessage("*ChatBox",1);
						}

			}
		}
	}

	SkillMouseButtonPosi[0] = 1;
	ChangeJobButtonclick = 0;
	ReStartOptionIndex = 0; //인덱스 초기화 
	//채팅창올리기 
	if(x > 7 && 22 > x && y > 579-SubChatHeight && y < 579+19-SubChatHeight){ 
		if(!ChatFlag)ChatFlag = 1;
		else ChatFlag = 0;
		if(ChatFlag)sinChatEnter = 1;
	}
	switch(sinRunCameraMapPosi){
	case 1: //걷기 달리기 
		if(sInterFlags.RunFlag)sInterFlags.RunFlag = 0;
		else sInterFlags.RunFlag =1;
	break;
	case 2: //카메라 자동 수동 
		sInterFlags.CameraAutoFlag++;
		if(sInterFlags.CameraAutoFlag > 2)sInterFlags.CameraAutoFlag = 0;

	break;
	case 3: //오토맵 
		if(sInterFlags.MapOnFlag)sInterFlags.MapOnFlag = 0;
		else sInterFlags.MapOnFlag = 1;

	break;
	
	};

	if(sinMenuBottonPosi == 3){ //게임을 종료한다 
			QuitGame();
	}

	char *HelpFilePath = "image\\Sinimage\\Help\\Help.sin"; 
	if(sinMenuBottonPosi == 1){ //도움말을 띄운다 (호동프린스가 -50해달란다)
			cSinHelp.sinShowHelp(SIN_HELP_KIND_NORMAL,208,61-50,11,11, RGBA_MAKE(19,27,20,165),HelpFilePath); 
			ExitButtonClick = 0; 
			HelpBoxKindIndex = SIN_HELP;
	}

	if(MouseButtonPosi){ //버튼포지션에서 마우스 버튼이 클릭됬을때 
		InterButtonDown = 1;
		switch(MouseButtonPosi){ //PetMessage
			case 1:
				cHelpPet.PetMessage("*CarStatus",1);
			break;
			case 2:
				cHelpPet.PetMessage("*InvenTory",1);
			break;
			case 3:
				cHelpPet.PetMessage("*Skill",1);
			break;
			case 4:
				cHelpPet.PetMessage("*Party",1);
			break;
			case 5:
				cHelpPet.PetMessage("*Quest",1);
			break;
			case 6:
				cHelpPet.PetMessage("*System",1);
			break;
		}

	}
	if(ReStartFlag){    //전사 옵션 버튼 
		switch(ReStartIndex){
		case RESTART_TOWN:
			ReStartOptionIndex = RESTART_TOWN;
			ReStartFlag = 0;
			RestartCheckExp = 0; //초기화해준다 
		
		break;
		case RESTART_FEILD:
			if(RestartCheckExp)break;
			ReStartOptionIndex = RESTART_FEILD;
			ReStartFlag = 0;
			RestartCheckExp = 0; //초기화해준다 
		
		break;
		case RESTART_EXIT:
			ReStartOptionIndex = RESTART_EXIT;
			ReStartFlag = 0;
			RestartCheckExp = 0; //초기화해준다 

		break;

		}

		CheckExpFlag2 = 1;
		CheckExpFlag = 0;

	}
	//스킬 , 전업 
	if(SkillMasterIndex){

		if(SkillMasterIndex==1 &&  SkillMasterOkFlag[0]){
			SkillMasterFlag = SkillMasterIndex; //스킬 
			SkillNpcFlag = 0;
			cSkill.OpenFlag = 1;
			cInterFace.CheckAllBox(SIN_SKILL); //창을 하나만 띄운다 

		}
		if(SkillMasterIndex==2 &&  SkillMasterOkFlag[1]){
			SkillMasterFlag = SkillMasterIndex; //전업 
			SkillNpcFlag = 0;
			////////////////////// 메세지 박스를 띄워준다 
			sinJobList = GetJobDataCode(sinChar->JOB_CODE , sinChar->ChangeJob+1);
			cMessageBox.ShowMessage3(MESSAGE_CHANGE_JOB,sinJobList->szName2);


		}

		if(SkillMasterIndex==3){
			SkillMasterFlag = SkillMasterIndex; //종료  
			SkillNpcFlag = 0;
		}
	}

}
/*----------------------------------------------------------------------------*
*							   LButtonUp
*-----------------------------------------------------------------------------*/	
void cINTERFACE::LButtonUp(int x , int y)
{
	if(MouseButtonPosi){
		switch(MouseButtonPosi){
		case 1: //캐릭터 스테이터스 
			if(cCharStatus.OpenFlag)cCharStatus.OpenFlag = SIN_CLOSE;
			else cCharStatus.OpenFlag = SIN_OPEN;
			cInterFace.CheckAllBox(SIN_CHARSTATUS); //창을 하나만 띄운다 
			if(cCharStatus.OpenFlag){
				if(!sinFireShow){
					StartMenuFlame(0,350);
					sinPlaySound( 0 );
				
				}
				else
					sinPlaySound(SIN_SOUND_SHOW_INTER);
				sinFireShow = 1;
			}

		break;
		case 2: //인벤토리 
			if(cInvenTory.OpenFlag)cInvenTory.OpenFlag = SIN_CLOSE;
			else cInvenTory.OpenFlag = SIN_OPEN;
			cInterFace.CheckAllBox(SIN_INVENTORY); //창을 하나만 띄운다 
			if(cInvenTory.OpenFlag){
				if(!sinFireShow){
					StartMenuFlame(0,350);
					sinPlaySound( 0 );
				}
				else
					sinPlaySound(SIN_SOUND_SHOW_INTER);

				sinFireShow = 1;
			}

		break;
		case 3: //스킬 

			if(cSkill.OpenFlag == SIN_CLOSE)cSkill.OpenFlag = SIN_OPEN;
			else cSkill.OpenFlag = SIN_CLOSE;
			cInterFace.CheckAllBox(SIN_SKILL); //창을 하나만 띄운다 
			if(cSkill.OpenFlag){
				if(!sinFireShow){
					StartMenuFlame(0,350);
					sinPlaySound( 0 );
				}
				else
					sinPlaySound(SIN_SOUND_SHOW_INTER);

				sinFireShow = 1;
			}
			
		break;

		case 4:
			ShowParty();

		break;

		case 5:
			ShowQuest();
			
		break;

		case 6:
			if(!ExitButtonClick)ExitButtonClick =1;
			else ExitButtonClick = 0;
			cInterFace.CheckAllBox(SIN_SYSTEM); //창을 하나만 띄운다 

		break;
		}

	}

	InterButtonDown = 0; //버튼업일때는 초기화 


}
/*----------------------------------------------------------------------------*
*							   RButtonDown
*-----------------------------------------------------------------------------*/	
void cINTERFACE::RButtonDown(int x , int y)
{

	if ( GetFieldMapCursor()==TRUE && cInterFace.sInterFlags.MapOnFlag ) {
		cHelpPet.PetMessage("*Mini_Map",2);

	}


	if(MouseButtonPosi){ //버튼포지션에서 마우스 버튼이 클릭됬을때 
		switch(MouseButtonPosi){ //PetMessage
			case 1:
				cHelpPet.PetMessage("*CarStatus",2);
			break;
			case 2:
				cHelpPet.PetMessage("*InvenTory",2);
			break;
			case 3:
				cHelpPet.PetMessage("*Skill",2);
			break;
			case 4:
				cHelpPet.PetMessage("*Party",2);
			break;
			case 5:
				cHelpPet.PetMessage("*Quest",2);
			break;
			case 6:
				cHelpPet.PetMessage("*System",2);
			break;
		}

	}

	switch(sinRunCameraMapPosi){
		case 1:
			cHelpPet.PetMessage("*RunAndWalk",2);
		break;
		case 2:
			if(sInterFlags.CameraAutoFlag == 0){ //카메라 수동 
				cHelpPet.PetMessage("*Camera_Hand",2);
			}
			if(sInterFlags.CameraAutoFlag == 2){ //카메라 고정 
				cHelpPet.PetMessage("*Camera_Fix",2);
			}
			if(sInterFlags.CameraAutoFlag ==1 ){//카메라 자동 
				cHelpPet.PetMessage("*Camera_Auto",2);
			}

		break;
		case 3:
			//cHelpPet.PetMessage("*Mini_Map",2);
		break;
	}



	//Pet 메세지
	//PetMessage
	if (365< pCursorPos.x  && 435 > pCursorPos.x  && 
			589 < pCursorPos.y  && 596 > pCursorPos.y  ){
				cHelpPet.PetMessage("*DayandNight",2);

	}

	//Pet 메세지
	if(pCursorPos.x > 315 && pCursorPos.x < 337 && pCursorPos.y > 500 && pCursorPos.y < 594){//라이프 
		cHelpPet.PetMessage("*Bar_Life",2);
	}
	if(pCursorPos.x > 463 && pCursorPos.x < 483 && pCursorPos.y > 498 && pCursorPos.y < 595){//기력
		cHelpPet.PetMessage("*Bar_Mana",2);
	}
	if(pCursorPos.x > 300 && pCursorPos.x < 313 && pCursorPos.y > 513 && pCursorPos.y < 595){//근력
		cHelpPet.PetMessage("*Bar_Stamina",2);
	}	
	if(pCursorPos.x > 483 && pCursorPos.x < 483+10 && pCursorPos.y > 513 && pCursorPos.y < 595){//경험치
		cHelpPet.PetMessage("*Bar_Exp",2);  
	}	
	if(pCursorPos.x > 500 && pCursorPos.x < 570 && pCursorPos.y > 540 && pCursorPos.y < 594){//경험치 
		cHelpPet.PetMessage("*Exp_Num",2);
	}



	if(cInterFace.ChatFlag){	         
		if(SubChatHeight){ 
			if (0< pCursorPos.x  && 300 > pCursorPos.x  && 
					294 < pCursorPos.y  && 294+116 > pCursorPos.y  ){
						if(!ChatKindIndexNum){
							cHelpPet.PetMessage("*ChatBox",2);
						}

			}
		}
		else{
			if (0< pCursorPos.x  && 305 > pCursorPos.x  && 
					600-192 < pCursorPos.y  && 600-192+210 > pCursorPos.y  ){
						if(!ChatKindIndexNum){
							cHelpPet.PetMessage("*ChatBox",2);
						}

			}
		}
	}

	SkillMouseButtonPosi[1] = 1;
}
/*----------------------------------------------------------------------------*
*							   RButtonUp
*-----------------------------------------------------------------------------*/	
void cINTERFACE::RButtonUp(int x , int y)
{

}
/*----------------------------------------------------------------------------*
*							    KeyDown
*-----------------------------------------------------------------------------*/	
char InputSecretCode[256];
char sinBuff[256];
int  ResetQuset3Flag = 0;

void cINTERFACE::KeyDown()
{
	/////////////암호를 집어 넣는다 
	if(hFocusWnd){  
		//cHelpPet.PetMessage("*ChatBox",3); 일단 채팅은 뺀다
		if(smConfig.DebugMode){
			GetWindowText( hFocusWnd , InputSecretCode , 90 );
			if ( InputSecretCode[0] ) {
				if ( InputSecretCode[0]=='/'){
						memset(sinBuff,0,sizeof(sinBuff));
						wsprintf( sinBuff , "%s" , InputSecretCode );
				}
			}
		}
	}

	//PK시 경험치 경고를 한다 
	if( VRKeyBuff[VK_CONTROL]  ){
		CheckExpPercentControl();

	}
	else
		sinPKWarningExpFlag = 0;


//	char *p=0;
//	char sinCheatLevelBuff[256];
//	char sinLevelTempBuff[256];
//	int sinCheatLevel = 0;

	int t = 0;	

	//채팅 창 관련 (복잡하니까 그냥 냅둔다 )
	if(sinGetKeyClick(VK_RETURN)){ //엔터키를 입력하면     
		if( VRKeyBuff[VK_CONTROL]  ){
			if(!ChatFlag)
				ChatFlag = 1;
			else ChatFlag = 0;
		
		}
//해외 명령어관련 영어 확인할 것.
		if(!lstrcmpi(sinBuff ,"/\270\266\261\272\277\300\277\300\277\300\277\300\277\241\277\344")||!lstrcmpi(sinBuff ,"/PassMature"))
		{
			sinBuff[0] = 0;
			sinSecretFlag = 1;
			TestKeyAniFlag = 1;
			cInvenTory.ReFormInvenItem();
			cInvenTory.ReFormPotionNum();
			ReformCharForm();//재인증 

		}
		if(!lstrcmpi(sinBuff ,"/\263\257\270\260\264\331\264\242!") || !lstrcmpi(sinBuff ,"/FuckingInven!"))
		{
			memset(cInvenTory.InvenItem,0,sizeof(sITEM)*100);
			memset(cInvenTory.InvenItemTemp,0,sizeof(sITEM)*100);
			cInvenTory.ReFormInvenItem();
			cInvenTory.ReFormPotionNum();
			sinChar->Money = 0;
			ReformCharForm();//재인증 
			SendSaveMoney(); //금액 조작을 못하게하기위해 호출한다
			SendChatMessageToServer(" T.T Oh, No ~~~");//" T.T 흑흑 난 이제  틀렸어 ~~~");

		}
		

		char sinTestBuff5[256] ,sinTestBuff6[256];

		if(smConfig.DebugMode){ //레벨을 올리고내린다 아싸아~~
			if(sinBuff[0]){
				memset(sinTestBuff5,0,sizeof(sinTestBuff5));
				memset(sinTestBuff6,0,sizeof(sinTestBuff6));

				sscanf( sinBuff, "%s %s", sinTestBuff5, sinTestBuff6 );			
			
				if ( !lstrcmp( sinTestBuff5 , "/\300\323\261\272100\304\371\275\272\306\256" ) || !lstrcmpi(sinBuff ,"/MrIm's100"))
				{
					if(sinTestBuff6[0]){
						#ifdef	_WINMODE_DEBUG
						if(atoi(&sinTestBuff6[0]) == 1){
							if(chaQuest.sHaQuest100LV.CODE){
								memset(&chaQuest.sHaQuest100LV,0,sizeof(sQUEST_ELEMENTARY));
								cSkill.CancelContinueSkill(CHANGE_ELEMENTARY_QUEST);
							}
						}
						if(atoi(&sinTestBuff6[0]) == 2){
							if(haElementaryQuestLog)
								haElementaryQuestLog = (haElementaryQuestLog - QUESTBIT_FURYOFPHANTOM );
							chaQuest.sHaQuest100LV.State = 12;
						}
						SaveGameData();
						SendChatMessageToServer("MrIm: It's me");
						memset(sinTestBuff5,0,sizeof(sinTestBuff5));
						memset(sinTestBuff6,0,sizeof(sinTestBuff6));
						memset(sinBuff,0,sizeof(sinBuff));
						#endif
					}
				
				}

//해외
				if ( !lstrcmp( sinTestBuff5 , "/\300\323\261\272\303\312\272\270\304\371\275\272\306\256" ) || !lstrcmpi(sinTestBuff5 ,"/MrIm'sbeginner"))//(sinBuff ,"/MrIm'sbeginner"))
				{
					if(sinTestBuff6[0])
					{
						#ifdef	_WINMODE_DEBUG
						for(int i=0;i<8;i++)   //현재 진행중인 퀘스트만 초기화
						{
							if(chaQuest.sHaQuestElementary[i].CODE)
							{
								memset(&chaQuest.sHaQuestElementary[i],0,sizeof(sQUEST_ELEMENTARY));
								cSkill.CancelContinueSkill(CHANGE_ELEMENTARY_QUEST);
							}
						}
						SaveGameData();
						SendChatMessageToServer("MrIm:Reset OK");
						memset(sinTestBuff5,0,sizeof(sinTestBuff5));
						memset(sinTestBuff6,0,sizeof(sinTestBuff6));
						memset(sinBuff,0,sizeof(sinBuff));
						#endif
					}
				
				}

				if ( lstrcmp( sinTestBuff5 , "/\300\314\271\346\271\3744\302\367\304\371\275\272\306\256" )==0 || lstrcmp( sinTestBuff5 , "/Lee4thRankUp" )==0)
				{
					if(sinTestBuff6[0]){
						#ifdef	_WINMODE_DEBUG
						ResetQuset3Flag = atoi(sinTestBuff6);
						//퀘스트 셋팅
						sinCompleteChangeJob4(ResetQuset3Flag);
						memset(sinTestBuff5,0,sizeof(sinTestBuff5));
						memset(sinTestBuff6,0,sizeof(sinTestBuff6));
						memset(sinBuff,0,sizeof(sinBuff));
						#endif
					}
				}

				if ( lstrcmp( sinTestBuff5 , "/\300\314\271\346\271\3743\302\367\304\371\275\272\306\256" )==0  || lstrcmp( sinTestBuff5 , "/Lee3thRankUp" )==0)
				{
					if(sinTestBuff6[0]){
						#ifdef	_WINMODE_DEBUG
						ResetQuset3Flag = atoi(sinTestBuff6);
						if(ResetQuset3Flag == 1){
							memset(&sinQuest_ChangeJob3,0,sizeof(sQUEST_CHANGEJOB3));
							SaveGameData();
							SendChatMessageToServer("MrLee:Retry change Job Quest~ ");
						}
						if(ResetQuset3Flag == 2){
							CheckChangeJob3QuestItem(0);
						}
						memset(sinTestBuff5,0,sizeof(sinTestBuff5));
						memset(sinTestBuff6,0,sizeof(sinTestBuff6));
						memset(sinBuff,0,sizeof(sinBuff));
						#endif
					}
				}
				if ( lstrcmp( sinTestBuff5 , "/\300\314\271\346\271\374\267\271\272\247\304\371\275\272\306\256" )==0 || lstrcmp( sinTestBuff5 , "/LeeResetQuest" )==0)
				{
					if(sinTestBuff6[0]){
						#ifdef	_WINMODE_DEBUG
						ResetQuset3Flag = atoi(sinTestBuff6);
						switch(ResetQuset3Flag){
							case 30:
								memset(&sinQuest_Level30,0,sizeof(sQUEST_CHANGEJOB3));
								SaveGameData();
								SendChatMessageToServer("MrLee: Clear 30th Quest ~");
							break;
							case 55:
								memset(&sinQuest_Level55,0,sizeof(sQUEST_CHANGEJOB3));
								memset(&sinQuest_Level55_2,0,sizeof(sQUEST_CHANGEJOB3));
								SaveGameData();
								SendChatMessageToServer("MrLee: Clear 55th Quest ~");
							break;
							case 70:
								memset(&sinQuest_Level70,0,sizeof(sQUEST_CHANGEJOB3));
								SaveGameData();
								SendChatMessageToServer("MrLee: Clear 70th Quest ~");

							break;
							case 80:
								memset(&sinQuest_Level80,0,sizeof(sQUEST_CHANGEJOB3));
								SaveGameData();
								SendChatMessageToServer("MrLee: Clear 80th Quest ~");

							break;
							case 85:
								memset(&sinQuest_Level85,0,sizeof(sQUEST_CHANGEJOB3));
								SaveGameData();
								SendChatMessageToServer("MrLee: Clear 85th Quest ~");

							break;
							case 90:
								memset(&sinQuest_Level90,0,sizeof(sQUEST_CHANGEJOB3));
								SaveGameData();
								SendChatMessageToServer("MrLee: Clear 90th Quest ~");

							break;

							case 802:
								memset(&sinQuest_Level80_2,0,sizeof(sQUEST_CHANGEJOB3));
								SaveGameData();
								SendChatMessageToServer("MrLee: Clear 80_2nd Quest ~");

							break;

							case 902:
								memset(&sinQuest_Level90_2,0,sizeof(sQUEST_CHANGEJOB3));
								SaveGameData();
								SendChatMessageToServer("MrLee: Clear 90_2nd Quest ~(re-connect) ");

							break;
						}
						if(ResetQuset3Flag == 1){
							memset(&sinQuest_ChangeJob3,0,sizeof(sQUEST_CHANGEJOB3));
							SaveGameData();
							SendChatMessageToServer("MrLee: Try again Quest~");
						}
						if(ResetQuset3Flag == 2){
							CheckChangeJob3QuestItem(0);
						}
						memset(sinTestBuff5,0,sizeof(sinTestBuff5));
						memset(sinTestBuff6,0,sizeof(sinTestBuff6));
						memset(sinBuff,0,sizeof(sinBuff));
						
						#endif

					}


				}

				//p = GetWord( sinLevelTempBuff , sinBuff);
				if ( lstrcmp( sinTestBuff5 , "/\300\314\271\346\271\374\274\370\302\373\260\254\263\327" )==0  || lstrcmp( sinTestBuff5 , "/LeeWhereIs" )==0)
				{
					if(sinTestBuff6[0]){
						#ifdef	_WINMODE_DEBUG
						CheckCharForm(); //인증   
						sinChar->Level = atoi(sinTestBuff6);
						//sinChar->Exp = (int)cCharStatus.sinGetNextExp();

						SetExp64( sinChar , ExpLevelTable[sinChar->Level-1] ); //경험치 셋팅
						CodeXorCharInfo_Exp(); //Xor 반전

						ReformCharForm(); //재인증 
						SendChatMessageToServer("MrLee: OK! finished Level UP /(-_-)/ ");
						memset(sinTestBuff5,0,sizeof(sinTestBuff5));
						memset(sinTestBuff6,0,sizeof(sinTestBuff6));
						memset(sinBuff,0,sizeof(sinBuff));
						
						#endif

					}
				}
				if ( lstrcmp( sinTestBuff5 , "/\300\314\271\346\271\374\265\265\277\315\301\340\277\344\275\272\305\263" )==0  ||lstrcmp( sinTestBuff5 , "/LeeHelpMeSkill" )==0) {
					#ifdef	_WINMODE_DEBUG
					cSkill.InitSkillPoint(1);
					SendChatMessageToServer("MrLee: Skill reset/(-_-)/ ");
					memset(sinTestBuff5,0,sizeof(sinTestBuff5));
					memset(sinTestBuff6,0,sizeof(sinTestBuff6));
					memset(sinBuff,0,sizeof(sinBuff));
					#endif
				}
				if ( lstrcmp( sinTestBuff5 , "/\300\314\271\346\271\374\265\265\277\315\301\340\277\344\275\272\305\310" )==0  ||lstrcmp( sinTestBuff5 , "/LeeHelpMeStat" )==0) {
					#ifdef	_WINMODE_DEBUG
					sinChar->wVersion[1] = 0;
					cCharStatus.InitCharStatus();				
					cInvenTory.SetItemToChar();				
					if ( sinChar->Life[1]<sinGetLife() ) {
						sinSetLife( sinChar->Life[1] );
						ReformCharForm();//재인증 
						ResetEnergyGraph(4);
					}
					SendChatMessageToServer("MrLee: Stat Reset/(-_-)/ ");
					memset(sinTestBuff5,0,sizeof(sinTestBuff5));
					memset(sinTestBuff6,0,sizeof(sinTestBuff6));
					memset(sinBuff,0,sizeof(sinBuff));
					#endif
				}
				if ( lstrcmp( sinTestBuff5 , "/\300\314\271\346\271\374\301\365\301\244\276\306\300\314\305\333" )==0 ) {
					#ifdef	_WINMODE_DEBUG
					if(MouseItem.Flag){
						MouseItem.sItemInfo.SpecialItemFlag[0] = CHECK_GIVE_ITEM;
					}
					SendChatMessageToServer("\300\314\271\346\271\374: \300\337\276\262\260\324(-_-)/ ");
					memset(sinTestBuff5,0,sizeof(sinTestBuff5));
					memset(sinTestBuff6,0,sizeof(sinTestBuff6));
					memset(sinBuff,0,sizeof(sinBuff));

					//sinChar->wVersion[1] = 0;
					//cCharStatus.InitCharStatus();				
					//cInvenTory.SetItemToChar();				

					/*
					if ( sinChar->Life[1]<sinGetLife() ) {
						sinSetLife( sinChar->Life[1] );
						ReformCharForm();//재인증 
						ResetEnergyGraph(4);
					}
					SendChatMessageToServer("이방범: 스탯초기화/(-_-)/ ");
					memset(sinTestBuff5,0,sizeof(sinTestBuff5));
					memset(sinTestBuff6,0,sizeof(sinTestBuff6));
					memset(sinBuff,0,sizeof(sinBuff));
					*/
					#endif
				}
				if ( lstrcmp( sinTestBuff5 , "/\300\314\271\346\271\374\300\374\276\367" )==0  ||lstrcmp( sinTestBuff5 , "/PassRankUp" )==0)
				{
					#ifdef	_WINMODE_DEBUG
					SendChatMessageToServer("MeLee: it's good /(-_-)/ ");
					sinChar->ChangeJob++; //전업을 한다
					if(sinChar->ChangeJob > 4)sinChar->ChangeJob = 4;		// 4: the fifth rank (tier 5 skills)
					// finish it like a rank-up quest: title, face, skill window, saved
					sinJobList = GetJobDataCode(sinChar->JOB_CODE , sinChar->ChangeJob);
					if(sinJobList) lstrcpy(cInvenTory.JobName,sinJobList->szName2);
					cSkill.SearchUseSkill();
					SaveGameData();
					ChangeJobFace();
					memset(sinTestBuff5,0,sizeof(sinTestBuff5));
					memset(sinTestBuff6,0,sizeof(sinTestBuff6));
					memset(sinBuff,0,sizeof(sinBuff));
					#endif
				}

				if ( lstrcmp( sinTestBuff5 , "/\300\314\271\346\271\374\300\374\276\367\304\371\275\272\306\256" )==0  ||lstrcmp( sinTestBuff5 , "/LeeResetRankUp" )==0) { //1은 템스크론 2는 모라이온 
					if(sinTestBuff6[0]){
						#ifdef	_WINMODE_DEBUG
						ResetQuset3Flag = atoi(sinTestBuff6);
						if(ResetQuset3Flag == 1){
							memset(&sinQuest_ChangeJob,0,sizeof(sQUEST_CHANGEJOB));
							SaveGameData();
							SendChatMessageToServer("MrLee: Clear Job Quest of Temscron 2th");
						}
						if(ResetQuset3Flag == 2){
							
							memset(&sinQuest_ChangeJob2,0,sizeof(sQUEST_CHANGEJOB));
							SaveGameData();
							SendChatMessageToServer("MrLee: Clear Job Quest of Moryon 2th");
						}
						if(ResetQuset3Flag == 3){
							
							memset(&sinQuest_ChangeJob3,0,sizeof(sQUEST_CHANGEJOB3));
							SaveGameData();
							SendChatMessageToServer("MrLee: Clear Job Quest of 3th ");
						}
						if(ResetQuset3Flag == 4){
							if(sinChar->ChangeJob==3){
								sinChar->ChangeJob=2;
							}
							memset(&sinQuest_ChangeJob4,0,sizeof(sQUEST_CHANGEJOB3));
							SaveGameData();
							SendChatMessageToServer("MrLee:Reconnect again!! ");
						}
						if(ResetQuset3Flag == 5){		// the fifth rank back to the fourth (to test the rank-up again)
							if(sinChar->ChangeJob>=4){
								sinChar->ChangeJob=3;
								sinJobList = GetJobDataCode(sinChar->JOB_CODE , sinChar->ChangeJob);
								if(sinJobList) lstrcpy(cInvenTory.JobName,sinJobList->szName2);
								ChangeJobFace();
							}
							SaveGameData();
							SendChatMessageToServer("GM: rank 5 reset. Reconnect to refund the tier 5 skill points.");
						}

						memset(sinTestBuff5,0,sizeof(sinTestBuff5));
						memset(sinTestBuff6,0,sizeof(sinTestBuff6));
						memset(sinBuff,0,sizeof(sinBuff));
						#endif
					}

				}
	
				if ( lstrcmp( sinTestBuff5 , "/\300\314\271\346\271\374\304\371\275\272\306\256\303\312\261\342\310\255" )==0 ) {  
					#ifdef	_WINMODE_DEBUG

					SendChatMessageToServer("\300\314\271\346\271\374: \301\370\307\340\301\337\300\316\304\371\275\272\306\256\303\312\261\342\310\255");
					ClearNowQuest();
					memset(sinTestBuff5,0,sizeof(sinTestBuff5));
					memset(sinTestBuff6,0,sizeof(sinTestBuff6));
					memset(sinBuff,0,sizeof(sinBuff));
					#endif
				}

				if ( lstrcmp( sinTestBuff5 , "/\300\314\271\346\271\374\306\367\275\272" )==0  || lstrcmp( sinTestBuff5 , "/LeeForce" )==0) {
					#ifdef	_WINMODE_DEBUG
					SendChatMessageToServer("MrLee: I like force /(-_-)/ ");
					for(t = 0; t < 10 ; t++){
						if(ContinueSkill[t].Flag){
							if(ContinueSkill[t].CODE ==SKILL_FORCE_ORB){
								ContinueSkill[t].CheckTime = ContinueSkill[t].UseTime*70; //continue 스킬 초기화
								cInvenTory.SetItemToChar();
								break;

							}
						}
					}
					memset(sinTestBuff5,0,sizeof(sinTestBuff5));
					memset(sinTestBuff6,0,sizeof(sinTestBuff6));
					memset(sinBuff,0,sizeof(sinBuff));
					#endif
				}
				if ( lstrcmp( sinTestBuff5 , "/\300\314\271\346\271\374\274\364\305\260\303\312\261\342\310\255" )==0  ||lstrcmp( sinTestBuff5 , "/LeeShotKeyReset" )==0) {   
					#ifdef	_WINMODE_DEBUG
					cSkill.SkillShortKeyInit();
					SendChatMessageToServer("MtLee: ShortKey Skill reset ");
					memset(sinTestBuff5,0,sizeof(sinTestBuff5));
					memset(sinTestBuff6,0,sizeof(sinTestBuff6));
					memset(sinBuff,0,sizeof(sinBuff));
					#endif
				}
				if ( lstrcmp( sinTestBuff5 , "/\271\366\307\301\301\246\267\316" )==0 ) {  
					#ifdef	_WINMODE_DEBUG
					for(int i=0 ; i<4 ; i++)
					{
						chaPremiumitem.UpKeepItem[i].PosiX   = 0;
						chaPremiumitem.UpKeepItem[i].PosiY   = 0;
						chaPremiumitem.UpKeepItem[i].Visible = false;
						chaPremiumitem.UpKeepItem[i].TGAImageNumber = 0;
						chaPremiumitem.UpKeepItem[i].IconTime[0] = 0;   
						chaPremiumitem.UpKeepItem[i].IconTime[1] = 0;
						chaPremiumitem.UpKeepItem[i].Alpha       = 255;
						chaPremiumitem.UpKeepItem[i].Alphakind   = 0;
						chaPremiumitem.UpKeepItem[i].Per		 = 0;
						memset(chaPremiumitem.UpKeepItem[i].Id,NULL,sizeof(chaPremiumitem.UpKeepItem[i].Id));
					}
					SendChatMessageToServer("\300\314\271\346\271\374:\271\366\307\301\303\312\261\342\310\255 ");
					memset(sinTestBuff5,0,sizeof(sinTestBuff5));
					memset(sinTestBuff6,0,sizeof(sinTestBuff6));
					memset(sinBuff,0,sizeof(sinBuff));
					#endif
				}
			}
		}
	}
	if(!hFocusWnd) //포커스가 들어오면 채팅창이 나온다 
		sinChatEnter = 0;
	else sinChatEnter = 1;
	if(ChatFlag)sinChatEnter = 1;
	////////채팅관련 끝 ..

}
/*----------------------------------------------------------------------------*
*		라이프 마나 스테미나를 체크해서 보여준다 
*-----------------------------------------------------------------------------*/	
int DefaultLength[4] = {94,94,76,86};
float sinOldNowState[4] = {0,0,0,0};  
float sinOldMaxState[4] = {0,0,0,0};

//생명력 , 기력 , 근력 을 초기화한다 
void cINTERFACE::InitState()
{

	float sinTemp;
	float sinMaxState[4] = {(float)sinChar->Life[1],(float)sinChar->Mana[1] ,(float)sinChar->Stamina[1]};  
	float sinNowState[4] = {(float)sinGetLife(),(float)sinGetMana() ,(float)sinGetStamina()};  

	for(int i=0 ;i < 3; i++){
		if(sinMaxState[i])
			sinTemp = (sinNowState[i]/sinMaxState[i])*100; 
		else
			sinTemp = 0;
		sinBmpLength[i] = (int)((DefaultLength[i]*(sinTemp/100)));
		sinOldNowState[i] = sinNowState[i];

	}

	if ( sinChar->Level<79 ) {	
		sinMaxState[3] = (float)(ExpLevelTable[sinChar->Level] - ExpLevelTable[sinChar->Level-1]);
		sinNowState[3] = (float)((GetXorExp64( sinChar))- ExpLevelTable[sinChar->Level-1]);
	}
	else {
		sinMaxState[3] = (float)((ExpLevelTable[sinChar->Level] - ExpLevelTable[sinChar->Level-1])/1000000);
		sinNowState[3] = (float)(((GetXorExp64( sinChar))- ExpLevelTable[sinChar->Level-1])/1000000);
	}

	InitStateFlag = 1;

}

//생명력 , 기력 , 근력 의 변화에 따라 Bmp의 크기를 변화 시킨다
void cINTERFACE::CheckingState()
{
	
	float	sinTemp;
	//라이프 마나 스테미나 경험치 
	float sinMaxState[4] = {(float)(sinChar->Life[1]),(float)sinChar->Mana[1] ,(float)sinChar->Stamina[1]};// ,(float)sinChar->Next_Exp};  
	float sinNowState[4] = {(float)sinGetLife(),(float)sinGetMana() ,(float)sinGetStamina()};// ,(float)sinChar->Exp};  
	
	//sinMaxState[3] = (float)(ExpLevelTable[sinChar->Level] - ExpLevelTable[sinChar->Level-1]);
	//sinNowState[3] = (float)((sinChar->Exp^CodeXorExp)- ExpLevelTable[sinChar->Level-1]);

	if ( sinChar->Level<79 ) {	
		sinMaxState[3] = (float)(ExpLevelTable[sinChar->Level] - ExpLevelTable[sinChar->Level-1]);
		sinNowState[3] = (float)((GetXorExp64( sinChar))- ExpLevelTable[sinChar->Level-1]);
	}
	else {
		sinMaxState[3] = (float)((ExpLevelTable[sinChar->Level] - ExpLevelTable[sinChar->Level-1])/1000000);
		sinNowState[3] = (float)(((GetXorExp64( sinChar))- ExpLevelTable[sinChar->Level-1])/1000000);
	}

	for(int i = 0 ; i < 4 ; i++){
		if(sinOldNowState[i] != sinNowState[i] || sinOldNowState[i] != sinMaxState[i]){ //플러스 수치 
			if(sinOldNowState[i] != sinNowState[i]){ //수치변화 
				sinTemp = ((sinNowState[i])/sinMaxState[i])*100; 
				sinBmpLength[i] = (int)((DefaultLength[i]*(sinTemp/100)));
				if(sinBmpLength[i] >DefaultLength[i])sinBmpLength[i] = DefaultLength[i]; //이미지 크기보정
				sinOldNowState[i] = sinNowState[i];

			}
	
		}
	}
	
}



//생명력 기력 근력 경험치 수치를 표시한다 
void cINTERFACE::ShowParaState()
{

	InfoMessageBoxPos.x = 0; //좌표 초기화  
	InfoMessageBoxPos.y = 0;

	szInfoMsgBuff5[0] = 0;
	lstrcpy(szInfoMsgBuff5,""); //글자가 깨져서 땜빵 했다 

	//폰트가 깨진다고해서 그냥 자리바꿈 (날치기 땜빵)
	if(pCursorPos.x > 315 && pCursorPos.x < 337 && pCursorPos.y > 500 && pCursorPos.y < 594){//라이프 
		if(AddVirtualLife[1]){
			AddVirtualLife[0] = (int)((float)sinGetLife() * ((float)sinVirtualLifePercent / 100.f));
		}
			cHelpPet.PetMessage("*Bar_Life",0);
			wsprintf(szInfoMsgBuff5,sinStateName[0],(int)sinGetLife()+AddVirtualLife[0],(int)sinChar->Life[1]+AddVirtualLife[1]);
			InfoMessageBoxPos.x = 343;   
			InfoMessageBoxPos.y = 500;

	}
	/*
	if(pCursorPos.x > 463 && pCursorPos.x < 483 && pCursorPos.y > 498 && pCursorPos.y < 595){//기력
		wsprintf(szInfoMsgBuff5,sinStateName[1],(int)sinGetMana(),(int)sinChar->Mana[1]);
		cHelpPet.PetMessage("*Bar_Mana",0);
		InfoMessageBoxPos.x = 490;
		InfoMessageBoxPos.y = 498;

	}
	if(pCursorPos.x > 300 && pCursorPos.x < 313 && pCursorPos.y > 513 && pCursorPos.y < 595){//근력
		cHelpPet.PetMessage("*Bar_Stamina",0);
		wsprintf(szInfoMsgBuff5,sinStateName[2],(int)sinGetStamina(),(int)sinChar->Stamina[1]);
		InfoMessageBoxPos.x = 320;
		InfoMessageBoxPos.y = 513;

	}
	*/
	
	if(pCursorPos.x > 463 && pCursorPos.x < 483 && pCursorPos.y > 498 && pCursorPos.y < 595){//기력
		// 박재원 - 부스터 아이템(기력)
		if(AddVirtualMana[1]){
			AddVirtualMana[0] = (int)((float)sinGetMana() * ((float)sinVirtualManaPercent / 100.f));
		}
		wsprintf(szInfoMsgBuff5,sinStateName[1],(int)sinGetMana(),(int)sinChar->Mana[1]);
		cHelpPet.PetMessage("*Bar_Mana",0);
		InfoMessageBoxPos.x = 490;
		InfoMessageBoxPos.y = 498;

	}
	if(pCursorPos.x > 300 && pCursorPos.x < 313 && pCursorPos.y > 513 && pCursorPos.y < 595){//근력
		// 박재원 - 부스터 아이템(체력)
		if(AddVirtualStamina[1]){
			AddVirtualStamina[0] = (int)((float)sinGetStamina() * ((float)sinVirtualStaminaPercent / 100.f));
		}
		cHelpPet.PetMessage("*Bar_Stamina",0);
		wsprintf(szInfoMsgBuff5,sinStateName[2],(int)sinGetStamina(),(int)sinChar->Stamina[1]);
		InfoMessageBoxPos.x = 320;
		InfoMessageBoxPos.y = 513;

	}	

	if(pCursorPos.x > 483 && pCursorPos.x < 483+10 && pCursorPos.y > 513 && pCursorPos.y < 595){//경험치
		cHelpPet.PetMessage("*Bar_Exp",0);

	}	
	if(pCursorPos.x > 500 && pCursorPos.x < 570 && pCursorPos.y > 540 && pCursorPos.y < 594){//경험치 
		cHelpPet.PetMessage("*Exp_Num",0);
	}

	//int ExpPercent  = 0;
	//int ExpPercentSpot = 0;
	//int ExpSize = 0;
	//int ExpNowSize  = 0;


	/*
	if(pCursorPos.x > 570 && pCursorPos.x < 645 && pCursorPos.y > 586 && pCursorPos.y < 594){//경험치 
		if(sinChar->Level > 79 ){ //sinStateName[4] 는 Mega 표시   
			//wsprintf(szInfoMsgBuff5,sinStateName[4],cCharStatus.sinGetNowExp(),cCharStatus.sinGetNextExp());
			ExpSize = (int)(GetNextExp( sinChar->Level )/1000000) - (int)(GetNextExp( sinChar->Level-1 )/1000000);  
			ExpNowSize = cCharStatus.sinGetNowExp()-(int)(GetNextExp( sinChar->Level-1 )/1000000);
			ExpPercent = (int)(10000.0f/(float)ExpSize * (float)ExpNowSize); 
			ExpPercentSpot = (int)(ExpPercent%100);
			ExpPercent = ExpPercent/100; 
			if(ExpPercentSpot < 10 ){
				wsprintf(szInfoMsgBuff5,ExpPercent3,ExpPercent,ExpPercentSpot,"%  ");
			}
			else{
				wsprintf(szInfoMsgBuff5,ExpPercent2,ExpPercent,ExpPercentSpot,"%  ");
			}


		}
		else{ //sinStateName[3] 은 일반 경험치 표시 
			//Percent로 표시 
			ExpSize = (int)GetNextExp( sinChar->Level ) - (int)GetNextExp( sinChar->Level-1 );  
			ExpNowSize = cCharStatus.sinGetNowExp()-(int)GetNextExp( sinChar->Level-1 );
			ExpPercent = (int)(10000.0f/(float)ExpSize * (float)ExpNowSize); 
			ExpPercentSpot = (int)(ExpPercent%100);
			ExpPercent = ExpPercent/100;
			if(ExpPercentSpot < 10 ){
				wsprintf(szInfoMsgBuff5,ExpPercent3,ExpPercent,ExpPercentSpot,"%  ");
			}
			else{
				wsprintf(szInfoMsgBuff5,ExpPercent2,ExpPercent,ExpPercentSpot,"%  ");
			}

			//wsprintf(szInfoMsgBuff5,sinStateName[3],cCharStatus.sinGetNowExp(),cCharStatus.sinGetNextExp());
		}
		InfoMessageBoxPos.x = 488;    
		InfoMessageBoxPos.y = 537;

	}	
	*/

}

//텍스트를 표시해준다 
void cINTERFACE::DrawInterText()
{
	HDC	hdc;

	lpDDSBack->GetDC( &hdc );  
	SelectObject( hdc, sinFont);
    SetBkMode( hdc, TRANSPARENT );
	SetTextColor( hdc, RGB(255,255,255) );

	if(InfoMessageBoxPos.x != 0 || InfoMessageBoxPos.y != 0) //표시위치에 갔을때만 표시한다   
		dsTextLineOut(hdc,InfoMessageBoxPos.x,InfoMessageBoxPos.y , szInfoMsgBuff5,lstrlen(szInfoMsgBuff5));
	if(ShowExpPercentFlag){  
		SetTextColor( hdc, RGB(202,254,251) );
		dsDrawOffsetArray = dsARRAY_BOTTOM;
		dsTextLineOut(hdc,511,552 , szInfoMsgBuff6,lstrlen(szInfoMsgBuff6));
		dsDrawOffsetArray = dsARRAY_TOP;

	}

	/*
	SetTextColor( hdc, RGB(255,255,255) );
	if(ReStartFlag){
		if(ReStartIndex== 1 && RestartCheckExp ) //경험치기 안되서 필드에서 시작할수없을 경우에는 색을 붉은색으로
			SetTextColor( hdc, RGB(255,0,0) );
		else
			SetTextColor( hdc, RGB(255,255,255) );
		
		if(szReStartMsg[0]){
			dsTextLineOut(hdc,ReStartTextXY.x,ReStartTextXY.y+2 , szReStartMsg,lstrlen(szReStartMsg));
		}
		if(szReStartMsg2[0]){
			dsTextLineOut(hdc,ReStartTextXY.x,ReStartTextXY.y+18 , szReStartMsg2,lstrlen(szReStartMsg2));
		}

	}
	*/

	SetTextColor( hdc, RGB(255,255,255) );
	if(SkillNpcFlag){
		if(szSkillMasterMsg[0]){
			dsTextLineOut(hdc,SkillMasterTextXY.x,SkillMasterTextXY.y+2 , szSkillMasterMsg,lstrlen(szSkillMasterMsg));
		}
		if(szReStartMsg2[0]){
			dsTextLineOut(hdc,SkillMasterTextXY.x,SkillMasterTextXY.y+18 , szSkillMasterMsg2,lstrlen(szSkillMasterMsg2));
		}

	}

	lpDDSBack->ReleaseDC( hdc );

}

//박스를 하나만 보여준다  
void cINTERFACE::CheckAllBox(int ButtonFlag)
{

	int i = 0;
	for(i = 0 ; i < SIN_MAX_HELP_NUMBER; i++){        
		if(sSinHelp[i].KindFlag){
			if(( ButtonFlag ==SIN_CRAFTITEM || ButtonFlag ==SIN_AGING || ButtonFlag == SIN_SMELTING || ButtonFlag == SIN_MANUFACTURE ))return;
		}
		//***** 텔레포트 헬프창 열려있을때 거래가 들어오면 취소시킨다  텔레포트 버그수정( 성근 060621 )
		if(sSinHelp[i].KindFlag == SIN_HELP_KIND_TELEPORT_SCROLL ){
			if ( cTrade.OpenFlag ) {
					//거래중이면 거래 종료
					SendRequestTrade( cTrade.TradeCharCode , 3 );	//거래 취소 요구
					cTrade.CancelTradeItem();						//거래창 닫기
			}
			if(( ButtonFlag == SIN_MYSHOP || ButtonFlag == SIN_TRADE ))return;		
		}
	}
	
	//창고가 떠있을때 다른 창을 열수없다 
	if(cWareHouse.OpenFlag){  
		if(ButtonFlag == SIN_TRADE ||ButtonFlag == SIN_SHOP || 
			ButtonFlag == SIN_CRAFTITEM || ButtonFlag == SIN_AGING ||ButtonFlag == SIN_MYSHOP
			|| ButtonFlag == SIN_CHARSHOP || ButtonFlag == SIN_SMELTING || ButtonFlag == SIN_MANUFACTURE )return;
		
	}
	//개인상점이 떠있을때는 다른창을 열수없다
	if(MyShopSendButton || cMyShop.OpenFlag || cCharShop.OpenFlag){
		if(ButtonFlag == SIN_TRADE ||ButtonFlag == SIN_SHOP || ButtonFlag == SIN_WAREHOUSE ||
			ButtonFlag == SIN_CRAFTITEM || ButtonFlag == SIN_AGING  ||
			ButtonFlag == SIN_CHARSHOP || ButtonFlag == SIN_SMELTING || ButtonFlag == SIN_MANUFACTURE )return;

	}
	//다른 창이 떠있을때는 개인상점과 캐릭터상점을 열수가 없다
	if(cShop.OpenFlag || cCraftItem.OpenFlag || cAging.OpenFlag || cTrade.OpenFlag ||
		cCharShop.OpenFlag || SmeltingItem.OpenFlag || ManufactureItem.m_OpenFlag || cMixtureReset.OpenFlag ){ // 석지용 - 믹스쳐 리셋 창 추가
			if(ButtonFlag == SIN_MYSHOP || ButtonFlag == SIN_CHARSHOP)return;

		}

	if(cShop.OpenFlag || cCraftItem.OpenFlag || cAging.OpenFlag || cTrade.OpenFlag ||
		cMyShop.OpenFlag || cCharShop.OpenFlag || SmeltingItem.OpenFlag || ManufactureItem.m_OpenFlag ){
			if(ButtonFlag == SIN_CHARSHOP)return;

		}
	
	switch(ButtonFlag){
	case SIN_CHARSTATUS: //캐릭터 스테이터스 	
		if(cInvenTory.OpenFlag) cInvenTory.OpenFlag = SIN_CLOSE;
		if(cSkill.OpenFlag) cSkill.OpenFlag = SIN_CLOSE;
		ExitButtonClick = 0; //시스템 창 닫기 
		
	break;
	case SIN_SKILL: //스킬창 
		if(cCharStatus.OpenFlag)cCharStatus.OpenFlag = SIN_CLOSE;
		if(cInvenTory.OpenFlag) cInvenTory.OpenFlag = SIN_CLOSE;
		ExitButtonClick = 0; //시스템 창 닫기 
		
		break;
	case SIN_INVENTORY: //인벤토리 
		if(cCharStatus.OpenFlag)cCharStatus.OpenFlag = SIN_CLOSE;
		if(cSkill.OpenFlag) cSkill.OpenFlag = SIN_CLOSE;
		ExitButtonClick = 0; //시스템 창 닫기 

		break;

	case SIN_TRADE: //트레이드 
		if(cMixtureReset.OpenFlag)	cMixtureReset.OpenFlag = SIN_CLOSE;	// 석지용 - 믹스쳐 리셋 창 추가
		if(cAging.OpenFlag)cAging.OpenFlag = SIN_CLOSE;
		if(cCraftItem.OpenFlag)cCraftItem.OpenFlag = SIN_CLOSE;
		if(cShop.OpenFlag)cShop.OpenFlag = SIN_CLOSE; //상점을 닫아준다 
		if(cWareHouse.OpenFlag)cWareHouse.OpenFlag = SIN_CLOSE; //상점을 닫아준다 
		if(cCharStatus.OpenFlag)cCharStatus.OpenFlag = SIN_CLOSE;
		if(!cInvenTory.OpenFlag) cInvenTory.OpenFlag = SIN_OPEN; //인벤토리가 닫혀있으면 열어준다 		 
		if(cSkill.OpenFlag) cSkill.OpenFlag = SIN_CLOSE;
		// pluto 제련
		if( SmeltingItem.OpenFlag )
		{
			SmeltingItem.OpenFlag = SIN_CLOSE;
		}
		// pluto 제작
		if( ManufactureItem.m_OpenFlag )
		{
			ManufactureItem.m_OpenFlag = SIN_CLOSE;
		}
		cTrade.ReFormTradeItem();	//트레이드 아이템 체크섬 초기화 
		cInvenTory.ChangeSpecialItem(2); //퀘스트아이템을 정리한다

	break;

	case SIN_SHOP: //상점 
		if(cMixtureReset.OpenFlag)	cMixtureReset.OpenFlag = SIN_CLOSE;	// 석지용 - 믹스쳐 리셋 창 추가
		if(cAging.OpenFlag)cAging.OpenFlag = SIN_CLOSE;
		if(cCraftItem.OpenFlag)cCraftItem.OpenFlag = SIN_CLOSE;
		if(cTrade.OpenFlag)cTrade.OpenFlag = SIN_CLOSE; 
		if(cWareHouse.OpenFlag)cWareHouse.OpenFlag = SIN_CLOSE; 
		if(cSkill.OpenFlag) cSkill.OpenFlag = SIN_CLOSE;
		if(cCharStatus.OpenFlag)cCharStatus.OpenFlag = SIN_CLOSE;
		// pluto 제련
		if( SmeltingItem.OpenFlag )
		{
			SmeltingItem.OpenFlag = SIN_CLOSE;
		}
		// pluto 제작
		if( ManufactureItem.m_OpenFlag )
		{
			ManufactureItem.m_OpenFlag = SIN_CLOSE;
		}
		if(!cInvenTory.OpenFlag) cInvenTory.OpenFlag = SIN_OPEN; //인벤토리가 닫혀있으면 열어준다 		 
		if(!cShop.OpenFlag) cShop.OpenFlag = SIN_OPEN; //상점이 닫혀있으면 열어준다 
		cInvenTory.ChangeSpecialItem(2); //퀘스트아이템을 정리한다
				
	break;

	case SIN_SYSTEM: //시스템 
		if(cInvenTory.OpenFlag) cInvenTory.OpenFlag = SIN_CLOSE;
		if(cSkill.OpenFlag) cSkill.OpenFlag = SIN_CLOSE;
		if(cCharStatus.OpenFlag)cCharStatus.OpenFlag = SIN_CLOSE;

	break;
	case SIN_ALL_CLOSE:
		if(cTrade.OpenFlag)break;
		if(cInvenTory.OpenFlag) cInvenTory.OpenFlag = SIN_CLOSE;
		if(cSkill.OpenFlag) cSkill.OpenFlag = SIN_CLOSE;
		if(cCharStatus.OpenFlag)cCharStatus.OpenFlag = SIN_CLOSE;
		if(cShop.OpenFlag)cShop.OpenFlag = SIN_CLOSE;
		if(cTrade.OpenFlag)cTrade.OpenFlag = SIN_CLOSE;
		// 장별 - 제련창, 제작창 종료
		// pluto 제련
	//	if( SmeltingItem.OpenFlag )
	//	{
	//		SmeltingItem.OpenFlag = SIN_CLOSE;
	//	}
		// pluto 제작
	//	if( ManufactureItem.m_OpenFlag )
	//	{
	//		ManufactureItem.m_OpenFlag = SIN_CLOSE;
	//	}
		ExitButtonClick = 0; 
	break;

	case SIN_WAREHOUSE: //창고 
		//미니맵이떠있을겨우에는 창고가 열리지않는다
		if(g_IsDxProjectZoomIn)break;

		if(cMixtureReset.OpenFlag)	cMixtureReset.OpenFlag = SIN_CLOSE;	// 석지용 - 믹스쳐 리셋 창 추가
		if(cTrade.OpenFlag)cTrade.OpenFlag = SIN_CLOSE; 
		if(cShop.OpenFlag)cShop.OpenFlag = SIN_CLOSE; 
		if(cSkill.OpenFlag) cSkill.OpenFlag = SIN_CLOSE;
		if(cAging.OpenFlag)cAging.OpenFlag = SIN_CLOSE;
		if(cCraftItem.OpenFlag)cCraftItem.OpenFlag = SIN_CLOSE;
		if(cCharStatus.OpenFlag)cCharStatus.OpenFlag = SIN_CLOSE;
		if(!cInvenTory.OpenFlag) cInvenTory.OpenFlag = SIN_OPEN; //인벤토리가 닫혀있으면 열어준다 		 
		if(!cWareHouse.OpenFlag) cWareHouse.OpenFlag = SIN_OPEN; //상점이 닫혀있으면 열어준다 		 
		// pluto 제련
		if( SmeltingItem.OpenFlag )
		{
			SmeltingItem.OpenFlag = SIN_CLOSE;
		}
		// pluto 제작
		if( ManufactureItem.m_OpenFlag )
		{
			ManufactureItem.m_OpenFlag = SIN_CLOSE;
		}
		cWareHouse.ReFormWareHouse();
		cWareHouse.CheckCopyItem(); //복사된 아이템이 있는지를 체크한다 
		cWareHouse.DeleteCopyItem(); //창고의 복사아이템을 검색해서 없애준다 
		cInvenTory.ChangeSpecialItem(2); //퀘스트아이템을 정리한다
	

	break;

	case SIN_CRAFTITEM: //아이템 조합 
		if(cMixtureReset.OpenFlag)	cMixtureReset.OpenFlag = SIN_CLOSE;	// 석지용 - 믹스쳐 리셋 창 추가
		if(cTrade.OpenFlag)cTrade.OpenFlag = SIN_CLOSE; 
		if(cShop.OpenFlag)cShop.OpenFlag = SIN_CLOSE; 
		if(cSkill.OpenFlag) cSkill.OpenFlag = SIN_CLOSE;
		if(cAging.OpenFlag)cAging.OpenFlag = SIN_CLOSE;
		if(cWareHouse.OpenFlag) cWareHouse.OpenFlag = SIN_CLOSE;
		if(cCharStatus.OpenFlag)cCharStatus.OpenFlag = SIN_CLOSE;
		if(!cInvenTory.OpenFlag) cInvenTory.OpenFlag = SIN_OPEN; //인벤토리가 닫혀있으면 열어준다 		 
		if(!cCraftItem.OpenFlag) cCraftItem.OpenFlag = SIN_OPEN; //상점이 닫혀있으면 열어준다 		 
		// pluto 제련
		if( SmeltingItem.OpenFlag )
		{
			SmeltingItem.OpenFlag = SIN_CLOSE;
		}
		// pluto 제작
		if( ManufactureItem.m_OpenFlag )
		{
			ManufactureItem.m_OpenFlag = SIN_CLOSE;
		}
		cCraftItem.ReFormCraftItem(); //믹스쳐 아이템을 확인해준다 
		cInvenTory.ChangeSpecialItem(2); //퀘스트아이템을 정리한다

		
	break;
	case SIN_AGING:	//에이징 
		if(cMixtureReset.OpenFlag)	cMixtureReset.OpenFlag = SIN_CLOSE;	// 석지용 - 믹스쳐 리셋 창 추가
		if(cTrade.OpenFlag)cTrade.OpenFlag = SIN_CLOSE; 
		if(cShop.OpenFlag)cShop.OpenFlag = SIN_CLOSE; 
		if(cSkill.OpenFlag) cSkill.OpenFlag = SIN_CLOSE;
		if(cCraftItem.OpenFlag)cCraftItem.OpenFlag = SIN_CLOSE;
		if(cWareHouse.OpenFlag) cWareHouse.OpenFlag = SIN_CLOSE;
		if(cCharStatus.OpenFlag)cCharStatus.OpenFlag = SIN_CLOSE;
		if(!cInvenTory.OpenFlag) cInvenTory.OpenFlag = SIN_OPEN; //인벤토리가 닫혀있으면 열어준다 		 
		if(!cAging.OpenFlag)cAging.OpenFlag = SIN_OPEN;
		// pluto 제련
		if( SmeltingItem.OpenFlag )
		{
			SmeltingItem.OpenFlag = SIN_CLOSE;
		}
		// pluto 제작
		if( ManufactureItem.m_OpenFlag )
		{
			ManufactureItem.m_OpenFlag = SIN_CLOSE;
		}
		cInvenTory.ChangeSpecialItem(2); //퀘스트아이템을 정리한다
//		cCraftItem.ReFormCraftItem(); //믹스쳐 아이템을 확인해준다 

		break;
	case SIN_MIXTURE_RESET:	// 석지용 - 믹스쳐 리셋 창 빼곤 전부 닫는다.
		if(cTrade.OpenFlag)			cTrade.OpenFlag			= SIN_CLOSE;	// 트레이드 창 닫기
		if(cShop.OpenFlag)			cShop.OpenFlag			= SIN_CLOSE;	// 상점 창 닫기
		if(cSkill.OpenFlag)			cSkill.OpenFlag			= SIN_CLOSE;	// 스킬 창 닫기
		if(cCraftItem.OpenFlag)		cCraftItem.OpenFlag		= SIN_CLOSE;	// 조합 창 닫기
		if(cWareHouse.OpenFlag)		cWareHouse.OpenFlag		= SIN_CLOSE;	// 창고 닫기
		if(cCharStatus.OpenFlag)	cCharStatus.OpenFlag	= SIN_CLOSE;	// 캐릭정보 창 닫기
		if(cAging.OpenFlag)			cAging.OpenFlag			= SIN_CLOSE;	// 에이징 창 닫기
		if( SmeltingItem.OpenFlag )	SmeltingItem.OpenFlag	= SIN_CLOSE;	// 제련 창 닫기
		if( ManufactureItem.m_OpenFlag )	ManufactureItem.m_OpenFlag = SIN_CLOSE;	// 제작 창 닫기

		if(!cInvenTory.OpenFlag)	cInvenTory.OpenFlag		= SIN_OPEN;		// 인벤토리가 닫혀있으면 열어준다 	
		if(!cMixtureReset.OpenFlag)	// 믹스쳐 리셋 창이 닫혀있으면 열어준다.
		{
			cMixtureReset.OpenFlag = SIN_OPEN;
		}
		cInvenTory.ChangeSpecialItem(2); //퀘스트아이템을 정리한다
		break;
	case SIN_SMELTING:		// pluto 제련
		if(cMixtureReset.OpenFlag)	cMixtureReset.OpenFlag = SIN_CLOSE;	// 석지용 - 믹스쳐 리셋 창 추가
		if( cTrade.OpenFlag )
		{
			cTrade.OpenFlag = SIN_CLOSE;
		}

		if( cShop.OpenFlag )
		{
			cShop.OpenFlag = SIN_CLOSE;
		}

		if( cSkill.OpenFlag )
		{
			cSkill.OpenFlag = SIN_CLOSE;
		}

		if( cCraftItem.OpenFlag )
		{
			cCraftItem.OpenFlag = SIN_CLOSE;
		}

		if( cWareHouse.OpenFlag )
		{
			cWareHouse.OpenFlag = SIN_CLOSE;
		}

		if( cCharStatus.OpenFlag )
		{
			cCharStatus.OpenFlag = SIN_CLOSE;
		}

		if( cAging.OpenFlag )
		{
			cAging.OpenFlag = SIN_CLOSE;
		}
		// pluto 제작
		if( ManufactureItem.m_OpenFlag )
		{
			ManufactureItem.m_OpenFlag = SIN_CLOSE;
		}
		if( !cInvenTory.OpenFlag )
		{
			cInvenTory.OpenFlag = SIN_OPEN;
		}
		if( !SmeltingItem.OpenFlag )
		{
			SmeltingItem.OpenFlag = SIN_OPEN;
		}
		cInvenTory.ChangeSpecialItem(2); //퀘스트아이템을 정리한다
	break;
	case SIN_MANUFACTURE:		// pluto 제작
		if(cMixtureReset.OpenFlag)	cMixtureReset.OpenFlag = SIN_CLOSE;	// 석지용 - 믹스쳐 리셋 창 추가
		if( cTrade.OpenFlag )
		{
			cTrade.OpenFlag = SIN_CLOSE;
		}

		if( cShop.OpenFlag )
		{
			cShop.OpenFlag = SIN_CLOSE;
		}

		if( cSkill.OpenFlag )
		{
			cSkill.OpenFlag = SIN_CLOSE;
		}

		if( cCraftItem.OpenFlag )
		{
			cCraftItem.OpenFlag = SIN_CLOSE;
		}

		if( cWareHouse.OpenFlag )
		{
			cWareHouse.OpenFlag = SIN_CLOSE;
		}

		if( cCharStatus.OpenFlag )
		{
			cCharStatus.OpenFlag = SIN_CLOSE;
		}

		if( cAging.OpenFlag )
		{
			cAging.OpenFlag = SIN_CLOSE;
		}
		if( SmeltingItem.OpenFlag )
		{
			SmeltingItem.OpenFlag = SIN_CLOSE;
		}
		if( !cInvenTory.OpenFlag )
		{
			cInvenTory.OpenFlag = SIN_OPEN;
		}
		if( !ManufactureItem.m_OpenFlag )
		{
			ManufactureItem.m_OpenFlag = SIN_OPEN;
			// pluto 제작 아이템 가격, 무게 정보 가져 올려고
			if( !g_Manufacture_ItemInfo[0].ItemCode ) // 처음 제작창 열때만 보낸다
			{
				smTRANS_COMMAND	TransCommand;
				TransCommand.code =	smTRANSCODE_MANUFACTURE_WEIGHT_PRICE;
				TransCommand.size = sizeof(smTRANS_COMMAND);
				TransCommand.WParam = 0;
				TransCommand.LParam = 0;
				TransCommand.SParam = 0;
				smWINSOCK* lpsmSock;
				lpsmSock = GetAreaServerSock();		//보낼 서버구하기
				if( lpsmSock )
				{
					lpsmSock->Send2( (char*)&TransCommand, TransCommand.size, TRUE );
				}
			}
		}
		cInvenTory.ChangeSpecialItem(2); //퀘스트아이템을 정리한다
	break;
	case SIN_MYSHOP:
		if(lpCurPlayer->OnStageField>=0 && StageField[ lpCurPlayer->OnStageField ]->State == FIELD_STATE_VILLAGE ){
			if(cMixtureReset.OpenFlag)cMixtureReset.OpenFlag = SIN_CLOSE;	// 석지용 - 믹스쳐 리셋 창 추가
			if(cTrade.OpenFlag)cTrade.OpenFlag = SIN_CLOSE; 
			if(cShop.OpenFlag)cShop.OpenFlag = SIN_CLOSE; 
			if(cSkill.OpenFlag) cSkill.OpenFlag = SIN_CLOSE;
			if(cAging.OpenFlag)cAging.OpenFlag = SIN_CLOSE;
			if(cCraftItem.OpenFlag)cCraftItem.OpenFlag = SIN_CLOSE;
			if(cCharStatus.OpenFlag)cCharStatus.OpenFlag = SIN_CLOSE;
			// pluto 제련
			if( SmeltingItem.OpenFlag )
			{
				SmeltingItem.OpenFlag = SIN_CLOSE;
			}
			// pluto 제작
			if( ManufactureItem.m_OpenFlag )
			{
				ManufactureItem.m_OpenFlag = SIN_CLOSE;
			}
			if(cCharShop.OpenFlag){
				cCharShop.OpenFlag = 0; //상점을 닫는다
				memset(cCharShop.CharShopItem,0,sizeof(sITEM)*30); //아이템을 지워준다
			}
			if(!cInvenTory.OpenFlag) cInvenTory.OpenFlag = SIN_OPEN; //인벤토리가 닫혀있으면 열어준다 		 
			if(!cMyShop.OpenFlag) cMyShop.OpenFlag = SIN_OPEN; //상점이 닫혀있으면 열어준다 
			cInvenTory.ChangeSpecialItem(2); //퀘스트아이템을 정리한다
		}
		
	break;
	case SIN_CHARSHOP:
		if(lpCurPlayer->OnStageField>=0 && StageField[ lpCurPlayer->OnStageField ]->State == FIELD_STATE_VILLAGE){
			if(cMixtureReset.OpenFlag)cMixtureReset.OpenFlag = SIN_CLOSE;	// 석지용 - 믹스쳐 리셋 창 추가
			if(cTrade.OpenFlag)cTrade.OpenFlag = SIN_CLOSE; 
			if(cShop.OpenFlag)cShop.OpenFlag = SIN_CLOSE; 
			if(cSkill.OpenFlag) cSkill.OpenFlag = SIN_CLOSE;
			if(cAging.OpenFlag)cAging.OpenFlag = SIN_CLOSE;
			if(cCraftItem.OpenFlag)cCraftItem.OpenFlag = SIN_CLOSE;
			if(cCharStatus.OpenFlag)cCharStatus.OpenFlag = SIN_CLOSE;
			// pluto 제련
			if( SmeltingItem.OpenFlag )
			{
				SmeltingItem.OpenFlag = SIN_CLOSE;
			}
			// pluto 제작
			if( ManufactureItem.m_OpenFlag )
			{
				ManufactureItem.m_OpenFlag = SIN_CLOSE;
			}
			if(cMyShop.OpenFlag){
				cMyShop.OpenFlag = SIN_CLOSE;
				cMyShop.OpenFlag = 0;
				memset(cMyShop.MyShopItem,0,sizeof(sITEM)*30); //아이템을 지워준다
				memset(MyShopItemIndex,0,sizeof(int)*100);
				MyShopSendButton = 0; //등록버튼 초기화
			}
			if(!cInvenTory.OpenFlag) cInvenTory.OpenFlag = SIN_OPEN; //인벤토리가 닫혀있으면 열어준다 		 
			if(!cCharShop.OpenFlag) cCharShop.OpenFlag = SIN_OPEN; //상점이 닫혀있으면 열어준다 		 
			cInvenTory.ChangeSpecialItem(2); //퀘스트아이템을 정리한다
		}
	break;

	}
	

	
}
//시계 바늘을 움직인다 
int cINTERFACE::DrawClockArrow( int x, int y, int Angle )
{

	/////////////시계 게이지를 움직인다 
	//sinBmpLength[4] = 

	float fx[4],fy[4];
	int sinY,cosY;
	int dx[4],dy[4];
	int cnt;

	int sinTimeAngle;
	float TempTime , TempTime2 , TempTime3;

	TempTime2 = (float)dwGameHour*60;
	TempTime3 = (float)(TempTime2+dwGameMin)/1440;
	
	TempTime = (float)TempTime3*100;
	
	//sinTimeAngle = (int)2190+(780*(TempTime/100));
	sinTimeAngle = (int)(2190+(780*(TempTime/100)));

	sinY = GetSin[ sinTimeAngle&ANGCLIP ];
	cosY = GetCos[ sinTimeAngle&ANGCLIP ];

	dx[0] = 8; dy[0] = 48;
	dx[1] = 8; dy[1] = 32;
	dx[2] =  -8; dy[2] = 48;
	dx[3] =  -8; dy[3] = 32;
/*
	dx[0] = -8; dy[0] = -8;
	dx[1] = -8; dy[1] =  8;
	dx[2] =  8; dy[2] = -8;
	dx[3] =  8; dy[3] =  8;
*/
	for( cnt=0;cnt<4;cnt++) {
		fx[cnt] = (float) (( dy[cnt]*sinY + dx[cnt]*cosY )>>8);
		fy[cnt] = (float) (( dy[cnt]*cosY - dx[cnt]*sinY )>>8);
		fx[cnt] /= 256;
		fy[cnt] /= 256;
		fx[cnt] += (float)x;
		fy[cnt] += (float)y;
	}


	psDrawTexImage2( smMaterial[cInterFace.MatClock].smTexture[0] , fx , fy, 0,0,1,1,255,0 );
//	psDrawTexImage2( 0 , fx , fy, 0,0,1,1,255,0 );


	return TRUE;
}

//나침반을 그린다 
int cINTERFACE::DrawCompass(int x , int y ,int Angle)
{

	float fx[4],fy[4];
	int sinY,cosY;
	int dx[4],dy[4];
	int cnt;

	sinY = GetSin[ Angle&ANGCLIP ];
	cosY = GetCos[ (Angle+ANGLE_180)&ANGCLIP ];

	dx[0] = 8; dy[0] = 13+16;
	dx[1] = 8; dy[1] = 13;
	dx[2] =  -8; dy[2] = 13+16;
	dx[3] =  -8; dy[3] = 13;

	for( cnt=0;cnt<4;cnt++) {
		fx[cnt] = (float) (( dy[cnt]*sinY + dx[cnt]*cosY )>>8);
		fy[cnt] = (float) (( dy[cnt]*cosY - dx[cnt]*sinY )>>8);
		fx[cnt] /= 256;
		fy[cnt] /= 256;
		fx[cnt] += (float)x;
		fy[cnt] += (float)y;
	}


	psDrawTexImage2( smMaterial[cInterFace.MatComPass].smTexture[0] , fx , fy, 0,0,1,1,255,0 );


	return TRUE;
}

/*
//이미지를 로딩한다 
void cINTERFACE::LoadReStartImage()
{
	lpReStartMain		= LoadDibSurfaceOffscreen( "Image\\SinImage\\Inter\\ReStart\\ReStartMain.bmp"); //다시시작 메인 
	lpReStartButton[0]	= LoadDibSurfaceOffscreen( "Image\\SinImage\\Inter\\ReStart\\ReStart_Field.bmp"); //다시시작 버튼 
	lpReStartButton[1]	= LoadDibSurfaceOffscreen( "Image\\SinImage\\Inter\\ReStart\\ReStart_Town.bmp"); //다시시작 버튼 
	lpReStartButton[2]	= LoadDibSurfaceOffscreen( "Image\\SinImage\\Inter\\ReStart\\ReStart_Exit.bmp"); //다시시작 버튼 
	
}


void cINTERFACE::ReleaseReStartImage()
{
	dwCheckTime = 0;			//시간 초기화 
	ReStartImageLoadFlag = 0;	//로딩 플랙 초기화 
	if(lpReStartMain){
		lpReStartMain->Release();
		lpReStartMain = 0;
		
	}
	for(int i=0; i < 3 ; i++){	
		if(lpReStartButton[i]){
			lpReStartButton[i]->Release();
			lpReStartButton[i] = 0;
			
		}
	}
}
*/


//컨트롤 키가 눌렸을때 경험치가 4%미만이면 경험치 바가 깜박거린다 (경험치 테이블 변경으로 잠시 막아둠)
int cINTERFACE::CheckExpPercentControl()
{
	/*
	int ExpBefore , ExpNext; 
	float ExpPersent , Result;

	if(sinChar->Level > 1  ){
		ExpBefore = (int)GetNextExp(sinChar->Level - 1);
		//ExpBefore ^= CodeXorExp;
		ExpNext   = (int)GetNextExp(sinChar->Level);
		//ExpNext   ^= CodeXorExp;

		//ExpPersent = ((float)ExpBefore/(float)ExpNext)*100.0f;

		ExpPersent = (float)(ExpNext - ExpBefore);
		
		Result = ((float)((sinChar->Exp^CodeXorExp)-ExpBefore)/(float)ExpPersent)*100.0f;

		//Result = ((float)ExpPersent/(float)sinChar->Exp)*100.0f;

		if(Result < 4.0f)  
			sinPKWarningExpFlag = 1;

	}
	*/

	return TRUE;
}

/////////////시간을 구해서 표시한다 
int cINTERFACE::GetWordTimeDisplay()
{
	float TempfHour = (float)dwGameHour;
	//float TempfHour = (float)TestTime4;
	if(TempfHour >= 23 || TempfHour < 4)
		DayOrNightFlag = 2;
	else 
		DayOrNightFlag = 1;

	if(DayOrNightFlag == 1 ){
		sinGageTime2 = (int)(50.0f*((TempfHour-4.0f)/19.0f));
	}
	else{
		sinGageTime2 = (int)(50.0f*((TempfHour+1)/5.0f));
	}
	if(sinGageTime2 >  50)sinGageTime2 = 50;
	
	return TRUE;
}

//경험치Percent를 보여준다 
void cINTERFACE::ShowExpPercent()
{

	/*
	if(cInvenTory.OpenFlag || cCharStatus.OpenFlag ||cSkill.OpenFlag){
		ShowExpPercentFlag = 0;
		return;
	}
	else
		ShowExpPercentFlag = 1;
	*/
	ShowExpPercentFlag = 1; 

	int ExpPercent  = 0;
	int ExpPercentSpot = 0;
	int ExpSize = 0;
	int ExpNowSize  = 0;

	szInfoMsgBuff6[0] = 0;

	if(sinChar->Level > 79 ){ 
		ExpSize = (int)((GetNextExp( sinChar->Level ) - GetNextExp( sinChar->Level-1 ))/1000000);
		ExpNowSize = (int)(cCharStatus.sinGetNowExp()-(GetNextExp( sinChar->Level-1 )/1000000));
		ExpPercent = (int)(100000.0f/(float)ExpSize * (float)ExpNowSize); 
		ExpPercentSpot = (int)(ExpPercent%1000);
		ExpPercent = ExpPercent/1000; 

		if(ExpPercent >=100){ //순 날치기 땜빵 -_-
			ExpPercent = 99;
			ExpPercentSpot =999;
		}

		if(ExpPercentSpot < 10 ){
			wsprintf(szInfoMsgBuff6,"%d.00%d%s",ExpPercent,ExpPercentSpot,"%  ");
		}
		else if ( ExpPercentSpot >= 10 &&  ExpPercentSpot < 100 ){
			wsprintf(szInfoMsgBuff6,"%d.0%d%s",ExpPercent,ExpPercentSpot,"%  ");
		}
		else
			wsprintf(szInfoMsgBuff6,"%d.%d%s",ExpPercent,ExpPercentSpot,"%  ");

	}
	else{ 
		ExpSize = (int)GetNextExp( sinChar->Level ) - (int)GetNextExp( sinChar->Level-1 );  
		ExpNowSize = cCharStatus.sinGetNowExp()-(int)GetNextExp( sinChar->Level-1 );
		ExpPercent = (int)(100000.0f/(float)ExpSize * (float)ExpNowSize); 
		ExpPercentSpot = (int)(ExpPercent%1000);
		ExpPercent = ExpPercent/1000;

		//펫을 위해 경험치를 낼름 저장한다
		cHelpPet.PetExp = ExpPercent;

		if(ExpPercentSpot < 10 ){
			wsprintf(szInfoMsgBuff6,"%d.00%d%s",ExpPercent,ExpPercentSpot,"%  ");
		}
		else if ( ExpPercentSpot >= 10 &&  ExpPercentSpot < 100 ){
			wsprintf(szInfoMsgBuff6,"%d.0%d%s",ExpPercent,ExpPercentSpot,"%  ");
		}
		else
			wsprintf(szInfoMsgBuff6,"%d.%d%s",ExpPercent,ExpPercentSpot,"%  ");

	}

}


//글자이펙트를 보여준다============================================================================================.
int cINTERFACE::SetStringEffect(int Index)
{
	if(!HA_EffectIndex[Index]){
			HA_EffectIndex[Index]=CreateTextureMaterial(HaEffectFilePath[Index], 0, 0, 0,0, SMMAT_BLEND_ALPHA); 
			ReadTextures();	//텍스쳐 로딩
	}
	for(int i=0;i<MAX_HAEFFECTNUM;i++){
		if(!HaEffect[i].Flag){
			HaEffect[i].Flag = 1;
			HaEffect[i].Time = 0;
			HaEffect[i].Mat  = HA_EffectIndex[Index];
			HaEffect[i].Posi.x = 354;
			HaEffect[i].Posi.y = 488;
			HaEffect[i].Size.x = 128;
			HaEffect[i].Size.y = 64;
			return true;
		}
	}
	return false;
	
}
//===================================================================================================================
