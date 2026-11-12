/*----------------------------------------------------------------------------*
*	파일명 :  Morayion.cpp	
*	하는일 :  모라이온의 스킬
*	작성일 :  최종업데이트 9월
*	적성자 :  박상열 
*-----------------------------------------------------------------------------*/	

#include "..\\sinLinkHeader.h"

/*----------------------------------------------------------------------------*
*						기본 설정 함수 	
*-----------------------------------------------------------------------------*/	
//스킬을 사용할수있는지를 체크한다 
DWORD sinNotToggleSkill_CODE[][5] = {
	{SKILL_HOLY_BODY,SKILL_HOLY_VALOR,SKILL_DRASTIC_SPIRIT,0},
	{SKILL_ZENITH,0},
	{SKILL_WINDY,0},
	{SKILL_HOLY_REFLECTION,0},
	{SKILL_SPIRIT_ELEMENTAL,0},
	{SKILL_DANCING_SWORD,0},
	{SKILL_CONCENTRATION,0},
	{SKILL_SWIFT_AXE,0},
	{SKILL_FALCON,0},
	{SKILL_GOLDEN_FALCON,0},
	{SKILL_SCOUT_HAWK,0},
	{SKILL_DIVINE_INHALATION,0},
	{SKILL_METAL_ARMOR,0},
	{SKILL_SPARK_SHIELD,0},
	{SKILL_VANISH,0},
	{SKILL_GODLY_SHIELD,0},
	{SKILL_GOD_BLESS,0},
	{SKILL_FROST_JAVELIN,0},
	{SKILL_REGENERATION_FIELD,0},
	{SKILL_SUMMON_MUSPELL,SKILL_SUMMON_RAMIEL,0},		// tier 5: the two share the server's slot
	{SKILL_SUMMON_RAMIEL,SKILL_SUMMON_MUSPELL,0},
	{SKILL_FIRE_ELEMENTAL,0},
	{SKILL_DISTORTION,0},
//	{SKILL_COMPULSION,0}, // 장별 - 스킬밸런스 조정(10.08.10)
	{SKILL_MAGNETIC_SPHERE,0},
	{SKILL_METAL_GOLEM,0},
	{SKILL_BERSERKER,0},
	{SKILL_BLOODY_BERSERKER,0},		// tier 5 (it replaces a running Berserker: F_Bloody_Berserker)
	{SKILL_HYPER_SONIC,0},
	{SKILL_POWER_ENHANCEMENT,0},
	{SKILL_AMPLIFIED,0},
	{SKILL_HOLY_BENEDIC,0},
	{SKILL_TALARIA,0},
	{SKILL_SUMMON_ARCUDA,0},
	{SKILL_BLESSING_KRISHNA,0},
	{SKILL_THIRD_ANIMA,0},
	{SKILL_ASSASSIN_EYE,0},
	{SKILL_VAGUE,0},
	{SKILL_RECALL_WOLVERIN,0},
	//공성전 아이템 스킬
	{SCROLL_INVULNERABILITY,0},
	{SCROLL_CRITICAL,0},
	{SCROLL_EVASION,0},
	{STONE_R_FIRECRYTAL,0},
	{STONE_R_ICECRYTAL,0},
	{STONE_R_LINGHTINGCRYTAL,0},
	{STONE_A_FIGHTER,0},
	{STONE_A_MECHANICIAN,0},
	{STONE_A_PIKEMAN,0},
	{STONE_A_ARCHER,0},
	{STONE_A_KNIGHT,0},
	{STONE_A_ATALANTA,0},
	{STONE_A_MAGICIAN,0},
	{STONE_A_PRIESTESS,0},
	//{SKILL_TRIUMPH_OF_VALHALLA,SKILL_HALL_OF_VALHALLA,0},
	0,    //0을 써주지않으면 무한루프를 돌수있다
};

int sinCheckSkillUseOk(sSKILL *lpSkill)
{ 

	ContinueSkillCancelFlag = 0; //이곳에서 초기화해준다 
	int i = 0;

	//SkillInfo 데이타를 검사한다 
	if(!cSkill.CheckCharSkillInfo())return FALSE;  

	#define  CHAR_SKILL_MASK 0xFF000000
	//자신의 스킬이 맞는지를 체크한다 
	int   CharCode[8] = {SIN_CHAR_MECANICIAN,SIN_CHAR_FIGHTER,SIN_CHAR_ARCHER,SIN_CHAR_PIKEMAN,SIN_CHAR_KNIGHT,SIN_CHAR_ATALANTA,SIN_CHAR_PRIESTESS,SIN_CHAR_MAGICIAN};
	DWORD CharSkillCode[8] = {GROUP_MECHANICIAN,GROUP_FIGHTER ,  GROUP_ARCHER,GROUP_PIKEMAN,GROUP_KNIGHT,GROUP_ATALANTA,GROUP_PRIESTESS,GROUP_MAGICIAN};

	int j; for( j = 0 ; j < 8 ; j++){ 
		if(CharCode[j] == sinChar->JOB_CODE){
			if((lpSkill->CODE & CHAR_SKILL_MASK) !=CharSkillCode[j] ){
				return FALSE;
			}
		}
	}


	/*
	//마나와 스테미나를 한번더 체크한다 // 장별-마나 리듀스 썼을 경우
	if(lpSkill->Skill_Info.UseMana)
	{ 
		if( chaPremiumitem.m_ManaReducePotiontime > 0 )
		{
			if(sinGetMana() >= lpSkill->Skill_Info.UseMana[lpSkill->Point - 1] - (lpSkill->Skill_Info.UseMana[lpSkill->Point - 1] * chaPremiumitem.m_ManaReducePotionValue / 100) && 
				sinGetStamina() >= lpSkill->Skill_Info.UseStamina[0] + (lpSkill->Skill_Info.UseStamina[1]*(lpSkill->Point-1)))
			{
				return TRUE;
			}
			else
				return FALSE;
		}
		else
		{
			if((sinGetMana() - lpSkill->Skill_Info.UseMana[lpSkill->Point - 1]) < 0)
			return FALSE;
		}
	}

	if(lpSkill->Skill_Info.UseStamina[0]){
		if((sinGetStamina() - (lpSkill->Skill_Info.UseStamina[0] + (lpSkill->Skill_Info.UseStamina[1]*(lpSkill->Point-1)))) < 0 )
			return FALSE;

	}
	*/


	/*
	 //마나와 스테미나를 한번더 체크한다 // 장별-마나 리듀스 썼을 경우
	if(lpSkill->Skill_Info.UseMana)
	{ 
		if(chaPremiumitem.m_ManaReducePotiontime > 0) // 박재원 - 마나 리듀스 사용중일 경우 스킬 사용가능한 마나도 함께 줄인다.
		{
			if(sinGetMana() - ( lpSkill->Skill_Info.UseMana[lpSkill->Point - 1] - (lpSkill->Skill_Info.UseMana[lpSkill->Point - 1] * chaPremiumitem.m_ManaReducePotionValue / 100) ) < 0)
				return FALSE;
		}
		else
		{
			if((sinGetMana() - lpSkill->Skill_Info.UseMana[lpSkill->Point - 1]) < 0)
				return FALSE;	    
		}  
	}
	*/

	// 장별 - 테리 속성
	if(chaPremiumitem.m_ManaReducePotiontime > 0 && chaPremiumitem.m_TerryTime > 0 && sinChar->GravityScroolCheck[1] == 1) // 박재원 - 마나 리듀스 사용중일 경우 스킬 사용가능한 마나도 함께 줄인다.
	{
		if(sinGetMana() - ( lpSkill->Skill_Info.UseMana[lpSkill->Point - 1] - ((lpSkill->Skill_Info.UseMana[lpSkill->Point - 1] * chaPremiumitem.m_ManaReducePotionValue / 100) + 
			(lpSkill->Skill_Info.UseMana[lpSkill->Point - 1] * 20 / 100))) < 0)
			return FALSE;
	}


	else if( chaPremiumitem.m_ManaReducePotiontime > 0 )
	{
		if(sinGetMana() - ( lpSkill->Skill_Info.UseMana[lpSkill->Point - 1] - (lpSkill->Skill_Info.UseMana[lpSkill->Point - 1] * chaPremiumitem.m_ManaReducePotionValue / 100) ) < 0)
			return FALSE;
	}

	else if( chaPremiumitem.m_TerryTime > 0 && sinChar->GravityScroolCheck[1] == 1)
	{
		if(sinGetMana() - ( lpSkill->Skill_Info.UseMana[lpSkill->Point - 1] - (lpSkill->Skill_Info.UseMana[lpSkill->Point - 1] * 20 / 100) ) < 0)
			return FALSE;
	}

	else
	{
		if((sinGetMana() - lpSkill->Skill_Info.UseMana[lpSkill->Point - 1]) < 0)
			return FALSE;	    
	}  




	//토글되면 안되는 스킬을 체크해준다 
	int CntSkillNum = 0;  //스킬을 카운트 해준다
	int k = 0;
	
	int Index3 = 0; //포스오브네이처 
	if(Index3 = cSkill.SearchContiueSkillIndex(SKILL_FORCE_OF_NATURE)){
		if(ContinueSkill[Index3-1].Flag == 1 && lpSkill->CODE == SKILL_FORCE_OF_NATURE)
			return FALSE;
	}

	if(Index3 = cSkill.SearchContiueSkillIndex(SKILL_HALL_OF_VALHALLA)){
		if(ContinueSkill[Index3-1].Flag == 1 && lpSkill->CODE == SKILL_HALL_OF_VALHALLA)
			return FALSE;
	}

	while(1){
		if(!sinNotToggleSkill_CODE[CntSkillNum][0])break;    //지속형 스킬이 아니라면 while문을 빠져나온다.
		for(i = 0 ; 0 != sinNotToggleSkill_CODE[CntSkillNum][i] ; i++){
            if(sinNotToggleSkill_CODE[CntSkillNum][i] == lpSkill->CODE ){ //현재 사용된 지속형스킬을 찾는다.
				//같이 사용하면 안돼는 스킬을 체크해준다.
				for(k = 0 ; 0 != sinNotToggleSkill_CODE[CntSkillNum][k] ; k++){ 
					for( j = 0 ; j < MAX_CONTINUE_SKILL ; j++){   
						if(ContinueSkill[j].Flag && ContinueSkill[j].CODE == sinNotToggleSkill_CODE[CntSkillNum][k]){
							return FALSE;
						}
					}
				}
			}
		}
		CntSkillNum++;  
	}


	int UseFlag  = 0; 
	int UseFlag2 = 0;

	if(lpSkill->Skill_Info.UseWeaponCode[0]){
		UseFlag = 1;
		for(int i=0;i<8;i++){
			if(lpSkill->Skill_Info.UseWeaponCode[i]){
				if(lpSkill->Skill_Info.UseWeaponCode[i] == (cInvenTory.InvenItem[sInven[0].ItemIndex-1].CODE&sinITEM_MASK2)){ //
					UseFlag2 = 1;
					break;
				}
			}
		}
	}

	//메탈아머 매특만 사용할수있음
	if(lpSkill->CODE == SKILL_METAL_ARMOR){
		if(sInven[2].ItemIndex){
			if(cInvenTory.InvenItem[sInven[2].ItemIndex-1].sItemInfo.JobCodeMask == sinChar->JobBitMask){
				if(lpSkill->UseSkillFlag){
					return TRUE;
				}
			}
		}
		return FALSE;
	}

	if(UseFlag){
		if(UseFlag2 && lpSkill->UseSkillFlag)
			return TRUE;
	}
	else{
		if(lpSkill->UseSkillFlag)
			return TRUE;
	}

	//////////// 아틀란타 방패던지기 , 쉴드 스트라이크
	if(lpSkill->CODE == SKILL_SHIELD_STRIKE || lpSkill->CODE == SKILL_EXTREME_SHIELD || lpSkill->CODE == SKILL_DIVINE_INHALATION || lpSkill->CODE == SKILL_SPARK_SHIELD || lpSkill->CODE == SKILL_GODLY_SHIELD){
		if(lpSkill->UseSkillFlag){
			if(lpSkill->Skill_Info.UseWeaponCode[0] == (cInvenTory.InvenItem[sInven[1].ItemIndex-1].CODE&sinITEM_MASK2)){ //
				return TRUE;
			}
		}
	}

	/////////// 소울석커 (믹스쳐한 방패와 에이징한 방패만사용할수있다)
	if(lpSkill->CODE == SKILL_SOUL_SUCKER){ //믹스처랑 에이징 아이템이 아닌경우는 사용할수없다 
		if(cInvenTory.InvenItem[sInven[1].ItemIndex-1].sItemInfo.ItemKindCode == ITEM_KIND_CRAFT ||
			cInvenTory.InvenItem[sInven[1].ItemIndex-1].sItemInfo.ItemKindCode == ITEM_KIND_AGING ){
				if(lpSkill->Skill_Info.UseWeaponCode[0] == (cInvenTory.InvenItem[sInven[1].ItemIndex-1].CODE&sinITEM_MASK2)){ //
					if(lpSkill->UseSkillFlag){	//<============== 여기
						return TRUE;
					}
				}
			}
	
	}
	return FALSE;
}

//사용한 스킬의 기력과 마나를 소모한다
int sinUseManaStamina(sSKILL *lpSkill)
{
	/*
	if( chaPremiumitem.m_ManaReducePotiontime <= 0 )	// pluto 마나 리듀스 포션
	{
		if(lpSkill->Skill_Info.UseMana){
			sinSetMana(sinGetMana() - lpSkill->Skill_Info.UseMana[lpSkill->Point - 1]);
		}
	}
	else
	{
		if( lpSkill->Skill_Info.UseMana )
		{
			sinSetMana( sinGetMana() - ( lpSkill->Skill_Info.UseMana[ lpSkill->Point - 1] - (lpSkill->Skill_Info.UseMana[ lpSkill->Point - 1] * chaPremiumitem.m_ManaReducePotionValue / 100) )  );
		}
	}
	*/

	// 장별 - 테리 속성
	if( chaPremiumitem.m_ManaReducePotiontime > 0 && chaPremiumitem.m_TerryTime > 0  && sinChar->GravityScroolCheck[1] == 1)
	{
		sinSetMana( sinGetMana() - ( lpSkill->Skill_Info.UseMana[ lpSkill->Point - 1] - 
			((lpSkill->Skill_Info.UseMana[ lpSkill->Point - 1] * chaPremiumitem.m_ManaReducePotionValue / 100) +
			(lpSkill->Skill_Info.UseMana[ lpSkill->Point - 1] * 20 / 100)) )  );
	}

	else if( chaPremiumitem.m_ManaReducePotiontime > 0 )
	{
		sinSetMana( sinGetMana() - ( lpSkill->Skill_Info.UseMana[ lpSkill->Point - 1] - (lpSkill->Skill_Info.UseMana[ lpSkill->Point - 1] * chaPremiumitem.m_ManaReducePotionValue / 100) )  );
	}

	else if( chaPremiumitem.m_TerryTime > 0 && sinChar->GravityScroolCheck[1] == 1 )
	{
		sinSetMana( sinGetMana() - ( lpSkill->Skill_Info.UseMana[ lpSkill->Point - 1] - (lpSkill->Skill_Info.UseMana[ lpSkill->Point - 1] * 20 / 100) )  );
	}

	else
	{
		if(lpSkill->Skill_Info.UseMana)
			sinSetMana(sinGetMana() - lpSkill->Skill_Info.UseMana[lpSkill->Point - 1]);
	}


	// 박재원 - 근력 리듀스 포션
	if(lpSkill->Skill_Info.UseStamina[0])
	{
		if( chaPremiumitem.m_StaminaReducePotiontime <= 0 )
		{
			sinSetStamina(sinGetStamina() - (lpSkill->Skill_Info.UseStamina[0] + (lpSkill->Skill_Info.UseStamina[1]*(lpSkill->Point-1))));
		}
		else
		{
			sinSetStamina(sinGetStamina() - ( (lpSkill->Skill_Info.UseStamina[0] + (lpSkill->Skill_Info.UseStamina[1]*(lpSkill->Point-1)))
				- ( (lpSkill->Skill_Info.UseStamina[0] + (lpSkill->Skill_Info.UseStamina[1]*(lpSkill->Point-1))) * chaPremiumitem.m_StaminaReducePotionValue / 100)) );
		}
	}

	cSkill.CheckSkillMasteryForm(lpSkill); // 스킬 마스터리를 체크한다
	lpSkill->Mastery =0; //사용후 딜레이값 초기화 
	cSkill.ReformSkillMasteryForm(lpSkill);

	
	lpSkill->UseSKillIncreCount++;
	if(lpSkill->UseSKillIncreCount >= sinMasteryIncreaIndex[lpSkill->Skill_Info.SkillNum]+((lpSkill->Point-1)/3)){
		lpSkill->UseSKillIncreCount = 0;
		lpSkill->UseSkillCount += USE_SKILL_MASTERY_COUNT;

	}

	return TRUE;
}

//지속적으로 사용될 스킬을 셋팅한다 
int sinContinueSkillSet(sSKILL *lpSkill)
{
	int i = 0;
	//에너지 쉴드만 예외처리(해외)
	if(lpSkill->CODE == SKILL_ENERGY_SHIELD|| lpSkill->CODE == SKILL_COMPULSION) // 장별 - 스킬밸런스 조정(10.08.10)
	{
		for( i = 0 ; i < MAX_CONTINUE_SKILL ; i++)
		{
			if(ContinueSkill[i].Flag )
			{
				if(ContinueSkill[i].CODE == SKILL_ENERGY_SHIELD || ContinueSkill[i].CODE == SKILL_COMPULSION) // 장별 - 스킬밸런스 조정(10.08.10)
				{
					memset(&ContinueSkill[i],0,sizeof(sSKILL));
					break;
				}
			}
		}
	}

	for(i = 0 ; i < MAX_CONTINUE_SKILL ; i++){
		if(!ContinueSkill[i].Flag){
			memcpy(&ContinueSkill[i],lpSkill,sizeof(sSKILL));
			ContinueSkill[i].Flag = 1;
			cSkill.SearchSkillDoc(&ContinueSkill[i]);
			if((lpSkill->CODE != SKILL_FORCE_ORB && lpSkill->CODE != CHANGE_JOB3_QUEST &&lpSkill->CODE != CHANGE_ELEMENTARY_QUEST &&
				lpSkill->CODE != BOOSTER_ITEM_LIFE && lpSkill->CODE != BOOSTER_ITEM_MANA && lpSkill->CODE != BOOSTER_ITEM_STAMINA))  // 박재원 - 부스터 아이템
				ContinueSkill[i].MatIcon = 0;//초기화한번더해준다
			break;

		}
	}

	return TRUE;
}
//시간이 지나면 지속사용스킬셋팅을 초기화한다
int sinCheckContinueSkill()
{
	//템스크론 cpp에서 체크하고있음
	/*
	for(int i = 0 ; i < MAX_CONTINUE_SKILL ; i++){
		if(ContinueSkill[i].Flag){
			ContinueSkill[i].Time[0]++;
			if(ContinueSkill[i].Time[0] >= ContinueSkill[i].Time[1]*70){
				memset(&ContinueSkill[i],0,sizeof(sSKILL));
				cInvenTory.SetItemToChar(); //셋팅을 변경한다 
			}
		}
	}
	*/
	return TRUE;
}
/*----------------------------------------------------------------------------*
*						스킬을 토글한다 
*-----------------------------------------------------------------------------*/	
int sinSwitchSkill(sSKILL *pSkill)
{
	/* 파티원 스킬때문에 스킬을 스위치하지않는다 
	int i = 0;
	switch(pSkill->CODE){
		case SKILL_HOLY_VALOR:
			for(i = 0; i < 10 ; i++){
				if(ContinueSkill[i].CODE == SKILL_HOLY_BODY || ContinueSkill[i].CODE == SKILL_DRASTIC_SPIRIT){
					memset(&ContinueSkill[i],0,sizeof(sSKILL));
					cInvenTory.SetItemToChar();

				}
			}
		break;
		case SKILL_HOLY_BODY:
			for(i = 0; i < 10 ; i++){
				if(ContinueSkill[i].CODE == SKILL_HOLY_VALOR || ContinueSkill[i].CODE == SKILL_DRASTIC_SPIRIT){
					memset(&ContinueSkill[i],0,sizeof(sSKILL));
					cInvenTory.SetItemToChar();
				}
			}
		break;
		case SKILL_DRASTIC_SPIRIT:
			for(i = 0; i < 10 ; i++){
				if(ContinueSkill[i].CODE == SKILL_HOLY_VALOR || ContinueSkill[i].CODE == SKILL_DRASTIC_SPIRIT){
					memset(&ContinueSkill[i],0,sizeof(sSKILL));
					cInvenTory.SetItemToChar();
				}
			}
		break;

	}
	*/

	return TRUE;
}

/*----------------------------------------------------------------------------*
*						모라이온 스킬함수 
*-----------------------------------------------------------------------------*/	

//////////////// 나이트
int F_Sword_Blast()
{
	//할일엄따	
	sinUseManaStamina(pUseSkill);
	return TRUE;
}
int F_HolyBody()	
{
	pUseSkill->UseTime = HolyBody_Time[pUseSkill->Point-1];
	sinContinueSkillSet(pUseSkill);
	sinUseManaStamina(pUseSkill);
	sinSwitchSkill(pUseSkill);
	return TRUE;
}
int F_Physical_Traning()
{
	//할일엄따
	return TRUE;
}
int F_Double_Crash()
{
	//할일엄따
	sinUseManaStamina(pUseSkill);	
	return TRUE;
}

/////////// 아틀란타
int F_S_Strike()
{
	//할일엄따
	sinUseManaStamina(pUseSkill);	
	return TRUE;
}
int F_Farina()
{
	//할일 엄따
	sinUseManaStamina(pUseSkill);	
	return TRUE;
}
int F_D_Mastery()
{
	//으앙~~ 나두 할일엄따
	
	return TRUE;
}
int F_V_Spear()
{
	//으아아앙 나두 할일이 엄떠여 ㅜ.ㅜ 
	sinUseManaStamina(pUseSkill);	
	return TRUE;
}

/////////// 프리스티스
int F_Healing()
{
	//에이 나두 할일엄떠여...
	sinUseManaStamina(pUseSkill);	
	return TRUE;
}
int F_HolyBolt()
{
	//으앙 난 모하라구
	sinUseManaStamina(pUseSkill);	
	return TRUE;
}
int F_M_Spark()
{
	//흑흑 일좀 시켜줘여...
	sinUseManaStamina(pUseSkill);	
	return TRUE;
}
int F_HolyMind()
{
	//으앙 나두 할일이 엄떠여...
	sinUseManaStamina(pUseSkill);	
	return TRUE;
}

/////////// 메지션
int F_Agony()
{
	short TempLife,TempLife2,TempMana;
	sinUseManaStamina(pUseSkill);	
	TempLife = (short)(((float)sinChar->Life[1]/100.0f)*(float)Agony_ConvLife[pUseSkill->Point-1]);
	TempMana = (short)(((float)sinChar->Mana[1]/100.0f)*(float)Agony_ConvLife[pUseSkill->Point-1]);
	if((sinGetLife()-TempLife) > 0){
		TempLife2 = sinGetLife()-TempLife;
		sinSetLife(TempLife2);
		TempMana = sinGetMana()+TempMana;
		sinSetMana(TempMana);
	
	};
	
	//
	//sinSetLife

	return TRUE;
}
int F_FireBolt()
{
	//메롱
	sinUseManaStamina(pUseSkill);	
	return TRUE;
}
int F_Zenith()
{
	//pUseSkill->Time[1] = Zenith_Time[pUseSkill->Point-1];
	pUseSkill->UseTime = Zenith_Time[pUseSkill->Point-1]; 
	sinContinueSkillSet(pUseSkill);
	sinUseManaStamina(pUseSkill);	
	cInvenTory.SetItemToChar(); //셋팅을 변경한다 
	return TRUE;
}
int F_ZenithFromServer(int Time) //파티원에 적용시키기위해서 서버에서 받아온다 
{
	for(int i = 0 ; i < MAX_CONTINUE_SKILL ; i++){
		if(!ContinueSkill[i].Flag){
			ContinueSkill[i].Flag = 1;
			ContinueSkill[i].CODE = SKILL_ZENITH;
			ContinueSkill[i].Time[1] = Time;
			ContinueSkill[i].PartyFlag = 1;
			break;

		}
	}
	sinUseManaStamina(pUseSkill);	
	return TRUE;
}

int F_FireBall()
{
	//모라이온 끝!
	sinUseManaStamina(pUseSkill);	
	return TRUE;
}
/*----------------------------------------------------------------------------*
*						모라이온 2차스킬함수 
*-----------------------------------------------------------------------------*/	
//나이트
int F_Holy_Valor()
{

	pUseSkill->UseTime = Holy_Valor_Time[pUseSkill->Point-1];
	sinContinueSkillSet(pUseSkill);
	sinUseManaStamina(pUseSkill);
	sinSwitchSkill(pUseSkill);
	return TRUE;
}
/* 잠시대기 
int F_Holy_Valor_Party() //파티원 적용 
{
	pUseSkill->UseTime = Holy_Valor_Time[pUseSkill->Point-1];
	sinContinueSkillSet(pUseSkill);
	sinUseManaStamina(pUseSkill);
	sinSwitchSkill(pUseSkill);
	return TRUE;
}
*/
int F_Brandish()
{
	sinUseManaStamina(pUseSkill);
	return TRUE;
}
int F_Piercing()
{
	sinUseManaStamina(pUseSkill);
	return TRUE;
}

int F_Drastic_Spirit()
{
	pUseSkill->UseTime = Drastic_Spirit_Time[pUseSkill->Point-1];
	sinContinueSkillSet(pUseSkill);
	sinUseManaStamina(pUseSkill);
	sinSwitchSkill(pUseSkill);
	cInvenTory.SetItemToChar(); //셋팅을 변경한다 
	return TRUE;
}

int F_Windy()
{
	
	pUseSkill->UseTime = Windy_Time[pUseSkill->Point-1];
	sinContinueSkillSet(pUseSkill); 
	sinUseManaStamina(pUseSkill);
	cInvenTory.SetItemToChar(); //셋팅을 변경한다 
	return TRUE;
}

int F_Twist_Javelin()
{
	sinUseManaStamina(pUseSkill);
	return TRUE;
}

int F_Soul_Sucker()
{
	sinUseManaStamina(pUseSkill);
	return TRUE;
}

int F_Fire_Javelin()
{
	sinUseManaStamina(pUseSkill);
	return TRUE;
}


int F_Meditation()
{
	return TRUE;
}

int F_Divine_Lightning()
{
	sinUseManaStamina(pUseSkill);
	return TRUE;
}

int F_Holy_Reflection()
{
	pUseSkill->UseTime = Holy_Reflection_Time[pUseSkill->Point-1];
	sinContinueSkillSet(pUseSkill);
	sinUseManaStamina(pUseSkill);
	return TRUE;
}

int F_Grand_Healing()
{
	sinUseManaStamina(pUseSkill);
	return TRUE;
}

int F_Mental_Mastery()
{
	return TRUE;
}

int F_Watornado()
{
	sinUseManaStamina(pUseSkill);
	return TRUE;
}

int F_Enchant_Weapon()
{

	//pUseSkill->UseTime = Enchant_Weapon_Time[pUseSkill->Point-1];
	//sinContinueSkillSet(pUseSkill);
	sinUseManaStamina(pUseSkill);
	return TRUE;
}

int F_Dead_Ray()
{
	sinUseManaStamina(pUseSkill);
	return TRUE;
}

/*----------------------------------------------------------------------------*
*						모라이온 3차스킬함수 
*-----------------------------------------------------------------------------*/	
//나이트
int F_S_Mastery()
{
	
	return TRUE;
}

int F_D_Inhalation()
{
	pUseSkill->UseTime = D_Inhalation_Time[pUseSkill->Point-1]; 
	sinContinueSkillSet(pUseSkill);
	sinUseManaStamina(pUseSkill);
	SwitchSkill(pUseSkill);
	cInvenTory.SetItemToChar(); //셋팅을 변경한다 
	return TRUE; 
}
int F_Holy_Incantation()
{
	sinUseManaStamina(pUseSkill); 
	return TRUE;
}
int F_Grand_Cross()
{
	sinUseManaStamina(pUseSkill);
	return TRUE;

}

//아탈란타
int F_Split_Javelin()
{
	sinUseManaStamina(pUseSkill);
	return TRUE;
}
int F_T_Of_Valhalla()
{
	//pUseSkill->UseTime = T_Of_Valhalla_Time[pUseSkill->Point-1];
	//sinContinueSkillSet(pUseSkill);
	sinUseManaStamina(pUseSkill);
	//sinSwitchSkill(pUseSkill); //지금 현제는 스위치할게 엄따 
	//cInvenTory.SetItemToChar(); //셋팅을 변경한다 

	if(!pUseSkill->SkillTaget_CODE)
		SetT_Of_ValhallaFlag2 = 1;


	return TRUE;
}
int F_Lightning_Javelin()
{
	sinUseManaStamina(pUseSkill);
	return TRUE;
}
int F_Storm_Javelin()
{
	sinUseManaStamina(pUseSkill);
	return TRUE;
}

//프리스티스
int F_Vigor_Ball()
{
	sinUseManaStamina(pUseSkill);
	return TRUE;
}
int F_Resurrection()
{
	sinUseManaStamina(pUseSkill); //
	return TRUE;
}
int F_Extinction()
{
	sinUseManaStamina(pUseSkill); //
	return TRUE;
}
int F_Virtual_Life() //요놈참 문제일세
{
	//pUseSkill->UseTime = Virtual_Life_Time[pUseSkill->Point-1];
	//sinContinueSkillSet(pUseSkill);
	sinUseManaStamina(pUseSkill); //Set함수에서 해준다
	//sinSwitchSkill(pUseSkill); 
	//cInvenTory.SetItemToChar(); //셋팅을 변경한다 
	return TRUE;
}

//메지션
int F_Energy_Shield()
{
	pUseSkill->UseTime = Energy_Shield_Time[pUseSkill->Point-1];
	sinContinueSkillSet(pUseSkill);
	sinUseManaStamina(pUseSkill);
	return TRUE;
}
int F_Diastrophism()
{
	sinUseManaStamina(pUseSkill);
	return TRUE;
}
int F_Spirit_Elemental()
{
	pUseSkill->UseTime = Spirit_Elemental_Time[pUseSkill->Point-1];
	sinContinueSkillSet(pUseSkill);
	sinUseManaStamina(pUseSkill);
	cInvenTory.SetItemToChar(); //셋팅을 변경한다 
	return TRUE;
}
int F_D_Sword()
{
	pUseSkill->UseTime = Dancing_Sword_Time[pUseSkill->Point-1];
	sinContinueSkillSet(pUseSkill);
	sinUseManaStamina(pUseSkill);
	cInvenTory.SetItemToChar(); //셋팅을 변경한다 
	return TRUE;
}
/*----------------------------------------------------------------------------*
*						모라이온 4차스킬함수 
*-----------------------------------------------------------------------------*/
//나이트
int F_Divine_Piercing()
{
	sinUseManaStamina(pUseSkill);
	return TRUE;
}
int F_Godly_Shield()
{
	pUseSkill->UseTime = Godly_Shield_Time[pUseSkill->Point-1];
	sinContinueSkillSet(pUseSkill);
	sinUseManaStamina(pUseSkill);
	SwitchSkill(pUseSkill);
	return TRUE;
}
int F_God_Bless()
{
	pUseSkill->UseTime = God_Bless_Time[pUseSkill->Point-1];
	sinContinueSkillSet(pUseSkill);
	sinUseManaStamina(pUseSkill);
	cInvenTory.SetItemToChar(); //셋팅을 변경한다 
	return TRUE;
}
int F_Sword_Of_Justice()
{
	sinUseManaStamina(pUseSkill);
	return TRUE;
}

//아틀란타
int F_Hall_Of_Valhalla()
{
	//잠시대기
	sinUseManaStamina(pUseSkill);
	return TRUE;
}
int F_X_Rage()
{
	sinUseManaStamina(pUseSkill);
	return TRUE;
}
int F_Frost_Javelin()
{
	pUseSkill->UseTime = Frost_Javelin_Time[pUseSkill->Point-1];
	sinContinueSkillSet(pUseSkill);
	sinUseManaStamina(pUseSkill);
	cInvenTory.SetItemToChar(); //셋팅을 변경한다 
	return TRUE;
}
int F_Vengeance()
{
	sinUseManaStamina(pUseSkill);
	return TRUE;
}

//프리스티스
int F_Glacial_Spike()
{
	sinUseManaStamina(pUseSkill);
	return TRUE;
}
int F_Regeneration_Field()
{
	//pUseSkill->UseTime = Regeneration_Field_Time[pUseSkill->Point-1];
	sinUseManaStamina(pUseSkill);
	return TRUE;
}
int F_Chain_Lightning()
{
	sinUseManaStamina(pUseSkill);
	return TRUE;
}
int F_Summon_Muspell()
{
	pUseSkill->UseTime = Summon_Muspell_Time[pUseSkill->Point-1];
	sinUseManaStamina(pUseSkill); //소환물은 어케해야할지 대기
	sinContinueSkillSet(pUseSkill);
	return TRUE;
}

//매지션
int F_Fire_Elemental() //조금 생각해봐야함
{
	pUseSkill->UseTime = 6*60;;
	sinContinueSkillSet(pUseSkill);
	sinUseManaStamina(pUseSkill);
	SummonSkillCodeIndex[1] = 0;
	return TRUE;
}
int F_Flame_Wave()
{
	sinUseManaStamina(pUseSkill); //소환물은 어케해야할지 대기
	return TRUE;
}
int F_Distortion()
{
	pUseSkill->UseTime = Distortion_Time[pUseSkill->Point-1];
	//sinContinueSkillSet(pUseSkill);
	sinUseManaStamina(pUseSkill);
	//cInvenTory.SetItemToChar(); //셋팅을 변경한다 
	
	return TRUE;
}
int F_Meteo()
{
	sinUseManaStamina(pUseSkill); //소환물은 어케해야할지 대기
	return TRUE;
}

