/*----------------------------------------------------------------------------*
*	파일명 :  Tempskron.h	
*	하는일 :  템스크론의 스킬 헤더 
*	작성일 :  최종업데이트 1월
*	적성자 :  박상열 
*-----------------------------------------------------------------------------*/	

#define SIN2_ATTACK_RATE		0
#define SIN2_DAMAGE				1
#define SIN2_DEFENSE			2
#define SIN2_ABSORB				3
#define SIN2_SPEED				4
#define SIN2_LIFE				5
#define SIN2_MANA				6 // 박재원 - 부스터 아이템(기력)
#define SIN2_STAMINA			7 // 박재원 - 부스터 아이템(근력)

#define USE_SKILL_MASTERY_COUNT	100

//////////////// 공통 

int RequireStateCheck(int Mana);	//요구 조건을 검사한다 	
int ContinueSkillUse(sSKILL *pSkill,int Time); //지속적으로 사용되는 스킬 
void CheckContinueSkill();	//지속적으로 사용중인 스킬을 체크하여 시간이 지나면 초기화 해준다 
void SwitchSkill(sSKILL *pSkill);   //스킬을 스위치한다 

//////////////// 메카니션 
int F_Extreme_Shield();		//익스트림 쉴드 
int F_Mechanic_Bomb();			//메카닉 범 
int F_Physical_Absorption();	//피지컬 업솝
int F_Poison_Attribute();	//포이즌 어트리뷰트 

int F_Great_Smash();		//그래이트 스매쉬 
int F_Maximize();			//맥시마이즈
int F_Automation();			//오토메이션 
int F_Spark();				//스파크 

int F_Metal_Armor();		//메탈아머
int F_Grand_Smash();		//그랜드 스메쉬
int F_M_W_Mastery();		//메카닉웨폰마스터리
int F_Spark_Shield();		//스파크 쉴드

//4차 스킬
int F_Impulsion();          //임플로션
int F_Compulsion();         //컴플로션
int F_Magnetic_Sphere();    //매네틱 피어
int F_Metal_Golem();        //메탈 골렘

//////////////// 파이터
int F_Melee_Mastery();		//밀리 마스터리 
int F_Fire_Attribute();		//파이어 어트리뷰트 
int F_Raving();				//레이빙 
int F_Impact();				//임펙트 

int F_Triple_Impact();		//트리플 임펙트 
int F_Brutal_Swing();		//부루탈 스윙
int F_Roar();				//로어 
int F_RageOfZecram();		//레이지 오브 자크람

int F_Concentration();		//컨센트레이션
int F_Avanging_Crash();		//어벤징 크래쉬
int F_Swift_Axe();			//스위푸트엑스
int F_Bone_Crash();			//본 크래쉬

//4차 스킬
int F_Destoryer();          //디스토리어
int F_Berserker();          //버서커
int F_Cyclone_Strike();     //싸이클론 스트라이크
int F_Boost_Health();       //부스트 헬스

//Tier 5
int F_Down_Hit();
int F_Power_Dash();
int F_Mortal_Blow();
int F_Bloody_Berserker();
int F_Tier5_Pending();		// a tier 5 skill whose cast is not made yet: it can be learned, not used
int F_Land_Mining();
int F_Hyper_Sonic();
int F_Rolling_Smash();
int F_Power_Enhancement();

//////////////// 파이크맨 
int F_Pike_Wind();			//파이크윈드 
int F_Ice_Attribute();		//아이스 어트리뷰트 
int F_Critical_Hit();		//크리티컬 힛
int F_Jumping_Crash();		//비거 스피어 

int F_Ground_Pike();		//그라운드 스파이크 
int F_Tornado_Pike();		//토네이도  
int F_Weapone_Defence_Mastery();	//웨폰 디펜스 마스터리
int F_Expansion();			//익스펜션 

int F_Venom_Spear();		//베놈스피어
int F_Vanish();				//배니쉬
int F_Critical_Mastery();	//샤프 마스터리
int F_Chain_Lance();		//체인랜스
//4차 스킬
int F_Assassin_Eye();       //어쌔신 아이
int F_Charging_Strike();    //차징 스트라이크
int F_Vague();              //베이그
int F_Shadow_Master();      //새됴우 마스터

//////////////// 아쳐
int F_Scout_Hawk();			//스카우트 호크 
int F_Shooting_Mastery();	//슛팅 마스터리 
int F_Wind_Arrow();			//윈드 에로우 
int F_Perfect_Aim();		//퍼펙트 에임  

int F_Dions_Eye();			//디온스 아이 
int F_Falcon();				//팔콘 
int F_Arrow_of_Rage();		//에로우 오브 레이지 
int F_Avalanche();			//에벨 런츠 

int F_E_Shot();				//엘레멘탈샷
int F_G_Falcon();			//골든팔콘
int F_B_Shot();				//봄샷
int F_Perforation();		//퍼포레이션

//4차 스킬
int F_Recall_Wolverin();    //리콜 울버린
int F_Evasion_Mastery();    //이베이션 마스터리
int F_Phoenix_Shot();       //피닉스 샷
int F_Force_Of_Nature();    //포스 오브 내츄럴




////////////////////////////메카니션 스킬 전역변수 
//익스트림 쉴드 
extern int E_Shield_BlockRate[10];
extern int E_Shield_UseTime[10];
extern int E_Shield_UseMana[10];

//메카닉 범
extern int Mechanic_Bomb_Damage[10][2];
extern int Mechanic_Bomb_Attack_Range[10];
extern int Mechanic_Bomb_UseMana[10];

//피키컬 업솝 
extern int P_Absorb[10][2];
extern int P_Absorb_UseTime[10];
extern int P_Absor_bUseMana[10];

//포이즌 어트리뷰트 
extern int PlusPoison[10];

//그레이트 스매쉬 
extern int G_Smash_Attack_Rate[10];
extern int G_Smash_Attack_Rage[10];
extern int G_Smash_UseMana[10];

//맥시마이즈 
extern int Maximize_Damage[10];
extern int Maximize_Weapon_Speed[10];
extern int Maximize_Time[10]; 
extern int Maximize_UseMana[10];

//오토메이션  
extern int Automation_Damage[10];
extern int Automation_Speed[10];
extern int Automation_Time[10];
extern int Automation_UseMana[10];

//스파크 
extern int Spark_Damage[10][2];
extern int Spark_Area[10];
extern int Spark_UseMana[10];

//메탈 아머
extern int Metal_Armor_Defense[10];
extern int Metal_Armor_Time[10];
extern int Metal_Armor_UseMana[10];

//그랜드 스메쉬
extern int Grand_Smash_AttackRate[10];
extern int Grand_Smash_Damage[10];
extern int Grand_Smash_UseMana[10];

//메카닉 웨폰 마스터리
extern int M_Weapon_Mastey[10];

//스파크 쉴드 
extern int Spark_Shield_Damage[10];
extern int Spark_Shield_Defense[10];
extern int Spark_Shield_Time[10];
extern int Spark_Shield_UseMana[10];

//4차 스킬
//임플리션
extern int Impulsion_LightingDamage[10][2];   //라이트닝 추가 공격력
extern int Impulsion_Range[10];               //사정거리
extern int Impulsion_LightingNum[10];         //볼트 갯수
extern int Impulsion_UseMana[10];

//컴플리션
extern int Compulsion_AddAbsorb[10];		 //추가 흡수력
extern int Compulsion_Area[10];              //주변 범위
extern int Compulsion_Time[10];              //유지 시간
extern int Compulsion_UseMana[10];

//메네틱 피어
extern int Magnetic_Sphere_AddDamage[10][2]; //자기 공격력 증가데미지
extern int Magnetic_Sphere_Area[10];		 //유효반경
extern int Magnetic_Sphere_AttackDelay[10];  //어택 딜레이
extern int Magnetic_Sphere_Time[10];         //유지 타임
extern int Magnetic_Sphere_UseMana[10];

//메탈 골렘
extern int Metal_Golem_Damage[10][2];      //공격력
extern int Metal_Golem_Hit[10];			   //명중력
extern int Metal_Golem_Defense[10];		   //방어력
extern int Metal_Golem_Life[10];           //생명력
extern int Metal_Golem_UseMana[10];

////////////////////////////파이터 스킬 전역변수
//밀리 마스터리 
extern int Melee_Mastery_DamagePercent[10];

//파이어 어트리뷰트 
extern int PlusFire[10];

//레이빙 
extern int Raving_Damage[10];
extern int Ravind_Speed[10];
extern float Raving_UseLife[10];
extern int Raving_UseMana[10];

//임펙트 
extern int Impact_Attack_Rating[10];
extern int Impact_Damage[10];
extern int Impact_UseMana[10];

//트리플 임펙트
extern int T_Impact_Damage[10];
extern int T_Impact_Hit[10];
extern int T_Impact_UseMana[10];

//부르탈 스윙 
extern int B_Swing_Damage[10];
extern int B_Swing_Critical[10];
extern int B_Swing_UseMana[10];

//로어
extern int Roar_Range[10];
extern int Roar_Time[10];
extern int Roar_UseMana[10];

//레이지 오브 제크람 
extern int R_Zecram_Damage[10];
extern int R_Zecram_UseMana[10];

//컨센트레이션
extern int Concentration_AttackRate[10];
extern int Concentration_Time[10];
extern int Concentration_UseMana[10];

//어벤징 크래쉬
extern int A_Crash_Damage[10];
extern int A_Crash_AttackRate[10];
extern int A_Crash_UseMana[10];

//스위프트 엑스
extern int Swift_Axe_Speed[10];
extern int Swift_Axe_Time[10];
extern int Swift_Axe_UseMana[10];

//본 크래쉬
extern int B_Crash_Damage[10];
extern int B_Crash_DemonDamage[10];
extern int B_Crash_UseMana[10];


//**4차 스킬
//디스토리어
extern int Destoryer_DamagePercent[10];   //공격력 가중
extern int Destoryer_AddCritical[10];	   //크리티컬 추가
extern int Destoryer_UseMana[10];

//버서커
extern int Berserker_AddAttack[10];        //추가 공격력
extern int Berserker_SubAbsorb[10];        //흡수력 감소치
extern int Berserker_Time[10];             //유지시간
extern int Berserker_UseMana[10];

//싸이클론 스트라이크
extern int Cyclone_Strike_DamagePercent[10];      //공격력 가중
extern int Cyclone_Strike_AreaDamage[10][2];      //범위 공격력
extern int Cyclone_Strike_AttackNum[10];		  //최대 타격수
extern int Cyclone_Strike_Area[10];               //공격 범
extern int Cyclone_Strike_UseMana[10];

//붓트 헬스
extern int Boost_Health_Life[10];//생명력 영구적 추가

//**Tier 5 (all classes' mana per rank: sinSkill_Info.cpp)
extern int Land_Mining_UseMana[10], Hyper_Sonic_UseMana[10], Rolling_Smash_UseMana[10], Power_Enhancement_UseMana[10];
extern int Down_Hit_UseMana[10], Power_Dash_UseMana[10], Mortal_Blow_UseMana[10], Bloody_Berserker_UseMana[10];
extern int Dancing_Reaper_UseMana[10], Final_Spear_UseMana[10], Amplified_UseMana[10], Side_Spin_Attack_UseMana[10];
extern int Evade_Shot_UseMana[10], Thunder_Rope_UseMana[10], Nature_Splash_UseMana[10], Circle_Trap_UseMana[10];
extern int Soul_Breaker_UseMana[10], Crescent_Moon_UseMana[10], Saint_Blade_UseMana[10], Holy_Benedic_UseMana[10];
extern int Talaria_UseMana[10], Galaxy_Coup_UseMana[10], Summon_Arcuda_UseMana[10], Snippy_Fear_UseMana[10];
extern int Spirit_Impact_UseMana[10], Piercing_Ice_UseMana[10], Summon_Ramiel_UseMana[10], Blessing_Krishna_UseMana[10];
extern int Silraphim_UseMana[10], Vis_Tenus_UseMana[10], Prima_Ignis_UseMana[10], Third_Anima_UseMana[10];

//Mechanician tier 5
extern int Land_Mining_DamagePercent[10];
extern int Land_Mining_AreaDamage[10][2];
extern int Land_Mining_Area[10];
extern int Land_Mining_AttackNum[10];
extern int Hyper_Sonic_Speed[10];
extern int Hyper_Sonic_Time[10];
extern int Rolling_Smash_DamagePercent[10];
extern int Rolling_Smash_AddHit[10];
extern int Power_Enhancement_Damage[10];
extern int Power_Enhancement_Area[10];
extern int Power_Enhancement_Time[10];

//Pikeman tier 5
extern int Dancing_Reaper_DamagePercent[10];
extern int Final_Spear_Area[10];
extern int Final_Spear_AreaDamage[10][2];
extern int Amplified_SubAbsorb[10];
extern int Amplified_AddHit[10];
extern int Amplified_Damage[10];
extern int Amplified_Time[10];
extern int Side_Spin_DamagePercent[10];
extern int Side_Spin_LifeAbsorb[10];
int F_Dancing_Reaper();
int F_Final_Spear();
int F_Amplified();
int F_Side_Spin_Attack();

//Fighter tier 5
extern int Down_Hit_DamagePercent[10];
extern int Down_Hit_AddHit[10];
extern int Power_Dash_DamagePercent[10];
extern int Power_Dash_StunTime[10];
extern int Power_Dash_Range[10];
extern int Mortal_Blow_DamagePercent[10];
extern int Mortal_Blow_AreaDamage[10][2];
extern int Mortal_Blow_Area[10];
extern int Mortal_Blow_AttackNum[10];
extern int Bloody_Berserker_AddAttack[10];
extern int Bloody_Berserker_SubAbsorb[10];
extern int Bloody_Berserker_Time[10];
#define	BERSERKER_PARAM_BLOODY	0x100
int Berserker_AddAttackOf( int dwParam );		// the server's Berserker param -> added attack
int Berserker_SubAbsorbOf( int dwParam );		// -> absorption change (negative)


///////////////////////////파이크맨 스킬 전역변수
//파이크 윈드 
extern int Pike_Wind_Damage[10][2];
extern int Pike_Wind_Push_Lenght[10];
extern int Pike_Wind_Throw_Height[10];
extern int Pike_Wind_UseMana[10];

//아이스 어트리뷰트 
extern int PlusIce[10];

//크리티컬 힛 
extern int Critical_Hit_Critical[10];
extern int Critical_Hit_UseMana[10];

//점핑 크래쉬 
extern int Jumping_Crash_UseMana[10];
extern int Jumping_Crash_Attack_Rating[10];
extern int Jumping_Crash_Damage[10];

//그라운드 스파이크 
extern int G_Pike_Damage[10][2];
extern int G_Pike_Range[10];
extern int G_Pike_UseMana[10];
extern int G_Pike_Time[10];

//토네이도 
extern int Tornado_Damage[10];
extern int Tornado_Stun[10];
extern int Tornado_Range[10];
extern int Tornado_UseMana[10];

//웨폰 디펜스 마스터리 
extern int W_D_Mastery_Block[10];

//파 
extern int Expansion_Size[10];
extern int Expansion_UseMana[10];
extern int Expansion_Damage[10];

//베놈 스피어
extern int VenomSpear_Damage[10][2];
extern int VenomSpear_Num[10];
extern int VenomSpear_Time[10];
extern int VenomSpear_Range[10];
extern int VenomSpear_UseMana[10];

extern float Vanish_Speed[10];
extern int Vanish_Damage[10];
extern int Vanish_Range[10];
extern int Vanish_Time[10];
extern int Vanish_UseMana[10];

//샤프 마스터리
extern int Critical_Mastery_Critical[10];

//체인 랜스
extern int Chain_Lance_Damage[10];
//extern int Chain_Lance_AttackRate[10];
extern int Chain_Lance_UseMana[10];

//**4차스킬
//어쌔신 아이
extern int Assassin_Eye_AddCritical[10];       //크리티컬 공격력 증가
extern int Assassin_Eye_M_SubCritical[10];     //대 몬스터 크리티컬 능력 약화
extern int Assassin_Eye_Time[10];              //유지타임
extern int Assassin_Eye_UseMana[10];

//차징 스트라이크
extern int Charging_Strike_DamagePercent[10];        //공격력 가중
extern int Charging_Strike_CharingDamagePercent[10]; //차징시 공격력 가중
extern int Charging_Strike_Time[10];               //차징 시간
extern int Charging_Strike_UseMana[10];

//베이그
extern int Vague_EvasionPercent[10];  //추가 회피확률 중가
extern int Vague_Time[10];            //유지시간
extern int Vague_UseMana[10];

//새됴우 마스터
extern int Shadow_Master_DamagePercent[10];       //공격력 가중
extern int Shadow_Master_AddHit[10];              //명중력 증가
extern int Shadow_Master_ShadowNum[10];			  //잔상 콤보의 증가
extern int Shadow_Master_UseMana[10];


///////////////////////////아쳐 스킬 전역변수
//스카우트 호크 
extern int Scout_Hawk_Attack_Rate[10];
extern int Scout_Hawk_UseTime[10];
extern int Scout_Hawk_UseMana[10];

//슈팅 마스터리 
extern int S_Mastery_DamagePercent[10];

//윈드 에로우 
extern int Wind_Arrow_Speed[10];
extern int Wind_Arrow_Damage[10];
extern int Wind_Arrow_UseMana[10];

//퍼팩트 에임 

extern int Perfect_Aim_Attack_Rate[10];
extern int Perfect_Aim_Damage[10];
extern int Perfect_Aim_Damage_LV[10];
extern int Perfect_Aim_UseMana[10];

//디온스 아이 
extern int D_Eye_Attack_Rate[10];

//팔콘 
extern int Falcon_Damage[10][2];
extern int Falcon_Time[10];
extern int Falcon_UseMana[10];

//에로우 오브 레이지 
extern int A_Rage_ArrowNum[10];
extern int A_Rage_Damage[10];
extern int A_Rage_Area[10];
extern int A_Rage_UseMana[10];

//에벨란체 
extern int Avalanche_Speed[10];
extern int Avalanche_Damage[10];
extern int Avalanche_ArrowNum[10];
extern int Avalanche_UseMana[10];

//엘레멘탈 샷
extern int Elemental_Shot_Ice[10][2];
extern int Elemental_Shot_Fire[10][2];
extern int Elemental_Shot_Lightning[10][2];
extern int Elemental_Shot_UseMana[10];

//골든 팰컨
extern int   Golden_Falcon_Damage[10];
extern float Golden_Falcon_LifeRegen[10];
extern int   Golden_Falcon_Time[10];
extern int   Golden_Falcon_UseMana[10];

//범 샷
extern int BombShot_Damage[10];
extern int BombShot_DemonDamage[10];
extern int BombShot_AddDamage[10][2];
extern int BombShot_Area[10];
extern int BombShot_UseMana[10];

//퍼포레이션
extern int Perforation_Damage[10];
extern int Perforation_Critical[10];
extern int Perforation_UseMana[10];
extern int Perforation_Attack_Range[10];

//**4차 스킬
//리콜 울버린
extern int Recall_Wolverin_Damage[10][2];//공격력
extern int Recall_Wolverin_Hit[10];      //명중력
extern int Recall_Wolverin_Defense[10];  //방어력
extern int Recall_Wolverin_Life[10];     //생명력
extern int Recall_Wolverin_UseMana[10];

//이베이션 마스터리
extern int Evasion_Mastery_AddPercent[10];   //추가 회피  능력 증가

//피닉스 샷
extern int   Phoenix_Shot_DamagePercent[10];      //공격력 가중
extern int   Phoenix_Shot_Time[10];               //차징 시간
extern int   Phoenix_Shot_UseMana[10];

//포스 오브 내츄럴
extern int Force_Of_Nature_AddDamage[10];		  //추가 공격력
extern int Force_Of_Nature_AddHit[10];			  //추가 명중력
extern int Force_Of_Nature_FalconAddDamage[10];	  //펠컨의 추가 공격력
extern int Force_Of_Nature_Time[10];              //유지시간
extern int Force_Of_Nature_UseMana[10];



//스킬 습득시 사용되는 돈 
extern int sinMasterSkill_Money[SIN_MAX_USE_SKILL-1];
extern int sinMasterSkill_PlusMoney[SIN_MAX_USE_SKILL-1];


// 스킬 습득시 컬러 색을 바꾼다 
extern int SkillTextColor[10];

//요구치를 충족못했을경우에는 효과를 준다 
extern int NotUseSkillElement[3];	//요구치 충족 플렉 
extern int sinMasteryIncreaIndex[SIN_MAX_USE_SKILL-1];


//공성전 클랜스킬 
extern int C_Skill_HpPoint[10];      //hp 상승
extern int C_Skill_AttPercent[10];   //공격력 상승
extern int C_Skill_EvaPercent[10];      //회피율 상승