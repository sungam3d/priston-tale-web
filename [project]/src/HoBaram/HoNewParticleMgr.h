#ifndef _HO_NEW_PARTICLE_MGR_
#define _H0_NEW_PARTICLE_MGR_

#include <vector>
#include <string>


using namespace std;

struct HoNewParticleMatRes
{
    int     matNum;
    string  matName;
};

#define MAX_TOKEN_VECTORS   250
#define MAX_MATERIAL        1024	// textures of all particle scripts (was 150: later ones, e.g. the opening's leaves, got none)
#define MAX_EMITTER         150

class HoNewParticleScriptRes
{
public:
    string name;
    //string script;
    HoNewParticleEmitterTokenizer emitterTokenizer;
};

//스크립트 해석기
class HoNewParticleResMgr
{
    public:
        HoNewParticleResMgr()
        {
            init();
        }

        ~HoNewParticleResMgr()
        {

        }

    private:

        int NumScriptRes;
        std::vector<HoNewParticleScriptRes> ScriptRes; //스크립트 정보들..

        int NumMaterial;
        std::vector<HoNewParticleMatRes > MatRes; //리소스 정보..

    public:
        void init();
		//void clear();
		bool LoadScript(char *scr);
        HoNewParticleScriptRes *AddScript(char *scr, char *indexName = "test");
        HoNewParticleScriptRes *FindScript(char *find);

        int AddTexture(const char *mat);
		int SearchingAddTexture(const char *directory);
};

class HoNewParticleMgr
{
    public:
        HoNewParticleMgr()
        {
            NumEmitters = 0;
			Emitters.clear();
            Emitters.reserve(MAX_EMITTER);
            Running = false;
			//Rendering = true;
        }

        ~HoNewParticleMgr()
        {
            NumEmitters = 0;
			Destroy();
		}

    private:
        int     NumEmitters;
        std::vector<HoNewParticleEmitter> Emitters;   //실제로 동작을 하는

    public:
		void	Init();  //변수 초기화..
        bool    Running;
		void    Clear(); //메모리 초기화.

		void    Destroy();

		//bool    Rendering; //Rendering Flag;

        void    Main(float elapsedTime, float timeDelta);
        void    Draw(int x, int y, int z, int ax, int ay, int az);
        bool    IsRunning() { return Running; }
        
		int     Start(HoNewParticleScriptRes *scriptRes, POINT3D pos, float startDelay = 0.f);
        int     Start(char *fileName, POINT3D pos, float startDelay = 0.f);

		void    SetPos(int partID, D3DVECTOR pos);
		void    SetAttachPos(int partID, D3DVECTOR pos);
		void    SetStop(int partID);
		void    SetFastStop(int partID);
		void	SetAngle(int partID, POINT3D angle);

		bool	GetRunning(int partID);
		void    SetRendering(int partID, BOOL flag);

		
		
};

/*
//움직이는 이펙트들...
class HoNewEffectMove : public HoEffectObject
{
public:
	HoNewEffectMove();
	~HoNewEffectMove();
private:
	smPAT3D		*PatObj;
	int			CurrentFrame;
	POINT3D		DestPos;		//목적 위치..
	POINT3D     Angle;
	int			PartEmitterID;
	
	D3DVECTOR   Velocity;
	
	bool		ParticleStartFlag;
	smOBJ3D     *ObjBip;

	float       Length;
	POINT3D     OldPos;
	BOOL		BlendFlag;

	int         BlendStep;
	int			BlendCount;
	
	int			TimeCount;
public:
	int Start(POINT3D curPos, POINT3D destPos, smPAT3D *pat, int skillCode);
	int Start(POINT3D curPos, POINT3D destPos, int skillCode);
	int Start(POINT3D curPos, POINT3D destPos, POINT3D angle, smPAT3D *pat, int skillCode);

	int Main();
	int Draw(int x, int y, int z, int ax, int ay, int az);

};
*/

extern HoNewParticleResMgr  g_NewParticleResMgr; //파티클 리소스 메니저..
extern HoNewParticleMgr     g_NewParticleMgr; //파티클 에미터 메니저...

#endif
