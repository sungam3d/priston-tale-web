#include "smlib3d\\smd3d.h"
#include "Drawsub.h"
#include "sinbaram\\sinlinkheader.h"

extern int WinSizeX;
extern int WinSizeY;

int	dsDrawOffsetArray = 0;
int	dsDrawOffset_X = 0;
int	dsDrawOffset_Y = 0;

int dsTextLineOut( HDC hdc , int x, int y, LPCTSTR lpszString , int nCount )
{
	if ( dsDrawOffset_X || dsDrawOffset_Y ) {
		if ( dsDrawOffsetArray&dsARRAY_RIGHT )
			x += dsDrawOffset_X;

		if ( dsDrawOffsetArray&dsARRAY_BOTTOM )
			y += dsDrawOffset_Y;
	}

	return TextOut( hdc , x, y, lpszString , nCount );
}

int dsGetDrawOffset( int *sx , int *sy )
{

	if ( dsDrawOffset_X==0 && dsDrawOffset_Y==0 ) return TRUE;

	if ( dsDrawOffsetArray&dsARRAY_RIGHT )
		*sx += dsDrawOffset_X;

	if ( dsDrawOffsetArray&dsARRAY_BOTTOM )
		*sy += dsDrawOffset_Y;

	return TRUE;
}

static POINT BackCursorPos;

//메뉴커서 시물레이션
int dsMenuCursorPos( POINT *lpCursorPos , int Mode )
{
	int x,y;

	if ( Mode ) {
		BackCursorPos.x = lpCursorPos->x;
		BackCursorPos.y = lpCursorPos->y;

		// the same formula as the matching draw offset (Winmain.cpp dsDrawOffset_X/Y): this
		// turns a real cursor position back into "design space" for menu hit-testing, and
		// must move the cursor by exactly as much as the menu itself was actually drawn by
		{
		int uiX=(WinSizeX>1280?1280:WinSizeX), uiY=(WinSizeY>1024?1024:WinSizeY);
		x = (WinSizeX-uiX)/2 + (uiX-800);
		y = (WinSizeY-uiY)/2 + (uiY-600);
		}

		if ( x>0 ) {
			if ( lpCursorPos->x>=600 && lpCursorPos->y<=y+420 && ( !sSinWarpGate.ShowFlag || Mode!=1 ) ) {
				//수정---------------------------------------------------------
				if(MouseItem.Flag){
					lpCursorPos->x = 0;
					return TRUE;
				}
				//-------------------------------------------------------------
				//동료퀘스트창 보정( 워프게이트메뉴 제외 )
				lpCursorPos->x -= x;
				lpCursorPos->y -= y;
				if ( lpCursorPos->x<600) lpCursorPos->x = 0;
			}
			else {
				if ( lpCursorPos->y>400) {
					lpCursorPos->y -= y;
					if ( lpCursorPos->y<420 )lpCursorPos->y = 0;
				}
			}

		}
	}
	else {
		lpCursorPos->x = BackCursorPos.x;
		lpCursorPos->y = BackCursorPos.y;
	}

	return TRUE;
}
//211.155.2.203



//색깔있는 상자 그린다
int dsDrawColorBoxFloat( DWORD Color ,float x, float y, float sizeX, float sizeY )
{


	D3DTLVERTEX tlVertex[16];
	int cnt;
//	int size;

	for(cnt=0;cnt<4;cnt++) {
		tlVertex[cnt].rhw = 1;
		tlVertex[cnt].color = Color;
		tlVertex[cnt].specular  =  RGBA_MAKE( 0,0,0,0 );
		tlVertex[cnt].sz = 0;
	}

	tlVertex[0].sx = x;
	tlVertex[0].sy = y;
	tlVertex[0].tu = 0;
	tlVertex[0].tv = 0;

	tlVertex[1].sx = x;
	tlVertex[1].sy = (y+sizeY);
	tlVertex[1].tu = 0;
	tlVertex[1].tv = 1;

	tlVertex[2].sx = (x+sizeX);
	tlVertex[2].sy = y;
	tlVertex[2].tu = 1;
	tlVertex[2].tv = 0;

	tlVertex[3].sx = (x+sizeX);
	tlVertex[3].sy = (y+sizeY);
	tlVertex[3].tu = 1;
	tlVertex[3].tv = 1;

	lpD3DDevice->SetTexture( 0, 0 );

	lpD3DDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP ,
							D3DFVF_TLVERTEX  ,
							tlVertex,
							4,
							NULL);

	return TRUE;
}

int dsDrawColorBox( DWORD Color ,int x, int y, int sizeX, int sizeY )
{

	dsGetDrawOffset( &x , &y );

	return dsDrawColorBoxFloat( Color , (float)x, (float)y, (float)sizeX , (float)sizeY );
}



//그림 출력
int dsDrawTexImage( int Mat , int x, int y, int w, int h , DWORD Transp , int specular)
{
	dsGetDrawOffset( &x , &y );

	return dsDrawTexImageFloat ( Mat , (float)x,(float)y,(float)w,(float)h, Transp , specular );

}


//그림 출력
//int dsDrawTexImage( int Mat , int x, int y, float w, float h , float tx, float ty, float tw, float th, int Transp )
int dsDrawTexImage( int Mat, int x, int y, int w, int h, float surW, float surH, float tLeft, float tTop, float tRight, float tBottom, int Transp)
{

	return dsDrawTexImageFloat( Mat, (float)x, (float)y, (float)w, (float)h, surW, surH, tLeft, tTop, tRight, tBottom, Transp);
}

int dsDrawTexImageFloat( int Mat, float x, float y, float w, float h, float surW, float surH, float tLeft, float tTop, float tRight, float tBottom, int Transp)
{
	
	D3DTLVERTEX tlVertex[4];
	int cnt;

	for(cnt=0;cnt<4;cnt++)
	{
		tlVertex[cnt].rhw = 1;
		tlVertex[cnt].color	= RGBA_MAKE( 255,255,255,Transp );
		tlVertex[cnt].specular  =  RGBA_MAKE( 0,0,0,0 );
		tlVertex[cnt].sz = 0;
	}
	
	
	tlVertex[0].sx = x;
	tlVertex[0].sy = y;
	tlVertex[0].tu = 1*(tLeft/surW);
	tlVertex[0].tv = 1*(tTop/surH);

	tlVertex[1].sx = x;
	tlVertex[1].sy = (y+h);
	tlVertex[1].tu = 1*(tLeft/surW);
	

	tlVertex[1].tv = 1*((tBottom+1)/surH);

	tlVertex[2].sx = (x+w);
	tlVertex[2].sy = y;

	tlVertex[2].tu = 1*((tRight+1)/surW);
	tlVertex[2].tv = 1*(tTop/surH);

	tlVertex[3].sx = (x+w);
	tlVertex[3].sy = (y+h);
	tlVertex[3].tu = 1*((tRight+1)/surW);
	tlVertex[3].tv = 1*((tBottom+1)/surH);
	
	//뒤집힌면 그리기
	//lpD3DDevice->SetRenderState( D3DRENDERSTATE_CULLMODE , D3DCULL_NONE );
	if(lpD3DDevice != NULL)
	{
		
		// Turn on Z-buffering

		lpD3DDevice->SetRenderState( D3DRENDERSTATE_ZENABLE, TRUE );
		lpD3DDevice->SetRenderState( D3DRENDERSTATE_ZWRITEENABLE, TRUE );
		lpD3DDevice->SetRenderState( D3DRENDERSTATE_ZFUNC,   D3DCMP_LESSEQUAL );
		


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
		
		smRender.AlphaTestDepth = 60;
		

		lpD3DDevice->SetTextureStageState( 0, D3DTSS_MINFILTER, D3DTFN_POINT  );
        lpD3DDevice->SetTextureStageState( 0, D3DTSS_MAGFILTER, D3DTFG_POINT  );

		//lpD3DDevice->SetRenderState( D3DRENDERSTATE_DESTBLEND, D3DBLEND_ONE);
		//lpD3DDevice->SetRenderState( D3DRENDERSTATE_SRCBLEND, D3DBLEND_ONE);
		

		if(smMaterial[Mat].smTexture[0] != NULL)
		{
			if(smMaterial[Mat].smTexture[0]->lpD3DTexture != NULL)
			{
				if(FAILED(lpD3DDevice->SetTexture( 0, smMaterial[Mat].smTexture[0]->lpD3DTexture )))
				{
					OutputDebugString("SetTexture Error");
					return FALSE;
				}

				
				if(FAILED(lpD3DDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP ,
								D3DFVF_TLVERTEX  ,
								tlVertex,
								4,

								NULL)))
				{
					OutputDebugString("DrawPrimitive Error");
					return FALSE;
				}
			}
		}
		
		lpD3DDevice->SetTextureStageState( cnt, D3DTSS_MINFILTER, D3DTFN_LINEAR  );
        lpD3DDevice->SetTextureStageState( cnt, D3DTSS_MAGFILTER, D3DTFG_LINEAR  );
	}	
	

	return TRUE;
}





//int Transp
//그림 출력
int dsDrawTexImageFloat( int Mat , float x, float y, float w, float h , DWORD dwColor , int specular )
{
	int clipLeft;
	int clipTop;
	int clipRight;
	int clipBottom;
	DWORD dwClr;
	
	clipLeft = 0;
	clipTop = 0;
	clipRight = WinSizeX;
	clipBottom = WinSizeY;
	
	//Clipping 영역을 완전히 벗어나면...
	if ((x >= clipRight) || (y >= clipBottom) || ((x + w) <= clipLeft) || ((y+h) <= clipTop))
		return FALSE;
	
	//원래 사각 영역의 크기는 0, 0, 127, 127 이지만 이렇게 찍을 경우. 한 픽셀이 빠져서 0, 0, 128, 128로 보정한다.
	float x1 = x;
	float y1 = y;
	float x2 = (x+w);
	float y2 = (y+h);
	

	//x1, y1, x2, y2 는 클립된 사각영역이다..
	D3DTLVERTEX tlVertex[4];
	int cnt;

	if ( dwColor<256 )
		dwClr = RGBA_MAKE( 255,255,255,dwColor );
	else
		dwClr = dwColor;

	for(cnt=0;cnt<4;cnt++)
	{
		tlVertex[cnt].rhw = 1.f;
		tlVertex[cnt].color	= dwClr;
		tlVertex[cnt].specular  =  RGBA_MAKE( specular,specular,specular,0 );
		tlVertex[cnt].sz = 0;
	}
	
	tlVertex[0].sx = x1;
	tlVertex[0].sy = y1;
	tlVertex[0].tu = 0.0f;
	tlVertex[0].tv = 0.0f;
	
	tlVertex[1].sx = x1;
	tlVertex[1].sy = y2;
	tlVertex[1].tu = 0.0f;
	tlVertex[1].tv = 1.0f;
	
	tlVertex[2].sx = x2;
	tlVertex[2].sy = y1;
	tlVertex[2].tu = 1.0f;
	tlVertex[2].tv = 0.0f;
	
	tlVertex[3].sx = x2;
	tlVertex[3].sy = y2;
	tlVertex[3].tu = 1.0f;
	tlVertex[3].tv = 1.0f;
	

	//뒤집힌면 그리기
	//lpD3DDevice->SetRenderState( D3DRENDERSTATE_CULLMODE , D3DCULL_NONE );
	if(lpD3DDevice != NULL)
	{
		  // Turn on Z-buffering
		lpD3DDevice->SetRenderState( D3DRENDERSTATE_CULLMODE , D3DCULL_NONE );
		lpD3DDevice->SetRenderState( D3DRENDERSTATE_ZENABLE, TRUE );
		lpD3DDevice->SetRenderState( D3DRENDERSTATE_ZWRITEENABLE, TRUE );
		//lpD3DDevice->SetRenderState( D3DRENDERSTATE_ZFUNC,   D3DCMP_LESSEQUAL );
		//	lpD3DDevice->SetRenderState( D3DRENDERSTATE_ZENABLE, D3DZB_USEW );//TRUE );


		// null out the texture handle
		lpD3DDevice->SetRenderState( D3DRENDERSTATE_TEXTUREHANDLE, 0 );

		// turn on dithering
		//D3DDevice->SetRenderState( D3DRENDERSTATE_DITHERENABLE, TRUE );

		lpD3DDevice->SetRenderState( D3DRENDERSTATE_TEXTUREADDRESS, D3DTADDRESS_WRAP);
		//lpD3DDevice->SetRenderState( D3DRENDERSTATE_TEXTUREPERSPECTIVE, TRUE ); 

		// D3DFILTER_LINEAR
		//lpD3DDevice->SetRenderState( D3DRENDERSTATE_TEXTUREMAG, D3DFILTER_LINEAR );
		//lpD3DDevice->SetRenderState( D3DRENDERSTATE_TEXTUREMIN, D3DFILTER_LINEAR );
 
		lpD3DDevice->SetRenderState( D3DRENDERSTATE_SPECULARENABLE, TRUE );
		lpD3DDevice->SetRenderState( D3DRENDERSTATE_COLORKEYENABLE, FALSE );

		lpD3DDevice->SetRenderState( D3DRENDERSTATE_TEXTUREMAPBLEND, D3DTBLEND_MODULATE );

		lpD3DDevice->SetRenderState( D3DRENDERSTATE_ALPHABLENDENABLE, TRUE );
		lpD3DDevice->SetRenderState( D3DRENDERSTATE_DESTBLEND, D3DBLEND_INVSRCALPHA );
		lpD3DDevice->SetRenderState( D3DRENDERSTATE_SRCBLEND, D3DBLEND_SRCALPHA );

		smRender.AlphaTestDepth = 60;

		lpD3DDevice->SetTextureStageState( 0, D3DTSS_MINFILTER, D3DTFN_POINT  );
        lpD3DDevice->SetTextureStageState( 0, D3DTSS_MAGFILTER, D3DTFG_POINT  );

		//lpD3DDevice->SetRenderState( D3DRENDERSTATE_TEXTUREMIN, D3DTFN_POINT );
		//lpD3DDevice->SetRenderState( D3DRENDERSTATE_TEXTUREMAG, D3DTFN_POINT );
		

		if(smMaterial[Mat].smTexture[0] != NULL)
		{
			if(smMaterial[Mat].smTexture[0]->lpD3DTexture != NULL)
			{
				if(FAILED(lpD3DDevice->SetTexture( 0, smMaterial[Mat].smTexture[0]->lpD3DTexture )))
				{
					OutputDebugString("SetTexture Error");
					return FALSE;
				}

				
				if(FAILED(lpD3DDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP ,
								D3DFVF_TLVERTEX  ,
								tlVertex,
								4,
								NULL)))
				{
					OutputDebugString("DrawPrimitive Error");
					return FALSE;
				}
			}
		}
		
		lpD3DDevice->SetTextureStageState( cnt, D3DTSS_MINFILTER, D3DTFN_LINEAR  );
        lpD3DDevice->SetTextureStageState( cnt, D3DTSS_MAGFILTER, D3DTFG_LINEAR  );
		//D3DDevice->SetRenderState( D3DRENDERSTATE_TEXTUREADDRESS, D3DTADDRESS_WRAP);
		//lpD3DDevice->SetRenderState( D3DRENDERSTATE_TEXTUREMIN, D3DFILTER_LINEAR );
		//lpD3DDevice->SetRenderState( D3DRENDERSTATE_TEXTUREMAG, D3DFILTER_LINEAR );
	}	
	//lpD3DDevice->SetRenderState( D3DRENDERSTATE_CULLMODE , D3DCULL_CW );

	return TRUE;
}

int dsDrawTexImageFloatColor( int Mat, float x, float y, float w, float h, float surW, float surH, float tLeft, float tTop, float tRight, float tBottom, DWORD dwColor )
{
	
	D3DTLVERTEX tlVertex[4];
	int cnt;

	for(cnt=0;cnt<4;cnt++)
	{
		tlVertex[cnt].rhw = 1;
		tlVertex[cnt].color	= dwColor;
		tlVertex[cnt].specular  =  RGBA_MAKE( 0,0,0,0 );
		tlVertex[cnt].sz = 0;
	}
	
	
	tlVertex[0].sx = x;
	tlVertex[0].sy = y;
	tlVertex[0].tu = 1*(tLeft/surW);
	tlVertex[0].tv = 1*(tTop/surH);

	tlVertex[1].sx = x;
	tlVertex[1].sy = (y+h);
	tlVertex[1].tu = 1*(tLeft/surW);
	

	tlVertex[1].tv = 1*((tBottom+1)/surH);

	tlVertex[2].sx = (x+w);
	tlVertex[2].sy = y;

	tlVertex[2].tu = 1*((tRight+1)/surW);
	tlVertex[2].tv = 1*(tTop/surH);

	tlVertex[3].sx = (x+w);
	tlVertex[3].sy = (y+h);
	tlVertex[3].tu = 1*((tRight+1)/surW);
	tlVertex[3].tv = 1*((tBottom+1)/surH);
	
	//뒤집힌면 그리기
	//lpD3DDevice->SetRenderState( D3DRENDERSTATE_CULLMODE , D3DCULL_NONE );
	if(lpD3DDevice != NULL)
	{
		
		// Turn on Z-buffering

		lpD3DDevice->SetRenderState( D3DRENDERSTATE_ZENABLE, TRUE );
		lpD3DDevice->SetRenderState( D3DRENDERSTATE_ZWRITEENABLE, TRUE );
		lpD3DDevice->SetRenderState( D3DRENDERSTATE_ZFUNC,   D3DCMP_LESSEQUAL );
		


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
		
		smRender.AlphaTestDepth = 60;
		

		lpD3DDevice->SetTextureStageState( 0, D3DTSS_MINFILTER, D3DTFN_POINT  );
        lpD3DDevice->SetTextureStageState( 0, D3DTSS_MAGFILTER, D3DTFG_POINT  );

		//lpD3DDevice->SetRenderState( D3DRENDERSTATE_DESTBLEND, D3DBLEND_ONE);
		//lpD3DDevice->SetRenderState( D3DRENDERSTATE_SRCBLEND, D3DBLEND_ONE);
		

		if(smMaterial[Mat].smTexture[0] != NULL)
		{
			if(smMaterial[Mat].smTexture[0]->lpD3DTexture != NULL)
			{
				if(FAILED(lpD3DDevice->SetTexture( 0, smMaterial[Mat].smTexture[0]->lpD3DTexture )))
				{
					OutputDebugString("SetTexture Error");
					return FALSE;
				}

				
				if(FAILED(lpD3DDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP ,
								D3DFVF_TLVERTEX  ,
								tlVertex,
								4,

								NULL)))
				{
					OutputDebugString("DrawPrimitive Error");
					return FALSE;
				}
			}
		}
		
		lpD3DDevice->SetTextureStageState( cnt, D3DTSS_MINFILTER, D3DTFN_LINEAR  );
        lpD3DDevice->SetTextureStageState( cnt, D3DTSS_MAGFILTER, D3DTFG_LINEAR  );
	}	
	

	return TRUE;
}
