
#define CHARACTER_USER_MAX			6		// sPLAY_CHAR_MAX (record.h): characters per account
#define CHARACTER_JOB_MAX			6		// classes per race on the job screen: Tempskron 6 (with the Assassin and the Martial Artist), Morion 5 (with the Shaman)


#define CHARACTER_SELECT_STATE		0
#define RACE_SELECT_STATE			1
#define JOB_SELECT_STATE			2
#define FACE_SELECT_STATE			3
#define NAME_SELECT_STATE			4
#define COMPLETE_SELECT_STATE		5
#define START_GAME_STATE			6
#define RETURN_OPENING_STATE        7


#define TEMPSCRON					0
//캐릭터 선택 클래스



//ktj 넣음.
#define TRIBE_Tempscron 0		//뎀스크론 
#define TRIBE_Moryon	1		//모라이언 선택

struct HoNewCharInfo
{
	int	  JobCode;			//직업..
	int   Strength;			//힘 
	int   Spirit;			//정신력 
	int   Talent;			//재능 
	int   Dexterity;		//민첩성 
	int   Health;			//건강 
	
	
};

/*
struct HoPosImage
{
	int left, int top, int width, int height;
};
*/
//#define		ERROR_NONE					5
#define		ERROR_NONAME_NEWPLAYER		10	//캐릭터 생성 갯수 가 5명을 넘었을 경우..
#define		ERROR_MAX_NEWPLAYER			20	//캐릭터의 이름을 입력 하지 않았을 경우..
#define		ERROR_EXHIST_NEWPLAYER		30  //캐릭터의 이름이 존재함....
#define		ERROR_SELECT_CHAR			40	//캐릭터 선택을 안함
#define		ERROR_SELECT_RACE			50  //종족 선택을 안함.
#define		ERROR_SELECT_JOB			60  //직업 선택을 안함.
#define		ERROR_SELECT_FACE			80  //캐릭터 얼굴을 선택 안함.
#define     ERROR_DELETE_CLAN_MEMBER    90  //클랜원일 경우 지우지 않음.
#define     ERROR_CLAN_SERVER_NONE     100  //클랜 서버를 확인할수 없음.

#define     SUB_SELECT_NONE					0
#define		SUB_SELECT_CHARACTER_MANAGE		1 //캐릭터 관리.
#define		SUB_SELECT_CHARACTER_USER		2 //User 캐릭터 시작.
#define		SUB_SELECT_CHARACTER_NEW		3 //New 캐릭터 시작.
#define		SUB_SELECT_CHARACTER_NEW_SERVER	4 //서버에서 정보를 받아올 경우.
#define		SUB_SELECT_BACK					5 //Opening으로 돌아갈 경우.

class HoLogin
{
public:

	//ktj: 이부분은 모두 ktj가 만든부분임. =================================== start
	

	int tribeNum;			//종족번호
	RECT menuPos[2];		//메뉴셋팅.
	int						MoryonJobLogoImage[4];	//TempscronJobLogo Image 번호..

							//모라연 얼굴그림.
	int						MorFaceImage[CHARACTER_JOB_MAX][4][2];	//0:Fighter 1:Mechanician 2:Archer 3: Pikeman //얼굴의 종류.. //0:옆얼굴 1: 정면얼굴.


	int USE_MOR;			//기본은 1이며 Init_UseMor() 에서 셋팅됨.
	void Init_UseMor(int flag);	//0 : 모라이언선택안됨. 1:모라이언선택됨.

	//ktj: 이부분은 모두 ktj가 만든부분임. =================================== end
	
	BOOL					ClanMarkUpdateSuccessFlag;

	
	//새로 들어간것들..
	//게임화면 장식 테두리
	int			MainBoxFrameImage1[4];	 //0:mainbox_L.tga   1:mainbox_T.tga   2:mainbox_R.tga   3:mainbox_D.tga
	int			MainBoxFrameImage2[4];   //0:mainbox_CN1.tga 1:mainbox_CN2.tga 2:mainbox_CN3.tga 3:mainbox_CN4.tga
	int			TitleBoxFrameImage;		 //머리위에 있는 타이틀 이름
	int			TitleNameImage;			 //타이틀에 들어가 제목..
	
	//종족 마크 이미지
	int						TempscronMarkImage;	//TempscronMark Image 번호.
	int						TempscronLogoImage;	//TempscronLogo Image 번호.

	int						MoryonMarkImage;	//MoryonMark Image 번호.
	int						MoryonLogoImage;	//MoryonLogo Image 번호.

	int						TempscronTextImage;
	int						MoryonTextImage;

	int						RaceTextBoxImage[2];
	int						JobTextBoxImage[2];
	
	int						TempscronJobImage[CHARACTER_JOB_MAX][2];
	int						MoryonJobImage[CHARACTER_JOB_MAX][2];
	int						TempscronJobTextImage[CHARACTER_JOB_MAX];
	int						MoryonJobTextImage[CHARACTER_JOB_MAX];


	int						FaceBoxBigImage[2];
	int						FaceBoxSmallImage[3];

	BOOL					FaceSelectFirstFlag;
	int						DrawTitle(int state);

	int						ErrorDelayCount;
	BOOL					StartErrorDelayFlag;

	int						m_ScreenHalf;
private:
	//Stage 관련.
	smSTAGE3D	*Stage;
	POINT3D		CaAngle;
	POINT3D		TarPosi;
	int			TarDist;
	
	BOOL		CharacterStopFlag;
	BOOL		CharacterReturnFlag;

	//Login 상태변화 관련.
	int SelectCurrentState;		//현재 상태
	int SelectNextState;		//다음 상태
	
	int  SelectSubState;			//세부 설정시...			
	//BOOL StartBackOpeningFlag;
	BOOL FocusWndFlag; 
	
	//에러들...
	int		ErrorSelect;
	
	//상태변화 따른 버튼들.
	HoButton				OkButton;
	HoButton				BackButton;
	HoButton				QuitButton;
	HoButton				DeleteButton;
	HoButton				NewCharButton;
	
	
	//User Character
	smCHAR					UserCharacter[CHARACTER_USER_MAX];		//User가 만든 캐릭터.
	int						UserCharacterCount;
	//Job 선택시 나오는 캐릭터들.
	smCHAR					JobCharacter[CHARACTER_JOB_MAX];
	int						JobCharacterCount;
	
	//상태에 대한 표시 이미지.
	//LPDIRECTDRAWSURFACE4	CharacterSelectImage;
	int						NewCharacterImage;
	float					NewCharacterImageBlendCount;
	bool					NewCharacterImageBlendFlag;
	
	int						UserCharacterInfoImage;
	int						UserCharacterInfoBackImage;					

	//Race Select
	//LPDIRECTDRAWSURFACE4	RaceSelectImage;
	//int						TempscronMarkImage;	//TempscronMark Image 번호.
	//int						TempscronLogoImage;	//TempscronLogo Image 번호.
	
	//Job Select
	//LPDIRECTDRAWSURFACE4	JobSelectImage;
	//int						TempscronJobLogoImage[4];	//TempscronJobLogo Image 번호..

	

	//Face Select 2d 얼굴 이미지들...
	LPDIRECTDRAWSURFACE4	FaceSelectImage;
	//int						FaceFrameImage;
	int						TempFaceImage[CHARACTER_JOB_MAX][4][2];	//0:Fighter 1:Mechanician 2:Archer 3: Pikeman //얼굴의 종류.. //0:옆얼굴 1: 정면얼굴.
	

	//Name Select
	//LPDIRECTDRAWSURFACE4	NameSelectImage;
	
	
	int						MouseDownCharacterNum;
	int						MouseMoveCharacterNum;
	
	int						JobBodySelectNum; //몸통 선택 부분..
	int						JobFaceSelectNum; //머리 선택 부분..
	char					UserCharacterNewName[256];
	
	//MapImage
	LPDIRECTDRAWSURFACE4	GameMapImage;
	int						MapBlendCount;
	BOOL					MapBlendFlag;
	
	//메시지 출력..
	HoMessage				Message;
	HoMessageBox			MessageBox;


	//공용 이미지(게임 창틀)
	//int						ZoomFaceFrameImage[3];
	//int						GameFrameImage[6];
	int						BlackBoxImage;
	//int						FaceBoxImage;

	HFONT					Font;
	
	//상태에 따라 변화 되는것들.
	int						RaceMarkBlendCount;
	
	int						RaceMarkScaleSize;

	int						LoginVolume;						
public:
	int		Load();		//초기화 캐릭터 선택
	int		Init();
	int		Close();	//종료 캐릭터 선택
	int		Main();		//메인함수 캐릭터 선택
	int		Draw();		//그리기 함수 캐릭터 선택
	
	
	int		SetChangeJobFace(char *charFile1, char *chrFile2, int jobIndex);
	
	int		InitUserCharacter();
	int		DrawUserCharacter(int x, int y, int z, int ax, int ay, int az);
	int		DrawJobCharacter(int x, int y, int z, int ax, int ay, int az);
	int		DrawGameFrame();
	int		DrawLight();
	
	int		DrawMap(int alpha);
	int		DrawCharacterInfo();
	int		MainDefaultButton();
	
	int		KeyFilter();
	
	//Fade 관련..
	//FadeIn, FadeOut관련.
	BOOL		FadeCurrentState;	//0:아무것도 안보임 fade In을 해야함. 1:화면에 다보임 fade out을 해야함..
	BOOL		FadeNextState;

	int		BlendValue;
	int		DrawFade();
	int		MainFade();
	int		MainFadeIn();
	int		MainFadeOut();
	void	SetFade();
	int		ReLoad();

	HoLogin();
	HoLogin(int sw);
	~HoLogin();
private:
	int		DrawSelectSubState();
	int		CameraUpDown();
	int		AddUserCharacter(char *chrFile1, char *chrFile2, int jobCode = 0);
	int		AddJobCharacter(char *chrFile1, char *chrFile2, int index = -1);
	void	SetupJobCharacters(int tribe);		// the race's classes on the job screen
	int		GetSelectCharacter( smCHAR *lpChar );
	void	LoadLoadingImage(int level);

};


//얼굴
extern char *szTempPikeFaceName[2][3];
extern char *szTempArcherFaceName[2][3];
extern char *szTempMechFaceName[2][3];
extern char *szTempFighterFaceName[2][3];

#ifdef _LANGUAGE_BRAZIL
#define MAX_LOADING_IMAGE	6
#else
#define MAX_LOADING_IMAGE	19
#endif