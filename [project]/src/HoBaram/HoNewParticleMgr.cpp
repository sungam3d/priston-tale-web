//#include "smLib3d\\smd3d.h"
#include "HoLinkHeader.h"

//#include "..\\particle.h"
#include "HoNewParticleLinkHeader.h"
/*
#include "HoMinMax.h"
#include "HoNewParticle.h"
#include "HoNewParticleMgr.h"
*/

HoNewParticleResMgr  g_NewParticleResMgr;
HoNewParticleMgr     g_NewParticleMgr;

void HoNewParticleResMgr::init()
{
    MatRes.clear();
    ScriptRes.clear();

    MatRes.reserve(MAX_MATERIAL);
	ScriptRes.reserve(MAX_TOKEN_VECTORS);

    NumScriptRes = 0;
    NumMaterial = 0;
    
}

/*
void HoNewParticleResMgr::clear()
{
	for(int index = 0; index < ScriptRes[index].size(); index++)
	{
		ScriptRes[index].
	}
}
*/

HoNewParticleScriptRes *HoNewParticleResMgr::FindScript(char *find)
{
	
	if(ScriptRes.size() == 0)
		return NULL;

    for(int index = 0; index < NumScriptRes; index++)
    {
        //if(ScriptRes[index].name.compare(find) == 0)
		char fileName[256];
		memset(fileName, 0, sizeof(fileName));
		strcpy(fileName, ScriptRes[index].name.c_str());
		
		if( stricmp(fileName, find) == 0)
            return &ScriptRes[index];
    }
	

    return NULL;
}

bool HoNewParticleResMgr::LoadScript(char *scr)
{
	if(scr == NULL)
		return false;
	
	
	char particleBuffer[6000];
	memset(particleBuffer, 0, sizeof(particleBuffer));
	FILE *fp;
	fp = fopen(scr, "r+t");
	if(fp == NULL)
		return false;
	fread(particleBuffer, sizeof(particleBuffer), 1, fp);
	fclose(fp);
	
	char directoryBuffer[1024];
	char fileBuffer[256];
	memset(directoryBuffer, 0, sizeof(directoryBuffer));
	memset(fileBuffer, 0, sizeof(fileBuffer));

	strcpy(directoryBuffer, scr);
	
	for(int index = (strlen(directoryBuffer)-1); index > 0; index--)
	{
		if(directoryBuffer[index] == '.')
		{
			directoryBuffer[index] = '\0';
			break;
		}
	}

    for(int index = (strlen(directoryBuffer)-1); index > 0; index--)
	{
		if(directoryBuffer[index] == '\\')
		{
			strcpy(fileBuffer, &directoryBuffer[index+1]);
			break;
		}
	}

	AddScript(particleBuffer, fileBuffer);
	return true;
}

HoNewParticleScriptRes *HoNewParticleResMgr::AddScript(char *scr, char *indexName)
{
    if(MAX_TOKEN_VECTORS <= NumScriptRes)
        return NULL;

	if(FindScript(indexName)!= NULL)
		return NULL;

    HoNewParticleScriptRes particleScriptRes;
    //particleScriptRes.script.reserve(strlen(scr));
    particleScriptRes.name.reserve(strlen(indexName));

    //particleScriptRes.script = scr;
    particleScriptRes.name = indexName;
    particleScriptRes.emitterTokenizer.Tokenize(scr);

    ScriptRes.push_back(particleScriptRes);
    NumScriptRes++;

    return &ScriptRes[NumScriptRes-1];
}

int HoNewParticleResMgr::SearchingAddTexture(const char *directory)
{
	HANDLE hSrch;
    WIN32_FIND_DATA wfd;
    BOOL    bResult = TRUE;
    hSrch = FindFirstFile(directory, &wfd);
    char drive[200];
    char dir[200];
    //char newpath[256];
    //char fileFullName[256];
    hSrch = FindFirstFile(directory, &wfd);

    _splitpath(directory,drive,dir,NULL,NULL);
    while(bResult)
    {
		if(!(wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
		{
			if(strlen(wfd.cFileName) > 3)
			{
				//wsprintf(fileFullName, "%s%s",dir, wfd.cFileName);
				//wsprintf(fileFullName, "%s%s","effect\\\\particle\\\\", wfd.cFileName);
				std::string texBuffer;
				texBuffer = wfd.cFileName;//fileFullName;
				MakeUpperCase(texBuffer);
				AddTexture(texBuffer.c_str());
			}

		}
		
		bResult = FindNextFile(hSrch, &wfd);
    }
    FindClose(hSrch);

	return TRUE;
}

int HoNewParticleResMgr::AddTexture(const char *mat)
{
    if(MAX_MATERIAL <= NumMaterial)
        return -1;			// not drawn (false = 0 drew material 0, a hit spark)
	
	
	char testBuffer[256];
	memset(testBuffer, 0, sizeof(testBuffer));

	for(int index = 0; index < NumMaterial; index++)
	{
 		strcpy(testBuffer, MatRes[index].matName.c_str());
        
		if(strcmp(MatRes[index].matName.c_str(), mat) == 0)
		//if(MatRes[index].matName.compare(mat) == 0)
               return MatRes[index].matNum;
    }

    char buffer[256];
    char FullDirectoryName[500];
	memset(buffer, 0, sizeof(buffer));
    strcpy(buffer, mat);
	
	HoNewParticleMatRes partScriptRes;
	partScriptRes.matName = buffer;
	MakeUpperCase(partScriptRes.matName);
	
	wsprintf(FullDirectoryName, "%s%s", "effect\\particle\\", mat);
	partScriptRes.matNum = CreateTextureMaterial( FullDirectoryName, 0, 0, 0,0, SMMAT_BLEND_LAMP);
	
    MatRes.push_back(partScriptRes);
	if(MatRes[NumMaterial].matNum != -1)
    {
		//ReadTextures();
		smMaterialGroup->ReadTextures();
        NumMaterial++;
        return MatRes[NumMaterial-1].matNum;
    }
    return -1;
}

void HoNewParticleMgr::Main(float elapsedTime, float timeDelta)
{
    std::vector<HoNewParticleEmitter>::iterator i;
    for (i = Emitters.begin(); i != Emitters.end();++i)
    {
        if((*i).IsRunning())
        {
            (*i).Main(elapsedTime, timeDelta);
        }
    }
}

void HoNewParticleMgr::Draw(int x, int y, int z, int ax, int ay, int az)
{
    std::vector<HoNewParticleEmitter>::iterator i;

	
    
	for (i = Emitters.begin(); i != Emitters.end();++i)
    {
        if((*i).IsRunning())
        {
            (*i).Draw(x, y, z, ax, ay, az);
        }
    }
}

int HoNewParticleMgr::Start(char *fileName, POINT3D pos, float startDelay)
{
	
    HoNewParticleScriptRes *scriptRes = g_NewParticleResMgr.FindScript(fileName);
    if(scriptRes != NULL)
    {
        return Start(scriptRes, pos, startDelay);
    }

    return -1;
}

int HoNewParticleMgr::Start(HoNewParticleScriptRes *tokenRes, POINT3D pos, float startDelay)
{
	D3DVECTOR particlePos;
	particlePos.x = (float)pos.x;
	particlePos.y = (float)pos.y;
	particlePos.z = (float)pos.z;
	
	if(Emitters.size() >= MAX_EMITTER)
    {
		int count = 0;
		vector<HoNewParticleEmitter>::iterator i;
		for (i = Emitters.begin(); i != Emitters.end(); ++i)
		{
			if(!(*i).IsRunning())
			{
				(*i).AddScript(tokenRes);
				#ifdef _HO_WIN_32_
					(*i).SetPos(particlePos);
				#endif
					(*i).Start(startDelay);

				return count;
			}
			count++;
		}
	}
	
	HoNewParticleEmitter emitter;
	emitter.AddScript(tokenRes);
	#ifdef _HO_WIN_32_
		emitter.SetPos(particlePos);
	#endif
		emitter.Start(startDelay);

	if(NumEmitters < MAX_EMITTER)
	{
		Emitters.push_back(emitter);
		NumEmitters++;
		return (NumEmitters-1);
	}
	return (-1);
}

void HoNewParticleMgr::SetPos(int partID, D3DVECTOR pos)
{
	if(partID < 0 || partID >= MAX_EMITTER)
		return;

	if(Emitters[partID].IsRunning())
		Emitters[partID].SetPos(pos);
}

void HoNewParticleMgr::SetAngle(int partID, POINT3D angle)
{
	if(partID < 0 || partID >= MAX_EMITTER)
		return;

	if(Emitters[partID].IsRunning())
	{
		D3DVECTOR tempAngle;
		tempAngle.x = (float)angle.x;
		tempAngle.y = (float)angle.y;
		tempAngle.z = (float)angle.z;

		Emitters[partID].SetAngle(tempAngle);
	}
}

bool HoNewParticleMgr::GetRunning(int partID)
{
	if(partID < 0 || partID >= MAX_EMITTER)
		return false;
	if(Emitters[partID].IsRunning())
		return true;

	return false;
}


void HoNewParticleMgr::SetAttachPos(int partID, D3DVECTOR pos)
{
	if(partID < 0 || partID >= MAX_EMITTER)
		return;

	if(Emitters[partID].IsRunning())
		Emitters[partID].SetAttachPos(pos);
}

void HoNewParticleMgr::SetStop(int partID)
{
	if(partID < 0 || partID >= MAX_EMITTER)
		return;
	if(Emitters[partID].IsRunning())
		Emitters[partID].FadeStop();
}

void HoNewParticleMgr::SetFastStop(int partID)
{
	if(partID < 0 || partID >= MAX_EMITTER)
		return;
	if(Emitters[partID].IsRunning())
		Emitters[partID].FastStop();
}

void HoNewParticleMgr::SetRendering(int partID, BOOL flag)
{
	if(partID < 0 || partID >= MAX_EMITTER)
		return;
	if(Emitters[partID].IsRunning())
	{
		Emitters[partID].SetRendering(flag);
	}
}

void HoNewParticleMgr::Init()
{
	NumEmitters = 0;
	Emitters.clear();
	Emitters.reserve(MAX_EMITTER+1);
	Running = false;
}

void HoNewParticleMgr::Clear()
{
	vector<HoNewParticleEmitter>::iterator i;
	for (i = Emitters.begin(); i != Emitters.end(); ++i)
	{
		(*i).init();
	}
}

void HoNewParticleMgr::Destroy()
{

	vector<HoNewParticleEmitter>::iterator i;
	for (i = Emitters.begin(); i != Emitters.end(); ++i)
	{
		(*i).init();
	}
}







