/*----------------------------------------------------------------------------*
*	파일명 :  sinQuest.h	
*	하는일 :  신바람 퀘스트 
*	작성일 :  최종업데이트 2002년 7월
*	적성자 :  박상열 
*-----------------------------------------------------------------------------*/	

//몬스터 코드
#define MONSTER_BAGON				1
#define MONSTER_SKELETON_WARRIOR	2
#define MONSTER_HEAD_CUTTER			3
#define MONSTER_ARMORED_BETTLE		4
#define MONSTER_SKELETON_RANGER		5
#define MONSTER_TITAN				6
#define MONSTER_MURPIN				7
#define MONSTER_DIREBEE				8
#define MONSTER_TYPOON				9
#define MONSTER_RATOO				10
#define MONSTER_GROTESQUE			11
#define MONSTER_IRONFIST			12
#define MONSTER_OMEGA				13
#define MONSTER_D_MACHINE			14
#define MONSTER_MOUNTAIN			15

//첫번째 과제
#define MONSTER_SKELETON_KNIGHT     16 //스켈레톤 나이트
#define MONSTER_SOLID_SNAIL         17 //솔리드 스네일
#define MONSTER_THORN_CROWLER       18 //쏜 크로울러
#define MONSTER_MUMMY               19 //머미
#define MONSTER_DOOM_GUARD          20 //둠 가드
#define MONSTER_FIGON               21 //파이곤
#define MONSTER_STONE_GIANT         22 //스톤 자이언트
#define MONSTER_STONE_COLEM         23 //스톤 골렘
#define MONSTER_IRON_GUARD          24 //아이언 가드
#define MONSTER_AVELLRIN            25 //아벨린
//두번쨰 과제
#define MONSTER_AVELRISK_L          26 //아벨리스크-L
#define MONSTER_BEEBUL              27 //비블
#define MONSTER_ILLUSION_KNIGHT     28 //일루젼나이트
#define MONSTER_NIGHT_MARE          29 //나이트 메어
#define MONSTER_WITCH               30 //위치
#define MONSTER_HEAVY_GOBLIN        31 //헤비 고블린
#define MONSTER_DAWLIN              32 //다우린
#define MONSTER_STYGIAN             33 //스티지언
#define MONSTER_INCUBUS             34 //인큐버스
#define MONSTER_CHAIN_GOLEM         35 //체인 고렘
#define MONSTER_DARK_SPECTOR	    36 //다크 스펙터
#define MONSTER_METRON              37 //메트론
//세번쨰 과제
#define MONSTER_AVELRISK_LOAD       38 //아벨리스크 로드
#define MONSTER_DUSK                39 //더스크
#define MONSTER_SADNESS             40 //새드리스

//신규 퀘스트 몬스터 추가 
#define MONSTER_HOPY                41 //호피
#define MONSTER_RABIE               42 //래비
#define MONSTER_NORTHGOBLIN         43 //노쓰 고블린
#define MONSTER_HOBGOBLIN           44 //흡 고블린
#define MONSTER_CORRUPT             45 //커 럽트
#define MONSTER_SKELETON            46 //스켈레톤Cyclops
#define MONSTER_CYCLOPS             47 //싸이클롭스
//100LV 퀘스트 몬스터
#define MONSTER_FERRUMHEART         48 //페룸의 심장
#define MONSTER_ICE                 49 //싸이클롭스





//Quest Bit Code
#define QUESTBIT_LEVEL_30			1
#define QUESTBIT_LEVEL_55			2
#define QUESTBIT_LEVEL_70			4
#define QUESTBIT_LEVEL_80			8
#define QUESTBIT_LEVEL_85			16
#define QUESTBIT_LEVEL_90			32
#define QUESTBIT_LEVEL_80_2			64
#define QUESTBIT_LEVEL_90_2			128





/////퀘스트의 종류
#define SIN_QUEST_CODE_CHANGEJOB		0x0001
#define SIN_QUEST_CODE_CHANGEJOB2_NPC_M	0x0002
#define SIN_QUEST_CODE_CHANGEJOB2_NPC_D	0x0003
#define SIN_QUEST_CODE_CHANGEJOB3   	0x0004
#define SIN_QUEST_CODE_CHANGEJOB4   	0x0010
#define SIN_QUEST_CODE_CHANGEJOB5   	0x0030	// tier 5: the "4th Rank-Up Test", to the fifth rank (ChangeJob 4)

//레벨 퀘스트
#define SIN_QUEST_CODE_LEVEL30			0x0005
#define SIN_QUEST_CODE_LEVEL55			0x0006
#define SIN_QUEST_CODE_LEVEL55_2		0x0007
#define SIN_QUEST_CODE_LEVEL70			0x0008
#define SIN_QUEST_CODE_LEVEL80			0x0009
#define SIN_QUEST_CODE_LEVEL85			0x000A
#define SIN_QUEST_CODE_LEVEL90			0x000B


#define SIN_QUEST_CODE_LEVEL80_2		0x000C //연계퀘스트가 추가퀘스트 
#define SIN_QUEST_CODE_LEVEL90_2		0x000D //연계퀘스트가 추가퀘스트 


/////퀘스트의 진행상태 
#define SIN_QUEST_PROGRESS				1
#define SIN_QUEST_COMPLETE				2

/*----------------------------------------------------------------------------*
*						Quest 구조
*-----------------------------------------------------------------------------*/	
struct sQUEST_CHANGEJOB{ //템스크론 모라이온 2차전직
	DWORD CODE;
	DWORD BackUpCode;
	int Kind;
	int State;

};

struct sQUEST_CHANGEJOB3{ //템스크론 모라이온 3차전직
	DWORD CODE;
	DWORD BackUpCode;
	int   Kind;
	int   State;        //숙성 Count
	short Monster[2];   //0 몬스터 종류 1 몬스터 카운트
	int   StartLevel;   //전업시작시 레벨
	int   AgingCount;   //현재에이징 레벨

};
/*
struct sQUEST_LEVEL{	//레벨퀘스트
	DWORD CODE;
	DWORD BackUpCode;
	int   Kind;
	int   State;        //숙성 Count
	short Monster[2];   //0 몬스터 종류 1 몬스터 카운트
	int   StartLevel;   //전업시작시 레벨
	int   AgingCount;   //현재에이징 레벨

};
*/

/*----------------------------------------------------------------------------*
*						Wing Item Quest
*-----------------------------------------------------------------------------*/	
struct sQUEST_WINGITEM{
	DWORD CODE;
	int Kind;
	int State;

};

/*----------------------------------------------------------------------------*
*						함수 
*-----------------------------------------------------------------------------*/	
int sinCheckChageJob();



int sinLoadQuest(int CODE ,void *Buff );	//퀘스트 데이타를 로드한다 
int sinSaveQuest(void *Buff );	//퀘스트 데이타를 세이브한다 

//모라이온 2차 스킬 전업
int sinMorayionNpcChangeJob_Two();	

//퀘스트시 NPC의 대사를 보여주고 아이템을 처리한다 (3차전직)
int sinNpcChangeJob3();	

//퀘스트에 사용될 아이템을 셋팅한다
int SetChangeJob3QuestItem(sITEMINFO *pItem , int Down = 0);

//잡은 몬스터에 따라 체크 
int CheckChangeJob3QuestItem(DWORD Monster_CODE);

//코드로 퀘스트 아이템을 지워준다
int DeleteQuestItem(DWORD CODE);

//3차전업이 끝났는지를 체크한다
int CheckChangeJob_QuestItem();

//레벨퀘스트
int sinCheckLevelQuest30();
int sinCheckLevelQuest55();
int sinCheckLevelQuest55_2();
int sinCheckLevelQuest70();
int sinCheckLevelQuest80();
int sinCheckLevelQuest85();
int sinCheckQuest85Die();
int sinCheckLevelQuest90();
int sinCheckLevelQuest80_2(int NpcNum=0);
int sinCheckLevelQuest80_2Item(); //퀘스트 아이템을 체크한다
int sinCheckLevelQuest90_2(int NpcNum=0); //스탯 +10 퀘스트

int sinSetSendItem80_2(); //아이템을 서버로보낸다
//진행중인 퀘스트가 있나 확인
int CheckNowQuestState(DWORD CODE);
//퀘스트를 초기화한다
int ClearNowQuest();
//4차스킬 전업퀘스트
int sinCheckChangeJobQuest4();
int sinSetQuestTimer(sQUEST_CHANGEJOB3 *pQuset, int UseTime=0 , int CheckTime=0);

int sinChangeJob4MonSet(sQUEST_CHANGEJOB3 *pQuset);

//npc와의 결전에서의 승부결과를 받는다
int sinBattleNpcResult(int WinFlag);

//몬스터 코드로 이름을 알아온다
char *sinGetMonsterName(int CODE);

//4차스킬을 임의Key로 셋팅한다
int sinCompleteChangeJob4(int Index);

// tier 5: the fifth rank-up quest (sinQuest_ChangeJob5, SIN_QUEST_CODE_CHANGEJOB5)
//  States: 1/3/5 hunt 1..3 running, 2/4/6 hunt done (report to the class master for a Moon Stone),
//  7 three stones: to Elder Hosean, 8 in the arena with Hestian, 9 Hestian beaten (back to Hosean),
//  10 holding the Gem of Hestian: the class master promotes.
int sinCheckChangeJob5Button();				// the class master's rank-up button (level 80, rank 4, all skills)
int sinCheckChangeJobQuest5();				// the class master, while the quest runs
int sinChangeJob5Hosean();					// Elder Hosean (NPC class 2100)
int sinChangeJob5DialogClosed();			// OK on one of this quest's dialogs (TRUE if it was one)
int sinChangeJob5Start();					// the start question answered yes
int sinChangeJob5Promote();					// the rank-up question answered yes
int sinChangeJob5Kill(DWORD Monster_CODE);	// a monster killed
int sinChangeJob5TimerOut();				// the quest timer ran out
int sinBattleNpcResult5(int WinFlag);		// the duel with Hestian ended
int sinCompleteChangeJob5(int Index);		// GM: /Lee5thRankUp


/*----------------------------------------------------------------------------*
*						extern 
*-----------------------------------------------------------------------------*/	
extern POINT QuestMessageBoxPosi2;
extern POINT QuestMessageBoxSize2;
extern int sinChangeJobButtonShow;
extern int sinChangeJobButtonShow2; //모라이온 전업
extern int sinChangeJobButtonShow3; //3차전직
extern int sinChangeJobButtonShow4; //4차전직

extern char *QuestFilePath[];
extern char *QuestFilePath3[];
extern int CheckQuestComplete;	//전업의 진행사항

extern int CheckQuestButtonShow1;
extern int CheckQuestButtonShow2;
extern int CheckQuestButtonShow3;



extern sQUEST_CHANGEJOB	sinQuest_ChangeJob; //템스크론 2차전업	
extern sQUEST_CHANGEJOB	sinQuest_ChangeJob2;//모라이온 2차전업
extern sQUEST_CHANGEJOB3   sinQuest_ChangeJob3;//템스크론 모라이온 3차 전직퀘스트 
extern sQUEST_CHANGEJOB3   sinQuest_ChangeJob4;//템스크론 모라이온 4차 전직퀘스트
extern sQUEST_CHANGEJOB3   sinQuest_ChangeJob5;	// tier 5: the fifth rank-up quest
extern int sinChangeJobButtonShow5;				// tier 5: the fifth rank-up button

extern char *WingQuestFilePath_a[];
extern char *WingQuestFilePath_b[];
extern char *WarpGateDoc[];
extern char *WarningInitPoint[];
extern char *HaWarpGateDoc[];

extern sITEM TempQuestItem;
extern int CheckQuestItemDownFlag;

extern char *BabelEventDoc;
extern char *NineTailFoxDoc;
extern char *StarDustDoc;		 //크리스마스 이벤트
extern char *CristalEventDoc;//일곱가지 크리스탈의 보은

//박재원 - 수박 모아오기
extern char *WatermelonDoc;//수박 모아오기 이벤트
//박재원 - 호박 모아오기
extern char *PumpkinDoc;//호박 모아오기 이벤트
//박재원 - 밤하늘의 소원 이벤트
extern char *StarDoc;
//장별 - 발렌타인 초콜릿 모아오기
extern char *ValentineDoc;
// 박재원 - 알파벳 조합 이벤트
extern char *PristonAlphabetDoc; // 박재원 - 알파벳 조합 이벤트

// 장별 - 캔디데이즈 캔디 모아오기
extern char *CandydaysDoc;

// 장별 - 매지컬그린
extern char *MagicalGreenDoc;

// 장별 - 카라의 눈물
extern char *TearOfKaraDoc;

// 장별 - 조사원을 찾아라
extern char*Findinvestigator1_1Doc;
extern char*Findinvestigator1_2Doc;
extern char*Findinvestigator1_3Doc;
extern char*Findinvestigator2_1Doc;
extern char*Findinvestigator2_2Doc;
extern char*Findinvestigator3_1Doc;
extern char*Findinvestigator3_2Doc;

extern sQUEST_CHANGEJOB3   sinQuest_Level30;
extern sQUEST_CHANGEJOB3   sinQuest_Level55;
extern sQUEST_CHANGEJOB3   sinQuest_Level55_2;
extern sQUEST_CHANGEJOB3   sinQuest_Level70;
extern sQUEST_CHANGEJOB3   sinQuest_Level80;
extern sQUEST_CHANGEJOB3   sinQuest_Level85;
extern sQUEST_CHANGEJOB3   sinQuest_Level90;
extern DWORD sinQuest_levelLog; //퀘스트를 수행했나 로그
extern sQUEST_CHANGEJOB3   sinQuest_Level80_2; //새로추가된 스텟 +7퀘스트
extern sQUEST_CHANGEJOB3   sinQuest_Level90_2; //90퀘스트
extern char *QuestFilePath4[];
extern char szQuestMonsterName[64];
