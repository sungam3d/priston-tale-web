/*----------------------------------------------------------------------------*
*	파일명 :  HaQuestBoard.cpp	
*	하는일 :  신규 초보 퀘스트
*	작성일 :  최종업데이트 2005년 5월
*	적성자 :  하대용 
*-----------------------------------------------------------------------------*/	
//퀘스트상태 보드에 설정
int SetQuestBoard()
{

	char	szTemp[512];
	DWORD	dwQCode;
	int		cnt,mcnt;
	sQUEST_CHANGEJOB3 *lpQuest_ChangeJob3;
	char	*lpTitle , *lpMission;
	int		qCnt;


	InterfaceParty.quest_Progress_Clear();
	InterfaceParty.quest_End_Clear();

	dwQCode = sinSaveQuest(szTemp);	//퀘스트 데이타를 세이브한다 



	//
	//sQUEST_ELEMENTARY* lpQuest_Elementary;


	//진행중인 퀘스트
	switch( dwQCode ) {
	case SIN_QUEST_CODE_CHANGEJOB:
		InterfaceParty.quest_Progress_Set( mgQuestName[0][0], mgQuestName[0][2] );
		break;
	case SIN_QUEST_CODE_CHANGEJOB2_NPC_M:
		//벌꿀 퀘스트
		InterfaceParty.quest_Progress_Set( mgQuestName[1][0], mgQuestName[1][2] );
		break;
	case SIN_QUEST_CODE_CHANGEJOB2_NPC_D:
		//발모제 퀘스트
		InterfaceParty.quest_Progress_Set( mgQuestName[2][0], mgQuestName[2][2] );
		break;

	case SIN_QUEST_CODE_CHANGEJOB3:
		//3차스킬 퀘스트
		lpQuest_ChangeJob3 = (sQUEST_CHANGEJOB3 *)szTemp;
/*
		switch( lpQuest_ChangeJob3->State ) {
		case 1:		//뱀프를 가져오기
			InterfaceParty.quest_Progress_Set( mgQuestName[3][0], mgQuestName[3][2] );
			break;
		case 3:		//아이템을 숙련해와
			InterfaceParty.quest_Progress_Set( mgQuestName[3][1], mgQuestName[3][3] );
			break;
		case 4:		//거의 끝
			InterfaceParty.quest_Progress_Set( mgQuestName[4][0], mgQuestName[4][2] );
			break;
		}
*/
		lpTitle = "";
		lpMission = lpTitle;
		cnt = 0;

		switch( lpCurPlayer->smCharInfo.JOB_CODE ) {
			case JOBCODE_MECHANICIAN:
			case JOBCODE_FIGHTER:
			case JOBCODE_PIKEMAN:
			case JOBCODE_KNIGHT:
				cnt =5;
				break;
			case JOBCODE_ARCHER:
			case JOBCODE_ATALANTA:
				cnt =8;
				break;
			case JOBCODE_MAGICIAN:
			case JOBCODE_PRIESTESS:
				cnt =11;
				break;
		}

		if ( cnt ) {

			qCnt = 0;

			if ( lpQuest_ChangeJob3->State==3 ) {
				if ( lpQuest_ChangeJob3->Monster[0]<=1 ) {
					qCnt = 1;
				}
				if ( lpQuest_ChangeJob3->Monster[0]==2 ) {
					qCnt = 2;
				}
				if ( lpQuest_ChangeJob3->Monster[0]==3 ) {
					qCnt = 3;
				}
				if ( lpQuest_ChangeJob3->Monster[0]==4 ) {
					qCnt = 4;
				}
			}

			if ( lpQuest_ChangeJob3->State==4 )
				qCnt = 4;

			if ( lpQuest_ChangeJob3->State==5 )
				qCnt = 5;

			lpTitle = mgQuestName[cnt][0]; lpMission = mgQuestName[cnt][2];
			InterfaceParty.quest_Progress_Set( lpTitle, lpMission );

			if ( qCnt>=1 ) {
				lpTitle = mgQuestName[cnt][1]; lpMission = mgQuestName[cnt][3];
				InterfaceParty.quest_Progress_Set( lpTitle, lpMission );
			}
			if ( qCnt>=2 ) {
				lpTitle = mgQuestName[cnt+1][0]; lpMission = mgQuestName[cnt+1][2];
				InterfaceParty.quest_Progress_Set( lpTitle, lpMission );
			}
			if ( qCnt>=3 ) {
				lpTitle = mgQuestName[cnt+1][1]; lpMission = mgQuestName[cnt+1][3];
				InterfaceParty.quest_Progress_Set( lpTitle, lpMission );
			}
/*
			if ( qCnt>=4 ) {
				lpTitle = mgQuestName[cnt+2][0]; lpMission = mgQuestName[cnt+2][2];
				InterfaceParty.quest_Progress_Set( lpTitle, lpMission );
			}
*/
			if ( qCnt>=5 ) {
				lpTitle = mgQuestName[cnt+2][0]; lpMission = mgQuestName[cnt+2][2];
				InterfaceParty.quest_Progress_Set( lpTitle, lpMission );
			}


/*
			switch( lpQuest_ChangeJob3->State ) {
			case 1:		//뱀프를 가져오기
				lpTitle = mgQuestName[cnt][0]; lpMission = mgQuestName[cnt][2];
				break;
			case 3:		//아이템을 숙련해와
				if ( lpQuest_ChangeJob3->Monster[0]<=1 ) {
					lpTitle = mgQuestName[cnt][1]; lpMission = mgQuestName[cnt][3];
				}
				if ( lpQuest_ChangeJob3->Monster[0]==2 ) {
					lpTitle = mgQuestName[cnt+1][0]; lpMission = mgQuestName[cnt+1][2];
				}
				if ( lpQuest_ChangeJob3->Monster[0]==3 ) {
					lpTitle = mgQuestName[cnt+1][1]; lpMission = mgQuestName[cnt+1][3];
				}
				if ( lpQuest_ChangeJob3->Monster[0]==4 ) {
					lpTitle = mgQuestName[cnt+2][0]; lpMission = mgQuestName[cnt+2][2];
				}
				break;
			case 4:		//거의 끝
				lpTitle = mgQuestName[cnt+2][0]; lpMission = mgQuestName[cnt+2][2];
				break;
			}
			InterfaceParty.quest_Progress_Set( lpTitle, lpMission );
*/
		}
		break;

		//레벨별 퀘스트
	case SIN_QUEST_CODE_LEVEL30:
		InterfaceParty.quest_Progress_Set( mgQuestName[14][0], mgQuestName[14][2] );
		break;

	case SIN_QUEST_CODE_LEVEL55:
		InterfaceParty.quest_Progress_Set( mgQuestName[15][0], mgQuestName[15][2] );
		break;

	case SIN_QUEST_CODE_LEVEL55_2:
		InterfaceParty.quest_Progress_Set( mgQuestName[16][0], mgQuestName[16][2] );
		break;

	case SIN_QUEST_CODE_LEVEL70:
		InterfaceParty.quest_Progress_Set( mgQuestName[17][0], mgQuestName[17][2] );
		break;

	case SIN_QUEST_CODE_LEVEL80:
		InterfaceParty.quest_Progress_Set( mgQuestName[18][0], mgQuestName[18][2] );
		break;

	case SIN_QUEST_CODE_LEVEL85:
		InterfaceParty.quest_Progress_Set( mgQuestName[19][0], mgQuestName[19][2] );
		break;

	case SIN_QUEST_CODE_LEVEL90:
		InterfaceParty.quest_Progress_Set( mgQuestName[20][0], mgQuestName[20][2] );
		break;

	case SIN_QUEST_CODE_LEVEL80_2:		//스텟 7씩 받는 80퀘스트
		lpQuest_ChangeJob3 = (sQUEST_CHANGEJOB3 *)szTemp;
		switch( lpQuest_ChangeJob3->State ) {
			case 1:		//미뉴렌 만나라
				InterfaceParty.quest_Progress_Set( mgQuestName[21][0], mgQuestName[21][2] );
				break;
			case 2:		//뱀프 구해와라
				InterfaceParty.quest_Progress_Set( mgQuestName[22][0], mgQuestName[22][2] );
				break;
			case 3:		//데런한테 가라
			case 4:		//데런한테 가라
				InterfaceParty.quest_Progress_Set( mgQuestName[23][0], mgQuestName[23][2] );
				break;
		}
		break;
	case SIN_QUEST_CODE_LEVEL90_2:		//스텟 10씩 받는 90퀘스트
		lpQuest_ChangeJob3 = (sQUEST_CHANGEJOB3 *)szTemp;
		switch( lpQuest_ChangeJob3->State ) {
			case 1:		//로열메이지 한테가라
				InterfaceParty.quest_Progress_Set( mgQuestName[24][0], mgQuestName[24][2] );
				break;
			case 2:		//몬스터를 사냥해라
				switch( lpQuest_ChangeJob3->Kind ) {
					case 1:		//오메가 사냥
					InterfaceParty.quest_Progress_Set( mgQuestName[25][0], mgQuestName[25][2] );
					break;
					case 2:		//디머신 사냥
					InterfaceParty.quest_Progress_Set( mgQuestName[26][0], mgQuestName[26][2] );
					break;
					case 3:		//마운틴 사냥
					InterfaceParty.quest_Progress_Set( mgQuestName[27][0], mgQuestName[27][2] );
					break;
				}
				lpCurPlayer->PlayCurseQuest = TRUE;		//저주 모드
				break;
		}
		break;

	case SIN_QUEST_CODE_CHANGEJOB4:		//3차 전업 퀘스트
		lpQuest_ChangeJob3 = (sQUEST_CHANGEJOB3 *)szTemp;
		switch( lpQuest_ChangeJob3->State ) {
			case 1:		//몬스터를 잡아라 1
			case 2:		//몬스터를 잡아라 1
				InterfaceParty.quest_Progress_Set( mgQuestName[28][0], mgQuestName[28][2] );
				break;
			case 3:		//몬스터를 잡아라 2
			case 4:		//몬스터를 잡아라 2
				InterfaceParty.quest_Progress_Set( mgQuestName[29][0], mgQuestName[29][2] );
				break;

			case 5:		//몬스터를 잡아라 3
				switch( lpQuest_ChangeJob3->Kind ) {
					case MONSTER_SOLID_SNAIL:
						InterfaceParty.quest_Progress_Set( mgQuestName[30][0], mgQuestName[30][2] );
						break;
					case MONSTER_ILLUSION_KNIGHT:
						InterfaceParty.quest_Progress_Set( mgQuestName[30][0], mgQuestName[30][3] );
						break;
					case MONSTER_AVELRISK_LOAD:
						InterfaceParty.quest_Progress_Set( mgQuestName[30][0], mgQuestName[31][2] );
						break;
					case MONSTER_DUSK:
						InterfaceParty.quest_Progress_Set( mgQuestName[30][0], mgQuestName[31][3] );
						break;
					case MONSTER_CHAIN_GOLEM:
						InterfaceParty.quest_Progress_Set( mgQuestName[30][0], mgQuestName[32][2] );
						break;
					case MONSTER_SADNESS:
						InterfaceParty.quest_Progress_Set( mgQuestName[30][0], mgQuestName[32][3] );
						break;
				}
				break;

			case 6:		//스킬마스터를 이겨라
			case 7:		//스킬마스터를 이겨라
			case 8:		//스킬마스터를 이겨라
				InterfaceParty.quest_Progress_Set( mgQuestName[33][0], mgQuestName[33][2] );
				break;
		}
	break;
	//신규 튜토리얼 퀘스트
	//DWORD	dwHaQCode;
	//dwHaQCode = 
	case HAQUEST_CODE_ELEMENTARY_A:
		if(JOB_IS_TEMPSKRON(sinChar->JOB_CODE))
			InterfaceParty.quest_Progress_Set(mgHaQuestName[0][0],mgHaQuestName[0][2]);
		else
			InterfaceParty.quest_Progress_Set(mgHaQuestName[1][0],mgHaQuestName[1][2]);			
	break;
	case HAQUEST_CODE_ELEMENTARY_B:
		if(JOB_IS_TEMPSKRON(sinChar->JOB_CODE))
			InterfaceParty.quest_Progress_Set(mgHaQuestName[2][0],mgHaQuestName[2][2]);
		else
			InterfaceParty.quest_Progress_Set(mgHaQuestName[3][0],mgHaQuestName[3][2]);			
	break;
	case HAQUEST_CODE_ELEMENTARY_C:
		if(JOB_IS_TEMPSKRON(sinChar->JOB_CODE))
			InterfaceParty.quest_Progress_Set(mgHaQuestName[4][0],mgHaQuestName[4][2]);
		else
			InterfaceParty.quest_Progress_Set(mgHaQuestName[5][0],mgHaQuestName[5][2]);			
	break;
	case HAQUEST_CODE_ELEMENTARY_D:
		if(JOB_IS_TEMPSKRON(sinChar->JOB_CODE))
			InterfaceParty.quest_Progress_Set(mgHaQuestName[6][0],mgHaQuestName[6][2]);
		else
			InterfaceParty.quest_Progress_Set(mgHaQuestName[7][0],mgHaQuestName[7][2]);			
	break;
	case HAQUEST_CODE_ELEMENTARY_E:
		if(JOB_IS_TEMPSKRON(sinChar->JOB_CODE))
			InterfaceParty.quest_Progress_Set(mgHaQuestName[8][0],mgHaQuestName[8][2]);
		else
			InterfaceParty.quest_Progress_Set(mgHaQuestName[9][0],mgHaQuestName[9][2]);			
	break;
	case HAQUEST_CODE_ELEMENTARY_F:
		InterfaceParty.quest_Progress_Set(mgHaQuestName[10][0],mgHaQuestName[10][2]);		
	break;
	case HAQUEST_CODE_ELEMENTARY_G:
		InterfaceParty.quest_Progress_Set(mgHaQuestName[11][0],mgHaQuestName[11][2]);				
	break;
	case HAQUEST_CODE_FURYOFPHANTOM:
		switch(chaQuest.sHaQuest100LV.State){
			case 1:case 2:case 3:
				InterfaceParty.quest_Progress_Set(mgHaQuestName[12][0],mgHaQuestName[12][2]);				
			break;
			case 4:
				InterfaceParty.quest_Progress_Set(mgHaQuestName[12][1],mgHaQuestName[12][3]);				
			break;
			case 7:case 5:case 6:case 8:
				InterfaceParty.quest_Progress_Set(mgHaQuestName[13][0],mgHaQuestName[13][2]);				
			break;
			case 9:case 10:
				InterfaceParty.quest_Progress_Set(mgHaQuestName[13][1],mgHaQuestName[13][3]);				
			break;
			case 11:case 12:
				InterfaceParty.quest_Progress_Set(mgHaQuestName[14][0],mgHaQuestName[14][2]);				
			break;
		}
	break;

	}

	
	//완료된 퀘스트
	for( cnt=RecordLastQuestInfo.LastQuestCount-1;cnt>=RecordLastQuestInfo.LastQuestCount-LAST_QUEST_MAX;cnt-- ) {

		if ( cnt<0 ) break;

		mcnt = cnt & LAST_QUEST_MASK;
		switch( RecordLastQuestInfo.wLastQuest[mcnt] ) {
		case SIN_QUEST_CODE_CHANGEJOB:
		case SIN_QUEST_CODE_CHANGEJOB2_NPC_M:
		case SIN_QUEST_CODE_CHANGEJOB2_NPC_D:
			InterfaceParty.quest_End_Set( mgQuestName[0][1], mgQuestName[0][3] );
			break;

		case SIN_QUEST_CODE_CHANGEJOB3:
			InterfaceParty.quest_End_Set( mgQuestName[7][1], mgQuestName[7][3] );
			break;

		case SIN_QUEST_CODE_LEVEL30:
			InterfaceParty.quest_End_Set( mgQuestName[14][1], mgQuestName[14][3] );
			break;

		case SIN_QUEST_CODE_LEVEL55:
			InterfaceParty.quest_End_Set( mgQuestName[15][1], mgQuestName[15][3] );
			break;

		case SIN_QUEST_CODE_LEVEL55_2:
			InterfaceParty.quest_End_Set( mgQuestName[16][1], mgQuestName[16][3] );
			break;

		case SIN_QUEST_CODE_LEVEL70:
			InterfaceParty.quest_End_Set( mgQuestName[17][1], mgQuestName[17][3] );
			break;

		case SIN_QUEST_CODE_LEVEL80:
			InterfaceParty.quest_End_Set( mgQuestName[18][1], mgQuestName[18][3] );
			break;

		case SIN_QUEST_CODE_LEVEL85:
			InterfaceParty.quest_End_Set( mgQuestName[19][1], mgQuestName[19][3] );
			break;

		case SIN_QUEST_CODE_LEVEL90:
			InterfaceParty.quest_End_Set( mgQuestName[20][1], mgQuestName[20][3] );
			break;

		case SIN_QUEST_CODE_LEVEL80_2:		//스텟 7씩 받는 80퀘스트
			InterfaceParty.quest_End_Set( mgQuestName[21][1], mgQuestName[21][3] );
			break;

		case SIN_QUEST_CODE_LEVEL90_2:		//스텟 10씩 받는 90퀘스트
			InterfaceParty.quest_End_Set( mgQuestName[24][1], mgQuestName[24][3] );
			lpCurPlayer->PlayCurseQuest = 0;
			break;

		case SIN_QUEST_CODE_CHANGEJOB4:		//3차 전업 퀘스트
			InterfaceParty.quest_End_Set( mgQuestName[33][1], mgQuestName[33][3] );
			break;
		//초보 퀘스트
		case HAQUEST_CODE_ELEMENTARY_A:
			InterfaceParty.quest_End_Set(mgHaQuestName[0][1],mgHaQuestName[0][3]);
		break;
		case HAQUEST_CODE_ELEMENTARY_B:
			InterfaceParty.quest_End_Set(mgHaQuestName[2][1],mgHaQuestName[2][3]);
		break;
		case HAQUEST_CODE_ELEMENTARY_C:
			InterfaceParty.quest_End_Set(mgHaQuestName[4][1],mgHaQuestName[4][3]);
		break;
		case HAQUEST_CODE_ELEMENTARY_D:
			InterfaceParty.quest_End_Set(mgHaQuestName[6][1],mgHaQuestName[6][3]);
		break;
		case HAQUEST_CODE_ELEMENTARY_E:
			InterfaceParty.quest_End_Set(mgHaQuestName[8][1],mgHaQuestName[8][3]);
		break;
		case HAQUEST_CODE_ELEMENTARY_F:
			InterfaceParty.quest_End_Set(mgHaQuestName[10][1],mgHaQuestName[10][3]);
		break;
		case HAQUEST_CODE_ELEMENTARY_G:
			InterfaceParty.quest_End_Set(mgHaQuestName[11][1],mgHaQuestName[11][3]);
		break;
		case HAQUEST_CODE_FURYOFPHANTOM:
			InterfaceParty.quest_End_Set(mgHaQuestName[14][1],mgHaQuestName[14][3]);				
		break;
		}
	}
	return TRUE;
}


