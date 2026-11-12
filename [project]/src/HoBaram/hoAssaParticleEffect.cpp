/*----------------------------------------------------------------------------*
*	파일명 :  hoAssaSkillEffect.cpp	
*	하는일 :  Assa 파티클 이펙트 
*	작성일 :  최종업데이트 2002년 12월
*	적성자 :  홍호동
*-----------------------------------------------------------------------------*/	

#include "..\\SinBaram\\SinLinkHeader.h"
#include "..\\SinBaram\\AssaEffect.h"
#include "hoAssaParticleEffect.h"

#include "HoNewParticleLinkHeader.h"
#include "AssaParticle.h"

#include "ParkFireCrackerTable.h"

//#define _PARK_DEBUG_
#ifdef _PARK_DEBUG_
int AssaTest_Shot(POINT3D *pPosi, int n)
{

	////chrEachMaster  적위치
	////lpCurPlayer  내위치

	switch(n)
	{
	case 0:
 		if(chrEachMaster)
			Mokova_SkillAttack(lpCurPlayer);
		break;
	case 1:
		if(lpCurPlayer)
			
		break;
	case 2:
		if(lpCurPlayer)
			KingSpider_SkillAttack(lpCurPlayer);
		break;
	case 3:
		if(lpCurPlayer)
			Seto_SkillAttack(lpCurPlayer);
		break;
	case 4:
		if(chrEachMaster)
			Mokova_NomalAttack(chrEachMaster);
		//StartEffect( lpCurPlayer->pX,lpCurPlayer->pY+48*fONE,lpCurPlayer->pZ, EFFECT_RETURN1 );
		//ParkAssaParticle_MonsterIceValentoBody(chrEachMaster);
		break;
	case 5:
		
		//ParkAssaParticle_ChaosKara2(chrEachMaster);
		break;
	case 6:
		
		//ParkAssaParticle_ChaosKaraSkill_Monster(chrEachMaster);
		break;
	case 7:
		
		
		//ParkAssaParticle_ChaosKaraSkill_User(lpCurPlayer);
		//ParkAssaParticle_MonsterIceValentoBody(chrEachMaster);
		break;
	case 8:
		
		//Valento_Body(chrEachMaster,60000);
		//ParkAssaParticle_MonsterIceValentoBody(chrEachMaster);
		break;
	}
	
	return true;
}
#endif

/*----------------------------------------------------------------------------*
* Line파티클이 모이는 함수.
* pPosi: 파티클이 모일 위치.
* texName: 파티클 텍스쳐.
* continueTime: 파티클이 나올 시간..
* speed; 파티클의 속도..
*-----------------------------------------------------------------------------*/
int AssaParticle_LineIn(POINT3D *pPosi, char *texName, int continueTime, int speed)
{
	POINT3D curPos;
	POINT3D rand;
   	for(int index = 0; index < continueTime*4; index++)
	{
		//파티클이 나올 영역.. 
        rand.x = GetRandomPos(-18000/1000, 18000/1000);
		rand.y = GetRandomPos(-18000/1000, 18000/1000);
		rand.z = GetRandomPos(-18000/1000, 18000/1000);
		
		double length = sqrt((float)rand.x*(float)rand.x+
			                 (float)rand.y*(float)rand.y+
  	 						 (float)rand.z*(float)rand.z);
		
	 	int range = GetRandomPos(20000, 21000);
 		curPos.x = int(((float)rand.x/length)*(float)range+(float)pPosi->x);
		curPos.y = int(((float)rand.y/length)*(float)range+(float)pPosi->y);
		curPos.z = int(((float)rand.z/length)*(float)range+(float)pPosi->z);

     	if(texName == NULL)
			Assa = SetAssaEffect( int(float(range)/(400.f+(float)speed)), "flare.tga", 0, &curPos, 0);
		else
			Assa = SetAssaEffect( int(float(range)/(400.f+(float)speed)), texName, 0, &curPos, 0);

		
		cAssaEffect[Assa]->FACE_TYPE = ASSAFACE_SCALE;
		
		length =  sqrt((double)((rand.x)*(rand.x)+
					   (rand.y)*(rand.y)+
  				       (rand.z)*(rand.z)));
		
  		if(length == 0)
			length = 1;
 		
        cAssaEffect[Assa]->MOVECODE = ASSA_MOVE;
 		cAssaEffect[Assa]->Face.r = 50;
		cAssaEffect[Assa]->Face.g = 100;
		cAssaEffect[Assa]->Face.b = 255;
		cAssaEffect[Assa]->Face.Transparency = 255;
 		cAssaEffect[Assa]->Time = -(index/2);
  	 	cAssaEffect[Assa]->MoveSpeed.x = -int(float(rand.x)/length*(400.f+(float)speed));
		cAssaEffect[Assa]->MoveSpeed.y = -int(float(rand.y)/length*(400.f+(float)speed));

		cAssaEffect[Assa]->MoveSpeed.z = -int(float(rand.z)/length*(400.f+(float)speed));
		
		//파티클의 넓이..
 		cAssaEffect[Assa]->Size.w = 500;

		//파티클의 길이..
		POINT3D inScale;
  		inScale.x = int((rand.x)/(float)length*8000.f);
		inScale.y = int((rand.y)/(float)length*8000.f);
		inScale.z = int((rand.z)/(float)length*8000.f);
		
		cAssaEffect[Assa]->ScalePosi.x = -inScale.x;
		cAssaEffect[Assa]->ScalePosi.y = -inScale.y;
		cAssaEffect[Assa]->ScalePosi.z = -inScale.z;
	}

	return TRUE;
}


/*----------------------------------------------------------------------------*
* Line파티클이 나가는 함수.
* pPosi: 파티클이 나올 위치.
* texName: 파티클 텍스쳐.
* continueTime: 파티클이 나올 시간..
* speed; 파티클의 속도..
*-----------------------------------------------------------------------------*/	
int AssaParticle_LineOut(POINT3D *pPosi , char *texName, int continueTime, int speed)
{
    int delay = 20;
	POINT3D curPos;
	POINT3D rand;
	for(int index = 0; index < continueTime*4; index++)
	{
		//파티클이 나올 영역..
 		rand.x = GetRandomPos(-10000, 10000)/100;
		rand.y = GetRandomPos(-10000, 10000)/100;
		rand.z = GetRandomPos(-10000, 10000)/100;
		
		double length = sqrt((float)rand.x*(float)rand.x+
			                 (float)rand.y*(float)rand.y+
							 (float)rand.z*(float)rand.z);
		
 		int range = GetRandomPos(5000,7000);
     	curPos.x = int(float(rand.x)/length*(float)range+(float)pPosi->x);
		curPos.y = int(float(rand.y)/length*(float)range+(float)pPosi->y);
		curPos.z = int(float(rand.z)/length*(float)range+(float)pPosi->z);
		
 		if(texName == NULL)
			Assa = SetAssaEffect(150-(speed/2),"flare.tga",0,&curPos,0);                
		else
			Assa = SetAssaEffect(150-(speed/2),texName,0,&curPos,0);                
		
  		cAssaEffect[Assa]->FACE_TYPE = ASSAFACE_SCALE;
  		cAssaEffect[Assa]->Time = GetRandomPos(-2,0)-delay;
		
		if(length == 0)
			length = 1;
 		
  		cAssaEffect[Assa]->MOVECODE = ASSA_MOVE;
  	 	cAssaEffect[Assa]->MoveSpeed.x = int(float(rand.x)/length*(200.f+speed*3));
		cAssaEffect[Assa]->MoveSpeed.y = int(float(rand.y)/length*(200.f+speed*3));
		cAssaEffect[Assa]->MoveSpeed.z = int(float(rand.z)/length*(200.f+speed*3));
		
		//파티클의 넓이..
 		cAssaEffect[Assa]->Size.w = 1000;
		cAssaEffect[Assa]->Face.r = 50;
		cAssaEffect[Assa]->Face.g = 100;
		cAssaEffect[Assa]->Face.b = 255;

		//파티클의 길이..
		POINT3D destPos;
  		destPos.x = int(float(rand.x)/length*10000.f);
		destPos.y = int(float(rand.y)/length*10000.f);
		destPos.z = int(float(rand.z)/length*10000.f);
		
		cAssaEffect[Assa]->ScalePosi.x = -destPos.x;
		cAssaEffect[Assa]->ScalePosi.y = -destPos.y;
		cAssaEffect[Assa]->ScalePosi.z = -destPos.z;
	}
	
	return TRUE;
}

void SkillGodBless(smCHAR *pChar)
{
	POINT3D pos;
	memcpy(&pos, &pChar->Posi, sizeof(POINT3D));
   	g_NewParticleMgr.Start("GodlyShield", pos, 0.3f);
	pos.y += 8000;
	
	POINT3D curPos;
	POINT3D rand;


	//모이는 파티클..
    for(int index = 0; index < 20; index++)
	{
		//파티클이 나올 영역.. 
        rand.x = GetRandomPos(-18, 18);
		rand.y = GetRandomPos(-18, 18);
		rand.z = GetRandomPos(-18, 18);
		
		double length = sqrt((float)rand.x*(float)rand.x+(float)rand.y*(float)rand.y+(float)rand.z*(float)rand.z);
		
		if(length == 0)
			length = 1;
	 	int range = GetRandomPos(19000, 20000);
 		curPos.x = int(((float)rand.x/length)*(float)range+(float)pos.x);
		curPos.y = int(((float)rand.y/length)*(float)range+(float)pos.y);
		curPos.z = int(((float)rand.z/length)*(float)range+(float)pos.z);
		
		Assa = SetAssaEffect( 80, "flare.tga", 0, &curPos, 0);
		cAssaEffect[Assa]->FACE_TYPE = ASSAFACE_SCALE;
		
		cAssaEffect[Assa]->MOVECODE = ASSA_MOVE;
		

 		cAssaEffect[Assa]->Face.r = 100;
		cAssaEffect[Assa]->Face.g = 180;
		cAssaEffect[Assa]->Face.b = 230;
		cAssaEffect[Assa]->Face.Transparency = 255;
 		cAssaEffect[Assa]->Time = -(index/2);
  	 	cAssaEffect[Assa]->MoveSpeed.x = -int(float(rand.x)/length*400.f);
		cAssaEffect[Assa]->MoveSpeed.y = -int(float(rand.y)/length*400.f);
		cAssaEffect[Assa]->MoveSpeed.z = -int(float(rand.z)/length*400.f);
		
		//파티클의 넓이..
 		cAssaEffect[Assa]->Size.w = 400;

		//파티클의 길이..
		POINT3D inScale;
  		inScale.x = int((rand.x)/(float)length*8000.f);
		inScale.y = int((rand.y)/(float)length*8000.f);
		inScale.z = int((rand.z)/(float)length*8000.f);
		
		cAssaEffect[Assa]->ScalePosi.x = -inScale.x;
		cAssaEffect[Assa]->ScalePosi.y = -inScale.y;
		cAssaEffect[Assa]->ScalePosi.z = -inScale.z;
	}
	
	//나가는 파티클..
    for(int index = 0; index < 40; index++)
	{
		//파티클이 나올 영역..
   		rand.x = GetRandomPos(-100, 100);
		rand.y = GetRandomPos(-100, 100);
		rand.z = GetRandomPos(-100, 100);
		
		double length = sqrt((float)rand.x*(float)rand.x+(float)rand.y*(float)rand.y+(float)rand.z*(float)rand.z);
		
		if(length == 0)
			length = 1;
		
 		int range = GetRandomPos(5000,7000);
     	curPos.x = int(float(rand.x)/length*(float)range+(float)pos.x);
		curPos.y = int(float(rand.y)/length*(float)range+(float)pos.y);
		curPos.z = int(float(rand.z)/length*(float)range+(float)pos.z);
		
		Assa = SetAssaEffect(200,"flare.tga",0,&curPos,0);                
		
 		cAssaEffect[Assa]->FACE_TYPE = ASSAFACE_SCALE;
   		cAssaEffect[Assa]->Time = -(15+index/2);
		
		cAssaEffect[Assa]->MOVECODE = ASSA_MOVE;
  	 	cAssaEffect[Assa]->MoveSpeed.x = int(float(rand.x)/length*(150.f));
		cAssaEffect[Assa]->MoveSpeed.y = int(float(rand.y)/length*(150.f));
		cAssaEffect[Assa]->MoveSpeed.z = int(float(rand.z)/length*(150.f));
		
		//파티클의 넓이..
 		cAssaEffect[Assa]->Size.w = 500;
		cAssaEffect[Assa]->Face.r = 100;
		cAssaEffect[Assa]->Face.g = 180;
		cAssaEffect[Assa]->Face.b = 230;
		cAssaEffect[Assa]->Face.Transparency = 255;

		//파티클의 길이..
		POINT3D destPos;
   		destPos.x = int(float(rand.x)/length*12000.f);
		destPos.y = int(float(rand.y)/length*12000.f);
		destPos.z = int(float(rand.z)/length*12000.f);
		
		cAssaEffect[Assa]->ScalePosi.x = -destPos.x;
		cAssaEffect[Assa]->ScalePosi.y = -destPos.y;
		cAssaEffect[Assa]->ScalePosi.z = -destPos.z;
	}

}

int AssaParticle_HolyValor_Jang(smCHAR *pChar, int maxTime)
{
	if(GetAssaCodeEffect(pChar, DRASTIC_ACTION))
		return FALSE;

	POINT3D pos;
	memcpy(&pos, &pChar->Posi, sizeof(POINT3D));
   	pos.y += 8000;
	
	POINT3D curPos;
	POINT3D rand;
	//모이는 파티클..
    for(int index = 0; index < 20; index++)
	{
		//파티클이 나올 영역.. 
        rand.x = GetRandomPos(-18, 18);
		rand.y = GetRandomPos(-18, 18);
		rand.z = GetRandomPos(-18, 18);
		
		double length = sqrt((float)rand.x*(float)rand.x+(float)rand.y*(float)rand.y+(float)rand.z*(float)rand.z);
		
		if(length == 0)
			length = 1;
	 	int range = GetRandomPos(19000, 20000);
 		curPos.x = int(((float)rand.x/length)*(float)range+(float)pos.x);
		curPos.y = int(((float)rand.y/length)*(float)range+(float)pos.y);
		curPos.z = int(((float)rand.z/length)*(float)range+(float)pos.z);
		
		Assa = SetAssaEffect( 80, "flare.tga", 0, &curPos, 0);
		cAssaEffect[Assa]->FACE_TYPE = ASSAFACE_SCALE;
		
		cAssaEffect[Assa]->MOVECODE = ASSA_MOVE;
		

 		cAssaEffect[Assa]->Face.r = 100;
		cAssaEffect[Assa]->Face.g = 180;
		cAssaEffect[Assa]->Face.b = 230;
		cAssaEffect[Assa]->Face.Transparency = 255;
 		cAssaEffect[Assa]->Time = -(index/2);
  	 	cAssaEffect[Assa]->MoveSpeed.x = -int(float(rand.x)/length*400.f);
		cAssaEffect[Assa]->MoveSpeed.y = -int(float(rand.y)/length*400.f);
		cAssaEffect[Assa]->MoveSpeed.z = -int(float(rand.z)/length*400.f);
		
		//파티클의 넓이..
 		cAssaEffect[Assa]->Size.w = 400;

		//파티클의 길이..
		POINT3D inScale;
  		inScale.x = int((rand.x)/(float)length*8000.f);
		inScale.y = int((rand.y)/(float)length*8000.f);
		inScale.z = int((rand.z)/(float)length*8000.f);
		
		cAssaEffect[Assa]->ScalePosi.x = -inScale.x;
		cAssaEffect[Assa]->ScalePosi.y = -inScale.y;
		cAssaEffect[Assa]->ScalePosi.z = -inScale.z;
	}
	
	//나가는 파티클..
    for(int index = 0; index < 40; index++)
	{
		//파티클이 나올 영역..
   		rand.x = GetRandomPos(-100, 100);
		rand.y = GetRandomPos(-100, 100);
		rand.z = GetRandomPos(-100, 100);
		
		double length = sqrt((float)rand.x*(float)rand.x+(float)rand.y*(float)rand.y+(float)rand.z*(float)rand.z);
		
		if(length == 0)
			length = 1;
		
 		int range = GetRandomPos(5000,7000);
     	curPos.x = int(float(rand.x)/length*(float)range+(float)pos.x);
		curPos.y = int(float(rand.y)/length*(float)range+(float)pos.y);
		curPos.z = int(float(rand.z)/length*(float)range+(float)pos.z);
		
		Assa = SetAssaEffect(200,"flare.tga",0,&curPos,0);                
		
 		cAssaEffect[Assa]->FACE_TYPE = ASSAFACE_SCALE;
   		cAssaEffect[Assa]->Time = -(15+index/2);
		
		cAssaEffect[Assa]->MOVECODE = ASSA_MOVE;
  	 	cAssaEffect[Assa]->MoveSpeed.x = int(float(rand.x)/length*(150.f));
		cAssaEffect[Assa]->MoveSpeed.y = int(float(rand.y)/length*(150.f));
		cAssaEffect[Assa]->MoveSpeed.z = int(float(rand.z)/length*(150.f));
		
		//파티클의 넓이..
 		cAssaEffect[Assa]->Size.w = 500;
		cAssaEffect[Assa]->Face.r = 100;
		cAssaEffect[Assa]->Face.g = 180;
		cAssaEffect[Assa]->Face.b = 230;
		cAssaEffect[Assa]->Face.Transparency = 255;

		//파티클의 길이..
		POINT3D destPos;
   		destPos.x = int(float(rand.x)/length*12000.f);
		destPos.y = int(float(rand.y)/length*12000.f);
		destPos.z = int(float(rand.z)/length*12000.f);
		
		cAssaEffect[Assa]->ScalePosi.x = -destPos.x;
		cAssaEffect[Assa]->ScalePosi.y = -destPos.y;
		cAssaEffect[Assa]->ScalePosi.z = -destPos.z;
	}
	
	//같은 캐릭터가 유지 스킬을 쓰면 없애준다.
	StopAssaCodeEffect(pChar, HOLY_ACTION);

	//실제로 유지되는 파티클 호출..
 	int i = GetAssaEffect();
 	cASSAChar *partChar = new cASSAChar;
	partChar->Start(pChar, HOLY_VALOR_JANG, maxTime*70);
	cAssaEffect[i] = partChar;

	
	return TRUE;
}


int AssaParticle_HolyValor_Member(smCHAR *pChar, int maxTime)
{
	//같은 캐릭터가 유지 스킬을 쓰면 없애준다.
	if(GetAssaCodeEffect(pChar, DRASTIC_ACTION))
		return FALSE;

 	StopAssaCodeEffect(pChar, HOLY_ACTION);

 	//실제로 유지되는 파티클 호출..
 	int i = GetAssaEffect();
 	cASSAChar *partChar = new cASSAChar;
	partChar->Start(pChar, HOLY_VALOR_MEMBER, maxTime*70);
	cAssaEffect[i] = partChar;
	return TRUE;
}

int AssaParticle_DrasticSpirit_Jang(smCHAR *pChar, int maxTime)
{
	POINT3D pos;
    memcpy(&pos, &pChar->Posi, sizeof(POINT3D));
  	

 	//Assa = SetAssaEffect(0,"DrasticSpirit.ASE",0,&pos,0);
  	//cAssaEffect[Assa]->Angle.y = (pChar->Angle.y+ANGLE_180)&ANGCLIP;
	//cAssaEffect[Assa]->AniMaxCount = 30;
	//cAssaEffect[Assa]->AniDelayTime = 5;
	
 	pos.y += 7000;
	POINT3D curPos;
	POINT3D rand;

	//모이는 파티클..
    for(int index = 0; index < 20; index++)
	{
		//파티클이 나올 영역.. 
        rand.x = GetRandomPos(-18, 18);
		rand.y = GetRandomPos(-18, 18);
		rand.z = GetRandomPos(-18, 18);
		
		double length = sqrt((float)rand.x*(float)rand.x+(float)rand.y*(float)rand.y+(float)rand.z*(float)rand.z);
		
		if(length == 0)
			length = 1;
	 	int range = GetRandomPos(19000, 20000);
 		curPos.x = int(((float)rand.x/length)*(float)range+(float)pos.x);
		curPos.y = int(((float)rand.y/length)*(float)range+(float)pos.y);
		curPos.z = int(((float)rand.z/length)*(float)range+(float)pos.z);
		
		Assa = SetAssaEffect( 80, "flare.tga", 0, &curPos, 0);
		cAssaEffect[Assa]->FACE_TYPE = ASSAFACE_SCALE;
		
 		cAssaEffect[Assa]->MOVECODE = ASSA_MOVE;
  		cAssaEffect[Assa]->Face.r = 130;
		cAssaEffect[Assa]->Face.g = 100;
		cAssaEffect[Assa]->Face.b = 220;
		cAssaEffect[Assa]->Face.Transparency = 200;
 		cAssaEffect[Assa]->Time = -(index/2);
  	 	cAssaEffect[Assa]->MoveSpeed.x = -int(float(rand.x)/length*400.f);
		cAssaEffect[Assa]->MoveSpeed.y = -int(float(rand.y)/length*400.f);
		cAssaEffect[Assa]->MoveSpeed.z = -int(float(rand.z)/length*400.f);
		
		//파티클의 넓이..
 		cAssaEffect[Assa]->Size.w = 400;

		//파티클의 길이..
		POINT3D inScale;
  		inScale.x = int((rand.x)/(float)length*8000.f);
		inScale.y = int((rand.y)/(float)length*8000.f);
		inScale.z = int((rand.z)/(float)length*8000.f);
		
		cAssaEffect[Assa]->ScalePosi.x = -inScale.x;
		cAssaEffect[Assa]->ScalePosi.y = -inScale.y;
		cAssaEffect[Assa]->ScalePosi.z = -inScale.z;
	}
	
	//나가는 파티클..
    for(int index = 0; index < 40; index++)
	{
		//파티클이 나올 영역..
  		rand.x = GetRandomPos(-100, 100);
		rand.y = GetRandomPos(-100, 100);
		rand.z = GetRandomPos(-100, 100);
		
		double length = sqrt((float)rand.x*(float)rand.x+(float)rand.y*(float)rand.y+(float)rand.z*(float)rand.z);
		
		if(length == 0)
			length = 1;
		
		int range = GetRandomPos(5000,7000);
     	curPos.x = int(float(rand.x)/length*(float)range+(float)pos.x);
		curPos.y = int(float(rand.y)/length*(float)range+(float)pos.y);
		curPos.z = int(float(rand.z)/length*(float)range+(float)pos.z);
		
		Assa = SetAssaEffect(200,"flare.tga",0,&curPos,0);                
		
		cAssaEffect[Assa]->FACE_TYPE = ASSAFACE_SCALE;
  		cAssaEffect[Assa]->Time = -(15+index/2);
		
		cAssaEffect[Assa]->MOVECODE = ASSA_MOVE;
  	 	cAssaEffect[Assa]->MoveSpeed.x = int(float(rand.x)/length*(150.f));
		cAssaEffect[Assa]->MoveSpeed.y = int(float(rand.y)/length*(150.f));
		cAssaEffect[Assa]->MoveSpeed.z = int(float(rand.z)/length*(150.f));
		
		//파티클의 넓이..
    	cAssaEffect[Assa]->Size.w = 500;
 		cAssaEffect[Assa]->Face.r = 140;
		cAssaEffect[Assa]->Face.g = 100;
		cAssaEffect[Assa]->Face.b = 220;
		cAssaEffect[Assa]->Face.Transparency = 255;

		//파티클의 길이..
		POINT3D destPos;
   		destPos.x = int(float(rand.x)/length*12000.f);
		destPos.y = int(float(rand.y)/length*12000.f);
		destPos.z = int(float(rand.z)/length*12000.f);
		
		cAssaEffect[Assa]->ScalePosi.x = -destPos.x;
		cAssaEffect[Assa]->ScalePosi.y = -destPos.y;
		cAssaEffect[Assa]->ScalePosi.z = -destPos.z;
	}
	
	//같은 캐릭터가 유지 스킬을 쓰면 없애준다.
 	StopAssaCodeEffect(pChar, DRASTIC_ACTION);
	StopAssaCodeEffect(pChar, HOLY_ACTION);
	
	//실제로 유지되는 파티클 호출..
	int i = GetAssaEffect();
 	cASSAChar *partChar = new cASSAChar;
	partChar->Start(pChar, DRASTIC_SPIRIT_JANG, maxTime*70);
	cAssaEffect[i] = partChar;

	return TRUE;
}

int AssaParticle_DrasticSpirit_Member(smCHAR *pChar, int maxTime)
{
	//같은 캐릭터가 유지 스킬을 쓰면 없애준다.
 	StopAssaCodeEffect(pChar, DRASTIC_ACTION);
	StopAssaCodeEffect(pChar, HOLY_ACTION);
	
	//실제로 유지되는 파티클 호출..
 	int i = GetAssaEffect();
 	cASSAChar *partChar = new cASSAChar;
	partChar->Start(pChar, DRASTIC_SPIRIT_MEMBER, maxTime*70);
	cAssaEffect[i] = partChar;
	return TRUE;
}

int AssaParticle_Windy(smCHAR *pChar, int maxTime)
{
	//같은 캐릭터가 유지 스킬을 쓰면 없애준다.
   	StopAssaCodeEffect(pChar, WINDY_ACTION, 3);

	//실제로 유지되는 파티클 호출..
 	int i = GetAssaEffect();
 	cASSAChar *partChar = new cASSAChar;
	partChar->Start(pChar, WINDY_ACTION, maxTime*70);
	cAssaEffect[i] = partChar;
	return TRUE;
}

void SkillParticle_FrostJavelin(smCHAR *pChar, float time)
{
	sinSkillEffect_Windy(pChar);
	
	//실제로 유지되는 파티클 호출..
 	int i = GetAssaEffect();
 	cASSAChar *partChar = new cASSAChar;
	partChar->Start(pChar, SKILL_FROST_JAVELIN, int(time*70.f));
	cAssaEffect[i] = partChar;
}

int AssaParticle_TwistJavelinShot(POINT3D *pPosi, POINT3D *pDesti)
{
  	
  	Assa = SetAssaEffect(0,"TwistJavelin2.ASE",0,pPosi,0);
	
	cAssaEffect[Assa]->Time = 0;
	cAssaEffect[Assa]->AniMaxCount = 60;
	cAssaEffect[Assa]->AniDelayTime = 4;
 	cAssaEffect[Assa]->MOVECODE = ASSA_ANGLE_ROTATE;
	cAssaEffect[Assa]->Angle.y = ANGLE_180;

	GetRadian3D( pPosi->x, pPosi->y, pPosi->z, pDesti->x, pDesti->y, pDesti->z);
	cAssaEffect[Assa]->Angle.x =  GeoResult_X;
	cAssaEffect[Assa]->Angle.y =  GeoResult_Y;
	cAssaEffect[Assa]->Angle.z =  0;
	
	cAssaEffect[Assa]->Angle.y = (-GeoResult_Y+ANGLE_180)&ANGCLIP;
	cAssaEffect[Assa]->NotFadeIn = TRUE;
	cAssaEffect[Assa]->ARotateSpeed.z = 200;
	int i = GetAssaEffect();
	cASSAPARTSHOT *partShot = new cASSAPARTSHOT;
	partShot->Start(pPosi, pDesti, cAssaEffect[Assa], SKILL_TWIST_JAVELIN);
	cAssaEffect[i] = partShot;

	return TRUE;
}

int AssaParticle_FireJavelin(POINT3D *pPosi, POINT3D *pDesti)
{
    int i = GetAssaEffect();
	cASSAPARTSHOT *partShot = new cASSAPARTSHOT;
	partShot->Start(pPosi, pDesti, SKILL_FIRE_JAVELIN);
	cAssaEffect[i] = partShot;

	return TRUE;
}

int AssaParticle_FireJavelin(smCHAR *pChar, POINT3D *pDesti)
{
     int i = GetAssaEffect();
	cASSAPARTSHOT *partShot = new cASSAPARTSHOT;

	partShot->Start(pChar, pDesti, SKILL_FIRE_JAVELIN);
	cAssaEffect[i] = partShot;

	return TRUE;
}

int AssaParticle_Star(POINT3D *pPosi)
{
	POINT3D curPos;
   	for(int index = 0; index < 40; index++)
	{
	 	curPos.x = rand()%10000-5000+pPosi->x;
 		curPos.y = rand()%10000-5000+pPosi->y;
		curPos.z = rand()%10000-5000+pPosi->z;
		
		Assa = SetAssaEffect(50,"star04B_01.bmp",0,&curPos,0);
		AssaGetAniFileMat(cAssaEffect[Assa]);
		int temp = rand()%4000-2000;
 		cAssaEffect[Assa]->Size.w = 4000+temp;
		cAssaEffect[Assa]->Size.h = 4000+temp;
		cAssaEffect[Assa]->AniDelayTime = 10;
		cAssaEffect[Assa]->Time = -index*2-30;
		cAssaEffect[Assa]->FACE_TYPE = ASSAFACE_BILLBOARD;
	}
	g_NewParticleMgr.Start("HolyValor", *pPosi);
	return TRUE;
}

int AssaParticle_DivineLighting(smCHAR *pChar)
{
	int i = GetAssaEffect();
	cASSAPARTSHOT *partShot = new cASSAPARTSHOT;
	
	POINT3D curPos;
	POINT3D desPos;
	curPos.x = pChar->pX;
	curPos.y = pChar->pY+100000;
	curPos.z = pChar->pZ;

	desPos.x = pChar->pX;
	desPos.y = pChar->pY+5000;
	desPos.z = pChar->pZ;
	partShot->Start(&curPos, &desPos, ASSA_SHOT_SPARK);
	cAssaEffect[i] = partShot;
	
	return TRUE;
}

void SkillCelestialChainLighting(smCHAR **pCharList, int charCount)
{
	//if(charCount < 2)
		//return;
	int i = GetAssaEffect();
	cASSAPARTSHOT *partShot = new cASSAPARTSHOT;
	
	POINT3D curPos;
	POINT3D desPos;
	curPos.x = pCharList[0]->pX;
	curPos.y = pCharList[0]->pY+100000;
	curPos.z = pCharList[0]->pZ;

	desPos.x = pCharList[0]->pX;
	desPos.y = pCharList[0]->pY+5000;
	desPos.z = pCharList[0]->pZ;
	partShot->Start(&curPos, &desPos, ASSA_SHOT_SPARK);
	partShot->SetChainLighting(pCharList, charCount);
	cAssaEffect[i] = partShot;
}

int AssaParticle_Sprak(POINT3D *pPosi)
{
	for(int index = 0; index < 5; index++)
	{
		int i = GetAssaEffect();
		cASSAPARTSPARK *partSpark = new cASSAPARTSPARK;
		
		partSpark->Start(pPosi);
		partSpark->Time = GetRandomPos(0,20);
		cAssaEffect[i] = partSpark;
	}
	
	SetDynLight( pPosi->x, pPosi->y, pPosi->z, 255, 255, 255, 255, 100, 2);
	g_NewParticleMgr.Start("DivineLightning", *pPosi);
	return TRUE;
}

int AssaParticle_ColumnOfWater(POINT3D *pPosi, POINT3D *pDesti, int level)
{
   	if(level < 1 || level > 10)
		return false;
	
	if(level == 1 || level == 2 || level == 3)
		Assa = SetAssaEffect(0,"ColumnOfWaterLV_123.ASE",0,pPosi,0);
	else if(level == 4 || level == 5 || level == 6)
		Assa = SetAssaEffect(0,"ColumnOfWaterLV_456.ASE",0,pPosi,0);
	else if(level == 7 || level == 8 || level == 9)
		Assa = SetAssaEffect(0,"ColumnOfWaterLV_789.ASE",0,pPosi,0);
	else if(level == 10)
		Assa = SetAssaEffect(0,"ColumnOfWaterLV_10.ASE",0,pPosi,0);
	else 
		return false;
	
	cAssaEffect[Assa]->AniMaxCount = 20;
	cAssaEffect[Assa]->AniDelayTime = 4;
	cAssaEffect[Assa]->MOVECODE = ASSA_ANGLE_ROTATE;
 	cAssaEffect[Assa]->ARotateSpeed.y = -50;
	
	
	int i = GetAssaEffect();
	cASSAPARTSHOT *partShot = new cASSAPARTSHOT;
	partShot->Start(pPosi, pDesti, cAssaEffect[Assa]);
	cAssaEffect[i] = partShot;
	
	return TRUE;
}

int AssaParticle_ColumnOfWaterHit(POINT3D *pPosi)
{
    Assa = SetAssaEffect(0,"ColumnOfWaterHit.ASE",0,pPosi,0);                 
	cAssaEffect[Assa]->AniMaxCount = 20;
	cAssaEffect[Assa]->AniDelayTime = 4;
	g_NewParticleMgr.Start("ColumnOfWater2", *pPosi);
	return TRUE;
}

int AssaParticle_EnchantWeaponIceJang(smCHAR *pChar, int maxTime)
{
	//같은 캐릭터가 유지 스킬을 쓰면 없애준다.
    StopAssaCodeEffect(pChar, ENCHANT_WEAPON, 3);
	int i = GetAssaEffect();
  	cASSAChar *partChar = new cASSAChar;
	partChar->Start(pChar, ENCHANT_WEAPON_ICE, maxTime*70, 1.0f);
	cAssaEffect[i] = partChar;
	
	POINT3D pos;
	pos.x = pChar->pX;
	pos.y = pChar->pY+2000;
	pos.z = pChar->pZ;
	g_NewParticleMgr.Start("EnchantWeaponJang", pos);
	return TRUE;
}

void SkillImpulsionLight(smCHAR *pChar, float time)
{
	StopAssaCodeEffect(pChar, IMPULSION_LIGHT, 3);
	int i = GetAssaEffect();
  	cASSAChar *partChar = new cASSAChar;
	partChar->Start(pChar, IMPULSION_LIGHT, int(time * 70.f), 1.0f);
	cAssaEffect[i] = partChar;
}

void SkillImpulsionLightHit(POINT3D *pos)
{
	StartSkill(pos->x, pos->y, pos->z, 0, 0, 0, SKILL_GRAND_SMASH);
	SetDynLight( pos->x,pos->y,pos->z, 0,100,50,0, 250 , 5);
	g_NewParticleMgr.Start("Skill4ImpulsionHit1", *pos);
}

int AssaParticle_EnchantWeaponFireJang(smCHAR *pChar, int maxTime)
{
	//같은 캐릭터가 유지 스킬을 쓰면 없애준다.
  	StopAssaCodeEffect(pChar, ENCHANT_WEAPON, 3);

    int i = GetAssaEffect();
 	cASSAChar *partChar = new cASSAChar;
	partChar->Start(pChar, ENCHANT_WEAPON_FIRE, maxTime*70, 1.0f);
	cAssaEffect[i] = partChar;
	
	POINT3D pos;
	pos.x = pChar->pX;
	pos.y = pChar->pY+2000;
	pos.z = pChar->pZ;
	g_NewParticleMgr.Start("EnchantWeaponJang", pos);

	return TRUE;
}

int AssaParticle_EnchantWeaponLightJang(smCHAR *pChar, int maxTime)
{
	//같은 캐릭터가 유지 스킬을 쓰면 없애준다.
 	StopAssaCodeEffect(pChar, ENCHANT_WEAPON, 3);

    int i = GetAssaEffect();
 	cASSAChar *partChar = new cASSAChar;
	partChar->Start(pChar, ENCHANT_WEAPON_LIGHT, maxTime*70, 1.0f);
	cAssaEffect[i] = partChar;
	
	POINT3D pos;
	pos.x = pChar->pX;
	pos.y = pChar->pY+2000;
	pos.z = pChar->pZ;
	g_NewParticleMgr.Start("EnchantWeaponJang", pos);

	return TRUE;
}

int AssaParticle_EnchantWeaponJinMember(smCHAR *pChar)
{
	int i = GetAssaEffect();
  	cASSAChar *partChar = new cASSAChar;
	partChar->Start(pChar, ENCHANT_MEMBER_JIN, 140);
	cAssaEffect[i] = partChar;

	return TRUE;
}

int AssaParticle_EnchantWeaponIceMember(smCHAR *pChar, int maxTime)
{
	//같은 캐릭터가 유지 스킬을 쓰면 없애준다.
 	StopAssaCodeEffect(pChar, ENCHANT_WEAPON, 3);

	int i = GetAssaEffect();
  	cASSAChar *partChar = new cASSAChar;
	partChar->Start(pChar, ENCHANT_WEAPON_ICE, maxTime*70);
	cAssaEffect[i] = partChar;
	
	return TRUE;
}

int AssaParticle_EnchantWeaponFireMember(smCHAR *pChar, int maxTime)
{
	//같은 캐릭터가 유지 스킬을 쓰면 없애준다.
 	StopAssaCodeEffect(pChar, ENCHANT_WEAPON, 3);
    
	int i = GetAssaEffect();
 	cASSAChar *partChar = new cASSAChar;
	partChar->Start(pChar, ENCHANT_WEAPON_FIRE, maxTime*70);
	cAssaEffect[i] = partChar;
	
	return TRUE;
}
int AssaParticle_EnchantWeaponLightMember(smCHAR *pChar, int maxTime)
{
	//같은 캐릭터가 유지 스킬을 쓰면 없애준다.
	StopAssaCodeEffect(pChar, ENCHANT_WEAPON, 3);

    int i = GetAssaEffect();
 	cASSAChar *partChar = new cASSAChar;
	partChar->Start(pChar, ENCHANT_WEAPON_LIGHT, maxTime*70);
	cAssaEffect[i] = partChar;
	return TRUE;
}

//Dead Lay 레이저 쏘기..
int AssaParticle_DeadLay(POINT3D *pPosi, POINT3D *pDesti)
{
 	int i = GetAssaEffect();
	cASSAPARTSHOT *partShot = new cASSAPARTSHOT;
	partShot->Start(pPosi, pDesti, 100);
	cAssaEffect[i] = partShot;
	
	//쏠때의 파티클..

	g_NewParticleMgr.Start("DeadLayStart1", *pPosi);
	//타격 파티클..
	g_NewParticleMgr.Start("DeadLayHit1", *pDesti);
	
	return TRUE;
}


/////////////////////////////// 박철호 /////////////////////////////////////////////
//ColdEye 레이저 쏘기..
int ParkAssaParticle_ColdEye(POINT3D *pPosi, POINT3D *pDesti)
{
 	int i = GetAssaEffect();
	cASSAPARTSHOT *partShot = new cASSAPARTSHOT;
	partShot->Start(pPosi, pDesti, ASSA_SHOT_LASER2);
	cAssaEffect[i] = partShot;
	
	//쏠때의 파티클..

	g_NewParticleMgr.Start("IceColdEyeStart1", *pPosi);
	//타격 파티클..
	g_NewParticleMgr.Start("IceColdEyeHit1", *pDesti);
	
	return TRUE;
}

//ColdEye 스킬공격
int ParkParticle_ColdEyeSkill(smCHAR *pChar)
{
	POINT3D pos;
	pos.x = pChar->pX;
	pos.y = pChar->pY;
	pos.z = pChar->pZ;

	//SkillArchMageFlameWave(pChar);
	StartEffectMonster(pos.x, pos.y, pos.z, MONSTER_COLDEYE_SKILL);
	return TRUE;
}

//미스틱 스피넬 자폭하기
int ParkAssaParticle_MisticSpinel(smCHAR *pChar)
{
	POINT3D pos;
    pos.x = pChar->pX;
	pos.y = pChar->pY + 7500;
	pos.z = pChar->pZ;

 	g_NewParticleMgr.Start("IceMisticSpinelSkill1", pos);
	
	return TRUE;
}

// 프로스트 에이션트 디아 이펙트
int ParkAssaParticle_AcientDia(smCHAR *pChar)
{
	POINT3D charPos;
    charPos.x = pChar->pX;
	charPos.y = pChar->pY+1000;
	charPos.z = pChar->pZ;


	//SetDynLight( charPos.x, charPos.y, charPos.z,-150, -150, -255, 255, 250, 2);
   	//Assa = SetAssaEffect(0,"Mg3_Diastrophism24.ASE",0,&charPos,0, 0);
	Assa = SetAssaEffect(0,"Monster_Diastrophism.ASE",0,&charPos,0, 0);
	
	cAssaEffect[Assa]->AniMaxCount = 24;
	cAssaEffect[Assa]->AniDelayTime = 3;
	cAssaEffect[Assa]->StartPosi.x = 256*8*6;
	
	cAssaEffect[Assa]->StartPosi.z = -256*8*3;
	cAssaEffect[Assa]->Angle.y = ((pChar->Angle.y&ANGCLIP)+ANGLE_270)&ANGCLIP;

	AssaSetStartPosi(cAssaEffect[Assa]);
	cAssaEffect[Assa]->Angle.y =  -((pChar->Angle.y&ANGCLIP))&ANGCLIP;

	cAssaEffect[Assa]->Posi.y = cAssaEffect[Assa]->Posi.y + 1000;;
	

	SetDynLight( cAssaEffect[Assa]->Posi.x , cAssaEffect[Assa]->Posi.y , cAssaEffect[Assa]->Posi.z,-150, -150, -255, 255, 250, 2);

	g_NewParticleMgr.Start("IceAceintSkill2", cAssaEffect[Assa]->Posi);
//	g_NewParticleMgr.Start("Skill3MagicianDiastrophism1", cAssaEffect[Assa]->Posi);
	//g_NewParticleMgr.Start("IceAceintSkill2", pChar->Posi);
	
	return TRUE;
}

int AssaParticle_NoWeaPon_AcientPiercing(cASSAEFFECT *pEffect)
{
	POINT3D TempPosi;
	cAssaEffect[Assa]->Face.MatNum = g_NewParticleMgr.Start("Piercing1", pEffect->Posi); //호바람 파티클을 붙인다 
	
	//날아갈곳 지점을 구한다 
	GetMoveLocation(  0, 0,256*64 , pEffect->Angle.x, ((pEffect->pChar->Angle.y&ANGCLIP)+ANGLE_180)&ANGCLIP, 0 );
	TempPosi.x = pEffect->Posi.x+GeoResult_X+1;
	TempPosi.y = pEffect->Posi.y;
	TempPosi.z = pEffect->Posi.z+GeoResult_Z;

	pEffect->MOVECODE = ASSA_MOVE;
	pEffect->MoveSpeed.x = (pEffect->Posi.x - TempPosi.x)/20;
	pEffect->MoveSpeed.z = (pEffect->Posi.z - TempPosi.z)/20;
	pEffect->CODE = 0x50000004; 


	return TRUE;
}

int sinSkillEffect_NoWeaponPiercing(smCHAR *pChar)
{
	//Bone
	POINT3D TempPosi = {pChar->pX,pChar->pY,pChar->pZ}; 	        	  
	int TempIndex = 0 ,Temp4 = 0;
	
	//라이트 
	//AssaParticle_Char_Light2(pChar,ASSA_COLOR_CODE_BLUE,70);


	SetDynLight( pChar->pX , pChar->pY , pChar->pZ ,0, 150, 255, 255, 130 ,1);

	return TRUE;
}

//extern int sinSkillEffect_Piercing(smCHAR *pChar);
// 프로스트 에이션트 피어싱
int ParksinSkillEffect_AcientPiercing(smCHAR *pChar)
{
	POINT3D pTmp={pChar->pX, pChar->pY, pChar->pZ};
	//pChar->pY = pTmp.y+15500;

	sinSkillEffect_NoWeaponPiercing(pChar);

	Assa = SetAssaEffect(30,"Piercing.ASE",pChar,0,15500);    
	cAssaEffect[Assa]->NotFadeIn = 1;

	cAssaEffect[Assa]->StartPosi.x = 512*8*6;
 	cAssaEffect[Assa]->Angle.y =  ((pChar->Angle.y&ANGCLIP)+ANGLE_270)&ANGCLIP;
	AssaSetStartPosi(cAssaEffect[Assa]);
	cAssaEffect[Assa]->Angle.y =  (-(pChar->Angle.y&ANGCLIP)+ANGLE_180)&ANGCLIP;

	AssaParticle_NoWeaPon_AcientPiercing(cAssaEffect[Assa]); //두개찍어준다

	return TRUE;
}



// 아이스 골렘 이펙트
int ParkAssaParticle_IceGolemShot(smCHAR *pChar)
{
	POINT3D pos;
     pos.x = pChar->pX;
	pos.y = pChar->pY;
	pos.z = pChar->pZ;
	
	GetMoveLocation( 0, 0, -30*fONE, pChar->Angle.x, pChar->Angle.y, 0);
	pos.x += GeoResult_X;
	pos.z += GeoResult_Z;
    
	Assa = SetAssaEffect(0,"m_IceGolem.ASE",0,&pos,0, 1);
	cAssaEffect[Assa]->AniMaxCount = 30;
	cAssaEffect[Assa]->AniDelayTime = 2;
	cAssaEffect[Assa]->Angle.y = (-pChar->Angle.y+ANGLE_180)&ANGCLIP;
	cAssaEffect[Assa]->NotFadeIn = TRUE;
	
	pos.x = pos.x;
	pos.y = pos.y+1500;
	pos.z = pos.z;
	
 	GetMoveLocation( 0, 0, 120*fONE, pChar->Angle.x, pChar->Angle.y, 0);
	pos.x += GeoResult_X;
	pos.z += GeoResult_Z;
	//g_NewParticleMgr.Start("StoneGolemHit1", pos);
	g_NewParticleMgr.Start("IceGolemHit1", pos);

	return TRUE;
}

// 아이스 고블린
int ParkAssaParticle_IceGoblinHit1(smCHAR *pChar)
{

	int i = GetAssaEffect();
	/*
	cAssaMotionBlur *motionBlur = new cAssaMotionBlur;
	motionBlur->Start(pChar, "Bip-skill01", "Bip-skill", 150);
	motionBlur->SetScale(fScale);
	cAssaEffect[i] = motionBlur;

     i = GetAssaEffect();
	 */
 	cAssaTraceCharMeshParticle *traceMeshParticle = new cAssaTraceCharMeshParticle;
 	traceMeshParticle->Start(pChar, "Bip-hammer", "IceGoblin", false, 130);
	cAssaEffect[i] = traceMeshParticle;

	POINT3D pos;
 	pos.x = pChar->pX;
	pos.y = pChar->pY;
	pos.z = pChar->pZ;

  
    GetMoveLocation( 0, 0, 100*fONE, pChar->Angle.x, pChar->Angle.y, 0);

	pos.x += GeoResult_X;
	pos.z += GeoResult_Z;
	pos.y += fONE*10;
	g_NewParticleMgr.Start("IceGoblinHit1", pos, 1.2f);

	return TRUE;
}

// 프로즌 미스트	기본공격
int ParkAssaParticle_MistIceBolt(smCHAR* pChar, smCHAR* pDestChar)
{
	//StartMonsterDest(POINT3D current, POINT3D dest, int effectType)
	//StartMonsterDest(pCha   MONSTER_PIGON_SHOT1);
	//sinEffect_FireBolt(pChar, pDestChar);
	GetMoveLocation( 0 , 60*fONE, 20*fONE , 0, pChar->Angle.y , 0 );
	POINT3D curPos;
	POINT3D desPos;
	curPos.x = pChar->pX+GeoResult_X;
	curPos.y = pChar->pY+GeoResult_Y;
	curPos.z = pChar->pZ+GeoResult_Z;
	
	desPos.x = pDestChar->pX;
	desPos.y = pDestChar->pY+24*fONE;
	desPos.z = pDestChar->pZ;

	StartEffectMonsterDest(curPos.x, curPos.y, curPos.z, desPos.x, desPos.y, desPos.z, MONSTER_FORZENMIST_SHOT1 );
    
	return TRUE;
}
// 프로즌 미스트	스킬공격
int ParkAssaParticle_MistIceBall(smCHAR* pChar, smCHAR* pDestChar)
{
	GetMoveLocation( 0 , 60*fONE, 20*fONE , 0, pChar->Angle.y , 0 );
	POINT3D curPos;
	POINT3D desPos;
	curPos.x = pChar->pX+GeoResult_X;
	curPos.y = pChar->pY+GeoResult_Y;
	curPos.z = pChar->pZ+GeoResult_Z;
	
	desPos.x = pDestChar->pX;
	desPos.y = pDestChar->pY+24*fONE;
	desPos.z = pDestChar->pZ;


	StartEffectMonsterDest(curPos.x, curPos.y, curPos.z, desPos.x, desPos.y, desPos.z, MONSTER_FORZENMIST_SHOT2 );
	return TRUE;
}


int Assa_Piercing(cASSAEFFECT *pEffect)
{
	POINT3D TempPosi;
	//무기의 앞부분 과 캐릭의 1직선 방향으로 날린다  
	AssaSearchWeaponPos(pEffect->pChar,&pEffect->Posi,1);            
	cAssaEffect[Assa]->Face.MatNum = g_NewParticleMgr.Start("Piercing1", pEffect->Posi); //호바람 파티클을 붙인다 

	//날아갈곳 지점을 구한다 
	GetMoveLocation(  0, 0,256*128 , pEffect->Angle.x, ((pEffect->pChar->Angle.y&ANGCLIP)+ANGLE_180)&ANGCLIP, 0 );
	TempPosi.x = pEffect->Posi.x+GeoResult_X;
	TempPosi.y = pEffect->Posi.y;
	TempPosi.z = pEffect->Posi.z+GeoResult_Z;

	pEffect->MOVECODE = ASSA_MOVE;
	pEffect->MoveSpeed.x = (pEffect->Posi.x - TempPosi.x)/20;
	pEffect->MoveSpeed.z = (pEffect->Posi.z - TempPosi.z)/20;
	pEffect->CODE = 0x50000004; 


	return TRUE;
}


// 발렌토 피어싱
int AssaParticle_NoWeaPon_ValentoPiercing(cASSAEFFECT *pEffect)
{
	POINT3D TempPosi;
	cAssaEffect[Assa]->Face.MatNum = g_NewParticleMgr.Start("IceValentoPiercing", pEffect->Posi); //호바람 파티클을 붙인다 

	
	//날아갈곳 지점을 구한다 
	GetMoveLocation(  0, 0,256*128 + 2000 , pEffect->Angle.x, ((pEffect->pChar->Angle.y&ANGCLIP)+ANGLE_180)&ANGCLIP, 0 );
	TempPosi.x = pEffect->Posi.x+GeoResult_X+1;
	TempPosi.y = pEffect->Posi.y;
	TempPosi.z = pEffect->Posi.z+GeoResult_Z;

	pEffect->MOVECODE = ASSA_MOVE;
	pEffect->MoveSpeed.x = (pEffect->Posi.x - TempPosi.x)/20;
	pEffect->MoveSpeed.z = (pEffect->Posi.z - TempPosi.z)/20;
	pEffect->CODE = 0x50000004; 


	return TRUE;
}

// 발렌토 피어싱
void ParksinSkillEffect_ValentoPiercing(smCHAR* pChar)
{
	POINT3D pTmp={pChar->pX, pChar->pY, pChar->pZ};
	pChar->pY = pTmp.y;

	sinSkillEffect_NoWeaponPiercing(pChar);

	Assa = SetAssaEffect(30,"Piercing.ASE",pChar,0,15500);    
	cAssaEffect[Assa]->NotFadeIn = 1;

	cAssaEffect[Assa]->StartPosi.x = 512*8*6;
	cAssaEffect[Assa]->StartPosi.z = -256*8*6;
 	cAssaEffect[Assa]->Angle.y =  ((pChar->Angle.y&ANGCLIP)+ANGLE_270)&ANGCLIP;
	AssaSetStartPosi(cAssaEffect[Assa]);
	cAssaEffect[Assa]->Angle.y =  (-(pChar->Angle.y&ANGCLIP)+ANGLE_180)&ANGCLIP;

	AssaParticle_NoWeaPon_ValentoPiercing(cAssaEffect[Assa]); //두개찍어준다

}
int ParkAssaParticle_ValentoBran(smCHAR* pChar)
{

	POINT3D pos;
 	pos.x = pChar->pX;
	pos.y = pChar->pY;
	pos.z = pChar->pZ;

  
    GetMoveLocation( 0, 0, 100*fONE, pChar->Angle.x, pChar->Angle.y, 1);
	pos.x += GeoResult_X;
	pos.z += GeoResult_Z;
	pos.y += fONE*20;
	//g_NewParticleMgr.Start("RatooHit1", pos, 0);
	
    pos.y -= fONE*20;
  	Assa = SetAssaEffect(0,"m_ratoo.ASE",0,&pos,0);
	cAssaEffect[Assa]->Time = -4;
	cAssaEffect[Assa]->AniMaxCount = 20;
	cAssaEffect[Assa]->AniDelayTime = 5;
	cAssaEffect[Assa]->Angle.y = (-pChar->Angle.y+ANGLE_180)&ANGCLIP;


	POINT3D partPos;
  	partPos.x = cAssaEffect[Assa]->Posi.x;
	partPos.y = cAssaEffect[Assa]->Posi.y+2000;
	partPos.z = cAssaEffect[Assa]->Posi.z;
	g_NewParticleMgr.Start("IceValentoBrandish",partPos);

	return TRUE;

}

void ParkValentSwordShow(smCHAR* pValento, int nTime)
{
	if(pValento)
	{
		int i = GetAssaEffect();
		cAssaMotionBlur *motionBlur = new cAssaMotionBlur;
		motionBlur->Start(pValento, "Bip01gho01", "Bip01gho02", nTime);
		motionBlur->Color.R = 200;
		motionBlur->Color.G = 100;
		motionBlur->Color.B = 200;
		motionBlur->Color.A = 100;
		cAssaEffect[i] = motionBlur;
	}
}

// 데스나이트 브랜디쉬
int ParkAssaParticle_Valento1(smCHAR* pValento)
{
	if(pValento)
	{
		//ParkValentSwordShow(pChar, 140);
        ParkAssaParticle_ValentoBran(pValento);
	}
	return TRUE;
}
// 데스나이트 피어싱 ( 버서커 )
int ParkAssaParticle_Valento2_1(smCHAR* pValento)
{
	if(pValento)
	{
		//ParkValentSwordShow(pChar, 70);
		// 버서커
		Valento_PowerUp(pValento);

	}

	return TRUE;
}

// 데스나이트 피어싱 (  피어싱 )
int ParkAssaParticle_Valento2_2(smCHAR* pValento)
{
	if(pValento)
	{
		// 피어싱
		//ParkValentSwordShow(pChar, 70);
		ParksinSkillEffect_ValentoPiercing(pValento);

	}

	return TRUE;
}


int ParkAssaParticle_Valento3_1(smCHAR* pValento)
{
	if(pValento)
	{
		//ParkValentSwordShow(pChar, 140);
		//소드오브저스티스
		for(int i = 0; i <2 ; i++){

		Assa = SetAssaEffect(0,"Nt_4-4.ASE",pValento,0,0);
		cAssaEffect[Assa]->AniMaxCount = 16; 
		cAssaEffect[Assa]->AniDelayTime = 2;
		cAssaEffect[Assa]->StartPosi.x = 512*8*6;
	 	cAssaEffect[Assa]->Angle.y =  ((pValento->Angle.y&ANGCLIP)+ANGLE_270)&ANGCLIP;
		AssaSetStartPosi(cAssaEffect[Assa]);
		cAssaEffect[Assa]->Angle.y =  (-(pValento->Angle.y&ANGCLIP)+ANGLE_180)&ANGCLIP;

		Assa = SetAssaEffect(0,"Nt_4-41.ASE",pValento,0,0);
		cAssaEffect[Assa]->AniMaxCount = 18; 
		cAssaEffect[Assa]->AniDelayTime = 3;
		cAssaEffect[Assa]->StartPosi.x = 512*8*6;
	 	cAssaEffect[Assa]->Angle.y =  ((pValento->Angle.y&ANGCLIP)+ANGLE_270)&ANGCLIP;
		AssaSetStartPosi(cAssaEffect[Assa]);
		cAssaEffect[Assa]->Angle.y =  (-(pValento->Angle.y&ANGCLIP)+ANGLE_180)&ANGCLIP;

		Assa = SetAssaEffect(0,"Nt_4-42.ASE",pValento,0,0);
		cAssaEffect[Assa]->AniMaxCount = 20; 
		cAssaEffect[Assa]->AniDelayTime = 4;
		cAssaEffect[Assa]->StartPosi.x = 512*8*6;
	 	cAssaEffect[Assa]->Angle.y =  ((pValento->Angle.y&ANGCLIP)+ANGLE_270)&ANGCLIP;
		AssaSetStartPosi(cAssaEffect[Assa]);
		cAssaEffect[Assa]->Angle.y =  (-(pValento->Angle.y&ANGCLIP)+ANGLE_180)&ANGCLIP;


		}

		Valento_Justice(pValento);

	}
	return TRUE;
}


void ParkAssaParticle_Normal1_1(smCHAR *pDest)
{
	POINT3D charPos;
    charPos.x = pDest->pX;
	charPos.y = pDest->pY+500;
	charPos.z = pDest->pZ;
	
  	g_NewParticleMgr.Start("ChaosKaraNormal1_1", charPos);
	
	Assa = SetAssaEffect(0,"chao_glacial.ASE",0,&charPos,0, 0);
	cAssaEffect[Assa]->AniMaxCount = 25;
	cAssaEffect[Assa]->AniDelayTime = 2;

	charPos.x = charPos.x;
	charPos.y = charPos.y+1000;
	charPos.z = charPos.z;

	g_NewParticleMgr.Start("ChaosKaraNormal1_2", charPos, 0.3f);
}


int ParkAssaParticle_Skill(smCHAR *pChar)
{
 	POINT3D posi;
	posi.x = pChar->pX;
	posi.y = pChar->pY+2500;
	posi.z = pChar->pZ;
	
 	g_NewParticleMgr.Start("ChaosKaraSkill", posi);


	return TRUE;

}

int ParkAssaParticle_SkilltoUser(smCHAR *pDestChar)
{
 	POINT3D posi;
	posi.x = pDestChar->pX;
	posi.y = pDestChar->pY+2500;
	posi.z = pDestChar->pZ;

  	g_NewParticleMgr.Start("ChaosKaraSkillUser",posi, 0.1f);

	return TRUE;

}

void ParkAssaParticle_ChaosKaraTerrainFire(POINT3D *pCur, POINT3D *pDest, int delay=0)
{
	int i = GetAssaEffect();
	ParkAssaChaosKaraMeteo *skyParticle = new ParkAssaChaosKaraMeteo;
	skyParticle->Start(pCur, pDest, delay);
	cAssaEffect[i] = skyParticle;
}

void ChaosKaraMeteo(POINT3D *desPos)
{
	POINT3D curPos;
	POINT3D attackPos;
     attackPos.x = desPos->x;
	attackPos.y = desPos->y;
	attackPos.z = desPos->z+10000;
	curPos.x = attackPos.x;
	curPos.y = attackPos.y+130000;
	curPos.z = attackPos.z+50000;
	ParkAssaParticle_ChaosKaraTerrainFire(&curPos, &attackPos);
	
	attackPos.x = desPos->x;
	attackPos.y = desPos->y;
	attackPos.z = desPos->z-10000;
	curPos.x = attackPos.x;
	curPos.y = attackPos.y+130000;
	curPos.z = attackPos.z+50000;
	ParkAssaParticle_ChaosKaraTerrainFire(&curPos, &attackPos, 30);

	attackPos.x = desPos->x+10000;
	attackPos.y = desPos->y;
	attackPos.z = desPos->z;
	curPos.x = attackPos.x;
	curPos.y = attackPos.y+130000;
	curPos.z = attackPos.z+50000;
	ParkAssaParticle_ChaosKaraTerrainFire(&curPos, &attackPos, 60);

	attackPos.x = desPos->x-10000;
	attackPos.y = desPos->y;
	attackPos.z = desPos->z;
	curPos.x = attackPos.x;
	curPos.y = attackPos.y+130000;
	curPos.z = attackPos.z+50000;
	ParkAssaParticle_ChaosKaraTerrainFire(&curPos, &attackPos, 90);
}

// 카오스 카라 일반공격 (베놈 스피어)
int ParkAssaParticle_ChaosKara1(smCHAR* pChar)
{
	if(pChar)
	{
		ParkAssaParticle_Normal1_1(pChar);
	}
	return TRUE;
}

// 카오스 카라 아이스 미티어 (미티어 아이스)
int ParkAssaParticle_ChaosKara2(smCHAR* pChar)
{
	if(pChar)
	{
		//ParkAssaParticle_MonsterIceChaosKara(pChar);	// 스태프에 붙는 파티클
		ChaosKaraMeteo(&pChar->Posi);  // 이것이 진짜 미티어 아이스
	}
	return TRUE;
}

// 카오스 카라 뱀파이어릭 (법진)
int ParkAssaParticle_ChaosKaraSkill_Monster(smCHAR* pMonster)
{
	if(pMonster)
	{
		ParkAssaParticle_Skill(pMonster);
	}
	return TRUE;
}

// 카오스 카라 뱀파이어릭 ( 소울 서커 )
int ParkAssaParticle_ChaosKaraSkill_User(smCHAR* pUser)
{
	ParkAssaParticle_SkilltoUser(pUser);

	return TRUE;

}


// 블리자드 자이언트
void ParkBlizzardGiantSkillToTarget(smCHAR *pTarget)
{
	POINT3D charPos;
	if(pTarget)
	{
		GetMoveLocation( 5*fONE, 30*fONE, 0, 0, (pTarget->Angle.y-ANGLE_90)&ANGCLIP, 0);
		charPos.x = pTarget->pX+GeoResult_X;
		charPos.y = pTarget->pY+GeoResult_Y;
		charPos.z = pTarget->pZ+GeoResult_Z;

		g_NewParticleMgr.Start("DevilBirdNormal", charPos);
	}
	
}

// 데블 버드 공격동작 2번 타격이펙트
void ParkDevilBirdAttackNormal(smCHAR *pTarget)
{
	POINT3D charPos;
	if(pTarget)
	{
		GetMoveLocation( 5*fONE, 30*fONE, 0, 0, (pTarget->Angle.y-ANGLE_90)&ANGCLIP, 0);
		charPos.x = pTarget->pX+GeoResult_X;
		charPos.y = pTarget->pY+GeoResult_Y;
		charPos.z = pTarget->pZ+GeoResult_Z;

		g_NewParticleMgr.Start("IceHIt1", charPos);
	}	
}

void ParkDevilBirdBlur(smCHAR* pDevilBird, int nType)
{
	int i = GetAssaEffect();
	if(pDevilBird)
	{
		cAssaMotionBlur *motionBlur = new cAssaMotionBlur;
		
		switch(nType)
		{
		case 0:
			motionBlur->Start(pDevilBird, "Bip01 handbone01", "Bip01 handbone02", 30);
			break;

		case 1:
			motionBlur->Start(pDevilBird, "Bip01 wingbong01", "Bip01 wingbong03", 30);
			break;

		case 2:
			motionBlur->Start(pDevilBird, "Bip01 wingbong02", "Bip01 wingbong04", 30);
			break;
		}

		motionBlur->Color.R = 200;
		motionBlur->Color.G = 100;
		motionBlur->Color.B = 200;
		motionBlur->Color.A = 100;
		cAssaEffect[i] = motionBlur;
	}
}
// 데블 버드 공격동작 2번 날개짓 이펙 (날개와 주먹에 들어갈 모션 블러)
void ParkDevilBirdAttackBlur(smCHAR* pDevilBird)
{
	if(pDevilBird)
	{
		ParkDevilBirdBlur(pDevilBird, 0);
		ParkDevilBirdBlur(pDevilBird, 1);
		ParkDevilBirdBlur(pDevilBird, 2);

	}
}
// 데블 버드 기술동작 타격 이펙트
void ParkDevilBirdSkillToTarget(smCHAR *pTarget)
{
	POINT3D charPos;
	if(pTarget)
	{
		GetMoveLocation( 5*fONE, 40*fONE, 0, 0, (pTarget->Angle.y-ANGLE_90)&ANGCLIP, 0);

		charPos.x = pTarget->pX+GeoResult_X;
		charPos.y = pTarget->pY+GeoResult_Y;
		charPos.z = pTarget->pZ+GeoResult_Z;

  		g_NewParticleMgr.Start("DevilBirdSkillHit", charPos);
		

		charPos.x = charPos.x;
		charPos.y = charPos.y-6000;
		charPos.z = charPos.z;

		g_NewParticleMgr.Start("DevilBirdSkillHit", charPos, 0.15f);

		charPos.x = charPos.x;
		charPos.y = charPos.y+4500;
		charPos.z = charPos.z;

		g_NewParticleMgr.Start("DevilBirdSkillHit", charPos, 0.3f);
	}
}



// 스파이더 트루퍼 독가스 뿜어내기
void ParkAssaParticle_SpiderTrooperHit1(smCHAR *pChar)
{
	GetMoveLocation( 45*fONE, 0, 0, 0, (pChar->Angle.y-ANGLE_90)&ANGCLIP, 0);
	
	
	int num = -1;
 	POINT3D pos;
	pos.x = pChar->pX+GeoResult_X;
	pos.y = pChar->pY+14000;
	pos.z = pChar->pZ+GeoResult_Z;
	
	//GetMoveLocation( 0, 0, 30*fONE, 0, pChar->Angle.y, 0);

	//테스트용.
	D3DVECTOR  velocity;
	D3DVECTOR  gravity;
   	int i = GetAssaEffect(); 
  	velocity.x = float((float)GeoResult_X/15.f);
	velocity.y = 0.f;
	velocity.z = float((float)GeoResult_Z/15.f);

  	gravity.x = 0.f;
 	gravity.y = 0.f;
	gravity.z = 0.f;
/*	
	AssaSkill3BombShotExplision *explision = new AssaSkill3BombShotExplision;
	explision->Start(&pos, &velocity, &gravity, "IronMonsterBombHit2");
	cAssaEffect[i] = explision;
*/	
	
	num = g_NewParticleMgr.Start("ParkSpiderTrooper", pos);
	if(num != -1)
	{
		POINT3D angle;
		angle.x = 0;
		angle.y = pChar->Angle.y;
		angle.z = 0;
		g_NewParticleMgr.SetAngle(num, angle);
	}
	
}


int ParkAssaParticle_DivineLighting(smCHAR *pChar)
{
	int i = GetAssaEffect();
	cASSAPARTSHOT *partShot = new cASSAPARTSHOT;
	
	POINT3D curPos;
	POINT3D desPos;
	curPos.x = pChar->pX;
	curPos.y = pChar->pY+100000;
	curPos.z = pChar->pZ;

	desPos.x = pChar->pX;
	desPos.y = pChar->pY+5000;
	desPos.z = pChar->pZ;
	partShot->Start(&curPos, &desPos, ASSA_SHOT_PARKSPARK);
	cAssaEffect[i] = partShot;
	
	return TRUE;
}

int ParkAssaParticle_Sprak1(POINT3D *pPosi)
{
	for(int index = 0; index < 5; index++)
	{
		int i = GetAssaEffect();
		cASSAPARTSPARK *partSpark = new cASSAPARTSPARK;
		
		partSpark->Start(pPosi, "spark04_01.bmp");
		partSpark->Time = GetRandomPos(0,20);
		cAssaEffect[i] = partSpark;
	}
	
	SetDynLight( pPosi->x, pPosi->y, pPosi->z, 255, 255, 255, 255, 100, 2);
	g_NewParticleMgr.Start("DivineLightning", *pPosi);
	return TRUE;
}


//디바인 라이트닝 스킬 연출 ( 이펙트 실현 ) 최대 10개
int ParkDivineLight_Effect( smCHAR *lpChar )
{
	int cnt,TargetCount;
	int x,y,z,dist;
	int	LightningCount;
	int cCount;
	smCHAR *lpTarChar;

	TargetCount = 0;

	if ( lpChar==lpCurPlayer ) {
		for( cnt=0;cnt<DivineLightning_Target_Count;cnt++ ) {
			lpTarChar = FindAutoPlayer( dwSkill_DivineLightning_Target[cnt] );
			if ( lpTarChar ) {
				ParkAssaParticle_DivineLighting( lpTarChar );
				TargetCount++;
			}
		}
	}
	else {
		LightningCount = 10;

		for( cnt=0;cnt<OTHER_PLAYER_MAX;cnt++ ) {
			cCount = (DivineLightning_FindCount+cnt)&OTHER_PLAYER_MASK;
			if ( chrOtherPlayer[cCount].Flag && chrOtherPlayer[cCount].dwObjectSerial &&
				//lpChar!=&chrOtherPlayer[cCount] && chrOtherPlayer[cCount].smCharInfo.State==smCHAR_STATE_ENEMY &&
				lpChar!=&chrOtherPlayer[cCount] && chrOtherPlayer[cCount].smCharInfo.State==smCHAR_MONSTER_USER &&
				//chrOtherPlayer[cCount].smCharInfo.Brood!=smCHAR_MONSTER_USER &&
				chrOtherPlayer[cCount].smCharInfo.Brood!=smCHAR_STATE_USER &&
				chrOtherPlayer[cCount].smCharInfo.Life[0]>0 ) {

				x = ( lpChar->pX - chrOtherPlayer[cCount].pX )>>FLOATNS;
				y = ( lpChar->pY - chrOtherPlayer[cCount].pY )>>FLOATNS;
				z = ( lpChar->pZ - chrOtherPlayer[cCount].pZ )>>FLOATNS;

				dist = x*x+y*y+z*z;

				if ( dist<(160*160) ) {
					ParkAssaParticle_DivineLighting( &chrOtherPlayer[cCount] );
					TargetCount++;
					if ( TargetCount>=LightningCount ) {
						DivineLightning_FindCount = cCount;
						break;
					}
				}
			}
		}

		if ( TargetCount<LightningCount && lpChar->smCharInfo.State==smCHAR_STATE_ENEMY ) {
		//if ( TargetCount<LightningCount && lpChar->smCharInfo.State==smCHAR_STATE_USER ) {
		
			x = ( lpChar->pX - lpCurPlayer->pX )>>FLOATNS;
			y = ( lpChar->pY - lpCurPlayer->pY )>>FLOATNS;
			z = ( lpChar->pZ - lpCurPlayer->pZ )>>FLOATNS;

			dist = x*x+y*y+z*z;

			if ( dist<(160*160) ) {
				ParkAssaParticle_DivineLighting( lpCurPlayer );
				TargetCount++;
			}
		}

	}

	return TargetCount;
}

// 스팅레이 독가스 뿜어내기
void ParkAssaParticle_StingRay(smCHAR *pChar)
{
	GetMoveLocation( 45*fONE, 0, 0, 0, (pChar->Angle.y-ANGLE_90)&ANGCLIP, 0);

	int num = -1;
 	POINT3D pos;
	pos.x = pChar->pX+GeoResult_X;
	pos.y = pChar->pY+8000;
	pos.z = pChar->pZ+GeoResult_Z;
	
	//테스트용.
	D3DVECTOR  velocity;
	D3DVECTOR  gravity;
   	int i = GetAssaEffect(); 
  	velocity.x = float((float)GeoResult_X/15.f);
	velocity.y = 0.f;
	velocity.z = float((float)GeoResult_Z/15.f);

  	gravity.x = 0.f;
 	gravity.y = 0.f;
	gravity.z = 0.f;

	
	num = g_NewParticleMgr.Start("ParkStingRay", pos);
	if(num != -1)
	{
		POINT3D angle;
		angle.x = 0;
		angle.y = pChar->Angle.y;
		angle.z = 0;
		g_NewParticleMgr.SetAngle(num, angle);
	}
}

int ParkAssaParticle_Posion(smCHAR *pChar, int liveCount)
{
	//if(!AddAssaCodeEffectTime(pChar, MONSTER_SERQBUS_STATE1, liveCount*70))
	if(!AddAssaCodeEffectTime(pChar, MONSTER_POSION_STATE1, liveCount*70))
 	{
		int i = GetAssaEffect();
		cASSAChar *partChar = new cASSAChar;
		partChar->Start(pChar, MONSTER_POSION_STATE1, liveCount*70);
		cAssaEffect[i] = partChar;
	}

	return TRUE;
}

// 머미 통상 공격
void ParkAssaParticle_MummyShot(smCHAR *pChar, smCHAR *pDestChar)
{
  	GetMoveLocation( 0 , 30*fONE, 20*fONE , 0, pChar->Angle.y , 0 );
	POINT3D curPos;
	POINT3D desPos;
	curPos.x = pChar->pX+GeoResult_X;
	curPos.y = pChar->pY+GeoResult_Y;
	curPos.z = pChar->pZ+GeoResult_Z;
	
	desPos.x = pDestChar->pX;
	desPos.y = pDestChar->pY+24*fONE;
	desPos.z = pDestChar->pZ;
	
	StartEffectMonsterDest(curPos.x, curPos.y, curPos.z, desPos.x, desPos.y, desPos.z, MONSTER_MUMMY_SHOT );

}



// 메테오 대략 범용 만듬
void ParkAssaParticle_TerrainFire(POINT3D *pCur, POINT3D *pDest, int delay, int nType)
{
	int i = GetAssaEffect();
	AssaSkyMeteoParticle *skyParticle = new AssaSkyMeteoParticle;
	skyParticle->Start(pCur, pDest, delay, nType);
	cAssaEffect[i] = skyParticle;
}

void ParkAssaParticle_TerrainFireHit(POINT3D *pCur, int nType)
{
    D3DVECTOR vector;
    vector.x = float(pCur->x - lpCurPlayer->pX)/fONE;
	vector.y = float(pCur->y - lpCurPlayer->pY)/fONE;
	vector.z = float(pCur->z - lpCurPlayer->pZ)/fONE;
	
	float length;
 	length = (float)sqrt(vector.x*vector.x+vector.y*vector.y+vector.z*vector.z);
	if(length == 0)
		length = 1;

    SetDynLight( pCur->x, pCur->y, pCur->z, 255, 200, 100, 255, 250, 2);

	if(nType==0)
 		g_NewParticleMgr.Start("FireBigHit1", *pCur);
	else if(nType==1)
		g_NewParticleMgr.Start("IronMonsterBombHit1", *pCur);

    EffectWaveCamera(int((500.f - length)/15.f), 2);
}

// Dark Phalanx 기술 동작 이펙트 (창으로 찌를때 창이 파고드는 느낌의 이펙트 - 피어싱으로 처리)
void ParkAssaParticle_DarkPhalanx(smCHAR *pDarkPhalanx)
{
	if(pDarkPhalanx)
	{
		sinSkillEffect_NoWeaponPiercing(pDarkPhalanx);

		Assa = SetAssaEffect(30,"Piercing.ASE",pDarkPhalanx,0,15500);    
		cAssaEffect[Assa]->NotFadeIn = 1;

		//cAssaEffect[Assa]->StartPosi.x = -512*8*6;
		cAssaEffect[Assa]->StartPosi.z = -64*8*6;
		cAssaEffect[Assa]->StartPosi.y = -128*8*6;
 		cAssaEffect[Assa]->Angle.y =  ((pDarkPhalanx->Angle.y&ANGCLIP)+ANGLE_270)&ANGCLIP;
		AssaSetStartPosi(cAssaEffect[Assa]);
		cAssaEffect[Assa]->Angle.y =  (-(pDarkPhalanx->Angle.y&ANGCLIP)+ANGLE_180)&ANGCLIP;

		AssaParticle_NoWeaPon_ValentoPiercing(cAssaEffect[Assa]); //두개찍어준다

	}
}

void ParkAssaParticle_FireWorm( smCHAR* pTarget, BOOL bSkill)	// 파이어 웜 공격
{
	POINT3D curPos;
	POINT3D attackPos;

	attackPos.x = pTarget->pX;
 	attackPos.y = pTarget->pY;
	attackPos.z = pTarget->pZ+1000;

	curPos.x = attackPos.x;
	curPos.y = attackPos.y+130000;
	curPos.z = attackPos.z+50000;
	if ( !bSkill )
		ParkAssaParticle_TerrainFire(&curPos, &attackPos, 0, 1);
	else
		ParkAssaParticle_TerrainFire(&curPos, &attackPos, 0, 0);
}

// 키메라 일반공격 ( 발톱 블러 )
void ParkAssaParticle_ChimeraNormal(smCHAR *pChimera)
{
	int nTime = 70;
	if(pChimera)
	{
		int i = GetAssaEffect();
		cAssaMotionBlur *motionBlur = new cAssaMotionBlur;
		motionBlur->Start(pChimera, "Bip01 asw", "Bip01 asw01", nTime);
		motionBlur->Color.R = 200;
		motionBlur->Color.G = 100;
		motionBlur->Color.B = 200;
		motionBlur->Color.A = 100;
		cAssaEffect[i] = motionBlur;
	}
}

// 키메라 스킬공격 (헤딩 - 머리에 익스트림실드)
void ParkAssaParticle_ChimeraSkill(smCHAR *pChar)
{
	GetMoveLocation( 0, 0, 0, 0, pChar->Angle.y, 0);
	
 	POINT3D pos;
	pos.x = pChar->pX+GeoResult_X;
	pos.y = pChar->pY+7000;
	pos.z = pChar->pZ+GeoResult_Z;
	
	//GetMoveLocation( 0, 0, 30*fONE, 0, pHellHound->Angle.y, 0);

	
	StartSkill( pos.x, pos.y, pos.z, 0,0,0, EFFECT_CHIMERA_SKILL );
}

// Bloody Knight 스킬 공격 (로어)
void ParkAssaParticle_BloodySkill(smCHAR *pBloodKnight)
{
	StartSkill( pBloodKnight->pX, pBloodKnight->pY, pBloodKnight->pZ, 0,0,0, EFFECT_BLOODKNIGHT_SKILL, 10 );
}

// Bloody Knight 스킬 공격 피격 (스턴)
void ParkAssaParticle_BloodySkillStun(smCHAR *pChar)
{
	sinSetEffect_Stun(pChar , 90 );
}

// 헬 하운드 노말공격 (두발을 들어 대상을 내려찍음)
void ParkAssaParticle_HellHoundNormal(smCHAR *pChar)
{
	POINT3D pos;

    GetMoveLocation( 0, 0, 0, 0, pChar->Angle.y, 0);

	pos.x = pChar->pX+GeoResult_X;
	pos.y = pChar->pY+1000;
	pos.z = pChar->pZ+GeoResult_Z;
	
 	g_NewParticleMgr.Start("LostMonsterHellHoundAttack", pos);
}

// 헬 하운드 스킬공격 (입에서 불나감)
void ParkAssaParticle_HellHoundSkillAttack(smCHAR *pHellHound)
{
	GetMoveLocation( 0, 0, 50*fONE, 0, pHellHound->Angle.y, 0);
	
	int num = -1;
 	POINT3D pos;
	pos.x = pHellHound->pX+GeoResult_X;
	pos.y = pHellHound->pY+10000;
	pos.z = pHellHound->pZ+GeoResult_Z;
	
	GetMoveLocation( 0, 0, 30*fONE, 0, pHellHound->Angle.y, 0);

	//테스트용.
	D3DVECTOR  velocity;
	D3DVECTOR  gravity;
   	int i = GetAssaEffect(); 
  	velocity.x = float((float)GeoResult_X/15.f);
	velocity.y = 0.f;
	velocity.z = float((float)GeoResult_Z/15.f);

  	gravity.x = 0.f;
 	gravity.y = 0.f;
	gravity.z = 0.f;
	
	AssaSkill3BombShotExplision *explision = new AssaSkill3BombShotExplision;
	explision->Start(&pos, &velocity, &gravity, "IronMonsterBombHit2");
	cAssaEffect[i] = explision;
	
	
	num = g_NewParticleMgr.Start("MonsterIron1", pos);
	if(num != -1)
	{
		POINT3D angle;
		angle.x = 0;
		angle.y = pHellHound->Angle.y;
		angle.z = 0;
		g_NewParticleMgr.SetAngle(num, angle);
	}
}

// 헬 하운드 스킬공격의 피격시 (화염에 휩싸인듯한 이펙트)
void ParkAssaParticle_HellHoundSkillTaget(smCHAR *pChar)
{
	POINT3D pos;

    GetMoveLocation( 0, 0, 0, 0, pChar->Angle.y, 0);

	pos.x = pChar->pX+GeoResult_X;
	pos.y = pChar->pY+3000;
	pos.z = pChar->pZ+GeoResult_Z;
	
 	g_NewParticleMgr.Start("LostMonsterDarkMageHit1", pos);
}

// 다크 가드 기본공격 피격시
void ParkAssaParticle_DarkGuardTaget(smCHAR *pChar)
{
}

// 다크 가드 스킬공격
void ParkAssaParticle_DarkGuardSkill(smCHAR *pDarkGuard, smCHAR *pChar)
{
	GetMoveLocation( 0, 30*fONE, 0, 0, pDarkGuard->Angle.y, 0);
	POINT3D pCur, pDest;
	pCur.x = pDarkGuard->pX+GeoResult_X;
	pCur.y = pDarkGuard->pY+GeoResult_Y;
	pCur.z = pDarkGuard->pZ+GeoResult_Z;

	GetMoveLocation( 0, 30*fONE, 0, 0, pChar->Angle.y, 0);
	pDest.x = pChar->pX+GeoResult_X;
	pDest.y = pChar->pY+GeoResult_Y;
	pDest.z = pChar->pZ+GeoResult_Z;

	Assa = SetAssaEffect(0,"arrow.ASE",0, &pCur,0);
	
	cAssaEffect[Assa]->Time = 0;
	cAssaEffect[Assa]->AniMaxCount = 60;
	cAssaEffect[Assa]->AniDelayTime = 4;
 	cAssaEffect[Assa]->MOVECODE = ASSA_ANGLE_ROTATE;
	cAssaEffect[Assa]->Angle.y = ANGLE_1;
	cAssaEffect[Assa]->ScalePosi.x = 5;
	cAssaEffect[Assa]->ScalePosi.y = 5;
	cAssaEffect[Assa]->ScalePosi.z = 5;

	GetRadian3D( pCur.x, pCur.y, pCur.z, pDest.x, pDest.y, pDest.z);

	cAssaEffect[Assa]->Angle.x =  GeoResult_X;
	cAssaEffect[Assa]->Angle.y =  GeoResult_Y;
	cAssaEffect[Assa]->Angle.z =  0;
	
	cAssaEffect[Assa]->Angle.y = (-GeoResult_Y+ANGLE_1)&ANGCLIP;
	cAssaEffect[Assa]->NotFadeIn = TRUE;
	cAssaEffect[Assa]->ARotateSpeed.z = 100;
	int i = GetAssaEffect();
	cASSAPARTSHOT *partShot = new cASSAPARTSHOT;
	partShot->Start(&pCur, &pDest, cAssaEffect[Assa], SKILL_TWIST_JAVELIN);
	cAssaEffect[i] = partShot;
}

// 다크 메이지 기본공격
void ParkAssaParticle_DarkMageNormal(smCHAR *pDarkMage, smCHAR *pChar)
{
	GetMoveLocation( 0 , 30*fONE, 20*fONE , 0, pDarkMage->Angle.y , 0 );
	POINT3D curPos;
	POINT3D desPos;
	curPos.x = pDarkMage->pX+GeoResult_X;
	curPos.y = pDarkMage->pY+GeoResult_Y;
	curPos.z = pDarkMage->pZ+GeoResult_Z;
	
	desPos.x = pChar->pX;
	desPos.y = pChar->pY+24*fONE;
	desPos.z = pChar->pZ;

	StartEffectMonsterDest(curPos.x, curPos.y, curPos.z, desPos.x, desPos.y, desPos.z, MONSTER_PIGON_SHOT1 );
}

// 다크 메이지 스킬공격
void ParkAssaParticle_DarkMageSkill(smCHAR *pChar)
{
	ParkAssaParticle_FireWorm(pChar, 1);
}

// 다크 메이지 죽을때
void ParkAssaParticle_DarkMageDead(smCHAR *pDarkMage)
{
	GetMoveLocation( 0 , 0, 0 , 0, pDarkMage->Angle.y , 0 );
	POINT3D pos;

	pos.x = pDarkMage->pX+GeoResult_X;
	pos.y = pDarkMage->pY+3000;
	pos.z = pDarkMage->pZ+GeoResult_Z;
	
 	g_NewParticleMgr.Start("LostMonsterDarkMageDead", pos);
}


int AssaParticle_LightShot(POINT3D *pPosi, POINT3D *pDesti)
{
    g_NewParticleMgr.Start("LightStart1", *pPosi);	
	g_NewParticleMgr.Start("LightHit1", *pDesti);
	
	SetDynLight( pDesti->x, pDesti->y, pDesti->z, 255, 255, 255, 255, 150, 2);

    Assa = SetAssaEffect(100,"spark01_01.bmp",0,pPosi,0);
	AssaGetAniFileMat(cAssaEffect[Assa]);
	cAssaEffect[Assa]->AniDelayTime = 3;
	cAssaEffect[Assa]->Face.r = 255;
	cAssaEffect[Assa]->Face.g = 255;
	cAssaEffect[Assa]->Face.b = 255;
	cAssaEffect[Assa]->Face.Transparency = 255;
	cAssaEffect[Assa]->Size.w = 5000;
	cAssaEffect[Assa]->FACE_TYPE = ASSAFACE_SCALE; 
	
	cAssaEffect[Assa]->ScalePosi.x = pDesti->x - pPosi->x;
	cAssaEffect[Assa]->ScalePosi.y = pDesti->y - pPosi->y;
	cAssaEffect[Assa]->ScalePosi.z = pDesti->z - pPosi->z;

	Assa = SetAssaEffect(100,"spark01_01.bmp",0,pPosi,0);
	AssaGetAniFileMat(cAssaEffect[Assa]);
	cAssaEffect[Assa]->AniDelayTime = 3;
	cAssaEffect[Assa]->Face.r = 255;
	cAssaEffect[Assa]->Face.g = 255;
	cAssaEffect[Assa]->Face.b = 255;
	cAssaEffect[Assa]->Face.Transparency = 255;
	cAssaEffect[Assa]->Size.w = 5000;
	cAssaEffect[Assa]->FACE_TYPE = ASSAFACE_SCALE; 
	
	cAssaEffect[Assa]->ScalePosi.x = pDesti->x - pPosi->x;
	cAssaEffect[Assa]->ScalePosi.y = pDesti->y - pPosi->y;
	cAssaEffect[Assa]->ScalePosi.z = pDesti->z - pPosi->z;
	return true;
}

int AssaParticle_FireShot(POINT3D *pPosi, POINT3D *pDesti)
{
 	int i = GetAssaEffect();
	cASSAPARTSHOT *partShot = new cASSAPARTSHOT;
	partShot->Start(pPosi, pDesti, ASSA_SHOT_FIRE);
	cAssaEffect[i] = partShot;

	return TRUE;
}

int AssaParticle_IceShot(POINT3D *pPosi, POINT3D *pDesti)
{
	int i = GetAssaEffect();
	cASSAPARTSHOT *partShot = new cASSAPARTSHOT;
	partShot->Start(pPosi, pDesti, ASSA_SHOT_ICE);
	cAssaEffect[i] = partShot;

	return TRUE;
}

int AssaParticle_LightHit1(POINT3D *pPosi)
{
	SetDynLight( pPosi->x, pPosi->y, pPosi->z, 255, 255, 255, 255, 150, 2);
	g_NewParticleMgr.Start("LightHit1", *pPosi);
	return TRUE;
}

int AssaParticle_IceHit1(POINT3D *pPosi)
{
 	SetDynLight( pPosi->x, pPosi->y, pPosi->z, 150, 150, 255, 255, 120, 2);
	Assa = SetAssaEffect(0,"EW-ice.ASE",0,pPosi,0);                 
	cAssaEffect[Assa]->AniMaxCount = 20;
	cAssaEffect[Assa]->AniDelayTime = 1;
	
	g_NewParticleMgr.Start("IceHit1", *pPosi);
	return TRUE;
}

int AssaParticle_FireHit1(POINT3D *pPosi)
{
  	SetDynLight( pPosi->x, pPosi->y, pPosi->z, 255, 200, 100, 255, 120, 2);
	g_NewParticleMgr.Start("FireHit1", *pPosi);
	return TRUE;
}

int AssaParticle_FireHit2(POINT3D *pPosi)
{
	POINT3D pos;
	pos.x = pPosi->x;
	pos.y = pPosi->y;
	pos.z = pPosi->z;

	g_NewParticleMgr.Start("FireHIt2", pos);
	return TRUE;
}

int AssaParticle_BigFireHit1(POINT3D *pPosi)
{
   	SetDynLight( pPosi->x, pPosi->y, pPosi->z, 255, 200, 100, 255, 200, 2);
	g_NewParticleMgr.Start("FireBigHit1", *pPosi);
	return TRUE;
}



int AssaParticle_HolyMind_Attack(smCHAR *pChar, int liveCount)
{
     StartEffectMonster(pChar->pX, pChar->pY, pChar->pZ, MONSTER_SERQBUS_MAGIC3);
	 if(!AddAssaCodeEffectTime(pChar, HOLY_MIND_ATTACK, liveCount*70))
	 {
		//StopAssaCodeEffect(pChar, HOLY_MIND_ATTACK);
		//실제로 유지되는 파티클 호출..
		int i = GetAssaEffect();
		cASSAChar *partChar = new cASSAChar;
		partChar->Start(pChar, HOLY_MIND_ATTACK, liveCount*70);
		cAssaEffect[i] = partChar;
	 }
	 return TRUE;
}

/*
int AssaParticle_AgingBody(smCHAR *pChar)
{
	int i = GetAssaEffect();
	cASSAItem *partItem = new cASSAItem;
	partItem->Start(pChar, AGING_BODY);
	cAssaEffect[i] = partItem;
	
	return TRUE;
}
*/

int AssaParticle_Fury_Eye(smCHAR *pChar)
{
	if(GetAssaCodeEffect(pChar, FURY_EYE1) == FALSE)
	{
  		int i = GetAssaEffect();
		cAssaTraceCharMeshParticle *traceMeshParticle = new cAssaTraceCharMeshParticle;
		 traceMeshParticle->Start(pChar, "Bip EyeT01", "FuryEye");
		traceMeshParticle->SetScale(float(pChar->smCharInfo.wPlayClass[1])/256.f);
		cAssaEffect[i] = traceMeshParticle;
		cAssaEffect[i]->CODE = FURY_EYE1;
		

 		i = GetAssaEffect();
		traceMeshParticle = NULL;
		traceMeshParticle = new cAssaTraceCharMeshParticle;
		traceMeshParticle->Start(pChar, "Bip EyeT02", "FuryEye");
		traceMeshParticle->SetScale(float(pChar->smCharInfo.wPlayClass[1])/256.f);
		cAssaEffect[i] = traceMeshParticle;
		cAssaEffect[i]->CODE = FURY_EYE1;
	}

	return TRUE;
}

int AssaParticle_Fury_ShotFire(smCHAR *pChar, POINT3D *destPos)
{
 	smOBJ3D *searchObj = pChar->AnimPattern->GetObjectFromName("Bip MouthT"); //오브젝트를 읽는다
	POINT3D curPos;
	if(AssaSearchObjPos(pChar, searchObj, &curPos))
	{
		int i = GetAssaEffect();
		cASSAPARTSHOT *partShot = new cASSAPARTSHOT;
		partShot->Start(&curPos, destPos, ASSA_SHOT_BIGFIRE1);
		cAssaEffect[i] = partShot;
	}
	//AssaParticle_FireShot(&curPos, destPos);

	return TRUE;
}

int AssaParticle_Fury_RespawnBody(smCHAR *pChar)
{
  	int i = GetAssaEffect();
	cAssaTraceCharMeshParticle *traceMeshParticle = new cAssaTraceCharMeshParticle;
 	traceMeshParticle->Start(pChar, "Bip MouthT", "FuryRespawnMouse");
	cAssaEffect[i] = traceMeshParticle;
	
	POINT3D pos;
	pos.x = pChar->pX;
	pos.y = pChar->pY+2000;
	pos.z = pChar->pZ;

	g_NewParticleMgr.Start("FuryRespawnDown", pos);
	return TRUE;
}

int AssaParticle_Fury_RespawnMonster(POINT3D *pPosi)
{
	g_NewParticleMgr.Start("FuryRespawn", *pPosi);
	Assa = SetAssaEffect(0,"m_fury.ASE",0,pPosi,0);
	cAssaEffect[Assa]->Time = 0;
	cAssaEffect[Assa]->AniMaxCount = 20;
	cAssaEffect[Assa]->AniDelayTime = 5;
	
	
	return TRUE;
}


int AssaParticle_Fury_Skill1(POINT3D *pPosi, POINT3D *pAngle)
{
	POINT3D posi;
	POINT3D floorPosi;
    posi.x = pPosi->x;
	posi.y = pPosi->y;
	posi.z = pPosi->z;
	
 	floorPosi.x = pPosi->x;
	floorPosi.y = pPosi->y+2500;
	floorPosi.z = pPosi->z;
	
 	g_NewParticleMgr.Start("FuryHit1", posi);
	g_NewParticleMgr.Start("FuryHit2", floorPosi);

    Assa = SetAssaEffect(0,"m_furyhead.ASE",0,&posi,0);
	cAssaEffect[Assa]->Time = -2;
	cAssaEffect[Assa]->AniMaxCount = 30;
	cAssaEffect[Assa]->AniDelayTime = 3;
	cAssaEffect[Assa]->Angle.y = (-pAngle->y+ANGLE_180)&ANGCLIP;

	Assa = SetAssaEffect(0,"m_furyhead.ASE",0,&posi,0);
	cAssaEffect[Assa]->Time = -8;
	cAssaEffect[Assa]->AniMaxCount = 30;
	cAssaEffect[Assa]->AniDelayTime = 4;
	cAssaEffect[Assa]->Angle.y = (-pAngle->y+ANGLE_180)&ANGCLIP;
	
	Assa = SetAssaEffect(0,"m_fury-jin.ASE",0,&posi,0);
	cAssaEffect[Assa]->Time = -2;
	cAssaEffect[Assa]->AniMaxCount = 30;
	cAssaEffect[Assa]->AniDelayTime = 4;
	cAssaEffect[Assa]->Angle.y = (-pAngle->y+ANGLE_180)&ANGCLIP;

	return TRUE;
}

int AssaParticle_Fury_Skill1(smCHAR *pChar)
{
	POINT3D posi;
	POINT3D floorPosi;
    posi.x = pChar->pX;
	posi.y = pChar->pY;
	posi.z = pChar->pZ;
	
 	floorPosi.x = pChar->pX;
	floorPosi.y = pChar->pY+2500;
	floorPosi.z = pChar->pZ;
	
 	g_NewParticleMgr.Start("FuryHit1", posi);
	g_NewParticleMgr.Start("FuryHit2", floorPosi);

    Assa = SetAssaEffect(0,"m_furyhead.ASE",0,&posi,0);
	cAssaEffect[Assa]->Time = -2;
	cAssaEffect[Assa]->AniMaxCount = 30;
	cAssaEffect[Assa]->AniDelayTime = 3;
	cAssaEffect[Assa]->Angle.y = (-pChar->Angle.y+ANGLE_180)&ANGCLIP;

	Assa = SetAssaEffect(0,"m_furyhead.ASE",0,&posi,0);
	cAssaEffect[Assa]->Time = -8;
	cAssaEffect[Assa]->AniMaxCount = 30;
	cAssaEffect[Assa]->AniDelayTime = 4;
	cAssaEffect[Assa]->Angle.y = (-pChar->Angle.y+ANGLE_180)&ANGCLIP;
	
	Assa = SetAssaEffect(0,"m_fury-jin.ASE",0,&posi,0);
	cAssaEffect[Assa]->Time = -2;
	cAssaEffect[Assa]->AniMaxCount = 30;
	cAssaEffect[Assa]->AniDelayTime = 4;
	cAssaEffect[Assa]->Angle.y = (-pChar->Angle.y+ANGLE_180)&ANGCLIP;

	return TRUE;
}

void AssaParticle_CastleRed(smCHAR *pChar)
{
	POINT3D posi;
	POINT3D floorPosi;
    posi.x = pChar->pX;
	posi.y = pChar->pY;
	posi.z = pChar->pZ;
	
 	floorPosi.x = pChar->pX;
	floorPosi.y = pChar->pY+2500;
	floorPosi.z = pChar->pZ;
	
 	g_NewParticleMgr.Start("FuryHit1", posi);
	g_NewParticleMgr.Start("FuryHit2", floorPosi);

	Assa = SetAssaEffect(0,"m_fury-jin.ASE",0,&posi,0);
	cAssaEffect[Assa]->Time = -2;
	cAssaEffect[Assa]->AniMaxCount = 30;
	cAssaEffect[Assa]->AniDelayTime = 4;
	cAssaEffect[Assa]->Angle.y = (-pChar->Angle.y+ANGLE_180)&ANGCLIP;
}

void AssaParticle_CastleGreen(smCHAR *pChar)
{
	POINT3D posi;
	POINT3D floorPosi;
    posi.x = pChar->pX;
	posi.y = pChar->pY;
	posi.z = pChar->pZ;
	
 	floorPosi.x = pChar->pX;
	floorPosi.y = pChar->pY+2500;
	floorPosi.z = pChar->pZ;
	
 	g_NewParticleMgr.Start("FuryHit1_g", posi);
	g_NewParticleMgr.Start("FuryHit2_g", floorPosi);

	Assa = SetAssaEffect(0,"m_fury-jin_g.ASE",0,&posi,0);
	cAssaEffect[Assa]->Time = -2;
	cAssaEffect[Assa]->AniMaxCount = 30;
	cAssaEffect[Assa]->AniDelayTime = 4;
	cAssaEffect[Assa]->Angle.y = (-pChar->Angle.y+ANGLE_180)&ANGCLIP;
}

void AssaParticle_CastleBlue(smCHAR *pChar)
{
	POINT3D posi;
	POINT3D floorPosi;
    posi.x = pChar->pX;
	posi.y = pChar->pY;
	posi.z = pChar->pZ;
	
 	floorPosi.x = pChar->pX;
	floorPosi.y = pChar->pY+2500;
	floorPosi.z = pChar->pZ;
	
 	g_NewParticleMgr.Start("FuryHit1_b", posi);
	g_NewParticleMgr.Start("FuryHit2_b", floorPosi);

	Assa = SetAssaEffect(0,"m_fury-jin_b.ASE",0,&posi,0);
	cAssaEffect[Assa]->Time = -2;
	cAssaEffect[Assa]->AniMaxCount = 30;
	cAssaEffect[Assa]->AniDelayTime = 4;
	cAssaEffect[Assa]->Angle.y = (-pChar->Angle.y+ANGLE_180)&ANGCLIP;
}


int StartEffect_Serqbus_Skill1(smCHAR *pChar, int liveCount)
{
	//StartEffectMonster(pChar->pX, pChar->pY+pChar->PatHeight, pChar->pZ, MONSTER_SERQBUS_MAGIC_ATTACK1);
	if(!AddAssaCodeEffectTime(pChar, MONSTER_SERQBUS_STATE1, liveCount*70))
 	{
		int i = GetAssaEffect();
		cASSAChar *partChar = new cASSAChar;
		partChar->Start(pChar, MONSTER_SERQBUS_STATE1, liveCount*70);
		cAssaEffect[i] = partChar;
	}

	return TRUE;
}

int StartEffect_Serqbus_Skill2(smCHAR *pChar, int liveCount)
{
	//StartEffectMonster(pChar->pX, pChar->pY+pChar->PatHeight, pChar->pZ, MONSTER_SERQBUS_MAGIC_ATTACK2);
	if(!AddAssaCodeEffectTime(pChar, MONSTER_SERQBUS_STATE2, liveCount*70))
	{
		int i = GetAssaEffect();
		cASSAChar *partChar = new cASSAChar;
		partChar->Start(pChar, MONSTER_SERQBUS_STATE2, liveCount*70);
		cAssaEffect[i] = partChar;
 	}
	return TRUE;
}

int StartEffect_Serqbus_Skill3(smCHAR *pChar, int liveCount)
{
	//StartEffectMonster(pChar->pX, pChar->pY+pChar->PatHeight, pChar->pZ, MONSTER_SERQBUS_MAGIC_ATTACK3);
	if(!AddAssaCodeEffectTime(pChar, MONSTER_SERQBUS_STATE3, liveCount*70))
	{
		int i = GetAssaEffect();
		cASSAChar *partChar = new cASSAChar;
		partChar->Start(pChar, MONSTER_SERQBUS_STATE3, liveCount*70);
		cAssaEffect[i] = partChar;
	}
	return TRUE;
}

int AssaParticle_EvilMapleShot(smCHAR *pChar)//(POINT3D *pPosi, POINT3D *pAngle)
{
	POINT3D posi;
  	posi.x = pChar->pX;
	posi.y = pChar->pY+8000;
	posi.z = pChar->pZ;

  	Assa = SetAssaEffect(0,"m_EvilMaple.ASE",0,&posi,0);
	cAssaEffect[Assa]->AniMaxCount = 30;
	cAssaEffect[Assa]->AniDelayTime = 3;
	cAssaEffect[Assa]->Angle.x = (ANGLE_90)&ANGCLIP;
	cAssaEffect[Assa]->Angle.y = (-pChar->Angle.y+ANGLE_180)&ANGCLIP;
	
	GetMoveLocation( 0, 0, 25*fONE, 0, pChar->Angle.y, 0);
	posi.x += GeoResult_X;
	posi.z += GeoResult_Z;
	g_NewParticleMgr.Start("EvilMaple", posi);

	

	return TRUE;
}

int AssaParticle_EvilPlantShot(smCHAR *pChar)//(POINT3D *pPosi, POINT3D *pAngle)
{
	POINT3D posi;
      posi.x = pChar->pX;
	posi.y = pChar->pY+8000;
	posi.z = pChar->pZ;

      Assa = SetAssaEffect(0,"m_EvilPlant.ASE",0,&posi,0,5);
	
 	cAssaEffect[Assa]->AniMaxCount = 30;
	cAssaEffect[Assa]->AniDelayTime = 3;
	cAssaEffect[Assa]->Angle.x = (ANGLE_90)&ANGCLIP;
	cAssaEffect[Assa]->Angle.y = (-pChar->Angle.y+ANGLE_180)&ANGCLIP;
	
	Assa = SetAssaEffect(0,"m_EvilPlant2.ASE",0,&posi,0, 5);
	cAssaEffect[Assa]->AniMaxCount = 30;
	cAssaEffect[Assa]->AniDelayTime = 3;
	cAssaEffect[Assa]->Angle.x = (ANGLE_90)&ANGCLIP;
	cAssaEffect[Assa]->Angle.y = (-pChar->Angle.y+ANGLE_180)&ANGCLIP;
	

   	GetMoveLocation( 0, 0, 25*fONE, 0, pChar->Angle.y, 0);
	posi.x += GeoResult_X;
	posi.z += GeoResult_Z;
	g_NewParticleMgr.Start("EvilPlant", posi);
	return TRUE;
}

int AssaParticle_ThornCrawlerShot(smCHAR *pChar, POINT3D *destPos)
{
 	smPAT3D *pat = (smPAT3D *)AssaSearchRes("m_thorncrowler.ase", SMMAT_BLEND_LAMP);
	if(pat == NULL)
		return FALSE;
	
	POINT3D curPos;
	curPos.x = pChar->pX;
	curPos.y = pChar->pY+9000;
	curPos.z = pChar->pZ;
	
	D3DVECTOR term;
	term.x = (float)(destPos->x - curPos.x)/(float)fONE;
	term.y = (float)(destPos->y - curPos.y)/(float)fONE;
	term.z = (float)(destPos->z - curPos.z)/(float)fONE;
	
	float length;
 	length = (float)sqrt(term.x*term.x+term.y*term.y+term.z*term.z);
	
	int curLength;
	curLength = (int)(length*fONE);
	
 	if(curLength > 50000)
	{
		destPos->x = pChar->pX;
		destPos->y = pChar->pY;
		destPos->z = pChar->pZ;
		
		GetMoveLocation( 0, 0, 50000, pChar->Angle.x, pChar->Angle.y, 0);
		destPos->x += GeoResult_X;
		destPos->z += GeoResult_Z;
	}
	
	if(curLength < 40000)
	{
		destPos->x = pChar->pX;
		destPos->y = pChar->pY;
		destPos->z = pChar->pZ;
		
		GetMoveLocation( 0, 0, 40000, pChar->Angle.x, pChar->Angle.y, 0);
		destPos->x += GeoResult_X;
		destPos->z += GeoResult_Z;
	}

	int i = GetAssaEffect();
 	cAssaTraceMeshDestParticle *traceMeshDestParticle = new cAssaTraceMeshDestParticle;
 	traceMeshDestParticle->Start(pat,"Box01", &curPos, destPos, "ThornCrawler");
	cAssaEffect[i] = traceMeshDestParticle;

	
	/*
  	GetMoveLocation( 0, 0, -15*fONE, pChar->Angle.x, pChar->Angle.y, 0);
	posi.x += GeoResult_X;
	posi.z += GeoResult_Z;

    Assa = SetAssaEffect(0,"m_thorncrowler.ASE",0,&posi,0, 1);
	cAssaEffect[Assa]->AniMaxCount = 20;
	cAssaEffect[Assa]->AniDelayTime = 2;
	cAssaEffect[Assa]->Angle.y = (-pChar->Angle.y+ANGLE_180)&ANGCLIP;
	cAssaEffect[Assa]->NotFadeIn = TRUE;
	*/

    //GetMoveLocation( 0, 0, 25*fONE, pChar->Angle.x, pChar->Angle.y, 0);
	//posi.x += GeoResult_X;
	//posi.z += GeoResult_Z;
	//posi.y += fONE*30;
	//g_NewParticleMgr.Start("ThornCrawler", posi);
	
	return TRUE;
}

int AssaParticle_StoneGolemShot(smCHAR *pChar)
{
	POINT3D pos;
     pos.x = pChar->pX;
	pos.y = pChar->pY;
	pos.z = pChar->pZ;
	
	GetMoveLocation( 0, 0, -30*fONE, pChar->Angle.x, pChar->Angle.y, 0);
	pos.x += GeoResult_X;
	pos.z += GeoResult_Z;
    
	Assa = SetAssaEffect(0,"m_StoneGolem.ASE",0,&pos,0, 1);
	cAssaEffect[Assa]->AniMaxCount = 30;
	cAssaEffect[Assa]->AniDelayTime = 2;
	cAssaEffect[Assa]->Angle.y = (-pChar->Angle.y+ANGLE_180)&ANGCLIP;
	cAssaEffect[Assa]->NotFadeIn = TRUE;
	
	pos.x = pos.x;
	pos.y = pos.y+1500;
	pos.z = pos.z;
	
 	GetMoveLocation( 0, 0, 120*fONE, pChar->Angle.x, pChar->Angle.y, 0);
	pos.x += GeoResult_X;
	pos.z += GeoResult_Z;
	g_NewParticleMgr.Start("StoneGolemHit1", pos);
	

	return TRUE;
}

int AssaParticle_HeavyGoblinHit1(smCHAR *pChar)
{
    int i = GetAssaEffect();
 	cAssaTraceCharMeshParticle *traceMeshParticle = new cAssaTraceCharMeshParticle;
 	traceMeshParticle->Start(pChar, "Bip-hammer", "HeavyGoblin", false, 130);
	cAssaEffect[i] = traceMeshParticle;

	POINT3D pos;
 	pos.x = pChar->pX;
	pos.y = pChar->pY;
	pos.z = pChar->pZ;

  
    GetMoveLocation( 0, 0, 50*fONE, pChar->Angle.x, pChar->Angle.y, 0);
	pos.x += GeoResult_X;
	pos.z += GeoResult_Z;
	pos.y += fONE*10;
	g_NewParticleMgr.Start("HeavyGoblinHit1", pos, 1.2f);

	return TRUE;
}

int AssaParticle_HeavyGoblinHit2(smCHAR *pChar)
{
    int i = GetAssaEffect();
 	cAssaTraceCharMeshParticle *traceMeshParticle = new cAssaTraceCharMeshParticle;
 	traceMeshParticle->Start(pChar, "Bip-hammer", "HeavyGoblin", false, 90);
	cAssaEffect[i] = traceMeshParticle;

	POINT3D pos;
 	pos.x = pChar->pX;
	pos.y = pChar->pY;
	pos.z = pChar->pZ;

  
    GetMoveLocation( 0, 0, 50*fONE, pChar->Angle.x, pChar->Angle.y, 0);
	pos.x += GeoResult_X;
	pos.z += GeoResult_Z;
	pos.y += fONE*10;
	g_NewParticleMgr.Start("HeavyGoblinHit1", pos, 0.2f);

	return TRUE;
}


int AssaParticle_DoomGuardHit1(smCHAR *pChar)
{
 	int   iScale = pChar->smCharInfo.wPlayClass[1];
	float fScale = float(pChar->smCharInfo.wPlayClass[1])/256.f;
	
	if(iScale <= 1)
		iScale = 0;
	if(fScale <= 1.f)
		fScale = 1.f;

	int i = GetAssaEffect();
  	cAssaTraceCharMeshParticle *traceMeshParticle = new cAssaTraceCharMeshParticle;
	if(fScale <= 1.f)
  		traceMeshParticle->Start(pChar, "Bip-skill01", "DoomGuard", false, 140);
	else
		traceMeshParticle->Start(pChar, "Bip-skill01", "DoomGuardBig", false, 140);
	
	traceMeshParticle->SetScale(fScale);
 	cAssaEffect[i] = traceMeshParticle;
	
	i = GetAssaEffect();
	cAssaMotionBlur *motionBlur = new cAssaMotionBlur;
	motionBlur->Start(pChar, "Bip-skill01", "Bip-skill", 50);
	motionBlur->SetScale(fScale);
	cAssaEffect[i] = motionBlur;
	
	POINT3D pos;
 	pos.x = pChar->pX;
	pos.y = pChar->pY;
	pos.z = pChar->pZ;
	
  	GetMoveLocation( 0, 0, 50*fONE+iScale, pChar->Angle.x, pChar->Angle.y, 0);
	pos.x += GeoResult_X;
	pos.z += GeoResult_Z;
	pos.y += fONE*10;
	
	if(fScale <= 1.f)
   		g_NewParticleMgr.Start("MonsterHit1", pos, 0.5f);
	else 
		g_NewParticleMgr.Start("DoomGuardBigHit", pos, 0.5f);

	return TRUE;
}

void AssaMotionBlur(smCHAR *pChar, char *meshName1, char *meshName2, int timeCount)
{
	int i = GetAssaEffect();
	cAssaMotionBlur *motionBlur = new cAssaMotionBlur;
	motionBlur->Start(pChar, meshName1, meshName2, timeCount);
	cAssaEffect[i] = motionBlur;
}

int AssaParticle_DoomGuardHit2(smCHAR *pChar)
{
	int   iScale = pChar->smCharInfo.wPlayClass[1];
	float fScale = float(pChar->smCharInfo.wPlayClass[1])/256.f;

	if(iScale <= 1)
		iScale = 0;
	if(fScale <= 1.f)
		fScale = 1.f;

 	int i = GetAssaEffect();
	cAssaTraceCharMeshParticle *traceMeshParticle = new cAssaTraceCharMeshParticle;
 	if(fScale <= 1.f)
		traceMeshParticle->Start(pChar, "Bip-skill01", "DoomGuard", false, 160);
	else
		traceMeshParticle->Start(pChar, "Bip-skill01", "DoomGuardBig", false, 160);
	
	traceMeshParticle->SetScale(fScale);
 	
	cAssaEffect[i] = traceMeshParticle;
	
	i = GetAssaEffect();
	cAssaMotionBlur *motionBlur = new cAssaMotionBlur;
	motionBlur->Start(pChar, "Bip-skill01", "Bip-skill", 150);
	motionBlur->SetScale(fScale);
	cAssaEffect[i] = motionBlur;
	
	POINT3D pos;
 	pos.x = pChar->pX;
	pos.y = pChar->pY;
	pos.z = pChar->pZ;
	
   	GetMoveLocation( 0, 0, 70*fONE+iScale, pChar->Angle.x, pChar->Angle.y, 0);
	pos.x += GeoResult_X;
	pos.z += GeoResult_Z;
	pos.y += fONE*20;
	
	if(fScale <= 1.f)
	{
		g_NewParticleMgr.Start("MonsterHit1", pos, 0.3f);
		g_NewParticleMgr.Start("MonsterHit1", pos, 1.f);
	}
	else
	{
		g_NewParticleMgr.Start("DoomGuardBigHit", pos, 0.3f);
		g_NewParticleMgr.Start("DoomGuardBigHit", pos, 1.f);
	}
	return TRUE;
}


void AssaParticle_MonsterIronFist(smCHAR *pChar)
{
       int i = GetAssaEffect();
	cAssaMotionBlur *motionBlur = new cAssaMotionBlur;
	//motionBlur->Start(pChar, "Bip01 L Hand", "Bip01 Box05", 60);
 	motionBlur->Start(pChar, "Bip01 Box05", "Bip01 L Hand", 45);
	motionBlur->SetDelay(15);
	motionBlur->SetAlpha(-180);
	cAssaEffect[i] = motionBlur;
    
	i = GetAssaEffect();
	motionBlur = new cAssaMotionBlur;
	//motionBlur->Start(pChar, "Bip01 R Hand", "Bip01 Box08", 60);
	motionBlur->Start(pChar, "Bip01 Box08", "Bip01 R Hand", 45);
	motionBlur->SetDelay(15);
	motionBlur->SetAlpha(-180);
	cAssaEffect[i] = motionBlur;
}

void AssaParticle_MonsterIronMorgonHit(smCHAR *pChar)
{
	POINT3D pos;
  	pos.x = pChar->pX;
	pos.y = pChar->pY;
	pos.z = pChar->pZ;
	
   	//GetMoveLocation( 0, 0,20*fONE, 0, pChar->Angle.y, 0);
     GetMoveLocation( 0, 0, 30*fONE, pChar->Angle.x, pChar->Angle.y, 0);

	pos.x += GeoResult_X;
	pos.z += GeoResult_Z;

	g_NewParticleMgr.Start("IronMonsterMorgon", pos);
}

int AssaParticle_Hit1(POINT3D *destPos)
{
	g_NewParticleMgr.Start("MonsterHit1", *destPos);
	return TRUE;
}

int AssaParticle_SnailShot(smCHAR *pChar, POINT3D *destPos)
{
    smPAT3D *pat = (smPAT3D *)AssaSearchRes("m_snail.ase", SMMAT_BLEND_LAMP);
	if(pat == NULL)
		return FALSE;
	
	POINT3D curPos;
	curPos.x = pChar->pX;
	curPos.y = pChar->pY+5000;
	curPos.z = pChar->pZ;
	
	GetMoveLocation( 0, 0, -5*fONE, pChar->Angle.x, pChar->Angle.y, 0);
	curPos.x += GeoResult_X;
	curPos.z += GeoResult_Z;
	
	
	D3DVECTOR term;
	term.x = (float)(destPos->x - curPos.x)/(float)fONE;
	term.y = (float)(destPos->y - curPos.y)/(float)fONE;
	term.z = (float)(destPos->z - curPos.z)/(float)fONE;
	
	float length;
 	length = (float)sqrt(term.x*term.x+term.y*term.y+term.z*term.z);
	
	int curLength;
	curLength = (int)(length*fONE);
	
 	if(curLength > 50000)
	{
		destPos->x = pChar->pX;
		destPos->y = pChar->pY;
		destPos->z = pChar->pZ;
		
		GetMoveLocation( 0, 0, 50000, pChar->Angle.x, pChar->Angle.y, 0);
		destPos->x += GeoResult_X;
		destPos->z += GeoResult_Z;
	}
	
	if(curLength < 40000)
	{
		destPos->x = pChar->pX;
		destPos->y = pChar->pY;
		destPos->z = pChar->pZ;
		
		GetMoveLocation( 0, 0, 40000, pChar->Angle.x, pChar->Angle.y, 0);
		destPos->x += GeoResult_X;
		destPos->z += GeoResult_Z;
	}

	int i = GetAssaEffect();
 	cAssaTraceMeshDestParticle *traceMeshDestParticle = new cAssaTraceMeshDestParticle;
 	traceMeshDestParticle->Start(pat,"GeoSphere01", &curPos, destPos, "Snail");
	traceMeshDestParticle->SetRenderMesh(false);
	cAssaEffect[i] = traceMeshDestParticle;
	
	return TRUE;
}


int AssaParticle_NightMare(smCHAR *pChar)
{
	POINT3D posi;

    posi.x = pChar->pX;
	posi.y = pChar->pY+10000;
 	posi.z = pChar->pZ;

    Assa = SetAssaEffect(0,"m_CaveMummy.ASE",0,&posi,0);

	cAssaEffect[Assa]->AniMaxCount = 30;
	cAssaEffect[Assa]->AniDelayTime = 1;
	cAssaEffect[Assa]->Angle.y = (-pChar->Angle.y+ANGLE_180)&ANGCLIP;
	
 	POINT3D posi2;
 	posi2.x = pChar->pX;
	posi2.y = pChar->pY+12000;
 	posi2.z = pChar->pZ;
	
	Assa = SetAssaEffect(0,"m_CaveMummy1.ASE",0,&posi2,0);
	cAssaEffect[Assa]->AniMaxCount = 30;
	cAssaEffect[Assa]->AniDelayTime = 2;
	cAssaEffect[Assa]->Angle.y = (-pChar->Angle.y+ANGLE_180)&ANGCLIP;
	
	
 	GetMoveLocation( 0, 0, 50*fONE, pChar->Angle.x, pChar->Angle.y, 0);
	posi.x += GeoResult_X;
	posi.z += GeoResult_Z;
	posi.y -= fONE*20;
	g_NewParticleMgr.Start("CaveMummy", posi);

	return TRUE;
}

int AssaParticle_RatooHit1(smCHAR *pChar)
{
	
    int i = GetAssaEffect();
 	cAssaTraceCharMeshParticle *traceMeshParticle = new cAssaTraceCharMeshParticle;
 	traceMeshParticle->Start(pChar, "Bip-skill-02", "Ratoo", false, 60);
  	traceMeshParticle->SetPosLength(-20*fONE);
 	cAssaEffect[i] = traceMeshParticle;
	
	i = GetAssaEffect();
 	traceMeshParticle = new cAssaTraceCharMeshParticle;
 	traceMeshParticle->Start(pChar, "Bip-skill-02", "Ratoo", false, 50);
  	traceMeshParticle->SetPosLength(-45*fONE);
 	cAssaEffect[i] = traceMeshParticle;

	i = GetAssaEffect();
 	traceMeshParticle = new cAssaTraceCharMeshParticle;
 	traceMeshParticle->Start(pChar, "Bip-skill-02", "Ratoo", false, 40);
  	traceMeshParticle->SetPosLength(-70*fONE);
 	cAssaEffect[i] = traceMeshParticle;
	
	
	i = GetAssaEffect();
	cAssaMotionBlur *motionBlur = new cAssaMotionBlur;
	motionBlur->Start(pChar, "Bip-skill-01", "Bip-skill-02", 20);
	motionBlur->SetDelay(2);
	cAssaEffect[i] = motionBlur;


	POINT3D pos;
 	pos.x = pChar->pX;
	pos.y = pChar->pY;
	pos.z = pChar->pZ;

  
    GetMoveLocation( 0, 0, 150*fONE, pChar->Angle.x, pChar->Angle.y, 1);
	pos.x += GeoResult_X;
	pos.z += GeoResult_Z;
	pos.y += fONE*20;
	g_NewParticleMgr.Start("RatooHit1", pos, 0);
	
     pos.y -= fONE*20;
  	Assa = SetAssaEffect(0,"m_ratoo.ASE",0,&pos,0);
	cAssaEffect[Assa]->Time = -4;
	cAssaEffect[Assa]->AniMaxCount = 20;
	cAssaEffect[Assa]->AniDelayTime = 5;
	cAssaEffect[Assa]->Angle.y = (-pChar->Angle.y+ANGLE_180)&ANGCLIP;
	
    SetDynLight( pos.x, pos.y, pos.z, 100, 150, 200, 255, 350, 5);
	
	return TRUE;
}

int AssaParticleSpiderSkill1(smCHAR *pChar)
{
	POINT3D posi;
    posi.x = pChar->pX;
	posi.y = pChar->pY;
	posi.z = pChar->pZ;
	
	Assa = SetAssaEffect(0,"m_Spider.ASE",0,&posi,0, 1);
 	cAssaEffect[Assa]->AniMaxCount = 110;
	cAssaEffect[Assa]->AniDelayTime = 2;
	cAssaEffect[Assa]->Angle.y = (-pChar->Angle.y+ANGLE_180)&ANGCLIP;
	
	return TRUE;
}

int AssaParticleClanMonster(smCHAR *pChar)
{
  	if(GetAssaCodeEffect(pChar, CLAN_MONSTER_PART1) == FALSE)
	{
    	int i = GetAssaEffect();
		cAssaTraceCharMeshParticle *traceMeshParticle = new cAssaTraceCharMeshParticle;
		traceMeshParticle->Start(pChar, "Bip01 center", "ClanMonster1");
		cAssaEffect[i] = traceMeshParticle;
		cAssaEffect[i]->CODE = CLAN_MONSTER_PART1;

		i = GetAssaEffect();
		traceMeshParticle = new cAssaTraceCharMeshParticle;
		traceMeshParticle->Start(pChar, "Bip01 center01", "ClanMonster2");
		
		cAssaEffect[i] = traceMeshParticle;
		cAssaEffect[i]->CODE = CLAN_MONSTER_PART1;

		i = GetAssaEffect();
		traceMeshParticle = new cAssaTraceCharMeshParticle;
		traceMeshParticle->Start(pChar, "Bip01 center02", "ClanMonster2");
		
		cAssaEffect[i] = traceMeshParticle;
		cAssaEffect[i]->CODE = CLAN_MONSTER_PART1;


		i = GetAssaEffect();
		cAssaTwoPointAni *twoPointAni = new cAssaTwoPointAni;
		twoPointAni->Start(pChar, "bip01 s01", "bip01 s02");

		cAssaEffect[i] = twoPointAni;
		cAssaEffect[i]->CODE = CLAN_MONSTER_PART1;
	}

 	return TRUE;
}

int AssaParticleClanMonsterHit1(smCHAR *pChar)
{
 	
	if(pChar->AnimPattern == NULL)
		return FALSE;
	
 	smOBJ3D *SearchObj1 = pChar->AnimPattern->GetObjectFromName("bip01 s01"); //오브젝트를 읽는다
	if(SearchObj1 == NULL)
		return FALSE;
	POINT3D pos;

	smMATRIX *mWorld;
	mWorld = &SearchObj1->mWorld;
	POINT3D angle;
	memcpy( &angle, &pChar->Angle , sizeof( POINT3D ) );
 	angle.y = (-angle.y+ANGLE_180)&ANGCLIP;
	
	AnimObjectTree( SearchObj1, 198400, angle.x, angle.y, angle.z);
	
	pos.x = pChar->pX + mWorld->_41;
	pos.z = pChar->pZ + mWorld->_42;
	pos.y = pChar->pY + mWorld->_43;

	//AssaSearchObjPos(pChar, SearchObj1, &pos);
	
 	pos.y = pChar->pY;

	int i = GetAssaEffect();
	cASSAPARTSHOT *partShot = new cASSAPARTSHOT;
	
	POINT3D curPos;
	POINT3D desPos;
	curPos.x = pos.x;
	curPos.y = pos.y+50000;
	curPos.z = pos.z;

	desPos.x = pos.x;
	desPos.y = pos.y+5000;
	desPos.z = pos.z-20000;
 	partShot->Start(&curPos, &desPos, ASSA_SHOT_SPARK1);
	cAssaEffect[i] = partShot;

    i = GetAssaEffect();
	partShot = new cASSAPARTSHOT;
	
	
	curPos.x = pos.x;
	curPos.y = pos.y+50000;
	curPos.z = pos.z;

	desPos.x = pos.x;
	desPos.y = pos.y+5000;
	desPos.z = pos.z+20000;
	partShot->Start(&curPos, &desPos, ASSA_SHOT_SPARK1);
	cAssaEffect[i] = partShot;


	i = GetAssaEffect();
	partShot = new cASSAPARTSHOT;
	curPos.x = pos.x;
	curPos.y = pos.y+50000;
	curPos.z = pos.z;

	desPos.x = pos.x+20000;
	desPos.y = pos.y+5000;
	desPos.z = pos.z;
	partShot->Start(&curPos, &desPos, ASSA_SHOT_SPARK1);
	cAssaEffect[i] = partShot;


	i = GetAssaEffect();
	partShot = new cASSAPARTSHOT;
	curPos.x = pos.x;
	curPos.y = pos.y+50000;
	curPos.z = pos.z;

	desPos.x = pos.x-20000;
	desPos.y = pos.y+5000;
	desPos.z = pos.z;

	partShot->Start(&curPos, &desPos, ASSA_SHOT_SPARK1);
	cAssaEffect[i] = partShot;
	

	 return TRUE;
}


int AssaParticleClanMonsterHit2(smCHAR *pChar)
{
  	int i = GetAssaEffect();
    cAssaTraceCharMeshParticle *traceMeshParticle = new cAssaTraceCharMeshParticle;
   	traceMeshParticle->Start(pChar, "bip01 s01", "DoomGuard", false, 50);
   	//traceMeshParticle->SetPosLength(10*fONE);
	
	cAssaEffect[i] = traceMeshParticle;
	i = GetAssaEffect();
	cAssaMotionBlur *motionBlur = new cAssaMotionBlur;
   	motionBlur->Start(pChar, "bip01 s01", "bip01 s02", 25);
	cAssaEffect[i] = motionBlur;
	
	return TRUE;
}

int AssaParticle_Sprak1(POINT3D *pPosi)
{
    for(int index = 0; index < 4; index++)
	{
		int i = GetAssaEffect();
		cASSAPARTSPARK *partSpark = new cASSAPARTSPARK;
		
		partSpark->Start(pPosi, "spark01_01.bmp");
		partSpark->Time = GetRandomPos(0,15);
		cAssaEffect[i] = partSpark;
	}
	SetDynLight( pPosi->x, pPosi->y, pPosi->z, 200, 255, 255, 200, 100, 2);
	
 	g_NewParticleMgr.Start("ClanMonster4", *pPosi);
	//g_NewParticleMgr.Start("ClanMonster4", *pPosi);
	//g_NewParticleMgr.Start("DivineLightning", *pPosi);
	
	return TRUE;
}




void AssaParticle_Concentration(smCHAR *pChar, int liveCount)
{
	StopAssaCodeEffect(pChar, SKILL_CONCENTRATION);
	POINT3D pos;
 	memcpy(&pos, &pChar->Posi, sizeof(POINT3D));
   	pos.y += 8000;
	
	POINT3D curPos;
	POINT3D rand;
	//모이는 파티클..
    for(int index = 0; index < 20; index++)
	{
		//파티클이 나올 영역.. 
        rand.x = GetRandomPos(-18, 18);
		rand.y = GetRandomPos(-18, 18);
		rand.z = GetRandomPos(-18, 18);
		
		double length = sqrt((float)rand.x*(float)rand.x+(float)rand.y*(float)rand.y+(float)rand.z*(float)rand.z);
		
		if(length == 0)
			length = 1;
	 	int range = GetRandomPos(19000, 20000);
 		curPos.x = int(((float)rand.x/length)*(float)range+(float)pos.x);
		curPos.y = int(((float)rand.y/length)*(float)range+(float)pos.y);
		curPos.z = int(((float)rand.z/length)*(float)range+(float)pos.z);
		
		Assa = SetAssaEffect( 80, "flare.tga", 0, &curPos, 0);
		cAssaEffect[Assa]->FACE_TYPE = ASSAFACE_SCALE;
		
		cAssaEffect[Assa]->MOVECODE = ASSA_MOVE;
		

 		cAssaEffect[Assa]->Face.r = 255;
		cAssaEffect[Assa]->Face.g = 150;
		cAssaEffect[Assa]->Face.b = 50;
		cAssaEffect[Assa]->Face.Transparency = 255;
 		cAssaEffect[Assa]->Time = -(index/2);
  	 	cAssaEffect[Assa]->MoveSpeed.x = -int(float(rand.x)/length*400.f);
		cAssaEffect[Assa]->MoveSpeed.y = -int(float(rand.y)/length*400.f);
		cAssaEffect[Assa]->MoveSpeed.z = -int(float(rand.z)/length*400.f);
		
		//파티클의 넓이..
 		cAssaEffect[Assa]->Size.w = 400;

		//파티클의 길이..
		POINT3D inScale;
  		inScale.x = int((rand.x)/(float)length*8000.f);
		inScale.y = int((rand.y)/(float)length*8000.f);
		inScale.z = int((rand.z)/(float)length*8000.f);
		
		cAssaEffect[Assa]->ScalePosi.x = -inScale.x;
		cAssaEffect[Assa]->ScalePosi.y = -inScale.y;
		cAssaEffect[Assa]->ScalePosi.z = -inScale.z;
	}
	
	//나가는 파티클..
    for(int index = 0; index < 40; index++)
	{
		//파티클이 나올 영역..
   		rand.x = GetRandomPos(-100, 100);
		rand.y = GetRandomPos(-100, 100);
		rand.z = GetRandomPos(-100, 100);
		
		double length = sqrt((float)rand.x*(float)rand.x+(float)rand.y*(float)rand.y+(float)rand.z*(float)rand.z);
		
		if(length == 0)
			length = 1;
		
 		int range = GetRandomPos(5000,7000);
     	curPos.x = int(float(rand.x)/length*(float)range+(float)pos.x);
		curPos.y = int(float(rand.y)/length*(float)range+(float)pos.y);
		curPos.z = int(float(rand.z)/length*(float)range+(float)pos.z);
		
		Assa = SetAssaEffect(200,"flare.tga",0,&curPos,0);                
		
 		cAssaEffect[Assa]->FACE_TYPE = ASSAFACE_SCALE;
   		cAssaEffect[Assa]->Time = -(15+index/2);
		
		cAssaEffect[Assa]->MOVECODE = ASSA_MOVE;
  	 	cAssaEffect[Assa]->MoveSpeed.x = int(float(rand.x)/length*(150.f));
		cAssaEffect[Assa]->MoveSpeed.y = int(float(rand.y)/length*(150.f));
		cAssaEffect[Assa]->MoveSpeed.z = int(float(rand.z)/length*(150.f));
		
		//파티클의 넓이..
 		cAssaEffect[Assa]->Size.w = 500;
		cAssaEffect[Assa]->Face.r = 255;
		cAssaEffect[Assa]->Face.g = 150;
		cAssaEffect[Assa]->Face.b = 50;
		cAssaEffect[Assa]->Face.Transparency = 255;

		//파티클의 길이..
		POINT3D destPos;
   		destPos.x = int(float(rand.x)/length*12000.f);
		destPos.y = int(float(rand.y)/length*12000.f);
		destPos.z = int(float(rand.z)/length*12000.f);
		
		cAssaEffect[Assa]->ScalePosi.x = -destPos.x;
		cAssaEffect[Assa]->ScalePosi.y = -destPos.y;
		cAssaEffect[Assa]->ScalePosi.z = -destPos.z;
	}

	/*
	POINT3D charPos;
	charPos.x = pChar->pX;
	charPos.y = pChar->pY;
	charPos.z = pChar->pZ;

	g_NewParticleMgr.Start("Skill3FighterConcentration1", charPos);
	*/
	int i = GetAssaEffect();
	AssaSkill3Concentration *concentration = new AssaSkill3Concentration;
	concentration->Start(pChar, liveCount);
	cAssaEffect[i] = concentration;
}



void AssaParticle_AvangingCrash(smCHAR* pChar)
{
 	POINT3D pos;
	if(pChar->GetAttackPoint(&pos.x, &pos.y, &pos.z) == TRUE)
	{
		
   		SetDynLight( pos.x, pChar->pY, pos.z, 255, 100, 50,0, 190, 3);
	 	
		g_NewParticleMgr.Start("Skill3Hit1", pos);
	}
}

void AssaParticle_BoneSmash1(smCHAR* pChar)
{
	if(pChar == NULL)
		return;
 	int motionEvent = pChar->MotionEvent;
	
  	int i = GetAssaEffect();
 	AssaSkillWeaponMotionBlur *motionBlur = new AssaSkillWeaponMotionBlur;
	if(motionEvent == 1)
	{
		motionBlur->Start(pChar, 12);
		//SetDynLight( pos.x, pChar->pY, pos.z, 255, 100, 50,0, 160, 3);
	}
	else
	{
		
		motionBlur->Start(pChar, 15);
	}

  	cAssaEffect[i] = motionBlur;
	
	POINT3D pos;
     if ( pChar->GetAttackPoint( &pos.x,&pos.y,&pos.z)==TRUE )
	{
		pos.y += 3000;
		
		g_NewParticleMgr.Start("Skill3Hit6", pos);

		if(motionEvent == 1)
		{
			SetDynLight( pos.x, pChar->pY, pos.z, 255, 100, 50,0, 140, 3);
		}
		else
		{
			pos.y = pChar->pY+2000;
			SetDynLight( pos.x, pChar->pY, pos.z, 255, 100, 50,0, 200, 3);
			g_NewParticleMgr.Start("Skill3FighterBoneSmash1", pos);
		}
	}

	
}


void AssaParticle_SwiftAxe(smCHAR *pChar, int liveCount)
{
	StopAssaCodeEffect(pChar, SKILL_SWIFT_AXE);
	POINT3D pos;
 	memcpy(&pos, &pChar->Posi, sizeof(POINT3D));
   	pos.y += 8000;
	
	POINT3D curPos;
	POINT3D rand;
	//모이는 파티클..
    for(int index = 0; index < 20; index++)
	{
		//파티클이 나올 영역.. 
        rand.x = GetRandomPos(-18, 18);
		rand.y = GetRandomPos(-18, 18);
		rand.z = GetRandomPos(-18, 18);
		
		double length = sqrt((float)rand.x*(float)rand.x+(float)rand.y*(float)rand.y+(float)rand.z*(float)rand.z);
		
		if(length == 0)
			length = 1;
	 	int range = GetRandomPos(19000, 20000);
 		curPos.x = int(((float)rand.x/length)*(float)range+(float)pos.x);
		curPos.y = int(((float)rand.y/length)*(float)range+(float)pos.y);
		curPos.z = int(((float)rand.z/length)*(float)range+(float)pos.z);
		
		Assa = SetAssaEffect( 80, "flare.tga", 0, &curPos, 0);
		cAssaEffect[Assa]->FACE_TYPE = ASSAFACE_SCALE;
		
		cAssaEffect[Assa]->MOVECODE = ASSA_MOVE;
		

 		cAssaEffect[Assa]->Face.r = 50;
		cAssaEffect[Assa]->Face.g = 80;
		cAssaEffect[Assa]->Face.b = 255;
		cAssaEffect[Assa]->Face.Transparency = 255;
 		cAssaEffect[Assa]->Time = -(index/2);
  	 	cAssaEffect[Assa]->MoveSpeed.x = -int(float(rand.x)/length*400.f);
		cAssaEffect[Assa]->MoveSpeed.y = -int(float(rand.y)/length*400.f);
		cAssaEffect[Assa]->MoveSpeed.z = -int(float(rand.z)/length*400.f);
		
		//파티클의 넓이..
 		cAssaEffect[Assa]->Size.w = 400;

		//파티클의 길이..
		POINT3D inScale;
  		inScale.x = int((rand.x)/(float)length*8000.f);
		inScale.y = int((rand.y)/(float)length*8000.f);
		inScale.z = int((rand.z)/(float)length*8000.f);
		
		cAssaEffect[Assa]->ScalePosi.x = -inScale.x;
		cAssaEffect[Assa]->ScalePosi.y = -inScale.y;
		cAssaEffect[Assa]->ScalePosi.z = -inScale.z;
	}
	
	//나가는 파티클..
    for(int index = 0; index < 40; index++)
	{
		//파티클이 나올 영역..
   		rand.x = GetRandomPos(-100, 100);
		rand.y = GetRandomPos(-100, 100);
		rand.z = GetRandomPos(-100, 100);
		
		double length = sqrt((float)rand.x*(float)rand.x+(float)rand.y*(float)rand.y+(float)rand.z*(float)rand.z);
		
		if(length == 0)
			length = 1;
		
 		int range = GetRandomPos(5000,7000);
     	curPos.x = int(float(rand.x)/length*(float)range+(float)pos.x);
		curPos.y = int(float(rand.y)/length*(float)range+(float)pos.y);
		curPos.z = int(float(rand.z)/length*(float)range+(float)pos.z);
		
		Assa = SetAssaEffect(200,"flare.tga",0,&curPos,0);                
		
 		cAssaEffect[Assa]->FACE_TYPE = ASSAFACE_SCALE;
   		cAssaEffect[Assa]->Time = -(15+index/2);
		
		cAssaEffect[Assa]->MOVECODE = ASSA_MOVE;
  	 	cAssaEffect[Assa]->MoveSpeed.x = int(float(rand.x)/length*(150.f));
		cAssaEffect[Assa]->MoveSpeed.y = int(float(rand.y)/length*(150.f));
		cAssaEffect[Assa]->MoveSpeed.z = int(float(rand.z)/length*(150.f));
		
		//파티클의 넓이..
 		cAssaEffect[Assa]->Size.w = 500;
		cAssaEffect[Assa]->Face.r = 50;
		cAssaEffect[Assa]->Face.g = 80;
		cAssaEffect[Assa]->Face.b = 255;
		cAssaEffect[Assa]->Face.Transparency = 255;

		//파티클의 길이..
		POINT3D destPos;
   		destPos.x = int(float(rand.x)/length*12000.f);
		destPos.y = int(float(rand.y)/length*12000.f);
		destPos.z = int(float(rand.z)/length*12000.f);
		
		cAssaEffect[Assa]->ScalePosi.x = -destPos.x;
		cAssaEffect[Assa]->ScalePosi.y = -destPos.y;
		cAssaEffect[Assa]->ScalePosi.z = -destPos.z;
	}

	/*
	POINT3D charPos;
 	charPos.x = pChar->pX;
	charPos.y = pChar->pY;
	charPos.z = pChar->pZ;

	g_NewParticleMgr.Start("Skill3FighterSwiftAxe1", charPos);
	*/

	int i = GetAssaEffect();
	AssaSkill3SwiftAxe *swiftAxe = new AssaSkill3SwiftAxe;
	swiftAxe->Start(pChar, liveCount);
	cAssaEffect[i] = swiftAxe;
}



void AssaParticle_BoneSmash2(smCHAR* pChar)
{
	int i = GetAssaEffect();
	AssaSkillWeaponMotionBlur *motionBlur = new AssaSkillWeaponMotionBlur;
	motionBlur->Start(pChar, 20);
	cAssaEffect[i] = motionBlur;
}

void AssaParticle_ChainLance(POINT3D *pPosi)
{
	
 	pPosi->y += 3000;
	int partID = g_NewParticleMgr.Start("Skill3Hit3", *pPosi);

	POINT3D angle;
	angle.x = rand()%ANGLE_45;
	angle.y = rand()%ANGLE_360;
	angle.z = rand()%ANGLE_45;
	g_NewParticleMgr.SetAngle(partID, angle); 
   	SetDynLight( pPosi->x, pPosi->y, pPosi->z, 100,50,0,0, 250 , 5);
}

void SkillShadowMasterHit(POINT3D *pos)
{
	AssaParticle_ChainLance(pos);
}

void AssaParticle_VeonmSpear(smCHAR *pDest)
{
	POINT3D charPos;
      charPos.x = pDest->pX;
	charPos.y = pDest->pY;
	charPos.z = pDest->pZ;
	
    float delayTime = float(rand()%3);
 	delayTime = delayTime / 3.f;
  	g_NewParticleMgr.Start("Skill3PikeManVenomSpear", charPos, delayTime);
	
	Assa = SetAssaEffect(0,"pk3_Venom24-2.ASE",0,&charPos,0, 0);
 	cAssaEffect[Assa]->AniMaxCount = 24;
	cAssaEffect[Assa]->AniDelayTime = 3;
	cAssaEffect[Assa]->Time = -int(delayTime*70);
}


void AssaParticle_VeonmSpearPike(smCHAR *pChar)
{
 	GetMoveLocation( 0, 0, 25*fONE, 0, pChar->Angle.y, 0);
 	POINT3D charPos;
   	charPos.x = pChar->pX + GeoResult_X;
	charPos.y = pChar->pY+1000;
	charPos.z = pChar->pZ + GeoResult_Z;
	
  	g_NewParticleMgr.Start("Skill3PikeManVenomSpear1", charPos, 1.f);
}

void AssaParticle_Vanish(smCHAR *pChar)
{
	POINT3D pos;
  	pos.x = pChar->pX;
	pos.y = pChar->pY+1000;
	pos.z = pChar->pZ;
	
	int i = GetAssaEffect();
	AssaSkill3Vanish *vanish = new AssaSkill3Vanish;
	vanish->Start(pChar);
	cAssaEffect[i] = vanish;
	
	
	
	POINT3D rand;
	POINT3D curPos;
 	pos.y += 5000;

     for(int index = 0; index < 100; index++)
	{
		//파티클이 나올 영역.. 
		rand.x = GetRandomPos(-20, 20);
		rand.y = GetRandomPos(-20, 20);
		rand.z = GetRandomPos(-20, 20);
		
 		double length = sqrt((float)rand.x*(float)rand.x+(float)rand.y*(float)rand.y+(float)rand.z*(float)rand.z);
		
		if(length == 0)
			length = 1;
	 	int range = GetRandomPos(19000, 20000);
 		curPos.x = int(((float)rand.x/length)*(float)range+(float)pos.x);
		curPos.y = int(((float)rand.y/length)*(float)range+(float)pos.y);
		curPos.z = int(((float)rand.z/length)*(float)range+(float)pos.z);
		
  		Assa = SetAssaEffect( 50, "flare.tga", 0, &curPos, 0);
		cAssaEffect[Assa]->FACE_TYPE = ASSAFACE_BILLBOARD;
		
		cAssaEffect[Assa]->MOVECODE = ASSA_MOVE;
		

        cAssaEffect[Assa]->Face.r = 150;
		cAssaEffect[Assa]->Face.g = 150;
		cAssaEffect[Assa]->Face.b = 255;
		cAssaEffect[Assa]->Face.Transparency = 220;
 		cAssaEffect[Assa]->Time = -(index);
   	 	cAssaEffect[Assa]->MoveSpeed.x = -int(float(rand.x)/length*610.f);
 		cAssaEffect[Assa]->MoveSpeed.y = -int(float(rand.y)/length*610.f);
 		cAssaEffect[Assa]->MoveSpeed.z = -int(float(rand.z)/length*610.f);
		
		//파티클의 넓이..
   		cAssaEffect[Assa]->Size.w = 1200+GetRandomPos(0,500);
	}

}



void AssaParticle_ElementalShotFire(POINT3D* pCur, POINT3D* pDest)
{
	int i = GetAssaEffect(); 
 	AssaSkill3ElementalShot *elementalShot = new AssaSkill3ElementalShot;
 	elementalShot->Start(pCur, pDest, ELEMENTAL_FIRE);
	cAssaEffect[i] = elementalShot;
}

void AssaParticle_ElementalShotLighting(POINT3D* pCur, POINT3D* pDest)
{
	int i = GetAssaEffect(); 
	AssaSkill3LightingJavelin *lightingJavelin = new AssaSkill3LightingJavelin;
	lightingJavelin->StartShort(pCur, pDest);
	cAssaEffect[i] = lightingJavelin;
}


void AssaParticle_GoldenFalcon(smCHAR *pChar, int liveCount)
{
	//Character->HoSkillCode
	//pChar->HoSkillCode = SKILL_GOLDEN_FALCON;
	StartSkill(pChar->pX, pChar->pY, pChar->pZ, 0, 0, 0, pChar, SKILL_GOLDEN_FALCON, liveCount);
}


void AssaParticle_BombShot(POINT3D* pCur, POINT3D* pDest)
{
	int i = GetAssaEffect(); 
 	AssaSkill3BombShot *bombShot = new AssaSkill3BombShot;
 	bombShot->Start(pCur, pDest);
	cAssaEffect[i] = bombShot;
}


void AssaParticle_BombHit1(POINT3D *pPosi)
{
	POINT3D pos;
	memcpy(&pos, pPosi, sizeof(POINT3D));
	D3DVECTOR velocity;
 	D3DVECTOR gravity;
	
    
 	for(int index = 0; index < 3; index++)
  	{
		int i = GetAssaEffect(); 
		velocity.x = float(rand()%2400-1200);
		velocity.y = float(rand()%500+800);
		velocity.z = float(rand()%2400-1200);

  		gravity.x = 0;
 		gravity.y = float((rand()%500-670));
		gravity.z = 0;
		
		AssaSkill3BombShotExplision *explision = new AssaSkill3BombShotExplision;
		explision->Start(&pos, &velocity, &gravity);
		cAssaEffect[i] = explision;
	}

	g_NewParticleMgr.Start("Skill3ArcherBombShotHit1", pos);
 	SetDynLight( pos.x, pos.y, pos.z, 255, 150, 50, 255, 200, 2);

}

void AssaParticle_Perforation(POINT3D* pCur, POINT3D* pDest)
{
	int i = GetAssaEffect(); 
	AssaSkill3Perforation *perforation = new AssaSkill3Perforation;
	perforation->Start(pCur, pDest);
	cAssaEffect[i] = perforation;
}

void AssaParticle_PerforationHit(smCHAR* monster, smCHAR* character, float delay)
{
	POINT3D pos;
 	pos.x = monster->pX;
	pos.y = monster->pY+2000;
	pos.z = monster->pZ;

	int num = -1;
	if(delay == 0.f)
		num = g_NewParticleMgr.Start("TwistJavelin2", pos, 0.f);
	else
		num = g_NewParticleMgr.Start("TwistJavelin2", pos, delay);

	POINT3D angle;
	angle.x = 0;
	angle.y = character->Angle.y;
	angle.z = 0;
    g_NewParticleMgr.SetAngle(num, angle);
}


void AssaParticle_GrandSmash(smCHAR *pChar)//(POINT3D *pPosi)
{
	/*
 	pPosi->y += 3000;
	int partID = g_NewParticleMgr.Start("Skill3Hit2", *pPosi);

	POINT3D angle;
	angle.x = rand()%ANGLE_90;
	angle.y = rand()%ANGLE_360;
	angle.z = rand()%ANGLE_90;
	g_NewParticleMgr.SetAngle(partID, angle); 
	*/

 	if(pChar == NULL)
		return;
 	int motionEvent = pChar->MotionEvent;
	
  	int i = GetAssaEffect();
 	AssaSkillWeaponMotionBlur *motionBlur = new AssaSkillWeaponMotionBlur;
	if(motionEvent == 1)
		motionBlur->Start(pChar, 12);
	else
		motionBlur->Start(pChar, 15);
	

     cAssaEffect[i] = motionBlur;
	
	POINT3D pos;
    if ( pChar->GetAttackPoint( &pos.x,&pos.y,&pos.z)==TRUE )
	{
 		pos.y += 3000;
		g_NewParticleMgr.Start("Skill3Hit2", pos);
		StartSkill(pos.x, pos.y, pos.z, 0, 0, 0, SKILL_GRAND_SMASH);
		SetDynLight( pos.x,pos.y,pos.z, 0,100,50,0, 250 , 5);
	}

}

void AssaParticle_MetalArmor(smCHAR *pChar, int liveCount)
{
	StopAssaCodeEffect(pChar, SKILL_METAL_ARMOR);

	POINT3D charPos;
	charPos.x = pChar->pX;
	charPos.y = pChar->pY;
	charPos.z = pChar->pZ;
	StartSkill( charPos.x,charPos.y,charPos.z, 0, 0, 0, SKILL_METALARMOR ); 
	
	int i = GetAssaEffect();
	AssaSkill3MetalArmor *metalArmor = new AssaSkill3MetalArmor;
	metalArmor->Start(pChar, liveCount);
	cAssaEffect[i] = metalArmor;
}

void AssaParticle_SparkShieldDefence(POINT3D *pPosi, POINT3D *pDesti)
{
	g_NewParticleMgr.Start("LightStart1", *pPosi);	
	g_NewParticleMgr.Start("LightHit1", *pDesti);
	
	SetDynLight( pDesti->x, pDesti->y, pDesti->z, 255, 255, 255, 255, 150, 2);

	Assa = SetAssaEffect(100,"spark01_01.bmp",0,pPosi,0);
	AssaGetAniFileMat(cAssaEffect[Assa]);
	cAssaEffect[Assa]->AniDelayTime = 3;
	cAssaEffect[Assa]->Face.r = 150;
	cAssaEffect[Assa]->Face.g = 255;
	cAssaEffect[Assa]->Face.b = 150;
	cAssaEffect[Assa]->Face.Transparency = 255;
	cAssaEffect[Assa]->Size.w = 5000;
	cAssaEffect[Assa]->FACE_TYPE = ASSAFACE_SCALE; 
	
	cAssaEffect[Assa]->ScalePosi.x = pDesti->x - pPosi->x;
	cAssaEffect[Assa]->ScalePosi.y = pDesti->y - pPosi->y;
	cAssaEffect[Assa]->ScalePosi.z = pDesti->z - pPosi->z;

	Assa = SetAssaEffect(100,"spark01_01.bmp",0,pPosi,0);
	AssaGetAniFileMat(cAssaEffect[Assa]);
	cAssaEffect[Assa]->AniDelayTime = 3;
	cAssaEffect[Assa]->Face.r = 150;
	cAssaEffect[Assa]->Face.g = 255;
	cAssaEffect[Assa]->Face.b = 150;
	cAssaEffect[Assa]->Face.Transparency = 255;
	cAssaEffect[Assa]->Size.w = 5000;
	cAssaEffect[Assa]->FACE_TYPE = ASSAFACE_SCALE; 
	
	cAssaEffect[Assa]->ScalePosi.x = pDesti->x - pPosi->x;
	cAssaEffect[Assa]->ScalePosi.y = pDesti->y - pPosi->y;
	cAssaEffect[Assa]->ScalePosi.z = pDesti->z - pPosi->z;
}

void AssaParticle_SparkShieldStart(smCHAR *pChar, int liveCount)
{
	
 	StopAssaCodeEffect(pChar, SKILL_SPARK_SHIELD);
	
  	GetMoveLocation( 0, 0, 10*fONE, 0, pChar->Angle.y, 0);
	POINT3D curPos;
  	curPos.x = pChar->pX + GeoResult_X;
	curPos.y = pChar->pY+8000;
	curPos.z = pChar->pZ + GeoResult_Z;
	

	g_NewParticleMgr.Start("Skill3MechanicianSparkShield", curPos, 0.2f);
	POINT3D destPos;
	destPos.x = curPos.x+30000;
	destPos.y = curPos.y+80000;
	destPos.z = curPos.z+30000;
	
   	Assa = SetAssaEffect(90,"spark01_01.bmp",0,&curPos,0);
	AssaGetAniFileMat(cAssaEffect[Assa]);
	cAssaEffect[Assa]->AniDelayTime = 3;
	cAssaEffect[Assa]->Face.r = 150;
	cAssaEffect[Assa]->Face.g = 255;
	cAssaEffect[Assa]->Face.b = 150;
	cAssaEffect[Assa]->Face.Transparency = 200;
	cAssaEffect[Assa]->Size.w = 5000;
	cAssaEffect[Assa]->FACE_TYPE = ASSAFACE_SCALE; 
	
	cAssaEffect[Assa]->ScalePosi.x = destPos.x - curPos.x;
	cAssaEffect[Assa]->ScalePosi.y = destPos.y - curPos.y;
	cAssaEffect[Assa]->ScalePosi.z = destPos.z - curPos.z;

	
  	destPos.x = curPos.x+22000;
	destPos.y = curPos.y+75000;
	destPos.z = curPos.z+22000;
	
  	Assa = SetAssaEffect(100,"spark01_01.bmp",0,&curPos,0);
	AssaGetAniFileMat(cAssaEffect[Assa]);
	cAssaEffect[Assa]->AniDelayTime = 3;
	cAssaEffect[Assa]->Face.r = 150;
	cAssaEffect[Assa]->Face.g = 255;
	cAssaEffect[Assa]->Face.b = 150;
	cAssaEffect[Assa]->Face.Transparency = 200;
	cAssaEffect[Assa]->Size.w = 5000;
	cAssaEffect[Assa]->FACE_TYPE = ASSAFACE_SCALE; 
	
	cAssaEffect[Assa]->ScalePosi.x = destPos.x - curPos.x;
	cAssaEffect[Assa]->ScalePosi.y = destPos.y - curPos.y;
	cAssaEffect[Assa]->ScalePosi.z = destPos.z - curPos.z;

	
	destPos.x = curPos.x-30000;
	destPos.y = curPos.y+80000;
	destPos.z = curPos.z-30000;

 	Assa = SetAssaEffect(80,"spark01_01.bmp",0,&curPos,0);
	AssaGetAniFileMat(cAssaEffect[Assa]);
	cAssaEffect[Assa]->AniDelayTime = 3;
	cAssaEffect[Assa]->Face.r = 150;
	cAssaEffect[Assa]->Face.g = 255;
	cAssaEffect[Assa]->Face.b = 150;
	cAssaEffect[Assa]->Face.Transparency = 200;
	cAssaEffect[Assa]->Size.w = 5000;
	cAssaEffect[Assa]->FACE_TYPE = ASSAFACE_SCALE; 
	
	cAssaEffect[Assa]->ScalePosi.x = destPos.x - curPos.x;
	cAssaEffect[Assa]->ScalePosi.y = destPos.y - curPos.y;
	cAssaEffect[Assa]->ScalePosi.z = destPos.z - curPos.z;

	destPos.x = curPos.x-22000;
	destPos.y = curPos.y+75000;
	destPos.z = curPos.z-22000;

   	Assa = SetAssaEffect(85,"spark01_01.bmp",0,&curPos,0);
	AssaGetAniFileMat(cAssaEffect[Assa]);
	cAssaEffect[Assa]->AniDelayTime = 3;
	cAssaEffect[Assa]->Face.r = 150;
	cAssaEffect[Assa]->Face.g = 255;
	cAssaEffect[Assa]->Face.b = 150;
	cAssaEffect[Assa]->Face.Transparency = 200;
	cAssaEffect[Assa]->Size.w = 5000;
	cAssaEffect[Assa]->FACE_TYPE = ASSAFACE_SCALE; 
	
	cAssaEffect[Assa]->ScalePosi.x = destPos.x - curPos.x;
	cAssaEffect[Assa]->ScalePosi.y = destPos.y - curPos.y;
	cAssaEffect[Assa]->ScalePosi.z = destPos.z - curPos.z;

	int i = GetAssaEffect();
	AssaSkill3SparkShield *sparkShield = new AssaSkill3SparkShield;
	sparkShield->Start(pChar, liveCount);
 	cAssaEffect[i] = sparkShield;

	SetDynLight(pChar->pX, pChar->pY, pChar->pZ, 150,255,150,255,  150, 3);
}

void AssaParticle_EnergyShield(smCHAR *pChar,int liveCount)
{
	StopAssaCodeEffect(pChar, SKILL_ENERGY_SHIELD);

	int i = GetAssaEffect();
	AssaSkill3EnergyShield *energyShield = new AssaSkill3EnergyShield;
	energyShield->Start(pChar, liveCount);
	cAssaEffect[i] = energyShield;
}

void AssaParticle_EnergyShieldDefence(smCHAR *pChar)
{
	int i = GetAssaEffect();
	AssaSkill3EnergyShieldDefence *shieldDefence = new AssaSkill3EnergyShieldDefence;
	shieldDefence->Start(pChar);
	cAssaEffect[i] = shieldDefence;
}

void AssaParticle_SpiritElemental(smCHAR *pChar, int liveCount)
{
	StopAssaCodeEffect(pChar, SKILL_SPIRIT_ELEMENTAL);

	int i = GetAssaEffect();
	AssaSkill3SpiritElemental *spiritElemental = new AssaSkill3SpiritElemental;
	spiritElemental->Start(pChar, liveCount);
	cAssaEffect[i] = spiritElemental;
}


void AssaParticle_DancingSwordFire(smCHAR *pChar, int liveCount, int level)
{
	StopAssaCodeEffect(pChar, SKILL_DANCING_SWORD);

	int i = GetAssaEffect();
	AssaSkill3DancingSword * dancingSword = new AssaSkill3DancingSword;
	dancingSword->Start(pChar, 0, liveCount, level);
	cAssaEffect[i] = dancingSword;
}
void AssaParticle_DancingSwordIce(smCHAR *pChar,int liveCount, int level)
{
 	StopAssaCodeEffect(pChar, SKILL_DANCING_SWORD);

	int i = GetAssaEffect();
	AssaSkill3DancingSword * dancingSword = new AssaSkill3DancingSword;
	dancingSword->Start(pChar, 1, liveCount, level);
	cAssaEffect[i] = dancingSword;
}


void AssaParticle_DiastrophismPart(smCHAR *pChar)
{
	POINT3D pos;
	pos.x = pChar->pX;
	pos.y = pChar->pY;
	pos.z = pChar->pZ;

     Assa = SetAssaEffect(100,"Mg3_Dia2.ASE",pChar,&pos,pChar->PatHeight+2000);
  	cAssaEffect[Assa]->AniMaxCount = 20;
	cAssaEffect[Assa]->AniDelayTime = 4;
	cAssaEffect[Assa]->MOVECODE = ASSA_CHAR_POSI;
}

void AssaParticle_Diastrophism(smCHAR *pChar)
{
	POINT3D charPos;
    charPos.x = pChar->pX;
	charPos.y = pChar->pY;
	charPos.z = pChar->pZ;
	
	SetDynLight( charPos.x, charPos.y, charPos.z,-150, -150, -255, 255, 300, 2);
   	Assa = SetAssaEffect(0,"Mg3_Diastrophism24.ASE",0,&charPos,0, 0);
	cAssaEffect[Assa]->AniMaxCount = 24;
	cAssaEffect[Assa]->AniDelayTime = 3;
  	g_NewParticleMgr.Start("Skill3MagicianDiastrophism1", charPos);

      if(lpCurPlayer == pChar)
	{
		
   		EffectWaveCamera(160, 1);

	}

	/*
	else
	{
		D3DVECTOR vector;
  		vector.x = float(pChar->pX - lpCurPlayer->pX)/fONE;
		vector.y = float(pChar->pY - lpCurPlayer->pY)/fONE;
		vector.z = float(pChar->pZ - lpCurPlayer->pZ)/fONE;
		
		float length;
 		length = (float)sqrt(vector.x*vector.x+vector.y*vector.y+vector.z*vector.z);
		if(length == 0)
			length = 1;

		float factor;
 	 	factor = float(((50000/fONE)-length)/(50000/fONE))*40.f+10.f;
		if(factor == 0)
			factor = 1;
		
  		float power = 0;
		if(factor < 5)
			power = 10;
		else
			power  = factor;		//100000만 거리에 있는 캐릭만 영향을 받게 한다.
		EffectWaveCamera((int)power, 3);
	}
	*/
}

void AssaParticle_HolyIncantationShot(smCHAR* pDestChar)
{
	int i = GetAssaEffect(); 
	AssaSkill3HolyIncantationShot *incantationShot = new AssaSkill3HolyIncantationShot;
	incantationShot->Start(pDestChar);
	cAssaEffect[i] = incantationShot;
}

void AssaParticle_HolyIncantationPart(smCHAR* pDestChar, int liveCount)
 {
	StopAssaCodeEffect(pDestChar, SKILL_HOLY_INCANTATION);
	
	int i = GetAssaEffect(); 
	AssaSkill3HolyIncantationPart *incantationPart = new AssaSkill3HolyIncantationPart;
	incantationPart->Start(pDestChar, liveCount);
	cAssaEffect[i] = incantationPart;
}

void AssaParticle_HolyIncantationHand(smCHAR* pDestChar)
{
 	int i = GetAssaEffect(); 
	AssaSkill3HolyIncantationPartHand *incantationPartHand = new AssaSkill3HolyIncantationPartHand;
	incantationPartHand->Start(pDestChar, 3);
	cAssaEffect[i] = incantationPartHand;
}

void AssaParticle_GrandCrossHorz(POINT3D* pPosi, int angleY)
{
   	SetDynLight( pPosi->x, pPosi->y, pPosi->z, 50, 100, 255, 0, 200, 4);
	pPosi->y += 3000;
	//Assa = SetAssaEffect(0,"Nt3_GrandCross20.ASE",0,pPosi,0, 0);
 	//cAssaEffect[Assa]->AniMaxCount = 20;
	//cAssaEffect[Assa]->AniDelayTime = 3;
 	//cAssaEffect[Assa]->Angle.y = (-angleY)&ANGCLIP;
	
 	int partID = g_NewParticleMgr.Start("Skill3Hit4", *pPosi);
	
	POINT3D angle;
 	angle.x = 0;
    	angle.y = angleY;
	angle.z = 0;
    g_NewParticleMgr.SetAngle(partID, angle); 
	
}

void AssaParticle_GrandCrossVert(POINT3D* pPosi, int angleY)
{
	SetDynLight( pPosi->x, pPosi->y, pPosi->z, 50, 100, 255, 0, 200, 4);
	pPosi->y += 4000;
	int partID = g_NewParticleMgr.Start("Skill3Hit5", *pPosi);
	
	POINT3D angle;
	angle.x = 0;
	angle.y = angleY;
	angle.z = 0;

	g_NewParticleMgr.SetAngle(partID, angle); 
	
	pPosi->y += 1000;
	Assa = SetAssaEffect(0,"Nt3_GrandCross20.ASE",0,pPosi,0, 0);
 	cAssaEffect[Assa]->AniMaxCount = 20;
	cAssaEffect[Assa]->AniDelayTime = 3;
 	cAssaEffect[Assa]->Angle.y = (-angleY)&ANGCLIP;
}

void AssaParticle_FlameBrandish(smCHAR *pChar)
{
	
	for(int i = 0; i <2 ; i++){    
		Assa = SetAssaEffect(0,"Brabdish.ASE",pChar,0,0);                              
		cAssaEffect[Assa]->AniMaxCount = 20; 
		cAssaEffect[Assa]->AniDelayTime = 2;
		cAssaEffect[Assa]->StartPosi.x = 256*8*6;
		cAssaEffect[Assa]->Angle.y =  ((pChar->Angle.y&ANGCLIP)+ANGLE_270)&ANGCLIP;
		AssaSetStartPosi(cAssaEffect[Assa]);
		cAssaEffect[Assa]->Angle.y =  -((pChar->Angle.y&ANGCLIP))&ANGCLIP;
		//AssaParticle_SpecialHit2(&cAssaEffect[Assa]->Posi,0);
	}

	POINT3D partPos;
  	partPos.x = cAssaEffect[Assa]->Posi.x;
	partPos.y = cAssaEffect[Assa]->Posi.y+2000;
	partPos.z = cAssaEffect[Assa]->Posi.z;
  	g_NewParticleMgr.Start("Skill3KnightFlameBrandish1",partPos, 0.1f);

	SetDynLight( cAssaEffect[Assa]->Posi.x , cAssaEffect[Assa]->Posi.y , cAssaEffect[Assa]->Posi.z ,255, 120, 0, 255, 200 ,1);
	
    int t = GetAssaEffect(); 
	AssaSkill3FlameBrandish *flameBrandish = new AssaSkill3FlameBrandish;
	flameBrandish->Start(pChar);
	cAssaEffect[t] = flameBrandish;
}


void AssaParticle_FlameBrandishHit1(smCHAR *pChar)
{
	POINT3D pos;
	pos.x = pChar->pX;
	pos.y = pChar->pY;
	pos.z = pChar->pZ;

	AssaParticle_FireHit2(&pos);
}

void AssaParticle_DivineInhalationStart(smCHAR* pChar, int liveCount)
{
 	StopAssaCodeEffect(pChar, SKILL_DIVINE_INHALATION);
 	//if(AddAssaCodeEffectTime(pChar, SKILL_DIVINE_INHALATION, liveCount*70+140) == false)
	//{

		POINT3D pos;
		POINT3D curPos;
		POINT3D rand;
 		if(pChar->AnimPattern)
		{
			smOBJ3D *searchObj = pChar->AnimPattern->GetObjectFromName("Bip01 L Hand"); //오브젝트를 읽는다
			smMATRIX *mWorld;
			AnimObjectTree( searchObj, pChar->frame, 0, (-pChar->Angle.y+ANGLE_180)&ANGCLIP, 0);
			mWorld = &searchObj->mWorld;
			
			
			pos.x = pChar->pX+mWorld->_41;
			pos.y = pChar->pY+mWorld->_43;
			pos.z = pChar->pZ+mWorld->_42;
		}
		else
		{
			pos.x = pChar->pX;
			pos.y = pChar->pY+10000;
			pos.z = pChar->pZ;
		}
   		SetDynLight( pos.x, pos.y, pos.z, 50, 100, 255, 0, 250, 2);
		//나가는 파티클..
   		for(int index = 0; index < 30; index++)
		{
			//파티클이 나올 영역..
   			rand.x = GetRandomPos(-100, 100);
			rand.y = GetRandomPos(-100, 100);
			rand.z = GetRandomPos(-100, 100);
			
			double length = sqrt((float)rand.x*(float)rand.x+(float)rand.y*(float)rand.y+(float)rand.z*(float)rand.z);
			
			if(length == 0)
				length = 1;
			
  			int range = GetRandomPos(5000,7000);
     		curPos.x = int(float(rand.x)/length*(float)range+(float)pos.x);
			curPos.y = int(float(rand.y)/length*(float)range+(float)pos.y);
 			curPos.z = int(float(rand.z)/length*(float)range+(float)pos.z);
			
   			Assa = SetAssaEffect(130+index,"flare.bmp",0,&curPos,0);                
			
  			cAssaEffect[Assa]->FACE_TYPE = ASSAFACE_SCALE;
   			cAssaEffect[Assa]->Time = -(index/2);
			
    		cAssaEffect[Assa]->MOVECODE = ASSA_MOVE;
  			cAssaEffect[Assa]->MoveSpeed.x = int(float(rand.x)/length*(150.f));
			cAssaEffect[Assa]->MoveSpeed.y = int(float(rand.y)/length*(150.f));
			cAssaEffect[Assa]->MoveSpeed.z = int(float(rand.z)/length*(150.f));
			
			//파티클의 넓이..
    		cAssaEffect[Assa]->Size.w = 500;
			cAssaEffect[Assa]->Face.r = 100;
			cAssaEffect[Assa]->Face.g = 255;
			cAssaEffect[Assa]->Face.b = 255;
			cAssaEffect[Assa]->Face.Transparency = 255;

			//파티클의 길이..
			POINT3D destPos;
   			destPos.x = int(float(rand.x)/length*12000.f);
			destPos.y = int(float(rand.y)/length*12000.f);
			destPos.z = int(float(rand.z)/length*12000.f);
			
			cAssaEffect[Assa]->ScalePosi.x = -destPos.x;
			cAssaEffect[Assa]->ScalePosi.y = -destPos.y;
			cAssaEffect[Assa]->ScalePosi.z = -destPos.z;
		}
		
   		g_NewParticleMgr.Start("Skill3KnightDivineInhalation1", pos);
		AssaParticle_DivineInhalationPart(pChar, liveCount);
	//}
}

void SkillSaintGodlyShield(smCHAR *pChar, float time)
{
	int liveCount = int(time * 70.f);
	//StopAssaCodeEffect(pChar, SKILL_DIVINE_INHALATION);
	POINT3D pos;
	POINT3D curPos;
	POINT3D rand;
 	if(pChar->AnimPattern)
	{
		smOBJ3D *searchObj = pChar->AnimPattern->GetObjectFromName("Bip01 L Hand"); //오브젝트를 읽는다
		smMATRIX *mWorld;
		AnimObjectTree( searchObj, pChar->frame, 0, (-pChar->Angle.y+ANGLE_180)&ANGCLIP, 0);
		mWorld = &searchObj->mWorld;
		
		
		pos.x = pChar->pX+mWorld->_41;
		pos.y = pChar->pY+mWorld->_43;
		pos.z = pChar->pZ+mWorld->_42;
	}
	else
	{
		pos.x = pChar->pX;
		pos.y = pChar->pY+10000;
		pos.z = pChar->pZ;
	}
   	SetDynLight( pos.x, pos.y, pos.z, 50, 100, 255, 0, 250, 2);
	//나가는 파티클..
   	for(int index = 0; index < 30; index++)
	{
		//파티클이 나올 영역..
   		rand.x = GetRandomPos(-100, 100);
		rand.y = GetRandomPos(-100, 100);
		rand.z = GetRandomPos(-100, 100);
		
		double length = sqrt((float)rand.x*(float)rand.x+(float)rand.y*(float)rand.y+(float)rand.z*(float)rand.z);
		
		if(length == 0)
			length = 1;
		
  		int range = GetRandomPos(5000,7000);
     	curPos.x = int(float(rand.x)/length*(float)range+(float)pos.x);
		curPos.y = int(float(rand.y)/length*(float)range+(float)pos.y);
 		curPos.z = int(float(rand.z)/length*(float)range+(float)pos.z);
		
   		Assa = SetAssaEffect(130+index,"flare.bmp",0,&curPos,0);                
		
  		cAssaEffect[Assa]->FACE_TYPE = ASSAFACE_SCALE;
   		cAssaEffect[Assa]->Time = -(index/2);
		
    	cAssaEffect[Assa]->MOVECODE = ASSA_MOVE;
  		cAssaEffect[Assa]->MoveSpeed.x = int(float(rand.x)/length*(150.f));
		cAssaEffect[Assa]->MoveSpeed.y = int(float(rand.y)/length*(150.f));
		cAssaEffect[Assa]->MoveSpeed.z = int(float(rand.z)/length*(150.f));
		
		//파티클의 넓이..
    	cAssaEffect[Assa]->Size.w = 500;
		cAssaEffect[Assa]->Face.r = 100;
		cAssaEffect[Assa]->Face.g = 255;
		cAssaEffect[Assa]->Face.b = 255;
		cAssaEffect[Assa]->Face.Transparency = 255;

		//파티클의 길이..
		POINT3D destPos;
   		destPos.x = int(float(rand.x)/length*12000.f);
		destPos.y = int(float(rand.y)/length*12000.f);
		destPos.z = int(float(rand.z)/length*12000.f);
		
		cAssaEffect[Assa]->ScalePosi.x = -destPos.x;
		cAssaEffect[Assa]->ScalePosi.y = -destPos.y;
		cAssaEffect[Assa]->ScalePosi.z = -destPos.z;
	}
	
   	g_NewParticleMgr.Start("Skill3KnightDivineInhalation1", pos);
	AssaParticle_DivineInhalationPart(pChar, liveCount);


}

void AssaParticle_DivineInhalationPart(smCHAR* pChar, int liveCount)
{
	
	int i = GetAssaEffect(); 
	AssaSkill3DivineInhalation *divineInhalation = new AssaSkill3DivineInhalation;
	divineInhalation->Start(pChar, liveCount);
	cAssaEffect[i] = divineInhalation;
}


void AssaParticle_DivineInhalationDefence(POINT3D *pDest)
{
	StartEffect( pDest->x, pDest->y, pDest->z, EFFECT_PHYSICAL_ABSORB_DAMAGE );
}

void AssaParticle_VigorBall(smCHAR* pCurChar, smCHAR* pDestChar)
{
	POINT3D curPos;
   	curPos.x = pCurChar->pX;
	curPos.y = pCurChar->pY+5000;
	curPos.z = pCurChar->pZ;

	D3DVECTOR velocity;
	memset(&velocity, 0, sizeof(velocity));

	int motionEvent = pCurChar->MotionEvent;
	int i  = 0;
	if(motionEvent == 1)
	{
 		i = GetAssaEffect(); 
		AssaSkill3VigorBall *vigorBall = new AssaSkill3VigorBall;
		
		velocity.x = 0.f;
		velocity.y = 0.f;
		velocity.z = 0.f;

		GetMoveLocation( 0, 0, 3*fONE+rand()%(2*fONE)-fONE, 0, pCurChar->Angle.y-ANGLE_45, 0);
		velocity.x += ((float)GeoResult_X*2.f/(float)fONE);
		velocity.z += ((float)GeoResult_Z*2.f/(float)fONE);
		vigorBall->Start(&curPos, pDestChar, &velocity, 0);
		cAssaEffect[i] = vigorBall;
	}
	else
	{
		i = GetAssaEffect(); 
		AssaSkill3VigorBall *vigorBall = new AssaSkill3VigorBall;
	
		velocity.x = 0.f;
		velocity.y = 0.f;
		velocity.z = 0.f;

 		GetMoveLocation( 0, 0, 3*fONE+rand()%(2*fONE)-fONE, 0, pCurChar->Angle.y+ANGLE_45, 0);
		velocity.x += ((float)GeoResult_X*2.f/(float)fONE);
		velocity.z += ((float)GeoResult_Z*2.f/(float)fONE);
		vigorBall->Start(&curPos, pDestChar, &velocity, 0);
		cAssaEffect[i] = vigorBall;
	}

}

void AssaParticle_Extinction(smCHAR* pDestChar)
{
	int i = GetAssaEffect(); 
	AssaSkill3Extinction *extinction = new AssaSkill3Extinction;
	extinction->Start(pDestChar);
	cAssaEffect[i] = extinction;
}

void AssaParticle_VirtualLife(smCHAR* pDestChar,int liveCount)
{
	//StopAssaCodeEffect(pDestChar, SKILL_VIRTUAL_LIFE);
	if(AddAssaCodeEffectTime(pDestChar, SKILL_VIRTUAL_LIFE, liveCount*70+140) == false)
	{
		POINT3D charPos;
 		charPos.x = pDestChar->pX;
		charPos.y = pDestChar->pY;
		charPos.z = pDestChar->pZ;

		Assa = SetAssaEffect(0,"Pt3_VirtualLife20.ASE",0,&charPos,0, 0);
		cAssaEffect[Assa]->Time = -60;
 		cAssaEffect[Assa]->AniMaxCount = 20;
		cAssaEffect[Assa]->AniDelayTime = 6;
		cAssaEffect[Assa]->MOVECODE = ASSA_CHAR_POSI;
		cAssaEffect[Assa]->pChar = pDestChar;

		cAssaEffect[Assa]->Temp[8] = 1;
		cAssaEffect[Assa]->TempPosi.x = 0;
		cAssaEffect[Assa]->TempPosi.y = 1000;
		cAssaEffect[Assa]->TempPosi.z = 0;

		int i = GetAssaEffect(); 
		AssaSkillVirtualLifeMember *virtualLifeMember = new AssaSkillVirtualLifeMember;
		virtualLifeMember->Start(pDestChar, liveCount);
		cAssaEffect[i] = virtualLifeMember;
	}
}

void AssaParticle_ResurrectionShot(smCHAR* pDestChar)
{
	//크리티칼 섹션 선언
	//EnterCriticalSection( &cLoadSection );
	//EnterCriticalSection( &cDrawSection );

	int i = GetAssaEffect(); 
	AssaSkill3ResurrectionShot *ressurectionShot = new AssaSkill3ResurrectionShot;
	ressurectionShot->Start(pDestChar, 65);
	cAssaEffect[i] = ressurectionShot;

	//크리티칼 섹션 해제
//	LeaveCriticalSection( &cDrawSection );
//	LeaveCriticalSection( &cLoadSection );
}

void AssaParticle_ResurrectionPart(smCHAR* pDestChar)
{
	
	int i = GetAssaEffect(); 
	AssaSkill3ResurrectionPart *resurrectionPart = new AssaSkill3ResurrectionPart;
	resurrectionPart->Start(pDestChar, 10);
	cAssaEffect[i] = resurrectionPart;
	

}


void AssaParticle_LightingJavelin(POINT3D* pCur, POINT3D* pDest)
{
	int i = GetAssaEffect(); 
	AssaSkill3LightingJavelin *lightingJavelin = new AssaSkill3LightingJavelin;
	lightingJavelin->Start(pCur, pDest);
	cAssaEffect[i] = lightingJavelin;
}

void AssaParticle_StormJavelin(POINT3D* pCur, POINT3D* pDest)
{
	int i = GetAssaEffect(); 
	AssaSkill3StormJavelin *stormJavelin = new AssaSkill3StormJavelin;
	stormJavelin->Start(pCur, pDest);
	cAssaEffect[i] = stormJavelin;
}

void AssaParticle_StormJavelinHit(smCHAR* monster, smCHAR* character, float delay)
{
	POINT3D pos;
 	pos.x = monster->pX;
	pos.y = monster->pY+2000;
	pos.z = monster->pZ;

	int num = -1;
	if(delay == 0.f)
		num = g_NewParticleMgr.Start("TwistJavelin2", pos, 0.f);
	else
		num = g_NewParticleMgr.Start("TwistJavelin2", pos, delay);

	POINT3D angle;
	angle.x = 0;
	angle.y = character->Angle.y;
	angle.z = 0;
    g_NewParticleMgr.SetAngle(num, angle);

}

void AssaParticle_TriumphOfValhalla(smCHAR *pDestChar, int liveCount)
{
	StopAssaCodeEffect(pDestChar, SKILL_TRIUMPH_OF_VALHALLA);
	int i = GetAssaEffect(); 
	AssaSkillTriumphOfValhallaPart *triumphOfValhallaPart = new AssaSkillTriumphOfValhallaPart;
	triumphOfValhallaPart->Start(pDestChar, liveCount);
	cAssaEffect[i] = triumphOfValhallaPart;
}

void AssaParticle_TriumphOfValhallaMember(smCHAR *pCurChar, smCHAR *pDestChar, int liveCount)
{
	int i = GetAssaEffect(); 
	AssaSkillTriumphOfValhallaShot *triumphOfValhallaShot = new AssaSkillTriumphOfValhallaShot;
	
	D3DVECTOR velocity;
	velocity.x = 0.f;
	velocity.y = 7.0f;
	velocity.z = 0.f;
	triumphOfValhallaShot->Start(pCurChar, pDestChar, &velocity, liveCount);
	
	cAssaEffect[i] = triumphOfValhallaShot;
}

void SkillValhallaHallOfValhallaHandEffect(smCHAR *pChar, float time)
{
	if(!AddAssaCodeEffectTime(pChar, SKILL_HALL_OF_VALHALLA, (int)(time*70.f)))
	{
		//StopAssaCodeEffect(pChar, SKILL_HALL_OF_VALHALLA);
		int i = GetAssaEffect();
		AssaSkillTriumphOfValhallaPart *triumphOfValhallaPart = new AssaSkillTriumphOfValhallaPart;
		triumphOfValhallaPart->StartHallOfValhalla(pChar, int(time));
		cAssaEffect[i] = triumphOfValhallaPart;
	}
}

//count; 화살의 갯수
void AssaParticle_SplitJavelin(POINT3D* pCur, POINT3D* pDest, int count)
{
  	for(int index = 0; index < count; index++)
	{
		POINT3D worldPos;
  		GetMoveLocation( 0, 5*fONE, 0, 0, 0,ANGLE_360/count*index);
		memcpy(&worldPos, pCur, sizeof(POINT3D));
 		worldPos.x += GeoResult_X;
		worldPos.y += GeoResult_Y;
		worldPos.z += GeoResult_Z;
		
		pDest->x += GeoResult_X;
		pDest->y += GeoResult_Y;
		pDest->z += GeoResult_Z;

		int i = GetAssaEffect(); 
  		AssaSkill3SplitJavelin *splitJavelin = new AssaSkill3SplitJavelin;
 		splitJavelin->Start(&worldPos, pDest, index*10);
 		cAssaEffect[i] = splitJavelin;
	}
}

void AssaParticle_MonsterPigonHand(smCHAR* pChar)
{
	if(GetAssaCodeEffect(pChar, PIGON_HAND_FIRE) == FALSE)
	{
  		int i = GetAssaEffect();
		cAssaTraceCharMeshParticle *traceMeshParticle = new cAssaTraceCharMeshParticle;
		traceMeshParticle->Start(pChar, "Bip01 L Finger1", "MonsterPigonFire1", FALSE);
		cAssaEffect[i] = traceMeshParticle;
		cAssaEffect[i]->CODE = PIGON_HAND_FIRE;
		

 		i = GetAssaEffect();
		traceMeshParticle = NULL;
		traceMeshParticle = new cAssaTraceCharMeshParticle;
		traceMeshParticle->Start(pChar, "Bip01 R Finger1", "MonsterPigonFire1", FALSE);
		cAssaEffect[i] = traceMeshParticle;
		cAssaEffect[i]->CODE = PIGON_HAND_FIRE;
		
		i = GetAssaEffect();
		traceMeshParticle = NULL;
		traceMeshParticle = new cAssaTraceCharMeshParticle;
		traceMeshParticle->Start(pChar, "Bip01 Tail", "MonsterPigonFireTail", FALSE);
		cAssaEffect[i] = traceMeshParticle;
		cAssaEffect[i]->CODE = PIGON_HAND_FIRE;
	}
}

void AssaParticle_ClassUpWeapon(smCHAR* pChar)
{
	if(GetAssaCodeEffect(pChar, CLASS_UP_PART1) == FALSE)
	{
		if( (pChar->HvRightHand.dwItemCode && pChar->HvRightHand.DispEffect == 1) ||
			(pChar->HvLeftHand.dwItemCode  && pChar->HvLeftHand.DispEffect == 1) )
		{
			int i = GetAssaEffect(); 
  			AssaClassUpWeapon *classUpWeapon = new AssaClassUpWeapon;
 			classUpWeapon->Start(pChar);
 			cAssaEffect[i] = classUpWeapon;
		}
	}
}

// aging +21..+25: the light orb around the held weapon (one per character; it ends by
// itself when the weapon goes or is no longer +21 or more)
int GetAgingOrbLevel( smCHARTOOL *lpCharTool );		// playsub.cpp
void AssaParticle_AgingOrb(smCHAR* pChar)
{
	if ( !pChar ) return;
	if ( GetAgingOrbLevel( &pChar->HvRightHand )<=0 && GetAgingOrbLevel( &pChar->HvLeftHand )<=0 ) return;
	if ( GetAssaCodeEffect(pChar, AGING_ORB_CODE)==FALSE ) {
		int i = GetAssaEffect();
		AssaAgingOrb *orb = new AssaAgingOrb;
		orb->Start(pChar);
		cAssaEffect[i] = orb;
	}
}

void AssaParticle_StopClassUpWeapon(smCHAR* pChar)
{
	for(int i = 0; i < cASSAEFFECT_MAX; i++)
	{
		if(cAssaEffect[i])
			if(cAssaEffect[i]->CODE == CLASS_UP_PART1)
				if(cAssaEffect[i]->pChar == pChar)
					cAssaEffect[i]->Type = STOP_PART_FLAG;
   	}
}

void AssaParticle_AgingBody(smCHAR* pChar, int num)
{
	if(num < 4 || num > 11)
		return;
	int i = GetAssaEffect();
	cAssaTraceCharMeshParticle *traceMeshParticle = new cAssaTraceCharMeshParticle;
	char nameBuffer[64];
	wsprintf(nameBuffer, "%s%d", "AgingBody", num);
	traceMeshParticle->Start(pChar, "Bip01 Spine1", nameBuffer);
	cAssaEffect[i] = traceMeshParticle;
}


void AssaParticle_MonsterIronSadNessHit1(smCHAR *pChar)
{
 	 GetMoveLocation( 0, 0, 30*fONE, 0, pChar->Angle.y-ANGLE_45, 0);
	
	
	int num = -1;
 	POINT3D pos;
	pos.x = pChar->pX+GeoResult_X;
	pos.y = pChar->pY+10000;
	pos.z = pChar->pZ+GeoResult_Z;
	
	GetMoveLocation( 0, 0, 30*fONE, 0, pChar->Angle.y, 0);

	//테스트용.
	D3DVECTOR  velocity;
	D3DVECTOR  gravity;
   	int i = GetAssaEffect(); 
  	velocity.x = float((float)GeoResult_X/15.f);
	velocity.y = 0.f;
	velocity.z = float((float)GeoResult_Z/15.f);

  	gravity.x = 0.f;
 	gravity.y = 0.f;
	gravity.z = 0.f;
	
	AssaSkill3BombShotExplision *explision = new AssaSkill3BombShotExplision;
	explision->Start(&pos, &velocity, &gravity, "IronMonsterBombHit2");
	cAssaEffect[i] = explision;
	
	
	num = g_NewParticleMgr.Start("MonsterIron1", pos);
	if(num != -1)
	{
		POINT3D angle;
		angle.x = 0;
		angle.y = pChar->Angle.y;
		angle.z = 0;
		g_NewParticleMgr.SetAngle(num, angle);
	}
	num = -1;
	
 	GetMoveLocation( 0, 0, 30*fONE, 0, pChar->Angle.y+ANGLE_45, 0);
	pos.x = pChar->pX+GeoResult_X;
	pos.y = pChar->pY+10000;
	pos.z = pChar->pZ+GeoResult_Z;
	
	
 	GetMoveLocation( 0, 0, 30*fONE, 0, pChar->Angle.y, 0);
	//테스트용.
	i = GetAssaEffect(); 
  	velocity.x = float((float)GeoResult_X/15.f);
	velocity.y = 0.f;
	velocity.z = float((float)GeoResult_Z/15.f);

  	gravity.x = 0.f;
 	gravity.y = 0.f;
	gravity.z = 0.f;
	
	explision = new AssaSkill3BombShotExplision;
	explision->Start(&pos, &velocity, &gravity, "IronMonsterBombHit2");
	cAssaEffect[i] = explision;
	
	
	num = g_NewParticleMgr.Start("MonsterIron1", pos);
	if(num != -1)
	{
		POINT3D angle;
		angle.x = 0;
		angle.y = pChar->Angle.y;
		angle.z = 0;
		g_NewParticleMgr.SetAngle(num, angle);
	}
}


void AssaParticle_MonsterIronSadNessSkill1(smCHAR *pChar)
{
    GetMoveLocation( 0, 0, 0, 0,0, 0);
	
 	int num = -1;
 	POINT3D pos;
	pos.x = pChar->pX+GeoResult_X;
	pos.y = pChar->pY+10000;
	pos.z = pChar->pZ+GeoResult_Z;
	
	GetMoveLocation( 0, 0, 30*fONE, 0, pChar->Angle.y, 0);
	//테스트용.
	D3DVECTOR  velocity;
	D3DVECTOR  gravity;
   	int i = GetAssaEffect(); 
  	velocity.x = float((float)GeoResult_X/8.f);
	velocity.y = 0.f;
	velocity.z = float((float)GeoResult_Z/8.f);

  	gravity.x = 0.f;
 	gravity.y = 0.f;
	gravity.z = 0.f;
	
 	AssaSkill3BombShotExplision *explision = new AssaSkill3BombShotExplision;
	explision->Start(&pos, &velocity, &gravity, "IronMonsterSadnessShot");
	cAssaEffect[i] = explision;
	
	/*
	num = g_NewParticleMgr.Start("MonsterIron1", pos);
	if(num != -1)
	{
		POINT3D angle;
		angle.x = 0;
		angle.y = pChar->Angle.y;
		angle.z = 0;
		g_NewParticleMgr.SetAngle(num, angle);
	}
	*/

}

void SkillArchMageFlameWave(smCHAR *pChar)
{
 	int factor = ANGLE_45+ANGLE_45/3;

	int num = -1;
	GetMoveLocation( 0, 0, 30*fONE, 0, pChar->Angle.y, 0);

  	POINT3D pos;
	pos.x = pChar->pX+GeoResult_X;
	pos.y = pChar->pY+10000;
	pos.z = pChar->pZ+GeoResult_Z;
	
	GetMoveLocation( 0, 0, 30*fONE, 0, pChar->Angle.y-factor/2, 0);
	//테스트용.
	D3DVECTOR  velocity;
	D3DVECTOR  gravity;
   	int i = GetAssaEffect(); 
  	velocity.x = float((float)GeoResult_X/6.f);
	velocity.y = 0.f;
	velocity.z = float((float)GeoResult_Z/6.f);

  	gravity.x = 0.f;
 	gravity.y = 0.f;
	gravity.z = 0.f;
	
 	AssaSkill3BombShotExplision *explision = new AssaSkill3BombShotExplision;
	explision->Start(&pos, &velocity, &gravity, "IronMonsterSadnessShot");
	cAssaEffect[i] = explision;
	

	GetMoveLocation( 0, 0, 30*fONE, 0, pChar->Angle.y-factor/3, 0);
	i = GetAssaEffect(); 
  	velocity.x = float((float)GeoResult_X/6.f);
	velocity.y = 0.f;
	velocity.z = float((float)GeoResult_Z/6.f);

  	gravity.x = 0.f;
 	gravity.y = 0.f;
	gravity.z = 0.f;
	
 	explision = new AssaSkill3BombShotExplision;
	explision->Start(&pos, &velocity, &gravity, "IronMonsterSadnessShot");
	cAssaEffect[i] = explision;

	GetMoveLocation( 0, 0, 30*fONE, 0, pChar->Angle.y, 0);
	i = GetAssaEffect(); 
  	velocity.x = float((float)GeoResult_X/6.f);
	velocity.y = 0.f;
	velocity.z = float((float)GeoResult_Z/6.f);

  	gravity.x = 0.f;
 	gravity.y = 0.f;
	gravity.z = 0.f;
	
 	explision = new AssaSkill3BombShotExplision;
	explision->Start(&pos, &velocity, &gravity, "IronMonsterSadnessShot");
	cAssaEffect[i] = explision;

	GetMoveLocation( 0, 0, 30*fONE, 0, pChar->Angle.y+factor/2, 0);
	i = GetAssaEffect(); 
  	velocity.x = float((float)GeoResult_X/6.f);
	velocity.y = 0.f;
	velocity.z = float((float)GeoResult_Z/6.f);

  	gravity.x = 0.f;
 	gravity.y = 0.f;
	gravity.z = 0.f;
	
 	explision = new AssaSkill3BombShotExplision;
	explision->Start(&pos, &velocity, &gravity, "IronMonsterSadnessShot");
	cAssaEffect[i] = explision;

	GetMoveLocation( 0, 0, 30*fONE, 0, pChar->Angle.y+factor/3, 0);
	i = GetAssaEffect(); 
  	velocity.x = float((float)GeoResult_X/6.f);
	velocity.y = 0.f;
	velocity.z = float((float)GeoResult_Z/6.f);

  	gravity.x = 0.f;
 	gravity.y = 0.f;
	gravity.z = 0.f;
	
 	explision = new AssaSkill3BombShotExplision;
	explision->Start(&pos, &velocity, &gravity, "IronMonsterSadnessShot");
	cAssaEffect[i] = explision;
}

void AssaParticle_MonsterIronHyperHit1(smCHAR *pChar, POINT3D* pDest)
{
    GetMoveLocation(0, 0, 50*fONE, 0, pChar->Angle.y, 0);

	POINT3D pos;
  	pos.x = pChar->pX+GeoResult_X;
	pos.y = pChar->pY + 12000;
	pos.z = pChar->pZ+GeoResult_Z;

 	StartEffectMonsterDest(pos.x, pos.y, pos.z, pDest->x, pDest->y, pDest->z, MONSTER_DMACHINE_MISSILE1);
}

void AssaParticle_MonsterIronHyperSkill1(smCHAR *pChar, POINT3D* pDest)
{
    GetMoveLocation(0, 0, 50*fONE, 0, pChar->Angle.y, 0);

	POINT3D pos;
  	pos.x = pChar->pX+GeoResult_X;
	pos.y = pChar->pY + 12000;
	pos.z = pChar->pZ+GeoResult_Z;

 	StartEffectMonsterDest(pos.x, pos.y, pos.z, pDest->x, pDest->y, pDest->z, MONSTER_DMACHINE_MISSILE1);
}

//////////////////////////////////////////////////////////////////////////////

void ParkKelvezuHit(smCHAR *pChar)		// 켈베쥬 공격시 캐릭터 피격이펙트 (번쩍이는)
{
 	POINT3D pos;
    GetMoveLocation( 0, 0, 0, 0, pChar->Angle.y, 0);

    pos.x = pChar->pX+GeoResult_X;
	pos.y = pChar->pY+7000;
	pos.z = pChar->pZ+GeoResult_Z;
	
 	g_NewParticleMgr.Start("IceKelvezuHit", pos);
}

void ParkKelvezuSkillHit(smCHAR *pChar)	// 켈베쥬 공격시 캐릭터 피격이펙트
{
	POINT3D pos;

    GetMoveLocation( 0, 0, 0, 0, pChar->Angle.y, 0);

	pos.x = pChar->pX+GeoResult_X;
	pos.y = pChar->pY+1000;
	pos.z = pChar->pZ+GeoResult_Z;
	
 	g_NewParticleMgr.Start("IceKelvezuNormal2", pos);
}

void ParkKelvezuNormal1(smCHAR *pChar)	// 켈베쥬 노말공격1 사용시 캐릭터 피격이펙트
{
 	POINT3D pos;
    GetMoveLocation( 0, 0, 0, 0, pChar->Angle.y, 0);

    pos.x = pChar->pX+GeoResult_X;
	pos.y = pChar->pY+1000;
	pos.z = pChar->pZ+GeoResult_Z;
	
 	g_NewParticleMgr.Start("IceKelvezuNormal1", pos);
}

void ParkKelvezuNormal2(smCHAR *pKelvezu)	// 켈베쥬 노말공격2 사용시 캐릭터 피격이펙트
{
 	POINT3D pos;

 	pos.x = pKelvezu->pX;
	pos.y = pKelvezu->pY;
	pos.z = pKelvezu->pZ;
  
    GetMoveLocation( 10*fONE, 0, 250*fONE, pKelvezu->Angle.x, pKelvezu->Angle.y, 1);
	pos.x += GeoResult_X;
	pos.z += GeoResult_Z;
	pos.y += GeoResult_Y;
	
  	Assa = SetAssaEffect(0,"m_ratoo.ASE",0,&pos,0);
	cAssaEffect[Assa]->Time = -4;
	cAssaEffect[Assa]->AniMaxCount = 20;
	cAssaEffect[Assa]->AniDelayTime = 5;
	cAssaEffect[Assa]->Angle.y = (-pKelvezu->Angle.y+ANGLE_180)&ANGCLIP;

	POINT3D partPos;
  	partPos.x = cAssaEffect[Assa]->Posi.x;
	partPos.y = cAssaEffect[Assa]->Posi.y+2000;
	partPos.z = cAssaEffect[Assa]->Posi.z;
	g_NewParticleMgr.Start("IceKelvezuNormal3",partPos);

	//sinSkillEffect_Brandish(pChar);
}

void ParkKelvezuSkill1(smCHAR *pKelvezu)	// 켈베쥬 스킬공격1 사용시 캐릭터 피격이펙트
{
	float fVelocity;
 	int factor = ANGLE_45+ANGLE_45/3;

	fVelocity = 0.2f;

	int num = -1;
	GetMoveLocation( 0, 0, 160*fONE, pKelvezu->Angle.x, pKelvezu->Angle.y, 1);
	//GetMoveLocation( 0, 0, 30*fONE, pKelvezu->Angle.x, pKelvezu->Angle.y, 1);

  	POINT3D pos;
	pos.x = pKelvezu->pX+GeoResult_X;
	pos.y = pKelvezu->pY+23000;
	pos.z = pKelvezu->pZ+GeoResult_Z;
	
	GetMoveLocation( 0, 0, fONE, 0, pKelvezu->Angle.y-factor, 1);
	//테스트용.
	D3DVECTOR  velocity;
	D3DVECTOR  gravity;
   	int i = GetAssaEffect(); 
  	velocity.x = float((float)GeoResult_X/fVelocity);
	velocity.y = -1000;
	velocity.z = float((float)GeoResult_Z/fVelocity);

  	gravity.x = 0;
 	gravity.y = 0;
	gravity.z = 0;
	
 	AssaSkill3BombShotExplision *explision = new AssaSkill3BombShotExplision;
	explision->Start(&pos, &velocity, &gravity, "IronMonsterSadnessShot");
	cAssaEffect[i] = explision;
	cAssaEffect[i]->Gravity = 10000;

	GetMoveLocation( 0, 0, fONE, 0, pKelvezu->Angle.y-factor/2, 1);
   	i = GetAssaEffect(); 
  	velocity.x = float((float)GeoResult_X/fVelocity);
	velocity.y = -1000;
	velocity.z = float((float)GeoResult_Z/fVelocity);

  	gravity.x = 0;
 	gravity.y = 0;
	gravity.z = 0;
	
 	explision = new AssaSkill3BombShotExplision;
	explision->Start(&pos, &velocity, &gravity, "IronMonsterSadnessShot");
	cAssaEffect[i] = explision;
	cAssaEffect[i]->Gravity = 10000;
	

	GetMoveLocation( 0, 0, fONE, 0, pKelvezu->Angle.y-factor/3, 1);
	i = GetAssaEffect(); 
  	velocity.x = float((float)GeoResult_X/fVelocity);
	velocity.y = -1000;
	velocity.z = float((float)GeoResult_Z/fVelocity);

  	gravity.x = 0;
 	gravity.y = 0;
	gravity.z = 0;
	
 	explision = new AssaSkill3BombShotExplision;
	explision->Start(&pos, &velocity, &gravity, "IronMonsterSadnessShot");
	cAssaEffect[i] = explision;
	cAssaEffect[i]->Gravity = 10000;

	GetMoveLocation( 0, 0, fONE, 0, pKelvezu->Angle.y, 1);
	i = GetAssaEffect(); 
  	velocity.x = float((float)GeoResult_X/fVelocity);
	velocity.y = -1000.f;
	velocity.z = float((float)GeoResult_Z/fVelocity);

  	gravity.x = 0.f;
 	gravity.y = 0.f;
	gravity.z = 0.f;
	
 	explision = new AssaSkill3BombShotExplision;
	explision->Start(&pos, &velocity, &gravity, "IronMonsterSadnessShot");
	cAssaEffect[i] = explision;
	cAssaEffect[i]->Gravity = 10000;

	GetMoveLocation( 0, 0, fONE, 0, pKelvezu->Angle.y+factor/2, 1);
	i = GetAssaEffect(); 
  	velocity.x = float((float)GeoResult_X/fVelocity);
	velocity.y = -1000.f;
	velocity.z = float((float)GeoResult_Z/fVelocity);

  	gravity.x = 0.f;
 	gravity.y = 0.f;
	gravity.z = 0.f;
	
 	explision = new AssaSkill3BombShotExplision;
	explision->Start(&pos, &velocity, &gravity, "IronMonsterSadnessShot");
	cAssaEffect[i] = explision;
	cAssaEffect[i]->Gravity = 10000;

	GetMoveLocation( 0, 0, fONE, 0, pKelvezu->Angle.y+factor/3, 1);
	i = GetAssaEffect(); 
  	velocity.x = float((float)GeoResult_X/fVelocity);
	velocity.y = -1000.f;
	velocity.z = float((float)GeoResult_Z/fVelocity);

  	gravity.x = 0.f;
 	gravity.y = 0.f;
	gravity.z = 0.f;
	
 	explision = new AssaSkill3BombShotExplision;
	explision->Start(&pos, &velocity, &gravity, "IronMonsterSadnessShot");
	cAssaEffect[i] = explision;
	cAssaEffect[i]->Gravity = 10000;

	GetMoveLocation( 0, 0, fONE, 0, pKelvezu->Angle.y+factor, 1);
	i = GetAssaEffect(); 
  	velocity.x = float((float)GeoResult_X/fVelocity);
	velocity.y = -1000.f;
	velocity.z = float((float)GeoResult_Z/fVelocity);

  	gravity.x = 0.f;
 	gravity.y = 0.f;
	gravity.z = 0.f;
	
 	explision = new AssaSkill3BombShotExplision;
	explision->Start(&pos, &velocity, &gravity, "IronMonsterSadnessShot");
	cAssaEffect[i] = explision;
	cAssaEffect[i]->Gravity = 10000;
	
}

void ParkKelvezuSkill2(smCHAR *pKelvezu)	// 켈베쥬 스킬공격1 사용시 캐릭터 피격이펙트
{
	POINT3D curPos;
	POINT3D attackPos;
	GetMoveLocation( 0, 0, 260*fONE, pKelvezu->Angle.x, pKelvezu->Angle.y, 1);

	attackPos.x = pKelvezu->pX+GeoResult_X;
	attackPos.y = pKelvezu->pY+GeoResult_Y;
	attackPos.z = pKelvezu->pZ+GeoResult_Z;

	curPos.x = pKelvezu->pX;
	curPos.y = pKelvezu->pY+130000;
	curPos.z = pKelvezu->pZ+50000;
	AssaParticle_TerrainFire(&curPos, &attackPos);

	GetMoveLocation( 0, 0, 210*fONE, pKelvezu->Angle.x, pKelvezu->Angle.y+ANGLE_45, 1);

	attackPos.x = pKelvezu->pX+GeoResult_X;
	attackPos.y = pKelvezu->pY+GeoResult_Y;
	attackPos.z = pKelvezu->pZ+GeoResult_Z;

	curPos.x = pKelvezu->pX;
	curPos.y = pKelvezu->pY+130000;
	curPos.z = pKelvezu->pZ+50000;
	AssaParticle_TerrainFire(&curPos, &attackPos, 10);

	GetMoveLocation( 0, 0, 160*fONE, pKelvezu->Angle.x, pKelvezu->Angle.y+ANGLE_90, 1);

	attackPos.x = pKelvezu->pX+GeoResult_X;
	attackPos.y = pKelvezu->pY+GeoResult_Y;
	attackPos.z = pKelvezu->pZ+GeoResult_Z;

	curPos.x = pKelvezu->pX;
	curPos.y = pKelvezu->pY+130000;
	curPos.z = pKelvezu->pZ+50000;
	AssaParticle_TerrainFire(&curPos, &attackPos, 20);

	GetMoveLocation( 0, 0, 210*fONE, pKelvezu->Angle.x, pKelvezu->Angle.y+ANGLE_45+ANGLE_90, 1);

	attackPos.x = pKelvezu->pX+GeoResult_X;
	attackPos.y = pKelvezu->pY+GeoResult_Y;
	attackPos.z = pKelvezu->pZ+GeoResult_Z;

	curPos.x = pKelvezu->pX;
	curPos.y = pKelvezu->pY+130000;
	curPos.z = pKelvezu->pZ+50000;
	AssaParticle_TerrainFire(&curPos, &attackPos, 30);

	GetMoveLocation( 0, 0, 260*fONE, pKelvezu->Angle.x, pKelvezu->Angle.y+ANGLE_180, 1);

	attackPos.x = pKelvezu->pX+GeoResult_X;
	attackPos.y = pKelvezu->pY+GeoResult_Y;
	attackPos.z = pKelvezu->pZ+GeoResult_Z;

	curPos.x = pKelvezu->pX;
	curPos.y = pKelvezu->pY+130000;
	curPos.z = pKelvezu->pZ+50000;
	AssaParticle_TerrainFire(&curPos, &attackPos, 40);

	GetMoveLocation( 0, 0, 210*fONE, pKelvezu->Angle.x, pKelvezu->Angle.y+ANGLE_180+ANGLE_45, 1);

	attackPos.x = pKelvezu->pX+GeoResult_X;
	attackPos.y = pKelvezu->pY+GeoResult_Y;
	attackPos.z = pKelvezu->pZ+GeoResult_Z;

	curPos.x = pKelvezu->pX;
	curPos.y = pKelvezu->pY+130000;
	curPos.z = pKelvezu->pZ+50000;
	AssaParticle_TerrainFire(&curPos, &attackPos, 50);

	GetMoveLocation( 0, 0, 160*fONE, pKelvezu->Angle.x, pKelvezu->Angle.y+ANGLE_180+ANGLE_90, 1);

	attackPos.x = pKelvezu->pX+GeoResult_X;
	attackPos.y = pKelvezu->pY+GeoResult_Y;
	attackPos.z = pKelvezu->pZ+GeoResult_Z;

	curPos.x = pKelvezu->pX;
	curPos.y = pKelvezu->pY+130000;
	curPos.z = pKelvezu->pZ+50000;
	AssaParticle_TerrainFire(&curPos, &attackPos, 60);
	GetMoveLocation( 0, 0, 210*fONE, pKelvezu->Angle.x, pKelvezu->Angle.y+ANGLE_180+ANGLE_90+ANGLE_45, 1);

	attackPos.x = pKelvezu->pX+GeoResult_X;
	attackPos.y = pKelvezu->pY+GeoResult_Y;
	attackPos.z = pKelvezu->pZ+GeoResult_Z;

	curPos.x = pKelvezu->pX;
	curPos.y = pKelvezu->pY+130000;
	curPos.z = pKelvezu->pZ+50000;
	AssaParticle_TerrainFire(&curPos, &attackPos, 70);

}

// 켈베쥬 마법진및 입에서 연기나는거
void ParkKelvezuJin(smCHAR *pKelvezu)
{
	if(GetAssaCodeEffect(pKelvezu, MONSTER_KELVEZU_CIRCLE) == FALSE)
	{
     	int i = GetAssaEffect();
		cAssaTraceCharMeshParticle *traceMeshParticle = new cAssaTraceCharMeshParticle;
		traceMeshParticle->Start(pKelvezu, "Bip01 Bae", "IceKelvezuBossJin1");
		cAssaEffect[i] = traceMeshParticle;
		cAssaEffect[i]->Posi.y = cAssaEffect[i]->Posi.y - 15000;
		cAssaEffect[i]->CODE = MONSTER_KELVEZU_CIRCLE;

		i = GetAssaEffect();
		traceMeshParticle = new cAssaTraceCharMeshParticle;
		traceMeshParticle->Start(pKelvezu, "BIP01 Mouse", "IceKelvezuBossMouseJin1");
		cAssaEffect[i] = traceMeshParticle;
		cAssaEffect[i]->CODE = MONSTER_KELVEZU_CIRCLE;

	}
}

// 켈베쥬 공격시 발톱에서 검기효과
void ParkKelvezuFingerShow(smCHAR* pKelvezu, int nTime)
{
	if(pKelvezu)
	{
		int i = GetAssaEffect();
		cAssaMotionBlur *motionBlur = new cAssaMotionBlur;
		motionBlur->Start(pKelvezu, "Bip01 Hand", "Bip01 Fin03", nTime);
		motionBlur->Color.R = 200;
		motionBlur->Color.G = 100;
		motionBlur->Color.B = 200;
		motionBlur->Color.A = 100;
		cAssaEffect[i] = motionBlur;

		i = GetAssaEffect();
		motionBlur = new cAssaMotionBlur;
		motionBlur->Start(pKelvezu, "Bip01 Hand", "Bip01 Fin02", nTime);
		motionBlur->Color.R = 200;
		motionBlur->Color.G = 100;
		motionBlur->Color.B = 200;
		motionBlur->Color.A = 100;
		cAssaEffect[i] = motionBlur;

		i = GetAssaEffect();
		motionBlur = new cAssaMotionBlur;
		motionBlur->Start(pKelvezu, "Bip01 Hand", "Bip01 Fin01", nTime);
		motionBlur->Color.R = 200;
		motionBlur->Color.G = 100;
		motionBlur->Color.B = 200;
		motionBlur->Color.A = 100;
		cAssaEffect[i] = motionBlur;
	}
}

// 켈베쥬 공격시 날개에서 검기효과
void ParkKelvezuWingShow(smCHAR* pKelvezu, int nTime)
{
	if(pKelvezu)
	{
		int i = GetAssaEffect();
		cAssaMotionBlur *motionBlur = new cAssaMotionBlur;
		motionBlur->Start(pKelvezu, "Bip01 Rwing03", "Bip01 Rwing02", nTime);
		motionBlur->Color.R = 200;
		motionBlur->Color.G = 100;
		motionBlur->Color.B = 200;
		motionBlur->Color.A = 100;
		cAssaEffect[i] = motionBlur;

		i = GetAssaEffect();
		motionBlur = new cAssaMotionBlur;
		motionBlur->Start(pKelvezu, "Bip01 Rwing02", "Bip01 Rwing01", nTime);
		motionBlur->Color.R = 200;
		motionBlur->Color.G = 100;
		motionBlur->Color.B = 200;
		motionBlur->Color.A = 100;
		cAssaEffect[i] = motionBlur;

		i = GetAssaEffect();
		motionBlur = new cAssaMotionBlur;
		motionBlur->Start(pKelvezu, "Bip01 Lwing03", "Bip01 Lwing02", nTime);
		motionBlur->Color.R = 200;
		motionBlur->Color.G = 100;
		motionBlur->Color.B = 200;
		motionBlur->Color.A = 100;
		cAssaEffect[i] = motionBlur;

		i = GetAssaEffect();
		motionBlur = new cAssaMotionBlur;
		motionBlur->Start(pKelvezu, "Bip01 Lwing02", "Bip01 Lwing01", nTime);
		motionBlur->Color.R = 200;
		motionBlur->Color.G = 100;
		motionBlur->Color.B = 200;
		motionBlur->Color.A = 100;
		cAssaEffect[i] = motionBlur;
	}
}

// 켈베쥬 공격시 꼬리에서 검기효과
void ParkKelvezuTaleShow(smCHAR* pKelvezu, int nTime)
{
	if(pKelvezu)
	{
		int i = GetAssaEffect();
		cAssaMotionBlur *motionBlur = new cAssaMotionBlur;
		motionBlur->Start(pKelvezu, "Bip01 Tale01", "Bip01 Tale02", nTime);
		motionBlur->Color.R = 200;
		motionBlur->Color.G = 100;
		motionBlur->Color.B = 200;
		motionBlur->Color.A = 100;
		cAssaEffect[i] = motionBlur;

		i = GetAssaEffect();
		motionBlur = new cAssaMotionBlur;
		motionBlur->Start(pKelvezu, "Bip01 Tale02", "Bip01 Tale03", nTime);
		motionBlur->Color.R = 200;
		motionBlur->Color.G = 100;
		motionBlur->Color.B = 200;
		motionBlur->Color.A = 100;
		cAssaEffect[i] = motionBlur;
	}
}

// 월드컵 몬스터 이펙트
void ParkAssaParticle_WorldCupMonsterAttack(smCHAR* pMonster, smCHAR *pChar)
{
	GetMoveLocation( 0, 0, 0, 0, pMonster->Angle.y, 0);
	POINT3D pCur, pDest;
	pCur.x = pMonster->pX+GeoResult_X;
	pCur.y = pMonster->pY+3000;
	pCur.z = pMonster->pZ+GeoResult_Z;

	GetMoveLocation( 0, 30*fONE, 0, 0, pChar->Angle.y, 0);
	pDest.x = pChar->pX+GeoResult_X;
	pDest.y = pChar->pY+GeoResult_Y;
	pDest.z = pChar->pZ+GeoResult_Z;

	Assa = SetAssaEffect(0,"bball.ASE",0, &pCur,0);
	
	cAssaEffect[Assa]->Time = 0;
	cAssaEffect[Assa]->AniMaxCount = 60;
	cAssaEffect[Assa]->AniDelayTime = 4;
 	cAssaEffect[Assa]->MOVECODE = ASSA_ANGLE_ROTATE;
	cAssaEffect[Assa]->Angle.y = ANGLE_1;

	GetRadian3D( pCur.x, pCur.y, pCur.z, pDest.x, pDest.y, pDest.z);

	cAssaEffect[Assa]->Angle.x =  GeoResult_X;
	cAssaEffect[Assa]->Angle.y =  GeoResult_Y;
	cAssaEffect[Assa]->Angle.z =  0;
	
	cAssaEffect[Assa]->Angle.y = (-GeoResult_Y+ANGLE_1)&ANGCLIP;
	cAssaEffect[Assa]->NotFadeIn = TRUE;
	cAssaEffect[Assa]->ARotateSpeed.z = 100;
	int i = GetAssaEffect();
	cASSAPARTSHOT *partShot = new cASSAPARTSHOT;
	partShot->Start(&pCur, &pDest, cAssaEffect[Assa], SKILL_TWIST_JAVELIN);
	cAssaEffect[i] = partShot;
	/*

	int i = GetAssaEffect(); 
	ParkMeshEffect *WorldCupMonster = new ParkMeshEffect;
	WorldCupMonster->Start(&pCur, &pDest, "Arrow.ase" , NULL, "twistJavelin1");
	cAssaEffect[i] = WorldCupMonster;
	*/
}

void ParkWorldCupMonsterHit(smCHAR *pChar)	// 월드컵 몬스터가 공격시 캐릭터 피격이펙트
{
	POINT3D pos;

    GetMoveLocation( 0, 0, 0, 0, pChar->Angle.y, 0);

	pos.x = pChar->pX+GeoResult_X;
	pos.y = pChar->pY+7000;
	pos.z = pChar->pZ+GeoResult_Z;
	
 	g_NewParticleMgr.Start("MonsterHit2", pos);
}
//////////////////////////////  박철호  ////////////////////////////////////////
void ParkAssaParticle_MonsterIceChaosKara(smCHAR *pChar)
{
	if(GetAssaCodeEffect(pChar, ICE_MONSTER_CHAOSKARA_STAFF) == FALSE)
	{
     	int i = GetAssaEffect();
		cAssaTraceCharMeshParticle *traceMeshParticle = new cAssaTraceCharMeshParticle;
		traceMeshParticle->Start(pChar, "Bip01 ob", "ChaosKaraStaff");
		cAssaEffect[i] = traceMeshParticle;
		cAssaEffect[i]->CODE = ICE_MONSTER_CHAOSKARA_STAFF;

	}
}

void ParkAssaParticle_MonsterIceValentoBody(smCHAR *pValento)
{
	if(GetAssaCodeEffect(pValento, ICE_MONSTER_VALENTO_BODY) == FALSE)
	{
		int i = GetAssaEffect();
		cAssaTraceCharMeshParticle *traceMeshParticle = new cAssaTraceCharMeshParticle;
		traceMeshParticle->Start(pValento, "BIP01fire07", "IceValentoLEye");
		traceMeshParticle->SetScale(float(pValento->smCharInfo.wPlayClass[1])/256.f);
		cAssaEffect[i] = traceMeshParticle;
		cAssaEffect[i]->CODE = ICE_MONSTER_VALENTO_BODY;
		
 		i = GetAssaEffect();
		traceMeshParticle = new cAssaTraceCharMeshParticle;
		traceMeshParticle->Start(pValento, "BIP01fire08", "IceValentoREye");
		traceMeshParticle->SetScale(float(pValento->smCharInfo.wPlayClass[1])/256.f);
		cAssaEffect[i] = traceMeshParticle;
		cAssaEffect[i]->CODE = ICE_MONSTER_VALENTO_BODY;


		//몸통
     	i = GetAssaEffect();
		traceMeshParticle = new cAssaTraceCharMeshParticle;
		traceMeshParticle->Start(pValento, "BIP01fire01", "IceValentoBody");
		cAssaEffect[i] = traceMeshParticle;
		cAssaEffect[i]->CODE = ICE_MONSTER_VALENTO_BODY;


		// 팔꿈치
		i = GetAssaEffect();
		traceMeshParticle = new cAssaTraceCharMeshParticle;
		traceMeshParticle->Start(pValento, "BIP01fire03", "IceValentoJoint");
		cAssaEffect[i] = traceMeshParticle;
		cAssaEffect[i]->CODE = ICE_MONSTER_VALENTO_BODY;

		i = GetAssaEffect();
		traceMeshParticle = new cAssaTraceCharMeshParticle;
		traceMeshParticle->Start(pValento, "BIP01fire04", "IceValentoJoint");
		cAssaEffect[i] = traceMeshParticle;
		cAssaEffect[i]->CODE = ICE_MONSTER_VALENTO_BODY;

		// 무릎
		i = GetAssaEffect();
		traceMeshParticle = new cAssaTraceCharMeshParticle;
		traceMeshParticle->Start(pValento, "BIP01fire05", "IceValentoJoint");
		cAssaEffect[i] = traceMeshParticle;
		cAssaEffect[i]->CODE = ICE_MONSTER_VALENTO_BODY;

		i = GetAssaEffect();
		traceMeshParticle = new cAssaTraceCharMeshParticle;
		traceMeshParticle->Start(pValento, "BIP01fire06", "IceValentoJoint");
		cAssaEffect[i] = traceMeshParticle;
		cAssaEffect[i]->CODE = ICE_MONSTER_VALENTO_BODY;

	}
}

void ParkAssaParticle_MonsterTurtleCanonNormal(smCHAR *pTCanon, smCHAR* pTarget)
{
	GetMoveLocation( 0 , 30*fONE, 10*fONE , 0, pTCanon->Angle.y , 0 );
	
	StartEffectMonsterDest(pTCanon->pX+GeoResult_X, pTCanon->pY+GeoResult_Y,pTCanon->pZ+GeoResult_Z, 
						pTarget->pX , pTarget->pY+24*fONE , pTarget->pZ ,
						MONSTER_TURTLECANON_SHOT);

 	//StartEffectMonsterDest(pos.x, pos.y, pos.z, pDest->x, pDest->y, pDest->z, MONSTER_TURTLECANON_SHOT);
}


// 폭죽 이펙트
void ParkAssaParticle_FireCracker(smCHAR *pChar, int nFireCracker )
{
	if(GetAssaCodeEffect(pChar, EFFECT_FIRECRACKER) == FALSE)
	{
		int xpos, ypos, zpos;

		POINT3D BomePos;	// 터지는 위치

		xpos = GetRandomPos(-20, 20)*fONE;
		ypos = GetRandomPos(-5, 5);
		zpos = GetRandomPos(-20, 20)*fONE;
		GetMoveLocation( xpos , (ypos+50)*fONE, zpos , 0, 0, 0 );

		BomePos.x = pChar->pX+GeoResult_X;
		BomePos.y = pChar->pY+ 100 *fONE+GeoResult_Y; //(60+GetRandomPos(-20, 50))*fONE+GeoResult_Y;
		BomePos.z = pChar->pZ+GeoResult_Z;

		StartEffectMonsterDest( pChar->pX+GeoResult_X,pChar->pY+GeoResult_Y,pChar->pZ+GeoResult_Z, 
				 BomePos.x, BomePos.y , BomePos.z ,	EFFECT_FIRE_CRACKER );


		switch (TableFireCracker[nFireCracker].nCracker)
		{
			case 0:
				g_NewParticleMgr.Start("FireCrackerAll", BomePos, 1.0f);
				break;

			case 1:
				g_NewParticleMgr.Start("FireCrackerHeart", BomePos, 1.0f);
				break;

			case 2:
				g_NewParticleMgr.Start("FireCrackerMoon", BomePos, 1.0f);
				break;

			case 3:
				g_NewParticleMgr.Start("FireCrackerStar", BomePos, 1.0f);
				break;

			case 4:
				g_NewParticleMgr.Start("FireCrackerNormal", BomePos, 1.0f);
				break;
		}

		switch(TableFireCracker[nFireCracker].nBome)
		{
			case 0:
				g_NewParticleMgr.Start("FireCrackerBombHeart", BomePos, 1.0f);
				break;
			case 1:
				g_NewParticleMgr.Start("FireCrackerBombMoon", BomePos, 1.0f);
				break;
			case 2:
				g_NewParticleMgr.Start("FireCrackerBombStar", BomePos, 1.0f);
				break;
			default:
				break;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////

void AssaParticle_MonsterIronChainGolem(smCHAR *pChar)
{
	if(GetAssaCodeEffect(pChar, IRON_MONSTER_CHAINGOLEM1) == FALSE)
	{
    	int i = GetAssaEffect();
		cAssaTraceCharMeshParticle *traceMeshParticle = new cAssaTraceCharMeshParticle;
		traceMeshParticle->Start(pChar, "Bip01 FX", "IronMonsterChainGolem1");
		cAssaEffect[i] = traceMeshParticle;
		cAssaEffect[i]->CODE = IRON_MONSTER_CHAINGOLEM1;

        i = GetAssaEffect();
		traceMeshParticle = new cAssaTraceCharMeshParticle;
		traceMeshParticle->Start(pChar, "Bip01 chain26", "IronMonsterChainGolem2", false);
		cAssaEffect[i] = traceMeshParticle;
		cAssaEffect[i]->CODE = IRON_MONSTER_CHAINGOLEM1;

	}
}

void AssaParticle_MonsterIronRunicGuardian(smCHAR *pChar, smCHAR *pDestChar)
{
	POINT3D curPos;
	POINT3D desPos;
    
 	curPos.x = pChar->pX;
	curPos.y = pChar->pY+40000;
	curPos.z = pChar->pZ;

	desPos.x = pDestChar->pX;
	desPos.y = pDestChar->pY+5000;
	desPos.z = pDestChar->pZ;

   	g_NewParticleMgr.Start("IronMonsterRunicGuardian", curPos);
	g_NewParticleMgr.Start("IronMonsterRunicGuardianLight1", curPos, 1.f);	
 	g_NewParticleMgr.Start("IronMonsterRunicGuardianLight2", desPos, 1.f);
	
 	//SetDynLight( desPos.x, desPos.y, desPos.z, 255, 255, 255, 255, 150, 2);

    Assa = SetAssaEffect(200,"spark04_01.bmp",0,&curPos,0);
	AssaGetAniFileMat(cAssaEffect[Assa]);
	cAssaEffect[Assa]->AniDelayTime = 3;
	cAssaEffect[Assa]->Face.r = 255;
	cAssaEffect[Assa]->Face.g = 255;
	cAssaEffect[Assa]->Face.b = 255;
	cAssaEffect[Assa]->Face.Transparency = 255;
	cAssaEffect[Assa]->Size.w = 8000;
	cAssaEffect[Assa]->FACE_TYPE = ASSAFACE_SCALE;
	cAssaEffect[Assa]->Time = -70;
	
	cAssaEffect[Assa]->ScalePosi.x = desPos.x - curPos.x;
	cAssaEffect[Assa]->ScalePosi.y = desPos.y - curPos.y;
	cAssaEffect[Assa]->ScalePosi.z = desPos.z - curPos.z;

  	Assa = SetAssaEffect(200,"light2_01.bmp",0,&curPos,0);
	AssaGetAniFileMat(cAssaEffect[Assa]);
	cAssaEffect[Assa]->AniDelayTime = 3;
	cAssaEffect[Assa]->Face.r = 255;
	cAssaEffect[Assa]->Face.g = 255;
	cAssaEffect[Assa]->Face.b = 255;
	cAssaEffect[Assa]->Face.Transparency = 255;
	cAssaEffect[Assa]->Size.w = 8000;
	cAssaEffect[Assa]->FACE_TYPE = ASSAFACE_SCALE; 
	cAssaEffect[Assa]->Time = -70;


	cAssaEffect[Assa]->ScalePosi.x = desPos.x - curPos.x;
	cAssaEffect[Assa]->ScalePosi.y = desPos.y - curPos.y;
	cAssaEffect[Assa]->ScalePosi.z = desPos.z - curPos.z;

}




void AssaParticle_MonsterIronMountainHit1(smCHAR *pChar)
{
	//Mountain
 	POINT3D pos;
    GetMoveLocation( 0, 0, 100*fONE, 0, pChar->Angle.y, 0);
	pos.x = pChar->pX+GeoResult_X;
	pos.y = pChar->pY+2000;
	pos.z = pChar->pZ+GeoResult_Z;
	
 	g_NewParticleMgr.Start("IronMonsterMountainHit1", pos);
}

void AssaParticle_MonsterIronTowerGolemHit1(smCHAR *pChar)
{
	 POINT3D pos;
    pos.x = pChar->pX;
	pos.y = pChar->pY;
	pos.z = pChar->pZ;
	
   	GetMoveLocation( 0, 0, -50*fONE, pChar->Angle.x, pChar->Angle.y, 0);
	pos.x += GeoResult_X;
	pos.z += GeoResult_Z;
    
	Assa = SetAssaEffect(0,"m_StoneGolem.ASE",0,&pos,0, 1);
	cAssaEffect[Assa]->AniMaxCount = 30;
	cAssaEffect[Assa]->AniDelayTime = 2;
	cAssaEffect[Assa]->Angle.y = (-pChar->Angle.y+ANGLE_180)&ANGCLIP;
	cAssaEffect[Assa]->NotFadeIn = TRUE;
	
	pos.x = pos.x;
	pos.y = pos.y+1500;
	pos.z = pos.z;
	
 	GetMoveLocation( 0, 0, 120*fONE, pChar->Angle.x, pChar->Angle.y, 0);
	pos.x += GeoResult_X;
	pos.z += GeoResult_Z;
	g_NewParticleMgr.Start("StoneGolemHit1", pos);
}

void AssaParticle_MonsterIronTowerGolemSkill1(smCHAR *pChar)
{
 	GetMoveLocation( 0, 0, 30*fONE, 0, pChar->Angle.y, 0);
	
  	int num = -1;
 	POINT3D pos;
	pos.x = pChar->pX+GeoResult_X;
	pos.y = pChar->pY+3000;
	pos.z = pChar->pZ+GeoResult_Z;
	
	GetMoveLocation( 0, 0, 30*fONE, 0, pChar->Angle.y, 0);

	//테스트용.
	D3DVECTOR  velocity;
	D3DVECTOR  gravity;
   	int i = GetAssaEffect();
  	velocity.x = float((float)GeoResult_X/8.f);
	velocity.y = 0.f;
	velocity.z = float((float)GeoResult_Z/8.f);

  	gravity.x = 0.f;
 	gravity.y = 0.f;
	gravity.z = 0.f;
	
	AssaSkill3BombShotExplision *explision = new AssaSkill3BombShotExplision;
	explision->Start(&pos, &velocity, &gravity, "IronMonsterBombHit2");
	cAssaEffect[i] = explision;
	
	
	num = g_NewParticleMgr.Start("MonsterIron1", pos);
	if(num != -1)
	{
		POINT3D angle;
		angle.x = 0;
		angle.y = pChar->Angle.y;
		angle.z = 0;
		g_NewParticleMgr.SetAngle(num, angle);
	}
}


void AssaParticle_MonsterAvelArcher(smCHAR *pChar, smCHAR *pDestChar)
{
  	GetMoveLocation( 0 , 60*fONE, 20*fONE , 0, pChar->Angle.y , 0 );
	POINT3D curPos;
	POINT3D desPos;
	curPos.x = pChar->pX+GeoResult_X;
	curPos.y = pChar->pY+GeoResult_Y;
	curPos.z = pChar->pZ+GeoResult_Z;
	
	desPos.x = pDestChar->pX;
	desPos.y = pDestChar->pY+24*fONE;
	desPos.z = pDestChar->pZ;
	
	StartEffectMonsterDest(curPos.x, curPos.y, curPos.z, desPos.x, desPos.y, desPos.z, MONSTER_SERQBUS_SHOT1 );

}


void AssaParticle_MonsterAvelQueen(smCHAR *pChar, smCHAR *pDestChar)
{
	GetMoveLocation( 0 , 60*fONE, 20*fONE , 0, pChar->Angle.y , 0 );
	POINT3D curPos;
	POINT3D desPos;
	curPos.x = pChar->pX+GeoResult_X;
	curPos.y = pChar->pY+GeoResult_Y;
	curPos.z = pChar->pZ+GeoResult_Z;
	
	desPos.x = pDestChar->pX;
	desPos.y = pDestChar->pY+24*fONE;
	desPos.z = pDestChar->pZ;
	
	StartEffectMonsterDest(curPos.x, curPos.y, curPos.z, desPos.x, desPos.y, desPos.z, MONSTER_SERQBUS_SHOT2 );

}


void AssaParticle_MonsterDeadZoneDead(smCHAR *pChar)
{
	POINT3D curPos;
  	curPos.x = pChar->pX;
	curPos.y = pChar->pY + 15000;
	curPos.z = pChar->pZ;
	
	g_NewParticleMgr.Start("IronMonsterDeadZoneDead", curPos);
}

void AssaParticle_MonsterHyperDead(smCHAR *pChar)
{
	POINT3D curPos;
 	curPos.x = pChar->pX;
	curPos.y = pChar->pY + 10000;
	curPos.z = pChar->pZ;

	g_NewParticleMgr.Start("IronMonsterHyperDead", curPos);
}


void AssaParticle_MonsterIronHit1(POINT3D *pPosi)
{
	g_NewParticleMgr.Start("HulkHit1", *pPosi);
}

void AssaParticle_MonsterIronMountainSkill1(smCHAR *pChar)
{
	//Mountain
 	POINT3D pos;
    GetMoveLocation( 0, 0, 130*fONE, 0, pChar->Angle.y-ANGLE_45/3, 0);
	pos.x = pChar->pX+GeoResult_X;
	pos.y = pChar->pY+1000;
	pos.z = pChar->pZ+GeoResult_Z;
	
 	g_NewParticleMgr.Start("IronMonsterMountainHit1", pos);

	
	GetMoveLocation( 0, 0, 130*fONE, 0, pChar->Angle.y+ANGLE_45/3, 0);
	pos.x = pChar->pX+GeoResult_X;
	pos.y = pChar->pY+1000;
	pos.z = pChar->pZ+GeoResult_Z;
	
 	g_NewParticleMgr.Start("IronMonsterMountainHit1", pos);
}

void AssaParticle_MonsterRunicGuardianHand(smCHAR *pChar)
{
	if(GetAssaCodeEffect(pChar, IRON_MONSTER_RUNIC_GUARDIAN_HAND1) == FALSE)
	{
    	int i = GetAssaEffect();
		cAssaTraceCharMeshParticle *traceMeshParticle = new cAssaTraceCharMeshParticle;
		traceMeshParticle->Start(pChar, "Bip01 L Hand", "IronMonsterRunicGuardianHand");
		cAssaEffect[i] = traceMeshParticle;
		cAssaEffect[i]->CODE = IRON_MONSTER_RUNIC_GUARDIAN_HAND1;

        i = GetAssaEffect();
		traceMeshParticle = new cAssaTraceCharMeshParticle;
		traceMeshParticle->Start(pChar, "Bip01 R Hand", "IronMonsterRunicGuardianHand");
		cAssaEffect[i] = traceMeshParticle;
		cAssaEffect[i]->CODE = IRON_MONSTER_RUNIC_GUARDIAN_HAND1;
	}
}

void AssaParticle_MonsterRunicGuardianShot(smCHAR *pChar, smCHAR *pDestChar)
{
	POINT3D curPos;
	POINT3D desPos;
   	curPos.x = pChar->pX;
	curPos.y = pChar->pY+30*fONE;
	curPos.z = pChar->pZ;

	desPos.x = pDestChar->pX;
	desPos.y = pDestChar->pY+24*fONE;
	desPos.z = pDestChar->pZ;

	int i = GetAssaEffect();
	AssaRunicGuardianShot *runicGuardianShot = new AssaRunicGuardianShot;
	runicGuardianShot->Start(&curPos, &desPos);
	cAssaEffect[i] = runicGuardianShot;
}

void AssaParticle_MonsterRunicGuardianBossShot(smCHAR *pChar, int blendType)
{
	
	
	D3DVECTOR velocity;
	POINT3D curPos;
	curPos.x = pChar->pX;
	curPos.y = pChar->pY + 30000;
 	curPos.z = pChar->pZ;
	
	//g_NewParticleMgr.Start("IronMonsterRunicGuardianBossSkill1", curPos);

	POINT3D desPos;
 	desPos.x = pChar->pX;
	desPos.y = pChar->pY;
	desPos.z = pChar->pZ;
	
     for(int index = 0; index < 5; index++)
	{
   		GetMoveLocation(0, 12*fONE, -5*fONE, 0, index*(ANGLE_45+ANGLE_45/2+GetRandomPos(0,ANGLE_45)), 0);
		
		velocity.x = (float)GeoResult_X/256.f;
		velocity.y = (float)GeoResult_Y/256.f;
		velocity.z = (float)GeoResult_Z/256.f;
		
 		if(index == 0)
		{
			desPos.x = pChar->pX;
			desPos.y = pChar->pY;
			desPos.z = pChar->pZ;
		}
		else
		{
 			desPos.x = pChar->pX + GeoResult_X*8;
			desPos.y = pChar->pY;
			desPos.z = pChar->pZ + GeoResult_Z*8;
		}


		int i = GetAssaEffect();
		AssaRunicGuardianBossShot *runicGuardianBossShot = new AssaRunicGuardianBossShot;
		runicGuardianBossShot->Start(&curPos, &desPos, &velocity, blendType);
		cAssaEffect[i] = runicGuardianBossShot;
		if(index == 0)
			runicGuardianBossShot->SetHitFlag(true);
	}
}

void AssaParticle_MonsterRunicGuardianBossLightShot(smCHAR **pCharList, int charCount)
{
	if(charCount > 1)
	{

 		for(int index = 0; index < charCount-1; index++)
		{
 			int i = GetAssaEffect();
 			AssaRunicGuardianLight *runicGuardianLight = new AssaRunicGuardianLight;
			//if(charCount < 2)
			//{
				runicGuardianLight->Start(pCharList[index],pCharList[index+1], 0, true, true);
			//}

			/*
			else
			{
				if(index == 0)
				{
					runicGuardianLight->Start(pCharList[index],pCharList[index+1], 0, true, false);
				}
				else if(index == (charCount - 2))
				{
					runicGuardianLight->Start(pCharList[index],pCharList[index+1], 0, false, true);
				}
				else
					runicGuardianLight->Start(pCharList[index],pCharList[index+1], 0, false, true);
			}
			*/

			cAssaEffect[i] = runicGuardianLight;
		}
	}
}


void AssaParticle_MonsterRunicGuardianBossWeapon(smCHAR *pChar)
{
  	if(GetAssaCodeEffect(pChar, IRON_MONSTER_RUNIC_GUARDIAN_HAND1) == FALSE)
	{
    	int i = GetAssaEffect();
		cAssaTraceCharMeshParticle *traceMeshParticle = new cAssaTraceCharMeshParticle;
		traceMeshParticle->Start(pChar, "Bip01 Effect", "IronMonsterRunicGuardianBossWeapon2");
        traceMeshParticle->SetScale(float(pChar->smCharInfo.wPlayClass[1])/256.f);

		cAssaEffect[i] = traceMeshParticle;
 		cAssaEffect[i]->CODE = IRON_MONSTER_RUNIC_GUARDIAN_HAND1;

		i = GetAssaEffect();
		traceMeshParticle = new cAssaTraceCharMeshParticle;
		traceMeshParticle->Start(pChar, "Bip01 Pelvis", "IronMonsterRunicGuardianBossJin2");
		cAssaEffect[i] = traceMeshParticle;
		traceMeshParticle->SetScale(float(pChar->smCharInfo.wPlayClass[1])/256.f);
		cAssaEffect[i]->CODE = IRON_MONSTER_RUNIC_GUARDIAN_HAND1;
	}
}


void AssaParticle_MonsterRunicGuardianBossHit(smCHAR *pCurChar, smCHAR *pDestChar, smCHAR **pCharList, int charCount)
{
	POINT3D curPos;
   	curPos.x = pCurChar->pX;
	curPos.y = pCurChar->pY+15000;
	curPos.z = pCurChar->pZ;

	D3DVECTOR velocity;
	memset(&velocity, 0, sizeof(velocity));

	int motionEvent = pCurChar->MotionEvent;
	int i  = 0;
	if(motionEvent == 1)
	{
 		i = GetAssaEffect(); 
		AssaRunicGuardianBossHit *guardianBossHit = new AssaRunicGuardianBossHit;
		
		velocity.x = 0.f;
		velocity.y = 0.f;
		velocity.z = 0.f;

		GetMoveLocation( 0, 0, 3*fONE+rand()%(2*fONE)-fONE, 0, pCurChar->Angle.y-ANGLE_45, 0);
		velocity.x += ((float)GeoResult_X*2.f/(float)fONE);
		velocity.z += ((float)GeoResult_Z*2.f/(float)fONE);
		if(charCount == 0)
			guardianBossHit->Start(&curPos, pDestChar, &velocity, 0);
		else
			guardianBossHit->Start(&curPos, pDestChar, &velocity, 0, pCharList, charCount);

		cAssaEffect[i] = guardianBossHit;
	}
	else
	{
		i = GetAssaEffect(); 
		AssaRunicGuardianBossHit *guardianBosHit = new AssaRunicGuardianBossHit;
	
		velocity.x = 0.f;
		velocity.y = 0.f;
		velocity.z = 0.f;

 		GetMoveLocation( 0, 0, 3*fONE+rand()%(2*fONE)-fONE, 0, pCurChar->Angle.y+ANGLE_45, 0);
		velocity.x += ((float)GeoResult_X*2.f/(float)fONE);
		velocity.z += ((float)GeoResult_Z*2.f/(float)fONE);
		if(charCount == 0)
			guardianBosHit->Start(&curPos, pDestChar, &velocity, 0);
		else
		 	guardianBosHit->Start(&curPos, pDestChar, &velocity, 0, pCharList, charCount);

		cAssaEffect[i] = guardianBosHit;
	}
}

void AssaParticle_MonsterRunicGuardianBossSkillHit(POINT3D *pDest)
{
	D3DVECTOR vector;
    vector.x = float(pDest->x - lpCurPlayer->pX)/fONE;
	vector.y = float(pDest->y - lpCurPlayer->pY)/fONE;
	vector.z = float(pDest->z - lpCurPlayer->pZ)/fONE;
	
	float length;
 	length = (float)sqrt(vector.x*vector.x+vector.y*vector.y+vector.z*vector.z);
	if(length == 0)
		length = 1;

	g_NewParticleMgr.Start("IronMonsterRunicGuardianSkillHit1", *pDest);
	SetDynLight( pDest->x, pDest->y, pDest->z,-150, -150, -255, 255, 300, 4);
 	EffectWaveCamera(int((1000.f - length)/25.f), 1);

}

void AssaParticle_TerrainFireHit(POINT3D *pCur)
{
    D3DVECTOR vector;
    vector.x = float(pCur->x - lpCurPlayer->pX)/fONE;
	vector.y = float(pCur->y - lpCurPlayer->pY)/fONE;
	vector.z = float(pCur->z - lpCurPlayer->pZ)/fONE;
	
	float length;
 	length = (float)sqrt(vector.x*vector.x+vector.y*vector.y+vector.z*vector.z);
	if(length == 0)
		length = 1;

    SetDynLight( pCur->x, pCur->y, pCur->z, 255, 200, 100, 255, 250, 2);
 	g_NewParticleMgr.Start("FireBigHit1", *pCur);
    EffectWaveCamera(int((500.f - length)/15.f), 2);
}

void AssaParticle_MonsterRunicGrardianBossSkill(smCHAR *pChar)
{
     int i = GetAssaEffect();
	AssaRunicGuardianBossSkill *runicGuardianBossSkill = new AssaRunicGuardianBossSkill;
	runicGuardianBossSkill->Start(pChar, 70+50);
	cAssaEffect[i] = runicGuardianBossSkill;
	
	POINT3D pos;
   	memcpy(&pos, &pChar->Posi, sizeof(POINT3D));
     pos.y += 26000;
	
	POINT3D curPos;
	POINT3D rand;
	//모이는 파티클..
     for(int index = 0; index < 100; index++)
	{
		//파티클이 나올 영역.. 
        rand.x = GetRandomPos(-80, 80);
		rand.y = GetRandomPos(-80, 80);
		rand.z = GetRandomPos(-80, 80);
		
		double length = sqrt((float)rand.x*(float)rand.x+(float)rand.y*(float)rand.y+(float)rand.z*(float)rand.z);
		
		if(length == 0)
			length = 1;
	 	int range = GetRandomPos(19000, 20000);
 		curPos.x = int(((float)rand.x/length)*(float)range+(float)pos.x);
		curPos.y = int(((float)rand.y/length)*(float)range+(float)pos.y);
		curPos.z = int(((float)rand.z/length)*(float)range+(float)pos.z);
		
	 	Assa = SetAssaEffect( 50, "Shield1.bmp", 0, &curPos, 0, 0);
		cAssaEffect[Assa]->FACE_TYPE = ASSAFACE_SCALE;
		
		cAssaEffect[Assa]->MOVECODE = ASSA_MOVE;
		

 		cAssaEffect[Assa]->Face.r = 255;
		cAssaEffect[Assa]->Face.g = 150;
		cAssaEffect[Assa]->Face.b = 50;
		cAssaEffect[Assa]->Face.Transparency = 255;
 		cAssaEffect[Assa]->Time = -(index);
  	 	cAssaEffect[Assa]->MoveSpeed.x = -int(float(rand.x)/length*600.f);
		cAssaEffect[Assa]->MoveSpeed.y = -int(float(rand.y)/length*600.f);
		cAssaEffect[Assa]->MoveSpeed.z = -int(float(rand.z)/length*600.f);
		
		//파티클의 넓이..
  		cAssaEffect[Assa]->Size.w = 600+GetRandomPos(0,500);

		//파티클의 길이..
		POINT3D inScale;
   		inScale.x = int((rand.x)/(float)length*14000.f);
		inScale.y = int((rand.y)/(float)length*14000.f);
		inScale.z = int((rand.z)/(float)length*14000.f);
		
		cAssaEffect[Assa]->ScalePosi.x = inScale.x;
		cAssaEffect[Assa]->ScalePosi.y = inScale.y;
		cAssaEffect[Assa]->ScalePosi.z = inScale.z;
	}
}



void AssaParticle_MonsterIronOmuHit1(smCHAR *pChar)
{
	POINT3D charPos;
     GetMoveLocation( 0, 0, 30*fONE, 0, pChar->Angle.y, 0);

    charPos.x = pChar->pX+GeoResult_X;
	charPos.y = pChar->pY+7000;
	charPos.z = pChar->pZ+GeoResult_Z;
	
	int num;
	num = g_NewParticleMgr.Start("IronMonsterOmuHit1", charPos);
	if(num != -1)
	{
		POINT3D angle;
  		angle.x = 0;
		angle.y = pChar->Angle.y;
		angle.z = 0;
		g_NewParticleMgr.SetAngle(num, angle);
	}
}



void AssaParticle_ShelltomShot(POINT3D *pPosi, POINT3D *pDest, int type)
{
   	int i = GetAssaEffect();
	AssaShelltomShot *shelltomShot = new AssaShelltomShot;
	shelltomShot->Start(pPosi, pDest, type);
	cAssaEffect[i] = shelltomShot;
}

void AssaParticle_BamShot(POINT3D *pPosi, POINT3D *pDest)
{
	int i = GetAssaEffect();
	AssaShelltomShot *shelltomShot = new AssaShelltomShot;
	shelltomShot->Start(pPosi, pDest,1);
	cAssaEffect[i] = shelltomShot;
}

void AssaParticle_MonsterExplision(POINT3D *pPosi)
{
	POINT3D pos;
 	memcpy(&pos, pPosi, sizeof(POINT3D));
	D3DVECTOR velocity;
 	D3DVECTOR gravity;
	
    
  	for(int index = 0; index < 3; index++)
  	{
		int i = GetAssaEffect(); 
		velocity.x = float(rand()%2400-1200);
		velocity.y = float(rand()%500+900);
		velocity.z = float(rand()%2400-1200);

  		gravity.x = 0;
 		gravity.y = float((rand()%500-670));
		gravity.z = 0;
		
		AssaSkill3BombShotExplision *explision = new AssaSkill3BombShotExplision;
		explision->Start(&pos, &velocity, &gravity, "IronMonsterBombHit2");
		cAssaEffect[i] = explision;
	}

	g_NewParticleMgr.Start("IronMonsterBombHit1", pos);
 	SetDynLight( pos.x, pos.y, pos.z, 255, 150, 50, 255, 250, 2);
}

void AssaMonsterHitTest(smCHAR *pChar)
{
	//AssaParticle_MonsterIronFist(pChar);
	AssaParticle_MonsterIronMorgonHit(pChar);
}
 
void AssaParticle_TerrainShowStart()
{
	int i = GetAssaEffect();
	AssaNatureSnow *natureSnow = new AssaNatureSnow;
	
	natureSnow->Start();
	cAssaEffect[i] = natureSnow;
}

void AssaParticle_ShelltomWeaponHit(POINT3D *pPosi)
{
    g_NewParticleMgr.Start("ShellTomWeaponHit", *pPosi);
	SetDynLight( pPosi->x,pPosi->y+22*fONE,pPosi->z , 100, 0, 0, 0, 250, 3);
} 


void AssaParticle_ShelltomWeapon(smCHAR *pChar, int time, int code)
{
	bool bFlag = false;
	for(int i = 0; i < cASSAEFFECT_MAX; i++)
	{
		if(cAssaEffect[i])
		{
			if(cAssaEffect[i]->CODE == SHELLTOM_WEAPON)
			{
				if(cAssaEffect[i]->pChar == pChar)
				{
					bFlag = true;
   					cAssaEffect[i]->Time = 0;
					cAssaEffect[i]->Max_Time = time;
				}
			}
		}
	}
	

	if(bFlag == false)
	{
   		int i = GetAssaEffect();
 		AssaShelltomUpWeapon2 *shelltomWeapon = new AssaShelltomUpWeapon2;
  		shelltomWeapon->Start(pChar, time, code);
		cAssaEffect[i] = shelltomWeapon;
	}

}


void AssaParticle_TerrainFire(POINT3D *pCur, POINT3D *pDest, int delay)
{
	int i = GetAssaEffect();
	AssaSkyParticle *skyParticle = new AssaSkyParticle;
	skyParticle->Start(pCur, pDest, delay);
	cAssaEffect[i] = skyParticle;
}

void SkillPhoenixShotFlame(smCHAR *pChar, POINT3D *curPos)
{
     GetMoveLocation( 0, 0, 30*fONE, 0, (ANGLE_45+pChar->Angle.y)&ANGCLIP, 0);
	//테스트용.
	D3DVECTOR  velocity;
	D3DVECTOR  gravity;
	
 	int i = GetAssaEffect();
		
	POINT3D partPos;
	partPos.x = curPos->x+GeoResult_X;
	partPos.y = curPos->y;
	partPos.z = curPos->z+GeoResult_Z;
	
	GetMoveLocation( 0, 0, 30*fONE, 0, (pChar->Angle.y)&ANGCLIP, 0);
	velocity.x = (float)(GeoResult_X)/8.f;
	velocity.y = 0.f;
	velocity.z = (float)(GeoResult_Z)/8.f;

  	gravity.x = 0.f;
 	gravity.y = 0.f;
	gravity.z = 0.f;
	
  	AssaSkill3BombShotExplision *explision = new AssaSkill3BombShotExplision;
	explision->Start(&partPos, &velocity, &gravity, "IronMonsterSadnessShot");
	cAssaEffect[i] = explision;
	
    GetMoveLocation( 0, 0, 30*fONE, 0, -((ANGLE_45+pChar->Angle.y)&ANGCLIP), 0);
	partPos.x = curPos->x+GeoResult_X;
	partPos.y = curPos->y;
	partPos.z = curPos->z+GeoResult_Z;

	i = GetAssaEffect();

  	GetMoveLocation( 0, 0, 30*fONE, 0, (pChar->Angle.y)&ANGCLIP, 0);
	velocity.x = (float)(GeoResult_X)/8.f;
	velocity.y = 0.f;
	velocity.z = (float)(GeoResult_Z)/8.f;

  	gravity.x = 0.f;
 	gravity.y = 0.f;
	gravity.z = 0.f;
	
	explision = new AssaSkill3BombShotExplision;
	explision->Start(&partPos, &velocity, &gravity, "IronMonsterSadnessShot");
	cAssaEffect[i] = explision;
}

BOOL	GetAssaUseEffect(smCHAR *pChar, int code)
{
	return GetAssaCodeEffect(pChar, code);
}


bool	GetAssaContinueSkill(int skillCode)
{
	for(int index = 0; index < 10; index++)
	{
		if(ContinueSkill[index].Flag)
		{
			if(ContinueSkill[index].CODE == skillCode)
				return true;
		}
	}
	
	return false;
}

/*
//같은 상태 이펙트를 쓰는지 찾는다..
BOOL GetAssaCodeEffect(smCHAR *pChar, int code)
{
    for(int i = 0; i < cASSAEFFECT_MAX; i++)
	{
		if(cAssaEffect[i])
		{
			if(cAssaEffect[i]->CODE == code)
			{
				if(cAssaEffect[i]->pChar == pChar)
				{
   					return TRUE;
				}
			}
	
		}
	}

	BOOL ret = FALSE;
	ret = HoEffectManager::GetInstance()->GetSkillCode();
	if(ret)
		return ret;
	return FALSE;
}
*/

//같은 상태 이펙트를 쓰는지 찾는 함수 찾았으면 시간을 더해 준다.
bool AddAssaCodeEffectTime(smCHAR *pChar, int code, int time)
{
	 for(int i = 0; i < cASSAEFFECT_MAX; i++)
	{
		if(cAssaEffect[i])
		{
			if(cAssaEffect[i]->CODE == code)
			{
				if(cAssaEffect[i]->pChar == pChar)
				{
   					cAssaEffect[i]->Time = 0;
					cAssaEffect[i]->Max_Time = time;
					return true;
				}
			}
		}
	}
	return false;
}

//같은 상태 이펙트를 쓰는지 찾는 함수 찾았으면 true, 아니면 false 그리고 찾았으면 이펙트를 없애게 만든다.
int StopAssaCodeEffect(smCHAR *pChar, int code, int setTime)
{
    for(int i = 0; i < cASSAEFFECT_MAX; i++)
	{
		if(cAssaEffect[i])
		{
			if(cAssaEffect[i]->CODE == code)
			{
				if(cAssaEffect[i]->pChar == pChar)
				{
   					if(setTime == 0)
					{
 			 			if(cAssaEffect[i]->Time < cAssaEffect[i]->Max_Time-70*4)
							cAssaEffect[i]->Time = cAssaEffect[i]->Max_Time-70*4;
					}
					else
					{
						if(cAssaEffect[i]->Time < cAssaEffect[i]->Max_Time-setTime)
							cAssaEffect[i]->Time = cAssaEffect[i]->Max_Time-setTime;
					}
				}
			}
	
		}
	}

	return true;
}



int GetAssaEffect()
{
	for(int i = 0; i < cASSAEFFECT_MAX; i++)
	{
		if(!cAssaEffect[i])
		{
			return i;
		}
	}
	return -1;
}


// 푹죽 터트릴시 20개의 4차원배열 즉 20 ^ 4 의 테이블을 정의하여 리턴시켜준다.
// 번호만 입력하면 그 번호에 맞는 배열을 찿아서 인코드 하여 리턴시켜준다.
DWORD GetTableNum(int n)
{
	int total = 0;
	DWORD	dwData=0;

	for (int a = 0; a <= 19; a++)
	{
		for (int b = 0; b <= 19; b++)
		{
			for(int c = 0; c <= 19; c++)
			{
				for(int d = 0; d <= 19; d++)
				{
					if( total != n )
					{
						total++;
					}
					else
					{
						dwData = Park_VarEncode(a,b,c,d);
						return dwData;
					}
				}
			}
		}
	}
	return -1;
}

// 4개의 값을 인코드한다.
DWORD Park_VarEncode(int a, int b, int c, int d)
{
	DWORD dw_Code1,dw_Code2,dw_Code3,dw_Code4;
	DWORD dwData = 0;

	dw_Code1 = dw_Code2 = dw_Code3 = dw_Code4 = 0;

	dw_Code1 = (a | 0x00000000) << 24;
	dw_Code2 = (b | 0x00000000) << 16;
	dw_Code3 = (c | 0x00000000) << 8;
	dw_Code4 = (d | 0x00000000) ;
	
	dwData = dw_Code1 | dw_Code2 | dw_Code3 | dw_Code4;

	return dwData;
}

// 인코드된 값을 다시 디코드 해준다.
void Park_VarDecode(DWORD dwData, int &nData1, int &nData2, int &nData3, int &nData4)
{
	nData1 = (dwData >> 24) & 0xff;
	nData2 = (dwData >> 16) & 0x00ff;
	nData3 = (dwData >> 8) & 0x0000ff;
	nData4 = dwData & 0x000000ff;
}