/*----------------------------------------------------------------------------*
*	파일명 :  AssaUtil.cpp	
*	하는일 :  이것저것 잡다한거 
*	작성일 :  최종업데이트 2002년 12월
*	적성자 :  박상열 
*-----------------------------------------------------------------------------*/	

#include "sinLinkHeader.h"

/*----------------------------------------------------------------------------*
* fileNmae: 찾을 파일명
* Return: 찾은 파일의 전체 경로명이 넘어 온다.
*-----------------------------------------------------------------------------*/	
#define MAX_RES_PATH	300
#define MAX_RES_DRIVE   200
#define MAX_FIND_FILE   300
char FindFileName[MAX_FIND_FILE];

char *AssaErrorFile   = "AssaErrorFile.txt";
char *AssaLoadingFile = "AssaLoadingFile.txt";

BOOL FindFileRecursive(char *path, char *fileName)
{
	HANDLE hSrch;
	WIN32_FIND_DATA wfd;
	BOOL bResult = TRUE;
	char drive[MAX_RES_DRIVE];
	char dir[MAX_RES_PATH];
	char newpath[MAX_RES_PATH];

	hSrch = FindFirstFile(path,&wfd);
	while (bResult) 
	{
		_splitpath(path,drive,dir,NULL,NULL);
		if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			if (wfd.cFileName[0] != '.') 
			{
				wsprintf(newpath,"%s%s%s\\*.*",drive,dir,wfd.cFileName);
				if(FindFileRecursive(newpath, fileName))
					return TRUE;
			}
		} 
		else 
		{
			if(stricmp(wfd.cFileName, fileName)== 0)
			{
				wsprintf(FindFileName, "%s%s%s", drive, dir, wfd.cFileName);
				return TRUE;
			}
		}
		bResult = FindNextFile(hSrch, &wfd);
	}

	FindClose(hSrch);
	return FALSE;
}


char *FindFile(char *fileName)
{
	char Path[MAX_RES_PATH];
	GetCurrentDirectory(MAX_RES_PATH, Path);
	strcat(Path, "\\image\\Sinimage\\TestEffect\\*.*"); //여기가 찾을 디렉토리의 최상위 디렉토리 설정.
	

	memset(FindFileName, 0, sizeof(FindFileName));
	FindFileRecursive(Path, fileName);
	return FindFileName;
}

#define MAX_RES_NAME	  64
#define MAX_RES_FULL_NAME 512
#define MAX_RES_BUFFER    1000			//리소스 버퍼의 크기..

#define TYPE_MAT        100
#define TYPE_ASE        200
#define TYPE_BONE       300
#define TYPE_BONE_ANI   400				//본에니가 들어있는 Res

struct cASSARES
{
	void *res;
	char resName[MAX_RES_NAME];
	char resFullName[MAX_RES_FULL_NAME];
	int  type;
	BOOL LoadFlag;
};


cASSARES AssaRes[MAX_RES_BUFFER];
int		 AssaResCount;


//파일을 읽었는지 안읽었는지 검사하는 함수.
bool GetLoadedFile(char *searchFileName) //true이면 읽은 파일 false이면 읽지 않은 파일..
{
	for(int index = 0; index < AssaResCount; index++)
	{
		if(AssaRes[index].type == TYPE_ASE ||
		   AssaRes[index].type == TYPE_BONE ||
		   AssaRes[index].type == TYPE_BONE_ANI)
		{
			if(stricmp(searchFileName, AssaRes[index].resName) == 0)
				return true;
		}
	}

	 return false;
}

//Ase 파일이 있는지 검사한다.
bool ExistAseFile(char *directory)
{
	HANDLE hSrch;
	WIN32_FIND_DATA wfd;
	BOOL bResult = TRUE;
	
	char tempFileName[512];
	char tempExtName[32];

	hSrch = FindFirstFile(directory, &wfd);
	while (bResult)
	{
		strcpy(tempFileName, wfd.cFileName);
		
		for(int index = strlen(tempFileName)-1; index > 0; index--)
		{
			if(tempFileName[index] == '.')
			{
				tempFileName[index] = '\0';
				strcpy(tempExtName, &tempFileName[index+1]);
				break;
			}
		}
		
		//Ase인경우.
		if( stricmp(tempExtName, "ase") == 0 || stricmp(tempExtName, "smd") == 0)
			return true;

		bResult = FindNextFile(hSrch, &wfd);
	}
	FindClose(hSrch);

	return false;
}




void LoadResFile(char *path, bool aseExistFlag)
{
	HANDLE hSrch;
	WIN32_FIND_DATA wfd;
	BOOL bResult = TRUE;
	char drive[MAX_RES_DRIVE];
	char dir[MAX_RES_PATH];
	char newpath[MAX_RES_PATH];
	hSrch = FindFirstFile(path,&wfd);
	
	while (bResult) 
	{
		_splitpath(path,drive,dir,NULL,NULL);
		if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			if (wfd.cFileName[0] != '.') 
			{
				wsprintf(newpath,"%s%s%s\\*.*",drive,dir,wfd.cFileName);
				LoadResFile(newpath, ExistAseFile(newpath));
			}
		} 
		else 
		{
			if(strlen(wfd.cFileName) > 4)
			{
				if(AssaResCount >= MAX_RES_BUFFER)
					return;
				
				char fileDirectoryName[1024];
				memset(fileDirectoryName, 0, sizeof(fileDirectoryName));
				wsprintf(fileDirectoryName, "%s%s%s", drive, dir, wfd.cFileName);
				
				char tempFileName[256];
				char tempExtName[10];
				strcpy(tempFileName, wfd.cFileName);
				
				for(int index = strlen(tempFileName)-1; index > 0; index--)
				{
					if(tempFileName[index] == '.')
					{
						tempFileName[index] = '\0';
						strcpy(tempExtName, &tempFileName[index+1]);
						break;
					}
				}
				
				if(aseExistFlag == false)
				{
					//Bmp인 경우.
					if( stricmp(tempExtName, "bmp") == 0 ||
						stricmp(tempExtName, "tga") == 0)
					{
						if(smConfig.DebugMode)
						{
							FILE *loadFp;
							loadFp = fopen(AssaLoadingFile, "at");
							fprintf(loadFp, "%s\n", fileDirectoryName);
							fclose(loadFp);


							FILE *errorFp;
							errorFp = fopen(AssaErrorFile, "wt");
							fprintf(errorFp, "%s\n", fileDirectoryName);
							fclose(errorFp);
						}
						strcpy(AssaRes[AssaResCount].resName, wfd.cFileName);
						
						AssaRes[AssaResCount].type = TYPE_MAT;
						strcpy(AssaRes[AssaResCount].resFullName, fileDirectoryName);


#ifdef _EFFECT_RES_LOADING_
						if(HoEffectResLoadingFlag)
						{
							if(AssaRes[AssaResCount].LoadFlag == FALSE)
							{
 								int *matNum = new int;
								(*matNum) = CreateTextureMaterial( AssaRes[AssaResCount].resFullName, 0, 0, 0,0, SMMAT_BLEND_LAMP);
								AssaRes[AssaResCount].res = matNum;
								AssaRes[AssaResCount].LoadFlag = TRUE;
								smMaterialGroup->ReadTextures();
							}
						}
#endif
						AssaResCount++;
					}
				}
				
				//Ase인경우.
				if( stricmp(tempExtName, "ase") == 0 || stricmp(tempExtName, "smd") == 0)
				{
					//본인지 구별을 위해서..
					char boneName[8];
					memset(boneName, 0, sizeof(boneName));
					memcpy(boneName, wfd.cFileName, 2);

					char aseName[256];
					memset(aseName, 0, sizeof(aseName)); 
					strcpy(aseName, wfd.cFileName);
					
					for(int index = strlen(aseName)-1; index > 0; index--)
					{
						if(aseName[index] == '.')
						{
							aseName[index] = '\0';
							strcat(aseName, ".ase");
							break;
						}
					}
					
					memset(fileDirectoryName, 0, sizeof(fileDirectoryName));
					wsprintf(fileDirectoryName, "%s%s%s", drive, dir, aseName);
					
					if(GetLoadedFile(aseName) == true) //파일을 읽었는지 검사한다.
					{
						bResult = FindNextFile(hSrch, &wfd);
						continue;
					}
					
					if(smConfig.DebugMode)
					{
						FILE *loadFp;
						loadFp = fopen(AssaLoadingFile, "at");
						fprintf(loadFp, "%s\n", fileDirectoryName);
						fclose(loadFp);


						FILE *errorFp;
						errorFp = fopen(AssaErrorFile, "wt");
						fprintf(errorFp, "%s\n", fileDirectoryName);
						fclose(errorFp);
					}

					if(stricmp(boneName, "b_") == 0)
					{
						strcpy(AssaRes[AssaResCount].resName, aseName);
						strcpy(AssaRes[AssaResCount].resFullName, fileDirectoryName);
						AssaRes[AssaResCount].type = TYPE_BONE_ANI;
						AssaResCount++;
						

						char tempName[64];
						memset(tempName, 0, sizeof(tempName));
						strcpy(tempName, aseName);
						for(int index = 0; index < (int)strlen(tempName); index++)
						{
							if(tempName[index] == '_')
							{
								strcpy(AssaRes[AssaResCount].resName, &tempName[index+1]);
								break;
							}
						}
						strcpy(AssaRes[AssaResCount].resFullName, fileDirectoryName);

#ifdef _EFFECT_RES_LOADING_
						if(HoEffectResLoadingFlag)
						{
							if( (AssaResCount-1)>= 0)
							{

								if(AssaRes[AssaResCount-1].LoadFlag == FALSE)
								{
									//우선 피지크를 읽는다.
									AssaRes[AssaResCount-1].res = (void *)smASE_ReadBone(AssaRes[AssaResCount-1].resFullName);
									AssaRes[AssaResCount-1].LoadFlag = TRUE;
									smASE_SetPhysique( ((smPAT3D *)AssaRes[AssaResCount-1].res) );
								}
								
								//피지크에 모델을 세팅한다.
								AssaRes[AssaResCount].res = (void *)smASE_Read( AssaRes[AssaResCount].resFullName);
								((smPAT3D *)AssaRes[AssaResCount].res)->ZeroNormals();
								smASE_SetPhysique(NULL);

								AssaRes[AssaResCount].LoadFlag = TRUE;
								((smPAT3D *)AssaRes[AssaResCount].res)->smMaterialGroup->ReadTextures();
							}
						}

#endif
						AssaRes[AssaResCount].type = TYPE_BONE;
						AssaResCount++;
					}
					else
					{
						strcpy(AssaRes[AssaResCount].resName, aseName);
						strcpy(AssaRes[AssaResCount].resFullName, fileDirectoryName);

#ifdef _EFFECT_RES_LOADING_
						if(HoEffectResLoadingFlag)
						{
							if(AssaRes[AssaResCount].LoadFlag == FALSE)
							{
								smASE_SetPhysique(NULL);
								AssaRes[AssaResCount].res = (void *)smASE_Read(AssaRes[AssaResCount].resFullName);
								
								((smPAT3D *)AssaRes[AssaResCount].res)->ZeroNormals();
								AssaRes[AssaResCount].LoadFlag = TRUE;
								((smPAT3D *)AssaRes[AssaResCount].res)->smMaterialGroup->ReadTextures();
							}	
						}
#endif
						AssaRes[AssaResCount].type = TYPE_ASE;
						AssaResCount++;
					}
				}
			}
		}
		bResult = FindNextFile(hSrch, &wfd);
	}
	FindClose(hSrch);
}



//첨에 꼭 한번은 호출 해야 함..
void InitAssaRes()
{
	AssaResCount = 0;
	memset(AssaRes, 0, sizeof(AssaRes));
}

//리소스를 지울때..
void CloseAssaRes()
{
	for(int index = 0; index < AssaResCount; index++)
	{
		if(AssaRes[index].res != NULL)
		{
			if(AssaRes[index].type == TYPE_MAT)
			{
				delete ((int *)AssaRes[index].res);
				AssaRes[index].res = NULL;
			}
			if(AssaRes[index].type == TYPE_ASE)
			{
				delete ((smPAT3D *)AssaRes[index].res);
				AssaRes[index].res = NULL;
				memset(AssaRes[index].resName, 0, sizeof(AssaRes[index].resName));
				AssaRes[index].type = 0;
			}
		}
	}
}

/*----------------------------------------------------------------------------*
* 리소스들을 직접 읽는 함수이다.. 
*-----------------------------------------------------------------------------*/	
void LoadAssaRes()
{
	CloseAssaRes();
	char Path[MAX_RES_PATH];
	
	if(smConfig.DebugMode)
		DeleteFile(AssaLoadingFile);
	strcpy(Path, "image\\Sinimage\\AssaEffect\\*.*"); //Sin 리소스 읽기..
	LoadResFile(Path, false);

	strcpy(Path, "Effect\\AssaEffect\\*.*"); //Ho 리소스 읽기..
	LoadResFile(Path, false);
	
	if(smConfig.DebugMode)
		DeleteFile(AssaErrorFile);
#ifdef PT_WEB
	printf( "[assa] %d effect resources registered\n" , AssaResCount );	// was ~10 while sub-folders read as files
#endif


	//ReadTextures();
}



/*----------------------------------------------------------------------------*
* 리소스를 찾아서 리턴하는 함수이다..
* resName: 리소스의 파일 이름(aa.bmp bb.tga cc.ase 등등)
* blendType: 바꾸고 싶은 블렌드 타입..
* return: 메모리상에 있는 리소스의 값을 void* 값으로 넘긴다. 쓸때는 쓰는 형으로
* 변환을 해야 한다. 텍스쳐일 경우 *((int *)Ase)일경우 (smPAT3D *)이다.
*-----------------------------------------------------------------------------*/	
void *AssaSearchRes(char *resName, int blendType)
{
	for(int index = 0; index < AssaResCount; index++)
	{
		if(stricmp(AssaRes[index].resName, resName) == 0)
		{
 			if(AssaRes[index].type == TYPE_ASE)
			{
				if(AssaRes[index].LoadFlag == FALSE)
				{
					smASE_SetPhysique(NULL);
					AssaRes[index].res = (void *)smASE_Read(AssaRes[index].resFullName);
					
					((smPAT3D *)AssaRes[index].res)->ZeroNormals();
					AssaRes[index].LoadFlag = TRUE;
					((smPAT3D *)AssaRes[index].res)->smMaterialGroup->ReadTextures();
					//ReadTextures();
				}

				if(((smPAT3D *)AssaRes[index].res)->smMaterialGroup){ //메트리얼이 있을겨우에만 블렌드 셋팅
					int matCount = ((smPAT3D *)AssaRes[index].res)->smMaterialGroup->MaterialCount;
					for(int i = 0; i< matCount; i++)
						((smPAT3D *)AssaRes[index].res)->smMaterialGroup->smMaterial[i].BlendType = blendType;
				}

				return AssaRes[index].res;
			}
			else if(AssaRes[index].type == TYPE_BONE)
			{
				if(AssaRes[index].LoadFlag == FALSE)
				{
					if( (index-1)>= 0)
					{

						if(AssaRes[index-1].LoadFlag == FALSE)
						{
							//우선 피지크를 읽는다.
							AssaRes[index-1].res = (void *)smASE_ReadBone(AssaRes[index-1].resFullName);
							AssaRes[index-1].LoadFlag = TRUE;
						}

						smASE_SetPhysique( ((smPAT3D *)AssaRes[index-1].res) );
						
						//피지크에 모델을 세팅한다.
						AssaRes[index].res = (void *)smASE_Read( AssaRes[index].resFullName);
						((smPAT3D *)AssaRes[index].res)->ZeroNormals();
						smASE_SetPhysique(NULL);

						AssaRes[index].LoadFlag = TRUE;
						((smPAT3D *)AssaRes[index].res)->smMaterialGroup->ReadTextures();
						//ReadTextures();
					}
				}
					
				if(((smPAT3D *)AssaRes[index].res)->smMaterialGroup){ //메트리얼이 있을경우에만 블렌드 셋팅
					int matCount = ((smPAT3D *)AssaRes[index].res)->smMaterialGroup->MaterialCount;
					for(int i = 0; i< matCount; i++)
						((smPAT3D *)AssaRes[index].res)->smMaterialGroup->smMaterial[i].BlendType = blendType;
				}

				return AssaRes[index].res;
			}
			else if(AssaRes[index].type == TYPE_BONE_ANI)
			{
				if(AssaRes[index-1].LoadFlag == FALSE)
				{
						
					//우선 피지크를 읽는다.
					AssaRes[index-1].res = (void *)smASE_ReadBone(AssaRes[index-1].resFullName);
					AssaRes[index-1].LoadFlag = TRUE;
				}
				return AssaRes[index].res;
			}
			else if(AssaRes[index].type == TYPE_MAT)
			{
				if(AssaRes[index].LoadFlag == FALSE)
				{
 					int *matNum = new int;
					(*matNum) = CreateTextureMaterial( AssaRes[index].resFullName, 0, 0, 0,0, SMMAT_BLEND_LAMP);
					AssaRes[index].res = matNum;
					AssaRes[index].LoadFlag = TRUE;
					smMaterialGroup->ReadTextures();
					//ReadTextures();
				}

				int matNum = *((int *)AssaRes[index].res);
				smMaterialGroup->smMaterial[matNum].TwoSide = TRUE;
				smMaterialGroup->smMaterial[matNum].BlendType  = blendType;
				return AssaRes[index].res;
			}

		}
	}

	return NULL;
}


#define FLOATS_EM				(FLOATNS+3)
#define SM_DIST_NEARZ			16
#define SMFLOAT_DIST_NEARZ		(SM_DIST_NEARZ<<FLOATNS)

bool AssaGetCameraCoord(POINT3D &world)
{
	int rx, ry, rz;

    //중심 좌표를 카메라 좌표로 변환
	world.x -= smRender.CameraPosiX;
	world.y -= smRender.CameraPosiY;
	world.z -= smRender.CameraPosiZ;
	
	rx = world.x * smRender.mCamera._11 +
		 world.y * smRender.mCamera._21 +
		 world.z * smRender.mCamera._31 ;
	
	ry = world.x * smRender.mCamera._12 +
		 world.y * smRender.mCamera._22 +
		 world.z * smRender.mCamera._32 ;
	
	rz = world.x * smRender.mCamera._13 +
		 world.y * smRender.mCamera._23 +
		 world.z * smRender.mCamera._33 ;

	world.x = rx>>FLOATS_EM;
	world.y = ry>>FLOATS_EM;
	world.z = rz>>FLOATS_EM;

    if ( world.z<SMFLOAT_DIST_NEARZ )
		return FALSE;
	return TRUE;
}

/*----------------------------------------------------------------------------*
* 2d Billboard를 찍는 함수
* ZPosiFlag: TRUE이면 빌보드를 앞쪽으로 빼서 찍는다. FALSE이면 원래 좌표대로 찍
* 는다.
*-----------------------------------------------------------------------------*/
bool AssaAddFace2D ( smFACE2D *face , int ZPosiFlag, int angle)
{
	int x,y,z;
	int rx , ry, rz;
	int width , height;

	smRENDVERTEX *v[4];
	smRENDFACE	 *rf;
	short		sColor[4];

	sColor[ SMC_A ] = face->Transparency;
	sColor[ SMC_R ] = face->r;
	sColor[ SMC_G ] = face->g;
	sColor[ SMC_B ] = face->b;

	x = face->x;
	y = face->y;
	z = face->z;

	//중심 좌표를 카메라 좌표를 변환
	x -= smRender.CameraPosiX;
	y -= smRender.CameraPosiY;
	z -= smRender.CameraPosiZ;

	rx = x * smRender.mCamera._11 +
		 y * smRender.mCamera._21 +
		 z * smRender.mCamera._31 ;

	ry = x * smRender.mCamera._12 +
		 y * smRender.mCamera._22 +
		 z * smRender.mCamera._32 ;

	rz = x * smRender.mCamera._13 +
		 y * smRender.mCamera._23 +
		 z * smRender.
		 mCamera._33 ;

	x = rx>>FLOATS_EM;
	y = ry>>FLOATS_EM;
	z = rz>>FLOATS_EM;

//	z -= 12*fONE;
	if(ZPosiFlag)
		z -= 24*fONE;


	if ( z<SMFLOAT_DIST_NEARZ ) return false;

	width  = face->width >> 1;
	height = face->height >> 1;
	
	if(angle != 0)
	{
		//회전 적용
		int		ex[4],ey[4];
		int		mx,my;
		int		sin,cos;

		sin = GetSin[angle&ANGCLIP]>>FLOATNS;
		cos = GetCos[angle&ANGCLIP]>>FLOATNS;

		mx = -width;	my = -height;
		ex[0] = (mx * cos - my * sin) >> FLOATNS;
		ey[0] = (mx * sin + my * cos) >> FLOATNS;

		mx = width;	my = -height;
		ex[1] = (mx * cos - my * sin) >> FLOATNS;
		ey[1] = (mx * sin + my * cos) >> FLOATNS;

		mx = -width;	my = +height;
		ex[2] = (mx * cos - my * sin) >> FLOATNS;
		ey[2] = (mx * sin + my * cos) >> FLOATNS;

		mx = width;	my = height;
		ex[3] = (mx * cos - my * sin) >> FLOATNS;
		ey[3] = (mx * sin + my * cos) >> FLOATNS;
	

		//렌더링 버텍스 추가 ( 좌표 직접 입력 )
		v[0] = smRender.AddRendVertex ( x+ex[0] , y+ey[0] , z, sColor );
		v[1] = smRender.AddRendVertex ( x+ex[1] , y+ey[1] , z, sColor );
		v[2] = smRender.AddRendVertex ( x+ex[2] , y+ey[2] , z, sColor );
		v[3] = smRender.AddRendVertex ( x+ex[3] , y+ey[3] , z, sColor );
	}
	else
	{
		//렌더링 버텍스 추가 ( 좌표 직접 입력 )
		v[0] = smRender.AddRendVertex ( x-width , y-height , z, sColor );
		v[1] = smRender.AddRendVertex ( x+width , y-height , z, sColor );
		v[2] = smRender.AddRendVertex ( x-width , y+height , z, sColor );
		v[3] = smRender.AddRendVertex ( x+width , y+height , z, sColor );
	}

	//첫번째 폴리곤 ( 1 - 2 - 3 );
	rf = &smRender.RendFace[ smRender.nRendFace++ ];
	rf->lpRendVertex[0] = v[0];
	rf->lpRendVertex[1] = v[1];
	rf->lpRendVertex[2] = v[2];
	rf->Matrial    = face->MatNum ;		//메트리얼 복사
	rf->ClipStatus = v[0]->ClipStatus|v[1]->ClipStatus|v[2]->ClipStatus;
	rf->lpTexLink  = 0;

	//텍스쳐 연결
	smRender.AddRendTempTexLink( rf , 0,	face->TexRect.left , face->TexRect.bottom,
								face->TexRect.right , face->TexRect.bottom,
								face->TexRect.left , face->TexRect.top );


	//두번째 폴리곤 ( 2 - 4 - 3 );
	rf = &smRender.RendFace[ smRender.nRendFace++ ];
	rf->lpRendVertex[0] = v[1];
	rf->lpRendVertex[1] = v[3];
	rf->lpRendVertex[2] = v[2];
	rf->Matrial    = face->MatNum ;		//메트리얼 복사
	rf->ClipStatus = v[1]->ClipStatus|v[2]->ClipStatus|v[3]->ClipStatus;
	rf->lpTexLink  = 0;

	//텍스쳐 연결
	smRender.AddRendTempTexLink( rf , 0,	face->TexRect.right , face->TexRect.bottom,
								face->TexRect.right , face->TexRect.top,
								face->TexRect.left , face->TexRect.top );


	return true;
}

/*----------------------------------------------------------------------------*
* 2d Billboard를 destPos위치 가지 늘려서 그리는 함수이다.
* face의 width: Billboad의 넓이이고 height: 쓰지 않는 값이다..
* localDestPos: Local좌표에서 늘어나는 값이다. 현재 위치에서 늘어나는 크기이다..
* centerFlag: 페이스가 늘어나는 위치가 센타에서 부터인지..
*-----------------------------------------------------------------------------*/	
bool AssaAddFace2DLine(smFACE2D *face, POINT3D *localDestPos, BOOL centerFlag)
{
    int x,y,z;
    int width;

    smRENDVERTEX *v[4];
    smRENDFACE	 *rf;
    short		sColor[4];

    sColor[ SMC_A ] = face->Transparency;
    sColor[ SMC_R ] = face->r;
    sColor[ SMC_G ] = face->g;
    sColor[ SMC_B ] = face->b;

    x = face->x;
    y = face->y;
    z = face->z;

    POINT3D currentPos;
	POINT3D destPos;
    width = face->width/2;
	
	currentPos.x   = x;
    currentPos.y   = y;
    currentPos.z   = z;
	
	destPos.x = localDestPos->x;
	destPos.y = localDestPos->y;
	destPos.z = localDestPos->z;

	if(centerFlag == FALSE)
	{
		destPos.x = currentPos.x+destPos.x;
		destPos.y = currentPos.y+destPos.y;
		destPos.z = currentPos.z+destPos.z;
	}
	else
	{
		POINT3D centerDestPos;
		memcpy(&centerDestPos, &destPos, sizeof(centerDestPos));
		centerDestPos.x = centerDestPos.x / 2;
		centerDestPos.y = centerDestPos.y / 2;
		centerDestPos.z = centerDestPos.z / 2;

		destPos.x = currentPos.x+centerDestPos.x;
		destPos.y = currentPos.y+centerDestPos.y;
		destPos.z = currentPos.z+centerDestPos.z;

		currentPos.x = currentPos.x - (centerDestPos.x);
		currentPos.y = currentPos.y - (centerDestPos.y);
		currentPos.z = currentPos.z - (centerDestPos.z);
	}


	
  	if(AssaGetCameraCoord(currentPos) == FALSE)
			return FALSE;

    if(AssaGetCameraCoord(destPos) == FALSE)
			return FALSE;
	
	//AssaGetCameraCoord(currentPos);
	//AssaGetCameraCoord(destPos);

	sColor[ SMC_A ] = face->Transparency;
	sColor[ SMC_R ] = face->r;
	sColor[ SMC_G ] = face->g;
	sColor[ SMC_B ] = face->b;

    smTEXRECT texRect;
    texRect.left = 0;
    texRect.top  = 0;
    texRect.right = 1;
    texRect.bottom = 1;

    float dx     = float(destPos.x - currentPos.x); //대상 까지의 x축의 기울기를 구한다.
    float dy     = float(destPos.y - currentPos.y); //대상 까지의 y축의 기울기를 구한다.
    float length = (float)sqrt(dx*dx+dy*dy);        //대상 까지의 거리를 구한다.

    dx = dx/length*width; //날아가는 파티클의 넓이..
    dy = dy/length*width;

    D3DVECTOR persp;
    persp.x = -dy;
    persp.y = +dx;
    persp.z = 0;

    if (abs((int)dx) > 1.0f) currentPos.x -= (int)dx;
    if (abs((int)dy) > 1.0f) currentPos.y -= (int)dy;

    v[0] = smRender.AddRendVertex ( int(destPos.x-persp.x),    int(destPos.y-persp.y), int(destPos.z), sColor );
    v[1] = smRender.AddRendVertex ( int(destPos.x+persp.x),    int(destPos.y+persp.y), int(destPos.z), sColor );
    v[2] = smRender.AddRendVertex ( int(currentPos.x-persp.x), int(currentPos.y-persp.y), int(currentPos.z), sColor );
    v[3] = smRender.AddRendVertex ( int(currentPos.x+persp.x), int(currentPos.y+persp.y), int(currentPos.z), sColor );

    //첫번째 폴리곤 ( 1 - 2 - 3 );
    rf = &smRender.RendFace[ smRender.nRendFace++ ];
    rf->lpRendVertex[0] = v[0];
    rf->lpRendVertex[1] = v[1];
    rf->lpRendVertex[2] = v[2];
    rf->Matrial    = face->MatNum;		//메트리얼 복사
    rf->ClipStatus = v[0]->ClipStatus|v[1]->ClipStatus|v[2]->ClipStatus;
    rf->lpTexLink  = 0;

    //텍스쳐 연결
    smRender.AddRendTempTexLink( rf , 0,	texRect.left , texRect.bottom,
                                texRect.right , texRect.bottom,
                                texRect.left , texRect.top );


    //두번째 폴리곤 ( 2 - 4 - 3 );
    rf = &smRender.RendFace[ smRender.nRendFace++ ];
    rf->lpRendVertex[0] = v[1];
    rf->lpRendVertex[1] = v[3];
    rf->lpRendVertex[2] = v[2];
    rf->Matrial    = face->MatNum;		//메트리얼 복사
    rf->ClipStatus = v[1]->ClipStatus|v[2]->ClipStatus|v[3]->ClipStatus;
    rf->lpTexLink  = 0;

    //텍스쳐 연결
    smRender.AddRendTempTexLink( rf , 0,	texRect.right , texRect.bottom,
                                            texRect.right , texRect.top,
                                            texRect.left  , texRect.top );
	return true;
}


/*----------------------------------------------------------------------------*
* World에 판하나를 그리는 함수이다.(판의 방향은 위로 향한다).
*-----------------------------------------------------------------------------*/
bool AssaAddFaceWorld(smFACE2D *face, POINT3D *inAngle)
{
	int x,y,z;
    int width , height;

    smRENDVERTEX *v[4];
    smRENDFACE	 *rf;
    short		sColor[4];

    sColor[ SMC_A ] = face->Transparency;
    sColor[ SMC_R ] = face->r;
    sColor[ SMC_G ] = face->g;
    sColor[ SMC_B ] = face->b;

    x = face->x;
    y = face->y;
    z = face->z;

    POINT3D inVertex[4];
    POINT3D outVertex[4];

    width = face->width/2;
    height = face->height/2;

	inVertex[0].x   = x-width;
    inVertex[0].y   = y;
    inVertex[0].z   = z+height;

    inVertex[1].x   = x+width;
    inVertex[1].y   = y;
    inVertex[1].z   = z+height;

    inVertex[2].x   = x-width;
    inVertex[2].y   = y;
    inVertex[2].z   = z-height;

    inVertex[3].x   = x+width;
    inVertex[3].y   = y;
	inVertex[3].z   = z-height;
	
	int index = 0;
	if(inAngle != NULL)
	{
		inVertex[0].x   = -width;
		inVertex[0].y   = 0;
		inVertex[0].z   = height;

		inVertex[1].x   = width;
		inVertex[1].y   = 0;
		inVertex[1].z   = height;

		inVertex[2].x   = -width;
		inVertex[2].y   = 9;
		inVertex[2].z   = -height;

		inVertex[3].x   = width;
		inVertex[3].y   = 0;
		inVertex[3].z   = -height;

		POINT3D angle;
		angle.x = (int)inAngle->x;
		angle.y = (int)inAngle->y;
		angle.z = (int)inAngle->z;

		angle.x = angle.x & ANGCLIP;
		angle.y = angle.y & ANGCLIP;
		angle.z = angle.z & ANGCLIP;

		smMATRIX inRotXMatrix;
		smMATRIX inRotYMatrix;
		smMATRIX inRotZMatrix;

		smMATRIX outMatrix;
		smIdentityMatrix(outMatrix);
		smIdentityMatrix(inRotXMatrix);
		smIdentityMatrix(inRotYMatrix);
		smIdentityMatrix(inRotZMatrix);

		smRotateXMatrix(inRotXMatrix, angle.x);
		smRotateYMatrix(inRotYMatrix, angle.y);
		smRotateZMatrix(inRotZMatrix, angle.z);

		smMatrixMult(outMatrix, inRotXMatrix, inRotYMatrix);
		smMatrixMult(outMatrix, outMatrix, inRotZMatrix);
		
		for(index = 0; index < 4; index++)
		{
			outVertex[index].x = inVertex[index].x * outMatrix._11 +
								 inVertex[index].y * outMatrix._21 +
								 inVertex[index].z * outMatrix._31 ;
			

			outVertex[index].y = inVertex[index].x * outMatrix._12 +
								 inVertex[index].y * outMatrix._22 +
								 inVertex[index].z * outMatrix._32 ;
			

			outVertex[index].z = inVertex[index].x * outMatrix._13 +
								 inVertex[index].y * outMatrix._23 +
								 inVertex[index].z * outMatrix._33 ;

			outVertex[index].x = outVertex[index].x/fONE+x;
			outVertex[index].y = outVertex[index].y/fONE+y;
			outVertex[index].z = outVertex[index].z/fONE+z;
		
		}
		
		memcpy(inVertex, outVertex, sizeof(inVertex));
	}

	//int index;
	for(index = 0; index < 4; index++)
    {
        inVertex[index].x -= smRender.CameraPosiX;
        inVertex[index].y -= smRender.CameraPosiY;
        inVertex[index].z -= smRender.CameraPosiZ;
    }

	for(index = 0; index < 4; index++)
    {
        outVertex[index].x = inVertex[index].x*smRender.mCamera._11 +
                             inVertex[index].y*smRender.mCamera._21 +
                             inVertex[index].z*smRender.mCamera._31;

        outVertex[index].y = inVertex[index].x*smRender.mCamera._12 +
                             inVertex[index].y*smRender.mCamera._22 +
                             inVertex[index].z*smRender.mCamera._32;

        outVertex[index].z = inVertex[index].x*smRender.mCamera._13 +
                             inVertex[index].y*smRender.mCamera._23 +
                             inVertex[index].z*smRender.mCamera._33;

        outVertex[index].x = outVertex[index].x >> FLOATS_EM;
        outVertex[index].y = outVertex[index].y >> FLOATS_EM;
        outVertex[index].z = outVertex[index].z >> FLOATS_EM;
		
		if(outVertex[index].z < SMFLOAT_DIST_NEARZ)
            return false;
    }
	

	//렌더링 버텍스 추가 ( 좌표 직접 입력 )
    v[0] = smRender.AddRendVertex ( outVertex[0].x , outVertex[0].y , outVertex[0].z, sColor);
    v[1] = smRender.AddRendVertex ( outVertex[1].x,  outVertex[1].y , outVertex[1].z, sColor);
    v[2] = smRender.AddRendVertex ( outVertex[2].x , outVertex[2].y , outVertex[2].z, sColor);
    v[3] = smRender.AddRendVertex ( outVertex[3].x , outVertex[3].y , outVertex[3].z, sColor);

		//첫번째 폴리곤 ( 1 - 2 - 3 );
		rf = &smRender.RendFace[ smRender.nRendFace++ ];
		rf->lpRendVertex[0] = v[0];
		rf->lpRendVertex[1] = v[1];
		rf->lpRendVertex[2] = v[2];
		rf->Matrial    = face->MatNum ;		//메트리얼 복사
		rf->ClipStatus = v[0]->ClipStatus|v[1]->ClipStatus|v[2]->ClipStatus;
		rf->lpTexLink  = 0;

		//텍스쳐 연결
		smRender.AddRendTempTexLink( rf , 0,	face->TexRect.left , face->TexRect.bottom,
									face->TexRect.right , face->TexRect.bottom,
									face->TexRect.left , face->TexRect.top );


		//두번째 폴리곤 ( 2 - 4 - 3 );
		rf = &smRender.RendFace[ smRender.nRendFace++ ];
		rf->lpRendVertex[0] = v[1];
		rf->lpRendVertex[1] = v[3];
		rf->lpRendVertex[2] = v[2];
		rf->Matrial    = face->MatNum ;		//메트리얼 복사
		rf->ClipStatus = v[1]->ClipStatus|v[2]->ClipStatus|v[3]->ClipStatus;
		rf->lpTexLink  = 0;

		//텍스쳐 연결
		smRender.AddRendTempTexLink( rf , 0,	face->TexRect.right , face->TexRect.bottom,
									face->TexRect.right , face->TexRect.top,
									face->TexRect.left , face->TexRect.top );
    return true;
}


/*----------------------------------------------------------------------------*
* 좌표의 리스트를 잇는 폴리곤이다.
*-----------------------------------------------------------------------------*/
bool AssaAddFaceTrace(smFACE2D *face, cASSATrace *assaTrace)
{
    if(face == NULL || assaTrace == NULL)
        return false;
	
	if(assaTrace->TraceList.size() < 2)
		return FALSE;
    
	POINT3D destPos;
	POINT3D currentPos;
	
	std::list<POINT3D>::iterator i;
	i = assaTrace->TraceList.begin();
    destPos.x = i->x;
    destPos.y = i->y;
    destPos.z = i->z;

    int texSizeCount = 0;
    int test = assaTrace->TraceList.size();
    float cellSize = 1.f/(assaTrace->TraceList.size()-1);
    ++i;
	POINT3D OldVertex[2];
    memset(OldVertex, 0, sizeof(OldVertex));
	
	for (; i != assaTrace->TraceList.end();i++)
    {
		currentPos.x = i->x;
        currentPos.y = i->y;
        currentPos.z = i->z;
			
		if(AssaGetCameraCoord(currentPos) == FALSE)
			    return FALSE;
		
		if(AssaGetCameraCoord(destPos) == FALSE)
			    return FALSE;

        smRENDVERTEX *v[4];
	    smRENDFACE	 *rf;

	    short		sColor[4];

        sColor[ SMC_R ] = face->r;
	    sColor[ SMC_G ] = face->g;
	    sColor[ SMC_B ] = face->b;
		sColor[ SMC_A ] = face->Transparency;

        smTEXRECT texRect;
        texRect.left   = 0;
        texRect.top    = (float)texSizeCount*cellSize;
        texRect.right  = 1;
        texRect.bottom = (float)(texSizeCount+1)*cellSize;


        int size;
        size = (int)face->width;
        //size = size << FLOATNS;
        size = size >> 1;

        float dx     = float(destPos.x - currentPos.x); //대상 까지의 x축의 기울기를 구한다.
		float dy     = float(destPos.y - currentPos.y); //대상 까지의 y축의 기울기를 구한다.
		float length = (float)sqrt(dx*dx+dy*dy);        //대상 까지의 거리를 구한다.

        if(length == 0)
            length = 1;

        dx = dx/length*size; //날아가는 파티클의 넓이..
		dy = dy/length*size;

        D3DVECTOR persp;
        persp.x = -dy;
		persp.y = +dx;
        persp.z = 0;
		
		if(texSizeCount == 0)
		{
			v[0] = smRender.AddRendVertex ( int(destPos.x-persp.x), int(destPos.y-persp.y), int(destPos.z), sColor );
			v[1] = smRender.AddRendVertex ( int(destPos.x+persp.x), int(destPos.y+persp.y), int(destPos.z), sColor );
		}
		else
		{
			v[0] = smRender.AddRendVertex ( OldVertex[0].x, OldVertex[0].y, OldVertex[0].z, sColor );
            v[1] = smRender.AddRendVertex ( OldVertex[1].x, OldVertex[1].y, OldVertex[1].z, sColor );
		}
        
		OldVertex[0].x = int(currentPos.x-persp.x);
        OldVertex[0].y = int(currentPos.y-persp.y);
        OldVertex[0].z = int(currentPos.z);

        OldVertex[1].x = int(currentPos.x+persp.x);
        OldVertex[1].y = int(currentPos.y+persp.y);
        OldVertex[1].z = int(currentPos.z);

        destPos.x = int(i->x);
        destPos.y = int(i->y);
        destPos.z = int(i->z);

        v[2] = smRender.AddRendVertex ( int(currentPos.x-persp.x), int(currentPos.y-persp.y), int(currentPos.z), sColor );
        v[3] = smRender.AddRendVertex ( int(currentPos.x+persp.x), int(currentPos.y+persp.y), int(currentPos.z), sColor );

        //첫번째 폴리곤 ( 1 - 2 - 3 );
        rf = &smRender.RendFace[ smRender.nRendFace++ ];
        rf->lpRendVertex[0] = v[0];
        rf->lpRendVertex[1] = v[1];
        rf->lpRendVertex[2] = v[2];
		rf->Matrial    = face->MatNum;		//메트리얼 복사
        rf->ClipStatus = v[0]->ClipStatus|v[1]->ClipStatus|v[2]->ClipStatus;
        rf->lpTexLink  = 0;

        //텍스쳐 연결
        smRender.AddRendTempTexLink( rf , 0,	texRect.left , texRect.top,
                            texRect.right , texRect.top,
                            texRect.left , texRect.bottom );


        //두번째 폴리곤 ( 2 - 4 - 3 );
        rf = &smRender.RendFace[ smRender.nRendFace++ ];
        rf->lpRendVertex[0] = v[1];
        rf->lpRendVertex[1] = v[3];
        rf->lpRendVertex[2] = v[2];
        rf->Matrial    = face->MatNum;		//메트리얼 복사
        rf->ClipStatus = v[1]->ClipStatus|v[2]->ClipStatus|v[3]->ClipStatus;
        rf->lpTexLink  = 0;

        //텍스쳐 연결
        smRender.AddRendTempTexLink( rf , 0, texRect.right , texRect.top,
                                                texRect.right , texRect.bottom,
                                                texRect.left  , texRect.bottom );

        texSizeCount++;
	}
	return true;
}


/*----------------------------------------------------------------------------*
* //캐릭터의 오브젝트의 위치를 찾는 함수..
* character: 얻고자 하는 캐릭터 포인터..
* obj: 얻고자 하는 오브젝트 포인트
* outObjPos: 오브젝트의 위치를 받을 POINT3D
*-----------------------------------------------------------------------------*/
bool AssaSearchObjPos(smCHAR *character, smOBJ3D *obj, POINT3D *outObjPos)
{
	if(character == NULL ||
		obj == NULL)
		return false;
	
	smMATRIX *mWorld;
	mWorld = &obj->mWorld;
	POINT3D angle;
	memcpy( &angle, &character->Angle , sizeof( POINT3D ) );
	angle.y = (-angle.y+ANGLE_180)&ANGCLIP;
	AnimObjectTree( obj, character->frame, angle.x, angle.y, angle.z);
	
	outObjPos->x = character->pX + mWorld->_41;
	outObjPos->z = character->pZ + mWorld->_42;
	outObjPos->y = character->pY + mWorld->_43;

	return true;	
}

/*----------------------------------------------------------------------------*
* //캐릭터의 오브젝트의 위치를 찾는 함수..
* character: 얻고자 하는 캐릭터 포인터..
* obj: 얻고자 하는 오브젝트 포인트
* outObjPos: 오브젝트의 위치를 받을 POINT3D
* length: 아래,위 길이 보정..
*-----------------------------------------------------------------------------*/
bool AssaSearchObjPos(smCHAR *character, smOBJ3D *obj, POINT3D *outObjPos, int length)
{
	if(character == NULL ||
		obj == NULL)
		return false;
	
	smMATRIX *mWorld;
	mWorld = &obj->mWorld;
	POINT3D angle;
	memcpy( &angle, &character->Angle , sizeof( POINT3D ) );
	angle.y = (-angle.y+ANGLE_180)&ANGCLIP;
	AnimObjectTree( obj, character->frame, angle.x, angle.y, angle.z);
	
	POINT3D pBot;
	pBot.x = 0;
	pBot.y = 0;
	pBot.z = length;

	int rx, ry, rz;
	rx =    pBot.x * mWorld->_11 +
		    pBot.y * mWorld->_21 +
		    pBot.z * mWorld->_31 ;

	ry = pBot.x * mWorld->_12 +
			pBot.y * mWorld->_22 +
			pBot.z * mWorld->_32 ;

	rz = pBot.x * mWorld->_13 +
			pBot.y * mWorld->_23 +
			pBot.z * mWorld->_33 ;

	outObjPos->x = character->pX +(rx>>FLOATNS) + mWorld->_41;
	outObjPos->z = character->pZ +(ry>>FLOATNS) + mWorld->_42;
	outObjPos->y = character->pY +(rz>>FLOATNS) + mWorld->_43;
	
	//outWeaponPos->x = character->pX + (rx>>FLOATNS) + mWorld->_41;
	//outWeaponPos->z = character->pZ + (ry>>FLOATNS) + mWorld->_42;
	//outWeaponPos->y = character->pY + (rz>>FLOATNS) + mWorld->_43;

	return true;	
}

/*----------------------------------------------------------------------------*
* //캐릭터의 무기의 위치를 찾는 함수
* character: 캐릭터 포인터..
* outWeaponPos: 무기의 위치를 받을 POINT3D
* u: 무기의 상대적 위치 (0-1 사이) 0.5이면 무기의 중앙 위치를 얻는다..
*-----------------------------------------------------------------------------*/
bool AssaSearchWeaponPos(smCHAR *character, POINT3D *outWeaponPos, float u)
{
	if(character == NULL)
		return false;

	smMATRIX *mWorld;
	mWorld = &character->HvRightHand.ObjBip->mWorld;
	
	POINT3D pBot;
	pBot.x = 0;
	pBot.y = 0;
	
	if(character->ShootingFlag)
	{
		outWeaponPos->x = character->ShootingPosi.x;
		outWeaponPos->y = character->ShootingPosi.y;
		outWeaponPos->z = character->ShootingPosi.z;
		return true;
	}

	if((character->HvRightHand.dwItemCode & sinITEM_MASK2) == sinWT1)
	{
		pBot.z = -character->HvRightHand.SizeMax+int((float)character->HvRightHand.SizeMax*u);
	}
	else
	{
		pBot.z = character->HvRightHand.SizeMax-int((float)character->HvRightHand.SizeMax*u);
	}
		
	POINT3D angle;
	memcpy( &angle, &character->Angle , sizeof( POINT3D ) );
	angle.y = (-angle.y+ANGLE_180)&ANGCLIP;
	
	AnimObjectTree( character->HvRightHand.ObjBip, character->frame, angle.x, angle.y, angle.z);
	int rx, ry, rz;
	rx = pBot.x * mWorld->_11 +
		    pBot.y * mWorld->_21 +
		    pBot.z * mWorld->_31 ;

	ry = pBot.x * mWorld->_12 +
			pBot.y * mWorld->_22 +
			pBot.z * mWorld->_32 ;

	rz = pBot.x * mWorld->_13 +
			pBot.y * mWorld->_23 +
			pBot.z * mWorld->_33 ;

	outWeaponPos->x = character->pX + (rx>>FLOATNS) + mWorld->_41;
	outWeaponPos->z = character->pZ + (ry>>FLOATNS) + mWorld->_42;
	outWeaponPos->y = character->pY + (rz>>FLOATNS) + mWorld->_43;

	return true;
}


/*----------------------------------------------------------------------------*
*					        이미지 테스트 
*-----------------------------------------------------------------------------*/	
static char sinDecode[512];
static char sinLine[512];
int AssaTestEffectImage()
{

	char *szFilePath = "image\\Sinimage\\AssaEffect\\AssaTest.INI"; 

	FILE	*fp;
	char strBuff[64];

	char *p;
	int Count = 0;
	int NextFlag = 0;
	int Index = 0;
	int sinMeshCount = 0 ; //Mesh의 갯수 

	fp = fopen( szFilePath , "rb" );
	if ( fp==NULL ) return FALSE;


	while( !feof( fp ) )					//  feof: file end까지 읽어라 
	{
		NextFlag = 0;
		if( fgets( sinLine, 255, fp ) == NULL)	
			break;

		p = GetWord( sinDecode , sinLine);
		
		//pb=p;p=GetWord(strBuff,p);if(strBuff[0]==34)p=GetString(strBuff,pb);	 " " 묶인 스트링을 같이 찾을때 쓰임

		if ( !NextFlag && lstrcmp( sinDecode , "*\306\304\300\317\300\314\270\247" )==0 ) {
			char boneName2[8];
			char MeshName[32];
			memset(boneName2, 0, sizeof(boneName2));
			memcpy(boneName2, strBuff, 2);
			p=GetWord(strBuff,p);
			if(stricmp(boneName2, "b_") == 0){ //Bone 인지를 가려낸다 
				cAssaEffect[Index]->Bone = 1;  //Bone 일경우 Bone을 먼저읽고 Mesh를 읽는다 
				Index = SetAssaEffect(0,strBuff,lpCurPlayer,0,0);
				lstrcpy(MeshName,&strBuff[2]);
				Index = SetAssaEffect(0,MeshName,lpCurPlayer,0,0);

			}
			else{
				Index = SetAssaEffect(0,strBuff,lpCurPlayer,0,0); //Mesh를 읽는다
			}

			/////////// Angle값을 보정해준다 
			cAssaEffect[Index]->Angle.y = (-(lpCurPlayer->Angle.y&ANGCLIP)+ANGLE_180)&ANGCLIP; //자신의 방향으로 오브젝트를찍는다 
			NextFlag = TRUE;

		}

		if ( !NextFlag && lstrcmp( sinDecode , "*\307\301\267\271\300\323" )==0 ) {
			p=GetWord(strBuff,p);
			cAssaEffect[Index]->AniMaxCount = atoi(strBuff);
			NextFlag = TRUE;

		}

		if ( !NextFlag && lstrcmp( sinDecode , "*\307\301\267\271\300\323\265\364\267\271\300\314" )==0 ) {
			p=GetWord(strBuff,p);
			cAssaEffect[Index]->AniDelayTime = atoi(strBuff);
			NextFlag = TRUE;

		}
		if ( !NextFlag && lstrcmp( sinDecode , "*\263\364\300\314" )==0 ) {
			p=GetWord(strBuff,p);
			cAssaEffect[Index]->Posi.y += atoi(strBuff);
			NextFlag = TRUE;

		}

	}
	if (fp) fclose(fp);
	return TRUE;

}
