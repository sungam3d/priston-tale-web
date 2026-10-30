#include "..\\HoBaram\\HoLinkHeader.h"
#include "..\\field.h"
#include "..\\HoBaram\\HoNewParticleLinkHeader.h"


HoSky	SkyBackGround;



void HoSkyDrawInfo::LoadRes()
{
	for(unsigned index = 0; index < SkyResInfoList.size(); index++)
	{
		if(SkyResInfoList[index].stageSky == NULL)
		{
			char buffer[256];
 			strcpy(buffer, SkyResInfoList[index].resName.c_str());
			smPAT3D *stageSky = smASE_Read(buffer);
			if(stageSky)
			{
 				stageSky->ZeroNormals();
 				for(int i = 0; i < stageSky->nObj3d; i++)
				{
 					if(ClippingFlag)
 						stageSky->obj3d[i]->ClipStates = ( SMCLIP_NEARZ | SMCLIP_LEFT | SMCLIP_RIGHT | SMCLIP_TOP | SMCLIP_BOTTOM );
					else
						stageSky->obj3d[i]->ClipStates = SMCLIP_NEARZ | SMCLIP_TOP;
						

					stageSky->obj3d[i]->ColorEffect = FALSE;
				}
  				stageSky->smMaterialGroup->ReadTextures();
				SkyResInfoList[index].stageSky = stageSky;
   				//stageSky->Bound = 200000;
 			}
		}
	}
}

void HoSkyDrawInfo::DestroyRes()
{
	for(unsigned index = 0; index < SkyResInfoList.size(); index++)
	{
		if(SkyResInfoList[index].stageSky != NULL)
		{
			delete SkyResInfoList[index].stageSky;
			SkyResInfoList[index].stageSky = NULL;
		}
	}
}

HoSky::HoSky()
{
	FirstSkyLoadFlag = FALSE;
	//TestModeFlag = TRUE;
	Init();
}

HoSky::~HoSky()
{
	Destroy();
}


#ifdef PT_WEB
// Diagnostics for the web port (black sky in the world): every 15 s for the first
// two minutes after entering the world, one line in the page's log.
static int s_SkyRendered;
void HoSky_WebDiag( HoSky &S, int x, int y, int z, int ax, int ay, int az )
{
	static DWORD s_Start = 0, s_Next = 0;
	DWORD now = GetCurrentTime();
	if ( !lpCurPlayer || lpCurPlayer->OnStageField<0 ) return;
	if ( !s_Start ) { s_Start = now; s_Next = now + 5000; }
	if ( now<s_Next || now-s_Start>125000 ) return;
	s_Next = now + 15000;
	sFIELD *f = StageField[lpCurPlayer->OnStageField];
	int nObj = -1, nRes = -1;
	if ( S.DrawIndex>=0 && S.DrawIndex<(int)S.SkyDrawInfoList.size() ) {
		nRes = (int)S.SkyDrawInfoList[S.DrawIndex].SkyResInfoList.size();
		nObj = 0;
		for ( int i=0; i<nRes; i++ )
			if ( S.SkyDrawInfoList[S.DrawIndex].SkyResInfoList[i].stageSky ) nObj += S.SkyDrawInfoList[S.DrawIndex].SkyResInfoList[i].stageSky->nObj3d;
	}
	printf( "[sky] field %d state %X back %d/%d/%d | code %d index %d (alpha %d next %d) old %d (alpha %d) | skies %d, layers %d, objects %d, drawn %d | test %d | cam %d,%d,%d ang %d,%d,%d | dark %d view %d\n",
		f ? f->FieldCode : -1, f ? f->State : -1, f ? f->BackImageCode[0] : -1, f ? f->BackImageCode[1] : -1, f ? f->BackImageCode[2] : -1,
		S.CodeID, S.DrawIndex, S.DrawAlphaCount, S.DrawNextAlphaCount, S.DrawOldIndex, S.DrawOldAlphaCount,
		(int)S.SkyDrawInfoList.size(), nRes, nObj, s_SkyRendered, S.TestModeFlag,
		x>>FLOATNS, y>>FLOATNS, z>>FLOATNS, ax, ay, az, DarkLevel, ViewPointLen );
}
#endif

int HoSky::Draw(int x, int y, int z, int ax, int ay, int az)
{
#ifdef PT_WEB
	HoSky_WebDiag( *this, x, y, z, ax, ay, az );
	s_SkyRendered = 0;
#endif

	if(DrawIndex != -1)
	 	Draw(x, y, z, ax, ay, az, DrawIndex, DrawAlphaCount, DrawNextAlphaCount);
	
	if(DrawOldIndex != -1)
		Draw(x, y, z, ax, ay, az, DrawOldIndex, DrawOldAlphaCount, DrawNextOldAlphaCount);
	

 	if(smConfig.DebugMode)
	{
		static BOOL ColorInfoFlag = FALSE;

 		if(VRKeyBuff[VK_NUMPAD7])
		{
		 	if(ColorInfoFlag == FALSE)
				ColorInfoFlag = TRUE;
			VRKeyBuff[VK_NUMPAD0] = FALSE;
		}
		if(ColorInfoFlag)
		{
			HDC opHdc;
			lpDDSBack->GetDC( &opHdc );
			SetBkMode( opHdc, TRANSPARENT );
		 	
			char buffer[256];
			memset(buffer, 0, sizeof(buffer));

			wsprintf(buffer, "ColorR:%d ColorG:%d ColorB:%d Dark:%d", BackColor_R, BackColor_G, BackColor_B, DarkLevel);
			HFONT oldFont;
			if(hFont != NULL)
				oldFont = (HFONT)SelectObject( opHdc , hFont );
		  	
 			SetTextColor( opHdc, RGB(0, 0, 0) );
 			dsTextLineOut(opHdc, 11, 101, buffer, strlen(buffer)); 

			SetTextColor( opHdc, RGB(255, 255, 255) );
			dsTextLineOut(opHdc, 10, 100, buffer, strlen(buffer)); 
			
			if(smRender.m_FogMode)
				wsprintf(buffer, "\306\367\261\327\273\347\277\353: FogColorR:%d FogColorG:%d FogColorB:%d", FogEndColorR, FogEndColorG, FogEndColorB, FogEndColorA);
			else
				wsprintf(buffer, "\306\367\261\327\273\347\277\353\276\310\307\324");

			SetTextColor( opHdc, RGB(255, 255, 255) );

			SetTextColor( opHdc, RGB(0, 0, 0) );
 			dsTextLineOut(opHdc, 11, 116, buffer, strlen(buffer)); 
			
			SetTextColor( opHdc, RGB(255, 255, 255) );
			dsTextLineOut(opHdc, 10, 115, buffer, strlen(buffer)); 

			wsprintf(buffer, "\306\367\261\327\275\303\300\333\300\247\304\241:%d \306\367\261\327\263\241\300\247\304\241:%d", FogStartPos2, FogEndPos2);


			if(oldFont)
				SelectObject(opHdc, oldFont);


			lpDDSBack->ReleaseDC( opHdc );
		}
	}
	
	return TRUE;
}


int HoSky::Draw(int x, int y, int z, int ax, int ay, int az, unsigned int drawIndex, int alphaCount, int alphaNextCount)
{
	
	int drawR = 0;
	int drawG = 0;
	int drawB = 0;
	int drawA = 0;
	
	int oldR = 0;
	int oldG = 0;
	int oldB = 0;
	int oldA = 0;

	POINT3D camera;
	POINT3D	angle;

	memset(&camera, 0, sizeof(camera));
	memset(&angle, 0, sizeof(angle));

	if(smRender.Color_R > 255)
	{
		int test = 0;
	}
	if(SkyDrawInfoList.size() && drawIndex < (int)SkyDrawInfoList.size())
	{
		if(SkyDrawInfoList[drawIndex].SkyResInfoList.size())
		{

	 		//int test = SkyDrawInfoList[drawIndex].SkyResInfoList.size();
 			for(unsigned int index = 0; index < SkyDrawInfoList[drawIndex].SkyResInfoList.size(); index++)
			{
				

				int alphaTestDepth = smRender.AlphaTestDepth;
  				smRender.AlphaTestDepth = 0;

 				oldR = smRender.Color_R;
				oldG = smRender.Color_G;
				oldB = smRender.Color_B;
 				oldA = smRender.Color_A;
				
				
				if(BlinkFlag && BlinkDrawFlag)
				{
					/*
 					drawR = SkyDrawInfoList[drawIndex].SkyResInfoList[index].colorR + BlinkColor;
   			 	    drawG = SkyDrawInfoList[drawIndex].SkyResInfoList[index].colorG + BlinkColor;
					drawB = SkyDrawInfoList[drawIndex].SkyResInfoList[index].colorB + BlinkColor;
					drawA = SkyDrawInfoList[drawIndex].SkyResInfoList[index].colorA + BlinkColor;
					*/
					
 					if(index == 0)
					{
     		 			drawR = SkyDrawInfoList[drawIndex].SkyResInfoList[index].colorR + BlinkColor;
  						drawG = SkyDrawInfoList[drawIndex].SkyResInfoList[index].colorG + BlinkColor;
						drawB = SkyDrawInfoList[drawIndex].SkyResInfoList[index].colorB + BlinkColor;
						drawA = SkyDrawInfoList[drawIndex].SkyResInfoList[index].colorA + BlinkColor;
					}
					else
					{
				 		drawR = SkyDrawInfoList[drawIndex].SkyResInfoList[index].colorR;
 						drawG = SkyDrawInfoList[drawIndex].SkyResInfoList[index].colorG;
						drawB = SkyDrawInfoList[drawIndex].SkyResInfoList[index].colorB;
						drawA = SkyDrawInfoList[drawIndex].SkyResInfoList[index].colorA;
					}
				}
				else
				{
				
					drawR = SkyDrawInfoList[drawIndex].SkyResInfoList[index].colorR;
 					drawG = SkyDrawInfoList[drawIndex].SkyResInfoList[index].colorG;
					drawB = SkyDrawInfoList[drawIndex].SkyResInfoList[index].colorB;
					drawA = SkyDrawInfoList[drawIndex].SkyResInfoList[index].colorA;
				}
				
				if(SkyDrawInfoList[drawIndex].SkyResInfoList[index].fadeSlowFlag ==	0)
				{
					if(alphaCount == 100)
					{
  						smRender.Color_R = drawR-255;
						smRender.Color_G = drawG-255;
 						smRender.Color_B = drawB-255;
 						smRender.Color_A = drawA-255;
					}
					else
					{
 						smRender.Color_R = drawR-255;
						smRender.Color_G = drawG-255;
 						smRender.Color_B = drawB-255;
  						smRender.Color_A = int((float)drawA*((float)alphaCount/100.f))-255;
					}
				}
				else 
				{
					//산이 찍힐 최종적이 컬러 값이다...
					if(alphaNextCount == 100)
					{
  						smRender.Color_R = drawR-255;
						smRender.Color_G = drawG-255;
 						smRender.Color_B = drawB-255;
 						smRender.Color_A = drawA-255;
					}
					else
					{
						smRender.Color_R = drawR-255;
						smRender.Color_G = drawG-255;
 						smRender.Color_B = drawB-255;
  						smRender.Color_A = int((float)drawA*((float)alphaNextCount/100.f))-255;
					}
				}
				/*
		 		if ( y>230000 ) 
					y=230000;
				*/
			 	if ( y>150000 ) 
					y=150000;

 				camera.x = 0;
				camera.y = 0;
				camera.z = 0;

				angle.x = 0;
				angle.y = 0;
   				angle.z = 0;
				
				if(SkyDrawInfoList[drawIndex].SkyResInfoList[index].stageSky != NULL)
				{
					if( g_IsDxProjectZoomIn )
					{
						if(!SkyDrawInfoList[drawIndex].ClippingFlag)
						{
							for(int i = 0; i < SkyDrawInfoList[drawIndex].SkyResInfoList[index].stageSky->nObj3d; i++)
							{
								SkyDrawInfoList[drawIndex].SkyResInfoList[index].stageSky->obj3d[i]->ClipStates = 
								( SMCLIP_NEARZ | SMCLIP_LEFT | SMCLIP_RIGHT | SMCLIP_TOP | SMCLIP_BOTTOM );
							}
						}
					}
					else
					{
						if(!SkyDrawInfoList[drawIndex].ClippingFlag)
						{
							for(int i = 0; i < SkyDrawInfoList[drawIndex].SkyResInfoList[index].stageSky->nObj3d; i++)
							{
 								SkyDrawInfoList[drawIndex].SkyResInfoList[index].stageSky->obj3d[i]->ClipStates = 
									SMCLIP_NEARZ | SMCLIP_TOP;
							}
						}
						
					}

   		 			lpD3DDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, FALSE);
  			 		SkyDrawInfoList[drawIndex].SkyResInfoList[index].stageSky->TmAnimation( 0, 0,0,0 );
					SkyDrawInfoList[drawIndex].SkyResInfoList[index].stageSky->SetPosi(&camera, &angle);

 					camera.x = 0;
					camera.y = y/8;
					camera.z = 0;

   					angle.x = ax;
					angle.y = ay;
					angle.z = az;
					
					//smRender.Init();
  					smRender.SetCameraPosi(x, y, z, ax, ay, az);
#ifdef PT_WEB
					s_SkyRendered +=
#endif
					SkyDrawInfoList[drawIndex].SkyResInfoList[index].stageSky->RenderD3D( &camera, &angle );
 					
					lpD3DDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, TRUE);
				}
				
				smRender.Color_R = oldR;
				smRender.Color_G = oldG;
				smRender.Color_B = oldB;
				smRender.Color_A = oldA;
				
				smRender.AlphaTestDepth = alphaTestDepth;
			}
		}
	}
	
	return TRUE;
}


void HoSky::ParticleEffectSpawn()
{
	float charPosX;
	float charPosZ;

	float cameraPosX;
	float cameraPosZ;

  	charPosX = (float)lpCurPlayer->pX/256.f;
	charPosZ = (float)lpCurPlayer->pZ/256.f;
	
	cameraPosX = (float)smRender.CameraPosiX/256.f;
	cameraPosZ = (float)smRender.CameraPosiZ/256.f;
	
	float termX = (charPosX - cameraPosX);
	float termZ = (charPosZ - cameraPosZ);

 	float length = (float)sqrt(termX*termX + termZ*termZ);
	
	termX = termX / length;
	termZ = termZ / length;

	POINT3D curPos;
	POINT3D desPos;

 	GetMoveLocation(200*fONE, 0, 0, 0, smRender.CameraAngY, 0);
	curPos.x = lpCurPlayer->pX + int(termX * 256.f * float(rand()%700 + 100))+GeoResult_X;
	curPos.y = lpCurPlayer->pY + 130000;
	curPos.z = lpCurPlayer->pZ + int(termZ * 256.f * float(rand()%700 + 100))+GeoResult_Z;

 	GetMoveLocation(-200*fONE, 0, 0, 0, smRender.CameraAngY, 0);
	desPos.x = curPos.x+GeoResult_X;
	desPos.y = lpCurPlayer->pY;
	desPos.z = curPos.z+GeoResult_Z;
	
 	AssaParticle_TerrainFire(&curPos, &desPos);
	
	
	/*
	POINT3D curPos;
	POINT3D desPos;

  	GetMoveLocation(0, 0, -200*fONE, 0, smRender.CameraAngY, 0);
	curPos.x = smRender.CameraPosiX + GeoResult_X;
	curPos.y = smRender.CameraPosiY + 50000;
	curPos.z = smRender.CameraPosiZ + GeoResult_Z;

 	GetMoveLocation(-200*fONE, 0, 0, 0, smRender.CameraAngY, 0);
	desPos.x = curPos.x+GeoResult_X;
	desPos.y = -200000;
	desPos.z = curPos.z+GeoResult_Z;
	
 	AssaParticle_TerrainFire(&curPos, &desPos);
	*/

}

int HoSky::Main()
{
  	if( ((DrawIndex == 21) && (CodeID == FIELD_BACKIMAGE_IRONBOSS)) ||
		(CodeID == 0x80) )
	{
        BlinkTime--;
		if(BlinkTime <= 0)
			BlinkTime = 400;
        
  		if((BlinkTime%300) == 0 && !BlinkFlag)
		{
  			BlinkFlag = TRUE;
			BlinkCount = 0;
			BlinkColor = 90;
		}
		
   		if(BlinkFlag)
		{
			if((BlinkTime%5) == 0)
			{
				BlinkDrawFlag = !BlinkDrawFlag;
				BlinkCount++;
				BlinkColor -= 5;
				if(BlinkCount >= 7)
				{
					BlinkFlag = FALSE;
					BlinkDrawFlag = FALSE;
					BlinkCount = 0;

				}
			}
		}
		if(CodeID != 0x23)
		{
			if( (ParticleCount <= 70 * 8) && (BlinkTime%60) == 0)
				ParticleEffectSpawn();
			if( (ParticleCount > 70 * 8) && (BlinkTime%350) == 0)
				ParticleEffectSpawn();
			ParticleCount++;
		}
	}
	else
	{
		BlinkFlag = FALSE;
		BlinkDrawFlag = FALSE;
		BlinkCount = 0;
		ParticleCount = 0;
	}
	
	

 	if((CodeID == FIELD_BACKIMAGE_DAYSNOW ||
	    CodeID == FIELD_BACKIMAGE_GLOWSNOW ||
  	    CodeID == FIELD_BACKIMAGE_NIGHTSNOW) && StartSnowFlag == FALSE)
	{
		if(lpCurPlayer != NULL)
		{
			POINT3D charPos;
			charPos.x = lpCurPlayer->pX;
			charPos.y = lpCurPlayer->pY+100000;
			charPos.z = lpCurPlayer->pZ;

			SnowParticleNum = g_NewParticleMgr.Start("Snow", charPos);
			StartSnowFlag = TRUE;
		}
	}
	
	if((CodeID != FIELD_BACKIMAGE_DAYSNOW) &&
	   (CodeID != FIELD_BACKIMAGE_GLOWSNOW) &&
 	   (CodeID != FIELD_BACKIMAGE_NIGHTSNOW) )
	{
		StartSnowFlag = FALSE;
		if(SnowParticleNum != -1)
			g_NewParticleMgr.SetStop(SnowParticleNum);
	}

	if((CodeID == FIELD_BACKIMAGE_DAYSNOW ||
	    CodeID == FIELD_BACKIMAGE_GLOWSNOW ||
	    CodeID == FIELD_BACKIMAGE_NIGHTSNOW) && StartSnowFlag == TRUE)
	{
		if(lpCurPlayer != NULL)
		{
			GetMoveLocation( 0, 0, 40*fONE, 0, lpCurPlayer->Angle.y, 0);
		
			D3DVECTOR charPos;
  			charPos.x = (float)lpCurPlayer->pX+GeoResult_X;
			charPos.y = (float)lpCurPlayer->pY+30000;
			charPos.z = (float)lpCurPlayer->pZ+GeoResult_Z;
			g_NewParticleMgr.SetPos(SnowParticleNum, charPos);
		}
	}


	if(DrawAlphaCount < 100 && (TimeCount%6) == 0 )
		DrawAlphaCount++;

	if(DrawOldAlphaCount > 0 && (TimeCount%4) == 0)
		DrawOldAlphaCount--;
	
	if(DrawNextAlphaCount < 100)
		DrawNextAlphaCount++;
	
  	if(DrawNextAlphaCount >= 100 && DrawNextFlag == FALSE)
		DrawNextFlag = TRUE;
	
    if(DrawNextFlag)
  	{
		if(DrawOldAlphaCount < 20 && DrawNextOldAlphaCount > 0 && (TimeCount%2) == 0)
 			DrawNextOldAlphaCount--;
	}
	
	//하늘의 알파값들이 완전이 변해서 하늘의 변화가 완전히 끝났을 경우..
	if(DrawOldAlphaCount == 0 && DrawNextOldAlphaCount == 0)
	{
		DrawOldAlphaCount = -1;
		DrawNextOldAlphaCount = -1;
		DrawOldIndex = -1;
	}
	
	if(FieldStartColorDelay != -1)
	{
		if(FieldStartColorDelay <= TimeCount)
		{ 
			BackColor_mR = FieldColorR;
			BackColor_mG = FieldColorG;
			BackColor_mB = FieldColorB;
			FieldStartColorDelay = -1;
		}
	}
	
	if( (FogStartColorR != FogEndColorR) ||
		(FogStartColorG != FogEndColorG) ||
		(FogStartColorB != FogEndColorB) ||
 		(FogStartColorA != FogEndColorA) ||
		(FogStartPos1 != FogStartPos2) ||
		(FogEndPos1 != FogEndPos2))
	{
		if(FogStartColorR < FogEndColorR)
			FogStartColorR++;
		if(FogStartColorR > FogEndColorR)
			FogStartColorR--;
		
		if(FogStartColorG < FogEndColorG)
			FogStartColorG++;
		if(FogStartColorG > FogEndColorG)
			FogStartColorG--;

		if(FogStartColorB < FogEndColorB)
			FogStartColorB++;
		if(FogStartColorB > FogEndColorB)
			FogStartColorB--;

		if(FogStartColorA < FogEndColorA)
			FogStartColorA++;
		if(FogStartColorA > FogEndColorA)
			FogStartColorA--;
		


		if(FogStartPos1 < FogStartPos2)
			FogStartPos1++;
		if(FogStartPos1 > FogStartPos2)
			FogStartPos1--;
		
		if(FogEndPos1 < FogEndPos2)
			FogEndPos1++;
		if(FogEndPos1 > FogEndPos2)
			FogEndPos1--;
		

  		smRender.m_dwFogColor = RGBA_MAKE(FogStartColorR, FogStartColorG, FogStartColorB, FogStartColorA);
		if( (smRender.m_dwFogColor%10) == 0)
		lpD3DDevice->SetRenderState( D3DRENDERSTATE_FOGCOLOR, smRender.m_dwFogColor);
		
		smRender.m_fFogStNum  = (float)FogStartPos1*4.f;
		if( (FogStartPos1 % 10) == 0)
		lpD3DDevice->SetRenderState( D3DRENDERSTATE_FOGTABLESTART, *((DWORD*)(&smRender.m_fFogStNum)) );
		
		if( (FogEndPos1 % 10) == 0)
		smRender.m_fFogEndNum = (float)FogEndPos1*4.f;
		lpD3DDevice->SetRenderState( D3DRENDERSTATE_FOGTABLEEND, *((DWORD*)(&smRender.m_fFogEndNum)) );

		if( (FogStartColorR == FogEndColorR) &&
			(FogStartColorG == FogEndColorG) &&
			(FogStartColorB == FogEndColorB) &&
			(FogStartColorA == FogEndColorA) &&
			(FogStartPos1 == FogStartPos2) &&
		    (FogEndPos1 == FogEndPos2) )

		{
		 	smRender.m_dwFogColor = RGBA_MAKE(FogStartColorR, FogStartColorG, FogStartColorB, FogStartColorA);
			lpD3DDevice->SetRenderState( D3DRENDERSTATE_FOGCOLOR, smRender.m_dwFogColor);
		
			smRender.m_fFogStNum  = (float)FogStartPos1*4.f;
			lpD3DDevice->SetRenderState( D3DRENDERSTATE_FOGTABLESTART, *((DWORD*)(&smRender.m_fFogStNum)) );
		
			smRender.m_fFogEndNum = (float)FogEndPos1*4.f;
			lpD3DDevice->SetRenderState( D3DRENDERSTATE_FOGTABLEEND, *((DWORD*)(&smRender.m_fFogEndNum)) );
			
			if(!SkyDrawInfoList[DrawIndex].FogFlag)
				if(smRender.m_FogMode)
					smRender.m_FogMode = 0;
		}
	}

	TimeCount++;
	return true;
}

void HoSky::LoadRes()
{
	for(unsigned int index = 0; index < SkyDrawInfoList.size(); index++)
	{
		SkyDrawInfoList[index].LoadRes();
	}

}


void HoSky::DestroyRes()
{
	for(unsigned int index = 0; index < SkyDrawInfoList.size(); index++)
	{
		SkyDrawInfoList[index].DestroyRes();
	}
}

extern int DarkLight;
void HoSky::Create()
{

	FILE *fp;	if(smConfig.ScreenColorBit == 32)
 		fp = fopen("Effect\\Sky\\test32.txt", "rt");
	else
		fp = fopen("Effect\\Sky\\test16.txt ", "rt");

	if( fp == NULL)
		return;

	HoSkyDrawInfo	tempSkyDrawInfo;
	skyResInfo      tempSkyResInfo;
	char	bufferLine[256];
	char	bufferResName[256];
	char	token[256];
	while( !feof(fp) )
	{
		fgets(bufferLine, 256, fp);
		sscanf(bufferLine, "%s", token);
		if(_stricmp(token, "TestMode:") == 0)
			sscanf(bufferLine, "%s%d", token, &TestModeFlag);
		if(_stricmp(token, "StartSky:") == 0)
			sscanf(bufferLine, "%s%d", token, &DrawIndex);
		sscanf(bufferLine, "%s%s", token, token);		//하늘 이름이 나오고 '{' 이 나온다.

		if(_stricmp(token, "{") == 0)
		{
 			tempSkyDrawInfo.FogFlag = false;
			while(_stricmp(token, "}") != 0)
			{
				if(_stricmp(token, "LightColor:") == 0)
					sscanf(bufferLine, "%s%d%d%d", token, &tempSkyDrawInfo.LightColorR, &tempSkyDrawInfo.LightColorG, &tempSkyDrawInfo.LightColorB);
				if(_stricmp(token, "LightDark:") == 0)
					sscanf(bufferLine, "%s%d", token, &tempSkyDrawInfo.LightDark);
				if(_stricmp(token, "FileName:") == 0)
				{
					sscanf(bufferLine, "%s%s%s%d%s%d%d%d%d",
					token, bufferResName, token, &tempSkyResInfo.fadeSlowFlag, token, 
					&tempSkyResInfo.colorR,
					&tempSkyResInfo.colorG,
					&tempSkyResInfo.colorB,
 					&tempSkyResInfo.colorA);
					
					tempSkyResInfo.resName = bufferResName;
					tempSkyDrawInfo.SkyResInfoList.push_back(tempSkyResInfo);
					tempSkyResInfo.fadeSlowFlag = 0;
				}

				//포그 관련 값을 읽어 들이기....
				if(_stricmp(token, "Fog:") ==  0) 
				{
					tempSkyDrawInfo.FogFlag = true;
					sscanf(bufferLine, "%s%s%d%s%d%s%d%d%d%d", token, token
						,&tempSkyDrawInfo.FogStartPos, token, &tempSkyDrawInfo.FogEndPos, token, &tempSkyDrawInfo.FogColorR,
						&tempSkyDrawInfo.FogColorG, &tempSkyDrawInfo.FogColorB, &tempSkyDrawInfo.FogColorA);

				}
				
				//Clipping 관련
				if(_stricmp(token, "Clipping:") == 0)
				{
					sscanf(bufferLine, "%s%d", token, &tempSkyDrawInfo.ClippingFlag);
				}
				
				// 한라인 처리하기
				fgets(bufferLine, 256, fp);
				sscanf(bufferLine, "%s", token);
				// 주석 처리
				if(strlen(token) >= 10)
				{
					if(token[0] == '/' && token[1] == '/')
					{
						fgets(bufferLine, 256, fp);
						sscanf(bufferLine, "%s", token);	//한줄을 넘긴다.
					}
				}
			}

 			//if(tempSkyDrawInfo.SkyResInfoList.size())
			//{
 				SkyDrawInfoList.push_back(tempSkyDrawInfo);
				tempSkyDrawInfo.FogFlag = false;
				tempSkyDrawInfo.ClippingFlag = true;
				tempSkyDrawInfo.ChangeSkyNum = 0;
				tempSkyDrawInfo.ChangeSkyTime = 0;
				tempSkyDrawInfo.ChangeNextSkyEnd = false;
				tempSkyDrawInfo.SkyResInfoList.erase(tempSkyDrawInfo.SkyResInfoList.begin(), tempSkyDrawInfo.SkyResInfoList.end());
			//}

		}
	}

	if(fp)
		fclose(fp);
	
	LoadRes();
	
	if(DrawIndex != -1 && TestModeFlag)
	{
		//이미지를 처음 읽을 때만..
 		if(SkyDrawInfoList[DrawIndex].FogFlag)
		{
 			smRender.m_FogMode = 1;
			
			FogEndColorR = SkyDrawInfoList[DrawIndex].FogColorR;
			FogEndColorG = SkyDrawInfoList[DrawIndex].FogColorG;
			FogEndColorB = SkyDrawInfoList[DrawIndex].FogColorB;
			FogEndColorA = SkyDrawInfoList[DrawIndex].FogColorA;

			FogStartPos2 = SkyDrawInfoList[DrawIndex].FogStartPos/4;
 			FogEndPos2   = SkyDrawInfoList[DrawIndex].FogEndPos/4;
			
			if(FogStartPos2 > 3000/4)
				FogStartPos2 = 3000/4;

			if(FogEndPos2 > 3000/4)
				FogEndPos2 = 3000/4;

		}
		else
		{
			//포그 모드인 경우 포그는 꺼준다.
			if(smRender.m_FogMode)
			{
				FogStartPos2 = FogStartPos1 + 3000/4;
				FogEndPos2 = FogEndPos1 + 3000/4;
			}
		}	
		
		SetFieldColor(SkyDrawInfoList[DrawIndex].LightColorR, SkyDrawInfoList[DrawIndex].LightColorG,
		SkyDrawInfoList[DrawIndex].LightColorB, SkyDrawInfoList[DrawIndex].LightDelay);
		
		DarkLight = SkyDrawInfoList[DrawIndex].LightDark;
		DarkLevel_Fix = 0;
		TimeCount = 0;
	}
	
	FirstSkyLoadFlag = TRUE;
}


void HoSky::Init()
{
    DrawIndex = -1;
	DrawOldIndex = -1;
	DrawAlphaCount = 100;
	TimeCount = 0;
	CodeID = -1;
	FieldStartColorDelay = -1;

	FieldColorR = 0;
	FieldColorG = 0;
	FieldColorB = 0;

	//FogStartColor = 0;
	//FogEndColor = 0;
	FogStartColorR = 0;
	FogStartColorG = 0;
	FogStartColorB = 0;
	FogStartColorA = 0;

	FogEndColorR = 0;
	FogEndColorG = 0;
	FogEndColorB = 0;
	FogEndColorA = 0;

	FogStartPos1 = 1000;
	FogStartPos2 = 1000;

	FogEndPos1 = 3000;
	FogEndPos2 = 3000;
	
	BlinkTime = 200;
	BlinkColor = 65;
	BlinkFlag = FALSE;
	BlinkDrawFlag = FALSE;
	BlinkCount = 0;
	ParticleCount = 0;
	SnowParticleNum = -1;
	StartSnowFlag = FALSE;
}


//버퍼 내용 초기화..
void HoSky::Destroy()
{
	DestroyRes();
	SkyDrawInfoList.erase(SkyDrawInfoList.begin(), SkyDrawInfoList.end());
	DrawIndex = -1;
}


unsigned int HoSky::SetDrawIndex(unsigned drawIndex, int codeID)
{

	CodeID = codeID;
 	if(drawIndex >= SkyDrawInfoList.size() && drawIndex < -1)
		return -1;

	//이미지를 처음 읽을 때만..
 	if(DrawIndex == -1)
	{
		DrawOldIndex = -1;
		DrawIndex = drawIndex;

		DrawAlphaCount = 100;
	}
	else
	{
		DrawOldIndex = DrawIndex;
		DrawIndex = drawIndex;
		
		
		DrawAlphaCount = 50;
		DrawOldAlphaCount = 100;
		
		DrawNextAlphaCount = 0;
		DrawNextOldAlphaCount = 100;
		DrawNextFlag = FALSE;
	}
	
	if(DrawIndex != -1)
 	{
		if(SkyDrawInfoList[DrawIndex].FogFlag)
		{
 			smRender.m_FogMode = 1;
			
			FogEndColorR = SkyDrawInfoList[DrawIndex].FogColorR;
			FogEndColorG = SkyDrawInfoList[DrawIndex].FogColorG;
			FogEndColorB = SkyDrawInfoList[DrawIndex].FogColorB;
			FogEndColorA = SkyDrawInfoList[DrawIndex].FogColorA;

			FogStartPos2 = SkyDrawInfoList[DrawIndex].FogStartPos/4;
 			FogEndPos2   = SkyDrawInfoList[DrawIndex].FogEndPos/4;
			
			if(FogStartPos2 > 3000/4)
				FogStartPos2 = 3000/4;

			if(FogEndPos2 > 3000/4)
				FogEndPos2 = 3000/4;

		}
		else
		{
			//포그 모드인 경우 포그는 꺼준다.
			if(smRender.m_FogMode)
			{
				FogStartPos2 = FogStartPos1 + 3000/4;
				FogEndPos2 = FogEndPos1 + 3000/4;
			}
		}	
	}
	
	SetFieldColor(SkyDrawInfoList[DrawIndex].LightColorR, SkyDrawInfoList[DrawIndex].LightColorG,
		SkyDrawInfoList[DrawIndex].LightColorB, SkyDrawInfoList[DrawIndex].LightDelay);
	
	DarkLight = SkyDrawInfoList[DrawIndex].LightDark;
	DarkLevel_Fix = 0;
	TimeCount = 0;
	
	return DrawIndex;
}


unsigned int HoSky::SetNextDrawIndex()
{
    DrawOldIndex = DrawIndex;
	DrawIndex++;
	if(DrawIndex >= (int)SkyDrawInfoList.size())
		DrawIndex = 0;
	if(DrawIndex != -1)
 	{
		/*
		int testNum = SkyDrawInfoList[DrawIndex].ChangeSkyNum;

		if(SkyDrawInfoList[DrawIndex].ChangeSkyNum != -1)
		{
			SkyChangeFlag = TRUE;
			SkyChangeState = 0;
			SkyChangeTimeCount = 0;

			//SkyChangeTimeCount = SkyDrawInfoList[DrawIndex].ChangeSkyTime;
		}
		*/

 		if(SkyDrawInfoList[DrawIndex].FogFlag)
		{
 			smRender.m_FogMode = 1;
			
			FogEndColorR = SkyDrawInfoList[DrawIndex].FogColorR;
			FogEndColorG = SkyDrawInfoList[DrawIndex].FogColorG;
			FogEndColorB = SkyDrawInfoList[DrawIndex].FogColorB;
			FogEndColorA = SkyDrawInfoList[DrawIndex].FogColorA;

			FogStartPos2 = SkyDrawInfoList[DrawIndex].FogStartPos/4;
 			FogEndPos2   = SkyDrawInfoList[DrawIndex].FogEndPos/4;
			
			if(FogStartPos2 > 3000/4)
				FogStartPos2 = 3000/4;

			if(FogEndPos2 > 3000/4)
				FogEndPos2 = 3000/4;

		}
		else
		{
			//포그 모드인 경우 포그는 꺼준다.
			if(smRender.m_FogMode)
			{
				FogStartPos2 = FogStartPos1 + 3000/4;
				FogEndPos2 = FogEndPos1 + 3000/4;
			}
		}	
	}


	DrawAlphaCount = 50;
	DrawOldAlphaCount = 100;

	DrawNextAlphaCount = 0;
	DrawNextOldAlphaCount = 100;
	DrawNextFlag = FALSE;
	
	/*
	if(DrawTimeAlphaCountState == 4)
	{
		DrawTimeAlphaCount = 0;
		DrawTimeAlphaCountState = 0;
	}
	*/

	TimeCount = 0;
	
	int r = SkyDrawInfoList[DrawIndex].LightColorR;
	int g = SkyDrawInfoList[DrawIndex].LightColorG;
	int b = SkyDrawInfoList[DrawIndex].LightColorB;
	
 	SetFieldColor(SkyDrawInfoList[DrawIndex].LightColorR, SkyDrawInfoList[DrawIndex].LightColorG,
		SkyDrawInfoList[DrawIndex].LightColorB, SkyDrawInfoList[DrawIndex].LightDelay);
	
	DarkLight = SkyDrawInfoList[DrawIndex].LightDark;
	
	return DrawIndex;
}

int InitSky()
{
	SkyBackGround.Init();
	return TRUE;
}

int CreateSky()
{
	if(SkyBackGround.FirstSkyLoadFlag == FALSE)
	{
		SkyBackGround.Init();
		SkyBackGround.Destroy();
		SkyBackGround.Create();
	}
	
	return TRUE;
}

int ReCreateSky()
{
	SkyBackGround.Init();
	SkyBackGround.Destroy();
	SkyBackGround.Create();
	
	return TRUE;
}

int DrawSky(int x, int y, int z, int ax, int ay, int az) //eCAMERA_TRACE *cameraTrace)
{
	
	int OnStageField;
	if ( !lpCurPlayer ) 
		OnStageField=0;
	else 
		OnStageField = lpCurPlayer->OnStageField;

	if(StageField[OnStageField] != NULL)
	{
		if( StageField[OnStageField]->State != FIELD_STATE_DUNGEON && StageField[OnStageField]->State!=FIELD_STATE_ROOM)
		SkyBackGround.Draw(x, y, z, ax, ay, az);
	}
	return TRUE;
}

int DrawOpeningSky(int x, int y, int z, int ax, int ay, int az)
{
	SkyBackGround.Draw(x, y, z, ax, ay, az);
	//SkyBackGround.DrawZBuffer(x, y, z, ax, ay, az);
	return TRUE;
}


//static BOOL DungeonInFlag = FALSE;
//static int  DungeonDarkLightFieldCode = 0;

int MainSky()
{
  	SkyBackGround.Main();
	

	int OnStageField;
	if ( !lpCurPlayer ) 
		OnStageField=0;
	else 
		OnStageField = lpCurPlayer->OnStageField;

	//던전일 경우 필드 배경 칼라 값을 바로 바꾼다.
	if ( StageField[OnStageField] &&
	   ( StageField[OnStageField]->State==FIELD_STATE_DUNGEON ||
 		 StageField[OnStageField]->State==FIELD_STATE_ROOM))
	{

		int codeID = StageField[OnStageField]->FieldCode+USER_SKY_ADD_CODE;
		if(SkyBackGround.GetCodeID() != codeID)
		{
			smRender.m_FogMode = 0;
			SkyBackGround.SetCodeID(codeID);
			
			
			DarkLevel_Fix = 0;
			BackColor_R = 0;
			BackColor_G = 0;
			BackColor_B = 0;

			BackColor_mR = 0;
			BackColor_mG = 0;
			BackColor_mB = 0;
				
			DarkLevel = 110;
			DarkLight = 110;
			
			switch( StageField[OnStageField]->FieldCode ) {
			case 24: //버섯 동굴
  				BackColor_R = 0;
				BackColor_G = 10;
				BackColor_B = 40;

 		 		BackColor_mR = 0;
				BackColor_mG = 10;
				BackColor_mB = 40;
				
				DarkLevel = 60;
				DarkLight = 60;
				break;

			case 25: //벌집 동굴
				
  	  			BackColor_R = 10;
				BackColor_G = 0;
				BackColor_B = -20;

				BackColor_mR = 10;
				BackColor_mG = 0;
				BackColor_mB = -20; 
				
				DarkLevel = 60;
				DarkLight = 60;

				break;

			case 26: //어둠의 성전
				
 				BackColor_R = 0;
				BackColor_G = 10;
				BackColor_B = 40;

				BackColor_mR = 0;
				BackColor_mG = 10;
				BackColor_mB = 40; 

				DarkLevel = 40;
				DarkLight = 40;

				break;

			case 22: //신전1
			case 23: //신전2
				BackColor_R = 0;
				BackColor_G = 0;
				BackColor_B = 0;

				BackColor_mR = 0;
				BackColor_mG = 0;
				BackColor_mB = 0;
				
 				DarkLevel = 40;
				DarkLight = 40;
				break;
			case 40: // pluto 추가 던전
			case 41:
				BackColor_R = 0;
				BackColor_G = 10;
				BackColor_B = 40;

				BackColor_mR = 0;
				BackColor_mG = 10;
				BackColor_mB = 40; 

				DarkLevel = 40;
				DarkLight = 40;

				break;
			}
		}
	}
	return TRUE;
}

int DestroySky()
{
	SkyBackGround.Destroy();
	return TRUE;
}

int ChangeSky(int codeID)
{
   	if(SkyBackGround.TestModeFlag == FALSE)
	{
		if(codeID == FIELD_BACKIMAGE_NIGHTIRON1 ||
	 	   codeID == FIELD_BACKIMAGE_NIGHTIRON2)
		{
			if(dwGameHour >= 1 && dwGameHour < 3)
				codeID = FIELD_BACKIMAGE_AURORA;
		}
  		
		if(SkyBackGround.GetCodeID() != codeID) //필드에 변화가 생겼을 경우만 변하게 한다.
		{
	 	 	//던젼의 경우는 갑자기 필드가 변하므로 조명을 완전히 바꾼다..
			//게임에서 넘어오는 하늘 상태 값들 
   	  		if(codeID == FIELD_BACKIMAGE_RUIN1)
				SkyBackGround.SetDrawIndex(0, codeID);   //0번 하늘
			else if(codeID == FIELD_BACKIMAGE_RUIN2)
				SkyBackGround.SetDrawIndex(1, codeID);   //1번 하늘
			else if(codeID == FIELD_BACKIMAGE_GLOWRUIN1)
				SkyBackGround.SetDrawIndex(2, codeID);   //2번 하늘
			else if(codeID == FIELD_BACKIMAGE_GLOWRUIN2)
				SkyBackGround.SetDrawIndex(2, codeID);   //2번 하늘
			else if(codeID == FIELD_BACKIMAGE_NIGHTRUIN1)
				SkyBackGround.SetDrawIndex(3, codeID);	 //3번 하늘
			else if(codeID == FIELD_BACKIMAGE_NIGHTRUIN2)
				SkyBackGround.SetDrawIndex(3, codeID);   //3번 하늘
			else if(codeID == FIELD_BACKIMAGE_DAY)
				SkyBackGround.SetDrawIndex(4, codeID);   //4번 하늘
			else if(codeID == FIELD_BACKIMAGE_GLOWDAY)
				SkyBackGround.SetDrawIndex(5, codeID);   //5번 하늘
			else if(codeID == FIELD_BACKIMAGE_NIGHT)
				SkyBackGround.SetDrawIndex(6, codeID);   //6번 하늘
			else if(codeID == FIELD_BACKIMAGE_DESERT)
				SkyBackGround.SetDrawIndex(8, codeID);   //8번 하늘
			else if(codeID == FIELD_BACKIMAGE_GLOWDESERT)
				SkyBackGround.SetDrawIndex(9, codeID);   //9번 하늘
			else if(codeID == FIELD_BACKIMAGE_NIGHTDESERT)
				SkyBackGround.SetDrawIndex(10, codeID);   //10번 하늘
			else if(codeID == FIELD_BACKIMAGE_DAYFALL)
				SkyBackGround.SetDrawIndex(11, codeID);   //11번 하늘
			else if(codeID == FIELD_BACKIMAGE_GLOWFALL)
				SkyBackGround.SetDrawIndex(12, codeID);   //12번 하늘
			else if(codeID == FIELD_BACKIMAGE_NIGHTFALL)
				SkyBackGround.SetDrawIndex(13, codeID);   //13번 하늘
			else if(codeID == FIELD_BACKIMAGE_DAYIRON)
				SkyBackGround.SetDrawIndex(14, codeID);   //14번 하늘
			else if(codeID == FIELD_BACKIMAGE_GLOWIRON)
				SkyBackGround.SetDrawIndex(15, codeID);   //15번 하늘
			else if(codeID == FIELD_BACKIMAGE_NIGHTIRON1 || codeID == FIELD_BACKIMAGE_NIGHTIRON2)
				SkyBackGround.SetDrawIndex(16, codeID);   //16번 하늘
			else if(codeID == FIELD_BACKIMAGE_AURORA)
				SkyBackGround.SetDrawIndex(17, codeID);   //17번 하늘
			else if(codeID == FIELD_BACKIMAGE_SODSKY)     //천공의 홀
				SkyBackGround.SetDrawIndex(18, codeID);   
			else if(codeID == FIELD_BACKIMAGE_SODMOON)     //달의 장
				SkyBackGround.SetDrawIndex(19, codeID);
			else if(codeID == FIELD_BACKIMAGE_SODSUN)      //태양의 장
				SkyBackGround.SetDrawIndex(9, codeID);
			else if(codeID == FIELD_BACKIMAGE_SODNONE)     //아무것두 안나오는 소드
				SkyBackGround.SetDrawIndex(20, codeID);
			else if(codeID == FIELD_BACKIMAGE_IRONBOSS)    //아이언 보스몹 하늘
				SkyBackGround.SetDrawIndex(21, codeID);
			else if(codeID == FIELD_BACKIMAGE_DAYSNOW)
				SkyBackGround.SetDrawIndex(23, codeID);
			else if(codeID == FIELD_BACKIMAGE_GLOWSNOW)
				SkyBackGround.SetDrawIndex(24, codeID);
			else if(codeID == FIELD_BACKIMAGE_NIGHTSNOW)
				SkyBackGround.SetDrawIndex(25, codeID);
			else if(codeID == FIELD_BACKIMAGE_DAYGREDDY)    //그리디 호수
				SkyBackGround.SetDrawIndex(26, codeID);
			else if(codeID == FIELD_BACKIMAGE_GLOWGREDDY)   //그리디 호수
				SkyBackGround.SetDrawIndex(27, codeID);
			else if(codeID == FIELD_BACKIMAGE_NIGHTGREDDY)  //그리디 호수
				SkyBackGround.SetDrawIndex(28, codeID);	
			
			else if(codeID == FIELD_BACKIMAGE_DAYLOST)		//로스트월드 
				SkyBackGround.SetDrawIndex(29, codeID);
			else if(codeID == FIELD_BACKIMAGE_GLOWLOST)		//로스트월드
				SkyBackGround.SetDrawIndex(30, codeID);
			else if(codeID == FIELD_BACKIMAGE_NIGHTLOST)	//로스트월드
				SkyBackGround.SetDrawIndex(31, codeID);	
			else if(codeID == FIELD_BACKIMAGE_DAYTEMPLE )	//로스트템플
				SkyBackGround.SetDrawIndex(32, codeID);
			else if(codeID == FIELD_BACKIMAGE_GLOWTEMPLE)	//로스트템플
				SkyBackGround.SetDrawIndex(33, codeID);		
			else if(codeID == FIELD_BACKIMAGE_NIGHTTEMPLE)	//로스트템플
				SkyBackGround.SetDrawIndex(34, codeID);		

			else if(codeID == 0x80)
				SkyBackGround.SetDrawIndex(22, 0x80);    //Opening 하늘
			else if(codeID == FIELD_BACKIMAGE_RAIN)
			{
				int OnStageField;
				if ( !lpCurPlayer ) 
					OnStageField=0;
				else 
					OnStageField = lpCurPlayer->OnStageField;
				
				if(StageField[OnStageField] != NULL) 
				{
					//던젼이 아닐 경우에만 비가 오게 한다.
					if( StageField[OnStageField]->State != FIELD_STATE_DUNGEON && StageField[OnStageField]->State!=FIELD_STATE_ROOM)
						SkyBackGround.SetDrawIndex(7, codeID);   //1번 비올때..
				}
				else//NULL일 경우는 필드를 처음 읽을때 이다..
					SkyBackGround.SetDrawIndex(7, codeID);   //1번 비올때..
			}
		}
	}

	return TRUE;
}

int SetNextSky() 
{
    unsigned int drawIndex = SkyBackGround.SetNextDrawIndex();
	return TRUE;
}
