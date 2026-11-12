/*----------------------------------------------------------------------------*
*	파일명 :  sinSkill.h
*	하는일 :  아이템 관리 
*	작성일 :  최종업데이트 12월 
*	적성자 :  박상열 
*-----------------------------------------------------------------------------*/	
#define SIN_MAX_SKILL			200			//넓게 잡아놓는다 (200: room for the tier 5 skills)
#define SIN_MAX_USE_SKILL		21			//한캐릭에서 사용할수있는 최대 스킬치 (노말 어택포함)	(1 + 4 tiers x 4 + tier 5's 4)
#define SIN_MAX_SKILLBOX		21
#define SIN_TIER5_FIRST			17			// UseSkill index of the first tier 5 skill (17..20, the skill window's second page)

#define	sinSKILL_MASK1	0xFF000000			//마스크 
#define	sinSKILL_MASK2	0xFFFF0000
#define	sinSKILL_MASK3	0x0000FFFF

#define SKILL_ALL_WEAPON_USE	0x11000000  //모든 무기 사용가능 코드 
 
////////////////////////////////////////////////// 직업군 CODE
///////////////////////////////// 템스크론 

#define GROUP_MECHANICIAN		0x01000000
#define GROUP_FIGHTER			0x02000000
#define GROUP_PIKEMAN			0x03000000
#define	GROUP_ARCHER			0x04000000

///////////////////////////////// 모라이온 
#define GROUP_KNIGHT			0x05000000
#define GROUP_ATALANTA			0x06000000
#define GROUP_PRIESTESS			0x07000000
#define	GROUP_MAGICIAN			0x08000000

///////////////////////////////// 그외의 스킬
#define	GROUP_OTHERSKILL		0x09000000

/////////////////////////////// 전업 CODE
#define	CHANGE_JOB1				0x00010000
#define	CHANGE_JOB2				0x00020000
#define	CHANGE_JOB3				0x00040000
#define	CHANGE_JOB4				0x00080000
#define	CHANGE_JOB5				0x00100000		// the fifth rank's skills (tier 5)

/////////////////////////////// 스킬 세부코드 
#define SKILL_1					0x00000001
#define SKILL_2					0x00000002
#define SKILL_3					0x00000003
#define SKILL_4					0x00000004
#define SKILL_5					0x00000005
#define SKILL_6					0x00000006
#define SKILL_7					0x00000007
#define SKILL_8					0x00000008
#define SKILL_9					0x00000009
#define SKILL_10				0x00000010

/*----------------------------------------------------------------------------*
*								스킬 CODE (모라이온)
*-----------------------------------------------------------------------------*/	
//////////////// 나이트 그룹 
#define SKILL_SWORD_BLAST		(GROUP_KNIGHT | CHANGE_JOB1 | SKILL_1)
#define SKILL_HOLY_BODY			(GROUP_KNIGHT | CHANGE_JOB1 | SKILL_2)
#define SKILL_PHYSICAL_TRANING	(GROUP_KNIGHT | CHANGE_JOB1 | SKILL_3)
#define SKILL_DOUBLE_CRASH		(GROUP_KNIGHT | CHANGE_JOB1 | SKILL_4)

#define SKILL_HOLY_VALOR		(GROUP_KNIGHT | CHANGE_JOB2 | SKILL_1)
#define SKILL_BRANDISH			(GROUP_KNIGHT | CHANGE_JOB2 | SKILL_2)
#define SKILL_PIERCING			(GROUP_KNIGHT | CHANGE_JOB2 | SKILL_3)
#define SKILL_DRASTIC_SPIRIT	(GROUP_KNIGHT | CHANGE_JOB2 | SKILL_4)

#define SKILL_SWORD_MASTERY 	(GROUP_KNIGHT | CHANGE_JOB3 | SKILL_1)
#define SKILL_DIVINE_INHALATION (GROUP_KNIGHT | CHANGE_JOB3 | SKILL_2)
#define SKILL_HOLY_INCANTATION  (GROUP_KNIGHT | CHANGE_JOB3 | SKILL_3)
#define SKILL_GRAND_CROSS		(GROUP_KNIGHT | CHANGE_JOB3 | SKILL_4)

//4스킬
#define SKILL_SWORD_OF_JUSTICE	(GROUP_KNIGHT | CHANGE_JOB4 | SKILL_1)//스워드 오브 저스틱스
#define SKILL_GODLY_SHIELD	    (GROUP_KNIGHT | CHANGE_JOB4 | SKILL_2)//갓들리 실드
#define SKILL_GOD_BLESS		    (GROUP_KNIGHT | CHANGE_JOB4 | SKILL_3)//갓 블레스
#define SKILL_DIVINE_PIERCING	(GROUP_KNIGHT | CHANGE_JOB4 | SKILL_4)//디바인 피어싱




//////////////// 아틀란타 그룹 
#define SKILL_SHIELD_STRIKE		(GROUP_ATALANTA | CHANGE_JOB1 | SKILL_1)
#define SKILL_FARINA			(GROUP_ATALANTA | CHANGE_JOB1 | SKILL_2)
#define SKILL_THROWING_MASTERY	(GROUP_ATALANTA | CHANGE_JOB1 | SKILL_3)
#define SKILL_VIGOR_SPEAR		(GROUP_ATALANTA | CHANGE_JOB1 | SKILL_4)

#define SKILL_WINDY				(GROUP_ATALANTA | CHANGE_JOB2 | SKILL_1)
#define SKILL_TWIST_JAVELIN		(GROUP_ATALANTA | CHANGE_JOB2 | SKILL_2)
#define SKILL_SOUL_SUCKER		(GROUP_ATALANTA | CHANGE_JOB2 | SKILL_3)
#define SKILL_FIRE_JAVELIN		(GROUP_ATALANTA | CHANGE_JOB2 | SKILL_4)

#define SKILL_SPLIT_JAVELIN		(GROUP_ATALANTA | CHANGE_JOB3 | SKILL_1)
#define SKILL_TRIUMPH_OF_VALHALLA (GROUP_ATALANTA | CHANGE_JOB3 | SKILL_2)
#define SKILL_LIGHTNING_JAVELIN (GROUP_ATALANTA | CHANGE_JOB3 | SKILL_3)
#define SKILL_STORM_JAVELIN		(GROUP_ATALANTA | CHANGE_JOB3 | SKILL_4)

//4차 스킬
#define SKILL_HALL_OF_VALHALLA	(GROUP_ATALANTA | CHANGE_JOB4 | SKILL_1)//헬 오브 발할라
#define SKILL_X_RAGE			(GROUP_ATALANTA | CHANGE_JOB4 | SKILL_2)//엑스 라지
#define SKILL_FROST_JAVELIN		(GROUP_ATALANTA | CHANGE_JOB4 | SKILL_3)//프로스트 재블린
#define SKILL_VENGEANCE			(GROUP_ATALANTA | CHANGE_JOB4 | SKILL_4)//벤첸



//////////////// 프리스티스 
#define SKILL_HEALING			(GROUP_PRIESTESS | CHANGE_JOB1 | SKILL_1)
#define SKILL_HOLY_BOLT			(GROUP_PRIESTESS | CHANGE_JOB1 | SKILL_2)
#define SKILL_MULTISPARK		(GROUP_PRIESTESS | CHANGE_JOB1 | SKILL_3)
#define SKILL_HOLY_MIND			(GROUP_PRIESTESS | CHANGE_JOB1 | SKILL_4)

#define SKILL_MEDITATION		(GROUP_PRIESTESS | CHANGE_JOB2 | SKILL_1)
#define SKILL_DIVINE_LIGHTNING  (GROUP_PRIESTESS | CHANGE_JOB2 | SKILL_2)
#define SKILL_HOLY_REFLECTION	(GROUP_PRIESTESS | CHANGE_JOB2 | SKILL_3)
#define SKILL_GRAND_HEALING		(GROUP_PRIESTESS | CHANGE_JOB2 | SKILL_4)

#define SKILL_VIGOR_BALL		(GROUP_PRIESTESS | CHANGE_JOB3 | SKILL_1)
#define SKILL_RESURRECTION		(GROUP_PRIESTESS | CHANGE_JOB3 | SKILL_2)
#define SKILL_EXTINCTION		(GROUP_PRIESTESS | CHANGE_JOB3 | SKILL_3)
#define SKILL_VIRTUAL_LIFE		(GROUP_PRIESTESS | CHANGE_JOB3 | SKILL_4)

//4차 스킬 
#define SKILL_GLACIAL_SPIKE		 (GROUP_PRIESTESS | CHANGE_JOB4 | SKILL_1)//클래이셔 스파이크
#define SKILL_REGENERATION_FIELD (GROUP_PRIESTESS | CHANGE_JOB4 | SKILL_2)//리제너레이션 필드
#define SKILL_CHAIN_LIGHTNING	 (GROUP_PRIESTESS | CHANGE_JOB4 | SKILL_3)//체인 라이트닝
#define SKILL_SUMMON_MUSPELL	 (GROUP_PRIESTESS | CHANGE_JOB4 | SKILL_4)//서몬 뮤스펠



//////////////// 메지션 
#define SKILL_AGONY				(GROUP_MAGICIAN | CHANGE_JOB1 | SKILL_1)
#define SKILL_FIRE_BOLT			(GROUP_MAGICIAN | CHANGE_JOB1 | SKILL_2)
#define SKILL_ZENITH			(GROUP_MAGICIAN | CHANGE_JOB1 | SKILL_3)
#define SKILL_FIRE_BALL			(GROUP_MAGICIAN | CHANGE_JOB1 | SKILL_4)

#define SKILL_MENTAL_MASTERY	(GROUP_MAGICIAN | CHANGE_JOB2 | SKILL_1)
#define SKILL_WATORNADO         (GROUP_MAGICIAN | CHANGE_JOB2 | SKILL_2)
#define SKILL_ENCHANT_WEAPON	(GROUP_MAGICIAN | CHANGE_JOB2 | SKILL_3)
#define SKILL_DEAD_RAY			(GROUP_MAGICIAN | CHANGE_JOB2 | SKILL_4)

#define SKILL_ENERGY_SHIELD		(GROUP_MAGICIAN | CHANGE_JOB3 | SKILL_1)
#define SKILL_DIASTROPHISM		(GROUP_MAGICIAN | CHANGE_JOB3 | SKILL_2)
#define SKILL_SPIRIT_ELEMENTAL	(GROUP_MAGICIAN | CHANGE_JOB3 | SKILL_3)
#define SKILL_DANCING_SWORD		(GROUP_MAGICIAN | CHANGE_JOB3 | SKILL_4)

//4차스킬
#define SKILL_FIRE_ELEMENTAL	(GROUP_MAGICIAN | CHANGE_JOB4 | SKILL_1)//파이어 엘레멘탈
#define SKILL_FLAME_WAVE		(GROUP_MAGICIAN | CHANGE_JOB4 | SKILL_2)//플레임 웨이브
#define SKILL_DISTORTION		(GROUP_MAGICIAN | CHANGE_JOB4 | SKILL_3)//디스트록션
#define SKILL_M_METEO			(GROUP_MAGICIAN | CHANGE_JOB4 | SKILL_4)//메테오

/*----------------------------------------------------------------------------*
*								스킬 CODE (템스크론 )
*-----------------------------------------------------------------------------*/	

#define SKILL_NORMAL_ATTACK     0x11111111	//기본 공격 
////////////////매카니션 그룹 

#define SKILL_EXTREME_SHIELD	(GROUP_MECHANICIAN | CHANGE_JOB1 | SKILL_1)
#define SKILL_MECHANIC_BOMB		(GROUP_MECHANICIAN | CHANGE_JOB1 | SKILL_2)
#define SKILL_PHYSICAL_ABSORB	(GROUP_MECHANICIAN | CHANGE_JOB1 | SKILL_3)
#define SKILL_POISON_ATTRIBUTE	(GROUP_MECHANICIAN | CHANGE_JOB1 | SKILL_4)

#define SKILL_GREAT_SMASH		(GROUP_MECHANICIAN | CHANGE_JOB2 | SKILL_1)
#define SKILL_MAXIMIZE			(GROUP_MECHANICIAN | CHANGE_JOB2 | SKILL_2)
#define SKILL_AUTOMATION		(GROUP_MECHANICIAN | CHANGE_JOB2 | SKILL_3)
#define SKILL_SPARK				(GROUP_MECHANICIAN | CHANGE_JOB2 | SKILL_4)

#define SKILL_METAL_ARMOR		(GROUP_MECHANICIAN | CHANGE_JOB3 | SKILL_1)
#define SKILL_GRAND_SMASH		(GROUP_MECHANICIAN | CHANGE_JOB3 | SKILL_2)
#define SKILL_MECHANIC_WEAPON	(GROUP_MECHANICIAN | CHANGE_JOB3 | SKILL_3)
#define SKILL_SPARK_SHIELD		(GROUP_MECHANICIAN | CHANGE_JOB3 | SKILL_4)

//4차 스킬
#define SKILL_IMPULSION			(GROUP_MECHANICIAN | CHANGE_JOB4 | SKILL_1)//임플리션
#define SKILL_COMPULSION		(GROUP_MECHANICIAN | CHANGE_JOB4 | SKILL_2)//컴플리션
#define SKILL_MAGNETIC_SPHERE	(GROUP_MECHANICIAN | CHANGE_JOB4 | SKILL_3)//매네틱 피어
#define SKILL_METAL_GOLEM		(GROUP_MECHANICIAN | CHANGE_JOB4 | SKILL_4)//메탈 골렘

///////////////파이터 그룹 
#define SKILL_MELEE_MASTERY		(GROUP_FIGHTER | CHANGE_JOB1 | SKILL_1)
#define SKILL_FIRE_ATTRIBUTE	(GROUP_FIGHTER | CHANGE_JOB1 | SKILL_2)
#define SKILL_RAVING			(GROUP_FIGHTER | CHANGE_JOB1 | SKILL_3)
#define SKILL_IMPACT			(GROUP_FIGHTER | CHANGE_JOB1 | SKILL_4)

#define SKILL_TRIPLE_IMPACT		(GROUP_FIGHTER | CHANGE_JOB2 | SKILL_1)
#define SKILL_BRUTAL_SWING		(GROUP_FIGHTER | CHANGE_JOB2 | SKILL_2)
#define SKILL_ROAR				(GROUP_FIGHTER | CHANGE_JOB2 | SKILL_3)
#define SKILL_RAGE_OF_ZECRAM	(GROUP_FIGHTER | CHANGE_JOB2 | SKILL_4)

#define SKILL_CONCENTRATION		(GROUP_FIGHTER | CHANGE_JOB3 | SKILL_1)
#define SKILL_AVANGING_CRASH	(GROUP_FIGHTER | CHANGE_JOB3 | SKILL_2)
#define SKILL_SWIFT_AXE			(GROUP_FIGHTER | CHANGE_JOB3 | SKILL_3)
#define SKILL_BONE_CRASH		(GROUP_FIGHTER | CHANGE_JOB3 | SKILL_4)

//4차 스킬
#define SKILL_DETORYER			(GROUP_FIGHTER | CHANGE_JOB4 | SKILL_1)//디스토리어
#define SKILL_BERSERKER			(GROUP_FIGHTER | CHANGE_JOB4 | SKILL_2)//버서커
#define SKILL_CYCLONE_STRIKE	(GROUP_FIGHTER | CHANGE_JOB4 | SKILL_3)//싸이클론 스트라이크
#define SKILL_BOOST_HEALTH		(GROUP_FIGHTER | CHANGE_JOB4 | SKILL_4)//부스트 헬스

////////////////파이크맨 그룹 

#define SKILL_PIKE_WIND  		(GROUP_PIKEMAN | CHANGE_JOB1 | SKILL_1)
#define SKILL_ICE_ATTRIBUTE 	(GROUP_PIKEMAN | CHANGE_JOB1 | SKILL_2)
#define SKILL_CRITICAL_HIT      (GROUP_PIKEMAN | CHANGE_JOB1 | SKILL_3)
#define SKILL_JUMPING_CRASH     (GROUP_PIKEMAN | CHANGE_JOB1 | SKILL_4)

#define SKILL_GROUND_PIKE    	(GROUP_PIKEMAN | CHANGE_JOB2 | SKILL_1)
#define SKILL_TORNADO     		(GROUP_PIKEMAN | CHANGE_JOB2 | SKILL_2)
#define SKILL_WEAPONE_DEFENCE_MASTERY      		(GROUP_PIKEMAN | CHANGE_JOB2 | SKILL_3)
#define SKILL_EXPANSION     	(GROUP_PIKEMAN | CHANGE_JOB2 | SKILL_4)

#define SKILL_VENOM_SPEAR    	(GROUP_PIKEMAN | CHANGE_JOB3 | SKILL_1)
#define SKILL_VANISH			(GROUP_PIKEMAN | CHANGE_JOB3 | SKILL_2)
#define SKILL_CRITICAL_MASTERY 	(GROUP_PIKEMAN | CHANGE_JOB3 | SKILL_3)
#define SKILL_CHAIN_LANCE	 	(GROUP_PIKEMAN | CHANGE_JOB3 | SKILL_4)

//4차스킬
#define SKILL_ASSASSIN_EYE    	(GROUP_PIKEMAN | CHANGE_JOB4 | SKILL_1)//어쌔신 아이
#define SKILL_CHARGING_STRIKE	(GROUP_PIKEMAN | CHANGE_JOB4 | SKILL_2)//차징 스트라이크
#define SKILL_VAGUE			 	(GROUP_PIKEMAN | CHANGE_JOB4 | SKILL_3)//베이그
#define SKILL_SHADOW_MASTER	 	(GROUP_PIKEMAN | CHANGE_JOB4 | SKILL_4)//새됴우 마스터


////////////////아쳐  그룹 

#define SKILL_SCOUT_HAWK  		(GROUP_ARCHER | CHANGE_JOB1 | SKILL_1)
#define SKILL_SHOOTING_MASTERY  (GROUP_ARCHER | CHANGE_JOB1 | SKILL_2)
#define SKILL_WIND_ARROW		(GROUP_ARCHER | CHANGE_JOB1 | SKILL_3)
#define SKILL_PERFECT_AIM		(GROUP_ARCHER | CHANGE_JOB1 | SKILL_4)

#define SKILL_DIONS_EYE  		(GROUP_ARCHER | CHANGE_JOB2 | SKILL_1)
#define SKILL_FALCON      		(GROUP_ARCHER | CHANGE_JOB2 | SKILL_2)
#define SKILL_ARROW_OF_RAGE		(GROUP_ARCHER | CHANGE_JOB2 | SKILL_3)
#define SKILL_AVALANCHE       	(GROUP_ARCHER | CHANGE_JOB2 | SKILL_4)

#define SKILL_ELEMENTAL_SHOT	(GROUP_ARCHER | CHANGE_JOB3 | SKILL_1)
#define SKILL_GOLDEN_FALCON		(GROUP_ARCHER | CHANGE_JOB3 | SKILL_2)
#define SKILL_BOMB_SHOT    		(GROUP_ARCHER | CHANGE_JOB3 | SKILL_3)
#define SKILL_PERFORATION   	(GROUP_ARCHER | CHANGE_JOB3 | SKILL_4)

//4차 스킬
#define SKILL_RECALL_WOLVERIN	(GROUP_ARCHER | CHANGE_JOB4 | SKILL_1)//리콜 울버린
#define SKILL_EVASION_MASTERY	(GROUP_ARCHER | CHANGE_JOB4 | SKILL_2)//이베이션 마스터리
#define SKILL_PHOENIX_SHOT   	(GROUP_ARCHER | CHANGE_JOB4 | SKILL_3)//피닉스 샷
#define SKILL_FORCE_OF_NATURE  	(GROUP_ARCHER | CHANGE_JOB4 | SKILL_4)//포스 오브 내츄

/*----------------------------------------------------------------------------*
*	Tier 5 skills (the fifth rank, ChangeJob 4), learned at levels 80/83/86/90
*	with the fourth tier's points; the skill window's second page shows them.
*-----------------------------------------------------------------------------*/
#define SKILL_LAND_MINING		(GROUP_MECHANICIAN | CHANGE_JOB5 | SKILL_1)
#define SKILL_HYPER_SONIC		(GROUP_MECHANICIAN | CHANGE_JOB5 | SKILL_2)
#define SKILL_ROLLING_SMASH		(GROUP_MECHANICIAN | CHANGE_JOB5 | SKILL_3)
#define SKILL_POWER_ENHANCEMENT	(GROUP_MECHANICIAN | CHANGE_JOB5 | SKILL_4)

#define SKILL_DOWN_HIT			(GROUP_FIGHTER | CHANGE_JOB5 | SKILL_1)
#define SKILL_POWER_DASH		(GROUP_FIGHTER | CHANGE_JOB5 | SKILL_2)
#define SKILL_MORTAL_BLOW		(GROUP_FIGHTER | CHANGE_JOB5 | SKILL_3)
#define SKILL_BLOODY_BERSERKER	(GROUP_FIGHTER | CHANGE_JOB5 | SKILL_4)

#define SKILL_DANCING_REAPER	(GROUP_PIKEMAN | CHANGE_JOB5 | SKILL_1)
#define SKILL_FINAL_SPEAR		(GROUP_PIKEMAN | CHANGE_JOB5 | SKILL_2)
#define SKILL_AMPLIFIED			(GROUP_PIKEMAN | CHANGE_JOB5 | SKILL_3)
#define SKILL_SIDE_SPIN_ATTACK	(GROUP_PIKEMAN | CHANGE_JOB5 | SKILL_4)

#define SKILL_EVADE_SHOT		(GROUP_ARCHER | CHANGE_JOB5 | SKILL_1)
#define SKILL_THUNDER_ROPE		(GROUP_ARCHER | CHANGE_JOB5 | SKILL_2)
#define SKILL_NATURE_SPLASH		(GROUP_ARCHER | CHANGE_JOB5 | SKILL_3)
#define SKILL_CIRCLE_TRAP		(GROUP_ARCHER | CHANGE_JOB5 | SKILL_4)

#define SKILL_SOUL_BREAKER		(GROUP_KNIGHT | CHANGE_JOB5 | SKILL_1)
#define SKILL_CRESCENT_MOON		(GROUP_KNIGHT | CHANGE_JOB5 | SKILL_2)
#define SKILL_SAINT_BLADE		(GROUP_KNIGHT | CHANGE_JOB5 | SKILL_3)
#define SKILL_HOLY_BENEDIC		(GROUP_KNIGHT | CHANGE_JOB5 | SKILL_4)

#define SKILL_TALARIA			(GROUP_ATALANTA | CHANGE_JOB5 | SKILL_1)
#define SKILL_GALAXY_COUP		(GROUP_ATALANTA | CHANGE_JOB5 | SKILL_2)
#define SKILL_SUMMON_ARCUDA		(GROUP_ATALANTA | CHANGE_JOB5 | SKILL_3)
#define SKILL_SNIPPY_FEAR		(GROUP_ATALANTA | CHANGE_JOB5 | SKILL_4)

#define SKILL_SPIRIT_IMPACT		(GROUP_PRIESTESS | CHANGE_JOB5 | SKILL_1)
#define SKILL_PIERCING_ICE		(GROUP_PRIESTESS | CHANGE_JOB5 | SKILL_2)
#define SKILL_SUMMON_RAMIEL		(GROUP_PRIESTESS | CHANGE_JOB5 | SKILL_3)
#define SKILL_BLESSING_KRISHNA	(GROUP_PRIESTESS | CHANGE_JOB5 | SKILL_4)

#define SKILL_SILRAPHIM			(GROUP_MAGICIAN | CHANGE_JOB5 | SKILL_1)
#define SKILL_VIS_TENUS			(GROUP_MAGICIAN | CHANGE_JOB5 | SKILL_2)
#define SKILL_PRIMA_IGNIS		(GROUP_MAGICIAN | CHANGE_JOB5 | SKILL_3)
#define SKILL_THIRD_ANIMA		(GROUP_MAGICIAN | CHANGE_JOB5 | SKILL_4)



///////////////////////////////
/*----------------------------------------------------------------------------*
*							그외의 스킬
*-----------------------------------------------------------------------------*/	
#define SKILL_FORCE_ORB			(GROUP_OTHERSKILL | CHANGE_JOB1 | SKILL_1)

//스킬은 아니지만 유지형 아이콘을 표시하기위해서
#define CHANGE_JOB3_QUEST		(GROUP_OTHERSKILL | CHANGE_JOB1 | SKILL_2)
#define CHANGE_ELEMENTARY_QUEST	(GROUP_OTHERSKILL | CHANGE_JOB1 | SKILL_3)



/*----------------------------------------------------------------------------*
*							공성전 클랜  스킬
*-----------------------------------------------------------------------------*/	
#define CLANSKILL_ABSORB        (GROUP_OTHERSKILL | CHANGE_JOB1 | SKILL_4)
#define CLANSKILL_ATTACK        (GROUP_OTHERSKILL | CHANGE_JOB1 | SKILL_5)
#define CLANSKILL_EVASION       (GROUP_OTHERSKILL | CHANGE_JOB1 | SKILL_6)

// 박재원 - 부스터 아이템(생명력, 기력, 근력)
#define BOOSTER_ITEM_LIFE		(GROUP_OTHERSKILL | CHANGE_JOB1 | SKILL_7)
#define BOOSTER_ITEM_MANA		(GROUP_OTHERSKILL | CHANGE_JOB1 | SKILL_8)
#define BOOSTER_ITEM_STAMINA	(GROUP_OTHERSKILL | CHANGE_JOB1 | SKILL_9)

// 장별 - 스킬 딜레이
#define SKILLDELAY_ITEM_LIFE		(GROUP_OTHERSKILL | CHANGE_JOB2 | SKILL_1)


/*----------------------------------------------------------------------------*
*							공성전 아이템 사용 스킬
*-----------------------------------------------------------------------------*/	
#define SCROLL_INVULNERABILITY	(GROUP_OTHERSKILL | CHANGE_JOB2 | SKILL_1)
#define SCROLL_CRITICAL			(GROUP_OTHERSKILL | CHANGE_JOB2 | SKILL_2)
#define SCROLL_EVASION			(GROUP_OTHERSKILL | CHANGE_JOB2 | SKILL_3)

#define STONE_R_FIRECRYTAL      (GROUP_OTHERSKILL | CHANGE_JOB2 | SKILL_4)
#define STONE_R_ICECRYTAL       (GROUP_OTHERSKILL | CHANGE_JOB2 | SKILL_5)
#define STONE_R_LINGHTINGCRYTAL (GROUP_OTHERSKILL | CHANGE_JOB2 | SKILL_6)

#define STONE_A_FIGHTER			(GROUP_OTHERSKILL | CHANGE_JOB3 | SKILL_1)
#define STONE_A_MECHANICIAN		(GROUP_OTHERSKILL | CHANGE_JOB3 | SKILL_2)
#define STONE_A_PIKEMAN			(GROUP_OTHERSKILL | CHANGE_JOB3 | SKILL_3)
#define STONE_A_ARCHER			(GROUP_OTHERSKILL | CHANGE_JOB3 | SKILL_4)

#define STONE_A_KNIGHT			(GROUP_OTHERSKILL | CHANGE_JOB3 | SKILL_5)
#define STONE_A_ATALANTA		(GROUP_OTHERSKILL | CHANGE_JOB3 | SKILL_6)
#define STONE_A_MAGICIAN		(GROUP_OTHERSKILL | CHANGE_JOB3 | SKILL_7)
#define STONE_A_PRIESTESS		(GROUP_OTHERSKILL | CHANGE_JOB3 | SKILL_8)

#define SCROLL_P_INVULNERABILITY    (GROUP_OTHERSKILL | CHANGE_JOB4 | SKILL_1)
#define SCROLL_P_CRITICAL		    (GROUP_OTHERSKILL | CHANGE_JOB4 | SKILL_2)
#define SCROLL_P_EVASION		    (GROUP_OTHERSKILL | CHANGE_JOB4 | SKILL_3)

/*----------------------------------------------------------------------------*
*								스킬 구조체 
*-----------------------------------------------------------------------------*/
//스킬 세부정보 구조체 

struct sSKILL_INFO{
	char SkillName[64];			//스킬 이름 
	char SkillDoc[150];			//스킬 설명 
	int  RequireLevel;			//사용할수 있는 레벨 
	int  UseStamina[2];			//0 사용되는 근력 1 증가치 
	int	 RequireMastery[2];	    //요구 숙련도 최소 최대 	
	int  Element[3];			//스킬 원소 속성 
	DWORD UseWeaponCode[8];		//스킬사용가능한 무기 최대8
	int (*FuncPointer)();       //(sUSESKILL *pSkill);	//함수 포인터  
	DWORD	CODE;				//코드 
	DWORD   USECODE;			//사용가능한 위치 
	int *UseMana;				//마나사용 포인터 
	
	///////////위에 꺼는 파일에서 읽어와 셋팅 /////////////// 건들지 마시오!!!
	int     SkillNum;			//몇번째 스킬인지를 저장한다 
	

};

//////////////스킬 박스 구조체 
struct sSKILLBOX{ 
	RECT BoxRect;  //스킬 박스의 좌표 
	RECT GageRect;  //스킬 박스의 좌표 

};	

/////////////스킬 테이블 구조체 
struct sSKILL{  
	char	sSkillName[32];			//스킬 이름 
	DWORD	CODE;					//스킬 코드 
	char    FileName[32];			//스킬 파일 이름 
	char    IconName[32];			//유지형 스킬파일 
	int     Flag;					//사용가능 플랙 
	int     Use;					//포인트 할당가능 플랙 
	int     Point;					//스킬에 할당된 포인트 
	int     ShortKey;				//펑션 키 
	int     MousePosi;				//스킬 마우스 포지션 
	int     Position;				
	int     UseTime;				//사용 시간 
	int     CheckTime;				//사용시간을 체크한다 
	int     GageLength;				//마스터리 게이지 길이   /* 딜레이 값*/
	int     GageLength2;			//마스터리 게이지 길이   /* 딜레이 값*/
	int     Mastery;				//마스터리 증가 수치 
	int     UseSKillIncreCount;		//스킬 사용시 카운트 증가 
	int     UseSkillCount;			//스킬사용 수치 
	int 	UseSkillMastery;		//사용시 올라가는 수치 
	int     UseSkillMasteryGage;	//사용시 올라가는 게이지 
	int     UseSkillFlag;			//1은 사용 0의 사용불가 
	int     PlusState[5];			//플러스 
	int		Time[2];				//0 스킬 시작타임 1 맥스타임
	int		PartyFlag;				//파티원에적용되는 플렉
	int     ElementIndex;			//어떤 속성이 적용중인다 
	int     MatIcon;
	
	//유지형 아이콘 부분
	int     IconAlpha;
	int     IconPosiX;
	int     IconTime;
	int     IconFlag;

	DWORD   SkillTaget_CODE;  //0 자기자신 아닌경우 다른캐릭
	int     SummonFlag;

	sSKILL_INFO Skill_Info;
};

/////////////현재 사용되고있는 스킬 구조체 
struct sUSESKILL{
	sSKILL UseSkill[SIN_MAX_USE_SKILL]; //사용할수있는 스킬 
	sSKILLBOX SkillBox[SIN_MAX_USE_SKILL];
	sSKILL *pLeftSkill;		//마우스 왼쪽 버튼에 할당된 스킬				
	sSKILL *pRightSkill;	//마우스 오른쪽에 할당된 스킬 
	int   SkillPoint;
	int   SkillPoint4;

};

struct sPASSIVE_INFO{
	int Flag; //사용여부 
	float DamagePercent;		//데미지 증가 
	DWORD UseWeaponCode[8];	//사용 가능 무기 
	DWORD CODE;


};

///////////// 패시브로 저장되는 캐릭터 능력치 
struct sSKILLPASSIVE{
	sPASSIVE_INFO sPassive_Info[10];
	int Resistance[8];

};


/*----------------------------------------------------------------------------*
*								스킬 클래스 
*-----------------------------------------------------------------------------*/
class cSKILL{

public:

	int OpenFlag;		//현재 인벤토리가 열려있나 아닌가에 대한 플랙 
	
	int MatSkill[4];	//스킬 메인 
	DWORD UseSkillBox;    //현재 사용하는 스킬 박스 

	int CheckMasteryData[SIN_MAX_USE_SKILL]; //0번은 사용되고있지않다
	int CheckSkillPointData[SIN_MAX_USE_SKILL]; //0번은 사용되고있지않다
	int CheckSkillMasteyData[SIN_MAX_USE_SKILL]; //0번은 사용되고있지않다

	LPDIRECTDRAWSURFACE4	lpSkillGage[2];		//0은 통짜 1은 꼭따리 
	LPDIRECTDRAWSURFACE4	lpSkillGage2[2];	//0은 통짜 1은 꼭따리 
	LPDIRECTDRAWSURFACE4	lpSkillGage3;		//
	LPDIRECTDRAWSURFACE4	lpSkillGage4;		//
	LPDIRECTDRAWSURFACE4	lpSkillGage5;		//
	LPDIRECTDRAWSURFACE4	lpSkillTitle[5];	//직업 타이틀 (5: the fifth rank's, on the second page)
	LPDIRECTDRAWSURFACE4    lpSkillButton_Gray[SIN_MAX_USE_SKILL];	//버튼 비활성화 이미지
	LPDIRECTDRAWSURFACE4    lpSkillButton[SIN_MAX_USE_SKILL];	//버튼 이미지
	LPDIRECTDRAWSURFACE4    lpSelectSkill;		//버튼 선택 
	LPDIRECTDRAWSURFACE4    lpShortKey[8];		//펑션키 
	LPDIRECTDRAWSURFACE4    lpSkillBoxBack;		//스킬 이미지없을때 배경 
	LPDIRECTDRAWSURFACE4    lpWeaponIcon[10];	//무기 아이콘 
	LPDIRECTDRAWSURFACE4    lpMasteryInfo;		//숙련도 표시 박스 
	LPDIRECTDRAWSURFACE4    lpSkillDown;        //스킬 다운 버튼표시

public:
	cSKILL();
	~cSKILL();

	void Init();
	void Load();
	void Release();
	void Close();
	void Draw();
	void Main();
	void LButtonDown(int x , int y);
	void LButtonUp(int x , int y);
	void RButtonDown(int x , int y);
	void RButtonUp(int x, int y);
	void KeyDown(); 
	void LoadUseSkillImage();	//사용될 스킬 이미지를 로드한다 

	int SearchUseSkill();		//사용할 스킬창을 찾는다 
	
	void DrawSkillText();		//텍스트를 찍어준다 	

	///////////////////// 스킬 세부사항 함수 
	void CheckingNowSkillState(); //현재의 스킬상태를 체크한다 

	//마스터리를 체크한다 
	void CheckSkillMastery();

	//스킬의 정보를 보여준다 
	int ShowSkillInfo(sSKILL *pSkill , sSKILLBOX *pSkillBox , int Icon = 0);

	//박스의 크기와 사이즈를 구한다 
	int GetInfoBoxSize(sSKILLBOX *pSkillBox, int Col , int Icon = 0);

	//스킬의 숙련도를 표시한다 
	int GetSkillMastery(sSKILL *pSkill , sSKILLBOX *pSkillBox);

	//스킬을 사용할수있는 지를 체크한다  
	int CheckUseSkillState(sSKILL *pSkill);

	///////////해킹에 대비해서 스킬 데이타를 체크하다 
	int CheckSkillData();

	//스킬포이트 초기화 
	int InitSkillPoint(int kind=0);


	//////////////////////////////// 스킬 포인트 초기화 함수 ///////
	int SearchClearSkillPoint(DWORD CODE); //초기화 될 스킬 포인트가 있는지 찾는다 
 
	int ShowClearSkillPointButton(int x , int y); //초기화 버튼을 보여준다 

	int ClearSkillPointSection(); //스킬 포인트를 초기화한다 

	//캐릭터별 SkillInfo를 체크한다
	int ReFormCharSkillInfo();
	int CheckCharSkillInfo();

	//Skill 마스터리관련 정보를 체크한다
	//int CheckSkillMasteryForm(int Index);
	int CheckSkillMasteryForm(sSKILL *pSkill=0 , int Index=0);
	int ReformSkillMasteryForm(sSKILL *pSkill=0 , int Index=0);

	//스킬 포인트를 체크한다 
	int CheckSkillPointForm(sSKILL *pSkill);
	int ReformSkillPointForm(sSKILL *pSkill);
	int InitReformSkillPointForm(); //처음포인트를 저장해놓는다


	//스킬 포인트를 구해온다
	int GetSkillPoint(DWORD CODE);

	//현재사용중인 유지형 스킬을 찾아서 포인트를 리턴한다
	int SearchContiueSkill(DWORD CODE);

	//버추얼라이프 셋팅함수
	int SetVirtualLife(int Time , int Point);

	// 박재원 - 부스터 아이템
	int UseBoosterItem(sITEM *pItem);

	// 박재원 - 부스터 아이템
	int SetBoosterItem(DWORD ItemCODE , int Time);

	//트라이엄프 오브 벨헬라
	int SetT_Of_Valhalla( int Point , int LV);

	int SetEnchant_Weapon(int Time , int Point);

	//지속 스킬체크 
	int CheckContinueIcon();

	//원그리기 
	int SkillBarDraw(sSKILL *pSkill , int x , int y , int Alpha);

	//스킬 내용을 찾는다
	int SearchSkillDoc(sSKILL *pSkill);

	//지속스킬을 취소한다
	int CancelContinueSkill(DWORD CODE);

	//윗단 Draw를 분리한다
	void DrawUp();

	//스킬 숏키초기화
	int SkillShortKeyInit();

	//리제러레이션 필드
	int Regeneration_Field(int Time , int Point , int Party=0 );

	//포스오브 네이쳐
	int ForeceOfNature(int Time , int Point , int Party=0 );

	//헬오브 발할라
	int HellOfValhalla(int Time , int Point,int TVPoint , int LV , int Party=0);

	//사용중인 코드의 스킬 인덱스를찾아 리턴한다
	int SearchContiueSkillIndex(DWORD CODE);

	//사용중인 코드를 리턴한다.
	int SearchContiueSkillCODE(DWORD CODE);

	// 박재원 - 스킬 마스터(전직별로 스킬의 숙련 여부를 체크한다.)
	int CheckMaturedSkill(int ChangeJob);

	// 장별 - 스킬 딜레이
	int UseSkillDelayItem(sITEM *pItem);
	int SetSkillDelayItem(DWORD ItemCODE , int Time);
	
	

};

extern cSKILL	cSkill;
extern sUSESKILL sinSkill;
extern sSKILL_INFO sSkill_Info[SIN_MAX_SKILL];
extern sSKILLBOX sLeftRightSkill[2];
extern sSKILL  *pUseSkill; //사용하기전에 값을 셋팅해준다 

extern sSKILL  ContinueSkill[10];	//지속적으로 사용되는 스킬 
extern sSKILLPASSIVE sSkillPassive; //캐릭터에 저장되는 패시브 스킬 

extern sSKILL sSkill[SIN_MAX_SKILL];

extern int SelectInterSkill;

////////스킬 정보 보여주기 관련 
extern int     SkillInfoShowFlag;
extern POINT	SkillInfoBoxPosi;
extern POINT   SkillInfoTextPosi;
extern POINT   SkillBoxSize;
extern char    szSkillInfoBuff[5000];

extern int     SkillDocLineCnt[6];
extern int     NextSkillLevelLine;


////////스킬 습득 
extern sSKILL *pMasterSkill;  //스킬 습득 포인터 
extern int  SkillMasterFlag;  //1 스킬 2 전업 3 종료   

extern int ShowSkillUpInfo; //메세지 박스와 함께 스킬정보를 보여준다 

extern int  SkillMasterOkFlag[2];

extern JOB_DATA_CODE *sinJobList; //직업 리스트

///////스킬 아이콘 
extern int   SkillIconIndex[10];
extern POINT SkillIconPosi;
extern int   SkillInconCnt;
 
//////TEST 딜레이 타임 
extern int TestSkillDelayTime[16];

///// 
extern int ChangeJobSkillPlus;

extern POINT SkillUpInfo;
extern POINT SkillUpInfoPosi;

extern int ShowSkillClose;

extern DWORD ClearSkillCODE; //초기화될 스킬이 있나없나 

extern int ContinueSkillCancelFlag;

extern int SetT_Of_ValhallaFlag;
extern int SetT_Of_ValhallaFlag2;
extern int SetT_Of_ValhallaLV;    //시전자의 레벨
extern int SetH_Of_ValhallaLV;
extern int SetT_Of_ValhallaPOINT;


//버추얼 라이프
extern int AddVirtualLife[2];
extern int sinVirtualLifeTime;
extern int sinVirtualLifePercent;

// 박재원 - 부스터 아이템(생명력)
extern int AddBoosterLife;
extern int BoosterLifeTime;
extern int BoosterLifePercent;
extern int LifeFlag;
extern short tempLife[];

// 박재원 - 부스터 아이템(기력)
extern int AddVirtualMana[2];
extern int sinVirtualManaTime;
extern int sinVirtualManaPercent;
extern int ManaFlag;
extern short tempMana[];

// 박재원 - 부스터 아이템(근력)
extern int AddVirtualStamina[2];
extern int sinVirtualStaminaTime;
extern int sinVirtualStaminaPercent;
extern int StaminaFlag;
extern short tempStamina[];

// 박재원 - 부스터 아이템
extern int Mat_BoosterItem[];

extern int MatForceOrb[];
extern int MatQuestTimer;

// 박재원 - 빌링 매직 포스
extern int MatBillingMagicForceOrb[];

extern int SkillButtonIndex;
extern int Skill4ButtonIndex;


//유지형스킬 마우스아이콘인덱스
extern int ContinueSkillMouseIndex;
extern sSKILL  TempContinueSkill[10]; 

// 장별 - 스킬 딜레이
extern int nMat_SkillDelayItem[];
extern int sinAddSkillDelay;
extern int sinSkillDelayTime; 
extern int SkillDelayFlag;