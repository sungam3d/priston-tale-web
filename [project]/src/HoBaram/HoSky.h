#ifndef _HO_SKY_H_
#define _HO_SKY_H_

#include <vector>
#include <list>
#include <string>

#define		USER_SKY_ADD_CODE		10000						//코드값이 겹쳐서 더해주는 코드값 (하늘 코드값과 던젼 코드값이 겹쳐서)


//읽어 들인 리소스 정보
struct skyResInfo
{
	std::string		resName;			//읽어들일 리소스의 이름
	int				fadeSlowFlag;		//Fade를 늦게 하는 Sky 요소(Fade를 같이 할경우 메시의 라인이 보인다)
	smPAT3D			*stageSky;			//실제로 읽어 들인 리소스

	int colorR, colorG, colorB, colorA;

	skyResInfo()
	{
		fadeSlowFlag = 0;
		stageSky = NULL;
		colorR = 255;
		colorG = 255;
		colorB = 255;
		colorA = 255;
	}
};


//하늘 하나에 대한 Res들
class HoSkyDrawInfo
{
public:
	int LightColorR; 
	int LightColorG; 
	int LightColorB;
	int	LightDark;
	int	LightDelay;
	
	bool	FogFlag;
	int		FogColorR;
	int		FogColorG;
	int		FogColorB;
	int		FogColorA;

	int		FogStartPos;
	int		FogEndPos;

	int		ChangeSkyNum;
	int		ChangeSkyTime;
	BOOL	ChangeNextSkyEnd;
	
	BOOL	ClippingFlag;
	std::vector<skyResInfo> SkyResInfoList;

	HoSkyDrawInfo()
	{
		LightColorR = 0;
		LightColorG = 0;
		LightColorB = 0;
		LightDark   = 0;
		LightDelay  = 300;

		FogFlag = false;
		FogColorR = 0;
		FogColorG = 0;
		FogColorB = 0;
		FogColorA = 0;

		FogStartPos = 50;
		FogEndPos   = 5000;

		ClippingFlag = TRUE;
	}
	
	void	LoadRes();
	void	DestroyRes();

};

#define FIELD_BACKIMAGE_AURORA	0x200

class HoSky
{
	friend void HoSky_WebDiag( HoSky &S, int x, int y, int z, int ax, int ay, int az );	// HoSky.cpp (web port)
private:
	std::vector <HoSkyDrawInfo>		    SkyDrawInfoList;

	int							CodeID;
	int							DrawIndex;
	int							DrawOldIndex;

	
	int							DrawAlphaCount;
	int							DrawOldAlphaCount;

	int							DrawNextAlphaCount;			//제일 나중에 서서히 없어져야할 오브젝트
	int							DrawNextOldAlphaCount;
	
	
	BOOL						DrawNextFlag;
	int							TimeCount;
	
	int							FogStartColorR;
	int							FogStartColorG;
	int							FogStartColorB;
	int							FogStartColorA;

	int							FogEndColorR;
	int							FogEndColorG;
	int							FogEndColorB;
	int							FogEndColorA;

	int							FogStartPos1;
	int							FogStartPos2;

	int							FogEndPos1;
	int							FogEndPos2;
	int							FieldStartColorDelay;
	

	int							BlinkTime;
	int							BlinkColor;
	BOOL						BlinkFlag;
	BOOL						BlinkDrawFlag;
	int							BlinkCount;
	int							ParticleCount;
	int							SnowParticleNum;
	BOOL						StartSnowFlag;
public:
	int							FieldColorR, FieldColorG, FieldColorB;
	BOOL						FirstSkyLoadFlag;
	BOOL						TestModeFlag;

private:
	void		ParticleEffectSpawn();

public:
	HoSky();
	~HoSky();

	int			Main();
	int			Draw(int x, int y, int z, int ax, int ay, int az);
	int			Draw(int x, int y, int z, int ax, int ay, int az, unsigned int index, int alphaCount = 100, int alphaNextCount = 100); //alpha 100이면 색깔이 완전히 나오고 0이면 완전히 사라짐..
	
	void		Destroy();
	void		Create();
	
	void		LoadRes();
	
	
	void		DestroyRes();

	unsigned int	SetDrawIndex(unsigned int ID, int codeID);     //하늘 설정 파일 순서대로 로드 (0, 1, 2)
	int				GetCodeID() { return CodeID; }
	void			SetCodeID(int code) { CodeID = code; }

	unsigned int	SetNextDrawIndex();
	void			Init();

	void			SetFieldColor(int r, int g, int b, int startDelay = 0)
	{
		FieldColorR = r;
		FieldColorG = g;
		FieldColorB = b;

		FieldStartColorDelay = startDelay;
	}

	int			GetDarkLight()
	{
		if(DrawIndex == -1)
			return 0;
		
		return SkyDrawInfoList[DrawIndex].LightDark;
	}
};


extern int InitSky();
extern int CreateSky();
extern int ReCreateSky();

extern int DrawOpeningSky(int x, int y, int z, int ax, int ay, int az);
extern int DrawSky(int x, int y, int z, int ax, int ay, int az);
extern int MainSky();
extern int ChangeSky(int id);
extern int SetNextSky();
extern int DestroySky();


#endif