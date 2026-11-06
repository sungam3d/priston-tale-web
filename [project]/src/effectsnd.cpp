#include "smlib3d\\smd3d.h"
#include "smwsock.h"

#include "character.h"
#include "srcsound\\dxwav.h"
#include "particle.h"
#include "fileread.h"
#include "playmain.h"
#include "drawsub.h"
#include "netplay.h"
#include "effectsnd.h"
#include "field.h"


#include "language.h"

extern char sangJuWav[NUM_SOUNDS][64];	// pluto wav 파일 추가 할때 갯수
extern char *sangJuWav2[NUM_SOUNDS];
extern int  sangJuWav_F[NUM_SOUNDS];

struct	snFIND_EFFECT {
	char	*lpStringCode;
	DWORD	CharSoundCode;
};


struct	snEFFECT {
	DWORD	RealSoundNum;
	DWORD	CharSoundCode;
	DWORD	MotionState;
	DWORD	WeaponState;
};

//#define	snFIND_EFFECT_MAX			76+14
#define	snFIND_EFFECT_MAX			167			// 박재원 - 저주받은 신전 3층(신규필드 몬스터) 5종 추가  // 117+34+5 (+1: Lord Hopy)
												// 박재원 - 끝없는 탑 3층(신규필드 몬스터) 7종 추가      // 117+34+5+7
												// 장별 - 수박몬스터 117+34+5+7+1
												// 장별 - 복날 몬스터 2종 추가 117+34+5+7+1+2

snFIND_EFFECT	snFindEffects[snFIND_EFFECT_MAX] = {
	////////////////////////// 1차 몬스터 //////////////////////////////
	{ "wav\\Effects\\monster\\cyclops\\"	,	snCHAR_SOUND_CYCLOPS	},
	{ "wav\\Effects\\monster\\hob goblin\\"	,	snCHAR_SOUND_HOBGOBLIN	},
	{ "wav\\Effects\\monster\\Imp\\"		,	snCHAR_SOUND_IMP		},
	{ "wav\\Effects\\monster\\miniG\\"		,	snCHAR_SOUND_MINIG		},
	{ "wav\\Effects\\monster\\plant\\"		,	snCHAR_SOUND_PLANT		},
	{ "wav\\Effects\\monster\\skeleton\\"	,	snCHAR_SOUND_SKELETON	},
	{ "wav\\Effects\\monster\\zombi\\"		,	snCHAR_SOUND_ZOMBI		},
	{ "wav\\Effects\\monster\\obit\\"		,	snCHAR_SOUND_OBIT		},

	{ "wav\\Effects\\monster\\hopt\\"		,	snCHAR_SOUND_HOPT		},
	{ "wav\\Effects\\monster\\bargon\\"		,	snCHAR_SOUND_BARGON		},
	{ "wav\\Effects\\monster\\leech\\"		,	snCHAR_SOUND_LEECH		},
	{ "wav\\Effects\\monster\\mushroom\\"	,	snCHAR_SOUND_MUSHROOM	},

	{ "wav\\Effects\\monster\\Arma\\"		,	snCHAR_SOUND_ARMA		},
	{ "wav\\Effects\\monster\\scorpion\\"	,	snCHAR_SOUND_SCORPION	},

	////////////////////////// 2차 몬스터 //////////////////////////////
	{ "wav\\Effects\\monster\\HeadCutter\\"	,	snCHAR_SOUND_HEADCUTTER	},
	{ "wav\\Effects\\monster\\Sandlem\\"	,	snCHAR_SOUND_SANDLEM	},
	{ "wav\\Effects\\monster\\Web\\"		,	snCHAR_SOUND_WEB		},
	{ "wav\\Effects\\monster\\HopyKing\\"	,	snCHAR_SOUND_HOPYKING	},
	{ "wav\\Effects\\monster\\Crip\\"		,	snCHAR_SOUND_CRIP		},
	{ "wav\\Effects\\monster\\Buma\\"		,	snCHAR_SOUND_BUMA		},
	{ "wav\\Effects\\monster\\Decoy\\"		,	snCHAR_SOUND_DECOY		},
	{ "wav\\Effects\\monster\\Doral\\"		,	snCHAR_SOUND_DORAL		},
	{ "wav\\Effects\\monster\\Figon\\"		,	snCHAR_SOUND_FIGON		},
	{ "wav\\Effects\\monster\\StoneGiant\\"	,	snCHAR_SOUND_STONEGIANT	},
	{ "wav\\Effects\\monster\\Greven\\"		,	snCHAR_SOUND_GREVEN		},
	{ "wav\\Effects\\monster\\illusionknight\\" ,snCHAR_SOUND_ILLUSIONKNIGHT },
	{ "wav\\Effects\\monster\\SkeletonRange\\"	,	snCHAR_SOUND_SKELETONRANGE},
	{ "wav\\Effects\\monster\\SkeletonMelee\\"	,	snCHAR_SOUND_SKELETONMELEE},

	{ "wav\\Effects\\monster\\wolverin\\"		,	snCHAR_SOUND_WOLVERLIN		},

	///////////////////////// 3차 몬스터 ///////////////////////////////
	{ "wav\\Effects\\monster\\Rabie\\"		,	snCHAR_SOUND_RABIE		},
	{ "wav\\Effects\\monster\\Mudy\\"		,	snCHAR_SOUND_MUDY		},
	{ "wav\\Effects\\monster\\Sen\\"		,	snCHAR_SOUND_SEN		},
	{ "wav\\Effects\\monster\\Egan\\"		,	snCHAR_SOUND_EGAN		},
	{ "wav\\Effects\\monster\\BeeDog\\"		,	snCHAR_SOUND_BEEDOG		},
	{ "wav\\Effects\\monster\\MutantPlant\\",	snCHAR_SOUND_MUTANTPLANT},
	{ "wav\\Effects\\monster\\MutantRabie\\",	snCHAR_SOUND_MUTANTRABIE},
	{ "wav\\Effects\\monster\\MutantTree\\"	,	snCHAR_SOUND_MUTANTTREE	},
	{ "wav\\Effects\\monster\\Avelisk\\"	,	snCHAR_SOUND_AVELISK	},
	{ "wav\\Effects\\monster\\Naz\\"		,	snCHAR_SOUND_NAZ		},
	{ "wav\\Effects\\monster\\Mummy\\"		,	snCHAR_SOUND_MUMMY		},
	{ "wav\\Effects\\monster\\Hulk\\"		,	snCHAR_SOUND_HULK		},
	{ "wav\\Effects\\monster\\Hungky\\"		,	snCHAR_SOUND_HUNGKY	},
	{ "wav\\Effects\\monster\\Succubus\\"	,	snCHAR_SOUND_SUCCUBUS	},
	{ "wav\\Effects\\monster\\Dawlin\\"		,	snCHAR_SOUND_DAWLIN		},
	{ "wav\\Effects\\monster\\Shadow\\"		,	snCHAR_SOUND_SHADOW		},
	{ "wav\\Effects\\monster\\Berserker\\"	,	snCHAR_SOUND_BERSERKER	},
	{ "wav\\Effects\\monster\\IronGuard\\"	,	snCHAR_SOUND_IRONGUARD	},
	{ "wav\\Effects\\monster\\Fury\\"		,	snCHAR_SOUND_FURY		},
	{ "wav\\Effects\\monster\\Sliver\\"		,	snCHAR_SOUND_SLIVER		},

	{ "wav\\Effects\\monster\\Ratoo\\"		,	snCHAR_SOUND_RATOO		},
	{ "wav\\Effects\\monster\\Stygianlord\\",	snCHAR_SOUND_STYGIANLORD},
	{ "wav\\Effects\\monster\\Omicron\\"	,	snCHAR_SOUND_OMICRON	},
	{ "wav\\Effects\\monster\\D-Machine\\"	,	snCHAR_SOUND_DMACHINE	},
	{ "wav\\Effects\\monster\\Metron\\"		,	snCHAR_SOUND_METRON		},

	{ "wav\\Effects\\monster\\MrGhost\\"	,	snCHAR_SOUND_MRGHOST	},


	{ "wav\\Effects\\monster\\VAMPIRICBAT\\",	snCHAR_SOUND_VAMPIRICBAT	},
	{ "wav\\Effects\\monster\\MIREKEEPER\\"	,	snCHAR_SOUND_MIREKEEPER		},
	{ "wav\\Effects\\monster\\MUFFIN\\"		,	snCHAR_SOUND_MUFFIN			},
	{ "wav\\Effects\\monster\\SOLIDSNAIL\\"	,	snCHAR_SOUND_SOLIDSNAIL		},
	{ "wav\\Effects\\monster\\BEEVIL\\"		,	snCHAR_SOUND_BEEVIL			},
	{ "wav\\Effects\\monster\\NIGHTMARE\\"	,	snCHAR_SOUND_NIGHTMARE		},
	{ "wav\\Effects\\monster\\STONEGOLEM\\"	,	snCHAR_SOUND_STONEGOLEM		},
	{ "wav\\Effects\\monster\\THORNCRAWLER\\",	snCHAR_SOUND_THORNCRAWLER	},
	{ "wav\\Effects\\monster\\HEAVYGOBLIN\\",	snCHAR_SOUND_HEAVYGOBLIN	},
	{ "wav\\Effects\\monster\\EVILPLANT\\"	,	snCHAR_SOUND_EVILPLANT		},
	{ "wav\\Effects\\monster\\HAUNTINGPLANT\\",	snCHAR_SOUND_HAUNTINGPLANT	},
	{ "wav\\Effects\\monster\\DARKKNIGHT\\"	,	snCHAR_SOUND_DARKKNIGHT		},
	{ "wav\\Effects\\monster\\Guardian-Saint\\",snCHAR_SOUND_GUARDIAN_SAINT	},



	//////////////////// 아이언 추가 몬스터 /////////////////////////////////
	{ "wav\\Effects\\monster\\ChainGolem\\"	,	snCHAR_SOUND_CHAINGOLEM		},
	{ "wav\\Effects\\monster\\DeadZone\\"	,	snCHAR_SOUND_DEADZONE		},
	{ "wav\\Effects\\monster\\Grotesque\\"	,	snCHAR_SOUND_GROTESQUE		},
	{ "wav\\Effects\\monster\\Hypermachine\\",	snCHAR_SOUND_HYPERMACHINE	},
	{ "wav\\Effects\\monster\\IronFist\\"	,	snCHAR_SOUND_IRONFIST		},
	{ "wav\\Effects\\monster\\Morgon\\"		,	snCHAR_SOUND_MORGON			},
	{ "wav\\Effects\\monster\\Mountain\\"	,	snCHAR_SOUND_MOUNTAIN		},
	{ "wav\\Effects\\monster\\Rampage\\"	,	snCHAR_SOUND_RAMPAGE		},
	{ "wav\\Effects\\monster\\RunicGuardian\\",	snCHAR_SOUND_RUNICGUARDIAN	},
	{ "wav\\Effects\\monster\\Sadness\\"	,	snCHAR_SOUND_SADNESS		},
	{ "wav\\Effects\\monster\\TowerGolem\\"	,	snCHAR_SOUND_TOWERGOLEM		},
	{ "wav\\Effects\\monster\\VampiricBee\\",	snCHAR_SOUND_VAMPRICBEE		},
	{ "wav\\Effects\\monster\\VampiricMachine\\",snCHAR_SOUND_VAMPRICMACHINE},
	{ "wav\\Effects\\monster\\Omu\\",			snCHAR_SOUND_OMU			},

	{ "wav\\Effects\\monster\\AvelinArcher\\",	snCHAR_SOUND_AVELIN_ARCHER	},
	{ "wav\\Effects\\monster\\AvelinQueen\\",	snCHAR_SOUND_AVELIN_QUEEN	},
	{ "wav\\Effects\\monster\\Babel\\",			snCHAR_SOUND_BABEL	},

	////////////////////////// 아이스 몬스터 추가 //////////////////////
	{ "wav\\Effects\\monster\\Mystic\\",		snCHAR_SOUND_MYSTIC			},
	{ "wav\\Effects\\monster\\HEAVYGOBLIN\\",	snCHAR_SOUND_ICEGOBLIN		},
	{ "wav\\Effects\\monster\\ColdEye\\",		snCHAR_SOUND_COLDEYE		},
	{ "wav\\Effects\\monster\\Frozen\\",		snCHAR_SOUND_FROZEN			},
	{ "wav\\Effects\\monster\\STONEGOLEM\\",	snCHAR_SOUND_ICEGOLEM		},
	{ "wav\\Effects\\monster\\Frost\\",			snCHAR_SOUND_FROST			},
	{ "wav\\Effects\\monster\\ChaosCara\\",		snCHAR_SOUND_CHAOSCARA		},
	{ "wav\\Effects\\monster\\DeathKnight\\",	snCHAR_SOUND_DEATHKNIGHT	},

	////////////////////////// 그리디호수 몬스터 추가 //////////////////////
	{ "wav\\Effects\\monster\\GreateGreven\\",	snCHAR_SOUND_GREATE_GREVEN	},
	{ "wav\\Effects\\monster\\LizardFolk\\",	snCHAR_SOUND_LIZARDFOLK		},
	{ "wav\\Effects\\monster\\M_Lord\\",		snCHAR_SOUND_M_LORD			},
	{ "wav\\Effects\\monster\\Spider\\",		snCHAR_SOUND_SPIDER			},
	{ "wav\\Effects\\monster\\Stingray\\",		snCHAR_SOUND_STINGRAY		},
	{ "wav\\Effects\\monster\\Strider\\",		snCHAR_SOUND_STRIDER		},

	////////////////////////// 아이스2 몬스터 추가 //////////////////////
	{ "wav\\Effects\\monster\\TurtleCannon\\",	snCHAR_SOUND_TURTLE_CANNON	},
	{ "wav\\Effects\\monster\\DevilBird\\",		snCHAR_SOUND_DEVIL_BIRD		},
	{ "wav\\Effects\\monster\\BlizzardGiant\\",	snCHAR_SOUND_BLIZZARD_GIANT	},
	{ "wav\\Effects\\monster\\Kelvezu\\",		snCHAR_SOUND_KELVEZU		},

	////////////////////////// 신규필드 추가 //////////////////////
	{ "wav\\Effects\\monster\\DarkPhalanx\\",	snCHAR_SOUND_DARKPHALANX	},
	{ "wav\\Effects\\monster\\BloodyKnight\\",	snCHAR_SOUND_BLOODYKNIGHT	},
	{ "wav\\Effects\\monster\\Chimera\\",		snCHAR_SOUND_CHIMERA		},
	{ "wav\\Effects\\monster\\FireWorm\\",		snCHAR_SOUND_FIREWORM		},
	{ "wav\\Effects\\monster\\HellHound\\",		snCHAR_SOUND_HELLHOUND		},
	{ "wav\\Effects\\monster\\DarkGuard\\",		snCHAR_SOUND_DARKGUARD		},
	{ "wav\\Effects\\monster\\DarkMage\\",		snCHAR_SOUND_DARKMAGE		},

	{ "wav\\Effects\\monster\\MonMokova\\",		snCHAR_SOUND_MOKOVA			},
	{ "wav\\Effects\\monster\\MonTempleguard\\",snCHAR_SOUND_TEMPLEGUARD	},
	{ "wav\\Effects\\monster\\MonSeto\\",		snCHAR_SOUND_SETO			},
	{ "wav\\Effects\\monster\\MonKingSpider\\",	snCHAR_SOUND_KINGSPIDER		},

	////////////////////////// pluto 신규필드 몬스터 추가 //////////////////////
	{ "wav\\Effects\\monster\\d_kn\\",			snCHAR_SOUND_REVIVED_KNIGHT	}, //되살아난 나이트
	{ "wav\\Effects\\monster\\d_magi\\",		snCHAR_SOUND_REVIVED_MAGICIAN }, //되살아난 매지션
	{ "wav\\Effects\\monster\\d_atal\\",		snCHAR_SOUND_REVIVED_ATALANTA }, // 되살아난 아탈란타
	{ "wav\\Effects\\monster\\d_fi\\",			snCHAR_SOUND_REVIVED_FIGTHER }, // 되살아난 파이터
	{ "wav\\Effects\\monster\\d_ar\\",			snCHAR_SOUND_REVIVED_ARCHER	},	// 되살아난 아쳐
	{ "wav\\Effects\\monster\\d_meca\\",		snCHAR_SOUND_REVIVED_MECANICIAN }, // 되살아난 메카니션
	{ "wav\\Effects\\monster\\d_pa\\",			snCHAR_SOUND_REVIVED_PIKEMAN }, // 되살아난 파이크맨
	{ "wav\\Effects\\monster\\d_pr\\",			snCHAR_SOUND_REVIVED_PRIESTESS }, // 되살아난 프리스티스
	{ "wav\\Effects\\monster\\deadhopt\\",		snCHAR_SOUND_DEADHOPT }, // 데드호피
	{ "wav\\Effects\\monster\\deadkinghopy\\",	snCHAR_SOUND_DEADKINGHOPY }, // 데드킹호피
	{ "wav\\Effects\\monster\\gorgon\\",		snCHAR_SOUND_GORGON},	// 고르곤
	{ "wav\\Effects\\monster\\hobogolem\\",		snCHAR_SOUND_HOBOGOLEM}, // 호보고렘
	
	// 박재원 - 저주받은 신전 3층(신규필드 몬스터)
	{ "wav\\Effects\\monster\\Niken\\",			snCHAR_SOUND_NIKEN			}, // 니켄
	{ "wav\\Effects\\monster\\Mimic\\",			snCHAR_SOUND_MIMIC			}, // 미믹
	{ "wav\\Effects\\monster\\KingBat\\",		snCHAR_SOUND_KINGBAT		}, // 킹뱃
	{ "wav\\Effects\\monster\\GoblinShaman\\",	snCHAR_SOUND_GOBLINSHAMAN	}, // 고블린샤먼
	{ "wav\\Effects\\monster\\Hest\\",			snCHAR_SOUND_HEST			}, // 헤스트

	// 박재원 - 끝없는 탑 3층(신규필드 몬스터)
	{ "wav\\Effects\\monster\\Ruca\\",			snCHAR_SOUND_RUCA			}, // 루카
	{ "wav\\Effects\\monster\\NazSenior\\",		snCHAR_SOUND_NAZSENIOR		}, // 나즈 시니어
	{ "wav\\Effects\\monster\\Igolation\\",		snCHAR_SOUND_IGOLATION		}, // 이골라티온
	{ "wav\\Effects\\monster\\Kakoa\\",			snCHAR_SOUND_KAKOA			}, // 카코아
	{ "wav\\Effects\\monster\\Sprin\\",			snCHAR_SOUND_SPRIN			}, // 스프린
	{ "wav\\Effects\\monster\\UndeadMaple\\",	snCHAR_SOUND_UNDEADMAPLE	}, // 언데드 메이플
	{ "wav\\Effects\\monster\\Xetan\\",			snCHAR_SOUND_XETAN			}, // 제탄


	// 박재원 - 복날 이벤트 몬스터 추가
	{ "wav\\Effects\\monster\\Bebechick\\",			snCHAR_SOUND_BEBECHICK		}, // 아기 꼬꼬
	{ "wav\\Effects\\monster\\PaPachick\\",			snCHAR_SOUND_PAPACHICK		}, // 아빠 꼬꼬

	{ "wav\\Effects\\monster\\LordHopy\\",			snCHAR_SOUND_LORDHOPY		}, // Lord Hopy

	//////////////////////// NPC //////////////////////////////////
	{ "wav\\Effects\\npc\\morif\\",				snCHAR_SOUND_NPC_MORIF	},
	{ "wav\\Effects\\npc\\mollywolf\\",			snCHAR_SOUND_NPC_MOLLYWOLF	},
	{ "wav\\Effects\\npc\\SkillMaster\\",		snCHAR_SOUND_NPC_SKILLMASTER	},
	{ "wav\\Effects\\npc\\MagicMaster\\",		snCHAR_SOUND_NPC_MAGICMASTER	},
	{ "wav\\Effects\\npc\\worldcup\\",			snCHAR_SOUND_WORLDCUP		},

	///////////////////////// 공성전 //////////////////////////////
	{ "wav\\Effects\\npc\\castledoor\\",	snCHAR_SOUND_CASTLE_DOOR		},
	{ "wav\\Effects\\npc\\crystal_r\\",		snCHAR_SOUND_CASTLE_CRYSTAL_R	},
	{ "wav\\Effects\\npc\\crystal_g\\",		snCHAR_SOUND_CASTLE_CRYSTAL_G	},
	{ "wav\\Effects\\npc\\crystal_b\\",		snCHAR_SOUND_CASTLE_CRYSTAL_B	},
	{ "wav\\Effects\\npc\\crystal_n\\",		snCHAR_SOUND_CASTLE_CRYSTAL_N	},
	{ "wav\\Effects\\npc\\tower-b\\",		snCHAR_SOUND_CASTLE_TOWER_B		},

	{ "wav\\Effects\\npc\\Soldier_A\\",		snCHAR_SOUND_CASTLE_SOLDER_A	},
	{ "wav\\Effects\\npc\\Soldier_B\\",		snCHAR_SOUND_CASTLE_SOLDER_B	},
	{ "wav\\Effects\\npc\\Soldier_C\\",		snCHAR_SOUND_CASTLE_SOLDER_C	},

	/////////////////////// 4차스킬 소환몹 /////////////////////////
/*
	{ "wav\\Effects\\monster\\wolverin\\"		,	snCHAR_SOUND_S_WOLVERLIN	},
	{ "wav\\Effects\\monster\\MetalGolem\\"		,	snCHAR_SOUND_S_METALGOLEM	},
	{ "wav\\Effects\\monster\\FireElemental\\"	,	snCHAR_SOUND_S_F_ELEMENTAL	},
*/
	{ "wav\\Effects\\monster\\wolverin\\"		,	snCHAR_SOUND_S_WOLVERLIN	},
	{ "wav\\Effects\\monster\\MetalGolem\\"		,	snCHAR_SOUND_S_METALGOLEM	},
	{ "wav\\Effects\\monster\\Figon\\"			,	snCHAR_SOUND_S_F_ELEMENTAL	},


	//////////////////////// 템스트론 //////////////////////////////////
	{ "wav\\Effects\\player\\fighter\\"		,	snCHAR_SOUND_FIGHTER	},
	{ "wav\\Effects\\player\\Mechanician\\"	,	snCHAR_SOUND_MECHANICAN	},
	{ "wav\\Effects\\player\\archer\\"		,	snCHAR_SOUND_ARCHER		},
	{ "wav\\Effects\\player\\pikeman\\"		,	snCHAR_SOUND_PIKEMAN	},

	//////////////////////// 모라이온 //////////////////////////////////
	{ "wav\\Effects\\player\\Knight\\"		,	snCHAR_SOUND_KNIGHT		},
	{ "wav\\Effects\\player\\Atalanta\\"	,	snCHAR_SOUND_ATALANTA	},
	{ "wav\\Effects\\player\\Priestess\\"	,	snCHAR_SOUND_PRIESTESS	},
	{ "wav\\Effects\\player\\Magician\\"	,	snCHAR_SOUND_MAGICIAN	}, 

	// 장별 - 수박몬스터
	{ "wav\\Effects\\monster\\Watermelon\\",	snCHAR_SOUND_WATERMELON	}

};


//#define	snCHAR_SOUND_SKELETONBOW	0x11C0
//snCHAR_SOUND_ILLUSIONKNIGHT	illuisionknight

/*
#define	snCHAR_SOUND_HEADCUTTER		0x1100
#define	snCHAR_SOUND_SANDLEM		0x1110
#define	snCHAR_SOUND_WEB			0x1120
#define	snCHAR_SOUND_HOPYKING		0x1130
#define	snCHAR_SOUND_CRIP			0x1140
#define	snCHAR_SOUND_BUMA			0x1150
#define	snCHAR_SOUND_DECOY			0x1160
#define	snCHAR_SOUND_DORAL			0x1170
#define	snCHAR_SOUND_FIGON			0x1180
#define	snCHAR_SOUND_GOLEM			0x1190
#define	snCHAR_SOUND_GREVEN			0x11A0


*/


char *szSndWeaponPath = "wav\\Effects\\weapon\\";

#define	snFIND_EFFECT_MOTION_MAX	9
#define	snFIND_EFFECT_WEAPON_MAX	18

snFIND_EFFECT	snFindEffectsMotion[snFIND_EFFECT_MOTION_MAX] = 
{
	{	"attack"		,		CHRMOTION_STATE_ATTACK	},
	{	"damage"		,		CHRMOTION_STATE_DAMAGE	},
	{	"dead"			,		CHRMOTION_STATE_DEAD	},
	{	"neutral"		,		CHRMOTION_STATE_STAND	},
	{	"move"			,		CHRMOTION_STATE_WALK	},
	{	"disappear"		,		CHRMOTION_STATE_WARP	},
	{	"change"		,		CHRMOTION_STATE_WARP	},
	{	"skill"			,		CHRMOTION_STATE_SKILL	},
	{	"hammer"		,		CHRMOTION_STATE_HAMMER	}
};

snFIND_EFFECT	snFindEffectsWeapon[snFIND_EFFECT_WEAPON_MAX] = 
{
	{	"one hand swing axe"		,		1	},
	{	"two hand swing axe"		,		2	},
	{	"one hand swing sword"		,		3	},
	{	"two hand swing sword"		,		4	},
	{	"one hand swing spear"		,		5	},
	{	"two hand swing spear"		,		6	},

	{	"one hand scratch"			,		7	},
	{	"bow shoot"					,		8	},
	{	"one hand swing blunt"		,		9	},
	{	"two hand swing blunt"		,		10	},
	{	"throwing"					,		11	},
	{	"one hand miss"				,		12	},
	{	"two hand miss"				,		13	},
	{	"punch hit"					,		14	},
	{	"one hand small swing"		,		15	},
	{	"critical hit"				,		16	},

	{	"one hand casting"			,		17	},
	{	"two hand casting"			,		18	}
};



//문자 비교
int	CompareHeadString( char *szCodeString , char *szString )
{
	int cnt;
	int	len;
	char ch;

	len = lstrlen( szCodeString );

	for( cnt=0;cnt<len;cnt++ ) {
		ch = szString[cnt];
		if ( ch==0 ) return FALSE;
		if ( szCodeString[cnt]!=ch ) {
			if ( ch<'A' && ch>'Z' )	return FALSE;
			if ( szCodeString[cnt]!=(ch+0x20) ) 
				return FALSE;
		}
	}

	return TRUE;
}

DWORD FindMotionState( char *szFile )
{
	int cnt;

	for( cnt=0;cnt<snFIND_EFFECT_MOTION_MAX;cnt++) {
		if ( CompareHeadString( snFindEffectsMotion[cnt].lpStringCode , szFile )==TRUE ) {
			return snFindEffectsMotion[cnt].CharSoundCode;
		}
	}

	return NULL;
}

DWORD FindWeaponState( char *szFile )
{
	int cnt;

	for( cnt=0;cnt<snFIND_EFFECT_WEAPON_MAX;cnt++) {
		if ( CompareHeadString( snFindEffectsWeapon[cnt].lpStringCode , szFile )==TRUE ) {
			return snFindEffectsWeapon[cnt].CharSoundCode;
		}
	}

	return NULL;
}


#define snEFFECT_MAX	1300

int	LoadSoundNum;

snEFFECT	snEffect[snEFFECT_MAX];
int	snEffectCnt;

snEFFECT	snEffectWeapon[snEFFECT_MAX];
int	snEffectWeaponCnt;



int sinSoundNum = 0;
extern char *sinSoundWav[];


char *esSoundWav[] = {
	"wav\\Effects\\FootSteps\\step01.wav",
	"wav\\Effects\\FootSteps\\step02.wav",
	"wav\\Effects\\FootSteps\\stonerun1.wav",
	"wav\\Effects\\FootSteps\\stonerun2.wav",
	"wav\\Effects\\FootSteps\\stonerun3.wav",
	"wav\\Ambient\\stone-mill.wav",						//5 지역 효과음
	"wav\\Effects\\event\\fieldstart.wav",				//6
	"wav\\Effects\\event\\level UP.wav",				//7
	"wav\\Effects\\event\\hosound.wav",					//8
	"wav\\Ambient\\tempskron-town.wav",					//AMBIENT_SOUND_NUM 배경 효과음 9

	"wav\\Effects\\monster\\Figon\\impact 1.wav",		//10	파이곤 불
	"wav\\Effects\\FootSteps\\stone-dun1.wav",			//11	던전 발소리
	"wav\\Effects\\FootSteps\\stone-dun2.wav",			//12	던전 발소리
	"wav\\Effects\\FootSteps\\stone-dun3.wav",			//13	던전 발소리

	"wav\\Effects\\FootSteps\\inwater 1.wav",			//14	물 발소리
	"wav\\Effects\\FootSteps\\inwater 2.wav",			//15	물 발소리
	"wav\\Effects\\FootSteps\\inwater 3.wav",			//16	물 발소리

	"wav\\Effects\\FootSteps\\inwater 3.wav",			//17	CONTINUE_SOUND_NUM 연결 사운드	
	"wav\\Sod\\turn.wav",								//18

	"wav\\Sod\\SOD-item.wav",							//19	SOD아이템 사용

	"wav\\Effects\\Menu\\Event\\meteo 1.wav",			//20	유성 떨어짐
	"wav\\Effects\\Menu\\Event\\meteo 2.wav",			//21	유성 떨어짐
	"wav\\Effects\\Event\\sheltom_b.wav",				//22	쉘텀깨지는 소리
	"wav\\Effects\\Event\\firecracker01.wav",			//23	폭죽소리1
	"wav\\Effects\\Event\\firecracker02.wav",			//24	폭죽소리1
	"wav\\Effects\\Event\\firecracker03.wav",			//25	폭죽소리1

	0
};


int esSoundNum = 0;


#define	AMBIENT_SOUND_NUM		9
#define	AMBIENT_SOUND_NUM2		5
#define	CONTINUE_SOUND_NUM		17

int	AmbientSound = -1;
int	AmbientSound2 = -1;
int	AmbientSoundNum = 0;
int	AmbientSoundNum2 = 0;
int	ContinueSoundNum = 0;
int	AmbientObjectSound = 0;

int	ContSoundCount = 0;

//배경 효과음
char *szAmbientSound[] = {
	"wav\\Ambient\\tempskron-town.wav",
	"wav\\Ambient\\forest-day.wav",
	"wav\\Ambient\\forest-night.wav",
	"wav\\Ambient\\ruin wind.wav",
	"wav\\Ambient\\desert wind.wav",
	"wav\\Ambient\\dungeon.wav",

	"wav\\Ambient\\temple_amb.wav",
	"wav\\Ambient\\cave_amb.wav",
	"wav\\Ambient\\darksanc_amb.wav",

	"wav\\Ambient\\iron_amb.wav",
	"wav\\Ambient\\iron_amb2.wav",
	"wav\\Ambient\\Babel Approach.wav",
	"wav\\Ambient\\icewind.wav",

	0
};

struct	sSKILL_SOUND_FILE {
	char	*szFile;
	DWORD	dwCode;
};

int	SkillSoundNum = 0;

sSKILL_SOUND_FILE	SkillSoundWav[] = {
	{	"wav\\Effects\\Skill\\Learn_Skill.wav",							SKILL_SOUND_LEARN	},	

	{	"wav\\Effects\\Skill\\Tempskron\\Extreme_Shield 01.wav",		SKILL_SOUND_EXTREME_SHIELD	},	
	{	"wav\\Effects\\Skill\\Tempskron\\Jumping_Crash 01.wav",			SKILL_SOUND_JUMPING_CRASH1	},	
	{	"wav\\Effects\\Skill\\Tempskron\\Jumping_Crash 02.wav",			SKILL_SOUND_JUMPING_CRASH2	},	
	{	"wav\\Effects\\Skill\\Tempskron\\Mechanic_Bomb 01.wav",			SKILL_SOUND_MECHANIC_BOMB1	},	
	{	"wav\\Effects\\Skill\\Tempskron\\Mechanic_Bomb 02.wav",			SKILL_SOUND_MECHANIC_BOMB2	},	
	{	"wav\\Effects\\Skill\\Tempskron\\Perfect Aim 01.wav",			SKILL_SOUND_PERFECT_AIM	},	
	{	"wav\\Effects\\Skill\\Tempskron\\Physical_Absorption 01.wav",	SKILL_SOUND_PHYSICAL_ABSORB1	},	
	{	"wav\\Effects\\Skill\\Tempskron\\Physical_Absorption 02.wav",	SKILL_SOUND_PHYSICAL_ABSORB2	},	
	{	"wav\\Effects\\Skill\\Tempskron\\Physical_Absorption 03.wav",	SKILL_SOUND_PHYSICAL_ABSORB3	},

	{	"wav\\Effects\\Skill\\Tempskron\\Pike_Wind 01.wav",				SKILL_SOUND_PIKE_WIND	},	
	{	"wav\\Effects\\Skill\\Tempskron\\Scout_Hawk 01.wav",			SKILL_SOUND_SCOUT_HAWK1	},	
	{	"wav\\Effects\\Skill\\Tempskron\\Scout_Hawk 02.wav",			SKILL_SOUND_SCOUT_HAWK2	},	
	{	"wav\\Effects\\Skill\\Tempskron\\Wind_Arrow 01.wav",			SKILL_SOUND_WIND_ARROW	},	

	{	"wav\\Effects\\Skill\\bow_skill_hit 01.wav",					SKILL_SOUND_ARROW_HIT1 },
	{	"wav\\Effects\\Skill\\bow_skill_hit 02.wav",					SKILL_SOUND_ARROW_HIT2 },

	{	"wav\\Effects\\Skill\\Skill_Swing 01.wav",						SKILL_SOUND_SKILL_SWING1 },	
	{	"wav\\Effects\\Skill\\Skill_Swing 02.wav",						SKILL_SOUND_SKILL_SWING2 },	
	
	{	"wav\\Effects\\Skill\\Tempskron\\Automation 01.wav",			SKILL_SOUND_SKILL_AUTOMATION1 },	
	{	"wav\\Effects\\Skill\\Tempskron\\Automation 02.wav",			SKILL_SOUND_SKILL_AUTOMATION2 },	
	{	"wav\\Effects\\Skill\\Tempskron\\Automation 03.wav",			SKILL_SOUND_SKILL_AUTOMATION3 },	

	{	"wav\\Effects\\Skill\\Tempskron\\Maximize 01.wav",				SKILL_SOUND_SKILL_MAXIMIZE1 },	
	{	"wav\\Effects\\Skill\\Tempskron\\Maximize 01.wav",				SKILL_SOUND_SKILL_MAXIMIZE2 },	
	{	"wav\\Effects\\Skill\\Tempskron\\Maximize 01.wav",				SKILL_SOUND_SKILL_MAXIMIZE3 },	

	{	"wav\\Effects\\Skill\\Tempskron\\Spark 01.wav",					SKILL_SOUND_SKILL_SPARK1 },	
	{	"wav\\Effects\\Skill\\Tempskron\\Spark 02.wav",					SKILL_SOUND_SKILL_SPARK2 },	
	{	"wav\\Effects\\Skill\\Tempskron\\Spark 03.wav",					SKILL_SOUND_SKILL_SPARK3 },	

	{	"wav\\Effects\\Skill\\Tempskron\\Ground Pike 01.wav",			SKILL_SOUND_SKILL_GROUND_PIKE },	
	{	"wav\\Effects\\Skill\\Tempskron\\Tornado 01.wav",				SKILL_SOUND_SKILL_TORNADO1 },	

	{	"wav\\Effects\\Skill\\Tempskron\\Falcon 02.wav",				SKILL_SOUND_SKILL_FALCON },	
	{	"wav\\Effects\\Skill\\Tempskron\\Falcon Attack 01.wav",			SKILL_SOUND_SKILL_FALCON_ATTACK1 },	
	{	"wav\\Effects\\Skill\\Tempskron\\Falcon Attack 02.wav",			SKILL_SOUND_SKILL_FALCON_ATTACK2 },	

	{	"wav\\Effects\\Skill\\Tempskron\\Avalanche 01.wav",				SKILL_SOUND_SKILL_AVALANCHE1 },	
	{	"wav\\Effects\\Skill\\Tempskron\\Avalanche 02.wav",				SKILL_SOUND_SKILL_AVALANCHE2 },	
	{	"wav\\Effects\\Skill\\Tempskron\\Avalanche 03.wav",				SKILL_SOUND_SKILL_AVALANCHE3 },	

	{	"wav\\Effects\\Skill\\Tempskron\\Expansion 01.wav",				SKILL_SOUND_SKILL_EXPANSION1 },	
	{	"wav\\Effects\\Skill\\Tempskron\\Expansion 02.wav",				SKILL_SOUND_SKILL_EXPANSION2 },	

	{	"wav\\Effects\\Skill\\Tempskron\\RoZ 01.wav",					SKILL_SOUND_SKILL_RAGE_OF_ZECRAM1 },	
	{	"wav\\Effects\\Skill\\Tempskron\\RoZ 02.wav",					SKILL_SOUND_SKILL_RAGE_OF_ZECRAM2 },	
	{	"wav\\Effects\\Skill\\Tempskron\\RoZ 04.wav",					SKILL_SOUND_SKILL_RAGE_OF_ZECRAM3 },	
	//{	"wav\\Effects\\Skill\\Tempskron\\RoZ 04.wav",					SKILL_SOUND_SKILL_RAGE_OF_ZECRAM4 },	

	{	"wav\\Effects\\Skill\\Tempskron\\Roar 01.wav",					SKILL_SOUND_SKILL_ROAR1 },	
	{	"wav\\Effects\\Skill\\Tempskron\\Roar 02.wav",					SKILL_SOUND_SKILL_ROAR2 },	

	{	"wav\\Effects\\Skill\\Tempskron\\AofR 01.wav",					SKILL_SOUND_SKILL_ARROW_OF_RANGE },	

	///////////////////////////////// 모라이온 ////////////////////////////////
	{	"wav\\Effects\\Skill\\Morion\\Casting_M.wav",					SKILL_SOUND_SKILL_CASTING_MAGICIAN },	
	{	"wav\\Effects\\Skill\\Morion\\Casting_P.wav",					SKILL_SOUND_SKILL_CASTING_PRIEST },	

	{	"wav\\Effects\\Skill\\Morion\\Agony 1.wav",						SKILL_SOUND_SKILL_AGONY },	
	{	"wav\\Effects\\Skill\\Morion\\Farina 1.wav",					SKILL_SOUND_SKILL_FARINA },	
	{	"wav\\Effects\\Skill\\Morion\\FireBall 1.wav",					SKILL_SOUND_SKILL_FIREBALL1 },	
	{	"wav\\Effects\\Skill\\Morion\\FireBall 2.wav",					SKILL_SOUND_SKILL_FIREBALL2 },	
	{	"wav\\Effects\\Skill\\Morion\\FireBolt 1.wav",					SKILL_SOUND_SKILL_FIREBOLT1 },	
	{	"wav\\Effects\\Skill\\Morion\\FireBolt 2.wav",					SKILL_SOUND_SKILL_FIREBOLT2 },	
	
	{	"wav\\Effects\\Skill\\Morion\\Healing 1.wav",					SKILL_SOUND_SKILL_HEALING },	
	{	"wav\\Effects\\Skill\\Morion\\HolyBody 1.wav",					SKILL_SOUND_SKILL_HOLYBODY },	
	{	"wav\\Effects\\Skill\\Morion\\HolyBolt 1.wav",					SKILL_SOUND_SKILL_HOLYBOLT },	
	{	"wav\\Effects\\Skill\\Morion\\HolyMind 1.wav",					SKILL_SOUND_SKILL_HOLYMIND },	
	{	"wav\\Effects\\Skill\\Morion\\MultiSpark 1.wav",				SKILL_SOUND_SKILL_MULTISPARK },	
	{	"wav\\Effects\\Skill\\Morion\\ShieldSpake 1.wav",				SKILL_SOUND_SKILL_SHIELDSTRIKE },	
	{	"wav\\Effects\\Skill\\Morion\\SwordBlast 1.wav",				SKILL_SOUND_SKILL_SWORDBLAST1 },	
	{	"wav\\Effects\\Skill\\Morion\\SwordBlast 2.wav",				SKILL_SOUND_SKILL_SWORDBLAST2 },	
	{	"wav\\Effects\\Skill\\Morion\\VigorSpear 1.wav",				SKILL_SOUND_SKILL_VIGORSPEAR },	
	{	"wav\\Effects\\Skill\\Morion\\Zenith 1.wav",					SKILL_SOUND_SKILL_ZENITH },	


	//시전
	{	"wav\\Effects\\Skill\\Morion\\HolyValor 1.wav",					SKILL_SOUND_SKILL_HOLY_VALOR1 },	
	//유지 (반복연주)
	{	"wav\\Effects\\Skill\\Morion\\HolyValor 3.wav",					SKILL_SOUND_SKILL_HOLY_VALOR3 },

	//떴을때
	{	"wav\\Effects\\Skill\\Morion\\Brandish 1.wav",					SKILL_SOUND_SKILL_BLANDISH1 },
	//타격
	{	"wav\\Effects\\Skill\\Morion\\Brandish 2.wav",					SKILL_SOUND_SKILL_BLANDISH2 },

	//피어싱
	{	"wav\\Effects\\Skill\\Morion\\Piercing 2.wav",					SKILL_SOUND_SKILL_PIERCING },

	//시전
	{	"wav\\Effects\\Skill\\Morion\\DrasticSpirit 1.wav",				SKILL_SOUND_SKILL_DRASTICSPIRIT1 },
	//유지(반복)
	{	"wav\\Effects\\Skill\\Morion\\DrasticSpirit 3.wav",				SKILL_SOUND_SKILL_DRASTICSPIRIT3 },



	//시전
	{	"wav\\Effects\\Skill\\Morion\\Windy 1.wav",						SKILL_SOUND_SKILL_WINDY1 },
	//유지(반복)
	{	"wav\\Effects\\Skill\\Morion\\Windy 3.wav",						SKILL_SOUND_SKILL_WINDY3 },


	{	"wav\\Effects\\Skill\\Morion\\TwistJavelin.wav",				SKILL_SOUND_SKILL_TWISTJAVELIN },

	//영혼빠지기
	{	"wav\\Effects\\Skill\\Morion\\SoulSucker 2.wav",				SKILL_SOUND_SKILL_SOULSUCKER2 },
	//흡수
	{	"wav\\Effects\\Skill\\Morion\\SoulSucker 3.wav",				SKILL_SOUND_SKILL_SOULSUCKER3 },
	//생명력
	{	"wav\\Effects\\Skill\\Morion\\SoulSucker 4.wav",				SKILL_SOUND_SKILL_SOULSUCKER4 },

	{	"wav\\Effects\\Skill\\Morion\\FireJavelin 1.wav",				SKILL_SOUND_SKILL_FIREJAVELIN },


	{	"wav\\Effects\\Skill\\Morion\\DivineLightning 1.wav",			SKILL_SOUND_SKILL_DIVINELIGHTNING1 },
	{	"wav\\Effects\\Skill\\Morion\\DivineLightning 2.wav",			SKILL_SOUND_SKILL_DIVINELIGHTNING2 },
	{	"wav\\Effects\\Skill\\Morion\\DivineLightning 3.wav",			SKILL_SOUND_SKILL_DIVINELIGHTNING3 },

	//시전
	{	"wav\\Effects\\Skill\\Morion\\HolyReflection 1.wav",			SKILL_SOUND_SKILL_HOLYREFLECTION1 },
	//유지(반복)
	{	"wav\\Effects\\Skill\\Morion\\HolyReflection 2.wav",			SKILL_SOUND_SKILL_HOLYREFLECTION2 },

	{	"wav\\Effects\\Skill\\Morion\\GrandHealing 1.wav",				SKILL_SOUND_SKILL_GRANDHEALING },


	{	"wav\\Effects\\Skill\\Morion\\Watornado 1.wav",					SKILL_SOUND_SKILL_WATORNADO1 },
	{	"wav\\Effects\\Skill\\Morion\\Watornado 2.wav",					SKILL_SOUND_SKILL_WATORNADO2 },


	//얼음 타격
	{	"wav\\Effects\\Skill\\Morion\\EnchantWpn_add_C 1.wav",			SKILL_SOUND_SKILL_ENCHANTWEAPON_ICE1  },
	{	"wav\\Effects\\Skill\\Morion\\EnchantWpn_add_C 2.wav",			SKILL_SOUND_SKILL_ENCHANTWEAPON_ICE2 },
	//불 타격
	{	"wav\\Effects\\Skill\\Morion\\EnchantWpn_add_F 1.wav",			SKILL_SOUND_SKILL_ENCHANTWEAPON_FIRE1  },
	{	"wav\\Effects\\Skill\\Morion\\EnchantWpn_add_F 2.wav",			SKILL_SOUND_SKILL_ENCHANTWEAPON_FIRE2  },
	//전기 타격
	{	"wav\\Effects\\Skill\\Morion\\EnchantWpn_add_L 1.wav",			SKILL_SOUND_SKILL_ENCHANTWEAPON_LIGHTNING1  },
	{	"wav\\Effects\\Skill\\Morion\\EnchantWpn_add_L 2.wav",			SKILL_SOUND_SKILL_ENCHANTWEAPON_LIGHTNING2  },
	//시전
	{	"wav\\Effects\\Skill\\Morion\\EnchantWpn_cast.wav",				SKILL_SOUND_SKILL_ENCHANTWEAPON_CAST  },

	//마법으로 번개
	{	"wav\\Effects\\Skill\\Morion\\EnchantWpn_magic_L 1.wav",		SKILL_SOUND_SKILL_ENCHANTWEAPON_MAGIC_LIGHTNING1  },
	{	"wav\\Effects\\Skill\\Morion\\EnchantWpn_magic_L 2.wav",		SKILL_SOUND_SKILL_ENCHANTWEAPON_MAGIC_LIGHTNING2 },

	//마법으로 얼음
	{	"wav\\Effects\\Skill\\Morion\\EnchantWpn_magic_C 1.wav",		SKILL_SOUND_SKILL_ENCHANTWEAPON_MAGIC_ICE1  },
	{	"wav\\Effects\\Skill\\Morion\\EnchantWpn_magic_C 2.wav",		SKILL_SOUND_SKILL_ENCHANTWEAPON_MAGIC_ICE2  },

	
	{	"wav\\Effects\\Skill\\Morion\\DeathRay 1.wav",					SKILL_SOUND_SKILL_DEATHRAY },


	///////////////////////////////////// 3차 스킬 /////////////////////////////////

	{	"wav\\Effects\\Skill\\Tempskron\\AvengingCrash 01.wav",			SKILL_SOUND_SKILL_AVANGING_CRASH	},
	{	"wav\\Effects\\Skill\\Tempskron\\ElementalShot 01.wav",			SKILL_SOUND_SKILL_ELEMENTAL_SHOT	},
	{	"wav\\Effects\\Skill\\Tempskron\\BombShot 01.wav",				SKILL_SOUND_SKILL_BOMB_SHOT1		},
	{	"wav\\Effects\\Skill\\Tempskron\\BombShot 02.wav",				SKILL_SOUND_SKILL_BOMB_SHOT2		},
	{	"wav\\Effects\\Skill\\Tempskron\\BoneCrash 01.wav",				SKILL_SOUND_SKILL_BONE_CRASH1		},
	{	"wav\\Effects\\Skill\\Tempskron\\ChainLance 01.wav",			SKILL_SOUND_SKILL_CHAIN_LANCE1		},
	{	"wav\\Effects\\Skill\\Tempskron\\ChainLance 02.wav",			SKILL_SOUND_SKILL_CHAIN_LANCE2		},
	{	"wav\\Effects\\Skill\\Tempskron\\ChainLance 03.wav",			SKILL_SOUND_SKILL_CHAIN_LANCE3		},
	{	"wav\\Effects\\Skill\\Tempskron\\Concentration 01.wav",			SKILL_SOUND_SKILL_CONCENTRATION1	},
	{	"wav\\Effects\\Skill\\Tempskron\\Concentration 02.wav",			SKILL_SOUND_SKILL_CONCENTRATION2	},
	{	"wav\\Effects\\Skill\\Tempskron\\GoldFalcon 02.wav",			SKILL_SOUND_SKILL_GOLDEN_FALCON		},
	{	"wav\\Effects\\Skill\\Tempskron\\MetalArmor 01.wav",			SKILL_SOUND_SKILL_METAL_ARMOR		},
	{	"wav\\Effects\\Skill\\Tempskron\\Perforation 01.wav",			SKILL_SOUND_SKILL_PERFORATION		},
	{	"wav\\Effects\\Skill\\Tempskron\\SparkShield 02.wav",			SKILL_SOUND_SKILL_SPARK_SHIELD		},
	{	"wav\\Effects\\Skill\\Tempskron\\SwiftAxe 01.wav",				SKILL_SOUND_SKILL_SWIFT_AXE1		},
	{	"wav\\Effects\\Skill\\Tempskron\\SwiftAxe 02.wav",				SKILL_SOUND_SKILL_SWIFT_AXE2		},
	{	"wav\\Effects\\Skill\\Tempskron\\VenomSpear 01.wav",			SKILL_SOUND_SKILL_VENOM_SPEAR1		},
	{	"wav\\Effects\\Skill\\Tempskron\\VenomSpear 02.wav",			SKILL_SOUND_SKILL_VENOM_SPEAR2		},
	{	"wav\\Effects\\Skill\\Tempskron\\vanish 01.wav",				SKILL_SOUND_SKILL_VANISH1			},
	{	"wav\\Effects\\Skill\\Tempskron\\vanish 02.wav",				SKILL_SOUND_SKILL_VANISH2			},



	{	"wav\\Effects\\Skill\\Morion\\DancingSword 1.wav",				SKILL_SOUND_SKILL_DANCING_SWORD		},
	{	"wav\\Effects\\Skill\\Morion\\DancingSword_atk_1.wav",			SKILL_SOUND_SKILL_DANCING_SWORD_ATK1},
	{	"wav\\Effects\\Skill\\Morion\\DancingSword_atk_2.wav",			SKILL_SOUND_SKILL_DANCING_SWORD_ATK2},
	{	"wav\\Effects\\Skill\\Morion\\Diastrophism 2.wav",				SKILL_SOUND_SKILL_DIASTROPHISM		},
	{	"wav\\Effects\\Skill\\Morion\\DivineInhalation 1.wav",			SKILL_SOUND_SKILL_DIVINE_INHALATION	},
	{	"wav\\Effects\\Skill\\Morion\\EnergyShield 1.wav",				SKILL_SOUND_SKILL_ENERGY_SHIELD		},
	{	"wav\\Effects\\Skill\\Morion\\Extinct 1.wav",					SKILL_SOUND_SKILL_EXTINTION			},
	{	"wav\\Effects\\Skill\\Morion\\FlameBrandish 2.wav",				SKILL_SOUND_SKILL_FLAME_BRANDISH	},
	{	"wav\\Effects\\Skill\\Morion\\GrandCross 2.wav",				SKILL_SOUND_SKILL_GRAND_CROSS		},
	{	"wav\\Effects\\Skill\\Morion\\Holy Incantation 1.wav",			SKILL_SOUND_SKILL_HOLY_INCANTATION	},
	{	"wav\\Effects\\Skill\\Morion\\LightningJavelin 1.wav",			SKILL_SOUND_SKILL_LIGHTNING_JAVELIN1},
	{	"wav\\Effects\\Skill\\Morion\\LightningJavelin 2.wav",			SKILL_SOUND_SKILL_LIGHTNING_JAVELIN2},
	{	"wav\\Effects\\Skill\\Morion\\Resurrection 2.wav",				SKILL_SOUND_SKILL_RESURRECTION		},
	{	"wav\\Effects\\Skill\\Morion\\SpiritElemental 1.wav",			SKILL_SOUND_SKILL_SPIRIT_ELEMENTAL	},
	{	"wav\\Effects\\Skill\\Morion\\SplitJavelin 1.wav",				SKILL_SOUND_SKILL_SPLIT_JAVELIN1	},
	{	"wav\\Effects\\Skill\\Morion\\SplitJavelin 2.wav",				SKILL_SOUND_SKILL_SPLIT_JAVELIN2	},
	{	"wav\\Effects\\Skill\\Morion\\StormJavelin 1.wav",				SKILL_SOUND_SKILL_STORM_JAVELIN1	},
	{	"wav\\Effects\\Skill\\Morion\\StormJavelin 2.wav",				SKILL_SOUND_SKILL_STORM_JAVELIN2	},
	{	"wav\\Effects\\Skill\\Morion\\TriumphofValhalla 1.wav",			SKILL_SOUND_SKILL_TRIUMPH_OF_VALHALLA1	},
	{	"wav\\Effects\\Skill\\Morion\\TriumphofValhalla 3.wav",			SKILL_SOUND_SKILL_TRIUMPH_OF_VALHALLA2  },
	{	"wav\\Effects\\Skill\\Morion\\VigorBall 1.wav",					SKILL_SOUND_SKILL_VIGOR_BALL1		},
	{	"wav\\Effects\\Skill\\Morion\\VigorBall 2.wav",					SKILL_SOUND_SKILL_VIGOR_BALL2		},
	{	"wav\\Effects\\Skill\\Morion\\VirtualLife 1.wav",				SKILL_SOUND_SKILL_VIRTUAL_LIFE		},

	///////////////////////////////////// 4차 스킬 /////////////////////////////////

	{	"wav\\Effects\\Skill\\Tempskron\\AssassinEye 01.wav",			SKILL_SOUND_SKILL_ASSASSIN_EYE		},
	{	"wav\\Effects\\Skill\\Tempskron\\Berserker 01.wav",				SKILL_SOUND_SKILL_BERSERKER			},
	{	"wav\\Effects\\Skill\\Tempskron\\Charging 01.wav",				SKILL_SOUND_SKILL_CHARGING			},
	{	"wav\\Effects\\Skill\\Tempskron\\ChargingStrike 03.wav",		SKILL_SOUND_SKILL_CHARGING_STRIKE	},
	{	"wav\\Effects\\Skill\\Tempskron\\Compulsion 01.wav",			SKILL_SOUND_SKILL_COMPULSION1		},
	{	"wav\\Effects\\Skill\\Tempskron\\Compulsion 05.wav",			SKILL_SOUND_SKILL_COMPULSION2		},
	{	"wav\\Effects\\Skill\\Tempskron\\Compulsion 06.wav",			SKILL_SOUND_SKILL_COMPULSION3		},
	{	"wav\\Effects\\Skill\\Tempskron\\CycloneStrike 01.wav",			SKILL_SOUND_SKILL_CYCLONE_STRIKE	},
	{	"wav\\Effects\\Skill\\Tempskron\\ForceOfNature 01.wav",			SKILL_SOUND_SKILL_FORCE_OF_NATURE	},
	{	"wav\\Effects\\Skill\\Tempskron\\GodsBless 01.wav",				SKILL_SOUND_SKILL_GODS_BLESS		},
	{	"wav\\Effects\\Skill\\Tempskron\\GodlyShield 01.wav",			SKILL_SOUND_SKILL_GODLY_SHIELD		},
	{	"wav\\Effects\\Skill\\Tempskron\\impulsion 02.wav",				SKILL_SOUND_SKILL_IMPULSION			},
	{	"wav\\Effects\\Skill\\Tempskron\\MetalGolem 01.wav",			SKILL_SOUND_SKILL_METAL_GOLEM		},
	{	"wav\\Effects\\Skill\\Tempskron\\PhoenixShot 01.wav",			SKILL_SOUND_SKILL_PHOENIX_SHOT		},
	{	"wav\\Effects\\Skill\\Tempskron\\ReCallWolverin 01.wav",		SKILL_SOUND_SKILL_RECALL_WOLVERIN	},
	{	"wav\\Effects\\Skill\\Tempskron\\ShadowStrike 01.wav",			SKILL_SOUND_SKILL_SHADOW_STRIKE1	},
	{	"wav\\Effects\\Skill\\Tempskron\\ShadowStrike 02.wav",			SKILL_SOUND_SKILL_SHADOW_STRIKE2	},
	{	"wav\\Effects\\Skill\\Tempskron\\ShadowStrike 03.wav",			SKILL_SOUND_SKILL_SHADOW_STRIKE3	},
	{	"wav\\Effects\\Skill\\Tempskron\\ShadowStrikeFinish 01.wav",	SKILL_SOUND_SKILL_SHADOW_STRIKE4	},
	{	"wav\\Effects\\Skill\\Tempskron\\vague 01.wav",					SKILL_SOUND_SKILL_VAGUE				},


	{	"wav\\Effects\\Skill\\Morion\\ChainLightning 01.wav",			SKILL_SOUND_SKILL_CHAIN_LIGHTNING1	},
	{	"wav\\Effects\\Skill\\Morion\\ChainLightning 02.wav",			SKILL_SOUND_SKILL_CHAIN_LIGHTNING2	},
	{	"wav\\Effects\\Skill\\Morion\\Distortion 01.wav",				SKILL_SOUND_SKILL_DISTORTION		},
	{	"wav\\Effects\\Skill\\Morion\\DIvinePiercing 07.wav",			SKILL_SOUND_SKILL_DVINE_PIERCING	},
	{	"wav\\Effects\\Skill\\Morion\\eXtremeRage 01.wav",				SKILL_SOUND_SKILL_EXTREM_RAGE1		},
	{	"wav\\Effects\\Skill\\Morion\\eXtremeRage 02.wav",				SKILL_SOUND_SKILL_EXTREM_RAGE2		},
	{	"wav\\Effects\\Skill\\Morion\\eXtremeRage 03.wav",				SKILL_SOUND_SKILL_EXTREM_RAGE3		},
	{	"wav\\Effects\\Skill\\Morion\\FireElemental 01.wav",			SKILL_SOUND_SKILL_FIRE_ELEMENTAL	},
	{	"wav\\Effects\\Skill\\Morion\\FlameWave 01.wav",				SKILL_SOUND_SKILL_FLAME_WAVE1		},
	{	"wav\\Effects\\Skill\\Morion\\FlameWave 02.wav",				SKILL_SOUND_SKILL_FLAME_WAVE2		},
	{	"wav\\Effects\\Skill\\Morion\\FlameWaveA 01.wav",				SKILL_SOUND_SKILL_FLAME_WAVE3		},
	{	"wav\\Effects\\Skill\\Morion\\FrostJavelin 01.wav",				SKILL_SOUND_SKILL_FROST_JAVELIN		},
	{	"wav\\Effects\\Skill\\Morion\\GlacialSpike 01.wav",				SKILL_SOUND_SKILL_GLACIAL_SPIKE1	},
	{	"wav\\Effects\\Skill\\Morion\\GlacialSpike 02.wav",				SKILL_SOUND_SKILL_GLACIAL_SPIKE2	},
	{	"wav\\Effects\\Skill\\Morion\\HallOfValhalla 01.wav",			SKILL_SOUND_SKILL_HALL_OF_VALHALLA	},
	{	"wav\\Effects\\Skill\\Morion\\Meteo 01.wav",					SKILL_SOUND_SKILL_METEO1			},
	{	"wav\\Effects\\Skill\\Morion\\Meteo 02.wav",					SKILL_SOUND_SKILL_METEO2			},
	{	"wav\\Effects\\Skill\\Morion\\Meteo 03.wav",					SKILL_SOUND_SKILL_METEO3			},
	{	"wav\\Effects\\Skill\\Morion\\Muspell 01.wav",					SKILL_SOUND_SKILL_MUSPELL1			},
	{	"wav\\Effects\\Skill\\Morion\\Muspell 02.wav",					SKILL_SOUND_SKILL_MUSPELL2			},
	{	"wav\\Effects\\Skill\\Morion\\RegenField 01.wav",				SKILL_SOUND_SKILL_REGEN_FIELD1		},
	{	"wav\\Effects\\Skill\\Morion\\RegenField 02.wav",				SKILL_SOUND_SKILL_REGEN_FIELD2		},
	{	"wav\\Effects\\Skill\\Morion\\SwordofJustice 01.wav",			SKILL_SOUND_SKILL_SWORD_OF_JUSTICE	},
	{	"wav\\Effects\\Skill\\Morion\\Vengeance 01.wav",				SKILL_SOUND_SKILL_VENGEANCE1		},
	{	"wav\\Effects\\Skill\\Morion\\Vengeance 02.wav",				SKILL_SOUND_SKILL_VENGEANCE2		},

	{	0,0 }
};



//사운드 초기화
int	InitSoundEffect( HWND hWnd )
{
	int	cnt;
	HANDLE hFindHandle;
	WIN32_FIND_DATA	fd;
	char	szFindPath[64];

	LoadSoundNum=0;

	for(cnt=0;cnt<NUM_SOUNDS;cnt++)
		sangJuWav_F[cnt] = 1;

	while(1) {
		if ( sangJuWav2[LoadSoundNum]==NULL ) break;
		lstrcpy( sangJuWav[LoadSoundNum] , sangJuWav2[LoadSoundNum] );
		LoadSoundNum++;
	}

	snEffectCnt = 0;

	for( cnt=0;cnt<snFIND_EFFECT_MAX;cnt++ ) {
		lstrcpy( szFindPath , snFindEffects[cnt].lpStringCode );
		lstrcat( szFindPath , "*.wav" );

		//WAV 파일을 찾음
		hFindHandle = FindFirstFile( szFindPath , &fd );
		if ( hFindHandle!=INVALID_HANDLE_VALUE ) {
			while(1) {
				//웨이브파일 등록
				lstrcpy( sangJuWav[LoadSoundNum] , snFindEffects[cnt].lpStringCode );
				lstrcat( sangJuWav[LoadSoundNum] , fd.cFileName );

				if ( snFindEffects[cnt].CharSoundCode>=0x1000 ) {
					//몬스터 소리는 초기화시 읽지 않고 연주할때 바로 읽어 들인다
					sangJuWav_F[LoadSoundNum] = 0;
				}

				//이펙트 등록
				snEffect[snEffectCnt].RealSoundNum = LoadSoundNum+1;
				snEffect[snEffectCnt].CharSoundCode = snFindEffects[cnt].CharSoundCode;
				snEffect[snEffectCnt].MotionState = FindMotionState( fd.cFileName );

				snEffectCnt ++;
				LoadSoundNum ++;

				//다음 파일 찾음
				if ( FindNextFile( hFindHandle , &fd )==FALSE ) break;
				if ( snEffectCnt>=snEFFECT_MAX ) break;
			}
		}
		else {
			snEffectCnt = snEffectCnt;

		}

		FindClose( hFindHandle );
	}

	snEffectWeaponCnt = 0;

	lstrcpy( szFindPath , szSndWeaponPath );
	lstrcat( szFindPath , "*.wav" );

	//WAV 파일을 찾음
	hFindHandle = FindFirstFile( szFindPath , &fd );
	if ( hFindHandle!=INVALID_HANDLE_VALUE ) {
		while(1) {
			//웨이브파일 등록
			lstrcpy( sangJuWav[LoadSoundNum] , szSndWeaponPath );
			lstrcat( sangJuWav[LoadSoundNum] , fd.cFileName );

			//이펙트 등록
			snEffectWeapon[snEffectWeaponCnt].RealSoundNum = LoadSoundNum+1;
			snEffectWeapon[snEffectWeaponCnt].WeaponState = FindWeaponState( fd.cFileName );

			snEffectWeaponCnt ++;
			LoadSoundNum ++;

			//다음 파일 찾음
			if ( FindNextFile( hFindHandle , &fd )==FALSE ) break;
			if ( snEffectWeaponCnt>=snEFFECT_MAX ) break;

		}
		FindClose( hFindHandle );
	}

	sinSoundNum = LoadSoundNum+1;
	cnt = 0;
	while(1) {
		if ( sinSoundWav[cnt]==NULL ) break;
		lstrcpy( sangJuWav[LoadSoundNum] , sinSoundWav[cnt++] );
		LoadSoundNum++;
	}

	esSoundNum = LoadSoundNum+1;
	cnt = 0;
	while(1) {
		if ( esSoundWav[cnt]==NULL ) break;
		lstrcpy( sangJuWav[LoadSoundNum] , esSoundWav[cnt++] );
		LoadSoundNum++;
	}

	SkillSoundNum = LoadSoundNum+1;
	cnt = 0;
	while(1) {
		if ( SkillSoundWav[cnt].szFile==NULL ) break;
		lstrcpy( sangJuWav[LoadSoundNum] , SkillSoundWav[cnt++].szFile );
		LoadSoundNum++;
	}

	//배경 효과음 번호
	AmbientSoundNum = esSoundNum+AMBIENT_SOUND_NUM;
	AmbientSoundNum2 = esSoundNum+AMBIENT_SOUND_NUM2;
	ContinueSoundNum = esSoundNum+CONTINUE_SOUND_NUM;

	sangJuWav[LoadSoundNum][0] = 0;

	InitDX_Wav2( hWnd ); //초기화 

	return TRUE;
}


//사운드 닫기
int CloseSoundEffect()
{
	
	TermDX_Wav2();			   //말기화

	return TRUE;
}


//신바람 사운드 연주
int	sinPlaySound( int Num , int Volume )
{

	PlayWav2( sinSoundNum+Num , 0 , Volume , 2205 );

	return TRUE;
}



//사운드 연주
int	esPlaySound( int Num , int Volume )
{

	PlayWav2( esSoundNum+Num , 0 , Volume , 2205 );

	return TRUE;
}

//사운드 연주
int	esPlaySound( int Num , int Volume , int SampleRate )
{

	PlayWav2( esSoundNum+Num , 0 , Volume , SampleRate );

	return TRUE;
}


int	esObjectSoundNum =0;
int esObjectVolume =0;

//물체 거리에 따른 사운드
int esPlayObjectSound( int Num , int x, int y, int z )
{
	int vol;
	int OldSound;
	int SoundNum;

	if ( Num<0 ) {
		if ( esObjectSoundNum ) {
			StopWav( esObjectSoundNum+esSoundNum );
			esObjectSoundNum = 0;
		}
		return TRUE;
	}

	SoundNum = esSoundNum+Num;
	vol =  GetDistVolume(x,y,z)+100;
	if ( vol>400 ) vol=400;

	OldSound = esObjectSoundNum;

	if ( Num==esObjectSoundNum && vol==esObjectVolume ) return TRUE;

	if ( vol ) {
		if ( esObjectSoundNum==Num ) {
			PlayWav_VolChange(SoundNum,vol);
		}
		else {
			PlayWav2( SoundNum , 1 , vol , 2205 );
			esObjectSoundNum = Num;
			if ( OldSound )
				StopWav( OldSound+Num );
		}
	}
	else {
		if ( OldSound ) {
			StopWav( OldSound+Num );
		}
	}

	esObjectVolume = vol;

	return TRUE;
}

//배경 효과음
char *szAmbientSound2[] = {
	"wav\\Ambient\\stone-mill.wav",				// 0 

	"wav\\Ambient\\dungeon_pumping_b.wav",		// 1
	"wav\\Ambient\\dungeon_pumping_l.wav",		// 2	마을 수레
	"wav\\Ambient\\dungeon_spin_b.wav",			// 3
	"wav\\Ambient\\lake 1.wav",					// 4 작은 물
	"wav\\Ambient\\mystic 1.wav",				// 5 신비
	"wav\\Ambient\\mystic 2.wav",				// 6 신비 2
	"wav\\Ambient\\ship_swaing_l.wav",			// 7	텔레포트 존
	"wav\\Ambient\\stream.wav",					// 8	조금 큰 물소리
	"wav\\Ambient\\town_bell 1.wav",			// 9	종
	"wav\\Ambient\\town_bell 2.wav",			// 10   종 2
	"wav\\Ambient\\town_spin_b.wav",			// 11
	"wav\\Ambient\\town_spin_l.wav",			// 12	큰 바람개비
	"wav\\Ambient\\waterfall.wav",				// 13   폭포
	"wav\\Ambient\\stone-mill2.wav",			// 14	작은 굴러가는 소리

	"wav\\Ambient\\watermill 1.wav",			// 15	물레방아 소리
	"wav\\Ambient\\windmill 1.wav",				// 16	풍차
	"wav\\Ambient\\windmill 2.wav",				// 17	풍차	- 마을
	"wav\\Ambient\\windmill 3.wav",				// 18	풍차	- 필드 일반
	"wav\\Ambient\\windmill 4.wav",				// 19	풍차	- 빠른 풍차
	"wav\\Ambient\\owl 1.wav",					// 20	부엉

	"wav\\Ambient\\dungeon_big-bolt 1.wav",		// 21	
	"wav\\Ambient\\dungeon_stonepole.wav",		// 22	
	"wav\\Ambient\\zombiemill 1.wav",			// 23	좀비 바퀴
	"wav\\Ambient\\dungeon_weight.wav",			// 24	추
	"wav\\Ambient\\dungeon_device_side.wav",	// 25	돌아가는 칼
	"wav\\Ambient\\dungeon_device_axe.wav",		// 26	
	"wav\\Ambient\\dungeon_cage 1.wav",			// 27	수레

	0
};


//Sod 관련소리
char *szSodSound[] = {
	"wav\\Sod\\start.wav",				// 0	Start
	"wav\\Sod\\round1.wav",				// 1	1
	"wav\\Sod\\round2.wav",				// 2	1
	"wav\\Sod\\round3.wav",				// 3	1
	"wav\\Sod\\round4.wav",				// 4	1
	"wav\\Sod\\round5.wav",				// 5	1
	"wav\\Sod\\round6.wav",				// 6	1
	"wav\\Sod\\round7.wav",				// 7	1
	"wav\\Sod\\round8.wav",				// 8	1
	"wav\\Sod\\round9.wav",				// 9	1
	"wav\\Sod\\stage.wav",				// 10	Stage
	"wav\\Sod\\complete.wav",			// 11	complete
	"wav\\Sod\\fail.wav",				// 12	fail.wav
	"wav\\Sod\\congratulation.wav",		// 13	congratulation


	"wav\\Sod\\BGM\\Stage_Complete.wav",//	14
	"wav\\Sod\\BGM\\Stage_Fail.wav",	//	15
	"wav\\Sod\\BGM\\Ending.wav",		//	16

	0
};

#define	CONT_SOUND_MAX	8
#define	CONT_SOUND_MASK	7

int	ContSoundBuff[CONT_SOUND_MAX] = { -1, };
int	ContSoundPush = 0;
int	ContSoundPop = 0;
int	ContSoundNum = -1;

static int	LastVolumeDist = 0xFFFF;
static int  AmbientFlag = 0;

//마지막 볼륨 거리를 초기화후 값을 반환
int ResetLastVolumeDist()
{
	int cnt;

	cnt = LastVolumeDist;
	LastVolumeDist = 0xFFFF;
	AmbientFlag = 0;

	return cnt;
}


//거리에 따른 볼륨을 구함 ( 반경을 지정 )
int GetDistVolume2( int dx, int dy, int dz , int round )
{
	int vol;
	int x,y,z;

	x = (lpCurPlayer->pX-dx)>>FLOATNS;
	y = (lpCurPlayer->pY-dy)>>FLOATNS;
	z = (lpCurPlayer->pZ-dz)>>FLOATNS;

	if ( abs(x)<256*fONE && abs(y)<256*fONE && abs(z)<256*fONE ) {
		vol = x*x+z*z+y*y-round*round;
		vol /= 1100;
	}
	else
		vol = 0xFFFF;

	if ( LastVolumeDist>vol ) LastVolumeDist = vol;

	if ( vol>400 ) vol=400;

	vol = 400-vol;

	return vol;
}

//물체 거리에 따른 사운드2
int esPlayObjectSound2( int AmbNum , int round , int x, int y, int z )
{
	int vol;

	if ( AmbNum<0 ) {
		//소리 제거
		if ( AmbientSound2>=0 ) {
			StopWav( AmbientSoundNum2 );
			AmbientSound2 = -1;
		}
		return TRUE;
	}

	vol =  GetDistVolume2(x,y,z,round);

	if ( vol>100 ) {
		vol += 100;
		if ( vol>399 ) vol=399;

		if ( AmbNum!=AmbientSound2 && !AmbientFlag ) {
			if ( CreateBufferFromWaveFile2( szAmbientSound2[AmbNum], AmbientSoundNum2 )==FALSE )
				return FALSE;
			PlayWav2( AmbientSoundNum2 , 1 , vol , 2205 );
			AmbientSound2 = AmbNum;
		}
		else {
			PlayWav_VolChange(AmbientSoundNum2,vol);
		}
		AmbientFlag ++;
	}

	return TRUE;

}

int esContSound()
{
	int cnt;

	ContSoundCount++;

	if ( (ContSoundCount&0x7)!=0 ) return FALSE;

	if ( ContSoundNum>=0 ) {
		if ( IsPlayWav(ContinueSoundNum)==TRUE ) {	//현재 연주중
			return FALSE;
		}

		ContSoundNum = -1;
	}

	if ( ContSoundPush>ContSoundPop ) {
		cnt = ContSoundPop&CONT_SOUND_MASK;
		ContSoundNum = ContSoundBuff[cnt];

		if ( CreateBufferFromWaveFile2( szSodSound[ContSoundNum], ContinueSoundNum )==FALSE )
			return FALSE;

		PlayWav2( ContinueSoundNum , 0 , 360 , 2205 );

		ContSoundPop++;

		return TRUE;
	}

	return FALSE;
}

//연결 효과음 삽입
int esPlayContSound( int SoundNum )
{
	int cnt;

	if ( (ContSoundPush-ContSoundPop)>=CONT_SOUND_MAX ) return FALSE;

	cnt = ContSoundPush&CONT_SOUND_MASK;
	ContSoundBuff[cnt] = SoundNum;
	ContSoundPush++;

	return TRUE;
}

//스킬 사운드 연주
int	SkillPlaySound( int Num , int Volume , int Loop  )
{

	PlayWav2( SkillSoundNum+Num , Loop , Volume , 2205 );

	return TRUE;
}

//스킬 사운드 연주
int	SkillPlaySound( DWORD dwCode , int x, int y, int z , int Flag  )
{
	int cnt;

	cnt = 0;
	while(1) {
		if ( !SkillSoundWav[cnt].szFile ) break;
		if ( SkillSoundWav[cnt].dwCode==dwCode ) {
			return SkillPlaySound( cnt , GetDistVolume( x,y,z ) , Flag );
		}
		cnt++;
	}

	return FALSE;
}

//스킬 사운드 연주
int	SkillStopSound( DWORD dwCode )
{
	int cnt;

	cnt = 0;
	while(1) {
		if ( !SkillSoundWav[cnt].szFile ) break;
		if ( SkillSoundWav[cnt].dwCode==dwCode ) {
			return StopWav( SkillSoundNum+cnt );
		}
		cnt++;
	}

	return FALSE;
}



//int	AmbientSound = 0;
//int	AmbientSoundNum = 0;
//char *szAmbientSound[] = {


//사운드 연주
int	esPlayEffectBgm( int Num )
{

	if ( Num<0 ) {
		if ( AmbientSound>=0 ) {
			StopWav(AmbientSoundNum);
			AmbientSound = Num;
		}
		return TRUE;
	}

	if ( AmbientSound!=Num ) {
		if ( CreateBufferFromWaveFile2( szAmbientSound[Num], AmbientSoundNum )==FALSE )
			return FALSE;

		PlayWav2( AmbientSoundNum , 1 , 400 , 2205 );
		AmbientSound = Num;
	}

	return TRUE;
}


static int	FootStepCount = 0;

//발자국 소리
int PlayFootStep( smCHAR *lpChar )
{
	int floor;
	int cnt;


	FootStepCount++;

	if ( lpChar->OnStageField>=0 ) {
		floor = StageField[lpChar->OnStageField]->State;

		if ( lpChar->WaterHeight!=CLIP_OUT && 
			lpChar->WaterHeight>lpChar->pY && lpChar->WaterHeight<(lpChar->pY+32*fONE) ) {

				if ( FootStepCount&1 ) {
					//물 발소리
					cnt = 14;
					esPlaySound( cnt+((FootStepCount>>1)%3) , 400 );
					//esPlaySound( cnt+(rand()%3) , 400 );
				}
				return TRUE;
			}

		switch(floor) {
		case FIELD_STATE_VILLAGE:
		case FIELD_STATE_RUIN:
			//딱딱한 발소리
			cnt = 2;
			esPlaySound( cnt+(rand()%3) , 400 );
			break;

		case FIELD_STATE_FOREST:
		case FIELD_STATE_DESERT:
			//부드러운 발소리
			cnt = 0;
			esPlaySound( cnt+(rand()%2) , 400 );
			break;

		case FIELD_STATE_DUNGEON:
			//부드러운 발소리
			cnt = 11;
			esPlaySound( cnt+(rand()%3) , 400 );
			break;
		}
	}

	return TRUE;
}


//캐릭터 사운드
int	CharPlaySound( smCHAR *lpChar )
{
	int cnt;

	DWORD	dwMotionCode;
	DWORD	dwCharSoundCode;
	int	vol;
	int x,y,z;

	int	CodeBuff[32];
	int	CodeBuffCnt = 0;

	x = (lpChar->pX - lpCurPlayer->pX)>>FLOATNS;
	z = (lpChar->pZ - lpCurPlayer->pZ)>>FLOATNS;
	y = (lpChar->pY - lpCurPlayer->pY)>>FLOATNS;

	vol = x*x+z*z+y*y;
	vol /= 1100;

	if ( vol>400 ) vol=400;
	if ( vol<0 ) return TRUE;

	vol = 400-vol;


	if ( lpChar->smCharInfo.State==smCHAR_STATE_USER ) 
		dwCharSoundCode = lpChar->smCharInfo.JOB_CODE;
	else
		dwCharSoundCode = lpChar->smCharInfo.dwCharSoundCode;



	if ( !lpChar->MotionInfo ) return FALSE;

	dwMotionCode = lpChar->MotionInfo->State;


	if ( dwMotionCode==CHRMOTION_STATE_RESTART ) {
		esPlaySound( 6 , vol );
		return TRUE;
	}

	if ( dwMotionCode==CHRMOTION_STATE_WALK || dwMotionCode==CHRMOTION_STATE_RUN ) {
		//발자국 소리는 근처에서만 들림
		if ( vol<300 ) return TRUE;
		dwMotionCode = CHRMOTION_STATE_WALK;
	}


	//뽕망치 공격 소리전환
	if ( lpChar->smCharInfo.wPlayClass[0]==MONSTER_CLASS_HAMMER && dwMotionCode==CHRMOTION_STATE_ATTACK )
		dwMotionCode=CHRMOTION_STATE_HAMMER;

	for( cnt=0;cnt<snEffectCnt;cnt++) {
		if ( snEffect[cnt].CharSoundCode==dwCharSoundCode &&
			snEffect[cnt].MotionState==dwMotionCode ) {

			CodeBuff[CodeBuffCnt++] = cnt;
			//PlayWav( snEffect[cnt].RealSoundNum , 0 , vol , 2205 );
		}
	}

	if ( CodeBuffCnt ) {

		int	Feq;
		Feq =  2205;
		if ( lpChar->smCharInfo.wPlayClass[1] && lpChar->smCharInfo.Brood!=smCHAR_MONSTER_USER ) {
			if ( lpChar->smCharInfo.wPlayClass[1]<fONE )
				Feq = 3600;
			if ( lpChar->smCharInfo.wPlayClass[1]>fONE*2 )
				Feq = 1400;
		}
		if ( lpChar->smCharInfo.State==smCHAR_STATE_USER && lpChar->smCharInfo.SizeLevel>=0x1000 )
			Feq = 2800;

		PlayWav2( snEffect[CodeBuff[ rand()%CodeBuffCnt ]].RealSoundNum , 0 , vol , Feq );
		return TRUE;
	}


	return FALSE;
}


//무기 사운드
int	WeaponPlaySound( smCHAR *lpChar )
{
	int cnt;

	DWORD	dwCode;

	int	vol;
	int x,y,z;
	int	ItemNum;
	int HandType;

	int	CodeBuff[32];
	int	CodeBuffCnt = 0;

	if ( lpChar!=lpCurPlayer ) {

		x = (lpChar->pX - lpCurPlayer->pX)>>FLOATNS;
		z = (lpChar->pZ - lpCurPlayer->pZ)>>FLOATNS;
		y = (lpChar->pY - lpCurPlayer->pY)>>FLOATNS;

		vol = x*x+z*z+y*y;
		vol /= 1100;

		if ( vol>400 ) vol=400;
		if ( vol<0 ) return TRUE;

		vol = 400-vol;
	}
	else
		vol = 400;


/*
#define sinWA1			0x01010000	//Axes
#define sinWC1			0x01020000	//Claws
#define sinWH1			0x01030000	//Hammer & So On
#define sinWM1			0x01040000	//Magicial Stuffs
#define sinWP1			0x01050000	//Poles & Spears
#define sinWS1			0x01060000	//Shooters
#define sinWS2			0x01070000	//Swords
#define sinWT1			0x01080000	//Throwing Arms
	{	"one hand swing axe"		,		1	},
	{	"two hand swing axe"		,		2	},
	{	"one hand swing sword"		,		3	},
	{	"two hand swing sword"		,		4	},
	{	"one hand swing spear"		,		5	},
	{	"two hand swing spear"		,		6	},

	{	"one hand scratch"			,		7	},
	{	"bow shoot"					,		8	},
	{	"one hand swing blunt"		,		9	},
	{	"two hand swing blunt"		,		10	},
	{	"throwing"					,		11	},
	{	"one hand miss"				,		12	},
	{	"two hand miss"				,		13	}
*/

	for( cnt=0;cnt<MAX_ITEM;cnt++) {
		if ( sItem[cnt].CODE==lpChar->dwActionItemCode ) {
			ItemNum = cnt;
			HandType = sItem[cnt].Class;
			break;
		}
	}
	if ( cnt==MAX_ITEM ) return FALSE;

	if ( lpChar->AttackCritcal<0 ) {
		//공격실패
		if ( HandType==ITEM_CLASS_WEAPON_ONE ) 
			dwCode = 12;
		else 
			dwCode = 13;
	}
	else {
		dwCode = 14;			//펀치

			switch(lpChar->dwActionItemCode&sinITEM_MASK2) {
			case sinWA1:
				//도끼
				if (HandType==ITEM_CLASS_WEAPON_ONE) 
					dwCode = 1;
				else
					dwCode = 2;
				break;

			case sinWS2:
				//칼
				if ( (lpChar->dwActionItemCode&sinITEM_MASK3)<=sin03 ) {
					//작은칼
					dwCode = 15;
					break;
				}

				//큰칼
				if (HandType==ITEM_CLASS_WEAPON_ONE) 
					dwCode = 3;
				else
					dwCode = 4;
				break;

			case sinWP1:
				//봉 창
				if (HandType==ITEM_CLASS_WEAPON_ONE) 
					dwCode = 5;
				else
					dwCode = 6;
				break;

			case sinWC1:
				//클러
				dwCode = 7;
				break;

			case sinWS1:
				//활 석궁
				dwCode = 8;
				break;

			case sinWM1:
				//마법지팡이
				if ( lpChar->smCharInfo.JOB_CODE==8 || lpChar->smCharInfo.JOB_CODE==7 ) {
					if ( lpChar->WeaponEffect ) return TRUE;
					//마법
					if (HandType==ITEM_CLASS_WEAPON_ONE) 
						dwCode = 17;
					else
						dwCode = 18;
				}
				else {
					//타격
					if (HandType==ITEM_CLASS_WEAPON_ONE) 
						dwCode = 9;
					else
						dwCode = 10;
				}
				break;

			case sinWH1:
				//망치
				if (HandType==ITEM_CLASS_WEAPON_ONE) 
					dwCode = 9;
				else
					dwCode = 10;
				break;

			case sinWT1:
				//던지는 무기
				dwCode = 11;
				break;
			}
	}

	for( cnt=0;cnt<snEffectWeaponCnt;cnt++) {
		if ( snEffectWeapon[cnt].WeaponState==dwCode ) {
			CodeBuff[CodeBuffCnt++] = cnt;
		}
	}

	if ( CodeBuffCnt ) {
		PlayWav2( snEffectWeapon[CodeBuff[ rand()%CodeBuffCnt ]].RealSoundNum , 0 , vol , 2205 );

		if ( (lpChar->AttackCritcal>0 || (lpChar->AttackCritcal>=0 && lpChar->AttackEffect) ) && 
			lpChar->dwActionItemCode) {
			//크리티컬 사운드
			dwCode = 16;
			CodeBuffCnt = 0;
			for( cnt=0;cnt<snEffectWeaponCnt;cnt++) {
				if ( snEffectWeapon[cnt].WeaponState==dwCode ) {
					CodeBuff[CodeBuffCnt++] = cnt;
				}
			}
			if ( CodeBuffCnt ) 
				PlayWav2( snEffectWeapon[CodeBuff[ rand()%CodeBuffCnt ]].RealSoundNum , 0 , vol , 2205 );
		}
		return TRUE;
	}

	return FALSE;
}

//무기 소리 직접 연주
int PlayWaponSoundDirect( int x,int y, int z , DWORD dwCode )
{
	
	int cnt;
	int	vol;

	int	CodeBuff[32];
	int	CodeBuffCnt = 0;

	//dwCode = 16;
	CodeBuffCnt = 0;
	for( cnt=0;cnt<snEffectWeaponCnt;cnt++) {
		if ( snEffectWeapon[cnt].WeaponState==dwCode ) {
			CodeBuff[CodeBuffCnt++] = cnt;
		}
	}

	vol = GetDistVolume( x,y,z );

	if ( CodeBuffCnt && vol>10 ) 
		PlayWav2( snEffectWeapon[CodeBuff[ rand()%CodeBuffCnt ]].RealSoundNum , 0 , vol , 2205 );


	return TRUE;
}


//int PlayBackMusic( int Num )

char *szMusicFile[] = {
	"wav\\bgm\\Field - Desert - Pilgrim.bgm",
	"wav\\bgm\\Town - Tempskron_Stronghold.bgm",
	"wav\\bgm\\Town 1 - Tempskron_Ricarten - When wind comes-o.bgm",
	"wav\\bgm\\wind loop.bgm",
	"wav\\bgm\\Field - Forest - DarkWood.bgm",
	"wav\\bgm\\Dungeon - Gloomy Heart.bgm",
	"wav\\bgm\\Town 2 - Morion_Philliy - Voyage above the Clouds.bgm",

	//SoD 뮤직
	"wav\\Sod\\BGM\\Stage_Play1.bgm",
	"wav\\Sod\\BGM\\Stage_Play1.bgm",		//임시 2가 없어서...
	"wav\\Sod\\BGM\\Stage_Play3.bgm",
	"wav\\bgm\\Field - Desert - Pilgrim.bgm",
	"wav\\bgm\\Ice 1.bgm",					//11 아이스 배경음악

	NULL
};


/*
	OpenBGM("wav\\Field - Desert - Pilgrim.wav");
	PlayBGM();
*/
int	NextMusicNum;
DWORD	dwFadeStartTime =0;
int LastMusicVolume;
int	PlayingBgmNum =-1;


//음악 연주
int PlayBGM_Direct( int MusicNum )
{

	if ( smConfig.BGM_Mode && MusicNum>=0 ) {
		OpenBGM( szMusicFile[MusicNum] );
		PlayBGM();
		dwFadeStartTime = 0;
		PlayingBgmNum = MusicNum;
		NextMusicNum = MusicNum;
		SetVolumeBGM( 360 );
		LastMusicVolume = 360;
	}
	return TRUE;
}


//배경 음악 변경
int ChangeBGM( int MusicNum )
{
	if ( smConfig.BGM_Mode ) {
		if ( !dwFadeStartTime ) {
			if ( NextMusicNum==MusicNum ) return FALSE;
			NextMusicNum = MusicNum;
			dwFadeStartTime = dwPlayTime;
			//페이드후 연주
		}
		else {
			NextMusicNum = MusicNum;
		}
	}

	return TRUE;
}

int BGM_Main()
{
	int vol;

	if ( dwFadeStartTime ) { 
		vol = (dwPlayTime-dwFadeStartTime)/10;
		vol = 360-vol;

		if ( vol<=0 ) {
			if ( NextMusicNum>=0 ) {
				OpenBGM( szMusicFile[NextMusicNum] );
				PlayBGM();
			}
			else {
				StopBGM();
			}

			dwFadeStartTime = 0;
			PlayingBgmNum = NextMusicNum;
		}
		else {
			if ( LastMusicVolume!=vol ) {
				SetVolumeBGM( vol );
			}
		}

		LastMusicVolume = vol;
	}

	esContSound();

	return TRUE;
}

int GetDistVolume( int dx, int dy, int dz )
{
	int vol;
	int x,y,z;

	x = (lpCurPlayer->pX-dx)>>FLOATNS;
	y = (lpCurPlayer->pY-dy)>>FLOATNS;
	z = (lpCurPlayer->pZ-dz)>>FLOATNS;

	vol = x*x+z*z+y*y;
	vol /= 1100;

	if ( vol>400 ) vol=400;

	vol = 400-vol;

	return vol;
}


//######################################################################################
//작 성 자 : 오 영 석
const char *TALK_SOUND_DIRECOTRY = { "wav\\Talk\\" };

char		g_szTalkSoundFullName[128];
char		g_szTalkSoundName[64];
DWORD		g_TalkSoundSum;

char		g_szPlayTalkSoundName[64];
DWORD		g_PlayTalkSoundSum;

static int  s_TalkSoundBuffNum;
static int  s_TalkSoundIsPlay;
//######################################################################################

//######################################################################################
//작 성 자 : 오 영 석
int FindTalkSound( char *lpSrcMessage )
{
	g_TalkSoundSum = GetSpeedSum( lpSrcMessage );
	::_ui64toa( g_TalkSoundSum, g_szTalkSoundName, 10 );
	strcat( g_szTalkSoundName, ".wav" );
	
	strcpy( g_szTalkSoundFullName, TALK_SOUND_DIRECOTRY );
	strcat( g_szTalkSoundFullName, g_szTalkSoundName );

	FILE *fp = fopen( g_szTalkSoundFullName, "rb" );
	if( fp == NULL )
		return FALSE;

	return TRUE;
}
//######################################################################################


//######################################################################################
//작 성 자 : 오 영 석
extern int isPlaying(int i);
int IsPlayTalkSound(void)
{
	if( s_TalkSoundBuffNum && s_TalkSoundIsPlay )
	{
		s_TalkSoundIsPlay = isPlaying( s_TalkSoundBuffNum );
		return s_TalkSoundIsPlay;
	}

	return 0;
}
//######################################################################################


//######################################################################################
//작 성 자 : 오 영 석
void PlayTalkSound(void)
{
	if( s_TalkSoundBuffNum == 0 )
	{
		s_TalkSoundBuffNum = LoadSoundNum;
		sangJuWav_F[ s_TalkSoundBuffNum ] = 0;
		LoadSoundNum++;
	}

	if( CreateBufferFromWaveFile2( g_szTalkSoundFullName, s_TalkSoundBuffNum ) == FALSE )
		return;

	PlayWav2( s_TalkSoundBuffNum, 0, 400, 2205 );

	lstrcpy( g_szPlayTalkSoundName, g_szTalkSoundName );
	g_PlayTalkSoundSum = g_TalkSoundSum;
	s_TalkSoundIsPlay = 1;
}
//######################################################################################


//######################################################################################
//작 성 자 : 오 영 석
void StopTalkSound(void)
{
	if( s_TalkSoundBuffNum )
		StopWav( s_TalkSoundBuffNum );

	s_TalkSoundIsPlay = 0;
}
//######################################################################################



