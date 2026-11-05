#define FILED_GATE_MAX			12
#define FIELD_MAX				256
#define FIELD_AMBENT_MAX		80
class sFIELD;


//필드의 게이트
struct sFGATE	{
	int	x,z,y;				//필드 게이트 위치
	sFIELD	*lpsField;		//필드 포인터
};

//필드의 게이트
struct sWARPGATE	{
	int	x,z,y;								//필드 게이트 위치
	int	height,size;						//범위
	sFGATE	OutGate[FILED_GATE_MAX];		//출구 위치
	int		OutGateCount;					//출구 카운터
	int		LimitLevel;						//레벨제한
	int		SpecialEffect;					//특수 효과
};

struct	sAMBIENT_POS {
	int	x,y,z;
	int	round;
	int	AmbentNum;
};

#define FIELD_STATE_VILLAGE		0x100
#define FIELD_STATE_FOREST		0x200
#define FIELD_STATE_DESERT		0x300
#define FIELD_STATE_RUIN		0x400
#define FIELD_STATE_DUNGEON		0x500
#define FIELD_STATE_IRON		0x600
#define FIELD_STATE_ROOM		0x800
#define FIELD_STATE_QUEST_ARENA	FIELD_STATE_DUNGEON

//######################################################################################
//작 성 자 : 오 영 석
#define FIELD_STATE_ICE			0x900
//######################################################################################

#define FIELD_STATE_CASTLE		0xA00
#define FIELD_STATE_ACTION		0xB00


#define FIELD_STATE_ALL			0x1000

#define FIELD_BACKIMAGE_RAIN		0x00
#define FIELD_BACKIMAGE_NIGHT		0x01
#define FIELD_BACKIMAGE_DAY			0x02
#define FIELD_BACKIMAGE_GLOWDAY		0x03
#define FIELD_BACKIMAGE_DESERT		0x04
#define FIELD_BACKIMAGE_GLOWDESERT	0x05
#define FIELD_BACKIMAGE_NIGHTDESERT	0x06
#define FIELD_BACKIMAGE_RUIN1		0x07
#define FIELD_BACKIMAGE_RUIN2		0x08
#define FIELD_BACKIMAGE_NIGHTRUIN1	0x09
#define FIELD_BACKIMAGE_NIGHTRUIN2	0x0A

#define FIELD_BACKIMAGE_GLOWRUIN1	0x0B
#define FIELD_BACKIMAGE_GLOWRUIN2	0x0C


#define FIELD_BACKIMAGE_NIGHTFALL	0x11
#define FIELD_BACKIMAGE_DAYFALL		0x12
#define FIELD_BACKIMAGE_GLOWFALL	0x13

#define FIELD_BACKIMAGE_NIGHTIRON1	0x14
#define FIELD_BACKIMAGE_NIGHTIRON2	0x15
#define FIELD_BACKIMAGE_DAYIRON		0x16
#define FIELD_BACKIMAGE_GLOWIRON	0x17

#define FIELD_BACKIMAGE_SODSKY      0x18    //천공의 홀
#define FIELD_BACKIMAGE_SODMOON     0x19    //달의 장
#define FIELD_BACKIMAGE_SODSUN      0x20    //태양의 장
#define FIELD_BACKIMAGE_SODNONE     0x21    //아무것두 안나오는 하늘   나머지 장들은 하늘을 없애 버립니다.
#define FIELD_BACKIMAGE_IRONBOSS    0x22    //아이언 보스몹 나오는 하늘

#define FIELD_BACKIMAGE_DAYSNOW		0x23    //
#define FIELD_BACKIMAGE_GLOWSNOW    0x24    //
#define FIELD_BACKIMAGE_NIGHTSNOW   0x25    //

#define FIELD_BACKIMAGE_DAYGREDDY	 0x26    //
#define FIELD_BACKIMAGE_GLOWGREDDY   0x27    //
#define FIELD_BACKIMAGE_NIGHTGREDDY  0x28    //


#define FIELD_BACKIMAGE_DAYLOST		 0x29    //로스트월드 낮 (성근추가)
#define FIELD_BACKIMAGE_GLOWLOST	 0x2A    //로스트월드 저녁 
#define FIELD_BACKIMAGE_NIGHTLOST    0x2B    //로스트월드 밤

#define FIELD_BACKIMAGE_DAYTEMPLE	 0x2C	//로스트템플 낮
#define FIELD_BACKIMAGE_GLOWTEMPLE	 0x2D	//로스트템플 저녁
#define FIELD_BACKIMAGE_NIGHTTEMPLE	 0x2E	//로스트템플 밤


#define	FIELD_EVENT_NIGHTMARE		0x01

#define	FIELD_START_POINT_MAX		8

#define	FIELD_STAGE_OBJ_MAX			50


////////////////// 시작 필드 번호 ///////////////////////
#define START_FIELD_NUM			3
#define START_FIELD_NEBISCO		9
#define START_FIELD_MORYON		21
#define START_FIELD_CASTLE		33

struct	ACTION_FIELD_CAMERA {
	POINT3D	FixPos;
	int		LeftX,RightX;
};

//필드 구조
class sFIELD {
	DWORD	head;
public:
	char	szName[64];						//필드 파일 이름
	char	szNameMap[64];					//지역 지도 이미지 파일
	char	szNameTitle[64];				//지역 이름 이미지 파일
	int		State;							//필드 속성
	int		BackImageCode[3];				//기본 배경 하늘 번호
	int		BackMusicCode;					//배경음악 코드
	int		FieldEvent;						//필드의 이벤트

	int		GateCount;						//필드 게이트의 수
	sFGATE	FieldGate[FILED_GATE_MAX];		//필드 게이트 ( 연결되는 필드 )

	int		WarpGateCount;					//워프 게이트의 수
	int		WarpGateActiveNum;				//워프케이트 활성 번호
	sWARPGATE	WarpGate[FILED_GATE_MAX];	//워프 게이트
	POINT3D	PosWarpOut;						//워프 게이트 출구

	sAMBIENT_POS	AmbientPos[FIELD_AMBENT_MAX];	//배경 효과음
	int		AmbentCount;							//배경 효과음 수


	int		LimitLevel;						//레벨제한
	int		FieldSight;						//필드시야

	char	*lpStageObjectName[FIELD_STAGE_OBJ_MAX];		//배경 보조 오브젝트
	DWORD	StgObjBip[FIELD_STAGE_OBJ_MAX];					//배경 보조 오브젝트 BIP애니메이션 사용유무
	int		StgObjCount;

	int		cX,cZ;											//필드 중앙
	int		FieldCode;										//필드의 코드 번호
	int		ServerCode;

	POINT	StartPoint[FIELD_START_POINT_MAX];				//필드 시작 지점
	int		StartPointCnt;									//시작 지점 포인트

	ACTION_FIELD_CAMERA	ActionCamera;					//액션필드에서의 카메라 정보

	// Maps of an old client moved to an empty part of the world (web\port_old_maps.py):
	// the stage file is already moved; its props keep their own position and are
	// drawn moved by OffsetX/Z. OriginField = today's version of the map (-1: none).
	int		OffsetX, OffsetZ;
	int		OriginField;

	int	AddStageObject( char *szStgObjName , int BipAnimation=0 );	//배경 보조 오브젝트 추가
	int	GetStageObjectName( int num , char *szNameBuff );			//배경 보조 오브젝트 이름 얻기

	int	SetCenterPos( int x, int z );								//필드 중앙 좌표 입력
	int AddGate( sFIELD *lpsField , int x, int z, int y );			//게이트 추가
	int SetName( char *lpName , char *lpNameMap=0 );				//이름 설정
	int AddGate2( sFIELD *lpsField , int x, int z, int y );			//게이트 추가

	int	AddWarpGate( int x, int y, int z, int size , int height );	//워프게이트 추가
	int	AddWarpOutGate( sFIELD *lpsField , int x, int z, int y );	//워프게이트 출구 추가
	int	CheckWarpGate( int x, int y, int z );						//워프게이트 확인

	int AddStartPoint( int x, int z );							//시작 좌표를 설정 추가
	int GetStartPoint( int x, int z , int *mx , int *mz );		//제일 근접한 시작 좌표를 얻는다 
	int CheckStartPoint( int x, int z );						//시작 좌표상에 캐릭터가 있는지 확인 

	int AddAmbientPos( int x, int y, int z , int round , int AmbCode );	//배경 효과음 추가
	int	PlayAmbient();											//배경 효과음 연주 ( 주기적 호출 )

};



//필드 자동 변경 메인
int FieldMain( int x, int y, int z );
//필드 초기화
int InitField();
//필드 시작
int StartField();
//필드 시작
int WarpNextField( int *mx, int *mz );
//필드 워프
int WarpField( int FieldNum , int *mx, int *mz );
//필드 워프
int WarpFieldNearPos( int FieldNum , int x, int z , int *mx, int *mz );
//필드 시작
int WarpStartField( int *mx, int *mz );
//커스텀 필드로 이동
int WarpCustomField( int *mx, int *mz );

//필드 시작 ( 귀환용 )
int WarpStartField( int FieldNum , int *mx, int *mz );

//감옥에서 시작
int WarpPrisonField( int *mx, int *mz );

//윙을 사용한 워프
int	WingWarpGate_Field( int dwFieldCode );

//워프 필드
int WarpField2( int Num );

//워프 블레스 캐슬 필드
int WarpCastleField( int MasterClan , int *dx, int *dz );



extern sFIELD	sField[ FIELD_MAX ];
extern int	PrisonX;		//감옥위치
extern int	PrisonZ;		//감옥위치
extern RECT	PrisonRect;		//감옥 지역
extern DWORD	dwNextWarpDelay;	//워프 딜레이

/////////////////////// playmain.cpp 선언 ////////////////////////
//배경을 읽어 온다 ( 쓰레드 사용 )
smSTAGE3D *LoadStageFromField( sFIELD *lpField , sFIELD *lpSecondField );
//배경을 읽어 온다 
smSTAGE3D *LoadStageFromField2( sFIELD *lpField , sFIELD *lpSecondField );

extern	sFIELD	*StageField[2];

extern  int FieldLimitLevel_Table[];

//######################################################################################
//작 성 자 : 오 영 석
extern int FieldCount;
//######################################################################################
