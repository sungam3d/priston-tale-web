/*----------------------------------------------------------------------------*
*	파일명 :  HaQuest.cpp	
*	하는일 :  신규 초보 퀘스트
*	작성일 :  최종업데이트 2005년 5월
*	적성자 :  하대용 
*-----------------------------------------------------------------------------*/	

#include "sinLinkHeader.h"
#include "..\\field.h"
/*----------------------------------------------------------------------------*
*						 문서 파일 						
*-----------------------------------------------------------------------------*/	
char *haQuestFilePathA[] = {
	"image\\Sinimage\\Quest\\haQuesttired_Astart.sin",
	"image\\Sinimage\\Quest\\haQuesttired_Apass1.sin",
	"image\\Sinimage\\Quest\\haQuesttired_Apass.sin",
	"image\\Sinimage\\Quest\\haQuesttired_Aend.sin",
};
char *haQuestFilePathB[] = {
	"image\\Sinimage\\Quest\\haQuestterrified_Bstart_M.sin",
	"image\\Sinimage\\Quest\\haQuestterrified_Bstart_T.sin",
	"image\\Sinimage\\Quest\\haQuestterrified_Bpass_M.sin",
	"image\\Sinimage\\Quest\\haQuestterrified_Bpass_T.sin",
	"image\\Sinimage\\Quest\\haQuestterrified_Bend.sin",
};
char *haQuestFilePathC[] = {
	"image\\Sinimage\\Quest\\haQuestcandy_Cstart_M.sin",
	"image\\Sinimage\\Quest\\haQuestcandy_Cstart_T.sin",
	"image\\Sinimage\\Quest\\haQuestcandy_Cpass.sin",
	"image\\Sinimage\\Quest\\haQuestcandy_Cend.sin",
};
char *haQuestFilePathD[] = {
	"image\\Sinimage\\Quest\\haQuestGoblin_Dstart_M.sin",
	"image\\Sinimage\\Quest\\haQuestGoblin_Dstart_T.sin",
	"image\\Sinimage\\Quest\\haQuestGoblin_Dpass_M.sin",
	"image\\Sinimage\\Quest\\haQuestGoblin_Dpass_T.sin",
	"image\\Sinimage\\Quest\\haQuestGoblin_Dend.sin",
};
char *haQuestFilePathE[] = {
	"image\\Sinimage\\Quest\\haQuestundead_Estart_M.sin",
	"image\\Sinimage\\Quest\\haQuestundead_Estart_T.sin",
	"image\\Sinimage\\Quest\\haQuestundead_Epass_M.sin",
	"image\\Sinimage\\Quest\\haQuestundead_Epass_T.sin",
	"image\\Sinimage\\Quest\\haQuestundead_Eend.sin",
};
char *haQuestFilePathF[] = {
	"image\\Sinimage\\Quest\\haQuestcyclops_Fstart.sin",
	"image\\Sinimage\\Quest\\haQuestcyclops_Fpass.sin",
	"image\\Sinimage\\Quest\\haQuestcyclops_Fend.sin",
};
char *haQuestFilePathG[] = {
	"image\\Sinimage\\Quest\\haQuesthammer_Gstart.sin",
	"image\\Sinimage\\Quest\\haQuesthammer_Gpass.sin",
	"image\\Sinimage\\Quest\\haQuesthammer_Gend.sin",
};
char *haQuestLevelOver = {
	"image\\Sinimage\\Quest\\haQuestLevel_Over.sin",	
};

/*----------------------------------------------------------------------------*
*							전역 변수 						
*-----------------------------------------------------------------------------*/	
DWORD haElementaryQuestLog =0; //초보 튜토리얼 퀘스트 수행여부

cHaQuest chaQuest;

cHaQuest::cHaQuest()
{
	//
	iJob_Index = 0;
	
	//
	bFuryQuestFlag   = false;
	//iFuryQuestIndex[2]=0; // 누가 이따구로 만든거야~!!!!(증거자료:해외)
	iFuryQuestIndex[0] = 0;
	iFuryQuestIndex[1] = 0;

}

cHaQuest::~cHaQuest()
{

}
/*----------------------------------------------------------------------------*
*							haCheckQuest
*-----------------------------------------------------------------------------*/
int cHaQuest::haQuestCheck()
{
	//
	for(int cnt=0;cnt<8;cnt++){
		if(sHaQuestElementary[cnt].CODE){
			cMessageBox.ShowMessage(MESSAGE_QUEST_NOW_EXIT);
			return FALSE;
		}
	}
	//
	if(sHaQuest100LV.CODE){
			cMessageBox.ShowMessage(MESSAGE_QUEST_NOW_EXIT);
			return FALSE;
	}

	return TRUE;
}

/*----------------------------------------------------------------------------*
*							HaCheckQuestElementary						
*-----------------------------------------------------------------------------*/
int cHaQuest::HaCheckQuestElementary(int npcNum,int QuestKey,int KindClan)
{
	// pluto
	if( cSinHelp.sinGetHelpShowFlag() ) return FALSE;	// 무한 텔레포트 코어 때문에 넣었음
	if(!CheckNowQuestState(0))return FALSE;  //현재진행중 퀘스트 체크
	if(sinChar->Level > 39){
		if(npcNum==0){
			for(int i=0;i<8;i++){   //현재 진행중인 퀘스트만 초기화
				if(chaQuest.sHaQuestElementary[i].CODE){
					memset(&chaQuest.sHaQuestElementary[i],0,sizeof(sQUEST_ELEMENTARY));
					cSkill.CancelContinueSkill(CHANGE_ELEMENTARY_QUEST);
					DeleteQuestItem(sinQT1|sin14);
					DeleteQuestItem(sinQT1|sin15);
					DeleteQuestItem(sinQT1|sin16);
				}
			}
			cSinHelp.sinShowHelp(HAQUEST_CODE_ELEMENTARY_X,QuestMessageBoxPosi2.x,QuestMessageBoxPosi2.y,QuestMessageBoxSize2.x,QuestMessageBoxSize2.y,
				RGBA_MAKE(0,15,128,125),haQuestLevelOver);  											
		}
		return FALSE;    	
	}
	int cnt=0;	

	//0 모라이온 1 템스크론
	if(JOB_IS_TEMPSKRON(sinChar->JOB_CODE))
		iJob_Index = 1; //템스크론
	else
		iJob_Index = 2; //모라이온

	if((QUESTBIT_ELEMENTARY_A & haElementaryQuestLog)==QUESTBIT_ELEMENTARY_A){
			cnt++;
	}
	if((QUESTBIT_ELEMENTARY_B & haElementaryQuestLog)==QUESTBIT_ELEMENTARY_B){
			cnt++;
	}
	if((QUESTBIT_ELEMENTARY_C & haElementaryQuestLog)==QUESTBIT_ELEMENTARY_C){
			cnt++;
	}
	if((QUESTBIT_ELEMENTARY_D & haElementaryQuestLog)==QUESTBIT_ELEMENTARY_D){
			cnt++;
	}
	if((QUESTBIT_ELEMENTARY_E & haElementaryQuestLog)==QUESTBIT_ELEMENTARY_E){
			cnt++;
	}
	if((QUESTBIT_ELEMENTARY_F & haElementaryQuestLog)==QUESTBIT_ELEMENTARY_F){
			cnt++;
	}
	if((QUESTBIT_ELEMENTARY_G & haElementaryQuestLog)==QUESTBIT_ELEMENTARY_G){
			cnt++;
	}


	//현재 진행중인 퀘스트 Key값
	QuestKeyCode = cnt;

	KindClanIndex=0;
	if(KindClan)KindClanIndex = KindClan;  //종족 구분을 할때 세팅
	if(QuestKey)
		if(QuestKey != cnt+1)return FALSE; //퀘스트 번호가 맞지 않는 npc는 리턴시킨다.
	
	//순차적 진행
	switch(cnt){
		case 0:
			haElementaryQuest_A(npcNum);    //피곤한 창고 지기
		break;
		case 1:
			if(npcNum==0)
				haElementaryQuest_B();      //두려운 기운
		break;
		case 2:
			haElementaryQuest_C(npcNum);    //사탕 배달
		break;
		case 3:
			haElementaryQuest_D(npcNum);    //고블린 동산
		break;
		case 4:
			haElementaryQuest_E(npcNum);    //언데드의 위협
		break;
		case 5:
			if(npcNum==0)
				haElementaryQuest_F();      //외눈박이 괴물	
		break;
		case 6:
			haElementaryQuest_G(npcNum);    //잃어버린 망치
		break;		
	}
	return FALSE;
}

/*----------------------------------------------------------------------------*
*					피곤한 창고 지기					
*-----------------------------------------------------------------------------*/	
int cHaQuest::haElementaryQuest_A(int npcNum)
{
	switch(npcNum){
		case 0:   //렌에게 말을걸다.
			switch(sHaQuestElementary[Quest_A].State){
				case 0: //퀘스트 메세지를 
					cSinHelp.sinShowHelp(HAQUEST_CODE_ELEMENTARY_A,QuestMessageBoxPosi2.x,QuestMessageBoxPosi2.y,QuestMessageBoxSize2.x,QuestMessageBoxSize2.y, 
						RGBA_MAKE(0,15,128,125),haQuestFilePathA[0]);  					
				break;
				case 1: //퀘스트 진행중 렌에게 말을 걸을때
					cSinHelp.sinShowHelp(HAQUEST_CODE_ELEMENTARY_A,QuestMessageBoxPosi2.x,QuestMessageBoxPosi2.y,QuestMessageBoxSize2.x,QuestMessageBoxSize2.y, 
						RGBA_MAKE(0,15,128,125),haQuestFilePathA[1]);  									
				break;
				case 2: //완료 메세지
					//아이템을 주고
					SetQuestBoard();
					chaQuest.sHaQuestElementary[Quest_A].State = 100;
					cSinHelp.sinShowHelp(HAQUEST_CODE_ELEMENTARY_A,QuestMessageBoxPosi2.x,QuestMessageBoxPosi2.y,QuestMessageBoxSize2.x,QuestMessageBoxSize2.y,
						RGBA_MAKE(0,15,128,125),haQuestFilePathA[3]);  									
				break;
			}		
		break; 
		case 1:  //아렌트 & 라벤다 를 찾아 갓을때
			if(KindClanIndex != iJob_Index) return FALSE;
			if(sHaQuestElementary[Quest_A].State < 1) return FALSE;
			if(cInvenTory.SearchItemCode(sinQT1|sin15)){
				DeleteQuestItem(sinQT1|sin15);
				sHaQuestElementary[Quest_A].State = 2;
				cSinHelp.sinShowHelp(HAQUEST_CODE_ELEMENTARY_A,QuestMessageBoxPosi2.x,QuestMessageBoxPosi2.y,QuestMessageBoxSize2.x,QuestMessageBoxSize2.y, 
					RGBA_MAKE(0,15,128,125),haQuestFilePathA[2]);  									
			}
		break;
	}
	return TRUE;
}

/*----------------------------------------------------------------------------*
*					  두려운 기운				
*-----------------------------------------------------------------------------*/	
int cHaQuest::haElementaryQuest_B()
{
	switch(sHaQuestElementary[Quest_B].State){
		case 0: //렌에게 말을걸다.
			if(iJob_Index==1)
				cSinHelp.sinShowHelp(HAQUEST_CODE_ELEMENTARY_B,QuestMessageBoxPosi2.x,QuestMessageBoxPosi2.y,QuestMessageBoxSize2.x,QuestMessageBoxSize2.y,
				RGBA_MAKE(0,15,128,125),haQuestFilePathB[1]); 
			if(iJob_Index==2)
				cSinHelp.sinShowHelp(HAQUEST_CODE_ELEMENTARY_B,QuestMessageBoxPosi2.x,QuestMessageBoxPosi2.y,QuestMessageBoxSize2.x,QuestMessageBoxSize2.y,
				RGBA_MAKE(0,15,128,125),haQuestFilePathB[0]); 
		break;
		case 1: //사냥시작
			memset(&szBuff,0,sizeof(szBuff));
			memset(&szQuestMonsterName,0,sizeof(szQuestMonsterName));	
			haSetQuestTimer(&sHaQuestElementary[Quest_B]);
			if(iJob_Index==1){ //템스크론 
				chaQuest.sHaQuestElementary[Quest_B].Kind = MONSTER_HOPY;
				lstrcpy(szQuestMonsterName ,haQuestMonsterName[0]);
			}
			if(iJob_Index==2){ //모라이온
				chaQuest.sHaQuestElementary[Quest_B].Kind = MONSTER_RABIE;	
				lstrcpy(szQuestMonsterName ,haQuestMonsterName[1]);
			}
			chaQuest.sHaQuestElementary[Quest_B].Monster[1] = 5;
#ifdef	_LANGUAGE_ENGLISH
			wsprintf(szBuff,sinLevelQusetDoc2, chaQuest.sHaQuestElementary[Quest_B].Monster[1], szQuestMonsterName );
#else
#ifdef	_LANGUAGE_BRAZIL
			wsprintf(szBuff,sinLevelQusetDoc2, chaQuest.sHaQuestElementary[Quest_B].Monster[1], szQuestMonsterName );
#else
#ifdef	_LANGUAGE_ARGENTINA
			wsprintf(szBuff,sinLevelQusetDoc2, chaQuest.sHaQuestElementary[Quest_B].Monster[1], szQuestMonsterName );
#else
#ifdef	_LANGUAGE_CHINESE
			wsprintf(szBuff,sinLevelQusetDoc2, chaQuest.sHaQuestElementary[Quest_B].Monster[1], szQuestMonsterName );
#else
			wsprintf(szBuff,sinLevelQusetDoc2,szQuestMonsterName , chaQuest.sHaQuestElementary[Quest_B].Monster[1]);
#endif
#endif
#endif
#endif
			cMessageBox.ShowMessageEvent(szBuff);
			sHaQuestElementary[Quest_B].State = 2;
		break;
		case 2://사냥중
				if(iJob_Index==1)
					cSinHelp.sinShowHelp(HAQUEST_CODE_ELEMENTARY_B,QuestMessageBoxPosi2.x,QuestMessageBoxPosi2.y,QuestMessageBoxSize2.x,QuestMessageBoxSize2.y,
					RGBA_MAKE(0,15,128,125),haQuestFilePathB[3]); 
				if(iJob_Index==2)
					cSinHelp.sinShowHelp(HAQUEST_CODE_ELEMENTARY_B,QuestMessageBoxPosi2.x,QuestMessageBoxPosi2.y,QuestMessageBoxSize2.x,QuestMessageBoxSize2.y,
					RGBA_MAKE(0,15,128,125),haQuestFilePathB[2]); 
		break;
		case 3: //사냥 완료 종료 메세지
			//아이템을 주고
			sHaQuestElementary[Quest_B].State = 100;
			cSinHelp.sinShowHelp(HAQUEST_CODE_ELEMENTARY_B,QuestMessageBoxPosi2.x,QuestMessageBoxPosi2.y,QuestMessageBoxSize2.x,QuestMessageBoxSize2.y,
				RGBA_MAKE(0,15,128,125),haQuestFilePathB[4]);  									
		break;
	}
	
	return TRUE;
}

/*----------------------------------------------------------------------------*
*					 사탕 배달				
*-----------------------------------------------------------------------------*/	
int cHaQuest::haElementaryQuest_C(int npcNum)
{
	switch(npcNum){
		case 0:  //렌에게 말을 걸다.
			switch(sHaQuestElementary[Quest_C].State){
				case 0:  //퀘스트 메세지
					if(iJob_Index == 1)//템스크론
						cSinHelp.sinShowHelp(HAQUEST_CODE_ELEMENTARY_C,QuestMessageBoxPosi2.x,QuestMessageBoxPosi2.y,QuestMessageBoxSize2.x,QuestMessageBoxSize2.y, 
						RGBA_MAKE(0,15,128,125),haQuestFilePathC[1]);  											
					if(iJob_Index == 2)//모라이온
						cSinHelp.sinShowHelp(HAQUEST_CODE_ELEMENTARY_C,QuestMessageBoxPosi2.x,QuestMessageBoxPosi2.y,QuestMessageBoxSize2.x,QuestMessageBoxSize2.y, 
						RGBA_MAKE(0,15,128,125),haQuestFilePathC[0]);  											
				break;
				case 1:
					cSinHelp.sinShowHelp(HAQUEST_CODE_ELEMENTARY_C,QuestMessageBoxPosi2.x,QuestMessageBoxPosi2.y,QuestMessageBoxSize2.x,QuestMessageBoxSize2.y,
						RGBA_MAKE(0,15,128,125),haQuestFilePathC[2]);  																
				break;
			}
		break;
		case 1: //필리아& 알리아
			if(KindClanIndex != iJob_Index) return FALSE;
			if(sHaQuestElementary[Quest_C].State < 1) return FALSE;
			if(cInvenTory.SearchItemCode(sinQT1|sin14)){
				SetQuestBoard();
				//아이템을 주고
				sHaQuestElementary[Quest_C].State = 100;
				cSinHelp.sinShowHelp(HAQUEST_CODE_ELEMENTARY_C,QuestMessageBoxPosi2.x,QuestMessageBoxPosi2.y,QuestMessageBoxSize2.x,QuestMessageBoxSize2.y, 
					RGBA_MAKE(0,15,128,125),haQuestFilePathC[3]);  																
			
			}
		break;
	}
	
	return TRUE;
}
/*----------------------------------------------------------------------------*
*					 고블린 동산				
*-----------------------------------------------------------------------------*/
int cHaQuest::haElementaryQuest_D(int npcNum)
{
	switch(npcNum){
		case 0: //렌
			switch(sHaQuestElementary[Quest_D].State){
				case 0: //렌에게 말을걸다.
					if(iJob_Index==1) //템스크론 
						cSinHelp.sinShowHelp(HAQUEST_CODE_ELEMENTARY_D,QuestMessageBoxPosi2.x,QuestMessageBoxPosi2.y,QuestMessageBoxSize2.x,QuestMessageBoxSize2.y, 
						RGBA_MAKE(0,15,128,125),haQuestFilePathD[1]);  									
					if(iJob_Index==2) //모라이온 
						cSinHelp.sinShowHelp(HAQUEST_CODE_ELEMENTARY_D,QuestMessageBoxPosi2.x,QuestMessageBoxPosi2.y,QuestMessageBoxSize2.x,QuestMessageBoxSize2.y, 
						RGBA_MAKE(0,15,128,125),haQuestFilePathD[0]);  															
				break;
				case 1: //사냥시작
					memset(&szBuff,0,sizeof(szBuff));
					memset(&szQuestMonsterName,0,sizeof(szQuestMonsterName));	
					haSetQuestTimer(&sHaQuestElementary[Quest_D]);
					if(iJob_Index==1){ //템스크론 
						chaQuest.sHaQuestElementary[Quest_D].Kind = MONSTER_HOBGOBLIN;
						lstrcpy(szQuestMonsterName ,haQuestMonsterName[2]);
					}
					if(iJob_Index==2){ //모라이온
						chaQuest.sHaQuestElementary[Quest_D].Kind = MONSTER_NORTHGOBLIN;	
						lstrcpy(szQuestMonsterName ,haQuestMonsterName[3]);
					}
					chaQuest.sHaQuestElementary[Quest_D].Monster[1] = 5;
#ifdef	_LANGUAGE_ENGLISH
					wsprintf(szBuff,sinLevelQusetDoc2, chaQuest.sHaQuestElementary[Quest_D].Monster[1], szQuestMonsterName);
#else
#ifdef	_LANGUAGE_BRAZIL
					wsprintf(szBuff,sinLevelQusetDoc2, chaQuest.sHaQuestElementary[Quest_D].Monster[1], szQuestMonsterName );
#else
#ifdef	_LANGUAGE_ARGENTINA
					wsprintf(szBuff,sinLevelQusetDoc2, chaQuest.sHaQuestElementary[Quest_D].Monster[1], szQuestMonsterName );
#else
#ifdef	_LANGUAGE_CHINESE
					wsprintf(szBuff,sinLevelQusetDoc2, chaQuest.sHaQuestElementary[Quest_D].Monster[1], szQuestMonsterName);
#else
					wsprintf(szBuff,sinLevelQusetDoc2, szQuestMonsterName ,chaQuest.sHaQuestElementary[Quest_D].Monster[1]);
#endif
#endif
#endif
#endif
					cMessageBox.ShowMessageEvent(szBuff);
					sHaQuestElementary[Quest_D].State = 2;
				break;
				case 2://사냥중
					if(iJob_Index==1) //템스크론 
						cSinHelp.sinShowHelp(HAQUEST_CODE_ELEMENTARY_D,QuestMessageBoxPosi2.x,QuestMessageBoxPosi2.y,QuestMessageBoxSize2.x,QuestMessageBoxSize2.y, 
						RGBA_MAKE(0,15,128,125),haQuestFilePathD[3]);  									
					if(iJob_Index==2) //모라이온 
						cSinHelp.sinShowHelp(HAQUEST_CODE_ELEMENTARY_D,QuestMessageBoxPosi2.x,QuestMessageBoxPosi2.y,QuestMessageBoxSize2.x,QuestMessageBoxSize2.y,
						RGBA_MAKE(0,15,128,125),haQuestFilePathD[2]);  															
				break;
			}
		break;
		case 1: //각마을 경비병
 			if(KindClanIndex != iJob_Index) return FALSE;
			if(sHaQuestElementary[Quest_D].State == 3){
				//아이템을 주고
				sHaQuestElementary[Quest_D].State = 100;
				cSinHelp.sinShowHelp(HAQUEST_CODE_ELEMENTARY_D,QuestMessageBoxPosi2.x,QuestMessageBoxPosi2.y,QuestMessageBoxSize2.x,QuestMessageBoxSize2.y,
					RGBA_MAKE(0,15,128,125),haQuestFilePathD[4]);  												
			}
		break;
	}
	return TRUE;
}
/*----------------------------------------------------------------------------*
*					 언데드의 위협				
*-----------------------------------------------------------------------------*/
int cHaQuest::haElementaryQuest_E(int npcNum)
{
	switch(npcNum){
		case 0: //렌
			switch(sHaQuestElementary[Quest_E].State){
				case 0: //렌에게 말을걸다.
					if(iJob_Index==1) //템스크론 
						cSinHelp.sinShowHelp(HAQUEST_CODE_ELEMENTARY_E,QuestMessageBoxPosi2.x,QuestMessageBoxPosi2.y,QuestMessageBoxSize2.x,QuestMessageBoxSize2.y, 
						RGBA_MAKE(0,15,128,125),haQuestFilePathE[1]);  									
					if(iJob_Index==2) //모라이온
						cSinHelp.sinShowHelp(HAQUEST_CODE_ELEMENTARY_E,QuestMessageBoxPosi2.x,QuestMessageBoxPosi2.y,QuestMessageBoxSize2.x,QuestMessageBoxSize2.y, 
						RGBA_MAKE(0,15,128,125),haQuestFilePathE[0]);  															
				break;
				case 1: //사냥시작
					memset(&szBuff,0,sizeof(szBuff));
					memset(&szQuestMonsterName,0,sizeof(szQuestMonsterName));
					haSetQuestTimer(&sHaQuestElementary[Quest_E]);
					if(iJob_Index==1){ //템스크론 
						chaQuest.sHaQuestElementary[Quest_E].Kind = MONSTER_SKELETON;
						lstrcpy(szQuestMonsterName ,haQuestMonsterName[4]);
					}
					if(iJob_Index==2){ //모라이온
						chaQuest.sHaQuestElementary[Quest_E].Kind = MONSTER_CORRUPT;	
						lstrcpy(szQuestMonsterName ,haQuestMonsterName[5]);
					}
					chaQuest.sHaQuestElementary[Quest_E].Monster[1] = 10;
#ifdef	_LANGUAGE_ENGLISH
					wsprintf(szBuff,sinLevelQusetDoc2, chaQuest.sHaQuestElementary[Quest_E].Monster[1], szQuestMonsterName);
#else
#ifdef	_LANGUAGE_BRAZIL
					wsprintf(szBuff,sinLevelQusetDoc2, chaQuest.sHaQuestElementary[Quest_E].Monster[1], szQuestMonsterName );
#else
#ifdef	_LANGUAGE_ARGENTINA
					wsprintf(szBuff,sinLevelQusetDoc2, chaQuest.sHaQuestElementary[Quest_E].Monster[1], szQuestMonsterName );
#else
#ifdef	_LANGUAGE_CHINESE
					wsprintf(szBuff,sinLevelQusetDoc2, chaQuest.sHaQuestElementary[Quest_E].Monster[1], szQuestMonsterName);
#else
					wsprintf(szBuff,sinLevelQusetDoc2, szQuestMonsterName ,chaQuest.sHaQuestElementary[Quest_E].Monster[1]);
#endif
#endif
#endif
#endif
					cMessageBox.ShowMessageEvent(szBuff);
					sHaQuestElementary[Quest_E].State = 2;
				break;
				case 2://사냥중
					if(iJob_Index==1) //템스크론 
						cSinHelp.sinShowHelp(HAQUEST_CODE_ELEMENTARY_E,QuestMessageBoxPosi2.x,QuestMessageBoxPosi2.y,QuestMessageBoxSize2.x,QuestMessageBoxSize2.y, 
						RGBA_MAKE(0,15,128,125),haQuestFilePathE[3]);  									
					if(iJob_Index==2) //모라이온
						cSinHelp.sinShowHelp(HAQUEST_CODE_ELEMENTARY_E,QuestMessageBoxPosi2.x,QuestMessageBoxPosi2.y,QuestMessageBoxSize2.x,QuestMessageBoxSize2.y, 
						RGBA_MAKE(0,15,128,125),haQuestFilePathE[2]);  															
				break;		  
			}
		break;
		case 1: //골덴
			if(sHaQuestElementary[Quest_E].State == 3){
				//아이템을 주고			
				sHaQuestElementary[Quest_E].State = 100;
				cSinHelp.sinShowHelp(HAQUEST_CODE_ELEMENTARY_E,QuestMessageBoxPosi2.x,QuestMessageBoxPosi2.y,QuestMessageBoxSize2.x,QuestMessageBoxSize2.y, 
					RGBA_MAKE(0,15,128,125),haQuestFilePathE[4]);  									
			}
		break;
	}
	return TRUE;
}
/*----------------------------------------------------------------------------*
*					 외눈박이 괴물			
*-----------------------------------------------------------------------------*/
int cHaQuest::haElementaryQuest_F()
{
	switch(sHaQuestElementary[Quest_F].State){
		case 0: //렌에게 말을걸다.
			cSinHelp.sinShowHelp(HAQUEST_CODE_ELEMENTARY_F,QuestMessageBoxPosi2.x,QuestMessageBoxPosi2.y,QuestMessageBoxSize2.x,QuestMessageBoxSize2.y, RGBA_MAKE(0,15,128,125),haQuestFilePathF[0]);  									
		break;
		case 1: //사냥시작
			memset(&szBuff,0,sizeof(szBuff));
			memset(&szQuestMonsterName,0,sizeof(szQuestMonsterName));	
			haSetQuestTimer(&sHaQuestElementary[Quest_F]);
			chaQuest.sHaQuestElementary[Quest_F].Kind = MONSTER_CYCLOPS;
			lstrcpy(szQuestMonsterName ,haQuestMonsterName[6]);
			chaQuest.sHaQuestElementary[Quest_F].Monster[1] = 10; //Monster Max
#ifdef	_LANGUAGE_ENGLISH
			wsprintf(szBuff,sinLevelQusetDoc2, chaQuest.sHaQuestElementary[Quest_F].Monster[1], szQuestMonsterName);
#else
#ifdef	_LANGUAGE_BRAZIL
			wsprintf(szBuff,sinLevelQusetDoc2, chaQuest.sHaQuestElementary[Quest_F].Monster[1], szQuestMonsterName );
#else
#ifdef	_LANGUAGE_ARGENTINA
			wsprintf(szBuff,sinLevelQusetDoc2, chaQuest.sHaQuestElementary[Quest_F].Monster[1], szQuestMonsterName );
#else
#ifdef	_LANGUAGE_CHINESE
			wsprintf(szBuff,sinLevelQusetDoc2, chaQuest.sHaQuestElementary[Quest_F].Monster[1], szQuestMonsterName);
#else
			wsprintf(szBuff,sinLevelQusetDoc2, szQuestMonsterName, chaQuest.sHaQuestElementary[Quest_F].Monster[1]);
#endif
#endif
#endif
#endif
			cMessageBox.ShowMessageEvent(szBuff);
			sHaQuestElementary[Quest_F].State = 2;
		break;
		case 2: //사냥중
			cSinHelp.sinShowHelp(HAQUEST_CODE_ELEMENTARY_F,QuestMessageBoxPosi2.x,QuestMessageBoxPosi2.y,QuestMessageBoxSize2.x,QuestMessageBoxSize2.y, RGBA_MAKE(0,15,128,125),haQuestFilePathF[1]);  									
		break;
		case 3: //사냥 완료 종료 메세지
			//아이템을 주고
			sHaQuestElementary[Quest_F].State = 100;
			cSinHelp.sinShowHelp(HAQUEST_CODE_ELEMENTARY_F,QuestMessageBoxPosi2.x,QuestMessageBoxPosi2.y,QuestMessageBoxSize2.x,QuestMessageBoxSize2.y, RGBA_MAKE(0,15,128,125),haQuestFilePathF[2]);  									
		break;
	}
	return TRUE;
}
/*----------------------------------------------------------------------------*
*					 잃어버린 망치			
*-----------------------------------------------------------------------------*/
int cHaQuest::haElementaryQuest_G(int npcNum)
{
	switch(npcNum){
		case 0: //렌
			switch(sHaQuestElementary[Quest_G].State){
				case 0:  //렌에게 말을 걸다
					cSinHelp.sinShowHelp(HAQUEST_CODE_ELEMENTARY_G,QuestMessageBoxPosi2.x,QuestMessageBoxPosi2.y,QuestMessageBoxSize2.x,QuestMessageBoxSize2.y, RGBA_MAKE(0,15,128,125),haQuestFilePathG[0]);  													
				break;
				case 1:  //망치 획득퀘스트 중 렌에게 말을 걸엇을때
					cSinHelp.sinShowHelp(HAQUEST_CODE_ELEMENTARY_G,QuestMessageBoxPosi2.x,QuestMessageBoxPosi2.y,QuestMessageBoxSize2.x,QuestMessageBoxSize2.y, RGBA_MAKE(0,15,128,125),haQuestFilePathG[1]);  													
				break;
			}
		break;
		case 1: //엄프
			if(sHaQuestElementary[Quest_G].State<1)return FALSE;
			if(cInvenTory.SearchItemCode(sinQT1|sin16))
			{
				sHaQuestElementary[Quest_G].State = 2;
				SetQuestBoard();				
			}
			if(sHaQuestElementary[Quest_G].State == 2)
			{
				//아이템을주고
				DeleteQuestItem(sinQT1|sin16);
				sHaQuestElementary[Quest_G].State = 100;
				cSinHelp.sinShowHelp(HAQUEST_CODE_ELEMENTARY_G,QuestMessageBoxPosi2.x,QuestMessageBoxPosi2.y,QuestMessageBoxSize2.x,QuestMessageBoxSize2.y, RGBA_MAKE(0,15,128,125),haQuestFilePathG[2]);  																
			}
		break;
	}
	return TRUE;
}
/*----------------------------------------------------------------------------*
*					  퀘스트 타이머			
*-----------------------------------------------------------------------------*/
int cHaQuest::haSetQuestTimer(sQUEST_ELEMENTARY *pQuest)
{
	sSKILL TempSkill;
	memset(&TempSkill,0,sizeof(sSKILL)); 
	//텍스쳐 로딩
	char *Path = "image\\Sinimage\\skill\\keep\\QuestTimer.tga";      
	if(!MatQuestTimer){
		MatQuestTimer = CreateTextureMaterial(Path, 0, 0, 0,0, SMMAT_BLEND_ALPHA );
		ReadTextures();	//텍스쳐 로딩
	}

	TempSkill.CODE = CHANGE_ELEMENTARY_QUEST; 
	TempSkill.UseTime = 70*70;
	TempSkill.Point = 1;
	TempSkill.MatIcon = MatQuestTimer;
	sinContinueSkillSet(&TempSkill);
	sinPlaySound(SIN_SOUND_EAT_POTION2);//대박 사운드
	return TRUE;
}
/*----------------------------------------------------------------------------*
*					  Npc 처리 메세지(PlayMain.cpp)		
*-----------------------------------------------------------------------------*/
void HaCheckQuestNpc(int NpcCount)
{
	switch(NpcCount){
		case 1014:  //렌
			chaQuest.HaCheckQuestElementary(0);
		break;
		case 1015:  //아렌트
			chaQuest.HaCheckQuestElementary(1,1,1);
		break;
		case 1016:  //라벤다
			chaQuest.HaCheckQuestElementary(1,1,2);
		break;
		case 1017:  //바츠
			chaQuest.HaCheckQuestElementary(1,3,2);
		break;
		case 1018:  //고덴
			chaQuest.HaCheckQuestElementary(1,3,1);
		break;
		case 1019:  //템스크론 경비병
			chaQuest.HaCheckQuestElementary(1,4,1);
		break;
		case 1020:  //모라이온 경비병
			chaQuest.HaCheckQuestElementary(1,4,2);
		break;
		case 1021:  //루이넨 빌리지 골덴
			chaQuest.HaCheckQuestElementary(1,5);
		break;
		case 1022:  //엄프
			chaQuest.HaCheckQuestElementary(1,7);
		break;
	}
}
/*----------------------------------------------------------------------------*
*					  Main		
*-----------------------------------------------------------------------------*/
void cHaQuest::init()
{
	lpQuestItemImage[0] = LoadDibSurfaceOffscreen( "image\\Sinimage\\Items\\Weapon\\itwa120.bmp" );
	lpQuestItemImage[1] = LoadDibSurfaceOffscreen( "image\\Sinimage\\Items\\Weapon\\itwc120.bmp" );
	lpQuestItemImage[2] = LoadDibSurfaceOffscreen( "image\\Sinimage\\Items\\Weapon\\itwh121.bmp" );
	lpQuestItemImage[3] = LoadDibSurfaceOffscreen( "image\\Sinimage\\Items\\Weapon\\itwp121.bmp" );
	lpQuestItemImage[4] = LoadDibSurfaceOffscreen( "image\\Sinimage\\Items\\Weapon\\itws122.bmp" );
	lpQuestItemImage[5] = LoadDibSurfaceOffscreen( "image\\Sinimage\\Items\\Weapon\\itws223.bmp" );
	lpQuestItemImage[6] = LoadDibSurfaceOffscreen( "image\\Sinimage\\Items\\Weapon\\itwt121.bmp" );
	lpQuestItemImage[7] = LoadDibSurfaceOffscreen( "image\\Sinimage\\Items\\Weapon\\itwm121.bmp" );

	lpQuestButtonImage[0] = LoadDibSurfaceOffscreen( "image\\Sinimage\\shopall\\LeftArrow.bmp" );
	lpQuestButtonImage[1] = LoadDibSurfaceOffscreen( "image\\Sinimage\\shopall\\RightArrow.bmp" );
	lpQuestButtonImage[2] = LoadDibSurfaceOffscreen( "image\\Sinimage\\Inven\\LeftArrow.bmp" );
	lpQuestButtonImage[3] = LoadDibSurfaceOffscreen( "image\\Sinimage\\Inven\\RightArrow.bmp" );


	//초기화
	memset(&cItemName,0,sizeof(cItemName));
	sHaQuest100LV.State=0;
	FuryBattleFlag = false;

}
int ItemButtonFlag[2]={0};
/*----------------------------------------------------------------------------*
*					  Main		
*-----------------------------------------------------------------------------*/
void cHaQuest::Main()
{

	if(chaQuest.sHaQuest100LV.State == 13){
		if(pCursorPos.x >100 && pCursorPos.x < 100+20 && 
			pCursorPos.y >365 && pCursorPos.y <365+20)
				ItemButtonFlag[0] = 1;
		else if(pCursorPos.x >330 && pCursorPos.x < 330+20 &&
			pCursorPos.y >365 && pCursorPos.y <365+20)
				ItemButtonFlag[1] = 1;
		else{
				ItemButtonFlag[0] = 0;
				ItemButtonFlag[1] = 0;
		}
	}

}
/*----------------------------------------------------------------------------*
*					  Draw		
*-----------------------------------------------------------------------------*/
void cHaQuest::Draw()
{
	//
	if(bFuryQuestFlag == true){
		switch(iFuryQuestIndex[1]){
		  case 1:case 3:case 4:case 5:case 6:case 8:
			DrawSprite(100,130,lpQuestItemImage[iFuryQuestIndex[1]-1],0,0,44,88); 
		  break;
		  case 2:
			DrawSprite(100,130,lpQuestItemImage[iFuryQuestIndex[1]-1],0,0,44,66); 
		  break;
		  case 7:
			DrawSprite(100,130,lpQuestItemImage[iFuryQuestIndex[1]-1],0,0,22,88); 
		  break;
		}
		if(chaQuest.sHaQuest100LV.State == 13){
			if(ItemButtonFlag[0])
				DrawSprite(100,365,lpQuestButtonImage[2],0,0,20,20);
			else	
				DrawSprite(100,365,lpQuestButtonImage[0],0,0,20,20);
			if(ItemButtonFlag[1])
				DrawSprite(330,365,lpQuestButtonImage[3],0,0,20,20); 
			else
				DrawSprite(330,365,lpQuestButtonImage[1],0,0,20,20); 
		}	
	}
}
/*----------------------------------------------------------------------------*
*					  KeyDown		
*-----------------------------------------------------------------------------*/
void cHaQuest::KeyDown()
{

}
/*----------------------------------------------------------------------------*
*					  LButtonDown		
*-----------------------------------------------------------------------------*/
void cHaQuest::LButtonDown(int x , int y)
{
	if(chaQuest.sHaQuest100LV.State == 13){
		if(ItemButtonFlag[0]){
			if(chaQuest.iFuryQuestIndex[1] > 1)
				chaQuest.iFuryQuestIndex[1]--;
		}
		if(ItemButtonFlag[1]){
			if(chaQuest.iFuryQuestIndex[1] < 8)
			chaQuest.iFuryQuestIndex[1]++;
		}
		if(bFuryQuestFlag == true){
			showFuryQuestItem();
		}
	}
}
/*----------------------------------------------------------------------------*
*					  LButtonUp		
*-----------------------------------------------------------------------------*/
void cHaQuest::LButtonUp(int x , int y)
{

}
//100LV 퀘스트 목록
char* haQuestFuryOfPhantom[12]   ={
	"image\\Sinimage\\Quest\\100LVQuest_1Start.sin",
	"image\\Sinimage\\Quest\\100LVQuest_2Start.sin",
	"image\\Sinimage\\Quest\\100LVQuest_2pass.sin",
	"image\\Sinimage\\Quest\\100LVQuest_3Start.sin",
	"image\\Sinimage\\Quest\\100LVQuest_3Pass.sin",
	"image\\Sinimage\\Quest\\100LVQuest_4Start.sin",
	"image\\Sinimage\\Quest\\100LVQuest_4Pass.sin",
	"image\\Sinimage\\Quest\\100LVQuest_5Start.sin",
	"image\\Sinimage\\Quest\\100LVQuest_5Pass.sin",
	"image\\Sinimage\\Quest\\100LVQuest_6Start.sin",
	"image\\Sinimage\\Quest\\100LVQuest_6Pass.sin",
	"image\\Sinimage\\Quest\\100LVQuest_7End.sin",
};
char* haQuestFuryOfPhantomItem[8] = {
	"image\\Sinimage\\Quest\\haQuestFuryItem1.sin",
	"image\\Sinimage\\Quest\\haQuestFuryItem2.sin",
	"image\\Sinimage\\Quest\\haQuestFuryItem3.sin",
	"image\\Sinimage\\Quest\\haQuestFuryItem4.sin",
	"image\\Sinimage\\Quest\\haQuestFuryItem5.sin",
	"image\\Sinimage\\Quest\\haQuestFuryItem6.sin",
	"image\\Sinimage\\Quest\\haQuestFuryItem7.sin",
	"image\\Sinimage\\Quest\\haQuestFuryItem8.sin",
};
char* haQuestFuryOfPhantomVictory = "image\\Sinimage\\Quest\\100LVQuest_Victory.sin";
/*----------------------------------------------------------------------------*
*					  showFuryQuest	
*-----------------------------------------------------------------------------*/
int  cHaQuest::showFuryQuest()
{
	//
	if((QUESTBIT_FURYOFPHANTOM & haElementaryQuestLog)==QUESTBIT_FURYOFPHANTOM)return true;
	if(sinChar->Level<100)return false;
	if(!CheckNowQuestState(0))return FALSE;  //현재진행중 퀘스트 체크
	//
	bFuryQuestFlag = true;	
	switch(sHaQuest100LV.State)
	{
		case 0:
			cSinHelp.sinShowHelp(HAQUEST_CODE_FURYOFPHANTOM,QuestMessageBoxPosi2.x,QuestMessageBoxPosi2.y,QuestMessageBoxSize2.x,QuestMessageBoxSize2.y, RGBA_MAKE(0,15,128,125),haQuestFuryOfPhantom[0]);  																			
		break;
		case 1:
			memset(&szBuff,0,sizeof(szBuff));
			memset(&szQuestMonsterName,0,sizeof(szQuestMonsterName));
			haSetQuestTimer(&chaQuest.sHaQuest100LV);
			chaQuest.sHaQuest100LV.Kind = MONSTER_FERRUMHEART;
			lstrcpy(szQuestMonsterName ,ha100LVQuestMonterName[0]);
			chaQuest.sHaQuest100LV.Monster[1] = 50;
#ifdef	_LANGUAGE_ENGLISH
			wsprintf(szBuff,sinLevelQusetDoc2, chaQuest.sHaQuest100LV.Monster[1], szQuestMonsterName);
#else
#ifdef	_LANGUAGE_BRAZIL
			wsprintf(szBuff,sinLevelQusetDoc2, chaQuest.sHaQuest100LV.Monster[1], szQuestMonsterName);
#else
#ifdef	_LANGUAGE_ARGENTINA
			wsprintf(szBuff,sinLevelQusetDoc2, chaQuest.sHaQuest100LV.Monster[1], szQuestMonsterName);
#else
#ifdef	_LANGUAGE_CHINESE
			wsprintf(szBuff,sinLevelQusetDoc2, chaQuest.sHaQuest100LV.Monster[1], szQuestMonsterName);
#else
			wsprintf(szBuff,sinLevelQusetDoc2, szQuestMonsterName , chaQuest.sHaQuest100LV.Monster[1]);
#endif
#endif
#endif
#endif
			cMessageBox.ShowMessageEvent(szBuff);
			cSinHelp.sinShowHelp(HAQUEST_CODE_FURYOFPHANTOM,QuestMessageBoxPosi2.x,QuestMessageBoxPosi2.y,QuestMessageBoxSize2.x,QuestMessageBoxSize2.y, RGBA_MAKE(0,15,128,125),haQuestFuryOfPhantom[1]);  																			
			SaveGameData();		
		break;
		case 2:
			cSinHelp.sinShowHelp(HAQUEST_CODE_FURYOFPHANTOM,QuestMessageBoxPosi2.x,QuestMessageBoxPosi2.y,QuestMessageBoxSize2.x,QuestMessageBoxSize2.y, RGBA_MAKE(0,15,128,125),haQuestFuryOfPhantom[2]);  																			
		break;
		case 3:
			cSkill.CancelContinueSkill(CHANGE_ELEMENTARY_QUEST);
			cSinHelp.sinShowHelp(HAQUEST_CODE_FURYOFPHANTOM,QuestMessageBoxPosi2.x,QuestMessageBoxPosi2.y,QuestMessageBoxSize2.x,QuestMessageBoxSize2.y, RGBA_MAKE(0,15,128,125),haQuestFuryOfPhantom[3]);  																			
			SaveGameData();
		break;
		case 4:
			cSinHelp.sinShowHelp(HAQUEST_CODE_FURYOFPHANTOM,QuestMessageBoxPosi2.x,QuestMessageBoxPosi2.y,QuestMessageBoxSize2.x,QuestMessageBoxSize2.y, RGBA_MAKE(0,15,128,125),haQuestFuryOfPhantom[4]);  																			
		break;
		case 5:
			cSinHelp.sinShowHelp(HAQUEST_CODE_FURYOFPHANTOM,QuestMessageBoxPosi2.x,QuestMessageBoxPosi2.y,QuestMessageBoxSize2.x,QuestMessageBoxSize2.y, RGBA_MAKE(0,15,128,125),haQuestFuryOfPhantom[5]);  																			
		break;
		case 6:
			memset(&szBuff,0,sizeof(szBuff));
			memset(&szQuestMonsterName,0,sizeof(szQuestMonsterName));
			haSetQuestTimer(&chaQuest.sHaQuest100LV);
			chaQuest.sHaQuest100LV.Kind = MONSTER_ICE;
			lstrcpy(szQuestMonsterName ,ha100LVQuestMonterName[1]);
			chaQuest.sHaQuest100LV.Monster[0] = 0;
			chaQuest.sHaQuest100LV.Monster[1] = 100;
#ifdef _LANGUAGE_ENGLISH
			wsprintf(szBuff, sinLevelQusetDoc2, chaQuest.sHaQuest100LV.Monster[1], szQuestMonsterName);
#else
#ifdef _LANGUAGE_BRAZIL
			wsprintf(szBuff, sinLevelQusetDoc2, chaQuest.sHaQuest100LV.Monster[1], szQuestMonsterName);
#else
#ifdef _LANGUAGE_ARGENTINA
			wsprintf(szBuff, sinLevelQusetDoc2, chaQuest.sHaQuest100LV.Monster[1], szQuestMonsterName);
#else
#ifdef _LANGUAGE_CHINESE
			wsprintf(szBuff, sinLevelQusetDoc2, chaQuest.sHaQuest100LV.Monster[1], szQuestMonsterName);
#else
			wsprintf(szBuff, sinLevelQusetDoc2, szQuestMonsterName , chaQuest.sHaQuest100LV.Monster[1]);
#endif
#endif
#endif
#endif
			cMessageBox.ShowMessageEvent(szBuff);
			chaQuest.sHaQuest100LV.State = 7;
			SetQuestBoard();
			SaveGameData();
		break;
		case 7:
			cSinHelp.sinShowHelp(HAQUEST_CODE_FURYOFPHANTOM,QuestMessageBoxPosi2.x,QuestMessageBoxPosi2.y,QuestMessageBoxSize2.x,QuestMessageBoxSize2.y, RGBA_MAKE(0,15,128,125),haQuestFuryOfPhantom[6]);  																						
		break;
		case 8:
			cSkill.CancelContinueSkill(CHANGE_ELEMENTARY_QUEST);
			cSinHelp.sinShowHelp(HAQUEST_CODE_FURYOFPHANTOM,QuestMessageBoxPosi2.x,QuestMessageBoxPosi2.y,QuestMessageBoxSize2.x,QuestMessageBoxSize2.y, RGBA_MAKE(0,15,128,125),haQuestFuryOfPhantom[7]);  																						
			SaveGameData();			
		break;
		case 9:
			cSinHelp.sinShowHelp(HAQUEST_CODE_FURYOFPHANTOM,QuestMessageBoxPosi2.x,QuestMessageBoxPosi2.y,QuestMessageBoxSize2.x,QuestMessageBoxSize2.y, RGBA_MAKE(0,15,128,125),haQuestFuryOfPhantom[8]);  																						
		break;
		case 10:
			cSinHelp.sinShowHelp(HAQUEST_CODE_FURYOFPHANTOM,QuestMessageBoxPosi2.x,QuestMessageBoxPosi2.y,QuestMessageBoxSize2.x,QuestMessageBoxSize2.y, RGBA_MAKE(0,15,128,125),haQuestFuryOfPhantom[9]);  																						
		break;
		case 11:
			cSinHelp.sinShowHelp(HAQUEST_CODE_FURYOFPHANTOM,QuestMessageBoxPosi2.x,QuestMessageBoxPosi2.y,QuestMessageBoxSize2.x,QuestMessageBoxSize2.y, RGBA_MAKE(0,15,128,125),haQuestFuryOfPhantom[10]);  																						
			SaveGameData();		
		break;
		case 12:
			cSinHelp.sinShowHelp(HAQUEST_CODE_FURYOFPHANTOM,QuestMessageBoxPosi2.x,QuestMessageBoxPosi2.y,QuestMessageBoxSize2.x,QuestMessageBoxSize2.y, RGBA_MAKE(0,15,128,125),haQuestFuryOfPhantom[11]);  																						
		break;
	}	
	return true;
}
/*----------------------------------------------------------------------------*
*					  showFuryQuestitem	
*-----------------------------------------------------------------------------*/
int  cHaQuest::showFuryQuestItem(int Kind)
{
	//
	if(Kind)
		cSinHelp.sinShowHelp(HAQUEST_CODE_FURYOFPHANTOM,QuestMessageBoxPosi2.x,QuestMessageBoxPosi2.y,QuestMessageBoxSize2.x,QuestMessageBoxSize2.y, RGBA_MAKE(0,15,128,125),haQuestFuryOfPhantom[11]);  																						
	else{
		chaQuest.bFuryQuestFlag = true; 
		if(iFuryQuestIndex[1])
			cSinHelp.sinShowHelp(HAQUEST_CODE_FURYOFPHANTOM,QuestMessageBoxPosi2.x,QuestMessageBoxPosi2.y,QuestMessageBoxSize2.x,QuestMessageBoxSize2.y, RGBA_MAKE(0,15,128,125),haQuestFuryOfPhantomItem[iFuryQuestIndex[1]-1]);
		else
			return false;
	}
	return true;
}
/*----------------------------------------------------------------------------*
*					  RecvBattleNpcResult
*-----------------------------------------------------------------------------*/
int  cHaQuest::RecvBattleNpcResult(int flag)
{	
	//
	if(sHaQuest100LV.State==4 ||sHaQuest100LV.State==9 || sHaQuest100LV.State==11)
	{
		if(flag){    //승리
			FuryBattleFlag = true;
			cSinHelp.sinShowHelp(HAQUEST_CODE_FURYOFPHANTOM,QuestMessageBoxPosi2.x,QuestMessageBoxPosi2.y,QuestMessageBoxSize2.x,QuestMessageBoxSize2.y, RGBA_MAKE(0,15,128,125),haQuestFuryOfPhantomVictory);  																						
		}
		else{        //패배
			cMessageBox.ShowMessage(MESSAGE_100LVBATTLELOSE);		
		}
		return true;
	}
	return false;
}
 
