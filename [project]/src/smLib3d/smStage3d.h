

#define ADD_TEMPFACE	2048
#define ADD_TEMPVERTEX	2048
#define MAP_SIZE		256

// Face index list per map area. This source used WORD (max 65535 faces per
// map); the v3215 .smd maps store DWORD entries, which bigger maps need.
typedef DWORD smAREA_INDEX;


//걸어 다닐수 있는 최대 고저차
extern int Stage_StepHeight;
extern int smStage_WaterChk;

class smSTAGE3D {
public:
	DWORD	Head;

	smAREA_INDEX *StageArea[MAP_SIZE][MAP_SIZE];	//구역별 페이스 값 버퍼 리스트
	POINT *AreaList;						//구역별 페이스 리스트 임시저장 
	int AreaListCnt;						//구역 페이스의 커운터

	int	MemMode;							//메모리 설정 모드 ( 예정 )

	DWORD SumCount;							// 렌더링 첵크 카운터 
	int CalcSumCount;						// 연산 번호 카운터

	smSTAGE_VERTEX		*Vertex;			//버텍스 리스트
	smSTAGE_FACE		*Face;				//페이스 리스트
	smTEXLINK			*TexLink;			//텍스쳐 좌표 리스트
	smLIGHT3D			*smLight;			//조명 설정

	smMATERIAL_GROUP	*smMaterialGroup;	//메트리얼 그룹

	smSTAGE_OBJECT		*StageObject;		//맵의 구역상의 오브젝트들
	smMATERIAL			*smMaterial;		//메트리얼 리스트 포인터

	int nVertex;							//버텍스 수
	int nFace;								//페이스 수
	int nTexLink;							//텍스쳐좌표 연결 수
	int	nLight;								//조명 수

	int	nVertColor;								//버텍스 색 입력 카운터

	int	Contrast;								//채도 ( 선명도 )
	int Bright;									//명도 ( 밝기 정도 )

	POINT3D VectLight;							//쉐이드용 라이트 벡터

	smAREA_INDEX	*lpwAreaBuff;						//구역별 저장 버퍼
	int		wAreaSize;							//구역별 저장 버퍼 크기
	RECT	StageMapRect;						//전체 STAGE가 존재하는 구역 위치

	smSTAGE3D();
	~smSTAGE3D();
	smSTAGE3D( int nv , int nf );

	//######################################################################################
	//작 성 자 : 오 영 석
	void Init(void);
	//######################################################################################

	int Init( int nv , int nf );
	int Close();
	//버텍스 좌표 추가
	int AddVertex ( int x, int y, int z );
	//페이스 추가
	int AddFace ( int a, int b, int c , int matrial=0 );
	//페이스에 메트리얼 입력
	int SetFaceMaterial( int FaceNum , int MatNum  );

	//버텍스에 색 설정
	int SetVertexColor ( DWORD NumVertex , BYTE r , BYTE g, BYTE b , BYTE a=255 );
	//텍스쳐 좌표연결을 추가한다 
	int AddTexLink( int FaceNum , DWORD *hTex , 
							  smFTPOINT *t1 , smFTPOINT *t2 , smFTPOINT *t3 );
	
	//노말 값을 구하여 버텍스에 쉐이딩 명암을 추가한다
	//######################################################################################
	//작 성 자 : 오 영 석
	int SetVertexShade( int isSetLight = TRUE );
	//######################################################################################

	//모든 버텍스에 조명값을 추가 한다
	int AddVertexLightRound( POINT3D *LightPos , int r, int g, int b, int Range );

	//동적 조명 초기화 ( 설정 갯수 )
	int	InitDynLight( int nl );
	//모든 버텍스에 조명값을 추가 한다
	int AddDynLight( int type , POINT3D *LightPos , int r, int g, int b, int Range );

	//######################################################################################
	//작 성 자 : 오 영 석
	int CheckFaceIceFoot( POINT3D *Pos, POINT3D *Angle, int CheckHeight );
	//######################################################################################

	//임시 페이스 목록을 작성한다
	int MakeAreaFaceList( int x,int z, int width, int height , int top , int bottom );

	//페이스에 해당하는 높이를 구한다
	int GetPolyHeight( smSTAGE_FACE *face , int x, int z );
	//영역에 해당하는 페이스를 검사하여 높이를 구한다
	int GetAreaHeight( int ax, int az , int x, int z );
	//영역에 해당하는 페이스를 검사하여 높이를 구한다 ( 반투명 값도 유효 처리 )
	int GetAreaHeight2( int ax, int az , int x, int z );

	//페이스에 해당하는 높이를 구한다
	int GetHeight( int x, int z  );
	//페이스에 해당하는 높이를 구한다
	int GetHeight2( int x, int z );

	//이동할 위치의 장애물을 검사한다
	int CheckSolid( int sx, int sy, int sz , int dx, int dy, int dz );

	int GetThroughPlane( smSTAGE_FACE *face , POINT3D *sp , POINT3D *ep );

	//평면과 1점의 위치의 값을 구함 ( 평면 방정식 )
	int GetPlaneProduct( smSTAGE_FACE *face , POINT3D *p );
	//각 라인이 페이스를 관통하는지 검사한다 ( 모든 선분이 관통하지 않을 경우 TRUE 그외의 경우 FALSE )
	int GetTriangleImact( smSTAGE_FACE *face, smLINE3D *pLines , int LineCnt );
	//이동할 방향과 위치를 검사하여 좌표를 돌려준다
	int CheckNextMove( POINT3D *Posi, POINT3D *Angle , POINT3D *MovePosi, int dist , int ObjWidth , int ObjHeight , int CheckOverLap=0 );

	//지정된 방향의 거리를 잇는 선분에 폴리건이 충돌하는지 확인 
	int CheckVecImpact( POINT3D *Posi, POINT3D *Angle , int dist );
	//지정된 방향의 거리를 잇는 선분에 폴리건이 충돌하는지 확인 
	int CheckVecImpact2( int sx,int sy,int sz, int ex,int ey,int ez );

	//현재 위치에서의 바닥 높이를 구한다
	int GetFloorHeight( int x, int y, int z , int ObjHeight );
	//현재 위치에서의 바닥 높이를 구한다
	int GetFloorHeight2( int x, int y, int z , int ObjHeight );
	//현재 높이에 겹치는 폴리곤이 있는 지 확인후 있으면 전체 높이를 구한다
	int GetEmptyHeight( int x, int y, int z , int ObjHeight );

	//바닥 폴리곤 면에 y==height 와 일치하는지 확인
	int CheckFloorFaceHeight( int x, int y, int z , int hSize );


	// StageArea를 0 으로 초기화
	void clearStageArea();
	// 삼각페이스가 위치한 구역(64x64) 을 구하여 AreaList 에 설정 
	int getPolyAreas( POINT3D *ip1 , POINT3D *ip2, POINT3D *ip3 );
	//각 폴리곤이 위치한 영역 정보를 만들어 저장한다
	int SetupPolyAreas();
	// 렌더링을 위한 연산과 정렬
	int RenderGeom();

	//카메라 위치에 맞춰서 그린다 
	int DrawStage(int x , int y, int z, int angX, int angY, int angZ , smEMATRIX *eRotMatrix=0 );
	//카메라 위치에 맞춰서 그린다 ( 포함된 오브젝트 제외 )
	int DrawStage2(int x , int y, int z, int angX, int angY, int angZ , smEMATRIX *eRotMatrix=0 );

	//######################################################################################
	//작 성 자 : 오 영 석
	int DrawOpeningStage(int x, int y, int z, int angX, int angY, int angZ, int FrameStep );
	//######################################################################################

	//데이타를 파일로 저장
	int	SaveFile( char *szFile );
	//데이타를 파일로 부터 읽음
	int	LoadFile( char *szFile );


};

#ifdef PT_GFX_GL
// Real-time shadows: uploads the map's mesh for casting once per loaded map
// (shadow mesh slot 0/1); call every frame before PtGfx_ShadowFrame.
int smStage_ShadowMesh( smSTAGE3D *st, int slot );
#endif




