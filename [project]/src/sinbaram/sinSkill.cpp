/*----------------------------------------------------------------------------*
*	파일명 :  sinSkill.cpp
*	하는일 :  아이템 관리 
*	작성일 :  최종업데이트 12월 
*	적성자 :  박상열 
*-----------------------------------------------------------------------------*/	

#include "sinLinkHeader.h"
#include "..\\skillsub.h"
#include "..\\field.h" // 박재원 - 부스터 아이템 사용장소를 설정하기 위해
/*----------------------------------------------------------------------------*
*								전역변수 			
*-----------------------------------------------------------------------------*/	

cSKILL	cSkill;
sUSESKILL sinSkill;
sSKILL  *pUseSkill;			//사용하기전에 값을 셋팅해준다 
sSKILL  ContinueSkill[10];	//지속적으로 사용되는 스킬 
sSKILL  TempContinueSkill[10];	//지속적으로 사용되는 스킬 
int SkillCountTime2[SIN_MAX_USE_SKILL] = {0};
int ContinueSkillMouseIndex = 0;

JOB_DATA_CODE *sinJobList;

sSKILL *pMasterSkill;  //스킬 습득 포인터 

sSKILLPASSIVE sSkillPassive; //캐릭터에 저장되는 패시브 스킬 

/////////////
int ShowSkillUpInfo = 0;     //메세지 박스와 함께 스킬정보를 보여준다 
int  SkillMasterOkFlag[2] = {0,0};
int  SkillMasterFlag = 0;    //NPC한테 스킬 을 배우고자 한다 

int SkillMasteryShowFlag = 0;		//숙련도를 표시한다 
char szUseSkillMastery[32];			//스킬 숙련도 
POINT UseSkillMasteryPosi = {0,0};	//숙련도 표시 포지션 

int MatForceOrb[12] = {0};

// 박재원 - 빌링 매직 포스
int MatBillingMagicForceOrb[3] = {0};

// 박재원 - 부스터 아이템
int Mat_BoosterItem[9] = {0};

// 장별 - 스킬 딜레이
int nMat_SkillDelayItem[3] = {0};

int MatQuestTimer = 0;

/////////////스킬 정보  보여주기 

POINT SkillUpInfo = { 13,21};       //스킬 정보 보여주기 
POINT SkillUpInfoPosi= { 90,100};

////////////
int     SkillInfoLineCnt = 0;

int     SkillInfoShowFlag = 0;
POINT	SkillInfoBoxPosi  = {0,0};
POINT   SkillInfoTextPosi = {0,0};
POINT   SkillBoxSize = {0,0};


char    szSkillInfoBuff[5000];
char    szSkillMasterInfoBuff[5000];

char    szSkillInfoBuff2[5000];
char    szSkillMasterInfoBuff2[5000];


int     SkillDocLineCnt[6] = {0,0,0,0,0,0};
int     SkillUseWeaponClass = 0;
int     NextSkillLevelLine = 0;
/////////////

int ShowSkillClose = 0; //닫기 버튼 플랙 
int SkillButtonIndex = 0;
int Skill4ButtonIndex = 0;

int DownButtonUseShorKey = 0 ; //단축키를 설정했을때는 스킬을 정션하지 않는다 

int DownButtonIndex  = 0;  //버튼을 누르는 효과를 주기위한 인덱스 
int LDownButtonIndex = 0;  //왼쪽 버튼 
int RDownButtonIndex = 0;  //오른쪽 버튼 

DWORD sinSkillBoxColor = RGBA_MAKE( 0,0,132,128 ); 

int ChangeJobSkillPlus = 0;
int UseSkillIndex[2] = {0,0}; //왼쪽 오른쪽 

int SelectInterSkill = 0;

/////////////////미니 Define
#define	NORMAL_SKILL_INDEX		14
#define MAX_USE_SKILL_POINT		10

#define SIN_MOUSE_POSI_LEFT		1
#define SIN_MOUSE_POSI_RIGHT	2

/////////////////

///////////////// 스킬 아이콘 
int   SkillIconIndex[10] = {0,0,0,0,0,0,0,0,0,0};
POINT SkillIconPosi = {0,0};
int   SkillInconCnt = 0;


int GageLenght[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

DWORD ClearSkillCODE = 0; //초기화될 스킬이 있나없나 
POINT ClearSkillButtonPosi = {0,0}; //스킬 초기화 버튼 위치 

int SetT_Of_ValhallaFlag = 0;
int SetT_Of_ValhallaFlag2 = 0; //순서때문에 플렉하나를 더둔다
int SetT_Of_ValhallaLV = 0;    //시전자의 레벨

int SetH_Of_ValhallaLV = 0;    //시전자의 레벨
int SetT_Of_ValhallaPOINT = 0; //시전자의 트라이엄프 오브 발할라포인트
 
int MatCircleIcon = 0;

sSKILL_INFO sSkill_Info[SIN_MAX_SKILL] = {

#include "sinSkill_Info.h"

};

//스킬 세부사항 
sSKILL sSkill[SIN_MAX_SKILL] = {
	
	//스킬 이름            스킬 코드          요구레벨   버튼 파일네임 
	{"\263\353\270\326\276\356\305\303 "		,SKILL_NORMAL_ATTACK	,"Skill_Normal",},	 		

	//////////////메카닉 계열
	{"\300\315\275\272\306\256\270\262\275\257\265\345"		,SKILL_EXTREME_SHIELD	,"TM10 Ex_shield","Mc1_E_SHIELD.tga"},	 	
	{"\270\336\304\253\264\320\271\374"			,SKILL_MECHANIC_BOMB	,"TM12 M_bomb"},
	{"\306\367\300\314\301\360 \276\356\306\256\270\256\272\344\306\256",SKILL_POISON_ATTRIBUTE ,"TM14 P_attribute"},
	{"\307\307\301\366\304\303 \276\367\274\331"		,SKILL_PHYSICAL_ABSORB	,"TM17 Ph_absorb","Mc1_P_ABSORB.tga"},

	{"\266\363\277\356\265\345 \300\323\306\345\306\256"	,SKILL_GREAT_SMASH		,"TM20 G_Smash"},
	{"\270\306\275\303\270\266\300\314\301\356 "		,SKILL_MAXIMIZE			,"TM23 Maximize","Mc2_MAXIMIZE.tga"},
	{"\277\300\305\344\270\336\300\314\274\307"		,SKILL_AUTOMATION		,"TM26 Automation","Mc2_AUTOMATION.tga"},
	{"\275\272\306\304\305\251"			,SKILL_SPARK			,"TM30 Spark"},

	{"\270\336\305\273 \276\306\270\323"		,SKILL_METAL_ARMOR		,"TM40 M_Armor","Mc3_M_ARMOR.tga"},
	{"\261\327\267\243\265\345 \275\272\270\336\275\254"	,SKILL_GRAND_SMASH		,"TM43 Grand_Smash"},
	{"\270\336\304\253\264\320 \277\376\306\335"		,SKILL_MECHANIC_WEAPON	,"TM44 M_Weapon"},
	{"\275\272\306\304\306\256 \275\257\265\345"		,SKILL_SPARK_SHIELD		,"TM50 S_Shield","Mc3_S_SHIELD.tga"},

	//4차 스킬
	{"\300\323\307\303\267\271\300\314\274\307"		,SKILL_IMPULSION		,"TM60 Impulsion"},
	{"\304\304\307\303\267\271\300\314\274\307"		,SKILL_COMPULSION		,"TM63 Compulsion","Mc4_COMPULSION.tga"},
	{"\270\336\263\327\306\275 \307\307\276\356"		,SKILL_MAGNETIC_SPHERE	,"TM66 M_Sphere","Mc4_M_SPHERE.tga"},
	{"\270\336\305\273 \260\361\267\275"		,SKILL_METAL_GOLEM		,"TM70 M_Golem","Mc4_M_GOLEM.tga"},

	////////////// 파이터 계열
	{"\271\350\306\262 \270\266\275\272\305\315\270\256"	,SKILL_MELEE_MASTERY	,"TF10 M_mastery"},
	{"\306\304\300\314\305\315 \276\356\306\256\270\256\272\344\306\256",SKILL_FIRE_ATTRIBUTE	,"TF12 F_attribute"},
	{"\267\271\300\314\272\371"			,SKILL_RAVING			,"TF14 raving"},
	{"\300\323\306\321\306\256"			,SKILL_IMPACT			,"TF17 impact"},

	{"\306\256\270\256\307\303 \300\323\306\321\306\256"	,SKILL_TRIPLE_IMPACT	,"TF20 T_Impact"},
	{"\272\316\267\347\306\262 \275\272\300\256"		,SKILL_BRUTAL_SWING		,"TF23 B_Swing"},
	{"\267\316\276\356"				,SKILL_ROAR				,"TF26 Roar"},
	{"\267\271\300\314\301\366\277\300\272\352 \301\246\305\251\266\367",SKILL_RAGE_OF_ZECRAM	,"TF30 R_Zecram"},

	{"\304\301\274\276\306\256\267\271\300\314\274\307"		,SKILL_CONCENTRATION	,"TF40 Concentration","Ft3_CONCENTRATION.tga"},
	{"\276\356\272\245\302\241 \305\251\267\241\275\254"	,SKILL_AVANGING_CRASH	,"TF43 A_Crash"},
	{"\275\272\300\247\307\301\277\242\275\272"		,SKILL_SWIFT_AXE		,"TF46 S_Axe","Ft3_S_AXE.tga"},
	{"\272\273 \305\251\267\241\275\254"		,SKILL_BONE_CRASH		,"TF50 B_Crash"},

	//4차 스킬
	{"\265\360\275\272\305\344\270\256\276\356"		,SKILL_DETORYER		    ,"TF60 Destoryer"},
	{"\271\366\274\255\304\277"		    ,SKILL_BERSERKER		,"TF63 Berserker","Ft4_BERSERKER.tga"},
	{"\275\316\300\314\305\254\267\320 \275\272\306\304\300\314\305\251",SKILL_CYCLONE_STRIKE	,"TF66 C_Strike"},
	{"\272\316\275\272\306\256 \307\357\275\272"		,SKILL_BOOST_HEALTH		,"TF70 B_Health"},

	////////////// 파이크맨 계열 
	{"\306\304\300\314\305\251\300\251\265\345"   	,SKILL_PIKE_WIND		,"TP10 P_wind"},
	{"\276\306\300\314\275\272 \276\356\306\256\270\256\272\344\306\256",SKILL_ICE_ATTRIBUTE	,"TP12 I_attribute"},
	{"\305\251\270\256\306\274\304\303 \310\375"		,SKILL_CRITICAL_HIT		,"TP14 Cri_hit"},
	{"\301\241\307\316\305\251\267\241\275\254"		,SKILL_JUMPING_CRASH	,"TP17 J_Crash"},

	{"\261\327\266\363\277\356\265\345\306\304\300\314\305\251"	,SKILL_GROUND_PIKE		,"TP20 G_Pike"},
	{"\305\344\263\327\300\314\265\265"			,SKILL_TORNADO			,"TP23 Tornado"},
	{"\277\376\306\335\265\360\306\346\275\272\270\266\275\272\305\315\270\256",SKILL_WEAPONE_DEFENCE_MASTERY	,"TP26 W_D_Mastery"},
	{"\300\315\275\272\306\346\274\307" 		,SKILL_EXPANSION		,"TP30 Expasion"},

	{"\272\243\263\360 \275\272\307\307\276\356"		,SKILL_VENOM_SPEAR		,"TP40 V_Spear"},
	{"\272\355\267\271\300\314\265\345\277\300\272\352\272\355\267\271\300\314\301\356",SKILL_VANISH        ,"TP43 Vanish","Pk3_VANISH.tga"},
	{"\273\376\307\301\270\266\275\272\305\315\270\256"		,SKILL_CRITICAL_MASTERY	,"TP46 C_Mastery"},
	{"\303\274\300\316\267\243\275\272" 		,SKILL_CHAIN_LANCE		,"TP50 C_Lance"},

	//4차 스킬
	{"\276\356\275\330\275\305 \276\306\300\314"		,SKILL_ASSASSIN_EYE		,"TP60 A_Eye","Pk4_A_EYE.tga"},
	{"\302\367\302\241 \275\272\306\256\266\363\300\314\305\251"	,SKILL_CHARGING_STRIKE	,"TP63 C_Strike"},
	{"\272\243\300\314\261\327"           ,SKILL_VAGUE	        ,"TP66 Vague","Pk4_VAGUE.tga"},
	{"\273\365\265\315\277\354 \270\266\275\272\305\315"	,SKILL_SHADOW_MASTER	,"TP70 S_Master"},

	////////////// 아처 계열 
	{"\275\272\304\253\277\364 \310\243\305\251"		,SKILL_SCOUT_HAWK		,"TA10 S_hawk","Ac1_S_HAWK.tga"},
	{"\275\264\306\303 \270\266\275\272\305\315\270\256"	,SKILL_SHOOTING_MASTERY	,"TA12 S_mastery"},
	{"\300\251\265\345 \276\326\267\316\277\354"		,SKILL_WIND_ARROW		,"TA14 W_arrow"},
	{"\306\333\306\345\306\256 \277\241\300\323"		,SKILL_PERFECT_AIM		,"TA17 P_aim"},

	{"\265\360\277\302\275\272 \276\306\300\314"		,SKILL_DIONS_EYE		,"TA20 D_Eye"},
	{"\306\310\304\334"				,SKILL_FALCON			,"TA23 Falcon","Ac2_FALCON.tga"},
	{"\276\326\267\316\277\354 \277\300\272\352\267\271\300\314\301\366",SKILL_ARROW_OF_RAGE	,"TA26 A_Rage"},
	{"\276\306\271\337\266\365\303\274"			,SKILL_AVALANCHE		,"TA30 Avalanchie"},

	{"\277\244\267\271\270\340\305\273\274\246"		,SKILL_ELEMENTAL_SHOT	,"TA40 E_Shot"},
	{"\260\361\265\347\306\310\304\334"			,SKILL_GOLDEN_FALCON	,"TA43 G_Falcon","Ac3_G_FALCON.tga"},
	{"\271\374 \274\246"			,SKILL_BOMB_SHOT		,"TA46 B_Shot"},
	{"\306\333\306\367\267\271\300\314\274\307"		,SKILL_PERFORATION		,"TA50 Perforation"},

	//4차 스킬
	{"\270\256\304\335 \277\357\271\366\270\260"		  ,SKILL_RECALL_WOLVERIN	,"TA60 R_Wolverin","Ac4_R_WOLVERIN.tga"},
	{"\300\314\272\243\300\314\274\307 \270\266\275\272\305\315\270\256"  ,SKILL_EVASION_MASTERY	,"TA63 E_Mastery"},
	{"\307\307\264\320\275\272 \274\246"          ,SKILL_PHOENIX_SHOT	    ,"TA66 P_Shot"},
	{"\306\367\275\272 \277\300\272\352 \263\273\303\362\267\262"	  ,SKILL_FORCE_OF_NATURE	,"TA70 F_O_Nature","Ac4_F_O_NATURE.tga"},


	///////////////// 나이트
	{"\275\272\277\366\275\272\272\355\267\271\275\272\306\256"	,SKILL_SWORD_BLAST	    ,"MN10 S_Blast"},
	{"\310\246\270\256 \271\331\265\360"		,SKILL_HOLY_BODY		,"MN12 H_Body","Kt1_H_BODY.tga"},
	{"\307\307\301\366\304\303\306\256\267\271\300\314\264\327"	,SKILL_PHYSICAL_TRANING ,"MN14 P_Traning"},
	{"\266\260\272\355 \305\251\267\241\275\254"   	,SKILL_DOUBLE_CRASH     ,"MN17 D_Crash"},

	{"\310\246\270\256 \271\353\267\257"	    ,SKILL_HOLY_VALOR       ,"MN20 H_Valor","Kt2_H_VALOR.tga"},
	{"\272\352\267\243\265\360\275\254"	        ,SKILL_BRANDISH         ,"MN23 Brandish"},
	{"\307\307\276\356\275\314"	        ,SKILL_PIERCING         ,"MN26 Piercing"},
	{"\265\345\267\271\275\272\306\275 \275\272\307\307\270\264"	,SKILL_DRASTIC_SPIRIT   ,"MN30 D_Spirit","Kt2_D_SPIRIT.tga"},

	{"\307\301\267\271\300\323 \272\352\267\243\265\360\275\254"	,SKILL_SWORD_MASTERY   ,"MN40 S_Mastery"},
	{"\265\360\271\331\300\316\300\316\310\246\267\271\300\314\274\307" ,SKILL_DIVINE_INHALATION,"MN43 D_Inhalation","Kt3_D_INHALATION.tga"},
	{"\310\246\270\256\300\316\304\265\305\327\300\314\274\307"	,SKILL_HOLY_INCANTATION ,"MN46 H_Incantation"},
	{"\261\327\267\243\265\345 \305\251\267\316\275\272"	,SKILL_GRAND_CROSS		,"MN50 G_Cross"},

	//4차 스킬
	{"\275\272\277\366\265\345 \277\300\272\352 \300\372\275\272\306\275\275\272"	,SKILL_SWORD_OF_JUSTICE	,"MN60 S_O_Justice"},
	{"\260\253\265\351\270\256 \275\257\265\345"			,SKILL_GODLY_SHIELD		,"MN63 G_Shield","Kt4_G_SHIELD.tga"},
	{"\260\253 \272\355\267\271\275\272"            ,SKILL_GOD_BLESS	    ,"MN66 G_Bless","Kt4_G_BLESS.tga"},
	{"\265\360\271\331\300\316 \307\307\276\356\275\314"		,SKILL_DIVINE_PIERCING	,"MN70 D_Piercing"},
	


	///////////////// 아틀란타
	{"\275\257\265\345 \275\272\306\256\266\363\300\314\305\251"	,SKILL_SHIELD_STRIKE	,"MA10 S_Strike"},
	{"\306\304\270\256\263\252 "			,SKILL_FARINA			,"MA12 Farina"},
	{"\265\345\267\316\300\327\270\266\275\272\305\315\270\256"	,SKILL_THROWING_MASTERY ,"MA14 T_Mastery"},
	{"\272\361\260\305 \275\272\307\307\276\356"	    ,SKILL_VIGOR_SPEAR      ,"MA17 V_Spear"},

	{"\300\251\265\360"	            ,SKILL_WINDY            ,"MA20 Windy","At2_WINDY.tga"},
	{"\306\256\300\247\275\272\306\256 \300\347\272\355\270\260"	,SKILL_TWIST_JAVELIN    ,"MA23 T_Javelin"},
	{"\274\322\277\357 \275\342\304\277"	    ,SKILL_SOUL_SUCKER      ,"MA26 S_Sucker"},
	{"\306\304\300\314\276\356 \300\347\272\355\270\260"	,SKILL_FIRE_JAVELIN     ,"MA30 F_Javelin"},

	{"\275\272\307\303\270\264 \300\332\272\247\270\260"	,SKILL_SPLIT_JAVELIN         ,"MA40 Split_Javelin"},
	{"\306\256\266\363\300\314\276\366\307\301\277\300\272\352\271\337\307\322\266\363",SKILL_TRIUMPH_OF_VALHALLA,"MA43 T_Of_Valhalla","At3_T_O_VALHALLA.tga"},
	{"\266\363\300\314\306\256\264\327\300\347\272\355\270\260"	,SKILL_LIGHTNING_JAVELIN     ,"MA46 L_Javelin"},
	{"\275\272\305\350 \300\347\272\355\270\260"		,SKILL_STORM_JAVELIN         ,"MA50 Storm_Javelin"},

	//4차 스킬
	{"\307\357 \277\300\272\352 \271\337\307\322\266\363"		,SKILL_HALL_OF_VALHALLA ,"MA60 H_O_Valhalla","At4_H_O_VALHALLA.tga"},
	{"\277\242\275\272 \266\363\301\366"			,SKILL_X_RAGE		    ,"MA63 X_Rage"},
	{"\307\301\267\316\275\272\306\256 \300\347\272\355\270\260"      ,SKILL_FROST_JAVELIN	,"MA66 F_Javelin","At4_F_JAVELIN.tga"},
	{"\272\245\303\276"					,SKILL_VENGEANCE		,"MA70 Vengeance"},


	///////////////// 프리스티스
	{"\310\372\270\265"				,SKILL_HEALING			,"MP10 Healing"},
	{"\310\246\270\256\272\274\306\256 "		,SKILL_HOLY_BOLT		,"MP12 HolyBolt"},
	{"\270\326\306\274\275\272\306\304\305\251"		,SKILL_MULTISPARK		,"MP14 M_Spark"},
	{"\310\246\270\256\270\266\300\316\265\345"	    ,SKILL_HOLY_MIND	    ,"MP17 HolyMind"},

	{"\270\336\265\360\305\327\300\314\274\307"	    ,SKILL_MEDITATION	    ,"MP20 Meditation"},
	{"\265\360\271\331\300\316\266\363\300\314\306\256\264\327"	,SKILL_DIVINE_LIGHTNING ,"MP23 D_Lightning"},
	{"\310\246\270\256 \270\256\307\303\267\272\274\307"	,SKILL_HOLY_REFLECTION	,"MP26 H_Reflection","Pr2_H_REFLECTION.tga"},
	{"\261\327\267\271\300\314\306\256 \310\372\270\265"	,SKILL_GRAND_HEALING	,"MP30 G_Healing"},

	{"\272\361\260\305 \272\274"		    ,SKILL_VIGOR_BALL	    ,"MP40 V_Ball"},
	{"\270\256\300\372\267\272\274\307"			,SKILL_RESURRECTION		,"MP43 Resurrection"},
	{"\300\315\275\272\306\303\274\307"			,SKILL_EXTINCTION		,"MP46 Extinction"},
	{"\271\366\303\362\276\363 \266\363\300\314\307\301"	,SKILL_VIRTUAL_LIFE		,"MP50 V_Life","Pr3_V_LIFE.tga"},

	//4차 스킬
	{"\261\333\267\241\274\305 \275\272\306\304\300\314\305\251"	 ,SKILL_GLACIAL_SPIKE	  ,"MP60 G_Spike"},
	{"\270\256\301\246\263\312\267\271\300\314\274\307 \307\312\265\345" ,SKILL_REGENERATION_FIELD,"MP63 R_Field","Pr4_R_FIELD.tga"},
	{"\303\274\300\316 \266\363\300\314\306\256\264\327"     ,SKILL_CHAIN_LIGHTNING	  ,"MP66 C_Lightning"},
	{"\274\255\270\363 \271\302\275\272\306\347"		 ,SKILL_SUMMON_MUSPELL	  ,"MP70 S_Muspell","Pr4_S_MUSPELL.tga"},



	///////////////// 메지션
	{"\276\356\260\355\264\317"			,SKILL_AGONY			,"MM10 Agony"},
	{"\306\304\300\314\276\356\272\274\306\256 "		,SKILL_FIRE_BOLT		,"MM12 FireBolt"},
	{"\301\246\264\317\276\262"			,SKILL_ZENITH			,"MM14 Zenith","Mg1_ZENITH.tga"},
	{"\306\304\300\314\276\356\272\274"			,SKILL_FIRE_BALL	    ,"MM17 FireBall"},

	{"\270\307\305\273 \270\266\275\272\305\315\270\256"	,SKILL_MENTAL_MASTERY   ,"MM20 M_Mastery"},
	{"\304\335\267\353\277\300\272\352\277\366\305\315"		,SKILL_WATORNADO	    ,"MM23 Watornado"},
	{"\300\316\303\246\306\256 \277\376\306\371"		,SKILL_ENCHANT_WEAPON	,"MM26 E_Weapon","Mg2_E_WEAPON.tga"},
	{"\265\245\265\345\267\271\300\314"			,SKILL_DEAD_RAY  	    ,"MM30 D_Ray"},

	{"\277\241\263\312\301\366 \275\257\265\345"		,SKILL_ENERGY_SHIELD    ,"MM40 E_Shield","Mg3_E_SHIELD.tga"},
	{"\264\331\300\314\276\306\275\272\306\256\267\316\307\307\301\362"	,SKILL_DIASTROPHISM	    ,"MM43 Diastrophism"},
	{"\275\272\307\307\270\264 \277\244\267\271\270\340\305\273"	,SKILL_SPIRIT_ELEMENTAL	,"MM46 S_Elemental","Mg3_S_ELEMENTAL.tga"},
	{"\264\355\275\314\275\272\277\366\265\345"		,SKILL_DANCING_SWORD  	,"MM50 D_Sword","Mg3_D_SWORD.tga"},

	//4차 스킬
	{"\306\304\300\314\276\356 \277\244\267\271\270\340\305\273"	,SKILL_FIRE_ELEMENTAL	,"MM60 F_Elemental","Mg4_F_ELEMENTAL.tga"},
	{"\307\303\266\363\300\323 \277\376\300\314\272\352"    ,SKILL_FLAME_WAVE		,"MM63 F_Wave"},
	{"\265\360\275\272\306\256\267\317\274\307"       ,SKILL_DISTORTION	    ,"MM66 Distortion","Mg4_DISTORTION.tga"},
	{"\270\336\305\327\277\300"		    ,SKILL_M_METEO		    ,"MM70 Meteo"},

	// tier 5 (the fifth rank). Each must come after its class's fourth tier: SearchUseSkill
	// fills the class's window slots in table order, so these land in slots 17..20.
	{"Land Mining"		,SKILL_LAND_MINING		,"TM80 landminning"},
	{"Hyper Sonic"		,SKILL_HYPER_SONIC		,"TM83 H_Sonic","Me5_H_Sonic.tga"},
	{"Rolling Smash"	,SKILL_ROLLING_SMASH	,"TM86 R_Smash"},
	{"Power Enhancement",SKILL_POWER_ENHANCEMENT,"TM90 P_Enhence","Me5_P_Enhence.tga"},

	{"Down Hit"			,SKILL_DOWN_HIT			,"TF80 D_Hit"},
	{"Power Dash"		,SKILL_POWER_DASH		,"TF83 P_Dash"},
	{"Mortal Blow"		,SKILL_MORTAL_BLOW		,"TF86 M_Blow"},
	{"Bloody Berserker"	,SKILL_BLOODY_BERSERKER	,"TF90 B_Berserker","Ft5_B_Berserker.tga"},

	{"Dancing Reaper"	,SKILL_DANCING_REAPER	,"TP80 D_Reaper"},
	{"Final Spear"		,SKILL_FINAL_SPEAR		,"TP83 F_Spear"},
	{"Amplified"		,SKILL_AMPLIFIED		,"TP86 Amplified","Pk5_Amplified.tga"},
	{"Side Spin Attack"	,SKILL_SIDE_SPIN_ATTACK	,"TP90 SS_Attack"},

	{"Evade Shot"		,SKILL_EVADE_SHOT		,"TA80 E_Shot"},
	{"Thunder Rope"		,SKILL_THUNDER_ROPE		,"TA83 S_Rope"},
	{"Nature Splash"	,SKILL_NATURE_SPLASH	,"TA86 N_Splash"},
	{"Circle Trap"		,SKILL_CIRCLE_TRAP		,"TA90 C_Trap"},

	{"Soul Breaker"		,SKILL_SOUL_BREAKER		,"MN80 S_Breaker"},
	{"Crescent Moon"	,SKILL_CRESCENT_MOON	,"MN83 C_Moon"},
	{"Saint Blade"		,SKILL_SAINT_BLADE		,"MN86 S_Blade"},
	{"Holy Benedic"		,SKILL_HOLY_BENEDIC		,"MN90 H_Benedic","Kn5_H_Benedic.tga"},

	{"Talaria"			,SKILL_TALARIA			,"MA80 Talaria","At5_Talaria.tga"},
	{"Galaxy Coup"		,SKILL_GALAXY_COUP		,"MA83 G_Coup"},
	{"Summon Arcuda"	,SKILL_SUMMON_ARCUDA	,"MA86 S_Arcuda","At5_S_Arcuda.tga"},
	{"Snippy Fear"		,SKILL_SNIPPY_FEAR		,"MA90 S_Fear"},

	{"Spirit Impact"	,SKILL_SPIRIT_IMPACT	,"MP80 S_Impact"},
	{"Piercing Ice"		,SKILL_PIERCING_ICE		,"MP83 P_Ice"},
	{"Summon Ramiel"	,SKILL_SUMMON_RAMIEL	,"MP86 S_Ramiel","Pr5_S_Ramiel.tga"},
	{"Blessing Krishna"	,SKILL_BLESSING_KRISHNA	,"MP90 B_Krishna","Pr5_B_Krishna.tga"},

	{"Silraphim"		,SKILL_SILRAPHIM		,"MM80 Silraphim"},
	{"Vis Tenus"		,SKILL_VIS_TENUS		,"MM83 V_Tenus"},
	{"Prima Ignis"		,SKILL_PRIMA_IGNIS		,"MM86 P_Ignis"},
	{"Third Anima"		,SKILL_THIRD_ANIMA		,"MM90 T_Anima","Ma5_T_Anima.tga"},

	//공성전 클랜 스킬
	{"\310\355\274\366\267\302 \273\363\275\302"	    ,CLANSKILL_ABSORB	 ,"","wa1-1.tga"},
	{"\260\370\260\335\267\302 \273\363\275\302"      ,CLANSKILL_ATTACK	 ,"","wa1-3.tga"},
	{"\310\270\307\307\300\262 \273\363\275\302"      ,CLANSKILL_EVASION	 ,"","wa1-2.tga"},

	//공성전 아이템 스킬
	{"\276\306\305\270\263\252\275\303\276\306"			 ,SCROLL_INVULNERABILITY	 ,"","S_Ath.tga"},
	{"\265\245\265\351\270\256 \277\241\301\366"			 ,SCROLL_CRITICAL            ,"","S_Dea.tga"},
	{"\276\356\272\243\300\317 \277\300\272\352 \300\314\272\243\300\314\265\345"	 ,SCROLL_EVASION	         ,"","S_Eva.tga"},
	{"\265\360\306\304\300\314\276\360\275\272 \275\272\305\346"		 ,STONE_R_FIRECRYTAL	     ,"","F_C_M.tga"},
	{"\265\360\306\304\300\314\276\360\275\272 \275\272\305\346"		 ,STONE_R_ICECRYTAL	         ,"","I_C_M.tga"},
	{"\265\360\306\304\300\314\276\360\275\272 \275\272\305\346"	     ,STONE_R_LINGHTINGCRYTAL    ,"","L_C_M.tga"},
	{"\270\266\300\314\306\256 \275\272\305\346"			 ,STONE_A_FIGHTER		     ,"","Ft_R.tga"},
	{"\270\266\300\314\306\256 \275\272\305\346"			 ,STONE_A_MECHANICIAN	     ,"","Mc_R.tga"},
	{"\270\266\300\314\306\256 \275\272\305\346"			 ,STONE_A_PIKEMAN		     ,"","Pk_R.tga"},
	{"\270\266\300\314\306\256 \275\272\305\346"	         ,STONE_A_ARCHER			 ,"","Ac_R.tga"},
	{"\270\266\300\314\306\256 \275\272\305\346"           ,STONE_A_KNIGHT			 ,"","Kt_R.tga"},
	{"\270\266\300\314\306\256 \275\272\305\346"           ,STONE_A_ATALANTA		     ,"","At_R.tga"},
	{"\270\266\300\314\306\256 \275\272\305\346"           ,STONE_A_MAGICIAN		     ,"","Mg_R.tga"},
	{"\270\266\300\314\306\256 \275\272\305\346"           ,STONE_A_PRIESTESS		     ,"","Pt_R.tga"},

	{"\300\314\305\315\263\316 \266\363\300\314\307\301"	     ,SCROLL_P_INVULNERABILITY	 ,"","S_Ath.tga"},
	{"\306\344\300\314\306\262 \277\241\301\366"			 ,SCROLL_P_CRITICAL          ,"","S_Dea.tga"},
	{"\276\356\271\366\306\256 \275\272\305\251\267\321"	     ,SCROLL_P_EVASION	         ,"","S_Eva.tga"},

	// 박재원 - 부스터 아이템(생명력, 기력, 근력)
	{"\273\375\270\355\267\302 \272\316\275\272\305\315",		BOOSTER_ITEM_LIFE,			"",		"Booster_01.tga"},
	{"\261\342\267\302 \272\316\275\272\305\315",			BOOSTER_ITEM_MANA,			"",		"Booster_02.tga"},
	{"\261\331\267\302 \272\316\275\272\305\315",			BOOSTER_ITEM_STAMINA,		"",		"Booster_03.tga"},

	// 장별 - 스킬 딜레이
	{"\275\272\305\263 \265\364\267\271\300\314",			SKILLDELAY_ITEM_LIFE,			"",		"Booster_01.tga"},

};


//스킬 창 박스 위치 정보 
sSKILLBOX	sSkillBox[SIN_MAX_SKILLBOX] = {
	{{142,537,142+49,537+46}},							//노말 

	{{27-10,502,27+49-10,502+46},{27+47-10,502+5,27+47+5-10,502+40}},		//1단계
	{{67-10,546,67+49-10,546+46},{67+47-10,546+5,67+47+5-10,546+40}},
	{{77-10,466,77+49-10,466+46},{77+47-10,466+5,77+47+5-10,466+40}},
	{{137-10,476,137+49-10,476+46},{137+47-10,476+5,137+47+5-10,476+40}},

	{{212-15,466,212+49-15,466+46},{212+47-15,466+5,212+47+5-15,466+40}},	//2단계
	{{232-15,516,232+49-15,516+46},{232+47-15,516+5,232+47+5-15,516+40}},
	{{272-15,466,272+49-15,466+46},{272+47-15,466+5,272+47+5-15,466+40}},
	{{332,466,332+49,466+46},{332+47,466+5,332+47+5,466+40}},

	{{402,476,402+49, 476+46},{402+47,476+5,402+47+5,476+40}},	//3단계
	{{472,456,472+49, 456+46},{472+47,456+5,472+47+5,456+40}},
	{{512,506,512+49, 506+46},{512+47,506+5,512+47+5,506+40}},
	{{532,456,532+49, 456+46},{532+47,456+5,532+47+5,456+40}},

	{{602,481,602+49, 481+46},{602+47,481+5,602+47+5,481+40}},	//4단계
	{{662,511,662+49, 511+46},{662+47,511+5,662+47+5,511+40}},
	{{681,461,681+49, 461+46},{681+47,461+5,681+47+5,461+40}},
	{{722,511,722+49, 511+46},{722+47,511+5,722+47+5,511+40}},

	// tier 5, on the second page (Skil25.bmp): its four slots sit where the first tier's do
	{{27-10,502,27+49-10,502+46},{27+47-10,502+5,27+47+5-10,502+40}},
	{{67-10,546,67+49-10,546+46},{67+47-10,546+5,67+47+5-10,546+40}},
	{{77-10,466,77+49-10,466+46},{77+47-10,466+5,77+47+5-10,466+40}},
	{{137-10,476,137+49-10,476+46},{137+47-10,476+5,137+47+5-10,476+40}},
};

// whether skill window slot i shows on the page that is open (page 2 = the tier 5 slots)
extern int SkillMain2OpenFlag;
static int SkillOnShownPage( int i )
{
	if ( i==0 ) return TRUE;			// normal attack: the fist box is on both pages
	return SkillMain2OpenFlag ? ( i>=SIN_TIER5_FIRST ) : ( i<SIN_TIER5_FIRST );
}

sSKILLBOX sLeftRightSkill[2] = {
	{{349,541,349+49,541+46},{349,558,349+5,558+35}},  //스킬 왼쪽 
	{{401+2,541,401+49,541+46},{446,558,446+5,558+35}},  //스킬 오른쪽 

};

int SkillTitle[4][2] = {
	{85,438-4},
	{280,438-4},
	{470,438-9},
	{675,438-4},

};
//스킬 메인=======================================
LPDIRECTDRAWSURFACE4	lpSkillMain;
LPDIRECTDRAWSURFACE4	lpSkillMain2; //스킬 메인2
//스킬 메인버튼
LPDIRECTDRAWSURFACE4    lpSkillMainLeftB;
LPDIRECTDRAWSURFACE4    lpSkillMainRightB;

int SkillMain2OpenFlag   = 0;        //스킬 메인 flag
int SkillMain2ButtonFlag[2] = {0,0};
//================================================


//4차 스킬 마스터flag
int Skill4MasterFlag=0;




int MonsterDamageLine = 0;
int ChainDamageLine   = 0;
int MatSkillIconBackGround   = 0;
/*----------------------------------------------------------------------------*
*							클래스 초기, 종료 
*-----------------------------------------------------------------------------*/	
cSKILL::cSKILL()
{
	memset(&ContinueSkill,0,sizeof(sSKILL)*10); //지속 사용스킬 초기화 

}
cSKILL::~cSKILL()
{

}
/*----------------------------------------------------------------------------*
*							     초기화 
*-----------------------------------------------------------------------------*/	
void cSKILL::Init()
{
	
	char *FilePath = "Image\\SinImage\\skill\\keep\\";
	char szFilePath[128];
	for(int j = 0 ; j < SIN_MAX_SKILL; j++){
		if(sSkill[j].IconName){
			wsprintf(szFilePath,"%s%s",FilePath,sSkill[j].IconName); 
			sSkill[j].MatIcon =  CreateTextureMaterial( szFilePath , 0, 0, 0,0, SMMAT_BLEND_ALPHA );

		}
	}


	//MatSkill[0] =  CreateTextureMaterial( "Image\\SinImage\\Skill\\Skill-1.tga" , 0, 0, 0,0, SMMAT_BLEND_ALPHA );
	//MatSkill[1] =  CreateTextureMaterial( "Image\\SinImage\\Skill\\Skill-2.tga" , 0, 0, 0,0, SMMAT_BLEND_ALPHA );
	//MatSkill[2] =  CreateTextureMaterial( "Image\\SinImage\\Skill\\Skill-3.tga" , 0, 0, 0,0, SMMAT_BLEND_ALPHA );
	//MatSkill[3] =  CreateTextureMaterial( "Image\\SinImage\\Skill\\Skill-4.tga" , 0, 0, 0,0, SMMAT_BLEND_ALPHA );

	MatCircleIcon = CreateTextureMaterial( "Image\\SinImage\\Skill\\keep\\GA_Mon.tga" , 0, 0, 0,0, SMMAT_BLEND_ALPHA );;
	MatSkillIconBackGround = CreateTextureMaterial( "Image\\SinImage\\Skill\\keep\\GA_.tga" , 0, 0, 0,0, SMMAT_BLEND_ALPHA );;

	SearchUseSkill(); //현재 사용할수 있는 캐릭터의 스킬을 찾는다 
	Load();

	//기본공격을 셋팅한다 
	sinSkill.pLeftSkill = &sinSkill.UseSkill[0];
	sinSkill.UseSkill[0].MousePosi = 1;
	sinSkill.pLeftSkill->Position = 1;

	
	
}
/*----------------------------------------------------------------------------*
*						    Bmp파일을 로드한다 
*-----------------------------------------------------------------------------*/	
void cSKILL::Load()
{
	char ShortKeyDirPath[128];
	char WeaponIconPath[128];
	
	lpSkillDown    = LoadDibSurfaceOffscreen("Image\\SinImage\\skill\\skilldown.bmp");

	//스킬 메인2=========================================================================== 
	lpSkillMain    = LoadDibSurfaceOffscreen("Image\\SinImage\\skill\\Skil24.bmp");
	//lpSkillMain2   = LoadDibSurfaceOffscreen("Image\\SinImage\\skill\\Skil25.bmp");

	//스킬 메인 버튼
	lpSkillMainLeftB  = LoadDibSurfaceOffscreen("Image\\SinImage\\skill\\LeftArrow.bmp");
	lpSkillMainRightB = LoadDibSurfaceOffscreen("Image\\SinImage\\skill\\RightArrow.bmp");
	//======================================================================================



	lpSkillGage[0] = LoadDibSurfaceOffscreen("Image\\SinImage\\skill\\Gage.bmp");
	lpSkillGage[1] = LoadDibSurfaceOffscreen("Image\\SinImage\\skill\\Gage2.bmp");
	lpSkillGage2[0] = LoadDibSurfaceOffscreen("Image\\SinImage\\skill\\Gage-2.bmp");
	lpSkillGage2[1] = LoadDibSurfaceOffscreen("Image\\SinImage\\skill\\Gage-22.bmp");
	lpSkillGage3 = LoadDibSurfaceOffscreen("Image\\SinImage\\skill\\Gage-3.bmp");
	lpSkillGage4 = LoadDibSurfaceOffscreen("Image\\SinImage\\skill\\Gage-4.bmp");
	lpSkillGage5 = LoadDibSurfaceOffscreen("Image\\SinImage\\skill\\Gage-5.bmp");

	lpMasteryInfo = LoadDibSurfaceOffscreen("Image\\SinImage\\skill\\MasteryInfo.bmp");

	lpSelectSkill  = LoadDibSurfaceOffscreen("Image\\SinImage\\skill\\SelectSkill.bmp");
	lpSkillBoxBack = LoadDibSurfaceOffscreen("Image\\SinImage\\skill\\BOX4040.bmp");

	int i; for( i =0;i < 8; i++){
		wsprintf(ShortKeyDirPath,"Image\\SinImage\\skill\\ShortKey\\f%d.bmp",i+1);
		lpShortKey[i]= LoadDibSurfaceOffscreen(ShortKeyDirPath);//펑션키 

	}
	for(i=0;i < 10; i++){
		wsprintf(WeaponIconPath,"Image\\SinImage\\skill\\WeaponIcon\\%d.bmp",i+1);
		lpWeaponIcon[i]= LoadDibSurfaceOffscreen(WeaponIconPath);//무기 아이콘 

	}
	
}

//스킬이미지를 로드한다 
void cSKILL::LoadUseSkillImage()
{
	int i=0;
	char TempDirPath[128];
	char TempTitleDirPath[128];
	char ButtonImageDirPath[128];

	TempDirPath[0] = 0;
	switch(UseSkillBox){
		case GROUP_MECHANICIAN:
			lstrcpy(TempDirPath,"Image\\SinImage\\Skill\\Mecha\\" );

		break;
		case GROUP_FIGHTER:
			lstrcpy(TempDirPath,"Image\\SinImage\\Skill\\Fighter\\" );

		break;
		case GROUP_PIKEMAN:
			lstrcpy(TempDirPath,"Image\\SinImage\\Skill\\Pikeman\\" );

		break;
		case GROUP_ARCHER:
			lstrcpy(TempDirPath,"Image\\SinImage\\Skill\\Archer\\" );

		break;
		case GROUP_ATALANTA:
			lstrcpy(TempDirPath,"Image\\SinImage\\Skill\\Atalanta\\" );
		break;
		case GROUP_KNIGHT:
			lstrcpy(TempDirPath,"Image\\SinImage\\Skill\\Knight\\" );
			
		break;
		case GROUP_MAGICIAN:
			lstrcpy(TempDirPath,"Image\\SinImage\\Skill\\Magician\\" );
			
		break;
		case GROUP_PRIESTESS:
			lstrcpy(TempDirPath,"Image\\SinImage\\Skill\\Priestess\\" );
			
		break;


	}
	if(!TempDirPath[0]) //경로명을 찾지못하면 리턴한다 
		return;


	lpSkillButton[0] = LoadDibSurfaceOffscreen("Image\\SinImage\\Skill\\Skill_Normal.bmp"); //노멀 어택 
	lpSkillButton_Gray[0] = LoadDibSurfaceOffscreen("Image\\SinImage\\Skill\\Skill_Normal.bmp"); //노멀 어택 
	for(i = 0 ; i < 5 ; i++){		// 5: the fifth rank's title, drawn on the second page
		wsprintf(TempTitleDirPath,"%s%s%d%s",TempDirPath,"JobTitle\\",i+1,".bmp");
		lpSkillTitle[i] = LoadDibSurfaceOffscreen(TempTitleDirPath);

	}

	/////////////////버튼 이미지 로딩
	for(i = 1 ; i < SIN_MAX_USE_SKILL ; i++){ //노멀 어택 이미지를 뺀 나머지 스킬 이미지
		if(sinSkill.UseSkill[i].Flag){
			if(sinSkill.UseSkill[i].FileName[0]){
				wsprintf(ButtonImageDirPath,"%s%s%s%s",TempDirPath,"Button\\",sinSkill.UseSkill[i].FileName,".bmp");
				lpSkillButton[i] = 	LoadDibSurfaceOffscreen(ButtonImageDirPath);
				wsprintf(ButtonImageDirPath,"%s%s%s%s%s",TempDirPath,"Button\\",sinSkill.UseSkill[i].FileName,"_",".bmp");
				lpSkillButton_Gray[i] = 	LoadDibSurfaceOffscreen(ButtonImageDirPath);
			}
		}
	}

}

/*----------------------------------------------------------------------------*
*						     Release
*-----------------------------------------------------------------------------*/	
void cSKILL::Release()
{
	int i = 0;
	if(lpSkillGage[0]){
		lpSkillGage[0]->Release();
		lpSkillGage[0] = 0;
	}
	if(lpSkillGage[1]){
		lpSkillGage[1]->Release();
		lpSkillGage[1] = 0;


	}
	if(lpSelectSkill){
		lpSelectSkill->Release();
		lpSelectSkill = 0;

	}
	for(i = 0 ; i < 5 ; i++){
		if(lpSkillTitle[i]){
			lpSkillTitle[i]->Release();
			lpSkillTitle[i] = 0;

		}
	}
	for(i = 0 ; i <SIN_MAX_USE_SKILL ; i ++){
		if(lpSkillButton[i]){
			lpSkillButton[i]->Release();
			lpSkillButton[i] = 0;


		}
		if(lpSkillButton_Gray[i]){
			lpSkillButton_Gray[i]->Release();
			lpSkillButton_Gray[i] = 0;

		}
	}
	for(i=0; i <  8 ; i++){
		if(lpShortKey[i]){
			lpShortKey[i]->Release();
			lpShortKey[i] = 0;
			
		}
	}
	for(i=0; i <  10 ; i++){
		if(lpWeaponIcon[i]){
			lpWeaponIcon[i]->Release();
			lpWeaponIcon[i] = 0;
		}
	}

}
/*----------------------------------------------------------------------------*
*							     그리기  
*-----------------------------------------------------------------------------*/	
int CheckVirtualLifeTime = 0;
int CheckBoosterLifeTime = 0; // 박재원 - 부스터 아이템(생명력)
int CheckVirtualManaTime = 0; // 박재원 - 부스터 아이템(기력)
int CheckVirtualStaminaTime = 0; // 박재원 - 부스터 아이템(근력)
int CheckSkillDelayTime = 0; // 장별 - 스킬 딜레이
void cSKILL::DrawUp()
{
	int i=0;
	//지속 스킬을 그린다
	for(i = 0 ; i < MAX_CONTINUE_SKILL ; i++){     
		if(TempContinueSkill[i].Flag){
			if(TempContinueSkill[i].MatIcon){
				if(!TempContinueSkill[i].IconFlag){
					dsDrawTexImage(MatSkillIconBackGround , 10+TempContinueSkill[i].IconPosiX, 10, 32, 32 , TempContinueSkill[i].IconAlpha );       
					SkillBarDraw(&TempContinueSkill[i],10+16+TempContinueSkill[i].IconPosiX,10+16,TempContinueSkill[i].IconAlpha);  
					dsDrawTexImage(TempContinueSkill[i].MatIcon , 10+TempContinueSkill[i].IconPosiX, 10, 32, 32 , TempContinueSkill[i].IconAlpha );       
				}
			}
		}
	}

}
void cSKILL::Draw()
{
	int i=0;

	/*
	//지속 스킬을 그린다
	for(i = 0 ; i < MAX_CONTINUE_SKILL ; i++){     
		if(TempContinueSkill[i].Flag){
			if(TempContinueSkill[i].MatIcon){
				if(!TempContinueSkill[i].IconFlag){
					dsDrawTexImage(MatSkillIconBackGround , 10+TempContinueSkill[i].IconPosiX, 10, 32, 32 , TempContinueSkill[i].IconAlpha );       
					SkillBarDraw(&TempContinueSkill[i],10+16+TempContinueSkill[i].IconPosiX,10+16,TempContinueSkill[i].IconAlpha);  
					dsDrawTexImage(TempContinueSkill[i].MatIcon , 10+TempContinueSkill[i].IconPosiX, 10, 32, 32 , TempContinueSkill[i].IconAlpha );       
				}
			}
		}
	}
	*/

	/////////////이미지 테스트 
/*
	for(i=0; i <  9 ; i++){
		if(lpWeaponIcon[i]){
			DrawSprite(100+(i*20),100,lpWeaponIcon[i],0,0,18,24);
			
		} 
	}
*/
	/////////////////스킬 박스 
	//dsDrawTexImage( MatSkill[0] , 0, 600-sinMoveKindInter[SIN_SKILL], 256, 256 , 255 );       
	//dsDrawTexImage( MatSkill[1] , 256, 600-sinMoveKindInter[SIN_SKILL], 256, 256 , 255 );   
	//dsDrawTexImage( MatSkill[2] , 256+256, 600-sinMoveKindInter[SIN_SKILL], 256, 256 , 255 );       
	//dsDrawTexImage( MatSkill[3] , 256+256+256, 600-sinMoveKindInter[SIN_SKILL], 32, 256 , 255 );   

	//DrawSprite(0,0,lpSkillMain,0,0,800,200);
	
	//스킬 메인2=====================================================================================
	if(SkillMain2OpenFlag){
		DrawSprite(0,600-sinMoveKindInter[SIN_SKILL]+56,lpSkillMain2,0,0,800,200);
		//왼쪽
		if(SkillMain2ButtonFlag[0]){
			DrawSprite(13,600+83-sinMoveKindInter[SIN_SKILL]+56,lpSkillMainLeftB,0,0,20,20);
		}
	}

	if(!SkillMain2OpenFlag){
		DrawSprite(0,600-sinMoveKindInter[SIN_SKILL]+56,lpSkillMain,0,0,800,200);
		//오른쪽
		if(SkillMain2ButtonFlag[1]){
			DrawSprite(766,600+83-sinMoveKindInter[SIN_SKILL]+56,lpSkillMainRightB,0,0,20,20);
		}
	}
	//===========================================================================================
	if(OpenFlag){
		if(SkillMain2OpenFlag){		// the second page: the fifth rank's title over its tier 5 column
			if(sinChar->ChangeJob>=4 && lpSkillTitle[4])
				DrawSprite(SkillTitle[0][0],SkillTitle[0][1]+(256-sinMoveKindInter[SIN_SKILL]),lpSkillTitle[4],0,0,100,20);
		}
		else {
			int Titles = (int)sinChar->ChangeJob+1;
			if(Titles > 4) Titles = 4;
			for(i = 0 ; i < Titles ; i++){ //타이틀
				DrawSprite(SkillTitle[i][0],SkillTitle[i][1]+(256-sinMoveKindInter[SIN_SKILL]),lpSkillTitle[i],0,0,100,20);
			}
		}
		////////////////////스킬 버튼
		for(i = 1 ; i < ChangeJobSkillPlus ; i++){ //노멀 어택을 빼준다
			if(!SkillOnShownPage(i)) continue;
			if(sinSkill.UseSkill[i].Flag && sinSkill.UseSkill[i].Use && SkillMasterFlag){//활성화
				if(lpSkillButton[i]){  
					DrawSprite(sinSkill.SkillBox[i].BoxRect.left , sinSkill.SkillBox[i].BoxRect.top +(256-sinMoveKindInter[SIN_SKILL]) ,
							lpSkillButton[i],0,0,49,46);
				}
			}
			if(sinSkill.UseSkill[i].Flag && !sinSkill.UseSkill[i].Use  && SkillMasterFlag ){
				if(lpSkillButton_Gray[i]){
					DrawSprite(sinSkill.SkillBox[i].BoxRect.left , sinSkill.SkillBox[i].BoxRect.top +(256-sinMoveKindInter[SIN_SKILL]) ,
							lpSkillButton_Gray[i],0,0,49,46);
				}
			}
			if(sinSkill.UseSkill[i].Flag &&  SkillMasterFlag && sinSkill.UseSkill[i].Point >= 10){//비활성화
				if(lpSkillButton_Gray[i]){
					DrawSprite(sinSkill.SkillBox[i].BoxRect.left , sinSkill.SkillBox[i].BoxRect.top +(256-sinMoveKindInter[SIN_SKILL]) ,
							lpSkillButton_Gray[i],0,0,49,46);
				}
			}
			if(!SkillMasterFlag){ //사용할수있는 스킬을 표시해준다 
				if(sinSkill.UseSkill[i].Flag && sinSkill.UseSkill[i].Point){//활성화
					if(lpSkillButton[i]){  
						DrawSprite(sinSkill.SkillBox[i].BoxRect.left , sinSkill.SkillBox[i].BoxRect.top +(256-sinMoveKindInter[SIN_SKILL]) ,
								lpSkillButton[i],0,0,49,46);
						
					}
				}
				if(sinSkill.UseSkill[i].Flag && !sinSkill.UseSkill[i].Point){//비활성화
					if(lpSkillButton_Gray[i]){
						DrawSprite(sinSkill.SkillBox[i].BoxRect.left , sinSkill.SkillBox[i].BoxRect.top +(256-sinMoveKindInter[SIN_SKILL]) ,
								lpSkillButton_Gray[i],0,0,49,46);
					}
				}
			}
			//숙련도 게이지를 그려준다
			if(sinSkill.UseSkill[i].Flag && sinSkill.UseSkill[i].Skill_Info.USECODE == SIN_SKILL_USE_NOT){
				DrawSprite(sinSkill.SkillBox[i].GageRect.left ,sinSkill.SkillBox[i].GageRect.top+(256-sinMoveKindInter[SIN_SKILL]) , lpSkillGage4 , 0,0,5,35);
			} 

			if(sinSkill.UseSkill[i].Flag && (sinSkill.UseSkill[i].Skill_Info.USECODE != SIN_SKILL_USE_NOT || sinSkill.UseSkill[i].Skill_Info.Element[0])){
				DrawSprite(sinSkill.SkillBox[i].GageRect.left ,sinSkill.SkillBox[i].GageRect.top+(256-sinMoveKindInter[SIN_SKILL]) , lpSkillGage3 , 0,0,5,35);
				DrawSprite(sinSkill.SkillBox[i].GageRect.left ,sinSkill.SkillBox[i].GageRect.top+(35-sinSkill.UseSkill[i].UseSkillMasteryGage)+(256-sinMoveKindInter[SIN_SKILL]) , lpSkillGage2[0] , 0,35-sinSkill.UseSkill[i].UseSkillMasteryGage,5,35);
				if(sinSkill.UseSkill[i].UseSkillMasteryGage >=3)
					DrawSprite(sinSkill.SkillBox[i].GageRect.left ,sinSkill.SkillBox[i].GageRect.top+(35-sinSkill.UseSkill[i].UseSkillMasteryGage)+(256-sinMoveKindInter[SIN_SKILL]) , lpSkillGage2[1] , 0,0,5,3);

			}
			if(sinSkill.UseSkill[i].Flag){
				DrawSprite(sinSkill.SkillBox[i].GageRect.right+1,sinSkill.SkillBox[i].GageRect.top+(35-sinSkill.UseSkill[i].GageLength)+(256-sinMoveKindInter[SIN_SKILL]) , lpSkillGage[0] , 0,35-sinSkill.UseSkill[i].GageLength,5,35);
				if(sinSkill.UseSkill[i].GageLength >=3)
					DrawSprite(sinSkill.SkillBox[i].GageRect.right+1 ,sinSkill.SkillBox[i].GageRect.top+(35-sinSkill.UseSkill[i].GageLength)+(256-sinMoveKindInter[SIN_SKILL]) , lpSkillGage[0] , 0,0,5,3);

			}
			//딜레이없는 4차스킬게이지
			if(sinSkill.UseSkill[i].Flag && sinSkill.UseSkill[i].Skill_Info.Element[0]){
				DrawSprite(sinSkill.SkillBox[i].GageRect.left ,sinSkill.SkillBox[i].GageRect.top+(256-sinMoveKindInter[SIN_SKILL]) , lpSkillGage5 , 0,0,5,35);
			} 


		}
		if(DownButtonIndex){  //버튼 다운 액션  
			DrawSprite(sinSkill.SkillBox[DownButtonIndex-1].BoxRect.left+1 , sinSkill.SkillBox[DownButtonIndex-1].BoxRect.top+1 +(256-sinMoveKindInter[SIN_SKILL]) ,
					lpSkillButton[DownButtonIndex-1],0,0,49,46); //픽셀을 옆으로이동 

		}
		if(SkillButtonIndex){
			if(sinSkill.UseSkill[SkillButtonIndex-1].Flag){ 
				if(DownButtonIndex){
					DrawSprite(sinSkill.SkillBox[SkillButtonIndex-1].BoxRect.left+1,
						sinSkill.SkillBox[SkillButtonIndex-1].BoxRect.top+1+(256-sinMoveKindInter[SIN_SKILL]),
						lpSelectSkill,0,0,49,46);

				}
				else{
					DrawSprite(sinSkill.SkillBox[SkillButtonIndex-1].BoxRect.left,
						sinSkill.SkillBox[SkillButtonIndex-1].BoxRect.top+(256-sinMoveKindInter[SIN_SKILL]),
						lpSelectSkill,0,0,49,46);
				}
			}

		}

		//////////////////// ShorKey 
		for(i = 0 ; i < ChangeJobSkillPlus ; i++){
			if(!SkillOnShownPage(i)) continue;
			if(sinSkill.UseSkill[i].Flag){
				if(sinSkill.UseSkill[i].ShortKey){
					DrawSprite(sinSkill.SkillBox[i].BoxRect.left+9 , sinSkill.SkillBox[i].BoxRect.top-6+(256-sinMoveKindInter[SIN_SKILL]) ,
							lpShortKey[sinSkill.UseSkill[i].ShortKey-1],0,0,16,16);

				}
			}
		}
	}
			//숙련도 표시 
	if(SkillMasteryShowFlag){
		DrawSprite(UseSkillMasteryPosi.x,UseSkillMasteryPosi.y+(256-sinMoveKindInter[SIN_SKILL]) , lpMasteryInfo , 0,0,39,27);
	}
	
	/////스킬 게이지는 인터페이스에서 그린다 
	if(sinSkill.pLeftSkill){		 //왼쪽 스킬 
		if(sinSkill.pLeftSkill->UseSkillFlag)
			DrawSprite(sLeftRightSkill[0].BoxRect.left , sLeftRightSkill[0].BoxRect.top , //버튼 
					lpSkillButton[sinSkill.pLeftSkill->Position-1],0,0,49,46);
		else
			DrawSprite(sLeftRightSkill[0].BoxRect.left , sLeftRightSkill[0].BoxRect.top , //버튼 
					lpSkillButton_Gray[sinSkill.pLeftSkill->Position-1],0,0,49,46);


	}
	else{ //스킬 추가가 되지않았을때 이미지 
		DrawSprite(sLeftRightSkill[0].BoxRect.left , sLeftRightSkill[0].BoxRect.top , //버튼 
				lpSkillBoxBack,0,0,49,46);

	}
	if(sinSkill.pRightSkill){		//오른쪽 스킬  
		if(sinSkill.pRightSkill->UseSkillFlag) //사용가능 
			DrawSprite(sLeftRightSkill[1].BoxRect.left , sLeftRightSkill[1].BoxRect.top , //버튼 
					lpSkillButton[sinSkill.pRightSkill->Position-1],0,0,49,46);
		else	//사용 불가능 
			DrawSprite(sLeftRightSkill[1].BoxRect.left , sLeftRightSkill[1].BoxRect.top , //버튼 
					lpSkillButton_Gray[sinSkill.pRightSkill->Position-1],0,0,49,46);


	}
	else{ //스킬 추가가되지않았을때 이미지 
		DrawSprite(sLeftRightSkill[1].BoxRect.left , sLeftRightSkill[1].BoxRect.top , //버튼 
				lpSkillBoxBack,0,0,49,46);

	}
	/*  인터페이스에 가리는 관계로 이동한다 
	if(ShowSkillClose){ //닫기 버튼 
		DrawSprite(269,571+(256-sinMoveKindInter[SIN_SKILL]),cShop.lpExit,0,0,20,20);
		DrawSprite(269,571-27+(256-sinMoveKindInter[SIN_SKILL]),cInvenTory.lpExitInfo,0 ,0 , 47,27);
	}
	*/

}
/*----------------------------------------------------------------------------*
*							     메인  
*-----------------------------------------------------------------------------*/	
void cSKILL::Main()
{

	int i = 0 , j = 0;
	int k = 0 , Num = 0;

	sSKILL  TempSkillData;
	CheckContinueIcon(); //스킬아이콘을 체크한다
	memset(TempContinueSkill,0,sizeof(sSKILL)*10);

	for(k = 0 ; k < MAX_CONTINUE_SKILL ; k++){      
		if(ContinueSkill[k].Flag){ 
			if(ContinueSkill[k].MatIcon){
				if(Num*32 > ContinueSkill[k].IconPosiX){
					ContinueSkill[k].IconPosiX +=2;
				}
				else if( Num*32 < ContinueSkill[k].IconPosiX){
					ContinueSkill[k].IconPosiX -=2;

				}
				if( ContinueSkill[k].CheckTime < 255){
					ContinueSkill[k].IconAlpha++;
				}
				if( ContinueSkill[k].CheckTime >= 255){ //로딩시
					ContinueSkill[k].IconAlpha = 255;

				}
				if(((ContinueSkill[k].UseTime*70) - ContinueSkill[k].CheckTime) < 180){
					ContinueSkill[k].IconAlpha--;
				}
				if(ContinueSkill[k].IconAlpha > 255)ContinueSkill[k].IconAlpha = 255;
				if(ContinueSkill[k].IconAlpha < 0 )ContinueSkill[k].IconAlpha = 0;

				if(ContinueSkill[k].CheckTime > (ContinueSkill[k].UseTime-4)*70 ){ 
					ContinueSkill[k].IconTime++;
					if(ContinueSkill[k].IconTime > 30){
						ContinueSkill[k].IconTime = 0;
						ContinueSkill[k].IconFlag ^=1;
					}
				}

				memcpy(&TempContinueSkill[Num],&ContinueSkill[k],sizeof(sSKILL));
				Num++;
				
			}
		} 
	}

	//소팅 
	int TempXPosi = 0;
	for(i = Num; i > 0; i--)    
	{
 		for(j = 0; j < i; j++)
		{
			if(TempContinueSkill[j].CheckTime < TempContinueSkill[j+1].CheckTime)
			{
				if(TempContinueSkill[j].CheckTime!=0){
					TempSkillData = TempContinueSkill[j+1];
					TempXPosi = TempContinueSkill[j+1].IconPosiX;
					TempContinueSkill[j+1] = TempContinueSkill[j];
					TempContinueSkill[j+1].IconPosiX = TempXPosi;
					TempXPosi = TempContinueSkill[j].IconPosiX;
					TempContinueSkill[j] = TempSkillData;
					TempContinueSkill[j].IconPosiX = TempXPosi;
				}
			}
		}
	}


	//버추얼라이프 시간을 체크한다
	if(AddVirtualLife[1]){ 
		CheckVirtualLifeTime++;
		if(CheckVirtualLifeTime >= sinVirtualLifeTime*70){
			CheckVirtualLifeTime = 0; //전부 초기화 
			AddVirtualLife[0] = 0;
			AddVirtualLife[1] = 0;
			sinVirtualLifePercent = 0;
		}
	}

	// 박재원 - 부스터 아이템(생명력)
	if(AddBoosterLife){ 
		CheckBoosterLifeTime++;
		if(CheckBoosterLifeTime >= BoosterLifeTime*70){
			CheckBoosterLifeTime = 0; 
			AddBoosterLife = 0;
			BoosterLifePercent = 0;
			tempLife[0] = 0;
			LifeFlag = 0;
		}
	}

	// 박재원 - 부스터 아이템(생명력)
	// 부스터 아이템(생명력)과 버추얼 라이프(프티 스킬) 중복 사용중 부스터 아이템 유지시간이 먼저 종료될 때
	if(AddVirtualLife[1] && !AddBoosterLife)
	{
		for( i=0 ; i < 10 ; i++)
		{
			if(ContinueSkill[i].Flag)
			{
				if(ContinueSkill[i].CODE == SKILL_VIRTUAL_LIFE)
				{
					AddVirtualLife[1]  = ((int)sinChar->Life[1] * Virtual_Life_Percent[ContinueSkill[i].Point-1])/100;
				}
			}
		}
	}

	// 박재원 - 부스터 아이템(기력)
	if(AddVirtualMana[1]){ 
		CheckVirtualManaTime++;
		if(CheckVirtualManaTime >= sinVirtualManaTime*70){
			CheckVirtualManaTime = 0; //전부 초기화 
			AddVirtualMana[0] = 0;
			AddVirtualMana[1] = 0;
			sinVirtualManaPercent = 0;
			tempMana[0] = 0;
			ManaFlag = 0;
		}
	}

	// 박재원 - 부스터 아이템(근력)
	if(AddVirtualStamina[1]){ 
		CheckVirtualStaminaTime++;
		if(CheckVirtualStaminaTime >= sinVirtualStaminaTime*70){
			CheckVirtualStaminaTime = 0; //전부 초기화 
			AddVirtualStamina[0] = 0;
			AddVirtualStamina[1] = 0;
			sinVirtualStaminaPercent = 0;
			tempStamina[0] = 0;
			StaminaFlag = 0;
		}
	}

	// 장별 - 스킬 딜레이
	if( sinAddSkillDelay )
	{
		CheckSkillDelayTime++;
		if( CheckSkillDelayTime >= sinSkillDelayTime*70 )
		{
			CheckSkillDelayTime = 0;
			sinAddSkillDelay = 0;
			sinSkillDelayTime = 0;
			SkillDelayFlag = 0;
		}
	}

	sinCheckContinueSkill(); //계속적으로 사용되는 스킬의 시간을 체크한다   
	CheckSkillData(); //스킬 데이타를 확인해서 나쁜놈을 척살한다 음하하하하 
	if( !ClearSkillCODE || !cSkill.OpenFlag || pCursorPos.y < 400){ //캐릭하나마다 한가지 스킬만 초기화할수있다
		ClearSkillCODE = 0;
		SkillInfoShowFlag = 0;
		
	}

	SkillMasteryShowFlag = 0;
	SelectInterSkill = 0;
	CheckingNowSkillState(); //현재 스킬상태를 검사한다  
	//CheckSkillMastery(); //마스터리를 체크한다 
	
	sinCheckChageJob(); //전업을 할수있는 지를 구해온다 
	
	ShowSkillClose = 0;    //닫기 버튼 플랙 
	SkillButtonIndex = 0;  //스킬 버튼 인덱스 

	int Space = 0;
	sSKILLBOX TempSkillBox;
	ContinueSkillMouseIndex = 0;
	for(i = 0 ; i < MAX_CONTINUE_SKILL ; i++){      
		if(TempContinueSkill[i].Flag){
			if(TempContinueSkill[i].MatIcon){
				if(10+(Space*32) < pCursorPos.x && 10+(Space*32)+32 > pCursorPos.x &&
					10 < pCursorPos.y && 10+32 > pCursorPos.y ){ 
						TempSkillBox.BoxRect.left = 10+(Space*32); 
						TempSkillBox.BoxRect.top = 10+32;
						SkillInfoShowFlag = 2;
						ContinueSkillMouseIndex = i+1;
						ShowSkillInfo(&TempContinueSkill[i],&TempSkillBox,1); //스킬의 정보를 보여준다
					}
				Space++; 
			}
		}
	}


	if ( 269 < pCursorPos.x  && 287 > pCursorPos.x  &&  571 < pCursorPos.y  && 590 > pCursorPos.y  )
		ShowSkillClose = 1;

	if(OpenFlag){
		for(int i = 0; i < ChangeJobSkillPlus ; i++){
			if(!SkillOnShownPage(i)) continue;
			if(sinSkill.UseSkill[i].Flag){
				if(sinSkill.SkillBox[i].BoxRect.left < pCursorPos.x && sinSkill.SkillBox[i].BoxRect.right > pCursorPos.x &&
					sinSkill.SkillBox[i].BoxRect.top < pCursorPos.y && sinSkill.SkillBox[i].BoxRect.bottom > pCursorPos.y ){
					SkillButtonIndex = i+1; 
					
					sinSkill.UseSkill[i].Position = i+1;
					ShowSkillInfo(&sinSkill.UseSkill[i],&sinSkill.SkillBox[i]); //스킬의 정보를 보여준다
					SkillInfoShowFlag = 1;
					///////초기화될 스킬을 찾는다 
					if(sinSkill.UseSkill[i].Point && SkillMasterFlag)
						SearchClearSkillPoint(sinSkill.UseSkill[i].CODE);
					//전업버튼을 비활성화한다
					//sinChangeJobButtonShow  = 0;
					//sinChangeJobButtonShow2 = 0;
					//sinChangeJobButtonShow3 = 0;

				}
				if(sinSkill.SkillBox[i].GageRect.left < pCursorPos.x && sinSkill.SkillBox[i].GageRect.right > pCursorPos.x &&
					sinSkill.SkillBox[i].GageRect.top < pCursorPos.y && sinSkill.SkillBox[i].GageRect.bottom > pCursorPos.y ){
					if(sinSkill.UseSkill[i].Skill_Info.USECODE != SIN_SKILL_USE_NOT){
						GetSkillMastery(&sinSkill.UseSkill[i],&sinSkill.SkillBox[i]); //스킬의 정보를 보여준다
						SkillMasteryShowFlag = 1;
					}
				}
			}
		}
	}
	for(i=0 ; i < 2 ; i ++){
		if(sLeftRightSkill[i].BoxRect.left < pCursorPos.x && sLeftRightSkill[i].BoxRect.right > pCursorPos.x &&
			sLeftRightSkill[i].BoxRect.top < pCursorPos.y && sLeftRightSkill[i].BoxRect.bottom > pCursorPos.y ){
			if(i == 0 && sinSkill.pLeftSkill){
				cHelpPet.PetMessage("*LeftSkill",0);
				ShowSkillInfo(sinSkill.pLeftSkill,&sLeftRightSkill[i]); //스킬의 정보를 보여준다
				SkillInfoShowFlag = 1;
				SelectInterSkill = 1;
			}
			if(i == 1 && sinSkill.pRightSkill){
				cHelpPet.PetMessage("*RightSkill",0);
				ShowSkillInfo(sinSkill.pRightSkill,&sLeftRightSkill[i]); //스킬의 정보를 보여준다
				SkillInfoShowFlag = 1;
				SelectInterSkill = 2;
			}

		}
	}
	//스킬 메인2==============================================================
	if(OpenFlag&&SkillMain2OpenFlag){
		//왼쪽
		if(12 < pCursorPos.x && 12+20 > pCursorPos.x &&
		   482 < pCursorPos.y && 482+20 > pCursorPos.y ){
				SkillMain2ButtonFlag[0] = 1;
		}
		else{
			    SkillMain2ButtonFlag[0] = 0;
		}

	}
	if(SkillMain2OpenFlag && sinChar->ChangeJob<4){	// no fifth rank (any more): back to the first page
		SkillMain2OpenFlag = 0;
		SkillMain2ButtonFlag[0] = 0;
	}
	if(OpenFlag&&!SkillMain2OpenFlag&&sinChar->ChangeJob>=4){	// the second page holds the tier 5 skills
		//오른쪽
		if(765 < pCursorPos.x && 765+20 > pCursorPos.x &&
		   482 < pCursorPos.y && 482+20 > pCursorPos.y ){
				SkillMain2ButtonFlag[1] = 1;
		}
		else{
				SkillMain2ButtonFlag[1] = 0;
		}
	}
	//============================================================================
}
/*----------------------------------------------------------------------------*
*							     종료 
*-----------------------------------------------------------------------------*/	
void cSKILL::Close()
{

}
/*----------------------------------------------------------------------------*
*							   LButtonDown
*-----------------------------------------------------------------------------*/	
void cSKILL::LButtonDown(int x , int y)
{

	for(int i=0 ; i < 2 ; i ++){
		if(sLeftRightSkill[i].BoxRect.left < pCursorPos.x && sLeftRightSkill[i].BoxRect.right > pCursorPos.x &&
			sLeftRightSkill[i].BoxRect.top < pCursorPos.y && sLeftRightSkill[i].BoxRect.bottom > pCursorPos.y ){
			if(i == 0 && sinSkill.pLeftSkill){
				cHelpPet.PetMessage("*LeftSkill",1);
			}
			if(i == 1 && sinSkill.pRightSkill){
				cHelpPet.PetMessage("*RightSkill",1);
			}

		}
	}

	//스킬 포인트를 내린다 
	if ( ClearSkillButtonPosi.x  < pCursorPos.x  && ClearSkillButtonPosi.x + 45 > pCursorPos.x  && 
		 ClearSkillButtonPosi.y <  pCursorPos.y  && ClearSkillButtonPosi.y + 45 > pCursorPos.y  ){
			 ClearSkillPointSection();

	}


	DownButtonUseShorKey = 0;
	LDownButtonIndex = 0;  //눌려진 버튼 인덱스 
	if(ShowSkillClose){    //스킬창 닫기 버튼 
		OpenFlag = 0;
	}

	if(SkillButtonIndex){
		if(sinSkill.UseSkill[SkillButtonIndex-1].Flag ){
			LDownButtonIndex = SkillButtonIndex;
		}
	}

	DownButtonIndex = 0;  //눌려진 버튼 인덱스 
	if(ShowSkillClose){    //스킬창 닫기 
		OpenFlag = 0;
	}
	if(SkillButtonIndex <=13 && SkillMasterFlag == 1  && sinSkill.SkillPoint){
		if(sinSkill.UseSkill[SkillButtonIndex-1].Flag && sinSkill.UseSkill[SkillButtonIndex-1].Use && sinSkill.UseSkill[SkillButtonIndex-1].Point < 10){
			DownButtonIndex = SkillButtonIndex;
			Skill4ButtonIndex = SkillButtonIndex;
		}
	}
	if(SkillButtonIndex>13 && SkillMasterFlag == 1  && sinSkill.SkillPoint4){
		if(sinSkill.UseSkill[SkillButtonIndex-1].Flag && sinSkill.UseSkill[SkillButtonIndex-1].Use && sinSkill.UseSkill[SkillButtonIndex-1].Point < 10){
			DownButtonIndex = SkillButtonIndex;
			Skill4ButtonIndex = SkillButtonIndex;
		}
	}
	
	//====================================================================================================
	if(SelectInterSkill){
		if(cSkill.OpenFlag == SIN_CLOSE)cSkill.OpenFlag = SIN_OPEN;
		else cSkill.OpenFlag = SIN_CLOSE;
		cInterFace.CheckAllBox(SIN_SKILL); //창을 하나만 띄운다 
		if(cSkill.OpenFlag){
			if(!sinFireShow){
				StartMenuFlame(0,350);
				sinPlaySound( 0 );
			}
			else
				sinPlaySound(SIN_SOUND_SHOW_INTER);
				sinFireShow = 1;
		}
	}

	//스킬 메인===============================================
	//오른쪽
	if(SkillMain2ButtonFlag[1]){
		SkillMain2ButtonFlag[1] = 0;
		SkillMain2OpenFlag=1;
		if(!lpSkillMain2)
			lpSkillMain2  = LoadDibSurfaceOffscreen("Image\\SinImage\\skill\\Skil25.bmp");
	}
	//왼쪽
	if(SkillMain2ButtonFlag[0]){
		SkillMain2ButtonFlag[0] = 0;
		SkillMain2OpenFlag=0;
	}
	//========================================================

}
/*----------------------------------------------------------------------------*
*							   LButtonUp
*-----------------------------------------------------------------------------*/	
void cSKILL::LButtonUp(int x , int y)
{

	DownButtonIndex  = 0;
	LDownButtonIndex = 0; //버튼 이미지 정상화 
	if(SkillButtonIndex  && !DownButtonUseShorKey){
		if((sinSkill.UseSkill[SkillButtonIndex-1].Skill_Info.USECODE == SIN_SKILL_USE_LEFT ||
			sinSkill.UseSkill[SkillButtonIndex-1].Skill_Info.USECODE == SIN_SKILL_USE_ALL) && !SkillMasterFlag
			&&sinSkill.UseSkill[SkillButtonIndex-1].Point )
			sinSkill.pLeftSkill = &sinSkill.UseSkill[SkillButtonIndex-1];
	
	}
//	if(SkillButtonIndex == 1 )  //노멀 공격 이면 
//		sinSkill.pLeftSkill = &sinSkill.UseSkill[SkillButtonIndex-1];

	//스킬을 분배한다 
	if(SkillButtonIndex != 1  && SkillMasterFlag == 1  ){
		//4차 스킬 
		if(SkillButtonIndex>13&& sinSkill.SkillPoint4 >0){
			if(sinSkill.UseSkill[SkillButtonIndex-1].Flag && sinSkill.UseSkill[SkillButtonIndex-1].Use){
				if(sinSkill.UseSkill[SkillButtonIndex-1].Point < MAX_USE_SKILL_POINT){ //최대 스킬 포인트 치를 못넘는다 
					///////////////여기서 메세지 박스를 띄워주고 돈을 지불하고 스킬을 올려준다 

					pMasterSkill = &sinSkill.UseSkill[SkillButtonIndex-1];
					cMessageBox.ShowMessage3(MESSAGE_MASTER_SKILL,sinSkill.UseSkill[SkillButtonIndex-1].Skill_Info.SkillName);
					ShowSkillUpInfo = 1;

				}
			}
		}
		//1.2.3차 스킬
		if(SkillButtonIndex<=13&&sinSkill.SkillPoint > 0){
			if(sinSkill.UseSkill[SkillButtonIndex-1].Flag && sinSkill.UseSkill[SkillButtonIndex-1].Use){
				if(sinSkill.UseSkill[SkillButtonIndex-1].Point < MAX_USE_SKILL_POINT){ //최대 스킬 포인트 치를 못넘는다 
					///////////////여기서 메세지 박스를 띄워주고 돈을 지불하고 스킬을 올려준다 

					pMasterSkill = &sinSkill.UseSkill[SkillButtonIndex-1];
					cMessageBox.ShowMessage3(MESSAGE_MASTER_SKILL,sinSkill.UseSkill[SkillButtonIndex-1].Skill_Info.SkillName);
					ShowSkillUpInfo = 1;

				}
			}
		}
	}
}
/*----------------------------------------------------------------------------*
*							   RButtonDown
*-----------------------------------------------------------------------------*/	
void cSKILL::RButtonDown(int x , int y)
{
	for(int i=0 ; i < 2 ; i ++){
		if(sLeftRightSkill[i].BoxRect.left < pCursorPos.x && sLeftRightSkill[i].BoxRect.right > pCursorPos.x &&
			sLeftRightSkill[i].BoxRect.top < pCursorPos.y && sLeftRightSkill[i].BoxRect.bottom > pCursorPos.y ){
			if(i == 0 && sinSkill.pLeftSkill){
				cHelpPet.PetMessage("*LeftSkill",2);
			}
			if(i == 1 && sinSkill.pRightSkill){
				cHelpPet.PetMessage("*RightSkill",2);
			}

		}
	}

	DownButtonUseShorKey = 0;
	if(SkillButtonIndex){
		if(sinSkill.UseSkill[SkillButtonIndex-1].Flag ){
			RDownButtonIndex = SkillButtonIndex;
		}
	}
}
/*----------------------------------------------------------------------------*
*							   RButtonUp
*-----------------------------------------------------------------------------*/	
void cSKILL::RButtonUp(int x , int y)
{
	RDownButtonIndex = 0;
	LDownButtonIndex = 0; //버튼 이미지 정상화 
	if(SkillButtonIndex && !DownButtonUseShorKey){
		if((sinSkill.UseSkill[SkillButtonIndex-1].Skill_Info.USECODE == SIN_SKILL_USE_RIGHT ||
			sinSkill.UseSkill[SkillButtonIndex-1].Skill_Info.USECODE == SIN_SKILL_USE_ALL )&& !SkillMasterFlag &&
			sinSkill.UseSkill[SkillButtonIndex-1].Point)
			sinSkill.pRightSkill = &sinSkill.UseSkill[SkillButtonIndex-1];
	}

}
/*----------------------------------------------------------------------------*
*							    KeyDown
*-----------------------------------------------------------------------------*/	
void cSKILL::KeyDown()
{
//	sSkill_Info[0].FuncPointer();

	
	int i=0;
	char szKeyBuff[8]={VK_F1,VK_F2,VK_F3,VK_F4,VK_F5,VK_F6,VK_F7,VK_F8}; //키버퍼 
	for(i=0; i < 8 ; i++){
		if(sinGetKeyClick(szKeyBuff[i])){
			if(SkillButtonIndex){ 
				//if(SkillButtonIndex == 1)break; //주먹키
				if(LDownButtonIndex){
					if(sinSkill.UseSkill[SkillButtonIndex-1].Skill_Info.USECODE == SIN_SKILL_USE_LEFT ||
						sinSkill.UseSkill[SkillButtonIndex-1].Skill_Info.USECODE == SIN_SKILL_USE_ALL &&
						sinSkill.UseSkill[SkillButtonIndex-1].Point&&!SkillMasterFlag){
						sinSkill.UseSkill[SkillButtonIndex-1].ShortKey = i+1;
						sinSkill.UseSkill[SkillButtonIndex-1].MousePosi = SIN_MOUSE_POSI_LEFT;
						DownButtonUseShorKey = 1;
					}
				}
				else{
					if(RDownButtonIndex){
						if(sinSkill.UseSkill[SkillButtonIndex-1].Skill_Info.USECODE == SIN_SKILL_USE_RIGHT ||
							sinSkill.UseSkill[SkillButtonIndex-1].Skill_Info.USECODE == SIN_SKILL_USE_ALL &&
							sinSkill.UseSkill[SkillButtonIndex-1].Point&&!SkillMasterFlag){
							sinSkill.UseSkill[SkillButtonIndex-1].ShortKey = i+1;
							sinSkill.UseSkill[SkillButtonIndex-1].MousePosi = SIN_MOUSE_POSI_RIGHT;
							DownButtonUseShorKey = 1;
						}
					}

				}
				
				for(int j=0; j < SIN_MAX_USE_SKILL ; j++){
					if(j == SkillButtonIndex-1)continue;
					if(sinSkill.UseSkill[SkillButtonIndex-1].ShortKey ==sinSkill.UseSkill[j].ShortKey){
						sinSkill.UseSkill[j].ShortKey = 0;
					}
				}
			}
			else{// 단축키로 스킬 사용
				for(int j=0; j < SIN_MAX_USE_SKILL ; j++){
					if(i ==sinSkill.UseSkill[j].ShortKey-1){
						if(sinSkill.UseSkill[j].Point){
							if(sinSkill.UseSkill[j].MousePosi == SIN_MOUSE_POSI_LEFT){ //왼쪽
								sinSkill.pLeftSkill = &sinSkill.UseSkill[j];
								
							}
							if(sinSkill.UseSkill[j].MousePosi == SIN_MOUSE_POSI_RIGHT){ //오른쪽 
								sinSkill.pRightSkill = &sinSkill.UseSkill[j];
							}
						}
					}
				}
			}
		}
	}
	if(sinGetKeyClick('S')){
		cHelpPet.PetMessage("*Skill",3);
		if(cSkill.OpenFlag)cSkill.OpenFlag = SIN_CLOSE;
		else cSkill.OpenFlag = SIN_OPEN;
		cInterFace.CheckAllBox(SIN_SKILL); //창을 하나만 띄운다 
		if(cSkill.OpenFlag){
			if(!sinFireShow){
				StartMenuFlame(0,350);
				sinPlaySound( 0 );
			}
			else
				sinPlaySound(SIN_SOUND_SHOW_INTER);
			sinFireShow = 1;
		}
	}

}


//사용할 스킬을 찾는다 
int cSKILL::SearchUseSkill()
{

	switch(sinChar->JOB_CODE){  
		case 1:
			UseSkillBox = GROUP_FIGHTER;

		break;
		case 2:
			UseSkillBox = GROUP_MECHANICIAN;

		break;
		case 3:
			UseSkillBox = GROUP_ARCHER;

		break;
		case 4:
			UseSkillBox = GROUP_PIKEMAN;

		break;
		case 5:
			UseSkillBox = GROUP_ATALANTA;
		break;
		case 6:
			UseSkillBox = GROUP_KNIGHT;
		break;
		case 7:
			UseSkillBox = GROUP_MAGICIAN;
		break;
		case 8:
			UseSkillBox = GROUP_PRIESTESS;
		break;

	}
	if(!UseSkillBox)
		return FALSE;

	int Index = 0;
	memcpy(&sinSkill.UseSkill[0],&sSkill[0],sizeof(sSKILL));
	sinSkill.UseSkill[0].Flag = 1; //노말 어택 default설정 
	sinSkill.UseSkill[0].Use  = 1; //노말 어택 default설정 
	sinSkill.UseSkill[0].Skill_Info.USECODE = SIN_SKILL_USE_ALL;  //노말 어택 강제설정 
	sinSkill.UseSkill[0].Point = 1;

	for(int i=0;i<SIN_MAX_SKILL;i++){
		if(	(sSkill[i].CODE & sinSKILL_MASK1) == UseSkillBox ){ //자신에 맞는 스킬을 찾는다 
			for(int j=1;j < SIN_MAX_USE_SKILL; j++){
				if(!sinSkill.UseSkill[j].Flag){
					memcpy(&sinSkill.UseSkill[j],&sSkill[i],sizeof(sSKILL));
					for(int k=0; k < SIN_MAX_SKILL ; k++){
						if(sinSkill.UseSkill[j].CODE == sSkill_Info[k].CODE){
							memcpy(&sinSkill.UseSkill[j].Skill_Info,&sSkill_Info[k],sizeof(sSKILL_INFO));
							sinSkill.UseSkill[j].Skill_Info.SkillNum = j-1;
							break;
						}
					}
					sinSkill.UseSkill[j].Flag = 1;
					
					////////////TEST 를 위해서 포인트1을 준다 
					//sinSkill.UseSkill[j].Point = 1;

					break;
				}
			}
		}
	}
	memcpy(&sinSkill.SkillBox,&sSkillBox,sizeof(sSKILLBOX)*SIN_MAX_USE_SKILL); //좌표를 복사한다 

	LoadUseSkillImage(); //스킬이미지를 로드한다 (전업을 할때두 로드해준다 )

	sinSkill.UseSkill[0].Use  = 1; //노말 어택 default설정 
	sinSkill.UseSkill[0].Skill_Info.USECODE = SIN_SKILL_USE_ALL;  //노말 어택 강제설정 
	sinSkill.UseSkill[0].Point = 1;
	ReFormCharSkillInfo(); //스킬정보를 갱신한다 (해킹방지)
	return FALSE;
}

void cSKILL::DrawSkillText()
{
	HDC	hdc;
	if(SkillInfoShowFlag || sinMoveKindInter[SIN_SKILL]){ //스킬  스테이터스가 닫혀있으면 리턴한다 

	int BoldFlag = 0;
	int MasteryLen = 0;
	int MasteryShowX = 0;

	char strBuff[128];
	memset(strBuff,0,sizeof(strBuff));
	lpDDSBack->GetDC( &hdc );
	SelectObject( hdc, sinFont);
    SetBkMode( hdc, TRANSPARENT );
	SetTextColor( hdc, RGB(255,255,255) );

	if(OpenFlag){
		
		if(sinSkill.SkillPoint >= 0|| sinSkill.SkillPoint4 >= 0){
			//1/2/3차 스킬포인트
			wsprintf(strBuff,"%d",sinSkill.SkillPoint); //현재 보유 잔여 포인트
			dsTextLineOut(hdc,CheckEditSize(165,253, strBuff),578+(256-sinMoveKindInter[SIN_SKILL]),
				strBuff,lstrlen(strBuff));
		
			//4차 스킬 잔여 포인트===================================================================
			wsprintf(strBuff,"%d",sinSkill.SkillPoint4);    //4차 스킬 잔여 포인트
			dsTextLineOut(hdc,CheckEditSize(250,253, strBuff),578+(256-sinMoveKindInter[SIN_SKILL]),
				strBuff,lstrlen(strBuff));
			//=======================================================================================
		}
		
		if(SkillMasteryShowFlag){
			MasteryLen = lstrlen(szUseSkillMastery);
			lstrcpy(strBuff,szUseSkillMastery);
			MasteryShowX = UseSkillMasteryPosi.x+(12/MasteryLen);
			if(MasteryLen <= 2)MasteryShowX +=4;
			dsTextLineOut(hdc,MasteryShowX,UseSkillMasteryPosi.y+6+(256-sinMoveKindInter[SIN_SKILL]),
				strBuff,lstrlen(strBuff));


		}
	}
	int SetTextXposi=0 , SetTextXposi2=0;
	char *pSkillInfo[40];
	char *pSkillInfo2[40];
	int len = 0 , len2 = 0;
	int Count = 0;
	int Textlen = 0 , Textlen2 = 0;
	int TempCnt = 0 , TempCnt2 = 0 ;

	char szSkillInfoBuffBack[5000];
	char szSkillInfoBuff2Back[5000];

	lstrcpy(szSkillInfoBuffBack,szSkillInfoBuff); 
	lstrcpy(szSkillInfoBuff2Back,szSkillInfoBuff2);

	len = lstrlen(szSkillInfoBuff);
	len2 = lstrlen(szSkillInfoBuff2);

	SetTextColor( hdc, RGB(255,255,255) );
	

	if(SkillInfoShowFlag && !ShowSkillUpInfo){  
		if(SkillInfoShowFlag == 2)
			dsDrawOffsetArray = dsARRAY_TOP;
		for(int i = 0 ; i < len ; i++){
			for(int j = 0; j < len2 ; j++){
				if(szSkillInfoBuffBack[i] == '\r'){
					if(szSkillInfoBuff2Back[j] == '\r'){  
						pSkillInfo[Count] = &szSkillInfoBuffBack[TempCnt];  
						pSkillInfo2[Count] = &szSkillInfoBuff2Back[TempCnt2];
						TempCnt = i+1;
						TempCnt2 = j+1;
						szSkillInfoBuffBack[i] = 0;
						szSkillInfoBuff2Back[j] = 0;

						Textlen = lstrlen(pSkillInfo[Count]);
						Textlen2 = lstrlen(pSkillInfo2[Count]);

						SetTextXposi = (SkillInfoBoxPosi.x + 120) - ((Textlen/2)*11);
						SetTextXposi2 = (SkillInfoBoxPosi.x + 120)+10;
						if(Textlen2 > 0){
							if(Textlen > 8 )
								SetTextXposi --;
							if(Textlen > 10)
								SetTextXposi -= 2;
						}
						if(Textlen2 < 1)
							SetTextXposi = (SkillInfoBoxPosi.x) +  (((SkillBoxSize.x*16)- ((Textlen/2)*11))/2)-10;

						//줄간격을 맞추기 위한 살짝 땡빵 -_-;;
						if(Textlen2 > 12 ){
							SetTextXposi -=12;
							SetTextXposi2 -=12;

						}
						SelectObject( hdc, sinFont);
						SetBkMode( hdc, TRANSPARENT );
						SetTextColor( hdc, RGB(239,190,109) );
						BoldFlag = 0;
						if(Count+1 == 1){
							SelectObject( hdc, sinBoldFont);
							SetBkMode( hdc, TRANSPARENT );
							SetTextColor( hdc, RGB(222,231,255) );
							dsTextLineOut(hdc,SetTextXposi,SkillInfoBoxPosi.y+27+((Count-1)*14), pSkillInfo[Count] , Textlen );
							BoldFlag = 1;

						}

						for(int t = 0 ; t < 6 ; t++){
							if(Count+1 == SkillDocLineCnt[t]){
								SetTextColor( hdc, RGB(255,255,255) );
							}
						}
						if(Count+1 == NextSkillLevelLine) //스킬 포인트 
							SetTextColor( hdc, RGB(255,220,0) );
	
						if(Count+1 == SkillUseWeaponClass) //사용가능 무기군 
							SetTextColor( hdc, RGB(164,199,41) );

						if(SkillMasterFlag == 1){  //상점이 열려있으면 가격을 표시한다 			
							if(Count == SkillInfoLineCnt-1)
								SetTextColor( hdc, RGB(247,243,193) ); //가격 
						}
						if( MonsterDamageLine && Count+1 == MonsterDamageLine){          
							SetTextColor( hdc, RGB(220,120,255) );
						}
						if(ChainDamageLine && Count+1 == ChainDamageLine){    
							SetTextColor( hdc, RGB(170,170,255) );

						}

						// +데미지는 색을 달리해준다 (하려고했느데 걍 관둘란다 ㅡ,.ㅡ )
						/*
						char szColorDamage[128];
						int count6 = 0 ;
						int CheckFlag2 = 0; 
						int ColorDamagePosi = 0;
						lstrcpy(szColorDamage,pSkillInfo2[Count]);
						int len5 = lstrlen(szColorDamage); 
						int p; for( p = 0 ; p < len5 ; p++){  
							count6++;
							if(szColorDamage[p] == '+' || CheckFlag2){
								if(szColorDamage[p] == ')'){
									CheckFlag2 = 0;
									continue;
								}
								SelectObject( hdc, sinBoldFont); 
								SetBkMode( hdc, TRANSPARENT );
								SetTextColor( hdc, RGB(255,255,0) );
								ColorDamagePosi = SetTextXposi2+(count6*5);
								dsTextLineOut(hdc,ColorDamagePosi,SkillInfoBoxPosi.y+28+((Count-1)*15), &szColorDamage[p] , 1 );
								CheckFlag2 = 1;
							}
						}
						*/
						if(!BoldFlag){
							dsTextLineOut(hdc,SetTextXposi,SkillInfoBoxPosi.y+28+((Count-1)*15), pSkillInfo[Count] , Textlen );
							dsTextLineOut(hdc,SetTextXposi2,SkillInfoBoxPosi.y+28+((Count-1)*15), pSkillInfo2[Count] , Textlen2 );
							dsTextLineOut(hdc,SetTextXposi2+1,SkillInfoBoxPosi.y+28+((Count-1)*15), pSkillInfo2[Count] , Textlen2 );
						}
						Count++;
						break;
					}
				}
			}
		}
	}

	//////////////스킬 정보를 다른곳에 보여준다 
	SetTextXposi=0;
	SetTextXposi2=0;

	Count = 0;
	Textlen = 0;
	Textlen2 = 0;
	TempCnt = 0 ;
	TempCnt2 = 0 ;
	char *pSkillMasterInfo[40];
	char *pSkillMasterInfo2[40];

	char TempSkillInfo[5000];
	char TempSkillInfo2[5000];


    SetBkMode( hdc, TRANSPARENT );
	SetTextColor( hdc, RGB(255,255,255) );
	lstrcpy(TempSkillInfo ,szSkillMasterInfoBuff);
	lstrcpy(TempSkillInfo2 ,szSkillMasterInfoBuff2);

	len = lstrlen(szSkillMasterInfoBuff);
	len2 = lstrlen(szSkillMasterInfoBuff2);

	int dsDrawOffsetArrayBackup = dsDrawOffsetArray;

	
	if(ShowSkillUpInfo){
		dsDrawOffsetArray = dsARRAY_TOP;
		for(int i = 0 ; i < len ; i++){
			for(int j=0; j < len2 ; j++){
				if(TempSkillInfo[i] == '\r'){
					if(TempSkillInfo2[j] == '\r'){
						pSkillMasterInfo[Count] = &TempSkillInfo[TempCnt];
						pSkillMasterInfo2[Count] = &TempSkillInfo2[TempCnt2];

						TempCnt = i+1;
						TempCnt2 = j+1;

						TempSkillInfo[i] = 0;
						TempSkillInfo2[j] = 0;
						Textlen = lstrlen(pSkillMasterInfo[Count]);
						Textlen2 = lstrlen(pSkillMasterInfo2[Count]);
						
						SetTextXposi = (SkillUpInfoPosi.x + 120) - ((Textlen/2)*11);
						SetTextXposi2 = (SkillUpInfoPosi.x + 120)+10;

						if(Textlen2 > 0){
							if(Textlen > 8 )
								SetTextXposi --;
							if(Textlen > 10)
								SetTextXposi -= 2;
						}
						if(Textlen2 < 1)
							SetTextXposi = (SkillUpInfoPosi.x) +  (((SkillUpInfo.x*16)- ((Textlen/2)*11))/2)-10;


						SelectObject( hdc, sinFont);
						SetBkMode( hdc, TRANSPARENT );
						//SetTextColor( hdc, RGB(255,255,255) );
						SetTextColor( hdc, RGB(239,190,109) );
				
						BoldFlag = 0;
						if(Count+1 == 1){
							SelectObject( hdc, sinBoldFont);
							SetBkMode( hdc, TRANSPARENT );
							SetTextColor( hdc, RGB(222,231,255) );
							dsTextLineOut(hdc,SetTextXposi,SkillUpInfoPosi.y+27+((Count-1)*14), pSkillMasterInfo[Count] , Textlen );
							BoldFlag = 1;

						}

						for(int t = 0 ; t < 6 ; t++){
							if(Count+1 == SkillDocLineCnt[t]){
								SetTextColor( hdc, RGB(255,255,255) );

							}
						}
						if(Count+1 == NextSkillLevelLine) //스킬 포인트 
							SetTextColor( hdc, RGB(255,220,0) );

						if(Count+1 == SkillUseWeaponClass) //사용가능 무기군 
							SetTextColor( hdc, RGB(164,199,41) );

						if(SkillMasterFlag == 1){  //상점이 열려있으면 가격을 표시한다 			
							if(Count == SkillInfoLineCnt-1)
								SetTextColor( hdc, RGB(247,243,193) ); //가격 
						}


						if(!BoldFlag){
							dsTextLineOut(hdc,SetTextXposi,SkillUpInfoPosi.y+28+((Count-1)*15), pSkillMasterInfo[Count] , Textlen );
							dsTextLineOut(hdc,SetTextXposi2,SkillUpInfoPosi.y+28+((Count-1)*15), pSkillMasterInfo2[Count] , Textlen2 );
							dsTextLineOut(hdc,SetTextXposi2+1,SkillUpInfoPosi.y+28+((Count-1)*15), pSkillMasterInfo2[Count] , Textlen2 );
						}
						Count++;
						break;
					}
				}
			}
		}
		dsDrawOffsetArray = dsDrawOffsetArrayBackup;
	}

	SelectObject( hdc, sinFont);
    SetBkMode( hdc, TRANSPARENT );
	SetTextColor( hdc, RGB(255,255,255) );



	/////////////////TEST
/*
	TempLenght = (float)(35/(sinSkill.UseSkill[j].Mastery/2));
			if(TempLenght <35){
				SkillCountTime[j]++;
				if(  SkillCountTime[j] >= 70){
					sinSkill.UseSkill[j].GageLength2 += TempLenght;
					SkillCountTime[j] = 0;
					if(sinSkill.UseSkill[j].GageLength2 < 35)

				}


			}
			else{
				sinSkill.UseSkill[j].GageLength2++;
				SkillCountTime2[j] +=2;

			}


*/
/*	
	for(int j=1;j < SIN_MAX_USE_SKILL; j++){
		if(sinSkill.UseSkill[j].Flag && sinSkill.UseSkill[j].Point){
			wsprintf(strBuff,"%d",sinSkill.UseSkill[j].UseSkillMastery); //현재 보유 잔여 포인트
			dsTextLineOut(hdc,100+(j*100),200,strBuff,lstrlen(strBuff));
			wsprintf(strBuff,"%d",sinSkill.UseSkill[j].Mastery); 
			dsTextLineOut(hdc,100+(j*100),200+100,strBuff,lstrlen(strBuff));


		}
	}
*/

	lpDDSBack->ReleaseDC( hdc );
	}

}

int JobSkillTemp = 0;
//현재 스킬의 상태를 검사한다 
void cSKILL::CheckingNowSkillState()
{

	//노말스킬을 포함한 4,4,4,4
	switch(sinChar->ChangeJob){
	case 0:
		ChangeJobSkillPlus = 5;
	break;
	case 1:
		ChangeJobSkillPlus = 9;
	break;
	case 2:
		ChangeJobSkillPlus = 13;
	break;
	case 3:
		ChangeJobSkillPlus = 17;
	break;
	default:					// the fifth rank: + the four tier 5 skills
		ChangeJobSkillPlus = 21;
	break;

	}
	if(JobSkillTemp != ChangeJobSkillPlus){	//전업데이타가 나중에 들어와서 날치기 땜빵으로 보정했다  음하하 
		JobSkillTemp = ChangeJobSkillPlus;
		cInvenTory.SetItemToChar();
	}

	for(int j = 0 ; j < ChangeJobSkillPlus ; j ++){  
		if(j>12){
			if(sinSkill.SkillPoint4){ //4차스킬 포인트가 있을경우에만 활성화 시킨다 
				sinSkill.UseSkill[j].Flag = 1; //사용할수있는 스킬을 추가한다 
				if(sinSkill.UseSkill[j].Point <= MAX_USE_SKILL_POINT){ 
					if(sinSkill.UseSkill[j-1].Point){//트리를 탄다 
						if(sinSkill.UseSkill[j].Skill_Info.RequireLevel+(sinSkill.UseSkill[j].Point*2) <= sinChar->Level) //스킬을 사용할수있는 레벨인지 판다 
							sinSkill.UseSkill[j].Use = 1;
						else
							sinSkill.UseSkill[j].Use = 0;
					}
				}
			}
			else{ //없을경우는 다시 비활성화 
				sinSkill.UseSkill[j].Use = 0;
			}
			continue;
		}
		if(sinSkill.SkillPoint){ //스킬 포인트가 있을경우에만 활성화 시킨다 
			sinSkill.UseSkill[j].Flag = 1; //사용할수있는 스킬을 추가한다 
			if(sinSkill.UseSkill[j].Point <= MAX_USE_SKILL_POINT){
				if(j==0)continue; //노말 스킬 
				if(sinSkill.UseSkill[j-1].Point){//트리를 탄다 
					if(sinSkill.UseSkill[j].Skill_Info.RequireLevel+(sinSkill.UseSkill[j].Point*2) <= sinChar->Level) //스킬을 사용할수있는 레벨인지 판다 
						sinSkill.UseSkill[j].Use = 1;
					else
						sinSkill.UseSkill[j].Use = 0;
				}
			}
		}
		else{ //없을경우는 다시 비활성화 
				sinSkill.UseSkill[j].Use = 0;
		}
	}
}



//마스터리를 조정한다 
void cSKILL::CheckSkillMastery()
{
	
	int TempLenght = 0; 
	
	int TempTalent = 0;
	

	//양쪽 스킬들을 체크한다 
	if(sinSkill.pLeftSkill){
		if(!sinSkill.pLeftSkill->Point)
			sinSkill.pLeftSkill->UseSkillFlag = 0;
	}
	if(sinSkill.pRightSkill){
		if(!sinSkill.pRightSkill->Point)
			sinSkill.pRightSkill->UseSkillFlag = 0;
	}

	for(int j=1;j < SIN_MAX_USE_SKILL; j++){ 
		if(sinSkill.UseSkill[j].Flag && sinSkill.UseSkill[j].Point ){

			CheckSkillMasteryForm(0,j); //스킬 마스터리를 체크한다

			if(sinSkill.UseSkill[j].Mastery == 0){
				sinSkill.UseSkill[j].GageLength = 0; //스킬 사용시
				sinSkill.UseSkill[j].GageLength2 = 0; //스킬 사용시
				SkillCountTime2[j] = 0;

			}

			//사용시 올라가는 숙련도 
			TempTalent = (int)(sinChar->Talent/3)+(int)sinAdd_fMagic_Mastery; 
			if(TempTalent > 50 )TempTalent = 50;
			sinSkill.UseSkill[j].UseSkillMastery =  (int)(TempTalent*100)+sinSkill.UseSkill[j].UseSkillCount;
			if(sinSkill.UseSkill[j].Skill_Info.Element[0]){ //4차스킬은 숙련치를 없애준다
				sinSkill.UseSkill[j].UseSkillMastery = 10000;

			}

			if(sinSkill.UseSkill[j].UseSkillMastery >= 10000)sinSkill.UseSkill[j].UseSkillMastery = 10000;
			if(sinSkill.UseSkill[j].UseSkillMastery){
				sinSkill.UseSkill[j].UseSkillMasteryGage = (int)((35*((float)sinSkill.UseSkill[j].UseSkillMastery/10000)));
				if(sinSkill.UseSkill[j].UseSkillMasteryGage > 35 )sinSkill.UseSkill[j].UseSkillMasteryGage = 35;
			}
			
			sinSkill.UseSkill[j].Mastery =   (sinSkill.UseSkill[j].Skill_Info.RequireMastery[0]+ (sinSkill.UseSkill[j].Skill_Info.RequireMastery[1]*sinSkill.UseSkill[j].Point) - 
											(sinSkill.UseSkill[j].UseSkillMastery/100));
			if(sinSkill.UseSkill[j].Mastery <= 0) sinSkill.UseSkill[j].Mastery = 1; //최소값 0.5초 
			if(sinSkill.UseSkill[j].Mastery > 70)sinSkill.UseSkill[j].Mastery = 70;

			// 장별 - 스킬 딜레이
			if( sinAddSkillDelay )
			{
				sinSkill.UseSkill[j].Mastery = 1;
			}

			else
			{
				sinSkill.UseSkill[j].Mastery =   (sinSkill.UseSkill[j].Skill_Info.RequireMastery[0]+ (sinSkill.UseSkill[j].Skill_Info.RequireMastery[1]*sinSkill.UseSkill[j].Point) - 
											(sinSkill.UseSkill[j].UseSkillMastery/100));

				if(sinSkill.UseSkill[j].Mastery <= 0) sinSkill.UseSkill[j].Mastery = 1; //최소값 0.5초 
				if(sinSkill.UseSkill[j].Mastery > 70)sinSkill.UseSkill[j].Mastery = 70;
			}

			//딜레이 값 
			TempLenght = (int)(35/((float)sinSkill.UseSkill[j].Mastery/2)); 

			sinSkill.UseSkill[j].GageLength2 += TempLenght;

			if(sinSkill.UseSkill[j].GageLength < sinSkill.UseSkill[j].GageLength2){
				SkillCountTime2[j]++;
				if(SkillCountTime2[j] >= (int)(35/(float)TempLenght)){
					sinSkill.UseSkill[j].GageLength++;
					SkillCountTime2[j] = 0;
			
				}

			}

			if(sinSkill.UseSkill[j].GageLength >=35){
				sinSkill.UseSkill[j].GageLength = 35;
				sinSkill.UseSkill[j].GageLength2 = 35;
				sinSkill.UseSkill[j].UseSkillFlag = 1;
				CheckUseSkillState(&sinSkill.UseSkill[j]); //스킬을 사용할수있는지를 체크한다 
				

			}
			else
				sinSkill.UseSkill[j].UseSkillFlag = 0;

			ReformSkillMasteryForm(0,j); //데이타를 갱신한다
				
		}
	
	}
	
	
}

DWORD UseSkillItemInfo[10] = {0,sinWA1,sinWM1,sinWH1,sinDS1,sinWP1,sinWS2,sinWC1,sinWS1,sinWT1};



    
//스킬의 정보를 보여준다 
int cSKILL::ShowSkillInfo(sSKILL *pSkill , sSKILLBOX *pSkillBox ,  int Icon )
{
	
	if(ShowSkillUpInfo)return TRUE;
	
	int PlusMinD = 0;
	int PlusMaxD = 0;

	int DivideMin = 8;
	int DivideMax = 4;

	int sinTemp1 = 0;
	int sinTemp2 = 0;
	int LeftSpot = 0;
	int RightSpot = 0;

	int TempPoint = 0;

	int i=0;
	int j=0;

	//
	int TempLife=0,TempMana=0;

	

	int LineCount=0;
	int p; for( p =0;p<6;p++)
		SkillDocLineCnt[p] = 0;
	for(p=0;p<10;p++)
		SkillIconIndex[p] = 0;
	int CopyCount=0;
	int	SkillDoclen=0;
	NextSkillLevelLine = 0;   //다음 레벨 Text라인 
	SkillUseWeaponClass = 0;  //사용가능 아이템 Text 라인 
	SkillInconCnt = 0;
	char szTempSkillDoc[256];
	char szTempSkillDoc2[256];

	char *szTemp = "\r";
	memset(&szTempSkillDoc,0,sizeof(szTempSkillDoc));
	memset(&szTempSkillDoc2,0,sizeof(szTempSkillDoc2));
	memset(&szSkillInfoBuff,0,sizeof(szSkillInfoBuff)); //버퍼 초기화 
	memset(&szSkillInfoBuff2,0,sizeof(szSkillInfoBuff2)); //버퍼 초기화 

	////////////기본 공격 
	if(pSkill->CODE == SKILL_NORMAL_ATTACK){ 
		wsprintf(szSkillInfoBuff,"%s\r",NormalAttckName); 
		lstrcpy(szTempSkillDoc2,"\r");
		lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
		LineCount++;
		GetInfoBoxSize(pSkillBox,LineCount);
		return TRUE;

	}
	
	
	if(pSkill->Point){     
		if(pSkill->CODE == SKILL_FORCE_ORB || pSkill->CODE == CHANGE_JOB3_QUEST || pSkill->CODE == CHANGE_ELEMENTARY_QUEST){
			wsprintf(szSkillInfoBuff,"%s\r",pSkill->Skill_Info.SkillName); //스킬 이름 
		}
		else{
			wsprintf(szSkillInfoBuff,"%s LV:%d\r",pSkill->Skill_Info.SkillName,pSkill->Point); //스킬 이름 
		}
		lstrcpy(szTempSkillDoc2,"\r");
		lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
		LineCount++;

		if(pSkill->Point < 10){
			if(!Icon){
				wsprintf(szTempSkillDoc2,RequirLevel3,pSkill->Skill_Info.RequireLevel+(pSkill->Point*2)); //스킬 이름 
				lstrcat(szSkillInfoBuff,szTempSkillDoc2);
				lstrcpy(szTempSkillDoc2,"\r");
				lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
				LineCount++;
			}
		}

	}
	else{
		if(pSkill->CODE == CLANSKILL_ABSORB       || pSkill->CODE == CLANSKILL_EVASION   || pSkill->CODE == CLANSKILL_ATTACK||
		   pSkill->CODE == SCROLL_INVULNERABILITY || pSkill->CODE == SCROLL_CRITICAL     ||pSkill->CODE == SCROLL_EVASION||
		   pSkill->CODE == STONE_R_FIRECRYTAL     || pSkill->CODE == STONE_R_ICECRYTAL   ||pSkill->CODE == STONE_R_LINGHTINGCRYTAL ||
		   pSkill->CODE == STONE_A_FIGHTER        || pSkill->CODE == STONE_A_MECHANICIAN ||pSkill->CODE == STONE_A_PIKEMAN         ||pSkill->CODE == STONE_A_ARCHER ||
		   pSkill->CODE == STONE_A_KNIGHT         || pSkill->CODE == STONE_A_ATALANTA    ||pSkill->CODE == STONE_A_MAGICIAN        ||pSkill->CODE == STONE_A_PRIESTESS||
		   pSkill->CODE == SCROLL_P_INVULNERABILITY || pSkill->CODE == SCROLL_P_CRITICAL ||pSkill->CODE == SCROLL_P_EVASION  ||
		   pSkill->CODE == BOOSTER_ITEM_LIFE	  || pSkill->CODE == BOOSTER_ITEM_MANA	 ||pSkill->CODE == BOOSTER_ITEM_STAMINA ) // 박재원 - 부스터 아이템
		{
			wsprintf(szSkillInfoBuff,"%s\r",pSkill->Skill_Info.SkillName); //스킬 이름 
		}
		else{
			wsprintf(szSkillInfoBuff,RequirLevel,pSkill->Skill_Info.SkillName,pSkill->Skill_Info.RequireLevel+(pSkill->Point*2)); //스킬 이름 
		}
		lstrcpy(szTempSkillDoc2,"\r");
		lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
		LineCount++;

	}
	
	SkillDoclen = lstrlen(pSkill->Skill_Info.SkillDoc); //길이를 구한다 

	BYTE ch;
	int StrCnt;
	int cnt,cnt2;


	cnt2 = 0;
	StrCnt = 0;
	for( cnt=0;cnt<SkillDoclen;cnt++ ) {
		ch = (BYTE)pSkill->Skill_Info.SkillDoc[cnt];

		if ( ch>=0x80 ) {
			szTempSkillDoc[cnt2++] = pSkill->Skill_Info.SkillDoc[cnt++];
			szTempSkillDoc[cnt2++] = pSkill->Skill_Info.SkillDoc[cnt];
			StrCnt+=2;
		}
		else {
			szTempSkillDoc[cnt2++] = pSkill->Skill_Info.SkillDoc[cnt];
			StrCnt++;
		}
#ifdef	_LANGUAGE_JAPANESE      //일본어 Win98에서의 폰트문제로 약간 땜빵 
		if ( StrCnt>26 ) {
			StrCnt = 0;
			szTempSkillDoc[cnt2++] = '\r';
			lstrcpy(szTempSkillDoc2,"\r");
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;
			SkillDocLineCnt[LineCount-2] = LineCount;
		}

#else
		if ( StrCnt>30 ) {
			StrCnt = 0;
			szTempSkillDoc[cnt2++] = '\r';
			lstrcpy(szTempSkillDoc2,"\r");
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;
			SkillDocLineCnt[LineCount-2] = LineCount;
		}
#endif
	}
	SkillDocLineCnt[LineCount+1-2] = LineCount+1; //라인 보정 


	lstrcat(szSkillInfoBuff,szTempSkillDoc);
	lstrcat(szSkillInfoBuff,szTemp);   
	lstrcpy(szTempSkillDoc2,"\r");
	lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
	LineCount++;
	// 장별 - 초 -> 한국어 들어가 있는 것 수정
	if( pSkill->CODE == SCROLL_P_CRITICAL || pSkill->CODE == SCROLL_P_EVASION )		// pluto 페이틀 스크롤 어버트 스크롤 시간 표시
	{
		lstrcpy(szTempSkillDoc,CountinueTime);
		lstrcat(szSkillInfoBuff,szTempSkillDoc);
		wsprintf(szTempSkillDoc2,"%d%s\r", 3600, SecName );
		lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
		LineCount++;
	}

	else if(pSkill->CODE == BOOSTER_ITEM_LIFE || pSkill->CODE == BOOSTER_ITEM_MANA || pSkill->CODE == BOOSTER_ITEM_STAMINA) 
	{// 박재원 - 부스터 아이템 시간 표시
		lstrcpy(szTempSkillDoc,CountinueTime);
		lstrcat(szSkillInfoBuff,szTempSkillDoc);
		wsprintf(szTempSkillDoc2,"%d%s\r",pSkill->UseTime,SecName);
		lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
		LineCount++;
	}

	lstrcat(szSkillInfoBuff,"\r");   
	lstrcpy(szTempSkillDoc2,"\r");
	lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
	LineCount++;

	

	if(pSkill->Skill_Info.UseWeaponCode[0]){ //특정 무기만 사용될경우  
		lstrcpy(szTempSkillDoc,UseItemGroupName);
		lstrcat(szSkillInfoBuff,szTempSkillDoc);
		lstrcpy(szTempSkillDoc2,"\r");
		lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
		LineCount++;
		SkillUseWeaponClass = LineCount;
		for(i=0;i<8;i++){
			if(pSkill->Skill_Info.UseWeaponCode[i]){
				for(int j=0; j < 10 ; j++){
					if(pSkill->Skill_Info.UseWeaponCode[i] == UseSkillItemInfo[j]){
						SkillIconIndex[i] = j;
						SkillIconPosi.y = (LineCount*17)+2;
						SkillInconCnt++;
						
					}
				}
			}
		}
	}
	if(SkillInconCnt){ 
		SkillIconPosi.x = ((14*16) - (SkillInconCnt*18))/2;
		lstrcat(szSkillInfoBuff,"\r"); 
		lstrcpy(szTempSkillDoc2,"\r");
		lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
		LineCount++;
		lstrcat(szSkillInfoBuff,"\r"); 
		lstrcpy(szTempSkillDoc2,"\r");
		lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
		LineCount++;

	}

	//몬스터에 추가데미지 출력
	MonsterDamageLine = 0;
	ChainDamageLine = 0;

	switch(pSkill->CODE){   
		case SKILL_MECHANIC_BOMB:
			wsprintf(szTempSkillDoc,"(%s.%s VS %d%s %s",Mutant7,Mechanic7,50,"%",MonsterAddDamage2); //스킬 이름 
			lstrcpy(szTempSkillDoc2,"\r");
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;
			MonsterDamageLine = LineCount;
		break;
		case SKILL_SPARK:
			wsprintf(szTempSkillDoc,"(%s VS %d%s %s",Mechanic7,50,"%",MonsterAddDamage2); //스킬 이름 
			lstrcpy(szTempSkillDoc2,"\r");
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;
			MonsterDamageLine = LineCount;
		break;
		case SKILL_BRUTAL_SWING:
			wsprintf(szTempSkillDoc,"(%s VS %d%s %s",Demon7,40,"%",MonsterAddDamage2); //스킬 이름 
			lstrcpy(szTempSkillDoc2,"\r");
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;
			MonsterDamageLine = LineCount;
		break;
		case SKILL_JUMPING_CRASH:
			wsprintf(szTempSkillDoc,"(%s VS %d%s %s",Demon7,30,"%",MonsterAddDamage2); //스킬 이름 
			lstrcpy(szTempSkillDoc2,"\r");
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;
			MonsterDamageLine = LineCount;
		break;
		case SKILL_EXPANSION:
			wsprintf(szTempSkillDoc,"(%s VS %d%s %s",Mutant7,30,"%",MonsterAddDamage2); //스킬 이름 
			lstrcpy(szTempSkillDoc2,"\r");
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;
			MonsterDamageLine = LineCount;
		break;
		case SKILL_SWORD_BLAST:
			wsprintf(szTempSkillDoc,"(%s.%s VS %d%s %s",Nomal7,Mutant7,30,"%",MonsterAddDamage2); //스킬 이름 
			lstrcpy(szTempSkillDoc2,"\r");
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;
			MonsterDamageLine = LineCount;
		break;
		case SKILL_PIERCING:
			wsprintf(szTempSkillDoc,"(%s VS %d%s %s",Mutant7,30,"%",MonsterAddDamage2); //스킬 이름 
			lstrcpy(szTempSkillDoc2,"\r");
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;
			MonsterDamageLine = LineCount;
		break;
		case SKILL_TWIST_JAVELIN:
			wsprintf(szTempSkillDoc,"(%s VS %d%s %s",Demon7,30,"%",MonsterAddDamage2); //스킬 이름 
			lstrcpy(szTempSkillDoc2,"\r");
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;
			MonsterDamageLine = LineCount;
		break;
		case SKILL_FIRE_JAVELIN:
			wsprintf(szTempSkillDoc,"(%s VS %d%s %s",Mutant7,50,"%",MonsterAddDamageFire); //스킬 이름 
			lstrcpy(szTempSkillDoc2,"\r");
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;
			MonsterDamageLine = LineCount;
		break;
		case SKILL_WIND_ARROW:
			wsprintf(szTempSkillDoc,"(%s VS %d%s %s",Nomal7,30,"%",MonsterAddDamage2); //스킬 이름 
			lstrcpy(szTempSkillDoc2,"\r");
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;
			MonsterDamageLine = LineCount;
		break;
		case SKILL_PERFECT_AIM:
			wsprintf(szTempSkillDoc,"(%s VS %d%s %s",Demon7,30,"%",MonsterAddDamage2); //스킬 이름 
			lstrcpy(szTempSkillDoc2,"\r");
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;
			MonsterDamageLine = LineCount;
		break;
		case SKILL_METAL_ARMOR:
			//흡수량 인계
			wsprintf(szTempSkillDoc,"(%s %d%s %s",PhysicalAbsorb3,120,"%",ChainDamage3); //스킬 이름 
			lstrcpy(szTempSkillDoc2,"\r");
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;
			ChainDamageLine = LineCount;
		break;
		case SKILL_SPARK_SHIELD:
			wsprintf(szTempSkillDoc,"(%s VS %d%s %s",Mechanic7,50,"%",MonsterAddDamage2); //스킬 이름  
			lstrcpy(szTempSkillDoc2,"\r");
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;
			MonsterDamageLine = LineCount;

			//데미지 인계
			/*
			wsprintf(szTempSkillDoc,"(%s %d%s %s",SparkDamage3,Spark_Shield_Damage[pSkill->Point-1],"%",ChainDamage3); //스킬 이름 
			lstrcpy(szTempSkillDoc2,"\r");
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;
			ChainDamageLine = LineCount;
			*/

		break;
		case SKILL_AVANGING_CRASH:
			//크리티컬 인계
			wsprintf(szTempSkillDoc,"(%s %d%s %s",BrutalSwingCritical3,50,"%",ChainDamage3); //스킬 이름 
			lstrcpy(szTempSkillDoc2,"\r");
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;
			ChainDamageLine = LineCount;
		break;
		case SKILL_LIGHTNING_JAVELIN:
			wsprintf(szTempSkillDoc,"(%s VS %d%s %s",Undead7,50,"%",MonsterAddDamage2); //스킬 이름  
			lstrcpy(szTempSkillDoc2,"\r");
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;
			MonsterDamageLine = LineCount;
		break;
		case SKILL_IMPULSION:     //메카닉 몹에 30%가중
			wsprintf(szTempSkillDoc,"(%s VS %d%s %s",Mechanic7,30,"%",MonsterAddDamage2); //스킬 이름 
			lstrcpy(szTempSkillDoc2,"\r");
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;
			MonsterDamageLine = LineCount;
		break;
		case SKILL_MAGNETIC_SPHERE: //메카닉 몹에 30%가중
			wsprintf(szTempSkillDoc,"(%s VS %d%s %s",Mechanic7,30,"%",MonsterAddDamage2); //스킬 이름 
			lstrcpy(szTempSkillDoc2,"\r");
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;
			MonsterDamageLine = LineCount;
		break;
		case SKILL_DETORYER:       //디먼형 몹에 30%가중
			wsprintf(szTempSkillDoc,"(%s VS %d%s %s",Demon7,30,"%",MonsterAddDamage2); //스킬 이름 
			lstrcpy(szTempSkillDoc2,"\r");
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;
			MonsterDamageLine = LineCount;
        break;
		case SKILL_HALL_OF_VALHALLA: //Triumph100%인계
			wsprintf(szTempSkillDoc,"(%s %d%s %s",Triumph4,100,"%",ChainDamage3); //스킬 이름 
			lstrcpy(szTempSkillDoc2,"\r");
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;
			ChainDamageLine = LineCount;
		break;
		case SKILL_GODLY_SHIELD:    //디바인 쉴드 블록율 100%인계
			wsprintf(szTempSkillDoc,"(%s %d%s %s",DivineShield4,100,"%",ChainDamage3); //스킬 이름 
			lstrcpy(szTempSkillDoc2,"\r");
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;
			ChainDamageLine = LineCount;
		break;
	}


	for(i=0;i<2;i++){
		if(i == 1 && Icon )break;
		if(i==1 && pSkill->Point == MAX_USE_SKILL_POINT)break;
		if(i==0 && !pSkill->Point)continue;
		if(i==1 ){
			lstrcat(szSkillInfoBuff,NextLevelText); 
			lstrcpy(szTempSkillDoc2,"\r");
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;
			NextSkillLevelLine = LineCount;
		}

		// 장별 - 초 -> 한국어 들어가 있는 것 수정
		switch(pSkill->CODE){
		/////////////////////메카니션 
		case SKILL_EXTREME_SHIELD:	
			lstrcpy(szTempSkillDoc,IncreBlock);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d%s\r",E_Shield_BlockRate[pSkill->Point+i-1],"%");
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;
			
			lstrcpy(szTempSkillDoc,CountinueTime);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d%s\r",E_Shield_UseTime[pSkill->Point+i-1],SecName);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;
		break;

		case SKILL_MECHANIC_BOMB:
			lstrcpy(szTempSkillDoc,DamageName);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d-%d\r",Mechanic_Bomb_Damage[pSkill->Point+i-1][0],Mechanic_Bomb_Damage[pSkill->Point+i-1][1]);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

			lstrcpy(szTempSkillDoc,ShootingRangeName);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d\r",Mechanic_Bomb_Attack_Range[pSkill->Point+i-1]);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;
			break;

		case SKILL_PHYSICAL_ABSORB:
			lstrcpy(szTempSkillDoc,IncreAsorb);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d-%d\r",P_Absorb[pSkill->Point+i-1][0],P_Absorb[pSkill->Point+i-1][1]);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

			lstrcpy(szTempSkillDoc,CountinueTime);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d%s\r",P_Absorb_UseTime[pSkill->Point+i-1],SecName);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;
			break;
		case SKILL_POISON_ATTRIBUTE:
			lstrcpy(szTempSkillDoc,PoisonName);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d\r",PlusPoison[pSkill->Point+i-1]);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

			break;

		case SKILL_GREAT_SMASH:
			lstrcpy(szTempSkillDoc,IncreAttack_RateName);   
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d\r",G_Smash_Attack_Rate[pSkill->Point+i-1]);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;
			
			lstrcpy(szTempSkillDoc,AddMaxDamage); 
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d-%d\r",G_Smash_Attack_Rage[pSkill->Point+i-1],G_Smash_Attack_Rage[pSkill->Point+i-1]+sinChar->Strength/5);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;
			
			lstrcpy(szTempSkillDoc,DamageAddName); 
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d\r",G_Smash_Attack_Rage[pSkill->Point+i-1]);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

			break;

			
		case SKILL_MAXIMIZE:
			lstrcpy(szTempSkillDoc,Max_DamageIncre);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d%s\r",Maximize_Damage[pSkill->Point+i-1],"%");
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

			lstrcpy(szTempSkillDoc,CountinueTime);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d%s\r",Maximize_Time[pSkill->Point+i-1],SecName );
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;
			break;

		case SKILL_AUTOMATION:
			lstrcpy(szTempSkillDoc,DamageAddName);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d%s\r",Automation_Damage[pSkill->Point+i-1],"%");
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

			lstrcpy(szTempSkillDoc,Attack_SpeedAdd);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d\r",Automation_Speed[pSkill->Point+i-1]);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

			lstrcpy(szTempSkillDoc,CountinueTime);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d%s\r",Automation_Time[pSkill->Point+i-1],SecName );
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;
			break;
		

		case SKILL_SPARK:
			lstrcpy(szTempSkillDoc,DamageName);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d-%d+(LV/5)\r",Spark_Damage[pSkill->Point+i-1][0],Spark_Damage[pSkill->Point+i-1][1]);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

			lstrcpy(szTempSkillDoc,AttackAreaName);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d\r",Spark_Area[pSkill->Point+i-1]);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;
			break;

		case SKILL_METAL_ARMOR:

			lstrcpy(szTempSkillDoc,AddDefense8);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d%s\r",Metal_Armor_Defense[pSkill->Point+i-1],"%");
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

			lstrcpy(szTempSkillDoc,CountinueTime);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d%s\r",Metal_Armor_Time[pSkill->Point+i-1],SecName );
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;


			break;
		case SKILL_GRAND_SMASH:

			lstrcpy(szTempSkillDoc,IncreAttack_RateName);  
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d\r",Grand_Smash_AttackRate[pSkill->Point+i-1]);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

			lstrcpy(szTempSkillDoc,DamageAddName); 
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d%s\r",Grand_Smash_Damage[pSkill->Point+i-1],"%");
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

			break;
		case SKILL_MECHANIC_WEAPON:
			lstrcpy(szTempSkillDoc,DamageAddName);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d%s\r",M_Weapon_Mastey[pSkill->Point+i-1],"%");
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;
			//임의대로 한줄을 더 넣어준다 
			lstrcat(szSkillInfoBuff,"\r"); 
			lstrcpy(szTempSkillDoc2,"\r");
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

			break;
		case SKILL_SPARK_SHIELD:

			/*
			lstrcpy(szTempSkillDoc,LightningDamage2);  
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%s%d%s\r",SparkDamage3,Spark_Shield_Damage[pSkill->Point+i-1],"%");
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;
			*/

			lstrcpy(szTempSkillDoc,SparkDamage10); 
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d%s\r",Spark_Shield_Damage[pSkill->Point+i-1],"%");
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;


			lstrcpy(szTempSkillDoc,SheildDefense); 
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d\r",Spark_Shield_Defense[pSkill->Point+i-1]);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

			lstrcpy(szTempSkillDoc,CountinueTime);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d%s\r",Spark_Shield_Time[pSkill->Point+i-1],SecName );
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

			break;

		/////////////////////파이터 
		case SKILL_MELEE_MASTERY: 
			lstrcpy(szTempSkillDoc,DamageAddName);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d%s\r",Melee_Mastery_DamagePercent[pSkill->Point+i-1],"%");
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;
			//임의대로 한줄을 더 넣어준다 
			lstrcat(szSkillInfoBuff,"\r"); 
			lstrcpy(szTempSkillDoc2,"\r");
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

			break;
		case SKILL_FIRE_ATTRIBUTE:
			lstrcpy(szTempSkillDoc,FireName);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d\r",PlusFire[pSkill->Point+i-1]);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

			break;
		case SKILL_RAVING:
			lstrcpy(szTempSkillDoc,DamageAddName);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d%s\r",Raving_Damage[pSkill->Point+i-1],"%");
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

			lstrcpy(szTempSkillDoc,HitNumName);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d\r",Ravind_Speed[pSkill->Point+i-1]);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

			lstrcpy(szTempSkillDoc,DecreLifeName);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);

			sinTemp1 = (int)(Raving_UseLife[pSkill->Point+i-1]*10);
			RightSpot = sinTemp1 %10;
			LeftSpot  = (sinTemp1-RightSpot)/10;

			wsprintf(szTempSkillDoc2,"%d.%d%s\r",LeftSpot,RightSpot,"%");
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;
			break;
		case SKILL_IMPACT:
			lstrcpy(szTempSkillDoc,Attck_RateAdd);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d\r",Impact_Attack_Rating[pSkill->Point+i-1]);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

			lstrcpy(szTempSkillDoc,DamageAddName);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d%s\r",Impact_Damage[pSkill->Point+i-1],"%");
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;
			break;

		case SKILL_TRIPLE_IMPACT:
			lstrcpy(szTempSkillDoc,DamageAddName);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d%s\r",T_Impact_Damage[pSkill->Point+i-1],"%");
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

			lstrcpy(szTempSkillDoc,HitNumName);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d\r",T_Impact_Hit[pSkill->Point+i-1]);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;
			break;

		case SKILL_BRUTAL_SWING:
			lstrcpy(szTempSkillDoc,DamageAddName);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d%s\r",B_Swing_Damage[pSkill->Point+i-1],"%");
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

			lstrcpy(szTempSkillDoc,Add_CriticalName);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d%s\r",B_Swing_Critical[pSkill->Point+i-1],"%");
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;
			break;
		case SKILL_ROAR:
			lstrcpy(szTempSkillDoc,StunRage);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d\r",Roar_Range[pSkill->Point+i-1]);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

			lstrcpy(szTempSkillDoc,CountinueTime);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d%s\r",Roar_Time[pSkill->Point+i-1],SecName);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;
			break;

		case SKILL_RAGE_OF_ZECRAM:
			lstrcpy(szTempSkillDoc,DamageAddName);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d%s\r",R_Zecram_Damage[pSkill->Point+i-1],"%");
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

			lstrcpy(szTempSkillDoc,FireAttackDamageAdd);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d%s\r",R_Zecram_Damage[pSkill->Point+i-1],"%");
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;
			break;

		case SKILL_CONCENTRATION:
			lstrcpy(szTempSkillDoc,Attck_RateAdd5);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d\r",Concentration_AttackRate[pSkill->Point+i-1]);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

			lstrcpy(szTempSkillDoc,CountinueTime);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d%s\r",Concentration_Time[pSkill->Point+i-1],SecName );
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;
			

			break;

		case SKILL_AVANGING_CRASH:
			lstrcpy(szTempSkillDoc,AddAttack_Rate9);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d%s\r",A_Crash_AttackRate[pSkill->Point+i-1],"%");
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

			lstrcpy(szTempSkillDoc,DamageAddName);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d%s\r",A_Crash_Damage[pSkill->Point+i-1],"%");
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

			break;
		case SKILL_SWIFT_AXE:

			lstrcpy(szTempSkillDoc,Attack_SpeedAdd);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d%s\r",Swift_Axe_Speed[pSkill->Point+i-1],"%");
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

			lstrcpy(szTempSkillDoc,CountinueTime);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d%s\r",Swift_Axe_Time[pSkill->Point+i-1],SecName );
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

			break;
		case SKILL_BONE_CRASH:
			lstrcpy(szTempSkillDoc,DamageAddName);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d%s\r",B_Crash_Damage[pSkill->Point+i-1],"%");
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

			lstrcpy(szTempSkillDoc,DemonDamage4);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d%s\r",B_Crash_DemonDamage[pSkill->Point+i-1],"%");
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

			break;
			
		/////////////////////파이크맨
		case SKILL_PIKE_WIND:
			lstrcpy(szTempSkillDoc,DamageName);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d-%d\r",Pike_Wind_Damage[pSkill->Point+i-1][0],Pike_Wind_Damage[pSkill->Point+i-1][1]);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

			lstrcpy(szTempSkillDoc,Push_AreaName);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d\r",Pike_Wind_Push_Lenght[pSkill->Point+i-1]);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;
			break;

		case SKILL_ICE_ATTRIBUTE:
			lstrcpy(szTempSkillDoc,IceName);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d\r",PlusIce[pSkill->Point+i-1]);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;
			//임의대로 한줄을 더 넣어준다 
			lstrcat(szSkillInfoBuff,"\r"); 
			lstrcpy(szTempSkillDoc2,"\r");
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;


			break;
		case SKILL_CRITICAL_HIT:
			lstrcpy(szTempSkillDoc,Add_CriticalName);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d%s\r",Critical_Hit_Critical[pSkill->Point+i-1],"%");
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;
			break;

		case SKILL_JUMPING_CRASH:
			lstrcpy(szTempSkillDoc,Attck_RateAdd);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d%s\r",Jumping_Crash_Attack_Rating[pSkill->Point+i-1],"%");
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

			lstrcpy(szTempSkillDoc,PlusDamageName);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d%s\r",Jumping_Crash_Damage[pSkill->Point+i-1],"%");
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;
			break;

		case SKILL_GROUND_PIKE:

			lstrcpy(szTempSkillDoc,sinG_Pike_Time3);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d\r",G_Pike_Time[pSkill->Point+i-1]);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;


			lstrcpy(szTempSkillDoc,IceAttackDamageAdd);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d-%d\r",G_Pike_Damage[pSkill->Point+i-1][0],G_Pike_Damage[pSkill->Point+i-1][1]);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

			lstrcpy(szTempSkillDoc,ShootingRangeName);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d\r",G_Pike_Range[pSkill->Point+i-1]);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;
			break;

		case SKILL_TORNADO:
			lstrcpy(szTempSkillDoc,DamageAddName);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d%s\r",Tornado_Damage[pSkill->Point+i-1],"%");
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

			lstrcpy(szTempSkillDoc,ShootingRangeName);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d\r",Tornado_Range[pSkill->Point+i-1]);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;
			break;
	
		case SKILL_WEAPONE_DEFENCE_MASTERY:
			lstrcpy(szTempSkillDoc,DefenseRateIncre);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d%s\r",W_D_Mastery_Block[pSkill->Point+i-1],"%");
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;
			//임의대로 한줄을 더 넣어준다 
			lstrcat(szSkillInfoBuff,"\r"); 
			lstrcpy(szTempSkillDoc2,"\r");
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;


			break;

		case SKILL_EXPANSION:
			lstrcpy(szTempSkillDoc,WeaponSizeName);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d%s\r",Expansion_Size[pSkill->Point+i-1],"%");
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

			lstrcpy(szTempSkillDoc,DamageAddName);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d%s\r",Expansion_Damage[pSkill->Point+i-1],"%");
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

			break;
		case SKILL_VENOM_SPEAR:
			PlusMinD = 0;
			PlusMaxD = 0;

			if(sInven[0].ItemIndex && !cInvenTory.InvenItem[sInven[0].ItemIndex-1].sItemInfo.NotUseFlag){
				PlusMinD = cInvenTory.InvenItem[sInven[0].ItemIndex-1].sItemInfo.Damage[0];
				PlusMaxD = cInvenTory.InvenItem[sInven[0].ItemIndex-1].sItemInfo.Damage[1];
			}
			lstrcpy(szTempSkillDoc,PoisonDamage3);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d-%d\r",VenomSpear_Damage[pSkill->Point+i-1][0],VenomSpear_Damage[pSkill->Point+i-1][1]);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;
			
			lstrcpy(szTempSkillDoc,PikeNum4);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d%s",VenomSpear_Num[pSkill->Point+i-1],sinNumCount3);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

			lstrcpy(szTempSkillDoc,poisoningTime);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d%s\r",VenomSpear_Time[pSkill->Point+i-1],SecName );
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

			lstrcpy(szTempSkillDoc,ShootingRangeName);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d\r",VenomSpear_Range[pSkill->Point+i-1]);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

			break;

		case SKILL_VANISH:
			/*
			lstrcpy(szTempSkillDoc,AddSpeed7);     
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			sinTemp1 = (int)(Vanish_Speed[pSkill->Point+i-1]*10);
			RightSpot = sinTemp1 %10;
			LeftSpot  = (sinTemp1-RightSpot)/10;
			wsprintf(szTempSkillDoc2,"%d.%d\r",LeftSpot,RightSpot);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;
			*/

			wsprintf(szTempSkillDoc,"%s",AfterDamage7);       
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d%s\r",Vanish_Damage[pSkill->Point+i-1],"%");
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

			lstrcpy(szTempSkillDoc,MonsterSight7);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d%s\r",Vanish_Range[pSkill->Point+i-1],"%");
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

			lstrcpy(szTempSkillDoc,CountinueTime);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d%s\r",Vanish_Time[pSkill->Point+i-1],SecName );
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

			break;

		case SKILL_CRITICAL_MASTERY:
			lstrcpy(szTempSkillDoc,PlusCriticalName); 
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d%s\r",Critical_Mastery_Critical[pSkill->Point+i-1],"%");
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;
			//임의대로 한줄을 더 넣어준다 
			lstrcat(szSkillInfoBuff,"\r"); 
			lstrcpy(szTempSkillDoc2,"\r");
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;
			
			break;

		case SKILL_CHAIN_LANCE:
			lstrcpy(szTempSkillDoc,DamageAddName);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d%s\r",Chain_Lance_Damage[pSkill->Point+i-1],"%");
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

			/*
			lstrcpy(szTempSkillDoc,IncreAttack_RateName);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d\r",Chain_Lance_AttackRate[pSkill->Point+i-1]);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;
			*/
			break;

		///////////////////// 아쳐 
		case SKILL_SCOUT_HAWK:  
			lstrcpy(szTempSkillDoc,Attck_RateAdd);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d%s\r",Scout_Hawk_Attack_Rate[pSkill->Point+i-1],"%");
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

			lstrcpy(szTempSkillDoc,HwakRotationNum);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d%s\r",Scout_Hawk_UseTime[pSkill->Point+i-1],sinNum7);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;
			break;
		case SKILL_SHOOTING_MASTERY:
			lstrcpy(szTempSkillDoc,DamageAddName);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d%s\r",S_Mastery_DamagePercent[pSkill->Point+i-1],"%");
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;
			//임의대로 한줄을 더 넣어준다 
			lstrcat(szSkillInfoBuff,"\r"); 
			lstrcpy(szTempSkillDoc2,"\r");
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;


			break;
		case SKILL_WIND_ARROW:
			lstrcpy(szTempSkillDoc,WeaponSpeedAddName);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d\r",Wind_Arrow_Speed[pSkill->Point+i-1]);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

			lstrcpy(szTempSkillDoc,DamageAddName);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d%s\r",Wind_Arrow_Damage[pSkill->Point+i-1],"%");
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;
			break;
		case SKILL_PERFECT_AIM:
			lstrcpy(szTempSkillDoc,IncreAttack_RateName);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d%s\r",Perfect_Aim_Attack_Rate[pSkill->Point+i-1],"%");
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

			lstrcpy(szTempSkillDoc,DamageAddName);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d+(LV/%d)\r",Perfect_Aim_Damage[pSkill->Point+i-1],Perfect_Aim_Damage_LV[pSkill->Point+i-1]);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;
			break;
		case SKILL_DIONS_EYE:
			lstrcpy(szTempSkillDoc,IncreAttack_RateName);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d%s\r",D_Eye_Attack_Rate[pSkill->Point+i-1],"%");
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;
			//임의대로 한줄을 더 넣어준다 
			lstrcat(szSkillInfoBuff,"\r"); 
			lstrcpy(szTempSkillDoc2,"\r");
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

			break;

		case SKILL_FALCON:
			lstrcpy(szTempSkillDoc,DamageName);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d-%d\r",Falcon_Damage[pSkill->Point+i-1][0],Falcon_Damage[pSkill->Point+i-1][1]);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

			lstrcpy(szTempSkillDoc,CountinueTime);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d\r",Falcon_Time[pSkill->Point+i-1]);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;
			break;

		case SKILL_ARROW_OF_RAGE:

			lstrcpy(szTempSkillDoc,DamageAddName); 
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"LV/%d\r",A_Rage_Damage[pSkill->Point+i-1]);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

			lstrcpy(szTempSkillDoc,AttackAreaName);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d\r",A_Rage_Area[pSkill->Point+i-1]);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;
			break;

		case SKILL_AVALANCHE:
			lstrcpy(szTempSkillDoc,WeaponSpeedAddName);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d\r",Avalanche_Speed[pSkill->Point+i-1]);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

			lstrcpy(szTempSkillDoc,DamageAddName);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d%s\r",Avalanche_Damage[pSkill->Point+i-1],"%");
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

			lstrcpy(szTempSkillDoc,SeriesShootingCount);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d\r",Avalanche_ArrowNum[pSkill->Point+i-1]);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;
			break;
		case SKILL_ELEMENTAL_SHOT:
			lstrcpy(szTempSkillDoc,FireDamage); 
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d-%d\r",Elemental_Shot_Fire[pSkill->Point+i-1][0],Elemental_Shot_Fire[pSkill->Point+i-1][1]);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

			/*
			lstrcpy(szTempSkillDoc,IceDamage); 
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d-%d\r",Elemental_Shot_Ice[pSkill->Point+i-1][0],Elemental_Shot_Ice[pSkill->Point+i-1][1]);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;
			*/

			lstrcpy(szTempSkillDoc,LightningDamage); 
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d-%d\r",Elemental_Shot_Lightning[pSkill->Point+i-1][0],Elemental_Shot_Lightning[pSkill->Point+i-1][1]);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

			break;

		case SKILL_GOLDEN_FALCON:
			lstrcpy(szTempSkillDoc,SpiritFalconDamage2);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d%s\r",Golden_Falcon_Damage[pSkill->Point+i-1],"%");
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

			lstrcpy(szTempSkillDoc,LifeGegenPlus);     
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			sinTemp1 = (int)(Golden_Falcon_LifeRegen[pSkill->Point+i-1]*10);
			RightSpot = sinTemp1 %10;
			LeftSpot  = (sinTemp1-RightSpot)/10;
			wsprintf(szTempSkillDoc2,"%d.%d\r",LeftSpot,RightSpot);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

			lstrcpy(szTempSkillDoc,CountinueTime);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d\r",Golden_Falcon_Time[pSkill->Point+i-1]);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

			break;

		case SKILL_BOMB_SHOT:
			lstrcpy(szTempSkillDoc,DamageAddName);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d%s\r",BombShot_Damage[pSkill->Point+i-1],"%");
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

			lstrcpy(szTempSkillDoc,DemonDamage4);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d%s\r",BombShot_DemonDamage[pSkill->Point+i-1],"%");
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

			lstrcpy(szTempSkillDoc,PlusDamage4); 
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d-%d\r",BombShot_AddDamage[pSkill->Point+i-1][0],BombShot_AddDamage[pSkill->Point+i-1][1]);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

			lstrcpy(szTempSkillDoc,Area18);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d\r",BombShot_Area[pSkill->Point+i-1]);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

			break;
		case SKILL_PERFORATION:
/*
			lstrcpy(szTempSkillDoc,PiercingPercent);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d%s\r",Perforation_Percent[pSkill->Point+i-1],"%");
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;
*/

			lstrcpy(szTempSkillDoc,AddShootingRange);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d\r",Perforation_Attack_Range[pSkill->Point+i-1]);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

			lstrcpy(szTempSkillDoc,DamageAddName);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d%s\r",Perforation_Damage[pSkill->Point+i-1],"%");
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

			lstrcpy(szTempSkillDoc,Add_CriticalName);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d%s\r",Perforation_Critical[pSkill->Point+i-1],"%");
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;
/*
			//사정거리 빠졌음 (잠쉬 대기)
			lstrcpy(szTempSkillDoc,AddShootingRange);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d\r",30);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;
*/
			break;

			
			/*********************모라이온************************************/
		
		//////////////// 나이트 그룹 
		case SKILL_SWORD_BLAST:
			lstrcpy(szTempSkillDoc,DamageName);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d-%d(+LV/2)\r",Sword_Blast_Damage[pSkill->Point+i-1][0],Sword_Blast_Damage[pSkill->Point+i-1][1]);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;
			/*
			lstrcpy(szTempSkillDoc,sinMaxDamageAdd);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			lstrcpy(szTempSkillDoc2,"LV/3\r");
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;
			*/

			lstrcpy(szTempSkillDoc,ShootingRangeName);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d\r",Sword_Blast_ShootingRange[pSkill->Point+i-1]);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;
			/*
			lstrcpy(szTempSkillDoc,sinDamagePiercing);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			lstrcpy(szTempSkillDoc2,"\r");
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;
			*/
		break;
		case SKILL_HOLY_BODY:
			lstrcpy(szTempSkillDoc,IncreAsorb);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d%s\r",HolyBody_Absorb[pSkill->Point+i-1],"%");
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

			lstrcpy(szTempSkillDoc,CountinueTime);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d%s\r",HolyBody_Time[pSkill->Point+i-1],SecName);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

		break;
		case SKILL_PHYSICAL_TRANING: 
			lstrcpy(szTempSkillDoc,sinAddStamina);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d%s\r",P_Traning_Stamina[pSkill->Point+i-1],"%");
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;
			//임의대로 한줄을 더 넣어준다 
			lstrcat(szSkillInfoBuff,"\r"); 
			lstrcpy(szTempSkillDoc2,"\r");
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;


		break;
		case SKILL_DOUBLE_CRASH:
			lstrcpy(szTempSkillDoc,DamageAddName);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d%s\r",D_Crash_Damage[pSkill->Point+i-1],"%");
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

			lstrcpy(szTempSkillDoc,Add_CriticalName);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d%s\r",D_Crash_Critical[pSkill->Point+i-1],"%");
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

		break;
		case SKILL_HOLY_VALOR:
			lstrcpy(szTempSkillDoc,PartyArea7); 
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d\r",300);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

			lstrcpy(szTempSkillDoc,DamageAddName);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d%s\r",Holy_Valor_Damage[pSkill->Point+i-1],"%");
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

			lstrcpy(szTempSkillDoc,CountinueTime);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d%s\r",Holy_Valor_Time[pSkill->Point+i-1],SecName);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

		break;
		case SKILL_BRANDISH:
			lstrcpy(szTempSkillDoc,Area17);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d\r",Brandish_Range[pSkill->Point+i-1]);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

			lstrcpy(szTempSkillDoc,DamageAddName);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d%s\r",Brandish_Damage[pSkill->Point+i-1],"%");
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;
			
		break;
		case SKILL_PIERCING:
			lstrcpy(szTempSkillDoc,DamageAddName); 
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d%s\r",Piercing_Damage[pSkill->Point+i-1],"%");
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

			lstrcpy(szTempSkillDoc,AddAttack_Rate9);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d\r",Piercing_Attack_Rating[pSkill->Point+i-1]);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

			lstrcpy(szTempSkillDoc,PiercingRange);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d\r",110);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;
		break;
		case SKILL_DRASTIC_SPIRIT:
			lstrcpy(szTempSkillDoc,IncreDefense);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d%s\r",Drastic_Spirit_Defense[pSkill->Point+i-1],"%");
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

			lstrcpy(szTempSkillDoc,CountinueTime);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d%s\r",Drastic_Spirit_Time[pSkill->Point+i-1],SecName);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

		break;

		case SKILL_SWORD_MASTERY:
			lstrcpy(szTempSkillDoc,DamageAddName);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d%s\r",Sword_Mastery_DamagePercent[pSkill->Point+i-1],"%");
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;
			//임의대로 한줄을 더 넣어준다 
			lstrcat(szSkillInfoBuff,"\r"); 
			lstrcpy(szTempSkillDoc2,"\r");
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;


		break;
		case SKILL_DIVINE_INHALATION:
			lstrcpy(szTempSkillDoc,IncreBlock);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d%s\r",D_Inhalation_Block[pSkill->Point+i-1],"%");
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

			lstrcpy(szTempSkillDoc,DamageToLife);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d%s\r",D_Inhalation_Life[pSkill->Point+i-1],"%");
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

			lstrcpy(szTempSkillDoc,CountinueTime);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d%s\r",D_Inhalation_Time[pSkill->Point+i-1],SecName);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

		break;

		case SKILL_HOLY_INCANTATION:
			lstrcpy(szTempSkillDoc,MyMonster7);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d%s\r",H_Incantation_Success[pSkill->Point+i-1],"%");
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

			lstrcpy(szTempSkillDoc,LifeIncre4);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d%s\r",H_Incantation_AddLife[pSkill->Point+i-1],"%");
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

			lstrcpy(szTempSkillDoc,CountinueTime);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d%s\r",H_Incantation_Time[pSkill->Point+i-1],SecName);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

		break;
		case SKILL_GRAND_CROSS:
			lstrcpy(szTempSkillDoc,DamageAddName);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d%s\r",Grand_Cross_Damage[pSkill->Point+i-1],"%");
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

			lstrcpy(szTempSkillDoc,AddAttack_Rate9);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d%s\r",Grand_Cross_AttackRate[pSkill->Point+i-1],"%");
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;
			
			lstrcpy(szTempSkillDoc,UndeadDamage3);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d%s\r",Grand_Cross_UndeadDamage[pSkill->Point+i-1],"%");
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;
			
		break;



		//////////////// 아탈란타 그룹 
		case SKILL_SHIELD_STRIKE:
			/*
			lstrcpy(szTempSkillDoc,ShootingRangeName); 
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d\r",110);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;
			*/
			lstrcpy(szTempSkillDoc,sinDamagePiercing);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			lstrcpy(szTempSkillDoc2,"\r");
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

			lstrcpy(szTempSkillDoc,DamageName);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d-%d(+LV/5)\r",S_Strike_Damage[pSkill->Point+i-1][0],S_Strike_Damage[pSkill->Point+i-1][1]);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

			/*
			lstrcpy(szTempSkillDoc,StuneRate);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d%s\r",S_Strike_Stun[pSkill->Point+i-1],"%");
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;
			*/

		break;
		
		case SKILL_FARINA:
			lstrcpy(szTempSkillDoc,WeaponSpeedAddName);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d\r",Farina_Speed[pSkill->Point+i-1]);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

			lstrcpy(szTempSkillDoc,IncreAttack_RateName);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d%s\r",Farina_AttackRate[pSkill->Point+i-1],"(+LV)");
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

		break;
		case SKILL_THROWING_MASTERY:
			lstrcpy(szTempSkillDoc,DamageAddName);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d%s\r",T_Mastery_Damage[pSkill->Point+i-1],"%");
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;
			//임의대로 한줄을 더 넣어준다 
			lstrcat(szSkillInfoBuff,"\r"); 
			lstrcpy(szTempSkillDoc2,"\r");
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

		break;
		case SKILL_VIGOR_SPEAR:
			lstrcpy(szTempSkillDoc,sinAddDamage7);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d-%d+(LV/2)\r",V_Spear_Damage[pSkill->Point+i-1][0],V_Spear_Damage[pSkill->Point+i-1][1]);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

		break;

		case SKILL_WINDY:
			lstrcpy(szTempSkillDoc,IncreWeaponAttack_RateName);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d%s\r",Windy_Attack_Rating[pSkill->Point+i-1],"%");
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

			lstrcpy(szTempSkillDoc,AddShootingRange);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d\r",30);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

			lstrcpy(szTempSkillDoc,CountinueTime);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d%s\r",Windy_Time[pSkill->Point+i-1],SecName);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;
			

		break;
		case SKILL_TWIST_JAVELIN:
			lstrcpy(szTempSkillDoc,DamageAddName);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d%s\r",Twist_Javelin_Damage[pSkill->Point+i-1],"%");
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

			lstrcpy(szTempSkillDoc,AddAttack_Rate9);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d%s\r",Twist_Javelin_Attack_Rating[pSkill->Point+i-1],"%");
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

		break;
		case SKILL_SOUL_SUCKER:
			lstrcpy(szTempSkillDoc,LifeAbsorb);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d%s\r",Soul_Sucker_Absorb[pSkill->Point+i-1],"%");
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;
		break;
		case SKILL_FIRE_JAVELIN:
			lstrcpy(szTempSkillDoc,FireDamage);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d-%d+(LV/3)\r",Fire_Javelin_Damage[pSkill->Point+i-1][0],Fire_Javelin_Damage[pSkill->Point+i-1][1]);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;
		break;
		case SKILL_SPLIT_JAVELIN:
			lstrcpy(szTempSkillDoc,AttackNum3);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d\r",Split_Javelin_AttackNum[pSkill->Point+i-1]);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

			lstrcpy(szTempSkillDoc,DamageAddName);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d%s\r",Split_Javelin_Damage[pSkill->Point+i-1],"%");
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

			lstrcpy(szTempSkillDoc,sinAbilityName[5]);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d%s\r",Split_Javelin_AttackRate[pSkill->Point+i-1],"%");
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

			/*
			lstrcpy(szTempSkillDoc,AttackRateMinus);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"-%d%s\r",Split_Javelin_AttackRate[pSkill->Point+i-1],"%");
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;
			*/

		break;
		
		case SKILL_TRIUMPH_OF_VALHALLA:
			lstrcpy(szTempSkillDoc,MaxDamagePlus2);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d+(LV/4)\r",T_Of_Valhalla_Damage[pSkill->Point+i-1]); // 장별 - 스킬밸런스 조정(10.08.10)
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

			lstrcpy(szTempSkillDoc,CountinueTime);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d%s\r",T_Of_Valhalla_Time[pSkill->Point+i-1],SecName);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

		break;
		case SKILL_LIGHTNING_JAVELIN:
			lstrcpy(szTempSkillDoc,LightningAddDamage3);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d-%d+(LV/3)\r",Lightning_Javelin_Damage[pSkill->Point+i-1][0],Lightning_Javelin_Damage[pSkill->Point+i-1][1]);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;
		break;
		case SKILL_STORM_JAVELIN:
			lstrcpy(szTempSkillDoc,DamageAddName);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d%s\r",Storm_Javelin_Damage[pSkill->Point+i-1],"%");
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

		break;

		//////////////// 프리스티스 
		case SKILL_HEALING: 
			PlusMinD = 0;
			PlusMaxD = 0;
			/*
			if(sInven[0].ItemIndex && !cInvenTory.InvenItem[sInven[0].ItemIndex-1].sItemInfo.NotUseFlag){
				PlusMinD = cInvenTory.InvenItem[sInven[0].ItemIndex-1].sItemInfo.Damage[0];
				PlusMaxD = cInvenTory.InvenItem[sInven[0].ItemIndex-1].sItemInfo.Damage[1];
			}
			*/
			// 장별 - 스킬밸런스 조정(10.08.10)
			PlusMinD += cInvenTory.InvenItem[sInven[0].ItemIndex-1].sItemInfo.Damage[0]/3;
			PlusMaxD += cInvenTory.InvenItem[sInven[0].ItemIndex-1].sItemInfo.Damage[1]/3;
			PlusMinD += sinChar->Spirit/8;
			PlusMaxD += sinChar->Spirit/6;
			

			lstrcpy(szTempSkillDoc,sinAddLife); 
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d(+%d)-%d(+%d)\r",Healing_Heal[pSkill->Point+i-1][0],PlusMinD,Healing_Heal[pSkill->Point+i-1][1],PlusMaxD);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;
		break;
		case SKILL_HOLY_BOLT:
			PlusMinD = 0;
			PlusMaxD = 0;
			/*
			if(sInven[0].ItemIndex && !cInvenTory.InvenItem[sInven[0].ItemIndex-1].sItemInfo.NotUseFlag){
				PlusMinD = cInvenTory.InvenItem[sInven[0].ItemIndex-1].sItemInfo.Damage[0];
				PlusMaxD = cInvenTory.InvenItem[sInven[0].ItemIndex-1].sItemInfo.Damage[1];
			}
			*/
			//PlusMinD += sinChar->Spirit/DivideMin;
			//PlusMaxD += sinChar->Spirit/DivideMax;

			// 장별 - 스킬밸런스 조정(10.08.10)
			PlusMinD += cInvenTory.InvenItem[sInven[0].ItemIndex-1].sItemInfo.Damage[0]/3;
			PlusMaxD += cInvenTory.InvenItem[sInven[0].ItemIndex-1].sItemInfo.Damage[1]/3;
			PlusMinD += sinChar->Spirit/8;
			PlusMaxD += sinChar->Spirit/6;

			wsprintf(szTempSkillDoc,"%s",DamageName);       
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d(+%d)-%d(+%d)\r",HolyBolt_Damage[pSkill->Point+i-1][0],PlusMinD,HolyBolt_Damage[pSkill->Point+i-1][1],PlusMaxD);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

		break;
		case SKILL_MULTISPARK:
			PlusMinD = 0;
			PlusMaxD = 0;
			/*
			if(sInven[0].ItemIndex && !cInvenTory.InvenItem[sInven[0].ItemIndex-1].sItemInfo.NotUseFlag){
				PlusMinD = cInvenTory.InvenItem[sInven[0].ItemIndex-1].sItemInfo.Damage[0];
				PlusMaxD = cInvenTory.InvenItem[sInven[0].ItemIndex-1].sItemInfo.Damage[1];
			}
			*/
			//PlusMinD += sinChar->Spirit/DivideMin;
			//PlusMaxD += sinChar->Spirit/DivideMax;

			// 장별 - 스킬밸런스 조정(10.08.10)
			PlusMinD += cInvenTory.InvenItem[sInven[0].ItemIndex-1].sItemInfo.Damage[0]/3;
			PlusMaxD += cInvenTory.InvenItem[sInven[0].ItemIndex-1].sItemInfo.Damage[1]/3;
			PlusMinD += sinChar->Spirit/8;
			PlusMaxD += sinChar->Spirit/6;

			lstrcpy(szTempSkillDoc,SparkDamage); 
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d(+%d)-%d(+%d)\r",M_Spark_Damage[pSkill->Point+i-1][0],PlusMinD,M_Spark_Damage[pSkill->Point+i-1][1],PlusMaxD);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

			lstrcpy(szTempSkillDoc,SparkNum7); 
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d-%d%s",(M_Spark_Num[pSkill->Point+i-1]/2)+1,M_Spark_Num[pSkill->Point+i-1],sinNumCount3);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

		break;
		case SKILL_HOLY_MIND:
			lstrcpy(szTempSkillDoc,sinDecreDamage);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d%s\r",HolyMind_DecDamage[pSkill->Point+i-1],"%");
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

			lstrcpy(szTempSkillDoc,CountinueTime);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"15%s\r",SecName);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

		break;

		case SKILL_MEDITATION:
			lstrcpy(szTempSkillDoc,AddManaRegen);     
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			sinTemp1 = (int)(Meditation_Regen[pSkill->Point+i-1]*10);
			RightSpot = sinTemp1 %10;
			LeftSpot  = (sinTemp1-RightSpot)/10;
			wsprintf(szTempSkillDoc2,"%d.%d\r",LeftSpot,RightSpot);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;
			//임의대로 한줄을 더 넣어준다 
			lstrcat(szSkillInfoBuff,"\r"); 
			lstrcpy(szTempSkillDoc2,"\r");
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

		break;
		case SKILL_DIVINE_LIGHTNING:
			lstrcpy(szTempSkillDoc,AttackAreaName);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d\r",180);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

			PlusMinD = 0;
			PlusMaxD = 0;

			/*
			if(sInven[0].ItemIndex && !cInvenTory.InvenItem[sInven[0].ItemIndex-1].sItemInfo.NotUseFlag){
				PlusMinD = cInvenTory.InvenItem[sInven[0].ItemIndex-1].sItemInfo.Damage[0];
				PlusMaxD = cInvenTory.InvenItem[sInven[0].ItemIndex-1].sItemInfo.Damage[1];
			}
			PlusMinD += sinChar->Spirit/DivideMin;
			PlusMaxD += sinChar->Spirit/DivideMax;
			*/

			// 장별 - 스킬밸런스 조정(10.08.10)
			PlusMinD = sinChar->Level/2;
			PlusMaxD = sinChar->Level/2;

			PlusMinD += sinChar->Spirit/6;
			PlusMaxD += sinChar->Spirit/3;



			lstrcpy(szTempSkillDoc,SparkDamage); 
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d(+%d)-%d(+%d)\r",Divine_Lightning_Damage[pSkill->Point+i-1][0],PlusMinD,Divine_Lightning_Damage[pSkill->Point+i-1][1],PlusMaxD);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

			lstrcpy(szTempSkillDoc,LightNum);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d\r",Divine_Lightning_Num[pSkill->Point+i-1]);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

		break;
		case SKILL_HOLY_REFLECTION:
			lstrcpy(szTempSkillDoc,ReturnDamage);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d%s\r",Holy_Reflection_Return_Damage[pSkill->Point+i-1],"%");
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

			lstrcpy(szTempSkillDoc,CountinueTime);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d%s\r",Holy_Reflection_Time[pSkill->Point+i-1],SecName);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

		break;
		case SKILL_GRAND_HEALING:
			PlusMinD = 0;
			PlusMaxD = 0;

			/*
			if(sInven[0].ItemIndex && !cInvenTory.InvenItem[sInven[0].ItemIndex-1].sItemInfo.NotUseFlag){
				PlusMinD = cInvenTory.InvenItem[sInven[0].ItemIndex-1].sItemInfo.Damage[0];
				PlusMaxD = cInvenTory.InvenItem[sInven[0].ItemIndex-1].sItemInfo.Damage[1];
			}
			*/

			// 장별 - 스킬밸런스 조정(10.08.10)
			PlusMinD += cInvenTory.InvenItem[sInven[0].ItemIndex-1].sItemInfo.Damage[0]/3;
			PlusMaxD += cInvenTory.InvenItem[sInven[0].ItemIndex-1].sItemInfo.Damage[1]/3;
			PlusMinD += sinChar->Spirit/8;
			PlusMaxD += sinChar->Spirit/6;


			lstrcpy(szTempSkillDoc,sinAddLife); 
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d(+%d)-%d(+%d)\r",Grand_Healing[pSkill->Point+i-1][0],PlusMinD,Grand_Healing[pSkill->Point+i-1][1],PlusMaxD);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

			lstrcpy(szTempSkillDoc,Area18);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d\r",Grand_Healing_Range[pSkill->Point+i-1]);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

		break;
		case SKILL_VIGOR_BALL:
			PlusMinD = 0;
			PlusMaxD = 0;

			/*
			if(sInven[0].ItemIndex && !cInvenTory.InvenItem[sInven[0].ItemIndex-1].sItemInfo.NotUseFlag){
				PlusMinD = cInvenTory.InvenItem[sInven[0].ItemIndex-1].sItemInfo.Damage[0];
				PlusMaxD = cInvenTory.InvenItem[sInven[0].ItemIndex-1].sItemInfo.Damage[1];
			}
			PlusMinD += sinChar->Spirit/DivideMin;
			PlusMaxD += sinChar->Spirit/DivideMax;
			*/

			// 장별 - 스킬밸런스 조정(10.08.10)
			PlusMinD = sinChar->Level/2;
			PlusMaxD = sinChar->Level/2;

			PlusMinD += sinChar->Spirit/6;
			PlusMaxD += sinChar->Spirit/3;


			wsprintf(szTempSkillDoc,"%s",DamageName);       
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d(+%d)-%d(+%d)\r",Vigor_Ball_Damage[pSkill->Point+i-1][0],PlusMinD,Vigor_Ball_Damage[pSkill->Point+i-1][1],PlusMaxD);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

		break;
		
		case SKILL_RESURRECTION:
			lstrcpy(szTempSkillDoc,ReLifePercent4); 
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d%s\r",Resurrection_Percent[pSkill->Point+i-1],"%");
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

			/*
			lstrcpy(szTempSkillDoc,GetExp3); 
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%dx%s\r",Resurrection_Exp[pSkill->Point+i-1],ResurrectionChar4);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;
			*/

		break;

		case SKILL_EXTINCTION:
			lstrcpy(szTempSkillDoc,ExtinctionPercent2);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d+(LV/5)%s\r",Extinction_Percent[pSkill->Point+i-1],"%");
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

			lstrcpy(szTempSkillDoc,ExtinctionAmount2);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d%s\r",Extinction_Amount[pSkill->Point+i-1],"%");
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;
		break;
		case SKILL_VIRTUAL_LIFE:
			lstrcpy(szTempSkillDoc,IncreLifePercent2); 
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d%s\r",Virtual_Life_Percent[pSkill->Point+i-1],"%");
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

			lstrcpy(szTempSkillDoc,CountinueTime);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d%s\r",Virtual_Life_Time[pSkill->Point+i-1],SecName);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

		break;

		//////////////// 메지션 
		case SKILL_AGONY:
			lstrcpy(szTempSkillDoc,sinConvert4);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d%s\r",Agony_ConvLife[pSkill->Point+i-1],"%");
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

		break;
		case SKILL_FIRE_BOLT:
			PlusMinD = 0;
			PlusMaxD = 0;

			/*
			if(sInven[0].ItemIndex && !cInvenTory.InvenItem[sInven[0].ItemIndex-1].sItemInfo.NotUseFlag){
				PlusMinD = cInvenTory.InvenItem[sInven[0].ItemIndex-1].sItemInfo.Damage[0];
				PlusMaxD = cInvenTory.InvenItem[sInven[0].ItemIndex-1].sItemInfo.Damage[1];
			}
			*/
			//PlusMinD += sinChar->Spirit/DivideMin;
			//PlusMaxD += sinChar->Spirit/DivideMax;

			// 장별 - 스킬밸런스 조정(10.08.10)
			PlusMinD += sinChar->Spirit/8;
			PlusMaxD += sinChar->Spirit/6;
			PlusMinD += (cInvenTory.InvenItem[sInven[0].ItemIndex-1].sItemInfo.Damage[0]/3);
			PlusMaxD += (cInvenTory.InvenItem[sInven[0].ItemIndex-1].sItemInfo.Damage[1]/3);

			TempPoint = SearchContiueSkill(SKILL_SPIRIT_ELEMENTAL);
			if(TempPoint){
				PlusMinD += (FireBolt_Damage[pSkill->Point-1][0] * Spirit_Elemental_Damage[TempPoint-1])/100;
				PlusMaxD += (FireBolt_Damage[pSkill->Point-1][1] * Spirit_Elemental_Damage[TempPoint-1])/100;
			}
			wsprintf(szTempSkillDoc,"%s",DamageName);       
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d(+%d)-%d(+%d)\r",FireBolt_Damage[pSkill->Point+i-1][0],PlusMinD,FireBolt_Damage[pSkill->Point+i-1][1],PlusMaxD);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

			/*
			lstrcpy(szTempSkillDoc,sinMaxDamageAdd);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			lstrcpy(szTempSkillDoc2,"LV\r");
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;
			*/

			lstrcpy(szTempSkillDoc,ShootingRangeName);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d\r",200);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

		break;
		case SKILL_ZENITH:
			lstrcpy(szTempSkillDoc,sinIncreElement); 
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d\r",Zenith_Element[pSkill->Point+i-1]);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

			lstrcpy(szTempSkillDoc,CountinueTime);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d%s\r",Zenith_Time[pSkill->Point+i-1],SecName);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;
		
			/*
			lstrcpy(szTempSkillDoc,sinPartyIncreElement);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			lstrcpy(szTempSkillDoc2,"\r");
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;
			*/


		break;
		case SKILL_FIRE_BALL: 
			/*
			lstrcpy(szTempSkillDoc,sinFireBallDamage2);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			lstrcpy(szTempSkillDoc2,"\r");
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;]

			lstrcpy(szTempSkillDoc,sinFireBallDamage3);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			lstrcpy(szTempSkillDoc2,"\r");
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;
			*/

			PlusMinD = 0;
			PlusMaxD = 0;

			/*
			if(sInven[0].ItemIndex && !cInvenTory.InvenItem[sInven[0].ItemIndex-1].sItemInfo.NotUseFlag){
				PlusMinD = cInvenTory.InvenItem[sInven[0].ItemIndex-1].sItemInfo.Damage[0];
				PlusMaxD = cInvenTory.InvenItem[sInven[0].ItemIndex-1].sItemInfo.Damage[1];
			}
			PlusMinD += sinChar->Spirit/DivideMin;
			PlusMaxD += sinChar->Spirit/DivideMax;
			*/

			// 장별 - 스킬밸런스 조정(10.08.10)
			PlusMinD = sinChar->Level/2;
			PlusMaxD = sinChar->Level/2;

			PlusMinD += sinChar->Spirit/6;
			PlusMaxD += sinChar->Spirit/DivideMax;

			TempPoint = SearchContiueSkill(SKILL_SPIRIT_ELEMENTAL);
			if(TempPoint){
				PlusMinD += (FireBall_Damage[pSkill->Point-1][0] * Spirit_Elemental_Damage[TempPoint-1])/100;
				PlusMaxD += (FireBall_Damage[pSkill->Point-1][1] * Spirit_Elemental_Damage[TempPoint-1])/100;
			}

			lstrcpy(szTempSkillDoc,DamageName); 
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d(+%d)-%d(+%d)\r",FireBall_Damage[pSkill->Point+i-1][0],PlusMinD,FireBall_Damage[pSkill->Point+i-1][1],PlusMaxD);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

			lstrcpy(szTempSkillDoc,ShootingRangeName);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d\r",FireBall_Range[pSkill->Point+i-1]);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

			lstrcpy(szTempSkillDoc,AttackAreaName);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d\r",FireBall_Area[pSkill->Point+i-1]);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;


		break;
		case SKILL_MENTAL_MASTERY:
			lstrcpy(szTempSkillDoc,IncreMana5);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d%s\r",Mental_Mastery_Mana[pSkill->Point+i-1],"%");
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;
			//임의대로 한줄을 더 넣어준다 
			lstrcat(szSkillInfoBuff,"\r"); 
			lstrcpy(szTempSkillDoc2,"\r");
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;


		break;
		case SKILL_WATORNADO:
			PlusMinD = 0;
			PlusMaxD = 0;

			/*
			if(sInven[0].ItemIndex && !cInvenTory.InvenItem[sInven[0].ItemIndex-1].sItemInfo.NotUseFlag){
				PlusMinD = cInvenTory.InvenItem[sInven[0].ItemIndex-1].sItemInfo.Damage[0];
				PlusMaxD = cInvenTory.InvenItem[sInven[0].ItemIndex-1].sItemInfo.Damage[1];
			}
			PlusMinD += sinChar->Spirit/DivideMin;
			PlusMaxD += sinChar->Spirit/DivideMax;
			*/

			// 장별 - 스킬밸런스 조정(10.08.10)
			PlusMinD = sinChar->Level/2;
			PlusMaxD = sinChar->Level/2;

			PlusMinD += sinChar->Spirit/6;
			PlusMaxD += sinChar->Spirit/3;

			TempPoint = SearchContiueSkill(SKILL_SPIRIT_ELEMENTAL);
			if(TempPoint){
				PlusMinD += (Watornado_Damage[pSkill->Point-1][0] * Spirit_Elemental_Damage[TempPoint-1])/100;
				PlusMaxD += (Watornado_Damage[pSkill->Point-1][1] * Spirit_Elemental_Damage[TempPoint-1])/100;
			}

			lstrcpy(szTempSkillDoc,DamageName); 
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d(+%d)-%d(+%d)\r",Watornado_Damage[pSkill->Point+i-1][0],PlusMinD,Watornado_Damage[pSkill->Point+i-1][1],PlusMaxD);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

			lstrcpy(szTempSkillDoc,ShootingRangeName);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d\r",Watornado_Range[pSkill->Point+i-1]);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

			lstrcpy(szTempSkillDoc,AttackAreaName);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d\r",Watornado_Area[pSkill->Point+i-1]);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

		break;
		case SKILL_ENCHANT_WEAPON:
			lstrcpy(szTempSkillDoc,FireDamage); 
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d-%d\r",Enchant_Weapon_Damage_Fire[pSkill->Point+i-1][0],Enchant_Weapon_Damage_Fire[pSkill->Point+i-1][1]);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

			lstrcpy(szTempSkillDoc,IceDamage); 
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d-%d\r",Enchant_Weapon_Damage_Ice[pSkill->Point+i-1][0],Enchant_Weapon_Damage_Ice[pSkill->Point+i-1][1]);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

			lstrcpy(szTempSkillDoc,LightningDamage); 
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d-%d\r",Enchant_Weapon_Damage_Lightning[pSkill->Point+i-1][0],Enchant_Weapon_Damage_Lightning[pSkill->Point+i-1][1]);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

			lstrcpy(szTempSkillDoc,CountinueTime);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d%s\r",Enchant_Weapon_Time[pSkill->Point+i-1],SecName);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;
		break;
		case SKILL_DEAD_RAY:
			PlusMinD = 0;
			PlusMaxD = 0;

			/*
			if(sInven[0].ItemIndex && !cInvenTory.InvenItem[sInven[0].ItemIndex-1].sItemInfo.NotUseFlag){
				PlusMinD = cInvenTory.InvenItem[sInven[0].ItemIndex-1].sItemInfo.Damage[0];
				PlusMaxD = cInvenTory.InvenItem[sInven[0].ItemIndex-1].sItemInfo.Damage[1];
			}
			PlusMinD += sinChar->Spirit/DivideMin;
			PlusMaxD += sinChar->Spirit/DivideMax;
			*/

			// 장별 - 스킬밸런스 조정(10.08.10)
			PlusMinD = sinChar->Level/2;
			PlusMaxD = sinChar->Level/2;
			PlusMinD += cInvenTory.InvenItem[sInven[0].ItemIndex-1].sItemInfo.Damage[0];
			PlusMaxD += cInvenTory.InvenItem[sInven[0].ItemIndex-1].sItemInfo.Damage[1];
			PlusMinD += sinChar->Spirit/4;
			PlusMaxD += sinChar->Spirit/2;

			TempPoint = SearchContiueSkill(SKILL_SPIRIT_ELEMENTAL);
			if(TempPoint){
				PlusMinD += (Dead_Ray_Damage[pSkill->Point-1][0] * Spirit_Elemental_Damage[TempPoint-1])/100;
				PlusMaxD += (Dead_Ray_Damage[pSkill->Point-1][1] * Spirit_Elemental_Damage[TempPoint-1])/100;
			}

			lstrcpy(szTempSkillDoc,DamageName); 
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d(+%d)-%d(+%d)\r",Dead_Ray_Damage[pSkill->Point+i-1][0],PlusMinD,Dead_Ray_Damage[pSkill->Point+i-1][1],PlusMaxD);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

			lstrcpy(szTempSkillDoc,ShootingRangeName);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d\r",Dead_Ray_Range[pSkill->Point+i-1]);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

		break;

		case SKILL_ENERGY_SHIELD:
			lstrcpy(szTempSkillDoc,ReduceDamage3);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d%s\r",Energy_Shield_DecDamage[pSkill->Point+i-1],"%");
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

			lstrcpy(szTempSkillDoc,CountinueTime);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d%s\r",Energy_Shield_Time[pSkill->Point+i-1],SecName);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

		break;

		case SKILL_DIASTROPHISM:
			PlusMinD = 0;
			PlusMaxD = 0;

			/*
			if(sInven[0].ItemIndex && !cInvenTory.InvenItem[sInven[0].ItemIndex-1].sItemInfo.NotUseFlag){
				PlusMinD = cInvenTory.InvenItem[sInven[0].ItemIndex-1].sItemInfo.Damage[0];
				PlusMaxD = cInvenTory.InvenItem[sInven[0].ItemIndex-1].sItemInfo.Damage[1];
			}
			PlusMinD += sinChar->Spirit/DivideMin;
			PlusMaxD += sinChar->Spirit/DivideMax;
			*/

			// 장별 - 스킬밸런스 조정(10.08.10)
			PlusMinD = sinChar->Level/2;
			PlusMaxD = sinChar->Level/2;

			PlusMinD += sinChar->Spirit/6;
			PlusMaxD += sinChar->Spirit/3;

			TempPoint = SearchContiueSkill(SKILL_SPIRIT_ELEMENTAL);
			if(TempPoint){
				PlusMinD += (Diastrophism_Damage[pSkill->Point-1][0] * Spirit_Elemental_Damage[TempPoint-1])/100;
				PlusMaxD += (Diastrophism_Damage[pSkill->Point-1][1] * Spirit_Elemental_Damage[TempPoint-1])/100;
			}

			lstrcpy(szTempSkillDoc,DamageName); 
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d(+%d)-%d(+%d)\r",Diastrophism_Damage[pSkill->Point+i-1][0],PlusMinD,Diastrophism_Damage[pSkill->Point+i-1][1],PlusMaxD);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

			lstrcpy(szTempSkillDoc,Area18);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d\r",Diastrophism_Area[pSkill->Point+i-1]);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

		break;
		case SKILL_SPIRIT_ELEMENTAL:
			/*
			lstrcpy(szTempSkillDoc,AddManaRegen);     
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			sinTemp1 = (int)(Spirit_Elemental_RegenMana[pSkill->Point+i-1]*10);
			RightSpot = sinTemp1 %10;
			LeftSpot  = (sinTemp1-RightSpot)/10;
			wsprintf(szTempSkillDoc2,"%d.%d\r",LeftSpot,RightSpot);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;
			*/

			lstrcpy(szTempSkillDoc,IncreMagicDamage);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d%s\r",Spirit_Elemental_Damage[pSkill->Point+i-1],"%");
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

			lstrcpy(szTempSkillDoc,CountinueTime);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d%s\r",Spirit_Elemental_Time[pSkill->Point+i-1],SecName);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

		break;

		case SKILL_DANCING_SWORD:
			PlusMinD = 0;
			PlusMaxD = 0;

			/*
			if(sInven[0].ItemIndex && !cInvenTory.InvenItem[sInven[0].ItemIndex-1].sItemInfo.NotUseFlag){
				PlusMinD = cInvenTory.InvenItem[sInven[0].ItemIndex-1].sItemInfo.Damage[0];
				PlusMaxD = cInvenTory.InvenItem[sInven[0].ItemIndex-1].sItemInfo.Damage[1];
			}
			*/
			//PlusMinD += sinChar->Spirit/DivideMin;
			//PlusMaxD += sinChar->Spirit/DivideMax;

			// 장별 - 스킬밸런스 조정(10.08.10)
			PlusMinD += (cInvenTory.InvenItem[sInven[0].ItemIndex-1].sItemInfo.Damage[0]/3);
			PlusMaxD += (cInvenTory.InvenItem[sInven[0].ItemIndex-1].sItemInfo.Damage[1]/3);
			PlusMinD += sinChar->Spirit/8;
			PlusMaxD += sinChar->Spirit/6;

			lstrcpy(szTempSkillDoc,FireDamage2); 
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d(+%d)-%d(+%d)\r",Dancing_Sword_FireDamage[pSkill->Point+i-1][0],PlusMinD,Dancing_Sword_FireDamage[pSkill->Point+i-1][1],PlusMaxD);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

			lstrcpy(szTempSkillDoc,IceDamage2); 
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d(+%d)-%d(+%d)\r",Dancing_Sword_IceDamage[pSkill->Point+i-1][0],PlusMinD,Dancing_Sword_IceDamage[pSkill->Point+i-1][1],PlusMaxD);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;
			
			lstrcpy(szTempSkillDoc,AttackDelay3);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d\r",Dancing_Sword_AttackDelay[pSkill->Point+i-1]);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

			lstrcpy(szTempSkillDoc,CountinueTime);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d%s\r",Dancing_Sword_Time[pSkill->Point+i-1],SecName);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

		break;
		case SKILL_FORCE_ORB:
			if(pSkill->ElementIndex == 1000) // 박재원 - 빌링 매직 포스 사용시 유지형 아이콘에 표시될 내용
			{
				lstrcpy(szTempSkillDoc,DamageAddName);     
				lstrcat(szSkillInfoBuff,szTempSkillDoc);
				wsprintf(szTempSkillDoc2,"%d%s\r",15,"%");  // 빌링 매직 포스 공격력가중:15%
				lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
				LineCount++;

				lstrcpy(szTempSkillDoc,CountinueTime);
				lstrcat(szSkillInfoBuff,szTempSkillDoc);
				wsprintf(szTempSkillDoc2,"%d%s\r",BillingMagicForceOrbUseTime[pSkill->Point+i-1],SecName);
				lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
				LineCount++;
			}
			else
			{
				lstrcpy(szTempSkillDoc,sinAddDamage7);   
				lstrcat(szSkillInfoBuff,szTempSkillDoc);
				wsprintf(szTempSkillDoc2,"%d\r",ForceOrbDamage[pSkill->Point+i-1]);
				lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
				LineCount++;
				if(pSkill->Point >= 7 && pSkill->Point <= 10){ // 머키 포스 부터 공격력 가중이 붙는다.
					lstrcpy(szTempSkillDoc,DamageAddName);     // 머키 ~ 미라지 까지는 10% 공격력가중
					lstrcat(szSkillInfoBuff,szTempSkillDoc);
					wsprintf(szTempSkillDoc2,"%d%s\r",10,"%");
					lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
					LineCount++;
				}
				else if(pSkill->Point == 11){ // 인페르나 포스
					lstrcpy(szTempSkillDoc,DamageAddName);
					lstrcat(szSkillInfoBuff,szTempSkillDoc);
					wsprintf(szTempSkillDoc2,"%d%s\r",15,"%");
					lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
					LineCount++;
				}
				else if(pSkill->Point == 12){ // 이니그마 포스
					lstrcpy(szTempSkillDoc,DamageAddName);
					lstrcat(szSkillInfoBuff,szTempSkillDoc);
					wsprintf(szTempSkillDoc2,"%d%s\r",20,"%");
					lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
					LineCount++;
				}
				lstrcpy(szTempSkillDoc,CountinueTime);
				lstrcat(szSkillInfoBuff,szTempSkillDoc);
				wsprintf(szTempSkillDoc2,"%d%s\r",ForceOrbUseTime[pSkill->Point+i-1],SecName); // 박재원 - 유지시간은 일반 포스와 매직 포스가 동일하므로 같이 사용한다. ForceOrbUseTime[]
				lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
				LineCount++;
			}
			break;
		break;
		case CHANGE_JOB3_QUEST:
			lstrcpy(szTempSkillDoc,sinLimitTiem2);         
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d%s\r",70,sinMinute2);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

			//lstrcpy(szTempSkillDoc,szQuestMonsterName);   
			wsprintf(szTempSkillDoc,"%s: \r",szQuestMonsterName);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"(%d/%d)\r",sinQuest_ChangeJob4.Monster[0],sinQuest_ChangeJob4.Monster[1]);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

		break;
		case CHANGE_ELEMENTARY_QUEST:
			if(chaQuest.sHaQuest100LV.CODE)
			{
				wsprintf(szTempSkillDoc,"%s: \r",chaQuest.szQuestMonsterName);
				lstrcat(szSkillInfoBuff,szTempSkillDoc);
				wsprintf(szTempSkillDoc2,"(%d/%d)\r",chaQuest.sHaQuest100LV.Monster[0],chaQuest.sHaQuest100LV.Monster[1]);
				lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
				LineCount++;
			}
			else
			{
				wsprintf(szTempSkillDoc,"%s: \r",chaQuest.szQuestMonsterName);
				lstrcat(szSkillInfoBuff,szTempSkillDoc);
				wsprintf(szTempSkillDoc2,"(%d/%d)\r",chaQuest.sHaQuestElementary[chaQuest.QuestKeyCode].Monster[0],chaQuest.sHaQuestElementary[chaQuest.QuestKeyCode].Monster[1]);
				lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
				LineCount++;
			}
		break;
		//*****************************************************/
		//*           템스크론   4차스킬                       /
		//*****************************************************/
        //[메카니션]
		case SKILL_IMPULSION:
			lstrcpy(szTempSkillDoc,LinghtingAddDamage4);//라이트닝 추가 공력
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d-%d\r",Impulsion_LightingDamage[pSkill->Point+i-1][0],Impulsion_LightingDamage[pSkill->Point+i-1][1]);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

			lstrcpy(szTempSkillDoc,Area19); //주변사정거리
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d\r",Impulsion_Range[pSkill->Point+i-1]);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;
 
			lstrcpy(szTempSkillDoc,MaxBoltNum4); //최대 볼트수
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d\r",Impulsion_LightingNum[pSkill->Point+i-1]);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;
		break;
		case SKILL_COMPULSION: 
			lstrcpy(szTempSkillDoc,AddAbsorb4); //흡수력 추가
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d\r",Compulsion_AddAbsorb[pSkill->Point+i-1]);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

			lstrcpy(szTempSkillDoc,IncreArea4);//범위증가
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d\r",Compulsion_Area[pSkill->Point+i-1]);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

			lstrcpy(szTempSkillDoc,CountinueTime);//유지시간
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d%s\r",Compulsion_Time[pSkill->Point+i-1],SecName);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;
		break;
		case SKILL_MAGNETIC_SPHERE:
			lstrcpy(szTempSkillDoc,IncreAttack4);//자기 공격력 증가
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d-%d\r",Magnetic_Sphere_AddDamage[pSkill->Point+i-1][0],Magnetic_Sphere_AddDamage[pSkill->Point+i-1][1]);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

			lstrcpy(szTempSkillDoc,Area18); //유효반경
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d\r",Magnetic_Sphere_Area[pSkill->Point+i-1]);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

			lstrcpy(szTempSkillDoc,AttackDelay3); //공격 딜레이
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d\r",Magnetic_Sphere_AttackDelay[pSkill->Point+i-1]);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

			lstrcpy(szTempSkillDoc,CountinueTime);//유지시간
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d%s\r",Magnetic_Sphere_Time[pSkill->Point+i-1],SecName);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;
		break;
		case SKILL_METAL_GOLEM:
			lstrcpy(szTempSkillDoc,sinAttackType);//공격타입
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			lstrcpy(szTempSkillDoc2,sinAttackType2[SummonSkillCodeIndex[0]]);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

			lstrcpy(szTempSkillDoc,CountinueTime);//유지시간
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d%s\r",360,SecName);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

			lstrcpy(szTempSkillDoc,DamageName); //공격력
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d-%d\r",Metal_Golem_Damage[pSkill->Point+i-1][0],Metal_Golem_Damage[pSkill->Point+i-1][1]);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

			lstrcpy(szTempSkillDoc,WolverinRate4);//명중력
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d\r",Metal_Golem_Hit[pSkill->Point+i-1]);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

			lstrcpy(szTempSkillDoc,WolverinDefense4);//방어력
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d\r",Metal_Golem_Defense[pSkill->Point+i-1]);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;
			
			lstrcpy(szTempSkillDoc,GolemLife4); //생명력+(힘)
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d+(%d)\r",Metal_Golem_Life[pSkill->Point+i-1],(sinChar->Strength*2));
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;
		break;
		//[파이터]
		case SKILL_DETORYER:
			lstrcpy(szTempSkillDoc,AddPercentDamage4);//공격력가중
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d%s\r",Destoryer_DamagePercent[pSkill->Point+i-1],"%");
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

			lstrcpy(szTempSkillDoc,Add_CriticalName);//크리티컬추가
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d\r",Destoryer_AddCritical[pSkill->Point+i-1]);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;
		break;
		case SKILL_BERSERKER:
			lstrcpy(szTempSkillDoc,PlusDamageName);//공력력 추가
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d\r",Berserker_AddAttack[pSkill->Point+i-1]);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

			lstrcpy(szTempSkillDoc,SubAbsorb4);//흠수력 하락
			lstrcat(szSkillInfoBuff,szTempSkillDoc); 
			wsprintf(szTempSkillDoc2,"%d\r",Berserker_SubAbsorb[pSkill->Point+i-1]);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

			lstrcpy(szTempSkillDoc,CountinueTime);//유지시간
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d%s\r",Berserker_Time[pSkill->Point+i-1],SecName);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;
        break;
		case SKILL_CYCLONE_STRIKE:
			lstrcpy(szTempSkillDoc,AddPercentDamage4);//공격력가중
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d%s\r",Cyclone_Strike_DamagePercent[pSkill->Point+i-1],"%");
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

			lstrcpy(szTempSkillDoc,AreaDamage4);//범위 공격력
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d-%d\r",Cyclone_Strike_AreaDamage[pSkill->Point+i-1][0],Cyclone_Strike_AreaDamage[pSkill->Point+i-1][1]);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

			lstrcpy(szTempSkillDoc,MaxAttackNum4); //최대 타격수
			lstrcat(szSkillInfoBuff,szTempSkillDoc); 
			wsprintf(szTempSkillDoc2,"%d\r",Cyclone_Strike_AttackNum[pSkill->Point+i-1]);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

			lstrcpy(szTempSkillDoc,AttackAreaName);//흠수력 하락
			lstrcat(szSkillInfoBuff,szTempSkillDoc); 
			wsprintf(szTempSkillDoc2,"%d\r",Cyclone_Strike_Area[pSkill->Point+i-1]);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;
        break;
		case SKILL_BOOST_HEALTH:
			lstrcpy(szTempSkillDoc,LifeUp4); //생명력 추가량
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d\r",Boost_Health_Life[pSkill->Point+i-1]);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;
        break;
// one tooltip line for a tier 5 skill: the label (ending in \r), then the value
#define T5TIP(label, ...) { lstrcpy(szTempSkillDoc,label); lstrcat(szSkillInfoBuff,szTempSkillDoc); \
	wsprintf(szTempSkillDoc2,__VA_ARGS__); lstrcat(szSkillInfoBuff2,szTempSkillDoc2); LineCount++; }
#define T5P (pSkill->Point+i-1)
		//[Mechanician] tier 5
		case SKILL_LAND_MINING:
			T5TIP(AddPercentDamage4, "%d%s\r", Land_Mining_DamagePercent[T5P], "%");
			T5TIP(AreaDamage4, "%d-%d\r", Land_Mining_AreaDamage[T5P][0], Land_Mining_AreaDamage[T5P][1]);
			T5TIP(MaxAttackNum4, "%d\r", Land_Mining_AttackNum[T5P]);
			T5TIP(AttackAreaName, "%d\r", Land_Mining_Area[T5P]);
		break;
		case SKILL_HYPER_SONIC:
			T5TIP("Attack Speed:\r", "%d%s\r", Hyper_Sonic_Speed[T5P], "%");
			T5TIP(CountinueTime, "%d%s\r", Hyper_Sonic_Time[T5P], SecName);
		break;
		case SKILL_ROLLING_SMASH:
			T5TIP(AddPercentDamage4, "%d%s\r", Rolling_Smash_DamagePercent[T5P], "%");
			T5TIP("Attack Rating:\r", "+%d\r", Rolling_Smash_AddHit[T5P]);
		break;
		case SKILL_POWER_ENHANCEMENT:
			T5TIP("Attack Power:\r", "+%d%s\r", Power_Enhancement_Damage[T5P], "%");
			T5TIP(CountinueTime, "%d%s\r", Power_Enhancement_Time[T5P], SecName);
		break;
		//[Archer] tier 5
		case SKILL_EVADE_SHOT:
			T5TIP(AddPercentDamage4, "%d%s\r", Evade_Shot_DamagePercent[T5P], "%");
			T5TIP("Stun Time:\r", "%d%s\r", Evade_Shot_StunTime[T5P], SecName);
		break;
		case SKILL_THUNDER_ROPE:
			T5TIP(AddPercentDamage4, "%d%s\r", Thunder_Rope_DamagePercent[T5P], "%");
			T5TIP("Attack Rating:\r", "+%d%s\r", Thunder_Rope_AddHit[T5P], "%");
			T5TIP("Shot Range:\r", "+%d\r", Thunder_Rope_Range[T5P]);
		break;
		case SKILL_NATURE_SPLASH:
			T5TIP(AddPercentDamage4, "%d%s\r", Nature_Splash_DamagePercent[T5P], "%");
			T5TIP(AttackAreaName, "%d\r", Nature_Splash_Area[T5P]);
		break;
		case SKILL_CIRCLE_TRAP:
			T5TIP(AddPercentDamage4, "%d%s\r", Circle_Trap_DamagePercent[T5P], "%");
			T5TIP(AttackAreaName, "%d\r", Circle_Trap_Area[T5P]);
			T5TIP("Slow:\r", "%d%s\r", Circle_Trap_Slow[T5P], "%");
			T5TIP("Slow Time:\r", "%d.%d%s\r", Circle_Trap_SlowTime10[T5P]/10, Circle_Trap_SlowTime10[T5P]%10, SecName);
		break;
		//[Pikeman] tier 5
		case SKILL_DANCING_REAPER:
			T5TIP(AddPercentDamage4, "%d%s\r", Dancing_Reaper_DamagePercent[T5P], "%");
		break;
		case SKILL_FINAL_SPEAR:
			T5TIP(AreaDamage4, "%d-%d\r", Final_Spear_AreaDamage[T5P][0], Final_Spear_AreaDamage[T5P][1]);
			T5TIP(AttackAreaName, "%d\r", Final_Spear_Area[T5P]);
		break;
		case SKILL_AMPLIFIED:
			T5TIP("Attack Power:\r", "+%d%s\r", Amplified_Damage[T5P], "%");
			T5TIP("Attack Rating:\r", "+%d%s\r", Amplified_AddHit[T5P], "%");
			T5TIP("Absorption:\r", "%d\r", Amplified_SubAbsorb[T5P]);
			T5TIP(CountinueTime, "%d%s\r", Amplified_Time[T5P], SecName);
		break;
		case SKILL_SIDE_SPIN_ATTACK:
			T5TIP(AddPercentDamage4, "%d%s\r", Side_Spin_DamagePercent[T5P], "%");
			T5TIP("Life Drain:\r", "%d%s\r", Side_Spin_LifeAbsorb[T5P], "%");
		break;
		//[Fighter] tier 5
		case SKILL_DOWN_HIT:
			lstrcpy(szTempSkillDoc,AddPercentDamage4);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d%s\r",Down_Hit_DamagePercent[pSkill->Point+i-1],"%");
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;
		break;
		case SKILL_POWER_DASH:
			lstrcpy(szTempSkillDoc,AddPercentDamage4);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d%s\r",Power_Dash_DamagePercent[pSkill->Point+i-1],"%");
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

			lstrcpy(szTempSkillDoc,"Stun Time:\r");
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d%s\r",Power_Dash_StunTime[pSkill->Point+i-1],SecName);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;
		break;
		case SKILL_MORTAL_BLOW:
			lstrcpy(szTempSkillDoc,AddPercentDamage4);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d%s\r",Mortal_Blow_DamagePercent[pSkill->Point+i-1],"%");
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

			lstrcpy(szTempSkillDoc,AreaDamage4);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d-%d\r",Mortal_Blow_AreaDamage[pSkill->Point+i-1][0],Mortal_Blow_AreaDamage[pSkill->Point+i-1][1]);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

			lstrcpy(szTempSkillDoc,MaxAttackNum4);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d\r",Mortal_Blow_AttackNum[pSkill->Point+i-1]);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

			lstrcpy(szTempSkillDoc,AttackAreaName);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d\r",Mortal_Blow_Area[pSkill->Point+i-1]);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;
		break;
		case SKILL_BLOODY_BERSERKER:
			lstrcpy(szTempSkillDoc,PlusDamageName);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d\r",Bloody_Berserker_AddAttack[pSkill->Point+i-1]);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

			lstrcpy(szTempSkillDoc,SubAbsorb4);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d\r",Bloody_Berserker_SubAbsorb[pSkill->Point+i-1]);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

			lstrcpy(szTempSkillDoc,CountinueTime);
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d%s\r",Bloody_Berserker_Time[pSkill->Point+i-1],SecName);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;
		break;
		//[파이크맨]
		case SKILL_ASSASSIN_EYE:
			lstrcpy(szTempSkillDoc,CriticalAddDamage4);//크리티컬 공격력 증가
			lstrcat(szSkillInfoBuff,szTempSkillDoc); 
			wsprintf(szTempSkillDoc2,"%d\r",Assassin_Eye_AddCritical[pSkill->Point+i-1]);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

			lstrcpy(szTempSkillDoc,MonstervsSubCritical4);//몬스터의 대 크리티컬 능력약화
			lstrcat(szSkillInfoBuff,szTempSkillDoc); 
			wsprintf(szTempSkillDoc2,"%d\r",Assassin_Eye_M_SubCritical[pSkill->Point+i-1]);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

			lstrcpy(szTempSkillDoc,CountinueTime);//유지시간
			lstrcat(szSkillInfoBuff,szTempSkillDoc); 
			wsprintf(szTempSkillDoc2,"%d%s\r",Assassin_Eye_Time[pSkill->Point+i-1],SecName);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;
		break;
		case SKILL_CHARGING_STRIKE:
			lstrcpy(szTempSkillDoc,AddPercentDamage4);//공격력가중
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d%s\r",Charging_Strike_DamagePercent[pSkill->Point+i-1],"%");
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

			lstrcpy(szTempSkillDoc,ChargingAddPercentDamage4);//차징시 공격력가중
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d%s\r",Charging_Strike_CharingDamagePercent[pSkill->Point+i-1],"%");
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

			lstrcpy(szTempSkillDoc,ChargingTime4);//차징시간
			lstrcat(szSkillInfoBuff,szTempSkillDoc); 
			if((pSkill->Point+i)==1){
				wsprintf(szTempSkillDoc2,"%d%s\r",0,"%");
			}
			else
				wsprintf(szTempSkillDoc2,"%d%s\r",Charging_Strike_Time[(pSkill->Point+i-1)-1],"%");
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;
		break;
		case SKILL_VAGUE:
			lstrcpy(szTempSkillDoc,IncreEvasionPercent4);//추가 회피 확률 증가
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d%s\r",Vague_EvasionPercent[pSkill->Point+i-1],"%");
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

			lstrcpy(szTempSkillDoc,CountinueTime);//유지시간
			lstrcat(szSkillInfoBuff,szTempSkillDoc); 
			wsprintf(szTempSkillDoc2,"%d%s\r",Vague_Time[pSkill->Point+i-1],SecName);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;
		break;
		case SKILL_SHADOW_MASTER:
			lstrcpy(szTempSkillDoc,IncreAttack4); //공격력증가
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d%s\r",Shadow_Master_DamagePercent[pSkill->Point+i-1],"%");
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;
           /*
			lstrcpy(szTempSkillDoc,IncreAttack_RateName);//명중력증가
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d\r",Shadow_Master_AddHit[pSkill->Point+i-1]);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;
		*/
			lstrcpy(szTempSkillDoc,AddShadowNum4);//잔상 콤보 증가
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d\r",Shadow_Master_ShadowNum[pSkill->Point+i-1]);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;
		break;
		//[아쳐]
		case SKILL_RECALL_WOLVERIN:

			lstrcpy(szTempSkillDoc,sinAttackType);//공격타입
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			lstrcpy(szTempSkillDoc2,sinAttackType2[SummonSkillCodeIndex[2]]);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

			lstrcpy(szTempSkillDoc,CountinueTime);//유지시간
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d%s\r",360,SecName);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

			lstrcpy(szTempSkillDoc,DamageName);//공격력
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d-%d\r",Recall_Wolverin_Damage[pSkill->Point+i-1][0],Recall_Wolverin_Damage[pSkill->Point+i-1][1]);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

			lstrcpy(szTempSkillDoc,WolverinRate4);//명중력
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d\r",Recall_Wolverin_Hit[pSkill->Point+i-1]);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

			lstrcpy(szTempSkillDoc,WolverinDefense4);//방어력
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d+(%d)\r",Recall_Wolverin_Defense[pSkill->Point+i-1],(sinChar->Dexterity*2));
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;
			
			lstrcpy(szTempSkillDoc,WolverinLife4);//생명력
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d\r",Recall_Wolverin_Life[pSkill->Point+i-1]);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;
		break;
		case SKILL_EVASION_MASTERY:
			lstrcpy(szTempSkillDoc,AddEvasion4);//추가 회피 능력 증가
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d%s\r",Evasion_Mastery_AddPercent[pSkill->Point+i-1],"%");
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;
		break;

		case SKILL_PHOENIX_SHOT:
			
			lstrcpy(szTempSkillDoc,AddPercentDamage4);//공격력가중
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d%s\r",Phoenix_Shot_DamagePercent[pSkill->Point+i-1],"%");
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

			lstrcpy(szTempSkillDoc,ChargingTime4);//피닉스타임
			lstrcat(szSkillInfoBuff,szTempSkillDoc); 
			sinTemp1=(int)((Phoenix_Shot_Time[pSkill->Point+i-1]+0.001f)*10);
			RightSpot=sinTemp1%10;
			LeftSpot=(sinTemp1-RightSpot)/10;

			wsprintf(szTempSkillDoc2,"%d.%d%s\r",LeftSpot,RightSpot,SecName);

			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;
		break;
		case SKILL_FORCE_OF_NATURE:
			lstrcpy(szTempSkillDoc,AddDamage4);//추가 공격력증가
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d\r",Force_Of_Nature_AddDamage[pSkill->Point+i-1]);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

			lstrcpy(szTempSkillDoc,IncreAttack_RateName);//명중력증가
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d\r",Force_Of_Nature_AddHit[pSkill->Point+i-1]);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

			lstrcpy(szTempSkillDoc,FalconAddDamage4);//falcon 추가 공격력 증가
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d\r",Force_Of_Nature_FalconAddDamage[pSkill->Point+i-1]);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

			lstrcpy(szTempSkillDoc,CountinueTime);//유지시간
			lstrcat(szSkillInfoBuff,szTempSkillDoc); 
			wsprintf(szTempSkillDoc2,"%d%s\r",Force_Of_Nature_Time[pSkill->Point+i-1],SecName);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;
	
		break;
		//*****************************************************/
		//*           모라이온   4차스킬                       /
		//*****************************************************/
        //[나이트]
		case SKILL_DIVINE_PIERCING:
			lstrcpy(szTempSkillDoc,AddPercentDamage4); //공격력가중
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d%s\r",Divine_Piercing_DamagePercent[pSkill->Point+i-1],"%");
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;
          /*   
			lstrcpy(szTempSkillDoc,Attck_RateAdd5); //명중력추가
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d\r",Divine_Piercing_AddHit[pSkill->Point+i-1]);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;
		  */
			lstrcpy(szTempSkillDoc,AttackNum3);  //연속 공격 횟수
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d\r",Divine_Piercing_AttackNum[pSkill->Point+i-1]);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;
		break;
		case SKILL_GODLY_SHIELD:
			lstrcpy(szTempSkillDoc,IncreAttackAbsorb4);//공격 흡수력 증가
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d%s\r",Godly_Shield_AbsorbPercent[pSkill->Point+i-1],"%");
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

			lstrcpy(szTempSkillDoc,CountinueTime); //유지시간
			lstrcat(szSkillInfoBuff,szTempSkillDoc); 
			wsprintf(szTempSkillDoc2,"%d%s\r",Godly_Shield_Time[pSkill->Point+i-1],SecName);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;
		
		break;

		case SKILL_GOD_BLESS:
			lstrcpy(szTempSkillDoc,IncreAttack4);//공격력증가
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d\r",God_Bless_AddDamage[pSkill->Point+i-1]);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

			lstrcpy(szTempSkillDoc,CountinueTime);//유지시간
			lstrcat(szSkillInfoBuff,szTempSkillDoc); 
			wsprintf(szTempSkillDoc2,"%d%s\r",God_Bless_Time[pSkill->Point+i-1],SecName);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;
	
		break;
		case SKILL_SWORD_OF_JUSTICE:
			lstrcpy(szTempSkillDoc,AddPercentDamage4);//공격력가중
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d%s\r",Sword_Of_Justice_DamagePercent[pSkill->Point+i-1],"%");
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

			lstrcpy(szTempSkillDoc,AttackAreaName);//공격력 범위
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d\r",Sword_Of_Justice_Area[pSkill->Point+i-1]);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

		break;
		//[아틀란타]
		case SKILL_HALL_OF_VALHALLA:
			lstrcpy(szTempSkillDoc,IncreEvasionPercent4); //추가 회피 확률 증가
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d%s\r",Hall_Of_Valhalla_AddEvation[pSkill->Point+i-1],"%");
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

			lstrcpy(szTempSkillDoc,CountinueTime); //유지시간
			lstrcat(szSkillInfoBuff,szTempSkillDoc); 
			wsprintf(szTempSkillDoc2,"%d%s\r",Hall_Of_Valhalla_Time[pSkill->Point+i-1],SecName);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;
	
		break;
		case SKILL_X_RAGE:
			lstrcpy(szTempSkillDoc,AddPercentDamage4);//공격력가중
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d%s\r",X_Rage_DamagePercent[pSkill->Point+i-1],"%");
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

			lstrcpy(szTempSkillDoc,RectAngleArea4);//직각공격 범위
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d\r",X_Rage_Area[pSkill->Point+i-1]);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;
        break;
		case SKILL_FROST_JAVELIN:
			lstrcpy(szTempSkillDoc,AddIceDamage4);//ice 추가 공력
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d-%d\r",Frost_Javelin_IceAddDamage[pSkill->Point+i-1][0],Frost_Javelin_IceAddDamage[pSkill->Point+i-1][1]);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

		    lstrcpy(szTempSkillDoc,MonsterSubSpeed4);//타격된 적의 속도저하치
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d%s\r",Frost_Javelin_SpeedSubPercent[pSkill->Point+i-1],"%");
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

			lstrcpy(szTempSkillDoc,IceTime4);//아이싱시간
			lstrcat(szSkillInfoBuff,szTempSkillDoc); 
			wsprintf(szTempSkillDoc2,"%d%s\r",Frost_Javelin_IceTime[pSkill->Point+i-1],SecName);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

			lstrcpy(szTempSkillDoc,CountinueTime);//유지시간
			lstrcat(szSkillInfoBuff,szTempSkillDoc); 
			wsprintf(szTempSkillDoc2,"%d%s\r",Frost_Javelin_Time[pSkill->Point+i-1],SecName);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;
		
		break;
		case SKILL_VENGEANCE:
			lstrcpy(szTempSkillDoc,AddPercentDamage4);//공격력가중
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d%s\r",Vengeance_DamagePercent[pSkill->Point+i-1],"%");
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

			// 장별 - 스킬밸런스 조정(10.08.10)
			lstrcpy(szTempSkillDoc,Add_CriticalName);//크리티컬추가
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d%s\r",Vengeance_AddCritical[pSkill->Point+i-1],"%");
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;


			lstrcpy(szTempSkillDoc,ShootingRangeName);//공격력 범위
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d\r",Vengeance_Range[pSkill->Point+i-1]);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;
		break;
		//[프리스티스]
		case SKILL_GLACIAL_SPIKE:
			PlusMinD = 0;
			PlusMaxD = 0;

			/*
			if(sInven[0].ItemIndex && !cInvenTory.InvenItem[sInven[0].ItemIndex-1].sItemInfo.NotUseFlag){
				PlusMinD = cInvenTory.InvenItem[sInven[0].ItemIndex-1].sItemInfo.Damage[0];
				PlusMaxD = cInvenTory.InvenItem[sInven[0].ItemIndex-1].sItemInfo.Damage[1];
			}
			PlusMinD += sinChar->Spirit/DivideMin;
			PlusMaxD += sinChar->Spirit/DivideMax;
			*/

			// 장별 - 스킬밸런스 조정(10.08.10)
			PlusMinD = sinChar->Level/2;
			PlusMaxD = sinChar->Level/2;

			PlusMinD += sinChar->Spirit/6;
			PlusMaxD += sinChar->Spirit/3;	
			

			lstrcpy(szTempSkillDoc,DamageName);//공격력
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d(+%d)-%d(+%d)\r",Glacial_Spike_Damage[pSkill->Point+i-1][0],PlusMinD,Glacial_Spike_Damage[pSkill->Point+i-1][1],PlusMaxD);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

			lstrcpy(szTempSkillDoc,AttackAreaName);//공격력 범위
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d\r",Glacial_Spike_Area[pSkill->Point+i-1]);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;
		break;
		case SKILL_REGENERATION_FIELD:

			lstrcpy(szTempSkillDoc,AddLifeRegen4);//생명 재생력 추가
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
	        TempLife=(int)(Regeneration_Field_LifeRegen[pSkill->Point+i-1]*10+0.001f);
			RightSpot=TempLife%10;
			LeftSpot=(TempLife-RightSpot)/10;
			wsprintf(szTempSkillDoc2,"%d.%d\r",LeftSpot,RightSpot);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

			lstrcpy(szTempSkillDoc,AddManaRegen4);//마나 재생력추가
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
	        TempMana=(int)(Regeneration_Field_ManaRegen[pSkill->Point+i-1]*10+0.001f);
			RightSpot=TempMana%10;
			LeftSpot=(TempMana-RightSpot)/10;
			wsprintf(szTempSkillDoc2,"%d.%d\r",LeftSpot,RightSpot);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

			lstrcpy(szTempSkillDoc,CountinueTime);//유지시간
			lstrcat(szSkillInfoBuff,szTempSkillDoc); 
			wsprintf(szTempSkillDoc2,"%d%s\r",Regeneration_Field_Time[pSkill->Point+i-1],SecName);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

		    lstrcpy(szTempSkillDoc,MagicArea4);//시전 범위
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d\r",Regeneration_Field_Area[pSkill->Point+i-1]);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;
				
		break;
		case SKILL_CHAIN_LIGHTNING:
			PlusMinD = 0;
			PlusMaxD = 0;

			/*
			if(sInven[0].ItemIndex && !cInvenTory.InvenItem[sInven[0].ItemIndex-1].sItemInfo.NotUseFlag){
				PlusMinD = cInvenTory.InvenItem[sInven[0].ItemIndex-1].sItemInfo.Damage[0];
				PlusMaxD = cInvenTory.InvenItem[sInven[0].ItemIndex-1].sItemInfo.Damage[1];
			}
			PlusMinD += sinChar->Spirit/DivideMin;
			PlusMaxD += sinChar->Spirit/DivideMax;
			*/
		
			// 장별 - 스킬밸런스 조정(10.08.10)

			PlusMinD = sinChar->Level/3;
			PlusMaxD = sinChar->Level/3;

			PlusMinD += cInvenTory.InvenItem[sInven[0].ItemIndex-1].sItemInfo.Damage[0]/3;
			PlusMaxD += cInvenTory.InvenItem[sInven[0].ItemIndex-1].sItemInfo.Damage[1]/3;

			PlusMinD += sinChar->Spirit/6;
			PlusMaxD += sinChar->Spirit/3;


			lstrcpy(szTempSkillDoc,DamageName);//공격력
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d(+%d)-%d(+%d)\r",Chain_Lightning_Damage[pSkill->Point+i-1][0],PlusMinD,Chain_Lightning_Damage[pSkill->Point+i-1][1],PlusMaxD);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

			lstrcpy(szTempSkillDoc,ChainNum4);//최대 연결갯수
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d\r",Chain_Lightning_Num[pSkill->Point+i-1]);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;
  
			lstrcpy(szTempSkillDoc,ChainRange4);//최대 연결거리
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d\r",Chain_Lightning_Range[pSkill->Point+i-1]);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;
    	
		break;
		case SKILL_SUMMON_MUSPELL:

			lstrcpy(szTempSkillDoc,DamageName);//공격력
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d-%d\r",Summon_Muspell_Damage[pSkill->Point+i-1][0],Summon_Muspell_Damage[pSkill->Point+i-1][1]);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

			lstrcpy(szTempSkillDoc,AttackDelay3 );//공격딜레이
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d\r",Summon_Muspell_Attack_Delay[pSkill->Point+i-1]);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

			lstrcpy(szTempSkillDoc,UndeadDamageAbsorb4);//언데드의 공격력 흡수치
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d%s\r",Summon_Muspell_UndeadAbsorbPercent[pSkill->Point+i-1],"%");
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

			lstrcpy(szTempSkillDoc,UserBlockPercent4);//유저 블럭율
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d%s\r",Summon_Muspell_BlockPercent[pSkill->Point+i-1],"%");
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

			lstrcpy(szTempSkillDoc,CountinueTime);//유지시간
			lstrcat(szSkillInfoBuff,szTempSkillDoc); 
			wsprintf(szTempSkillDoc2,"%d%s\r",Summon_Muspell_Time[pSkill->Point+i-1],SecName);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

		break;
		//[메지션]
		case SKILL_FIRE_ELEMENTAL:

			lstrcpy(szTempSkillDoc,sinAttackType);//공격타입
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			lstrcpy(szTempSkillDoc2,sinAttackType2[SummonSkillCodeIndex[1]]);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;


			lstrcpy(szTempSkillDoc,CountinueTime);//유지시간
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d%s\r",360,SecName);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

			lstrcpy(szTempSkillDoc,DamageName);     //공격력
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d-%d\r",Fire_Elemental_Damage[pSkill->Point+i-1][0],Fire_Elemental_Damage[pSkill->Point+i-1][1]);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

			lstrcpy(szTempSkillDoc,WolverinRate4);   //명중력
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d\r",Fire_Elemental_Hit[pSkill->Point+i-1]);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

			lstrcpy(szTempSkillDoc,WolverinLife4);    //생명력
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d\r",(Fire_Elemental_Life[pSkill->Point+i-1]));
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

		break;
		case SKILL_FLAME_WAVE:
			PlusMinD = 0;
			PlusMaxD = 0;

			/*
			if(sInven[0].ItemIndex && !cInvenTory.InvenItem[sInven[0].ItemIndex-1].sItemInfo.NotUseFlag){
				PlusMinD = cInvenTory.InvenItem[sInven[0].ItemIndex-1].sItemInfo.Damage[0];
				PlusMaxD = cInvenTory.InvenItem[sInven[0].ItemIndex-1].sItemInfo.Damage[1];
			}
			PlusMinD += sinChar->Spirit/DivideMin;
			PlusMaxD += sinChar->Spirit/DivideMax;
			*/

			// 장별 - 스킬밸런스 조정(10.08.10)
			PlusMinD = sinChar->Level/2;
			PlusMaxD = sinChar->Level/2;

			PlusMinD += sinChar->Spirit/6;
			PlusMaxD += sinChar->Spirit/3;		


			TempPoint = SearchContiueSkill(SKILL_SPIRIT_ELEMENTAL);
			if(TempPoint){
				PlusMinD += (Flame_Wave_FireDamage[pSkill->Point-1][0] * Spirit_Elemental_Damage[TempPoint-1])/100;
				PlusMaxD += (Flame_Wave_FireDamage[pSkill->Point-1][1] * Spirit_Elemental_Damage[TempPoint-1])/100;
			}

			lstrcpy(szTempSkillDoc,FireDamage4);//불 공격력
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d(+%d)-%d(+%d)\r",Flame_Wave_FireDamage[pSkill->Point+i-1][0],PlusMinD,Flame_Wave_FireDamage[pSkill->Point+i-1][1],PlusMaxD);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;
	
			lstrcpy(szTempSkillDoc,AttackAreaName);//공격력 범위
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d\r",Flame_Wave_Area[pSkill->Point+i-1]);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;
	
		break;
		case SKILL_DISTORTION:
			lstrcpy(szTempSkillDoc,DecreSpeed4);//속도 저하
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d%s\r",Distortion_SpeedSubPercent[pSkill->Point+i-1],"%");
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

			lstrcpy(szTempSkillDoc,DecreAttack4);//공격 저하
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d%s\r",Distortion_DamageSubPercent[pSkill->Point+i-1],"%");
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

		    lstrcpy(szTempSkillDoc,MagicArea4); //시전 범위
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d\r",Distortion_Area[pSkill->Point+i-1]);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;
		
			lstrcpy(szTempSkillDoc,CountinueTime);//유지시간
			lstrcat(szSkillInfoBuff,szTempSkillDoc); 
			wsprintf(szTempSkillDoc2,"%d%s\r",Distortion_Time[pSkill->Point+i-1],SecName);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

		break;
		case SKILL_M_METEO:
			PlusMinD = 0;
			PlusMaxD = 0;

			/*
			if(sInven[0].ItemIndex && !cInvenTory.InvenItem[sInven[0].ItemIndex-1].sItemInfo.NotUseFlag){
				PlusMinD = cInvenTory.InvenItem[sInven[0].ItemIndex-1].sItemInfo.Damage[0];
				PlusMaxD = cInvenTory.InvenItem[sInven[0].ItemIndex-1].sItemInfo.Damage[1];
			}
			PlusMinD += sinChar->Spirit/DivideMin;
			PlusMaxD += sinChar->Spirit/DivideMax;
			*/

			// 장별 - 스킬밸런스 조정(10.08.10)
			PlusMinD = sinChar->Level/3;
			PlusMaxD = sinChar->Level/3;

			PlusMinD += cInvenTory.InvenItem[sInven[0].ItemIndex-1].sItemInfo.Damage[0]/3;
			PlusMaxD += cInvenTory.InvenItem[sInven[0].ItemIndex-1].sItemInfo.Damage[1]/3;

			PlusMinD += sinChar->Spirit/6;
			PlusMaxD += sinChar->Spirit/3;

			TempPoint = SearchContiueSkill(SKILL_SPIRIT_ELEMENTAL);
			if(TempPoint){
				PlusMinD += (Meteo_Damage[pSkill->Point-1][0] * Spirit_Elemental_Damage[TempPoint-1])/100;
				PlusMaxD += (Meteo_Damage[pSkill->Point-1][1] * Spirit_Elemental_Damage[TempPoint-1])/100;
			}

		
			lstrcpy(szTempSkillDoc,DamageName);//공격력
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d(+%d)-%d(+%d)\r",Meteo_Damage[pSkill->Point+i-1][0],PlusMinD,Meteo_Damage[pSkill->Point+i-1][1],PlusMaxD);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;

			lstrcpy(szTempSkillDoc,AttackAreaName);//공격력 범위
			lstrcat(szSkillInfoBuff,szTempSkillDoc);
			wsprintf(szTempSkillDoc2,"%d\r",Meteo_Area[pSkill->Point+i-1]);
			lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
			LineCount++;
	
		break;
		}
		//기력 
		if(!Icon){
			if(pSkill->Skill_Info.UseMana){
				lstrcpy(szTempSkillDoc,UseManaName); 
				lstrcat(szSkillInfoBuff,szTempSkillDoc);

				/*
				if( chaPremiumitem.m_ManaReducePotiontime > 0 )	// pluto 마나 리듀스 포션 텍스트
				{
					wsprintf(szTempSkillDoc2,"%d(-%d)\r",pSkill->Skill_Info.UseMana[pSkill->Point+i-1], (pSkill->Skill_Info.UseMana[pSkill->Point+i-1] * chaPremiumitem.m_ManaReducePotionValue / 100) );
				}
				else
				{
					wsprintf(szTempSkillDoc2,"%d\r",pSkill->Skill_Info.UseMana[pSkill->Point+i-1]);
				}
				*/

				
				// 장별 - 테리 속성
				if( chaPremiumitem.m_ManaReducePotiontime > 0 && chaPremiumitem.m_TerryTime > 0 && sinChar->GravityScroolCheck[1] == 1)	// pluto 마나 리듀스 포션 텍스트 && 테리
				{
					wsprintf(szTempSkillDoc2,"%d(-%d)\r",pSkill->Skill_Info.UseMana[pSkill->Point+i-1], 
						(pSkill->Skill_Info.UseMana[pSkill->Point+i-1] * chaPremiumitem.m_ManaReducePotionValue / 100) + 
						(pSkill->Skill_Info.UseMana[pSkill->Point+i-1] * 20 / 100));
				}

				// pluto 마나 리듀스 포션 텍스트
				else if( chaPremiumitem.m_ManaReducePotiontime > 0 )
				{
					wsprintf(szTempSkillDoc2,"%d(-%d)\r",pSkill->Skill_Info.UseMana[pSkill->Point+i-1], (pSkill->Skill_Info.UseMana[pSkill->Point+i-1] * chaPremiumitem.m_ManaReducePotionValue / 100) );
				}

				// 레드 드래곤(테리) 사용 시 마나소비 15% 감소 텍스트 출력
				else if( chaPremiumitem.m_TerryTime > 0 && sinChar->GravityScroolCheck[1] == 1)
				{
					wsprintf(szTempSkillDoc2,"%d(-%d)\r",pSkill->Skill_Info.UseMana[pSkill->Point+i-1], (pSkill->Skill_Info.UseMana[pSkill->Point+i-1] * 20 / 100) );
				}

				// 아무것도 안썼을 때
				else
				{
					wsprintf(szTempSkillDoc2,"%d\r",pSkill->Skill_Info.UseMana[pSkill->Point+i-1]);
				}


				lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
				LineCount++;
			}
			//근력
			if(i==0 ){
				if(pSkill->Skill_Info.UseStamina[0]){
					lstrcpy(szTempSkillDoc,UseStaminaName);
					lstrcat(szSkillInfoBuff,szTempSkillDoc);
					if( chaPremiumitem.m_StaminaReducePotiontime > 0 )	// 박재원 - 근력 리듀스 포션
					{
						wsprintf(szTempSkillDoc2,"%d(-%d)\r",pSkill->Skill_Info.UseStamina[0]+(pSkill->Skill_Info.UseStamina[1]*(pSkill->Point-1)), ((pSkill->Skill_Info.UseStamina[0]+(pSkill->Skill_Info.UseStamina[1]*(pSkill->Point-1))) * chaPremiumitem.m_StaminaReducePotionValue / 100) );
					}
					else
					{
						wsprintf(szTempSkillDoc2,"%d\r",pSkill->Skill_Info.UseStamina[0]+(pSkill->Skill_Info.UseStamina[1]*(pSkill->Point-1)));
					}
					lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
					LineCount++;
					lstrcat(szSkillInfoBuff,"\r"); 
					lstrcpy(szTempSkillDoc2,"\r");
					lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
					LineCount++;
				}
			}
			if(i == 1){
				if(pSkill->Skill_Info.UseStamina[0]){
					lstrcpy(szTempSkillDoc,UseStaminaName);
					lstrcat(szSkillInfoBuff,szTempSkillDoc);
					if( chaPremiumitem.m_StaminaReducePotiontime > 0 )	// 박재원 - 근력 리듀스 포션
					{
						wsprintf(szTempSkillDoc2,"%d(-%d)\r",pSkill->Skill_Info.UseStamina[0]+(pSkill->Skill_Info.UseStamina[1]*(pSkill->Point)), ((pSkill->Skill_Info.UseStamina[0]+(pSkill->Skill_Info.UseStamina[1]*(pSkill->Point))) * chaPremiumitem.m_StaminaReducePotionValue / 100) );
					}
					else
					{
						wsprintf(szTempSkillDoc2,"%d\r",pSkill->Skill_Info.UseStamina[0]+(pSkill->Skill_Info.UseStamina[1]*(pSkill->Point)));
					}
					lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
					LineCount++;
				}

			}
		}	
	}

	if(SkillMasterFlag == 1 && pSkill->Point < 10){ //스킬 배움 
		lstrcpy(szTempSkillDoc,MasterSkillMoneyName);
		lstrcat(szSkillInfoBuff,szTempSkillDoc);
		//--------------------------------------------------------------------------//
		#ifdef HASIEGE_MODE
				//<ha>공성전 세율적용에 의해 수정됨
			wsprintf(szTempSkillDoc2,"%d\r",cShop.haShopItemPrice(sinMasterSkill_Money[pSkill->Skill_Info.SkillNum]+(sinMasterSkill_PlusMoney[pSkill->Skill_Info.SkillNum]*(pSkill->Point))));
		#else
			wsprintf(szTempSkillDoc2,"%d\r",sinMasterSkill_Money[pSkill->Skill_Info.SkillNum]+(sinMasterSkill_PlusMoney[pSkill->Skill_Info.SkillNum]*(pSkill->Point)));
		#endif
		//---------------------------------------------------------------------------//
		lstrcat(szSkillInfoBuff2,szTempSkillDoc2);
		LineCount++;
	}

	if(!ShowSkillUpInfo){ //정보를 크게 보여주기 일때는 복사하지 않는다 
		lstrcpy(szSkillMasterInfoBuff,szSkillInfoBuff);
		lstrcpy(szSkillMasterInfoBuff2,szSkillInfoBuff2);
	}
	SkillInfoLineCnt = LineCount;
	GetInfoBoxSize(pSkillBox,LineCount,Icon);

	return TRUE;

}

int cSKILL::GetInfoBoxSize(sSKILLBOX *pSkillBox, int Col,int Icon)
{

	int ty , tx;

	SkillBoxSize.x = 14;  //현제 가로 사이즈는 256으로 고정한다 (왜? 내맘이니까 음하하하)
	SkillBoxSize.y = Col+1; //글자 수만큼 사이즈를 잡는다 
	
	SkillInfoBoxPosi.x = (pSkillBox->BoxRect.left-112)+20;
	SkillInfoBoxPosi.y = pSkillBox->BoxRect.top - (SkillBoxSize.y*16); //아이템 정보박스의 시작 위치 

	if(Icon){
		SkillInfoBoxPosi.x = pSkillBox->BoxRect.left;
		SkillInfoBoxPosi.y = pSkillBox->BoxRect.top;
	
	}

	ty = SkillInfoBoxPosi.y + (SkillBoxSize.y*16);
	tx = SkillInfoBoxPosi.x + (SkillBoxSize.x*16);

	/////////////화면밖으로 나가지 않게 위치 보정 
	if(SkillInfoBoxPosi.y < 0)
		SkillInfoBoxPosi.y = 0;
	if(SkillInfoBoxPosi.x < 0)
		SkillInfoBoxPosi.x = 0;

	if(ty > 600)
		SkillInfoBoxPosi.y -= (ty - 600);
	if(tx > 800)
		SkillInfoBoxPosi.x -= (tx - 800);

	return TRUE;
}


//숙련도를 표시한다 
int cSKILL::GetSkillMastery(sSKILL *pSkill , sSKILLBOX *pSkillBox)
{
	int SkillMasteryTemp = 0;
	SkillMasteryTemp = ((int)pSkill->UseSkillMastery/100);
	wsprintf(szUseSkillMastery , "%d",SkillMasteryTemp);

	UseSkillMasteryPosi.x = pSkillBox->GageRect.left - (34/2);
	UseSkillMasteryPosi.y = pSkillBox->GageRect.top - 29;

	return TRUE;
}

//스킬을 사용할수있는지를 체크한다 // 장별-마나 리듀스 썼을 경우
int cSKILL::CheckUseSkillState(sSKILL *pSkill)
{
	if(pSkill->Skill_Info.UseMana)
	{
		// 장별 - 테리 속성
		// 장별 - 마나리듀스, 테리, 근력리듀스
		if( chaPremiumitem.m_ManaReducePotiontime > 0 && chaPremiumitem.m_TerryTime > 0 && sinChar->GravityScroolCheck[1] == 1 && chaPremiumitem.m_StaminaReducePotiontime > 0 )
		{
			if(sinGetMana() >= pSkill->Skill_Info.UseMana[pSkill->Point - 1] - ((pSkill->Skill_Info.UseMana[pSkill->Point - 1] * 
				chaPremiumitem.m_ManaReducePotionValue / 100) + (pSkill->Skill_Info.UseMana[pSkill->Point-1] * 20 / 100)) && 
				sinGetStamina() >= (pSkill->Skill_Info.UseStamina[0] + (pSkill->Skill_Info.UseStamina[1]*(pSkill->Point-1)))-( (pSkill->Skill_Info.UseStamina[0] + (pSkill->Skill_Info.UseStamina[1]*(pSkill->Point-1))) * chaPremiumitem.m_StaminaReducePotionValue / 100))
			{
				pSkill->UseSkillFlag = 1;
			}

			else
				pSkill->UseSkillFlag = 0;
		}

		// 장별 - 마나리듀스, 근력리듀스
		else if( chaPremiumitem.m_ManaReducePotiontime > 0 && chaPremiumitem.m_StaminaReducePotiontime > 0 )
		{
			if(sinGetMana() >= pSkill->Skill_Info.UseMana[pSkill->Point - 1] - (pSkill->Skill_Info.UseMana[pSkill->Point - 1] * chaPremiumitem.m_ManaReducePotionValue / 100) && 
				sinGetStamina() >= (pSkill->Skill_Info.UseStamina[0] + (pSkill->Skill_Info.UseStamina[1]*(pSkill->Point-1)))-( (pSkill->Skill_Info.UseStamina[0] + (pSkill->Skill_Info.UseStamina[1]*(pSkill->Point-1))) * chaPremiumitem.m_StaminaReducePotionValue / 100))
			{
				pSkill->UseSkillFlag = 1;
			}
			else
				pSkill->UseSkillFlag = 0;
		}

		// 장별 - 테리, 근력리듀스
		else if( chaPremiumitem.m_TerryTime > 0 && sinChar->GravityScroolCheck[1] == 1 && chaPremiumitem.m_StaminaReducePotiontime > 0 )
		{
			if(sinGetMana() >= pSkill->Skill_Info.UseMana[pSkill->Point - 1] - (pSkill->Skill_Info.UseMana[pSkill->Point - 1] * 20 / 100) && 
				sinGetStamina() >= (pSkill->Skill_Info.UseStamina[0] + (pSkill->Skill_Info.UseStamina[1]*(pSkill->Point-1)))-( (pSkill->Skill_Info.UseStamina[0] + (pSkill->Skill_Info.UseStamina[1]*(pSkill->Point-1))) * chaPremiumitem.m_StaminaReducePotionValue / 100))
			{
				pSkill->UseSkillFlag = 1;
			}
			else
				pSkill->UseSkillFlag = 0;
		}

		// 장별 - 마나리듀스
		else if( chaPremiumitem.m_ManaReducePotiontime > 0)
		{
			if(sinGetMana() >= pSkill->Skill_Info.UseMana[pSkill->Point - 1] - (pSkill->Skill_Info.UseMana[pSkill->Point - 1] * chaPremiumitem.m_ManaReducePotionValue / 100) && 
				sinGetStamina() >= pSkill->Skill_Info.UseStamina[0] + (pSkill->Skill_Info.UseStamina[1]*(pSkill->Point-1)))
			{
				pSkill->UseSkillFlag = 1;
			}
			else
				pSkill->UseSkillFlag = 0;
		}

		// 장별 - 테리
		else if( chaPremiumitem.m_TerryTime > 0 && sinChar->GravityScroolCheck[1] == 1)
		{
			if(sinGetMana() >= pSkill->Skill_Info.UseMana[pSkill->Point - 1] - (pSkill->Skill_Info.UseMana[pSkill->Point - 1] * 20 / 100) && 
				sinGetStamina() >= pSkill->Skill_Info.UseStamina[0] + (pSkill->Skill_Info.UseStamina[1]*(pSkill->Point-1)))
			{
				pSkill->UseSkillFlag = 1;
			}
			else
				pSkill->UseSkillFlag = 0;
		}

		// 장별 - 근력리듀스
		else if( chaPremiumitem.m_StaminaReducePotiontime > 0)
		{
			if(sinGetMana() >=  pSkill->Skill_Info.UseMana[pSkill->Point - 1] &&
				sinGetStamina() >= (pSkill->Skill_Info.UseStamina[0] + (pSkill->Skill_Info.UseStamina[1]*(pSkill->Point-1)))-( (pSkill->Skill_Info.UseStamina[0] + (pSkill->Skill_Info.UseStamina[1]*(pSkill->Point-1))) * chaPremiumitem.m_StaminaReducePotionValue / 100))
			{
				pSkill->UseSkillFlag = 1;

			}
			else
				pSkill->UseSkillFlag = 0;
		}

		// 아이템 사용 안했다
		else 
		{
			if(sinGetMana() >=  pSkill->Skill_Info.UseMana[pSkill->Point - 1] &&
				sinGetStamina() >= pSkill->Skill_Info.UseStamina[0] + (pSkill->Skill_Info.UseStamina[1]*(pSkill->Point-1)))
			{
				pSkill->UseSkillFlag = 1;

			}
			else
				pSkill->UseSkillFlag = 0;
		}
	}
	return TRUE;
}


int SkillDataCheckDelayTime = 0;
int SkillDataCheckFlag2 = 0;
//스킬 데이타를 확인한다 
int cSKILL::CheckSkillData()
{
	int i;
	int k;

	SkillDataCheckDelayTime++;
	if(SkillDataCheckDelayTime < 70*10)return FALSE;
	SkillDataCheckDelayTime = 0;

	
	unsigned int SkillDataCheckSum = 0;

	
	unsigned int SkillInfoDataCheckSum = 0;


	for( i = 0; i < 10 ; i++){
		k = i+1;
		SkillDataCheckSum += k*E_Shield_BlockRate[i];
		SkillDataCheckSum += k*E_Shield_UseTime[i];
		SkillDataCheckSum += k*E_Shield_UseMana[i];
		SkillDataCheckSum += k*Mechanic_Bomb_Damage[i][0];
		SkillDataCheckSum += k*Mechanic_Bomb_Damage[i][1];
		SkillDataCheckSum += k*Mechanic_Bomb_Attack_Range[i];
		SkillDataCheckSum += k*Mechanic_Bomb_UseMana[i];
		SkillDataCheckSum += k*P_Absorb[i][0];
		SkillDataCheckSum += k*P_Absorb[i][1];
		SkillDataCheckSum += k*P_Absorb_UseTime[i];
		SkillDataCheckSum += k*P_Absor_bUseMana[i];
		SkillDataCheckSum += k*PlusPoison[i];
		SkillDataCheckSum += (int)(k*Melee_Mastery_DamagePercent[i]);
		SkillDataCheckSum += k*PlusFire[i];
		SkillDataCheckSum += k*Raving_Damage[i];
		SkillDataCheckSum += k*Ravind_Speed[i];
		SkillDataCheckSum += (int)(k*Raving_UseLife[i]);
		SkillDataCheckSum += k*Raving_UseMana[i];
		SkillDataCheckSum += k*Impact_Attack_Rating[i];
		SkillDataCheckSum += k*Impact_Damage[i];
		SkillDataCheckSum += k*Impact_UseMana[i];
		SkillDataCheckSum += k*Pike_Wind_Throw_Height[i];
		SkillDataCheckSum += k*Pike_Wind_Damage[i][0];
		SkillDataCheckSum += k*Pike_Wind_Damage[i][1];
		SkillDataCheckSum += k*Pike_Wind_Push_Lenght[i];
		SkillDataCheckSum += k*Pike_Wind_UseMana[i];
		SkillDataCheckSum += k*PlusIce[i];
		SkillDataCheckSum += k*Critical_Hit_Critical[i];
		SkillDataCheckSum += k*Critical_Hit_UseMana[i];
		SkillDataCheckSum += k*Jumping_Crash_UseMana[i];
		SkillDataCheckSum += k*Jumping_Crash_Attack_Rating[i];
		SkillDataCheckSum += k*Jumping_Crash_Damage[i];
		SkillDataCheckSum += k*Scout_Hawk_Attack_Rate[i];
		SkillDataCheckSum += k*Scout_Hawk_UseTime[i];
		SkillDataCheckSum += k*Scout_Hawk_UseMana[i];
		SkillDataCheckSum += k*S_Mastery_DamagePercent[i];
		SkillDataCheckSum += k*Wind_Arrow_Speed[i];
		SkillDataCheckSum += k*Wind_Arrow_Damage[i];
		SkillDataCheckSum += k*Wind_Arrow_UseMana[i];
		SkillDataCheckSum += k*Perfect_Aim_Attack_Rate[i];
		SkillDataCheckSum += k*Perfect_Aim_Damage[i];
		SkillDataCheckSum += k*Perfect_Aim_Damage_LV[i];
		SkillDataCheckSum += k*Perfect_Aim_UseMana[i];
		SkillDataCheckSum += k*G_Smash_Attack_Rate[i];
		SkillDataCheckSum += k*G_Smash_Attack_Rage[i];
		SkillDataCheckSum += k*G_Smash_UseMana[i];
		SkillDataCheckSum += k*Maximize_Damage[i];
		SkillDataCheckSum += k*Maximize_Time[i];
		SkillDataCheckSum += k*Maximize_UseMana[i];
		SkillDataCheckSum += k*Automation_Damage[i];
		SkillDataCheckSum += k*Automation_Speed[i];
		SkillDataCheckSum += k*Automation_Time[i];
		SkillDataCheckSum += k*Automation_UseMana[i];
		SkillDataCheckSum += k*Spark_Damage[i][0];
		SkillDataCheckSum += k*Spark_Damage[i][1];
		SkillDataCheckSum += k*Spark_Area[i];
		SkillDataCheckSum += k*Spark_UseMana[i];
		SkillDataCheckSum += k*T_Impact_Damage[i];
		SkillDataCheckSum += k*T_Impact_Hit[i];
		SkillDataCheckSum += k*T_Impact_UseMana[i];
		SkillDataCheckSum += k*B_Swing_Damage[i];
		SkillDataCheckSum += k*B_Swing_Critical[i];
		SkillDataCheckSum += k*B_Swing_UseMana[i];
		SkillDataCheckSum += k*Roar_Range[i];
		SkillDataCheckSum += k*Roar_Time[i];
		SkillDataCheckSum += k*Roar_UseMana[i];
		SkillDataCheckSum += k*R_Zecram_Damage[i];
		SkillDataCheckSum += k*R_Zecram_UseMana[i];
		SkillDataCheckSum += k*G_Pike_Damage[i][0];
		SkillDataCheckSum += k*G_Pike_Damage[i][1];
		SkillDataCheckSum += k*G_Pike_Range[i];
		SkillDataCheckSum += k*G_Pike_UseMana[i];
		SkillDataCheckSum += k*G_Pike_Time[i];
		SkillDataCheckSum += k*Tornado_Damage[i];
		SkillDataCheckSum += k*Tornado_Stun[i];
		SkillDataCheckSum += k*Tornado_Range[i];
		SkillDataCheckSum += k*Tornado_UseMana[i];
		SkillDataCheckSum += k*W_D_Mastery_Block[i];
		SkillDataCheckSum += k*Expansion_Size[i];
		SkillDataCheckSum += k*Expansion_UseMana[i];
		SkillDataCheckSum += k*D_Eye_Attack_Rate[i];
		SkillDataCheckSum += k*Falcon_Damage[i][0];
		SkillDataCheckSum += k*Falcon_Damage[i][1];
		SkillDataCheckSum += k*Falcon_Time[i];
		SkillDataCheckSum += k*Falcon_UseMana[i];
		SkillDataCheckSum += k*A_Rage_ArrowNum[i];
		SkillDataCheckSum += k*A_Rage_Damage[i];
		SkillDataCheckSum += k*A_Rage_Area[i];
		SkillDataCheckSum += k*A_Rage_UseMana[i];
		SkillDataCheckSum += k*Avalanche_Speed[i];
		SkillDataCheckSum += k*Avalanche_Damage[i];
		SkillDataCheckSum += k*Avalanche_ArrowNum[i];
		SkillDataCheckSum += k*Avalanche_UseMana[i];
		SkillDataCheckSum += k*Expansion_Damage[i]; 
	}								//<========== 여기
	for( i = 0; i < 10 ; i++){					//<========== 여기
		k = i+1;						//<========== 여기
		SkillDataCheckSum += k*Sword_Blast_UseMana[i]; 
		SkillDataCheckSum += k*Sword_Blast_Damage[i][0]; 
		SkillDataCheckSum += k*Sword_Blast_Damage[i][1]; 
		SkillDataCheckSum += k*Sword_Blast_ShootingRange[i]; 
		SkillDataCheckSum += k*HolyBody_UseMana[i]; 
		SkillDataCheckSum += k*HolyBody_Absorb[i]; 
		SkillDataCheckSum += k*HolyBody_Time[i]; 
		SkillDataCheckSum += k*P_Traning_Stamina[i]; 
		SkillDataCheckSum += k*D_Crash_UseMana[i]; 
		SkillDataCheckSum += k*D_Crash_Damage[i]; 
		SkillDataCheckSum += k*D_Crash_Critical[i]; 
		SkillDataCheckSum += k*S_Strike_UseMana[i]; 
		SkillDataCheckSum += k*S_Strike_Damage[i][0]; 
		SkillDataCheckSum += k*S_Strike_Damage[i][1]; 
		SkillDataCheckSum += k*Farina_UseMana[i]; 
		SkillDataCheckSum += k*Farina_Speed[i]; 
		SkillDataCheckSum += k*Farina_AttackRate[i]; 
		SkillDataCheckSum += k*T_Mastery_Damage[i]; 
		SkillDataCheckSum += k*V_Spear_UseMana[i]; 
		SkillDataCheckSum += k*V_Spear_Damage[i][0]; 
		SkillDataCheckSum += k*V_Spear_Damage[i][1]; 
		SkillDataCheckSum += k*Healing_UseMana[i]; 
		SkillDataCheckSum += k*Healing_Heal[i][0]; 
		SkillDataCheckSum += k*Healing_Heal[i][1]; 
		SkillDataCheckSum += k*HolyBolt_UseMana[i]; 
		SkillDataCheckSum += k*HolyBolt_Damage[i][0]; 
		SkillDataCheckSum += k*HolyBolt_Damage[i][1]; 
		SkillDataCheckSum += k*M_Spark_UseMana[i]; 
		SkillDataCheckSum += k*M_Spark_Damage[i][0]; 
		SkillDataCheckSum += k*M_Spark_Damage[i][1]; 
		SkillDataCheckSum += k*M_Spark_Num[i]; 
		SkillDataCheckSum += k*HolyMind_UseMana[i]; 
		SkillDataCheckSum += k*HolyMind_DecDamage[i]; 
		SkillDataCheckSum += k*HolyMind_Time[i]; 
		SkillDataCheckSum += k*Agony_UseMana[i]; 
		SkillDataCheckSum += k*Agony_ConvLife[i]; 
		SkillDataCheckSum += k*FireBolt_UseMana[i]; 
		SkillDataCheckSum += k*FireBolt_Damage[i][0]; 
		SkillDataCheckSum += k*FireBolt_Damage[i][1]; 
		SkillDataCheckSum += k*Zenith_UseMana[i]; 
		SkillDataCheckSum += k*Zenith_Element[i]; 
		SkillDataCheckSum += k*Zenith_Time[i]; 
		SkillDataCheckSum += k*FireBall_UseMana[i]; 
		SkillDataCheckSum += k*FireBall_Damage[i][0]; 
		SkillDataCheckSum += k*FireBall_Damage[i][1]; 
		SkillDataCheckSum += k*FireBall_Range[i]; 
		SkillDataCheckSum += k*Holy_Valor_Damage[i]; 
		SkillDataCheckSum += k*Holy_Valor_Time[i]; 
		SkillDataCheckSum += k*Holy_Valor_Mana[i]; 
		SkillDataCheckSum += k*Brandish_Range[i]; 
		SkillDataCheckSum += k*Brandish_Damage[i]; 
		SkillDataCheckSum += k*Brandish_Mana[i]; 
		SkillDataCheckSum += k*Piercing_Damage[i]; 
		SkillDataCheckSum += k*Piercing_Attack_Rating[i]; 
		SkillDataCheckSum += k*Piercing_Mana[i]; 
		SkillDataCheckSum += k*Drastic_Spirit_Defense[i]; 
		SkillDataCheckSum += k*Drastic_Spirit_Time[i]; 
		SkillDataCheckSum += k*Drastic_Spirit_Mana[i]; 
		SkillDataCheckSum += k*Windy_Attack_Rating[i]; 
		SkillDataCheckSum += k*Windy_Time[i]; 
		SkillDataCheckSum += k*Windy_Mana[i]; 
		SkillDataCheckSum += k*Twist_Javelin_Damage[i]; 
		SkillDataCheckSum += k*Twist_Javelin_Attack_Rating[i]; 
		SkillDataCheckSum += k*Twist_Javelin_Mana[i]; 
		SkillDataCheckSum += k*Soul_Sucker_Absorb[i]; 
		SkillDataCheckSum += k*Soul_Sucker_Mana[i]; 
		SkillDataCheckSum += k*Fire_Javelin_Damage[i][0]; 
		SkillDataCheckSum += k*Fire_Javelin_Damage[i][1]; 
		SkillDataCheckSum += k*Fire_Javelin_Mana[i]; 
		SkillDataCheckSum += k*(int)Meditation_Regen[i]; 
		SkillDataCheckSum += k*Divine_Lightning_Damage[i][0]; 
		SkillDataCheckSum += k*Divine_Lightning_Damage[i][1]; 
		SkillDataCheckSum += k*Divine_Lightning_Num[i]; 
		SkillDataCheckSum += k*Divine_Lightning_Mana[i]; 
		SkillDataCheckSum += k*Holy_Reflection_Return_Damage[i]; 
		SkillDataCheckSum += k*Holy_Reflection_Time[i]; 
		SkillDataCheckSum += k*Holy_Reflection_Mana[i]; 
		SkillDataCheckSum += k*Grand_Healing[i][0]; 
		SkillDataCheckSum += k*Grand_Healing[i][1]; 
		SkillDataCheckSum += k*Grand_Healing_Range[i]; 
		SkillDataCheckSum += k*Grand_Healing_Mana[i]; 
		SkillDataCheckSum += k*Mental_Mastery_Mana[i]; 
		SkillDataCheckSum += k*Watornado_Damage[i][0]; 
		SkillDataCheckSum += k*Watornado_Damage[i][1]; 
		SkillDataCheckSum += k*Watornado_Area[i]; 
		SkillDataCheckSum += k*Watornado_Range[i]; 
		SkillDataCheckSum += k*Watornado_Mana[i]; 
		SkillDataCheckSum += k*Enchant_Weapon_Damage_Ice[i][0]; 
		SkillDataCheckSum += k*Enchant_Weapon_Damage_Ice[i][1]; 
		SkillDataCheckSum += k*Enchant_Weapon_Damage_Fire[i][0]; 
		SkillDataCheckSum += k*Enchant_Weapon_Damage_Fire[i][1]; 
		SkillDataCheckSum += k*Enchant_Weapon_Damage_Lightning[i][0]; 
		SkillDataCheckSum += k*Enchant_Weapon_Damage_Lightning[i][1]; 
		SkillDataCheckSum += k*Enchant_Weapon_Time[i]; 
		SkillDataCheckSum += k*Enchant_Weapon_Mana[i]; 
		SkillDataCheckSum += k*Dead_Ray_Damage[i][0]; 
		SkillDataCheckSum += k*Dead_Ray_Damage[i][1]; 
		SkillDataCheckSum += k*Dead_Ray_Range[i]; 
		SkillDataCheckSum += k*Dead_Ray_Mana[i]; 

	}

	for( i = 0; i < 10 ; i++){			//<========== 여기
		k = i+1;						//<========== 여기
		//메탈 아머
		SkillDataCheckSum += k*Metal_Armor_Defense[i];
		SkillDataCheckSum += k*Metal_Armor_Time[i];
		SkillDataCheckSum += k*Metal_Armor_UseMana[i];

		//그랜드 스메쉬
		SkillDataCheckSum += k*Grand_Smash_AttackRate[i];
		SkillDataCheckSum += k*Grand_Smash_Damage[i];
		SkillDataCheckSum += k*Grand_Smash_UseMana[i];

		//메카닉 웨폰 마스터리
		SkillDataCheckSum += k*M_Weapon_Mastey[i];

		//스파크 쉴드 
		SkillDataCheckSum += k*Spark_Shield_Damage[i];
		SkillDataCheckSum += k*Spark_Shield_Defense[i];
		SkillDataCheckSum += k*Spark_Shield_Time[i];
		SkillDataCheckSum += k*Spark_Shield_UseMana[i];
				

		SkillDataCheckSum += k*Concentration_AttackRate[i];
		SkillDataCheckSum += k*Concentration_Time[i];
		SkillDataCheckSum += k*Concentration_UseMana[i];

		//어벤징 크래쉬
		SkillDataCheckSum += k*A_Crash_Damage[i];
		SkillDataCheckSum += k*A_Crash_AttackRate[i];
		SkillDataCheckSum += k*A_Crash_UseMana[i];

		//스위프트 엑스
		SkillDataCheckSum += k*Swift_Axe_Speed[i];
		SkillDataCheckSum += k*Swift_Axe_Time[i];
		SkillDataCheckSum += k*Swift_Axe_UseMana[i];

		//본 크래쉬
		SkillDataCheckSum += k*B_Crash_Damage[i];
		SkillDataCheckSum += k*B_Crash_DemonDamage[i];
		SkillDataCheckSum += k*B_Crash_UseMana[i];

		SkillDataCheckSum += k*VenomSpear_Damage[i][0];
		SkillDataCheckSum += k*VenomSpear_Damage[i][1];
		SkillDataCheckSum += k*VenomSpear_Num[i];
		SkillDataCheckSum += k*VenomSpear_Time[i];
		SkillDataCheckSum += k*VenomSpear_Range[i];
		SkillDataCheckSum += k*VenomSpear_UseMana[i];

		//배니쉬
		SkillDataCheckSum += k*(int)Vanish_Speed[i];
		SkillDataCheckSum += k*Vanish_Damage[i];
		SkillDataCheckSum += k*Vanish_Range[i];
		SkillDataCheckSum += k*Vanish_Time[i];
		SkillDataCheckSum += k*Vanish_UseMana[i];

		//크리티컬 마스터리
		SkillDataCheckSum += k*Critical_Mastery_Critical[i];

		//체인 랜스
		SkillDataCheckSum += k*Chain_Lance_Damage[i];
		SkillDataCheckSum += k*Chain_Lance_UseMana[i];

		SkillDataCheckSum += k*Elemental_Shot_Ice[i][0];
		SkillDataCheckSum += k*Elemental_Shot_Ice[i][1];
		SkillDataCheckSum += k*Elemental_Shot_Fire[i][0];
		SkillDataCheckSum += k*Elemental_Shot_Fire[i][1];
		SkillDataCheckSum += k*Elemental_Shot_Lightning[i][0];
		SkillDataCheckSum += k*Elemental_Shot_Lightning[i][1];
		SkillDataCheckSum += k*Elemental_Shot_UseMana[i];

		//골든 팰컨
		SkillDataCheckSum += k*Golden_Falcon_Damage[i];
		SkillDataCheckSum += k*(int)Golden_Falcon_LifeRegen[i];
		SkillDataCheckSum += k*Golden_Falcon_Time[i];
		SkillDataCheckSum += k*Golden_Falcon_UseMana[i];

		//범 샷
		SkillDataCheckSum += k*BombShot_Damage[i];
		SkillDataCheckSum += k*BombShot_DemonDamage[i];
		SkillDataCheckSum += k*BombShot_AddDamage[i][0];
		SkillDataCheckSum += k*BombShot_AddDamage[i][1];
		SkillDataCheckSum += k*BombShot_Area[i];
		SkillDataCheckSum += k*BombShot_UseMana[i];

		//퍼포레이션
		SkillDataCheckSum += k*Perforation_Damage[i];
		SkillDataCheckSum += k*Perforation_Critical[i];
		SkillDataCheckSum += k*Perforation_UseMana[i];
		SkillDataCheckSum += k*Perforation_Attack_Range[i];
	}

	for( i = 0; i < 10 ; i++){			//<========== 여기
		k = i+1;						//<========== 여기

		SkillDataCheckSum += k*Sword_Mastery_DamagePercent[i];

		//디바인 인홀레이션 (디바인 쉴드로 이름만 바뀜)
		SkillDataCheckSum += k*D_Inhalation_Life[i];
		SkillDataCheckSum += k*D_Inhalation_Block[i];
		SkillDataCheckSum += k*D_Inhalation_Time[i];
		SkillDataCheckSum += k*D_Inhalation_UseMana[i];

		//홀리 인켄테이션
		SkillDataCheckSum += k*H_Incantation_Success[i];
		SkillDataCheckSum += k*H_Incantation_AddLife[i];
		SkillDataCheckSum += k*H_Incantation_Time[i];
		SkillDataCheckSum += k*H_Incantation_UseMana[i];

		//그랜드 크로스
		SkillDataCheckSum += k*Grand_Cross_Damage[i];
		SkillDataCheckSum += k*Grand_Cross_AttackRate[i];
		SkillDataCheckSum += k*Grand_Cross_UndeadDamage[i];
		SkillDataCheckSum += k*Grand_Cross_UseMana[i];

		SkillDataCheckSum += k*Split_Javelin_AttackNum[i];
		SkillDataCheckSum += k*Split_Javelin_Damage[i];
		SkillDataCheckSum += k*Split_Javelin_AttackRate[i];
		SkillDataCheckSum += k*Split_Javelin_UseMana[i];

		//트라이엄프 오브 발할라
		SkillDataCheckSum += k*T_Of_Valhalla_Damage[i];
		SkillDataCheckSum += k*T_Of_Valhalla_Time[i];
		SkillDataCheckSum += k*T_Of_Valhalla_UseMana[i];

		//라이트닝 재블린
		SkillDataCheckSum += k*Lightning_Javelin_Damage[i][0];
		SkillDataCheckSum += k*Lightning_Javelin_Damage[i][1];
		SkillDataCheckSum += k*Lightning_Javelin_UseMana[i];

		//스톰 재블린
		SkillDataCheckSum += k*Storm_Javelin_Damage[i];
		SkillDataCheckSum += k*Storm_Javelin_UseMana[i];

		SkillDataCheckSum += k*Vigor_Ball_Damage[i][0];
		SkillDataCheckSum += k*Vigor_Ball_Damage[i][1];
		SkillDataCheckSum += k*Vigor_Ball_UseMana[i];

		//리져렉션
		SkillDataCheckSum += k*Resurrection_Percent[i];
		SkillDataCheckSum += k*Resurrection_Exp[i];
		SkillDataCheckSum += k*Resurrection_UseMana[i];

		//익스팅션
		SkillDataCheckSum += k*Extinction_Percent[i];
		SkillDataCheckSum += k*Extinction_Amount[i];
		SkillDataCheckSum += k*Extinction_UseMana[i];

		//버추얼 라이프
		SkillDataCheckSum += k*Virtual_Life_Percent[i];
		SkillDataCheckSum += k*Virtual_Life_Time[i];
		SkillDataCheckSum += k*Virtual_Life_UseMana[i];

		SkillDataCheckSum += k*Energy_Shield_DecDamage[i];
		SkillDataCheckSum += k*Energy_Shield_Time[i];
		SkillDataCheckSum += k*Energy_Shield_UseMana[i];

		//디아스트로피즘
		SkillDataCheckSum += k*Diastrophism_Damage[i][0];
		SkillDataCheckSum += k*Diastrophism_Damage[i][1];
		SkillDataCheckSum += k*Diastrophism_Area[i];
		SkillDataCheckSum += k*Diastrophism_UseMana[i];

		//스피릿 엘레멘탈
		SkillDataCheckSum += k*(int)Spirit_Elemental_RegenMana[i];
		SkillDataCheckSum += k*Spirit_Elemental_Damage[i];
		SkillDataCheckSum += k*Spirit_Elemental_Time[i];
		SkillDataCheckSum += k*Spirit_Elemental_UseMana[i];

		//댄싱 소드
		SkillDataCheckSum += k*Dancing_Sword_IceDamage[i][0];
		SkillDataCheckSum += k*Dancing_Sword_IceDamage[i][1];
		SkillDataCheckSum += k*Dancing_Sword_FireDamage[i][0];
		SkillDataCheckSum += k*Dancing_Sword_FireDamage[i][1];
		SkillDataCheckSum += k*Dancing_Sword_AttackDelay[i];
		SkillDataCheckSum += k*Dancing_Sword_Time[i];
		SkillDataCheckSum += k*Dancing_Sword_UseMana[i];


	}
	/*************************************************************************/
	/*                            4차 스킬                                   */
    /*************************************************************************/
	//**4차스킬(템스크론)
	for( i = 0; i < 10 ; i++){			//<========== 여기
		k = i+1;
		//임플리션
		SkillDataCheckSum +=k*Impulsion_LightingDamage[i][0];
		SkillDataCheckSum +=k*Impulsion_LightingDamage[i][1];
		SkillDataCheckSum +=k*Impulsion_Range[i];
		SkillDataCheckSum +=k*Impulsion_LightingNum[i];
		SkillDataCheckSum +=k*Impulsion_UseMana[i];
		//컴플리션
		SkillDataCheckSum +=k*Compulsion_AddAbsorb[i];
		SkillDataCheckSum +=k*Compulsion_Area[i];
		SkillDataCheckSum +=k*Compulsion_Time[i];
	    SkillDataCheckSum +=k*Compulsion_UseMana[i];
		//메네틱 피어
        SkillDataCheckSum +=k*Magnetic_Sphere_AddDamage[i][0];
        SkillDataCheckSum +=k*Magnetic_Sphere_AddDamage[i][1];
		SkillDataCheckSum +=k*Magnetic_Sphere_Area[i];
        SkillDataCheckSum +=k*Magnetic_Sphere_AttackDelay[i];
        SkillDataCheckSum +=k*Magnetic_Sphere_Time[i];
        SkillDataCheckSum +=k*Magnetic_Sphere_UseMana[i];
		//메탈 골렘
        SkillDataCheckSum +=k*Metal_Golem_Damage[i][0];
        SkillDataCheckSum +=k*Metal_Golem_Damage[i][1];
        SkillDataCheckSum +=k*Metal_Golem_Hit[i];
        SkillDataCheckSum +=k*Metal_Golem_Defense[i];
        SkillDataCheckSum +=k*Metal_Golem_Life[i];
        SkillDataCheckSum +=k*Metal_Golem_UseMana[i];
		//디스토리어
		SkillDataCheckSum +=k*Destoryer_DamagePercent[i];
		SkillDataCheckSum +=k*Destoryer_AddCritical[i];
		SkillDataCheckSum +=k*Destoryer_UseMana[i];
		//버서커
		SkillDataCheckSum +=k*Berserker_AddAttack[i];
		SkillDataCheckSum +=k*Berserker_SubAbsorb[i];
		SkillDataCheckSum +=k*Berserker_Time[i];
		SkillDataCheckSum +=k*Berserker_UseMana[i];
		//싸이클론 스트라이크
		SkillDataCheckSum +=k*Cyclone_Strike_DamagePercent[i];
		SkillDataCheckSum +=k*Cyclone_Strike_AreaDamage[i][0];
		SkillDataCheckSum +=k*Cyclone_Strike_AreaDamage[i][1];
		SkillDataCheckSum +=k*Cyclone_Strike_AttackNum[i];
		SkillDataCheckSum +=k*Cyclone_Strike_Area[i];
		SkillDataCheckSum +=k*Cyclone_Strike_UseMana[i];
		//붓트 헬스
		SkillDataCheckSum +=k*Boost_Health_Life[i];
		//어쌔신 아이
		SkillDataCheckSum +=k*Assassin_Eye_AddCritical[i];
		SkillDataCheckSum +=k*Assassin_Eye_M_SubCritical[i];
		SkillDataCheckSum +=k*Assassin_Eye_Time[i];
		SkillDataCheckSum +=k*Assassin_Eye_UseMana[i];
		//차징 스트라이크
		SkillDataCheckSum +=k*Charging_Strike_DamagePercent[i];
		SkillDataCheckSum +=k*Charging_Strike_CharingDamagePercent[i];
		SkillDataCheckSum +=k*(int)Charging_Strike_Time[i];
		SkillDataCheckSum +=k*Charging_Strike_UseMana[i];
		//베이그
		SkillDataCheckSum +=k*Vague_EvasionPercent[i];
		SkillDataCheckSum +=k*Vague_Time[i];
		SkillDataCheckSum +=k*Vague_UseMana[i];
		//새됴우 마스터
		SkillDataCheckSum +=k*Shadow_Master_DamagePercent[i];
		SkillDataCheckSum +=k*Shadow_Master_AddHit[i];
		SkillDataCheckSum +=k*Shadow_Master_ShadowNum[i];
		SkillDataCheckSum +=k*Shadow_Master_UseMana[i];
		//리콜 울버린
		SkillDataCheckSum +=k*Recall_Wolverin_Damage[i][0];
		SkillDataCheckSum +=k*Recall_Wolverin_Damage[i][1];
		SkillDataCheckSum +=k*Recall_Wolverin_Hit[i];
		SkillDataCheckSum +=k*Recall_Wolverin_Defense[i];
		SkillDataCheckSum +=k*Recall_Wolverin_Life[i];
		SkillDataCheckSum +=k*Recall_Wolverin_UseMana[i];
		//이베이션 마스터리
		SkillDataCheckSum +=k*Evasion_Mastery_AddPercent[i];
		//피닉스 샷
		SkillDataCheckSum +=k*Phoenix_Shot_DamagePercent[i];
		SkillDataCheckSum +=k*(int)Phoenix_Shot_Time[i];
		SkillDataCheckSum +=k*Phoenix_Shot_UseMana[i];
		//포스 오브 내츄럴
		SkillDataCheckSum +=k*Force_Of_Nature_AddDamage[i];
		SkillDataCheckSum +=k*Force_Of_Nature_AddHit[i];
		SkillDataCheckSum +=k*Force_Of_Nature_FalconAddDamage[i];
		SkillDataCheckSum +=k*Force_Of_Nature_Time[i];
		SkillDataCheckSum +=k*Force_Of_Nature_UseMana[i];
	}
	//**4차 스킬(모라이온)
	for( i = 0; i < 10 ; i++){			//<========== 여기
		k = i+1;	
		
		//디바인 피어싱
		SkillDataCheckSum +=k*Divine_Piercing_DamagePercent[i];
		SkillDataCheckSum +=k*Divine_Piercing_AddHit[i];
		SkillDataCheckSum +=k*Divine_Piercing_AttackNum[i];
		SkillDataCheckSum +=k*Divine_Piercing_UseMana[i];
		//갓들리 쉴드
		SkillDataCheckSum +=k*Godly_Shield_AbsorbPercent[i];
		SkillDataCheckSum +=k*Godly_Shield_Time[i];
		SkillDataCheckSum +=k*Godly_Shield_UseMana[i];
		//갓 블레스
		SkillDataCheckSum +=k*God_Bless_AddDamage[i];
		SkillDataCheckSum +=k*God_Bless_Time[i];
		SkillDataCheckSum +=k*God_Bless_UseMana[i];
		//스워드 오브 저스틱스
		SkillDataCheckSum +=k*Sword_Of_Justice_DamagePercent[i];
		SkillDataCheckSum +=k*Sword_Of_Justice_Area[i];
		SkillDataCheckSum +=k*Sword_Of_Justice_UseMana[i];
		//헬오브 발할라
		SkillDataCheckSum +=k*Hall_Of_Valhalla_AddEvation[i];
		SkillDataCheckSum +=k*Hall_Of_Valhalla_Time[i];
		SkillDataCheckSum +=k*Hall_Of_Valhalla_UseMana[i];
		//엑스 라지
		SkillDataCheckSum +=k*X_Rage_DamagePercent[i];
		SkillDataCheckSum +=k*X_Rage_Area[i];
		SkillDataCheckSum +=k*X_Rage_UseMana[i];
		//프로스트 재블린
		SkillDataCheckSum +=k*Frost_Javelin_IceAddDamage[i][0];
		SkillDataCheckSum +=k*Frost_Javelin_IceAddDamage[i][1];
		SkillDataCheckSum +=k*Frost_Javelin_SpeedSubPercent[i];
		SkillDataCheckSum +=k*Frost_Javelin_IceTime[i];
		SkillDataCheckSum +=k*Frost_Javelin_Time[i];
		SkillDataCheckSum +=k*Frost_Javelin_UseMana[i];
		//벤첸스
		SkillDataCheckSum +=k*Vengeance_DamagePercent[i];
		SkillDataCheckSum +=k*Vengeance_AddCritical[i]; // 장별 - 스킬밸런스 조정(10.08.10)
		SkillDataCheckSum +=k*Vengeance_Range[i];
		SkillDataCheckSum +=k*Vengeance_UseMana[i];
		//글래이셔 스트라이크
		SkillDataCheckSum +=k*Glacial_Spike_Damage[i][0];
		SkillDataCheckSum +=k*Glacial_Spike_Damage[i][1];
		SkillDataCheckSum +=k*Glacial_Spike_Area[i];
		SkillDataCheckSum +=k*Glacial_Spike_UseMana[i];
		//리제너레이션 필드
		SkillDataCheckSum +=k*(int)Regeneration_Field_LifeRegen[i];
		SkillDataCheckSum +=k*(int)Regeneration_Field_ManaRegen[i];
		SkillDataCheckSum +=k*Regeneration_Field_Time[i];
		SkillDataCheckSum +=k*Regeneration_Field_Area[i];
		SkillDataCheckSum +=k*Regeneration_Field_UseMana[i];
		//체인라이트닝
		SkillDataCheckSum +=k*Chain_Lightning_Damage[i][0];
		SkillDataCheckSum +=k*Chain_Lightning_Damage[i][1];
		SkillDataCheckSum +=k*Chain_Lightning_Num[i];
		SkillDataCheckSum +=k*Chain_Lightning_Range[i];
		SkillDataCheckSum +=k*Chain_Lightning_UseMana[i];
		//서몬 뮤스펠
		SkillDataCheckSum +=k*Summon_Muspell_Damage[i][0];
		SkillDataCheckSum +=k*Summon_Muspell_Damage[i][1];
		SkillDataCheckSum +=k*Summon_Muspell_Attack_Delay[i];
		SkillDataCheckSum +=k*Summon_Muspell_UndeadAbsorbPercent[i];
		SkillDataCheckSum +=k*Summon_Muspell_BlockPercent[i];
		SkillDataCheckSum +=k*Summon_Muspell_Time[i];
		SkillDataCheckSum +=k*Summon_Muspell_UseMana[i];
		//파이어 엘레멘탈
		SkillDataCheckSum +=k*Fire_Elemental_Damage[i][0];
		SkillDataCheckSum +=k*Fire_Elemental_Damage[i][1];
		SkillDataCheckSum +=k*Fire_Elemental_Hit[i];
		SkillDataCheckSum +=k*Fire_Elemental_Life[i];
		SkillDataCheckSum +=k*Fire_Elemental_UseMana[i];
		//플레임 웨이브
		SkillDataCheckSum +=k*Flame_Wave_FireDamage[i][0];
		SkillDataCheckSum +=k*Flame_Wave_FireDamage[i][1];
		SkillDataCheckSum +=k*Flame_Wave_Area[i];
		SkillDataCheckSum +=k*Flame_Wave_UseMana[i];
		//디스트록션
		SkillDataCheckSum +=k*Distortion_SpeedSubPercent[i];
		SkillDataCheckSum +=k*Distortion_DamageSubPercent[i];
		SkillDataCheckSum +=k*Distortion_Area[i];
		SkillDataCheckSum +=k*Distortion_Time[i];
		SkillDataCheckSum +=k*Distortion_UseMana[i];
		//메테오
		SkillDataCheckSum +=k*Meteo_Damage[i][0];
		SkillDataCheckSum +=k*Meteo_Damage[i][1];
		SkillDataCheckSum +=k*Meteo_Area[i];
		SkillDataCheckSum +=k*Meteo_UseMana[i];

	}//<========== 여기

	// 장별 - 스킬밸런스 조정(10.08.10)
	unsigned int sinSKillInfoData = 612842  ;//현재 값이 어케나올지는 모른다 // sinSkill_Info.h 
	unsigned int sinSKillData     = 2614433 ;   //구버전
	

	if(	sinSKillData != SkillDataCheckSum){        
		SendSetHackUser(5); //해킹을 하려고했던 못된 유저를 고발 TRUE 접속 종료 
	}

	//스킬정보 테이블의 조작을 방지한다 
	for(i=0;i< SIN_MAX_SKILL;i++){
		if ( (sSkill_Info[i].CODE&sinSKILL_MASK1)!=GROUP_OTHERSKILL && (sSkill_Info[i].CODE&CHANGE_JOB5) ) continue;	// the tier 5 rows came later: the sum stays the original's
		k = i+1;
		SkillInfoDataCheckSum += k*sSkill_Info[i].RequireLevel;
		SkillInfoDataCheckSum += k*sSkill_Info[i].UseStamina[0];
		SkillInfoDataCheckSum += k*sSkill_Info[i].UseStamina[1];
	}

	//SkillInfoDataCheckSum += sinSkill.UseSkill[j]
	if(	sinSKillInfoData != SkillInfoDataCheckSum){
		SendSetHackUser(5); //해킹을 하려고했던 못된 유저를 고발 TRUE 접속 종료 

	}
	//스킬포인트를 체크한다
	for(int j=1;j < SIN_MAX_USE_SKILL; j++){
		if(sinSkill.UseSkill[j].Flag){
			//한번만 리폼해준다
			if(!SkillDataCheckFlag2){
				ReformSkillPointForm(&sinSkill.UseSkill[j]);
			}
			if(!CheckSkillPointForm(&sinSkill.UseSkill[j]))return FALSE;
			if(!CheckSkillMasteryForm(&sinSkill.UseSkill[j]))return FALSE;

		}
	}
	SkillDataCheckFlag2 = 1; //두번째부터는 리폼을 해주지않는다


	return TRUE;     
}


//스킬포인트 초기화하기 
int cSKILL::InitSkillPoint(int kind)
{
    //kind 0 이벤트 초기화kind 1레드스톤 초기화
	switch(kind){
		case 0:
			if(!sinInitPointPassFlag){ 
				if(sinChar->wVersion[1] == 1){
					cMessageBox.ShowMessage(MESSAGE_ALREADY_INITSTATE);
					return FALSE;
				}
			}				
		break;
	}
	sinInitPointPassFlag = 0;

	CheckingNowSkillState(); //스킬포인트를 
	CheckCharForm();//인증 
	int Count  = 0;
	int Count2 = 0;

	for(int j = 1 ; j < ChangeJobSkillPlus ; j ++){
		if(j >12){
			if(sinSkill.UseSkill[j].Point > 0)
				Count2 += sinSkill.UseSkill[j].Point;
		}
		else{
			if(sinSkill.UseSkill[j].Point > 0)
				Count += sinSkill.UseSkill[j].Point;
		}

		cSkill.CheckSkillMasteryForm(0,j); // 스킬 마스터리를 체크한다
		CheckSkillPointForm(&sinSkill.UseSkill[j]);
		sinSkill.UseSkill[j].Point = 0; //스킬포인트 초기화 
		sinSkill.UseSkill[j].GageLength   = 0;
		sinSkill.UseSkill[j].GageLength2  = 0;
		sinSkill.UseSkill[j].Mastery      = 0;
		sinSkill.UseSkill[j].UseSKillIncreCount = 0;
		sinSkill.UseSkill[j].UseSkillCount = 0;
		sinSkill.UseSkill[j].UseSkillMastery = 0;
		sinSkill.UseSkill[j].UseSkillMasteryGage = 0;
		sinSkill.UseSkill[j].Use = 0;
		ReformSkillPointForm(&sinSkill.UseSkill[j]);
		ReformSkillMasteryForm(0,j);

	}
	sinSkill.SkillPoint  += Count;
	sinSkill.SkillPoint4 += Count2;
	sinChar->wVersion[1] = 1;
	ReformCharForm();//재인증 
	cMessageBox.ShowMessage(MESSAGE_GYUNGFUM_OK);
	cInvenTory.SetItemToChar();

	return TRUE;
}

//DWORD ClearSkillPointCODE[10] = {SKILL_HOLY_BODY , SKILL_HOLY_MIND , 0,0,0,0,0,0,0,0};
DWORD ClearSkillPointCODE[10] = {0,0,0,0,0,0,0,0,0,0}; //부분 스킬 초기화 

int cSKILL::SearchClearSkillPoint(DWORD CODE)
{
	
	for(int i = 0 ; i < 10 ; i++){
		if(ClearSkillPointCODE[i] ==CODE){
			ClearSkillCODE = ClearSkillPointCODE[i];
			return TRUE;
		}
		else
			ClearSkillCODE = 0;
	}
	
	return FALSE;
	
}

////////////// 초기화할 스킬포인트에 초기화 버튼을 띄운다 
int cSKILL::ShowClearSkillPointButton(int x , int y)
{
	ClearSkillButtonPosi.x = -50;   
	ClearSkillButtonPosi.y = -50;  

	if(ClearSkillCODE && !sinMessageBoxShowFlag){   
		ClearSkillButtonPosi.x = x-57;  
		ClearSkillButtonPosi.y = y-51;  
		DrawSprite(ClearSkillButtonPosi.x , ClearSkillButtonPosi.y ,	lpSkillDown,0,0,50,44);
	}

	return TRUE;
}

////////////// 특정 스킬포인트를 초기화한다 
int cSKILL::ClearSkillPointSection( )
{

	CheckingNowSkillState(); //스킬포인트를 
	CheckCharForm();//인증 
	int Count = 0;
	for(int j = 1 ; j < ChangeJobSkillPlus ; j ++){ 
		if(sinSkill.UseSkill[j].Flag && sinSkill.UseSkill[j].Point > 1){
			for(int i = 0 ; i < 10 ; i++){
				if(sinSkill.UseSkill[j].CODE== ClearSkillPointCODE[i]){ 
					cSkill.CheckSkillMasteryForm(0,j); // 스킬 마스터리를 체크한다
					CheckSkillPointForm(&sinSkill.UseSkill[j]);
					sinSkill.UseSkill[j].Point--; //스킬포인트 초기화 
					sinSkill.UseSkill[j].GageLength   = 0;
					sinSkill.UseSkill[j].GageLength2  = 0;
					sinSkill.UseSkill[j].Mastery      = 0;
					sinSkill.UseSkill[j].UseSKillIncreCount = 0;
					sinSkill.UseSkill[j].UseSkillCount = 0;
					sinSkill.UseSkill[j].UseSkillMastery = 0;
					sinSkill.UseSkill[j].UseSkillMasteryGage = 0;
					sinSkill.UseSkill[j].Use = 0;
					ReformSkillPointForm(&sinSkill.UseSkill[j]);
					ReformSkillMasteryForm(0,j);
					sinSkill.SkillPoint ++;
					ClearSkillCODE = 0;
					LearnSkillEffect(0);
					ReformCharForm();//재인증 
					return TRUE;
				}
			}		
		}
	}
	return TRUE;
}


unsigned int CharSkillInfoData[8] = {0,}; //캐릭정보 CheckSum이셋팅될 배열 
int   ChangeJob = -1;
//캐릭터별 SkillInfo를 갱신한다 
int cSKILL::ReFormCharSkillInfo()
{
	int   i = 0 ,j = 0, k = 0;
	int   CharNum = 0;
	
	int   CharCode[8] = {SIN_CHAR_MECANICIAN,SIN_CHAR_FIGHTER,SIN_CHAR_ARCHER,SIN_CHAR_PIKEMAN,SIN_CHAR_KNIGHT,SIN_CHAR_ATALANTA,SIN_CHAR_PRIESTESS,SIN_CHAR_MAGICIAN};
	//자신의 직업을 구한다 
	for(i = 0 ; i < 8 ; i++){
		if(sinChar->JOB_CODE == CharCode[i] ){
			CharNum = i;
			break;
		}
	}

	if(ChangeJob != sinChar->ChangeJob){
		ChangeJob = sinChar->ChangeJob;
		CharSkillInfoData[CharNum] = 0; //초기화 
		for(j=1;j < SIN_MAX_USE_SKILL; j++){
			if(sinSkill.UseSkill[j].Flag){
				k = j+1;
				CharSkillInfoData[CharNum] += k*sinSkill.UseSkill[j].Skill_Info.RequireLevel;
				CharSkillInfoData[CharNum] += k*sinSkill.UseSkill[j].Skill_Info.UseStamina[0];
				CharSkillInfoData[CharNum] += k*sinSkill.UseSkill[j].Skill_Info.UseStamina[1];
			}
		}
	}
	return TRUE;
}

//캐릭터별 SkillInfo를 체크한다
int cSKILL::CheckCharSkillInfo()
{
	//DWORD	dwXorKey = lpCurPlayer->dwObjectSerial&0x5F;
	int   i = 0 ,j = 0, k = 0;
	int   CharNum = 0;
	int   CharCode[8] = {SIN_CHAR_MECANICIAN,SIN_CHAR_FIGHTER,SIN_CHAR_ARCHER,SIN_CHAR_PIKEMAN,SIN_CHAR_KNIGHT,SIN_CHAR_ATALANTA,SIN_CHAR_PRIESTESS,SIN_CHAR_MAGICIAN};
	

//	unsigned int CharSkillInfoData[8] = {2549,2597,2246,2230,0,0,0,0};
	
	unsigned int CharSkillInfoDataCheckSum[8] ={0,};

	//자신의 직업을 구한다 
	for(i = 0 ; i < 8 ; i++){
		if(sinChar->JOB_CODE == CharCode[i] ){
			CharNum = i;
			break;
		}
	}

	//각각 캐릭터에 맞는 스킬테이블을 체크한다 
	for(j=1;j < SIN_MAX_USE_SKILL; j++){
		if(sinSkill.UseSkill[j].Flag){
			k = j+1;
			CharSkillInfoDataCheckSum[CharNum] += k*sinSkill.UseSkill[j].Skill_Info.RequireLevel;
			CharSkillInfoDataCheckSum[CharNum] += k*sinSkill.UseSkill[j].Skill_Info.UseStamina[0];
			CharSkillInfoDataCheckSum[CharNum] += k*sinSkill.UseSkill[j].Skill_Info.UseStamina[1];

			if(!sinSkill.UseSkill[j].UseSkillCount)continue;
			if(!CheckSkillMasteyData[j]){
				CheckSkillMasteyData[j]  = sinSkill.UseSkill[j].UseSkillCount;//^dwXorKey;
			}
			if(CheckSkillMasteyData[j] <(sinSkill.UseSkill[j].UseSkillCount)-200){
				SendSetHackUser3(55,CheckSkillMasteyData[j],sinSkill.UseSkill[j].UseSkillCount); //해킹을 하려고했던 못된 유저를 고발 TRUE 접속 종료 
				return FALSE;

			}
			else{
				CheckSkillMasteyData[j]  = sinSkill.UseSkill[j].UseSkillCount;//^dwXorKey;

			}
		}
	}
	if(CharSkillInfoDataCheckSum[CharNum] != CharSkillInfoData[CharNum]){
		//SendSetHackUser(51); //해킹을 하려고했던 못된 유저를 고발 TRUE 접속 종료 
		SendSetHackUser3(51,CharSkillInfoDataCheckSum[CharNum],CharSkillInfoData[CharNum]);
		return FALSE;
	}
   

	return TRUE;
}

int cSKILL::CheckSkillMasteryForm( sSKILL *pSkill , int Index)
{
	if(pSkill){
		for(int j=1;j < SIN_MAX_USE_SKILL; j++){
			if(sinSkill.UseSkill[j].Flag){
				if(pSkill->CODE == sinSkill.UseSkill[j].CODE){
					Index = j;
					break;
				}
			}
		}
	}
	//if(!CheckMasteryData[Index])return FALSE; //데이타가 없을경우에는 넘어간다 


	int CheckMasteryDataSum = lpCurPlayer->dwObjectSerial+sinSkill.UseSkill[Index].CODE;

	CheckMasteryDataSum += (sinSkill.UseSkill[Index].GageLength*Index);
	CheckMasteryDataSum += (sinSkill.UseSkill[Index].GageLength2*Index);
	CheckMasteryDataSum += (sinSkill.UseSkill[Index].Mastery*Index);
	
	if(CheckMasteryData[Index] != CheckMasteryDataSum){
		//CheckMasteryDataSum = 0; 테스트용 
		//SendSetHackUser(52); //해킹을 하려고했던 못된 유저를 고발 TRUE 접속 종료 
		SendSetHackUser3(52,CheckMasteryData[Index],CheckMasteryDataSum);
		return FALSE;

	}

	return TRUE; 
}
int cSKILL::ReformSkillMasteryForm( sSKILL *pSkill ,int Index)
{
	
	if(pSkill){
		for(int j=1;j < SIN_MAX_USE_SKILL; j++){
			if(sinSkill.UseSkill[j].Flag){
				if(pSkill->CODE == sinSkill.UseSkill[j].CODE){
					Index = j;
				}
			}
		}
	}

	CheckMasteryData[Index] = lpCurPlayer->dwObjectSerial+sinSkill.UseSkill[Index].CODE; //초기화
	CheckMasteryData[Index] += (sinSkill.UseSkill[Index].GageLength*Index);
	CheckMasteryData[Index] += (sinSkill.UseSkill[Index].GageLength2*Index);
	CheckMasteryData[Index] += (sinSkill.UseSkill[Index].Mastery*Index);

	return TRUE;
}

//스킬포인트를 체크한다
int cSKILL::CheckSkillPointForm(sSKILL *pSkill)
{
	DWORD	dwXorKey = lpCurPlayer->dwObjectSerial&0x5F;

	for(int j=1;j < SIN_MAX_USE_SKILL; j++){
		if(sinSkill.UseSkill[j].Flag &&sinSkill.UseSkill[j].Use &&sinSkill.UseSkill[j].Point){
			if(pSkill->CODE == sinSkill.UseSkill[j].CODE){
				//if(CheckSkillPointData[j]){
					if(CheckSkillPointData[j] != (sinSkill.UseSkill[j].Point^dwXorKey) ){
						SendSetHackUser3(54,CheckSkillPointData[j],sinSkill.UseSkill[j].Point);
						return FALSE;
					}
				//}
			}
		}
	}
	
	return TRUE;
}

//스킬포인트를 저장한다
int cSKILL::ReformSkillPointForm(sSKILL *pSkill)
{
	DWORD	dwXorKey = lpCurPlayer->dwObjectSerial&0x5F;

	for(int j=1;j < SIN_MAX_USE_SKILL; j++){
		if(sinSkill.UseSkill[j].Flag ){
			if(pSkill->CODE == sinSkill.UseSkill[j].CODE){
				CheckSkillPointData[j] = ( sinSkill.UseSkill[j].Point^dwXorKey);
				break;
			}
		}
	}
	return TRUE;
}

//스킬포인트 첫포인트를 저장한다 
int cSKILL::InitReformSkillPointForm()
{
	/*
	DWORD	dwXorKey = lpCurPlayer->dwObjectSerial&0x5F;
	
	for(int j=1;j < SIN_MAX_USE_SKILL; j++){
		if(sinSkill.UseSkill[j].Flag && sinSkill.UseSkill[j].Use && sinSkill.UseSkill[j].Point){
			CheckSkillPointData[j] = ( sinSkill.UseSkill[j].Point^dwXorKey);
		}
	}
	*/
	
	return TRUE;
}

//스킬의 스킬포인트를 얻어온다
int cSKILL::GetSkillPoint(DWORD CODE)
{
	for(int j=1;j < SIN_MAX_USE_SKILL; j++){
		if(sinSkill.UseSkill[j].Flag){
			if(CODE == sinSkill.UseSkill[j].CODE){
				return sinSkill.UseSkill[j].Point;
			}
		}
	}

	return 0;	
}

//현재사용중인 유지형 스킬을 찾아서 포인트를 리턴한다
int cSKILL::SearchContiueSkill(DWORD CODE)
{
	for(int k = 0; k < 10 ; k++){
		if(ContinueSkill[k].Flag){
			if(ContinueSkill[k].CODE ==CODE){
				return ContinueSkill[k].Point;

			}
		}
	}
	return 0;
}

//현재 사용중인 유지형스킬의 인덱스를 찾아 리턴한다
int cSKILL::SearchContiueSkillIndex(DWORD CODE)
{
	for(int k = 0; k < 10 ; k++){
		if(ContinueSkill[k].Flag){
			if(ContinueSkill[k].CODE ==CODE){
				return k+1;
			}
		}
	}
	return 0;
}
//현재 사용중인 유지형스킬의 코드를 찾아 리턴한다
int cSKILL::SearchContiueSkillCODE(DWORD CODE)
{
	for(int k = 0; k < 10 ; k++){
		if(ContinueSkill[k].Flag){
			if(ContinueSkill[k].CODE ==CODE){
				return ContinueSkill[k].CODE;
			}
		}
	}
	return 0;
}

//버추얼라이프를 셋팅한다
int AddVirtualLife[2] = {0,0};
int sinVirtualLifeTime = 0;
int sinVirtualLifePercent = 0;

// 박재원 - 부스터 아이템(생명력)
int AddBoosterLife = 0;
int BoosterLifeTime = 0;
int BoosterLifePercent = 0;
int LifeFlag = 0;
short tempLife[2] = {0,0};

// 박재원 - 부스터 아이템(기력)
int AddVirtualMana[2] = {0,0};
int sinVirtualManaTime = 0;
int sinVirtualManaPercent = 0;
int ManaFlag = 0;
short tempMana[2] = {0,0};

// 박재원 - 부스터 아이템(근력)
int AddVirtualStamina[2] = {0,0};
int sinVirtualStaminaTime = 0;
int sinVirtualStaminaPercent = 0;
int StaminaFlag = 0;
short tempStamina[2] = {0,0};

// 장별 - 스킬 딜레이
int sinAddSkillDelay = 0;
int sinSkillDelayTime = 0;
int SkillDelayFlag = 0;

int cSKILL::SetVirtualLife(int Time , int Point)
{

	//Contiue 초기화
	for(int i = 0 ; i < MAX_CONTINUE_SKILL ; i++){
		if(ContinueSkill[i].Flag){
			if(ContinueSkill[i].CODE == SKILL_VIRTUAL_LIFE){
				//memset(&ContinueSkill[i],0,sizeof(sSKILL));
				return FALSE; //중복시전 불가
			}
		}
	}

	for(int i = 0 ; i < MAX_CONTINUE_SKILL ; i++){
		if(!ContinueSkill[i].Flag){
			ContinueSkill[i].Flag = 1;
			ContinueSkill[i].CODE = SKILL_VIRTUAL_LIFE;
			ContinueSkill[i].UseTime = Time;
			ContinueSkill[i].Point = Point;
			SearchSkillDoc(&ContinueSkill[i]);
			break;
		}
	}

	AddVirtualLife[1]  = ((int)sinChar->Life[1] * Virtual_Life_Percent[Point-1])/100;
	sinVirtualLifeTime = Time;
	sinVirtualLifePercent = Virtual_Life_Percent[Point-1];
	CheckVirtualLifeTime = 0; //체크시간초기화
	cInvenTory.SetItemToChar();
	return TRUE;
}

// 박재원 - 부스터 아이템
int cSKILL::UseBoosterItem(sITEM *pItem)
{
	if(lpCurPlayer->OnStageField>=0 && StageField[ lpCurPlayer->OnStageField ]->State == FIELD_STATE_VILLAGE)
		return FALSE; //마을에서는 못사용한다

	DWORD BoosterCODE;
	DWORD ItemMask;
	int BoosterTime = 0;
	int cnt = 0;

	ItemMask = pItem->CODE&sinITEM_MASK3;
	if(ItemMask==sin21 || ItemMask==sin24 || ItemMask==sin27) cnt = 0;
	if(ItemMask==sin22 || ItemMask==sin25 || ItemMask==sin28) cnt = 1;
	if(ItemMask==sin23 || ItemMask==sin26 || ItemMask==sin29) cnt = 2;

	switch(ItemMask){
		case sin21: // 생명력 - 1시간
		case sin22: // 생명력 - 3시간
		case sin23: // 생명력 - 1일
			BoosterCODE = BOOSTER_ITEM_LIFE;
			BoosterTime = BoosterItem_UseTime[cnt];

			AddBoosterLife = ((int)sinChar->Life[1] * BoosterItem_DataPercent[0])/100;
			BoosterLifeTime = BoosterTime;
			BoosterLifePercent = BoosterItem_DataPercent[0];
			CheckBoosterLifeTime = 0; //체크시간초기화
			break;
		case sin24: // 기력 - 1시간
		case sin25: // 기력 - 3시간
		case sin26: // 기력 - 1일
			BoosterCODE = BOOSTER_ITEM_MANA;
			BoosterTime = BoosterItem_UseTime[cnt]; 

			// 박재원 - 부스터 아이템(기력)
			AddVirtualMana[1]  = ((int)sinChar->Mana[1] * BoosterItem_DataPercent[1])/100;
			sinVirtualManaTime = BoosterTime;
			sinVirtualManaPercent = BoosterItem_DataPercent[1];
			CheckVirtualManaTime = 0; //체크시간초기화
			break;
		case sin27: // 근력 - 1시간
		case sin28: // 근력 - 3시간
		case sin29: // 근력 - 1일
			BoosterCODE = BOOSTER_ITEM_STAMINA;
			BoosterTime = BoosterItem_UseTime[cnt]; 

			// 박재원 - 부스터 아이템(근력)
			AddVirtualStamina[1]  = ((int)sinChar->Stamina[1] * BoosterItem_DataPercent[2])/100;
			sinVirtualStaminaTime = BoosterTime;
			sinVirtualStaminaPercent = BoosterItem_DataPercent[2];
			CheckVirtualStaminaTime = 0; //체크시간초기화
			break;
	}
	//해당 돼는 아이템이 맞으면 아이템스킬을 세팅한다.
	sSKILL TempSkill;

	if(cSkill.SearchContiueSkillCODE(BoosterCODE)==BoosterCODE && BoosterCODE != 0){
		cMessageBox.ShowMessage(MESSAGE_CLANSKILL_USE);
		haCastleSkillUseFlag = 0;
		return TRUE;
	}
	haCastleSkillUseFlag =1;

	for(int j = 0 ; j < SIN_MAX_SKILL; j++){
		if(sSkill[j].CODE == BoosterCODE){
			memcpy(&TempSkill,&sSkill[j],sizeof(sSKILL));
			TempSkill.UseTime=BoosterTime;
			sinContinueSkillSet(&TempSkill);
			SwitchSkill(&TempSkill);
			break;
		}
	}

	tempLife[1] = sinChar->Level;
	tempMana[1] = sinChar->Level;
	tempStamina[1] = sinChar->Level;

	sinPlaySound(SIN_SOUND_EAT_POTION2);//대박 사운드
	cInvenTory.SetItemToChar();
	return TRUE;
}

// 박재원 - 데미지 부스터를 셋팅한다
int cSKILL::SetBoosterItem(DWORD ItemCODE , int Time)
{
	int UseFlag = 0; 
	sSKILL TempSkill;
	memset(&TempSkill,0,sizeof(sSKILL));
	char *Path;

	DWORD ItemMask;
	int cnt = 0;

	ItemMask = ItemCODE&sinITEM_MASK3;

	if(ItemMask==sin21 || ItemMask==sin24 || ItemMask==sin27) cnt = 0; // 1시간
	if(ItemMask==sin22 || ItemMask==sin25 || ItemMask==sin28) cnt = 1; // 3시간
	if(ItemMask==sin23 || ItemMask==sin26 || ItemMask==sin29) cnt = 2; // 1일

	switch(ItemMask)
	{
		// 생명력 부스터
	case sin21 :  
	case sin22 :
	case sin23 :
		UseFlag = 0;
		Path = "image\\Sinimage\\skill\\keep\\Booster_01.tga";
		TempSkill.CODE = BOOSTER_ITEM_LIFE;
		TempSkill.Point = 0; // 부스터 아이템은 레벨이 없다.
		TempSkill.UseTime = BoosterItem_UseTime[cnt];
		TempSkill.CheckTime = (BoosterItem_UseTime[cnt]*70) - (Time*70); 
		TempSkill.MatIcon = Mat_BoosterItem[UseFlag];
		break;

		// 기력 부스터
	case sin24 :  
	case sin25 :
	case sin26 :  
		UseFlag = 1;
		Path = "image\\Sinimage\\skill\\keep\\Booster_02.tga";
		TempSkill.CODE = BOOSTER_ITEM_MANA;
		TempSkill.Point = 0; // 부스터 아이템은 레벨이 없다.
		TempSkill.UseTime = BoosterItem_UseTime[cnt];
		TempSkill.CheckTime = (BoosterItem_UseTime[cnt]*70) - (Time*70); 
		TempSkill.MatIcon = Mat_BoosterItem[UseFlag];
		break;

		// 근력 부스터
	case sin27 :  
	case sin28 :
	case sin29 :   
		UseFlag = 2;
		Path = "image\\Sinimage\\skill\\keep\\Booster_03.tga";
		TempSkill.CODE = BOOSTER_ITEM_STAMINA;
		TempSkill.Point = 0; // 부스터 아이템은 레벨이 없다.
		TempSkill.UseTime = BoosterItem_UseTime[cnt];
		TempSkill.CheckTime = (BoosterItem_UseTime[cnt]*70) - (Time*70); 
		TempSkill.MatIcon = Mat_BoosterItem[UseFlag];
		break;
	}

	char szBuff[128];
	if(!Mat_BoosterItem[UseFlag]){
		wsprintf(szBuff,Path,UseFlag);
		Mat_BoosterItem[UseFlag] = CreateTextureMaterial(szBuff, 0, 0, 0,0, SMMAT_BLEND_ALPHA );
		ReadTextures();	//텍스쳐 로딩
	}

	sinContinueSkillSet(&TempSkill);

	if(UseFlag == 0)
	{
		// 박재원 - 부스터 아이템(생명력)
		AddBoosterLife  = ((int)sinChar->Life[1] * BoosterItem_DataPercent[UseFlag])/100;
		BoosterLifeTime = Time;
		BoosterLifePercent = BoosterItem_DataPercent[UseFlag];
		CheckBoosterLifeTime = 0; //체크시간초기화
	}
	else if(UseFlag == 1)
	{
		// 박재원 - 부스터 아이템(기력)
		AddVirtualMana[1]  = ((int)sinChar->Mana[1] * BoosterItem_DataPercent[UseFlag])/100;
		sinVirtualManaTime = Time;
		sinVirtualManaPercent = BoosterItem_DataPercent[UseFlag];
		CheckVirtualManaTime = 0; //체크시간초기화
	}
	else if(UseFlag==2)
	{
		// 박재원 - 부스터 아이템(근력)
		AddVirtualStamina[1]  = ((int)sinChar->Stamina[1] * BoosterItem_DataPercent[UseFlag])/100;
		sinVirtualStaminaTime = Time;
		sinVirtualStaminaPercent = BoosterItem_DataPercent[UseFlag];
		CheckVirtualStaminaTime = 0; //체크시간초기화
	}

	cInvenTory.SetItemToChar();

	return TRUE;
}

// 장별 - 스킬 딜레이
int cSKILL::SetSkillDelayItem(DWORD ItemCODE , int Time)
{
	int UseFlag = 0; 
	sSKILL TempSkill;
	memset(&TempSkill,0,sizeof(sSKILL));
	char *Path;

	DWORD ItemMask;
	int cnt = 0;

	ItemMask = ItemCODE&sinITEM_MASK3;

	if(ItemMask==sin30 || ItemMask==sin33 || ItemMask==sin36) cnt = 0;
	if(ItemMask==sin31 || ItemMask==sin34 || ItemMask==sin37) cnt = 1;
	if(ItemMask==sin32 || ItemMask==sin35 || ItemMask==sin38) cnt = 2;

	switch(ItemMask)
	{
		// 생명력 부스터
	case sin30 :  
	case sin31 :
	case sin32 :
		UseFlag = 0;
		Path = "image\\Sinimage\\skill\\keep\\Booster_01.tga";
		TempSkill.CODE = SKILLDELAY_ITEM_LIFE;
		TempSkill.Point = 0; 
		TempSkill.UseTime = nSkillDelay_UseTime[cnt];
		TempSkill.CheckTime = (nSkillDelay_UseTime[cnt]*70) - (Time*70); 
		TempSkill.MatIcon = nMat_SkillDelayItem[UseFlag];
		break;
	}


	char szBuff[128];
	if(!nMat_SkillDelayItem[UseFlag]){
		wsprintf(szBuff,Path,UseFlag);
		nMat_SkillDelayItem[UseFlag] = CreateTextureMaterial(szBuff, 0, 0, 0,0, SMMAT_BLEND_ALPHA );
		ReadTextures();	//텍스쳐 로딩
	}

	sinContinueSkillSet(&TempSkill);

	if(UseFlag == 0)
	{
		sinAddSkillDelay  = 10;
		sinSkillDelayTime = Time;
		BoosterLifePercent = BoosterItem_DataPercent[UseFlag];
		CheckSkillDelayTime = 0; //체크시간초기화
	}


	cInvenTory.SetItemToChar();

	return TRUE;
}


// 장별 - 스킬 딜레이
int cSKILL::UseSkillDelayItem(sITEM *pItem)
{
	DWORD SkillDelayCODE;
	DWORD ItemMask;
	int SkillDelayTime = 0;
	int cnt = 0;

	ItemMask = pItem->CODE&sinITEM_MASK3;
	if(ItemMask==sin30 || ItemMask==sin33 || ItemMask==sin36) cnt = 0;
	if(ItemMask==sin31 || ItemMask==sin34 || ItemMask==sin37) cnt = 1;
	if(ItemMask==sin32 || ItemMask==sin35 || ItemMask==sin38) cnt = 2;

	switch(ItemMask){
		case sin30: // 1시간
		case sin31: // 3시간
		case sin32: // 1일
			SkillDelayCODE = SKILLDELAY_ITEM_LIFE;
			SkillDelayTime = nSkillDelay_UseTime[cnt];

			sinAddSkillDelay = 10;
			sinSkillDelayTime = SkillDelayTime;
			CheckSkillDelayTime = 0; //체크시간초기화
			break;
	}
	//해당 돼는 아이템이 맞으면 아이템스킬을 세팅한다.
	sSKILL TempSkill;

	if(cSkill.SearchContiueSkillCODE(SkillDelayCODE)==SkillDelayCODE && SkillDelayCODE != 0){
		cMessageBox.ShowMessage(MESSAGE_CLANSKILL_USE);
		haCastleSkillUseFlag = 0;
		return TRUE;
	}
	haCastleSkillUseFlag =1;

	for(int j = 0 ; j < SIN_MAX_SKILL; j++){
		if(sSkill[j].CODE == SkillDelayCODE){
			memcpy(&TempSkill,&sSkill[j],sizeof(sSKILL));
			TempSkill.UseTime=SkillDelayTime;
			sinContinueSkillSet(&TempSkill);
			SwitchSkill(&TempSkill);
			break;
		}
	}

	sinPlaySound(SIN_SOUND_EAT_POTION2);//대박 사운드
	cInvenTory.SetItemToChar();
	return TRUE;
}


//트라이엄프 오브 발할라
int cSKILL::SetT_Of_Valhalla( int Point , int LV)
{

	//Contiue 초기화
	for(int i = 0 ; i < MAX_CONTINUE_SKILL ; i++){
		if(ContinueSkill[i].Flag){
			if(ContinueSkill[i].CODE == SKILL_TRIUMPH_OF_VALHALLA ||ContinueSkill[i].CODE == SKILL_HALL_OF_VALHALLA){
				//memset(&ContinueSkill[i],0,sizeof(sSKILL));
				return FALSE; //중복시전 불가
			}
		}
	}

	for(int i = 0 ; i < MAX_CONTINUE_SKILL ; i++){
		if(!ContinueSkill[i].Flag){
			ContinueSkill[i].Flag = 1;
			ContinueSkill[i].CODE = SKILL_TRIUMPH_OF_VALHALLA;
			ContinueSkill[i].UseTime = T_Of_Valhalla_Time[Point-1];
			ContinueSkill[i].Point = Point;
			SearchSkillDoc(&ContinueSkill[i]);
			break;
		}
	}

	SetT_Of_ValhallaLV = LV;

	if(SetT_Of_ValhallaFlag2){ //자신이 사용한것이면
		SetT_Of_ValhallaFlag2 = 0;
		SetT_Of_ValhallaFlag  = 0;
	}
	else{
		SetT_Of_ValhallaFlag  = 1; //남이사용해준것이면

	}
	cInvenTory.SetItemToChar();
	return TRUE;
}

//인첸트 웨폰
int cSKILL::SetEnchant_Weapon(int Time , int Point)
{

	//Contiue 초기화
	for(int i = 0 ; i < MAX_CONTINUE_SKILL ; i++){
		if(ContinueSkill[i].Flag){
			if(ContinueSkill[i].CODE == SKILL_ENCHANT_WEAPON){
				//memset(&ContinueSkill[i],0,sizeof(sSKILL));
				return FALSE; //중복시전 불가
			}
		}
	}

	for(int i = 0 ; i < MAX_CONTINUE_SKILL ; i++){
		if(!ContinueSkill[i].Flag){
			ContinueSkill[i].Flag = 1;
			ContinueSkill[i].CODE = SKILL_ENCHANT_WEAPON;
			ContinueSkill[i].UseTime = Time;
			ContinueSkill[i].Point = Point;
			SearchSkillDoc(&ContinueSkill[i]);
			break;
		}
	}
	cInvenTory.SetItemToChar();
	return TRUE;
}

//포스오브네이쳐
int cSKILL::ForeceOfNature(int Time , int Point , int Party)
{

	//Contiue 초기화
	for(int i = 0 ; i < MAX_CONTINUE_SKILL ; i++){
		if(ContinueSkill[i].Flag){
			if(ContinueSkill[i].CODE == SKILL_FORCE_OF_NATURE){
				if(ContinueSkill[i].Flag == 2 && !Party ){
					memset(&ContinueSkill[i],0,sizeof(sSKILL));
					break;
				}
				else return FALSE; //중복시전 불가
			}
		}
	}

	for(int i = 0 ; i < MAX_CONTINUE_SKILL ; i++){
		if(!ContinueSkill[i].Flag){
			ContinueSkill[i].Flag = Party+1; //파티원은 2로한다
			ContinueSkill[i].CODE = SKILL_FORCE_OF_NATURE;
			ContinueSkill[i].UseTime = Time;
			ContinueSkill[i].Point = Point;
			SearchSkillDoc(&ContinueSkill[i]);
			break;
		}
	}
	cInvenTory.SetItemToChar();
	return TRUE;
}

//헬오브 발할라
int cSKILL::HellOfValhalla(int Time , int Point,int TVPoint , int LV , int Party)
{

	//Contiue 초기화
	for(int i = 0 ; i < MAX_CONTINUE_SKILL ; i++){
		if(ContinueSkill[i].Flag){
			if(ContinueSkill[i].CODE == SKILL_HALL_OF_VALHALLA){
				if(ContinueSkill[i].Flag == 2 && !Party){
					memset(&ContinueSkill[i],0,sizeof(sSKILL));
					break;
				}
				else return FALSE; //중복시전 불가
			}
		}
	}

	for(int i = 0 ; i < MAX_CONTINUE_SKILL ; i++){
		if(!ContinueSkill[i].Flag){
			ContinueSkill[i].Flag = Party+1; //파티원은 2로한다
			ContinueSkill[i].CODE = SKILL_HALL_OF_VALHALLA;
			ContinueSkill[i].UseTime = Time;
			ContinueSkill[i].Point = Point;
			SearchSkillDoc(&ContinueSkill[i]);
			break;
		}
	}
	SetT_Of_ValhallaPOINT = TVPoint;
	SetH_Of_ValhallaLV = LV;    //시전자의 레벨
	cInvenTory.SetItemToChar();
	return TRUE;
}


//리제너레이터 필드
int cSKILL::Regeneration_Field(int Time , int Point , int Party)
{

	//Contiue 초기화
	for(int i = 0 ; i < MAX_CONTINUE_SKILL ; i++){
		if(ContinueSkill[i].Flag){
			if(ContinueSkill[i].CODE == SKILL_REGENERATION_FIELD){
				if(ContinueSkill[i].UseTime > Time)return FALSE; //같은 캐릭사용시 리턴
				ContinueSkill[i].UseTime = Time;
				return TRUE; //시간갱신
			}
		}
	}

	for(int i = 0 ; i < MAX_CONTINUE_SKILL ; i++){
		if(!ContinueSkill[i].Flag){
			ContinueSkill[i].Flag = 1+Party; //파티원은 2로한다
			ContinueSkill[i].CODE = SKILL_REGENERATION_FIELD;
			ContinueSkill[i].UseTime = Time;
			ContinueSkill[i].Point = Point;
			SearchSkillDoc(&ContinueSkill[i]);
			break;
		}
	}
	cInvenTory.SetItemToChar();
	return TRUE;
}

//스킬 내용을 찾는다
int cSKILL::SearchSkillDoc(sSKILL *pSkill)
{

	for(int i = 0; i < SIN_MAX_SKILL ; i++){
		if(pSkill->CODE == sSkill_Info[i].CODE){
			if(pSkill->CODE == SKILL_FORCE_ORB){
#ifdef _LANGUAGE_CHINESE
				wsprintf(pSkill->Skill_Info.SkillName,"%s%s",SheltomName2[pSkill->Point-1],sSkill_Info[i].SkillName);
#else
				if(pSkill->ElementIndex == 1000) // 박재원 - 빌링 매직 포스 유지형 아이콘 글자 표시
				{
					wsprintf(pSkill->Skill_Info.SkillName,"%s %s",BillingMagicForceName,sSkill_Info[i].SkillName);
				}
				else if(pSkill->ElementIndex == 2000 )// 박재원 - 매직 포스 유지형 아이콘 글자 표시
				{
					wsprintf(pSkill->Skill_Info.SkillName,"%s %s %s",MagicForceName,SheltomName2[pSkill->Point-1],sSkill_Info[i].SkillName);
				}
				else
					wsprintf(pSkill->Skill_Info.SkillName,"%s %s",SheltomName2[pSkill->Point-1],sSkill_Info[i].SkillName);
#endif

			}
			else{
				lstrcpy(pSkill->Skill_Info.SkillName,sSkill_Info[i].SkillName);
			}
			lstrcpy(pSkill->Skill_Info.SkillDoc,sSkill_Info[i].SkillDoc);
			break;

		}

	}

	return TRUE;
}

//원그리기
int cSKILL::SkillBarDraw(sSKILL *pSkill , int x , int y, int Alpha)
{

	D3DTLVERTEX tlVertex[73];  //4개로 나눠서 노가다 좌표로 구한다 

	int Time = 0;   

	Time = (int)(((float)pSkill->CheckTime/((float)pSkill->UseTime*70.0f))*72.0f);   

	float fx , fy =0; 
	
	int i = 0 , j = 0;  
	int sinY,cosY;
	tlVertex[0].rhw = 1;
	tlVertex[0].color	= RGBA_MAKE(0,Alpha,Alpha,Alpha); 
	tlVertex[0].specular  =  RGBA_MAKE( 0,0,0,0 );
	tlVertex[0].sz = 0;

	tlVertex[0].sx = float(x);
	tlVertex[0].sy = float(y);
	tlVertex[0].tu = 0.5;
	tlVertex[0].tv = 0.5f;

	//요기서 타임을 체크해준다 
	for( i = 1 ; i < Time+1 ; i++){     
		tlVertex[i].rhw = 1;   
		if(pSkill->CODE == SKILL_FORCE_ORB){
			tlVertex[i].color = RGBA_MAKE(255,190,30,Alpha);
		}
		else
			tlVertex[i].color	= RGBA_MAKE(0,255,200,Alpha);
		tlVertex[i].specular  =  RGBA_MAKE( 0,0,0,0 );
		tlVertex[i].sz = 0;

		sinY = GetSin[ (1024/2)+(i* 56)&ANGCLIP];      
		cosY = GetCos[ (1024/2)+((i* 56)+ANGLE_180)&ANGCLIP ];

		fx = (float) (( 10*sinY + 10*cosY )>>8); 
		fy = (float) (( 10*cosY - 10*sinY )>>8);
		fx /= 256;
		fy /= 256;

		tlVertex[i].sx = (float)(fx+x);  
		tlVertex[i].sy = (float)(fy+y);
		tlVertex[i].tu = 0.5f;
		tlVertex[i].tv = 0.5f;

	}


	lpD3DDevice->SetTexture( 0, 0 );
	lpD3DDevice->DrawPrimitive( D3DPT_TRIANGLEFAN ,
							D3DFVF_TLVERTEX  ,
							tlVertex,
							i,
							NULL);

	return TRUE;
}



//지속 스킬 체크
int cSKILL::CheckContinueIcon()
{
	for(int i = 0 ; i < MAX_CONTINUE_SKILL ; i++){ 
		if(ContinueSkill[i].Flag){
			if(!ContinueSkill[i].MatIcon){
				for(int j = 0 ; j < SIN_MAX_SKILL; j++){
					if(ContinueSkill[i].CODE == sSkill[j].CODE){
						if(sSkill[j].MatIcon){ 
							ContinueSkill[i].MatIcon = sSkill[j].MatIcon;
							return TRUE;
						}
					}
				}
			}
		}
	}
	return FALSE;
}

//지속 스킬 체크
int cSKILL::CancelContinueSkill(DWORD CODE)
{

	for(int i = 0 ; i < MAX_CONTINUE_SKILL ; i++){ 
		if(ContinueSkill[i].Flag){
			if(ContinueSkill[i].CODE == CODE){
				memset(&ContinueSkill[i],0,sizeof(sSKILL));
				cInvenTory.SetItemToChar(); //스텟을 바꿔준다 (색이있을경우는 스위치로 바꿔준다)
				break;
			}
		}
	}
	return FALSE;
}

//스킬단축기 초기화
int cSKILL::SkillShortKeyInit()
{
	for(int j=1;j < SIN_MAX_USE_SKILL; j++){
		sinSkill.UseSkill[j].ShortKey = 0;
	}
	return TRUE;
}

// 박재원 - 스킬 마스터(전직별로 스킬의 숙련 여부를 체크한다.)
int cSKILL::CheckMaturedSkill(int ChangeJob)
{
	int matured_skill_count = 0;

	switch(ChangeJob)
	{
	case 1:
		for(int j=1 ; j<5 ; j++) // 1차 전직후 생기는 스킬
		{
			if( sinSkill.UseSkill[j].UseSkillCount<10000 && sinSkill.UseSkill[j].Skill_Info.USECODE != SIN_SKILL_USE_NOT && 
				sinSkill.UseSkill[j].UseSkillMastery)
			{
				matured_skill_count++;
			}
		}
		break;
	case 2:
		for(int j=5 ; j<9 ; j++) // 2차 전직후 생기는 스킬
		{
			if( sinSkill.UseSkill[j].UseSkillCount<10000 && sinSkill.UseSkill[j].Skill_Info.USECODE != SIN_SKILL_USE_NOT && 
				sinSkill.UseSkill[j].UseSkillMastery)
			{
				matured_skill_count++;
			}
		}
		break;
	case 3:
		for(int j=9 ; j<13 ; j++) // 3차 전직후 생기는 스킬
		{
			if( sinSkill.UseSkill[j].UseSkillCount<10000 && sinSkill.UseSkill[j].Skill_Info.USECODE != SIN_SKILL_USE_NOT && 
				sinSkill.UseSkill[j].UseSkillMastery)
			{
				matured_skill_count++;
			}
		}
		break;
	}

	if(matured_skill_count <= 4 && matured_skill_count>0)
		return TRUE; // 사용 가능
	else
		return FALSE;  // 사용 불가능
}
/*
//원그리기
int cSKILL::SkillBarDraw(sSKILL *pSkill , int x , int y)
{

	D3DTLVERTEX tlVertex[72][4];  //4개로 나눠서 노가다 좌표로 구한다 

	int Time = 0; 
	float Time2 = 0;

	Time = (int)(((float)pSkill->CheckTime/((float)pSkill->UseTime*70.0f))*72.0f);   
	Time2++;

	float fx , fy =0; 
	
	int i = 0 , j = 0;
	int sinY,cosY;
	for( i = 0 ; i < Time ; i++){  
		for( j = 0 ; j < 4 ; j++){
			tlVertex[i][j].rhw = 1;
			tlVertex[i][j].color	= RGBA_MAKE(255,0,0,255);
			tlVertex[i][j].specular  =  RGBA_MAKE( 0,0,0,0 );
			tlVertex[i][j].sz = 0;
		}


		sinY = GetSin[ (i* 56)&ANGCLIP];      
		cosY = GetCos[ ((i* 56)+ANGLE_180)&ANGCLIP ];

		fx = (float) (( 16*sinY + 16*cosY )>>8); 
		fy = (float) (( 16*cosY - 16*sinY )>>8);
		fx /= 256;
		fy /= 256;

		//dsDrawColorBox( sinInvenColor[0] ,fx+x,fy+y, 16, 16 );

		tlVertex[i][0].sx = (float)(fx+x);  
		tlVertex[i][0].sy = (float)(fy+y);
		tlVertex[i][0].tu = 0;
		tlVertex[i][0].tv = 0;

		tlVertex[i][1].sx = float(x);
		tlVertex[i][1].sy = float(y);
		tlVertex[i][1].tu = 0;
		tlVertex[i][1].tv = 1.0f;


		sinY = GetSin[ ((i+1)* 56)&ANGCLIP];
		cosY = GetCos[ (((i+1)* 56)+ANGLE_180)&ANGCLIP ];

		fx = (float) (( 16*sinY + 16*cosY )>>8);
		fy = (float) (( 16*cosY - 16*sinY )>>8);
		fx /= 256;
		fy /= 256;

		tlVertex[i][2].sx = (float)(fx+x);
		tlVertex[i][2].sy = (float)(fy+y);
		tlVertex[i][2].tu = 1.0f;
		tlVertex[i][2].tv = 0;
		

	}


	lpD3DDevice->SetTexture( 0, 0 );
	for( i = 0 ; i < 72 ; i++){
		lpD3DDevice->DrawPrimitive( D3DPT_TRIANGLEFAN ,
							D3DFVF_TLVERTEX  ,
							tlVertex[i],
							3,
							NULL);
	}
	return TRUE;
}
*/
