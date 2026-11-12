JOB_DATA_CODE	JobDataBase[] = {
	//Tempskron
	{	"Mechanician"		,"Mechanician"		,JOB_CODE_MECHANICIAN	,	2	,BROOD_CODE_TEMPSKRON ,65,2,2,2,2,2,0},
	{	"Fighter"		,"Fighter"		,JOB_CODE_FIGHTER	,	1	,BROOD_CODE_TEMPSKRON ,70,1,3,1,1,2,0},
	{	"Pikeman"		,"Pikeman"		,JOB_CODE_PIKEMAN	,	4	,BROOD_CODE_TEMPSKRON ,75,1,3,1,1,2,0},
	{	"Archer"		,"Archer"		,JOB_CODE_ARCHER	,	3	,BROOD_CODE_TEMPSKRON ,80,3,3,2,3,1,0},
												 
	{	"Mechanic Master"	,"Mechanic Master"	,JOB_CODE_MECHANICMASTER,	2	,BROOD_CODE_TEMPSKRON },
	{	"Warrior"		,"Warrior"		,JOB_CODE_WARRIOR	,	1	,BROOD_CODE_TEMPSKRON },
	{	"Combatant"		,"Combatant"		,JOB_CODE_COMBATANT	,	4	,BROOD_CODE_TEMPSKRON },
	{	"Hunter Master"		,"Huntress Master"	,JOB_CODE_HUNTERMASTER	,	3	,BROOD_CODE_TEMPSKRON },
												 
	{	"Metal Leader"		,"Metal Leader"		,JOB_CODE_METALLEADER	,	2	,BROOD_CODE_TEMPSKRON },
	{	"Champion"		,"Champion"		,JOB_CODE_CHAMPION	,	1	,BROOD_CODE_TEMPSKRON },
	{	"Lancer"		,"Lancer"		,JOB_CODE_LANCER	,	4	,BROOD_CODE_TEMPSKRON },
	{	"Dion's Disciple"	,"Dion's Disciple"	,JOB_CODE_DIONS_DISCIPLE,	3	,BROOD_CODE_TEMPSKRON },
												 
	{	"Metallion"		,"Metallion"		,JOB_CODE_HEAVYMETAL	,	2	,BROOD_CODE_TEMPSKRON },
	{	"Immortal Warrior"	,"Immortal Warrior"	,JOB_CODE_IMMORTALWARRIOR,	1	,BROOD_CODE_TEMPSKRON },
	{	"Lancelot"		,"Lancelot"		,JOB_CODE_LANCELOT	,	4	,BROOD_CODE_TEMPSKRON },
	{	"Sagittarion"		,"Sagittarion"		,JOB_CODE_SAGITTARION	,	3	,BROOD_CODE_TEMPSKRON },

	// the fifth rank (tier 5 skills): the 32 job bits are all taken, so each shares its
	// fourth rank's bit, and whatever that rank may use, this one may too
	{	"Metal Rage"		,"Metal Rage"		,JOB_CODE_HEAVYMETAL	,	2	,BROOD_CODE_TEMPSKRON },
	{	"Warlord"		,"Warlord"		,JOB_CODE_IMMORTALWARRIOR,	1	,BROOD_CODE_TEMPSKRON },
	{	"Bayiolunt"		,"Bayiolunt"		,JOB_CODE_LANCELOT	,	4	,BROOD_CODE_TEMPSKRON },
	{	"Iskar"			,"Iskar"		,JOB_CODE_SAGITTARION	,	3	,BROOD_CODE_TEMPSKRON },

	//Morion
	{	"Knight"		,"Knight"		,JOB_CODE_KNIGHT	,	6	,BROOD_CODE_MORAYION  , 190,2,2,2,1,2,0},
	{	"Atalanta"		,"Atalanta"		,JOB_CODE_ATALANTA	,	5	,BROOD_CODE_MORAYION  , 200,2,2,2,2,1,0},
	{	"Priestess"		,"Priestess"		,JOB_CODE_PRIEST	,	8	,BROOD_CODE_MORAYION  , 200,4,1,3,3,2,2},
	{	"Magician"		,"Magician"		,JOB_CODE_MAGICIAN	,	7	,BROOD_CODE_MORAYION  , 190,5,1,3,3,2,1},

	{	"Paladin"		,"Paladin"		,JOB_CODE_PALADIN	,	6	,BROOD_CODE_MORAYION },
	{	"Valkyrie"		,"Valkyrie"		,JOB_CODE_VALKYRIE	,	5	,BROOD_CODE_MORAYION },
	{	"Saintess"		,"Saintess"		,JOB_CODE_SAINTESS	,	8	,BROOD_CODE_MORAYION },
	{	"Wizard"		,"Wizard"		,JOB_CODE_WIZARD	,	7	,BROOD_CODE_MORAYION },
	
	{	"HolyKnight"		,"Holy Knight"		,JOB_CODE_HOLYKNIGHT	,	6	,BROOD_CODE_MORAYION },
	{	"Brunhild"		,"Brunhild"		,JOB_CODE_BRUNHILD	,	5	,BROOD_CODE_MORAYION },
	{	"Bishop"		,"Bishop"		,JOB_CODE_BISHOP	,	8	,BROOD_CODE_MORAYION },
	{	"RoyalWizard"		,"RoyalWizard"		,JOB_CODE_ROYALKNIGHT	,	7	,BROOD_CODE_MORAYION },

	{	"SaintKnight"		,"SaintKnight"		,JOB_CODE_SAINTKNIGHT	,	6	,BROOD_CODE_MORAYION },
	{	"Valhalla"		,"Valhalla"		,JOB_CODE_VALHALLA	,	5	,BROOD_CODE_MORAYION },
	{	"Celestial"		,"Celestial"		,JOB_CODE_CELESTIAL	,	8	,BROOD_CODE_MORAYION },
	{	"ArchMage"		,"ArchMage"		,JOB_CODE_ARCHMAGE	,	7	,BROOD_CODE_MORAYION },

	// the fifth rank (see the Tempskron ones)
	{	"Valiant"		,"Valiant"		,JOB_CODE_SAINTKNIGHT	,	6	,BROOD_CODE_MORAYION },
	{	"Queen of Valhalla"	,"Queen of Valhalla"	,JOB_CODE_VALHALLA	,	5	,BROOD_CODE_MORAYION },
	{	"Arc Angel"		,"Arc Angel"		,JOB_CODE_CELESTIAL	,	8	,BROOD_CODE_MORAYION },
	{	"Elemental Master"	,"Elemental Master"	,JOB_CODE_ARCHMAGE	,	7	,BROOD_CODE_MORAYION },

	// The later classes (JOB_CODE 9 Assassin, 10 Shaman, 11 Martial Artist): starting stats,
	// sight and growth functions as the later game has them. No job bits are free, so each
	// takes the bit the later game gave it (shared with a second or third rank above, which no
	// item names). They stay LAST: sinItem.cpp's list stops at the first row without a bit.
	{	"Assassin"		,"Assassin"		,0x00000010		,	9	,BROOD_CODE_TEMPSKRON , 80,3,3,2,1,2,0},
	{	"Rogue"			,"Rogue"		,0x00000010		,	9	,BROOD_CODE_TEMPSKRON },
	{	"Shadow"		,"Shadow"		,0x00000010		,	9	,BROOD_CODE_TEMPSKRON },
	{	"Shadow Walker"		,"Shadow Walker"	,0x00000010		,	9	,BROOD_CODE_TEMPSKRON },
	{	"Shadow Master"		,"Shadow Master"	,0x00000010		,	9	,BROOD_CODE_TEMPSKRON },

	{	"Shaman"		,"Shaman"		,0x00100000		,	10	,BROOD_CODE_MORAYION  , 200,2,1,2,3,2,1},
	{	"Summoner"		,"Summoner"		,0x00100000		,	10	,BROOD_CODE_MORAYION },
	{	"High Summoner"		,"High Summoner"	,0x00100000		,	10	,BROOD_CODE_MORAYION },
	{	"Soul Catcher"		,"Soul Catcher"		,0x00100000		,	10	,BROOD_CODE_MORAYION },
	{	"Soul Master"		,"Soul Master"		,0x00100000		,	10	,BROOD_CODE_MORAYION },

	{	"MartialArtist"		,"Martial Artist"	,0x00000020		,	11	,BROOD_CODE_TEMPSKRON , 80,3,3,2,1,2,0},
	{	"Striker"		,"Striker"		,0x00000020		,	11	,BROOD_CODE_TEMPSKRON },
	{	"Trick Artist"		,"Trick Artist"		,0x00000020		,	11	,BROOD_CODE_TEMPSKRON },
	{	"Master"		,"Master"		,0x00000020		,	11	,BROOD_CODE_TEMPSKRON },
	{	"Grand Master"		,"Grand Master"		,0x00000020		,	11	,BROOD_CODE_TEMPSKRON },

	{	"","",0,0,0 }
};