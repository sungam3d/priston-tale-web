/*----------------------------------------------------------------------------*
*	파일명 :  sinCharStatus.cpp	
*	하는일 :  캐릭터 스테이터스에 관한 모든걸 관리한다 
*	작성일 :  최종업데이트 12월
*	적성자 :  박상열 
*-----------------------------------------------------------------------------*/	

#include "sinLinkHeader.h"
#include "..\\Language.h"
#include "..\\tjboy\\clanmenu\\tjclanDEF.h"
#include "..\\tjboy\\clanmenu\\tjclan.h"
#include "..\\tjboy\\clanmenu\\cE_CViewClanInfo.h"

/*----------------------------------------------------------------------------*
*								전역변수 			
*-----------------------------------------------------------------------------*/	
cCHARSTATUS cCharStatus;
sDISPLAYSTATE sDisplayState[10];
sDISPLAYSTATE sDisplayStateShow;


int OldLevel;					//레벨을 저장한다 

int OldLevelTemp;				//레벨을 저장한다 후 또 저장하다 (왜? 나쁜넘들이 설치지못하도록)

int sinLevelPoint= 0;		    //레벨 포인트를 저장한다 

int sinPointButtonDown = 0;     

int ShowCharArrow=0;

int RegiInfoPosi = 0;

int ReHeight3 = 2;

sCHARRECTPOSI	sCharRectPosi[MAX_CHAR_RECT_POSI]={
	//기본정보
	{"Class"    ,{ 75   , 442+ReHeight3  , 160 , 454  }}, //0
	{"Name"     ,{ 75   , 463+ReHeight3  , 160 , 479  }}, //1
	{"Level"    ,{ 75   , 503+ReHeight3  , 128 , 504  }}, //2
	{"Exp"      ,{ 75   , 543+ReHeight3  , 160 , 549  }}, //3
	{"Next_Exp" ,{ 75   , 563+ReHeight3  , 160 , 574  }}, //4
	{"Life"		,{ 391  , 429+ReHeight3  , 482 , 440  }}, //5
	{"Mana"     ,{ 391  , 454+ReHeight3  , 482 , 466  }}, //6
	{"Stamina"  ,{ 391  , 479+ReHeight3  , 482 , 490  }}, //7

	//능력치 
	{"Strength" ,{ 585 , 433+ReHeight3 , 632 , 444  }}, //8
	{"Spirit"   ,{ 585 , 454+ReHeight3 , 632 , 464  }}, //9  
	{"Talent"   ,{ 585 , 473+ReHeight3 , 632 , 485  }}, //10  
	{"Dexterity",{ 585 , 493+ReHeight3 , 632 , 504  }}, //11
	{"Health"   ,{ 585 , 513+ReHeight3 , 632 , 524  }}, //12	

	//저항 
	{"Bionic"   ,{ 217+3 , 449+8+ReHeight3 , 248  , 468 }}, //13
	{"Poison"   ,{ 286+3 , 448+8+ReHeight3 , 315  , 469 }}, //14
	{"Fire"     ,{ 217+3 , 486+8+ReHeight3 , 248  , 469 }}, //15
	{"Lighting" ,{ 286+3 , 486+8+ReHeight3 , 315  , 505 }}, //16
	{"Ice"      ,{ 217+3 , 523+8+ReHeight3 , 248  , 543 }}, //17

	//공격 방어 관련 
	{"Attack_Rating" ,{ 710 , 455+ReHeight3-1 , 769 , 464 }}, //18
	{"Attack_Damage" ,{ 710 , 474+ReHeight3-1 , 769 , 484 }}, //19
	{"Defence"       ,{ 710 , 495+ReHeight3-1 , 769 , 504 }}, //20
	{"Speed"         ,{ 710 , 515+ReHeight3-1 , 769 , 524 }}, //21
	{"Absorption"    ,{ 710 , 534+ReHeight3-1 , 769 , 545 }}, //22
	{"Point"         ,{ 585 , 533+ReHeight3-1 , 632 , 544 }}, //23

	
};

int RegiBox[5][4] = {
	{ 185 , 445 , 216  , 475 }, //바이오
	{ 185 , 482 , 216  , 512 }, //불
	{ 185 , 519 , 216  , 550 }, //얼음	
	{ 251 , 445 , 281  , 475 }, //독
	{ 251 , 482 , 281  , 512 }, //번개


};

int PointButton[6][4]={
	{565+1,431+1,583,449},
	{565+1,452+1,583,469},
	{565+1,472+1,583,488},
	{565+1,491+1,583,508},
	{565+1,511+1,583,529},
	{565+1,531+1,583,549},

};

int TempStatePoint[5] = {0,0,0,0,0}; //저장된 스테이트를 뺄수 있게 하기위해서 Temp를 둔다 

int PointButtonPosi =0;

LPDIRECTDRAWSURFACE4	lpStatusMain;

////////////////// 채팅이미지 
int MatChatBotton_Glay = 0;
LPDIRECTDRAWSURFACE4 lpChatButton[5];
LPDIRECTDRAWSURFACE4 lpChatButton_Notice[3];
LPDIRECTDRAWSURFACE4 lpChatLine;
int NoticeChatIndex = 0;
int NoticeChatIndexNum = 0;
int ChatKindIndex = 0;
int ChatKindIndexNum = 0;

// 마이크
int sinMatMike = 0;
/*----------------------------------------------------------------------------*
*							클래스 초기, 종료 
*-----------------------------------------------------------------------------*/	
cCHARSTATUS::cCHARSTATUS()
{
	OpenFlag = 0;

}

cCHARSTATUS::~cCHARSTATUS()
{

}

/*----------------------------------------------------------------------------*
*							     초기화 
*-----------------------------------------------------------------------------*/	
void cCHARSTATUS::Init()
{

	//MatStatus[0] =  CreateTextureMaterial( "Image\\SinImage\\Status\\Status-1.tga" , 0, 0, 0,0, SMMAT_BLEND_ALPHA );
	//MatStatus[1] =  CreateTextureMaterial( "Image\\SinImage\\Status\\Status-2.tga" , 0, 0, 0,0, SMMAT_BLEND_ALPHA );
	//MatStatus[2] =  CreateTextureMaterial( "Image\\SinImage\\Status\\Status-3.tga" , 0, 0, 0,0, SMMAT_BLEND_ALPHA );
	//MatStatus[3] =  CreateTextureMaterial( "Image\\SinImage\\Status\\Status-4.tga" , 0, 0, 0,0, SMMAT_BLEND_ALPHA );

	Load();

	//텍스트 값을 흰색으로 초기화해 준다 
	SkillTextColor[SIN2_ATTACK_RATE] = SIN_TEXT_COLOR_WHITE;
	SkillTextColor[SIN2_DAMAGE] = SIN_TEXT_COLOR_WHITE;
	SkillTextColor[SIN2_DEFENSE] = SIN_TEXT_COLOR_WHITE;
	SkillTextColor[SIN2_ABSORB] = SIN_TEXT_COLOR_WHITE;
	SkillTextColor[SIN2_SPEED] = SIN_TEXT_COLOR_WHITE;


	MatChatBotton_Glay = CreateTextureMaterial( "Image\\SinImage\\Inter\\Chat\\Chatting06.tga" , 0, 0, 0,0, SMMAT_BLEND_ALPHA );

	//마이크
	sinMatMike      = CreateTextureMaterial("Image\\SinImage\\Inter\\mike.tga", 0, 0, 0,0, SMMAT_BLEND_ALPHA); //시계 

}
/*----------------------------------------------------------------------------*
*						    Bmp파일을 로드한다 
*-----------------------------------------------------------------------------*/	
void cCHARSTATUS::Load()
{
	lpRegiBox[0]    = LoadDibSurfaceOffscreen("Image\\SinImage\\Status\\RegiInfo\\BioInfo.bmp");
	lpRegiBox[1]    = LoadDibSurfaceOffscreen("Image\\SinImage\\Status\\RegiInfo\\FireInfo.bmp");
	lpRegiBox[2]    = LoadDibSurfaceOffscreen("Image\\SinImage\\Status\\RegiInfo\\IceInfo.bmp");
	lpRegiBox[3]    = LoadDibSurfaceOffscreen("Image\\SinImage\\Status\\RegiInfo\\PoisonInfo.bmp");
	lpRegiBox[4]    = LoadDibSurfaceOffscreen("Image\\SinImage\\Status\\RegiInfo\\LightInfo.bmp");
	

    lpChangeArrow[0]   = LoadDibSurfaceOffscreen("Image\\SinImage\\Status\\ChangeArrow.bmp");
    lpSelectArrow[0]   = LoadDibSurfaceOffscreen("Image\\SinImage\\Status\\SelectArrow.bmp");
	lpSelectArrow[1]   = LoadDibSurfaceOffscreen("Image\\SinImage\\Status\\SelectArrow2.bmp");
	lpStatusPoint	   = LoadDibSurfaceOffscreen("Image\\SinImage\\Status\\StatusPoint.bmp");

	lpStatusMain	   = LoadDibSurfaceOffscreen("Image\\SinImage\\Status\\Status.bmp");

	lpChatButton[0]  = LoadDibSurfaceOffscreen("Image\\SinImage\\Inter\\Chat\\ChatTap_01.bmp");
	lpChatButton[1]  = LoadDibSurfaceOffscreen("Image\\SinImage\\Inter\\Chat\\ChatTap_02.bmp");
	lpChatButton[2]  = LoadDibSurfaceOffscreen("Image\\SinImage\\Inter\\Chat\\ChatTap_03.bmp");
	lpChatButton[3]  = LoadDibSurfaceOffscreen("Image\\SinImage\\Inter\\Chat\\ChatTap_04.bmp");
	lpChatButton[4]  = LoadDibSurfaceOffscreen("Image\\SinImage\\Inter\\Chat\\ChatTap_05.bmp");

	lpChatButton_Notice[0]  = LoadDibSurfaceOffscreen("Image\\SinImage\\Inter\\Chat\\ChatTap_N11.bmp");
	lpChatButton_Notice[1]  = LoadDibSurfaceOffscreen("Image\\SinImage\\Inter\\Chat\\ChatTap_N12.bmp");
	lpChatButton_Notice[2]  = LoadDibSurfaceOffscreen("Image\\SinImage\\Inter\\Chat\\ChatTap_N22.bmp");

	lpChatLine = LoadDibSurfaceOffscreen("Image\\SinImage\\Inter\\Chat\\n_line.bmp");
}
/*----------------------------------------------------------------------------*
*						     Release
*-----------------------------------------------------------------------------*/	
void cCHARSTATUS::Release()
{
	int i;
	for(i=0 ;i < 5 ; i++){
		if(lpRegiBox[i]){
			lpRegiBox[i]->Release();
			lpRegiBox[i] = 0;

		}

	}
	if(lpChangeArrow[0]){
		lpChangeArrow[0]->Release();
		lpChangeArrow[0] = 0;
	}
	if(lpSelectArrow[0]){
		lpSelectArrow[0]->Release();
		lpSelectArrow[0] = 0;
	}
	if(lpSelectArrow[1]){
		lpSelectArrow[1]->Release();
		lpSelectArrow[1] = 0;
	}
	if(lpStatusPoint){
		lpStatusPoint->Release();

	}

}
/*----------------------------------------------------------------------------*
*							     그리기  
*-----------------------------------------------------------------------------*/	
extern BOOL bMic; //마이크
void cCHARSTATUS::Draw()
{

	int TempLen = 0; 
	int TempCount2 = 0;
	//lstrcpy(cldata.name,"메롱클랜짜장");       
	
	/////////////////// 채팅 (이미지 때문에 최상위로 빼다 음하하) 
	if(sinChatEnter){       
		if(SubChatHeight){                 
			dsDrawTexImage( cInterFace.MatChatDoc[0] , 4, 576-SubChatHeight, 242, 32 , 255 );    //채팅창내용  
			dsDrawTexImage( cInterFace.MatChatDoc[1] , 246, 576-SubChatHeight, 164, 32 , 255 );    //채팅창내용  
		}
		else
			dsDrawTexImage( cInterFace.MatChatDoc[0] , 4, 576-SubChatHeight, 296, 32 , 255 );    //채팅창내용  

	}

	if(cInterFace.ChatFlag){	          
		if(SubChatHeight){ 
				dsDrawTexImage( cInterFace.MatChatSmall[0] , 0, 294, 300, 116 , 255 );     //새끼  채팅창  
				//dsDrawTexImage( cInterFace.MatChatSmall[1] , 256, 600-152-128-21, 128, 128 , 255 );   //새끼  채팅창 
		}
		else{
			dsDrawTexImage( cInterFace.MatChatDefault[0] , 0, 600-192, 305, 210 , 255 );   //기본 채팅창    
			
			//채팅 버튼 
			dsDrawTexImage( MatChatBotton_Glay , 100, 564, 256, 16 , 255 );   //기본 팅창    
			DrawSprite(100+ChatKindIndex*40,564 , lpChatButton[ChatKindIndex],0,0,40,16);
			
			//공지 
			DrawSprite(4,564 , lpChatButton_Notice[NoticeChatIndex],0,0,45,16);

			if(NoticeChatIndex == 1){
				DrawSprite(10,460 , lpChatLine,0,0,287,3);
				//마이크 
				if(bMic)
					dsDrawTexImage( sinMatMike , 283, 464, 32, 32 , 255 );   //기본 팅창    
			}
			else{
				//마이크
				if(bMic)
					dsDrawTexImage( sinMatMike , 283, 409, 32, 32 , 255 );   //기본 팅창    
			}
			
		}
	}

	if(sinMoveKindInter[SIN_CHARSTATUS] != 0){
		DrawSprite(0,600-sinMoveKindInter[SIN_CHARSTATUS]+56 , lpStatusMain,0,0,800,200);
		/*
		dsDrawTexImage( MatStatus[0] , 0, 600-sinMoveKindInter[SIN_CHARSTATUS], 256, 256 , 255 );       
		dsDrawTexImage( MatStatus[1] , 256, 600-sinMoveKindInter[SIN_CHARSTATUS], 256, 256 , 255 );   
		dsDrawTexImage( MatStatus[2] , 256+256, 600-sinMoveKindInter[SIN_CHARSTATUS], 256, 256 , 255 );       
		dsDrawTexImage( MatStatus[3] , 256+256+256, 600-sinMoveKindInter[SIN_CHARSTATUS], 32, 256 , 255 );   
		*/

		if(ShowCharArrow){ 
			//if(pCursorPos.x > 251 && pCursorPos.x < 269 && pCursorPos.y > 565 && pCursorPos.y < 583) 
			DrawSprite(251,565+(256-sinMoveKindInter[SIN_CHARSTATUS]),cShop.lpExit,0,0,20,20);
			DrawSprite(251,565-27+(256-sinMoveKindInter[SIN_CHARSTATUS]),cInvenTory.lpExitInfo,0 ,0 , 47,27);
		}	
			
		if(sinChar->StatePoint){//능력치를 올릴수있으면   
			DrawSprite(520,531+(256-sinMoveKindInter[SIN_CHARSTATUS]),lpStatusPoint,0,0,115,19); 
			for(int i=0 ; i< 5 ;i++){ //받은 스킬을 제외한 5가지 스킬치 
						DrawSprite(PointButton[i][0],PointButton[i][1]+(256-sinMoveKindInter[SIN_CHARSTATUS]),lpChangeArrow[0],0,0,18,17);
			}
			if(sinPointButtonDown){
				if(PointButtonPosi <= 5 &&  PointButtonPosi != 0)
					DrawSprite(PointButton[PointButtonPosi-1][0]+1,PointButton[PointButtonPosi-1][1]+(256-sinMoveKindInter[SIN_CHARSTATUS])+1,lpSelectArrow[0],0,0,18,17);

				if(PointButtonPosi == 6)
					DrawSprite(PointButton[PointButtonPosi-1][0]+1,PointButton[PointButtonPosi-1][1]+(256-sinMoveKindInter[SIN_CHARSTATUS])+1,lpSelectArrow[1],0,0,18,17);
			}

		}
		//클랜마크를 찍어준다 
#ifdef  __CLANSUBCHIP__
		if(cldata.myPosition == 100 || cldata.myPosition == 101 || cldata.myPosition == 104){          
#else
		if(cldata.myPosition == 100 || cldata.myPosition == 101){          
#endif
			if(cldata.hClanMark16){
				TempLen = lstrlen(cldata.name);
				TempCount2 = 6 - (TempLen/2);
				DrawSprite(CheckEditSize(sCharRectPosi[1].Rect.left,sCharRectPosi[1].Rect.right,cldata.name )-((12-TempLen/2))-TempCount2,
					sCharRectPosi[1].Rect.top+15+(256-sinMoveKindInter[SIN_CHARSTATUS]),cldata.hClanMark16,0,0,16,16);
			}
		}

/*	
#ifndef	_LANGUAGE_CHINESE //나쁜띵호와들이 그림을 지멋대루 바꿨음 ( 할수없이 땜빵함)
		switch(RegiInfoPosi){

		case 1:
			DrawSprite(RegiBox[0][0]+3,RegiBox[0][1]-27+(256-sinMoveKindInter[SIN_CHARSTATUS]),lpRegiBox[0],0,0,70,27);
		break;
		case 2:
			DrawSprite(RegiBox[1][0]+3,RegiBox[1][1]-27+(256-sinMoveKindInter[SIN_CHARSTATUS]),lpRegiBox[1],0,0,70,27);
		break;
		case 3:
			DrawSprite(RegiBox[2][0]+3,RegiBox[2][1]-27+(256-sinMoveKindInter[SIN_CHARSTATUS]),lpRegiBox[2],0,0,70,27);
		break;
		case 4:
			DrawSprite(RegiBox[3][0]+3,RegiBox[3][1]-27+(256-sinMoveKindInter[SIN_CHARSTATUS]),lpRegiBox[3],0,0,70,27);
		break;
		case 5:
			DrawSprite(RegiBox[4][0]+3,RegiBox[4][1]-27+(256-sinMoveKindInter[SIN_CHARSTATUS]),lpRegiBox[4],0,0,70,27);
		break;

		}

#else
*/
		switch(RegiInfoPosi){
		case 1:
			DrawSprite(RegiBox[0][0]+3,RegiBox[0][1]-27+(256-sinMoveKindInter[SIN_CHARSTATUS]),lpRegiBox[0],0,0,70,27);
		break;
		case 2:
			DrawSprite(RegiBox[1][0]+3,RegiBox[1][1]-27+(256-sinMoveKindInter[SIN_CHARSTATUS]),lpRegiBox[1],0,0,70,27);
		break;
		case 3:
			DrawSprite(RegiBox[2][0]+3,RegiBox[2][1]-27+(256-sinMoveKindInter[SIN_CHARSTATUS]),lpRegiBox[2],0,0,70,27);
		break;
		case 4:
			DrawSprite(RegiBox[3][0]+3,RegiBox[3][1]-27+(256-sinMoveKindInter[SIN_CHARSTATUS]),lpRegiBox[3],0,0,70,27);
		break;
		case 5:
			DrawSprite(RegiBox[4][0]+3,RegiBox[4][1]-27+(256-sinMoveKindInter[SIN_CHARSTATUS]),lpRegiBox[4],0,0,70,27);
		break;

		}

//#endif
	}

}
/*----------------------------------------------------------------------------*
*							     메인  
*-----------------------------------------------------------------------------*/	
DWORD dwSkillTextTime = 0;
DWORD dwSkillTextTimeFlag = 0;
void cCHARSTATUS::Main()
{

	if(cSkill.SearchContiueSkill(SKILL_FROST_JAVELIN)){ 
		dwSkillTextTime++;
		if(dwSkillTextTime > 70){
			switch(dwSkillTextTimeFlag){
				case 0:
					dwSkillTextTimeFlag = 1;
				break;
				case 1:
					dwSkillTextTimeFlag = 2;
				break;
				case 2:
					dwSkillTextTimeFlag = 1;
				break;
			}
			dwSkillTextTime = 0;
		}
	}
	else dwSkillTextTimeFlag = 0;

	int  i;
	ShowCharArrow=0;
	RegiInfoPosi = 0;
	PointButtonPosi = 0;

	///////////////채팅 필터
	ChatKindIndexNum = 0; 
	if(cInterFace.ChatFlag){	    
		if(!SubChatHeight){
			for(int t = 0 ; t < 5 ; t++){
				if(pCursorPos.x > 100+(t*40) && pCursorPos.x < 100+((t+1)*40) && pCursorPos.y > 563 && pCursorPos.y < 563+16) 
					ChatKindIndexNum = t+1;
					switch(ChatKindIndexNum){
						case 1: //채팅전체
							cHelpPet.PetMessage("*ChatTab_All",0);

						break;
						case 2: //채팅클랜
							cHelpPet.PetMessage("*ChatTab_Clan",0);

						break;
						case 3: //채팅동료
							cHelpPet.PetMessage("*ChatTab_Party",0);

						break;
						case 4: //채팅 거래
							cHelpPet.PetMessage("*ChatTab_Trade",0);

						break;
						case 5: //채팅 귓말
							cHelpPet.PetMessage("*ChatTab_Whisper",0);

						break;
					}
				
			}
		}
	}

	//공지 채팅 
	NoticeChatIndexNum = 0;
	if(cInterFace.ChatFlag){	    
		if(!SubChatHeight){
			if(pCursorPos.x > 0 && pCursorPos.x < 45 && pCursorPos.y > 563 && pCursorPos.y < 563+16) {
				NoticeChatIndexNum = 1;
			}
		}
	}
	if(pCursorPos.x > 251 && pCursorPos.x < 269 && pCursorPos.y > 565 && pCursorPos.y < 583) 
		ShowCharArrow=1;
	if(OpenFlag){			
		for(i=0 ; i < 8 ; i++)
			if(pCursorPos.x >= RegiBox[i][0] && pCursorPos.x <= RegiBox[i][2] &&
				pCursorPos.y >= RegiBox[i][1] && pCursorPos.y <= RegiBox[i][3]){
				RegiInfoPosi = i+1;

			}
		for(i=0 ; i < 6 ; i++)
			if(pCursorPos.x >= PointButton[i][0] && pCursorPos.x <= PointButton[i][2] &&
				pCursorPos.y >= PointButton[i][1] && pCursorPos.y <= PointButton[i][3]){
				PointButtonPosi = i+1;
			}
		
	}
	else{ //캐릭터 스테이터스가 닫혀있으면 보정할수 있는 능력수치 초기화 
		for(i = 0; i < 5 ; i++) 
			TempStatePoint[i] = 0; 

	}

	if(sinLoadOK)
		CheckLevel();

}
/*----------------------------------------------------------------------------*
*							     종료 
*-----------------------------------------------------------------------------*/	
void cCHARSTATUS::Close()
{

}
/*----------------------------------------------------------------------------*
*							   LButtonDown
*-----------------------------------------------------------------------------*/	
void cCHARSTATUS::LButtonDown(int x , int y)
{
	int cnt;

	//*공지채팅
	if(NoticeChatIndexNum){ 
		NoticeChatIndex++;
		if(NoticeChatIndex >=3)NoticeChatIndex=0;

	}

	if(ChatKindIndexNum){
		ChatKindIndex = ChatKindIndexNum-1;
		SetChatMsgFilter(ChatKindIndex);
		switch(ChatKindIndexNum){
			case 1: //채팅전체
				cHelpPet.PetMessage("*ChatTab_All",1);

			break;
			case 2: //채팅클랜
				cHelpPet.PetMessage("*ChatTab_Clan",1);

			break;
			case 3: //채팅동료
				cHelpPet.PetMessage("*ChatTab_Party",1);

			break;
			case 4: //채팅 거래
				cHelpPet.PetMessage("*ChatTab_Trade",1);

			break;
			case 5: //채팅 귓말
				cHelpPet.PetMessage("*ChatTab_Whisper",1);

			break;
		}

	}

	if(OpenFlag){
		if(x > 251 && x < 269 && y > 565 && y < 583) //체크표시를 누르면 창을 닫는다 
			OpenFlag = SIN_CLOSE;
		
		if(smConfig.DebugMode && VRKeyBuff['B']){          
			if(sinChar->StatePoint >9){
				sinPointButtonDown = 1;
				switch(PointButtonPosi){
				case 1: //힘
					CheckCharForm(); //인증 
					sinChar->Strength +=9;
					sinChar->StatePoint -=9;
					for(cnt = 0; cnt < 4 ;cnt++) //한칸씩 앞으로 전진    
						TempStatePoint[cnt] = TempStatePoint[cnt+1];	
					TempStatePoint[4] = POINT_STRENGTH;
					ReformCharForm(); //재인증 
					cInvenTory.SetItemToChar();
					cInvenTory.CheckDamage(); //서버에 최고데미지를 보낸다

				break;
				case 2: //정신력 
					CheckCharForm(); //인증 
					sinChar->Spirit +=9;
					sinChar->StatePoint -=9;
					for(cnt = 0; cnt < 4 ; cnt++)
						TempStatePoint[cnt] = TempStatePoint[cnt+1];	
					TempStatePoint[4] = POINT_SPIRIT;
					ReformCharForm(); //재인증 
					cInvenTory.SetItemToChar();
					cInvenTory.CheckDamage(); //서버에 최고데미지를 보낸다


				break;

				case 3: //재능 
					CheckCharForm(); //인증 
					sinChar->Talent+=9;
					sinChar->StatePoint -=9;
					for(cnt = 0; cnt < 4 ;cnt++) //한칸씩 앞으로 전진    
						TempStatePoint[cnt] = TempStatePoint[cnt+1];	
					TempStatePoint[4] = POINT_TALENT;
					ReformCharForm(); //재인증 
					cInvenTory.SetItemToChar();

				break;

				case 4: //미첩성 
					CheckCharForm(); //인증 
					sinChar->Dexterity+=9;
					sinChar->StatePoint -=9;
					for(cnt = 0; cnt < 4 ;cnt++) //한칸씩 앞으로 전진    
						TempStatePoint[cnt] = TempStatePoint[cnt+1];	
					TempStatePoint[4] = POINT_DEXTERITY;
					ReformCharForm(); //재인증 
					cInvenTory.SetItemToChar();
					cInvenTory.CheckDamage(); //서버에 최고데미지를 보낸다

				break;
			
				case 5: //건강 
					CheckCharForm(); //인증 
					sinChar->Health+=9;
					sinChar->StatePoint -=9;
					for(cnt = 0; cnt < 4 ;cnt++) //한칸씩 앞으로 전진    
						TempStatePoint[cnt] = TempStatePoint[cnt+1];	
					TempStatePoint[4] = POINT_HEALTH;
					ReformCharForm(); //재인증 
					cInvenTory.SetItemToChar();
					cInvenTory.CheckDamage(); //서버에 최고데미지를 보낸다

				break;

				case 6: //스테이터스능력치를 되돌린다 
					for(cnt = 4; cnt >= 0 ; cnt--){
						if(TempStatePoint[cnt]){
							switch(TempStatePoint[cnt]){
							case POINT_STRENGTH:
								CheckCharForm(); //인증 
								sinChar->Strength -=9;
								sinChar->StatePoint +=9;
								TempStatePoint[cnt] = 0;
								ReformCharForm(); //재인증 
								cInvenTory.SetItemToChar();
								cInvenTory.CheckDamage(); //서버에 최고데미지를 보낸다
							break;
							case POINT_SPIRIT:
								CheckCharForm(); //인증 
								sinChar->Spirit -=9;
								sinChar->StatePoint +=9;
								TempStatePoint[cnt] = 0;
								ReformCharForm(); //재인증 
								cInvenTory.SetItemToChar();
								cInvenTory.CheckDamage(); //서버에 최고데미지를 보낸다
							break;
							case POINT_TALENT:
								CheckCharForm(); //인증 
								sinChar->Talent -=9;
								sinChar->StatePoint +=9;
								TempStatePoint[cnt] = 0;
								ReformCharForm(); //재인증 
								cInvenTory.SetItemToChar();
								cInvenTory.CheckDamage(); //서버에 최고데미지를 보낸다
							break;
							case POINT_DEXTERITY:
								CheckCharForm(); //인증 
								sinChar->Dexterity -=9;
								sinChar->StatePoint +=9;
								TempStatePoint[cnt] = 0;
								ReformCharForm(); //재인증 
								cInvenTory.SetItemToChar();
								cInvenTory.CheckDamage(); //서버에 최고데미지를 보낸다

							break;
							case POINT_HEALTH:
								CheckCharForm(); //인증 
								sinChar->Health -=9;
								sinChar->StatePoint +=9;
								TempStatePoint[cnt] = 0;
								ReformCharForm(); //재인증 
								cInvenTory.SetItemToChar();		
								cInvenTory.CheckDamage(); //서버에 최고데미지를 보낸다
							break;

							}
						break;
						}
					}

				break;
				}
			}

		}
		

		if(sinChar->StatePoint >0){
			sinPointButtonDown = 1;
			switch(PointButtonPosi){
			case 1: //힘
				CheckCharForm(); //인증 
				sinChar->Strength ++;
				sinChar->StatePoint --;
				for(cnt = 0; cnt < 4 ;cnt++) //한칸씩 앞으로 전진    
					TempStatePoint[cnt] = TempStatePoint[cnt+1];	
				TempStatePoint[4] = POINT_STRENGTH;
				ReformCharForm(); //재인증 
				cInvenTory.SetItemToChar();
				cInvenTory.CheckDamage(); //서버에 최고데미지를 보낸다

			break;
			case 2: //정신력 
				CheckCharForm(); //인증 
				sinChar->Spirit ++;
				sinChar->StatePoint --;
				for(cnt = 0; cnt < 4 ; cnt++)
					TempStatePoint[cnt] = TempStatePoint[cnt+1];	
				TempStatePoint[4] = POINT_SPIRIT;
				ReformCharForm(); //재인증 
				cInvenTory.SetItemToChar();
				cInvenTory.CheckDamage(); //서버에 최고데미지를 보낸다


			break;

			case 3: //재능 
				CheckCharForm(); //인증 
				sinChar->Talent++;
				sinChar->StatePoint --;
				for(cnt = 0; cnt < 4 ;cnt++) //한칸씩 앞으로 전진    
					TempStatePoint[cnt] = TempStatePoint[cnt+1];	
				TempStatePoint[4] = POINT_TALENT;
				ReformCharForm(); //재인증 
				cInvenTory.SetItemToChar();
				cInvenTory.CheckDamage(); //서버에 최고데미지를 보낸다

			break;

			case 4: //미첩성 
				CheckCharForm(); //인증 
				sinChar->Dexterity++;
				sinChar->StatePoint --;
				for(cnt = 0; cnt < 4 ;cnt++) //한칸씩 앞으로 전진    
					TempStatePoint[cnt] = TempStatePoint[cnt+1];	
				TempStatePoint[4] = POINT_DEXTERITY;
				ReformCharForm(); //재인증 
				cInvenTory.SetItemToChar();
				cInvenTory.CheckDamage(); //서버에 최고데미지를 보낸다

			break;
		
			case 5: //건강 
				CheckCharForm(); //인증 
				sinChar->Health++;
				sinChar->StatePoint --;
				for(cnt = 0; cnt < 4 ;cnt++) //한칸씩 앞으로 전진    
					TempStatePoint[cnt] = TempStatePoint[cnt+1];	
				TempStatePoint[4] = POINT_HEALTH;
				ReformCharForm(); //재인증 
				cInvenTory.SetItemToChar();
				cInvenTory.CheckDamage(); //서버에 최고데미지를 보낸다

			break;

			case 6: //스테이터스능력치를 되돌린다 
				for(cnt = 4; cnt >= 0 ; cnt--){
					if(TempStatePoint[cnt]){
						switch(TempStatePoint[cnt]){
						case POINT_STRENGTH:
							CheckCharForm(); //인증 
							sinChar->Strength --;
							sinChar->StatePoint ++;
							TempStatePoint[cnt] = 0;
							ReformCharForm(); //재인증 
							cInvenTory.SetItemToChar();
						break;
						case POINT_SPIRIT:
							CheckCharForm(); //인증 
							sinChar->Spirit --;
							sinChar->StatePoint ++;
							TempStatePoint[cnt] = 0;
							ReformCharForm(); //재인증 
							cInvenTory.SetItemToChar();
							cInvenTory.CheckDamage(); //서버에 최고데미지를 보낸다
						break;
						case POINT_TALENT:
							CheckCharForm(); //인증 
							sinChar->Talent --;
							sinChar->StatePoint ++;
							TempStatePoint[cnt] = 0;
							ReformCharForm(); //재인증 
							cInvenTory.SetItemToChar();
							cInvenTory.CheckDamage(); //서버에 최고데미지를 보낸다
						break;
						case POINT_DEXTERITY:
							CheckCharForm(); //인증 
							sinChar->Dexterity --;
							sinChar->StatePoint ++;
							TempStatePoint[cnt] = 0;
							ReformCharForm(); //재인증 
							cInvenTory.SetItemToChar();		
							cInvenTory.CheckDamage(); //서버에 최고데미지를 보낸다

						break;
						case POINT_HEALTH:
							CheckCharForm(); //인증 
							sinChar->Health --;
							sinChar->StatePoint ++;
							TempStatePoint[cnt] = 0;
							ReformCharForm(); //재인증 
							cInvenTory.SetItemToChar();		
							cInvenTory.CheckDamage(); //서버에 최고데미지를 보낸다
						break;

						}
					break;
					}
				}

			break;
			}

		}
		
	}
	
	
}
/*----------------------------------------------------------------------------*
*							   LButtonUp
*-----------------------------------------------------------------------------*/	
void cCHARSTATUS::LButtonUp(int x , int y)
{
	sinPointButtonDown = 0; //버튼업일때 초기화 
}
/*----------------------------------------------------------------------------*
*							   RButtonDown
*-----------------------------------------------------------------------------*/	
void cCHARSTATUS::RButtonDown(int x , int y)
{
	if(ChatKindIndexNum){
		switch(ChatKindIndexNum){
			case 1: //채팅전체
				cHelpPet.PetMessage("*ChatTab_All",2);

			break;
			case 2: //채팅클랜
				cHelpPet.PetMessage("*ChatTab_Clan",2);

			break;
			case 3: //채팅동료
				cHelpPet.PetMessage("*ChatTab_Party",2);

			break;
			case 4: //채팅 거래
				cHelpPet.PetMessage("*ChatTab_Trade",2);

			break;
			case 5: //채팅 귓말
				cHelpPet.PetMessage("*ChatTab_Whisper",2);

			break;
		}

	}

}
/*----------------------------------------------------------------------------*
*							   RButtonUp
*-----------------------------------------------------------------------------*/	
void cCHARSTATUS::RButtonUp(int x , int y)
{

}
/*----------------------------------------------------------------------------*
*							    KeyDown
*-----------------------------------------------------------------------------*/	
void cCHARSTATUS::KeyDown()
{

	if(sinGetKeyClick('C')){
		cHelpPet.PetMessage("*CarStatus",3);
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
	}
}

/*----------------------------------------------------------------------------*
*
*-----------------------------------------------------------------------------*/	
DWORD CharStatusColor[10] = {RGB( 255,255,255) , RGB( 255,0,0 ) ,RGB( 255,220,0),
					RGB( 142,230,254 ) , RGB( 128,255,128 ) ,RGB( 255,225,0 ),
					RGB( 255,180,200) , RGB(255,230,10) ,   RGB( 255,220,0 ),
					RGB( 255,220,0 ) 

};

int sinBackupTextColor = 0;

//캐릭터 능력치를 보여준다 
void cCHARSTATUS::DrawCharText()
{

	CheckChageStateDisplay(); //디스플레이될 색을 체크한다
	if(!sinMoveKindInter[SIN_CHARSTATUS])return; //캐릭터 스테이터스가 닫혀있으면 리턴한다 

	int TempLen = 0;
	HDC	hdc;
	char strBuff[128];
	memset(strBuff,0,sizeof(strBuff));
	lpDDSBack->GetDC( &hdc );
	SelectObject( hdc, sinFont);
    SetBkMode( hdc, TRANSPARENT );
	SetTextColor( hdc, RGB(255,255,255) );
	

	/*캐릭터 기본 정보 */
	wsprintf(strBuff,"%s",cInvenTory.JobName); //직업 코드 
	dsTextLineOut(hdc,CheckEditSize(sCharRectPosi[0].Rect.left,sCharRectPosi[0].Rect.right, strBuff),sCharRectPosi[0].Rect.top+(256-sinMoveKindInter[SIN_CHARSTATUS]),
				strBuff,lstrlen(strBuff));
	
	wsprintf(strBuff,"%s",sinCharDisplay.szName); //캐릭터 이름  
	dsTextLineOut(hdc,CheckEditSize(sCharRectPosi[1].Rect.left,sCharRectPosi[1].Rect.right, strBuff),sCharRectPosi[1].Rect.top+(256-sinMoveKindInter[SIN_CHARSTATUS]),
				strBuff,lstrlen(strBuff));

#ifdef  __CLANSUBCHIP__
	if(cldata.myPosition == 100 || 
		cldata.myPosition == 101 || cldata.myPosition == 104){  
#else
	if(cldata.myPosition == 100 || cldata.myPosition == 101){  
#endif
		TempLen = lstrlen(cldata.name);
		wsprintf(strBuff,"%s",cldata.name ); //캐릭터 이름 
		dsTextLineOut(hdc,CheckEditSize(sCharRectPosi[1].Rect.left,sCharRectPosi[1].Rect.right, strBuff)+TempLen,sCharRectPosi[1].Rect.top+20+(256-sinMoveKindInter[SIN_CHARSTATUS]),
				strBuff,lstrlen(strBuff));
	}

	wsprintf(strBuff,"%d",sinCharDisplay.Level); //레벨 
	dsTextLineOut(hdc,CheckEditSize(sCharRectPosi[2].Rect.left,sCharRectPosi[2].Rect.right, strBuff),sCharRectPosi[2].Rect.top+(256-sinMoveKindInter[SIN_CHARSTATUS]),
				strBuff,lstrlen(strBuff));

	
	if(sinChar->Level > 79){
		wsprintf(strBuff,"%dM",cCharStatus.sinGetNowExp()); //현재 경험치 
		dsTextLineOut(hdc,CheckEditSize(sCharRectPosi[3].Rect.left,sCharRectPosi[3].Rect.right, strBuff),sCharRectPosi[3].Rect.top+(256-sinMoveKindInter[SIN_CHARSTATUS]),
					strBuff,lstrlen(strBuff));

	}
	else{
		wsprintf(strBuff,"%d",cCharStatus.sinGetNowExp()); //현재 경험치 
		dsTextLineOut(hdc,CheckEditSize(sCharRectPosi[3].Rect.left,sCharRectPosi[3].Rect.right, strBuff),sCharRectPosi[3].Rect.top+(256-sinMoveKindInter[SIN_CHARSTATUS]),
					strBuff,lstrlen(strBuff));

	}
	

	if(sinChar->Level > 79){
		wsprintf(strBuff,"%dM",cCharStatus.sinGetNextExp()); //다음 레벨업 경험치  
		dsTextLineOut(hdc,CheckEditSize(sCharRectPosi[4].Rect.left,sCharRectPosi[4].Rect.right, strBuff),sCharRectPosi[4].Rect.top+(256-sinMoveKindInter[SIN_CHARSTATUS]),
					strBuff,lstrlen(strBuff));
	}
	else{
		wsprintf(strBuff,"%d",cCharStatus.sinGetNextExp()); //다음 레벨업 경험치  
		dsTextLineOut(hdc,CheckEditSize(sCharRectPosi[4].Rect.left,sCharRectPosi[4].Rect.right, strBuff),sCharRectPosi[4].Rect.top+(256-sinMoveKindInter[SIN_CHARSTATUS]),
					strBuff,lstrlen(strBuff));

	}

	if(AddVirtualLife[1]){
		AddVirtualLife[0] = ((int)sinGetLife() * sinVirtualLifePercent)/100;

	}

	// 박재원 - 부스터 아이템(기력) 
	if(AddVirtualMana[1]){
		AddVirtualMana[0] = ((int)sinGetMana() * sinVirtualManaPercent)/100;

	}

	// 박재원 - 부스터 아이템(근력)
	if(AddVirtualStamina[1]){
		AddVirtualStamina[0] = ((int)sinGetStamina() * sinVirtualStaminaPercent)/100;

	}

	SetTextColor( hdc, CharStatusColor[SkillTextColor[SIN2_LIFE]]); 
	wsprintf(strBuff,"%d/%d",sinGetLife()+AddVirtualLife[0],sinCharDisplay.Life[1]+AddVirtualLife[1]); // 라이프 
	dsTextLineOut(hdc,CheckEditSize(sCharRectPosi[5].Rect.left,sCharRectPosi[5].Rect.right, strBuff),sCharRectPosi[5].Rect.top+(256-sinMoveKindInter[SIN_CHARSTATUS]),
				strBuff,lstrlen(strBuff));
//	SetTextColor( hdc, RGB(255,255,255) );
	
	// 박재원 - 부스터 아이템(기력)
	SetTextColor( hdc, CharStatusColor[SkillTextColor[SIN2_MANA]]); 
	wsprintf(strBuff,"%d/%d",sinGetMana(),sinCharDisplay.Mana[1]); //현재 마나  
	dsTextLineOut(hdc,CheckEditSize(sCharRectPosi[6].Rect.left,sCharRectPosi[6].Rect.right, strBuff),sCharRectPosi[6].Rect.top+(256-sinMoveKindInter[SIN_CHARSTATUS]),
		strBuff,lstrlen(strBuff));

	// 박재원 - 부스터 아이템(근력)
	SetTextColor( hdc, CharStatusColor[SkillTextColor[SIN2_STAMINA]]); 
	wsprintf(strBuff,"%d/%d",sinGetStamina(),sinCharDisplay.Stamina[1]); //현재 스테미나  
	dsTextLineOut(hdc,CheckEditSize(sCharRectPosi[7].Rect.left,sCharRectPosi[7].Rect.right, strBuff),sCharRectPosi[7].Rect.top+(256-sinMoveKindInter[SIN_CHARSTATUS]),
		strBuff,lstrlen(strBuff));

	SetTextColor( hdc, RGB(255,255,255) );

	
	/*캐릭터 능력치 */
	wsprintf(strBuff,"%d",sinCharDisplay.Strength); //힘 
	dsTextLineOut(hdc,CheckEditSize(sCharRectPosi[8].Rect.left,sCharRectPosi[8].Rect.right, strBuff),sCharRectPosi[8].Rect.top+(256-sinMoveKindInter[SIN_CHARSTATUS]),
				strBuff,lstrlen(strBuff));


	wsprintf(strBuff,"%d",sinCharDisplay.Spirit); //정신력  
	dsTextLineOut(hdc,CheckEditSize(sCharRectPosi[9].Rect.left,sCharRectPosi[9].Rect.right, strBuff),sCharRectPosi[9].Rect.top+(256-sinMoveKindInter[SIN_CHARSTATUS]),
				strBuff,lstrlen(strBuff));

	

	wsprintf(strBuff,"%d",sinCharDisplay.Talent); //재능   
	dsTextLineOut(hdc,CheckEditSize(sCharRectPosi[10].Rect.left,sCharRectPosi[10].Rect.right, strBuff),sCharRectPosi[10].Rect.top+(256-sinMoveKindInter[SIN_CHARSTATUS]),
				strBuff,lstrlen(strBuff));

	

	wsprintf(strBuff,"%d",sinCharDisplay.Dexterity); //민첩성   
	dsTextLineOut(hdc,CheckEditSize(sCharRectPosi[11].Rect.left,sCharRectPosi[11].Rect.right, strBuff),sCharRectPosi[11].Rect.top+(256-sinMoveKindInter[SIN_CHARSTATUS]),
				strBuff,lstrlen(strBuff));

	wsprintf(strBuff,"%d",sinCharDisplay.Health); //건강    
	dsTextLineOut(hdc,CheckEditSize(sCharRectPosi[12].Rect.left,sCharRectPosi[12].Rect.right, strBuff),sCharRectPosi[12].Rect.top+(256-sinMoveKindInter[SIN_CHARSTATUS]),
				strBuff,lstrlen(strBuff));

	if(sinChar->StatePoint){
		wsprintf(strBuff,"%d",sinChar->StatePoint); //포인트      
		dsTextLineOut(hdc,CheckEditSize(sCharRectPosi[23].Rect.left,sCharRectPosi[23].Rect.right, strBuff),sCharRectPosi[23].Rect.top+(256-sinMoveKindInter[SIN_CHARSTATUS]),
					strBuff,lstrlen(strBuff));

	}
//#define SIN_TEXT_COLOR_WHITE		0
//#define SIN_TEXT_COLOR_RED			1
//#define SIN_TEXT_COLOR_YELLOW		2
//#define SIN_TEXT_COLOR_BLUE			3

//#define SIN2_ATTACK_RATE		0
//#define SIN2_DAMAGE				1
//#define SIN2_DEFENSE			2
//#define SIN2_ABSORB				3
//#define SIN2_SPEED				4

//SkillTextColor[SIN2_ATTACK_RATE]
	/*공격 방어 */

	
	SetTextColor( hdc, CharStatusColor[SkillTextColor[SIN2_ATTACK_RATE]]);
	wsprintf(strBuff,"%d",sinCharDisplay.Attack_Rating+sDisplayStateShow.Attack_Rate); //어택 레이팅  
	dsTextLineOut(hdc,CheckEditSize(sCharRectPosi[18].Rect.left,sCharRectPosi[18].Rect.right, strBuff),sCharRectPosi[18].Rect.top+(256-sinMoveKindInter[SIN_CHARSTATUS]),
				strBuff,lstrlen(strBuff));

	
	//프로스트 재블린 
	if(dwSkillTextTimeFlag == 1){
		SkillTextColor[SIN2_DAMAGE] = SIN_TEXT_COLOR_BLUE;
	}
	if(dwSkillTextTimeFlag == 2){
		SkillTextColor[SIN2_DAMAGE] = SIN_TEXT_COLOR_YELLOW;
	}

	//데미지가 마이너스가 될경우를 보정해준다
	int DisplayDamage2[2] = {0,0};
	SetTextColor( hdc, CharStatusColor[SkillTextColor[SIN2_DAMAGE]]);
	DisplayDamage2[0] = sinCharDisplay.Attack_Damage[0]+sinTempDamage2[0]+sDisplayStateShow.Damage[0];
	DisplayDamage2[1] = sinCharDisplay.Attack_Damage[1]+sinTempDamage2[1]+sDisplayStateShow.Damage[1];
	if(DisplayDamage2[0] < 0 )DisplayDamage2[0] = 0;
	if(DisplayDamage2[1] < 0 )DisplayDamage2[1] = 0;
	wsprintf(strBuff,"%d-%d",DisplayDamage2[0],DisplayDamage2[1]); //데미지
	dsTextLineOut(hdc,CheckEditSize(sCharRectPosi[19].Rect.left,sCharRectPosi[19].Rect.right, strBuff),sCharRectPosi[19].Rect.top+(256-sinMoveKindInter[SIN_CHARSTATUS]),
				strBuff,lstrlen(strBuff));

	SetTextColor( hdc, CharStatusColor[SkillTextColor[SIN2_DEFENSE]]);
	wsprintf(strBuff,"%d",sinCharDisplay.Defence+sDisplayStateShow.Defense); //디펜스 
	dsTextLineOut(hdc,CheckEditSize(sCharRectPosi[20].Rect.left,sCharRectPosi[20].Rect.right, strBuff),sCharRectPosi[20].Rect.top+(256-sinMoveKindInter[SIN_CHARSTATUS]),
				strBuff,lstrlen(strBuff));

	SetTextColor( hdc, CharStatusColor[SkillTextColor[SIN2_ABSORB]]);
	wsprintf(strBuff,"%d",sinCharDisplay.Absorption+sinTempAbsorb+sDisplayStateShow.Absorb); //흡수력 
	dsTextLineOut(hdc,CheckEditSize(sCharRectPosi[21].Rect.left,sCharRectPosi[21].Rect.right, strBuff),sCharRectPosi[21].Rect.top+(256-sinMoveKindInter[SIN_CHARSTATUS]),
				strBuff,lstrlen(strBuff));


	SetTextColor( hdc, CharStatusColor[SkillTextColor[SIN2_SPEED]]);
	wsprintf(strBuff,"%d",sinCharDisplay.Move_Speed); //속도 
	dsTextLineOut(hdc,CheckEditSize(sCharRectPosi[22].Rect.left,sCharRectPosi[22].Rect.right, strBuff),sCharRectPosi[22].Rect.top+(256-sinMoveKindInter[SIN_CHARSTATUS]),
				strBuff,lstrlen(strBuff));
	


	//저항력 
	SetTextColor( hdc, RGB(255,255,255) );
	wsprintf(strBuff,"%d",sinCharDisplay.Resistance[0]); //생체  
	dsTextLineOut(hdc,CheckEditSize(sCharRectPosi[13].Rect.left,sCharRectPosi[13].Rect.right, strBuff),sCharRectPosi[13].Rect.top+(256-sinMoveKindInter[SIN_CHARSTATUS]),
				strBuff,lstrlen(strBuff));

	wsprintf(strBuff,"%d",sinCharDisplay.Resistance[5]); //독 
	dsTextLineOut(hdc,CheckEditSize(sCharRectPosi[14].Rect.left,sCharRectPosi[14].Rect.right, strBuff),sCharRectPosi[14].Rect.top+(256-sinMoveKindInter[SIN_CHARSTATUS]),
				strBuff,lstrlen(strBuff));

	wsprintf(strBuff,"%d",sinCharDisplay.Resistance[2]); //불 
	dsTextLineOut(hdc,CheckEditSize(sCharRectPosi[15].Rect.left,sCharRectPosi[15].Rect.right, strBuff),sCharRectPosi[15].Rect.top+(256-sinMoveKindInter[SIN_CHARSTATUS]),
				strBuff,lstrlen(strBuff));

	wsprintf(strBuff,"%d",sinCharDisplay.Resistance[4]); //번개 
	dsTextLineOut(hdc,CheckEditSize(sCharRectPosi[16].Rect.left,sCharRectPosi[16].Rect.right, strBuff),sCharRectPosi[16].Rect.top+(256-sinMoveKindInter[SIN_CHARSTATUS]),
				strBuff,lstrlen(strBuff));

	wsprintf(strBuff,"%d",sinCharDisplay.Resistance[3]); //냉기 
	dsTextLineOut(hdc,CheckEditSize(sCharRectPosi[17].Rect.left,sCharRectPosi[17].Rect.right, strBuff),sCharRectPosi[17].Rect.top+(256-sinMoveKindInter[SIN_CHARSTATUS]),
				strBuff,lstrlen(strBuff));

	///////////글자색을 바꿀수있게 할 예정이다 두둥!!


	lpDDSBack->ReleaseDC( hdc );

}

//캐릭터스테이스터스에 보이지않는 인자를 보여준다 
void cCHARSTATUS::OtherStateShow()
{
	HDC	hdc;
	char strBuff[128];
	memset(strBuff,0,sizeof(strBuff));
	lpDDSBack->GetDC( &hdc );
    SetBkMode( hdc, TRANSPARENT );
	SetTextColor( hdc, RGB(255,255,255) );
	wsprintf(strBuff,"\260\370\260\335\274\323\265\265:%d",sinCharDisplay.Attack_Speed); //공격속도
	dsTextLineOut(hdc,100,300,strBuff,lstrlen(strBuff));

	wsprintf(strBuff,"\272\355\267\260\300\262:%d",sinCharDisplay.Chance_Block); //블럭율
	dsTextLineOut(hdc,200,300,strBuff,lstrlen(strBuff));

	wsprintf(strBuff,"\305\251\270\256\306\274\304\303:%d",sinCharDisplay.Critical_Hit); //크리티컬
	dsTextLineOut(hdc,300,300,strBuff,lstrlen(strBuff));

	wsprintf(strBuff,"\275\264\306\303\261\346\300\314:%d",sinCharDisplay.Shooting_Range); //사정거리 
	dsTextLineOut(hdc,400,300,strBuff,lstrlen(strBuff));

	lpDDSBack->ReleaseDC( hdc );
	
}

//레벨을 체크한다 
void cCHARSTATUS::CheckLevel()
{
	int UpLevelPoint;
	if(OldLevelTemp-176 != OldLevel)return;
	if(sinChar->Level > OldLevel){
		cInvenTory.SetItemToChar(); //레벨이 올라감에 따른 능력치 반응 
		UpLevelPoint = sinChar->Level - OldLevel;
		CheckCharForm(); //인증 
		if((sinChar->Level%2) == 0 && sinChar->Level >= 10){	//레벨이 올라간 만큼스킬포인트를 올려준다 	
				sinSkill.SkillPoint++;   //스킬 포인트를 올려준다 
		}
		if((sinChar->Level%2) == 0 && sinChar->Level >= 60){	//레벨이 올라간 만큼스킬포인트를 올려준다 	
				sinSkill.SkillPoint4++;   //스킬 포인트를 올려준다 
		}
		

		sinLevelPoint = sinChar->Level - OldLevel;
		
		if((sinQuest_levelLog & QUESTBIT_LEVEL_80_2) == QUESTBIT_LEVEL_80_2){
			if((sinQuest_levelLog & QUESTBIT_LEVEL_90_2) == QUESTBIT_LEVEL_90_2){
				if(sinChar->Level >=90){ //퀘스트를 수행하고난후에는 포인트가 10씩올라간다
					sinChar->StatePoint +=3;
				}
			}
			if(sinChar->Level >=80){ //퀘스트를 수행하고난후에는 포인트가 7씩올라간다
				sinChar->StatePoint += 2;
			}
		}
		sinChar->StatePoint += sinLevelPoint*5; 
		OldLevel = sinChar->Level;
		OldLevelTemp = OldLevel+176;
		ReformCharForm(); //재인증 
		sinSetLife(sinChar->Life[1]);	//라이프와 마나는 자기가 체크한다 
		sinSetMana(sinChar->Mana[1]);
		sinSetStamina(sinChar->Stamina[1]);
		cInvenTory.CheckDamage(); //서버에 최고데미지를 보낸다

		//Pet 메세지 관련
		if(cHelpPet.PetKind && cHelpPet.PetShow){
			cHelpPet.LevelFlag = 1;
			switch(sinChar->Level){
				case 2:
					cHelpPet.PetMessageHelp("*Level_Message2-1");
					cHelpPet.PetMessageHelp("*Level_Message2-2");
				break;
				case 3:
					cHelpPet.PetMessageHelp("*Level_Message3");
				break;
				case 4:
					cHelpPet.PetMessageHelp("*Level_Message4");
				break;
				case 5:
					cHelpPet.PetMessageHelp("*Level_Message5");
				break;
				case 6:
					cHelpPet.PetMessageHelp("*Level_Message6-1");
					cHelpPet.PetMessageHelp("*Level_Message6-2");
				break;
				case 7:
					cHelpPet.PetMessageHelp("*Level_Message7");
				break;
				case 8:
					cHelpPet.PetMessageHelp("*Level_Message8");
				break;
				case 9:
					cHelpPet.PetMessageHelp("*Level_Message9-1");
					cHelpPet.PetMessageHelp("*Level_Message9-2");
				break;
				case 10:
					/*
					if(JOB_IS_TEMPSKRON(sinChar->JOB_CODE) ){ //템스크론
						cHelpPet.PetMessageHelp("*Level_Message10_T");
					}
					else{ //모라이온
						cHelpPet.PetMessageHelp("*Level_Message10_M");
					}
					*/
					cHelpPet.PetMessageHelp("*Level_Message10-1");
					cHelpPet.PetMessageHelp("*Level_Message10-2");
				break;
			}

		}		
		//에너지 그래프 첵크 초기화 *****************************************
		ResetEnergyGraph( 3 );

		//유광열 레벨업을 했을 경우 레벨에 관련된 도움말을 띄워준다
		void InitHelp();
		InitHelp();
		//====================end
		
		
	}
}

//캐릭터 능력치를 백업한다 (해킹방지를 위해서 )
void cCHARSTATUS::BackUpsinCharState(smCHAR *pChar)
{






}


//현제의 경험치를 얻어온다 
int cCHARSTATUS::sinGetNowExp()
{

	//level<80
	//INT64 aa = GetXorExp64();
	//int ab = (int)(aa/1000000);
	//(int)  /INT64 GetXorExp64( smCHAR_INFO *smCharInfo )


	INT64 sinExp64 = 0;
	int   sinExp = 0;
	sinExp64 = GetXorExp64(sinChar);
	if(sinChar->Level > 79 ){
		sinExp  = (int)(sinExp64/1000000);

	}
	else{
		sinExp  = (int)(sinExp64);
	}

	return sinExp;
	
}

//다음 레벨의 경험치를 얻어온다 
int cCHARSTATUS::sinGetNextExp()
{
	//(int) /INT64 GetNextExp( int Level )

	int sinExp = 0;
	if(sinChar->Level > 79 ){

		sinExp = (int)((GetNextExp( sinChar->Level )/1000000));

	}
	else{
		sinExp  = (int)GetNextExp( sinChar->Level );
	}

	return sinExp;
}


//캐릭터 스테이터스를 초기화한다 
int cCHARSTATUS::InitCharStatus(int kind)
{
	int Total,Total2;

	//kind 0이벤트 초기화 kind 1블루스톤 초기화
	switch(kind){
		case 0:
			if(sinChar->wVersion[1] == 1){
				sinSkillPontInitButtonFlag = 0;
				cMessageBox.ShowMessage(MESSAGE_ALREADY_INITSTATE);
				return FALSE;
			}
			sinSkillPontInitButtonFlag = 1;
		 	sinInitPointPassFlag = 1;
		break;
	}
	cMessageBox.ShowMessage(MESSAGE_GYUNGFUM_OK);
	Total = sinChar->Strength+sinChar->Spirit+sinChar->Talent+sinChar->Dexterity+sinChar->Health+sinChar->StatePoint;

	for(int i = 0 ; i < NEW_CHAR_JOB_MAX ; i++){
		if(sinChar->JOB_CODE == TempNewCharacterInit[i][0] ){
			sinChar->Strength  = TempNewCharacterInit[i][1];
			sinChar->Spirit    = TempNewCharacterInit[i][2];
			sinChar->Talent    = TempNewCharacterInit[i][3];
			sinChar->Dexterity = TempNewCharacterInit[i][4];
			sinChar->Health    = TempNewCharacterInit[i][5];
			Total2 = sinChar->Strength+sinChar->Spirit+sinChar->Talent+sinChar->Dexterity+sinChar->Health;
			sinChar->StatePoint = Total-Total2;
			sinChar->wVersion[1] = 1;
			ReformCharForm();//재인증 
			cInvenTory.SetItemToChar();
			return TRUE;
		}
		if(sinChar->JOB_CODE == MorNewCharacterInit[i][0] ){
			sinChar->Strength  = MorNewCharacterInit[i][1];
			sinChar->Spirit    = MorNewCharacterInit[i][2];
			sinChar->Talent    = MorNewCharacterInit[i][3];
			sinChar->Dexterity = MorNewCharacterInit[i][4];
			sinChar->Health    = MorNewCharacterInit[i][5];
			Total2 = sinChar->Strength+sinChar->Spirit+sinChar->Talent+sinChar->Dexterity+sinChar->Health;
			sinChar->StatePoint = Total-Total2;
			sinChar->wVersion[1] = 1;
			ReformCharForm();//재인증 
			cInvenTory.SetItemToChar();
			return TRUE;
		}

	}

	return TRUE;
}

//캐릭터의 현제 상태를 체크해 Stats를 디스플레이해준다
int cCHARSTATUS::CheckChageStateDisplay()
{
	//시간체크 
	memset(&sDisplayStateShow,0,sizeof(sDISPLAYSTATE));    
	for(int i = 0 ; i < 10 ; i++){
		if(sDisplayState[i].MaxTime){
			sDisplayStateShow.Absorb      += sDisplayState[i].Absorb;
			sDisplayStateShow.Attack_Rate += sDisplayState[i].Attack_Rate;
			sDisplayStateShow.Damage[0]   += sDisplayState[i].Damage[0];
			sDisplayStateShow.Damage[1]   += sDisplayState[i].Damage[1];
			if(sDisplayState[i].PercentDamage[0])
				sDisplayStateShow.Damage[0]   += (short)(sinChar->Attack_Damage[0]*sDisplayState[i].PercentDamage[0])/100;
			if(sDisplayState[i].PercentDamage[1])
				sDisplayStateShow.Damage[1]   += (short)(sinChar->Attack_Damage[1]*sDisplayState[i].PercentDamage[1])/100;
			
			sDisplayStateShow.Defense     += sDisplayState[i].Defense;

			if(sDisplayStateShow.Absorb <0)
				SkillTextColor[SIN2_ABSORB] = SIN_TEXT_COLOR_RED;
			if(sDisplayStateShow.Defense <0)
				SkillTextColor[SIN2_DEFENSE] = SIN_TEXT_COLOR_RED;
			if(sDisplayStateShow.Damage[0] <0 || sDisplayStateShow.Damage[1] )
				SkillTextColor[SIN2_DAMAGE] = SIN_TEXT_COLOR_RED;


			if(sDisplayState[i].MaxTime <= dwPlayTime){
				memset(&sDisplayState[i],0,sizeof(sDISPLAYSTATE));
				cInvenTory.SetItemToChar();
			}

		}

	}
	//LV 90퀘스트 저주 
	if(sinQuest_Level90_2.CODE && sinQuest_Level90_2.State >= 2 ){
		SkillTextColor[SIN2_DAMAGE] = SIN_TEXT_COLOR_RED;

	}
	return TRUE;
}


// 장별 - 그라비티 스크롤
int cCHARSTATUS::UseGravityScrool()
{

	float	fstrength, fhealth,  ftalent , fLevel ,fSpirit;


	fstrength = (float)sinChar->Strength;
	fhealth   = (float)sinChar->Health;
	ftalent   = (float)sinChar->Talent;
	fSpirit   = (float)sinChar->Spirit;
	fLevel    = (float)sinChar->Level;


	if( sinChar->GravityScroolCheck[0] == 0 ) 
	{
		sinChar->Weight[1] = (short)(((fstrength*2) +(fhealth*1.5) + fLevel*3)+60);
		
	}

	if( sinChar->GravityScroolCheck[0] > 0 )
	{
		sinChar->Weight[1] = (short)(((fstrength*2) +(fhealth*1.5) + fLevel*3)+60) + ( sinChar->GravityScroolCheck[0] )*50;
		
	}



	sinChar->wVersion[1] = 1;
	ReformCharForm();//재인증 11111111111111
	cInvenTory.SetItemToChar();

	return TRUE;
}


// 박재원 - 캐릭터 속성별 스탯 초기화 아이템
int cCHARSTATUS::InitCharStatus_Attribute(int kind)
{
	int Total,Total2;
	int Brood_code = 0; // 종족 구분
	int Job_Num = 0;

	cMessageBox.ShowMessage(MESSAGE_GYUNGFUM_OK);
	Total = sinChar->Strength+sinChar->Spirit+sinChar->Talent+sinChar->Dexterity+sinChar->Health+sinChar->StatePoint;
	
	for(int i = 0 ; i < NEW_CHAR_JOB_MAX ; i++)
	{
		if(sinChar->JOB_CODE == TempNewCharacterInit[i][0])
		{
			Brood_code = BROOD_CODE_TEMPSKRON;
			Job_Num = i;
		}
		if(sinChar->JOB_CODE == MorNewCharacterInit[i][0])
		{
			Brood_code = BROOD_CODE_MORAYION;
			Job_Num = i;
		}
	}

	if(Brood_code == BROOD_CODE_TEMPSKRON)  // 템스크론
	{
		switch(kind)
		{
		case 1: // 힘
			sinChar->Strength  = TempNewCharacterInit[Job_Num][1];
			break;
		case 2: // 정신력
			sinChar->Spirit    = TempNewCharacterInit[Job_Num][2];
			break;
		case 3: // 재능
			sinChar->Talent    = TempNewCharacterInit[Job_Num][3];
			break;
		case 4: // 민첩성
			sinChar->Dexterity = TempNewCharacterInit[Job_Num][4];
			break;
		case 5: // 건강
			sinChar->Health    = TempNewCharacterInit[Job_Num][5];
			break;		
		}
	}
	else // 모라이온
	{
		switch(kind)
		{
		case 1: // 힘
			sinChar->Strength  = MorNewCharacterInit[Job_Num][1];
			break;
		case 2: // 정신력
			sinChar->Spirit    = MorNewCharacterInit[Job_Num][2];
			break;
		case 3: // 재능
			sinChar->Talent    = MorNewCharacterInit[Job_Num][3];
			break;
		case 4: // 민첩성
			sinChar->Dexterity = MorNewCharacterInit[Job_Num][4];
			break;
		case 5: // 건강
			sinChar->Health    = MorNewCharacterInit[Job_Num][5];
			break;		
		}
	}	

	sinPlaySound(SIN_SOUND_EAT_POTION2);//대박 사운드
	StartEffect(lpCurPlayer->pX, lpCurPlayer->pY, lpCurPlayer->pZ, EFFECT_AGING); //에이징이펙트

	Total2 = sinChar->Strength+sinChar->Spirit+sinChar->Talent+sinChar->Dexterity+sinChar->Health;
	sinChar->StatePoint = Total-Total2;
	sinChar->wVersion[1] = 1;
	ReformCharForm();//재인증 
	cInvenTory.SetItemToChar();

	return TRUE;

}
