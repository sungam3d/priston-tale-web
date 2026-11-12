/*----------------------------------------------------------------------------*
*	파일명 :  sinShop.cpp	
*	하는일 :  상점을 관리한다 
*	작성일 :  최종업데이트 12월
*	적성자 :  박상열 
*-----------------------------------------------------------------------------*/	

#include "sinLinkHeader.h"
#include "..\\tjboy\\clanmenu\\tjclan.h"
#include "..\\tjboy\\clanmenu\\clan_Enti.h"
#include "..\\CurseFilter.h"		//IsCurse 함수(해외)

/*----------------------------------------------------------------------------*
*								전역변수 			
*-----------------------------------------------------------------------------*/	
cSHOP		cShop;
cMYSHOP		cMyShop;
cCHARSHOP	cCharShop;
sMYSHOP     sMyShop;       //보내는 MyShop 구조체
sMYSHOP     sRecvMyShop;   //받는 MyShop 구조체
sMYSHOP_ITEM_SERVER sMyShop_Server;

sITEMINFO   MyShopPotion;

int CloseMyShopFlag = 0;
int CloseCharShopFlag = 0;
int SelectShopButtonPosi = 0;  //상점의 버튼 위치값 
int MyShopItemIndex[100] ={0,};


int sinShopKind = 1;  //현재 상점의 종류  1  무기 2 방어 3 물약 (defalt 는 상점)

int ShopArrowPosi = 0;

RECT ShopItemRect={0,0,0,0};

int	GridRow , GridCol ,StartGirdX , StartGirdY; //그리드 

int ShopItemPrice=0;//물건가격 
int ShowShopItemIndex=0; //아이템의 인덱스를 저장한다 

int TalkNpcState = 0; //현재 상점 NPC와 함께 있을경우 

int haCharShopDelayFlag=0;

//좌표보정으로 하나더만듬
int ReStartButtonRect2[3][4] = {
	{21,22,21+44,22+44},			//필드에서 시작 
	{72,22,72+44,22+44},			//마을에서 시작 
	{123,22,123+44,22+44},			//게임 종료 
	
};

int ShopButtonPosi[9][4] = {
	{269,278+sinInterHeight2,269+28,278+28+sinInterHeight2}, //다고치기
	{238,278+sinInterHeight2,238+28,278+28+sinInterHeight2}, //고치기 
	{238,309+sinInterHeight2,238+28,309+28+sinInterHeight2}, //팔기  
	{269,309+sinInterHeight2,269+28,309+28+sinInterHeight2}, //사기 
	{279,182+sinInterHeight2,279+20,182+20+sinInterHeight2}, //오른쪽 화살표 
	{257,182+sinInterHeight2,257+20,182+20+sinInterHeight2}, //왼쪽 화살표 
	{304,314+sinInterHeight2,304+20,314+20+sinInterHeight2}, //나가기 
	{231,136+sinInterHeight2,231+42,136+46+sinInterHeight2}, //무기탭
	{281,136+sinInterHeight2,281+42,136+46+sinInterHeight2}, //방어탭

};

//금액의 위치 
int ShopGoldEdit[2][4] = {
	{231,213+5+sinInterHeight2,231+120,213+5+24+sinInterHeight2}, //물건 가격 
	{231,243+5+sinInterHeight2,231+120,243+5+24+sinInterHeight2}, //내돈 

};

//////////상점에 보여줄수있는아이템의 한계치 
int ShowShopItemIndex2 = 0;

int BuyItemServerFlag = 0;
int BuyItemIndexCopy = 0; //상점에서 없어질 아이템의 인덱스 

int SelectMyShopButtonPosi = 0;
int SelectMyShopItemIndex = 0;

int MyShopSendButton   = 0;
int MyShopExpBox	   = 0;
int MyShopExpButtonOK  = 0;
int MyShopExpButtonCancel  = 0;

int SelectCharShopButtonPosi = 0;

int MyShopExpBoxX = 250;
int MyShopExpBoxY = 100;

int MyShopItemCancelIndex[2] = {0,0};


/*----------------------------------------------------------------------------*
*							클래스 초기, 종료 
*-----------------------------------------------------------------------------*/	
cSHOP::cSHOP()
{

}
cSHOP::~cSHOP()
{

}
/*----------------------------------------------------------------------------*
*							     초기화 
*-----------------------------------------------------------------------------*/	
void cSHOP::Init()
{
	//MatShop[0] =  CreateTextureMaterial( "Image\\SinImage\\Shopall\\Shop-1.tga" , 0, 0, 0,0, SMMAT_BLEND_ALPHA );
	//MatShop[1] =  CreateTextureMaterial( "Image\\SinImage\\Shopall\\Trade-2.tga" , 0, 0, 0,0, SMMAT_BLEND_ALPHA );
	//MatShop[2] =  CreateTextureMaterial( "Image\\SinImage\\Shopall\\Warehouse-2.tga" , 0, 0, 0,0, SMMAT_BLEND_ALPHA );

	Load();

	MatMyShop_Button = CreateTextureMaterial( "Image\\SinImage\\Shopall\\MyShopButton.tga" , 0, 0, 0,0, SMMAT_BLEND_ALPHA );

}
/*----------------------------------------------------------------------------*
*						    Bmp파일을 로드한다 
*-----------------------------------------------------------------------------*/	
void cSHOP::Load()
{

	lpShopMain = LoadDibSurfaceOffscreen("Image\\SinImage\\Shopall\\shop-1.bmp");
	lpShopMain2 = LoadDibSurfaceOffscreen("Image\\SinImage\\Shopall\\shop-2.bmp");

	lpGoldEdit = LoadDibSurfaceOffscreen("Image\\SinImage\\Shopall\\my_money.bmp");

	lpAllRepair = LoadDibSurfaceOffscreen("Image\\SinImage\\Shopall\\AllRepair.bmp");
	lpRepair    = LoadDibSurfaceOffscreen("Image\\SinImage\\Shopall\\Repair.bmp");

	lpSell      = LoadDibSurfaceOffscreen("Image\\SinImage\\Shopall\\Sell.bmp");
	lpBuy       = LoadDibSurfaceOffscreen("Image\\SinImage\\Shopall\\Buy.bmp") ;

	lpAllRepair_D = LoadDibSurfaceOffscreen("Image\\SinImage\\Shopall\\DefaultAllRepair.bmp");
	lpRepair_D    = LoadDibSurfaceOffscreen("Image\\SinImage\\Shopall\\DefaultRepair.bmp");

	lpSell_D      = LoadDibSurfaceOffscreen("Image\\SinImage\\Shopall\\Sell0.bmp");
	lpBuy_D       = LoadDibSurfaceOffscreen("Image\\SinImage\\Shopall\\Buy0.bmp") ;

	lpRightArrow = LoadDibSurfaceOffscreen("Image\\SinImage\\Shopall\\RightArrow.bmp");
	lpLeftArrow  = LoadDibSurfaceOffscreen("Image\\SinImage\\Shopall\\LeftArrow.bmp");

	lpTabAttack  = LoadDibSurfaceOffscreen("Image\\SinImage\\Shopall\\TabAttack.bmp");
	lpTabDefense = LoadDibSurfaceOffscreen("Image\\SinImage\\Shopall\\TabDefense.bmp");
	lpTabEtc     = LoadDibSurfaceOffscreen("Image\\SinImage\\Shopall\\TabEtc.bmp");

	lpTabAttack_D  = LoadDibSurfaceOffscreen("Image\\SinImage\\Shopall\\TabAttack0.bmp");
	lpTabDefense_D = LoadDibSurfaceOffscreen("Image\\SinImage\\Shopall\\TabDefense0.bmp");
	lpTabEtc_D     = LoadDibSurfaceOffscreen("Image\\SinImage\\Shopall\\TabEtc.bmp");

	lpExit       = LoadDibSurfaceOffscreen("Image\\SinImage\\Shopall\\ButtonExit.bmp");
	lpGrid       = LoadDibSurfaceOffscreen("Image\\SinImage\\Shopall\\Grid.bmp");

	lpRepairAllInfo	= LoadDibSurfaceOffscreen("Image\\SinImage\\Shopall\\ButtonInfo\\RepairAllInfo.bmp");	
	lpRepairInfo	= LoadDibSurfaceOffscreen("Image\\SinImage\\Shopall\\ButtonInfo\\RepairInfo.bmp");
	lpSellInfo		= LoadDibSurfaceOffscreen("Image\\SinImage\\Shopall\\ButtonInfo\\SellInfo.bmp");
	lpBuyInfo		= LoadDibSurfaceOffscreen("Image\\SinImage\\Shopall\\ButtonInfo\\BuyInfo.bmp");

	lpDefaultRepairAll = LoadDibSurfaceOffscreen("Image\\SinImage\\Shopall\\DefaultAllRepair.bmp");	
	lpDefaultRepair    = LoadDibSurfaceOffscreen("Image\\SinImage\\Shopall\\DefaultRepair.bmp");

	lpTitle_Attack = LoadDibSurfaceOffscreen("Image\\SinImage\\Shopall\\title-shop.bmp");
	lpTitle_Etc = LoadDibSurfaceOffscreen("Image\\SinImage\\Shopall\\title-TabEtc.bmp");
	lpTitle_Trade = LoadDibSurfaceOffscreen("Image\\SinImage\\Shopall\\Trade.bmp");
	lpTitle_WareHouse = LoadDibSurfaceOffscreen("Image\\SinImage\\Shopall\\Warehouse.bmp");

	lpWeightEdit = LoadDibSurfaceOffscreen("Image\\SinImage\\Shopall\\weight.bmp");

	lpNoUseBox = LoadDibSurfaceOffscreen("Image\\SinImage\\Shopall\\NoUseBox.bmp");

	lpCraftItemMain			= LoadDibSurfaceOffscreen("Image\\SinImage\\Shopall\\CraftItem\\CraftItemMain.bmp");
	lpCraftItemButton		= LoadDibSurfaceOffscreen("Image\\SinImage\\Shopall\\CraftItem\\Button.bmp");
	lpCraftItemButtonInfo	= LoadDibSurfaceOffscreen("Image\\SinImage\\Shopall\\CraftItem\\ButtonInfo.bmp");
	lpTitle_CraftItem		= LoadDibSurfaceOffscreen("Image\\SinImage\\Shopall\\CraftItem\\Title.bmp");
	lpTitle_Aging			= LoadDibSurfaceOffscreen("Image\\SinImage\\Shopall\\Aging\\Aging_Title.bmp");
	lpAging_Info			= LoadDibSurfaceOffscreen("Image\\SinImage\\Shopall\\Aging\\ButtonInfo.bmp");

	// pluto 제련
	lpSmeltingItemMain		= LoadDibSurfaceOffscreen( "Image\\SinImage\\Shopall\\SmeltingItem\\SmeltingMain.bmp" );
	lpTitle_SmeltingItem	= LoadDibSurfaceOffscreen( "Image\\SinImage\\Shopall\\SmeltingItem\\SmeltingTitle.bmp");
	lpSmeltingItemButtonInfo = LoadDibSurfaceOffscreen( "Image\\SinImage\\Shopall\\SmeltingItem\\ButtonInfo.bmp");

	// pluto 제작
	lpManufactureItemMain	= LoadDibSurfaceOffscreen("Image\\SinImage\\Shopall\\Manufacture\\ManufactureMain.bmp" );
	lpTitle_ManufactureItem = LoadDibSurfaceOffscreen( "Image\\SinImage\\Shopall\\Manufacture\\ManufactureTitle.bmp" );
	lpManufactureItemButtonInfo = LoadDibSurfaceOffscreen( "Image\\SinImage\\Shopall\\Manufacture\\ButtonInfo.bmp" );

	// 석지용 - 믹스쳐 리셋을 위한 이미지 로딩 ( 위의 lpCraftItemButton(버튼) 변수 필요할듯 )
	lpMResetTitle			= LoadDibSurfaceOffscreen("Image\\SinImage\\Shopall\\CraftItem\\MResetTitle.bmp");
	lpMResetMain			= LoadDibSurfaceOffscreen("Image\\SinImage\\Shopall\\CraftItem\\MResetMain.bmp");
	lpMResetButton			= LoadDibSurfaceOffscreen("Image\\SinImage\\Shopall\\CraftItem\\MResetButton.bmp");
	lpMResetButtonInfo		= LoadDibSurfaceOffscreen("Image\\SinImage\\Shopall\\CraftItem\\MResetButtonInfo.bmp");

	lpMyShopExp				= LoadDibSurfaceOffscreen("Image\\SinImage\\indishop\\indishop_exp.bmp");
	lpMyShopExp_			= LoadDibSurfaceOffscreen("Image\\SinImage\\indishop\\indishop_exp_.bmp");
	lpMyShopExp_T			= LoadDibSurfaceOffscreen("Image\\SinImage\\indishop\\indishop_expT.bmp");
	lpMyShopNoSale			= LoadDibSurfaceOffscreen("Image\\SinImage\\indishop\\indishop_nosale.bmp");
	lpMyShopNoSale_			= LoadDibSurfaceOffscreen("Image\\SinImage\\indishop\\indishop_nosale_.bmp");
	lpMyShopNoSale_T		= LoadDibSurfaceOffscreen("Image\\SinImage\\indishop\\indishop_nosaleT.bmp");
	lpMyShopSale			= LoadDibSurfaceOffscreen("Image\\SinImage\\indishop\\indishop_sale.bmp");
	lpMyShopSale_			= LoadDibSurfaceOffscreen("Image\\SinImage\\indishop\\indishop_sale_.bmp");
	lpMyShopSale_T			= LoadDibSurfaceOffscreen("Image\\SinImage\\indishop\\indishop_saleT.bmp");
	lpTitle_MyShop			= LoadDibSurfaceOffscreen("Image\\SinImage\\indishop\\indishop_title.bmp");
	lpImage_MyShop			= LoadDibSurfaceOffscreen("Image\\SinImage\\indishop\\indishop_image.bmp");
	lpMyShop_T				= LoadDibSurfaceOffscreen("Image\\SinImage\\indishop\\indishop_T.bmp");

}
/*----------------------------------------------------------------------------*
*						     Release
*-----------------------------------------------------------------------------*/	
void cSHOP::Release()
{
	if(lpAllRepair){
		lpAllRepair->Release();
		lpAllRepair = 0;
	}
	if(lpRepair){
		lpRepair->Release();
		lpRepair = 0;
	}
	if(lpSell){
		lpSell->Release();
		lpSell = 0;
	}
	if(lpBuy){
		lpBuy->Release();
		lpBuy = 0;
	}
	if(lpAllRepair_D){
		lpAllRepair_D->Release();
		lpAllRepair_D = 0;
	}
	if(lpRepair_D){
		lpRepair_D->Release();
		lpRepair_D = 0;
	}
	if(lpSell_D){
		lpSell_D->Release();
		lpSell_D = 0;
	}
	if(lpBuy_D){
		lpBuy_D->Release();
		lpBuy_D = 0;
	}

	if(lpRightArrow){
		lpRightArrow->Release();
		lpRightArrow = 0;
	}
	if(lpLeftArrow){
		lpLeftArrow->Release();
		lpLeftArrow = 0;
	}
	if(lpTabAttack){
		lpTabAttack->Release();
		lpTabAttack = 0;
	}
	if(lpTabDefense){
		lpTabDefense->Release();
		lpTabDefense = 0;
	}
	if(lpTabEtc){
		lpTabEtc->Release();
		lpTabEtc = 0;
	}
	if(lpTabAttack_D){
		lpTabAttack_D->Release();
		lpTabAttack_D = 0;
	}
	if(lpTabDefense_D){
		lpTabDefense_D->Release();
		lpTabDefense_D = 0;
	}
	if(lpTabEtc_D){
		lpTabEtc_D->Release();
		lpTabEtc_D = 0;
	}

	if(lpExit){
		lpExit->Release();
		lpExit = 0;
	}
	if(lpGrid){
		lpGrid->Release();
		lpGrid = 0;
	}
	if(lpRepairAllInfo){
		lpRepairAllInfo->Release();
		lpRepairAllInfo = 0;
	}
	if(lpRepairInfo){
		lpRepairInfo->Release();
		lpRepairInfo = 0;
	}
	if(lpSellInfo){
		lpSellInfo->Release();
		lpSellInfo = 0;
	}
	if(lpBuyInfo){
		lpBuyInfo->Release();
		lpBuyInfo = 0;
	}
	if(lpDefaultRepairAll){
		lpDefaultRepairAll->Release();
		lpDefaultRepairAll = 0;
	}
	if(lpDefaultRepair){
		lpDefaultRepair->Release();
		lpDefaultRepair = 0;
	}
	if(lpTitle_Attack){
		lpTitle_Attack->Release();
		lpTitle_Attack = 0;
	}
	if(lpTitle_Etc){
		lpTitle_Etc->Release();
		lpTitle_Etc = 0;
	}
	if(lpTitle_Trade){
		lpTitle_Trade->Release();
		lpTitle_Trade = 0;
	}
	if(lpTitle_WareHouse){
		lpTitle_WareHouse->Release();
		lpTitle_WareHouse = 0;
	}
	if(lpCraftItemMain){
		lpCraftItemMain->Release();
		lpCraftItemMain = 0;
	}

	if(lpCraftItemButton){
		lpCraftItemButton->Release();
		lpCraftItemButton = 0;
	}
	if(lpCraftItemButtonInfo){
		lpCraftItemButtonInfo->Release();
		lpCraftItemButtonInfo = 0;
	}
	if(lpTitle_CraftItem){
		lpTitle_CraftItem->Release();
		lpTitle_CraftItem = 0;
	}

	// pluto 제련
	if( lpSmeltingItemMain )
	{
		lpSmeltingItemMain->Release();
		lpSmeltingItemMain = 0;
	}

	if( lpTitle_SmeltingItem )
	{
		lpTitle_SmeltingItem->Release();
		lpTitle_SmeltingItem = 0;
	}

	if( lpSmeltingItemButtonInfo )
	{
		lpSmeltingItemButtonInfo->Release();
		lpSmeltingItemButtonInfo = 0;
	}

	// pluto 제작
	if( lpManufactureItemMain )
	{
		lpManufactureItemMain->Release();
		lpManufactureItemMain = 0;
	}
	if( lpTitle_ManufactureItem )
	{
		lpTitle_ManufactureItem->Release();
		lpTitle_ManufactureItem = 0;
	}
	if( lpManufactureItemButtonInfo)
	{
		lpManufactureItemButtonInfo->Release();
		lpManufactureItemButtonInfo = 0;
	}

	// 석지용 - 믹스쳐 리셋 해제
	if( lpMResetTitle )
	{
		lpMResetTitle->Release();
		lpMResetTitle = 0;
	}
	if( lpMResetMain )
	{
		lpMResetMain->Release();
		lpMResetMain = 0;
	}
	if( lpMResetButton )
	{
		lpMResetButton->Release();
		lpMResetButton = 0;
	}
	if( lpMResetButtonInfo )
	{
		lpMResetButtonInfo->Release();
		lpMResetButtonInfo = 0;
	}
}
/*----------------------------------------------------------------------------*
*							     그리기  
*-----------------------------------------------------------------------------*/	
void cSHOP::Draw()
{


	////////////////전사 (필드, 마을, 종료)
	if(ReStartFlag){
		DrawSprite(ReStartMainXY.x,ReStartMainXY.y,cInterFace.lpReStartMain,0,0,188,128); //다시시작 메인 
		if(ReStartIndex && RestartCheckExp){
			if(ReStartIndex != 1)
				DrawSprite(ReStartMainXY.x+ReStartButtonRect2[ReStartIndex-1][0],ReStartMainXY.y+ReStartButtonRect2[ReStartIndex-1][1],cInterFace.lpReStartButton[ReStartIndex-1],0,0,44,44); //다시시작 메인 
		}
		if(ReStartIndex== 1 && !RestartCheckExp ){
			DrawSprite(ReStartMainXY.x+ReStartButtonRect2[ReStartIndex-1][0],ReStartMainXY.y+ReStartButtonRect2[ReStartIndex-1][1],cInterFace.lpReStartButton[ReStartIndex-1],0,0,44,44); //다시시작 메인 
		}
	}

	int i=0,j=0;

	//개인상점 등록
	DrawSprite(0-(256+128-sinMoveKindInter[SIN_MYSHOP]),90+sinInterHeight2,cShop.lpShopMain2,0,0,344,264);	       
 
	//캐릭터 상점  
	DrawSprite(0-(256+128-sinMoveKindInter[SIN_CHARSHOP]),90+sinInterHeight2,cShop.lpShopMain2,0,0,344,264);	       
	
	DrawSprite(0-(256+128-sinMoveKindInter[SIN_SHOP]),90+sinInterHeight2,lpShopMain,0,0,344,264);	       
	//dsDrawTexImage( MatShop[0] , 0 - (256+128-sinMoveKindInter[SIN_SHOP]),  96  , 256, 256 , 255 );       
	//dsDrawTexImage( MatShop[2] , 256-(256+128-sinMoveKindInter[SIN_SHOP])-25, 96 ,128 , 256 , 255 );   

 	if(sinShopKind == SIN_WEAPON || sinShopKind == SIN_DEFENSE){    

		DrawSprite(239-(256+128-sinMoveKindInter[SIN_SHOP]),87,lpTabAttack_D,0,0,41,45);	
		DrawSprite(280-(256+128-sinMoveKindInter[SIN_SHOP]),87,lpTabDefense_D,0,0,41,45);	
		DrawSprite(ShopButtonPosi[SINALLREPAIR-1][0]-(256+128-sinMoveKindInter[SIN_SHOP]),ShopButtonPosi[SINALLREPAIR-1][1],lpAllRepair_D,0,0,28,28); 
		DrawSprite(ShopButtonPosi[SINREPAIR-1][0]-(256+128-sinMoveKindInter[SIN_SHOP]),ShopButtonPosi[SINREPAIR-1][1],lpRepair_D,0,0,28,28);
		
		DrawSprite(225-(256+128-sinMoveKindInter[SIN_SHOP]),47,lpTitle_Attack,0,0,111,32);	

		if(sinShopKind == SIN_WEAPON)
			DrawSprite(239-(256+128-sinMoveKindInter[SIN_SHOP]),87,lpTabAttack,0,0,41,45);	
		if(sinShopKind == SIN_DEFENSE)
			DrawSprite(280-(256+128-sinMoveKindInter[SIN_SHOP]),87,lpTabDefense,0,0,41,45);	


		//if(SelectShopButtonPosi == 8)
		//	DrawSprite(231-(256+128-sinMoveKindInter[SIN_SHOP]),136,lpTabAttack,0,0,42,46);	

		//if(SelectShopButtonPosi == 9)
		//	DrawSprite(281-(256+128-sinMoveKindInter[SIN_SHOP]),136,lpTabDefense,0,0,42,46);	
			

	}

	if(sinShopKind == SIN_ETC){ 
		DrawSprite(232-(256+128-sinMoveKindInter[SIN_SHOP]),93,lpTabEtc_D,0,0,98,40);	
		DrawSprite(225-(256+128-sinMoveKindInter[SIN_SHOP]),47,lpTitle_Etc,0,0,111,32);	

	}
	DrawSprite(ShopButtonPosi[SINSELL-1][0]-(256+128-sinMoveKindInter[SIN_SHOP]),ShopButtonPosi[SINSELL-1][1],lpSell_D,0,0,28,28);		
	DrawSprite(ShopButtonPosi[SINBUY-1][0]-(256+128-sinMoveKindInter[SIN_SHOP]),ShopButtonPosi[SINBUY-1][1],lpBuy_D ,0,0,28,28);		

	//DrawSprite(231-(256+128-sinMoveKindInter[SIN_SHOP]),213,lpGoldEdit,0,0,99,24); //돈이 놓일 위치 
	DrawSprite(231-(256+128-sinMoveKindInter[SIN_SHOP]),243+sinInterHeight2,lpGoldEdit,0,0,99,24);

	DrawSprite(282-(256+128-sinMoveKindInter[SIN_SHOP]),182+sinInterHeight2,lpRightArrow,0,0,20,20); //화살표 
	DrawSprite(260-(256+128-sinMoveKindInter[SIN_SHOP]),182+sinInterHeight2,lpLeftArrow,0,0,20,20);

	//상점 화살표 
	if(ShopArrowPosi == 0 || ShopArrowPosi == 3)   
		DrawSprite(260-(256+128-sinMoveKindInter[SIN_SHOP]),132,cInvenTory.lpInvenArrow[0],0,0,20,20);				
	if(ShopArrowPosi == 1 || ShopArrowPosi == 4)
		DrawSprite(282-(256+128-sinMoveKindInter[SIN_SHOP]),132,cInvenTory.lpInvenArrow[1],0,0,20,20);		


	//개인상점 
	DrawSprite(ShopButtonPosi[SINALLREPAIR-1][0]-(256+128-sinMoveKindInter[SIN_MYSHOP]),ShopButtonPosi[SINALLREPAIR-1][1],lpMyShopNoSale_,0,0,28,28);   
	DrawSprite(ShopButtonPosi[SINREPAIR-1][0]-(256+128-sinMoveKindInter[SIN_MYSHOP]),ShopButtonPosi[SINREPAIR-1][1],lpMyShopSale_,0,0,28,28);
	//DrawSprite(ShopButtonPosi[SINBUY-1][0]-(256+128-sinMoveKindInter[SIN_MYSHOP]),ShopButtonPosi[SINBUY-1][1],lpMyShopExp_,0,0,28,28);		
	DrawSprite(238-(256+128-sinMoveKindInter[SIN_MYSHOP]),90,lpImage_MyShop,0,0,53,59);	
	DrawSprite(225-(256+128-sinMoveKindInter[SIN_MYSHOP]),47,lpTitle_MyShop,0,0,111,32);	


	//캐릭터 상점
	DrawSprite(ShopButtonPosi[SINBUY-1][0]-(256+128-sinMoveKindInter[SIN_CHARSHOP]),ShopButtonPosi[SINBUY-1][1],lpBuy_D,0,0,28,28);		
	DrawSprite(238-(256+128-sinMoveKindInter[SIN_CHARSHOP]),90,lpImage_MyShop,0,0,53,59);	
	DrawSprite(225-(256+128-sinMoveKindInter[SIN_CHARSHOP]),47,lpTitle_MyShop,0,0,111,32);	
 
	

	//겹치는게 아름답지않아 땜빵을 살짝해본다 푸핫 -_-
	HDC	hdc;
	char strBuff[128];
	memset(strBuff,0,sizeof(strBuff));
	lpDDSBack->GetDC( &hdc );
	SelectObject( hdc, sinFont);
    SetBkMode( hdc, TRANSPARENT );
	SetTextColor( hdc, RGB(255,255,255) );

	int Money2 =0;

	int len3 = 0;

	if(cMyShop.OpenFlag){  
		for(int i = 0 ; i < 30 ; i++){
			if(cMyShop.MyShopItem[i].Flag){      
				if(cMyShop.MyShopItem[i].Class == ITEM_CLASS_POTION){
					Money2 += cMyShop.MyShopItem[i].sItemInfo.Money * cMyShop.MyShopItem[i].sItemInfo.PotionCount;

				}
				else{
					Money2 += cMyShop.MyShopItem[i].sItemInfo.Money;

				}
				
			}
		}

		NumLineComa(cMyShop.GetLimitMoney(Money2) , strBuff);
		len3 = lstrlen(strBuff);
		//wsprintf(strBuff,"%d",cMyShop.GetLimitMoney(Money2)); //소지가능 금액  
		dsTextLineOut(hdc,CheckEditSize(ShopGoldEdit[1][0],ShopGoldEdit[1][2], strBuff)-(256+128-sinMoveKindInter[SIN_MYSHOP])-(len3/2),ShopGoldEdit[1][1],
					strBuff,lstrlen(strBuff));

		memset(strBuff,0,sizeof(strBuff));
		NumLineComa(Money2 , strBuff);
		len3 = lstrlen(strBuff);
		//wsprintf(strBuff,"%d",Money2 ); //판매 총액 
		dsTextLineOut(hdc,CheckEditSize(ShopGoldEdit[0][0],ShopGoldEdit[0][2], strBuff)-(256+128-sinMoveKindInter[SIN_MYSHOP])-(len3/2),ShopGoldEdit[0][1],
					strBuff,lstrlen(strBuff));

	}
	if(cCharShop.OpenFlag){
		memset(strBuff,0,sizeof(strBuff));
		NumLineComa(sinChar->Money , strBuff);
		len3 = lstrlen(strBuff);
		//wsprintf(strBuff,"%d",sinChar->Money); //소지가능 금액  
		dsTextLineOut(hdc,CheckEditSize(ShopGoldEdit[1][0],ShopGoldEdit[1][2], strBuff)-(256+128-sinMoveKindInter[SIN_MYSHOP])-(len3/2),ShopGoldEdit[1][1],
					strBuff,lstrlen(strBuff));

	}
	lpDDSBack->ReleaseDC( hdc );


	switch(SelectCharShopButtonPosi){    
		case 4:
			DrawSprite(ShopButtonPosi[SINBUY-1][0]-(256+128-sinMoveKindInter[SIN_CHARSHOP]),ShopButtonPosi[SINBUY-1][1],lpBuy ,0,0,28,28);		
			DrawSprite(ShopButtonPosi[SINBUY-1][0]-26-(256+128-sinMoveKindInter[SIN_CHARSHOP]),ShopButtonPosi[SINBUY-1][1]-27,lpBuyInfo,0,0,77,27);
		break;
		case SINSHOPEXIT:
			DrawSprite(304-(256+128-sinMoveKindInter[SIN_CHARSHOP]),313+sinInterHeight2,lpExit,0,0,20,20);	
			DrawSprite(314-10-(256+128-sinMoveKindInter[SIN_CHARSHOP]),313-27+sinInterHeight2,cInvenTory.lpExitInfo,0 ,0 , 47,27);
		break;

	}


	//등록이 되면 아이콘을 활성화 시켜놓는다
	if(MyShopSendButton){
		DrawSprite(ShopButtonPosi[SINREPAIR-1][0]-(256+128-sinMoveKindInter[SIN_MYSHOP]),ShopButtonPosi[SINREPAIR-1][1],lpMyShopSale,0,0,28,28);			
	}

	switch(SelectMyShopButtonPosi){   
		case 1:  
			DrawSprite(ShopButtonPosi[SINALLREPAIR-1][0]-(256+128-sinMoveKindInter[SIN_MYSHOP]),ShopButtonPosi[SINALLREPAIR-1][1],lpMyShopNoSale,0,0,28,28);
			DrawSprite(ShopButtonPosi[SINALLREPAIR-1][0]-26-(256+128-sinMoveKindInter[SIN_MYSHOP])-8,ShopButtonPosi[SINALLREPAIR-1][1]-27,lpMyShopNoSale_T,0,0,100,26);
		break;
		
		case 2:
			DrawSprite(ShopButtonPosi[SINREPAIR-1][0]-(256+128-sinMoveKindInter[SIN_MYSHOP]),ShopButtonPosi[SINREPAIR-1][1],lpMyShopSale,0,0,28,28);			
			DrawSprite(ShopButtonPosi[SINREPAIR-1][0]-26-(256+128-sinMoveKindInter[SIN_MYSHOP])-8,ShopButtonPosi[SINREPAIR-1][1]-27,lpMyShopSale_T,0,0,100,26);

		break;
		case 4:
			//DrawSprite(ShopButtonPosi[SINBUY-1][0]-(256+128-sinMoveKindInter[SIN_MYSHOP]),ShopButtonPosi[SINBUY-1][1],lpMyShopExp,0,0,28,28);		
			//DrawSprite(ShopButtonPosi[SINBUY-1][0]-26-(256+128-sinMoveKindInter[SIN_MYSHOP])-8,ShopButtonPosi[SINBUY-1][1]-27,lpMyShopExp_T,0,0,100,26);
		break;
		case SINSHOPEXIT:
			DrawSprite(304-(256+128-sinMoveKindInter[SIN_MYSHOP]),313+sinInterHeight2,lpExit,0,0,20,20);	
			DrawSprite(314-10-(256+128-sinMoveKindInter[SIN_MYSHOP]),313-27+sinInterHeight2,cInvenTory.lpExitInfo,0 ,0 , 47,27);
		break;


	}
	
	switch(SelectShopButtonPosi){ 
		case SINALLREPAIR:
			if(sinShopKind != 3){ //물약 장사가 아니면 그려준다 
				DrawSprite(ShopButtonPosi[SINALLREPAIR-1][0]-(256+128-sinMoveKindInter[SIN_SHOP]),ShopButtonPosi[SINALLREPAIR-1][1],lpAllRepair,0,0,28,28);
				DrawSprite(ShopButtonPosi[SINALLREPAIR-1][0]-26-(256+128-sinMoveKindInter[SIN_SHOP]),ShopButtonPosi[SINALLREPAIR-1][1]-27,lpRepairAllInfo,0,0,77,27);
			}
		break;
		case SINREPAIR:
			if(sinShopKind != 3){ //물약 장사가 아니면 그려준다 
				DrawSprite(ShopButtonPosi[SINREPAIR-1][0]-(256+128-sinMoveKindInter[SIN_SHOP]),ShopButtonPosi[SINREPAIR-1][1],lpRepair,0,0,28,28);		
				DrawSprite(ShopButtonPosi[SINREPAIR-1][0]-26-(256+128-sinMoveKindInter[SIN_SHOP]),ShopButtonPosi[SINREPAIR-1][1]-27,lpRepairInfo,0,0,77,27);
			} 
		break;
		case SINSELL:
			DrawSprite(ShopButtonPosi[SINSELL-1][0]-(256+128-sinMoveKindInter[SIN_SHOP]),ShopButtonPosi[SINSELL-1][1],lpSell,0,0,28,28);		
			DrawSprite(ShopButtonPosi[SINSELL-1][0]-26-(256+128-sinMoveKindInter[SIN_SHOP]),ShopButtonPosi[SINSELL-1][1]-27,lpSellInfo,0,0,77,27);
		break;
		case SINBUY:
			DrawSprite(ShopButtonPosi[SINBUY-1][0]-(256+128-sinMoveKindInter[SIN_SHOP]),ShopButtonPosi[SINBUY-1][1],lpBuy ,0,0,28,28);		
			DrawSprite(ShopButtonPosi[SINBUY-1][0]-26-(256+128-sinMoveKindInter[SIN_SHOP]),ShopButtonPosi[SINBUY-1][1]-27,lpBuyInfo,0,0,77,27);
		break;
		case SINRIGHTARROW: 
			DrawSprite(282-(256+128-sinMoveKindInter[SIN_SHOP]),182+sinInterHeight2,cInvenTory.lpInvenArrow[1],0,0,20,20);		

		break;
		case SINLEFTARROW:		
			DrawSprite(260-(256+128-sinMoveKindInter[SIN_SHOP]),182+sinInterHeight2,cInvenTory.lpInvenArrow[0],0,0,20,20);				
		break;
		case SINSHOPEXIT:
			DrawSprite(304-(256+128-sinMoveKindInter[SIN_SHOP]),313+sinInterHeight2,lpExit,0,0,20,20);	
			DrawSprite(314-10-(256+128-sinMoveKindInter[SIN_SHOP]),313-27+sinInterHeight2,cInvenTory.lpExitInfo,0 ,0 , 47,27);
		
		break;
					
	}

	/*
	switch(SelectShopButtonPosi){ 
		case SINALLREPAIR:
			if(sinShopKind != 3){ //물약 장사가 아니면 그려준다 
				DrawSprite(264-(256+128-sinMoveKindInter[SIN_SHOP]),306+sinInterHeight2,lpAllRepair,0,0,28,28);
				DrawSprite(264-26-(256+128-sinMoveKindInter[SIN_SHOP]),306-27+sinInterHeight2,lpRepairAllInfo,0,0,77,27);
			}
		break;
		case SINREPAIR:
			if(sinShopKind != 3){ //물약 장사가 아니면 그려준다 
				DrawSprite(231-(256+128-sinMoveKindInter[SIN_SHOP]),306+sinInterHeight2,lpRepair,0,0,28,28);		
				DrawSprite(231-26-(256+128-sinMoveKindInter[SIN_SHOP]),306-27+sinInterHeight2,lpRepairInfo,0,0,77,27);
			} 
		break;
		case SINSELL:
			DrawSprite(231-(256+128-sinMoveKindInter[SIN_SHOP]),275+sinInterHeight2,lpSell,0,0,28,28);		
			DrawSprite(231-26-(256+128-sinMoveKindInter[SIN_SHOP]),275-27+sinInterHeight2,lpSellInfo,0,0,77,27);
		break;
		case SINBUY:
			DrawSprite(263-(256+128-sinMoveKindInter[SIN_SHOP]),275+sinInterHeight2,lpBuy ,0,0,28,28);		
			DrawSprite(263-26-(256+128-sinMoveKindInter[SIN_SHOP]),275-27+sinInterHeight2,lpBuyInfo,0,0,77,27);
		break;
		case SINRIGHTARROW: 
			DrawSprite(282-(256+128-sinMoveKindInter[SIN_SHOP]),182+sinInterHeight2,cInvenTory.lpInvenArrow[1],0,0,20,20);		

		break;
		case SINLEFTARROW:		
			DrawSprite(260-(256+128-sinMoveKindInter[SIN_SHOP]),182+sinInterHeight2,cInvenTory.lpInvenArrow[0],0,0,20,20);				
		break;
		case SINSHOPEXIT:
			DrawSprite(304-(256+128-sinMoveKindInter[SIN_SHOP]),313+sinInterHeight2,lpExit,0,0,20,20);	
			DrawSprite(314-10-(256+128-sinMoveKindInter[SIN_SHOP]),313-27+sinInterHeight2,cInvenTory.lpExitInfo,0 ,0 , 47,27);
		break;
					
	}
	*/

	//사용할수없는 아이템을 체크한다 
	if(cShop.OpenFlag){ 
		for(int cnt = 0 ;cnt < 30 ; cnt++){
			if(ShowShopItem[cnt].Flag == 1){ //인벤토리에 아이템이 있을때 아이템을 그린다 
				if(ShowShopItem[cnt].sItemInfo.NotUseFlag){
					dsDrawColorBox( sinInvenColor[2] ,ShowShopItem[cnt].x-(256+128-sinMoveKindInter[SIN_SHOP]),ShowShopItem[cnt].y, ShowShopItem[cnt].w, ShowShopItem[cnt].h );
				}

			}
		}
	}


	if(ShowShopItemIndex){
		for(i=0; i<GridRow;i++){
			for(j=0;j<GridCol;j++){
				DrawSprite(StartGirdX+(22*i)-(256+128-sinMoveKindInter[SIN_SHOP]),StartGirdY+(22*j) ,lpGrid,0,0, 22,22); 

			}
		}
		dsDrawColorBox( sinInvenColor[0] ,ShopItemRect.left-(256+128-sinMoveKindInter[SIN_SHOP]),ShopItemRect.top, ShopItemRect.right, ShopItemRect.bottom );

	}

	for( i=0; i < 30 ; i++) //보여줄수있는 최대치의 아이템을 보여준다  
		if(ShowShopItem[i].Flag){
			DrawSprite(ShowShopItem[i].x-(256+128-sinMoveKindInter[SIN_SHOP]),ShowShopItem[i].y ,ShowShopItem[i].lpItem,0,0, ShowShopItem[i].w,ShowShopItem[i].h); 
			

		}


	//MyShopItem을 그린다

	if(SelectMyShopItemIndex){  
		dsDrawColorBox( sinInvenColor[4] ,cMyShop.MyShopItem[SelectMyShopItemIndex-1].x-(256+128-sinMoveKindInter[SIN_MYSHOP]),
					cMyShop.MyShopItem[SelectMyShopItemIndex-1].y, cMyShop.MyShopItem[SelectMyShopItemIndex-1].w, cMyShop.MyShopItem[SelectMyShopItemIndex-1].h );	
	}

	if(cMyShop.OpenFlag){   
		for(i = 0 ; i < 30 ; i++){
			if(cMyShop.MyShopItem[i].Flag){
				DrawSprite(cMyShop.MyShopItem[i].x-(256+128-sinMoveKindInter[SIN_MYSHOP]) , cMyShop.MyShopItem[i].y, cMyShop.MyShopItem[i].lpItem ,0,0, cMyShop.MyShopItem[i].w , cMyShop.MyShopItem[i].h);
				if((cMyShop.MyShopItem[i].CODE & sinITEM_MASK2) == sinFO1 ){
					cInvenTory.DrawForceOrbEffect(cMyShop.MyShopItem[i].x-(256+128-sinMoveKindInter[SIN_MYSHOP]) , cMyShop.MyShopItem[i].y);
				}
			}
		}
	}


	int x , y;
	if(cMyShop.OpenFlag){  
		if(MyShopMouseItem.Flag){
			if(!sMessageBox2[MESSAGE_MYSHOP_ITEM].Flag){ //메세지 박스가 떠있을경우에는 띄워주지않는다(야매 -_-)
				x = pCursorPos.x-(MyShopMouseItem.w/2);
				y = pCursorPos.y -(MyShopMouseItem.h/2);

				dsDrawOffsetArray = dsARRAY_TOP;
				DrawSprite(x , y , MyShopMouseItem.lpItem,0,0,MyShopMouseItem.w,MyShopMouseItem.h);
				dsDrawColorBox( RGBA_MAKE(0,0,128,125) ,x ,y,MyShopMouseItem.w ,MyShopMouseItem.h);
				DrawSprite(x , y , MyShopMouseItem.lpItem,0,0,MyShopMouseItem.w,MyShopMouseItem.h);
			}
		}
	}


	if(SelectMyShopItemIndex){  
		dsDrawColorBox( sinInvenColor[4] ,cCharShop.CharShopItem[SelectMyShopItemIndex-1].x-(256+128-sinMoveKindInter[SIN_CHARSHOP]),
					cCharShop.CharShopItem[SelectMyShopItemIndex-1].y, cCharShop.CharShopItem[SelectMyShopItemIndex-1].w, cCharShop.CharShopItem[SelectMyShopItemIndex-1].h );	
	}


	//CharShop 아이템을 그린다
	if(cCharShop.OpenFlag){   
		for(i = 0 ; i < 30 ; i++){
			if(cCharShop.CharShopItem[i].Flag){
				DrawSprite(cCharShop.CharShopItem[i].x-(256+128-sinMoveKindInter[SIN_CHARSHOP]) , cCharShop.CharShopItem[i].y, cCharShop.CharShopItem[i].lpItem ,0,0, cCharShop.CharShopItem[i].w , cCharShop.CharShopItem[i].h);
				if((cCharShop.CharShopItem[i].CODE & sinITEM_MASK2) == sinFO1 ){
					cInvenTory.DrawForceOrbEffect(cCharShop.CharShopItem[i].x-(256+128-sinMoveKindInter[SIN_CHARSHOP]), cCharShop.CharShopItem[i].y);
				}
			}
		}
	}


	//상점 버튼
	if(MyShopSendButton && !cMyShop.OpenFlag){    
		//DrawSprite(50,200, lpMyShop_Button ,0,0, 93 , 79);
		dsDrawTexImage(MatMyShop_Button,10,220,70,70,255);   
	}


	int BackUpPosX , BackUpPosY = 0;
	int BackUpButton2X , BackUpButton2Y = 0;
	int BackUpButton3X , BackUpButton3Y = 0;


	//홍보말을 입력한다
 	if(MyShopExpBox){              
		BackUpButton2X = 35;    
		BackUpButton2Y = 157;
		BackUpButton3X = 115;
		BackUpButton3Y = 157;

		BackUpPosX = BackStartPos.x;
		BackUpPosY = BackStartPos.y;

		BackStartPos.x = MyShopExpBoxX;
		BackStartPos.y = MyShopExpBoxY;

		Interface_clanmenu.DrawBox(BackStartPos.x,BackStartPos.y,2,3);	            
		dsDrawTexImage(MatHelpBoxImage[9],BackStartPos.x+7,BackStartPos.y+35,180,15,255);
		
		
        dsDrawTexImage(MatMyShopPRBoxLine,BackStartPos.x+28,BackStartPos.y+50,254,90,255);

		DrawSprite(BackStartPos.x+BackUpButton2X,BackStartPos.y+BackUpButton2Y, cMessageBox.lpButtonBox07,  0, 0, 48, 23, 1);
		DrawSprite(BackStartPos.x+BackUpButton3X,BackStartPos.y+BackUpButton3Y, cMessageBox.lpButtonBox07,  0, 0, 48, 23, 1);

		if(MyShopExpButtonOK)  
			DrawSprite(BackStartPos.x+BackUpButton2X+7,BackStartPos.y+BackUpButton2Y+3, cMessageBox.lpButtonYes07,  0, 0, 32, 16, 1);
		else DrawSprite(BackStartPos.x+BackUpButton2X+7,BackStartPos.y+BackUpButton2Y+3, cMessageBox.lpButtonYesGlay07,  0, 0, 32, 16, 1);
		if(MyShopExpButtonCancel)
			DrawSprite(BackStartPos.x+BackUpButton3X+7,BackStartPos.y+BackUpButton3Y+3, cMessageBox.lpButtonNo07,  0, 0, 32, 16, 1);
		else DrawSprite(BackStartPos.x+BackUpButton3X+7,BackStartPos.y+BackUpButton3Y+3, cMessageBox.lpButtonNoGlay07,  0, 0, 32, 16, 1);

		BackStartPos.x = BackUpPosX;
		BackStartPos.y = BackUpPosY;

	}


}
/*----------------------------------------------------------------------------*
*							     메인  
*-----------------------------------------------------------------------------*/	
DWORD dwBuyItemServerTime = 0;
int MyShopItemSellMoney2 = 0;
DWORD dwBuyCharShopTime = 0;
void cSHOP::Main()
{
	int i = 0; 
	//개인상점 이용시 1초딜레이를 준다.
	if(haCharShopDelayFlag){
		dwBuyCharShopTime++;
		if( dwBuyCharShopTime >= 1*70){
			haCharShopDelayFlag = 0;
			dwBuyCharShopTime = 0;
		}
	}

	cMyShop.AutoCloseShop();

	MyShopExpBoxX = 300;
	MyShopExpBoxY = 170;

	CheckShopNpcState();      //NPC와의 헤어짐을 체크한다 
	SelectShopButtonPosi = 0; //초기화 
	ShowShopItemIndex = 0;
	ShopItemPrice = 0;

	CloseMyShopFlag = 0;
	CloseCharShopFlag = 0;


	int BackUpPosX , BackUpPosY = 0;

	//홍보말을 입력한다
	MyShopExpButtonOK = 0;
	MyShopExpButtonCancel = 0;

	int BackUpButton2X , BackUpButton2Y = 0;
	int BackUpButton3X , BackUpButton3Y = 0;

	if(MyShopExpBox){ 
		BackUpPosX = BackStartPos.x; 
		BackUpPosY = BackStartPos.y;

		BackStartPos.x = MyShopExpBoxX;
		BackStartPos.y = MyShopExpBoxY;

		BackUpButton2X = 35;    
		BackUpButton2Y = 157;
		BackUpButton3X = 115;
		BackUpButton3Y = 157;

		if(pCursorPos.x >BackStartPos.x+BackUpButton2X && pCursorPos.x < BackStartPos.x+BackUpButton2X+32 && 
			pCursorPos.y >BackStartPos.y+BackUpButton2Y && pCursorPos.y < BackStartPos.y+BackUpButton2Y+16)
			MyShopExpButtonOK = 1;
		
		if(pCursorPos.x >BackStartPos.x+BackUpButton3X && pCursorPos.x < BackStartPos.x+BackUpButton3X+32 && 
			pCursorPos.y >BackStartPos.y+BackUpButton3Y && pCursorPos.y < BackStartPos.y+BackUpButton3Y+16)
			MyShopExpButtonCancel = 1;

		BackStartPos.x = BackUpPosX;
		BackStartPos.y = BackUpPosY;

	}

	/////////////개인 상점
	if(cMyShop.OpenFlag){ 
		if(pCursorPos.x >= DownTradePosi[T_CLOSE_POSI][0] && pCursorPos.x <= DownTradePosi[T_CLOSE_POSI][2] &&
			pCursorPos.y >= DownTradePosi[T_CLOSE_POSI][1] && pCursorPos.y <= DownTradePosi[T_CLOSE_POSI][3]){
				CloseMyShopFlag  = 1;
		}
	}

	/////////////캐릭터 상점
	if(cCharShop.OpenFlag){
		if(pCursorPos.x >= DownTradePosi[T_CLOSE_POSI][0] && pCursorPos.x <= DownTradePosi[T_CLOSE_POSI][2] &&
			pCursorPos.y >= DownTradePosi[T_CLOSE_POSI][1] && pCursorPos.y <= DownTradePosi[T_CLOSE_POSI][3]){
				CloseCharShopFlag  = 1;
		}
	}

	SelectMyShopButtonPosi = 0; 
	if(cMyShop.OpenFlag){
		if(!MyShopExpBox){
			for( i=0;i<9;i++)
				if ( ShopButtonPosi[i][0] < pCursorPos.x  && ShopButtonPosi[i][2] > pCursorPos.x && 
					 ShopButtonPosi[i][1] < pCursorPos.y  && ShopButtonPosi[i][3] > pCursorPos.y && !MouseItem.Flag)
					 SelectMyShopButtonPosi = i+1;
		}
			
	}

	SelectCharShopButtonPosi = 0;
	if(cCharShop.OpenFlag){
		for( i=0;i<9;i++)
			if ( ShopButtonPosi[i][0] < pCursorPos.x  && ShopButtonPosi[i][2] > pCursorPos.x && 
				 ShopButtonPosi[i][1] < pCursorPos.y  && ShopButtonPosi[i][3] > pCursorPos.y && !MouseItem.Flag)
				 SelectCharShopButtonPosi = i+1;
			
	}


	int Index2 = 0;
	int BackUpX = 0 , BackUpY = 0; 
	SelectMyShopItemIndex = 0;
	if(cMyShop.OpenFlag){
		for(i=0 ; i < 30 ; i++){ 
			if(cMyShop.MyShopItem[i].Flag){
				if ( cMyShop.MyShopItem[i].x < pCursorPos.x  && cMyShop.MyShopItem[i].x + cMyShop.MyShopItem[i].w > pCursorPos.x  && 
						cMyShop.MyShopItem[i].y < pCursorPos.y  && cMyShop.MyShopItem[i].y + cMyShop.MyShopItem[i].h > pCursorPos.y ){
						Index2 = cMyShop.SearchShopItemToInven(&cMyShop.MyShopItem[i]);
						if(Index2){
							SelectMyShopItemIndex = i+1;
							BackUpX = cInvenTory.InvenItem[Index2-1].x;
							BackUpY = cInvenTory.InvenItem[Index2-1].y;
							cInvenTory.InvenItem[Index2-1].x = cMyShop.MyShopItem[i].x;
							cInvenTory.InvenItem[Index2-1].y = cMyShop.MyShopItem[i].y;
							sinShowItemInfoFlag = 1; //아이템의 정보를 보여준다 
							MyShopItemSellMoney2 = cMyShop.MyShopItem[i].sItemInfo.Money;
							cItem.ShowItemInfo(&cInvenTory.InvenItem[Index2-1],3,Index2-1);
							cInvenTory.InvenItem[Index2-1].x = BackUpX;
							cInvenTory.InvenItem[Index2-1].y = BackUpY;

						}
					}
			}
		}
	}
	if(cCharShop.OpenFlag){
		for(i=0 ; i < 30 ; i++){ 
			if(cCharShop.CharShopItem[i].Flag){
				if ( cCharShop.CharShopItem[i].x < pCursorPos.x  && cCharShop.CharShopItem[i].x + cCharShop.CharShopItem[i].w > pCursorPos.x  && 
						cCharShop.CharShopItem[i].y < pCursorPos.y  && cCharShop.CharShopItem[i].y + cCharShop.CharShopItem[i].h > pCursorPos.y ){
						SelectMyShopItemIndex = i+1;
						Index2 = cCharShop.SearchMyShopItem(cCharShop.CharShopItem[i].sItemInfo.CODE , cCharShop.CharShopItem[i].sItemInfo.ItemHeader.Head ,cCharShop.CharShopItem[i].sItemInfo.ItemHeader.dwChkSum);
						if(Index2){
							sinShowItemInfoFlag = 1; //아이템의 정보를 보여준다 
							MyShopItemSellMoney2 = sRecvMyShop.sMyShopItem[Index2-1].Price;
							cItem.ShowItemInfo(&cCharShop.CharShopItem[i],3);
						}
				}
			}
		}
	}


	//서버에서 아이템을 살때 오류발생시 10초후 다시 초기화해준다 
	if(BuyItemServerFlag){
	 	dwBuyItemServerTime++;
		if( dwBuyItemServerTime >= 10*70){
			BuyItemServerFlag = 0;
			dwBuyItemServerTime = 0;
		}

	}
	


	if(OpenFlag){
		for( i=0;i<9;i++)
			if ( ShopButtonPosi[i][0] < pCursorPos.x  && ShopButtonPosi[i][2] > pCursorPos.x && 
				 ShopButtonPosi[i][1] < pCursorPos.y  && ShopButtonPosi[i][3] > pCursorPos.y && !MouseItem.Flag)
				 SelectShopButtonPosi = i+1;
			
	}
	if(OpenFlag){ 
		for(i =0; i <30 ; i++){
			cInvenTory.CharOnlySetItem(&ShowShopItem[i]);		//캐릭전용 아이템
			cInvenTory.CheckRequireItemToSet(&ShowShopItem[i]); //능력치에 맞는 아이템
			if ( ShowShopItem[i].x < pCursorPos.x  && ShowShopItem[i].x + ShowShopItem[i].w > pCursorPos.x && 
				ShowShopItem[i].y < pCursorPos.y  && ShowShopItem[i].y + ShowShopItem[i].h > pCursorPos.y ){
				if(ShowShopItem[i].Flag){
					if(!MouseItem.Flag){
						ShopItemRect.left   = ShowShopItem[i].x;
						ShopItemRect.top    = ShowShopItem[i].y;
						ShopItemRect.right  = ShowShopItem[i].w;
						ShopItemRect.bottom = ShowShopItem[i].h;
					
						GridRow = ShowShopItem[i].w/22;
						GridCol = ShowShopItem[i].h/22;
						StartGirdX = ShowShopItem[i].x;
						StartGirdY = ShowShopItem[i].y;
						sinShowItemInfoFlag = 1;  

						ShopItemPrice = ShowShopItem[i].sItemInfo.Price; //가격을 넣는다 
	     				cItem.ShowItemInfo(&ShowShopItem[i] , 1); //상점은 1
						ShowShopItemIndex = i+1;
					}
				}
			}
		}

	}
}
/*----------------------------------------------------------------------------*
*							     종료 
*-----------------------------------------------------------------------------*/	
void cSHOP::Close()
{
	Release();
}
/*----------------------------------------------------------------------------*
*							   LButtonDown
*-----------------------------------------------------------------------------*/	
void cSHOP::LButtonDown(int x , int y)
{
	int cnt = 0;
	sITEM TempItem;
	
	//아이콘 클릭
	if(MyShopSendButton && !cMyShop.OpenFlag){   
		if(x >= 10 && x <= 10+70 && y >= 220 && y<= 220+70){
			cInterFace.CheckAllBox(SIN_MYSHOP);
		}
		//DrawSprite(50,200, lpMyShop_Button ,0,0, 93 , 79);
	}

	memset(&TempItem,0,sizeof(sITEM));
	
	MyShopItemCancelIndex[0] = 0; 
	MyShopItemCancelIndex[1] = 0;
	int Index3 = 0;

	if(cMyShop.OpenFlag&&!MyShopMouseItem.Flag){
		//등록된 아이템을 취소한다
		if(!MyShopSendButton){
			for(int i=0 ; i < 30 ; i++){ 
				if(cMyShop.MyShopItem[i].Flag){
					if ( cMyShop.MyShopItem[i].x < pCursorPos.x  && cMyShop.MyShopItem[i].x + cMyShop.MyShopItem[i].w > pCursorPos.x  && 
							cMyShop.MyShopItem[i].y < pCursorPos.y  && cMyShop.MyShopItem[i].y + cMyShop.MyShopItem[i].h > pCursorPos.y ){
							Index3 = cMyShop.SearchShopItemToInven(&cMyShop.MyShopItem[i]);
							if(Index3){
								cMessageBox.ShowMessage3(MESSAGE_MYSHOP_ITEM_CANCEL,cInvenTory.InvenItem[Index3-1].sItemInfo.ItemName);
								MyShopItemCancelIndex[0] = Index3;
								MyShopItemCancelIndex[1] = i+1;
								break;
							}

					}
				}
			}
		}
	}

	//개인상점
	if(cMyShop.OpenFlag){   
		if(MyShopMouseItem.Flag){
			if(!sinMyShopItemPass && !MyShopSendButton){
				if(!cMyShop.SetMyShopItem(&MyShopMouseItem))
					memset(&MyShopMouseItem,0,sizeof(sITEM)); //등록이 안되면지워준다
			}
		}
		if(CloseMyShopFlag){
			if(!MyShopSendButton){
				cMyShop.OpenFlag = 0;
				memset(cMyShop.MyShopItem,0,sizeof(sITEM)*30); //아이템을 지워준다
				memset(MyShopItemIndex,0,sizeof(int)*100);
				MyShopSendButton = 0; //등록버튼 초기화
			}
			else{
				cMyShop.OpenFlag = 0;

			}
		}
	}
	
	//캐릭터 상점
	int Index2 = 0;
	if(cCharShop.OpenFlag){ 
		if(CloseCharShopFlag){
			cCharShop.OpenFlag = 0; //상점을 닫는다
			memset(cCharShop.CharShopItem,0,sizeof(sITEM)*30); //아이템을 지워준다

		}

		memset(&sMyShop_Server,0,sizeof(sMYSHOP_ITEM_SERVER));
		if(SelectMyShopItemIndex){ 
			if(!haCharShopDelayFlag){  //해당물건이 거래완료후에만 클릭이 된다.
				cCharShop.BuyItem(SelectMyShopItemIndex-1); //인덱스를 뺀후에 넣어준다
				haCharShopDelayFlag = 1;
			}
		}

/*
		if(SelectMyShopItemIndex && CursorClass == SIN_CURSOR_BUY){
			cCharShop.BuyItem(SelectMyShopItemIndex-1); //인덱스를 뺀후에 넣어준다

		}
*/
	}

	///////상점 홍보
	if(MyShopExpButtonOK){ 
		lstrcpy(cMyShop.szSendDoc,cMyShop.szDoc);
		MyShopExpBox = 0; 
		isDrawClanMenu = 0;
		hFocusWnd = 0;  
		cInterFace.ChatFlag = 0;
		//SetWindowText( hFocusWnd , "");
		sinChatEnter = 0; //임의로 글씨가 안보이게한다
		sinMessageBoxShowFlag = 0; //다른로직때문에 메세지박스 플렉을 세워야함 
		MyShopSendButton = 1;
		cMyShop.SendMyShopItem();
		SetIME_Mode(0);

	} 
	if(MyShopExpButtonCancel){
		MyShopExpBox = 0; 
		isDrawClanMenu = 0;
		hFocusWnd = 0;  
		cInterFace.ChatFlag = 0;
		//SetWindowText( hFocusWnd , "");
		sinChatEnter = 0; //임의로 글씨가 안보이게한다
		sinMessageBoxShowFlag = 0; //다른로직때문에 메세지박스 플렉을 세워야함 
		//cMyShop.szSendDoc[0] = 0;  //서버로보낼 메세지 초기화
		SetIME_Mode(0);
		
	}

	int j = 0;
	int CheckMyShopItemFlag = 0;

	//개인 상점
	switch(SelectMyShopButtonPosi){
		case 1: //판매중지
			MyShopSendButton = 0;
			SendClosePersonalTrade();

		break;
		case 2: //판매시작 
			for(j = 0 ; j < 100 ;j++){ 
				if(MyShopItemIndex[j]){ 
					CheckMyShopItemFlag = 1;
				}
			}

			if(CheckMyShopItemFlag){
				if(!MyShopSendButton ){
					//
					if(!MyShopSendButton){
						SetIME_Mode(1);
						MyShopExpBox = 1;   
						hFocusWnd = hTextWnd;  
						//SetWindowText( hFocusWnd , "");
						cInterFace.ChatFlag = 0;
						sinChatEnter = 0;
						sinMessageBoxShowFlag = 1;
					}

//					MyShopSendButton = 1;
//					cMyShop.SendMyShopItem();
				}
			}
			else{
				cMessageBox.ShowMessage(MESSAGE_NOT_SALE_ITEM);

			}

		break;
		case 4: //홍보
			/*
			if(!MyShopSendButton){
				MyShopExpBox = 1;   
				hFocusWnd = hTextWnd;  
				//SetWindowText( hFocusWnd , "");
				cInterFace.ChatFlag = 0;
				sinChatEnter = 0;
				sinMessageBoxShowFlag = 1;
			}
			*/
		break;

	}

	//캐릭터 상점
	switch(SelectCharShopButtonPosi){
		case 3: //사기

		break;


	}


	sITEMPRICE sinItemPrice;
	
	if(ShowShopItemIndex){ //아이템 사기 
		if(CursorClass == 1 || CursorClass == SIN_CURSOR_BUY){ 
			if(ShowShopItem[ShowShopItemIndex-1].Class == ITEM_CLASS_POTION){ //물약 
				pMessageItem = &ShowShopItem[ShowShopItemIndex-1];
				cMessageBox.ShowMessage2(MESSAGE_BUY_ITEM);
				BuyItemIndexCopy = ShowShopItemIndex;

			}
			else{ //그냥 아이템 
				pMessageItem = &ShowShopItem[ShowShopItemIndex-1];
				cMessageBox.ShowMessage2(MESSAGE_BUY_ITEM_DEFALT);
				BuyItemIndexCopy = ShowShopItemIndex;
			}
			
		}

	}

	switch(SelectShopButtonPosi){
		case 1: //다고치기
			if(sinShopKind == 3)break; //물약 상인일경우에는 break;
			for(cnt = 0 ; cnt < INVENTORY_MAXITEM ; cnt++){
				if(cInvenTory.InvenItem[cnt].Flag){
//					if(cInvenTory.InvenItem[cnt].ItemPosition){
						sinItemPrice = cInvenTory.GetInvenItemPrice(&cInvenTory.InvenItem[cnt]);
					//돈이 있는것 만큼만 고친다 
					if(!cInvenTory.RepairInvenItem(&cInvenTory.InvenItem[cnt] ,sinItemPrice.RepairCost))  
						continue; 
//					}
				}
			}
			sinPlaySound(SIN_SOUND_REPAIR_ITEM);

		break;

		case 2: //고치기 
			if(sinShopKind == 3)break; //물약 상인일경우에는 break;
			CursorClass = SIN_CURSOR_REPAIR;
		break;

		case 3: //팔기 
			CursorClass = SIN_CURSOR_SELL;

		break;

		case 4: //물건사기
			CursorClass = SIN_CURSOR_BUY;
			//cMessageBox.ShowMessage(MESSAGE_HELP_1);

		break;

		case 5: //오른쪽 화살표 
			ShopArrowPosi = 1;

			if(sinShopKind == 2)//방어구 이면 
				CopyShopItemToShow(ShowShopItemIndex2,1);

			else
				CopyShopItemToShow(ShowShopItemIndex2,0);


		break;
		case 6: //왼쪽 화살표 

			ShopArrowPosi =0;
			if(sinShopKind == 2)//방어구 이면 
				CopyShopItemToShow(0,1);

			else
				CopyShopItemToShow(0,0);

		break;

		case 7: //나가기 
			OpenFlag = 0;

		break;

		case 8: //무기구
			if(sinShopKind==3)break; //잡화이면 브레이크 
			sinShopKind = 1;
			ShowShopItemIndex2 = 0;
			CopyShopItemToShow(0,0);   //무기는 0-30
			ShopArrowPosi = 0;       //Tab을 누르면 초기화 한다 
		break;

		case 9: //방어구
			if(sinShopKind==3)break; //잡화이면 브레이크 
			sinShopKind = 2;
			ShowShopItemIndex2 = 0;
			CopyShopItemToShow(0,1);   //방어구는 30-60  
			ShopArrowPosi = 0;       //Tab을 누르면 초기화 한다 

	
		break;
		default:
			if(MouseItem.Flag){    //아이템의 위치 보정 
				CursorClass = 0; //마우스 커서를 없앤다 
			}
			else{
				CursorClass = 1;

			}
		break;
	}


	//캐릭터 상점의 마우스 커서
	if(cCharShop.OpenFlag){ 
		if(SelectCharShopButtonPosi == 4){
			CursorClass = SIN_CURSOR_BUY;

		}
	}

	//상점아이템을 초기화한다
	DeleteShopItem();

	/////////////////////////////////아이템 겹치기를 없애기 위해 잡은 아이템의 셋팅여부를 체크해준다 
	////////////권호씨에게 이영광을 !!!!음하하하하 
	if(MouseItem.Flag)
		cInvenTory.CheckRequireItemToSet(&MouseItem); //아이템이 셋팅될수있는지를 체크한다 

}
/*----------------------------------------------------------------------------*
*							   LButtonUp
*-----------------------------------------------------------------------------*/	
void cSHOP::LButtonUp(int x , int y)
{


}
/*----------------------------------------------------------------------------*
*							   RButtonDown
*-----------------------------------------------------------------------------*/	
void cSHOP::RButtonDown(int x , int y)
{
	
	sITEM TempItem;
    memset(&TempItem,0,sizeof(sITEM));
	if(cCharShop.OpenFlag){
		if(SelectMyShopItemIndex){ 
			cCharShop.BuyItem(SelectMyShopItemIndex-1); //인덱스를 뺀후에 넣어준다
		}
	}

	if(ShowShopItemIndex){ //오른쪽 버튼으로 아이템을 산다
		if(CursorClass == 1 || CursorClass == SIN_CURSOR_BUY){ 
			if(ShowShopItem[ShowShopItemIndex-1].Class == ITEM_CLASS_POTION){
				pMessageItem = &ShowShopItem[ShowShopItemIndex-1];
				cMessageBox.ShowMessage2(MESSAGE_BUY_ITEM);
				BuyItemIndexCopy = ShowShopItemIndex;

			}
			else{
				if(!BuyItemServerFlag){
					SendBuyItemToServer(&ShowShopItem[ShowShopItemIndex-1]);
					
				}
			}
		}
	}
}
/*----------------------------------------------------------------------------*
*							   RButtonUp
*-----------------------------------------------------------------------------*/	
void cSHOP::RButtonUp(int x , int y)
{

}
/*----------------------------------------------------------------------------*
*							    KeyDown
*-----------------------------------------------------------------------------*/	
void cSHOP::KeyDown()
{
	//상점아이템을 초기화한다
	DeleteShopItem();
}
/*----------------------------------------------------------------------------*
*	
*-----------------------------------------------------------------------------*/	
void cSHOP::CheckShopNpcState() 
{
	if(!DispEachMode){
		if(TalkNpcState){
			cShop.OpenFlag = 0;
			cInvenTory.OpenFlag = 0;
			TalkNpcState = 0;
		}

	}

}

//서버에서 받아온 아이템을 보여줄 아이템으로 복사한다 
void cSHOP::CopyShopItemToShow(int Index,int Kind)
{
	memset(ShowShopItem,0,sizeof(sITEM)*30); //보여질 아이템을 초기화한다 

	for(int i= 0 ; i < 30 -Index; i++){
		if(GetShopItemXY(&ShopItem[i+Index+(Kind*30)])){//한번에 들어갈수있는 아이템 
			memcpy(&ShowShopItem[i],&ShopItem[i+Index+(Kind*30)],sizeof(sITEM));

		}
		else{//들어갈수없는 아이템 
			ShowShopItemIndex2 = i+Index;
			break;

		}
			
	}

}
//아이템이 셋팅될 좌표를 구해온다 
int cSHOP::GetShopItemXY(sITEM *pItem)
{

	int cntx,cnty;
	int cx,cy;

	RECT	rect;
	int		flag;

	int StartX = 21;
	int StartY = 136+sinInterHeight2;

	cx = (22*SHOP_ROW)-pItem->w; //아이템이 들어갈수있는 가장 끝 X좌표 
	cy = (22*SHOP_COL)-pItem->h; //아이템이 들어갈수있는 가장 끝 Y좌표

	for( cntx=StartX ; cntx <= StartX+cx ; cntx+=22){//코드가 같지 않거나 아이템이 비어있지 않은 곳에 셋팅할때 검색해서 위치를 찾아낸다 
		for( cnty=StartY ; cnty <= StartY+cy ; cnty+=22){ 
			flag = 0;
			for(int i=0; i < 30 ; i++){
				if(ShowShopItem[i].Flag){
					rect.left   = ShowShopItem[i].x;
					rect.right  = ShowShopItem[i].x+ShowShopItem[i].w;
					rect.top    = ShowShopItem[i].y;
					rect.bottom = ShowShopItem[i].y+ShowShopItem[i].h;

					if( cntx > rect.left-pItem->w && cntx< rect.right && 
						cnty > rect.top-pItem->h && cnty< rect.bottom )
						flag++;
				}
			}
			if ( flag==0 ) {
				pItem->x = cntx;
				pItem->y = cnty;
				return TRUE;
			}
		}
	}
	return FALSE;
}

//아이템을 서버에서 산다 
int cSHOP::SendBuyItemToServer(sITEM *pItem , int ItemCount )
{
	if(!cInvenTory.CheckSetEmptyArea(pItem)){ 
		cMessageBox.ShowMessage(MESSAGE_OVER_SPACE); //공간 초과  
		return FALSE;
	}
	//--------------------------------------------------------------------------//
	#ifdef HASIEGE_MODE
		//공성전 세율적용 아이템 구입시 돈체크
		if(cShop.haBuyMoneyCheck(pItem->sItemInfo.Price*ItemCount)==0){
	#else
		if((sinChar->Money - (pItem->sItemInfo.Price*ItemCount)) < 0){
	#endif
	//---------------------------------------------------------------------------//
		cMessageBox.ShowMessage(MESSAGE_NOT_ENOUGH_MONEY); //돈부족
		return FALSE;
	}
	if(pItem->sItemInfo.Weight+sinChar->Weight[0] > sinChar->Weight[1]){
		cMessageBox.ShowMessage(MESSAGE_OVER_WEIGHT);    //무게초과 
		return FALSE;

	}
	BuyItemServerFlag = 1; //중복으로 서버에 메세지를 보내지 않기 위해서 플렉을 준다 

	TRANS_BUY_SHOPITEM	TransBuyShopItem;

	TransBuyShopItem.code = smTRANSCODE_SHOPTITEM;
	TransBuyShopItem.size = sizeof(TRANS_BUY_SHOPITEM);
	TransBuyShopItem.ItemCount = ItemCount;
	memcpy( &TransBuyShopItem.sItem , pItem , sizeof(sITEM) );
	if ( smWsockDataServer ) {
		return smWsockDataServer->Send2( (char *)&TransBuyShopItem , TransBuyShopItem.size , TRUE );
	}
	return FALSE;
}

//아이템 사기 
int cSHOP::RecvBuyItemToServer(sITEM *pItem ,int ItemCount )
{
	if ( smConfig.DebugMode ) {
		// test: the bought item's serial as it arrives, and the struct layout (compare with the
		// server's LogDebug\InvenTrace.log "layout" line)
		char szDbg[160];
		extern int AddChatBuff( char *szMsg );
		wsprintf( szDbg , "BUY in: code %d serial %d / %d | sITEM %d, info at %d, header at %d" ,
			(int)pItem->sItemInfo.CODE , (int)pItem->sItemInfo.ItemHeader.Head , (int)pItem->sItemInfo.ItemHeader.dwChkSum ,
			(int)sizeof(sITEM) , (int)((char *)&pItem->sItemInfo - (char *)pItem) ,
			(int)((char *)&pItem->sItemInfo.ItemHeader - (char *)&pItem->sItemInfo) );
		AddChatBuff( szDbg );
	}

	BuyItemServerFlag = 0;
	CheckCharForm();//인증

	//현재 포션은 잠시 내버려둔다 
    //--------------------------------------------------------------------------//
	#ifdef HASIEGE_MODE
		//공성전 세율적용에 의해 수정됨
		if(haBuyMoneyCheck(pItem->sItemInfo.Price*ItemCount)){
	#else
		if((sinChar->Money - (pItem->sItemInfo.Price*ItemCount)) >= 0){
	#endif
	//---------------------------------------------------------------------------//
	    //sinChar->Money -= (pItem->sItemInfo.Price*ItemCount); //가격지불 
		sinMinusMoney(pItem->sItemInfo.Price*ItemCount,1);
		if(pItem->Class == ITEM_CLASS_POTION){ //포션일 경우 
			pItem->sItemInfo.PotionCount = ItemCount; //사게된 겟수의 포션을 체크한다 

		}
		ReformCharForm();//재인증 
		SendSaveMoney(); //금액 조작을 못하게하기위해 호출한다 
		if(cInvenTory.CheckRequireItemToSet(pItem)){ //일단 오토셋팅이 될수있는지 체크한다 
			if(CheckItemForm(&pItem->sItemInfo)){
				ReformItem( &pItem->sItemInfo );  //아이템 인증 받기 
			}
			else{
				SendSetHackUser(3); //해킹을 하려고했던 못된 유저를 고발 TRUE 접속 종료 
				return FALSE;
			}
			
			if(cInvenTory.AutoSetInvenItem(pItem)){ //인벤토리에 셋팅 상점에서 셋팅시에는 플랙을 준다
				cInvenTory.SetItemToChar(); //파라메터 셋팅
				if ( smConfig.DebugMode ) {
					char szDbg[128];
					extern int AddChatBuff( char *szMsg );
					wsprintf( szDbg , "BUY kept: serial %d / %d" , (int)pItem->sItemInfo.ItemHeader.Head , (int)pItem->sItemInfo.ItemHeader.dwChkSum );
					AddChatBuff( szDbg );
				}
				if(ShowShopItemIndex){
					ShowShopItem[ShowShopItemIndex-1].Flag = 0; //아이템을 없애준다 
				}
				else{
					if(BuyItemIndexCopy){
						ShowShopItem[BuyItemIndexCopy-1].Flag = 0; //아이템을 없애준다 

					}
				}
				BuyItemIndexCopy = 0;
				ShowShopItemIndex = 0; //아이템 인덱스 초기화
				cInvenTory.CheckWeight();
				if(pItem->Class == ITEM_CLASS_POTION || pItem->Class == ITEM_CLASS_ECORE) //포션일 경우 				
					pItem->Flag = 1; //포션일경우에는 아이템을 없애지 않는다
				SaveGameData(); //게임 데이타 저장 
				return TRUE;
			}
			else {
				//셋팅 실패
				CheckCharForm();//인증 				
				//sinChar->Money +=(pItem->sItemInfo.Price*ItemCount); //셋팅이 되지않을시는 돈을 다시 보정해준다 
				sinPlusMoney(pItem->sItemInfo.Price*ItemCount);
				ReformCharForm();//재인증 
				SendSaveMoney(); //금액 조작을 못하게하기위해 호출한다 

			}

		}
	}
	else
		cMessageBox.ShowMessage(MESSAGE_NOT_ENOUGH_MONEY); //돈부족 메세지 

	return FALSE;
}


//상점에 아이템을 판다 
int cSHOP::SellItemToShop(sITEM *pItem,int ItemCount)
{

	////////////////////////// 치트된 아이템이면 돈을 깍는다 
	if(!CheckItemForm(&pItem->sItemInfo)){ 
		SendSetHackUser(4); //해킹을 하려고했던 못된 유저를 고발 TRUE 접속 종료 
		pItem->Flag = 0;  //치트된 아이템을 없애버힌다 
		CheckCharForm();//인증 
		//sinChar->Money -= pItem->SellPrice*ItemCount;
		sinMinusMoney(pItem->SellPrice*ItemCount);
		if(sinChar->Money < 0 )sinChar->Money = 0;
		ReformCharForm();//재인증 
		SendSaveMoney(); //금액 조작을 못하게하기위해 호출한다
		sinPlaySound(pItem->SoundIndex);
		return FALSE;

	}
	//소지할수있는 금액을 물건을팔때두 제한한다 
	if(sinChar->Level <= 10){
		if(sinChar->Money+pItem->SellPrice*ItemCount > 200000){
			cMessageBox.ShowMessage(MESSAGE_OVER_MONEY);
			cInvenTory.ReFormInvenItem();
			return FALSE;

		}
	}
	else{
		switch(sinChar->ChangeJob){
			case 0:
				if((sinChar->Money +pItem->SellPrice*ItemCount) >=(sinChar->Level*200000)-1800000) {
					cMessageBox.ShowMessage(MESSAGE_OVER_MONEY);
					cInvenTory.ReFormInvenItem();
					return FALSE;
				}
				
			break;
			case 1:
				if( ( sinChar->Money + pItem->SellPrice*ItemCount) >=10000000 ){
					cMessageBox.ShowMessage(MESSAGE_OVER_MONEY);
					cInvenTory.ReFormInvenItem();
					return FALSE;
				}
			break;
			case 2:
				if( ( sinChar->Money + pItem->SellPrice*ItemCount) >=50000000 ){
					cMessageBox.ShowMessage(MESSAGE_OVER_MONEY);
					cInvenTory.ReFormInvenItem();
					return FALSE;
				}

			break;
			case 3:
			case 4:		// tier 5: the fifth rank
				if( ( sinChar->Money + pItem->SellPrice*ItemCount) >=100000000 ){
					cMessageBox.ShowMessage(MESSAGE_OVER_MONEY);
					cInvenTory.ReFormInvenItem();
					return FALSE;
				}

			break;


		}

	}
/*
	CheckCharForm();//인증 
	//sinChar->Money += pItem->SellPrice*ItemCount;
	sinPlusMoney(pItem->SellPrice*ItemCount);
	ReformCharForm();//재인증 
	SendSaveMoney(); //금액 조작을 못하게하기위해 호출한다
	sinPlaySound(pItem->SoundIndex);
	SendSaveThrowItem(&pItem->sItemInfo);
	pItem->Flag = 0;
	ResetInvenItemCode();		//이벤토리 아이템 코드 초기화 **************************
*/

	//투핸드 무기체크 
	if(pItem->Class == ITEM_CLASS_WEAPON_TWO && CursorClass == SIN_CURSOR_SELL) //커서로 아이템을 팔경우에만해당
		cInvenTory.PickUpTwoHandItem(pItem);

	//판매할 아이템을 서버로 보냄
	SellItemToServer( &pItem->sItemInfo , ItemCount );

	sinPlaySound(pItem->SoundIndex);
	pItem->Flag = 0;
	ResetInvenItemCode();		//이벤토리 아이템 코드 초기화 **************************

	return TRUE;
}

//상점 텍스트를 표시한다 
void cSHOP::DrawShopText()
{
	
	
	if(ReStartFlag){
		HDC	hdc;
		char strBuff[128];
		memset(strBuff,0,sizeof(strBuff));
		lpDDSBack->GetDC( &hdc );
		SelectObject( hdc, sinFont);
		SetBkMode( hdc, TRANSPARENT ); 
		SetTextColor( hdc, RGB(255,255,255) );

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
		lpDDSBack->ReleaseDC( hdc );

	}

	if(!sinMoveKindInter[SIN_SHOP] && !sinMoveKindInter[SIN_MYSHOP] && !sinMoveKindInter[SIN_CHARSHOP])return; //상점이 닫혀있으면 리턴

	HDC	hdc;
	char strBuff[128];
	memset(strBuff,0,sizeof(strBuff));
	lpDDSBack->GetDC( &hdc );
	SelectObject( hdc, sinFont);
    SetBkMode( hdc, TRANSPARENT ); 
	SetTextColor( hdc, RGB(255,255,255) );

	if(OpenFlag){   
		NumLineComa(sinChar->Money, strBuff);
		//wsprintf(strBuff,"%d",sinChar->Money); //내돈 
		dsTextLineOut(hdc,CheckEditSize(ShopGoldEdit[1][0],ShopGoldEdit[1][2], strBuff)-(256+128-sinMoveKindInter[SIN_SHOP]),ShopGoldEdit[1][1],
					strBuff,lstrlen(strBuff));

		if(ShopItemPrice){
			memset(strBuff,0,sizeof(strBuff));
			////상점 아이템 세율적용된 가격을 표시해준다.
			//--------------------------------------------------------------------------//
			#ifdef HASIEGE_MODE
				//공성전 세율적용에 의해 수정됨
				NumLineComa(haShopItemPrice(ShopItemPrice), strBuff);
			#else
				NumLineComa(ShopItemPrice, strBuff);
			#endif
			//---------------------------------------------------------------------------//
			
			//wsprintf(strBuff,"%d",ShopItemPrice); //물건 가격 
			dsTextLineOut(hdc,CheckEditSize(ShopGoldEdit[0][0],ShopGoldEdit[0][2], strBuff)-(256+128-sinMoveKindInter[SIN_SHOP]),ShopGoldEdit[0][1],
					strBuff,lstrlen(strBuff));
		}

	}
	int BackUpPosX , BackUpPosY = 0; 
	int Start = 0 , End = 0 ,Conut = 0;

	char szBuff2[64];

	if(cMyShop.OpenFlag){
		for(int i = 0 ; i < 30 ; i++){
			if(cMyShop.MyShopItem[i].Flag){      
				if(cMyShop.MyShopItem[i].Class == ITEM_CLASS_POTION){
					wsprintf(strBuff,"%d",cMyShop.MyShopItem[i].sItemInfo.PotionCount);
					dsTextLineOut(hdc,cMyShop.MyShopItem[i].x-(256+128-sinMoveKindInter[SIN_MYSHOP]) , cMyShop.MyShopItem[i].y,strBuff,lstrlen(strBuff));

				}
			
			}
		}
		/*
		wsprintf(strBuff,"%d",cMyShop.GetLimitMoney(Money2)); //소지가능 금액  
		dsTextLineOut(hdc,CheckEditSize(ShopGoldEdit[1][0],ShopGoldEdit[1][2], strBuff)-(256+128-sinMoveKindInter[SIN_MYSHOP]),ShopGoldEdit[1][1],
					strBuff,lstrlen(strBuff));

		wsprintf(strBuff,"%d",Money2 ); //판매 총액 
		dsTextLineOut(hdc,CheckEditSize(ShopGoldEdit[0][0],ShopGoldEdit[0][2], strBuff)-(256+128-sinMoveKindInter[SIN_MYSHOP]),ShopGoldEdit[0][1],
					strBuff,lstrlen(strBuff));
					*/

	}

	/*
	if(cMyShop.OpenFlag){
		for(int i = 0 ; i < 30 ; i++){
			if(cMyShop.MyShopItem[i].Flag){  
				if(cMyShop.MyShopItem[i].Class == ITEM_CLASS_POTION){
					wsprintf(strBuff,"%d",cMyShop.MyShopItem[i].sItemInfo.PotionCount);
					dsTextLineOut(hdc,cMyShop.MyShopItem[i].x , cMyShop.MyShopItem[i].y,strBuff,lstrlen(strBuff));
					Money2 += sRecvMyShop.sMyShopItem[i].Price * cMyShop.MyShopItem[i].sItemInfo.PotionCount;

				}
				else{
					Money2 += sRecvMyShop.sMyShopItem[i].Price;

				}
				
			}
		}
		wsprintf(strBuff,"%d",Money2); //내돈 
		dsTextLineOut(hdc,CheckEditSize(ShopGoldEdit[1][0],ShopGoldEdit[1][2], strBuff)-(256+128-sinMoveKindInter[SIN_SHOP]),ShopGoldEdit[1][1],
					strBuff,lstrlen(strBuff));

		wsprintf(strBuff,"%d",cMyShop.GetLimitMoney(Money2)); //물건 가격 
		dsTextLineOut(hdc,CheckEditSize(ShopGoldEdit[0][0],ShopGoldEdit[0][2], strBuff)-(256+128-sinMoveKindInter[SIN_SHOP]),ShopGoldEdit[0][1],
					strBuff,lstrlen(strBuff));

	}
	*/
	if(cCharShop.OpenFlag){ 
		for(int i = 0 ; i < 30 ; i++){
			if(cCharShop.CharShopItem[i].Flag){  
				if(cCharShop.CharShopItem[i].Class == ITEM_CLASS_POTION){
					wsprintf(strBuff,"%d",cCharShop.CharShopItem[i].sItemInfo.PotionCount);
					dsTextLineOut(hdc,cCharShop.CharShopItem[i].x-(256+128-sinMoveKindInter[SIN_CHARSHOP]) , cCharShop.CharShopItem[i].y,strBuff,lstrlen(strBuff));

				}
			}
		}
		/*
		wsprintf(strBuff,"%d",sinChar->Money); //소지가능 금액  
		dsTextLineOut(hdc,CheckEditSize(ShopGoldEdit[1][0],ShopGoldEdit[1][2], strBuff)-(256+128-sinMoveKindInter[SIN_MYSHOP]),ShopGoldEdit[1][1],
					strBuff,lstrlen(strBuff));
		*/

	}

	//홍보말을 입력한다
 	if(MyShopExpBox){   
		BackUpPosX = BackStartPos.x;
		BackUpPosY = BackStartPos.y; 

		BackStartPos.x = MyShopExpBoxX;
		BackStartPos.y = MyShopExpBoxY; 


		//요거는 타이틀 문구
		
		SelectObject( hdc, sinBoldFont);   
		SetBkMode( hdc, TRANSPARENT ); 
		SetTextColor( hdc, RGB(65,177,240) );
		dsTextLineOut(hdc,BackStartPos.x+60,BackStartPos.y+20,MyShopExpDoc7,lstrlen(MyShopExpDoc7));

		sinChatEnter = 0; //임의로 글씨가 안보이게한다
		cInterFace.ChatFlag = 0;
		hFocusWnd = hTextWnd;  
		GetWindowText( hFocusWnd , cMyShop.szDoc , 128 );

		SelectObject( hdc, sinFont);
		SetBkMode( hdc, TRANSPARENT );
		SetTextColor( hdc, RGB(255,255,255) );


		int len = 0;
		int line = 1;
		len = lstrlen(cMyShop.szDoc);

		line += (len/20);  

		for(int  i = 0; i < len ; i++){     
			memset(szBuff2,0,sizeof(szBuff2));
			while(1){
				if(cMyShop.szDoc[End] == NULL)break;
				if(cMyShop.szDoc[End] & 0x80){ //한글이면..
					End   += 2;
					Conut += 2;
				}
				else{ //영문이면..
					End++;
					Conut++;
				}
				if(Conut > 20)break;
			}
			memcpy(szBuff2 , &cMyShop.szDoc[Start],End - Start); 
/*
//해외 중국(개인상점이름관련)
#ifdef _LANGUAGE_CHINESE
			ConvertStringTHAI(szBuff2, lstrlen(szBuff2)+16);
#endif
*/
			dsTextLineOut(hdc,BackStartPos.x+32,BackStartPos.y+60+(i*20),szBuff2,lstrlen(szBuff2));

			if(cMyShop.szDoc[End] != NULL ){
				if(cMyShop.szDoc[End] == ' ')
					End += 1;
				Start = End;
				Conut = 0;

			}
			else break;
			
		}

		BackStartPos.x = BackUpPosX;
		BackStartPos.y = BackUpPosY;

	}
	lpDDSBack->ReleaseDC( hdc );
}

//상점아이템을 지운다 
void cSHOP::DeleteShopItem()
{
	if(!OpenFlag){
		if(cShop.ShopItem){
			memset(&cShop.ShopItem,0,sizeof(sITEM)*60); //아이템 정보 초기화 
		}
		if(ShowShopItem){
			memset(&ShowShopItem,0,sizeof(sITEM)*30); //아이템 정보 초기화 

		}
	}
}


//좋은 아이템은 체크를 해서 메세지박스를 띄운다
int cSHOP::CheckHighRankItem(sITEM *pItem)
{
	// pluto 광석, 수정, 룬, 레시피 추가
	DWORD Code[] = {sinOS1,sinQT1,sinSP1,sinGP1,sinQW1,sinGF1,sinMA1,sinMA2,sinSE1,sinBI1, sinBI2, sinWR1, sinDR1, sinPR1, sinPR2, sinPR3 };
	int Cnt = 0;
	while(1){
		if(!Code[Cnt])break;
		if((pItem->sItemInfo.CODE & sinITEM_MASK2) == Code[Cnt])return TRUE;
		Cnt++;

	}
	if((pItem->sItemInfo.CODE&sinITEM_MASK3) >sin30)return TRUE;
	if(pItem->sItemInfo.Level >= 41)return TRUE;
	if(pItem->sItemInfo.ItemKindCode == ITEM_KIND_CRAFT ||
		pItem->sItemInfo.ItemKindCode == ITEM_KIND_AGING )return TRUE;

	return FALSE;
}

//아이템이 셋팅될 공간을 찾는다 
int cMYSHOP::SetMyShopItem(sITEM *pItem)
{
	int i;
	int SetFlag = 0;
	int SetCount = 0; 
	
	int MyShopStartX = 21;  
	int MyShopStartY = 136-50;
	int MyShopEndX   = MyShopStartX + (22*9);
	int MyShopEndY   = MyShopStartY + (22*9);

	if(MyShopStartX <= pItem->x+11 && MyShopEndX > pItem->x && 
		MyShopStartY <=pItem->y+11 && MyShopEndY > pItem->y){ //인벤토리 박스에 들어갔는지를 체크 
			SetFlag = 1;

	}
	

	if(!SetFlag)return FALSE; 

	//셋팅해준다
	int cntx,cnty;
	int cx,cy;

	RECT	rect;
	int		flag;

	cx = (22*9)-pItem->w; //아이템이 들어갈수있는 가장 끝 X좌표 
	cy = (22*9)-pItem->h; //아이템이 들어갈수있는 가장 끝 Y좌표

	//코드가 같지 않거나 아이템이 비어있지 않은 곳에 셋팅할때 검색해서 위치를 찾아낸다 
	for( cntx=MyShopStartX ; cntx <= MyShopStartX+cx ; cntx+=22){
		for( cnty=MyShopStartY ; cnty <= MyShopStartY+cy ; cnty+=22){ 
			flag = 0;
			for( i=0 ; i<30 ; i++){
				if(MyShopItem[i].Flag){
					rect.left   = MyShopItem[i].x;
					rect.right  = MyShopItem[i].x+MyShopItem[i].w;
					rect.top    = MyShopItem[i].y;
					rect.bottom = MyShopItem[i].y+MyShopItem[i].h;

					if( cntx > rect.left-pItem->w && cntx< rect.right && 
						cnty > rect.top-pItem->h && cnty< rect.bottom )
						flag++;
				}
			}
			if ( flag==0 ) 
			{     
				if((pItem->CODE & sinITEM_MASK2) == sinPZ1 || (pItem->CODE & sinITEM_MASK2) == sinPZ2 || (pItem->CODE & sinITEM_MASK2) == sinBI1 ||
					(pItem->CODE & sinITEM_MASK2) == sinBI2 )
				{
					//if(pItem->sItemInfo.PotionCount == 1){
					cMessageBox.ShowMessage(MESSAGE_NOT_MYSHOP_ITEM);
					return TRUE;
					//}
						
				}
				else if( (pItem->CODE & sinITEM_MASK2) == sinDA1 || (pItem->CODE & sinITEM_MASK2) == sinDA2)
				{
					if( //파티복 코스튬   //패키지 캐쉬아이템
						(pItem->CODE & sinITEM_MASK3) == sin31 || (pItem->CODE & sinITEM_MASK3) == sin32 || (pItem->CODE & sinITEM_MASK3) == sin33 || 
						(pItem->CODE & sinITEM_MASK3) == sin34 || 
						(pItem->CODE & sinITEM_MASK3) == sin35 || (pItem->CODE & sinITEM_MASK3) == sin36 || (pItem->CODE & sinITEM_MASK3) == sin37 || 
						(pItem->CODE & sinITEM_MASK3) == sin38 ||
						(pItem->CODE & sinITEM_MASK3) == sin39 || (pItem->CODE & sinITEM_MASK3) == sin40 || (pItem->CODE & sinITEM_MASK3) == sin41 || 
						(pItem->CODE & sinITEM_MASK3) == sin42 ||
						(pItem->CODE & sinITEM_MASK3) == sin43 || (pItem->CODE & sinITEM_MASK3) == sin44 || (pItem->CODE & sinITEM_MASK3) == sin45 || 
						(pItem->CODE & sinITEM_MASK3) == sin46 ||
						(pItem->CODE & sinITEM_MASK3) == sin54 || (pItem->CODE & sinITEM_MASK3) == sin55 ) // 박재원 - 수영복 복장 추가 - 개인상점 금지
					{
						cMessageBox.ShowMessage(MESSAGE_NOT_MYSHOP_ITEM);
						return TRUE;
					}
				}
				else if( (pItem->CODE & sinITEM_MASK2) == sinOA2 ) // 박재원 - 슈퍼 암릿(7일, 30일) 추가
				{
					if( (pItem->CODE & sinITEM_MASK3) == sin31 || (pItem->CODE & sinITEM_MASK3) == sin32 ||
						(pItem->CODE & sinITEM_MASK3) == sin33  ||	// 장별 - 슈퍼 암릿(1일))
						(pItem->CODE & sinITEM_MASK3) == sin34 )	// 장별 - 슈퍼 암릿(1시간)
					{
						cMessageBox.ShowMessage(MESSAGE_NOT_MYSHOP_ITEM);
						return TRUE;
					}
				}
				else if( (pItem->CODE & sinITEM_MASK2) == sinDB1 )  // 박재원 - 스피드 부츠(7일, 30일) 추가
				{
					if( (pItem->CODE & sinITEM_MASK3) == sin31 || (pItem->CODE & sinITEM_MASK3) == sin32 || 
						 (pItem->CODE & sinITEM_MASK3) == sin33  || // 장별 - 스피드 부츠(1일) 추가
						  (pItem->CODE & sinITEM_MASK3) == sin34 ) // 장별 - 스피드 부츠(1시간) 추가
					{
						cMessageBox.ShowMessage(MESSAGE_NOT_MYSHOP_ITEM);
						return TRUE;
					}
				}

				else if( (pItem->CODE & sinITEM_MASK2) == sinOR2 )  // 장별 - 하트링(7일) 추가
				{
					if( (pItem->CODE & sinITEM_MASK3) == sin33 )
					{
						cMessageBox.ShowMessage(MESSAGE_NOT_MYSHOP_ITEM);
						return TRUE;
					}
				}


				else if( (pItem->CODE & sinITEM_MASK2) == sinOA1 )  // 장별 - 눈결정 목걸이(7일) 추가
				{
					if( (pItem->CODE & sinITEM_MASK3) == sin36 )
					{
						cMessageBox.ShowMessage(MESSAGE_NOT_MYSHOP_ITEM);
						return TRUE;
					}
				}


				else if( (pItem->CODE & sinITEM_MASK2) == sinOA1 )  // 장별 - 캔디데이즈 하트아뮬렛(7일) 추가
				{
					if( (pItem->CODE & sinITEM_MASK3) == sin37 )
					{
						cMessageBox.ShowMessage(MESSAGE_NOT_MYSHOP_ITEM);
						return TRUE;
					}
				}

				else if( (pItem->CODE & sinITEM_MASK2) == sinGF1 )  // 장별 - 조사원을 찾아라 아뮬렛
				{
					if( (pItem->CODE & sinITEM_MASK3) == sin07 ||  (pItem->CODE & sinITEM_MASK3) == sin08 )
					{
						cMessageBox.ShowMessage(MESSAGE_NOT_MYSHOP_ITEM);
						return TRUE;
					}
				}


				cMessageBox.ShowMessage2(MESSAGE_MYSHOP_ITEM);
				return TRUE;
			}
	
		}
	}
	cMessageBox.ShowMessage(MESSAGE_OVER_SPACE);
	return FALSE;
}

//아이템을 셋팅한다
int cMYSHOP::LastSetMyShopItem(sITEM *pItem)
{
	//셋팅해준다
	int cntx,cnty;
	int cx,cy;
	int MyShopStartX = 21;  
	int MyShopStartY = 136-50;
	int MyShopEndX   = MyShopStartX + (22*9);
	int MyShopEndY   = MyShopStartY + (22*9);

	RECT	rect;
	int		flag;

	cx = (22*9)-pItem->w; //아이템이 들어갈수있는 가장 끝 X좌표 
	cy = (22*9)-pItem->h; //아이템이 들어갈수있는 가장 끝 Y좌표

	

	//코드가 같지 않거나 아이템이 비어있지 않은 곳에 셋팅할때 검색해서 위치를 찾아낸다 
	for( cntx=MyShopStartX ; cntx <= MyShopStartX+cx ; cntx+=22){
		for( cnty=MyShopStartY ; cnty <= MyShopStartY+cy ; cnty+=22){ 
			flag = 0;
			for(int i=0 ; i<30 ; i++){
				if(MyShopItem[i].Flag){
					rect.left   = MyShopItem[i].x;
					rect.right  = MyShopItem[i].x+MyShopItem[i].w;
					rect.top    = MyShopItem[i].y;
					rect.bottom = MyShopItem[i].y+MyShopItem[i].h;

					if( cntx > rect.left-pItem->w && cntx< rect.right && 
						cnty > rect.top-pItem->h && cnty< rect.bottom )
						flag++;
				}
			}
			if ( flag==0 ) {
				for(int  j = 0 ; j < 30 ; j++){
					if(!cMyShop.MyShopItem[j].Flag){
						memcpy(&cMyShop.MyShopItem[j],pItem,sizeof(sITEM));
						cMyShop.MyShopItem[j].x = cntx;
						cMyShop.MyShopItem[j].y = cnty;
						SetShopItemToInven(pItem);
						pItem->Flag = 0;
						return TRUE;
					}
				}
			}
		}
	}
	return FALSE;
}

//아이템을 등록한다
int cMYSHOP::SetShopItemToInven(sITEM *pItem)
{
	int i = 0,j = 0;
	for(i = 0; i < INVENTORY_MAXITEM ; i++){
		if(cInvenTory.InvenItem[i].Flag){
			if(i+1 == pItem->sItemInfo.Index &&
				cInvenTory.InvenItem[i].sItemInfo.ItemHeader.Head == pItem->sItemInfo.ItemHeader.Head &&
				cInvenTory.InvenItem[i].sItemInfo.ItemHeader.dwChkSum == pItem->sItemInfo.ItemHeader.dwChkSum ){
					MyShopItemIndex[i] = 1; //등록된 아이템의 배열에 1을 셋팅 
					return TRUE;
			}
		}
	}
	return TRUE;
}

//상점에 등록되있는 아이템을 인벤토리에서 찾는다
int cMYSHOP::SearchShopItemToInven(sITEM *pItem)
{
	int i = 0,j = 0;
	for(i = 0; i < INVENTORY_MAXITEM ; i++){
		if(cInvenTory.InvenItem[i].Flag){
			if(i+1 == pItem->sItemInfo.Index &&
				cInvenTory.InvenItem[i].sItemInfo.ItemHeader.Head == pItem->sItemInfo.ItemHeader.Head &&
				cInvenTory.InvenItem[i].sItemInfo.ItemHeader.dwChkSum == pItem->sItemInfo.ItemHeader.dwChkSum 
				){
					return i+1;
			}
		}
	}
	return FALSE;
}

//서버로 아이템을 보낸다 
int cMYSHOP::SendMyShopItem()
{

	int i = 0,j = 0,index=0;

	//초기화
	memset(&sMyShop,0,sizeof(sMYSHOP)); 
	for(i = 0; i < 30 ; i++){
		if(cMyShop.MyShopItem[i].Flag){
			index = SearchShopItemToInven(&cMyShop.MyShopItem[i]);
			if(index){
				for(j = 0 ; j < 30 ;j++){
					if(!sMyShop.sMyShopItem[j].sItem.CODE){
						sMyShop.sMyShopItem[j].Price = cMyShop.MyShopItem[i].sItemInfo.Money;
						sMyShop.sMyShopItem[j].Posi[0] = cMyShop.MyShopItem[i].x;
						sMyShop.sMyShopItem[j].Posi[1] = cMyShop.MyShopItem[i].y;
						memcpy(&sMyShop.sMyShopItem[j].sItem,&cInvenTory.InvenItem[index-1].sItemInfo,sizeof(sITEMINFO));
						break;
					}
				}
			}
		}
	}

	//아이템을 서버에 보낸다
	sMyShop.CHAR_CODE = lpCurPlayer->dwObjectSerial; //캐릭터 코드
	SendOpenPersonalTrade( cMyShop.szSendDoc , &sMyShop );
	//cCharShop.RecvShopItem(&sMyShop);

	return TRUE;
}

//등록된 아이템과 서버에 보낸아이템이 같은가를 체크한다
int SerchsMyShopItem(sITEM *pItem)
{
	int i = 0,j = 0;
	for(i = 0; i < 30 ; i++){
		if(sMyShop.sMyShopItem[i].sItem.ItemHeader.Head == pItem->sItemInfo.ItemHeader.Head &&
			sMyShop.sMyShopItem[i].sItem.ItemHeader.dwChkSum == pItem->sItemInfo.ItemHeader.dwChkSum &&
			sMyShop.sMyShopItem[i].sItem.CODE == pItem->CODE){
				return i+1;
			}

	}
	return 0;

}

//서버에서 구매요청이 들어온 아이템을 받는다
int cMYSHOP::RecvMyShopItem( DWORD dwCharCode , sMYSHOP_ITEM_SERVER *lpShopItem )
{
	
	smCHAR *lpChar;
	char   szChatBuff[128];

	//CheckInvenEmpty
	int i = 0 , j = 0 , index = 0;
	memset(&sMyShop_Server,0,sizeof(sMYSHOP_ITEM_SERVER));
	DWORD 	MASK_CODE = 0;
	MASK_CODE = (lpShopItem->CODE & sinITEM_MASK2);

	int ExitItemFlag = 0;
	int index5 =0;

	switch(lpShopItem->SendFlag){
		case 1:  //구매요청 
			if(MyShopSendButton){ 
				for(i = 0; i < 30 ; i++){
					if(cMyShop.MyShopItem[i].Flag){
						index = SearchShopItemToInven(&cMyShop.MyShopItem[i]); 
						if(index){
							if(lpShopItem->CkSum == cInvenTory.InvenItem[index-1].sItemInfo.ItemHeader.dwChkSum &&
								lpShopItem->Head == cInvenTory.InvenItem[index-1].sItemInfo.ItemHeader.Head &&
								lpShopItem->CODE == cInvenTory.InvenItem[index-1].sItemInfo.CODE){

									ExitItemFlag = 1; //아이템의 존재여부
								
								//서버로보낸 아이템과 현제 등록된 아이템을 체크한다
								index5 = SerchsMyShopItem(&cMyShop.MyShopItem[i]);
								if(!index5){
									//인위적으로 데이타조작됨 
									break;

								}

								//가격이 맞지않을경우에
								if(MASK_CODE == sinPM1 || MASK_CODE == sinPL1 || MASK_CODE == sinPS1 ){
									if(lpShopItem->Price != sMyShop.sMyShopItem[index5-1].Price*lpShopItem->sTime){
										memcpy(&sMyShop_Server,lpShopItem,sizeof(sMYSHOP_ITEM_SERVER));
										sMyShop_Server.SendFlag = 7;
										sMyShop_Server.Price = sMyShop.sMyShopItem[index5-1].Price;
										Send_PersonalShopItem( sMyShop_Server.Buyer , &sMyShop_Server );
										UpdateMyShopList(&sMyShop);
										break;
									}

								}
								else{
									if(lpShopItem->Price != sMyShop.sMyShopItem[index5-1].Price){
										memcpy(&sMyShop_Server,lpShopItem,sizeof(sMYSHOP_ITEM_SERVER));
										sMyShop_Server.SendFlag = 7;
										sMyShop_Server.Price = sMyShop.sMyShopItem[index5-1].Price;
										Send_PersonalShopItem( sMyShop_Server.Buyer , &sMyShop_Server );
										UpdateMyShopList(&sMyShop);
										break;
									}
								}

								lpChar = FindAutoPlayer( dwCharCode ); //물건을 사간 유저를 찾는다
								CheckCharForm();//인증 	
								//물약
								if(MASK_CODE == sinPM1 || MASK_CODE == sinPL1 || MASK_CODE == sinPS1 ){
									if(cInvenTory.InvenItem[index-1].sItemInfo.PotionCount >= (int)lpShopItem->sTime){
										sinPlusMoney(lpShopItem->Price);
										cInvenTory.InvenItem[index-1].sItemInfo.PotionCount -=lpShopItem->sTime;
										cMyShop.MyShopItem[i].sItemInfo.PotionCount -=lpShopItem->sTime;
										sMyShop.sMyShopItem[index5-1].sItem.PotionCount -=lpShopItem->sTime;
										if(cMyShop.MyShopItem[i].sItemInfo.PotionCount == 0){
											cInvenTory.InvenItem[index-1].Flag = 0;
											cMyShop.MyShopItem[i].Flag = 0; //보여지는 부분초기화 
											memset(&sMyShop.sMyShopItem[index5-1],0,sizeof(sMYSHOP_ITEM)); //서버에보내질부분 초기화

										}
										if ( lpChar ) {
#ifdef _LANGUAGE_THAI
											wsprintf(szChatBuff,BuyMyShopItem7,lpChar->smCharInfo.szName,
												cInvenTory.InvenItem[index-1].sItemInfo.ItemName,lpShopItem->sTime,lpShopItem->Price);
#else
#ifdef	_LANGUAGE_ENGLISH		
											wsprintf(szChatBuff,BuyMyShopItem7,lpChar->smCharInfo.szName,
												lpShopItem->sTime,cInvenTory.InvenItem[index-1].sItemInfo.ItemName,lpShopItem->Price);
#else
#ifdef	_LANGUAGE_CHINESE
											wsprintf(szChatBuff,BuyMyShopItem7,lpChar->smCharInfo.szName,
												lpShopItem->sTime,cInvenTory.InvenItem[index-1].sItemInfo.ItemName,lpShopItem->Price);
#else
#ifdef	_LANGUAGE_BRAZIL		
											wsprintf(szChatBuff,BuyMyShopItem7,lpChar->smCharInfo.szName,
												lpShopItem->sTime,cInvenTory.InvenItem[index-1].sItemInfo.ItemName,lpShopItem->Price);
#else
#ifdef	_LANGUAGE_VEITNAM
											wsprintf(szChatBuff,BuyMyShopItem7,lpChar->smCharInfo.szName,
												lpShopItem->sTime,cInvenTory.InvenItem[index-1].sItemInfo.ItemName,lpShopItem->Price);
#else
#ifdef	_LANGUAGE_ARGENTINA		
											wsprintf(szChatBuff,BuyMyShopItem7,lpChar->smCharInfo.szName,
												lpShopItem->sTime,cInvenTory.InvenItem[index-1].sItemInfo.ItemName,lpShopItem->Price);
#else
											wsprintf(szChatBuff,BuyMyShopItem7,lpChar->smCharInfo.szName,
												cInvenTory.InvenItem[index-1].sItemInfo.ItemName,lpShopItem->Price,lpShopItem->sTime);
#endif
#endif
#endif
#endif
#endif
#endif
											AddChatBuff( szChatBuff , 6 );

										}
										memcpy(&sMyShop_Server,lpShopItem,sizeof(sMYSHOP_ITEM_SERVER)); 
										sMyShop_Server.SendFlag = 2;

									}
									else{
										//예외처리
										memcpy(&sMyShop_Server,lpShopItem,sizeof(sMYSHOP_ITEM_SERVER)); 
										sMyShop_Server.sTime = cInvenTory.InvenItem[index-1].sItemInfo.PotionCount;
										cMyShop.MyShopItem[i].sItemInfo.PotionCount =cInvenTory.InvenItem[index-1].sItemInfo.PotionCount;
										sMyShop.sMyShopItem[index5-1].sItem.PotionCount =cInvenTory.InvenItem[index-1].sItemInfo.PotionCount;
										sMyShop_Server.SendFlag = 6;

									}

								}
								else{
									if ( lpChar ) {
#ifdef _LANGUAGE_THAI
										wsprintf(szChatBuff,BuyMyShopItem7,lpChar->smCharInfo.szName,
											cInvenTory.InvenItem[index-1].sItemInfo.ItemName,1,lpShopItem->Price);
#else
#ifdef	_LANGUAGE_ENGLISH		
										wsprintf(szChatBuff,BuyMyShopItem7,lpChar->smCharInfo.szName,
											1,cInvenTory.InvenItem[index-1].sItemInfo.ItemName,lpShopItem->Price);
#else
#ifdef	_LANGUAGE_CHINESE
										wsprintf(szChatBuff,BuyMyShopItem7,lpChar->smCharInfo.szName,
											1,cInvenTory.InvenItem[index-1].sItemInfo.ItemName,lpShopItem->Price);
#else
#ifdef	_LANGUAGE_BRAZIL
										wsprintf(szChatBuff,BuyMyShopItem7,lpChar->smCharInfo.szName,
											1,cInvenTory.InvenItem[index-1].sItemInfo.ItemName,lpShopItem->Price);
#else
#ifdef	_LANGUAGE_VEITNAM		
										wsprintf(szChatBuff,BuyMyShopItem7,lpChar->smCharInfo.szName,
											1,cInvenTory.InvenItem[index-1].sItemInfo.ItemName,lpShopItem->Price);
#else
#ifdef	_LANGUAGE_ARGENTINA
										wsprintf(szChatBuff,BuyMyShopItem7,lpChar->smCharInfo.szName,
											1,cInvenTory.InvenItem[index-1].sItemInfo.ItemName,lpShopItem->Price);
#else
										wsprintf(szChatBuff,BuyMyShopItem7,lpChar->smCharInfo.szName,
											cInvenTory.InvenItem[index-1].sItemInfo.ItemName,lpShopItem->Price,1);
#endif
#endif
#endif
#endif
#endif
#endif
										AddChatBuff( szChatBuff , 6 );

									}
									sinPlusMoney(lpShopItem->Price);
									cInvenTory.InvenItem[index-1].Flag = 0;
									cMyShop.MyShopItem[i].Flag = 0; //보여지는 부분초기화 
									memset(&sMyShop.sMyShopItem[index5-1],0,sizeof(sMYSHOP_ITEM)); //서버에보내질부분 초기화
									memcpy(&sMyShop_Server,lpShopItem,sizeof(sMYSHOP_ITEM_SERVER)); 
									sMyShop_Server.SendFlag = 2;

								} 
								Send_PersonalShopItem( dwCharCode , &sMyShop_Server );
								UpdateMyShopList(&sMyShop);
								cInvenTory.ReFormInvenItem();
								ReformCharForm();//재인증 
								ResetInvenItemCode();
								cInvenTory.ReFormPotionNum();
								ResetPotion();
								cInvenTory.CheckWeight();
								SaveGameData();
								break;
							}
						}
					}
				}
				//아이템이 없음 
				if(!ExitItemFlag){
					memcpy(&sMyShop_Server,lpShopItem,sizeof(sMYSHOP_ITEM_SERVER));
					sMyShop_Server.SendFlag = 3;
					Send_PersonalShopItem( sMyShop_Server.Buyer , &sMyShop_Server );
				}
			}
			else{
				memcpy(&sMyShop_Server,lpShopItem,sizeof(sMYSHOP_ITEM_SERVER));
				sMyShop_Server.SendFlag = 4;
				Send_PersonalShopItem( sMyShop_Server.Buyer , &sMyShop_Server );
			}			
		break;
		case 2:	//성공 
			//받은걸 내껄루 그리고 돈뺀다
			lpChar = FindAutoPlayer( dwCharCode ); //물건을 판매한 유저를 찾는다
			index = cCharShop.SearchMyShopItem(lpShopItem->CODE , lpShopItem->Head , lpShopItem->CkSum);
			if(index){ 
				CheckCharForm();//인증 				
				//물약
				if(MASK_CODE == sinPM1 || MASK_CODE == sinPL1 || MASK_CODE == sinPS1 ){
					if ( lpChar ) {
#ifdef	_LANGUAGE_THAI	
						wsprintf(szChatBuff,BuyMyShopItem8,sRecvMyShop.sMyShopItem[index-1].sItem.ItemName,
							lpChar->smCharInfo.szName,lpShopItem->sTime,lpShopItem->Price);
#else
#ifdef	_LANGUAGE_ENGLISH		
						wsprintf(szChatBuff,BuyMyShopItem8,lpChar->smCharInfo.szName,
							lpShopItem->sTime,sRecvMyShop.sMyShopItem[index-1].sItem.ItemName,lpShopItem->Price);
#else
//중국
#ifdef	_LANGUAGE_CHINESE
						wsprintf(szChatBuff,BuyMyShopItem8,lpChar->smCharInfo.szName,
							lpShopItem->sTime,sRecvMyShop.sMyShopItem[index-1].sItem.ItemName,lpShopItem->Price);
#else
#ifdef  _LANGUAGE_BRAZIL
						wsprintf(szChatBuff,BuyMyShopItem8,lpChar->smCharInfo.szName,
							lpShopItem->sTime,sRecvMyShop.sMyShopItem[index-1].sItem.ItemName,lpShopItem->Price);
#else
#ifdef	_LANGUAGE_VEITNAM
						wsprintf(szChatBuff,BuyMyShopItem8,lpChar->smCharInfo.szName,
							lpShopItem->sTime,sRecvMyShop.sMyShopItem[index-1].sItem.ItemName,lpShopItem->Price);
#else
#ifdef  _LANGUAGE_ARGENTINA
						wsprintf(szChatBuff,BuyMyShopItem8,lpChar->smCharInfo.szName,
							lpShopItem->sTime,sRecvMyShop.sMyShopItem[index-1].sItem.ItemName,lpShopItem->Price);
#else
						wsprintf(szChatBuff,BuyMyShopItem8,lpChar->smCharInfo.szName,
							sRecvMyShop.sMyShopItem[index-1].sItem.ItemName,lpShopItem->Price,lpShopItem->sTime);
#endif
#endif
#endif
#endif
#endif
#endif
						AddChatBuff( szChatBuff , 6 );

					}

					//셋팅할 카운트 갯수 
					sRecvMyShop.sMyShopItem[index-1].sItem.PotionCount = lpShopItem->sTime;
					cCharShop.CharShopItem[index-1].sItemInfo.PotionCount -= lpShopItem->sTime;
					sinMinusMoney(lpShopItem->Price);
					cCharShop.SetCharShopItemToInven(&sRecvMyShop.sMyShopItem[index-1].sItem);
					if(cCharShop.CharShopItem[index-1].sItemInfo.PotionCount == 0){
						cCharShop.CharShopItem[index-1].Flag = 0;
						memset(&sRecvMyShop.sMyShopItem[index-1].sItem,0,sizeof(sITEMINFO));
					}
				}
				else{
					if ( lpChar ) {
#ifdef	_LANGUAGE_THAI	
						wsprintf(szChatBuff,BuyMyShopItem8,sRecvMyShop.sMyShopItem[index-1].sItem.ItemName,
							lpChar->smCharInfo.szName,1,lpShopItem->Price);
#else
#ifdef	_LANGUAGE_ENGLISH		
						wsprintf(szChatBuff,BuyMyShopItem8,lpChar->smCharInfo.szName,
							1,sRecvMyShop.sMyShopItem[index-1].sItem.ItemName,lpShopItem->Price);
#else
//중국
#ifdef	_LANGUAGE_CHINESE
						wsprintf(szChatBuff,BuyMyShopItem8,lpChar->smCharInfo.szName,
							1,sRecvMyShop.sMyShopItem[index-1].sItem.ItemName,lpShopItem->Price);
#else
#ifdef _LANGUAGE_BRAZIL
						wsprintf(szChatBuff,BuyMyShopItem8,lpChar->smCharInfo.szName,
							1,sRecvMyShop.sMyShopItem[index-1].sItem.ItemName,lpShopItem->Price);
#else
#ifdef	_LANGUAGE_VEITNAM
						wsprintf(szChatBuff,BuyMyShopItem8,lpChar->smCharInfo.szName,
							1,sRecvMyShop.sMyShopItem[index-1].sItem.ItemName,lpShopItem->Price);
#else
#ifdef _LANGUAGE_ARGENTINA
						wsprintf(szChatBuff,BuyMyShopItem8,lpChar->smCharInfo.szName,
							1,sRecvMyShop.sMyShopItem[index-1].sItem.ItemName,lpShopItem->Price);
#else
						wsprintf(szChatBuff,BuyMyShopItem8,lpChar->smCharInfo.szName,
							sRecvMyShop.sMyShopItem[index-1].sItem.ItemName,lpShopItem->Price,1);
#endif
#endif
#endif
#endif
#endif
#endif
						AddChatBuff( szChatBuff , 6 );
					}
					sinMinusMoney(lpShopItem->Price);
					cCharShop.CharShopItem[index-1].Flag = 0;
					cCharShop.SetCharShopItemToInven(&sRecvMyShop.sMyShopItem[index-1].sItem);
					memset(&sRecvMyShop.sMyShopItem[index-1].sItem,0,sizeof(sITEMINFO));

				}
				cInvenTory.ReFormInvenItem();
				ReformCharForm();//재인증 
				ResetInvenItemCode();
				cInvenTory.ReFormPotionNum();
				ResetPotion(); //ResetPotion2 가이나라 ResetPotion으로 해준다 
				cInvenTory.CheckWeight();
				SaveGameData();

			}
		break;
		case 3: //ex) 팔림
			cMessageBox.ShowMessage(MESSAGE_MYSHOP_ITEM_DOWN);
		break;
		case 4: //기타 등등
			cMessageBox.ShowMessage(MESSAGE_MYSHOP_CLOSE);
		break;
		case 5: //판매자를 찾을수없음
			cMessageBox.ShowMessage(MESSAGE_MYSHOP_NOT_SEARCH);
		break;
		case 6: //물약갯수가 맞지않음
			index = cCharShop.SearchMyShopItem(lpShopItem->CODE , lpShopItem->Head , lpShopItem->CkSum);
			if(index){
				//물약
				if(MASK_CODE == sinPM1 || MASK_CODE == sinPL1 || MASK_CODE == sinPS1 ){
					sRecvMyShop.sMyShopItem[index-1].sItem.PotionCount = lpShopItem->sTime;
					cCharShop.CharShopItem[index-1].sItemInfo.PotionCount = lpShopItem->sTime;
					cMessageBox.ShowMessage(MESSAGE_MYSHOP_POTION_NUM2);
				}
			}
		break;
		case 7: //가격변동
			index = cCharShop.SearchMyShopItem(lpShopItem->CODE , lpShopItem->Head , lpShopItem->CkSum);
			if(index){ 
				sRecvMyShop.sMyShopItem[index-1].Price = lpShopItem->Price;
				cCharShop.CharShopItem[index-1].sItemInfo.Price = lpShopItem->Price;
				cMessageBox.ShowMessage(MESSAGE_MYSHOP_CHANGE_PRICE);
			}
		break;

	}

	return TRUE;
}

int cCHARSHOP::RecvShopItem(sMYSHOP *sMyShop)
{
	
	char szFilePath[256];
	sITEM TempItem;
	for(int i=0; i < 30 ; i++){  
		if(sMyShop->sMyShopItem[i].Price){ //일단은 가격으로 체크
			for(int j=0 ; j < MAX_ITEM ; j++){
				if(sMyShop->sMyShopItem[i].sItem.CODE == sItem[j].CODE){
					wsprintf(szFilePath ,"Image\\sinImage\\Items\\%s\\it%s.bmp",sItem[j].ItemFilePath,sItem[j].LastCategory);
					memcpy(&TempItem ,&sItem[j],sizeof(sITEM));  //구조체에 정의된 정보를 저장한다 

					
					//복주 머니 관련
				   if((sMyShop->sMyShopItem[i].sItem.CODE & sinITEM_MASK2) == sinPZ1 || (sMyShop->sMyShopItem[i].sItem.CODE & sinITEM_MASK2) == sinPZ2){
						if(sMyShop->sMyShopItem[i].sItem.PotionCount==2){
							wsprintf(szFilePath ,"Image\\sinImage\\Items\\%s\\it%s.bmp",sItem[j].ItemFilePath,sItem[j].LastCategory);
							memcpy(&TempItem ,&sItem[j],sizeof(sITEM));  //구조체에 정의된 정보를 저장한다 
							sItem[j].lpTempItem = LoadDibSurfaceOffscreen(szFilePath); //이미지를 먼저읽어준다
						}
						
					}

					memcpy(&TempItem.sItemInfo ,&sMyShop->sMyShopItem[i].sItem,sizeof(sITEMINFO)); //일단 템프에 저장한다 
					if(!sItem[j].lpTempItem) //이미지가 없으면 로드한다
						sItem[j].lpTempItem = LoadDibSurfaceOffscreen(szFilePath);
					TempItem.lpItem = sItem[j].lpTempItem; //템프에 있던 이미지 포인터를 넣어준다 
					TempItem.x = sMyShop->sMyShopItem[i].Posi[0];
					TempItem.y = sMyShop->sMyShopItem[i].Posi[1];
					memcpy(&CharShopItem[i],&TempItem,sizeof(sITEM));
					CharShopItem[i].Flag = 1;
					break;

				}
			}
		}
	}
	memcpy(&sRecvMyShop,sMyShop,sizeof(sMYSHOP));
	cInterFace.CheckAllBox(SIN_CHARSHOP);
	return TRUE;
}


//받은 아이템을 찾아서 인덱스를 리턴한다
int cCHARSHOP::SearchMyShopItem(DWORD CODE ,DWORD Head , DWORD CheckSum)
{
	for(int i = 0 ; i < 30 ; i++){
		if(sRecvMyShop.sMyShopItem[i].sItem.ItemHeader.Head == Head &&
			sRecvMyShop.sMyShopItem[i].sItem.ItemHeader.dwChkSum == CheckSum &&
			sRecvMyShop.sMyShopItem[i].sItem.CODE == CODE ){
				return i+1;

		}
	}
	return FALSE;
	
}

//개인상점에서산 아이템을 인벤토리로 셋팅한다
int cCHARSHOP::SetCharShopItemToInven(sITEMINFO *pItem_Info)
{
	char szTestBuff2[256];
	sITEM TempItem;
	szTestBuff2[0] = 0; //버퍼초기화  
	memset(&TempItem,0,sizeof(sITEM));
	for(int j=0 ; j < MAX_ITEM ; j++){ 
		if(pItem_Info->CODE == sItem[j].CODE){	
			wsprintf(szTestBuff2 ,"Image\\sinImage\\Items\\%s\\it%s.bmp",sItem[j].ItemFilePath,sItem[j].LastCategory);
			if(!sItem[j].lpTempItem)
				sItem[j].lpTempItem = LoadDibSurfaceOffscreen(szTestBuff2);
			memcpy(&TempItem,&sItem[j],sizeof(sITEM));
			memcpy(&TempItem.sItemInfo,pItem_Info,sizeof(sITEMINFO));
			TempItem.lpItem = sItem[j].lpTempItem;
			TempItem.Flag =1;
			cInvenTory.CheckRequireItemToSet(&TempItem); 
			cInvenTory.AutoSetInvenItem(&TempItem);
			cInvenTory.ReFormInvenItem(); //아이템의 데이타를 셋팅한다 
			break;
		}
	}
	return TRUE;
}

//아아템을 산다
int cCHARSHOP::BuyItem(int Index)
{
	
	int Index2 = 0; 
	DWORD MASK_CODE = 0;
	memset(&MyShopPotion,0,sizeof(sITEMINFO));
	Index2 = cCharShop.SearchMyShopItem(cCharShop.CharShopItem[Index].sItemInfo.CODE , cCharShop.CharShopItem[SelectMyShopItemIndex-1].sItemInfo.ItemHeader.Head ,cCharShop.CharShopItem[Index].sItemInfo.ItemHeader.dwChkSum);
	if(Index2){
		sMyShop_Server.Buyer = lpCurPlayer->dwObjectSerial;
		sMyShop_Server.CkSum = sRecvMyShop.sMyShopItem[Index2-1].sItem.ItemHeader.dwChkSum;
		sMyShop_Server.Head  = sRecvMyShop.sMyShopItem[Index2-1].sItem.ItemHeader.Head;
		sMyShop_Server.Price = sRecvMyShop.sMyShopItem[Index2-1].Price;
		sMyShop_Server.sTime = sRecvMyShop.sMyShopItem[Index2-1].sItem.ItemHeader.dwTime;
		sMyShop_Server.CODE  = sRecvMyShop.sMyShopItem[Index2-1].sItem.CODE;
		sMyShop_Server.SendFlag = 1;
		TempItem.CODE = sRecvMyShop.sMyShopItem[Index2-1].sItem.CODE;

		//여기는 포션
		MASK_CODE = (TempItem.CODE & sinITEM_MASK2); 
		if(MASK_CODE == sinPM1 || MASK_CODE == sinPL1 || MASK_CODE == sinPS1 ){
			if(cInvenTory.CheckInvenEmpty(&TempItem)){
				memcpy(&MyShopPotion,&sRecvMyShop.sMyShopItem[Index2-1].sItem,sizeof(sITEMINFO));
				MyShopPotion.ItemHeader.dwVersion = sRecvMyShop.sMyShopItem[Index2-1].Price; //버전에 가격을넣는다
				MyShopPotion.PotionCount = cCharShop.CharShopItem[Index].sItemInfo.PotionCount; //복잡하니까 건들지말자
				cMessageBox.ShowMessage2(MESSAGE_MYSHOP_POTION);
				return TRUE;
			}
			else{
				cMessageBox.ShowMessage(MESSAGE_OVER_SPACE);

			}
		}

		//여기는 일반 아이템 
		if( (sinChar->Money - (int)sMyShop_Server.Price) >=0 ){ 
			if(cInvenTory.CheckInvenEmpty(&TempItem)){
				if(sinChar->Weight[0]+sRecvMyShop.sMyShopItem[Index2-1].sItem.Weight <= sinChar->Weight[1] ){
					cMessageBox.ShowMessage3(MESSAGE_MYSHOP_ITEM_BUY,sRecvMyShop.sMyShopItem[Index2-1].sItem.ItemName);
				}
				else{
					cMessageBox.ShowMessage(MESSAGE_OVER_WEIGHT);
				}
			}
			else{
				cMessageBox.ShowMessage(MESSAGE_OVER_SPACE);

			}
		}
		else{
			cMessageBox.ShowMessage(MESSAGE_NOT_ENOUGH_MONEY);
		}

	}
	return TRUE;
}

//소지가능한 금액을 리턴한다
int cMYSHOP::GetLimitMoney(int Money)
{

	int CheckMoney = 0;  
	if(sinChar->Level <= 10){
		 return 200000 - (Money - sinChar->Money);

	}
	else{
		switch(sinChar->ChangeJob){
			case 0:
				return (((sinChar->Level*200000)-1800000) - (Money + sinChar->Money));
					
			break;

			case 1:
				return ((10000000) - (Money + sinChar->Money));
				if( ( sinChar->Money + Money) > 10000000 )return TRUE;
			break;
			case 2:
				return ((50000000) - (Money + sinChar->Money));
			break;
			case 3:
			case 4:		// tier 5: the fifth rank (it got no room at all)
				return ((100000000) - (Money + sinChar->Money));
			break;

		}
	}
	return FALSE;
}
//물건이 다팔리면 상점을 닫는다
int cMYSHOP::AutoCloseShop()
{
	int i = 0;
	int CheckFlag = 0;  
	if(MyShopSendButton){   
		CheckFlag = 1;
		for(i = 0 ; i < 30 ; i++){
			if(cMyShop.MyShopItem[i].Flag){
				CheckFlag = 2;

			}
		}
	}
	//상점을닫는다
	if(MyShopSendButton){
		if(CheckFlag == 1){
			cMyShop.OpenFlag = 0;
			memset(cMyShop.MyShopItem,0,sizeof(sITEM)*30); //아이템을 지워준다
			memset(MyShopItemIndex,0,sizeof(int)*100);
			MyShopSendButton = 0; //등록버튼 초기화
			SendClosePersonalTrade();
		}
	}
	return TRUE;
}

//개인상점의 총액을 구해온다 
int cMYSHOP::GetTotalMyShopItemMoney()
{
	int TotalMoney = 0;
	if(cMyShop.OpenFlag){
		for(int i=0 ; i < 30 ; i++){ 
			if(cMyShop.MyShopItem[i].Flag){
				if(cMyShop.MyShopItem[i].Class == ITEM_CLASS_POTION){
					TotalMoney +=cMyShop.MyShopItem[i].sItemInfo.Money*cMyShop.MyShopItem[i].sItemInfo.PotionCount;
				}
				else{
					TotalMoney +=cMyShop.MyShopItem[i].sItemInfo.Money;
				}
			}
		}
	}
	return TotalMoney;
}
/*----------------------------------------------------------------------------*
*	             공성전 세율변경에 의한 함수 
*-----------------------------------------------------------------------------*/
//<ha>물건을 구입할수 있는지 체크한다.
int cSHOP::haBuyMoneyCheck(int BuyMoney)
{
	//캐릭터 머니와 사용될 가격을 비교한다.
	if(sinChar->Money-(BuyMoney+(BuyMoney*cSinSiege.GetTaxRate())/100)>=0){
		return TRUE;
	}
	return FALSE;
}
//<ha>상점 아이템 세율적용된 가격을 표시해준다.
int cSHOP::haShopItemPrice(int Money)
{
	
	if(Money){
		Money+=((Money*cSinSiege.GetTaxRate())/100);
		return Money; 
	}
	return FALSE;
}