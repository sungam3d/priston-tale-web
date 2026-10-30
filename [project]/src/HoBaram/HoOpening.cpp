#include "..\\sinbaram\\SinLinkHeader.h"
#ifdef PT_GFX_GL
#include "..\\ptgfx\\ptgfx.h"			// shadow test view (?shadowtest)
#include "..\\weather.h"				// the weather picked in the settings, behind the login
#endif
#ifdef PT_WEB
#include <emscripten.h>
#endif
#include "..\\HoBaram\\HoLinkHeader.h"
#include "..\\smreg.h"
#include "..\\HoBaram\\HoTextFile.h"
#include "..\\HoBaram\\HoMinMax.h"
#include "..\\HoBaram\\HoNewParticle.h"
#include "..\\HoBaram\\HoNewParticleMgr.h"

#include "..\\language.h"

//######################################################################################
//작 성 자 : 오 영 석
#include "..\\CAutoCamera.h"

static CAutoCamera		s_AutoCamera;
//######################################################################################

#ifdef _LANGUAGE_KOREAN
char *szJoinWebUrl="http://www.pristontale.co.kr";
#endif

#ifdef _LANGUAGE_CHINESE
char *szJoinWebUrl="http://www.pristontale.cn";
#endif

#ifdef _LANGUAGE_JAPANESE
char *szJoinWebUrl="http://www.pristontale.jp";
#endif

#ifdef	_LANGUAGE_TAIWAN
char *szJoinWebUrl="http://www.pristontale.com.tw";
#endif

#ifdef _LANGUAGE_BRAZIL	
char *szJoinWebUrl="http://www.pristontale.com";
#endif

#ifdef _LANGUAGE_ARGENTINA
char *szJoinWebUrl="http://www.axeso5.com";
#endif

#ifdef	_LANGUAGE_ENGLISH
#ifdef	_LANGUAGE_PHILIPIN
char *szJoinWebUrl="http://www.netplay.ph";
#else
char *szJoinWebUrl="http://www.pristontale.com";
#endif
#endif

#ifdef _LANGUAGE_VEITNAM
char *szJoinWebUrl="http://www.ptv.com.vn";
#endif

#ifdef	_LANGUAGE_THAI
char *szJoinWebUrl="http://www.pristontale.in.th";
#endif

extern POINT3D WaveCameraPosi;									
extern int	   WaveCameraFactor;
extern int	   WaveCameraTimeCount;
extern int	   WaveCameraDelay;
extern BOOL	   WaveCameraFlag;

char	UserAccount[256];
char	UserPassword[256];
HoOpening::HoOpening()
{
	//Bird = NULL;
	//BirdBone = NULL;
	Stage = NULL;
	PatCamera = NULL;

	LogoBlendValue = 0;
	LoginBlendValue = 0;
	
	Load();
	StartConnectServerFlag = FALSE;
	WorkState = STATE_OPENING;
	
	MidY = (600/2);
	SetWindowText(hTextWnd, NULL);
	//ID, 비밀번호 Buffer초기화..
	memset(UserAccount, 0, sizeof(UserAccount));
	memset(UserPassword, 0, sizeof(UserPassword));
	
	TarDist = 1600*fONE;
	CaAngle.x = 220;
	CaAngle.y = 0;
	CaAngle.z = 0;

	//TarPosi.x = 0;
	//TarPosi.y = 0;
	//TarPosi.z = 0;
	MouseDownServerIndex = -1;
	CameraFrame = 0;
}


HoOpening::~HoOpening()
{
	Close();
}

int HoOpening::Init()
{
	GameErrorValue = -1;
	ConnectErrorCount = 0;
	FocusWndFlag = FALSE;
	SetWindowText(hTextWnd, "");
	SendMessage( hTextWnd , EM_SETLIMITTEXT , 31, 0 );
	
	LoginInputState = 0;
	ConnectError = ERROR_NONE;
	ServerWaitTimeCount = 0;
	StartQuitFlag = FALSE;
	
	WorkState = STATE_OPENING;
	StartConnectServerFlag = FALSE;
	ConnectServerSucessFlag = FALSE;

	FadeCurrentState = FADE_OUT;
	FadeNextState = FADE_OUT;
	
	
	LogoBlendValue = 0;
	LoginBlendValue = 0;
	ServerSelectBlendValue = 0;
	
	MidX = WinSizeX / 2;
	MidY = WinSizeY	/ 2;
	//smRender.Init();
	smRender.SMMULT_PERSPECTIVE_HEIGHT = RENDCLIP_WIDE_MULT_PERSPECTIVE_HEIGHT;

	SetWindowText(hTextWnd, NULL);
	//ID, 비밀번호 Buffer초기화..
	memset(UserAccount, 0, sizeof(UserAccount));
	memset(UserPassword, 0, sizeof(UserPassword));
	
	TarDist = 400*fONE;
	
    CaAngle.x = 100;
    CaAngle.y = 0;
	CaAngle.z = 0;
	
	SetFade();
	
	smInitColorZClip(0, 0, 0);
		
	smRender.Color_R = 40;
	smRender.Color_G = 0;
	smRender.Color_B = -30;
	smRender.Color_A = 0;
	smRender.ClearLight();
	
	StopBGM();
	OpenBGM("StartImage\\Opening\\Intro.wav");
	PlayBGM();
	SetVolumeBGM(OpeningBgmVolume);

/* 해외버전 주석처리	
#ifndef _LANGUAGE_KOREAN
	char *buffer;
	buffer = GetRegString( HKEY_LOCAL_MACHINE , "SOFTWARE\\Triglow Pictures\\PristonTale", "Account" );
    
	if(buffer != NULL)
	{
		memset(UserAccount, 0, sizeof(UserAccount));
		memcpy(UserAccount, buffer, strlen(buffer));
		LoginInputState = 1;
	}
#endif
*/

	CurrentWorkState = STATE_OPENING;
	NextWorkState    = STATE_OPENING;
	MouseDownServerIndex = -1;
	MouseDownFlag = FALSE;
	
	CameraFrame = 160;
	ChangeSky(0x21);
	ChangeSky(0x80);

	g_NewParticleMgr.Init();
	InitAssaEffect();

	InitCinemaFadeEffect();

	//######################################################################################
	//작 성 자 : 오 영 석		
	smIdentityMatrix( m_CameraRotInvert );

	m_IsDrawOpening		     = TRUE;
	m_AutoCameraFrameTime	 = 50;
	m_ParticleSpawnTimeCount = 0;

	s_AutoCamera.m_Frame = 0;
	//######################################################################################
	
	m_AddWidth  = (smScreenWidth-800)/2;
	m_AddHeight = (smScreenHeight-600)/2;

	startLogoX = 100 + m_AddWidth;
	startLogoY = 63 + m_AddHeight;
	startLogoAddX = 32;
	startAccountAddX = 230;
	startAccountAddY = 290;

	return TRUE;
}


//######################################################################################
//작 성 자 : 오 영 석
void HoOpening::SetCameraCharPos(void)
{
	int x = 0;
	int y = 0;
	int z = (440 << FLOATNS);

	smEMATRIX eCameraRotInvert;	
	smEMatrixInvert( eCameraRotInvert, s_AutoCamera.m_eCameraRotMat );

	memcpy( (smEMATRIX *)&m_CameraRotInvert, &eCameraRotInvert, sizeof(smEMATRIX) );
	smMatrixShift( m_CameraRotInvert, 4 );

	int rx = x * m_CameraRotInvert._11 +
			 y * m_CameraRotInvert._21 +
			 z * m_CameraRotInvert._31 ;

	int ry = x * m_CameraRotInvert._12 +
			 y * m_CameraRotInvert._22 +
			 z * m_CameraRotInvert._32 ;

	int rz = x * m_CameraRotInvert._13 +
		     y * m_CameraRotInvert._23 +
			 z * m_CameraRotInvert._33 ;

	rx >>= FLOATS_EM;
	ry >>= FLOATS_EM;
	rz >>= FLOATS_EM;

	lpCurPlayer->pX = int(s_AutoCamera.m_fCameraPos_x) + rx;
	lpCurPlayer->pY = int(s_AutoCamera.m_fCameraPos_y) + ry;
	lpCurPlayer->pZ = int(s_AutoCamera.m_fCameraPos_z) + rz;
}
//######################################################################################


//######################################################################################
//작 성 자 : 오 영 석
void HoOpening::SetParticleEffectSpawn(void)
{
	m_ParticleSpawnTimeCount++;
	if( (m_ParticleSpawnTimeCount % 70) != 0 )
		return;

	m_ParticleSpawnTimeCount = 0;

	//
	POINT3D curPos;
	POINT3D desPos;
	int		rx, ry, rz;
	int		Type   = (rand() % 100);
	int		rand_1 = (rand() % 1000) << 8;
	int		rand_2 = (rand() % 1000) << 8;
	int		rand_3 = (rand() % 1000) << 8;
	int		rand_4 = (rand() % 1000) << 8;
	
	//
	if( Type < 10 )
	{
		curPos.x = -rand_1;
		desPos.x =  rand_2;
	}
	else
	{
		curPos.x =  rand_1;
		desPos.x = -rand_2;
	}
	
	//
	curPos.y =  200000;
	desPos.y = -200000;

	if( Type < 30 )
	{
		curPos.z = rand_1;
		desPos.z = rand_2;
	}
	else
	{
		curPos.z = rand_3;
		desPos.z = rand_4;
	}

	//
	rx = curPos.x * m_CameraRotInvert._11 +
  		 curPos.y * m_CameraRotInvert._21 +
		 curPos.z * m_CameraRotInvert._31;

	ry = curPos.x * m_CameraRotInvert._12 +
		 curPos.y * m_CameraRotInvert._22 +
		 curPos.z * m_CameraRotInvert._32;

	rz = curPos.x * m_CameraRotInvert._13 +
		 curPos.y * m_CameraRotInvert._23 +
		 curPos.z * m_CameraRotInvert._33;

	rx >>= FLOATS_EM;
	ry >>= FLOATS_EM;
	rz >>= FLOATS_EM;

	curPos.x = lpCurPlayer->pX + rx;
	curPos.y = lpCurPlayer->pY + ry;
	curPos.z = lpCurPlayer->pZ + rz;

	//
	rx = desPos.x * m_CameraRotInvert._11 +
  		 desPos.y * m_CameraRotInvert._21 +
		 desPos.z * m_CameraRotInvert._31;

	ry = desPos.x * m_CameraRotInvert._12 +
		 desPos.y * m_CameraRotInvert._22 +
		 desPos.z * m_CameraRotInvert._32;

	rz = desPos.x * m_CameraRotInvert._13 +
		 desPos.y * m_CameraRotInvert._23 +
		 desPos.z * m_CameraRotInvert._33;

	rx >>= FLOATS_EM;
	ry >>= FLOATS_EM;
	rz >>= FLOATS_EM;

	desPos.x = lpCurPlayer->pX + rx;
	desPos.y = lpCurPlayer->pY + ry;
	desPos.z = lpCurPlayer->pZ + rz;

	//
	AssaParticle_TerrainFire(&curPos, &desPos);
}

//######################################################################################


//######################################################################################
//작 성 자 : 오 영 석
int HoOpening::DebugModeLoadFieldInfo(void)
{
	FILE *fp = fopen( "StartImage\\Opening\\OpeningField.txt", "rb" );
	if( fp == NULL )
		return FALSE;

	Stage = NULL;

	//
	char *p, line[256], decode[256], strBuff[256];
	while( !feof( fp ) )//  feof: file end까지 읽어라 
	{
		if( fgets( line, 255, fp ) == NULL)
		{
			fclose(fp);
			break;
		}

		p = GetWord( decode, line);

		//
		if( lstrcmp( decode, "*\307\312\265\345\270\355") == 0 )
		{
			p = GetWord( strBuff, p );
			Stage = smSTAGE3D_ReadASE( strBuff );
			if( Stage )
                Stage->Head = TRUE;
		}
		else if( lstrcmp( decode, "*\304\253\270\336\266\363\270\355") == 0 )
		{
			p = GetWord( strBuff, p );
			s_AutoCamera.ReadASE_AutoCamera( strBuff );
		}
		else if( lstrcmp( decode, "*\304\253\270\336\266\363\274\323\265\265") == 0 )
		{
			p = GetWord( strBuff, p );
			m_AutoCameraFrameTime = atoi( strBuff );
		}
		else if( lstrcmp( decode, "*\277\366\307\301\307\301\267\271\300\323") == 0 )
		{
			p = GetWord( strBuff, p );
			int StFrame = atoi( strBuff );

			p = GetWord( strBuff, p );
			int EndFrame = atoi( strBuff );

			s_AutoCamera.SetWarpFrame( StFrame, EndFrame );
		}
		else if( lstrcmp( decode, "*\301\366\307\374\271\260\303\274") == 0 )
		{
			p = GetWord( strBuff, p );				
			Stage->StageObject->AddObjectFile( strBuff );
		}
		else if( lstrcmp( decode, "*\301\366\307\374\276\326\264\317") == 0 )
		{
			p = GetWord( strBuff, p );
			Stage->StageObject->AddObjectFile( strBuff, strBuff );
		}
	}

	fclose(fp);

	if( Stage == NULL )
		return FALSE;

	return TRUE;
}
//######################################################################################


int HoOpening::Load()
{
	//######################################################################################
	//작 성 자 : 오 영 석
	int IsLoad = FALSE;
	PatCamera = NULL;
	ObjCamera = NULL;

	if( smConfig.DebugMode ) 
		IsLoad = DebugModeLoadFieldInfo();

	if( IsLoad == FALSE )
	{
		Stage = smSTAGE3D_ReadASE( "StartImage\\Opening\\map\\stage.ASE" );
		if( Stage )
		{
			Stage->Head = TRUE;

			s_AutoCamera.ReadASE_AutoCamera( "StartImage\\Opening\\opening_camera.ASE" );
			m_AutoCameraFrameTime = 50;
			s_AutoCamera.SetWarpFrame( 472, 473 );

			Stage->StageObject->AddObjectFile( "StartImage\\Opening\\map\\st-ani02.ASE" );
			Stage->StageObject->AddObjectFile( "StartImage\\Opening\\map\\st-ani03.ASE" );

			Stage->StageObject->AddObjectFile( "StartImage\\Opening\\babel\\st-ani04.ASE", "StartImage\\Opening\\babel\\st-ani04.ASE" );
			Stage->StageObject->AddObjectFile( "StartImage\\Opening\\npc\\opening_NPC_A.ASE", "StartImage\\Opening\\npc\\opening_NPC_A.ASE" );
			Stage->StageObject->AddObjectFile( "StartImage\\Opening\\npc\\opening_NPC_B.ASE", "StartImage\\Opening\\npc\\opening_NPC_B.ASE" );
			Stage->StageObject->AddObjectFile( "StartImage\\Opening\\npc\\opening_NPC_C.ASE", "StartImage\\Opening\\npc\\opening_NPC_C.ASE" );
			Stage->StageObject->AddObjectFile( "StartImage\\Opening\\npc\\opening_NPC_D.ASE", "StartImage\\Opening\\npc\\opening_NPC_D.ASE" );
			Stage->StageObject->AddObjectFile( "StartImage\\Opening\\npc\\opening_NPC_E.ASE", "StartImage\\Opening\\npc\\opening_NPC_E.ASE" );
			Stage->StageObject->AddObjectFile( "StartImage\\Opening\\npc\\opening_NPC_F.ASE", "StartImage\\Opening\\npc\\opening_NPC_F.ASE" );
		}
	}

	if( Stage == NULL )
		return FALSE;

	smGameStage[0] = Stage;
	smGameStage[1] = NULL;
	//######################################################################################
	
	GameLogo[0]     = CreateTextureMaterial( "StartImage\\Opening\\GameLogo1.tga"  , 0, 0, 0,0, SMMAT_BLEND_ALPHA );//Test
	GameLogo[1]     = CreateTextureMaterial( "StartImage\\Opening\\GameLogo2.tga"  , 0, 0, 0,0, SMMAT_BLEND_ALPHA );//Test
	GameLogo[2]     = CreateTextureMaterial( "StartImage\\Opening\\GameLogo3.tga"  , 0, 0, 0,0, SMMAT_BLEND_ALPHA );//Test
	GameLogo[3]     = CreateTextureMaterial( "StartImage\\Opening\\GameLogo4.tga"  , 0, 0, 0,0, SMMAT_BLEND_ALPHA );//Test
	GameLogo[4]     = CreateTextureMaterial( "StartImage\\Opening\\GameLogo5.tga"  , 0, 0, 0,0, SMMAT_BLEND_ALPHA );//Test
	GameLogo[5]     = CreateTextureMaterial( "StartImage\\Opening\\GameLogo6.tga"  , 0, 0, 0,0, SMMAT_BLEND_ALPHA );//Test
	
	ButtonLoginImage[0] = CreateTextureMaterial( "StartImage\\Opening\\S_login_.tga"  , 0, 0, 0,0, SMMAT_BLEND_ALPHA );//Test
	ButtonLoginImage[1] = CreateTextureMaterial( "StartImage\\Opening\\S_login.tga"  , 0, 0, 0,0, SMMAT_BLEND_ALPHA );//Test

	ButtonNewJoinImage[0] = CreateTextureMaterial( "StartImage\\Opening\\S_New_.tga"  , 0, 0, 0,0, SMMAT_BLEND_ALPHA );//Test 
	ButtonNewJoinImage[1] = CreateTextureMaterial( "StartImage\\Opening\\S_New.tga"  , 0, 0, 0,0, SMMAT_BLEND_ALPHA );//Test 

	ButtonQuitImage[0] = CreateTextureMaterial( "StartImage\\Opening\\S_Exit_.tga"  , 0, 0, 0,0, SMMAT_BLEND_ALPHA );//Test 
	ButtonQuitImage[1] = CreateTextureMaterial( "StartImage\\Opening\\S_Exit.tga"  , 0, 0, 0,0, SMMAT_BLEND_ALPHA );//Test 
	
	#ifdef _LANGUAGE_JAPANESE
	PristonLogoImage = CreateTextureMaterial( "StartImage\\Opening\\BlackBox.tga"  , 0, 0, 0,0, SMMAT_BLEND_ALPHA );//T
	#else
	PristonLogoImage = CreateTextureMaterial( "StartImage\\Opening\\priston_txt.tga"  , 0, 0, 0,0, SMMAT_BLEND_ALPHA );//T
	#endif

	BlackBoxImage = CreateTextureMaterial( "StartImage\\Login\\BlackBox.tga" ,0 , TRUE ,FALSE , FALSE , SMMAT_BLEND_ALPHA );
	WhiteBoxImage = CreateTextureMaterial( "StartImage\\Login\\WhiteBox.tga" ,0 , TRUE ,FALSE , FALSE , SMMAT_BLEND_ALPHA );


	Message.Init();
	MessageBox.Init();
	
	ReadTextures();
		
	if(Stage)
	{
		Stage->smMaterialGroup->CheckMatreialTextureSwap();
	}	

	return TRUE; 
}

/*
int HoOpening::DrawWide()
{
	dsDrawTexImage(BlackBoxImage,0, 0, 800, 62, 255);
	dsDrawTexImage(BlackBoxImage,0, 600-62, 800, 600, 255);
	return TRUE;
}
*/

int		CaretCount = 0;
DWORD	End = 0;
extern int Client_Version;

#define	DISP_LOGIN_LEN		15




int HoOpening::Draw()
{
	FastFade();
	//CaAngle.y += 1;
	//Event 처리...
	int x,y,z;
	int anx,any;

	//######################################################################################
	//작 성 자 : 오 영 석
	if( GetFilterEffect() == FILTER_EFFECT_SET_BRIGHT_CONTRAST )
	{
		BeginRender();
        ChangeRenderTarget( NEW_TARGET_BACK );
	}
	//######################################################################################

	BeginRender();	

	//######################################################################################
	//작 성 자 : 오 영 석
	int anz;

	x = int(s_AutoCamera.m_fCameraPos_x);
	y = int(s_AutoCamera.m_fCameraPos_y);
	z = int(s_AutoCamera.m_fCameraPos_z) + (WaveCameraFactor*8);

	anx = s_AutoCamera.m_iCameraRot_x;
	any = s_AutoCamera.m_iCameraRot_y;
	anz = s_AutoCamera.m_iCameraRot_z;

	smRender.OpenCameraPosi( x, y, z, &s_AutoCamera.m_eCameraRotMat );
#ifdef PT_WEB
	{	// sky test view (?skytest=N): field sky N (field.h FIELD_BACKIMAGE_*) behind the login
		static int s_SkyTest = -2;
		if ( s_SkyTest==-2 ) {
			s_SkyTest = EM_ASM_INT({ var m = /[?&]skytest=([0-9]+)/.exec(location.search); return m ? +m[1] : -1; });
			if ( s_SkyTest>=0 ) {
				ChangeSky( s_SkyTest );
				extern int ViewPointLen, ccDistZMin;
				printf( "[web] sky test: field sky %d (view %d, colour fade from %d, field view %d step %d)\n", s_SkyTest,
					ViewPointLen, ccDistZMin, smRender.m_GameFieldView, smRender.m_GameFieldViewStep );
			}
		}
		if ( s_SkyTest>=0 ) MainSky();			// fades to it, as in the game
	}
#endif
#ifdef PT_GFX_GL
	PtWeather_Frame( TRUE );
	PtGfx_WeatherWorld( TRUE );
#endif
	DrawOpeningSky(x, y, z, anx, any, anz);
	if(CinemaFadeState != STATE_FADE_OUT)
	{
#ifdef PT_GFX_GL
		if ( PtGfx_ShadowTestBegin( (float)x/fONE, (float)y/fONE, (float)z/fONE ) )	// only with ?shadowtest
			smStage_ShadowMesh( Stage, 0 );		// the intro map casts from its mesh (from the next frame)
#endif
		Stage->DrawOpeningStage( x,y,z, anx, any, anz, s_AutoCamera.m_Frame );
#ifdef PT_GFX_GL
		PtGfx_ShadowTestEnd();
		PtGfx_WeatherWorld( FALSE );
		smRender.OpenCameraPosi( x, y, z, &s_AutoCamera.m_eCameraRotMat );		// the camera the scene was drawn with
		PtWeather_Draw( TRUE, Stage, NULL );
#endif
	}
#ifdef PT_GFX_GL
	PtGfx_WeatherWorld( FALSE );
#endif

	DrawCinemaFadeEffect();

	lpD3DDevice->BeginScene();
	g_NewParticleMgr.Draw(x,y,z, anx, any, anz);
	lpD3DDevice->EndScene();
	
	smRender.CloseCameraPosi();
	//######################################################################################

	
	//######################################################################################
	//작 성 자 : 오 영 석
	if( GetFilterEffect() == FILTER_EFFECT_SET_BRIGHT_CONTRAST && GetRenderTarget() == NEW_TARGET_BACK )
	{
		SetNewTargetTextureState();
		lpD3DDevice->BeginScene();
		RenderFilterEffect();
		lpD3DDevice->EndScene();
		DefaultsNewTargetTextureState();
	}
	//######################################################################################

	//텍스쳐 프레임 증가
	IncTextureFrame();
	//EndRender();
	
	//######################################################################################
	//바뀐곳
    //int startLogoX = 100;
	//int startLogoY = 63;
	//int startLogoAddX = 32;
	//int startAccountAddX = 230;
	//int startAccountAddY = 290;
	
	if(NextWorkState == STATE_LOGIN_INPUT || 
	   NextWorkState == STATE_SERVER_SELECT ||
	   NextWorkState == STATE_END)
	{
		lpD3DDevice->BeginScene();
		//GameLogo찍기.
		dsDrawTexImage( GameLogo[0], startLogoX,	   startLogoY, 256, 256, LogoBlendValue);
		dsDrawTexImage( GameLogo[1], startLogoX+256,   startLogoY, 256, 256, LogoBlendValue);
		dsDrawTexImage( GameLogo[2], startLogoX+256*2, startLogoY,  64, 256, LogoBlendValue);
  		dsDrawTexImage( PristonLogoImage, 423+startLogoX, 114+startLogoY,  128, 32, LogoBlendValue);
		
		lpD3DDevice->EndScene();
	}
	
	if( NextWorkState == STATE_LOGIN_INPUT || CurrentWorkState == STATE_LOGIN_INPUT || 
   	   (NextWorkState == STATE_SERVER_SELECT && CurrentWorkState != STATE_SERVER_SELECT))
	{
   		lpD3DDevice->BeginScene();
		dsDrawTexImage( GameLogo[3], startLogoX+startLogoAddX,	  startLogoY+256, 256, 256, LogoBlendValue);
		dsDrawTexImage( GameLogo[4], startLogoX+startLogoAddX+256,startLogoY+256, 256, 256, LogoBlendValue);
   		HoDrawTexImage( GameLogo[5], float(startLogoX+startAccountAddX), float(startLogoY+startAccountAddY), 128.f, 100.f, 0, 0, 128.f, 100.f, LogoBlendValue);
		
		if(pCursorPos.x > (startLogoX+230-30) &&
		   pCursorPos.y > (startLogoY+320+93) && 
		   pCursorPos.x < (startLogoX+230-30+64) && 
   		   pCursorPos.y < (startLogoY+320+93+25) )
		{
			dsDrawTexImage( ButtonLoginImage[1], startLogoX+230-30,
				startLogoY+320+93,
   				64, 32, LogoBlendValue);
		}
		else
		{
			dsDrawTexImage( ButtonLoginImage[0], startLogoX+230-30,
				startLogoY+320+93,
   				64, 32, LogoBlendValue);
		}
        
 		if(pCursorPos.x > (startLogoX+230+32) && 
		   pCursorPos.y > (startLogoY+320+93) && 
		   pCursorPos.x < (startLogoX+230+32+64) && 
  		   pCursorPos.y < (startLogoY+320+93+25) )
		{
			dsDrawTexImage( ButtonNewJoinImage[1], startLogoX+230+32,
				startLogoY+320+93,
      				64, 32, LogoBlendValue);
		}
		else
		{
			dsDrawTexImage( ButtonNewJoinImage[0], startLogoX+230+32,
				startLogoY+320+93,
      				64, 32, LogoBlendValue);
		}
		

		if(pCursorPos.x > (startLogoX+230+100) && 
		   pCursorPos.y > (startLogoY+320+93) && 
		   pCursorPos.x < (startLogoX+230+100+64) && 
		   pCursorPos.y < (startLogoY+320+93+25) )
		{
			dsDrawTexImage( ButtonQuitImage[1], startLogoX+230+100,
				startLogoY+320+93,
     				64, 32, LogoBlendValue);
		}
		else
		{
			dsDrawTexImage( ButtonQuitImage[0], startLogoX+230+100,
				startLogoY+320+93,
     				64, 32, LogoBlendValue);
		}
		lpD3DDevice->EndScene();
	}
	

	//서버 선택시...
	if(NextWorkState == STATE_SERVER_SELECT || NextWorkState == STATE_END
	   || (CurrentWorkState == STATE_SERVER_SELECT && NextWorkState == STATE_LOGIN_INPUT) )
	{
   		lpD3DDevice->BeginScene();
		dsDrawTexImage( GameLogo[3], startLogoX+startLogoAddX,	  startLogoY+256, 256, 256, LogoBlendValue);
 		dsDrawTexImage( GameLogo[4], startLogoX+startLogoAddX+256,startLogoY+256, 256, 256, LogoBlendValue);

		HoDrawTexImage( GameLogo[5], float(startLogoX+startAccountAddX), 
  			float(startLogoY+startAccountAddY), 128.f, 28.f, 0, 100.f, 128.f, 28.f, LogoBlendValue);

     	
		
		if(pCursorPos.x > (startLogoX+230+32) && 
		   pCursorPos.y > (startLogoY+320+93) && 
		   pCursorPos.x < (startLogoX+230+32+64) && 
   		   pCursorPos.y < (startLogoY+320+93+25) )
		{
			dsDrawTexImage( ButtonQuitImage[1], startLogoX+230+35,
				startLogoY+320+93,
      				64, 32, LogoBlendValue);
		}
		else
		{
			dsDrawTexImage( ButtonQuitImage[0], startLogoX+230+35,
				startLogoY+320+93,
      				64, 32, LogoBlendValue);
		}
		lpD3DDevice->EndScene();

		
		//서버 정보 표시..
		char buffer[256];
		memset(buffer, 0, sizeof(buffer));
		HDC opHdc;
		lpDDSBack->GetDC( &opHdc );
			
		SetBkMode( opHdc, TRANSPARENT );
		
		HFONT oldFont;
		if(hFont != NULL)
			oldFont = (HFONT)SelectObject( opHdc , hFont );
		
		int selectHeight;
		int selectStartPosX;
		int selectStartPosY;
		
  		if(UserServerListInfoCount != 0)
			selectHeight = 90/UserServerListInfoCount;
 		selectStartPosX = 230+startLogoX;
  		selectStartPosY = 315+startLogoY+(selectHeight-12)/2;
 		
 		for(int index = 0; index < UserServerListInfoCount; index++)
		{
			sprintf(buffer, "%s", UserServerListInfo[index].szServerName);
			POINT startServerText;

			if(MouseDownServerIndex != index)
			{
				startServerText.x = selectStartPosX+65-(strlen(buffer)*6/2);
				startServerText.y = selectStartPosY+index*selectHeight;//344+index*(14+4);
			}
			else
			{
				startServerText.x = selectStartPosX+65-(strlen(buffer)*6/2);
				startServerText.y = selectStartPosY+index*selectHeight;//344+index*(14+4);
			}

			//그림자 찍기..
			SetTextColor( opHdc, RGB(0, 0, 0));
			dsTextLineOut(opHdc, startServerText.x+1, startServerText.y+1, buffer, strlen(buffer));
			
 			if(MouseDownServerIndex == -1)
			{
				if( pCursorPos.x > selectStartPosX && (pCursorPos.y > selectStartPosY+index*selectHeight) &&
					pCursorPos.x < (selectStartPosX + 140)  && (pCursorPos.y < (selectStartPosY+index*selectHeight+12)) &&
					ConnectError == ERROR_NONE && !StartConnectServerFlag)
					SetTextColor( opHdc, RGB(250, 200, 0) );
				else
					SetTextColor( opHdc, RGB(190, 190, 190) );
			}
			else
			{
				if(MouseDownServerIndex == index)
					SetTextColor( opHdc, RGB(250, 200, 0));
				else
					SetTextColor( opHdc, RGB(190, 190, 190));
			}
			dsTextLineOut(opHdc, startServerText.x, startServerText.y, buffer, strlen(buffer));
		}
		if(oldFont)
			SelectObject(opHdc, oldFont);
		lpDDSBack->ReleaseDC( opHdc );
	}
	
	
	

	lpD3DDevice->BeginScene();
	DrawFade();

	//와이드로 보일때 뿌옇게 보이는 부분 없애기...
	//DrawWide();

	lpD3DDevice->EndScene();
	
	//버전 표시...
	char buffer[256];
	memset(buffer, 0, sizeof(buffer));
	HDC opHdc;
	lpDDSBack->GetDC( &opHdc );
		
	SetBkMode( opHdc, TRANSPARENT );
	SetTextColor( opHdc, RGB(255, 255, 255) );
	if(hFont != NULL)
	{
		HFONT oldFont;
		oldFont = (HFONT)SelectObject( opHdc , hFont );
		
		if ( Client_Version<1000 ) {
			float Version = (float)Client_Version/100;
			sprintf(buffer, "%s%2.2f%s", "Ver: ",Version,"b");
		}
		else {
			int v1 = Client_Version/1000;
			int v2 = (Client_Version/10)%100;
			int v3 = Client_Version%10;
			wsprintf(buffer, "%s%d.%d.%d", "Ver: ",v1,v2,v3 );
		}
     		dsTextLineOut(opHdc, smScreenWidth-80, smScreenHeight-75, buffer, strlen(buffer));
		
		if(oldFont)
			SelectObject(opHdc, oldFont);
	}
	lpDDSBack->ReleaseDC( opHdc );

	if(CurrentWorkState == STATE_LOGIN_INPUT && NextWorkState == STATE_LOGIN_INPUT
		&&ScreenBlendValue <= 100)
	{
		//Login 계정과 암호 입력...
		if(LoginBlendValue >= 255)
		{
			if(FocusWndFlag == FALSE)
			{
				FocusWndFlag = TRUE;
				hFocusWnd = hTextWnd;
			}
			
			if(StartConnectServerFlag == FALSE && ConnectError == ERROR_NONE)
			{
				if(pCursorPos.x > (startLogoX+startAccountAddX+10) && 
				   pCursorPos.x < (startLogoX+startAccountAddX+10+95) && 
				   pCursorPos.y > (startLogoY+startAccountAddY+20) && 
				   pCursorPos.y < (startLogoY+startAccountAddY+23+14) && 
 				   MouseButton[0])
				{
					if(LoginInputState == 1)
					{
						LoginInputState = 0;
						SetWindowText(hFocusWnd, UserAccount);
						SendMessage(hTextWnd, EM_SETSEL, lstrlen(UserAccount), lstrlen(UserAccount)+1);
					}
				}
			
				if(pCursorPos.x > (startLogoX+startAccountAddX+10) && 
				   pCursorPos.x < (startLogoX+startAccountAddX+10+95) && 
				   pCursorPos.y > (startLogoY+startAccountAddY+70) && 
				   pCursorPos.y < (startLogoY+startAccountAddY+73+14) && 
				   MouseButton[0])
				{
					if(LoginInputState == 0)
					{
						SetIME_Mode(0);
						LoginInputState = 1;
						SetWindowText(hFocusWnd, UserPassword);
						SendMessage(hTextWnd, EM_SETSEL, lstrlen(UserPassword), lstrlen(UserPassword)+1);
					}
				}
			}
			
			//입력할수 있는 글자의 최대 갯수는 영문 갯수로 15
			//SendMessage(hTextWnd, EM_SETLIMITTEXT, DISP_LOGIN_LEN, 0);
  			if(hFocusWnd && ConnectError == ERROR_NONE && StartConnectServerFlag==FALSE)
			{
				if(LoginInputState == 0) //Account 입력 상태..
				{
					GetWindowText(hFocusWnd, UserAccount, sizeof(UserAccount));
					KeyFilter(UserAccount);
					
				}	
				else if(LoginInputState == 1) //Password 입력 상태..
				{
					GetWindowText(hFocusWnd, UserPassword, sizeof(UserPassword));
					KeyFilter(UserPassword);
				}
			}
			
			if(hFocusWnd)
				SendMessage(hFocusWnd, EM_GETSEL, NULL, (LPARAM)&End);
			
			HDC opHdc;
			HFONT oldFont;
			lpDDSBack->GetDC( &opHdc );
			
			SetBkMode( opHdc, TRANSPARENT );
			SetTextColor( opHdc, RGB(255, 255, 255) );
			if(hFont != NULL)
				oldFont = (HFONT)SelectObject( opHdc , hFont );
			
 			if(hFocusWnd)
			{
  				if( CaretCount < 16)
				{
 					if ( End>DISP_LOGIN_LEN ) End=DISP_LOGIN_LEN;

        				if(LoginInputState == 0)//Account 입력시..
						dsTextLineOut(opHdc, startLogoX+startAccountAddX+11+End*6+1,startLogoY+startAccountAddY+23, "|", 1);
					else if(LoginInputState == 1)//Password 입력시..
						dsTextLineOut(opHdc, startLogoX+startAccountAddX+11+End*6+1,startLogoY+startAccountAddY+73, "|", 1);
				}

				CaretCount++;
				if(CaretCount > 32)
					CaretCount = 0;
			}			
			
			int sp,len;
			
			sp = 0;
  			len = lstrlen(UserAccount);
			if ( len>DISP_LOGIN_LEN ) {
 				sp = len-DISP_LOGIN_LEN;
				len -= sp;
			}
   			dsTextLineOut(opHdc , startLogoX+startAccountAddX+12, startLogoY+startAccountAddY+23, UserAccount+sp, len);
			len = lstrlen(UserPassword);
			if ( len>DISP_LOGIN_LEN ) len=DISP_LOGIN_LEN;

			for(int count = 0; count < len; count++)
				dsTextLineOut(opHdc, startLogoX+startAccountAddX+12+count*6, startLogoY+startAccountAddY+73, "*", 1);
			
			if(oldFont)
			SelectObject(opHdc, oldFont);
			lpDDSBack->ReleaseDC( opHdc );
		}
	}
	
	if(StartConnectServerFlag && ConnectError == ERROR_NONE)
		DrawMessage(295+m_AddWidth, 300+m_AddHeight, HoTextLoginMessage[0]);//"      서버에 접속중....    ");
	
 	if(ConnectError == ERROR_CONNECT)
	{
		StartConnectServerFlag = FALSE;
 		DrawMessage(295+m_AddWidth, 300+m_AddHeight, HoTextLoginMessage[1]);//"      서버 접속 불가       ");
	}
	else if(ConnectError == ERROR_NO_ACCOUNT)
		DrawMessage(295+m_AddWidth, 300+m_AddHeight, HoTextLoginMessage[2]);//"   계정을 다시 입력하세요  ");
	else if(ConnectError == ERROR_NO_PASSWORD)
		DrawMessage(295+m_AddWidth, 300+m_AddHeight, HoTextLoginMessage[3]);//"비밀 번호를 다시 입력하세요");
   	else if(ConnectError == ERROR_NO_BETA_TESTER)
		DrawMessage(205+m_AddWidth, 300+m_AddHeight, HoTextLoginMessage[4]);//"  해당 아이디는 현재 사용권한이 없으니 고객센터로 문의 바랍니다.  ");
	else if(ConnectError == ERROR_NO_ACCOUNT_IN)
		DrawMessage(295+m_AddWidth, 300+m_AddHeight, HoTextLoginMessage[5]);//"   아이디가 접속중입니다   ");
	else if(ConnectError == ERROR_NO_VERSION)
		DrawMessage(295+m_AddWidth, 300+m_AddHeight, HoTextLoginMessage[6]);//"  Version이 맞지 않습니다  ");
	else if(ConnectError == ERROR_NO_LIMIT)
		DrawMessage(295+m_AddWidth, 300+m_AddHeight, HoTextLoginMessage[7]);//"   서버에 인원이 많습니다  ");
	else if(ConnectError == ERROR_NO_SERVER_CONNECT)
		DrawMessage(275+m_AddWidth, 300+m_AddHeight, HoTextLoginMessage[8]);//"서버와의 연결이 끊어 졌습니다");
	else if(ConnectError == ERROR_NO_USE_DATE_OVER)
		DrawMessage(295+m_AddWidth, 300+m_AddHeight, HoTextLoginMessage[9]);//"  사용기간이 종료되었습니다 "
	else if(ConnectError == ERROR_NO_USE_TIME_OVER)
		DrawMessage(295+m_AddWidth, 300+m_AddHeight, HoTextLoginMessage[10]);   //"  사용시간이 종료되었습니다 "
	else if(ConnectError == ERROR_DELETE_ACCOUNT_NOTIFY) //" 서버계정 삭제 요청중입니다 "
		DrawMessage(295+m_AddWidth, 300+m_AddHeight, HoTextLoginMessage[11]);
	else if(ConnectError == ERROR_SERVER_TRAFFIC_BUSY)   //서버폭주!!! 잠시후 이용 바랍니다.
     	DrawMessage(275+m_AddWidth, 300+m_AddHeight, HoTextLoginMessage[12]);
	else if(ConnectError == ERROR_SERVER_WAIT_MOMNENT)   //잠시후 이용해 주십시요
		DrawMessage(295+m_AddWidth, 300+m_AddHeight, HoTextLoginMessage[13]);
	else if(ConnectError == ERROR_THIRD_CONNECT)         //비밀번호 3회 이상 오류입니다. 홈페이지 참조 바랍니다.
     	DrawMessage(285+m_AddWidth, 300+m_AddHeight, HoTextLoginMessage[14], 30);
	else if(ConnectError == ERROR_HAN_PASSWORD)          //Password에 한글문자나 사용할수 없는 문자가 있습니다.
		DrawMessage(285+m_AddWidth, 300+m_AddHeight, HoTextLoginMessage[15], 30);
	else if(ConnectError == ERROR_12YEARS)				//12세 이하 입장 불가
		DrawMessage(285+m_AddWidth, 300+m_AddHeight, HoTextLoginMessage[16], 30);
	else if(ConnectError == ERROR_HOMEAGREE)			//12세 이하 입장 불가
		DrawMessage(285+m_AddWidth, 300+m_AddHeight, HoTextLoginMessage[17], 30);	
	else if(ConnectError == ERROR_UNTIL_LOGINDENIED)								//일정기간 로그인정지
		DrawMessage(295+m_AddWidth, 300+m_AddHeight, HoTextLoginMessage[18], 30);	//_LANGUAGE_JAPANESE (-23)
	else if(ConnectError == ERROR_LOGIN_DENIED)			//로긴정지					//로그인정지
		DrawMessage(295+m_AddWidth, 300+m_AddHeight, HoTextLoginMessage[19], 30);	//_LANGUAGE_JAPANESE (-24)


	return TRUE;
}


HANDLE	hHoLoginThread =0;

DWORD WINAPI HoLoginThreadProc( void *pInfo )
{
	HoOpening	*lpHoOpening = (HoOpening *)pInfo;
	
	//서버와의 접속을 시도...
	if(!smWsockDataServer) {
		ConnectServer_InfoMain();
	
		if(smWsockDataServer)
		{
			ServerUserCharacterInfo = NULL;
			DownServerListInfo = NULL;
			ServerLoginError = NULL;
			lpHoOpening->ServerWaitTimeCount = 0;
			
			if(HoCommandLineFlag)
			{
				memset(UserAccount, 0, sizeof(UserAccount));
				memset(UserPassword, 0, sizeof(UserPassword));

				memcpy(UserAccount,  HoCommandInfo.Id,   strlen(HoCommandInfo.Id));
				memcpy(UserPassword, HoCommandInfo.Pass, strlen(HoCommandInfo.Pass));
			}
			TransUserCommand(smTRANSCODE_ID_GETUSERINFO, UserAccount,UserPassword); //User의 정보를 알아 오기...
		}
		else {
			lpHoOpening->ConnectError = ERROR_CONNECT;
			lpHoOpening->StartConnectServerFlag = FALSE;
		}
	}

	hHoLoginThread = 0;

	ExitThread( TRUE );
	return TRUE;
}

int HoOpening::LoginDataServer()
{
	DWORD dwHoLoginThreadId;

	if ( !hHoLoginThread ) {
		hHoLoginThread = CreateThread( NULL , 0, HoLoginThreadProc , this , 0, &dwHoLoginThreadId );

		PlayWav2(13-1);
		StartConnectServerFlag = TRUE;
		return TRUE;
	}
	return FALSE;
}

int HoOpening::LoginGameServer(int serverIndex)
{
	
	PlayWav2(13-1);
	DisconnectServer_GameMain();
	
	//이부분이 받은 서버로 접속 하는 부분이다...
	if(ConnectServer_GameMain( UserServerListInfo[serverIndex].szServerIp1, 
							   UserServerListInfo[serverIndex].dwServerPort1, 
							   UserServerListInfo[serverIndex].szServerIp2, 
							   UserServerListInfo[serverIndex].dwServerPort2,
							   UserServerListInfo[serverIndex].szServerIp3,
							   UserServerListInfo[serverIndex].dwServerPort3))
	{
		StartConnectServerFlag = TRUE;
	}
	else
		ConnectError = ERROR_CONNECT;
	
	return TRUE;
}

//KeyFilter를 한다...
int HoOpening::KeyFilter(char *message)
{
	if(message == NULL || hFocusWnd == NULL)
		return FALSE;

	//if(lstrlen(message) >= 17 && message[16] != '\t') {
	//	message[16] = 0;
	if(lstrlen(message) >= 31 && message[30] != '\t')
	{
		message[30] = 0;
		SetWindowText(hFocusWnd, 0);
		SetWindowText(hFocusWnd, message);
		SendMessage(hFocusWnd, EM_SETSEL, lstrlen(message)+1, lstrlen(message)+1);
	}
	
	/*
	//Edit에서 Left를 안먹게 한다.
	int leftCaret;
	SendMessage(hFocusWnd, EM_GETSEL, (WPARAM)&leftCaret, NULL);
	if(LoginInputState == 0)
	{
		if(leftCaret < (int)lstrlen(UserAccount))
			SendMessage(hFocusWnd, EM_SETSEL, lstrlen(UserAccount), lstrlen(UserAccount)+1);
	}
	else if(LoginInputState == 1)
	{
		if(leftCaret < (int)lstrlen(UserPassword))
			SendMessage(hFocusWnd, EM_SETSEL, lstrlen(UserPassword), lstrlen(UserPassword)+1);
	}
	*/

	//안쓰는 문자는 체크한다.(Filter)
	for(int index = 0; index < (int)strlen(message); index++)
	{
		if(message[index] =='\t') //Tab키를 눌렀을 경우.
		{
			message[index] = 0;
			SetWindowText(hFocusWnd, 0);
			
			if(LoginInputState == 0)
			{
				SetIME_Mode(0);
				LoginInputState = 1;
				SetWindowText(hFocusWnd, UserPassword);
				SendMessage(hTextWnd, EM_SETSEL, lstrlen(UserPassword)+1, lstrlen(UserPassword)+1);
			}
			else if(LoginInputState == 1)
			{
				LoginInputState = 0;
				SetWindowText(hFocusWnd, UserAccount);
				SendMessage(hTextWnd, EM_SETSEL, lstrlen(UserAccount)+1, lstrlen(UserAccount)+1);
			}
		}
		
		
		if(message[index] == ' ')
		{
			message[index] = 0;
			SetWindowText(hFocusWnd, message);
			SendMessage(hTextWnd, EM_SETSEL, lstrlen(message)+1, lstrlen(message)+1);
			break;
		}
		
/*
		if(LoginInputState == 1) //암호 입력중..
		{
			//영문자하고 숫자만 받는다.
			if( !((message[index] >= 0x61 && message[index] <= 0x7a) || 
				(message[index]>= 0x41 && message[index] <= 0x5a)  || 
				(message[index] >= 0x30 && message[index] < 0x31+10)))
			{
				
				message[index] = 0;
				SetWindowText(hFocusWnd, 0);
				SetWindowText(hFocusWnd, UserPassword);
				SendMessage(hTextWnd, EM_SETSEL, lstrlen(UserPassword)+1, lstrlen(UserPassword)+1);
			}
		}
*/
	}
	return TRUE;
};

//서서히 밝아짐...
int HoOpening::MainFadeIn()
{
	if(FadeNextState == FADE_IN)
	{
		ScreenBlendValue -= 2;
		if(ScreenBlendValue < 0)
			ScreenBlendValue = 0;
		
		if(ScreenBlendValue <= 0 )
		{
			ScreenBlendValue = 0;
			FadeCurrentState = FADE_IN;
			FadeNextState = FADE_IN;
		}
	}
	return TRUE;
}

//서서히 어두워짐...
int HoOpening::MainFadeOut()
{
	if(FadeNextState == FADE_OUT)
	{
		if(ScreenBlendValue < 255)
		{
			OpeningBgmVolume -= 2;
			if(OpeningBgmVolume < 0)
				OpeningBgmVolume = 0;
			ScreenBlendValue += 3;
		}

		if(ScreenBlendValue >= 255)
		{
			ScreenBlendValue = 255;
			LogoBlendValue = 255;
			LoginBlendValue = 255;
			FadeCurrentState = FADE_OUT;
			FadeNextState = FADE_OUT;
		}
	}
	return TRUE;
}

//Cinema 초기화
void HoOpening::InitCinemaFadeEffect()
{
	CinemaFadeOutFlag = FALSE;
	CinemaBlendValue = 0;
	CinemaFadeState = STATE_FADE_NONE;
}

//Cinema 서서히 어두워짐
void HoOpening::MainCinemaFadeEffect()
{
	if(CinemaFadeState == STATE_FADE_IN)
	{
		CinemaBlendValue += 5;
		if(CinemaBlendValue >= 255)
			CinemaBlendValue = 255;
	}

	if(CinemaFadeState == STATE_FADE_IN && CinemaBlendValue >= 255)
		CinemaFadeState = STATE_FADE_OUT;

	if(CinemaFadeState == STATE_FADE_OUT)
	{
		CinemaBlendValue -= 5;
		if(CinemaBlendValue <= 0)
			CinemaBlendValue = 0;
	}
}


//Cinema 서서히 어두워지는거 그리기
void HoOpening::DrawCinemaFadeEffect()
{
	if(CinemaFadeState != STATE_FADE_NONE)
	{
		if(CinemaBlendValue >= 0 && CinemaBlendValue <= 255 )
		{
			lpD3DDevice->BeginScene();
			dsDrawTexImage(WhiteBoxImage,0, 0, smScreenWidth, smScreenHeight, CinemaBlendValue);
			lpD3DDevice->EndScene();
		}
	}
}


int HoOpening::FastFade()
{
	if( (FadeNextState == FADE_IN && LoginBlendValue < 255) ||
		(FadeNextState == FADE_OUT && LoginBlendValue > 0) )
	{

		static bool fastBlendFlag = false;
		for(int index = 0; index < 256; index++)
		{
			if(VRKeyBuff[index] == TRUE)
			{
				fastBlendFlag = true;
				break;
			}
		}
		
		if(MouseDownFlag == FALSE)
		{
 			if(MouseButton[0] && fastBlendFlag == false)
			{
 				fastBlendFlag = true;
				MouseDownFlag = TRUE;
			}
		}

		if(fastBlendFlag)
		{
			if(FadeNextState == FADE_IN) //환해진다.
			{
				if( (CurrentWorkState == STATE_OPENING &&
				    NextWorkState == STATE_OPENING) ||
				   (CurrentWorkState == STATE_OPENING && NextWorkState == STATE_LOGIN_INPUT) )
				{
					LogoBlendValue = 255;
					ScreenBlendValue = 0;
					FadeCurrentState = FADE_IN;
					
					if(HoCommandLineFlag == FALSE)
					{
						LoginBlendValue = 255;
						CurrentWorkState = STATE_LOGIN_INPUT;
						NextWorkState = STATE_LOGIN_INPUT;
					}

					if(HoCommandLineFlag)
						NextWorkState = STATE_LOGIN_INPUT;
					//CinemaFadeOutFlag = TRUE;
				}				
			}
			else if(FadeNextState == FADE_OUT)//어두워 진다.
			{
//				StartLogoLoginFlag = FALSE;	
				LogoBlendValue = 0;
				LoginBlendValue = 0;
				ScreenBlendValue = 255;
				OpeningBgmVolume = 0;
				FadeCurrentState = FADE_OUT;
				
			}
			fastBlendFlag = false;
		}
	}
	return TRUE;
}

int HoOpening::DrawFade()
{
	if(ScreenBlendValue >= 0 && ScreenBlendValue <= 255)
		dsDrawTexImage(BlackBoxImage,0, 0, smScreenWidth, smScreenHeight, ScreenBlendValue);

	return TRUE;
}

int HoOpening::MainFade()
{
	if( (FadeNextState == FADE_IN) ||
		(FadeNextState == FADE_OUT) )
	{
		if(FadeCurrentState != FadeNextState)
		{
			if(FadeNextState == FADE_IN)
				MainFadeIn();
			else
			{
				SetVolumeBGM(OpeningBgmVolume);
				MainFadeOut();
			}
		}
	}
	return TRUE;
}

void HoOpening::SetFade()
{
	//Fade In을 해야 할경우....(서서희 밝아짐)

	if(FadeCurrentState == FADE_OUT)
	{
		FadeNextState = FADE_IN;
		ScreenBlendValue = 254;
		OpeningBgmVolume = 350;
	}
	else//Fade Out을 해야 할 경우...(어두워짐)
	{
		FadeNextState = FADE_OUT;
		ScreenBlendValue = 1;
	}
}


void HoOpening::Close()
{
	if(Stage)
	{
		delete Stage;
		Stage = NULL;

		//######################################################################################
		//작 성 자 : 오 영 석		
		lpCurPlayer->pX = lpCurPlayer->pY = lpCurPlayer->pZ = 0;
		smGameStage[0] = NULL;
		smGameStage[1] = NULL;
		//######################################################################################
	}

	if(PatCamera)
	{
		delete PatCamera;
		PatCamera = NULL;
	}
	
	BackColor_R = 0;
	BackColor_G = 0;
	BackColor_B = 0;

	BackColor_mR = 0;
	BackColor_mG = 0;
	BackColor_mB = 0;
	
	InitSky();

	/*
	if(Bird)
	{
		delete Bird;
		Bird = NULL;
	}

	if(BirdBone)
	{
		delete BirdBone;
		BirdBone = NULL;
	}
	*/

	//InitSky();
	//smRender.Color_R = 40;
	//smRender.Color_G = 0;
	//smRender.Color_B = -30;
	//smRender.Color_A = 0;
}



//static BOOL HanGameErrorFlag = FALSE;
//return 1: 현재 Login진행시.
//return 2: Loing 성공시..
int HoOpening::Main()
{
	if(MouseButton[0] == FALSE)
	{
		MouseDownFlag = FALSE;
	}

	//######################################################################################
	//작 성 자 : 오 영 석
	float timeDelta = 1.f/70.f;
	g_NewParticleMgr.Main(0.0f, timeDelta);

	MainSky();
	m_IsDrawOpening = s_AutoCamera.FrameMain( m_AutoCameraFrameTime );
	if( m_IsDrawOpening > 1 && CinemaFadeOutFlag == FALSE)
	{
		CinemaFadeState = STATE_FADE_IN;
		CinemaFadeOutFlag = TRUE;
	}
	
	
	SetCameraCharPos();
	if( m_IsDrawOpening == 1 )
       SetParticleEffectSpawn();
	
	MainAssaEffect();
	MainCinemaFadeEffect();

	if(WaveCameraFlag)
	{
		WaveCameraTimeCount++;
			
		if((WaveCameraTimeCount>WaveCameraDelay))
		{
			WaveCameraTimeCount = 0;
			if(WaveCameraDelay > 1 && WaveCameraFactor < 40)
                		WaveCameraFactor = -int((float)WaveCameraFactor/10.f*9.f);
			else
				WaveCameraFactor = -int((float)WaveCameraFactor/10.f*8.f);
		}

  		if(abs(WaveCameraFactor) < 1)
		{
			WaveCameraFlag = FALSE;
			WaveCameraTimeCount = 0;
		}
	}
	//######################################################################################


	//에러 메시지 출력시.. 없애는 경우...
	if(ConnectError != ERROR_NONE)
	{
		if(ConnectErrorCount >= 160)
		{
			MouseDownServerIndex = -1;
			bool errorEndFlag = false;
			//마우스를 클릭 했을 경우.
			if(MouseButton[0] || MouseDblClick)
				errorEndFlag = true;
			
			//키보드를 눌렀을 경우.
			for(int index = 0; index < 256; index++)
			{
				if(VRKeyBuff[index] == TRUE)
				{
					errorEndFlag = true;
					break;
				}
			}
			//시간을 초과 했을 경우.
			if(ConnectErrorCount >= 250)
				errorEndFlag = true;
			
			if(errorEndFlag)
			{
				ConnectError = ERROR_NONE;
				FocusWndFlag = FALSE;
				ConnectErrorCount = 0;
			}
			
			//한게임 에러 처리...
			if( errorEndFlag && HoCommandLineFlag
				&& CurrentWorkState == STATE_LOGIN_INPUT
				&& NextWorkState    == STATE_LOGIN_INPUT)
			{
//				HanGameErrorFlag		= TRUE;
				HoCommandLineFlag		= FALSE;
				NextWorkState			= STATE_LOGIN_INPUT;
				CurrentWorkState		= STATE_OPENING;
				ConnectServerSucessFlag = FALSE;
				DisconnectServerFull();
			}
		}
		ConnectErrorCount++;
	}
	
	

	MainFade();
	
	//지형
 	if(CurrentWorkState == STATE_OPENING && NextWorkState == STATE_OPENING )
	{
		if(CinemaFadeState == STATE_FADE_OUT)
			NextWorkState = STATE_LOGIN_INPUT;
		//if(CameraFrame > 250*160) //Opening이 어느 정도 진행이 되면 작업 상태 변경.
			//NextWorkState = STATE_LOGIN_INPUT;
	}
	
	//Opening 상태에서 Login 입력 상태로 변화시...
	if(CurrentWorkState == STATE_OPENING && NextWorkState == STATE_LOGIN_INPUT)
	{
		if(FadeNextState == FADE_IN) //FADE_IN이 다됐을 경우만...
		{
			LogoBlendValue += 1;
			if(LogoBlendValue >= 255)
				LogoBlendValue = 255;

			if(LogoBlendValue >= 255)
			{
				//한게임 로긴 아이디를 성공적으로 받았을 경우..
				if(HoCommandLineFlag)
				{
					CurrentWorkState = STATE_LOGIN_INPUT;
					LoginDataServer();
				}
				else//아닌 경우 로긴 창을 보여준다.
				{
					LoginBlendValue += 1;
					if(LoginBlendValue >= 255)
					{
						LoginBlendValue = 255;
						CurrentWorkState = STATE_LOGIN_INPUT;
					}
				}
			}
		}
	}
	
	//바뀐곳
	//int startLogoX = 100;
	//int startLogoY = 63;
	//int startLogoAddX = 32;
	//int startAccountAddX = 230;
	//int startAccountAddY = 290;

	//로긴 입력을 할 경우.
	if( CurrentWorkState == STATE_LOGIN_INPUT
		&& NextWorkState == STATE_LOGIN_INPUT
		&& (LoginBlendValue >= 255)
		&& (ScreenBlendValue <= 0)
		&& StartConnectServerFlag == FALSE
		&& ConnectError == ERROR_NONE)
	{
		//Enter를 눌렀을 경우.
		if( FocusWndFlag && hFocusWnd == NULL 
			&& ConnectServerSucessFlag == FALSE
			&& ConnectError == ERROR_NONE)
		{
			LoginDataServer();
			//CinemaFadeOutFlag = TRUE;
		}
		
		int downFlag = -1;
		
		
		if(MouseDownFlag == FALSE)
		{
			if(MouseButton[0])
			{
				if(pCursorPos.x > (startLogoX+230-30) &&
				   pCursorPos.y > (startLogoY+320+93) && 
				   pCursorPos.x < (startLogoX+230-30+64) && 
  				   pCursorPos.y < (startLogoY+320+93+25) )
				{
					MouseDownFlag = TRUE;
					hFocusWnd = NULL;
				}
				else if(pCursorPos.x > (startLogoX+230+32) && 
					    pCursorPos.y > (startLogoY+320+93) && 
					    pCursorPos.x < (startLogoX+230+32+64) && 
  					    pCursorPos.y < (startLogoY+320+93+25) )
			    {
					MouseDownFlag = TRUE;
					PlayWav2(4-1);
					if(HoCommunity == COMMUNITY_TRIGLOW)
						ShellExecute(NULL, NULL, szJoinWebUrl, NULL, NULL, NULL);
					else if(HoCommunity == COMMUNITY_HANGAME)
						ShellExecute(NULL, NULL, "http://www.hangame.naver.com", NULL, NULL, NULL);
					
					quit = 1;
				}
				else if(pCursorPos.x > (startLogoX+230+100) && 
					    pCursorPos.y > (startLogoY+320+93) && 
		                pCursorPos.x < (startLogoX+230+100+64) && 
		                pCursorPos.y < (startLogoY+320+93+25) )
				{
					MouseDownFlag = TRUE;
					StartQuitFlag = TRUE;
					SetFade();
					PlayWav2(4-1);
				}
			}
		}
	}
	
	//Game Server를 선택하기
	if( CurrentWorkState == STATE_SERVER_SELECT 
		&& NextWorkState == STATE_SERVER_SELECT)
	{
		int selectHeight;
		int selectStartPosX;
		int selectStartPosY;
		
		if(UserServerListInfoCount != 0)
			selectHeight = 90/UserServerListInfoCount;
   		selectStartPosX = 230+startLogoX;
  		selectStartPosY = 315+startLogoY+(selectHeight-12)/2;
		
  		for(int index = 0; index < UserServerListInfoCount; index++)
		{
			if( pCursorPos.x > (selectStartPosX) && (pCursorPos.y > selectStartPosY+index*selectHeight) &&
				pCursorPos.x < (selectStartPosX+140)  && (pCursorPos.y < (selectStartPosY+index*selectHeight+12)) &&
				ConnectError == ERROR_NONE && !StartConnectServerFlag)
			{
				if(MouseButton[0] && MouseDownFlag == FALSE)
				{
 					MouseDownFlag = TRUE;
					MouseDownServerIndex = index;
					LoginGameServer(index);
				}
			}
		}
	}

	if( CurrentWorkState == STATE_SERVER_SELECT
		&& NextWorkState == STATE_SERVER_SELECT)
	{
		BOOL flag=FALSE;
		if(ConnectError == ERROR_NONE && !StartConnectServerFlag && StartQuitFlag == FALSE)
		{
			if(MouseDownFlag == FALSE)
			{
				if(MouseButton[0])
				{
					if(pCursorPos.x > (startLogoX+230+32) && 
					pCursorPos.y > (startLogoY+320+93) && 
					pCursorPos.x < (startLogoX+230+32+64) && 
   					pCursorPos.y < (startLogoY+320+93+25) )
					{
						flag = TRUE;
						MouseDownFlag = TRUE;
					}
				}
			}
		}
		
		//DataServer와의 연결이 끊어 질경우..
		if(!smWsockDataServer && GameErrorValue == 0)
		{
			NextWorkState = STATE_LOGIN_INPUT;
			ConnectServerSucessFlag = FALSE;
			DisconnectServerFull();
			ConnectError = ERROR_NO_SERVER_CONNECT;
		}

		//나가기 버튼을 누른 경우..
		if(flag)
		{
			PlayWav2(4-1);
			//한게임이 아니면.
			if(!HoCommandLineFlag)
			{
				FocusWndFlag = FALSE;
				NextWorkState = STATE_LOGIN_INPUT;
				CurrentWorkState = STATE_LOGIN_INPUT;
				ConnectServerSucessFlag = FALSE;
				DisconnectServerFull();
			}
			else
			{
				DisconnectServerFull();
				StartQuitFlag = TRUE;
				SetFade();
				ConnectServerSucessFlag = FALSE;
				MouseDownFlag = FALSE;
			}
		}
	}
	
	//서버에 연결중이면....
	if(StartConnectServerFlag)
	{
		if(ConnectError != ERROR_NONE)
		{
			StartConnectServerFlag = FALSE;
			ServerWaitTimeCount = 0;
		}
		
		if(ServerLoginError)
		{
			if( CurrentWorkState == STATE_LOGIN_INPUT
				&& NextWorkState == STATE_LOGIN_INPUT)
				DisconnectServerFull();

			if( CurrentWorkState == STATE_SERVER_SELECT
				&& NextWorkState == STATE_SERVER_SELECT)
				DisconnectServer_GameMain();
			
			if(ServerLoginError->WParam == 0) //연결 불가.
				ConnectError = ERROR_CONNECT;
			else if(ServerLoginError->WParam == -1)
				ConnectError = ERROR_NO_ACCOUNT;
			else if(ServerLoginError->WParam == -2)
				ConnectError = ERROR_NO_PASSWORD;
			else if(ServerLoginError->WParam == -3)
				ConnectError = ERROR_NO_BETA_TESTER;
			else if(ServerLoginError->WParam == -4)
				ConnectError = ERROR_NO_ACCOUNT_IN;
			else if(ServerLoginError->WParam == -5)
				ConnectError = ERROR_NO_USE_DATE_OVER;
			else if(ServerLoginError->WParam == -6)
				ConnectError = ERROR_NO_USE_TIME_OVER;
			else if(ServerLoginError->WParam == -8)
				ConnectError = ERROR_DELETE_ACCOUNT_NOTIFY;
			else if(ServerLoginError->WParam == -16)
				ConnectError = ERROR_SERVER_TRAFFIC_BUSY;
			else if(ServerLoginError->WParam == -17)
				ConnectError = ERROR_SERVER_WAIT_MOMNENT;
			else if(ServerLoginError->WParam == -18)
				ConnectError = ERROR_THIRD_CONNECT;
			else if(ServerLoginError->WParam == -19)
				ConnectError = ERROR_HAN_PASSWORD;
			else if(ServerLoginError->WParam == -12)
				ConnectError = ERROR_12YEARS;
			else if(ServerLoginError->WParam == -13)
				ConnectError = ERROR_HOMEAGREE;
			else if(ServerLoginError->WParam == -23)
				ConnectError = ERROR_UNTIL_LOGINDENIED;
			else if(ServerLoginError->WParam == -24)
				ConnectError = ERROR_LOGIN_DENIED;



			StartConnectServerFlag = FALSE;
			ServerWaitTimeCount = 0;
			ServerLoginError = NULL;
		}

 		ServerWaitTimeCount++;
		if(ServerWaitTimeCount >= 3000)	//서버를 기다리는 시간...
		{
			ServerWaitTimeCount = 0;
			ConnectError = ERROR_CONNECT;
			StartConnectServerFlag = FALSE;
			ConnectServerSucessFlag = FALSE;
		}
		
		//Data서버 로그인시..
		if( CurrentWorkState == STATE_LOGIN_INPUT &&
			NextWorkState == STATE_LOGIN_INPUT)
		{
			if( ServerUserCharacterInfo
				&& DownServerListInfo
				&& ConnectError == ERROR_NONE
				&& GameErrorValue == 0)
			{
				#ifdef _XTRAP_GUARD_4_CLIENT
					XTrap_C_SetUserInfo(UserAccount,szConnServerName,NULL,NULL,0);	//XTrapD5
				#endif

				StartConnectServerFlag = FALSE;
				ConnectServerSucessFlag = TRUE;
				CurrentWorkState = STATE_SERVER_SELECT;
				NextWorkState = STATE_SERVER_SELECT;
				GameErrorValue = -1;
			}
		}
		
		//Game서버 로그인시..
		if( CurrentWorkState == STATE_SERVER_SELECT 
			&& NextWorkState == STATE_SERVER_SELECT) 
		{
			//count = 0;
			if( ConnectError == ERROR_NONE && GameErrorValue == 0)
			{
				StartConnectServerFlag = FALSE;
				SetFade();
				NextWorkState = STATE_END;
				GameErrorValue = -1;
			}
		}
		
		if(GameErrorValue != 0)
		{
			//Version이 맞지 않을 경우...
			if(GameErrorValue == 1)
			{
				DisconnectServerFull();
				ConnectError = ERROR_NO_VERSION;
				StartConnectServerFlag = FALSE;
				GameErrorValue = -1;
			}
			
			//서버 인원수 제한이 있을 경우..
			if(GameErrorValue == 2)
			{
				if(CurrentWorkState == STATE_LOGIN_INPUT
				   &&NextWorkState == STATE_LOGIN_INPUT)
					DisconnectServerFull();

				if(CurrentWorkState == STATE_SERVER_SELECT
					&&NextWorkState == STATE_SERVER_SELECT)
					DisconnectServer_GameMain();
				
				ConnectError = ERROR_NO_LIMIT;
				StartConnectServerFlag = FALSE;
				GameErrorValue = -1;
			}
		}
	}
	
	if(ConnectServerSucessFlag)
	{
		if(FadeCurrentState == FADE_OUT && FadeNextState == FADE_OUT)
		{
#ifdef  _LANGUAGE_KOREAN			
			SetRegString( HKEY_LOCAL_MACHINE , "SOFTWARE\\Triglow Pictures\\PristonTale" , "Account","  ");
#endif

#ifndef _LANGUAGE_KOREAN			
			//Login 계정을 레지스트리에 저장한다..
			SetRegString( HKEY_LOCAL_MACHINE , "SOFTWARE\\Triglow Pictures\\PristonTale" , "Account", UserAccount);
#endif

			SetWindowText(hTextWnd, NULL);
			SendMessage( hTextWnd , EM_SETLIMITTEXT , 80, 0 );

			StopBGM();
			return 2;
		}
	}
	
	if(StartQuitFlag)
	{
		if(FadeCurrentState == FADE_OUT && FadeNextState == FADE_OUT)
			quit = 1;
	}

	return 1;	
}


