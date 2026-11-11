/*----------------------------------------------------------------------------*
*	파일명 :  sinMessageBox.cpp
*	하는일 :  각종 메세지 박스들을 관리한다 
*	작성일 :  최종업데이트 12월
*	적성자 :  박상열 
*-----------------------------------------------------------------------------*/	

#include "sinLinkHeader.h"
#include "..\\skillsub.h"
#include "..\\tjboy\\clanmenu\\tjclan.h"
#include "..\\tjboy\\clanmenu\\clan_Enti.h"
#include "..\\field.h"

/*----------------------------------------------------------------------------*
*								전역변수 			
*-----------------------------------------------------------------------------*/	
#define SMASHTV_MONEY	300

sITEM MessageBoxItem; //메세지 박스에서 쓰일 구조체 
sITEM *pMessageItem;  //메세지 박스에서 쓰일 아이템 포인터 
sITEM *pMessageItem2;  //메세지 박스에서 쓰일 아이템 포인터 (건들기 시러서 한개 더맹금)

cMESSAGEBOX cMessageBox;
int MessageTimeCount = 0 ;

int sinMessageBoxShowFlag = 0; //현재 메세지박스가 있나 없나를 플랙으로 알수있다 

int MessageArrowPosi = 0;      //박스의 화살표 포지션 
int MessageButtonPosi = 0;     //버튼 포지션 

int CountTemp = 0;

int CheckCountSize[10] = {0,10,100,1000,10000,100000,1000000,10000000,100000000,1000000000};
int MoneyWeight = 10000;

int ExpressItemFlag = 0;

int sinSkillPontInitButtonFlag = 0; //스킬초기화 메세지 박스 
int sinGiveMoneyCheckFlag = 0; //기부금 한번더 체크
int sinGiveMoneyAmount = 0;
char szGiveMoneyBuff[32];

int sinInitPointFlag7 = 0;
int sinInitPointPassFlag = 0; //첫번 한번만 포인트 체크를 패스한다


sMESSAGEBOX_RECVITEM sMessageBox_RecvItem;

LPDIRECTDRAWSURFACE4 lpButtonYes07;
LPDIRECTDRAWSURFACE4 lpButtonYesGlay07;
LPDIRECTDRAWSURFACE4 lpButtonNo07;
LPDIRECTDRAWSURFACE4 lpButtonNoGlay07;
LPDIRECTDRAWSURFACE4 lpButtonBox07;

//별아이템
LPDIRECTDRAWSURFACE4 lpStarIcon;

LPDIRECTDRAWSURFACE4 lpStarShopClose_;
LPDIRECTDRAWSURFACE4 lpStarShopClose;


//별아이템
sSTAR_SHOP sStarShop;

//사라진 아이템
sLOST_ITEM sLost_Item;
int GiftPrice = 0;
int MatMyShopPRBoxLine = 0;
/*----------------------------------------------------------------------------*
*							클래스 초기, 종료 
*-----------------------------------------------------------------------------*/	
cMESSAGEBOX::cMESSAGEBOX()
{

}
cMESSAGEBOX::~cMESSAGEBOX()
{

}
/*----------------------------------------------------------------------------*
*							     초기화 
*-----------------------------------------------------------------------------*/	
void cMESSAGEBOX::Init()
{
	/////////////////////// 상태만 표시하는 메세지 
	MatBoxLeft	= CreateTextureMaterial( "Image\\SinImage\\MessageBox\\CutBox\\BoxLeft.tga" , 0, 0, 0,0, SMMAT_BLEND_ALPHA );   //좌측 박스 그림 
	MatBoxRight	= CreateTextureMaterial( "Image\\SinImage\\MessageBox\\CutBox\\BoxRight.tga" , 0, 0, 0,0, SMMAT_BLEND_ALPHA );   //우측 박스 그림 
	MatBoxCenter = CreateTextureMaterial( "Image\\SinImage\\MessageBox\\CutBox\\BoxCenter.tga" , 0, 0, 0,0, SMMAT_BLEND_ALPHA );  //하단 박스 그림 
	
	MatBoxMain = CreateTextureMaterial( "Image\\SinImage\\MessageBox\\BoxMain.tga" , 0, 0, 0,0, SMMAT_BLEND_ALPHA );  //박스 메인 

	//홍보박스 라인
	MatMyShopPRBoxLine = CreateTextureMaterial( "Image\\SinImage\\Inven\\PR_Box.tga" , 0, 0, 0,0, SMMAT_BLEND_ALPHA );  //박스 메인 

	Load();
}
/*----------------------------------------------------------------------------*
*						    Bmp파일을 로드한다 
*-----------------------------------------------------------------------------*/	
void cMESSAGEBOX::Load()
{

	lpButtonYes07		= LoadDibSurfaceOffscreen("image\\Sinimage\\MessageBox\\Button_Yes_Yellow.bmp");
	lpButtonYesGlay07	= LoadDibSurfaceOffscreen("image\\Sinimage\\MessageBox\\Button_Yes_Glay.bmp");
	lpButtonNo07		= LoadDibSurfaceOffscreen("image\\Sinimage\\MessageBox\\Button_No_Yellow.bmp");
	lpButtonNoGlay07	= LoadDibSurfaceOffscreen("image\\Sinimage\\MessageBox\\Button_No_Glay.bmp");	
	lpButtonBox07       = LoadDibSurfaceOffscreen("image\\Sinimage\\Help\\box-test-title2.bmp");

	lpArrowUpDown[0]  = LoadDibSurfaceOffscreen("Image\\SinImage\\MessageBox\\Arrow_Up.bmp"); // 0 Up  1 Down
	lpArrowUpDown[1]  = LoadDibSurfaceOffscreen("Image\\SinImage\\MessageBox\\Arrow_Down.bmp"); // 0 Up  1 Down

	lpButtonNo[0]     = LoadDibSurfaceOffscreen("Image\\SinImage\\MessageBox\\Button_No_Glay.bmp"); 
	lpButtonNo[1]     = LoadDibSurfaceOffscreen("Image\\SinImage\\MessageBox\\Button_No_Yellow.bmp"); 

	lpButtonYes[0]    = LoadDibSurfaceOffscreen("Image\\SinImage\\MessageBox\\Button_Yes_Glay.bmp"); 
	lpButtonYes[1]    = LoadDibSurfaceOffscreen("Image\\SinImage\\MessageBox\\Button_Yes_Yellow.bmp"); 

	lpButtonOk[0]     = LoadDibSurfaceOffscreen("Image\\SinImage\\MessageBox\\Button_Ok_Glay.bmp"); 
	lpButtonOk[1]     = LoadDibSurfaceOffscreen("Image\\SinImage\\MessageBox\\Button_Ok_Yellow.bmp"); 
	
	lpButtonCancel[0] = LoadDibSurfaceOffscreen("Image\\SinImage\\MessageBox\\Button_Cancel_Glay.bmp"); 
	lpButtonCancel[1] = LoadDibSurfaceOffscreen("Image\\SinImage\\MessageBox\\Button_Cancel_Yellow.bmp"); 

	lpCount = LoadDibSurfaceOffscreen("Image\\SinImage\\MessageBox\\COUNT.bmp"); 

	//별아이템
	lpStarIcon = LoadDibSurfaceOffscreen("image\\Sinimage\\MessageBox\\Star.bmp");
	lpStarShopClose = LoadDibSurfaceOffscreen("image\\Sinimage\\MessageBox\\c.bmp");
	lpStarShopClose_ = LoadDibSurfaceOffscreen("image\\Sinimage\\MessageBox\\c1.bmp");

}
/*----------------------------------------------------------------------------*
*						     Release
*-----------------------------------------------------------------------------*/	
void cMESSAGEBOX::Release()
{
	if(lpArrowUpDown[0]){
		lpArrowUpDown[0]->Release();
		lpArrowUpDown[0] = 0;

	}
	if(lpArrowUpDown[1]){
		lpArrowUpDown[1]->Release();
		lpArrowUpDown[1] = 0;

	}
	if(lpButtonNo[0]){
		lpButtonNo[0]->Release();
		lpButtonNo[0] = 0;

	}
	if(lpButtonNo[1]){
		lpButtonNo[1]->Release();
		lpButtonNo[1] = 0;

	}
	if(lpButtonYes[0]){
		lpButtonYes[0]->Release();
		lpButtonYes[0] = 0;

	}
	if(lpButtonYes[1]){
		lpButtonYes[1]->Release();
		lpButtonYes[1] = 0;
	}

	if(lpButtonOk[0]){
		lpButtonOk[0]->Release();
		lpButtonOk[0] = 0;
	}
	if(lpButtonOk[1]){
		lpButtonOk[1]->Release();
		lpButtonOk[1] = 0;
	}
	if(lpButtonCancel[0]){
		lpButtonCancel[0]->Release();
		lpButtonCancel[0] = 0;
	}
	if(lpButtonCancel[1]){
		lpButtonCancel[1]->Release();
		lpButtonCancel[1] = 0;
	}

}
/*----------------------------------------------------------------------------*
*							     그리기  
*-----------------------------------------------------------------------------*/	
void cMESSAGEBOX::Draw()
{
	int i =0;   
	int x = 0 , y = 0;

	//사라진 아이템
	if(sLost_Item.Flag){      
		Interface_clanmenu.DrawBox(BackStartPos.x,BackStartPos.y,4,5);	    

		if(sLost_Item.LostItem[0].CODE){ //사라질 아이템의 이미지를 보여준다
			x = (60 - sLost_Item.LostItem[0].w )/2;
			y = (80 - sLost_Item.LostItem[0].h )/2;

			DrawSprite(BackStartPos.x+20+x,BackStartPos.y+80+y,sLost_Item.LostItem[0].lpItem,0,0,sLost_Item.LostItem[0].w,sLost_Item.LostItem[0].h);
		}
		if(sLost_Item.LostItem[1].CODE){ //사라질 아이템의 이미지를 보여준다
			x = (60 - sLost_Item.LostItem[0].w )/2;
			y = (80 - sLost_Item.LostItem[0].h )/2;

			DrawSprite(BackStartPos.x+20+x+50,BackStartPos.y+80+y+22,sLost_Item.LostItem[1].lpItem,0,0,sLost_Item.LostItem[1].w,sLost_Item.LostItem[1].h);
		}

		DrawSprite(BackStartPos.x+BUTTON2_BOX_X+35,BackStartPos.y+BUTTON2_BOX_Y, lpButtonBox07,  0, 0, 48, 23, 1); 
		if(sLost_Item.BuyOk){
			DrawSprite(BackStartPos.x+BUTTON2_X+35,BackStartPos.y+BUTTON2_BOX_Y+4, lpButtonOk[1],  0, 0, 32, 16, 1);
		}
		else DrawSprite(BackStartPos.x+BUTTON2_X+35,BackStartPos.y+BUTTON2_BOX_Y+4, lpButtonOk[0],  0, 0, 32, 16, 1);

	}
	////////////////////별 아이템 관련
	if(sStarShop.Flag){        
		Interface_clanmenu.DrawBox(BackStartPos.x,BackStartPos.y,4,5);	    

		DrawSprite(BackStartPos.x+60,BackStartPos.y+60, lpStarIcon,  0, 0, 32, 32, 1);
		DrawSprite(BackStartPos.x+BUTTON2_BOX_X+35,BackStartPos.y+BUTTON2_BOX_Y, lpButtonBox07,  0, 0, 48, 23, 1);
		if(sStarShop.ButtonClose)
			DrawSprite(BackStartPos.x+BUTTON2_X+35,BackStartPos.y+BUTTON2_Y, lpStarShopClose_,  0, 0, 32, 16, 1);
		else DrawSprite(BackStartPos.x+BUTTON2_X+35,BackStartPos.y+BUTTON2_Y, lpStarShopClose,  0, 0, 32, 16, 1);

		DrawSprite(BackStartPos.x+BUTTON2_BOX_X+35,BackStartPos.y+160, lpButtonBox07,  0, 0, 48, 23, 1); 
		if(sStarShop.BuyOk){
			DrawSprite(BackStartPos.x+BUTTON2_X+35,BackStartPos.y+163, lpButtonYes07,  0, 0, 32, 16, 1);
		}
		else DrawSprite(BackStartPos.x+BUTTON2_X+35,BackStartPos.y+163, lpButtonYesGlay07,  0, 0, 32, 16, 1);
	}
	////////////////////

	//아이템 받기 메세지 박스
	if(sMessageBox_RecvItem.Flag){   
		Interface_clanmenu.DrawBox(BackStartPos.x,BackStartPos.y,4,5);	
		DrawSprite(BackStartPos.x+BUTTON2_BOX_X,BackStartPos.y+BUTTON2_BOX_Y, lpButtonBox07,  0, 0, 48, 23, 1);
		DrawSprite(BackStartPos.x+BUTTON3_BOX_X,BackStartPos.y+BUTTON3_BOX_Y, lpButtonBox07,  0, 0, 48, 23, 1);
	
		if(sMessageBox_RecvItem.ButtonYes)
			DrawSprite(BackStartPos.x+BUTTON2_X,BackStartPos.y+BUTTON2_Y, lpButtonYes07,  0, 0, 32, 16, 1);
		else DrawSprite(BackStartPos.x+BUTTON2_X,BackStartPos.y+BUTTON2_Y, lpButtonYesGlay07,  0, 0, 32, 16, 1);
		if(sMessageBox_RecvItem.ButtonNo)
			DrawSprite(BackStartPos.x+BUTTON3_X,BackStartPos.y+BUTTON3_Y, lpButtonNo07,  0, 0, 32, 16, 1);
		else DrawSprite(BackStartPos.x+BUTTON3_X,BackStartPos.y+BUTTON3_Y, lpButtonNoGlay07,  0, 0, 32, 16, 1);

		if(sMessageBox_RecvItem.RecvItem.CODE){ //받을 아이템의 이미지를 그려준다 
			x = (60 - sMessageBox_RecvItem.RecvItem.w )/2;
			y = (80 - sMessageBox_RecvItem.RecvItem.h )/2;

			DrawSprite(BackStartPos.x+20+x,BackStartPos.y+80+y,sMessageBox_RecvItem.RecvItem.lpItem,0,0,sMessageBox_RecvItem.RecvItem.w,sMessageBox_RecvItem.RecvItem.h);
		}
	}
	for(i = 0 ; i < MESSAGE3_MAX ; i++){
		if(sMessageBox3[i].Flag){
			dsDrawTexImage( MatBoxMain , sMessageBox3[i].x , sMessageBox3[i].y , 256, 128 , 255 );  //상단
			DrawSprite(sMessageBox3[i].x+40  , sMessageBox3[i].y+95 , lpButtonOk[0] ,0 ,0 , 32 , 16);	
			DrawSprite(sMessageBox3[i].x+94 , sMessageBox3[i].y+95 , lpButtonCancel[0] ,0 ,0 , 32 , 16);
		
			if(MessageButtonPosi == 1)//확인 
				DrawSprite(sMessageBox3[i].x+40  , sMessageBox3[i].y+95 , lpButtonOk[1] ,0 ,0 , 32 , 16);
			if(MessageButtonPosi == 2)//취소 
				DrawSprite(sMessageBox3[i].x+94 , sMessageBox3[i].y+95 , lpButtonCancel[1] ,0 ,0 , 32 , 16);
		}
	}

	for(i = 0 ; i < 20 ; i++){  
		if(sMessageBox2[i].Flag){
			dsDrawTexImage( MatBoxMain , sMessageBox2[i].x , sMessageBox2[i].y , 256, 128 , 255 );  //상단
			DrawSprite(sMessageBox2[i].x+34 , sMessageBox2[i].y+62 , lpCount ,0 ,0 , 109 , 25);
			if( i != MESSAGE_BUY_STAR){ //예 아니오로 하기위해 살짝 땜빵
				DrawSprite(sMessageBox2[i].x+40  , sMessageBox2[i].y+95 , lpButtonOk[0] ,0 ,0 , 32 , 16);	
				DrawSprite(sMessageBox2[i].x+94 , sMessageBox2[i].y+95 , lpButtonCancel[0] ,0 ,0 , 32 , 16);
				if(MessageButtonPosi == 1)//확인 
					DrawSprite(sMessageBox2[i].x+40  , sMessageBox2[i].y+95 , lpButtonOk[1] ,0 ,0 , 32 , 16);
				if(MessageButtonPosi == 2)//취소 
					DrawSprite(sMessageBox2[i].x+94 , sMessageBox2[i].y+95 , lpButtonCancel[1] ,0 ,0 , 32 , 16);
			}	
			if(MessageArrowPosi == 1) //Up
				DrawSprite(sMessageBox2[i].x+124 , sMessageBox2[i].y+62 , lpArrowUpDown[0] ,0 ,0 , 19 , 13);
			if(MessageArrowPosi == 2) //Down
				DrawSprite(sMessageBox2[i].x+124 , sMessageBox2[i].y+75 , lpArrowUpDown[1] ,0 ,0 , 19 , 13);
			
			if( i== MESSAGE_BUY_STAR){
				DrawSprite(sMessageBox2[i].x+40  , sMessageBox2[i].y+95 , lpButtonYes[0] ,0 ,0 , 32 , 16);	
				DrawSprite(sMessageBox2[i].x+94 , sMessageBox2[i].y+95 , lpButtonNo[0] ,0 ,0 , 32 , 16);
				if(MessageButtonPosi == 1)//확인 
					DrawSprite(sMessageBox2[i].x+40  , sMessageBox2[i].y+95 , lpButtonYes[1] ,0 ,0 , 32 , 16);
				if(MessageButtonPosi == 2)//취소 
					DrawSprite(sMessageBox2[i].x+94 , sMessageBox2[i].y+95 , lpButtonNo[1] ,0 ,0 , 32 , 16);
			}
		}
	}
	
	for(i = 0 ; i < 100 ; i++){
		if(sMessageBox[i].Flag){
			dsDrawTexImage( MatBoxLeft , sMessageBox[i].x , sMessageBox[i].y, 32, 64 , 255 );  //좌측 
			int j; for( j =0 ; j< sMessageBox[i].Line ; j++){
				dsDrawTexImage( MatBoxCenter , sMessageBox[i].x+32+(j*16) , sMessageBox[i].y, 16, 64, 255 );//가운데
			}
			dsDrawTexImage( MatBoxRight , sMessageBox[i].x+32+(j*16) , sMessageBox[i].y , 32, 64 , 255 );  //상단
	
		}
	}
}
/*----------------------------------------------------------------------------*
*							     메인  
*-----------------------------------------------------------------------------*/	
void cMESSAGEBOX::Main()
{

    MessageArrowPosi = 0;      //박스의 화살표 포지션 
    MessageButtonPosi = 0;     //버튼 포지션 

	int i=0;
	for( i = 0 ; i < 100 ; i++){ 
		if(sMessageBox[i].Flag){
			MessageTimeCount++;
			if(	i == MESSAGE_MOVE_TRADE_ITEM){	//사기방지 메세지는 가운데 보여준다 
				if(MessageTimeCount > 70*5){ //3초후 메세지를 지워준다 
					sMessageBox[i].Flag = 0;
					MessageTimeCount = 0;    //초기화 
				}
			}
			else{
				 /////// 이벤트 점수 스코어만 시간을 길게해준다 (스페셜 날치기 땜빵)
				if( i == MESSAGE_SCORE ) {
					if(MessageTimeCount > 70*6){ //3초후 메세지를 지워준다 
						sMessageBox[i].Flag = 0;
						MessageTimeCount = 0;    //초기화 
					}

				}
				else{
					if(MessageTimeCount > 70*3){ //3초후 메세지를 지워준다 
						sMessageBox[i].Flag = 0;
						MessageTimeCount = 0;    //초기화 
					}
				}
			}
		}
	}

	if(sLost_Item.Flag){
		BackStartPos.x = 101;
		BackStartPos.y = 114; 
		sLost_Item.BuyOk = 0;
			if(pCursorPos.x >BackStartPos.x+BUTTON2_X+35 && pCursorPos.x < BackStartPos.x+BUTTON2_X+35+32 && 
				pCursorPos.y >BackStartPos.y+BUTTON2_BOX_Y && pCursorPos.y < BackStartPos.y+BUTTON2_BOX_Y+16)
				sLost_Item.BuyOk = 1;

	}

	//별아이템
	if(sStarShop.Flag){    
		BackStartPos.x = 101;
		BackStartPos.y = 114;
		sStarShop.ButtonClose = 0;
		if(!sinMessageBoxShowFlag){ //메세지 박스가 중복으로 뜨는걸 막는다
			if(pCursorPos.x >BackStartPos.x+BUTTON2_X+35 && pCursorPos.x < BackStartPos.x+BUTTON2_X+35+32 && 
				pCursorPos.y >BackStartPos.y+BUTTON2_Y && pCursorPos.y < BackStartPos.y+BUTTON2_Y+16)
				sStarShop.ButtonClose = 1;

			sStarShop.BuyOk = 0;
			if(pCursorPos.x >BackStartPos.x+BUTTON2_X+35 && pCursorPos.x < BackStartPos.x+BUTTON2_X+35+32 && 
				pCursorPos.y >BackStartPos.y+160 && pCursorPos.y < BackStartPos.y+160+16)
				sStarShop.BuyOk = 1;
		}
	}

	//아이템 택배 서비스 
	if(sMessageBox_RecvItem.Flag){
		BackStartPos.x = 101;
		BackStartPos.y = 114;
		sMessageBox_RecvItem.ButtonYes = 0;
		sMessageBox_RecvItem.ButtonNo = 0;
		if(pCursorPos.x >BackStartPos.x+BUTTON2_X && pCursorPos.x < BackStartPos.x+BUTTON2_X+32 && 
			pCursorPos.y >BackStartPos.y+BUTTON2_Y && pCursorPos.y < BackStartPos.y+BUTTON2_Y+16)
			sMessageBox_RecvItem.ButtonYes = 1;
		
		if(pCursorPos.x >BackStartPos.x+BUTTON3_X && pCursorPos.x < BackStartPos.x+BUTTON3_X+32 && 
			pCursorPos.y >BackStartPos.y+BUTTON3_Y && pCursorPos.y < BackStartPos.y+BUTTON3_Y+16)
			sMessageBox_RecvItem.ButtonNo = 1;
	}

	for( i = 0 ; i < 20 ; i++){
		if(sMessageBox2[i].Flag){
			if(pCursorPos.x >sMessageBox2[i].x+124 && pCursorPos.x < sMessageBox2[i].x+124+19 && 
				pCursorPos.y >sMessageBox2[i].y+62 && pCursorPos.y < sMessageBox2[i].y+62+13)
				MessageArrowPosi = 1; //화살표 업 

			if(pCursorPos.x >sMessageBox2[i].x+124 && pCursorPos.x < sMessageBox2[i].x+124+19 && 
				pCursorPos.y >sMessageBox2[i].y+75 && pCursorPos.y < sMessageBox2[i].y+75+13)
				MessageArrowPosi = 2; //화살표 다운 

			if(pCursorPos.x >sMessageBox2[i].x+40 && pCursorPos.x < sMessageBox2[i].x+40+32 && 
				pCursorPos.y >sMessageBox2[i].y+95 && pCursorPos.y < sMessageBox2[i].y+98+16)
				MessageButtonPosi = 1; //OK

			if(pCursorPos.x >sMessageBox2[i].x+94 && pCursorPos.x < sMessageBox2[i].x+94+32 && 
				pCursorPos.y >sMessageBox2[i].y+95 && pCursorPos.y < sMessageBox2[i].y+95+16)
				MessageButtonPosi = 2; //Cancel
		}

	}
	for( i = 0 ; i <MESSAGE3_MAX ; i++){
		if(sMessageBox3[i].Flag){
			if(pCursorPos.x >sMessageBox3[i].x+40 && pCursorPos.x < sMessageBox3[i].x+40+32 && 
				pCursorPos.y >sMessageBox3[i].y+95 && pCursorPos.y < sMessageBox3[i].y+98+16)
				MessageButtonPosi = 1; //OK

			if(pCursorPos.x >sMessageBox3[i].x+94 && pCursorPos.x < sMessageBox3[i].x+94+32 && 
				pCursorPos.y >sMessageBox3[i].y+95 && pCursorPos.y < sMessageBox3[i].y+95+16)
				MessageButtonPosi = 2; //Cancel
		}

	}
	//////////////////// 연결 메세지 박스
	//기부금을 다시한번 확인해본다
	if(sinGiveMoneyCheckFlag){
		sinGiveMoneyCheckFlag = 0;
		cMessageBox.ShowMessage3(MESSAGE_GIVE_MONEY_CHECK,szGiveMoneyBuff); 

	}

}
/*----------------------------------------------------------------------------*
*							     종료 
*-----------------------------------------------------------------------------*/	
void cMESSAGEBOX::Close()
{
	Release();
}
/*----------------------------------------------------------------------------*
*							   LButtonDown
*-----------------------------------------------------------------------------*/	
void cMESSAGEBOX::LButtonDown(int x , int y)
{

	int i = 0;
	
	if(sLost_Item.Flag){  
		if(sLost_Item.BuyOk){
			memset(&sLost_Item,0,sizeof(sLOST_ITEM));
			isDrawClanMenu = 0;
			OverDay_Item_Flag = 0;
		}
	}

	//별아이템
	if(sStarShop.Flag){    
		if(sStarShop.ButtonClose){
			sStarShop.Flag = 0;
			isDrawClanMenu = 0;
		}
		if(sStarShop.BuyOk){
			sStarShop.BuyOk = 0;
			//sStarShop.Flag = 0;
			//isDrawClanMenu = 0;
			cMessageBox.ShowMessage2(MESSAGE_BUY_STAR);

		}

	}

	//아이템 택배서비스
	if(sMessageBox_RecvItem.Flag){  
		if(sMessageBox_RecvItem.ButtonYes){ //아이템 인증 서버로 코드보냄 
			if(cInvenTory.CheckInvenEmpty(&sMessageBox_RecvItem.RecvItem)){
				if(sMessageBox_RecvItem.Param[0]){
					cMessageBox.ShowMessage3(MESSAGE_SECRET_NUM,SecretNumName);
					isDrawClanMenu = 0;
				}
				else{
					if(sMessageBox_RecvItem.RecvItem.CODE == (sinGG1|sin01)){ //돈일 경우는 따로 처리해준다
						if(!cInvenTory.CheckMoneyLimit(sMessageBox_RecvItem.RecvItem.SellPrice)){
							cMessageBox.ShowMessage(MESSAGE_OVER_MONEY);
							isDrawClanMenu = 0;
							sMessageBox_RecvItem.Flag = 0;
						}
						else{
							SendItemExpress(sMessageBox_RecvItem.RecvItem.CODE);
							isDrawClanMenu = 0;
							sMessageBox_RecvItem.Flag = 0;
						}
					}
					else{
						SendItemExpress(sMessageBox_RecvItem.RecvItem.CODE);
						isDrawClanMenu = 0;
						sMessageBox_RecvItem.Flag = 0;
					}

				}
			}
			else{
				cMessageBox.ShowMessage(MESSAGE_OVER_SPACE);
				isDrawClanMenu = 0;
				sMessageBox_RecvItem.Flag = 0;
			}

		}
		if(sMessageBox_RecvItem.ButtonNo){
			sMessageBox_RecvItem.Flag = 0;
			isDrawClanMenu = 0;
		}
	}

	//////////스킬 습득 메세지 
	for( i = 0 ; i < MESSAGE3_MAX ; i++){    
		if(sMessageBox3[i].Flag){
			if(MessageButtonPosi == 1){ //확인 
				CheckOkMessageBox2(i);
				//LearnSkillEffect(0);
			}
			if(MessageButtonPosi == 2){ //취소
				if( i == MESSAGE_QUEST_100LVITEMOK){
					chaQuest.sHaQuest100LV.State--;
					chaQuest.showFuryQuestItem(1);
				}
				if( i == MESSAGE_CHANGE_JOB4_2){
					if(sinChar->JOB_CODE <=4 ){
						WarpField2(START_FIELD_NUM); //돌아간다
					}else
					{
						WarpField2(START_FIELD_MORYON); //돌아간다
					}
				}
				if(i == MESSAGE_SELL_HIGHRANK_ITEM){
					MouseItem.Flag = 1;
					ResetInvenItemCode();
					cInvenTory.AutoSetInvenItem(&MouseItem);
					
				}
				//능력치 초기화후 스킬초기화때문에 임시로 넣어준다 
				if(i == MESSAGE_EVENT_GYUNGFUM){
					if(sinChar->wVersion[1] == 1){
						sinSkillPontInitButtonFlag = 0;
					}
					else sinSkillPontInitButtonFlag = 1;
						
				}
				//워프게이트 
				if(i == MESSAGE_WARP){ 
					WingWarpGate_Field(-1);

				}
				if( i ==MESSAGE_SECRET_NUM){
					hFocusWnd = 0;  
					cInterFace.ChatFlag = 0;
					SetWindowText( hFocusWnd , "");
					sinChatEnter = 0; //임의로 글씨가 안보이게한다
				}
				//100LV퀘스트
				if( i == MESSAGE_QUEST_100LVITEMOK){
					chaQuest.iFuryQuestIndex[1]=0;
				}

				sinMessageBoxShowFlag  = 0;
				sMessageBox3[i].Flag   = 0;
				ShowSkillUpInfo = 0;
				sinInitPointFlag7 = 0;
			}
		}
	}

	for( i = 0 ; i < 20 ; i++){ 
		if(sMessageBox2[i].Flag){
			if(MessageArrowPosi == 1){
				if(i != MESSAGE_BUY_ITEM_DEFALT)
					sMessageBox2[i].Count ++;
				if(i == 3 || i == MESSAGE_MOVE_MONEY_TRADE || i == MESSAGE_MOVE_MONEY_MOVE || i == MESSAGE_GIVE_MONEY){//돈이면 
					if(sinChar->Money < sMessageBox2[i].Count){
						sMessageBox2[i].Count = sinChar->Money;
					}
				}
				if(i == MESSAGE_SOD2_GET_MONEY  ){
					if(cInvenTory.CheckMoneyLimit(sMessageBox2[i].Count)){ 
						if(sMessageBox2[i].Count > (int)cSinSod2.ClanMoney){
							sMessageBox2[i].Count = (int)cSinSod2.ClanMoney;
						}
					}
					else
						sMessageBox2[i].Count /= 10;
				}
				if(i == MESSAGE_SIEGE_SET_MERCENRAY){

//#ifdef _TEST_SERVER	
					//돈체크
		//			if((sinChar->Money - ((haMercenrayMoney[haMercenrayIndex-1])/10)*sMessageBox2[i].Count) < 0){

//#else
					//돈체크
					if((sinChar->Money - (haMercenrayMoney[haMercenrayIndex-1])*sMessageBox2[i].Count) < 0){
//#endif	
						sMessageBox2[i].Count-=1;
						cMessageBox.ShowMessage(MESSAGE_NOT_ENOUGH_MONEY); //돈부족
						break;
					}
					if(cSinSiege.MercenaryNum[haMercenrayIndex-1]+sMessageBox2[i].Count > 20){
						sMessageBox2[i].Count-=1;
						ShowMessage(MESSAGE_MERCENRAY_OVER);
						break;
					}
					if(sinChar->Money <= sMessageBox2[i].Count*(int)1){
						sMessageBox2[i].Count-=1;
						ShowMessage(MESSAGE_NOT_ENOUGH_MONEY);
						//CloseMessage(); //메세지박스를닫아준다 
					}
					
				}
				if(i == MESSAGE_MYSHOP_POTION|| i == MESSAGE_SIEGE_GET_MONEY){ 
					//돈
					if(sinChar->Money <= sMessageBox2[i].Count*(int)MyShopPotion.ItemHeader.dwVersion){
						sMessageBox2[i].Count = sinChar->Money/(int)MyShopPotion.ItemHeader.dwVersion;
					} 
					//무게 
					if(sinChar->Weight[0]+sMessageBox2[i].Count > sinChar->Weight[1]){
						sMessageBox2[i].Count = sinChar->Weight[1]-sinChar->Weight[0];

					}
					//갯수 
					if((int)MyShopPotion.PotionCount < sMessageBox2[i].Count){
						sMessageBox2[i].Count = (int)MyShopPotion.PotionCount;

					}
					/*
					//돈 한도 초과 
					if(!cInvenTory.CheckMoneyLimit(sMessageBox2[i].Count*(int)MyShopPotion.ItemHeader.dwVersion)){
						while(1){
							sMessageBox2[i].Count--;
							if(cInvenTory.CheckMoneyLimit(sMessageBox2[i].Count*(int)MyShopPotion.ItemHeader.dwVersion))
								break;
							if(sMessageBox2[i].Count <= 0){
								sMessageBox2[i].Count = 0;
								break;
							}
						}
					}
					*/
				}

				if(i == MESSAGE_BUY_STAR){
					/*별을 판매하지 않는다.*/
					//if(sinChar->Money <= sMessageBox2[i].Count*100000){
					//	sMessageBox2[i].Count = sinChar->Money/100000;
					//}
					sMessageBox2[i].Count = 0;
				}

				if(i == MESSAGE_MOVE_MONEY_MOVE) { //창고에 돈을 옮길때 
					if(sinChar->Money < sMessageBox2[i].Count){
						sMessageBox2[i].Count = sinChar->Money;
					}

					if( (sWareHouse.Money-2023) + sMessageBox2[i].Count > 100000000 ){
						sMessageBox2[i].Count -= ((sWareHouse.Money-2023) + sMessageBox2[i].Count) - 100000000;
					}

				}

				if( i == MESSAGE_MOVE_MONEY_RECV){
					if(sWareHouse.Money-2023 < sMessageBox2[i].Count)
						sMessageBox2[i].Count = sWareHouse.Money-2023;
					if(sinChar->Level <= 10){
						if(sinChar->Money+sMessageBox2[i].Count > 200000){
							sMessageBox2[i].Count = 200000 - sinChar->Money;
							if(sMessageBox2[i].Count < 0 ) sMessageBox2[i].Count = 0;

						}
					}
					else{
						switch(sinChar->ChangeJob){
							case 0:
								if((sinChar->Money + sMessageBox2[i].Count) > (sinChar->Level*200000)-1800000){
									sMessageBox2[i].Count = (sinChar->Level*200000)-1800000 - sinChar->Money;
									if(sMessageBox2[i].Count < 0 ) sMessageBox2[i].Count = 0;

								}
							break;

							case 1:
								if( ( sinChar->Money + sMessageBox2[i].Count) > 10000000 ){
									sMessageBox2[i].Count = 10000000 - sinChar->Money;
									if(sMessageBox2[i].Count < 0 ) sMessageBox2[i].Count = 0;
								}
							break;
							case 2:
								if( ( sinChar->Money + sMessageBox2[i].Count) > 50000000 ){
									sMessageBox2[i].Count = 50000000 - sinChar->Money;
									if(sMessageBox2[i].Count < 0 ) sMessageBox2[i].Count = 0;
								}
							break;
							//4차전업(임시)
							case 3:
							case 4:		// tier 5: the fifth rank
								if( ( sinChar->Money + sMessageBox2[i].Count) > 100000000 ){
									sMessageBox2[i].Count = 100000000 - sinChar->Money;
									if(sMessageBox2[i].Count < 0 ) sMessageBox2[i].Count = 0;
								}


							break;


						}
					}

				}
				
				if(i == MESSAGE_EVENT_SMASHTV){//기부금 
					if(sinChar->Money < sMessageBox2[i].Count){
						sMessageBox2[i].Count = sinChar->Money;
						if(sMessageBox2[i].Count < (sinChar->Level * SMASHTV_MONEY)){
							ShowMessage(MESSAGE_NOT_ENOUGH_MONEY);
							CloseMessage(); //메세지박스를닫아준다 


						}
					}
					if( sMessageBox2[i].Count > (sinChar->Level * SMASHTV_MONEY)){ 
						sMessageBox2[i].Count = (sinChar->Level * SMASHTV_MONEY);
					}
				}

				if(i == 2 || i == 1){ //아이템 버리기나 팔기이면 
					if(pMessageItem->sItemInfo.PotionCount < sMessageBox2[i].Count){
							sMessageBox2[i].Count = pMessageItem->sItemInfo.PotionCount;
					}
				}
				if(i==0){//아이템 사기이면 
					CountTemp = sMessageBox2[i].Count*pMessageItem->sItemInfo.Price;
					if(CountTemp > sinChar->Money){
						ShowMessage(MESSAGE_NOT_ENOUGH_MONEY); //메세지를 보여준다 
						for(int k=0; k < CountTemp ; k++){
							if((CountTemp-k) <= sinChar->Money){
								sMessageBox2[i].Count =(CountTemp-k)/pMessageItem->sItemInfo.Price;
								break;
							}

						}

					}
					CountTemp = (sMessageBox2[i].Count*pMessageItem->sItemInfo.Weight)+sinChar->Weight[0];
					if(sinChar->Weight[1] < CountTemp){ //무게를 넘어서면
						ShowMessage(MESSAGE_OVER_WEIGHT); //메세지를 보여준다 
						for(int p=0; p < CountTemp ; p++){
							if((CountTemp-p) <= sinChar->Weight[1]){
								sMessageBox2[i].Count -=p;
								break;
							}
						}
					}
				}
			}
			if(MessageArrowPosi == 2){
				if(i != MESSAGE_BUY_ITEM_DEFALT){
					sMessageBox2[i].Count --;
					if(sMessageBox2[i].Count < 0){
						sMessageBox2[i].Count = 0;
					}
				}
			}
			if(MessageButtonPosi == 1){ //확인 
				CheckOkMessageBox(i);
			}
			if(MessageButtonPosi == 2){ //취소 
				sinMessageBoxShowFlag  = 0;
				sMessageBox2[i].Flag   = 0;
				cSinSod2.ClanMasterMessageBoxFlag = 0;
				if( i == MESSAGE_MOVE_MONEY_TRADE)
					sTrade.Money = 0;
				//물약 겹침 방지.
				//if(i == MESSAGE_THROW_ITEM)
				//	sinThrowItemToFeild(&MouseItem);//아이템을 버린다
			}
		}
	}
}
/*----------------------------------------------------------------------------*
*							   LButtonUp
*-----------------------------------------------------------------------------*/	
void cMESSAGEBOX::LButtonUp(int x , int y)
{

	//스킬 포이트 초기화 박스를 띄운다 ( 이벤트가 중복으로 처리되서어 살짝 땜빵)
	if(sinSkillPontInitButtonFlag){ 
		sinSkillPontInitButtonFlag = 0;
		cMessageBox.ShowMessage3(MESSAGE_INIT_SKILL_POINT,sinSkillPointName); 
	}

}
/*----------------------------------------------------------------------------*
*							   RButtonDown
*-----------------------------------------------------------------------------*/	
void cMESSAGEBOX::RButtonDown(int x , int y)
{

}
/*----------------------------------------------------------------------------*
*							   RButtonUp
*-----------------------------------------------------------------------------*/	
void cMESSAGEBOX::RButtonUp(int x , int y)
{

}
/*----------------------------------------------------------------------------*
*							    KeyDown
*-----------------------------------------------------------------------------*/	
void cMESSAGEBOX::KeyDown()
{
	int i = 0;

	char szKeyBuffPad[10]={VK_NUMPAD0,VK_NUMPAD1,VK_NUMPAD2,VK_NUMPAD3,VK_NUMPAD4,
		VK_NUMPAD5,VK_NUMPAD6,VK_NUMPAD7,VK_NUMPAD8,VK_NUMPAD9};
	char szKeyBuff[10]={'0','1','2','3','4','5','6','7','8','9'}; //키버퍼 

	for(i = 0 ; i < 20 ; i++){  
   		if(sMessageBox2[i].Flag){ 
			for(int j=0; j < 10 ; j++){
				if(sinGetKeyClick(szKeyBuff[j])||sinGetKeyClick(szKeyBuffPad[j])){
					if(i == 3 || i ==MESSAGE_MOVE_MONEY_TRADE || i == MESSAGE_MOVE_MONEY_MOVE || i ==MESSAGE_GIVE_MONEY){  //돈일경우에 
						sMessageBox2[i].szCount[sMessageBox2[i].Line]=szKeyBuff[j];
						sMessageBox2[i].Count = atoi(sMessageBox2[i].szCount);
						if(sinChar->Money < sMessageBox2[i].Count){
							sMessageBox2[i].Count = sinChar->Money;
						}
					}
					if(i == MESSAGE_SIEGE_GET_MONEY){//<ha>공성전 돈찾기
						sMessageBox2[i].szCount[sMessageBox2[i].Line]=szKeyBuff[j];
						sMessageBox2[i].Count = atoi(sMessageBox2[i].szCount); 
						//찾을 돈이 0이면 찾을수 없다.
						if((int)cSinSiege.TotalTax <= 0){
							cMessageBox.ShowMessage(MESSAGE_NOT_CASTLE_TOTALMONEY);
							break;
						}
						if(cInvenTory.CheckMoneyLimit(sMessageBox2[i].Count)){ 
							if(sMessageBox2[i].Count > (int)cSinSiege.TotalTax){
								sMessageBox2[i].Count = (int)cSinSiege.TotalTax;
							}
						}
						else
							//sMessageBox2[i].Count = (int)cSinSiege.TotalTax;
						    sMessageBox2[i].Count /= 10;
				
					}
					/*
					if(i == MESSAGE_SIEGE_SET_MERCENRAY){
						sMessageBox2[i].szCount[sMessageBox2[i].Line]=szKeyBuff[j];
						sMessageBox2[i].Count = atoi(sMessageBox2[i].szCount);
						if(cSinSiege.MercenaryNum[haMercenrayIndex-1]+sMessageBox2[i].Count > 20){
							sMessageBox2[i].Count = 0;
						}

					}
					*/
					
		
 					if (i == MESSAGE_SOD2_GET_MONEY ){
						sMessageBox2[i].szCount[sMessageBox2[i].Line]=szKeyBuff[j];
						sMessageBox2[i].Count = atoi(sMessageBox2[i].szCount); 
						if(cInvenTory.CheckMoneyLimit(sMessageBox2[i].Count)){ 
							if(sMessageBox2[i].Count > (int)cSinSod2.ClanMoney){
								sMessageBox2[i].Count = (int)cSinSod2.ClanMoney;
							}
						}
						else
							sMessageBox2[i].Count /= 10;
					}

					if(i == MESSAGE_MYSHOP_POTION){ 
						sMessageBox2[i].szCount[sMessageBox2[i].Line]=szKeyBuff[j];
						sMessageBox2[i].Count = atoi(sMessageBox2[i].szCount);

						//가격
						if(sinChar->Money <= sMessageBox2[i].Count*(int)MyShopPotion.ItemHeader.dwVersion){
							sMessageBox2[i].Count = sinChar->Money/(int)MyShopPotion.ItemHeader.dwVersion;
						}
						//무게 
						if(sinChar->Weight[0]+sMessageBox2[i].Count > sinChar->Weight[1]){
							sMessageBox2[i].Count = sinChar->Weight[1]-sinChar->Weight[0];

						}
						//갯수 
						if((int)MyShopPotion.PotionCount < sMessageBox2[i].Count){
							sMessageBox2[i].Count = (int)MyShopPotion.PotionCount;

						}
						//돈 한도 초과 
						/*
						if(!cInvenTory.CheckMoneyLimit(sMessageBox2[i].Count*(int)MyShopPotion.ItemHeader.dwVersion)){
							while(1){
								sMessageBox2[i].Count--;
								if(cInvenTory.CheckMoneyLimit(sMessageBox2[i].Count*(int)MyShopPotion.ItemHeader.dwVersion))
									break;
								if(sMessageBox2[i].Count <= 0){
									sMessageBox2[i].Count = 0;
									break;
								}
							}
						}
						*/
					}

					if(i == MESSAGE_BUY_STAR){
						/*별을 판매하지 않는다.*/
						//if(sinChar->Money <= sMessageBox2[i].Count*100000){
						//	sMessageBox2[i].Count = sinChar->Money/100000;
						//}
						sMessageBox2[i].Count = 0;
					}

					if(i == MESSAGE_MOVE_MONEY_MOVE) { //창고에 돈을 옮길때 
						if(sinChar->Money < sMessageBox2[i].Count){
							sMessageBox2[i].Count = sinChar->Money;
						}

						if( (sWareHouse.Money-2023) + sMessageBox2[i].Count > 100000000 ){
							sMessageBox2[i].Count -= ((sWareHouse.Money-2023) + sMessageBox2[i].Count) - 100000000;
						}

					}
					//상점 등록
					if( i == MESSAGE_MYSHOP_ITEM){    
						sMessageBox2[i].szCount[sMessageBox2[i].Line]=szKeyBuff[j];
						sMessageBox2[i].Count = atoi(sMessageBox2[i].szCount);

						if(MyShopMouseItem.Class == ITEM_CLASS_POTION){
							if(!cInvenTory.CheckMoneyLimit(cMyShop.GetTotalMyShopItemMoney()+(sMessageBox2[i].Count*MyShopMouseItem.sItemInfo.PotionCount))){
								while(1){
									sMessageBox2[i].Count /= 10;
									if(cInvenTory.CheckMoneyLimit(cMyShop.GetTotalMyShopItemMoney()+(sMessageBox2[i].Count*MyShopMouseItem.sItemInfo.PotionCount))){
										ShowMessage(MESSAGE_OVER_MONEY);
										break;
									}
									if(sMessageBox2[i].Count <= 0 ){
										sMessageBox2[i].Count = 0;
										ShowMessage(MESSAGE_OVER_MONEY);
										break;
									}
								}
							}
						}
						else{
							if(!cInvenTory.CheckMoneyLimit(cMyShop.GetTotalMyShopItemMoney()+sMessageBox2[i].Count)){
								while(1){
									sMessageBox2[i].Count /= 10;
									if(cInvenTory.CheckMoneyLimit(cMyShop.GetTotalMyShopItemMoney()+sMessageBox2[i].Count)){
										ShowMessage(MESSAGE_OVER_MONEY);
										break;
									}
									if(sMessageBox2[i].Count <= 0 ){
										sMessageBox2[i].Count = 0;
										ShowMessage(MESSAGE_OVER_MONEY);
										break;
									}
								}
							}

						}
					}


					if( i == MESSAGE_MOVE_MONEY_RECV){
						sMessageBox2[i].szCount[sMessageBox2[i].Line]=szKeyBuff[j];
						sMessageBox2[i].Count = atoi(sMessageBox2[i].szCount);
						if(sWareHouse.Money-2023 < sMessageBox2[i].Count)
							sMessageBox2[i].Count = sWareHouse.Money-2023;
						if(sinChar->Level <= 10){
							if(sinChar->Money+sMessageBox2[i].Count > 200000){
								sMessageBox2[i].Count = 200000 - sinChar->Money;
								if(sMessageBox2[i].Count < 0 ) sMessageBox2[i].Count = 0;

							}
						}
						else{
							switch(sinChar->ChangeJob){
								case 0:
									if((sinChar->Money + sMessageBox2[i].Count) > (sinChar->Level*200000)-1800000){
										sMessageBox2[i].Count = (sinChar->Level*200000)-1800000 - sinChar->Money;
										if(sMessageBox2[i].Count < 0 ) sMessageBox2[i].Count = 0;

									}
								break;
								case 1:
									if( ( sinChar->Money + sMessageBox2[i].Count) > 10000000 ){
										sMessageBox2[i].Count = 10000000 - sinChar->Money;
										if(sMessageBox2[i].Count < 0 ) sMessageBox2[i].Count = 0;
									}
								break;
								case 2:
									if( ( sinChar->Money + sMessageBox2[i].Count) > 50000000 ){
										sMessageBox2[i].Count = 50000000 - sinChar->Money;
										if(sMessageBox2[i].Count < 0 ) sMessageBox2[i].Count = 0;
									}
								break;
								//4차전업(임시)
								case 3:
								case 4:		// tier 5: the fifth rank
									if( ( sinChar->Money + sMessageBox2[i].Count) > 100000000 ){
										sMessageBox2[i].Count = 100000000 - sinChar->Money;
										if(sMessageBox2[i].Count < 0 ) sMessageBox2[i].Count = 0;
									}
								break;

							}

						}

					}
					
					if(i == MESSAGE_EVENT_SMASHTV){//스매쉬 TV
						sMessageBox2[i].szCount[sMessageBox2[i].Line]=szKeyBuff[j];
						sMessageBox2[i].Count = atoi(sMessageBox2[i].szCount);
						if(sinChar->Money < sMessageBox2[i].Count){
							sMessageBox2[i].Count = sinChar->Money;
							if(sMessageBox2[i].Count < (sinChar->Level * SMASHTV_MONEY)){
								ShowMessage(MESSAGE_NOT_ENOUGH_MONEY);
								CloseMessage(); //메세지박스를닫아준다 
							}
						}
						if( sMessageBox2[i].Count > (sinChar->Level * SMASHTV_MONEY)){ 
							sMessageBox2[i].Count = (sinChar->Level * SMASHTV_MONEY);

						}
		
					}

					if(i == 2 || i ==1 ){//포션 팔기나 버리기일경우에 
						sMessageBox2[i].szCount[sMessageBox2[i].Line]=szKeyBuff[j];
						sMessageBox2[i].Count = atoi(sMessageBox2[i].szCount); 
						if(pMessageItem && sMessageBox2[i].Count >=  pMessageItem->sItemInfo.PotionCount )
							sMessageBox2[i].Count = pMessageItem->sItemInfo.PotionCount;
					}
					if(i ==0 ){ //포션 사기일경우 
						sMessageBox2[i].szCount[sMessageBox2[i].Line]=szKeyBuff[j];
						sMessageBox2[i].Count = atoi(sMessageBox2[i].szCount);
						CountTemp = sMessageBox2[i].Count*pMessageItem->sItemInfo.Price;
						if(CountTemp > sinChar->Money){
							for(int k=0; k < CountTemp ; k++){
								if((CountTemp-k) <= sinChar->Money){
									sMessageBox2[i].Count =(CountTemp-k)/pMessageItem->sItemInfo.Price;
									break;
								}
							}
						}

						CountTemp = (sMessageBox2[i].Count*pMessageItem->sItemInfo.Weight)+sinChar->Weight[0];
						if(sinChar->Weight[1] < CountTemp){ //무게를 넘어서면
							for(int p=0; p < CountTemp ; p++){
								if((CountTemp-p) <= sinChar->Weight[1]){
									sMessageBox2[i].Count -=p;
									break;
								}
							}
						}
					}
				}	
			}
			if(sinGetKeyClick(VK_BACK)){
				if(i !=MESSAGE_BUY_ITEM_DEFALT){
					if(sMessageBox2[i].Count <=0)break;
					sMessageBox2[i].szCount[sMessageBox2[i].Line-1]=0;
					sMessageBox2[i].Count = atoi(sMessageBox2[i].szCount);
				}
			}
			if(sinGetKeyClick(VK_RETURN)){ //확인 버튼 
				CheckOkMessageBox(i);
			}
		}

	}
	if(sinGetKeyClick(VK_ESCAPE)){
		for(int i = 0 ; i < 20 ; i++){
			if(sMessageBox2[i].Flag  ){
				sMessageBox2[i].Flag = 0;
				sinMessageBoxShowFlag = 0;
			
			}

		}
		cSinSod2.ClanMasterMessageBoxFlag = 0;
		
		/////////// 창을 닫아준다  (보관소를 농락할수있다 안돼!!!!)
		/*
		if(cWareHouse.OpenFlag){
			cWareHouse.OpenFlag = 0;
			cWareHouse.CloseWareHouse();
		}
		*/

	}
	//////////스킬 습득 메세지 
	for( i = 0 ; i < MESSAGE3_MAX ; i++){
		if(sMessageBox3[i].Flag){
			if(sinGetKeyClick(VK_RETURN)){ //확인 버튼 
				CheckOkMessageBox2(i);
				//LearnSkillEffect(0);
			}

		}
	}
}
/*----------------------------------------------------------------------------*
*				이벤트시 메세지를 보여준다  
*-----------------------------------------------------------------------------*/	
int cMESSAGEBOX::ShowMessageEvent(char *Msg)
{ 
	for(int i = 0 ; i < 49 ; i++){ //이벤트 관련 메세지를 제외하곤 다지워준다
		if(sMessageBox[i].Flag)
			sMessageBox[i].Flag = 0;
	}

	sMessageBox[49].Flag = 1; //플랙을 준다 
	lstrcpy(sMessageBox[49].MessageDoc,Msg);
	sMessageBox[49].len = lstrlen(Msg);
	sMessageBox[49].x = 400-((sMessageBox[49].len*10)/2)+40; 
	sMessageBox[49].y = 5; 
	sMessageBox[49].Line = (sMessageBox[49].len*10/16)-6;
#ifdef _LANGUAGE_ENGLISH
	sMessageBox[49].Line = (sMessageBox[49].len*10/16)-3;
#endif
#ifdef _LANGUAGE_JAPANESE
	sMessageBox[49].Line = (sMessageBox[49].len*10/16)-4;
#endif
	return TRUE;
}
/*----------------------------------------------------------------------------*
*		메세지 박스 호출 (두종류로 나뉘다 ) *메세지만 보여주거나 *기능이있거나 
*-----------------------------------------------------------------------------*/	
int cMESSAGEBOX::ShowMessage(int Kind)   
{ //메세지만 보여준다 
	for(int i = 0 ; i < 100 ; i++){
		if(sMessageBox[i].Flag)
			sMessageBox[i].Flag = 0;
	}
	sMessageBox[Kind].Flag = 1; //플랙을 준다 
	if(Kind == MESSAGE_100LVITEM_OK){
		//이벤트 메세지에 아이템 이름을 넣기위한 TempBuff사용
		lstrcpy(sMessageBox[Kind].MessageDoc2,ha100LVQuestItemName[chaQuest.iFuryQuestIndex[1]-1]);
		lstrcat(sMessageBox[Kind].MessageDoc2,sMessageBox[Kind].MessageDoc);
		sMessageBox[Kind].len = lstrlen(sMessageBox[Kind].MessageDoc2);		
	}
	else
		sMessageBox[Kind].len = lstrlen(sMessageBox[Kind].MessageDoc);
	if(Kind == MESSAGE_MOVE_TRADE_ITEM ||Kind == MESSAGE_NOTEXIT_ITEMIMAGE ){	//사기방지 메세지는 가운데 보여준다 ,무게 보정메세지두
		sMessageBox[Kind].x = 400-((sMessageBox[Kind].len*10)/2)+40; 
		sMessageBox[Kind].y = 300-(64/2); 
		sMessageBox[Kind].Line = (sMessageBox[Kind].len*10/16)-6;
#ifdef _LANGUAGE_ENGLISH
		sMessageBox[Kind].Line = (sMessageBox[Kind].len*10/16)-3;
#endif
#ifdef _LANGUAGE_JAPANESE
	sMessageBox[Kind].Line = (sMessageBox[Kind].len*10/16)-4;
#endif
	}
	else{
		sMessageBox[Kind].x = 400-((sMessageBox[Kind].len*10)/2)+40; 
		//sMessageBox[Kind].y = 300-(64/2); 
		sMessageBox[Kind].y = 5; 
		sMessageBox[Kind].Line = (sMessageBox[Kind].len*10/16)-6;
#ifdef _LANGUAGE_ENGLISH
		sMessageBox[Kind].Line = (sMessageBox[Kind].len*10/16)-3;
#endif
#ifdef _LANGUAGE_JAPANESE
	sMessageBox[Kind].Line = (sMessageBox[Kind].len*10/16)-4;
#endif
	}
	
	return TRUE;
}

int cMESSAGEBOX::ShowMessage2(int Kind)   
{ //기능이 있는 메세지 박스 
	
	for(int i = 0 ; i < 20 ; i++){
		if(sMessageBox2[i].Flag)
			sMessageBox2[i].Flag = 0;
	}


	sMessageBox2[Kind].Flag = 1;   //플랙을 준다 
	memset(sMessageBox2[Kind].szCount,0,sizeof(sMessageBox2[Kind].szCount));
	if(Kind == MESSAGE_BUY_ITEM_DEFALT)
		sMessageBox2[Kind].Count = 1; //카운트는 1을 준다  (그냥 아이템 사기일경우 )
	else if(Kind == 1)
		sMessageBox2[Kind].Count = pMessageItem->sItemInfo.PotionCount; //팔수있는 아이템의 최대치 

	else
		sMessageBox2[Kind].Count = 0; //카운트는 0을 준다 

	if(MESSAGE_MYSHOP_ITEM == Kind){
		if(MyShopMouseItem.Class == ITEM_CLASS_POTION){
			lstrcpy(sMessageBox2[Kind].MessageDoc1,PotionCntDoc2);
		}
		else{
			lstrcpy(sMessageBox2[Kind].MessageDoc1,ItemPrice7);

		}
	}

	sMessageBox2[Kind].x    = 300;  
	sMessageBox2[Kind].y    = 200; 
	sMessageBox2[Kind].Line = 1; //한줄 
	sinMessageBoxShowFlag   = 1;
	hFocusWnd = 0; //메세지 박스가 있을때 채팅 포커스를 죽인다 

	//돈메세지 박스가 뜨면 복귀해준다 
	if(Kind == MESSAGE_MOVE_MONEY_TRADE){
		if(sTrade.Money){
			if(!CheckTrade((void *)&sTrade)) //잘못된 데이타 이면 거래창을 닫는다
				cTrade.CancelTradeItem();
			CheckCharForm();//인증 
			TempShowMoney = 0; //초기화  
			//sinChar->Money += sTrade.Money-193;
			sinPlusMoney(sTrade.Money-193);
			sTrade.Money = 0;
			ReformCharForm();//재인증 	
			SendSaveMoney(); //금액 조작을 못하게하기위해 호출한다 
			ReformTrade((void *)&sTrade);
			SendTradeItem(cTrade.TradeCharCode);
		}
	}
	
	return TRUE;
}

//메세지가 첨가될 메세지 박스를 보여준다 
int cMESSAGEBOX::ShowMessage3(int Kind , char *Doc) 
{
	int i=0; 
	for(i = 0 ; i < 20 ; i++){ 
		if(sMessageBox2[i].Flag)
			sMessageBox2[i].Flag = 0; 
	}
	for(i = 0 ; i < MESSAGE3_MAX ; i++){
		if(sMessageBox3[i].Flag)
			sMessageBox3[i].Flag = 0;
	}
	sMessageBox3[Kind].Flag = 1;   //플랙을 준다 
	sMessageBox3[Kind].x    = 300;  
	sMessageBox3[Kind].y    = 200; 
	sinMessageBoxShowFlag   = 1;
	lstrcpy(sMessageBox3[Kind].RecvMessage,Doc);
	
	if( Kind == MESSAGE_FALLGAME){
		sMessageBox3[Kind].x += 120;
		sMessageBox3[Kind].y += 100;
	}
	if(Kind == MESSAGE_INIT_SKILL_POINT){ //야매 땜빵 -_-
		sMessageBox3[Kind].x +=10;

	}
	if(Kind == MESSAGE_EVENT_GYUNGFUM){  
		sMessageBox3[Kind].x +=10;
		//sMessageBox3[Kind].y 
		sinInitPointFlag7 = 1;
		cSinHelp.sinShowHelp(SIN_HELP_KIND_INITPOINT,QuestMessageBoxPosi2.x,QuestMessageBoxPosi2.y,QuestMessageBoxSize2.x-3,QuestMessageBoxSize2.y-1, RGBA_MAKE(0,15,128,125),WarningInitPoint[0]);
	}
	if(Kind == MESSAGE_SECRET_NUM){
		hFocusWnd = hTextWnd;  
		SetWindowText( hFocusWnd , "");
		cInterFace.ChatFlag = 0;
		
	}

	if( Kind == MESSAGE_TELEPORT_DUNGEON ) // pluto 추가던전 메세지
	{
		sMessageBox3[Kind].x += 120;
		sMessageBox3[Kind].y += 100;
	}
	return TRUE;
}


int cMESSAGEBOX::CloseMessage()
{//메세지 박스를 닫아준다 
	for(int i = 0 ; i < 20 ; i++){
		if(sMessageBox2[i].Flag){
			sMessageBox2[i].Flag = 0;
			memset(sMessageBox2[i].szCount,0,sizeof(sMessageBox2[i].szCount));
			sMessageBox2[i].Count = 0;
		}
	}
	sinMessageBoxShowFlag = 0;
	

	return TRUE;
}

//메세지 박스의 내용을 보여준다 
void cMESSAGEBOX::DrawMessageText()
{


	HDC	hdc;
	int TempFlag = 0;
	char szTemp2[256];
	for(int i = 0 ; i < 100 ; i++){
		if(sMessageBox[i].Flag){
			TempFlag = 1;
		}
	}


	int len = 0;
	int XPosi = 0;
	char szBuffDoc[128];
	char szBuffDoc2[128];
	int Yposi = 0; 
	int PassNum =0; 

	//lstrcpy(sMessageBox_RecvItem.szDoc,"박상열만세 만세 만만만만1만만1만만만만만만만만만세  ioi ");
	//아이템 택배서비스  넓이 256 
	//sinBoldFont

	char HoldStarNum[32];   
	//별아이템
	if(sStarShop.Flag){                        
		lpDDSBack->GetDC( &hdc );   
		SelectObject( hdc, sinBoldFont);
		SetBkMode( hdc, TRANSPARENT );
		SetTextColor( hdc, RGB(222,231,255) );
		dsTextLineOut(hdc,BackStartPos.x +48,BackStartPos.y+35,HoldStarNumDoc,lstrlen(HoldStarNumDoc));

		wsprintf(HoldStarNum,"%s","x");
		dsTextLineOut(hdc,BackStartPos.x +120,BackStartPos.y+75,HoldStarNum,lstrlen(HoldStarNum));

		wsprintf(HoldStarNum,"%d",sStarShop.Count);
		dsTextLineOut(hdc,BackStartPos.x +160,BackStartPos.y+75,HoldStarNum,lstrlen(HoldStarNum));

		dsTextLineOut(hdc,BackStartPos.x +83,BackStartPos.y+140,StarItemBuy7,lstrlen(StarItemBuy7));

		SelectObject( hdc, sinFont);
		SetBkMode( hdc, TRANSPARENT );
		SetTextColor( hdc, RGB(222,231,25) );
		dsTextLineOut(hdc,BackStartPos.x +53,BackStartPos.y+120,ChangeMoney7,lstrlen(ChangeMoney7));
		lpDDSBack->ReleaseDC( hdc );

	}

	char szTempBuff5[128];
	if(sLost_Item.Flag){ 
		lpDDSBack->GetDC( &hdc );
		SelectObject( hdc, sinMessageFont);
		SetBkMode( hdc, TRANSPARENT );
		SetTextColor( hdc, RGB(128,255,128) );
		if(OverDay_Item_Flag){
			wsprintf(szTempBuff5,"%s %s",sLost_Item.szDoc,ItemName87);
			dsTextLineOut(hdc,BackStartPos.x +110,BackStartPos.y+90,szTempBuff5,lstrlen(szTempBuff5));

		}
		else{
			dsTextLineOut(hdc,BackStartPos.x +110,BackStartPos.y+90,Quset3ItemDoc15,lstrlen(Quset3ItemDoc15));
			lpDDSBack->ReleaseDC( hdc );
		}

		lpDDSBack->GetDC( &hdc );
		SelectObject( hdc, sinMessageFont);
		SetBkMode( hdc, TRANSPARENT );
		SetTextColor( hdc, RGB(255,255,255) );
		dsTextLineOut(hdc,BackStartPos.x +110,BackStartPos.y+120,Quset3ItemDoc16,lstrlen(Quset3ItemDoc16));
		lpDDSBack->ReleaseDC( hdc );

	}

	if(sMessageBox_RecvItem.Flag){        
		lpDDSBack->GetDC( &hdc );   
		SelectObject( hdc, sinBoldFont);
		SetBkMode( hdc, TRANSPARENT );
		SetTextColor( hdc, RGB(222,231,255) ); 
		memset(szBuffDoc,0,sizeof(szBuffDoc));
		memset(szBuffDoc2,0,sizeof(szBuffDoc2));

		len = lstrlen(sMessageBox_RecvItem.szDoc); 
		if((len*7) > 256){
			if((BYTE)sMessageBox_RecvItem.szDoc[256/7] & 0x80)
				PassNum = 0;
			else
				PassNum = 1;

			memcpy(szBuffDoc,sMessageBox_RecvItem.szDoc,(256/7)+PassNum);
			lstrcpy(szBuffDoc2,&sMessageBox_RecvItem.szDoc[(256/7)+PassNum]);
			dsTextLineOut(hdc,BackStartPos.x +10,BackStartPos.y+25,szBuffDoc,lstrlen(szBuffDoc));
			dsTextLineOut(hdc,BackStartPos.x +10,BackStartPos.y+40,szBuffDoc2,lstrlen(szBuffDoc2));
		}
		else{
			XPosi = ((256-(len*6))/2);
			if(XPosi <10 )XPosi = 10;
			dsTextLineOut(hdc,BackStartPos.x +XPosi,BackStartPos.y+30,sMessageBox_RecvItem.szDoc,lstrlen(sMessageBox_RecvItem.szDoc));
		}
		lpDDSBack->ReleaseDC( hdc );

		//요건 돈
		if(sMessageBox_RecvItem.RecvItem.SellPrice){
			lpDDSBack->GetDC( &hdc );
			SelectObject( hdc, sinMessageFont);
			SetBkMode( hdc, TRANSPARENT );
			SetTextColor( hdc, RGB(128,255,128) );
			wsprintf(szBuffDoc,"%d",sMessageBox_RecvItem.RecvItem.SellPrice);
			dsTextLineOut(hdc,BackStartPos.x +120,BackStartPos.y+90,szBuffDoc,lstrlen(szBuffDoc));
			lpDDSBack->ReleaseDC( hdc );

			lpDDSBack->GetDC( &hdc );
			SelectObject( hdc, sinMessageFont);
			SetBkMode( hdc, TRANSPARENT );
			SetTextColor( hdc, RGB(255,255,255) );
			dsTextLineOut(hdc,BackStartPos.x +130,BackStartPos.y+120,Money5,lstrlen(Money5));
			lpDDSBack->ReleaseDC( hdc );


		}
		//요건 경험치
		else if(sMessageBox_RecvItem.RecvItem.OldX){
			lpDDSBack->GetDC( &hdc );
			SelectObject( hdc, sinMessageFont);
			SetBkMode( hdc, TRANSPARENT );
			SetTextColor( hdc, RGB(128,255,128) );
			wsprintf(szBuffDoc,"%d",sMessageBox_RecvItem.RecvItem.OldX);
			dsTextLineOut(hdc,BackStartPos.x +120,BackStartPos.y+90,szBuffDoc,lstrlen(szBuffDoc));
			lpDDSBack->ReleaseDC( hdc );

			lpDDSBack->GetDC( &hdc );
			SelectObject( hdc, sinMessageFont);
			SetBkMode( hdc, TRANSPARENT );
			SetTextColor( hdc, RGB(255,255,255) );
			dsTextLineOut(hdc,BackStartPos.x +120,BackStartPos.y+120,Exp5,lstrlen(Exp5));
			lpDDSBack->ReleaseDC( hdc );

		}
		//요건 물약
		else if(sMessageBox_RecvItem.RecvItem.PotionCount){
			lpDDSBack->GetDC( &hdc );
			SelectObject( hdc, sinMessageFont);
			SetBkMode( hdc, TRANSPARENT );
			SetTextColor( hdc, RGB(128,255,128) );
			wsprintf(szBuffDoc,"%d",sMessageBox_RecvItem.RecvItem.PotionCount);
			dsTextLineOut(hdc,BackStartPos.x +120,BackStartPos.y+90,szBuffDoc,lstrlen(szBuffDoc));
			lpDDSBack->ReleaseDC( hdc );

			lpDDSBack->GetDC( &hdc );
			SelectObject( hdc, sinMessageFont);
			SetBkMode( hdc, TRANSPARENT );
			SetTextColor( hdc, RGB(255,255,255) );
			dsTextLineOut(hdc,BackStartPos.x +120,BackStartPos.y+120,RecvPotion7,lstrlen(RecvPotion7));
			lpDDSBack->ReleaseDC( hdc );


		}
		//요건 아이템 
		else{
			lpDDSBack->GetDC( &hdc );
			SelectObject( hdc, sinMessageFont);
			SetBkMode( hdc, TRANSPARENT );
			SetTextColor( hdc, RGB(128,255,128) );
			dsTextLineOut(hdc,BackStartPos.x +110,BackStartPos.y+90,sMessageBox_RecvItem.szItem,lstrlen(sMessageBox_RecvItem.szItem));
			lpDDSBack->ReleaseDC( hdc );

			lpDDSBack->GetDC( &hdc );
			SelectObject( hdc, sinMessageFont);
			SetBkMode( hdc, TRANSPARENT );
			SetTextColor( hdc, RGB(255,255,255) );
			dsTextLineOut(hdc,BackStartPos.x +120,BackStartPos.y+120,Itemul,lstrlen(Itemul));
			lpDDSBack->ReleaseDC( hdc );
		}

		lpDDSBack->GetDC( &hdc );
		SelectObject( hdc, sinMessageFont);
		SetBkMode( hdc, TRANSPARENT );
		SetTextColor( hdc, RGB(255,255,255) );
		dsTextLineOut(hdc,BackStartPos.x +110,BackStartPos.y+140,ItemRecvOk,lstrlen(ItemRecvOk));
		lpDDSBack->ReleaseDC( hdc );
		
	}
	
	if(sinMessageBoxShowFlag || TempFlag){

	
	int i=0;
	int TempLenght = 0;

	lpDDSBack->GetDC( &hdc );
	SelectObject( hdc, sinMessageFont);
    SetBkMode( hdc, TRANSPARENT );
	SetTextColor( hdc, RGB(255,255,255) );
	char szBuff5[128];
	memset(szBuff5,0,sizeof(szBuff5));


	for(i = 0 ; i < 100 ; i++){
		if(sMessageBox[i].Flag){ //날치기로 맞췄다 
			if(i == MESSAGE_100LVITEM_OK){
				//이벤트 메세지에 아이템 이름을 넣기위한 사용
				dsTextLineOut(hdc,sMessageBox[i].x+43,sMessageBox[i].y+17,sMessageBox[i].MessageDoc2,sMessageBox[i].len );
			}
			else
				dsTextLineOut(hdc,sMessageBox[i].x+43,sMessageBox[i].y+17,sMessageBox[i].MessageDoc,sMessageBox[i].len );
		}
	}

	char szBuff[128];
	char szBuff15[128];
	int len2 = 0;  
	memset(szBuff,0,sizeof(szBuff));
	memset(szBuff15,0,sizeof(szBuff15));
	
	for(i = 0 ; i < 20 ; i++){
		if(sMessageBox2[i].Flag){ //날치기로 맞췄다 
			dsTextLineOut(hdc,sMessageBox2[i].x+45,sMessageBox2[i].y+15,sMessageBox2[i].MessageDoc1,lstrlen(sMessageBox2[i].MessageDoc1) );
			dsTextLineOut(hdc,sMessageBox2[i].x+45,sMessageBox2[i].y+40,sMessageBox2[i].MessageDoc2,lstrlen(sMessageBox2[i].MessageDoc2) );
	 		wsprintf(sMessageBox2[i].szCount,"%d",sMessageBox2[i].Count); 
			NumLineComa(sMessageBox2[i].Count, szBuff5);
			for(int t = 0 ; t < 10 ; t ++){
				if(CheckCountSize[t] > sMessageBox2[i].Count){ 
					sMessageBox2[i].Line = t;
					break;
				}
			}
			if(i == MESSAGE_MYSHOP_POTION){ //포션사기일경우에는 가격을 표시해준다            
				NumLineComa(sMyShop_Server.Price*sMessageBox2[i].Count,szBuff);
				len2 = lstrlen(szBuff);
				//wsprintf(szBuff15,"(%s) %s",szBuff5,szBuff);
				sMessageBox2[i].TextSetX =  (int)(((sMessageBox2[i].x + 60)) -(((sMessageBox2[i].Line) * 6.5)/2));

				wsprintf(szBuff15,"%s %s",BuyPotionKind,szBuff5);
				len2 = lstrlen(szBuff15);
				dsTextLineOut(hdc,sMessageBox2[i].TextSetX-(len2),sMessageBox2[i].y+58,szBuff15,lstrlen(szBuff15) );	

				wsprintf(szBuff15,"%s %d",BuyPotionMoney,sMyShop_Server.Price*sMessageBox2[i].Count);
				len2 = lstrlen(szBuff15);
				dsTextLineOut(hdc,sMessageBox2[i].TextSetX-(len2),sMessageBox2[i].y+72,szBuff15,lstrlen(szBuff15) );	

				//dsTextLineOut(hdc,sMessageBox2[i].TextSetX,sMessageBox2[i].y+72,sMessageBox2[i].szCount,lstrlen(sMessageBox2[i].szCount) );	
//				dsTextLineOut(hdc,sMessageBox2[i].TextSetX,sMessageBox2[i].y+72,szBuff15,lstrlen(szBuff15) );	

			}
			else{
				len2 = lstrlen(szBuff5);
				sMessageBox2[i].TextSetX =  (int)((sMessageBox2[i].x + 85-(len2)) -(((sMessageBox2[i].Line) * 6.5)/2));
				//dsTextLineOut(hdc,sMessageBox2[i].TextSetX,sMessageBox2[i].y+72,sMessageBox2[i].szCount,lstrlen(sMessageBox2[i].szCount) );	
				dsTextLineOut(hdc,sMessageBox2[i].TextSetX,sMessageBox2[i].y+72,szBuff5,lstrlen(szBuff5) );	
			}
		}
	}
	for( i = 0 ; i < MESSAGE3_MAX ; i++){ 
		if(sMessageBox3[i].Flag){  
			TempLenght = (int)((172-(lstrlen(sMessageBox3[i].RecvMessage)*5.5))/2-5);
			SelectObject( hdc, sinBoldFont);
			SetBkMode( hdc, TRANSPARENT );
			SetTextColor( hdc, RGB(222,231,255) );
			dsTextLineOut(hdc,sMessageBox3[i].x+TempLenght,sMessageBox3[i].y+20,sMessageBox3[i].RecvMessage,lstrlen(sMessageBox3[i].RecvMessage) );
			SelectObject( hdc, sinFont);
			SetBkMode( hdc, TRANSPARENT );
			SetTextColor( hdc, RGB(255,255,255) );
			TempLenght = (int)((172-(lstrlen(sMessageBox3[i].MessageDoc1)*5.5))/2-5);
			dsTextLineOut(hdc,sMessageBox3[i].x+TempLenght,sMessageBox3[i].y+35+8,sMessageBox3[i].MessageDoc1,lstrlen(sMessageBox3[i].MessageDoc1) );
			if( i == MESSAGE_EVENT_SMASHTV){ //왕따시 날치기 ㅡ,.ㅡ
				wsprintf(szTemp2,"%d%s",sinChar->Level*SMASHTV_MONEY , sMessageBox3[i].MessageDoc2);	
				dsTextLineOut(hdc,sMessageBox3[i].x+48,sMessageBox3[i].y+55+5,szTemp2,lstrlen(szTemp2) );
			}
			else if( i ==MESSAGE_SECRET_NUM){
				sinChatEnter = 0; //임의로 글씨가 안보이게한다
				cInterFace.ChatFlag = 0;
				hFocusWnd = hTextWnd;  
				GetWindowText( hFocusWnd , sMessageBox3[i].MessageDoc2 , 15 );
				dsTextLineOut(hdc,sMessageBox3[i].x+48,sMessageBox3[i].y+55+5,sMessageBox3[i].MessageDoc2,lstrlen(sMessageBox3[i].MessageDoc2) );

			}
			else{
				TempLenght = (int)((172-(lstrlen(sMessageBox3[i].MessageDoc2)*5.5))/2-5);
				dsTextLineOut(hdc,sMessageBox3[i].x+TempLenght+2,sMessageBox3[i].y+55+5,sMessageBox3[i].MessageDoc2,lstrlen(sMessageBox3[i].MessageDoc2) );
			}
		}
	}
	lpDDSBack->ReleaseDC( hdc );
	}

}
int sinTeleportIndexArray[4] = {0,18,7,12};
///메세지가 삽일될 메세지 박스 
int cMESSAGEBOX::CheckOkMessageBox2(int Kind)
{

	int Temp = 0;
	int TempTalent = 0;
	sITEM TempItem;

	switch(Kind){    
		case MESSAGE_MASTER_SKILL:   //스킬 배우기 
			if(pMasterSkill){
				//--------------------------------------------------------------------------//
				#ifdef HASIEGE_MODE
					//<ha>공성전 세율적용에 스킬 배우기
					if(cShop.haBuyMoneyCheck(sinMasterSkill_Money[pMasterSkill->Skill_Info.SkillNum]+(sinMasterSkill_PlusMoney[pMasterSkill->Skill_Info.SkillNum]*(pMasterSkill->Point)))){
				#else
					if(	0 <=  (sinChar->Money - (sinMasterSkill_Money[pMasterSkill->Skill_Info.SkillNum]+(sinMasterSkill_PlusMoney[pMasterSkill->Skill_Info.SkillNum]*(pMasterSkill->Point))))){
				#endif
				//---------------------------------------------------------------------------//	
					CheckCharForm();//인증 
					//sinChar->Money -= sinMasterSkill_Money[pMasterSkill->Skill_Info.SkillNum]+(sinMasterSkill_PlusMoney[pMasterSkill->Skill_Info.SkillNum]*(pMasterSkill->Point));
					//<ha>공성전 세율적용한 스킬 머니를 서버로 보낸다.
					SendPaymentMoneyToServer((sinMasterSkill_Money[pMasterSkill->Skill_Info.SkillNum]+(sinMasterSkill_PlusMoney[pMasterSkill->Skill_Info.SkillNum]*(pMasterSkill->Point)))+
						((sinMasterSkill_Money[pMasterSkill->Skill_Info.SkillNum]+(sinMasterSkill_PlusMoney[pMasterSkill->Skill_Info.SkillNum]*(pMasterSkill->Point)))*cSinSiege.GetTaxRate())/100,1);

					
					sinMinusMoney(sinMasterSkill_Money[pMasterSkill->Skill_Info.SkillNum]+(sinMasterSkill_PlusMoney[pMasterSkill->Skill_Info.SkillNum]*(pMasterSkill->Point)),1);
					cSkill.CheckSkillPointForm(pMasterSkill);
					pMasterSkill->Point++;
					cSkill.ReformSkillPointForm(pMasterSkill);
					
					//4차스킬일경우
					if(Skill4ButtonIndex > 13){
						sinSkill.SkillPoint4--;
					}
					//1.2.3차 스킬일경우
					if(Skill4ButtonIndex <= 13){
						sinSkill.SkillPoint --;
					}
					
					ReformCharForm();//재인증 
					SendSaveMoney(); //금액 조작을 못하게하기위해 호출한다 
					sinPlaySound(SIN_SOUND_COIN);
					switch(pMasterSkill->CODE){
						case SKILL_POISON_ATTRIBUTE:	//독속성 증가 
							pUseSkill = pMasterSkill;
							pUseSkill->Skill_Info.FuncPointer();
							break;
						case SKILL_MELEE_MASTERY:       //데미지 증가 
							pUseSkill = pMasterSkill;
							pUseSkill->Skill_Info.FuncPointer();
							break;
						case SKILL_FIRE_ATTRIBUTE:      //불속성 증가 
							pUseSkill = pMasterSkill;
							pUseSkill->Skill_Info.FuncPointer();
							break;
						case SKILL_ICE_ATTRIBUTE:       //얼음 속성 증가 
							pUseSkill = pMasterSkill;
							pUseSkill->Skill_Info.FuncPointer();
							break;
						case SKILL_SHOOTING_MASTERY:    //활 데미지 증가  
							pUseSkill = pMasterSkill;
							pUseSkill->Skill_Info.FuncPointer();
							break;
						case SKILL_MAXIMIZE:			//최대 데미지 증가 
							pUseSkill = pMasterSkill;
							pUseSkill->Skill_Info.FuncPointer();
							break;
						case SKILL_WEAPONE_DEFENCE_MASTERY:	//블럭율 증가 
							pUseSkill = pMasterSkill;
							pUseSkill->Skill_Info.FuncPointer();
							break;
						case SKILL_DIONS_EYE:	//명중률 증가  
							pUseSkill = pMasterSkill;
							pUseSkill->Skill_Info.FuncPointer();
							break;
						case SKILL_PHYSICAL_TRANING: //근력량 추가 
							cInvenTory.SetItemToChar();
							break;
						case SKILL_FIRE_JAVELIN:
							cInvenTory.SetItemToChar();
							break;
						case SKILL_MEDITATION:
							cInvenTory.SetItemToChar();
							break;
						case SKILL_MENTAL_MASTERY:
							cInvenTory.SetItemToChar();
							break;
						case SKILL_THROWING_MASTERY:
							cInvenTory.SetItemToChar();
							break;
						case SKILL_MECHANIC_WEAPON:
							cInvenTory.SetItemToChar();
							break;
						case SKILL_CRITICAL_MASTERY:
							cInvenTory.SetItemToChar();
							break;
						case SKILL_SWORD_MASTERY:
							cInvenTory.SetItemToChar();
							break;
						case SKILL_BOOST_HEALTH:
							cInvenTory.SetItemToChar();
							break;
						case SKILL_EVASION_MASTERY:
							cInvenTory.SetItemToChar();
							break;

					};
					cInvenTory.CheckDamage(); //데미지를 체크해서 서버로보낸다
					LearnSkillEffect(0);
					TempTalent = (int)(sinChar->Talent/3);   ////////숙련도를 보정한다 
					if(TempTalent > 50 )TempTalent = 50;
					Temp = 10000 -  (TempTalent*100);
					
					if(Temp >=pMasterSkill->UseSkillCount)
						pMasterSkill->UseSkillCount /=2;
					else
						pMasterSkill->UseSkillCount = Temp/2;
					SaveGameData();

				}
				else
					ShowMessage(MESSAGE_NOT_ENOUGH_MONEY);

			}
			
		break;
		case MESSAGE_CHANGE_JOB:
			if(sinQuest_ChangeJob3.CODE){ 
				TempItem.CODE = (sinOR2|sin02);
				if(!cInvenTory.CheckInvenEmpty(&TempItem)){
					ShowMessage(MESSAGE_OVER_SPACE);
					break;
				}

			}
			///////////////// 이곳에 퀘스트 수행 메세지가 나온다 
			sinChar->ChangeJob++; //전업을 한다 
			cSkill.OpenFlag = 1;
			cInterFace.CheckAllBox(SIN_SKILL); //창을 하나만 띄운다 
			SkillMasterFlag = 1;  //스킬을 할당할수있게 해준다

			if(sinQuest_ChangeJob.CODE){ //템스크론
				DeleteQuestItem((sinQT1|sin01)); //퀘스트 아이템을 지워준다
				DeleteQuestItem((sinQT1|sin02));
				DeleteQuestItem((sinQT1|sin03));

				sinQuest_ChangeJob.State = SIN_QUEST_COMPLETE;
				sinJobList = GetJobDataCode(sinChar->JOB_CODE , sinChar->ChangeJob);
				lstrcpy(cInvenTory.JobName,sinJobList->szName2);
				
				sinQuest_ChangeJob.BackUpCode = sinQuest_ChangeJob.CODE;
				sinQuest_ChangeJob.CODE = 0; //퀘스트가 완료되면 코드가 사라진다 
				sinChar->wVersion[1]    = 0;
				EndQuest_Code(sinQuest_ChangeJob.BackUpCode);
				
				cInvenTory.ReFormInvenItem();
				sinSkillPontInitButtonFlag = 1; //스킬 포이트 초기화 박스를 띄운다 
			}
			if(FindLastQuestCode(SIN_QUEST_CODE_CHANGEJOB2_NPC_M) ||FindLastQuestCode(SIN_QUEST_CODE_CHANGEJOB2_NPC_D)){ //모라이온
				sinJobList = GetJobDataCode(sinChar->JOB_CODE , sinChar->ChangeJob);
				lstrcpy(cInvenTory.JobName,sinJobList->szName2);
				sinSkillPontInitButtonFlag = 1; //스킬 포이트 초기화 박스를 띄운다 
				//이부분에서 퀘스트코드관련 초기화를 해준
				sinQuest_ChangeJob2.CODE = 0;
				
			}
			if(sinQuest_ChangeJob3.CODE){ 
			//if(FindLastQuestCode(SIN_QUEST_CODE_CHANGEJOB3)){
				sinJobList = GetJobDataCode(sinChar->JOB_CODE , sinChar->ChangeJob);
				lstrcpy(cInvenTory.JobName,sinJobList->szName2);
				sinSkillPontInitButtonFlag = 1; //스킬 포이트 초기화 박스를 띄운다 
				//이부분에서 퀘스트코드관련 초기화를 해준
				//sinQuest_ChangeJob3.BackUpCode = sinQuest_ChangeJob3.CODE;
				//sinQuest_ChangeJob3.CODE = 0; //퀘스트가 완료되면 코드가 사라진다 
				//EndQuest_Code(sinQuest_ChangeJob3.BackUpCode);
				sinQuest_ChangeJob3.State = 5;
				StartQuest_Code(sinQuest_ChangeJob3.CODE);
			}

	
			SaveGameData(); //전업후 저장한다
			ChangeJobFace();
		break;
		case MESSAGE_QUEST:	 //퀘스트 수행메세지 표시 
			sinQuest_ChangeJob.CODE = SIN_QUEST_CODE_CHANGEJOB;
			sinQuest_ChangeJob.State = SIN_QUEST_PROGRESS;
			SaveGameData();
			StartQuest_Code(sinQuest_ChangeJob.CODE);
		break;
		case MESSAGE_QUEST_CHANGEJOB_MORAYOIN:
			sinQuest_ChangeJob2.State = SIN_QUEST_PROGRESS;
			// sinQuest_ChangeJob2.Kind = GetRandomPos(1,2); //전업퀘스트를 A,B로 셋팅한다
			//sinQuest_ChangeJob2.Kind = 2;
			SaveGameData();
			StartQuest_Code(sinQuest_ChangeJob2.CODE); //셋팅되는 부분은 나중에 다시봐야한다 
			switch(sinQuest_ChangeJob2.Kind){
				case 1:
				sinQuest_ChangeJob2.CODE = SIN_QUEST_CODE_CHANGEJOB2_NPC_M; //몰리 할머니 벌꿀퀘스트
				break;
				case 2:
				sinQuest_ChangeJob2.CODE = SIN_QUEST_CODE_CHANGEJOB2_NPC_D; //몰리 할머니 벌꿀퀘스트
				break;
			}
			SaveGameData();
			StartQuest_Code(sinQuest_ChangeJob2.CODE); //셋팅되는 부분은 나중에 다시봐야한다 
			//퀘스트아이템이 있는지를 체크해서 날려버린다 
			cInvenTory.ClearQuestItem();
		break;
		case MESSAGE_QUEST_CHANGEJOB3: //3차전직 퀘스트 
			sinQuest_ChangeJob3.CODE  = SIN_QUEST_CODE_CHANGEJOB3;
			sinQuest_ChangeJob3.State = 1;
			SaveGameData();
			StartQuest_Code(sinQuest_ChangeJob3.CODE); //셋팅되는 부분은 나중에 다시봐야한다 
		break;

		case MESSAGE_INIT_SKILL_POINT: //스킬 포인트 초기화 
			cSkill.InitSkillPoint();
		break;
		case MESSAGE_EVENT_GYUNGFUM: //능력치 초기화로 사용한다 
			cCharStatus.InitCharStatus();	 			
			cInvenTory.SetItemToChar();				
			if ( sinChar->Life[1]<sinGetLife() ) {
				sinSetLife( sinChar->Life[1] );
				ReformCharForm();//재인증 
				ResetEnergyGraph(4);
			}
			sinInitPointFlag7 = 0;
			//cMessageBox.ShowMessage(MESSAGE_WARNING_WEIGHT);
			//sinSendEventItem(1); //이벤트 아이템을 서버로보내고 삭제한다 
		break;
		
		case MESSAGE_EVENT_SMASHTV: //이벤트 스메쉬 TV 
			if(sinChar->Money < (sinChar->Level * SMASHTV_MONEY)){  
				CloseMessage(); //메세지박스를닫아준다 
				ShowMessage(MESSAGE_NOT_ENOUGH_MONEY);
				break;

			}
			CheckCharForm();//인증 
			//sinChar->Money -= sMessageBox2[Kind].Count;
			sinMinusMoney((sinChar->Level * SMASHTV_MONEY));
			ReformCharForm();//재인증 	
			SendCollectMoney((sinChar->Level * SMASHTV_MONEY));
			sinPlaySound(SIN_SOUND_COIN);
			//ShowMessage(MESSAGE_GIVE_MONEY_TRUE);
			//요기서 이벤트에 입장하면 되겠심다 두둥!
			
		break;
		case MESSAGE_POSTBOX:
			//여기서 아이템 받는 함수를 호출한다 
			//SendItemExpress( ExpressItemCode );
			/*
			if(sPostbox_Item[PostBoxIndexCnt+1].CODE){
				PostBoxIndexCnt++;
			}
			else PostBoxIndexCnt =0; //0으로 플랙을 초기화해서 체크한다 
			*/
			

		break;
		case MESSAGE_LINKCORE:
			//cInvenTory.SetLinkCore(); //링크코어를 셋팅한다
			cInvenTory.UnionCoreParticle2(UnionCorePosi.x , UnionCorePosi.y); //이펙트 
			sinPlaySound(SIN_SOUND_EAT_POTION2); //유니온코어 사운드
			SendLinkCoreToServer(&TempLinkCoreItem);	

		break;
		case MESSAGE_RECVITEM:			//아이템을 받을때 확인창
			SendWingQuestItem();
			sinbaram_Stop = 1;
			//WingItemQuestIndex
			//서버에 아이템을 보낸다 
		break;

		case MESSAGE_WARP:
			if(sSinWarpGate.GateUseIndex > 2 && sSinWarpGate.MoveFieldNum >1 && !sSinWarpGate.SameAreaFlag ){ 
				CheckCharForm(); //인증 
				//<ha>공성전 세율적용한 워프게이트 머니를 서버로 보낸다.
				SendPaymentMoneyToServer(WarpGateUseCost[sSinWarpGate.GateUseIndex-4]+((WarpGateUseCost[sSinWarpGate.GateUseIndex-4]*cSinSiege.GetTaxRate())/100),2);
				sinMinusMoney(WarpGateUseCost[sSinWarpGate.GateUseIndex-4],1); //돈을뺀다
				ReformCharForm(); //재인증 
			}
			if(sHaWarpGate.GateUseIndex > 0 &&sHaWarpGate.MoveFieldNum >0&&!sSinWarpGate.SameAreaFlag ){
				CheckCharForm(); //인증 
				//<ha>공성전 세율적용한 워프게이트 머니를 서버로 보낸다.
				SendPaymentMoneyToServer(WarpGateUseCost[sSinWarpGate.GateUseIndex-4]+((WarpGateUseCost[sSinWarpGate.GateUseIndex-4]*cSinSiege.GetTaxRate())/100),2);
				sinMinusMoney(WarpGateUseCost[sSinWarpGate.GateUseIndex-4],1); //돈을뺀다
				ReformCharForm(); //재인증 
				WingWarpGate_Field(haWarpGateCODE[sHaWarpGate.MoveFieldNum-1]);	
			}
			else
				WingWarpGate_Field(sinWarpGateCODE[sSinWarpGate.MoveFieldNum]);

			SendSaveMoney(); //금액 조작을 못하게하기위해 호출한다 

		break;
		case MESSAGE_SECRET_NUM:
			SendItemExpress(sMessageBox_RecvItem.RecvItem.CODE,sMessageBox3[Kind].MessageDoc2);
			isDrawClanMenu = 0;
			sMessageBox_RecvItem.Flag = 0;
			hFocusWnd = hTextWnd;
			SetWindowText(hFocusWnd,"");
			cInterFace.ChatFlag = 0;
		break;
		case MESSAGE_GIFT_TICKET:
			//GiftPrice (양만큼 별구매)
			SendStarPointToServer(GiftPrice,0);
			DeleteGiftItem();
			StarShopNPC();
		break;
		case MESSAGE_SELL_HIGHRANK_ITEM:
			MouseItem.Flag = 1; //살짜쿵 땜빵 움화화홧
			ResetInvenItemCode(); //마우스에 플렉을 넣을때 체크해준다
			cShop.SellItemToShop(&MouseItem);
			
		break; 
		case MESSAGE_GIVE_MONEY_CHECK:
			CheckCharForm();//인증 
			sinMinusMoney2(sinGiveMoneyAmount);
			ReformCharForm();//재인증 	
			//여기서 서버로 돈을 보낸다
			SendGiveMoneyToServer(sinGiveMoneyAmount);
			sinPlaySound(SIN_SOUND_COIN);
			sinGiveMoneyAmount = 0;

		break;
		case MESSAGE_PUZZLE_SEND:
			if(cInvenTory.CheckSendPuzzle()){ 
				SendWingQuestItem();
			}
		break;
		case MESSAGE_MYSHOP_ITEM_BUY:
			if(cCharShop.OpenFlag){
				Send_PersonalShopItem(sRecvMyShop.CHAR_CODE,&sMyShop_Server); //서버로 구매할 아이템의 정보를 보낸다
			}
		break;
		case MESSAGE_MYSHOP_ITEM_CANCEL:
			MyShopItemIndex[MyShopItemCancelIndex[0]-1] = 0; 
			cMyShop.MyShopItem[MyShopItemCancelIndex[1]-1].Flag = 0;
			memset(&cMyShop.MyShopItem[MyShopItemCancelIndex[1]-1],0,sizeof(sITEM));
		break;
		case MESSAGE_EVENTITEM_SEND:
			if(cInvenTory.SendEventItem()){ 
				SendWingQuestItem();
			}
		break;
		case MESSAGE_FOXEVENTITEM_SEND:
			if(cInvenTory.SendFoxEventItem()){   
				SendWingQuestItem();
			}
		case MESSAGE_QUEST_LEVEL30:
			sinQuest_Level30.CODE  = SIN_QUEST_CODE_LEVEL30;
			sinQuest_Level30.State = 1;
			sinQuest_Level30.Monster[1] = 20; //바곤 20마리
			SaveGameData();
			StartQuest_Code(sinQuest_Level30.CODE); 
		break;
		case MESSAGE_QUEST_LEVEL55:
			sinQuest_Level55.CODE  = SIN_QUEST_CODE_LEVEL55;
			sinQuest_Level55.State = 1;
			sinQuest_Level55.Monster[1] = 20; //머핀 20마리
			SaveGameData();
			StartQuest_Code(sinQuest_Level55.CODE); 
		break;
		case MESSAGE_QUEST_LEVEL55_2:
			sinQuest_Level55_2.CODE  = SIN_QUEST_CODE_LEVEL55_2;
			sinQuest_Level55_2.State = 1;
			sinQuest_Level55_2.Monster[1] = 20; //다이어비 20마리
			SaveGameData();
			StartQuest_Code(sinQuest_Level55_2.CODE); 
		break;

		case MESSAGE_QUEST_LEVEL70:
			sinQuest_Level70.CODE  = SIN_QUEST_CODE_LEVEL70;
			sinQuest_Level70.State = 1;
			sinQuest_Level70.Monster[1] = 10; //타이푼 10마리
			SaveGameData();
			StartQuest_Code(sinQuest_Level70.CODE); 

		break;
		case MESSAGE_QUEST_LEVEL80:
			sinQuest_Level80.CODE  = SIN_QUEST_CODE_LEVEL80;
			sinQuest_Level80.State = 1;
			sinQuest_Level80.Monster[1] = 5; //라투 5마리
			SaveGameData();
			StartQuest_Code(sinQuest_Level80.CODE); 

		break;
		case MESSAGE_QUEST_LEVEL85:
			sinQuest_Level85.CODE  = SIN_QUEST_CODE_LEVEL85;
			sinQuest_Level85.State = 2; //상태가 1이면 사망 
			sinQuest_Level85.Monster[1] = 10; //그로테스크 10마리
			SaveGameData();
			StartQuest_Code(sinQuest_Level85.CODE); 
			if(!cInvenTory.SearchItemCode(sinQT1|sin08)){
				SendQuestCommandToServer( SIN_QUEST_CODE_LEVEL85 , 0, 0, 0 );
			}

		break;
		case MESSAGE_QUEST_LEVEL90:
			sinQuest_Level90.CODE  = SIN_QUEST_CODE_LEVEL90;
			sinQuest_Level90.State = 1; 
			sinQuest_Level90.Monster[1] = 10; //그로테스크 10마리
			SaveGameData();
			StartQuest_Code(sinQuest_Level90.CODE); 
		break;
		case MESSAGE_QUEST_LEVEL80_2:
			sinQuest_Level80_2.CODE  = SIN_QUEST_CODE_LEVEL80_2;
			sinQuest_Level80_2.State = 1; 
			SaveGameData();
			StartQuest_Code(sinQuest_Level80_2.CODE); 
		break;
		case MESSAGE_QUEST_LEVEL90_2:
			sinQuest_Level90_2.CODE  = SIN_QUEST_CODE_LEVEL90_2;
			sinQuest_Level90_2.State = 1; 
			SaveGameData();
			StartQuest_Code(sinQuest_Level90_2.CODE); 
		break;

		case MESSAGE_TELEPORT:
			CheckCharForm(); //인증 
			//<ha>공성전 세율적용한 텔레포트 머니를 서버로 보낸다.
			SendPaymentMoneyToServer(TeleportUseCose[sinTeleportMoneyIndex-1]+((TeleportUseCose[sinTeleportMoneyIndex-1]*cSinSiege.GetTaxRate())/100),2);
			
			sinMinusMoney(TeleportUseCose[sinTeleportMoneyIndex-1],1); //돈을뺀다
			WarpField2( sinTeleportIndexArray[sinTeleportMoneyIndex-1] );
			sinPlaySound(SIN_SOUND_COIN);
			ReformCharForm(); //재인증 
		break;
		case MESSAGE_CHANGE_JOB4: //퀘스트 시작
			sinQuest_ChangeJob4.CODE  = SIN_QUEST_CODE_CHANGEJOB4;
			sinQuest_ChangeJob4.State = 1; 
			StartQuest_Code(sinQuest_ChangeJob4.CODE); 
			sinChangeJob4MonSet(&sinQuest_ChangeJob4); //몬스터를 셋팅한다
			SaveGameData();
		break;
		case MESSAGE_CHANGE_JOB4_2: //퀘스트 완료
			///////////////// 이곳에 퀘스트 수행 메세지가 나온다 
			sinChar->ChangeJob++; //전업을 한다 
			cSkill.OpenFlag = 1;
			cInterFace.CheckAllBox(SIN_SKILL); //창을 하나만 띄운다 
			SkillMasterFlag = 1;  //스킬을 할당할수있게 해준다
			sinQuest_ChangeJob4.State = 0;
			sinJobList = GetJobDataCode(sinChar->JOB_CODE , sinChar->ChangeJob);
			lstrcpy(cInvenTory.JobName,sinJobList->szName2);
				
			sinQuest_ChangeJob4.BackUpCode = sinQuest_ChangeJob4.CODE;
			sinQuest_ChangeJob4.CODE = 0; //퀘스트가 완료되면 코드가 사라진다 
			sinChar->wVersion[1]     = 0;
			EndQuest_Code(sinQuest_ChangeJob4.BackUpCode);
				
			cInvenTory.ReFormInvenItem();
			sinSkillPontInitButtonFlag = 1; //스킬 포인트 초기화 박스를 띄운다 
			SaveGameData(); //전업후 저장한다

			ChangeJobFace();
			if(sinChar->JOB_CODE <=4 ){
				WarpField2(START_FIELD_NUM); //돌아간다
			}else
			{
				WarpField2(START_FIELD_MORYON); //돌아간다
			}
		break;
		case MESSAGE_CRISTALEVENTITEM_SEND: //일곱가지 크리스탈의 보은
			if(cInvenTory.SendCristalEventItem()){   
				SendWingQuestItem();
			}
		break;
		case MESSAGE_WATERMELON_EVENTITEM: // 박재원 - 수박 모아오기
			if(cInvenTory.SendWatermelonEventItem()){   
				SendWingQuestItem();
			}
		break;
		case MESSAGE_VALENTINE_EVENTITEM: // 장별 - 발렌타인 초콜릿 모아오기
			if(cInvenTory.SendValentineEventItem())
			{   
				SendWingQuestItem();
			}
		break;
		case MESSAGE_PRISTON_ALPHABET_EVENTITEM: // 박재원 - 알파벳 조합 이벤트
			if(cInvenTory.SendPristonAlphabetEventItem()){   
				SendWingQuestItem();
			}
		break;

		case MESSAGE_CANDYDAYS_EVENTITEM: // 장별 - 캔디데이즈
			if(cInvenTory.SendCandydaysEventItem())
			{   
				SendWingQuestItem();
			}
		break;

		case MESSAGE_MAGICALGREEN_EVENTITEM: // 장별 - 매지컬그린
			if(cInvenTory.SendMagicalGreenEventItem())
			{   
				SendWingQuestItem();
			}
		break;

		case MESSAGE_MAGICALGREEN_EVENTITEM_EMERALD: // 장별 - 매지컬그린 에메랄드
			if(cInvenTory.SendMagicalGreenEmeraldEventItem())
			{   
				SendWingQuestItem();
			}
		break;

		case MESSAGE_TEAROFKARA_EVENTITEM: // 장별 - 카라의 눈물
			if(cInvenTory.SendTearOfKaraEventItem())
			{   
				SendWingQuestItem();
			}
		break;

		case MESSAGE_FIND_EVENTITEM: // 장별 - 조사원을 찾아라 - 마력 연구자
			if(cInvenTory.SendFindinvestigatorNineEventItem())
			{   
				SendWingQuestItem();
			}
		break;

		case MESSAGE_FINDKAMIU_EVENTITEM: // 장별 - 조사원을 찾아라 - 카미유
			if(cInvenTory.SendFindinvestigatorTaleEventItem())
			{   
				SendWingQuestItem();
			}
		break;

		case MESSAGE_FINDETO_EVENTITEM:	// 장별 - 조사원을 찾아라 - 에토
			if(cInvenTory.SendFindinvestigatorTaleEtoEventItem())
			{   
				SendWingQuestItem();
			}
		break;

		case MESSAGE_CASTLE_TELEPORT :      //블레스 캐슬 텔레포트
			CheckCharForm(); //인증 

			WarpField2( 33 );
			sinMinusMoney(haC_TeleportMoney); //돈을뺀다
			sinPlaySound(SIN_SOUND_COIN);
			ReformCharForm(); //재인증 
			
			// 장별 - 블레스 캐슬
			/*
			//<ha>15세이상만 공성전 필드로 이동한다.
			if ( sBiInfo && (sBiInfo->EventPlay[0]&BIMASK_ADULT_USER)!=0 ) {
					WarpField2( 33 );
					sinMinusMoney(haC_TeleportMoney); //돈을뺀다
					sinPlaySound(SIN_SOUND_COIN);
					ReformCharForm(); //재인증 
			}
			else{
				cMessageBox.ShowMessage(MESSAGE_NOT_CASTLE_TELEPORT);

			}
			*/

			
		break;
		case MESSAGE_CASTLE_BUYTOWER:       //블레스 캐슬 크리스탈 타워를 산다.
			if((sinChar->Money - haTowerMoney) < 0){
					cMessageBox.ShowMessage(MESSAGE_NOT_ENOUGH_MONEY); //돈부족
					break;
			}
			for(int i=0;i<6;i++){
				if(cSinSiege.TowerTypeDraw[i][0]){
					cSinSiege.TowerTypeDraw[i][1] = haSendTowerIndex;
				}

			}

			cSinSiege.Price=haTowerMoney;
			sinMinusMoney(haTowerMoney);

			chaSiege.SendServerSiegeData(); //다시 저장해준다.
		//	ReformCharForm();//재인증
		//	SendSaveMoney();
			sinPlaySound(SIN_SOUND_COIN);


		break;
		//피곤한 창고지기
		case MESSAGE_QUEST_ElementaryA:
 			chaQuest.sHaQuestElementary[Quest_A].CODE  = HAQUEST_CODE_ELEMENTARY_A;
			chaQuest.sHaQuestElementary[Quest_A].State = 1;
			if(!cInvenTory.SearchItemCode(sinQT1|sin15)){
				SendQuestCommandToServer( HAQUEST_CODE_ELEMENTARY_A , 1, 0, 0 );
			}
			SaveGameData();
			StartQuest_Code(chaQuest.sHaQuestElementary[Quest_A].CODE);
		break;
		//두려운 마음
		case MESSAGE_QUEST_ElementaryB:
			chaQuest.sHaQuestElementary[Quest_B].CODE  = HAQUEST_CODE_ELEMENTARY_B;
			chaQuest.sHaQuestElementary[Quest_B].State = 1;
			chaQuest.haElementaryQuest_B();
			SaveGameData();
			StartQuest_Code(chaQuest.sHaQuestElementary[Quest_B].CODE);
		break;
		//사탕 배달
		case MESSAGE_QUEST_ElementaryC:
			chaQuest.sHaQuestElementary[Quest_C].CODE  = HAQUEST_CODE_ELEMENTARY_C;
			chaQuest.sHaQuestElementary[Quest_C].State = 1;
			if(!cInvenTory.SearchItemCode(sinQT1|sin14)){
				SendQuestCommandToServer( HAQUEST_CODE_ELEMENTARY_C ,1, 0, 0 );
			}
			SaveGameData();
			StartQuest_Code(chaQuest.sHaQuestElementary[Quest_C].CODE);		
		break;
		//고블린 동산
		case MESSAGE_QUEST_ElementaryD:
			chaQuest.sHaQuestElementary[Quest_D].CODE  = HAQUEST_CODE_ELEMENTARY_D;
			chaQuest.sHaQuestElementary[Quest_D].State = 1;
			chaQuest.haElementaryQuest_D();
			SaveGameData();
			StartQuest_Code(chaQuest.sHaQuestElementary[Quest_D].CODE);
		break;
		//언데드의 위협
		case MESSAGE_QUEST_ElementaryE:
			chaQuest.sHaQuestElementary[Quest_E].CODE  = HAQUEST_CODE_ELEMENTARY_E;
			chaQuest.sHaQuestElementary[Quest_E].State = 1;
			chaQuest.haElementaryQuest_E();
			SaveGameData();
			StartQuest_Code(chaQuest.sHaQuestElementary[Quest_E].CODE);
		break;
		//외눈박이 괴물
		case MESSAGE_QUEST_ElementaryF:
			chaQuest.sHaQuestElementary[Quest_F].CODE  = HAQUEST_CODE_ELEMENTARY_F;
			chaQuest.sHaQuestElementary[Quest_F].State = 1;
			chaQuest.haElementaryQuest_F();
			SaveGameData();
			StartQuest_Code(chaQuest.sHaQuestElementary[Quest_F].CODE);
		break;
		//잃어버린 망치
		case MESSAGE_QUEST_ElementaryG:
			chaQuest.sHaQuestElementary[Quest_G].CODE  = HAQUEST_CODE_ELEMENTARY_G;
			chaQuest.sHaQuestElementary[Quest_G].State = 1;
			SaveGameData();
			StartQuest_Code(chaQuest.sHaQuestElementary[Quest_G].CODE);
		break;
		case  MESSAGE_EVENT_ITEMOK:
			chaPremiumitem.UsePremiumItem(cInvenTory.iEventBuyItemKind);
		break;
		//100LV 궤스트 
		case MESSAGE_QUEST_100LV:
			chaQuest.sHaQuest100LV.CODE  = HAQUEST_CODE_FURYOFPHANTOM ;
			chaQuest.sHaQuest100LV.State = 1;
			StartQuest_Code(chaQuest.sHaQuest100LV.CODE);
			chaQuest.showFuryQuest();
			SaveGameData();
		break;
		case MESSAGE_QUEST_100LVITEMOK:
			//가상의 공간 체크용 임시 아이템
			sITEM TempItem;
			TempItem.w = ITEMSIZE*2;
			TempItem.h = ITEMSIZE*4;
			TempItem.sItemInfo.Weight = 60;
			TempItem.Flag = 1;

			if(sinChar->Weight[0]+TempItem.sItemInfo.Weight > sinChar->Weight[1]){
				cMessageBox.ShowMessage(MESSAGE_OVER_WEIGHT); //무게초과
				chaQuest.sHaQuest100LV.State = 12;
				chaQuest.iFuryQuestIndex[1] = 0;			
				break;
			}
			//아이템주기 
			if(cInvenTory.CheckSetEmptyArea(&TempItem)){
				SendQuestCommandToServer( HAQUEST_CODE_FURYOFPHANTOM ,2, chaQuest.iFuryQuestIndex[1], 0 );
				haElementaryQuestLog |= QUESTBIT_FURYOFPHANTOM; //코드를 셋팅해준다
				chaQuest.sHaQuest100LV.State = 100;
				chaQuest.sHaQuest100LV.BackUpCode = chaQuest.sHaQuest100LV.CODE;
				chaQuest.sHaQuest100LV.CODE =0;
				EndQuest_Code(chaQuest.sHaQuest100LV.BackUpCode);
				cMessageBox.ShowMessage(MESSAGE_100LVITEM_OK);
				SaveGameData();
			}
			else{
				cMessageBox.ShowMessage(MESSAGE_OVER_SPACE);
				chaQuest.sHaQuest100LV.State = 12;
				chaQuest.iFuryQuestIndex[1] = 0;
				break;						
			}
			chaQuest.iFuryQuestIndex[1] = 0;
			SaveGameData();
		break;
		case MESSAGE_QUEST_100LVBATTLEOK:   //대련장으로 이동 
			if(chaQuest.sHaQuest100LV.State == 3 || 
				chaQuest.sHaQuest100LV.State == 8 || 
				chaQuest.sHaQuest100LV.State == 10)
				chaQuest.sHaQuest100LV.State++;
			Start_QuestArena(HAQUEST_CODE_FURYOFPHANTOM , chaQuest.sHaQuest100LV.State/5+1, 0 );
			SetQuestBoard();
		break;
		case MESSAGE_TELEPORT_CORE:        //텔레포트 코어
			chaPremiumitem.Using_ItemKind = 0;
			chaPremiumitem.UseTeleportScrollItem(chaPremiumitem.TelePortCore.ItemIndex);
		break;
/*		
		case MESSAGE_STARDUST_SEND:			
			if(cInvenTory.SendStarDustItem()){		//빛나는 가루를 보내준다 //크리스마스
				SendWingQuestItem();
			}
		break;
*/
		case MESSAGE_FALLGAME:   //
			WarpField2( 39 );
		break;
		case MESSAGE_MANUFACTURE:		// pluto 제작 아이템 종류 메세지
			sinCraftItemStartFlag = 1;
			SetInterEffectManufacture();
			MixCancelButtonDelayFlag = 1; // pluto 여기다 쓸라고 외부 변수로 바꿨다 -_-
		break;
		case MESSAGE_TELEPORT_DUNGEON:		// pluto 추가던전으로 워프한다.
			cSinHelp.NPCTeleportFlag = 0;
			WarpField2( 40 );
			break;
	}
	sinMessageBoxShowFlag  = 0; //처리를 다해준다음에 박스를 닫아준다 
	sinInitPointFlag7 = 0;
	sMessageBox3[Kind].Flag   = 0; 
	ShowSkillUpInfo = 0; //스킬 정보를 없애준다 
	
	return TRUE;
}


//확인 을 눌렀을 경우나 엔터를 입력했을 경우 
int cMESSAGEBOX::CheckOkMessageBox(int Kind)
{
	int j=0;

	if(sMessageBox2[Kind].Count <= 0){ //버릴 아이템이 없으면  
		sinMessageBoxShowFlag  = 0;
		sMessageBox2[Kind].Flag   = 0; 
		cSinSod2.ClanMasterMessageBoxFlag = 0; //입력이 없는 상태에서 리턴될때 초기화해준다
		return FALSE;
	}

	memset(&MessageBoxItem,0,sizeof(sITEM)); //초기화 
	switch(Kind){
		case MESSAGE_BUY_ITEM:   //아이템 사기 
			if(!BuyItemServerFlag){		
				cShop.SendBuyItemToServer(pMessageItem,sMessageBox2[Kind].Count);
				pMessageItem = 0;
				cInvenTory.CheckWeight(); //무게보정 
				
			}

		break;
		case MESSAGE_SELL_ITEM:  //아이템 팔기 
			memcpy(&MessageBoxItem,pMessageItem,sizeof(sITEM)); //아이템 복사 
			if(sMessageBox2[Kind].Count >= pMessageItem->sItemInfo.PotionCount){
				cShop.SellItemToShop(&MessageBoxItem,MessageBoxItem.sItemInfo.PotionCount);
				pMessageItem->Flag=0; //플랙 초기화 
				sInven[pMessageItem->ItemPosition-1].ItemIndex = 0; //아이템 인덱스 초기화 
			}
			else{
				pMessageItem->sItemInfo.PotionCount -= sMessageBox2[Kind].Count;
				MessageBoxItem.sItemInfo.PotionCount = sMessageBox2[Kind].Count;
				cShop.SellItemToShop(&MessageBoxItem,MessageBoxItem.sItemInfo.PotionCount);
			}
			pMessageItem=0; //포인터 초기화 
			cInvenTory.CheckWeight(); //무게보정 

		break;
		case MESSAGE_THROW_ITEM: //아이템 버리기 
			if(cMixtureReset.OpenFlag)break;	// 석지용 - 믹스쳐 리셋 창 추가
			if(cWareHouse.OpenFlag)break;
			if(cTrade.OpenFlag)break;
			if(cAging.OpenFlag)break;
			if(cCraftItem.OpenFlag)break;
			if(sSinWarpGate.ShowFlag)break;
			if(!pMessageItem)break;
			// pluto 제련
			if( SmeltingItem.OpenFlag )
			{
				break;
			}
			// pluto 제작
			if( ManufactureItem.m_OpenFlag )
			{
				break;
			}

			memcpy(&MessageBoxItem,pMessageItem,sizeof(sITEM)); //아이템 복사 
			if(sMessageBox2[Kind].Count >= pMessageItem->sItemInfo.PotionCount){
				if(sinThrowItemToFeild(&MessageBoxItem)){
					sinPlaySound(MessageBoxItem.SoundIndex);
					pMessageItem->Flag=0; //플랙 초기화 
					sInven[pMessageItem->ItemPosition-1].ItemIndex = 0; //아이템 인덱스 초기화 
					sinSaveGame(MessageBoxItem.sItemInfo.CODE ,sMessageBox2[Kind].Count);
				}
			}
			else{
				pMessageItem->sItemInfo.PotionCount -= sMessageBox2[Kind].Count;
				MessageBoxItem.sItemInfo.PotionCount = sMessageBox2[Kind].Count;
			
				if(sinThrowItemToFeild(&MessageBoxItem)){
					sinPlaySound(MessageBoxItem.SoundIndex);
					sinSaveGame(MessageBoxItem.sItemInfo.CODE ,sMessageBox2[Kind].Count);
		
				}

			}
			pMessageItem=0; //포인터 초기화 
			cInvenTory.CheckWeight(); //무게보정 
			cInvenTory.ReFormPotionNum();	//물약 갯수 체크 
		break;
		case MESSAGE_THROW_MONEY: //돈버리기 
			if(cMixtureReset.OpenFlag)break;	// 석지용 - 믹스쳐 리셋 창 추가
			if(cWareHouse.OpenFlag)break;
			if(cTrade.OpenFlag)break;
			if(cAging.OpenFlag)break;
			if(cCraftItem.OpenFlag)break;
			if(sSinWarpGate.ShowFlag)break;
			// pluto 제련
			if( SmeltingItem.OpenFlag )
			{
				break;
			}
			// pluto 제작
			if( ManufactureItem.m_OpenFlag )
			{
				break;
			}
			memset(&MessageBoxItem,0,sizeof(sITEM));
			MessageBoxItem.sItemInfo.CODE = sinGG1|sin01;
			for(j=0 ; j < MAX_ITEM ; j++){
				if(sItem[j].CODE == MessageBoxItem.sItemInfo.CODE){ //코인이면 
				memcpy(&MessageBoxItem,&sItem[j],sizeof(sITEM));
				break;
				}
			}
			if(sMessageBox2[Kind].Count > sinChar->Money){
				SendSetHackUser2(4100,1); //해킹한 유저를 신고한다 
				sMessageBox2[Kind].Count = sinChar->Money;

			}
			MessageBoxItem.sItemInfo.Money = sMessageBox2[Kind].Count; //금액을 넘겨준다 
			wsprintf(MessageBoxItem.sItemInfo.ItemName,sinGold,MessageBoxItem.sItemInfo.Money);
			ReformItem( &MessageBoxItem.sItemInfo ); //인증 
			if(sinThrowItemToFeild(&MessageBoxItem)){
				sinPlaySound(MessageBoxItem.SoundIndex);
				CheckCharForm();//인증 
				//sinChar->Money -=MessageBoxItem.sItemInfo.Money;
				sinMinusMoney(MessageBoxItem.sItemInfo.Money);
				ReformCharForm();//재인증 	
				SendSaveMoney(); //금액 조작을 못하게하기위해 호출한다 
			}
		break;
		case  MESSAGE_MOVE_MONEY_TRADE://돈거래
			if(!CheckTrade((void *)&sTrade)) //잘못된 데이타 이면 거래창을 닫는다
				cTrade.CancelTradeItem();
			CheckCharForm();//인증 
			if(sinChar->Money - sMessageBox2[Kind].Count < 0){
				SendSetHackUser2(4100,2); //해킹한 유저를 신고한다 
				sMessageBox2[Kind].Count = sinChar->Money;

			}
			//sinChar->Money -= sMessageBox2[Kind].Count;
			sinMinusMoney(sMessageBox2[Kind].Count);
			ReformCharForm();//재인증 	
			SendSaveMoney(); //금액 조작을 못하게하기위해 호출한다 
			sTrade.Money = sMessageBox2[Kind].Count;
			TempShowMoney = sTrade.Money;
			sTrade.Money = sTrade.Money+193; 
			sinPlaySound(SIN_SOUND_COIN);
			ReformTrade((void *)&sTrade);
			sTrade.CheckFlag = 0; //돈을 옮기게되면 플랙을 초기화 
			SendTradeItem(cTrade.TradeCharCode);
			
		break;
		case MESSAGE_MOVE_MONEY_MOVE:
			if(sinChar->Money - sMessageBox2[Kind].Count < 0){
				SendSetHackUser2(4100,3); //해킹한 유저를 신고한다 
				sMessageBox2[Kind].Count = sinChar->Money;

			}

			CheckCharForm();//인증 
			//sinChar->Money -= sMessageBox2[Kind].Count;
			sinMinusMoney2(sMessageBox2[Kind].Count);
			ReformCharForm();//재인증 	
			//SendSaveMoney(); //금액 조작을 못하게하기위해 호출한다 
			sWareHouse.Money += sMessageBox2[Kind].Count;
			sinPlaySound(SIN_SOUND_COIN);
			WareHouseSaveFlag = 1;

		break;

		case MESSAGE_MOVE_MONEY_RECV:
			if((sWareHouse.Money-2023) - sMessageBox2[Kind].Count < 0){
				SendSetHackUser2(4100,4); //해킹한 유저를 신고한다 
				sMessageBox2[Kind].Count = sWareHouse.Money-2023;

			}

			CheckCharForm();//인증 
			//sinChar->Money += sMessageBox2[Kind].Count;
			sinPlusMoney2(sMessageBox2[Kind].Count);
			ReformCharForm();//재인증 	
			//SendSaveMoney(); //금액 조작을 못하게하기위해 호출한다 
			sWareHouse.Money -= sMessageBox2[Kind].Count;
			sinPlaySound(SIN_SOUND_COIN);
			WareHouseSaveFlag = 1;

		break;
		case MESSAGE_BUY_ITEM_DEFALT: //그냥 아이템 사기일경우 
			if(!BuyItemServerFlag){
				cShop.SendBuyItemToServer(pMessageItem,sMessageBox2[Kind].Count);
				cInvenTory.CheckWeight(); //무게보정 
				
			}

		break;
		case MESSAGE_BUY_STAR: //별을 산다
			SendStarPointToServer(sMessageBox2[Kind].Count*100000,1); //서버에 보낸다 
			CheckCharForm();//인증 
			sinMinusMoney2(sMessageBox2[Kind].Count*100000);
			sinPlaySound(SIN_SOUND_COIN);
			ReformCharForm();//재인증 	
			StarShopNPC();
		break;

		case MESSAGE_GIVE_MONEY:
			sinGiveMoneyCheckFlag = 1;
			sinGiveMoneyAmount = sMessageBox2[Kind].Count;
			wsprintf(szGiveMoneyBuff,"%d",sinGiveMoneyAmount);

			/*
			if(sinChar->Money - sMessageBox2[Kind].Count < 0){
				SendSetHackUser2(4100,5); //해킹한 유저를 신고한다 
				sMessageBox2[Kind].Count = sinChar->Money;

			}

			CheckCharForm();//인증 
			sinMinusMoney2(sMessageBox2[Kind].Count);
			ReformCharForm();//재인증 	
			//여기서 서버로 돈을 보낸다
//			SendGiveMoneyToServer(sMessageBox2[Kind].Count);
			sinPlaySound(SIN_SOUND_COIN);
			*/

		break;
		case MESSAGE_MYSHOP_ITEM: //개인상점에 아이템 등록 
			if(cInvenTory.CheckMoneyLimit(sMessageBox2[Kind].Count)){ 
				MyShopMouseItem.sItemInfo.Money = sMessageBox2[Kind].Count;
				cMyShop.LastSetMyShopItem(&MyShopMouseItem);
			}

			/*
			if(MyShopMouseItem.Class == ITEM_CLASS_POTION){
				if(cInvenTory.CheckMoneyLimit(sMessageBox2[Kind].Count*MyShopMouseItem.sItemInfo.PotionCount)){
					MyShopMouseItem.sItemInfo.Money = sMessageBox2[Kind].Count;
					cMyShop.LastSetMyShopItem(&MyShopMouseItem);
				}
			}
			else{
				if(cInvenTory.CheckMoneyLimit(sMessageBox2[Kind].Count)){ 
					MyShopMouseItem.sItemInfo.Money = sMessageBox2[Kind].Count;
					cMyShop.LastSetMyShopItem(&MyShopMouseItem);
				}
			}
			*/
		break;
		case MESSAGE_MYSHOP_POTION:
			if(cCharShop.OpenFlag){
				sMyShop_Server.sTime = sMessageBox2[Kind].Count;
				sMyShop_Server.Price *= sMessageBox2[Kind].Count; 
				Send_PersonalShopItem(sRecvMyShop.CHAR_CODE,&sMyShop_Server); //서버로 구매할 아이템의 정보를 보낸다
			}
		break;
		case MESSAGE_SOD2_GET_MONEY:
			if(sinMessageBox_New.Flag && cSinSod2.BoxIndex ==1 && sMessageBox2[Kind].Count){
				if(cSinSod2.UserType == 6){ 
					SendClanMoneyToServer(sMessageBox2[Kind].Count,0);
				}
			}
			cSinSod2.ClanMasterMessageBoxFlag = 0;
			//여기서 상금을 찾는다
			//cSinSod2.ClanMoney

		break;
		case MESSAGE_SIEGE_GET_MONEY: //<ha>공성전 현재 세금총액에서 돈을 찾는다. //세금회수 버튼누르고 숫자창 뜨고 거기서 숫자 누르고 확인 누르면 여기 들어옴
			if(haSiegeMenuFlag)		//해외
			{
				if( (cSinSiege.TotalTax >= sMessageBox2[Kind].Count) &&		//해외 세금
					(cSinSiege.ExpectedTotalTax >= sMessageBox2[Kind].Count))
				{
					cSinSiege.ExpectedTotalTax = cSinSiege.ExpectedTotalTax - sMessageBox2[Kind].Count; //해외 세금
					SendClanMoneyToServer(sMessageBox2[Kind].Count,0,1);
				}
			}
		break;
		case MESSAGE_SIEGE_SET_MERCENRAY:   //<ha>공성전 용병설정
			
			if(haSiegeMenuFlag){
				 //현재 용병 갯수에 데이타를 세팅한다.
				if(haMercenrayIndex){
					cSinSiege.MercenaryNum[haMercenrayIndex-1]+=sMessageBox2[Kind].Count; 
					cSinSiege.Price=haMercenrayMoney[haMercenrayIndex-1]*sMessageBox2[Kind].Count;
					chaSiege.SendServerSiegeData(); //저장한다.
					cSinSiege.Price = 0;
				}
			}
			//용병가격을 빼준다.
			sinMinusMoney(haMercenrayMoney[haMercenrayIndex-1]*sMessageBox2[Kind].Count);
			sinPlaySound(SIN_SOUND_COIN);
		break;

/*

		case MESSAGE_EVENT_SMASHTV: //이벤트 스메쉬 TV 
			///////////////// 어디로 보낼지는 정해지지 않았슴
			if(sinChar->Money - sMessageBox2[Kind].Count < 0 || sMessageBox2[Kind].Count > 100000){
				SendSetHackUser2(4100,5); //해킹한 유저를 신고한다 
				sMessageBox2[Kind].Count = sinChar->Money;

			}
			if(sMessageBox2[Kind].Count < 100000){
				CloseMessage(); //메세지박스를닫아준다 
				ShowMessage(MESSAGE_NOT_ENOUGH_MONEY);
				return FALSE;

			}
			CheckCharForm();//인증 
			//sinChar->Money -= sMessageBox2[Kind].Count;
			sinMinusMoney(sMessageBox2[Kind].Count);
			ReformCharForm();//재인증 	
			SendCollectMoney(sMessageBox2[Kind].Count);
			sinPlaySound(SIN_SOUND_COIN);
			//ShowMessage(MESSAGE_GIVE_MONEY_TRUE);
			//요기서 이벤트에 입장하면 되겠심다 두둥!
			
		break;

*/

	}
	sinMessageBoxShowFlag  = 0; //처리를 다해준다음에 박스를 닫아준다 
	sMessageBox2[Kind].Flag   = 0; 
	memset(&MessageBoxItem,0,sizeof(sITEM)); //초기화 
	cInvenTory.ReFormPotionNum();	//물약 갯수 보정 
	return TRUE;
}

//별아템 npc
int StarShopNPC()
{
	//상품권이 있을경우 별로 바꾼다
	SendStarPointToServer(0,0);//일단 결과를 받는다 
	sITEM *pItem=0;
	GiftPrice = 0;
	sStarShop.Flag = 1;
	for(int i = 0; i < INVENTORY_MAXITEM*2 ; i++){
		if(i<100){
			if(!cInvenTory.InvenItem[i].Flag)continue;
			pItem = &cInvenTory.InvenItem[i];
		}
		else{
			if(!cInvenTory.InvenItemTemp[i-100].Flag)continue;
			pItem = &cInvenTory.InvenItemTemp[i-100];
		}
		if(pItem->sItemInfo.CODE ==(sinGF1|sin01)){
			GiftPrice += pItem->sItemInfo.Price;
		}
	}	
	if(GiftPrice){
		cMessageBox.ShowMessage3(MESSAGE_GIFT_TICKET,"\273\363\307\260\261\307\300\270\267\316");
	}
	return TRUE;
}

//별아이템 포인트를 받는다
int sinStarPoint(int Point)
{
	sStarShop.Count = Point;
	return TRUE;
}

//상품권 아이템을 지운다
int DeleteGiftItem()
{
	sITEM *pItem=0;
	for(int i = 0; i < INVENTORY_MAXITEM*2 ; i++){
		if(i<100){
			if(!cInvenTory.InvenItem[i].Flag)continue;
			pItem = &cInvenTory.InvenItem[i];
		}
		else{
			if(!cInvenTory.InvenItemTemp[i-100].Flag)continue;
			pItem = &cInvenTory.InvenItemTemp[i-100];
		}
		if(pItem->sItemInfo.CODE ==(sinGF1|sin01)){
			pItem->Flag =0;
			
		}
	}	
	cInvenTory.ReFormInvenItem(); //아이템을 지우고 리폼해준다
	ReformCharForm();//재인증 
	return TRUE;

}