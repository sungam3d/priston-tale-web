
#define MAP_SIZE			256
#define MAP_CLIP			0xFF	//(AND 0xFF)
#define MAX_CELL			4096
#define MAPTEXTURE_SIZE		64
#define RAYCLIP_ANGLE		ANGLE_45+ANGLE_1
//#define RAYCLIP_ANGLE		260
#define RAYCLIP_ADD			5

#define ANGCLIP				ANGLE_MASK
#define CLIP_OUT			-32767
#define SMFLAG_NONE			0xFFFFFFFF
/*
#define	fONE		512
#define FLOATNS		9
#define FLOATDS		7
*/


///////////////////// 고정 소수점 단위 //////////////////
#define	fONE		256
#define FLOATNS		8
#define FLOATDS		8

//MATRIX용 16비트 소수점
#define wfONE		32768
#define wFLOATS		15
#define wSHIFT_FLOAT	(wFLOATS-FLOATNS)

///////////////////// 맵 크기 관련 값들 ///////////////////
#define SizeMAPCELL			64			//맵상의 1 CELL 실제 크기
#define ShiftMAPCELL_MULT	6		    //맵상의 1 CELL 실제 크기 쉬프트 곱
#define ShiftMAPCELL_DIV	6		    //맵상의 1 CELL 실제 크기 쉬프트 나눔

#define SHIFT_MAPHEIGHT		(ShiftMAPCELL_MULT-3)
#define AND_SizeMAPCELL		(SizeMAPCELL-1)
#define OR_SizeMAPCELL		(0xFFFFFFFF-AND_SizeMAPCELL)

//////////////////// 렌더링 및 배경 기본 밝기 ////////////////////
#define DEFAULT_CONTRAST			300
#define DEFAULT_BRIGHT				160



////////////////  smRENDER 클리핑 요구 상태 값 ( ClipStatus ) //////////////////
#define SMCLIP_NEARZ		0x00000001
#define SMCLIP_FARZ			0x00000002
#define SMCLIP_LEFT			0x00000004
#define SMCLIP_RIGHT		0x00000008
#define SMCLIP_TOP			0x00000010
#define SMCLIP_BOTTOM		0x00000020
#define SMCLIP_TEXTURE		0x00000040
#define SMCLIP_DISPLAYOUT	0x00000080
#define SMCLIP_DISPLAYIN	0x00000100

#define SMCLIP_VIEWPORT		( SMCLIP_NEARZ | SMCLIP_FARZ | SMCLIP_LEFT | SMCLIP_RIGHT | SMCLIP_TOP | SMCLIP_BOTTOM | SMCLIP_DISPLAYOUT )

////////////////// RGBA 배열 //////////////////////
#define SMC_A				3
#define SMC_R				2
#define SMC_G				1
#define SMC_B				0



//######################################################################################
// 작 성 자 : 오 영 석
// 실장님이 변경 하라고 하셨음.
// 텍스쳐 최대치

#include "..\\nettype.hpp"
#ifdef _W_SERVER
#define MAX_TEXTURE		8000
#else
#define MAX_TEXTURE		5000
#endif
//######################################################################################





//고정 소수점 행렬
struct smMATRIX { 
    int _11, _12, _13, _14; 
    int _21, _22, _23, _24; 
    int _31, _32, _33, _34; 
    int _41, _42, _43, _44; 
};

//고정 소수점 행렬2
struct smEMATRIX { 
    int _11, _12, _13, _14; 
    int _21, _22, _23, _24; 
    int _31, _32, _33, _34; 
    int _41, _42, _43, _44; 
};


//부동 소수 Double 형 행렬
struct smDMATRIX { 
    double _11, _12, _13, _14; 
    double _21, _22, _23, _24; 
    double _31, _32, _33, _34; 
    double _41, _42, _43, _44; 
};

//부동 소수 float 형 행렬
struct smFMATRIX { 
    float _11, _12, _13, _14; 
    float _21, _22, _23, _24; 
    float _31, _32, _33, _34; 
    float _41, _42, _43, _44; 
};

struct smRGB {
	DWORD r,g,b;
};
struct smLIGHTLEVEL {
	BYTE		cr,cg,cb;
	BYTE		sr,sg,sb;
};

struct smLIGHT3D {
	int type;
	int x,y,z;
	int Range;
	short r,g,b;

};

#define	smLIGHT_TYPE_NIGHT		0x00001
#define	smLIGHT_TYPE_LENS		0x00002
#define	smLIGHT_TYPE_PULSE2		0x00004				// 쓰이는 곳이 없음. ???
//######################################################################################
//작 성 자 : 오 영 석
#define	SMLIGHT_TYPE_OBJ		0x00008
//######################################################################################
#define	smLIGHT_TYPE_DYNAMIC	0x80000



struct POINT3D {
	int x,y,z;
};

struct TPOINT3D {
	int x,y,z;
	float u,v;
	float zb;
	BYTE	bCol[4];
	BYTE	bSpe[4];
};

struct smLINE3D {
	POINT3D sp;
	POINT3D ep;
};

struct smTRECT {
	float u0,v0;
	float u1,v1;
	float u2,v2;
	float u3,v3;
};

struct smTPOINT {
	int u,v;				
};

struct smFTPOINT {
	float u,v;				
};



// 3각 폴리곤의 텍스쳐 좌표 와 연결 포인트
struct smTEXLINK {
	float	u[3],v[3];	
	DWORD	*hTexture;
	smTEXLINK	*NextTex;
};

struct smVERTEX{
	int x,y,z;
	int nx,ny,nz;
};

struct smFACE{
	WORD v[4];			// a,b,c , matrial
	smFTPOINT t[3];		// 텍스쳐 포인트

	smTEXLINK	*lpTexLink;		// 텍스쳐 연결 좌표 

};

struct smTM_ROT {
	int frame;
	float x,y,z,w;
};
struct smDTM_ROT {
	int frame;
	double x,y,z,w;
};

//######################################################################################
//작 성 자 : 오 영 석
struct smTM_POS {
	int   frame;
	float x, y, z;
};
//######################################################################################

struct smTM_SCALE {
	int frame;
	int x,y,z;
};


// 렌더링을 위한 버텍스
struct smRENDVERTEX {

	// 계산된 버텍스
	int		tx,ty,tz;			// Transrate 카메라 좌표
	DWORD	ClipStatus;			// 클리핑 요구 상태
//	short	xy[2];				// 2D 변환 좌표
	float	xy[2];				// 2D 변환 좌표
	float	zb;					// Z Buffer 깊이
	float	rhw;				// rhw

	//######################################################################################
	//작 성 자 : 오 영 석
	int		nx,ny,nz;
	//######################################################################################

	short	sLight[4];			// 빛계산 버퍼 (RGBA)

	BYTE	bCol[4];			// 버텍스 색상 (RGBA)
	BYTE	bSpe[4];			// 버텍스 Specular (RGBA)

	void	*lpSourceVertex;	//원본 버텍스 포인터
};

// 렌더링을 위한 페이스
struct smRENDFACE {

	smRENDVERTEX	*lpRendVertex[3];	// a,b,c
	DWORD			Matrial;			// 메트리얼
	smTEXLINK		*lpTexLink;			// 텍스쳐 연결 좌표 
	DWORD			ClipStatus;			// 클리핑 요구 상태
	smRENDFACE		*NexRendFace;		// 다음 렌더링페이스 포인터

};

//렌더링할 빛 구조체
struct smRENDLIGHT {
	int type;
	int x,y,z;
	int rx,ry,rz;
	int Range;
	int dRange;
	int r,g,b,a;
};


//메트리얼별 렌더링 페이스의 연결체
struct smRENDMATRIAL {

	int RegistRendList;					// 렌더링 리스트에 등록 여부 표시

	// 일반형
	DWORD		MatrialCounter;			// 렌더링할 페이스의 수 ( 메트리얼 당 )
	smRENDFACE	*StartFace;				// 렌더링할 시작 페이스 포인터
	smRENDFACE	*LastLinkFace;			// 마지막으로 연결한 페이스

	// 텍스쳐 클립형
	DWORD		TexClip_MatrialCounter;	// 렌더링할 페이스의 수 ( 메트리얼 당 )
	smRENDFACE	*TexClip_StartFace;		// 렌더링할 시작 페이스 포인터
	smRENDFACE	*TexClip_LastLinkFace;	// 마지막으로 연결한 페이스
};

//텍스쳐 영역
struct smTEXRECT {
	float	left,	right;
	float	top,	bottom;
};

//2D 이미지 페이스
struct smFACE2D {
	int			x, y, z;			// 좌표
	int			width , height;		// 이미지 크기
	smTEXRECT	TexRect;			// 텍스쳐 영역
	int			MatNum;				// 메트리얼 번호
	int			Transparency;		// 투명도
	int			r,g,b;
};


// 버텍스
struct smSTAGE_VERTEX {
	DWORD			sum;				// 최근 연산 번호 
	smRENDVERTEX	*lpRendVertex;		// 렌더링을 위한 버텍스 포인터

	// 소스 버텍스
	int				x,y,z;				// 월드 좌표
	//BYTE			bDef_Color[4];		// 기본 색상 (RGBA)(Color )
	//BYTE			bDef_Specular[4];	// 기본 색상 (RGBA)(Specular )

	short			sDef_Color[4];		// 기본 색상 ( RGBA )

};	// 현재 크기 28 바이트


// 페이스
struct smSTAGE_FACE {
	DWORD	sum;				// 최근 연산 번호
	int		CalcSum;			// 기타 연산 번호

	WORD	Vertex[4];			// a,b,c,Matrial

	smTEXLINK	*lpTexLink;		// 텍스쳐 연결 좌표 

	short	VectNormal[4];			// Cross벡터( Normal )  ( nx , ny , nz , [0,1,0]벡터 Y ); 
	//int		NormalY;			// ( 0,1,0 ) 벡터의 DotProduct 값
};




#define		CONFIG_KEY_MONSTER_MAX		5

//기본 설정 구조체
struct smCONFIG {

	int		WinMode;					//화면 모드
	POINT	ScreenSize;					//화면 해상도
	DWORD	ScreenColorBit;				//색상 비트
	
	int		TextureQuality;				//텍스쳐 품질
	int		BGM_Mode;					//배경 음악 사용 
	int		NetworkQuality;				//네트워크 품질
	int		WeatherSwitch;				// pluto 해외 비

	char	szFile_BackGround[2][64];	//배경 화면
	char	szFile_Menu[64];			//메뉴 화면
	char	szFile_Player[64];			// 주인공
	POINT	Posi_Player;
	char	szFile_Enemy[64];			// 나쁜놈
	POINT	Posi_Enemy;

	char	szFile_Stage[64];				//지형
	POINT	Posi_Stage;						//지형 시작 위치
	char	szFile_StageObject[100][64];	//지형에 올라가는 오브젝트
	int		StageObjCnt;					//지형 오브젝트 갯수

	//######################################################################################
	//작 성 자 : 오 영 석
	int		IsStageAniObject[100];
	//######################################################################################

	//신규 맵 불러 올때만 적용됨
	int		MapBright;						//밝기
	int		MapContrast;					//선명도
	POINT3D	MapLightVector;					//벡터

	char	szServerIP[32];					//서버주소
	DWORD	dwServerPort;					//서버포트
	char	szDataServerIP[32];				//데이타 서버주소
	DWORD	dwDataServerPort;				//데이타 서버포트
	char	szUserServerIP[32];				//유저 서버주소
	DWORD	dwUserServerPort;				//유저 서버포트
	char	szExtendServerIP[32];			//확장 서버주소
	DWORD	dwExtendServerPort;				//확장 서버포트

	DWORD	DebugMode;						//디버깅 모드 설정


	char	szCmdOpenMonster[CONFIG_KEY_MONSTER_MAX][32];		//생성시킬 몬스터 이름
	int		CmdMonsterCount;
};




#define MOTION_LIST_MAX			32
#define MOTION_INFO_MAX			512
#define	MOTION_TOOL_MAX			104		// 52 in this source; the v3215 data (.inx) uses 104. See fileread.cpp smModelInfoFromOldFile()
#define	MOTION_TOOL_MAX_OLD		52
#define	MOTION_SKIL_MAX			8


//######################################################################################
//작 성 자 : 오 영 석
// 동작을 랜더하게 비율에 의해서 (% 값에 의한.)  작동을 할때에.
// 얼굴 동작의 2 단계로 (표정) (말하기).  <- 표정 모션 합쳐서 100%, 말하기 모션 합쳐서 100%
// NPC  동작은 1 단계로 작동 합니다.      <- 모든 모션 합쳐서 100%.
#define NPC_MOTION_INFO_MAX		 30
#define TALK_MOTION_INFO_MAX	 30

// MotionFrame  값이.
// 실장님의 데이타의 경우는.  "MotionFrame-1" 로 사용.
// 내가 만든것은. (얼굴)  MotionFrame 값 그대로 적용. 
#define TALK_MOTION_FILE_MAX	 2
#define TALK_MOTION_FILE		 0
#define FACIAL_MOTION_FILE		 1
//######################################################################################


struct smMOTIONINFO {
	DWORD	State;							//상태 TRUE면 유효
	
	//######################################################################################
	//작 성 자 : 오 영 석
	DWORD	MotionKeyWord_1;
	DWORD	StartFrame;						//시작 프레임
	DWORD	MotionKeyWord_2;
	DWORD	EndFrame;						//종료 프레임
	//######################################################################################

	DWORD	EventFrame[4];					//이벤트 생성 프레임

	int		ItemCodeCount;					//해당 아이템 리스트 카운터 ( 0 없음  -1 전체 해당 )
	// the weapons this clip is for: 16-bit indices into the v3215 client's item table, as the
	// .inx files store them (0xFFFF = bare hands). Older files (52 one-byte entries, our own item
	// table's indices) and .ini files are converted to these on loading (fileread.cpp).
	WORD	ItemCodeList[MOTION_TOOL_MAX/2];	//해당 아이템 코드 리스트
	DWORD	dwJobCodeBit;					//해당 직업별 비트 마스크
	BYTE	SkillCodeList[MOTION_SKIL_MAX];	//해당 스킬 번호
	//BYTE	JobCodeList[MOTION_JOB_MAX];	//해당 스킬 번호

	int		MapPosition;					//해당 맵 적용 ( 0-관계없음 마을 1 - 필드 2 )

	DWORD	Repeat;							//반복 여부
	CHAR	KeyCode;						//동작 시작 키
	int		MotionFrame;					//모션 연결 파일 번호
};

struct _MODELGROUP {
	int ModelNameCnt;
	char szModelName[4][16];
};

struct smMODELINFO {

	char	szModelFile[64];
	char	szMotionFile[64];
	char	szSubModelFile[64];

	_MODELGROUP	HighModel;
	_MODELGROUP	DefaultModel;
	_MODELGROUP	LowModel;

	smMOTIONINFO	MotionInfo[MOTION_INFO_MAX];
	DWORD	MotionCount;

	//######################################################################################
	//작 성 자 : 오 영 석
	DWORD			FileTypeKeyWord;
	DWORD			LinkFileKeyWord;
	//######################################################################################

	//######################################################################################
	//작 성 자 : 오 영 석
	char			szLinkFile[64];
	//######################################################################################

	//######################################################################################
	//작 성 자 : 오 영 석
	// ******************** 주의 ******************** 
	// 그 파일 2개가 *.ini  *.in  파일명이 같은 경우. 둘다 같은 이름의 *.inx 로 나오기땜시.
	// 파일명이 달라야 합니다.
	char			szTalkLinkFile[64];	
	char			szTalkMotionFile[64];
	smMOTIONINFO	TalkMotionInfo[ TALK_MOTION_INFO_MAX ];
	DWORD			TalkMotionCount;

	int				NpcMotionRate[NPC_MOTION_INFO_MAX];
	int				NpcMotionRateCnt[ 100 ];

	int				TalkMotionRate[ TALK_MOTION_INFO_MAX ];
	int				TalkMotionRateCnt[ TALK_MOTION_FILE_MAX ][ 100 ];
	//######################################################################################	
};




extern BYTE VRKeyBuff[256];





struct smTEXPOINT {
	float	u,v;
	DWORD	hTexture;
};

struct smFCOLOR {
	float	r,g,b;
};

//텍스쳐 맵 렌더링 좌표 속성
#define smTEXSTATE_FS_NONE		0
#define smTEXSTATE_FS_FORMX		1
#define smTEXSTATE_FS_FORMY		2
#define smTEXSTATE_FS_FORMZ		3
#define smTEXSTATE_FS_SCROLL	4
#define smTEXSTATE_FS_REFLEX	5

#define smTEXSTATE_FS_SCROLL2	6
#define smTEXSTATE_FS_SCROLL3	7
#define smTEXSTATE_FS_SCROLL4	8
#define smTEXSTATE_FS_SCROLL5	9
#define smTEXSTATE_FS_SCROLL6	10
#define smTEXSTATE_FS_SCROLL7	11
#define smTEXSTATE_FS_SCROLL8	12
#define smTEXSTATE_FS_SCROLL9	13
#define smTEXSTATE_FS_SCROLL10	14

//######################################################################################
//작 성 자 : 동동동
#define smTEXSTATE_FS_SCROLLSLOW1	15
#define smTEXSTATE_FS_SCROLLSLOW2	16
#define smTEXSTATE_FS_SCROLLSLOW3	17
#define smTEXSTATE_FS_SCROLLSLOW4	18
//######################################################################################


/////////////////////////// Texture Map 핸들 구조 //////////////////////////
struct smTEXTUREHANDLE {
	char Name[64];
	char NameA[64];
    LPDIRECT3DTEXTURE2 lpD3DTexture;
	LPDIRECTDRAWSURFACE4 lpDDSurface;

    LPDIRECT3DTEXTURE2 lpD3DTextureLarge;		//큰사이즈의 텍스쳐 ( 시스템 메모리와의 스왑으로 얻어짐 )
	LPDIRECTDRAWSURFACE4 lpDDSSysMemory;		//시스템 메모리 텍스쳐

	int Width , Height;
	int UsedTime;

	int UseCounter;

	int	MapOpacity;								//맵 오피시티 여부 ( TRUE , FALSE )

	DWORD	TexSwapMode;						//스왑 텍스쳐를 사용여부 ( TRUE / FALSE )

	smTEXTUREHANDLE *TexChild;
};


/////////////////////////// ASE 에서 읽어 들인 메트리얼 //////////////////////////
struct ASE_MATERIAL {
	int			Regist;						//smMATERIAL 에 등록 여부
	int			RegistNum;					//smMATERIAL 에 등록된 번호

	int			TextureCounter;				//비트맵 사용 갯수
	DWORD		UseCounter;					//사용 카운터 ( 사용수가 0 이면 제거 목적 )

	smFCOLOR	Diffuse;					// Diffuse 색상
	float		Transparency;				// 투명도
	float		SelfIllum;					// 자체 발광
	DWORD		TwoSide;					// 양면 표시 모드 ( 뒤집힌면 표시 여부 )
	DWORD		ScriptState;				// 스크립트로 받은 속성

	//######################################################################################
	//작 성 자 : 오 영 석
	char		BITMAP[8][64];				// Bitmap 이름
	//######################################################################################

	DWORD		BitmapStateState[8];		// 비트맵 스테이지 속성
	DWORD		BitmapFormState[8];			// 비트맵 폼 속성

	//######################################################################################
	//작 성 자 : 오 영 석
	char		MAP_OPACITY[64];			// 반투명 맵 이름
	//######################################################################################

	float		UVW_U_OFFSET[8];			// 맵 OFFSET
	float		UVW_V_OFFSET[8];			//
	float		UVW_U_TILING[8];			// 맵 TILING
	float		UVW_V_TILING[8];
	float		UVW_ANGLE[8];				// 맵 ANGLE

	int			SubPoint;					// 서브 메트리얼 번호

	int			BlendType;					//블렌드 타입

};



//////////////////////////////// 메트리얼  //////////////////////////////
struct smMATERIAL {
	DWORD			InUse;						//메트리얼 사용 여부
	DWORD			TextureCounter;				//텍스쳐 카운터
	smTEXTUREHANDLE	*smTexture[8];				//텍스쳐 핸들 리스트
	//속성
	DWORD			TextureStageState[8];		//그리기 속성 
	DWORD			TextureFormState[8];		//폼 속성  
	int				ReformTexture;				//텍스쳐 리폼속성 확인

	int				MapOpacity;					//맵 오피시티 여부 ( TRUE , FALSE )

	//일반적인 속성
	DWORD			TextureType;				//텍스쳐 타입 ( 멀티믹스 / 애니메이션 )
	DWORD			BlendType;					//브랜드 형식 ( SMMAT_BLEND_XXXX )

	DWORD			Shade;						//쉐이딩 방식 ( 노쉐이드 / 그로쉐이드 )
	DWORD			TwoSide;					//양면 사용 여부
	DWORD			SerialNum;					//메트리얼에 대한 고유 번호

	smFCOLOR		Diffuse;					// Diffuse 색상
	float			Transparency;				// 투명도
	float			SelfIllum;					// 자체 발광

	int				TextureSwap;				// 텍스쳐 스와핑
	int				MatFrame;					// 사용프레임 ( 사용시 동기를 맞추기 위함 )
	int				TextureClip;				// 스와핑용 텍스쳐 클립유무 ( TRUE 면 텍스쳐 클리핑 허가 )

	//메쉬 관련 속성
	int				UseState;					// 용도 ( 사용 속성 )
	int				MeshState;					// MESH의 질감 속성값 

	//Mesh 변형 관련 설정
	int				WindMeshBottom;				// 바람불기 메쉬 변형 시작 값

	//에니메이션 텍스쳐 속성
	smTEXTUREHANDLE	*smAnimTexture[32];			//애니메이션 텍스쳐 핸들 리스트
	DWORD			AnimTexCounter;				//애미네이션 텍스쳐 카운터
	DWORD			FrameMask;					//애니메이션용 프레임 마스크
	DWORD			Shift_FrameSpeed;			//프레임 증가 속도 ( 타이머를 Shift 하여 계산 )
	DWORD			AnimationFrame;				//프레임 번호 ( 고정일 경우 프레임값 / SMTEX_AUTOANIMATION 은 자동 )


};

//////////////// 메트리얼 속성 설정 값들 ////////////////////

//TextureType 적용
#define SMTEX_TYPE_MULTIMIX		0x0000
#define SMTEX_TYPE_ANIMATION	0x0001


//Texture 에니메이션 프레임중 자동일 경우
#define SMTEX_AUTOANIMATION		0x100


//MeshState 적용
//바닥 체크 여부
#define SMMAT_STAT_CHECK_FACE	0x00000001

//
#define SMMAT_BLEND_NONE		0x00
#define SMMAT_BLEND_ALPHA		0x01
#define SMMAT_BLEND_COLOR		0x02
#define SMMAT_BLEND_SHADOW		0x03
#define SMMAT_BLEND_LAMP		0x04
#define SMMAT_BLEND_ADDCOLOR	0x05
#define SMMAT_BLEND_INVSHADOW	0x06



////////////////////////////////////////////////////////////////



//////////////// ASE 라이트 오브젝트 ////////////////////
struct smASE_LIGHT {

	int	x,y,z;
	int r,g,b;

	int Size;
	int Range;
	int	Dist;
	int Type;
};


//카메라 추적 좌표
struct eCAMERA_TRACE {
	int x,y,z;					//실제 카메라 좌표
	int tx,ty,tz;				//목표점 좌표

	smEMATRIX	eRotMatrix;		//회전 계산된 행렬
	int AngX,AngY;				//측정된 각도 ( 오차 범위큼 )
};


