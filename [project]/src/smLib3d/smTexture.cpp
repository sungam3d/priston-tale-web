
#include "smd3d.h"
#include <io.h>


//######################################################################################
//작 성 자 : 오 영 석
int		g_IsReadTextures = 1;

// Image decode buffer. Was a fixed BYTE[1024*768*4] (3 MB); the v3215 data has
// bigger images (e.g. 1920x1280 BMPs), which overflowed it and overwrote the
// texture table that follows it in memory. Now it grows to fit each image.
BYTE	*g_DibImageDateBuff = NULL;
static DWORD g_DibImageDateBuffSize = 0;

static BOOL ReserveDibImageBuff( DWORD dwSize )
{
	if ( dwSize > 64*1024*1024 ) return FALSE;		//이상한 크기 (손상된 파일)
	if ( dwSize <= g_DibImageDateBuffSize && g_DibImageDateBuff ) return TRUE;
	DWORD dwNew = dwSize > (1024*768)*4 ? dwSize : (1024*768)*4;
	BYTE *lpNew = (BYTE *)malloc( dwNew );
	if ( !lpNew ) return FALSE;
	if ( g_DibImageDateBuff ) free( g_DibImageDateBuff );
	g_DibImageDateBuff = lpNew;
	g_DibImageDateBuffSize = dwNew;
	return TRUE;
}
int		g_DibLastReadWidth;
int		g_DibLastReadHeight;
int		g_TgaLastBitCnt;

#ifdef PT_WEB
#include <emscripten.h>
#include <string>
// Browser: the texture files a map or model needs next, one path per line, to the
// service worker (web\sw.js), which downloads them several at once while the game
// reads them one by one. Without a service worker (no 204 back) it stops asking.
// Worker threads send it synchronously (so it is there before the files are asked
// for); the page's own thread with fetch (Chrome does not pass a synchronous request
// from the page through the service worker), without waiting for the answer.
EM_JS( void, PtWeb_PrefetchHint, ( const char *list ), {
	if ( Module['ptNoPrefetch'] ) return;
	var body = UTF8ToString( list );
	if ( typeof window != 'undefined' ) {
		fetch( '__pt_prefetch', { method: 'POST', body: body } )
			.then( function ( r ) { if ( r.status != 204 ) Module['ptNoPrefetch'] = 1; },
				   function () { Module['ptNoPrefetch'] = 1; } );
		return;
	}
	try {
		var xhr = new XMLHttpRequest();
		xhr.open( 'POST', '__pt_prefetch', false );
		xhr.send( body );
		if ( xhr.status != 204 ) Module['ptNoPrefetch'] = 1;
	} catch ( e ) { Module['ptNoPrefetch'] = 1; }
} );
static void PrefetchHint( const std::string &list, int count )
{
	if ( count>=3 ) PtWeb_PrefetchHint( list.c_str() );		// one or two: not worth a request
}
#endif
int		g_TgaLastSwapHeight;

int						new_smCreateTexture( smTEXTUREHANDLE *lpTexHandle, int MipMap = 0 );

int						new_LoadDib( char *Filename );
int						new_LoadTga( char *Filename );
int						New_DrawSurfaceFromTga( LPDIRECTDRAWSURFACE4 lpDDSurface );

LPDIRECTDRAWSURFACE4	new_LoadBmpCreateTexSurface( char *Filename, int MipMap, int &rTexHalf );
LPDIRECTDRAWSURFACE4	new_LoadTgaCreateTexSurface( char *Filename, int MipMap, int &rTexHalf );
LPDIRECTDRAWSURFACE4	new_LoadBmpSurfaceOffscreen( char *Filename );
//######################################################################################


//알파채널을 위한 픽셀 포맷 보관
DDPIXELFORMAT	ddTextureFormat;
//알파채널을 위한 픽셀 포맷 보관
DDPIXELFORMAT	ddDefaultTextureFormat;
//알파 텍스쳐 포맷을 구하는 함수
int GetPixelFormatAlphaTexture();

#define ERROR_LIST_MAX		32
#define ERROR_LIST_MASK		(ERROR_LIST_MAX-1)

char szErrorList[ERROR_LIST_MAX][64];
int ErrorListCnt  = 0;


int AddErrorList( char *szErrMsg )
{
	int cnt;

	cnt = ErrorListCnt & ERROR_LIST_MASK;

	lstrcpy( szErrorList[cnt] , szErrMsg );

	ErrorListCnt++;

	return ErrorListCnt;
}

int DisplayError( HDC hdc , int x, int y )
{
	int cnt,mcnt;
	int len;
	int dy;

	dy = y;

	for(cnt=ErrorListCnt-ERROR_LIST_MASK;cnt<ErrorListCnt;cnt++) {
		if ( cnt>=0 ) {
			mcnt = cnt & ERROR_LIST_MASK;
			len = lstrlen( szErrorList[mcnt] );
			//dsTextLineOut( hdc , x,dy ,szErrorList[mcnt] , len );
			dy+=14;
		}
	}

	return TRUE;
}




//비디오 사용 메모리 예상치
DWORD	VramTotal = 0;


//######################################################################################
//작 성 자 : 오 영 석
#define	MAX_MAP_BLINK_COLOR_TIME				8

const char *szBlinkTimeScript[ MAX_MAP_BLINK_COLOR_TIME ] =
{
	"BLINK_TIME1:",
	"BLINK_TIME2:",
	"BLINK_TIME3:",
	"BLINK_TIME4:",
	"BLINK_TIME5:",
	"BLINK_TIME6:",
	"BLINK_TIME7:",
	"BLINK_TIME8:",
};

const DWORD dwBlinkTimeCode[ MAX_MAP_BLINK_COLOR_TIME ] =
{
    9, 10, 11, 12, 13, 14, 15, 16,
};
//######################################################################################


//######################################################################################
//작 성 자 : 오 영 석
int MipMapModeCreate;						// 밉맵 사용 가능 한지.
int TextureSwapMode	= TRUE;					// 그래픽 퀄리티에 의해서 밉맵 사용 할지 알기위해. 전역으로.
static int TexturHalfSize = FALSE;			// 텍스쳐 절반으로 크기 줄임

// 6.0 에서의 밉맵 싸이즈 레벨 단계 256×256, 128×128, 64×64, 32×32, 16×16
// 또한 256×128 로 시작 할수도 있지만. 카드별로 안되는 것도 있다고 합니다.
// 프테 에서는 최대 3단계 까지만 사용 하기로 했음.
static char MipMapFileListBuff[ MIPMAP_FILELIST ][16] = { "", "_mm1", "_mm2", "_mm3", "_mm4", };

static int  MipMapCount = 0;
static char MipMapFileBuff[ MIPMAP_FILELIST ][126] = { 0,};
static int  MipMapCountA = 0;
static char MipMapFileABuff[ MIPMAP_FILELIST ][126] = { 0,};
static char FileTypeBuff[16] = { 0,};
//######################################################################################


int		   MaterialFrame;

//텍스쳐 메모리 스왑 모드 설정
int SetTextureSwapMode( int mode )
{
	int OldValue;

	OldValue = TextureSwapMode;

	TextureSwapMode = mode;

	return OldValue;
}

//텍스쳐의 레벨 설정 ( 0-고화질 1-보통 2-저화질 )
int smSetTextureLevel( int Level )
{
	switch( Level ) {
	case 0:	//( 비디오 메모리가 큰 시스템에서 유리 )
		//모든 텍스쳐를 비디오 메모리로 올린다
		TextureSwapMode = FALSE;
		TexturHalfSize = 0;
		break;

	case 1: //( 메모리가 큰 시스템에서 유리 )
		//텍스쳐를 비디오와 시스템 메모리에 섞음
		TextureSwapMode = TRUE;
		TexturHalfSize = 0;
		break;

	case 2: //( 모두 후진 시스템에서 사용 )
		//모든 텍스쳐를 절반크기로 줄여서 비디오 메모리에 올림
		TextureSwapMode = TRUE;
		TexturHalfSize = 1;
		break;

	case 3: //( VRAM 8메가 이하 절대 후진 시스템에서 사용 )
		//모든 텍스쳐를 1/4크기로 줄여서 비디오 메모리에 올림
		TextureSwapMode = TRUE;
		TexturHalfSize = 2;
		break;
	}

	return TRUE;
}



smTEXTURE::smTEXTURE()
{
	Init();
}

smTEXTURE::~smTEXTURE()
{
//	Remove();
}

void smTEXTURE::Init()
{
	//######################################################################################
	//작 성 자 : 오 영 석
	ZeroMemory( TexSpeedSum, sizeof(TexSpeedSum) );
	//######################################################################################

	ZeroMemory( Texture , sizeof( smTEXTUREHANDLE ) * MAX_TEXTURE ); 
	ZeroMemory( SwapTexture , sizeof( smSWAPTEXTURE ) * SWAPTEXTURE_MAX ); 
	TextureCnt	= 0;
	SwapCount	= 0;

	//크리티컬 섹션 초기화
	InitializeCriticalSection( &cTextureSection );

}

int smTEXTURE::Remove()
{
	int cnt;

	//크리티칼 섹션 선언
	EnterCriticalSection( &cTextureSection );

	for( cnt=0; cnt< SWAPTEXTURE_MAX ; cnt++ ) {
		if ( SwapTexture[cnt].SwapState == TRUE ) {
			DeleteSwapTexture( cnt );
		}
	}

	for( cnt=0;cnt<MAX_TEXTURE;cnt++) {
		if ( Texture[cnt].UseCounter>0 ) {
			//텍스쳐 관련 핸들 제거
			if ( Texture[cnt].lpD3DTexture ) Texture[cnt].lpD3DTexture->Release();
			if ( Texture[cnt].lpDDSurface ) Texture[cnt].lpDDSurface->Release();
			if ( Texture[cnt].lpDDSSysMemory ) Texture[cnt].lpDDSSysMemory->Release();
			Texture[cnt].UseCounter = 0;
		}
	}
	//크리티칼 섹션 해제
	LeaveCriticalSection( &cTextureSection );

	//크리티컬 섹션 제거
	DeleteCriticalSection( &cTextureSection );

	return TRUE;
}


//######################################################################################
//작 성 자 : 오 영 석
//int smTEXTURE::Find( char *name )
int smTEXTURE::Find( DWORD dwCode, char *Name )
{
	int cnt;
	for( cnt=0; cnt < MAX_TEXTURE; cnt++ )
	{
		if( TexSpeedSum[cnt] == dwCode )
		{
			if( Texture[cnt].UseCounter && _stricmp( Texture[cnt].Name, Name ) == 0 )
				return cnt;
		}
	}

	return -1;
}
//######################################################################################


//텍스쳐 목록을 추가
smTEXTUREHANDLE *smTEXTURE::Add( char *name , char *nameA )
{
	int TexNum;
	int cnt;

	//######################################################################################
	//작 성 자 : 오 영 석	
	extern DWORD GetSpeedSum( char *szName );

	DWORD dwCode = GetSpeedSum(name);
	TexNum = Find( dwCode, name );
	//TexNum = Find( name );
	//######################################################################################

	if ( TexNum>=0 ) {
		LastTextureCnt = TexNum;
		Texture[ TexNum ].UseCounter++;
		return &Texture[ TexNum ];
	}

	for( cnt=0;cnt<MAX_TEXTURE;cnt++) {
		if ( Texture[ cnt ].UseCounter==0 ) break;
	}
	if ( cnt==MAX_TEXTURE ) return NULL;
	TextureCnt = cnt;

	lpLastTextureSurface = &Texture[ TextureCnt ].lpDDSurface;
	lpLastTexture = &Texture[ TextureCnt ].lpD3DTexture;

	//초기 핸들 값은 0 이다
	Texture[TextureCnt].lpDDSurface = NULL;
	Texture[TextureCnt].lpD3DTexture = NULL;

	lstrcpy( Texture[ TextureCnt ].Name , name );
	if ( nameA )
		lstrcpy( Texture[ TextureCnt ].NameA , nameA );
	else 
		Texture[ TextureCnt ].NameA[0] = 0;

	Texture[ TextureCnt ].TexSwapMode = TextureSwapMode;			// 기본적으로 스왑모드 허가
	Texture[ TextureCnt ].UseCounter++;
	Texture[ TextureCnt ].lpD3DTextureLarge = NULL;

	//######################################################################################
	//작 성 자 : 오 영 석	
	TexSpeedSum[ TextureCnt ] = dwCode;
	//######################################################################################

	return &Texture[ TextureCnt ];
}

//텍스쳐를 제거한다 ( 1단계식 제거하여 메트리얼과의 링크가 없을 경우 완전 제거 )
int smTEXTURE::Delete( smTEXTUREHANDLE *lpTex )
{
	int cnt;

	if ( lpTex->UseCounter<=0 ) return FALSE;

	lpTex->UseCounter--;

	if ( lpTex->UseCounter<=0 ) {
		//스왑 텍스쳐에서 사용중이면 스왑텍스쳐를 찾아내어 제거
		if ( lpTex->lpD3DTextureLarge ) {
			for( cnt=0; cnt<SWAPTEXTURE_MAX ; cnt++ ) {
				if ( SwapTexture[cnt].SwapState && SwapTexture[cnt].smTextureMaster==lpTex ) {
					DeleteSwapTexture( cnt );
					break;
				}
			}
		}
		//텍스쳐 관련 핸들 제거
		if ( lpTex->lpDDSurface ) lpTex->lpDDSurface->Release();
		if ( lpTex->lpD3DTexture ) lpTex->lpD3DTexture->Release();
		if ( lpTex->lpDDSSysMemory ) lpTex->lpDDSSysMemory->Release();

		lpTex->lpD3DTexture = 0;
		lpTex->lpDDSurface = 0;
		lpTex->lpDDSSysMemory = 0;

		VramTotal -= lpTex->Width * lpTex->Height  * 2 ;

	}

	return lpTex->UseCounter;
}

/*
LPDIRECT3DTEXTURE2 *smTEXTURE::AddName( char *name )
{
	int TexNum;

	TexNum = Find( name );
	if ( TexNum>=0 ) {
		LastTextureCnt = TexNum;
		return &Texture[ TexNum ].lpD3DTexture;
	}

	lpLastTextureSurface = &Texture[ TextureCnt ].lpDDSurface;
	lpLastTexture = &Texture[ TextureCnt ].lpD3DTexture;

	lstrcpy( Texture[ TextureCnt ].Name , name );
	Texture[ TextureCnt ].NameA[0] = 0;
	TextureCnt++;

	return &Texture[ TextureCnt-1 ].lpD3DTexture;
}

LPDIRECT3DTEXTURE2 *smTEXTURE::AddName( char *name , char *nameA )
{
	int TexNum;
	char szBuff[64];
	LPDIRECT3DTEXTURE2 *lpTexture;
	FILE *fp;

	TexNum = Find( name );
	if ( TexNum>=0 ) {
		LastTextureCnt = TexNum;
		return &Texture[ TexNum ].lpD3DTexture;
	}

	lpTexture = &Texture[ TextureCnt ].lpD3DTexture ;

	lpLastTextureSurface = &Texture[ TextureCnt ].lpDDSurface;
	lpLastTexture = &Texture[ TextureCnt ].lpD3DTexture;
	LastTextureCnt = TextureCnt;

	lstrcpy( Texture[ TextureCnt ].Name , name );
	lstrcpy( Texture[ TextureCnt ].NameA , nameA );
	TextureCnt++;

	return lpTexture;
}
*/


//SWAP 텍스쳐를 지움
int smTEXTURE::DeleteSwapTexture( int TexNum )
{
	//크리티칼 섹션 선언
	EnterCriticalSection( &cTextureSection );

	if (SwapTexture[TexNum].lpDDSurface) {
		SwapTexture[TexNum].lpDDSurface->Release();
		SwapTexture[TexNum].lpDDSurface = 0;
	}

	if ( SwapTexture[TexNum].lpD3DTexture ) {
		SwapTexture[TexNum].lpD3DTexture->Release();
		SwapTexture[TexNum].lpD3DTexture = 0;
	}

	SwapTexture[TexNum].smTextureMaster->lpD3DTextureLarge = NULL;
	SwapTexture[TexNum].SwapState = FALSE;

	//비디오 메모리 사용 예상치
	VramTotal -= SwapTexture[TexNum].smTextureMaster->Width *  
		 SwapTexture[TexNum].smTextureMaster->Height  * 2 ;

	//크리티칼 섹션 해제
	LeaveCriticalSection( &cTextureSection );

	return TRUE;
}

//큰텍스쳐를 SWAP 하여 만든다 
int smTEXTURE::SetLargeTexture( smTEXTUREHANDLE *TexHandle )
{
	int cnt;
	int OldTex = 0;
	int OldTexNum = 0;

	// 스왑할 서페이스가 없는경우 종료
	if( TexHandle->lpDDSSysMemory==NULL ) return FALSE;

	// 기존에 사용중인 스왑 텍스쳐 인지 확인하여 돌려줌
	for ( cnt=0; cnt<SWAPTEXTURE_MAX ; cnt++ ) {
		if ( SwapTexture[cnt].SwapState==TRUE && SwapTexture[cnt].smTextureMaster==TexHandle ) {
			SwapTexture[cnt].UsedTime = SwapCount;
			return TRUE;
		}
	}

	// 빈공간을 찾는다
	for( cnt=0 ; cnt<SWAPTEXTURE_MAX ; cnt++ ) {
		if ( SwapTexture[cnt].SwapState==NULL ) break;
	}

	if ( cnt==SWAPTEXTURE_MAX ) {
		//빈공간이 없으면 사용한지 오래된 텍스쳐를 찾는다

		OldTex = SwapTexture[0].UsedTime;		//예전 코드는 SwapTexture[SWAPTEXTURE_MAX] (배열 밖) 을 읽었음

		for( cnt=0 ; cnt<SWAPTEXTURE_MAX ; cnt++ ) {
			if ( SwapTexture[cnt].UsedTime < OldTex ) {
				OldTex = SwapTexture[cnt].UsedTime;
				OldTexNum = cnt;
			}
		}

		//오래된 텍스쳐가 없는 경우 종료
		if ( OldTex==SwapCount ) return FALSE;

		//오래된 텍스쳐를 지운다
		DeleteSwapTexture( OldTexNum );
		cnt = OldTexNum;
	}

	//크리티칼 섹션 선언
	EnterCriticalSection( &cTextureSection );

	// Now create surface
    LPDIRECT3DTEXTURE2 lpD3DTexture;
	LPDIRECTDRAWSURFACE4 lpDDSurface;
	DDSURFACEDESC2		ddsd;
	RECT				ddRect;
	int hresult;

	// 새로운 텍스쳐 서페이스 생성
	ZeroMemory( &ddsd, sizeof(DDSURFACEDESC2) );
	ddsd.dwSize		= sizeof(DDSURFACEDESC2);
	ddsd.dwWidth	= TexHandle->Width ;
    ddsd.dwHeight	= TexHandle->Height;
	ddsd.dwFlags	= DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT;

	//알파 채널이 있으면 픽셀 포맷을 정해줘야함
	if ( TexHandle->NameA[0] ) {
		memcpy( &ddsd.ddpfPixelFormat, &ddTextureFormat, sizeof(DDPIXELFORMAT) );
	}
	else {
		memcpy( &ddsd.ddpfPixelFormat, &ddDefaultTextureFormat, sizeof(DDPIXELFORMAT) );
	}

	//######################################################################################
	//작 성 자 : 오 영 석
	if ( lpD3DDeviceDesc->bIsHardware )
	{
		//ddsd.ddsCaps.dwCaps = DDSCAPS_VIDEOMEMORY | DDSCAPS_TEXTURE;
		ddsd.ddsCaps.dwCaps  = DDSCAPS_TEXTURE;
		ddsd.ddsCaps.dwCaps2 = DDSCAPS2_TEXTUREMANAGE;
	}
    else
		ddsd.ddsCaps.dwCaps = DDSCAPS_SYSTEMMEMORY| DDSCAPS_TEXTURE;
	//######################################################################################

    lpDD->CreateSurface( &ddsd, &lpDDSurface, NULL );


	// 텍스쳐 핸들로 연결
	if ( lpDDSurface ) {
		hresult = lpDDSurface->QueryInterface( IID_IDirect3DTexture2, (LPVOID*)&lpD3DTexture );
		if ( hresult != D3D_OK ) { 
			lpDDSurface->Release();	
			
			LeaveCriticalSection( &cTextureSection );	//크리티칼 섹션 해제
			return FALSE; 
		}
	}
	else {
		LeaveCriticalSection( &cTextureSection );		//크리티칼 섹션 해제
		return FALSE;
	}

	ddRect.left		= 0;
	ddRect.right	= TexHandle->Width;
	ddRect.top		= 0;
	ddRect.bottom	= TexHandle ->Height;

	// 멋진 큰 그림을 새 텍스쳐로 복사
	lpDDSurface->BltFast( 0, 0, TexHandle->lpDDSSysMemory ,&ddRect , DDBLTFAST_WAIT);

	// 핸들 저장
	TexHandle->lpD3DTextureLarge  = lpD3DTexture;
	SwapTexture[cnt].lpD3DTexture = lpD3DTexture;
	SwapTexture[cnt].lpDDSurface  = lpDDSurface;
	SwapTexture[cnt].smTextureMaster = TexHandle;
	SwapTexture[cnt].UsedTime = SwapCount;
	SwapTexture[cnt].SwapState = TRUE;

	//비디오 메모리 사용 예상치
	VramTotal += TexHandle->Width * TexHandle->Height * 2; 

	//크리티칼 섹션 해제
	LeaveCriticalSection( &cTextureSection );


	return TRUE;
}

//텍스쳐 
int smTEXTURE::ReadTexture()
{
	//######################################################################################
	//작 성 자 : 오 영 석
	if( g_IsReadTextures == 0 )
		return 0;
	//######################################################################################

	int cnt;
	char szErrMsg[128];
	int	Result;
	int ReadCount = 0;

	for( cnt=0;cnt<MAX_TEXTURE;cnt++ ) {
		if ( Texture[cnt].UseCounter && !Texture[cnt].lpD3DTexture ) {

			EnterCriticalSection( &cTextureSection );			//크리티칼 섹션 선언

			//######################################################################################
			//작 성 자 : 오 영 석
			//Result = smCreateTexture( &Texture[cnt] );
			Result = new_smCreateTexture( &Texture[cnt] );
			//######################################################################################

			if ( Result<=0 ) {
				switch( Result ) {
				case 0:
					wsprintf( szErrMsg , "TEX_D3D \277\300\267\371 [%s]" , Texture[cnt].Name );
					break;
				case -1:
					wsprintf( szErrMsg , "TEX_FILE \277\300\267\371 [%s]" , Texture[cnt].Name );
					break;
				case -2:
					wsprintf( szErrMsg , "TEX_ALPHA \277\300\267\371 [%s]" , Texture[cnt].Name );
					break;
				}
				//오류 목록 삽입
				AddErrorList( szErrMsg );
			}
			ReadCount++;

			LeaveCriticalSection( &cTextureSection );		//크리티칼 섹션 해제

			if ( (ReadCount&0x7)==0 )	smPlayWindowMessage();	//메세지 처리
		}
	}

	return TRUE;
}

//잃어 버린 텍스쳐를 지우고 다시 불러 들인다
int smTEXTURE::Restore()
{
	int cnt;
	//int hresult;

	for( cnt=0; cnt< SWAPTEXTURE_MAX ; cnt++ ) {
		if ( SwapTexture[cnt].SwapState == TRUE ) {
			DeleteSwapTexture( cnt );
		}
	}


	EnterCriticalSection( &cTextureSection );			//크리티칼 섹션 선언

	for( cnt=0;cnt<MAX_TEXTURE;cnt++ ) {
		if ( Texture[cnt].UseCounter && Texture[cnt].lpD3DTexture ) {

			//hresult = Texture[cnt].lpDDSurface->IsLost();
			//if ( hresult!=DD_OK ) {

				if ( Texture[cnt].lpD3DTexture ) Texture[cnt].lpD3DTexture->Release();
				if ( Texture[cnt].lpDDSurface ) Texture[cnt].lpDDSurface->Release();
				if ( Texture[cnt].lpDDSSysMemory ) Texture[cnt].lpDDSSysMemory->Release();

				Texture[cnt].lpD3DTexture = 0;
				Texture[cnt].lpDDSurface = 0;
				Texture[cnt].lpDDSSysMemory = 0;
			//}
		}
	}
	LeaveCriticalSection( &cTextureSection );		//크리티칼 섹션 해제

	ReadTexture();

	return TRUE;
}
/*
		else {
			//서페이스가 이미 생성 되 있음
			hresult = lpTexHandle->lpDDSurface->GetBltStatus( DDGBS_CANBLT );
			if ( hresult==DDERR_SURFACELOST ) {
				//서페이스를 읽어 버린 경우
				lpTexHandle->lpDDSurface->Restore();
			}
		}
*/

smTEXTURE smTexture;

////////////////////// smMATERIAL_GROUP ////////////////////////


smMATERIAL_GROUP::smMATERIAL_GROUP()
{

}

smMATERIAL_GROUP::smMATERIAL_GROUP( int MaterialMax )
{
	Init( MaterialMax );
}

smMATERIAL_GROUP::~smMATERIAL_GROUP()
{
	Close();
}

// 메트리얼 초기화
void smMATERIAL_GROUP::Init( int MaterialMax )
{

	MaterialCount = 0;
	MaxMaterial = MaterialMax;
	ReformTexture = 0;

	smMaterial = new smMATERIAL[ MaterialMax ];
	ZeroMemory( smMaterial , sizeof( smMATERIAL )*MaterialMax );

}

// 메트리얼 말기화
int smMATERIAL_GROUP::Close()
{
	DWORD cnt , tcnt;

	for( cnt=0;cnt<MaterialCount;cnt++ ) {
		if ( smMaterial[cnt].InUse ) {
			//텍스쳐를 제거한다
			for( tcnt=0; tcnt<smMaterial[cnt].TextureCounter; tcnt++ ) 
				if ( smMaterial[cnt].smTexture[tcnt] )
					smTexture.Delete (  smMaterial[cnt].smTexture[tcnt]  );

			for( tcnt=0; tcnt<smMaterial[cnt].AnimTexCounter; tcnt++ ) 
				if ( smMaterial[cnt].smAnimTexture[tcnt] )
					smTexture.Delete (  smMaterial[cnt].smAnimTexture[tcnt]  );

		}
	}

	delete smMaterial;

	return TRUE;
}

#define TEXFILENAME_SIZE	64

//저장할 데이타의 크기를 미리 알려줌
int smMATERIAL_GROUP::GetSaveSize()
{
	int size;
	DWORD cnt ,tcnt;
	int	len,alen;

	size	 = sizeof( smMATERIAL_GROUP );

	for(cnt=0;cnt<MaterialCount;cnt++) {
		size+= sizeof( smMATERIAL );
		if ( smMaterial[cnt].InUse ) {
			size += sizeof(int);					//텍스쳐 이름 길이가 들어갈 int형
			for( tcnt=0; tcnt<smMaterial[cnt].TextureCounter ; tcnt++) {
				len = lstrlen( smMaterial[cnt].smTexture[tcnt]->Name )+1;
				alen = lstrlen( smMaterial[cnt].smTexture[tcnt]->NameA )+1;
				size += len;
				size += alen;
			}

			for( tcnt=0; tcnt<smMaterial[cnt].AnimTexCounter ; tcnt++) {
				len = lstrlen( smMaterial[cnt].smAnimTexture[tcnt]->Name )+1;
				alen = lstrlen( smMaterial[cnt].smAnimTexture[tcnt]->NameA )+1;
				size += len;
				size += alen;
			}

		}
	}
	return size;
}

//데이타를 파일로 저장 
int smMATERIAL_GROUP::SaveFile( HANDLE hFile )
{
	DWORD	dwAcess;
	DWORD cnt ,tcnt;
	int	len;
	int size;

	size =	WriteFile( hFile , &Head	, sizeof( smMATERIAL_GROUP )	, &dwAcess , NULL );

	for(cnt=0;cnt<MaterialCount;cnt++) {
		//메트리얼 저장
		size+= WriteFile( hFile , &smMaterial[cnt] , sizeof( smMATERIAL )	, &dwAcess , NULL );

		if ( smMaterial[cnt].InUse ) {

			//텍스쳐 파일 목록에 대한 이름 길이 계산
			len = 0;
			for( tcnt=0; tcnt<smMaterial[cnt].TextureCounter ; tcnt++) {
				len += lstrlen( smMaterial[cnt].smTexture[tcnt]->Name )+1;
				len += lstrlen( smMaterial[cnt].smTexture[tcnt]->NameA )+1;
			}
			for( tcnt=0; tcnt<smMaterial[cnt].AnimTexCounter ; tcnt++) {
				len += lstrlen( smMaterial[cnt].smAnimTexture[tcnt]->Name )+1;
				len += lstrlen( smMaterial[cnt].smAnimTexture[tcnt]->NameA )+1;
			}
			//문자열 길이 미리 계산
			size+= WriteFile( hFile , &len , sizeof(int) , &dwAcess , NULL );

			//텍스쳐 파일 이름을 연속적으로 저장
			for( tcnt=0; tcnt<smMaterial[cnt].TextureCounter ; tcnt++) {
				len = lstrlen( smMaterial[cnt].smTexture[tcnt]->Name )+1;
				size+= WriteFile( hFile , smMaterial[cnt].smTexture[tcnt]->Name , len , &dwAcess , NULL );
				len = lstrlen( smMaterial[cnt].smTexture[tcnt]->NameA )+1;
				size+= WriteFile( hFile , smMaterial[cnt].smTexture[tcnt]->NameA , len , &dwAcess , NULL );
			}

			for( tcnt=0; tcnt<smMaterial[cnt].AnimTexCounter ; tcnt++) {
				len = lstrlen( smMaterial[cnt].smAnimTexture[tcnt]->Name )+1;
				size+= WriteFile( hFile , smMaterial[cnt].smAnimTexture[tcnt]->Name , len , &dwAcess , NULL );
				len = lstrlen( smMaterial[cnt].smAnimTexture[tcnt]->NameA )+1;
				size+= WriteFile( hFile , smMaterial[cnt].smAnimTexture[tcnt]->NameA , len , &dwAcess , NULL );
			}

		}
	}

	return size;
}

//파일에서 데이타를 로드 
// Empty texture slot: an all-zero handle (no D3D texture, no opacity), so
// code that reads a material's texture never follows a junk/NULL pointer.
smTEXTUREHANDLE smNullTextureHandle;

int smMATERIAL_GROUP::LoadFile( HANDLE hFile )
{
	DWORD	dwAcess;
	DWORD	cnt ,tcnt;
	int		StrLen;
	int		size;
	char	szNameBuff[4096];
	char	*lpNameBuff;
	char	*szName , *szNameA;

	//그룹의 내용을 읽어옴
	size=ReadFile( hFile , &Head , sizeof( smMATERIAL_GROUP ) , &dwAcess , NULL );

	//메트리얼 메모리 잡음
	smMaterial = new smMATERIAL[ MaterialCount ];

	for(cnt=0;cnt<MaterialCount;cnt++) {
		//메트리얼 데이타 읽어옴
		size+=	ReadFile( hFile , &smMaterial[cnt] , sizeof( smMATERIAL ) , &dwAcess , NULL );

		// The texture handle pointers come from the file (old addresses).
		// Clear them so only the ones set up below are ever used; an unused
		// material or a slot past TextureCounter would otherwise hand the
		// renderer a junk pointer (crash in d3dim.dll SetTexture).
		for( tcnt=0; tcnt<8; tcnt++ ) smMaterial[cnt].smTexture[tcnt] = &smNullTextureHandle;
		for( tcnt=0; tcnt<32; tcnt++ ) smMaterial[cnt].smAnimTexture[tcnt] = &smNullTextureHandle;
		if ( smMaterial[cnt].TextureCounter>8 ) smMaterial[cnt].TextureCounter = 8;
		if ( smMaterial[cnt].AnimTexCounter>32 ) smMaterial[cnt].AnimTexCounter = 32;
		if ( !smMaterial[cnt].InUse ) {
			smMaterial[cnt].TextureCounter = 0;
			smMaterial[cnt].AnimTexCounter = 0;
		}

		if ( smMaterial[cnt].InUse ) {
			//텍스쳐 네임 버퍼 크기 읽음
			size+=	ReadFile( hFile , &StrLen , sizeof( int ) , &dwAcess , NULL );
			//텍스쳐 네임 버퍼 크기만큼 데이타 읽음 
			size+=	ReadFile( hFile , szNameBuff , StrLen, &dwAcess , NULL );

			lpNameBuff = szNameBuff;

			//텍스쳐 설정
			for( tcnt=0; tcnt<smMaterial[cnt].TextureCounter ; tcnt++) {
				szName = lpNameBuff;
				lpNameBuff += lstrlen( szName )+1;
				szNameA = lpNameBuff;
				lpNameBuff += lstrlen( szNameA )+1;

				if ( szNameA[0] )
					smMaterial[cnt].smTexture[tcnt] = smTexture.Add( szName , szNameA );
				else
					smMaterial[cnt].smTexture[tcnt] = smTexture.Add( szName );
				if ( !smMaterial[cnt].smTexture[tcnt] ) smMaterial[cnt].smTexture[tcnt] = &smNullTextureHandle;	//텍스쳐 목록이 가득 찬 경우
			}

			//에니메이션 텍스쳐 설정
			for( tcnt=0; tcnt<smMaterial[cnt].AnimTexCounter ; tcnt++) {
				szName = lpNameBuff;
				lpNameBuff += lstrlen( szName )+1;
				szNameA = lpNameBuff;
				lpNameBuff += lstrlen( szNameA )+1;

				if ( szNameA[0] )
					smMaterial[cnt].smAnimTexture[tcnt] = smTexture.Add( szName , szNameA );
				else
					smMaterial[cnt].smAnimTexture[tcnt] = smTexture.Add( szName );
				if ( !smMaterial[cnt].smAnimTexture[tcnt] ) smMaterial[cnt].smAnimTexture[tcnt] = &smNullTextureHandle;
			}
		}

	}

	return size;
}

//사용되지 않는 메트리얼 제거
int smMATERIAL_GROUP::DeleteUnuseMaterial()
{
	DWORD cnt , dcnt , tcnt;

	dcnt = 0;
	for( cnt=0;cnt<MaterialCount;cnt++ ) {
		if ( smMaterial[cnt].InUse==TRUE ) {
			//텍스쳐를 제거한다
			for( tcnt=0; tcnt<smMaterial[cnt].TextureCounter; tcnt++ ) 
				smTexture.Delete (  smMaterial[cnt].smTexture[tcnt]  );

			for( tcnt=0; tcnt<smMaterial[cnt].AnimTexCounter; tcnt++ ) 
				smTexture.Delete (  smMaterial[cnt].smAnimTexture[tcnt]  );

			ZeroMemory( &smMaterial[cnt] , sizeof( smMATERIAL ) );
			//smMaterial[cnt].InUse = FALSE;
			dcnt++;
		}
	}

	return dcnt;
}


//메트리얼을 등록
int smMATERIAL_GROUP::AddMaterial( ASE_MATERIAL *aseMaterial , char *szScript )
{
	int cnt,len;
	int	AnimCount,FrameSpeed;
	int MatNum;
	char	*lpAnimFileList[16];
	char	*szDirectory;
	char	*lpScript;
	char	szTemp[64];

	MatNum = MaterialCount;
	MaterialCount++;

	smMaterial[MatNum].InUse  = TRUE;
	smMaterial[MatNum].Shade  = TRUE;
	smMaterial[MatNum].BlendType = SMMAT_BLEND_ALPHA;
	smMaterial[MatNum].TwoSide = aseMaterial->TwoSide;
	smMaterial[MatNum].ReformTexture = 0;

	// 맵을 텍스쳐에 등록
	if ( !aseMaterial->MAP_OPACITY[0] ) {
		for( cnt=0;cnt<aseMaterial->TextureCounter;cnt++) {
			smMaterial[MatNum].smTexture[cnt] = smTexture.Add( aseMaterial->BITMAP[cnt] );
			smMaterial[MatNum].TextureStageState[cnt] = aseMaterial->BitmapStateState[cnt];
			smMaterial[MatNum].TextureFormState[cnt] = aseMaterial->BitmapFormState[cnt];
			smMaterial[MatNum].ReformTexture += aseMaterial->BitmapFormState[cnt];
		}
		smMaterial[MatNum].MapOpacity = FALSE;
	}
	else {
		//######################################################################################
		//작 성 자 : 오 영 석
		if( aseMaterial->TextureCounter > 1 )
		{
			for( cnt=0; cnt < aseMaterial->TextureCounter-1; cnt++ )
			{
				smMaterial[MatNum].smTexture[cnt] = smTexture.Add( aseMaterial->BITMAP[cnt], aseMaterial->MAP_OPACITY );
				smMaterial[MatNum].TextureStageState[cnt] = aseMaterial->BitmapStateState[cnt];
				smMaterial[MatNum].TextureFormState[cnt] = aseMaterial->BitmapFormState[cnt];
				smMaterial[MatNum].ReformTexture += aseMaterial->BitmapFormState[cnt];
			}

			smMaterial[MatNum].smTexture[cnt] = smTexture.Add( aseMaterial->BITMAP[cnt] );
			smMaterial[MatNum].TextureStageState[cnt] = aseMaterial->BitmapStateState[cnt];
			smMaterial[MatNum].TextureFormState[cnt] = aseMaterial->BitmapFormState[cnt];
			smMaterial[MatNum].ReformTexture += aseMaterial->BitmapFormState[cnt];
		}
		else
		{
			for( cnt=0; cnt < aseMaterial->TextureCounter; cnt++ )
			{
				smMaterial[MatNum].smTexture[cnt] = smTexture.Add( aseMaterial->BITMAP[cnt] , aseMaterial->MAP_OPACITY );
				smMaterial[MatNum].TextureStageState[cnt] = aseMaterial->BitmapStateState[cnt];
				smMaterial[MatNum].TextureFormState[cnt] = aseMaterial->BitmapFormState[cnt];
				smMaterial[MatNum].ReformTexture += aseMaterial->BitmapFormState[cnt];
			}
		}

		smMaterial[MatNum].MapOpacity = TRUE;
		//######################################################################################
	}

	//좌표 변형 텍스쳐 유무 첵크하여 저장
	if ( smMaterial[MatNum].ReformTexture )	ReformTexture ++;

	smMaterial[MatNum].TextureCounter = aseMaterial->TextureCounter;	//텍스쳐맵의 수
	smMaterial[MatNum].Transparency   = aseMaterial->Transparency ;		//반투명도
	smMaterial[MatNum].SelfIllum	  = aseMaterial->SelfIllum ;		//자체발광
	smMaterial[MatNum].Diffuse        = aseMaterial->Diffuse;			//기본색상

	smMaterial[MatNum].UseState			= aseMaterial->ScriptState;
	smMaterial[MatNum].TextureType		= SMTEX_TYPE_MULTIMIX;

	if( aseMaterial->BlendType )
		smMaterial[MatNum].BlendType = aseMaterial->BlendType;

	//바닥 충돌 체크
	if ( smMaterial[MatNum].Transparency==0 ) 
		smMaterial[MatNum].MeshState		= SMMAT_STAT_CHECK_FACE;

	//나무 흔들리기
	if ( (aseMaterial->ScriptState&sMATS_SCRIPT_WIND) ) {
		smMaterial[MatNum].WindMeshBottom = sMATS_SCRIPT_WINDZ1;
		smMaterial[MatNum].MeshState		= 0;
	}
	if ( (aseMaterial->ScriptState&sMATS_SCRIPT_WINDX1) ) {
		smMaterial[MatNum].WindMeshBottom = sMATS_SCRIPT_WINDX1;
		smMaterial[MatNum].MeshState		= 0;
	}
	if ( (aseMaterial->ScriptState&sMATS_SCRIPT_WINDX2) ) {
		smMaterial[MatNum].WindMeshBottom = sMATS_SCRIPT_WINDX2;
		smMaterial[MatNum].MeshState		= 0;
	}
	if ( (aseMaterial->ScriptState&sMATS_SCRIPT_WINDZ1) ) {
		smMaterial[MatNum].WindMeshBottom = sMATS_SCRIPT_WINDZ1;
		smMaterial[MatNum].MeshState		= 0;
	}
	if ( (aseMaterial->ScriptState&sMATS_SCRIPT_WINDZ2) ) {
		smMaterial[MatNum].WindMeshBottom = sMATS_SCRIPT_WINDZ2;
		smMaterial[MatNum].MeshState		= 0;
	}

	if ( (aseMaterial->ScriptState&sMATS_SCRIPT_WINDZ2) ) {
		smMaterial[MatNum].WindMeshBottom = sMATS_SCRIPT_WINDZ2;
		smMaterial[MatNum].MeshState		= 0;
	}

	if ( (aseMaterial->ScriptState&sMATS_SCRIPT_WATER) ) {
		smMaterial[MatNum].WindMeshBottom = sMATS_SCRIPT_WATER;
		smMaterial[MatNum].MeshState		= 0;
	}

	//메트리얼 충돌 체크 허가
	if ( (aseMaterial->ScriptState&sMATS_SCRIPT_NOTPASS) ) {
		smMaterial[MatNum].MeshState		= SMMAT_STAT_CHECK_FACE;	//허가
	}
	else {
		if ( (aseMaterial->ScriptState&sMATS_SCRIPT_PASS) ) {
			smMaterial[MatNum].MeshState		= 0;					//불가
		}
	}
	//레더링 나중에 그리기
	if ( (aseMaterial->ScriptState&sMATS_SCRIPT_RENDLATTER) ) {
		smMaterial[MatNum].MeshState |= sMATS_SCRIPT_RENDLATTER;
	}

	//######################################################################################
	//작 성 자 : 오 영 석
	if( (aseMaterial->ScriptState & sMATS_SCRIPT_CHECK_ICE) )
		smMaterial[MatNum].MeshState |= sMATS_SCRIPT_CHECK_ICE;
	//######################################################################################

	//######################################################################################
	//작 성 자 : 오 영 석
	if( (aseMaterial->ScriptState & sMATS_SCRIPT_ORG_WATER) )
		smMaterial[MatNum].MeshState = sMATS_SCRIPT_ORG_WATER;
	//######################################################################################

	//자동 에니메이션
	AnimCount = 0;
	if ( szScript ) {
		if ( aseMaterial->ScriptState&sMATS_SCRIPT_ANIM2 ) AnimCount=2;
		if ( aseMaterial->ScriptState&sMATS_SCRIPT_ANIM4 ) AnimCount=4;
		if ( aseMaterial->ScriptState&sMATS_SCRIPT_ANIM8 ) AnimCount=8;
		if ( aseMaterial->ScriptState&sMATS_SCRIPT_ANIM16 ) AnimCount=16;
	}

	if ( AnimCount ) {
		len = lstrlen(szScript);

		for( cnt=0;cnt<len;cnt++) {
			if ( szScript[cnt]=='a' && szScript[cnt+1]=='n' && 
				szScript[cnt+2]=='i' && szScript[cnt+3]=='m' ) {
					for( cnt=cnt;cnt<len;cnt++) {
						if ( szScript[cnt]==':' ) break;
					}
					break;
			}
		}

		if ( cnt<len ) {
			cnt++;
			lpScript = &szScript[cnt];
			FrameSpeed = 0;

			for(cnt=cnt;cnt<len;cnt++) {
				if ( szScript[cnt]==':' ) {
					FrameSpeed = atoi( &szScript[cnt+1] );
					szScript[cnt] = 0;
					break;
				}
			}

			szDirectory = SetDirectoryFromFile( aseMaterial->BITMAP[0] );

			if ( szDirectory ) {
				for(cnt=0;cnt<AnimCount;cnt++) {
					lpAnimFileList[cnt] = new char[64];
					lstrcpy( szTemp , szDirectory );
					lstrcat( szTemp , lpScript );
					wsprintf( lpAnimFileList[cnt] , szTemp , cnt );
				}
			
				ChangeMaterialToAnimation( &smMaterial[MatNum] , lpAnimFileList , AnimCount , smMaterial[MatNum].BlendType , TRUE );
				if ( FrameSpeed ) smMaterial[MatNum].Shift_FrameSpeed = FrameSpeed;			//에니메이션 타이머 쉬프트

				smMaterial[MatNum].Transparency   = aseMaterial->Transparency ;		//반투명도

				for(cnt=AnimCount-1;cnt>=0;cnt--) delete lpAnimFileList[cnt];
			}
		}
	}

	//######################################################################################
	// 작 성 자 : 오 영 석
	if( (aseMaterial->ScriptState & sMATS_SCRIPT_BLINK_COLOR) && smMaterial[MatNum].WindMeshBottom == 0 )
	{
		for( cnt=0; cnt < MAX_MAP_BLINK_COLOR_TIME; cnt++ )
		{
			if( strstr( szScript, szBlinkTimeScript[cnt] ) != 0 )
				break;
		}

		if( cnt >= MAX_MAP_BLINK_COLOR_TIME )
			smMaterial[MatNum].WindMeshBottom = dwBlinkTimeCode[0];
		else
			smMaterial[MatNum].WindMeshBottom = dwBlinkTimeCode[cnt];
	}
	//######################################################################################

	aseMaterial->Regist ++;												//메트리얼 목록에 등록
	aseMaterial->RegistNum = MatNum;

	return MatNum;
}

//텍스쳐 메트리얼 생성
int smMATERIAL_GROUP::CreateTextureMaterial( char *szBitmap , char *szOpacity , int TwoSide ,int Shade , DWORD TexSwap , DWORD BlendType )
{
	int MatNum;


	MatNum = MaterialCount;
	MaterialCount++;

	smMaterial[MatNum].InUse  = TRUE;
	smMaterial[MatNum].Shade  = Shade;
	smMaterial[MatNum].TwoSide = TwoSide;

	if ( szOpacity ) 
		smMaterial[MatNum].smTexture[0] = smTexture.Add( szBitmap , szOpacity );
	else 
		smMaterial[MatNum].smTexture[0] = smTexture.Add( szBitmap );


	smMaterial[MatNum].ReformTexture = 0;

	smMaterial[MatNum].TextureStageState[0] = 0;
	smMaterial[MatNum].TextureFormState[0] = 0;

	smMaterial[MatNum].smTexture[0]->TexSwapMode = TexSwap;
	smMaterial[MatNum].MapOpacity = FALSE;

	smMaterial[MatNum].TextureCounter = 1;			//텍스쳐맵의 수
	smMaterial[MatNum].Transparency   = 0;			//반투명도
//	smMaterial[MatNum].Diffuse        = 0;			//기본색상

	smMaterial[MatNum].UseState			= 0;
	smMaterial[MatNum].TextureType		= SMTEX_TYPE_MULTIMIX;
	smMaterial[MatNum].BlendType		= BlendType;


	smMaterial[MatNum].InUse++;

	return MatNum;
}

//메트리얼에 텍스쳐를 추가한다
int	smMATERIAL_GROUP::AddTexture( int MatNum , char *szBitmap , char *szOpacity )
{
	int cnt;

	cnt = smMaterial[MatNum].TextureCounter;			//텍스쳐맵의 수

	if ( szOpacity ) 
		smMaterial[MatNum].smTexture[cnt] = smTexture.Add( szBitmap , szOpacity );
	else 
		smMaterial[MatNum].smTexture[cnt] = smTexture.Add( szBitmap );

	if ( smMaterial[MatNum].smTexture[cnt] )
		smMaterial[MatNum].TextureCounter++;

	return TRUE;
}

//텍스쳐 이름으로 다음 메트리얼을 찾는다
smMATERIAL *smMATERIAL_GROUP::FindNextMaterialFromTextureName()
{
	DWORD cnt,cnt2;

	char *szName;

	szName = szLastSearchName;


	//빈 메트리얼 찾기
	for( cnt=LastSearchMaterial;cnt<MaterialCount;cnt++) {
		if ( smMaterial[cnt].InUse ) {

			for(cnt2=0;cnt2<smMaterial[cnt].TextureCounter;cnt2++) {
				if ( lstrcmpi( smMaterial[cnt].smTexture[cnt2]->Name , szName )==0 ) {
					return	&smMaterial[cnt];
				}
			}

			for(cnt2=0;cnt2<smMaterial[cnt].AnimTexCounter;cnt2++) {
				if ( lstrcmpi( smMaterial[cnt].smAnimTexture[cnt2]->Name , szName )==0 ) {
					return	&smMaterial[cnt];
				}
			}

		}
	}

	return NULL;
}

//텍스쳐 이름으로 메트리얼을 찾는다
smMATERIAL *smMATERIAL_GROUP::FindMaterialFromTextureName( char *szName )
{
	LastSearchMaterial = 0;

	lstrcpy( szLastSearchName, szName );

	return FindNextMaterialFromTextureName();
}

//메트리얼의 에니메이션 프레임을 설정
int smMATERIAL_GROUP::SetMaterialAnimFrame( smMATERIAL *lpMaterial , int frame )
{
	int OldFrame;

	OldFrame = lpMaterial->AnimationFrame;
	lpMaterial->AnimationFrame = frame;

	return OldFrame;
}



//메트리얼을 에니메이션 가능하게 변경
int smMATERIAL_GROUP::ChangeMaterialToAnimation( smMATERIAL *lpMaterial , char **szBitmap , int NumTex , DWORD BlendType , int AutoAnimMode )
{
	int cnt;

	for( cnt=0;cnt<NumTex;cnt++) {
		lpMaterial->smAnimTexture[cnt] = smTexture.Add( szBitmap[cnt] );
		lpMaterial->smAnimTexture[cnt]->TexSwapMode = FALSE;
	}


	lpMaterial->MapOpacity = FALSE;
	lpMaterial->TextureCounter = 1;//NumTex;		//텍스쳐맵의 수
	lpMaterial->Transparency   = 0;			//반투명도

	lpMaterial->AnimTexCounter	= NumTex;		//에니메이션 텍스쳐 카운터
	lpMaterial->FrameMask		= NumTex -1;	//에니메이션 프레임 마스크
	lpMaterial->Shift_FrameSpeed = 6;			//에니메이션 타이머 쉬프트

	lpMaterial->UseState			= 0;
	lpMaterial->TextureType		= SMTEX_TYPE_ANIMATION;
	lpMaterial->BlendType		= BlendType;

	//에니메이션 자동
	if ( AutoAnimMode )	
		lpMaterial->AnimationFrame	= SMTEX_AUTOANIMATION;
	else
		lpMaterial->AnimationFrame	= 0;

	lpMaterial->InUse++;

	return TRUE;
}


//애니메이션 메트리얼 생성
int smMATERIAL_GROUP::CreateAnimationMaterial( char **szBitmap , int NumTex , DWORD BlendType , int AutoAnimMode )
{
	int cnt;

	smMATERIAL *lpMaterial;

	cnt = MaterialCount;
	MaterialCount++;

	lpMaterial = &smMaterial[cnt];

	lpMaterial->InUse  = TRUE;
	lpMaterial->Shade  = FALSE;
	lpMaterial->TwoSide = TRUE;

	ChangeMaterialToAnimation( lpMaterial , szBitmap , NumTex , BlendType , AutoAnimMode );

	return cnt;
}



//텍스쳐 갱신용 프레임
int smMATERIAL_GROUP::IncTextureFrame()
{
	return TRUE;
}



//메트리얼의 텍스쳐 스와핑값을 조사후 설정한다 ( 로딩 완료후 가능 )
int smMATERIAL_GROUP::CheckMatreialTextureSwap()
{
	//######################################################################################
	//작 성 자 : 오 영 석
    return TRUE;
	//######################################################################################

	DWORD cnt , cnt2;

	//빈 메트리얼 찾기
	for( cnt=0;cnt<MaterialCount;cnt++) 
		if ( smMaterial[cnt].InUse ) {
			if ( smMaterial[cnt].TextureType==SMTEX_TYPE_MULTIMIX ) {
				for( cnt2=0; cnt2<smMaterial[cnt].TextureCounter; cnt2++ ) {
					if ( smMaterial[cnt].smTexture[cnt2] && smMaterial[cnt].smTexture[cnt2]->lpDDSSysMemory ) {
						smMaterial[cnt].TextureSwap = TRUE;
						break;
					}
				}
			}

			if ( smMaterial[cnt].TextureType==SMTEX_TYPE_ANIMATION ) {
				for( cnt2=0; cnt2<smMaterial[cnt].AnimTexCounter; cnt2++ ) {
					if ( smMaterial[cnt].smAnimTexture[cnt2] && smMaterial[cnt].smAnimTexture[cnt2]->lpDDSSysMemory ) {
						smMaterial[cnt].TextureSwap = TRUE;
						break;
					}
				}
			}

		}

	return TRUE;
}


//메트리얼의 스와핑 텍스쳐를 찾아 설정한다
int smMATERIAL_GROUP::ReSwapMaterial( smMATERIAL *lpMaterial )
{
	//######################################################################################
	//작 성 자 : 오 영 석
    return TRUE;
	//######################################################################################

	DWORD cnt;
	int result;

	lpMaterial->MatFrame = MaterialFrame;

	lpMaterial->TextureClip = FALSE;

	if ( lpMaterial->TextureSwap ) {
		result = 0;
		for( cnt=0;cnt<lpMaterial->TextureCounter;cnt++) {
			result |= smTexture.SetLargeTexture( lpMaterial->smTexture[cnt] );
		}

		if ( result ) {
			lpMaterial->TextureClip = TRUE;
			return TRUE;
		}
	}

	return FALSE;
}

//######################################################################################
//작 성 자 : 오 영 석
//해당 메트리얼의 모든 텍스쳐를 로딩
int smMATERIAL_GROUP::ReadTextures( int MipMap )
//######################################################################################
{
	//######################################################################################
	//작 성 자 : 오 영 석
	if( g_IsReadTextures == 0 )
		return 0;
	//######################################################################################

	DWORD cnt;
	DWORD tcnt;
	int	Result;
	int	ReadCount;
	smTEXTUREHANDLE	*lpTexture;

	//텍스쳐 영역 크리티칼 섹션 선언
	smEnterTextureCriticalSection();

	ReadCount = 0;

#ifdef PT_WEB
	{	// every texture this loop will read, to the service worker first (downloads in parallel)
		std::string list;
		int n = 0;
		for( cnt=0;cnt<MaterialCount;cnt++ ) {
			if ( !smMaterial[cnt].InUse ) continue;
			for( tcnt=0; tcnt<smMaterial[cnt].TextureCounter; tcnt++ ) {
				lpTexture = smMaterial[cnt].smTexture[tcnt];
				if ( lpTexture && lpTexture->UseCounter && !lpTexture->lpD3DTexture && lpTexture->Name[0] ) { list += lpTexture->Name; list += '\n'; n++; }
			}
			for( tcnt=0; tcnt<smMaterial[cnt].AnimTexCounter; tcnt++ ) {
				lpTexture = smMaterial[cnt].smAnimTexture[tcnt];
				if ( lpTexture && lpTexture->UseCounter && !lpTexture->lpD3DTexture && lpTexture->Name[0] ) { list += lpTexture->Name; list += '\n'; n++; }
			}
		}
		PrefetchHint( list, n );
	}
#endif

	for( cnt=0;cnt<MaterialCount;cnt++ ) {
		if ( smMaterial[cnt].InUse ) {
			for( tcnt=0; tcnt<smMaterial[cnt].TextureCounter; tcnt++ ) {
				if ( smMaterial[cnt].smTexture[tcnt] ) {
					lpTexture = smMaterial[cnt].smTexture[tcnt];

					if ( lpTexture->UseCounter && !lpTexture->lpD3DTexture )
					{
						//######################################################################################
						//작 성 자 : 오 영 석
						//Result = smCreateTexture( lpTexture, MipMap );
						Result = new_smCreateTexture( lpTexture, MipMap );
						//######################################################################################

						ReadCount++;
					}
				}
			}

			for( tcnt=0; tcnt<smMaterial[cnt].AnimTexCounter; tcnt++ ) {
				if ( smMaterial[cnt].smAnimTexture[tcnt] ) {
					lpTexture = smMaterial[cnt].smAnimTexture[tcnt];
					if ( lpTexture->UseCounter && !lpTexture->lpD3DTexture )
					{
						//######################################################################################
						//작 성 자 : 오 영 석
						//Result = smCreateTexture( lpTexture, MipMap );
						Result = new_smCreateTexture( lpTexture, MipMap );
						//######################################################################################

						ReadCount++;
					}
				}
			}
		}
	}

	//텍스쳐 영역 크리티칼 섹션 해제
	smLeaveTextureCriticalSection();

	return ReadCount;
}































////////////////////////////////////////////////////////////////////////////////////////////////


//smMATERIAL smMaterial[ MATRIAL_MAX ];
smMATERIAL *smMaterial;

smMATERIAL_GROUP	*smMaterialGroup;

// 메트리얼 초기화
int InitMaterial()
{
//	smMaterialGroup.Init( MATRIAL_MAX );
	smMaterialGroup = new smMATERIAL_GROUP( MATRIAL_MAX );
	smMaterial = smMaterialGroup->smMaterial;

	MaterialFrame = 0;

	return TRUE;
}

//메트리얼 종료
int CloseMaterial()
{
//	smMaterialGroup.Close();
	delete smMaterialGroup;
	return TRUE;
}

//사용되지 않는 메트리얼 제거
int DeleteUnuseMaterial()
{
	return smMaterialGroup->DeleteUnuseMaterial();
}

//메트리얼을 등록
int AddMaterial( ASE_MATERIAL *aseMaterial )
{
	return smMaterialGroup->AddMaterial( aseMaterial );
}

//텍스쳐 메트리얼 생성
int CreateTextureMaterial( char *szBitmap , char *szOpacity , int TwoSide ,int Shade , DWORD TexSwap , DWORD BlendType )
{
	return smMaterialGroup->CreateTextureMaterial( szBitmap, szOpacity, TwoSide ,Shade ,TexSwap ,BlendType);
}

//메트리얼에 텍스쳐를 추가한다
int	AddTexture( int MatNum , char *szBitmap , char *szOpacity )
{
	return smMaterialGroup->AddTexture( MatNum , szBitmap , szOpacity );
}

//텍스쳐 이름으로 다음 메트리얼을 찾는다
smMATERIAL *FindNextMaterialFromTextureName()
{
	return smMaterialGroup->FindNextMaterialFromTextureName();
}

//텍스쳐 이름으로 메트리얼을 찾는다
smMATERIAL *FindMaterialFromTextureName( char *szName )
{
	return smMaterialGroup->FindMaterialFromTextureName( szName );
}

//메트리얼의 에니메이션 프레임을 설정
int SetMaterialAnimFrame( smMATERIAL *lpMaterial , int frame )
{
	return smMaterialGroup->SetMaterialAnimFrame( lpMaterial , frame );
}


//메트리얼을 에니메이션 가능하게 변경
int ChangeMaterialToAnimation( smMATERIAL *lpMaterial , char **szBitmap , int NumTex , DWORD BlendType , int AutoAnimMode )
{
	return smMaterialGroup->ChangeMaterialToAnimation( lpMaterial, szBitmap, NumTex, BlendType, AutoAnimMode );
}


//애니메이션 메트리얼 생성
int CreateAnimationMaterial( char **szBitmap , int NumTex , DWORD BlendType , int AutoAnimMode )
{
	return smMaterialGroup->CreateAnimationMaterial( szBitmap, NumTex, BlendType, AutoAnimMode );
}


//텍스쳐 갱신용 프레임
int IncTextureFrame()
{
	MaterialFrame++;
	return smTexture.SwapCount ++;
}

//메트리얼의 텍스쳐 스와핑값을 조사후 설정한다 ( 로딩 완료후 가능 )
int CheckMatreialTextureSwap()
{
	return smMaterialGroup->CheckMatreialTextureSwap();
}


//메트리얼의 스와핑 텍스쳐를 찾아 설정한다
int ReSwapMaterial( smMATERIAL *lpMaterial )
{
	return smMaterialGroup->ReSwapMaterial( lpMaterial );
}




struct smTEXTUREFORMAT {
	DWORD	BPP_DW;
	DWORD	MinAlpha;			//알파비트 취소치 ( 0이면 없음 )
	BOOL	bUsePalette;		//팔레트 사용 여부
	BOOL	bUseFourCC;			//FourCC 옵션 ( FourCC?? 압축관련으로 추정 )
	BOOL	bFoundGoodFormat;	//발견 확인
	DDPIXELFORMAT	*pddpf;
};



// 텍스쳐 포맷 찾기 위한 CALLBACK 함수
static HRESULT CALLBACK TextureSearchCallback( DDPIXELFORMAT* pddpf,
											   VOID* param )
{
    if( NULL==pddpf || NULL==param )
        return DDENUMRET_OK;

	smTEXTUREFORMAT* ptsi = (smTEXTUREFORMAT*)param;

    // Skip any funky modes
    if( pddpf->dwFlags & (DDPF_LUMINANCE|DDPF_BUMPLUMINANCE|DDPF_BUMPDUDV) )
        return DDENUMRET_OK;

	// Check for palettized formats
	if( ptsi->bUsePalette )
	{
		if( !( pddpf->dwFlags & DDPF_PALETTEINDEXED8 ) )
			return DDENUMRET_OK;

		// Accept the first 8-bit palettized format we get
        memcpy( ptsi->pddpf, pddpf, sizeof(DDPIXELFORMAT) );
		ptsi->bFoundGoodFormat = TRUE;
        return DDENUMRET_CANCEL;
    }

	// Else, skip any paletized formats (all modes under 16bpp)
	if( pddpf->dwRGBBitCount < 16 )
		return DDENUMRET_OK;

	// Check for FourCC formats
    if( ptsi->bUseFourCC )
	{
		if( pddpf->dwFourCC == 0 )
		    return DDENUMRET_OK;

		return DDENUMRET_CANCEL;
	}

	// Else, skip any FourCC formats
	if( pddpf->dwFourCC != 0 )
		return DDENUMRET_OK;

	// Make sure current alpha format agrees with requested format type
	if( (ptsi->MinAlpha>0) && !(pddpf->dwFlags&DDPF_ALPHAPIXELS) )
		return DDENUMRET_OK;
	if( (ptsi->MinAlpha==0) && (pddpf->dwFlags&DDPF_ALPHAPIXELS) )
		return DDENUMRET_OK;

	DWORD	AlphaBitTest;
	int cnt;

	if( ptsi->MinAlpha>0) {
		AlphaBitTest = pddpf->dwRGBAlphaBitMask;
		for(cnt=0;cnt<32;cnt++) {
			if ( (AlphaBitTest&1) ) break;
			AlphaBitTest>>=1;
		}
		if ( ptsi->MinAlpha>AlphaBitTest ) 
			return DDENUMRET_OK;
	}

    // Check if we found a good match
    if( pddpf->dwRGBBitCount == ptsi->BPP_DW )
    {
        memcpy( ptsi->pddpf, pddpf, sizeof(DDPIXELFORMAT) );
		ptsi->bFoundGoodFormat = TRUE;
        return DDENUMRET_CANCEL;
    }

    return DDENUMRET_OK;
}

int GetPixelFormatAlphaTexture()
{
	smTEXTUREFORMAT TextureFormat;

	if ( smTextureBPP==32 ) TextureFormat.BPP_DW = 32;
	else TextureFormat.BPP_DW = 16;

	TextureFormat.MinAlpha = 8;				//알파 최소치 ( 8단계 이상의 앞파채널 )
	TextureFormat.bUseFourCC = FALSE;
	TextureFormat.bUsePalette = FALSE;
	TextureFormat.bFoundGoodFormat = FALSE;
	TextureFormat.pddpf = &ddTextureFormat;
	lpD3DDevice->EnumTextureFormats( TextureSearchCallback , &TextureFormat );

	if ( smTextureBPP==32 ) TextureFormat.BPP_DW = 32;
	else TextureFormat.BPP_DW = 16;

	TextureFormat.MinAlpha = 0;				//알파없음
	TextureFormat.bUseFourCC = FALSE;
	TextureFormat.bUsePalette = FALSE;
	TextureFormat.bFoundGoodFormat = FALSE;
	TextureFormat.pddpf = &ddDefaultTextureFormat;
	lpD3DDevice->EnumTextureFormats( TextureSearchCallback , &TextureFormat );

	return TRUE;

}



int InitTexture()
{
	smTexture.Init();

	GetPixelFormatAlphaTexture();			//알파 텍스쳐 픽셀포맷 찾기

	return TRUE;
}

int CloseTexture()
{
	smTexture.Remove();

	return TRUE;
}

int ReadTextures()
{
	//######################################################################################
	//작 성 자 : 오 영 석
	if( g_IsReadTextures == 0 )
		return 0;
	//######################################################################################

	int val;

	val = smTexture.ReadTexture();
	CheckMatreialTextureSwap();

	return val;
}

//텍스쳐를 첵크하여 잃어버린 텍스쳐를 다시 만든다
int RestoreTextures()
{
	return smTexture.Restore();
}

/*
int LoadDibTextureNum( char *Filename )
{

	if ( smTexture.AddName( Filename ) )
		return smTexture.TextureCnt;
	else 
		return -1;
}
*/


LPDIRECT3DTEXTURE2 GetTextureHandle( int num )
{
	return smTexture.Texture[ num ].lpD3DTexture ;
}

LPDIRECT3DTEXTURE2 GetTextureChildHandle( int num )
{
	if ( smTexture.Texture[num].TexChild )
		return smTexture.Texture[ num ].TexChild->lpD3DTexture ;
	else 
		return NULL;
}



LPDIRECTDRAWSURFACE4 *GetLastTextureSurface()
{
	return smTexture.lpLastTextureSurface;
}

LPDIRECT3DTEXTURE2 *GetLastTexture()
{
	return smTexture.lpLastTexture ;
}

//텍스쳐 영역 크리티칼 섹션 선언
void smEnterTextureCriticalSection()
{
	EnterCriticalSection( &smTexture.cTextureSection );
}

//텍스쳐 영역 크리티칼 섹션 해제
void smLeaveTextureCriticalSection()
{
	LeaveCriticalSection( &smTexture.cTextureSection );
}

int hresult;


////////////////// BMP FILE READ ///////////////////
DWORD GetDibInfoHeaderSize(BYTE  *lpDib)
{
	return ((BITMAPINFOHEADER  *)lpDib)->biSize;
}
WORD GetDibWidth(BYTE  *lpDib)
{
	if (GetDibInfoHeaderSize(lpDib)+sizeof(BITMAPCOREHEADER))
		return (WORD) (((BITMAPCOREHEADER  *)lpDib)->bcWidth);
	else
		return (WORD) (((BITMAPINFOHEADER  *)lpDib)->biWidth);
}
WORD GetDibHeight (BYTE  *lpDib)
{
	if (GetDibInfoHeaderSize(lpDib)==sizeof(BITMAPCOREHEADER))
		return (WORD) (((BITMAPCOREHEADER  *) lpDib)->bcHeight);
	else
		return (WORD) (((BITMAPINFOHEADER  *) lpDib)->biHeight);
}

WORD GetDibBitCount(BYTE  *lpDib)
{
	return ((BITMAPINFOHEADER  *)lpDib)->biBitCount;
}

BYTE  *GetDibBitsAddr(BYTE  *lpDib)
{
	DWORD dwNumColors, dwColorTableSize;
	WORD  wBitCount;

	if (GetDibInfoHeaderSize(lpDib)==sizeof(BITMAPCOREHEADER))
	{
		wBitCount=((BITMAPCOREHEADER  *) lpDib)->bcBitCount;
		if(wBitCount!=24)
			dwNumColors=1L<<wBitCount;
		else dwNumColors=0;
		dwColorTableSize=dwNumColors*sizeof(RGBTRIPLE);
	}
	else
	{
		wBitCount=((BITMAPINFOHEADER  *)lpDib)->biBitCount;
		if(GetDibInfoHeaderSize(lpDib)>=36)
			dwNumColors=((BITMAPINFOHEADER  *)lpDib)->biClrUsed;
		else
			dwNumColors=0;
		if(dwNumColors==0)
		{
			if(wBitCount!=24)
				dwNumColors=1L<<wBitCount;
			else
				dwNumColors=0;
		}
		dwColorTableSize=dwNumColors*sizeof(RGBQUAD);
	}
	return lpDib+GetDibInfoHeaderSize(lpDib)+dwColorTableSize;
}



/////////////////////////[ BMP 파일 로드 ]///////////////////////////////
BYTE *LoadDib( char *Filename )
{
	BITMAPFILEHEADER	bmfh;
	BYTE 				*lpDib;
	DWORD				dwDibSize,dwHeaderSize;
	HANDLE	hFile;
	DWORD	dwAcess;


	//파일을 연다
	hFile = CreateFile( Filename , GENERIC_READ , FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING , FILE_ATTRIBUTE_NORMAL , NULL );
	if ( hFile == INVALID_HANDLE_VALUE ) return NULL;
	//비트맵 헤더 읽음
	ReadFile( hFile , &bmfh , sizeof( BITMAPFILEHEADER ) , &dwAcess , NULL );
	if (bmfh.bfType != *(WORD *)"BM" && bmfh.bfType != *(WORD *)"C8" && bmfh.bfType != *(WORD *)"A8" ) 
	{ CloseHandle( hFile ); return NULL; }

	//헤더 암호화 버전
	if ( bmfh.bfType == *(WORD *)"A8" ) {
		BYTE	ch;
		BYTE	*lpHeader;
		int		cnt;

		//헤더를 암호화
		lpHeader = (BYTE *)&bmfh;
		for( cnt=0;cnt<sizeof(bmfh);cnt++) {
			ch = (BYTE)( lpHeader[cnt]-(cnt*cnt) );
			lpHeader[cnt] = ch;
		}
	}

	dwDibSize=bmfh.bfSize - sizeof(BITMAPFILEHEADER);
	lpDib = new BYTE [dwDibSize];
	if(lpDib==NULL)	{ CloseHandle( hFile );	return NULL; }
	//비트맵 읽음
	ReadFile( hFile , lpDib , dwDibSize , &dwAcess , NULL );
	CloseHandle( hFile );
	dwHeaderSize=GetDibInfoHeaderSize(lpDib);
	if (dwHeaderSize<12||(dwHeaderSize>12 && dwHeaderSize < 16) ) 
	{	delete lpDib; return NULL; }

	return lpDib;
}

//////////////////// TGA 파일 읽어들이는 크래스 //////////////////
class cTGA	{
public:
	BYTE	*m_pBuff;
	BYTE	*m_pData;
	int		Width , Height;
	int		SwapHeight;
	int		BitCount;

	cTGA();
	~cTGA();

	int LoadTga( char *Filename );
	int GetWidth();
	int GetHeight();
	int GetBitCount();

};

cTGA::cTGA()
{
	m_pBuff = 0;
	m_pData = 0;
	Width = 0;
	Height = 0;
	BitCount = 0;
}

cTGA::~cTGA()
{
	if ( m_pBuff ) delete m_pData;
}

// TGA파일 로드
int cTGA::LoadTga( char *Filename )
{
	int len;
	BYTE TGAHeader[0x12];
	WORD *Bit16Buff;
	WORD r,g,b;

	int DestCnt;
	int SrcCnt;

	HANDLE	hFile;
	DWORD	dwAcess;

	//파일을 연다
	hFile = CreateFile( Filename , GENERIC_READ , FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING , FILE_ATTRIBUTE_NORMAL , NULL );
	if ( hFile == INVALID_HANDLE_VALUE ) return FALSE;

	//TGA 헤더 읽음
	ReadFile( hFile , TGAHeader , 0x12 , &dwAcess , NULL );

	//헤더 암호화 버전
	if ( TGAHeader[0]=='G' && TGAHeader[1]=='8' ) {
		BYTE	ch;
		BYTE	*lpHeader;
		int		cnt;

		//헤더를 암호화
		lpHeader = (BYTE *)&TGAHeader;
		for( cnt=0;cnt<sizeof(TGAHeader);cnt++) {
			ch = (BYTE)( lpHeader[cnt]-(cnt*cnt) );
			lpHeader[cnt] = ch;
		}
	}

	Width = *((WORD *)(&TGAHeader[0xC]));
	Height = *((WORD *)(&TGAHeader[0xE]));

	if ( (TGAHeader[0x11]&0x20) ) {
		SwapHeight = FALSE;
	}
	else
		SwapHeight = TRUE;

	switch( TGAHeader[0x10] ) {
	case 16:		//555 를 RGB 888 로 변환
		m_pBuff = new BYTE [Width*Height*3];
		len = Width*Height;
		Bit16Buff = new WORD [ len ];
		ReadFile( hFile , Bit16Buff , len*2 , &dwAcess , NULL );
		//fread( Bit16Buff , len*2 , 1, fp );
		DestCnt = 0;
		for( SrcCnt=0;SrcCnt<len;SrcCnt++ ) {
			r = ( Bit16Buff[SrcCnt]>>10 ) & 0x1F;
			g = ( Bit16Buff[SrcCnt]>>5 ) & 0x1F;
			b = ( Bit16Buff[SrcCnt] ) & 0x1F;

			m_pBuff[ DestCnt   ] = (r<<3) | (r>>2);
			m_pBuff[ DestCnt+1 ] = (g<<3) | (g>>2);
			m_pBuff[ DestCnt+2 ] = (b<<3) | (b>>2);
			DestCnt+=3;
		}
		delete Bit16Buff;
		BitCount = 24;
		break;
	case 24:
		m_pBuff = new BYTE [Width*Height*3];
		ReadFile( hFile , m_pBuff , Width*Height*3 , &dwAcess , NULL );
		//fread( m_pBuff , Width*Height*3 , 1, fp );
		BitCount = 24;
		break;
	case 32:
		m_pBuff = new BYTE [Width*Height*4];
		ReadFile( hFile , m_pBuff , Width*Height*4 , &dwAcess , NULL );
		//fread( m_pBuff , Width*Height*4 , 1, fp );
		BitCount = 32;
		break;
	}
	CloseHandle( hFile );

	m_pData = m_pBuff;

	return TRUE;
}

int cTGA::GetWidth()
{
	return Width;
}

int cTGA::GetHeight()
{
	return Height;
}

int cTGA::GetBitCount()
{
	return BitCount;
}

#define smTEXFILETYPE_BMP		0
#define smTEXFILETYPE_JPG		1
#define smTEXFILETYPE_TGA		2

//파일의 확장자를 검사하여 파일 포맷을 찾는다
int GetTextureFileType( char *Filename ) 
{
	int len;
	int cnt;
	char *szFileExt = 0;

	len = lstrlen( Filename );

	for(cnt=len-1;cnt>=0;cnt--) {
		if ( Filename[cnt]=='.' ) {
			szFileExt = Filename+cnt+1;
			break;
		}
	}

	if ( !szFileExt ) 
		return -1;

	if ( ( szFileExt[0]=='j' || szFileExt[0]=='J') && 
		 ( szFileExt[1]=='p' || szFileExt[1]=='P') &&
		 ( szFileExt[2]=='g' || szFileExt[2]=='G') ) 
		 return smTEXFILETYPE_JPG;

	if ( ( szFileExt[0]=='b' || szFileExt[0]=='B') && 
		 ( szFileExt[1]=='m' || szFileExt[1]=='M') &&
		 ( szFileExt[2]=='p' || szFileExt[2]=='P') ) 
		 return smTEXFILETYPE_BMP;

	if ( ( szFileExt[0]=='t' || szFileExt[0]=='T') && 
		 ( szFileExt[1]=='g' || szFileExt[1]=='G') &&
		 ( szFileExt[2]=='a' || szFileExt[2]=='A') ) 
		 return smTEXFILETYPE_TGA;


	if ( ( szFileExt[0]=='s' || szFileExt[0]=='S') && 
		 ( szFileExt[1]=='t' || szFileExt[1]=='T') &&
		 ( szFileExt[2]=='g' || szFileExt[2]=='G') ) 
		 return smTEXFILETYPE_JPG;

	if ( ( szFileExt[0]=='s' || szFileExt[0]=='S') && 
		 ( szFileExt[1]=='t' || szFileExt[1]=='T') &&
		 ( szFileExt[2]=='a' || szFileExt[2]=='A') ) 
		 return smTEXFILETYPE_TGA;

	return -1;
}


//######################################################################################
//작 성 자 : 오 영 석
// 설 명   : smCreateTexture() 함수에서 호출시에는 아무 문제 없지만. 만약 따로 다른 곳에서
//			 호출 했다면.  MipMapCount 값 등을 초기화 후에 호출해서 사용 해야 합니다.
int LoadDibSurfaceAlphaMipMap( LPDIRECTDRAWSURFACE4 lpDDSurface, char *Filename, char *FileAlpha, int CheckWidth, int CheckHeight )
//######################################################################################
{
	BYTE 				*lpDib;
	BYTE				*dAddr;
	int					height,width;

	BYTE 				*lpDibAlpha;
	BYTE				*dAddrAlpha;
	int					BitCnt;
	int					AlphaBitCnt=0;
	int					SwapHeight;
	int					SwapHeightAl;

	int FileTypeCl;
	int FileTypeAl;
	CJpeg	*cJpeg =0;
	CJpeg	*cJpegAl =0;
	cTGA	*cTga = 0;
	cTGA	*cTgaAl = 0;
	lpDib = 0;
	lpDibAlpha = 0;

	SwapHeight = TRUE;
	SwapHeightAl = TRUE;

	FileTypeCl = GetTextureFileType( Filename ) ;
	FileTypeAl = GetTextureFileType( FileAlpha ) ;


	switch( FileTypeCl ) {
	case smTEXFILETYPE_TGA:
		cTga = new cTGA;
		if ( cTga->LoadTga( Filename ) ) {
			dAddr = cTga->m_pData;
			width = cTga->GetWidth();
			height = cTga->GetHeight();
			BitCnt = cTga->GetBitCount();
			SwapHeightAl = cTga->SwapHeight;
//			SwapHeight = FALSE;
		}
		else
		{
			//######################################################################################
			//작 성 자 : 오 영 석
			if( cTga ) delete cTga;			
			BitCnt = 0;
			return -1;
			//######################################################################################
		}

		break;

	case smTEXFILETYPE_BMP:
		lpDib = LoadDib( Filename );
		if ( lpDib ) {
			dAddr	=GetDibBitsAddr(lpDib);
			height	=GetDibHeight(lpDib);
			width	=GetDibWidth(lpDib);
			BitCnt = GetDibBitCount(lpDib);
		}
		else
		{
			//######################################################################################
			//작 성 자 : 오 영 석
			if( lpDib ) delete lpDib;
			return -1;
			//######################################################################################
		}
		break;

	case smTEXFILETYPE_JPG:
		cJpeg = new CJpeg;
		cJpeg->LoadJPG( Filename );
		if ( cJpeg->m_pData ) {
			dAddr	=cJpeg->m_pData;
			height	=cJpeg->GetHeight();
			width	=cJpeg->GetWidth();
			BitCnt = 24;
		}
		else
		{
			//######################################################################################
			//작 성 자 : 오 영 석
			if( cJpeg ) delete cJpeg;
			return -1;
			//######################################################################################
		}
		break;
	}

	//######################################################################################
	//작 성 자 : 오 영 석
	if( width != CheckWidth || height != CheckHeight )
	{
		if( lpDib ) delete lpDib;
		if( cTga ) delete cTga;
		if( cJpeg ) delete cJpeg;
		return -1;
	}
	//######################################################################################


	//######################################################################################
	//작 성 자 : 오 영 석
	int ErrorFileTypeAl = 0;
	switch( FileTypeAl )
	{
	case smTEXFILETYPE_TGA:
		cTgaAl = new cTGA;
		if( cTgaAl->LoadTga( FileAlpha ) )
		{
			dAddrAlpha = cTgaAl->m_pData;
			AlphaBitCnt = cTgaAl->GetBitCount();
			SwapHeightAl = cTgaAl->SwapHeight;

			if( CheckWidth != cTgaAl->GetWidth() || CheckHeight != cTgaAl->GetHeight() )
				ErrorFileTypeAl = 1;
		}
		else
		{
			dAddrAlpha = NULL;
			AlphaBitCnt = 0;
		}
		break;

	case smTEXFILETYPE_BMP:
		lpDibAlpha = LoadDib( FileAlpha );
		if( lpDibAlpha )
		{
			dAddrAlpha	=GetDibBitsAddr(lpDibAlpha);
			AlphaBitCnt = GetDibBitCount(lpDibAlpha);

			if( CheckWidth != GetDibWidth(lpDibAlpha) || CheckHeight != GetDibHeight(lpDibAlpha) )
				ErrorFileTypeAl = 1;
		}
		else
		{
			dAddrAlpha = NULL;
			AlphaBitCnt = 0;
		}
		break;

	case smTEXFILETYPE_JPG:
		cJpegAl = new CJpeg;
		cJpegAl->LoadJPG( FileAlpha );
		if( cJpegAl->m_pData )
		{
			dAddrAlpha	=cJpegAl->m_pData;
			AlphaBitCnt = 24;

			if( CheckWidth != cJpegAl->GetWidth() || CheckHeight != cJpegAl->GetHeight() )
				ErrorFileTypeAl = 1;
		}
		else
		{
			dAddrAlpha = NULL;
			AlphaBitCnt = 0;
		}
		break;
	}

	if( ErrorFileTypeAl )
	{
		if( lpDib ) delete lpDib;
		if( cTga ) delete cTga;
		if( cJpeg ) delete cJpeg;
		
		if ( lpDibAlpha ) delete lpDibAlpha;
		if ( cJpegAl ) delete cJpegAl;
		if ( cTgaAl ) delete cTgaAl;

		return -2;
	}
	//######################################################################################

	int i,j,k;
	int r,g,b,a,tp;
	BYTE *Dest;
	DWORD MaskAll;

	DWORD Mask[4];
	DWORD Shift[4];
	DWORD ShiftPixel[4];
	DWORD ShiftTemp;


	Mask[0] = ddTextureFormat.dwRBitMask;
	Mask[1] = ddTextureFormat.dwGBitMask;
	Mask[2] = ddTextureFormat.dwBBitMask;
	Mask[3] = ddTextureFormat.dwRGBAlphaBitMask;

	MaskAll = Mask[0]|Mask[1]|Mask[2]|Mask[3];

	for( i=0; i<4; i++ ) {
		if ( Mask[i] ) {
			ShiftTemp = Mask[i];
			for( j=0;j<32;j++ ) {
				if ( ShiftTemp&1 ) break;
				ShiftTemp>>=1;
			}
			Shift[i] = j;
			for( j=0;j<32;j++) {
				if ( (ShiftTemp&1)==0 ) break;
				ShiftTemp>>=1;
			}
			ShiftPixel[i] = j;
		}
		else {
			Shift[i] = 32;
			ShiftPixel[i] = 32;
		}
	}

	int pBuff,pBuffAl;
	int incByte = 3;
	int incByteAl = 3;
	int pBuff_k;

	DDSURFACEDESC2 ddsd;
	ZeroMemory( &ddsd, sizeof(DDSURFACEDESC2) );

	ddsd.dwSize = sizeof(DDSURFACEDESC2);
	lpDDSurface->Lock(NULL,&ddsd,DDLOCK_WAIT,NULL);
	Dest = (BYTE *)ddsd.lpSurface;

	if ( !Dest ) goto Lop_Quit;
/*
			if ( (int)ddsd.lPitch!=0 )
				pBuff_k = 0;//(int)ddsd.lPitch*(height-1);
			else {
				if ( MaskAll<=0xFFFF ) 
					pBuff_k = 0;//width * (height-1) * 2;
				else
					pBuff_k = 0;//width * (height-1) * 4;
			}
*/

		pBuff_k = 0;
		k=0;
		for(i=0;i<height;i++) {
			switch( BitCnt ) {
			case 24:
				if ( SwapHeight ) pBuff = (height-i-1)*width*3;
				else pBuff = i*width*3;
				incByte = 3;
				break;
			case 32:
				if ( SwapHeight ) pBuff = ((height-i-1)*width)<<2;
				else pBuff = (i*width)<<2;
				incByte = 4;
				break;
			}

			switch( AlphaBitCnt ) {
			case 8:
				if ( SwapHeightAl ) pBuffAl = (height-i-1)*width;
				else pBuffAl = i*width;
				incByteAl = 1;
				break;

			case 24:
				if ( SwapHeightAl ) pBuffAl = (height-i-1)*width*3;
				else pBuffAl = i*width*3;
				incByteAl = 3;
				break;

			case 32:
				if ( SwapHeightAl ) pBuffAl = ((height-i-1)*width)<<2;
				else pBuffAl = (i*width)<<2;
				incByteAl = 4;
				break;
			}

			//if ( (int)ddsd.lPitch!=0 )  k = ((int)ddsd.lPitch*i);			// 서페이스 피치 조절

			k = pBuff_k;

			for(j=0;j<width;j++) {

				b = dAddr[ pBuff ]; 
				g = dAddr[ pBuff +1]; 
				r = dAddr[ pBuff +2];

				if ( BitCnt==32 ) {
					a = dAddr[ pBuff +3];
				}
				else {
					a = 255;

					switch( AlphaBitCnt ) {
					case 8: 
						a = dAddrAlpha[ pBuffAl ];//(height-i-1)*width + j ];
						break;
					case 24:
					case 32:
						a = dAddrAlpha[ pBuffAl ] + dAddrAlpha[ pBuffAl+1 ] + dAddrAlpha[ pBuffAl+2 ];
						a /=3;
						break;
					}
					pBuffAl+=incByteAl;
				}
				pBuff +=incByte;

				r = (( r>>(8-ShiftPixel[0] ) )<< Shift[0] ) & Mask[0];
				g = (( g>>(8-ShiftPixel[1] ) )<< Shift[1] ) & Mask[1];
				b = (( b>>(8-ShiftPixel[2] ) )<< Shift[2] ) & Mask[2];
				a = (( a>>(8-ShiftPixel[3] ) )<< Shift[3] ) & Mask[3];

				tp = r|g|b|a;											// 하이컬러로 변환

//				Dest[ k++ ] = tp&0xFF;									// 저장은 하이컬러로
//				Dest[ k++ ] = tp>>8;

				if ( MaskAll<=0xFFFF ) {
					*((WORD *)&Dest[ k ]) = (WORD)tp;
					k+=2;
				}
				else {
					*((DWORD *)&Dest[ k ]) = tp;
					k+=4;
				}
			}

			pBuff_k += (int)ddsd.lPitch;

		}
		lpDDSurface->Unlock(NULL);
		//######################################################################################

Lop_Quit:

	if ( cJpeg ) delete cJpeg;
	if ( lpDib ) delete lpDib;
	if ( cTga ) delete cTga;

	if ( cJpegAl ) delete cJpegAl;
	if ( lpDibAlpha ) delete lpDibAlpha;
	if ( cTgaAl ) delete cTgaAl;

	return TRUE;
}


/////////[ BMP파일을 텍스쳐 서페이스로 만든다 ( 알파채널 사용 ) ] ///////////
LPDIRECTDRAWSURFACE4 LoadDibSurfaceAlpha( char *Filename , char *FileAlpha , int SysMem )
{
	BYTE 				*lpDib;
	BYTE				*dAddr;
	int					height,width;

	BYTE 				*lpDibAlpha;
	BYTE				*dAddrAlpha;
	int					BitCnt;
	int					AlphaBitCnt=0;
	int					SwapHeight;
	int					SwapHeightAl;

	int FileTypeCl;
	int FileTypeAl;
	CJpeg	*cJpeg =0;
	CJpeg	*cJpegAl =0;
	cTGA	*cTga = 0;
	cTGA	*cTgaAl = 0;
	lpDib = 0;
	lpDibAlpha = 0;

	SwapHeight = TRUE;
	SwapHeightAl = TRUE;

	FileTypeCl = GetTextureFileType( Filename ) ;
	FileTypeAl = GetTextureFileType( FileAlpha ) ;


	switch( FileTypeCl ) {
	case smTEXFILETYPE_TGA:
		cTga = new cTGA;
		if ( cTga->LoadTga( Filename ) ) {
			dAddr = cTga->m_pData;
			width = cTga->GetWidth();
			height = cTga->GetHeight();
			BitCnt = cTga->GetBitCount();
			SwapHeightAl = cTga->SwapHeight;
//			SwapHeight = FALSE;
		}
		else
		{
			//######################################################################################
			//작 성 자 : 오 영 석
			if( cTga ) delete cTga;
			//######################################################################################
			BitCnt = 0;
			return NULL;
		}

		break;

	case smTEXFILETYPE_BMP:
		lpDib = LoadDib( Filename );
		if ( lpDib ) {
			dAddr	=GetDibBitsAddr(lpDib);
			height	=GetDibHeight(lpDib);
			width	=GetDibWidth(lpDib);
			BitCnt = GetDibBitCount(lpDib);
		}
		else
		{
			//######################################################################################
			//작 성 자 : 오 영 석
			if( lpDib ) delete lpDib;
			//######################################################################################
			return NULL;
		}
		break;

	case smTEXFILETYPE_JPG:
		cJpeg = new CJpeg;
		cJpeg->LoadJPG( Filename );
		if ( cJpeg->m_pData ) {
			dAddr	=cJpeg->m_pData;
			height	=cJpeg->GetHeight();
			width	=cJpeg->GetWidth();
			BitCnt = 24;
		}
		else
		{
			//######################################################################################
			//작 성 자 : 오 영 석
			if( cJpeg ) delete cJpeg;
			//######################################################################################
			return NULL;
		}
		break;
	}

	switch( FileTypeAl ) {
	case smTEXFILETYPE_TGA:
		cTgaAl = new cTGA;
		if ( cTgaAl->LoadTga( FileAlpha ) ) {
			dAddrAlpha = cTgaAl->m_pData;
			AlphaBitCnt = cTgaAl->GetBitCount();
			SwapHeightAl = cTgaAl->SwapHeight;
		}
		else  {
			dAddrAlpha = NULL;
			AlphaBitCnt = 0;
		}

		break;

	case smTEXFILETYPE_BMP:
		lpDibAlpha = LoadDib( FileAlpha );
		if ( lpDibAlpha ) {
			dAddrAlpha	=GetDibBitsAddr(lpDibAlpha);
			AlphaBitCnt = GetDibBitCount(lpDibAlpha);
		}
		else {
			dAddrAlpha = NULL;
			AlphaBitCnt = 0;
		}

		break;

	case smTEXFILETYPE_JPG:
		cJpegAl = new CJpeg;
		cJpegAl->LoadJPG( FileAlpha );
		if ( cJpegAl->m_pData ) {
			dAddrAlpha	=cJpegAl->m_pData;
			AlphaBitCnt = 24;
		}
		else {
			dAddrAlpha = NULL;
			AlphaBitCnt = 0;
		}
		break;
	}

	// Now create surface
	LPDIRECTDRAWSURFACE4 lpDDSurface;
	DDSURFACEDESC2		ddsd;

	ZeroMemory( &ddsd, sizeof(DDSURFACEDESC2) );

	ddsd.dwSize		= sizeof(DDSURFACEDESC2);
	ddsd.dwFlags	= DDSD_CAPS |
					  DDSD_HEIGHT |
					  DDSD_WIDTH  |
					  DDSD_PIXELFORMAT;
	ddsd.dwWidth	= width;
    ddsd.dwHeight	= height;
/*
	TextureFormat.BPP_DW = 16;
	TextureFormat.MinAlpha = 8;				//알파 최소치 ( 8단계 이상의 앞파채널 )
	TextureFormat.bUseFourCC = FALSE;
	TextureFormat.bUsePalette = FALSE;
	TextureFormat.bFoundGoodFormat = FALSE;
	TextureFormat.pddpf = &ddsd.ddpfPixelFormat;

	lpD3DDevice->EnumTextureFormats( TextureSearchCallback , &TextureFormat );
    memcpy( &ddTextureFormat, &ddsd.ddpfPixelFormat, sizeof(DDPIXELFORMAT) );
*/
    memcpy( &ddsd.ddpfPixelFormat , &ddTextureFormat, sizeof(DDPIXELFORMAT) );

	//######################################################################################
	//작 성 자 : 오 영 석
	if ( lpD3DDeviceDesc->bIsHardware && SysMem==FALSE )
	{
		//ddsd.ddsCaps.dwCaps = DDSCAPS_VIDEOMEMORY | DDSCAPS_TEXTURE;
		ddsd.ddsCaps.dwCaps  = DDSCAPS_TEXTURE;
		ddsd.ddsCaps.dwCaps2 = DDSCAPS2_TEXTUREMANAGE;
	}
    else
        ddsd.ddsCaps.dwCaps = DDSCAPS_SYSTEMMEMORY| DDSCAPS_TEXTURE;
	//######################################################################################

    hresult = lpDD->CreateSurface( &ddsd, &lpDDSurface, NULL );

	//
    if ( hresult != DD_OK ) { 
		if ( cJpeg ) delete cJpeg;
		if ( lpDib ) delete lpDib;
		if ( cTga ) delete cTga;
		if ( cJpegAl ) delete cJpegAl;
		if( lpDibAlpha ) delete lpDibAlpha;
		if ( cTgaAl ) delete cTgaAl;
		return NULL; 
	}


	int i,j,k;
	int r,g,b,a,tp;
	BYTE *Dest;
	DWORD MaskAll;

	DWORD Mask[4];
	DWORD Shift[4];
	DWORD ShiftPixel[4];
	DWORD ShiftTemp;


	Mask[0] = ddTextureFormat.dwRBitMask;
	Mask[1] = ddTextureFormat.dwGBitMask;
	Mask[2] = ddTextureFormat.dwBBitMask;
	Mask[3] = ddTextureFormat.dwRGBAlphaBitMask;

	MaskAll = Mask[0]|Mask[1]|Mask[2]|Mask[3];

	for( i=0; i<4; i++ ) {
		if ( Mask[i] ) {
			ShiftTemp = Mask[i];
			for( j=0;j<32;j++ ) {
				if ( ShiftTemp&1 ) break;
				ShiftTemp>>=1;
			}
			Shift[i] = j;
			for( j=0;j<32;j++) {
				if ( (ShiftTemp&1)==0 ) break;
				ShiftTemp>>=1;
			}
			ShiftPixel[i] = j;
		}
		else {
			Shift[i] = 32;
			ShiftPixel[i] = 32;
		}
	}

/*
	for( i=0; i<4; i++ ) {
		if ( Mask[i] ) {
			ShiftTemp = Mask[i];
			for( j=0;j<32;j++ ) {
				if ( ShiftTemp&1 ) break;
				ShiftTemp>>=1;
			}
			Shift[i] = j;
			for( j=0;j<32;j++) {
				if ( (ShiftTemp&1)==0 ) break;
				ShiftTemp>>=1;
			}
			ShiftPixel[i] = j;
		}
		else {
			Shift[i] = 16;
			ShiftPixel[i] = 16;
		}
	}
*/

	int pBuff,pBuffAl;
	int incByte = 3;
	int incByteAl = 3;
	int pBuff_k;


		ddsd.dwSize		= sizeof(DDSURFACEDESC2);
		lpDDSurface->Lock(NULL,&ddsd,DDLOCK_WAIT,NULL);
		Dest = (BYTE *)ddsd.lpSurface;

	if ( !Dest ) goto Lop_Quit;
/*
			if ( (int)ddsd.lPitch!=0 )
				pBuff_k = 0;//(int)ddsd.lPitch*(height-1);
			else {
				if ( MaskAll<=0xFFFF ) 
					pBuff_k = 0;//width * (height-1) * 2;
				else
					pBuff_k = 0;//width * (height-1) * 4;
			}
*/

		pBuff_k = 0;
		k=0;
		for(i=0;i<height;i++) {
			switch( BitCnt ) {
			case 24:
				if ( SwapHeight ) pBuff = (height-i-1)*width*3;
				else pBuff = i*width*3;
				incByte = 3;
				break;
			case 32:
				if ( SwapHeight ) pBuff = ((height-i-1)*width)<<2;
				else pBuff = (i*width)<<2;
				incByte = 4;
				break;
			}

			switch( AlphaBitCnt ) {
			case 8:
				if ( SwapHeightAl ) pBuffAl = (height-i-1)*width;
				else pBuffAl = i*width;
				incByteAl = 1;
				break;

			case 24:
				if ( SwapHeightAl ) pBuffAl = (height-i-1)*width*3;
				else pBuffAl = i*width*3;
				incByteAl = 3;
				break;

			case 32:
				if ( SwapHeightAl ) pBuffAl = ((height-i-1)*width)<<2;
				else pBuffAl = (i*width)<<2;
				incByteAl = 4;
				break;
			}

			//if ( (int)ddsd.lPitch!=0 )  k = ((int)ddsd.lPitch*i);			// 서페이스 피치 조절

			k = pBuff_k;

			for(j=0;j<width;j++) {

				b = dAddr[ pBuff ]; 
				g = dAddr[ pBuff +1]; 
				r = dAddr[ pBuff +2];

				if ( BitCnt==32 ) {
					a = dAddr[ pBuff +3];
				}
				else {
					a = 255;

					switch( AlphaBitCnt ) {
					case 8: 
						a = dAddrAlpha[ pBuffAl ];//(height-i-1)*width + j ];
						break;
					case 24:
					case 32:
						a = dAddrAlpha[ pBuffAl ] + dAddrAlpha[ pBuffAl+1 ] + dAddrAlpha[ pBuffAl+2 ];
						a /=3;
						break;
					}
					pBuffAl+=incByteAl;
				}
				pBuff +=incByte;

				r = (( r>>(8-ShiftPixel[0] ) )<< Shift[0] ) & Mask[0];
				g = (( g>>(8-ShiftPixel[1] ) )<< Shift[1] ) & Mask[1];
				b = (( b>>(8-ShiftPixel[2] ) )<< Shift[2] ) & Mask[2];
				a = (( a>>(8-ShiftPixel[3] ) )<< Shift[3] ) & Mask[3];

				tp = r|g|b|a;											// 하이컬러로 변환

//				Dest[ k++ ] = tp&0xFF;									// 저장은 하이컬러로
//				Dest[ k++ ] = tp>>8;

				if ( MaskAll<=0xFFFF ) {
					*((WORD *)&Dest[ k ]) = (WORD)tp;
					k+=2;
				}
				else {
					*((DWORD *)&Dest[ k ]) = tp;
					k+=4;
				}
			}

			pBuff_k += (int)ddsd.lPitch;

		}
		lpDDSurface->Unlock(NULL);

Lop_Quit:

	if ( cJpeg ) delete cJpeg;
	if ( lpDib ) delete lpDib;
	if ( cTga ) delete cTga;

	if ( cJpegAl ) delete cJpegAl;
	if ( lpDibAlpha ) delete lpDibAlpha;
	if ( cTgaAl ) delete cTgaAl;

	return lpDDSurface;

}


//////////////// [ BMP파일을 텍스쳐 서페이스로 만든다 ] ///////////////
LPDIRECTDRAWSURFACE4 LoadDibSurface( char *Filename , int SysMem )
{

	BYTE 				*lpDib;
	BYTE				*dAddr;
	int					height,width;
	
	lpDib = LoadDib( Filename );
	if ( lpDib==NULL ) return NULL;

	dAddr	=GetDibBitsAddr(lpDib);
	height	=GetDibHeight(lpDib);
	width	=GetDibWidth(lpDib);

	// Now create surface
	LPDIRECTDRAWSURFACE4 lpDDSurface;
	DDSURFACEDESC2		ddsd;

	ZeroMemory( &ddsd, sizeof(DDSURFACEDESC2) );

	ddsd.dwSize		= sizeof(DDSURFACEDESC2);
	ddsd.dwFlags	= DDSD_CAPS |
					  DDSD_HEIGHT |
					  DDSD_WIDTH  ;
	ddsd.dwWidth	= width;
    ddsd.dwHeight	= height;

	//######################################################################################
	//작 성 자 : 오 영 석
	if ( lpD3DDeviceDesc->bIsHardware && SysMem==FALSE )
	{
		//ddsd.ddsCaps.dwCaps = DDSCAPS_VIDEOMEMORY | DDSCAPS_TEXTURE;
		ddsd.ddsCaps.dwCaps  = DDSCAPS_TEXTURE;
		ddsd.ddsCaps.dwCaps2 = DDSCAPS2_TEXTUREMANAGE;
	}
    else
        ddsd.ddsCaps.dwCaps = DDSCAPS_SYSTEMMEMORY| DDSCAPS_TEXTURE;
	//######################################################################################

    hresult = lpDD->CreateSurface( &ddsd, &lpDDSurface, NULL );
    if ( hresult != DD_OK ) { delete lpDib; return NULL; }

/*
	HDC	hdc;
	lpDDSurface->GetDC( &hdc );
	SetDIBitsToDevice( hdc , 0,0,width,height,0,0, 0,height, (LPSTR) dAddr,
		(LPBITMAPINFO )lpDib , DIB_RGB_COLORS );
	lpDDSurface->ReleaseDC( hdc );
	delete lpDib;
*/
	DrawSurfaceFromDib( lpDDSurface , lpDib );
	delete lpDib;

	// RGB( 0,0,0 ) ColorKey;
	DDCOLORKEY ColKey;
	ColKey.dwColorSpaceLowValue=RGB(0,0,0); 
    ColKey.dwColorSpaceHighValue=RGB(255,255,255);  
	lpDDSurface->SetColorKey(DDCKEY_SRCBLT  , &ColKey );

	return lpDDSurface;
}

// Dib를 서페이스에 그린다
int DrawSurfaceFromDib( LPDIRECTDRAWSURFACE4 lpDDSurface , BYTE *lpDib )
{
		int i,j,k;
		DWORD r,g,b,a,tp;
		BYTE *Dest;

		DWORD MaskAll;

		DWORD Mask[4];
		DWORD Shift[4];
		DWORD ShiftPixel[4];
		DWORD ShiftTemp;

		DDSURFACEDESC2		ddsd;
		DDPIXELFORMAT		ddPixelFormat;

		BYTE				*dAddr;
		int					height,width;


		dAddr	=GetDibBitsAddr(lpDib);
		height	=GetDibHeight(lpDib);
		width	=GetDibWidth(lpDib);

		//픽셀 포맷 구하기
		ddPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
		lpDDSurface->GetPixelFormat( &ddPixelFormat );

		Mask[0] = ddPixelFormat.dwRBitMask;
		Mask[1] = ddPixelFormat.dwGBitMask;
		Mask[2] = ddPixelFormat.dwBBitMask;
		Mask[3] = ddPixelFormat.dwRGBAlphaBitMask;

		MaskAll = Mask[0]|Mask[1]|Mask[2]|Mask[3];

		for( i=0; i<4; i++ ) {
			if ( Mask[i] ) {
				ShiftTemp = Mask[i];
				for( j=0;j<32;j++ ) {
					if ( ShiftTemp&1 ) break;
					ShiftTemp>>=1;
				}
				Shift[i] = j;
				for( j=0;j<32;j++) {
					if ( (ShiftTemp&1)==0 ) break;
					ShiftTemp>>=1;
				}
				ShiftPixel[i] = j;
			}
			else {
				Shift[i] = 32;
				ShiftPixel[i] = 32;
			}
		}

		int pBuff;
		int pBuff_w;
		int pBuff_k;

		if (GetDibBitCount(lpDib)==24) {
			ddsd.dwSize		= sizeof(DDSURFACEDESC2);
			lpDDSurface->Lock(NULL,&ddsd,DDLOCK_WAIT,NULL);
			Dest = (BYTE *)ddsd.lpSurface;
			if ( !Dest ) {
				lpDDSurface->Unlock(NULL);
				return FALSE;
			}

			pBuff_w = 0;
			if ( (int)ddsd.lPitch!=0 )
				pBuff_k = (int)ddsd.lPitch*(height-1);
			else {
				if ( MaskAll<=0xFFFF ) 
					pBuff_k = width * (height-1) * 2;
				else
					pBuff_k = width * (height-1) * 4;
			}

			for(i=0;i<height;i++) {
				//if ( (int)ddsd.lPitch!=0 )  k = ((int)ddsd.lPitch*i);			// 서페이스 피치 조절
				k = pBuff_k;
				for(j=0;j<width;j++) {
				
					//pBuff = (height-i-1)*width*3 + (j*3);
					pBuff = pBuff_w + (j*3);
					b = dAddr[ pBuff ]; 
					g = dAddr[ pBuff +1]; 
					r = dAddr[ pBuff +2];
					a = 255;

					if ( (r+g+b)>0 && r<9 && g<9 && b<9 ) {
						r = g = b = 9;
					}

					r = (( r>>(8-ShiftPixel[0] ) )<< Shift[0] ) & Mask[0];
					g = (( g>>(8-ShiftPixel[1] ) )<< Shift[1] ) & Mask[1];
					b = (( b>>(8-ShiftPixel[2] ) )<< Shift[2] ) & Mask[2];
					a = (( a>>(8-ShiftPixel[3] ) )<< Shift[3] ) & Mask[3];

					tp = r|g|b|a;											// 하이컬러로 변환

					if ( MaskAll<=0xFFFF ) {
						*((WORD *)&Dest[ k ]) = (WORD)tp;
						k+=2;
					}
					else {
						*((DWORD *)&Dest[ k ]) = tp;
						k+=4;
					}

				}
				pBuff_w =  ((pBuff_w+width*3)+3)&0xFFFFFFFC;
				pBuff_k -= (int)ddsd.lPitch;
			}
			lpDDSurface->Unlock(NULL);
		}

/*
		//이거 사용하면 망함 ( NDIVIA 꺼 윈 2000 에서 안됨 )
		HDC	hdc;
		lpDDSurface->GetDC( &hdc );
		SetDIBitsToDevice( hdc , 0,0,width,height,0,0, 0,height, (LPSTR) dAddr,
			(LPBITMAPINFO )lpDib , DIB_RGB_COLORS );
		lpDDSurface->ReleaseDC( hdc );
*/


	return TRUE;
}



// Jpeg를 서페이스에 그린다
int DrawSurfaceFromJpeg( LPDIRECTDRAWSURFACE4 lpDDSurface , CJpeg *cJpeg )
{
		//######################################################################################
		//작 성 자 : 오 영 석
		int i,j,k;
		DWORD r,g,b,a,tp;		// 데이타 형 바꿈.
		BYTE *Dest;

		DWORD MaskAll;
		//######################################################################################

		DWORD Mask[4];
		DWORD Shift[4];
		DWORD ShiftPixel[4];
		DWORD ShiftTemp;

		DDSURFACEDESC2		ddsd;
		DDPIXELFORMAT		ddPixelFormat;

		BYTE				*dAddr;
		int					height,width;

		//픽셀 포맷 구하기
		ddPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
		lpDDSurface->GetPixelFormat( &ddPixelFormat );

		dAddr	=cJpeg->m_pData;
		height	=cJpeg->GetHeight();
		width	=cJpeg->GetWidth();

		Mask[0] = ddPixelFormat.dwRBitMask;
		Mask[1] = ddPixelFormat.dwGBitMask;
		Mask[2] = ddPixelFormat.dwBBitMask;
		Mask[3] = ddPixelFormat.dwRGBAlphaBitMask;

		//######################################################################################
		//작 성 자 : 오 영 석
		MaskAll = Mask[0]|Mask[1]|Mask[2]|Mask[3];
		//######################################################################################

		for( i=0; i<4; i++ ) {
			if ( Mask[i] ) {
				ShiftTemp = Mask[i];
				for( j=0;j<16;j++ ) {
					if ( ShiftTemp&1 ) break;
					ShiftTemp>>=1;
				}
				Shift[i] = j;
				for( j=0;j<16;j++) {
					if ( (ShiftTemp&1)==0 ) break;
					ShiftTemp>>=1;
				}
				ShiftPixel[i] = j;
			}
			else {
				Shift[i] = 16;
				ShiftPixel[i] = 16;
			}
		}

		int pBuff;

			ddsd.dwSize		= sizeof(DDSURFACEDESC2);
			lpDDSurface->Lock(NULL,&ddsd,DDLOCK_WAIT,NULL);
			Dest = (BYTE *)ddsd.lpSurface;

			k=0;
			for(i=0;i<height;i++) {
				if ( (int)ddsd.lPitch!=0 )  k = ((int)ddsd.lPitch*i);			// 서페이스 피치 조절
				for(j=0;j<width;j++) {
				
					pBuff = (height-i-1)*width*3 + (j*3);
					b = dAddr[ pBuff ]; 
					g = dAddr[ pBuff +1]; 
					r = dAddr[ pBuff +2];
					a = 255;
					
					r = (( r>>(8-ShiftPixel[0] ) )<< Shift[0] ) & Mask[0];
					g = (( g>>(8-ShiftPixel[1] ) )<< Shift[1] ) & Mask[1];
					b = (( b>>(8-ShiftPixel[2] ) )<< Shift[2] ) & Mask[2];
					a = (( a>>(8-ShiftPixel[3] ) )<< Shift[3] ) & Mask[3];

					tp = r|g|b|a;											// 하이컬러로 변환

					//######################################################################################
					//작 성 자 : 오 영 석
					if( MaskAll <= 0xFFFF )
					{
						*((WORD *)&Dest[ k ]) = (WORD)tp;
						k+=2;
					}
					else
					{
						*((DWORD *)&Dest[ k ]) = tp;
						k+=4;
					}
					//######################################################################################
				}
			}
			lpDDSurface->Unlock(NULL);


	return TRUE;
}


// TGA를 서페이스에 그린다 ( 24비트 만 해당 / 32비트는 알파채널과 같이 읽으므로 여기서 그리지 않는다 )
int DrawSurfaceFromTga( LPDIRECTDRAWSURFACE4 lpDDSurface , cTGA *cTga )
{
		//######################################################################################
		//작 성 자 : 오 영 석
		int i,j,k;
		int r,g,b,a,tp;
		BYTE *Dest;
		int SwapHeight;

		DWORD MaskAll;
		//######################################################################################

		DWORD Mask[4];
		DWORD Shift[4];
		DWORD ShiftPixel[4];
		DWORD ShiftTemp;

		DDSURFACEDESC2		ddsd;
		DDPIXELFORMAT		ddPixelFormat;

		BYTE				*dAddr;
		int					height,width;

		dAddr	=cTga->m_pData;
		height	=cTga->GetHeight();
		width	=cTga->GetWidth();
		SwapHeight = cTga->SwapHeight;

		//픽셀 포맷 구하기
		ddPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
		lpDDSurface->GetPixelFormat( &ddPixelFormat );


		Mask[0] = ddPixelFormat.dwRBitMask;
		Mask[1] = ddPixelFormat.dwGBitMask;
		Mask[2] = ddPixelFormat.dwBBitMask;
		Mask[3] = ddPixelFormat.dwRGBAlphaBitMask;

		//######################################################################################
		//작 성 자 : 오 영 석
		MaskAll = Mask[0]|Mask[1]|Mask[2]|Mask[3];
		//######################################################################################

		for( i=0; i<4; i++ ) {
			if ( Mask[i] ) {
				ShiftTemp = Mask[i];
				for( j=0;j<16;j++ ) {
					if ( ShiftTemp&1 ) break;
					ShiftTemp>>=1;
				}
				Shift[i] = j;
				for( j=0;j<16;j++) {
					if ( (ShiftTemp&1)==0 ) break;
					ShiftTemp>>=1;
				}
				ShiftPixel[i] = j;
			}
			else {
				Shift[i] = 16;
				ShiftPixel[i] = 16;
			}
		}

		int pBuff;

			ddsd.dwSize		= sizeof(DDSURFACEDESC2);
			lpDDSurface->Lock(NULL,&ddsd,DDLOCK_WAIT,NULL);
			Dest = (BYTE *)ddsd.lpSurface;

			k=0;
			for(i=0;i<height;i++) {
				if ( (int)ddsd.lPitch!=0 )  k = ((int)ddsd.lPitch*i);			// 서페이스 피치 조절
				for(j=0;j<width;j++) {
	
					if ( SwapHeight ) pBuff = (height-i-1)*width+j;
					else pBuff = i*width+j;
				
					pBuff *= 3;
//					pBuff = i*width*3 + j*3;
					b = dAddr[ pBuff ]; 
					g = dAddr[ pBuff +1]; 
					r = dAddr[ pBuff +2];
					a = 255;
					
					r = (( r>>(8-ShiftPixel[0] ) )<< Shift[0] ) & Mask[0];
					g = (( g>>(8-ShiftPixel[1] ) )<< Shift[1] ) & Mask[1];
					b = (( b>>(8-ShiftPixel[2] ) )<< Shift[2] ) & Mask[2];
					a = (( a>>(8-ShiftPixel[3] ) )<< Shift[3] ) & Mask[3];

					tp = r|g|b|a;											// 하이컬러로 변환

					//######################################################################################
					//작 성 자 : 오 영 석
					if( MaskAll <= 0xFFFF )
					{
						*((WORD *)&Dest[ k ]) = (WORD)tp;
						k+=2;
					}
					else
					{
						*((DWORD *)&Dest[ k ]) = tp;
						k+=4;
					}
					//######################################################################################
				}
			}
			lpDDSurface->Unlock(NULL);


	return TRUE;
}



//######################################################################################
//작 성 자 : 오 영 석
int MipMapCreateTextureCheck( int width, int height, int MipMapCount )
{
	if( width > 256 || height > 256 )
		return 0;

	switch( MipMapCount )
	{
	case 5:
		if( width == 256 && height == 256 )
			return 1;
		break;

	case 4:
		if( width >= 128 && height >= 128 )
			return 1;
		break;

	case 3:
		if( width >= 64 && height >= 64 )
			return 1;
		break;

	case 2:
		if( width >= 32 && height >= 32 )
			return 1;
		break;

	case 1:
		return 0;

	default:
		return 0;
	}

	return 0;
}
//######################################################################################





//######################################################################################
//작 성 자 : 오 영 석
//////////////// [ BMP파일을 텍스쳐 서페이스로 만든다 (자동 스왑) ] ///////////////
int smCreateTexture( smTEXTUREHANDLE *lpTexHandle, int MipMap )
{
	MipMapCount = 0;
	::ZeroMemory( MipMapFileBuff, sizeof(MipMapFileBuff) );
	MipMapCountA = 0;
	::ZeroMemory( MipMapFileABuff, sizeof(MipMapFileABuff) );
	::ZeroMemory( FileTypeBuff, sizeof(FileTypeBuff) );

	char szMipMapFileError[256] = {"MipMap File Error : "};
	//######################################################################################

	BYTE 				*lpDib;
	BYTE				*dAddr;
	int					height,width;
	int					height2,width2;
	int					TexSwap;
	int					TexHalf;


	// Now create surface
    LPDIRECT3DTEXTURE2 lpD3DTexture;
	LPDIRECTDRAWSURFACE4 lpDDSurface;
	LPDIRECTDRAWSURFACE4 lpDDSurface2;
	DDSURFACEDESC2		ddsd;
	//DDSURFACEDESC2		ddsd2;

	char *Filename;
	CJpeg	*cJpeg = 0;
	cTGA	*cTga = 0;
	int		FileType;
	int		TgaBitCount =0;

	lpDib = 0;
	TexHalf = 0;

	Filename = lpTexHandle->Name;
	FileType = GetTextureFileType( Filename ) ;

	if ( FileType<0 ) return -1;

	switch( FileType ) {

	case smTEXFILETYPE_TGA:
		cTga = new cTGA;

		if ( cTga->LoadTga( Filename ) ) {
			dAddr = cTga->m_pData;
			width = cTga->GetWidth();
			height = cTga->GetHeight();
			TgaBitCount = cTga->GetBitCount();
		}
		else {
			delete cTga;
			return -1;
		}
		
		//######################################################################################
		//작 성 자 : 오 영 석
		::strcpy( FileTypeBuff, ".tga" );
		//######################################################################################
		break;

	case smTEXFILETYPE_JPG:
		cJpeg = new CJpeg;
		cJpeg->LoadJPG( Filename );
		//######################################################################################
		//작 성 자 : 오 영 석
		if( ! cJpeg->m_pData )
		{
			delete cJpeg;
			return -1;
		}
		//######################################################################################

		dAddr = cJpeg->m_pData;
		width = cJpeg->GetWidth();
		height = cJpeg->GetHeight();

		//######################################################################################
		//작 성 자 : 오 영 석
		::strcpy( FileTypeBuff, ".jpg" );
		//######################################################################################
		break;

	case smTEXFILETYPE_BMP:
		lpDib = LoadDib( Filename );
		if ( lpDib==NULL ) return -1;

		dAddr	=GetDibBitsAddr(lpDib);
		height	=GetDibHeight(lpDib);
		width	=GetDibWidth(lpDib);

		//######################################################################################
		//작 성 자 : 오 영 석
		::strcpy( FileTypeBuff, ".bmp" );
		//######################################################################################
		break;

	}

	TexSwap=FALSE;
	//######################################################################################
	// 작 성 자 : 오 영 석
	// 텍스쳐 스왑 기능 더이상 사용치 않음. 현재. 텍스쳐 퀄리티 1번 일때에. 스왑사용치 않고.
	// 단지 싸이즈가 128 이거나 그 이상은 싸이즈 줄이는 것만을 사용키로 했어요.
	
	//텍스쳐 스왑 기능 허가된 텍스쳐
	//if ( lpTexHandle->TexSwapMode==TRUE ) {
		//텍스쳐의 크기가 클경우 SWAP모드
	//	if ( width>=128 || height>=128 ) TexSwap = TRUE;
	//}
	//######################################################################################

	lpTexHandle->Width		= width;
	lpTexHandle->Height		= height;

	//######################################################################################
	//작 성 자 : 오 영 석
	if( MipMap && MipMapModeCreate && width < 512 && height < 512 )
	{
		if( MipMapModeCreate == NOSQUARE_MODE || width == height )
		{
			int i, len = ::lstrlen( Filename );
			for( i=len-1; i >= 0; i-- )
			{
				if( Filename[i] == '.' )
				{
					::CopyMemory( MipMapFileBuff[0], Filename, i );
					break;
				}
			}

			// 프테 에서는밉맵 최대 3단계 까지만 사용 하기로 했음.
			for( i=1; i < MIPMAP_FILELIST; i++ )
			{
				::strcpy( MipMapFileBuff[i], MipMapFileBuff[0] );
				::strcat( MipMapFileBuff[i], MipMapFileListBuff[i] );
				::strcat( MipMapFileBuff[i], FileTypeBuff );

				if( PathFileExists( MipMapFileBuff[i] ) )
					MipMapCount++;
				else
					break;
			}

			::strcat( MipMapFileBuff[0], FileTypeBuff );
		}
	}
	//######################################################################################

	if ( !lpTexHandle->NameA[0] && TgaBitCount<32 )
	{
		//서페이스 생성
		ZeroMemory( &ddsd, sizeof(DDSURFACEDESC2) );

		//######################################################################################
		//작 성 자 : 오 영 석
		if( MipMapCount )
		{
			MipMapCount++;

			if( MipMapCreateTextureCheck(width, height, MipMapCount) <= 0 )
			{
				if ( cJpeg ) delete cJpeg;
				if ( cTga ) delete cTga;
				if ( lpDib ) delete lpDib;

				::strcat( szMipMapFileError, Filename );
				MESSAGE( szMipMapFileError );
				return NULL;
			}

			ddsd.dwSize          = sizeof(DDSURFACEDESC2);
			ddsd.dwFlags         = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT | DDSD_PIXELFORMAT | DDSD_MIPMAPCOUNT;
			ddsd.dwWidth         = width;
			ddsd.dwHeight        = height;
			ddsd.dwMipMapCount   = MipMapCount;
			ddsd.ddsCaps.dwCaps  = DDSCAPS_MIPMAP | DDSCAPS_COMPLEX | DDSCAPS_TEXTURE;
			ddsd.ddsCaps.dwCaps2 = DDSCAPS2_TEXTUREMANAGE;
		}
		else
		{
			ddsd.dwSize	  = sizeof(DDSURFACEDESC2);
			ddsd.dwFlags  = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT;
			ddsd.dwWidth  = width;
			ddsd.dwHeight = height;

			if( lpD3DDeviceDesc->bIsHardware && TexSwap==FALSE )
			{
				//ddsd.ddsCaps.dwCaps = DDSCAPS_VIDEOMEMORY | DDSCAPS_TEXTURE;
				ddsd.ddsCaps.dwCaps  = DDSCAPS_TEXTURE;
				ddsd.ddsCaps.dwCaps2 = DDSCAPS2_TEXTUREMANAGE;
			}
			else
				ddsd.ddsCaps.dwCaps = DDSCAPS_SYSTEMMEMORY| DDSCAPS_TEXTURE;
		}
		//######################################################################################

		memcpy( &ddsd.ddpfPixelFormat , &ddDefaultTextureFormat, sizeof(DDPIXELFORMAT) );

	    hresult = lpDD->CreateSurface( &ddsd, &lpDDSurface, NULL );
		width2  = width;
		height2 = height;

		// 부두에서 256x256 초과 텍스쳐 처리 
		// ( 일단 메모리에 올린후 크기를 줄여서 비디오로 올림 )
		if ( hresult != DD_OK )
		{
			if( TexSwap==FALSE && (width>256 || height>256) )
			{
				//######################################################################################
				//작 성 자 : 오 영 석
				if( lpD3DDeviceDesc->bIsHardware )
				{
					ddsd.ddsCaps.dwCaps = DDSCAPS_SYSTEMMEMORY| DDSCAPS_TEXTURE;

					hresult = lpDD->CreateSurface( &ddsd, &lpDDSurface, NULL );
					if ( hresult==DD_OK ) TexHalf = TRUE;
				}
				//######################################################################################
			}

			if( hresult != DD_OK )
			{
				if ( cJpeg ) delete cJpeg;
				if ( cTga ) delete cTga;
				if ( lpDib ) delete lpDib;
				return NULL;
			}
		}

		//######################################################################################
		//작 성 자 : 오 영 석
		if( lpDib )
		{
			DrawSurfaceFromDib( lpDDSurface, lpDib );
			delete lpDib;
		}

		if( cJpeg )
		{
            DrawSurfaceFromJpeg( lpDDSurface, cJpeg );
            delete cJpeg;
		}

		if( cTga )
		{
			DrawSurfaceFromTga( lpDDSurface, cTga );
			delete cTga;
		}

		DDSCAPS2 ddsCaps = {0,};
		ddsCaps.dwCaps   = DDSCAPS_TEXTURE | DDSCAPS_MIPMAP;

		lpDDSurface2 = lpDDSurface;
		for( int i=1; i < MipMapCount; i++ )
		{
			LPDIRECTDRAWSURFACE4 lpDDNextLevel = NULL;
			if( FAILED( lpDDSurface2->GetAttachedSurface(&ddsCaps, &lpDDNextLevel) ) )
			{
				lpDDSurface->Release();
				return NULL;
			}

			width2  >>= 1;
			height2 >>= 1;

			switch( FileType )
			{
			case smTEXFILETYPE_BMP:
				lpDib = LoadDib( MipMapFileBuff[i] );
				if( lpDib == NULL )
				{
					lpDDNextLevel->Release();
					lpDDSurface->Release();
					return NULL;
				}

				if( width2 != GetDibWidth(lpDib) || height2 != GetDibHeight(lpDib) )
				{
					lpDDNextLevel->Release();
					lpDDSurface->Release();

					::strcat( szMipMapFileError, MipMapFileBuff[i] );
					MESSAGE( szMipMapFileError );
					return NULL;
				}

				DrawSurfaceFromDib( lpDDNextLevel, lpDib );
				delete lpDib;
				break;

			case smTEXFILETYPE_JPG:
				cJpeg = new CJpeg;
				cJpeg->LoadJPG( MipMapFileBuff[i] );
				if( cJpeg->m_pData == NULL )
				{
					delete cJpeg;
					lpDDNextLevel->Release();
					lpDDSurface->Release();
					return NULL;
				}

				if( width2 != cJpeg->GetWidth() || height2 != cJpeg->GetHeight() )
				{
					delete cJpeg;
					lpDDNextLevel->Release();
					lpDDSurface->Release();

					::strcat( szMipMapFileError, MipMapFileBuff[i] );
					MESSAGE( szMipMapFileError );
					return NULL;
				}

				DrawSurfaceFromJpeg( lpDDNextLevel, cJpeg );
				delete cJpeg;
				break;

			case smTEXFILETYPE_TGA:
				cTga = new cTGA;
				cTga->LoadTga( MipMapFileBuff[i] );
				if( cTga->m_pBuff == NULL )
				{
					delete cTga;
					lpDDNextLevel->Release();
					lpDDSurface->Release();
					return NULL;
				}

				if( width2 != cTga->GetWidth() || height2 != cTga->GetHeight() )
				{
					delete cTga;
					lpDDNextLevel->Release();
					lpDDSurface->Release();

					::strcat( szMipMapFileError, MipMapFileBuff[i] );
					MESSAGE( szMipMapFileError );
					return NULL;
				}

				DrawSurfaceFromTga( lpDDNextLevel, cTga );
				delete cTga;
                break;
			}

			lpDDSurface2 = lpDDNextLevel;
			lpDDNextLevel->Release();
		}

		lpDDSurface2 = NULL;
		//######################################################################################
		
		lpTexHandle->MapOpacity = FALSE;
	}
	else
	{
		//알파 텍스쳐, 알파채널 합성
		if ( cJpeg ) delete cJpeg;
		if ( lpDib ) delete lpDib;
		if ( cTga ) delete cTga;

		//######################################################################################
		//작 성 자 : 오 영 석
		if( MipMapCount )
		{
			int i;
			if( lpTexHandle->NameA[0] )
			{
				::ZeroMemory( FileTypeBuff, sizeof(FileTypeBuff) );

				int len = ::lstrlen( lpTexHandle->NameA );
				for( i=len-1; i >= 0; i-- )
				{
					if( lpTexHandle->NameA[i] == '.' )
					{
						::CopyMemory( MipMapFileABuff[0], lpTexHandle->NameA, i );

						if( stricmp( &lpTexHandle->NameA[i], ".bmp" ) == 0 )
							::strcpy( FileTypeBuff, ".bmp" );
						else if( stricmp( &lpTexHandle->NameA[i], ".jpg" ) == 0 )
							::strcpy( FileTypeBuff, ".jpg" );
						else if( stricmp( &lpTexHandle->NameA[i], ".tga" ) == 0 )
							::strcpy( FileTypeBuff, ".tga" );
						else
							return -1;
						break;
					}
				}

				for( i=1; i < MIPMAP_FILELIST; i++ )
				{
					::strcpy( MipMapFileABuff[i], MipMapFileABuff[0] );
					::strcat( MipMapFileABuff[i], MipMapFileListBuff[i] );
					::strcat( MipMapFileABuff[i], FileTypeBuff );

					if( PathFileExists( MipMapFileABuff[i] ) )
						MipMapCountA++;
					else
						break;
				}

				::strcat( MipMapFileABuff[0], FileTypeBuff );

				//
				if( MipMapCount != MipMapCountA )
					return -1;

				MipMapCountA++;
			}

			MipMapCount++;


			//######################################################################################
			// 주의................................................................................
			// 예외 처리중에서. lpTexHandle->NameA 으로 들어 오는 넘은 지금 넣지를 않았습니다.
			// 글픽 에서 오파시티로 따로 알파값을 넣은 경우. 같은 텍스쳐를 쓸 경우라고 했습니다.
			// 맥스에서 알파값을 빼서 보기 위해서요.
			// 또한 오파시티에 값이 있다고 해도. 엔진에서. 기본 파일이. tga 32bit 일 경웅.
			// 오파시티 알파값을 무시 하도록 되어 있기에.
			if( MipMapCreateTextureCheck(width, height, MipMapCount) <= 0 )
			{
				::strcat( szMipMapFileError, Filename );
				MESSAGE( szMipMapFileError );
				return -2;
			}
			//######################################################################################

			ZeroMemory( &ddsd, sizeof(DDSURFACEDESC2) );
			ddsd.dwSize          = sizeof(DDSURFACEDESC2);
			ddsd.dwFlags         = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT | DDSD_PIXELFORMAT | DDSD_MIPMAPCOUNT;
			ddsd.dwWidth         = width;
			ddsd.dwHeight        = height;
			ddsd.dwMipMapCount   = MipMapCount;
			ddsd.ddsCaps.dwCaps  = DDSCAPS_MIPMAP | DDSCAPS_COMPLEX | DDSCAPS_TEXTURE;
			ddsd.ddsCaps.dwCaps2 = DDSCAPS2_TEXTUREMANAGE;
			memcpy( &ddsd.ddpfPixelFormat, &ddTextureFormat, sizeof(DDPIXELFORMAT) );
			
			hresult = lpDD->CreateSurface( &ddsd, &lpDDSurface, NULL );
			if( hresult != DD_OK )
				return -2;

			width2  = width;
			height2 = height;
			if( LoadDibSurfaceAlphaMipMap( lpDDSurface, MipMapFileBuff[0], MipMapFileABuff[0], width2, height2 ) == FALSE )
			{
				lpDDSurface->Release();
				return -2;
			}

			DDSCAPS2 ddsCaps = {0,};
			ddsCaps.dwCaps   = DDSCAPS_TEXTURE | DDSCAPS_MIPMAP;

			lpDDSurface2 = lpDDSurface;
			for( i=1; i < MipMapCount; i++ )
			{
				LPDIRECTDRAWSURFACE4 lpDDNextLevel = NULL;
				if( FAILED( lpDDSurface2->GetAttachedSurface(&ddsCaps, &lpDDNextLevel) ) )
				{
					lpDDSurface->Release();
					return -2;
				}

				width2  >>= 1;
				height2 >>= 1;

				int ErrorAlphaMipMap = LoadDibSurfaceAlphaMipMap( lpDDNextLevel, MipMapFileBuff[i], MipMapFileABuff[i], width2, height2 );
				if( ErrorAlphaMipMap == -1 )
				{
					lpDDNextLevel->Release();
					lpDDSurface->Release();

					::strcat( szMipMapFileError, MipMapFileBuff[i] );
					MESSAGE( szMipMapFileError );
					return -2;
				}
				else if( ErrorAlphaMipMap == -2 )
				{
					lpDDNextLevel->Release();
					lpDDSurface->Release();

					::strcat( szMipMapFileError, MipMapFileABuff[i] );
					MESSAGE( szMipMapFileError );
					return -2;
				}

				lpDDSurface2 = lpDDNextLevel;
				lpDDNextLevel->Release();
			}

			if( ! lpTexHandle->NameA[0] )
				lpTexHandle->NameA[0] = TRUE;

			lpDDSurface2 = NULL;
		}
		else
		{
			lpDDSurface = LoadDibSurfaceAlpha( Filename , lpTexHandle->NameA , TexSwap );
			if( ! lpTexHandle->NameA[0] ) lpTexHandle->NameA[0] = TRUE;
			if( lpDDSurface == NULL )
				return -2;
		}
		//######################################################################################

		lpTexHandle->MapOpacity = TRUE;
	}

	//스왑 비디오 메모리에 작은 사이즈 서페이스 만듬
	if ( TexSwap==TRUE || TexHalf || ( TexturHalfSize && width>32 && height>32 && lpTexHandle->TexSwapMode ) )
	{
		if( TexturHalfSize == 2 ) {
			width2 = width/4;
			height2 = height/4;
		}
		else {
			width2 = width/2;
			height2 = height/2;
		}

		ZeroMemory( &ddsd, sizeof(DDSURFACEDESC2) );
		ddsd.dwSize		= sizeof(DDSURFACEDESC2);
		ddsd.dwWidth	= width2;
	    ddsd.dwHeight	= height2;
		ddsd.dwFlags	= DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT;

		//알파 채널이 있으면 픽셀 포맷을 정해줘야함
		if ( lpTexHandle->MapOpacity ) {
			memcpy( &ddsd.ddpfPixelFormat, &ddTextureFormat, sizeof(DDPIXELFORMAT) );
		}
		else {
			memcpy( &ddsd.ddpfPixelFormat , &ddDefaultTextureFormat, sizeof(DDPIXELFORMAT) );
		}

		//######################################################################################
		//작 성 자 : 오 영 석
		if ( lpD3DDeviceDesc->bIsHardware )
		{
			//ddsd.ddsCaps.dwCaps = DDSCAPS_VIDEOMEMORY | DDSCAPS_TEXTURE;
			ddsd.ddsCaps.dwCaps  = DDSCAPS_TEXTURE;
			ddsd.ddsCaps.dwCaps2 = DDSCAPS2_TEXTUREMANAGE;
		}
	    else
	        ddsd.ddsCaps.dwCaps = DDSCAPS_SYSTEMMEMORY| DDSCAPS_TEXTURE;
		//######################################################################################

		hresult = lpDD->CreateSurface( &ddsd, &lpDDSurface2, NULL );
		if ( hresult != DD_OK ) { 
			lpDDSurface->Release();
			return NULL; 
		}

		//######################################################################################
		//작 성 자 : 오 영 석

		//서페이스 축소 복사
		RECT srcRect, destRect;

		srcRect.top = 0;
		srcRect.left = 0;
		srcRect.right = width;
		srcRect.bottom = height;

		destRect.top = 0;
		destRect.left = 0;
		destRect.right = width2;
		destRect.bottom = height2;

		lpDDSurface2->Blt( &destRect, lpDDSurface, &srcRect, DDBLT_WAIT, NULL );

		/*
		int cntW , cntH , DestCnt , SrcCnt;
		WORD *CopySrc , *CopyDest;

		ZeroMemory( &ddsd2, sizeof(DDSURFACEDESC2) );
		ddsd2.dwSize		= sizeof(DDSURFACEDESC2);

		lpDDSurface->Lock(NULL,&ddsd,DDLOCK_WAIT,NULL);
		CopySrc = (WORD *)ddsd.lpSurface;
		lpDDSurface2->Lock(NULL,&ddsd2,DDLOCK_WAIT,NULL);
		CopyDest = (WORD *)ddsd2.lpSurface;

		if ( CopySrc && CopyDest ) {
			if( TexturHalfSize==2 ) {
				// 1/4 크기 텍스쳐 제작
				for( cntH=0; cntH<height ; cntH+=4 ) {
					DestCnt = 0;
					SrcCnt = 0;
					for( cntW=0; cntW<width ; cntW+=4 ) {
						CopyDest[DestCnt] = CopySrc[SrcCnt];
						DestCnt++;
						SrcCnt +=4;
					}
					CopySrc += ddsd.lPitch*2;
					CopyDest += ddsd2.lPitch/2;
				}
			}
			else {
				// 1/2 크기 텍스쳐 제작
				for( cntH=0; cntH<height ; cntH+=2 ) {
					DestCnt = 0;
					SrcCnt = 0;
					for( cntW=0; cntW<width ; cntW+=2 ) {
						CopyDest[DestCnt] = CopySrc[SrcCnt];
						DestCnt++;
						SrcCnt +=2;
					}
					CopySrc += ddsd.lPitch;
					CopyDest += ddsd2.lPitch/2;
				}
			}
		}
		lpDDSurface2->Unlock(NULL);
		lpDDSurface->Unlock(NULL);
		*/
		//######################################################################################


		//######################################################################################
		//작 성 자 : 오 영 석
		
		//if ( TexturHalfSize || TexHalf ) {
			//절반 모드일 경우 시스템 메모리 텍스쳐 제거
		//	lpDDSurface->Release();
		//}
		//else {
			//시스템 메모리 스왑 가능설정
		//	lpTexHandle->lpDDSSysMemory = lpDDSurface;
		//}

		lpTexHandle->lpDDSurface = lpDDSurface2;
		lpTexHandle->lpDDSSysMemory = NULL;

		if( lpDDSurface2 != NULL )
		{
			hresult = lpDDSurface2->QueryInterface( IID_IDirect3DTexture2, (LPVOID*)&lpD3DTexture );
			if( hresult == D3D_OK )
				lpTexHandle->lpD3DTexture = lpD3DTexture;
			else
			{
				lpDDSurface2->Release();
				lpTexHandle->lpDDSurface = NULL;
				return 0;
			}
		}
		//######################################################################################

		//비디오 메모리 사용 예상치
		VramTotal += width2 * height2 * 2 ;

		return TRUE;
	}

	lpTexHandle->lpDDSurface = lpDDSurface;
	lpTexHandle->lpDDSSysMemory = NULL;
	if ( lpDDSurface!=NULL ) {
		hresult = lpDDSurface->QueryInterface( IID_IDirect3DTexture2, (LPVOID*)&lpD3DTexture );
		if( hresult == D3D_OK )
			lpTexHandle->lpD3DTexture = lpD3DTexture;
		else
		{
			lpDDSurface->Release();
			lpTexHandle->lpDDSurface = NULL;
			return 0;
		}
	}

	//비디오 메모리 사용 예상치
	VramTotal += width * height * 2 ;

	return TRUE;
}




/////////////////[ BMP 파일을 OFFSCREEN 서페이스로 만든다 //////////////
LPDIRECTDRAWSURFACE4 LoadDibSurfaceOffscreen( char *Filename )
{
	//######################################################################################
	//작 성 자 : 오 영 석
	return new_LoadBmpSurfaceOffscreen( Filename );
	//######################################################################################


	BYTE 				*lpDib;
	BYTE				*dAddr;
	int					height,width;
	
	lpDib = LoadDib( Filename );
	if ( lpDib==NULL ) return NULL;

	dAddr	=GetDibBitsAddr(lpDib);
	height	=GetDibHeight(lpDib);
	width	=GetDibWidth(lpDib);

	// Now create surface
	LPDIRECTDRAWSURFACE4 lpDDSurface;
	DDSURFACEDESC2		ddsd;

	ZeroMemory( &ddsd, sizeof(DDSURFACEDESC2) );

	ddsd.dwSize		= sizeof(DDSURFACEDESC2);
	ddsd.dwFlags	= DDSD_CAPS |
					  DDSD_HEIGHT |
					  DDSD_WIDTH;
	ddsd.dwWidth	= width;
    ddsd.dwHeight	= height;
	ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;

    hresult = lpDD->CreateSurface( &ddsd, &lpDDSurface, NULL );
    if ( hresult != DD_OK ) { delete lpDib; return NULL; }

/*
	HDC	hdc;
	lpDDSurface->GetDC( &hdc );
	SetDIBitsToDevice( hdc , 0,0,width,height,0,0, 0,height, (LPSTR) dAddr,
		(LPBITMAPINFO )lpDib , DIB_RGB_COLORS );
	lpDDSurface->ReleaseDC( hdc );
	delete lpDib;
*/
	DrawSurfaceFromDib( lpDDSurface , lpDib );
	delete lpDib;

	// RGB( 0,0,0 ) ColorKey;
	DDCOLORKEY ColKey;
	ColKey.dwColorSpaceLowValue=RGB(0,0,0); 
    ColKey.dwColorSpaceHighValue=RGB(255,255,255);  
	lpDDSurface->SetColorKey(DDCKEY_SRCBLT  , &ColKey );

	return lpDDSurface;
}









extern HWND hwnd;


LPDIRECTDRAWSURFACE4	lpDDSMapTexture[256];
LPDIRECTDRAWSURFACE4	lpDDSMapTextureVideo[8*8];
LPDIRECT3DTEXTURE2		lpD3DMapTexture[8*8];

int MapTexCode[8][8];

int MapTextureSize;

int LoadDibMapTexture( char *Filename, int tSize ) 
{

	BYTE 				*lpDib;
	BYTE				*dAddr;
	int					height,width;

	
	lpDib = LoadDib( Filename );
	if ( lpDib==NULL ) return NULL;

	dAddr	=GetDibBitsAddr(lpDib);
	height	=GetDibHeight(lpDib);
	width	=GetDibWidth(lpDib);

	MapTextureSize = tSize;

	int cnt;

	// Now create surface
	DDSURFACEDESC2		ddsd;

	for( cnt=0;cnt<64; cnt++) {
		ZeroMemory( &ddsd, sizeof(DDSURFACEDESC2) );

		ddsd.dwSize		= sizeof(DDSURFACEDESC2);
		ddsd.dwFlags	= DDSD_CAPS |
						  DDSD_HEIGHT |
						  DDSD_WIDTH  ;
		ddsd.dwWidth	= tSize;
	    ddsd.dwHeight	= tSize;

		//######################################################################################
		//작 성 자 : 오 영 석
		if ( lpD3DDeviceDesc->bIsHardware )
		{
	        //ddsd.ddsCaps.dwCaps = DDSCAPS_VIDEOMEMORY | DDSCAPS_TEXTURE;
			ddsd.ddsCaps.dwCaps  = DDSCAPS_TEXTURE;
			ddsd.ddsCaps.dwCaps2 = DDSCAPS2_TEXTUREMANAGE;
		}
	    else
	        ddsd.ddsCaps.dwCaps = DDSCAPS_SYSTEMMEMORY| DDSCAPS_TEXTURE;
		//######################################################################################

	    hresult = lpDD->CreateSurface( &ddsd, &lpDDSMapTextureVideo[cnt], NULL );
		if ( hresult != DD_OK ) { delete lpDib; return NULL;	}
	}

	for( cnt=0;cnt<256; cnt++) {
		ZeroMemory( &ddsd, sizeof(DDSURFACEDESC2) );

		ddsd.dwSize		= sizeof(DDSURFACEDESC2);
		ddsd.dwFlags	= DDSD_CAPS |
						  DDSD_HEIGHT |
						  DDSD_WIDTH  ;
		ddsd.dwWidth	= tSize;
	    ddsd.dwHeight	= tSize;

        ddsd.ddsCaps.dwCaps =  DDSCAPS_SYSTEMMEMORY|DDSCAPS_TEXTURE;

	    hresult = lpDD->CreateSurface( &ddsd, &lpDDSMapTexture[cnt], NULL );
		if ( hresult != DD_OK ) { delete lpDib; return NULL;	}
	}


	HBITMAP	TempBitmap;
	HDC		TempDC;
	HDC		BitmapDC;
	HDC		hdc;
	int		cntX,cntY,cntH;
	RECT	ddRect;


	TempDC = GetDC( hwnd );
	TempBitmap = CreateCompatibleBitmap( TempDC, width, height );
	BitmapDC = CreateCompatibleDC(TempDC);
	SelectObject( BitmapDC, TempBitmap );
	SetDIBitsToDevice( BitmapDC , 0,0,width,height,0,0, 0,height, (LPSTR) dAddr, (LPBITMAPINFO )lpDib , DIB_RGB_COLORS );

	cnt = 0;
	for( cntY=0;cntY<16;cntY++) {
		cntH = 15-cntY;
		for( cntX=0;cntX<16;cntX++) {

			lpDDSMapTexture[cnt]->GetDC( &hdc );

			BitBlt( hdc , 1,1, tSize-2, tSize-2 , BitmapDC, cntX*(tSize-2) , cntH*(tSize-2) , SRCCOPY );

			BitBlt( hdc , 0,0,       tSize, 1 , BitmapDC, cntX*(tSize-2) , cntH*(tSize-2) , SRCCOPY );
			BitBlt( hdc , 0,tSize-1, tSize, 1 , BitmapDC, cntX*(tSize-2) , cntH*(tSize-2)+(tSize-1) , SRCCOPY );

			BitBlt( hdc , 0,0,       1, tSize , BitmapDC, cntX*(tSize-2) , cntH*(tSize-2) , SRCCOPY );
			BitBlt( hdc , tSize-1, 0, 1, tSize , BitmapDC, cntX*(tSize-2)+(tSize-1) , cntH*(tSize-2), SRCCOPY );

			lpDDSMapTexture[cnt]->ReleaseDC( hdc );
			cnt++;
		}
	}

	DeleteDC( BitmapDC );
	DeleteObject( TempBitmap );
	ReleaseDC( hwnd, TempDC );

	delete lpDib;


	for( cnt=0;cnt<64;cnt++) {
	    hresult = lpDDSMapTextureVideo[cnt]->QueryInterface( IID_IDirect3DTexture2, (LPVOID*)&lpD3DMapTexture[cnt] );
		if ( hresult != D3D_OK ) {
			lpDDSMapTextureVideo[cnt]->Release();
			lpDDSMapTextureVideo[cnt] = 0;
			lpD3DMapTexture[cnt] = 0;
			return NULL;
		}
	}



	for(cntY=0;cntY<8;cntY++) {
		for(cntX=0;cntX<8;cntX++) {


			ddRect.left   = 0;
			ddRect.right  = tSize;
			ddRect.top    = 0;
			ddRect.bottom = tSize;


			lpDDSMapTextureVideo[cntX+(cntY*8)]->BltFast(0,0,lpDDSMapTexture[cntX+(cntY*16)],&ddRect,DDBLTFAST_WAIT|DDBLTFAST_NOCOLORKEY);

			MapTexCode[cntX][cntY] = cntX+(cntY*16);

		}
	}


	return TRUE;

}

int RemoveMapTexture()
{
	int cnt;

	for(cnt=63;cnt>=0;cnt--) {
		lpD3DMapTexture[ cnt ]->Release();
		lpDDSMapTextureVideo[ cnt ]->Release();
	}

	for(cnt=255;cnt>=0;cnt--) {
		lpDDSMapTexture[ cnt ]->Release();
	}

	return TRUE;
}



int SetMapTextureMem( int x, int y )
{

	RECT ddRect;
	int mx,my;
	int vx,vy;
	int ddval;
	int sc,dc;

	ddRect.left   = 0;
	ddRect.right  = MapTextureSize;
	ddRect.top    = 0;
	ddRect.bottom = MapTextureSize;

	mx = x&15;
	my = y&15;

	vx = x&7;
	vy = y&7;

	dc = vx+(vy<<3);
	sc = mx+(my<<4);

	if ( MapTexCode[vx][vy]!=sc ) {
		ddval = lpDDSMapTextureVideo[dc]->BltFast(0,0,lpDDSMapTexture[sc],&ddRect,DDBLTFAST_WAIT|DDBLTFAST_NOCOLORKEY);
		MapTexCode[ vx ][ vy ] = sc;
	}

	return ddval;
}

int SetMapTextureWidth( int x, int y )
{
	int cnt;

	for( cnt=0;cnt<8;cnt++ ) {
		SetMapTextureMem( x+cnt, y );
	}

	return TRUE;
}

int SetMapTextureHeight( int x, int y )
{
	int cnt;

	for( cnt=0;cnt<8;cnt++ ) {
		SetMapTextureMem( x, y+cnt );
	}


	return TRUE;
}

int SetMapImage( int x , int y )
{
	int sx,sy;
	int cntX,cntY;

	sx = x>>(ShiftMAPCELL_DIV+3);
	sy = y>>(ShiftMAPCELL_DIV+3);


	for( cntX=sx-3;cntX<sx+4;cntX++) {
		for( cntY=sy-3;cntY<sy+4;cntY++) {
			SetMapTextureMem( cntX, cntY );
		}
	}


/*
	SetMapTextureWidth( sx-3 , sy+3 );
	SetMapTextureWidth( sx-3 , sy-3 );

	SetMapTextureWidth( sx+3 , sy-3 );
	SetMapTextureWidth( sx-3 , sy-3 );
*/
	return TRUE;

}








LPDIRECTDRAWSURFACE4 LoadDibBGTexture( char *Filename ) 
{

	BYTE 				*lpDib;
	BYTE				*dAddr;
	int					height,width;
	LPDIRECTDRAWSURFACE4	lpDDSurface;

	
	lpDib = LoadDib( Filename );
	if ( lpDib==NULL ) return NULL;

	dAddr	=GetDibBitsAddr(lpDib);
	height	=GetDibHeight(lpDib);
	width	=GetDibWidth(lpDib);


		// Now create surface
		DDSURFACEDESC2		ddsd;

		ZeroMemory( &ddsd, sizeof(DDSURFACEDESC2) );

		ddsd.dwSize		= sizeof(DDSURFACEDESC2);
		ddsd.dwFlags	= DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT;
		ddsd.dwWidth	= width+2;
	    ddsd.dwHeight	= height;

		memcpy( &ddsd.ddpfPixelFormat, &ddDefaultTextureFormat, sizeof(DDPIXELFORMAT) );

        ddsd.ddsCaps.dwCaps =  DDSCAPS_SYSTEMMEMORY|DDSCAPS_TEXTURE;

	    hresult = lpDD->CreateSurface( &ddsd, &lpDDSurface, NULL );
		if ( hresult != DD_OK ) { delete lpDib; return NULL;	}


	HBITMAP	TempBitmap;
	HDC		TempDC;
	HDC		BitmapDC;
	HDC		hdc;


	TempDC = GetDC( hwnd );
	TempBitmap = CreateCompatibleBitmap( TempDC, width, height );
	BitmapDC = CreateCompatibleDC(TempDC);
	SelectObject( BitmapDC, TempBitmap );
	SetDIBitsToDevice( BitmapDC , 0,0,width,height,0,0, 0,height, (LPSTR) dAddr, (LPBITMAPINFO )lpDib , DIB_RGB_COLORS );

	lpDDSurface->GetDC( &hdc );

	BitBlt( hdc , 1,0, width , height , BitmapDC, 0 , 0 , SRCCOPY );

	BitBlt( hdc , 0,0, 1, height , BitmapDC, 0 , 0 , SRCCOPY );
	BitBlt( hdc , width+1 , 0, 1, height , BitmapDC, width-1, 0 , SRCCOPY );

	lpDDSurface->ReleaseDC( hdc );

	DeleteDC( BitmapDC );
	DeleteObject( TempBitmap );
	ReleaseDC( hwnd, TempDC );

	delete lpDib;

	return lpDDSurface;

}


char *BGTextureName[32] = {
	"clo00.bmp",
	"clo01.bmp",
	"clo02.bmp",
	"clo03.bmp",
	"clo04.bmp",
	"clo05.bmp",
	"clo06.bmp",
	"clo07.bmp",
	"clo08.bmp",
	"clo09.bmp",
	"clo10.bmp",
	"clo11.bmp",
	"clo12.bmp",
	"clo13.bmp",
	"clo14.bmp",
	"clo15.bmp",
	"clo16.bmp",
	"clo17.bmp",
	"clo18.bmp",
	"clo19.bmp",
	"clo20.bmp",
	"clo21.bmp",
	"clo22.bmp",
	"clo23.bmp",
	"clo24.bmp",
	"clo25.bmp",
	"clo26.bmp",
	"clo27.bmp",
	"clo28.bmp",
	"clo29.bmp",
	"clo30.bmp",
	"clo31.bmp"
};


LPDIRECTDRAWSURFACE4	lpDDSAnimBG[32];

int DrawAnimBG( LPDIRECTDRAWSURFACE4 lpDDSSrc , int frame )
{

	RECT ddRect;
	int ddval;

	ddRect.left   = 0;
	ddRect.right  = 256;
	ddRect.top    = 0;
	ddRect.bottom = 128;

	ddval = lpDDSSrc->BltFast(0,90,lpDDSAnimBG[frame&0x1F],&ddRect,DDBLTFAST_WAIT|DDBLTFAST_NOCOLORKEY);

	return ddval;
}

int LoadAnimBG()
{
	int cnt;
	char strName[64];

	for(cnt=0;cnt<32;cnt++) {
		lstrcpy( strName , "cloud\\" );
		lstrcat( strName , BGTextureName[cnt] );
		lpDDSAnimBG[cnt] = LoadDibBGTexture( strName  );
	}

	return TRUE;
}

int ClearBG( LPDIRECTDRAWSURFACE4 lpDDSSrc, DWORD color )
{


    // Surface 지우기
    DDBLTFX bltFx;
    ZeroMemory( &bltFx, sizeof(DDBLTFX) );
    bltFx.dwSize        = sizeof(DDBLTFX);
    bltFx.dwFillColor   = color;
    lpDDSSrc->Blt( NULL, NULL, NULL, DDBLT_WAIT | DDBLT_COLORFILL, &bltFx );

	return TRUE;

}

int CloseAnimBG()
{
	int cnt;
	for( cnt=31;cnt>=0;cnt--) {
		if ( lpDDSAnimBG[cnt] ) lpDDSAnimBG[cnt]->Release();
	}

	return TRUE;
}


//######################################################################################
//작 성 자 : 오 영 석
int new_smCreateTexture( smTEXTUREHANDLE *lpTexHandle, int MipMap )
{
	//
	MipMapCount = 0;
	::ZeroMemory( MipMapFileBuff, sizeof(MipMapFileBuff) );
	MipMapCountA = 0;
	::ZeroMemory( MipMapFileABuff, sizeof(MipMapFileABuff) );
	::ZeroMemory( FileTypeBuff, sizeof(FileTypeBuff) );

	//
	char *Filename = lpTexHandle->Name;
	int	  FileType = GetTextureFileType( Filename );
	if( FileType < 0 )
		return -1;

	//
	int					 TexHalf = 0;
	LPDIRECTDRAWSURFACE4 lpDDSurface = NULL;

	if( FileType == smTEXFILETYPE_BMP )
	{
		lpDDSurface = new_LoadBmpCreateTexSurface( Filename, MipMap, TexHalf );
		if( lpDDSurface )
			lpTexHandle->MapOpacity = FALSE;
		else
			return -1;
	}
	else if( FileType == smTEXFILETYPE_TGA )
	{
		lpDDSurface = new_LoadTgaCreateTexSurface( Filename, MipMap, TexHalf );
		if( lpDDSurface )
		{
			lpTexHandle->MapOpacity = TRUE;
			if( ! lpTexHandle->NameA[0] )
                lpTexHandle->NameA[0] = TRUE;
		}
		else
			return -1;
	}
	else
		return -1;

	/////////////////////////////////////////////////////////////////////////////////////////
	//
	LPDIRECT3DTEXTURE2 lpD3DTexture;

	// 작은 사이즈 텍스쳐 만듬
	if( TexHalf || (TexturHalfSize && g_DibLastReadWidth > 32 && g_DibLastReadHeight > 32 && lpTexHandle->TexSwapMode) )
	{
		int width2, height2;

		if( TexturHalfSize == 2 )
		{
			width2  = g_DibLastReadWidth  >> 2;
			height2 = g_DibLastReadHeight >> 2;
		}
		else
		{
			width2  = g_DibLastReadWidth  >> 1;
			height2 = g_DibLastReadHeight >> 1;
		}

		DDSURFACEDESC2 ddsd;
		ZeroMemory( &ddsd, sizeof(DDSURFACEDESC2) );

		ddsd.dwSize		= sizeof(DDSURFACEDESC2);
		ddsd.dwFlags	= DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT;
		ddsd.dwWidth	= width2;
	    ddsd.dwHeight	= height2;	

		if( FileType == smTEXFILETYPE_BMP )
			memcpy( &ddsd.ddpfPixelFormat , &ddDefaultTextureFormat, sizeof(DDPIXELFORMAT) );
		else
			memcpy( &ddsd.ddpfPixelFormat , &ddTextureFormat, sizeof(DDPIXELFORMAT) );

		if( lpD3DDeviceDesc->bIsHardware )
		{
			ddsd.ddsCaps.dwCaps  = DDSCAPS_TEXTURE;
			ddsd.ddsCaps.dwCaps2 = DDSCAPS2_TEXTUREMANAGE;
		}
		else
			ddsd.ddsCaps.dwCaps = DDSCAPS_SYSTEMMEMORY | DDSCAPS_TEXTURE;

		LPDIRECTDRAWSURFACE4 lpDDSurface2;
		hresult = lpDD->CreateSurface( &ddsd, &lpDDSurface2, NULL );
		if ( hresult != DD_OK )
		{ 
			lpDDSurface->Release();
			lpTexHandle->lpDDSurface = NULL;
			return 0;
		}

		//
		RECT srcRect, destRect;

		srcRect.top    = 0;
		srcRect.left   = 0;
		srcRect.right  = g_DibLastReadWidth;
		srcRect.bottom = g_DibLastReadHeight;

		destRect.top    = 0;
		destRect.left   = 0;
		destRect.right  = width2;
		destRect.bottom = height2;

		lpDDSurface2->Blt( &destRect, lpDDSurface, &srcRect, DDBLT_WAIT, NULL );
		lpDDSurface->Release();

		g_DibLastReadWidth  = width2;
		g_DibLastReadHeight = height2;

		lpTexHandle->Width			= width2;
		lpTexHandle->Height			= height2;
		lpTexHandle->lpDDSurface    = lpDDSurface2;
		lpTexHandle->lpDDSSysMemory = NULL;

		hresult = lpDDSurface2->QueryInterface( IID_IDirect3DTexture2, (LPVOID*)&lpD3DTexture );
		if( hresult == D3D_OK )
			lpTexHandle->lpD3DTexture = lpD3DTexture;
		else
		{
			lpDDSurface2->Release();
			lpTexHandle->lpDDSurface = NULL;
			return 0;
		}

		//비디오 메모리 사용 예상치
		VramTotal += width2 * height2 * 2;
	}
	else
	{
		//
		lpTexHandle->Width			= g_DibLastReadWidth;
		lpTexHandle->Height			= g_DibLastReadHeight;
		lpTexHandle->lpDDSurface    = lpDDSurface;
		lpTexHandle->lpDDSSysMemory = NULL;

		hresult = lpDDSurface->QueryInterface( IID_IDirect3DTexture2, (LPVOID*)&lpD3DTexture );
		if( hresult == D3D_OK )
			lpTexHandle->lpD3DTexture = lpD3DTexture;
		else
		{
			lpDDSurface->Release();
			lpTexHandle->lpDDSurface = NULL;
			return 0;
		}

		//비디오 메모리 사용 예상치
		VramTotal += g_DibLastReadWidth * g_DibLastReadHeight * 2;
	}

	return TRUE;
}


/////////////////////////////////////////////////////////////////////////////////////////
//
int new_LoadDib( char *Filename )
{
	HANDLE				hFile;
	DWORD				dwAcess;
	BITMAPFILEHEADER	bmfh;

	//파일을 연다
	hFile = CreateFile( Filename , GENERIC_READ , FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING , FILE_ATTRIBUTE_NORMAL , NULL );
	if( hFile == INVALID_HANDLE_VALUE )
		return NULL;

	//비트맵 헤더 읽음
	ReadFile( hFile , &bmfh , sizeof( BITMAPFILEHEADER ) , &dwAcess , NULL );
	if( bmfh.bfType != *(WORD *)"BM" && bmfh.bfType != *(WORD *)"C8" && bmfh.bfType != *(WORD *)"A8" ) 
	{
		CloseHandle( hFile );
		return NULL;
	}

	//헤더 암호화 버전
	if( bmfh.bfType == *(WORD *)"A8" )
	{
		BYTE	ch;
		BYTE	*lpHeader;
		int		cnt;

		//헤더를 암호화
		lpHeader = (BYTE *)&bmfh;
		for( cnt=0;cnt<sizeof(bmfh);cnt++)
		{
			ch = (BYTE)( lpHeader[cnt]-(cnt*cnt) );
			lpHeader[cnt] = ch;
		}
	}

	DWORD dwDibSize = bmfh.bfSize - sizeof(BITMAPFILEHEADER);
	if ( !ReserveDibImageBuff( dwDibSize ) ) {
		CloseHandle( hFile );
		return NULL;
	}
	BYTE *lpDib     = g_DibImageDateBuff;
	
	//비트맵 읽음
	ReadFile( hFile , lpDib , dwDibSize , &dwAcess , NULL );
	CloseHandle( hFile );

	DWORD dwHeaderSize = GetDibInfoHeaderSize(lpDib);
	if( dwHeaderSize < 12 || (dwHeaderSize>12 && dwHeaderSize < 16) ) 
		return NULL;

	return TRUE;
}


/////////////////////////////////////////////////////////////////////////////////////////
//
int new_LoadTga( char *Filename )
{
    BYTE	TGAHeader[0x12];
	HANDLE	hFile;
	DWORD	dwAcess;

	//파일을 연다
	hFile = CreateFile( Filename , GENERIC_READ , FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING , FILE_ATTRIBUTE_NORMAL , NULL );
	if( hFile == INVALID_HANDLE_VALUE )
		return NULL;

	//TGA 헤더 읽음
	ReadFile( hFile , TGAHeader , 0x12 , &dwAcess , NULL );

	//헤더 암호화 버전
	if ( TGAHeader[0]=='G' && TGAHeader[1]=='8' ) {
		BYTE	ch;
		BYTE	*lpHeader;
		int		cnt;

		//헤더를 암호화
		lpHeader = (BYTE *)&TGAHeader;
		for( cnt=0;cnt<sizeof(TGAHeader);cnt++) {
			ch = (BYTE)( lpHeader[cnt]-(cnt*cnt) );
			lpHeader[cnt] = ch;
		}
	}

	g_DibLastReadWidth  = *((WORD *)(&TGAHeader[0xC]));
	g_DibLastReadHeight = *((WORD *)(&TGAHeader[0xE]));

	if ( (TGAHeader[0x11]&0x20) )
		g_TgaLastSwapHeight = FALSE;
	else
		g_TgaLastSwapHeight = TRUE;

	switch( TGAHeader[0x10] )
	{
	//case 16:
	case 24:
		g_TgaLastBitCnt = 24;
		if ( !ReserveDibImageBuff( g_DibLastReadWidth*g_DibLastReadHeight*3 ) ) { CloseHandle( hFile ); return NULL; }
		ReadFile( hFile, g_DibImageDateBuff, g_DibLastReadWidth*g_DibLastReadHeight*3, &dwAcess, NULL );
		break;

	case 32:
		g_TgaLastBitCnt = 32;
		if ( !ReserveDibImageBuff( g_DibLastReadWidth*g_DibLastReadHeight*4 ) ) { CloseHandle( hFile ); return NULL; }
		ReadFile( hFile, g_DibImageDateBuff, g_DibLastReadWidth*g_DibLastReadHeight*4, &dwAcess, NULL );
		break;

	default:
		CloseHandle( hFile );
		return NULL;
	}

	CloseHandle( hFile );
	return TRUE;
}


/////////////////////////////////////////////////////////////////////////////////////////
//
int New_DrawSurfaceFromTga( LPDIRECTDRAWSURFACE4 lpDDSurface )
{
	BYTE *dAddr  = g_DibImageDateBuff;
	int   width  = g_DibLastReadWidth;
	int   height = g_DibLastReadHeight;

	//
	int i,j,k;
	int r,g,b,a,tp;	
	DWORD MaskAll;

	DWORD Mask[4];
	DWORD Shift[4];
	DWORD ShiftPixel[4];
	DWORD ShiftTemp;

	Mask[0] = ddTextureFormat.dwRBitMask;
	Mask[1] = ddTextureFormat.dwGBitMask;
	Mask[2] = ddTextureFormat.dwBBitMask;
	Mask[3] = ddTextureFormat.dwRGBAlphaBitMask;

	MaskAll = Mask[0]|Mask[1]|Mask[2]|Mask[3];

	for( i=0; i<4; i++ )
	{
		if ( Mask[i] )
		{
			ShiftTemp = Mask[i];
			for( j=0;j<32;j++ )
			{
				if ( ShiftTemp&1 ) break;
				ShiftTemp>>=1;
			}
			Shift[i] = j;
			for( j=0;j<32;j++)
			{
				if ( (ShiftTemp&1)==0 ) break;
				ShiftTemp>>=1;
			}
			ShiftPixel[i] = j;
		}
		else
		{
			Shift[i] = 32;
			ShiftPixel[i] = 32;
		}
	}

	//
	BYTE *Dest;
	int   pBuff;
	int   incByte;
	int   pBuff_k;

	DDSURFACEDESC2 ddsd;
	ZeroMemory( &ddsd, sizeof(DDSURFACEDESC2) );

	ddsd.dwSize = sizeof(DDSURFACEDESC2);
	lpDDSurface->Lock( NULL, &ddsd, DDLOCK_WAIT, NULL );
	
	Dest = (BYTE *)ddsd.lpSurface;
	if( Dest == NULL )
	{
		lpDDSurface->Release();
		return NULL;
	}

	pBuff_k = 0;
	k=0;
	for( i=0; i < height; i++ )
	{
		switch( g_TgaLastBitCnt )
		{
		case 24:
			incByte = 3;
			if( g_TgaLastSwapHeight )		pBuff = ((height-i-1)*width)*3;
			else							pBuff = (i*width)*3;
			break;

		case 32:
			incByte = 4;
			if( g_TgaLastSwapHeight )		pBuff = ((height-i-1)*width)<<2;
			else							pBuff = (i*width)<<2;
			break;
		}

		k = pBuff_k;
		for( j=0; j < width; j++ )
		{
			b = dAddr[ pBuff ]; 
			g = dAddr[ pBuff +1]; 
			r = dAddr[ pBuff +2];

			if ( g_TgaLastBitCnt==32 )
                a = dAddr[ pBuff +3];
			else
				a = 255;

			pBuff += incByte;

			r = (( r>>(8-ShiftPixel[0] ) )<< Shift[0] ) & Mask[0];
			g = (( g>>(8-ShiftPixel[1] ) )<< Shift[1] ) & Mask[1];
			b = (( b>>(8-ShiftPixel[2] ) )<< Shift[2] ) & Mask[2];
			a = (( a>>(8-ShiftPixel[3] ) )<< Shift[3] ) & Mask[3];

			tp = r|g|b|a;											// 하이컬러로 변환

			if( MaskAll <= 0xFFFF )
			{
				*((WORD *)&Dest[ k ]) = (WORD)tp;
				k+=2;
			}
			else {
				*((DWORD *)&Dest[ k ]) = tp;
				k+=4;
			}
		}

		pBuff_k += (int)ddsd.lPitch;

	}

    lpDDSurface->Unlock(NULL);
	return TRUE;
}


/////////////////////////////////////////////////////////////////////////////////////////
//
LPDIRECTDRAWSURFACE4 new_LoadBmpCreateTexSurface( char *Filename, int MipMap, int &rTexHalf )
{
	//
	if( new_LoadDib(Filename) == NULL )
		return NULL;

	//
	BYTE *lpDib = g_DibImageDateBuff;
	g_DibLastReadWidth  = GetDibWidth(lpDib);
	g_DibLastReadHeight = GetDibHeight(lpDib);

	if( MipMap && MipMapModeCreate && g_DibLastReadWidth < 512 && g_DibLastReadHeight < 512 )
	{
		if( MipMapModeCreate == NOSQUARE_MODE || g_DibLastReadWidth == g_DibLastReadHeight )
		{
			int i, len = ::lstrlen( Filename );
			for( i=len-1; i >= 0; i-- )
			{
				if( Filename[i] == '.' )
				{
					::CopyMemory( MipMapFileBuff[0], Filename, i );
					break;
				}
			}

			// 프테 에서는밉맵 최대 3단계 까지만 사용 하기로 했음.
			for( i=1; i < MIPMAP_FILELIST; i++ )
			{
				::strcpy( MipMapFileBuff[i], MipMapFileBuff[0] );
				::strcat( MipMapFileBuff[i], MipMapFileListBuff[i] );
				::strcat( MipMapFileBuff[i], ".bmp" );

				if( PathFileExists( MipMapFileBuff[i] ) )
					MipMapCount++;
				else
					break;
			}

			::strcat( MipMapFileBuff[0], ".bmp" );
		}
	}

	//
	DDSURFACEDESC2 ddsd;
	ZeroMemory( &ddsd, sizeof(DDSURFACEDESC2) );

	memcpy( &ddsd.ddpfPixelFormat , &ddDefaultTextureFormat, sizeof(DDPIXELFORMAT) );
	ddsd.dwSize	  = sizeof(DDSURFACEDESC2);
	ddsd.dwWidth  = g_DibLastReadWidth;
	ddsd.dwHeight = g_DibLastReadHeight;

	if( MipMapCount )
	{
		MipMapCount++;
		if( MipMapCreateTextureCheck(g_DibLastReadWidth, g_DibLastReadHeight, MipMapCount) <= 0 )
			return NULL;

		ddsd.dwFlags         = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT | DDSD_PIXELFORMAT | DDSD_MIPMAPCOUNT;		
		ddsd.dwMipMapCount   = MipMapCount;
		ddsd.ddsCaps.dwCaps  = DDSCAPS_MIPMAP | DDSCAPS_COMPLEX | DDSCAPS_TEXTURE;
		ddsd.ddsCaps.dwCaps2 = DDSCAPS2_TEXTUREMANAGE;
	}
	else
	{
		ddsd.dwFlags  = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT;
		if( lpD3DDeviceDesc->bIsHardware )
		{
			ddsd.ddsCaps.dwCaps  = DDSCAPS_TEXTURE;
			ddsd.ddsCaps.dwCaps2 = DDSCAPS2_TEXTUREMANAGE;
		}
		else
			ddsd.ddsCaps.dwCaps = DDSCAPS_SYSTEMMEMORY | DDSCAPS_TEXTURE;
	}
	
	//
	LPDIRECTDRAWSURFACE4 lpDDSurface;
	hresult = lpDD->CreateSurface( &ddsd, &lpDDSurface, NULL );
	if( hresult != DD_OK )
	{
		// 256x256 초과 텍스쳐 이라면.
		if( g_DibLastReadWidth > 256 || g_DibLastReadHeight > 256 )
		{
			if( lpD3DDeviceDesc->bIsHardware )
			{
				ddsd.ddsCaps.dwCaps  = DDSCAPS_SYSTEMMEMORY | DDSCAPS_TEXTURE;
				ddsd.ddsCaps.dwCaps2 = 0;

				hresult = lpDD->CreateSurface( &ddsd, &lpDDSurface, NULL );
				if( hresult == DD_OK )
					rTexHalf = TRUE;
			}
		}

		if( hresult != DD_OK )
            return NULL;
	}

	if( DrawSurfaceFromDib(lpDDSurface, lpDib) == FALSE )
	{
		lpDDSurface->Release();
		return NULL;
	}

	//
	if( MipMapCount && rTexHalf == FALSE )
	{
		DDSCAPS2 ddsCaps;
		ZeroMemory( &ddsCaps, sizeof(DDSCAPS2) );
		ddsCaps.dwCaps = DDSCAPS_TEXTURE | DDSCAPS_MIPMAP;

		LPDIRECTDRAWSURFACE4 lpDDSurface2 = lpDDSurface;
		int					 width2		  = g_DibLastReadWidth;
		int					 height2	  = g_DibLastReadHeight;

		for( int i=1; i < MipMapCount; i++ )
		{
			LPDIRECTDRAWSURFACE4 lpDDNextLevel = NULL;
			if( FAILED( lpDDSurface2->GetAttachedSurface(&ddsCaps, &lpDDNextLevel) ) )
			{
				lpDDSurface->Release();
				return NULL;
			}

			width2  >>= 1;
			height2 >>= 1;

			if( new_LoadDib( MipMapFileBuff[i] ) == NULL )
			{
				lpDDNextLevel->Release();
				lpDDSurface->Release();
				return NULL;
			}

			if( width2 != GetDibWidth(lpDib) || height2 != GetDibHeight(lpDib) )
			{
				lpDDNextLevel->Release();
				lpDDSurface->Release();
				return NULL;
			}

			if( DrawSurfaceFromDib(lpDDNextLevel, lpDib) == FALSE )
			{
				lpDDNextLevel->Release();
				lpDDSurface->Release();
				return NULL;
			}

			lpDDSurface2 = lpDDNextLevel;
			lpDDNextLevel->Release();
		}
	}

	//
	return lpDDSurface;
}


/////////////////////////////////////////////////////////////////////////////////////////
//
LPDIRECTDRAWSURFACE4 new_LoadTgaCreateTexSurface( char *Filename, int MipMap, int &rTexHalf )
{
	//
	if( new_LoadTga(Filename) == NULL )
		return NULL;

	//
	if( MipMap && MipMapModeCreate && g_DibLastReadWidth < 512 && g_DibLastReadHeight < 512 )
	{
		if( MipMapModeCreate == NOSQUARE_MODE || g_DibLastReadWidth == g_DibLastReadHeight )
		{
			int i, len = ::lstrlen( Filename );
			for( i=len-1; i >= 0; i-- )
			{
				if( Filename[i] == '.' )
				{
					::CopyMemory( MipMapFileBuff[0], Filename, i );
					break;
				}
			}

			// 프테 에서는밉맵 최대 3단계 까지만 사용 하기로 했음.
			for( i=1; i < MIPMAP_FILELIST; i++ )
			{
				::strcpy( MipMapFileBuff[i], MipMapFileBuff[0] );
				::strcat( MipMapFileBuff[i], MipMapFileListBuff[i] );
				::strcat( MipMapFileBuff[i], ".tga" );

				if( PathFileExists( MipMapFileBuff[i] ) )
					MipMapCount++;
				else
					break;
			}

			::strcat( MipMapFileBuff[0], ".tga" );
		}
	}

	//
	DDSURFACEDESC2 ddsd;
	ZeroMemory( &ddsd, sizeof(DDSURFACEDESC2) );

	memcpy( &ddsd.ddpfPixelFormat, &ddTextureFormat, sizeof(DDPIXELFORMAT) );
	ddsd.dwSize	  = sizeof(DDSURFACEDESC2);
	ddsd.dwWidth  = g_DibLastReadWidth;
	ddsd.dwHeight = g_DibLastReadHeight;

	if( MipMapCount )
	{
		MipMapCount++;
		if( MipMapCreateTextureCheck(g_DibLastReadWidth, g_DibLastReadHeight, MipMapCount) <= 0 )
			return NULL;

		ddsd.dwFlags         = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT | DDSD_PIXELFORMAT | DDSD_MIPMAPCOUNT;		
		ddsd.dwMipMapCount   = MipMapCount;
		ddsd.ddsCaps.dwCaps  = DDSCAPS_MIPMAP | DDSCAPS_COMPLEX | DDSCAPS_TEXTURE;
		ddsd.ddsCaps.dwCaps2 = DDSCAPS2_TEXTUREMANAGE;
	}
	else
	{
		ddsd.dwFlags  = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT;
		if( lpD3DDeviceDesc->bIsHardware )
		{
			ddsd.ddsCaps.dwCaps  = DDSCAPS_TEXTURE;
			ddsd.ddsCaps.dwCaps2 = DDSCAPS2_TEXTUREMANAGE;
		}
		else
			ddsd.ddsCaps.dwCaps = DDSCAPS_SYSTEMMEMORY | DDSCAPS_TEXTURE;
	}

	//
	LPDIRECTDRAWSURFACE4 lpDDSurface;
    hresult = lpDD->CreateSurface( &ddsd, &lpDDSurface, NULL );
	if( hresult != DD_OK )
	{
        // 256x256 초과 텍스쳐 이라면.
		if( g_DibLastReadWidth > 256 || g_DibLastReadHeight > 256 )
		{
			if( lpD3DDeviceDesc->bIsHardware )
			{
				ddsd.ddsCaps.dwCaps  = DDSCAPS_SYSTEMMEMORY | DDSCAPS_TEXTURE;
				ddsd.ddsCaps.dwCaps2 = 0;

				hresult = lpDD->CreateSurface( &ddsd, &lpDDSurface, NULL );
				if( hresult == DD_OK )
					rTexHalf = TRUE;
			}
		}

		if( hresult != DD_OK )
            return NULL;
	}

	//
	if( New_DrawSurfaceFromTga(lpDDSurface) == FALSE )
	{
		lpDDSurface->Release();
		return NULL;
	}

	//
	if( MipMapCount && rTexHalf == FALSE )
	{
		DDSCAPS2 ddsCaps;
		ZeroMemory( &ddsCaps, sizeof(DDSCAPS2) );
		ddsCaps.dwCaps = DDSCAPS_TEXTURE | DDSCAPS_MIPMAP;

		LPDIRECTDRAWSURFACE4 lpDDSurface2 = lpDDSurface;
		
		int width2, height2, OrgWidth, OrgHeight;
		width2  = OrgWidth  = g_DibLastReadWidth;
        height2	= OrgHeight = g_DibLastReadHeight;

		for( int i=1; i < MipMapCount; i++ )
		{
			LPDIRECTDRAWSURFACE4 lpDDNextLevel = NULL;
			if( FAILED( lpDDSurface2->GetAttachedSurface(&ddsCaps, &lpDDNextLevel) ) )
			{
				lpDDSurface->Release();
				return NULL;
			}

			width2  >>= 1;
			height2 >>= 1;

			if( new_LoadTga( MipMapFileBuff[i] ) == NULL )
			{
				lpDDNextLevel->Release();
				lpDDSurface->Release();
				return NULL;
			}

			if( width2 != g_DibLastReadWidth || height2 != g_DibLastReadHeight )
			{
				lpDDNextLevel->Release();
				lpDDSurface->Release();
				return NULL;
			}

			if( New_DrawSurfaceFromTga(lpDDNextLevel) == FALSE )
			{
				lpDDNextLevel->Release();
				lpDDSurface->Release();
				return NULL;
			}

			lpDDSurface2 = lpDDNextLevel;
			lpDDNextLevel->Release();
		}

		g_DibLastReadWidth  = OrgWidth;
		g_DibLastReadHeight = OrgHeight;
	}

	//
	return lpDDSurface;
}


/////////////////////////////////////////////////////////////////////////////////////////
//
LPDIRECTDRAWSURFACE4 new_LoadBmpSurfaceOffscreen( char *Filename )
{
	//
	if( new_LoadDib(Filename) == NULL )
		return NULL;
	
	//
	BYTE *lpDib = g_DibImageDateBuff;
	g_DibLastReadWidth  = GetDibWidth(lpDib);
	g_DibLastReadHeight = GetDibHeight(lpDib);

	DDSURFACEDESC2 ddsd;
	ZeroMemory( &ddsd, sizeof(DDSURFACEDESC2) );

	ddsd.dwSize			= sizeof(DDSURFACEDESC2);
	ddsd.dwFlags		= DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;
	ddsd.dwWidth		= g_DibLastReadWidth;
	ddsd.dwHeight		= g_DibLastReadHeight;
	ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;

	LPDIRECTDRAWSURFACE4 lpDDSurface;
	hresult = lpDD->CreateSurface( &ddsd, &lpDDSurface, NULL );
	if( hresult != DD_OK )
        return NULL;

	if( DrawSurfaceFromDib(lpDDSurface, lpDib) == FALSE )
	{
		lpDDSurface->Release();
		return NULL;
	}

	DDCOLORKEY ColKey;
	ColKey.dwColorSpaceLowValue=RGB(0,0,0); 
    ColKey.dwColorSpaceHighValue=RGB(255,255,255);  
	lpDDSurface->SetColorKey( DDCKEY_SRCBLT, &ColKey );

	return lpDDSurface;
}
//######################################################################################


