/*----------------------------------------------------------------------------*
*	파일명 :  sinCharStatus.h	
*	하는일 :  캐릭터 스테이터스의 모든설정을 관리한다 
*	작성일 :  최종업데이트 12월
*	적성자 :  박상열 
*-----------------------------------------------------------------------------*/	
#define MAX_CHAR_RECT_POSI			30  //캐릭터 스테이터스에서 쓰일 RECT의 MAX 

#define POINT_STRENGTH				1
#define POINT_SPIRIT				2
#define POINT_TALENT				3
#define POINT_DEXTERITY				4
#define POINT_HEALTH				5


#define SIN_TEXT_COLOR_WHITE		0
#define SIN_TEXT_COLOR_RED			1
#define SIN_TEXT_COLOR_YELLOW		2
#define SIN_TEXT_COLOR_BLUE			3
#define SIN_TEXT_COLOR_GREEN		4
#define SIN_TEXT_COLOR_GOLD			5
#define SIN_TEXT_COLOR_ORANGE		6
#define SIN_TEXT_COLOR_PINK			7

/*----------------------------------------------------------------------------*
*				(캐릭터 스테이터스)캐릭터 정보 구조체
*-----------------------------------------------------------------------------*/
struct sCHARRECTPOSI{

	char Para[64];
	RECT Rect;

};

struct sDISPLAYSTATE{
	short Damage[2];
	short PercentDamage[2];
	int   Absorb;
	int   Attack_Rate;
	int   Defense;
	DWORD   MaxTime;
	DWORD   Time;
	int   Flag;
};
/*----------------------------------------------------------------------------*
*				(캐릭터 스테이터스)class
*-----------------------------------------------------------------------------*/	
class cCHARSTATUS{

public:
	int OpenFlag;       //포인터 로 접근하기 때문에 위치가 절대루 바뀌면 안된다 
	int MatStatus[4];   //캐릭터 스테이터스 메트리얼 넘버를 저장한다 

	LPDIRECTDRAWSURFACE4    lpRegiBox[5];
	LPDIRECTDRAWSURFACE4    lpChangeArrow[2];
	LPDIRECTDRAWSURFACE4    lpSelectArrow[2];
	LPDIRECTDRAWSURFACE4	lpStatusPoint;


public:
	cCHARSTATUS();
	~cCHARSTATUS();

	void Init();
	void Load();
	void Release();
	void Close();
	void Draw();
	void Main();
	void LButtonDown(int x , int y);
	void LButtonUp(int x , int y);
	void RButtonDown(int x , int y);
	void RButtonUp(int x, int y);
	void KeyDown(); 
	
	void OtherStateShow();	//캐릭터스테이스터스에 보이지않는 인자를 보여준다 
	
	void DrawCharText();	//캐릭터 능력치를 보여준다 

	void CheckLevel();		//레벨을 체크한다 

	void BackUpsinCharState(smCHAR *pChar); //캐릭터 능력치를 백업한다 (해킹방지를 위해)
	
	int sinGetNowExp();		//현제의 경험치를 얻어온다 
	
	int sinGetNextExp();	//다음 레벨의 경험치를 얻어온다 

	int InitCharStatus(int kind=0);   //캐릭터 스텟 초기화

	int InitCharStatus_Attribute(int kind);   // 박재원 - 캐릭터 속성별 스탯 초기화 아이템

	int UseGravityScrool(); // 장별 - 그라비티 스크롤
	
	int CheckChageStateDisplay(); //캐릭터의 현제 상태를 체크해 Stats를 디스플레이해준다
	
};

extern cCHARSTATUS cCharStatus;

extern int sinMoveKindInter[MAX_SIN_KIND_INTER]; //각종 폼이 움직이는 값 

extern int OldLevel;		//레벨을 저장한다 
extern int sinLevelPoint;	//레벨 포인트를 저장한다 
extern int OldLevelTemp;	//레벨을 저장한다 후 또 저장하다 (왜? 나쁜넘들이 설치지못하도록)

// rows of new-character stats per race (HoLogin.cpp): Tempskron 6 classes, Morion 5 (+ an empty row)
#ifndef NEW_CHAR_JOB_MAX
#define NEW_CHAR_JOB_MAX	6
#endif
extern  int TempNewCharacterInit[NEW_CHAR_JOB_MAX][6];
extern  int MorNewCharacterInit[NEW_CHAR_JOB_MAX][6];
extern  sDISPLAYSTATE sDisplayState[10];

extern  int ChatKindIndexNum;
extern  int NoticeChatIndexNum;
extern  int NoticeChatIndex;
/*외부 extern */
extern INT64 ExpLevelTable[]; //경험치 테이블 


