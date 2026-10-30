
#include "smd3d.h"
#ifdef PT_GFX_GL
#include "../ptgfx/ptgfx.h"			// real-time shadows: PtGfx_ShadowCamera
#endif

#define SM_DIST_NEARZ				16

#define SMFLOAT_DIST_NEARZ			(SM_DIST_NEARZ<<FLOATNS)
//#define SM_DIST_FARZ				64*26
#define SM_DIST_FARZ				64*25
// Faces with see-through textures (leaves, fences) are cut off at this distance
// (ClipRendFace), the rest of the world at the draw distance. The GL build cuts
// them at the draw distance too (ViewPointLen, page setting), so trees keep their
// leaves as far as you can see them.
#ifdef PT_GFX_GL
#define SMFLOAT_DIST_FARZ			( ( ViewPointLen>SM_DIST_FARZ ? ViewPointLen : SM_DIST_FARZ )<<FLOATNS )
#else
#define SMFLOAT_DIST_FARZ			(SM_DIST_FARZ<<FLOATNS)
#endif

#define SM_DIST_TEXCLIP				64*8
#define SMFLOAT_DIST_TEXCLIP		(SM_DIST_TEXCLIP<<FLOATNS)
/*
#define SMSHIFT_PERSPECTIVE_WIDTH	1
//#define SMMULT_PERSPECTIVE_HEIGHT	(int)( ((float)2.7) * fONE )

#define SMMULT_PERSPECTIVE_HEIGHT	(int)( ((float)3.36) * fONE )
*/

#define smCLIP_DETAIL			(fONE*8)
#define smCLIP_DETAIL_SHIFT		(FLOATNS+3)


#define FLOATS_EM				(FLOATNS+3)


//######################################################################################
//작 성 자 : 오 영 석
//static POINT3D	pLight = { -fONE, 1, fONE/2 };
static POINT3D	pLight = { fONE/4 , fONE/2, -fONE };
//######################################################################################


//######################################################################################
//작 성 자 : 오 영 석
//
#define MAX_ITEM2PASS_TEX										10
static  smTEXTUREHANDLE			*s_Item2PassTexHandleBuff[ MAX_ITEM2PASS_TEX ];
static  char					*s_Item2PassTexName[ MAX_ITEM2PASS_TEX ] = 
{
	"mixS_01.bmp", "mixS_02.bmp", "mixS_03.bmp", "mixS_04.bmp", "mixS_05.bmp",
	"mixM_01.bmp", "mixM_02.bmp", "mixM_03.bmp", "mixM_04.bmp", "mixM_05.bmp",
};

//
int CreateItem2PassTexture(void)
{
	char szImagePath[64] = {0,};
	for( int i=0; i < MAX_ITEM2PASS_TEX; i++ )
	{
		wsprintf( szImagePath, "Image\\sinImage\\Items\\DropItem\\%s", s_Item2PassTexName[i] );
        s_Item2PassTexHandleBuff[i] = smTexture.Add( szImagePath );
		if( s_Item2PassTexHandleBuff[i] == NULL )
            return FALSE;
	}

	return TRUE;
}

//
static  int						 s_Item2PassTexFormState;
static  smTEXTUREHANDLE			*s_lpItem2PassTexHandle;

void SetItem2PassTexture( int TexHandleNum, int TexFormState )
{
	if( TexHandleNum < 0 || TexHandleNum >= MAX_ITEM2PASS_TEX || s_Item2PassTexHandleBuff[TexHandleNum] == NULL )
	{
		s_lpItem2PassTexHandle = NULL;
		return;
	}

	s_lpItem2PassTexHandle   = s_Item2PassTexHandleBuff[ TexHandleNum ];
	s_Item2PassTexFormState  = TexFormState;
}
//######################################################################################


//######################################################################################
// 작 성 자 : 오 영 석	
extern DWORD dwPlayTime;
static void SetBlinkColor( short sColors[4], smFCOLOR BlinkCol, int BlinkStep )
{
	int r = int(BlinkCol.r * 255);
	int g = int(BlinkCol.g * 255);
	int b = int(BlinkCol.b * 255);
 
	int BlinkTime = 1 << BlinkStep;
	int BlinkMsk  = BlinkTime - 1;
	int bTime     = dwPlayTime & BlinkMsk;

	if( (dwPlayTime & BlinkTime) == 0 )
		bTime = BlinkMsk - bTime;

	sColors[SMC_R] = (bTime * r) >> BlinkStep;
	sColors[SMC_G] = (bTime * g) >> BlinkStep;
	sColors[SMC_B] = (bTime * b) >> BlinkStep;
	//if( a < 0 )
    //    sColors[SMC_A] = (bTime * a) >> BlinkStep;
	//else if( a > 0 )
	//	sColors[SMC_A] = -a;
	//else
		sColors[SMC_A] = 0;
}

//
static int SetBlinkColorVertex( smRENDVERTEX *rv, short sColors[4], BYTE Color[4], BYTE Specular[4] )
{
	short BlinkColor[4] = {0,};
	::CopyMemory( BlinkColor, sColors, sizeof(BlinkColor) );

	int z = rv->tz >> FLOATNS;
	if( z > ccDistZMin )
	{
		int lev = ( (z - ccDistZMin) << (13 - ShiftMAPCELL_MULT) ) >> 8;
		if( lev >= 256 )
			return FALSE;

		BlinkColor[SMC_R] -= ((sColors[SMC_R] * lev ) >> 8);
		BlinkColor[SMC_G] -= ((sColors[SMC_G] * lev ) >> 8);
		BlinkColor[SMC_B] -= ((sColors[SMC_B] * lev ) >> 8);
		BlinkColor[SMC_A] -= ((sColors[SMC_A] * lev ) >> 8);
	}

	for( int cnt=0; cnt < 4; cnt++ )
	{
		BlinkColor[cnt] += rv->bCol[cnt];
		if( BlinkColor[cnt] > 255 )
		{
			short spe = rv->bSpe[cnt] + (BlinkColor[cnt] - 256);
			Color[cnt] = 255;
			if( spe > 255 )
				Specular[cnt] = 255;
			else
				Specular[cnt] = (BYTE)spe;
		}
		else
		{
			Color[cnt]    = (BYTE)BlinkColor[cnt];
			Specular[cnt] = rv->bSpe[cnt];
		}
	}

	return TRUE;
}
//######################################################################################


//######################################################################################
//작 성 자 : 오 영 석
//
static float s_DxProjection_w;
static float s_DxProjection_h;
static float s_DxProjection_q1;
static float s_DxProjection_q2;
static int   s_CenterX;
static int   s_CenterY;

//
int			 g_IsDxProjectZoomIn;

float		 g_fZoomInAdd_x;
float		 g_fZoomInAdd_y;
float		 g_fZoomInDiv_x;
float		 g_fZoomInDiv_y;

//
void SetDxProjection( float radians, int SizeX, int SizeY, float near_z, float far_z )
{
	s_DxProjection_h  = float(1.f / tan(radians/2.f));
	s_DxProjection_w  = s_DxProjection_h / (float(SizeX) / float(SizeY));
	s_DxProjection_q1 = far_z / (far_z - near_z);
	s_DxProjection_q2 = -near_z * s_DxProjection_q1;

	s_CenterX = SizeX >> 1;
	s_CenterY = SizeY >> 1;

	//
	float fWinSizeOffset_X = float(SizeX) / 800.f;
	float fWinSizeOffset_Y = float(SizeY) / 600.f;

	g_fZoomInAdd_x = (30.f  * fWinSizeOffset_X);
	g_fZoomInAdd_y = (337.f * fWinSizeOffset_Y);
	g_fZoomInDiv_x = (float(SizeX) / (320.f * fWinSizeOffset_X));
	g_fZoomInDiv_y = (float(SizeY) / (240.f * fWinSizeOffset_Y));
}

#ifdef PT_GFX_GL
// The camera smRender draws with now and its projection, for what ptgfx draws in
// the world itself (weather.cpp: rain and snow).
void smRender_GetWeatherCamera( PtWeatherScene *s )
{
	const float k = 1.0f / (float)(1<<FLOATS_EM);
	const smMATRIX &m = smRender.mCamera;
	float r[9] = { m._11*k, m._12*k, m._13*k, m._21*k, m._22*k, m._23*k, m._31*k, m._32*k, m._33*k };
	memcpy( s->Rot, r, sizeof(r) );
	s->Cam[0] = (double)smRender.CameraPosiX/fONE;
	s->Cam[1] = (double)smRender.CameraPosiY/fONE;
	s->Cam[2] = (double)smRender.CameraPosiZ/fONE;
	s->ProjW = s_DxProjection_w;
	s->ProjH = s_DxProjection_h;
	s->CenterX = (float)s_CenterX;
	s->CenterY = (float)s_CenterY;
	s->DepthQ1 = s_DxProjection_q1;
	s->DepthQ2 = s_DxProjection_q2;
	// faces are clipped where |y| * SMMULT_PERSPECTIVE_HEIGHT > z (the login screen's wide view)
	s->ClipHalfH = smRender.SMMULT_PERSPECTIVE_HEIGHT>0 ? s_DxProjection_h*fONE/smRender.SMMULT_PERSPECTIVE_HEIGHT*s_CenterY : 0;
}
#endif

//
void DxProjectionWork( smRENDVERTEX *lpRendVer )
{
	float fx, fy, fz;
	fx = float(lpRendVer->tx) / fONE;
	fy = float(lpRendVer->ty) / fONE;
	fz = float(lpRendVer->tz) / fONE;

	float fx_p, fy_p, fz_p,  fw_p;
    fx_p = s_DxProjection_w * fx;
    fy_p = s_DxProjection_h * fy;
    fz_p = (s_DxProjection_q1 * fz) + s_DxProjection_q2;
    fw_p = fz;

	lpRendVer->xy[0]  = ( 1.0f + (fx_p / fw_p) ) * s_CenterX;
	lpRendVer->xy[1]  = ( 1.0f - (fy_p / fw_p) ) * s_CenterY;
	lpRendVer->zb     = fz_p / fw_p;
	lpRendVer->rhw    =  1   / fw_p;

	if( g_IsDxProjectZoomIn )
	{
		//lpRendVer->xy[0] /= 2.5f;
		//lpRendVer->xy[1] /= 2.5f;
		//lpRendVer->xy[0] += 30;
        //lpRendVer->xy[1] += 337;

		if( lpRendVer->xy[0] < 0 )
			lpRendVer->xy[0] = 0;
		else if( lpRendVer->xy[0] > (s_CenterX << 1) )
			lpRendVer->xy[0] = float(s_CenterX << 1);

		if( lpRendVer->xy[1] < 0 )
			lpRendVer->xy[1] = 0;
		else if( lpRendVer->xy[1] > (s_CenterY << 1) )
			lpRendVer->xy[1] = float(s_CenterY << 1);

		lpRendVer->xy[0] /= g_fZoomInDiv_x;
		lpRendVer->xy[1] /= g_fZoomInDiv_y;
		lpRendVer->xy[0] += g_fZoomInAdd_x;
        lpRendVer->xy[1] += g_fZoomInAdd_y;
	}
	else
	{
		lpRendVer->xy[0] -= (s_CenterX - MidX);
		lpRendVer->xy[1] -= (s_CenterY - MidY);
	}
}
//######################################################################################


//렌더링 클래스 
smRENDER3D	smRender;

D3DVALUE	RendBuff[ 64*16384 ];

DWORD D3DFVF_TEXTURE[9] = {
	D3DFVF_TLVERTEX0 , 
	D3DFVF_TLVERTEX1 , 
	D3DFVF_TLVERTEX2 , 
	D3DFVF_TLVERTEX3 , 
	D3DFVF_TLVERTEX4 , 
	D3DFVF_TLVERTEX5 , 
	D3DFVF_TLVERTEX6 , 
	D3DFVF_TLVERTEX7 , 
	D3DFVF_TLVERTEX8
};

DWORD D3DTLVERTEX_SIZE[9] = {
	sizeof( D3DFVF_TLVERTEX0 ),
	sizeof( D3DFVF_TLVERTEX1 ),
	sizeof( D3DFVF_TLVERTEX2 ),
	sizeof( D3DFVF_TLVERTEX3 ),
	sizeof( D3DFVF_TLVERTEX4 ),
	sizeof( D3DFVF_TLVERTEX5 ),
	sizeof( D3DFVF_TLVERTEX6 ),
	sizeof( D3DFVF_TLVERTEX7 ),
	sizeof( D3DFVF_TLVERTEX8 )
};

smRENDER3D::smRENDER3D()
{
	//######################################################################################
	//작 성 자 : 오 영 석
	m_dwRenderType = 0;
	//######################################################################################

	//######################################################################################
	//작 성 자 : 오 영 석
	m_FogIsRend  = FALSE;
	m_FogMode    = FALSE;
	m_fFogStNum  = 400.f;
	m_fFogEndNum = 3000.f;
	m_dwFogColor = 0xC0C0C0C0;
	//######################################################################################

	//######################################################################################
	//작 성 자 : 오 영 석
	nRendObjLight = 0;
	//######################################################################################

	CameraPosiMask = FALSE;
	nRendLight = 0;

	SMSHIFT_PERSPECTIVE_WIDTH = RENDCLIP_DEFAULT_SHIFT_PERSPECTIVE_WIDTH;
	SMMULT_PERSPECTIVE_HEIGHT = RENDCLIP_DEFAULT_MULT_PERSPECTIVE_HEIGHT;

	ObjectVertexScale = 0;
	DistortionMesh = 0;			//메쉬 왜곡 모드

	//######################################################################################
	//작 성 자 : 오 영 석	
	m_GameFieldView     = 0;
	m_GameFieldViewStep = 0;

	RendVertex = NULL;
	RendFace   = NULL;
	//######################################################################################

}

smRENDER3D::~smRENDER3D()
{
	//######################################################################################
	//작 성 자 : 오 영 석	
	if( RendVertex )
	{
		delete [] RendVertex;
		RendVertex = NULL;
	}

	if( RendFace )
	{
		delete [] RendFace;
		RendFace = NULL;
	}
	//######################################################################################

}


//######################################################################################
//작 성 자 : 오 영 석
// Size of RendVertex / RendFace (AddStageFace stops short of it: nothing else checks).
// The web build has a draw distance setting up to ~3x the default view, so its
// buffers are bigger.
static int s_RendVertexCap = 0, s_RendFaceCap = 0;
#define WEB_REND_MAX		160000
#define REND_ROOM			8192			// left for the faces/vertices clipping adds

void smRENDER3D::CreateRenderBuff( int FieldViewType )
{
	//
	if( RendVertex )
	{
		delete [] RendVertex;
		RendVertex = NULL;
	}

	if( RendFace )
	{
		delete [] RendFace;
		RendFace = NULL;
	}

	//
	m_GameFieldView = FieldViewType;
#ifdef PT_WEB
	RendVertex = new smRENDVERTEX[ WEB_REND_MAX ];
	::ZeroMemory( RendVertex, sizeof(smRENDVERTEX) * WEB_REND_MAX );
	RendFace   = new smRENDFACE[ WEB_REND_MAX ];
	::ZeroMemory( RendFace, sizeof(smRENDFACE) * WEB_REND_MAX );
	s_RendVertexCap = s_RendFaceCap = WEB_REND_MAX;
	return;
#endif
	if( m_GameFieldView )
	{
		// 게임 필드 확장 시야.
		RendVertex = new smRENDVERTEX[ 40000 ];
		::ZeroMemory( RendVertex, sizeof(smRENDVERTEX) * 40000 );

		RendFace   = new smRENDFACE[ 40000 ];
		::ZeroMemory( RendFace, sizeof(smRENDFACE) * 40000 );
		s_RendVertexCap = s_RendFaceCap = 40000;
	}
	else
	{
		// 기존 게임 필드 시야.
		RendVertex = new smRENDVERTEX[ RENDER_VERTEX_MAX ];
		::ZeroMemory( RendVertex, sizeof(smRENDVERTEX) * RENDER_VERTEX_MAX );

		RendFace   = new smRENDFACE[ RENDER_FACE_MAX ];
		::ZeroMemory( RendFace, sizeof(smRENDFACE) * RENDER_FACE_MAX );
		s_RendVertexCap = RENDER_VERTEX_MAX;
		s_RendFaceCap = RENDER_FACE_MAX;
	}
}


void smRENDER3D::SetGameFieldViewStep(void)
{
	if( m_GameFieldView == 0 )
		return;

	ViewPointLen = (38 + m_GameFieldViewStep) * SizeMAPCELL;
	ccDistZMin   = ViewPointLen - (20 + (m_GameFieldViewStep/4)) * SizeMAPCELL;
}
//######################################################################################


// 좌표를 2D 영역으로 변환하여 돌려준다  
int smRENDER3D::GetRect2D ( int x, int y, int z, int width, int height, RECT *Rect ,POINT *Pos )
{
	//int rx,ry,rz;
	int cx,cy,cz;
	int w,h;
	int cnt;
	int kz;
	POINT3D p[3];

		x -= CameraPosiX;
		y -= CameraPosiY;
		z -= CameraPosiZ;

		//추가하면서 카메라 변환 메트릭스를 곱하여 카메라 좌표를 변환하여 저장

		//######################################################################################
		//작 성 자 : 오 영 석		
		__int64 x_64, y_64, z_64, rx_64, ry_64, rz_64;
		rx_64 = ry_64 = rz_64 = 0;

		x_64 = x;
		y_64 = y;
		z_64 = z;

		rx_64  = ((x_64 * mCamera._11) >> FLOATS_EM);
		rx_64 += ((y_64 * mCamera._21) >> FLOATS_EM);
		rx_64 += ((z_64 * mCamera._31) >> FLOATS_EM);

		ry_64  = ((x_64 * mCamera._12) >> FLOATS_EM);
		ry_64 += ((y_64 * mCamera._22) >> FLOATS_EM);
		ry_64 += ((z_64 * mCamera._32) >> FLOATS_EM);

		rz_64  = ((x_64 * mCamera._13) >> FLOATS_EM);
		rz_64 += ((y_64 * mCamera._23) >> FLOATS_EM);
		rz_64 += ((z_64 * mCamera._33) >> FLOATS_EM);
	
		cx = int(rx_64);
		cy = int(ry_64);
		cz = int(rz_64);
		
		/*
		rx = x * mCamera._11 +
			 y * mCamera._21 +
			 z * mCamera._31 ;

		ry = x * mCamera._12 +
			 y * mCamera._22 +
			 z * mCamera._32 ;

		rz = x * mCamera._13 +
			 y * mCamera._23 +
			 z * mCamera._33 ;

		cx = rx>>FLOATS_EM;
		cy = ry>>FLOATS_EM;
		cz = rz>>FLOATS_EM;
		*/
		//######################################################################################

		w = width>>1;
		h = height>>1;

		p[0].x = cx-w;
		p[1].x = cx+w;
		p[0].y = cy-h;
		p[1].y = cy+h;
		p[0].z = cz;
		p[1].z = cz;

		p[2].x = cx;
		p[2].y = cy;
		p[2].z = cz;
		

		kz = cz;//>>FLOATNS;
		if ( kz<=0 ) return NULL;

		for(cnt=0;cnt<3;cnt++) {

//			p[cnt].x = MidX + ( (p[cnt].x>>FLOATNS) * viewdistZ )/ kz;			// 2D X
//			p[cnt].y = MidY - ( (p[cnt].y>>FLOATNS) * viewdistZ )/ kz;			// 2D Y
			p[cnt].x = MidX + ( p[cnt].x * viewdistZ )/ kz;			// 2D X
			p[cnt].y = MidY - ( p[cnt].y * viewdistZ )/ kz;			// 2D Y

		}

	Rect->top    = p[1].y;
	Rect->bottom = p[0].y;

	Rect->left   = p[0].x;
	Rect->right  = p[1].x;

	if ( Pos ) {
		Pos->x = p[2].x;
		Pos->y = p[2].y;
	}

	return cz;
}

// 카메라 Z좌표를 구해줌  
int smRENDER3D::GetDepthZ ( int x, int y, int z )
{
	int rz;

	x -= CameraPosiX;
	y -= CameraPosiY;
	z -= CameraPosiZ;

	rz = x * mCamera._13 +
		 y * mCamera._23 +
		 z * mCamera._33 ;

	return (rz>>FLOATS_EM);
}



// 렌더링할 카메라 위치를 입력 
int smRENDER3D::SetCameraPosi ( int x, int y, int z, int angX, int angY, int angZ )
{

	smEMATRIX	rX,rY,rZ,tm1;

	//카메라 위치값 저장
	CameraPosiX = x;
	CameraPosiY = y;
	CameraPosiZ = z;

	CameraAngX = angX;
	CameraAngY = angY;
	CameraAngZ = angZ;

	if ( CameraPosiMask ) return FALSE;

	//행렬 초기화
	smEIdentityMatrix( rX );
	smEIdentityMatrix( rY );
	smEIdentityMatrix( rZ );

	//회전 이동 메트릭스 설정
	smERotateXMatrix( rX , GetRadian(-angX) );
	smERotateYMatrix( rY , GetRadian(-angY) );
	smERotateZMatrix( rZ , GetRadian(-angZ) );

	//메트릭스 연산
	smEMatrixMult( tm1 , rY , rX );
	smEMatrixMult( (smEMATRIX &)mCamera , tm1 , rZ );
//	smMatrixShift( mCamera , wSHIFT_FLOAT );
	smMatrixShift( mCamera , 4 );

	//smMatrixMult( tm1 , tm1 , rZ );
	//smMatrixMult( mCamera, mTrans , tm1 );

	return TRUE;
}


// 렌더링할 카메라 위치를 입력 
int smRENDER3D::SetCameraPosi ( int x, int y, int z, smEMATRIX *lpeMatrix )
{

	//카메라 위치값 저장
	CameraPosiX = x;
	CameraPosiY = y;
	CameraPosiZ = z;

	if ( CameraPosiMask ) return FALSE;

	memcpy( (smEMATRIX *)&mCamera , lpeMatrix , sizeof( smEMATRIX ) );
	smMatrixShift( mCamera , 4 );


	return TRUE;
}


//######################################################################################
//작 성 자 : 오 영 석
int	smRENDER3D::AddDynamicLight( int type, int x, int y, int z, int r, int g, int b, int a, int Range )
{
	//######################################################################################
	//작 성 자 : 오 영 석		
	__int64 x_64, y_64, z_64, rx_64, ry_64, rz_64;
	//######################################################################################

	//int rx,ry,rz;
	int dRange;

	if( type & SMLIGHT_TYPE_OBJ )
	{  
		if( nRendObjLight < REND_LIGHT_MAX )
		{
			smRendObjLight[ nRendObjLight ].x = x;
			smRendObjLight[ nRendObjLight ].y = y;
			smRendObjLight[ nRendObjLight ].z = z;

			smRendObjLight[ nRendObjLight ].r = r;
			smRendObjLight[ nRendObjLight ].g = g;
			smRendObjLight[ nRendObjLight ].b = b;
			smRendObjLight[ nRendObjLight ].a = a;

			smRendObjLight[ nRendObjLight ].Range = Range;
			dRange = Range>>FLOATNS;
			smRendObjLight[ nRendObjLight ].dRange = dRange*dRange;

			x -= CameraPosiX;
			y -= CameraPosiY;
			z -= CameraPosiZ;

			//######################################################################################
			//작 성 자 : 오 영 석		
			rx_64 = ry_64 = rz_64 = 0;

			x_64 = x;
			y_64 = y;
			z_64 = z;

			rx_64  = ((x_64 * mCamera._11) >> FLOATS_EM);
			rx_64 += ((y_64 * mCamera._21) >> FLOATS_EM);
			rx_64 += ((z_64 * mCamera._31) >> FLOATS_EM);

			ry_64  = ((x_64 * mCamera._12) >> FLOATS_EM);
			ry_64 += ((y_64 * mCamera._22) >> FLOATS_EM);
			ry_64 += ((z_64 * mCamera._32) >> FLOATS_EM);

			rz_64  = ((x_64 * mCamera._13) >> FLOATS_EM);
			rz_64 += ((y_64 * mCamera._23) >> FLOATS_EM);
			rz_64 += ((z_64 * mCamera._33) >> FLOATS_EM);
		
			x = int(rx_64);
			y = int(ry_64);
			z = int(rz_64);

			/*
			rx = x * mCamera._11 +
				 y * mCamera._21 +
				 z * mCamera._31 ;

			ry = x * mCamera._12 +
				 y * mCamera._22 +
				 z * mCamera._32 ;

			rz = x * mCamera._13 +
				 y * mCamera._23 +
				 z * mCamera._33 ;

			x = rx>>FLOATS_EM;
			y = ry>>FLOATS_EM;
			z = rz>>FLOATS_EM;
			*/
			//######################################################################################

			smRendObjLight[ nRendObjLight ].rx = x ;
			smRendObjLight[ nRendObjLight ].ry = y ;
			smRendObjLight[ nRendObjLight ].rz = z ;

			nRendObjLight++;

			return TRUE;
		}
	}
	else
	{
		if( nRendLight < REND_LIGHT_MAX )
		{
			smRendLight[ nRendLight ].x = x;
			smRendLight[ nRendLight ].y = y;
			smRendLight[ nRendLight ].z = z;

			smRendLight[ nRendLight ].r = r;
			smRendLight[ nRendLight ].g = g;
			smRendLight[ nRendLight ].b = b;
			smRendLight[ nRendLight ].a = a;

			smRendLight[ nRendLight ].Range = Range;
			dRange = Range>>FLOATNS;
			smRendLight[ nRendLight ].dRange = dRange*dRange;

			x -= CameraPosiX;
			y -= CameraPosiY;
			z -= CameraPosiZ;

			//######################################################################################
			//작 성 자 : 오 영 석
			rx_64 = ry_64 = rz_64 = 0;

			x_64 = x;
			y_64 = y;
			z_64 = z;

			rx_64  = ((x_64 * mCamera._11) >> FLOATS_EM);
			rx_64 += ((y_64 * mCamera._21) >> FLOATS_EM);
			rx_64 += ((z_64 * mCamera._31) >> FLOATS_EM);

			ry_64  = ((x_64 * mCamera._12) >> FLOATS_EM);
			ry_64 += ((y_64 * mCamera._22) >> FLOATS_EM);
			ry_64 += ((z_64 * mCamera._32) >> FLOATS_EM);

			rz_64  = ((x_64 * mCamera._13) >> FLOATS_EM);
			rz_64 += ((y_64 * mCamera._23) >> FLOATS_EM);
			rz_64 += ((z_64 * mCamera._33) >> FLOATS_EM);
		
			x = int(rx_64);
			y = int(ry_64);
			z = int(rz_64);

			/*
			rx = x * mCamera._11 +
				y * mCamera._21 +
				z * mCamera._31 ;

			ry = x * mCamera._12 +
				y * mCamera._22 +
				z * mCamera._32 ;

			rz = x * mCamera._13 +
				y * mCamera._23 +
				z * mCamera._33 ;

			x = rx>>FLOATS_EM;
			y = ry>>FLOATS_EM;
			z = rz>>FLOATS_EM;
			*/
			//######################################################################################

			smRendLight[ nRendLight ].rx = x ;
			smRendLight[ nRendLight ].ry = y ;
			smRendLight[ nRendLight ].rz = z ;

			nRendLight++;

			return TRUE;
		}
	}

	return FALSE;
}

// 빛을 모두 제거 한다
int smRENDER3D::ClearObjLight()
{
	bApplyRendObjLight = 0;
	nRendObjLight = 0;
	return TRUE;
}
//######################################################################################


// 렌더링할 빛을 추가 한다
int	smRENDER3D::AddDynamicLight( int x, int y, int z, int r, int g, int b, int a, int Range )
{
	//int rx,ry,rz;
	int dRange;

	if ( nRendLight<REND_LIGHT_MAX ) {
		smRendLight[ nRendLight ].x = x;
		smRendLight[ nRendLight ].y = y;
		smRendLight[ nRendLight ].z = z;

		smRendLight[ nRendLight ].r = r;
		smRendLight[ nRendLight ].g = g;
		smRendLight[ nRendLight ].b = b;
		smRendLight[ nRendLight ].a = a;

		smRendLight[ nRendLight ].Range = Range;
		dRange = Range>>FLOATNS;
		smRendLight[ nRendLight ].dRange = dRange*dRange;

		x -= CameraPosiX;
		y -= CameraPosiY;
		z -= CameraPosiZ;

		//######################################################################################
		//작 성 자 : 오 영 석		
		__int64 x_64, y_64, z_64, rx_64, ry_64, rz_64;
		rx_64 = ry_64 = rz_64 = 0;

		x_64 = x;
		y_64 = y;
		z_64 = z;

		rx_64  = ((x_64 * mCamera._11) >> FLOATS_EM);
		rx_64 += ((y_64 * mCamera._21) >> FLOATS_EM);
		rx_64 += ((z_64 * mCamera._31) >> FLOATS_EM);

		ry_64  = ((x_64 * mCamera._12) >> FLOATS_EM);
		ry_64 += ((y_64 * mCamera._22) >> FLOATS_EM);
		ry_64 += ((z_64 * mCamera._32) >> FLOATS_EM);

		rz_64  = ((x_64 * mCamera._13) >> FLOATS_EM);
		rz_64 += ((y_64 * mCamera._23) >> FLOATS_EM);
		rz_64 += ((z_64 * mCamera._33) >> FLOATS_EM);
	
		x = int(rx_64);
		y = int(ry_64);
		z = int(rz_64);

		/*
		rx = x * mCamera._11 +
			 y * mCamera._21 +
			 z * mCamera._31 ;

		ry = x * mCamera._12 +
			 y * mCamera._22 +
			 z * mCamera._32 ;

		rz = x * mCamera._13 +
			 y * mCamera._23 +
			 z * mCamera._33 ;

		x = rx>>FLOATS_EM;
		y = ry>>FLOATS_EM;
		z = rz>>FLOATS_EM;
		*/
		//######################################################################################

		smRendLight[ nRendLight ].rx = x ;
		smRendLight[ nRendLight ].ry = y ;
		smRendLight[ nRendLight ].rz = z ;

		nRendLight++;

		return TRUE;
	}

	return FALSE;
}

// 빛을 모두 제거 한다
int smRENDER3D::ClearLight()
{
	nRendLight = 0;

	return TRUE;
}


// 렌더링할 카메라 위치를 입력 
int smRENDER3D::OpenCameraPosi ( int x, int y, int z, smEMATRIX *lpeMatrix )
{

	memcpy( (smEMATRIX *)&mCamera , lpeMatrix , sizeof( smEMATRIX ) );
	smMatrixShift( mCamera , 4 );

	//카메라 위치값 저장
	CameraPosiX = x;
	CameraPosiY = y;
	CameraPosiZ = z;

	CameraPosiMask = TRUE;

	return TRUE;
}

// 카메라 위치 닫음
int smRENDER3D::CloseCameraPosi()
{
	CameraPosiMask = FALSE;
	return TRUE;
}



// 텍스쳐 중심 좌표 설정
int smRENDER3D::SetTexPosi( int x, int y, int z , int sx, int sy, int sz )
{
	TexPosiX = x;
	TexPosiY = y;
	TexPosiZ = z;

	TexSizeX = sx;
	TexSizeY = sy;
	TexSizeZ = sz;

	return TRUE;
}


// 임시 메시를 지우고 초기화 함
int smRENDER3D::InitStageMesh ( smSTAGE_VERTEX *StageVertex , DWORD sum )
{
	nTempTexLink	= 0;

	nRendVertex		= 0;							//렌더링 할 버텍스 수
	nRendFace		= 0;							//렌더링 할 페이스 수

	RMatListCnt		= 0;							//메트리얼 관련 카운터 초기화
	RMatListRearCnt = nMaterial-1;					//메트리얼 관련 카운터 초기화
//	RMatListRearCnt = MATRIAL_MAX-1;					//메트리얼 관련 카운터 초기화

	Vertex = StageVertex;							//StageVertex 포인터
	
//	nMaterial		=	MATRIAL_MAX;					//렌더링 할 전체 메트리얼 갯수
//	smRendMaterial	=	smMaterial;


	ClipStates = ( SMCLIP_NEARZ | SMCLIP_FARZ | SMCLIP_LEFT | SMCLIP_RIGHT | SMCLIP_TOP | SMCLIP_BOTTOM );

	SumCount = sum;

	//메트리얼 별 페이스 카운터를 0으로 초기화
	ZeroMemory( RendMatrial , sizeof( smRENDMATRIAL ) * nMaterial );

	RendStatTime = GetCurrentTime();

	return TRUE;
}

// 임시 메시를 지우고 초기화 함
int smRENDER3D::Init()
{
	return InitStageMesh( 0 , 0 );
}

// 클리핑값 설정
int smRENDER3D::SetClipStates( DWORD States )
{
	ClipStates = States;
	return TRUE;
}

// 메트리얼 그룹 변경
int smRENDER3D::SetMaterialGroup( smMATERIAL_GROUP *MatGroup )
{
	smRendMaterial	= MatGroup->smMaterial;		//메트리얼 포인터
//	nMaterial		= MatGroup->MaterialCount;	//렌더링 할 전체 메트리얼 갯수
	nMaterial		= MatGroup->MaxMaterial;	//렌더링 할 전체 메트리얼 갯수
	RMatListRearCnt = nMaterial-1;				//메트리얼 관련 카운터 초기화

	return TRUE;
}


//텍스쳐 좌표연결을 추가한다 
int smRENDER3D::AddRendTempTexLink( smRENDFACE *lpRendFace , DWORD *hTex , 
						  float u0 , float v0 , float u1 , float v1 , float u2 , float v2 )
{
	int cnt;
	smTEXLINK	*tl;

	//좌표설정
	TempTexLink[ nTempTexLink ].u[0] = u0;
	TempTexLink[ nTempTexLink ].v[0] = v0;
	TempTexLink[ nTempTexLink ].u[1] = u1;
	TempTexLink[ nTempTexLink ].v[1] = v1;
	TempTexLink[ nTempTexLink ].u[2] = u2;
	TempTexLink[ nTempTexLink ].v[2] = v2;
	TempTexLink[ nTempTexLink ].hTexture = 0;//hTex;
	TempTexLink[ nTempTexLink ].NextTex = 0;

	//지정된 페이스와 연결시킨다
	if ( !lpRendFace->lpTexLink ) {
		//최초의 텍스쳐좌표일 경우
		lpRendFace->lpTexLink = &TempTexLink[ nTempTexLink ];
	}
	else {
		//이미 연결되있는 좌표가 있는경우
		tl = lpRendFace->lpTexLink;

		for( cnt=0;cnt<8;cnt++ ) {
			if ( !tl->NextTex ) {
				//마지막으로 연결된 좌표를 찾아 연결
				tl->NextTex = &TempTexLink[ nTempTexLink ];
				break;
			}
			else {
				//다음 좌표로 넘김
				tl = tl->NextTex;
			}

		}
	}

	nTempTexLink++;

	return nTempTexLink-1;
}


//스테이지 버텍스로 렌더링 버텍스 추가 ( 월드 좌표 소스 사용 )
smRENDVERTEX *smRENDER3D::AddStageVertex ( smSTAGE_VERTEX *sv , smMATERIAL *lpMaterial )
{

	int rx,rz; //,ry 
	int x,y,z;
	int ad;
	DWORD ClipStatus;
	int ShiftMesh;

	DWORD *dwColor , *dwDefColor;


	int ttCnt , ttFlag;



	/*
	{ "wind_z1:" , sMATS_SCRIPT_WINDZ1 },
	{ "wind_z2:" , sMATS_SCRIPT_WINDZ2 },
	{ "wind_x1:" , sMATS_SCRIPT_WINDX1 },
	{ "wind_x2:" , sMATS_SCRIPT_WINDX2 },

	{ "water:" , sMATS_SCRIPT_WATER }
	*/

	//######################################################################################
	// 작 성 자 : 오 영 석
	if( lpMaterial->WindMeshBottom && (lpMaterial->UseState & sMATS_SCRIPT_BLINK_COLOR) == 0 )
	//######################################################################################
	{
		//특수 효과를 위한 좌표 변형
		switch( lpMaterial->WindMeshBottom & 0x7FF ) {

		case sMATS_SCRIPT_WINDX1:
			ttCnt = (RendStatTime>>2)&0xFF;
			ttFlag = (RendStatTime>>10)&1;

			if ( !ttFlag ) ttCnt = 255-ttCnt;
			ShiftMesh = (GetCos[ttCnt+256] )>>5;

			z =  sv->z;
			y =  sv->y;
			x =  ( sv->x - ShiftMesh );
			break;

		case sMATS_SCRIPT_WINDZ1:
			ttCnt = (RendStatTime>>2)&0xFF;
			ttFlag = (RendStatTime>>10)&1;

			if ( !ttFlag ) ttCnt = 255-ttCnt;
			ShiftMesh = (GetCos[ttCnt+256] )>>5;

			x =  sv->x;
			y =  sv->y;
			z =  ( sv->z - ShiftMesh );
			break;

		case sMATS_SCRIPT_WINDX2:
			ttCnt = (RendStatTime>>2)&0xFF;
			ttFlag = (RendStatTime>>10)&1;

			if ( !ttFlag ) ttCnt = 255-ttCnt;
			ShiftMesh = (GetCos[ttCnt+256] )>>3;

			z =  sv->z;
			y =  sv->y;
			x =  ( sv->x - ShiftMesh );
			break;

		case sMATS_SCRIPT_WINDZ2:
			ttCnt = (RendStatTime>>2)&0xFF;
			ttFlag = (RendStatTime>>10)&1;

			if ( !ttFlag ) ttCnt = 255-ttCnt;
			ShiftMesh = (GetCos[ttCnt+256] )>>3;

			x =  sv->x;
			y =  sv->y;
			z =  ( sv->z - ShiftMesh );
			break;

		case sMATS_SCRIPT_WATER:
			x = sv->x;
			z = sv->z;
			y = sv->y;

			rx = (((x<<3)+RendStatTime)>>1);
			rz = (((z<<3)+RendStatTime)>>1);

			rx = GetSin[ rx&ANGCLIP ];
			rz = GetSin[ rz&ANGCLIP ];

			x += rx>>4;
			z += rz>>4;
			break;
		}
	}
	else {
		x = sv->x;
		y = sv->y;
		z = sv->z;
	}
/*
			x = sv->x;
			z = sv->z;
			y = sv->y;

			rx = (((x<<3)+RendStatTime)>>1);
			rz = (((z<<3)+RendStatTime)>>1);

			rx = GetSin[ rx&ANGCLIP ];
			rz = GetSin[ rz&ANGCLIP ];

			x += rx>>4;
			z += rz>>4;
*/
		x -= CameraPosiX;
		y -= CameraPosiY;
		z -= CameraPosiZ;
/*
		double dx,dy,dz;
		double drx,dry,drz;

		dx = (double)x/(float)fONE;
		dy = (double)y/(float)fONE;
		dz = (double)z/(float)fONE;
		//추가하면서 카메라 변환 메트릭스를 곱하여 카메라 좌표를 변환하여 저장
		drx = dx * mDCamera._11 +
			 dy * mDCamera._21 +
			 dz * mDCamera._31 ;

		dry = dx * mDCamera._12 +
			 dy * mDCamera._22 +
			 dz * mDCamera._32 ;

		drz = dx * mDCamera._13 +
			 dy * mDCamera._23 +
			 dz * mDCamera._33 ;

		RendVertex[nRendVertex].dx = drx;
		RendVertex[nRendVertex].dy = dry;
		RendVertex[nRendVertex].dz = drz;

		x = drx*fONE;
		y = dry*fONE;
		z = drz*fONE;

*/

		//추가하면서 카메라 변환 메트릭스를 곱하여 카메라 좌표를 변환하여 저장

		//######################################################################################
		//작 성 자 : 오 영 석		
		__int64 x_64, y_64, z_64, rx_64, ry_64, rz_64;
		rx_64 = ry_64 = rz_64 = 0;

		x_64 = x;
		y_64 = y;
		z_64 = z;

		rx_64  = ((x_64 * mCamera._11) >> FLOATS_EM);
		rx_64 += ((y_64 * mCamera._21) >> FLOATS_EM);
		rx_64 += ((z_64 * mCamera._31) >> FLOATS_EM);

		ry_64  = ((x_64 * mCamera._12) >> FLOATS_EM);
		ry_64 += ((y_64 * mCamera._22) >> FLOATS_EM);
		ry_64 += ((z_64 * mCamera._32) >> FLOATS_EM);

		rz_64  = ((x_64 * mCamera._13) >> FLOATS_EM);
		rz_64 += ((y_64 * mCamera._23) >> FLOATS_EM);
		rz_64 += ((z_64 * mCamera._33) >> FLOATS_EM);

		x = int(rx_64);
		y = int(ry_64);
		z = int(rz_64);
		
		/*rx = x * mCamera._11 +
			 y * mCamera._21 +
			 z * mCamera._31 ;

		ry = x * mCamera._12 +
			 y * mCamera._22 +
			 z * mCamera._32 ;

		rz = x * mCamera._13 +
			 y * mCamera._23 +
			 z * mCamera._33 ;

		x = rx>>FLOATS_EM;
		y = ry>>FLOATS_EM;
		z = rz>>FLOATS_EM;*/
		//######################################################################################

		RendVertex[ nRendVertex ].tx = x ;
		RendVertex[ nRendVertex ].ty = y ;
		RendVertex[ nRendVertex ].tz = z ;
		RendVertex[ nRendVertex ].lpSourceVertex = sv;
		ClipStatus = 0;

		// 클립핑 Z
		if ( z<=SMFLOAT_DIST_NEARZ ) ClipStatus |= SMCLIP_NEARZ;
		if ( z>SMFLOAT_DIST_FARZ ) ClipStatus |= SMCLIP_FARZ;
		//if ( z<=SMFLOAT_DIST_TEXCLIP ) ClipStatus |= SMCLIP_TEXTURE;
//		else {
			// 45도 클립핑 , 좌우
			ad = abs( x )<< SMSHIFT_PERSPECTIVE_WIDTH;

			if ( x<0 ) {
				if ( z<=ad ) ClipStatus |=SMCLIP_LEFT;
			}
			else {
				if ( z<=ad ) ClipStatus |=SMCLIP_RIGHT;
			}
			// 45도 클립핑 , 상하
			ad = ( abs( y ) * SMMULT_PERSPECTIVE_HEIGHT ) >>FLOATNS;

			if ( y<0 ) {
				if ( z<=ad ) ClipStatus |=SMCLIP_BOTTOM;
			}
			else {
				if ( z<=ad ) ClipStatus |=SMCLIP_TOP;
			}
//		}

		RendVertex[ nRendVertex ].ClipStatus = ClipStatus;


		//색상 복사

		dwColor = (DWORD*)RendVertex[ nRendVertex ].sLight;
		dwDefColor =(DWORD *)sv->sDef_Color;

		*dwColor ++ = *dwDefColor++;
		*dwColor	= *dwDefColor;

		nRendVertex ++;

	return &RendVertex[ nRendVertex-1 ];

}


//스테이지 페이스로 렌더링 페이스 추가
smRENDFACE *smRENDER3D::AddStageFace ( smSTAGE_FACE *face )
{
	int cnt;
	smSTAGE_VERTEX *sv;
	DWORD ClipStatus;
	smMATERIAL *lpMaterial;

	if ( SumCount!=face->sum ) {

		// buffers nearly full (long draw distance): leave the rest of the map out
		// rather than writing past their end
		if ( s_RendFaceCap && ( nRendFace>=s_RendFaceCap-REND_ROOM || nRendVertex>=s_RendVertexCap-REND_ROOM ) )
			return NULL;

		ClipStatus = 0;
		lpMaterial = &smRendMaterial[ face->Vertex[3] ];
		face->sum = SumCount;

		if ( lpMaterial->UseState & dwMatDispMask ) 
			return NULL;

		for( cnt=0; cnt<3 ; cnt ++ ) {
			sv = &Vertex[ face->Vertex[cnt] ];
			if ( sv->sum!=SumCount ) {
				//연산및 렌더링리스트에 추가되지 않은 버텍스
				sv->sum = SumCount;
				sv->lpRendVertex = AddStageVertex( sv , lpMaterial );
			}
			RendFace[ nRendFace ].lpRendVertex[cnt] = sv->lpRendVertex;
			ClipStatus |= sv->lpRendVertex->ClipStatus;			//클리핑 정보 얻음
		}

		RendFace[ nRendFace ].Matrial   = face->Vertex[3];		//메트리얼 복사
		RendFace[ nRendFace ].lpTexLink = face->lpTexLink;	//텍스쳐좌표연결 포인터 복사
		RendFace[ nRendFace ].ClipStatus = ClipStatus;



/*
	POINT3D p1,p2,p3,p;

	p.x = 25*64*fONE;
	p.z = 25*64*fONE;
	p.y = 500*fONE;
	
		sv = &Vertex[ face->Vertex[0] ];
		p1.x = sv->x;
		p1.y = sv->y;
		p1.z = sv->z;

		sv = &Vertex[ face->Vertex[1] ];
		p2.x = sv->x;
		p2.y = sv->y;
		p2.z = sv->z;

		sv = &Vertex[ face->Vertex[2] ];
		p3.x = sv->x;
		p3.y = sv->y;
		p3.z = sv->z;

		if ( GetPlaneProduct( &p1 ,&p2 , &p3 , &p )>0 )
			RendFace[ nRendFace ].ClipStatus = SMCLIP_DISPLAYOUT;

*/

		nRendFace ++;
		return &RendFace[ nRendFace-1 ];
	}

	return NULL;
}





//오브젝트 버텍스로 렌더링 버텍스 추가 ( 월드 좌표 소스 사용 )
//######################################################################################
//작 성 자 : 오 영 석
//smRENDVERTEX *smRENDER3D::AddObjectVertex ( smVERTEX *sv )
smRENDVERTEX *smRENDER3D::AddObjectVertex ( smVERTEX *sv, DWORD dwObjHead )
//######################################################################################
{

	int rx,ry,rz;
	int x,y,z;
	int ad;
	DWORD ClipStatus;

		x = sv->x;
		y = sv->y;
		z = sv->z;

		if ( ObjectVertexScale ) {
			x = (x*ObjectVertexScale)/fONE;
			y = (y*ObjectVertexScale)/fONE;
			z = (z*ObjectVertexScale)/fONE;
		}

		x -= CameraPosiX;
		y -= CameraPosiY;
		z -= CameraPosiZ;

		//추가하면서 카메라 변환 메트릭스를 곱하여 카메라 좌표를 변환하여 저장

		//######################################################################################
		//작 성 자 : 오 영 석
		__int64 x_64, y_64, z_64, rx_64, ry_64, rz_64;
		rx_64 = ry_64 = rz_64 = 0;

		x_64 = x;
		y_64 = y;
		z_64 = z;

		rx_64  = ((x_64 * mCamera._11) >> FLOATS_EM);
		rx_64 += ((y_64 * mCamera._21) >> FLOATS_EM);
		rx_64 += ((z_64 * mCamera._31) >> FLOATS_EM);

		ry_64  = ((x_64 * mCamera._12) >> FLOATS_EM);
		ry_64 += ((y_64 * mCamera._22) >> FLOATS_EM);
		ry_64 += ((z_64 * mCamera._32) >> FLOATS_EM);

		rz_64  = ((x_64 * mCamera._13) >> FLOATS_EM);
		rz_64 += ((y_64 * mCamera._23) >> FLOATS_EM);
		rz_64 += ((z_64 * mCamera._33) >> FLOATS_EM);
	
		x = int(rx_64);
		y = int(ry_64);
		z = int(rz_64);
		
		/*
		rx = x * mCamera._11 +
			 y * mCamera._21 +
			 z * mCamera._31 ;

		ry = x * mCamera._12 +
			 y * mCamera._22 +
			 z * mCamera._32 ;

		rz = x * mCamera._13 +
			 y * mCamera._23 +
			 z * mCamera._33 ;

		x = rx>>FLOATS_EM;
		y = ry>>FLOATS_EM;
		z = rz>>FLOATS_EM;

		///////////////////////////////

		rx = x * (mCamera._11>>1) +
			 y * (mCamera._21>>1) +
			 z * (mCamera._31>>1) ;

		ry = x * (mCamera._12>>1) +
			 y * (mCamera._22>>1) +
			 z * (mCamera._32>>1) ;

		rz = x * (mCamera._13>>1) +
			 y * (mCamera._23>>1) +
			 z * (mCamera._33>>1) ;

		x = rx>>(FLOATS_EM-1);
		y = ry>>(FLOATS_EM-1);
		z = rz>>(FLOATS_EM-1);
		*/
		//######################################################################################

		if ( DistortionMesh ) {

			rx = (((x>>1)+RendStatTime));
			ry = (((y>>1)+RendStatTime));
			rz = (((z>>1)+RendStatTime));

			rx = GetSin[ rx&ANGCLIP ];
			ry = GetSin[ ry&ANGCLIP ];
			rz = GetCos[ rz&ANGCLIP ];

			x += rx>>7;
			y += ry>>7;
			z += rz>>7;

		}

		RendVertex[ nRendVertex ].tx = x ;
		RendVertex[ nRendVertex ].ty = y ;
		RendVertex[ nRendVertex ].tz = z ;

		ClipStatus = 0;

		// 클립핑 Z
		if ( z<=SMFLOAT_DIST_NEARZ ) ClipStatus |= SMCLIP_NEARZ;
		if ( z>SMFLOAT_DIST_FARZ ) ClipStatus |= SMCLIP_FARZ;
		//if ( z<=SMFLOAT_DIST_TEXCLIP ) ClipStatus |= SMCLIP_TEXTURE;
		//if ( z>0 ) {

			// 45도 클립핑 , 좌우
			ad = abs( x )<< SMSHIFT_PERSPECTIVE_WIDTH;

			if ( x<0 ) {
				if ( z<=ad ) ClipStatus |=SMCLIP_LEFT;
			}
			else {
				if ( z<=ad ) ClipStatus |=SMCLIP_RIGHT;
			}

			// 45도 클립핑 , 상하
			ad = ( abs( y ) * SMMULT_PERSPECTIVE_HEIGHT ) >>FLOATNS;
			if ( y<0 ) {
				if ( z<=ad ) ClipStatus |=SMCLIP_BOTTOM;
			}
			else {
				if ( z<=ad ) ClipStatus |=SMCLIP_TOP;
			}

		//}
		RendVertex[ nRendVertex ].ClipStatus = ClipStatus;


//		POINT3D	pLight;
		int gShade;
		DWORD r,g,b;
		short *sColor;
/*
		// 라이트 벡터
		pLight.x = -fONE;
		pLight.y = -fONE;
		pLight.z = fONE/2;
*/

		//######################################################################################
		//작 성 자 : 오 영 석
		if( dwObjHead & OBJ_HEAD_TYPE_NEW_NORMAL )
		{
			x = sv->nx;
			y = sv->ny;
			z = sv->nz;

			if( ObjectVertexScale )
			{
				x = (x*ObjectVertexScale) / fONE;
				y = (y*ObjectVertexScale) / fONE;
				z = (z*ObjectVertexScale) / fONE;
			}
			
			x -= CameraPosiX;
			y -= CameraPosiY;
			z -= CameraPosiZ;

			//
			x_64 = x;
			y_64 = y;
			z_64 = z;

			rx_64  = ((x_64 * mCamera._11) >> FLOATS_EM);
			rx_64 += ((y_64 * mCamera._21) >> FLOATS_EM);
			rx_64 += ((z_64 * mCamera._31) >> FLOATS_EM);

			ry_64  = ((x_64 * mCamera._12) >> FLOATS_EM);
			ry_64 += ((y_64 * mCamera._22) >> FLOATS_EM);
			ry_64 += ((z_64 * mCamera._32) >> FLOATS_EM);

			rz_64  = ((x_64 * mCamera._13) >> FLOATS_EM);
			rz_64 += ((y_64 * mCamera._23) >> FLOATS_EM);
			rz_64 += ((z_64 * mCamera._33) >> FLOATS_EM);
		
			x = int(rx_64);
			y = int(ry_64);
			z = int(rz_64);

			/*
			rx = x * mCamera._11 +
				 y * mCamera._21 +
				 z * mCamera._31 ;

			ry = x * mCamera._12 +
				 y * mCamera._22 +
				 z * mCamera._32 ;

			rz = x * mCamera._13 +
				 y * mCamera._23 +
				 z * mCamera._33 ;

			x = rx>>FLOATS_EM;
			y = ry>>FLOATS_EM;
			z = rz>>FLOATS_EM;
			*/

			RendVertex[ nRendVertex ].nx = x;
			RendVertex[ nRendVertex ].ny = y;
			RendVertex[ nRendVertex ].nz = z;
		}
		else
		{
			RendVertex[ nRendVertex ].nx = 0;
			RendVertex[ nRendVertex ].ny = 0;
			RendVertex[ nRendVertex ].nz = 0;
		}
		//######################################################################################


		//######################################################################################
		//작 성 자 : 오 영 석
		if( dwObjHead & OBJ_HEAD_TYPE_NEW_NORMAL )
		{
			x = sv->nx - sv->x;
			y = sv->ny - sv->y;
			z = sv->nz - sv->z;
			gShade = ((x * pLight.x) + (y * pLight.y) + (z * pLight.z)) >> FLOATNS;

			gShade = abs(gShade);
			gShade = gShade / 140;												//CONTRAST
			r = g = b = DEFAULT_BRIGHT-50;										// BRIGHT

			r += gShade;
			g += int(gShade * 0.93f);
			b += int(gShade * 0.84f); 
		}
		else
		{
			gShade = ((sv->nx * pLight.x) + (sv->ny * pLight.y) + (sv->nz * pLight.z)) >> FLOATNS;
			gShade /= DEFAULT_CONTRAST+64;										//CONTRAST
			gShade += DEFAULT_BRIGHT+48;										//BRIGHT

			r = g = b = gShade;
		}
		//######################################################################################
		
		sColor = RendVertex[ nRendVertex ].sLight;
		*sColor++ = (short)r;
		*sColor++ = (short)g;
		*sColor++ = (short)b;
		*sColor   = 255;

		nRendVertex ++;

	return &RendVertex[ nRendVertex-1 ];
}

//오브젝트 페이스로 렌더링 페이스 추가
smRENDFACE *smRENDER3D::AddObjectFace ( smFACE *face )
{
	int cnt;
	smRENDVERTEX *sv;
	DWORD ClipStatus;

		ClipStatus = 0;

		for( cnt=0; cnt<3 ; cnt ++ ) {
			sv = &RendVertex[ face->v[cnt] ];
			RendFace[ nRendFace ].lpRendVertex[cnt] = sv;
			ClipStatus |= sv->ClipStatus;					//클리핑 정보 얻음
		}

		RendFace[ nRendFace ].Matrial   = face->v[3];		//메트리얼 복사
		RendFace[ nRendFace ].lpTexLink = face->lpTexLink;	//텍스쳐좌표연결 포인터 복사
		RendFace[ nRendFace ].ClipStatus = ClipStatus;

		nRendFace ++;
		return &RendFace[ nRendFace-1 ];

	return NULL;
}



//2D 이미지 오브젝트 추가
smRENDFACE *smRENDER3D::AddFace2D ( smFACE2D *face )
{
	int x,y,z;
	//int rx , ry, rz;
	int width , height;

	smRENDVERTEX *v[4];
	smRENDFACE	 *rf;
	short		sColor[4];

	sColor[ SMC_A ] = face->Transparency;
	sColor[ SMC_R ] = face->r;
	sColor[ SMC_G ] = face->g;
	sColor[ SMC_B ] = face->b;

	x = face->x;
	y = face->y;
	z = face->z;

	//중심 좌표를 카메라 좌표를 변환
	x -= CameraPosiX;
	y -= CameraPosiY;
	z -= CameraPosiZ;

	//######################################################################################
	//작 성 자 : 오 영 석		
	__int64 x_64, y_64, z_64, rx_64, ry_64, rz_64;
	rx_64 = ry_64 = rz_64 = 0;

	x_64 = x;
	y_64 = y;
	z_64 = z;

	rx_64  = ((x_64 * mCamera._11) >> FLOATS_EM);
	rx_64 += ((y_64 * mCamera._21) >> FLOATS_EM);
	rx_64 += ((z_64 * mCamera._31) >> FLOATS_EM);

	ry_64  = ((x_64 * mCamera._12) >> FLOATS_EM);
	ry_64 += ((y_64 * mCamera._22) >> FLOATS_EM);
	ry_64 += ((z_64 * mCamera._32) >> FLOATS_EM);

	rz_64  = ((x_64 * mCamera._13) >> FLOATS_EM);
	rz_64 += ((y_64 * mCamera._23) >> FLOATS_EM);
	rz_64 += ((z_64 * mCamera._33) >> FLOATS_EM);

	x = int(rx_64);
	y = int(ry_64);
	z = int(rz_64);

	/*
	rx = x * mCamera._11 +
		 y * mCamera._21 +
		 z * mCamera._31 ;

	ry = x * mCamera._12 +
		 y * mCamera._22 +
		 z * mCamera._32 ;

	rz = x * mCamera._13 +
		 y * mCamera._23 +
		 z * mCamera._33 ;

	x = rx>>FLOATS_EM;
	y = ry>>FLOATS_EM;
	z = rz>>FLOATS_EM;
	*/
	//######################################################################################

//	z -= 12*fONE;
	z -= 24*fONE;


	if ( z<SMFLOAT_DIST_NEARZ ) return NULL;

	width  = face->width >> 1;
	height = face->height >> 1;

	//렌더링 버텍스 추가 ( 좌표 직접 입력 )

	v[0] = AddRendVertex ( x-width , y-height , z, sColor );
	v[1] = AddRendVertex ( x+width , y-height , z, sColor );
	v[2] = AddRendVertex ( x-width , y+height , z, sColor );
	v[3] = AddRendVertex ( x+width , y+height , z, sColor );

	//첫번째 폴리곤 ( 1 - 2 - 3 );
	rf = &RendFace[ nRendFace++ ];
	rf->lpRendVertex[0] = v[0];
	rf->lpRendVertex[1] = v[1];
	rf->lpRendVertex[2] = v[2];
	rf->Matrial    = face->MatNum ;		//메트리얼 복사
	rf->ClipStatus = v[0]->ClipStatus|v[1]->ClipStatus|v[2]->ClipStatus;
	rf->lpTexLink  = 0;

	//텍스쳐 연결
	AddRendTempTexLink( rf , 0,	face->TexRect.left , face->TexRect.bottom,
								face->TexRect.right , face->TexRect.bottom,
								face->TexRect.left , face->TexRect.top );


	//두번째 폴리곤 ( 2 - 4 - 3 );
	rf = &RendFace[ nRendFace++ ];
	rf->lpRendVertex[0] = v[1];
	rf->lpRendVertex[1] = v[3];
	rf->lpRendVertex[2] = v[2];
	rf->Matrial    = face->MatNum ;		//메트리얼 복사
	rf->ClipStatus = v[1]->ClipStatus|v[2]->ClipStatus|v[3]->ClipStatus;
	rf->lpTexLink  = 0;

	//텍스쳐 연결
	AddRendTempTexLink( rf , 0,	face->TexRect.right , face->TexRect.bottom,
								face->TexRect.right , face->TexRect.top,
								face->TexRect.left , face->TexRect.top );


	return &RendFace[ nRendFace-1 ];
}

//2D 이미지 오브젝트 추가 ( 회전 적용 )
smRENDFACE *smRENDER3D::AddFace2D ( smFACE2D *face , int angle )
{
	int x,y,z;
	//int rx , ry, rz;
	int width , height;

	smRENDVERTEX *v[4];
	smRENDFACE	 *rf;
	short		sColor[4];

	sColor[ SMC_A ] = face->Transparency;
	sColor[ SMC_R ] = face->r;
	sColor[ SMC_G ] = face->g;
	sColor[ SMC_B ] = face->b;

	x = face->x;
	y = face->y;
	z = face->z;

	//중심 좌표를 카메라 좌표를 변환
	x -= CameraPosiX;
	y -= CameraPosiY;
	z -= CameraPosiZ;

	//######################################################################################
	//작 성 자 : 오 영 석		
	__int64 x_64, y_64, z_64, rx_64, ry_64, rz_64;
	rx_64 = ry_64 = rz_64 = 0;

	x_64 = x;
	y_64 = y;
	z_64 = z;

	rx_64  = ((x_64 * mCamera._11) >> FLOATS_EM);
	rx_64 += ((y_64 * mCamera._21) >> FLOATS_EM);
	rx_64 += ((z_64 * mCamera._31) >> FLOATS_EM);

	ry_64  = ((x_64 * mCamera._12) >> FLOATS_EM);
	ry_64 += ((y_64 * mCamera._22) >> FLOATS_EM);
	ry_64 += ((z_64 * mCamera._32) >> FLOATS_EM);

	rz_64  = ((x_64 * mCamera._13) >> FLOATS_EM);
	rz_64 += ((y_64 * mCamera._23) >> FLOATS_EM);
	rz_64 += ((z_64 * mCamera._33) >> FLOATS_EM);

	x = int(rx_64);
	y = int(ry_64);
	z = int(rz_64);

	/*
	rx = x * mCamera._11 +
		 y * mCamera._21 +
		 z * mCamera._31 ;

	ry = x * mCamera._12 +
		 y * mCamera._22 +
		 z * mCamera._32 ;

	rz = x * mCamera._13 +
		 y * mCamera._23 +
		 z * mCamera._33 ;

	x = rx>>FLOATS_EM;
	y = ry>>FLOATS_EM;
	z = rz>>FLOATS_EM;
	*/
	//######################################################################################

//	z -= 12*fONE;
	z -= 24*fONE;


	if ( z<SMFLOAT_DIST_NEARZ ) return NULL;

	width  = face->width >> 1;
	height = face->height >> 1;


	//회전 적용
	int		ex[4],ey[4];
	int		mx,my;
	int		sin,cos;

	sin = GetSin[angle&ANGCLIP]>>FLOATNS;
	cos = GetCos[angle&ANGCLIP]>>FLOATNS;

	mx = -width;	my = -height;
	ex[0] = (mx * cos - my * sin) >> FLOATNS;
    ey[0] = (mx * sin + my * cos) >> FLOATNS;

	mx = width;	my = -height;
	ex[1] = (mx * cos - my * sin) >> FLOATNS;
    ey[1] = (mx * sin + my * cos) >> FLOATNS;

	mx = -width;	my = +height;
	ex[2] = (mx * cos - my * sin) >> FLOATNS;
    ey[2] = (mx * sin + my * cos) >> FLOATNS;

	mx = width;	my = height;
	ex[3] = (mx * cos - my * sin) >> FLOATNS;
    ey[3] = (mx * sin + my * cos) >> FLOATNS;


	//렌더링 버텍스 추가 ( 좌표 직접 입력 )
	v[0] = AddRendVertex ( x+ex[0] , y+ey[0] , z, sColor );
	v[1] = AddRendVertex ( x+ex[1] , y+ey[1] , z, sColor );
	v[2] = AddRendVertex ( x+ex[2] , y+ey[2] , z, sColor );
	v[3] = AddRendVertex ( x+ex[3] , y+ey[3] , z, sColor );

	//첫번째 폴리곤 ( 1 - 2 - 3 );
	rf = &RendFace[ nRendFace++ ];
	rf->lpRendVertex[0] = v[0];
	rf->lpRendVertex[1] = v[1];
	rf->lpRendVertex[2] = v[2];
	rf->Matrial    = face->MatNum ;		//메트리얼 복사
	rf->ClipStatus = v[0]->ClipStatus|v[1]->ClipStatus|v[2]->ClipStatus;
	rf->lpTexLink  = 0;

	//텍스쳐 연결
	AddRendTempTexLink( rf , 0,	face->TexRect.left , face->TexRect.bottom,
								face->TexRect.right , face->TexRect.bottom,
								face->TexRect.left , face->TexRect.top );


	//두번째 폴리곤 ( 2 - 4 - 3 );
	rf = &RendFace[ nRendFace++ ];
	rf->lpRendVertex[0] = v[1];
	rf->lpRendVertex[1] = v[3];
	rf->lpRendVertex[2] = v[2];
	rf->Matrial    = face->MatNum ;		//메트리얼 복사
	rf->ClipStatus = v[1]->ClipStatus|v[2]->ClipStatus|v[3]->ClipStatus;
	rf->lpTexLink  = 0;

	//텍스쳐 연결
	AddRendTempTexLink( rf , 0,	face->TexRect.right , face->TexRect.bottom,
								face->TexRect.right , face->TexRect.top,
								face->TexRect.left , face->TexRect.top );


	return &RendFace[ nRendFace-1 ];
}


//2D 이미지 오브젝트 추가 ( 화면좌표에 직접 그린다 )
smRENDFACE *smRENDER3D::AddFace2D_Direct ( smFACE2D *face )
{
	int cnt;
	int x1,x2,y1,y2;

	smRENDVERTEX *v[4];
	smRENDFACE	 *rf;
	DWORD		dwColor;

	dwColor	= 0xFFFFFFFF;
	((BYTE *)&dwColor)[ SMC_A ] = face->Transparency;

	for( cnt =0; cnt<4; cnt++ ) {
		v[cnt] = &RendVertex[ nRendVertex++ ];
		v[cnt]->zb = smConvZ3D( float(face->z) );
		v[cnt]->rhw = 1;
		v[cnt]->ClipStatus = 0;
		((DWORD *)v[cnt]->bCol)[0] = dwColor;
		((DWORD *)v[cnt]->bSpe)[0] = 0;
	}

	x1 = face->x; x2 = x1+face->width ;
	y1 = face->y; y2 = y1+face->height;

	v[0]->xy[0] = (float)x1;
	v[0]->xy[1] = (float)y1;
	v[1]->xy[0] = (float)x2;
	v[1]->xy[1] = (float)y1;
	v[2]->xy[0] = (float)x1;
	v[2]->xy[1] = (float)y2;
	v[3]->xy[0] = (float)x2;
	v[3]->xy[1] = (float)y2;

	//첫번째 폴리곤 ( 1 - 2 - 3 );
	rf = &RendFace[ nRendFace++ ];
	rf->lpRendVertex[0] = v[0];
	rf->lpRendVertex[1] = v[1];
	rf->lpRendVertex[2] = v[2];
	rf->Matrial    = face->MatNum ;		//메트리얼 복사
	rf->ClipStatus = 0;
	rf->lpTexLink  = 0;

	//텍스쳐 연결
	AddRendTempTexLink( rf , 0,	face->TexRect.left , face->TexRect.top,
								face->TexRect.right , face->TexRect.top,
								face->TexRect.left , face->TexRect.bottom );


	//두번째 폴리곤 ( 2 - 4 - 3 );
	rf = &RendFace[ nRendFace++ ];
	rf->lpRendVertex[0] = v[1];
	rf->lpRendVertex[1] = v[3];
	rf->lpRendVertex[2] = v[2];
	rf->Matrial    = face->MatNum ;		//메트리얼 복사
	rf->ClipStatus = 0;
	rf->lpTexLink  = 0;

	//텍스쳐 연결
	AddRendTempTexLink( rf , 0,	face->TexRect.right , face->TexRect.top,
								face->TexRect.right , face->TexRect.bottom,
								face->TexRect.left , face->TexRect.bottom );


	return &RendFace[ nRendFace-1 ];
}












//######################################################################################
//작 성 자 : 오 영 석
//렌더링 버텍스 추가 ( 좌표 직접 입력 )
//smRENDVERTEX *smRENDER3D::AddRendVertex ( int x, int y, int z, short *pLight )
smRENDVERTEX *smRENDER3D::AddRendVertex ( int x, int y, int z, short *pLight, int nx, int ny, int nz )
//######################################################################################
{

	DWORD ClipStatus;
	int	ad;

	RendVertex[ nRendVertex ].tx = x ;
	RendVertex[ nRendVertex ].ty = y ;
	RendVertex[ nRendVertex ].tz = z ;

	//######################################################################################
	//작 성 자 : 오 영 석
	RendVertex[ nRendVertex ].nx = nx;
	RendVertex[ nRendVertex ].ny = ny;
	RendVertex[ nRendVertex ].nz = nz;
	//######################################################################################

	ClipStatus = 0;

	// 클립핑 Z
	if ( z<SMFLOAT_DIST_NEARZ ) ClipStatus |= SMCLIP_NEARZ;
	if ( z>SMFLOAT_DIST_FARZ ) ClipStatus |= SMCLIP_FARZ;
	//if ( z<SMFLOAT_DIST_TEXCLIP ) ClipStatus |= SMCLIP_TEXTURE;
//	else {
		// 45도 클립핑 , 좌우
		ad = abs( x )<< SMSHIFT_PERSPECTIVE_WIDTH;
		if ( x<0 ) {
			if ( z<ad ) ClipStatus |=SMCLIP_LEFT;
		}
		else {
			if ( z<ad ) ClipStatus |=SMCLIP_RIGHT;
		}

		// 45도 클립핑 , 상하
		ad = ( abs( y ) * SMMULT_PERSPECTIVE_HEIGHT ) >>FLOATNS;
		if ( y<0 ) {
			if ( z<ad ) ClipStatus |=SMCLIP_BOTTOM;
		}
		else {
			if ( z<ad ) ClipStatus |=SMCLIP_TOP;
		}
//	}

	RendVertex[ nRendVertex ].ClipStatus = ClipStatus;

	//색상과 스펙큘러 복사
	DWORD *dwLight , *dwpLight;

	dwLight = (DWORD *)RendVertex[ nRendVertex ].sLight;
	dwpLight = (DWORD *)pLight;

	*dwLight++	= *dwpLight++;
	*dwLight	= *dwpLight;

	nRendVertex ++;

	return &RendVertex[ nRendVertex-1 ];
}


//렌더링 페이스 추가 ( 버텍스 포인터 직접 입력 )
smRENDFACE *smRENDER3D::AddRendFace ( smRENDVERTEX *v0 , smRENDVERTEX *v1, smRENDVERTEX *v2 , 
									DWORD matrial , smTEXLINK *lpTexLink )
{

	RendFace[ nRendFace ].lpRendVertex[0] = v0;
	RendFace[ nRendFace ].lpRendVertex[1] = v1;
	RendFace[ nRendFace ].lpRendVertex[2] = v2;

	RendFace[ nRendFace ].Matrial   = matrial;			//메트리얼 복사
	RendFace[ nRendFace ].lpTexLink = lpTexLink;		//텍스쳐좌표연결 포인터 복사

	RendFace[ nRendFace ].ClipStatus = v0->ClipStatus|v1->ClipStatus|v2->ClipStatus;

	nRendFace ++;
	return &RendFace[ nRendFace-1 ];
}

//페이스 사이드값 구함 ( DotProduct )
int smRENDER3D::GetFaceSide( smRENDFACE *rendface )
{


	smRENDVERTEX	*v0;
	smRENDVERTEX	*v1;
	smRENDVERTEX	*v2;

	v0 = rendface->lpRendVertex[0];
	v1 = rendface->lpRendVertex[1];
	v2 = rendface->lpRendVertex[2];

	int temp1 , temp2, temp3;
	int tempX , tempY, tempZ;
	int tempA , tempB, tempC;

	tempX = v1->tx - v0->tx;
	tempY = v1->ty - v0->ty;
	tempZ = v1->tz - v0->tz;

	tempA = v2->tx - v0->tx;
	tempB = v2->ty - v0->ty;
	tempC = v2->tz - v0->tz;

	temp1 = ( ( tempY * tempC ) - ( tempZ * tempB ) )>>FLOATNS;   
	temp2 = ( ( tempX * tempC ) - ( tempZ * tempA ) )>>FLOATNS;   
	temp3 = ( ( tempX * tempB ) - ( tempY * tempA ) )>>FLOATNS;   

//	return ( temp1 * v1->tx ) + ( temp2 * v1->ty ) + ( temp3 * v1->tz );
	return temp3;
}



// 렌더링 페이스를 클리핑 ( dwClipMode는 SMCLIP_[방향] ) 
int smRENDER3D::AddRendFaceClip( smRENDFACE *rendface , DWORD dwClipMode )
{

	smRENDVERTEX *v[3];
	smRENDVERTEX *cv[2];			//교점 버텍스
	float		 fMultPoint[2];
	float		 u0,v0 , u1,v1;

	int cnt;
	int StatsClips[3];
	int midcnt , nexcnt , lascnt;

	smTEXLINK	*texLink;
	smRENDFACE	*rFace[2];


	//원래 페이스는 출력하지 않는다
	rendface->ClipStatus |= SMCLIP_DISPLAYOUT;

	// 페이스의 면이 뒤집혀 있을 경우 표시 하지 않음 ( NULL 값으로 돌려주고 연산 종료 )
	//if ( GetFaceSide( rendface )<0 ) return NULL;
	
	StatsClips[0] = rendface->lpRendVertex[0]->ClipStatus & dwClipMode ;
	StatsClips[1] = rendface->lpRendVertex[1]->ClipStatus & dwClipMode ;
	StatsClips[2] = rendface->lpRendVertex[2]->ClipStatus & dwClipMode ;

	//모든 버텍스가 화면 밖에 위치 하면 표시하지 않음 ( NULL 값을 돌려주고 연산 종료 )
	if ( StatsClips[0] && StatsClips[1] && StatsClips[2] ) return NULL;

	v[0] = rendface->lpRendVertex[0];
	v[1] = rendface->lpRendVertex[1];
	v[2] = rendface->lpRendVertex[2];

	for( midcnt=0;midcnt<3;midcnt++) {
		nexcnt = midcnt+1;
		if ( nexcnt==3 ) nexcnt = 0;
		if ( StatsClips[midcnt]==0 && StatsClips[nexcnt]!=0 ) break;
	}

	lascnt = midcnt-1;
	if ( lascnt<0 ) lascnt = 2;

	if ( !StatsClips[lascnt] ) {
		//폴리곤 나누기 2개 짜리
		switch( dwClipMode ) {
		case SMCLIP_LEFT:
			cv[0] = AddRendVertex_ClipLeft( rendface , midcnt, nexcnt ,  &fMultPoint[0] );
			cv[1] = AddRendVertex_ClipLeft( rendface , lascnt, nexcnt ,  &fMultPoint[1] );
			break;
		case SMCLIP_RIGHT:
			cv[0] = AddRendVertex_ClipRight( rendface , midcnt, nexcnt ,  &fMultPoint[0] );
			cv[1] = AddRendVertex_ClipRight( rendface , lascnt, nexcnt ,  &fMultPoint[1] );
			break;
		case SMCLIP_TOP:
			cv[0] = AddRendVertex_ClipTop( rendface , midcnt, nexcnt ,  &fMultPoint[0] );
			cv[1] = AddRendVertex_ClipTop( rendface , lascnt, nexcnt ,  &fMultPoint[1] );
			break;
		case SMCLIP_BOTTOM:
			cv[0] = AddRendVertex_ClipBottom( rendface , midcnt, nexcnt ,  &fMultPoint[0] );
			cv[1] = AddRendVertex_ClipBottom( rendface , lascnt, nexcnt ,  &fMultPoint[1] );
			break;
		case SMCLIP_NEARZ:
			cv[0] = AddRendVertex_ClipNearZ( rendface , midcnt, nexcnt ,  &fMultPoint[0] );
			cv[1] = AddRendVertex_ClipNearZ( rendface , lascnt, nexcnt ,  &fMultPoint[1] );
			break;
		case SMCLIP_FARZ:
			cv[0] = AddRendVertex_ClipFarZ( rendface , midcnt, nexcnt ,  &fMultPoint[0] );
			cv[1] = AddRendVertex_ClipFarZ( rendface , lascnt, nexcnt ,  &fMultPoint[1] );
			break;
		}

		// 페이스 추가 
		// ( v[midcnt] ) -> cv[0] -> v[lascnt]
		rFace[0] = AddRendFace( v[midcnt] , cv[0] , v[lascnt] , rendface->Matrial , NULL );
		// ( cv[0] ) ->  cv[1] -> v[lascnt]
		rFace[1] = AddRendFace( cv[0] , cv[1] , v[lascnt] , rendface->Matrial , NULL );


		// 텍스쳐 좌표를 순차적으로 구함
		texLink = rendface->lpTexLink;
		for( cnt=0;cnt<8;cnt++) {
			if ( !texLink ) break;
			u0 = texLink->u[midcnt] + ( texLink->u[nexcnt] - texLink->u[midcnt] ) * fMultPoint[0];
			v0 = texLink->v[midcnt] + ( texLink->v[nexcnt] - texLink->v[midcnt] ) * fMultPoint[0];
			u1 = texLink->u[lascnt] + ( texLink->u[nexcnt] - texLink->u[lascnt] ) * fMultPoint[1];
			v1 = texLink->v[lascnt] + ( texLink->v[nexcnt] - texLink->v[lascnt] ) * fMultPoint[1];

			// ( v[midcnt] ) -> tv[0] -> v[lascnt]
			AddRendTempTexLink( rFace[0] , texLink->hTexture , 
				texLink->u[midcnt] , texLink->v[midcnt] , u0 , v0 , texLink->u[lascnt] , texLink->v[lascnt] );

			// ( tv[0] ) ->  tv[1] -> v[lascnt]
			AddRendTempTexLink( rFace[1] , texLink->hTexture , 
				u0 , v0 , u1 , v1 , texLink->u[lascnt] , texLink->v[lascnt] );

			texLink = texLink->NextTex;
		}
		/*
		GetCrossLeft( tv[0] , face , cnt, nexcnt );
		GetCrossLeft( tv[1] , face , lascnt , nexcnt );
		*/

	}
	else {
		//폴리곤 나누기 1개 짜리
		switch( dwClipMode ) {
		case SMCLIP_LEFT:
			cv[0] = AddRendVertex_ClipLeft( rendface , midcnt, nexcnt , &fMultPoint[0] );
			cv[1] = AddRendVertex_ClipLeft( rendface , midcnt, lascnt , &fMultPoint[1] );
			break;
		case SMCLIP_RIGHT:
			cv[0] = AddRendVertex_ClipRight( rendface , midcnt, nexcnt , &fMultPoint[0] );
			cv[1] = AddRendVertex_ClipRight( rendface , midcnt, lascnt , &fMultPoint[1] );
			break;
		case SMCLIP_TOP:
			cv[0] = AddRendVertex_ClipTop( rendface , midcnt, nexcnt , &fMultPoint[0] );
			cv[1] = AddRendVertex_ClipTop( rendface , midcnt, lascnt , &fMultPoint[1] );
			break;
		case SMCLIP_BOTTOM:
			cv[0] = AddRendVertex_ClipBottom( rendface , midcnt, nexcnt , &fMultPoint[0] );
			cv[1] = AddRendVertex_ClipBottom( rendface , midcnt, lascnt , &fMultPoint[1] );
			break;
		case SMCLIP_NEARZ:
			cv[0] = AddRendVertex_ClipNearZ( rendface , midcnt, nexcnt , &fMultPoint[0] );
			cv[1] = AddRendVertex_ClipNearZ( rendface , midcnt, lascnt , &fMultPoint[1] );
			break;
		case SMCLIP_FARZ:
			cv[0] = AddRendVertex_ClipFarZ( rendface , midcnt, nexcnt , &fMultPoint[0] );
			cv[1] = AddRendVertex_ClipFarZ( rendface , midcnt, lascnt , &fMultPoint[1] );
			break;
		}

		// 페이스 추가 
		// ( v[midcnt] ) -> cv[0] -> cv[1]
		rFace[0] = AddRendFace( v[midcnt] , cv[0] , cv[1] , rendface->Matrial , NULL );

		// 텍스쳐 좌표를 순차적으로 구함
		texLink = rendface->lpTexLink;
		for( cnt=0;cnt<8;cnt++) {
			if ( !texLink ) break;
			u0 = texLink->u[midcnt] + ( texLink->u[nexcnt] - texLink->u[midcnt] ) * fMultPoint[0];
			v0 = texLink->v[midcnt] + ( texLink->v[nexcnt] - texLink->v[midcnt] ) * fMultPoint[0];
			u1 = texLink->u[midcnt] + ( texLink->u[lascnt] - texLink->u[midcnt] ) * fMultPoint[1];
			v1 = texLink->v[midcnt] + ( texLink->v[lascnt] - texLink->v[midcnt] ) * fMultPoint[1];

			// ( v[cnt] ) -> cv[0] -> cv[1]
			AddRendTempTexLink( rFace[0] , texLink->hTexture , 
				texLink->u[midcnt] , texLink->v[midcnt] , u0 , v0 , u1, v1 );

			texLink = texLink->NextTex;
		}

		/*

		GetCrossLeft( tv[0] , face , cnt, nexcnt );
		GetCrossLeft( tv[1] , face , cnt, lascnt );
		*/

	}

	return TRUE;
}




// 렌더링 페이스를 텍스쳐 클리핑 
int smRENDER3D::AddRendFaceTextureClip( smRENDFACE *rendface )
{

	smRENDVERTEX *v[3];
	smRENDVERTEX *cv[2];			//교점 버텍스
	float		 fMultPoint[2];
	float		 u0,v0 , u1,v1;
	int cnt;
	int StatsClips[3];
	int midcnt , nexcnt , lascnt;

	smTEXLINK	*texLink;
	smRENDFACE	*rFace[3];

	StatsClips[0] = rendface->lpRendVertex[0]->ClipStatus & SMCLIP_TEXTURE ;
	StatsClips[1] = rendface->lpRendVertex[1]->ClipStatus & SMCLIP_TEXTURE ;
	StatsClips[2] = rendface->lpRendVertex[2]->ClipStatus & SMCLIP_TEXTURE ;

	//모든 버텍스가 화면 밖에 위치 하면 표시하지 않음 ( NULL 값을 돌려주고 연산 종료 )
	if ( StatsClips[0] && StatsClips[1] && StatsClips[2] ) return NULL;

	//원래 페이스는 출력하지 않는다
	rendface->ClipStatus |= SMCLIP_DISPLAYOUT;

	v[0] = rendface->lpRendVertex[0];
	v[1] = rendface->lpRendVertex[1];
	v[2] = rendface->lpRendVertex[2];

	for( midcnt=0;midcnt<3;midcnt++) {
		nexcnt = midcnt+1;
		if ( nexcnt==3 ) nexcnt = 0;
		if ( StatsClips[midcnt]==0 && StatsClips[nexcnt]!=0 ) break;
	}

	lascnt = midcnt-1;
	if ( lascnt<0 ) lascnt = 2;

	if ( !StatsClips[lascnt] ) {
		//폴리곤 나누기 2개 짜리
		cv[0] = AddRendVertex_ClipTexture( rendface , midcnt, nexcnt ,  &fMultPoint[0] );
		cv[1] = AddRendVertex_ClipTexture( rendface , lascnt, nexcnt ,  &fMultPoint[1] );

		// 페이스 추가 
		// ( v[midcnt] ) -> cv[0] -> v[lascnt]
		rFace[0] = AddRendFace( v[midcnt] , cv[0] , v[lascnt] , rendface->Matrial , NULL );
		// ( cv[0] ) ->  cv[1] -> v[lascnt]
		rFace[1] = AddRendFace( cv[0] , cv[1] , v[lascnt] , rendface->Matrial , NULL );
		// ( cv[0] ) -> v[nexcnt] -> cv[1]
		rFace[2] = AddRendFace( cv[0] , v[nexcnt] , cv[1] , rendface->Matrial , NULL );

		// 텍스쳐 좌표를 순차적으로 구함
		texLink = rendface->lpTexLink;
		for( cnt=0;cnt<8;cnt++) {
			if ( !texLink ) break;
			u0 = texLink->u[midcnt] + ( texLink->u[nexcnt] - texLink->u[midcnt] ) * fMultPoint[0];
			v0 = texLink->v[midcnt] + ( texLink->v[nexcnt] - texLink->v[midcnt] ) * fMultPoint[0];
			u1 = texLink->u[lascnt] + ( texLink->u[nexcnt] - texLink->u[lascnt] ) * fMultPoint[1];
			v1 = texLink->v[lascnt] + ( texLink->v[nexcnt] - texLink->v[lascnt] ) * fMultPoint[1];

			// ( v[midcnt] ) -> tv[0] -> v[lascnt]
			AddRendTempTexLink( rFace[0] , texLink->hTexture , 
				texLink->u[midcnt] , texLink->v[midcnt] , u0 , v0 , texLink->u[lascnt] , texLink->v[lascnt] );

			// ( tv[0] ) ->  tv[1] -> v[lascnt]
			AddRendTempTexLink( rFace[1] , texLink->hTexture , 
				u0 , v0 , u1 , v1 , texLink->u[lascnt] , texLink->v[lascnt] );

			// ( cv[0] ) -> v[nexcnt] ->  cv[1] 
			AddRendTempTexLink( rFace[2] , texLink->hTexture , 
				u0 , v0 , texLink->u[nexcnt] , texLink->v[nexcnt] , u1 , v1  );

			texLink = texLink->NextTex;
		}

	}
	else {
		//폴리곤 나누기 1개 짜리
		cv[0] = AddRendVertex_ClipTexture( rendface , midcnt, nexcnt , &fMultPoint[0] );
		cv[1] = AddRendVertex_ClipTexture( rendface , midcnt, lascnt , &fMultPoint[1] );

		// 페이스 추가 
		// ( v[midcnt] ) -> cv[0] -> cv[1]
		rFace[0] = AddRendFace( v[midcnt] , cv[0] , cv[1] , rendface->Matrial , NULL );
		// cv[0] -> v[nexcnt] -> v[lascnt]
		rFace[1] = AddRendFace( cv[0] , v[nexcnt] , v[lascnt] , rendface->Matrial , NULL );
		// cv[0] -> v[lascnt] -> cv[1]
		rFace[2] = AddRendFace( cv[0] , v[lascnt] , cv[1] , rendface->Matrial , NULL );


		// 텍스쳐 좌표를 순차적으로 구함
		texLink = rendface->lpTexLink;
		for( cnt=0;cnt<8;cnt++) {
			if ( !texLink ) break;
			u0 = texLink->u[midcnt] + ( texLink->u[nexcnt] - texLink->u[midcnt] ) * fMultPoint[0];
			v0 = texLink->v[midcnt] + ( texLink->v[nexcnt] - texLink->v[midcnt] ) * fMultPoint[0];
			u1 = texLink->u[midcnt] + ( texLink->u[lascnt] - texLink->u[midcnt] ) * fMultPoint[1];
			v1 = texLink->v[midcnt] + ( texLink->v[lascnt] - texLink->v[midcnt] ) * fMultPoint[1];

			// ( v[midcnt] ) -> cv[0] -> cv[1]
			AddRendTempTexLink( rFace[0] , texLink->hTexture , 
				texLink->u[midcnt] , texLink->v[midcnt] , u0 , v0 , u1, v1 );

			// cv[0] -> v[nexcnt] -> v[lascnt]
			AddRendTempTexLink( rFace[1] , texLink->hTexture , 
				u0, v0, texLink->u[nexcnt] , texLink->v[nexcnt] , texLink->u[lascnt] , texLink->v[lascnt] );

			// cv[0] -> v[lascnt] -> cv[1]
			AddRendTempTexLink( rFace[2] , texLink->hTexture , 
				u0, v0, texLink->u[lascnt] , texLink->v[lascnt] , u1, v1 );

			texLink = texLink->NextTex;
		}

	}

	return TRUE;
}


//######################################################################################
//작 성 자 : 오 영 석
int smRENDER3D::SetDynamicObjLight( smRENDVERTEX *sv, int z )
{
	int cnt;

	int sx,sy,sz;
	int dist;
	int range,drange;
	int power;
	int r,g,b,a;
	int drg;

	if( z < ccDistZMin + (6*64) )
	{
		for( cnt=0; cnt < nRendObjLight; cnt++ )
		{
			sx = abs(sv->tx - smRendObjLight[cnt].rx);
			sy = abs(sv->ty - smRendObjLight[cnt].ry);
			sz = abs(sv->tz - smRendObjLight[cnt].rz);
			range = smRendObjLight[cnt].Range;
			drange = smRendObjLight[cnt].dRange;

			if( sx<range && sy<range && sz<range )
			{
				sx>>=FLOATNS;
				sy>>=FLOATNS;
				sz>>=FLOATNS;

				dist = sx*sx+sz*sz+sy*sy;
				if( dist < drange )
				{
					drg = (drange>>FLOATNS);
					if( drg == 0 ) 
						drg = 1;

					power = (drange-dist)/drg;
					r = (smRendObjLight[cnt].r * power)>>FLOATNS;
					g = (smRendObjLight[cnt].g * power)>>FLOATNS;
					b = (smRendObjLight[cnt].b * power)>>FLOATNS;
					a = (smRendObjLight[cnt].a * power)>>FLOATNS;
					AddLight( sv , r, g, b, a );

					if ( sv->sLight[SMC_R]>540 ) sv->sLight[SMC_R] = 540;
					if ( sv->sLight[SMC_G]>540 ) sv->sLight[SMC_G] = 540;
					if ( sv->sLight[SMC_B]>540 ) sv->sLight[SMC_B] = 540;
				}
			}
		}
	}

	return TRUE;
}
//######################################################################################


//int ccDistNearClip = 8*64;

// 다이나믹 라이트 설정
int smRENDER3D::SetDynamicLight( smRENDVERTEX *sv , int z )
{
	int cnt;

	int sx,sy,sz;
	int dist;
	int range,drange;
	int power;
	int r,g,b,a;
	int drg;


	if ( z < ccDistZMin+(6*64) ) {
		for( cnt=0;cnt<nRendLight;cnt++) {
			sx = abs(sv->tx - smRendLight[cnt].rx);
			sy = abs(sv->ty - smRendLight[cnt].ry);
			sz = abs(sv->tz - smRendLight[cnt].rz);
			range = smRendLight[cnt].Range;
			drange = smRendLight[cnt].dRange;

			if ( sx<range && sy<range && sz<range ) {
				sx>>=FLOATNS;
				sy>>=FLOATNS;
				sz>>=FLOATNS;

				dist = sx*sx+sz*sz+sy*sy;
				if ( dist<drange ) {
					drg = (drange>>FLOATNS);
					if ( drg==0 ) drg=1;
					power = (drange-dist)/drg;
					r = (smRendLight[cnt].r * power)>>FLOATNS;
					g = (smRendLight[cnt].g * power)>>FLOATNS;
					b = (smRendLight[cnt].b * power)>>FLOATNS;
					a = (smRendLight[cnt].a * power)>>FLOATNS;
					AddLight( sv , r, g, b, a );

					if ( sv->sLight[SMC_R]>540 ) sv->sLight[SMC_R] = 540;
					if ( sv->sLight[SMC_G]>540 ) sv->sLight[SMC_G] = 540;
					if ( sv->sLight[SMC_B]>540 ) sv->sLight[SMC_B] = 540;

				}
			}
		}
	}

	return TRUE;
}

// Z 값에 따른 색상치를 BackColor값에 가깝도록 조절한다 
int smRENDER3D::SetColorZclip( smRENDVERTEX *sv , int z )
{ 
	if ( z > ccDistZMin ) {
		int result;
		int lev;

		lev = (z-ccDistZMin) << ( 13 - ShiftMAPCELL_MULT );
		result = ShiftLight( sv, lev>>8, ccBackColor.r>>8 , ccBackColor.g>>8, ccBackColor.b>>8 ,0 );
		return result;
	}

	return FALSE;
}

// 버텍스의 색상을 주어진 값에 가깝도록 단계적으로 조절
int smRENDER3D::ShiftLight( smRENDVERTEX *sv , int lev , int r,int g,int b, int a )
{
	int cFlag;
	int dlev;

	int col,spe;

	if ( lev>=256 ) {
		lev=255;
		cFlag = 1;
	}
	else
		cFlag = 0;

	dlev = lev;

	// RED
	col = sv->bCol[0];
	spe = sv->bSpe[0];
	col -= ( col * dlev )>>8;
	spe += ( ( r-spe ) * dlev )>>8;
	sv->bCol[0]=col;
	sv->bSpe[0]=spe;

	// GREEN
	col = sv->bCol[1];
	spe = sv->bSpe[1];
	col -= ( col * dlev )>>8;
	spe += ( ( g-spe ) * dlev )>>8;
	sv->bCol[1]=col;
	sv->bSpe[1]=spe;

	// BLUE
	col = sv->bCol[2];
	spe = sv->bSpe[2];
	col -= ( col * dlev )>>8;
	spe += ( ( b-spe ) * dlev )>>8;
	sv->bCol[2]=col;
	sv->bSpe[2]=spe;

	// ALPHA
	col = sv->bCol[3];
	spe = sv->bSpe[3];
	col -= ( col * dlev )>>8;
	spe += ( ( a-spe ) * dlev )>>8;
	sv->bCol[3]=col;
	sv->bSpe[3]=spe;

	if ( cFlag ) return TRUE;

	return FALSE;
}


//sLight 색상에 더한다
int smRENDER3D::AddLight( smRENDVERTEX *sv, int r,int g,int b , int a )
{

	sv->sLight[ SMC_R ] += r;
	sv->sLight[ SMC_G ] += g;
	sv->sLight[ SMC_B ] += b;
	sv->sLight[ SMC_A ] += a;

	return TRUE;
}

//렌더버텍스의 sLight를 Color 와 Specular 로 설정 한다
int smRENDER3D::SetRendColor( smRENDVERTEX *sv )
{
	int col , spe;
	int cnt;

	for( cnt=0;cnt<4;cnt++ ) {
		col = sv->sLight[ cnt ];
		spe = 0;
		if ( col<0 )	{ col=0; spe=0; }
		if ( col>255 )	{ spe=col-256; col=255; }
		if ( spe>255 )  spe = 255;
		sv->bCol[ cnt ] = col;
		sv->bSpe[ cnt ] = spe;
	}

	return TRUE;
}

//메트리얼 리스트에 목록 추가
int smRENDER3D::AddRendMaterialList( int MatNum )
{

	if ( !RendMatrial[ MatNum ].RegistRendList ) {
		if ( smRendMaterial[MatNum].MapOpacity==TRUE || smRendMaterial[MatNum].Transparency!=0 ) {
			RendMatrialList[ RMatListRearCnt-- ] = MatNum;
		}
		else {
			RendMatrialList[ RMatListCnt++ ] = MatNum;
		}

		RendMatrial[ MatNum ].RegistRendList = TRUE;
	}

	return TRUE;

}

// 전체 렌더링 페이스를 클리핑  
int smRENDER3D::ClipRendFace()
{
	int cnt;
	DWORD ClipStatus;
	smRENDMATRIAL	*rmat;
	int nrface;

	int ClipCnt;
	DWORD ClipValue;
	smTEXTUREHANDLE	*smTex;



	ClipValue = 1;

	for( ClipCnt=0;ClipCnt<6;ClipCnt++) {
		if ( ClipStates&ClipValue ) {
			for( cnt=0; cnt<nRendFace; cnt++ ) {
				ClipStatus = RendFace[ cnt ].ClipStatus;
				if ( !(ClipStatus&SMCLIP_DISPLAYOUT) && (ClipStatus&ClipValue) ) {
					AddRendFaceClip( &RendFace[cnt], ClipValue );
				}
			}
		}
		ClipValue<<=1;



		// FARZ 클립 ( Alpha값 메트리얼을 사용 하는 경우만 해당 되게 했음 )
		if ( ClipValue==SMCLIP_FARZ )
		{
			//######################################################################################
			//작 성 자 : 오 영 석
			if( m_GameFieldView == 0 || m_GameFieldViewStep == 1 )
			{
				for( cnt=0; cnt<nRendFace; cnt++ )
				{
					smTex = smRendMaterial[ RendFace[cnt].Matrial ].smTexture[0];
					if ( smTex && smTex->NameA[0] )
					{
						ClipStatus = RendFace[ cnt ].ClipStatus;
						if ( !(ClipStatus&SMCLIP_DISPLAYOUT) && (ClipStatus&ClipValue) )
							AddRendFaceClip( &RendFace[cnt], ClipValue );
					}
				}
			}
			//######################################################################################
			ClipValue<<=1;
			ClipCnt++;
		}
	}


	// TEXTURE 클리핑 
	nrface = nRendFace;

	for( cnt=0; cnt<nrface; cnt++ ) {
		ClipStatus = RendFace[ cnt ].ClipStatus;
		if ( !(ClipStatus&SMCLIP_DISPLAYOUT) && (ClipStatus&ClipValue) ) {
			if ( smRendMaterial[ RendFace[cnt].Matrial ].MatFrame!=MaterialFrame ) 
				ReSwapMaterial( &smRendMaterial[ RendFace[cnt].Matrial ] );
			//######################################################################################
			//작 성 자 : 오 영 석
			//if ( smRendMaterial[ RendFace[cnt].Matrial ].TextureClip )
			//	AddRendFaceTextureClip( &RendFace[cnt] );
			//######################################################################################
		}
	}

	int MatNum;

	for( cnt=0; cnt<nRendFace; cnt++ ) {
		ClipStatus = RendFace[ cnt ].ClipStatus;
		if ( !(ClipStatus&SMCLIP_DISPLAYOUT) ) {
						//메트리얼 별 렌더링 페이스 연결
						MatNum = RendFace[cnt].Matrial; 
						rmat = &RendMatrial[ MatNum  ];

						//######################################################################################
						//작 성 자 : 오 영 석
						/*if ( RendFace[cnt].ClipStatus&SMCLIP_TEXTURE && smRendMaterial[ MatNum ].TextureClip ) {

							//해당 메트리얼의 페이스카운터 증가
							rmat->TexClip_MatrialCounter ++;
							if ( !rmat->TexClip_LastLinkFace ) {
								//연결된 메트리얼이 없음
								rmat->TexClip_StartFace = &RendFace[cnt];
								rmat->TexClip_LastLinkFace = &RendFace[cnt];
								AddRendMaterialList( MatNum );
							}
							else {
								//기존 메트리얼 페이스와 연결
								rmat->TexClip_LastLinkFace->NexRendFace = &RendFace[cnt];
								rmat->TexClip_LastLinkFace = &RendFace[cnt];
							}

						}
						else*/
						//######################################################################################
						{

							//해당 메트리얼의 페이스카운터 증가
							rmat->MatrialCounter ++;
							if ( !rmat->LastLinkFace ) {
								//연결된 메트리얼이 없음
								rmat->StartFace = &RendFace[cnt];
								rmat->LastLinkFace = &RendFace[cnt];
								AddRendMaterialList( MatNum );
							}
							else {
								//기존 메트리얼 페이스와 연결
								rmat->LastLinkFace->NexRendFace = &RendFace[cnt];
								rmat->LastLinkFace = &RendFace[cnt];
							}
						}


						RendFace[ cnt ].lpRendVertex[0]->ClipStatus |= SMCLIP_DISPLAYIN;
						RendFace[ cnt ].lpRendVertex[1]->ClipStatus |= SMCLIP_DISPLAYIN;
						RendFace[ cnt ].lpRendVertex[2]->ClipStatus |= SMCLIP_DISPLAYIN;
		}
	}


	return TRUE;
}


// 버텍스를 2D좌표로 변환  
int smRENDER3D::GeomVertex2D( int ColorEffect )
{

	//######################################################################################
	//작 성 자 : 오 영 석
	int				 cnt, z;
	smRENDVERTEX	*rv;

	if( ColorEffect == TRUE )
	{
		//색상 값 표현 여부
		for( cnt=0; cnt < nRendVertex; cnt++ )
		{
			rv = &RendVertex[ cnt ];
			//클리핑에 해당하지 않는 버텍스는 2D좌표로 투시변환한다
			if ( rv->ClipStatus & SMCLIP_DISPLAYIN )
			{
				z = int( float(rv->tz) / fONE );
				DxProjectionWork( rv );

				//######################################################################################
				//작 성 자 : 오 영 석
				if( bApplyRendObjLight && nRendObjLight )
					SetDynamicObjLight( rv, z );
				//######################################################################################

				if ( nRendLight ) SetDynamicLight( rv , z );		// 다이나믹 라이트 설정
				AddLight( rv, Color_R , Color_G , Color_B, Color_A );
				SetRendColor( rv );

				SetColorZclip( rv , z);							// 원거리 색상값 변경
			}
		}
	}
	else
	{
		for( cnt=0; cnt < nRendVertex; cnt++ )
		{
			rv = &RendVertex[ cnt ];
			//클리핑에 해당하지 않는 버텍스는 2D좌표로 투시변환한다
			if ( rv->ClipStatus & SMCLIP_DISPLAYIN )
			{
				z = int( float(rv->tz) / fONE );
				DxProjectionWork( rv );

				//######################################################################################
				//작 성 자 : 오 영 석
				if( bApplyRendObjLight && nRendObjLight )
					SetDynamicObjLight( rv, z );
				//######################################################################################

				if ( nRendLight ) SetDynamicLight( rv , z );		// 다이나믹 라이트 설정
				AddLight( rv, Color_R , Color_G , Color_B, Color_A );
				SetRendColor( rv );
			}
		}
	}	
	//######################################################################################


	/*	
	//////////////////////////////////////////////////////////////////////////////////////////////	
	int				cnt;
	int				x,y,z;
	float			fz;
	smRENDVERTEX	*rv;

	if ( ColorEffect==TRUE ) {
		//색상 값 표현 여부
		for( cnt=0;cnt<nRendVertex;cnt++ ) {
			rv = &RendVertex[ cnt ];
			//클리핑에 해당하지 않는 버텍스는 2D좌표로 투시변환한다
			if ( rv->ClipStatus & SMCLIP_DISPLAYIN ) {
				x = rv->tx ;
				y = rv->ty ;
				z = rv->tz ;
				fz = (float)z/fONE;

				rv->xy[0] = MidX + ( (float)( x * viewdistZ ))/ (float)z ;			// 2D X
				rv->xy[1] = MidY - ( (float)( y * viewdistZ ))/ (float)z ;			// 2D Y

				z >>= FLOATNS;
				rv->zb = smConvZ3D( fz );						// Z Buffer Depth
				rv->rhw = 1/fz;									// RHW

				//######################################################################################
				//작 성 자 : 오 영 석
				if( bApplyRendObjLight && nRendObjLight )
					SetDynamicObjLight( rv, z );
				//######################################################################################

				if ( nRendLight ) SetDynamicLight( rv , z );		// 다이나믹 라이트 설정
				AddLight( rv, Color_R , Color_G , Color_B, Color_A );
				SetRendColor( rv );

				SetColorZclip( rv , z);							// 원거리 색상값 변경

			}
		}
	}
	else {
		for( cnt=0;cnt<nRendVertex;cnt++ ) {
			rv = &RendVertex[ cnt ];
			//클리핑에 해당하지 않는 버텍스는 2D좌표로 투시변환한다
			if ( rv->ClipStatus & SMCLIP_DISPLAYIN ) {
				x = rv->tx ;
				y = rv->ty ;
				z = rv->tz ;
				fz = (float)z/fONE;

				rv->xy[0] = MidX + ( (float)( x * viewdistZ ))/ (float)z ;			// 2D X
				rv->xy[1] = MidY - ( (float)( y * viewdistZ ))/ (float)z ;			// 2D Y

				z >>= FLOATNS;
				rv->zb = smConvZ3D( fz );								// Z Buffer Depth
				rv->rhw = 1/fz;											// RHW

				//######################################################################################
				//작 성 자 : 오 영 석
				if( bApplyRendObjLight && nRendObjLight )
					SetDynamicObjLight( rv, z );
				//######################################################################################

				if ( nRendLight ) SetDynamicLight( rv , z );		// 다이나믹 라이트 설정
				AddLight( rv, Color_R , Color_G , Color_B, Color_A );
				SetRendColor( rv );

			}
		}
	}
	//////////////////////////////////////////////////////////////////////////////////////////////
	*/
	

	return TRUE;
}

// 윈쪽을 클리핑점을 구하여 렌더링 버텍스에 추가한다 ( fMultPoint 교점 좌표를 구하기 위한 방정식의 값  ) 
smRENDVERTEX * smRENDER3D::AddRendVertex_ClipLeft( smRENDFACE *rendface , int a, int b , float *fMultPoint )
{
	int		x1 , x2;
	int		z1 , z2;
	int		s1 , s2;
	float	fMult;
	int		Mult;
	int		cnt;

	int		x,y,z;
	short	sCol[4];

	smRENDVERTEX *t1;
	smRENDVERTEX *t2;


	// 45도 각의 선분과 만나는 점을 방정식을 통하여 구함 
	t1 = rendface->lpRendVertex[a];
	t2 = rendface->lpRendVertex[b];

	x1 = t1->tx   ;
	z1 = -(x1<<SMSHIFT_PERSPECTIVE_WIDTH) ;
	x2 = t2->tx   ;
	z2 = -(x2<<SMSHIFT_PERSPECTIVE_WIDTH) ;

	s1 = abs( t1->tz - z1 );
	s2 = abs( t2->tz - z2 );

	// 방정식을 통해 구한 교점의 값
	fMult = (float)((1/((double)(s2 + s1)))*(double)s1);
	Mult  = (int)( fMult * smCLIP_DETAIL );

	// 교점의 좌표 
	x = t1->tx + ( ( ( t2->tx - t1->tx ) * Mult ) >> smCLIP_DETAIL_SHIFT );
	y = t1->ty + ( ( ( t2->ty - t1->ty ) * Mult ) >> smCLIP_DETAIL_SHIFT );
	z = (-x)<<SMSHIFT_PERSPECTIVE_WIDTH;

	// 교점의 색상값 
	for( cnt=0;cnt<4;cnt++ ) {
		sCol[cnt] = (short)((int)t1->sLight[cnt] + (( ( (int)t2->sLight[cnt] - (int)t1->sLight[cnt] ) * Mult )>>smCLIP_DETAIL_SHIFT ) );
	}

/*
	x = t1->tx + (int)( ( (double)( t2->tx - t1->tx ) * fMult ));
	y = t1->ty + (int)( ( (double)( t2->ty - t1->ty ) * fMult ));
	z = (-x)<<SMSHIFT_PERSPECTIVE_WIDTH;
*/

	*fMultPoint = fMult;

	//######################################################################################
	//작 성 자 : 오 영 석
	// 교점의 버텍스를 렌더링 버텍스에 추가
	//return AddRendVertex ( x, y, z, sCol );
	int nx = t2->nx - t2->tx;
	int ny = t2->ny - t2->ty;
	int nz = t2->nz - t2->tz;
	return AddRendVertex ( x, y, z, sCol, x+nx, y+ny, z+nz );
	//######################################################################################

}

// 윈쪽을 클리핑점을 구하여 렌더링 버텍스에 추가한다 ( fMultPoint 교점 좌표를 구하기 위한 방정식의 값  ) 
smRENDVERTEX * smRENDER3D::AddRendVertex_ClipRight( smRENDFACE *rendface , int a, int b , float *fMultPoint )
{
	int		x1 , x2;
	int		z1 , z2;
	int		s1 , s2;
	float	fMult;
	int		Mult;
	int		cnt;

	int		x,y,z;
	short	sCol[4];

	smRENDVERTEX *t1;
	smRENDVERTEX *t2;


	// 45도 각의 선분과 만나는 점을 방정식을 통하여 구함 
	t1 = rendface->lpRendVertex[a];
	t2 = rendface->lpRendVertex[b];

	x1 = t1->tx   ;
	z1 = (x1<<SMSHIFT_PERSPECTIVE_WIDTH) ;
	x2 = t2->tx   ;
	z2 = (x2<<SMSHIFT_PERSPECTIVE_WIDTH) ;

	s1 = abs( t1->tz - z1 );
	s2 = abs( t2->tz - z2 );

	// 방정식을 통해 구한 교점의 값
	fMult = (float)((1/((double)(s2 + s1)))*(double)s1);
	Mult  = (int)( fMult * smCLIP_DETAIL );

	// 교점의 좌표 
	x = t1->tx + ( ( ( t2->tx - t1->tx ) * Mult ) >> smCLIP_DETAIL_SHIFT );
	y = t1->ty + ( ( ( t2->ty - t1->ty ) * Mult ) >> smCLIP_DETAIL_SHIFT );
	z = x<<SMSHIFT_PERSPECTIVE_WIDTH ;

	// 교점의 색상값 
	for( cnt=0;cnt<4;cnt++ ) {
		sCol[cnt] = (short)((int)t1->sLight[cnt] + (( ( (int)t2->sLight[cnt] - (int)t1->sLight[cnt] ) * Mult )>>smCLIP_DETAIL_SHIFT) );
	}

/*
	x = t1->tx + ( ( (double)( t2->tx - t1->tx ) * fMult ) );
	y = t1->ty + ( ( (double)( t2->ty - t1->ty ) * fMult ) );
	z = x<<SMSHIFT_PERSPECTIVE_WIDTH ;
*/

	*fMultPoint = fMult;

	//######################################################################################
	//작 성 자 : 오 영 석
	// 교점의 버텍스를 렌더링 버텍스에 추가
	//return AddRendVertex ( x, y, z, sCol );
	int nx = t2->nx - t2->tx;
	int ny = t2->ny - t2->ty;
	int nz = t2->nz - t2->tz;
	return AddRendVertex ( x, y, z, sCol, x+nx, y+ny, z+nz );
	//######################################################################################
}
	
// 아래쪽 클리핑점을 구하여 렌더링 버텍스에 추가한다 ( fMultPoint 교점 좌표를 구하기 위한 방정식의 값  ) 
smRENDVERTEX * smRENDER3D::AddRendVertex_ClipBottom( smRENDFACE *rendface , int a, int b , float *fMultPoint )
{
	int		y1 , y2;
	int		z1 , z2;
	int		s1 , s2;
	float	fMult;
	int		Mult;
	int		cnt;

	int		x,y,z;
	short	sCol[4];

	smRENDVERTEX *t1;
	smRENDVERTEX *t2;



	// 45도 각의 선분과 만나는 점을 방정식을 통하여 구함 
	t1 = rendface->lpRendVertex[a];
	t2 = rendface->lpRendVertex[b];

	y1 = t1->ty   ;
	z1 = -( ( y1 * SMMULT_PERSPECTIVE_HEIGHT ) >> FLOATNS );
	y2 = t2->ty   ;
	z2 = -( ( y2 * SMMULT_PERSPECTIVE_HEIGHT ) >> FLOATNS );

	s1 = abs( t1->tz - z1 );
	s2 = abs( t2->tz - z2 );

	// 방정식을 통해 구한 교점의 값
	fMult = (float)((1/((double)(s2 + s1)))*(double)s1);
//	Mult  = (int)( fMult * fONE );
	Mult  = (int)( fMult * smCLIP_DETAIL );

	// 교점의 좌표 
	x = t1->tx + ( ( ( t2->tx - t1->tx ) * Mult ) >> smCLIP_DETAIL_SHIFT );
	y = t1->ty + ( ( ( t2->ty - t1->ty ) * Mult ) >> smCLIP_DETAIL_SHIFT );
	z = ((-y) * SMMULT_PERSPECTIVE_HEIGHT ) >> FLOATNS;

	// 교점의 색상값 
	for( cnt=0;cnt<4;cnt++ ) {
		sCol[cnt] = (short)((int)t1->sLight[cnt] + (( ( (int)t2->sLight[cnt] - (int)t1->sLight[cnt] ) * Mult )>>smCLIP_DETAIL_SHIFT ) );
	}

/*
	x = t1->tx + (int)( ( (double)( t2->tx - t1->tx ) * fMult )  );
	y = t1->ty + (int)( ( (double)( t2->ty - t1->ty ) * fMult )  );
	z = ((-y) * SMMULT_PERSPECTIVE_HEIGHT ) >> FLOATNS;
*/


	*fMultPoint = fMult;

	//######################################################################################
	//작 성 자 : 오 영 석
	// 교점의 버텍스를 렌더링 버텍스에 추가
	//return AddRendVertex ( x, y, z, sCol );
	int nx = t2->nx - t2->tx;
	int ny = t2->ny - t2->ty;
	int nz = t2->nz - t2->tz;
	return AddRendVertex ( x, y, z, sCol, x+nx, y+ny, z+nz );
	//######################################################################################
}

// 위쪽 클리핑점을 구하여 렌더링 버텍스에 추가한다 ( fMultPoint 교점 좌표를 구하기 위한 방정식의 값  ) 
smRENDVERTEX * smRENDER3D::AddRendVertex_ClipTop( smRENDFACE *rendface , int a, int b , float *fMultPoint )
{
	int		y1 , y2;
	int		z1 , z2;
	int		s1 , s2;
	float	fMult;
	int		Mult;
	int		cnt;

	int		x,y,z;
	short	sCol[4];

	smRENDVERTEX *t1;
	smRENDVERTEX *t2;



	// 45도 각의 선분과 만나는 점을 방정식을 통하여 구함 
	t1 = rendface->lpRendVertex[a];
	t2 = rendface->lpRendVertex[b];

	y1 = t1->ty   ;
	z1 = ( ( y1 * SMMULT_PERSPECTIVE_HEIGHT ) >> FLOATNS );
	y2 = t2->ty   ;
	z2 = ( ( y2 * SMMULT_PERSPECTIVE_HEIGHT ) >> FLOATNS );

	s1 = abs( t1->tz - z1 );
	s2 = abs( t2->tz - z2 );

	// 방정식을 통해 구한 교점의 값
	fMult = (float)((1/((double)(s2 + s1)))*(double)s1);
//	Mult  = (int)( fMult * fONE );
	Mult  = (int)( fMult * smCLIP_DETAIL );

	// 교점의 좌표 
	x = t1->tx + ( ( ( t2->tx - t1->tx ) * Mult ) >> smCLIP_DETAIL_SHIFT );
	y = t1->ty + ( ( ( t2->ty - t1->ty ) * Mult ) >> smCLIP_DETAIL_SHIFT );
	z = y<<SMSHIFT_PERSPECTIVE_WIDTH;
	z = ( y * SMMULT_PERSPECTIVE_HEIGHT ) >> FLOATNS;

/*
	x = t1->tx + (int)( ( (double)( t2->tx - t1->tx ) * fMult ) );
	y = t1->ty + (int)( ( (double)( t2->ty - t1->ty ) * fMult ) );
	z = y<<SMSHIFT_PERSPECTIVE_WIDTH;
	z = ( y * SMMULT_PERSPECTIVE_HEIGHT ) >> FLOATNS;
*/

	// 교점의 색상값 
	for( cnt=0;cnt<4;cnt++ ) {
		sCol[cnt] = (short)((int)t1->sLight[cnt] + (( ( (int)t2->sLight[cnt] - (int)t1->sLight[cnt] ) * Mult )>>smCLIP_DETAIL_SHIFT ) );
	}

	*fMultPoint = fMult;

	//######################################################################################
	//작 성 자 : 오 영 석
	// 교점의 버텍스를 렌더링 버텍스에 추가
	//return AddRendVertex ( x, y, z, sCol );
	int nx = t2->nx - t2->tx;
	int ny = t2->ny - t2->ty;
	int nz = t2->nz - t2->tz;
	return AddRendVertex ( x, y, z, sCol, x+nx, y+ny, z+nz );
	//######################################################################################
}


// Z NEAR 클리핑점을 구하여 렌더링 버텍스에 추가한다 ( fMultPoint 교점 좌표를 구하기 위한 방정식의 값  ) 
smRENDVERTEX * smRENDER3D::AddRendVertex_ClipNearZ( smRENDFACE *rendface , int a, int b , float *fMultPoint )
{
	int		s1 , s2;
	float	fMult;
	int		Mult;
	int		cnt;

	int		x,y,z;
	short	sCol[4];

	smRENDVERTEX *t1;
	smRENDVERTEX *t2;



	// 45도 각의 선분과 만나는 점을 방정식을 통하여 구함 
	t1 = rendface->lpRendVertex[a];
	t2 = rendface->lpRendVertex[b];

	s1 = abs( t1->tz - SMFLOAT_DIST_NEARZ );
	s2 = abs( t2->tz - SMFLOAT_DIST_NEARZ );

	// 방정식을 통해 구한 교점의 값
	fMult = (float)((1/((double)(s2 + s1)))*(double)s1);
//	Mult  = (int)( fMult * fONE );
	Mult  = (int)( fMult * smCLIP_DETAIL );

	// 교점의 좌표 
	x = t1->tx + ( ( ( t2->tx - t1->tx ) * Mult ) >> smCLIP_DETAIL_SHIFT );
	y = t1->ty + ( ( ( t2->ty - t1->ty ) * Mult ) >> smCLIP_DETAIL_SHIFT );
	z = SMFLOAT_DIST_NEARZ;

	// 교점의 색상값 
	for( cnt=0;cnt<4;cnt++ ) {
		sCol[cnt] = (short)((int)t1->sLight[cnt] + (( ( (int)t2->sLight[cnt] - (int)t1->sLight[cnt] ) * Mult )>>smCLIP_DETAIL_SHIFT ) );
	}

	*fMultPoint = fMult;

	//######################################################################################
	//작 성 자 : 오 영 석
	// 교점의 버텍스를 렌더링 버텍스에 추가
	//return AddRendVertex ( x, y, z, sCol );
	int nx = t2->nx - t2->tx;
	int ny = t2->ny - t2->ty;
	int nz = t2->nz - t2->tz;
	return AddRendVertex ( x, y, z, sCol, x+nx, y+ny, z+nz );
	//######################################################################################
}


// Z FAR 클리핑점을 구하여 렌더링 버텍스에 추가한다 ( fMultPoint 교점 좌표를 구하기 위한 방정식의 값  ) 
smRENDVERTEX * smRENDER3D::AddRendVertex_ClipFarZ( smRENDFACE *rendface , int a, int b , float *fMultPoint )
{
	int		s1 , s2;
	float	fMult;
	int		Mult;
	int		cnt;

	int		x,y,z;
	short	sCol[4];

	smRENDVERTEX *t1;
	smRENDVERTEX *t2;


	t1 = rendface->lpRendVertex[a];
	t2 = rendface->lpRendVertex[b];

	s1 = abs( t1->tz - SMFLOAT_DIST_FARZ );
	s2 = abs( t2->tz - SMFLOAT_DIST_FARZ );

	// 방정식을 통해 구한 교점의 값
	fMult = (float)((1/((double)(s2 + s1)))*(double)s1);
	Mult  = (int)( fMult * smCLIP_DETAIL );

	// 교점의 좌표 
	x = t1->tx + ( ( ( t2->tx - t1->tx ) * Mult ) >> smCLIP_DETAIL_SHIFT );
	y = t1->ty + ( ( ( t2->ty - t1->ty ) * Mult ) >> smCLIP_DETAIL_SHIFT );
	z = SMFLOAT_DIST_FARZ;

	// 교점의 색상값 
	for( cnt=0;cnt<4;cnt++ ) {
		sCol[cnt] = (short)((int)t1->sLight[cnt] + (( ( (int)t2->sLight[cnt] - (int)t1->sLight[cnt] ) * Mult )>>smCLIP_DETAIL_SHIFT ) );
	}

	*fMultPoint = fMult;

	//######################################################################################
	//작 성 자 : 오 영 석
	// 교점의 버텍스를 렌더링 버텍스에 추가
	//return AddRendVertex ( x, y, z, sCol );
	int nx = t2->nx - t2->tx;
	int ny = t2->ny - t2->ty;
	int nz = t2->nz - t2->tz;
	return AddRendVertex ( x, y, z, sCol, x+nx, y+ny, z+nz );
	//######################################################################################
}


// TEXTURE 클리핑점을 구하여 렌더링 버텍스에 추가한다 ( fMultPoint 교점 좌표를 구하기 위한 방정식의 값  ) 
smRENDVERTEX * smRENDER3D::AddRendVertex_ClipTexture( smRENDFACE *rendface , int a, int b , float *fMultPoint )
{
	int		s1 , s2;
	float	fMult;
	int		Mult;
	int		cnt;

	int		x,y,z;
	short	sCol[4];

	smRENDVERTEX *t1;
	smRENDVERTEX *t2;



	// 45도 각의 선분과 만나는 점을 방정식을 통하여 구함 
	t1 = rendface->lpRendVertex[a];
	t2 = rendface->lpRendVertex[b];

	s1 = abs( t1->tz - SMFLOAT_DIST_TEXCLIP );
	s2 = abs( t2->tz - SMFLOAT_DIST_TEXCLIP );

	// 방정식을 통해 구한 교점의 값
	fMult = (float)((1/((double)(s2 + s1)))*(double)s1);
	Mult  = (int)( fMult * smCLIP_DETAIL );

	// 교점의 좌표 
	x = t1->tx + ( ( ( t2->tx - t1->tx ) * Mult ) >> smCLIP_DETAIL_SHIFT );
	y = t1->ty + ( ( ( t2->ty - t1->ty ) * Mult ) >> smCLIP_DETAIL_SHIFT );
	z = SMFLOAT_DIST_TEXCLIP;

	// 교점의 색상값 
	for( cnt=0;cnt<4;cnt++ ) {
		sCol[cnt] = (short)((int)t1->sLight[cnt] + (( ( (int)t2->sLight[cnt] - (int)t1->sLight[cnt] ) * Mult )>>smCLIP_DETAIL_SHIFT ) );
	}

	*fMultPoint = fMult;

	// 교점의 버텍스를 렌더링 버텍스에 추가
	return AddRendVertex ( x, y, z, sCol );

}

// D3D 렌더링을 위한 버퍼값 설정
int smRENDER3D::SetD3DRendBuff( D3DVALUE *RenderBuff , int matrial , int TexClip )
{
	smRENDFACE *rendface;
	smRENDVERTEX *rv;
	smTEXLINK *tl;
	int cnt;
	int vcnt , tcnt , mcnt;
	int rendcnt;

	BYTE	dwSpecular[4];
	BYTE	dwColor[4];

//	int x,y,z;
//	float fx,fy,fz;

		int wtime = (RendStatTime>>6)&0xFF;
		float fwtime = (float)wtime / 256;


	if ( TexClip==TRUE ) {
		//텍스쳐 클립핑 큰 텍스쳐 ( 근거리용 )
		rendface = RendMatrial[ matrial ].TexClip_StartFace;
		mcnt = RendMatrial[ matrial ].TexClip_MatrialCounter;
	}
	else {
		//일반적인 작은 텍스쳐 ( 원거리용 )
		rendface = RendMatrial[ matrial ].StartFace;
		mcnt = RendMatrial[ matrial ].MatrialCounter;
	}

	rendcnt = 0;

	for ( cnt=0; cnt<mcnt; cnt++ ) {

		if ( !(rendface->ClipStatus&SMCLIP_DISPLAYOUT) ) {
			for( vcnt=0;vcnt<3;vcnt++ ) {

				rv = rendface->lpRendVertex[vcnt];

				*(RenderBuff++) = (D3DVALUE)rv->xy[0];
				*(RenderBuff++) = (D3DVALUE)rv->xy[1];
				*(RenderBuff++) = (D3DVALUE)rv->zb;
				*(RenderBuff++) = (D3DVALUE)rv->rhw;


				if ( RendAlphaFlag && rv->tz>(ccDistZMin<<FLOATNS) ) {
					*((DWORD *)dwColor) = *((DWORD *)rv->bCol);
					*((DWORD *)dwSpecular) = *((DWORD *)rv->bSpe);
					dwSpecular[3] = dwSpecular[3]/2 + dwSpecular[3]/3;
					dwColor[3] = dwColor[3]/2 + dwColor[3]/3;

					*((DWORD *)(RenderBuff++)) = *((DWORD *)dwColor) ;
					*((DWORD *)(RenderBuff++)) = *((DWORD *)dwSpecular) ;
				}
				else {
					*((DWORD *)(RenderBuff++)) = *((DWORD *)rv->bCol) ;
					*((DWORD *)(RenderBuff++)) = *((DWORD *)rv->bSpe) ;
				}
/*
#define smTEXSTATE_FS_SCROLL2	6
#define smTEXSTATE_FS_SCROLL3	7
#define smTEXSTATE_FS_SCROLL4	8
#define smTEXSTATE_FS_SCROLL5	9
#define smTEXSTATE_FS_SCROLL6	10
#define smTEXSTATE_FS_SCROLL7	11
#define smTEXSTATE_FS_SCROLL8	12
#define smTEXSTATE_FS_SCROLL9	13
#define smTEXSTATE_FS_SCROLL10	14
*/

				tl = rendface->lpTexLink;
				for ( tcnt=0;tcnt<8;tcnt++ )
				{
					//######################################################################################
					// 작 성 자 : 오 영 석					
					if( tl == NULL )
					{
						if( tcnt == 1 && s_lpItem2PassTexHandle )
							tl = rendface->lpTexLink;
						else
                            break;
					}
					//if ( !tl ) break;
					//######################################################################################

					if ( TextureFormState[tcnt] ) {
						switch( TextureFormState[tcnt] ) {

						case smTEXSTATE_FS_FORMX:
							/*
							z = rv->wz- TexPosiZ;
							y = rv->wy- TexPosiY;
	
							z = ( (z<<FLOATNS) / TexSizeZ );
							fz = ( (float)z) /fONE;
							y = ( (y<<FLOATNS) / TexSizeY );
							fy = -( (float)y )/fONE;

							fz -= 0.5;
							fy -= 0.5;

							*(RenderBuff++) = (D3DVALUE)fz;
							*(RenderBuff++) = (D3DVALUE)fy;
							break;
							*/
						case smTEXSTATE_FS_FORMY:
						case smTEXSTATE_FS_FORMZ:

						case smTEXSTATE_FS_NONE:
							//변형 없음
							*(RenderBuff++) = (D3DVALUE)tl->u[vcnt];
							*(RenderBuff++) = (D3DVALUE)tl->v[vcnt];
							break;

						case smTEXSTATE_FS_SCROLL:
							//스크롤
							*(RenderBuff++) = (D3DVALUE)(tl->u[vcnt]+fwtime);
							*(RenderBuff++) = (D3DVALUE)(tl->v[vcnt]);//+fwtime);					
							break;

						case smTEXSTATE_FS_REFLEX:
							//*(RenderBuff++) = (D3DVALUE)( tl->u[vcnt] + ((float)(rv->tx+(rv->tz>>12)))/65536) ;
							//*(RenderBuff++) = (D3DVALUE)( tl->v[vcnt] + ((float)(rv->ty+(rv->tz>>12)))/65536) ;
							//기존에 원본. *(RenderBuff++) = (D3DVALUE)( tl->u[vcnt] + ((float)(rv->tx>>1)/65536)) ;
							//기존에 원본. *(RenderBuff++) = (D3DVALUE)( tl->v[vcnt] + ((float)(rv->ty>>2)/65536)) ;
							//*(RenderBuff++) = (D3DVALUE)tl->v[vcnt];
							//*(RenderBuff++) = (D3DVALUE)( tl->v[vcnt] + ((float)(rv->ty)/65536)) ;
							//######################################################################################
							//작 성 자 : 오 영 석
							{
								float fEx  = float(rv->tx) / fONE;
								float fEy  = float(rv->ty) / fONE;
								float fEz  = float(rv->tz) / fONE;
								float fLen = (float)sqrt( (fEx*fEx) + (fEy*fEy) + (fEz*fEz) );		
								fEx /= fLen;
								fEy /= fLen;
								fEz /= fLen;		
			
								float fNx = float(rv->nx - rv->tx) / 32767.f;
								float fNy = float(rv->ny - rv->ty) / 32767.f;
								float fNz = float(rv->nz - rv->tz) / 32767.f;

								float fDotNum = (fEx*fNx) + (fEy*fNy) + (fEz*fNz);
								float fx = fEx - (fDotNum * fNx * 2);
								float fy = fEy - (fDotNum * fNy * 2);
								
								*(RenderBuff++) = (fx * 0.5f) + 0.5f;
								*(RenderBuff++) = (-fy * 0.5f) + 0.5f;
							}
							//######################################################################################
							break;

						}

						if ( TextureFormState[tcnt]>=smTEXSTATE_FS_SCROLL2 && TextureFormState[tcnt]<=smTEXSTATE_FS_SCROLL10 ) {
							//스크롤 ( 배속 2~10 )
							*(RenderBuff++) = (D3DVALUE)(tl->u[vcnt]+fwtime* (TextureFormState[tcnt]-(smTEXSTATE_FS_SCROLL2-2)) );
							*(RenderBuff++) = (D3DVALUE)(tl->v[vcnt]);
						}
						//######################################################################################
						//작 성 자 : 동동동
						else if( TextureFormState[tcnt]>=smTEXSTATE_FS_SCROLLSLOW1 && TextureFormState[tcnt]<=smTEXSTATE_FS_SCROLLSLOW4) //동동동 추가..
						{
 							int factor = smTEXSTATE_FS_SCROLLSLOW4 - TextureFormState[tcnt] + 4;
							wtime  = (RendStatTime >> 6) & (0xFFFF >> factor);

							fwtime = float(wtime) / float(0xFFFF >> factor);
							*(RenderBuff++) = (D3DVALUE)(tl->u[vcnt] + fwtime);
							*(RenderBuff++) = (D3DVALUE)(tl->v[vcnt]);
						}
						//######################################################################################
					}
					else {
						//변형 없음
						*(RenderBuff++) = (D3DVALUE)tl->u[vcnt];
						*(RenderBuff++) = (D3DVALUE)tl->v[vcnt];
					}

					tl = tl->NextTex;
				}
				rendcnt++;
			}
		}
		rendface = rendface->NexRendFace;
	}

	return rendcnt;
}


// D3D 렌더링을 위한 버퍼값 설정
int smRENDER3D::SetD3DRendBuff2( D3DVALUE *RenderBuff , int matrial , int TexClip )
{
	smRENDFACE *rendface;
	smRENDVERTEX *rv;
	smTEXLINK *tl;
	int cnt;
	int vcnt , tcnt , mcnt;
	int rendcnt;

	int wtime = (RendStatTime>>6)&0xFF;
	float fwtime = (float)wtime / 256;


	if ( TexClip==TRUE ) {
		//텍스쳐 클립핑 큰 텍스쳐 ( 근거리용 )
		rendface = RendMatrial[ matrial ].TexClip_StartFace;
		mcnt = RendMatrial[ matrial ].TexClip_MatrialCounter;
	}
	else {
		//일반적인 작은 텍스쳐 ( 원거리용 )
		rendface = RendMatrial[ matrial ].StartFace;
		mcnt = RendMatrial[ matrial ].MatrialCounter;
	}

	rendcnt = 0;

	for ( cnt=0; cnt<mcnt; cnt++ ) {

		if ( !(rendface->ClipStatus&SMCLIP_DISPLAYOUT) ) {
			for( vcnt=0;vcnt<3;vcnt++ ) {

				rv = rendface->lpRendVertex[vcnt];

				*(RenderBuff++) = (D3DVALUE)rv->xy[0];
				*(RenderBuff++) = (D3DVALUE)rv->xy[1];
				*(RenderBuff++) = (D3DVALUE)rv->zb;
				*(RenderBuff++) = (D3DVALUE)rv->rhw;

				*((DWORD *)(RenderBuff++)) = (*((DWORD *)rv->bCol))|0x00FFFFFF;
				*((DWORD *)(RenderBuff++)) = 0x00000000;

				tl = rendface->lpTexLink;
				for ( tcnt=0;tcnt<8;tcnt++ )
				{
					//######################################################################################
					// 작 성 자 : 오 영 석
					if( tl == NULL )
					{
						if( tcnt == 1 && s_lpItem2PassTexHandle )
							tl = rendface->lpTexLink;
						else
                            break;
					}
					//if ( !tl ) break;
					//######################################################################################					

					

					//////////////////////////////////////////////////////////////////
					if ( TextureFormState[tcnt] ) {
						switch( TextureFormState[tcnt] ) {

						case smTEXSTATE_FS_FORMX:
							/*
							z = rv->wz- TexPosiZ;
							y = rv->wy- TexPosiY;
	
							z = ( (z<<FLOATNS) / TexSizeZ );
							fz = ( (float)z) /fONE;
							y = ( (y<<FLOATNS) / TexSizeY );
							fy = -( (float)y )/fONE;

							fz -= 0.5;
							fy -= 0.5;

							*(RenderBuff++) = (D3DVALUE)fz;
							*(RenderBuff++) = (D3DVALUE)fy;
							break;
							*/
						case smTEXSTATE_FS_FORMY:
						case smTEXSTATE_FS_FORMZ:

						case smTEXSTATE_FS_NONE:
							//변형 없음
							*(RenderBuff++) = (D3DVALUE)tl->u[vcnt];
							*(RenderBuff++) = (D3DVALUE)tl->v[vcnt];
							break;

						case smTEXSTATE_FS_SCROLL:
							//스크롤
							*(RenderBuff++) = (D3DVALUE)(tl->u[vcnt]+fwtime);
							*(RenderBuff++) = (D3DVALUE)(tl->v[vcnt]);//+fwtime);					
							break;

						case smTEXSTATE_FS_REFLEX:
							//*(RenderBuff++) = (D3DVALUE)( tl->u[vcnt] + ((float)(rv->tx+(rv->tz>>12)))/65536) ;
							//*(RenderBuff++) = (D3DVALUE)( tl->v[vcnt] + ((float)(rv->ty+(rv->tz>>12)))/65536) ;
							//기존에 원본. *(RenderBuff++) = (D3DVALUE)( tl->u[vcnt] + ((float)(rv->tx>>1)/65536)) ;
							//기존에 원본. *(RenderBuff++) = (D3DVALUE)( tl->v[vcnt] + ((float)(rv->ty>>2)/65536)) ;
							//*(RenderBuff++) = (D3DVALUE)tl->v[vcnt];
							//*(RenderBuff++) = (D3DVALUE)( tl->v[vcnt] + ((float)(rv->ty)/65536)) ;
							//######################################################################################
							//작 성 자 : 오 영 석
							{
								float fEx  = float(rv->tx) / fONE;
								float fEy  = float(rv->ty) / fONE;
								float fEz  = float(rv->tz) / fONE;
								float fLen = (float)sqrt( (fEx*fEx) + (fEy*fEy) + (fEz*fEz) );		
								fEx /= fLen;
								fEy /= fLen;
								fEz /= fLen;		
			
								float fNx = float(rv->nx - rv->tx) / 32767.f;
								float fNy = float(rv->ny - rv->ty) / 32767.f;
								float fNz = float(rv->nz - rv->tz) / 32767.f;

								float fDotNum = (fEx*fNx) + (fEy*fNy) + (fEz*fNz);
								float fx = fEx - (fDotNum * fNx * 2);
								float fy = fEy - (fDotNum * fNy * 2);
								
								*(RenderBuff++) = (fx * 0.5f) + 0.5f;
								*(RenderBuff++) = (-fy * 0.5f) + 0.5f;
							}
							//######################################################################################
							break;
						}

						if ( TextureFormState[tcnt]>=smTEXSTATE_FS_SCROLL2 && TextureFormState[tcnt]<=smTEXSTATE_FS_SCROLL10 ) {
							//스크롤 ( 배속 2~10 )
							*(RenderBuff++) = (D3DVALUE)(tl->u[vcnt]+fwtime* (TextureFormState[tcnt]-(smTEXSTATE_FS_SCROLL2-2)) );
							*(RenderBuff++) = (D3DVALUE)(tl->v[vcnt]);
						}
						//######################################################################################
						//작 성 자 : 동동동
						else if( TextureFormState[tcnt]>=smTEXSTATE_FS_SCROLLSLOW1 && TextureFormState[tcnt]<=smTEXSTATE_FS_SCROLLSLOW4) //동동동 추가..
						{
 							int factor = smTEXSTATE_FS_SCROLLSLOW4 - TextureFormState[tcnt] + 4;
							wtime  = (RendStatTime >> 6) & (0xFFFF >> factor);

							fwtime = float(wtime) / float(0xFFFF >> factor);
							*(RenderBuff++) = (D3DVALUE)(tl->u[vcnt] + fwtime);
							*(RenderBuff++) = (D3DVALUE)(tl->v[vcnt]);
						}
						//######################################################################################
					}
					else {
						//변형 없음
						*(RenderBuff++) = (D3DVALUE)tl->u[vcnt];
						*(RenderBuff++) = (D3DVALUE)tl->v[vcnt];
					}
					//////////////////////////////////////////////////////////////////

					//변형 없음
					//*(RenderBuff++) = (D3DVALUE)tl->u[vcnt];
					//*(RenderBuff++) = (D3DVALUE)tl->v[vcnt];

					tl = tl->NextTex;
				}
				rendcnt++;
			}
		}
		rendface = rendface->NexRendFace;
	}

	return rendcnt;
}


//######################################################################################
// 작 성 자 : 오 영 석
// D3D 렌더링을 위한 버퍼값 설정  : smD3D 버전. 맵 특수 효과 설정
int smRENDER3D::SetD3DRendBuff3( D3DVALUE *RenderBuff , int matrial , int TexClip, short sColors[4] )
{
	smRENDFACE *rendface;
	smRENDVERTEX *rv;
	smTEXLINK *tl;
	int cnt;
	int vcnt , tcnt , mcnt;
	int rendcnt;

	BYTE	dwSpecular[4];
	BYTE	dwColor[4];

	int wtime = (RendStatTime>>6)&0xFF;
	float fwtime = (float)wtime / 256;

	if ( TexClip==TRUE ) {
		//텍스쳐 클립핑 큰 텍스쳐 ( 근거리용 )
		rendface = RendMatrial[ matrial ].TexClip_StartFace;
		mcnt = RendMatrial[ matrial ].TexClip_MatrialCounter;
	}
	else {
		//일반적인 작은 텍스쳐 ( 원거리용 )
		rendface = RendMatrial[ matrial ].StartFace;
		mcnt = RendMatrial[ matrial ].MatrialCounter;
	}

	rendcnt = 0;

	for ( cnt=0; cnt<mcnt; cnt++ )
	{
		if ( !(rendface->ClipStatus&SMCLIP_DISPLAYOUT) )
		{
			for( vcnt=0;vcnt<3;vcnt++ )
			{

				rv = rendface->lpRendVertex[vcnt];

				*(RenderBuff++) = (D3DVALUE)rv->xy[0];
				*(RenderBuff++) = (D3DVALUE)rv->xy[1];
				*(RenderBuff++) = (D3DVALUE)rv->zb;
				*(RenderBuff++) = (D3DVALUE)rv->rhw;


				if ( RendAlphaFlag && rv->tz>(ccDistZMin<<FLOATNS) )
				{
					*((DWORD *)dwColor) = *((DWORD *)rv->bCol);
					*((DWORD *)dwSpecular) = *((DWORD *)rv->bSpe);
					dwSpecular[3] = dwSpecular[3]/2 + dwSpecular[3]/3;
					dwColor[3] = dwColor[3]/2 + dwColor[3]/3;

					*((DWORD *)(RenderBuff++)) = *((DWORD *)dwColor) ;
					*((DWORD *)(RenderBuff++)) = *((DWORD *)dwSpecular) ;
				}
				else
				{
					if( SetBlinkColorVertex(rv, sColors, dwColor, dwSpecular) )
					{
                        *((DWORD *)(RenderBuff++)) = *((DWORD *)dwColor) ;
                        *((DWORD *)(RenderBuff++)) = *((DWORD *)dwSpecular) ;
					}
					else
					{
						*((DWORD *)(RenderBuff++)) = *((DWORD *)rv->bCol) ;
						*((DWORD *)(RenderBuff++)) = *((DWORD *)rv->bSpe) ;
					}
				}

				tl = rendface->lpTexLink;
				for ( tcnt=0;tcnt<8;tcnt++ )
				{
					if ( !tl ) break;
					if ( TextureFormState[tcnt] ) {
						switch( TextureFormState[tcnt] ) {

						case smTEXSTATE_FS_FORMX:
						case smTEXSTATE_FS_FORMY:
						case smTEXSTATE_FS_FORMZ:

						case smTEXSTATE_FS_NONE:
							//변형 없음
							*(RenderBuff++) = (D3DVALUE)tl->u[vcnt];
							*(RenderBuff++) = (D3DVALUE)tl->v[vcnt];
							break;

						case smTEXSTATE_FS_SCROLL:
							//스크롤
							*(RenderBuff++) = (D3DVALUE)(tl->u[vcnt]+fwtime);
							*(RenderBuff++) = (D3DVALUE)(tl->v[vcnt]);//+fwtime);					
							break;

						case smTEXSTATE_FS_REFLEX:
							*(RenderBuff++) = (D3DVALUE)( tl->u[vcnt] + ((float)(rv->tx>>1)/65536)) ;
							*(RenderBuff++) = (D3DVALUE)( tl->v[vcnt] + ((float)(rv->ty>>2)/65536)) ;
							break;

						}

						if ( TextureFormState[tcnt]>=smTEXSTATE_FS_SCROLL2 && TextureFormState[tcnt]<=smTEXSTATE_FS_SCROLL10 ) {
							//스크롤 ( 배속 2~10 )
							*(RenderBuff++) = (D3DVALUE)(tl->u[vcnt]+fwtime* (TextureFormState[tcnt]-(smTEXSTATE_FS_SCROLL2-2)) );
							*(RenderBuff++) = (D3DVALUE)(tl->v[vcnt]);
						}
						//######################################################################################
						//작 성 자 : 동동동
						else if( TextureFormState[tcnt]>=smTEXSTATE_FS_SCROLLSLOW1 && TextureFormState[tcnt]<=smTEXSTATE_FS_SCROLLSLOW4) //동동동 추가..
						{
 							int factor = smTEXSTATE_FS_SCROLLSLOW4 - TextureFormState[tcnt] + 4;
							wtime  = (RendStatTime >> 6) & (0xFFFF >> factor);

							fwtime = float(wtime) / float(0xFFFF >> factor);
							*(RenderBuff++) = (D3DVALUE)(tl->u[vcnt] + fwtime);
							*(RenderBuff++) = (D3DVALUE)(tl->v[vcnt]);
						}
						//######################################################################################
					}
					else {
						//변형 없음
						*(RenderBuff++) = (D3DVALUE)tl->u[vcnt];
						*(RenderBuff++) = (D3DVALUE)tl->v[vcnt];
					}

					tl = tl->NextTex;
				}
				rendcnt++;
			}
		}
		rendface = rendface->NexRendFace;
	}

	return rendcnt;
}
//######################################################################################




//######################################################################################
// 작 성 자 : 오 영 석
int smRENDER3D::SetItem2PassD3DRendState( smMATERIAL *lpMaterial )
{
	DWORD cnt;
	int	MapOpacity;
	int MapDualRend;

	MapOpacity = 0;
	MapDualRend = 0;
	
	if( lpMaterial->TextureType == SMTEX_TYPE_MULTIMIX )
	{
		//
		for( cnt=0; cnt < 2; cnt++ )
		{
			//
			if( cnt == 0 )
                TextureFormState[cnt] = lpMaterial->TextureFormState[cnt];
			else // if( cnt == 1 )
				TextureFormState[cnt] = s_Item2PassTexFormState;

			//
			lpD3DDevice->SetTextureStageState( cnt, D3DTSS_TEXCOORDINDEX, cnt );

			if( cnt == 0 )
                lpD3DDevice->SetTextureStageState( cnt, D3DTSS_COLOROP, D3DTOP_MODULATE );
			else // if( cnt == 1 )				
				lpD3DDevice->SetTextureStageState( cnt, D3DTSS_COLOROP, D3DTOP_ADD );

			lpD3DDevice->SetTextureStageState( cnt, D3DTSS_COLORARG1, D3DTA_TEXTURE );
            lpD3DDevice->SetTextureStageState( cnt, D3DTSS_COLORARG2, D3DTA_CURRENT );

			lpD3DDevice->SetTextureStageState( cnt, D3DTSS_ALPHAOP,   D3DTOP_MODULATE );
			lpD3DDevice->SetTextureStageState( cnt, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
            lpD3DDevice->SetTextureStageState( cnt, D3DTSS_ALPHAARG2, D3DTA_CURRENT );

			lpD3DDevice->SetTextureStageState( cnt, D3DTSS_MINFILTER, D3DTFN_LINEAR  );
            lpD3DDevice->SetTextureStageState( cnt, D3DTSS_MAGFILTER, D3DTFG_LINEAR  );

			//
			if( cnt == 0 )
                lpD3DDevice->SetTexture( cnt, lpMaterial->smTexture[cnt]->lpD3DTexture );
			else // if( cnt == 1 )
				lpD3DDevice->SetTexture( cnt, s_lpItem2PassTexHandle->lpD3DTexture );

			//
			if( cnt == 0 )
			{
				if( lpMaterial->smTexture[cnt]->MapOpacity )
					MapOpacity += lpMaterial->smTexture[cnt]->MapOpacity;
			}
		}

		//
		for( cnt=2; cnt < 8; cnt++ )
		{
            lpD3DDevice->SetTextureStageState( cnt, D3DTSS_TEXCOORDINDEX, 0 );

            lpD3DDevice->SetTextureStageState( cnt, D3DTSS_COLOROP, D3DTOP_DISABLE );
			lpD3DDevice->SetTextureStageState( cnt, D3DTSS_COLORARG1, D3DTA_TEXTURE );
			lpD3DDevice->SetTextureStageState( cnt, D3DTSS_COLORARG2, D3DTA_CURRENT );

            lpD3DDevice->SetTextureStageState( cnt, D3DTSS_ALPHAOP,   D3DTOP_DISABLE );
            lpD3DDevice->SetTextureStageState( cnt, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
            lpD3DDevice->SetTextureStageState( cnt, D3DTSS_ALPHAARG2, D3DTA_CURRENT );

            lpD3DDevice->SetTextureStageState( cnt, D3DTSS_MINFILTER, D3DTFN_POINT  );
            lpD3DDevice->SetTextureStageState( cnt, D3DTSS_MAGFILTER, D3DTFG_POINT  );

			lpD3DDevice->SetTexture( cnt, 0 );
		}		
	}
	else
	{
		for( cnt=0; cnt < 8; cnt++ )
		{
            lpD3DDevice->SetTextureStageState( cnt, D3DTSS_TEXCOORDINDEX, 0 );

            lpD3DDevice->SetTextureStageState( cnt, D3DTSS_COLOROP, D3DTOP_DISABLE );
			lpD3DDevice->SetTextureStageState( cnt, D3DTSS_COLORARG1, D3DTA_TEXTURE );
			lpD3DDevice->SetTextureStageState( cnt, D3DTSS_COLORARG2, D3DTA_CURRENT );

            lpD3DDevice->SetTextureStageState( cnt, D3DTSS_ALPHAOP,   D3DTOP_DISABLE );
            lpD3DDevice->SetTextureStageState( cnt, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
            lpD3DDevice->SetTextureStageState( cnt, D3DTSS_ALPHAARG2, D3DTA_CURRENT );

            lpD3DDevice->SetTextureStageState( cnt, D3DTSS_MINFILTER, D3DTFN_POINT  );
            lpD3DDevice->SetTextureStageState( cnt, D3DTSS_MAGFILTER, D3DTFG_POINT  );

			lpD3DDevice->SetTexture( cnt, 0 );
		}
	}

	lpD3DDevice->SetRenderState( D3DRENDERSTATE_TEXTUREMAPBLEND, D3DTBLEND_MODULATE );

	switch( lpMaterial->BlendType )
	{
	case SMMAT_BLEND_NONE:
		lpD3DDevice->SetRenderState( D3DRENDERSTATE_ALPHABLENDENABLE, FALSE );
		lpD3DDevice->SetRenderState( D3DRENDERSTATE_DESTBLEND, D3DBLEND_ONE  );
		lpD3DDevice->SetRenderState( D3DRENDERSTATE_SRCBLEND, D3DBLEND_ONE  );
		break;
	case SMMAT_BLEND_ALPHA:
		lpD3DDevice->SetRenderState( D3DRENDERSTATE_ALPHABLENDENABLE, TRUE );
		lpD3DDevice->SetRenderState( D3DRENDERSTATE_DESTBLEND, D3DBLEND_INVSRCALPHA  );
		lpD3DDevice->SetRenderState( D3DRENDERSTATE_SRCBLEND, D3DBLEND_SRCALPHA  );
		break;
	case SMMAT_BLEND_COLOR:
		lpD3DDevice->SetRenderState( D3DRENDERSTATE_ALPHABLENDENABLE, TRUE );
		lpD3DDevice->SetRenderState( D3DRENDERSTATE_DESTBLEND, D3DBLEND_INVSRCCOLOR  );
		lpD3DDevice->SetRenderState( D3DRENDERSTATE_SRCBLEND, D3DBLEND_SRCCOLOR  );
		break;

	case SMMAT_BLEND_ADDCOLOR:
		lpD3DDevice->SetRenderState( D3DRENDERSTATE_ALPHABLENDENABLE, TRUE );
		lpD3DDevice->SetRenderState( D3DRENDERSTATE_DESTBLEND, D3DBLEND_ONE  );
		lpD3DDevice->SetRenderState( D3DRENDERSTATE_SRCBLEND, D3DBLEND_SRCCOLOR  );
		break;

	case SMMAT_BLEND_SHADOW:
		lpD3DDevice->SetRenderState( D3DRENDERSTATE_ALPHABLENDENABLE, TRUE );
		lpD3DDevice->SetRenderState( D3DRENDERSTATE_DESTBLEND, D3DBLEND_SRCCOLOR );
		lpD3DDevice->SetRenderState( D3DRENDERSTATE_SRCBLEND, D3DBLEND_ZERO );
		break;

	case SMMAT_BLEND_LAMP:
		lpD3DDevice->SetRenderState( D3DRENDERSTATE_ALPHABLENDENABLE, TRUE );
		lpD3DDevice->SetRenderState( D3DRENDERSTATE_SRCBLEND , D3DBLEND_SRCALPHA );
		lpD3DDevice->SetRenderState( D3DRENDERSTATE_DESTBLEND, D3DBLEND_ONE );
		break;

	case SMMAT_BLEND_INVSHADOW:
		lpD3DDevice->SetRenderState( D3DRENDERSTATE_ALPHABLENDENABLE, TRUE );
		lpD3DDevice->SetRenderState( D3DRENDERSTATE_DESTBLEND, D3DBLEND_INVSRCCOLOR );
		lpD3DDevice->SetRenderState( D3DRENDERSTATE_SRCBLEND, D3DBLEND_ZERO );
		break;

	}

	if ( lpMaterial->TwoSide ) 
		lpD3DDevice->SetRenderState( D3DRENDERSTATE_CULLMODE , D3DCULL_NONE );
	else 
		lpD3DDevice->SetRenderState( D3DRENDERSTATE_CULLMODE , D3DCULL_CW );

	if ( MapOpacity ) {
		lpD3DDevice->SetRenderState( D3DRENDERSTATE_ALPHATESTENABLE , TRUE );
		lpD3DDevice->SetRenderState(D3DRENDERSTATE_ALPHAREF, AlphaTestDepth );
		lpD3DDevice->SetRenderState(D3DRENDERSTATE_ALPHAFUNC, D3DCMP_GREATEREQUAL  );
	}
	else {
		lpD3DDevice->SetRenderState( D3DRENDERSTATE_ALPHATESTENABLE , FALSE );
	}

	if ( ZWriteAuto ) {
		if ( lpMaterial->Transparency>0.2f )
			lpD3DDevice->SetRenderState( D3DRENDERSTATE_ZWRITEENABLE, FALSE );
		else
			lpD3DDevice->SetRenderState( D3DRENDERSTATE_ZWRITEENABLE, TRUE );
	}


	return MapDualRend;
}
//######################################################################################


// D3D렌더링 속성 설정 
int smRENDER3D::SetD3DRendState( smMATERIAL *lpMaterial , int TexClip )
{
	DWORD cnt;
	int	MapOpacity;
	int MapDualRend;

	MapOpacity = 0;
	MapDualRend = 0;

	switch( lpMaterial->TextureType ) {
	case SMTEX_TYPE_MULTIMIX:
		for( cnt=0;cnt<lpMaterial->TextureCounter;cnt++ ) {

			TextureFormState[cnt] = lpMaterial->TextureFormState[cnt];

            lpD3DDevice->SetTextureStageState( cnt, D3DTSS_TEXCOORDINDEX, cnt );

			if ( lpMaterial->TextureStageState[cnt] )
	            lpD3DDevice->SetTextureStageState( cnt, D3DTSS_COLOROP,   lpMaterial->TextureStageState[cnt] );
			else
	            lpD3DDevice->SetTextureStageState( cnt, D3DTSS_COLOROP,   D3DTOP_MODULATE    );

            lpD3DDevice->SetTextureStageState( cnt, D3DTSS_COLORARG1, D3DTA_TEXTURE );
			lpD3DDevice->SetTextureStageState( cnt, D3DTSS_COLORARG2, D3DTA_CURRENT ); 

            lpD3DDevice->SetTextureStageState( cnt, D3DTSS_ALPHAOP,   D3DTOP_MODULATE );
            lpD3DDevice->SetTextureStageState( cnt, D3DTSS_ALPHAARG2, D3DTA_CURRENT );
            lpD3DDevice->SetTextureStageState( cnt, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );

            lpD3DDevice->SetTextureStageState( cnt, D3DTSS_MINFILTER, D3DTFN_LINEAR  );
            lpD3DDevice->SetTextureStageState( cnt, D3DTSS_MAGFILTER, D3DTFG_LINEAR  );

			if ( !lpMaterial->smTexture[cnt] ) {		//텍스쳐를 불러오지 못한 경우
				lpD3DDevice->SetTexture( cnt, 0 );
				continue;
			}

			if ( TexClip==TRUE && lpMaterial->smTexture[cnt]->lpD3DTextureLarge )
				lpD3DDevice->SetTexture( cnt, lpMaterial->smTexture[cnt]->lpD3DTextureLarge );
			else
				lpD3DDevice->SetTexture( cnt, lpMaterial->smTexture[cnt]->lpD3DTexture );

			if ( lpMaterial->smTexture[cnt]->MapOpacity ) {
				MapOpacity += lpMaterial->smTexture[cnt]->MapOpacity;
				if ( cnt>0 ) {
					lpD3DDevice->SetTexture( cnt, 0 );
					MapDualRend = TRUE;
				}
			}
		}

		for( cnt=lpMaterial->TextureCounter; cnt<8; cnt++ ) {
			
            lpD3DDevice->SetTextureStageState( cnt, D3DTSS_TEXCOORDINDEX, 0 );

			
            lpD3DDevice->SetTextureStageState( cnt, D3DTSS_COLOROP, D3DTOP_DISABLE );
			lpD3DDevice->SetTextureStageState( cnt, D3DTSS_COLORARG1, D3DTA_TEXTURE );
			lpD3DDevice->SetTextureStageState( cnt, D3DTSS_COLORARG2, D3DTA_CURRENT );

            lpD3DDevice->SetTextureStageState( cnt, D3DTSS_ALPHAOP,   D3DTOP_DISABLE );
            lpD3DDevice->SetTextureStageState( cnt, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
            lpD3DDevice->SetTextureStageState( cnt, D3DTSS_ALPHAARG2, D3DTA_CURRENT );

            lpD3DDevice->SetTextureStageState( cnt, D3DTSS_MINFILTER, D3DTFN_POINT  );
            lpD3DDevice->SetTextureStageState( cnt, D3DTSS_MAGFILTER, D3DTFG_POINT  );

			lpD3DDevice->SetTexture( cnt, 0 );
		}

		break;

	case SMTEX_TYPE_ANIMATION:

		cnt = 0;

            lpD3DDevice->SetTextureStageState( cnt, D3DTSS_TEXCOORDINDEX, cnt );

            lpD3DDevice->SetTextureStageState( cnt, D3DTSS_COLOROP,   D3DTOP_MODULATE );
            lpD3DDevice->SetTextureStageState( cnt, D3DTSS_COLORARG1, D3DTA_TEXTURE );
			lpD3DDevice->SetTextureStageState( cnt, D3DTSS_COLORARG2, D3DTA_CURRENT ); 

            lpD3DDevice->SetTextureStageState( cnt, D3DTSS_ALPHAOP,   D3DTOP_MODULATE );
            lpD3DDevice->SetTextureStageState( cnt, D3DTSS_ALPHAARG2, D3DTA_CURRENT );
            lpD3DDevice->SetTextureStageState( cnt, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );

            lpD3DDevice->SetTextureStageState( cnt, D3DTSS_MINFILTER, D3DTFN_LINEAR  );
            lpD3DDevice->SetTextureStageState( cnt, D3DTSS_MAGFILTER, D3DTFG_LINEAR  );

		if ( lpMaterial->AnimationFrame==SMTEX_AUTOANIMATION )
			cnt = (RendStatTime>>lpMaterial->Shift_FrameSpeed)&lpMaterial->FrameMask;
		else
			cnt = lpMaterial->AnimationFrame;

		if ( cnt<32 && lpMaterial->smAnimTexture[cnt] ) {
			lpD3DDevice->SetTexture( 0, lpMaterial->smAnimTexture[cnt]->lpD3DTexture );
			MapOpacity += lpMaterial->smAnimTexture[cnt]->MapOpacity;
		}
		else
			lpD3DDevice->SetTexture( 0, 0 );			//프레임 텍스쳐 없음

		for( cnt=1; cnt<8; cnt++ ) {
            lpD3DDevice->SetTextureStageState( cnt, D3DTSS_TEXCOORDINDEX, 0 );


            lpD3DDevice->SetTextureStageState( cnt, D3DTSS_COLOROP, D3DTOP_DISABLE );
			lpD3DDevice->SetTextureStageState( cnt, D3DTSS_COLORARG1, D3DTA_TEXTURE );
			lpD3DDevice->SetTextureStageState( cnt, D3DTSS_COLORARG2, D3DTA_CURRENT );

            lpD3DDevice->SetTextureStageState( cnt, D3DTSS_ALPHAOP,   D3DTOP_DISABLE );
            lpD3DDevice->SetTextureStageState( cnt, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
            lpD3DDevice->SetTextureStageState( cnt, D3DTSS_ALPHAARG2, D3DTA_CURRENT );

            lpD3DDevice->SetTextureStageState( cnt, D3DTSS_MINFILTER, D3DTFN_POINT  );
            lpD3DDevice->SetTextureStageState( cnt, D3DTSS_MAGFILTER, D3DTFG_POINT  );

			lpD3DDevice->SetTexture( cnt, 0 );
		}


		break;
	}

	lpD3DDevice->SetRenderState( D3DRENDERSTATE_TEXTUREMAPBLEND, D3DTBLEND_MODULATE );

	switch( lpMaterial->BlendType )
	{
	case SMMAT_BLEND_NONE:
		lpD3DDevice->SetRenderState( D3DRENDERSTATE_ALPHABLENDENABLE, FALSE );
		lpD3DDevice->SetRenderState( D3DRENDERSTATE_DESTBLEND, D3DBLEND_ONE  );
		lpD3DDevice->SetRenderState( D3DRENDERSTATE_SRCBLEND, D3DBLEND_ONE  );
		break;
	case SMMAT_BLEND_ALPHA:
		lpD3DDevice->SetRenderState( D3DRENDERSTATE_ALPHABLENDENABLE, TRUE );
		lpD3DDevice->SetRenderState( D3DRENDERSTATE_DESTBLEND, D3DBLEND_INVSRCALPHA  );
		lpD3DDevice->SetRenderState( D3DRENDERSTATE_SRCBLEND, D3DBLEND_SRCALPHA  );
		break;
	case SMMAT_BLEND_COLOR:
		lpD3DDevice->SetRenderState( D3DRENDERSTATE_ALPHABLENDENABLE, TRUE );
		lpD3DDevice->SetRenderState( D3DRENDERSTATE_DESTBLEND, D3DBLEND_INVSRCCOLOR  );
		lpD3DDevice->SetRenderState( D3DRENDERSTATE_SRCBLEND, D3DBLEND_SRCCOLOR  );
		break;

	case SMMAT_BLEND_ADDCOLOR:
		lpD3DDevice->SetRenderState( D3DRENDERSTATE_ALPHABLENDENABLE, TRUE );
		lpD3DDevice->SetRenderState( D3DRENDERSTATE_DESTBLEND, D3DBLEND_ONE  );
		lpD3DDevice->SetRenderState( D3DRENDERSTATE_SRCBLEND, D3DBLEND_SRCCOLOR  );
		break;

	case SMMAT_BLEND_SHADOW:
		lpD3DDevice->SetRenderState( D3DRENDERSTATE_ALPHABLENDENABLE, TRUE );
		lpD3DDevice->SetRenderState( D3DRENDERSTATE_DESTBLEND, D3DBLEND_SRCCOLOR );
		lpD3DDevice->SetRenderState( D3DRENDERSTATE_SRCBLEND, D3DBLEND_ZERO );
		break;

	case SMMAT_BLEND_LAMP:
		lpD3DDevice->SetRenderState( D3DRENDERSTATE_ALPHABLENDENABLE, TRUE );
		lpD3DDevice->SetRenderState( D3DRENDERSTATE_SRCBLEND , D3DBLEND_SRCALPHA );
		lpD3DDevice->SetRenderState( D3DRENDERSTATE_DESTBLEND, D3DBLEND_ONE );
		break;

	case SMMAT_BLEND_INVSHADOW:
		lpD3DDevice->SetRenderState( D3DRENDERSTATE_ALPHABLENDENABLE, TRUE );
		lpD3DDevice->SetRenderState( D3DRENDERSTATE_DESTBLEND, D3DBLEND_INVSRCCOLOR );
		lpD3DDevice->SetRenderState( D3DRENDERSTATE_SRCBLEND, D3DBLEND_ZERO );
		break;

	}

/*
	lpD3DDevice->SetRenderState( D3DRENDERSTATE_CULLMODE , D3DCULL_CW );
	lpD3DDevice->SetRenderState( D3DRENDERSTATE_ALPHABLENDENABLE, TRUE );
	lpD3DDevice->SetRenderState( D3DRENDERSTATE_DESTBLEND, D3DBLEND_INVSRCALPHA  );
	lpD3DDevice->SetRenderState( D3DRENDERSTATE_SRCBLEND, D3DBLEND_SRCALPHA  );

            lpD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
            lpD3DDevice->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
            lpD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );

            lpD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1 );
            lpD3DDevice->SetTextureStageState( 0, D3DTSS_TEXCOORDINDEX, 0 );

            lpD3DDevice->SetTextureStageState( 0, D3DTSS_MINFILTER, D3DTFN_LINEAR );
            lpD3DDevice->SetTextureStageState( 0, D3DTSS_MAGFILTER, D3DTFG_LINEAR );



            lpD3DDevice->SetTextureStageState( 1, D3DTSS_TEXCOORDINDEX, 0 );
            lpD3DDevice->SetTextureStageState( 1, D3DTSS_COLORARG1, D3DTA_TEXTURE );
            lpD3DDevice->SetTextureStageState( 1, D3DTSS_COLORARG1, D3DTA_CURRENT );

            lpD3DDevice->SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_MODULATE );
            lpD3DDevice->SetTextureStageState( 1, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE );
            lpD3DDevice->SetTextureStageState( 1, D3DTSS_ALPHAOP, D3DTOP_SELECTARG2 );

            lpD3DDevice->SetTextureStageState( 1, D3DTSS_MINFILTER, D3DTFN_LINEAR );
            lpD3DDevice->SetTextureStageState( 1, D3DTSS_MAGFILTER, D3DTFG_LINEAR );


			lpD3DDevice->SetTexture( 1, lpMaterial->smTexture[1]->lpD3DTexture  );
			lpD3DDevice->SetTexture( 0, lpMaterial->smTexture[0]->lpD3DTexture  );


            lpD3DDevice->SetTextureStageState( 0, D3DTSS_TEXCOORDINDEX, 0 );
            lpD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
            lpD3DDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );
            lpD3DDevice->SetTextureStageState( 1, D3DTSS_TEXCOORDINDEX, 1 );
            lpD3DDevice->SetTextureStageState( 1, D3DTSS_COLORARG1, D3DTA_TEXTURE );
            lpD3DDevice->SetTextureStageState( 1, D3DTSS_COLOROP,   D3DTOP_MODULATE );

			lpD3DDevice->SetTexture( 1, lpMaterial->smTexture[0]->lpD3DTexture  );
			lpD3DDevice->SetTexture( 0, lpMaterial->smTexture[1]->lpD3DTexture  );
*/


	if ( lpMaterial->TwoSide ) 
		lpD3DDevice->SetRenderState( D3DRENDERSTATE_CULLMODE , D3DCULL_NONE );
	else 
		lpD3DDevice->SetRenderState( D3DRENDERSTATE_CULLMODE , D3DCULL_CW );

	if ( MapOpacity ) {
		lpD3DDevice->SetRenderState( D3DRENDERSTATE_ALPHATESTENABLE , TRUE );
		lpD3DDevice->SetRenderState(D3DRENDERSTATE_ALPHAREF, AlphaTestDepth );
		lpD3DDevice->SetRenderState(D3DRENDERSTATE_ALPHAFUNC, D3DCMP_GREATEREQUAL  );
	}
	else {
		lpD3DDevice->SetRenderState( D3DRENDERSTATE_ALPHATESTENABLE , FALSE );
	}

	if ( ZWriteAuto ) {
		if ( lpMaterial->Transparency>0.2f )
			lpD3DDevice->SetRenderState( D3DRENDERSTATE_ZWRITEENABLE, FALSE );
		else
			lpD3DDevice->SetRenderState( D3DRENDERSTATE_ZWRITEENABLE, TRUE );
	}


	return MapDualRend;
}







// D3D렌더링 속성 설정 알파만 설정
int smRENDER3D::SetD3DRendStateOnlyAlpha( smMATERIAL *lpMaterial , int TexClip )
{
	DWORD cnt;
	DWORD MapCnt;

	MapCnt = 0;

	for( cnt=0;cnt<lpMaterial->TextureCounter;cnt++ ) {

			if ( lpMaterial->smTexture[cnt] && lpMaterial->smTexture[cnt]->MapOpacity ) {

				lpD3DDevice->SetTextureStageState( MapCnt, D3DTSS_TEXCOORDINDEX, cnt );

	            lpD3DDevice->SetTextureStageState( MapCnt, D3DTSS_COLOROP,   D3DTOP_MODULATE );
	            lpD3DDevice->SetTextureStageState( MapCnt, D3DTSS_COLORARG1, D3DTA_TEXTURE );
				lpD3DDevice->SetTextureStageState( MapCnt, D3DTSS_COLORARG2, D3DTA_CURRENT ); 

	            lpD3DDevice->SetTextureStageState( MapCnt, D3DTSS_ALPHAOP,   D3DTOP_MODULATE );
	            lpD3DDevice->SetTextureStageState( MapCnt, D3DTSS_ALPHAARG2, D3DTA_CURRENT );
	            lpD3DDevice->SetTextureStageState( MapCnt, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
	
	            lpD3DDevice->SetTextureStageState( MapCnt, D3DTSS_MINFILTER, D3DTFN_LINEAR  );
	            lpD3DDevice->SetTextureStageState( MapCnt, D3DTSS_MAGFILTER, D3DTFG_LINEAR  );

				if ( TexClip==TRUE && lpMaterial->smTexture[cnt]->lpD3DTextureLarge )
					lpD3DDevice->SetTexture( MapCnt, lpMaterial->smTexture[cnt]->lpD3DTextureLarge );				
				else
					lpD3DDevice->SetTexture( MapCnt, lpMaterial->smTexture[cnt]->lpD3DTexture );

				MapCnt++;

				break;
			}

	}

	for( cnt=MapCnt; cnt<8; cnt++ ) {
			
            lpD3DDevice->SetTextureStageState( cnt, D3DTSS_TEXCOORDINDEX, 0 );

			
            lpD3DDevice->SetTextureStageState( cnt, D3DTSS_COLOROP, D3DTOP_DISABLE );
			lpD3DDevice->SetTextureStageState( cnt, D3DTSS_COLORARG1, D3DTA_TEXTURE );
			lpD3DDevice->SetTextureStageState( cnt, D3DTSS_COLORARG2, D3DTA_CURRENT );

            lpD3DDevice->SetTextureStageState( cnt, D3DTSS_ALPHAOP,   D3DTOP_DISABLE );
            lpD3DDevice->SetTextureStageState( cnt, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
            lpD3DDevice->SetTextureStageState( cnt, D3DTSS_ALPHAARG2, D3DTA_CURRENT );

            lpD3DDevice->SetTextureStageState( cnt, D3DTSS_MINFILTER, D3DTFN_POINT  );
            lpD3DDevice->SetTextureStageState( cnt, D3DTSS_MAGFILTER, D3DTFG_POINT  );

			lpD3DDevice->SetTexture( cnt, 0 );
	}

	lpD3DDevice->SetRenderState( D3DRENDERSTATE_TEXTUREMAPBLEND, D3DTBLEND_MODULATE );

	//알파블렌딩 설정
	lpD3DDevice->SetRenderState( D3DRENDERSTATE_ALPHABLENDENABLE, TRUE );
	lpD3DDevice->SetRenderState( D3DRENDERSTATE_DESTBLEND, D3DBLEND_INVSRCALPHA  );
	lpD3DDevice->SetRenderState( D3DRENDERSTATE_SRCBLEND, D3DBLEND_SRCALPHA  );

	if ( lpMaterial->TwoSide ) 
		lpD3DDevice->SetRenderState( D3DRENDERSTATE_CULLMODE , D3DCULL_NONE );
	else 
		lpD3DDevice->SetRenderState( D3DRENDERSTATE_CULLMODE , D3DCULL_CW );
/*
	if ( MapOpacity ) {
		lpD3DDevice->SetRenderState( D3DRENDERSTATE_ALPHATESTENABLE , TRUE );
		lpD3DDevice->SetRenderState(D3DRENDERSTATE_ALPHAREF, AlphaTestDepth );
		lpD3DDevice->SetRenderState(D3DRENDERSTATE_ALPHAFUNC, D3DCMP_GREATEREQUAL  );
	}
	else {
		lpD3DDevice->SetRenderState( D3DRENDERSTATE_ALPHATESTENABLE , FALSE );
	}
*/
	lpD3DDevice->SetRenderState( D3DRENDERSTATE_ALPHATESTENABLE , FALSE );

	return TRUE;
}


//메트리얼 블록을 D3D로 렌더링
int smRENDER3D::RenderD3D( int MatNum )
{
	int rendcnt;
	//int MatTexClip;

	if ( smRendMaterial[MatNum].TextureCounter==0 && smRendMaterial[MatNum].AnimTexCounter==0 ) return FALSE;

	//######################################################################################
	// 작 성 자 : 오 영 석
	short sColors[4] = {0,};
	if( smRendMaterial[MatNum].UseState & sMATS_SCRIPT_BLINK_COLOR )
		SetBlinkColor( sColors, smRendMaterial[MatNum].Diffuse, smRendMaterial[MatNum].WindMeshBottom );
	//######################################################################################

	//일반 메트리얼의 페이스 그리기
	if ( RendMatrial[MatNum].MatrialCounter )
	{
		//######################################################################################
		// 작 성 자 : 오 영 석
		int TexCnt = 0;
		if( s_lpItem2PassTexHandle )
		{
            RendAlphaFlag = SetItem2PassD3DRendState( &smRendMaterial[ MatNum ] );	//메트리얼로 렌더링 설정
			TexCnt = 1;
		}
		else
			RendAlphaFlag = SetD3DRendState( &smRendMaterial[MatNum], FALSE );		//메트리얼로 렌더링 설정
		//if ( RendAlphaFlag ) SetD3DRendStateOnlyAlpha( &smRendMaterial[MatNum] , FALSE );
		//######################################################################################
		

		if ( smRendMaterial[MatNum].SelfIllum!=0 )
			rendcnt = SetD3DRendBuff2( RendBuff , MatNum , FALSE );		//드로윙 버퍼 설정 
		//######################################################################################
		// 작 성 자 : 오 영 석
		else if( smRendMaterial[MatNum].UseState & sMATS_SCRIPT_BLINK_COLOR )
			rendcnt = SetD3DRendBuff3( RendBuff , MatNum , FALSE, sColors ); //드로윙 버퍼 설정 
		else
			rendcnt = SetD3DRendBuff( RendBuff , MatNum , FALSE );		//드로윙 버퍼 설정 
		//######################################################################################

		//######################################################################################
		// 작 성 자 : 오 영 석
		lpD3DDevice->DrawPrimitive( D3DPT_TRIANGLELIST,
			                        D3DFVF_TEXTURE[ smRendMaterial[MatNum].TextureCounter + TexCnt ],
                                    (LPVOID)RendBuff, rendcnt, NULL );
		//######################################################################################

		if ( RendAlphaFlag ) {
			//알파 합성 메트리얼일 경우 알파만 다시 렌더링
			SetD3DRendStateOnlyAlpha( &smRendMaterial[MatNum] , FALSE );
			lpD3DDevice->DrawPrimitive(D3DPT_TRIANGLELIST,
								D3DFVF_TEXTURE[ smRendMaterial[MatNum].TextureCounter ] ,
								(LPVOID)RendBuff,
								rendcnt,
								NULL);

		}
	}

	//텍스쳐 클리핑된 큰 텍스쳐 페이스 그리기 
	if ( RendMatrial[MatNum].TexClip_MatrialCounter )
	{
		RendAlphaFlag = SetD3DRendState( &smRendMaterial[MatNum] , TRUE );				//메트리얼로 렌더링 설정
		//if ( RendAlphaFlag ) SetD3DRendStateOnlyAlpha( &smRendMaterial[MatNum] , TRUE );
		if ( smRendMaterial[MatNum].SelfIllum!=0 )
			rendcnt = SetD3DRendBuff2( RendBuff , MatNum , TRUE );		//드로윙 버퍼 설정 
		//######################################################################################
		// 작 성 자 : 오 영 석
		else if( smRendMaterial[MatNum].UseState & sMATS_SCRIPT_BLINK_COLOR )
			rendcnt = SetD3DRendBuff3( RendBuff , MatNum , TRUE, sColors ); //드로윙 버퍼 설정 
		else
			rendcnt = SetD3DRendBuff( RendBuff , MatNum , TRUE );		//드로윙 버퍼 설정 
		//######################################################################################

		lpD3DDevice->DrawPrimitive(D3DPT_TRIANGLELIST,
									D3DFVF_TEXTURE[ smRendMaterial[MatNum].TextureCounter ] ,
									(LPVOID)RendBuff,
									rendcnt,
									NULL);

		if ( RendAlphaFlag ) {
			//알파 합성 메트리얼일 경우 알파만 다시 렌더링
			SetD3DRendStateOnlyAlpha( &smRendMaterial[MatNum] , TRUE );
			lpD3DDevice->DrawPrimitive(D3DPT_TRIANGLELIST,
								D3DFVF_TEXTURE[ smRendMaterial[MatNum].TextureCounter ] ,
								(LPVOID)RendBuff,
								rendcnt,
								NULL);
		}
	}

	return rendcnt;
}




// D3D로 렌더링
int smRENDER3D::RenderD3D()
{
	int cnt;
	int TotalRendCnt;
	int	MatCnt;

	int	RendLatterCount;
	int	RendLatterBuff[1024];

	RendLatterCount = 0;

#ifdef PT_GFX_GL
	// real-time shadows rebuild 3D positions from the screen vertices: give the
	// renderer the camera they were made with (camera = offset * mCamera >> FLOATS_EM)
	if ( !g_IsDxProjectZoomIn ) {
		const float s = 1.0f / (float)(1<<FLOATS_EM);
		float m[9] = { mCamera._11*s, mCamera._12*s, mCamera._13*s,
					   mCamera._21*s, mCamera._22*s, mCamera._23*s,
					   mCamera._31*s, mCamera._32*s, mCamera._33*s };
		PtGfx_ShadowCamera( m, s_DxProjection_w, s_DxProjection_h, (float)s_CenterX, (float)s_CenterY );
	}
#endif

	if ( DeviceRendMode ) lpD3DDevice->BeginScene();

	TotalRendCnt = 0;

	//######################################################################################
	// 작 성 자 : 오 영 석
	if( isRenderOutLine > 0 )
		TotalRendCnt += RenderD3D_OutLine();
	//######################################################################################

	//smMaterial[MatNum].MeshState |= sMATS_SCRIPT_RENDLATTER;

	//메트리얼 별로 렌더링 및 드로윙
	for( cnt=0;cnt<RMatListCnt;cnt++ ) {
		MatCnt = RendMatrialList[cnt];
		if ( (smRendMaterial[MatCnt].MeshState&sMATS_SCRIPT_RENDLATTER)==0 ) {
			TotalRendCnt+= RenderD3D( MatCnt );
		}
		else {
			if ( RendLatterCount<1024 )
				RendLatterBuff[RendLatterCount++]=MatCnt;
			else
				TotalRendCnt+= RenderD3D( MatCnt );
		}
	}

	//메트리얼 별로 렌더링 및 드로윙
	for( cnt=RMatListRearCnt+1;cnt<nMaterial;cnt++ ) {
		MatCnt = RendMatrialList[cnt];
		if ( (smRendMaterial[MatCnt].MeshState&sMATS_SCRIPT_RENDLATTER)==0 ) {
			TotalRendCnt+= RenderD3D( MatCnt );
		}
		else {
			if ( RendLatterCount<1024 )
				RendLatterBuff[RendLatterCount++]=MatCnt;
			else
				TotalRendCnt+= RenderD3D( MatCnt );
		}
	}

	//나중에 렌더링 하도록 설정된 메트리얼만 따로 모아 드로윙
	for( cnt=0;cnt<RendLatterCount;cnt++ ) {
		TotalRendCnt+= RenderD3D( RendLatterBuff[cnt] );
	}

	if ( DeviceRendMode ) lpD3DDevice->EndScene();

	if ( ZWriteAuto )
		lpD3DDevice->SetRenderState( D3DRENDERSTATE_ZWRITEENABLE, TRUE );

	return TotalRendCnt;
}

//렌더링용 라이트 벡터
void smSetRendLightVect( int x, int y, int z )
{
	pLight.x = x;
	pLight.y = y;
	pLight.z = z;
}


//######################################################################################
// 작 성 자 : 오 영 석
void smRENDER3D::SetD3D_OutLine( int isSet, int Dist, float fsetX, float fsetY, DWORD Color )
{
	isRenderOutLine = isSet;
	if( isRenderOutLine <= 0 )
		return;

	if( Dist < 300 )
        fOutLineRatio = 1.f;
    else if( Dist < 1300 )
		fOutLineRatio = 1.f - float(Dist - 300) / 1000.f;
	else
		fOutLineRatio = 0.f;

	fOutLineX    = fsetX;
	fOutLineY    = fsetY;
	OutLineColor = Color;
}
//######################################################################################


//######################################################################################
// 작 성 자 : 오 영 석
int smRENDER3D::RenderD3D_OutLine(void)
{
	smMATERIAL *lpMaterial;
	int MatNum, cnt, TotalRendCnt = 0;

	//
	lpD3DDevice->SetRenderState( D3DRENDERSTATE_TEXTUREFACTOR, OutLineColor );
	lpD3DDevice->SetTextureStageState( 0, D3DTSS_TEXCOORDINDEX, 0 );
	lpD3DDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );
    lpD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TFACTOR );
	lpD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_CURRENT );
    lpD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_MODULATE );
	lpD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
    lpD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_CURRENT );

	lpD3DDevice->SetTexture( 1, NULL );
	lpD3DDevice->SetTextureStageState( 1, D3DTSS_TEXCOORDINDEX, 0 );
	lpD3DDevice->SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_DISABLE );
	lpD3DDevice->SetTextureStageState( 1, D3DTSS_ALPHAOP, D3DTOP_DISABLE );
	lpD3DDevice->SetTextureStageState( 0, D3DTSS_MINFILTER, D3DTFN_POINT );
	lpD3DDevice->SetTextureStageState( 0, D3DTSS_MAGFILTER, D3DTFG_POINT );
	lpD3DDevice->SetTextureStageState( 1, D3DTSS_MINFILTER, D3DTFN_POINT );
	lpD3DDevice->SetTextureStageState( 1, D3DTSS_MAGFILTER, D3DTFG_POINT );

	//메트리얼 별로 렌더링 및 드로윙
	for( cnt=0; cnt < RMatListCnt; cnt++ )
	{
		MatNum	   = RendMatrialList[ cnt ];
		lpMaterial = &smRendMaterial[ MatNum ];
		
		if( lpMaterial->TwoSide ) 
			lpD3DDevice->SetRenderState( D3DRENDERSTATE_CULLMODE, D3DCULL_NONE );
		else 
			lpD3DDevice->SetRenderState( D3DRENDERSTATE_CULLMODE, D3DCULL_CW );

		if( lpMaterial->smTexture[0] )
		{
			lpD3DDevice->SetTexture( 0, lpMaterial->smTexture[0]->lpD3DTexture );

			if( lpMaterial->smTexture[0]->MapOpacity )
			{
				lpD3DDevice->SetRenderState( D3DRENDERSTATE_ALPHATESTENABLE, TRUE );
				lpD3DDevice->SetRenderState( D3DRENDERSTATE_ALPHAREF, AlphaTestDepth );
				lpD3DDevice->SetRenderState( D3DRENDERSTATE_ALPHAFUNC, D3DCMP_GREATEREQUAL );
			}
			else
			{
				lpD3DDevice->SetRenderState( D3DRENDERSTATE_ALPHATESTENABLE, FALSE );
			}
		}

		switch( lpMaterial->BlendType )
		{
		case SMMAT_BLEND_NONE:
			lpD3DDevice->SetRenderState( D3DRENDERSTATE_ALPHABLENDENABLE, FALSE );
			lpD3DDevice->SetRenderState( D3DRENDERSTATE_SRCBLEND, D3DBLEND_ONE  );
			lpD3DDevice->SetRenderState( D3DRENDERSTATE_DESTBLEND, D3DBLEND_ONE  );			
			break;
		case SMMAT_BLEND_ALPHA:
			lpD3DDevice->SetRenderState( D3DRENDERSTATE_ALPHABLENDENABLE, TRUE );
			lpD3DDevice->SetRenderState( D3DRENDERSTATE_SRCBLEND, D3DBLEND_SRCALPHA  );
			lpD3DDevice->SetRenderState( D3DRENDERSTATE_DESTBLEND, D3DBLEND_INVSRCALPHA  );			
			break;
		case SMMAT_BLEND_COLOR:
			lpD3DDevice->SetRenderState( D3DRENDERSTATE_ALPHABLENDENABLE, TRUE );
			lpD3DDevice->SetRenderState( D3DRENDERSTATE_SRCBLEND, D3DBLEND_SRCCOLOR  );
			lpD3DDevice->SetRenderState( D3DRENDERSTATE_DESTBLEND, D3DBLEND_INVSRCCOLOR  );			
			break;
		case SMMAT_BLEND_ADDCOLOR:
			lpD3DDevice->SetRenderState( D3DRENDERSTATE_ALPHABLENDENABLE, TRUE );
			lpD3DDevice->SetRenderState( D3DRENDERSTATE_SRCBLEND, D3DBLEND_SRCCOLOR  );
			lpD3DDevice->SetRenderState( D3DRENDERSTATE_DESTBLEND, D3DBLEND_ONE  );
			break;
		case SMMAT_BLEND_SHADOW:
			lpD3DDevice->SetRenderState( D3DRENDERSTATE_ALPHABLENDENABLE, TRUE );
			lpD3DDevice->SetRenderState( D3DRENDERSTATE_SRCBLEND, D3DBLEND_ZERO );
			lpD3DDevice->SetRenderState( D3DRENDERSTATE_DESTBLEND, D3DBLEND_SRCCOLOR );			
			break;
		case SMMAT_BLEND_LAMP:
			lpD3DDevice->SetRenderState( D3DRENDERSTATE_ALPHABLENDENABLE, TRUE );
			lpD3DDevice->SetRenderState( D3DRENDERSTATE_SRCBLEND , D3DBLEND_SRCALPHA );
			lpD3DDevice->SetRenderState( D3DRENDERSTATE_DESTBLEND, D3DBLEND_ONE );
			break;
		case SMMAT_BLEND_INVSHADOW:
			lpD3DDevice->SetRenderState( D3DRENDERSTATE_ALPHABLENDENABLE, TRUE );
			lpD3DDevice->SetRenderState( D3DRENDERSTATE_SRCBLEND, D3DBLEND_ZERO );
			lpD3DDevice->SetRenderState( D3DRENDERSTATE_DESTBLEND, D3DBLEND_INVSRCCOLOR );			
			break;
		}

		TotalRendCnt += RenderD3D_OutLine( MatNum );
	}

	//메트리얼 별로 렌더링 및 드로윙
	for( cnt=RMatListRearCnt+1; cnt < nMaterial; cnt++ )
	{
		MatNum	   = RendMatrialList[ cnt ];
		lpMaterial = &smRendMaterial[ MatNum ];

		if( lpMaterial->TwoSide ) 
			lpD3DDevice->SetRenderState( D3DRENDERSTATE_CULLMODE, D3DCULL_NONE );
		else 
			lpD3DDevice->SetRenderState( D3DRENDERSTATE_CULLMODE, D3DCULL_CW );

		if( lpMaterial->smTexture[0] )
		{
			lpD3DDevice->SetTexture( 0, lpMaterial->smTexture[0]->lpD3DTexture );

			if( lpMaterial->smTexture[0]->MapOpacity )
			{
				lpD3DDevice->SetRenderState( D3DRENDERSTATE_ALPHATESTENABLE, TRUE );
				lpD3DDevice->SetRenderState( D3DRENDERSTATE_ALPHAREF, AlphaTestDepth );
				lpD3DDevice->SetRenderState( D3DRENDERSTATE_ALPHAFUNC, D3DCMP_GREATEREQUAL );
			}
			else
			{
				lpD3DDevice->SetRenderState( D3DRENDERSTATE_ALPHATESTENABLE, FALSE );
			}
		}

		switch( lpMaterial->BlendType )
		{
		case SMMAT_BLEND_NONE:
			lpD3DDevice->SetRenderState( D3DRENDERSTATE_ALPHABLENDENABLE, FALSE );
			lpD3DDevice->SetRenderState( D3DRENDERSTATE_SRCBLEND, D3DBLEND_ONE  );
			lpD3DDevice->SetRenderState( D3DRENDERSTATE_DESTBLEND, D3DBLEND_ONE  );			
			break;
		case SMMAT_BLEND_ALPHA:
			lpD3DDevice->SetRenderState( D3DRENDERSTATE_ALPHABLENDENABLE, TRUE );
			lpD3DDevice->SetRenderState( D3DRENDERSTATE_SRCBLEND, D3DBLEND_SRCALPHA  );
			lpD3DDevice->SetRenderState( D3DRENDERSTATE_DESTBLEND, D3DBLEND_INVSRCALPHA  );			
			break;
		case SMMAT_BLEND_COLOR:
			lpD3DDevice->SetRenderState( D3DRENDERSTATE_ALPHABLENDENABLE, TRUE );
			lpD3DDevice->SetRenderState( D3DRENDERSTATE_SRCBLEND, D3DBLEND_SRCCOLOR  );
			lpD3DDevice->SetRenderState( D3DRENDERSTATE_DESTBLEND, D3DBLEND_INVSRCCOLOR  );			
			break;
		case SMMAT_BLEND_ADDCOLOR:
			lpD3DDevice->SetRenderState( D3DRENDERSTATE_ALPHABLENDENABLE, TRUE );
			lpD3DDevice->SetRenderState( D3DRENDERSTATE_SRCBLEND, D3DBLEND_SRCCOLOR  );
			lpD3DDevice->SetRenderState( D3DRENDERSTATE_DESTBLEND, D3DBLEND_ONE  );
			break;
		case SMMAT_BLEND_SHADOW:
			lpD3DDevice->SetRenderState( D3DRENDERSTATE_ALPHABLENDENABLE, TRUE );
			lpD3DDevice->SetRenderState( D3DRENDERSTATE_SRCBLEND, D3DBLEND_ZERO );
			lpD3DDevice->SetRenderState( D3DRENDERSTATE_DESTBLEND, D3DBLEND_SRCCOLOR );			
			break;
		case SMMAT_BLEND_LAMP:
			lpD3DDevice->SetRenderState( D3DRENDERSTATE_ALPHABLENDENABLE, TRUE );
			lpD3DDevice->SetRenderState( D3DRENDERSTATE_SRCBLEND , D3DBLEND_SRCALPHA );
			lpD3DDevice->SetRenderState( D3DRENDERSTATE_DESTBLEND, D3DBLEND_ONE );
			break;
		case SMMAT_BLEND_INVSHADOW:
			lpD3DDevice->SetRenderState( D3DRENDERSTATE_ALPHABLENDENABLE, TRUE );
			lpD3DDevice->SetRenderState( D3DRENDERSTATE_SRCBLEND, D3DBLEND_ZERO );
			lpD3DDevice->SetRenderState( D3DRENDERSTATE_DESTBLEND, D3DBLEND_INVSRCCOLOR );			
			break;
		}

		TotalRendCnt += RenderD3D_OutLine( MatNum );
	}

	return TotalRendCnt;
}
//######################################################################################


//######################################################################################
// 작 성 자 : 오 영 석
int smRENDER3D::RenderD3D_OutLine( int MatNum )
{
	if( smRendMaterial[MatNum].TextureCounter <= 0 && smRendMaterial[MatNum].AnimTexCounter <= 0 )
		return 0;

	if( RendMatrial[MatNum].MatrialCounter <= 0 )
		return 0;

	//BYTE dwSpecular[4];
	BYTE dwColor[4];
	
	D3DVALUE   *RenderBuff = RendBuff;
	smRENDFACE *rendface   = RendMatrial[ MatNum ].StartFace;
	int		    mcnt       = RendMatrial[ MatNum ].MatrialCounter;

	int   rendcnt = 0;
	for( int cnt=0; cnt < mcnt; cnt++ )
	{
		if( ! (rendface->ClipStatus & SMCLIP_DISPLAYOUT) )
		{
			for( int vcnt=0; vcnt < 3; vcnt++ )
			{
				smRENDVERTEX *rv = rendface->lpRendVertex[vcnt];
				smTEXLINK    *tl = rendface->lpTexLink;

				*(RenderBuff++) = (D3DVALUE)rv->xy[0] + (fOutLineX * fOutLineRatio);
				*(RenderBuff++) = (D3DVALUE)rv->xy[1] + (fOutLineY * fOutLineRatio);
				*(RenderBuff++) = (D3DVALUE)(rv->zb + (rv->rhw/2.f));
				*(RenderBuff++) = (D3DVALUE)rv->rhw;

				if( smRendMaterial[MatNum].SelfIllum != 0 )
					*((DWORD*)(RenderBuff++)) = (*((DWORD*)rv->bCol)) | 0x00FFFFFF;
				else
				{
					if( RendAlphaFlag && rv->tz > (ccDistZMin<<FLOATNS) )
					{
						*((DWORD*)dwColor) = *((DWORD*)rv->bCol);
						dwColor[3] = dwColor[3]/2 + dwColor[3]/3;

						*((DWORD*)(RenderBuff++)) = *((DWORD*)dwColor);
					}
					else
						*((DWORD*)(RenderBuff++)) = *((DWORD*)rv->bCol);
				}

				*(RenderBuff++) = (D3DVALUE)tl->u[vcnt];
				*(RenderBuff++) = (D3DVALUE)tl->v[vcnt];

				rendcnt++;
			}
		}

		rendface = rendface->NexRendFace;
	}

	lpD3DDevice->DrawPrimitive( D3DPT_TRIANGLELIST, D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1,
								(LPVOID)RendBuff, rendcnt, NULL );
	return rendcnt;
}
//######################################################################################

//페이스에 해당하는 높이를 구한다
int smGetPolyHeight( POINT3D *p1 , POINT3D *p2 , POINT3D *p3 , int x, int z );


//2d 좌표가 포함된 렌더링된 페이스를 찾는다
int smRENDER3D::FindRenderFace2D( smRENDFACE **lpRendFace , int x, int y )
{
	int cnt,vcnt;
	POINT3D lpVertex2D[3];
	int z;

	smRENDFACE *lpLastRendFace = 0;
	int			LastDepthZ = 65536;
	//int			Mat;

//smMaterial[ face->Vertex[3] ].MeshState & SMMAT_STAT_CHECK_FACE

	for(cnt=0;cnt<nRendFace;cnt++) {
		//Mat = smMaterial[ RendFace[cnt].Matrial ].MeshState & SMMAT_STAT_CHECK_FACE;
		if ( (RendFace[cnt].ClipStatus&SMCLIP_DISPLAYOUT)==0 ) {//&& Mat) {
			for( vcnt=0;vcnt<3;vcnt++ ) {
				lpVertex2D[vcnt].x = (int)RendFace[cnt].lpRendVertex[vcnt]->xy[0];
				lpVertex2D[vcnt].z = (int)RendFace[cnt].lpRendVertex[vcnt]->xy[1];
				lpVertex2D[vcnt].y = int(RendFace[cnt].lpRendVertex[vcnt]->zb*65536);
			}

			z = smGetPolyHeight( &lpVertex2D[0],&lpVertex2D[1],&lpVertex2D[2] , x,y );
			if ( z!=CLIP_OUT && z<LastDepthZ ) {
				lpLastRendFace = &RendFace[cnt];
				LastDepthZ = z;
			}
		}
	}

	if ( lpLastRendFace ) {
		*lpRendFace = lpLastRendFace;
		return LastDepthZ;
	}

	lpRendFace = 0;
	return NULL;
}







//페이스에 해당하는 높이를 구한다
int smGetPolyHeight( POINT3D *p1 , POINT3D *p2 , POINT3D *p3 , int x, int z )
{

	POINT3D		*p[3];
	POINT3D		*ptop, *pmid , *pbot;

	int cnt;

	int lx , tx , bx;
	int ly , ty , by;
	int   lz , tz , bz;

	int   x1 , x2;
	int   y1 , y2;
	int	  ye,xe;
	int	  yl;

	p[0] = p1;
	p[1] = p2;
	p[2] = p3;

	ptop = p[0];
	pbot = p[1];
	pmid = p[2];
	
	//최상위 포인트
	for(cnt=0;cnt<3;cnt++) {
		if ( p[cnt]->z<ptop->z ) ptop = p[cnt];
	}

	if ( ptop==pbot ) pbot = pmid;

	//최하위 포인트
	for(cnt=0;cnt<3;cnt++) {
		if ( p[cnt]->z>pbot->z && ptop!=p[cnt] ) pbot = p[cnt];
	}

	//중간 포인트
	for(cnt=0;cnt<3;cnt++) {
		if ( ptop!=p[cnt] && pbot!=p[cnt] ) {
			pmid = p[cnt];
			break;
		}
	}

	if ( z<ptop->z || z>pbot->z ) 
		return CLIP_OUT;

	lz = pbot->z - ptop->z;
	tz = pmid->z - ptop->z;
	bz = pbot->z - pmid->z;

	if ( lz!=0 ) lx = ((pbot->x - ptop->x)<<FLOATNS) / lz ;
	else lx = 0;
	if ( tz!=0 ) tx = ((pmid->x - ptop->x)<<FLOATNS) / tz ;
	else tx = 0;
	if ( bz!=0 ) bx = ((pbot->x - pmid->x)<<FLOATNS) / bz ;
	else bx = 0;

	x1 = (( lx * ( z - ptop->z ) )>>FLOATNS) + ptop->x;

	if( z<pmid->z ) { 
		x2 = (( tx * ( z - ptop->z ) )>>FLOATNS) + ptop->x;
	}
	else {
		x2 = (( bx * ( z - pmid->z ) )>>FLOATNS) + pmid->x;
	}


	if ( lz!=0 ) ly = ((pbot->y - ptop->y)<<FLOATNS) / lz ;
	else ly = 0;
	if ( tz!=0 ) ty = ((pmid->y - ptop->y)<<FLOATNS) / tz ;
	else ty = 0;
	if ( bz!=0 ) by = ((pbot->y - pmid->y)<<FLOATNS) / bz ;
	else by = 0;

	y1 = (( ly * ( z - ptop->z ) ) >>FLOATNS) + ptop->y;

	if( z<pmid->z ) { 
		y2 = (( ty * ( z - ptop->z ) )>>FLOATNS) + ptop->y;
	}
	else {
		y2 = (( by * ( z - pmid->z ) )>>FLOATNS) + pmid->y;
	}

	if ( x1>x2 )  { 
		cnt=x1; x1=x2; x2=cnt;  
		cnt=y1; y1=y2; y2=cnt;  
	}

	if ( x<x1 || x>x2 ) 
		return CLIP_OUT;


	xe = x2-x1;
	ye = y2-y1;

	if ( xe!=0 ) yl = (ye<<FLOATNS) / xe;
	else yl = y1;

	return (( yl * (x - x1 ))>>FLOATNS)+y1;
}
