#include "..\\SinBaram\\SinLinkHeader.h"
#include "hoAssaParticleEffect.h"
#include "AssaParticle.h"
#include "..\\HoBaram\\HoNewParticleLinkHeader.h"
#include "..\\field.h"
#include "..\\HoBaram\\NewEffect\\HoEffect\\HoNewEffectFunction.h"

cASSAPARTSHOT::cASSAPARTSHOT()
{
	cAssaTrace->SetLength(50);
	Effect = NULL;
	PartNum = -1;
	TimeCount = 0;
	State = 0;
	ChainLightingFlag = false;
}

cASSAPARTSHOT::~cASSAPARTSHOT()
{

}

bool HoAddFaceTrace(smFACE2D *face, cASSATrace *assaTrace)
{
    if(face == NULL || assaTrace == NULL)
        return false;
	
	if(assaTrace->TraceList.size() < 2)
		return FALSE;
    
	POINT3D destPos;
	POINT3D currentPos;
	
	std::list<POINT3D>::iterator i;
	i = assaTrace->TraceList.begin();
    destPos.x = i->x;
    destPos.y = i->y;
    destPos.z = i->z;

    int texSizeCount = 0;
    int test = assaTrace->TraceList.size();
    float cellSize = 1.f/(assaTrace->TraceList.size()-1);
    ++i;
	POINT3D OldVertex[2];
    memset(OldVertex, 0, sizeof(OldVertex));
	
	int blendStep;
	int blendCount = 0;
	blendStep = face->Transparency/10;
	for (; i != assaTrace->TraceList.end();i++)
    {
		currentPos.x = i->x;
        currentPos.y = i->y;
        currentPos.z = i->z;
			
		if(GetCameraCoord(currentPos) == FALSE)
			    return FALSE;
		
		if(GetCameraCoord(destPos) == FALSE)
			    return FALSE;
		//GetCameraCoord(destPos);
		//GetCameraCoord(currentPos);
        smRENDVERTEX *v[4];
	    smRENDFACE	 *rf;

	    short		sColor[4];

        sColor[ SMC_R ] = face->r;
	    sColor[ SMC_G ] = face->g;
	    sColor[ SMC_B ] = face->b;
		if(blendCount < 10)
			sColor[ SMC_A ] = blendStep*blendCount;
		else
			sColor[ SMC_A ] = face->Transparency;

        smTEXRECT texRect;
        texRect.left   = 0;
        texRect.top    = (float)texSizeCount*cellSize;
        texRect.right  = 1;
        texRect.bottom = (float)(texSizeCount+1)*cellSize;


        int size;
        size = (int)face->width;
        //size = size << FLOATNS;
        size = size >> 1;

        float dx     = float(destPos.x - currentPos.x); //대상 까지의 x축의 기울기를 구한다.
		float dy     = float(destPos.y - currentPos.y); //대상 까지의 y축의 기울기를 구한다.
		float length = (float)sqrt(dx*dx+dy*dy);        //대상 까지의 거리를 구한다.

        if(length == 0)
            length = 1;

        dx = dx/length*size; //날아가는 파티클의 넓이..
		dy = dy/length*size;

        D3DVECTOR persp;
        persp.x = -dy;
		persp.y = +dx;
        persp.z = 0;
		
		if(texSizeCount == 0)
		{
			v[0] = smRender.AddRendVertex ( int(destPos.x-persp.x), int(destPos.y-persp.y), int(destPos.z), sColor );
			v[1] = smRender.AddRendVertex ( int(destPos.x+persp.x), int(destPos.y+persp.y), int(destPos.z), sColor );
		}
		else
		{
			v[0] = smRender.AddRendVertex ( OldVertex[0].x, OldVertex[0].y, OldVertex[0].z, sColor );
            v[1] = smRender.AddRendVertex ( OldVertex[1].x, OldVertex[1].y, OldVertex[1].z, sColor );
		}
        
		OldVertex[0].x = int(currentPos.x-persp.x);
        OldVertex[0].y = int(currentPos.y-persp.y);
        OldVertex[0].z = int(currentPos.z);

        OldVertex[1].x = int(currentPos.x+persp.x);
        OldVertex[1].y = int(currentPos.y+persp.y);
        OldVertex[1].z = int(currentPos.z);

        destPos.x = int(i->x);
        destPos.y = int(i->y);
        destPos.z = int(i->z);

        v[2] = smRender.AddRendVertex ( int(currentPos.x-persp.x), int(currentPos.y-persp.y), int(currentPos.z), sColor );
        v[3] = smRender.AddRendVertex ( int(currentPos.x+persp.x), int(currentPos.y+persp.y), int(currentPos.z), sColor );

        //첫번째 폴리곤 ( 1 - 2 - 3 );
        rf = &smRender.RendFace[ smRender.nRendFace++ ];
        rf->lpRendVertex[0] = v[0];
        rf->lpRendVertex[1] = v[1];
        rf->lpRendVertex[2] = v[2];
		rf->Matrial    = face->MatNum;		//메트리얼 복사
        rf->ClipStatus = v[0]->ClipStatus|v[1]->ClipStatus|v[2]->ClipStatus;
        rf->lpTexLink  = 0;

        //텍스쳐 연결
        smRender.AddRendTempTexLink( rf , 0,	texRect.left , texRect.top,
                            texRect.right , texRect.top,
                            texRect.left , texRect.bottom );


        //두번째 폴리곤 ( 2 - 4 - 3 );
        rf = &smRender.RendFace[ smRender.nRendFace++ ];
        rf->lpRendVertex[0] = v[1];
        rf->lpRendVertex[1] = v[3];
        rf->lpRendVertex[2] = v[2];
        rf->Matrial    = face->MatNum;		//메트리얼 복사
        rf->ClipStatus = v[1]->ClipStatus|v[2]->ClipStatus|v[3]->ClipStatus;
        rf->lpTexLink  = 0;

        //텍스쳐 연결
        smRender.AddRendTempTexLink( rf , 0, texRect.right , texRect.top,
                                                texRect.right , texRect.bottom,
                                                texRect.left  , texRect.bottom );

        texSizeCount++;
		blendCount++;
	}
	return true;
}


void cASSAPARTSHOT::Draw(POINT3D *pPosi , POINT3D *pAngle)
{

	if(!Face.MatNum)return;
		
		// Turn on Z-buffering
		//lpD3DDevice->SetRenderState( D3DRENDERSTATE_ZENABLE, TRUE ); (문제가 생기는거 같아서 빼줬다)
		lpD3DDevice->SetRenderState( D3DRENDERSTATE_ZWRITEENABLE, FALSE );
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

		lpD3DDevice->SetTextureStageState( 0, D3DTSS_MINFILTER, D3DTFN_LINEAR  );
		lpD3DDevice->SetTextureStageState( 0, D3DTSS_MAGFILTER, D3DTFN_LINEAR  );


		smRender.Init(); //데이터 초기화 

		smMaterialGroup->smMaterial[Face.MatNum].TwoSide = TRUE; //카메라 회전과 상관없이 보이기 
			
		smRender.SetMaterialGroup(smMaterialGroup);  //읽어드린 메트리얼을 연결 
			

		//현재 찍을 Frame을 세팅한다..
		smMaterialGroup->SetMaterialAnimFrame( &smMaterial[Face.MatNum], 2);
		//Camera각도 입력.
		smRender.SetCameraPosi(pPosi->x, pPosi->y, pPosi->z,pAngle->x,pAngle->y,pAngle->z);
		
		if(EffectType == ASSA_SHOT_FIRE)
		{
			if(!AssaAddFace2DLine(&Face, &ScalePosi))
				return;
		}
		else if(EffectType == ASSA_SHOT_LASER || EffectType == ASSA_SHOT_LASER2 || EffectType == ASSA_SHOT_LASER3)
		{
			//if(!AssaAddFace2DLine(&Face, &DesPosi))
				//return;
			if(!HoAddFaceTrace(&Face, cAssaTrace))
				return;
		}
		else if(EffectType == ASSA_SHOT_SPARK || EffectType == ASSA_SHOT_SPARK1 || EffectType==ASSA_SHOT_PARKSPARK)
		{
			if(!AssaAddFaceTrace(&Face,cAssaTrace))
				return;
		}
		else if(EffectType == SKILL_TWIST_JAVELIN)
		{
			if(!AssaAddFaceTrace(&Face,cAssaTrace))
				return;
		}
		smRender.SetClipStates(SMCLIP_LEFT | SMCLIP_RIGHT | SMCLIP_TOP | SMCLIP_BOTTOM);
		smRender.ClipRendFace();
		smRender.GeomVertex2D(FALSE);
		smRender.RenderD3D();

		lpD3DDevice->SetRenderState( D3DRENDERSTATE_ZWRITEENABLE, TRUE );
}


void cASSAPARTSHOT::Main()
{

	if(EffectType == SKILL_TWIST_JAVELIN)
	{
		if(Time >= Max_Time)
			return;
		
		POINT3D pos;
 		pos.x = int(Posi.x);
		pos.y = int(Posi.y);
		pos.z = int(Posi.z);

		D3DVECTOR term;
		term.x = float((DesPosi.x - pos.x)/ fONE);
		term.y = float((DesPosi.y - pos.y)/ fONE);
		term.z = float((DesPosi.z - pos.z)/ fONE);
		
		float length = term.x*term.x + term.y*term.y + term.z*term.z;
		length = (float)sqrt(length);
		
		if(length < 5)
		{
			if(PartNum != -1) 
			{
				g_NewParticleMgr.SetStop(PartNum);
				Time = Max_Time;

				if(Effect != NULL)
					Effect->Time = Max_Time-20;
				
				int num = g_NewParticleMgr.Start("TwistJavelin2", Posi);
				g_NewParticleMgr.SetAngle(num, Angle);
			}
		}

		
		Posi.x += (int)Velocity.x;
		Posi.y += (int)Velocity.y;
		Posi.z += (int)Velocity.z;
		

		if(Effect != NULL)
		{
			Effect->Posi.x = Posi.x;
			Effect->Posi.y = Posi.y;
			Effect->Posi.z = Posi.z;
		}
		
		D3DVECTOR partPos;
		partPos.x = (float)Posi.x;
		partPos.y = (float)Posi.y;
		partPos.z = (float)Posi.z;

		if(PartNum != -1)
			g_NewParticleMgr.SetPos(PartNum, partPos);
	}
	else if(EffectType == SKILL_FIRE_JAVELIN)
	{
		int step = 6*fONE;				//창이나 기타 던지는 무기
		GetMoveLocation( 0, 0, step, ShootingAngle.x, ShootingAngle.y, ShootingAngle.z );
		Posi.x += GeoResult_X;
		Posi.y += GeoResult_Y;
		Posi.z += GeoResult_Z;
		
		POINT3D pos;
 		pos.x = int(Posi.x);
		pos.y = int(Posi.y);
		pos.z = int(Posi.z);

		D3DVECTOR term;
		term.x = float((DesPosi.x - pos.x)/ fONE);
		term.y = float((DesPosi.y - pos.y)/ fONE);
		term.z = float((DesPosi.z - pos.z)/ fONE);
		
		float length = term.x*term.x + term.y*term.y + term.z*term.z;
		length = (float)sqrt(length);
		
 		if(length < 15)
		{
   			if(PartNum != -1)
			{
				g_NewParticleMgr.SetStop(PartNum);
				
				Time = Max_Time;
				AssaParticle_FireHit1(&Posi);
			}
		}

   	 	//Posi.x += int(Velocity.x);
		//Posi.y += int(Velocity.y);
		//Posi.z += int(Velocity.z);
		
		Face.x = Posi.x;
		Face.y = Posi.y;
		Face.z = Posi.z;

		D3DVECTOR partPos;
		partPos.x = (float)Posi.x;
		partPos.y = (float)Posi.y;
		partPos.z = (float)Posi.z;

		if(PartNum != -1) 
			g_NewParticleMgr.SetPos(PartNum, partPos);

	}
	else if(EffectType == ASSA_SHOT_FIRE || EffectType == ASSA_SHOT_BIGFIRE1)
	{
		if(Time >= Max_Time)
			return; 
		
		POINT3D pos;
 		pos.x = int(Posi.x);
		pos.y = int(Posi.y);
		pos.z = int(Posi.z);
		
		/*
		int step = 6*fONE;				//창이나 기타 던지는 무기
		GetMoveLocation( 0, 0, step, ShootingAngle.x, ShootingAngle.y, ShootingAngle.z );
		Posi.x += GeoResult_X;
		Posi.y += GeoResult_Y;
		Posi.z += GeoResult_Z;
		*/
		

		D3DVECTOR term;
		term.x = float((DesPosi.x - pos.x)/ fONE);
		term.y = float((DesPosi.y - pos.y)/ fONE);
		term.z = float((DesPosi.z - pos.z)/ fONE);
		
		float length = term.x*term.x + term.y*term.y + term.z*term.z;
		length = (float)sqrt(length);
		
		if(length < 5)
		{
			if(PartNum != -1)
			{
				g_NewParticleMgr.SetStop(PartNum);
				
				Time = Max_Time;
				if(EffectType == ASSA_SHOT_FIRE)
					AssaParticle_FireHit1(&Posi);
				else if(EffectType == ASSA_SHOT_BIGFIRE1)
				{
					Posi.y += 1000;
					AssaParticle_BigFireHit1(&Posi);
				}
			}
		}

   	 	Posi.x += int(Velocity.x);
		Posi.y += int(Velocity.y);
		Posi.z += int(Velocity.z);
		
		Face.x = Posi.x;
		Face.y = Posi.y;
		Face.z = Posi.z;

		D3DVECTOR partPos;
		partPos.x = (float)Posi.x;
		partPos.y = (float)Posi.y;
		partPos.z = (float)Posi.z;

		if(PartNum != -1) 
			g_NewParticleMgr.SetPos(PartNum, partPos);
	}
	else if(EffectType == ASSA_SHOT_ICE)
	{
		if(Time >= Max_Time)
			return;

		POINT3D pos;
 		pos.x = int(Posi.x);
		pos.y = int(Posi.y);
		pos.z = int(Posi.z);

		D3DVECTOR term;
		term.x = float((DesPosi.x - pos.x)/ fONE);
		term.y = float((DesPosi.y - pos.y)/ fONE);
		term.z = float((DesPosi.z - pos.z)/ fONE);
		
		float length = term.x*term.x + term.y*term.y + term.z*term.z;
		length = (float)sqrt(length);
		
		if(length < 5)
		{
			if(PartNum != -1)
			{
				g_NewParticleMgr.SetStop(PartNum);
				
				Time = Max_Time;
				AssaParticle_IceHit1(&Posi);
			}
		}

   	 	Posi.x += int(Velocity.x);
		Posi.y += int(Velocity.y);
		Posi.z += int(Velocity.z);
		
		Face.x = Posi.x;
		Face.y = Posi.y;
		Face.z = Posi.z;

		D3DVECTOR partPos;
		partPos.x = (float)Posi.x;
		partPos.y = (float)Posi.y;
		partPos.z = (float)Posi.z;

		if(PartNum != -1) 
			g_NewParticleMgr.SetPos(PartNum, partPos);
	}
	else if(EffectType == ASSA_SHOT_WATER && Effect != NULL)
	{
		if(Effect->Time >= Effect->Max_Time)
		{
			Time = Max_Time;

			g_NewParticleMgr.SetStop(PartNum);
 			Effect = NULL;
			return;
		}
		
		int mapY1, mapY2;
		if(smGameStage[0])
		{
			mapY1 = smGameStage[0]->GetHeight(Effect->Posi.x, Effect->Posi.z);
		}
		if(smGameStage[1])
		{
			mapY2 = smGameStage[1]->GetHeight(Effect->Posi.x, Effect->Posi.z);
			if (mapY1 < mapY2) mapY1 = mapY2;
		}

		
		Effect->Posi.y = mapY1-500;
		Effect->Posi.x += (int)Velocity.x;
		Effect->Posi.z += (int)Velocity.z;
		
		D3DVECTOR pos;
		pos.x = (float)Effect->Posi.x;
		pos.y = (float)Effect->Posi.y;
		pos.z = (float)Effect->Posi.z;
		
		g_NewParticleMgr.SetAttachPos(PartNum, pos);
		
		//충돌 체크 부분..
		D3DVECTOR term;
		term.x = float(DesPosi.x - Effect->Posi.x)/ (float)fONE;
		term.z = float(DesPosi.z - Effect->Posi.z)/ (float)fONE;
		
		float length = term.x*term.x + term.z*term.z;
		length = (float)sqrt(length);
		if(State == 0 && length < 40)
		{
			POINT3D hitPos;
			hitPos.x = DesPosi.x;
			hitPos.y = Effect->Posi.y+500;
			hitPos.z = DesPosi.z;
			AssaParticle_ColumnOfWaterHit(&hitPos);
			State = 1;
		}

		if(State == 1)
		{
			if(length < 10)
			{
				Velocity.x = Velocity.x * 0.15f;
				Velocity.z = Velocity.z * 0.15f;
				State = 2;
			}
		}
		
	}
	else if(EffectType == ASSA_SHOT_LASER || EffectType == ASSA_SHOT_LASER2 || EffectType == ASSA_SHOT_LASER3)
	{
		
 		if(State == 0)
		{
    		if(WidthStep < 00)
				WidthStep += 200;
			else
				State = 1;
		}
		if(WidthStep > 0 && State == 1)
			WidthStep -= 40;
		
 		if(WidthStep < 0)
			WidthStep = 0;
	}
	else if(EffectType == ASSA_SHOT_SPARK || EffectType == ASSA_SHOT_SPARK1 || EffectType==ASSA_SHOT_PARKSPARK)
	{
	 	if(Time >= Max_Time)
			return;

		POINT3D pos;
 		pos.x = int(Posi.x);
		pos.y = int(Posi.y);
		pos.z = int(Posi.z);

		D3DVECTOR term;
		term.x = float((DesPosi.x - pos.x)/ fONE);
		term.y = float((DesPosi.y - pos.y)/ fONE);
		term.z = float((DesPosi.z - pos.z)/ fONE);
		
		float length = term.x*term.x + term.y*term.y + term.z*term.z;
		length = (float)sqrt(length);
		
		cAssaTrace->AddData(&Posi);		
 		if(length < 10 && State == 0) //충돌을 했을 경우..
		{
 			State = 1;
			if(EffectType == ASSA_SHOT_SPARK)
			{
				if(ChainLightingFlag == false)
					AssaParticle_Sprak(&DesPosi);
				else
				{
					if(CharCount >= 2)
						CelestialChainLighting(CharList, CharCount);
					AssaParticle_Sprak(&DesPosi);
 				}
			}

			else if(EffectType == ASSA_SHOT_SPARK1)
			{
				DesPosi.y -= 1500;
				AssaParticle_Sprak1(&DesPosi);
			}
			else if(EffectType==ASSA_SHOT_PARKSPARK)
			{
				ParkAssaParticle_Sprak1(&DesPosi);
			}
		}
		
 		D3DVECTOR velocity;
		velocity.x = term.x/length;
		velocity.y = term.y/length;
		velocity.z = term.z/length;
		

 		Velocity.x += velocity.x;
		Velocity.y += velocity.y;
		Velocity.z += velocity.z;
		
		if(length < 80)
		{
			Velocity.x = Velocity.x * 0.8f;
			Velocity.y = Velocity.y * 0.8f;
			Velocity.z = Velocity.z * 0.8f;
		}
		
		Posi.x += int(Velocity.x * fONE);
		Posi.y += int(Velocity.y * fONE);
		Posi.z += int(Velocity.z * fONE);
	}

	Face.width = WidthStep;
	TimeCount++;
}

void cASSAPARTSHOT::Start(POINT3D *pCurPosi, POINT3D *pDestPosi, cASSAEFFECT *pEffect,int type)
{
 	if(pEffect == NULL)
		return;
	
	EffectType = type;
	if(EffectType == ASSA_SHOT_WATER)
	{
		Effect =pEffect;
		Posi = *pCurPosi;
		DesPosi = *pDestPosi;
		PartNum = g_NewParticleMgr.Start("ColumnOfWater1", Posi);

		Max_Time = 500;
		memset(&Velocity, 0, sizeof(Velocity));

		D3DVECTOR term;
		term.x = float(DesPosi.x - Posi.x)/(float)fONE;
		term.y = float(DesPosi.y - Posi.y)/(float)fONE;
		term.z = float(DesPosi.z - Posi.z)/(float)fONE;
		
		double length = sqrt(term.x*term.x+term.z*term.z);
		if(length == 0)
			length = 1;

   		Velocity.x = term.x/(float)length*900.f;
		Velocity.z = term.z/(float)length*900.f;

		Effect->Posi.x = Posi.x+(DesPosi.x-Posi.x)/3;
		Effect->Posi.z = Posi.z+(DesPosi.z-Posi.z)/3;
	}
	else if(EffectType == SKILL_TWIST_JAVELIN)
	{
		Effect = pEffect;
		Posi    = *pCurPosi;
		DesPosi = *pDestPosi;

		Max_Time = 200;
		PartNum = g_NewParticleMgr.Start("TwistJavelin1", Posi);
		
		GetRadian3D( Posi.x, Posi.y, Posi.z, DesPosi.x, DesPosi.y, DesPosi.z);
		Angle.x = GeoResult_X;
		Angle.y = GeoResult_Y;
		Angle.z = 0;
		
		D3DVECTOR term;
		term.x = float((DesPosi.x - Posi.x)/ fONE);
		term.y = float((DesPosi.y - Posi.y)/ fONE);
		term.z = float((DesPosi.z - Posi.z)/ fONE);
		
		float length = term.x*term.x + term.y*term.y + term.z*term.z;
		length = (float)sqrt(length);
		
		memset(&Velocity, 0, sizeof(Velocity));
		Velocity.x = term.x/length*1800;
		Velocity.y = term.y/length*1800;
		Velocity.z = term.z/length*1800;
		g_NewParticleMgr.SetAngle(PartNum, Angle);
	}
}

void cASSAPARTSHOT::SetChainLighting(smCHAR **pCharList, int count)
{
 	memcpy(CharList, pCharList, 4*count);
	CharCount = count;
	ChainLightingFlag = true;
}

void cASSAPARTSHOT::Start(smCHAR *pChar, POINT3D *pDestPosi, int type)
{
	Time = 0;
	Face.width = 4000;
	Face.x = Posi.x;
	Face.y = Posi.y;
	Face.z = Posi.z;

	Face.TexRect.left = 0;
	Face.TexRect.top  = 0;
	Face.TexRect.right = 1; 
	Face.TexRect.bottom = 1; 
	
	TimeCount = 0;
	EffectType = type;
	if(EffectType == SKILL_FIRE_JAVELIN)
	{
  		DesPosi = *pDestPosi;
		DesPosi.y -= 28 * fONE;
  		GetRadian3D( pChar->pX,pChar->pY, pChar->pZ, DesPosi.x, DesPosi.y, DesPosi.z);
		int angX = GeoResult_X;
		int angY = GeoResult_Y;

		Posi.x = pChar->pX;
		Posi.y = pChar->pY+34*fONE;
		Posi.z = pChar->pZ;
		
		ShootingAngle.x = angX;
		ShootingAngle.y = angY;
		ShootingAngle.z = 0;

		DesPosi.y += 28 * fONE;

		Face.r = 200;
		Face.g = 200;
		Face.b = 200;
		Face.Transparency = 200;

  		WidthStep = 3000;
		State = 0;
		int *matNum = (int *)AssaSearchRes("fire-b3.bmp", SMMAT_BLEND_INVSHADOW);
		if(matNum == NULL)
			return;
		Face.MatNum = *matNum;
		Max_Time = 150;

		D3DVECTOR term;
		term.x = float(DesPosi.x - Posi.x);
		term.y = float(DesPosi.y - Posi.y);
		term.z = float(DesPosi.z - Posi.z);
		
		double length = sqrt(term.x*term.x+term.y*term.y+term.z*term.z);
		if(length == 0)
			length = 0;

		ScalePosi.x = -int(term.x/length*8000.f);
		ScalePosi.y = -int(term.y/length*8000.f);
		ScalePosi.z = -int(term.z/length*8000.f);

		memset(&Velocity, 0, sizeof(Velocity));
		Velocity.x = float(term.x/length*1800.f);
		Velocity.y = float(term.y/length*1800.f);
		Velocity.z = float(term.z/length*1800.f);
		
 		PartNum = g_NewParticleMgr.Start("FireJavelinFire", Posi);
		///EffectType = ASSA_SHOT_FIRE;
	}
}

void cASSAPARTSHOT::Start(POINT3D *pCurPosi, POINT3D *pDestPosi, int type)
{
    Posi = *pCurPosi;
	DesPosi = *pDestPosi;
	
	Time = 0;
	Face.width = 4000;
	
	Face.x = Posi.x;
	Face.y = Posi.y;
	Face.z = Posi.z;

	Face.TexRect.left = 0;
	Face.TexRect.top  = 0;
	Face.TexRect.right = 1; 
	Face.TexRect.bottom = 1; 
	
	TimeCount = 0;
	EffectType = type;
 	if(EffectType == ASSA_SHOT_LASER || EffectType == ASSA_SHOT_LASER2 || EffectType == ASSA_SHOT_LASER3)
	{
 		Face.r = 200;
		Face.g = 200;
		Face.b = 200;
		Face.Transparency = 255;

     	Face.width = 1000;
 		State = 0;
		WidthStep = 3000;
		int *matNum;
		if(EffectType == ASSA_SHOT_LASER )
		{
	 		matNum = (int *)AssaSearchRes("Ray01.bmp", SMMAT_BLEND_LAMP);
		}
		else if(EffectType == ASSA_SHOT_LASER2 || EffectType == ASSA_SHOT_LASER3)
		{
			matNum = (int *)AssaSearchRes("Ray02.bmp", SMMAT_BLEND_LAMP);
		}
		if(matNum == NULL)
			return;
		Face.MatNum = *matNum;
		Max_Time = 60;

		if(EffectType == ASSA_SHOT_LASER3)
		{
			Max_Time = 120;
		}
		
		D3DVECTOR term;
		term.x = float(DesPosi.x - Posi.x);
		term.y = float(DesPosi.y - Posi.y);
		term.z = float(DesPosi.z - Posi.z);
		
		double length = sqrt(term.x*term.x+term.y*term.y+term.z*term.z);
		if(length == 0)
			length = 0;
		
		Velocity.x = term.x/(float)length*(150000.f/20.f);
		Velocity.y = term.y/(float)length*(150000.f/20.f);
		Velocity.z = term.z/(float)length*(150000.f/20.f);

		POINT3D pos;
		pos.x = Posi.x;
		pos.y = Posi.y;
		pos.z = Posi.z;
		cAssaTrace->AddData(&pos);

 		int index; for( index = 0; index < 20; index++)
		{
			pos.x += (int)Velocity.x;
			pos.y += (int)Velocity.y;
			pos.z += (int)Velocity.z;
			cAssaTrace->AddData(&pos);
		}

	}
	else if(EffectType == ASSA_SHOT_SPARK || EffectType == ASSA_SHOT_PARKSPARK)
	{
  		Face.r = 255;
 		Face.g = 255;
		Face.b = 255;
		Face.Transparency = 255;
		
 		WidthStep = 8000;
		State = 0;
		memset(&Velocity, 0, sizeof(Velocity));
		int *matNum;

		if(EffectType==ASSA_SHOT_SPARK)
		{
			matNum = (int *)AssaSearchRes("spark01_01.bmp", SMMAT_BLEND_LAMP);
		}
		else if(EffectType==ASSA_SHOT_PARKSPARK)
		{
			matNum = (int *)AssaSearchRes("spark04_01.bmp", SMMAT_BLEND_LAMP);
		}
		
		if(matNum == NULL)
			return;
		Face.MatNum = *matNum;
		Max_Time = 150;

	}
	else if(EffectType == ASSA_SHOT_SPARK1)
	{
 		Face.r = 255;
		Face.g = 255;
		Face.b = 255;
		Face.Transparency = 255;
		
 		WidthStep = 8000;
		State = 0;
		memset(&Velocity, 0, sizeof(Velocity));
		int *matNum = (int *)AssaSearchRes("light_G003.bmp", SMMAT_BLEND_LAMP);
		if(matNum == NULL)
			return;
		Face.MatNum = *matNum;
 		Max_Time = 150;
		cAssaTrace->SetLength(30);
	}
	else if(EffectType == SKILL_FIRE_JAVELIN)
	{
		Posi = *pCurPosi;
 		DesPosi = *pDestPosi;

 		DesPosi.y += 4 * fONE;
   		GetRadian3D( pCurPosi->x,pCurPosi->y, pCurPosi->z, DesPosi.x, DesPosi.y, DesPosi.z);
		int angX = GeoResult_X;
		int angY = GeoResult_Y;

		//Posi.x = pChar->pX;
		//Posi.y = pChar->pY+34*fONE;
		//Posi.z = pChar->pZ;
		
		ShootingAngle.x = angX;
		ShootingAngle.y = angY;
		ShootingAngle.z = 0;

		//DesPosi.y += 28 * fONE;

		Face.r = 200;
		Face.g = 200;
		Face.b = 200;
		Face.Transparency = 200;

  		WidthStep = 3000;
		State = 0;
		int *matNum = (int *)AssaSearchRes("fire-b3.bmp", SMMAT_BLEND_INVSHADOW);
		if(matNum == NULL)
			return;
		Face.MatNum = *matNum;
		Max_Time = 150;
		
   		int step = 12*fONE;				//창이나 기타 던지는 무기
 		GetMoveLocation( 0, 0, step, ShootingAngle.x, ShootingAngle.y, ShootingAngle.z );
		Posi.x += GeoResult_X;
		Posi.y += GeoResult_Y;
		Posi.z += GeoResult_Z;

		/*
		D3DVECTOR term;
		term.x = float(DesPosi.x - Posi.x);
		term.y = float(DesPosi.y - Posi.y);
		term.z = float(DesPosi.z - Posi.z);
		
		double length = sqrt(term.x*term.x+term.y*term.y+term.z*term.z);
		if(length == 0)
			length = 0;

		ScalePosi.x = -int(term.x/length*8000.f);
		ScalePosi.y = -int(term.y/length*8000.f);
		ScalePosi.z = -int(term.z/length*8000.f);

		memset(&Velocity, 0, sizeof(Velocity));
		Velocity.x = float(term.x/length*1800.f);
		Velocity.y = float(term.y/length*1800.f);
		Velocity.z = float(term.z/length*1800.f);
		*/
		
 		PartNum = g_NewParticleMgr.Start("FireJavelinFire", Posi);
	}
	else if(EffectType == ASSA_SHOT_FIRE || EffectType == ASSA_SHOT_BIGFIRE1)
	{
		Face.r = 200;
		Face.g = 200;
		Face.b = 200;
		Face.Transparency = 200;

  		WidthStep = 3000;
		State = 0;
		int *matNum = (int *)AssaSearchRes("fire-b3.bmp", SMMAT_BLEND_INVSHADOW);
		if(matNum == NULL)
			return;
		Face.MatNum = *matNum;
		Max_Time = 150;

		D3DVECTOR term;
		term.x = float(DesPosi.x - Posi.x);
		term.y = float(DesPosi.y - Posi.y);
		term.z = float(DesPosi.z - Posi.z);
		
		double length = sqrt(term.x*term.x+term.y*term.y+term.z*term.z);
		if(length == 0)
			length = 0;

		ScalePosi.x = -int(term.x/length*8000.f);
		ScalePosi.y = -int(term.y/length*8000.f);
		ScalePosi.z = -int(term.z/length*8000.f);

		memset(&Velocity, 0, sizeof(Velocity));
		Velocity.x = float(term.x/length*1800.f);
		Velocity.y = float(term.y/length*1800.f);
		Velocity.z = float(term.z/length*1800.f);
		
		if(EffectType == ASSA_SHOT_FIRE)
			PartNum = g_NewParticleMgr.Start("FireShot1", Posi);
		else if(EffectType == ASSA_SHOT_BIGFIRE1)
			PartNum = g_NewParticleMgr.Start("FireBigShot1", Posi);

	}
	else if(EffectType == ASSA_SHOT_ICE)
	{
		
		State = 0;
		Max_Time = 150;
		D3DVECTOR term;
		term.x = float(DesPosi.x - Posi.x);
		term.y = float(DesPosi.y - Posi.y);
		term.z = float(DesPosi.z - Posi.z);
		
		double length = sqrt(term.x*term.x+term.y*term.y+term.z*term.z);
		if(length == 0)
			length = 0;

		ScalePosi.x = -int(term.x/length*8000.f);
		ScalePosi.y = -int(term.y/length*8000.f);
		ScalePosi.z = -int(term.z/length*8000.f);

		memset(&Velocity, 0, sizeof(Velocity));
		Velocity.x = float(term.x/length*1800.f);
		Velocity.y = float(term.y/length*1800.f);
		Velocity.z = float(term.z/length*1800.f);

		PartNum = g_NewParticleMgr.Start("IceShot1", Posi);
	}
}

cASSAPARTSPARK::cASSAPARTSPARK()
{
	memset(&Velocity, 0, sizeof(Velocity));
	//cAssaTrace->SetLength(30);
}

cASSAPARTSPARK::~cASSAPARTSPARK()
{

}
void cASSAPARTSPARK::Start(POINT3D *pCurPosi, char *fileName)
{
	
	Posi = *pCurPosi;
	DesPosi = *pCurPosi;
	Max_Time = 150;
	memset(&Velocity, 0, sizeof(Velocity));
	
  	DesPosi.y -= 5000;
 	DesPosi.x += GetRandomPos(-1300,1300);
	DesPosi.z += GetRandomPos(-1300,1300);

	int flag = GetRandomPos(0,1);
	if(flag == 0)
		Velocity.x = (float)GetRandomPos(10,18);
	else if(flag == 1)
		Velocity.x = (float)GetRandomPos(-10,-18);

	Velocity.y = (float)GetRandomPos( 6,8);
	flag = GetRandomPos(0,1);
	if(flag == 0)
		Velocity.z = (float)GetRandomPos(10,18);
	else if(flag == 1)
		Velocity.z = (float)GetRandomPos(-10,-18);

 	Face.x = Posi.x;
	Face.y = Posi.y;
	Face.z = Posi.z;

	Face.TexRect.left = 0;
	Face.TexRect.top  = 0;
	Face.TexRect.right = 1; 
	Face.TexRect.bottom = 1;

	Face.r = 255;
	Face.g = 255;
	Face.b = 255;
	Face.Transparency = 255;
	
 	Face.width = 4000;
	int *matNum = (int *)AssaSearchRes(fileName, SMMAT_BLEND_LAMP);
	if(matNum == NULL)
		return;
	Face.MatNum = *matNum;	
}

void cASSAPARTSPARK::Start(POINT3D *pCurPosi)
{
  	Posi = *pCurPosi;
	DesPosi = *pCurPosi;
	Max_Time = 150;
	memset(&Velocity, 0, sizeof(Velocity));
	
 	DesPosi.y -= 5000;
	DesPosi.x += GetRandomPos(-1000,1000);
	DesPosi.z += GetRandomPos(-1000,1000);

	int flag = GetRandomPos(0,1);
	if(flag == 0)
		Velocity.x = (float)GetRandomPos(10,18);
	else if(flag == 1)
		Velocity.x = (float)GetRandomPos(-10,-18);

	Velocity.y = (float)GetRandomPos( 6,8);
	flag = GetRandomPos(0,1);
	if(flag == 0)
		Velocity.z = (float)GetRandomPos(10,18);
	else if(flag == 1)
		Velocity.z = (float)GetRandomPos(-10,-18);

 	Face.x = Posi.x;
	Face.y = Posi.y;
	Face.z = Posi.z;

	Face.TexRect.left = 0;
	Face.TexRect.top  = 0;
	Face.TexRect.right = 1; 
	Face.TexRect.bottom = 1;

	Face.r = 255;
	Face.g = 255;
	Face.b = 255;
	Face.Transparency = 255;
	
	Face.width = 4000;
	int *matNum = (int *)AssaSearchRes("spark01_01.bmp", SMMAT_BLEND_LAMP);
	/*
	if(EffectType==ASSA_SHOT_SPARK)
	{
		int *matNum = (int *)AssaSearchRes("spark01_01.bmp", SMMAT_BLEND_LAMP);
	}
	else if(EffectType==ASSA_SHOT_PARKSPARK)
	{
		int *matNum = (int *)AssaSearchRes("spark04_01.bmp", SMMAT_BLEND_LAMP);
	}
	*/
	if(matNum == NULL)
		return;
	Face.MatNum = *matNum;
}

void cASSAPARTSPARK::Main()
{
	POINT3D pos;
 	pos.x = int(Posi.x);
	pos.y = int(Posi.y);
	pos.z = int(Posi.z);

	D3DVECTOR term;
	term.x = float((DesPosi.x - pos.x)/ fONE);
	term.y = float((DesPosi.y - pos.y)/ fONE);
	term.z = float((DesPosi.z - pos.z)/ fONE);
		
	float length = term.x*term.x + term.y*term.y + term.z*term.z;
	length = (float)sqrt(length);
	
	if(Time%2)
	cAssaTrace->AddData(&Posi);		
 	
	D3DVECTOR velocity;
	if(length == 0)
		length = 1;
	velocity.x = term.x/length;
	velocity.y = term.y/length;
	velocity.z = term.z/length;
	
	Velocity.x += velocity.x;
	Velocity.y += velocity.y;
	Velocity.z += velocity.z;
	
	
	if(length < 60)
	{
		Velocity.x = Velocity.x * 0.8f;
		Velocity.y = Velocity.y * 0.8f;
		Velocity.z = Velocity.z * 0.8f;
	}
	

	Posi.x += int(Velocity.x * fONE);
	Posi.y += int(Velocity.y * fONE);
	Posi.z += int(Velocity.z * fONE);
	Time++;
	
	/*
	if(Time == 0)
	{
		SetDynLight( Posi.x, Posi.y, Posi.z, 255, 255, 255, 255, 100, 2);
		g_NewParticleMgr.Start("DivineLightning", Posi);
	}
	*/
}

void cASSAPARTSPARK::Draw(POINT3D *pPosi, POINT3D *pAngle)
{
	if(!Face.MatNum)return;
		
	// Turn on Z-buffering
	//lpD3DDevice->SetRenderState( D3DRENDERSTATE_ZENABLE, TRUE ); (문제가 생기는거 같아서 빼줬다)
	lpD3DDevice->SetRenderState( D3DRENDERSTATE_ZWRITEENABLE, FALSE );
	lpD3DDevice->SetRenderState( D3DRENDERSTATE_ZFUNC,   D3DCMP_LESSEQUAL );
	//lpD3DDevice->SetRenderState( D3DRENDERSTATE_ZENABLE, D3DZB_USEW );//TRUE );


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

	lpD3DDevice->SetTextureStageState( 0, D3DTSS_MINFILTER, D3DTFN_LINEAR  );
	lpD3DDevice->SetTextureStageState( 0, D3DTSS_MAGFILTER, D3DTFN_LINEAR  );


	smRender.Init(); //데이터 초기화 

	smMaterialGroup->smMaterial[Face.MatNum].TwoSide = TRUE; //카메라 회전과 상관없이 보이기 
		
	smRender.SetMaterialGroup(smMaterialGroup);  //읽어드린 메트리얼을 연결 
		

	//현재 찍을 Frame을 세팅한다..
	smMaterialGroup->SetMaterialAnimFrame( &smMaterial[Face.MatNum], 2);
	//Camera각도 입력.
	smRender.SetCameraPosi(pPosi->x, pPosi->y, pPosi->z,pAngle->x,pAngle->y,pAngle->z);
	
	if(!AssaAddFaceTrace(&Face,cAssaTrace))
		return;
	
	smRender.SetClipStates(SMCLIP_LEFT | SMCLIP_RIGHT | SMCLIP_TOP | SMCLIP_BOTTOM);
	smRender.ClipRendFace();
	smRender.GeomVertex2D(FALSE);
	smRender.RenderD3D();

	lpD3DDevice->SetRenderState( D3DRENDERSTATE_ZWRITEENABLE, TRUE );
}


bool HoGetWeaponPos(smCHAR *character, WEAPON_PART_POS *partPos)
{
	 if(character == NULL)
		return false;

	 if ( !character->Pattern || !character->HvLeftHand.ObjBip || !character->HvRightHand.ObjBip)
		 return false;

	if(character->ShootingFlag)
	{
		partPos->posCount = 1;
		partPos->pos[0].x = character->ShootingPosi.x;
		partPos->pos[0].y = character->ShootingPosi.y;
		partPos->pos[0].z = character->ShootingPosi.z;
		return true;
	}

	float u = 0.2f;
	smMATRIX *mWorld;
	POINT3D pBot[3];
	int index; for( index = 0; index < 3; index++)
	{
		pBot[index].x = 0;
		pBot[index].y = 0;
	}
	
	if(character->HvRightHand.dwItemCode != 0)
	{
		mWorld = &character->HvRightHand.ObjBip->mWorld;
		if((character->HvRightHand.dwItemCode & sinITEM_MASK2) == sinWT1) //제블린일 경우..
		{
			partPos->posCount = 1;
			pBot[0].z = -character->HvRightHand.SizeMax+int((float)character->HvRightHand.SizeMax*u);
		}
		else if((character->HvRightHand.dwItemCode & sinITEM_MASK2) == sinWS2) //칼인경우..
		{
			if(character->HvRightHand.SizeMax > 5000)
			{
				partPos->posCount = 3;
				pBot[0].z = character->HvRightHand.SizeMax-int((float)character->HvRightHand.SizeMax*0.25f);
				pBot[1].z = character->HvRightHand.SizeMax-int((float)character->HvRightHand.SizeMax*0.48f);
				pBot[2].z = character->HvRightHand.SizeMax-int((float)character->HvRightHand.SizeMax*0.69f);
				
			}
			else
			{
				partPos->posCount = 2;
				pBot[0].z = character->HvRightHand.SizeMax-int((float)character->HvRightHand.SizeMax*0.2f);
				pBot[1].z = character->HvRightHand.SizeMax-int((float)character->HvRightHand.SizeMax*0.6f);
			}
		}
		else if((character->HvRightHand.dwItemCode & sinITEM_MASK2) ==  sinWM1)//완드인경우..
		{
	 		partPos->posCount = 1;
			pBot[0].z = character->HvRightHand.SizeMax-int((float)character->HvRightHand.SizeMax*0.2f);
		}
		else
		{
			partPos->posCount = 1;
			pBot[0].z = character->HvRightHand.SizeMax-int((float)character->HvRightHand.SizeMax*u);
		}
	}
	else if(character->HvLeftHand.dwItemCode != 0)
	{
 		mWorld = &character->HvLeftHand.ObjBip->mWorld;
		partPos->posCount = 1;
		pBot[0].z = character->HvLeftHand.SizeMax-fONE*4;
		
		int cnt;
		if( character->HvLeftHand.dwItemCode != 0) //활을 든경우 보정..
		{
			
 			cnt = (ANGLE_90)&ANGCLIP;
			GetMoveLocation(pBot[0].x, pBot[0].y, pBot[0].z, 0, cnt, 0);
			pBot[0].x = GeoResult_X;
			pBot[0].y = GeoResult_Y;
			pBot[0].z = GeoResult_Z;
		}
	}
	else
		return false;
	
	//양손 무기 5도 기울어짐..
	int cnt;
	int sin;
	int cos;
	if ( character->dwActionItemTwoHand ) 
	{
		for(index = 0; index < partPos->posCount; index++)
		{
			cnt = (-ANGLE_45/5)&ANGCLIP;
			
			sin = GetSin[cnt]>>8;
			cos = GetCos[cnt]>>8;
			
			pBot[index].y = -( pBot[index].z * sin )>>8;
			pBot[index].z =  ( pBot[index].z * cos )>>8;

		}
	}
	
	POINT3D angle;
	memcpy( &angle, &character->Angle , sizeof( POINT3D ) );
	angle.y = (-angle.y+ANGLE_180)&ANGCLIP;
	
	if(character->HvRightHand.dwItemCode != 0)
		AnimObjectTree( character->HvRightHand.ObjBip, character->frame, angle.x, angle.y, angle.z);
	else if(character->HvLeftHand.dwItemCode != 0)
		AnimObjectTree( character->HvLeftHand.ObjBip, character->frame, angle.x, angle.y, angle.z);
	else 
		return false;

	
	
	int rx, ry, rz;
	for(index = 0; index < partPos->posCount; index++)
	{
		rx = pBot[index].x * mWorld->_11 +
			 pBot[index].y * mWorld->_21 +
			 pBot[index].z * mWorld->_31 ;

		ry = pBot[index].x * mWorld->_12 +
			 pBot[index].y * mWorld->_22 +
			 pBot[index].z * mWorld->_32 ;

		rz = pBot[index].x * mWorld->_13 +
			 pBot[index].y * mWorld->_23 +
			 pBot[index].z * mWorld->_33 ;

		partPos->pos[index].x = character->pX + (rx>>FLOATNS) + mWorld->_41;
		partPos->pos[index].z = character->pZ + (ry>>FLOATNS) + mWorld->_42;
		partPos->pos[index].y = character->pY + (rz>>FLOATNS) + mWorld->_43;
	}
	return true;
}


bool HoGetJavelinWeaponPos(smCHAR *character, WEAPON_PART_POS *partPos)
{
	if(character == NULL)
		return false;

	if ( !character->Pattern || !character->HvLeftHand.ObjBip || !character->HvRightHand.ObjBip)
		 return false;

	if(character->ShootingFlag)
	{
		partPos->posCount = 1;
		partPos->pos[0].x = character->ShootingPosi.x;
		partPos->pos[0].y = character->ShootingPosi.y;
		partPos->pos[0].z = character->ShootingPosi.z;
		return true;
	}

	float u = 0.2f;
	smMATRIX *mWorld;
	POINT3D pBot[3];
	int index; for( index = 0; index < 3; index++)
	{
		pBot[index].x = 0;
		pBot[index].y = 0;

	}

	if(character->HvRightHand.dwItemCode != 0)
	{
		mWorld = &character->HvRightHand.ObjBip->mWorld;
		if((character->HvRightHand.dwItemCode & sinITEM_MASK2) == sinWT1) //제블린일 경우..
		{
			partPos->posCount = 1;
			pBot[0].z = -character->HvRightHand.SizeMax+int((float)character->HvRightHand.SizeMax*u);
		}
		else 
			return false;
		
	}
	else
		return false;
	
	//양손 무기 5도 기울어짐..
	int cnt;
	int sin;
	int cos;
	if ( character->dwActionItemTwoHand ) 
	{
		for(index = 0; index < partPos->posCount; index++)
		{
			cnt = (-ANGLE_45/5)&ANGCLIP;
			
			sin = GetSin[cnt]>>8;
			cos = GetCos[cnt]>>8;
			
			pBot[index].y = -( pBot[index].z * sin )>>8;
			pBot[index].z =  ( pBot[index].z * cos )>>8;

		}
	}
	
	POINT3D angle;
	memcpy( &angle, &character->Angle , sizeof( POINT3D ) );
	angle.y = (-angle.y+ANGLE_180)&ANGCLIP;
	
	if(character->HvRightHand.dwItemCode != 0)
		AnimObjectTree( character->HvRightHand.ObjBip, character->frame, angle.x, angle.y, angle.z);
	else if(character->HvLeftHand.dwItemCode != 0)
		AnimObjectTree( character->HvLeftHand.ObjBip, character->frame, angle.x, angle.y, angle.z);
	else 
		return false;

	
	
	int rx, ry, rz;
	for(index = 0; index < partPos->posCount; index++)
	{
		rx = pBot[index].x * mWorld->_11 +
			 pBot[index].y * mWorld->_21 +
			 pBot[index].z * mWorld->_31 ;

		ry = pBot[index].x * mWorld->_12 +
			 pBot[index].y * mWorld->_22 +
			 pBot[index].z * mWorld->_32 ;

		rz = pBot[index].x * mWorld->_13 +
			 pBot[index].y * mWorld->_23 +
			 pBot[index].z * mWorld->_33 ;

		partPos->pos[index].x = character->pX + (rx>>FLOATNS) + mWorld->_41;
		partPos->pos[index].z = character->pZ + (ry>>FLOATNS) + mWorld->_42;
		partPos->pos[index].y = character->pY + (rz>>FLOATNS) + mWorld->_43;
	}
	return true;
}

cASSAChar::cASSAChar()
{
	PartNum = -1;
	PartNumExt1 = -1;
	

	
	EnchantPartNum[0] = -1;
	EnchantPartNum[1] = -1;
	EnchantPartNum[2] = -1;
	MyCharacterFlag = false;
	State = 0;
	FadeStopFlag = false;
	FlagShow = true;
}

cASSAChar::~cASSAChar()
{

}

void cASSAChar::Main()
{
	if(pChar == NULL)
		return;
	
	if(Time >= Max_Time)
		return;
	
	//Flag == 0이면 캐릭터가 안보이는거다..
	if(!MyCharacterFlag)
		if(pChar->Flag == 0)
			StopEffect();
	
	//포인터 바뀌는거 보정..
	if(MyCharacterFlag)
	{
		if(pChar != lpCurPlayer)
			pChar = lpCurPlayer;
	}
	
	if((EffectType == ENCHANT_WEAPON_FIRE ||
	   EffectType == ENCHANT_WEAPON_ICE ||
	   EffectType == ENCHANT_WEAPON_LIGHT||
	   EffectType == IMPULSION_LIGHT) && FadeStopFlag==false)
	{
		
		if(pChar->dwWeaponEffectTime == 0 && EffectType != IMPULSION_LIGHT)
		{
			FadeStopFlag = true;
		 	Time = Max_Time-2;
		}

		//화면 출력 여부가 바뀌면 다시 세팅한다.
		if(FlagShow != pChar->FlagShow)
		{
			FlagShow = pChar->FlagShow;
			for(int index =0; index < 3; index++)
			{
				if(EnchantPartNum[index] != -1)
					g_NewParticleMgr.SetRendering(EnchantPartNum[index], FlagShow);
			}
		}
		
		//무기를 하나두 들지 않았거나 마을로 갔을 경우 파티클을 끈다.
 		if(pChar->HvRightHand.dwItemCode == 0 && pChar->HvLeftHand.dwItemCode == 0 ||
		   (pChar->OnStageField>=0 && StageField[ pChar->OnStageField ]->State == FIELD_STATE_VILLAGE))
		{
			for(int index = 0; index < 3; index++)
			{
				if(EnchantPartNum[index] != -1)
				{
					g_NewParticleMgr.SetStop(EnchantPartNum[index]);
					EnchantPartNum[index] = -1;
				}
			}
			ItemCode = 0;
			return;
		}

		WEAPON_PART_POS PART_POS;
 		memset(&PART_POS, 0, sizeof(PART_POS));
	 	HoGetWeaponPos(pChar, &PART_POS);
		
	 	for(int index = 0; index < PART_POS.posCount; index++)
		{
			if(EnchantPartNum[index] != -1)
			{
				D3DVECTOR partPos;
				memset(&partPos, 0, sizeof(partPos));
				partPos.x = (float)PART_POS.pos[index].x;
				partPos.y = (float)PART_POS.pos[index].y;
				partPos.z = (float)PART_POS.pos[index].z;

				g_NewParticleMgr.SetPos(EnchantPartNum[index], partPos);
				//g_NewParticleMgr.SetAttachPos(EnchantPartNum[index], partPos);
			}
		}

		//아이템을 바꿔 들었을 경우..
		if((ItemCode != pChar->HvRightHand.dwItemCode && pChar->HvLeftHand.dwItemCode == 0) ||
		   (ItemCode != pChar->HvLeftHand.dwItemCode && pChar->HvRightHand.dwItemCode == 0) )
		{
			
			if(pChar->HvRightHand.dwItemCode != 0)
				ItemCode = pChar->HvRightHand.dwItemCode;
			else if(pChar->HvLeftHand.dwItemCode != 0)
				ItemCode = pChar->HvLeftHand.dwItemCode;
			
			for(int index = 0; index < 3; index++)
			{
				if(EnchantPartNum[index] != -1)
				{
					g_NewParticleMgr.SetStop(EnchantPartNum[index]);
					EnchantPartNum[index] = -1;
				}
			}
			
			if(EffectType == ENCHANT_WEAPON_FIRE)
			{
				char fileName[256];
				int partCount = 1;
				if(PART_POS.posCount == 2)
					partCount = 2;
				for(int index = 0; index < PART_POS.posCount; index++)
				{
 	 	 			wsprintf(fileName, "%s%d", "EnchantWeaponFire", index+partCount);
					EnchantPartNum[index] = g_NewParticleMgr.Start(fileName, PART_POS.pos[index]);
				}
			}
			else if(EffectType == ENCHANT_WEAPON_ICE)
			{
				for(int index = 0; index < PART_POS.posCount; index++)
					EnchantPartNum[index] = g_NewParticleMgr.Start("EnchantWeaponIce", PART_POS.pos[index]);

				
			}
			else if(EffectType == ENCHANT_WEAPON_LIGHT || EffectType == IMPULSION_LIGHT)
			{
				for(int index = 0; index < PART_POS.posCount; index++)
					EnchantPartNum[index] = g_NewParticleMgr.Start("EnchantWeaponLight", PART_POS.pos[index]);
			}

		}

	}

	if(EffectType == HOLY_VALOR_JANG || EffectType == HOLY_VALOR_MEMBER ||
	   EffectType == DRASTIC_SPIRIT_JANG || EffectType == DRASTIC_SPIRIT_MEMBER)
	{
		D3DVECTOR partPos;
 	 	partPos.x = (float)pChar->pX;
		partPos.y = (float)pChar->pY+1000;
		partPos.z = (float)pChar->pZ;

		if(PartNum != -1)
			g_NewParticleMgr.SetAttachPos(PartNum, partPos);

		if(FlagShow != pChar->FlagShow)
		{
			FlagShow = pChar->FlagShow;
			if(PartNum != -1)
				g_NewParticleMgr.SetRendering(PartNum, FlagShow);
		}
	}
	
	if(EffectType == MONSTER_SERQBUS_STATE1 ||
	   EffectType == MONSTER_SERQBUS_STATE2 ||
	   EffectType == MONSTER_SERQBUS_STATE3 ||
	   EffectType == MONSTER_POSION_STATE1 || // 박철호
	   EffectType == HOLY_MIND_ATTACK)
	{
		//화면 출력 여부가 바뀌면 다시 세팅한다.
		if(FlagShow != pChar->FlagShow)
		{
			FlagShow = pChar->FlagShow;
			if(PartNum != -1)
				g_NewParticleMgr.SetRendering(PartNum, FlagShow);
		}

		D3DVECTOR partPos;
  	 	partPos.x = (float)pChar->pX;
		partPos.y = (float)pChar->pY+(float)pChar->PatHeight;
		partPos.z = (float)pChar->pZ;
		if(PartNum != -1)
			g_NewParticleMgr.SetAttachPos(PartNum, partPos);
	}
	
	if(EffectType == SKILL_FROST_JAVELIN && FadeStopFlag == false)
	{
		if(FlagShow != pChar->FlagShow)
		{
			FlagShow = pChar->FlagShow;
			if(PartNum != -1)
				g_NewParticleMgr.SetRendering(PartNum, FlagShow);
		}
		
		if(pChar->ShootingFlag)
			g_NewParticleMgr.SetRendering(PartNum, FALSE);
		else
 			g_NewParticleMgr.SetRendering(PartNum, TRUE);

		//무기를 하나두 들지 않았을 때는 파티클을 끈다..
 		if( (pChar->HvRightHand.dwItemCode == 0 && pChar->HvLeftHand.dwItemCode == 0) ||
			//(ItemCode != pChar->HvRightHand.dwItemCode) ||
			((pChar->HvRightHand.dwItemCode & sinITEM_MASK2) != sinWT1) ||
			(pChar->OnStageField>=0 && StageField[ pChar->OnStageField ]->State == FIELD_STATE_VILLAGE))
		{
			g_NewParticleMgr.SetStop(PartNum);
			PartNum = -1;
			Time = Max_Time-2;
			return;
		} 
		
		
		D3DVECTOR partPos;
		WEAPON_PART_POS PART_POS;
		memset(&PART_POS, 0, sizeof(PART_POS));
		//무기 좌표가 있을 경우..
		if(HoGetJavelinWeaponPos(pChar, &PART_POS))
		{
			partPos.x = (float)PART_POS.pos[0].x;
			partPos.y = (float)PART_POS.pos[0].y;
			partPos.z = (float)PART_POS.pos[0].z;
			
			if(PartNum != -1)
				g_NewParticleMgr.SetAttachPos(PartNum, partPos);
			else 
				PartNum = g_NewParticleMgr.Start("Skill4FrostJavelin", PART_POS.pos[0]);
		}
		else
 		{
			Time = Max_Time-2;
		}
	}

	//Windy일 경우..
	if(EffectType == WINDY_ACTION && FadeStopFlag==false)
	{
		if(FlagShow)
		{
    		 if(Time%(70*2) == 0)
				 AssaParticle_Windy2(pChar);
		}

   		if(FlagShow != pChar->FlagShow)
		{
			FlagShow = pChar->FlagShow;
			if(PartNum != -1)
				g_NewParticleMgr.SetRendering(PartNum, FlagShow);
			if(PartNumExt1 != -1)
				g_NewParticleMgr.SetRendering(PartNum, FlagShow);
		}

		//무기를 하나두 들지 않았을 때는 파티클을 끈다..
 		if( (pChar->HvRightHand.dwItemCode == 0 && pChar->HvLeftHand.dwItemCode == 0) ||
			//(ItemCode != pChar->HvRightHand.dwItemCode) ||
 			((pChar->HvRightHand.dwItemCode & sinITEM_MASK2) != sinWT1) ||
			(pChar->OnStageField>=0 && StageField[ pChar->OnStageField ]->State == FIELD_STATE_VILLAGE))
		{
			g_NewParticleMgr.SetStop(PartNum);
			PartNum = -1;
			Time = Max_Time-2;
			return;
		} 

		D3DVECTOR partPos;
		WEAPON_PART_POS PART_POS;
		memset(&PART_POS, 0, sizeof(PART_POS));
		//무기 좌표가 있을 경우..
		if(HoGetJavelinWeaponPos(pChar, &PART_POS))
		{
			partPos.x = (float)PART_POS.pos[0].x;
			partPos.y = (float)PART_POS.pos[0].y;
			partPos.z = (float)PART_POS.pos[0].z;
			
			if(PartNum != -1)
				g_NewParticleMgr.SetAttachPos(PartNum, partPos);
			else 
				PartNum = g_NewParticleMgr.Start("Windy1", PART_POS.pos[0]);

			if(pChar->ShootingFlag)
			{
				if(State == 0)
				{
					PartNumExt1 = g_NewParticleMgr.Start("Windy2", PART_POS.pos[0]);
					State = 1;
				}
			}
		}
		else
 		{
			Time = Max_Time-2;
		}
		
		if(State == 1)
		{
			if(PartNumExt1 != -1)
				g_NewParticleMgr.SetPos(PartNumExt1, partPos);
		}
		if(!pChar->ShootingFlag && State == 1)
		{
			if(PartNumExt1 != -1)
			{
				g_NewParticleMgr.SetStop(PartNumExt1);
				State = 0;
				PartNumExt1 = -1;
			}
		}
	}

	if(EffectType == ENCHANT_MEMBER_JIN)
	{
		D3DVECTOR partPos;
		partPos.x = (float)pChar->pX;
		partPos.y = (float)pChar->pY+1000;
		partPos.z = (float)pChar->pZ;

		if(PartNum != -1)
		{
			if(g_NewParticleMgr.GetRunning(PartNum))
			{
				g_NewParticleMgr.SetAttachPos(PartNum, partPos);
			}
			else
				Time = Max_Time;
		}
	}
	
	
	if((Time+70*3 >= Max_Time)) 
 	{
	 	if(EffectType == HOLY_VALOR_JANG     || EffectType == HOLY_VALOR_MEMBER ||
 		   EffectType == DRASTIC_SPIRIT_JANG || EffectType == DRASTIC_SPIRIT_MEMBER)
		{
			g_NewParticleMgr.SetFastStop(PartNum);
			//PartNum = -1;
		}
	}
	
	if( (Time+2) >= Max_Time)
		StopEffect();

	if(!MyCharacterFlag)
		if(pChar->Flag)
			if( !pChar->MotionInfo || pChar->MotionInfo->State == CHRMOTION_STATE_DEAD)
				StopEffect();

	if(MyCharacterFlag)
		if(!pChar->MotionInfo || pChar->MotionInfo->State == CHRMOTION_STATE_DEAD)
			StopEffect();
}

void cASSAChar::StopEffect()
{
	if(EffectType == HOLY_VALOR_JANG     || EffectType == HOLY_VALOR_MEMBER ||
	   EffectType == DRASTIC_SPIRIT_JANG || EffectType == DRASTIC_SPIRIT_MEMBER)
	{
		g_NewParticleMgr.SetFastStop(PartNum);
		g_NewParticleMgr.SetStop(PartNum);
	}
	else
	{
		FadeStopFlag = TRUE;
		if(PartNum != -1)
		{
			g_NewParticleMgr.SetStop(PartNum);
			PartNum = -1;
		}

		if(PartNumExt1 != -1)
		{
			g_NewParticleMgr.SetStop(PartNumExt1);
			PartNumExt1 = -1;
		}

		for(int index = 0; index < 3; index++)
		{
			if(EnchantPartNum[index] != -1)
			{
				g_NewParticleMgr.SetStop(EnchantPartNum[index]);
				EnchantPartNum[index] = -1;
			}
		}
	}
}

void cASSAChar::Start(smCHAR *character, int code, int maxTime, float delay)
{
	if(character == NULL)
		return;

	EffectType = code;
    pChar = character;
	
	//마을에서는 스킬을 쓸수 없음..
 	if(pChar->OnStageField>=0 && StageField[ pChar->OnStageField ]->State == FIELD_STATE_VILLAGE)
		return;
	
	if(maxTime == 0)
		Max_Time = 70*5;
	else
		Max_Time = maxTime;
	
	Posi.x = pChar->pX;
	Posi.y = pChar->pY;
	Posi.z = pChar->pZ;

	CODE = code;
	if(EffectType == ENCHANT_WEAPON_FIRE ||
	   EffectType == ENCHANT_WEAPON_LIGHT ||
	   EffectType == ENCHANT_WEAPON_ICE) 
	   //EffectType == IMPULSION_LIGHT)
	{
		CODE = ENCHANT_WEAPON;
		Max_Time = maxTime+70*3; //시간 오차 보정..
	}
	if(EffectType == IMPULSION_LIGHT)
		CODE = IMPULSION_LIGHT;
 
	if(EffectType == HOLY_VALOR_JANG || EffectType == HOLY_VALOR_MEMBER)
		CODE = HOLY_ACTION;
	else if(EffectType == DRASTIC_SPIRIT_JANG || EffectType == DRASTIC_SPIRIT_MEMBER)
		CODE = DRASTIC_ACTION;

		//EffectType == DRASTIC_SPIRIT_JANG || EffectType == DRASTIC_SPIRIT_MEMBER)

	   
	
 	if(EffectType == ENCHANT_WEAPON_FIRE)
	{
		if(pChar->HvRightHand.dwItemCode != 0)
			ItemCode = pChar->HvRightHand.dwItemCode;
		else if(pChar->HvLeftHand.dwItemCode != 0)
			ItemCode = pChar->HvLeftHand.dwItemCode;
		
		WEAPON_PART_POS PART_POS;
		memset(&PART_POS, 0, sizeof(PART_POS));
		HoGetWeaponPos(pChar, &PART_POS);
		
		char fileName[256];
		int partCount = 1;
		if(PART_POS.posCount == 2)
			partCount = 2;
		for(int index = 0; index < PART_POS.posCount; index++)
		{
 	 	 	wsprintf(fileName, "%s%d", "EnchantWeaponFire", index+partCount);

			EnchantPartNum[index] = g_NewParticleMgr.Start(fileName, PART_POS.pos[index], delay);
		}
	}
	else if(EffectType == ENCHANT_WEAPON_ICE)
	{
		if(pChar->HvRightHand.dwItemCode != 0)
			ItemCode = pChar->HvRightHand.dwItemCode;
		else if(pChar->HvLeftHand.dwItemCode != 0)
			ItemCode = pChar->HvLeftHand.dwItemCode;

		WEAPON_PART_POS PART_POS;
		memset(&PART_POS, 0, sizeof(PART_POS));
		HoGetWeaponPos(pChar, &PART_POS);
		
		for(int index = 0; index < PART_POS.posCount; index++)
			EnchantPartNum[index] = g_NewParticleMgr.Start("EnchantWeaponIce", PART_POS.pos[index], delay);
	}
	else if(EffectType == ENCHANT_WEAPON_LIGHT || EffectType == IMPULSION_LIGHT)
	{
 		if(pChar->HvRightHand.dwItemCode != 0)
			ItemCode = pChar->HvRightHand.dwItemCode;
		else if(pChar->HvLeftHand.dwItemCode != 0)
			ItemCode = pChar->HvLeftHand.dwItemCode;

		WEAPON_PART_POS PART_POS;
		memset(&PART_POS, 0, sizeof(PART_POS));
		HoGetWeaponPos(pChar, &PART_POS);

		for(int index = 0; index < PART_POS.posCount; index++)
			EnchantPartNum[index] = g_NewParticleMgr.Start("EnchantWeaponLight", PART_POS.pos[index], delay);
	}
	
	if(EffectType == HOLY_VALOR_MEMBER)
	{
		Max_Time += 70*3;
		PartNum = g_NewParticleMgr.Start("HolyValor", Posi);
	}
	else if(EffectType == HOLY_VALOR_JANG)
	{
		Max_Time += 70*3;
		PartNum = g_NewParticleMgr.Start("HolyValor", Posi, 0.3f);
	}
	if(EffectType == DRASTIC_SPIRIT_MEMBER)
	{
		Max_Time += 70*3;
		PartNum = g_NewParticleMgr.Start("DrasticSpirit", Posi);
	}
	else if(EffectType == DRASTIC_SPIRIT_JANG)
	{
		Max_Time += 70*3;
 		PartNum = g_NewParticleMgr.Start("DrasticSpirit", Posi, 0.3f);
	}
	
	if(EffectType == WINDY_ACTION)
	{
		if(pChar->HvRightHand.dwItemCode != 0)
			ItemCode = pChar->HvRightHand.dwItemCode;
		else
			ItemCode = 0;
		PartNum = g_NewParticleMgr.Start("Windy1", Posi);

	}

	if(EffectType == SKILL_FROST_JAVELIN)
	{
		if(pChar->HvRightHand.dwItemCode != 0)
			ItemCode = pChar->HvRightHand.dwItemCode;
		else
			ItemCode = 0;
		PartNum = g_NewParticleMgr.Start("Skill4FrostJavelin", Posi);
	}

	if(EffectType == ENCHANT_MEMBER_JIN)
	{
		PartNum = g_NewParticleMgr.Start("EnchantWeaponMember", Posi);
	}

 	if(EffectType == MONSTER_SERQBUS_STATE1)
		PartNum = g_NewParticleMgr.Start("SerqbusState1", Posi);
	else if(EffectType == MONSTER_SERQBUS_STATE2)
		PartNum = g_NewParticleMgr.Start("SerqbusState2", Posi);
	else if(EffectType == MONSTER_SERQBUS_STATE3 || EffectType == HOLY_MIND_ATTACK)
		PartNum = g_NewParticleMgr.Start("SerqbusState3", Posi);

	if(EffectType == MONSTER_POSION_STATE1)	// 박철호 (독효과)
	{
		PartNum = g_NewParticleMgr.Start("ParkPosion", Posi);
	}


	MyCharacterFlag = false;
	if(pChar == lpCurPlayer)
		MyCharacterFlag = true;
}

cASSAItem::cASSAItem()
{
	MyCharacterFlag = false;
	FlagShow = FALSE;
}

cASSAItem::~cASSAItem()
{
	PartNum = -1;
}

void cASSAItem::Main()
{
	
	if(pChar == NULL)
		return;
	
	//Flag == 0이면 캐릭터가 안보이는거다..
	if(!MyCharacterFlag)
	{
		if(pChar->Flag == 0)
			StopEffect();
	}
	
	//포인터 바뀌는거 보정..
	if(MyCharacterFlag)
	{
		if(pChar != lpCurPlayer)
			pChar = lpCurPlayer;
	}

	D3DVECTOR partPos;
 	partPos.x = (float)pChar->pX;
	partPos.y = (float)pChar->pY+1000;
	partPos.z = (float)pChar->pZ;

	if(PartNum != -1)
		g_NewParticleMgr.SetAttachPos(PartNum, partPos);

	if(FlagShow != pChar->FlagShow)
	{
		FlagShow = pChar->FlagShow;
		if(PartNum != -1)
			g_NewParticleMgr.SetRendering(PartNum, FlagShow);
	}
}

void cASSAItem::StopEffect()
{
	if(PartNum != -1)
	{
		g_NewParticleMgr.SetStop(PartNum);
		PartNum = -1;
	}
}

void cASSAItem::Start(smCHAR *character, int code)
{
	if(character == NULL)
		return;

	EffectType = code;
	CODE = code;
    pChar = character;
	
	Posi.x = pChar->pX;
	Posi.y = pChar->pY;
	Posi.z = pChar->pZ;

 	if(EffectType == AGING_BODY)
		PartNum = g_NewParticleMgr.Start("AgingBody", Posi);

	Max_Time = 70*50;
}

bool SearchObjLocalPos(smCHAR *character, smOBJ3D *obj, POINT3D *outObjPos)
{
	if(character == NULL ||
		obj == NULL)
		return false;
	
	smMATRIX *mWorld;
	mWorld = &obj->mWorld;
	POINT3D angle;
	memcpy( &angle, &character->Angle , sizeof( POINT3D ) );
	angle.y = (-angle.y+ANGLE_180)&ANGCLIP;
	AnimObjectTree( obj, character->frame, angle.x, angle.y, angle.z);
	
	outObjPos->x = mWorld->_41;
	outObjPos->z = mWorld->_42;
	outObjPos->y = mWorld->_43;

	return true;	
}

bool SearchObjLocalPos(smCHAR *character, smOBJ3D *obj, POINT3D *outObjPos, int length)
{
	if(character == NULL ||
		obj == NULL)
		return false;
	
	smMATRIX *mWorld;
	mWorld = &obj->mWorld;
	POINT3D angle;
	memcpy( &angle, &character->Angle , sizeof( POINT3D ) );
	angle.y = (-angle.y+ANGLE_180)&ANGCLIP;
	AnimObjectTree( obj, character->frame, angle.x, angle.y, angle.z);
	
	outObjPos->x = mWorld->_41;
	outObjPos->z = mWorld->_42;
	outObjPos->y = mWorld->_43;

	return true;	
}

cAssaTracePatMeshParticle::cAssaTracePatMeshParticle()
{
	PartNum = -1;
	FlagShow = FALSE;
	PatObj = NULL;
	SearchObj = NULL;
	AttachFlag = FALSE;
}

cAssaTracePatMeshParticle::~cAssaTracePatMeshParticle()
{

}

void cAssaTracePatMeshParticle::Start(smCHAR *character, smPAT3D *pat, smOBJ3D *searchObj, char *particleName, int liveTime)
{
	if(pat == NULL || character == NULL)
		return;
	
	PatObj = pat;
	Posi.x = character->pX;
	Posi.y = character->pY;
	Posi.z = character->pZ;
	
	memcpy(&Angle, &character->Angle, sizeof(POINT3D));
	SearchObj = searchObj;//pat->GetObjectFromName(meshName); //오브젝트를 읽는다
	
	smMATRIX *mWorld;
	mWorld = &SearchObj->mWorld;
	Angle.y = (-Angle.y+ANGLE_180)&ANGCLIP;
	AnimObjectTree( SearchObj, PatObj->Frame, Angle.x, Angle.y, Angle.z);
	
	POINT3D localPos;
	localPos.x = mWorld->_41;
	localPos.z = mWorld->_42;
	localPos.y = mWorld->_43;
	
	POINT3D worldPos;
	worldPos.x = Posi.x + localPos.x;
	worldPos.y = Posi.y + localPos.y;
	worldPos.z = Posi.z + localPos.z;

	PartNum = g_NewParticleMgr.Start(particleName, worldPos);

	Max_Time = 50000;

	TimeCount = 0;
}


void cAssaTracePatMeshParticle::Main()
{
	if(PartNum != -1)
	{
		smMATRIX *mWorld;
		mWorld = &SearchObj->mWorld;
		Angle.y = (-Angle.y+ANGLE_180)&ANGCLIP;
		AnimObjectTree( SearchObj, PatObj->Frame, Angle.x, Angle.y, Angle.z);
		
		if(PatObj->Frame > 0)
		{
 			int temp;
			temp = 1;
		}
		POINT3D localPos;
		localPos.x = mWorld->_41;
		localPos.z = mWorld->_42;
		localPos.y = mWorld->_43;
		
		D3DVECTOR worldPos;
		worldPos.x = float(Posi.x + localPos.x);
		worldPos.y = float(Posi.y + localPos.y);
		worldPos.z = float(Posi.z + localPos.z);
		
		
		g_NewParticleMgr.SetPos(PartNum, worldPos);
		if(!g_NewParticleMgr.GetRunning(PartNum))
			PartNum = -1;
	}
	TimeCount++;
}


cAssaTraceCharMeshParticle::cAssaTraceCharMeshParticle()
{
	PartNum = -1;
	FlagShow = FALSE;
	SearchObj = NULL;
	MyCharacterFlag = FALSE;
	AttachFlag = true;
	Length = 0;

	Scale = 1.0f;
}
cAssaTraceCharMeshParticle::~cAssaTraceCharMeshParticle()
{
}

void cAssaTraceCharMeshParticle::Start(smCHAR *character, char *meshName, char *particleName, bool attachFlag, int liveTime)
{
	if(character == NULL)
		return;
	
 	pChar = character;
	Posi.x = pChar->pX;
	Posi.y = pChar->pY;
	Posi.z = pChar->pZ;

	PartNum = g_NewParticleMgr.Start(particleName, Posi);
	
	if(pChar->AnimPattern == NULL)
		return;
	SearchObj = pChar->AnimPattern->GetObjectFromName(meshName); //오브젝트를 읽는다
	
	Max_Time = 50000;
	if(character == lpCurPlayer)
		MyCharacterFlag = TRUE;

	AttachFlag = attachFlag;

	TimeCount = 0;
	LiveTime = liveTime;

	FlagShow = TRUE;
}


void cAssaTraceCharMeshParticle::Main()
{
	if(!MyCharacterFlag && pChar->Flag == 0)
	{
		if(PartNum != -1)
		{
	 		g_NewParticleMgr.SetFastStop(PartNum);
			PartNum = -1;
		}
		Time = Max_Time;

		return;
	}
 	
	if(!MyCharacterFlag)
		if(pChar->Flag)
			if( !pChar->MotionInfo || pChar->MotionInfo->State == CHRMOTION_STATE_DEAD)
			{
				if(PartNum != -1)
				{
	 				g_NewParticleMgr.SetFastStop(PartNum);
					g_NewParticleMgr.SetStop(PartNum);
					PartNum = -1;
				}

				Time = Max_Time;
				return;
			}

	if(MyCharacterFlag)
	{
		if(lpCurPlayer != pChar)
			pChar = lpCurPlayer;
	}
	
	//화면 출력 여부가 바뀌면 다시 세팅한다.
	if(FlagShow != pChar->FlagShow)
	{
		FlagShow = pChar->FlagShow;
		if(PartNum != -1)
		{
			g_NewParticleMgr.SetRendering(PartNum, FlagShow);
		}
	}
	
	bool flag = false;
	//if(Length == 0)
	flag = SearchObjLocalPos(pChar, SearchObj, &Posi);
	//else
		//flag = AssaSearchObjPos(pChar, SearchObj, &Posi, Length);

	if(flag)
	{
		D3DVECTOR pos;
 	 	pos.x = (float)Posi.x*Scale+(float)pChar->pX;
		pos.y = (float)Posi.y*Scale+(float)pChar->pY;
		pos.z = (float)Posi.z*Scale+(float)pChar->pZ+Length;
		
		//파티클이 살아 있으면..
		if(g_NewParticleMgr.GetRunning(PartNum))
		{
			if(AttachFlag)
 				g_NewParticleMgr.SetAttachPos(PartNum, pos);
			else
				g_NewParticleMgr.SetPos(PartNum, pos);

			g_NewParticleMgr.SetAngle(PartNum, pChar->Angle);

			if(LiveTime != -1)
			{
				if(LiveTime <= TimeCount)
				{
					Time = Max_Time;
					if(PartNum != -1)
					{
 						g_NewParticleMgr.SetFastStop(PartNum);
						PartNum = -1;
					}
				}
				else
				{
					//무한 루프를 돌리기 위해서..
 					if(Time+70 >= Max_Time)
						Time = 0;
				}
			}
			else
			{
				if(Time+70 >= Max_Time)
					Time = 0;
			}
		}
		else //파티클이 살아 있지 않으면 죽인다.
		{
			Max_Time = Time;	
		}
	}
	else
		Max_Time = Time;

	TimeCount++;
}


cAssaTraceMeshDestParticle::cAssaTraceMeshDestParticle()
{
	MeshRenderFlag = TRUE;
}

cAssaTraceMeshDestParticle::~cAssaTraceMeshDestParticle()
{

}

void cAssaTraceMeshDestParticle::Start(smPAT3D *pat, char *meshName, POINT3D *curPos, POINT3D *destPos, char *particleName)
{

	PatMesh = pat;
	SearchObj = pat->GetObjectFromName(meshName);
	
	Posi.x = curPos->x;
	Posi.y = curPos->y;
	Posi.z = curPos->z;

	DesPosi.x = destPos->x;
	DesPosi.y = destPos->y;
	DesPosi.z = destPos->z;
	
	PartNum = g_NewParticleMgr.Start(particleName, Posi);
	CurFrame = 0;

	GetRadian3D( Posi.x, Posi.y, Posi.z, DesPosi.x, DesPosi.y, DesPosi.z);
	Angle.x = GeoResult_X;
	Angle.y = GeoResult_Y;
	Angle.z = 0;
	
	Angle.y = (-Angle.y+ANGLE_180)&ANGCLIP;
	memset(&LocalPos, 0, sizeof(LocalPos));

	Max_Time = 5000;
}

void cAssaTraceMeshDestParticle::Main()
{
	D3DVECTOR term;
	term.x = float((DesPosi.x - (Posi.x+LocalPos.x))/ fONE);
	term.y = float((DesPosi.y - (Posi.y+LocalPos.y))/ fONE);
	term.z = float((DesPosi.z - (Posi.z+LocalPos.z))/ fONE);
	
	float length = term.x*term.x + term.y*term.y + term.z*term.z;
	length = (float)sqrt(length);
	
 	if(length < 100)
	{
		Max_Time = Time;
		if(PartNum != -1)
		{
			g_NewParticleMgr.SetStop(PartNum);
			PartNum = -1;
		}
	}
	else
	{
		//무한 루프를 돌리기 위해서..
 		if(Time+70 >= Max_Time)
			Time = 0;
		
 		CurFrame += 80;
		if(PatMesh->MaxFrame <= CurFrame)
		{
			CurFrame = 0;
		}
		
		smMATRIX *mWorld;
 		mWorld = &SearchObj->mWorld;
 		AnimObjectTree( SearchObj, CurFrame, Angle.x, Angle.y, Angle.z);
		LocalPos.x = mWorld->_41;
		LocalPos.z = mWorld->_42;
		LocalPos.y = mWorld->_43;
		

		D3DVECTOR partPos;
		partPos.x = float(Posi.x + LocalPos.x);
		partPos.y = float(Posi.y + LocalPos.y);
		partPos.z = float(Posi.z + LocalPos.z);

		if(PartNum != -1)
			g_NewParticleMgr.SetPos(PartNum, partPos);

	}
}

void cAssaTraceMeshDestParticle::Draw(POINT3D *pPosi, POINT3D *pAngle)
{
	
	if(MeshRenderFlag)
	{
 		PatMesh->Frame = CurFrame;
		PatMesh->SetPosi(&Posi, &Angle);
		
		lpD3DDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, FALSE);
		PatMesh->RenderD3D(pPosi, pAngle);
		lpD3DDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, TRUE);
	}
}


cAssaMotionBlur::cAssaMotionBlur()
{
	SearchObj1 = NULL;
	SearchObj2 = NULL;
	MyCharacterFlag = FALSE;
	Scale = 1.f;
}

cAssaMotionBlur::~cAssaMotionBlur()
{
}

void cAssaMotionBlur::Main()
{
 	if(MyCharacterFlag)
	{
		if(lpCurPlayer != pChar)
			pChar = lpCurPlayer;
	}
	
	TimeCount++;
	if(StartDelay == 0)
	{
		if(LiveTime <= Time)
			Time = Max_Time;
		else
		{
			//무한 루프를 돌리기 위해서..
 			if(Time+70 >= Max_Time)
			Time = 0;
		}
	}
	else
	{
 		if(TimeCount >= StartDelay)
		{
			StartDelay = 0;
			TimeCount = 0;
		}
	}
}

//static smTEXLINK MotionBlurTexLink[64];		//텍스쳐 좌표
//static smVERTEX  MotionBlurVertex[32];
//static smFACE	 MotionBlurFace[32];
static smTEXLINK MotionBlurTexLink[128];		//텍스쳐 좌표
static smVERTEX  MotionBlurVertex[64];
static smFACE	 MotionBlurFace[64];
void cAssaMotionBlur::Draw(POINT3D *pPosi , POINT3D *pAngle)
{
 	if(StartDelay > 0)
		return;
	if(Time >= Max_Time)
		return;
	
	int mLevel = 32; 
	int mFrames = 30;
	int mLevel2 = 0;
	DWORD pframe;
	int	vcnt = 0;
	smVERTEX	*lpVertex;
	smFACE		*lpFace;
	
	mLevel2 = mLevel<<1;
	
	lpVertex = MotionBlurVertex;
	lpFace = MotionBlurFace;
	
	POINT3D angle;
 	memcpy( &angle, &pChar->Angle , sizeof( POINT3D ) );
	angle.y = (-angle.y+ANGLE_180)&ANGCLIP;
	smMATRIX *mWorld;
	int cnt; for( cnt =0;cnt<mLevel;cnt++) 
	{
		pframe = pChar->frame-(cnt*mFrames);
		if ( pframe<pChar->MotionInfo->StartFrame ) 
			pframe = pChar->MotionInfo->StartFrame;
 		vcnt = cnt<<1;
		mWorld = &SearchObj1->mWorld;
		AnimObjectTree( SearchObj1, pframe, angle.x , angle.y , angle.z );
		lpVertex[vcnt].x = pChar->pX + int((float)mWorld->_41*Scale);
		lpVertex[vcnt].z = pChar->pZ + int((float)mWorld->_42*Scale);
		lpVertex[vcnt].y = pChar->pY + int((float)mWorld->_43*Scale);
		vcnt++;
		
		mWorld = &SearchObj2->mWorld;
		AnimObjectTree( SearchObj2, pframe, angle.x , angle.y , angle.z );
		lpVertex[vcnt].x = pChar->pX + int((float)mWorld->_41*Scale);
		lpVertex[vcnt].z = pChar->pZ + int((float)mWorld->_42*Scale);
		lpVertex[vcnt].y = pChar->pY + int((float)mWorld->_43*Scale);
	}

	//클리핑 모드값
	int ClipStates = ( SMCLIP_NEARZ | SMCLIP_FARZ | SMCLIP_LEFT | SMCLIP_RIGHT | SMCLIP_TOP | SMCLIP_BOTTOM );

	smRender.Init();
	smRender.SetMaterialGroup(smMaterialGroup);

	smRender.SetCameraPosi(pPosi->x, pPosi->y, pPosi->z, pAngle->x, pAngle->y, pAngle->z);
	for (cnt = 0; cnt<mLevel2; cnt++) 
		smRender.AddObjectVertex( &lpVertex[cnt] );
	
	smVERTEX *vp1,*vp2;
	smVERTEX *vp3,*vp4;
	for (cnt = 0; cnt<(mLevel2-2); cnt+=2) 
	{
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

	if (smRender.nRendFace) 
	{
    	//이부분 고쳐야 한다.
		int alpha = smRender.Color_A;
		smRender.Color_A = Alpha;

		smRender.SetClipStates( ClipStates );				// 클리핑 선택
		smRender.ClipRendFace();							// 전체 렌더링 페이스를 클리핑
		smRender.GeomVertex2D( FALSE );						// 버텍스를 2D좌표로 변환  
		smRender.RenderD3D();
		
		smRender.Color_A = Alpha;
	}
}


void cAssaMotionBlur::Start(smCHAR *character, char *meshName1, char *meshName2, int liveTime)
{
	if(character == NULL)
		return;

	TimeCount = 0;
	LiveTime = liveTime;
	Alpha = 0;

	pChar = character;
	if(pChar->AnimPattern == NULL)
		return;
	SearchObj1 = pChar->AnimPattern->GetObjectFromName(meshName1); //오브젝트를 읽는다
	SearchObj2 = pChar->AnimPattern->GetObjectFromName(meshName2); //오브젝트를 읽는다

	if(character == lpCurPlayer)
		MyCharacterFlag = TRUE;
	
	
  	int matNum =  *(int *)AssaSearchRes("m_DoomG-01.bmp", SMMAT_BLEND_LAMP);
	//CreateTextureMaterial( "Effect\\Temp\\m_DoomG-01.bmp" ,NULL , TRUE , FALSE ,FALSE  , SMMAT_BLEND_LAMP);//ALPHA );
	//ReadTextures();
	float fBlurLevel = 1.0f/32;
	int TlCnt = 0;
	float fVCnt = 0;
	float fVCnt2 = 0+ fBlurLevel;

	int cnt; for( cnt =0; cnt<32; cnt++ ) {

		MotionBlurTexLink[TlCnt].NextTex  = 0;
		MotionBlurTexLink[TlCnt].hTexture = 0;
		MotionBlurTexLink[TlCnt].v[0] = fVCnt;
		MotionBlurTexLink[TlCnt].v[1] = fVCnt;
		MotionBlurTexLink[TlCnt].v[2] = fVCnt2;
		MotionBlurTexLink[TlCnt].u[0] = 0;
		MotionBlurTexLink[TlCnt].u[1] = 1;
		MotionBlurTexLink[TlCnt].u[2] = 0;
		
		TlCnt++;

		MotionBlurTexLink[TlCnt].NextTex  = 0;
		MotionBlurTexLink[TlCnt].hTexture = 0;
		MotionBlurTexLink[TlCnt].v[0] = fVCnt2;
		MotionBlurTexLink[TlCnt].v[1] = fVCnt;
		MotionBlurTexLink[TlCnt].v[2] = fVCnt2;
		MotionBlurTexLink[TlCnt].u[0] = 0;
		MotionBlurTexLink[TlCnt].u[1] = 1;
		MotionBlurTexLink[TlCnt].u[2] = 1;

		TlCnt++;

		fVCnt = fVCnt2;
		fVCnt2 += fBlurLevel;

	}

	int MbCnt = 0;
	int MvCnt = 0;

 	ZeroMemory( MotionBlurVertex , sizeof( smVERTEX ) * 64 );

	for( cnt=0; cnt<32; cnt++) {

		MotionBlurFace[ MbCnt ].lpTexLink = &MotionBlurTexLink[MbCnt];
		MotionBlurFace[ MbCnt ].v[0] = MvCnt;
		MotionBlurFace[ MbCnt ].v[1] = MvCnt+1;
		MotionBlurFace[ MbCnt ].v[2] = MvCnt+2;
		MotionBlurFace[ MbCnt ].v[3] = matNum;
		MbCnt++;

		MotionBlurFace[ MbCnt ].lpTexLink = &MotionBlurTexLink[MbCnt];
		MotionBlurFace[ MbCnt ].v[0] = MvCnt+2;
		MotionBlurFace[ MbCnt ].v[1] = MvCnt+1;
		MotionBlurFace[ MbCnt ].v[2] = MvCnt+3;
		MotionBlurFace[ MbCnt ].v[3] = matNum;
		MbCnt++;
		MvCnt+=2;
	}
	Max_Time = 50000;

	StartDelay = 0;
}


cAssaTwoPointAni::cAssaTwoPointAni()
{
	SearchObj1 = NULL;
	SearchObj2 = NULL;
	MyCharacterFlag = FALSE;
	TimeCount = 0;
}

cAssaTwoPointAni::~cAssaTwoPointAni()
{

}

/*
void cAssaTwoPointAni::Draw(POINT3D *pPosi, POINT3D *pAngle)
{
	if(pChar != NULL)
	{
		cASSAFACE::Draw(pPosi, pAngle);
	}
}
*/

void cAssaTwoPointAni::Main()
{
	if(!MyCharacterFlag && pChar->Flag == 0)
	{
		Time = Max_Time;
		return;
	}

 	if(MyCharacterFlag)
	{
		if(lpCurPlayer != pChar)
			pChar = lpCurPlayer;
	}
	
	if(!MyCharacterFlag)
		if(pChar->Flag)
			if( !pChar->MotionInfo || pChar->MotionInfo->State == CHRMOTION_STATE_DEAD)
				Time = Max_Time;

	if(pChar == NULL || SearchObj1 == NULL || SearchObj2 == NULL)
		Time = Max_Time;

	POINT3D dest;

  	AssaSearchObjPos(pChar, SearchObj1, &Posi);
 	AssaSearchObjPos(pChar, SearchObj2, &dest);

    Face.x = Posi.x;
	Face.y = Posi.y;
	Face.z = Posi.z;
 	Face.width = 5000;
	AniDelayTime = 4;

  	ScalePosi.x = dest.x - Posi.x;
	ScalePosi.y = dest.y - Posi.y;
	ScalePosi.z = dest.z - Posi.z;

	
	Face.r = 255;
	Face.g = 255; 
	Face.b = 255;
	//Face.a = 255;

	TimeCount++;
	if(StartDelay == 0)
	{
		
		//무한 루프를 돌리기 위해서..
 		if(Time+70 >= Max_Time)
		Time = 0;
		
	}
	else
	{
 		if(TimeCount >= StartDelay)
		{
			StartDelay = 0;
			TimeCount = 0;
		}
	}
	
	//////// Animation
 	if(AniReverse == 2){ //역순
		if(AniDelayTime &&  (Time % AniDelayTime) == 0 ){
			AniCount--;
			if(AniCount < 1){
				if(	AniReverse == 2)AniReverse = 1;

			}
			Face.MatNum = AniMatIndex[AniCount];
		}

	}
	else{ //바른 차순
		if(Time >0  && AniDelayTime &&  (Time % AniDelayTime) == 0 ){
			Face.MatNum = AniMatIndex[AniCount];
			AniCount++;
			if(AniCount > AniMaxCount-1){
				if(	AniReverse == 1)AniReverse = 2;
				else AniCount = 0;
			}
		}
	}

	//cASSAFACE::Main();
}

void cAssaTwoPointAni::Start(smCHAR *character, char *meshName1, char *meshName2)
{
	if(character == NULL)
		return;

	TimeCount = 0;
	LiveTime = 5000;

	pChar = character;
	if(pChar->AnimPattern == NULL)
		return;
	SearchObj1 = pChar->AnimPattern->GetObjectFromName(meshName1); //오브젝트를 읽는다
	SearchObj2 = pChar->AnimPattern->GetObjectFromName(meshName2); //오브젝트를 읽는다

	if(character == lpCurPlayer)
		MyCharacterFlag = TRUE;
	
 	Face.MatNum = *(int *)AssaSearchRes("light_G001.bmp",SMMAT_BLEND_LAMP);
	strcpy(szFileName, "light_G001.bmp");

 	AssaGetAniFileMat(this);
	AniDelayTime = 3;
	Face.r = 255;
	Face.g = 255; 
	Face.b = 255;
	Face.Transparency = 255;
 	Face.width = 6000;
    
	FACE_TYPE = ASSAFACE_SCALE; 
	
	POINT3D dest;
	AssaSearchObjPos(pChar, SearchObj1, &Posi);
	AssaSearchObjPos(pChar, SearchObj2, &dest);

	ScalePosi.x = dest.x - Posi.x;
	ScalePosi.y = dest.y - Posi.y;
	ScalePosi.z = dest.z - Posi.z;

	Max_Time = 5000;
}

AssaSkill3Concentration::AssaSkill3Concentration()
{

	pChar = NULL;
	MyCharacterFlag = FALSE;
	ParticleID = -1;
}

AssaSkill3Concentration::~AssaSkill3Concentration()
{

}

void AssaSkill3Concentration::Start(smCHAR* character, int liveCount)
{
	pChar = character;
	if(pChar == NULL)
		return;
	if(lpCurPlayer == pChar)
	{
		MyCharacterFlag = TRUE;
	}
	
	POINT3D charPos;
	charPos.x = pChar->pX;
	charPos.y = pChar->pY+10000;
	charPos.z = pChar->pZ;

	ParticleID = g_NewParticleMgr.Start("Skill3FighterConcentration1", charPos);
	//Max_Time = 70*180;	//3분..

	Max_Time = liveCount*70+140;
	CODE = SKILL_CONCENTRATION;
	EndFlag = FALSE;
}

void AssaSkill3Concentration::Main()
{	
	if(ParticleID == -1)
		return;
	
	if(!g_NewParticleMgr.GetRunning(ParticleID) && EndFlag)
	{
		ParticleID = -1;
		return;
	}

 	if(MyCharacterFlag)
	{
		if(lpCurPlayer != pChar)
			pChar = lpCurPlayer;
	}
	
	if(Time == 40)
 		SetDynLight(pChar->pX, pChar->pY, pChar->pZ, 255, 100, 50,0, 150, 1);

	D3DVECTOR charPos;
	charPos.x = (float)pChar->pX;
	charPos.y = (float)pChar->pY;
	charPos.z = (float)pChar->pZ;
	g_NewParticleMgr.SetAttachPos(ParticleID, charPos);
	g_NewParticleMgr.SetRendering(ParticleID, pChar->FlagShow);
	
 	if(EndFlag == FALSE)
	{
		if(Time > Max_Time-140 ||
		(!MyCharacterFlag && pChar->Flag == 0) ||
		(!pChar->MotionInfo || pChar->MotionInfo->State == CHRMOTION_STATE_DEAD) ||
		(GetAssaContinueSkill(SKILL_SWIFT_AXE) && Time > 70) ||
		(pChar->OnStageField>=0 && StageField[ pChar->OnStageField ]->State == FIELD_STATE_VILLAGE) )
		{
 			Time = Max_Time-140;
			if(ParticleID != -1)
			{
				g_NewParticleMgr.SetFastStop(ParticleID);
				//ParticleID = -1;
			}
			EndFlag = TRUE;
		}
	}
}

AssaSkill3SwiftAxe::AssaSkill3SwiftAxe()
{
	pChar = NULL;	
	MyCharacterFlag = FALSE;
	ParticleID = -1;
}

AssaSkill3SwiftAxe::~AssaSkill3SwiftAxe()
{
	
}

void AssaSkill3SwiftAxe::Start(smCHAR* character, int liveCount)
{
    pChar = character;
	if(pChar == NULL)
		return;
		if(lpCurPlayer == pChar)
	{
		MyCharacterFlag = TRUE;
	}
	
	POINT3D charPos;
	charPos.x = pChar->pX;
	charPos.y = pChar->pY+10000;
	charPos.z = pChar->pZ;

	ParticleID = g_NewParticleMgr.Start("Skill3FighterSwiftAxe1", charPos);
	//Max_Time = 70*180;

 	//LiveCount = liveCount*70;
 	EndFlag = FALSE;
  	Max_Time = liveCount*70+140;
	CODE = SKILL_SWIFT_AXE;

}


void AssaSkill3SwiftAxe::Main()
{	
	if(ParticleID == -1)
		return;
	
	if(!g_NewParticleMgr.GetRunning(ParticleID) && EndFlag)
	{
		ParticleID = -1;
		return;
	}

	if(MyCharacterFlag)
	{
		if(lpCurPlayer != pChar)
			pChar = lpCurPlayer;
	}
	
	if(Time == 40)
  		SetDynLight(pChar->pX, pChar->pY, pChar->pZ, 50, 100, 255,0, 140, 1);

	D3DVECTOR charPos;
	charPos.x = (float)pChar->pX;
	charPos.y = (float)pChar->pY;
	charPos.z = (float)pChar->pZ;
	g_NewParticleMgr.SetAttachPos(ParticleID, charPos);

	if(pChar->FlagShow == FALSE || ((pChar->dwActionItemCode & sinITEM_MASK2) != sinWA1))
		g_NewParticleMgr.SetRendering(ParticleID, FALSE);
	else
		g_NewParticleMgr.SetRendering(ParticleID, TRUE);
	
 	if(EndFlag == FALSE)
	{
		if(Time > Max_Time-140 ||
		(!MyCharacterFlag && pChar->Flag == 0) ||
		(!pChar->MotionInfo || pChar->MotionInfo->State == CHRMOTION_STATE_DEAD) ||
		(GetAssaContinueSkill(SKILL_CONCENTRATION) && Time > 70) ||
		(pChar->OnStageField>=0 && StageField[ pChar->OnStageField ]->State == FIELD_STATE_VILLAGE))
		{
 			Time = Max_Time-140;
			if(ParticleID != -1)
			{
				g_NewParticleMgr.SetFastStop(ParticleID);
				//ParticleID = -1;
			}
			EndFlag = TRUE;
		}
	}
}



AssaSkillWeaponMotionBlur::AssaSkillWeaponMotionBlur()
{
	MyCharacterFlag = FALSE;
}

AssaSkillWeaponMotionBlur::~AssaSkillWeaponMotionBlur()
{
}

void AssaSkillWeaponMotionBlur::Main()
{
 	if(MyCharacterFlag)
	{
		if(lpCurPlayer != pChar)
			pChar = lpCurPlayer;
	}
	
	TimeCount++;
	if(StartDelay == 0)
	{
		if(LiveTime <= Time)
			Time = Max_Time;
		else
		{
			//무한 루프를 돌리기 위해서..
 			if(Time+70 >= Max_Time)
			Time = 0;
		}
	}
	else
	{
 		if(TimeCount >= StartDelay)
		{
			StartDelay = 0;
			TimeCount = 0;
		}
	}
}

void AssaSkillWeaponMotionBlur::Draw(POINT3D *pPosi , POINT3D *pAngle)
{
 	if(StartDelay > 0)
		return;
	if(Time >= Max_Time)
		return;
	
	int mLevel = 32; 
	int mFrames = 30;
	int mLevel2 = 0;
	DWORD pframe;
	int	vcnt = 0;
	smVERTEX	*lpVertex;
	smFACE		*lpFace;
	
	mLevel2 = mLevel<<1;
	
	lpVertex = MotionBlurVertex;
	lpFace = MotionBlurFace;
	
	POINT3D angle;
 	memcpy( &angle, &pChar->Angle , sizeof( POINT3D ) );
	angle.y = (-angle.y+ANGLE_180)&ANGCLIP;
	
	
	POINT3D weaponTopPos;
	POINT3D weaponBottomPos;
	
	memset(&weaponTopPos, 0, sizeof(weaponTopPos));
 	memset(&weaponBottomPos, 0, sizeof(weaponBottomPos));
	weaponBottomPos.z = pChar->HvRightHand.SizeMax;	

	//양손 무기 5도 기울어짐..
	int cnt;
	int sin;
	int cos;
	if ( pChar->dwActionItemTwoHand ) 
	{
		cnt = (-ANGLE_45/5)&ANGCLIP;
			
		sin = GetSin[cnt]>>8;
		cos = GetCos[cnt]>>8;
		
		weaponTopPos.y = -( weaponTopPos.z * sin )>>8;
		weaponTopPos.z =  ( weaponTopPos.z * cos )>>8;

		weaponBottomPos.y = -( weaponBottomPos.z * sin )>>8;
		weaponBottomPos.z =  ( weaponBottomPos.z * cos )>>8;
	}

	smMATRIX *mWorld;
	mWorld = &pChar->HvRightHand.ObjBip->mWorld;
	int rx, ry, rz;
	
	for(cnt=0;cnt<mLevel;cnt++) 
	{
		pframe = pChar->frame-(cnt*mFrames);
		if ( pframe<pChar->MotionInfo->StartFrame ) 
			pframe = pChar->MotionInfo->StartFrame;
 		vcnt = cnt<<1;
		
		AnimObjectTree( pChar->HvRightHand.ObjBip, pframe, angle.x , angle.y , angle.z );
		rx = weaponTopPos.x * mWorld->_11 +
			 weaponTopPos.y * mWorld->_21 +
			 weaponTopPos.z * mWorld->_31 ;

		ry = weaponTopPos.x * mWorld->_12 +
			 weaponTopPos.y * mWorld->_22 +
			 weaponTopPos.z * mWorld->_32 ;

		rz = weaponTopPos.x * mWorld->_13 +
			 weaponTopPos.y * mWorld->_23 +
			 weaponTopPos.z * mWorld->_33 ;

		lpVertex[vcnt].x = pChar->pX + (rx>>FLOATNS) + mWorld->_41;
		lpVertex[vcnt].z = pChar->pZ + (ry>>FLOATNS) + mWorld->_42;
		lpVertex[vcnt].y = pChar->pY + (rz>>FLOATNS) + mWorld->_43;
		vcnt++;
		
		rx = weaponBottomPos.x * mWorld->_11 +
			 weaponBottomPos.y * mWorld->_21 +
			 weaponBottomPos.z * mWorld->_31 ;

		ry = weaponBottomPos.x * mWorld->_12 +
			 weaponBottomPos.y * mWorld->_22 +
			 weaponBottomPos.z * mWorld->_32 ;

		rz = weaponBottomPos.x * mWorld->_13 +
			 weaponBottomPos.y * mWorld->_23 +
			 weaponBottomPos.z * mWorld->_33 ;

		lpVertex[vcnt].x = pChar->pX + (rx>>FLOATNS) + mWorld->_41;
		lpVertex[vcnt].z = pChar->pZ + (ry>>FLOATNS) + mWorld->_42;
		lpVertex[vcnt].y = pChar->pY + (rz>>FLOATNS) + mWorld->_43;
	}

	//클리핑 모드값
	int ClipStates = ( SMCLIP_NEARZ | SMCLIP_FARZ | SMCLIP_LEFT | SMCLIP_RIGHT | SMCLIP_TOP | SMCLIP_BOTTOM );

	smRender.Init();
	smRender.SetMaterialGroup(smMaterialGroup);

	smRender.SetCameraPosi(pPosi->x, pPosi->y, pPosi->z, pAngle->x, pAngle->y, pAngle->z);
	for (cnt = 0; cnt<mLevel2; cnt++) 
		smRender.AddObjectVertex( &lpVertex[cnt] );
	
	smVERTEX *vp1,*vp2;
	smVERTEX *vp3,*vp4;
	for (cnt = 0; cnt<(mLevel2-2); cnt+=2) 
	{
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

	if (smRender.nRendFace) 
	{
	 	smRender.SetClipStates( ClipStates );				// 클리핑 선택
		smRender.ClipRendFace();							// 전체 렌더링 페이스를 클리핑
		smRender.GeomVertex2D( FALSE );						// 버텍스를 2D좌표로 변환  
		smRender.RenderD3D();
	}
}


void AssaSkillWeaponMotionBlur::Start(smCHAR *character, int liveTime)
{
	if(character == NULL)
		return;

	TimeCount = 0;
	LiveTime = liveTime;

	pChar = character;
	if(character == lpCurPlayer)
		MyCharacterFlag = TRUE;
	
	int matNum =  *(int *)AssaSearchRes("ft3_boneCrash.bmp", SMMAT_BLEND_LAMP);
	
	float fBlurLevel = 1.0f/32;
	int TlCnt = 0;
	float fVCnt = 0;
	float fVCnt2 = 0+ fBlurLevel;

	int cnt; for( cnt =0; cnt<32; cnt++ ) {

		MotionBlurTexLink[TlCnt].NextTex  = 0;
		MotionBlurTexLink[TlCnt].hTexture = 0;
		MotionBlurTexLink[TlCnt].v[0] = fVCnt;
		MotionBlurTexLink[TlCnt].v[1] = fVCnt;
		MotionBlurTexLink[TlCnt].v[2] = fVCnt2;
		MotionBlurTexLink[TlCnt].u[0] = 0;
		MotionBlurTexLink[TlCnt].u[1] = 1;
		MotionBlurTexLink[TlCnt].u[2] = 0;

		TlCnt++;

		MotionBlurTexLink[TlCnt].NextTex  = 0;
		MotionBlurTexLink[TlCnt].hTexture = 0;
		MotionBlurTexLink[TlCnt].v[0] = fVCnt2;
		MotionBlurTexLink[TlCnt].v[1] = fVCnt;
		MotionBlurTexLink[TlCnt].v[2] = fVCnt2;
		MotionBlurTexLink[TlCnt].u[0] = 0;
		MotionBlurTexLink[TlCnt].u[1] = 1;
		MotionBlurTexLink[TlCnt].u[2] = 1;

		TlCnt++;

		fVCnt = fVCnt2;
		fVCnt2 += fBlurLevel;

	}

	int MbCnt = 0;
	int MvCnt = 0;

 	ZeroMemory( MotionBlurVertex , sizeof( smVERTEX ) * 64 );

	for( cnt=0; cnt<32; cnt++) {

		MotionBlurFace[ MbCnt ].lpTexLink = &MotionBlurTexLink[MbCnt];
		MotionBlurFace[ MbCnt ].v[0] = MvCnt;
		MotionBlurFace[ MbCnt ].v[1] = MvCnt+1;
		MotionBlurFace[ MbCnt ].v[2] = MvCnt+2;
		MotionBlurFace[ MbCnt ].v[3] = matNum;
		MbCnt++;

		MotionBlurFace[ MbCnt ].lpTexLink = &MotionBlurTexLink[MbCnt];
		MotionBlurFace[ MbCnt ].v[0] = MvCnt+2;
		MotionBlurFace[ MbCnt ].v[1] = MvCnt+1;
		MotionBlurFace[ MbCnt ].v[2] = MvCnt+3;
		MotionBlurFace[ MbCnt ].v[3] = matNum;
		MbCnt++;
		MvCnt+=2;
	}
	Max_Time = 50000;

	StartDelay = 0;
}

AssaSkill3ElementalShot::AssaSkill3ElementalShot()
{
	ParticleID = -1;
	memset(&ShootingAngle, 0, sizeof(ShootingAngle));
	Type = -1;
	
}

AssaSkill3ElementalShot::~AssaSkill3ElementalShot()
{

}

void AssaSkill3ElementalShot::Start(POINT3D *pCur, POINT3D *pDest, int type)
{
	Type = type;
	if(Type == ELEMENTAL_FIRE)
		ParticleID = g_NewParticleMgr.Start("FireJavelinFire", Posi);
	else
		ParticleID = g_NewParticleMgr.Start("EnchantWeaponLight", Posi);
	Posi.x = pCur->x;
	Posi.y = pCur->y;
	Posi.z = pCur->z;
	
	DesPosi.x = pDest->x;
	DesPosi.y = pDest->y;
	DesPosi.z = pDest->z;

	GetRadian3D( Posi.x,Posi.y, Posi.z, DesPosi.x, DesPosi.y, DesPosi.z);
	int angX = GeoResult_X;
	int angY = GeoResult_Y;
	ShootingAngle.x = angX;
	ShootingAngle.y = angY;
	ShootingAngle.z = 0;

	int step = 12*fONE;				//창이나 기타 던지는 무기
 	GetMoveLocation( 0, 0, step, ShootingAngle.x, ShootingAngle.y, ShootingAngle.z );
	Posi.x += GeoResult_X;
	Posi.y += GeoResult_Y;
	Posi.z += GeoResult_Z;

	Max_Time = 5000;
}

void AssaSkill3ElementalShot::Main()
{
  	int step = 7*fONE;				//창이나 기타 던지는 무기
	GetMoveLocation( 0, 0, step, ShootingAngle.x, ShootingAngle.y, ShootingAngle.z );
	Posi.x += GeoResult_X;
	Posi.y += GeoResult_Y;
	Posi.z += GeoResult_Z;
		
	POINT3D pos;
 	pos.x = int(Posi.x);
	pos.y = int(Posi.y);
	pos.z = int(Posi.z);

	D3DVECTOR term;
	term.x = float((DesPosi.x - pos.x)/ fONE);
	term.y = float((DesPosi.y - pos.y)/ fONE);
	term.z = float((DesPosi.z - pos.z)/ fONE);
		
	float length = term.x*term.x + term.y*term.y + term.z*term.z;
	length = (float)sqrt(length);
		
  	if(length < 20 || Time > 60)
	{
   		if(ParticleID != -1)
		{
			g_NewParticleMgr.SetStop(ParticleID);
			Time = Max_Time;

			if(Type == ELEMENTAL_FIRE)
			{
				if ( (rand()%1)==0 )
					SkillPlaySound( SKILL_SOUND_SKILL_ENCHANTWEAPON_FIRE1 	 , Posi.x, Posi.y, Posi.z);
				else
					SkillPlaySound( SKILL_SOUND_SKILL_ENCHANTWEAPON_FIRE2 	 , Posi.x, Posi.y, Posi.z);
				AssaParticle_FireHit1(&Posi);
			}
			else if(Type == ELEMENTAL_LIGHTING)
			{
				if ( (rand()%1)==0 )
					SkillPlaySound( SKILL_SOUND_SKILL_ENCHANTWEAPON_LIGHTNING1   , Posi.x, Posi.y, Posi.z);
				else
					SkillPlaySound( SKILL_SOUND_SKILL_ENCHANTWEAPON_LIGHTNING2 , Posi.x, Posi.y, Posi.z);

				AssaParticle_LightHit1(&Posi);
			}
		}
	}

	D3DVECTOR partPos;
	partPos.x = (float)Posi.x;
	partPos.y = (float)Posi.y;
	partPos.z = (float)Posi.z;

	if(ParticleID != -1) 
		g_NewParticleMgr.SetPos(ParticleID, partPos);
}


AssaSkill3BombShot::AssaSkill3BombShot()
{
	ParticleID = -1;
	memset(&ShootingAngle, 0, sizeof(ShootingAngle));
	PatMesh = NULL;
}

AssaSkill3BombShot::~AssaSkill3BombShot()
{

}

void AssaSkill3BombShot::Start(POINT3D *pCur, POINT3D *pDest)
{
	ParticleID = g_NewParticleMgr.Start("Skill3ArcherBombShot1", Posi);
	
	Posi.x = pCur->x;
	Posi.y = pCur->y;
	Posi.z = pCur->z;
	
	DesPosi.x = pDest->x;
	DesPosi.y = pDest->y;
	DesPosi.z = pDest->z;

	GetRadian3D( Posi.x,Posi.y, Posi.z, DesPosi.x, DesPosi.y, DesPosi.z);
	int angX = GeoResult_X;
	int angY = GeoResult_Y;
	ShootingAngle.x = angX&ANGCLIP;
	ShootingAngle.y = angY&ANGCLIP;
	ShootingAngle.z = 0;

     int step = 40*fONE;				//창이나 기타 던지는 무기
 	GetMoveLocation( 0, 0, step, ShootingAngle.x, ShootingAngle.y, ShootingAngle.z );
	Posi.x += GeoResult_X;
	Posi.y += GeoResult_Y;
	Posi.z += GeoResult_Z;

	Max_Time = 5000;
 	PatMesh = (smPAT3D *)AssaSearchRes("At3_SplitJavelin40.ase", SMMAT_BLEND_ALPHA);
}



void AssaSkill3BombShot::Main()
{
 	int step = 6*fONE;				//창이나 기타 던지는 무기
	GetMoveLocation( 0, 0, step, ShootingAngle.x, ShootingAngle.y, ShootingAngle.z );
	Posi.x += GeoResult_X;
	Posi.y += GeoResult_Y;
	Posi.z += GeoResult_Z;
		
	POINT3D pos;
 	pos.x = int(Posi.x);
	pos.y = int(Posi.y);
	pos.z = int(Posi.z);

	D3DVECTOR term;
	term.x = float((DesPosi.x - pos.x)/ fONE);
	term.y = float((DesPosi.y - pos.y)/ fONE);
	term.z = float((DesPosi.z - pos.z)/ fONE);
		
	float length = term.x*term.x + term.y*term.y + term.z*term.z;
	length = (float)sqrt(length);
		
   	if(length < 25 || Time > 60)
	{
   		if(ParticleID != -1)
		{
			g_NewParticleMgr.SetStop(ParticleID);
			Time = Max_Time;
			AssaParticle_BombHit1(&pos);
			SkillPlaySound( SKILL_SOUND_SKILL_BOMB_SHOT2	 ,pos.x, pos.y, pos.z);
		}
	}
	
	D3DVECTOR partPos;
	partPos.x = (float)Posi.x;
	partPos.y = (float)Posi.y;
	partPos.z = (float)Posi.z;

	if(ParticleID != -1)
		g_NewParticleMgr.SetPos(ParticleID, partPos);
}


void AssaSkill3BombShot::Draw(POINT3D *pPosi , POINT3D *pAngle)
{
	if(PatMesh != NULL)
	{
   		smRender.Init(); //데이터 초기화 
		smRender.SetCameraPosi(pPosi->x, pPosi->y, pPosi->z,pAngle->x,pAngle->y,pAngle->z);
		
		POINT3D meshAngle;
		meshAngle.x = ShootingAngle.x;
  		meshAngle.y = (-ShootingAngle.y+ANGLE_180)&ANGCLIP;
 		meshAngle.z = ShootingAngle.z;

        //meshAngle.x = -(ShootingAngle.x&ANGCLIP);
   		//meshAngle.y = -(ShootingAngle.y&ANGCLIP);
 		//meshAngle.z = ShootingAngle.z&ANGCLIP;
		PatMesh->Frame = 0;
		PatMesh->SetPosi(&Posi, &meshAngle);
		

		//lpD3DDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, FALSE);
		PatMesh->RenderD3D(pPosi, pAngle);
		//lpD3DDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, TRUE);
		
	}
}



AssaSkill3Perforation::AssaSkill3Perforation()
{
	PatMesh = NULL;
	ParticleID = -1;
	State = 0;
}

AssaSkill3Perforation::~AssaSkill3Perforation()
{

}

void AssaSkill3Perforation::Start(POINT3D *pCur, POINT3D *pDest)
{
    int *matNum = (int *)AssaSearchRes("Shield1.bmp", SMMAT_BLEND_LAMP);
 	if(matNum == NULL)
		return;
 	PatMesh = (smPAT3D *)AssaSearchRes("Arrow.ase", SMMAT_BLEND_LAMP);
	PatMesh->SetPosi(&Posi, &ShootingAngle);

    Face.MatNum = *matNum;
 	Face.r = 255;
	Face.g = 255;
	Face.b = 255;
	Face.Transparency = 255;
 	Face.width = 5000;
	cAssaTrace->SetLength(15);

	Posi.x = pCur->x;
	Posi.y = pCur->y;
	Posi.z = pCur->z;
	
	DesPosi.x = pDest->x;
	DesPosi.y = pDest->y;
	DesPosi.z = pDest->z;

  	GetRadian3D( Posi.x,Posi.y, Posi.z, DesPosi.x, DesPosi.y, DesPosi.z);
	int angX = GeoResult_X;
	int angY = GeoResult_Y;
	ShootingAngle.x = angX;
	ShootingAngle.y = angY;
	ShootingAngle.z = 0;
	
	//int step = 12*fONE;				//창이나 기타 던지는 무기
 	//GetMoveLocation( 0, 0, step, ShootingAngle.x, ShootingAngle.y, ShootingAngle.z );
	//Posi.x += GeoResult_X;
	//Posi.y += GeoResult_Y;
	//Posi.z += GeoResult_Z;

 	Max_Time = 300;
	State = 0;
	ParticleID = g_NewParticleMgr.Start("twistJavelin1", Posi);
	State1TimeCount = 0;
}


void AssaSkill3Perforation::Draw(POINT3D *pPosi , POINT3D *pAngle)
{
 	if(!Face.MatNum)
		return;
	smRender.Init(); //데이터 초기화 
	smMaterialGroup->smMaterial[Face.MatNum].TwoSide = TRUE; //카메라 회전과 상관없이 보이기 
	smRender.SetMaterialGroup(smMaterialGroup);  //읽어드린 메트리얼을 연결 
	Face.width = 2000;
	if(!AssaAddFaceTrace(&Face, cAssaTrace))
			return;
	
	Face.width = 8000;
	if(!AssaAddFaceTrace(&Face, cAssaTrace))
			return;

	smRender.SetClipStates(SMCLIP_LEFT | SMCLIP_RIGHT | SMCLIP_TOP | SMCLIP_BOTTOM);
	smRender.ClipRendFace();
	smRender.GeomVertex2D(FALSE);
	smRender.RenderD3D();
}







void AssaSkill3Perforation::Main()
{
	float length;

    int step = 10*fONE;				//창이나 기타 던지는 무기
	GetMoveLocation( 0, 0, step, ShootingAngle.x, ShootingAngle.y, ShootingAngle.z );
	Posi.x += GeoResult_X;
	Posi.y += GeoResult_Y;
	Posi.z += GeoResult_Z;

	if(State == 0 || State == 1 || State == 2)
	cAssaTrace->AddData(&Posi);			
	
	if(ParticleID != -1)
	{
		D3DVECTOR pos;
		pos.x = (float)Posi.x;
		pos.y = (float)Posi.y;
		pos.z = (float)Posi.z;
		g_NewParticleMgr.SetPos(ParticleID, pos);
	}

	if(State == 0)
	{
		POINT3D pos;
 		pos.x = int(Posi.x);
		pos.y = int(Posi.y);
		pos.z = int(Posi.z);

		D3DVECTOR term;
		term.x = float((DesPosi.x - pos.x)/ fONE);
		term.y = float((DesPosi.y - pos.y)/ fONE);
		term.z = float((DesPosi.z - pos.z)/ fONE);
			
		length = term.x*term.x + term.y*term.y + term.z*term.z;
		length = (float)sqrt(length);
		
   		if(length < 20 || Time > 40)
 		{
			State = 1;
		}
	}
	
	//충돌을 한경우.
    if(State == 1)
	{
	 	if(ParticleID != -1)
		{
			g_NewParticleMgr.SetStop(ParticleID);
			ParticleID = -1;
		}

 		//State1TimeCount++;
		//if(State1TimeCount > 10)
		//{
			State = 2;
			State1TimeCount = 0;
		//}
		
		
		//if(Face.Transparency > 0)
			//Face.Transparency-= 4;
	}

 	if(State == 2)
 	{
 		if(Face.Transparency > 0)
		{
			Face.r -= 4;
			Face.g -= 4;
			Face.b -= 4;

			Face.Transparency -= 4;
		}
		else
			State1TimeCount++;
		
		if(State1TimeCount > 2)
			Time = Max_Time;
	}
}

AssaSkill3BombShotExplision::AssaSkill3BombShotExplision()
{
	memset(&Pos, 0, sizeof(Pos));
	memset(&Velocity, 0, sizeof(Pos));
	memset(&Gravity, 0, sizeof(Gravity));

	ParticleID = -1;
}

AssaSkill3BombShotExplision::~AssaSkill3BombShotExplision()
{

}

void AssaSkill3BombShotExplision::Start(POINT3D *pPosi, D3DVECTOR *velocity, D3DVECTOR *gravity)
{
 	Pos.x = (float)pPosi->x;
	Pos.y = (float)pPosi->y;
	Pos.z = (float)pPosi->z;
	
	Velocity.x = velocity->x;
	Velocity.y = velocity->y;
	Velocity.z = velocity->z;

	Gravity.x = gravity->x;
	Gravity.y = gravity->y;
	Gravity.z = gravity->z;
	
	ParticleID = g_NewParticleMgr.Start("Skill3ArcherBombShotHit2", *pPosi);
	Max_Time = 800;
}

void AssaSkill3BombShotExplision::Start(POINT3D *pPosi, D3DVECTOR *velocity, D3DVECTOR *gravity, char *particleName)
{
  	Pos.x = (float)pPosi->x;
	Pos.y = (float)pPosi->y;
	Pos.z = (float)pPosi->z;
	
	Velocity.x = velocity->x;
	Velocity.y = velocity->y;
	Velocity.z = velocity->z;

	Gravity.x = gravity->x;
	Gravity.y = gravity->y;
	Gravity.z = gravity->z;
	
  	ParticleID = g_NewParticleMgr.Start(particleName, *pPosi);
 	Max_Time = 200;
}

void AssaSkill3BombShotExplision::Main()
{
	
    if(ParticleID == -1)
		return;
	if(!g_NewParticleMgr.GetRunning(ParticleID))
	{
		Max_Time = 0;
		return;
	}
	
	Pos.x += Velocity.x;
	Pos.y += Velocity.y;
	Pos.z += Velocity.z;

	Pos.x += Gravity.x;
	Pos.y += Gravity.y;
	Pos.z += Gravity.z;
	
	g_NewParticleMgr.SetPos(ParticleID, Pos);
}



AssaSkill3SparkShield::AssaSkill3SparkShield()
{
	pChar = NULL;	
	MyCharacterFlag = FALSE;
	ParticleID = -1;
	SearchObj = NULL;
}

AssaSkill3SparkShield::~AssaSkill3SparkShield()
{
	
}

void AssaSkill3SparkShield::Start(smCHAR* character, int liveCount)
{
    pChar = character;
	if(pChar == NULL)
		return;
	if(lpCurPlayer == pChar)
	{
		MyCharacterFlag = TRUE;
	}
	
	POINT3D charPos;
	charPos.x = pChar->pX;
	charPos.y = pChar->pY+10000;
	charPos.z = pChar->pZ;

 	ParticleID = g_NewParticleMgr.Start("Skill3MechanicianSparkShield1", charPos);
	EndFlag = FALSE;
  	Max_Time = liveCount*70+140;
	CODE = SKILL_SPARK_SHIELD;

	if(pChar->AnimPattern == NULL)
		return;
 	SearchObj = pChar->AnimPattern->GetObjectFromName("Bip01 L Hand"); //오브젝트를 읽는다
}


void AssaSkill3SparkShield::Main()
{	
	if(ParticleID == -1)
		return;
	
	if(!g_NewParticleMgr.GetRunning(ParticleID) && EndFlag)
	{
		ParticleID = -1;
		return;
	}

	if(MyCharacterFlag)
	{
		if(lpCurPlayer != pChar)
			pChar = lpCurPlayer;
	}
	
	smMATRIX *mWorld;
    AnimObjectTree( SearchObj, pChar->frame, 0, (-pChar->Angle.y+ANGLE_180)&ANGCLIP, 0);
	mWorld = &SearchObj->mWorld;
	
	
	

	D3DVECTOR charPos;
	charPos.x = (float)pChar->pX+mWorld->_41;
	charPos.y = (float)pChar->pY+mWorld->_43;
	charPos.z = (float)pChar->pZ+mWorld->_42;

 	g_NewParticleMgr.SetAttachPos(ParticleID, charPos);
	if(pChar->HvLeftHand.dwItemCode == 0 || pChar->FlagShow == 0)
		g_NewParticleMgr.SetRendering(ParticleID, FALSE);
	else
		g_NewParticleMgr.SetRendering(ParticleID, TRUE);

	
 	if(EndFlag == FALSE)
	{
		if(Time > Max_Time-140 ||
		(!MyCharacterFlag && pChar->Flag == 0) ||
		(!pChar->MotionInfo || pChar->MotionInfo->State == CHRMOTION_STATE_DEAD) ||
		
		//((pChar->HvLeftHand.dwItemCode & sinITEM_MASK2)!=sinDS1 ) || //방패가 아닌 다른것을 들었을 경우
		!(pChar->dwDispAppSkill&SKILL_APPMASK_SPARK_SHIELD) ||
		(GetAssaContinueSkill(SKILL_EXTREME_SHIELD) && Time > 70)  || // //ExtremeShield를 쓰고 있을 경우
		(pChar->OnStageField>=0 && StageField[ pChar->OnStageField ]->State == FIELD_STATE_VILLAGE))                
		{
 			Time = Max_Time-140;
			if(ParticleID != -1)
			{
				g_NewParticleMgr.SetFastStop(ParticleID);
				//ParticleID = -1;
			}
			EndFlag = TRUE;
		}
	}
}


AssaSkill3MetalArmor::AssaSkill3MetalArmor()
{
	pChar = NULL;	
	MyCharacterFlag = FALSE;
	ParticleID = -1;
	PatMesh = NULL;
}

AssaSkill3MetalArmor::~AssaSkill3MetalArmor()
{
	
}

void AssaSkill3MetalArmor::Start(smCHAR* character, int liveCount)
{
  	pChar = character;
	if(pChar == NULL)
		return;
	if(lpCurPlayer == pChar)
	{
		MyCharacterFlag = TRUE;
	}
	
	POINT3D charPos;
 	charPos.x = pChar->pX;
	charPos.y = pChar->pY;
	charPos.z = pChar->pZ;

  	ParticleID = g_NewParticleMgr.Start("Skill3MechanicianMetalArmor", charPos);
	PatMesh = (smPAT3D *)AssaSearchRes("Mc3_MetalA60.ASE", SMMAT_BLEND_LAMP);
	
	CurrentFrame = 0;
	Max_Time = liveCount*70+150;
	CODE = SKILL_METAL_ARMOR;
	EndFlag = FALSE;
	MeshBlendStep = -255;
}


void AssaSkill3MetalArmor::Draw(POINT3D *pPosi , POINT3D *pAngle)
{
	if(PatMesh != NULL || ParticleID != -1)
	{
		if(pChar->FlagShow)
		{
			PatMesh->Frame = CurrentFrame;
			POINT3D angle;
			angle.x = 0;
			angle.y = 0;
			angle.z = 0;
			PatMesh->SetPosi(&Posi, &angle);

			int r,g,b,a;
			r = smRender.Color_R;
			g = smRender.Color_G;
			b = smRender.Color_B;
			a = smRender.Color_A;

 			smRender.Color_R = r;
			smRender.Color_G = g;
			smRender.Color_B = b;
			smRender.Color_A = a+MeshBlendStep;

			PatMesh->RenderD3D(pPosi, pAngle);

			smRender.Color_R = r;
			smRender.Color_G = g;
			smRender.Color_B = b;
			smRender.Color_A = a;

			//PatMesh->RenderD3D(pPosi, pAngle);
		}
	}
}

void AssaSkill3MetalArmor::Main()
{	
	if(ParticleID == -1)
		return;
	
	if(!g_NewParticleMgr.GetRunning(ParticleID) && EndFlag)
	{
		ParticleID = -1;
		return;
	}

	if(MyCharacterFlag)
	{
		if(lpCurPlayer != pChar)
			pChar = lpCurPlayer;
	}
	
     if(Time+170 > Max_Time)
 		MeshBlendStep -= 10;
	if(Time > 70)
	{
		MeshBlendStep += 5;
		if(MeshBlendStep >= 0)
			MeshBlendStep = 0;
	}
	
	Posi.x = pChar->pX;
 	Posi.y = pChar->pY+7000;
	Posi.z = pChar->pZ;

	D3DVECTOR charPos;
 	charPos.x = (float)pChar->pX;
	charPos.y = (float)pChar->pY;
	charPos.z = (float)pChar->pZ;
	g_NewParticleMgr.SetAttachPos(ParticleID, charPos);
	g_NewParticleMgr.SetRendering(ParticleID, pChar->FlagShow);
	
	CurrentFrame += 120;
 	if(CurrentFrame >= 160*59)
	{
		CurrentFrame = 0;
	}

    if(EndFlag == FALSE)
	{
		if(Time > Max_Time-140 ||
		(!MyCharacterFlag && pChar->Flag == 0) ||
		(!pChar->MotionInfo || pChar->MotionInfo->State == CHRMOTION_STATE_DEAD) ||
		
		(GetAssaContinueSkill(SKILL_PHYSICAL_ABSORB) && Time > 70) || //스킬이 없어지는 시간 약간 딜레이를 준건 시작 순간 Continue에 있는 정보가 바로갱신안되고
		(GetAssaContinueSkill(SKILL_MAXIMIZE) && Time > 70) ||        //아직도 쓰고 있다는 정보가 남아 있어서 Metal Armor도 없어지기 때문이다
      	(GetAssaContinueSkill(SKILL_AUTOMATION) && Time > 70) ||
		(pChar->OnStageField>=0 && StageField[ pChar->OnStageField ]->State == FIELD_STATE_VILLAGE) )
		{
 			Time = Max_Time-140;
			if(ParticleID != -1)
			{
				g_NewParticleMgr.SetFastStop(ParticleID);
				//ParticleID = -1;
			}
			EndFlag = TRUE;
		}
	}
}


AssaSkill3Vanish::AssaSkill3Vanish()
{
	pChar = NULL;	
	MyCharacterFlag = FALSE;
	ParticleID = -1;
}

AssaSkill3Vanish::~AssaSkill3Vanish()
{
	
}

void AssaSkill3Vanish::Start(smCHAR* character)
{
    pChar = character;
	if(pChar == NULL)
		return;
	if(lpCurPlayer == pChar)
	{
		MyCharacterFlag = TRUE;
	}
	
	POINT3D charPos;
	charPos.x = pChar->pX;
	charPos.y = pChar->pY+10000;
	charPos.z = pChar->pZ;

	ParticleID = g_NewParticleMgr.Start("Skill3PikeVanish1", charPos);
	Max_Time = 70 * 100;
}


void AssaSkill3Vanish::Main()
{	
	if(ParticleID == -1)
		return;
	
	if(!g_NewParticleMgr.GetRunning(ParticleID) ||
	   (!pChar->MotionInfo || pChar->MotionInfo->State == CHRMOTION_STATE_DEAD) )
	{
		ParticleID = -1;
		Time = Max_Time;
		return;
	}

	if(MyCharacterFlag)
	{
		if(lpCurPlayer != pChar)
			pChar = lpCurPlayer;
	}
	
	D3DVECTOR charPos;
	charPos.x = (float)pChar->pX;
	charPos.y = (float)pChar->pY+1000;
	charPos.z = (float)pChar->pZ;
	g_NewParticleMgr.SetAttachPos(ParticleID, charPos);
	g_NewParticleMgr.SetRendering(ParticleID, pChar->FlagShow);
}






/*
AssaSkill3BladeOfBlaze::AssaSkill3BladeOfBlaze()
{
	PatMesh = NULL;
	CurrentFrame = 0;
	ParticleID = -1;
	ObjBip = NULL;
	TimeCount = 0.f;
	Delay = 0.f;
	HitFlag = FALSE;
}

AssaSkill3BladeOfBlaze::~AssaSkill3BladeOfBlaze()
{
		
}

void AssaSkill3BladeOfBlaze::Start(smCHAR *character, int angleY, float delay)
{
  	Posi.x = character->pX;
	Posi.y = character->pY+35000;
	Posi.z = character->pZ;

	PatMesh = (smPAT3D *)AssaSearchRes("pk3_BladeofBlaze36.ASE", SMMAT_BLEND_LAMP);
	//ParticleID = g_NewParticleMgr.Start("Skill3PikeManBladeOfBlaze", Posi);
	ObjBip = PatMesh->GetObjectFromName("itwp115");
	if(PatMesh == NULL || ObjBip == NULL)
		return;

	//if(ParticleID == -1)
	//{
		//return;
	//}
	
	Angle.x = 0;
	Angle.y = angleY;
	Angle.z = 0;
	Max_Time = 1000;
	MaxFrame = PatMesh->MaxFrame;
	PatMesh->Frame = 0;
 	CurrentFrame = 0;

	Delay = delay;
	HitFlag = FALSE;
	MeshBlendStep = 0;
}

void AssaSkill3BladeOfBlaze::Draw(POINT3D *pPosi , POINT3D *pAngle)
{
	if(PatMesh == NULL || TimeCount < Delay)
		return;
	PatMesh->Frame = CurrentFrame;
	PatMesh->SetPosi(&Posi, &Angle);

	int r,g,b,a;
	r = smRender.Color_R;
	g = smRender.Color_G;
	b = smRender.Color_B;
	a = smRender.Color_A;

	smRender.Color_R = r+MeshBlendStep;
	smRender.Color_G = g+MeshBlendStep;
	smRender.Color_B = b+MeshBlendStep;
	smRender.Color_A = a+MeshBlendStep;
	PatMesh->RenderD3D(pPosi, pAngle);

	smRender.Color_R = r;
	smRender.Color_G = g;
	smRender.Color_B = b;
	smRender.Color_A = a;
}

void AssaSkill3BladeOfBlaze::Main()
{
	
	//float elapsedTime = (float)TimeCount/70.f;

   	if(TimeCount < Delay)
	//if(elapsedTime < Delay)
	{
		TimeCount += 1.f/70.f;
		return;
	}
	else
	{
		if(ParticleID == -1 && HitFlag == FALSE)
		{
			ParticleID = g_NewParticleMgr.Start("Skill3PikeManBladeOfBlaze", Posi);
		}
	}

	if(HitFlag)
	{
		MeshBlendStep -= 2;
	}

	if(PatMesh == NULL || ParticleID == -1)
		return;
	
	
  	CurrentFrame += 160;
	smMATRIX *mWorld;
	AnimObjectTree( ObjBip , CurrentFrame, 0, Angle.y, 0);
	mWorld = &ObjBip->mWorld;
	
	if(ParticleID != -1)
	{
		D3DVECTOR charPos;
		charPos.x = (float)(Posi.x+mWorld->_41);
		charPos.y = (float)(Posi.y+mWorld->_43);
		charPos.z = (float)(Posi.z+mWorld->_42);
		
		g_NewParticleMgr.SetPos(ParticleID, charPos);

	
		POINT3D terrainPos;
		terrainPos.x = (int)charPos.x;
		terrainPos.y = (int)charPos.y;
		terrainPos.z = (int)charPos.z;

		int pY;
		int sy;

 		if ( smGameStage[0] )
			pY = smGameStage[0]->GetHeight( terrainPos.x, terrainPos.z);

		if ( smGameStage[1] ) {
			sy = smGameStage[1]->GetHeight( terrainPos.x, terrainPos.z);
			if ( pY<sy ) pY = sy;
		}

    	if(pY >= (terrainPos.y-1000))
		{
 			g_NewParticleMgr.SetStop(ParticleID);
			ParticleID = -1;

			Time = Max_Time - 70*2;
			HitFlag = TRUE;
			
			g_NewParticleMgr.Start("Skill3PikeManBladeOfBlazeHit", terrainPos); 
		}
	}
	
	if(CurrentFrame > MaxFrame)
	{
		CurrentFrame = MaxFrame;
		Time = Max_Time;

		if(ParticleID != -1)
		{
			g_NewParticleMgr.SetStop(ParticleID);
			ParticleID = -1;
			HitFlag = TRUE;
		}
	} 
}
*/


AssaSkill3EnergyShield::AssaSkill3EnergyShield()
{
	pChar = NULL;	
	MyCharacterFlag = FALSE;
	ParticleID = -1;
	PatMesh = NULL;
	memset(&Angle, 0, sizeof(Angle));
	CurrentFrame = 0;
}

AssaSkill3EnergyShield::~AssaSkill3EnergyShield()
{
	
}

void AssaSkill3EnergyShield::Start(smCHAR* character,int liveCount)
{
	pChar = character;
	if(pChar == NULL)
		return;
	if(lpCurPlayer == pChar)
	{
		MyCharacterFlag = TRUE;
	}
	
	POINT3D charPos;
	charPos.x = pChar->pX;
	charPos.y = pChar->pY;
	charPos.z = pChar->pZ;

 	ParticleID = g_NewParticleMgr.Start("Skill3MagicianEngrgyShield", charPos);
	
	Max_Time = liveCount*70+150;
	CODE = SKILL_ENERGY_SHIELD;
	EndFlag = FALSE;
	CurrentFrame = 0;
	PatMesh = (smPAT3D *)AssaSearchRes("Mg3_EnergyS20.ASE", SMMAT_BLEND_LAMP);
	MeshBlendStep = -200;
}

void AssaSkill3EnergyShield::Draw(POINT3D *pPosi, POINT3D *pAngle)
{
	if(PatMesh == NULL)
		return;

	if(pChar)
	{
		if(pChar->FlagShow == 0)
			return;
	}

   	PatMesh->Frame = CurrentFrame;
	POINT3D curPosi;
	curPosi.x = Posi.x;
	curPosi.y = Posi.y+6500;
	curPosi.z = Posi.z;

	PatMesh->SetPosi(&curPosi, &Angle);
	
	int r,g,b,a;
	r = smRender.Color_R;
	g = smRender.Color_G;
	b = smRender.Color_B;
	a = smRender.Color_A;

 	smRender.Color_R = r;
	smRender.Color_G = g;
	smRender.Color_B = b;
	smRender.Color_A = a+MeshBlendStep;

 	
	PatMesh->RenderD3D(pPosi, pAngle);
	

	smRender.Color_R = r;
	smRender.Color_G = g;
	smRender.Color_B = b;
	smRender.Color_A = a;
}

void AssaSkill3EnergyShield::Main()
{	
	if(ParticleID == -1)
		return;
	
	if(!g_NewParticleMgr.GetRunning(ParticleID) && EndFlag)
	{
		ParticleID = -1;
		return;
	}

	if(MyCharacterFlag)
	{
		if(lpCurPlayer != pChar)
			pChar = lpCurPlayer;
	}
	
	Posi.x = pChar->pX;
 	Posi.y = pChar->pY;
	Posi.z = pChar->pZ;

	D3DVECTOR charPos;
  	charPos.x = (float)pChar->pX;
	charPos.y = (float)pChar->pY+500.f;
	charPos.z = (float)pChar->pZ;
	g_NewParticleMgr.SetAttachPos(ParticleID, charPos);
	g_NewParticleMgr.SetRendering(ParticleID, pChar->FlagShow);

   	CurrentFrame += 20;
	if(CurrentFrame >= 160*20)
		CurrentFrame = 0;
	if(EndFlag)
	{
		MeshBlendStep -= 3;
	}
	else
	{
  		MeshBlendStep += 2;
		if(MeshBlendStep > 0)
			MeshBlendStep = 0;
	}

	if(EndFlag == FALSE)
	{
		if(Time > Max_Time-140 ||
		(!MyCharacterFlag && pChar->Flag == 0) ||
		(!pChar->MotionInfo || pChar->MotionInfo->State == CHRMOTION_STATE_DEAD) ||
		
		(pChar->OnStageField>=0 && StageField[ pChar->OnStageField ]->State == FIELD_STATE_VILLAGE))
		{
 			Time = Max_Time-140;
			if(ParticleID != -1)
			{
				g_NewParticleMgr.SetFastStop(ParticleID);
				//ParticleID = -1;
			}
			EndFlag = TRUE;
		}
	}
}












AssaSkill3EnergyShieldDefence::AssaSkill3EnergyShieldDefence()
{
	pChar = NULL;	
	MyCharacterFlag = FALSE;
	ParticleID = -1;
}

AssaSkill3EnergyShieldDefence::~AssaSkill3EnergyShieldDefence()
{
	
}

void AssaSkill3EnergyShieldDefence::Start(smCHAR* character)
{
	pChar = character;
	if(pChar == NULL)
		return;
	if(lpCurPlayer == pChar)
	{
		MyCharacterFlag = TRUE;
	}
	
	POINT3D charPos;
	charPos.x = pChar->pX;
	charPos.y = pChar->pY;
	charPos.z = pChar->pZ;

 	ParticleID = g_NewParticleMgr.Start("Skill3MagicianEngrgyShield1", charPos);
	Max_Time = 70*10;
}


void AssaSkill3EnergyShieldDefence::Main()
{	
	if(ParticleID == -1)
		return;
	
	if(!g_NewParticleMgr.GetRunning(ParticleID))
	{
		ParticleID = -1;
		Time = Max_Time;
		return;
	}

	if(MyCharacterFlag)
	{
		if(lpCurPlayer != pChar)
			pChar = lpCurPlayer;
	}
	
	Posi.x = pChar->pX;
 	Posi.y = pChar->pY;
	Posi.z = pChar->pZ;

	D3DVECTOR charPos;
  	charPos.x = (float)pChar->pX;
	charPos.y = (float)pChar->pY+500.f;
	charPos.z = (float)pChar->pZ;
	g_NewParticleMgr.SetAttachPos(ParticleID, charPos);
	g_NewParticleMgr.SetRendering(ParticleID, pChar->FlagShow);
	
	if(Time > Max_Time-140 ||
	(!MyCharacterFlag && pChar->Flag == 0) ||
	(!pChar->MotionInfo || pChar->MotionInfo->State == CHRMOTION_STATE_DEAD) )
	{
 		Time = Max_Time-140;
		if(ParticleID != -1)
		{
			g_NewParticleMgr.SetStop(ParticleID);
			ParticleID = -1;
		}
	}
}










AssaSkill3SpiritElemental::AssaSkill3SpiritElemental()
{
	PatMesh = NULL;
	CurrentFrame = 0;
	ParticleID = -1;
	ObjBip = NULL;
	ParticleIDExt1 = -1;
	MyCharacterFlag = FALSE;
}

AssaSkill3SpiritElemental::~AssaSkill3SpiritElemental()
{
		
}

void AssaSkill3SpiritElemental::Start(smCHAR *character, int liveCount)
{
	pChar = character;
	if(pChar == NULL)
		return;
	
	if(lpCurPlayer == pChar)
	{
		MyCharacterFlag = TRUE;
	}

    Posi.x = character->pX;
	Posi.y = character->pY;
	Posi.z = character->pZ;
	
 	PatMesh = (smPAT3D *)AssaSearchRes("Mg3_spiritE60.ASE", SMMAT_BLEND_LAMP);
	ParticleID = g_NewParticleMgr.Start("Skill3MagicianSpiritElemental", Posi);
	ParticleIDExt1 = g_NewParticleMgr.Start("Skill3MagicianSpiritElementalExt1", Posi);

	
	smPAT3D* tempPatMesh;

	tempPatMesh = (smPAT3D *)AssaSearchRes("b_Mg3_spiritE60.ASE", SMMAT_BLEND_LAMP);
	if(PatMesh == NULL || tempPatMesh == NULL)
	{
		Max_Time = 0;
		Time = 0;
		return;
	}
	
	ObjBip = tempPatMesh->GetObjectFromName("Bip01");
	if(ObjBip == NULL)
	{
		Max_Time = 0;
		Time = 0;
		return;
	}

	Angle.x = 0;
 	Angle.y = character->Angle.y&ANGCLIP;
 	Angle.z = 0;
	CurrentFrame = 0;
	MaxFrame = 160*60;
	
	//Max_Time = 5000;
	Max_Time = liveCount * 70+150;
	CODE = SKILL_SPIRIT_ELEMENTAL;
	EndFlag = FALSE;
}

void AssaSkill3SpiritElemental::Draw(POINT3D *pPosi , POINT3D *pAngle)
{
  	if(PatMesh == NULL || EndFlag == TRUE)
		return;
	if(pChar)
	{
		if(pChar->FlagShow == 0)
			return;
	}

	PatMesh->Frame = CurrentFrame;
	PatMesh->SetPosi(&Posi, &Angle);
	PatMesh->RenderD3D(pPosi, pAngle);
}

void AssaSkill3SpiritElemental::Main()
{
 	if(PatMesh == NULL || ParticleID == -1 || ObjBip == NULL)
	{
		Time = Max_Time;
		return;
	}
	
 	if(EndFlag)
	{
		if(!g_NewParticleMgr.GetRunning(ParticleID))
		{
			ParticleID = -1;
			
		}

		if(!g_NewParticleMgr.GetRunning(ParticleIDExt1))
		{
			ParticleIDExt1 = -1;
			
		}
		
	}

	if(MyCharacterFlag)
	{
		if(lpCurPlayer != pChar)
			pChar = lpCurPlayer;
	}
	
  	Posi.x = pChar->pX;
	Posi.y = pChar->pY+15000; 
	Posi.z = pChar->pZ;

	CurrentFrame += 40;
	smMATRIX *mWorld;
	AnimObjectTree( ObjBip , CurrentFrame, 0, Angle.y, 0);
	mWorld = &ObjBip->mWorld;
	
	if(ParticleID != -1)
	{
		D3DVECTOR charPos;
		charPos.x = (float)(Posi.x+mWorld->_41);
		charPos.y = (float)(Posi.y+mWorld->_43);
		charPos.z = (float)(Posi.z+mWorld->_42);
		
		g_NewParticleMgr.SetPos(ParticleID, charPos);
		g_NewParticleMgr.SetRendering(ParticleID, pChar->FlagShow);
	}
	if(ParticleIDExt1 != -1)
	{
		D3DVECTOR charPos;
		charPos.x = (float)(Posi.x+mWorld->_41);
		charPos.y = (float)(Posi.y+mWorld->_43);
		charPos.z = (float)(Posi.z+mWorld->_42);
		
		g_NewParticleMgr.SetAttachPos(ParticleIDExt1, charPos);
		g_NewParticleMgr.SetRendering(ParticleIDExt1, pChar->FlagShow);
	}

	if(CurrentFrame > MaxFrame)
	{
		CurrentFrame = 0;
	}
	
	
	if(EndFlag == FALSE)
	{
		if(Time > Max_Time-140 ||
		(!MyCharacterFlag && pChar->Flag == 0) ||
		(!pChar->MotionInfo || pChar->MotionInfo->State == CHRMOTION_STATE_DEAD) ||
		(pChar->OnStageField>=0 && StageField[ pChar->OnStageField ]->State == FIELD_STATE_VILLAGE) )
		{
 			Time = Max_Time-140;
			if(ParticleID != -1)
			{
				g_NewParticleMgr.SetStop(ParticleID);
			}

			if(ParticleIDExt1 != -1)
			{
				g_NewParticleMgr.SetFastStop(ParticleIDExt1);
			}
			EndFlag = TRUE;
		}
	}
}





AssaSkill3DancingSword::AssaSkill3DancingSword()
{
	PatMesh = NULL;
	CurrentFrame = 0;
	MyCharacterFlag = FALSE;
	
	ParticleID1 = -1;
	ParticleID2 = -1;
	ParticleID3 = -1;
	ParticleID4 = -1;

	ObjBip1 = NULL;
	ObjBip2 = NULL;
	ObjBip3 = NULL;

	State = 0;
	memset(&AttackLocalPosi, 0, sizeof(AttackLocalPosi));

	DetectionLength = 0.f;

	Type = 0;
	FrameSpeed = 70;
	FrameSpeedFlag = FALSE;
}

AssaSkill3DancingSword::~AssaSkill3DancingSword()
{
		
}

void AssaSkill3DancingSword::Start(smCHAR *character,int type, int liveCount, int level)
{
	pChar = character;
	if(pChar == NULL)
		return;
	if(lpCurPlayer == pChar)
	{
		MyCharacterFlag = TRUE;
	}
	
	Type = type;
    Posi.x = character->pX;
	Posi.y = character->pY;
	Posi.z = character->pZ;
	
 	PatMesh = (smPAT3D *)AssaSearchRes("mg3_danceSd40_110_125.ASE", SMMAT_BLEND_LAMP);

	
 	ObjBip1 = PatMesh->GetObjectFromName("bip 02");
	ObjBip2 = PatMesh->GetObjectFromName("bip 03");
	ObjBip3 = PatMesh->GetObjectFromName("bip 04");
	ObjBip4 = PatMesh->GetObjectFromName("bip 05");

	if(PatMesh == NULL)
	{
		Max_Time = 0;
		Time = 0;
		return;
	}
	Angle.x = 0;
   	Angle.y = (-character->Angle.y+ANGLE_180)&ANGCLIP;
 	Angle.z = 0;

	Max_Time = 5000;
	//MaxFrame = 160*40;
	CurrentFrame = 0;

	if(Type == 0)
	{
		ParticleID1 = g_NewParticleMgr.Start("Skill3MagicianDancingSwordFire1", Posi);
		ParticleID2 = g_NewParticleMgr.Start("Skill3MagicianDancingSwordFire1", Posi);
		ParticleID3 = g_NewParticleMgr.Start("Skill3MagicianDancingSwordFire1", Posi);
		ParticleID4 = -1;//= g_NewParticleMgr.Start("Skill3MagicianDancingSwordFire2", Posi);
	}
	else if(Type == 1)
	{
		ParticleID1 = g_NewParticleMgr.Start("Skill3MagicianDancingSwordIce1", Posi);
		ParticleID2 = g_NewParticleMgr.Start("Skill3MagicianDancingSwordIce1", Posi);
		ParticleID3 = g_NewParticleMgr.Start("Skill3MagicianDancingSwordIce1", Posi);
	}

	if(ObjBip1 && ObjBip2 && ObjBip3 && ObjBip4)
	{
		smMATRIX *mWorld;
		AnimObjectTree( ObjBip1, CurrentFrame, 0, Angle.y, 0);
		mWorld = &ObjBip1->mWorld;
		
		if(ParticleID1 != -1)
		{
			D3DVECTOR charPos;
			charPos.x = (float)(Posi.x+mWorld->_41);
			charPos.y = (float)(Posi.y+mWorld->_43);
			charPos.z = (float)(Posi.z+mWorld->_42);
			
			g_NewParticleMgr.SetPos(ParticleID1, charPos);
		}

		AnimObjectTree( ObjBip1, CurrentFrame, 0, Angle.y, 0);
		mWorld = &ObjBip2->mWorld;
		
		if(ParticleID2 != -1)
		{
			D3DVECTOR charPos;
			charPos.x = (float)(Posi.x+mWorld->_41);
			charPos.y = (float)(Posi.y+mWorld->_43);
			charPos.z = (float)(Posi.z+mWorld->_42);
			
			g_NewParticleMgr.SetPos(ParticleID2, charPos);
		}

		AnimObjectTree( ObjBip1, CurrentFrame, 0, Angle.y, 0);
		mWorld = &ObjBip3->mWorld;
		
		if(ParticleID3 != -1)
		{
			D3DVECTOR charPos;
			charPos.x = (float)(Posi.x+mWorld->_41);
			charPos.y = (float)(Posi.y+mWorld->_43);
			charPos.z = (float)(Posi.z+mWorld->_42);
			
			g_NewParticleMgr.SetPos(ParticleID3, charPos);
		}
	}
	State = DANCING_MAINTAIN;
	
	Max_Time = liveCount * 70+150;
	CODE = SKILL_DANCING_SWORD;
	EndFlag = FALSE;
	
	if(level >= 1 && level <= 4)
		AttackTime = 5*70;
	else if(level >= 5 && level <= 8)
		AttackTime = 4*70;
	else if(level >= 9)
		AttackTime = 3*70;
	else
		AttackTime = 5*70;

	AttackMonster = NULL;
}

void AssaSkill3DancingSword::Draw(POINT3D *pPosi , POINT3D *pAngle)
{
	if(PatMesh == NULL || EndFlag == TRUE || pChar->FlagShow == 0)
		return;
	if(pChar)
	{
		if(pChar->FlagShow == 0)
			return;
	}

	PatMesh->Frame = CurrentFrame;
	POINT3D curPosi;
	curPosi.x = Posi.x + AttackLocalPosi.x;
	curPosi.y = Posi.y + AttackLocalPosi.y;
	curPosi.z = Posi.z + AttackLocalPosi.z;

 	PatMesh->SetPosi(&curPosi, &Angle);
	//lpD3DDevice->SetRenderState( D3DRENDERSTATE_ZWRITEENABLE, FALSE );
	PatMesh->RenderD3D(pPosi, pAngle);
	//lpD3DDevice->SetRenderState( D3DRENDERSTATE_ZWRITEENABLE, TRUE );
}

void AssaSkill3DancingSword::Main()
{
  	if(PatMesh == NULL)
	{
		Time = Max_Time;
		return;
	}
	
 	if(EndFlag)
	{
		if(!g_NewParticleMgr.GetRunning(ParticleID1))
			ParticleID1 = -1;
		if(!g_NewParticleMgr.GetRunning(ParticleID2))
			ParticleID2 = -1;
		if(!g_NewParticleMgr.GetRunning(ParticleID3))
			ParticleID3 = -1;
		if(!g_NewParticleMgr.GetRunning(ParticleID4))
			ParticleID4 = -1;
	}
	
	if(MyCharacterFlag)
	{
		if(lpCurPlayer != pChar)
			pChar = lpCurPlayer;
	}
	
	if(pChar)
	{
		g_NewParticleMgr.SetRendering(ParticleID1, pChar->FlagShow);
		g_NewParticleMgr.SetRendering(ParticleID2, pChar->FlagShow);
		g_NewParticleMgr.SetRendering(ParticleID3, pChar->FlagShow);
		g_NewParticleMgr.SetRendering(ParticleID4, pChar->FlagShow);
	}

  	if(State == DANCING_MAINTAIN && EndFlag == FALSE)
	{
  		if(CurrentFrame > 160*40)
			CurrentFrame = 0;
		Angle.y = (-pChar->Angle.y+ANGLE_180)&ANGCLIP;
 		CurrentFrame += FrameSpeed;
		Posi.x = pChar->pX;
		Posi.y = pChar->pY+5000; 
		Posi.z = pChar->pZ;
		
  		if( (Time%AttackTime) == 0)
		{
			AttackMonster = FindNearMonster(pChar);
			if(AttackMonster)
			{
 				State = DANCING_ATTACK_START1;
				CurrentFrame = 160*39;

				if(MyCharacterFlag)
				SendDancingSwordDamage(AttackMonster);
			}
		}
		if(FrameSpeedFlag == FALSE)
			FrameSpeed--;
		else
			FrameSpeed++;

  		if(FrameSpeed < 30)
			FrameSpeedFlag = TRUE;
		if(FrameSpeed > 110)
			FrameSpeedFlag = FALSE;
	}
	
 	if(State == DANCING_ATTACK_START1)
	{
		Posi.x = pChar->pX;
		Posi.y = pChar->pY+5000; 
		Posi.z = pChar->pZ;
		
		if(CurrentFrame == 160*39)
		{
			if(AttackMonster)
			{
				AttackDest.x = AttackMonster->pX;
				AttackDest.y = AttackMonster->pY+5000;
				AttackDest.z = AttackMonster->pZ;
				
				GetRadian3D( Posi.x,Posi.y,Posi.z, AttackDest.x,AttackDest.y,AttackDest.z);
 				Angle.y = (-GeoResult_Y+ANGLE_180)&ANGCLIP;
			}
		}
		
 		CurrentFrame += 70;
 		if(CurrentFrame > 160*60)
		{
			if(AttackMonster)
			{
				AttackDest.x = AttackMonster->pX;
				AttackDest.y = AttackMonster->pY+5000;
				AttackDest.z = AttackMonster->pZ;
				
				D3DVECTOR	attractLocation;
				attractLocation.x = (float)AttackDest.x - float(Posi.x);
				attractLocation.y = (float)AttackDest.y - float(Posi.y);
				attractLocation.z = (float)AttackDest.z - float(Posi.z);
				
				D3DVECTOR attractDir = attractLocation;

				//벡터의 길이를 구한다.
				float length;
				length = (float)(attractDir.x*attractDir.x+attractDir.y*attractDir.y+attractDir.z*attractDir.z);
				DetectionLength = length;
				
				GetRadian3D( Posi.x,Posi.y,Posi.z, AttackDest.x,AttackDest.y,AttackDest.z);
 				Angle.y = (-GeoResult_Y+ANGLE_180)&ANGCLIP;
			}
			
			State = DANCING_ATTACK_START2;
			CurrentFrame = 160*58;
		}
	}


 	if(State == DANCING_ATTACK_START2)
	{
		smMATRIX *mWorld;
		AnimObjectTree( ObjBip2, CurrentFrame, 0, Angle.y, 0);
		mWorld = &ObjBip1->mWorld;
		
		POINT3D attackCurrent;
		attackCurrent.x = (Posi.x+mWorld->_41);
		attackCurrent.y = (Posi.y+mWorld->_43);
		attackCurrent.z = (Posi.z+mWorld->_42);
		
		GetRadian3D( attackCurrent.x, attackCurrent.y, attackCurrent.z, AttackDest.x, AttackDest.y, AttackDest.z);
		AttackAngle.x = GeoResult_X;
		AttackAngle.y = GeoResult_Y;
		AttackAngle.z = GeoResult_Z;
		
		AttackLocalPosi.x = 0;
		AttackLocalPosi.y = 0;
		AttackLocalPosi.z = 0;
		
		State = DANCING_ATTACK_PROCESS;
	}
	
     if(State == DANCING_ATTACK_PROCESS)
	{
		GetMoveLocation( 0, 0, fONE*7, AttackAngle.x, AttackAngle.y, 0);
		AttackLocalPosi.x += GeoResult_X;
		AttackLocalPosi.y += GeoResult_Y;
		AttackLocalPosi.z += GeoResult_Z;
		
		float length;
 		length = (float)((float)AttackLocalPosi.x*(float)AttackLocalPosi.x+
			(float)AttackLocalPosi.y*(float)AttackLocalPosi.y+
  			(float)AttackLocalPosi.z*(float)AttackLocalPosi.z);

  		if((length-20*fONE) > DetectionLength)
		{
			State = DANCING_ATTACK_HIT;
			if(Type == 0)
				AssaParticle_FireHit1(&AttackDest);
			else if(Type == 1)
				AssaParticle_IceHit1(&AttackDest);
		}
	}
 
	if(State == DANCING_ATTACK_HIT)
	{
 		if (AttackMonster != NULL)
		{
			AttackMonster = NULL;
			
			State = DANCING_MAINTAIN;
			CurrentFrame = 0;

			memset(&AttackLocalPosi, 0, sizeof(AttackLocalPosi));
		}
		if(ParticleID4 != -1)
		{
			g_NewParticleMgr.SetStop(ParticleID4);
			ParticleID4 = -1;
		}
		FrameSpeed = 70;
	}
	
	if(ObjBip1 && ObjBip2 && ObjBip3 && ObjBip4)
	{
 		smMATRIX *mWorld;
		AnimObjectTree( ObjBip1 , CurrentFrame, 0, Angle.y, 0);
		mWorld = &ObjBip1->mWorld;
		
		if(ParticleID1 != -1)
		{
			D3DVECTOR charPos;
			charPos.x = (float)(Posi.x+mWorld->_41+AttackLocalPosi.x);
			charPos.y = (float)(Posi.y+mWorld->_43+AttackLocalPosi.y);
			charPos.z = (float)(Posi.z+mWorld->_42+AttackLocalPosi.z);
			
			g_NewParticleMgr.SetPos(ParticleID1, charPos);
		}

		AnimObjectTree( ObjBip2 , CurrentFrame, 0, Angle.y, 0);
		mWorld = &ObjBip2->mWorld;
		
		if(ParticleID2 != -1)
		{
			D3DVECTOR charPos;
			charPos.x = (float)(Posi.x+mWorld->_41+AttackLocalPosi.x);
			charPos.y = (float)(Posi.y+mWorld->_43+AttackLocalPosi.y);
			charPos.z = (float)(Posi.z+mWorld->_42+AttackLocalPosi.z);
			
			g_NewParticleMgr.SetPos(ParticleID2, charPos);
		}

		AnimObjectTree( ObjBip3 , CurrentFrame, 0, Angle.y, 0);
		mWorld = &ObjBip3->mWorld;
		
		if(ParticleID3 != -1)
		{
			D3DVECTOR charPos;
			charPos.x = (float)(Posi.x+mWorld->_41+AttackLocalPosi.x);
			charPos.y = (float)(Posi.y+mWorld->_43+AttackLocalPosi.y);
			charPos.z = (float)(Posi.z+mWorld->_42+AttackLocalPosi.z);
			
			g_NewParticleMgr.SetPos(ParticleID3, charPos);
		}

		AnimObjectTree( ObjBip4 , CurrentFrame, 0, Angle.y, 0);
		mWorld = &ObjBip4->mWorld;
		
  		if(ParticleID4 != -1)
		{
			D3DVECTOR charPos;
			charPos.x = (float)(Posi.x+mWorld->_41+AttackLocalPosi.x);
			charPos.y = (float)(Posi.y+mWorld->_43+AttackLocalPosi.y);
			charPos.z = (float)(Posi.z+mWorld->_42+AttackLocalPosi.z);
			
			g_NewParticleMgr.SetPos(ParticleID4, charPos);
		}
	}

	if(EndFlag == FALSE)
	{
		if(Time > Max_Time-140 ||
		(!MyCharacterFlag && pChar->Flag == 0) ||
		(!pChar->MotionInfo || pChar->MotionInfo->State == CHRMOTION_STATE_DEAD) ||
		
		(pChar->OnStageField>=0 && StageField[ pChar->OnStageField ]->State == FIELD_STATE_VILLAGE) )
		{
 			Time = Max_Time-140;
			if(ParticleID1 != -1)
				g_NewParticleMgr.SetStop(ParticleID1);
			if(ParticleID2 != -1)
				g_NewParticleMgr.SetStop(ParticleID2);
			if(ParticleID3 != -1)
				g_NewParticleMgr.SetStop(ParticleID3);
			if(ParticleID4 != -1)
				g_NewParticleMgr.SetStop(ParticleID4);
			
			EndFlag = TRUE;
		}
	}
}





AssaSkill3DivineInhalation::AssaSkill3DivineInhalation()
{
	pChar = NULL;	
	MyCharacterFlag = FALSE;
	ParticleID = -1;
	SearchObj = NULL;
}

AssaSkill3DivineInhalation::~AssaSkill3DivineInhalation()
{
	
}

void AssaSkill3DivineInhalation::Start(smCHAR* character, int liveCount)
{
    pChar = character;
	if(pChar == NULL)
		return;
	if(lpCurPlayer == pChar)
	{
		MyCharacterFlag = TRUE;
	}
	
	POINT3D charPos;
	charPos.x = pChar->pX;
	charPos.y = pChar->pY+10000;
	charPos.z = pChar->pZ;

 	ParticleID = g_NewParticleMgr.Start("Skill3KnightDivineInhalation2", charPos);
	EndFlag = FALSE;
  	Max_Time = liveCount*70+140;
	CODE = SKILL_DIVINE_INHALATION;

	if(pChar->AnimPattern == NULL)
		return;
 	SearchObj = pChar->AnimPattern->GetObjectFromName("Bip01 L Hand"); //오브젝트를 읽는다
}


void AssaSkill3DivineInhalation::Main()
{	
	if(ParticleID == -1)
		return;
	
	if(!g_NewParticleMgr.GetRunning(ParticleID) && EndFlag)
	{
		ParticleID = -1;
		return;
	}

	if(MyCharacterFlag)
	{
		if(lpCurPlayer != pChar)
			pChar = lpCurPlayer;
	}
	
	smMATRIX *mWorld;
    AnimObjectTree( SearchObj, pChar->frame, 0, (-pChar->Angle.y+ANGLE_180)&ANGCLIP, 0);
	mWorld = &SearchObj->mWorld;
	
	
	

	D3DVECTOR charPos;
	charPos.x = (float)pChar->pX+mWorld->_41;
	charPos.y = (float)pChar->pY+mWorld->_43;
	charPos.z = (float)pChar->pZ+mWorld->_42;

	g_NewParticleMgr.SetAttachPos(ParticleID, charPos);
	//g_NewParticleMgr.SetRendering(ParticleID, pChar->FlagShow);
	if(pChar->HvLeftHand.dwItemCode == 0 || pChar->FlagShow == 0)
		g_NewParticleMgr.SetRendering(ParticleID, FALSE);
	else
		g_NewParticleMgr.SetRendering(ParticleID, TRUE);

	
  	if(EndFlag == FALSE)
	{
		if(Time > Max_Time-140 ||
		(!MyCharacterFlag && pChar->Flag == 0) ||
		(!pChar->MotionInfo || pChar->MotionInfo->State == CHRMOTION_STATE_DEAD) ||
		
	    //((pChar->HvLeftHand.dwItemCode & sinITEM_MASK2)!=sinDS1 ) ||
		!(pChar->dwDispAppSkill&SKILL_APPMASK_SPARK_SHIELD) ||
		(pChar->OnStageField>=0 && StageField[ pChar->OnStageField ]->State == FIELD_STATE_VILLAGE) )   //방패가 아닌 다른것을 들었을 경우
		{
 			Time = Max_Time-140;
			if(ParticleID != -1)
			{
				g_NewParticleMgr.SetFastStop(ParticleID);
				//ParticleID = -1;
			}
			EndFlag = TRUE;
		}
	}
}







AssaSkill3HolyIncantationShot::AssaSkill3HolyIncantationShot()
{
	ParticleID = -1;
	MyCharacterFlag = FALSE;
}

AssaSkill3HolyIncantationShot::~AssaSkill3HolyIncantationShot()
{

}

void AssaSkill3HolyIncantationShot::Start(smCHAR *character)
{
	pChar = character;
	POINT3D charPos;
	charPos.x = pChar->pX;
	charPos.y = pChar->pY;
	charPos.z = pChar->pZ;

	ParticleID = g_NewParticleMgr.Start("Skill3KnightHolyIncantation1", charPos);
	Max_Time = 5000;
	
	if(lpCurPlayer == pChar)
	{
		MyCharacterFlag = TRUE;
	}

}

void AssaSkill3HolyIncantationShot::Main()
{
	if(ParticleID != -1) 
	{
		if(!g_NewParticleMgr.GetRunning(ParticleID))
		{
			Time = Max_Time;
			ParticleID = -1;
		}
		
		D3DVECTOR charPos;
		charPos.x = (float)pChar->pX;
		charPos.y = (float)pChar->pY;
		charPos.z = (float)pChar->pZ;
		g_NewParticleMgr.SetAttachPos(ParticleID, charPos);
	}
}


AssaSkill3HolyIncantationPart::AssaSkill3HolyIncantationPart()
{
	pChar = NULL;	
	MyCharacterFlag = FALSE;
	ParticleID = -1;
}

AssaSkill3HolyIncantationPart::~AssaSkill3HolyIncantationPart()
{

}

void AssaSkill3HolyIncantationPart::Start(smCHAR* character, int liveCount)
{
 	pChar = character;
 	if(pChar == NULL)
		return;
	if(lpCurPlayer == pChar)
	{
		MyCharacterFlag = TRUE;
	}
	
	POINT3D charPos;
	charPos.x = pChar->pX;
	charPos.y = pChar->pY+pChar->PatHeight;
	charPos.z = pChar->pZ;

 	ParticleID = g_NewParticleMgr.Start("Skill3KnightHolyIncantation2", charPos);
	

	Max_Time = liveCount*70+140;
	CODE = SKILL_HOLY_INCANTATION;
	EndFlag = FALSE;

}

void AssaSkill3HolyIncantationPart::Main()
{
	if(ParticleID == -1)
		return;
	
	if(!g_NewParticleMgr.GetRunning(ParticleID) && EndFlag)
		ParticleID = -1;
	
	if(MyCharacterFlag)
	{
		if(lpCurPlayer != pChar)
			pChar = lpCurPlayer;
	}

	if(ParticleID != -1)
	{
		D3DVECTOR charPos;
 		charPos.x = (float)pChar->pX;
   		charPos.y = float(pChar->pY+pChar->PatHeight+26*fONE);
		charPos.z = (float)pChar->pZ;
		
		g_NewParticleMgr.SetAttachPos(ParticleID, charPos);
	}
	
	if(EndFlag == FALSE)
	{
		if(Time > Max_Time-140 ||
		(!MyCharacterFlag && pChar->Flag == 0) ||
		(!pChar->MotionInfo || pChar->MotionInfo->State == CHRMOTION_STATE_DEAD) ||
		(pChar->OnStageField>=0 && StageField[ pChar->OnStageField ]->State == FIELD_STATE_VILLAGE) )
		{
 			Time = Max_Time-140;
			if(ParticleID != -1)
			{
				g_NewParticleMgr.SetFastStop(ParticleID);
				//ParticleID = -1;
			}
			EndFlag = TRUE;
		}
	}

}




AssaSkill3HolyIncantationPartHand::AssaSkill3HolyIncantationPartHand()
{
	pChar = NULL;	
	MyCharacterFlag = FALSE;
	ParticleID = -1;
	SearchObj = NULL;
}

AssaSkill3HolyIncantationPartHand::~AssaSkill3HolyIncantationPartHand()
{
	
}

void AssaSkill3HolyIncantationPartHand::Start(smCHAR* character, int liveCount)
{
    pChar = character;
	if(pChar == NULL)
		return;
	if(lpCurPlayer == pChar)
	{
		MyCharacterFlag = TRUE;
	}
	
	POINT3D charPos;
	charPos.x = pChar->pX;
	charPos.y = pChar->pY+10000;
	charPos.z = pChar->pZ;

 	ParticleID = g_NewParticleMgr.Start("Skill3KnightHolyIncantationHand1", charPos);
	ParticleIDExt = g_NewParticleMgr.Start("Skill3KnightHolyIncantationHand2", charPos);
	EndFlag = FALSE;
  	Max_Time = liveCount*70+140;
	
	if(pChar->AnimPattern == NULL)
		return;
 	SearchObj = pChar->AnimPattern->GetObjectFromName("Bip01 L Hand"); //오브젝트를 읽는다
}


void AssaSkill3HolyIncantationPartHand::Main()
{	
	if(ParticleID == -1)
		return;
	
	if(!g_NewParticleMgr.GetRunning(ParticleID) && EndFlag)
	{
		ParticleID = -1;
		return;
	}

	if(MyCharacterFlag)
	{
		if(lpCurPlayer != pChar)
			pChar = lpCurPlayer;
	}
	
	smMATRIX *mWorld;
    AnimObjectTree( SearchObj, pChar->frame, 0, (-pChar->Angle.y+ANGLE_180)&ANGCLIP, 0);
	mWorld = &SearchObj->mWorld;
	
	D3DVECTOR charPos;
	charPos.x = (float)pChar->pX+mWorld->_41;
	charPos.y = (float)pChar->pY+mWorld->_43;
	charPos.z = (float)pChar->pZ+mWorld->_42;

 	g_NewParticleMgr.SetAttachPos(ParticleID, charPos);
	g_NewParticleMgr.SetRendering(ParticleID, pChar->FlagShow);

	if(ParticleIDExt != -1)
	{
		g_NewParticleMgr.SetPos(ParticleIDExt, charPos);
		g_NewParticleMgr.SetRendering(ParticleIDExt, pChar->FlagShow);
	}
	
 	if(EndFlag == FALSE)
	{
		if(Time > Max_Time-140 ||
		(!MyCharacterFlag && pChar->Flag == 0) ||
		(!pChar->MotionInfo || pChar->MotionInfo->State == CHRMOTION_STATE_DEAD))
		
		{
 			Time = Max_Time-140;
			if(ParticleID != -1)
			{
				g_NewParticleMgr.SetFastStop(ParticleID);
			}
			
			if(ParticleIDExt != -1)
			{
				g_NewParticleMgr.SetStop(ParticleIDExt);
				ParticleIDExt = -1;
			}

			EndFlag = TRUE;
		}
	}
}










AssaSkill3FlameBrandish::AssaSkill3FlameBrandish()
{
	PatMesh = NULL;
	CurrentFrame = 0;

	for(int index = 0; index < 7; index++)
	{
		ParticleID[index] = -1;
		ObjBip[index] = NULL;
	}
}

AssaSkill3FlameBrandish::~AssaSkill3FlameBrandish()
{
		
}

void AssaSkill3FlameBrandish::Start(smCHAR *character)
{
     Angle.y = (-character->Angle.y)&ANGCLIP;
  	GetMoveLocation( 0, 0, 55*fONE, 0, -Angle.y, 0);

	//velocity.x += ((float)GeoResult_X*2.f/(float)fONE);
	//velocity.z += ((float)GeoResult_Z*2.f/(float)fONE);

     Posi.x = character->pX+GeoResult_X;
  	Posi.y = character->pY+50;
	Posi.z = character->pZ+GeoResult_Z;
	PatMesh = (smPAT3D *)AssaSearchRes("Nt3_FlameB36.ASE", SMMAT_BLEND_LAMP);


   	ObjBip[0] = PatMesh->GetObjectFromName("B01");
	ObjBip[1] = PatMesh->GetObjectFromName("B02");
	ObjBip[2] = PatMesh->GetObjectFromName("B03");
	ObjBip[3] = PatMesh->GetObjectFromName("B04");
	ObjBip[4] = PatMesh->GetObjectFromName("B05");
	ObjBip[5] = PatMesh->GetObjectFromName("B06");
	ObjBip[6] = PatMesh->GetObjectFromName("B07");
	
	if(PatMesh == NULL || ObjBip[1] == NULL ||
	   ObjBip[2] == NULL || ObjBip[3] == NULL ||
	   ObjBip[4] == NULL || ObjBip[5] == NULL ||
	   ObjBip[6] == NULL)
	{
		Time = Max_Time;
		return;
	}
	
	for(int index = 0; index < 7; index++)
	{
		//ParticleID[index] = g_NewParticleMgr.Start("Skill3ArcherBombShot1", Posi);
		ParticleID[index] = g_NewParticleMgr.Start("Skill3KnightFlameBrandish", Posi);
	}
	
  	Max_Time = 5000;
	MaxFrame = 160*31;
	
	PatMesh->Frame = 0;
 	CurrentFrame = 0;
}

void AssaSkill3FlameBrandish::Draw(POINT3D *pPosi , POINT3D *pAngle)
{
	if(PatMesh == NULL)
		return;
	PatMesh->Frame = CurrentFrame;
	PatMesh->SetPosi(&Posi, &Angle);
	PatMesh->RenderD3D(pPosi, pAngle);
}

void AssaSkill3FlameBrandish::Main()
{
	if(Time == Max_Time)
 		return;
	
	if(CurrentFrame > MaxFrame)
 	{
		CurrentFrame = MaxFrame;
		Time = Max_Time;
 
		for(int index = 0; index < 7; index++)
		{
			if(ParticleID[index] != -1)
			{
				g_NewParticleMgr.SetStop(ParticleID[index]);
				ParticleID[index] = -1;
			}
		}
  		return;
	}

    CurrentFrame += 100;
	smMATRIX *mWorld;
 	
   	for(int index = 0; index < 7; index++)
	{
		AnimObjectTree( ObjBip[index], CurrentFrame, 0, Angle.y, 0);
		mWorld = &ObjBip[index]->mWorld;
	
 		if(ParticleID[index] != -1)
		{
			D3DVECTOR charPos;
			charPos.x = (float)(Posi.x+mWorld->_41);
			charPos.y = (float)(Posi.y+mWorld->_43);
			charPos.z = (float)(Posi.z+mWorld->_42);
			
			g_NewParticleMgr.SetPos(ParticleID[index], charPos);
		}
	}
}




AssaSkill3VigorBall::AssaSkill3VigorBall()
{
	DestChar = NULL;
	ParticleID = -1;
	ParticleIDExt1 = -1;
	memset(&Velocity, 0, sizeof(Velocity));
}

AssaSkill3VigorBall::~AssaSkill3VigorBall()
{
	
}

void AssaSkill3VigorBall::Main()
{
	if(Time < 0)
		return;
	if(ParticleID == -1)
		return;
	
	D3DVECTOR destPos;
  	destPos.x = (float)DestChar->pX;
	destPos.y = float(DestChar->pY+DestChar->PatHeight/2);
	destPos.z = (float)DestChar->pZ;
	
	D3DVECTOR term;
	term.x = float((destPos.x - Pos.x)/ fONE);
	term.y = float((destPos.y - Pos.y)/ fONE);
	term.z = float((destPos.z - Pos.z)/ fONE);
		
  	float length = term.x*term.x + term.y*term.y + term.z*term.z;
	length = (float)sqrt(length);
	if(length < 15 || Time > 100) //충) //충돌을 했을 경우..
	{
		if(ParticleID != -1)
		{
			g_NewParticleMgr.SetStop(ParticleID);
			ParticleID = -1;
			Time = Max_Time;
		}

		if(ParticleIDExt1 != -1)
		{
 			g_NewParticleMgr.SetFastStop(ParticleIDExt1);
			g_NewParticleMgr.SetStop(ParticleIDExt1);
			ParticleIDExt1 = -1;
			Time = Max_Time;
			
			POINT3D partPos;
			partPos.x = (int)Pos.x;
			partPos.y = (int)Pos.y;
			partPos.z = (int)Pos.z;

			g_NewParticleMgr.Start("Skill3PriestessVigorBallHit1", partPos); 
			
		}
	}
	
	D3DVECTOR velocity;
	velocity.x = term.x/length;
	velocity.y = term.y/length;
	velocity.z = term.z/length;
		
 	Velocity.x += velocity.x;
	Velocity.y += velocity.y;
	Velocity.z += velocity.z;
 	if(length < 100)
	{
		Velocity.x = Velocity.x * 0.85f;
		Velocity.y = Velocity.y * 0.85f;
		Velocity.z = Velocity.z * 0.85f;
	}
	
  	length = (float)sqrt(Velocity.x*Velocity.x+Velocity.y*Velocity.y+Velocity.z*Velocity.z);
	if(length > 10)
	{
	 	Velocity.x = Velocity.x * 0.9f;
		Velocity.y = Velocity.y * 0.9f;
		Velocity.z = Velocity.z * 0.9f;
	}

  	Pos.x += Velocity.x * fONE;
	Pos.y += Velocity.y * fONE;
	Pos.z += Velocity.z * fONE;
	
	
  	if(ParticleID != -1)
	{
		g_NewParticleMgr.SetPos(ParticleID, Pos);
	}
	

  	if(ParticleIDExt1 != -1)
	{
		g_NewParticleMgr.SetAttachPos(ParticleIDExt1, Pos);
	}

}

void AssaSkill3VigorBall::Start(POINT3D *pCur, smCHAR *destChar, D3DVECTOR *velocity, int delay)
{
 	Pos.x = (float)pCur->x;
	Pos.y = (float)pCur->y;
	Pos.z = (float)pCur->z;

	DestChar = destChar;

	if(velocity != NULL)
	{
		Velocity.x = velocity->x;
		Velocity.y = velocity->y;
		Velocity.z = velocity->z;
	}
	POINT3D pos;
	pos.x = (int)Pos.x;
	pos.y = (int)Pos.y;
	pos.z = (int)Pos.z;
 	ParticleID = g_NewParticleMgr.Start("Skill3PriestessVigorBall1", pos);
 	ParticleIDExt1 = g_NewParticleMgr.Start("Skill3PriestessVigorBall2", pos);
	
	Max_Time = 2000;
	Time = -delay;
}

AssaSkill3ResurrectionShot::AssaSkill3ResurrectionShot()
{
	PatMesh = NULL;
	CurrentFrame = 0;
	ParticleID = -1;
	ObjBip = NULL;
	TimeCount = 0;
	Delay = 0;
	EndFlag = FALSE;

	MyCharacterFlag = FALSE;
}

AssaSkill3ResurrectionShot::~AssaSkill3ResurrectionShot()
{
		
}

void AssaSkill3ResurrectionShot::Start(smCHAR *character, int delay)
{
	
 	pChar = character;
	if(pChar == lpCurPlayer)
		MyCharacterFlag = TRUE;

 	Posi.x = pChar->pX;
	Posi.y = pChar->pY;
	Posi.z = pChar->pZ;
	ParticleID = g_NewParticleMgr.Start("Skill3PriestessResurrection1", Posi);
	ParticleIDExt1 = g_NewParticleMgr.Start("Skill3PriestessResurrection2", Posi);
	
	
 	PatMesh = (smPAT3D *)AssaSearchRes("Pt3_Resurrection60.ASE", SMMAT_BLEND_LAMP);
	smPAT3D *PatBoneMesh = (smPAT3D *)AssaSearchRes("b_Pt3_Resurrection60.ASE", SMMAT_BLEND_LAMP);
	ObjBip = PatBoneMesh->GetObjectFromName("Bip01 Head");
	
	if(PatMesh == NULL || ObjBip == NULL)
	{
		return;
	}
	

   	PatMesh->Frame = 0;
	CurrentFrame = 0;
	Delay = delay;
	MaxFrame = 90*159;
	Max_Time = 5000;
	EndFlag = FALSE;
	MeshBlendStep = -255;
}


void AssaSkill3ResurrectionShot::Draw(POINT3D *pPosi , POINT3D *pAngle)
{
	
 	if(PatMesh == NULL || Time < Delay)
		return;
	POINT3D meshPos;
	meshPos.x = Posi.x;
	meshPos.y = Posi.y+8500;
	meshPos.z = Posi.z;

	PatMesh->Frame = CurrentFrame;
	PatMesh->SetPosi(&meshPos, &Angle);
	int r,g,b,a;
	r = smRender.Color_R;
	g = smRender.Color_G;
	b = smRender.Color_B;
	a = smRender.Color_A;

	smRender.Color_R = r+MeshBlendStep;
	smRender.Color_G = g+MeshBlendStep;
	smRender.Color_B = b+MeshBlendStep;
	smRender.Color_A = a+MeshBlendStep;

	PatMesh->RenderD3D(pPosi, pAngle);

	smRender.Color_R = r+MeshBlendStep;
	smRender.Color_G = g+MeshBlendStep;
	smRender.Color_B = b+MeshBlendStep;
	smRender.Color_A = a+MeshBlendStep;
	

}

void AssaSkill3ResurrectionShot::Main()
{
	if(!g_NewParticleMgr.GetRunning(ParticleID))
		ParticleID = -1;
	
	if(MyCharacterFlag)
	{
		 if(lpCurPlayer != pChar)
			pChar = lpCurPlayer;
	}
	
	Posi.x = pChar->pX;
	Posi.y = pChar->pY;
	Posi.z = pChar->pZ;
	
	if(Time > Delay)
	{
   		if(CurrentFrame < 60*5)
			MeshBlendStep += 255/5;
 		if(CurrentFrame > MaxFrame-60*40)
			MeshBlendStep -= 255/30;

    	if(CurrentFrame > MaxFrame-60*40 && EndFlag == FALSE)
 		{
 			if(ParticleIDExt1 != -1)
				g_NewParticleMgr.SetFastStop(ParticleIDExt1);
			EndFlag = TRUE;
		}

		if(!g_NewParticleMgr.GetRunning(ParticleIDExt1) && EndFlag)
		{
			ParticleIDExt1 = -1;
			CurrentFrame = MaxFrame;
			Time = Max_Time;
		}

		CurrentFrame += 60;

		
		smMATRIX *mWorld;
 		AnimObjectTree( ObjBip , CurrentFrame, 0, Angle.y, 0);
		mWorld = &ObjBip->mWorld;
		
		if(ParticleIDExt1 != -1)
		{
			D3DVECTOR charPos;
			charPos.x = (float)(Posi.x+mWorld->_41);
			charPos.y = (float)(Posi.y+mWorld->_43+8500);
			charPos.z = (float)(Posi.z+mWorld->_42);
			
			g_NewParticleMgr.SetAttachPos(ParticleIDExt1, charPos);
		}
		
	}
	

	if(ParticleID != -1)
	{
		D3DVECTOR charPos;
		charPos.x = (float)(Posi.x);
		charPos.y = (float)(Posi.y);
		charPos.z = (float)(Posi.z);
		
		g_NewParticleMgr.SetAttachPos(ParticleID, charPos);
	}
}

AssaSkill3ResurrectionPart::AssaSkill3ResurrectionPart()
{
	pChar = NULL;	
	MyCharacterFlag = FALSE;
	ParticleID = -1;
}

AssaSkill3ResurrectionPart::~AssaSkill3ResurrectionPart()
{

}

void AssaSkill3ResurrectionPart::Start(smCHAR* character, int liveCount)
{
  	pChar = character;
 	if(pChar == NULL)
		return;
	if(lpCurPlayer == pChar)
	{
		MyCharacterFlag = TRUE;
	}
	
	POINT3D charPos;
	charPos.x = pChar->pX;
	charPos.y = pChar->pY+pChar->PatHeight;
	charPos.z = pChar->pZ;

 	ParticleID = g_NewParticleMgr.Start("Skill3PriestessResurrection3", charPos);
	

	Max_Time = liveCount*70+140;
	CODE = SKILL_RESURRECTION;
	EndFlag = FALSE;

}

void AssaSkill3ResurrectionPart::Main()
{
 	if(ParticleID == -1)
		return;
	
	if(!g_NewParticleMgr.GetRunning(ParticleID) && EndFlag)
		ParticleID = -1;
	
	if(MyCharacterFlag)
	{
		if(lpCurPlayer != pChar)
			pChar = lpCurPlayer;
	}

  	if(ParticleID != -1)
	{
		D3DVECTOR charPos;
  		charPos.x = (float)pChar->pX;
		charPos.y = float(pChar->pY+pChar->PatHeight+fONE*30);
		charPos.z = (float)pChar->pZ;
		
		g_NewParticleMgr.SetAttachPos(ParticleID, charPos);
		g_NewParticleMgr.SetRendering(ParticleID, pChar->FlagShow);
	}
	
	if(EndFlag == FALSE)
	{
		if(Time > Max_Time-140)
		{
 			Time = Max_Time-140;
			if(ParticleID != -1)
			{
				g_NewParticleMgr.SetFastStop(ParticleID);
			}
			EndFlag = TRUE;
		}
	}

}





AssaSkill3Extinction::AssaSkill3Extinction()
{
	pChar = NULL;	
	MyCharacterFlag = FALSE;
	ParticleID = -1;
}

AssaSkill3Extinction::~AssaSkill3Extinction()
{

}

void AssaSkill3Extinction::Start(smCHAR* character)
{
  	pChar = character;
 	if(pChar == NULL)
		return;
	if(lpCurPlayer == pChar)
	{
		MyCharacterFlag = TRUE;
	}
	
	POINT3D charPos;
	charPos.x = pChar->pX;
	charPos.y = pChar->pY+pChar->PatHeight;
	charPos.z = pChar->pZ;

 	ParticleID = g_NewParticleMgr.Start("Skill3PriestessExtinction2", charPos);
	

	Max_Time = 400;
}

void AssaSkill3Extinction::Main()
{
 	if(ParticleID == -1)
		return;
	
	if(!g_NewParticleMgr.GetRunning(ParticleID))
	{
		ParticleID = -1;
		Time = Max_Time;
	}

	if(MyCharacterFlag)
	{
		if(lpCurPlayer != pChar)
			pChar = lpCurPlayer;
	}

  	if(ParticleID != -1)
	{
		D3DVECTOR charPos;
 		charPos.x = (float)pChar->pX;
		charPos.y = float(pChar->pY+1500);
		charPos.z = (float)pChar->pZ;
		
		g_NewParticleMgr.SetAttachPos(ParticleID, charPos);
	}
}



AssaSkillVirtualLifeMember::AssaSkillVirtualLifeMember()
{
	ParticleID = -1;
	MyCharacterFlag = FALSE;
}

AssaSkillVirtualLifeMember::~AssaSkillVirtualLifeMember()
{
		
}

void AssaSkillVirtualLifeMember::Start(smCHAR *character, int liveCount)
{
	pChar = character;
	if(pChar == NULL)
		return;
	if(lpCurPlayer == pChar)
	{
		MyCharacterFlag = TRUE;
	}

    Posi.x = pChar->pX;
	Posi.y = pChar->pY;
	Posi.z = pChar->pZ;
	
	ParticleID = g_NewParticleMgr.Start("Skill3PriestessVirtualLifeMember", Posi);
	

	Max_Time = liveCount*70+140;
	CODE = SKILL_VIRTUAL_LIFE;
	EndFlag = FALSE;
}



void AssaSkillVirtualLifeMember::Main()
{
	if(Time == Max_Time)
 		return;
	
	if(!g_NewParticleMgr.GetRunning(ParticleID) && EndFlag)
		ParticleID = -1;
	
	if(MyCharacterFlag)
	{
		if(lpCurPlayer != pChar)
			pChar = lpCurPlayer;
	}
	
	if(ParticleID != -1)
	{
		if(pChar)
		{
			D3DVECTOR charPos;
			charPos.x = (float)(pChar->pX);
 			charPos.y = (float)(pChar->pY+1000);
			charPos.z = (float)(pChar->pZ);
			g_NewParticleMgr.SetAttachPos(ParticleID, charPos);
			g_NewParticleMgr.SetRendering(ParticleID, pChar->FlagShow);
		}
	}

	if(EndFlag == FALSE)
	{
		if(Time > Max_Time-140 ||
		(!MyCharacterFlag && pChar->Flag == 0) ||
		(!pChar->MotionInfo || pChar->MotionInfo->State == CHRMOTION_STATE_DEAD) ||
		(pChar->OnStageField>=0 && StageField[ pChar->OnStageField ]->State == FIELD_STATE_VILLAGE))
		{
 			Time = Max_Time-140;
			if(ParticleID != -1)
			{
				g_NewParticleMgr.SetFastStop(ParticleID);
				//ParticleID = -1;
			}
			EndFlag = TRUE;
		}
	}
}





AssaSkillTriumphOfValhallaPart::AssaSkillTriumphOfValhallaPart()
{
	ParticleID = -1;
	ParticleIDExt = -1;
	MyCharacterFlag = FALSE;
	ObjBip = NULL;
	ObjBipExt = NULL;
}

AssaSkillTriumphOfValhallaPart::~AssaSkillTriumphOfValhallaPart()
{
		
}

void AssaSkillTriumphOfValhallaPart::Start(smCHAR *character, int liveCount)
{
	pChar = character;
	if(pChar == NULL)
		return;
	if(lpCurPlayer == pChar)
	{
		MyCharacterFlag = TRUE;
	}

    Posi.x = pChar->pX;
	Posi.y = pChar->pY+10000;
	Posi.z = pChar->pZ;
	
	ParticleID = g_NewParticleMgr.Start("Skill3AtalantaTriumphOfValhalla", Posi);
	Max_Time = 2000;

	ObjBip = pChar->AnimPattern->GetObjectFromName("Bip01 R Hand");


	Max_Time = liveCount*70+150;
	CODE = SKILL_TRIUMPH_OF_VALHALLA;
	EndFlag = FALSE;
}

void AssaSkillTriumphOfValhallaPart::StartHallOfValhalla(smCHAR *character, int liveCount)
{
	pChar = character;
	if(pChar == NULL)
		return;
	if(lpCurPlayer == pChar)
	{
		MyCharacterFlag = TRUE;
	}

    Posi.x = pChar->pX;
	Posi.y = pChar->pY+10000;
	Posi.z = pChar->pZ;
	
	ParticleID = g_NewParticleMgr.Start("Skill3AtalantaTriumphOfValhalla", Posi);
	ParticleIDExt = g_NewParticleMgr.Start("Skill3AtalantaTriumphOfValhalla", Posi);
	Max_Time = 2000;

	if( pChar->AnimPattern != NULL ) // NULL 값일때 이 밑에 함수를 호출시켜서 GM모드에서 field 명령어로 이동시 뻑남 // 해외
	{
		ObjBip    = pChar->AnimPattern->GetObjectFromName("Bip01 R Hand");
		ObjBipExt = pChar->AnimPattern->GetObjectFromName("Bip01 R Forearm");
	}

	Max_Time = liveCount*70+150;
	CODE = SKILL_HALL_OF_VALHALLA;
	EndFlag = FALSE;
}

void AssaSkillTriumphOfValhallaPart::Main()
{
	if(Time == Max_Time)
 		return;
	
	if(!g_NewParticleMgr.GetRunning(ParticleID) && EndFlag)
	{
		ParticleID = -1;
		ParticleIDExt = -1;
		return;
	}

	if(MyCharacterFlag)
	{
		if(lpCurPlayer != pChar)
			pChar = lpCurPlayer;
	}

    if(ParticleID != -1)
	{
		if(ObjBip != NULL)
		{
			smMATRIX *mWorld;
			
			AnimObjectTree( ObjBip , pChar->frame, 0, (-pChar->Angle.y+ANGLE_180)&ANGCLIP, 0);
			mWorld = &ObjBip->mWorld;
	
			if(ParticleID != -1)
			{
				D3DVECTOR charPos;
				charPos.x = (float)(pChar->pX+mWorld->_41);
				charPos.y = (float)(pChar->pY+mWorld->_43);
				charPos.z = (float)(pChar->pZ+mWorld->_42);
				
				g_NewParticleMgr.SetAttachPos(ParticleID, charPos);
				g_NewParticleMgr.SetRendering(ParticleID, pChar->FlagShow);
			}
		}
	}

	if(ParticleIDExt != -1)
	{
		if(ObjBipExt != NULL)
		{
			smMATRIX *mWorld;
			
			AnimObjectTree( ObjBipExt , pChar->frame, 0, (-pChar->Angle.y+ANGLE_180)&ANGCLIP, 0);
			mWorld = &ObjBip->mWorld;
	
			if(ParticleID != -1)
			{
				D3DVECTOR charPos;
				charPos.x = (float)(pChar->pX+mWorld->_41);
				charPos.y = (float)(pChar->pY+mWorld->_43);
				charPos.z = (float)(pChar->pZ+mWorld->_42);
				
				g_NewParticleMgr.SetAttachPos(ParticleIDExt, charPos);
				g_NewParticleMgr.SetRendering(ParticleIDExt, pChar->FlagShow);
			}
		}
	}

	if(EndFlag == FALSE)
	{
		if(Time > Max_Time-140 ||
		(!MyCharacterFlag && pChar->Flag == 0) ||
		(!pChar->MotionInfo || pChar->MotionInfo->State == CHRMOTION_STATE_DEAD) ||
		(pChar->OnStageField>=0 && StageField[ pChar->OnStageField ]->State == FIELD_STATE_VILLAGE))
		{
 			Time = Max_Time-140;
			if(ParticleID != -1)
			{
				g_NewParticleMgr.SetFastStop(ParticleID);
				//ParticleID = -1;
			}

			if(ParticleIDExt != -1)
				g_NewParticleMgr.SetFastStop(ParticleIDExt);

			EndFlag = TRUE;
		}
	}
}

AssaSkillTriumphOfValhallaShot::AssaSkillTriumphOfValhallaShot()
{
	DestChar = NULL;
	ParticleID = -1;
	//ParticleIDExt1 = -1;
	memset(&Velocity, 0, sizeof(Velocity));
	LiveCount = 0;
	SearchObj = NULL;
}

AssaSkillTriumphOfValhallaShot::~AssaSkillTriumphOfValhallaShot()
{
	
}

void AssaSkillTriumphOfValhallaShot::Main()
{
	smMATRIX *mWorld;
    AnimObjectTree( SearchObj, DestChar->frame, 0, (-DestChar->Angle.y+ANGLE_180)&ANGCLIP, 0);
	mWorld = &SearchObj->mWorld;
	
	D3DVECTOR destPos;
	destPos.x = float(DestChar->pX+mWorld->_41);
	destPos.y = float(DestChar->pY+mWorld->_43);
	destPos.z = float(DestChar->pZ+mWorld->_42);

	D3DVECTOR term;
	term.x = float((destPos.x - Pos.x)/ fONE);
	term.y = float((destPos.y - Pos.y)/ fONE);
	term.z = float((destPos.z - Pos.z)/ fONE);
		
  	float length = term.x*term.x + term.y*term.y + term.z*term.z;
	length = (float)sqrt(length);
	if(length < 15 || (!DestChar->MotionInfo || DestChar->MotionInfo->State == CHRMOTION_STATE_DEAD) ||
	   Time > 100) //충돌을 했거나 시간이 너무 지났을 경우.
	{
		if(ParticleID != -1)
		{
			g_NewParticleMgr.SetRendering(ParticleID, FALSE);
			g_NewParticleMgr.SetStop(ParticleID);
			g_NewParticleMgr.SetFastStop(ParticleID);
			ParticleID = -1;
		}
		AssaParticle_TriumphOfValhalla(DestChar, LiveCount);
		Time = Max_Time;
	}
	
	D3DVECTOR velocity;
	velocity.x = term.x/length;
	velocity.y = term.y/length;
	velocity.z = term.z/length;
		
	Velocity.x += velocity.x;
	Velocity.y += velocity.y;
	Velocity.z += velocity.z;
	
	
	if(length < 100)
	{
		Velocity.x = Velocity.x * 0.8f;
		Velocity.y = Velocity.y * 0.8f;
		Velocity.z = Velocity.z * 0.8f;
	}
	
  	Pos.x += Velocity.x * fONE;
	Pos.y += Velocity.y * fONE;
	Pos.z += Velocity.z * fONE;
	
	POINT3D pos;
	pos.x = (int)Pos.x;
	pos.y = (int)Pos.y;
	pos.z = (int)Pos.z;

	cAssaTrace->AddData(&pos);			
  	if(ParticleID != -1)
	{
		g_NewParticleMgr.SetAttachPos(ParticleID, Pos);
	}

}

void AssaSkillTriumphOfValhallaShot::Draw(POINT3D *pPosi , POINT3D *pAngle)
{
	if(!Face.MatNum)return;
	smRender.Init(); //데이터 초기화 
	smMaterialGroup->smMaterial[Face.MatNum].TwoSide = TRUE; //카메라 회전과 상관없이 보이기 
	smRender.SetMaterialGroup(smMaterialGroup);  //읽어드린 메트리얼을 연결 
	Face.width = 2000;
	if(!AssaAddFaceTrace(&Face, cAssaTrace))
			return;
	
	Face.width = 8000;
	if(!AssaAddFaceTrace(&Face, cAssaTrace))
			return;

	smRender.SetClipStates(SMCLIP_LEFT | SMCLIP_RIGHT | SMCLIP_TOP | SMCLIP_BOTTOM);
	smRender.ClipRendFace();
	smRender.GeomVertex2D(FALSE);

 	//lpD3DDevice->SetRenderState( D3DRENDERSTATE_ZWRITEENABLE, FALSE );
  	smRender.RenderD3D();
}

void AssaSkillTriumphOfValhallaShot::Start(smCHAR *curChar, smCHAR *destChar, D3DVECTOR *velocity, int liveCount)
{
	
	int *matNum = (int *)AssaSearchRes("shield1.bmp", SMMAT_BLEND_LAMP);
 	if(matNum == NULL)
		return;

	SearchObj = destChar->AnimPattern->GetObjectFromName("Bip01 R Hand"); //오브젝트를 읽는다

  	Pos.x = (float)curChar->pX;
	Pos.y = (float)curChar->pY+10000;
	Pos.z = (float)curChar->pZ;

	DestChar = destChar;

	if(velocity != NULL)
	{
		Velocity.x = velocity->x;
		Velocity.y = velocity->y;
		Velocity.z = velocity->z;
	}
	POINT3D pos;
	pos.x = (int)Pos.x;
	pos.y = (int)Pos.y;
	pos.z = (int)Pos.z;
    	ParticleID = g_NewParticleMgr.Start("Skill3AtalantaTriumphOfValhalla", pos);
 	Max_Time = 2000;
	LiveCount = liveCount;

 	Face.MatNum = *matNum;
 	Face.r = 255;
	Face.g = 150;
	Face.b = 50;
	Face.Transparency = 200;
   	Face.width = 5000;
	cAssaTrace->SetLength(20);
}



AssaSkill3LightingJavelin::AssaSkill3LightingJavelin()
{
	memset(&ShootingAngle, 0, sizeof(ShootingAngle));
	AniIndex = 0;
	AniDelay = 5;
	ParticleID = -1;
}

AssaSkill3LightingJavelin::~AssaSkill3LightingJavelin()
{

}

void AssaSkill3LightingJavelin::StartShort(POINT3D *pCur, POINT3D *pDest)
{
	Posi = *pCur;
	Posi.y -= 1000;
 	DesPosi = *pDest;
	
	DesPosi.y += 4 * fONE;
   	GetRadian3D( pCur->x,pCur->y, pCur->z, DesPosi.x, DesPosi.y, DesPosi.z);
	int angX = GeoResult_X;
	int angY = GeoResult_Y;
	
	ShootingAngle.x = angX;
	ShootingAngle.y = angY;
	ShootingAngle.z = 0;
	
	int step = 12*fONE;				//창이나 기타 던지는 무기
 	GetMoveLocation( 0, 0, step, ShootingAngle.x, ShootingAngle.y, ShootingAngle.z );
	Posi.x += GeoResult_X;
	Posi.y += GeoResult_Y;
	Posi.z += GeoResult_Z;
	
	Max_Time = 500;
	
	D3DVECTOR term;
	term.x = float(DesPosi.x - Posi.x);
	term.y = float(DesPosi.y - Posi.y);
	term.z = float(DesPosi.z - Posi.z);
	
	double length = sqrt(term.x*term.x+term.y*term.y+term.z*term.z);
	if(length == 0)
		length = 0;

   	ScalePosi.x = -int(term.x/length*12000.f);
	ScalePosi.y = -int(term.y/length*12000.f);
	ScalePosi.z = -int(term.z/length*12000.f);
	
	AnimationID[0] = *(int *)AssaSearchRes("light3_01.bmp", SMMAT_BLEND_LAMP);
	AnimationID[1] = *(int *)AssaSearchRes("light3_02.bmp", SMMAT_BLEND_LAMP);
	AnimationID[2] = *(int *)AssaSearchRes("light3_03.bmp", SMMAT_BLEND_LAMP);
	AnimationID[3] = *(int *)AssaSearchRes("light3_04.bmp", SMMAT_BLEND_LAMP);
	//AnimationID[4] = *(int *)AssaSearchRes("light1_05.bmp", SMMAT_BLEND_LAMP);
	//AnimationID[5] = *(int *)AssaSearchRes("light1_06.bmp", SMMAT_BLEND_LAMP);
	
    Face.MatNum = AnimationID[AniIndex];
 	Face.r = 255;
	Face.g = 255;
	Face.b = 255;
	Face.Transparency = 200;
   	Face.width = 3500;
	
	Face.TexRect.left = 0;
	Face.TexRect.top  = 0;
	Face.TexRect.right = 1; 
	Face.TexRect.bottom = 1; 

  	Face.x = Posi.x;
	Face.y = Posi.y;
	Face.z = Posi.z;
    AniDelay = 1;
}

void AssaSkill3LightingJavelin::Start(POINT3D *pCur, POINT3D *pDest)
{
   	Posi = *pCur;
	Posi.y -= 1000;
 	DesPosi = *pDest;
	
	DesPosi.y += 4 * fONE;
   	GetRadian3D( pCur->x,pCur->y, pCur->z, DesPosi.x, DesPosi.y, DesPosi.z);
	int angX = GeoResult_X;
	int angY = GeoResult_Y;
	
	ShootingAngle.x = angX;
	ShootingAngle.y = angY;
	ShootingAngle.z = 0;
	
	int step = 12*fONE;				//창이나 기타 던지는 무기
 	GetMoveLocation( 0, 0, step, ShootingAngle.x, ShootingAngle.y, ShootingAngle.z );
	Posi.x += GeoResult_X;
	Posi.y += GeoResult_Y;
	Posi.z += GeoResult_Z;
	
	Max_Time = 500;
	
	D3DVECTOR term;
	term.x = float(DesPosi.x - Posi.x);
	term.y = float(DesPosi.y - Posi.y);
	term.z = float(DesPosi.z - Posi.z);
	
	double length = sqrt(term.x*term.x+term.y*term.y+term.z*term.z);
	if(length == 0)
		length = 0;

   	ScalePosi.x = -int(term.x/length*15000.f);
	ScalePosi.y = -int(term.y/length*15000.f);
	ScalePosi.z = -int(term.z/length*15000.f);
	
	AnimationID[0] = *(int *)AssaSearchRes("light3_01.bmp", SMMAT_BLEND_LAMP);
	AnimationID[1] = *(int *)AssaSearchRes("light3_02.bmp", SMMAT_BLEND_LAMP);
	AnimationID[2] = *(int *)AssaSearchRes("light3_03.bmp", SMMAT_BLEND_LAMP);
	AnimationID[3] = *(int *)AssaSearchRes("light3_04.bmp", SMMAT_BLEND_LAMP);
	//AnimationID[4] = *(int *)AssaSearchRes("light1_05.bmp", SMMAT_BLEND_LAMP);
	//AnimationID[5] = *(int *)AssaSearchRes("light1_06.bmp", SMMAT_BLEND_LAMP);
	
    Face.MatNum = AnimationID[AniIndex];
 	Face.r = 255;
	Face.g = 255;
	Face.b = 255;
	Face.Transparency = 230;
   	Face.width = 4000;
	
	Face.TexRect.left = 0;
	Face.TexRect.top  = 0;
	Face.TexRect.right = 1; 
	Face.TexRect.bottom = 1; 

  	Face.x = Posi.x;
	Face.y = Posi.y;
	Face.z = Posi.z;
    
	AniDelay = 1;
}

void AssaSkill3LightingJavelin::Main()
{
	if(Max_Time == Time)
		return;
	
 	int step = 7*fONE;				//창이나 기타 던지는 무기
	GetMoveLocation( 0, 0, step, ShootingAngle.x, ShootingAngle.y, ShootingAngle.z );
	Posi.x += GeoResult_X;
	Posi.y += GeoResult_Y;
	Posi.z += GeoResult_Z;
	
	POINT3D pos;
 	pos.x = int(Posi.x);
	pos.y = int(Posi.y);
	pos.z = int(Posi.z);
		
	D3DVECTOR term;
	term.x = float((DesPosi.x - pos.x)/ fONE);
	term.y = float((DesPosi.y - pos.y)/ fONE);
	term.z = float((DesPosi.z - pos.z)/ fONE);
		
	float length = term.x*term.x + term.y*term.y + term.z*term.z;
	length = (float)sqrt(length);
	

 	if(length < 15 || Time > 35)
	{
 	 	Time = Max_Time;
		//AssaParticle_LightHit1(&Posi);
		AssaParticle_Sprak(&Posi);
		if ( (rand()%1)==0 )
			SkillPlaySound( SKILL_SOUND_SKILL_ENCHANTWEAPON_LIGHTNING1   , Posi.x, Posi.y, Posi.z);
		else
			SkillPlaySound( SKILL_SOUND_SKILL_ENCHANTWEAPON_LIGHTNING2 , Posi.x, Posi.y, Posi.z);

		if(ParticleID != -1)
 		{
			g_NewParticleMgr.SetStop(ParticleID);
			ParticleID = -1;
		}
	}
	
	Face.x = Posi.x;
	Face.y = Posi.y;
	Face.z = Posi.z;
	
	D3DVECTOR partPos;
	partPos.x = (float)Posi.x;
	partPos.y = (float)Posi.y;
	partPos.z = (float)Posi.z;

	if(ParticleID != -1) 
		g_NewParticleMgr.SetPos(ParticleID, partPos);


	if(Time%AniDelay == 0)
	{
  		AniIndex++;
		if(AniIndex > 3)
			AniIndex = 0;
		Face.MatNum = AnimationID[AniIndex];
	}
}



void AssaSkill3LightingJavelin::Draw(POINT3D *pPosi , POINT3D *pAngle)
{
 	if(!Face.MatNum)return;
	
	smRender.Init(); //데이터 초기화 
	smMaterialGroup->smMaterial[Face.MatNum].TwoSide = TRUE; //카메라 회전과 상관없이 보이기 
	smRender.SetMaterialGroup(smMaterialGroup);  //읽어드린 메트리얼을 연결 
 	
	//Camera각도 입력.
	if(!AssaAddFace2DLine(&Face, &ScalePosi))
				return;
	
	smRender.SetClipStates(SMCLIP_LEFT | SMCLIP_RIGHT | SMCLIP_TOP | SMCLIP_BOTTOM);
	smRender.ClipRendFace();
	smRender.GeomVertex2D(FALSE);
 	smRender.RenderD3D();
}



AssaSkill3StormJavelin::AssaSkill3StormJavelin()
{
	memset(&ShootingAngle, 0, sizeof(ShootingAngle));
	AniIndex = 0;
	AniDelay = 5;
	ParticleID = -1;
	PatMesh = NULL;
}

AssaSkill3StormJavelin::~AssaSkill3StormJavelin()
{

}

void AssaSkill3StormJavelin::Start(POINT3D *pCur, POINT3D *pDest)
{
    Posi = *pCur;
	Posi.y -= 1000;
  	DesPosi = *pDest;
	//if(DesPosi.y < Posi.y)
		//DesPosi.y = Posi.y;

 	DesPosi.y += 4 * fONE;
	
	D3DVECTOR term;
 	term.x = float(DesPosi.x - Posi.x);
	term.y = float(DesPosi.y - Posi.y);
	term.z = float(DesPosi.z - Posi.z);
	
	double length = sqrt(term.x*term.x+term.y*term.y+term.z*term.z);
	if(length == 0)
		length = 0;

    ScalePosExtStep.x = float(term.x/length);
	ScalePosExtStep.y = float(term.y/length);
	ScalePosExtStep.z = float(term.z/length);

	ScalePosi.x = -int(ScalePosExtStep.x*15000.f);
	ScalePosi.y = -int(ScalePosExtStep.y*15000.f);
   	ScalePosi.z = -int(ScalePosExtStep.z*15000.f);
	
 	ScalePosExt.x = -int(ScalePosExtStep.x*17000.f);
	ScalePosExt.y = -int(ScalePosExtStep.y*17000.f);
   	ScalePosExt.z = -int(ScalePosExtStep.z*17000.f);
	
	
	GetRadian3D( pCur->x,pCur->y, pCur->z, DesPosi.x, DesPosi.y, DesPosi.z);
	int angX = GeoResult_X;
	int angY = GeoResult_Y;
	
    DesPosi.x = int(term.x/length*float(1000*fONE));
	DesPosi.y = int(term.y/length*float(1000*fONE));
   	DesPosi.z = int(term.z/length*float(1000*fONE));

	ShootingAngle.x = angX;
	ShootingAngle.y = angY;
	ShootingAngle.z = 0;
	
  	int step = 30*fONE;				//창이나 기타 던지는 무기
 	GetMoveLocation( 0, 0, step, ShootingAngle.x, ShootingAngle.y, ShootingAngle.z );
	Posi.x += GeoResult_X;
	Posi.y += GeoResult_Y;
	Posi.z += GeoResult_Z;
	
	Max_Time = 500;
	TextureID[0] = *(int *)AssaSearchRes("light1_01.bmp", SMMAT_BLEND_LAMP);
	TextureID[1] = *(int *)AssaSearchRes("light1_02.bmp", SMMAT_BLEND_LAMP);
	TextureID[2] = *(int *)AssaSearchRes("light1_03.bmp", SMMAT_BLEND_LAMP);
	TextureID[3] = *(int *)AssaSearchRes("light1_04.bmp", SMMAT_BLEND_LAMP);
	TextureID[4] = *(int *)AssaSearchRes("light1_05.bmp", SMMAT_BLEND_LAMP);
	TextureID[5] = *(int *)AssaSearchRes("light1_06.bmp", SMMAT_BLEND_LAMP);
	
	TextureIDExt1 = *(int *)AssaSearchRes("At3_StormJ2.bmp", SMMAT_BLEND_LAMP);

	Face.MatNum = TextureID[AniIndex];
 	Face.r = 200;
	Face.g = 200;
	Face.b = 255;
	Face.Transparency = 255;
   	Face.width = 10000;
	
	Face.TexRect.left = 0;
	Face.TexRect.top  = 0;
	Face.TexRect.right = 1; 
	Face.TexRect.bottom = 1; 

	Face.x = Posi.x;
	Face.y = Posi.y;
	Face.z = Posi.z;
    AniDelay = 2;

  	PatMesh = (smPAT3D *)AssaSearchRes("At3_StormJ.ASE", SMMAT_BLEND_LAMP);
	PatMesh->SetPosi(&Posi, &ShootingAngle);
 	BlendStep = -50;
	POINT3D charPos;
	charPos.x = Posi.x;
	charPos.y = Posi.y;
	charPos.z = Posi.z;
	ParticleID = g_NewParticleMgr.Start("twistJavelin1", charPos);
}

void AssaSkill3StormJavelin::Main()
{
	if(Max_Time == Time)
		return;
	
	int step = 6*fONE;				//창이나 기타 던지는 무기
	GetMoveLocation( 0, 0, step, ShootingAngle.x, ShootingAngle.y, ShootingAngle.z );
	Posi.x += GeoResult_X;
	Posi.y += GeoResult_Y;
	Posi.z += GeoResult_Z;
	
	POINT3D pos;
 	pos.x = int(Posi.x);
	pos.y = int(Posi.y);
	pos.z = int(Posi.z);
		
	D3DVECTOR term;
	term.x = float((DesPosi.x - pos.x)/ fONE);
	term.y = float((DesPosi.y - pos.y)/ fONE);
	term.z = float((DesPosi.z - pos.z)/ fONE);
		
	float length = term.x*term.x + term.y*term.y + term.z*term.z;
	length = (float)sqrt(length);
	

  	if(Time > 50)
	{
 	 	Time = Max_Time;
		if(ParticleID != -1)
 		{
			g_NewParticleMgr.SetStop(ParticleID);
			ParticleID = -1;
		}
	}

  	if(Time > 35)
	{
 		BlendStep -= 10;
		if(BlendStep < -255)
			BlendStep = -255;
	}
	
	Face.x = Posi.x;
	Face.y = Posi.y;
	Face.z = Posi.z;
	
	D3DVECTOR partPos;
	partPos.x = (float)Posi.x;
	partPos.y = (float)Posi.y;
	partPos.z = (float)Posi.z;

	if(ParticleID != -1) 
		g_NewParticleMgr.SetPos(ParticleID, partPos);


	if(Time%AniDelay == 0)
	{
 		AniIndex++;
		if(AniIndex > 5)
			AniIndex = 0;
		Face.MatNum = TextureID[AniIndex];
	}
}



void AssaSkill3StormJavelin::Draw(POINT3D *pPosi , POINT3D *pAngle)
{
 	if(!Face.MatNum)return;
	// Turn on Z-buffering
	lpD3DDevice->SetRenderState( D3DRENDERSTATE_ZWRITEENABLE, FALSE );
	
	smRender.Init(); //데이터 초기화 
	smMaterialGroup->smMaterial[Face.MatNum].TwoSide = TRUE; //카메라 회전과 상관없이 보이기 
	smRender.SetMaterialGroup(smMaterialGroup);  //읽어드린 메트리얼을 연결 
 		
	int r,g,b,a;
	r = smRender.Color_R;
	g = smRender.Color_G;
	b = smRender.Color_B;
	a = smRender.Color_A;

	smRender.Color_R = r+BlendStep;
	smRender.Color_G = g+BlendStep;
	smRender.Color_B = b+BlendStep;
 	smRender.Color_A = a+BlendStep;


	//현재 찍을 Frame을 세팅한다..
	//Camera각도 입력.
  	smRender.SetCameraPosi(pPosi->x, pPosi->y, pPosi->z,pAngle->x,pAngle->y,pAngle->z);
	Face.width = 10000;
	if(!AssaAddFace2DLine(&Face, &ScalePosi))
				return;

  	Face.MatNum = TextureIDExt1;
	Face.width = 15000;
	if(!AssaAddFace2DLine(&Face, &ScalePosExt))
				return;

  	smRender.SetClipStates(SMCLIP_LEFT | SMCLIP_RIGHT | SMCLIP_TOP | SMCLIP_BOTTOM);
	smRender.ClipRendFace();
	smRender.GeomVertex2D(FALSE);
 	smRender.RenderD3D();
	
	if(PatMesh != NULL)
	{
  		POINT3D meshAngle;
 		meshAngle.x = 0;
 		meshAngle.y = (-ShootingAngle.y+ANGLE_180)&ANGCLIP;
  		meshAngle.z = (Time*120)&ANGCLIP;
	 
  		PatMesh->SetPosi(&Posi, &meshAngle);
		PatMesh->RenderD3D(pPosi, pAngle);
	}
	smRender.Color_R = r;
	smRender.Color_G = g;
	smRender.Color_B = b;
	smRender.Color_A = a;

	lpD3DDevice->SetRenderState( D3DRENDERSTATE_ZWRITEENABLE, TRUE );
}




AssaSkill3SplitJavelin::AssaSkill3SplitJavelin()
{
	memset(&ShootingAngle, 0, sizeof(ShootingAngle));
	PatMesh = NULL;
	Delay = 0;
}

AssaSkill3SplitJavelin::~AssaSkill3SplitJavelin()
{

}

void AssaSkill3SplitJavelin::Start(POINT3D *pCur, POINT3D *pDest, int delay)
{
     int *matNum = (int *)AssaSearchRes("shield1.bmp", SMMAT_BLEND_LAMP);
 	if(matNum == NULL)
		return;

    Face.MatNum = *matNum;
 	Face.r = 255;
	Face.g = 150;
	Face.b = 50;
	Face.Transparency = 130;
 	Face.width = 2500;
	
	Posi = *pCur;
	DesPosi = *pDest;
	DesPosi.y += 4 * fONE;
	
	D3DVECTOR term;
 	term.x = float(DesPosi.x - Posi.x);
	term.y = float(DesPosi.y - Posi.y);
	term.z = float(DesPosi.z - Posi.z);
	
	GetRadian3D(Posi.x,Posi.y, Posi.z, DesPosi.x, DesPosi.y, DesPosi.z);
	ShootingAngle.x = GeoResult_X;
	ShootingAngle.y = GeoResult_Y;
	ShootingAngle.z = 0;
	
	
   	int step = -40*fONE;				
	
	
	//창이나 기타 던지는 무기
  	GetMoveLocation( 0, 0, step, ShootingAngle.x, ShootingAngle.y, ShootingAngle.z );


	
	ScalePosi.x = GeoResult_X;
	ScalePosi.y = GeoResult_Y;
	ScalePosi.z = GeoResult_Z;

	CurrentFrame = 0;
	Delay = delay;

	Max_Time = 500;
	PatMesh = (smPAT3D *)AssaSearchRes("At3_SplitJavelin40.ASE", SMMAT_BLEND_LAMP);
	POINT3D meshAngle;
    meshAngle.x = ShootingAngle.x;
  	meshAngle.y = (-ShootingAngle.y+ANGLE_180)&ANGCLIP;
  	meshAngle.z = ShootingAngle.z;
	
	PatMesh->Frame = CurrentFrame;
	PatMesh->SetPosi(&Posi, &meshAngle);
	
	Face.x = Posi.x;
	Face.y = Posi.y;
	Face.z = Posi.z;
	BlendStep = -255;
}


void AssaSkill3SplitJavelin::Main()
{
    	if(Max_Time == Time)
		return;
	if(Time > Delay)
 	{
		if(BlendStep >= 0)
			BlendStep = 0;
		else
			BlendStep += 30;
	}

	if(Time < Delay+40)
		return;
	
  	

 	Face.x = Posi.x;
	Face.y = Posi.y;
	Face.z = Posi.z;

      int step = 10*fONE;				//창이나 기타 던지는 무기
 	GetMoveLocation( 0, 0, step, ShootingAngle.x, ShootingAngle.y, ShootingAngle.z );
	Posi.x += GeoResult_X;
	Posi.y += GeoResult_Y;
	Posi.z += GeoResult_Z;
	
	POINT3D pos;
 	pos.x = int(Posi.x);
	pos.y = int(Posi.y);
	pos.z = int(Posi.z);
		
	D3DVECTOR term;
	term.x = float((DesPosi.x - pos.x)/ fONE);
	term.y = float((DesPosi.y - pos.y)/ fONE);
	term.z = float((DesPosi.z - pos.z)/ fONE);
		
 	float length = term.x*term.x + term.y*term.y + term.z*term.z;
	length = (float)sqrt(length);
	
	if(length < 20 || Time > (35+Delay+20))
	{
  	 	Time = Max_Time;
		int num = g_NewParticleMgr.Start("Skill3AtalantaSplitJavelin1", DesPosi);
		g_NewParticleMgr.SetAngle(num, ShootingAngle);
	}
}

void AssaSkill3SplitJavelin::Draw(POINT3D *pPosi , POINT3D *pAngle)
{
	if(PatMesh != NULL)
	{
 		
		smRender.Init(); //데이터 초기화 
		POINT3D meshAngle;
        meshAngle.x = ShootingAngle.x;
  		meshAngle.y = (-ShootingAngle.y+ANGLE_180)&ANGCLIP;
 		meshAngle.z = ShootingAngle.z;
		
		int r,g,b,a;
		r = smRender.Color_R;
		g = smRender.Color_G;
		b = smRender.Color_B;
		a = smRender.Color_A;

		smRender.Color_R = r+BlendStep;
		smRender.Color_G = g+BlendStep;
		smRender.Color_B = b+BlendStep;
 		smRender.Color_A = a+BlendStep;

		PatMesh->Frame = CurrentFrame;
		PatMesh->SetPosi(&Posi, &meshAngle);
		PatMesh->RenderD3D(pPosi, pAngle);
		
		smRender.Init(); //데이터 초기화 
 		smMaterialGroup->smMaterial[Face.MatNum].TwoSide = TRUE; //카메라 회전과 상관없이 보이기 
		smRender.SetMaterialGroup(smMaterialGroup);  //읽어드린 메트리얼을 연결 
		smRender.SetCameraPosi(pPosi->x, pPosi->y, pPosi->z,pAngle->x,pAngle->y,pAngle->z);
		
		if(!AssaAddFace2DLine(&Face, &ScalePosi))
				return;
		
 		//smRender.SetClipStates(SMCLIP_LEFT | SMCLIP_RIGHT | SMCLIP_TOP | SMCLIP_BOTTOM);
		smRender.ClipRendFace();
  		smRender.GeomVertex2D(FALSE);
		smRender.RenderD3D();

		smRender.Color_R = r;
		smRender.Color_G = g;
		smRender.Color_B = b;
		smRender.Color_A = a;

	}
}






AssaClassUpWeapon::AssaClassUpWeapon()
{
	pChar = NULL;	
	MyCharacterFlag = FALSE;
	LocalAngle = 0;
	memset(&LocalPos, 0, sizeof(POINT3D));
	LocalPosTransFlag = FALSE;

	for(int index = 0; index < 4; index++)
		ParticleIDGroup[index] = -1;
}

AssaClassUpWeapon::~AssaClassUpWeapon()
{
	
}

void AssaClassUpWeapon::Start(smCHAR* character)
{
	

    pChar = character;
	if(pChar == NULL)
		return;
	

	if(lpCurPlayer == pChar)
	{
		MyCharacterFlag = TRUE;
	}
	
	EndFlag = FALSE;
  	Max_Time = 5000;

	LocalPos.y = 1500;
	POINT3D charPos;
	charPos.x = pChar->pX;
	charPos.y = pChar->pY;
	charPos.z = pChar->pZ;
	
	ParticleIDGroup[0] = g_NewParticleMgr.Start("ClassUpWeapon1", charPos);
	ParticleIDGroup[1] = g_NewParticleMgr.Start("ClassUpWeapon2", charPos);
	ParticleIDGroup[2] = g_NewParticleMgr.Start("ClassUpWeapon1", charPos);
	ParticleIDGroup[3] = g_NewParticleMgr.Start("ClassUpWeapon2", charPos);
	
	CODE = CLASS_UP_PART1;
}

void AssaClassUpWeapon::RespawnParticle()
{
	if(ParticleIDGroup[0] == -1 ||
	   ParticleIDGroup[1] == -1 ||
	   ParticleIDGroup[2] == -1 ||
	   ParticleIDGroup[3] == -1)
	{
		POINT3D charPos;
		charPos.x = pChar->pX;
		charPos.y = pChar->pY;
		charPos.z = pChar->pZ;
		
		ParticleIDGroup[0] = g_NewParticleMgr.Start("ClassUpWeapon1", charPos);
		ParticleIDGroup[1] = g_NewParticleMgr.Start("ClassUpWeapon2", charPos);
		ParticleIDGroup[2] = g_NewParticleMgr.Start("ClassUpWeapon1", charPos);
		ParticleIDGroup[3] = g_NewParticleMgr.Start("ClassUpWeapon2", charPos);
	}
}

void AssaClassUpWeapon::StopParticle()
{
	//파티클을 다끈다.
 	for(int index = 0; index < 4; index++)
	{
		if(ParticleIDGroup[index] != -1)
		{
			g_NewParticleMgr.SetFastStop(ParticleIDGroup[index]);
			g_NewParticleMgr.SetStop(ParticleIDGroup[index]);
			g_NewParticleMgr.SetRendering(ParticleIDGroup[index], FALSE);
		}
	}
}

void AssaClassUpWeapon::StopParticleID()
{
	for(int index = 0; index < 4; index++)
	{
		if(ParticleIDGroup[index] != -1)
			ParticleIDGroup[index] = -1;
	}
}

void AssaClassUpWeapon::Main()
{	
 	if(MyCharacterFlag)
	{
 		if(lpCurPlayer != pChar)
			pChar = lpCurPlayer;
	}
	
   	if(EndFlag)
	{
		Time = Max_Time;
		return;
	}
	else
	{
		if(Time > Max_Time-140)
			Time = Max_Time-150;
	}

	smMATRIX *mWorld;
	POINT3D weaponPos;
	memset(&weaponPos, 0, sizeof(weaponPos));
   	if(pChar->HvRightHand.dwItemCode != 0)
	{
		mWorld = &pChar->HvRightHand_Matrix;
 		if((pChar->HvRightHand.dwItemCode & sinITEM_MASK2) == sinWT1) //제블린일 경우..
		{
			weaponPos.z = -pChar->HvRightHand.SizeMax+int((float)pChar->HvRightHand.SizeMax*0.6f);
		}
		else if((pChar->HvRightHand.dwItemCode & sinITEM_MASK2) == sinWS2) //칼인경우..
		{
  			if(pChar->HvRightHand.SizeMax > 5000)
				weaponPos.z = pChar->HvRightHand.SizeMax-int((float)pChar->HvRightHand.SizeMax*0.25f);
			else
				weaponPos.z = pChar->HvRightHand.SizeMax-int((float)pChar->HvRightHand.SizeMax*0.2f);
		}
		else if((pChar->HvRightHand.dwItemCode & sinITEM_MASK2) == sinWC1) //클러일 경우..
		{
  			weaponPos.x = pChar->HvRightHand.SizeMax+pChar->HvRightHand.SizeMax;
 		}
		else if((pChar->HvRightHand.dwItemCode & sinITEM_MASK2) ==  sinWM1)//완드인경우..
  			weaponPos.z = pChar->HvRightHand.SizeMax-int((float)pChar->HvRightHand.SizeMax*0.3f);
		else if((pChar->HvRightHand.dwItemCode & sinITEM_MASK2) ==  sinWS1)//활인경우..
		{
   			weaponPos.z = pChar->HvRightHand.SizeMax-int((float)pChar->HvRightHand.SizeMax*0.3f);

    			//mWorld = &pChar->HvLeftHand_Matrix;
  			//weaponPos.z = pChar->HvLeftHand.SizeMax;

			
			int cnt;
    		if( pChar->HvRightHand.dwItemCode != 0) //활을 든경우 보정..
			{
				cnt = (ANGLE_45)&ANGCLIP;
				GetMoveLocation(weaponPos.x, weaponPos.y, weaponPos.z, 0, cnt, 0);
				weaponPos.x = GeoResult_X;
				weaponPos.y = GeoResult_Y;
				weaponPos.z = GeoResult_Z;
			}
			
			
		}	
		else
			weaponPos.z = pChar->HvRightHand.SizeMax-int((float)pChar->HvRightHand.SizeMax*0.5f);
	}
	else if(pChar->HvLeftHand.dwItemCode != 0)
	{
 		if((pChar->HvLeftHand.dwItemCode & sinITEM_MASK2) == sinWS1) //활인 경우
		{
			mWorld = &pChar->HvLeftHand_Matrix;
 			weaponPos.z = pChar->HvLeftHand.SizeMax-10*fONE;
			int cnt;
			if( pChar->HvLeftHand.dwItemCode != 0) //활을 든경우 보정..
			{
				cnt = (ANGLE_90)&ANGCLIP;
				GetMoveLocation(weaponPos.x, weaponPos.y, weaponPos.z, 0, cnt, 0);
				weaponPos.x = GeoResult_X;
				weaponPos.y = GeoResult_Y;
				weaponPos.z = GeoResult_Z;
			}
		}
		else //다른 무기를 들고 있고 오른손에 무기를 들고 있지 않을 경우
		{
			Time = Max_Time;
			StopParticle();
			StopParticleID();
			EndFlag = TRUE;
			CODE = 0;
			return;
		}
	}
	else //손에 무기를 들고 있지 않을 경우.
	{
		Time = Max_Time;
		StopParticle();
		StopParticleID();
		EndFlag = TRUE;
		CODE = 0;
		return;
	}
	
	//양손 무기 5도 기울어짐..
	int cnt;
	int sin;
	int cos;
	if ( pChar->dwActionItemTwoHand ) 
	{
		cnt = (-ANGLE_45/5)&ANGCLIP;
			
		sin = GetSin[cnt]>>8;
		cos = GetCos[cnt]>>8;
		
		weaponPos.y = -( weaponPos.z * sin )>>8;
		weaponPos.z =  ( weaponPos.z * cos )>>8;
	}

    LocalAngle += 70;
	if(LocalAngle >= 4096)
		LocalAngle = 0;
	
     if(LocalPosTransFlag == FALSE)
	{
		LocalPos.y -= 10;
		if(LocalPos.y < 1000)
			LocalPosTransFlag = TRUE;
	}
	else
	{
		LocalPos.y += 10;
		if(LocalPos.y > 1600)
			LocalPosTransFlag = FALSE;
	}

	POINT3D part1Pos;
	POINT3D part2Pos;

  	memcpy(&part1Pos, &weaponPos, sizeof(POINT3D));
	memcpy(&part2Pos, &weaponPos, sizeof(POINT3D));
   	if(pChar->HvRightHand.dwItemCode != 0)
	{
 		//GetMoveLocation( 0, LocalPos.y, 0, 0, 0, LocalAngle);
   		if((pChar->HvRightHand.dwItemCode & sinITEM_MASK2) == sinWC1 || //클러일 경우.
		   (pChar->HvRightHand.dwItemCode & sinITEM_MASK2) == sinWS1)	
			GetMoveLocation( 0, LocalPos.y, 0, LocalAngle, 0, 0);
		else
			GetMoveLocation( 0, LocalPos.y, 0, 0, 0, LocalAngle);
	}
	else if(pChar->HvLeftHand.dwItemCode != 0)
		GetMoveLocation( 0, LocalPos.y, 0, LocalAngle, 0, 0);

	part1Pos.x += GeoResult_X;
	part1Pos.y += GeoResult_Y;
	part1Pos.z += GeoResult_Z;
    
  	if(pChar->HvRightHand.dwItemCode != 0)
	{
		if((pChar->HvRightHand.dwItemCode & sinITEM_MASK2) == sinWC1 || //클러인 경우.
		   (pChar->HvRightHand.dwItemCode & sinITEM_MASK2) == sinWS1 ) 
 			GetMoveLocation( 0, -LocalPos.y, 0, LocalAngle, 0, 0);
		else //클러일경우.
			GetMoveLocation( 0, -LocalPos.y, 0, 0, 0, LocalAngle);
			
	}
	else if(pChar->HvLeftHand.dwItemCode != 0)
		GetMoveLocation( 0, -LocalPos.y, 0, LocalAngle, 0, 0);


	part2Pos.x += GeoResult_X;
	part2Pos.y += GeoResult_Y;
	part2Pos.z += GeoResult_Z;

	POINT3D angle;
  	memcpy( &angle, &pChar->Angle , sizeof( POINT3D ) );
	angle.y = (-angle.y+ANGLE_180)&ANGCLIP;
	
	if(pChar->HvRightHand.dwItemCode != 0)
		AnimObjectTree( pChar->HvRightHand.ObjBip, pChar->frame, angle.x,angle.y,angle.z);
	else if(pChar->HvLeftHand.dwItemCode != 0)
		AnimObjectTree( pChar->HvLeftHand.ObjBip, pChar->frame, angle.x,angle.y,angle.z);


	int rx, ry, rz;
	rx = part1Pos.x * mWorld->_11 +
		 part1Pos.y * mWorld->_21 +
 		 part1Pos.z * mWorld->_31 ;

	ry = part1Pos.x * mWorld->_12 +
	     part1Pos.y * mWorld->_22 +
	     part1Pos.z * mWorld->_32 ;

	rz = part1Pos.x * mWorld->_13 +
		 part1Pos.y * mWorld->_23 +
		 part1Pos.z * mWorld->_33 ;
	
	D3DVECTOR charPos;
	charPos.x = (float)(pChar->pX + (rx>>FLOATNS) + mWorld->_41);
	charPos.z = (float)(pChar->pZ + (ry>>FLOATNS) + mWorld->_42);
	charPos.y = (float)(pChar->pY + (rz>>FLOATNS) + mWorld->_43);

	g_NewParticleMgr.SetAttachPos(ParticleIDGroup[0], charPos);
	g_NewParticleMgr.SetRendering(ParticleIDGroup[0], pChar->FlagShow);
	
	g_NewParticleMgr.SetPos(ParticleIDGroup[1], charPos);
	g_NewParticleMgr.SetRendering(ParticleIDGroup[1], pChar->FlagShow);
	
	rx = part2Pos.x * mWorld->_11 +
		 part2Pos.y * mWorld->_21 +
 		 part2Pos.z * mWorld->_31 ;

	ry = part2Pos.x * mWorld->_12 +
	     part2Pos.y * mWorld->_22 +
	     part2Pos.z * mWorld->_32 ;

	rz = part2Pos.x * mWorld->_13 +
		 part2Pos.y * mWorld->_23 +
		 part2Pos.z * mWorld->_33 ;

	charPos.x = (float)(pChar->pX + (rx>>FLOATNS) + mWorld->_41);
	charPos.z = (float)(pChar->pZ + (ry>>FLOATNS) + mWorld->_42);
	charPos.y = (float)(pChar->pY + (rz>>FLOATNS) + mWorld->_43);

 	g_NewParticleMgr.SetAttachPos(ParticleIDGroup[2], charPos);
	g_NewParticleMgr.SetRendering(ParticleIDGroup[2], pChar->FlagShow);
	
	g_NewParticleMgr.SetPos(ParticleIDGroup[3], charPos);
	g_NewParticleMgr.SetRendering(ParticleIDGroup[3], pChar->FlagShow);
	

    if(EndFlag == FALSE) 
	{
 	 	int flag = 0;		//1: left 2: right
		if(pChar->dwActionItemCode == pChar->HvLeftHand.dwItemCode)
			flag = 1;
		if(pChar->dwActionItemCode == pChar->HvRightHand.dwItemCode)
			flag = 2;

		if((!MyCharacterFlag && pChar->Flag == 0) ||
	    (flag == 1 && pChar->HvLeftHand.DispEffect == 0) || (flag == 2 && pChar->HvRightHand.DispEffect == 0) )
		{
			
   			Time = Max_Time;
			StopParticle();
			StopParticleID();
			EndFlag = TRUE;
			CODE = 0;
		}
	}
}


AssaNatureSnow::AssaNatureSnow()
{

	pChar = NULL;
	MyCharacterFlag = FALSE;
	ParticleID = -1;
}


AssaNatureSnow::~AssaNatureSnow()
{

}

void AssaNatureSnow::Start()
{
	pChar = lpCurPlayer;
	if(pChar == NULL)
		return;
	if(lpCurPlayer == pChar)
	{
		MyCharacterFlag = TRUE;
	}
	
	POINT3D charPos;
	charPos.x = pChar->pX;
	charPos.y = pChar->pY+100000;
	charPos.z = pChar->pZ;

	ParticleID = g_NewParticleMgr.Start("Snow", charPos);
	
	Max_Time = 100*70+140;
	EndFlag = FALSE;
}

void AssaNatureSnow::Main()
{	
	if(ParticleID == -1)
		return;
	
	if(!g_NewParticleMgr.GetRunning(ParticleID) && EndFlag)
	{
		ParticleID = -1;
		return;
	}

	if(MyCharacterFlag)
	{
		if(lpCurPlayer != pChar)
			pChar = lpCurPlayer;
	}
	
 	GetMoveLocation( 0, 0, 40*fONE, 0, pChar->Angle.y, 0);
	
	D3DVECTOR charPos;
  	charPos.x = (float)pChar->pX+GeoResult_X;
	charPos.y = (float)pChar->pY+30000;
	charPos.z = (float)pChar->pZ+GeoResult_Z;
	g_NewParticleMgr.SetPos(ParticleID, charPos);
	
	if(Max_Time > Time-3)
		Time = 0;
}



AssaRunicGuardianShot::AssaRunicGuardianShot()
{
	ParticleID = -1;
	memset(&ShootingAngle, 0, sizeof(ShootingAngle));
}

AssaRunicGuardianShot::~AssaRunicGuardianShot()
{

}

void AssaRunicGuardianShot::Start(POINT3D *pCur, POINT3D *pDest)
{
 	ParticleID = g_NewParticleMgr.Start("IronMonsterRunicGuardianShot1", Posi);
	
	Posi.x = pCur->x;
	Posi.y = pCur->y;
	Posi.z = pCur->z;
	
	DesPosi.x = pDest->x;
	DesPosi.y = pDest->y;
	DesPosi.z = pDest->z;

	GetRadian3D( Posi.x,Posi.y, Posi.z, DesPosi.x, DesPosi.y, DesPosi.z);
	int angX = GeoResult_X;
	int angY = GeoResult_Y;
	ShootingAngle.x = angX&ANGCLIP;
	ShootingAngle.y = angY&ANGCLIP;
	ShootingAngle.z = 0;

    //int step = 40*fONE;				//창이나 기타 던지는 무기
 	//GetMoveLocation( 0, 0, step, ShootingAngle.x, ShootingAngle.y, ShootingAngle.z );
	//Posi.x += GeoResult_X;
	//Posi.y += GeoResult_Y;
	//Posi.z += GeoResult_Z;

	Max_Time = 5000;
 	
}



void AssaRunicGuardianShot::Main()
{
    int step = 5*fONE+100;				//창이나 기타 던지는 무기
	GetMoveLocation( 0, 0, step, ShootingAngle.x, ShootingAngle.y, ShootingAngle.z );
	Posi.x += GeoResult_X;
	Posi.y += GeoResult_Y;
	Posi.z += GeoResult_Z;
		
	POINT3D pos;
 	pos.x = int(Posi.x);
	pos.y = int(Posi.y);
	pos.z = int(Posi.z);

	D3DVECTOR term;
	term.x = float((DesPosi.x - pos.x)/ fONE);
	term.y = float((DesPosi.y - pos.y)/ fONE);
	term.z = float((DesPosi.z - pos.z)/ fONE);
		
	float length = term.x*term.x + term.y*term.y + term.z*term.z;
	length = (float)sqrt(length);
		
   	if(length < 25 || Time > 60)
	{
   		if(ParticleID != -1)
		{
			g_NewParticleMgr.SetStop(ParticleID);
			Time = Max_Time;
			SetDynLight( pos.x, pos.y, pos.z, 255, 150, 50, 255, 200, 2);
			//AssaParticle_BombHit1(&pos);
			//SkillPlaySound( SKILL_SOUND_SKILL_BOMB_SHOT2	 ,pos.x, pos.y, pos.z);
		}
	}
	
 	D3DVECTOR partPos;
	partPos.x = (float)Posi.x;
	partPos.y = (float)Posi.y;
	partPos.z = (float)Posi.z;

	if(ParticleID != -1)
		g_NewParticleMgr.SetAttachPos(ParticleID, partPos);
}






/*
AssaRunicGuardianBossShot::AssaRunicGuardianBossShot()
{
	ParticleID = -1;
	memset(&Velocity, 0, sizeof(Velocity));
	LiveCount = 0;
}

AssaRunicGuardianBossShot::~AssaRunicGuardianBossShot()
{
	
}

void AssaRunicGuardianBossShot::Start(POINT3D *pShotPos, POINT3D *pAttackPos, D3DVECTOR* velocity)
{
	
	int *matNum = (int *)AssaSearchRes("shield1.bmp", SMMAT_BLEND_LAMP);
 	if(matNum == NULL)
		return;

	//SearchObj = destChar->AnimPattern->GetObjectFromName("Bip01 R Hand"); //오브젝트를 읽는다
	CurPos.x = (float)pShotPos->x;
	CurPos.y = (float)pShotPos->y;
 	CurPos.z = (float)pShotPos->z;
	
	AttackPos.x = (float)pAttackPos->x;
	AttackPos.y = (float)pAttackPos->y;
	AttackPos.z = (float)pAttackPos->z;

	if(velocity != NULL)
	{
		Velocity.x = velocity->x;
		Velocity.y = velocity->y;
		Velocity.z = velocity->z;
	}
	
	ParticleID = g_NewParticleMgr.Start("Skill3AtalantaTriumphOfValhalla", *pShotPos);
 	Max_Time = 2000;
	//LiveCount = liveCount;
	
	Face.MatNum = *matNum;
 	Face.r = 255;
	Face.g = 150;
	Face.b = 50;
	Face.Transparency = 200;
   	Face.width = 5000;
	cAssaTrace->SetLength(20);
}


void AssaRunicGuardianBossShot::Main()
{
	//smMATRIX *mWorld;
    //AnimObjectTree( SearchObj, DestChar->frame, 0, (-DestChar->Angle.y+ANGLE_180)&ANGCLIP, 0);
	//mWorld = &SearchObj->mWorld;
	
	D3DVECTOR term;
	term.x = (AttackPos.x - CurPos.x) / fONE;
	term.y = (AttackPos.y - CurPos.y) / fONE;
	term.z = (AttackPos.z - CurPos.z) / fONE;
	float length = term.x*term.x + term.y*term.y + term.z*term.z;
	length = (float)sqrt(length);
	if(length < 15 || Time > 100) //충돌을 했거나 시간이 너무 지났을 경우.
	{
		if(ParticleID != -1)
		{
			g_NewParticleMgr.SetRendering(ParticleID, FALSE);
			g_NewParticleMgr.SetStop(ParticleID);
			g_NewParticleMgr.SetFastStop(ParticleID);
			ParticleID = -1;
		}
		Time = Max_Time;
	}
	
	D3DVECTOR velocity;
	velocity.x = term.x / length;
	velocity.y = term.y / length;
	velocity.z = term.z / length;
	
	Velocity.x += velocity.x;
	Velocity.y += velocity.y;
	Velocity.z += velocity.z;

	
	if(length < 100)
	{
		Velocity.x = Velocity.x * 0.8f;
		Velocity.y = Velocity.y * 0.8f;
		Velocity.z = Velocity.z * 0.8f;
	}
	
  	CurPos.x += Velocity.x * fONE;
	CurPos.y += Velocity.y * fONE;
	CurPos.z += Velocity.z * fONE;
	

	POINT3D pos;
	pos.x = (int)CurPos.x;
	pos.y = (int)CurPos.y;
	pos.z = (int)CurPos.z;

	cAssaTrace->AddData(&pos);			
  	if(ParticleID != -1)
	{
		g_NewParticleMgr.SetAttachPos(ParticleID, CurPos);
	}
}






void AssaRunicGuardianBossShot::Draw(POINT3D *pPosi , POINT3D *pAngle)
{
 	if(!Face.MatNum)return;
	smRender.Init(); //데이터 초기화 
	smMaterialGroup->smMaterial[Face.MatNum].TwoSide = TRUE; //카메라 회전과 상관없이 보이기 
	smRender.SetMaterialGroup(smMaterialGroup);  //읽어드린 메트리얼을 연결 
	Face.width = 1000;
	if(!AssaAddFaceTrace(&Face, cAssaTrace))
			return;
	
	Face.width = 4000;
	if(!AssaAddFaceTrace(&Face, cAssaTrace))
			return;

	smRender.SetClipStates(SMCLIP_LEFT | SMCLIP_RIGHT | SMCLIP_TOP | SMCLIP_BOTTOM);
	smRender.ClipRendFace();
	smRender.GeomVertex2D(FALSE);

 	//lpD3DDevice->SetRenderState( D3DRENDERSTATE_ZWRITEENABLE, FALSE );
  	smRender.RenderD3D();
}
*/











AssaRunicGuardianBossShot::AssaRunicGuardianBossShot()
{
	ParticleID = -1;
	memset(&Velocity, 0, sizeof(Velocity));
	LiveCount = 0;
	HitEffectFlag = false;
}

AssaRunicGuardianBossShot::~AssaRunicGuardianBossShot()
{
	
}

void AssaRunicGuardianBossShot::Start(POINT3D *pShotPos, POINT3D *pAttackPos, D3DVECTOR* velocity, int BlendType)
{
	
    int *matNum;
  	if(BlendType == SMMAT_BLEND_LAMP)
		matNum = (int *)AssaSearchRes("shield1.bmp", SMMAT_BLEND_LAMP);
	else
		matNum = (int *)AssaSearchRes("flare.bmp", SMMAT_BLEND_INVSHADOW);

 	if(matNum == NULL)
		return;

	//SearchObj = destChar->AnimPattern->GetObjectFromName("Bip01 R Hand"); //오브젝트를 읽는다
	CurPos.x = (float)pShotPos->x;
	CurPos.y = (float)pShotPos->y;
 	CurPos.z = (float)pShotPos->z;
	
	AttackPos.x = (float)pAttackPos->x;
	AttackPos.y = (float)pAttackPos->y;
	AttackPos.z = (float)pAttackPos->z;

	if(velocity != NULL)
	{
		Velocity.x = velocity->x;
		Velocity.y = velocity->y;
		Velocity.z = velocity->z;
	}
	
 	ParticleID = g_NewParticleMgr.Start("Skill3AtalantaTriumphOfValhalla", *pShotPos);
 	Max_Time = 2000;
	//LiveCount = liveCount;
	
	Face.MatNum = *matNum;
 	Face.r = 255;
	Face.g = 150;
	Face.b = 50;
	Face.Transparency = 200;
   	Face.width = 8000;
	cAssaTrace->SetLength(17);
}


void AssaRunicGuardianBossShot::Main()
{
	D3DVECTOR term;
	term.x = (AttackPos.x - CurPos.x) / fONE;
	term.y = (AttackPos.y - CurPos.y) / fONE;
	term.z = (AttackPos.z - CurPos.z) / fONE;
 	float length = term.x*term.x + term.y*term.y + term.z*term.z;
  	length = (float)sqrt(length);
     if(length < 50 || Time > 80 && ParticleID != -1) //충돌을 했거나 시간이 너무 지났을 경우.
	{
		if(ParticleID != -1)
		{
  			g_NewParticleMgr.SetRendering(ParticleID, FALSE);
			g_NewParticleMgr.SetStop(ParticleID);
			g_NewParticleMgr.SetFastStop(ParticleID);
			ParticleID = -1;
			
 			if(HitEffectFlag)
			{
				POINT3D partPos;
				partPos.x = (int)AttackPos.x;
				partPos.y = (int)AttackPos.y;
				partPos.z = (int)AttackPos.z;
				AssaParticle_MonsterRunicGuardianBossSkillHit(&partPos);

				
			}
		}
		Time = Max_Time-20;
	}
	
	D3DVECTOR velocity;
   	velocity.x = term.x / length;
 	velocity.y = term.y / length;
	velocity.z = term.z / length;

 	

     if(length < 70)
    {
		//Velocity.x += velocity.x*4.f;//Velocity.x * 0.9f;
 		//Velocity.y += velocity.y*4.f;//Velocity.y * 0.9f;
		//Velocity.z += velocity.z*4.f;//Velocity.z * 0.9f;

       	//Velocity.x += velocity.x*4.f;//Velocity.x * 0.9f;
 		//Velocity.y += velocity.y*4.f;//Velocity.y * 0.9f;
		//Velocity.z += velocity.z*4.f;//Velocity.z * 0.9f;

		//Velocity.x = Velocity.x * 0.91f;
		//Velocity.y = Velocity.y * 0.91f;
		//Velocity.z = Velocity.z * 0.91f;

	}
	else
	{
		Velocity.x += velocity.x * 0.45f;
		Velocity.y += velocity.y * 0.45f;
		Velocity.z += velocity.z * 0.45f;
	}
	
	
  	CurPos.x += Velocity.x * fONE;
	CurPos.y += Velocity.y * fONE;
	CurPos.z += Velocity.z * fONE;
	

	POINT3D pos;
	pos.x = (int)CurPos.x;
	pos.y = (int)CurPos.y;
	pos.z = (int)CurPos.z;

	cAssaTrace->AddData(&pos);			
  	if(ParticleID != -1)
	{
		g_NewParticleMgr.SetAttachPos(ParticleID, CurPos);
	}
}






void AssaRunicGuardianBossShot::Draw(POINT3D *pPosi , POINT3D *pAngle)
{
        if(!Face.MatNum)return;
	smRender.Init(); //데이터 초기화 
	smMaterialGroup->smMaterial[Face.MatNum].TwoSide = TRUE; //카메라 회전과 상관없이 보이기 
	smRender.SetMaterialGroup(smMaterialGroup);  //읽어드린 메트리얼을 연결 
	Face.width = 5000;
	if(!AssaAddFaceTrace(&Face, cAssaTrace))
			return;
	
	Face.width = 13000;
	if(!AssaAddFaceTrace(&Face, cAssaTrace))
			return;

	smRender.SetClipStates(SMCLIP_LEFT | SMCLIP_RIGHT | SMCLIP_TOP | SMCLIP_BOTTOM);
	smRender.ClipRendFace();
	smRender.GeomVertex2D(FALSE);

 	//lpD3DDevice->SetRenderState( D3DRENDERSTATE_ZWRITEENABLE, FALSE );66
  	smRender.RenderD3D();
}






AssaRunicGuardianBossHit::AssaRunicGuardianBossHit()
{
	DestChar = NULL;
	ParticleID = -1;
	ParticleIDExt1 = -1;
	memset(&Velocity, 0, sizeof(Velocity));
}

AssaRunicGuardianBossHit::~AssaRunicGuardianBossHit()
{
	
}

void AssaRunicGuardianBossHit::Main()
{
	if(Time < 0)
		return;
	if(ParticleID == -1)
		return;
	
	if(TempMyIndex != -1)
	{
		if(CharList[TempMyIndex] != lpCurPlayer)
		{
			CharList[TempMyIndex] = lpCurPlayer;
		}
	}

	D3DVECTOR destPos;
  	destPos.x = (float)DestChar->pX;
	destPos.y = float(DestChar->pY+DestChar->PatHeight/2);
	destPos.z = (float)DestChar->pZ;
	
	D3DVECTOR term;
	term.x = float((destPos.x - Pos.x)/ fONE);
	term.y = float((destPos.y - Pos.y)/ fONE);
	term.z = float((destPos.z - Pos.z)/ fONE);
		
  	float length = term.x*term.x + term.y*term.y + term.z*term.z;
	length = (float)sqrt(length);
	if(length < 15 || Time > 100) //충) //충돌을 했을 경우..
	{
		if(ParticleID != -1)
		{
			g_NewParticleMgr.SetStop(ParticleID);
			ParticleID = -1;
			Time = Max_Time;
		}

		if(ParticleIDExt1 != -1)
		{
 			g_NewParticleMgr.SetFastStop(ParticleIDExt1);
			g_NewParticleMgr.SetStop(ParticleIDExt1);
			ParticleIDExt1 = -1;
			Time = Max_Time;
			
			POINT3D partPos;
			partPos.x = (int)Pos.x;
			partPos.y = (int)Pos.y;
			partPos.z = (int)Pos.z;

			g_NewParticleMgr.Start("IronMonsterRunicGuardianBossHit1", partPos);
			if(CharCount > 0)
            AssaParticle_MonsterRunicGuardianBossLightShot(CharList, CharCount);
		}
	}
	
	D3DVECTOR velocity;
	velocity.x = term.x/length;
	velocity.y = term.y/length;
	velocity.z = term.z/length;
		
 	Velocity.x += velocity.x;
	Velocity.y += velocity.y;
	Velocity.z += velocity.z;
 	if(length < 100)
	{
		Velocity.x = Velocity.x * 0.85f;
		Velocity.y = Velocity.y * 0.85f;
		Velocity.z = Velocity.z * 0.85f;
	}
	
  	length = (float)sqrt(Velocity.x*Velocity.x+Velocity.y*Velocity.y+Velocity.z*Velocity.z);
	if(length > 10)
	{
	 	Velocity.x = Velocity.x * 0.9f;
		Velocity.y = Velocity.y * 0.9f;
		Velocity.z = Velocity.z * 0.9f;
	}

  	Pos.x += Velocity.x * fONE;
	Pos.y += Velocity.y * fONE;
	Pos.z += Velocity.z * fONE;
	
	
  	if(ParticleID != -1)
	{
		g_NewParticleMgr.SetPos(ParticleID, Pos);
	}
	

  	if(ParticleIDExt1 != -1)
	{
		g_NewParticleMgr.SetAttachPos(ParticleIDExt1, Pos);
	}

}

void AssaRunicGuardianBossHit::Start(POINT3D *pCur, smCHAR *destChar, D3DVECTOR *velocity, int delay, smCHAR **charList, int charCount)
{
 	Pos.x = (float)pCur->x;
	Pos.y = (float)pCur->y;
	Pos.z = (float)pCur->z;

	DestChar = destChar;

	if(velocity != NULL)
	{
		Velocity.x = velocity->x;
		Velocity.y = velocity->y;
		Velocity.z = velocity->z;
	}
	POINT3D pos;
	pos.x = (int)Pos.x;
	pos.y = (int)Pos.y;
	pos.z = (int)Pos.z;
 	ParticleID = g_NewParticleMgr.Start("IronMonsterRunicGuardianBossWeapon1", pos);
 	ParticleIDExt1 = g_NewParticleMgr.Start("IronMonsterRunicGuardianBossWeapon2", pos);
	
	Max_Time = 2000;
	Time = -delay;

	CharCount = 0;

	TempMyIndex = -1;
	if(charCount > 0)
	{
		memcpy(CharList, charList, 4*charCount);
		CharCount = charCount;

		for(int index = 0; index < CharCount; index++)
		{
			if(CharList[index] == lpCurPlayer)
			{
				TempMyIndex = index;
				break;
			}
		}
	}
}




AssaRunicGuardianBossSkill::AssaRunicGuardianBossSkill()
{
	pChar = NULL;	
	MyCharacterFlag = FALSE;
	ParticleID = -1;
}

AssaRunicGuardianBossSkill::~AssaRunicGuardianBossSkill()
{
	
}

void AssaRunicGuardianBossSkill::Start(smCHAR* character, int liveCount)
{
     pChar = character;
	if(pChar == NULL)
		return;
	if(lpCurPlayer == pChar)
	{
		MyCharacterFlag = TRUE;
	}
	
	POINT3D charPos;
 	charPos.x = pChar->pX;
	charPos.y = pChar->pY+30000;
	charPos.z = pChar->pZ;

   	ParticleID = g_NewParticleMgr.Start("IronMonsterRunicGuardianBossSkill1", charPos, 0.2f);

	charPos.y = pChar->pY + 4000;
	int num = g_NewParticleMgr.Start("IronMonsterRunicGuardianBossJin1", charPos);
	POINT3D angle;
	angle.x = 0;
	angle.y = pChar->Angle.y;
	angle.z = 0;
	g_NewParticleMgr.SetAngle(num, angle);

	

 	
	EndFlag = FALSE;
  	Max_Time = liveCount+140;
}


void AssaRunicGuardianBossSkill::Main()
{	
   	 if(ParticleID == -1)
		return;
	
 	if(!g_NewParticleMgr.GetRunning(ParticleID) && EndFlag)
	{
		ParticleID = -1;
		return;
	}
	
  	if(EndFlag && (Time > (Max_Time-120)))
	{
		AssaParticle_MonsterRunicGuardianBossShot(pChar, SMMAT_BLEND_LAMP);	

		Time = Max_Time;
		ParticleID = -1;
	}

	if(MyCharacterFlag)
	{
		if(lpCurPlayer != pChar)
			pChar = lpCurPlayer;
	}
	
	//if(Time == 40)
  		//SetDynLight(pChar->pX, pChar->pY, pChar->pZ, 50, 100, 255,0, 140, 1);

	D3DVECTOR charPos;
	charPos.x = (float)pChar->pX;
	charPos.y = (float)pChar->pY + 30000;
	charPos.z = (float)pChar->pZ;
	g_NewParticleMgr.SetAttachPos(ParticleID, charPos);

	if(pChar->FlagShow == FALSE)
		g_NewParticleMgr.SetRendering(ParticleID, FALSE);
	else
		g_NewParticleMgr.SetRendering(ParticleID, TRUE);
	
 	if(EndFlag == FALSE)
	{
		if(Time > Max_Time-140 ||
		(!MyCharacterFlag && pChar->Flag == 0) ||
		(!pChar->MotionInfo || pChar->MotionInfo->State == CHRMOTION_STATE_DEAD) )
 		{
 			Time = Max_Time-140;
 			if(ParticleID != -1)
			{
 				g_NewParticleMgr.SetFastStop(ParticleID);
				g_NewParticleMgr.SetStop(ParticleID);
  				AssaParticle_MonsterRunicGuardianBossShot(pChar, SMMAT_BLEND_INVSHADOW);

			}
			EndFlag = TRUE;
		}
	}
}


AssaSkyParticle::AssaSkyParticle()
{
	ParticleID = -1;
	TimeCount = 0;
	ParticleSpawnFlag = false;
}

AssaSkyParticle::~AssaSkyParticle()
{

}

void AssaSkyParticle::Main()
{
	if(ParticleID != -1)
	{
		Pos.x += Velocity.x*256.f;
		Pos.y += Velocity.y*256.f;
		Pos.z += Velocity.z*256.f;

		g_NewParticleMgr.SetPos(ParticleID, Pos);
		TimeCount--;

		if(TimeCount <= 0)
		{
 			g_NewParticleMgr.SetStop(ParticleID);
			ParticleID = -1;
			Time = Max_Time;
		}

		int mapY1, mapY2;
		if(smGameStage[0])
		{
			mapY1 = smGameStage[0]->GetHeight((int)Pos.x, (int)Pos.z);
		}
		if(smGameStage[1])
		{
			mapY2 = smGameStage[1]->GetHeight((int)Pos.x, (int)Pos.z);
		}

		if(smGameStage[0] && smGameStage[1])
			if (mapY1 < mapY2) mapY1 = mapY2;
		if(smGameStage[0] == NULL && smGameStage[1])
			mapY1 = mapY2;
		
		//맵과 총돌을 했는지 검사한다.
		if(mapY1 > (int)Pos.y)
		{
			g_NewParticleMgr.SetStop(ParticleID);
			ParticleID = -1;
			Time = Max_Time;
			POINT3D hitPos;
			hitPos.x = (int)Pos.x;
			hitPos.y = (int)Pos.y;
			hitPos.z = (int)Pos.z;

 			AssaParticle_TerrainFireHit(&hitPos);
			D3DVECTOR vector;

			vector.x = float(hitPos.x - lpCurPlayer->pX)/fONE;
			vector.y = float(hitPos.y - lpCurPlayer->pY)/fONE;
			vector.z = float(hitPos.z - lpCurPlayer->pZ)/fONE;
			
			float length;
 			length = (float)sqrt(vector.x*vector.x+vector.y*vector.y+vector.z*vector.z);
			if(length == 0)
				length = 1;
			
			
			//esPlaySound( 21, int((300000.f-length)*0.01f)  );
			if(GameMode != 1)
   		  	esPlaySound( 21, 400-int(length/10.f));
		}

	}


	if(ParticleSpawnFlag == false)
	{
		Delay--;
		if(Delay <= 0)
		{
			POINT3D partPos;
			partPos.x = (int)Pos.x;
			partPos.y = (int)Pos.y;
			partPos.z = (int)Pos.z;

  			ParticleID = g_NewParticleMgr.Start("TerrainFire", partPos);
			ParticleSpawnFlag = true;
		}
	}
}

void AssaSkyParticle::Start(POINT3D *pCur, POINT3D *pDest, int delay)
{
    Pos.x = (float)pCur->x;
	Pos.y = (float)pCur->y;
	Pos.z = (float)pCur->z;

	D3DVECTOR term;
	term.x = float((pDest->x - Pos.x)/ fONE);
	term.y = float((pDest->y - Pos.y)/ fONE);
	term.z = float((pDest->z - Pos.z)/ fONE);
	
 	float length = term.x*term.x + term.y*term.y + term.z*term.z;
	length = (float)sqrt(length);


	Velocity.x = term.x/length*8.f;
	Velocity.y = term.y/length*8.f;
	Velocity.z = term.z/length*8.f;

	POINT3D partPos;
	partPos.x = (int)Pos.x;
	partPos.y = (int)Pos.y;
	partPos.z = (int)Pos.z;

	Delay = delay;
	if(delay == 1)
	{
		ParticleSpawnFlag = true;
  		ParticleID = g_NewParticleMgr.Start("TerrainFire", partPos);
	}

	if(delay == 2)
	{
		ParticleSpawnFlag = true;
  		ParticleID = g_NewParticleMgr.Start("ChaosKaraMeteo", partPos);
	}

 	Max_Time = 2000;
	TimeCount = 70*10;
	

	
	D3DVECTOR vector;
    vector.x = float(partPos.x - lpCurPlayer->pX)/fONE;
	vector.y = 0.f;
	vector.z = float(partPos.z - lpCurPlayer->pZ)/fONE;
	
	
  	length = (float)sqrt(vector.x*vector.x+vector.y*vector.y+vector.z*vector.z);
	if(length == 0)
		length = 1;
	
	if(GameMode != 1 )
  		esPlaySound( 20, 400-int(length/100.f));
}






AssaRunicGuardianLight::AssaRunicGuardianLight()
{
	ObjBip1 = NULL;
}

AssaRunicGuardianLight::~AssaRunicGuardianLight()
{

}

void AssaRunicGuardianLight::Draw(POINT3D *pPosi, POINT3D *pAngle)
{
  	if(Time >= (Max_Time-40))
		return;

  	smRender.Init(); //데이터 초기화 
	smMaterialGroup->smMaterial[AnimationID[AniIndex]].TwoSide = TRUE; //카메라 회전과 상관없이 보이기 
	smRender.SetMaterialGroup(smMaterialGroup);  //읽어드린 메트리얼을 연결 

	
	AddFace2DLine(&CurPos, &DesPos, 10000, AnimationID[AniIndex]);
	smRender.SetClipStates(SMCLIP_LEFT | SMCLIP_RIGHT | SMCLIP_TOP | SMCLIP_BOTTOM);
	smRender.ClipRendFace();
	smRender.GeomVertex2D(FALSE);

	//lpD3DDevice->SetRenderState( D3DRENDERSTATE_ZWRITEENABLE, TRUE );
 	smRender.RenderD3D();
	//lpD3DDevice->SetRenderState( D3DRENDERSTATE_ZWRITEENABLE, FALSE );

}

void AssaRunicGuardianLight::Main()
{
 	if(Time%AniDelay == 0)
	{
   	  	AniIndex++;
		if(AniIndex >= 3)
			AniIndex = 0;
	}

	if(pCurChar && pDestChar)
	{
 		if(CurMyCharacterFlag)
			pCurChar = lpCurPlayer;

		if(DestMyCharacterFlag)
			pDestChar = lpCurPlayer;

		if(ObjBip1 == NULL)
		{
			CurPos.x = pCurChar->pX;
			CurPos.y = pCurChar->pY + 10000;
			CurPos.z = pCurChar->pZ;
		}
		else
		{
			smMATRIX *mWorld;
 			AnimObjectTree( ObjBip1, pCurChar->frame, 0, (-pCurChar->Angle.y+ANGLE_180)&ANGCLIP, 0);
			mWorld = &ObjBip1->mWorld;
			
			CurPos.x = pCurChar->pX + mWorld->_41;
			CurPos.y = pCurChar->pY + mWorld->_43;
			CurPos.z = pCurChar->pZ + mWorld->_42;
		}

		DesPos.x = pDestChar->pX;
		DesPos.y = pDestChar->pY + 10000;
		DesPos.z = pDestChar->pZ;

		if(ParticleID1 != -1)
		{
			D3DVECTOR pos;
			pos.x = (float)CurPos.x;
			pos.y = (float)CurPos.y;
			pos.z = (float)CurPos.z;

			g_NewParticleMgr.SetPos(ParticleID1, pos);			
		}
		
		if(ParticleID2 != -1)
		{
			D3DVECTOR pos;
			pos.x = (float)DesPos.x;
			pos.y = (float)DesPos.y;
			pos.z = (float)DesPos.z;

			g_NewParticleMgr.SetPos(ParticleID2, pos);
		}

		int NearDist = 500*500;
		int x, y, z;
		x = (DesPos.x-CurPos.x) >> FLOATNS;
		y = (DesPos.y-CurPos.y) >> FLOATNS;
		z = (DesPos.z-CurPos.z) >> FLOATNS;
		int dist = x*x+z*z+y*y;
		
		if(dist >= NearDist)
		{
			Time = Max_Time;
		}
		
		if(pChar)
		{
			if(!pChar->MotionInfo || pChar->MotionInfo->State == CHRMOTION_STATE_DEAD)
			{
				Time = Max_Time;
			}
		}
	}
	
	if(Time >= (Max_Time-40))
		StopFlag = true;

	if(StopFlag)
	{
		if(ParticleID1 != -1)
		{
			g_NewParticleMgr.SetStop(ParticleID1);
			ParticleID1 = -1;
		}

		if(ParticleID2 != -1)
		{
			g_NewParticleMgr.SetStop(ParticleID2);
			ParticleID2 = -1;
		}
	}
}

void AssaRunicGuardianLight::Start(smCHAR *curChar, smCHAR *desChar,bool ObjectFlag, bool firstPartFlag, bool secondPartFlag)
{
   	AnimationID[0] = *(int *)AssaSearchRes("R_e001.bmp", SMMAT_BLEND_LAMP);
	AnimationID[1] = *(int *)AssaSearchRes("R_e002.bmp", SMMAT_BLEND_LAMP);
	AnimationID[2] = *(int *)AssaSearchRes("R_e003.bmp", SMMAT_BLEND_LAMP);

	Max_Time = 100;
	AniDelay = 1;
	AniIndex = 0;

	CurMyCharacterFlag = false;
	DestMyCharacterFlag = false;

	pCurChar = curChar;
	pDestChar = desChar;

	if(pCurChar == lpCurPlayer)
		CurMyCharacterFlag = true;

	if(pDestChar == lpCurPlayer)
		DestMyCharacterFlag = true;


	CurPos.x = pCurChar->pX;
	CurPos.y = pCurChar->pY+10000;
	CurPos.z = pCurChar->pZ;
	if(firstPartFlag)
	ParticleID1 = g_NewParticleMgr.Start("IronMonsterRunicGuardianLight3", CurPos);

	DesPos.x = pDestChar->pX;
	DesPos.y = pDestChar->pY + 10000;
	DesPos.z = pDestChar->pZ;
	if(secondPartFlag)
	ParticleID2 = g_NewParticleMgr.Start("IronMonsterRunicGuardianLight3", DesPos);
    
	if(ObjectFlag)
		ObjBip1 = curChar->AnimPattern->GetObjectFromName("Bip01 Effect");
	else
		ObjBip1 = NULL;

	StopFlag = false;
}



#define FLOATS_EM				(FLOATNS+3)
#define SM_DIST_NEARZ			16
#define SMFLOAT_DIST_NEARZ		(SM_DIST_NEARZ<<FLOATNS)

bool AssaRunicGuardianLight::GetCameraCoord(POINT3D &world)
{
	int rx, ry, rz;

    //중심 좌표를 카메라 좌표로 변환
	world.x -= smRender.CameraPosiX;
	world.y -= smRender.CameraPosiY;
	world.z -= smRender.CameraPosiZ;
	
	rx = world.x * smRender.mCamera._11 +
		 world.y * smRender.mCamera._21 +
		 world.z * smRender.mCamera._31 ;
	
	ry = world.x * smRender.mCamera._12 +
		 world.y * smRender.mCamera._22 +
		 world.z * smRender.mCamera._32 ;
	
	rz = world.x * smRender.mCamera._13 +
		 world.y * smRender.mCamera._23 +
		 world.z * smRender.mCamera._33 ;

	world.x = rx>>FLOATS_EM;
	world.y = ry>>FLOATS_EM;
	world.z = rz>>FLOATS_EM;

    if ( world.z<SMFLOAT_DIST_NEARZ )
		return FALSE;
	return TRUE;
}

bool AssaRunicGuardianLight::AddFace2DLine(POINT3D *curPos, POINT3D *desPos, int width, int matNum)
{
    
    
	smRENDVERTEX *v[4];
    smRENDFACE	 *rf;
    short		sColor[4];
	
	sColor[ SMC_A ] = 255;
    sColor[ SMC_R ] = 255;
    sColor[ SMC_G ] = 255;
    sColor[ SMC_B ] = 255;
	
	POINT3D currentPos;
	POINT3D destPos;
    
	width = width/2;
	
	currentPos.x   = curPos->x;
    currentPos.y   = curPos->y;
    currentPos.z   = curPos->z;
	
	destPos.x = desPos->x;
	destPos.y = desPos->y;
	destPos.z = desPos->z;
	
	if(GetCameraCoord(currentPos) == FALSE)
			return FALSE;

    if(GetCameraCoord(destPos) == FALSE)
			return FALSE;
	
	smTEXRECT texRect;
    texRect.left = 0;
    texRect.top  = 0;
    texRect.right = 1;
    texRect.bottom = 1;

    float dx     = float(destPos.x - currentPos.x); //대상 까지의 x축의 기울기를 구한다.
    float dy     = float(destPos.y - currentPos.y); //대상 까지의 y축의 기울기를 구한다.
    float length = (float)sqrt(dx*dx+dy*dy);        //대상 까지의 거리를 구한다.

    dx = dx/length*width; //날아가는 파티클의 넓이..
    dy = dy/length*width;

    D3DVECTOR persp;
    persp.x = -dy;
    persp.y = +dx;
    persp.z = 0;

    if (abs((int)dx) > 1.0f) currentPos.x -= (int)dx;
    if (abs((int)dy) > 1.0f) currentPos.y -= (int)dy;

    v[0] = smRender.AddRendVertex ( int(destPos.x-persp.x),    int(destPos.y-persp.y), int(destPos.z), sColor );
    v[1] = smRender.AddRendVertex ( int(destPos.x+persp.x),    int(destPos.y+persp.y), int(destPos.z), sColor );
    v[2] = smRender.AddRendVertex ( int(currentPos.x-persp.x), int(currentPos.y-persp.y), int(currentPos.z), sColor );
    v[3] = smRender.AddRendVertex ( int(currentPos.x+persp.x), int(currentPos.y+persp.y), int(currentPos.z), sColor );

    //첫번째 폴리곤 ( 1 - 2 - 3 );
    rf = &smRender.RendFace[ smRender.nRendFace++ ];
    rf->lpRendVertex[0] = v[0];
    rf->lpRendVertex[1] = v[1];
    rf->lpRendVertex[2] = v[2];
    rf->Matrial    = matNum;		//메트리얼 복사
    rf->ClipStatus = v[0]->ClipStatus|v[1]->ClipStatus|v[2]->ClipStatus;
    rf->lpTexLink  = 0;

    //텍스쳐 연결
    smRender.AddRendTempTexLink( rf , 0,	texRect.left , texRect.bottom,
                                texRect.right , texRect.bottom,
                                texRect.left , texRect.top );


    //두번째 폴리곤 ( 2 - 4 - 3 );
    rf = &smRender.RendFace[ smRender.nRendFace++ ];
    rf->lpRendVertex[0] = v[1];
    rf->lpRendVertex[1] = v[3];
    rf->lpRendVertex[2] = v[2];
    rf->Matrial    = matNum;		//메트리얼 복사
    rf->ClipStatus = v[1]->ClipStatus|v[2]->ClipStatus|v[3]->ClipStatus;
    rf->lpTexLink  = 0;

    //텍스쳐 연결
    smRender.AddRendTempTexLink( rf , 0,	texRect.right , texRect.bottom,
                                            texRect.right , texRect.top,
                                            texRect.left  , texRect.top );
	return true;
}




AssaShelltomShot::AssaShelltomShot()
{
	ParticleID = -1;
	memset(&ShootingAngle, 0, sizeof(ShootingAngle));
	PatMesh = NULL;
}

AssaShelltomShot::~AssaShelltomShot()
{

}

void AssaShelltomShot::Draw(POINT3D *pPosi, POINT3D *pAngle)
{
	if(PatMesh != NULL)
	{
    	smRender.Init(); //데이터 초기화 
		smRender.SetCameraPosi(pPosi->x, pPosi->y, pPosi->z,pAngle->x,pAngle->y,pAngle->z);
		
		POINT3D meshAngle;
    		meshAngle.x = 0;
  		meshAngle.y = ShootingAngle.z;
 		meshAngle.z = 0;
		PatMesh->Frame = 0;
		PatMesh->SetPosi(&Posi, &meshAngle);
		
		PatMesh->RenderD3D(pPosi, pAngle);
	}
}

void AssaShelltomShot::Start(POINT3D *pCur, POINT3D *pDest, int shellType)
{
	Posi.x = pCur->x;
	Posi.y = pCur->y;
	Posi.z = pCur->z;
	
	DesPosi.x = pDest->x;
	DesPosi.y = pDest->y;
	DesPosi.z = pDest->z;
	
	ShootingAngle.z = 0;
	Max_Time = 5000;
	m_ShellType = shellType;
	if(m_ShellType == 0) //파데오
 		PatMesh = (smPAT3D *)AssaSearchRes("stm01.ASE", SMMAT_BLEND_LAMP);
	else if(m_ShellType == 1) //스파키
		PatMesh = (smPAT3D *)AssaSearchRes("stm02.ASE", SMMAT_BLEND_LAMP);
	else if(m_ShellType == 2) //쎄레네오
		PatMesh = (smPAT3D *)AssaSearchRes("stm03.ASE", SMMAT_BLEND_LAMP);
	else if(m_ShellType == 3) //밤던지기
		PatMesh = (smPAT3D *)AssaSearchRes("molybam.ASE", SMMAT_BLEND_ALPHA);
	
	if(m_ShellType != 3)
		ParticleID = g_NewParticleMgr.Start("Shelltom", Posi);

	//Step = 4*fONE;
	//이동할 위치에 대한 벡터의 정규화를 위해서.....
	//WorldY = WorldY+1000;
 	
	D3DVECTOR term;
 	term.x = float((DesPosi.x - Posi.x)/ fONE);
	term.y = float((DesPosi.y - Posi.y)/ fONE);
	term.z = float((DesPosi.z - Posi.z)/ fONE);
		
	float length = term.x*term.x + term.y*term.y + term.z*term.z;
	length = (float)sqrt(length);
	
	if(length >= 30000)
		VelocityY = 800;
	else
		VelocityY = 400;
}

void AssaShelltomShot::Main()
{
 	ShootingAngle.z += fONE * 2;
 	ShootingAngle.z = (ShootingAngle.z & ANGCLIP);
	POINT3D pos;
 	pos.x = int(Posi.x);
	pos.y = int(Posi.y);
	pos.z = int(Posi.z);
    
	
	D3DVECTOR term;
 	term.x = float((DesPosi.x - pos.x)/ fONE);
	term.y = float((DesPosi.y - pos.y)/ fONE);
	term.z = float((DesPosi.z - pos.z)/ fONE);
		
	float length = term.x*term.x + term.y*term.y + term.z*term.z;
	length = (float)sqrt(length);
	
	if(length < 20000)
	{
		
		if(length > 10000)
			VelocityY += int(term.y/length * 140.0f);
		else
			VelocityY += int(term.y/length * 80.0f);

 		if(VelocityY >= 5000)
		{
			VelocityY *= 0.75f;
		}

    	Posi.x += int((term.x/length) * (256.f*3.f));
		Posi.z += int((term.z/length) * (256.f*3.f));
		Posi.y += (int)VelocityY;
		
 		if(length < 25 || Time > 60)
		{
   			if(ParticleID != -1)
			{
				g_NewParticleMgr.SetStop(ParticleID);
			}
			Time = Max_Time;
			if(m_ShellType != 3)
			{
				esPlaySound( 22, GetDistVolume( pos.x, pos.y, pos.z) );		
				g_NewParticleMgr.Start("ShelltomHit1", pos);
			}
			else
			{
				esPlaySound( 23, GetDistVolume( pos.x, pos.y, pos.z) );		
				g_NewParticleMgr.Start("Bam", pos);
			}
		}
	}
	
	D3DVECTOR partPos;
	partPos.x = (float)Posi.x;
	partPos.y = (float)Posi.y;
	partPos.z = (float)Posi.z;
	
	if(ParticleID != -1)
		g_NewParticleMgr.SetPos(ParticleID, partPos);
}







AssaShelltomUpWeapon::AssaShelltomUpWeapon()
{

}

AssaShelltomUpWeapon::~AssaShelltomUpWeapon()
{

}

void AssaShelltomUpWeapon::Start(smCHAR *pCharacter, int timeCount)
{
	pChar = pCharacter;
	if(pChar == NULL)
		return;
	
	if(lpCurPlayer == pChar)
	{
		MyCharacterFlag = TRUE;
	}
	
	Max_Time = 5000;
	EndFlag = FALSE;
	
	ParticleID[0] = -1;
	ParticleID[1] = -1;

	POINT3D curPos;
	curPos.x = pChar->pX;
	curPos.y = pChar->pY;
	curPos.z = pChar->pZ;
	
	ParticleID[0] = g_NewParticleMgr.Start("ShellTomUp1", curPos);
	ParticleID[1] = g_NewParticleMgr.Start("ShellTomUp2", curPos);

	ZPos[0] = fONE*5;
	ZPos[1] = 0;

	TimeCount = timeCount;

	//CODE = SKILL_DANCING_SWORD;
}

void AssaShelltomUpWeapon::Main()
{
	if(MyCharacterFlag)
	{
 		if(lpCurPlayer != pChar)
			pChar = lpCurPlayer;
	}
	
	 if(Time >= (TimeCount))
		EndFlag = TRUE;
	
	if(EndFlag)
	{
		Time = Max_Time;

		if(ParticleID[0] != -1)
		{
			g_NewParticleMgr.SetStop(ParticleID[0]);
			ParticleID[0] = -1;
		}
		
		if(ParticleID[1] != -1)
		{
			g_NewParticleMgr.SetStop(ParticleID[1]);
			ParticleID[1] = -1;
		}
		return;
	}
	
	POINT3D weaponPos1;
	POINT3D weaponPos2;

	smMATRIX *mWorld;
    memset(&weaponPos1, 0, sizeof(weaponPos1));
	memset(&weaponPos2, 0, sizeof(weaponPos2));

 	if(pChar->HvRightHand.dwItemCode != 0)
	{
		
		POINT3D curPos;
  		curPos.x = pChar->pX;
		curPos.y = pChar->pY;
		curPos.z = pChar->pZ;
		
		if(ParticleID[0] == -1)
			ParticleID[0] = g_NewParticleMgr.Start("ShellTomUp1", curPos);

		if(ParticleID[1] == -1)
			ParticleID[1] = g_NewParticleMgr.Start("ShellTomUp2", curPos);

		mWorld = &pChar->HvRightHand_Matrix;
		if((pChar->HvRightHand.dwItemCode & sinITEM_MASK2) == sinWT1) //제블린일 경우..
		{
		 	weaponPos1.z = -pChar->HvRightHand.SizeMax + ZPos[0];
			weaponPos2.z = -pChar->HvRightHand.SizeMax + ZPos[1];

			ZPos[0] += (pChar->HvRightHand.SizeMax)/200;
			ZPos[1] += (pChar->HvRightHand.SizeMax)/200;
			if(ZPos[0] >= (pChar->HvRightHand.SizeMax))
				ZPos[0] = 0;
			if(ZPos[1] >= (pChar->HvRightHand.SizeMax))
				ZPos[1] = 0;
		}
		else if((pChar->HvRightHand.dwItemCode & sinITEM_MASK2) == sinWP1)
		{
			weaponPos1.z = ZPos[0];
			weaponPos2.z = ZPos[1];
			
		 	ZPos[0] += pChar->HvRightHand.SizeMax/300;
			ZPos[1] += pChar->HvRightHand.SizeMax/300;
			
			if(ZPos[0] >= (pChar->HvRightHand.SizeMax/2))
				ZPos[0] = 0;
			if(ZPos[1] >= (pChar->HvRightHand.SizeMax/2))
				ZPos[1] = 0;
		}
		else if((pChar->HvRightHand.dwItemCode & sinITEM_MASK2) == sinWC1) //클러일 경우..
		{
  			weaponPos1.x = pChar->HvRightHand.SizeMax+ZPos[0];
            weaponPos2.x = pChar->HvRightHand.SizeMax+ZPos[1];
			
			ZPos[0] += 10;
			ZPos[1] += 10;
			if(ZPos[0] >= (pChar->HvRightHand.SizeMax))
				ZPos[0] = 0;
			if(ZPos[1] >= (pChar->HvRightHand.SizeMax))
				ZPos[1] = 0;

 		}
		else if((pChar->HvRightHand.dwItemCode & sinITEM_MASK2) ==  sinWS1)//활인경우..
		{
   			weaponPos1.z = ZPos[0]+pChar->HvRightHand.SizeMax/2;
			weaponPos2.z = ZPos[1]+pChar->HvRightHand.SizeMax/2;
			
			int cnt;
    		if( pChar->HvRightHand.dwItemCode != 0) //활을 든경우 보정..
			{
				cnt = (ANGLE_45)&ANGCLIP;
				GetMoveLocation(weaponPos1.x, weaponPos1.y, weaponPos1.z, 0, cnt, 0);
				weaponPos1.x = GeoResult_X;
				weaponPos1.y = GeoResult_Y;
				weaponPos1.z = GeoResult_Z;

                GetMoveLocation(weaponPos2.x, weaponPos2.y, weaponPos2.z, 0, cnt, 0);
				weaponPos2.x = GeoResult_X;
				weaponPos2.y = GeoResult_Y;
				weaponPos2.z = GeoResult_Z;
			}
		}	
		else
		{
 		 	weaponPos1.z = ZPos[0];
			weaponPos2.z = ZPos[1];
			
		 	ZPos[0] += pChar->HvRightHand.SizeMax/200;
			ZPos[1] += pChar->HvRightHand.SizeMax/200;
			
			if(ZPos[0] >= (pChar->HvRightHand.SizeMax-fONE*5))
				ZPos[0] = 0;
			if(ZPos[1] >= (pChar->HvRightHand.SizeMax-fONE*5))
				ZPos[1] = 0;
		}
	}
	else if(pChar->HvLeftHand.dwItemCode != 0)
	{
		POINT3D curPos;
		curPos.x = pChar->pX;
		curPos.y = pChar->pY;
		curPos.z = pChar->pZ;
		
		if(ParticleID[0] == -1)
			ParticleID[0] = g_NewParticleMgr.Start("ShellTomUp1", curPos);

		if(ParticleID[1] == -1)
			ParticleID[1] = g_NewParticleMgr.Start("ShellTomUp2", curPos);

		mWorld = &pChar->HvLeftHand_Matrix;
		weaponPos1.z = pChar->HvLeftHand.SizeMax-10*fONE + ZPos[0];
		weaponPos2.z = pChar->HvLeftHand.SizeMax-10*fONE + ZPos[1];
		int cnt;
		if( pChar->HvLeftHand.dwItemCode != 0) //활을 든경우 보정..
		{
			cnt = (ANGLE_90)&ANGCLIP;
			GetMoveLocation(weaponPos1.x, weaponPos1.y, weaponPos1.z, 0, cnt, 0);
			weaponPos1.x = GeoResult_X;
			weaponPos1.y = GeoResult_Y;
			weaponPos1.z = GeoResult_Z;

            GetMoveLocation(weaponPos2.x, weaponPos2.y, weaponPos2.z, 0, cnt, 0);
			weaponPos2.x = GeoResult_X;
			weaponPos2.y = GeoResult_Y;
			weaponPos2.z = GeoResult_Z;
		}
	}
	else //아무 무기도 들지 않았을 경우. 파티클을 끈다.
	{
		if(ParticleID[0] != -1)
		{
			g_NewParticleMgr.SetStop(ParticleID[0]);
			ParticleID[0] = -1;
		}

		if(ParticleID[1] != -1)
		{
			g_NewParticleMgr.SetStop(ParticleID[1]);
			ParticleID[1] = -1;
		}
		return;
	} 

	//양손 무기 5도 기울어짐..
	int cnt;
	int sin;
	int cos;
	if ( pChar->dwActionItemTwoHand ) 
	{
		cnt = (-ANGLE_45/5)&ANGCLIP;
			
		sin = GetSin[cnt]>>8;
		cos = GetCos[cnt]>>8;
		
		weaponPos1.y = -( weaponPos1.z * sin )>>8;
		weaponPos1.z =  ( weaponPos1.z * cos )>>8;

        weaponPos2.y = -( weaponPos1.z * sin )>>8;
		weaponPos2.z =  ( weaponPos1.z * cos )>>8;
	}
	
	POINT3D angle;
  	memcpy( &angle, &pChar->Angle , sizeof( POINT3D ) );
	angle.y = (-angle.y+ANGLE_180)&ANGCLIP;
	
	if(pChar->HvRightHand.dwItemCode != 0)
		AnimObjectTree( pChar->HvRightHand.ObjBip, pChar->frame, angle.x,angle.y,angle.z);
	
	int rx, ry, rz;
	rx = weaponPos1.x * mWorld->_11 +
		 weaponPos1.y * mWorld->_21 +
 		 weaponPos1.z * mWorld->_31;

	ry = weaponPos1.x * mWorld->_12 +
	     weaponPos1.y * mWorld->_22 +
	     weaponPos1.z * mWorld->_32;

	rz = weaponPos1.x * mWorld->_13 +
		 weaponPos1.y * mWorld->_23 +
		 weaponPos1.z * mWorld->_33;
	
	weaponPos1.x = (pChar->pX + (rx>>FLOATNS) + mWorld->_41);
	weaponPos1.z = (pChar->pZ + (ry>>FLOATNS) + mWorld->_42);
	weaponPos1.y = (pChar->pY + (rz>>FLOATNS) + mWorld->_43);
    
    rx = weaponPos2.x * mWorld->_11 +
		 weaponPos2.y * mWorld->_21 +
 		 weaponPos2.z * mWorld->_31;

	ry = weaponPos2.x * mWorld->_12 +
	     weaponPos2.y * mWorld->_22 +
	     weaponPos2.z * mWorld->_32;

	rz = weaponPos2.x * mWorld->_13 +
		 weaponPos2.y * mWorld->_23 +
		 weaponPos2.z * mWorld->_33;

	weaponPos2.x = (pChar->pX + (rx>>FLOATNS) + mWorld->_41);
	weaponPos2.z = (pChar->pZ + (ry>>FLOATNS) + mWorld->_42);
	weaponPos2.y = (pChar->pY + (rz>>FLOATNS) + mWorld->_43);


	D3DVECTOR pos1, pos2;
	pos1.x = (float)weaponPos1.x;
	pos1.y = (float)weaponPos1.y;
	pos1.z = (float)weaponPos1.z;

	pos2.x = (float)weaponPos2.x;
	pos2.y = (float)weaponPos2.y;
	pos2.z = (float)weaponPos2.z;
	
	g_NewParticleMgr.SetAttachPos(ParticleID[0], pos1);
	g_NewParticleMgr.SetRendering(ParticleID[0], pChar->FlagShow);

	g_NewParticleMgr.SetAttachPos(ParticleID[1], pos2);
	g_NewParticleMgr.SetRendering(ParticleID[1], pChar->FlagShow);
}



BOOL AssaShelltomUpWeapon::GetCameraCoord(POINT3D &world)
{
	int rx, ry, rz;

    //중심 좌표를 카메라 좌표로 변환
	world.x -= smRender.CameraPosiX;
	world.y -= smRender.CameraPosiY;
	world.z -= smRender.CameraPosiZ;
	
	rx = world.x * smRender.mCamera._11 +
		 world.y * smRender.mCamera._21 +
		 world.z * smRender.mCamera._31 ;
	
	ry = world.x * smRender.mCamera._12 +
		 world.y * smRender.mCamera._22 +
		 world.z * smRender.mCamera._32 ;
	
	rz = world.x * smRender.mCamera._13 +
		 world.y * smRender.mCamera._23 +
		 world.z * smRender.mCamera._33 ;

	world.x = rx>>FLOATS_EM;
	world.y = ry>>FLOATS_EM;
	world.z = rz>>FLOATS_EM;

    if ( world.z<SMFLOAT_DIST_NEARZ )
		return FALSE;
	return TRUE;
}


BOOL AssaShelltomUpWeapon::AddFace2DLine(POINT3D *curPos, POINT3D *desPos, int width, int matNum)
{
	smRENDVERTEX *v[4];
    smRENDFACE	 *rf;
    short		sColor[4];
 	
	sColor[ SMC_A ] = 255;
    sColor[ SMC_R ] = 255;
    sColor[ SMC_G ] = 255;
     sColor[ SMC_B ] = 255;
	
	POINT3D currentPos;
	POINT3D destPos;
    
	width = width/2;
	
	currentPos.x   = curPos->x;
    currentPos.y   = curPos->y;
    currentPos.z   = curPos->z;
	
	destPos.x = desPos->x;
	destPos.y = desPos->y;
	destPos.z = desPos->z;
	
	if(GetCameraCoord(currentPos) == FALSE)
			return FALSE;

    if(GetCameraCoord(destPos) == FALSE)
			return FALSE;
	
	smTEXRECT texRect;
    texRect.left = 0;
    texRect.top  = 0;
    texRect.right = 1;
    texRect.bottom = 1;

    float dx     = float(destPos.x - currentPos.x); //대상 까지의 x축의 기울기를 구한다.
    float dy     = float(destPos.y - currentPos.y); //대상 까지의 y축의 기울기를 구한다.
    float length = (float)sqrt(dx*dx+dy*dy);        //대상 까지의 거리를 구한다.

    dx = dx/length*width; //날아가는 파티클의 넓이..
    dy = dy/length*width;

    D3DVECTOR persp;
    persp.x = -dy;
    persp.y = +dx;
    persp.z = 0;

    if (abs((int)dx) > 1.0f) currentPos.x -= (int)dx;
    if (abs((int)dy) > 1.0f) currentPos.y -= (int)dy;

    v[0] = smRender.AddRendVertex ( int(destPos.x-persp.x),    int(destPos.y-persp.y), int(destPos.z), sColor );
    v[1] = smRender.AddRendVertex ( int(destPos.x+persp.x),    int(destPos.y+persp.y), int(destPos.z), sColor );
    v[2] = smRender.AddRendVertex ( int(currentPos.x-persp.x), int(currentPos.y-persp.y), int(currentPos.z), sColor );
    v[3] = smRender.AddRendVertex ( int(currentPos.x+persp.x), int(currentPos.y+persp.y), int(currentPos.z), sColor );

    //첫번째 폴리곤 ( 1 - 2 - 3 );
    rf = &smRender.RendFace[ smRender.nRendFace++ ];
    rf->lpRendVertex[0] = v[0];
    rf->lpRendVertex[1] = v[1];
    rf->lpRendVertex[2] = v[2];
    rf->Matrial    = matNum;		//메트리얼 복사
    rf->ClipStatus = v[0]->ClipStatus|v[1]->ClipStatus|v[2]->ClipStatus;
    rf->lpTexLink  = 0;

    //텍스쳐 연결
    smRender.AddRendTempTexLink( rf , 0,	texRect.left , texRect.bottom,
                                texRect.right , texRect.bottom,
                                texRect.left , texRect.top );


    //두번째 폴리곤 ( 2 - 4 - 3 );
    rf = &smRender.RendFace[ smRender.nRendFace++ ];
    rf->lpRendVertex[0] = v[1];
    rf->lpRendVertex[1] = v[3];
    rf->lpRendVertex[2] = v[2];
    rf->Matrial    = matNum;		//메트리얼 복사
    rf->ClipStatus = v[1]->ClipStatus|v[2]->ClipStatus|v[3]->ClipStatus;
    rf->lpTexLink  = 0;

    //텍스쳐 연결
    smRender.AddRendTempTexLink( rf , 0,	texRect.right , texRect.bottom,
                                            texRect.right , texRect.top,
                                            texRect.left  , texRect.top );
	return TRUE;
}


AssaShelltomUpWeapon1::AssaShelltomUpWeapon1()
{

}

AssaShelltomUpWeapon1::~AssaShelltomUpWeapon1()
{

}

void AssaShelltomUpWeapon1::Start(smCHAR *pCharacter, int timeCount)
{
	pChar = pCharacter;
	if(pChar == NULL)
		return;
	
	if(lpCurPlayer == pChar)
	{
		MyCharacterFlag = TRUE;
	}
	
	Max_Time = 5000;
	EndFlag = FALSE;
	
	TimeCount = timeCount;
	
	//CODE = SKILL_DANCING_SWORD;
   	int *matNum = (int *)AssaSearchRes("force.bmp", SMMAT_BLEND_LAMP);
 	if(matNum == NULL)
		return;

    Face.MatNum = *matNum;
 	Face.r = 255;
	Face.g = 255;
	Face.b = 255;
 	Face.Transparency = 255;
  	Face.width = 500;
    	cAssaTrace->SetLength(30);
	
  	LocalPos.z = 0;
 	LocalPos.y = 700;
    LocalPosTransFlag = FALSE;
}


void AssaShelltomUpWeapon1::Main()
{
	if(MyCharacterFlag)
	{
 		if(lpCurPlayer != pChar)
			pChar = lpCurPlayer;
	}
	
	 if(Time >= (TimeCount))
		EndFlag = TRUE;
	
	if(EndFlag)
	{
		Time = Max_Time;
		return;
	}
	

	smMATRIX *mWorld;
	POINT3D weaponPos;
   	memset(&weaponPos, 0, sizeof(weaponPos));
    if(pChar->HvRightHand.dwItemCode != 0)
	{
		mWorld = &pChar->HvRightHand_Matrix;
 		if((pChar->HvRightHand.dwItemCode & sinITEM_MASK2) == sinWT1) //제블린일 경우..
		{
			weaponPos.z = -pChar->HvRightHand.SizeMax+int((float)pChar->HvRightHand.SizeMax*0.6f);
		}
		else if((pChar->HvRightHand.dwItemCode & sinITEM_MASK2) == sinWS2) //칼인경우..
		{
        	if(pChar->HvRightHand.SizeMax > 5000)
			{
 				weaponPos.z = LocalPos.z;
			}
			else
				weaponPos.z = pChar->HvRightHand.SizeMax-int((float)pChar->HvRightHand.SizeMax*0.2f);
			
			
 			if(LocalPosTransFlag == FALSE)
			{
				LocalPos.z += 160;
				if(LocalPos.z > 5000)
					LocalPosTransFlag = TRUE;
			}
			else
			{
				LocalPos.z -= 160;
				if(LocalPos.z < 0)
					LocalPosTransFlag = FALSE;
			}
			

		}
		else if((pChar->HvRightHand.dwItemCode & sinITEM_MASK2) == sinWC1) //클러일 경우..
		{
  			weaponPos.x = pChar->HvRightHand.SizeMax+pChar->HvRightHand.SizeMax;
 		}
		else if((pChar->HvRightHand.dwItemCode & sinITEM_MASK2) ==  sinWM1)//완드인경우..
  			weaponPos.z = pChar->HvRightHand.SizeMax-int((float)pChar->HvRightHand.SizeMax*0.3f);
		else if((pChar->HvRightHand.dwItemCode & sinITEM_MASK2) ==  sinWS1)//활인경우..
		{
   			weaponPos.z = pChar->HvRightHand.SizeMax-int((float)pChar->HvRightHand.SizeMax*0.3f);
			int cnt;
    		if( pChar->HvRightHand.dwItemCode != 0) //활을 든경우 보정..
			{
				cnt = (ANGLE_45)&ANGCLIP;
				GetMoveLocation(weaponPos.x, weaponPos.y, weaponPos.z, 0, cnt, 0);
				weaponPos.x = GeoResult_X;
				weaponPos.y = GeoResult_Y;
				weaponPos.z = GeoResult_Z;
			}
		}	
		else
			weaponPos.z = pChar->HvRightHand.SizeMax-int((float)pChar->HvRightHand.SizeMax*0.5f);
	}
	else if(pChar->HvLeftHand.dwItemCode != 0)
	{
 		if((pChar->HvLeftHand.dwItemCode & sinITEM_MASK2) == sinWS1) //활인 경우
		{
			mWorld = &pChar->HvLeftHand_Matrix;
 			weaponPos.z = pChar->HvLeftHand.SizeMax-10*fONE;
			int cnt;
			if( pChar->HvLeftHand.dwItemCode != 0) //활을 든경우 보정..
			{
				cnt = (ANGLE_90)&ANGCLIP;
				GetMoveLocation(weaponPos.x, weaponPos.y, weaponPos.z, 0, cnt, 0);
				weaponPos.x = GeoResult_X;
				weaponPos.y = GeoResult_Y;
				weaponPos.z = GeoResult_Z;
			}
		}
		else //다른 무기를 들고 있고 오른손에 무기를 들고 있지 않을 경우
		{
			Time = Max_Time;
			//StopParticle();
			//StopParticleID();
			EndFlag = TRUE;
			CODE = 0;
			return;
		}
	}
	else //손에 무기를 들고 있지 않을 경우.
	{
		Time = Max_Time;
		//StopParticle();
		//StopParticleID();
		EndFlag = TRUE;
		CODE = 0;
		return;
	}
	
	//양손 무기 5도 기울어짐..
	int cnt;
	int sin;
	int cos;
	if ( pChar->dwActionItemTwoHand ) 
	{
		cnt = (-ANGLE_45/5)&ANGCLIP;
			
		sin = GetSin[cnt]>>8;
		cos = GetCos[cnt]>>8;
		
		weaponPos.y = -( weaponPos.z * sin )>>8;
		weaponPos.z =  ( weaponPos.z * cos )>>8;
	}

     LocalAngle += 80;
	if(LocalAngle >= 4096)
	LocalAngle = 0;
	
	/*
    if(LocalPosTransFlag == FALSE)
	{
		LocalPos.y -= 10;
		if(LocalPos.y < 0)
			LocalPosTransFlag = TRUE;
	}
	else
	{
		LocalPos.y += 10;
		if(LocalPos.y > 1600)
			LocalPosTransFlag = FALSE;
	}
	*/

	POINT3D part1Pos;
	POINT3D part2Pos;

  	memcpy(&part1Pos, &weaponPos, sizeof(POINT3D));
	memcpy(&part2Pos, &weaponPos, sizeof(POINT3D));
   	if(pChar->HvRightHand.dwItemCode != 0)
	{
 		//GetMoveLocation( 0, LocalPos.y, 0, 0, 0, LocalAngle);
   		if((pChar->HvRightHand.dwItemCode & sinITEM_MASK2) == sinWC1 || //클러일 경우.
		   (pChar->HvRightHand.dwItemCode & sinITEM_MASK2) == sinWS1)	
			GetMoveLocation( 0, LocalPos.y, 0, LocalAngle, 0, 0);
		else
			GetMoveLocation( 0, LocalPos.y, 0, 0, 0, LocalAngle);
	}
	else if(pChar->HvLeftHand.dwItemCode != 0)
		GetMoveLocation( 0, LocalPos.y, 0, LocalAngle, 0, 0);

	part1Pos.x += GeoResult_X;
	part1Pos.y += GeoResult_Y;
	part1Pos.z += GeoResult_Z;
    
  	if(pChar->HvRightHand.dwItemCode != 0)
	{
		if((pChar->HvRightHand.dwItemCode & sinITEM_MASK2) == sinWC1 || //클러인 경우.
		   (pChar->HvRightHand.dwItemCode & sinITEM_MASK2) == sinWS1 ) 
 			GetMoveLocation( 0, -LocalPos.y, 0, LocalAngle, 0, 0);
		else //클러일경우.
			GetMoveLocation( 0, -LocalPos.y, 0, 0, 0, LocalAngle);
			
	}
	else if(pChar->HvLeftHand.dwItemCode != 0)
		GetMoveLocation( 0, -LocalPos.y, 0, LocalAngle, 0, 0);


	part2Pos.x += GeoResult_X;
	part2Pos.y += GeoResult_Y;
	part2Pos.z += GeoResult_Z;

	POINT3D angle;
  	memcpy( &angle, &pChar->Angle , sizeof( POINT3D ) );
	angle.y = (-angle.y+ANGLE_180)&ANGCLIP;
	
	if(pChar->HvRightHand.dwItemCode != 0)
		AnimObjectTree( pChar->HvRightHand.ObjBip, pChar->frame, angle.x,angle.y,angle.z);
	else if(pChar->HvLeftHand.dwItemCode != 0)
		AnimObjectTree( pChar->HvLeftHand.ObjBip, pChar->frame, angle.x,angle.y,angle.z);


	int rx, ry, rz;
	rx = part1Pos.x * mWorld->_11 +
		 part1Pos.y * mWorld->_21 +
 		 part1Pos.z * mWorld->_31 ;

	ry = part1Pos.x * mWorld->_12 +
	     part1Pos.y * mWorld->_22 +
	     part1Pos.z * mWorld->_32 ;

	rz = part1Pos.x * mWorld->_13 +
		 part1Pos.y * mWorld->_23 +
		 part1Pos.z * mWorld->_33 ;
	
	D3DVECTOR charPos;
	charPos.x = (float)(pChar->pX + (rx>>FLOATNS) + mWorld->_41);
	charPos.z = (float)(pChar->pZ + (ry>>FLOATNS) + mWorld->_42);
	charPos.y = (float)(pChar->pY + (rz>>FLOATNS) + mWorld->_43);
	
	rx = part2Pos.x * mWorld->_11 +
		 part2Pos.y * mWorld->_21 +
 		 part2Pos.z * mWorld->_31 ;

	ry = part2Pos.x * mWorld->_12 +
	     part2Pos.y * mWorld->_22 +
	     part2Pos.z * mWorld->_32 ;

	rz = part2Pos.x * mWorld->_13 +
		 part2Pos.y * mWorld->_23 +
		 part2Pos.z * mWorld->_33 ;

	charPos.x = (float)(pChar->pX + (rx>>FLOATNS) + mWorld->_41);
	charPos.z = (float)(pChar->pZ + (ry>>FLOATNS) + mWorld->_42);
	charPos.y = (float)(pChar->pY + (rz>>FLOATNS) + mWorld->_43);
	
	POINT3D pos;
	pos.x = (int)charPos.x;
	pos.y = (int)charPos.y;
	pos.z = (int)charPos.z;

	cAssaTrace->AddData(&pos);

}

void AssaShelltomUpWeapon1::Draw(POINT3D *pPosi, POINT3D *pAngle)
{
	smRender.Init(); //데이터 초기화 
	smMaterialGroup->smMaterial[Face.MatNum].TwoSide = TRUE; //카메라 회전과 상관없이 보이기 
	smRender.SetMaterialGroup(smMaterialGroup);  //읽어드린 메트리얼을 연결 
	
	
	if(!AssaAddFaceTrace(&Face, cAssaTrace))
			return;

	smRender.SetClipStates(SMCLIP_LEFT | SMCLIP_RIGHT | SMCLIP_TOP | SMCLIP_BOTTOM);
	smRender.ClipRendFace();
	smRender.GeomVertex2D(FALSE);
	smRender.RenderD3D();
}







AssaShelltomUpWeapon2::AssaShelltomUpWeapon2()
{
	//TimeCount = 0;
	MaterialNum = 0;
	MyCharacterFlag = FALSE;
}

AssaShelltomUpWeapon2::~AssaShelltomUpWeapon2()
{

}

void AssaShelltomUpWeapon2::Start(smCHAR *pCharacter, int timeCount, int shellTomCode)
{
 	pChar = pCharacter;
	if(pChar == NULL)
		return;

	if(lpCurPlayer == pChar)
	{
		MyCharacterFlag = TRUE;
	}
	
	EndFlag = FALSE;
	Max_Time = timeCount;
	
	int *matNum = (int *)AssaSearchRes("flare.tga", SMMAT_BLEND_LAMP);
 	if(matNum == NULL)
		return;

    if(shellTomCode == 0) //러씨디
	{
  		Face.r = 80;
		Face.g = 20;
		Face.b = 150;
	}
	else if(shellTomCode == 1) //세레네오
	{
		Face.r = 80;
		Face.g = 20;
		Face.b = 150;
	}
	else if(shellTomCode == 2) //파데오
	{
		Face.r = 80;
		Face.g = 20;
		Face.b = 150;
	}
	else if(shellTomCode == 3) //스파키
	{
		Face.r = 80;
		Face.g = 20;
		Face.b = 150;
	}
	else if(shellTomCode == 4) //레이던트
	{
 		Face.r = 140;
		Face.g = 90;
		Face.b = 20;
	}
	else if(shellTomCode == 5) //트랜스패로
	{
   		Face.r = 20;
		Face.g = 100;
		Face.b = 20;
	}
	else if(shellTomCode == 6) //머키
	{
  		Face.r = 0;
		Face.g = 100;
		Face.b = 200;
	}
	else if(shellTomCode == 7) //데비네
	{
 		Face.r = 190;
		Face.g = 30;
		Face.b = 150;
	}
	else if(shellTomCode == 8) //셀레스토
 	{
  		Face.r = 255;
		Face.g = 40;
 		Face.b = 20;
	}
	// 박재원 : 빌링 포스 추가(무기 이펙트 조정)
	else if(shellTomCode == 9) //미라지
	{
		Face.r = 220;
		Face.g = 500;
		Face.b = 10;
	}
	// 박재원 : 빌링 포스 추가(무기 이펙트 조정)
	else if(shellTomCode == 10) //인페르나
	{
		Face.r = 10;
		Face.g = 255;
		Face.b = 10;
	}
	// 박재원 : 빌링 포스 추가(무기 이펙트 조정)
	else if(shellTomCode == 11) //이니그마
	{
		Face.r = 255;
		Face.g = 255;
		Face.b = 255;
	}
	
	Face.MatNum = *matNum;
  	Face.Transparency = 120;
  	Face.width  = 15*fONE;
	Face.height = 15*fONE;
	Face.TexRect.right = 1;
	Face.TexRect.bottom = 1;
	
	Width = 14*fONE;
	ScaleWidth = 0;
	ScaleFlag = TRUE;
	memset(WeaponPos, 0, sizeof(WeaponPos));
	WeaponPosCount = 3;

 	GlowWidth = 60*fONE; 
	GlowFlag = FALSE;
	
 	GlowFace.MatNum = *matNum;
	GlowFace.r = Face.r;
	GlowFace.g = Face.g;
	GlowFace.b = Face.b;
	GlowFace.Transparency = 160;
	GlowFace.width = GlowWidth;
	GlowFace.height = GlowWidth;
	
	GlowFace.TexRect.left   = 0;
	GlowFace.TexRect.top    = 0;
	GlowFace.TexRect.right  = 1; 
	GlowFace.TexRect.bottom = 1;

	CODE = SHELLTOM_WEAPON;

}

void AssaShelltomUpWeapon2::Main()
{
	if(MyCharacterFlag)
	{
 		if(lpCurPlayer != pChar)
			pChar = lpCurPlayer;
	}
	
	if(Time >= (Max_Time-5))
		EndFlag = TRUE;
	if(EndFlag)
	{
		Time = Max_Time;
		return;
	}
	
	
  	if(ScaleFlag)
		ScaleWidth += 100;
	else
		ScaleWidth -= 100;

 	if(ScaleWidth >= fONE && ScaleFlag == TRUE)
		ScaleFlag = FALSE;
	if(ScaleWidth <= -fONE && ScaleFlag == FALSE)
		ScaleFlag = TRUE;
	
	smMATRIX *mWorld;
  	memset(WeaponPos, 0, sizeof(WeaponPos));
	int index = 0;
	int StartZPos = 0;
	int WeaponSize = 0;

	GlowFlag = FALSE;
 	if(pChar->HvRightHand.dwItemCode != 0)
	{
		mWorld = &pChar->HvRightHand_Matrix;
		WeaponSize = pChar->HvRightHand.SizeMax;

 		if((pChar->HvRightHand.dwItemCode & sinITEM_MASK2) == sinWT1) //제블린일 경우..
		{
             StartZPos = -(pChar->HvRightHand.SizeMax-fONE*4);
  			WeaponSize = int((float)pChar->HvRightHand.SizeMax*1.5f);
			Width = 12*fONE;
			WeaponPosCount = WeaponSize/1000;
			

			for(index = 0; index < MAX_WEAPON_POS; index++)
				WeaponPos[index].z = StartZPos + index*1000;
		}
		else if((pChar->HvRightHand.dwItemCode & sinITEM_MASK2) == sinWC1) //클러일 경우..
		{
    		StartZPos = 0;
 			WeaponSize = pChar->HvRightHand.SizeMax*3;
			Width = 15*fONE;
			WeaponPosCount = WeaponSize/1000;

 			for(index = 0; index < MAX_WEAPON_POS; index++)
				WeaponPos[index].x = pChar->HvRightHand.SizeMax + index*1000;
		}
		else if((pChar->HvRightHand.dwItemCode & sinITEM_MASK2) ==  sinWM1)//완드인경우..
		{
    		StartZPos = 0;
			WeaponPosCount = WeaponSize/1000;
			GlowWidth = 35*fONE;
			GlowFlag = TRUE;

			Width = 12*fONE;
			for(index = 0; index < MAX_WEAPON_POS; index++)
				WeaponPos[index].z = StartZPos + index*1000;

			int glowPosIndex = (WeaponSize-int((float)WeaponSize*0.2f))/1000;
			if(glowPosIndex >= MAX_WEAPON_POS)
				glowPosIndex = MAX_WEAPON_POS-1;
			GlowPosIndex = glowPosIndex;
		}
		else if((pChar->HvRightHand.dwItemCode & sinITEM_MASK2) ==  sinWS1)//쏘는 활
		{
    		WeaponPosCount = (pChar->HvRightHand.SizeMax+pChar->HvRightHand.SizeMax/2)/1000;
 			if( ((pChar->HvRightHand.dwItemCode & sinITEM_MASK3)) == sin17)
				WeaponPosCount = 7;
			
			int cnt;
			for(index = 0; index < MAX_WEAPON_POS; index++)
			{
				WeaponPos[index].z = index * 1000;
				
				cnt = (ANGLE_45)&ANGCLIP;
				GetMoveLocation(WeaponPos[index].x, WeaponPos[index].y, WeaponPos[index].z, 0, cnt, 0);
				
				WeaponPos[index].x = GeoResult_X;
				WeaponPos[index].y = GeoResult_Y;
				WeaponPos[index].z = GeoResult_Z;
			}
		}
		else if((pChar->HvRightHand.dwItemCode & sinITEM_MASK2) ==  sinWH1)//망치
		{
  			StartZPos = 0;
			Width = 12*fONE;
			WeaponPosCount = WeaponSize/1000;
			GlowWidth = 40*fONE;
			GlowFlag = TRUE;
			for(index = 0; index < MAX_WEAPON_POS; index++)
				WeaponPos[index].z = StartZPos + index*1000;

			int glowPosIndex = (WeaponSize-int((float)WeaponSize*0.2f))/1000;
			if(glowPosIndex >= MAX_WEAPON_POS)
				glowPosIndex = MAX_WEAPON_POS-1;
			
			GlowPosIndex = glowPosIndex;
		}
		else if((pChar->HvRightHand.dwItemCode & sinITEM_MASK2) ==  sinWA1)//도끼
		{
 			StartZPos = 0;
			Width = 12*fONE;
			WeaponPosCount = WeaponSize/1000;
			GlowWidth = 45*fONE;
			GlowFlag = TRUE;
			for(index = 0; index < MAX_WEAPON_POS; index++)
				WeaponPos[index].z = StartZPos + index*1000;

			int glowPosIndex = (WeaponSize-int((float)WeaponSize*0.2f))/1000;
			if(glowPosIndex >= MAX_WEAPON_POS)
				glowPosIndex = MAX_WEAPON_POS-1;
			
			GlowPosIndex = glowPosIndex;

 			if( ((pChar->HvRightHand.dwItemCode & sinITEM_MASK3)) == sin14)
			{
				int cnt;
				int sin;
				int cos;
	 			
				cnt = (-ANGLE_45/5)&ANGCLIP;
						
				sin = GetSin[cnt]>>8;
				cos = GetCos[cnt]>>8;
				
				for(int index = 0; index < MAX_WEAPON_POS; index++)
				{
					WeaponPos[index].y = -( WeaponPos[index].z * sin )>>8;
					WeaponPos[index].z =  ( WeaponPos[index].z * cos )>>8;
				}
			}
		}
		else if((pChar->HvRightHand.dwItemCode & sinITEM_MASK2) ==  sinWP1)//사이드
		{
 			StartZPos = 0;
			WeaponPosCount = (WeaponSize-4500)/1000;

			for(index = 0; index < MAX_WEAPON_POS; index++)
				WeaponPos[index].z = StartZPos + index*1000;
		}
		else
 		{
			StartZPos = 0;
			WeaponSize = pChar->HvRightHand.SizeMax;
			WeaponPosCount = WeaponSize/1000;

			for(index = 0; index < MAX_WEAPON_POS; index++)
				WeaponPos[index].z = StartZPos + index*1000;
		}
	}
	else if(pChar->HvLeftHand.dwItemCode != 0) //양손 무기인 경우(보우 같은 경우)
	{
         mWorld = &pChar->HvLeftHand_Matrix;
		WeaponPosCount = pChar->HvLeftHand.SizeMax/1000;
 		StartZPos = -pChar->HvLeftHand.SizeMax/3;
		
    	for(index = 0; index < MAX_WEAPON_POS; index++)
		{
			WeaponPos[index].z = StartZPos + 1000*index;
			WeaponPos[index].x = 600;
		}
	}
	else //아무 무기도 들지 않았을 경우. 파티클을 끈다.
	{
		return;
	} 

	//양손 무기 5도 기울어짐..
	int cnt;
	int sin;
	int cos;
 	if ( pChar->dwActionItemTwoHand) 
	{
		cnt = (-ANGLE_45/5)&ANGCLIP;
			
		sin = GetSin[cnt]>>8;
		cos = GetCos[cnt]>>8;
		
		for(int index = 0; index < MAX_WEAPON_POS; index++)
		{
			WeaponPos[index].y = -( WeaponPos[index].z * sin )>>8;
			WeaponPos[index].z =  ( WeaponPos[index].z * cos )>>8;
		}
	}
	
	POINT3D angle;
   	memcpy( &angle, &pChar->Angle , sizeof( POINT3D ) );
	angle.y = (-angle.y+ANGLE_180)&ANGCLIP;
	
 	if(pChar->HvRightHand.dwItemCode != 0)
		AnimObjectTree( pChar->HvRightHand.ObjBip, pChar->frame, angle.x,angle.y,angle.z);
	
	int rx, ry, rz;

	for(int index = 0; index < MAX_WEAPON_POS; index++)
	{
		rx = WeaponPos[index].x * mWorld->_11 +
			 WeaponPos[index].y * mWorld->_21 +
 			 WeaponPos[index].z * mWorld->_31;

		ry = WeaponPos[index].x * mWorld->_12 +
			 WeaponPos[index].y * mWorld->_22 +
			 WeaponPos[index].z * mWorld->_32;

		rz = WeaponPos[index].x * mWorld->_13 +
			 WeaponPos[index].y * mWorld->_23 +
			 WeaponPos[index].z * mWorld->_33;

		WeaponPos[index].x = (pChar->pX + (rx>>FLOATNS) + mWorld->_41);
		WeaponPos[index].z = (pChar->pZ + (ry>>FLOATNS) + mWorld->_42);
		WeaponPos[index].y = (pChar->pY + (rz>>FLOATNS) + mWorld->_43);
	}
	Posi.x = WeaponPos[0].x;
	Posi.y = WeaponPos[0].y;
	Posi.z = WeaponPos[0].z;
}

void AssaShelltomUpWeapon2::Draw(POINT3D *pPosi, POINT3D *pAngle)
{
   	if(pChar->FlagShow == FALSE)
		return;
	smMaterialGroup->smMaterial[Face.MatNum].TwoSide = TRUE; //카메라 회전과 상관없이 보이기 
	
	//lpD3DDevice->SetRenderState( D3DRENDERSTATE_ZWRITEENABLE, FALSE );
   	for(int index = 0; index < WeaponPosCount; index++)
	{
		smRender.Init(); //데이터 초기화 
		smRender.SetMaterialGroup(smMaterialGroup);				 //읽어드린 메트리얼을 연결 

		Face.x = WeaponPos[index].x;
		Face.y = WeaponPos[index].y;
		Face.z = WeaponPos[index].z;
		Face.width  = Width+ScaleWidth-index*5;
		Face.height = Width+ScaleWidth-index*5;
		if(!AssaAddFace2D(&Face))
			continue;

		smRender.SetClipStates(SMCLIP_LEFT | SMCLIP_RIGHT | SMCLIP_TOP | SMCLIP_BOTTOM);
		smRender.ClipRendFace();
		smRender.GeomVertex2D(FALSE);
		smRender.RenderD3D();
	}
	
	if(GlowFlag)
	{
		smRender.Init(); //데이터 초기화 
		smRender.SetMaterialGroup(smMaterialGroup);				 //읽어드린 메트리얼을 연결 

		GlowFace.x = WeaponPos[GlowPosIndex].x;
		GlowFace.y = WeaponPos[GlowPosIndex].y;
		GlowFace.z = WeaponPos[GlowPosIndex].z;
 		GlowFace.width  = GlowWidth+ScaleWidth;
		GlowFace.height = GlowWidth+ScaleWidth;
		
		if(!AssaAddFace2D(&GlowFace))
			return;

		smRender.SetClipStates(SMCLIP_LEFT | SMCLIP_RIGHT | SMCLIP_TOP | SMCLIP_BOTTOM);
		smRender.ClipRendFace();
		smRender.GeomVertex2D(FALSE);
		smRender.RenderD3D();
	}
	
	//smRender.SetClipStates(SMCLIP_LEFT | SMCLIP_RIGHT | SMCLIP_TOP | SMCLIP_BOTTOM);
	//smRender.ClipRendFace();
	//smRender.GeomVertex2D(FALSE);
	//smRender.RenderD3D();
	//lpD3DDevice->SetRenderState( D3DRENDERSTATE_ZWRITEENABLE, TRUE );
 }



ParkAssaChaosKaraMeteo::ParkAssaChaosKaraMeteo()
{
	ParticleID = -1;
	TimeCount = 0;
	ParticleSpawnFlag = false;
}

ParkAssaChaosKaraMeteo::~ParkAssaChaosKaraMeteo()
{

}

void ParkAssaParticle_ChaosKaraTerrainFireHit(POINT3D *pCur)
{
    D3DVECTOR vector;
    vector.x = float(pCur->x - lpCurPlayer->pX)/fONE;
	vector.y = float(pCur->y - lpCurPlayer->pY)/fONE;
	vector.z = float(pCur->z - lpCurPlayer->pZ)/fONE;
	
	float length;
 	length = (float)sqrt(vector.x*vector.x+vector.y*vector.y+vector.z*vector.z);
	if(length == 0)
		length = 1;

    SetDynLight( pCur->x, pCur->y, pCur->z, 100, 200, 255, 255, 250, 2);
 	g_NewParticleMgr.Start("ChaosKaraMeteoHit", *pCur);
    EffectWaveCamera(int((500.f - length)/15.f), 2);
}

void ParkAssaChaosKaraMeteo::Main()
{
	if(ParticleID != -1)
	{
		Pos.x += Velocity.x*256.f;
		Pos.y += Velocity.y*256.f;
		Pos.z += Velocity.z*256.f;

		g_NewParticleMgr.SetPos(ParticleID, Pos);
		TimeCount--;

		if(TimeCount <= 0)
		{
 			g_NewParticleMgr.SetStop(ParticleID);
			ParticleID = -1;
			Time = Max_Time;
		}

		int mapY1, mapY2;
		if(smGameStage[0])
		{
			mapY1 = smGameStage[0]->GetHeight((int)Pos.x, (int)Pos.z);
		}
		if(smGameStage[1])
		{
			mapY2 = smGameStage[1]->GetHeight((int)Pos.x, (int)Pos.z);
		}

		if(smGameStage[0] && smGameStage[1])
			if (mapY1 < mapY2) mapY1 = mapY2;
		if(smGameStage[0] == NULL && smGameStage[1])
			mapY1 = mapY2;
		
		//맵과 총돌을 했는지 검사한다.
		if(mapY1 > (int)Pos.y)
		{
			g_NewParticleMgr.SetStop(ParticleID);
			ParticleID = -1;
			Time = Max_Time;
			POINT3D hitPos;
			hitPos.x = (int)Pos.x;
			hitPos.y = (int)Pos.y;
			hitPos.z = (int)Pos.z;

 			ParkAssaParticle_ChaosKaraTerrainFireHit(&hitPos);
			D3DVECTOR vector;

			vector.x = float(hitPos.x - lpCurPlayer->pX)/fONE;
			vector.y = float(hitPos.y - lpCurPlayer->pY)/fONE;
			vector.z = float(hitPos.z - lpCurPlayer->pZ)/fONE;
			
			float length;
 			length = (float)sqrt(vector.x*vector.x+vector.y*vector.y+vector.z*vector.z);
			if(length == 0)
				length = 1;
			
			
			//esPlaySound( 21, int((300000.f-length)*0.01f)  );
			if(GameMode != 1)
   		  	esPlaySound( 21, 400-int(length/10.f));
		}

	}


	if(ParticleSpawnFlag == false)
	{
		Delay--;
		if(Delay <= 0)
		{
			POINT3D partPos;
			partPos.x = (int)Pos.x;
			partPos.y = (int)Pos.y;
			partPos.z = (int)Pos.z;

  			ParticleID = g_NewParticleMgr.Start("ChaosKaraMeteo", partPos);
			ParticleSpawnFlag = true;
		}
	}
}

void ParkAssaChaosKaraMeteo::Start(POINT3D *pCur, POINT3D *pDest, int delay)
{
    Pos.x = (float)pCur->x;
	Pos.y = (float)pCur->y;
	Pos.z = (float)pCur->z;

	D3DVECTOR term;
	term.x = float((pDest->x - Pos.x)/ fONE);
	term.y = float((pDest->y - Pos.y)/ fONE);
	term.z = float((pDest->z - Pos.z)/ fONE);
	
 	float length = term.x*term.x + term.y*term.y + term.z*term.z;
	length = (float)sqrt(length);


	Velocity.x = term.x/length*8.f;
	Velocity.y = term.y/length*8.f;
	Velocity.z = term.z/length*8.f;

	POINT3D partPos;
	partPos.x = (int)Pos.x;
	partPos.y = (int)Pos.y;
	partPos.z = (int)Pos.z;

	Delay = delay;
	if(delay == 1)
	{
		ParticleSpawnFlag = true;
  		ParticleID = g_NewParticleMgr.Start("ChaosKaraMeteo", partPos);
	}

 	Max_Time = 2000;
	TimeCount = 70*10;
	

	
	D3DVECTOR vector;
    vector.x = float(partPos.x - lpCurPlayer->pX)/fONE;
	vector.y = 0.f;
	vector.z = float(partPos.z - lpCurPlayer->pZ)/fONE;
	
	
  	length = (float)sqrt(vector.x*vector.x+vector.y*vector.y+vector.z*vector.z);
	if(length == 0)
		length = 1;
	
	if(GameMode != 1 )
  		esPlaySound( 20, 400-int(length/100.f));
}


ParkAssaLizardfolk::ParkAssaLizardfolk()
{
	memset(&ShootingAngle, 0, sizeof(ShootingAngle));
	PatMesh = NULL;
	Delay = 0;
}

ParkAssaLizardfolk::~ParkAssaLizardfolk()
{

}

void ParkAssaLizardfolk::Start(POINT3D *pCur, POINT3D *pDest, int delay)
{
     int *matNum = (int *)AssaSearchRes("shield1.bmp", SMMAT_BLEND_LAMP);
 	if(matNum == NULL)
		return;

    Face.MatNum = *matNum;
 	Face.r = 255;
	Face.g = 150;
	Face.b = 50;
	Face.Transparency = 130;
 	Face.width = 2500;
	
	Posi = *pCur;
	DesPosi = *pDest;
	DesPosi.y += 4 * fONE;
	
	D3DVECTOR term;
 	term.x = float(DesPosi.x - Posi.x);
	term.y = float(DesPosi.y - Posi.y);
	term.z = float(DesPosi.z - Posi.z);
	
	GetRadian3D(Posi.x,Posi.y, Posi.z, DesPosi.x, DesPosi.y, DesPosi.z);
	ShootingAngle.x = GeoResult_X;
	ShootingAngle.y = GeoResult_Y;
	ShootingAngle.z = 0;
	
	
   	int step = -40*fONE;				
	
	
	//창이나 기타 던지는 무기
  	GetMoveLocation( 0, 0, step, ShootingAngle.x, ShootingAngle.y, ShootingAngle.z );


	
	ScalePosi.x = GeoResult_X;
	ScalePosi.y = GeoResult_Y;
	ScalePosi.z = GeoResult_Z;

	CurrentFrame = 0;
	Delay = delay;

	Max_Time = 500;
	PatMesh = (smPAT3D *)AssaSearchRes("At3_SplitJavelin40.ASE", SMMAT_BLEND_LAMP);
	POINT3D meshAngle;
    meshAngle.x = ShootingAngle.x;
  	meshAngle.y = (-ShootingAngle.y+ANGLE_180)&ANGCLIP;
  	meshAngle.z = ShootingAngle.z;
	
	PatMesh->Frame = CurrentFrame;
	PatMesh->SetPosi(&Posi, &meshAngle);
	
	Face.x = Posi.x;
	Face.y = Posi.y;
	Face.z = Posi.z;
	BlendStep = -255;
}


void ParkAssaLizardfolk::Main()
{
    	if(Max_Time == Time)
		return;
	if(Time > Delay)
 	{
		if(BlendStep >= 0)
			BlendStep = 0;
		else
			BlendStep += 30;
	}

	if(Time < Delay+40)
		return;
	
  	

 	Face.x = Posi.x;
	Face.y = Posi.y;
	Face.z = Posi.z;

      int step = 10*fONE;				//창이나 기타 던지는 무기
 	GetMoveLocation( 0, 0, step, ShootingAngle.x, ShootingAngle.y, ShootingAngle.z );
	Posi.x += GeoResult_X;
	Posi.y += GeoResult_Y;
	Posi.z += GeoResult_Z;
	
	POINT3D pos;
 	pos.x = int(Posi.x);
	pos.y = int(Posi.y);
	pos.z = int(Posi.z);
		
	D3DVECTOR term;
	term.x = float((DesPosi.x - pos.x)/ fONE);
	term.y = float((DesPosi.y - pos.y)/ fONE);
	term.z = float((DesPosi.z - pos.z)/ fONE);
		
 	float length = term.x*term.x + term.y*term.y + term.z*term.z;
	length = (float)sqrt(length);
	
	if(length < 20 || Time > (35+Delay+20))
	{
  	 	Time = Max_Time;
		int num = g_NewParticleMgr.Start("Skill3AtalantaSplitJavelin1", DesPosi);
		g_NewParticleMgr.SetAngle(num, ShootingAngle);
	}
}

void ParkAssaLizardfolk::Draw(POINT3D *pPosi , POINT3D *pAngle)
{
	if(PatMesh != NULL)
	{
 		
		smRender.Init(); //데이터 초기화 
		POINT3D meshAngle;
        meshAngle.x = ShootingAngle.x;
  		meshAngle.y = (-ShootingAngle.y+ANGLE_180)&ANGCLIP;
 		meshAngle.z = ShootingAngle.z;
		
		int r,g,b,a;
		r = smRender.Color_R;
		g = smRender.Color_G;
		b = smRender.Color_B;
		a = smRender.Color_A;

		smRender.Color_R = r+BlendStep;
		smRender.Color_G = g+BlendStep;
		smRender.Color_B = b+BlendStep;
 		smRender.Color_A = a+BlendStep;

		PatMesh->Frame = CurrentFrame;
		PatMesh->SetPosi(&Posi, &meshAngle);
		PatMesh->RenderD3D(pPosi, pAngle);
		
		smRender.Init(); //데이터 초기화 
 		smMaterialGroup->smMaterial[Face.MatNum].TwoSide = TRUE; //카메라 회전과 상관없이 보이기 
		smRender.SetMaterialGroup(smMaterialGroup);  //읽어드린 메트리얼을 연결 
		smRender.SetCameraPosi(pPosi->x, pPosi->y, pPosi->z,pAngle->x,pAngle->y,pAngle->z);
		
		if(!AssaAddFace2DLine(&Face, &ScalePosi))
				return;
		
 		//smRender.SetClipStates(SMCLIP_LEFT | SMCLIP_RIGHT | SMCLIP_TOP | SMCLIP_BOTTOM);
		smRender.ClipRendFace();
  		smRender.GeomVertex2D(FALSE);
		smRender.RenderD3D();

		smRender.Color_R = r;
		smRender.Color_G = g;
		smRender.Color_B = b;
		smRender.Color_A = a;

	}
}


AssaSkyMeteoParticle::AssaSkyMeteoParticle()
{
	ParticleID = -1;
	TimeCount = 0;
	ParticleSpawnFlag = false;
}

AssaSkyMeteoParticle::~AssaSkyMeteoParticle()
{

}

void AssaSkyMeteoParticle::Main()
{
	if(ParticleID != -1)
	{
		Pos.x += Velocity.x*256.f;
		Pos.y += Velocity.y*256.f;
		Pos.z += Velocity.z*256.f;

		g_NewParticleMgr.SetPos(ParticleID, Pos);
		TimeCount--;

		if(TimeCount <= 0)
		{
 			g_NewParticleMgr.SetStop(ParticleID);
			ParticleID = -1;
			Time = Max_Time;
		}

		int mapY1, mapY2;
		if(smGameStage[0])
		{
			mapY1 = smGameStage[0]->GetHeight((int)Pos.x, (int)Pos.z);
		}
		if(smGameStage[1])
		{
			mapY2 = smGameStage[1]->GetHeight((int)Pos.x, (int)Pos.z);
		}

		if(smGameStage[0] && smGameStage[1])
			if (mapY1 < mapY2) mapY1 = mapY2;
		if(smGameStage[0] == NULL && smGameStage[1])
			mapY1 = mapY2;
		
		//맵과 총돌을 했는지 검사한다.
		if(mapY1 > (int)Pos.y)
		{
			g_NewParticleMgr.SetStop(ParticleID);
			ParticleID = -1;
			Time = Max_Time;
			POINT3D hitPos;
			hitPos.x = (int)Pos.x;
			hitPos.y = (int)Pos.y;
			hitPos.z = (int)Pos.z;

			ParkAssaParticle_TerrainFireHit(&hitPos, nEffectType);

			D3DVECTOR vector;

			vector.x = float(hitPos.x - lpCurPlayer->pX)/fONE;
			vector.y = float(hitPos.y - lpCurPlayer->pY)/fONE;
			vector.z = float(hitPos.z - lpCurPlayer->pZ)/fONE;
			
			float length;
 			length = (float)sqrt(vector.x*vector.x+vector.y*vector.y+vector.z*vector.z);
			if(length == 0)
				length = 1;
			
			
			//esPlaySound( 21, int((300000.f-length)*0.01f)  );
			if(GameMode != 1)
   		  	esPlaySound( 21, 400-int(length/10.f));
		}

	}


	if(ParticleSpawnFlag == false)
	{
		Delay--;
		if(Delay <= 0)
		{
			POINT3D partPos;
			partPos.x = (int)Pos.x;
			partPos.y = (int)Pos.y;
			partPos.z = (int)Pos.z;

  			ParticleID = g_NewParticleMgr.Start("TerrainFire", partPos);
			ParticleSpawnFlag = true;
		}
	}
}

void AssaSkyMeteoParticle::Start(POINT3D *pCur, POINT3D *pDest, int delay, int nType)
{
    Pos.x = (float)pCur->x;
	Pos.y = (float)pCur->y;
	Pos.z = (float)pCur->z;

	D3DVECTOR term;
	term.x = float((pDest->x - Pos.x)/ fONE);
	term.y = float((pDest->y - Pos.y)/ fONE);
	term.z = float((pDest->z - Pos.z)/ fONE);
	
 	float length = term.x*term.x + term.y*term.y + term.z*term.z;
	length = (float)sqrt(length);


	Velocity.x = term.x/length*8.f;
	Velocity.y = term.y/length*8.f;
	Velocity.z = term.z/length*8.f;

	POINT3D partPos;
	partPos.x = (int)Pos.x;
	partPos.y = (int)Pos.y;
	partPos.z = (int)Pos.z;

	Delay = delay;
	nEffectType = nType;
	if(nType == 0)
	{
		ParticleSpawnFlag = true;
  		ParticleID = g_NewParticleMgr.Start("TerrainFire", partPos);
	}
	else if(nType == 1)
	{
		ParticleSpawnFlag = true;
  		ParticleID = g_NewParticleMgr.Start("TerrainFireSmall", partPos);
	}		

 	Max_Time = 2000;
	TimeCount = 70*10;
	
	
	D3DVECTOR vector;
    vector.x = float(partPos.x - lpCurPlayer->pX)/fONE;
	vector.y = 0.f;
	vector.z = float(partPos.z - lpCurPlayer->pZ)/fONE;
	
	
  	length = (float)sqrt(vector.x*vector.x+vector.y*vector.y+vector.z*vector.z);
	if(length == 0)
		length = 1;
	
	if(GameMode != 1 )
  		esPlaySound( 20, 400-int(length/100.f));
}


/*----------------------------------------------------------------------------*
*	Aging +21..+25: a bluish light orb circling the weapon, with a trail.
*	The weapon itself stays +20's white. Level 1..5 (= +21..+25) sets the orb's size
*	and the trail's length. Other players' clients know the level from the weapon's
*	blink colours (playsub.cpp AgingBlinkColor rows 17..21, GetAgingOrbLevel).
*-----------------------------------------------------------------------------*/
int GetAgingOrbLevel( smCHARTOOL *lpCharTool );		// playsub.cpp

AssaAgingOrb::AssaAgingOrb()
{
	MyCharacterFlag = FALSE;
	EndFlag = FALSE;
	MatNum = 0;
	Level = 0;
	OrbitAngle = 0;
	SlidePhase = 0;
	TrailCount = 0;
	TrailHead = 0;
	memset( Trail, 0, sizeof(Trail) );
}

AssaAgingOrb::~AssaAgingOrb()
{
}

void AssaAgingOrb::Start(smCHAR *pCharacter)
{
	pChar = pCharacter;
	EndFlag = TRUE;
	if ( pChar==NULL ) return;
	if ( lpCurPlayer==pChar ) MyCharacterFlag = TRUE;

	int *matNum = (int *)AssaSearchRes("flare.tga", SMMAT_BLEND_LAMP);
	if ( matNum==NULL ) return;
	MatNum = *matNum;

	memset( &Face, 0, sizeof(Face) );
	Face.MatNum = MatNum;
	Face.TexRect.right = 1;
	Face.TexRect.bottom = 1;

	EndFlag = FALSE;
	Max_Time = 5000;
	CODE = AGING_ORB_CODE;
}

void AssaAgingOrb::Main()
{
	if ( MyCharacterFlag && lpCurPlayer!=pChar ) pChar = lpCurPlayer;

	if ( !EndFlag && pChar && pChar->Flag ) {
		int lv = GetAgingOrbLevel( &pChar->HvRightHand );
		int left = 0;
		if ( !lv || !pChar->HvRightHand.dwItemCode ) {
			lv = GetAgingOrbLevel( &pChar->HvLeftHand );
			left = 1;
		}
		if ( lv<=0 || (left ? !pChar->HvLeftHand.dwItemCode : !pChar->HvRightHand.dwItemCode) ) EndFlag = TRUE;
		Level = lv;
		if ( !EndFlag ) {
			smCHARTOOL *tool = left ? &pChar->HvLeftHand : &pChar->HvRightHand;
			smMATRIX *mWorld = left ? &pChar->HvLeftHand_Matrix : &pChar->HvRightHand_Matrix;
			DWORD kind = tool->dwItemCode & sinITEM_MASK2;
			int size = tool->SizeMax;
			POINT3D wp = { 0,0,0 };

			// where on the weapon the orb circles (as AssaClassUpWeapon), sliding up and down the blade
			SlidePhase = (SlidePhase+24)&ANGCLIP;
			int slide = ((size/4) * (GetSin[SlidePhase]>>8))>>8;
			if ( kind==sinWT1 )			wp.z = -size+(size*6)/10 + slide;
			else if ( kind==sinWC1 )	wp.x = size*2;
			else						wp.z = (size*55)/100 + slide;

			if ( kind==sinWS1 ) {		// bows are held at 45 degrees
				GetMoveLocation( wp.x, wp.y, wp.z, 0, ANGLE_45&ANGCLIP, 0 );
				wp.x = GeoResult_X; wp.y = GeoResult_Y; wp.z = GeoResult_Z;
			}
			if ( pChar->dwActionItemTwoHand ) {		// two-handed weapons lean 5 degrees
				int a = (-ANGLE_45/5)&ANGCLIP;
				int s = GetSin[a]>>8, c = GetCos[a]>>8;
				int z = wp.z;
				wp.y = -( z*s )>>8;
				wp.z =  ( z*c )>>8;
			}

			// the orbit: faster and wider with the age
			OrbitAngle = (OrbitAngle+60+Level*10)&ANGCLIP;
			int radius = 900 + Level*150;
			if ( kind==sinWC1 || kind==sinWS1 || left )
				GetMoveLocation( 0, radius, 0, OrbitAngle, 0, 0 );
			else
				GetMoveLocation( 0, radius, 0, 0, 0, OrbitAngle );
			wp.x += GeoResult_X; wp.y += GeoResult_Y; wp.z += GeoResult_Z;

			// local -> world (the hand matrix is relative to the character)
			int rx = wp.x*mWorld->_11 + wp.y*mWorld->_21 + wp.z*mWorld->_31;
			int ry = wp.x*mWorld->_12 + wp.y*mWorld->_22 + wp.z*mWorld->_32;
			int rz = wp.x*mWorld->_13 + wp.y*mWorld->_23 + wp.z*mWorld->_33;
			POINT3D world;
			world.x = pChar->pX + (rx>>FLOATNS) + mWorld->_41;
			world.z = pChar->pZ + (ry>>FLOATNS) + mWorld->_42;
			world.y = pChar->pY + (rz>>FLOATNS) + mWorld->_43;

			TrailHead = (TrailHead+1)%AGING_ORB_TRAIL_MAX;
			Trail[TrailHead] = world;
			if ( TrailCount<AGING_ORB_TRAIL_MAX ) TrailCount++;
		}
	}
	else
		EndFlag = TRUE;

	if ( EndFlag ) {
		Time = Max_Time;
		CODE = 0;
		return;
	}
	if ( Time>Max_Time-140 ) Time = Max_Time-150;		// lives as long as the weapon is held
}

void AssaAgingOrb::Draw(POINT3D *pPosi, POINT3D *pAngle)
{
	if ( EndFlag || !pChar || pChar->FlagShow==FALSE || Level<=0 || TrailCount<=0 ) return;

	smMaterialGroup->smMaterial[MatNum].TwoSide = TRUE;

	int trailLen = 6 + Level*6;					// +21: 12 .. +25: 36
	if ( trailLen>TrailCount ) trailLen = TrailCount;
	int orbSize = (10 + Level*5)*fONE;			// +21: 15 .. +25: 35

	// the trail, oldest first, so the orb draws on top
	for ( int i=trailLen-1; i>=0; i-- ) {
		POINT3D *p = &Trail[ (TrailHead-i+AGING_ORB_TRAIL_MAX*2)%AGING_ORB_TRAIL_MAX ];
		int k = trailLen-i;						// trailLen at the orb, 1 at the tail
		smRender.Init();
		smRender.SetMaterialGroup(smMaterialGroup);
		Face.x = p->x; Face.y = p->y; Face.z = p->z;
		Face.width = Face.height = (i==0) ? orbSize : (orbSize*k)/(trailLen+trailLen/2) + fONE*2;
		Face.r = 70; Face.g = 150; Face.b = 255;
		Face.Transparency = (i==0) ? 230 : (170*k)/trailLen;
		if ( !AssaAddFace2D(&Face) ) continue;
		smRender.SetClipStates(SMCLIP_LEFT | SMCLIP_RIGHT | SMCLIP_TOP | SMCLIP_BOTTOM);
		smRender.ClipRendFace();
		smRender.GeomVertex2D(FALSE);
		smRender.RenderD3D();
	}

	// a white core in the orb
	POINT3D *head = &Trail[TrailHead];
	smRender.Init();
	smRender.SetMaterialGroup(smMaterialGroup);
	Face.x = head->x; Face.y = head->y; Face.z = head->z;
	Face.width = Face.height = orbSize/2;
	Face.r = 255; Face.g = 255; Face.b = 255;
	Face.Transparency = 220;
	if ( AssaAddFace2D(&Face) ) {
		smRender.SetClipStates(SMCLIP_LEFT | SMCLIP_RIGHT | SMCLIP_TOP | SMCLIP_BOTTOM);
		smRender.ClipRendFace();
		smRender.GeomVertex2D(FALSE);
		smRender.RenderD3D();
	}
}