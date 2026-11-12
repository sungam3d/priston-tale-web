#include "HoEffectManager.h"
#include "HoEffectResMgr.h"
#include "HoEffectView.h"
#include "HoNewEffectFunction.h"
#include "..\\..\\..\\netplay.h"
#include "..\\..\\HoMinMax.h"
#include "..\\..\\HoNewParticle.h"
#include "..\\..\\HoNewParticleMgr.h"
#include "..\\..\\HoAssaParticleEffect.h"
#include "..\\..\\..\\field.h"
#include "..\\..\\..\\SinBaram\\SinLinkHeader.h"
extern smCHAR *lpCurPlayer;
void HoEffectType_Default::Init(HoEffectGroup *effectGroup, int x, int y, int z, int angleX, int angleY, int angleZ)
{
   	point3 pos;
	pos.x = (float)x/256.f;
	pos.y = (float)y/256.f;
	pos.z = (float)z/256.f;
	point3 angle;
 	angle.x = (float)angleX;
    angle.y = (float)(angleY&ANGCLIP); //(float)((-angleY+ANGLE_180)&ANGCLIP);
   	//angle.y = (float)(angleY+ANGLE_180);
	
	angle.z = (float)angleZ;

	m_pEffectGroup = effectGroup;
	m_pEffectGroup->SetCalcTranslate(&pos, &angle);
}

HoEffectType_Default::~HoEffectType_Default()
{
	if(m_pEffectGroup)
	{
		delete m_pEffectGroup;
		m_pEffectGroup = NULL;
	}
}

void HoEffectType_Default::UpdateView(int x, int y, int z, int ax, int ay, int az)
{
	if(m_pEffectGroup)
		m_pEffectGroup->UpdateView(x, y, z, ax, ay, az);
}

void HoEffectType_Default::Main(float elapsedTime)
{
	if(m_pEffectGroup)
	{
		m_pEffectGroup->Main(elapsedTime);
		if(!m_pEffectGroup->IsLive())
			m_bLiveFlag = false;
	}
}


void HoEffectType_Attach_Avater::Init(smCHAR *pChar, HoEffectGroup *effectGroup, float endTime, float height)
{
	m_pAttachChar  = pChar;
	m_pEffectGroup = effectGroup;
	m_fHeight      = height/256.f;
	m_bLiveFlag    = true;
	m_fEndTime     = endTime;

	point3 pos;
	pos.x = (float)m_pAttachChar->pX/256.f;
	pos.y = (float)m_pAttachChar->pY/256.f + m_fHeight;
	pos.z = (float)m_pAttachChar->pZ/256.f;

	point3 angle;
 	angle.x = (float)pChar->Angle.x;
	angle.y = (float)pChar->Angle.y;
	//angle.y = (float)((-pChar->Angle.y+ANGLE_180)&ANGCLIP);
	angle.z = (float)pChar->Angle.z;
	
	m_pEffectGroup->SetCalcTranslate(&pos, &angle);
	if(pChar == lpCurPlayer)
		m_bMyCharacterFlag = true;
}

HoEffectType_Attach_Avater::~HoEffectType_Attach_Avater()
{
	if(m_pEffectGroup)
	{
		delete m_pEffectGroup;
		m_pEffectGroup = NULL;
	}
}

void HoEffectType_Attach_Avater::UpdateView(int x, int y, int z, int ax, int ay, int az)
{
	if(m_pAttachChar == NULL || m_pAttachChar->FlagShow == 0)
		return;

	if(m_pEffectGroup)
		m_pEffectGroup->UpdateView(x, y, z, ax, ay, az);
}

void HoEffectType_Attach_Avater::Main(float elapsedTime)
{
 	if(m_pAttachChar)
	{
		if(m_bMyCharacterFlag)
		{
			if(m_pAttachChar != lpCurPlayer)
				m_pAttachChar = lpCurPlayer;
		}

 		point3 pos;
		pos.x = (float)m_pAttachChar->pX/256.f;
		pos.y = (float)m_pAttachChar->pY/256.f + m_fHeight;
		pos.z = (float)m_pAttachChar->pZ/256.f;

		m_pEffectGroup->SetCalcTranslate(&pos, NULL);
		m_fElapsedTime += elapsedTime;
		if(m_iParticleID != -1)
		{
			D3DVECTOR partPos;
			partPos.x = (float)m_pAttachChar->pX;
			partPos.y = (float)m_pAttachChar->pY + m_fHeight;
			partPos.z = (float)m_pAttachChar->pZ;
			g_NewParticleMgr.SetAttachPos(m_iParticleID, partPos);
			if(m_bParticleDeadFlag)
			{
				if(!g_NewParticleMgr.GetRunning(m_iParticleID))
					m_iParticleID = -1;
			}

			if(m_fElapsedTime >= m_fEndTime - 0.3f && m_bParticleDeadFlag == false)
			{
				g_NewParticleMgr.SetFastStop(m_iParticleID);
				m_bParticleDeadFlag = true;
				//m_iParticleID = -1;
			}
		}

		if(m_pAttachChar->Flag == 0 || !m_pAttachChar->MotionInfo || m_fElapsedTime >= m_fEndTime || !m_pEffectGroup->IsLive() ||
		    (m_pAttachChar->MotionInfo->State == CHRMOTION_STATE_DEAD) ||
		    (m_pAttachChar->OnStageField>=0 && StageField[ m_pAttachChar->OnStageField ]->State == FIELD_STATE_VILLAGE) )
		{
			m_bLiveFlag = false;
			if(m_iParticleID != -1) {
				g_NewParticleMgr.SetFastStop(m_iParticleID);		//장순목 추가 (버서커 이펙트 제거되지 않는 버그 땜에 추가)
				m_iParticleID = -1;
			}
		}
		
		m_pEffectGroup->Main(elapsedTime);
	}
}


HoEffectType_Avater_Shot_Default::HoEffectType_Avater_Shot_Default()
{
	m_AttackPos.x = 0;
	m_AttackPos.y = 0;
	m_AttackPos.z = 0;
	
	memset(m_EffectGroup, 0, sizeof(m_EffectGroup));
	m_ShotState = SHOT_START;
	m_bLiveFlag = true;
	m_bLineAttachFlag = false;
}
HoEffectType_Avater_Shot_Default::~HoEffectType_Avater_Shot_Default()
{
	for(int index = 0; index < SHOT_END; index++)
	{
		if(m_EffectGroup[index] != NULL)
		{
			delete m_EffectGroup[index];
			m_EffectGroup[index] = NULL;
		}
	}
}

void HoEffectType_Avater_Shot_Default::Init(const HoEffectType_Avater_Shot_Info &info)
{
	m_CurrentPos.x = (float)info.m_CurrentPos.x / 256.f;
	m_CurrentPos.y = (float)info.m_CurrentPos.y / 256.f;
	m_CurrentPos.z = (float)info.m_CurrentPos.z / 256.f;

	m_AttackPos.x  = (float)info.m_AttackPos.x / 256.f;
	m_AttackPos.y  = (float)info.m_AttackPos.y / 256.f;
	m_AttackPos.z  = (float)info.m_AttackPos.z / 256.f;
	//m_AttachChar = info.m_AttachChar;
	m_EffectGroup[SHOT_START]       = info.m_EffectGroup[SHOT_START];
	m_EffectGroup[SHOT_PROCESS]     = info.m_EffectGroup[SHOT_PROCESS];
	m_EffectGroup[SHOT_END]         = info.m_EffectGroup[SHOT_END];
	
	m_fSpeed                        = info.m_fSpeed;

	point3 dist  = m_AttackPos - m_CurrentPos;
	float length = dist.Mag();
	m_Velocity = dist / length * m_fSpeed;
	if(info.m_LineTextureName.size())
	{
		m_bLineAttachFlag = true;
		m_EffectLineModel.m_TextureNameProperty.Init(info.m_LineTextureName);
		m_EffectLineModel.SetSize(info.m_LineSize);
	}
	
	//메시 각도를 튼다.
	GetRadian3D(info.m_CurrentPos.x, info.m_CurrentPos.y, info.m_CurrentPos.z, info.m_AttackPos.x, info.m_AttackPos.y, info.m_AttackPos.z);
	point3 shotAngle;
	shotAngle.x = (float)GeoResult_X;
	shotAngle.y = (float)GeoResult_Y; 
	shotAngle.z = 0;
	
	//shotAngle.y = float((-(int)shotAngle.y+ANGLE_180)&ANGCLIP);
	if(m_EffectGroup[SHOT_PROCESS])
		m_EffectGroup[SHOT_PROCESS]->SetCalcTranslate(NULL, &shotAngle);
}

void HoEffectType_Avater_Shot_Default::Main(float elapsedTime)
{
	if(m_ShotState == SHOT_START)    //준비 동작
	{
		if(m_EffectGroup[m_ShotState] == NULL)
			m_ShotState = SHOT_PROCESS;
		else
			m_EffectGroup[m_ShotState]->Main(elapsedTime);
	}
	else if(m_ShotState == SHOT_PROCESS)
	{
		if(m_EffectGroup[m_ShotState] != NULL)
		{
  			m_CurrentPos = m_CurrentPos + m_Velocity;
			m_EffectGroup[m_ShotState]->SetCalcTranslate(&m_CurrentPos, NULL);
			m_EffectGroup[m_ShotState]->Main(elapsedTime);
			point3 dist = m_AttackPos - m_CurrentPos;
			//충돌 했을 경우(오차 범위가 있으므로 라인, 구 충돌로 고려해야 한다.
		 	if(dist.MagSquared() < 10.f*10.f)
			{
				m_ShotState = SHOT_END;
				m_EffectGroup[m_ShotState]->SetCalcTranslate(&m_CurrentPos, NULL);
				m_EffectGroup[m_ShotState]->Main(0.f);
			}
			if(m_bLineAttachFlag)
				m_EffectLineModel.AddData(m_CurrentPos);
		}
	}
	else if(m_ShotState == SHOT_END) //타격 이펙트
	{
		if(GetSkillCode() == SKILL_X_RAGE)
		{
			m_bLiveFlag = false;
			POINT3D pos;
			pos.x = int(m_CurrentPos.x * 256.f);
			pos.y = int(m_CurrentPos.y * 256.f);
			pos.z = int(m_CurrentPos.z * 256.f);

			g_NewParticleMgr.Start("Skill3FighterBoneSmash1", pos);
		}
		else
		{
			m_EffectGroup[m_ShotState]->Main(elapsedTime);
			if(!m_EffectGroup[m_ShotState]->IsLive())
				m_bLiveFlag = false;
		}
	}
}

void HoEffectType_Avater_Shot_Default::UpdateView(int x, int y, int z, int ax, int ay, int az)
{
 	if(m_EffectGroup[m_ShotState] != NULL && m_bLiveFlag)
		m_EffectGroup[m_ShotState]->UpdateView(x, y, z, ax, ay, az);
	
	if(m_bLineAttachFlag && (m_ShotState == SHOT_PROCESS || m_ShotState == SHOT_END))
		m_EffectLineModel.UpdateView(x, y, z, ax, ay, az);
}


/*
void HoEffectType_Attach_AvaterObject::Init(HoEffectType_Attach_AvaterObject_Info *info)
{
	m_pAttachChar = info->m_pAttachChar;
	memcpy(m_pEffectGroupList, info->m_pEffectGroupList,4*MAX_ATTACH_OBJECT);
	memcpy(&m_Attach_Obj_Info, &info->m_Attach_Obj_Info, sizeof(HoEffectType_Attach_Obj_Info) * MAX_ATTACH_OBJECT);
}

void HoEffectType_Attach_AvaterObject::Main(float elapsedTime)
{
	for(int index = 0; index < MAX_ATTACH_OBJECT; index++)
	{
		if(m_pEffectGroupList[index] != NULL)
			m_pEffectGroupList[index]->Main(elapsedTime);
	}
}

void HoEffectType_Attach_AvaterObject::UpdateView(int x, int y, int z, int ax, int ay, int az)
{
	for(int index = 0; index < MAX_ATTACH_OBJECT; index++)
	{
		if(m_Attach_Obj_Info[index].m_pOBJ3D != NULL)
		{
			smMATRIX *mWorld;
			mWorld = &m_Attach_Obj_Info[index].m_pOBJ3D->mWorld;
			POINT3D angle;
			memcpy( &angle, &m_pAttachChar->Angle, sizeof( POINT3D ) );
			angle.y = (-angle.y+ANGLE_180)&ANGCLIP;
			AnimObjectTree( m_Attach_Obj_Info[index].m_pOBJ3D, m_pAttachChar->frame, angle.x, angle.y, angle.z);
			
			point3 pos;
			pos.x = (float)mWorld->_41/256.f;
			pos.z = (float)mWorld->_42/256.f;
			pos.y = (float)mWorld->_43/256.f;
			m_Attach_Obj_Info[index].m_pEffectGroup->SetCalcTranslate(&pos, NULL);
		}
	}
}
*/

void HoEffectType_Attach_AvaterToAvater::Init(HoEffectGroup *currentEffectGroup, HoEffectGroup *destEffectGroup)
{
	m_pCurrentEffectGroup = currentEffectGroup;
	m_pDestEffectGroup    = destEffectGroup;
}

void HoEffectType_Attach_AvaterToAvater::Init(smCHAR *char1, smCHAR *char2, const std::string &textureName, int textureCount, float aniDelayTime)
{
 	m_pAttachChar1 = char1;
	m_pAttachChar2 = char2;
	
	if(m_pAttachChar1 == NULL || m_pAttachChar2 == NULL)
	{
		m_bLiveFlag = false;
		return;
	}
 	m_CurrentPos.x = (float)m_pAttachChar1->pX/256.f;
	m_CurrentPos.y = (float)m_pAttachChar1->pY/256.f;
	m_CurrentPos.z = (float)m_pAttachChar1->pZ/256.f;

	m_DestPos.x = (float)m_pAttachChar2->pX/256.f;
	m_DestPos.y = (float)m_pAttachChar2->pY/256.f;
	m_DestPos.z = (float)m_pAttachChar2->pZ/256.f;
}

void HoEffectType_Attach_AvaterToAvater::Init(smCHAR *char1, smCHAR *char2, int partID1, int partID2)
{
	m_pAttachChar1 = char1;
	m_pAttachChar2 = char2;

	if(m_pAttachChar1 == NULL || m_pAttachChar2 == NULL)
	{
		m_bLiveFlag = false;
		return;
	}

	m_iParticleID[0] = partID1;
	m_iParticleID[1] = partID2;
}

HoEffectType_Attach_AvaterToAvater::~HoEffectType_Attach_AvaterToAvater()
{
	if(m_pCurrentEffectGroup)
	{
		delete m_pCurrentEffectGroup;
		m_pCurrentEffectGroup = NULL;
	}
	if(m_pDestEffectGroup)
	{
		delete m_pDestEffectGroup;
		m_pDestEffectGroup = NULL;
	}
}

void HoEffectType_Attach_AvaterToAvater::Main(float elapsed)
{
	if(m_pAttachChar1 == NULL || m_pAttachChar2 == NULL)
		return;
	m_CurrentPos.x = (float)m_pAttachChar1->pX/256.f;
	m_CurrentPos.y = (float)m_pAttachChar1->pY/256.f+(float)(m_pAttachChar1->PatHeight/2)/256.f;
	m_CurrentPos.z = (float)m_pAttachChar1->pZ/256.f;
	
	m_DestPos.x = (float)m_pAttachChar2->pX/256.f;
	m_DestPos.y = (float)m_pAttachChar2->pY/256.f+(float)m_pAttachChar2->PatHeight/256.f;
	m_DestPos.z = (float)m_pAttachChar2->pZ/256.f;

	m_AxialController.InitPos(m_CurrentPos.x, m_CurrentPos.y, m_CurrentPos.z, m_DestPos.x, m_DestPos.y, m_DestPos.z);
	m_AxialController.Main(elapsed);
	
	if(!m_AxialController.IsLive())
	{
		if(m_iParticleID[0] != -1)
		{
			g_NewParticleMgr.SetFastStop(m_iParticleID[0]);
			m_iParticleID[0] = -1;
		}

		if(m_iParticleID[1] != -1)
		{
			g_NewParticleMgr.SetFastStop(m_iParticleID[1]);
			m_iParticleID[1] = -1;
		}
		m_bLiveFlag = false;
	}

	
	if(m_pCurrentEffectGroup)
		m_pCurrentEffectGroup->Main(elapsed);
	if(m_pDestEffectGroup)
		m_pDestEffectGroup->Main(elapsed);

	if(m_iParticleID[0] != -1 && m_iParticleID[1] != -1)
	{
		D3DVECTOR curPos;
		D3DVECTOR desPos;
		curPos.x = m_CurrentPos.x * 256.f;
		curPos.y = m_CurrentPos.y * 256.f;
		curPos.z = m_CurrentPos.z * 256.f;

		desPos.x = m_DestPos.x * 256.f;
		desPos.y = m_DestPos.y * 256.f;
		desPos.z = m_DestPos.z * 256.f;

		g_NewParticleMgr.SetAttachPos(m_iParticleID[0], curPos);
		g_NewParticleMgr.SetAttachPos(m_iParticleID[1], desPos);
	}
}

void HoEffectType_Attach_AvaterToAvater::UpdateView(int x, int y, int z, int ax, int ay, int az)
{
	m_AxialController.UpdateView(x, y, z, ax, ay, az);
	if(m_pCurrentEffectGroup != NULL)
	{
		m_pCurrentEffectGroup->SetCalcTranslate(&m_CurrentPos, NULL);
		m_pCurrentEffectGroup->UpdateView(x, y, z, ax, ay, az);
	}
	if(m_pDestEffectGroup != NULL)
	{
		m_pDestEffectGroup->SetCalcTranslate(&m_DestPos, NULL);
		m_pDestEffectGroup->UpdateView(x, y, z, ax, ay, az);
	}
}

void HoEffectType_Avater_Blade_Blur::Main(float elapsedTime)
{
	m_fElapsedTime += elapsedTime;
	if(m_fEndTime <= m_fElapsedTime)
		m_bLiveFlag = false;
	if(m_bMyCharacterFlag)
	{
		if(m_pAttachChar != lpCurPlayer)
			m_pAttachChar = lpCurPlayer;
	}
}

void HoEffectType_Avater_Blade_Blur::Init(smCHAR *pChar, const std::string &fileName, float endTime)
{
	m_TextureNameProperty.Init(fileName);
	
	if(pChar == lpCurPlayer)
		m_bMyCharacterFlag = true;
	m_pAttachChar = pChar;
	m_fEndTime = endTime;

	int matNum =  m_TextureNameProperty.GetMatNum();
	
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
}

void HoEffectType_Avater_Blade_Blur::UpdateView(int x, int y, int z, int ax, int ay, int az)
{
	if(m_pAttachChar == NULL)
		return;
	int mLevel = 32; 
	int mFrames = 30;
	int mLevel2 = 0;
	DWORD pframe;
	int	vcnt = 0;
	smVERTEX	*lpVertex;
	smFACE		*lpFace;
	
	mLevel2  = mLevel<<1;
	
	lpVertex = MotionBlurVertex;
	lpFace   = MotionBlurFace;
	
	POINT3D angle;
 	memcpy( &angle, &m_pAttachChar->Angle , sizeof( POINT3D ) );
	angle.y = (-angle.y+ANGLE_180)&ANGCLIP;
	
	
	POINT3D weaponTopPos;
	POINT3D weaponBottomPos;
	
	memset(&weaponTopPos, 0, sizeof(weaponTopPos));
 	memset(&weaponBottomPos, 0, sizeof(weaponBottomPos));
	weaponBottomPos.z = m_pAttachChar->HvRightHand.SizeMax;	

	//양손 무기 5도 기울어짐..
	int cnt;
	int sin;
	int cos;
	if ( m_pAttachChar->dwActionItemTwoHand ) 
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
	mWorld = &m_pAttachChar->HvRightHand.ObjBip->mWorld;
	int rx, ry, rz;
	
	for(cnt=0;cnt<mLevel;cnt++) 
	{
		pframe = m_pAttachChar->frame-(cnt*mFrames);
		if ( pframe<m_pAttachChar->MotionInfo->StartFrame ) 
			pframe = m_pAttachChar->MotionInfo->StartFrame;
 		vcnt = cnt<<1;
		
		AnimObjectTree( m_pAttachChar->HvRightHand.ObjBip, pframe, angle.x , angle.y , angle.z );
		rx = weaponTopPos.x * mWorld->_11 +
			 weaponTopPos.y * mWorld->_21 +
			 weaponTopPos.z * mWorld->_31 ;

		ry = weaponTopPos.x * mWorld->_12 +
			 weaponTopPos.y * mWorld->_22 +
			 weaponTopPos.z * mWorld->_32 ;

		rz = weaponTopPos.x * mWorld->_13 +
			 weaponTopPos.y * mWorld->_23 +
			 weaponTopPos.z * mWorld->_33 ;

		lpVertex[vcnt].x = m_pAttachChar->pX + (rx>>FLOATNS) + mWorld->_41;
		lpVertex[vcnt].z = m_pAttachChar->pZ + (ry>>FLOATNS) + mWorld->_42;
		lpVertex[vcnt].y = m_pAttachChar->pY + (rz>>FLOATNS) + mWorld->_43;
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

		lpVertex[vcnt].x = m_pAttachChar->pX + (rx>>FLOATNS) + mWorld->_41;
		lpVertex[vcnt].z = m_pAttachChar->pZ + (ry>>FLOATNS) + mWorld->_42;
		lpVertex[vcnt].y = m_pAttachChar->pY + (rz>>FLOATNS) + mWorld->_43;
	}

	//클리핑 모드값
	int ClipStates = ( SMCLIP_NEARZ | SMCLIP_FARZ | SMCLIP_LEFT | SMCLIP_RIGHT | SMCLIP_TOP | SMCLIP_BOTTOM );

	smRender.Init();
	smRender.SetMaterialGroup(smMaterialGroup);

	smRender.SetCameraPosi(x, y, z, ax, ay, az);
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

void HoEffectType_Magnetic_Sphere::Init(smCHAR *pChar, float endTime, float attackTime)
{
	std::string meshName = "Effect\\NewEffect\\Res\\Object\\MC_4-3.ASE";
	std::string boneName;
	m_pMesh = HoEffectResManager::GetInstance()->GetResMesh(meshName, boneName);
    if(m_pMesh == NULL)
	{
		m_bLiveFlag = false;
        return;
	}
	
	m_pObj[0] = m_pMesh->GetObjectFromName("L-02");
	m_pObj[1] = m_pMesh->GetObjectFromName("L-03");
	m_pObj[2] = m_pMesh->GetObjectFromName("L-04");

	POINT3D pos;
	pos.x = 0;
	pos.y = 0;
	pos.z = 0;
	m_ParticleID[0] = g_NewParticleMgr.Start("magneticlight", pos);
	m_ParticleID[1] = g_NewParticleMgr.Start("magneticlight", pos);
	m_ParticleID[2] = g_NewParticleMgr.Start("magneticlight", pos);

	m_pAttachChar = pChar;
	if(m_pAttachChar == lpCurPlayer)
		m_bMyCharacterFlag = true;

	m_fEndTime = endTime;

	m_fAttackTime = attackTime;
}

void HoEffectType_Magnetic_Sphere::UpdateView(int x, int y, int z, int ax, int ay, int az)
{
	SMeshRenderInfo renderInfo;
	point3 translate;
	point3 angle;
	translate.x = m_pAttachChar->pX/256.f;
	translate.y = (m_pAttachChar->pY+10000)/256.f;
	translate.z = m_pAttachChar->pZ/256.f;
	
	angle.x = 0;
	angle.y = 0;
	angle.z = 0;

	color4 color;
	color.r = 255.f;
	color.g = 255.f;
	color.b = 255.f;
	color.a = 255.f;

    renderInfo.m_Pos           = translate;
    renderInfo.m_Angle         = angle;
    renderInfo.m_Pat           = m_pMesh;
    renderInfo.m_Color         = color;
    renderInfo.m_iBlendType    = SMMAT_BLEND_LAMP;
    renderInfo.m_iCurrentFrame = (int)m_fCurrentFrame;
    HoEffectView::UpdateMesh(&renderInfo, x, y, z, ax, ay, az);
}

void HoEffectType_Magnetic_Sphere::Main(float elapsedTime)
{
 	if(m_bLiveFlag == false)
		return;
	
 	if(m_bMyCharacterFlag)
	{
		if(m_pAttachChar != lpCurPlayer)
			m_pAttachChar = lpCurPlayer;
	}
	
	smCHAR *attackMonster = NULL;

	if(m_fAttackTimeCount >= m_fAttackTime)
	{
		m_fAttackTimeCount = 0.f;
		attackMonster = FindNearMonster(m_pAttachChar);
	}

	m_fAttackTimeCount += elapsedTime;
	m_fCurrentFrame += (160.f*30.f)*elapsedTime;
	m_fElapsedTime += elapsedTime;
	if( m_fCurrentFrame >= 58.f*160.f)
		m_fCurrentFrame = 0.f;


	
	for(int index = 0; index < 3; index++)
	{
		smMATRIX *mWorld;
  		AnimObjectTree(m_pObj[index],(int)m_fCurrentFrame, 0, 0, 0);
		mWorld = &m_pObj[index]->mWorld;

		if(m_ParticleID[index] != -1)
		{
			D3DVECTOR charPos;
			charPos.x = (float)(m_pAttachChar->pX+mWorld->_41);
			charPos.y = (float)(m_pAttachChar->pY+mWorld->_43+10000);
			charPos.z = (float)(m_pAttachChar->pZ+mWorld->_42);
			
 			g_NewParticleMgr.SetAttachPos(m_ParticleID[index], charPos);
			if(attackMonster)
			{
				if(attackMonster->Flag && attackMonster->DisplayFlag)
				{
					POINT3D curPos;
					POINT3D desPos;
					curPos.x = (int)charPos.x;
					curPos.y = (int)charPos.y;
					curPos.z = (int)charPos.z;
					desPos.x = attackMonster->pX;
					desPos.y = attackMonster->pY+5000;
					desPos.z = attackMonster->pZ;
					if(index == 0)
						SkillMetalionSphereDefence(&curPos, &desPos, true);
					else
						SkillMetalionSphereDefence(&curPos, &desPos, false);
					if(m_bMyCharacterFlag && attackMonster && index == 0)
						SendMegneticSphereDamage(attackMonster);
				}
			}
			
		}
	}
	
	if(m_fElapsedTime >= m_fEndTime || m_pAttachChar->Flag == 0 ||
	   (!m_pAttachChar->MotionInfo || m_pAttachChar->MotionInfo->State == CHRMOTION_STATE_DEAD) ||
	   (m_pAttachChar->OnStageField>=0 && StageField[ m_pAttachChar->OnStageField ]->State == FIELD_STATE_VILLAGE) )
	{
		m_bLiveFlag = false;
		for(int i = 0; i < 3; i++)
		{
			if(m_ParticleID[i] != -1)
			{
				g_NewParticleMgr.SetFastStop(m_ParticleID[i]);
				m_ParticleID[i] = -1;
			}
		}
	}
}




void HoEffectType_MusPel::UpdateView(int x, int y, int z, int ax, int ay, int az)
{
  	SMeshRenderInfo renderInfo;
	point3 translate;
	point3 angle;
	
	translate.x = m_CurrentPos.x/256.f;
	translate.y = m_CurrentPos.y/256.f;
	translate.z = m_CurrentPos.z/256.f;
	
	angle.y = (float)((-m_pAttachChar->Angle.y+ANGLE_180)&ANGCLIP);
	angle.x = 0;
	angle.z = 0;

	color4 color;
	color.r = 255.f;
	color.g = 255.f;
	color.b = 255.f;
	color.a = m_Alpha; 

    renderInfo.m_Pos           = translate;
    renderInfo.m_Angle         = angle;
    renderInfo.m_Pat           = m_pMesh;
    renderInfo.m_Color         = color;
    renderInfo.m_iBlendType    = SMMAT_BLEND_ALPHA;
    renderInfo.m_iCurrentFrame = (int)m_fCurrentFrame;
    HoEffectView::UpdateMesh(&renderInfo, x, y, z, ax, ay, az);
}

void HoEffectType_MusPel::Main(float elapsedTime)
{
	if(m_bLiveFlag == false)
		return;
	
	if(m_bMyCharacterFlag)
	{
		if(m_pAttachChar != lpCurPlayer)
			m_pAttachChar = lpCurPlayer;
	}

	m_fCurrentFrame += (160.f*30.f)*elapsedTime;
	Process();
	m_fElapsedTime += elapsedTime;
	m_fAttackTimeCount += elapsedTime;

	if(m_fElapsedTime >= m_fEndTime || m_pAttachChar->Flag == 0 ||
	   (!m_pAttachChar->MotionInfo || m_pAttachChar->MotionInfo->State == CHRMOTION_STATE_DEAD) ||
 	   (m_pAttachChar->OnStageField>=0 && StageField[ m_pAttachChar->OnStageField ]->State == FIELD_STATE_VILLAGE) )
		SetState(MUSPEL_DEAD);
	
      if(m_ParticleID != -1)
	{
  		if(m_pObj)
		{
			smMATRIX *mWorld;
  			AnimObjectTree(m_pObj,(int)m_fCurrentFrame, 0, (-m_pAttachChar->Angle.y+ANGLE_180)&ANGCLIP, 0);
			mWorld = &m_pObj->mWorld;
			D3DVECTOR charPos;
			charPos.x = (m_CurrentPos.x+float(mWorld->_41) );
			charPos.y = (m_CurrentPos.y+float(mWorld->_43) + 1000);
			charPos.z = (m_CurrentPos.z+float(mWorld->_42) );

			g_NewParticleMgr.SetPos(m_ParticleID, charPos);
		}
	}
}

void HoEffectType_MusPel::SetState(int state)
{
	m_NextState = state;
}

void HoEffectType_MusPel::Process()
{
	OnEvent(m_State, EVENT_UPDATE);
	if(m_State != m_NextState)
	{
		OnEvent(m_State, EVENT_EXIT);
		m_State = m_NextState;
		OnEvent(m_State, EVENT_INIT);
	}
}

void HoEffectType_MusPel::OnEvent(int curState, int curEvent)
{
     if(curState == MUSPEL_SPAWN)
	{
  		if(curEvent == EVENT_INIT)
		{
			m_Alpha = 60.f;
			POINT3D pos;
			pos.x = m_pAttachChar->pX;
			pos.y = m_pAttachChar->pY + 5000;
			pos.z = m_pAttachChar->pZ;

			m_CurrentPos.x = (float)m_pAttachChar->pX;
			m_CurrentPos.y = float(m_pAttachChar->pY+10000);
			m_CurrentPos.z = (float)m_pAttachChar->pZ;
			
			m_ParticleID = g_NewParticleMgr.Start("MusPell", pos);
			pos.y += 5000;
			g_NewParticleMgr.Start("MusPellStart", pos);
			m_fCurrentFrame = 10.f * 160.f;
		}
		else if(curEvent == EVENT_UPDATE)
		{
			m_Alpha += 1.f;
			if(m_Alpha >= 80.f)
				m_Alpha = 80.f;
			if(m_fCurrentFrame >= 70.f * 160.f)
 				m_fCurrentFrame = 70.f * 160.f;
			if(m_fCurrentFrame >= 70.f * 160.f && m_Alpha >= 80.f)
				SetState(MUSPEL_IDLE);
		}
	}
	
 	if(curState == MUSPEL_DEAD)
	{
		if(curEvent == EVENT_INIT)
		{
			m_fCurrentFrame = 80.f * 160.f;
				POINT3D pos;
				pos.x = (int)m_CurrentPos.x;
				pos.y = (int)m_CurrentPos.y;
				pos.z = (int)m_CurrentPos.z; 
			g_NewParticleMgr.Start("MusPellEnd", pos);
		}
		if(curEvent == EVENT_UPDATE)
		{
 			m_Alpha -= 1.f;
			if(m_Alpha <= 60.f)
				m_Alpha = 60.f;

			if(m_Alpha <= 60.f)
			{
				m_bLiveFlag = false;
				if(m_ParticleID != -1)
				{
					g_NewParticleMgr.SetStop(m_ParticleID);
					m_ParticleID = -1;
				}
			}
		}
	}

  	if(curState == MUSPEL_IDLE)
	{
		if(curEvent == EVENT_INIT)
			m_fCurrentFrame = 80.f * 160.f;
		else if(curEvent == EVENT_UPDATE)
		{
			if(m_fCurrentFrame >= 160.f * 160.f)
				m_fCurrentFrame = 80.f * 160.f;
		}
		if(m_fAttackTimeCount >= m_fAttackTime)
		{
			m_fAttackTimeCount = 0.f;
			m_AttackMonster = FindNearMonster(m_pAttachChar);
			if(m_AttackMonster)
			{
				if(m_AttackMonster->Flag && m_AttackMonster->DisplayFlag)
					SetState(MUSPEL_ATTACK);
			}
		}
		
		if(m_pAttachChar && m_pAttachChar->MotionInfo)
		{
			if ( m_pAttachChar->MotionInfo->State == CHRMOTION_STATE_WALK ||
				 m_pAttachChar->MotionInfo->State == CHRMOTION_STATE_RUN )
				SetState(MUSPEL_WALK);
		}
		
	}
	
	if(curState == MUSPEL_WALK)
	{
		if(curEvent == EVENT_INIT)
			m_fCurrentFrame = 170.f * 160.f;
		else if(curEvent == EVENT_UPDATE)
		{
 			if(m_fCurrentFrame >= 210.f * 160.f)
				m_fCurrentFrame = 170.f * 160.f;

			//if(m_pAttachChar->MotionInfo->State == CHRMOTION_STATE_STAND)
				//SetState(MUSPEL_IDLE);
			
			D3DVECTOR	attractLocation;
  			attractLocation.x = float((m_pAttachChar->pX) - m_CurrentPos.x);
			attractLocation.y = float((m_pAttachChar->pY+10000) - m_CurrentPos.y);
			attractLocation.z = float(m_pAttachChar->pZ - m_CurrentPos.z);
			D3DVECTOR attractDir = attractLocation;

			//벡터의 길이를 구한다.
			float length;
   			length = (float)sqrt(attractDir.x*attractDir.x+attractDir.y*attractDir.y+attractDir.z*attractDir.z);
			
			if(length > 300.f)
			{
				m_CurrentPos.x += int(((attractLocation.x)/length)*(length/15.f));
				m_CurrentPos.y += int(((attractLocation.y)/length)*(length/15.f));
				m_CurrentPos.z += int(((attractLocation.z)/length)*(length/15.f));
			}
			else
				SetState(MUSPEL_IDLE);
		}
	}
	
	if(curState == MUSPEL_ATTACK)
	{
 		if(curEvent == EVENT_INIT)
		{
			m_fCurrentFrame = 300 * 160.f;
		}
		else if(curEvent == EVENT_UPDATE)
		{
 			if((m_fCurrentFrame >= 330.f * 160.f) && m_bHandEffectFlag == false)
			{
				POINT3D pos;
 				pos.x = (int)m_CurrentPos.x;
				pos.y = (int)m_CurrentPos.y+5000;
				pos.z = (int)m_CurrentPos.z;

				m_HandParticleNum = g_NewParticleMgr.Start("MuspellHand", pos);
				m_bHandEffectFlag = true;
			}
			else
			{
				if(m_HandParticleNum != -1) 
				{
					D3DVECTOR pos;
 					pos.x = m_CurrentPos.x;
 					pos.y = m_CurrentPos.y+8000;
					pos.z = m_CurrentPos.z;
					g_NewParticleMgr.SetAttachPos(m_HandParticleNum, pos);
				}
			}
 			if((m_fCurrentFrame >= 340.f * 160.f) && m_bHitEffectFlag == false && m_AttackMonster != NULL)
			{
				POINT3D pos;
				pos.x = m_AttackMonster->pX;
				pos.y = m_AttackMonster->pY+m_AttackMonster->PatHeight/2;
				pos.z = m_AttackMonster->pZ;
				g_NewParticleMgr.Start("MuspellHit1", pos);
				m_bHitEffectFlag = true;
				SetDynLight( m_AttackMonster->pX, m_AttackMonster->pY, m_AttackMonster->pZ, 255, 150,50,0, 180, 3);

				if(m_bMyCharacterFlag)
					SendMuspellDamage(m_AttackMonster);
			}

			if(m_fCurrentFrame >= 380.f * 160.f)
				SetState(MUSPEL_WALK);
		}
		else if(curEvent == EVENT_EXIT)
		{
			m_bHandEffectFlag = false;
			m_bHitEffectFlag = false;
			m_HandParticleNum = -1;
		}
	}

	if(curState == MUSPEL_DEFENCE)
	{
		if(curEvent == EVENT_INIT)
			m_fCurrentFrame = 220.f * 160.f;
		else if(curEvent == EVENT_UPDATE)
			if(m_fCurrentFrame >= 250.f * 160.f)
				SetState(MUSPEL_IDLE);
	}
}

void HoEffectType_MusPel::Init(smCHAR *pChar, float endTime, float attackTime)
{
	m_pMesh = (smPAT3D *)AssaSearchRes("muspel.ASE", SMMAT_BLEND_ALPHA);
    if(m_pMesh == NULL)
	{
 		m_bLiveFlag = false;
        return;
	}
	m_pAttachChar = pChar;
	if(m_pAttachChar == lpCurPlayer)
		m_bMyCharacterFlag = true;

	m_fEndTime = endTime;

	smPAT3D *bone = (smPAT3D *)AssaSearchRes("b_muspel.ASE", SMMAT_BLEND_ALPHA);
	m_pObj = bone ? bone->GetObjectFromName("Bip01") : NULL;
	
	m_fAttackTime = attackTime;
}

void HoEffectType_RegenerationField::Init(smCHAR *pChar)
{
	m_pMesh = (smPAT3D *)AssaSearchRes("PT_4-2-40.ASE", SMMAT_BLEND_LAMP);
	if(m_pMesh == NULL)
	{
		m_bLiveFlag = false;
		return;
	}

	m_pAttachChar = pChar;
	if(m_pAttachChar == lpCurPlayer)
		m_bMyCharacterFlag = true;

	POINT3D pos;
	pos.x = m_pAttachChar->pX;
	pos.y = m_pAttachChar->pY;
	pos.z = m_pAttachChar->pZ;

	m_iParticleID = g_NewParticleMgr.Start("Skill4RegenerationField", pos);
	if(m_iParticleID == -1)
	{
		m_bLiveFlag = false;
		return;
	}
}

void HoEffectType_RegenerationField::UpdateView(int x, int y, int z, int ax, int ay, int az)
{
	SMeshRenderInfo renderInfo;
	point3 translate;
	point3 angle;
	
	translate.x = float(m_pAttachChar->pX)/256.f;
	translate.y = float(m_pAttachChar->pY+4000)/256.f;
	translate.z = float(m_pAttachChar->pZ)/256.f;
	
	angle.y = (float)((-m_pAttachChar->Angle.y+ANGLE_180)&ANGCLIP);
	angle.x = 0;
	angle.z = 0;

	color4 color;
	color.r = 255.f;
	color.g = 255.f;
	color.b = 255.f;
	color.a = 255.f; 

    renderInfo.m_Pos           = translate;
    renderInfo.m_Angle         = angle;
    renderInfo.m_Pat           = m_pMesh;
    renderInfo.m_Color         = color;
    renderInfo.m_iBlendType    = SMMAT_BLEND_LAMP;
    renderInfo.m_iCurrentFrame = (int)m_fCurrentFrame;
    HoEffectView::UpdateMesh(&renderInfo, x, y, z, ax, ay, az);

	POINT3D pos;
	pos.x = m_pAttachChar->pX;
	pos.y = m_pAttachChar->pY-5000;
	pos.z = m_pAttachChar->pZ;
}

void HoEffectType_RegenerationField::Main(float elapsedTime)
{
	if(!m_bLiveFlag)
		return;
	if(m_bMyCharacterFlag)
	{
		if(m_pAttachChar != lpCurPlayer)
			m_pAttachChar = lpCurPlayer;
	}

	m_fCurrentFrame += (160.f*30.f)*elapsedTime;
	if(m_fCurrentFrame >= 40*160.f)
		m_fCurrentFrame = 0.f;

	if(m_pAttachChar)
	{
		if(m_pAttachChar->Flag == 0 || m_pAttachChar->MotionInfo == NULL)
		{
			m_bLiveFlag = false;
			if(m_iParticleID != -1)
			{
				g_NewParticleMgr.SetStop(m_iParticleID);
				m_iParticleID = -1;
			}
			return;
		}

	 	int compFrame = m_pAttachChar->frame - (m_pAttachChar->MotionInfo->StartFrame*160);
		if( compFrame < (int)m_pAttachChar->MotionInfo->EventFrame[0] || compFrame > (int)m_pAttachChar->MotionInfo->EventFrame[1]) 
		{
			m_bLiveFlag = false;
			if(m_iParticleID != -1)
			{
				g_NewParticleMgr.SetStop(m_iParticleID);
				m_iParticleID = -1;
			}
		}
	}
}



void HoEffectType_PhoneixShot::Init(point3 &curPos, point3 &desPos)
{
 	m_CurrentPos.x = (float)curPos.x / 256.f;
	m_CurrentPos.y = (float)curPos.y / 256.f;
	m_CurrentPos.z = (float)curPos.z / 256.f;

	m_AttackPos.x  = (float)desPos.x / 256.f;
	m_AttackPos.y  = (float)desPos.y / 256.f;
	m_AttackPos.z  = (float)desPos.z / 256.f;
	
	point3 dist  = m_AttackPos - m_CurrentPos;
	float length = dist.Mag();
	m_Velocity = dist / length * 9.f;
	
	//메시 각도를 튼다.
	GetRadian3D(int(m_CurrentPos.x*256.f), 
		        int(m_CurrentPos.y*256.f),
				int(m_CurrentPos.z*256.f), 
				int(m_AttackPos.x*256.f), 
				int(m_AttackPos.y*256.f), 
				int(m_AttackPos.z*256.f) );
	
	m_ShotAngle.x = (float)(GeoResult_X & ANGCLIP);
	m_ShotAngle.y = (float)(GeoResult_Y & ANGCLIP); 
	m_ShotAngle.z = 0;
	m_ShotAngle.y = float((-(int)m_ShotAngle.y+ANGLE_180)&ANGCLIP);
 
 	m_pMesh = (smPAT3D *)AssaSearchRes("ac_4-3.ASE", SMMAT_BLEND_LAMP);

	POINT3D pos;
	pos.x = int(m_CurrentPos.x * 256.f);
	pos.y = int(m_CurrentPos.y * 256.f);
	pos.z = int(m_CurrentPos.z * 256.f);
	m_iParticleID = g_NewParticleMgr.Start("Skill4PhoenixShot",pos);
}

void HoEffectType_PhoneixShot::UpdateView(int x, int y, int z, int ax, int ay, int az)
{
	SMeshRenderInfo renderInfo;
	
	color4 color;
	color.r = 255.f;
	color.g = 255.f;
	color.b = 255.f;
	color.a = m_Alpha; 

    renderInfo.m_Pos           = m_CurrentPos;
    renderInfo.m_Angle         = m_ShotAngle;
    renderInfo.m_Pat           = m_pMesh;
    renderInfo.m_Color         = color;
    renderInfo.m_iBlendType    = SMMAT_BLEND_LAMP;
    renderInfo.m_iCurrentFrame = 160;
    HoEffectView::UpdateMesh(&renderInfo, x, y, z, ax, ay, az);	
}

void HoEffectType_PhoneixShot::Main(float elapsedTime)
{
	if(m_bLiveFlag == false)
		return;
	
	m_CurrentPos = m_CurrentPos + m_Velocity;
	point3 dist = m_AttackPos - m_CurrentPos;

	if(m_iParticleID != -1)
	{
		if(g_NewParticleMgr.GetRunning(m_iParticleID))
		{
			D3DVECTOR pos;
			pos.x = m_CurrentPos.x * 256.f;
			pos.y = m_CurrentPos.y * 256.f;
			pos.z = m_CurrentPos.z * 256.f;

			g_NewParticleMgr.SetPos(m_iParticleID, pos);
		}
		else
			m_iParticleID = -1;
	}

	if(m_bDeadFlag)
	{
		m_Alpha -= 10;
		if(m_Alpha <= 0)
		{
			if(m_iParticleID != -1)
			{
				g_NewParticleMgr.SetStop(m_iParticleID);
	 			m_iParticleID = -1;
				
			}
			m_bLiveFlag = true;
		}
		return;
	}
	
	//충돌 했을 경우(오차 범위가 있으므로 라인, 구 충돌로 고려해야 한다.
	if(dist.MagSquared() < 10.f*10.f)
	{
		m_bDeadFlag = true;
	}
}

void HoEffectType_FireShot::Init(point3 &curPos, point3 &desPos)
{
	m_CurrentPos.x = (float)curPos.x / 256.f;
	m_CurrentPos.y = (float)curPos.y / 256.f;
	m_CurrentPos.z = (float)curPos.z / 256.f;

	m_AttackPos.x  = (float)desPos.x / 256.f;
	m_AttackPos.y  = (float)desPos.y / 256.f;
	m_AttackPos.z  = (float)desPos.z / 256.f;
	
	point3 dist  = m_AttackPos - m_CurrentPos;
	float length = dist.Mag();
	m_Velocity = dist / length * 9.f;

	POINT3D pos;
	pos.x = int(m_CurrentPos.x * 256.f);
	pos.y = int(m_CurrentPos.y * 256.f);
	pos.z = int(m_CurrentPos.z * 256.f);
	m_iParticleID = g_NewParticleMgr.Start("Skill4PhoenixShot",pos);
}

void HoEffectType_FireShot::Main(float elapsedTime)
{
	if(m_bLiveFlag == false)
		return;
	
	m_CurrentPos = m_CurrentPos + m_Velocity;
	point3 dist = m_AttackPos - m_CurrentPos;
	
	if(m_iParticleID != -1)
	{
		D3DVECTOR pos;
		pos.x = m_CurrentPos.x * 256.f;
		pos.y = m_CurrentPos.y * 256.f;
		pos.z = m_CurrentPos.z * 256.f;

		g_NewParticleMgr.SetPos(m_iParticleID, pos);
	}

	if(dist.MagSquared() < 10.f*10.f)
	{
		if(m_iParticleID != -1)
		{
			g_NewParticleMgr.SetStop(m_iParticleID);
			m_iParticleID = -1;
			POINT3D pos;
			pos.x = int((float)m_CurrentPos.x * 256.f);
			pos.y = int((float)m_CurrentPos.y * 256.f);
			pos.y = int((float)m_CurrentPos.z * 256.f);

			g_NewParticleMgr.Start("FireElementalHit", pos);
		}
	}
}

void HoEffectType_FireShot::UpdateView(int x, int y, int z, int ax, int ay, int az)
{
}

HoEffectManager* HoEffectManager::m_Instance = 0;
HoEffectManager* HoEffectManager::GetInstance()
{
    if(m_Instance == 0)
        m_Instance = new HoEffectManager;
    return m_Instance;
}

void HoEffectManager::UpdateView(int x, int y, int z, int ax, int ay, int az)
{
	std::vector<HoEffectTypeInterface *>::iterator i;
	for (i = m_PostEffectTypeList.begin();i != m_PostEffectTypeList.end(); i++)
    {
 		if((*i) != NULL && (*i)->IsLive())
			(*i)->UpdateView(x, y, z, ax, ay, az);
	}

 	lpD3DDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, FALSE);
	//std::vector<HoEffectTypeInterface *>::iterator i;
	for (i = m_EffectTypeList.begin();i != m_EffectTypeList.end(); i++)
    {
 		if((*i) != NULL && (*i)->IsLive())
			(*i)->UpdateView(x, y, z, ax, ay, az);
	}

	for (i = m_MaintainEffectTypeList.begin();i != m_MaintainEffectTypeList.end(); i++)
    {
 		if((*i) != NULL && (*i)->IsLive())
			(*i)->UpdateView(x, y, z, ax, ay, az);
	}

	lpD3DDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, TRUE);
}

void HoEffectManager::Main(float elapsedTime)
{
	std::vector<HoEffectTypeInterface *>::iterator i;
	for (i = m_PostEffectTypeList.begin();i != m_PostEffectTypeList.end(); i++)
    {
		if((*i) != NULL)
		{
			if(!(*i)->IsLive())
			{ 
				delete (*i);
				(*i) = NULL;
			}
			else
				(*i)->Main(elapsedTime);
		}
	}	

	
	for (i = m_EffectTypeList.begin();i != m_EffectTypeList.end(); i++)
    {
		if((*i) != NULL)
		{
			if(!(*i)->IsLive())
			{ 
				delete (*i);
				(*i) = NULL;
			}
			else
				(*i)->Main(elapsedTime);
		}
	}

	for (i = m_MaintainEffectTypeList.begin();i != m_MaintainEffectTypeList.end(); i++)
    {
		if((*i) != NULL)
		{
			if(!(*i)->IsLive())
			{ 
				delete (*i);
				(*i) = NULL;
			}
			else
				(*i)->Main(elapsedTime);
		}
	}
}

BOOL HoEffectManager::GetSkillCode(smCHAR *pChar, int code)
{
	std::vector<HoEffectTypeInterface *>::iterator i;
	for (i = m_EffectTypeList.begin();i != m_EffectTypeList.end(); i++)
    {
        if( (*i) != NULL )
		{
			if((*i)->GetAttachChar() == pChar)
			{
				if((*i)->GetSkillCode() == code)
					return TRUE;
			}
		}
	}

	
	for (i = m_PostEffectTypeList.begin();i != m_PostEffectTypeList.end(); i++)
    {	
		if( (*i) != NULL )
		{
			if((*i)->GetAttachChar() == pChar)
			{
				if((*i)->GetSkillCode() == code)
					return TRUE;
			}
		}
	}
	
	for (i = m_MaintainEffectTypeList.begin();i != m_MaintainEffectTypeList.end(); i++)
    {
        if( (*i) != NULL )
		{
			if((*i)->GetAttachChar() == pChar)
			{
				if((*i)->GetSkillCode() == code)
					return TRUE;
			}
		}
	}

	return FALSE;
}

void HoEffectManager::AddEffect(HoEffectTypeInterface *effect)
{
	std::vector<HoEffectTypeInterface *>::iterator i;
	for (i = m_EffectTypeList.begin();i != m_EffectTypeList.end(); i++)
    {
        if( (*i) == NULL )
		{
			(*i) = effect;
			return;
		}
	}
	m_EffectTypeList.push_back(effect);
}

void HoEffectManager::AddPostEffect(HoEffectTypeInterface *effect)
{
	std::vector<HoEffectTypeInterface *>::iterator i;
	for (i = m_PostEffectTypeList.begin();i != m_PostEffectTypeList.end(); i++)
    {
        if( (*i) == NULL )
		{
			(*i) = effect;
			return;
		}
	}
	m_PostEffectTypeList.push_back(effect);
}

void HoEffectManager::AddMaintainEffect(HoEffectTypeInterface *effect)
{
	std::vector<HoEffectTypeInterface *>::iterator i;
	for (i = m_MaintainEffectTypeList.begin();i != m_MaintainEffectTypeList.end(); i++)
    {
        if( (*i) == NULL )
		{
			(*i) = effect;
			return;
		}
	}
	m_MaintainEffectTypeList.push_back(effect);
}

bool HoEffectManager::SetMaintainEffect(int code , smCHAR *pAttachChar, float time)
{
	int maintainEffectCount = m_MaintainEffectTypeList.size();
	
 	std::vector<HoEffectTypeInterface *>::iterator i;
	for (i = m_MaintainEffectTypeList.begin();i != m_MaintainEffectTypeList.end(); i++)
    {
		if((*i) != NULL)
		{
			if((*i)->GetSkillCode() == code)
			{
				if((*i)->GetAttachChar() == pAttachChar)
				{
					(*i)->SetAddEndTime(time);
					return true;
				}
			}
		}
	}
	return false;
}

void HoEffectManager::Clear()
{
	for (std::vector<HoEffectTypeInterface *>::iterator i = m_PostEffectTypeList.begin(); 
		i != m_PostEffectTypeList.end(); i++)
    {
        if (*i)
        {
            if( (*i)->IsLive())
			{
				delete (*i);
				(*i) = NULL;
			}
        }
    }
    m_PostEffectTypeList.clear();

	for (std::vector<HoEffectTypeInterface *>::iterator i = m_EffectTypeList.begin(); 
		i != m_EffectTypeList.end(); i++)
    {
        if (*i)
        {
            if( (*i)->IsLive())
			{
				delete (*i);
				(*i) = NULL;
			}
        }
    }
    m_EffectTypeList.clear();


	for (std::vector<HoEffectTypeInterface *>::iterator i = m_MaintainEffectTypeList.begin(); 
		i != m_MaintainEffectTypeList.end(); i++)
    {
        if (*i)
        {
            if( (*i)->IsLive())
			{
				delete (*i);
				(*i) = NULL;
			}
        }
    }
    m_MaintainEffectTypeList.clear();

}




