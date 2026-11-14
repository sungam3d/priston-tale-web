#include "..\\particle.h"
#include "..\\HoBaram\\HoLinkHeader.h"
#include "..\\sinbaram\\sinLinkHeader.h"

#include "..\\HoBaram\\HoMinMax.h"
#include "..\\HoBaram\\HoNewParticle.h"
#include "..\\HoBaram\\HoNewParticleMgr.h"

#include "hoAssaParticleEffect.h"
#include "AssaParticle.h"
#include "..\\field.h"
//#include "..\\HoBaram\\NewEffect\\HoEffect\\HoEffectMain.h"
#include "..\\HoBaram\\NewEffect\\HoEffect\\HoEffectManager.h"
#include "..\\HoBaram\\NewEffect\\HoEffect\\HoNewEffectFunction.h"
//#define Making_SKILL_TORNADO3


HoEffectObject::HoEffectObject()
{
	//TermFrame = 0;
	WorldX = 0;
	WorldY = 0;
	WorldZ = 0;
	ClassID = CLASS_NONE;
	SkillCode = 0;

	LocalX = 0;
	LocalY = 0;
	LocalZ = 0;

	Parent = NULL;
}

HoEffectObject::~HoEffectObject()
{

}

int HoEffectObject::Draw(int x, int y, int z, int ax, int ay, int az)
{

	return TRUE;
}

int HoEffectObject::Main()
{

	return TRUE;
}

HoEtc2dPrimitive::HoEtc2dPrimitive()
{
	
	Init();	
}

HoEtc2dPrimitive::~HoEtc2dPrimitive()
{

}

int HoEtc2dPrimitive::Load()
{
	AnimationData.ImageLoad("Effect\\Etc\\Flame.ini");
	AnimationData.SequenceLoad("Effect\\Etc\\Flame.ini");


	return TRUE;
}

static int FlameDownPos=0;
int HoEtc2dPrimitive::Init()
{
	AnimationData.Init();
	StartFlag = FALSE;
	return TRUE;
}

int HoEtc2dPrimitive::StartDown()
{
	WorkState = STATE_DOWN;
	return TRUE;
}

int HoEtc2dPrimitive::StartUp(int x, int y)
{
	//Init();
	AniDataIndex = 0;
	StartFlag = FALSE;
	WorkState = STATE_NONE;
	Velocity = -25;
	FlameDownPos = 0;

	VertexCount = 0;
	StartFlag = TRUE;
	WorkState = STATE_DOWN;

	for(int count=0;count<4;count++)
	{
		TLVertex[count].rhw = 1;
		TLVertex[count].color	= RGBA_MAKE( 255,255,255,255);
		TLVertex[count].specular  =  RGBA_MAKE( 0,0,0,0 );
		TLVertex[count].sz = 0;
	}

	TLVertex[0].sx = 0;
	TLVertex[0].sy = 0;
	TLVertex[0].sz = 0;
	TLVertex[0].tu = 0;
	TLVertex[0].tv = 0;

	TLVertex[1].sx = 0;
	TLVertex[1].sy = 0+250;
	TLVertex[1].sz = 0;
	TLVertex[1].tu = 0;
	TLVertex[1].tv = 1;

	TLVertex[2].sx = 0+800;
	TLVertex[2].sy = 0;
	TLVertex[2].sz = 0;
	TLVertex[2].tu = 2;
	TLVertex[2].tv = 0;

	TLVertex[3].sx = 0+800;
	TLVertex[3].sy = 0+250;
	TLVertex[3].sz = 0;
	TLVertex[3].tu = 2;
	TLVertex[3].tv = 1;

	VertexCount += 4;

	LocalX = (float)x;
	LocalY = (float)y;
	return TRUE;
}


int HoEtc2dPrimitive::Main()
{
	if(StartFlag)
	{
		//Animation 한번...
		if(CurrentFrame < AnimDataMgr.SequenceDataBuffer[AniDataIndex].AnimFrameCount)
		{
			if(TimeCount >= AnimDataMgr.SequenceDataBuffer[AniDataIndex].AnimFrame[CurrentFrame].Delay)
			{
				TimeCount = 0;
				if(CurrentFrame < AnimDataMgr.SequenceDataBuffer[AniDataIndex].AnimFrameCount-1)
					CurrentFrame++;
				
				if(CurrentFrame == AnimDataMgr.SequenceDataBuffer[AniDataIndex].AnimFrameCount-1)
					CurrentFrame=0;
				
				if(WorkState == STATE_UP)
				{
					LocalY -= 10;	
					if(LocalY <= 400)
						LocalY = 400;
				}
				else if(WorkState == STATE_DOWN)
				{
					if(LocalY >= 430&& FlameDownPos == 0)
					{
						LocalY -= 85;
						FlameDownPos = 1;
					}
					else if(LocalY >= 510&& FlameDownPos == 1)
					{
						LocalY -= 60;
						FlameDownPos = 2;
					}
					else if(LocalY >= 590&& FlameDownPos == 2)
					{
						LocalY -= 30;
						FlameDownPos = 3;
					}
					LocalY -= Velocity;
					if(LocalY >= 600)
					{
						AniDataIndex = 0;
						StartFlag = FALSE;
						WorkState = STATE_NONE;
						Velocity = -8;
						FlameDownPos = 0;

						//VertexCount = 0;
						//StartFlag = TRUE;
						//WorkState = STATE_DOWN;
					}
				}
			}
			TimeCount++;
		}
	}
	return TRUE;
}

int HoEtc2dPrimitive::Draw()
{
	if(StartFlag)
	{
		//현재 찍을 Frame을 세팅한다..
		int mat = AnimationData.ImageDataBuffer[AnimationData.SequenceDataBuffer[AniDataIndex].DataFileIndex].GetMaterialNum();
		if(FAILED(lpD3DDevice->SetTexture( 0, smMaterial[mat].smAnimTexture[AnimationData.SequenceDataBuffer[AniDataIndex].AnimFrame[CurrentFrame].ImageNum]->lpD3DTexture )))
		{
			OutputDebugString("SetTexture Error");
			return FALSE;
		}
		
		D3DTLVERTEX tlVertex[MAX_2DVERTEX];
		memcpy(tlVertex, TLVertex, sizeof(D3DTLVERTEX)*VertexCount);
		
		for(int index = 0; index < 4; index++)
		{
			tlVertex[index].sx += LocalX;
			tlVertex[index].sy += LocalY;
			
			//Clipping
			if(tlVertex[index].sy >= 600)
			{
				if(index == 1)
					tlVertex[1].tv = 1*(250-(tlVertex[1].sy-600+1))/250; 
				if(index == 3)
					tlVertex[3].tv = 1*(250-(tlVertex[3].sy-600+1))/250;

				tlVertex[index].sy = tlVertex[index].sy - ((tlVertex[index].sy-600)+1);
			}
		}
		
		lpD3DDevice->SetRenderState( D3DRENDERSTATE_ALPHABLENDENABLE, TRUE );
		lpD3DDevice->SetRenderState( D3DRENDERSTATE_DESTBLEND, D3DBLEND_INVSRCCOLOR  );
		lpD3DDevice->SetRenderState( D3DRENDERSTATE_SRCBLEND, D3DBLEND_SRCCOLOR  );
		
		lpD3DDevice->SetTextureStageState( 0, D3DTSS_MINFILTER, D3DFILTER_LINEAR  );
        lpD3DDevice->SetTextureStageState( 0, D3DTSS_MAGFILTER, D3DFILTER_LINEAR  );
		//lpD3DDevice->SetTextureStageState( 0, D3DTSS_ADDRESS, D3DTADDRESS_MIRROR  );

		lpD3DDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, FALSE);
			
		lpD3DDevice->BeginScene();
		if(FAILED(lpD3DDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP,
						D3DFVF_TLVERTEX  ,
						tlVertex,
						VertexCount,
						NULL)))
		{
			OutputDebugString("DrawPrimitive Error");
			return FALSE;
		}
		
		lpD3DDevice->EndScene();
		lpD3DDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, TRUE);
		//lpD3DDevice->SetTextureStageState( 0, D3DTSS_ADDRESS, D3DTADDRESS_WRAP  );
	}
	return TRUE;
}

HoPrimitivePolygon::HoPrimitivePolygon()
{
	SumCount = 0;
	Physics = NULL;
	Init();
}

HoPrimitivePolygon::~HoPrimitivePolygon()
{
	Init();
}

void HoPrimitivePolygon::Init()
{
	LocalX = 0;
	LocalY = 0;
	LocalZ = 0;
	TimeCount = 0;
	
	AniDataIndex = -1;
	AniType	= ANI_NONE;
	StartFlag = FALSE;
	CurrentFrame = 0;

	memset(Vertex, 0, sizeof(Vertex));
	VertexCount = 0;
	memset(Face, 0, sizeof(Face));
	FaceCount = 0;
	memset(TexLink, 0, sizeof(TexLink));
	TexLinkCount = 0;
	
	CurrentBlendValue = 255;
	BlendStep = 0;
//	TermFrame = 0;

	if(Physics != NULL)
	{
		delete Physics;
		Physics = NULL;
	}
}

int	HoPrimitivePolygon::Main()
{
	if(StartFlag == FALSE)
		return FALSE;

	//Animation 한번...
	if(CurrentFrame < AnimDataMgr.SequenceDataBuffer[AniDataIndex].AnimFrameCount)
	{
		if(TimeCount >= AnimDataMgr.SequenceDataBuffer[AniDataIndex].AnimFrame[CurrentFrame].Delay)
		{
			TimeCount = 0;
			bool flag = false;
			//if(TermFrame <= 0)
			//{
				if(AniType == ANI_ONE) //Animation 한번..
				{
					if(CurrentFrame < AnimDataMgr.SequenceDataBuffer[AniDataIndex].AnimFrameCount-1)
					{
						CurrentFrame++;
						flag = true;
					}
			
					if(flag==false && CurrentFrame == AnimDataMgr.SequenceDataBuffer[AniDataIndex].AnimFrameCount-1)
						Init();
				}
				else if(AniType == ANI_LOOP) //Animation 루프...
				{
					CurrentFrame = 0;
					
					if(CurrentFrame >= AnimDataMgr.SequenceDataBuffer[AniDataIndex].AnimFrameCount-1)
						CurrentFrame = 0;
					else
						CurrentFrame++;
					
				}
				
				if(AniDataIndex != -1)
				{
					//Blend 값 설정...
					if(CurrentFrame <= AnimDataMgr.SequenceDataBuffer[AniDataIndex].AnimFrameCount-1)
					{
						BlendStep = (AnimDataMgr.SequenceDataBuffer[AniDataIndex].AnimFrame[CurrentFrame].AlphaValue-CurrentBlendValue)/
							AnimDataMgr.SequenceDataBuffer[AniDataIndex].AnimFrame[CurrentFrame].Delay;
					}
				}
			}
		//}
		CurrentBlendValue = CurrentBlendValue+BlendStep;
		TimeCount++;
	
		if(Physics != NULL)
		{
			if(Physics->GetState())
			{
				Physics->Main();
				
				hoPOINT3D vector = Physics->GetVector();
				float xv = vector.x;
				float yv = vector.y;
				float zv = vector.z;

				LocalX += xv;
				LocalY += yv;
				LocalZ += zv;
			}
			else
			{
				AniType = ANI_ONE;
			}
		}
	}
	return TRUE;
}

int HoPrimitivePolygon::Draw(int x, int y, int z, int ax, int ay, int az)
{
	if(StartFlag == FALSE)
		return FALSE;
	
	smRender.Init();
	smMaterialGroup->smMaterial[AnimDataMgr.ImageDataBuffer[AniDataIndex].GetMaterialNum()].TwoSide = TRUE;
	smRender.SetMaterialGroup(smMaterialGroup);
	
	//현재 찍을 Frame을 세팅한다..
	smMaterialGroup->SetMaterialAnimFrame( &smMaterial[AnimDataMgr.ImageDataBuffer[AnimDataMgr.SequenceDataBuffer[AniDataIndex].DataFileIndex].GetMaterialNum()], 
	AnimDataMgr.SequenceDataBuffer[AniDataIndex].AnimFrame[CurrentFrame].ImageNum);
	
	//Camera각도 입력.
	smRender.SetCameraPosi(x, y, z, ax, ay, az);
	CreatePathFace();
	smRender.InitStageMesh(Vertex, SumCount);
	
	for(int index = 0; index < FaceCount; index++)
		smRender.AddStageFace(&Face[index]);
		
	smRender.SetClipStates( SMCLIP_LEFT | SMCLIP_RIGHT | SMCLIP_TOP | SMCLIP_BOTTOM);
	smRender.ClipRendFace();
	smRender.GeomVertex2D();
	smRender.RenderD3D();
	SumCount++;
	
	return TRUE;
}

int HoPrimitivePolygon::StartPathTri(POINT3D &currentPos, POINT3D &destPos, char *iniName)
{
	if(StartFlag)
		return FALSE;
	
	AniDataIndex = AnimDataMgr.GetSearchSequenceData(iniName);
	if(AniDataIndex == -1)
		return FALSE;

	WorldX = (float)currentPos.x;
	WorldY = (float)currentPos.y;
	WorldZ = (float)currentPos.z;
	
	LocalX = 0;
	LocalY = 0;
	LocalZ = 0;
	
//	TermFrame = 0;
	AniType = ANI_LOOP;
	
	if(Physics== NULL)
	{
		HoPhysicsDest *physicsDest = new HoPhysicsDest;
		if(physicsDest != NULL)
		{
			if(!physicsDest->Start(currentPos, destPos, 0))
				return FALSE;

			Physics = physicsDest;
		}
	}
	StartFlag = TRUE;
	SizeX = 7;
	SizeY = 23;
	
	return TRUE;	
}

int HoPrimitivePolygon::StartParticleTri(POINT3D &currentPos, POINT3D &velocity, char *iniName)
{
	if(StartFlag)
		return FALSE;

	AniDataIndex = AnimDataMgr.GetSearchSequenceData(iniName);
	if(AniDataIndex == -1)
		return FALSE;

	WorldX = (float)currentPos.x;
	WorldY = (float)currentPos.y;
	WorldZ = (float)currentPos.z;
	
	LocalX = 0;
	LocalY = 0;
	LocalZ = 0;
	
//	TermFrame = 0;
	AniType = ANI_LOOP;
	
	if(Physics== NULL)
	{
		HoPhysicsParticle *physicsParticle = new HoPhysicsParticle();
		if(physicsParticle != NULL)
		{

			if(!physicsParticle->Start(velocity))
				return FALSE;
			
			physicsParticle->SetGravity(-9);
			physicsParticle->SetLive(50);
			
			Physics = physicsParticle;
		}
	}
	StartFlag = TRUE;
	SizeX = 10;
	SizeY = 28;
	return TRUE;
}


int HoPrimitivePolygon::CreatePathFace()
{
	//Vertex 값 초기화..
	memset(Vertex, 0, sizeof(Vertex));
	memset(Face, 0, sizeof(Face));
	memset(TexLink, 0, sizeof(TexLink));
	VertexCount = 0;
	FaceCount = 0;
	TexLinkCount = 0;
	
	
	POINT3D currentPos;
	currentPos.x = int(WorldX+LocalX);
	currentPos.y = int(WorldY+LocalY);
	currentPos.z = int(WorldZ+LocalZ);

	//추적 하는 방향의 백터를 구한다.
	hoPOINT3D vector;
	memset(&vector, 0, sizeof(vector));
	if(Physics != NULL)
	vector = Physics->GetVector();
	
	float xv = float(vector.x);
	float yv = float(vector.y);
	float zv = float(vector.z);
	
	//앞쪽...
	Vertex[VertexCount].x   = int(currentPos.x);
	Vertex[VertexCount].y   = int(currentPos.y-SizeY*5);
	Vertex[VertexCount].z   = int(currentPos.z);
	
	
	Vertex[VertexCount+1].x = int(currentPos.x);
	Vertex[VertexCount+1].y = int(currentPos.y+SizeY*5);
	Vertex[VertexCount+1].z = int(currentPos.z);
	
	//꼬리 부분..
	Vertex[VertexCount+2].x = int(currentPos.x-xv*SizeX);
	Vertex[VertexCount+2].y = int(currentPos.y-yv*SizeX);
	Vertex[VertexCount+2].z = int(currentPos.z-zv*SizeX);
	
	VertexCount += 3;
	
	Vertex[0].sDef_Color[0] = 255;
	Vertex[0].sDef_Color[1] = 255;
	Vertex[0].sDef_Color[2] = 255;
	Vertex[0].sDef_Color[3] = (int)CurrentBlendValue;

	Vertex[1].sDef_Color[0] = 255;
	Vertex[1].sDef_Color[1] = 255;
	Vertex[1].sDef_Color[2] = 255;
	Vertex[1].sDef_Color[3] = (int)CurrentBlendValue;

	Vertex[2].sDef_Color[0] = 100;
	Vertex[2].sDef_Color[1] = 100;
	Vertex[2].sDef_Color[2] = 100;
	Vertex[2].sDef_Color[3] = (int)CurrentBlendValue;
	
	//Texture 좌표 관련 초기화...
	TexLink[TexLinkCount].hTexture = 0;
	TexLink[TexLinkCount].NextTex = 0;
	
	TexLink[TexLinkCount].u[0] = 0.0f;
	TexLink[TexLinkCount].v[0] = 0.0f;
	TexLink[TexLinkCount].u[1] = 0.0f;
	TexLink[TexLinkCount].v[1] = 1.0f;
	TexLink[TexLinkCount].u[2] = 1.0f;
	TexLink[TexLinkCount].v[2] = 0.0f;
	


	//삼각형 컬링 때문에....
	Face[FaceCount].Vertex[0] = 0;
	Face[FaceCount].Vertex[1] = 1;
	Face[FaceCount].Vertex[2] = 2;
	Face[FaceCount].Vertex[3] = AnimDataMgr.ImageDataBuffer[AnimDataMgr.SequenceDataBuffer[AniDataIndex].DataFileIndex].GetMaterialNum();
	Face[FaceCount].lpTexLink = &TexLink[TexLinkCount];
	Face[FaceCount].sum = 0;
	FaceCount++;
	TexLinkCount++;

	
	//앞쪽...
	//앞쪽...
	Vertex[VertexCount].x   = int(currentPos.x);
	Vertex[VertexCount].y   = int(currentPos.y);
	Vertex[VertexCount].z   = int(currentPos.z+SizeY*5);

	Vertex[VertexCount+1].x = int(currentPos.x);
	Vertex[VertexCount+1].y = int(currentPos.y);
	Vertex[VertexCount+1].z = int(currentPos.z-SizeY*5);
	
	//꼬리 부분..
	Vertex[VertexCount+2].x = int(currentPos.x-xv*SizeX);
	Vertex[VertexCount+2].y = int(currentPos.y-yv*SizeX);
	Vertex[VertexCount+2].z = int(currentPos.z-zv*SizeX);
	

	Vertex[VertexCount].sDef_Color[0] = 255;
	Vertex[VertexCount].sDef_Color[1] = 255;
	Vertex[VertexCount].sDef_Color[2] = 255;
	Vertex[VertexCount].sDef_Color[3] = (int)CurrentBlendValue;

	Vertex[VertexCount+1].sDef_Color[0] = 255;
	Vertex[VertexCount+1].sDef_Color[1] = 255;
	Vertex[VertexCount+1].sDef_Color[2] = 255;
	Vertex[VertexCount+1].sDef_Color[3] = (int)CurrentBlendValue;

	Vertex[VertexCount+2].sDef_Color[0] = 100;
	Vertex[VertexCount+2].sDef_Color[1] = 100;
	Vertex[VertexCount+2].sDef_Color[2] = 100;
	Vertex[VertexCount+2].sDef_Color[3] = (int)CurrentBlendValue;
	
	
	//Texture 좌표 관련 초기화...
	TexLink[TexLinkCount].hTexture = 0;
	TexLink[TexLinkCount].NextTex = 0;
	
	TexLink[TexLinkCount].u[0] = 0.0f;
	TexLink[TexLinkCount].v[0] = 0.0f;
	TexLink[TexLinkCount].u[1] = 0.0f;
	TexLink[TexLinkCount].v[1] = 1.0f;
	TexLink[TexLinkCount].u[2] = 1.0f;
	TexLink[TexLinkCount].v[2] = 0.0f;
	


	//삼각형 컬링 때문에....
	Face[FaceCount].Vertex[0] = VertexCount;
	Face[FaceCount].Vertex[1] = VertexCount+1;
	Face[FaceCount].Vertex[2] = VertexCount+2;
	Face[FaceCount].Vertex[3] = AnimDataMgr.ImageDataBuffer[AnimDataMgr.SequenceDataBuffer[AniDataIndex].DataFileIndex].GetMaterialNum();
	Face[FaceCount].lpTexLink = &TexLink[TexLinkCount];
	Face[FaceCount].sum = 0;

	VertexCount += 3;
	FaceCount++;
	TexLinkCount++;
	return TRUE;
}

HoPrimitiveBillboard::HoPrimitiveBillboard()
{
	//SumCount = 0;

	Physics = NULL;
	Parent = NULL;
	Init();
}

HoPrimitiveBillboard::~HoPrimitiveBillboard()
{
	Init();
}

void HoPrimitiveBillboard::Init()
{
	PutAngle  = 0;
	AngleStep = 0;
	WorldX = 0, WorldY = 0, WorldZ = 0;
	LocalX = 0, LocalY = 0, LocalZ = 0;
	
	TimeCount = 0;
	
	AniDataIndex = -1;
	AniType	= ANI_NONE;
	StartFlag = FALSE;
	CurrentFrame = 0;
	CurrentBlendValue = 255;
	BlendStep = 0;
//	TermFrame = 0;
	
	memset(&DirectionVelocity, 0, sizeof(DirectionVelocity));
	memset(&DirectionAngle, 0, sizeof(DirectionAngle));
	memset(&DirectionAngleStep, 0, sizeof(DirectionAngleStep));
	
	WorkState = 0;
	Parent = NULL;
}


int HoPrimitiveBillboard::Main()
{
	if(StartFlag == FALSE || AniType == ANI_NONE || AniDataIndex == -1)
		return FALSE;
	
	if(Parent != NULL)
	{
		if(Parent->GetState())
		{
			hoPOINT3D worldPos = Parent->GetWorld();
			TranslateWorld( (int)worldPos.x, (int)worldPos.y, (int)worldPos.z);
		}
		else
			Parent = NULL;
	}

	//Animation 한번...
	if(CurrentFrame < AnimDataMgr.SequenceDataBuffer[AniDataIndex].AnimFrameCount)
	{
		if(TimeCount >= AnimDataMgr.SequenceDataBuffer[AniDataIndex].AnimFrame[CurrentFrame].Delay)
		{
			TimeCount = 0;
			if(AniType == ANI_ONE) //Animation 한번..
			{
				
				CurrentFrame++;
				if(CurrentFrame >= AnimDataMgr.SequenceDataBuffer[AniDataIndex].AnimFrameCount)
				{
					Init();
					return TRUE;
				}
				
				
			}
			//Animation의 루프는 처음에 0번부터 시작을 해서 다음 루프때는
			//1번부 제일 끝 프레임 전까지 루프를 돈다.. 왜냐면 처음 Frmae과
			//끝프레임은 블렌딩을 먹게 하기 위해서이다...
			else if(AniType == ANI_LOOP) 
			{
				CurrentFrame++;
				if(CurrentFrame >= AnimDataMgr.SequenceDataBuffer[AniDataIndex].AnimFrameCount-1)
					CurrentFrame = 1;
			}

			//Blend 값 설정...
			BlendStep = (AnimDataMgr.SequenceDataBuffer[AniDataIndex].AnimFrame[CurrentFrame].AlphaValue-CurrentBlendValue)/
				AnimDataMgr.SequenceDataBuffer[AniDataIndex].AnimFrame[CurrentFrame].Delay;
			



			//사이즈 변화가 있을 경우..
			if(AnimDataMgr.SequenceDataBuffer[AniDataIndex].GetAnimInfo() == INFO_ONESIZE)
			{
				SizeStep = float((AnimDataMgr.SequenceDataBuffer[AniDataIndex].AnimFrame[CurrentFrame].SizeWidth-SizeWidth)/
					AnimDataMgr.SequenceDataBuffer[AniDataIndex].AnimFrame[CurrentFrame].Delay);
			}
			else if(AnimDataMgr.SequenceDataBuffer[AniDataIndex].GetAnimInfo() == INFO_ONESIZEANGLE)
			{
				SizeStep = float((AnimDataMgr.SequenceDataBuffer[AniDataIndex].AnimFrame[CurrentFrame].SizeWidth-SizeWidth)/
					AnimDataMgr.SequenceDataBuffer[AniDataIndex].AnimFrame[CurrentFrame].Delay);
				
				AngleStep = (AnimDataMgr.SequenceDataBuffer[AniDataIndex].AnimFrame[CurrentFrame].Angle-PutAngle)/
					(float)AnimDataMgr.SequenceDataBuffer[AniDataIndex].AnimFrame[CurrentFrame].Delay;
			}
		}
		
		CurrentBlendValue = CurrentBlendValue+BlendStep;
		//사이즈 변화가 있는 경우..
		if(AnimDataMgr.SequenceDataBuffer[AniDataIndex].GetAnimInfo() == INFO_ONESIZE)
		{
			SizeWidth    += SizeStep;
			SizeHeight   += SizeStep;
			Face2d.width  = (int)SizeWidth<<FLOATNS;
			Face2d.height = (int)SizeHeight<<FLOATNS;
		}
		if(AnimDataMgr.SequenceDataBuffer[AniDataIndex].GetAnimInfo() == INFO_ONESIZEANGLE)
		{
			SizeWidth    += SizeStep;
			SizeHeight   += SizeStep;
			Face2d.width  = (int)SizeWidth<<FLOATNS;
			Face2d.height = (int)SizeHeight<<FLOATNS;
			PutAngle     += AngleStep;
		}
		
		if(Physics != NULL)
		{
			if(WorkState & MOVE_OBJECT)
			{
				Physics->Main();
				//if(Physics->GetState())
				//{
					hoPOINT3D vector = Physics->GetVector();
					DirectionVelocity.x = (int)vector.x;
					DirectionVelocity.y = (int)vector.y;
					DirectionVelocity.z = (int)vector.z;
				//}
				//else
				//{
					//DirectionVelocity.x = 0;
					//DirectionVelocity.y = 0;
					//DirectionVelocity.z = 0;
				//}
				if(Physics->GetState() == FALSE)
				{
					AniType = ANI_ONE;
					CurrentFrame = AnimDataMgr.SequenceDataBuffer[AniDataIndex].AnimFrameCount-1;
					TimeCount = 0;
					CurrentBlendValue = AnimDataMgr.SequenceDataBuffer[AniDataIndex].AnimFrame[CurrentFrame-1].AlphaValue;
					//Blend 값 설정...
					BlendStep = (AnimDataMgr.SequenceDataBuffer[AniDataIndex].AnimFrame[CurrentFrame].AlphaValue-CurrentBlendValue)/
					AnimDataMgr.SequenceDataBuffer[AniDataIndex].AnimFrame[CurrentFrame].Delay;

					//EnterCriticalSection(&cLoadSection);
					delete Physics;
					Physics = NULL;
					//LeaveCriticalSection(&cLoadSection);

				}
			}
			
			
			if(Physics != NULL)
			{
				//이동중 각도 변화가 있는 경우..(y축 회전.)
				if(WorkState & MOVE_ANGLE)
				{
					hoPOINT3D directionAngleStep;
					if(Physics->GetState())
						directionAngleStep = Physics->GetAngle();

					DirectionAngleStep.x = (int)directionAngleStep.x;
					DirectionAngleStep.y = (int)directionAngleStep.y;
					DirectionAngleStep.z = (int)directionAngleStep.z;
				}
			}

		}
		
		DirectionAngle.x += DirectionAngleStep.x;
		DirectionAngle.y += DirectionAngleStep.y;
		DirectionAngle.z += DirectionAngleStep.z;

		LocalX  += DirectionVelocity.x;
		LocalY  += DirectionVelocity.y;
		LocalZ  += DirectionVelocity.z;

		//WorldX  += DirectionVelocity.x;
		//WorldY  += DirectionVelocity.y;
		//WorldZ  += DirectionVelocity.z;


		TimeCount++;
	}
	return TRUE;
}


//실장님 define 내가 원하는 빌보드가 없어서...여기에 구현한다.
#define FLOATS_EM				(FLOATNS+3)
#define SM_DIST_NEARZ			16
#define SMFLOAT_DIST_NEARZ		(SM_DIST_NEARZ<<FLOATNS)

BOOL GetCameraCoord(POINT3D &world)
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
	world.z -= 24*fONE;
	
	if ( world.z<SMFLOAT_DIST_NEARZ )
		return FALSE;
	return TRUE;
}

BOOL HoPrimitiveBillboard::AddFace2D(smFACE2D *face)
{
	if(WorkState == 0)
		return FALSE;

	int width , height;

	smRENDVERTEX *v[4];
	smRENDFACE	 *rf;

	short		sColor[4];

	sColor[ SMC_A ] = face->Transparency;
	sColor[ SMC_R ] = face->r;
	sColor[ SMC_G ] = face->g;
	sColor[ SMC_B ] = face->b;
	
	
	if(WorkState & PRIMITIVE_PATH_OBJECT) //목적지를 향해서 날아가는 경우..
	{
		POINT3D destPos;
		destPos.x = int(WorldX+LocalX);
		destPos.y = int(WorldY+LocalY);
		destPos.z = int(WorldZ+LocalZ);
		
		POINT3D currentPos;
		if(WorkState & PRIMITIVE_PATH_RECT)
		{
			currentPos.x = int(destPos.x-DirectionVelocity.x*SizeWidth); //날라가는 폴리곤의 길이..
			currentPos.y = int(destPos.y-DirectionVelocity.y*SizeWidth);
			currentPos.z = int(destPos.z-DirectionVelocity.z*SizeWidth);
		}
		
		if(WorkState & PRIMITIVE_PATH_RECT_STRETCH)
		{
			currentPos.x = int(WorldX+LocalStartPos.x); 
			currentPos.y = int(WorldY+LocalStartPos.y);
			currentPos.z = int(WorldZ+LocalStartPos.z);
		}

		if(GetCameraCoord(currentPos) == FALSE)
			return FALSE;

		if(GetCameraCoord(destPos) == FALSE)
			return FALSE;
		
		float dx    = float(destPos.x - currentPos.x); //대상 까지의 x축의 기울기를 구한다.
		float dy    = float(destPos.y - currentPos.y); //대상 까지의 y축의 기울기를 구한다.
		float oolen = (float)sqrt(dx*dx+dy*dy); //대상 까지의 거리를 구한다.

		dx = dx/oolen*SizeHeight; //넓이.
		dy = dy/oolen*SizeHeight;
		
		D3DVECTOR persp;
		persp.x = -dy;
		persp.y = +dx;
		persp.z = 0;
		
		if (abs((int)dx) > 1.0f) currentPos.x -= (int)dx;
		if (abs((int)dy) > 1.0f) currentPos.y -= (int)dy;

		v[0] = smRender.AddRendVertex ( int(destPos.x-persp.x), int(destPos.y-persp.y), int(destPos.z), sColor );
		v[1] = smRender.AddRendVertex ( int(destPos.x+persp.x), int(destPos.y+persp.y), int(destPos.z), sColor );
		v[3] = smRender.AddRendVertex ( int(currentPos.x+persp.x), int(currentPos.y+persp.y), int(currentPos.z), sColor );
		v[2] = smRender.AddRendVertex ( int(currentPos.x-persp.x), int(currentPos.y-persp.y), int(currentPos.z), sColor );
	}
	
	if(WorkState & PRIMITIVE_DEFAULT_RECT)
	{
		float localAngleX = 0;
		float localAngleY = 0;
		float localAngleZ = 0;
		
		double sin = dbGetSin[int(DirectionAngle.y)&ANGCLIP];
		double cos = dbGetCos[int(DirectionAngle.y)&ANGCLIP];
		
		localAngleX = float((LocalX*cos - LocalZ*sin));
		localAngleZ = float((LocalX*sin + LocalZ*cos));

		//중심 좌표를 카메라 좌료로 변환..
		POINT3D camera;
		if(WorkState & MOVE_ANGLE)
		{
			camera.x = int(WorldX+localAngleX);
			camera.y = int(WorldY+LocalY);
			camera.z = int(WorldZ+localAngleZ);
		}
		else
		{
			camera.x = face->x;
			camera.y = face->y;
			camera.z = face->z;
		}

		if(GetCameraCoord(camera) == FALSE)
			return FALSE;
		
		width  = face->width  >> 1;
		height = face->height >> 1;

		double fx[4], fy[4];
		double sinY, cosY;
		double  dx[4], dy[4];
		int cnt;
		
		//int t = int(PutAngle)&ANGCLIP;

		sinY = dbGetSin[int(PutAngle)&ANGCLIP];
		cosY = dbGetCos[int(PutAngle)&ANGCLIP];
		dx[0] = -width;   dy[0] = -height;
		dx[1] =  width;   dy[1] = -height;
		dx[2] = -width;	  dy[2] =  height;
		dx[3] =  width;   dy[3] =  height;

		for( cnt=0;cnt<4;cnt++) {
			fx[cnt] = dy[cnt]*sinY + dx[cnt]*cosY;
			fy[cnt] = dy[cnt]*cosY - dx[cnt]*sinY;

			fx[cnt] += (float)camera.x;
			fy[cnt] += (float)camera.y;
		}

		v[0] = smRender.AddRendVertex ( (int)fx[0], (int)fy[0], camera.z, sColor );
		v[1] = smRender.AddRendVertex ( (int)fx[1], (int)fy[1], camera.z, sColor );
		v[2] = smRender.AddRendVertex ( (int)fx[2], (int)fy[2], camera.z, sColor );
		v[3] = smRender.AddRendVertex ( (int)fx[3], (int)fy[3], camera.z, sColor );
	}
	
	//첫번째 폴리곤 ( 1 - 2 - 3 );
	rf = &smRender.RendFace[ smRender.nRendFace++ ];
	rf->lpRendVertex[0] = v[0];
	rf->lpRendVertex[1] = v[1];
	rf->lpRendVertex[2] = v[2];
	rf->Matrial    = face->MatNum ;		//메트리얼 복사
	rf->ClipStatus = v[0]->ClipStatus|v[1]->ClipStatus|v[2]->ClipStatus;
	rf->lpTexLink  = 0;

	//텍스쳐 연결
	smRender.AddRendTempTexLink( rf , 0,	face->TexRect.left , face->TexRect.bottom,
								face->TexRect.right , face->TexRect.bottom,
								face->TexRect.left , face->TexRect.top );


	//두번째 폴리곤 ( 2 - 4 - 3 );
	rf = &smRender.RendFace[ smRender.nRendFace++ ];
	rf->lpRendVertex[0] = v[1];
	rf->lpRendVertex[1] = v[3];
	rf->lpRendVertex[2] = v[2];
	rf->Matrial    = face->MatNum ;		//메트리얼 복사
	rf->ClipStatus = v[1]->ClipStatus|v[2]->ClipStatus|v[3]->ClipStatus;
	rf->lpTexLink  = 0;

	//텍스쳐 연결
	smRender.AddRendTempTexLink( rf , 0,	face->TexRect.right , face->TexRect.bottom,
											face->TexRect.right , face->TexRect.top,
											face->TexRect.left , face->TexRect.top );


	//return &RendFace[ nRendFace-1 ];
	return TRUE;
}




//===================================================================================
//ktj : 한장놓고 그림바꿔주는 이펙트
//===================================================================================

int HoPrimitiveBillboard::Draw(int x, int y, int z, int ax, int ay, int az)
{
	if(StartFlag == FALSE || AniDataIndex == -1)
		return FALSE;

	smRender.Init();
	//smMaterialGroup->smMaterial[AnimDataMgr.ImageDataBuffer[AniDataIndex].GetMaterialNum()].TwoSide = TRUE;
	smRender.SetMaterialGroup(smMaterialGroup);


	//현재 찍을 Frame을 세팅한다..
	smMaterialGroup->SetMaterialAnimFrame( 
		&smMaterial[AnimDataMgr.ImageDataBuffer[AnimDataMgr.SequenceDataBuffer[AniDataIndex].DataFileIndex].GetMaterialNum()], 
		AnimDataMgr.SequenceDataBuffer[AniDataIndex].AnimFrame[CurrentFrame].ImageNum);
	

	//Camera각도 입력.
	smRender.SetCameraPosi(x, y, z, ax, ay, az);
	Face2d.x = int(WorldX+LocalX);
	Face2d.y = int(WorldY+LocalY);
	Face2d.z = int(WorldZ+LocalZ);

	Face2d.r = int(CurrentBlendValue);
	Face2d.g = int(CurrentBlendValue);
	Face2d.b = int(CurrentBlendValue);
	Face2d.Transparency = (int)CurrentBlendValue; 
	
	if(AddFace2D(&Face2d))
	{
		smRender.SetClipStates(SMCLIP_LEFT | SMCLIP_RIGHT | SMCLIP_TOP | SMCLIP_BOTTOM);
		smRender.ClipRendFace();
		smRender.GeomVertex2D(FALSE);

		lpD3DDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, FALSE);
		smRender.RenderD3D();
		lpD3DDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, TRUE);
	}
	
	return TRUE;
}

/*
void HoPrimitiveBillboard::Move(int x, int y, int z)
{
	LocalX = (float)x;
	LocalY = (float)y;
	LocalZ = (float)z;
}
*/

int HoPrimitiveBillboard::MoveTo(int x, int y, int z)
{
	LocalX += (float)x;
	LocalY += (float)y;
	LocalZ += (float)z;
	return TRUE;
}


//Effect에 크기 정보가 들어 있지 않은 경우..
//중심점과 가로, 세로 크기만 있으면 되는 빌보드...
int HoPrimitiveBillboard::StartBillRect(int x, int y, int z, int sizeX, int sizeY, char *iniName, int aniType)  
{
	if(StartFlag)
		return FALSE;
	
	AniDataIndex = AnimDataMgr.GetSearchSequenceData(iniName);
	
	if(AniDataIndex == FALSE)
		return FALSE;
	Face2d.MatNum = AnimDataMgr.GetMaterialNum(AniDataIndex);
	if(Face2d.MatNum == -1)
		return FALSE;
	
	WorkState |= PRIMITIVE_DEFAULT_RECT;
	WorldX = (float)x;
	WorldY = (float)y;
	WorldZ = (float)z;
	
	LocalX = 0;
	LocalY = 0;
	LocalZ = 0;

	//RenderType = RENDER_BILLBOARD;
	StartFlag = TRUE;
	AniType = aniType;
	PutAngle = 0;
	SizeWidth  = (float)sizeX;
	SizeHeight = (float)sizeY;

	Face2d.x = (int)(WorldX+LocalX);
	Face2d.y = (int)(WorldX+LocalY);
	Face2d.z = (int)(WorldX+LocalZ);
	Face2d.TexRect.left = 0;
	Face2d.TexRect.top = 0;
	Face2d.TexRect.right = 1;
	Face2d.TexRect.bottom = 1;
	Face2d.width  = (int)SizeWidth<<FLOATNS;
	Face2d.height = (int)SizeHeight<<FLOATNS;
	
	Face2d.Transparency = 255;
	Face2d.r = 255;
	Face2d.g = 255;
	Face2d.b = 255;
	
	CurrentBlendValue = (float)AnimDataMgr.SequenceDataBuffer[AniDataIndex].StartBlendValue;
	
	if(AnimDataMgr.SequenceDataBufferCount > 1)
	{
		
		CurrentBlendValue = (float)AnimDataMgr.SequenceDataBuffer[AniDataIndex].StartBlendValue;
		BlendStep = (AnimDataMgr.SequenceDataBuffer[AniDataIndex].AnimFrame[CurrentFrame].AlphaValue-CurrentBlendValue)/
								AnimDataMgr.SequenceDataBuffer[AniDataIndex].AnimFrame[CurrentFrame].Delay;
		SizeStep = 0;
	}
	else
	{
		BlendStep = 0;
	}
	
	if(AnimDataMgr.SequenceDataBuffer[AniDataIndex].BlendType == 0)
		smMaterialGroup->smMaterial[AnimDataMgr.ImageDataBuffer[AnimDataMgr.SequenceDataBuffer[AniDataIndex].DataFileIndex].GetMaterialNum()].BlendType  = SMMAT_BLEND_COLOR;
	else if(AnimDataMgr.SequenceDataBuffer[AniDataIndex].BlendType == 1)
		smMaterialGroup->smMaterial[AnimDataMgr.ImageDataBuffer[AnimDataMgr.SequenceDataBuffer[AniDataIndex].DataFileIndex].GetMaterialNum()].BlendType  = SMMAT_BLEND_ALPHA;
	else if(AnimDataMgr.SequenceDataBuffer[AniDataIndex].BlendType == 2)
		smMaterialGroup->smMaterial[AnimDataMgr.ImageDataBuffer[AnimDataMgr.SequenceDataBuffer[AniDataIndex].DataFileIndex].GetMaterialNum()].BlendType  = SMMAT_BLEND_LAMP;
	else if(AnimDataMgr.SequenceDataBuffer[AniDataIndex].BlendType == 3)
		smMaterialGroup->smMaterial[AnimDataMgr.ImageDataBuffer[AnimDataMgr.SequenceDataBuffer[AniDataIndex].DataFileIndex].GetMaterialNum()].BlendType  = SMMAT_BLEND_SHADOW;
	
	Face2d.width  = (int)SizeWidth<<FLOATNS;
	Face2d.height = (int)SizeHeight<<FLOATNS;
	
	return TRUE;
}

//가로, 세로 크기가 필요 없는 경우..
int HoPrimitiveBillboard::StartBillRect(int x, int y, int z, char *iniName, int aniType, int workState)  
{
	if(StartFlag)
		return FALSE;
	
	//Ani정보를 찾는다...
	AniDataIndex = AnimDataMgr.GetSearchSequenceData(iniName);
	if(AniDataIndex == -1)
		return FALSE;
	
	Face2d.MatNum = AnimDataMgr.GetMaterialNum(AniDataIndex);
	if(Face2d.MatNum == -1)
		return FALSE;
	
	WorkState = workState;
	if(WorkState == 0)
		return FALSE;

	WorldX = (float)x;
	WorldY = (float)y;
	WorldZ = (float)z;
	
	LocalX = 0;
	LocalY = 0;
	LocalZ = 0;
	
	StartFlag = TRUE;
	AniType = aniType;
	
	SizeWidth  = 0.f;
	SizeHeight = 0.f;
	Face2d.x = (int)(WorldX+LocalX);
	Face2d.y = (int)(WorldX+LocalY);
	Face2d.z = (int)(WorldX+LocalZ);

	Face2d.TexRect.left = 0;
	Face2d.TexRect.top = 0;
	Face2d.TexRect.right = 1;
	Face2d.TexRect.bottom = 1;
	Face2d.width  = (int)SizeWidth<<FLOATNS;
	Face2d.height = (int)SizeHeight<<FLOATNS;
	
	Face2d.Transparency = 255;
	Face2d.r = 255;
	Face2d.g = 255;
	Face2d.b = 255;

	CurrentBlendValue = (float)AnimDataMgr.SequenceDataBuffer[AniDataIndex].StartBlendValue;
	
	if(AniDataIndex != -1)
	{
		CurrentBlendValue = (float)AnimDataMgr.SequenceDataBuffer[AniDataIndex].StartBlendValue;
		BlendStep = (AnimDataMgr.SequenceDataBuffer[AniDataIndex].AnimFrame[CurrentFrame].AlphaValue-CurrentBlendValue)/
								AnimDataMgr.SequenceDataBuffer[AniDataIndex].AnimFrame[CurrentFrame].Delay;
		
		//Size변화가 있는 에니메이션인 경우..(가로, 세로 동일하게 사이즈가 늘어나는 경우)
		if(AnimDataMgr.SequenceDataBuffer[AniDataIndex].GetAnimInfo() == INFO_ONESIZE)
		{
			SizeStep = float(AnimDataMgr.SequenceDataBuffer[AniDataIndex].AnimFrame[CurrentFrame].SizeWidth/
			AnimDataMgr.SequenceDataBuffer[AniDataIndex].AnimFrame[CurrentFrame].Delay);
		}
		else if(AnimDataMgr.SequenceDataBuffer[AniDataIndex].GetAnimInfo() == INFO_ONESIZEANGLE)
		{
			SizeStep = float(AnimDataMgr.SequenceDataBuffer[AniDataIndex].AnimFrame[CurrentFrame].SizeWidth/
			AnimDataMgr.SequenceDataBuffer[AniDataIndex].AnimFrame[CurrentFrame].Delay);

			AngleStep = AnimDataMgr.SequenceDataBuffer[AniDataIndex].AnimFrame[CurrentFrame].Angle/
				(float)AnimDataMgr.SequenceDataBuffer[AniDataIndex].AnimFrame[CurrentFrame].Delay;
			PutAngle = 0;
		}
		else
			SizeStep = 0;
		
		Face2d.Transparency = (int)CurrentBlendValue;
	}
	else
	{
		BlendStep = 0;
	}
	
	if(AnimDataMgr.SequenceDataBuffer[AniDataIndex].BlendType == 0)
		smMaterialGroup->smMaterial[AnimDataMgr.ImageDataBuffer[AnimDataMgr.SequenceDataBuffer[AniDataIndex].DataFileIndex].GetMaterialNum()].BlendType  = SMMAT_BLEND_COLOR;
	else if(AnimDataMgr.SequenceDataBuffer[AniDataIndex].BlendType == 1)
		smMaterialGroup->smMaterial[AnimDataMgr.ImageDataBuffer[AnimDataMgr.SequenceDataBuffer[AniDataIndex].DataFileIndex].GetMaterialNum()].BlendType  = SMMAT_BLEND_ALPHA;
	else if(AnimDataMgr.SequenceDataBuffer[AniDataIndex].BlendType == 2)
		smMaterialGroup->smMaterial[AnimDataMgr.ImageDataBuffer[AnimDataMgr.SequenceDataBuffer[AniDataIndex].DataFileIndex].GetMaterialNum()].BlendType  = SMMAT_BLEND_LAMP;
	else if(AnimDataMgr.SequenceDataBuffer[AniDataIndex].BlendType == 3)
		smMaterialGroup->smMaterial[AnimDataMgr.ImageDataBuffer[AnimDataMgr.SequenceDataBuffer[AniDataIndex].DataFileIndex].GetMaterialNum()].BlendType  = SMMAT_BLEND_SHADOW;

	
	
	Face2d.width  = (int)SizeWidth<<FLOATNS;
	Face2d.height = (int)SizeHeight<<FLOATNS;
	return TRUE;
}

//파티클 경로로 날아가는 BillBoard
int HoPrimitiveBillboard::StartParticlePath(POINT3D currentPos, POINT3D velocity, POINT size, char *iniName, int aniType, int workState)
{
	if(StartFlag)
		return FALSE;
	
	AniDataIndex = AnimDataMgr.GetSearchSequenceData(iniName);
	if(AniDataIndex == -1)
		return FALSE;

	//시작 좌표 초기화..
	WorldX = (float)currentPos.x;
	WorldY = (float)currentPos.y;
	WorldZ = (float)currentPos.z;
	LocalX = 0;
	LocalY = 0;
	LocalZ = 0;
	
	WorkState = workState;
	if(WorkState == 0)
		return FALSE;
	StartFlag = TRUE;
	AniType = aniType;
	PutAngle = 0;

	SizeWidth = (float)size.x;
	SizeHeight = (float)size.y;

	Face2d.x = (int)LocalX;
	Face2d.y = (int)LocalY;
	Face2d.z = (int)LocalZ;
	Face2d.TexRect.left = 0;
	Face2d.TexRect.top = 0;
	Face2d.TexRect.right = 1;
	Face2d.TexRect.bottom = 1;
	Face2d.width  = (int)SizeWidth<<FLOATNS;
	Face2d.height = (int)SizeHeight<<FLOATNS;
	
	Face2d.Transparency = 255;
	Face2d.r = 255;
	Face2d.g = 255;
	Face2d.b = 255;

	Face2d.MatNum = AnimDataMgr.ImageDataBuffer[AnimDataMgr.SequenceDataBuffer[AniDataIndex].DataFileIndex].GetMaterialNum();
	CurrentBlendValue = (float)AnimDataMgr.SequenceDataBuffer[AniDataIndex].StartBlendValue;
	
	if(AnimDataMgr.SequenceDataBufferCount > 1)
	{
		
		CurrentBlendValue = (float)AnimDataMgr.SequenceDataBuffer[AniDataIndex].StartBlendValue;
		BlendStep = (AnimDataMgr.SequenceDataBuffer[AniDataIndex].AnimFrame[CurrentFrame].AlphaValue-CurrentBlendValue)/
								AnimDataMgr.SequenceDataBuffer[AniDataIndex].AnimFrame[CurrentFrame].Delay;

		SizeStep = float(AnimDataMgr.SequenceDataBuffer[AniDataIndex].AnimFrame[CurrentFrame].SizeWidth/
			AnimDataMgr.SequenceDataBuffer[AniDataIndex].AnimFrame[CurrentFrame].Delay);
	}
	else
	{
		BlendStep = 0;
	}
	
	if(AnimDataMgr.SequenceDataBuffer[AniDataIndex].BlendType == 0)
		smMaterialGroup->smMaterial[AnimDataMgr.ImageDataBuffer[AnimDataMgr.SequenceDataBuffer[AniDataIndex].DataFileIndex].GetMaterialNum()].BlendType  = SMMAT_BLEND_COLOR;
	else if(AnimDataMgr.SequenceDataBuffer[AniDataIndex].BlendType == 1)
		smMaterialGroup->smMaterial[AnimDataMgr.ImageDataBuffer[AnimDataMgr.SequenceDataBuffer[AniDataIndex].DataFileIndex].GetMaterialNum()].BlendType  = SMMAT_BLEND_ALPHA;
	else if(AnimDataMgr.SequenceDataBuffer[AniDataIndex].BlendType == 2)
		smMaterialGroup->smMaterial[AnimDataMgr.ImageDataBuffer[AnimDataMgr.SequenceDataBuffer[AniDataIndex].DataFileIndex].GetMaterialNum()].BlendType  = SMMAT_BLEND_LAMP;
	else if(AnimDataMgr.SequenceDataBuffer[AniDataIndex].BlendType == 3)
		smMaterialGroup->smMaterial[AnimDataMgr.ImageDataBuffer[AnimDataMgr.SequenceDataBuffer[AniDataIndex].DataFileIndex].GetMaterialNum()].BlendType  = SMMAT_BLEND_SHADOW;

	Face2d.width  = (int)SizeWidth<<FLOATNS;
	Face2d.height = (int)SizeHeight<<FLOATNS;
	
	if(Physics== NULL)
	{
		HoPhysicsParticle *physicsParticle = new HoPhysicsParticle();
		if(physicsParticle != NULL)
		{
			if(!physicsParticle->Start(velocity))
				return FALSE;
			
			physicsParticle->SetGravity(-9);
			physicsParticle->SetLive(50);
			Physics = physicsParticle;
		}
	}

	return TRUE;
}

//직선 경로를 날아가는 BillBoard
//currentPos: 시작 위치
//destPos: 도착할 위치
//size: Billboard의 크기
//iniName: Ani정보
//aniType: ANI_ONE, ANI_LOOP
//stretchFlag: BillBoard가 이동시 그려지는 부분이 처음 시작 부터인지 이동전의 값까지 인지..
int HoPrimitiveBillboard::StartDestPath(POINT3D worldPos, POINT3D localPos, POINT3D destPos, POINT size, char *iniName,int speed, int workState)
{
	if(StartFlag)
		return FALSE;
	
	AniDataIndex = AnimDataMgr.GetSearchSequenceData(iniName);
	if(AniDataIndex == -1)
		return FALSE;
	
	Face2d.MatNum = AnimDataMgr.GetMaterialNum(AniDataIndex);
	if(Face2d.MatNum == -1)
		return FALSE;
	
	WorkState = workState;
	if(WorkState == 0)
		return FALSE;

	WorldX = (float)worldPos.x;
	WorldY = (float)worldPos.y;
	WorldZ = (float)worldPos.z;
	
	LocalX = (float)localPos.x;
	LocalY = (float)localPos.y;
	LocalZ = (float)localPos.z;
	
	StartFlag = TRUE;
	AniType = ANI_LOOP;
	PutAngle = 0;

	SizeWidth = 0.f;
	SizeHeight = 0.f;

	Face2d.x = (int)LocalX;
	Face2d.y = (int)LocalY;
	Face2d.z = (int)LocalZ;
	Face2d.TexRect.left = 0;
	Face2d.TexRect.top = 0;
	Face2d.TexRect.right = 1;
	Face2d.TexRect.bottom = 1;
	Face2d.width  = (int)SizeWidth<<FLOATNS;
	Face2d.height = (int)SizeHeight<<FLOATNS;
	
	Face2d.Transparency = 255;
	Face2d.r = 255;
	Face2d.g = 255;
	Face2d.b = 255;

	Face2d.MatNum = AnimDataMgr.ImageDataBuffer[AnimDataMgr.SequenceDataBuffer[AniDataIndex].DataFileIndex].GetMaterialNum();
	CurrentBlendValue = (float)AnimDataMgr.SequenceDataBuffer[AniDataIndex].StartBlendValue;
	
	if(AnimDataMgr.SequenceDataBufferCount > 1)
	{
		
		CurrentBlendValue = (float)AnimDataMgr.SequenceDataBuffer[AniDataIndex].StartBlendValue;
		BlendStep = (AnimDataMgr.SequenceDataBuffer[AniDataIndex].AnimFrame[CurrentFrame].AlphaValue-CurrentBlendValue)/
								AnimDataMgr.SequenceDataBuffer[AniDataIndex].AnimFrame[CurrentFrame].Delay;

		SizeStep = float(AnimDataMgr.SequenceDataBuffer[AniDataIndex].AnimFrame[CurrentFrame].SizeWidth/
			AnimDataMgr.SequenceDataBuffer[AniDataIndex].AnimFrame[CurrentFrame].Delay);
	}
	else
	{
		BlendStep = 0;
	}
	
	if(AnimDataMgr.SequenceDataBuffer[AniDataIndex].BlendType == 0)
		smMaterialGroup->smMaterial[AnimDataMgr.ImageDataBuffer[AnimDataMgr.SequenceDataBuffer[AniDataIndex].DataFileIndex].GetMaterialNum()].BlendType  = SMMAT_BLEND_COLOR;
	else if(AnimDataMgr.SequenceDataBuffer[AniDataIndex].BlendType == 1)
		smMaterialGroup->smMaterial[AnimDataMgr.ImageDataBuffer[AnimDataMgr.SequenceDataBuffer[AniDataIndex].DataFileIndex].GetMaterialNum()].BlendType  = SMMAT_BLEND_ALPHA;
	else if(AnimDataMgr.SequenceDataBuffer[AniDataIndex].BlendType == 2)
		smMaterialGroup->smMaterial[AnimDataMgr.ImageDataBuffer[AnimDataMgr.SequenceDataBuffer[AniDataIndex].DataFileIndex].GetMaterialNum()].BlendType  = SMMAT_BLEND_LAMP;
	else if(AnimDataMgr.SequenceDataBuffer[AniDataIndex].BlendType == 3)
		smMaterialGroup->smMaterial[AnimDataMgr.ImageDataBuffer[AnimDataMgr.SequenceDataBuffer[AniDataIndex].DataFileIndex].GetMaterialNum()].BlendType  = SMMAT_BLEND_SHADOW;
	
	SizeWidth  = (float)size.x;
	SizeHeight = (float)size.y;
	Face2d.width  = (int)SizeWidth<<FLOATNS;
	Face2d.height = (int)SizeHeight<<FLOATNS;
	
	POINT3D currentPos;
	currentPos.x = int(WorldX+LocalX);
	currentPos.y = int(WorldY+LocalY);
	currentPos.z = int(WorldZ+LocalZ);

	if(Physics== NULL)
	{
		HoPhysicsDest *physicsDest = new HoPhysicsDest;
		if(physicsDest != NULL)
		{
			if(!physicsDest->Start(currentPos, destPos, speed))
				return FALSE;
			Physics = physicsDest;
		}
	}

	LocalStartPos.x = LocalX;
	LocalStartPos.y = LocalY;
	LocalStartPos.z = LocalZ;

	return TRUE;
}

int HoPrimitiveBillboard::StartPath(POINT3D currentPos, char *iniName, int aniType)
{
	if(StartFlag)
		return FALSE;
	
	AniDataIndex = AnimDataMgr.GetSearchSequenceData(iniName);
	if(AniDataIndex == -1)
		return FALSE;
	
	Face2d.MatNum = AnimDataMgr.GetMaterialNum(AniDataIndex);
	if(Face2d.MatNum == -1)
		return FALSE;
	
	WorkState = PRIMITIVE_DEFAULT_RECT;
	
	WorldX = (float)currentPos.x;
	WorldY = (float)currentPos.y;
	WorldZ = (float)currentPos.z;
	LocalX = 0;
	LocalY = 0;
	LocalZ = 0;
	
	StartFlag = TRUE;
	AniType = aniType;
	PutAngle = 0;

	SizeWidth = 0.f;
	SizeHeight = 0.f;

	Face2d.x = 0;
	Face2d.y = 0;
	Face2d.z = 0;
	Face2d.TexRect.left = 0;
	Face2d.TexRect.top = 0;
	Face2d.TexRect.right = 1;
	Face2d.TexRect.bottom = 1;
	Face2d.width  = (int)SizeWidth<<FLOATNS;
	Face2d.height = (int)SizeHeight<<FLOATNS;
	
	Face2d.Transparency = 255;
	Face2d.r = 255;
	Face2d.g = 255;
	Face2d.b = 255;

	Face2d.MatNum = AnimDataMgr.ImageDataBuffer[AnimDataMgr.SequenceDataBuffer[AniDataIndex].DataFileIndex].GetMaterialNum();
	CurrentBlendValue = (float)AnimDataMgr.SequenceDataBuffer[AniDataIndex].StartBlendValue;
	
	if(AnimDataMgr.SequenceDataBufferCount > 1)
	{
		
		CurrentBlendValue = (float)AnimDataMgr.SequenceDataBuffer[AniDataIndex].StartBlendValue;
		BlendStep = (AnimDataMgr.SequenceDataBuffer[AniDataIndex].AnimFrame[CurrentFrame].AlphaValue-CurrentBlendValue)/
								AnimDataMgr.SequenceDataBuffer[AniDataIndex].AnimFrame[CurrentFrame].Delay;

		SizeStep = float(AnimDataMgr.SequenceDataBuffer[AniDataIndex].AnimFrame[CurrentFrame].SizeWidth/
			AnimDataMgr.SequenceDataBuffer[AniDataIndex].AnimFrame[CurrentFrame].Delay);
	}
	else
	{
		BlendStep = 0;
	}
	
	if(AnimDataMgr.SequenceDataBuffer[AniDataIndex].BlendType == 0)
		smMaterialGroup->smMaterial[AnimDataMgr.ImageDataBuffer[AnimDataMgr.SequenceDataBuffer[AniDataIndex].DataFileIndex].GetMaterialNum()].BlendType  = SMMAT_BLEND_COLOR;
	else if(AnimDataMgr.SequenceDataBuffer[AniDataIndex].BlendType == 1)
		smMaterialGroup->smMaterial[AnimDataMgr.ImageDataBuffer[AnimDataMgr.SequenceDataBuffer[AniDataIndex].DataFileIndex].GetMaterialNum()].BlendType  = SMMAT_BLEND_ALPHA;
	else if(AnimDataMgr.SequenceDataBuffer[AniDataIndex].BlendType == 2)
		smMaterialGroup->smMaterial[AnimDataMgr.ImageDataBuffer[AnimDataMgr.SequenceDataBuffer[AniDataIndex].DataFileIndex].GetMaterialNum()].BlendType  = SMMAT_BLEND_LAMP;
	else if(AnimDataMgr.SequenceDataBuffer[AniDataIndex].BlendType == 3)
		smMaterialGroup->smMaterial[AnimDataMgr.ImageDataBuffer[AnimDataMgr.SequenceDataBuffer[AniDataIndex].DataFileIndex].GetMaterialNum()].BlendType  = SMMAT_BLEND_SHADOW;
	
	SizeWidth  = 0;
	SizeHeight = 0;
	Face2d.width  = (int)SizeWidth<<FLOATNS;
	Face2d.height = (int)SizeHeight<<FLOATNS;
	
	Physics = NULL;
	return TRUE;
}


HoEffectCircleLine::HoEffectCircleLine()
{
	Step = 0.5f;
}

HoEffectCircleLine::~HoEffectCircleLine()
{


}


int HoEffectCircleLine::Start(int x, int y, int z, int sizeX, int sizeY, char *iniName, int aniType)
{
	HoPrimitiveBillboard::StartBillRect(x, y, z, sizeX, sizeY, iniName, aniType);
	//테스트용 속도가 느리므로 보정해야 함...
	double temp = (PI/180); //1radian의 값을 구한다.
	int		angle;
	float	velocity;
	angle = 45;
	velocity = rand()%100+100.0f;

	DxVelocity = float(velocity*cos(temp*angle));
	DyVelocity = float(-velocity*sin(temp*angle));
	
	CurrentFrame = 0;

	CircleTimeCount = 0;
	return TRUE;
}

int HoEffectCircleLine::Main()
{
	TimeCount++;
	CircleTimeCount++;

	if(CircleTimeCount >= 4)
	{
		LocalX += DxVelocity*CircleTimeCount;
		LocalZ += DyVelocity*CircleTimeCount;

		DxVelocity -= Step*CircleTimeCount;

		if(LocalX <= -100 && Step > 0)
		{
			Step = -0.5f*CircleTimeCount;
		}
		else if(LocalX >= 100 && Step < 0)
		{
			Step = 0.5f*CircleTimeCount;
		}

		if(TimeCount >= AnimDataMgr.SequenceDataBuffer[AniDataIndex].AnimFrame[CurrentFrame].Delay)
		{
			TimeCount = 0;
			
			
			bool flag = false;
			if(AniType == ANI_ONE) //Animation 한번..
			{
				if(CurrentFrame < AnimDataMgr.SequenceDataBuffer[AniDataIndex].AnimFrameCount-1)
				{
					CurrentFrame++;
					flag = true;
				}
		
				if(flag==false && CurrentFrame == AnimDataMgr.SequenceDataBuffer[AniDataIndex].AnimFrameCount-1)
					Init();
			}
			else if(AniType == ANI_LOOP) //Animation 루프...
			{
				if(CurrentFrame >= AnimDataMgr.SequenceDataBuffer[AniDataIndex].AnimFrameCount-2)
					CurrentFrame = 1;
				else
					CurrentFrame++;
			}

			if(AniDataIndex != -1)
			{
				//Blend 값 설정...
				if(CurrentFrame <= AnimDataMgr.SequenceDataBuffer[AniDataIndex].AnimFrameCount-1)
				{
					BlendStep = (AnimDataMgr.SequenceDataBuffer[AniDataIndex].AnimFrame[CurrentFrame].AlphaValue-CurrentBlendValue)/
						AnimDataMgr.SequenceDataBuffer[AniDataIndex].AnimFrame[CurrentFrame].Delay;
				}
			}
		}
		
		CurrentBlendValue = CurrentBlendValue+BlendStep;

		if(LocalZ < -70000)
			AniType = ANI_ONE;

		CircleTimeCount = 0;
	}	
	return TRUE;
}


HoEtcPrimitiveBillboardMove::HoEtcPrimitiveBillboardMove()
{
	Step = 40;
}

HoEtcPrimitiveBillboardMove::~HoEtcPrimitiveBillboardMove()
{

}

//===================================================================================
//ktj : 모르겟슴
//===================================================================================
int HoEtcPrimitiveBillboardMove::Draw(int x, int y, int z, int ax, int ay, int az)
{
	if(StartFlag == FALSE)
		return FALSE;

	//if(TermFrame <= 0)
	//{
		//if(RenderType == RENDER_NONE)
			//return FALSE;
		
		smRender.Init();
		smMaterialGroup->smMaterial[AnimDataMgr.ImageDataBuffer[AniDataIndex].GetMaterialNum()].TwoSide = TRUE;
		
		//smMaterial[hoEffectAnimationData.ImageDataBuffer[AniDataIndex].GetMaterialNum()].TwoSide = TRUE;
		smRender.SetMaterialGroup(smMaterialGroup);
		
		//현재 찍을 Frame을 세팅한다..
		smMaterialGroup->SetMaterialAnimFrame( &smMaterial[AnimDataMgr.ImageDataBuffer[AnimDataMgr.SequenceDataBuffer[AniDataIndex].DataFileIndex].GetMaterialNum()], 
		AnimDataMgr.SequenceDataBuffer[AniDataIndex].AnimFrame[CurrentFrame].ImageNum);
		//Camera각도 입력.
		smRender.SetCameraPosi(x, y, z, ax, ay, az);
		
		//if(RenderType == RENDER_BILLBOARD)
		//{
			Face2d.x = int(WorldX+LocalX);
			Face2d.y = int(WorldY+LocalY);
			Face2d.z = int(WorldZ+LocalZ);

			Face2d.r = (int)CurrentBlendValue;
			Face2d.g = (int)CurrentBlendValue;
			Face2d.b = (int)CurrentBlendValue;
			
			Face2d.Transparency = (int)CurrentBlendValue; //hoEffectAnimationData.SequenceDataBuffer[AniDataIndex].AnimFrame[CurrentFrame].BlendValue;
			AddRendFace = NULL;
			if(AddRendFace = smRender.AddFace2D(&Face2d))
			{
				smRender.SetClipStates(SMCLIP_LEFT | SMCLIP_RIGHT | SMCLIP_TOP | SMCLIP_BOTTOM);
				smRender.ClipRendFace();
				smRender.GeomVertex2D(FALSE);
				
				if(AddRendFace != NULL)
				{
					AddRendFace->lpRendVertex[1]->xy[0] += TranslateMoveX;
					AddRendFace->lpRendVertex[1]->xy[1] += TranslateMoveY;
				}
				AddRendFace--;
				if(AddRendFace != NULL)
				{
					AddRendFace->lpRendVertex[0]->xy[0] += TranslateMoveX;
					AddRendFace->lpRendVertex[1]->xy[0] += TranslateMoveX;
					AddRendFace->lpRendVertex[2]->xy[0] += TranslateMoveX;

					AddRendFace->lpRendVertex[0]->xy[1] += TranslateMoveY;
					AddRendFace->lpRendVertex[1]->xy[1] += TranslateMoveY;
					AddRendFace->lpRendVertex[2]->xy[1] += TranslateMoveY;
				}
				smRender.RenderD3D();
			}
		//}
	//}
	return TRUE;
}

int HoEtcPrimitiveBillboardMove::Start(int x, int y, int z, int sizeX, int sizeY, char *iniName, int aniType)
{
	HoPrimitiveBillboard::StartBillRect(x, y, z, sizeX, sizeY, iniName, aniType);
	Step = 35;
	TranslateMoveY = 0;
	PrimitiveMoveCount = 0;
	PrimitiveStopCount = 0;
	
	return TRUE;
}


int HoEtcPrimitiveBillboardMove::Main()
{
	if(StartFlag)
	{
		if(AniType == ANI_NONE)
			return FALSE;
		
		//Animation 한번...
		if(CurrentFrame < AnimDataMgr.SequenceDataBuffer[AniDataIndex].AnimFrameCount)
		{
			if(TimeCount >= AnimDataMgr.SequenceDataBuffer[AniDataIndex].AnimFrame[CurrentFrame].Delay)
			{
				TimeCount = 0;
				
				
				bool flag = false;
			
				if(AniType == ANI_ONE) //Animation 한번..
				{
					if(CurrentFrame < AnimDataMgr.SequenceDataBuffer[AniDataIndex].AnimFrameCount-1)
					{
						CurrentFrame++;
						flag = true;
					}
			
					if(flag==false && CurrentFrame == AnimDataMgr.SequenceDataBuffer[AniDataIndex].AnimFrameCount-1)
						Init();
				}
				if(AniDataIndex != -1)
				{
					//Blend 값 설정...
					if(CurrentFrame <= AnimDataMgr.SequenceDataBuffer[AniDataIndex].AnimFrameCount-1)
					{
						BlendStep = (AnimDataMgr.SequenceDataBuffer[AniDataIndex].AnimFrame[CurrentFrame].AlphaValue-CurrentBlendValue)/
							AnimDataMgr.SequenceDataBuffer[AniDataIndex].AnimFrame[CurrentFrame].Delay;
					}
				}	
			}
			TimeCount++;
		}

		CurrentBlendValue = CurrentBlendValue+BlendStep;
		
		if(PrimitiveMoveCount >= 4)
		{
			PrimitiveMoveCount = 0;
			
			if(TranslateMoveX != 0)
			{
				if(MoveState == 1)//RightMove
				{
					TranslateMoveX += Step;
					if(TranslateMoveX > 0)
						TranslateMoveX = 0;
				}
				else if(MoveState == 3)//LeftMove
				{
					TranslateMoveX += -Step;
					if(TranslateMoveX < 0)
						TranslateMoveX = 0;
				}
			}
			
			Step -= 3;
			if(Step < 5)
			{
				Step = 4;
			}
		}

		if(TranslateMoveX == 0 && PrimitiveStopCount >= 45)
		{
			TranslateMoveY -= 2;	
		}
		if(TranslateMoveX == 0)
			PrimitiveStopCount++;

		PrimitiveMoveCount++;
		
	}

	return TRUE;
}


HoEffectPatGetPos::HoEffectPatGetPos()
{
	PatObj = NULL;
	AnimationEnd = 0;

	SetSkillCode(0);
	Init();
}

HoEffectPatGetPos::~HoEffectPatGetPos()
{
	Init();
}

void HoEffectPatGetPos::Init()
{
	StartFlag = FALSE;
	CurrentFrame = 0;
	BlendStep = 0;
	FrameStep = 80;

}

int HoEffectPatGetPos::StartAni(int x, int y, int z, int angleX, int angleY, int angleZ, smPAT3D *pat, char *searchObjName)
{
	if(StartFlag)
		return FALSE;

	Init();
	PatObj = pat;
	WorldX = (float)x, WorldY = (float)y, WorldZ = (float)z;
	LocalX = 0;
	LocalY = 0;
	LocalZ = 0;

	StartFlag = TRUE;
	CurrentFrame = 159;
	PatObj->Frame = CurrentFrame;
	
	Angle.x = angleX;
	Angle.y = (ANGLE_180-angleY&ANGCLIP);
	Angle.z = angleZ;

	BlendStep = -50;
	PatObj = pat;
	
	ObjBip = NULL;
	if(searchObjName != NULL)
		ObjBip = pat->GetObjectFromName(searchObjName);

	return true;
}

//160이다...
int HoEffectPatGetPos::Main()
{
	if(!StartFlag)
		return FALSE;
	
	if(CurrentFrame > AnimationEnd*160-1)
	{
		AnimationEnd = 0;
		Init();
	}
	
	if(ObjBip != NULL)
	{
		smMATRIX *mWorld;
		mWorld = &ObjBip->mWorld;
		
		int x, y, z;
		x = int(mWorld->_41+WorldX);
		z = int(mWorld->_42+WorldZ);
		y = int(mWorld->_43+WorldY);

		StartEffect(x, y, z, EFFECT_NORMAL_HIT1);	
		
	}

	if(CurrentFrame > (AnimationEnd*160-1)-15*FrameStep)
		BlendStep -= 10;
	CurrentFrame += FrameStep;
	return TRUE;
}


int HoEffectPatGetPos::Draw(int x, int y, int z, int ax, int ay, int az)
{
	if(!StartFlag)
		return FALSE;
	
	POINT3D	CameraPos, CameraAngle;
	CameraPos.x = x;
	CameraPos.y = y;
	CameraPos.z = z;

	CameraAngle.x = ax;
	CameraAngle.y = ay;
	CameraAngle.z = az;
	
	POINT3D pos;
	pos.x = int(WorldX+LocalX);
	pos.y = int(WorldY+LocalY);
	pos.z = int(WorldZ+LocalZ);
	
	smRender.Init();
	smRender.Color_R = BlendStep;
	smRender.Color_G = BlendStep;
	smRender.Color_B = BlendStep;
	smRender.Color_A = BlendStep;
	
	PatObj->Frame = CurrentFrame;
	PatObj->SetPosi(&pos, &Angle);
	
	
	lpD3DDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, FALSE);
	PatObj->RenderD3D(&CameraPos, &CameraAngle);
	lpD3DDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, TRUE);
	
	smRender.Color_R = 0;
	smRender.Color_G = 0;
	smRender.Color_B = 0;
	smRender.Color_A = 0;
	
	return TRUE;
}








HoEffectPat::HoEffectPat()
{
	PatObj = NULL;
	AnimationEnd = 0;

	SetSkillCode(0);
	Init();
}

HoEffectPat::~HoEffectPat()
{
	/*
	if(PatObj != NULL)
	{
		PatObj = NULL;
	}
	*/
	Init();
}

void HoEffectPat::Init()
{
	StartFlag = FALSE;
	CurrentFrame = 0;
	BlendStep = 0;
//	TermFrame = 0;
	FrameStep = 80;

}

int HoEffectPat::StartAni(int x, int y, int z, int angleX, int angleY, int angleZ, smPAT3D *pat, int aniType)
{
	
	if(StartFlag)
		return FALSE;

	Init();
	PatObj = pat;
	WorldX = (float)x, WorldY = (float)y, WorldZ = (float)z;
	LocalX = 0;
	LocalY = 0;
	LocalZ = 0;

	StartFlag = TRUE;
	CurrentFrame = 159;
	PatObj->Frame = CurrentFrame;
	
	Angle.x = angleX;
	Angle.y = (ANGLE_180-angleY&ANGCLIP);
	Angle.z = angleZ;

	BlendStep = -50;
	PatObj = pat;
	AniType = aniType;
	return TRUE;
}


//160이다...
int HoEffectPat::Main()
{
	if(!StartFlag)
		return FALSE;
	
	if(CurrentFrame == 159 && GetSkillCode()==SKILL_PHYSICAL_ABSORB )
		SkillPlaySound( SKILL_SOUND_PHYSICAL_ABSORB2 , int(WorldX+LocalX), int(WorldY+LocalY), int(WorldZ+LocalZ));			//스킬 효과음

	if(CurrentFrame > AnimationEnd*160-1)
	{
		if(AniType == ANI_ONE)
		{
			AnimationEnd = 0;
			Init();
		}
		else
		{
			CurrentFrame = 159;
			BlendStep = 255;
		}
	}
	
	if(CurrentFrame > (AnimationEnd*160-1)-15*FrameStep)
		BlendStep -= 10;
	CurrentFrame += FrameStep;
	
	return TRUE;
}


//============================================================
//ktj : 창위로던지는 이펙트에 씀
//============================================================
int HoEffectPat::Draw(int x, int y, int z, int ax, int ay, int az)
{
	if(!StartFlag)
		return FALSE;
	
	POINT3D	CameraPos, CameraAngle;
	CameraPos.x = x;
	CameraPos.y = y;
	CameraPos.z = z;

	CameraAngle.x = ax;
	CameraAngle.y = ay;
	CameraAngle.z = az;
	
	POINT3D pos;
	pos.x = int(WorldX+LocalX);
	pos.y = int(WorldY+LocalY);
	pos.z = int(WorldZ+LocalZ);
	
	smRender.Init();
	smRender.Color_R = BlendStep;
	smRender.Color_G = BlendStep;
	smRender.Color_B = BlendStep;
	smRender.Color_A = BlendStep;
	
	PatObj->Frame = CurrentFrame;
	PatObj->SetPosi(&pos, &Angle);
	
	
	lpD3DDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, FALSE);
	PatObj->RenderD3D(&CameraPos, &CameraAngle);
	lpD3DDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, TRUE);
	
	smRender.Color_R = 0;
	smRender.Color_G = 0;
	smRender.Color_B = 0;
	smRender.Color_A = 0;
	
	return TRUE;
}

HoEffectPatHawk::HoEffectPatHawk()
{
	Loop = 0;
}

HoEffectPatHawk::~HoEffectPatHawk()
{
	Loop = 0;

}

//===================================================================================
//매를 날리기 위한.. 클래스...(Scout Hawk 전용)
//===================================================================================
int HoEffectPatHawk::Draw(int x, int y, int z, int ax, int ay, int az)
{
	if(Character == NULL)
		return FALSE;
	if(StartFlag && Character->FlagShow)
	{
		//int oldFrame = CurrentFrame;
		//int oldBlendStep = BlendStep;
		/*
		if(CurrentFrame > 160*2)
		{
			CurrentFrame -= 100;
			BlendStep -= 70;
			HoEffectPat::Draw(x, y, z, ax, ay, az);
			BlendStep -= 70;
			CurrentFrame -= 100;
			HoEffectPat::Draw(x, y, z, ax, ay, az);
			CurrentFrame = oldFrame;
			BlendStep = oldBlendStep;
		}
		*/
		
		HoEffectPat::Draw(x, y, z, ax, ay, az);
	}
	return TRUE;
}


int HoEffectPatHawk::Main()
{
	if(!StartFlag)
		return FALSE;
	
	smMATRIX *mWorld;
	int x, y, z;
	
 	if(Character)
	{
		if(Character->HoSkillCode != SkillCode || (Character->OnStageField>=0 && StageField[ Character->OnStageField ]->State == FIELD_STATE_VILLAGE))
		{
			//SkillEndAniFlag = FALSE;
			Loop = 1;
		}
	}
	
	if(ObjBip)
	{
		AnimObjectTree( ObjBip , CurrentFrame, 0, AngleY, 0);
		mWorld = &ObjBip->mWorld;
		
		x = int(mWorld->_41+WorldX);
		z = int(mWorld->_42+WorldZ);
		y = int(mWorld->_43+WorldY);
		
		WorldX = (float)x;
		WorldY = (float)y;
		WorldZ = (float)z;

		HawkPos.x = x;
		HawkPos.y = y;
		HawkPos.z = z;

		if(CurrentFrame < 160)
		{
			StartEffect(x, y, z, EFFECT_CRITICAL_HIT1);			
			StartEffect(x, y, z, EFFECT_GATE1);
		}

		if(Character != NULL)
		{
			Character->HoSkillPos.x = x;
			Character->HoSkillPos.y = y;
			Character->HoSkillPos.z = z;
		}
	}

 	if(CurrentFrame > AnimationEnd*160-1)
	{
		Init();
		StartEffect(x, y, z, EFFECT_CRITICAL_HIT1);			
		StartEffect(x, y, z, EFFECT_GATE1);

		for(int index = 0; index < CHR_IN_EFFECT_OBJECT_MAX; index++)
		{
			if(ChrInEffectObject[index].object == (HoEffectObject *)this)
			{
				ChrInEffectObject[index].object = NULL;
				break;
			}
		}

 		if(Character)
		{
			if(Character->HoSkillCode == SkillCode)
				Character->HoSkillCode = 0;
		}
		//Character->HoSkillCode = 0;
	}
	
	CurrentFrame += 60;
	if(CurrentFrame >= 90*160)
	{
		if(Loop > 1)
		{
			CurrentFrame = 30*160;
			Loop--;
		}
	}
	if(Character != NULL)
	{
		WorldX = (float)Character->pX;
		WorldY = (float)Character->pY;
		WorldZ = (float)Character->pZ;
	}
	else
	{
		CurrentFrame = AnimationEnd*160-159;
	}
	return TRUE;
}

//loop 매가 몇번 도는지...
// x, y, z: 매가 찍힐 World좌표...
// angleX, angleY, angleZ: 각도...
// patBone: Bone 정보..
// patAni: Ani정보..
// effectObj: 이펙트 정보..
// loop: 매가 도는 갯수..0이면 한바퀴.. 1이면 두바퀴...
int HoEffectPatHawk::StartAni(int x, int y, int z, 
							  int angleX, int angleY, int angleZ, 
							  smPAT3D *patBone, smPAT3D *patAni,
							  smCHAR *character,		
							  int loop)
{
	if(character != NULL)
	{
		Character = character;	
	}
	else
		Character = NULL;
	
	if(Character != NULL)
	{
		Character->HoSkillCode = SkillCode;
		//SkillEndAniFlag = FALSE;
	}
	
	ObjBip = NULL;
	angleY = (ANGLE_270-angleY&ANGCLIP);
	if(StartFlag)
		return FALSE;

	HoEffectPat::Init();
	PatObj = patAni;
	LocalX = 0, LocalY = 0, LocalZ = 0;
	WorldX = (float)x, WorldY = (float)y, WorldZ = (float)z;
	
	StartFlag = TRUE;
	CurrentFrame = 159;
	PatObj->Frame = CurrentFrame;
	
	Angle.x = angleX;
	Angle.y = angleY;
	Angle.z = angleZ;

	BlendStep = 0;
	Loop = loop;
	
	ObjBip = patBone->GetObjectFromName("Bip01");
	AngleY = angleY;
	
	return TRUE;
}


HoEffectPatFalcon::HoEffectPatFalcon()
{
	
	ParticleGlowID = -1;
	ParticleStarID = -1;
}

HoEffectPatFalcon::~HoEffectPatFalcon()
{

}


int HoEffectPatFalcon::StartGoldenFalconAni(int x, int y, int z, int angleX, int angleY, int angleZ, smPAT3D *patBone, smPAT3D *patAni, smCHAR *character, int liveCount)
{
	if(character != NULL)
	{
		Character = character;	
	}
	else
		Character = NULL;
	
	angleY = (ANGLE_180-angleY&ANGCLIP);
	angleY = angleY&ANGCLIP;
	if(StartFlag)
		return FALSE;

	HoEffectPat::Init();
	PatObj = patAni;

	//종료 시간 계산

/*
//팔콘 
int Falcon_Damage[10][2]  = {{10,36},{12,40},{14,44},{16,48},{18,52},{20,56},{22,60},{24,64},{26,68},{28,72}};
int Falcon_Time[10]       = {30,40,50,60,70,80,90,100,110,120};

 */

	pEndCounter =   liveCount*70;
	//pAttackDamage[0] =   Falcon_Damage[SkillPoint][0];
	//pAttackDamage[1] =   Falcon_Damage[SkillPoint][1];
	//pDelay = 70;
	pCounter = 0;
	
	if(Character == NULL)
	{
		LocalX = 0, LocalY = 0, LocalZ = 0;
		WorldX = (float)x, WorldY = (float)y, WorldZ = (float)z;
	}
	else
	{
		LocalX = 0, LocalY = 0, LocalZ = 0;
		WorldX = (float)(character->pX), WorldY = (float)(character->pY+20000), WorldZ = (float)(character->pZ);
		angleY = (ANGLE_180-(character->Angle.y))&ANGCLIP;

		Character->HoSkillCode = SkillCode;

	}
	
	StartFlag = TRUE;
	CurrentFrame = 0;
	PatObj->Frame = CurrentFrame;
	
	Angle.x = angleX;
	Angle.y = angleY;
	Angle.z = angleZ;
	
	BlendStep = -255;
	
	WorkState = FALCON_GATE_START;
	ObjBip = patBone->GetObjectFromName("Bip01");
	
	WorldX = (float)(Character->pX);
	WorldY = (float)(Character->pY+20000);
	WorldZ = (float)(Character->pZ);
	
 	if(SkillCode == SKILL_GOLDEN_FALCON)
	{
		POINT3D glowPos;
		glowPos.x = int(LocalX+WorldX);
		glowPos.y = int(LocalY+WorldY);
		glowPos.z = int(LocalZ+WorldZ);
		
		ParticleGlowPos.x = (float)glowPos.x;
		ParticleGlowPos.y = (float)glowPos.y;
		ParticleGlowPos.z = (float)glowPos.z;

		ParticleGlowID = g_NewParticleMgr.Start("Skill3ArcherGoldenFalcon1", glowPos);
		if(ParticleGlowID != -1)
		{
			g_NewParticleMgr.SetPos(ParticleGlowID, ParticleGlowPos);
		}
	}
	return TRUE;
}


int HoEffectPatFalcon::StartAni(int x, int y, int z, int angleX, int angleY, int angleZ, smPAT3D *patBone, smPAT3D *patAni, smCHAR *character , int SkillPoint )
{
	
 	if(character != NULL)
	{
		Character = character;	
	}
	else
		Character = NULL;
	
	angleY = (ANGLE_180-angleY&ANGCLIP);
	angleY = angleY&ANGCLIP;
	if(StartFlag)
		return FALSE;

	HoEffectPat::Init();
	PatObj = patAni;

	//종료 시간 계산

/*
//팔콘 
int Falcon_Damage[10][2]  = {{10,36},{12,40},{14,44},{16,48},{18,52},{20,56},{22,60},{24,64},{26,68},{28,72}};
int Falcon_Time[10]       = {30,40,50,60,70,80,90,100,110,120};

 */

	pEndCounter =   Falcon_Time[SkillPoint]*70;
	pAttackDamage[0] =   Falcon_Damage[SkillPoint][0];
	pAttackDamage[1] =   Falcon_Damage[SkillPoint][1];
	pDelay = 70;
	pCounter = 0;
	
	if(Character == NULL)
	{
		LocalX = 0, LocalY = 0, LocalZ = 0;
		WorldX = (float)x, WorldY = (float)y, WorldZ = (float)z;
	}
	else
	{
		LocalX = 0, LocalY = 0, LocalZ = 0;
		WorldX = (float)(character->pX), WorldY = (float)(character->pY+20000), WorldZ = (float)(character->pZ);
		angleY = (ANGLE_180-(character->Angle.y))&ANGCLIP;

		Character->HoSkillCode = SkillCode;

	}
	
	StartFlag = TRUE;
	CurrentFrame = 0;
	PatObj->Frame = CurrentFrame;
	
	Angle.x = angleX;
	Angle.y = angleY;
	Angle.z = angleZ;
	
	BlendStep = -255;
	
	WorkState = FALCON_GATE_START;
	ObjBip = patBone->GetObjectFromName("Bip01");
	
	WorldX = (float)(Character->pX);
	WorldY = (float)(Character->pY+20000);
	WorldZ = (float)(Character->pZ);
	
 	if(SkillCode == SKILL_GOLDEN_FALCON)
	{
		POINT3D glowPos;
		glowPos.x = int(LocalX+WorldX);
		glowPos.y = int(LocalY+WorldY);
		glowPos.z = int(LocalZ+WorldZ);
		
		ParticleGlowPos.x = (float)glowPos.x;
		ParticleGlowPos.y = (float)glowPos.y;
		ParticleGlowPos.z = (float)glowPos.z;

		ParticleGlowID = g_NewParticleMgr.Start("Skill3ArcherGoldenFalcon1", glowPos);
		if(ParticleGlowID != -1)
		{
			g_NewParticleMgr.SetPos(ParticleGlowID, ParticleGlowPos);
		}
	}
	return TRUE;
}




int HoEffectPatFalcon::Main()
{
	if(StartFlag)
	{
 		if(Character != NULL)
		{
			if(WorkState == FALCON_GATE_START)
			{
				static int delayCount = 0;
				if(CurrentFrame == 0)
				{
					CurrentFrame = 159;
					smMATRIX *mWorld;
					AnimObjectTree( ObjBip , 0, 0, Angle.y, 0);
					mWorld = &ObjBip->mWorld;
					StartEffect(int(WorldX+mWorld->_41), int(WorldY+mWorld->_43), int(WorldZ+mWorld->_42), EFFECT_FALCON_GATE1);


				}
				
				BlendStep += 255/40;
				if(delayCount > 40)
				{
					delayCount = 0;
					WorkState = FALCON_START;
					BlendStep = 0;
				}
				delayCount++;
			}

			if(WorkState == FALCON_START)
			{
				if(CurrentFrame >= 160*70-1)
					WorkState = FALCON_KEEP_ON;
			}
			
			static int heightCount = 0;
			static int heightTimeCount = 0;
			
 	 		if(WorkState == FALCON_KEEP_ON)
			{
				
				if(heightTimeCount < 120)
					heightCount += 30;
				if(heightTimeCount >= 120 && heightTimeCount < 240)
					heightCount -= 30;
				heightTimeCount++;
				if(heightTimeCount >= 240)
					heightTimeCount = 0;
			}
			

			if(WorkState != FALCON_GATE_START)
			{
				D3DVECTOR	attractLocation;
 				attractLocation.x = (Character->pX) -		(WorldX);
				attractLocation.y = (Character->pY+17000+heightCount) - (WorldY);
				attractLocation.z = (Character->pZ) -		(WorldZ);
				D3DVECTOR attractDir = attractLocation;

				//벡터의 길이를 구한다.
				float length;
 				length = (float)sqrt(attractDir.x*attractDir.x+attractDir.y*attractDir.y+attractDir.z*attractDir.z);
				
		 		if(length > 300.f)
				{
					WorldX += float((attractLocation.x)/length)*420.f;
					WorldY += float((attractLocation.y)/length)*420.f;
					WorldZ += float((attractLocation.z)/length)*420.f;
				}
				WorldY = (float)(Character->pY+17000+heightCount);
			}			
			
			//머리위에 유지를 하는 경우.
			if(WorkState == FALCON_KEEP_ON)
			{
			 	Angle.y = (ANGLE_180-(Character->Angle.y))&ANGCLIP;	
				if(CurrentFrame >= 160*100-1)
				CurrentFrame = 160*70;	


				if ( pCounter>pEndCounter ) {			//시간 끝
					StartFlag = 0;
				}
			}
			
			if(WorkState == FALCON_ATTACK)
			{
				if(CurrentFrame >= 160*120)
				{
					GetRadian3D( int(WorldX+LocalX), int(WorldY+LocalY), int(WorldZ+LocalZ), AttackDest.x, AttackDest.y, AttackDest.z);
					AttackAngle.x = GeoResult_X;
					AttackAngle.y = GeoResult_Y;
					AttackAngle.z = GeoResult_Z;
					
					Angle.x = GeoResult_X;
					Angle.y = (ANGLE_180-GeoResult_Y)&ANGCLIP;
					Angle.z = 0;
				}
				
				if(CurrentFrame >= 160*130-1)
				{
					GetMoveLocation( 0, 0, fONE*8, AttackAngle.x, AttackAngle.y, 0);
					
					LocalX += GeoResult_X;
					LocalY += GeoResult_Y;
					LocalZ += GeoResult_Z;
					CurrentFrame = 160*130-1;

					D3DVECTOR	attractLocation;
					attractLocation.x = AttackDest.x - (WorldX+LocalX);
					attractLocation.y = AttackDest.y - (WorldY+LocalY);
					attractLocation.z = AttackDest.z - (WorldZ+LocalZ);
					
					D3DVECTOR attractDir = attractLocation;
					//벡터의 길이를 구한다.
					float length;
					length = (float)(attractDir.x*attractDir.x+attractDir.y*attractDir.y+attractDir.z*attractDir.z);

					if(length < 2000*2000) //충돌 영역 결정..
					{
						WorkState = FALCON_TURN_START;
						CurrentFrame = 160*130;
						StartSkill(int(WorldX+LocalX), int(WorldY+LocalY), int(WorldZ+LocalZ), 0, 0, 0, SKILL_TRIPLE_IMPACT);

						//소리
						if ( (rand()%1)==0 ) 
							SkillPlaySound( SKILL_SOUND_SKILL_FALCON_ATTACK1 , int(WorldX+LocalX),int(WorldY+LocalY),int(WorldZ+LocalZ) );
						else
							SkillPlaySound( SKILL_SOUND_SKILL_FALCON_ATTACK2 , int(WorldX+LocalX),int(WorldY+LocalY),int(WorldZ+LocalZ) );

					}

				}
				
				/*
				if(ParticleGlowID != -1)
				{
					ParticleGlowPos.x = (float)(WorldX+LocalX);
					ParticleGlowPos.y = (float)(WorldY+LocalY);
					ParticleGlowPos.z = (float)(WorldZ+LocalZ);
				}
				*/

			}
			
			if(WorkState == FALCON_TURN_START)
			{
				if(CurrentFrame >= 160*150-1)
				{
					CurrentFrame = 160*150-1;
					WorkState = FALCON_TURN_PROCESS;
					
					smMATRIX *mWorld;
					AnimObjectTree( ObjBip , 160*150-1, 0, Angle.y, 0);
					mWorld = &ObjBip->mWorld;
					
					LocalX = float(mWorld->_41+LocalX);
					LocalY = float(mWorld->_43+LocalY);
					LocalZ = float(mWorld->_42+LocalZ);

					GetRadian3D( int(WorldX+LocalX), int(WorldY+LocalY), int(WorldZ+LocalZ), Character->pX+mWorld->_41, Character->pY+20000+mWorld->_43, Character->pZ+mWorld->_42);
					AttackAngle.x = GeoResult_X;
					AttackAngle.y = GeoResult_Y;
					AttackAngle.z = GeoResult_Z;
				
					LocalX = float(LocalX-mWorld->_41);
					LocalY = float(LocalY-mWorld->_43);
					LocalZ = float(LocalZ-mWorld->_42);
					
				}
			}

			if(WorkState == FALCON_TURN_PROCESS)
			{
				if(CurrentFrame >= 160*180)
				{
					CurrentFrame =160*150;
				}
				GetMoveLocation( 0, 0, fONE*6, AttackAngle.x, AttackAngle.y, 0);
				
				LocalX += GeoResult_X;
				LocalY += GeoResult_Y;
				LocalZ += GeoResult_Z;

				D3DVECTOR	attractLocation;
				attractLocation.x = (Character->pX) -		(WorldX+LocalX);
				attractLocation.y = (Character->pY+20000) - (WorldY+LocalY);
				attractLocation.z = (Character->pZ) -		(WorldZ+LocalZ);
				D3DVECTOR attractDir = attractLocation;

				//벡터의 길이를 구한다.
				float length;
				length = (float)(attractDir.x*attractDir.x+attractDir.y*attractDir.y+attractDir.z*attractDir.z);
				
				static bool endFlag = false;
				static int delayCount = 0;
				if(length < 12000*12000) //충돌 영역 결정..
				{
					endFlag = true;
				}

				if(endFlag)
				{
					if(delayCount > 8)
					{
						//////////////////////////////////

						endFlag = false;
						delayCount = 0;
						WorkState = FALCON_TURN_END;
						CurrentFrame = 160*180;
					}
					delayCount++;
				}
			}
			
			if(WorkState == FALCON_TURN_END)
			{
				/*
				smMATRIX *mWorld;
				AnimObjectTree( ObjBip , CurrentFrame, 0, Angle.y, 0);
				mWorld = &ObjBip->mWorld;
				

				D3DVECTOR	attractLocation;
				attractLocation.x = (Character->pX) -		(WorldX+LocalX+mWorld->_41);
				attractLocation.y = (Character->pY+20000) - (WorldY+LocalY+mWorld->_42);
				attractLocation.z = (Character->pZ) -		(WorldZ+LocalZ+mWorld->_43);
				D3DVECTOR attractDir = attractLocation;

				//벡터의 길이를 구한다.
				float length;
				length = (float)(attractDir.x*attractDir.x+attractDir.y*attractDir.y+attractDir.z*attractDir.z);
				
				if(length < 7000*7000)
				{
					WorkState = FALCON_KEEP_ON;
					CurrentFrame = 160*71;

					LocalX = 0.f;
					LocalY = 0.f;
					LocalZ = 0.f;

					Angle.x = 0;
					Angle.z = 0;
				}
				*/
				if(CurrentFrame >= 160*210)
				{
					WorkState = FALCON_KEEP_ON;
					CurrentFrame = 160*71;
					
					WorldX += LocalX;
					WorldY += LocalY;
					WorldZ += LocalZ;

					LocalX = 0.f;
					LocalY = 0.f;
					LocalZ = 0.f;
				}
				
			}

			if(WorkState == FALCON_KEEP_ON)
			{
				//if (Character->ShootingFlag > 1 && Character->chrAttackTarget)
				if ( Character->chrAttackTarget && Character->chrAttackTarget->Flag && 
					Character->chrAttackTarget->smCharInfo.Life[0]>0 &&
					(Character->MotionInfo->State==CHRMOTION_STATE_ATTACK || Character->MotionInfo->State==CHRMOTION_STATE_SKILL || Character->ShootingFlag ) )
				{
					WorkState = FALCON_ATTACK;
					AttackDest.x = Character->chrAttackTarget->pX;
					AttackDest.y = Character->chrAttackTarget->pY+5000;
					AttackDest.z = Character->chrAttackTarget->pZ;

					if ( Character==lpCurPlayer ) {
						SendFalconDamage( Character );

						/*
						//공격데이타 서버로 보냄
						int dmg = GetRandomPos(pAttackDamage[0],pAttackDamage[1]);
						if ( QuickSendTransAttack( Character->chrAttackTarget ,  dmg , 1 , 0 )==TRUE ) {
							//DamageExp( Character->chrAttackTarget , dmg );		//데미지에 따른 경험치 상승
						}
						*/

					}
/*
					if ( (rand()%1)==0 ) 
						SkillPlaySound( SKILL_SOUND_SKILL_FALCON_ATTACK1 , WorldX+LocalX,WorldY+LocalY,WorldZ+LocalZ );
					else
						SkillPlaySound( SKILL_SOUND_SKILL_FALCON_ATTACK2 , WorldX+LocalX,WorldY+LocalY,WorldZ+LocalZ );
*/
				}

			}
		}
		
		if(WorkState != FALCON_GATE_START)
		{
			if(WorkState == FALCON_START || WorkState == FALCON_ATTACK)
				CurrentFrame += 160;
			else
				CurrentFrame += 80;
		}

	}

	if ( !Character || pCounter>(pEndCounter+70*10) ) {
		StartFlag = 0;
	}

 	if ( !StartFlag || !Character || Character->HoSkillCode!=SkillCode ||
		(Character->OnStageField>=0 && StageField[ Character->OnStageField ]->State == FIELD_STATE_VILLAGE) ) 
	{
		if (Character) Character->HoSkillCode = 0;
		
		smMATRIX *mWorld;
		AnimObjectTree( ObjBip , CurrentFrame, 0, Angle.y, 0);
		mWorld = &ObjBip->mWorld;
		StartEffect(int(WorldX+LocalX+mWorld->_41), int(WorldY+LocalY+mWorld->_43), int(WorldZ+LocalZ+mWorld->_42), EFFECT_FALCON_GATE1);
		StartFlag = 0;

 		if(ParticleGlowID != -1)
		{
			g_NewParticleMgr.SetFastStop(ParticleGlowID);
			ParticleGlowID = -1;
		} 

		if(ParticleStarID != -1)
		{
			g_NewParticleMgr.SetFastStop(ParticleStarID);
			ParticleStarID = -1;
		}
	}
	
 	if(ParticleGlowID != -1)
	{
		//if(ParticleGlowID != -1)
		//{
			smMATRIX *mWorld;
   			AnimObjectTree( ObjBip , CurrentFrame, Angle.x, Angle.y, Angle.z);
			mWorld = &ObjBip->mWorld;

			ParticleGlowPos.x = (float)(WorldX+LocalX+mWorld->_41);
			ParticleGlowPos.y = (float)(WorldY+LocalY+mWorld->_43);
			ParticleGlowPos.z = (float)(WorldZ+LocalZ+mWorld->_42);
			
			g_NewParticleMgr.SetAttachPos(ParticleGlowID, ParticleGlowPos);
		//}
 		if(WorkState == FALCON_KEEP_ON)
		{
			if(ParticleStarID == -1)
			{
				POINT3D glowPos;
				glowPos.x = (int)ParticleGlowPos.x;
				glowPos.y = (int)ParticleGlowPos.y;
				glowPos.z = (int)ParticleGlowPos.z;

				ParticleStarID = g_NewParticleMgr.Start("Skill3ArcherGoldenFalcon2", glowPos);
			}
			else
				g_NewParticleMgr.SetPos(ParticleStarID, ParticleGlowPos);
		}
		else
		{
			if(ParticleStarID != -1)
			{
				g_NewParticleMgr.SetStop(ParticleStarID);
				ParticleStarID = -1;
			}
		}

	}
	pCounter++;				//카운터

	return TRUE;
}

//===================================================================================
//ktj : FALCON : 아처가 데리고 다니는 새이름임)
//===================================================================================
int HoEffectPatFalcon::Draw(int x, int y, int z, int ax, int ay, int az)
{
	/*
	if(StartFlag)
	{
		HoEffectPat::Draw(x, y, z, ax, ay, az);
	}
	*/

	if(!StartFlag)
		return FALSE;

	if(Character->FlagShow == FALSE)
		return FALSE;
	
	POINT3D	CameraPos, CameraAngle;
	CameraPos.x = x;
	CameraPos.y = y;
	CameraPos.z = z;

	CameraAngle.x = ax;
	CameraAngle.y = ay;
	CameraAngle.z = az;
	
	POINT3D pos;
	pos.x = int(WorldX+LocalX);
	pos.y = int(WorldY+LocalY);
	pos.z = int(WorldZ+LocalZ);
	
 	smRender.Init();
	//smRender.Color_R = BlendStep;
	//smRender.Color_G = BlendStep;
	//smRender.Color_B = BlendStep;
	smRender.Color_A = BlendStep;
	
	PatObj->Frame = CurrentFrame;
	PatObj->SetPosi(&pos, &Angle);
	
	lpD3DDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, FALSE);
	PatObj->RenderD3D(&CameraPos, &CameraAngle);
	lpD3DDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, TRUE);
	
	smRender.Color_R = 0;
	smRender.Color_G = 0;
	smRender.Color_B = 0;
	smRender.Color_A = 0;

	return TRUE;
}



/*
HoEffectParticle::HoEffectParticle()
{
	Init();
}

HoEffectParticle::~HoEffectParticle()
{
	Init();
}

void HoEffectParticle::Init()
{
	
	HoPrimitiveBillboard::Init();
	
	Xv = 0;
	Yv = 0;
	Zv = 0;
	TimeCount = 0;
	
	Gravity = 0.0f;
	Wind = 0.0f;
	
	SizeWidth = 10;
	SizeHeight = 10;
	Live = 0;
	StartFlag = FALSE;
}

int HoEffectParticle::Draw(int x, int y, int z, int ax, int ay, int az)
{
	if(StartFlag == FALSE)
		return FALSE;
	if(TermFrame <= 0)
	{
		//if(RenderType == RENDER_NONE)
			//return FALSE;
		
		smRender.Init();
		smMaterialGroup->smMaterial[AnimDataMgr.ImageDataBuffer[AniDataIndex].GetMaterialNum()].TwoSide = TRUE;
		smRender.SetMaterialGroup(smMaterialGroup);
		
		//현재 찍을 Frame을 세팅한다..
		smMaterialGroup->SetMaterialAnimFrame( &smMaterial[AnimDataMgr.ImageDataBuffer[AnimDataMgr.SequenceDataBuffer[AniDataIndex].DataFileIndex].GetMaterialNum()], 
		AnimDataMgr.SequenceDataBuffer[AniDataIndex].AnimFrame[CurrentFrame].ImageNum);
		//Camera각도 입력.
		smRender.SetCameraPosi(x, y, z, ax, ay, az);
		
		
		//if(RenderType == RENDER_BILLBOARD)
		//{
		
			
			Face2d.x = int(WorldX+LocalX);
			Face2d.y = int(WorldY+LocalY);
			Face2d.z = int(WorldZ+LocalZ);

			Face2d.r = (int)CurrentBlendValue;
			Face2d.g = (int)CurrentBlendValue;
			Face2d.b = (int)CurrentBlendValue;
			
			Face2d.Transparency = (int)CurrentBlendValue; 
			if(smRender.AddFace2D(&Face2d))
			{
				smRender.SetClipStates(SMCLIP_LEFT | SMCLIP_RIGHT | SMCLIP_TOP | SMCLIP_BOTTOM);
				smRender.ClipRendFace();
				smRender.GeomVertex2D(FALSE);
				smRender.RenderD3D();
			}
		//}

		/*
		else if(RenderType == RENDER_FACEPATH)
		{
			
			smMaterialGroup->smMaterial[AnimDataMgr.ImageDataBuffer[AnimDataMgr.SequenceDataBuffer[AniDataIndex].DataFileIndex].GetMaterialNum()].TwoSide = TRUE;
			CreatePathFace();

			smRender.InitStageMesh(Vertex, SumCount);
			int index; for( index = 0; index < FaceCount; index++)
				smRender.AddStageFace(&Face[index]);
				
			smRender.SetClipStates( SMCLIP_LEFT | SMCLIP_RIGHT | SMCLIP_TOP | SMCLIP_BOTTOM);
			smRender.ClipRendFace();
			smRender.GeomVertex2D(FALSE);
			smRender.RenderD3D();
			
			SumCount++;
			
		}
		*/
	//}

	//return TRUE;
//}

/*
int HoEffectParticle::Main()
{
	if(StartFlag)
	{
		if(AniType == ANI_NONE)
			return FALSE;
		
		//Animation 한번...
		if(CurrentFrame < AnimDataMgr.SequenceDataBuffer[AniDataIndex].AnimFrameCount)
		{
			if(TimeCount >= AnimDataMgr.SequenceDataBuffer[AniDataIndex].AnimFrame[CurrentFrame].Delay)
			{
				TimeCount = 0;
				if(AniType == ANI_ONE) //Animation 한번..
				{
					if(CurrentFrame >= AnimDataMgr.SequenceDataBuffer[AniDataIndex].AnimFrameCount-1)
						Init();
					else
						CurrentFrame++;
				}
				else if(AniType == ANI_LOOP) //Animation 루프...
				{
					if(CurrentFrame >= AnimDataMgr.SequenceDataBuffer[AniDataIndex].AnimFrameCount-2)
						CurrentFrame = 1;
					else
						CurrentFrame++;
				}

				//Blend 값 설정...
				if(CurrentFrame <= AnimDataMgr.SequenceDataBuffer[AniDataIndex].AnimFrameCount-1)
				{
					BlendStep = (AnimDataMgr.SequenceDataBuffer[AniDataIndex].AnimFrame[CurrentFrame].AlphaValue-CurrentBlendValue)/
						AnimDataMgr.SequenceDataBuffer[AniDataIndex].AnimFrame[CurrentFrame].Delay;
				}
			}
			TimeCount++;
		}
		
		LocalX += Xv;
		LocalY += Yv;
		LocalZ += Zv;
		
		Xv += Wind;
		Yv += Gravity;
		
		Live--;
		if(Live <= 0)
		{
			Init();
		}
		CurrentBlendValue = CurrentBlendValue+BlendStep;
		
	}
	return TRUE;
}

//BillBoard용 파티클...
int HoEffectParticle::StartBillBoard(int x, int y, int z, float xv, float yv, float zv,char *iniName, int aniType)
{
	SizeWidth = 7;
	SizeHeight = 7;
	HoPrimitiveBillboard::StartBillRect(x, y, z, (int)SizeWidth, (int)SizeHeight,iniName, aniType);
	StartFlag = TRUE;
	
	Xv = xv;
	Yv = yv;
	Zv = zv;
	
	return TRUE;
}

//삼각형 모양 폴리곤 날리기...
int HoEffectParticle::StartWorldTri(int x, int y, int z, int xv, int yv, int zv, char *iniName, int aniType)
{
	if(StartFlag)
		return FALSE;
	Move(x, y, z);
	
	//RenderType		= RENDER_FACEPATH;
	StartFlag		= TRUE;
	
	AniType			= aniType;
	AniDataIndex = AnimDataMgr.GetSearchSequenceData(iniName);
	
	Xv = (float)xv;
	Yv = (float)yv;
	Zv = (float)zv;
	return TRUE;
}
*/

HoEffectTime::HoEffectTime()
{
	DelayTimeCount = 0;
	StartFlag = FALSE;
	memset(TimeObjectBuffer, 0, sizeof(TimeObjectBuffer));
	memset(StartTime, 0, sizeof(StartTime));
}

HoEffectTime::~HoEffectTime()
{
	StartFlag = FALSE;
	memset(TimeObjectBuffer, 0, sizeof(TimeObjectBuffer));
	memset(StartTime, 0, sizeof(StartTime));
}

void HoEffectTime::Init()
{
	StartFlag = FALSE;
	DelayTimeCount = 0;
	int index; for( index = 0; index < MAX_TIMEOBJECT_BUFFER; index++)
	{
		if(TimeObjectBuffer[index] != NULL)
		{
			//EnterCriticalSection(&cLoadSection);
			delete TimeObjectBuffer[index];
			StartTime[index] = -1;
			TimeObjectBuffer[index] = NULL;
			//LeaveCriticalSection(&cLoadSection);
		}
	}

}

void HoEffectTime::Start()
{
	StartFlag = TRUE;
}



//이펙트가 여러게 겹치는 경우의 이펙트처리..
//ktj : 모든 이펙을 여기서 그리는지알수없슴.????????????????
void HoEffectTime::Draw(int x, int y, int z, int ax, int ay, int az)
{
	if(StartFlag)
	{
		int index; for( index = 0; index < MAX_TIMEOBJECT_BUFFER; index++)
		{
			if(TimeObjectBuffer[index] != NULL)
			{
				if(StartTime[index] < DelayTimeCount)
				{
					TimeObjectBuffer[index]->Draw(x, y, z, ax, ay, az);
				}
			}
		}
	}
}

void HoEffectTime::Main()
{
	if(StartFlag)
	{
		DelayTimeCount++;
		int liveObject = 0;
		
		int index; for( index = 0; index < MAX_TIMEOBJECT_BUFFER; index++)
		{
			if(TimeObjectBuffer[index] != NULL)
			{
				if(TimeObjectBuffer[index]->GetState())
				{
					liveObject++;
				
					if(StartTime[index] < DelayTimeCount)
					{
						TimeObjectBuffer[index]->Main();
					}
				}

			}
		}
		
		if(liveObject <= 0)
		{
			Init();
		}
	}
}

bool HoEffectTime::AddObject(HoEffectObject* object, int startTime)
{
	if(object == NULL)
		return false;

	int index; for( index = 0; index < MAX_TIMEOBJECT_BUFFER; index++)
	{
		if(TimeObjectBuffer[index] == NULL)
		{
			TimeObjectBuffer[index] = object;
			StartTime[index] = startTime;
			return true;
		}
	}
	return false;
}

//SetDynLight( int x,int y,int z, int r, int g, int b, int a, int power , int DecPower )

HoEffectLight::HoEffectLight()
{
	StartFlag = false;
}

HoEffectLight::~HoEffectLight()
{
	StartFlag = false;
}

int HoEffectLight::Init()
{
	StartFlag = FALSE;
	return TRUE;
}

int HoEffectLight::Main()
{
	if(StartFlag)
	{
		//SetDynLight( int x,int y,int z, int r, int g, int b, int a, int power , int DecPower )

		if(DecPower > 0)
			SetDynLight( int(WorldX+LocalX), int(WorldY+LocalY), int(WorldZ+LocalZ), R, G, B, A, Power, DecPower);
		else
			SetDynLight( int(WorldX+LocalX), int(WorldY+LocalY), int(WorldZ+LocalZ), R, G, B, A, Power, DecPower);//, EndPowerUp);
		StartFlag = FALSE;
	}
	return TRUE;
}


int HoEffectLight::Start(int x, int y, int z, int r, int g, int b, int a, int power, int decPower, int endPowerUp)
{
	if(GetState())
		return FALSE;

	SetState(TRUE);
	
	WorldX = (float)x;
	WorldY = (float)y;
	WorldZ = (float)z;

	LocalX = 0;
	LocalY = 0;
	LocalZ = 0;

	R = r;
	G = g;
	B = b;
	A = a;

	Power = power;
	DecPower = decPower;
	EndPowerUp = endPowerUp;
	return TRUE;
}



HoEffectMgr::HoEffectMgr()
{
	memset(EffectObjectBuffer, 0, sizeof(EffectObjectBuffer));
	memset(EffectZSortBuffer, 0, sizeof(EffectZSortBuffer));
	memset(MaterialNum, 0, sizeof(MaterialNum));
	memset(PatObj, 0, sizeof(PatObj));
	memset(EffectTimeObjectBuffer, 0, sizeof(EffectTimeObjectBuffer));
}

HoEffectMgr::~HoEffectMgr()
{
	int index; for( index = 0; index < MAX_EFFECTOBJECT_BUFFER; index++)
	{
		if(EffectZSortBuffer[index] != NULL)
		{
			delete EffectZSortBuffer[index];
			EffectZSortBuffer[index] = NULL;
		}
	}

	for(index = 0; index < MAX_EFFECTOBJECT_BUFFER; index++)
	{
		if(EffectObjectBuffer[index] != NULL)
		{
			delete EffectObjectBuffer[index];
			EffectObjectBuffer[index] = NULL;
		}
	}
	
	for(index = 0; index < MAX_EFFECTOBJECT_BUFFER; index++)
	{
		if(EffectTimeObjectBuffer[index].GetState())
		{
			EffectTimeObjectBuffer[index].Init();
		}
	}

	for(index = 0; index < MAX_OBJECT_NUM; index++)
	{
		if(PatObj[index] != NULL)
		PatObj[index]->Close();
	}
}


hoChrInfoObject ChrInEffectObject[CHR_IN_EFFECT_OBJECT_MAX];
void HoEffectMgr::Init()
{
	memset(ChrInEffectObject, 0, sizeof(ChrInEffectObject));

	int index; for( index = 0; index < MAX_EFFECTOBJECT_BUFFER; index++)
	{
		if(EffectZSortBuffer[index] != NULL)
		{
			delete EffectZSortBuffer[index];
			EffectZSortBuffer[index] = NULL;
		}
	}

	for(index = 0; index < MAX_EFFECTOBJECT_BUFFER; index++)
	{
		if(EffectObjectBuffer[index] != NULL)
		{
			delete EffectObjectBuffer[index];
			EffectObjectBuffer[index] = NULL;
		}
	}
	
	for(index = 0; index < MAX_EFFECTOBJECT_BUFFER; index++)
	{
		if(EffectTimeObjectBuffer[index].GetState())
		{
			EffectTimeObjectBuffer[index].Init();
		}
	}

	for(index = 0; index < MAX_OBJECT_NUM; index++)
	{
		if(PatObj[index] != NULL)
		PatObj[index]->Close();
	}

	if(PatMissile != NULL)
		PatMissile->Close();
	//int index;
	//계속 생성되는 파티클 소스 읽기...
	index = 0;
	MaterialNum[index] = CreateTextureMaterial( "Effect\\ImageData\\Particle\\dust.tga" ,0 , 0 , 0 ,0 , SMMAT_BLEND_LAMP );
	index++;
	
	MaterialNum[index] = CreateTextureMaterial( "Effect\\ImageData\\Particle\\bubble.tga" ,0 , 0 , 0 ,0 , SMMAT_BLEND_LAMP );
	index++;

	MaterialNum[index] = CreateTextureMaterial( "Effect\\ImageData\\Particle\\Red.tga" ,0 , 0 , 0 ,0 , SMMAT_BLEND_LAMP );
	index++;
	
	MaterialNum[index] = CreateTextureMaterial( "Effect\\ImageData\\Particle\\Blue.tga" ,0 , 0 , 0 ,0 , SMMAT_BLEND_LAMP );
	index++;
	
	MaterialNum[index] = CreateTextureMaterial( "Effect\\ImageData\\Particle\\cc.tga" ,0 , 0 , 0 ,0 , SMMAT_BLEND_LAMP );
	index++;
	
	MaterialNum[index] = CreateTextureMaterial( "Effect\\ImageData\\Particle\\Fire.tga" ,0 , 0 , 0 ,0 , SMMAT_BLEND_LAMP );
	index++;
	
	MaterialNum[index] = CreateTextureMaterial( "Effect\\ImageData\\Particle\\flare.tga" ,0 , 0 , 0 ,0 , SMMAT_BLEND_LAMP );
	index++;


	//ktj : 삽입	//index=7 : 블랙홀모양
	MaterialNum[index] = CreateTextureMaterial( "Effect\\ImageData\\Particle\\bkh0.tga" ,0 , 0 , 0,0 , SMMAT_BLEND_SHADOW );
	index++;

	//ktj : 삽입	//index=8 : 번개(a flash of lighting)용 그림.
	MaterialNum[index] = CreateTextureMaterial( "Effect\\ImageData\\Particle\\Red_KTJimsi.tga" ,0 , 0 , 0,0 , SMMAT_BLEND_SHADOW );
	index++;


	//Game Start0
	index = 0;
	char buffer[256];
	memset(buffer, 0, sizeof(buffer));
	GetPrivateProfileString("3DANIMATION", "DataName", NULL, buffer,sizeof(buffer), "Effect\\ObjAnimationData\\GameStart\\GameStart1.ini");
	PatAnimationEnd[index] = GetPrivateProfileInt("3DANIMATION","AnimationEnd",0,"Effect\\ObjAnimationData\\GameStart\\GameStart1.ini");
	smASE_SetPhysique(NULL);
	PatObj[index] = smASE_Read(buffer);
	PatObj[index]->ZeroNormals();
	PatObj[index]->smMaterialGroup->smMaterial[0].BlendType = SMMAT_BLEND_LAMP;
	index++;
	
	//Ground Pike1
	memset(buffer, 0, sizeof(buffer));
	GetPrivateProfileString("3DANIMATION", "DataName", NULL, buffer,sizeof(buffer), "Effect\\ObjAnimationData\\GroundPike\\GroundPike.ini");
	PatAnimationEnd[index] = GetPrivateProfileInt("3DANIMATION","AnimationEnd",0,"Effect\\ObjAnimationData\\GroundPIke\\GroundPike.ini");
	smASE_SetPhysique(NULL);
	PatObj[index] = smASE_Read(buffer);
	PatObj[index]->ZeroNormals();
	//PatObj[index]->smMaterialGroup->smMaterial[0].BlendType = SMMAT_BLEND_LAMP;
	index++;
	
	//Physical Absorb2
	memset(buffer, 0, sizeof(buffer));
	GetPrivateProfileString("3DANIMATION", "DataName", NULL, buffer,sizeof(buffer), "Effect\\ObjAnimationData\\PhysicalAbsorb\\PhysicalAbsorb.ini");
	PatAnimationEnd[index] = GetPrivateProfileInt("3DANIMATION","AnimationEnd",0,"Effect\\ObjAnimationData\\PhysicalAbsorb\\PhysicalAbsorb.ini");
	smASE_SetPhysique(NULL);
	PatObj[index] = smASE_Read(buffer);
	PatObj[index]->ZeroNormals();
	PatObj[index]->smMaterialGroup->smMaterial[0].BlendType = SMMAT_BLEND_LAMP;
	index++;
	

	//Pike Wind3
	memset(buffer, 0, sizeof(buffer));
	GetPrivateProfileString("3DANIMATION", "DataName", NULL, buffer,sizeof(buffer), "Effect\\ObjAnimationData\\PikeWind\\bong.ini");
	PatAnimationEnd[index] = GetPrivateProfileInt("3DANIMATION","AnimationEnd",0,"Effect\\ObjAnimationData\\PikeWind\\bong.ini");
	smASE_SetPhysique(NULL);
	PatObj[index] = smASE_Read(buffer);
	PatObj[index]->ZeroNormals();
	int i; for( i = 0; i< int(PatObj[index]->smMaterialGroup->MaterialCount); i++)
		PatObj[index]->smMaterialGroup->smMaterial[i].BlendType = SMMAT_BLEND_LAMP;
	
	index++;

	//Scout Hawk4,5
	PatObj[index] = smASE_ReadBone( "Effect\\ObjAnimationData\\ScoutHawk\\Hawk-ani.ASE");		// 애니메이션 읽기
	smASE_SetPhysique( PatObj[index]);
	index++;

	PatObj[index] = smASE_Read("Effect\\ObjAnimationData\\ScoutHawk\\Hawk-ani.ASE" , 0);
	PatObj[index]->ZeroNormals();
	
	smASE_SetPhysique(0);
	PatAnimationEnd[index] = 120;
	index++;
	
	//WindArrow6
	memset(buffer, 0, sizeof(buffer));
	GetPrivateProfileString("3DANIMATION", "DataName", NULL, buffer,sizeof(buffer), "Effect\\ObjAnimationData\\WindArrow\\WindArrow.ini");
	PatAnimationEnd[index] = GetPrivateProfileInt("3DANIMATION","AnimationEnd",0,"Effect\\ObjAnimationData\\WindArrow\\WindArrow.ini");
	smASE_SetPhysique(NULL);
	PatObj[index] = smASE_Read(buffer);
	PatObj[index]->ZeroNormals();
	for(i = 0; i< int(PatObj[index]->smMaterialGroup->MaterialCount); i++)
		PatObj[index]->smMaterialGroup->smMaterial[i].BlendType = SMMAT_BLEND_LAMP;
	index++;
	
	//ArrowRage7
	memset(buffer, 0, sizeof(buffer));
	GetPrivateProfileString("3DANIMATION", "DataName", NULL, buffer,sizeof(buffer), "Effect\\ObjAnimationData\\ArrowRage\\ArrowRage.ini");
	PatAnimationEnd[index] = GetPrivateProfileInt("3DANIMATION","AnimationEnd",0,"Effect\\ObjAnimationData\\ArrowRage\\ArrowRage.ini");
	smASE_SetPhysique(NULL);
	PatObj[index] = smASE_Read(buffer);
	PatObj[index]->ZeroNormals();
	for(i = 0; i< int(PatObj[index]->smMaterialGroup->MaterialCount); i++)
		PatObj[index]->smMaterialGroup->smMaterial[i].BlendType = SMMAT_BLEND_LAMP;
	index++;

	//Test8
	memset(buffer, 0, sizeof(buffer));
	GetPrivateProfileString("3DANIMATION", "DataName", NULL, buffer,sizeof(buffer), "Effect\\ObjAnimationData\\test\\test.ini");
	PatAnimationEnd[index] = GetPrivateProfileInt("3DANIMATION","AnimationEnd",0,"Effect\\ObjAnimationData\\test\\test.ini");
	smASE_SetPhysique(NULL);
	PatObj[index] = smASE_Read(buffer);
	PatObj[index]->ZeroNormals();
	for(i = 0; i< int(PatObj[index]->smMaterialGroup->MaterialCount); i++)
		PatObj[index]->smMaterialGroup->smMaterial[i].BlendType = SMMAT_BLEND_LAMP;
	index++;
	

	//메카닉 범 폭탄..9
	memset(buffer, 0, sizeof(buffer));
	GetPrivateProfileString("3DANIMATION", "DataName", NULL, buffer,sizeof(buffer), "Effect\\ObjAnimationData\\MechanicBomb\\MechanicBomb.ini");
	PatAnimationEnd[index] = GetPrivateProfileInt("3DANIMATION","AnimationEnd",0,"Effect\\ObjAnimationData\\MechanicBomb\\MechanicBomb.ini");
	smASE_SetPhysique(NULL);
	PatObj[index] = smASE_Read(buffer);
	PatObj[index]->ZeroNormals();
	for(i = 0; i< int(PatObj[index]->smMaterialGroup->MaterialCount); i++)
		PatObj[index]->smMaterialGroup->smMaterial[i].BlendType = SMMAT_BLEND_LAMP;
	index++;

	//SkillRoar..10
	memset(buffer, 0, sizeof(buffer));
	GetPrivateProfileString("3DANIMATION", "DataName", NULL, buffer,sizeof(buffer), "Effect\\ObjAnimationData\\Roar\\Roar.ini");
	PatAnimationEnd[index] = GetPrivateProfileInt("3DANIMATION","AnimationEnd",0,"Effect\\ObjAnimationData\\Roar\\Roar.ini");
	smASE_SetPhysique(NULL);
	PatObj[index] = smASE_Read(buffer);
	PatObj[index]->ZeroNormals();
	for(i = 0; i< int(PatObj[index]->smMaterialGroup->MaterialCount); i++)
		PatObj[index]->smMaterialGroup->smMaterial[i].BlendType = SMMAT_BLEND_LAMP;
	index++;

	//MonsterWeb..11
	memset(buffer, 0, sizeof(buffer));
	GetPrivateProfileString("3DANIMATION", "DataName", NULL, buffer,sizeof(buffer), "Effect\\ObjAnimationData\\MonsterWeb\\WebShot.ini");
	PatAnimationEnd[index] = GetPrivateProfileInt("3DANIMATION","AnimationEnd",0,"Effect\\ObjAnimationData\\MonsterWeb\\WebShot.ini");
	smASE_SetPhysique(NULL);
	PatObj[index] = smASE_Read(buffer);
	PatObj[index]->ZeroNormals();
	for(i = 0; i< int(PatObj[index]->smMaterialGroup->MaterialCount); i++)
		PatObj[index]->smMaterialGroup->smMaterial[i].BlendType = SMMAT_BLEND_LAMP;
	index++;
	
	//MonsterWeb..12
	memset(buffer, 0, sizeof(buffer));
	GetPrivateProfileString("3DANIMATION", "DataName", NULL, buffer,sizeof(buffer), "Effect\\ObjAnimationData\\MonsterWeb\\WebHit.ini");
	PatAnimationEnd[index] = GetPrivateProfileInt("3DANIMATION","AnimationEnd",0,"Effect\\ObjAnimationData\\MonsterWeb\\WebHit.ini");
	smASE_SetPhysique(NULL);
	PatObj[index] = smASE_Read(buffer);
	PatObj[index]->ZeroNormals();
	
	for(i = 0; i< int(PatObj[index]->smMaterialGroup->MaterialCount); i++)
		PatObj[index]->smMaterialGroup->smMaterial[i].BlendType = SMMAT_BLEND_LAMP;
	index++;
	
	//Avalanche 13
	memset(buffer, 0, sizeof(buffer));
	GetPrivateProfileString("3DANIMATION", "DataName", NULL, buffer,sizeof(buffer), "Effect\\ObjAnimationData\\Avalanche\\avalanche.ini");
	PatAnimationEnd[index] = GetPrivateProfileInt("3DANIMATION","AnimationEnd",0,"Effect\\ObjAnimationData\\Avalanche\\avalanche.ini");
	smASE_SetPhysique(NULL);
	PatObj[index] = smASE_Read(buffer);
	PatObj[index]->ZeroNormals();
	
	for(i = 0; i< int(PatObj[index]->smMaterialGroup->MaterialCount); i++)
		PatObj[index]->smMaterialGroup->smMaterial[i].BlendType = SMMAT_BLEND_LAMP;
	index++;
	
	
	//Falcon 14, 15
	PatObj[index] = smASE_ReadBone( "Effect\\ObjAnimationData\\Falcon\\FalconAni.ase");		// 애니메이션 읽기
	smASE_SetPhysique( PatObj[index]);
	index++;
	
	PatObj[index] = smASE_Read("Effect\\ObjAnimationData\\Falcon\\FalconAni.ase" , 0);
	PatObj[index]->ZeroNormals();
	smASE_SetPhysique(0);
	PatAnimationEnd[index] = 210;

	for(i = 0; i< int(PatObj[index]->smMaterialGroup->MaterialCount); i++)
		PatObj[index]->smMaterialGroup->smMaterial[i].BlendType = SMMAT_BLEND_LAMP;

	index++;
	
	//Brutal 16
	memset(buffer, 0, sizeof(buffer));
	GetPrivateProfileString("3DANIMATION", "DataName", NULL, buffer,sizeof(buffer), "Effect\\ObjAnimationData\\BrutalSwing\\BrutalSwing1.ini");
	PatAnimationEnd[index] = GetPrivateProfileInt("3DANIMATION","AnimationEnd",0,"Effect\\ObjAnimationData\\BrutalSwing\\BrutalSwing1.ini");
	smASE_SetPhysique(NULL);
	PatObj[index] = smASE_Read(buffer);
	PatObj[index]->ZeroNormals();
	
	for(i = 0; i< int(PatObj[index]->smMaterialGroup->MaterialCount); i++)
		PatObj[index]->smMaterialGroup->smMaterial[i].BlendType = SMMAT_BLEND_LAMP;
	index++;

	//Brutal 17
	memset(buffer, 0, sizeof(buffer));
	GetPrivateProfileString("3DANIMATION", "DataName", NULL, buffer,sizeof(buffer), "Effect\\ObjAnimationData\\BrutalSwing\\BrutalSwing2.ini");
	PatAnimationEnd[index] = GetPrivateProfileInt("3DANIMATION","AnimationEnd",0,"Effect\\ObjAnimationData\\BrutalSwing\\BrutalSwing2.ini");
	smASE_SetPhysique(NULL);
	PatObj[index] = smASE_Read(buffer);
	PatObj[index]->ZeroNormals();
	for(i = 0; i< int(PatObj[index]->smMaterialGroup->MaterialCount); i++)
		PatObj[index]->smMaterialGroup->smMaterial[i].BlendType = SMMAT_BLEND_LAMP;

	index++;

	//MonsterHeadCutter 18..
	memset(buffer, 0, sizeof(buffer));
	GetPrivateProfileString("3DANIMATION", "DataName", NULL, buffer,sizeof(buffer), "Effect\\ObjAnimationData\\MonsterHeadCutter\\HeadCutterHit1.ini");
	PatAnimationEnd[index] = GetPrivateProfileInt("3DANIMATION","AnimationEnd",0,"Effect\\ObjAnimationData\\MonsterHeadCutter\\HeadCutterHit1.ini");
	smASE_SetPhysique(NULL);
	PatObj[index] = smASE_Read(buffer);
	PatObj[index]->ZeroNormals();
	for(i = 0; i< int(PatObj[index]->smMaterialGroup->MaterialCount); i++)
		PatObj[index]->smMaterialGroup->smMaterial[i].BlendType = SMMAT_BLEND_LAMP;
	index++;
	
	/*
	//Fury Magic 19
	memset(buffer, 0, sizeof(buffer));
	GetPrivateProfileString("3DANIMATION", "DataName", NULL, buffer,sizeof(buffer), "Effect\\ObjAnimationData\\MonsterFury\\FuryMagic.ini");
	PatAnimationEnd[index] = GetPrivateProfileInt("3DANIMATION","AnimationEnd",0,"Effect\\ObjAnimationData\\MonsterFury\\FuryMagic.ini");
	smASE_SetPhysique(NULL);
	PatObj[index] = smASE_Read(buffer);
	PatObj[index]->ZeroNormals();
	for(i = 0; i< int(PatObj[index]->smMaterialGroup->MaterialCount); i++)
		PatObj[index]->smMaterialGroup->smMaterial[i].BlendType = SMMAT_BLEND_LAMP;
	index++;

	
	//Fury Head 20
	memset(buffer, 0, sizeof(buffer));
	GetPrivateProfileString("3DANIMATION", "DataName", NULL, buffer,sizeof(buffer), "Effect\\ObjAnimationData\\MonsterFury\\FuryHead.ini");
	PatAnimationEnd[index] = GetPrivateProfileInt("3DANIMATION","AnimationEnd",0,"Effect\\ObjAnimationData\\MonsterFury\\FuryHead.ini");
	smASE_SetPhysique(NULL);
	PatObj[index] = smASE_Read(buffer);
	PatObj[index]->ZeroNormals();
	for(i = 0; i< int(PatObj[index]->smMaterialGroup->MaterialCount); i++)
		PatObj[index]->smMaterialGroup->smMaterial[i].BlendType = SMMAT_BLEND_LAMP;
	index++;
	*/
	
	index++;
	index++;

	//Head Cutter 21 Hit2
	memset(buffer, 0, sizeof(buffer));
	GetPrivateProfileString("3DANIMATION", "DataName", NULL, buffer,sizeof(buffer), "Effect\\ObjAnimationData\\MonsterHeadCutter\\HeadCutterHit2.ini");
	PatAnimationEnd[index] = GetPrivateProfileInt("3DANIMATION","AnimationEnd",0,"Effect\\ObjAnimationData\\MonsterHeadCutter\\HeadCutterHit2.ini");
	smASE_SetPhysique(NULL);
	PatObj[index] = smASE_Read(buffer);
	PatObj[index]->ZeroNormals();
	for(i = 0; i< int(PatObj[index]->smMaterialGroup->MaterialCount); i++)
		PatObj[index]->smMaterialGroup->smMaterial[i].BlendType = SMMAT_BLEND_LAMP;
	index++;
	
	

	//OmiCron 22 Hit1
	memset(buffer, 0, sizeof(buffer));
	GetPrivateProfileString("3DANIMATION", "DataName", NULL, buffer,sizeof(buffer), "Effect\\ObjAnimationData\\OmiCron\\omicronHit1.ini");
	PatAnimationEnd[index] = GetPrivateProfileInt("3DANIMATION","AnimationEnd",0,"Effect\\ObjAnimationData\\OmiCron\\omicronHit1.ini");
	smASE_SetPhysique(NULL);
	PatObj[index] = smASE_Read(buffer);
	PatObj[index]->ZeroNormals();
	index++;
	
	//StygianLord 23 Shot1
	memset(buffer, 0, sizeof(buffer));
	GetPrivateProfileString("3DANIMATION", "DataName", NULL, buffer,sizeof(buffer), "Effect\\ObjAnimationData\\Stygianlord\\StygianlordShot1.ini");
	PatAnimationEnd[index] = GetPrivateProfileInt("3DANIMATION","AnimationEnd",0,"Effect\\ObjAnimationData\\Stygianlord\\StygianlordShot1.ini");
	smASE_SetPhysique(NULL);
	PatObj[index] = smASE_Read(buffer);
	PatObj[index]->ZeroNormals();
	index++;
	

	//ShadowShot1 24
	memset(buffer, 0, sizeof(buffer));
	GetPrivateProfileString("3DANIMATION", "DataName", NULL, buffer,sizeof(buffer), "Effect\\ObjAnimationData\\Shadow\\ShadowShot1.ini");
	PatAnimationEnd[index] = GetPrivateProfileInt("3DANIMATION","AnimationEnd",0,"Effect\\ObjAnimationData\\Shadow\\ShadowShot1.ini");
	smASE_SetPhysique(NULL);
	PatObj[index] = smASE_Read(buffer);
	PatObj[index]->ZeroNormals();
	for(i = 0; i< int(PatObj[index]->smMaterialGroup->MaterialCount); i++)
	{
		PatObj[index]->smMaterialGroup->smMaterial[i].BlendType = SMMAT_BLEND_SHADOW;
		PatObj[index]->smMaterialGroup->smMaterial[i].SelfIllum = TRUE;
	}
	index++;
	
	//Scout Hawk 25, 26
	//PatObj[index] = smASE_ReadBone( "Effect\\ObjAnimationData\\GoldenFalcon\\g-falcon01.ASE");		// 애니메이션 읽기
	PatObj[index] = smASE_ReadBone( "Effect\\ObjAnimationData\\Falcon\\FalconAni.ase");		// 애니메이션 읽기
	smASE_SetPhysique( PatObj[index]);
	index++;

	PatObj[index] = smASE_Read("Effect\\ObjAnimationData\\GoldenFalcon\\g-falcon01.ASE" , 0);
	PatObj[index]->ZeroNormals();
	
	smASE_SetPhysique(0);
	PatAnimationEnd[index] = 120;
	
	smASE_SetPhysique(NULL);
	PatMissile = smASE_Read("Effect\\ObjAnimationData\\Missile\\Mis.ase");
	PatMissile->ZeroNormals();
	
	ReadTextures();
}

int HoEffectMgr::EmptyObjectBufferIndex()
{
	int index; for( index = 0; index < MAX_EFFECTOBJECT_BUFFER; index++)
	{
		if(EffectObjectBuffer[index] == NULL)
			return index;
	}
	return -1;
}

int HoEffectMgr::EmptyZSortBufferIndex()
{
	int index; for( index =0; index < MAX_EFFECTOBJECT_BUFFER; index++)
	{
		if(EffectZSortBuffer[index] == NULL)
			return index;
	}
	return -1;
}

int HoEffectMgr::EmptyTimeObjectBufferIndex()
{
	int index; for( index = 0; index < MAX_EFFECTOBJECT_BUFFER; index++)
	{
		
		if(EffectTimeObjectBuffer[index].GetState() == FALSE)
		{
			EffectTimeObjectBuffer[index].Init();
			return index;
		}
	}
	return -1;
}

int HoEffectMgr::Main()
{
	
	int index; for( index = 0; index < MAX_EFFECTOBJECT_BUFFER; index++)
	{
		if(EffectObjectBuffer[index] != NULL)
		{
			if(EffectObjectBuffer[index]->GetState())
				EffectObjectBuffer[index]->Main();
			
			if(EffectObjectBuffer[index]->GetState() == FALSE)
			{
				//EnterCriticalSection(&cLoadSection);
				delete EffectObjectBuffer[index];
				EffectObjectBuffer[index] = NULL;
				//LeaveCriticalSection(&cLoadSection);
			}
		}
	}
	
	for(index = 0; index < MAX_EFFECTOBJECT_BUFFER; index++)
	{
		if(EffectZSortBuffer[index] != NULL)
		{
			if(EffectZSortBuffer[index]->GetState())
				EffectZSortBuffer[index]->Main();
			
			if(EffectZSortBuffer[index]->GetState() == FALSE)
			{
				//EnterCriticalSection(&cLoadSection);
				delete EffectZSortBuffer[index];
				EffectZSortBuffer[index] = NULL;
				//LeaveCriticalSection(&cLoadSection);
			}
		}
	}
	
	//TimeEffect
	for(index = 0; index < MAX_EFFECTOBJECT_BUFFER; index++)
	{
		if(EffectTimeObjectBuffer[index].GetState())
		{
			EffectTimeObjectBuffer[index].Main();
		}
	}

	return TRUE;
}

//소트
struct EFFECT_ZDEPTH
{
	int		ObjIndex;
	int		ZDepth;
};


EFFECT_ZDEPTH EffectObjDepth[MAX_EFFECTOBJECT_BUFFER];
void BubbleSort(EFFECT_ZDEPTH *list, int num)
{
	EFFECT_ZDEPTH temp;
	int i,j;

	for(i = num; i > 0; i--)
	{
		for(j = 0; j < i; j++)
		{
			if(list[j].ZDepth < list[j+1].ZDepth)
			{
				temp = list[j+1];
				list[j+1] = list[j];
				list[j] = temp;
			}
		}
	}
}

//================================================================
//ktj 
//임시 이펙트 한번만 나오고 마는 이펙트들(캐릭터선택등..)인것으로 생각됨
//================================================================
int HoEffectMgr::Draw(int x, int y, int z, int ax, int ay, int az)
{
	// Setup non-texture render states
	lpD3DDevice->SetRenderState(D3DRENDERSTATE_DITHERENABLE, FALSE);
	lpD3DDevice->SetRenderState(D3DRENDERSTATE_SPECULARENABLE, FALSE);
    lpD3DDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, FALSE);
	//lpD3DDevice->SetRenderState(D3DRENDERSTATE_ZENABLE, FALSE);
	lpD3DDevice->SetRenderState(D3DRENDERSTATE_CULLMODE, D3DCULL_NONE);
	
	
	int objCount = 0;
	int index; for( index = 0; index < MAX_EFFECTOBJECT_BUFFER; index++)
	{
		if(EffectZSortBuffer[index] != NULL)
		{
			if(EffectZSortBuffer[index]->GetState())
			{
				//Z축 정렬...
				EffectObjDepth[index].ObjIndex = index;
				POINT3D pos = EffectZSortBuffer[index]->GetPos();
				EffectObjDepth[index].ZDepth = smRender.GetDepthZ(pos.x, pos.y, pos.z);
				objCount++;
			}
		}
	}
	
	if(objCount > 0)
	{
		BubbleSort(EffectObjDepth, objCount);
		for(index = 0; index < objCount; index++)
		{
			if(EffectZSortBuffer[EffectObjDepth[index].ObjIndex] != NULL)
			{
				if(EffectZSortBuffer[EffectObjDepth[index].ObjIndex]->GetState())
					EffectZSortBuffer[EffectObjDepth[index].ObjIndex]->Draw(x, y, z, ax, ay, az);
			}
		}
	}
	
	//lpD3DDevice->SetRenderState(D3DRENDERSTATE_ZENABLE, TRUE );
	for(index = 0; index < MAX_EFFECTOBJECT_BUFFER; index++)
	{
		if(EffectObjectBuffer[index] != NULL)
		{
			if(EffectObjectBuffer[index]->GetState())
				EffectObjectBuffer[index]->Draw(x, y, z, ax, ay, az);
		}
	}
	
	//EffectTime...
	for(index = 0; index < MAX_EFFECTOBJECT_BUFFER; index++)
	{
		if(EffectTimeObjectBuffer[index].GetState())
			EffectTimeObjectBuffer[index].Draw(x, y, z, ax, ay, az);
	}
	
	lpD3DDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, TRUE);
	lpD3DDevice->SetRenderState( D3DRENDERSTATE_LASTPIXEL, FALSE );
	return TRUE;
}


int	HoEffectMgr::StartBillRectPrimitive(int x, int y, int z, int sizeX, int sizeY, char *iniName)
{
	int objIndex = EmptyZSortBufferIndex();
	if(objIndex == -1)
		return FALSE;
	
	HoPrimitiveBillboard *face = new HoPrimitiveBillboard();
	if(face != NULL)
	{
		face->StartBillRect(x, y, z, sizeX, sizeY, iniName, ANI_ONE);
		EffectZSortBuffer[objIndex] = (HoEffectObject *)face;
	}
	
	return TRUE;
}

int	HoEffectMgr::StartBillRectPrimitive(int x, int y, int z, char *iniName)
{
	//int objIndex = EmptyZSortBufferIndex();
	int objIndex = EmptyObjectBufferIndex();
	if(objIndex == -1)
		return FALSE;
	
	HoPrimitiveBillboard *face = new HoPrimitiveBillboard();

	if(face != NULL)
	{
		face->StartBillRect(x, y, z, iniName, ANI_ONE);
		//EffectZSortBuffer[objIndex] = (HoEffectObject *)face;
		EffectObjectBuffer[objIndex] = (HoEffectObject *)face;
	}
	
	return TRUE;
}

/*
int	HoEffectMgr::StartFaceUpCylinder(int x, int y, int z, int width, int termFrame, char *iniName)
{

	int effectId = EmptyObjectBufferIndex();
	if(effectId == -1)
		return -1;
	
	HoEffectCylinder *cylinder = new HoEffectCylinder();
	cylinder->StartAniHeight(x, y, z, width, termFrame,iniName, ANI_LOOP);
	EffectObjectBuffer[effectId] = (HoEffectObject *)cylinder;
	
	return effectId;
}
*/


int HoEffectMgr::StartSkillDest(int x, int y, int z, int destX, int destY, int destZ, int skillType, int level)
{
	switch(skillType)
	{
		//ktj : 새로 넣은부분 SKILL_TORNADO1의 색깔을 붉은색으로 바꾼것============================start
		case SKILL_TORNADO2:
		{
			int timeObjectIndex = EmptyTimeObjectBufferIndex();
			if(timeObjectIndex == -1)
				return FALSE;

			SetDynLight( x, y, z, 255, 100, 100, 255, 200 , 3);
			HoEffectHitLine *effectLine;
			//ktj : hologic.cpp에서 토네이도돌고난후 터지는 부분에 사용됨.
			effectLine = new HoEffectHitLine();
			effectLine->Start(x, y, z, destX, destY, destZ, 1);	//ktj : 1은 "SkillTornado2.ini";
			//ktj : 새로넣음.
			effectLine->Skill_n = SKILL_TORNADO2;

			if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(effectLine, 0))
			{
				delete effectLine;
				effectLine = NULL;
				return FALSE;
			}
			
			
			effectLine = new HoEffectHitLine();
			effectLine->Start(x, y+500, z, destX, destY+500, destZ, 1);	//ktj : 1은 "SkillTornado2.ini";
			//ktj : 새로넣음.
			effectLine->Skill_n = SKILL_TORNADO2;

			if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(effectLine, 2))
			{
				delete effectLine;
				effectLine = NULL;
				return FALSE;
			}
			
			EffectTimeObjectBuffer[timeObjectIndex].Start();
			break;
		}
		//==================================================end





		
		#ifdef Making_SKILL_TORNADO3
		//ktj : 새로 넣은부분 ==================================================start
		//ktj : 미완 번개용
		case SKILL_TORNADO3:
		{
			int timeObjectIndex = EmptyTimeObjectBufferIndex();
			if(timeObjectIndex == -1)
				return FALSE;

			SetDynLight( x, y, z, 255, 100, 100, 255, 200 , 3);
			HoEffectHitLine *effectLine;
			//ktj : hologic.cpp에서 토네이도돌고난후 터지는 부분에 사용됨.
			effectLine = new HoEffectHitLine();
			effectLine->Start(x, y, z, destX, destY, destZ, 2);	//ktj : 1은 "SkillTornado3.ini";
			//ktj : 새로넣음.
			effectLine->Skill_n = SKILL_TORNADO3;

			if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(effectLine, 0))
			{
				delete effectLine;
				effectLine = NULL;
				return FALSE;
			}
			
			
			
			
			//ktj : 일단 삭제함.
			//effectLine = new HoEffectHitLine();
			//effectLine->Start(x, y+500, z, destX, destY+500, destZ, 2);	//ktj : 1은 "SkillTornado3.ini";
			//ktj : 새로넣음.
			//effectLine->Skill_n = SKILL_TORNADO3;

			//if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(effectLine, 2))
			//{
			//	delete effectLine;
			//	effectLine = NULL;
			//	return FALSE;
			//}
			
		

			EffectTimeObjectBuffer[timeObjectIndex].Start();
			break;
		}
		//==================================================end
		#endif
		


		case SKILL_TORNADO:
		{
			int timeObjectIndex = EmptyTimeObjectBufferIndex();
			if(timeObjectIndex == -1)
				return FALSE;

			SetDynLight( x, y, z, 255, 100, 100, 255, 200 , 3);
			HoEffectHitLine *effectLine;
			//ktj : hologic.cpp에서 토네이도돌고난후 터지는 부분에 사용됨.
			effectLine = new HoEffectHitLine();
			effectLine->Start(x, y, z, destX, destY, destZ, 0);	//ktj : 0은 "SkillTornado.ini";
			//ktj : 새로넣음.
			effectLine->Skill_n = SKILL_TORNADO;

			if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(effectLine, 0))
			{
				delete effectLine;
				effectLine = NULL;
				return FALSE;
			}
			
			
			effectLine = new HoEffectHitLine();
			effectLine->Start(x, y+500, z, destX, destY+500, destZ, 0);	//ktj : 0은 "SkillTornado.ini";
			//ktj : 새로넣음.
			effectLine->Skill_n = SKILL_TORNADO;

			if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(effectLine, 2))
			{
				delete effectLine;
				effectLine = NULL;
				return FALSE;
			}
			

			EffectTimeObjectBuffer[timeObjectIndex].Start();
			break;
		}

		case SKILL_SPARK:
		{
			int timeObjectIndex = EmptyTimeObjectBufferIndex();
			if(timeObjectIndex == -1)
				return FALSE;
			
			HoEffectTracker *tracker = new HoEffectTracker();
			if(tracker != NULL)
			{
			
				POINT3D currentPos;
				POINT3D destPos;

				currentPos.x = x;
				currentPos.y = y;
				currentPos.z = z;
				
				destPos.x = destX;
				destPos.y = destY;
				destPos.z = destZ;
			
				tracker->Start(currentPos, destPos, skillType, NULL, level);
				
				if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(tracker, 0))
				{
					delete tracker;
					tracker = NULL;
					return FALSE;
				}

				EffectTimeObjectBuffer[timeObjectIndex].Start();
			}
			break;
		}

		

		/*
		case SKILL_HOMING:
		{
			int timeObjectIndex = EmptyTimeObjectBufferIndex();
			if(timeObjectIndex == -1)
				return FALSE;
			
			int index; for( index = 0; index < 4; index++)
			{

				HoEffectTracker *tracker = new HoEffectTracker();
				if(tracker != NULL)
				{
					POINT3D currentPos;
					POINT3D destPos;
					POINT3D	startVelocity;

					currentPos.x = x;
					currentPos.y = y;
					currentPos.z = z;

					destPos.x = destX;
					destPos.y = destY;
					destPos.z = destZ;
					
					
					startVelocity.x = 0;
					startVelocity.y = rand()%1000;
					startVelocity.z = rand()%2000-1000;
					
					HoParticleSystem *particleSystem = new HoParticleSystem();
					if(particleSystem != NULL)
					{
						particleSystem->Start(x, y, z, MaterialNum[1], 3);
						tracker->Start(currentPos, destPos, startVelocity, particleSystem);
						
						if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(tracker, index*10))
						{
							delete tracker;
							tracker = NULL;
							return FALSE;
						}
						EffectTimeObjectBuffer[timeObjectIndex].Start();
						EffectTimeObjectBuffer[timeObjectIndex].AddObject(particleSystem, index*10);
						EffectTimeObjectBuffer[timeObjectIndex].Start();
					}	
				}				
			}
			break;
		}
		*/


		

		case SKILL_METEO:
		{

			int timeObjectIndex = EmptyTimeObjectBufferIndex();
			if(timeObjectIndex == -1)
				return FALSE;
			
			int mapY1 = destY;
			int mapY2 = destY;
			//int mapY1, mapY2;
			if(smGameStage[0])
			{
				mapY1 = smGameStage[0]->GetHeight(destX, destZ);
			}
			if(smGameStage[1])
			{
				mapY2 = smGameStage[1]->GetHeight(destX, destZ);
			}

			if(smGameStage[0] && smGameStage[1])
				if (mapY1 < mapY2) mapY1 = mapY2;
			if(smGameStage[0] == NULL && smGameStage[1])
				mapY1 = mapY2;
			
			int index; for( index = 0; index < 3; index++)
			{
				HoEffectTracker *tracker = new HoEffectTracker();
				if(tracker != NULL)
				{
					int ang = ANGLE_360/3*index;
					POINT3D currentPos;
					POINT3D destPos;
					POINT3D	startVelocity;

					currentPos.x = destX+3000;
					currentPos.y = mapY1-5000+60000;
					currentPos.z = destZ+3000;
					
					destPos.x = destX+int((GetCos[ang]/65536.f)*(9000));
					destPos.y = mapY1-5000;
					destPos.z = destZ+int((GetSin[ang]/65536.f)*(9000));
					
					startVelocity.x = 0;
					startVelocity.y = 0;
					startVelocity.z = 0;
					
					HoParticleSystem *particleSystem = new HoParticleSystem();
					if(particleSystem != NULL)
					{
						particleSystem->Start(currentPos.x, currentPos.y, currentPos.z, MaterialNum[1], SKILL_METEO);
						tracker->Start(currentPos, destPos, SKILL_METEO, particleSystem);
						//tracker->Start(currentPos, destPos, startVelocity, particleSystem);
						//tracker->SearchSlowDownFlag = FALSE;
						
						if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(tracker, index*10))
						{
							delete tracker;
							tracker = NULL;
							return FALSE;
						}

						EffectTimeObjectBuffer[timeObjectIndex].Start();
						EffectTimeObjectBuffer[timeObjectIndex].AddObject(particleSystem, index*10);
						EffectTimeObjectBuffer[timeObjectIndex].Start();
					}
				}				
			}
			break;
		}
		
		case SKILL_AVALANCHE:
		{
			int timeObjectIndex = EmptyTimeObjectBufferIndex();
			if(timeObjectIndex == -1)
				return FALSE;
			
			HoEffectPatDest *patDest = new HoEffectPatDest();
			if(patDest != NULL)
			{
				POINT3D currentPos;
				POINT3D destPos;

				currentPos.x = x;
				currentPos.y = y;
				currentPos.z = z;
				
				destPos.x = destX;
				destPos.y = destY;
				destPos.z = destZ;

				patDest->StartDest(currentPos, destPos, PatObj[6], SKILL_AVALANCHE);
				patDest->AnimationEnd = PatAnimationEnd[6];

				if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(patDest, 0))
				{
					delete patDest;
					patDest = NULL;
					return FALSE;
				}
				EffectTimeObjectBuffer[timeObjectIndex].Start();
			}
			
			break;
		}

		case SKILL_ARROW_OF_RAGE:
		{
			int timeObjectIndex = EmptyTimeObjectBufferIndex();
			if(timeObjectIndex == -1)
				return FALSE;
			
			int index; for( index = 0; index < 1; index++)
			{
				HoEffectPatDest *patDest = new HoEffectPatDest();
				int ang = ANGLE_360/10*index;
				if(patDest != NULL)
				{
					POINT3D currentPos;
					POINT3D destPos;

					currentPos.x = x;
					currentPos.y = y;
					currentPos.z = z;

					destPos.x = int(destX+(GetCos[ang]/65536.f)*8000.f);
					destPos.y = int(destY);
					destPos.z = int(destZ+(GetSin[ang]/65536.f)*8000.f);
					
					patDest->StartDest(currentPos, destPos, PatObj[6], SKILL_ARROW_OF_RAGE, level);
					patDest->AnimationEnd = PatAnimationEnd[6];

					if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(patDest, rand()%30))
					{
						delete patDest;
						patDest = NULL;
						return FALSE;
					}
					EffectTimeObjectBuffer[timeObjectIndex].Start();
				}
			}
			break;
		}
		
		case SKILL_ARROW_OF_RAGE_END:
		{
			int timeObjectIndex = EmptyTimeObjectBufferIndex();
			if(timeObjectIndex == -1)
				return FALSE;
			
			int index; for( index = 0; index < 10+level/2; index++)
			{
				HoEffectPatDest *patDest = new HoEffectPatDest();
				int ang = ANGLE_360/(10+level/2)*index;
				if(patDest != NULL)
				{
					POINT3D currentPos;
					POINT3D destPos;

 					currentPos.x = x;
					currentPos.y = y;
					currentPos.z = z;

					destPos.x = int(destX+(GetCos[ang]/65536.f)*(4000.f+rand()%(level*500)));
					destPos.y = int(destY);
					destPos.z = int(destZ+(GetSin[ang]/65536.f)*(4000.f+rand()%(level*500)));
					
					patDest->StartDest(currentPos, destPos, PatObj[6], SKILL_ARROW_OF_RAGE_END);
					patDest->AnimationEnd = PatAnimationEnd[6];

					if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(patDest, rand()%30))
					{
						delete patDest;
						patDest = NULL;
						return FALSE;
					}
					EffectTimeObjectBuffer[timeObjectIndex].Start();
				}
				SkillPlaySound( SKILL_SOUND_SKILL_ARROW_OF_RANGE	 , x,y,z );	//떨어지는 소리
				SkillPlaySound( SKILL_SOUND_SCOUT_HAWK1	 , x,y,z );
			}
			break;
		}

		case SKILL_MECHANIC_BOMB:
		{
			int timeObjectIndex = EmptyTimeObjectBufferIndex();
			if(timeObjectIndex == -1)
				return FALSE;

			HoEffectPatDest *patDest = new HoEffectPatDest();
			if(patDest != NULL)
			{
				POINT3D currentPos;
				POINT3D destPos;

				currentPos.x = x;
				currentPos.y = y;
				currentPos.z = z;

				destPos.x = destX;
				destPos.y = destY;
				destPos.z = destZ;
				patDest->StartDest(currentPos, destPos, PatObj[9], SKILL_MECHANIC_BOMB, level);
				patDest->AnimationEnd = PatAnimationEnd[9];

				if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(patDest, 0))
				{
					delete patDest;
					patDest = NULL;
					return FALSE;
				}
				EffectTimeObjectBuffer[timeObjectIndex].Start();
			}
			break;
		}

		case SKILL_WIND_ARROW:
		{
			int timeObjectIndex = EmptyTimeObjectBufferIndex();
			if(timeObjectIndex == -1)
				return FALSE;

			HoEffectPatDest *patDest = new HoEffectPatDest();
			if(patDest != NULL)
			{
				POINT3D currentPos;
				POINT3D destPos;

				currentPos.x = x;
				currentPos.y = y;
				currentPos.z = z;

				destPos.x = destX;
				destPos.y = destY;
				destPos.z = destZ;
				patDest->StartDest(currentPos, destPos, PatObj[6], SKILL_WIND_ARROW);
				patDest->AnimationEnd = PatAnimationEnd[6];

				if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(patDest, 0))
				{
					delete patDest;
					patDest = NULL;
					return FALSE;
				}
				EffectTimeObjectBuffer[timeObjectIndex].Start();
			}
			break;
		}
		
		case SKILL_PERFECT_AIM:
		{
			int timeObjectIndex = EmptyTimeObjectBufferIndex();
			if(timeObjectIndex == -1)
				return FALSE;
			
			int index; for( index = 0; index < 4; index++)
			{
				int ang;
				ang = ANGLE_360/4*index;
				HoPrimitiveBillboard *primitiveBillBoard = new HoPrimitiveBillboard();
				POINT3D currentPos;
				currentPos.x = x;
				currentPos.y = y;
				currentPos.z = z;
				primitiveBillBoard->StartPath(currentPos, "PerfectAim2.ini", ANI_LOOP);
				
				POINT3D destPos;
				
				destPos.x = int(((float)GetCos[ang]/65536.f)*70.f)+x;
				destPos.y = int(((float)GetSin[ang]/65536.f)*70.f)+z;
				destPos.z = z;

				HoPhysicsDest	*physicsDest = new HoPhysicsDest();
				physicsDest->Start(destPos, currentPos, 20);
				primitiveBillBoard->SetPhysics(physicsDest, PRIMITIVE_DEFAULT_RECT_LINE);
				
				if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(primitiveBillBoard, index*3))
				{
					delete primitiveBillBoard;
					primitiveBillBoard = NULL;
					return FALSE;
				}
				
			}
			
			HoEffectPatDest *patDest = new HoEffectPatDest();
			if(patDest != NULL)
			{
				POINT3D currentPos;
				POINT3D destPos;

				currentPos.x = x;
				currentPos.y = y;
				currentPos.z = z;

				destPos.x = destX;
				destPos.y = destY;
				destPos.z = destZ;
				
				patDest->StartDest(currentPos, destPos, PatObj[6], SKILL_PERFECT_AIM);
				patDest->AnimationEnd = PatAnimationEnd[6];
				
				if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(patDest, 5))
				{
					delete patDest;
					patDest = NULL;
					return FALSE;
				}
				EffectTimeObjectBuffer[timeObjectIndex].Start();
			}
			break;	
		}

		
	}
	return TRUE;
}

int HoEffectMgr::StartSkill(int x, int y, int z, int angleX, int angleY, int angleZ, int skillType, int level)
{
	switch(skillType)
	{
		//2단계 Skill
		//Rage_Of_Zecram
//		case SKILL_RAGE_OF_ZECRAM_POWER1:
		case SKILL_RAGE_OF_ZECRAM_HIT1:
		{
			int timeObjectIndex = EmptyTimeObjectBufferIndex();
			if(timeObjectIndex == -1)
				return FALSE;
			
			HoPrimitiveBillboard *primitiveBillBoard = new HoPrimitiveBillboard();
			primitiveBillBoard->StartBillRect(x, y, z, "SkillZecram1.ini", ANI_ONE);
			if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(primitiveBillBoard, 0))
			{
				delete primitiveBillBoard;
				primitiveBillBoard = NULL;
				return FALSE;
			}
			EffectTimeObjectBuffer[timeObjectIndex].Start();
			
			primitiveBillBoard = new HoPrimitiveBillboard();
			POINT3D current;
			current.x = x;
			current.y = y;
			current.z = z;
			primitiveBillBoard->StartPath(current, "FireParticle1.ini", ANI_LOOP);
			
			int ang = rand()%ANGLE_360;
			HoPhysicsParticle *physicsParticle = new HoPhysicsParticle();
			POINT3D velocity;
			velocity.x = int(((float)GetCos[ang]/65536.f)*85.f);
			velocity.y = 70;
			velocity.z = int(((float)GetSin[ang]/65536.f)*85.f);

			physicsParticle->Start(velocity);
			physicsParticle->SetGravity(float(rand()%3+2));
			physicsParticle->SetLive(rand()%5+15);
			
			POINT3D angle;
			angle.x = 0;
			angle.y = ANGLE_360/10;
			angle.z = 0;
			physicsParticle->SetDestAngle(angle);
			primitiveBillBoard->SetPhysics(physicsParticle, PRIMITIVE_DEFAULT_RECT_PARTICLE | MOVE_ANGLE);
			
			POINT size;
			size.x = 25;
			size.y = 25;
			primitiveBillBoard->SetSize(size);
			if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(primitiveBillBoard, 0))
			{
				delete primitiveBillBoard;
				primitiveBillBoard = NULL;
				return FALSE;
			}

			EffectTimeObjectBuffer[timeObjectIndex].Start();
			break;
		}
		
//		case SKILL_RAGE_OF_ZECRAM_HIT1:
		case SKILL_RAGE_OF_ZECRAM_POWER1:
		{
			int objIndex;
			StartBillRectPrimitive(x+rand()%50, y+rand()%50, z+rand()%50, 120, 120, "Light1.ini");
			objIndex = EmptyZSortBufferIndex();
			if(objIndex == -1)
			{
				return FALSE;
			}
			
			HoPrimitiveBillboard *primitiveBillboard = new HoPrimitiveBillboard();
			if(primitiveBillboard == NULL)
				return FALSE;
			
			primitiveBillboard->StartBillRect(x+rand()%50, y+rand()%50, z+rand()%50, "CriticalHit1.ini", ANI_ONE);
			EffectObjectBuffer[objIndex] = primitiveBillboard;
			
			int timeObjectIndex = EmptyTimeObjectBufferIndex();
			if(timeObjectIndex == -1)
				return FALSE;
			
			int index; for( index = 0; index < 4; index++)
			{
				primitiveBillboard = NULL;
				primitiveBillboard = new HoPrimitiveBillboard();
				if(primitiveBillboard != NULL)
				{
					primitiveBillboard->StartBillRect(x, y, z, "SkillRageofZecram1.ini", ANI_ONE);
					if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(primitiveBillboard, index*2))
					{
						delete primitiveBillboard;
						primitiveBillboard = NULL;
						return FALSE;
					}
					EffectTimeObjectBuffer[timeObjectIndex].Start();
				}
			}
			break;
		}
		
		case SKILL_RAGE_OF_ZECRAM_HIT2:
		{
			int timeObjectIndex = EmptyTimeObjectBufferIndex();
			if(timeObjectIndex == -1)
				return FALSE;

			SetDynLight( x, y, z, 255, 50, 50, 255, 200 , 3);
			int index; for( index = 0; index < 2; index++)
			{
				HoEffectPat	*pat = new HoEffectPat();
				if(pat != NULL)
				{
					pat->StartAni(x, y, z,0, 0, 0, PatObj[16]);
					pat->AnimationEnd = PatAnimationEnd[16];

					if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(pat, index))
					{
						delete pat;
						pat = NULL;
						return FALSE;
					}
					EffectTimeObjectBuffer[timeObjectIndex].Start();
				}
			}
			
			
			for(index = 0; index < 2; index++)
			{
				HoEffectPat	*pat = new HoEffectPat();
				if(pat != NULL)
				{
					pat->StartAni(x, y-2000, z,0, angleY, 0, PatObj[17]);
					pat->AnimationEnd = PatAnimationEnd[17];

					if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(pat, index*5))
					{
						delete pat;
						pat = NULL;
						return FALSE;
					}
					EffectTimeObjectBuffer[timeObjectIndex].Start();
				}
			}
			
			
			for(index = 0; index < 1; index++)
			{
				HoEffectPat	*pat = new HoEffectPat();
				if(pat != NULL)
				{
					pat->StartAni(x, y-2000, z,0, angleY+ANGLE_180, 0, PatObj[17]);
					pat->AnimationEnd = PatAnimationEnd[17];

					if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(pat, index*5))
					{
						delete pat;
						pat = NULL;
						return FALSE;
					}
					EffectTimeObjectBuffer[timeObjectIndex].Start();
				}
			}
			
			
			HoPrimitiveBillboard *extPrimitive;
			extPrimitive = new HoPrimitiveBillboard();
			extPrimitive->StartBillRect(x, y, z,100, 100, "SkillTripleImpactHit.ini", ANI_ONE);
			if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(extPrimitive,5))
			{
					delete extPrimitive;
					extPrimitive = NULL;
					return FALSE;
			}
			EffectTimeObjectBuffer[timeObjectIndex].Start();
			
			
			for(index = 0; index < 30; index++)
			{
				HoPrimitiveBillboard *billBoard = new HoPrimitiveBillboard();
				if(billBoard != NULL)
				{
					int ang = rand()%ANGLE_360;

					//POINT3D currentPos;
					POINT3D worldPos;
					POINT3D localPos;
					POINT3D destPos;
					POINT	size;
					size.x = 25;
					size.y = 100+rand()%200;
					
					float RandomYaw = RANDOM_NUM   * 3.14159f * 2.0f;
					float RandomPitch = RANDOM_NUM * 180.f * 3.14159f / 180.0f ;
					
					//구모양으로 파티클이 날아가게 벡터를 조정한다.
					destPos.y = int(cosf( RandomPitch)*((16.f)*(16.f))+y);
					destPos.x = int(sinf( RandomPitch)*(20.f) * cosf(RandomYaw)*(20.f)+x);
					destPos.z = int(sinf( RandomPitch)*(20.f) * sinf(RandomYaw)*(20.f)+z);
					
					worldPos.x = x;
					worldPos.y = y;
					worldPos.z = z;
					
					
					localPos.y = 0;
					localPos.x = 0;
					localPos.z = 0;
					
					billBoard->StartDestPath(worldPos, localPos, destPos, size, "SkillTripleImpactParticle.ini", 800);
					if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(billBoard, index/2))
					{
						delete billBoard;
						billBoard = NULL;
						return FALSE;
					}
					EffectTimeObjectBuffer[timeObjectIndex].Start();
				}
			}
			
			y = y-2000;
			for(index = 0; index < 10; index++)
			{
				HoParticleDest *particleDest = new HoParticleDest;
				if(particleDest != NULL)
				{
					int ang = ANGLE_360/10*index;
					POINT3D destPos;
					destPos.x = int(((float)GetCos[ang]/65536.f)*8000.f)+x;
					destPos.y = y;
					destPos.z = int(((float)GetSin[ang]/65536.f)*8000.f)+z;
					
					particleDest->Start(x, y, z,
					destPos.x, 
					destPos.y, 
					destPos.z, MaterialNum[5], 2);
					particleDest->SetAge(0.29f+level*(0.2f/10.f));
					
					if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(particleDest, 10))
					{
						delete particleDest;
						particleDest = NULL;
						return FALSE;
					}
					EffectTimeObjectBuffer[timeObjectIndex].Start();
				}
			}
			break;
		}

		case SKILL_AVALANCHE_PARTICLE:
		{
			int timeObjectIndex = EmptyTimeObjectBufferIndex();
			if(timeObjectIndex == -1)
				return FALSE;

			HoEffectPat *pat = new HoEffectPat();
			if(pat != NULL)
			{
				pat->StartAni(x, y, z,angleX, angleY, angleZ, PatObj[13]);
				pat->AnimationEnd = PatAnimationEnd[13];
			}
			
			if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(pat, 0))
			{
				delete pat;
				pat = NULL;
				return FALSE;
			}
			EffectTimeObjectBuffer[timeObjectIndex].Start();

			break;
		}




		//ktj : 새로 만듦.================================================================end
		case SKILL_GREAT_SMASH2:
		{
			//ktj : 원본
			//SetDynLight( x, y, z, 90, 90, 250, 255, 200, 3);
			SetDynLight( x, y, z,    250, 0, 0,      255, 200, 3);	//ktj : 처음번쩍하는부분의 rgb는여기서고치면가능함
			
			int timeObjectIndex = EmptyTimeObjectBufferIndex();
			if(timeObjectIndex == -1)
				return FALSE;
			
			HoPrimitiveBillboard *extPrimitive;
			extPrimitive = new HoPrimitiveBillboard();
			extPrimitive->StartBillRect(x, y, z,120, 120, "SkillRoundImpactHit.ini", ANI_ONE);
			if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(extPrimitive, 0))
			{
					delete extPrimitive;
					extPrimitive = NULL;
					return FALSE;
			}
			EffectTimeObjectBuffer[timeObjectIndex].Start();

			
			//ktj : 길쭉한모양의 파티클
			int index; for( index = 0; index < 20; index++)
			{
				HoPrimitiveBillboard *billBoard = new HoPrimitiveBillboard();
				if(billBoard != NULL)
				{
					int ang = rand()%ANGLE_360;

					//POINT3D currentPos;
					POINT3D worldPos;
					POINT3D localPos;
					POINT3D destPos;
					POINT	size;
					size.x = 25;
					size.y = 100+rand()%200;
					
					float RandomYaw = RANDOM_NUM   * 3.14159f * 2.0f;
					float RandomPitch = RANDOM_NUM * 180.f * 3.14159f / 180.0f ;
					
					//구모양으로 파티클이 날아가게 벡터를 조정한다.
					destPos.y = int(cosf( RandomPitch)*((16.f)*(16.f))+y);
					destPos.x = int(sinf( RandomPitch)*(20.f) * cosf(RandomYaw)*(20.f)+x);
					destPos.z = int(sinf( RandomPitch)*(20.f) * sinf(RandomYaw)*(20.f)+z);
					
					worldPos.x = x;
					worldPos.y = y;
					worldPos.z = z;
					
					
					localPos.y = 0;
					localPos.x = 0;
					localPos.z = 0;
					
					//ktj : 바꿈.
					//billBoard->StartDestPath(worldPos, localPos, destPos, size, "SkillRoundImpactParticle.ini", 800);
					billBoard->StartDestPath(worldPos, localPos, destPos, size, "ParticleMeteo2.ini", 800);
					if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(billBoard, index))
					{
						delete billBoard;
						billBoard = NULL;
						return FALSE;
					}
					EffectTimeObjectBuffer[timeObjectIndex].Start();
				}
			}

			//ktj 동그란모양의파티클들
			for(index = 0; index < 15; index++)
			{
				HoPrimitiveBillboard *primitiveBillBoard = new HoPrimitiveBillboard();
				POINT3D currentPos;
				currentPos.x = x;
				currentPos.y = y;
				currentPos.z = z;

				//ktj : 바꿈.
				//primitiveBillBoard->StartPath(currentPos, "PhysicalAbsorbParticle2.ini", ANI_LOOP);
				primitiveBillBoard->StartPath(currentPos, "ParticleMeteo3.ini", ANI_LOOP);
				
				
				int ang = rand()%ANGLE_360;
				HoPhysicsParticle *physicsParticle = new HoPhysicsParticle();
				POINT3D velocity;
				velocity.x = int(((float)GetCos[ang]/65536.f)*200.f);

				velocity.y = 100;
				velocity.z = int(((float)GetSin[ang]/65536.f)*200.f);

				physicsParticle->Start(velocity);
				physicsParticle->SetGravity(float(-rand()%6));
				physicsParticle->SetLive(rand()%20+40);
				
			
				primitiveBillBoard->SetPhysics(physicsParticle, PRIMITIVE_DEFAULT_RECT_PARTICLE);
				
				POINT size;
				size.x = 9;
				size.y = 9;
				primitiveBillBoard->SetSize(size);
				if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(primitiveBillBoard, 0))
				{
					delete primitiveBillBoard;
					primitiveBillBoard = NULL;
					return FALSE;
				}
				EffectTimeObjectBuffer[timeObjectIndex].Start();
			}
			
			break;
		}

		//================================================================end

		
		case SKILL_GREAT_SMASH:
		{
			SetDynLight( x, y, z,    90, 90, 250,       255, 200, 3);
			
			int timeObjectIndex = EmptyTimeObjectBufferIndex();
			if(timeObjectIndex == -1)
				return FALSE;
			
			HoPrimitiveBillboard *extPrimitive;
			extPrimitive = new HoPrimitiveBillboard();
			extPrimitive->StartBillRect(x, y, z,120, 120, "SkillRoundImpactHit.ini", ANI_ONE);
			if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(extPrimitive, 0))
			{
					delete extPrimitive;
					extPrimitive = NULL;
					return FALSE;
			}
			EffectTimeObjectBuffer[timeObjectIndex].Start();

			
			//ktj : 길쭉한모양의 파티클
			int index; for( index = 0; index < 20; index++)
			{
				HoPrimitiveBillboard *billBoard = new HoPrimitiveBillboard();
				if(billBoard != NULL)
				{
					int ang = rand()%ANGLE_360;

					//POINT3D currentPos;
					POINT3D worldPos;
					POINT3D localPos;
					POINT3D destPos;
					POINT	size;
					size.x = 25;
					size.y = 100+rand()%200;
					
					float RandomYaw = RANDOM_NUM   * 3.14159f * 2.0f;
					float RandomPitch = RANDOM_NUM * 180.f * 3.14159f / 180.0f ;
					
					//구모양으로 파티클이 날아가게 벡터를 조정한다.
					destPos.y = int(cosf( RandomPitch)*((16.f)*(16.f))+y);
					destPos.x = int(sinf( RandomPitch)*(20.f) * cosf(RandomYaw)*(20.f)+x);
					destPos.z = int(sinf( RandomPitch)*(20.f) * sinf(RandomYaw)*(20.f)+z);
					
					worldPos.x = x;
					worldPos.y = y;
					worldPos.z = z;
					
					
					localPos.y = 0;
					localPos.x = 0;
					localPos.z = 0;
					
					billBoard->StartDestPath(worldPos, localPos, destPos, size, "SkillRoundImpactParticle.ini", 800);
					if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(billBoard, index))
					{
						delete billBoard;
						billBoard = NULL;
						return FALSE;
					}
					EffectTimeObjectBuffer[timeObjectIndex].Start();
				}
			}

			//ktj 동그란모양의파티클들
			for(index = 0; index < 15; index++)
			{
				HoPrimitiveBillboard *primitiveBillBoard = new HoPrimitiveBillboard();
				POINT3D currentPos;
				currentPos.x = x;
				currentPos.y = y;
				currentPos.z = z;
				primitiveBillBoard->StartPath(currentPos, "PhysicalAbsorbParticle2.ini", ANI_LOOP);
				
				int ang = rand()%ANGLE_360;
				HoPhysicsParticle *physicsParticle = new HoPhysicsParticle();
				POINT3D velocity;
				velocity.x = int(((float)GetCos[ang]/65536.f)*200.f);

				velocity.y = 100;
				velocity.z = int(((float)GetSin[ang]/65536.f)*200.f);

				physicsParticle->Start(velocity);
				physicsParticle->SetGravity(float(-rand()%6));
				physicsParticle->SetLive(rand()%20+40);
				
			
				primitiveBillBoard->SetPhysics(physicsParticle, PRIMITIVE_DEFAULT_RECT_PARTICLE);
				
				POINT size;
				size.x = 9;
				size.y = 9;
				primitiveBillBoard->SetSize(size);
				if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(primitiveBillBoard, 0))
				{
					delete primitiveBillBoard;
					primitiveBillBoard = NULL;
					return FALSE;
				}
				EffectTimeObjectBuffer[timeObjectIndex].Start();
			}
			
			break;
		}


	case SKILL_GRAND_SMASH:
	{
		int timeObjectIndex = EmptyTimeObjectBufferIndex();
		if(timeObjectIndex == -1)
			return FALSE;

		//ktj : 길쭉한모양의 파티클
		for(int index = 0; index < 20; index++)
		{
			HoPrimitiveBillboard *billBoard = new HoPrimitiveBillboard();
			if(billBoard != NULL)
			{
				int ang = rand()%ANGLE_360;

				//POINT3D currentPos;
				POINT3D worldPos;
				POINT3D localPos;
				POINT3D destPos;
				POINT	size;
				size.x = 25;
				size.y = 100+rand()%200;
				
				float RandomYaw = RANDOM_NUM   * 3.14159f * 2.0f;
				float RandomPitch = RANDOM_NUM * 180.f * 3.14159f / 180.0f ;
				
				//구모양으로 파티클이 날아가게 벡터를 조정한다.
				destPos.y = int(cosf( RandomPitch)*((16.f)*(16.f))+y);
				destPos.x = int(sinf( RandomPitch)*(20.f) * cosf(RandomYaw)*(20.f)+x);
				destPos.z = int(sinf( RandomPitch)*(20.f) * sinf(RandomYaw)*(20.f)+z);
				
				worldPos.x = x;
				worldPos.y = y;
				worldPos.z = z;
				
				
				localPos.y = 0;
				localPos.x = 0;
				localPos.z = 0;
				
				billBoard->StartDestPath(worldPos, localPos, destPos, size, "SkillRoundImpactParticle.ini", 800);
				if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(billBoard, index))
				{
					delete billBoard;
					billBoard = NULL;
					return FALSE;
				}
				EffectTimeObjectBuffer[timeObjectIndex].Start();
			}
		}
		
		break;
	}










		
		
		case SKILL_BRUTAL_SWING:
		{
			SetDynLight( x, y, z, 255, 100, 100, 255, 200 , 3);
			int timeObjectIndex = EmptyTimeObjectBufferIndex();
			if(timeObjectIndex == -1)
				return FALSE;
			
			HoPrimitiveBillboard *extPrimitive;
			extPrimitive = new HoPrimitiveBillboard();
			extPrimitive->StartBillRect(x, y, z,120, 120, "SkillTripleImpactHit.ini", ANI_ONE);
			if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(extPrimitive, 0))
			{
					delete extPrimitive;
					extPrimitive = NULL;
					return FALSE;
			}
			EffectTimeObjectBuffer[timeObjectIndex].Start();

			int index; for( index = 0; index < 20; index++)
			{
				HoPrimitiveBillboard *billBoard = new HoPrimitiveBillboard();
				if(billBoard != NULL)
				{
					int ang = rand()%ANGLE_360;

					//POINT3D currentPos;
					POINT3D worldPos;
					POINT3D localPos;
					POINT3D destPos;
					POINT	size;
					size.x = 25;
					size.y = 100+rand()%200;
					
					float RandomYaw = RANDOM_NUM   * 3.14159f * 2.0f;
					float RandomPitch = RANDOM_NUM * 180.f * 3.14159f / 180.0f ;
					
					//구모양으로 파티클이 날아가게 벡터를 조정한다.
					destPos.y = int(cosf( RandomPitch)*((16.f)*(16.f))+y);
					destPos.x = int(sinf( RandomPitch)*(20.f) * cosf(RandomYaw)*(20.f)+x);
					destPos.z = int(sinf( RandomPitch)*(20.f) * sinf(RandomYaw)*(20.f)+z);
					
					worldPos.x = x;
					worldPos.y = y;
					worldPos.z = z;
					
					
					localPos.y = 0;
					localPos.x = 0;
					localPos.z = 0;
					
					billBoard->StartDestPath(worldPos, localPos, destPos, size, "SkillTripleImpactParticle.ini", 800);
					if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(billBoard, index))
					{
						delete billBoard;
						billBoard = NULL;
						return FALSE;
					}
					EffectTimeObjectBuffer[timeObjectIndex].Start();
				}
			}
			

			for(index = 0; index < 15; index++)
			{
				HoPrimitiveBillboard *primitiveBillBoard = new HoPrimitiveBillboard();
				POINT3D currentPos;
				currentPos.x = x;
				currentPos.y = y;
				currentPos.z = z;
				primitiveBillBoard->StartPath(currentPos, "returnParticle1.ini", ANI_LOOP);
				
				int ang = rand()%ANGLE_360;
				HoPhysicsParticle *physicsParticle = new HoPhysicsParticle();
				POINT3D velocity;
				velocity.x = int(((float)GetCos[ang]/65536.f)*200.f);

				velocity.y = 100;
				velocity.z = int(((float)GetSin[ang]/65536.f)*200.f);

				physicsParticle->Start(velocity);
				physicsParticle->SetGravity(float(-rand()%6));
				physicsParticle->SetLive(rand()%20+40);
				
			
				primitiveBillBoard->SetPhysics(physicsParticle, PRIMITIVE_DEFAULT_RECT_PARTICLE);
				
				POINT size;
				size.x = 9;
				size.y = 9;
				primitiveBillBoard->SetSize(size);
				if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(primitiveBillBoard, 0))
				{
					delete primitiveBillBoard;
					primitiveBillBoard = NULL;
					return FALSE;
				}
				EffectTimeObjectBuffer[timeObjectIndex].Start();
			}
			break;
		}
		
		
		case SKILL_TRIPLE_IMPACT:

		{
			SetDynLight( x, y, z, 255, 100, 100, 255, 200 , 3);
			int timeObjectIndex = EmptyTimeObjectBufferIndex();
			if(timeObjectIndex == -1)
				return FALSE;
			
			HoPrimitiveBillboard *extPrimitive;
			extPrimitive = new HoPrimitiveBillboard();
			extPrimitive->StartBillRect(x, y, z,120, 120, "SkillTripleImpactHit.ini", ANI_ONE);
			if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(extPrimitive, 0))
			{
					delete extPrimitive;
					extPrimitive = NULL;
					return FALSE;
			}
			EffectTimeObjectBuffer[timeObjectIndex].Start();

			int index; for( index = 0; index < 20; index++)
			{
				HoPrimitiveBillboard *billBoard = new HoPrimitiveBillboard();
				if(billBoard != NULL)
				{
					int ang = rand()%ANGLE_360;

					//POINT3D currentPos;
					POINT3D worldPos;
					POINT3D localPos;
					POINT3D destPos;
					POINT	size;
					size.x = 25;
					size.y = 100+rand()%200;
					
					float RandomYaw = RANDOM_NUM   * 3.14159f * 2.0f;
					float RandomPitch = RANDOM_NUM * 180.f * 3.14159f / 180.0f ;
					
					//구모양으로 파티클이 날아가게 벡터를 조정한다.
					destPos.y = int(cosf( RandomPitch)*((16.f)*(16.f))+y);
					destPos.x = int(sinf( RandomPitch)*(20.f) * cosf(RandomYaw)*(20.f)+x);
					destPos.z = int(sinf( RandomPitch)*(20.f) * sinf(RandomYaw)*(20.f)+z);
					
					worldPos.x = x;
					worldPos.y = y;
					worldPos.z = z;
					
					
					localPos.y = 0;
					localPos.x = 0;
					localPos.z = 0;
					
					billBoard->StartDestPath(worldPos, localPos, destPos, size, "SkillTripleImpactParticle.ini", 800);
					if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(billBoard, index))
					{
						delete billBoard;
						billBoard = NULL;
						return FALSE;
					}
					EffectTimeObjectBuffer[timeObjectIndex].Start();
				}
			}
			

			for(index = 0; index < 15; index++)
			{
				HoPrimitiveBillboard *primitiveBillBoard = new HoPrimitiveBillboard();
				POINT3D currentPos;
				currentPos.x = x;
				currentPos.y = y;
				currentPos.z = z;
				primitiveBillBoard->StartPath(currentPos, "returnParticle1.ini", ANI_LOOP);
				
				int ang = rand()%ANGLE_360;
				HoPhysicsParticle *physicsParticle = new HoPhysicsParticle();
				POINT3D velocity;
				velocity.x = int(((float)GetCos[ang]/65536.f)*200.f);

				velocity.y = 100;
				velocity.z = int(((float)GetSin[ang]/65536.f)*200.f);

				physicsParticle->Start(velocity);
				physicsParticle->SetGravity(float(-rand()%6));
				physicsParticle->SetLive(rand()%20+40);
				
			
				primitiveBillBoard->SetPhysics(physicsParticle, PRIMITIVE_DEFAULT_RECT_PARTICLE);
				
				POINT size;
				size.x = 9;
				size.y = 9;
				primitiveBillBoard->SetSize(size);
				if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(primitiveBillBoard, 0))
				{
					delete primitiveBillBoard;
					primitiveBillBoard = NULL;
					return FALSE;
				}
				EffectTimeObjectBuffer[timeObjectIndex].Start();
			}
			break;
		}
		

		case SKILL_ROAR:
		{
     		int timeObjectIndex = EmptyTimeObjectBufferIndex();
			if(timeObjectIndex == -1)
				return FALSE;
			
			HoEffectLight *light = new HoEffectLight();
			if(light != NULL)
			{
				light->Start(x, y, z, 200, 150, 80, 255, 150, 2);
				if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(light, 0))
				{
					delete light;
					light = NULL;
					return FALSE;
				}
				EffectTimeObjectBuffer[timeObjectIndex].Start();
			}
			
			
			light = new HoEffectLight();
			if(light != NULL)
			{
				light->Start(x, y, z, 255, 200, 100, 255, 200, 3);
				if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(light, 45))
				{
					delete light;
					light = NULL;
					return FALSE;
				}
				EffectTimeObjectBuffer[timeObjectIndex].Start();
			}
			
 			int index; for( index = 0; index < 4+level/5; index++)
			{
				HoEffectPat	*pat = new HoEffectPat();
				if(pat != NULL)
				{
					pat->StartAni(x, y, z,angleX, angleY&ANGCLIP, angleZ, PatObj[10]);
					pat->AnimationEnd = 9+(level/5); //PatAnimationEnd[10];
					
					if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(pat, 45+index*2))
					{
						delete pat;
						pat = NULL;
						return FALSE;
					}
					EffectTimeObjectBuffer[timeObjectIndex].Start();
				}
			}

			POINT3D partPos;
    		partPos.x = x;
			partPos.y = y+3000;
			partPos.z = z;
  			g_NewParticleMgr.Start("Skill2FighterRoar1",partPos, 0.5f);

			break;
		}

		

		case SKILL_UP1:
		{
			SetDynLight( x, y, z, 200, 200, 200, 255, 260 , 1);
			int timeObjectIndex = EmptyTimeObjectBufferIndex();
			if(timeObjectIndex == -1)
				return FALSE;
			
			int index; for( index = 0; index < 60; index++)
			{
				HoPrimitiveBillboard *billBoard = new HoPrimitiveBillboard();
				if(billBoard != NULL)
				{
					int ang = rand()%ANGLE_360;

					
					POINT3D destPos;
					POINT3D worldPos;
					POINT3D localPos;
					
					POINT	size;
					size.x = 40;
					size.y = 300+rand()%300;
					
					worldPos.x = x;
					worldPos.y = y;
					worldPos.z = z;
					
					localPos.x = int((GetCos[ang]/65536.f)*2900.f);
					localPos.y = 0;
					localPos.z = int((GetSin[ang]/65536.f)*2900.f);

					destPos.x = int((GetCos[ang]/65536.f)*(4000.f-(4000.f-2900.f)/40*index))+x;
					destPos.y = y+1500;
					destPos.z = int((GetSin[ang]/65536.f)*(4000.f-(4000.f-2900.f)/40*index))+z;

					billBoard->StartDestPath(worldPos, localPos, destPos, size, "SkillUp2.ini", 600);
					if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(billBoard, rand()%20))
					{
						delete billBoard;
						billBoard = NULL;
						return FALSE;
					}
					EffectTimeObjectBuffer[timeObjectIndex].Start();
				}
			}
			
			for(index = 0; index < 50; index++)
			{
				HoPrimitiveBillboard *billBoard = new HoPrimitiveBillboard();
				if(billBoard != NULL)
				{
					int ang = rand()%ANGLE_360;
					//POINT3D currentPos;
					POINT3D worldPos;
					POINT3D localPos;
					POINT3D destPos;

					POINT	size;
					size.x = 25;
					size.y =500+rand()%300;

					worldPos.x = x;
					worldPos.y = y;
					worldPos.z = z;

					localPos.x = int((GetCos[ang]/65536.f)*3400.f);
					localPos.y = 0;
					localPos.z = int((GetSin[ang]/65536.f)*3400.f);


					destPos.x = int((GetCos[ang]/65536.f)*3990.f)+x;
					destPos.y = y+5000;
					destPos.z = int((GetSin[ang]/65536.f)*3990.f)+z;
					
					
					billBoard->StartDestPath(worldPos, localPos, destPos, size, "SkillUp1.ini", 1000);
					if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(billBoard, rand()%35+15))
					{
						delete billBoard;
						billBoard = NULL;
						return FALSE;
					}
					EffectTimeObjectBuffer[timeObjectIndex].Start();
				}
			}
			break;
		}
		
		case SKILL_METALARMOR:
		{
			int timeObjectIndex = EmptyTimeObjectBufferIndex();
			if(timeObjectIndex == -1)
				return FALSE;
			

			HoPrimitiveBillboard *extPrimitive = new HoPrimitiveBillboard();
			int index; for( index = 0; index < 20; index++)
			{
				HoPrimitiveBillboard *primitiveBillBoard = new HoPrimitiveBillboard();
				POINT3D currentPos;
				currentPos.x = x;
				currentPos.y = y-10000;
				currentPos.z = z;
				primitiveBillBoard->StartPath(currentPos, "PhysicalAbsorbParticle2.ini", ANI_LOOP);
				
				int ang = rand()%ANGLE_360;
				HoPhysicsParticle *physicsParticle = new HoPhysicsParticle();
				POINT3D velocity;
				velocity.x = int(((float)GetCos[ang]/65536.f)*120.f);
				velocity.y = 400;
				velocity.z = int(((float)GetSin[ang]/65536.f)*120.f);

				physicsParticle->Start(velocity);
				physicsParticle->SetGravity(float(rand()%3+2));
				physicsParticle->SetLive(rand()%20+30);
				
				POINT3D angle;
				angle.x = 0;
				angle.y = ANGLE_360/3;
				angle.z = 0;
				physicsParticle->SetDestAngle(angle);
				primitiveBillBoard->SetPhysics(physicsParticle, PRIMITIVE_DEFAULT_RECT_PARTICLE | MOVE_ANGLE);
				
				POINT size;
				size.x = 10;
				size.y = 10;
				primitiveBillBoard->SetSize(size);
				if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(primitiveBillBoard, index))
				{
					delete primitiveBillBoard;
					primitiveBillBoard = NULL;
					return FALSE;
				}
				EffectTimeObjectBuffer[timeObjectIndex].Start();
			}

			for(index = 0; index < 20; index++)
			{
				HoPrimitiveBillboard *primitiveBillBoard = new HoPrimitiveBillboard();
				POINT3D currentPos;
				currentPos.x = x;
				currentPos.y = y+4000;
				currentPos.z = z;
				primitiveBillBoard->StartPath(currentPos, "PhysicalAbsorbParticle2.ini", ANI_LOOP);
				
				int ang = rand()%ANGLE_360;
				HoPhysicsParticle *physicsParticle = new HoPhysicsParticle();
				POINT3D velocity;
				velocity.x = int(((float)GetCos[ang]/65536.f)*220.f);
				velocity.y = 200;
				velocity.z = int(((float)GetSin[ang]/65536.f)*220.f);

				physicsParticle->Start(velocity);
				physicsParticle->SetGravity(float(-rand()%6));
				physicsParticle->SetLive(rand()%20+40);
				
			
				primitiveBillBoard->SetPhysics(physicsParticle, PRIMITIVE_DEFAULT_RECT_PARTICLE);
				
				POINT size;
				size.x = 9;
				size.y = 9;
				primitiveBillBoard->SetSize(size);
				if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(primitiveBillBoard, 40))
				{
					delete primitiveBillBoard;
					primitiveBillBoard = NULL;
					return FALSE;
				}
				EffectTimeObjectBuffer[timeObjectIndex].Start();
			}

			for(index = 0; index < 30; index++)
			{
				HoPrimitiveBillboard *billBoard = new HoPrimitiveBillboard();
				if(billBoard != NULL)
				{
					int ang = rand()%ANGLE_360;

					//POINT3D currentPos;
					POINT3D worldPos;
					POINT3D localPos;
					POINT3D destPos;
					POINT	size;
					size.x = 20;
					size.y = 130+rand()%300;
					
					float RandomYaw = RANDOM_NUM   * 3.14159f * 2.0f;
					float RandomPitch = RANDOM_NUM * 180.f * 3.14159f / 180.0f ;
					
					//구모양으로 파티클이 날아가게 벡터를 조정한다.
					destPos.y = int(cosf( RandomPitch)*(100.f*100.f)+y+8000);
					destPos.x = int(sinf( RandomPitch)*150.f * cosf(RandomYaw)*150.f+x);
					destPos.z = int(sinf( RandomPitch)*150.f * sinf(RandomYaw)*150.f+z);
					
					worldPos.x = x;
					worldPos.y = y+8000;
					worldPos.z = z;

					localPos.y = int(cosf( RandomPitch)*(70.f*70.f));
					localPos.x = int(sinf( RandomPitch)*85.f * cosf(RandomYaw)*85.f);
					localPos.z = int(sinf( RandomPitch)*85.f * sinf(RandomYaw)*85.f);
					
					
					billBoard->StartDestPath(worldPos, localPos, destPos, size, "PhysicalAbsorbParticle1.ini", 600);
					if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(billBoard, 40))
					{
						delete billBoard;
						billBoard = NULL;
						return FALSE;
					}
					EffectTimeObjectBuffer[timeObjectIndex].Start();
				}
			}

			HoEffectLight *light = new HoEffectLight();
			if(light != NULL)
			{
				light->Start(x, y, z, 200, 200, 256, 255, 100, 1);
				if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(light, 0))
				{
					delete light;
					light = NULL;
					return FALSE;
				}
				EffectTimeObjectBuffer[timeObjectIndex].Start();
			}
			
			
			light = new HoEffectLight();
			if(light != NULL)
			{
				light->Start(x, y, z, 100, 100, 200, 256, 300, 2);
				if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(light, 45))
				{
					delete light;
					light = NULL;
					return FALSE;
				}
				EffectTimeObjectBuffer[timeObjectIndex].Start();
			}
			
			extPrimitive = NULL;
			extPrimitive = new HoPrimitiveBillboard();
			if(extPrimitive != NULL)
			{
				extPrimitive->StartBillRect(x, y+8000, z, "Pung1.ini", ANI_ONE);
				if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(extPrimitive, 45))
				{
					delete extPrimitive;
					extPrimitive = NULL;
					return FALSE;
				}
				EffectTimeObjectBuffer[timeObjectIndex].Start();
			}

			break;	
		}

		case SKILL_AUTOMATION: 
		{
			int timeObjectIndex = EmptyTimeObjectBufferIndex();
			if(timeObjectIndex == -1)
				return FALSE;
			

			HoPrimitiveBillboard *extPrimitive = new HoPrimitiveBillboard();
			int index; for( index = 0; index < 20; index++)
			{
				HoPrimitiveBillboard *primitiveBillBoard = new HoPrimitiveBillboard();
				POINT3D currentPos;
				currentPos.x = x;
				currentPos.y = y-10000;
				currentPos.z = z;
				primitiveBillBoard->StartPath(currentPos, "PhysicalAbsorbParticle2.ini", ANI_LOOP);
				
				int ang = rand()%ANGLE_360;
				HoPhysicsParticle *physicsParticle = new HoPhysicsParticle();
				POINT3D velocity;
				velocity.x = int(((float)GetCos[ang]/65536.f)*120.f);
				velocity.y = 400;
				velocity.z = int(((float)GetSin[ang]/65536.f)*120.f);

				physicsParticle->Start(velocity);
				physicsParticle->SetGravity(float(rand()%3+2));
				physicsParticle->SetLive(rand()%20+30);
				
				POINT3D angle;
				angle.x = 0;
				angle.y = ANGLE_360/3;
				angle.z = 0;
				physicsParticle->SetDestAngle(angle);
				primitiveBillBoard->SetPhysics(physicsParticle, PRIMITIVE_DEFAULT_RECT_PARTICLE | MOVE_ANGLE);
				
				POINT size;
				size.x = 10;
				size.y = 10;
				primitiveBillBoard->SetSize(size);
				if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(primitiveBillBoard, index))
				{
					delete primitiveBillBoard;
					primitiveBillBoard = NULL;
					return FALSE;
				}
				EffectTimeObjectBuffer[timeObjectIndex].Start();
			}

			for(index = 0; index < 20; index++)
			{
				HoPrimitiveBillboard *primitiveBillBoard = new HoPrimitiveBillboard();
				POINT3D currentPos;
				currentPos.x = x;
				currentPos.y = y+4000;
				currentPos.z = z;
				primitiveBillBoard->StartPath(currentPos, "PhysicalAbsorbParticle2.ini", ANI_LOOP);
				
				int ang = rand()%ANGLE_360;
				HoPhysicsParticle *physicsParticle = new HoPhysicsParticle();
				POINT3D velocity;
				velocity.x = int(((float)GetCos[ang]/65536.f)*220.f);
				velocity.y = 200;
				velocity.z = int(((float)GetSin[ang]/65536.f)*220.f);

				physicsParticle->Start(velocity);
				physicsParticle->SetGravity(float(-rand()%6));
				physicsParticle->SetLive(rand()%20+40);
				
			
				primitiveBillBoard->SetPhysics(physicsParticle, PRIMITIVE_DEFAULT_RECT_PARTICLE);
				
				POINT size;
				size.x = 9;
				size.y = 9;
				primitiveBillBoard->SetSize(size);
				if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(primitiveBillBoard, 40))
				{
					delete primitiveBillBoard;
					primitiveBillBoard = NULL;
					return FALSE;
				}
				EffectTimeObjectBuffer[timeObjectIndex].Start();
			}

			for(index = 0; index < 30; index++)
			{
				HoPrimitiveBillboard *billBoard = new HoPrimitiveBillboard();
				if(billBoard != NULL)
				{
					int ang = rand()%ANGLE_360;

					//POINT3D currentPos;
					POINT3D worldPos;
					POINT3D localPos;
					POINT3D destPos;
					POINT	size;
					size.x = 20;
					size.y = 130+rand()%300;
					
					float RandomYaw = RANDOM_NUM   * 3.14159f * 2.0f;
					float RandomPitch = RANDOM_NUM * 180.f * 3.14159f / 180.0f ;
					
					//구모양으로 파티클이 날아가게 벡터를 조정한다.
					destPos.y = int(cosf( RandomPitch)*(100.f*100.f)+y+8000);
					destPos.x = int(sinf( RandomPitch)*150.f * cosf(RandomYaw)*150.f+x);
					destPos.z = int(sinf( RandomPitch)*150.f * sinf(RandomYaw)*150.f+z);
					
					worldPos.x = x;
					worldPos.y = y+8000;
					worldPos.z = z;

					localPos.y = int(cosf( RandomPitch)*(70.f*70.f));
					localPos.x = int(sinf( RandomPitch)*85.f * cosf(RandomYaw)*85.f);
					localPos.z = int(sinf( RandomPitch)*85.f * sinf(RandomYaw)*85.f);
					
					
					billBoard->StartDestPath(worldPos, localPos, destPos, size, "PhysicalAbsorbParticle1.ini", 600);
					if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(billBoard, 40))
					{
						delete billBoard;
						billBoard = NULL;
						return FALSE;
					}
					EffectTimeObjectBuffer[timeObjectIndex].Start();
				}
			}

			HoEffectLight *light = new HoEffectLight();
			if(light != NULL)
			{
				light->Start(x, y, z, 200, 200, 256, 255, 100, 1);
				if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(light, 0))
				{
					delete light;
					light = NULL;
					return FALSE;
				}
				EffectTimeObjectBuffer[timeObjectIndex].Start();
			}
			
			
			light = new HoEffectLight();
			if(light != NULL)
			{
				light->Start(x, y, z, 100, 100, 200, 256, 300, 2);
				if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(light, 45))
				{
					delete light;
					light = NULL;
					return FALSE;
				}
				EffectTimeObjectBuffer[timeObjectIndex].Start();
			}
			

			//HoPrimitiveBillboard *extPrimitive;
			for(index = 0; index < 6; index++)
			{
				extPrimitive = NULL;
				extPrimitive = new HoPrimitiveBillboard();
				if(extPrimitive != NULL)
				{
					extPrimitive->StartBillRect(x, y+8000, z, "AutoMationLeft.ini", ANI_ONE);
					if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(extPrimitive, 45+index*2))
					{
						delete extPrimitive;
						extPrimitive = NULL;
						return FALSE;
					}
					EffectTimeObjectBuffer[timeObjectIndex].Start();
				}
				
				extPrimitive = NULL;
				extPrimitive = new HoPrimitiveBillboard();
				if(extPrimitive != NULL)
				{
					extPrimitive->StartBillRect(x, y+8000, z, "AutoMationRight.ini", ANI_ONE);
					if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(extPrimitive, 45+index*2))
					{
						delete extPrimitive;
						extPrimitive = NULL;
						return FALSE;
					}
					EffectTimeObjectBuffer[timeObjectIndex].Start();
				}
			}

			extPrimitive = NULL;
			extPrimitive = new HoPrimitiveBillboard();
			if(extPrimitive != NULL)
			{
				extPrimitive->StartBillRect(x, y+8000, z, "Pung1.ini", ANI_ONE);
				if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(extPrimitive, 45))
				{
					delete extPrimitive;
					extPrimitive = NULL;
					return FALSE;
				}
				EffectTimeObjectBuffer[timeObjectIndex].Start();
			}
			break;
		}
		
		//방패 강화...
		case SKILL_EXTREME_SHIELD:
		{
			SetDynLight( x, y, z, 230, 150, 150, 255, 200 , 1);
			StartBillRectPrimitive(x+rand()%50, y+rand()%50, z+rand()%50, 120, 120, "Light1.ini");

			int timeObjectIndex = EmptyTimeObjectBufferIndex();
			if(timeObjectIndex == -1)
				return FALSE;
			
			HoPrimitiveBillboard *extPrimitive = new HoPrimitiveBillboard();
			if(extPrimitive != NULL)
			{
				extPrimitive->StartBillRect(x, y, z, "SkillExtremeShieldBig.ini", ANI_ONE);
				if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(extPrimitive, 0))
				{
					delete extPrimitive;
					extPrimitive = NULL;
					return FALSE;
				}
				EffectTimeObjectBuffer[timeObjectIndex].Start();
			}
			
			
			extPrimitive = new HoPrimitiveBillboard();
			if(extPrimitive != NULL)
			{
				extPrimitive->StartBillRect(x, y, z, "SkillExtremeShieldBig.ini", ANI_ONE);
				if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(extPrimitive, 2))
				{
					delete extPrimitive;
					extPrimitive = NULL;
					return FALSE;
				}
				EffectTimeObjectBuffer[timeObjectIndex].Start();
			}
			
			int index; for( index = 0; index < 15; index++)
			{
				HoPrimitiveBillboard *primitiveBillBoard = new HoPrimitiveBillboard();
				POINT3D currentPos;
				currentPos.x = x;
				currentPos.y = y-4000;
				currentPos.z = z;
				primitiveBillBoard->StartPath(currentPos, "returnParticle1.ini", ANI_LOOP);
				
				int ang = rand()%ANGLE_360;
				HoPhysicsParticle *physicsParticle = new HoPhysicsParticle();
				POINT3D velocity;
				velocity.x = int(((float)GetCos[ang]/65536.f)*220.f);
				velocity.y = 200;
				velocity.z = int(((float)GetSin[ang]/65536.f)*220.f);

				physicsParticle->Start(velocity);
				physicsParticle->SetGravity(float(-rand()%6));
				physicsParticle->SetLive(rand()%20+40);
				
			
				primitiveBillBoard->SetPhysics(physicsParticle, PRIMITIVE_DEFAULT_RECT_PARTICLE);
				
				POINT size;
				size.x = 9;
				size.y = 9;
				primitiveBillBoard->SetSize(size);
				if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(primitiveBillBoard, 2))
				{
					delete primitiveBillBoard;
					primitiveBillBoard = NULL;
					return FALSE;
				}
				EffectTimeObjectBuffer[timeObjectIndex].Start();
			}
			
			
			extPrimitive = new HoPrimitiveBillboard();
			if(extPrimitive != NULL)
			{
				extPrimitive->StartBillRect(x, y, z, "SkillExtremeShieldSmall.ini", ANI_ONE);
				if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(extPrimitive, 15))
				{
					delete extPrimitive;
					extPrimitive = NULL;
					return FALSE;
				}
				EffectTimeObjectBuffer[timeObjectIndex].Start();
			}
			
			
			extPrimitive = new HoPrimitiveBillboard();
			if(extPrimitive != NULL)
			{
				extPrimitive->StartBillRect(x, y, z, "SkillExtremeShieldSmall.ini", ANI_ONE);
				if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(extPrimitive, 20))
				{
					delete extPrimitive;
					extPrimitive = NULL;
					return FALSE;
				}
				EffectTimeObjectBuffer[timeObjectIndex].Start();
			}
			break;
		}
		
		
		case SKILL_MAXIMIZE: 
		{
			int timeObjectIndex = EmptyTimeObjectBufferIndex();
			if(timeObjectIndex == -1)
				return FALSE;
			

			HoPrimitiveBillboard *extPrimitive = new HoPrimitiveBillboard();
			int index; for( index = 0; index < 20; index++)
			{
				HoPrimitiveBillboard *primitiveBillBoard = new HoPrimitiveBillboard();
				POINT3D currentPos;
				currentPos.x = x;
				currentPos.y = y-10000;
				currentPos.z = z;
				primitiveBillBoard->StartPath(currentPos, "PhysicalAbsorbParticle2.ini", ANI_LOOP);
				
				int ang = rand()%ANGLE_360;
				HoPhysicsParticle *physicsParticle = new HoPhysicsParticle();
				POINT3D velocity;
				velocity.x = int(((float)GetCos[ang]/65536.f)*120.f);
				velocity.y = 400;
				velocity.z = int(((float)GetSin[ang]/65536.f)*120.f);

				physicsParticle->Start(velocity);
				physicsParticle->SetGravity(float(rand()%3+2));
				physicsParticle->SetLive(rand()%20+30);
				
				POINT3D angle;
				angle.x = 0;
				angle.y = ANGLE_360/3;
				angle.z = 0;
				physicsParticle->SetDestAngle(angle);
				primitiveBillBoard->SetPhysics(physicsParticle, PRIMITIVE_DEFAULT_RECT_PARTICLE | MOVE_ANGLE);
				
				POINT size;
				size.x = 10;
				size.y = 10;
				primitiveBillBoard->SetSize(size);
				if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(primitiveBillBoard, index))
				{
					delete primitiveBillBoard;
					primitiveBillBoard = NULL;
					return FALSE;
				}
				EffectTimeObjectBuffer[timeObjectIndex].Start();
			}

			for(index = 0; index < 20; index++)
			{
				HoPrimitiveBillboard *primitiveBillBoard = new HoPrimitiveBillboard();
				POINT3D currentPos;
				currentPos.x = x;
				currentPos.y = y+4000;
				currentPos.z = z;
				primitiveBillBoard->StartPath(currentPos, "PhysicalAbsorbParticle2.ini", ANI_LOOP);
				
				int ang = rand()%ANGLE_360;
				HoPhysicsParticle *physicsParticle = new HoPhysicsParticle();
				POINT3D velocity;
				velocity.x = int(((float)GetCos[ang]/65536.f)*220.f);
				velocity.y = 200;
				velocity.z = int(((float)GetSin[ang]/65536.f)*220.f);

				physicsParticle->Start(velocity);
				physicsParticle->SetGravity(float(-rand()%6));
				physicsParticle->SetLive(rand()%20+40);
				
			
				primitiveBillBoard->SetPhysics(physicsParticle, PRIMITIVE_DEFAULT_RECT_PARTICLE);
				
				POINT size;
				size.x = 9;
				size.y = 9;
				primitiveBillBoard->SetSize(size);
				if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(primitiveBillBoard, 40))
				{
					delete primitiveBillBoard;
					primitiveBillBoard = NULL;
					return FALSE;
				}
				EffectTimeObjectBuffer[timeObjectIndex].Start();
			}

			for(index = 0; index < 30; index++)
			{
				HoPrimitiveBillboard *billBoard = new HoPrimitiveBillboard();
				if(billBoard != NULL)
				{
					int ang = rand()%ANGLE_360;

					//POINT3D currentPos;
					POINT3D worldPos;
					POINT3D localPos;
					POINT3D destPos;
					POINT	size;
					size.x = 20;
					size.y = 130+rand()%300;
					
					float RandomYaw = RANDOM_NUM   * 3.14159f * 2.0f;
					float RandomPitch = RANDOM_NUM * 180.f * 3.14159f / 180.0f ;
					
					//구모양으로 파티클이 날아가게 벡터를 조정한다.
					destPos.y = int(cosf( RandomPitch)*(100.f*100.f)+y+8000);
					destPos.x = int(sinf( RandomPitch)*150.f * cosf(RandomYaw)*150.f+x);
					destPos.z = int(sinf( RandomPitch)*150.f * sinf(RandomYaw)*150.f+z);
					
					worldPos.x = x;
					worldPos.y = y+8000;
					worldPos.z = z;

					localPos.y = int(cosf( RandomPitch)*(70.f*70.f));
					localPos.x = int(sinf( RandomPitch)*85.f * cosf(RandomYaw)*85.f);
					localPos.z = int(sinf( RandomPitch)*85.f * sinf(RandomYaw)*85.f);
					
					
					billBoard->StartDestPath(worldPos, localPos, destPos, size, "PhysicalAbsorbParticle1.ini", 600);
					if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(billBoard, 40))
					{
						delete billBoard;
						billBoard = NULL;
						return FALSE;
					}
					EffectTimeObjectBuffer[timeObjectIndex].Start();
				}
			}

			HoEffectLight *light = new HoEffectLight();
			if(light != NULL)
			{
				light->Start(x, y, z, 200, 200, 256, 255, 100, 1);
				if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(light, 0))
				{
					delete light;
					light = NULL;
					return FALSE;
				}
				EffectTimeObjectBuffer[timeObjectIndex].Start();
			}
			
			
			light = new HoEffectLight();
			if(light != NULL)
			{
				light->Start(x, y, z, 100, 100, 200, 256, 300, 2);
				if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(light, 45))
				{
					delete light;
					light = NULL;
					return FALSE;
				}
				EffectTimeObjectBuffer[timeObjectIndex].Start();
			}
			

			//HoPrimitiveBillboard *extPrimitive;
			for(index = 0; index < 6; index++)
			{
				extPrimitive = NULL;
				extPrimitive = new HoPrimitiveBillboard();
				if(extPrimitive != NULL)
				{
					extPrimitive->StartBillRect(x, y+8000, z, "MaximizeLeft.ini", ANI_ONE);
					if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(extPrimitive, 45+index*2))
					{
						delete extPrimitive;
						extPrimitive = NULL;
						return FALSE;
					}
					EffectTimeObjectBuffer[timeObjectIndex].Start();
				}
				
				extPrimitive = NULL;
				extPrimitive = new HoPrimitiveBillboard();
				if(extPrimitive != NULL)
				{
					extPrimitive->StartBillRect(x, y+8000, z, "MaximizeRight.ini", ANI_ONE);
					if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(extPrimitive, 45+index*2))
					{
						delete extPrimitive;
						extPrimitive = NULL;
						return FALSE;
					}
					EffectTimeObjectBuffer[timeObjectIndex].Start();
				}
			}

			extPrimitive = NULL;
			extPrimitive = new HoPrimitiveBillboard();
			if(extPrimitive != NULL)
			{
				extPrimitive->StartBillRect(x, y+8000, z, "Pung1.ini", ANI_ONE);
				if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(extPrimitive, 45))
				{
					delete extPrimitive;
					extPrimitive = NULL;
					return FALSE;
				}
				EffectTimeObjectBuffer[timeObjectIndex].Start();
			}
			break;
		}
		

		case SKILL_PHYSICAL_ABSORB: //잠시동안 힘을 키운다.
		{
			int timeObjectIndex = EmptyTimeObjectBufferIndex();
			if(timeObjectIndex == -1)
				return FALSE;
			
			HoPrimitiveBillboard *extPrimitive = new HoPrimitiveBillboard();
			int index; for( index = 0; index < 20; index++)
			{
				HoPrimitiveBillboard *primitiveBillBoard = new HoPrimitiveBillboard();
				POINT3D currentPos;
				currentPos.x = x;
				currentPos.y = y-10000;
				currentPos.z = z;
				primitiveBillBoard->StartPath(currentPos, "PhysicalAbsorbParticle2.ini", ANI_LOOP);
				
				int ang = rand()%ANGLE_360;
				HoPhysicsParticle *physicsParticle = new HoPhysicsParticle();
				POINT3D velocity;
				velocity.x = int(((float)GetCos[ang]/65536.f)*120.f);
				velocity.y = 400;
				velocity.z = int(((float)GetSin[ang]/65536.f)*120.f);

				physicsParticle->Start(velocity);
				physicsParticle->SetGravity(float(rand()%3+2));
				physicsParticle->SetLive(rand()%20+30);
				
				POINT3D angle;
				angle.x = 0;
				angle.y = ANGLE_360/3;
				angle.z = 0;
				physicsParticle->SetDestAngle(angle);
				primitiveBillBoard->SetPhysics(physicsParticle, PRIMITIVE_DEFAULT_RECT_PARTICLE | MOVE_ANGLE);
				
				POINT size;
				size.x = 10;
				size.y = 10;
				primitiveBillBoard->SetSize(size);
				if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(primitiveBillBoard, index))
				{
					delete primitiveBillBoard;
					primitiveBillBoard = NULL;
					return FALSE;
				}
				EffectTimeObjectBuffer[timeObjectIndex].Start();
			}

			for(index = 0; index < 20; index++)
			{
				HoPrimitiveBillboard *primitiveBillBoard = new HoPrimitiveBillboard();
				POINT3D currentPos;
				currentPos.x = x;
				currentPos.y = y+4000;
				currentPos.z = z;
				primitiveBillBoard->StartPath(currentPos, "PhysicalAbsorbParticle2.ini", ANI_LOOP);
				
				int ang = rand()%ANGLE_360;
				HoPhysicsParticle *physicsParticle = new HoPhysicsParticle();
				POINT3D velocity;
				velocity.x = int(((float)GetCos[ang]/65536.f)*220.f);
				velocity.y = 200;
				velocity.z = int(((float)GetSin[ang]/65536.f)*220.f);

				physicsParticle->Start(velocity);
				physicsParticle->SetGravity(float(-rand()%6));
				physicsParticle->SetLive(rand()%20+40);
				
			
				primitiveBillBoard->SetPhysics(physicsParticle, PRIMITIVE_DEFAULT_RECT_PARTICLE);
				
				POINT size;
				size.x = 9;
				size.y = 9;
				primitiveBillBoard->SetSize(size);
				if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(primitiveBillBoard, 40))
				{
					delete primitiveBillBoard;
					primitiveBillBoard = NULL;
					return FALSE;
				}
				EffectTimeObjectBuffer[timeObjectIndex].Start();
			}
				
			//기줄기..
			for(index = 0; index < 30; index++)
			{
				HoPrimitiveBillboard *billBoard = new HoPrimitiveBillboard();
				if(billBoard != NULL)
				{
					int ang = rand()%ANGLE_360;

					//POINT3D currentPos;
					POINT3D worldPos;
					POINT3D localPos;
					POINT3D destPos;
					POINT	size;
					size.x = 20;
					size.y = 130+rand()%300;
					
					float RandomYaw = RANDOM_NUM   * 3.14159f * 2.0f;
					float RandomPitch = RANDOM_NUM * 180.f * 3.14159f / 180.0f ;
					
					//구모양으로 파티클이 날아가게 벡터를 조정한다.
					destPos.y = int(cosf( RandomPitch)*(100.f*100.f)+y+8000);
					destPos.x = int(sinf( RandomPitch)*150.f * cosf(RandomYaw)*150.f+x);
					destPos.z = int(sinf( RandomPitch)*150.f * sinf(RandomYaw)*150.f+z);
					
					worldPos.x = x;
					worldPos.y = y+8000;
					worldPos.z = z;

					
					localPos.y = int(cosf( RandomPitch)*(50.f*50.f));
					localPos.x = int(sinf( RandomPitch)*75.f * cosf(RandomYaw)*75.f);
					localPos.z = int(sinf( RandomPitch)*75.f * sinf(RandomYaw)*75.f);
					

					billBoard->StartDestPath(worldPos, localPos, destPos, size, "PhysicalAbsorbParticle1.ini", 600);
					if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(billBoard, 40))
					{
						delete billBoard;
						billBoard = NULL;
						return FALSE;
					}
					EffectTimeObjectBuffer[timeObjectIndex].Start();
				}
			}
			
			HoEffectLight *light = new HoEffectLight();
			if(light != NULL)
			{
				//light->Start(x, y, z, 400, 400, 700, 255, 0, -1, 50);
				light->Start(x, y, z, 180, 180, 250, 255, 200, 1);
				if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(light, 0))
				{
					delete light;
					light = NULL;
					return FALSE;
				}
				EffectTimeObjectBuffer[timeObjectIndex].Start();
			}
			
			
			light = new HoEffectLight();
			if(light != NULL)
			{
				light->Start(x, y, z, 200, 200, 300, 255, 200, 2);
				if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(light, 45))
				{
					delete light;
					light = NULL;
					return FALSE;
				}
				EffectTimeObjectBuffer[timeObjectIndex].Start();
			}
			

			HoEffectPat	*pat = new HoEffectPat();
			if(pat != NULL)
			{
				pat->StartAni(x, y, z,angleX, angleY&ANGCLIP, angleZ, PatObj[2]);
				pat->AnimationEnd = PatAnimationEnd[2];
				
				if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(pat, 45))
				{
					delete pat;
					pat = NULL;
					return FALSE;
				}
				EffectTimeObjectBuffer[timeObjectIndex].Start();
			}
			pat->SetSkillCode(SKILL_PHYSICAL_ABSORB);
			
			extPrimitive = NULL;
			extPrimitive = new HoPrimitiveBillboard();
			if(extPrimitive != NULL)
			{
				extPrimitive->StartBillRect(x, y+8000, z, "Pung1.ini", ANI_ONE);
				if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(extPrimitive, 45))
				{
					delete extPrimitive;
					extPrimitive = NULL;
					return FALSE;
				}
				EffectTimeObjectBuffer[timeObjectIndex].Start();
			}
			break;
		}

		case SKILL_GROUND_PIKE://땅에 충격을 가한다.
		{
			int timeObjectIndex = EmptyTimeObjectBufferIndex();
			if(timeObjectIndex == -1)
				return FALSE;

			SetDynLight( x, y, z, 50, 100, 255, 255, 200+level*10, 3);
			int index; for( index = 0; index < 10; index++)
			{
				int ang = ANGLE_360/10*index;
				HoEffectPatDest *patDest = new HoEffectPatDest();
				POINT3D currentPos;
				POINT3D destPos;
				
				currentPos.x = x;
				currentPos.y = y;
				currentPos.z = z;
				
				destPos.x = int(x+(GetCos[ang]/65536.f)*(28000+level*(10000.f/10.f)));
				destPos.y = y;
				destPos.z = int(z+(GetSin[ang]/65536.f)*(28000+level*(10000.f/10.f)));

				if(patDest != NULL)
				{
					patDest->StartDest(currentPos, destPos, PatObj[1], SKILL_GROUND_PIKE);
					patDest->AnimationEnd = PatAnimationEnd[1];

					if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(patDest, rand()%10))
					{
						delete patDest;
						patDest = NULL;
						return FALSE;
					}
					EffectTimeObjectBuffer[timeObjectIndex].Start();
				}
				
				
				HoParticleDest *particleDest = new HoParticleDest;

				//int ang = ANGLE_360/10*index;
				if(particleDest != NULL)
				{
					
					particleDest->Start(x, y, z,
					destPos.x, 
					destPos.y, 
					destPos.z, MaterialNum[3], 0);
					particleDest->SetAge(0.29f+level*(0.2f/10.f));
					
					if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(particleDest, rand()%10))
					{
						delete particleDest;
						particleDest = NULL;
						return FALSE;
					}
					EffectTimeObjectBuffer[timeObjectIndex].Start();
				}
				
			}
			
			break;
		}
		
		//봉을 던진다.
		case SKILL_PIKE_WIND:
		{
			SetDynLight( x, y, z, 50, 50, 200, 255, 200+level*10 , 3);
			int timeObjectIndex = EmptyTimeObjectBufferIndex();
			if(timeObjectIndex == -1)
				return FALSE;
			
			//밀어내는 에니메이션..
			int index; for( index = 0; index < 15+level; index++)
			{
				int ang = ANGLE_360/(15+level/2)*index;
				HoEffectPat *pat = new HoEffectPat();
				if(pat != NULL)
				{
					pat->StartAni(x+int((GetCos[ang]/65536.f)*(level*650)), 
						y, 
						int(z+(GetSin[ang]/65536.f)*(level*650)), 0, ANGLE_180-ang, 0, PatObj[3]);

					pat->AnimationEnd = PatAnimationEnd[3];
					
					if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(pat, 8))
					{
						delete pat;
						pat = NULL;
						return FALSE;
					}
					
				}
			}

			EffectTimeObjectBuffer[timeObjectIndex].Start();
			
			y = y+500;
			for(index = 0; index < 15+level; index++)
			{
				HoParticleDest *particleDest = new HoParticleDest;
				int ang = ANGLE_360/(15+level)*index;
				if(particleDest != NULL)
				{
					particleDest->Start( int(x+(GetCos[ang]/65536.f)*(8000+level*325)), y,

						int(z+(GetSin[ang]/65536.f)*(8000+level*325)),
						int(x+(GetCos[ang]/65536.f)*(18000+level*325)+rand()%1000-500), y, 
						int(z+(GetSin[ang]/65536.f)*(18000+level*325)+rand()%1000-500), MaterialNum[0], 1);
					
					if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(particleDest, 10))
					{
						delete particleDest;
						particleDest = NULL;
						return FALSE;
					}
					EffectTimeObjectBuffer[timeObjectIndex].Start();
				}
			}
			

			break;
		}
		

		case SKILL_WIND_ARROW:
		{
			int timeObjectIndex = EmptyTimeObjectBufferIndex();
			if(timeObjectIndex == -1)
				return FALSE;
			
			HoEffectPat	*pat = new HoEffectPat();
			if(pat != NULL)
			{
				pat->StartAni(x, y, z,angleX, angleY, angleZ, PatObj[6]);
				pat->AnimationEnd = PatAnimationEnd[6];
				
				if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(pat, 0))
				{
					delete pat;
					pat = NULL;
					return FALSE;
				}
				EffectTimeObjectBuffer[timeObjectIndex].Start();
			}
			
			StartEffect( x, y, z,100,100,100,150, EFFECT_PARTICLE_BOW2 );

			break;
		}

		// 블러드 나이트 로어
		case EFFECT_BLOODKNIGHT_SKILL:
			{
     		int timeObjectIndex = EmptyTimeObjectBufferIndex();
			if(timeObjectIndex == -1)
				return FALSE;
			
			HoEffectLight *light = new HoEffectLight();
			if(light != NULL)
			{
				light->Start(x, y, z, 255, 10, 10, 255, 150, 2);
				if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(light, 0))
				{
					delete light;
					light = NULL;
					return FALSE;
				}
				EffectTimeObjectBuffer[timeObjectIndex].Start();
			}
			
			
			light = new HoEffectLight();
			if(light != NULL)
			{
				light->Start(x, y, z, 255, 10, 10, 255, 200, 3);
				if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(light, 45))
				{
					delete light;
					light = NULL;
					return FALSE;
				}
				EffectTimeObjectBuffer[timeObjectIndex].Start();
			}
			
 			int index; for( index = 0; index < 4+level/5; index++)
			{
				HoEffectPat	*pat = new HoEffectPat();
				if(pat != NULL)
				{
					pat->StartAni(x, y, z,angleX, angleY&ANGCLIP, angleZ, PatObj[10]);
					pat->AnimationEnd = 9+(level/5); //PatAnimationEnd[10];
					
					if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(pat, 45+index*2))
					{
						delete pat;
						pat = NULL;
						return FALSE;
					}
					EffectTimeObjectBuffer[timeObjectIndex].Start();
				}
			}

			POINT3D partPos;
    		partPos.x = x;
			partPos.y = y+3000;
			partPos.z = z;
  			g_NewParticleMgr.Start("Skill2FighterRoar1",partPos, 0.5f);

			break;
		}

		//키메라 헤딩
		case EFFECT_CHIMERA_SKILL:
		{
			SetDynLight( x, y, z, 230, 150, 150, 255, 200 , 1);
			StartBillRectPrimitive(x+rand()%50, y+rand()%50, z+rand()%50, 120, 120, "Light1.ini");

			int timeObjectIndex = EmptyTimeObjectBufferIndex();
			if(timeObjectIndex == -1)
				return FALSE;
			
			HoPrimitiveBillboard *extPrimitive = new HoPrimitiveBillboard();
			if(extPrimitive != NULL)
			{
				extPrimitive->StartBillRect(x, y, z, "SkillExtremeShieldBig.ini", ANI_ONE);
				if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(extPrimitive, 0))
				{
					delete extPrimitive;
					extPrimitive = NULL;
					return FALSE;
				}
				EffectTimeObjectBuffer[timeObjectIndex].Start();
			}
			
			
			extPrimitive = new HoPrimitiveBillboard();
			if(extPrimitive != NULL)
			{
				extPrimitive->StartBillRect(x, y, z, "SkillExtremeShieldBig.ini", ANI_ONE);
				if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(extPrimitive, 2))
				{
					delete extPrimitive;
					extPrimitive = NULL;
					return FALSE;
				}
				EffectTimeObjectBuffer[timeObjectIndex].Start();
			}
			
			int index; for( index = 0; index < 15; index++)
			{
				HoPrimitiveBillboard *primitiveBillBoard = new HoPrimitiveBillboard();
				POINT3D currentPos;
				currentPos.x = x;
				currentPos.y = y-4000;
				currentPos.z = z;
				primitiveBillBoard->StartPath(currentPos, "returnParticle1.ini", ANI_LOOP);
				
				int ang = rand()%ANGLE_360;
				HoPhysicsParticle *physicsParticle = new HoPhysicsParticle();
				POINT3D velocity;
				velocity.x = int(((float)GetCos[ang]/65536.f)*220.f);
				velocity.y = 200;
				velocity.z = int(((float)GetSin[ang]/65536.f)*220.f);

				physicsParticle->Start(velocity);
				physicsParticle->SetGravity(float(-rand()%6));
				physicsParticle->SetLive(rand()%20+40);
				
			
				primitiveBillBoard->SetPhysics(physicsParticle, PRIMITIVE_DEFAULT_RECT_PARTICLE);
				
				POINT size;
				size.x = 9;
				size.y = 9;
				primitiveBillBoard->SetSize(size);
				if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(primitiveBillBoard, 2))
				{
					delete primitiveBillBoard;
					primitiveBillBoard = NULL;
					return FALSE;
				}
				EffectTimeObjectBuffer[timeObjectIndex].Start();
			}
			
			
			extPrimitive = new HoPrimitiveBillboard();
			if(extPrimitive != NULL)
			{
				extPrimitive->StartBillRect(x, y, z, "SkillExtremeShieldSmall.ini", ANI_ONE);
				if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(extPrimitive, 15))
				{
					delete extPrimitive;
					extPrimitive = NULL;
					return FALSE;
				}
				EffectTimeObjectBuffer[timeObjectIndex].Start();
			}
			
			
			extPrimitive = new HoPrimitiveBillboard();
			if(extPrimitive != NULL)
			{
				extPrimitive->StartBillRect(x, y, z, "SkillExtremeShieldSmall.ini", ANI_ONE);
				if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(extPrimitive, 20))
				{
					delete extPrimitive;
					extPrimitive = NULL;
					return FALSE;
				}
				EffectTimeObjectBuffer[timeObjectIndex].Start();
			}
			break;
		}
		
		
			
		case EFFECT_TEST2:
		{
			int timeObjectIndex = EmptyTimeObjectBufferIndex();
			if(timeObjectIndex == -1)
				return FALSE;
			
			HoEffectPat	*pat = new HoEffectPat();
			if(pat != NULL)
			{
				pat->StartAni(x, y, z,angleX, angleY, angleZ, PatObj[8]);
				pat->AnimationEnd = PatAnimationEnd[8];
				
				if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(pat, 0))
				{
					delete pat;
					pat = NULL;
					return FALSE;
				}
				EffectTimeObjectBuffer[timeObjectIndex].Start();
			}
			break;
		}
		
		case EFFECT_TEST1:
		{
			int timeObjectIndex = EmptyTimeObjectBufferIndex();
			if(timeObjectIndex == -1)
				return FALSE;

//#ifdef _DEBUG
	
			HoPrimitiveBillboard *extPrimitive = new HoPrimitiveBillboard();
			if(extPrimitive != NULL)
			{
				if(extPrimitive->StartBillRect(x, y, z, "AniLayer1.ini", ANI_ONE) == FALSE)
				{
					delete extPrimitive;
					extPrimitive = NULL;
					
				}
				else
				{
					if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(extPrimitive, 0))
					{
						if(extPrimitive)
						{
							delete extPrimitive;
							extPrimitive = NULL;
						}
					}
					else
						EffectTimeObjectBuffer[timeObjectIndex].Start();
				}
			}

			extPrimitive = new HoPrimitiveBillboard();
			if(extPrimitive != NULL)
			{
				if(extPrimitive->StartBillRect(x, y, z, "AniLayer2.ini", ANI_ONE) == FALSE)
				{
					delete extPrimitive;
					extPrimitive = NULL;
					
				}
				else
				{
					if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(extPrimitive, 0))
					{
						if(extPrimitive)
						{
							delete extPrimitive;
							extPrimitive = NULL;
						}
					}
					else
						EffectTimeObjectBuffer[timeObjectIndex].Start();
				}
			}

			extPrimitive = new HoPrimitiveBillboard();
			if(extPrimitive != NULL)
			{
				if(extPrimitive->StartBillRect(x, y, z, "AniLayer3.ini", ANI_ONE) == FALSE)
				{
					delete extPrimitive;
					extPrimitive = NULL;
				
				}
				else
				{
					if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(extPrimitive, 0))
					{
						if(extPrimitive)
						{
							delete extPrimitive;
							extPrimitive = NULL;
						}
					}
					else
						EffectTimeObjectBuffer[timeObjectIndex].Start();
				}
			}

//#endif
			break;
		}
	}
	return TRUE;
}

//static int	SkillScoutHawkTrun[10] = {3,3,4,4,5,5,6,7,8,9};
extern int Scout_Hawk_UseTime[10];
int HoEffectMgr::StartSkill(int x, int y, int z, int angleX, int angleY, int angleZ, smCHAR *character, int skillType, int level)
{
	switch(skillType)
	{
		/*
		case EFFECT_POWER1:
		{
			int timeObjectIndex = EmptyTimeObjectBufferIndex();
			if(timeObjectIndex == -1)
				return FALSE;
			
			HoEffectPower *effectPower = new HoEffectPower();
			effectPower->Start(x, y, z, character);
			
			if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(effectPower, 0))
			{
				delete effectPower;
				effectPower = NULL;
				return FALSE;
			}
			EffectTimeObjectBuffer[timeObjectIndex].Start();
			break;
		}
		*/
		
		case SKILL_SHIELD1:
		{
			int timeObjectIndex = EmptyTimeObjectBufferIndex();
			if(timeObjectIndex == -1)
				return FALSE;

			//Effect에 character 정보가 들어가는 경우.
			int index; for( index = 0; index < CHR_IN_EFFECT_OBJECT_MAX; index++)
			{
				if(ChrInEffectObject[index].object != NULL && character != NULL)
				{
					if(ChrInEffectObject[index].object->GetSkillCode() == SKILL_SHIELD1
						&& ((HoEffectShield *)ChrInEffectObject[index].object)->Character == character)
					{
						((HoEffectShield *)ChrInEffectObject[index].object)->Init();
						ChrInEffectObject[index].object = NULL;
					}
				}
			}
			
			HoEffectShield *effectShield = new HoEffectShield();
			effectShield->SetSkillCode(SKILL_SHIELD1);
			effectShield->Start(x, y+6000, z, SKILL_SHIELD1, ANGLE_45, 3000+(level*100), character);
			if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(effectShield, 0))
			{
				delete effectShield;
				effectShield = NULL;
				return FALSE;
			}
			EffectTimeObjectBuffer[timeObjectIndex].Start();

			
			//Effect에 character 정보가 들어가는 경우.
			for(index = 0; index < CHR_IN_EFFECT_OBJECT_MAX; index++)
			{
				if(ChrInEffectObject[index].object == NULL)
				{
					if(effectShield != NULL && character != NULL)
					{
						ChrInEffectObject[index].object = effectShield;
						break;
					}
				}
			}
			
			
			effectShield = new HoEffectShield();
			effectShield->SetSkillCode(SKILL_SHIELD1);
			effectShield->Start(x, y+6000, z, SKILL_SHIELD1, ANGLE_360-ANGLE_45, 2500+(level*100), character);
			
			if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(effectShield, 8))
			{
				delete effectShield;
				effectShield = NULL;
				return FALSE;
			}
			//Effect에 character 정보가 들어가는 경우.
			for(index = 0; index < CHR_IN_EFFECT_OBJECT_MAX; index++)
			{
				if(ChrInEffectObject[index].object == NULL)
				{
					if(effectShield != NULL && character != NULL)
					{
						ChrInEffectObject[index].object = effectShield;
						break;
					}
				}
			}
			
			break;
		}

		/*
		//ktj : 보류
		//ktj : SKILL_SHIELD1의 색깔을 흰색으로 바꿧다==================================================start
		case SKILL_SHIELD1_WHITE:
		{
			int timeObjectIndex = EmptyTimeObjectBufferIndex();
			if(timeObjectIndex == -1)
				return FALSE;

			//Effect에 character 정보가 들어가는 경우.
			for(int index = 0; index < CHR_IN_EFFECT_OBJECT_MAX; index++)
			{
				if(ChrInEffectObject[index].object != NULL && character != NULL)
				{
					if(ChrInEffectObject[index].object->GetSkillCode() == SKILL_SHIELD1
						&& ((HoEffectShield *)ChrInEffectObject[index].object)->Character == character)
					{
						((HoEffectShield *)ChrInEffectObject[index].object)->Init();
						ChrInEffectObject[index].object = NULL;
					}
				}
			}

			
			HoEffectShield *effectShield = new HoEffectShield();
			effectShield->SetSkillCode(SKILL_SHIELD1);
			effectShield->Start(x, y+6000, z, SKILL_SHIELD1, ANGLE_45, 3000+(level*100), character);
			if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(effectShield, 0))
			{
				delete effectShield;
				effectShield = NULL;
				return FALSE;
			}
			EffectTimeObjectBuffer[timeObjectIndex].Start();

			
			//Effect에 character 정보가 들어가는 경우.
			for(index = 0; index < CHR_IN_EFFECT_OBJECT_MAX; index++)
			{
				if(ChrInEffectObject[index].object == NULL)
				{
					if(effectShield != NULL && character != NULL)
					{
						ChrInEffectObject[index].object = effectShield;
						break;
					}
				}
			}
			
			
			effectShield = new HoEffectShield();
			effectShield->SetSkillCode(SKILL_SHIELD1);
			effectShield->Start(x, y+6000, z, SKILL_SHIELD1, ANGLE_360-ANGLE_45, 2500+(level*100), character);
			
			if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(effectShield, 8))
			{
				delete effectShield;
				effectShield = NULL;
				return FALSE;
			}
			//Effect에 character 정보가 들어가는 경우.
			for(index = 0; index < CHR_IN_EFFECT_OBJECT_MAX; index++)
			{
				if(ChrInEffectObject[index].object == NULL)
				{
					if(effectShield != NULL && character != NULL)
					{
						ChrInEffectObject[index].object = effectShield;
						break;
					}
				}
			}
			
			break;
		}

		//=============================================================================END
		*/

		//Automation 임...
		case SKILL_SHIELD2:
		{
			int timeObjectIndex = EmptyTimeObjectBufferIndex();
			if(timeObjectIndex == -1)
				return FALSE;
			
			HoEffectShield *effectShield = new HoEffectShield();
			effectShield->SetSkillCode(SKILL_SHIELD2);

			effectShield->Start(x, y+6000, z, SKILL_SHIELD2, ANGLE_45, 3000+(level*100), character);
			if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(effectShield, 0))
			{
				delete effectShield;
				effectShield = NULL;
				return FALSE;
			}

			effectShield = new HoEffectShield();
			effectShield->SetSkillCode(SKILL_SHIELD2);
			effectShield->Start(x, y+6000, z, SKILL_SHIELD2, ANGLE_360-ANGLE_45, 2500+(level*100), character);
			
			if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(effectShield, 8))
			{
				delete effectShield;
				effectShield = NULL;
				return FALSE;
			}
			EffectTimeObjectBuffer[timeObjectIndex].Start();
			break;
		}
		
		case SKILL_GOLDEN_FALCON:
		{
			int timeObjectIndex = EmptyTimeObjectBufferIndex();
			if(timeObjectIndex == -1)
				return FALSE;

			 HoEffectPatFalcon *patFalcon = new HoEffectPatFalcon();
			if(patFalcon != NULL)
			{
  				patFalcon->SetSkillCode(SKILL_GOLDEN_FALCON);
				patFalcon->StartGoldenFalconAni(x, y, z,angleX, angleY, angleZ, PatObj[25], PatObj[26], character , level);
				patFalcon->AnimationEnd = PatAnimationEnd[15];
				
				if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(patFalcon, 0))
				{
					delete patFalcon;
					patFalcon = NULL;
					return FALSE;
				}

				EffectTimeObjectBuffer[timeObjectIndex].Start();
			}
			break;
		}

		case SKILL_FALCON:
		{
			int timeObjectIndex = EmptyTimeObjectBufferIndex();
			if(timeObjectIndex == -1)
				return FALSE;

			HoEffectPatFalcon *patFalcon = new HoEffectPatFalcon();
			if(patFalcon != NULL)
			{
				patFalcon->SetSkillCode(SKILL_FALCON);
				patFalcon->StartAni(x, y, z,angleX, angleY, angleZ, PatObj[14], PatObj[15], character , level );
				patFalcon->AnimationEnd = PatAnimationEnd[15];
				
				if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(patFalcon, 0))
				{
					delete patFalcon;
					patFalcon = NULL;
					return FALSE;
				}

				EffectTimeObjectBuffer[timeObjectIndex].Start();
			}
			break;
		}

		case SKILL_SCOUT_HAWK: //매를 소환..
		{
			int timeObjectIndex = EmptyTimeObjectBufferIndex();
			if(timeObjectIndex == -1)
				return FALSE;
			
			if(level < 1 || level > 10)
				return FALSE;
			
			//Effect에 character 정보가 들어가는 경우.
			for(int index = 0; index < CHR_IN_EFFECT_OBJECT_MAX; index++)
			{
				if(ChrInEffectObject[index].object != NULL && character != NULL)
				{
					if(ChrInEffectObject[index].object->GetSkillCode() == SKILL_SCOUT_HAWK
						&& ((HoEffectPatHawk *)ChrInEffectObject[index].object)->Character == character)
					{
						((HoEffectPatHawk *)ChrInEffectObject[index].object)->Loop = 0;
						ChrInEffectObject[index].object = NULL;
						break;
					}
				}
			}

			HoEffectPatHawk *patHawk = new HoEffectPatHawk();
			if(patHawk != NULL)
			{
				patHawk->SetSkillCode(SKILL_SCOUT_HAWK);
				patHawk->StartAni(x, y, z,angleX, angleY, angleZ, PatObj[4], PatObj[5],character, Scout_Hawk_UseTime[level-1]);
				patHawk->AnimationEnd = PatAnimationEnd[5];
				
				if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(patHawk, 5))
				{
					delete patHawk;
					patHawk = NULL;
					return FALSE;
				}
				
				//Effect에 character 정보가 들어가는 경우.
				for(int index = 0; index < CHR_IN_EFFECT_OBJECT_MAX; index++)
				{
					if(ChrInEffectObject[index].object == NULL)
					{
						if(patHawk != NULL && character != NULL)
						{
							ChrInEffectObject[index].object = patHawk;
							break;
						}
					}
				}
				EffectTimeObjectBuffer[timeObjectIndex].Start();
			}
			break;
		}
	}
	return TRUE;
}


int HoEffectMgr::Start(int x, int y, int z, int effectType, HoEffectObject *parent)
{
	switch(effectType)
	{
		case EFFECT_SPARK1:
		{
			int timeObjectIndex = EmptyTimeObjectBufferIndex();
			if(timeObjectIndex == -1)
				return FALSE;

			HoPrimitiveBillboard *primitiveBillBoard = new HoPrimitiveBillboard();
			primitiveBillBoard->StartBillRect(x, y, z, "SkillSpark2.ini", ANI_ONE);
			primitiveBillBoard->SetParentObject(parent);
			if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(primitiveBillBoard, 0))
			{
				delete primitiveBillBoard;
				primitiveBillBoard = NULL;
				return FALSE;
			}
			EffectTimeObjectBuffer[timeObjectIndex].Start();
			
			for(int index = 0; index < 3; index++)
			{
				HoPrimitiveBillboard *billBoard = new HoPrimitiveBillboard();
				if(billBoard != NULL)
				{
					int ang = rand()%ANGLE_360;

					//POINT3D currentPos;
					POINT3D worldPos;
					POINT3D localPos;
					POINT3D destPos;
					POINT	size;

					size.x = 3;
					size.y = 400;
					
					float RandomYaw = RANDOM_NUM   * 3.14159f * 2.0f;
					float RandomPitch = RANDOM_NUM * 180.f * 3.14159f / 180.0f ;
					
					//구모양으로 파티클이 날아가게 벡터를 조정한다.
					destPos.y = int(cosf( RandomPitch)*((35.f)*(35.f))+y);
					destPos.x = int(sinf( RandomPitch)*(50.f) * cosf(RandomYaw)*(50.f)+x);
					destPos.z = int(sinf( RandomPitch)*(50.f) * sinf(RandomYaw)*(50.f)+z);
					
					worldPos.x = x;
					worldPos.y = y;
					worldPos.z = z;

					localPos.y = rand()%1000-500;
					localPos.x = rand()%1000-500;
					localPos.z = rand()%1000-500;
					
					billBoard->StartDestPath(worldPos, localPos, destPos, size, "SkillSpark1.ini", 300);
					billBoard->SetParentObject(parent);
					if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(billBoard, 0))
					{
						delete billBoard;
						billBoard = NULL;
						return FALSE;
					}
					EffectTimeObjectBuffer[timeObjectIndex].Start();
				}
			}
			break;
		}

		case EFFECT_SHIELD2_PARTICLE:
		{
			int timeObjectIndex = EmptyTimeObjectBufferIndex();
			if(timeObjectIndex == -1)
				return FALSE;

			int index; for( index = 0; index < 1; index++)
			{
				HoPrimitiveBillboard *billBoard = new HoPrimitiveBillboard();
				if(billBoard != NULL)
				{
					int ang = rand()%ANGLE_360;

					//POINT3D currentPos;
					POINT3D worldPos;
					POINT3D localPos;
					POINT3D destPos;
					POINT	size;
					size.x = 20;
					size.y = 450;

					destPos.x = int((GetCos[ang]/65536.f)*2000.f)+x;
					destPos.y = y+4000;
					destPos.z = int((GetSin[ang]/65536.f)*2000.f)+z;
					
					worldPos.x = x;
					worldPos.y = y;
					worldPos.z = z;

					localPos.x = int((GetCos[ang]/65536.f)*2000.f);
					localPos.y = 0;
					localPos.z = int((GetSin[ang]/65536.f)*2000.f);
					
					billBoard->StartDestPath(worldPos, localPos, destPos, size, "Shield2.ini", 300,PRIMITIVE_PATH_RECT_STRETCH_LINE);
					if(parent != NULL)
					billBoard->SetParentObject(parent);

					if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(billBoard, index))
					{
						delete billBoard;
						billBoard = NULL;
						return FALSE;
					}
					EffectTimeObjectBuffer[timeObjectIndex].Start();
				}
			}
		}
		break;
	}

	return TRUE;
}

int HoEffectMgr::Start(int x, int y, int z, int effectType, int level)
{
	int objIndex;
	
	switch(effectType)
	{
		case EFFECT_AGING:
		{
			POINT3D pos; 
			pos.x = x;
			pos.y = y;
			pos.z = z;

			SetDynLight( x, y, z, 255, 255, 255, 255, 200, 1);
			pos.y += 10000;
			g_NewParticleMgr.Start("aging", pos);
			break;
		}
		

		case EFFECT_SPARK1:
		{
			int timeObjectIndex = EmptyTimeObjectBufferIndex();
			if(timeObjectIndex == -1)
				return FALSE;

			HoPrimitiveBillboard *primitiveBillBoard = new HoPrimitiveBillboard();
			primitiveBillBoard->StartBillRect(x, y, z, "SkillSpark2.ini", ANI_ONE);
			if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(primitiveBillBoard, 0))
			{
				delete primitiveBillBoard;
				primitiveBillBoard = NULL;
				return FALSE;
			}
			EffectTimeObjectBuffer[timeObjectIndex].Start();
			
			int index; for( index = 0; index < 3; index++)
			{
				HoPrimitiveBillboard *billBoard = new HoPrimitiveBillboard();
				if(billBoard != NULL)
				{
					int ang = rand()%ANGLE_360;

					//POINT3D currentPos;
					POINT3D worldPos;
					POINT3D localPos;
					POINT3D destPos;
					POINT	size;

					size.x = 3;
					size.y = 500;
					
					float RandomYaw = RANDOM_NUM   * 3.14159f * 2.0f;
					float RandomPitch = RANDOM_NUM * 180.f * 3.14159f / 180.0f ;
					
					//구모양으로 파티클이 날아가게 벡터를 조정한다.
					destPos.y = int(cosf( RandomPitch)*((30.f)*(30.f))+y);
					destPos.x = int(sinf( RandomPitch)*(40.f) * cosf(RandomYaw)*(40.f)+x);
					destPos.z = int(sinf( RandomPitch)*(40.f) * sinf(RandomYaw)*(40.f)+z);
					
					worldPos.x = x;
					worldPos.y = y;
					worldPos.z = z;

					localPos.y = rand()%1000-500;
					localPos.x = rand()%1000-500;
					localPos.z = rand()%1000-500;
					
					billBoard->StartDestPath(worldPos, localPos, destPos, size, "SkillSpark1.ini", 300);
					if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(billBoard, 0))
					{
						delete billBoard;
						billBoard = NULL;
						return FALSE;
					}
					EffectTimeObjectBuffer[timeObjectIndex].Start();
				}
			}
			break;
		}

		case EFFECT_FALCON_GATE1:
		{
			SetDynLight( x, y, z, 180, 180, 180, 255, 200 , 1);
			int timeObjectIndex = EmptyTimeObjectBufferIndex();
			if(timeObjectIndex == -1)
				return FALSE;
			y = y-2000;
			int index; for( index = 0; index < 20; index++)
			{
				HoPrimitiveBillboard *primitiveBillBoard = new HoPrimitiveBillboard();
				POINT3D currentPos;
				currentPos.x = x;
				currentPos.y = y-5000;
				currentPos.z = z;
				primitiveBillBoard->StartPath(currentPos, "ReturnParticle1.ini", ANI_LOOP);
				
				int ang = rand()%ANGLE_360;
				HoPhysicsParticle *physicsParticle = new HoPhysicsParticle();
				POINT3D velocity;
				velocity.x = int(((float)GetCos[ang]/65536.f)*50.f);
				velocity.y = 100;
				velocity.z = int(((float)GetSin[ang]/65536.f)*50.f);

				physicsParticle->Start(velocity);
				physicsParticle->SetGravity(float(rand()%5+3));
				physicsParticle->SetLive(rand()%20+40);
				
				POINT3D angle;
				angle.x = 0;
				angle.y = ANGLE_360/3;
				angle.z = 0;
				physicsParticle->SetDestAngle(angle);
				primitiveBillBoard->SetPhysics(physicsParticle, PRIMITIVE_DEFAULT_RECT_PARTICLE | MOVE_ANGLE);
				
				POINT size;
				size.x = 7;
				size.y = 7;
				primitiveBillBoard->SetSize(size);
				if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(primitiveBillBoard, index*2))
				{
					delete primitiveBillBoard;
					primitiveBillBoard = NULL;
					return FALSE;
				}
				EffectTimeObjectBuffer[timeObjectIndex].Start();
			}
			
			HoEffectLight *light = new HoEffectLight();
			if(light != NULL)
			{
				light->Start(x, y, z, 250, 250, 250, 255, 100, 1);
				if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(light, 10))
				{
					delete light;
					light = NULL;
					return FALSE;
				}
				EffectTimeObjectBuffer[timeObjectIndex].Start();
			}
			
			y = y-2000;
			for(index = 0; index < 10; index++)
			{
				HoPrimitiveBillboard *billBoard = new HoPrimitiveBillboard();
				if(billBoard != NULL)
				{
					int ang = ANGLE_360/10*index;

					//POINT3D currentPos;
					POINT3D worldPos;
					POINT3D localPos;
					POINT3D destPos;
					POINT	size;
					size.x = 10;
					size.y = 400;
					

					destPos.x = int((GetCos[ang]/65536.f)*1600.f)+x;
					destPos.y = y+600+rand()%400;
					destPos.z = int((GetSin[ang]/65536.f)*1600.f)+z;

					worldPos.x = x;
					worldPos.y = y;
					worldPos.z = z;

					localPos.x = int((GetCos[ang]/65536.f)*1600.f);
					localPos.y = 0;
					localPos.z = int((GetSin[ang]/65536.f)*1600.f);
					
					billBoard->StartDestPath(worldPos, localPos, destPos, size, "Return1.ini", 600,PRIMITIVE_PATH_RECT_STRETCH_LINE);
					if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(billBoard, index*2+10))
					{
						delete billBoard;
						billBoard = NULL;
						return FALSE;
					}
					EffectTimeObjectBuffer[timeObjectIndex].Start();
				}
			}

			break;
		}

		case EFFECT_GROUND_PIKE_PARTICLE:
		{
			int timeObjectIndex = EmptyTimeObjectBufferIndex();
			if(timeObjectIndex == -1)
				return FALSE;

			int index; for( index = 0; index < 8; index++)
			{
				HoPrimitiveBillboard *billBoard = new HoPrimitiveBillboard();
				if(billBoard != NULL)
				{
					int ang = rand()%ANGLE_360;

					//POINT3D currentPos;
					POINT3D worldPos;
					POINT3D localPos;
					POINT3D destPos;
					POINT	size;
					size.x = 10;
					size.y = 200+rand()%50;
					
					float RandomYaw = RANDOM_NUM   * 3.14159f * 2.0f;
					float RandomPitch = RANDOM_NUM * 180.f * 3.14159f / 180.0f ;
					
					//구모양으로 파티클이 날아가게 벡터를 조정한다.
					destPos.y = int(cosf( RandomPitch)*((90.f)*(90.f))+y);
					destPos.x = int(sinf( RandomPitch)*(100.f) * cosf(RandomYaw)*(100.f)+x);
					destPos.z = int(sinf( RandomPitch)*(100.f) * sinf(RandomYaw)*(100.f)+z);
					
					worldPos.x = x;
					worldPos.y = y;
					worldPos.z = z;

					localPos.x = 0;
					localPos.y = 0;
					localPos.z = 0;
					
					billBoard->StartDestPath(worldPos, localPos
						, destPos, size, "MonsterMephit2.ini", 200,PRIMITIVE_PATH_RECT_LINE);
					if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(billBoard, index*rand()%5))
					{
						delete billBoard;
						billBoard = NULL;
						return FALSE;
					}
					EffectTimeObjectBuffer[timeObjectIndex].Start();
				}
			}
			break;
		}

		case EFFECT_ARROW_OF_RAGE_HIT1:
		{
			SetDynLight( x, y, z, 255, 50, 50, 255, 80, 2);
			int timeObjectIndex = EmptyTimeObjectBufferIndex();
			if(timeObjectIndex == -1)
				return FALSE;
			
			int index; for( index = 0; index < 5; index++)
			{
				HoPrimitiveBillboard *primitiveBillBoard = new HoPrimitiveBillboard();
				POINT3D currentPos;
				currentPos.x = x;
				currentPos.y = y-2000;
				currentPos.z = z;
				primitiveBillBoard->StartPath(currentPos, "SkillMechanicBombDust1.ini", ANI_LOOP);
				
				int ang = rand()%ANGLE_360;
				HoPhysicsParticle *physicsParticle = new HoPhysicsParticle();
				POINT3D velocity;
				velocity.x = int(((float)GetCos[ang]/65536.f)*100.f);
				velocity.y = 100;
				velocity.z = int(((float)GetSin[ang]/65536.f)*100.f);

				physicsParticle->Start(velocity);
				physicsParticle->SetGravity(float(rand()%3+6));
				physicsParticle->SetLive(rand()%20+10);
				
				POINT3D angle;
				angle.x = 0;
				angle.y = ANGLE_360/3;
				angle.z = 0;
				physicsParticle->SetDestAngle(angle);
				primitiveBillBoard->SetPhysics(physicsParticle, PRIMITIVE_DEFAULT_RECT_PARTICLE | MOVE_ANGLE);
				//primitiveBillBoard->SetPhysics(physicsParticle, PRIMITIVE_DEFAULT_RECT_PARTICLE);


				POINT size;
				size.x = 10;
				size.y = 10;
				primitiveBillBoard->SetSize(size);
				if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(primitiveBillBoard, index))
				{
					delete primitiveBillBoard;
					primitiveBillBoard = NULL;
					return FALSE;
				}
				EffectTimeObjectBuffer[timeObjectIndex].Start();
			}

			break;
		}

		case ARROW_OF_RAGE_POWER1:
		{

			int timeObjectIndex = EmptyTimeObjectBufferIndex();
			if(timeObjectIndex == -1)
				return FALSE;
			
			HoPrimitiveBillboard *primitiveBillBoard = new HoPrimitiveBillboard();
			primitiveBillBoard->StartBillRect(x, y, z, "SkillArrowOfRage1.ini", ANI_ONE);
			if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(primitiveBillBoard, 0))
			{
				delete primitiveBillBoard;
				primitiveBillBoard = NULL;
				return FALSE;
			}
			EffectTimeObjectBuffer[timeObjectIndex].Start();

			break;
		}
		
		//Spark를 맞았을 경우..
		case EFFECT_SPARK2:
		{
   			SetDynLight( x, y, z, 50, 50, 255, 255, 300, 2);
			int timeObjectIndex = EmptyTimeObjectBufferIndex();
			if(timeObjectIndex == -1)
				return FALSE;
			int index; for( index = 0; index < 2; index++)
			{
				HoPrimitiveBillboard *primitiveBillBoard = new HoPrimitiveBillboard();
				primitiveBillBoard->StartBillRect(x, y, z, "SkillSpark3.ini", ANI_ONE);
				if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(primitiveBillBoard, index*4))
				{
					delete primitiveBillBoard;
					primitiveBillBoard = NULL;
					return FALSE;
				}
			}
			EffectTimeObjectBuffer[timeObjectIndex].Start();
			
			/*
			for(index = 0; index < 15; index++)
			{
				HoPrimitiveBillboard *primitiveBillBoard = new HoPrimitiveBillboard();
				POINT3D currentPos;
				currentPos.x = x;
				currentPos.y = y;
				currentPos.z = z;
				primitiveBillBoard->StartPath(currentPos, "SkillSpark4.ini", ANI_LOOP);
				
				int ang = rand()%ANGLE_360;
				HoPhysicsParticle *physicsParticle = new HoPhysicsParticle();
				POINT3D velocity;
				velocity.x = int(((float)GetCos[ang]/65536.f)*200.f);

				velocity.y = 100;
				velocity.z = int(((float)GetSin[ang]/65536.f)*200.f);

				physicsParticle->Start(velocity);
				physicsParticle->SetGravity(float(-rand()%6));
				physicsParticle->SetLive(rand()%20+40);
				
			
				primitiveBillBoard->SetPhysics(physicsParticle, PRIMITIVE_DEFAULT_RECT_PARTICLE);
				
				POINT size;
				size.x = 9;
				size.y = 9;
				primitiveBillBoard->SetSize(size);
				if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(primitiveBillBoard, 0))
				{
					delete primitiveBillBoard;
					primitiveBillBoard = NULL;
					return FALSE;
				}
				EffectTimeObjectBuffer[timeObjectIndex].Start();
			}
			*/
			/*
			for(index = 0; index < 25; index++)
			{
				HoPrimitiveBillboard *billBoard = new HoPrimitiveBillboard();
				if(billBoard != NULL)
				{
					int ang = rand()%ANGLE_360;

					//POINT3D currentPos;
					POINT3D worldPos;
					POINT3D localPos;
					POINT3D destPos;
					POINT	size;
					size.x = 25;
					size.y = 100+rand()%200;
					
					float RandomYaw = RANDOM_NUM   * 3.14159f * 2.0f;
					float RandomPitch = RANDOM_NUM * 180.f * 3.14159f / 180.0f ;
					
					//구모양으로 파티클이 날아가게 벡터를 조정한다.
					destPos.y = int(cosf( RandomPitch)*((16.f)*(16.f))+y);
					destPos.x = int(sinf( RandomPitch)*(20.f) * cosf(RandomYaw)*(20.f)+x);
					destPos.z = int(sinf( RandomPitch)*(20.f) * sinf(RandomYaw)*(20.f)+z);
					
					worldPos.x = x;
					worldPos.y = y;
					worldPos.z = z;
					
					
					localPos.y = 0;
					localPos.x = 0;
					localPos.z = 0;
					
					billBoard->StartDestPath(worldPos, localPos, destPos, size, "SkillRoundImpactParticle.ini", 800);
					if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(billBoard, index))
					{
						delete billBoard;
						billBoard = NULL;
						return FALSE;
					}
					EffectTimeObjectBuffer[timeObjectIndex].Start();
				}
			}
			*/

			for(index = 0; index < 20+level*5; index++)
			{
				HoPrimitiveBillboard *billBoard = new HoPrimitiveBillboard();
				if(billBoard != NULL)
				{
					int ang = rand()%ANGLE_360;

					//POINT3D currentPos;
					POINT3D worldPos;
					POINT3D localPos;
					POINT3D destPos;
					POINT	size;

					size.x = 15;

					size.y = 1600+rand()%300;
					
					float RandomYaw = RANDOM_NUM   * 3.14159f * 2.0f;
					float RandomPitch = RANDOM_NUM * 180.f * 3.14159f / 180.0f ;
					
					//구모양으로 파티클이 날아가게 벡터를 조정한다.
					destPos.y = int(cosf( RandomPitch)*((100.f)*(100.f))+y);
					destPos.x = int(sinf( RandomPitch)*(120.f) * cosf(RandomYaw)*(120.f)+x);
					destPos.z = int(sinf( RandomPitch)*(120.f) * sinf(RandomYaw)*(120.f)+z);
					
					worldPos.x = x;
					worldPos.y = y;
					worldPos.z = z;

					localPos.y = 0;//rand()%5000-2500;
					localPos.x = 0;//rand()%5000-2500;
					localPos.z = 0;//rand()%5000-2500;
					
					billBoard->StartDestPath(worldPos, localPos, destPos, size, "SkillSpark1.ini", 500+rand()%400);
					if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(billBoard, index/2))
					{
						delete billBoard;
						billBoard = NULL;
						return FALSE;
					}
					EffectTimeObjectBuffer[timeObjectIndex].Start();
				}
			}


			break;
		}

		case EFFECT_PHYSICAL_ABSORB_DAMAGE:
		{
			SetDynLight( x, y, z, 255, 255, 255, 255, 50, 1);
			int timeObjectIndex = EmptyTimeObjectBufferIndex();
			if(timeObjectIndex == -1)
				return FALSE;
			
			int index; for( index = 0; index < 4; index++)
			{
				HoPrimitiveBillboard *primitiveBillBoard = new HoPrimitiveBillboard();
				primitiveBillBoard->StartBillRect(x, y, z, "SkillPhysicalAbsorb1.ini", ANI_ONE);
				if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(primitiveBillBoard, index*2))
				{
					delete primitiveBillBoard;
					primitiveBillBoard = NULL;
					return FALSE;
				}

				EffectTimeObjectBuffer[timeObjectIndex].Start();
			}
			break;
		}

		case EFFECT_MECHANICBOMB_DUST1:
		{
			int objIndex = EmptyZSortBufferIndex();
			if(objIndex == -1)
			{
				return FALSE;
			}
			
			HoPrimitiveBillboard *extPrimitive = new HoPrimitiveBillboard();
			extPrimitive->StartBillRect(x+rand()%50, y+rand()%50, z+rand()%50, "SkillMechanicBombDust1.ini", ANI_ONE);
			EffectZSortBuffer[objIndex] = extPrimitive;

			break;
		}
		
		case EFFECT_TEST1:
		{
			int timeObjectIndex = EmptyTimeObjectBufferIndex();
			if(timeObjectIndex == -1)
				return FALSE;
			
			int index; for( index = 0; index < 200; index++)
			{
				HoPrimitiveBillboard *primitiveBillBoard = new HoPrimitiveBillboard();
				POINT3D currentPos;
				currentPos.x = x;
				currentPos.y = y-5000;
				currentPos.z = z;
				primitiveBillBoard->StartPath(currentPos, "ReturnParticle1.ini", ANI_LOOP);
				
				int ang = rand()%ANGLE_360;
				HoPhysicsParticle *physicsParticle = new HoPhysicsParticle();
				POINT3D velocity;
				velocity.x = int(((float)GetCos[ang]/65536.f)*400.f);
				velocity.y = 200;
				velocity.z = int(((float)GetSin[ang]/65536.f)*400.f);

				physicsParticle->Start(velocity);
				physicsParticle->SetGravity(float(rand()%3+2));
				physicsParticle->SetLive(rand()%20+200);
				
				POINT3D angle;
				angle.x = 0;
				angle.y = ANGLE_360/3;
				angle.z = 0;
				physicsParticle->SetDestAngle(angle);
				primitiveBillBoard->SetPhysics(physicsParticle, PRIMITIVE_DEFAULT_RECT_PARTICLE | MOVE_ANGLE);
				
				POINT size;
				size.x = 16;
				size.y = 16;
				primitiveBillBoard->SetSize(size);
				if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(primitiveBillBoard, index*4))
				{
					delete primitiveBillBoard;
					primitiveBillBoard = NULL;
					return FALSE;
				}
				EffectTimeObjectBuffer[timeObjectIndex].Start();
			}
		
			/*
			for(index = 0; index < 25; index++)
			{
				HoPrimitiveBillboard *billBoard = new HoPrimitiveBillboard();
				if(billBoard != NULL)
				{
					int ang = ANGLE_360/30*index;

					//POINT3D currentPos;
					POINT3D worldPos;
					POINT3D localPos;
					POINT3D destPos;
					POINT	size;
					size.x = 30;
					size.y = 1200;
					

					destPos.x = int((GetCos[ang]/65536.f)*15000.f)+x;
					destPos.y = y+12000;
					destPos.z = int((GetSin[ang]/65536.f)*15000.f)+z;

					worldPos.x = x;
					worldPos.y = y;
					worldPos.z = z;

					localPos.x = int((GetCos[ang]/65536.f)*15000.f);
					localPos.y = 0;
					localPos.z = int((GetSin[ang]/65536.f)*15000.f);
					
					billBoard->StartDestPath(worldPos, localPos, destPos, size, "Return1.ini", 800,PRIMITIVE_PATH_RECT_STRETCH_LINE);
					if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(billBoard, index*4))
					{
						delete billBoard;
						billBoard = NULL;
						return FALSE;
					}
					EffectTimeObjectBuffer[timeObjectIndex].Start();
				}
			}

			for(index = 0; index < 20; index++)

			{
				HoPrimitiveBillboard *billBoard = new HoPrimitiveBillboard();
				if(billBoard != NULL)
				{
					int ang = ANGLE_360/20*index;


					//POINT3D currentPos;
					POINT3D worldPos;
					POINT3D localPos;
					POINT3D destPos;
					POINT	size;
					size.x = 30;
					size.y = 1200;

					destPos.x = int((GetCos[ang]/65536.f)*13000.f)+x;
					destPos.y = y+9000;
					destPos.z = int((GetSin[ang]/65536.f)*13000.f)+z;
					
					worldPos.x = x;
					worldPos.y = y+5000;
					worldPos.z = z;
					
					
					localPos.x = int((GetCos[ang]/65536.f)*13000.f);
					localPos.y = 0;
					localPos.z = int((GetSin[ang]/65536.f)*13000.f);
					
					billBoard->StartDestPath(worldPos, localPos, destPos, size, "Return1.ini", 800,PRIMITIVE_PATH_RECT_LINE);
					if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(billBoard, index*4))
					{
						delete billBoard;
						billBoard = NULL;
						return FALSE;
					}
					EffectTimeObjectBuffer[timeObjectIndex].Start();
				}
			}
			*/
			break;

		}

		case EFFECT_RETURN1:
		{
			y = y-9000;
			SetDynLight( x, y, z, 180, 180, 180, 255, 200 , 1);
			int timeObjectIndex = EmptyTimeObjectBufferIndex();
			if(timeObjectIndex == -1)
				return FALSE;
			
			int index; for( index = 0; index < 20; index++)
			{
				HoPrimitiveBillboard *primitiveBillBoard = new HoPrimitiveBillboard();
				POINT3D currentPos;
				currentPos.x = x;
				currentPos.y = y+10000;
				currentPos.z = z;
				primitiveBillBoard->StartPath(currentPos, "ReturnParticle1.ini", ANI_LOOP);
				
				int ang = rand()%ANGLE_360;
				HoPhysicsParticle *physicsParticle = new HoPhysicsParticle();
				POINT3D velocity;
				velocity.x = int(((float)GetCos[ang]/65536.f)*70.f);
				velocity.y = 200;
				velocity.z = int(((float)GetSin[ang]/65536.f)*70.f);

				physicsParticle->Start(velocity);
				physicsParticle->SetGravity(float(-rand()%3-6));
				physicsParticle->SetLive(rand()%20+60);
				
				POINT3D angle;
				angle.x = 0;
				angle.y = ANGLE_360/2;
				angle.z = 0;
				physicsParticle->SetDestAngle(angle);
				primitiveBillBoard->SetPhysics(physicsParticle, PRIMITIVE_DEFAULT_RECT_PARTICLE | MOVE_ANGLE);
				
				POINT size;
				size.x = 10;
				size.y = 10;
				primitiveBillBoard->SetSize(size);
				if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(primitiveBillBoard, index*2))
				{
					delete primitiveBillBoard;
					primitiveBillBoard = NULL;
					return FALSE;
				}
				EffectTimeObjectBuffer[timeObjectIndex].Start();
			}
			

			for(index = 0; index < 20; index++)
			{
				HoPrimitiveBillboard *primitiveBillBoard = new HoPrimitiveBillboard();
				POINT3D currentPos;
				currentPos.x = x;
				currentPos.y = y-5000;
				currentPos.z = z;
				primitiveBillBoard->StartPath(currentPos, "ReturnParticle1.ini", ANI_LOOP);
				
				int ang = rand()%ANGLE_360;
				HoPhysicsParticle *physicsParticle = new HoPhysicsParticle();
				POINT3D velocity;
				velocity.x = int(((float)GetCos[ang]/65536.f)*70.f);
				velocity.y = 200;
				velocity.z = int(((float)GetSin[ang]/65536.f)*70.f);

				physicsParticle->Start(velocity);
				physicsParticle->SetGravity(float(rand()%3+2));
				physicsParticle->SetLive(rand()%20+80);
				
				POINT3D angle;
				angle.x = 0;
				angle.y = ANGLE_360/3;
				angle.z = 0;
				physicsParticle->SetDestAngle(angle);
				primitiveBillBoard->SetPhysics(physicsParticle, PRIMITIVE_DEFAULT_RECT_PARTICLE | MOVE_ANGLE);
				
				POINT size;
				size.x = 16;
				size.y = 16;
				primitiveBillBoard->SetSize(size);
				if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(primitiveBillBoard, index*4+60))
				{
					delete primitiveBillBoard;
					primitiveBillBoard = NULL;
					return FALSE;
				}
				EffectTimeObjectBuffer[timeObjectIndex].Start();
			}
			
			HoEffectLight *light = new HoEffectLight();
			if(light != NULL)
			{
				light->Start(x, y+5000, z, 250, 250, 250, 255, 200, 1);
				if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(light, 100))
				{
					delete light;
					light = NULL;
					return FALSE;
				}
				EffectTimeObjectBuffer[timeObjectIndex].Start();
			}
			
			light = new HoEffectLight();
			if(light != NULL)
			{
				light->Start(x, y+5000, z, 250, 250, 250, 255, 170, 1);
				if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(light, 160))
				{
					delete light;
					light = NULL;
					return FALSE;
				}
				EffectTimeObjectBuffer[timeObjectIndex].Start();
			}
			for(index = 0; index < 25; index++)
			{
				HoPrimitiveBillboard *billBoard = new HoPrimitiveBillboard();
				if(billBoard != NULL)
				{
					int ang = ANGLE_360/30*index;

					//POINT3D currentPos;
					POINT3D worldPos;
					POINT3D localPos;
					POINT3D destPos;
					POINT	size;
					size.x = 30;
					size.y = 900;
					

					destPos.x = int((GetCos[ang]/65536.f)*3100.f)+x;
					destPos.y = y+12000;
					destPos.z = int((GetSin[ang]/65536.f)*3100.f)+z;

					worldPos.x = x;
					worldPos.y = y;
					worldPos.z = z;

					localPos.x = int((GetCos[ang]/65536.f)*3000.f);
					localPos.y = 0;
					localPos.z = int((GetSin[ang]/65536.f)*3000.f);
					
					billBoard->StartDestPath(worldPos, localPos, destPos, size, "Return1.ini", 800,PRIMITIVE_PATH_RECT_STRETCH_LINE);
					if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(billBoard, index*rand()%35+100))
					{
						delete billBoard;
						billBoard = NULL;
						return FALSE;
					}
					EffectTimeObjectBuffer[timeObjectIndex].Start();
				}
			}

			for(index = 0; index < 20; index++)

			{
				HoPrimitiveBillboard *billBoard = new HoPrimitiveBillboard();
				if(billBoard != NULL)
				{
					int ang = ANGLE_360/20*index;


					//POINT3D currentPos;
					POINT3D worldPos;
					POINT3D localPos;
					POINT3D destPos;
					POINT	size;
					size.x = 30;
					size.y = 800;

					destPos.x = int((GetCos[ang]/65536.f)*2000.f)+x;
					destPos.y = y+9000;
					destPos.z = int((GetSin[ang]/65536.f)*2000.f)+z;
					
					worldPos.x = x;
					worldPos.y = y+5000;
					worldPos.z = z;
					
					
					localPos.x = int((GetCos[ang]/65536.f)*2000.f);
					localPos.y = 0;
					localPos.z = int((GetSin[ang]/65536.f)*2000.f);
					
					billBoard->StartDestPath(worldPos, localPos, destPos, size, "Return1.ini", 800,PRIMITIVE_PATH_RECT_LINE);
					if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(billBoard, index*rand()%35+130))
					{
						delete billBoard;
						billBoard = NULL;
						return FALSE;
					}
					EffectTimeObjectBuffer[timeObjectIndex].Start();
				}
			}
			break;
		}
		
		//기본 타격..
		case EFFECT_NORMAL_HIT1:
		{
			/*
			int objIndex = EmptyZSortBufferIndex();
			if(objIndex == -1)
			{
				return FALSE;
			}
			
			HoPrimitiveBillboard *extPrimitive = new HoPrimitiveBillboard();
			extPrimitive->StartBillRect(x+rand()%50, y+rand()%50, z+rand()%50, "NormalHit1.ini", ANI_ONE);
			EffectZSortBuffer[objIndex] = extPrimitive;
			*/

			int timeObjectIndex = EmptyTimeObjectBufferIndex();
			if(timeObjectIndex == -1)
				return FALSE;
			
			HoPrimitiveBillboard *extPrimitive;
			extPrimitive = new HoPrimitiveBillboard();
			extPrimitive->StartBillRect(x, y, z, "NormalHit1.ini", ANI_ONE);
			if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(extPrimitive, 0))
			{
					delete extPrimitive;
					extPrimitive = NULL;
					return FALSE;
			}
			EffectTimeObjectBuffer[timeObjectIndex].Start();

			break;
		}
		
		case EFFECT_NORMAL_HIT2:
		{

			//StartBillRectPrimitive(x+rand()%50, y+rand()%50, z+rand()%50, 5, 5, "NormalHit2.ini");
			int timeObjectIndex = EmptyTimeObjectBufferIndex();
			if(timeObjectIndex == -1)
				return FALSE;
			
			HoPrimitiveBillboard *extPrimitive;
			extPrimitive = new HoPrimitiveBillboard();
			extPrimitive->StartBillRect(x, y, z, "NormalHit2.ini", ANI_ONE);
			if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(extPrimitive, 0))
			{
					delete extPrimitive;
					extPrimitive = NULL;
					return FALSE;
			}
			EffectTimeObjectBuffer[timeObjectIndex].Start();

			break;
		}

		case EFFECT_CRITICAL_HIT1:
		{
			/*
			StartBillRectPrimitive(x+rand()%50, y+rand()%50, z+rand()%50, 120, 120, "Light1.ini");
			objIndex = EmptyZSortBufferIndex();
			if(objIndex == -1)
			{
				return FALSE;
			}
			
			HoPrimitiveBillboard *extPrimitive = new HoPrimitiveBillboard();
			extPrimitive->StartBillRect(x+rand()%50, y+rand()%50, z+rand()%50, "CriticalHit1.ini", ANI_ONE);
			EffectObjectBuffer[objIndex] = extPrimitive;
			*/
			
			int timeObjectIndex = EmptyTimeObjectBufferIndex();
			if(timeObjectIndex == -1)
				return FALSE;
			
			HoPrimitiveBillboard *extPrimitive;
			extPrimitive = new HoPrimitiveBillboard();
			extPrimitive->StartBillRect(x, y, z,120, 120, "Light1.ini", ANI_ONE);
			if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(extPrimitive, 0))
			{
					delete extPrimitive;
					extPrimitive = NULL;
					return FALSE;
			}
			EffectTimeObjectBuffer[timeObjectIndex].Start();
			
			
			extPrimitive = new HoPrimitiveBillboard();
			extPrimitive->StartBillRect(x, y, z, "CriticalHit1.ini", ANI_ONE);
			if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(extPrimitive, 0))
			{
					delete extPrimitive;
					extPrimitive = NULL;
					return FALSE;
			}
			EffectTimeObjectBuffer[timeObjectIndex].Start();
			

			break;
		}

		case EFFECT_CRITICAL_HIT2:
		{
			//StartBillRectPrimitive(x+rand()%50, y+rand()%50, z+rand()%50, 120, 120, "Light1.ini");
			SetDynLight( x, y, z, 200, 0, 0, 255, 200, 3);
			int timeObjectIndex = EmptyTimeObjectBufferIndex();
			if(timeObjectIndex == -1)
				return FALSE;
			
			int index = 0;
			HoPrimitiveBillboard *extPrimitive;
			
			for(index = 0; index < 3; index++)
			{
				extPrimitive = new HoPrimitiveBillboard();
				extPrimitive->StartBillRect(x, y, z, "CriticalHit21.ini", ANI_ONE);
				if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(extPrimitive, index*4))
				{
						delete extPrimitive;
						extPrimitive = NULL;
						return FALSE;
				}
			}
			EffectTimeObjectBuffer[timeObjectIndex].Start();
			break;
		}

		//Mechanic-Bomb용..
		case EFFECT_FIRE_HIT1:
		{
			SetDynLight( x, y, z, 255, 50, 50, 255, 250, 1);
			
			int timeObjectIndex = EmptyTimeObjectBufferIndex();
			if(timeObjectIndex == -1)
				return FALSE;
			
			int index = 0;
			int count = 0;
			
			//폭탄 파편(파티클)
			for(index = 0; index < 8+level*3; index++)
			{
				HoPrimitiveBillboard *primitiveBillBoard = new HoPrimitiveBillboard();
				if(primitiveBillBoard != NULL)
				{
					POINT3D currentPos;
					currentPos.x = x;
					currentPos.y = y;
					currentPos.z = z;
					primitiveBillBoard->StartPath(currentPos, "ParticleMeteo1.ini", ANI_LOOP);
					
					int ang = rand()%ANGLE_360;
					HoPhysicsParticle *physicsParticle = new HoPhysicsParticle();
					POINT3D velocity;
					velocity.x = int((GetCos[ang]/65536.f)*(150.f+rand()%150));
					velocity.y = 150+rand()%250;
					velocity.z = int((GetSin[ang]/65536.f)*(150.f+rand()%150));

					physicsParticle->Start(velocity);
					physicsParticle->SetGravity(float(-rand()%5-6));
					physicsParticle->SetLive(rand()%20+55);
					
					primitiveBillBoard->SetPhysics(physicsParticle, PRIMITIVE_PATH_RECT_LINE);

					
					POINT size;
					size.x = 8;
					size.y = 70;
					primitiveBillBoard->SetSize(size);
					if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(primitiveBillBoard, index/4))
					{
						delete primitiveBillBoard;
						primitiveBillBoard = NULL;
						return FALSE;
					}
					EffectTimeObjectBuffer[timeObjectIndex].Start();
				}
			}
			
			for(index = 0; index < 8+level*4; index++)
			{
				HoPrimitiveBillboard *billBoard = new HoPrimitiveBillboard();
				if(billBoard != NULL)
				{
					int ang = rand()%ANGLE_360;

					//POINT3D currentPos;
					POINT3D worldPos;
					POINT3D localPos;
					POINT3D destPos;
					POINT	size;
					size.x = 25;
					size.y = 100+rand()%300;
					
					float RandomYaw = RANDOM_NUM   * 3.14159f * 2.0f;
					float RandomPitch = RANDOM_NUM * 180.f * 3.14159f / 180.0f ;
					
					//구모양으로 파티클이 날아가게 벡터를 조정한다.
					destPos.y = int(cosf( RandomPitch)*((90.f+level*2)*(90.f+level*2))+y);
					destPos.x = int(sinf( RandomPitch)*(120.f+level*4) * cosf(RandomYaw)*(120.f+level*4)+x);
					destPos.z = int(sinf( RandomPitch)*(120.f+level*4) * sinf(RandomYaw)*(120.f+level*4)+z);
					
					worldPos.x = x;
					worldPos.y = y;
					worldPos.z = z;
					
					
					localPos.y = int(cosf( RandomPitch)*(65.f*65.f));
					localPos.x = int(sinf( RandomPitch)*85.f * cosf(RandomYaw)*85.f);
					localPos.z = int(sinf( RandomPitch)*85.f * sinf(RandomYaw)*85.f);
					
					billBoard->StartDestPath(worldPos, localPos, destPos, size, "ParticleMeteo2.ini", 600);
					if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(billBoard, index*rand()%15))
					{
						delete billBoard;
						billBoard = NULL;
						return FALSE;
					}
					EffectTimeObjectBuffer[timeObjectIndex].Start();
				}
			}
			
			
			
			HoPrimitiveBillboard *extPrimitive = new HoPrimitiveBillboard();
			extPrimitive->StartBillRect(x, y, z, "MechanicBomb1.ini", ANI_ONE);
			
			if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(extPrimitive, 0))
			{
					delete extPrimitive;
					extPrimitive = NULL;
					return FALSE;
			}
			EffectTimeObjectBuffer[timeObjectIndex].Start();

			
			extPrimitive = new HoPrimitiveBillboard();
			extPrimitive->StartBillRect(x, y, z, "MechanicBomb2.ini", ANI_ONE);
			
			if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(extPrimitive, 0))
			{
					delete extPrimitive;
					extPrimitive = NULL;
					return FALSE;
			}
			EffectTimeObjectBuffer[timeObjectIndex].Start();
			break;
		}

		//Meteo용..
		case EFFECT_FIRE_HIT2:
		{
			SetDynLight( x, y, z, 255, 50, 50, 255, 200 , 1);
			objIndex = EmptyZSortBufferIndex();
			if(objIndex == -1)
				return FALSE;
			
			int timeObjectIndex = EmptyTimeObjectBufferIndex();
			if(timeObjectIndex == -1)
				return FALSE;

			//폭탄 파편(파티클)
			int index; for( index = 0; index < 20; index++)
			{
				HoPrimitiveBillboard *primitiveBillBoard = new HoPrimitiveBillboard();
				if(primitiveBillBoard != NULL)
				{
					POINT3D currentPos;
					currentPos.x = x;
					currentPos.y = y;
					currentPos.z = z;
					primitiveBillBoard->StartPath(currentPos, "ParticleMeteo1.ini", ANI_LOOP);
					
					int ang = rand()%ANGLE_360;
					HoPhysicsParticle *physicsParticle = new HoPhysicsParticle();
					POINT3D velocity;
					velocity.x = int((GetCos[ang]/65536.f)*(150.f+rand()%150));
					velocity.y = 240+rand()%250;
					velocity.z = int((GetSin[ang]/65536.f)*(150.f+rand()%150));

					physicsParticle->Start(velocity);
					physicsParticle->SetGravity(float(-rand()%5-6));
					physicsParticle->SetLive(rand()%20+55);
					
					primitiveBillBoard->SetPhysics(physicsParticle, PRIMITIVE_PATH_RECT_LINE);

					
					POINT size;
					size.x = 10;
					size.y = 70;
					primitiveBillBoard->SetSize(size);
					if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(primitiveBillBoard, index/4))
					{
						delete primitiveBillBoard;
						primitiveBillBoard = NULL;
						return FALSE;
					}
					EffectTimeObjectBuffer[timeObjectIndex].Start();
				}
			}

			

			for(index = 0; index < 20; index++)
			{
				HoPrimitiveBillboard *billBoard = new HoPrimitiveBillboard();
				if(billBoard != NULL)
				{
					int ang = rand()%ANGLE_360;

					//POINT3D currentPos;
					POINT3D worldPos;
					POINT3D localPos;
					POINT3D destPos;
					POINT	size;
					size.x = 25;
					size.y = 100+rand()%300;
					
					float RandomYaw = RANDOM_NUM   * 3.14159f * 2.0f;
					float RandomPitch = RANDOM_NUM * 180.f * 3.14159f / 180.0f ;
					
					//구모양으로 파티클이 날아가게 벡터를 조정한다.
					destPos.y = rand()%100+10000+y;
					destPos.x = int(sinf( RandomPitch)*180.f * cosf(RandomYaw)*180.f+x);
					destPos.z = int(sinf( RandomPitch)*180.f * sinf(RandomYaw)*180.f+z);
					
					worldPos.x = x;
					worldPos.y = y;
					worldPos.z = z;

					
					localPos.y = 0;
					localPos.x = int(sinf( RandomPitch)*80.f * cosf(RandomYaw)*80.f);
					localPos.z = int(sinf( RandomPitch)*80.f * sinf(RandomYaw)*80.f);
					
					billBoard->StartDestPath(worldPos, localPos, destPos, size, "ParticleMeteo2.ini", 700);
					if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(billBoard, index*rand()%20))
					{
						delete billBoard;
						billBoard = NULL;
						return FALSE;
					}
					EffectTimeObjectBuffer[timeObjectIndex].Start();
				}
			}
			
			y = y+4000;
			for(index = 0; index < 3; index++)
			{
				HoPrimitiveBillboard *extPrimitive = new HoPrimitiveBillboard();
				extPrimitive->StartBillRect(x+rand()%2800*index, y, z+rand()%2800*index, "Meteo1.ini", ANI_ONE);
				
				if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(extPrimitive, index*rand()%7))
				{
						delete extPrimitive;
						extPrimitive = NULL;
						return FALSE;
				}
			}

			break;
		}

		case EFFECT_BANDI1:
		{
			int index; for( index = 0; index < 20; index++)
			{
				objIndex = EmptyZSortBufferIndex();
				if(objIndex == -1)
					break;

				HoEffectCircleLine *circleLine = new HoEffectCircleLine();
				if(circleLine != NULL)
				{
					circleLine->Start(x+(rand()%10000-5000)*22, y+(rand()%10000)*3, z+(rand()%10000-5000)*22,3, 3, "Bandi1.ini", ANI_LOOP);
					EffectZSortBuffer[objIndex] = circleLine;
				}
			}
			

			break;
		}
		
		case EFFECT_LEVELUP1:
		{
			int timeObjectIndex = EmptyTimeObjectBufferIndex();
			if(timeObjectIndex == -1)
				return FALSE;
			
			SetDynLight( x, y, z, 150, 150, 150, 255, 200 , 1);

			y = y-1000;
			int index; for( index = 0; index < 20; index++)
			{
				objIndex = EmptyObjectBufferIndex();
				if(objIndex == -1)
					break;

				//HoLevelUpParticle *particle = new HoLevelUpParticle();
				HoPrimitivePolygon *particle = new HoPrimitivePolygon();
				if(particle != NULL)
				{
					int ang = ANGLE_360/20*index;


					POINT3D destPos;
					destPos.x = x;
					destPos.y = y;
					destPos.z = z;
					
					POINT3D currentPos;
					currentPos.x = int(x+((float)GetCos[ang]/65536.f)*(21000+rand()%10*1000));
					currentPos.y = int(y+GetCos[ang%ANGLE_90]/65536.f*10000);//y+rand()%10000;
					currentPos.z = int(z+((float)GetSin[ang]/65536.f)*(21000+rand()%10*1000));			
					
					particle->StartPathTri(currentPos, destPos, "LevelUpParticle1.ini");
					//particle->SetSize(8,22);
					//particle->SetGravity(-rand()%4-3);
					//particle->SetLive(rand()%100);
					if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(particle, 0))
					{
						delete particle;
						particle = NULL;
						return FALSE;
					}
					EffectTimeObjectBuffer[timeObjectIndex].Start();
				}
			}

			for(index = 0; index < 5; index++)
			{
				HoPrimitiveBillboard *extPrimitive = new HoPrimitiveBillboard();
				extPrimitive->StartBillRect(x+rand()%50, y+rand()%50, z+rand()%50, "LevelUp.ini", ANI_ONE);
				if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(extPrimitive, index*5+25))
				{
					delete extPrimitive;
					extPrimitive = NULL;
					return FALSE;
				}
				EffectTimeObjectBuffer[timeObjectIndex].Start();
			}
			
			y = y+4000;
			for(index = 0; index < 5; index++)
			{
				HoEtcPrimitiveBillboardMove *primitiveMove = new HoEtcPrimitiveBillboardMove();
				if(primitiveMove != NULL)
				{
					primitiveMove->Start(x, y, z, 40, 10, "LevelUp1Left.ini", ANI_ONE);
					
					//RightMove
					primitiveMove->MoveState = 1;
					primitiveMove->TranslateMoveX = -230;
					
					if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(primitiveMove, index*5))
					{
						delete primitiveMove;
						primitiveMove = NULL;
						return FALSE;
					}
					EffectTimeObjectBuffer[timeObjectIndex].Start();
				}
				
				objIndex = EmptyObjectBufferIndex();
				if(objIndex == -1)
					return FALSE;
				primitiveMove = NULL;
				primitiveMove = new HoEtcPrimitiveBillboardMove();
				
				if(primitiveMove != NULL)
				{
					//LeftMove
					primitiveMove->Start(x, y, z, 40, 10, "LevelUp1Right.ini", ANI_ONE);
					primitiveMove->MoveState = 3;
					primitiveMove->TranslateMoveX = 230;
					if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(primitiveMove, index*5))
					{
						delete primitiveMove;
						primitiveMove = NULL;
						return FALSE;
					}
					EffectTimeObjectBuffer[timeObjectIndex].Start();
				}
			}

			break;
		}
		
		case EFFECT_GATE1:
		{
			int timeObjectIndex = EmptyTimeObjectBufferIndex();
			if(timeObjectIndex == -1)
				return FALSE;
			HoPrimitiveBillboard *extPrimitive;
			int index; for( index = 0; index < 2; index++)
			{
				extPrimitive = new HoPrimitiveBillboard();
				if(extPrimitive != NULL)
				{
					extPrimitive->StartBillRect(x, y, z, "ScoutHawkLight.ini", ANI_ONE);
					if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(extPrimitive, index))
					{
						delete extPrimitive;
						extPrimitive = NULL;
						return FALSE;
					}
					EffectTimeObjectBuffer[timeObjectIndex].Start();
				}
			}

			for(index = 0; index < 1; index++)
			{
				extPrimitive = new HoPrimitiveBillboard();
				if(extPrimitive != NULL)
				{
					extPrimitive->StartBillRect(x, y, z, "ScoutHawkGate.ini", ANI_ONE);
					if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(extPrimitive, 5+index))
					{
						delete extPrimitive;
						extPrimitive = NULL;
						return FALSE;
					}
					EffectTimeObjectBuffer[timeObjectIndex].Start();
				}
			}
			break;
		}
		
		case EFFECT_GAME_START1:
		{
			objIndex = EmptyZSortBufferIndex();
			if(objIndex == -1)
				return FALSE;
			
			HoEffectPat *pat = new HoEffectPat();
			if(pat != NULL)
			{
				pat->StartAni(x, y-1000, z,0, 0, 0, PatObj[0]);
				pat->AnimationEnd = PatAnimationEnd[1];
				EffectZSortBuffer[objIndex] = pat;
			}
			break;
		}
		case EFFECT_LIGHT1:
		{
			StartBillRectPrimitive(x, y, z, 5, 5, "Light1.ini");
			break;
		}
		
		
		case EFFECT_SHIELD1_PARTICLE:
		{
			int timeObjectIndex = EmptyTimeObjectBufferIndex();
			if(timeObjectIndex == -1)
				return FALSE;

			HoPrimitiveBillboard *primitiveBillBoard = new HoPrimitiveBillboard();
			POINT3D currentPos;
			currentPos.x = x;
			currentPos.y = y-5000;
			currentPos.z = z;
			primitiveBillBoard->StartPath(currentPos, "PhysicalAbsorbParticle2.ini", ANI_LOOP);
			
			int ang = rand()%ANGLE_360;
			HoPhysicsParticle *physicsParticle = new HoPhysicsParticle();
			POINT3D velocity;
			velocity.x = int(((float)GetCos[ang]/65536.f)*70.f);
			velocity.y = 200;
			velocity.z = int(((float)GetSin[ang]/65536.f)*70.f);

			physicsParticle->Start(velocity);
			physicsParticle->SetGravity(float(rand()%3+2));
			physicsParticle->SetLive(rand()%20+20);
			
			POINT3D angle;
			angle.x = 0;
			angle.y = ANGLE_360/3;
			angle.z = 0;
			physicsParticle->SetDestAngle(angle);
			primitiveBillBoard->SetPhysics(physicsParticle, PRIMITIVE_DEFAULT_RECT_PARTICLE | MOVE_ANGLE);
			
			POINT size;
			size.x = 8;
			size.y = 8;
			primitiveBillBoard->SetSize(size);
			if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(primitiveBillBoard, 0))
			{
				delete primitiveBillBoard;
				primitiveBillBoard = NULL;
				return FALSE;
			}
			EffectTimeObjectBuffer[timeObjectIndex].Start();

			break;
		}

		case EFFECT_POTION1:
		{
			StartBillRectPrimitive(x+rand()%50, y+rand()%50, z+rand()%50, 120, 120, "Light1.ini");
			for(int index = 0; index < 30; index++)
			{
				HoPrimitiveBillboard *primitiveBillBoard = new HoPrimitiveBillboard();
				POINT3D currentPos;
				currentPos.x = x;
				currentPos.y = y;
				currentPos.z = z;
				primitiveBillBoard->StartPath(currentPos, "Potion1.ini", ANI_LOOP);
				
				int ang = rand()%ANGLE_360;
				HoPhysicsParticle *physicsParticle = new HoPhysicsParticle();
				POINT3D velocity;
				velocity.x = int(((float)GetCos[ang]/65536.f)*70.f);
				velocity.y = 70;
				velocity.z = int(((float)GetSin[ang]/65536.f)*70.f);

				physicsParticle->Start(velocity);
				physicsParticle->SetGravity(float(-rand()%3-3));
				physicsParticle->SetLive(rand()%20+55);
				
				POINT3D angle;
				angle.x = 0;
				angle.y = ANGLE_360/3+500;
				angle.z = 0;
				physicsParticle->SetDestAngle(angle);
				primitiveBillBoard->SetPhysics(physicsParticle, PRIMITIVE_DEFAULT_RECT_PARTICLE | MOVE_ANGLE);
				
				POINT size;
				size.x = 8;
				size.y = 8;
				primitiveBillBoard->SetSize(size);
				objIndex = EmptyObjectBufferIndex();
				if(objIndex == -1)
					break;

				EffectObjectBuffer[objIndex] = primitiveBillBoard;
			}
			
			break;
		}
		case EFFECT_POTION2:
		{
			StartBillRectPrimitive(x+rand()%50, y+rand()%50, z+rand()%50, 120, 120, "Light1.ini");
			for(int index = 0; index < 30; index++)
			{
				HoPrimitiveBillboard *primitiveBillBoard = new HoPrimitiveBillboard();
				POINT3D currentPos;
				currentPos.x = x;
				currentPos.y = y;
				currentPos.z = z;
				primitiveBillBoard->StartPath(currentPos, "Potion2.ini", ANI_LOOP);
				
				int ang = rand()%ANGLE_360;
				HoPhysicsParticle *physicsParticle = new HoPhysicsParticle();
				POINT3D velocity;
				velocity.x = int(((float)GetCos[ang]/65536.f)*70.f);
				velocity.y = 70;
				velocity.z = int(((float)GetSin[ang]/65536.f)*70.f);

				physicsParticle->Start(velocity);
				physicsParticle->SetGravity(float(-rand()%3-3));
				physicsParticle->SetLive(rand()%20+55);
				
				POINT3D angle;
				angle.x = 0;
				angle.y = ANGLE_360/3+500;
				angle.z = 0;
				physicsParticle->SetDestAngle(angle);
				primitiveBillBoard->SetPhysics(physicsParticle, PRIMITIVE_DEFAULT_RECT_PARTICLE | MOVE_ANGLE);
				
				POINT size;
				size.x = 8;
				size.y = 8;
				primitiveBillBoard->SetSize(size);
				objIndex = EmptyObjectBufferIndex();
				if(objIndex == -1)
					break;

				EffectObjectBuffer[objIndex] = primitiveBillBoard;
			}

			break;
		}

		case EFFECT_POTION3:
		{
			StartBillRectPrimitive(x+rand()%50, y+rand()%50, z+rand()%50, 120, 120, "Light1.ini");
			for(int index = 0; index < 30; index++)
			{
				HoPrimitiveBillboard *primitiveBillBoard = new HoPrimitiveBillboard();
				POINT3D currentPos;
				currentPos.x = x;
				currentPos.y = y;
				currentPos.z = z;
				primitiveBillBoard->StartPath(currentPos, "Potion3.ini", ANI_LOOP);
				
				int ang = rand()%ANGLE_360;
				HoPhysicsParticle *physicsParticle = new HoPhysicsParticle();
				POINT3D velocity;
				velocity.x = int(((float)GetCos[ang]/65536.f)*70.f);
				velocity.y = 70;
				velocity.z = int(((float)GetSin[ang]/65536.f)*70.f);

				physicsParticle->Start(velocity);
				physicsParticle->SetGravity(float(-rand()%3-3));
				physicsParticle->SetLive(rand()%20+55);
				
				POINT3D angle;
				angle.x = 0;
				angle.y = ANGLE_360/3+500;
				angle.z = 0;
				physicsParticle->SetDestAngle(angle);
				primitiveBillBoard->SetPhysics(physicsParticle, PRIMITIVE_DEFAULT_RECT_PARTICLE | MOVE_ANGLE);
				
				POINT size;
				size.x = 8;
				size.y = 8;
				primitiveBillBoard->SetSize(size);
				objIndex = EmptyObjectBufferIndex();
				if(objIndex == -1)
					break;

				EffectObjectBuffer[objIndex] = primitiveBillBoard;
			}
			break;
		}
		
		
		case EFFECT_DAMAGE1:
		{
			
			int objIndex = EmptyZSortBufferIndex();
			if(objIndex == -1)
			{
				return FALSE;
			}
			
			HoPrimitiveBillboard *extPrimitive = new HoPrimitiveBillboard();
			extPrimitive->StartBillRect(x, y, z, "Blood1.ini", ANI_ONE);
			EffectZSortBuffer[objIndex] = extPrimitive;
			
			/*
			int timeObjectIndex = EmptyTimeObjectBufferIndex();
			if(timeObjectIndex == -1)
				return FALSE;

			for(int index = 0; index < 8; index++)
			{
				HoPrimitiveBillboard *primitiveBillBoard = new HoPrimitiveBillboard();
				if(primitiveBillBoard != NULL)
				{
					POINT3D currentPos;
					currentPos.x = x;
					currentPos.y = y;
					currentPos.z = z;
					primitiveBillBoard->StartPath(currentPos, "DamageParticle1.ini", ANI_LOOP);
					
					int ang = rand()%ANGLE_360;
					HoPhysicsParticle *physicsParticle = new HoPhysicsParticle();
					POINT3D velocity;
					velocity.x = int((GetCos[ang]/65536.f)*(50.f+rand()%30));
					velocity.y = 90+rand()%100;
					velocity.z = int((GetSin[ang]/65536.f)*(50.f+rand()%30));

					physicsParticle->Start(velocity);
					physicsParticle->SetGravity(float(-rand()%3-1));
					physicsParticle->SetLive(rand()%20+55);
					
					primitiveBillBoard->SetPhysics(physicsParticle, PRIMITIVE_PATH_RECT_LINE);

					
					POINT size;
					size.x = 20;
					size.y = 80;
					primitiveBillBoard->SetSize(size);
					if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(primitiveBillBoard, index*3))
					{
						delete primitiveBillBoard;
						primitiveBillBoard = NULL;
						return FALSE;
					}
					EffectTimeObjectBuffer[timeObjectIndex].Start();
				}
			}
			*/
			break;
		}
		

		case EFFECT_GAS1:
		{
			StartBillRectPrimitive(x, y, z, 120, 120, "Gas1.ini");
			break;
		}

		case EFFECT_ICE_HIT1:
		{
			SetDynLight( x, y, z, 50, 200, 200, 255, 100, 1);
			
			int timeObjectIndex = EmptyTimeObjectBufferIndex();
			if(timeObjectIndex == -1)
				return FALSE;
			
			int index = 0;
			int count = 0;
			
			//얼음 파편(파티클)
			for(index = 0; index < 8+level*3; index++)
			{
				HoPrimitiveBillboard *primitiveBillBoard = new HoPrimitiveBillboard();
				if(primitiveBillBoard != NULL)
				{
					POINT3D currentPos;
					currentPos.x = x;
					currentPos.y = y;
					currentPos.z = z;
					primitiveBillBoard->StartPath(currentPos, "ParticleMeteo1_blue.ini", ANI_LOOP);
					
					int ang = rand()%ANGLE_360;
					HoPhysicsParticle *physicsParticle = new HoPhysicsParticle();
					POINT3D velocity;
					velocity.x = int((GetCos[ang]/65536.f)*(150.f+rand()%150));
					velocity.y = 150+rand()%250;
					velocity.z = int((GetSin[ang]/65536.f)*(150.f+rand()%150));

					physicsParticle->Start(velocity);
					physicsParticle->SetGravity(float(-rand()%5-6));
					physicsParticle->SetLive(rand()%20+55);
					
					primitiveBillBoard->SetPhysics(physicsParticle, PRIMITIVE_PATH_RECT_LINE);

					
					POINT size;
					size.x = 8;
					size.y = 50;
					primitiveBillBoard->SetSize(size);
					if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(primitiveBillBoard, index/4))
					{
						delete primitiveBillBoard;
						primitiveBillBoard = NULL;
						return FALSE;
					}
					EffectTimeObjectBuffer[timeObjectIndex].Start();
				}
			}
			
			for(index = 0; index < 8+level*4; index++)
			{
				HoPrimitiveBillboard *billBoard = new HoPrimitiveBillboard();
				if(billBoard != NULL)
				{
					int ang = rand()%ANGLE_360;

					//POINT3D currentPos;
					POINT3D worldPos;
					POINT3D localPos;
					POINT3D destPos;
					POINT	size;
					size.x = 25;
					size.y = 100+rand()%300;
					
					float RandomYaw = RANDOM_NUM   * 3.14159f * 2.0f;
					float RandomPitch = RANDOM_NUM * 180.f * 3.14159f / 180.0f ;
					
					//구모양으로 파티클이 날아가게 벡터를 조정한다.
					destPos.y = int(cosf( RandomPitch)*((90.f+level*2)*(90.f+level*2))+y);
					destPos.x = int(sinf( RandomPitch)*(120.f+level*4) * cosf(RandomYaw)*(120.f+level*4)+x);
					destPos.z = int(sinf( RandomPitch)*(120.f+level*4) * sinf(RandomYaw)*(120.f+level*4)+z);
					
					worldPos.x = x;
					worldPos.y = y;
					worldPos.z = z;
					
					
					localPos.y = int(cosf( RandomPitch)*(65.f*65.f));
					localPos.x = int(sinf( RandomPitch)*85.f * cosf(RandomYaw)*85.f);
					localPos.z = int(sinf( RandomPitch)*85.f * sinf(RandomYaw)*85.f);
					
					billBoard->StartDestPath(worldPos, localPos, destPos, size, "ParticleMeteo1_blue.ini", 600);
					if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(billBoard, index*rand()%15))
					{
						delete billBoard;
						billBoard = NULL;
						return FALSE;
					}
					EffectTimeObjectBuffer[timeObjectIndex].Start();
				}
			}
			
			
			
			HoPrimitiveBillboard *extPrimitive = new HoPrimitiveBillboard();
			extPrimitive->StartBillRect(x, y, z, "ParticleMeteo1_blue.ini", ANI_ONE);
			
			if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(extPrimitive, 0))
			{
					delete extPrimitive;
					extPrimitive = NULL;
					return FALSE;
			}
			EffectTimeObjectBuffer[timeObjectIndex].Start();

			
			extPrimitive = new HoPrimitiveBillboard();
			extPrimitive->StartBillRect(x, y, z, "ParticleMeteo1_blue.ini", ANI_ONE);
			
			if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(extPrimitive, 0))
			{
					delete extPrimitive;
					extPrimitive = NULL;
					return FALSE;
			}
			EffectTimeObjectBuffer[timeObjectIndex].Start();
			break;
		}

		case EFFECT_ICE_HIT2:
		{
			SetDynLight( x, y, z, 50, 200, 200, 255, 170, 1);
			
			int timeObjectIndex = EmptyTimeObjectBufferIndex();
			if(timeObjectIndex == -1)
				return FALSE;
			
			int index = 0;
			int count = 0;
			
			//얼음 파편(파티클)
			for(index = 0; index < 8+level*3; index++)
			{
				HoPrimitiveBillboard *primitiveBillBoard = new HoPrimitiveBillboard();
				if(primitiveBillBoard != NULL)
				{
					POINT3D currentPos;
					currentPos.x = x;
					currentPos.y = y;
					currentPos.z = z;
					primitiveBillBoard->StartPath(currentPos, "ParticleMeteo1_blue.ini", ANI_LOOP);
					
					int ang = rand()%ANGLE_360;
					HoPhysicsParticle *physicsParticle = new HoPhysicsParticle();
					POINT3D velocity;
					velocity.x = int((GetCos[ang]/65536.f)*(150.f+rand()%150));
					velocity.y = 150+rand()%250;
					velocity.z = int((GetSin[ang]/65536.f)*(150.f+rand()%150));

					physicsParticle->Start(velocity);
					physicsParticle->SetGravity(float(-rand()%5-6));
					physicsParticle->SetLive(rand()%20+55);
					
					primitiveBillBoard->SetPhysics(physicsParticle, PRIMITIVE_PATH_RECT_LINE);

					
					POINT size;
					size.x = 8;
					size.y = 90;
					primitiveBillBoard->SetSize(size);
					if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(primitiveBillBoard, index/4))
					{
						delete primitiveBillBoard;
						primitiveBillBoard = NULL;
						return FALSE;
					}
					EffectTimeObjectBuffer[timeObjectIndex].Start();
				}
			}
			
			for(index = 0; index < 8+level*4; index++)
			{
				HoPrimitiveBillboard *billBoard = new HoPrimitiveBillboard();
				if(billBoard != NULL)
				{
					int ang = rand()%ANGLE_360;

					//POINT3D currentPos;
					POINT3D worldPos;
					POINT3D localPos;
					POINT3D destPos;
					POINT	size;
					size.x = 25;
					size.y = 100+rand()%300;
					
					float RandomYaw = RANDOM_NUM   * 3.14159f * 2.0f;
					float RandomPitch = RANDOM_NUM * 180.f * 3.14159f / 180.0f ;
					
					//구모양으로 파티클이 날아가게 벡터를 조정한다.
					destPos.y = int(cosf( RandomPitch)*((90.f+level*2)*(90.f+level*2))+y);
					destPos.x = int(sinf( RandomPitch)*(120.f+level*4) * cosf(RandomYaw)*(120.f+level*4)+x);
					destPos.z = int(sinf( RandomPitch)*(120.f+level*4) * sinf(RandomYaw)*(120.f+level*4)+z);
					
					worldPos.x = x;
					worldPos.y = y;
					worldPos.z = z;
					
					
					localPos.y = int(cosf( RandomPitch)*(65.f*65.f));
					localPos.x = int(sinf( RandomPitch)*85.f * cosf(RandomYaw)*85.f);
					localPos.z = int(sinf( RandomPitch)*85.f * sinf(RandomYaw)*85.f);
					
					billBoard->StartDestPath(worldPos, localPos, destPos, size, "ParticleMeteo1_blue.ini", 600);
					if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(billBoard, index*rand()%15))
					{
						delete billBoard;
						billBoard = NULL;
						return FALSE;
					}
					EffectTimeObjectBuffer[timeObjectIndex].Start();
				}
			}
			
			
			
			HoPrimitiveBillboard *extPrimitive = new HoPrimitiveBillboard();
			extPrimitive->StartBillRect(x, y, z, "ParticleMeteo1_blue.ini", ANI_ONE);
			
			if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(extPrimitive, 0))
			{
					delete extPrimitive;
					extPrimitive = NULL;
					return FALSE;
			}
			EffectTimeObjectBuffer[timeObjectIndex].Start();

			
			extPrimitive = new HoPrimitiveBillboard();
			extPrimitive->StartBillRect(x, y, z, "ParticleMeteo1_blue.ini", ANI_ONE);
			
			if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(extPrimitive, 0))
			{
					delete extPrimitive;
					extPrimitive = NULL;
					return FALSE;
			}
			EffectTimeObjectBuffer[timeObjectIndex].Start();
			break;
		}

	}
	return TRUE;
}

int HoEffectMgr::Start(int x, int y, int z, int sizeX, int sizeY, int effectType)
{
	switch(effectType)
	{
		case EFFECT_DUST1:
		{
			StartBillRectPrimitive(x, y, z, sizeX, sizeY, "Dust1.ini");
			break;
		}
	}
	return TRUE;
}


int HoEffectMgr::Start(int x, int y, int z, int r, int g, int b, int a, int effectType)
{
	switch(effectType)
	{
		case EFFECT_PARTICLE_BOW1:
		{
			int objIndex = EmptyObjectBufferIndex();
			if(objIndex == -1)
				return FALSE;
			HoParticleBow *particleBow = new HoParticleBow;
			
			if(particleBow != NULL)
			{
				particleBow->Start(x, y, z, r, g, b, a, MaterialNum[4]);
				EffectObjectBuffer[objIndex] = particleBow;
			}
			break;
		}

		case EFFECT_PARTICLE_BOW2:
		{
			int objIndex = EmptyObjectBufferIndex();
			if(objIndex == -1)
				return FALSE;
			HoParticleBow *particleBow = new HoParticleBow;
			if(particleBow != NULL)
			{
				particleBow->StartFire(x, y, z, r, g, b, a, MaterialNum[0]);
				EffectObjectBuffer[objIndex] = particleBow;
			}
			break;
		}
	}

	return TRUE;
}

int	HoEffectMgr::Start(int x, int y, int z, int effectType, smCHAR *character)
{
	switch(effectType)
	{
		case EFFECT_STUN1:
		{
			int timeObjectIndex = EmptyTimeObjectBufferIndex();
			if(timeObjectIndex == -1)
				return FALSE;
			
			for(int index = 0; index < 2; index++)
			{
				HoEffectStun	*effectStun = new HoEffectStun();
				if(effectStun != NULL)
				{
					effectStun->Start(x, y, z, character);
					if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(effectStun, index*80))
					{
						delete effectStun;
						effectStun = NULL;
						return FALSE;
					}
				}
			}
			EffectTimeObjectBuffer[timeObjectIndex].Start();
			break;
		}

	}

	return TRUE;
}

int HoEffectMgr::StartMonsterDest(POINT3D current, POINT3D dest, POINT3D angle, int effectType)
{
	switch(effectType)
	{
		case MONSTER_SHADOW_SHOT1:
		{
			int timeObjectIndex = EmptyTimeObjectBufferIndex();
			if(timeObjectIndex == -1)
				return FALSE;

			HoNewEffectMove *effectMove = new HoNewEffectMove();
			if(effectMove != NULL)
			{
				dest.y -= 5000;
				effectMove->Start(current, dest, PatObj[24], MONSTER_SHADOW_SHOT1);
				if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(effectMove, 0))
				{
					delete effectMove;
					effectMove = NULL;
					return false;
				}
				EffectTimeObjectBuffer[timeObjectIndex].Start();
			}
			break;
		}

		case MONSTER_STYGIANLORD_SHOT1:
		{
			int timeObjectIndex = EmptyTimeObjectBufferIndex();
			if(timeObjectIndex == -1)
				return FALSE;

			HoNewEffectMove *effectMove = new HoNewEffectMove();
			if(effectMove != NULL)
			{
				current.y += 1000;
				effectMove->Start(current, dest, angle, PatObj[23], MONSTER_STYGIANLORD_SHOT1);
				if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(effectMove, 0))
				{
					delete effectMove;
					effectMove = NULL;
					return false;
				}
				EffectTimeObjectBuffer[timeObjectIndex].Start();
			}
			break;
		}

	}
	return TRUE;

}



		
int HoEffectMgr::StartMonsterDest(POINT3D current, POINT3D dest, int effectType)
{
	switch(effectType)
	{
		case MONSTER_SERQBUS_SHOT1:
		{
			int timeObjectIndex = EmptyTimeObjectBufferIndex();
			if(timeObjectIndex == -1)
				return FALSE;

			HoNewEffectMove *effectMove = new HoNewEffectMove();
			if(effectMove != NULL)
			{
				
				dest.y -= 1000;
				effectMove->Start(current, dest, MONSTER_SERQBUS_SHOT1);
				if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(effectMove, 0))
				{
					delete effectMove;
					effectMove = NULL;
					return false;
				}
				EffectTimeObjectBuffer[timeObjectIndex].Start();
			}
			break;
		}
		case MONSTER_SERQBUS_SHOT2:
		{
			int timeObjectIndex = EmptyTimeObjectBufferIndex();
			if(timeObjectIndex == -1)
				return FALSE;

			HoNewEffectMove *effectMove = new HoNewEffectMove();
			if(effectMove != NULL)
			{
				
				dest.y -= 1000;
				effectMove->Start(current, dest, MONSTER_SERQBUS_SHOT2);
				if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(effectMove, 0))
				{
					delete effectMove;
					effectMove = NULL;
					return false;
				}
				EffectTimeObjectBuffer[timeObjectIndex].Start();
			}
			break;
		}

		case MONSTER_SERQBUS_SHOT3:
		{
			int timeObjectIndex = EmptyTimeObjectBufferIndex();
			if(timeObjectIndex == -1)
				return FALSE;

			HoNewEffectMove *effectMove = new HoNewEffectMove();
			if(effectMove != NULL)
			{
				
				dest.y -= 1000;
				effectMove->Start(current, dest, MONSTER_SERQBUS_SHOT3);
				if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(effectMove, 0))
				{
					delete effectMove;
					effectMove = NULL;
					return false;
				}
				EffectTimeObjectBuffer[timeObjectIndex].Start();
			}
			break;
		}

		// 박철호 (머미 통상공격)
		case MONSTER_MUMMY_SHOT:
			{
				int timeObjectIndex = EmptyTimeObjectBufferIndex();
				if(timeObjectIndex == -1)
					return FALSE;

				HoNewEffectMove *effectMove = new HoNewEffectMove();
				if(effectMove != NULL)
				{
					
					dest.y -= 1000;
					effectMove->Start(current, dest, MONSTER_MUMMY_SHOT);
					if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(effectMove, 0))
					{
						delete effectMove;
						effectMove = NULL;
						return false;
					}
					EffectTimeObjectBuffer[timeObjectIndex].Start();
				}
				break;
			}


		case MONSTER_DMACHINE_MISSILE1:
		case MONSTER_TURTLECANON_SHOT:
		{
			
			int timeObjectIndex = EmptyTimeObjectBufferIndex();
			if(timeObjectIndex == -1)
				return FALSE;

			HoNewEffectMove *effectMove = new HoNewEffectMove();
			if(effectMove != NULL)
			{
				// 터틀 캐논일때 좌표를 올려준다
				if(effectType == MONSTER_TURTLECANON_SHOT)
				{
					current.y += 5000;
					dest.y -= 15*fONE;
				}
				effectMove->Start(current, dest, PatMissile, MONSTER_DMACHINE_MISSILE1);
				if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(effectMove, 0))
				{
					delete effectMove;
					effectMove = NULL;
					return false;
				}
				EffectTimeObjectBuffer[timeObjectIndex].Start();
			}
			break;
		}

		case MONSTER_DMACHINE_MISSILE2:
		{
			int timeObjectIndex = EmptyTimeObjectBufferIndex();
			if(timeObjectIndex == -1)
				return FALSE;

			HoNewEffectMove *effectMove = new HoNewEffectMove();
			if(effectMove != NULL)
			{
				current.y += 1000;
				dest.y -= 15*fONE;
				effectMove->Start(current, dest, PatMissile, MONSTER_DMACHINE_MISSILE2);
				if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(effectMove, 0))
				{
					delete effectMove;
					effectMove = NULL;
					return false;
				}
				EffectTimeObjectBuffer[timeObjectIndex].Start();
			}
			break;
		}
		
		case MONSTER_MEPHIT_SHOT1:
		{
			int timeObjectIndex = EmptyTimeObjectBufferIndex();
			if(timeObjectIndex == -1)
				return FALSE;
			
			HoEffectTracker *tracker = new HoEffectTracker();
			if(tracker != NULL)
			{
				HoParticleSystem *particleSystem = new HoParticleSystem();
				if(particleSystem != NULL)
				{
					POINT3D currentPos;
					POINT3D destPos;

					currentPos.x = current.x;
					currentPos.y = current.y;
					currentPos.z = current.z;
					
					destPos.x = dest.x;
					destPos.y = dest.y;
					destPos.z = dest.z;
					
					particleSystem->Start(currentPos.x, currentPos.y, currentPos.z, MaterialNum[3], MONSTER_MEPHIT_SHOT1);
					tracker->Start(currentPos, destPos, effectType, particleSystem);
					
					
					if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(tracker, 0))
					{
						delete tracker;
						tracker = NULL;
						return FALSE;
					}
					
					EffectTimeObjectBuffer[timeObjectIndex].AddObject(particleSystem, 0);
					EffectTimeObjectBuffer[timeObjectIndex].Start();
				}
			}				
			break;	
		}
		

		//ktj : 새로 만듦.  =========================================================start
		case MONSTER_MEPHIT_SHOT2:
		{
			int timeObjectIndex = EmptyTimeObjectBufferIndex();
			if(timeObjectIndex == -1)
				return FALSE;
			
			HoEffectTracker *tracker = new HoEffectTracker();
			if(tracker != NULL)
			{
				HoParticleSystem *particleSystem = new HoParticleSystem();
				if(particleSystem != NULL)
				{
					POINT3D currentPos;
					POINT3D destPos;

					currentPos.x = current.x;
					currentPos.y = current.y;
					currentPos.z = current.z;
					
					destPos.x = dest.x;
					destPos.y = dest.y;
					destPos.z = dest.z;
					
					//ktj : 교체
					//particleSystem->Start(currentPos.x, currentPos.y, currentPos.z, MaterialNum[3], MONSTER_MEPHIT_SHOT1);
					particleSystem->Start(currentPos.x, currentPos.y, currentPos.z, MaterialNum[3], MONSTER_MEPHIT_SHOT2);
					tracker->Start(currentPos, destPos, effectType, particleSystem);
					
					
					if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(tracker, 0))
					{
						delete tracker;
						tracker = NULL;
						return FALSE;
					}
					
					EffectTimeObjectBuffer[timeObjectIndex].AddObject(particleSystem, 0);
					EffectTimeObjectBuffer[timeObjectIndex].Start();
				}
			}				
			break;	
		}
		//ktj : 새로 만듦.  =========================================================end



		case MONSTER_IMP_SHOT1:
		{
			int timeObjectIndex = EmptyTimeObjectBufferIndex();
			if(timeObjectIndex == -1)
				return FALSE;
			
			HoEffectTracker *tracker = new HoEffectTracker();
			if(tracker != NULL)
			{
				HoParticleSystem *particleSystem = new HoParticleSystem();
				if(particleSystem != NULL)
				{
					POINT3D currentPos;
					POINT3D destPos;

					currentPos.x = current.x;
					currentPos.y = current.y;
					currentPos.z = current.z;
					
					destPos.x = dest.x;
					destPos.y = dest.y;
					destPos.z = dest.z;
					
					particleSystem->Start(currentPos.x, currentPos.y, currentPos.z, MaterialNum[2], MONSTER_IMP_SHOT1);
					tracker->Start(currentPos, destPos, effectType, particleSystem);
					
					
					if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(tracker, 0))
					{
						delete tracker;
						tracker = NULL;
						return FALSE;
					}
					
					EffectTimeObjectBuffer[timeObjectIndex].AddObject(particleSystem, 0);
					EffectTimeObjectBuffer[timeObjectIndex].Start();
				}
			}				
			break;	
		}


		//ktj : 새로 만듦.  =========================================================start
		case MONSTER_IMP_SHOT2:
		{
			int timeObjectIndex = EmptyTimeObjectBufferIndex();
			if(timeObjectIndex == -1)
				return FALSE;
			
			HoEffectTracker *tracker = new HoEffectTracker();
			if(tracker != NULL)
			{
				HoParticleSystem *particleSystem = new HoParticleSystem();
				if(particleSystem != NULL)
				{
					POINT3D currentPos;
					POINT3D destPos;

					currentPos.x = current.x;
					currentPos.y = current.y;
					currentPos.z = current.z;
					
					destPos.x = dest.x;
					destPos.y = dest.y;
					destPos.z = dest.z;
					
					
					//ktj : 동그란 날아가는놈								//MaterialNum[2]안의 메트리얼을사용함에주의:
					particleSystem->Start(currentPos.x, currentPos.y, currentPos.z, MaterialNum[2], MONSTER_IMP_SHOT2);

					//ktj : 임시
					//tracker->Start(currentPos, destPos, effectType, particleSystem);
					tracker->Start(currentPos, destPos, MONSTER_IMP_SHOT2, particleSystem);

					if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(tracker, 0))
					{
						delete tracker;
						tracker = NULL;
						return FALSE;
					}
					
					EffectTimeObjectBuffer[timeObjectIndex].AddObject(particleSystem, 0);
					EffectTimeObjectBuffer[timeObjectIndex].Start();
					//EffectTimeObjectBuffer[timeObjectIndex].DelayTimeCount = 1.0f
				}
			}				
			break;	
		}
		//=========================================================end




/*
				HoPrimitiveBillboard *primitiveBillBoard = new HoPrimitiveBillboard();
				if(primitiveBillBoard != NULL)
				{
					primitiveBillBoard->StartBillRect(current.x, current.y, current.z, "MonsterWeb1.ini", ANI_LOOP);
					tracker->Start(current, dest, effectType, primitiveBillBoard);
					if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(tracker, 0))

*/

		//ktj : 블랙홀모양  =========================================================start
		case MONSTER_IMP_SHOT3:
		{
			int timeObjectIndex = EmptyTimeObjectBufferIndex();
			if(timeObjectIndex == -1)
				return FALSE;
			
			HoEffectTracker *tracker = new HoEffectTracker();
			if(tracker != NULL)
			{
				HoParticleSystem *particleSystem = new HoParticleSystem();
				if(particleSystem != NULL)
				{
					POINT3D currentPos;
					POINT3D destPos;

					currentPos.x = current.x;
					currentPos.y = current.y;
					currentPos.z = current.z;
					
					destPos.x = dest.x;
					destPos.y = dest.y;
					destPos.z = dest.z;
					
					
					//ktj : 동그란 날아가는놈								//MaterialNum[2]안의 메트리얼을사용함에주의:
																			//다른색을쓰기위해 MaterialNum[3]으로바꿈.
					//particleSystem->Start(currentPos.x, currentPos.y, currentPos.z, MaterialNum[2], MONSTER_IMP_SHOT2);
					//particleSystem->Start(currentPos.x, currentPos.y, currentPos.z, MaterialNum[3], MONSTER_IMP_SHOT2);
					particleSystem->Start(currentPos.x, currentPos.y, currentPos.z, MaterialNum[7], MONSTER_IMP_SHOT3);
					

					//ktj : 임시
					//tracker->Start(currentPos, destPos, effectType, particleSystem);
					tracker->Start(currentPos, destPos, MONSTER_IMP_SHOT3, particleSystem);
					

					if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(tracker, 0))
					{
						delete tracker;
						tracker = NULL;
						return FALSE;
					}
					
					EffectTimeObjectBuffer[timeObjectIndex].AddObject(particleSystem, 0);
					
					EffectTimeObjectBuffer[timeObjectIndex].Start();
				}
			}	
			break;	
		}
		//=========================================================end



		case MONSTER_PIGON_SHOT1:
		{
			int timeObjectIndex = EmptyTimeObjectBufferIndex();
			if(timeObjectIndex == -1)
				return FALSE;
			
			HoEffectTracker *tracker = new HoEffectTracker();
			if(tracker != NULL)
			{
				HoParticleSystem *particleSystem = new HoParticleSystem();
				//HoEffectPower *effectPower = new HoEffectPower();
				if(particleSystem != NULL)
				{
					POINT3D currentPos;
					POINT3D destPos;

					currentPos.x = current.x;
					currentPos.y = current.y;
					currentPos.z = current.z;
					
					destPos.x = dest.x;
					destPos.y = dest.y;
					destPos.z = dest.z;
					
					//effectPower->Start(current.x, current.y, current.z, ANI_LOOP);
					
					particleSystem->Start(currentPos.x, currentPos.y, currentPos.z, MaterialNum[1], MONSTER_PIGON_SHOT1);
					tracker->Start(currentPos, destPos, effectType, particleSystem);
					
					//tracker->Start(currentPos, destPos, SKILL_MONSTER_FIRE1, effectPower);
					if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(tracker, 0))
					{
						delete tracker;
						tracker = NULL;
						return FALSE;
					}
					//EffectTimeObjectBuffer[timeObjectIndex].Start();
					EffectTimeObjectBuffer[timeObjectIndex].AddObject(particleSystem, 0);
					EffectTimeObjectBuffer[timeObjectIndex].Start();
				}
			}				
			break;	
		}

		case EFFECT_FIRE_CRACKER:			// 박철호 추가 (폭죽)
		{
			int timeObjectIndex = EmptyTimeObjectBufferIndex();
			if(timeObjectIndex == -1)
				return FALSE;
			
			HoEffectTracker *tracker = new HoEffectTracker();
			if(tracker != NULL)
			{
				HoParticleSystem *particleSystem = new HoParticleSystem();
				if(particleSystem != NULL)
				{
					POINT3D currentPos;
					POINT3D destPos;

					currentPos.x = current.x;
					currentPos.y = current.y;
					currentPos.z = current.z;
					
					destPos.x = dest.x;
					destPos.y = dest.y;
					destPos.z = dest.z;
					
					
					//ktj : 동그란 날아가는놈								//MaterialNum[2]안의 메트리얼을사용함에주의:
					particleSystem->Start(currentPos.x, currentPos.y, currentPos.z, MaterialNum[2], EFFECT_FIRE_CRACKER);

					//ktj : 임시
					//tracker->Start(currentPos, destPos, effectType, particleSystem);
					tracker->Start(currentPos, destPos, EFFECT_FIRE_CRACKER, particleSystem);

					if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(tracker, 0))
					{
						delete tracker;
						tracker = NULL;
						return FALSE;
					}
					
					EffectTimeObjectBuffer[timeObjectIndex].AddObject(particleSystem, 0);
					EffectTimeObjectBuffer[timeObjectIndex].Start();
					//EffectTimeObjectBuffer[timeObjectIndex].DelayTimeCount = 1.0f
				}
			}				
			break;	
		}

		case MONSTER_FORZENMIST_SHOT1:	// 아이스 볼트
		{
			int timeObjectIndex = EmptyTimeObjectBufferIndex();
			if(timeObjectIndex == -1)
				return FALSE;
			
			HoEffectTracker *tracker = new HoEffectTracker();
			if(tracker != NULL)
			{
				HoParticleSystem *particleSystem = new HoParticleSystem();
				//HoEffectPower *effectPower = new HoEffectPower();
				if(particleSystem != NULL)
				{
					POINT3D currentPos;
					POINT3D destPos;

					currentPos.x = current.x;
					currentPos.y = current.y;
					currentPos.z = current.z;
					
					destPos.x = dest.x;
					destPos.y = dest.y;
					destPos.z = dest.z;
					
					//effectPower->Start(current.x, current.y, current.z, ANI_LOOP);
					
					particleSystem->Start(currentPos.x, currentPos.y, currentPos.z, MaterialNum[1], MONSTER_FORZENMIST_SHOT1);
					tracker->Start(currentPos, destPos, effectType, particleSystem);
					
					//tracker->Start(currentPos, destPos, SKILL_MONSTER_FIRE1, effectPower);
					if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(tracker, 0))
					{
						delete tracker;
						tracker = NULL;
						return FALSE;
					}
					//EffectTimeObjectBuffer[timeObjectIndex].Start();
					EffectTimeObjectBuffer[timeObjectIndex].AddObject(particleSystem, 0);
					EffectTimeObjectBuffer[timeObjectIndex].Start();
				}
			}				
			break;	
		}
	case MONSTER_FORZENMIST_SHOT2:
		{
			int timeObjectIndex = EmptyTimeObjectBufferIndex();
			if(timeObjectIndex == -1)
				return FALSE;
			
			HoEffectTracker *tracker = new HoEffectTracker();
			if(tracker != NULL)
			{
				HoParticleSystem *particleSystem = new HoParticleSystem();
				//HoEffectPower *effectPower = new HoEffectPower();
				if(particleSystem != NULL)
				{
					POINT3D currentPos;
					POINT3D destPos;

					currentPos.x = current.x;
					currentPos.y = current.y;
					currentPos.z = current.z;
					
					destPos.x = dest.x;
					destPos.y = dest.y;
					destPos.z = dest.z;
					
					//effectPower->Start(current.x, current.y, current.z, ANI_LOOP);
					
					particleSystem->Start(currentPos.x, currentPos.y, currentPos.z, MaterialNum[1], MONSTER_FORZENMIST_SHOT2);
					tracker->Start(currentPos, destPos, effectType, particleSystem);
					
					//tracker->Start(currentPos, destPos, SKILL_MONSTER_FIRE1, effectPower);
					if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(tracker, 0))
					{
						delete tracker;
						tracker = NULL;
						return FALSE;
					}
					//EffectTimeObjectBuffer[timeObjectIndex].Start();
					EffectTimeObjectBuffer[timeObjectIndex].AddObject(particleSystem, 0);
					EffectTimeObjectBuffer[timeObjectIndex].Start();
				}
			}				
			break;	
		}

		case MONSTER_WEB_SHOT1:
		{

			int timeObjectIndex = EmptyTimeObjectBufferIndex();
			if(timeObjectIndex == -1)
				return FALSE;
			
			HoEffectPatDest *patDest = new HoEffectPatDest();
			if(patDest != NULL)
			{
				POINT3D currentPos;
				POINT3D destPos;

				currentPos.x = current.x;
				currentPos.y = current.y;
				currentPos.z = current.z;

				destPos.x = dest.x;
				destPos.y = dest.y;
				destPos.z = dest.z;


				patDest->StartDest(currentPos, destPos, PatObj[11], MONSTER_WEB_SHOT1, 0);
				patDest->AnimationEnd = PatAnimationEnd[11];

				if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(patDest, 0))
				{
					delete patDest;
					patDest = NULL;
					return FALSE;
				}
				EffectTimeObjectBuffer[timeObjectIndex].Start();
			}

			/*
			for(int index = 0; index < 10; index++)
			{
				HoPrimitiveBillboard *billBoard = new HoPrimitiveBillboard();
				if(billBoard != NULL)
				{
					int ang = rand()%ANGLE_360;

					POINT3D currentPos;
					POINT3D destPos;
					POINT	size;
					size.x = 15;
					size.y = 200+rand()%50;
					
					float RandomYaw = RANDOM_NUM   * 3.14159f * 2.0f;
					float RandomPitch = RANDOM_NUM * 180.f * 3.14159f / 180.0f ;
					
					//구모양으로 파티클이 날아가게 벡터를 조정한다.
					destPos.y = int(cosf( RandomPitch)*((90.f)*(90.f))+current.y);
					destPos.x = int(sinf( RandomPitch)*(120.f) * cosf(RandomYaw)*(120.f)+current.x);
					destPos.z = int(sinf( RandomPitch)*(120.f) * sinf(RandomYaw)*(120.f)+current.z);
					
					currentPos.y = int(cosf( RandomPitch)*(65.f*65.f)+current.y);
					currentPos.x = int(sinf( RandomPitch)*85.f * cosf(RandomYaw)*85.f+current.x);
					currentPos.z = int(sinf( RandomPitch)*85.f * sinf(RandomYaw)*85.f+current.z);
					
					billBoard->StartDestPath(currentPos, destPos, size, "MonsterWebHit1.ini", 300,PRIMITIVE_PATH_RECT_LINE);
					if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(billBoard, index*rand()%5))
					{
						delete billBoard;
						billBoard = NULL;
						return FALSE;
					}
					EffectTimeObjectBuffer[timeObjectIndex].Start();
				}
			}
			*/
			break;
			
			
			//int timeObjectIndex = EmptyTimeObjectBufferIndex();
			//if(timeObjectIndex == -1)
				//return FALSE;
			/*
			HoEffectTracker *tracker = new HoEffectTracker();
			if(tracker != NULL)
			{
				//HoParticleSystem *particleSystem = new HoParticleSystem();
				HoPrimitiveBillboard *primitiveBillBoard = new HoPrimitiveBillboard();
				if(primitiveBillBoard != NULL)
				{
					primitiveBillBoard->StartBillRect(current.x, current.y, current.z, "MonsterWeb1.ini", ANI_LOOP);
					tracker->Start(current, dest, effectType, primitiveBillBoard);
					if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(tracker, 0))
					{
						delete tracker;
						tracker = NULL;
						return FALSE;
					}
					
					EffectTimeObjectBuffer[timeObjectIndex].AddObject(primitiveBillBoard, 0);
					EffectTimeObjectBuffer[timeObjectIndex].Start();
				}
			}
			*/
			break;		
		}
	}

	return TRUE;
}

int HoEffectMgr::StartMonster(POINT3D pos, POINT3D angle, int effectType)
{
	switch(effectType)
	{
		/*
		case SKILL_DIASTROPHISM:
		{
			int timeObjectIndex = EmptyTimeObjectBufferIndex();
			if(timeObjectIndex == -1)
				return FALSE;
			
			pos.y = pos.y-1000;
			HoEffectLight *light = new HoEffectLight();
			if(light != NULL)
			{
				light->Start(pos.x, pos.y, pos.z, -150, -150, -255, 255, 300, 2);
				if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(light, 0))
				{
					delete light;
					light = NULL;
					return FALSE;
				}
				EffectTimeObjectBuffer[timeObjectIndex].Start();
			}


			HoEffectPat	*pat = new HoEffectPat();
			if(pat != NULL)
			{
				 pat->StartAni( pos.x, pos.y, pos.z,0,0,0, PatObj[22]);
				pat->AnimationEnd = PatAnimationEnd[22];
				pat->SetBlendStep(255);
				if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(pat, 10))
				{
					delete pat;
					pat = NULL;
					return FALSE;
				}
				EffectTimeObjectBuffer[timeObjectIndex].Start();
			}
			break;
		}
		*/

		case MONSTER_OMICRON_HIT1:
		{
			int timeObjectIndex = EmptyTimeObjectBufferIndex();
			if(timeObjectIndex == -1)
				return FALSE;
			
			pos.y = pos.y-1000;
			HoEffectLight *light = new HoEffectLight();
			if(light != NULL)
			{
				light->Start(pos.x, pos.y, pos.z, -150, -150, -255, 255, 300, 2);
				if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(light, 0))
				{
					delete light;
					light = NULL;
					return FALSE;
				}
				EffectTimeObjectBuffer[timeObjectIndex].Start();
			}


			HoEffectPat	*pat = new HoEffectPat();
			if(pat != NULL)
			{
				 pat->StartAni( pos.x, pos.y, pos.z,0,0,0, PatObj[22]);
				pat->AnimationEnd = PatAnimationEnd[22];
				pat->SetBlendStep(255);
				if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(pat, 10))
				{
					delete pat;
					pat = NULL;
					return FALSE;
				}
				EffectTimeObjectBuffer[timeObjectIndex].Start();
			}
			break;
		}

		case MONSTER_COLDEYE_SKILL:
		{
			int timeObjectIndex = EmptyTimeObjectBufferIndex();
			if(timeObjectIndex == -1)
				return FALSE;
			break;
		}

		case MONSTER_STYGIANLORD_MAGIC1:
 		{
			int timeObjectIndex = EmptyTimeObjectBufferIndex();
			if(timeObjectIndex == -1)
				return FALSE;
			
			

			/*
			int term = 90;
			HoEffectLight *light = new HoEffectLight();
			if(light != NULL)
			{
				light->Start(pos.x-term*fONE, pos.y, pos.z, 255,100,100,255,190,2);
				if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(light, 0))
				{
					delete light;
					light = NULL;
					return FALSE;
				}
				EffectTimeObjectBuffer[timeObjectIndex].Start();
			}
		
			if(pat != NULL)
			{
				pat->StartAni( pos.x, pos.y, pos.z-10*fONE,0,0,0, PatObj[22], "stygianlord1");
				pat->AnimationEnd = PatAnimationEnd[19];

				if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(pat, 0))
				{
					delete pat;
					pat = NULL;
					return FALSE;
				}
				EffectTimeObjectBuffer[timeObjectIndex].Start();
			}

			pat = new HoEffectPatGetPos();
			if(pat != NULL)
			{
				pat->StartAni( pos.x, pos.y, pos.z+10*fONE,0,0,0, PatObj[23]);
				pat->AnimationEnd = PatAnimationEnd[19];

				if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(pat, 0))
				{
					delete pat;
					pat = NULL;
					return FALSE;
				}
				EffectTimeObjectBuffer[timeObjectIndex].Start();
			}

			pat = new HoEffectPatGetPos();
			if(pat != NULL)
			{
				pat->StartAni( pos.x-10*fONE, pos.y, pos.z,0,0,0, PatObj[24]);
				pat->AnimationEnd = PatAnimationEnd[19];

				if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(pat, 0))
				{
					delete pat;
					pat = NULL;
					return FALSE;
				}
				EffectTimeObjectBuffer[timeObjectIndex].Start();
			}
			

			pat = new HoEffectPatGetPos();
			if(pat != NULL)
			{
				pos.y += 1000;
				pat->StartAni( pos.x+10*fONE, pos.y, pos.z,0,0,0, PatObj[25]);
				pat->AnimationEnd = PatAnimationEnd[19];

				if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(pat, 15))
				{
					delete pat;
					pat = NULL;
					return FALSE;
				}
				EffectTimeObjectBuffer[timeObjectIndex].Start();
			}
			*/
			break;
		}

		/*
		case MONSTER_FURY_MAGIC1:
		{
			pos.y += 1500;
			int timeObjectIndex = EmptyTimeObjectBufferIndex();
			if(timeObjectIndex == -1)
				return FALSE;
			
			HoEffectLight *light = new HoEffectLight();
			if(light != NULL)
			{
				light->Start(pos.x, pos.y, pos.z, 255, 80, 0, 255, 200, 2);
				if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(light, 0))
				{
					delete light;
					light = NULL;
					return FALSE;
				}
				EffectTimeObjectBuffer[timeObjectIndex].Start();
			}

			light = new HoEffectLight();
			if(light != NULL)
			{
				light->Start(pos.x, pos.y, pos.z, 255, 80, 0, 255, 450, 2);
				if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(light, 25))
				{
					delete light;
					light = NULL;
					return FALSE;
				}
				EffectTimeObjectBuffer[timeObjectIndex].Start();
			}
			
			HoEffectPat	*pat = new HoEffectPat();
			if(pat != NULL)
			{
				pat->StartAni( pos.x, pos.y, pos.z,0,0,0, PatObj[19]);
				pat->AnimationEnd = PatAnimationEnd[19];

				if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(pat, 25))
				{
					delete pat;
					pat = NULL;
					return FALSE;
				}
				EffectTimeObjectBuffer[timeObjectIndex].Start();
			}
			
			pat = new HoEffectPat();
			if(pat != NULL)
			{
				pat->StartAni( pos.x, pos.y, pos.z,0,angle.y,0, PatObj[20]);
				pat->AnimationEnd = PatAnimationEnd[20];

				if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(pat, 25))
				{
					delete pat;
					pat = NULL;
					return FALSE;
				}
				EffectTimeObjectBuffer[timeObjectIndex].Start();
			}

			pat = new HoEffectPat();
			if(pat != NULL)
			{
				pat->StartAni( pos.x, pos.y, pos.z,0,angle.y,0, PatObj[20]);
				pat->AnimationEnd = PatAnimationEnd[20];

				if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(pat, 40))
				{
					delete pat;
					pat = NULL;
					return FALSE;
				}
				EffectTimeObjectBuffer[timeObjectIndex].Start();
			}

			break;

		}
		*/

		case MONSTER_HEADER_CUTTER_HIT1:
		{
			int timeObjectIndex = EmptyTimeObjectBufferIndex();
			if(timeObjectIndex == -1)
				return FALSE;

			SetDynLight( pos.x, pos.y, pos.z, 100, 100, 255, 255, 230, 1);
			HoEffectPat	*pat = new HoEffectPat();
			if(pat != NULL)
			{
				pat->StartAni( pos.x, pos.y, pos.z,angle.x, angle.y, angle.z, PatObj[18]);
				pat->AnimationEnd = PatAnimationEnd[18];

				if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(pat, 0))
				{
					delete pat;
					pat = NULL;
					return FALSE;
				}
				EffectTimeObjectBuffer[timeObjectIndex].Start();
			}

			pat = new HoEffectPat();
			if(pat != NULL)
			{
				pat->StartAni( pos.x, pos.y, pos.z,angle.x, angle.y, angle.z, PatObj[21]);
				pat->AnimationEnd = PatAnimationEnd[21];

				if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(pat, 6))
				{
					delete pat;
					pat = NULL;
					return FALSE;
				}
				EffectTimeObjectBuffer[timeObjectIndex].Start();
			}
			
		}	

	}
	return true;

}



int HoEffectMgr::StartMonster(POINT3D current, int effectType)
{
	switch(effectType)
	{
		case MONSTER_SERQBUS_MAGIC_ATTACK1:
		{
			SetDynLight( current.x, current.y, current.z, 100, 100, 255, 255, 200, 1);
			
			POINT3D pos;
			pos.x = current.x;
			pos.y = current.y;
			pos.z = current.z;
	
			g_NewParticleMgr.Start("SerqbusMagicAttack1", pos);
			break;
		}

		case MONSTER_SERQBUS_MAGIC_ATTACK2:
		{
			SetDynLight( current.x, current.y, current.z, 255, 100, 50, 255, 200, 1);

			POINT3D pos;
			pos.x = current.x;
			pos.y = current.y;
			pos.z = current.z;

			g_NewParticleMgr.Start("SerqbusMagicAttack2", pos);
			break;
		}

		case MONSTER_SERQBUS_MAGIC_ATTACK3:
		{
			SetDynLight( current.x, current.y, current.z, 108, 8, 136, 255, 250, 1);
            
			POINT3D pos;
			pos.x = current.x;
			pos.y = current.y;
			pos.z = current.z;

			g_NewParticleMgr.Start("SerqbusMagicAttack3", pos);
			break;
		}


		case MONSTER_SERQBUS_MAGIC1:
		{
			SetDynLight( current.x, current.y, current.z, 100, 100, 255, 255, 200, 1);
			
			POINT3D pos;
			pos.x = current.x;
			pos.y = current.y+5000;
			pos.z = current.z;
	
			g_NewParticleMgr.Start("SerqbusMagic1", pos);
			break;
		}

		case MONSTER_SERQBUS_MAGIC2:
		{
			SetDynLight( current.x, current.y, current.z, 255, 100, 50, 255, 200, 1);

			POINT3D pos;
			pos.x = current.x;
			pos.y = current.y+5000;
			pos.z = current.z;

			g_NewParticleMgr.Start("SerqbusMagic2", pos);
			break;
		}

		case MONSTER_SERQBUS_MAGIC3:
		{
			
			SetDynLight( current.x, current.y, current.z, 108, 8, 136, 255, 250, 1);
            
			POINT3D pos;
			pos.x = current.x;
			pos.y = current.y+5000;
			pos.z = current.z;

			g_NewParticleMgr.Start("SerqbusMagic3", pos);
			break;
		}

		case MONSTER_STYGIANLORD_MAGIC1:
		{
			int timeObjectIndex = EmptyTimeObjectBufferIndex();
			if(timeObjectIndex == -1)
				return FALSE;

			current.y = current.y - 1000;
			SetDynLight( current.x, current.y+1000, current.z, 220, 100, 50, 255, 220, 1);
			int index; for( index = 0; index < 10; index++)
			{
				HoNewEffectMove *effectMove = new HoNewEffectMove();
				if(effectMove != NULL)
				{
					int ang = ANGLE_360/10*index;
					ang = -ang;
					ang = ang&ANGCLIP;

					POINT3D dest;
					POINT3D cur;
					cur.x = current.x+int((GetCos[ang]/65536.f)*(float(12000)+rand()%6000) );
					cur.y = current.y;
					cur.z = current.z+int((GetSin[ang]/65536.f)*(float(12000)+rand()%6000) );

					dest.x = cur.x;
					dest.y = cur.y+15000+rand()%5000;
					dest.z = cur.z;


					effectMove->Start(cur, dest, MONSTER_STYGIANLORD_PARTICLE1);
					if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(effectMove, index*7))
					{
						delete effectMove;
						effectMove = NULL;
						return false;
					}
					EffectTimeObjectBuffer[timeObjectIndex].Start();
				}
			}

			break;
		}

		case MONSTER_PIGON_PARTICLE1:
		case EFFECT_FIRE_CRACKER:
		{
			int objIndex = -1;
			int timeObjectIndex = EmptyTimeObjectBufferIndex();
			if(timeObjectIndex == -1)
				return FALSE;

			HoPrimitiveBillboard *primitiveBillBoard = new HoPrimitiveBillboard();
			primitiveBillBoard->StartPath(current, "FireParticle1.ini", ANI_LOOP);	//ktj : 붉은색파티클
			
			int ang = rand()%ANGLE_360;
			HoPhysicsParticle *physicsParticle = new HoPhysicsParticle();
			POINT3D velocity;
			velocity.x = int(((float)GetCos[ang]/65536.f)*85.f);
			velocity.y = 70;
			velocity.z = int(((float)GetSin[ang]/65536.f)*85.f);

			physicsParticle->Start(velocity);
			physicsParticle->SetGravity(float(rand()%3+2));
			physicsParticle->SetLive(rand()%5+15);
			
			POINT3D angle;
			angle.x = 0;
			angle.y = ANGLE_360/10;
			angle.z = 0;
			physicsParticle->SetDestAngle(angle);
			primitiveBillBoard->SetPhysics(physicsParticle, PRIMITIVE_DEFAULT_RECT_PARTICLE | MOVE_ANGLE);
			
			POINT size;
			if(effectType == MONSTER_PIGON_PARTICLE1)
			{
				size.x = 25;
				size.y = 25;
			}
			else if(effectType == EFFECT_FIRE_CRACKER)
			{
				size.x = 10;
				size.y = 10;
			}
			primitiveBillBoard->SetSize(size);

			objIndex = EmptyObjectBufferIndex();
			if(objIndex == -1)
				break;

			EffectObjectBuffer[objIndex] = primitiveBillBoard;
			break;
		}




		//ktj :MONSTER_PIGON_PARTICLE1의 블랙홀에씀.====================start
		case MONSTER_PIGON_PARTICLE1_BLH:
		{
			int objIndex = -1;
			int timeObjectIndex = EmptyTimeObjectBufferIndex();
			if(timeObjectIndex == -1)
				return FALSE;

			HoPrimitiveBillboard *primitiveBillBoard = new HoPrimitiveBillboard();
			primitiveBillBoard->StartPath(current, "FireParticle1_blh.ini", ANI_LOOP);	//ktj : 붉은색파티클
			
			int ang = rand()%ANGLE_360;
			HoPhysicsParticle *physicsParticle = new HoPhysicsParticle();
			POINT3D velocity;
			velocity.x = int(((float)GetCos[ang]/65536.f)*85.f);
			velocity.y = 70;
			velocity.z = int(((float)GetSin[ang]/65536.f)*85.f);

			physicsParticle->Start(velocity);
			physicsParticle->SetGravity(float(rand()%3+2));
			physicsParticle->SetLive(rand()%5+15);
			
			POINT3D angle;
			angle.x = 0;
			angle.y = ANGLE_360/10;
			angle.z = 0;
			physicsParticle->SetDestAngle(angle);
			primitiveBillBoard->SetPhysics(physicsParticle, PRIMITIVE_DEFAULT_RECT_PARTICLE | MOVE_ANGLE);
			
			POINT size;
			size.x = 25;
			size.y = 25;
			primitiveBillBoard->SetSize(size);

			objIndex = EmptyObjectBufferIndex();
			if(objIndex == -1)
				break;

			EffectObjectBuffer[objIndex] = primitiveBillBoard;
			break;
		}



		//ktj : 몬스터의 검은기운이 타오르는 것을 표현하기위해 새로만듦.====================start
		case MONSTER_PIGON_PARTICLE2:
		{
			int objIndex = -1;
			int timeObjectIndex = EmptyTimeObjectBufferIndex();
			if(timeObjectIndex == -1)
				return FALSE;

			HoPrimitiveBillboard *primitiveBillBoard = new HoPrimitiveBillboard();
			primitiveBillBoard->StartPath(current, "FireParticle2.ini", ANI_LOOP);	//ktj : 검은색파티클
			
			//ktj : 새로만듦
			int ang = rand()%ANGLE_360;	//원본
			//int ang = rand()%ANGLE_90;
			HoPhysicsParticle *physicsParticle = new HoPhysicsParticle();
			POINT3D velocity;
			velocity.x = int(((float)GetCos[ang]/65536.f)*85.f);
			velocity.y = 70;
			velocity.z = int(((float)GetSin[ang]/65536.f)*85.f);

			physicsParticle->Start(velocity);
			physicsParticle->SetGravity(float(rand()%3+2));
			physicsParticle->SetLive(rand()%5+15);
			
			POINT3D angle;
			angle.x = 0;
			angle.y = ANGLE_360/10;
			angle.z = 0;
			physicsParticle->SetDestAngle(angle);
			primitiveBillBoard->SetPhysics(physicsParticle, PRIMITIVE_DEFAULT_RECT_PARTICLE | MOVE_ANGLE);
			
			POINT size;
			size.x = 25;
			size.y = 25;
			primitiveBillBoard->SetSize(size);

			objIndex = EmptyObjectBufferIndex();
			if(objIndex == -1)
				break;

			EffectObjectBuffer[objIndex] = primitiveBillBoard;
			break;
		}

		//===================================================================================end





		case MONSTER_PIGON_POWER1:
		{
			int timeObjectIndex = EmptyTimeObjectBufferIndex();
			if(timeObjectIndex == -1)
				return FALSE;
			
			HoEffectPower *effectPower = new HoEffectPower();
			effectPower->Start(current.x, current.y, current.z,  1);	//ktj : 1 -> power1.ini
			
			if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(effectPower, 0))
			{
				delete effectPower;
				effectPower = NULL;
				return FALSE;
			}
			EffectTimeObjectBuffer[timeObjectIndex].Start();
			break;
		}

		//ktj : 몬스터의 검은기운이 타오르는 것을 표현하기위해 새로만듦.====================start
		case MONSTER_PIGON_POWER2:
		{
			
			int timeObjectIndex = EmptyTimeObjectBufferIndex();
			if(timeObjectIndex == -1)
				return FALSE;
			
			HoEffectPower *effectPower = new HoEffectPower();
			effectPower->Start(current.x, current.y, current.z, 2);	//ktj : 2 -> power2.ini
			
			if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(effectPower, 0))
			{
				delete effectPower;
				effectPower = NULL;
				return FALSE;
			}
			EffectTimeObjectBuffer[timeObjectIndex].Start();
			
			break;
		}
		//===================================================================================end
		
		case MONSTER_MEPHIT_HIT1:
		case MONSTER_MEPHIT_HIT2:		//ktj : 추가
		{
			int timeObjectIndex = EmptyTimeObjectBufferIndex();
			if(timeObjectIndex == -1)
				return FALSE;
			
			int index; for( index = 0; index < 10; index++)
			{
				HoPrimitiveBillboard *billBoard = new HoPrimitiveBillboard();
				if(billBoard != NULL)
				{
					int ang = rand()%ANGLE_360;

					//POINT3D currentPos;
					POINT3D worldPos;
					POINT3D localPos;
					POINT3D destPos;
					POINT	size;
					size.x = 6;
					size.y = 70+rand()%50;
					
					float RandomYaw = RANDOM_NUM   * 3.14159f * 2.0f;
					float RandomPitch = RANDOM_NUM * 180.f * 3.14159f / 180.0f ;
					
					//구모양으로 파티클이 날아가게 벡터를 조정한다.
					destPos.y = int(cosf( RandomPitch)*((90.f)*(90.f))+current.y);
					destPos.x = int(sinf( RandomPitch)*(120.f) * cosf(RandomYaw)*(120.f)+current.x);
					destPos.z = int(sinf( RandomPitch)*(120.f) * sinf(RandomYaw)*(120.f)+current.z);
					
					worldPos.x = current.x;
					worldPos.y = current.y;
					worldPos.z = current.z;

					localPos.x = 0;
					localPos.y = 0;
					localPos.z = 0;
					
					billBoard->StartDestPath(worldPos, localPos
						, destPos, size, "MonsterMephit2.ini", 200,PRIMITIVE_PATH_RECT_LINE);
					if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(billBoard, index*rand()%5))
					{
						delete billBoard;
						billBoard = NULL;
						return FALSE;
					}
					EffectTimeObjectBuffer[timeObjectIndex].Start();
				}
			}
			
			//EffectTimeObjectBuffer[timeObjectIndex].Start();
			HoPrimitiveBillboard *primitiveBillBoard;
			for(index = 0; index < 4; index++)
			{
				primitiveBillBoard = new HoPrimitiveBillboard();
				POINT3D currentPos;
				currentPos.x = current.x;
				currentPos.y = current.y;
				currentPos.z = current.z;
				primitiveBillBoard->StartPath(currentPos, "MonsterMephit1.ini", ANI_LOOP);
				
				int ang = rand()%ANGLE_360;
				HoPhysicsParticle *physicsParticle = new HoPhysicsParticle();
				POINT3D velocity;
				velocity.x = int(((float)GetCos[ang]/65536.f)*85.f);
				velocity.y = 70;
				velocity.z = int(((float)GetSin[ang]/65536.f)*85.f);

				//ktj : 추가
				if(effectType == MONSTER_MEPHIT_HIT2) {
					velocity.x *= 2;
					velocity.y *= 2;
					velocity.z *= 2;
				}


				physicsParticle->Start(velocity);
				physicsParticle->SetGravity(float(rand()%3+2));
				physicsParticle->SetLive(rand()%5+10);
				
				POINT3D angle;
				angle.x = 0;
				angle.y = ANGLE_360/10;
				angle.z = 0;
				physicsParticle->SetDestAngle(angle);
				primitiveBillBoard->SetPhysics(physicsParticle, PRIMITIVE_DEFAULT_RECT_PARTICLE | MOVE_ANGLE);
				
				POINT size;
				size.x = 20;
				size.y = 20;

				//ktj : 추가
				if(effectType == MONSTER_MEPHIT_HIT2) {
					size.x *= 2;
					size.y *= 2;
				}

				primitiveBillBoard->SetSize(size);

				if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(primitiveBillBoard, index))
				{
					delete primitiveBillBoard;
					primitiveBillBoard = NULL;
					return FALSE;
				}
			}
			break;
		}

		case MONSTER_IMP_HIT1:
		case MONSTER_IMP_HIT2:		//ktj : 추가
		case MONSTER_IMP_HIT3:		//ktj : 추가
		{
			int index ;
			int timeObjectIndex = EmptyTimeObjectBufferIndex();
			if(timeObjectIndex == -1)
				return FALSE;
			
			HoPrimitiveBillboard *primitiveBillBoard = new HoPrimitiveBillboard();
			//ktj : 원본
			//primitiveBillBoard->StartBillRect(current.x, current.y, current.z, "Power1.ini", ANI_ONE);

			//ktj : 새로 넣음.
			
			if(effectType == MONSTER_IMP_HIT1) {
				primitiveBillBoard->StartBillRect(current.x, current.y, current.z, "Power1.ini", ANI_ONE);
			}
			if(effectType == MONSTER_IMP_HIT2) {								//KTJ : 크기를 따로넣을수있음.
				primitiveBillBoard->StartBillRect(current.x, current.y, current.z, 100,100, "Power1.ini", ANI_ONE);
			}

			if(effectType == MONSTER_IMP_HIT3) {								//KTJ : 크기를 따로넣을수있음.
				primitiveBillBoard->StartBillRect(current.x, current.y, current.z, 60,60, "blackhole.ini", ANI_ONE);
			}



			if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(primitiveBillBoard, 0))
			{
				delete primitiveBillBoard;
				primitiveBillBoard = NULL;
				return FALSE;
			}

			EffectTimeObjectBuffer[timeObjectIndex].Start();
			


			//ktj : MONSTER_IMP_HIT3은 검은기운이 나오므로 빨간색의 터지는 부분이 있으면 안되기 때문에 실행안함
			if(effectType != MONSTER_IMP_HIT3) 
			{
			
			for(index = 0; index < 3; index++)	//빨갛게 터지는 부분임.
			{
				primitiveBillBoard = new HoPrimitiveBillboard();
				POINT3D currentPos;
				currentPos.x = current.x;
				currentPos.y = current.y;
				currentPos.z = current.z;
				primitiveBillBoard->StartPath(currentPos, "MonsterImp1.ini", ANI_LOOP);
				
				int ang = rand()%ANGLE_360;
				HoPhysicsParticle *physicsParticle = new HoPhysicsParticle();
				POINT3D velocity;
				velocity.x = int(((float)GetCos[ang]/65536.f)*85.f);
				velocity.y = 70;
				velocity.z = int(((float)GetSin[ang]/65536.f)*85.f);

				physicsParticle->Start(velocity);
				physicsParticle->SetGravity(float(rand()%3+2));
				physicsParticle->SetLive(rand()%5+10);
				
				POINT3D angle;
				angle.x = 0;
				angle.y = ANGLE_360/10;
				angle.z = 0;
				physicsParticle->SetDestAngle(angle);
				primitiveBillBoard->SetPhysics(physicsParticle, PRIMITIVE_DEFAULT_RECT_PARTICLE | MOVE_ANGLE);
				
				POINT size;
				size.x = 20;
				size.y = 20;
				primitiveBillBoard->SetSize(size);

				if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(primitiveBillBoard, index))
				{
					delete primitiveBillBoard;
					primitiveBillBoard = NULL;
					return FALSE;
				}
			}

			}

			
			//폭탄 파편(파티클)
			for(index = 0; index < 10; index++)
			{
				HoPrimitiveBillboard *primitiveBillBoard = new HoPrimitiveBillboard();
				if(primitiveBillBoard != NULL)
				{
					POINT3D currentPos;
					currentPos.x = current.x;
					currentPos.y = current.y;
					currentPos.z = current.z;
					primitiveBillBoard->StartPath(currentPos, "ParticleMeteo1_blue.ini", ANI_LOOP);
					
					int ang = rand()%ANGLE_360;
					HoPhysicsParticle *physicsParticle = new HoPhysicsParticle();
					POINT3D velocity;
					/*
					//ktj : 원본
					velocity.x = int((GetCos[ang]/65536.f)*(150.f+rand()%50));
					velocity.y = 100+rand()%250;
					velocity.z = int((GetSin[ang]/65536.f)*(150.f+rand()%50));
					*/


					//ktj : 추가
					if(effectType == MONSTER_IMP_HIT2) {
						velocity.x = int((GetCos[ang]/65536.f)*(170.f+rand()%50));
						velocity.y = 100+rand()%250;
						velocity.z = int((GetSin[ang]/65536.f)*(170.f+rand()%50));
					}
					else {
						velocity.x = int((GetCos[ang]/65536.f)*(150.f+rand()%50));
						velocity.y = 100+rand()%250;
						velocity.z = int((GetSin[ang]/65536.f)*(150.f+rand()%50));
					}


					physicsParticle->Start(velocity);

					if(effectType == MONSTER_IMP_HIT2) {
						physicsParticle->SetGravity(float(-rand()%8-8));
					}
					else {
						physicsParticle->SetGravity(float(-rand()%5-5));
					}
					physicsParticle->SetLive(rand()%20+30);
					
					primitiveBillBoard->SetPhysics(physicsParticle, PRIMITIVE_PATH_RECT_LINE);

					
					POINT size;
					size.x = 7;
					size.y = 40;

					//ktj : 추가
					if(effectType == MONSTER_IMP_HIT2) {
						size.x *= 2;
						size.y *= 2;
					}

					primitiveBillBoard->SetSize(size);
					if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(primitiveBillBoard, index/4))
					{
						delete primitiveBillBoard;
						primitiveBillBoard = NULL;
						return FALSE;
					}
					EffectTimeObjectBuffer[timeObjectIndex].Start();
				}
			}

			EffectTimeObjectBuffer[timeObjectIndex].Start();
			break;
		}


		case MONSTER_COLDEYE_SKILL:
		{
			int timeObjectIndex = EmptyTimeObjectBufferIndex();
			if(timeObjectIndex == -1)
				return FALSE;

			//current.x = current.x - 1000;
			SetDynLight( current.x, current.y, current.z, 50, 100, 255, 255, 220, 1);
			for(int index = 0; index < 20; index++)
			{
				HoNewEffectMove *effectMove = new HoNewEffectMove();
				if(effectMove != NULL)
				{
					int ang = (ANGLE_360+ANGLE_360/4)/20*index;
					ang = -ang;
					ang = ang&ANGCLIP;

					POINT3D dest;
					POINT3D cur;
					cur.x = current.x+int((GetSin[ang]/65536.f)*(float(10000)+rand()%3500) );
					cur.y = current.y+8000;//;
					cur.z = current.z+int((GetCos[ang]/65536.f)*(float(10000)+rand()%3500) );

					dest.x = cur.x+int((GetSin[ang]/65536.f)*(float(22000)+rand()%6000) );
					dest.y = cur.y;
					dest.z = cur.z+int((GetCos[ang]/65536.f)*(float(22000)+rand()%6000) );


					effectMove->Start(cur, dest, MONSTER_COLDEYE_SKILL);
					if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(effectMove, index*7))
					{
						delete effectMove;
						effectMove = NULL;
						return false;
					}
					EffectTimeObjectBuffer[timeObjectIndex].Start();
				}
			}

			break;
		}

		case EFFECT_FIRE_CRACKER_HIT:
		{
			int index;
			int timeObjectIndex = EmptyTimeObjectBufferIndex();
			if(timeObjectIndex == -1)
				return FALSE;
			
			HoPrimitiveBillboard *primitiveBillBoard = new HoPrimitiveBillboard();

			primitiveBillBoard->StartBillRect(current.x, current.y, current.z, 100,100, "Power1.ini", ANI_ONE);


			if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(primitiveBillBoard, 0))
			{
				delete primitiveBillBoard;
				primitiveBillBoard = NULL;
				return FALSE;
			}

			EffectTimeObjectBuffer[timeObjectIndex].Start();

			
			for(index = 0; index < 3; index++)	//빨갛게 터지는 부분임.
			{
				primitiveBillBoard = new HoPrimitiveBillboard();
				POINT3D currentPos;
				currentPos.x = current.x;
				currentPos.y = current.y;
				currentPos.z = current.z;
				primitiveBillBoard->StartPath(currentPos, "MonsterImp1.ini", ANI_LOOP);
				
				int ang = rand()%ANGLE_360;
				HoPhysicsParticle *physicsParticle = new HoPhysicsParticle();
				POINT3D velocity;
				velocity.x = int(((float)GetCos[ang]/65536.f)*85.f);
				velocity.y = 70;
				velocity.z = int(((float)GetSin[ang]/65536.f)*85.f);

				physicsParticle->Start(velocity);
				physicsParticle->SetGravity(float(rand()%3+2));
				physicsParticle->SetLive(rand()%5+10);
				
				POINT3D angle;
				angle.x = 0;
				angle.y = ANGLE_360/10;
				angle.z = 0;
				physicsParticle->SetDestAngle(angle);
				primitiveBillBoard->SetPhysics(physicsParticle, PRIMITIVE_DEFAULT_RECT_PARTICLE | MOVE_ANGLE);
				
				POINT size;
				size.x = 20;
				size.y = 20;
				primitiveBillBoard->SetSize(size);

				if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(primitiveBillBoard, index))
				{
					delete primitiveBillBoard;
					primitiveBillBoard = NULL;
					return FALSE;
				}
			}

			
			//폭탄 파편(파티클)
			for(index = 0; index < 10; index++)
			{
				HoPrimitiveBillboard *primitiveBillBoard = new HoPrimitiveBillboard();
				if(primitiveBillBoard != NULL)
				{
					POINT3D currentPos;
					currentPos.x = current.x;
					currentPos.y = current.y;
					currentPos.z = current.z;
					primitiveBillBoard->StartPath(currentPos, "ParticleMeteo1_blue.ini", ANI_LOOP);
					
					int ang = rand()%ANGLE_360;
					HoPhysicsParticle *physicsParticle = new HoPhysicsParticle();
					POINT3D velocity;

					velocity.x = int((GetCos[ang]/65536.f)*(120.f+rand()%50));
					velocity.y = 70+rand()%250;
					velocity.z = int((GetSin[ang]/65536.f)*(120.f+rand()%50));


					physicsParticle->Start(velocity);

					physicsParticle->SetGravity(float(-rand()%8-8));

					physicsParticle->SetLive(rand()%20+50);
					
					primitiveBillBoard->SetPhysics(physicsParticle, PRIMITIVE_PATH_RECT_LINE);

					
					POINT size;
					size.x = 7;
					size.y = 40;

					size.x *= 2;
					size.y *= 2;

					primitiveBillBoard->SetSize(size);
					if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(primitiveBillBoard, index/4))
					{
						delete primitiveBillBoard;
						primitiveBillBoard = NULL;
						return FALSE;
					}
					EffectTimeObjectBuffer[timeObjectIndex].Start();
				}
			}

			EffectTimeObjectBuffer[timeObjectIndex].Start();
			break;
		}

		
		case MONSTER_WEB_HIT1:
		{
			SetDynLight( current.x, current.y, current.z, 200, 200, 200, 255, 100 , 1);
			
			int timeObjectIndex = EmptyTimeObjectBufferIndex();
			if(timeObjectIndex == -1)
				return FALSE;
			
			for(int index = 0; index < 10; index++)
			{
				HoPrimitiveBillboard *billBoard = new HoPrimitiveBillboard();
				if(billBoard != NULL)
				{
					int ang = rand()%ANGLE_360;

					//POINT3D currentPos;
					POINT3D worldPos;
					POINT3D localPos;
					POINT3D destPos;
					POINT	size;
					size.x = 15;
					size.y = 200+rand()%50;
					
					float RandomYaw = RANDOM_NUM   * 3.14159f * 2.0f;
					float RandomPitch = RANDOM_NUM * 180.f * 3.14159f / 180.0f ;
					
					//구모양으로 파티클이 날아가게 벡터를 조정한다.
					destPos.y = int(cosf( RandomPitch)*((90.f)*(90.f))+current.y);
					destPos.x = int(sinf( RandomPitch)*(120.f) * cosf(RandomYaw)*(120.f)+current.x);
					destPos.z = int(sinf( RandomPitch)*(120.f) * sinf(RandomYaw)*(120.f)+current.z);
					
					worldPos.x = current.x;
					worldPos.y = current.y;
					worldPos.z = current.z;

					
					localPos.y = int(cosf( RandomPitch)*(65.f*65.f));
					localPos.x = int(sinf( RandomPitch)*85.f * cosf(RandomYaw)*85.f);
					localPos.z = int(sinf( RandomPitch)*85.f * sinf(RandomYaw)*85.f);
					
					billBoard->StartDestPath(worldPos, localPos, destPos, size, "MonsterWebHit1.ini", 300,PRIMITIVE_PATH_RECT_LINE);
					if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(billBoard, index*rand()%5))
					{
						delete billBoard;
						billBoard = NULL;
						return FALSE;
					}
					EffectTimeObjectBuffer[timeObjectIndex].Start();
				}
			}

			/*
			//폭탄 파편(파티클)
			for(int index = 0; index < 5; index++)
			{
				HoPrimitiveBillboard *primitiveBillBoard = new HoPrimitiveBillboard();
				if(primitiveBillBoard != NULL)
				{
				
					primitiveBillBoard->StartPath(current, "MonsterWebHit1.ini", ANI_LOOP);
					
					int ang = rand()%ANGLE_360;
					HoPhysicsParticle *physicsParticle = new HoPhysicsParticle();
					POINT3D velocity;
					velocity.x = int((GetCos[ang]/65536.f)*(100.f+rand()%150));
					velocity.y = 250+rand()%200;
					velocity.z = int((GetSin[ang]/65536.f)*(100.f+rand()%150));


					physicsParticle->Start(velocity);
					physicsParticle->SetGravity(float(-rand()%5-8));
					physicsParticle->SetLive(rand()%20+65);
					
					primitiveBillBoard->SetPhysics(physicsParticle, PRIMITIVE_PATH_RECT_LINE);

					

					POINT size;
					size.x = 16;
					size.y = 300;
					primitiveBillBoard->SetSize(size);
					if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(primitiveBillBoard, index/4))
					{
						delete primitiveBillBoard;
						primitiveBillBoard = NULL;
						return FALSE;
					}
					EffectTimeObjectBuffer[timeObjectIndex].Start();
				}
			}
			*/

			HoEffectPat	*pat = new HoEffectPat();
			if(pat != NULL)
			{
				pat->StartAni(current.x, current.y, current.z, 0, 0, 0, PatObj[12]);
				pat->AnimationEnd = PatAnimationEnd[12];
				
				if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(pat, 0))
				{
					delete pat;
					pat = NULL;
					return FALSE;
				}
				EffectTimeObjectBuffer[timeObjectIndex].Start();
			}

			/*
			for(index = 0; index < 20; index++)
			{
				HoPrimitiveBillboard *billBoard = new HoPrimitiveBillboard();
				if(billBoard != NULL)
				{
					int ang = rand()%ANGLE_360;

					POINT3D currentPos;
					POINT3D destPos;
					POINT	size;
					size.x = 8;
					size.y = 100+rand()%100;
					
					float RandomYaw = RANDOM_NUM   * 3.14159f * 2.0f;
					float RandomPitch = RANDOM_NUM * 180.f * 3.14159f / 180.0f ;
					
					//구모양으로 파티클이 날아가게 벡터를 조정한다.
					destPos.y = rand()%100+10000+current.y;
					destPos.x = int(sinf( RandomPitch)*180.f * cosf(RandomYaw)*180.f+current.x);
					destPos.z = int(sinf( RandomPitch)*180.f * sinf(RandomYaw)*180.f+current.z);
					
					currentPos.y = current.y;
					currentPos.x = int(sinf( RandomPitch)*80.f * cosf(RandomYaw)*80.f+current.x);
					currentPos.z = int(sinf( RandomPitch)*80.f * sinf(RandomYaw)*80.f+current.z);
					
					billBoard->StartDestPath(currentPos, destPos, size, "MonsterWebHit2.ini", 700);
					if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(billBoard, index*rand()%20))
					{
						delete billBoard;
						billBoard = NULL;
						return FALSE;
					}
					EffectTimeObjectBuffer[timeObjectIndex].Start();
				}
			}
			*/
			break;
		}
	}

	return TRUE;
}

int HoEffectMgr::StartChrState(smCHAR *chr, int effectType, int liveCount)
{
	switch(effectType)
	{
		case MONSTER_SERQBUS_STATE1:
		case MONSTER_SERQBUS_STATE2:
		case MONSTER_SERQBUS_STATE3:
		case MONSTER_POSION_STATE1:	// 박철호 (독효과)

		{
			int timeObjectIndex = EmptyTimeObjectBufferIndex();
			if(timeObjectIndex == -1)
				return FALSE;

			HoNewEffectChrMove *chrMove = new HoNewEffectChrMove();
			if(chrMove != NULL)
			{
				chrMove->Start(chr, effectType, liveCount);
				if(!EffectTimeObjectBuffer[timeObjectIndex].AddObject(chrMove, 0))
				{
					delete chrMove;
					chrMove = NULL;
					return false;
				}
				EffectTimeObjectBuffer[timeObjectIndex].Start();
			}
			break;
		}
	}

	return TRUE;
}

HoEffectTracker::HoEffectTracker()
{
	StartFlag = FALSE;
	EffectObj = NULL;
	Level = 1;

	liveCount = 0;
	liveFlag = FALSE; 

	sparkCount = 0;
	IMP_SHOT2_liveCount = 0;
	IMP_SHOT3_liveCount = 0;

	WorldX = 0;
	WorldY = 0;
	WorldZ = 0;

	LocalX = 0;
	LocalY = 0;
	LocalZ = 0;

}

HoEffectTracker::~HoEffectTracker()
{

}

int HoEffectTracker::Main()
{
	if(StartFlag == FALSE)
		return FALSE;

	POINT3D currentPos;
	currentPos.x = int(WorldX+LocalX);
	currentPos.y = int(WorldY+LocalY);
	currentPos.z = int(WorldZ+LocalZ);

	//추적 하는 방향의 백터를 구한다.
	float vx = float(DestPos.x-currentPos.x);
	float vy = float(DestPos.y-currentPos.y);
	float vz = float(DestPos.z-currentPos.z);
	
	float length = vx*vx+vy*vy+vz*vz;
	if( (SkillCode == MONSTER_PIGON_SHOT1) ||
		(SkillCode == MONSTER_WEB_SHOT1) ||
		(SkillCode == MONSTER_IMP_SHOT1) || 
		(SkillCode == MONSTER_MEPHIT_SHOT1) ||
		//ktj : 추가
		(SkillCode == MONSTER_MEPHIT_SHOT2) ||
		//ktj : 추가함.
		(SkillCode == MONSTER_IMP_SHOT2) || (SkillCode == MONSTER_IMP_SHOT3)  
		// 박철호 추가
		|| (SkillCode == MONSTER_FORZENMIST_SHOT1) || (SkillCode == MONSTER_FORZENMIST_SHOT2)  
		|| (SkillCode == EFFECT_FIRE_CRACKER )			// 박철호 추가 (폭죽)
		)

	{
		
		if(EffectObj)
		{
			//ktj : 클라스변수로교체함.
			//static int	liveCount = 0;
			//static BOOL	liveFlag = FALSE; 

			//if(length < 2000*2000)
				//liveFlag = TRUE;
				
			if(liveFlag)
				liveCount++;

			if(length < 1000*1000)
			{
				liveFlag = TRUE;
				/*
				//ktj : 원본인데 여기다넣으면 이부분이 실행안되는경우가 많아서 지우고 아래쪽으로 옮김
				if(SkillCode == MONSTER_PIGON_SHOT1)
				{
					EffectObj->SetState(FALSE);
					StartFlag = FALSE;
					StartEffect( currentPos.x, currentPos.y, currentPos.z, EFFECT_FIRE_HIT2 );
					esPlaySound( esSOUND_FIGON_IMPACT , GetDistVolume(currentPos.x, currentPos.y, currentPos.z) );
				}
				*/
				if(SkillCode == MONSTER_WEB_SHOT1)
				{
					EffectObj->SetState(FALSE);
					StartFlag = FALSE;
					StartEffectMonster( currentPos.x, currentPos.y, currentPos.z, MONSTER_WEB_HIT1);
				}
			}

			if(length < 800*800 || liveCount > 3)
			{
				//ktj : 임시
				if(SkillCode == MONSTER_PIGON_SHOT1)
				{
					EffectObj->SetState(FALSE);
					StartFlag = FALSE;
					StartEffect( currentPos.x, currentPos.y, currentPos.z, EFFECT_FIRE_HIT2 );
					esPlaySound( esSOUND_FIGON_IMPACT , GetDistVolume(currentPos.x, currentPos.y, currentPos.z) );
				}
				liveCount = 0;
				liveFlag = FALSE;
				StartFlag = FALSE;
				EffectObj->SetState(FALSE);
				
				if(SkillCode == MONSTER_IMP_SHOT1) 
					StartEffectMonster( currentPos.x, currentPos.y, currentPos.z, MONSTER_IMP_HIT1);
				//ktj : 추가
				if(SkillCode == MONSTER_IMP_SHOT2)
					StartEffectMonster( currentPos.x, currentPos.y, currentPos.z, MONSTER_IMP_HIT2);

				//ktj : 추가
				if(SkillCode == MONSTER_IMP_SHOT3)
					StartEffectMonster( currentPos.x, currentPos.y, currentPos.z, MONSTER_IMP_HIT3);
				


				if(SkillCode == MONSTER_MEPHIT_SHOT1)
				{
					StartEffectMonster( currentPos.x, currentPos.y, currentPos.z, MONSTER_MEPHIT_HIT1);
				}

				if(SkillCode == MONSTER_MEPHIT_SHOT2)
				{
					StartEffectMonster( currentPos.x, currentPos.y, currentPos.z, MONSTER_MEPHIT_HIT2);
				}

				// 박철호 추가
				if(SkillCode == MONSTER_FORZENMIST_SHOT1)
				{
					EffectObj->SetState(FALSE);
					StartFlag = FALSE;
					//g_NewParticleMgr.Start("IceBoltHit1", currentPos);
					StartEffect( currentPos.x, currentPos.y, currentPos.z, EFFECT_ICE_HIT2 );
				}
				if(SkillCode == MONSTER_FORZENMIST_SHOT2)
				{
					EffectObj->SetState(FALSE);
					StartFlag = FALSE;
					//g_NewParticleMgr.Start("IceBallHit1", currentPos);
					StartEffect( currentPos.x, currentPos.y, currentPos.z, EFFECT_ICE_HIT1 );
				}
				if(SkillCode == EFFECT_FIRE_CRACKER )			// 박철호 추가 (폭죽)
				{
					StartEffectMonster( currentPos.x, currentPos.y, currentPos.z, EFFECT_FIRE_CRACKER_HIT);
					StartEffectMonster( currentPos.x, currentPos.y, currentPos.z, EFFECT_FIRE_CRACKER_HIT);
				}
			}					
		}
	}
	
	//ktj : 클라스변수로교체함.
	//static int sparkCount = 0;
	if( (SkillCode == SKILL_SPARK))
	{
		if(length < 1000*1000)
		{
			StartFlag = FALSE;
			StartEffect(currentPos.x, currentPos.y, currentPos.z, EFFECT_SPARK2, Level);

			//전기 소리
			SkillPlaySound( SKILL_SOUND_SKILL_SPARK3 , currentPos.x, currentPos.y, currentPos.z );

			
		}

		if(sparkCount >= 3)
		{
			sparkCount = 0;
			StartChildEffect(currentPos.x, currentPos.y , currentPos.z, EFFECT_SPARK1, this);
		}

		sparkCount++;
	}
	

	//ktj : 추가==============================================================================start
	//ktj : 꼬리에 불붙어 날아가는것을 표현하기위해 넣은것으로 불덩이를 따라서 연기가피어오름
	//ktj : 클라스변수로교체함.
	//static int IMP_SHOT2_liveCount = 0;
	if( SkillCode == MONSTER_IMP_SHOT2  ||
		SkillCode == EFFECT_FIRE_CRACKER )			// 박철호 추가 (폭죽)
	{
		IMP_SHOT2_liveCount++;
		IMP_SHOT2_liveCount &= 0x03;

		if(IMP_SHOT2_liveCount ==0)
		{
			if(SkillCode == MONSTER_IMP_SHOT2)
				StartEffectMonster(currentPos.x, currentPos.y, currentPos.z, MONSTER_PIGON_PARTICLE1);
			else if(SkillCode == EFFECT_FIRE_CRACKER)
				StartEffectMonster(currentPos.x, currentPos.y, currentPos.z, EFFECT_FIRE_CRACKER);
		}
	}

	//ktj : 클라스변수로교체함.
	//static int IMP_SHOT3_liveCount = 0;
	if( SkillCode == MONSTER_IMP_SHOT3 )
	{
		IMP_SHOT3_liveCount++;
		IMP_SHOT3_liveCount &= 0x03;

		if(IMP_SHOT3_liveCount ==0)
			StartEffectMonster(currentPos.x, currentPos.y, currentPos.z, MONSTER_PIGON_PARTICLE1_BLH);
	}
	//ktj : 추가==============================================================================end




	if(EffectObj)
		EffectObj->TranslateWorld(currentPos.x, currentPos.y, currentPos.z);

	if(SkillCode == SKILL_METEO)
	{
		int mapY1, mapY2;
		if(smGameStage[0])
		{
			mapY1 = smGameStage[0]->GetHeight(currentPos.x, currentPos.z);
		}
		if(smGameStage[1])
		{
			mapY2 = smGameStage[1]->GetHeight(currentPos.x, currentPos.z);
		}

		if(smGameStage[0] && smGameStage[1])
			if (mapY1 < mapY2) mapY1 = mapY2;
		if(smGameStage[0] == NULL && smGameStage[1])
			mapY1 = mapY2;
		
		//맵과 총돌을 했는지 검사한다.
		if(mapY1 > currentPos.y+Vy)
		{
			StartFlag = FALSE;
			currentPos.y = mapY1;
			EffectObj->SetState(FALSE);
			StartEffect( currentPos.x, currentPos.y, currentPos.z, EFFECT_FIRE_HIT2);
		}
	}
	WorldX += (int)Vx;
	WorldY += (int)Vy;
	WorldZ += (int)Vz;
	
	return TRUE;
}

/*
int HoEffectTracker::Start(POINT3D currentPos, POINT3D destPos, POINT3D startVelocity, HoEffectObject *effectObj)
{
	StartFlag    = TRUE;
	DestPos    = destPos;
	
	Vx = (float)startVelocity.x;
	Vy = (float)startVelocity.y;
	Vz = (float)startVelocity.z;
	
	//SearchSlowDownFlag = TRUE;
	//TimeCount = 0;
	//TrackerTimeCount = 0;
	EffectObj = effectObj;
	
	return TRUE;
}
*/
int HoEffectTracker::Start(POINT3D currentPos, POINT3D destPos, int skillCode, HoEffectObject *effectObj, int level)
{
	StartFlag = TRUE;
	WorldX = (float)currentPos.x;
	WorldY = (float)currentPos.y;
	WorldZ = (float)currentPos.z;
	
	DestPos.x = destPos.x;
	DestPos.y = destPos.y;
	DestPos.z = destPos.z;

	LocalX = 0.f;
	LocalY = 0.f;
	LocalZ = 0.f;
	
	SkillCode = skillCode;

	Level = 1;
	if(SkillCode == SKILL_METEO)
	{
		Vx = 0.f;
		Vy = 0.f;
		Vz = 0.f;
		
		EffectObj = effectObj;

		POINT3D currentPos;
		currentPos.x = int(WorldX+LocalX);
		currentPos.y = int(WorldY+LocalY);
		currentPos.z = int(WorldZ+LocalZ);
		
		//추적 하는 방향의 백터를 구한다.
		float vx = float(DestPos.x-currentPos.x);
		float vy = float(DestPos.y-currentPos.y);
		float vz = float(DestPos.z-currentPos.z);
		
		float length = (float)sqrt(vx*vx+vz*vz+vy*vy);
		Vx += vx/length*2000;
		Vy += vy/length*2000;
		Vz += vz/length*2000;

	}
	
	//ktj : 추가됨
	//if( (SkillCode == MONSTER_PIGON_SHOT1) || (SkillCode == MONSTER_WEB_SHOT1)
	//	|| (SkillCode == MONSTER_IMP_SHOT1) || (SkillCode == MONSTER_MEPHIT_SHOT1) )
	if( (SkillCode == MONSTER_PIGON_SHOT1) || (SkillCode == MONSTER_WEB_SHOT1)
		|| (SkillCode == MONSTER_IMP_SHOT1) || (SkillCode == MONSTER_MEPHIT_SHOT1) 
		|| (SkillCode == MONSTER_MEPHIT_SHOT2)	//ktj : 추가
		|| (SkillCode == MONSTER_IMP_SHOT2)		//ktj : 추가
		|| (SkillCode == MONSTER_IMP_SHOT3)		//ktj : 추가
		|| (SkillCode == MONSTER_FORZENMIST_SHOT1)	// 박철호 추가
		|| (SkillCode == MONSTER_FORZENMIST_SHOT2)	// 박철호 추가
		|| (SkillCode == EFFECT_FIRE_CRACKER)			// 박철호 추가 (폭죽)
		)
	{
		Vx = 0.f;
		Vy = 0.f;
		Vz = 0.f;
		
		EffectObj = effectObj;

		POINT3D currentPos;
		currentPos.x = int(WorldX+LocalX);
		currentPos.y = int(WorldY+LocalY);
		currentPos.z = int(WorldZ+LocalZ);
		
		//추적 하는 방향의 백터를 구한다.
		float vx = float(DestPos.x-currentPos.x);
		float vy = float(DestPos.y-currentPos.y);
		float vz = float(DestPos.z-currentPos.z);
		
		float length = (float)sqrt(vx*vx+vz*vz+vy*vy);
		
		if(SkillCode == MONSTER_PIGON_SHOT1 || SkillCode == MONSTER_FORZENMIST_SHOT1 || SkillCode == MONSTER_FORZENMIST_SHOT2)
		{
			Vx += vx/length*1500;
			Vy += vy/length*1500;
			Vz += vz/length*1500;
		}
		//ktj : 추가함.
		//else if(SkillCode == MONSTER_IMP_SHOT1 || SkillCode == MONSTER_MEPHIT_SHOT1)
		else if(SkillCode == MONSTER_IMP_SHOT1 || SkillCode == MONSTER_MEPHIT_SHOT1

			|| SkillCode == MONSTER_MEPHIT_SHOT2	//ktj : 추가

			|| SkillCode == MONSTER_IMP_SHOT2		//ktj : 추가
			)

		{
			Vx += vx/length*1300;
			Vy += vy/length*1300;
			Vz += vz/length*1300;
		}
		//ktj : 추가
		else if(SkillCode == MONSTER_IMP_SHOT3)
		{
			
			Vx += vx/length*1000;
			Vy += vy/length*1000;
			Vz += vz/length*1000;
			
		}
		else if( SkillCode == EFFECT_FIRE_CRACKER )			// 박철호 추가 (폭죽)
		{
			Vx += vx/length*400;
			Vy += vy/length*400;
			Vz += vz/length*400;
		}

        else
		{
			Vx += vx/length*800;
			Vy += vy/length*800;
			Vz += vz/length*800;
		}
	}
	
	
	if( SkillCode == SKILL_SPARK)
	{
		Vx = 0.f;
		Vy = 0.f;
		Vz = 0.f;
		
		POINT3D currentPos;
		currentPos.x = int(WorldX+LocalX);
		currentPos.y = int(WorldY+LocalY);
		currentPos.z = int(WorldZ+LocalZ);
		
		//추적 하는 방향의 백터를 구한다.
		float vx = float(DestPos.x-currentPos.x);
		float vy = float(DestPos.y-currentPos.y);
		float vz = float(DestPos.z-currentPos.z);
		
		float length = (float)sqrt(vx*vx+vz*vz+vy*vy);	
		Vx += vx/length*1600;
		Vy += vy/length*1600;
		Vz += vz/length*1600;

		Level = level;
	}

	return TRUE;
}

HoNewEffectChrMove::HoNewEffectChrMove()
{
	StartFlag = FALSE;
	PartEmitterID = -1;
	LiveCount = 70*20;
	MyCharacterFlag = FALSE;
	FlagShow = TRUE;
}

HoNewEffectChrMove::~HoNewEffectChrMove()
{

}



int HoNewEffectChrMove::Start(smCHAR *chr, int skillCode, int liveCount)
{
	if(chr == NULL || StartFlag == TRUE)
		return FALSE;
	
	SkillCode = skillCode;
	PartEmitterID = -1;
	Character = chr;
	StartFlag = TRUE;
	TimeCount = 0;
	
	if(chr == lpCurPlayer)
		MyCharacterFlag = TRUE;

	switch(SkillCode)
	{
		case MONSTER_SERQBUS_STATE1:
		{
			POINT3D pos;
			pos.x = Character->pX;
			pos.y = Character->pY+8000;
			pos.z = Character->pZ;
			
			PartEmitterID = g_NewParticleMgr.Start("SerqbusState1", pos);
			LiveCount = liveCount*70;
			break;
		}

		case MONSTER_SERQBUS_STATE2:
		{
			POINT3D pos;
			pos.x = Character->pX;
			pos.y = Character->pY;
			pos.z = Character->pZ;
			
			PartEmitterID = g_NewParticleMgr.Start("SerqbusState2", pos);
			LiveCount = liveCount*70;
			break;
		}

		case MONSTER_SERQBUS_STATE3:
		{
			POINT3D pos;
			pos.x = Character->pX;
			pos.y = Character->pY+8000;
			pos.z = Character->pZ;
			
			PartEmitterID = g_NewParticleMgr.Start("SerqbusState3", pos);
			LiveCount = liveCount*70;
			break;
		}

		case MONSTER_POSION_STATE1:	// 박철호 (독효과)
		{
			POINT3D pos;
			pos.x = Character->pX;
			pos.y = Character->pY+8000;
			pos.z = Character->pZ;
			
			PartEmitterID = g_NewParticleMgr.Start("ParkPosion", pos);
			LiveCount = liveCount*70;
			break;
		}
		
	}

	return TRUE;
}

int HoNewEffectChrMove::StopEffect()
{
	if(PartEmitterID != -1)
		g_NewParticleMgr.SetStop(PartEmitterID);	
	StartFlag = FALSE;
	
	return TRUE;
}

int HoNewEffectChrMove::Main()
{
	if(StartFlag == FALSE)
		return FALSE;
	if(Character == NULL || PartEmitterID == -1)
		return FALSE;
	
 	switch(SkillCode)
	{
		//캐릭터 포인터가 바뀌기 땜에 포인터를 보정..
		if(MyCharacterFlag)
		{
			if(Character != lpCurPlayer)
				Character = lpCurPlayer;
		}
	
		//화면 출력 여부가 바뀌면 다시 세팅한다.
		if(FlagShow != Character->FlagShow)
		{
			FlagShow = Character->FlagShow;
			if(PartEmitterID != -1)
			g_NewParticleMgr.SetRendering(PartEmitterID, FlagShow);
		}

		case MONSTER_SERQBUS_STATE1:
		{
			if(Character->Pattern == 0)
			{
				if(PartEmitterID != -1)
					g_NewParticleMgr.SetStop(PartEmitterID);	
				StartFlag = FALSE;
				return FALSE;
			}
			D3DVECTOR pos;
			pos.x = float(Character->pX);
			pos.y = float(Character->pY+Character->PatHeight);
			pos.z = float(Character->pZ);
			g_NewParticleMgr.SetAttachPos(PartEmitterID, pos);
			break;
		}

		case MONSTER_SERQBUS_STATE2:
		{
			
			if(Character->Pattern == 0)
			{
				if(PartEmitterID != -1)
				g_NewParticleMgr.SetStop(PartEmitterID);	
				StartFlag = FALSE;
				return FALSE;
			}
			
			D3DVECTOR pos;
			pos.x = float(Character->pX);
			pos.y = float(Character->pY+Character->PatHeight);
			pos.z = float(Character->pZ);
			g_NewParticleMgr.SetAttachPos(PartEmitterID, pos);
			break;
		}

		case MONSTER_SERQBUS_STATE3:
		{
			if(Character->Pattern == 0)
			{
				if(PartEmitterID != -1)
					g_NewParticleMgr.SetStop(PartEmitterID);	
				StartFlag = FALSE;
				return FALSE;
			}
			
			D3DVECTOR pos;
			pos.x = float(Character->pX);
			pos.y = float(Character->pY+Character->PatHeight);
			pos.z = float(Character->pZ);
			
			g_NewParticleMgr.SetAttachPos(PartEmitterID, pos);
			break;
		}

		case MONSTER_POSION_STATE1:	// 박철호 (독효과)
		{
			if(Character->Pattern == 0)
			{
				if(PartEmitterID != -1)
					g_NewParticleMgr.SetStop(PartEmitterID);	
				StartFlag = FALSE;
				return FALSE;
			}
			D3DVECTOR pos;
			pos.x = float(Character->pX);
			pos.y = float(Character->pY+Character->PatHeight);
			pos.z = float(Character->pZ);
			g_NewParticleMgr.SetAttachPos(PartEmitterID, pos);
			break;
		}
	}
	
	if(Character->MotionInfo)
	{
		if(Character->MotionInfo->State == CHRMOTION_STATE_DEAD)
			StopEffect();
	}
	if(LiveCount < TimeCount)
		StopEffect();

 	TimeCount++;
	return TRUE;
}




HoNewEffectMove::HoNewEffectMove()
{
	PatObj = NULL;
	StartFlag = false;
	CurrentFrame = 159;

	WorldX = 0;
	WorldY = 0;
	WorldZ = 0;

	LocalX = 0;
	LocalY = 0;
	LocalZ = 0;

	PartEmitterID = -1;
	PartEmitterIDExt= -1;
	Velocity.x = 0.0f;
	Velocity.y = 0.0f;
	Velocity.z = 0.0f;
	
	ParticleStartFlag = false;
	ObjBip = NULL;
	Length = 0;

	memset(&OldPos, 0, sizeof(OldPos));

	BlendFlag = FALSE;
	BlendCount = 50;
	BlendStep = 0;

	TimeCount = 0;
}

HoNewEffectMove::~HoNewEffectMove()
{

}


int HoNewEffectMove::Start(POINT3D curPos, POINT3D destPos, POINT3D angle, smPAT3D *pat, int skillCode)
{

	if(StartFlag == TRUE)
		return false;
	
	PatObj = pat;
	CurrentFrame = 160;
	StartFlag = true;

	WorldX = (float)curPos.x;
	WorldY = (float)curPos.y;
	WorldZ = (float)curPos.z;

	LocalX = 0;
	LocalY = 0;
	LocalZ = 0;

	DestPos.x = destPos.x;
	DestPos.y = destPos.y;
	DestPos.z = destPos.z;
	
	GetRadian3D( int(WorldX+LocalX), int(WorldY+LocalY), int(WorldZ+LocalZ), DestPos.x, DestPos.y, DestPos.z);
	Angle.y = GeoResult_Y;
	Angle.x = 0;
	Angle.y = (ANGLE_180-Angle.y)&ANGCLIP;
	Angle.z = 0;
	
	SkillCode = skillCode;
	
	if(SkillCode == MONSTER_STYGIANLORD_SHOT1)
	{

		ObjBip = PatObj->GetObjectFromName("Object13");
		
		Velocity.x = 0.0f;
		Velocity.y = 0.0f;
		Velocity.z = 0.0f;
		
		if(ObjBip != NULL)
		{
			smMATRIX *mWorld;
			AnimObjectTree( ObjBip , CurrentFrame, 0, Angle.y, 0);
			mWorld = &ObjBip->mWorld;
			OldPos.x = int(WorldX+LocalX+mWorld->_41);
			OldPos.y = int(WorldY+LocalY+mWorld->_43);
			OldPos.z = int(WorldZ+LocalZ+mWorld->_42);
			
			D3DVECTOR term;
			term.x = float((DestPos.x - OldPos.x)/ fONE);
			term.y = float((DestPos.y - OldPos.y)/ fONE);
			term.z = float((DestPos.z - OldPos.z)/ fONE);
			
			float length = term.x*term.x + term.y*term.y + term.z*term.z;
			Length = (float)sqrt(length);
		}
		BlendStep  = 0;
		BlendCount = 80;

		PatObj->Frame = CurrentFrame;
	}

	return TRUE;
}


int HoNewEffectMove::Start(POINT3D curPos, POINT3D destPos, int skillCode)
{
	if(StartFlag == TRUE)
		return false;
	
	CurrentFrame = 159;
	StartFlag = true;

	WorldX = (float)curPos.x;
	WorldY = (float)curPos.y;
	WorldZ = (float)curPos.z;

	LocalX = 0;
	LocalY = 0;
	LocalZ = 0;

	DestPos.x = destPos.x;
	DestPos.y = destPos.y;
	DestPos.z = destPos.z;
	
	SkillCode = skillCode;
	if( (SkillCode == MONSTER_SERQBUS_SHOT1) ||
	    (SkillCode == MONSTER_SERQBUS_SHOT2) ||
	    (SkillCode == MONSTER_SERQBUS_SHOT3) ||
		 SkillCode == MONSTER_MUMMY_SHOT)	//  머니 통상공격
	{
		POINT3D data;
		data.x = int(WorldX+LocalX);
		data.y = int(WorldY+LocalY);
		data.z = int(WorldZ+LocalZ);
		if(SkillCode == MONSTER_SERQBUS_SHOT1)
			PartEmitterID = g_NewParticleMgr.Start("SerqbusShot1", data);
		else if(SkillCode == MONSTER_SERQBUS_SHOT2)
			PartEmitterID = g_NewParticleMgr.Start("SerqbusShot2", data);
		else if(SkillCode == MONSTER_SERQBUS_SHOT3)
			PartEmitterID = g_NewParticleMgr.Start("SerqbusShot3", data);
		else if(SkillCode == MONSTER_MUMMY_SHOT)
			PartEmitterID = g_NewParticleMgr.Start("ParkMummy", data);

		
		Velocity.x = 0.0f;
		Velocity.y = 0.0f;
		Velocity.z = 0.0f;

		D3DVECTOR term;
		term.x = (DestPos.x - (WorldX+LocalX))/ fONE;
		term.y = (DestPos.y - (WorldY+LocalY))/ fONE;
		term.z = (DestPos.z - (WorldZ+LocalZ))/ fONE;
		
		float length = term.x*term.x + term.y*term.y + term.z*term.z;
		length = (float)sqrt(length);
		Velocity.x += (term.x/(int)length)*9;
		Velocity.y += (term.y/(int)length)*9;
		Velocity.z += (term.z/(int)length)*9;
	}

	if((SkillCode == MONSTER_STYGIANLORD_PARTICLE1) || (SkillCode == MONSTER_COLDEYE_SKILL))
	{
		
		//PartEmitterID = g_NewParticleMgr.Start("StygianlordParticle1", data);
		Velocity.x = 0.0f;
		Velocity.y = 0.0f;
		Velocity.z = 0.0f;

		D3DVECTOR term;
		term.x = (DestPos.x - (WorldX+LocalX))/ fONE;
		term.y = (DestPos.y - (WorldY+LocalY))/ fONE;
		term.z = (DestPos.z - (WorldZ+LocalZ))/ fONE;
		
		float length = term.x*term.x + term.y*term.y + term.z*term.z;
		length = (float)sqrt(length);
		Velocity.x += (term.x/(int)length)*5;
		Velocity.y += (term.y/(int)length)*5;
		Velocity.z += (term.z/(int)length)*5;

		POINT3D data;
		data.x = int(WorldX+LocalX);
		data.y = int(WorldY+LocalY);
		data.z = int(WorldZ+LocalZ);

		
	}

	return true;
}

int HoNewEffectMove::Start(POINT3D curPos, POINT3D destPos, smPAT3D *pat, int skillCode)
{

	if(pat == NULL || StartFlag == TRUE)
		return false;
	
	PatObj = pat;
	CurrentFrame = 159;
	StartFlag = true;

	WorldX = (float)curPos.x;
	WorldY = (float)curPos.y;
	WorldZ = (float)curPos.z;

	LocalX = 0;
	LocalY = 0;
	LocalZ = 0;

	DestPos.x = destPos.x;
	DestPos.y = destPos.y;
	DestPos.z = destPos.z;
	
	SkillCode = skillCode;
	
	if(SkillCode == MONSTER_SHADOW_SHOT1)
	{
		GetRadian3D( int(WorldX+LocalX), int(WorldY+LocalY), int(WorldZ+LocalZ), DestPos.x, DestPos.y, DestPos.z);
		Angle.x = GeoResult_X;
		Angle.y = GeoResult_Y;
		Angle.z = 0;
		Angle.y = (-Angle.y+ANGLE_180)&ANGCLIP;

		ObjBip = PatObj->GetObjectFromName("root");
		
		Velocity.x = 0.0f;
		Velocity.y = 0.0f;
		Velocity.z = 0.0f;
		
		if(ObjBip != NULL)
		{
			smMATRIX *mWorld;
			AnimObjectTree( ObjBip , CurrentFrame, 0, Angle.y, 0);
			mWorld = &ObjBip->mWorld;
			OldPos.x = int(WorldX+LocalX+mWorld->_41);
			OldPos.y = int(WorldY+LocalY+mWorld->_43);
			OldPos.z = int(WorldZ+LocalZ+mWorld->_42);
			
			D3DVECTOR term;
			term.x = float((DestPos.x - OldPos.x)/ fONE);
			term.y = float((DestPos.y - OldPos.y)/ fONE);
			term.z = float((DestPos.z - OldPos.z)/ fONE);
			
			float length = term.x*term.x + term.y*term.y + term.z*term.z;
			Length = (float)sqrt(length);
		}
		BlendStep  = 0;
		BlendCount = 80;

		PatObj->Frame = CurrentFrame;
		
		
		POINT3D data;
		data.x = int(WorldX+LocalX);
		data.y = int(WorldY+LocalY);
		data.z = int(WorldZ+LocalZ);
		PartEmitterID = g_NewParticleMgr.Start("ShadowParticle1", data);
		PartEmitterIDExt = g_NewParticleMgr.Start("ShadowParticle2", data);
	}

	if(SkillCode == MONSTER_DMACHINE_MISSILE1)
	{
		GetRadian3D( int(WorldX+LocalX), int(WorldY+LocalY), int(WorldZ+LocalZ), DestPos.x, DestPos.y, DestPos.z);
		Angle.x = GeoResult_X;
		Angle.y = GeoResult_Y;
		Angle.z = 0;
		Angle.y = (-Angle.y+ANGLE_180)&ANGCLIP;
		
		POINT3D data;
		data.x = int(WorldX+LocalX);
		data.y = int(WorldY+LocalY);
		data.z = int(WorldZ+LocalZ);
		PartEmitterID = g_NewParticleMgr.Start("DMachineParticle1", data);
		
		Velocity.x = 0.0f;
		Velocity.y = 0.0f;
		Velocity.z = 0.0f;

		D3DVECTOR term;
		term.x = (DestPos.x - (WorldX+LocalX))/ fONE;
		term.y = (DestPos.y - (WorldY+LocalY))/ fONE;
		term.z = (DestPos.z - (WorldZ+LocalZ))/ fONE;
		
		float length = term.x*term.x + term.y*term.y + term.z*term.z;
		length = (float)sqrt(length);
		Velocity.x += (term.x/(int)length)*10;
		Velocity.y += (term.y/(int)length)*10;
		Velocity.z += (term.z/(int)length)*10;
	}
	else if(SkillCode == MONSTER_DMACHINE_MISSILE2)
	{
		POINT3D data;
		data.x = int(WorldX+LocalX);
		data.y = int(WorldY+LocalY);
		data.z = int(WorldZ+LocalZ);
		PartEmitterID = g_NewParticleMgr.Start("DMachineParticle1", data);
		
		Velocity.x = 0.0f;
		Velocity.y = 0.0f;
		Velocity.z = 0.0f;
		
		Velocity.x += float(rand()%4-2);
		Velocity.y += 6.0f;
		Velocity.z += float(rand()%4-2);
		
		GetRadian3D( int(WorldX+LocalX), int(WorldY+LocalY), int(WorldZ+LocalZ), int(WorldX+LocalX), int(WorldY+LocalY)+int(Velocity.y), int(WorldZ+LocalZ));
		Angle.x = GeoResult_X;
		Angle.y = GeoResult_Y;
		Angle.z = 0;
		Angle.y = (-Angle.y+ANGLE_180)&ANGCLIP;
	}

	if(SkillCode == MONSTER_FORZENMIST_SHOT1)
	{
		GetRadian3D( int(WorldX+LocalX), int(WorldY+LocalY), int(WorldZ+LocalZ), DestPos.x, DestPos.y, DestPos.z);
		Angle.x = GeoResult_X;
		Angle.y = GeoResult_Y;
		Angle.z = 0;
		Angle.y = (-Angle.y+ANGLE_180)&ANGCLIP;
		
		POINT3D data;
		data.x = int(WorldX+LocalX);
		data.y = int(WorldY+LocalY);
		data.z = int(WorldZ+LocalZ);
		PartEmitterID = g_NewParticleMgr.Start("IceBoltHit1", data);
		
		Velocity.x = 0.0f;
		Velocity.y = 0.0f;
		Velocity.z = 0.0f;

		D3DVECTOR term;
		term.x = (DestPos.x - (WorldX+LocalX))/ fONE;
		term.y = (DestPos.y - (WorldY+LocalY))/ fONE;
		term.z = (DestPos.z - (WorldZ+LocalZ))/ fONE;
		
		float length = term.x*term.x + term.y*term.y + term.z*term.z;
		length = (float)sqrt(length);
		Velocity.x += (term.x/(int)length)*10;
		Velocity.y += (term.y/(int)length)*10;
		Velocity.z += (term.z/(int)length)*10;
	}
	else if(SkillCode == MONSTER_FORZENMIST_SHOT2)
	{
		GetRadian3D( int(WorldX+LocalX), int(WorldY+LocalY), int(WorldZ+LocalZ), DestPos.x, DestPos.y, DestPos.z);
		Angle.x = GeoResult_X;
		Angle.y = GeoResult_Y;
		Angle.z = 0;
		Angle.y = (-Angle.y+ANGLE_180)&ANGCLIP;
		
		POINT3D data;
		data.x = int(WorldX+LocalX);
		data.y = int(WorldY+LocalY);
		data.z = int(WorldZ+LocalZ);
		PartEmitterID = g_NewParticleMgr.Start("IceBallHit1", data);
		
		Velocity.x = 0.0f;
		Velocity.y = 0.0f;
		Velocity.z = 0.0f;

		D3DVECTOR term;
		term.x = (DestPos.x - (WorldX+LocalX))/ fONE;
		term.y = (DestPos.y - (WorldY+LocalY))/ fONE;
		term.z = (DestPos.z - (WorldZ+LocalZ))/ fONE;
		
		float length = term.x*term.x + term.y*term.y + term.z*term.z;
		length = (float)sqrt(length);
		Velocity.x += (term.x/(int)length)*10;
		Velocity.y += (term.y/(int)length)*10;
		Velocity.z += (term.z/(int)length)*10;
	}
	
	return true;
}


int HoNewEffectMove::Main()
{
	
	if(StartFlag == FALSE)
		return FALSE;
	
	if(SkillCode == MONSTER_SHADOW_SHOT1)
	{
		if(CurrentFrame >= 160 * 10)
		{
			CurrentFrame += 160;
			POINT3D pos;
			pos.x = int(WorldX+LocalX);
			pos.y = int(WorldY+LocalY);
			pos.z = int(WorldZ+LocalZ);

			D3DVECTOR term;
			term.x = float((DestPos.x - pos.x)/ fONE);
			term.y = float((DestPos.y - pos.y)/ fONE);
			term.z = float((DestPos.z - pos.z)/ fONE);
			
			float length = term.x*term.x + term.y*term.y + term.z*term.z;
			length = (float)sqrt(length);
			if(length < 10) //충돌을 했을 경우..
			{
				POINT3D data;
				data.x = int(WorldX+LocalX);
				data.y = int(WorldY+LocalY);
				data.z = int(WorldZ+LocalZ);

				
				SetDynLight( DestPos.x, DestPos.y, DestPos.z, 255, 100, 50, 255, 250, 2);
				g_NewParticleMgr.Start("DMachineParticle2", data);
				g_NewParticleMgr.Start("ShadowMagic1", data);

				if(PartEmitterID != -1)
				{
					g_NewParticleMgr.SetStop(PartEmitterID);
				}
				if(PartEmitterIDExt != -1)
				{
					g_NewParticleMgr.SetStop(PartEmitterIDExt);
				}

				StartFlag = false;
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
			
			LocalX += Velocity.x * fONE;
			LocalY += Velocity.y * fONE;
			LocalZ += Velocity.z * fONE;
			
			D3DVECTOR data;
			POINT3D tempPos;
			tempPos.x = int(WorldX+LocalX);
			tempPos.y = int(WorldY+LocalY);
			tempPos.z = int(WorldZ+LocalZ);
			
			GetRadian3D( int(WorldX+LocalX-Velocity.x*fONE), int(WorldY+LocalY-Velocity.y*fONE), int(WorldZ+LocalZ-Velocity.z*fONE), int(WorldX+LocalX), int(WorldY+LocalY), int(WorldZ+LocalZ));
			Angle.x = GeoResult_X&ANGCLIP;
			Angle.y = GeoResult_Y&ANGCLIP;
			Angle.z = 0;
			Angle.y = (-Angle.y+ANGLE_180)&ANGCLIP;
			
			if(PartEmitterID != -1)
			{
				data.x = (float)tempPos.x;
				data.y = (float)tempPos.y;
				data.z = (float)tempPos.z;
				g_NewParticleMgr.SetAttachPos(PartEmitterID, data);
				
				if(PartEmitterIDExt != -1)
				g_NewParticleMgr.SetPos(PartEmitterIDExt, data);
			}

		}
		else
		{
			CurrentFrame += 160;
		}
	}

	if(SkillCode == MONSTER_STYGIANLORD_PARTICLE1)
	{
		if(ParticleStartFlag == FALSE)
		{
			POINT3D data;
			data.x = int(WorldX+LocalX);
			data.y = int(WorldY+LocalY);
			data.z = int(WorldZ+LocalZ);
			PartEmitterID = g_NewParticleMgr.Start("StygianlordParticle1", data);
			ParticleStartFlag = TRUE;
			g_NewParticleMgr.Start("StygianlordParticle3", data);
		}
		

		POINT3D pos;
		pos.x = int(WorldX+LocalX);
		pos.y = int(WorldY+LocalY);
		pos.z = int(WorldZ+LocalZ);

		D3DVECTOR term;
		term.x = float((DestPos.x - pos.x)/ fONE);
		term.y = float((DestPos.y - pos.y)/ fONE);
		term.z = float((DestPos.z - pos.z)/ fONE);
		
		float length = term.x*term.x + term.y*term.y + term.z*term.z;
		length = (float)sqrt(length);
		if(length < 20)
		{
			if(PartEmitterID != -1)
			{
				g_NewParticleMgr.SetStop(PartEmitterID);
			}

			POINT3D pos;
			pos.x = int(WorldX+LocalX);
			pos.y = int(WorldY+LocalY);
			pos.z = int(WorldZ+LocalZ);
			
			StartFlag = false;
		}
		else
		{
			D3DVECTOR velocity;
			velocity.x = term.x/length;
			velocity.y = term.y/length;
			velocity.z = term.z/length;

			Velocity.x += velocity.x;
			Velocity.y += velocity.y;
			Velocity.z += velocity.z;

			if(length < 150)
			{
				Velocity.x *= 0.8f;
				Velocity.y *= 0.8f;
				Velocity.z *= 0.8f;
			}
			
			LocalX += Velocity.x * fONE;
			LocalY += Velocity.y * fONE;
			LocalZ += Velocity.z * fONE;

			D3DVECTOR data;
			data.x = WorldX+LocalX;
			data.y = WorldY+LocalY-fONE;
			data.z = WorldZ+LocalZ;
			g_NewParticleMgr.SetPos(PartEmitterID, data);
		}
	}

	if(SkillCode == MONSTER_STYGIANLORD_SHOT1)
	{
		if(BlendFlag == FALSE)
		{
			POINT3D curPos;
			if(ObjBip != NULL)
			{
				CurrentFrame += 120;
				smMATRIX *mWorld;
				AnimObjectTree( ObjBip , CurrentFrame, 0, Angle.y, 0);
				mWorld = &ObjBip->mWorld;
				curPos.x = int(WorldX+LocalX+mWorld->_41);
				curPos.y = int(WorldY+LocalY+mWorld->_43);
				curPos.z = int(WorldZ+LocalZ+mWorld->_42);
				
				D3DVECTOR term;
				term.x = float((OldPos.x - curPos.x)/ fONE);
				term.y = float((OldPos.y - curPos.y)/ fONE);
				term.z = float((OldPos.z - curPos.z)/ fONE);
				
				float length = term.x*term.x + term.y*term.y + term.z*term.z;
				length = (float)sqrt(length);
				
				float velocityY = (DestPos.y-curPos.y)/(Length-length);
				Velocity.y += velocityY;
				
				if( (Length-length) < 100)
				{
					Velocity.y = Velocity.y*0.75f;
				}
				
				LocalY += Velocity.y;

				if( (Length-length) < 15)
				{
					if(PartEmitterID != -1)
					{
						g_NewParticleMgr.SetStop(PartEmitterID);
						PartEmitterID = -1;
					}
					//StartFlag = FALSE;
					BlendFlag = TRUE;
					POINT3D data;
					data.x = int(curPos.x);
					data.y = int(curPos.y);
					data.z = int(curPos.z);

					g_NewParticleMgr.Start("StygianlordHit1", data);
				}
				
				if(ParticleStartFlag == FALSE)
				{
					POINT3D data;
					data.x = int(curPos.x);
					data.y = int(curPos.y);
					data.z = int(curPos.z);
					PartEmitterID = g_NewParticleMgr.Start("StygianlordParticle2", data);
					ParticleStartFlag = TRUE;
					
				}

				if(PartEmitterID != -1)
				{
					D3DVECTOR data;
					data.x = float(curPos.x);
					data.y = float(curPos.y);
					data.z = float(curPos.z);

					g_NewParticleMgr.SetPos(PartEmitterID, data);
				}
			}
		}
		else
		{
			BlendStep -= 5;
			if(BlendCount <= 0)
			{
				BlendFlag = FALSE;
				StartFlag = FALSE;
			}
			BlendCount--;
		}
	}

	
	if(SkillCode == MONSTER_SERQBUS_SHOT1 ||
	   SkillCode == MONSTER_SERQBUS_SHOT2 ||
	   SkillCode == MONSTER_SERQBUS_SHOT3 ||
	   SkillCode == MONSTER_MUMMY_SHOT)
	{
		POINT3D pos;
		pos.x = int(WorldX+LocalX);
		pos.y = int(WorldY+LocalY);
		pos.z = int(WorldZ+LocalZ);

		D3DVECTOR term;
		term.x = float((DestPos.x - pos.x)/ fONE);
		term.y = float((DestPos.y - pos.y)/ fONE);
		term.z = float((DestPos.z - pos.z)/ fONE);
		
		float length = term.x*term.x + term.y*term.y + term.z*term.z;
		length = (float)sqrt(length);
		

		if(length < 10)
		{
			if(PartEmitterID != -1)
			{
				g_NewParticleMgr.SetStop(PartEmitterID);
			}

			POINT3D pos;
			pos.x = int(WorldX+LocalX);
			pos.y = int(WorldY+LocalY);
			pos.z = int(WorldZ+LocalZ);
			
			if(SkillCode == MONSTER_SERQBUS_SHOT1)
			{
				g_NewParticleMgr.Start("SerqbusHit1", pos);
				SetDynLight( DestPos.x, DestPos.y, DestPos.z, 100, 100, 255, 255, 150, 2);
			}
			else if(SkillCode == MONSTER_SERQBUS_SHOT2)
			{
				g_NewParticleMgr.Start("SerqbusHit2", pos);
				SetDynLight( DestPos.x, DestPos.y, DestPos.z, 255, 50, 50, 255, 150, 2);
			}
			else if(SkillCode == MONSTER_SERQBUS_SHOT3)
			{
				g_NewParticleMgr.Start("SerqbusHit3", pos);
				SetDynLight( DestPos.x, DestPos.y, DestPos.z, 108, 8, 136, 255, 200, 2);
			}
			else if(SkillCode == MONSTER_MUMMY_SHOT)
			{
				g_NewParticleMgr.Start("ParkMummyHit", pos);
				SetDynLight( DestPos.x, DestPos.y, DestPos.z, 200, 44, 21, 255, 150, 2);
			}
			StartFlag = false;
		}
		else
		{
			D3DVECTOR velocity;
			velocity.x = term.x/length;
			velocity.y = term.y/length;
			velocity.z = term.z/length;

			Velocity.x += velocity.x;
			Velocity.y += velocity.y;
			Velocity.z += velocity.z;

			if(length < 100)
			{
				Velocity.x *= 0.75;
				Velocity.y *= 0.75;
				Velocity.z *= 0.75;
			}
			
			LocalX += Velocity.x * fONE;
			LocalY += Velocity.y * fONE;
			LocalZ += Velocity.z * fONE;
			
			D3DVECTOR data;
			data.x = WorldX+LocalX;
			data.y = WorldY+LocalY-fONE;
			data.z = WorldZ+LocalZ;
			
			g_NewParticleMgr.SetPos(PartEmitterID, data);
		}
	}
	
	

	if(SkillCode == MONSTER_DMACHINE_MISSILE1)
	{
		POINT3D pos;
		pos.x = int(WorldX+LocalX);
		pos.y = int(WorldY+LocalY);
		pos.z = int(WorldZ+LocalZ);

		D3DVECTOR term;
		term.x = float((DestPos.x - pos.x)/ fONE);
		term.y = float((DestPos.y - pos.y)/ fONE);
		term.z = float((DestPos.z - pos.z)/ fONE);
		
		float length = term.x*term.x + term.y*term.y + term.z*term.z;
		length = (float)sqrt(length);
		if(length < 10) //충돌을 했을 경우..
		{
			AssaParticle_MonsterExplision(&DestPos);
			if(PartEmitterID != -1)
			{
				g_NewParticleMgr.SetStop(PartEmitterID);
			}
			
			StartFlag = false;
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
		
		LocalX += Velocity.x * fONE;
		LocalY += Velocity.y * fONE;
		LocalZ += Velocity.z * fONE;
		
		D3DVECTOR data;
		POINT3D tempPos;
		tempPos.x = int(WorldX+LocalX);
		tempPos.y = int(WorldY+LocalY);
		tempPos.z = int(WorldZ+LocalZ);
		
		if(PartEmitterID != -1)
		{
			data.x = float(tempPos.x-(Velocity.x*fONE*4));
			data.y = float(tempPos.y-(Velocity.y*fONE*4));
			data.z = float(tempPos.z-(Velocity.z*fONE*4));
			g_NewParticleMgr.SetPos(PartEmitterID, data);
		}

		GetRadian3D( int(WorldX+LocalX-Velocity.x*fONE), int(WorldY+LocalY-Velocity.y*fONE), int(WorldZ+LocalZ-Velocity.z*fONE), int(WorldX+LocalX), int(WorldY+LocalY), int(WorldZ+LocalZ));
		Angle.x = GeoResult_X&ANGCLIP;
		Angle.y = GeoResult_Y&ANGCLIP;
		Angle.z = 0;
		Angle.y = (-Angle.y+ANGLE_180)&ANGCLIP;
	}

	if(SkillCode == MONSTER_DMACHINE_MISSILE2)
	{
		POINT3D pos;
		pos.x = int(WorldX+LocalX);
		pos.y = int(WorldY+LocalY);
		pos.z = int(WorldZ+LocalZ);

		D3DVECTOR term;
		term.x = float((DestPos.x - pos.x)/ fONE);
		term.y = float((DestPos.y - pos.y)/ fONE);
		term.z = float((DestPos.z - pos.z)/ fONE);
		
		float length = term.x*term.x + term.y*term.y + term.z*term.z;
		length = (float)sqrt(length);
		if(length < 15) //충돌을 했을 경우..
		{
			//POINT3D data;
			//data.x = int(WorldX+LocalX);
			//data.y = int(WorldY+LocalY);
			//data.z = int(WorldZ+LocalZ);

			//g_NewParticleMgr.Start("DMachineParticle2", data);
			//SetDynLight( DestPos.x, DestPos.y, DestPos.z, 255, 100, 50, 255, 200, 2);
			AssaParticle_BombHit1(&DestPos);
			
			if(PartEmitterID != -1)
			{
				g_NewParticleMgr.SetStop(PartEmitterID);
			}
			
			StartFlag = false;
		}
		
		D3DVECTOR velocity;
		velocity.x = term.x/length;
		velocity.y = term.y/length;
		velocity.z = term.z/length;
		
		Velocity.x += velocity.x;
		Velocity.y += velocity.y;
		Velocity.z += velocity.z;
		
		float velLength = Velocity.x*Velocity.x+Velocity.y*Velocity.y+Velocity.z*Velocity.z;
		velLength = (float)sqrt(velLength);
		if(velLength > 9)
		{
			Velocity.x = Velocity.x * 0.8f;
			Velocity.y = Velocity.y * 0.8f;
			Velocity.z = Velocity.z * 0.8f;
		}
		if(length < 84)
		{
			Velocity.x = Velocity.x * 0.75f;
			Velocity.y = Velocity.y * 0.75f;
			Velocity.z = Velocity.z * 0.75f;
		}
		
		LocalX += Velocity.x * fONE;
		LocalY += Velocity.y * fONE;
		LocalZ += Velocity.z * fONE;
		
		D3DVECTOR data;
		POINT3D tempPos;
		tempPos.x = int(WorldX+LocalX);
		tempPos.y = int(WorldY+LocalY);
		tempPos.z = int(WorldZ+LocalZ);
		
		if(PartEmitterID != -1)
		{
			data.x = float(tempPos.x-(Velocity.x*fONE*3));
			data.y = float(tempPos.y-(Velocity.y*fONE*3));
			data.z = float(tempPos.z-(Velocity.z*fONE*3));
			g_NewParticleMgr.SetPos(PartEmitterID, data);
		}

		GetRadian3D( int(WorldX+LocalX-Velocity.x*fONE), int(WorldY+LocalY-Velocity.y*fONE), int(WorldZ+LocalZ-Velocity.z*fONE), int(WorldX+LocalX), int(WorldY+LocalY), int(WorldZ+LocalZ));
		Angle.x = GeoResult_X&ANGCLIP;
		Angle.y = GeoResult_Y&ANGCLIP;
		Angle.z = 0;
		Angle.y = (-Angle.y+ANGLE_180)&ANGCLIP;
	}
	
	if(SkillCode == MONSTER_COLDEYE_SKILL)
	{
		if(ParticleStartFlag == FALSE)
		{
			POINT3D data;
			data.x = int(WorldX+LocalX);
			data.y = int(WorldY+LocalY);
			data.z = int(WorldZ+LocalZ);
			PartEmitterID = g_NewParticleMgr.Start("IceColdEyeSkill1", data);
			ParticleStartFlag = TRUE;
			g_NewParticleMgr.Start("StygianlordParticle3", data);
		}
		

		POINT3D pos;
		pos.x = int(WorldX+LocalX);
		pos.y = int(WorldY+LocalY);
		pos.z = int(WorldZ+LocalZ);

		D3DVECTOR term;
		term.x = float((DestPos.x - pos.x)/ fONE);
		term.y = float((DestPos.y - pos.y)/ fONE);
		term.z = float((DestPos.z - pos.z)/ fONE);
		
		float length = term.x*term.x + term.y*term.y + term.z*term.z;
		length = (float)sqrt(length);
		if(length < 20)
		{
			if(PartEmitterID != -1)
			{
				g_NewParticleMgr.SetStop(PartEmitterID);
			}

			POINT3D pos;
			pos.x = int(WorldX+LocalX);
			pos.y = int(WorldY+LocalY);
			pos.z = int(WorldZ+LocalZ);
			
			StartFlag = false;
		}
		else
		{
			D3DVECTOR velocity;
			velocity.x = term.x/length;
			velocity.y = term.y/length;
			velocity.z = term.z/length;

			Velocity.x += velocity.x;
			Velocity.y += velocity.y;
			Velocity.z += velocity.z;

			if(length < 150)
			{
				Velocity.x *= 0.8f;
				Velocity.y *= 0.8f;
				Velocity.z *= 0.8f;
			}
			
			LocalX += Velocity.x * fONE;
			LocalY += Velocity.y * fONE;
			LocalZ += Velocity.z * fONE;

			D3DVECTOR data;
			data.x = WorldX+LocalX;
			data.y = WorldY+LocalY-fONE;
			data.z = WorldZ+LocalZ;
			g_NewParticleMgr.SetPos(PartEmitterID, data);
		}
	}
	
	TimeCount++;
	if(TimeCount > 70*20)
	{
		if(PartEmitterID != -1)
		{
			g_NewParticleMgr.SetStop(PartEmitterID);
			PartEmitterID = -1;
		}
		StartFlag = FALSE;
	}
	
	return true;
}



int HoNewEffectMove::Draw(int x, int y, int z, int ax, int ay, int az)
{
	
	if(StartFlag == false)
		return false;
	
	if(PatObj != NULL)
	{
		
		POINT3D pos;
		pos.x = int(WorldX+LocalX);
		pos.y = int(WorldY+LocalY);
		pos.z = int(WorldZ+LocalZ);

		smRender.Init();

		PatObj->Frame = CurrentFrame;
		PatObj->SetPosi(&pos, &Angle);
		lpD3DDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, FALSE);
		POINT3D cameraPos;
		POINT3D cameraAngle;
		
		cameraPos.x = x;
		cameraPos.y = y;
		cameraPos.z = z;

		cameraAngle.x = ax;
		cameraAngle.y = ay;
		cameraAngle.z = az;
		
		smRender.Color_R = BlendStep;
		smRender.Color_G = BlendStep;
		smRender.Color_B = BlendStep;
		smRender.Color_A = BlendStep;

		PatObj->RenderD3D(&cameraPos, &cameraAngle);
		lpD3DDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, TRUE);
		smRender.Color_R = 0;
		smRender.Color_G = 0;
		smRender.Color_B = 0;
		smRender.Color_A = 0;
	}
	
	return true;
}










int StartTerrainEffect(int x, int y, int z, int effectType)
{
	//EnterCriticalSection(&cLoadSection);
	if(effectType == EFFECT_TERRAIN_WAVE)
	{
		POINT3D pos;
		pos.x = x;
		pos.y = y;
		pos.z = z;
		
		//물결 모양 그리기..
		g_NewParticleMgr.Start("Water", pos);
	}
	//LeaveCriticalSection(&cLoadSection);

	return TRUE;
}

//HitEffect의 월드에서의 좌표...
int StartEffect(int x, int y, int z, int sizeX, int sizeY,int effectType)
{
	//EnterCriticalSection(&cLoadSection);
	EffectMgr.Start(x, y, z, sizeX, sizeY, effectType);
	//LeaveCriticalSection(&cLoadSection);

	return TRUE;
}

int StartEffect(int x, int y, int z, int effectType, int Level)
{
	//EnterCriticalSection(&cLoadSection);
	EffectMgr.Start(x, y, z, effectType, Level);
	//LeaveCriticalSection(&cLoadSection);

	return TRUE;
}

int StartEffect(int x, int y, int z, int r, int g, int b, int a, int type)
{
	//EnterCriticalSection(&cLoadSection);
	EffectMgr.Start(x, y, z, r, g, b, a, type);
	//LeaveCriticalSection(&cLoadSection);

	return TRUE;
}

int StartEffect(int x, int y, int z, char *iniName)
{
	//EnterCriticalSection(&cLoadSection);
	EffectMgr.StartBillRectPrimitive(x, y, z, iniName);
	//LeaveCriticalSection(&cLoadSection);

	return TRUE;
}

int StartEffect(int x, int y, int z, int effectType, smCHAR *character)
{
	//EnterCriticalSection(&cLoadSection);
	EffectMgr.Start(x, y, z, effectType, character);
	//LeaveCriticalSection(&cLoadSection);
	
	return TRUE;
}

int StartChildEffect(int x, int y, int z, int effectType, HoEffectObject *parent)
{
	//EnterCriticalSection(&cLoadSection);
	EffectMgr.Start(x, y, z, effectType, parent);
	//LeaveCriticalSection(&cLoadSection);
	
	return TRUE;
}

int StartSkill(int x, int y, int z, int angleX, int angleY, int angleZ, int skillType, int level)
{
	//EnterCriticalSection(&cLoadSection);
	EffectMgr.StartSkill(x, y, z, angleX, angleY, angleZ,  skillType, level);
	//LeaveCriticalSection(&cLoadSection);


	return TRUE;
}

int StartSkill(int x, int y, int z, int angleX, int angleY, int angleZ, smCHAR *character, int skillType, int level)
{
	if(character == NULL)
		return FALSE;
	
	EffectMgr.StartSkill(x, y, z, angleX, angleY, angleZ, character, skillType, level);
	return TRUE;
}


int StartSkillDest(int x, int y, int z, int destX, int destY, int destZ, int skillType, int level)
{
	EffectMgr.StartSkillDest(x, y, z, destX, destY, destZ, skillType, level);
	
	return TRUE;
}

int StartEffectMonsterDest(int x, int y, int z, int destX, int destY, int destZ, int angleX, int angleY, int angleZ, int effectType)
{
	
	POINT3D current;
	POINT3D dest;
	POINT3D angle;

	current.x = x;
	current.y = y;
	current.z = z;

	dest.x = destX;
	dest.y = destY;
	dest.z = destZ;
	
	angle.x = angleX;
	angle.y = angleY;
	angle.z = angleZ;

	EffectMgr.StartMonsterDest(current, dest, angle, effectType);
	
	return TRUE;
}

int StartEffectMonsterDest(int x, int y, int z, int destX, int destY, int destZ, int effectType)
{
	
	POINT3D current;
	POINT3D dest;
	
	current.x = x;
	current.y = y;
	current.z = z;

	dest.x = destX;
	dest.y = destY;
	dest.z = destZ;

	EffectMgr.StartMonsterDest(current, dest, effectType);
	return TRUE;
}

int StartEffectMonster(int x, int y, int z, int effectType)
{
	POINT3D pos;
	pos.x = x;
	pos.y = y;
	pos.z = z;
	switch (effectType)
	{
		case MONSTER_DMACHINE_PARTICLE1:
			g_NewParticleMgr.Start("DMachineParticle1", pos);
			break;
		
		case MONSTER_HULK_HIT1:
		{
			SetDynLight( pos.x, pos.y, pos.z, 100, 255, 100, 255, 100, 1);
			int i = rand()%3;
			if(i == 0)
				g_NewParticleMgr.Start("HulkHit1", pos);
			else if(i == 1)
				g_NewParticleMgr.Start("HulkHit2", pos);
			else if(i == 2)
				g_NewParticleMgr.Start("HulkHit3", pos);
			break;
		}

		case MONSTER_VALENTO_HIT1:
			g_NewParticleMgr.Start("IceValentoHit1", pos);
			break;

		case MONSTER_VALENTO_HIT2:
			g_NewParticleMgr.Start("IceValentoHit2", pos);
			break;

		default:
			if(EffectMgr.StartMonster(pos, effectType)== FALSE)
			break;
	}
	return TRUE;
}

int StartEffectMonster(int x, int y, int z, int angleX, int angleY, int angleZ, int effectType)
{
	POINT3D pos;
	
	pos.x = x;
	pos.y = y;
	pos.z = z;
	
	POINT3D angle;
	angle.x = angleX;
	angle.y = angleY;
	angle.z = angleZ;

	if(effectType == MONSTER_FURY_MAGIC1)
	{
		AssaParticle_Fury_Skill1(&pos, &angle);
		return TRUE;
	}
	
	if(effectType == MONSTER_HEADER_CUTTER_HIT1)
		g_NewParticleMgr.Start("HeadCutterHit1", pos);
	
	if(effectType == MONSTER_OMICRON_HIT1)
		g_NewParticleMgr.Start("OmiCronHit1", pos);
	//else if(effectType == SKILL_DIASTROPHISM)
		//g_NewParticleMgr.Start("Skill3MagicianDiastrophism1", pos);
	
	EffectMgr.StartMonster(pos, angle, effectType);
	
	return TRUE;
}

int StartEffectChrState(smCHAR *chr, int effectType, int liveCount)
{
	
	EffectMgr.StartChrState(chr, effectType, liveCount);
	return TRUE;
}

int StopEffect(int index)
{
	return TRUE;
}








HoEtc2dPrimitive	MenuFlame;
int DrawMenuFlame()
{
	if(smConfig.TextureQuality == 0 ||
	   smConfig.TextureQuality == 1)
	{
		MenuFlame.Draw();
	}

	return TRUE;
}

int StartMenuFlame(int x, int y)
{
	MenuFlame.StartUp(x, y);
	return TRUE;
}




HoEffectMgr			EffectMgr;
HoPhysicsMgr		PhysicsMgr;


int InitEffect()
{
	AnimDataMgr.Init();
	

	AnimDataMgr.ImageLoad("Effect\\ImageData\\Hit1.ini");
	AnimDataMgr.ImageLoad("Effect\\ImageData\\Hit21.ini");
	AnimDataMgr.ImageLoad("Effect\\ImageData\\Hit22.ini");
	AnimDataMgr.ImageLoad("Effect\\ImageData\\LevelUp1Left.ini");
	AnimDataMgr.ImageLoad("Effect\\ImageData\\LevelUp1Right.ini");
	AnimDataMgr.ImageLoad("Effect\\ImageData\\LevelUpParticle1.ini");
	AnimDataMgr.ImageLoad("Effect\\ImageData\\Potion1.ini");
	AnimDataMgr.ImageLoad("Effect\\ImageData\\Potion2.ini");
	AnimDataMgr.ImageLoad("Effect\\ImageData\\Potion3.ini");
	AnimDataMgr.ImageLoad("Effect\\ImageData\\Blood1.ini");
	AnimDataMgr.ImageLoad("Effect\\ImageData\\Gas1.ini");
	AnimDataMgr.ImageLoad("Effect\\ImageData\\dust1.ini");
	AnimDataMgr.ImageLoad("Effect\\ImageData\\Light1.ini");
	AnimDataMgr.ImageLoad("Effect\\ImageData\\Bandi1.ini");
	AnimDataMgr.ImageLoad("Effect\\ImageData\\Pung1.ini");
	AnimDataMgr.ImageLoad("Effect\\ImageData\\Tail1.ini");
	AnimDataMgr.ImageLoad("Effect\\ImageData\\LevelUp.ini");
	AnimDataMgr.ImageLoad("Effect\\ImageData\\SkillExtremeShieldBig.ini");
	AnimDataMgr.ImageLoad("Effect\\ImageData\\SkillExtremeShieldSmall.ini");
	AnimDataMgr.ImageLoad("Effect\\ImageData\\SkillExtremeShieldParticle.ini");
	AnimDataMgr.ImageLoad("Effect\\ImageData\\Meteo1.ini");
	AnimDataMgr.ImageLoad("Effect\\ImageData\\ParticleMeteo1.ini");
	AnimDataMgr.ImageLoad("Effect\\ImageData\\SkillUp1.ini");
	AnimDataMgr.ImageLoad("Effect\\ImageData\\Return1.ini");
	AnimDataMgr.ImageLoad("Effect\\ImageData\\ParticleMeteo2.ini");
	AnimDataMgr.ImageLoad("Effect\\ImageData\\PhysicalAbsorbParticle1.ini");
	AnimDataMgr.ImageLoad("Effect\\ImageData\\ScoutHawkGate.ini");
	AnimDataMgr.ImageLoad("Effect\\ImageData\\ScoutHawkLight.ini");
	AnimDataMgr.ImageLoad("Effect\\ImageData\\AutomationLeft.ini");
	AnimDataMgr.ImageLoad("Effect\\ImageData\\AutomationRight.ini");
	AnimDataMgr.ImageLoad("Effect\\ImageData\\MaximizeLeft.ini");
	AnimDataMgr.ImageLoad("Effect\\ImageData\\MaximizeRight.ini");
	AnimDataMgr.ImageLoad("Effect\\ImageData\\SkillRageofZecram1.ini");
	AnimDataMgr.ImageLoad("Effect\\ImageData\\ReturnParticle1.ini");
//	AnimDataMgr.ImageLoad("Effect\\ImageData\\SkillTripleImpact2.ini");
	AnimDataMgr.ImageLoad("Effect\\ImageData\\SkillMechanicBombDust1.ini");
	AnimDataMgr.ImageLoad("Effect\\ImageData\\SkillPerfectAim1.ini");
	AnimDataMgr.ImageLoad("Effect\\ImageData\\SkillPerfectAim2.ini");
	AnimDataMgr.ImageLoad("Effect\\ImageData\\SkillPhysicalAbsorb1.ini");
	AnimDataMgr.ImageLoad("Effect\\ImageData\\PhysicalAbsorbParticle2.ini");
	AnimDataMgr.ImageLoad("Effect\\ImageData\\Shield1.ini");
	AnimDataMgr.ImageLoad("Effect\\ImageData\\Shield2.ini");
	AnimDataMgr.ImageLoad("Effect\\ImageData\\ShieldParticle1.ini");
	AnimDataMgr.ImageLoad("Effect\\ImageData\\FireParticle1.ini");
	
	AnimDataMgr.ImageLoad("Effect\\ImageData\\SkillSpark1.ini");
	AnimDataMgr.ImageLoad("Effect\\ImageData\\SkillSpark2.ini");
	//AnimDataMgr.ImageLoad("Effect\\ImageData\\SkillSpark3.ini");

	AnimDataMgr.ImageLoad("Effect\\ImageData\\SkillRoarParticle1.ini");
	AnimDataMgr.ImageLoad("Effect\\ImageData\\SkillRoarLineParticle1.ini");
	AnimDataMgr.ImageLoad("Effect\\ImageData\\Power1.ini");
	
	AnimDataMgr.ImageLoad("Effect\\ImageData\\MonsterWeb1.ini");
	AnimDataMgr.ImageLoad("Effect\\ImageData\\MonsterWebHit1.ini");
	AnimDataMgr.ImageLoad("Effect\\ImageData\\MonsterWebHit2.ini");
	AnimDataMgr.ImageLoad("Effect\\ImageData\\MonsterImp1.ini");
	AnimDataMgr.ImageLoad("Effect\\ImageData\\MonsterMephit1.ini");
	AnimDataMgr.ImageLoad("Effect\\ImageData\\MonsterMephit2.ini");
	
	AnimDataMgr.ImageLoad("Effect\\ImageData\\SkillTripleImpactParticle.ini");
	AnimDataMgr.ImageLoad("Effect\\ImageData\\SkillTripleImpactHit.ini");

	AnimDataMgr.ImageLoad("Effect\\ImageData\\SkillRoundImpactParticle.ini");
	AnimDataMgr.ImageLoad("Effect\\ImageData\\SkillRoundImpactHit.ini");

	AnimDataMgr.ImageLoad("Effect\\ImageData\\SkillTornado.ini");
	
	AnimDataMgr.ImageLoad("Effect\\ImageData\\SkillZecram1.ini");
	AnimDataMgr.ImageLoad("Effect\\ImageData\\Blur1.ini");
	AnimDataMgr.ImageLoad("Effect\\ImageData\\ContinueStun.ini");
	AnimDataMgr.ImageLoad("Effect\\ImageData\\DamageParticle1.ini");


	//ktj : 새로 넣음.
	AnimDataMgr.ImageLoad("Effect\\ImageData\\SkillTornado2.ini");
	AnimDataMgr.ImageLoad("Effect\\ImageData\\ParticleMeteo3.ini");
	AnimDataMgr.ImageLoad("Effect\\ImageData\\Power2.ini");
	AnimDataMgr.ImageLoad("Effect\\ImageData\\FireParticle2.ini");
	AnimDataMgr.ImageLoad("Effect\\ImageData\\blackhole.ini");
	AnimDataMgr.ImageLoad("Effect\\ImageData\\FireParticle1_blh.ini");
	AnimDataMgr.ImageLoad("Effect\\ImageData\\ParticleMeteo1_blue.ini");	//블랙홀에쓰는 파티클

	#ifdef Making_SKILL_TORNADO3
	AnimDataMgr.ImageLoad("Effect\\ImageData\\SkillTornado3.ini");	//임시번개용
	#endif
	





	AnimDataMgr.SequenceLoad("Effect\\AnimationData\\NormalHit1.ini");
	AnimDataMgr.SequenceLoad("Effect\\AnimationData\\CriticalHit1.ini");
	AnimDataMgr.SequenceLoad("Effect\\AnimationData\\CriticalHit21.ini");
	AnimDataMgr.SequenceLoad("Effect\\AnimationData\\CriticalHit22.ini");
	AnimDataMgr.SequenceLoad("Effect\\AnimationData\\LevelUp1Left.ini");
	AnimDataMgr.SequenceLoad("Effect\\AnimationData\\LevelUp1Right.ini");
	AnimDataMgr.SequenceLoad("Effect\\AnimationData\\LevelUpParticle1.ini");
	AnimDataMgr.SequenceLoad("Effect\\AnimationData\\Potion1.ini");
	AnimDataMgr.SequenceLoad("Effect\\AnimationData\\Potion2.ini");
	AnimDataMgr.SequenceLoad("Effect\\AnimationData\\Potion3.ini");
	AnimDataMgr.SequenceLoad("Effect\\AnimationData\\Blood1.ini");
	AnimDataMgr.SequenceLoad("Effect\\AnimationData\\Gas1.ini");
	AnimDataMgr.SequenceLoad("Effect\\AnimationData\\dust1.ini");
	AnimDataMgr.SequenceLoad("Effect\\AnimationData\\Light1.ini");
	AnimDataMgr.SequenceLoad("Effect\\AnimationData\\Bandi1.ini");
	AnimDataMgr.SequenceLoad("Effect\\AnimationData\\PotionLight1.ini");
	AnimDataMgr.SequenceLoad("Effect\\AnimationData\\LevelUpLight1.ini");
	AnimDataMgr.SequenceLoad("Effect\\AnimationData\\Pung1.ini");
	AnimDataMgr.SequenceLoad("Effect\\AnimationData\\Tail1.ini");
	AnimDataMgr.SequenceLoad("Effect\\AnimationData\\Tail2.ini");
	AnimDataMgr.SequenceLoad("Effect\\AnimationData\\LevelUp.ini");
	AnimDataMgr.SequenceLoad("Effect\\AnimationData\\SkillExtremeShieldBig.ini");
	AnimDataMgr.SequenceLoad("Effect\\AnimationData\\SkillExtremeShieldSmall.ini");
	AnimDataMgr.SequenceLoad("Effect\\AnimationData\\SkillExtremeShieldParticle.ini");
	AnimDataMgr.SequenceLoad("Effect\\AnimationData\\Meteo1.ini");
	AnimDataMgr.SequenceLoad("Effect\\AnimationData\\ParticleMeteo1.ini");
	AnimDataMgr.SequenceLoad("Effect\\AnimationData\\SkillUp1.ini");
	AnimDataMgr.SequenceLoad("Effect\\AnimationData\\Return1.ini");
	AnimDataMgr.SequenceLoad("Effect\\AnimationData\\ParticleMeteo2.ini");
	AnimDataMgr.SequenceLoad("Effect\\AnimationData\\PhysicalAbsorbParticle1.ini");
	AnimDataMgr.SequenceLoad("Effect\\AnimationData\\ScoutHawkGate.ini");

	AnimDataMgr.SequenceLoad("Effect\\AnimationData\\AutomationLeft.ini");
	AnimDataMgr.SequenceLoad("Effect\\AnimationData\\AutomationRight.ini");
	AnimDataMgr.SequenceLoad("Effect\\AnimationData\\MaximizeLeft.ini");
	AnimDataMgr.SequenceLoad("Effect\\AnimationData\\MaximizeRight.ini");
	AnimDataMgr.SequenceLoad("Effect\\AnimationData\\SkillRageofZecram1.ini");
	AnimDataMgr.SequenceLoad("Effect\\AnimationData\\ReturnParticle1.ini");
//	AnimDataMgr.SequenceLoad("Effect\\AnimationData\\SkillTripleImpact2.ini");
	AnimDataMgr.SequenceLoad("Effect\\AnimationData\\SkillMechanicBombDust1.ini");
	AnimDataMgr.SequenceLoad("Effect\\AnimationData\\SkillPerfectAim1.ini");
	AnimDataMgr.SequenceLoad("Effect\\AnimationData\\SkillPerfectAim2.ini");
	AnimDataMgr.SequenceLoad("Effect\\AnimationData\\SkillPhysicalAbsorb1.ini");
	AnimDataMgr.SequenceLoad("Effect\\AnimationData\\PhysicalAbsorbParticle2.ini");
	AnimDataMgr.SequenceLoad("Effect\\AnimationData\\SkillUp2.ini");
	AnimDataMgr.SequenceLoad("Effect\\AnimationData\\ScoutHawkLight.ini");
	AnimDataMgr.SequenceLoad("Effect\\AnimationData\\Shield1.ini");
	AnimDataMgr.SequenceLoad("Effect\\AnimationData\\Shield2.ini");
	AnimDataMgr.SequenceLoad("Effect\\AnimationData\\ShieldParticle1.ini");
	AnimDataMgr.SequenceLoad("Effect\\AnimationData\\MechanicBomb1.ini");
	AnimDataMgr.SequenceLoad("Effect\\AnimationData\\MechanicBomb2.ini");
	AnimDataMgr.SequenceLoad("Effect\\AnimationData\\FireParticle1.ini");
	AnimDataMgr.SequenceLoad("Effect\\AnimationData\\SkillSpark1.ini");
	AnimDataMgr.SequenceLoad("Effect\\AnimationData\\SkillSpark2.ini");
	AnimDataMgr.SequenceLoad("Effect\\AnimationData\\SkillSpark3.ini");
	//AnimDataMgr.SequenceLoad("Effect\\AnimationData\\SkillSpark4.ini");
	
	AnimDataMgr.SequenceLoad("Effect\\AnimationData\\SkillRoarParticle1.ini");
	AnimDataMgr.SequenceLoad("Effect\\AnimationData\\SkillRoarLineParticle1.ini");
	AnimDataMgr.SequenceLoad("Effect\\AnimationData\\Power1.ini");
	AnimDataMgr.SequenceLoad("Effect\\AnimationData\\MonsterWeb1.ini");
	AnimDataMgr.SequenceLoad("Effect\\AnimationData\\MonsterWebHit1.ini");
	AnimDataMgr.SequenceLoad("Effect\\AnimationData\\MonsterWebHit2.ini");
	AnimDataMgr.SequenceLoad("Effect\\AnimationData\\MonsterImp1.ini");
	AnimDataMgr.SequenceLoad("Effect\\AnimationData\\MonsterMephit1.ini");
	AnimDataMgr.SequenceLoad("Effect\\AnimationData\\MonsterMephit2.ini");

	

	AnimDataMgr.SequenceLoad("Effect\\AnimationData\\SkillTripleImpactParticle.ini");
	AnimDataMgr.SequenceLoad("Effect\\AnimationData\\SkillTripleImpactHit.ini");

	AnimDataMgr.SequenceLoad("Effect\\AnimationData\\SkillRoundImpactParticle.ini");
	AnimDataMgr.SequenceLoad("Effect\\AnimationData\\SkillRoundImpactHit.ini");
	AnimDataMgr.SequenceLoad("Effect\\AnimationData\\SkillTornado.ini");

	AnimDataMgr.SequenceLoad("Effect\\AnimationData\\SkillZecram1.ini");
	AnimDataMgr.SequenceLoad("Effect\\AnimationData\\Blur1.ini");
	AnimDataMgr.SequenceLoad("Effect\\AnimationData\\SkillArrowOfRage1.ini");
	
	AnimDataMgr.SequenceLoad("Effect\\AnimationData\\ContinueStun.ini");
	AnimDataMgr.SequenceLoad("Effect\\AnimationData\\DamageParticle1.ini");


	//ktj : 새로 넣음.
	AnimDataMgr.SequenceLoad("Effect\\AnimationData\\SkillTornado2.ini");
	AnimDataMgr.SequenceLoad("Effect\\AnimationData\\ParticleMeteo3.ini");
	AnimDataMgr.SequenceLoad("Effect\\AnimationData\\Power2.ini");
	AnimDataMgr.SequenceLoad("Effect\\AnimationData\\FireParticle2.ini");
	AnimDataMgr.SequenceLoad("Effect\\AnimationData\\blackhole.ini");			//블랙홀터질때
	AnimDataMgr.SequenceLoad("Effect\\AnimationData\\FireParticle1_blh.ini");	//블랙홀의꼬리에다는 검은파티클
	AnimDataMgr.SequenceLoad("Effect\\AnimationData\\ParticleMeteo1_blue.ini");	//블랙홀터질때튀어나오는파티클(파란색임)

	#ifdef Making_SKILL_TORNADO3
	AnimDataMgr.SequenceLoad("Effect\\AnimationData\\SkillTornado3.ini");	//임시번개용 SKILL_TORNADO3
	#endif
	
	if(smConfig.TextureQuality == 0 ||
	   smConfig.TextureQuality == 1)
	{
		MenuFlame.Init();
		MenuFlame.Load();
	}

	EffectMgr.Init();
	PhysicsMgr.Init();
	InterfaceParty.Init();
	EffectHitBoard.Load();
	
	
	g_NewParticleResMgr.init();
	g_NewParticleMgr.Clear();
	
	//파티클 데이타 읽는 부분..
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\Script\\water.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\Script\\NewCharacter.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\Script\\FuryHit1.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\Script\\Fire.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\Script\\HeadCutterHit1.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\Script\\leafDown.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\Script\\HulkHit1.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\Script\\HulkHit2.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\Script\\HulkHit3.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\Script\\OmiCronHit1.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\Script\\DMachineParticle1.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\Script\\DMachineParticle2.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\Script\\SerqbusShot1.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\Script\\SerqbusShot2.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\Script\\SerqbusShot3.part");

	g_NewParticleResMgr.LoadScript("Effect\\Particle\\Script\\SerqbusHit1.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\Script\\SerqbusHit2.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\Script\\SerqbusHit3.part");

	g_NewParticleResMgr.LoadScript("Effect\\Particle\\Script\\StygianlordHit1.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\Script\\StygianlordParticle1.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\Script\\StygianlordParticle2.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\Script\\StygianlordParticle3.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\Script\\ShadowParticle1.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\Script\\ShadowParticle2.part");

	g_NewParticleResMgr.LoadScript("Effect\\Particle\\Script\\ShadowMagic1.part");

	g_NewParticleResMgr.LoadScript("Effect\\Particle\\Script\\SerqbusMagic1.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\Script\\SerqbusMagic2.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\Script\\SerqbusMagic3.part");
	
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\Script\\SerqbusMagicAttack1.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\Script\\SerqbusMagicAttack2.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\Script\\SerqbusMagicAttack3.part");

	g_NewParticleResMgr.LoadScript("Effect\\Particle\\Script\\SerqbusState1.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\Script\\SerqbusState2.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\Script\\SerqbusState3.part");
	
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\Script\\Aging.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\Script\\SoulSuckerHit1.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\Script\\DeadLayStart1.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\Script\\DeadLayHit1.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\Script\\DivineLightning.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\Script\\ColumnOfWater1.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\Script\\ColumnOfWater2.part");
	
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\Script\\EnchantWeaponJang.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\Script\\EnchantWeaponMember.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\Script\\EnchantWeaponFire1.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\Script\\EnchantWeaponFire2.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\Script\\EnchantWeaponFire3.part");
	
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\Script\\EnchantWeaponIce.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\Script\\EnchantWeaponLight.part");
	
	
    
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\Script\\HolyValor.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\Script\\DrasticSpirit.part");
	
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\Script\\TwistJavelin1.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\Script\\TwistJavelin2.part");

	g_NewParticleResMgr.LoadScript("Effect\\Particle\\Script\\FireJavelinFire.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\Script\\LightHit1.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\Script\\LightStart1.part");
	
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\Script\\IceHit1.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\Script\\FireHit1.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\Script\\FireHIt2.part");

	g_NewParticleResMgr.LoadScript("Effect\\Particle\\Script\\FireBigHIt1.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\Script\\Windy1.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\Script\\Windy2.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\Script\\Piercing1.part");
    g_NewParticleResMgr.LoadScript("Effect\\Particle\\Script\\IceShot1.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\Script\\FireShot1.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\Script\\FireBigShot1.part");
	

	g_NewParticleResMgr.LoadScript("Effect\\Particle\\Script\\FuryEye.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\Script\\FuryRespawn.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\Script\\FuryRespawnMouse.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\Script\\FuryRespawnDown.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\Script\\FuryHit2.part");
    g_NewParticleResMgr.LoadScript("Effect\\Particle\\Script\\StoneGolemHit1.part");
	
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\Script\\HeavyGoblin.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\Script\\HeavyGoblinHit1.part");

	g_NewParticleResMgr.LoadScript("Effect\\Particle\\Script\\MonsterHit1.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\Script\\MonsterHit2.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\Script\\DoomGuard.part");
    g_NewParticleResMgr.LoadScript("Effect\\Particle\\Script\\DoomGuardBig.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\Script\\DoomGuardBigHit.part");


	g_NewParticleResMgr.LoadScript("Effect\\Particle\\Script\\CaveMummy.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\Script\\ThornCrawler.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\Script\\Snail.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\Script\\Ratoo.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\Script\\RatooHit1.part");
	
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\Script\\EvilMaple.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\Script\\EvilPlant.part");
    g_NewParticleResMgr.LoadScript("Effect\\Particle\\Script\\Opening.part");

	g_NewParticleResMgr.LoadScript("Effect\\Particle\\Script\\ClanMonster1.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\Script\\ClanMonster2.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\Script\\ClanMonster3.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\Script\\ClanMonster4.part");

	
	//공성 관련
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\Script\\FuryHit1_b.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\Script\\FuryHit2_b.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\Script\\FuryHit1_g.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\Script\\FuryHit2_g.part");
	



	
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\Script\\Skill2FighterRoar1.part");
	//Aging용
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\Script\\AgingBody4.part");

	//3차 스킬용
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\Script\\Skill3Hit1.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\Script\\Skill3Hit2.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\Script\\Skill3Hit3.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\Script\\Skill3Hit4.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\Script\\Skill3Hit5.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\Script\\Skill3Hit6.part");
	
	

	//파이터
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\script\\Skill3FighterConcentration1.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\script\\Skill3FighterSwiftAxe1.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\script\\Skill3FighterBoneSmash1.part");


	
	
	
	//파이크맨
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\script\\Skill3PikeManVenomSpear.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\script\\Skill3PikeManVenomSpear1.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\script\\Skill3PikeManBladeOfBlaze.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\script\\Skill3PikeManBladeOfBlazeHit.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\script\\Skill3PikeVanish1.part");
	

	
	//아처
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\script\\Skill3ArcherBombShotHit1.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\script\\Skill3ArcherBombShotHit2.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\script\\Skill3ArcherBombShot1.part");
	
	
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\script\\Skill3ArcherGoldenFalcon1.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\script\\Skill3ArcherGoldenFalcon2.part");


	//메카니션
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\script\\Skill3MechanicianMetalArmor.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\script\\Skill3MechanicianSparkShield.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\script\\Skill3MechanicianSparkShield1.part");
	
	
	
	//메지션
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\script\\Skill3MagicianEngrgyShield.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\script\\Skill3MagicianEngrgyShield1.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\script\\Skill3MagicianSpiritElemental.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\script\\Skill3MagicianDiastrophism1.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\script\\Skill3MagicianDancingSwordFire1.part");

	//g_NewParticleResMgr.LoadScript("Effect\\Particle\\Script\\IceAceintSkill2.part");
	
	
	//g_NewParticleResMgr.LoadScript("Effect\\Particle\\script\\Skill3MagicianDancingSwordFire2.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\script\\Skill3MagicianDancingSwordIce1.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\script\\Skill3MagicianSpiritElementalExt1.part");
	
	
	//나이트
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\script\\Skill3KnightHolyIncantation1.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\script\\Skill3KnightHolyIncantation2.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\script\\Skill3KnightHolyIncantationHand1.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\script\\Skill3KnightHolyIncantationHand2.part");
	
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\script\\Skill3KnightFlameBrandish.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\script\\Skill3KnightFlameBrandish1.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\script\\Skill3KnightDivineInhalation1.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\script\\Skill3KnightDivineInhalation2.part");

	

	//프리스티스
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\script\\Skill3PriestessResurrection1.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\script\\Skill3PriestessResurrection2.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\script\\Skill3PriestessResurrection3.part");

	g_NewParticleResMgr.LoadScript("Effect\\Particle\\script\\Skill3PriestessVigorBall1.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\script\\Skill3PriestessVigorBall2.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\script\\Skill3PriestessVigorBallHit1.part");
	

	g_NewParticleResMgr.LoadScript("Effect\\Particle\\script\\Skill3PriestessExtinction1.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\script\\Skill3PriestessExtinction2.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\script\\Skill3PriestessVirtualLifeMember.part");


	//아탈란타
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\script\\Skill3AtalantaTriumphOfValhalla.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\script\\Skill3AtalantaSplitJavelin1.part");
	

	//Monster 용
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\script\\MonsterPigonFire1.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\script\\MonsterIron1.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\script\\IronMonsterChainGolem1.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\script\\IronMonsterChainGolem2.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\script\\IronMonsterRunicGuardian.part");
    g_NewParticleResMgr.LoadScript("Effect\\Particle\\script\\IronMonsterRunicGuardianLight1.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\script\\IronMonsterRunicGuardianLight2.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\script\\IronMonsterRunicGuardianSkillHit1.part");
	
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\script\\IronMonsterMountainHit1.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\script\\IronMonsterOmuHit1.part");
	

	g_NewParticleResMgr.LoadScript("Effect\\Particle\\script\\IronMonsterBombHit1.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\script\\IronMonsterBombHit2.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\script\\IronMonsterBombHit3.part");
	

	g_NewParticleResMgr.LoadScript("Effect\\Particle\\script\\IronMonsterHyperDead.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\script\\IronMonsterDeadZoneDead.part");

	g_NewParticleResMgr.LoadScript("Effect\\Particle\\script\\IronMonsterRunicGuardianHand.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\script\\IronMonsterRunicGuardianShot1.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\script\\IronMonsterRunicGuardianBossWeapon1.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\script\\IronMonsterRunicGuardianBossWeapon2.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\script\\IronMonsterRunicGuardianBossHit1.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\script\\IronMonsterRunicGuardianBossSkill1.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\script\\IronMonsterRunicGuardianBossJin1.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\script\\IronMonsterRunicGuardianBossJin2.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\script\\IronMonsterRunicGuardianLight3.part");
	

	g_NewParticleResMgr.LoadScript("Effect\\Particle\\script\\Shelltom.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\script\\ShelltomHit1.part");
    g_NewParticleResMgr.LoadScript("Effect\\Particle\\script\\ShelltomUp1.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\script\\ShelltomUp2.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\script\\ShellTomWeaponHit.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\script\\IronMonsterSadnessShot.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\script\\IronMonsterMorgon.part");
	
	//ClassUp용 칼에 붙는 파티클
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\script\\ClassUpWeapon1.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\script\\ClassUpWeapon2.part");
	
	//지형 이펙트
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\script\\snow.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\script\\TerrainFire.part");
    g_NewParticleResMgr.LoadScript("Effect\\Particle\\script\\Bam.part");

	g_NewParticleResMgr.LoadScript("Effect\\Particle\\script\\MusPell.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\script\\MusPellHit1.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\script\\MusPellHand.part");

	g_NewParticleResMgr.LoadScript("Effect\\Particle\\script\\GodlyShield.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\script\\magneticlight.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\script\\magneticShot.part");
	
	//4차스킬
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\script\\Skill4Betserker.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\script\\Skill4FrostJavelin.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\script\\Skill4ImpulsionLight.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\script\\Skill4ImpulsionHit1.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\script\\Skill4CelestialChainLightingLight.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\script\\Skill4PhoenixShot.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\script\\FireElementalHit.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\script\\MonsterPigonFireTail.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\script\\Skill4RegenerationField.part");
	
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\script\\MuspellStart.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\script\\MuspellEnd.part");


	// 박철호 (아이스 몬스터 관련)
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\Script\\IceColdEyeStart1.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\Script\\IceColdEyeHit1.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\Script\\IceColdEyeSkill1.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\Script\\IceMisticSpinelSkill1.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\Script\\IceAceintSkill2.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\Script\\IceGolemHit1.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\Script\\IceFrozenMistBody1.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\Script\\IceBoltHit1.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\Script\\IceBallHit1.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\Script\\IceGoblinHit1.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\Script\\IceGoblin.part");

	// 발렌토이펙트
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\Script\\IceValentoPiercing.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\Script\\IceValentoBrandish.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\Script\\IceValentoJoint.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\Script\\IceValentoBody.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\Script\\IceValentoLEye.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\Script\\IceValentoREye.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\Script\\IceValentoHit1.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\Script\\IceValentoHit2.part");



	// 카오스 카라 이펙트
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\Script\\ChaosKaraNormal1_1.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\Script\\ChaosKaraNormal1_2.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\Script\\ChaosKaraMeteo.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\Script\\ChaosKaraMeteoHit.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\Script\\ChaosKaraStaff.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\Script\\ChaosKaraSkill.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\Script\\ChaosKaraSkillUser.part");

	// 스파이더 트루퍼 이펙트
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\Script\\ParkSpiderTrooper.part");
	
	// 스팅레이 이펙트
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\Script\\ParkStingRay.part");
	// 스팅레이에 당하다.
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\Script\\ParkPosion.part");

	// 머미로드 쏘다.
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\Script\\ParkMummy.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\Script\\ParkMummyHit.part");

	// 데스 버드
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\Script\\DevilBirdNormal.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\Script\\DevilBirdSkillHit.part");

	// 폭죽
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\Script\\FireCrackerNormal.part");	// 노말 폭죽
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\Script\\FireCrackerHeart.part");	// 하트 폭죽
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\Script\\FireCrackerMoon.part");	// 달 폭죽
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\Script\\FireCrackerStar.part");	// 별 폭죽
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\Script\\FireCrackerAll.part");	// 하트+달+별 폭죽

	g_NewParticleResMgr.LoadScript("Effect\\Particle\\Script\\FireCrackerBombHeart.part");	// 하트 터짐
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\Script\\FireCrackerBombMoon.part");	// 달 터짐
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\Script\\FireCrackerBombStar.part");	// 별 터짐

	// 켈베쥬 이펙트
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\Script\\Firesnow.part");	// 켈베쥬 던젼에 불똥 이펙트
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\Script\\IceKelvezuBossJin1.part");	// 켈베쥬 마법진
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\Script\\IceKelvezuBossMouseJin1.part");	// 켈베쥬 입에서의 연기
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\Script\\IceKelvezuHit.part");	// 켈베쥬가 캐릭터 공격시
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\Script\\IceKelvezuNormal1.part");	// 공격동작	1
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\Script\\IceKelvezuNormal2.part");	// 공격동작 2
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\Script\\IceKelvezuNormal3.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\Script\\IceKelvezuSkill1.part");	// 스킬동작 1
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\Script\\IceKelvezuSkill2.part");	// 스킬동작 2

	// 헬하운드 이펙트
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\Script\\LostMonsterHellHoundAttack.part");	// 헬하운드
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\Script\\LostMonsterDarkMageHit1.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\Script\\LostMonsterDarkMageDead.part");

	g_NewParticleResMgr.LoadScript("Effect\\Particle\\Script\\TerrainFireSmall.part");	// 메테오 작은거

	// Lord Hopy
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\Script\\LordHopyMouth.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\Script\\LordHopyBreath.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\Script\\LordHopyBreathHit.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\Script\\LordHopySlam.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\Script\\LordHopyMark.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\Script\\LordHopyCrystal.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\Script\\LordHopyIllusion.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\Script\\LordHopyCharge.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\Script\\LordHopyBlast.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\Script\\LordHopyAura2.part");
	g_NewParticleResMgr.LoadScript("Effect\\Particle\\Script\\LordHopyAura3.part");
	
	
#ifdef _EFFECT_RES_LOADING_
	if(HoEffectResLoadingFlag)
	{
		g_NewParticleResMgr.SearchingAddTexture("Effect\\Particle\\*.*");
	}
#endif


CreateSky();
	//SetSky(0);

    //g_CreatePetSystem();
	return TRUE;
}



int DrawEffect(int x, int y, int z, int ax, int ay, int az)
{
 	if(smConfig.DebugMode)
	{
		static int shellTomCount = 0;

		//하늘 다시 읽어들이는
		if(VRKeyBuff[VK_NUMPAD7])
		{
			VRKeyBuff[VK_NUMPAD7] = FALSE;
			ReCreateSky();
			//ColorInfoFlag = TRUE;
		}
		
		//하늘 바뀌는
		if(VRKeyBuff[VK_NUMPAD8])
		{
			SetNextSky();
			VRKeyBuff[VK_NUMPAD8] = FALSE;
		}
		

		if(VRKeyBuff[VK_NUMPAD1])
		{
			AssaParticle_CastleRed(lpCurPlayer);
			VRKeyBuff[VK_NUMPAD1] = FALSE;
		}
  
		if(VRKeyBuff[VK_NUMPAD2])
		{
			AssaParticle_CastleBlue(lpCurPlayer);
			VRKeyBuff[VK_NUMPAD2] = FALSE;
		}
		
		if(VRKeyBuff[VK_NUMPAD3])
		{
			AssaParticle_CastleGreen(lpCurPlayer);
			VRKeyBuff[VK_NUMPAD3] = FALSE;
		}

		static int count = 0;
		if(VRKeyBuff[VK_NUMPAD1])
		{
			count++;
  			if(28 < count)
				count = 0;
			VRKeyBuff[VK_NUMPAD1] = FALSE;
		}

		//하늘 눈오는
		if(VRKeyBuff[VK_NUMPAD2])
		{
			POINT3D curPos;
   			curPos.x = lpCurPlayer->pX;
 			curPos.y = lpCurPlayer->pY;
 			curPos.z = lpCurPlayer->pZ;
			
			if(count == 0)
			{
				smCHAR *MonsterCharacter[6];
				int count = 0;
				int NearDist = 300*300;

				
 				for( int cnt=0;cnt<OTHER_PLAYER_MAX;cnt++) 
				{
					if ( chrOtherPlayer[cnt].Flag && chrOtherPlayer[cnt].smCharInfo.State==smCHAR_STATE_ENEMY ) 
					{
						if (chrOtherPlayer[cnt].smCharInfo.Brood!=smCHAR_MONSTER_USER && chrOtherPlayer[cnt].smCharInfo.Life[0]>0 ) 
						{
							MonsterCharacter[count] = &chrOtherPlayer[cnt];
							count++;
							if(count >= 6)
								break;
						}
					}
					
				}
 				if(count > 2)
				{
					POINT3D curPos;
					curPos.x = lpCurPlayer->pX;
					curPos.y = lpCurPlayer->pY + 10000;
					curPos.z = lpCurPlayer->pZ;

					SkillImpulsionLight(lpCurPlayer, 10.f);           //칼에 붙는 이펙트
					SkillImpulsionLightHit(&curPos);                     //타격 이펙트
					SkillMetalionImpulsionHit(MonsterCharacter, count); //타격되는 몬스터들 연결되는 이펙트
				}
				//SkillMetalionImpulsionHit(MonsterCharacter, count);
			}
			else if(count == 1)
				SkillMetalionCompulsion(lpCurPlayer);
			else if(count == 2)
				SkillMetalionSphere(lpCurPlayer, 10.f, 5.f);
			else if(count == 3)
				SkillMetalionGolem(lpCurPlayer);
			else if(count == 4)
				SkillWarriorDestroyerBlade(lpCurPlayer, 2.f);
			else if(count == 5)
			{
				GetMoveLocation( 0, 0, 30*fONE, 0, lpCurPlayer->Angle.y, 0);
				POINT3D pos;
				pos.x = lpCurPlayer->pX+GeoResult_X;
				pos.y = lpCurPlayer->pY+5000;
				pos.z = lpCurPlayer->pZ+GeoResult_Z;

				SkillWarriorDestroyerHit(&pos);
			}
			else if(count == 6)
			{
				
				SkillWarriorBetserker(lpCurPlayer, 10.f);

			}
			else if(count == 7)
			{
				SkillWarriorCycloneStrikeBlade(lpCurPlayer, 10.f);
			}
			else if(count == 8)
				SkillLancelotAssassinEye(lpCurPlayer, 10.f);
			else if(count == 9)
			{
				SkillLancelotChargingStrike(lpCurPlayer);
			}
			else if(count == 10)
			{
				POINT3D curPos;
				POINT3D desPos;
				curPos.x = lpCurPlayer->pX;
				curPos.y = lpCurPlayer->pY + 10000;
				curPos.z = lpCurPlayer->pZ;

				desPos.x = curPos.x;
				desPos.y = curPos.y;
				desPos.z = curPos.z+50000;

				//SkillSagittarionPhoneixShot(&curPos, &desPos);
			}
			else if(count == 11)
				SkillSagittarionForceOfNature1(lpCurPlayer, 20.f);
			else if(count == 12)
			{
				GetMoveLocation( 0, 0, 30*fONE, 0, lpCurPlayer->Angle.y, 0);
				POINT3D pos;
				pos.x = lpCurPlayer->pX+GeoResult_X;
				pos.y = lpCurPlayer->pY+5000;
				pos.z = lpCurPlayer->pZ+GeoResult_Z;

				SkillSaintDivinePiercing(lpCurPlayer);
			}
			else if(count == 13)
				SkillSaintGodlyShield(lpCurPlayer, 20.f);
			else if(count == 14)
				SkillSaintGodBless(lpCurPlayer, 20.f);
			else if(count == 15)
				SkillSwordOfUstice(lpCurPlayer);
			else if(count == 16)
				SkillValhallaHallOfValhalla1(lpCurPlayer);
			else if(count == 17)
			{
 				GetMoveLocation( 0, 0, 150*fONE, 0, lpCurPlayer->Angle.y, 0);
				POINT3D curPos;
				POINT3D desPos;
				curPos.x = lpCurPlayer->pX;
				curPos.y = lpCurPlayer->pY+40000;
				curPos.z = lpCurPlayer->pZ;
				
				desPos.x = lpCurPlayer->pX+GeoResult_X;
				desPos.y = lpCurPlayer->pY;
				desPos.z = lpCurPlayer->pZ+GeoResult_Z;

				SkillValhallaXRage(&curPos, &desPos);
			}
			else if(count == 20)
			{
 				GetMoveLocation( 0, 0, 200*fONE, 0, lpCurPlayer->Angle.y, 0);
				POINT3D curPos;
				POINT3D desPos;
				curPos.x = lpCurPlayer->pX;
				curPos.y = lpCurPlayer->pY+10000;
				curPos.z = lpCurPlayer->pZ;
				
				desPos.x = lpCurPlayer->pX+GeoResult_X;
				desPos.y = lpCurPlayer->pY+10000;
				desPos.z = lpCurPlayer->pZ+GeoResult_Z;

				SkillFrostJavelin(&curPos, &desPos);
				
				SkillForstJavelinAttach(lpCurPlayer, 20.f);
 			}
			else if(count == 21)
			{
				GetMoveLocation( 0, 0, 200*fONE, 0, lpCurPlayer->Angle.y, 0);
				POINT3D curPos;
				POINT3D desPos;
				curPos.x = lpCurPlayer->pX;
				curPos.y = lpCurPlayer->pY+10000;
				curPos.z = lpCurPlayer->pZ;
				
				desPos.x = lpCurPlayer->pX+GeoResult_X;
				desPos.y = lpCurPlayer->pY+10000;
				desPos.z = lpCurPlayer->pZ+GeoResult_Z;
				SkillVengeance(&curPos, &desPos);
			}
			else if(count == 22)
			{
				SkillCelestialGlacialSpike(lpCurPlayer);
			}
			else if(count == 23)
			{
				SkillCelestialRegnerationField(lpCurPlayer, 20);
			}
			else if(count == 24)
			{
				smCHAR *MonsterCharacter[6];
				int count = 0;
				int NearDist = 300*300;

				
 				for( int cnt=0;cnt<OTHER_PLAYER_MAX;cnt++) 
				{
					if ( chrOtherPlayer[cnt].Flag && chrOtherPlayer[cnt].smCharInfo.State==smCHAR_STATE_ENEMY ) 
					{
						if (chrOtherPlayer[cnt].smCharInfo.Brood!=smCHAR_MONSTER_USER && chrOtherPlayer[cnt].smCharInfo.Life[0]>0 ) 
						{
							MonsterCharacter[count] = &chrOtherPlayer[cnt];
							count++;
							if(count >= 6)
								break;
						}
					}
					
				}
 				if(count > 2)
					SkillCelestialChainLighting(MonsterCharacter, count);
					//SkillMetalionImpulsionHit(MonsterCharacter, 2);
			}
			else if(count == 25)
			{
				SkillArchMageFlameWave(lpCurPlayer);
			}
			else if(count == 26)
			{
				POINT3D desPos;
				desPos.x = lpCurPlayer->pX;
				desPos.y = lpCurPlayer->pY;
				desPos.z = lpCurPlayer->pZ + 50000;
				SkillArchMageMeteo(&desPos);
			}
			else if(count == 27)
			{
				SkillSaintGodlyShield(lpCurPlayer, 10.f);
			}
			else if(count == 28)
				SkillValhallaHallOfValhallaHandEffect(lpCurPlayer, 20.f);


			VRKeyBuff[VK_NUMPAD2] = FALSE;
		}
		
		

		if(VRKeyBuff[VK_NUMPAD3])
		{
			//g_LowLevelAddPet(lpCurPlayer, HoLowLevelPet::Type::Mute);
			smCHAR *MonsterCharacter[6];
			int count = 0;
			int NearDist = 300*300;

			
 			for( int cnt=0;cnt<OTHER_PLAYER_MAX;cnt++) 
			{
				if ( chrOtherPlayer[cnt].Flag && chrOtherPlayer[cnt].smCharInfo.State==smCHAR_STATE_ENEMY ) 
				{
					if (chrOtherPlayer[cnt].smCharInfo.Brood!=smCHAR_MONSTER_USER && chrOtherPlayer[cnt].smCharInfo.Life[0]>0 ) 
					{
						

						MonsterCharacter[count] = &chrOtherPlayer[cnt];
						count++;
						if(count>=4)
							break;
					}
				}
				
			}
			
			MonsterCharacter[count] = lpCurPlayer;
			count++;
			AssaParticle_MonsterRunicGuardianBossHit( lpCurPlayer, lpCurPlayer, MonsterCharacter, count);
			VRKeyBuff[VK_NUMPAD3] = FALSE;
		}
	}
	
	HoEffectManager::GetInstance()->UpdateView(x, y, z, ax, ay, az);
	//MainWindow.Update(x, y, z, ax, ay, az);
	EffectMgr.Draw(x, y, z, ax, ay, az);
	g_NewParticleMgr.Draw(x, y, z, ax, ay, az);
	g_DrawPetSystem(x, y, z, ax, ay, az);

//박철호 : 2005-11-14 오후 8:20:37
	g_PCBang_DrawPetSystem(x, y, z, ax, ay, az);

	return TRUE;
}

int MainEffect()
{
	HoEffectManager::GetInstance()->Main(1.f/70.f);
	//상열씨 Main
	cSin3D.Main();
	EffectMgr.Main();
	PhysicsMgr.Main();
	MenuFlame.Main();
	
	float timeDelta = 1.f/70.f;
	{
		extern void T5Fx_Main();		// character.cpp: tier 5 effects that follow a character
		T5Fx_Main();
	}
	g_NewParticleMgr.Main(0.0f, timeDelta);
	MainSky();
	g_MainPetSystem();

//박철호 : 2005-11-14 오후 8:19:42
	g_PCBang_MainPetSystem();

	//MainWindow.Main(1.f/70.f);
	return TRUE;
}
		