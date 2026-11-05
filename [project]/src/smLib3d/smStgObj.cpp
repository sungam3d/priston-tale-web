
#include "smd3d.h"


smSTAGE_OBJECT::smSTAGE_OBJECT()
{
	int x,y;
	nObj = 0;

	for(y=0;y<MAP_SIZE;y++) {
		for(x=0;x<MAP_SIZE;x++) {
			ObjectMap[x][y] = 0;
		}
	}

	SumCnt = 0;
	Offset.x = Offset.y = Offset.z = 0;
}

smSTAGE_OBJECT::~smSTAGE_OBJECT()
{
	int cnt;

	for( cnt=nObj-1;cnt>=0;cnt--) {
		if ( mObj[cnt].Pattern ) delete mObj[cnt].Pattern;
	}

}

int smSTAGE_OBJECT::AddObject( smPAT3D *Pat, int x, int y, int z , int angX, int angY, int angZ )
{
	int w,h;
	smSTAGE_OBJ3D *obj;

	if ( nObj>=MAX_STAGEOBJ ) return FALSE;

	mObj[ nObj ].Pattern = Pat;

	mObj[ nObj ].Posi.x = x;
	mObj[ nObj ].Posi.y = y;
	mObj[ nObj ].Posi.z = z;

	mObj[ nObj ].Angle.x = angX;
	mObj[ nObj ].Angle.y = angY;
	mObj[ nObj ].Angle.z = angZ;

	mObj[ nObj ].sum = 0;
	mObj[ nObj ].LinkObj = 0;
	mObj[ nObj ].LastDrawTime = GetCurrentTime();

	w = (x>>(6+FLOATNS))&0xFF;
	h = (z>>(6+FLOATNS))&0xFF;

	if ( ObjectMap[w][h]==0 ) {
		ObjectMap[w][h] = &mObj[nObj];
	}
	else {
		obj = ObjectMap[w][h];
		while( obj!=0 ) {
			if ( obj->LinkObj==0 ) break;
			obj = obj->LinkObj;
		}
		obj->LinkObj = &mObj[nObj];
	}

	nObj++;

	return TRUE;
}

//오브젝트 삽입 자동좌표 ( 맵에 삽입된 오브젝트 )
int smSTAGE_OBJECT::AddObject( smPAT3D *Pat )
{
	int w,h;
	smSTAGE_OBJ3D *obj;
	int x,y,z;

	if ( nObj>=MAX_STAGEOBJ ) return FALSE;

	mObj[ nObj ].Pattern = Pat;

	x = Pat->obj3d[0]->Tm._41;
	y = Pat->obj3d[0]->Tm._43;
	z = Pat->obj3d[0]->Tm._42;

	mObj[ nObj ].Posi.x = x;
	mObj[ nObj ].Posi.y = y;
	mObj[ nObj ].Posi.z = z;

	mObj[ nObj ].Angle.x = 0;
	mObj[ nObj ].Angle.y = 0;
	mObj[ nObj ].Angle.z = 0;

	mObj[ nObj ].sum = 0;
	mObj[ nObj ].LinkObj = 0;
	mObj[ nObj ].LastDrawTime = GetCurrentTime();

	w = (x>>(6+FLOATNS))&0xFF;
	h = (z>>(6+FLOATNS))&0xFF;

	if ( ObjectMap[w][h]==0 ) {
		ObjectMap[w][h] = &mObj[nObj];
	}
	else {
		obj = ObjectMap[w][h];
		while( obj!=0 ) {
			if ( obj->LinkObj==0 ) break;
			obj = obj->LinkObj;
		}
		obj->LinkObj = &mObj[nObj];
	}

	nObj++;

	return TRUE;
}


//오브젝트 삽입 자동좌표 ( 맵에 삽입된 오브젝트 )
int smSTAGE_OBJECT::AddObjectFile( char *szFile , char *szBipFile )
{
	int w,h;
	smSTAGE_OBJ3D *obj;
	smPAT3D *Pat;
	smPAT3D *BipPat;
	int x,y,z;

	if ( nObj>=MAX_STAGEOBJ ) return FALSE;

	if ( szBipFile ) {
		//######################################################################################
		//작 성 자 : 오 영 석
		smASE_SetPhysique( 0 );
		//######################################################################################

		BipPat = smASE_ReadBone( szBipFile );
		smASE_SetPhysique( BipPat );
		if ( !BipPat ) return FALSE;
	}
	else {
		smASE_SetPhysique( 0 );
		BipPat = 0;
	}

	Pat = smASE_Read( szFile );
	if ( !Pat ) {
		if ( BipPat ) {
			delete BipPat;
			BipPat = 0;
		}
		return FALSE;
	}

	mObj[ nObj ].Pattern = Pat;
	mObj[ nObj ].BipPattern = BipPat;

	x = Pat->obj3d[0]->Tm._41;
	y = Pat->obj3d[0]->Tm._43;
	z = Pat->obj3d[0]->Tm._42;

	mObj[ nObj ].Posi.x = x;
	mObj[ nObj ].Posi.y = y;
	mObj[ nObj ].Posi.z = z;

	mObj[ nObj ].Angle.x = 0;
	mObj[ nObj ].Angle.y = 0;
	mObj[ nObj ].Angle.z = 0;

	mObj[ nObj ].sum = 0;
	mObj[ nObj ].LinkObj = 0;
	mObj[ nObj ].LastDrawTime = GetCurrentTime();

	w = (x>>(6+FLOATNS))&0xFF;
	h = (z>>(6+FLOATNS))&0xFF;

	if ( ObjectMap[w][h]==0 ) {
		ObjectMap[w][h] = &mObj[nObj];
	}
	else {
		obj = ObjectMap[w][h];
		while( obj!=0 ) {
			if ( obj->LinkObj==0 ) break;
			obj = obj->LinkObj;
		}
		obj->LinkObj = &mObj[nObj];
	}

	nObj++;

	return TRUE;
}





int smSTAGE_OBJECT::Draw( smSTAGE_OBJ3D *mobj )
{
	int rcnt;
	POINT3D aposi;
	int Frame;
	int MaxFrame;

	aposi.x = mobj->Posi.x;
	aposi.y = mobj->Posi.y;
	aposi.z = mobj->Posi.z;

	int tm2 , tm,overframe;

	tm= GetCurrentTime();
	tm2 = tm;
	tm -= mobj->LastDrawTime;

	Frame = mobj->Pattern->Frame;
	Frame += tm<<1;

	if ( mobj->Pattern->TmParent ) 
		MaxFrame = mobj->Pattern->TmParent->MaxFrame;
	else
		MaxFrame = mobj->Pattern->MaxFrame;

	overframe = Frame - MaxFrame;
	if ( overframe>0 ) {
		Frame -= MaxFrame;
		if ( Frame>=MaxFrame )
			Frame = 160;
		if ( Frame<160 ) Frame+=160;
	}

//	if ( aposi.y!=261072 )
//		return FALSE;

	smRender.SetClipStates( SMCLIP_LEFT|SMCLIP_RIGHT|SMCLIP_TOP|SMCLIP_BOTTOM );

	mobj->Pattern->Frame = Frame;
	mobj->Pattern->SetPosi( &aposi ,&mobj->Angle);
	// 패턴의 좌표를 원좌표로 고정
	mobj->Pattern->SetFixPosi();
	smObjFixPos = TRUE;
	// a moved map's prop is drawn at its own position: move the camera the other way
	POINT3D cam;
	cam.x = Camera.x-Offset.x;
	cam.y = Camera.y-Offset.y;
	cam.z = Camera.z-Offset.z;
	rcnt = mobj->Pattern->RenderD3D( &cam, &CameraAngle );
	smObjFixPos = FALSE;

	mobj->sum = SumCnt;
	mobj->LastDrawTime = tm2;

	return rcnt;

}


int smSTAGE_OBJECT::Draw( int x, int y, int z, int ax, int ay, int az )
{
	int h,w;
	int x1,x2,t;
	smSTAGE_OBJ3D *mobj;

	Camera.x = x;
	Camera.y = y;
	Camera.z = z;
	CameraAngle.x = ax;
	CameraAngle.y = ay;
	CameraAngle.z = az;

	for( h=MapPosiTop; h<MapPosiBot ; h++ ) {
			x1 = MapPosiLeft[ h&0xFF ];
			x2 = MapPosiRight[ h&0xFF ];
			if ( x1>x2 ) { 	t=x1; x1=x2 ; x2=t; }  // x1<->x2 바꿈

			for( w=x1; w<x2 ; w++ ) {
				mobj = ObjectMap[w&0xFF][h&0xFF];
				if ( mobj ) {
					while( mobj!=0 ) {
						if ( mobj->sum!=SumCnt ) Draw( mobj );
						mobj = mobj->LinkObj;
					}
				}
			}
	}

	SumCnt++;

	return TRUE;
}


int smSTAGE_OBJECT::Draw2( int x, int y, int z, int ax, int ay, int az )
{
	int h,w;
	int x1,x2,t;
	smSTAGE_OBJ3D *mobj;

	Camera.x = x;
	Camera.y = y;
	Camera.z = z;
	CameraAngle.x = ax;
	CameraAngle.y = ay;
	CameraAngle.z = az;

	CameraX = x>>FLOATNS;
	CameraY = y>>FLOATNS;
	CameraZ = z>>FLOATNS;

	CameraAngX = GetRadian( ax );
	CameraAngY = GetRadian( ay );
	CameraAngZ = GetRadian( az );

	SetViewLength();

	SetViewRadian();

	if ( MakeMapTable() ) {
		for( h=MapPosiTop; h<MapPosiBot ; h++ ) {
			x1 = MapPosiLeft[ h&0xFF ];
			x2 = MapPosiRight[ h&0xFF ];
			if ( x1>x2 ) { 	t=x1; x1=x2 ; x2=t; }  // x1<->x2 바꿈

			for( w=x1; w<x2 ; w++ ) {
				mobj = ObjectMap[w&0xFF][h&0xFF];
				if ( mobj ) {
					while( mobj!=0 ) {
						if ( mobj->sum!=SumCnt ) Draw( mobj );
						mobj = mobj->LinkObj;
					}
				}
	
			}
	
		}
	}
	SumCnt++;

	return TRUE;
}


//######################################################################################
//작 성 자 : 오 영 석
int smSTAGE_OBJECT::DrawOpening( smSTAGE_OBJ3D *mobj, int FrameStep )
{
	int rcnt;
	POINT3D aposi;
	int Frame;
	int MaxFrame;

	aposi.x = mobj->Posi.x;
	aposi.y = mobj->Posi.y;
	aposi.z = mobj->Posi.z;

	if ( mobj->Pattern->TmParent ) 
		MaxFrame = mobj->Pattern->TmParent->MaxFrame;
	else
		MaxFrame = mobj->Pattern->MaxFrame;

	if( FrameStep >= MaxFrame )
		Frame = MaxFrame - 1;

	smRender.SetClipStates( SMCLIP_LEFT|SMCLIP_RIGHT|SMCLIP_TOP|SMCLIP_BOTTOM );

	mobj->Pattern->Frame = FrameStep;
	mobj->Pattern->SetPosi( &aposi ,&mobj->Angle);
	// 패턴의 좌표를 원좌표로 고정
	mobj->Pattern->SetFixPosi();
	smObjFixPos = TRUE;
	
	rcnt = mobj->Pattern->RenderD3dOpening( &Camera, &CameraAngle );
	smObjFixPos = FALSE;

	mobj->sum = SumCnt;
	return rcnt;
}


int smSTAGE_OBJECT::DrawOpening( int x, int y, int z, int ax, int ay, int az, int FrameStep )
{
	Camera.x = x;
	Camera.y = y;
	Camera.z = z;
	CameraAngle.x = ax;
	CameraAngle.y = ay;
	CameraAngle.z = az;

	smSTAGE_OBJ3D *lpMapObj;
	for( int i=0; i < nObj; i++ )
	{
		lpMapObj = &mObj[ i ];
		while( lpMapObj != 0 )
		{
			if( lpMapObj->sum != SumCnt ) DrawOpening( lpMapObj, FrameStep );
			lpMapObj = lpMapObj->LinkObj;
		}
	}

	SumCnt++;

	return TRUE;
}
//######################################################################################