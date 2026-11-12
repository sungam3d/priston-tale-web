

#define SIN_SKILL_USE_RIGHT		0x01000000		//Available only for the right 
#define SIN_SKILL_USE_LEFT		0x02000000		//Available only for the left 
#define SIN_SKILL_USE_ALL		0x03000000		//Both available 
#define SIN_SKILL_USE_NOT		0x04000000		//Passive Skill

//Mechanician
{"Extreme Shield",		"Temporarily increases block    rating of the current shield",
10,		34,2,		110,4,
{0,0,0},			{sinDS1,0,0,0,0,0,0,0},
F_Extreme_Shield,SKILL_EXTREME_SHIELD,SIN_SKILL_USE_RIGHT,E_Shield_UseMana},

{"Mechanic Bomb",		"Throws a bomb to enemies within explosive blast radius",
12,		28,2,		73,3,
{0,0,0},			{0,0,0,0,0,0,0,0},
F_Mechanic_Bomb,SKILL_MECHANIC_BOMB,SIN_SKILL_USE_RIGHT,Mechanic_Bomb_UseMana},

{"Poison Attribute",		"Permanently raises resistance  to poison",
14,		0,0,		0,0,
{0,0,0},			{0,0,0,0,0,0,0,0},
F_Poison_Attribute,SKILL_POISON_ATTRIBUTE,SIN_SKILL_USE_NOT,0},

{"Physical Absorption",		"Temporarily increases the      body's ability to absorb damage",
17,		30,2,		110,5,
{0,0,0},			{0,0,0,0,0,0,0,0},
F_Physical_Absorption,SKILL_PHYSICAL_ABSORB,SIN_SKILL_USE_RIGHT,P_Absor_bUseMana},

{"Great Smash",			"Channels strength into one     powerful blow",
20,		38,2,		73,3,
{0,0,0},			{sinWA1,sinWC1,sinWH1,sinWP1,sinWS2,0,0,0},
F_Great_Smash,SKILL_GREAT_SMASH,SIN_SKILL_USE_ALL,G_Smash_UseMana},

{"Maximize",			"Optimizes weapons to raise     their attack power permanently",
23,		41,2,		110,4,
{0,0,0},			{sinWA1,sinWC1,sinWH1,sinWP1,sinWS2,sinWS1,sinWT1,0},
F_Maximize,SKILL_MAXIMIZE,SIN_SKILL_USE_RIGHT,Maximize_UseMana},

{"Automation",			"Automates projectile weapons   to raise their attack speed    and power temporarily",
26,		42,2,		110,4,
{0,0,0},			{sinWS1,sinWT1,0,0,0,0,0,0},
F_Automation,SKILL_AUTOMATION,SIN_SKILL_USE_RIGHT,Automation_UseMana},

{"Spark",			"Attacks nearby enemies with    powerful lightning bolts",
30,		40,2,		82,2,
{0,0,0},			{0,0,0,0,0,0,0,0},
F_Spark,SKILL_SPARK,SIN_SKILL_USE_RIGHT,Spark_UseMana},

{"Metal Armor",		"Raises the defense of the mechanician's armor in a limited time.",
40,		48,2,		110,5,
{0,0,0},			{0,0,0,0,0,0,0,0},
F_Metal_Armor,SKILL_METAL_ARMOR,SIN_SKILL_USE_RIGHT,Metal_Armor_UseMana},

{"Grand Smash",	"Accurate and continous attack to opponents. Raises attack rating",
43,		44,2,		82,2,
{0,0,0},			{sinWA1,sinWC1,sinWP1,sinWS2,0,0,0,0},
F_Grand_Smash,SKILL_GRAND_SMASH,SIN_SKILL_USE_RIGHT,Grand_Smash_UseMana},

{"Mechanic Weapon Mastery",	"Optimizes the mechanic spec. items.",
46,		0,0,		0,0,
{0,0,0},			{0,0,0,0,0,0,0,0},
F_M_W_Mastery,SKILL_MECHANIC_WEAPON,SIN_SKILL_USE_NOT,0},

{"Spark Shield",	    "Forms invisible electric guard.",
50,		52,2,		110,2,
{0,0,0},			{sinDS1,0,0,0,0,0,0,0},
F_Spark_Shield,SKILL_SPARK_SHIELD,SIN_SKILL_USE_RIGHT,Spark_Shield_UseMana},

//[메카니션]4차 스킬
{"Impulsion","It attacks closed monsters by an electricity of powerful blow.",
60,     	55,2,		130,0,
{1,0,0},            {sinWC1,sinWH1,sinWP1,sinWS2,0,0,0,0},
F_Impulsion,SKILL_IMPULSION,SIN_SKILL_USE_RIGHT,Impulsion_UseMana},

{"Completion","It gathers closed monsters by a powerful shout.",
63,     	65,2,		440,0,
{1,0,0},            {0,0,0,0,0,0,0,0},
F_Compulsion,SKILL_COMPULSION,SIN_SKILL_USE_RIGHT,Compulsion_UseMana},

{"Magnetic Sphere","It creates powerful magnetism Sphere to selected user.",
66,     	71,2,		600,0,
{1,0,0},            {0,0,0,0,0,0,0,0},
F_Magnetic_Sphere,SKILL_MAGNETIC_SPHERE,SIN_SKILL_USE_RIGHT,Magnetic_Sphere_UseMana},

{"Metal Golem","It increases your defense and ATK Rating by strong iron of Golem's cloth.",
70,     	71,2,		1000,0,
{1,0,0},            {0,0,0,0,0,0,0,0},
F_Metal_Golem,SKILL_METAL_GOLEM,SIN_SKILL_USE_RIGHT,Metal_Golem_UseMana},


//Fighter 
{"Melee Mastery",		"Increases efficiency in melee  weapons, increasing Attack     Power permanently",
10,		0,0,		0,0,
{0,0,0},			{sinWA1,sinWC1,sinWH1,sinWS2,0,0,0,0},
F_Melee_Mastery,SKILL_MELEE_MASTERY,SIN_SKILL_USE_NOT,0},

{"Fire Attribute",		"Permanently raises resistance  to fire",
12,		0,0,		0,0,
{0,0,0},			{0,0,0,0,0,0,0,0},
F_Fire_Attribute,SKILL_FIRE_ATTRIBUTE,SIN_SKILL_USE_NOT,0},

{"Raving",			"Goes berserk for a short period of time, but loses HP with each attack",
14,		35,2,		93,3,
{0,0,0},			{sinWA1,sinWC1,sinWH1,sinWP1,sinWS2,0,0,0},
F_Raving,SKILL_RAVING,SIN_SKILL_USE_ALL,Raving_UseMana},

{"Impact",			"Raises attack rating to        accurately strike the enemy",
17,		37,2,		86,4,
{0,0,0},			{sinWA1,sinWC1,sinWP1,sinWS2,0,0,0,0},
F_Impact,SKILL_IMPACT,SIN_SKILL_USE_ALL,Impact_UseMana},

{"Triple Impact",		"A more powerful version of     Impact",
20,		39,2,		80,4,
{0,0,0},			{sinWA1,sinWC1,sinWP1,sinWS2,0,0,0,0},
F_Triple_Impact,SKILL_TRIPLE_IMPACT,SIN_SKILL_USE_ALL,T_Impact_UseMana},

{"Brutal Swing",		"Delivers a powerful and fatal  blow to the enemy",
23,		41,2,		82,2,
{0,0,0},			{sinWA1,sinWS2,0,0,0,0,0,0},
F_Brutal_Swing,SKILL_BRUTAL_SWING,SIN_SKILL_USE_ALL,B_Swing_UseMana},

{"Roar",			"Shouting out a deafening roar to   momentarily stun and confuse   enemies",
26,		39,2,		112,4,
{0,0,0},			{0,0,0,0,0,0,0,0},
F_Roar,SKILL_ROAR,SIN_SKILL_USE_RIGHT,Roar_UseMana},

{"Rage of Zechram",		"Borrows the skills of the legendary hero Zechram",
30,		45,2,		84,4,
{0,0,0},			{sinWA1,sinWH1,sinWS2,0,0,0,0,0},
F_RageOfZecram,SKILL_RAGE_OF_ZECRAM,SIN_SKILL_USE_RIGHT,R_Zecram_UseMana},

{"Concentration",	"Increases attack rating for a while.",
40,		53,2,		110,3,
{0,0,0},			{sinWA1,sinWS2,0,0,0,0,0,0},
F_Concentration,SKILL_CONCENTRATION,SIN_SKILL_USE_RIGHT,Concentration_UseMana},

{"Avenging Crash",	"Powerful crashing attacks to opponents.",
43,		48,2,		73,3,
{0,0,0},			{sinWA1,sinWP1,sinWS2,0,0,0,0,0},
F_Avanging_Crash,SKILL_AVANGING_CRASH,SIN_SKILL_USE_RIGHT,A_Crash_UseMana},

{"Swift Axe",	"Raises attack speed of axe in a limited time.",
46,		53,2,		200,3,
{0,0,0},			{sinWA1,0,0,0,0,0,0,0},
F_Swift_Axe,SKILL_SWIFT_AXE,SIN_SKILL_USE_RIGHT,Swift_Axe_UseMana},

{"Bone crash",		"Used to defeat demons by ancestors. Powerful attack that hits opponent twice.",
50,		54,2,		73,3,
{0,0,0},			{sinWA1,sinWP1,sinWS2,0,0,0,0,0},
F_Bone_Crash,SKILL_BONE_CRASH,SIN_SKILL_USE_RIGHT,B_Crash_UseMana},

//[파이터]4차 스킬
{"Destroyer",		"Strongest Double-hit to a monster.",
60,		45,2,		150,0,
{1,0,0},			{sinWA1,sinWH1,0,0,0,0,0,0},
F_Destoryer,SKILL_DETORYER,SIN_SKILL_USE_ALL,Destoryer_UseMana},

{"Berserker","Change like uncontrol condition, but it decrease the absorb dramatically",
63,		60,2,		700,0,
{1,0,0},			{0,0,0,0,0,0,0,0},
F_Berserker,SKILL_BERSERKER,SIN_SKILL_USE_RIGHT,Berserker_UseMana},

{"Cyclone Strike","It attacks Monsters by strong spin ATK.",
66,		55,2,		135,0,
{1,0,0},			{sinWA1,sinWH1,sinWP1,sinWS2,0,0,0,0},
F_Cyclone_Strike,SKILL_CYCLONE_STRIKE,SIN_SKILL_USE_RIGHT,Cyclone_Strike_UseMana},

{"Boost Health","It increases HP permanently.",
70,		0,0,		0,0,
{1,0,0},			{0,0,0,0,0,0,0,0},
F_Boost_Health,SKILL_BOOST_HEALTH,SIN_SKILL_USE_NOT,},

//Pikeman
{"Pike Wind",			"Creates a whirlwind to push    back and damage enemies",
10,		27,2,		80,4,
{0,0,0},			{sinWP1,0,0,0,0,0,0,0},
F_Pike_Wind,SKILL_PIKE_WIND,SIN_SKILL_USE_RIGHT,Pike_Wind_UseMana},

{"Ice Attribute",		"Permanently raises resistance  to frost",
12,		0,0,		0,0,
{0,0,0},			{0,0,0,0,0,0,0,0},
F_Ice_Attribute,SKILL_ICE_ATTRIBUTE,SIN_SKILL_USE_NOT,0},

{"Critical Hit",		"Aims for enemies' weak spots   to raise probability of a      critical strike",
14,		31,2,		84,3,
{0,0,0},			{sinWP1,sinWC1,sinWH1,sinWS2,0,0,0,0},
F_Critical_Hit,SKILL_CRITICAL_HIT,SIN_SKILL_USE_ALL,Critical_Hit_UseMana},

{"Jumping Crash",		"Leaps up into the air to strikedown with a powerful blow",
17,		34,2,		76,4,
{0,0,0},			{sinWA1,sinWP1,sinWS2,0,0,0,0,0},
F_Jumping_Crash,SKILL_JUMPING_CRASH,SIN_SKILL_USE_ALL,Jumping_Crash_UseMana},

{"Ground Pike",			"Imbues weapon with frost       properties to deliver a        powerful attack",
20,		36,2,		84,4,
{0,0,0},			{sinWP1,0,0,0,0,0,0,0},
F_Ground_Pike,SKILL_GROUND_PIKE,SIN_SKILL_USE_RIGHT,G_Pike_UseMana},

{"Tornado",			"Summons a tornado to attack    nearby enemies",
23,		38,2,		83,4,
{0,0,0},			{sinWP1,0,0,0,0,0,0,0},
F_Tornado_Pike,SKILL_TORNADO,SIN_SKILL_USE_RIGHT,Tornado_UseMana},

{"Weapon DEF Mastery",		"Permanently raises defense     rating with polearms",
26,		0,0,		0,0,
{0,0,0},			{sinWA1,sinWH1,sinWP1,sinWS2,0,0,0,0},
F_Weapone_Defence_Mastery,SKILL_WEAPONE_DEFENCE_MASTERY,SIN_SKILL_USE_NOT,0},


{"Expansion",			"Imbues weapon with Vigor to    unleash a devastating attack",
30,		42,2,		82,4,
{0,0,0},			{sinWA1,sinWH1,sinWP1,0,0,0,0,0},
F_Expansion,SKILL_EXPANSION,SIN_SKILL_USE_ALL,Expansion_UseMana},

{"Venom sphere",		"Thrust a sphere to the ground to attack opponents.",
40,		50,2,		110,2,
{0,0,0},			{sinWP1,0,0,0,0,0,0,0},
F_Venom_Spear,SKILL_VENOM_SPEAR,SIN_SKILL_USE_RIGHT,VenomSpear_UseMana},

{"Vanish",			"It makes you an invisible man in a limited time.",
43,		53,2,		140,2,
{0,0,0},			{0,0,0,0,0,0,0,0},
F_Vanish,SKILL_VANISH,SIN_SKILL_USE_RIGHT,Vanish_UseMana},

{"Critical Mastery",		"The skill makes to increase Critical ATK (%) permanently",
46,		0,0,		0,0,
{0,0,0},			{sinWP1,0,0,0,0,0,0,0},
F_Critical_Mastery,SKILL_CRITICAL_MASTERY,SIN_SKILL_USE_NOT,0},

{"Chain Lancer",	  		  "Critical Spinning 3 Hits ATK",
50,		51,2,		88,3,
{0,0,0},			{sinWC1,sinWP1,sinWS2,0,0,0,0,0},
F_Chain_Lance,SKILL_CHAIN_LANCE,SIN_SKILL_USE_RIGHT,Chain_Lance_UseMana},

//[파이크맨]4차 스킬
{"Assassin Eye",	 "Find a monster's weak point, increase concentration.",
60,		61,2,		520,0,
{1,0,0},			{sinWP1,0,0,0,0,0,0,0},
F_Assassin_Eye,SKILL_ASSASSIN_EYE,SIN_SKILL_USE_RIGHT,Assassin_Eye_UseMana},

{"Charging Strike",	 "Gathers a power and Continuous ATK.",
63,		75,2,		150,0,
{1,0,0},			{sinWP1,sinWS2,0,0,0,0,0,0},
F_Charging_Strike,SKILL_CHARGING_STRIKE,SIN_SKILL_USE_RIGHT,Charging_Strike_UseMana},

{"Vague",	"It does not make your body clearly.",
66,		71,2,		650,0,
{1,0,0},			{sinWP1,0,0,0,0,0,0,0},
F_Vague,SKILL_VAGUE,SIN_SKILL_USE_RIGHT,Vague_UseMana},

{"Shadow Master",	"Some afterimages make a powerful damage to a monster in turn.",
70,		51,2,		160,0,
{1,0,0},			{sinWP1,sinWS2,0,0,0,0,0,0},
F_Shadow_Master,SKILL_SHADOW_MASTER,SIN_SKILL_USE_RIGHT,Shadow_Master_UseMana},


//Archer
{"Scout Hawk",			"Summons a hawk to scout the    surrounding area, raising attack rating",
10,		25,2,		115,5,
{0,0,0},			{0,0,0,0,0,0,0,0},
F_Scout_Hawk ,SKILL_SCOUT_HAWK,SIN_SKILL_USE_RIGHT,Scout_Hawk_UseMana },

{"Shooting Mastery",		"Permanently raises attack power with bows and crossbows",
12,		0,0,		0,0,
{0,0,0},			{sinWS1,0,0,0,0,0,0,0},
F_Shooting_Mastery,SKILL_SHOOTING_MASTERY,SIN_SKILL_USE_NOT,0},

{"Wind Arrow",			"Imbues an arrow with wind      properties to increase attack  power",
14,		25,2,		82,3,
{0,0,0},			{sinWS1,0,0,0,0,0,0,0},
F_Wind_Arrow,SKILL_WIND_ARROW,SIN_SKILL_USE_ALL,Wind_Arrow_UseMana},

{"Perfect Aim",			"Makes an aimed shot with       increased Attack Rating and    Power",
17,		29,2,		81,3,
{0,0,0},			{sinWS1,0,0,0,0,0,0,0},
F_Perfect_Aim,SKILL_PERFECT_AIM,SIN_SKILL_USE_ALL,Perfect_Aim_UseMana},


{"Dion's Eye",			"Trains eyesight like the       legendary archer Dion to       permanently increase attack rating",
20,		0,0,		0,0,
{0,0,0},			{sinWS1,0,0,0,0,0,0,0},
F_Dions_Eye,SKILL_DIONS_EYE,SIN_SKILL_USE_NOT,0},

{"Falcon",			"Summons a fierce falcon to     attack nearby enemies",
23,		36,2,		115,4,
{0,0,0},			{0,0,0,0,0,0,0,0},
F_Falcon ,SKILL_FALCON,SIN_SKILL_USE_RIGHT,Falcon_UseMana },

{"Arrow of Rage",		"Shoots an arrow over a desired area which explodes and rains  down fire on enemies",
26,		38,2,		82,4,
{0,0,0},			{sinWS1,0,0,0,0,0,0,0},
F_Arrow_of_Rage,SKILL_ARROW_OF_RAGE,SIN_SKILL_USE_RIGHT,A_Rage_UseMana},

{"Avalanche",			"Unleashes a barrage of piercing arrows on a single target",
30,		41,2,		103,2,
{0,0,0},			{sinWS1,0,0,0,0,0,0,0},
F_Avalanche,SKILL_AVALANCHE,SIN_SKILL_USE_RIGHT,Avalanche_UseMana},

{"Elemental Shot",		"Continuous elememtal attack to opponents",
40,		43,2,		82,2,
{0,0,0},			{sinWS1,0,0,0,0,0,0,0},
F_E_Shot,SKILL_ELEMENTAL_SHOT,SIN_SKILL_USE_RIGHT,Elemental_Shot_UseMana},

{"Golden Falcon",		"Summons a falcon to attack opponents",
43,		52,2,		120,4,
{0,0,0},			{0,0,0,0,0,0,0,0},
F_G_Falcon,SKILL_GOLDEN_FALCON,SIN_SKILL_USE_RIGHT,Golden_Falcon_UseMana},

{"Bum shot",		    "Fires explosive arrows twice. Splash damages opponents.",
46,		46,2,		87,3,
{0,0,0},			{sinWS1,0,0,0,0,0,0,0},
F_B_Shot,SKILL_BOMB_SHOT,SIN_SKILL_USE_RIGHT,BombShot_UseMana},

{"Perforation",		"Accurate and continuous attack that perforates opponents.",
50,		48,2,		82,2,
{0,0,0},			{sinWS1,0,0,0,0,0,0,0},
F_Perforation,SKILL_PERFORATION,SIN_SKILL_USE_RIGHT,Perforation_UseMana},

//[아쳐]4차 스킬
{"Recall Wolverine",		"Summons Wolverine.",
60,		58,2,		900,0,
{1,0,0},			{0,0,0,0,0,0,0,0},
F_Recall_Wolverin,SKILL_RECALL_WOLVERIN,SIN_SKILL_USE_RIGHT,Recall_Wolverin_UseMana},

{"Evasion Mastery",	"It avoids an enemy attack effectively.",
63,		0,0,		0,0,
{1,0,0},			{0,0,0,0,0,0,0,0},
F_Evasion_Mastery,SKILL_EVASION_MASTERY,SIN_SKILL_USE_NOT,0},

{"Phoenix Shot",	"It gives wide ATK with Energy of Phoenix.",
66,		55,2,		100,0,
{1,0,0},			{sinWS1,0,0,0,0,0,0,0},
F_Phoenix_Shot,SKILL_PHOENIX_SHOT,SIN_SKILL_USE_RIGHT,Phoenix_Shot_UseMana},

{"Force of Nature","It improves your ability by natural assimilation perfectly.",
70,		56,2,		900,0,
{1,0,0},			{sinWS1,0,0,0,0,0,0,0},
F_Force_Of_Nature,SKILL_FORCE_OF_NATURE,SIN_SKILL_USE_RIGHT,Force_Of_Nature_UseMana},


/*----------------------------------------------------------------------------*
*							Morion
*-----------------------------------------------------------------------------*/	
/////// Knight
{"Sword Blast",		"throws a devastating sword to attack enemies",
10,		22,2,		82,2,
{0,0,0},			{sinWS2,0,0,0,0,0,0,0},
F_Sword_Blast,SKILL_SWORD_BLAST,SIN_SKILL_USE_ALL,Sword_Blast_UseMana},

{"Holy Body",			"become holy to increase absorption against undead units.",
12,		28,2,		110,3,
{0,0,0},			{0,0,0,0,0,0,0,0},
F_HolyBody ,SKILL_HOLY_BODY,SIN_SKILL_USE_RIGHT,HolyBody_UseMana },

{"Physical Training",		"physical training increases stamina permanently",
14,		0,0,		0,0,
{0,0,0},			{0,0,0,0,0,0,0,0},
F_Physical_Traning,SKILL_PHYSICAL_TRANING,SIN_SKILL_USE_NOT,0},

{"Double Crash",		"makes 2 consecutive attacks to enemies.",
17,		35,2,		82,2,
{0,0,0},			{sinWA1,sinWC1,sinWS2,0,0,0,0,0},
F_Double_Crash,SKILL_DOUBLE_CRASH,SIN_SKILL_USE_RIGHT,D_Crash_UseMana},

{"Holy Valor",			"Increase in attack against undead with brave force.",
20,		36,2,		100,3,
{0,0,0},			{0,0,0,0,0,0,0,0},
F_Holy_Valor,SKILL_HOLY_VALOR,SIN_SKILL_USE_RIGHT,Holy_Valor_Mana},

{"Brandish",			"Attack all enemies within range.",
23,		38,2,		82,2,
{0,0,0},			{sinWP1,sinWS2,0,0,0,0,0,0},
F_Brandish,SKILL_BRANDISH,SIN_SKILL_USE_ALL,Brandish_Mana},

{"Piercing",			"Slash lots of enemies with strong power and spirit.",
26,		41,2,		82,2,
{0,0,0},			{sinWS2,0,0,0,0,0,0,0},
F_Piercing,SKILL_PIERCING,SIN_SKILL_USE_ALL,Piercing_Mana},

{"Drastic Spirit",		"Increase defense for a given time.",
30,		44,2,		100,2,
{0,0,0},			{0,0,0,0,0,0,0,0},
F_Drastic_Spirit,SKILL_DRASTIC_SPIRIT,SIN_SKILL_USE_RIGHT,Drastic_Spirit_Mana},

{"Sword Mastery",	"The skill improves ATK ability permanently with use of swords.",
40,		0,0,		0,0,
{0,0,0},			{sinWS2,0,0,0,0,0,0,0},
F_S_Mastery,SKILL_SWORD_MASTERY,SIN_SKILL_USE_NOT,0},

{"Divine Shield",		"It absorbs an Undead Monster ATK Power, and resurrects the HP.",
43,		46,2,		130,1,
{0,0,0},			{sinDS1,0,0,0,0,0,0,0},
F_D_Inhalation,SKILL_DIVINE_INHALATION,SIN_SKILL_USE_RIGHT,D_Inhalation_UseMana},

{"Holy incantation",	"Holy incantation that lures monsters.",
46,		50,2,		120,1,
{0,0,0},			{0,0,0,0,0,0,0,0},
F_Holy_Incantation,SKILL_HOLY_INCANTATION,SIN_SKILL_USE_RIGHT,H_Incantation_UseMana},

{"Grand cross",	"Deadly cross attacks to opponents.",
50,		45,2,		82,2,
{0,0,0},			{sinWH1,sinWS2,0,0,0,0,0,0},
F_Grand_Cross,SKILL_GRAND_CROSS,SIN_SKILL_USE_RIGHT,Grand_Cross_UseMana},

//[나이트]4차 스킬
{"Divine piercing",	"It attacks a monster by God's power with invincible sword dance.",
70,		51,2,		160,0,
{1,0,0},			{sinWS2,0,0,0,0,0,0,0},
F_Divine_Piercing,SKILL_DIVINE_PIERCING,SIN_SKILL_USE_RIGHT,Divine_Piercing_UseMana},

{"Godly Shield",	"It creates a holy shield blessed by God",
63,		50,2,		400,0,
{1,0,0},			{sinDS1,0,0,0,0,0,0,0},
F_Godly_Shield,SKILL_GODLY_SHIELD,SIN_SKILL_USE_RIGHT,Godly_Shield_UseMana},

{"God Bless",	"It makes great power of Saint Knight by the name of God.",
66,		51,2,		330,0,
{1,0,0},			{sinWS2,0,0,0,0,0,0,0},
F_God_Bless,SKILL_GOD_BLESS,SIN_SKILL_USE_RIGHT,God_Bless_UseMana},

{"Sword of Justice","Strike downward with strong power by bending back the body.",
60,		53,2,		250,0,
{1,0,0},			{sinWS2,0,0,0,0,0,0,0},
F_Sword_Of_Justice,SKILL_SWORD_OF_JUSTICE,SIN_SKILL_USE_RIGHT,Sword_Of_Justice_UseMana},

/////////// Atalanta
{"Shield Strike",		"hurls a shield to attack enemies.",
10,		23,2,		82,2,
{0,0,0},			{sinDS1,0,0,0,0,0,0,0},
F_S_Strike ,SKILL_SHIELD_STRIKE,SIN_SKILL_USE_RIGHT,S_Strike_UseMana },

{"Farina",			"throws a spear that creates illusions",
12,		26,2,		82,2,
{0,0,0},			{sinWT1,0,0,0,0,0,0,0},
F_Farina,SKILL_FARINA,SIN_SKILL_USE_ALL,Farina_UseMana},

{"Throwing Mastery",		"train javelin attacks to increase damage permanently.",
14,		0,0,		0,0,
{0,0,0},			{sinWT1,0,0,0,0,0,0,0},
F_D_Mastery,SKILL_THROWING_MASTERY,SIN_SKILL_USE_NOT,0},

{"Bigger Spear",		"attack an enemy with holy javelin.",
17,		32,2,		82,2,
{0,0,0},			{sinWP1,sinWT1,0,0,0,0,0,0},
F_V_Spear,SKILL_VIGOR_SPEAR,SIN_SKILL_USE_ALL,V_Spear_UseMana},

{"Windy",			"Increase distance and attack rating of javelin.",
20,		34,2,		100,2,
{0,0,0},			{sinWT1,0,0,0,0,0,0,0},
F_Windy,SKILL_WINDY,SIN_SKILL_USE_RIGHT,Windy_Mana},

{"Twist Javelin",		"Increase attack rating and power with twisting.",
23,		36,2,		82,2,
{0,0,0},			{sinWT1,0,0,0,0,0,0,0},
F_Twist_Javelin,SKILL_TWIST_JAVELIN,SIN_SKILL_USE_RIGHT,Twist_Javelin_Mana},

{"Soul Sucker",			"Absorb enemy's life.",
26,		42,2,		110,2,
{0,0,0},			{sinDS1,0,0,0,0,0,0,0},
F_Soul_Sucker,SKILL_SOUL_SUCKER,SIN_SKILL_USE_RIGHT,Soul_Sucker_Mana},

{"Fire Javelin",		"Attack enemy with power of fire.",
30,		40,2,		82,2,
{0,0,0},			{sinWT1,0,0,0,0,0,0,0},
F_Fire_Javelin,SKILL_FIRE_JAVELIN,SIN_SKILL_USE_ALL,Fire_Javelin_Mana},

{"Split javelin",	"Continuous attack to opponents at very fast speed.",
40,		43,2,		115,1,
{0,0,0},			{sinWT1,0,0,0,0,0,0,0},
F_Split_Javelin,SKILL_SPLIT_JAVELIN,SIN_SKILL_USE_RIGHT,Split_Javelin_UseMana},

{"Triumph of Valhalla",	"Increases the attack damage of the party members.",
43,		50,2,		120,1,
{0,0,0},			{sinWT1,sinWS2,sinWS1,sinWP1,sinWH1,sinWA1,sinWC1,sinWM1},
F_T_Of_Valhalla,SKILL_TRIUMPH_OF_VALHALLA,SIN_SKILL_USE_RIGHT,T_Of_Valhalla_UseMana},

{"Lightning javelin",	"Increases lightning elemental damage to javelin.",
46,		46,2,		82,2,
{0,0,0},			{sinWT1,0,0,0,0,0,0,0},
F_Lightning_Javelin,SKILL_LIGHTNING_JAVELIN,SIN_SKILL_USE_ALL,Lightning_Javelin_UseMana},

{"Storm javelin",	    "Strong tornado attack to enemies.",
50,		48,2,		82,2,
{0,0,0},			{sinWT1,0,0,0,0,0,0,0},
F_Storm_Javelin,SKILL_STORM_JAVELIN,SIN_SKILL_USE_RIGHT,Storm_Javelin_UseMana},

//[아틀란타]4차 스킬
{"Hall of Vahalla",	"It makes an ability for creating a space with Power of Vahalla.",
60,		52,2,		750,0,
{1,0,0},			{sinWA1,sinWC1,sinWH1,sinWP1,sinWS1,sinWT1,0,0},
F_Hall_Of_Valhalla,SKILL_HALL_OF_VALHALLA,SIN_SKILL_USE_RIGHT,Hall_Of_Valhalla_UseMana},

{"X Rage",	"It attacks an enemy with high jump for Power of Anger.",
63,		48,2,		210,0,
{1,0,0},			{sinWT1,0,0,0,0,0,0,0},
F_X_Rage,SKILL_X_RAGE,SIN_SKILL_USE_RIGHT,X_Rage_UseMana},

{"Frost Javelin",	"While it uses, increases your ice ability for a while.",
66,		50,2,		720,0,
{1,0,0},			{sinWT1,0,0,0,0,0,0,0},
F_Frost_Javelin,SKILL_FROST_JAVELIN,SIN_SKILL_USE_RIGHT,Frost_Javelin_UseMana},

{"Vengeance",	"It attacks fast, continuous ATK.",
70,		55,2,		100,0,
{1,0,0},			{sinWT1,0,0,0,0,0,0,0},
F_Vengeance,SKILL_VENGEANCE,SIN_SKILL_USE_RIGHT,Vengeance_UseMana},

///////////Priestess
{"Healing",			 "heals life with its holy magic.",
10,		15,1,		82,2,
{0,0,0},			{sinWM1,0,0,0,0,0,0,0},
F_Healing,SKILL_HEALING,SIN_SKILL_USE_RIGHT,Healing_UseMana},

{"Holy Bolt",			"hurls a holy bolt to enemies.",
12,		11,1,		82,2,
{0,0,0},			{sinWM1,0,0,0,0,0,0,0},
F_HolyBolt,SKILL_HOLY_BOLT,SIN_SKILL_USE_ALL,HolyBolt_UseMana},

{"Multi Spark",			"Releases magical sparks to attack enemies.",
14,		14,1,		82,2,
{0,0,0},			{sinWM1,0,0,0,0,0,0,0},
F_M_Spark,SKILL_MULTISPARK,SIN_SKILL_USE_RIGHT,M_Spark_UseMana},

{"Holy Mind",			"It weakens enemy's attack in a given time.",
17,		19,1,		100,2,
{0,0,0},			{sinWM1,0,0,0,0,0,0,0},
F_HolyMind,SKILL_HOLY_MIND,SIN_SKILL_USE_RIGHT,HolyMind_UseMana},

{"Meditation",			"Increase mana recovery permanently.",
20,		0,0,		0,0,
{0,0,0},			{0,0,0,0,0,0,0,0},
F_Meditation,SKILL_MEDITATION,SIN_SKILL_USE_NOT,0},

{"Divine Lightning",		"Attack enemies with holy lightning.",
23,		21,1,		82,2,
{0,0,0},			{sinWM1,0,0,0,0,0,0,0},
F_Divine_Lightning,SKILL_DIVINE_LIGHTNING,SIN_SKILL_USE_RIGHT,Divine_Lightning_Mana},

{"Holy Reflection",		"Reflect attacks from undead by creating holy magical shield.",
26,		23,1,		120,1,
{0,0,0},			{sinWM1,0,0,0,0,0,0,0},
F_Holy_Reflection,SKILL_HOLY_REFLECTION,SIN_SKILL_USE_RIGHT,Holy_Reflection_Mana},

{"Grand Healing",		"Heal life of a party member.",
30,		27,1,		82,2,
{0,0,0},			{sinWM1,0,0,0,0,0,0,0},
F_Grand_Healing,SKILL_GRAND_HEALING,SIN_SKILL_USE_RIGHT,Grand_Healing_Mana},

{"Vigor ball",			"Fires vigor ball twice to opponents.",
40,		24,1,		82,2,
{0,0,0},			{sinWM1,0,0,0,0,0,0,0},
F_Vigor_Ball,SKILL_VIGOR_BALL,SIN_SKILL_USE_ALL,Vigor_Ball_UseMana},

{"Resurrection",		"Revives a dead party member.",
43,		31,1,		130,1,
{0,0,0},			{sinWM1,0,0,0,0,0,0,0},
F_Resurrection,SKILL_RESURRECTION,SIN_SKILL_USE_RIGHT,Resurrection_UseMana},

{"Extinction",		"lethal incantation to undead monsters. Permanent damage to undead units.",
46,		28,1,		120,1,
{0,0,0},			{sinWM1,0,0,0,0,0,0,0},
F_Extinction,SKILL_EXTINCTION,SIN_SKILL_USE_RIGHT,Extinction_UseMana},

{"Virtual life",	"Increases party members' life for a while.",
50,		31,1,		134,1,
{0,0,0},			{sinWM1,0,0,0,0,0,0,0},
F_Virtual_Life,SKILL_VIRTUAL_LIFE,SIN_SKILL_USE_RIGHT,Virtual_Life_UseMana},

//[프리스티스]4차 스킬
{"Glacial Spike",	"Huge ice block released to attack enemies.",
60,		35,1,		120,0,
{1,0,0},			{sinWM1,0,0,0,0,0,0,0},
F_Glacial_Spike,SKILL_GLACIAL_SPIKE,SIN_SKILL_USE_RIGHT,Glacial_Spike_UseMana},

{"Regeneration Field",	"After your sacrifice, it recoveries your company continuously.",
63,		41,1,		950,0,
{1,0,0},			{sinWM1,0,0,0,0,0,0,0},
F_Regeneration_Field,SKILL_REGENERATION_FIELD,SIN_SKILL_USE_RIGHT,Regeneration_Field_UseMana},

{"Chain Lightning",	"It attacks powerful thunder ATK to Monsters.",
66,		41,1,		120,0,
{1,0,0},			{sinWM1,0,0,0,0,0,0,0},
F_Chain_Lightning,SKILL_CHAIN_LIGHTNING,SIN_SKILL_USE_RIGHT,Chain_Lightning_UseMana},
//신성계의 Giant를 소환한다.
{"Summon Muspell",	"It absorbs undead monster's ATK, resurrects HP and has summoner's ability.",
70,		47,1,		800,0,
{1,0,0},			{sinWM1,0,0,0,0,0,0,0},
F_Summon_Muspell,SKILL_SUMMON_MUSPELL,SIN_SKILL_USE_RIGHT,Summon_Muspell_UseMana},

///////////Magician
{"Agony",			"sacrifice oneself to increase spirit.",
10,		12,1,		82,2,
{0,0,0},			{sinWM1,0,0,0,0,0,0,0},
F_Agony,SKILL_AGONY,SIN_SKILL_USE_RIGHT,Agony_UseMana},

{"Fire Bolt",			"calls down small fireballs to attack enemies",
12,		11,1,		84,2,
{0,0,0},			{sinWM1,0,0,0,0,0,0,0},
F_FireBolt,SKILL_FIRE_BOLT,SIN_SKILL_USE_ALL,FireBolt_UseMana},

{"Zenith",			"Increase element features with strong spirit for a given time.",
14,		15,1,		100,3,
{0,0,0},			{sinWM1,0,0,0,0,0,0,0},
F_Zenith,SKILL_ZENITH,SIN_SKILL_USE_RIGHT,Zenith_UseMana},

{"Fire Ball",			"calls down huge fireballs to attack enemies",
17,		19,1,		82,2,
{0,0,0},			{sinWM1,0,0,0,0,0,0,0},
F_FireBall,SKILL_FIRE_BALL,SIN_SKILL_USE_RIGHT,FireBall_UseMana},

{"Mental Mastery",		"Increases maximum mana",
20,		0,0,		0,0,
{0,0,0},			{0,0,0,0,0,0,0,0},
F_Mental_Mastery,SKILL_MENTAL_MASTERY,SIN_SKILL_USE_NOT,0},

{"Watornado",			"A pilar of water strikes enemies",
23,		21,1,		82,2,
{0,0,0},			{sinWM1,0,0,0,0,0,0,0},
F_Watornado,SKILL_WATORNADO,SIN_SKILL_USE_RIGHT,Watornado_Mana},

{"Enchant Weapon",		"Enhance the attacking element in weapon of party member",
26,		28,1,		104,2,
{0,0,0},			{sinWM1,0,0,0,0,0,0,0},
F_Enchant_Weapon,SKILL_ENCHANT_WEAPON,SIN_SKILL_USE_RIGHT,Enchant_Weapon_Mana},

{"Death Ray",			"Fire Powerful ray from palms",
30,		26,1,		82,2,
{0,0,0},			{sinWM1,0,0,0,0,0,0,0},
F_Dead_Ray,SKILL_DEAD_RAY,SIN_SKILL_USE_ALL,Dead_Ray_Mana},

{"Energy shield",		"Forms a magical shield around the body",
40,		40,1,		150,1,
{0,0,0},			{sinWM1,0,0,0,0,0,0,0},
F_Energy_Shield,SKILL_ENERGY_SHIELD,SIN_SKILL_USE_RIGHT,Energy_Shield_UseMana},

{"Diastrophism",	"Strong geographical attack to opponents within range.",
43,		33,1,		106,1,
{0,0,0},			{sinWM1,0,0,0,0,0,0,0},
F_Diastrophism,SKILL_DIASTROPHISM,SIN_SKILL_USE_RIGHT,Diastrophism_UseMana},

{"Spirit elemental",	"Summons a ghost to increase magical damage",
46,		35,1,		150,1,
{0,0,0},			{sinWM1,0,0,0,0,0,0,0},
F_Spirit_Elemental,SKILL_SPIRIT_ELEMENTAL,SIN_SKILL_USE_RIGHT,Spirit_Elemental_UseMana},

{"Dancing sword",	"A phantom of sword appears and uses the power of element to attack enemies.",
50,		30,1,		150,1,
{0,0,0},			{sinWM1,0,0,0,0,0,0,0},
F_D_Sword,SKILL_DANCING_SWORD,SIN_SKILL_USE_RIGHT,Dancing_Sword_UseMana},

//[메지션]4차 스킬
{"Fire Elemental",	"The skill summons a strong elemental made with fire.",
60,		120,2,		900,0,
{1,0,0},			{sinWM1,0,0,0,0,0,0,0},
F_Fire_Elemental,SKILL_FIRE_ELEMENTAL,SIN_SKILL_USE_RIGHT,Fire_Elemental_UseMana},

{"Flame Wave",	"It attacks the front monsters with powerful fire flame.",
63,		32,1,		124,0,
{1,0,0},			{sinWM1,0,0,0,0,0,0,0},
F_Flame_Wave,SKILL_FLAME_WAVE,SIN_SKILL_USE_RIGHT,Flame_Wave_UseMana},

{"Distortion",	"It takes down a monster's ability with Distortion of Space.",
66,		43,1,		460,0,
{1,0,0},			{sinWM1,0,0,0,0,0,0,0},
F_Distortion,SKILL_DISTORTION,SIN_SKILL_USE_RIGHT,Distortion_UseMana},

{"Meteorite",	"It attacks nearby monsters with summoned meteorites.",
70,		39,1,		190,0,
{1,0,0},			{sinWM1,0,0,0,0,0,0,0},
F_Meteo,SKILL_M_METEO,SIN_SKILL_USE_RIGHT,Meteo_UseMana},

{"Orb",		"It improves ATK Power with Orb's power",
0,		0,0,		0,0,
{0,0,0},			{0,0,0,0,0,0,0,0},
NULL,SKILL_FORCE_ORB,0,0},

//퀘스트 타임 테이블
{"Quest",		"Hunt monsters!",
0,		0,0,		0,0,
{0,0,0},			{0,0,0,0,0,0,0,0},
NULL,CHANGE_JOB3_QUEST,0,0},

//퀘스트 타임 테이블
{"Quest",		"Hunt monsters!",
0,		0,0,		0,0,
{0,0,0},			{0,0,0,0,0,0,0,0},
NULL,CHANGE_ELEMENTARY_QUEST,0,0},

//공성전 클랜스킬 테이블
{"Absorption of Glory","Increase Absorption +10 for a week.",
0,		0,0,		0,0,
{0,0,0},			{0,0,0,0,0,0,0,0},
NULL,CLANSKILL_ABSORB,0,0},

{"Power Siege","Increase ATK Power 10% for a week.",
0,		0,0,		0,0,
{0,0,0},			{0,0,0,0,0,0,0,0},
NULL,CLANSKILL_ATTACK,0,0},

{"Evasion Adaption","Increase Evasion percent 10% for a week.",
0,		0,0,		0,0,
{0,0,0},			{0,0,0,0,0,0,0,0},
NULL,CLANSKILL_EVASION,0,0},

//공성전 아이템 사용 스킬
{"Atanasia","Decrease ATK Power 1/2 and God Mode for a while",
0,		0,0,		0,0,
{0,0,0},			{0,0,0,0,0,0,0,0},
NULL,SCROLL_INVULNERABILITY,0,0},

{"Deadly Edge","Increase Critical percent 5%",
0,		0,0,		0,0,
{0,0,0},			{0,0,0,0,0,0,0,0},
NULL,SCROLL_CRITICAL,0,0},

{"Avail of Evasion","Increase Evasion percent 5%",
0,		0,0,		0,0,
{0,0,0},			{0,0,0,0,0,0,0,0},
NULL,SCROLL_EVASION,0,0},

{"Defiance Stone","Decrease Damage 50% against ATK of Fire Crystal.",
0,		0,0,		0,0,
{0,0,0},			{0,0,0,0,0,0,0,0},
NULL,STONE_R_FIRECRYTAL,0,0},

{"Defiance Stone","Decrease Damage 50% against ATK of Ice Crystal.",
0,		0,0,		0,0,
{0,0,0},			{0,0,0,0,0,0,0,0},
NULL,STONE_R_ICECRYTAL,0,0},

{"Defiance Stone","Decrease Damage 50% against ATK of Lightning Crystal.",
0,		0,0,		0,0,
{0,0,0},			{0,0,0,0,0,0,0,0},
NULL,STONE_R_LINGHTINGCRYTAL,0,0},

{"Might Stone","Add final atk power 20% while Fighter hits it.",
0,		0,0,		0,0,
{0,0,0},			{0,0,0,0,0,0,0,0},
NULL,STONE_A_FIGHTER,0,0},

{"Might Stone","Add final atk power 20% while Mechanician hits it.",
0,		0,0,		0,0,
{0,0,0},			{0,0,0,0,0,0,0,0},
NULL,STONE_A_MECHANICIAN,0,0},

{"Might Stone","Add final atk power 20% while Pikeman hits it.",
0,		0,0,		0,0,
{0,0,0},			{0,0,0,0,0,0,0,0},
NULL,STONE_A_PIKEMAN,0,0},

{"Might Stone","Add final atk power 20% while Archer hits it.",
0,		0,0,		0,0,
{0,0,0},			{0,0,0,0,0,0,0,0},
NULL,STONE_A_ARCHER,0,0},

{"Might Stone","Add final atk power 20% while Knight hits it.",
0,		0,0,		0,0,
{0,0,0},			{0,0,0,0,0,0,0,0},
NULL,STONE_A_KNIGHT,0,0},

{"Might Stone","Add final atk power 20% while Atalanta hits it.",
0,		0,0,		0,0,
{0,0,0},			{0,0,0,0,0,0,0,0},
NULL,STONE_A_ATALANTA,0,0},

{"Might Stone","Add final atk power 20% while Magician hits it.",
0,		0,0,		0,0,
{0,0,0},			{0,0,0,0,0,0,0,0},
NULL,STONE_A_MAGICIAN,0,0},

{"Might Stone","Add final atk power 20% while Priestess hits it.",
0,		0,0,		0,0,
{0,0,0},			{0,0,0,0,0,0,0,0},
NULL,STONE_A_PRIESTESS,0,0},
//프리미엄 부분 유료화 아이템
{"Eternal Life","Becomes invincible for the set time. The attack power decrease in 1/2",
0,		0,0,		0,0,
{0,0,0},			{0,0,0,0,0,0,0,0},
NULL,SCROLL_P_INVULNERABILITY,0,0},

{"Fatal Edge","15% increase on the critical attack",
0,		0,0,		0,0,
{0,0,0},			{0,0,0,0,0,0,0,0},
NULL,SCROLL_P_CRITICAL,0,0},

{"Avert Scroll","15% increase of evade chance",
0,		0,0,		0,0,
{0,0,0},			{0,0,0,0,0,0,0,0},
NULL,SCROLL_P_EVASION,0,0},

// Tier 5 (the fifth rank). Keep these LAST: the skill-table checksum skips them and
// counts every older row by its position. Level, stamina, cooldown, weapons, mana.
//Mechanician
{"Land Mining","Quickly lays mines nearby that hit the enemies in range.",
80,		65,2,		93,3,
{1,0,0},			{sinWA1,sinWC1,sinWH1,sinWP1,sinWS2,0,0,0},
F_Land_Mining,SKILL_LAND_MINING,SIN_SKILL_USE_RIGHT,Land_Mining_UseMana},
{"Hyper Sonic","Raises the attack speed for a time.",
83,		90,2,		110,5,
{1,0,0},			{sinWA1,sinWC1,sinWH1,sinWP1,sinWS2,sinWS1,sinWT1,0},
F_Hyper_Sonic,SKILL_HYPER_SONIC,SIN_SKILL_USE_RIGHT,Hyper_Sonic_UseMana},
{"Rolling Smash","Four strong blows that deal fatal damage.",
86,		71,2,		82,3,
{1,0,0},			{sinWA1,sinWC1,sinWH1,sinWP1,sinWS2,0,0,0},
F_Rolling_Smash,SKILL_ROLLING_SMASH,SIN_SKILL_USE_RIGHT,Rolling_Smash_UseMana},
{"Power Enhancement","Raises your attack power, and the party's maximum attack power.",
90,		95,2,		110,5,
{1,0,0},			{sinWA1,sinWC1,sinWH1,sinWP1,sinWS2,sinWS1,sinWT1,0},
F_Power_Enhancement,SKILL_POWER_ENHANCEMENT,SIN_SKILL_USE_RIGHT,Power_Enhancement_UseMana},

//Fighter
{"Down Hit","Launches an enemy into the air and strikes it 3 times.",
80,		68,2,		82,3,
{1,0,0},			{sinWA1,sinWH1,sinWP1,sinWS2,0,0,0,0},
F_Down_Hit,SKILL_DOWN_HIT,SIN_SKILL_USE_RIGHT,Down_Hit_UseMana},
{"Power Dash","Charges at the enemy and stuns it.",
83,		81,2,		150,0,
{1,0,0},			{sinWA1,sinWH1,sinWS2,0,0,0,0,0},
F_Power_Dash,SKILL_POWER_DASH,SIN_SKILL_USE_RIGHT,Power_Dash_UseMana},
{"Mortal Blow","Twists the weapon around in slow, powerful swings.",
86,		70,2,		80,4,
{1,0,0},			{sinWA1,sinWH1,0,0,0,0,0,0},
F_Mortal_Blow,SKILL_MORTAL_BLOW,SIN_SKILL_USE_RIGHT,Mortal_Blow_UseMana},
{"Bloody Berserker","Lowers absorption to raise attack damage greatly.",
90,		93,2,		700,0,
{1,0,0},			{0,0,0,0,0,0,0,0},
F_Bloody_Berserker,SKILL_BLOODY_BERSERKER,SIN_SKILL_USE_RIGHT,Bloody_Berserker_UseMana},

//Pikeman
{"Dancing Reaper","Spins through the air and lands a powerful blow.",
80,		75,2,		82,3,
{1,0,0},			{sinWC1,sinWP1,sinWS2,0,0,0,0,0},
F_Dancing_Reaper,SKILL_DANCING_REAPER,SIN_SKILL_USE_RIGHT,Dancing_Reaper_UseMana},
{"Final Spear","Great spears burst from the ground to strike the enemies.",
83,		70,2,		150,0,
{1,0,0},			{sinWP1,0,0,0,0,0,0,0},
F_Final_Spear,SKILL_FINAL_SPEAR,SIN_SKILL_USE_RIGHT,Final_Spear_UseMana},
{"Amplified","Lowers absorption to raise damage and attack rating.",
86,		93,2,		160,0,
{1,0,0},			{sinWP1,sinWS2,0,0,0,0,0,0},
F_Amplified,SKILL_AMPLIFIED,SIN_SKILL_USE_RIGHT,Amplified_UseMana},
{"Side Spin Attack","Whirls around to strike, stealing the enemies' life.",
90,		81,2,		80,4,
{1,0,0},			{sinWP1,sinWS2,0,0,0,0,0,0},
F_Side_Spin_Attack,SKILL_SIDE_SPIN_ATTACK,SIN_SKILL_USE_RIGHT,Side_Spin_Attack_UseMana},

//Archer
{"Evade Shot","Jumps up and kicks the enemy, leaving it stunned.",
80,		52,2,		82,2,
{1,0,0},			{sinWS1,0,0,0,0,0,0,0},
F_Evade_Shot,SKILL_EVADE_SHOT,SIN_SKILL_USE_RIGHT,Evade_Shot_UseMana},
{"Thunder Rope","Fires two electric arrows for a powerful attack.",
83,		52,2,		82,2,
{1,0,0},			{sinWS1,0,0,0,0,0,0,0},
F_Thunder_Rope,SKILL_THUNDER_ROPE,SIN_SKILL_USE_RIGHT,Thunder_Rope_UseMana},
{"Nature Splash","An arrow filled with nature's power makes a vortex.",
86,		57,2,		87,3,
{1,0,0},			{sinWS1,0,0,0,0,0,0,0},
F_Nature_Splash,SKILL_NATURE_SPLASH,SIN_SKILL_USE_RIGHT,Nature_Splash_UseMana},
{"Circle Trap","Strikes the enemy's weakness and slows it down.",
90,		95,2,		87,3,
{1,0,0},			{sinWS1,0,0,0,0,0,0,0},
F_Circle_Trap,SKILL_CIRCLE_TRAP,SIN_SKILL_USE_RIGHT,Circle_Trap_UseMana},

//Knight
{"Soul Breaker","A powerful strike into the enemy's soul.",
80,		69,2,		82,3,
{1,0,0},			{sinWS2,0,0,0,0,0,0,0},
F_Tier5_Pending,SKILL_SOUL_BREAKER,SIN_SKILL_USE_RIGHT,Soul_Breaker_UseMana},
{"Crescent Moon","Rushes the nearby enemies with a crescent slash.",
83,		70,2,		160,0,
{1,0,0},			{sinWS2,0,0,0,0,0,0,0},
F_Tier5_Pending,SKILL_CRESCENT_MOON,SIN_SKILL_USE_RIGHT,Crescent_Moon_UseMana},
{"Saint Blade","Calls divine power into the blade to strike the enemy.",
86,		81,2,		82,2,
{1,0,0},			{sinWS2,sinWH1,0,0,0,0,0,0},
F_Tier5_Pending,SKILL_SAINT_BLADE,SIN_SKILL_USE_RIGHT,Saint_Blade_UseMana},
{"Holy Benedic","Adds damage against undead monsters.",
90,		93,2,		400,0,
{1,0,0},			{sinWA1,sinWC1,sinWH1,sinWP1,sinWS2,sinWS1,sinWT1,0},
F_Tier5_Pending,SKILL_HOLY_BENEDIC,SIN_SKILL_USE_RIGHT,Holy_Benedic_UseMana},

//Atalanta
{"Talaria","Makes the body light and quick.",
80,		68,2,		720,0,
{1,0,0},			{sinWA1,sinWC1,sinWH1,sinWP1,sinWS1,sinWT1,0,0},
F_Tier5_Pending,SKILL_TALARIA,SIN_SKILL_USE_RIGHT,Talaria_UseMana},
{"Galaxy Coup","Calls javelins of Valhalla down to pin the enemies to the ground.",
83,		52,2,		210,0,
{1,0,0},			{sinWT1,0,0,0,0,0,0,0},
F_Tier5_Pending,SKILL_GALAXY_COUP,SIN_SKILL_USE_RIGHT,Galaxy_Coup_UseMana},
{"Summon Arcuda","Summons Arcuda to fight beside you.",
86,		95,2,		720,0,
{1,0,0},			{0,0,0,0,0,0,0,0},
F_Tier5_Pending,SKILL_SUMMON_ARCUDA,SIN_SKILL_USE_RIGHT,Summon_Arcuda_UseMana},
{"Snippy Fear","Throws 2 javelins in a row for critical blows.",
90,		57,2,		100,0,
{1,0,0},			{sinWT1,0,0,0,0,0,0,0},
F_Tier5_Pending,SKILL_SNIPPY_FEAR,SIN_SKILL_USE_RIGHT,Snippy_Fear_UseMana},

//Priestess
{"Spirit Impact","Releases a powerful shockwave at the nearby enemies.",
80,		45,2,		77,3,
{1,0,0},			{sinWM1,0,0,0,0,0,0,0},
F_Tier5_Pending,SKILL_SPIRIT_IMPACT,SIN_SKILL_USE_RIGHT,Spirit_Impact_UseMana},
{"Piercing Ice","Calls a sharp spear of ice down on the enemy.",
83,		50,2,		87,3,
{1,0,0},			{sinWM1,0,0,0,0,0,0,0},
F_Tier5_Pending,SKILL_PIERCING_ICE,SIN_SKILL_USE_RIGHT,Piercing_Ice_UseMana},
{"Summon Ramiel","Summons an angel that strikes with lightning.",
86,		54,2,		800,0,
{1,0,0},			{sinWM1,0,0,0,0,0,0,0},
F_Tier5_Pending,SKILL_SUMMON_RAMIEL,SIN_SKILL_USE_RIGHT,Summon_Ramiel_UseMana},
{"Blessing Krishna","Raises defense and absorption by the grace of the gods.",
90,		60,2,		950,0,
{1,0,0},			{sinWM1,0,0,0,0,0,0,0},
F_Tier5_Pending,SKILL_BLESSING_KRISHNA,SIN_SKILL_USE_RIGHT,Blessing_Krishna_UseMana},

//Magician
{"Silraphim","Strikes the nearby enemies with the power of Silraphim.",
80,		45,2,		87,3,
{1,0,0},			{sinWM1,0,0,0,0,0,0,0},
F_Tier5_Pending,SKILL_SILRAPHIM,SIN_SKILL_USE_RIGHT,Silraphim_UseMana},
{"Vis Tenus","Shrinks the enemy, lowering its life and damage.",
83,		50,2,		460,0,
{1,0,0},			{sinWM1,0,0,0,0,0,0,0},
F_Tier5_Pending,SKILL_VIS_TENUS,SIN_SKILL_USE_RIGHT,Vis_Tenus_UseMana},
{"Prima Ignis","Conjures a primal fireball for a powerful attack.",
86,		54,2,		81,3,
{1,0,0},			{sinWM1,0,0,0,0,0,0,0},
F_Tier5_Pending,SKILL_PRIMA_IGNIS,SIN_SKILL_USE_RIGHT,Prima_Ignis_UseMana},
{"Third Anima","Lowers maximum mana to raise magic power.",
90,		60,2,		150,1,
{1,0,0},			{sinWM1,0,0,0,0,0,0,0},
F_Tier5_Pending,SKILL_THIRD_ANIMA,SIN_SKILL_USE_RIGHT,Third_Anima_UseMana},
