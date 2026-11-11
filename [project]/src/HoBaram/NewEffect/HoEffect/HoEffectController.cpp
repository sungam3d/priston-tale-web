#include "HoEffectController.h"
#include "HoEffectView.h"
#include "HoEffectResMgr.h"

void HoEffectEventColor::VisitEffectModel(HoEffectModelInterface &effectModel)
{
    if(!IsFade())
        effectModel.m_Color = m_Color.GetRandom();
    if(NextFadeEvent)
    {
        color4 newColor = static_cast<HoEffectEventColor *>(NextFadeEvent)->Get().GetRandom();
        float timeDelta = NextFadeEvent->GetStartTime() - GetStartTime();
        if(timeDelta == 0.f) timeDelta = 1.f;
        effectModel.m_ColorStep = (newColor - effectModel.m_Color)/timeDelta;
    }
}

void HoEffectEventSize::VisitEffectModel(HoEffectModelInterface &effectModel)
{
    if(!IsFade())
        effectModel.m_Size = m_Size.GetRandom();
    if(NextFadeEvent)
    {
        point3 newSize = static_cast<HoEffectEventSize *>(NextFadeEvent)->Get().GetRandom();
        float timeDelta = NextFadeEvent->GetStartTime() - GetStartTime();
        if(timeDelta == 0.f) timeDelta = 1.f;
        effectModel.m_SizeStep = (newSize - effectModel.m_Size)/timeDelta;
    }
}

HoEffectEventController::HoEffectEventController()
{
}

HoEffectEventController::~HoEffectEventController()
{
    Clear();
}

void HoEffectEventController::Clear()
{
    for (std::vector<HoEffectEventInterface *>::iterator i = m_EffectEvents.begin(); i != m_EffectEvents.end(); i++)
    {
        if (*i)
        delete (*i);
    }
    m_EffectEvents.clear();
}

bool HoEffectEventController::AddEvent(HoEffectEventInterface *event)
{
    m_EffectEvents.push_back(event);
    return true;
}

void HoEffectEventController::Init(HoEffectModelInterface *model)
{
    // An empty list leaves the model with no current event, as VC7 did (an empty vector's
    // begin() was a NULL pointer). HoEffectParticleController creates particles through its own,
    // empty controller and then runs them through its base class's, which holds the script's
    // events: a valid position in the empty list was walked against the other list's end(),
    // through unrelated memory - the crash on Destroyer's hit spark and other script effects.
    if ( m_EffectEvents.empty() ) {
        model->m_CurrentEvent = NULL;
        return;
    }

    std::vector<HoEffectEventInterface *>::iterator iter;
    for (iter = m_EffectEvents.begin();
         iter != m_EffectEvents.end() && !(*iter)->GetStartTime();
         iter++)
    {
        (*iter)->VisitEffectModel(*model);
    }
    model->m_CurrentEvent = iter;
}

bool HoEffectEventController::RunEvent(HoEffectModelInterface *model)
{
    if(model == NULL)
        return false;

    std::vector<HoEffectEventInterface *>::iterator i;
    for (i = model->m_CurrentEvent; i != m_EffectEvents.end() && (*i)->GetStartTime() <= model->m_fEventTimer; i++)
    {
        if( (*i)->GetClassID() != CLASS_EVENT_NONE)
        {
            float oldeventtimer = model->m_fEventTimer;
            (*i)->VisitEffectModel(*model);
            if (model->m_fEventTimer != oldeventtimer)
            {
                std::vector<HoEffectEventInterface *>::iterator RecalcIter;
                for (RecalcIter = m_EffectEvents.begin();
                RecalcIter != m_EffectEvents.end() && ((*RecalcIter)->GetStartTime()) < model->m_fEventTimer;
                RecalcIter++);

                i = RecalcIter-1;
            }
        }
    }
    model->m_CurrentEvent = i;
    return true;
}
void HoEffectEventController::CreateFadeList()
{
    HoEffectEvent_CompareFunc cf;
    std::sort(m_EffectEvents.begin(), m_EffectEvents.end(), cf);

    std::vector<HoEffectEventInterface *>::iterator iterStart;
    std::vector<HoEffectEventInterface *>::iterator iterEnd;
    for(iterStart = m_EffectEvents.begin(); iterStart != m_EffectEvents.end(); iterStart++)
    {
        for ( iterEnd = iterStart; iterEnd != m_EffectEvents.end(); iterEnd++)
        {
            if( iterEnd != iterStart && (*iterEnd)->IsFade())
            {
                if( (*iterEnd)->GetClassID() == (*iterStart)->GetClassID())
                {
                    (*iterStart)->SetFadeNextEvent(*iterEnd);
                    break;
                }
            }
        }
    }
}

bool HoEffectModelControllerInterface::InitEvent(HoEffectModelInterface *model)
{
    m_EffectEventController.Init(model);
	return true;
}

bool HoEffectModelControllerInterface::AddEvent(HoEffectEventInterface *event)
{
    m_EffectEventController.AddEvent(event);
    return true;
}

bool HoEffectModelControllerInterface::RunEvent(HoEffectModelInterface *model)
{
    m_EffectEventController.RunEvent(model);
    return true;
}

void HoEffectModelControllerInterface::CreateFadeList()
{
    m_EffectEventController.CreateFadeList();
}

void HoEffectMeshController::Init(const std::string &meshName)
{
    m_MeshModel.Init(meshName);
}

void HoEffectMeshController::Init(const std::string &meshName, const std::string &boneName)
{
    m_MeshModel.Init(meshName, boneName);
}

void HoEffectMeshController::InitMaxFrame(float frame)
{
    m_MeshModel.m_iMaxFrame = int(frame*160);
}

void HoEffectMeshController::InitLoop(int loop)
{
    m_iLoop = loop;
}

void HoEffectMeshController::InitColor(float r, float g, float b, float a)
{
    m_MeshModel.m_Color.r = r;
    m_MeshModel.m_Color.g = g;
    m_MeshModel.m_Color.b = b;
    m_MeshModel.m_Color.a = a;
}

void HoEffectMeshController::InitStartDelayTime(float delayTime)
{
    m_MeshModel.m_fStartDelayTime = delayTime;
}
void HoEffectMeshController::InitBlendType(int type)
{
    m_MeshModel.SetBlendType(type);
}

bool HoEffectMeshController::Main(float elapsedTime)
{
    if(m_bLiveFlag == false)
        return false;
	
	if(m_MeshModel.m_fStartDelayTime <= m_MeshModel.m_fTimer)
    {
        m_MeshModel.m_fCurrentFrame += (160.f*30.f)*elapsedTime;
        m_MeshModel.m_fEventTimer   += elapsedTime;
        m_MeshModel.m_Color         += m_MeshModel.m_ColorStep * elapsedTime;

        if(m_MeshModel.m_fCurrentFrame >= (float)m_MeshModel.m_iMaxFrame)
        {
            m_MeshModel.m_fCurrentFrame = 0.f;
            m_MeshModel.m_fEventTimer  = 0.f;
            m_MeshModel.m_CurrentEvent = NULL;

            m_iLoopCount++;
            if(m_iLoop > 0 && m_iLoopCount >= m_iLoop)
			{
                m_MeshModel.m_bLiveFlag = false;
				m_bLiveFlag = false;
			}
        }

        if(m_MeshModel.m_CurrentEvent == NULL)
            InitEvent(&m_MeshModel);
        else
            RunEvent(&m_MeshModel);
    }

    m_MeshModel.m_fTimer += elapsedTime;
    return true;
}

bool HoEffectMeshController::UpdateView(int x, int y, int z, int ax, int ay, int az)
{
    if(m_MeshModel.m_fStartDelayTime <= m_MeshModel.m_fTimer)
	{
		m_MeshModel.SetTranslate(m_CalcTranslate);

		point3 angle;
		angle.x = m_CalcAngle.x;
		angle.y = (float)((-(int)m_CalcAngle.y+ANGLE_180)&ANGCLIP);
		angle.z = m_CalcAngle.z;
		m_MeshModel.SetAngle(angle);
        m_MeshModel.UpdateView(x, y, z, ax, ay, az);
	}

    return true;
}

void HoEffectTextureController::SetLive(bool flag)
{
    m_TextureModel.SetLive(flag);
}
void HoEffectTextureController::Init(int type)
{
    m_TextureModel.SetClassID(type);
}

void HoEffectTextureController::Init(const std::string &textureName)
{
    m_TextureModel.Init(textureName);
}

void HoEffectTextureController::Init(const std::string &textureName, int textureCount, float aniDelayTime)
{
    m_TextureModel.Init(textureName, textureCount, aniDelayTime);
}

void HoEffectTextureController::InitBlendType(int type)
{
    m_TextureModel.m_iBlendType = type;
}

void HoEffectTextureController::InitEndTime(float endTime)
{
    m_TextureModel.m_fEndTime = endTime;
}

void HoEffectTextureController::InitLoop(int loop)
{
    m_iLoop = loop;
}

void HoEffectTextureController::InitColor(float r, float g, float b, float a)
{
    m_TextureModel.m_Color.r = r;
    m_TextureModel.m_Color.g = g;
    m_TextureModel.m_Color.b = b;
    m_TextureModel.m_Color.a = a;
}

void HoEffectTextureController::InitStartDelayTime(float delayTime)
{
    m_TextureModel.m_fStartDelayTime = delayTime;
}

void HoEffectTextureController::InitSize(float x, float y)
{
    m_TextureModel.m_Size.x = x;
    m_TextureModel.m_Size.y = y;
}

bool HoEffectTextureController::Main(float elapsedTime)
{
    if(m_bLiveFlag == false)
        return false;
 	if(m_TextureModel.m_fStartDelayTime <= m_TextureModel.m_fTimer)
    {
        m_TextureModel.m_Color       += m_TextureModel.m_ColorStep * elapsedTime;
		m_TextureModel.m_Size        += m_TextureModel.m_SizeStep  * elapsedTime;
        m_TextureModel.m_fEventTimer += elapsedTime;
		m_TextureModel.SetTranslate(m_CalcTranslate);

        if(m_TextureModel.m_fEventTimer >= m_TextureModel.m_fEndTime)
        {
            m_TextureModel.m_fEventTimer  = 0.f;
            m_TextureModel.m_CurrentEvent = NULL;
            m_iLoopCount++;
            if(m_iLoop > 0 && m_iLoopCount >= m_iLoop)
			{
                m_bLiveFlag = false;
				m_TextureModel.m_bLiveFlag = false;
			}
        }

        if(m_TextureModel.m_CurrentEvent == NULL)
            m_EffectEventController.Init(&m_TextureModel);
        else
            m_EffectEventController.RunEvent(&m_TextureModel);
        m_TextureModel.m_TextureNameProperty.Main(elapsedTime);
    }
	m_TextureModel.m_fTimer += elapsedTime;
    return true;
}

bool HoEffectTextureController::UpdateView(int x, int y, int z, int ax, int ay, int az)
{
    if(m_TextureModel.m_bLiveFlag == false)
        return false;
	m_TextureModel.SetTranslate(m_CalcTranslate);
    m_TextureModel.UpdateView(x, y, z, ax, ay, az);
    return true;
}

void HoEffectBillboardAxialController::Init(const std::string &textureName)
{
    m_BillboardAxialModel.Init(textureName);
}


void HoEffectBillboardAxialController::Init(const std::string &textureName, int textureCount, float aniDelayTime)
{
    m_BillboardAxialModel.Init(textureName, textureCount, aniDelayTime);
}

void HoEffectBillboardAxialController::InitPos(float x1, float y1, float z1, float x2, float y2, float z2)
{
    m_BillboardAxialModel.m_CurPos.x = x1;
    m_BillboardAxialModel.m_CurPos.y = y1;
    m_BillboardAxialModel.m_CurPos.z = z1;

    m_BillboardAxialModel.m_DesPos.x = x2;
    m_BillboardAxialModel.m_DesPos.y = y2;
    m_BillboardAxialModel.m_DesPos.z = z2;
}

void HoEffectBillboardAxialController::InitBlendType(int type)
{
    m_BillboardAxialModel.m_iBlendType = type;
}

void HoEffectBillboardAxialController::InitEndTime(float endTime)
{
    m_BillboardAxialModel.m_fEndTime = endTime;
}

void HoEffectBillboardAxialController::InitLoop(int loop)
{
    m_iLoop = loop;
}

void HoEffectBillboardAxialController::InitColor(float r, float g, float b, float a)
{
    m_BillboardAxialModel.m_Color.r = r;
    m_BillboardAxialModel.m_Color.g = g;
    m_BillboardAxialModel.m_Color.b = b;
    m_BillboardAxialModel.m_Color.a = a;
}

void HoEffectBillboardAxialController::InitStartDelayTime(float delayTime)
{
    m_BillboardAxialModel.m_fStartDelayTime = delayTime;
}

void HoEffectBillboardAxialController::InitSize(float x, float y)
{
    m_BillboardAxialModel.m_Size.x = x;
    m_BillboardAxialModel.m_Size.y = y;
}

bool HoEffectBillboardAxialController::Main(float elapsedTime)
{
    if(m_bLiveFlag == false)
        return false;
	if(m_BillboardAxialModel.m_fStartDelayTime <= m_BillboardAxialModel.m_fTimer)
    {
        m_BillboardAxialModel.m_Color       += m_BillboardAxialModel.m_ColorStep * elapsedTime;
        m_BillboardAxialModel.m_fEventTimer += elapsedTime;
        m_BillboardAxialModel.SetTranslate(m_CalcTranslate);
        
         if(m_BillboardAxialModel.m_fEventTimer >= m_BillboardAxialModel.m_fEndTime)
        {
            m_BillboardAxialModel.m_fEventTimer  = 0.f;
            m_BillboardAxialModel.m_CurrentEvent = NULL;
            m_iLoopCount++;
            if(m_iLoop > 0 && m_iLoopCount >= m_iLoop)
			{
				m_bLiveFlag = false;
				m_BillboardAxialModel.m_bLiveFlag = false;
			}
        }

        if(m_BillboardAxialModel.m_CurrentEvent == NULL)
            m_EffectEventController.Init(&m_BillboardAxialModel);
        else
            m_EffectEventController.RunEvent(&m_BillboardAxialModel);
        m_BillboardAxialModel.m_TextureNameProperty.Main(elapsedTime);
    }
    m_BillboardAxialModel.m_fTimer += elapsedTime;
    return true;
}

bool HoEffectBillboardAxialController::UpdateView(int x, int y, int z, int ax, int ay, int az)
{
    if(m_BillboardAxialModel.m_bLiveFlag == false)
        return false;
    m_BillboardAxialModel.SetTranslate(m_CalcTranslate);
	m_BillboardAxialModel.UpdateView(x, y, z, ax, ay, az);
    return true;
}

HoEffectParticleController::HoEffectParticleController() : m_fTimer(0),
                                                           m_fEmitRate(30.f),
                                                           m_iLoop(1),
                                                           m_fEmitRateSum(0.f),
                                                           m_fParticleNum(50.f),
                                                           m_fParticleTotalNum(0.f),
                                                           m_fParticleLiveNum(0.f),
                                                           m_iBlendType(SMMAT_BLEND_LAMP),
                                                           m_EmitRange(NULL)
{
    m_fEndTime.Min = 1.f;
    m_fEndTime.Max = 2.f;


    m_Color.Max.r = 255.f;
    m_Color.Min.r = 255.f;
    m_Color.Max.g = 255.f;
    m_Color.Min.g = 255.f;
    m_Color.Max.b = 255.f;
    m_Color.Min.b = 255.f;
    m_Color.Max.a = 255.f;
    m_Color.Min.a = 255.f;

    m_Velocity.Max.x =  10.f;
    m_Velocity.Min.x = -10.f;
    m_Velocity.Max.y =  10.f;
    m_Velocity.Min.y = -10.f;
    m_Velocity.Max.z =  10.f;
    m_Velocity.Min.z = -10.f;

    m_Size.Max.x = 10.f;
    m_Size.Min.x = 5.f;
    m_Size.Max.y = 10.f;
    m_Size.Min.y = 5.f;
    m_Size.Max.z = 10.f;
    m_Size.Min.z = 5.f;

    m_ParticleType = PARTICLE_BILLBOARD_DEFAULT;
	m_VelocityType = SPAWN_VELOCITY_RANDOM;
}

HoEffectParticleController::~HoEffectParticleController()
{
    if(m_EmitRange)
    {
        delete m_EmitRange;
        m_EmitRange = NULL;
    }
}

void HoEffectParticleController::Init(const std::string &fileName)
{
    m_TextureName = fileName;
}

void HoEffectParticleController::InitSize(float x, float y)
{
    m_Size.Min.x = x;
    m_Size.Max.x = x;
    m_Size.Min.y = y;
    m_Size.Max.y = y;
    m_Size.Min.z = y;
    m_Size.Max.z = y;
}

void HoEffectParticleController::InitSize(float x1, float x2, float y1, float y2)
{
    m_Size.Min.x = x1;
    m_Size.Max.x = x2;
    m_Size.Min.y = y1;
    m_Size.Max.y = y2;
}

void HoEffectParticleController::InitSpawnBoundingBox(float x1, float x2, float y1, float y2, float z1, float z2)
{
    if(m_EmitRange == NULL)
        m_EmitRange = new HoEffectBoundingBox;
    static_cast<HoEffectBoundingBox *>(m_EmitRange)->Init(x1, x2, y1, y2, z1, z2);
}

void HoEffectParticleController::InitSpawnBoundingSphere(float x1, float x2)
{
    if(m_EmitRange == NULL)
        m_EmitRange = new HoEffectBoundingSphere;
    static_cast<HoEffectBoundingSphere *>(m_EmitRange)->Init(x1, x2);
}

void HoEffectParticleController::InitSpawnBoundingDoughnut(float x1, float x2, float y1, float y2)
{
    if(m_EmitRange == NULL)
        m_EmitRange = new HoEffectBoundingDoughnut;
    static_cast<HoEffectBoundingDoughnut *>(m_EmitRange)->Init(x1, x2, y1, y2);
}

void HoEffectParticleController::InitVelocity(float x1, float x2, float y1, float y2, float z1, float z2)
{
    m_Velocity.Min.x =  x1;
    m_Velocity.Max.x =  x2;
    m_Velocity.Min.y =  y1;
    m_Velocity.Max.y =  y2;
    m_Velocity.Min.z =  z1;
    m_Velocity.Max.z =  z2;
}
void HoEffectParticleController::InitParticleNum(float particleNum)
{
    m_fParticleNum = particleNum;
}

void HoEffectParticleController::InitEmitRate(float emitRate)
{
    m_fEmitRate = emitRate;
}

void HoEffectParticleController::InitLoop(int loop)
{
    m_iLoop = loop;
}

void HoEffectParticleController::InitEndTime(float x1, float x2)
{
    m_fEndTime.Min = x1;
    m_fEndTime.Max = x2;
}

void HoEffectParticleController::InitColor(float r, float g, float b, float a)
{
    m_Color.Max.r = r;
    m_Color.Min.r = r;
    m_Color.Max.g = g;
    m_Color.Min.g = g;
    m_Color.Max.b = b;
    m_Color.Min.b = b;
    m_Color.Max.a = a;
    m_Color.Min.a = a;
}

void HoEffectParticleController::InitBlendType(int type)
{
    m_iBlendType = type;
}

void HoEffectParticleController::CreateNewParticle(HoEffectTextureModel &part)
{
     //파티클 생성
     part.m_bLiveFlag = true;
     if(m_EmitRange)
        part.m_Translate = m_EmitRange->GetPos();

     part.m_fTimer   = 0.f;
     part.m_fEndTime = m_fEndTime.GetRandom();
     part.m_Color    = m_Color.GetRandom();
     part.m_Size     = m_Size.GetRandom();
     part.m_TextureNameProperty.Init(m_TextureName);
     if(m_VelocityType == SPAWN_VELOCITY_RANDOM)
     {
        part.m_TranslateStep = m_Velocity.GetRandom();
        if(m_ParticleType == PARTICLE_BILLBOARD_AXIAL)
        {
            part.m_iClassID = CLASS_EFFECT_BILLBOARD_AXIAL;
            part.m_Direction = m_Velocity.GetRandom().Normalized();
        }
     }
     else if(m_VelocityType == SPAWN_VELOCITY_CURPOS)
     {
        point3 velocity = m_Velocity.GetRandom();
        part.m_TranslateStep = part.m_Translate.Normalized() * velocity.x;
        if(m_ParticleType == PARTICLE_BILLBOARD_AXIAL)
        {
            part.m_iClassID = CLASS_EFFECT_BILLBOARD_AXIAL;
            if(velocity.x < 0)
                part.m_Direction = part.m_Translate.Normalized() * -1.f;
            else
                part.m_Direction = part.m_Translate.Normalized();
        }
     }

	 point3 angle;
	 angle.x = m_CalcAngle.x;
	 angle.y = float(int(m_CalcAngle.y + ANGLE_180)&ANGCLIP);
	 angle.z = m_CalcAngle.z;

	 //약간 땜빵이다..
	 GetMoveLocation(int(m_Translate.x * 256.f), int(m_Translate.y * 256.f), int(m_Translate.z * 256.f),
     		 (int)angle.x, (int)angle.y, (int)angle.z);
     point3 localNew;
	 localNew.x = (float)GeoResult_X/256.f;
	 localNew.y = (float)GeoResult_Y/256.f;
	 localNew.z = (float)GeoResult_Z/256.f;

	  point3 local = m_CalcTranslate - m_Translate;
 	 part.m_Translate += (local + localNew);
	 
}


bool HoEffectParticleController::Main(float elapsedTime)
{
    if(m_bLiveFlag)
    {
        m_fTimer += elapsedTime;
        float spawnPartNum1 = m_fEmitRate*elapsedTime;
        if(m_fEmitRateSum >= 1.f)
        {
            m_fEmitRateSum -= 1.f;
            spawnPartNum1  += 1.f;
        }
        int   spawnPartNum2 = int(spawnPartNum1);
        m_fParticleTotalNum += (float)spawnPartNum2;

        if(m_iLoop > 0)
        {
            if(m_fParticleTotalNum >= m_fParticleNum * (float)m_iLoop)
            {
                spawnPartNum2 = 0;
                if(m_fParticleLiveNum == 0)
                    m_bLiveFlag = false;
            }
        }

        m_fEmitRateSum += (spawnPartNum1 - spawnPartNum2);
        if(m_fParticleNum <= m_fParticleLiveNum)
            m_fEmitRateSum = 0.f;
        for(int i = 0; i < MAX_PARTICLE_LIST; i++)
        {
            if(m_ParticleList[i].m_bLiveFlag)
            {
                m_ParticleList[i].m_Translate   += m_ParticleList[i].m_TranslateStep  * elapsedTime;
				m_ParticleList[i].m_Color       += m_ParticleList[i].m_ColorStep      * elapsedTime;
                m_ParticleList[i].m_fEventTimer += elapsedTime;
                m_ParticleList[i].m_fTimer      += elapsedTime;

                if(m_ParticleList[i].m_CurrentEvent == NULL)
                    InitEvent(&m_ParticleList[i]);
                else
                    RunEvent(&m_ParticleList[i]);

                if(m_ParticleList[i].m_fEndTime <= m_ParticleList[i].m_fTimer)
                {
                    m_ParticleList[i].Init();
                    m_fParticleLiveNum--;
                }
            }

            if(spawnPartNum2 > 0)
            {
                if(m_ParticleList[i].m_bLiveFlag == false)
                {
                    CreateNewParticle(m_ParticleList[i]);
                    m_fParticleLiveNum++;
                    spawnPartNum2--;
                    m_EffectEventController.Init(&m_ParticleList[i]);
                }
            }
        }
    }
    return true;
}

bool HoEffectParticleController::UpdateView(int x, int y, int z, int ax, int ay, int az)
{
    for(int i = 0; i < MAX_PARTICLE_LIST; i++)
    {
        if(m_ParticleList[i].m_bLiveFlag)
            m_ParticleList[i].UpdateView(x, y, z, ax, ay, az);
    }
    
    return true;
}

void HoEffectParticleController::InitVelocityType(int type)
{
    m_VelocityType = type;
}
void HoEffectParticleController::InitParticleType(int type)
{
    m_ParticleType = type;
}

HoEffectGroup::HoEffectGroup() : m_Translate(0.f, 0.f, 0.f),
								 m_Angle(0.f, 0.f, 0.f),
                                 m_bLiveFlag(false)
{
}
HoEffectGroup::~HoEffectGroup()
{
    Clear();
}

void HoEffectGroup::Clear()
{
    for (std::vector<HoEffectModelControllerInterface *>::iterator i = m_ControllerList.begin(); i != m_ControllerList.end(); i++)
    {
        if (*i)
        {
            delete (*i);
            (*i) = NULL;
        }
    }
    m_ControllerList.clear();
}

void HoEffectGroup::CreateFadeList()
{
    for (std::vector<HoEffectModelControllerInterface *>::iterator i = m_ControllerList.begin(); i != m_ControllerList.end(); i++)
    {
        if(*i)
            (*i)->CreateFadeList();
    }
}

bool HoEffectGroup::Main(float elapsedTime)
{
	if(!m_bLiveFlag)
		return false;

	int DeadCount = 0;
	for(unsigned int i = 0; i < m_ControllerList.size(); i++)
    {
        if(m_ControllerList[i]->IsLive())
			m_ControllerList[i]->Main(elapsedTime);
		else
			DeadCount++;
	}
	if(DeadCount == m_ControllerList.size())
		m_bLiveFlag = false;
	return true;
}

void HoEffectGroup::SetCalcTranslate(const point3 *trans, const point3 *ang)
{
	for(unsigned int i = 0; i < m_ControllerList.size(); i++)
    {
        if(m_ControllerList[i]->IsLive())
		{
			if(trans != NULL)
			{
				m_Translate = *trans;
				m_ControllerList[i]->SetCalcTranlsate(*trans);
			}
			if(ang != NULL)
			{
				m_Angle = *ang;
				m_ControllerList[i]->SetCalcAngle(*ang);
			}
		}
	}
}

bool HoEffectGroup::UpdateView(int x, int y, int z, int ax, int ay, int az)
{
    //lpD3DDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, FALSE);
    for(unsigned int i = 0; i < m_ControllerList.size(); i++)
        m_ControllerList[i]->UpdateView(x, y, z, ax, ay, az);
    //lpD3DDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, TRUE);

    return true;
}

HoEffectModelControllerInterface *HoEffectGroup::CreateFactory(int effectType)
{
    HoEffectModelControllerInterface *effectModelController = NULL;
    if(effectType == CLASS_EFFECT_MESH)
        effectModelController = new HoEffectMeshController();
    else if(effectType == CLASS_EFFECT_TEXTURE)
        effectModelController = new HoEffectTextureController();
    else if(effectType == CLASS_EFFECT_BILLBOARD_AXIAL)
        effectModelController = new HoEffectBillboardAxialController();
    else if(effectType == CLASS_EFFECT_PARTICLE_SYSTEM)
        effectModelController = new HoEffectParticleController();

    return effectModelController;
}

bool HoEffectGroup::AddModelController(HoEffectModelControllerInterface *modelController)
{
    if(modelController == NULL)
        return false;
    m_ControllerList.push_back(modelController);
	m_bLiveFlag = true;
    return true;
}




