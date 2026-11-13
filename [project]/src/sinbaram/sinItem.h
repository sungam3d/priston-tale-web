/*----------------------------------------------------------------------------*
*	파일명 :  sinItem.h
*	하는일 :  아이템 관리 
*	작성일 :  최종업데이트 12월 
*	적성자 :  박상열 
*-----------------------------------------------------------------------------*/	
#ifndef	_SINITEM_HEADER_

#define _SINITEM_HEADER_

#define MAX_ITEM					1400 // 1072 + the gear of levels 106-145 (221, sinItem_NewGear.h) // 983 + the daggers, phantoms and vambraces (89, tier 5 project) //071203 (700->720 ) pluto
										 //작성자 : 박재원,  작성일 : 08.04.07
										 //내  용 : 클랜치프 링 20개 추가(720->740)					
										 // 박재원 - 수표 삭제 후 746 부터 시작
										 // 박재원 - 슈퍼암릿(2), 스피드부츠(2), 스킬마스터(3), 에이징마스터(3)
										 // 이동상점(1), 경험치증가포션(3), 스탯초기 스톤(5)   746 -> 765
										 // 박재원 - 피닉스 펫(30일) 추가 765 -> 766
										 // 박재원 - 이벤트 링, 이벤트 아뮤렛 추가 766 -> 768
										 // 박재원 - 엘더 코퍼 오어, 슈퍼 에이징 스톤, 테이와즈 추가 768 -> 771
										 // 박재원 09.10.13 부스터 아이템(생명력, 기력, 근력) 9종 추가(771->780)
										 // 박재원 09.12.04 빌링 매직 포스 추가(780->783)
										 // 박재원 09.11.04 매직 쉘텀(14종), 매직 포스(14종), 일반 포스(2종 벨룸, 오르도) 추가(783->813) 매직 포스 추가
										 // 박재원 09.12.01 빌링 도우미 펫 추가(813->817)
										 // 박재원 09.12.08 산타링, 산타아뮬렛 추가(817->819)
										 // 박재원 09.12.17 이벤트링(1시간, 1일), 이벤트아뮬렛(1시간, 1일), 밤하늘의 별(1종) 추가(819->824)
										 // 박재원 10.02.02 보스 몬스터 링(퓨리링, 바벨링) 추가(824->826)
										 // 장별 10.02.01 발렌타인 초콜릿 추가(826->827)
										 // 장별 10.02.02 Heart Ring(7일) 추가(827->828)
										 // 박재원 10.02.12 오르도 쉘텀(os114) 추가 (828->829) 
									 	 // 박재원 10.02.12 알파벳 추가 (829->836) 
									   	 // 장별 10.02.16 화이트콘토라스트 이벤트 수정, 룬, 제작서, 눈꽃 목걸이  추가(836->847)
										 // 장별 10.02.16 빌링 도우미 펫 1일 추가(4마리)(847->851)
										 // 장별 10.03.01 하트아뮬렛(7일) 추가, 캔디데이즈 캔디(851->853)
										 // 장별 10.03.01 피닉스(1시간), 경험치 2배 증가약(1시간), 스피드부츠1일, 슈퍼암릿1일 추가 (853->857)
										 // 장별 10.03.01 빌링도우미 4마리 추가 // 장별 - 빌링 도우미 펫 추가(7일) (857->861)
										 // 박재원 10.03.08 에이징 마스터(2차) 아이템 추가 3종(D,E,F) (864->865)
										 // 박재원 10.03.26 호랑이 캡슐 추가(864->865)	
										 // 장별 10.03.29 대장장이의 혼 아이템(865->875)
										 // 장별 10.04.01 대장장이의 혼 보상 옷 아이템(875->879)
									   	// 장별 10.04.13 그라비티 스크롤, 매지컬그린(에메랄드, 비취), 메모리얼 블레이드, 기간제 4종(879->890)
										// 장별 10.05.04 슈퍼 에이지어 스톤 1.5, 기간제 4종(890->895)
										// 장별 10.06.10 뱀피릭커스핏EX, 카라의 눈물(894->898) 
										// 박재원 10.06.10 2010 월드컵 이벤트 (축구공 포션 추가) (898->899)
										// 박재원 10.07.13 믹스쳐 리셋 스톤, 리스펙 스톤, 근력 리듀스 포션(1일,7일), 필드 코어(1일, 7일)수영복 추가 (899 -> 909)
										// 장별 10.07.15  신규아이템 17종, 수박(909->927) 
										// 장별 10.08.10 소울스톤 9개, 링과 아뮬렛 9개 추가( 927->945 )
										// 장별 10.09.03 복날 이벤트 링, 아뮬 3개 추가(945 -> 948 )
			
#define ITEMSIZE					22   //아이템 이미지의 크기를 나누는 값 

#define CHECK_COPY_ITEM				1	//복사아이템
#define CHECK_GIVE_ITEM				2	//증정용아이템 


/*---------------------------------------------*
*			아이템이 놓여질 위치를 정의한다 '
*----------------------------------------------*/
#define INVENTORY_POS_BOX			    0x00000001  
#define INVENTORY_POS_LHAND				0x00000002 
#define INVENTORY_POS_RHAND				0x00000004 
#define INVENTORY_POS_ARMOR				0x00000008 
#define INVENTORY_POS_BOOTS				0x00000010 
#define INVENTORY_POS_GLOVES			0x00000020 
#define INVENTORY_POS_LRING				0x00000040 
#define INVENTORY_POS_RRING				0x00000080 
#define INVENTORY_POS_SHELTOM			0x00000100 
#define INVENTORY_POS_AMULET			0x00000200 
#define INVENTORY_POS_ARMLET			0x00000800
#define INVENTORY_POS_TWO_HAND			0x00001000
#define INVENTORY_POS_POTION			0x00002000
// the 3215 inventory: the costume slot (shown with the up arrow or T) and two premium ring slots
#define INVENTORY_POS_COSTUME			0x00004000
#define INVENTORY_POS_PRING1			0x00008000
#define INVENTORY_POS_PRING2			0x00010000


/*---------------------------------------------*
*			아이템의 클래스를  정의한다 
*----------------------------------------------*/

#define ITEM_CLASS_WEAPON_ONE			INVENTORY_POS_RHAND 
#define ITEM_CLASS_WEAPON_TWO			(INVENTORY_POS_RHAND |INVENTORY_POS_LHAND) //투핸드는 어디에나 들어갈수있다 
#define ITEM_CLASS_ARMOR				INVENTORY_POS_ARMOR 
#define ITEM_CLASS_BOOTS				INVENTORY_POS_BOOTS 
#define ITEM_CLASS_GLOVES				INVENTORY_POS_GLOVES 
#define ITEM_CLASS_SHIELDS				INVENTORY_POS_LHAND
#define ITEM_CLASS_RING					(INVENTORY_POS_LRING | INVENTORY_POS_RRING)
#define ITEM_CLASS_SHELTOM				INVENTORY_POS_SHELTOM 
#define ITEM_CLASS_AMULET				INVENTORY_POS_AMULET 
#define ITEM_CLASS_ARMLET				INVENTORY_POS_ARMLET 
#define ITEM_CLASS_MAGICIAL_STUFFS		INVENTORY_POS_LHAND
#define ITEM_CLASS_POTION				INVENTORY_POS_POTION
#define ITEM_CLASS_ECORE				INVENTORY_POS_BOX
#define ITEM_CLASS_QUEST				INVENTORY_POS_BOX
#define ITEM_CLASS_FORCEORB				INVENTORY_POS_BOX
#define ITEM_CLASS_SEEL					INVENTORY_POS_BOX
#define ITEM_CLASS_COSTUME				INVENTORY_POS_COSTUME		// costumes (sinItem_Costume.h)

//발사형인지 근접형인지 
#define ITEM_WEAPONCLASS_NOT_SHOOTING	1 //근접형 
#define ITEM_WEAPONCLASS_SHOOTING		2 //발사형 
#define ITEM_WEAPONCLASS_CASTING		3 //법사형

//무기의 클래스를 정의한다 
/*---------------------------------------------*
*			(무기)1차 카테고리 					    
*----------------------------------------------*/
#define sinWA1			0x01010000	//Axes
#define sinWC1			0x01020000	//Claws
#define sinWH1			0x01030000	//Hammer & So On
#define sinWM1			0x01040000	//Magicial Stuffs
#define sinWP1			0x01050000	//Poles & Spears
#define sinWS1			0x01060000	//Shooters
#define sinWS2			0x01070000	//Swords
#define sinWT1			0x01080000	//Throwing Arms
#define sinWN1			0x01090000	//Phantoms (the Shaman)
#define sinWD1			0x010A0000	//Daggers (the Assassin)
#define sinWV1			0x010B0000	//Vambraces (the Martial Artist)

// Daggers are the Assassin's alone, phantoms the Shaman's, vambraces the Martial Artist's, who
// wields no other weapon (classes 9 / 10 / 11, smPacket.h JOBCODE_*).
inline int CanClassUseWeapon( int JobCode, DWORD dwItemCode )
{
	DWORD dwFamily = dwItemCode & 0xFFFF0000;
	JobCode &= 0xFFFF;
	if ( dwFamily==sinWD1 ) return JobCode==9;
	if ( dwFamily==sinWN1 ) return JobCode==10;
	if ( dwFamily==sinWV1 ) return JobCode==11;
	if ( JobCode==11 && (dwItemCode&0xFF000000)==0x01000000 ) return 0;
	return 1;
}

/*---------------------------------------------*
*			(방어구)1차 카테고리 					    
*----------------------------------------------*/

#define sinDA1			0x02010000	//Armor
#define sinDB1			0x02020000	//Boots
#define sinDG1			0x02030000	//Gloves
#define sinDS1			0x02040000	//Shields
#define sinDA2			0x02050000	//Armor
// costumes (the 3215 codes): CA1xx armours, CA2xx robes, CA5xx/CA6xx outfits. Not 0x0201/0x0204,
// so record.cpp does not take one in the costume slot for a misplaced armour
#define sinCA1			0x02060000
#define sinCA2			0x02070000
#define sinCA5			0x02100000
#define sinCA6			0x02110000


/*---------------------------------------------*
*			(장신구류)1차 카테고리 					    
*----------------------------------------------*/

#define sinOA1			0x03010000	//Amulet
#define sinOA2			0x03020000	//Armlet
#define sinOM1			0x03030000	//Magicial Stuffs
#define sinOR1			0x03040000	//Ring
#define sinOR2			0x03050000	//Ring2
#define sinOS1			0x02350000	//Sheltom
#define sinFO1			0x03060000  //Force Orb
#define sinSE1			0x03070000  //Seel
#define sinPR1			0x03080000  // 광석 pluto
#define sinPR2			0x03090000  // 수정 pluto
#define sinPR3			0x03100000  // 룬(A) pluto
#define sinPR4			0x03110000  // 룬(B) pluto
/*---------------------------------------------*
*			(물약)1차 카테고리 					    
*----------------------------------------------*/
#define sinPM1			0x04010000  //Mana
#define sinPL1			0x04020000  //Life
#define sinPS1			0x04030000  //Stamina

/*---------------------------------------------*
*			(돈)1차 카테고리 					    
*----------------------------------------------*/
#define sinGG1			0x05010000 //Gold
#define sinBS1			0x05020000 //벨라토스톤

/*---------------------------------------------*
*			(귀환)1차 카테고리 					    
*----------------------------------------------*/
#define sinEC1			0x06010000

/*---------------------------------------------*
*			  퀘스트 아이템 
*----------------------------------------------*/

#define sinQT1			0x07010000


/*---------------------------------------------*
*			  이벤트 아이템 
*----------------------------------------------*/
#define sinSP1			0x08010000 
#define sinGP1			0x08020000		//0x09010000 으로하니 문제이네 그려.. (& sinITEM_MASK2 가 안먹음)
#define sinQW1			0x08030000		//워프게이트 윙
#define sinGF1			0x08040000		//별상품권
#define sinPZ1			0x08060000		//아처 퍼즐
#define sinPZ2			0x08070000		//킹호피 퍼즐
#define sinCH1			0x08080000		//발렌타인데이 초콜렛
#define sinSD2			0x08090000		//Sod2 발생 아이템
#define sinBC1          0x080A0000      //공성전 발생 아이템
#define sinBI1          0x080B0000      //유료구매 아이템  
#define sinBI2          0x080C0000      //유료구매 아이템(패키지 캐쉬 아이템 )  
#define sinGP2			0x080D0000		// 장별 - 소울스톤


/*---------------------------------------------*
*			  Make Item
*----------------------------------------------*/
#define sinMA1			0x09010000 
#define sinMA2			0x09020000


/*---------------------------------------------*
*			  제작 재료 아이템
*----------------------------------------------*/
// pluto 제작
#define sinWR1			0x0A010000 // 무구 레시피
#define sinDR1			0x0A020000 // 갑주 레시피

#define EXTREME_RECIPE		1		// 잊혀진 레시피
#define ANCIENT_RECIPE		2		// 고대의 레시피
#define	MINOTAUR_RECIPE		3		// 대지의 레시피
#define DOOM_RECIPE			4		// 어둠의 레시피
#define SALAMANDER_RECIPE	5		// 화명의 레시피
#define WYVERN_RECIPE		6		// 바람의 레시피
#define PHOENIX_RECIPE		7		// 태양의 레시피
#define FRENZY_RECIPE		8		// 광포한 레시피
#define HEAVENS_RECIPE		9		// 천상의 레시피 
#define SNOW_RECIPE			10		// 포설의 레시피
#define MEMORY_RECIPE		11		// 기억의 레시피 // 장별 - 대장장이의 혼

/*---------------------------------------------*
*			아이템 2차 카테고리(아이템별로 세분화되 구분 ) 
*----------------------------------------------*/
#define sin00			0x00000000	
#define sin01			0x00000100	
#define sin02			0x00000200
#define sin03			0x00000300
#define sin04			0x00000400
#define sin05			0x00000500
#define sin06			0x00000600
#define sin07			0x00000700
#define sin08			0x00000800
#define sin09			0x00000900
#define sin10			0x00000A00
#define sin11			0x00000B00
#define sin12			0x00000C00
#define sin13			0x00000D00
#define sin14			0x00000E00
#define sin15			0x00000F00
#define sin16			0x00001000
#define sin17			0x00001100
#define sin18			0x00001200
#define sin19			0x00001300
#define sin20			0x00001400
#define sin21			0x00001500
#define sin22			0x00001600
#define sin23			0x00001700
#define sin24			0x00001800
#define sin25			0x00001900 //16
#define sin26			0x00002A00
#define sin27			0x00002B00
#define sin28			0x00002C00
#define sin29			0x00002D00
#define sin30			0x00002E00
#define sin31			0x00002F00
#define sin32			0x00003000
#define sin33			0x00003100
#define sin34			0x00003200
#define sin35			0x00003300
#define sin36			0x00003400
#define sin37			0x00003500
#define sin38			0x00003600
#define sin39			0x00003700
#define sin40			0x00003800
#define sin41			0x00003900
#define sin42			0x00003A00
#define sin43			0x00003B00
#define sin44			0x00003C00
#define sin45			0x00003D00
#define sin46			0x00003E00
#define sin47			0x00003F00
#define sin48			0x00004000	// pluto 추가
#define sin49			0x00004100	// pluto 추가
#define sin50			0x00004200	// pluto 추가
#define sin51			0x00004300	// 박재원 - 피닉스 펫(30일) 추가 
#define sin52			0x00004400
#define sin53			0x00004500
#define sin54			0x00004600
#define sin55			0x00004700
#define sin56			0x00004800
#define sin57			0x00004900
#define sin58			0x00004A00
#define sin59			0x00004B00
#define sin60			0x00004C00  // 박재원 - 엘더 코퍼 오어 추가
#define sin61			0x00004D00  // 박재원 - 슈퍼 에이징 스톤 추가
#define sin62			0x00004E00
#define sin63			0x00004F00
#define sin64			0x00005000
#define sin65			0x00005100
#define sin66			0x00005200
#define sin67			0x00005300
#define sin68			0x00005400
#define sin69			0x00005500
#define sin70			0x00005600
#define sin71			0x00005700
#define sin72			0x00005800
#define sin73			0x00005900 
#define sin74			0x00005A00 
#define sin75			0x00005B00 
#define sin76			0x00005C00 
#define sin77			0x00005D00 
#define sin78			0x00005E00 
#define sin79			0x00005F00 
#define sin80			0x00006000 
#define sin81			0x00006100 // 박재원 - 에이징 마스터(2차) 아이템 추가
#define sin82			0x00006200 // 박재원 - 에이징 마스터(2차) 아이템 추가
#define sin83			0x00006300 // 박재원 - 에이징 마스터(2차) 아이템 추가
#define sin84			0x00006400 // 장별 - 그리바티 스크롤
#define sin85			0x00006500
#define sin86			0x00006600 
#define sin87			0x00006700 
#define sin88			0x00006800 
#define sin89			0x00006900 
#define sin90			0x00006A00 
#define sin91			0x00006B00 
#define sin92			0x00006C00 
#define sin93			0x00006D00 
#define sin94			0x00006E00 
#define sin95			0x00006F00 
#define sin96			0x00007000 
#define sin97			0x00007100 
#define sin98			0x00007200 









/*---------------------------------------------*
*			아이템 사운드 
*----------------------------------------------*/

#define		SIN_SOUND_AXES				1
#define		SIN_SOUND_CLAWS				2
#define		SIN_SOUND_HAMMER			3
#define		SIN_SOUND_MAGICIAL		 	4
#define		SIN_SOUND_POLES				5
#define		SIN_SOUND_SHOOTERS			6
#define		SIN_SOUND_SWORDS			7
#define		SIN_SOUND_THROWING			8
#define		SIN_SOUND_ARMOR				9
#define		SIN_SOUND_BOOTS				10
#define		SIN_SOUND_GLOVES			11
#define		SIN_SOUND_SHIELDS			12
#define		SIN_SOUND_AMULET			13
#define		SIN_SOUND_Armlet			14
#define		SIN_SOUND_RING				15
#define     SIN_SOUND_SHELTOM			16
#define     SIN_SOUND_POTION			17
#define     SIN_SOUND_COIN				18
#define     SIN_SOUND_STAFF				19
#define		SIN_SOUND_EAT_POTION		20
#define		SIN_SOUND_SHOW_INTER		21
#define		SIN_SOUND_REPAIR_ITEM		22
#define		SIN_SOUND_FAILD_MIXITEM		23
#define		SIN_SOUND_ARMOR2			24
#define		SIN_SOUND_EAT_POTION2		25


/** 원소 저항력 **/
#define   sITEMINFO_BIONIC			0	//생체 
#define   sITEMINFO_EARTH			1	//대자연 
#define   sITEMINFO_FIRE			2	//불 
#define	  sITEMINFO_ICE				3	//냉기 
#define   sITEMINFO_LIGHTING		4	//번개 
#define   sITEMINFO_POISON			5	//독 
#define   sITEMINFO_WATER			6	//물 
#define   sITEMINFO_WIND			7	//바람 
#define	  sITEMINFO_NONE			15	//속성 무시

//////////* 아이템 종류 *////////
#define ITEM_KIND_NORMAL			0   //기본 아이템 
#define ITEM_KIND_CRAFT				1   //조합 아이템 
#define ITEM_KIND_AGING				2   //에이징 아이템 
#define ITEM_KIND_QUEST				3	//퀘스트용 아이템 
#define ITEM_KIND_EVENT				4	//이벤트 아이템 
#define ITEM_KIND_MAKE_MAIN			5	//이벤트 아이템 
#define ITEM_KIND_MAKE_ELEMENT		6	//이벤트 아이템 
#define ITEM_KIND_QUEST_WEAPON		7	//이벤트 아이템 
#define ITEM_KIND_SPECIAL			8	//SpecialItemFlag[1] 에 플랙등록시 스페셜아이템으로 구분

/////////////////////////////////


#define   SIN_ADD_FIRE				0x00000001	//불
#define   SIN_ADD_ICE				0x00000002	//얼음
#define   SIN_ADD_LIGHTNING			0x00000004	//번개
#define   SIN_ADD_POISON			0x00000008	//독
#define   SIN_ADD_BIO				0x00000010	//물
#define   SIN_ADD_CRITICAL			0x00000020	//크리티컬
#define   SIN_ADD_ATTACK_RATE		0x00000040	//명중력
#define   SIN_ADD_DAMAGE_MIN		0x00000080	//최소공격력
#define   SIN_ADD_DAMAGE_MAX		0x00000100	//최대공격력
#define   SIN_ADD_ATTACK_SPEED		0x00000200	//공격속도
#define   SIN_ADD_ABSORB			0x00000400	//흡수력
#define   SIN_ADD_DEFENCE			0x00000800	//방어력
#define   SIN_ADD_BLOCK_RATE		0x00001000	//블럭율
#define   SIN_ADD_MOVE_SPEED		0x00002000	//이동속도
#define   SIN_ADD_LIFE				0x00004000	//최대생명력
#define   SIN_ADD_MANA				0x00008000	//최대기력
#define   SIN_ADD_STAMINA			0x00010000	//최대근력
#define   SIN_ADD_LIFEREGEN			0x00020000  //생명력재생
#define   SIN_ADD_MANAREGEN			0x00040000  //기력력재생
#define   SIN_ADD_STAMINAREGEN		0x00080000  //근력력재생


/*----------------------------------------------------------------------------*
*						 아이템 속성 
*-----------------------------------------------------------------------------*/
struct	sITEM_CREATE {
	DWORD	Head;			//헤더
	DWORD	dwVersion;		//생성 버전
	DWORD	dwTime;			//생성시간
	DWORD	dwChkSum;		//아이템 체크섬
};

struct sITEM_SPECIAL {

	float	Add_fAbsorb;			//흡수력 
  	int     Add_Defence;			//방어력 
	float   Add_fSpeed;				//이동 속도 
	float   Add_fBlock_Rating;		//(방패)블럭율 
	int     Add_Attack_Speed;		//공격속도
	int     Add_Critical_Hit;		//1.5배 데미지확율
	int     Add_Shooting_Range;		//사정거리 
	float   Add_fMagic_Mastery;     //마법숙련도 
	short	Add_Resistance[8];		//원소에대한 저항력 

	////////////////////////////////////////////////

	short	Lev_Attack_Resistance[8];	//원소에대한 공격력
	int		Lev_Mana;				//마나회복(최소)(최대)
	int		Lev_Life;				//라이프회복(최소)(최대) 
	int     Lev_Attack_Rating;		//명중력 
	short	Lev_Damage[2];			//공격력

	float	Per_Mana_Regen;			//마나 재생 6
	float	Per_Life_Regen;			//라이프 재생 
	float	Per_Stamina_Regen;		//스테미나 재생

	//////////////////////////////////////////////////

	DWORD	dwTemp[32];
};


struct sITEMINFO{

	DWORD	dwSize;						//아이템 구조 크기
	
	sITEM_CREATE	ItemHeader;			//아이템 생성 정보

	short	Durability[2];				//내구력

	//////// 실제 아이템 데이타정보가 들어갑니다 /////////

	/*-------------------------*
	*		 아이템 정보	
	*--------------------------*/
	DWORD	CODE;				//아이템 코드 	
	char	ItemName[32];		//아이템 이름 	

	/*-------------------------*
	*		공통 사항 	
	*--------------------------*/
	int     Weight;				//무게 		
	int     Price;				//가격

	/////////////////////////////////////////
	int		Index;				//인덱스
	int		PotionCount;		//포션카운터
	////////////////////////////////////////

	/**원소**/
	short	Resistance[8];		//원소에대한 저항력 
	/********/

	int		Sight;				//시야

	DWORD	Temp0;

	/*-------------------------*
	*		 공격성능 
	*--------------------------*/
	short	Damage[2];			//공격력 
	int     Shooting_Range;		//사정거리 
	int     Attack_Speed;		//공격속도 
	int     Attack_Rating;		//명중력 
	int     Critical_Hit;		//1.5배 데미지확율  

	/*-------------------------*
	*		 방어성능 
	*--------------------------*/

	float	fAbsorb;			//흡수력 
  	int     Defence;			//방어력 
	float   fBlock_Rating;		//(방패)블럭율 

	/*-------------------------*
	*		 이동성능 
	*--------------------------*/
	float     fSpeed;				//이동 속도 

	/*-------------------------*
	*		 팔찌 성능 
	*--------------------------*/

	int		Potion_Space;		//소켓 공간 할당 
	
	/**특수능력**/
	float   fMagic_Mastery;     //마법숙련도 
	float   fMana_Regen;		//마나 재생 
	float   fLife_Regen;		//라이프 재생 
	float   fStamina_Regen;	    //스테미나 재생 
	float   fIncrease_Life;     //라이프 최대치상승 
	float   fIncrease_Mana;     //마나 최대치상승
	float   fIncrease_Stamina;  //스테미나 최대치상승

	/************/

	/*-------------------------*
	*		요구특성 
	*--------------------------*/
	int     Level;				//레벨 
	int     Strength;			//힘 	
	int     Spirit;				//정신력 
	int     Talent;				//재능 
	int     Dexterity;			//민첩성 
	int     Health;				//건강 

	/*-------------------------*
	*		 회복약 
	*--------------------------*/
	short   Mana[2];            //마나회복(최소)(최대)
	short   Life[2];            //라이프회복(최소)(최대) 
	short   Stamina[2];         //스테미너회복(최소)(최대)
	
	/*-------------------------*
	*		 .....
	*--------------------------*/
	int		Money;				//보유한 돈
	int     NotUseFlag;         //사용불가능 플랙

	DWORD	BackUpKey;			//백업헤더
	DWORD	BackUpChkSum;		//백업 아이템 체크섬

	short   ScaleBlink[2];		//스케일 변경 
	DWORD	UniqueItem;			//유니크
	short	EffectBlink[2];		//색상표시 주기 (0) 이펙트 첵크섬(1)
	short	EffectColor[4];		//색상표시모드

	DWORD	DispEffect;			//아이템 표시 이펙트 (예정)


	/*-------------------------*
	*		캐릭터 특화아이템    (현재 미설정)
	*--------------------------*/

	DWORD			JobCodeMask;		//특화 직업	(직업별 비트 마스크)
	sITEM_SPECIAL	JobItem;			//특화 아이템 성능

	DWORD   ItemKindCode;		//아이템 종류 0이면 노멀 아이템 
	DWORD   ItemKindMask; 

	short   ItemAgingNum[2];	//0은 에이징+몇 1은 에이징이 되고있나 아닌가?
	short   ItemAgingCount[2];	//0아이템 숙련 카운트 1숙련치 최대 
	short   ItemAgingProtect[2];//에이징 보호 

	short   SpecialItemFlag[2];   //0은 저주아이템 플렉 1 은 퀘스트같은 곳에 사용되는 특수아이템

	DWORD	dwCreateTime;		//생성 시간
	DWORD	dwTemp[9];			//예비

};

/*----------------------------------------------------------------------------*
*						    struct  아이템
*-----------------------------------------------------------------------------*/
struct sITEM{

	DWORD	CODE;
	char	ItemNameIndex[32];	//아이템 이름 	

	/*-------------------------*
	*	초기화시 셋팅한다 
	*--------------------------*/
	char	LastCategory[16];   //최종 카테고리 		
	int		w;
	int     h;			        //아이템 사이즈 
	char	ItemFilePath[64];   //아이템 로드시 파일 경로 
	DWORD	Class;		        //아이템의 종류
	char    DorpItem[64];       //떨어졌을때 아이템 
	DWORD   SetModelPosi;		//아이템이 셋팅될 위치 
	int		SoundIndex;			//아이템의 사운드 
	int     WeaponClass;		//발사형인지 근접형인지 
	
	
	//--------------------------

	int Flag;			        //아이템 플랙 
	int x,y;			        //아이템이 그려질 좌표 
	int SetX,SetY;				//아이템이 세팅될 곳의 넣어준다 

	LPDIRECTDRAWSURFACE4	lpItem; //그림을 로드에 필요한 포인터 
	int		ItemPosition;       //아이템이 인벤토리상에 존재할때 포지션을 준다 (오른손 , 왼손 ,양손 ,값옷 ..)

	int     PotionCount;		//물약일때만 카운트한다 
	int     NotUseFlag;			//요구치가 충족되지 않을때 플랙을 준다 
	int     SellPrice;			//아이템이 팔릴 가격

	int     OldX,OldY;          //아이템이 예전에 세팅됬던 좌표  
	LPDIRECTDRAWSURFACE4	lpTempItem; //그림을 로드에 필요한 포인터 
	//int		LimitTimeFlag;		    //유통기한 

	sITEMINFO sItemInfo;

};

struct sITEMPRICE{
	int PureSellPrice;
	int SellPrice;
	int RepairCost;

};

//아이템 요구치 플랙 
struct sITEMREQUIRE{

	int     rLevel;				//레벨 
	int     rStrength;			//힘 	
	int     rSpirit;			//정신력 
	int     rTalent;			//재능 
	int     rDexterity;			//민첩성 
	int     rHealth;			//건강 

};
/*----------------------------------------------------------------------------*
*						    class   아이템
*-----------------------------------------------------------------------------*/
class cITEM
{	

public:

	char *DropItemPath;
	char *CharSetItemPath;

	int MatItemInfoBox;

	int MatItemInfoBox_TopLeft;
	int MatItemInfoBox_TopRight;
	int MatItemInfoBox_TopCenter;

	int MatItemInfoBox_BottomLeft;
	int MatItemInfoBox_BottomRight;
	int MatItemInfoBox_BottomCenter;

	int MatItemInfoBox_Left;
	int MatItemInfoBox_Right;
	int MatItemInfoBox_Center;


	LPDIRECTDRAWSURFACE4	AgingBar;
	LPDIRECTDRAWSURFACE4	AgingGage;
	LPDIRECTDRAWSURFACE4	AgingBar2;

public:
	cITEM();
	~cITEM();

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


	int ShowItemInfo(sITEM *pItem , int Flag,int Index=0); //정보를 찾는다  1은 상점 아이템 2는 인벤 아이템 

	int ShowItemInfo2(sITEM *pItem ,int Flag,int Index=0); //정보를 찾는다  1은 상점 아이템 2는 인벤 아이템 (두개로 나눴다)

	int DrawItemText(); //아이템 정보 텍스트 

	POINT SetTextXY(int x , int y, int len); //텍스트가 놓일 위치를 보정한다  

	int SearchSpecialItemJob(DWORD SpecialJob_CODE); //특화 캐릭터 전용 아이템  
	
	int GetInfoBoxSize(sITEM *pItem ,int Col); //박스의 사이즈를 구한다 

	/////////////////// 아이템 테이블의 조작을 방지한다 
	int CheckItemTable();


	/////// 아이템 유통기한 
	int GetItemLimitTime(sITEM *pItem);

	//아이템의 좌표를 얻어온다
	void GetMousePos(int *MouseX , int *MouseY);

	//가제 테스트
	void SetItemInfo(char *Buff);
};

extern cITEM cItem;	 
extern sITEM sItem[MAX_ITEM];

extern sITEM TempItem;      //잠시 저장될 아이템 구조체 변수 
extern sITEM MouseItem;	    //마우스에 옮겨질 아이템 

extern int sinShowItemInfoFlag;  //아이템 정보를 보여준다 
extern DWORD sinItemTime;   //아이템 유통기간 

extern int NotSell_Item_CODECnt;
extern int NotSell_Item_MASKCnt;
extern int NotSell_Item_KINDCnt;

extern int NotDrow_Item_CODECnt;
extern int NotDrow_Item_MASKCnt;
extern int NotDrow_Item_KINDCnt;

extern int NotSet_Item_CODECnt;
extern int NotSet_Item_MASKCnt;
extern int NotSet_Item_KINDCnt;

extern DWORD NotSell_Item_CODE[];
extern DWORD NotSell_Item_MASK[];
extern DWORD NotSell_Item_KIND[];

extern DWORD NotDrow_Item_CODE[];
extern DWORD NotDrow_Item_MASK[];
extern DWORD NotDrow_Item_KIND[];

extern DWORD NotSet_Item_CODE[];
extern DWORD NotSet_Item_MASK[];
extern DWORD NotSet_Item_KIND[];

//프리미엄 아이템 관련 
//파티복 추가 돼는 아머 코드
//DWORD   dwPremiumItemArmorCODE[] ={
//	(sinDA1|sin31) , (sinDA1|sin32) , (sinDA1|sin33) , (sinDA1|sin34) , (sinDA1|sin35) ,
//};




#endif