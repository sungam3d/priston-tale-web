#ifndef _HO_ASSA_PARTICLE_EFFECT_
#define _HO_ASSA_PARTICLE_EFFECT_




//비어 있는 버퍼를 리턴하는 함수..(함수 안에서 동적으로 생성 하면 sin의 함수를 고쳐야 하므로)
int		GetAssaEffect();
int		StopAssaCodeEffect(smCHAR *pChar, int code, int setTime = 0);
//BOOL	GetAssaCodeEffect(smCHAR *pChar, int code);
bool	AddAssaCodeEffectTime(smCHAR *pChar, int code, int time);

BOOL	GetAssaUseEffect(smCHAR *pChar, int code);
bool	GetAssaContinueSkill(int skillCode);			//현재 유지 스킬이 적용 되고 있는지 상열씨 쪽 검사 하는 함수
//모이는 라인 파티클...
extern int AssaParticle_LineIn(POINT3D *pPosi , char *texName = NULL, int continueTime = 15, int speed = 0);
extern int AssaParticle_LineOut(POINT3D *pPosi , char *texName = NULL, int continueTime = 10, int speed = 0);
extern int AssaParticle_Star(POINT3D *pPosi);

//Paladin HolyValor용 (Jang: 파티장 Member: 파티원)
extern int AssaParticle_HolyValor_Jang(smCHAR *pChar, int maxTime = 0);
extern int AssaParticle_HolyValor_Member(smCHAR *pChar, int maxTime = 0);

//Paladin Drastic Spirit용 (Jang: 파티장 Member: 파티원)
extern int AssaParticle_DrasticSpirit_Jang(smCHAR *pChar, int maxTime = 0);
extern int AssaParticle_DrasticSpirit_Member(smCHAR *pChar, int maxTime = 0);

//Valkyrie (Windy, TwistJavelin, FireJavelin)
extern int AssaParticle_Windy(smCHAR *pChar, int maxTime = 0);
extern int AssaParticle_TwistJavelinShot(POINT3D *pPosi, POINT3D *pDesti);
extern int AssaParticle_FireJavelin(POINT3D *pPosi, POINT3D *pDesti);
extern int AssaParticle_FireJavelin(smCHAR *pChar, POINT3D *pDesti);


//Saintess (DivineLighting)
//extern int AssaParticle_DivineLighting(POINT3D *pPosi, POINT3D *pDesti);
extern int AssaParticle_DivineLighting(smCHAR *pChar);
extern int AssaParticle_Sprak(POINT3D *pPosi);

//Wizard (ColumnOfWater, EnchantWeapon, DeadLay)
extern int AssaParticle_ColumnOfWater(POINT3D *pPosi, POINT3D *pDesti, int level = 1);
extern int AssaParticle_ColumnOfWaterHit(POINT3D *pPosi);

extern int AssaParticle_EnchantWeaponIceJang(smCHAR *pChar, int maxTime = 0);
extern int AssaParticle_EnchantWeaponFireJang(smCHAR *pChar, int maxTime = 0);
extern int AssaParticle_EnchantWeaponLightJang(smCHAR *pChar, int maxTime = 0);


extern int AssaParticle_EnchantWeaponJinMember(smCHAR *pChar);
extern int AssaParticle_EnchantWeaponIceMember(smCHAR *pChar, int maxTime = 0);
extern int AssaParticle_EnchantWeaponFireMember(smCHAR *pChar, int maxTime = 0);
extern int AssaParticle_EnchantWeaponLightMember(smCHAR *pChar, int maxTime = 0);

extern int AssaParticle_LightShot(POINT3D *pPosi, POINT3D *pDesti);
extern int AssaParticle_FireShot(POINT3D *pPosi, POINT3D *pDesti);
extern int AssaParticle_IceShot(POINT3D *pPosi, POINT3D *pDesti);

extern int AssaParticle_LightHit1(POINT3D *pPosi);
extern int AssaParticle_IceHit1(POINT3D *pPosi);
extern int AssaParticle_FireHit1(POINT3D *pPosi);
extern int AssaParticle_FireHit2(POINT3D *pPosi);

extern int AssaParticle_BigFireHit1(POINT3D *pPosi);

extern int AssaParticle_DeadLay(POINT3D *pPosi, POINT3D *pDesti);
extern int AssaParticle_HolyMind_Attack(smCHAR *pChar, int liveCount);

//int AssaParticle_AgingBody(smCHAR *pChar);

extern int StartEffect_Serqbus_Skill1(smCHAR *pChar, int liveCount);
extern int StartEffect_Serqbus_Skill2(smCHAR *pChar, int liveCount);
extern int StartEffect_Serqbus_Skill3(smCHAR *pChar, int liveCount);

//Fury 관련 이펙트
int AssaParticle_Fury_Eye(smCHAR *pChar);							//눈에 붙는 이펙트
int AssaParticle_Fury_ShotFire(smCHAR *pChar, POINT3D *destPos);	//불쏘기..

int AssaParticle_Fury_RespawnMonster(POINT3D *pPosi);						//몬스터들 소환..
int AssaParticle_Fury_RespawnBody(smCHAR *pChar);                  //몬스터 손환시 퓨리의 입에서 나오는 파티클.
int AssaParticle_Fury_Skill1(smCHAR *pChar);						//영역 공격..

int AssaParticle_Fury_Skill1(POINT3D *pPosi, POINT3D *pAngle);      //영역 공격.. 옛날 버전을 위해서..

//Evil Maple관련 이펙트..
int AssaParticle_EvilMapleShot(smCHAR *pChar);

//Evil Plant관련 이펙트..
int AssaParticle_EvilPlantShot(smCHAR *pChar);

//Thorn Crawler관련 이펙트..
int AssaParticle_ThornCrawlerShot(smCHAR *pChar, POINT3D *destPos);

//Stone Golem관련 이펙트..
int AssaParticle_StoneGolemShot(smCHAR *pChar);


//Heavy Goblin관련 이펙트..
int AssaParticle_HeavyGoblinHit1(smCHAR *pChar); //위에서 내려치는 동작
int AssaParticle_HeavyGoblinHit2(smCHAR *pChar); //양사이드에서 치는 동작

//Doom Gurad관련 이펙트..
int AssaParticle_DoomGuardHit1(smCHAR *pChar); //위에서 내려치는 동작
int AssaParticle_DoomGuardHit2(smCHAR *pChar); //양사이드 치는 동작

//Snail 관련 이펙트..
int AssaParticle_SnailShot(smCHAR *pChar, POINT3D *destPos);

//Night Mare관련 이펙트..
int AssaParticle_NightMare(smCHAR *pChar);

//Ratoo 이펙스
int AssaParticle_RatooHit1(smCHAR *pChar);

//Monster 치기
int AssaParticle_Hit1(POINT3D *destPos);


//Spider 거미 뺏어 내기
int AssaParticleSpiderSkill1(smCHAR *pChar);

//공성 이펙트
void AssaParticle_CastleRed(smCHAR *pChar);
void AssaParticle_CastleGreen(smCHAR *pChar);
void AssaParticle_CastleBlue(smCHAR *pChar);

//ClanMonster 이펙트
int AssaParticleClanMonster(smCHAR *pChar);
int AssaParticleClanMonsterHit1(smCHAR *pChar);
int AssaParticleClanMonsterHit2(smCHAR *pChar);
int AssaParticle_Sprak1(POINT3D *pPosi);


//3차 스킬 파이터
void AssaParticle_Concentration(smCHAR *pChar,int liveCount);
void AssaParticle_SwiftAxe(smCHAR *pChar, int liveCount);
void AssaParticle_AvangingCrash(smCHAR* pChar);
void AssaParticle_BoneSmash1(smCHAR* pChar);
void AssaParticle_BoneSmash2(smCHAR* pChar);

//3차 스킬 파이크
void AssaParticle_VeonmSpear(smCHAR *pDest);
void AssaParticle_VeonmSpearPike(smCHAR *pChar);
void AssaParticle_Vanish(smCHAR *pChar);
void AssaParticle_ChainLance(POINT3D *pPosi);


//3차 스킬 아처
void AssaParticle_ElementalShotFire(POINT3D* pCur, POINT3D* pDest);
void AssaParticle_ElementalShotLighting(POINT3D* pCur, POINT3D* pDest);
void AssaParticle_GoldenFalcon(smCHAR *pChar, int liveCount);
void AssaParticle_BombHit1(POINT3D *pPosi);
void AssaParticle_BombShot(POINT3D* pCur, POINT3D* pDest);
void AssaParticle_Perforation(POINT3D* pCur, POINT3D* Dest);
void AssaParticle_PerforationHit(smCHAR* monster, smCHAR* character, float delay = 0.f);


//3차 스킬 메카니션
void AssaParticle_MetalArmor(smCHAR *pChar, int liveCount);
//void AssaParticle_GrandSmash(POINT3D *pPosi);
void AssaParticle_GrandSmash(smCHAR *pChar);
void AssaParticle_SparkShieldStart(smCHAR* pChar, int liveCount);
void AssaParticle_SparkShieldDefence(POINT3D *pPosi, POINT3D *pDesti);


//3차 스킬 메지션
void AssaParticle_EnergyShield(smCHAR *pChar,int liveCount);
void AssaParticle_EnergyShieldDefence(smCHAR *pChar);
void AssaParticle_Diastrophism(smCHAR *pChar);
void AssaParticle_DiastrophismPart(smCHAR *pChar);
void AssaParticle_SpiritElemental(smCHAR *pChar, int liveCount);

void AssaParticle_DancingSwordFire(smCHAR *pChar,int liveCount, int level);
void AssaParticle_DancingSwordIce(smCHAR *pChar,int liveCount, int level);

//3차 스킬 나이트
void AssaParticle_FlameBrandish(smCHAR *pChar);
void AssaParticle_FlameBrandishHit1(smCHAR *pChar);
void AssaParticle_HolyIncantationShot(smCHAR* pDestChar);
void AssaParticle_HolyIncantationPart(smCHAR* pDestChar, int liveCount);
void AssaParticle_HolyIncantationHand(smCHAR* pDestChar);

void AssaParticle_GrandCrossHorz(POINT3D* pPosi, int angleY);
void AssaParticle_GrandCrossVert(POINT3D* pPosi, int angleY);
void AssaParticle_DivineInhalationStart(smCHAR* pChar, int liveCount);
void AssaParticle_DivineInhalationPart(smCHAR* pChar, int liveCount);
void AssaParticle_DivineInhalationDefence(POINT3D *pDest);


//3차 아탈란타
void AssaParticle_SplitJavelin(POINT3D* pCur, POINT3D* pDest, int count = 2);
void AssaParticle_TriumphOfValhalla(smCHAR *pDestChar, int liveCount);
void AssaParticle_TriumphOfValhallaMember(smCHAR *pCurChar, smCHAR *pDestChar, int liveCount);
void AssaParticle_LightingJavelin(POINT3D* pCur, POINT3D* pDest);
void AssaParticle_StormJavelin(POINT3D* pCur, POINT3D* pDest);
void AssaParticle_StormJavelinHit(smCHAR* monster, smCHAR* character, float delay = 0.f);


//3차 스킬 프리스티스
void AssaParticle_VigorBall(smCHAR* pCurChar, smCHAR* pDestChar);
void AssaParticle_ResurrectionShot(smCHAR* pDestChar);
void AssaParticle_ResurrectionPart(smCHAR* pDestChar);
void AssaParticle_Extinction(smCHAR* pDestChar);
void AssaParticle_VirtualLife(smCHAR* pDestChar, int liveCount);



//Monster 파티클
void AssaParticle_MonsterPigonHand(smCHAR* pChar);

//ClassUp 파티클
void AssaParticle_ClassUpWeapon(smCHAR* pChar);
void AssaParticle_StopClassUpWeapon(smCHAR* pChar);
void AssaParticle_AgingOrb(smCHAR* pChar);		// aging +21..+25
void AssaParticle_AgingBody(smCHAR* pChar, int num);

//Iron Monster 스킬
void AssaParticle_MonsterExplision(POINT3D *pPosi);
void AssaParticle_MonsterIronSadNessHit1(smCHAR *pChar);
void AssaParticle_MonsterIronSadNessSkill1(smCHAR *pChar);
void AssaParticle_MonsterIronHyperHit1(smCHAR *pChar, POINT3D *pDest);
void AssaParticle_MonsterIronHyperSkill1(smCHAR *pChar, POINT3D *pDest);

void AssaParticle_MonsterIronChainGolem(smCHAR *pChar);
void AssaParticle_MonsterIronRunicGuardian(smCHAR *pChar, smCHAR *pDestChar);
void AssaParticle_MonsterIronMountainHit1(smCHAR *pChar);
void AssaParticle_MonsterIronMountainSkill1(smCHAR *pChar);
void AssaParticle_MonsterIronTowerGolemHit1(smCHAR *pChar);
void AssaParticle_MonsterIronTowerGolemSkill1(smCHAR *pChar);
void AssaParticle_MonsterIronHit1(POINT3D *pPosi);

void AssaParticle_MonsterIronOmuHit1(smCHAR *pChar);
void AssaParticle_MonsterIronFist(smCHAR *pChar);
void AssaParticle_MonsterIronMorgonHit(smCHAR *pChar);
void AssaParticle_MonsterAvelArcher(smCHAR *pChar, smCHAR *pDestChar);
void AssaParticle_MonsterAvelQueen(smCHAR *pChar, smCHAR *pDestChar);
void AssaParticle_MonsterDeadZoneDead(smCHAR *pChar);
void AssaParticle_MonsterHyperDead(smCHAR *pChar);

void AssaParticle_MonsterRunicGuardianHand(smCHAR *pChar);
void AssaParticle_MonsterRunicGuardianShot(smCHAR *pChar, smCHAR *pDestChar);
void AssaParticle_MonsterRunicGuardianBossLightShot(smCHAR **pCharList, int charCount);

void AssaParticle_MonsterRunicGuardianBossShot(smCHAR *pChar, int blendType = SMMAT_BLEND_INVSHADOW);
void AssaParticle_MonsterRunicGrardianBossSkill(smCHAR *pChar);
void AssaParticle_MonsterRunicGuardianBossSkillHit(POINT3D *pDest);
void AssaParticle_MonsterRunicGuardianBossWeapon(smCHAR *pChar);
void AssaParticle_MonsterRunicGuardianBossHit(smCHAR *pCurChar, smCHAR *pDestChar, smCHAR **charList = 0, int charCount = 0);

void AssaParticle_ShelltomShot(POINT3D *pPosi, POINT3D *pDest, int type = 0);
void AssaParticle_BamShot(POINT3D *pPosi, POINT3D *pDest);

void AssaParticle_ShelltomWeapon(smCHAR *pChar, int time, int code);
void AssaParticle_ShelltomWeaponHit(POINT3D *pPosi);

//Terrain 효과
void AssaParticle_TerrainShowStart();
void AssaParticle_TerrainFire(POINT3D *pCur, POINT3D *pDest, int delay = 0);
void AssaParticle_TerrainFireHit(POINT3D *pCur);

void AssaMonsterHitTest(smCHAR *pChar);

//4차 스킬
void SkillSaintGodlyShield(smCHAR *pChar, float time);
void SkillArchMageFlameWave(smCHAR *pChar);
void SkillCelestialChainLighting(smCHAR **pCharList, int charCount);
void SkillValhallaHallOfValhallaHandEffect(smCHAR *pChar, float time);
void SkillImpulsionLight(smCHAR *pChar, float time);
void SkillImpulsionLightHit(POINT3D *pos);
void SkillShadowMasterHit(POINT3D *pos);
void SkillPhoenixShotFlame(smCHAR *pChar, POINT3D *curPos);
void SkillGodBless(smCHAR *pChar);
void SkillParticle_FrostJavelin(smCHAR *pChar, float time);

void AssaMotionBlur(smCHAR *pChar, char *meshName1, char *meshName2, int timeCount);


// 아이스 필드

//ColdEye 레이저 쏘기..
int ParkAssaParticle_ColdEye(POINT3D *pPosi, POINT3D *pDesti);
//ColdEye 스킬공격
int ParkParticle_ColdEyeSkill(smCHAR *pChar);
//미스틱 스피넬 자폭하기
int ParkAssaParticle_MisticSpinel(smCHAR *pChar);
// 프로스트 에이션트 디아 이펙트
int ParkAssaParticle_AcientDia(smCHAR *pChar);
// 프로스트 에이션트 피어싱
int ParksinSkillEffect_AcientPiercing(smCHAR *pChar);
int ParksinSkillEffect_AcientPiercingAtt(smCHAR *pChar);

// 아이스 골렘 이펙트
int ParkAssaParticle_IceGolemShot(smCHAR *pChar);
// 프로즌 미스트	기본공격
int ParkAssaParticle_MistIceBolt(smCHAR* pCurChar, smCHAR* pDestChar);
// 프로즌 미스트	스킬공격
int ParkAssaParticle_MistIceBall(smCHAR* pCurChar, smCHAR* pDestChar);

// 아이스 고블린
int ParkAssaParticle_IceGoblinHit1(smCHAR *pChar);


int ParkAssaParticle_Valento1(smCHAR* pValento);	// 데스나이트 브랜디쉬
int ParkAssaParticle_Valento2_1(smCHAR* pValento);	// 데스나이트 버서커 (피어싱의 기 모으기)
int ParkAssaParticle_Valento2_2(smCHAR* pValento);	// 버서커 후에 기를 모아쏜다.
int ParkAssaParticle_Valento3_1(smCHAR* pValento);	// 데스나이트 내려치기는 (ParkAssaParticle_Valento1) 호출 이함수는 올려치기.
void ParkAssaParticle_MonsterIceValentoBody(smCHAR *pValento);	// 데스나이트 몸통에 붙는 파티클
void ParkValentSwordShow(smCHAR* pValento, int nTime);	// 데스나이트 검기

int ParkAssaParticle_ChaosKara1(smCHAR* pChar);	// 카오스 카라 일반공격
int ParkAssaParticle_ChaosKara2(smCHAR* pChar);	// 카오스 카라 아이스 미티어
int ParkAssaParticle_ChaosKaraSkill_Monster(smCHAR* pMonster); // 카오스카라 뱀파이어릭 (몬스터 자신)
int ParkAssaParticle_ChaosKaraSkill_User(smCHAR* pUser);	// 카오스카라 뱀파이어릭 (유저)
void ParkAssaParticle_MonsterIceChaosKara(smCHAR *pChar);	// 카오스 카라 스태프 파티클


///////////////// 새로운 필드 ////////////////
// 박철호 //
void ParkAssaParticle_SpiderTrooperHit1(smCHAR *pChar);	// 스파이더 트루퍼 독가스 뿜어내기

void ParkAssaParticle_StingRay(smCHAR *pChar);	//스팅레이 독가스 뿜어내기..
int ParkDivineLight_Effect( smCHAR *lpChar );		//디바인 라이트닝 스킬 연출 ( 이펙트 실현 )
int ParkAssaParticle_DivineLighting(smCHAR *pChar);	//디바인 라이트닝 스킬 연출 ( 이펙트 실현 )
int ParkAssaParticle_Sprak1(POINT3D *pPosi);
int ParkAssaParticle_Posion(smCHAR *pChar, int liveCount);		// 독걸린거 표현
void ParkAssaParticle_MummyShot(smCHAR *pChar, smCHAR *pDestChar);	// 머미로드 통상공격
#define STINGRAY_POSION_TIME	5

void ParkBlizzardGiantSkillToTarget(smCHAR *pTarget);	//블리자드 자이언트 스킬 이펙트

void ParkDevilBirdAttackBlur(smCHAR* pDevilBird);	// 데블 버드 일반 공격 시작시 블러
void ParkDevilBirdAttackNormal(smCHAR *pTarget);	// 데블 버드 공격동작 2번 타격이펙트
void ParkDevilBirdSkillToTarget(smCHAR *pTarget);	// 데블 버드 기술동작 타격 이펙트

void ParkAssaParticle_MonsterTurtleCanonNormal(smCHAR *pTCanon, smCHAR* pTarget);	// 터틀캐논 일반 공격

void ParkAssaParticle_FireCracker(smCHAR *pChar, int nFireCracker);	// 폭죽이펙트

// 푹죽 터트릴시 20개의 4차원배열 즉 20 ^ 4 의 테이블을 정의하여 리턴시켜준다.
// 번호만 입력하면 그 번호에 맞는 배열을 찿아서 인코드 하여 리턴시켜준다.
DWORD GetTableNum(int n);

// 4개의 값을 인코드한다.
DWORD Park_VarEncode(int a, int b, int c, int d);

// 인코드된 값을 다시 디코드 해준다.
void Park_VarDecode(DWORD dwData, int &nData1, int &nData2, int &nData3, int &nData4);

// 켈베쥬 이펙트
void ParkKelvezuHit(smCHAR *pChar);	// 켈베쥬 공격시 캐릭터 피격이펙트
void ParkKelvezuSkillHit(smCHAR *pChar);	// 켈베쥬 공격시 캐릭터 피격이펙트
void ParkKelvezuNormal1(smCHAR *pChar);
void ParkKelvezuNormal2(smCHAR *pKelvezu);
void ParkKelvezuSkill1(smCHAR *pKelvezu);
void ParkKelvezuSkill2(smCHAR *pKelvezu);
void ParkKelvezuJin(smCHAR *pKelvezu);	// 켈베쥬 마법진및 입에서 연기나는거

// 켈베쥬 공격시 발톱에서 검기효과
void ParkKelvezuFingerShow(smCHAR* pKelvezu, int nTime);

// 켈베쥬 공격시 날개에서 검기효과
void ParkKelvezuWingShow(smCHAR* pKelvezu, int nTime);

// 켈베쥬 공격시 꼬리에서 검기효과
void ParkKelvezuTaleShow(smCHAR* pKelvezu, int nTime);

// 월드컵 몬스터 이펙트
void ParkAssaParticle_WorldCupMonsterAttack(smCHAR* pMonster, smCHAR *pChar);
	// 월드컵 몬스터가 공격시 캐릭터 피격이펙트
void ParkWorldCupMonsterHit(smCHAR *pChar);



// Dark Phalanx 기술 동작 이펙트 (창으로 찌를때 창이 파고드는 느낌의 이펙트 - 피어싱으로 처리)
void ParkAssaParticle_DarkPhalanx(smCHAR *pDarkPhalanx);

// 파이어 웜 일반 공격 및 스킬공격
void ParkAssaParticle_TerrainFireHit(POINT3D *pCur, int nType);
void ParkAssaParticle_TerrainFire(POINT3D *pCur, POINT3D *pDest, int delay = 0, int nType = 0);
void ParkAssaParticle_FireWorm(smCHAR* pTarget, BOOL bSkill = FALSE);

// 키메라 일반공격 ( 발톱 블러 )
void ParkAssaParticle_ChimeraNormal(smCHAR *pChimera);

// 키메라 스킬공격 (헤딩 - 머리에 익스트림실드)
void ParkAssaParticle_ChimeraSkill(smCHAR *pChar);

// Bloody Knight 스킬 공격 (로어)
void ParkAssaParticle_BloodySkill(smCHAR *pBloodKnight);

// Bloody Knight 스킬 공격 피격 (스턴)
void ParkAssaParticle_BloodySkillStun(smCHAR *pChar);

// 헬 하운드 노말공격 (두발을 들어 대상을 내려찍음)
void ParkAssaParticle_HellHoundNormal(smCHAR *pChar);

// 헬 하운드 스킬공격 (입에서 불나감)
void ParkAssaParticle_HellHoundSkillAttack(smCHAR *pHellHound);

// 헬 하운드 스킬공격의 피격시 (화염에 휩싸인듯한 이펙트)
void ParkAssaParticle_HellHoundSkillTaget(smCHAR *pChar);

// 다크 가드 기본공격 피격시
void ParkAssaParticle_DarkGuardTaget(smCHAR *pChar);

// 다크 가드 스킬공격
void ParkAssaParticle_DarkGuardSkill(smCHAR *pDarkGuard, smCHAR *pChar);

// 다크 메이지 기본공격
void ParkAssaParticle_DarkMageNormal(smCHAR *pDarkMage, smCHAR *pChar);

// 다크 메이지 스킬공격
void ParkAssaParticle_DarkMageSkill(smCHAR *pChar);

// 다크 메이지 죽을때
void ParkAssaParticle_DarkMageDead(smCHAR *pDarkMage);






#endif