#include <windows.h>

#include "smlib3d\\smd3d.h"
#include "smwsock.h"

#include "character.h"
#include "playmain.h"
#include "effectsnd.h"
#include "fileread.h"
#include "netplay.h"
#include "makeshadow.h"
#include "particle.h"
#include "field.h"
#include "hoBaram\\holinkheader.h"
#include "sinbaram\\sinlinkheader.h"
#include "playsub.h"
#include "skillsub.h"
#include "damage.h"
#include "areaserver.h"
/*
//메카닉 범
int Mechanic_Bomb_Damage[10][2] = {{2,5},{3,8},{4,11},{5,14},{6,17},{7,20},{8,23},{9,26},{10,29},{10,32}};
int Mechanic_Bomb_Attack_Range[10] = {50,53,56,59,62,65,68,71,74,77};
int Mechanic_Bomb_UseMana[10] = {8,12,16,20,25,30,36,42,49,56};  

//레이빙 
int Raving_Damage[10]  = {12,17,22,27,32,37,42,47,52,57};
int Ravind_Speed[10]   = {1,1,1,2,2,2,3,3,3,4};
int Raving_UseLife[10] = {10,14,18,20,24,28,30,32,34,38};
int Raving_UseMana[10] = {2,3,5,7,9,11,14,17,20,23};

*/

static sSKILL Skill_ScoutHawk;

//int	ScoutHawk_LevelRange



//스킬을 오픈한다
/*----------------------------------------------------------------------------*
*	The new classes' skills (Assassin, Shaman, Martial Artist): one engine driven by
*	the NcSkills table (sinbaram/NewClassSkill_Data.h, generated from the 3215 client).
*	Self casts here, target casts in NC_PlaySkillAttack, the motion events' extra hits
*	and the effects in character.cpp (NC_BeginSkill / NC_EventSkill).
*-----------------------------------------------------------------------------*/
int RetryPlayAttack( smCHAR *lpChar );
static int NC_AreaOf( sNCSKILL *nc , int point )
{
	int a = NC_Rank( nc->Area , point );
	return a ? a : (nc->ConstArea ? nc->ConstArea : 80);
}
static int NC_HitsOf( sNCSKILL *nc , int point )
{
	int h = NC_Rank( nc->MaxHits , point );
	return h ? h : (nc->ConstHits ? nc->ConstHits : 12);
}

// a self cast (buffs, and attacks around the caster): 0 when it is not one
static int NC_OpenPlaySkill( sSKILL *lpSkill )
{
	sNCSKILL *nc = NC_FindCode( lpSkill->Skill_Info.CODE );
	if ( !nc || !nc->Play || !lpSkill->Point ) return 0;
	if ( nc->Kind!=NCK_BUFF && nc->Kind!=NCK_DEBUFF_SELF && nc->Kind!=NCK_POISON_SELF && nc->Kind!=NCK_MAGIC_AREA_SELF && nc->Kind!=NCK_SUMMON ) return 0;
	if ( lpCurPlayer->MotionInfo->State>=0x100 ) return 0;
	if ( lpSkill->Skill_Info.FuncPointer()!=TRUE ) return 0;

	lpCurPlayer->AttackSkil = nc->Play | (lpSkill->Point<<8);
	lpCurPlayer->BeginSkill( lpCurPlayer->AttackSkil , 0 , 0 ,0,0,0 );
	lpCurPlayer->SetMotionFromCode( CHRMOTION_STATE_SKILL );
	lpCurPlayer->MotionLoopSpeed = 70;
	lpCurPlayer->MotionLoop = 1;

	if ( nc->Kind==NCK_SUMMON )
		SendProcessSKillToServer( nc->Play , lpSkill->Point , 0, 0 );		// the server opens the monster
	else if ( nc->Kind!=NCK_BUFF ) {
		dm_SelectRange( lpCurPlayer->pX, lpCurPlayer->pY, lpCurPlayer->pZ, NC_AreaOf(nc,lpSkill->Point) , TRUE , NC_HitsOf(nc,lpSkill->Point) );
		dm_SendRangeDamage( lpCurPlayer->pX, lpCurPlayer->pY, lpCurPlayer->pZ, 0 ,
			cInvenTory.InvenItem[ sInven[0].ItemIndex-1 ].sItemInfo.Damage[0],
			cInvenTory.InvenItem[ sInven[0].ItemIndex-1 ].sItemInfo.Damage[1], 0, 0, lpCurPlayer->AttackSkil );
	}
	return 1;
}

// a target cast: strikes, attacks around the target, magic and debuffs. -1 when it is not one
static int NC_PlaySkillAttack( sSKILL *lpSkill , smCHAR *lpChar , short *wpDamage )
{
	sNCSKILL *nc = NC_FindCode( lpSkill->CODE );
	if ( !nc ) return -1;
	if ( !nc->Play || !lpSkill->Point ) return TRUE;
	switch( nc->Kind ) {
	case NCK_BUFF: case NCK_DEBUFF_SELF: case NCK_POISON_SELF: case NCK_MAGIC_AREA_SELF: case NCK_PASSIVE: case NCK_PENDING: case NCK_SUMMON:
		return TRUE;		// not a target cast
	}
	if ( lpSkill->Skill_Info.FuncPointer()!=TRUE ) return TRUE;

	int magic = (nc->Kind==NCK_MAGIC || nc->Kind==NCK_MAGIC_AREA);
	if ( !magic ) RetryPlayAttack( lpChar );
	lpCurPlayer->AttackSkil = nc->Play | (lpSkill->Point<<8);
	lpCurPlayer->BeginSkill( lpCurPlayer->AttackSkil , 0 , lpChar ,0,0,0 );
	lpCurPlayer->SetMotionFromCode( CHRMOTION_STATE_SKILL );

	if ( nc->Kind==NCK_HIT || nc->Kind==NCK_MAGIC ) {
		int rate = lpCurPlayer->smCharInfo.Attack_Rating;
		int add = NC_Rank( nc->AddHit , lpSkill->Point );
		if ( add ) lpCurPlayer->smCharInfo.Attack_Rating += (rate*add)/100;
		int cnt = magic ? dm_SendTransDamage( lpChar , wpDamage[0],wpDamage[1] , 0, 0, lpCurPlayer->AttackSkil , FALSE )
						: dm_SendTransDamage( lpChar , 0,0 , 0, 0, lpCurPlayer->AttackSkil , TRUE );
		lpCurPlayer->smCharInfo.Attack_Rating = rate;
		if ( !cnt ) lpCurPlayer->AttackCritcal = -1;
		else {
			lpCurPlayer->AttackCritcal = 0;
			DamageExp( lpChar , cnt );
			int drain = NC_Rank( nc->Drain , lpSkill->Point );		// Haunt: life back
			if ( drain && sinGetLife()>0 ) {
				int life = sinGetLife() + (cnt*drain)/100;
				if ( life>lpCurPlayer->smCharInfo.Life[1] ) life = lpCurPlayer->smCharInfo.Life[1];
				sinSetLife( life );
			}
		}
	}
	else {	// around the target
		dm_SelectRange( lpChar->pX, lpChar->pY, lpChar->pZ, NC_AreaOf(nc,lpSkill->Point) , TRUE , NC_HitsOf(nc,lpSkill->Point) );
		dm_AddRangeDamage( lpChar );
		dm_SendRangeDamage( lpChar->pX, lpChar->pY, lpChar->pZ, lpChar , wpDamage[0], wpDamage[1], 0, 0, lpCurPlayer->AttackSkil );
	}
	lpCurPlayer->MotionLoopSpeed = magic ? GetAttackSpeedFrame( 8 ) : 70;
	lpCurPlayer->MotionLoop = 1;
	return TRUE;
}

// the cast range of a new-class skill (0 = the weapon's)
static int NC_SkillDistRange( sSKILL *lpSkill , int AttackRange )
{
	sNCSKILL *nc = NC_FindCode( lpSkill->CODE );
	if ( !nc || !lpSkill->Point ) return 0;
	int r = NC_Rank( nc->Range , lpSkill->Point );
	if ( !r ) r = nc->ConstRange;
	if ( !r && (nc->Kind==NCK_MAGIC || nc->Kind==NCK_MAGIC_AREA) ) r = 180;
	if ( !r && nc->Kind==NCK_DEBUFF_TGT ) r = 160;
	if ( !r ) return 0;
	return r*fONE > AttackRange ? r*fONE : AttackRange;
}

// extra evasion from the new classes' buffs and passives (character.cpp, when a hit comes in)
int NC_GetEvasion()
{
	int ev = 0, i;
	for( i=0; i<MAX_CONTINUE_SKILL; i++ ) {
		if ( !ContinueSkill[i].Flag ) continue;
		sNCSKILL *nc = NC_FindCode( ContinueSkill[i].CODE );
		if ( nc && nc->Kind==NCK_BUFF && nc->Stat==NCS_EVADE ) ev += NC_Rank( nc->Val , ContinueSkill[i].Point );
	}
	for( i=1; i<SIN_MAX_USE_SKILL; i++ ) {
		if ( !sinSkill.UseSkill[i].Flag || !sinSkill.UseSkill[i].Point ) continue;
		sNCSKILL *nc = NC_FindCode( sinSkill.UseSkill[i].CODE );
		if ( !nc || nc->Kind!=NCK_PASSIVE ) continue;
		if ( nc->Stat==NCS_EVADE ) ev += NC_Rank( nc->Val , sinSkill.UseSkill[i].Point );
		if ( nc->Stat2==NCS_EVADE ) ev += NC_Rank( nc->Val2 , sinSkill.UseSkill[i].Point );
	}
	return ev;
}

int OpenPlaySkill( sSKILL *lpSkill )
{
	int flag;
	//int	sp,ep;
	int cnt;
	short wpDamage[2];
	//smTRANS_COMMAND	smTransCommand;

	pUseSkill = lpSkill;

	flag = 0;

	if ( lpCurPlayer->OnStageField>=0 && StageField[ lpCurPlayer->OnStageField ]->State==FIELD_STATE_VILLAGE )
		return flag;

	if ( sinCheckSkillUseOk( lpSkill )==FALSE ) return flag;
	if ( lpSkill->Point>10 ) return flag;

	wpDamage[0] = cInvenTory.InvenItem[ sInven[0].ItemIndex-1 ].sItemInfo.Damage[0];
	wpDamage[1] = cInvenTory.InvenItem[ sInven[0].ItemIndex-1 ].sItemInfo.Damage[1];

	lpSkill->SkillTaget_CODE = 0;

	cnt = NC_OpenPlaySkill( lpSkill );		// the new classes' self casts
	if ( cnt ) return cnt;

	switch( lpSkill->Skill_Info.CODE ) {
				
	case SKILL_EXTREME_SHIELD:
		//익스트림 쉴드
		if ( lpCurPlayer->MotionInfo->State<0x100 ) {
			if ( lpSkill->Skill_Info.FuncPointer()==TRUE ) {
				lpCurPlayer->BeginSkill( SKILL_PLAY_EXTREME_SHIELD , 0 , 0 ,0,0,0 );
				lpCurPlayer->AttackSkil = SKILL_PLAY_EXTREME_SHIELD;
				lpCurPlayer->SetMotionFromCode( CHRMOTION_STATE_SKILL );
				lpCurPlayer->MotionLoopSpeed = 100;
				lpCurPlayer->MotionLoop = 1;
				flag++;
			}
		}
		break;

	case SKILL_PHYSICAL_ABSORB:
		//픽지클 옵져브
		if ( lpCurPlayer->MotionInfo->State<0x100 ) {
			if ( lpSkill->Skill_Info.FuncPointer()==TRUE ) {

				lpCurPlayer->AttackSkil = SKILL_PLAY_PHYSICAL_ABSORB;
				lpCurPlayer->AttackSkil |= (lpSkill->Point<<8);				//스킬 포인트 상위 바이트에 저장
				lpCurPlayer->BeginSkill( lpCurPlayer->AttackSkil , 0 , 0 ,0,0,0 );

				lpCurPlayer->SetMotionFromCode( CHRMOTION_STATE_SKILL );
				lpCurPlayer->MotionLoopSpeed = 70;
				lpCurPlayer->MotionLoop = 1;

				SendProcessSKillToServer( SKILL_PLAY_PHYSICAL_ABSORB , lpSkill->Point , 0, 0 );

				//스킬 유지 사운드
				SkillPlaySound( SKILL_SOUND_PHYSICAL_ABSORB3 , lpCurPlayer->pX,lpCurPlayer->pY,lpCurPlayer->pZ, TRUE );
				flag++;
			}
		}
		break;

	case SKILL_POISON_ATTRIBUTE:
		//포이즌 어트리븃
		if ( lpSkill->Skill_Info.FuncPointer()==TRUE ) {
			lpCurPlayer->BeginSkill( SKILL_PLAY_POISON_ATTRIBUTE , 0 , 0 ,0,0,0 );
			flag++;
		}
		break;


	case SKILL_PIKE_WIND:

		//파이크 윈드
		if ( lpSkill->Skill_Info.FuncPointer()==TRUE ) {
			if ( lpSkill->Point ) {
				lpCurPlayer->BeginSkill(SKILL_PLAY_PIKEWIND,0,0,0,0,0);
				lpCurPlayer->AttackSkil = SKILL_PLAY_PIKEWIND;
				lpCurPlayer->AttackSkil |= (lpSkill->Point<<8);				//스킬 포인트 상위 바이트에 저장
				lpCurPlayer->SetMotionFromCode( CHRMOTION_STATE_SKILL );

				lpCurPlayer->MotionLoopSpeed = 80;
				lpCurPlayer->MotionLoop = 1;
/*
				sp = Pike_Wind_Damage[lpSkill->Point-1][0];
				ep = Pike_Wind_Damage[lpSkill->Point-1][1];

				Skil_RangeAttack( lpCurPlayer->pX, lpCurPlayer->pY, lpCurPlayer->pZ, Pike_Wind_Push_Lenght[lpSkill->Point-1]*fONE , 
					GetRandomPos( sp, ep ) , 1 );
*/

				//범위형 공격 범위 선정 (원)
				dm_SelectRange( lpCurPlayer->pX, lpCurPlayer->pY, lpCurPlayer->pZ, Pike_Wind_Push_Lenght[lpSkill->Point-1] , FALSE );
				//범위공격 설정후 전송
				dm_SendRangeDamage( lpCurPlayer->pX, lpCurPlayer->pY, lpCurPlayer->pZ, 0 ,0, 0, 0, 0, lpCurPlayer->AttackSkil );

				flag++;
			}
		}
		break;

	case SKILL_SCOUT_HAWK:
		//스카우트 호크
		if ( lpCurPlayer->HoSkillCode!=SKILL_SCOUT_HAWK && lpSkill->Skill_Info.FuncPointer()==TRUE ) {

			lpCurPlayer->AttackSkil = SKILL_PLAY_SCOUT_HAWK;
			lpCurPlayer->AttackSkil |= (lpSkill->Point<<8);				//스킬 포인트 상위 바이트에 저장
			lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil,0,0,0,0,0);
			lpCurPlayer->SetMotionFromCode( CHRMOTION_STATE_SKILL );
			memcpy( &Skill_ScoutHawk , lpSkill , sizeof( sSKILL ) );
			lpCurPlayer->HideWeapon = TRUE;
			flag++;
		}
		break;

	/////////////////////////////////// 2단계 스킬 ///////////////////////////////////////

	//	SKILL_PLAY_GREAT_SMASH
	case SKILL_MAXIMIZE:
		//맥시마이즈
		if ( lpSkill->Skill_Info.FuncPointer()==TRUE && lpSkill->Point ) {

			lpCurPlayer->AttackSkil = SKILL_PLAY_MAXIMIZE;
			lpCurPlayer->AttackSkil |= (lpSkill->Point<<8);				//스킬 포인트 상위 바이트에 저장
			lpCurPlayer->BeginSkill( lpCurPlayer->AttackSkil , 0 , 0 ,0,0,0 );

			lpCurPlayer->SetMotionFromCode( CHRMOTION_STATE_SKILL );
			lpCurPlayer->MotionLoopSpeed = 70;
			lpCurPlayer->MotionLoop = 1;

			flag++;
		}
		break;

	case SKILL_AUTOMATION:
		//오토메이션
		if ( lpSkill->Skill_Info.FuncPointer()==TRUE && lpSkill->Point ) {

			lpCurPlayer->AttackSkil = SKILL_PLAY_AUTOMATION;
			lpCurPlayer->AttackSkil |= (lpSkill->Point<<8);				//스킬 포인트 상위 바이트에 저장
			lpCurPlayer->BeginSkill( lpCurPlayer->AttackSkil , 0 , 0 ,0,0,0 );

			lpCurPlayer->SetMotionFromCode( CHRMOTION_STATE_SKILL );
			lpCurPlayer->MotionLoopSpeed = 70;
			lpCurPlayer->MotionLoop = 1;
				
			SkillPlaySound( SKILL_SOUND_SKILL_AUTOMATION3 , lpCurPlayer->pX,lpCurPlayer->pY,lpCurPlayer->pZ, TRUE );

			flag++;
		}
		break;

	case SKILL_GROUND_PIKE:
		//그라운드 파이크 ( 얼음속성 공격 ) 10-(몬스터속성/10) 초
		if ( lpSkill->Skill_Info.FuncPointer()==TRUE ) {
			if ( lpSkill->Point ) {
				lpCurPlayer->AttackSkil = SKILL_PLAY_GROUND_PIKE;
				lpCurPlayer->AttackSkil |= (lpSkill->Point<<8);				//스킬 포인트 상위 바이트에 저장
				lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil,0,0,0,0,0);
				lpCurPlayer->SetMotionFromCode( CHRMOTION_STATE_SKILL );

				lpCurPlayer->MotionLoopSpeed = 80;
				lpCurPlayer->MotionLoop = 1;

/*
				sp = G_Pike_Damage[lpSkill->Point-1][0];
				ep = G_Pike_Damage[lpSkill->Point-1][1];

				Skil_RangeAttack( lpCurPlayer->pX, lpCurPlayer->pY, lpCurPlayer->pZ, G_Pike_Range[lpSkill->Point-1]*fONE , 
					GetRandomPos( sp, ep ) , 3 );
*/
				//범위형 공격 범위 선정 (원)
				dm_SelectRange( lpCurPlayer->pX, lpCurPlayer->pY, lpCurPlayer->pZ, G_Pike_Range[lpSkill->Point-1] , FALSE );
				//범위공격 설정후 전송
				dm_SendRangeDamage( lpCurPlayer->pX, lpCurPlayer->pY, lpCurPlayer->pZ, 0 ,0, 0, 0, 0, lpCurPlayer->AttackSkil );

				flag++;
			}
		}
		break;


	case SKILL_VENOM_SPEAR:
		//베놈 스피어
		if ( lpSkill->Skill_Info.FuncPointer()==TRUE ) {
			if ( lpSkill->Point ) {
				lpCurPlayer->AttackSkil = SKILL_PLAY_VENOM_SPEAR;
				lpCurPlayer->AttackSkil |= (lpSkill->Point<<8);				//스킬 포인트 상위 바이트에 저장
				lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil,0,0,0,0,0);
				lpCurPlayer->SetMotionFromCode( CHRMOTION_STATE_SKILL );

				lpCurPlayer->MotionLoopSpeed = 80;
				lpCurPlayer->MotionLoop = 1;
/*
				//범위형 공격 범위 선정 (원)
				dm_SelectRange( lpCurPlayer->pX, lpCurPlayer->pY, lpCurPlayer->pZ, G_Pike_Range[lpSkill->Point-1] , FALSE );
				//범위공격 설정후 전송
				dm_SendRangeDamage( lpCurPlayer->pX, lpCurPlayer->pY, lpCurPlayer->pZ, 0 ,0, 0, 0, 0, lpCurPlayer->AttackSkil );
*/
				flag++;
			}
		}
		break;

	case SKILL_VANISH:
		//배니쉬		//블레이드 오브 블레이즈
		if ( lpSkill->Skill_Info.FuncPointer()==TRUE ) {
			if ( lpSkill->Point ) {
				lpCurPlayer->AttackSkil = SKILL_PLAY_VANISH;
				lpCurPlayer->AttackSkil |= (lpSkill->Point<<8);				//스킬 포인트 상위 바이트에 저장
				lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil,0,0,0,0,0);
				lpCurPlayer->SetMotionFromCode( CHRMOTION_STATE_SKILL );

				lpCurPlayer->MotionLoopSpeed = 80;
				lpCurPlayer->MotionLoop = 1;

				flag++;
			}
		}
		break;



	case SKILL_FALCON:
		//팔콘
		if ( lpCurPlayer->HoSkillCode!=SKILL_FALCON && lpSkill->Skill_Info.FuncPointer()==TRUE ) {
			if ( lpSkill->Point ) {
				lpCurPlayer->AttackSkil = SKILL_PLAY_FALCON;
				lpCurPlayer->AttackSkil |= (lpSkill->Point<<8);				//스킬 포인트 상위 바이트에 저장
				lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil,0,0,0,0,0);
				lpCurPlayer->SetMotionFromCode( CHRMOTION_STATE_SKILL );

				lpCurPlayer->MotionLoopSpeed = 80;
				lpCurPlayer->MotionLoop = 1;
				lpCurPlayer->HideWeapon = TRUE;

				SkillFalconPoint = (lpSkill->Point-1)^lpCurPlayer->dwObjectSerial^((DWORD)chrOtherPlayer);

				SendProcessSKillToServer( SKILL_PLAY_FALCON , lpSkill->Point , 0, 0 );
			}
		}
		break;

	case SKILL_GOLDEN_FALCON:
		//골든 팔콘
		if ( lpCurPlayer->HoSkillCode!=SKILL_GOLDEN_FALCON && lpSkill->Skill_Info.FuncPointer()==TRUE ) {
			if ( lpSkill->Point ) {
				lpCurPlayer->AttackSkil = SKILL_PLAY_GOLDEN_FALCON;
				lpCurPlayer->AttackSkil |= (lpSkill->Point<<8);				//스킬 포인트 상위 바이트에 저장
				lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil,0,0,0,0,0);
				lpCurPlayer->SetMotionFromCode( CHRMOTION_STATE_SKILL );

				lpCurPlayer->MotionLoopSpeed = 80;
				lpCurPlayer->MotionLoop = 1;
				lpCurPlayer->HideWeapon = TRUE;

				SkillFalconPoint = (lpSkill->Point-1)^lpCurPlayer->dwObjectSerial^((DWORD)chrOtherPlayer);

				cnt = cSkill.GetSkillPoint( SKILL_FALCON );			//팔콘의 포인트
				SendProcessSKillToServer( SKILL_PLAY_GOLDEN_FALCON , lpSkill->Point , cnt, 0 );
			}
		}
		break;


	case SKILL_ROAR:
		//로어 ( 스턴확률  ((내꺼생속성 + 100 )-몬스터 생속성) %
		if ( lpSkill->Skill_Info.FuncPointer()==TRUE ) {
			if ( lpSkill->Point ) {
				lpCurPlayer->AttackSkil = SKILL_PLAY_ROAR;
				lpCurPlayer->AttackSkil |= (lpSkill->Point<<8);				//스킬 포인트 상위 바이트에 저장
				lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil,0,0,0,0,0);
				lpCurPlayer->SetMotionFromCode( CHRMOTION_STATE_SKILL );

				lpCurPlayer->MotionLoopSpeed = 80;
				lpCurPlayer->MotionLoop = 1;

				//밀어내기
				//Skil_RangeAttack( lpCurPlayer->pX, lpCurPlayer->pY, lpCurPlayer->pZ, Roar_Range[lpSkill->Point-1]*fONE , Roar_Time[lpSkill->Point-1] , 2 );

				//범위형 공격 범위 선정 (원)
				dm_SelectRange( lpCurPlayer->pX, lpCurPlayer->pY, lpCurPlayer->pZ, Roar_Range[lpSkill->Point-1] , FALSE );
				//범위공격 설정후 전송
				dm_SendRangeDamage( lpCurPlayer->pX, lpCurPlayer->pY, lpCurPlayer->pZ, 0 ,0, 0, 0, 0, lpCurPlayer->AttackSkil );

				flag++;
			}
		}
		break;

	case SKILL_CONCENTRATION:
		//콘센트
		if ( lpSkill->Skill_Info.FuncPointer()==TRUE ) {
			if ( lpSkill->Point ) {
				lpCurPlayer->AttackSkil = SKILL_PLAY_CONCENTRATION;
				lpCurPlayer->AttackSkil |= (lpSkill->Point<<8);				//스킬 포인트 상위 바이트에 저장
				lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil,0,0,0,0,0);
				lpCurPlayer->SetMotionFromCode( CHRMOTION_STATE_SKILL );

				lpCurPlayer->MotionLoopSpeed = 80;
				lpCurPlayer->MotionLoop = 1;

				flag++;
			}
		}
		break;

	case SKILL_SWIFT_AXE:
		//스위트액스
		if ( lpSkill->Skill_Info.FuncPointer()==TRUE ) {
			if ( lpSkill->Point ) {
				lpCurPlayer->AttackSkil = SKILL_PLAY_SWIFT_AXE;
				lpCurPlayer->AttackSkil |= (lpSkill->Point<<8);				//스킬 포인트 상위 바이트에 저장
				lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil,0,0,0,0,0);
				lpCurPlayer->SetMotionFromCode( CHRMOTION_STATE_SKILL );

				lpCurPlayer->MotionLoopSpeed = 80;
				lpCurPlayer->MotionLoop = 1;

				flag++;
			}
		}
		break;

	/////////////////////////////////// 3단계 스킬 ///////////////////////////////////////

	case SKILL_METAL_ARMOR:
		//메탈 아머
		if ( lpCurPlayer->MotionInfo->State<0x100 ) {
			if ( lpSkill->Skill_Info.FuncPointer()==TRUE ) {

				lpCurPlayer->AttackSkil = SKILL_PLAY_METAL_ARMOR;
				lpCurPlayer->AttackSkil |= (lpSkill->Point<<8);				//스킬 포인트 상위 바이트에 저장
				lpCurPlayer->BeginSkill( lpCurPlayer->AttackSkil , 0 , 0 ,0,0,0 );

				lpCurPlayer->SetMotionFromCode( CHRMOTION_STATE_SKILL );
				lpCurPlayer->MotionLoopSpeed = 70;
				lpCurPlayer->MotionLoop = 1;

				cnt = cSkill.GetSkillPoint( SKILL_PHYSICAL_ABSORB );			//피지컬의 포인트

				SendProcessSKillToServer( SKILL_PLAY_METAL_ARMOR , lpSkill->Point , cnt , 0 );
				//스킬 유지 사운드
				//SkillPlaySound( SKILL_SOUND_PHYSICAL_ABSORB3 , lpCurPlayer->pX,lpCurPlayer->pY,lpCurPlayer->pZ, TRUE );
				flag++;
			}
		}
		break;

	case SKILL_SPARK_SHIELD:
		//스파크쉴드
		if ( lpCurPlayer->MotionInfo->State<0x100 ) {
			if ( lpSkill->Skill_Info.FuncPointer()==TRUE ) {

				lpCurPlayer->AttackSkil = SKILL_PLAY_SPARK_SHIELD;
				lpCurPlayer->AttackSkil |= (lpSkill->Point<<8);				//스킬 포인트 상위 바이트에 저장
				lpCurPlayer->BeginSkill( lpCurPlayer->AttackSkil , 0 , 0 ,0,0,0 );

				lpCurPlayer->SetMotionFromCode( CHRMOTION_STATE_SKILL );
				lpCurPlayer->MotionLoopSpeed = 70;
				lpCurPlayer->MotionLoop = 1;

				cnt = cSkill.GetSkillPoint( SKILL_SPARK );			//스파크의 포인트

				SendProcessSKillToServer( SKILL_PLAY_SPARK_SHIELD , lpSkill->Point , cnt, 0 );
				//스킬 유지 사운드
				//SkillPlaySound( SKILL_SOUND_PHYSICAL_ABSORB3 , lpCurPlayer->pX,lpCurPlayer->pY,lpCurPlayer->pZ, TRUE );
				flag++;
			}
		}
		break;


		/////////////////////////////////////////////////////////////////////////
		///////////////////////////// 모라이온 스킬 /////////////////////////////
		/////////////////////////////////////////////////////////////////////////

		//////////////////////////// 나이트 /////////////////////////////////////

	case SKILL_HOLY_BODY:
		//픽지클 옵져브
		if ( lpCurPlayer->MotionInfo->State<0x100 ) {
			if ( lpSkill->Skill_Info.FuncPointer()==TRUE ) {

				lpCurPlayer->AttackSkil = SKILL_PLAY_HOLY_BODY;
				lpCurPlayer->AttackSkil |= (lpSkill->Point<<8);				//스킬 포인트 상위 바이트에 저장
				lpCurPlayer->BeginSkill( lpCurPlayer->AttackSkil , 0 , 0 ,0,0,0 );

				lpCurPlayer->SetMotionFromCode( CHRMOTION_STATE_SKILL );
				lpCurPlayer->MotionLoopSpeed = 70;
				lpCurPlayer->MotionLoop = 1;

				SendProcessSKillToServer( SKILL_PLAY_HOLY_BODY , lpSkill->Point , 0, 0 );

				//스킬 유지 사운드
				SkillPlaySound( SKILL_SOUND_PHYSICAL_ABSORB3 , lpCurPlayer->pX,lpCurPlayer->pY,lpCurPlayer->pZ, TRUE );
				flag++;
			}
		}
		break;
		//////////////////////// 2차 스킬 /////////////////////////
	case SKILL_HOLY_VALOR:
		if ( lpSkill->Skill_Info.FuncPointer()==TRUE ) {
			lpCurPlayer->AttackSkil = SKILL_PLAY_HOLY_VALOR;
			lpCurPlayer->AttackSkil |= (lpSkill->Point<<8);				//스킬 포인트 상위 바이트에 저장
			lpCurPlayer->BeginSkill( lpCurPlayer->AttackSkil , 0 , 0 ,0,0,0 );

			lpCurPlayer->SetMotionFromCode( CHRMOTION_STATE_SKILL );
			lpCurPlayer->MotionLoopSpeed = 70;
			lpCurPlayer->MotionLoop = 1;
			flag++;
		}
		break;

	case SKILL_DRASTIC_SPIRIT:
		//드래스틱스피리트
		if ( lpSkill->Skill_Info.FuncPointer()==TRUE ) {
			lpCurPlayer->AttackSkil = SKILL_PLAY_DRASTIC_SPIRIT;
			lpCurPlayer->AttackSkil |= (lpSkill->Point<<8);				//스킬 포인트 상위 바이트에 저장
			lpCurPlayer->BeginSkill( lpCurPlayer->AttackSkil , 0 , 0 ,0,0,0 );

			lpCurPlayer->SetMotionFromCode( CHRMOTION_STATE_SKILL );
			lpCurPlayer->MotionLoopSpeed = 70;
			lpCurPlayer->MotionLoop = 1;
			flag++;
		}
		break;

	case SKILL_DIVINE_INHALATION:
		//디바인 인헐레이션
		if ( lpCurPlayer->MotionInfo->State<0x100 ) {
			if ( lpSkill->Skill_Info.FuncPointer()==TRUE ) {

				lpCurPlayer->AttackSkil = SKILL_PLAY_DIVINE_INHALATION;
				lpCurPlayer->AttackSkil |= (lpSkill->Point<<8);				//스킬 포인트 상위 바이트에 저장
				lpCurPlayer->BeginSkill( lpCurPlayer->AttackSkil , 0 , 0 ,0,0,0 );

				lpCurPlayer->SetMotionFromCode( CHRMOTION_STATE_SKILL );
				lpCurPlayer->MotionLoopSpeed = 70;
				lpCurPlayer->MotionLoop = 1;

				SendProcessSKillToServer( SKILL_PLAY_DIVINE_INHALATION , lpSkill->Point , 0, 0 );
				//스킬 유지 사운드
				//SkillPlaySound( SKILL_SOUND_PHYSICAL_ABSORB3 , lpCurPlayer->pX,lpCurPlayer->pY,lpCurPlayer->pZ, TRUE );

				flag++;
			}
		}
		break;

		//////////////////////////// 아탈란타 /////////////////////////////////////
	case SKILL_WINDY:
		//윈디
		if ( lpSkill->Skill_Info.FuncPointer()==TRUE ) {
			lpCurPlayer->AttackSkil = SKILL_PLAY_WINDY;
			lpCurPlayer->AttackSkil |= (lpSkill->Point<<8);				//스킬 포인트 상위 바이트에 저장
			lpCurPlayer->BeginSkill( lpCurPlayer->AttackSkil , 0 , 0 ,0,0,0 );

			lpCurPlayer->SetMotionFromCode( CHRMOTION_STATE_SKILL );
			flag++;
		}
		break;

	case SKILL_TRIUMPH_OF_VALHALLA:
		//발할라
		if ( !lpCharSelPlayer && 
			GetAssaUseEffect(lpCurPlayer,SKILL_TRIUMPH_OF_VALHALLA)==FALSE && 
			!cSkill.SearchContiueSkill(SKILL_HALL_OF_VALHALLA) &&
			lpSkill->Skill_Info.FuncPointer()==TRUE ) {

			lpCurPlayer->AttackSkil = SKILL_PLAY_TRIUMPH_OF_VALHALLA;
			lpCurPlayer->AttackSkil |= (lpSkill->Point<<8);				//스킬 포인트 상위 바이트에 저장
			lpCurPlayer->BeginSkill( lpCurPlayer->AttackSkil , 0 , 0 ,0,0,0 );

			lpCurPlayer->SetMotionFromCode( CHRMOTION_STATE_SKILL );

			//파티원에게 스킬 사용 정보 보내기
			SendProcessSKillToServer( SKILL_PLAY_TRIUMPH_OF_VALHALLA , lpSkill->Point , 0, lpCurPlayer->smCharInfo.Level );
			lpCurPlayer->chrAttackTarget = 0; 
			cSkill.SetT_Of_Valhalla( lpSkill->Point , lpCurPlayer->smCharInfo.Level );

			flag++;
		}
		break;
/*
	case SKILL_LIGHTNING_JAVELIN:
		//라이트닝 재블린
		if ( lpSkill->Skill_Info.FuncPointer()==TRUE ) {
			lpCurPlayer->AttackSkil = SKILL_PLAY_LIGHTNING_JAVELIN;
			lpCurPlayer->AttackSkil |= (lpSkill->Point<<8);				//스킬 포인트 상위 바이트에 저장
			lpCurPlayer->BeginSkill( lpCurPlayer->AttackSkil , 0 , 0 ,0,0,0 );

			lpCurPlayer->SetMotionFromCode( CHRMOTION_STATE_SKILL );
			flag++;
		}
		break;
*/

		//////////////////////////// 프리스티스 /////////////////////////////////////
	case SKILL_HEALING:
		//힐링
		if ( lpCurPlayer->MotionInfo->State<0x100 && !lpCharSelPlayer ) {
			if ( lpSkill->Skill_Info.FuncPointer()==TRUE ) {

				lpCurPlayer->AttackSkil = SKILL_PLAY_HEALING;
				lpCurPlayer->AttackSkil |= (lpSkill->Point<<8);				//스킬 포인트 상위 바이트에 저장
				lpCurPlayer->BeginSkill( lpCurPlayer->AttackSkil , 0 , 0 ,0,0,0 );

				lpCurPlayer->SetMotionFromCode( CHRMOTION_STATE_SKILL );
				lpCurPlayer->MotionLoopSpeed = 70;
				lpCurPlayer->MotionLoop = 1;

				cnt = GetRandomPos( Healing_Heal[lpSkill->Point-1][0]+wpDamage[0], Healing_Heal[lpSkill->Point-1][1]+wpDamage[0] );

				sinSetLife( sinGetLife()+cnt );
				ResetEnergyGraph( 0 );


/*
				smTransCommand.size = sizeof(smTRANS_COMMAND);
				smTransCommand.code = smTRANSCODE_PARTY_HEALING;
				smTransCommand.WParam =	cnt;	//생명력
				smTransCommand.LParam =	0;		//
				smTransCommand.SParam =	0;		//사정거리
				smTransCommand.EParam = lpCurPlayer->dwObjectSerial;

				if ( smWsockUserServer && InterfaceParty.PartyMemberCount )
					smWsockUserServer->Send2( (char *)&smTransCommand , smTransCommand.size , TRUE );
*/
				flag++;
			}
		}
		break;
/*
	case SKILL_DIVINE_LIGHTNING:
		if ( lpSkill->Skill_Info.FuncPointer()==TRUE ) {
			lpCurPlayer->AttackSkil = SKILL_PLAY_DIVINE_LIGHTNING;
			lpCurPlayer->AttackSkil |= (lpSkill->Point<<8);				//스킬 포인트 상위 바이트에 저장
			lpCurPlayer->BeginSkill( lpCurPlayer->AttackSkil , 0 , 0 ,0,0,0 );

			lpCurPlayer->SetMotionFromCode( CHRMOTION_STATE_SKILL );
			flag++;
		}
		break;
*/
	case SKILL_DIVINE_LIGHTNING:
		//디바인 라이트닝
		if ( lpSkill->Skill_Info.FuncPointer()==TRUE ) {

			//SkillPlay_DivineLightning_Select( lpCurPlayer , lpSkill->Point );
			lpCurPlayer->AttackSkil = SKILL_PLAY_DIVINE_LIGHTNING;
			lpCurPlayer->AttackSkil |= (lpSkill->Point<<8);				//스킬 포인트 상위 바이트에 저장
			lpCurPlayer->BeginSkill( lpCurPlayer->AttackSkil,0,0,0,0,0 );

			lpCurPlayer->SetMotionFromCode( CHRMOTION_STATE_SKILL );
			lpCurPlayer->MotionLoopSpeed = 85;
			lpCurPlayer->MotionLoop = 1;

			flag++;
		}
		break;

	case SKILL_HOLY_REFLECTION:
		if ( lpSkill->Skill_Info.FuncPointer()==TRUE ) {
			lpCurPlayer->AttackSkil = SKILL_PLAY_HOLY_REFLECTION;
			lpCurPlayer->AttackSkil |= (lpSkill->Point<<8);				//스킬 포인트 상위 바이트에 저장
			lpCurPlayer->BeginSkill( lpCurPlayer->AttackSkil , 0 , 0 ,0,0,0 );

			lpCurPlayer->SetMotionFromCode( CHRMOTION_STATE_SKILL );
			flag++;
		}
		break;
	case SKILL_GRAND_HEALING:
		if ( InterfaceParty.PartyMemberCount>0 && lpSkill->Skill_Info.FuncPointer()==TRUE ) {
			lpCurPlayer->AttackSkil = SKILL_PLAY_GREAT_HEALING;
			lpCurPlayer->AttackSkil |= (lpSkill->Point<<8);				//스킬 포인트 상위 바이트에 저장
			lpCurPlayer->BeginSkill( lpCurPlayer->AttackSkil , 0 , 0 ,0,0,0 );
			lpCurPlayer->SetMotionFromCode( CHRMOTION_STATE_SKILL );

			lpCurPlayer->MotionLoopSpeed = 70;
			lpCurPlayer->MotionLoop = 1;
/*
			smTransCommand.size = sizeof(smTRANS_COMMAND);
			smTransCommand.code = smTRANSCODE_GRAND_HEALING;
			smTransCommand.WParam =	lpSkill->Point;					//스킬포인트
			smTransCommand.LParam =	GetRandomPos(Grand_Healing[lpSkill->Point-1][0] , Grand_Healing[lpSkill->Point-1][1] );
			smTransCommand.SParam =	Grand_Healing_Range[lpSkill->Point-1];
			smTransCommand.EParam = lpCurPlayer->dwObjectSerial;	//

			if ( smWsockUserServer )
				smWsockUserServer->Send2( (char *)&smTransCommand , smTransCommand.size , TRUE );
*/
			if ( AreaServerMode ) 
				dm_SendTransDamage( lpCurPlayer , wpDamage[0],wpDamage[1] ,0,0, lpCurPlayer->AttackSkil , FALSE , smWsockServer );	
			else
				dm_SendTransDamage( lpCurPlayer , wpDamage[0],wpDamage[1] ,0,0, lpCurPlayer->AttackSkil , FALSE );	

			flag++;
		}
		break;

	case SKILL_RESURRECTION:		//죽은자 소생
		if ( lpSkill->Skill_Info.FuncPointer()==TRUE ) {
			lpCurPlayer->AttackSkil = SKILL_PLAY_RESURRECTION;
			lpCurPlayer->AttackSkil |= (lpSkill->Point<<8);				//스킬 포인트 상위 바이트에 저장
			lpCurPlayer->BeginSkill( lpCurPlayer->AttackSkil , 0 , 0 ,0,0,0 );

			lpCurPlayer->SetMotionFromCode( CHRMOTION_STATE_SKILL );
			flag++;
		}
		break;

	case SKILL_EXTINCTION:		//언데드 소멸
		if ( lpSkill->Skill_Info.FuncPointer()==TRUE ) {
			lpCurPlayer->AttackSkil = SKILL_PLAY_EXTINCTION;
			lpCurPlayer->AttackSkil |= (lpSkill->Point<<8);				//스킬 포인트 상위 바이트에 저장
			lpCurPlayer->BeginSkill( lpCurPlayer->AttackSkil , 0 , 0 ,0,0,0 );

			lpCurPlayer->SetMotionFromCode( CHRMOTION_STATE_SKILL );

			//범위형 공격 범위 선정 (원)
			dm_SelectRange( lpCurPlayer->pX, lpCurPlayer->pY, lpCurPlayer->pZ, 160 , FALSE );
			//범위공격 설정후 전송
			dm_SendRangeDamage( lpCurPlayer->pX, lpCurPlayer->pY, lpCurPlayer->pZ, 0 ,0, 0, 0, 0, lpCurPlayer->AttackSkil );

			flag++;
		}
		break;

	case SKILL_VIRTUAL_LIFE:		//버츌얼 라이프
		if ( !lpCharSelPlayer && GetAssaUseEffect(lpCurPlayer,SKILL_VIRTUAL_LIFE)==FALSE && lpSkill->Skill_Info.FuncPointer()==TRUE ) {
			lpCurPlayer->AttackSkil = SKILL_PLAY_VIRTUAL_LIFE;
			lpCurPlayer->AttackSkil |= (lpSkill->Point<<8);				//스킬 포인트 상위 바이트에 저장
			lpCurPlayer->BeginSkill( lpCurPlayer->AttackSkil , 0 , 0 ,0,0,0 );

			lpCurPlayer->SetMotionFromCode( CHRMOTION_STATE_SKILL );

			SendProcessSKillToServer( SKILL_PLAY_VIRTUAL_LIFE , lpSkill->Point , 0, 0 );
			cSkill.SetVirtualLife( Virtual_Life_Time[ lpSkill->Point-1 ] , lpSkill->Point );
			lpCurPlayer->chrAttackTarget = 0;

			flag++;
		}
		break;

		//////////////////////////// 매지션 /////////////////////////////////////
	case SKILL_AGONY:
		if ( lpCurPlayer->MotionInfo->State<0x100 ) {
			if ( lpSkill->Skill_Info.FuncPointer()==TRUE ) {
				lpCurPlayer->AttackSkil = SKILL_PLAY_AGONY;
				lpCurPlayer->AttackSkil |= (lpSkill->Point<<8);				//스킬 포인트 상위 바이트에 저장
				lpCurPlayer->BeginSkill( lpCurPlayer->AttackSkil , 0 , 0 ,0,0,0 );

				lpCurPlayer->SetMotionFromCode( CHRMOTION_STATE_SKILL );
				lpCurPlayer->MotionLoopSpeed = 70;
				lpCurPlayer->MotionLoop = 1;
				flag++;

				//에너지 그래프 첵크 초기화
				ResetEnergyGraph( 3 );
			}
		}
		break;

	case SKILL_ZENITH:
		if ( lpCurPlayer->MotionInfo->State<0x100 ) {
			if ( lpSkill->Skill_Info.FuncPointer()==TRUE ) {
				lpCurPlayer->AttackSkil = SKILL_PLAY_ZENITH;
				lpCurPlayer->AttackSkil |= (lpSkill->Point<<8);				//스킬 포인트 상위 바이트에 저장
				lpCurPlayer->BeginSkill( lpCurPlayer->AttackSkil , 0 , 0 ,0,0,0 );

				lpCurPlayer->SetMotionFromCode( CHRMOTION_STATE_SKILL );
				lpCurPlayer->MotionLoopSpeed = 70;
				lpCurPlayer->MotionLoop = 1;

				//SendProcessSKillToServer( SKILL_PLAY_ZENITH , lpSkill->Point , 0, 0 );
				//속성정보 서버로 전송
				SendResistanceToServer();

				flag++;
			}
		}
		break;

	case SKILL_ENCHANT_WEAPON:
		if ( lpCurPlayer->MotionInfo->State<0x100 && !lpCharSelPlayer && !lpCurPlayer->WeaponEffect ) {
			if ( lpSkill->Skill_Info.FuncPointer()==TRUE ) {
				lpCurPlayer->chrAttackTarget = 0;

				lpCurPlayer->AttackSkil = SKILL_PLAY_ENCHANT_WEAPON;
				lpCurPlayer->AttackSkil |= (lpSkill->Point<<8);				//스킬 포인트 상위 바이트에 저장
				cnt = rand()%3;
				lpCurPlayer->AttackSkil |= ( cnt<<12 );
				lpCurPlayer->BeginSkill( lpCurPlayer->AttackSkil , 0 , 0 ,0,0,0 );

				lpCurPlayer->SetMotionFromCode( CHRMOTION_STATE_SKILL );
				lpCurPlayer->MotionLoopSpeed = 70;
				lpCurPlayer->MotionLoop = 1;
				flag++;
			}
		}
		break;


	case SKILL_ENERGY_SHIELD:
		if ( lpCurPlayer->MotionInfo->State<0x100 ) {
			if ( lpSkill->Skill_Info.FuncPointer()==TRUE ) {
				lpCurPlayer->AttackSkil = SKILL_PLAY_ENERGY_SHIELD;
				lpCurPlayer->AttackSkil |= (lpSkill->Point<<8);				//스킬 포인트 상위 바이트에 저장
				lpCurPlayer->BeginSkill( lpCurPlayer->AttackSkil , 0 , 0 ,0,0,0 );

				lpCurPlayer->SetMotionFromCode( CHRMOTION_STATE_SKILL );
				lpCurPlayer->MotionLoopSpeed = 80;
				lpCurPlayer->MotionLoop = 1;

				SendProcessSKillToServer( SKILL_PLAY_ENERGY_SHIELD , lpSkill->Point , 0, 0 );

				flag++;
			}
		}
		break;

	case SKILL_DIASTROPHISM:
		if ( lpCurPlayer->MotionInfo->State<0x100 ) {
			if ( lpSkill->Skill_Info.FuncPointer()==TRUE ) {

				lpCurPlayer->AttackSkil = SKILL_PLAY_DIASTROPHISM;
				lpCurPlayer->AttackSkil |= (lpSkill->Point<<8);				//스킬 포인트 상위 바이트에 저장
				lpCurPlayer->BeginSkill( lpCurPlayer->AttackSkil , 0 , 0 ,0,0,0 );

				lpCurPlayer->SetMotionFromCode( CHRMOTION_STATE_SKILL );
				lpCurPlayer->MotionLoopSpeed = 120;
				lpCurPlayer->MotionLoop = 1;
				flag++;
/*
				//범위형 공격 범위 선정 (원)
				dm_SelectRange( lpCurPlayer->pX, lpCurPlayer->pY, lpCurPlayer->pZ, Diastrophism_Area[lpSkill->Point-1] , FALSE );

				//범위공격 설정후 전송
				dm_SendRangeDamage( lpCurPlayer->pX, lpCurPlayer->pY, lpCurPlayer->pZ, 0,
					cInvenTory.InvenItem[ sInven[0].ItemIndex-1 ].sItemInfo.Damage[0] ,
					cInvenTory.InvenItem[ sInven[0].ItemIndex-1 ].sItemInfo.Damage[1] ,
					0, 0, lpCurPlayer->AttackSkil );
*/

			}
		}
		break;

	case SKILL_SPIRIT_ELEMENTAL:
		if ( lpCurPlayer->MotionInfo->State<0x100 ) {
			if ( lpSkill->Skill_Info.FuncPointer()==TRUE ) {

				lpCurPlayer->AttackSkil = SKILL_PLAY_SPIRIT_ELEMENTAL;
				lpCurPlayer->AttackSkil |= (lpSkill->Point<<8);				//스킬 포인트 상위 바이트에 저장
				lpCurPlayer->BeginSkill( lpCurPlayer->AttackSkil , 0 , 0 ,0,0,0 );

				lpCurPlayer->SetMotionFromCode( CHRMOTION_STATE_SKILL );
				lpCurPlayer->MotionLoopSpeed = 80;
				lpCurPlayer->MotionLoop = 1;

				SendProcessSKillToServer( SKILL_PLAY_SPIRIT_ELEMENTAL , lpSkill->Point , 0, 0 );

				flag++;
			}
		}
		break;

	case SKILL_DANCING_SWORD:
		if ( lpCurPlayer->MotionInfo->State<0x100 ) {
			if ( lpSkill->Skill_Info.FuncPointer()==TRUE ) {

				lpCurPlayer->AttackSkil = SKILL_PLAY_DANCING_SWORD;
				lpCurPlayer->AttackSkil |= (lpSkill->Point<<8);				//스킬 포인트 상위 바이트에 저장
				cnt = rand()%2;
				lpCurPlayer->AttackSkil |= ( cnt<<12 );
				lpCurPlayer->BeginSkill( lpCurPlayer->AttackSkil , 0 , 0 ,0,0,0 );

				lpCurPlayer->SetMotionFromCode( CHRMOTION_STATE_SKILL );
				lpCurPlayer->MotionLoopSpeed = 80;
				lpCurPlayer->MotionLoop = 1;

				SendProcessSKillToServer( SKILL_PLAY_DANCING_SWORD , lpSkill->Point , cnt, 0 );

				flag++;
			}
			
		}
		break;


	////////////////////// 4차 스킬 ////////////////////////
	///메카니션
	case SKILL_COMPULSION:
		if ( lpCurPlayer->MotionInfo->State<0x100 ) {
			if ( lpSkill->Skill_Info.FuncPointer()==TRUE ) {

				lpCurPlayer->AttackSkil = SKILL_PLAY_COMPULSION;
				lpCurPlayer->AttackSkil |= (lpSkill->Point<<8);				//스킬 포인트 상위 바이트에 저장
				lpCurPlayer->BeginSkill( lpCurPlayer->AttackSkil , 0 , 0 ,0,0,0 );

				lpCurPlayer->SetMotionFromCode( CHRMOTION_STATE_SKILL );
				lpCurPlayer->MotionLoopSpeed = 80;
				lpCurPlayer->MotionLoop = 1;

				//범위형 공격 범위 선정 (원)
				dm_SelectRange( lpCurPlayer->pX, lpCurPlayer->pY, lpCurPlayer->pZ, Compulsion_Area[lpSkill->Point-1] , FALSE );
				//범위공격 설정후 전송
				dm_SendRangeDamage( lpCurPlayer->pX, lpCurPlayer->pY, lpCurPlayer->pZ, 0 ,0, 0, 0, 0, lpCurPlayer->AttackSkil );

				flag++;
			}
		}
		break;

	case SKILL_MAGNETIC_SPHERE:		//댄싱소드랑 비슷하다고 함
		if ( lpCurPlayer->MotionInfo->State<0x100 ) {
			if ( lpSkill->Skill_Info.FuncPointer()==TRUE ) {

				lpCurPlayer->AttackSkil = SKILL_PLAY_MAGNETIC_SPHERE;
				lpCurPlayer->AttackSkil |= (lpSkill->Point<<8);				//스킬 포인트 상위 바이트에 저장
				lpCurPlayer->BeginSkill( lpCurPlayer->AttackSkil , 0 , 0 ,0,0,0 );

				lpCurPlayer->SetMotionFromCode( CHRMOTION_STATE_SKILL );
				lpCurPlayer->MotionLoopSpeed = 80;
				lpCurPlayer->MotionLoop = 1;

				SendProcessSKillToServer( SKILL_PLAY_MAGNETIC_SPHERE , lpSkill->Point , 0, 0 );

				flag++;
			}
		}
		break;

	case SKILL_METAL_GOLEM:
		if ( lpCurPlayer->MotionInfo->State<0x100 ) {
			if ( lpSkill->Skill_Info.FuncPointer()==TRUE ) {

				lpCurPlayer->AttackSkil = SKILL_PLAY_METAL_GOLEM;
				lpCurPlayer->AttackSkil |= (lpSkill->Point<<8);				//스킬 포인트 상위 바이트에 저장
				lpCurPlayer->BeginSkill( lpCurPlayer->AttackSkil , 0 , 0 ,0,0,0 );

				lpCurPlayer->SetMotionFromCode( CHRMOTION_STATE_SKILL );
				lpCurPlayer->MotionLoopSpeed = 80;
				lpCurPlayer->MotionLoop = 1;

				SendProcessSKillToServer( SKILL_PLAY_METAL_GOLEM , lpSkill->Point , 0, 0 );

				flag++;
			}
		}
		break;

	///파이터
	case SKILL_BERSERKER:
		if ( lpCurPlayer->MotionInfo->State<0x100 ) {
			if ( lpSkill->Skill_Info.FuncPointer()==TRUE ) {

				lpCurPlayer->AttackSkil = SKILL_PLAY_BERSERKER;
				lpCurPlayer->AttackSkil |= (lpSkill->Point<<8);				//스킬 포인트 상위 바이트에 저장
				lpCurPlayer->BeginSkill( lpCurPlayer->AttackSkil , 0 , 0 ,0,0,0 );

				lpCurPlayer->SetMotionFromCode( CHRMOTION_STATE_SKILL );
				lpCurPlayer->MotionLoopSpeed = 80;
				lpCurPlayer->MotionLoop = 1;

				SendProcessSKillToServer( SKILL_PLAY_BERSERKER , lpSkill->Point , 0, 0 );

				flag++;
			}
		}
		break;

	case SKILL_BLOODY_BERSERKER:		// tier 5
		if ( lpCurPlayer->MotionInfo->State<0x100 ) {
			if ( lpSkill->Skill_Info.FuncPointer()==TRUE ) {

				lpCurPlayer->AttackSkil = SKILL_PLAY_BLOODY_BERSERKER;
				lpCurPlayer->AttackSkil |= (lpSkill->Point<<8);
				lpCurPlayer->BeginSkill( lpCurPlayer->AttackSkil , 0 , 0 ,0,0,0 );

				lpCurPlayer->SetMotionFromCode( CHRMOTION_STATE_SKILL );
				lpCurPlayer->MotionLoopSpeed = 70;
				lpCurPlayer->MotionLoop = 1;

				SendProcessSKillToServer( SKILL_PLAY_BLOODY_BERSERKER , lpSkill->Point , 0, 0 );

				flag++;
			}
		}
		break;

	//Mechanician tier 5 buffs
	//tier 5 self buffs: the client keeps the timer; those the server counts are sent to it
	case SKILL_HYPER_SONIC:			//Mechanician: attack speed, any weapon (the client counts it)
	case SKILL_POWER_ENHANCEMENT:	//Mechanician: attack power (the server)
	case SKILL_AMPLIFIED:			//Pikeman: damage (the server), attack rating and absorption (the client)
	case SKILL_HOLY_BENEDIC:		//Knight: damage against the undead (the server)
	case SKILL_TALARIA:				//Atalanta: move speed (the client)
	case SKILL_SUMMON_ARCUDA:		//Atalanta: the server summons Arcuda
	case SKILL_SUMMON_RAMIEL:		//Priestess: an angel striking on a timer (Summon Muspell's slot)
	case SKILL_BLESSING_KRISHNA:	//Priestess: defense and absorption (the client)
	case SKILL_THIRD_ANIMA:			//Magician: magic damage (the server), less max mana (the client)
		if ( lpCurPlayer->MotionInfo->State<0x100 ) {
			int t5play = 0, t5server = 0;
			switch( lpSkill->CODE ) {
			case SKILL_SUMMON_RAMIEL:		t5play = SKILL_PLAY_SUMMON_RAMIEL;		t5server = 1;	break;
			case SKILL_BLESSING_KRISHNA:	t5play = SKILL_PLAY_BLESSING_KRISHNA;					break;
			case SKILL_THIRD_ANIMA:			t5play = SKILL_PLAY_THIRD_ANIMA;		t5server = 1;	break;
			case SKILL_TALARIA:				t5play = SKILL_PLAY_TALARIA;							break;
			case SKILL_SUMMON_ARCUDA:		t5play = SKILL_PLAY_SUMMON_ARCUDA;		t5server = 1;	break;
			case SKILL_HOLY_BENEDIC:		t5play = SKILL_PLAY_HOLY_BENEDIC;		t5server = 1;	break;
			case SKILL_HYPER_SONIC:			t5play = SKILL_PLAY_HYPER_SONIC;							break;
			case SKILL_POWER_ENHANCEMENT:	t5play = SKILL_PLAY_POWER_ENHANCEMENT;	t5server = 1;	break;
			case SKILL_AMPLIFIED:			t5play = SKILL_PLAY_AMPLIFIED;			t5server = 1;	break;
			}
			if ( t5play && lpSkill->Skill_Info.FuncPointer()==TRUE && lpSkill->Point ) {
				lpCurPlayer->AttackSkil = t5play;
				lpCurPlayer->AttackSkil |= (lpSkill->Point<<8);
				lpCurPlayer->BeginSkill( lpCurPlayer->AttackSkil , 0 , 0 ,0,0,0 );

				lpCurPlayer->SetMotionFromCode( CHRMOTION_STATE_SKILL );
				lpCurPlayer->MotionLoopSpeed = 70;
				lpCurPlayer->MotionLoop = 1;

				if ( t5server )
					SendProcessSKillToServer( t5play , lpSkill->Point , 0, 0 );

				flag++;
			}
		}
		break;
/*
	case SKILL_CYCLONE_STRIKE:
		//CYCLONE_STRIKE
		if ( lpCurPlayer->MotionInfo->State<0x100 && !lpCharSelPlayer ) {
			if ( lpSkill->Skill_Info.FuncPointer()==TRUE ) {

				lpCurPlayer->AttackSkil = SKILL_PLAY_CYCLONE_STRIKE;
				lpCurPlayer->AttackSkil |= (lpSkill->Point<<8);				//스킬 포인트 상위 바이트에 저장
				lpCurPlayer->BeginSkill( lpCurPlayer->AttackSkil , 0 , 0 ,0,0,0 );
				lpCurPlayer->SetMotionFromCode( CHRMOTION_STATE_SKILL );

				//범위형 공격 범위 선정 (원)
				dm_SelectRange( lpCurPlayer->pX, lpCurPlayer->pY, lpCurPlayer->pZ, Cyclone_Strike_Area[lpSkill->Point-1] , TRUE , Cyclone_Strike_AttackNum[lpSkill->Point-1] );
				//범위공격 설정후 전송
				dm_SendRangeDamage( lpCurPlayer->pX, lpCurPlayer->pY, lpCurPlayer->pZ,  0 , 0 , 0 ,0,0, lpCurPlayer->AttackSkil );

				flag++;
			}
		}
		return TRUE;
*/

	//파이크맨
		/*
	case SKILL_ASSASSIN_EYE:
		if ( lpCurPlayer->MotionInfo->State<0x100 && !lpCharSelPlayer ) {
			if ( lpSkill->Skill_Info.FuncPointer()==TRUE ) {

				lpCurPlayer->AttackSkil = SKILL_PLAY_ASSASSIN_EYE;
				lpCurPlayer->AttackSkil |= (lpSkill->Point<<8);				//스킬 포인트 상위 바이트에 저장
				lpCurPlayer->BeginSkill( lpCurPlayer->AttackSkil , 0 , 0 ,0,0,0 );

				lpCurPlayer->SetMotionFromCode( CHRMOTION_STATE_SKILL );
				lpCurPlayer->MotionLoopSpeed = 80;
				lpCurPlayer->MotionLoop = 1;

				SendProcessSKillToServer( SKILL_PLAY_ASSASSIN_EYE , lpSkill->Point , 0, 0 );

				flag++;
			}
		}
		break;
		*/
	case SKILL_VAGUE:
		if ( lpCurPlayer->MotionInfo->State<0x100 ) {
			if ( lpSkill->Skill_Info.FuncPointer()==TRUE ) {

				lpCurPlayer->AttackSkil = SKILL_PLAY_VAGUE;
				lpCurPlayer->AttackSkil |= (lpSkill->Point<<8);				//스킬 포인트 상위 바이트에 저장
				lpCurPlayer->BeginSkill( lpCurPlayer->AttackSkil , 0 , 0 ,0,0,0 );

				lpCurPlayer->SetMotionFromCode( CHRMOTION_STATE_SKILL );
				lpCurPlayer->MotionLoopSpeed = 80;
				lpCurPlayer->MotionLoop = 1;

				SendProcessSKillToServer( SKILL_PLAY_VAGUE , lpSkill->Point , 0, 0 );

				flag++;
			}
		}
		break;

	//아처
	case SKILL_RECALL_WOLVERIN:
		if ( lpCurPlayer->MotionInfo->State<0x100 ) {
			if ( lpSkill->Skill_Info.FuncPointer()==TRUE ) {

				lpCurPlayer->AttackSkil = SKILL_PLAY_RECALL_WOLVERIN;
				lpCurPlayer->AttackSkil |= (lpSkill->Point<<8);				//스킬 포인트 상위 바이트에 저장
				lpCurPlayer->BeginSkill( lpCurPlayer->AttackSkil , 0 , 0 ,0,0,0 );

				lpCurPlayer->SetMotionFromCode( CHRMOTION_STATE_SKILL );
				lpCurPlayer->MotionLoopSpeed = 80;
				lpCurPlayer->MotionLoop = 1;
				lpCurPlayer->HideWeapon = TRUE;
				flag++;

				SendProcessSKillToServer( SKILL_PLAY_RECALL_WOLVERIN , lpSkill->Point , 0, 0 );
			}
		}
		break;

	case SKILL_FORCE_OF_NATURE:
		if ( lpCurPlayer->MotionInfo->State<0x100 ) {
			if ( lpSkill->Skill_Info.FuncPointer()==TRUE ) {

				lpCurPlayer->AttackSkil = SKILL_PLAY_FORCE_OF_NATURE;
				lpCurPlayer->AttackSkil |= (lpSkill->Point<<8);				//스킬 포인트 상위 바이트에 저장
				lpCurPlayer->BeginSkill( lpCurPlayer->AttackSkil , 0 , 0 ,0,0,0 );

				lpCurPlayer->SetMotionFromCode( CHRMOTION_STATE_SKILL );
				lpCurPlayer->MotionLoopSpeed = 80;
				lpCurPlayer->MotionLoop = 1;

				SendProcessSKillToServer( SKILL_PLAY_FORCE_OF_NATURE , lpSkill->Point , 0, 0 );
				cSkill.ForeceOfNature( Force_Of_Nature_Time[lpSkill->Point-1] , lpSkill->Point );

				lpCurPlayer->dwForceOfNatureTime = dwPlayTime+Force_Of_Nature_Time[lpSkill->Point-1]*1000;	//포스오브 네이쳐 유지시간

				flag++;
			}
		}
		break;

	//나이트
	case SKILL_GODLY_SHIELD:
		if ( lpCurPlayer->MotionInfo->State<0x100 ) {
			if ( lpSkill->Skill_Info.FuncPointer()==TRUE ) {

				lpCurPlayer->AttackSkil = SKILL_PLAY_GODLY_SHIELD;
				lpCurPlayer->AttackSkil |= (lpSkill->Point<<8);				//스킬 포인트 상위 바이트에 저장
				lpCurPlayer->BeginSkill( lpCurPlayer->AttackSkil , 0 , 0 ,0,0,0 );

				lpCurPlayer->SetMotionFromCode( CHRMOTION_STATE_SKILL );
				lpCurPlayer->MotionLoopSpeed = 80;
				lpCurPlayer->MotionLoop = 1;

				SendProcessSKillToServer( SKILL_PLAY_GODLY_SHIELD , lpSkill->Point , 0, 0 );
				flag++;
			}
		}
		break;

	case SKILL_GOD_BLESS:
		if ( lpCurPlayer->MotionInfo->State<0x100 ) {
			if ( lpSkill->Skill_Info.FuncPointer()==TRUE ) {

				lpCurPlayer->AttackSkil = SKILL_PLAY_GODS_BLESS;
				lpCurPlayer->AttackSkil |= (lpSkill->Point<<8);				//스킬 포인트 상위 바이트에 저장
				lpCurPlayer->BeginSkill( lpCurPlayer->AttackSkil , 0 , 0 ,0,0,0 );

				lpCurPlayer->SetMotionFromCode( CHRMOTION_STATE_SKILL );
				lpCurPlayer->MotionLoopSpeed = 80;
				lpCurPlayer->MotionLoop = 1;

				SendProcessSKillToServer( SKILL_PLAY_GODS_BLESS , lpSkill->Point , 0, 0 );
				flag++;
			}
		}
		break;

	//아탈란타
	case SKILL_HALL_OF_VALHALLA:
		if ( lpCurPlayer->MotionInfo->State<0x100 ) {
			if ( GetAssaUseEffect(lpCurPlayer,SKILL_TRIUMPH_OF_VALHALLA)==FALSE && lpSkill->Skill_Info.FuncPointer()==TRUE ) {

				lpCurPlayer->AttackSkil = SKILL_PLAY_HALL_OF_VALHALLA;
				lpCurPlayer->AttackSkil |= (lpSkill->Point<<8);				//스킬 포인트 상위 바이트에 저장
				lpCurPlayer->BeginSkill( lpCurPlayer->AttackSkil , 0 , 0 ,0,0,0 );

				lpCurPlayer->SetMotionFromCode( CHRMOTION_STATE_SKILL );
				lpCurPlayer->MotionLoopSpeed = 80;
				lpCurPlayer->MotionLoop = 1;

				cnt = cSkill.GetSkillPoint( SKILL_TRIUMPH_OF_VALHALLA );			//트라이엄프발할라의 포인트
				SendProcessSKillToServer( SKILL_PLAY_HALL_OF_VALHALLA , lpSkill->Point , cnt, lpCurPlayer->smCharInfo.Level );

				cSkill.HellOfValhalla( Hall_Of_Valhalla_Time[lpSkill->Point-1] , lpSkill->Point , cnt , lpCurPlayer->smCharInfo.Level , 0 );

				lpCurPlayer->dwHallOfValhallaTime = dwPlayTime+Hall_Of_Valhalla_Time[lpSkill->Point-1]*1000;	//홀오브발할라 유지시간

				flag++;
			}
		}
		break;

	case SKILL_FROST_JAVELIN:
		if ( lpCurPlayer->MotionInfo->State<0x100 ) {
			if ( lpSkill->Skill_Info.FuncPointer()==TRUE ) {

				lpCurPlayer->AttackSkil = SKILL_PLAY_FROST_JAVELIN;
				lpCurPlayer->AttackSkil |= (lpSkill->Point<<8);				//스킬 포인트 상위 바이트에 저장
				lpCurPlayer->BeginSkill( lpCurPlayer->AttackSkil , 0 , 0 ,0,0,0 );

				lpCurPlayer->SetMotionFromCode( CHRMOTION_STATE_SKILL );
				lpCurPlayer->MotionLoopSpeed = 80;
				lpCurPlayer->MotionLoop = 1;

				SendProcessSKillToServer( SKILL_PLAY_FROST_JAVELIN , lpSkill->Point , 0, 0 );
				flag++;
			}
		}
		break;

	//프리스티스
	case SKILL_REGENERATION_FIELD:
		if ( lpCurPlayer->MotionInfo->State<0x100 ) {
			if ( lpSkill->Skill_Info.FuncPointer() == TRUE ){ // && InterfaceParty.PartyMemberCount>0 && lpSkill->PartyFlag && lpSkill->SkillTaget_CODE != 0) {

				lpCurPlayer->AttackSkil = SKILL_PLAY_REGENERATION_FIELD;
				lpCurPlayer->AttackSkil |= (lpSkill->Point<<8);				//스킬 포인트 상위 바이트에 저장
				lpCurPlayer->BeginSkill( lpCurPlayer->AttackSkil , 0 , 0 ,0,0,0 );

				lpCurPlayer->SetMotionFromCode( CHRMOTION_STATE_SKILL );
				lpCurPlayer->MotionLoopSpeed = 80;
				lpCurPlayer->MotionLoop = 1;

				cSkill.Regeneration_Field( Regeneration_Field_Time[lpSkill->Point-1] , lpSkill->Point , 0 );		//재생력 추가
				flag++;
			}
		}
		break;

	case SKILL_SUMMON_MUSPELL:
		if ( lpCurPlayer->MotionInfo->State<0x100 ) {
			if ( lpSkill->Skill_Info.FuncPointer()==TRUE ) {

				lpCurPlayer->AttackSkil = SKILL_PLAY_SUMMON_MUSPELL;
				lpCurPlayer->AttackSkil |= (lpSkill->Point<<8);				//스킬 포인트 상위 바이트에 저장
				lpCurPlayer->BeginSkill( lpCurPlayer->AttackSkil , 0 , 0 ,0,0,0 );

				lpCurPlayer->SetMotionFromCode( CHRMOTION_STATE_SKILL );
				lpCurPlayer->MotionLoopSpeed = 80;
				lpCurPlayer->MotionLoop = 1;

				SendProcessSKillToServer( SKILL_PLAY_SUMMON_MUSPELL , lpSkill->Point , 0, 0 );

				flag++;
			}
		}
		break;

	//매지션
	case SKILL_FIRE_ELEMENTAL:
		if ( lpCurPlayer->MotionInfo->State<0x100 ) {
			if ( lpSkill->Skill_Info.FuncPointer()==TRUE ) {

				lpCurPlayer->AttackSkil = SKILL_PLAY_FIRE_ELEMENTAL;
				lpCurPlayer->AttackSkil |= (lpSkill->Point<<8);				//스킬 포인트 상위 바이트에 저장
				lpCurPlayer->BeginSkill( lpCurPlayer->AttackSkil , 0 , 0 ,0,0,0 );

				lpCurPlayer->SetMotionFromCode( CHRMOTION_STATE_SKILL );
				lpCurPlayer->MotionLoopSpeed = 80;
				lpCurPlayer->MotionLoop = 1;

				SendProcessSKillToServer( SKILL_PLAY_FIRE_ELEMENTAL , lpSkill->Point , 0, 0 );

				flag++;
			}
		}
		break;
	case SKILL_DISTORTION:
		if ( lpCurPlayer->MotionInfo->State<0x100 ) {
			if ( lpSkill->Skill_Info.FuncPointer()==TRUE ) {

				lpCurPlayer->AttackSkil = SKILL_PLAY_DISTORTION;
				lpCurPlayer->AttackSkil |= (lpSkill->Point<<8);				//스킬 포인트 상위 바이트에 저장
				lpCurPlayer->BeginSkill( lpCurPlayer->AttackSkil , 0, 0 ,0,0,0 );

				lpCurPlayer->SetMotionFromCode( CHRMOTION_STATE_SKILL );
				lpCurPlayer->MotionLoopSpeed = 80;
				lpCurPlayer->MotionLoop = 1;

				//범위형 공격 범위 선정 (원)
				dm_SelectRange( lpCurPlayer->pX, lpCurPlayer->pY, lpCurPlayer->pZ, Distortion_Area[lpSkill->Point-1] , FALSE );
				//범위공격 설정후 전송
				dm_SendRangeDamage( lpCurPlayer->pX, lpCurPlayer->pY, lpCurPlayer->pZ, 0 ,0, 0, 0, 0, lpCurPlayer->AttackSkil );
								
/*
				//테스트용
				if ( dmSelected_CharCnt ) {
					for(cnt=0;cnt<dmSelected_CharCnt;cnt++) {
						lpSelected_Char[cnt]->PlayDistortion = 70*5;
					}
				}
				
				*/
				flag++;
			}
			
		}
		break;


	}

	if ( flag ) CheckPlaySkill();		////스킬감시

	return flag;
}


int	NormalAttackRange;			//기본 공격 거리
int	NormalAttackMode;
int	LastAttackRange;

//스킬 적용 거리를 구한다
int GetSkillDistRange( sSKILL *lpSkill , int AttackRange , int AttackMode )
{
	int cnt;

	NormalAttackRange = AttackRange;
	NormalAttackMode = AttackMode;

	cnt = NC_SkillDistRange( lpSkill , AttackRange );		// the new classes' magic and debuffs
	if ( cnt ) {
		LastAttackRange = cnt;
		return cnt;
	}

	switch( lpSkill->CODE ) {
	case SKILL_MECHANIC_BOMB:
		if ( lpSkill->Point ) {
			//메카닉범 사정거리 
			cnt = Mechanic_Bomb_Attack_Range[lpSkill->Point-1];
			return	(cnt*3*fONE);
		}
		break;

	case SKILL_SPARK:
		if ( lpSkill->Point ) {
			//스파크 사정거리 
			return	140*fONE;
		}
		break;

	case SKILL_EXPANSION:
		//익스팬션
		if ( lpSkill->Point ) {
			cnt = AttackRange+((AttackRange-50*fONE) * Expansion_Size[lpSkill->Point-1])/100;
			LastAttackRange = cnt;
			return cnt;
		}
		break;

	case SKILL_VENGEANCE:
		if ( lpSkill->Point ) {
			cnt = AttackRange+Vengeance_Range[lpSkill->Point-1]*fONE;
			LastAttackRange = cnt;
			return cnt;
		}
		break;

	case SKILL_TORNADO:
		//토네이도
		if ( lpSkill->Point ) {
			cnt = Tornado_Range[lpSkill->Point-1];
			if ( AttackRange>cnt ) break;
			LastAttackRange = cnt;
			return cnt;
		}
		break;

	case SKILL_SWORD_BLAST:
		//소드 블래스트
		if ( lpSkill->Point ) {
			cnt = Sword_Blast_ShootingRange[lpSkill->Point-1];
			if ( AttackRange>cnt ) break;
			LastAttackRange = cnt;
			return cnt;
		}
		break;

	case SKILL_SHIELD_STRIKE:
		//쉴드 스트라이크
		return	110*fONE;

	case SKILL_HEALING:
		//힐링
		return	180*fONE;
	case SKILL_HOLY_BOLT:
		//홀리볼트
		return	170*fONE;
	case SKILL_MULTISPARK:
		//멀티스파크
		return	210*fONE;
	case SKILL_HOLY_MIND:
		//홀리마인드
		return	160*fONE;

	case SKILL_DIVINE_LIGHTNING:
		return 160*fONE;

	case SKILL_FIRE_BOLT:
		if ( lpSkill->Point ) {
			//파이어 볼트
			return	200*fONE;
		}
		break;
	case SKILL_FIRE_BALL:
		if ( lpSkill->Point ) {
			//파이어 볼
			return	FireBall_Range[lpSkill->Point-1]*fONE;
		}
		break;

	case SKILL_WATORNADO:
		if ( lpSkill->Point ) {
			return Watornado_Range[lpSkill->Point-1]*fONE;
		}
		break;

	case SKILL_ENCHANT_WEAPON:
		return	140*fONE;

	case SKILL_DEAD_RAY:
		if ( lpSkill->Point ) {
			return	Dead_Ray_Range[lpSkill->Point-1]*fONE;
		}
		break;

	case SKILL_HOLY_INCANTATION:
		return	120*fONE;

	case SKILL_VIGOR_BALL:
		return	230*fONE;

	case SKILL_TRIUMPH_OF_VALHALLA:
		return	200*fONE;

	case SKILL_PERFORATION:
		if ( lpSkill->Point ) {
			return	AttackRange+(Perforation_Attack_Range[lpSkill->Point-1]*fONE);
		}
		break;

	case SKILL_VIRTUAL_LIFE:
		return 200*fONE;

	//tier 5
	case SKILL_CRESCENT_MOON:
		if ( lpSkill->Point ) {
			cnt = Crescent_Moon_Range[lpSkill->Point-1]*fONE;
			if ( AttackRange>cnt ) break;
			LastAttackRange = cnt;
			return cnt;
		}
		break;

	case SKILL_SPIRIT_IMPACT:
		if ( lpSkill->Point ) return Spirit_Impact_Range[lpSkill->Point-1]*fONE;
		break;
	case SKILL_SILRAPHIM:
		if ( lpSkill->Point ) return Silraphim_Range[lpSkill->Point-1]*fONE;
		break;
	case SKILL_PIERCING_ICE:
	case SKILL_PRIMA_IGNIS:
		return 200*fONE;
	case SKILL_VIS_TENUS:
		return 180*fONE;

	case SKILL_SNIPPY_FEAR:
		if ( lpSkill->Point ) {
			cnt = AttackRange+Snippy_Fear_Range[lpSkill->Point-1]*fONE;
			LastAttackRange = cnt;
			return cnt;
		}
		break;

	case SKILL_THUNDER_ROPE:
		if ( lpSkill->Point ) {
			cnt = AttackRange+Thunder_Rope_Range[lpSkill->Point-1]*fONE;
			LastAttackRange = cnt;
			return cnt;
		}
		break;

	}

	return NULL;
}

//공격 재시도
int RetryPlayAttack( smCHAR *lpChar ) {

	return lpCurPlayer->PlayAttackFromPosi( lpChar->pX , lpChar->pY , lpChar->pZ ,
							NormalAttackRange , NormalAttackMode ) ;

}


//라이프 감소
int PlaySkillSubLife( int Life )
{
	int sLife;

	CheckCharForm();
	//플레이어 죽음
	sLife = sinGetLife();
	sLife -= Life;
	if ( sLife<=0 ) sLife = 1;
	sinSetLife( sLife );
	//ReformCharForm();					//캐릭터 정보 인증

	return sLife;
}


//스킬 공격을 시도
int PlaySkillAttack( sSKILL *lpSkill , smCHAR *lpChar )
{

	int cnt;
	int	temp;//,temp2,temp3;
	int x,y,z;
	smTRANS_COMMAND	smTransCommand;
	RECT	rect;
	smWINSOCK	*lpsmSock;
	short wpDamage[2];
	int	 skFlag;

	lpCurPlayer->AttackSkil = 0;

	if ( !lpSkill ) return FALSE;

	if ( lpSkill->CODE==SKILL_HEALING || lpSkill->CODE==SKILL_ENCHANT_WEAPON || 
		lpSkill->CODE==SKILL_VIRTUAL_LIFE || lpSkill->CODE==SKILL_TRIUMPH_OF_VALHALLA ) {
			skFlag = TRUE;
		}
	else {
		skFlag = FALSE;
	}


	if ( !PkMode && lpChar->smCharInfo.State!=smCHAR_STATE_ENEMY && skFlag==FALSE )
		return FALSE;

	if ( sinCheckSkillUseOk( lpSkill )==FALSE || lpSkill->Point>10 ) {
		if ( lpSkill==sinSkill.pLeftSkill && RetryPlayAttack( lpChar ) ) return FALSE;;
		return TRUE;
	}

	CheckPlaySkill();		////스킬감시

	pUseSkill = lpSkill;


	wpDamage[0] = cInvenTory.InvenItem[ sInven[0].ItemIndex-1 ].sItemInfo.Damage[0];
	wpDamage[1] = cInvenTory.InvenItem[ sInven[0].ItemIndex-1 ].sItemInfo.Damage[1];

	lpSkill->SkillTaget_CODE = lpChar->dwObjectSerial;

	cnt = NC_PlaySkillAttack( lpSkill , lpChar , wpDamage );		// the new classes' target casts
	if ( cnt>=0 ) return cnt;

	switch( lpSkill->CODE ) {
	case SKILL_EXTREME_SHIELD:
		//익스트림 쉴드
		return TRUE;

	case SKILL_MECHANIC_BOMB:
		//메카닉범
		if ( lpSkill->Skill_Info.FuncPointer()==TRUE ) {
			lpCurPlayer->AttackSkil = SKILL_PLAY_MECHANIC_BOMB;
			lpCurPlayer->AttackSkil |= (lpSkill->Point<<8);				//스킬 포인트 상위 바이트에 저장
			lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil,0,lpChar,0,0,0);
			lpCurPlayer->SetMotionFromCode( CHRMOTION_STATE_SKILL );
			lpCurPlayer->HideWeapon = TRUE;
		}
		return TRUE;

	case SKILL_PHYSICAL_ABSORB:
		return TRUE;

	case SKILL_RAVING:				//레이빙
		if ( lpSkill->Skill_Info.FuncPointer()==TRUE ) {
			RetryPlayAttack( lpChar ); //공격 재시도
			lpCurPlayer->SetMotionFromCode( CHRMOTION_STATE_ATTACK );

			lpCurPlayer->AttackSkil = SKILL_PLAY_RAVING;
			lpCurPlayer->AttackSkil |= (lpSkill->Point<<8);				//스킬 포인트 상위 바이트에 저장
			lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil,0,lpChar,0,0,0);

			//공격 연사와 속도 설정
			if ( lpSkill->Point ) {
				lpCurPlayer->MotionLoop = Ravind_Speed[lpSkill->Point-1];
				//cnt = lpCurPlayer->SendTransAttack( lpChar , smWsockServer , 0 , Raving_Damage[lpSkill->Point-1] );
				cnt = dm_SendTransDamage( lpChar , 0,0 , 0, 0, lpCurPlayer->AttackSkil , TRUE );

				if ( !cnt ) { 
					lpCurPlayer->AttackCritcal = -1;				//빗나감
				}
				else {
					//라이프 감소
					PlaySkillSubLife( ((int)(Raving_UseLife[lpSkill->Point-1]*10)*lpCurPlayer->smCharInfo.Life[1])/1000 );
					//데미지에 따른 경험치 상승
					DamageExp( lpChar , cnt );
				}

			}
			else {
				lpCurPlayer->MotionLoop = 1;
			}

			lpCurPlayer->MotionLoopSpeed = 90+(10*lpCurPlayer->MotionLoop);
			if ( lpCurPlayer->MotionLoopSpeed<100 ) lpCurPlayer->MotionLoopSpeed = 100;


	//lpCurPlayer->MotionLoopSpeed = (80*(fONE + 32*cnt))>>FLOATNS;


			lpCurPlayer->AttackAnger = 150;
			return TRUE;
		}
		if ( lpSkill==sinSkill.pLeftSkill && RetryPlayAttack( lpChar ) ) break;
		return TRUE;

	case SKILL_IMPACT:				//임팩트

		if ( lpSkill->Skill_Info.FuncPointer()==TRUE ) {
			RetryPlayAttack( lpChar ); //공격 재시도
			lpCurPlayer->AttackSkil = SKILL_PLAY_IMPACT;
			lpCurPlayer->AttackSkil |= (lpSkill->Point<<8);				//스킬 포인트 상위 바이트에 저장
			lpCurPlayer->BeginSkill( lpCurPlayer->AttackSkil,0,lpChar,0,0,0);
			lpCurPlayer->SetMotionFromCode( CHRMOTION_STATE_SKILL );
/*
			//공격속도
			cnt = lpCurPlayer->smCharInfo.Attack_Speed-6;
			if ( cnt<0 ) cnt=0;
			if ( cnt>6 ) cnt=6;
			cnt+=2;					//속도 +2
			lpCurPlayer->MotionLoopSpeed = (80*(fONE + 32*cnt))>>FLOATNS;
			lpCurPlayer->MotionLoop = 1;
*/
			//공격 연사와 속도 설정
			if ( lpSkill->Point ) {
				temp = lpCurPlayer->smCharInfo.Attack_Rating;
				lpCurPlayer->smCharInfo.Attack_Rating += Impact_Attack_Rating[lpSkill->Point-1];
				//cnt = lpCurPlayer->SendTransAttack( lpChar , smWsockServer , 0 , Impact_Damage[lpSkill->Point-1] );
				cnt = dm_SendTransDamage( lpChar , 0,0 , 0, 0, lpCurPlayer->AttackSkil , TRUE );
				lpCurPlayer->smCharInfo.Attack_Rating = temp;

				if ( !cnt )
					lpCurPlayer->AttackCritcal = -1;				//빗나감
				else {
					//데미지에 따른 경험치 상승
					DamageExp( lpChar , cnt );
				}
			}
			return TRUE;
		}
		//좌측공격
		if ( lpSkill==sinSkill.pLeftSkill && RetryPlayAttack( lpChar ) ) break;
		return TRUE;

	case SKILL_PIKE_WIND:
		return TRUE;

	case SKILL_CRITICAL_HIT:
		//크리티컬 힛
		if ( lpSkill->Point && lpSkill->Skill_Info.FuncPointer()==TRUE ) {
			RetryPlayAttack( lpChar ); //공격 재시도

			lpCurPlayer->AttackSkil = SKILL_PLAY_CRITICAL_HIT;
			lpCurPlayer->AttackSkil |= (lpSkill->Point<<8);				//스킬 포인트 상위 바이트에 저장
/*
			//크리티컬 재설정
			cnt = lpCurPlayer->smCharInfo.Critical_Hit;
			lpCurPlayer->smCharInfo.Critical_Hit += Critical_Hit_Critical[lpSkill->Point-1];

			if ( (rand()%100)<sinGetCritical(lpChar->smCharInfo.Level)) {
				lpCurPlayer->AttackCritcal = TRUE;			//크리티컬 공격 성공
			}
			else {
				lpCurPlayer->AttackCritcal = FALSE;
			}
			lpCurPlayer->smCharInfo.Critical_Hit = cnt;
*/
			//공격 2번 시도
			cnt = lpCurPlayer->smCharInfo.Attack_Speed-6;
			if ( cnt<0 ) cnt=0;
			if ( cnt>6 ) cnt=6;
			cnt+=2;					//속도 +2.5
			lpCurPlayer->MotionLoopSpeed = (80*(fONE + 32*cnt+32))>>FLOATNS;

			//lpCurPlayer->MotionLoopSpeed = GetAttackSpeedFrame( lpCurPlayer->smCharInfo.Attack_Speed , 2 );
			lpCurPlayer->MotionLoop = 2;
			break;
		}
		//좌측공격
		if ( lpSkill==sinSkill.pLeftSkill && RetryPlayAttack( lpChar ) ) break;
		return TRUE;

	case SKILL_JUMPING_CRASH:
		//점핑 클래시
		if ( lpSkill->Skill_Info.FuncPointer()==TRUE ) {
			/*
			RetryPlayAttack( lpChar ); //공격 재시도
			lpCurPlayer->AttackCritcal = 1;
			lpCurPlayer->BeginSkill(SKILL_PLAY_JUMPING_CRASH,0,lpChar,0,0,0);
			lpCurPlayer->AttackSkil = SKILL_PLAY_JUMPING_CRASH;
			lpCurPlayer->SetMotionFromCode( CHRMOTION_STATE_SKILL );
			*/

			RetryPlayAttack( lpChar ); //공격 재시도
			lpCurPlayer->AttackSkil = SKILL_PLAY_JUMPING_CRASH;
			lpCurPlayer->AttackSkil |= (lpSkill->Point<<8);				//스킬 포인트 상위 바이트에 저장
			lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil,0,lpChar,0,0,0);
			lpCurPlayer->SetMotionFromCode( CHRMOTION_STATE_SKILL );

			//공격 연사와 속도 설정
			if ( lpSkill->Point ) {
				temp = lpCurPlayer->smCharInfo.Attack_Rating;
				//temp2 = lpCurPlayer->smCharInfo.Attack_Damage[1];
				//temp3 = lpCurPlayer->smCharInfo.Attack_Damage[0];
				//명중력 과 최대 데미지 보강
				lpCurPlayer->smCharInfo.Attack_Rating += 
					( lpCurPlayer->smCharInfo.Attack_Rating * Jumping_Crash_Attack_Rating[lpSkill->Point-1])/100;
/*
				lpCurPlayer->smCharInfo.Attack_Damage[0] += 
					(lpCurPlayer->smCharInfo.Attack_Damage[0]*Jumping_Crash_Damage[lpSkill->Point-1])/100;
				lpCurPlayer->smCharInfo.Attack_Damage[1] += 
					(lpCurPlayer->smCharInfo.Attack_Damage[1]*Jumping_Crash_Damage[lpSkill->Point-1])/100;

				cnt = lpCurPlayer->SendTransAttack( lpChar , smWsockServer , 0  );

				lpCurPlayer->smCharInfo.Attack_Damage[1] = temp2;
				lpCurPlayer->smCharInfo.Attack_Damage[0] = temp3;
*/
				cnt = dm_SendTransDamage( lpChar , 0,0 , 0, 0, lpCurPlayer->AttackSkil , TRUE );
				lpCurPlayer->smCharInfo.Attack_Rating = temp;

				if ( !cnt )
					lpCurPlayer->AttackCritcal = -1;				//빗나감
				else 
					DamageExp( lpChar , cnt );					//데미지에 따른 경험치 상승
			}
			lpCurPlayer->MotionLoopSpeed = 80;
			lpCurPlayer->MotionLoop = 1;
			return TRUE;
		}
		//좌측공격
		if ( lpSkill==sinSkill.pLeftSkill && RetryPlayAttack( lpChar ) ) break;
		return TRUE;

	case SKILL_SCOUT_HAWK:
		//스카우트 호크
		return TRUE;

	case SKILL_WIND_ARROW:
		//윈드 에로우 

		if ( lpCurPlayer->CheckShootingTest(lpChar)==FALSE ) {
			lpCurPlayer->AttackCritcal = -1;				//빗나감
			lpCurPlayer->SetMotionFromCode( CHRMOTION_STATE_STAND );
			return TRUE;
		}

		if ( lpSkill->Point && lpSkill->Skill_Info.FuncPointer()==TRUE ) {
			RetryPlayAttack( lpChar ); //공격 재시도
			if ( lpCurPlayer->CheckShootingTest(lpChar)==TRUE ) {

				lpCurPlayer->SetMotionFromCode( CHRMOTION_STATE_ATTACK );

				lpCurPlayer->AttackSkil = SKILL_PLAY_WIND_ARROW;
				lpCurPlayer->AttackSkil |= (lpSkill->Point<<8);				//스킬 포인트 상위 바이트에 저장
				lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil,0,lpChar,0,0,0);

				//cnt = lpCurPlayer->SendTransAttack( lpChar , smWsockServer , 0 , Wind_Arrow_Damage[lpSkill->Point-1] );
				cnt = dm_SendTransDamage( lpChar , 0,0 , 0, 0, lpCurPlayer->AttackSkil , TRUE );

				if ( !cnt )	
					lpCurPlayer->AttackCritcal = -1;				//빗나감
				else 
					DamageExp( lpChar , cnt );					//데미지에 따른 경험치 상승


				//무기속도 가속 
				/*
				cnt = lpCurPlayer->smCharInfo.Attack_Speed-6;
				if ( cnt<0 ) cnt=0;
				if ( cnt>6 ) cnt=6;
				cnt += Wind_Arrow_Speed[lpSkill->Point-1];
				lpCurPlayer->MotionLoopSpeed = (80*(fONE + 32*cnt))>>FLOATNS;
				*/
				lpCurPlayer->MotionLoopSpeed = GetAttackSpeedFrame( lpCurPlayer->smCharInfo.Attack_Speed , Wind_Arrow_Speed[lpSkill->Point-1] );

				lpCurPlayer->MotionLoop = 1;
			}
			return TRUE;
		}
		//좌측공격
		if ( lpSkill==sinSkill.pLeftSkill && RetryPlayAttack( lpChar ) ) break;
		return TRUE;

	case SKILL_PERFECT_AIM:
/*
int Perfect_Aim_Attack_Rate[10] = {26,27,29,32,36,41,47,54,62,71};
int Perfect_Aim_Damage[10] = {1,2,3,4,5,6,7,8,9,10};
*/

		//퍼펙트 에임
		if ( lpCurPlayer->CheckShootingTest(lpChar)==FALSE ) {
			lpCurPlayer->AttackCritcal = -1;				//빗나감
			lpCurPlayer->SetMotionFromCode( CHRMOTION_STATE_STAND );
			return TRUE;
		}

		if ( lpSkill->Point && lpSkill->Skill_Info.FuncPointer()==TRUE ) {
			RetryPlayAttack( lpChar ); //공격 재시도

				lpCurPlayer->SetMotionFromCode( CHRMOTION_STATE_ATTACK );

				lpCurPlayer->AttackSkil = SKILL_PLAY_PERFECT_AIM;
				lpCurPlayer->AttackSkil |= (lpSkill->Point<<8);				//스킬 포인트 상위 바이트에 저장
				lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil,0,lpChar,0,0,0);

				temp = lpCurPlayer->smCharInfo.Attack_Rating;
				//temp2 = lpCurPlayer->smCharInfo.Attack_Damage[0];
				//temp3 = lpCurPlayer->smCharInfo.Attack_Damage[1];

				//명중력 / 공격력 증가
				lpCurPlayer->smCharInfo.Attack_Rating += 
					(lpCurPlayer->smCharInfo.Attack_Rating * Perfect_Aim_Attack_Rate[lpSkill->Point-1])/100;
/*
				cnt = Perfect_Aim_Damage[lpSkill->Point-1];
				lpCurPlayer->smCharInfo.Attack_Damage[0] += cnt;
				lpCurPlayer->smCharInfo.Attack_Damage[1] += cnt;
				cnt = lpCurPlayer->smCharInfo.Level/Perfect_Aim_Damage_LV[lpSkill->Point-1];
				lpCurPlayer->smCharInfo.Attack_Damage[0] += cnt;
				lpCurPlayer->smCharInfo.Attack_Damage[1] += cnt;
*/
				//cnt = lpCurPlayer->SendTransAttack( lpChar , smWsockServer , 0 );
				cnt = dm_SendTransDamage( lpChar , 0,0 , 0, 0, lpCurPlayer->AttackSkil , TRUE );

				if ( !cnt )	
					lpCurPlayer->AttackCritcal = -1;				//빗나감
				else
					DamageExp( lpChar , cnt );					//데미지에 따른 경험치 상승

				//명중,공격력 수치 복구
				lpCurPlayer->smCharInfo.Attack_Rating = temp;
				//lpCurPlayer->smCharInfo.Attack_Damage[0] = temp2;
				//lpCurPlayer->smCharInfo.Attack_Damage[1] = temp3;


			return TRUE;
		}
		//좌측공격
		if ( lpSkill==sinSkill.pLeftSkill && RetryPlayAttack( lpChar ) ) break;
		return TRUE;

		/////////////////////////////// 2차 스킬 /////////////////////////////////////

	case SKILL_GREAT_SMASH:
		//그레이트스매쉬 
		if ( lpSkill->Skill_Info.FuncPointer()==TRUE ) {

			RetryPlayAttack( lpChar ); //공격 재시도
			lpCurPlayer->AttackSkil = SKILL_PLAY_GREAT_SMASH;
			lpCurPlayer->AttackSkil |= (lpSkill->Point<<8);				//스킬 포인트 상위 바이트에 저장
			lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil,0,lpChar,0,0,0);

			lpCurPlayer->SetMotionFromCode( CHRMOTION_STATE_SKILL );
			lpCurPlayer->MotionLoopSpeed = 110;
			lpCurPlayer->MotionLoop = 1;

			//공격 연사와 속도 설정
			if ( lpSkill->Point ) {
				temp = lpCurPlayer->smCharInfo.Attack_Rating;
				//temp2 = lpCurPlayer->smCharInfo.Attack_Damage[1];
				//temp3 = lpCurPlayer->smCharInfo.Attack_Damage[0];

				//명중력 보강
				lpCurPlayer->smCharInfo.Attack_Rating += G_Smash_Attack_Rate[lpSkill->Point-1];
/*
				cnt = lpCurPlayer->smCharInfo.Level/G_Smash_Attack_Rage[lpSkill->Point-1];
				lpCurPlayer->smCharInfo.Attack_Damage[0] = (lpCurPlayer->smCharInfo.Attack_Damage[0]+cnt)/2;
				lpCurPlayer->smCharInfo.Attack_Damage[1] = (lpCurPlayer->smCharInfo.Attack_Damage[1]+cnt)/2;

				cnt = lpCurPlayer->SendTransAttack( lpChar , smWsockServer , 0  );
				Skil_RangeAttack( lpChar->pX, lpChar->pY, lpChar->pZ, 70*fONE , 
					GetRandomPos( lpCurPlayer->smCharInfo.Attack_Damage[0], lpCurPlayer->smCharInfo.Attack_Damage[1] ) , 0 );

				lpCurPlayer->smCharInfo.Attack_Damage[1] = temp2;
				lpCurPlayer->smCharInfo.Attack_Damage[0] = temp3;

				if ( !cnt )
					lpCurPlayer->AttackCritcal = -1;				//빗나감
				else 
					DamageExp( lpChar , cnt );					//데미지에 따른 경험치 상승
*/

				//범위형 공격 범위 선정 (원)
				dm_SelectRange( lpChar->pX, lpChar->pY, lpChar->pZ, 70 , TRUE );
				//범위공격 설정후 전송
				dm_SendRangeDamage( lpChar->pX, lpChar->pY, lpChar->pZ, lpChar ,0, 0, 0, 0, lpCurPlayer->AttackSkil );

				lpCurPlayer->smCharInfo.Attack_Rating = temp;
			}
			return TRUE;
		}
		//좌측공격
		if ( lpSkill==sinSkill.pLeftSkill && RetryPlayAttack( lpChar ) ) break;
		return TRUE;

	case SKILL_MAXIMIZE:
	case SKILL_AUTOMATION:
		return TRUE;

	case SKILL_SPARK:
		//스파크

		if ( lpSkill->Skill_Info.FuncPointer()==TRUE ) {
			lpCurPlayer->AttackSkil = SKILL_PLAY_SPARK;
			lpCurPlayer->AttackSkil |= (lpSkill->Point<<8);				//스킬 포인트 상위 바이트에 저장
			lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil,0,lpChar,0,0,0);
			lpCurPlayer->SetMotionFromCode( CHRMOTION_STATE_SKILL );
			lpCurPlayer->HideWeapon = TRUE;
		}
		return TRUE;


	case SKILL_GRAND_SMASH:
		//그렌드스매쉬 
		if ( lpSkill->Skill_Info.FuncPointer()==TRUE ) {

			RetryPlayAttack( lpChar ); //공격 재시도
			lpCurPlayer->AttackSkil = SKILL_PLAY_GRAND_SMASH;
			lpCurPlayer->AttackSkil |= (lpSkill->Point<<8);				//스킬 포인트 상위 바이트에 저장
			lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil,0,lpChar,0,0,0);

			lpCurPlayer->SetMotionFromCode( CHRMOTION_STATE_SKILL );
			lpCurPlayer->MotionLoopSpeed = GetAttackSpeedFrame( lpCurPlayer->smCharInfo.Attack_Speed , 4 );
			lpCurPlayer->MotionLoop = 1;

			//공격 연사와 속도 설정
			if ( lpSkill->Point ) {
				temp = lpCurPlayer->smCharInfo.Attack_Rating;

				//명중력 보강
				lpCurPlayer->smCharInfo.Attack_Rating += Grand_Smash_AttackRate[lpSkill->Point-1];

				cnt = dm_SendTransDamage( lpChar , 0,0 , 0, 0, lpCurPlayer->AttackSkil , TRUE );
				if ( !cnt ) { 
					lpCurPlayer->AttackCritcal = -1;				//빗나감
				}

				lpCurPlayer->smCharInfo.Attack_Rating = temp;
			}

			return TRUE;
		}
		//좌측공격
		if ( lpSkill==sinSkill.pLeftSkill && RetryPlayAttack( lpChar ) ) break;
		return TRUE;



	case SKILL_GROUND_PIKE:
		return TRUE;

	case SKILL_TORNADO:
		//토네이도

		if ( lpSkill->Skill_Info.FuncPointer()==TRUE ) {
			lpCurPlayer->AttackSkil = SKILL_PLAY_TORNADO;
			lpCurPlayer->AttackSkil |= (lpSkill->Point<<8);				//스킬 포인트 상위 바이트에 저장
			lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil,0,lpChar,0,0,0);
			lpCurPlayer->SetMotionFromCode( CHRMOTION_STATE_SKILL );

			cnt = GetRandomPos( lpCurPlayer->smCharInfo.Attack_Damage[0],lpCurPlayer->smCharInfo.Attack_Damage[1] );
			cnt += (cnt*Tornado_Damage[lpSkill->Point-1])/100;

			//Skil_RangeAttack( lpCurPlayer->pX, lpCurPlayer->pY, lpCurPlayer->pZ, (Tornado_Range[lpSkill->Point-1]+8)*fONE , cnt , 0 );

			x = (lpCurPlayer->pX - lpChar->pX)>>FLOATNS;
			z = (lpCurPlayer->pZ - lpChar->pZ)>>FLOATNS;

			y = (int)sqrt( x*x+z*z );
			//if ( y>Tornado_Range[lpSkill->Point-1] ) y=Tornado_Range[lpSkill->Point-1];
			y+=8;
			//y*=fONE;

			//Skil_RangeAttack( lpCurPlayer->pX, lpCurPlayer->pY, lpCurPlayer->pZ, y , cnt , 0 );

			//범위형 공격 범위 선정 (원)
			dm_SelectRange( lpCurPlayer->pX, lpCurPlayer->pY, lpCurPlayer->pZ, y , FALSE );
			//범위공격 설정후 전송
			dm_SendRangeDamage( lpCurPlayer->pX, lpCurPlayer->pY, lpCurPlayer->pZ, 0 ,0, 0, 0, 0, lpCurPlayer->AttackSkil );


		}
		return TRUE;

	case SKILL_CHAIN_LANCE:
		//체인랜스
		if ( lpSkill->Skill_Info.FuncPointer()==TRUE ) {

			RetryPlayAttack( lpChar ); //공격 재시도
			lpCurPlayer->AttackSkil = SKILL_PLAY_CHAIN_LANCE;
			lpCurPlayer->AttackSkil |= (lpSkill->Point<<8);				//스킬 포인트 상위 바이트에 저장
			lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil,0,lpChar,0,0,0);

			lpCurPlayer->SetMotionFromCode( CHRMOTION_STATE_SKILL );
			lpCurPlayer->MotionLoopSpeed = GetAttackSpeedFrame( lpCurPlayer->smCharInfo.Attack_Speed );
			lpCurPlayer->MotionLoop = 1;

			//공격 연사와 속도 설정
			if ( lpSkill->Point ) {
				//temp = lpCurPlayer->smCharInfo.Attack_Rating;
				//명중력 보강
				//lpCurPlayer->smCharInfo.Attack_Rating += Chain_Lance_AttackRate[lpSkill->Point-1];

				cnt = dm_SendTransDamage( lpChar , 0,0 , 0, 0, lpCurPlayer->AttackSkil , TRUE );
				if ( !cnt ) { 
					lpCurPlayer->AttackCritcal = -1;				//빗나감
				}

				//lpCurPlayer->smCharInfo.Attack_Rating = temp;
			}
			return TRUE;
		}
		//좌측공격
		if ( lpSkill==sinSkill.pLeftSkill && RetryPlayAttack( lpChar ) ) break;
		return TRUE;


	case SKILL_ARROW_OF_RAGE:
		//엘로우오프레이지
		if ( lpSkill->Skill_Info.FuncPointer()==TRUE ) {
			//RetryPlayAttack( lpChar ); //공격 재시도
			lpCurPlayer->AttackSkil = SKILL_PLAY_ARROWOF_RAGE;
			lpCurPlayer->AttackSkil |= (lpSkill->Point<<8);				//스킬 포인트 상위 바이트에 저장
			lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil,0,lpChar,0,0,0);
			lpCurPlayer->SetMotionFromCode( CHRMOTION_STATE_SKILL );

			lpCurPlayer->MotionLoopSpeed = 120;
			lpCurPlayer->MotionLoop = 1;
		}
		return TRUE;

	case SKILL_FALCON:
		//팔콘
		return TRUE;

	case SKILL_AVALANCHE:
		//아발란체
		if ( lpSkill->Skill_Info.FuncPointer()==TRUE ) {
			RetryPlayAttack( lpChar ); //공격 재시도
			lpCurPlayer->SetMotionFromCode( CHRMOTION_STATE_ATTACK );

			lpCurPlayer->AttackSkil = SKILL_PLAY_AVALANCHE;
			lpCurPlayer->AttackSkil |= (lpSkill->Point<<8);				//스킬 포인트 상위 바이트에 저장
			lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil,0,lpChar,0,0,0);
			lpCurPlayer->AttackCriticalCount = 0;

			//cnt = lpCurPlayer->SendTransAttack( lpChar , smWsockServer , 0 , Avalanche_Damage[lpSkill->Point-1] );
			cnt = dm_SendTransDamage( lpChar , 0,0 , 0, 0, lpCurPlayer->AttackSkil , TRUE );

			if ( !cnt ) { 
				lpCurPlayer->AttackCritcal = -1;				//빗나감
			}
			if ( lpCurPlayer->AttackCritcal>0 ) lpCurPlayer->AttackCriticalCount++;
/*
int Avalanche_Speed[10]		= {1,1,1,2,2,2,3,3,3,4};
int Avalanche_Damage[10]	= {8,17,25,31,36,40,43,45,46,47};
int Avalanche_ArrowNum[10]	= {3,3,3,4,4,4,5,5,5,6};
*/
			//공격속도
			/*
			cnt = lpCurPlayer->smCharInfo.Attack_Speed-6;
			if ( cnt<0 ) cnt=0;
			if ( cnt>6 ) cnt=6;
			cnt+=Avalanche_Speed[lpSkill->Point-1];					//속도 +2
			lpCurPlayer->MotionLoopSpeed = (80*(fONE + 32*cnt))>>FLOATNS;
			*/
			lpCurPlayer->MotionLoopSpeed = GetAttackSpeedFrame( lpCurPlayer->smCharInfo.Attack_Speed , Avalanche_Speed[lpSkill->Point-1] );
			lpCurPlayer->MotionLoop = Avalanche_ArrowNum[lpSkill->Point-1];

			return TRUE;
		}
		if ( lpSkill==sinSkill.pLeftSkill && RetryPlayAttack( lpChar ) ) break;
		return TRUE;


	case SKILL_ELEMENTAL_SHOT:
		//엘리멘탈 샷
		if ( lpSkill->Skill_Info.FuncPointer()==TRUE ) {
			RetryPlayAttack( lpChar ); //공격 재시도
			lpCurPlayer->SetMotionFromCode( CHRMOTION_STATE_ATTACK );

			lpCurPlayer->AttackSkil = SKILL_PLAY_ELEMENTAL_SHOT;
			lpCurPlayer->AttackSkil |= (lpSkill->Point<<8);				//스킬 포인트 상위 바이트에 저장
			cnt = rand()%2;
			lpCurPlayer->AttackSkil |= ( cnt<<12 );

			lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil,0,lpChar,0,0,0);
			lpCurPlayer->AttackCriticalCount = 0;

			if ( cnt==0 ) {
				cnt = dm_SendTransDamage( lpChar , 0,0 , 0, 0, lpCurPlayer->AttackSkil , FALSE );
				if ( !cnt ) { 
					lpCurPlayer->AttackCritcal = -1;				//빗나감
				}
			}
			else {
				//범위형 공격 범위 선정 (원)
				dm_SelectRange( lpChar->pX, lpChar->pY, lpChar->pZ, 75 , FALSE );
				//범위공격 설정후 전송
				dm_SendRangeDamage( lpChar->pX, lpChar->pY, lpChar->pZ, lpChar ,0, 0, 0, 0, lpCurPlayer->AttackSkil );
			}

			lpCurPlayer->MotionLoopSpeed = GetAttackSpeedFrame( lpCurPlayer->smCharInfo.Attack_Speed , 3 );
			lpCurPlayer->MotionLoop = 2;

			return TRUE;
		}
		if ( lpSkill==sinSkill.pLeftSkill && RetryPlayAttack( lpChar ) ) break;
		return TRUE;

	case SKILL_BOMB_SHOT:
		//범 샷
		if ( lpSkill->Skill_Info.FuncPointer()==TRUE ) {
			RetryPlayAttack( lpChar ); //공격 재시도
			lpCurPlayer->SetMotionFromCode( CHRMOTION_STATE_ATTACK );

			lpCurPlayer->AttackSkil = SKILL_PLAY_BOMB_SHOT;
			lpCurPlayer->AttackSkil |= (lpSkill->Point<<8);				//스킬 포인트 상위 바이트에 저장
			lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil,0,lpChar,0,0,0);
			lpCurPlayer->AttackCriticalCount = 0;

			//cnt = dm_SendTransDamage( lpChar , 0,0 , 0, 0, lpCurPlayer->AttackSkil , TRUE );

			//범위형 공격 범위 선정 (원)
			dm_SelectRange( lpChar->pX, lpChar->pY, lpChar->pZ, BombShot_Area[lpSkill->Point-1] , FALSE );
			//범위공격 설정후 전송
			dm_SendRangeDamage( lpChar->pX, lpChar->pY, lpChar->pZ, lpChar ,0, 0, 0, 0, lpCurPlayer->AttackSkil );

			lpCurPlayer->MotionLoopSpeed = (GetAttackSpeedFrame( lpCurPlayer->smCharInfo.Attack_Speed )*80)/100;
			lpCurPlayer->MotionLoop = 2;

			return TRUE;
		}
		if ( lpSkill==sinSkill.pLeftSkill && RetryPlayAttack( lpChar ) ) break;
		return TRUE;

	case SKILL_PERFORATION:
		//퍼포레이션
		if ( lpSkill->Skill_Info.FuncPointer()==TRUE ) {
			RetryPlayAttack( lpChar ); //공격 재시도
			lpCurPlayer->SetMotionFromCode( CHRMOTION_STATE_ATTACK );

			lpCurPlayer->AttackSkil = SKILL_PLAY_PERFORATION;
			lpCurPlayer->AttackSkil |= (lpSkill->Point<<8);				//스킬 포인트 상위 바이트에 저장
			lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil,0,lpChar,0,0,0);
			lpCurPlayer->AttackCriticalCount = 0;

			//cnt = lpCurPlayer->SendTransAttack( lpChar , smWsockServer , 0 , Avalanche_Damage[lpSkill->Point-1] );
			//cnt = dm_SendTransDamage( lpChar , 0,0 , 0, 0, lpCurPlayer->AttackSkil , TRUE );

			rect.left = -40;
			rect.right = 40;
			rect.top = 0;
			rect.bottom = lpCurPlayer->smCharInfo.Shooting_Range+Perforation_Attack_Range[lpSkill->Point-1];
			rect.bottom+=20;
			rect.bottom += 20;

			//범위형 공격 범위 선정 (박스)
			dm_SelectRangeBox( lpCurPlayer , &rect , FALSE );
			//범위공격 설정후 전송
			dm_SendRangeDamage( lpCurPlayer->pX, lpCurPlayer->pY, lpCurPlayer->pZ, 0 ,0, 0, 0, 0, lpCurPlayer->AttackSkil );

			lpCurPlayer->MotionLoopSpeed = GetAttackSpeedFrame( lpCurPlayer->smCharInfo.Attack_Speed );
			lpCurPlayer->MotionLoop = 1;

			return TRUE;
		}
		if ( lpSkill==sinSkill.pLeftSkill && RetryPlayAttack( lpChar ) ) break;
		return TRUE;


	case SKILL_TRIPLE_IMPACT:				
		//트리플 임팩트
		if ( lpSkill->Skill_Info.FuncPointer()==TRUE ) {
			RetryPlayAttack( lpChar ); //공격 재시도
			lpCurPlayer->AttackSkil = SKILL_PLAY_TRIPLE_IMPACT;
			lpCurPlayer->AttackSkil |= (lpSkill->Point<<8);				//스킬 포인트 상위 바이트에 저장
			lpCurPlayer->BeginSkill( lpCurPlayer->AttackSkil,0,lpChar,0,0,0);

			switch( T_Impact_Hit[lpSkill->Point-1] ) {
			case 1:
				//1번 치기
				cnt = lpCurPlayer->AttackSkil;
				lpCurPlayer->SetMotionFromCode( CHRMOTION_STATE_ATTACK );
				lpCurPlayer->AttackSkil = cnt;				
				break;
			case 2:
				//2번치기
				cnt = lpCurPlayer->AttackSkil;
				lpCurPlayer->AttackSkil = SKILL_PLAY_IMPACT;
				lpCurPlayer->SetMotionFromCode( CHRMOTION_STATE_SKILL );
				lpCurPlayer->AttackSkil = cnt;				
				break;
			case 3:
				//3번치기
				lpCurPlayer->SetMotionFromCode( CHRMOTION_STATE_SKILL );
				break;
			}
/*
			//공격속도
			cnt = lpCurPlayer->smCharInfo.Attack_Speed-6;
			if ( cnt<0 ) cnt=0;
			if ( cnt>6 ) cnt=6;
			cnt+=2;					//속도 +2
			lpCurPlayer->MotionLoopSpeed = (80*(fONE + 32*cnt))>>FLOATNS;
			lpCurPlayer->MotionLoop = 1;
*/

			//공격 연사와 속도 설정
			if ( lpSkill->Point ) {
				//cnt = lpCurPlayer->SendTransAttack( lpChar , smWsockServer , 0 , T_Impact_Damage[lpSkill->Point-1] );
				cnt = dm_SendTransDamage( lpChar , 0,0 , 0, 0, lpCurPlayer->AttackSkil , TRUE );

				if ( !cnt )
					lpCurPlayer->AttackCritcal = -1;				//빗나감
				else {
					//데미지에 따른 경험치 상승
					DamageExp( lpChar , cnt );
				}
			}

			return TRUE;
		}
		//좌측공격
		if ( lpSkill==sinSkill.pLeftSkill && RetryPlayAttack( lpChar ) ) break;
		return TRUE;

	case SKILL_BRUTAL_SWING:				
		//부르탈 스윙
		if ( lpSkill->Skill_Info.FuncPointer()==TRUE ) {
			RetryPlayAttack( lpChar ); //공격 재시도
			lpCurPlayer->AttackSkil = SKILL_PLAY_BRUTAL_SWING;
			lpCurPlayer->AttackSkil |= (lpSkill->Point<<8);				//스킬 포인트 상위 바이트에 저장
			lpCurPlayer->BeginSkill( lpCurPlayer->AttackSkil,0,lpChar,0,0,0);
			lpCurPlayer->SetMotionFromCode( CHRMOTION_STATE_SKILL );
/*
			//공격속도
			cnt = lpCurPlayer->smCharInfo.Attack_Speed-6;
			if ( cnt<0 ) cnt=0;
			if ( cnt>6 ) cnt=6;
			cnt+=2;					//속도 +2
			lpCurPlayer->MotionLoopSpeed = (80*(fONE + 32*cnt))>>FLOATNS;
			lpCurPlayer->MotionLoop = 1;
*/
			//공격 연사와 속도 설정
			if ( lpSkill->Point ) {
				/*
				temp = lpCurPlayer->smCharInfo.Critical_Hit;
				lpCurPlayer->smCharInfo.Critical_Hit += (lpCurPlayer->smCharInfo.Critical_Hit* B_Swing_Critical[lpSkill->Point-1])/100;
				cnt = lpCurPlayer->SendTransAttack( lpChar , smWsockServer , 0 , B_Swing_Damage[lpSkill->Point-1] );
				lpCurPlayer->smCharInfo.Critical_Hit = temp;
				*/
				cnt = dm_SendTransDamage( lpChar , 0,0 , 0, 0, lpCurPlayer->AttackSkil , TRUE );

				if ( !cnt )
					lpCurPlayer->AttackCritcal = -1;				//빗나감
				else {
					//데미지에 따른 경험치 상승
					DamageExp( lpChar , cnt );
				}
			}
			return TRUE;
		}
		//좌측공격
		if ( lpSkill==sinSkill.pLeftSkill && RetryPlayAttack( lpChar ) ) break;
		return TRUE;

	case SKILL_ROAR:
		return TRUE;

	case SKILL_RAGE_OF_ZECRAM:				
		//제크람 ( 몬스터 화 속성 % 삭감)
		if ( lpSkill->Skill_Info.FuncPointer()==TRUE ) {
			RetryPlayAttack( lpChar ); //공격 재시도
			lpCurPlayer->AttackSkil = SKILL_PLAY_RAGEOF_ZECRAM;
			lpCurPlayer->AttackSkil |= (lpSkill->Point<<8);				//스킬 포인트 상위 바이트에 저장
			lpCurPlayer->BeginSkill( lpCurPlayer->AttackSkil,0,lpChar,0,0,0);
			lpCurPlayer->SetMotionFromCode( CHRMOTION_STATE_SKILL );
/*
			//공격속도
			cnt = lpCurPlayer->smCharInfo.Attack_Speed-6;
			if ( cnt<0 ) cnt=0;
			if ( cnt>6 ) cnt=6;
			cnt+=2;					//속도 +2
			lpCurPlayer->MotionLoopSpeed = (80*(fONE + 32*cnt))>>FLOATNS;
			lpCurPlayer->MotionLoop = 1;
*/
			//공격 연사와 속도 설정
			if ( lpSkill->Point ) {
				//cnt = lpCurPlayer->SendTransAttack( lpChar , smWsockServer , 0 , R_Zecram_Damage[lpSkill->Point-1] , (sITEMINFO_FIRE+1) );	//불속성 적용
				cnt = dm_SendTransDamage( lpChar , 0,0 , 0, 0, lpCurPlayer->AttackSkil , TRUE );

				if ( !cnt )
					lpCurPlayer->AttackCritcal = -1;				//빗나감
				else {
					//데미지에 따른 경험치 상승
					DamageExp( lpChar , cnt );
				}
			}
			return TRUE;
		}
		//좌측공격
		if ( lpSkill==sinSkill.pLeftSkill && RetryPlayAttack( lpChar ) ) break;
		return TRUE;

	case SKILL_AVANGING_CRASH:				
		//어벤징크래쉬
		if ( lpSkill->Skill_Info.FuncPointer()==TRUE ) {
			RetryPlayAttack( lpChar ); //공격 재시도
			lpCurPlayer->AttackSkil = SKILL_PLAY_AVANGING_CRASH;
			lpCurPlayer->AttackSkil |= (lpSkill->Point<<8);				//스킬 포인트 상위 바이트에 저장
			lpCurPlayer->BeginSkill( lpCurPlayer->AttackSkil,0,lpChar,0,0,0);
			lpCurPlayer->SetMotionFromCode( CHRMOTION_STATE_SKILL );

			temp = lpCurPlayer->smCharInfo.Attack_Rating;

			//명중력 증가
			lpCurPlayer->smCharInfo.Attack_Rating += 
					(lpCurPlayer->smCharInfo.Attack_Rating * A_Crash_AttackRate[lpSkill->Point-1])/100;

			//공격 연사와 속도 설정
			if ( lpSkill->Point ) {
				cnt = dm_SendTransDamage( lpChar , cSkill.GetSkillPoint(SKILL_BRUTAL_SWING)  ,0 , 0, 0, lpCurPlayer->AttackSkil , TRUE );

				if ( !cnt )
					lpCurPlayer->AttackCritcal = -1;				//빗나감
				else {
					//데미지에 따른 경험치 상승
					DamageExp( lpChar , cnt );
				}
			}

			lpCurPlayer->smCharInfo.Attack_Rating = temp;

			return TRUE;
		}
		//좌측공격
		if ( lpSkill==sinSkill.pLeftSkill && RetryPlayAttack( lpChar ) ) break;
		return TRUE;

	case SKILL_BONE_CRASH:				
		//본스매쉬
		if ( lpSkill->Skill_Info.FuncPointer()==TRUE ) {
			RetryPlayAttack( lpChar ); //공격 재시도
			lpCurPlayer->AttackSkil = SKILL_PLAY_BONE_SMASH;
			lpCurPlayer->AttackSkil |= (lpSkill->Point<<8);				//스킬 포인트 상위 바이트에 저장
			lpCurPlayer->BeginSkill( lpCurPlayer->AttackSkil,0,lpChar,0,0,0);
			lpCurPlayer->SetMotionFromCode( CHRMOTION_STATE_SKILL );

			//공격 연사와 속도 설정
			if ( lpSkill->Point ) {
				cnt = dm_SendTransDamage( lpChar , 0,0 , 0, 0, lpCurPlayer->AttackSkil , TRUE );

				if ( !cnt )
					lpCurPlayer->AttackCritcal = -1;				//빗나감
				else {
					//데미지에 따른 경험치 상승
					DamageExp( lpChar , cnt );
				}
			}

			return TRUE;
		}
		//좌측공격
		if ( lpSkill==sinSkill.pLeftSkill && RetryPlayAttack( lpChar ) ) break;
		return TRUE;


	case SKILL_EXPANSION:
		//익스팬션
		if ( lpSkill->Point && lpSkill->Skill_Info.FuncPointer()==TRUE ) {
			RetryPlayAttack( lpChar ); //공격 재시도

			lpCurPlayer->SetMotionFromCode( CHRMOTION_STATE_ATTACK );

			lpCurPlayer->AttackSkil = SKILL_PLAY_EXPANSION;
			lpCurPlayer->AttackSkil |= (lpSkill->Point<<8);				//스킬 포인트 상위 바이트에 저장
			lpCurPlayer->BeginSkill( lpCurPlayer->AttackSkil,0,lpChar,0,0,0);


			rect.left = -30;
			rect.right = 30;
			rect.top = 0;
			rect.bottom = LastAttackRange>>FLOATNS;
			rect.bottom += 20;

			//범위형 공격 범위 선정 (박스)
			dm_SelectRangeBox( lpCurPlayer , &rect , FALSE );
			//범위공격 설정후 전송
			dm_SendRangeDamage( lpCurPlayer->pX, lpCurPlayer->pY, lpCurPlayer->pZ, lpChar ,0, 0, 0, 0, lpCurPlayer->AttackSkil );

/*
			temp = (LastAttackRange*40)/100;			//40%위치
			lpCurPlayer->AttackCritcal = 2;
			lpCurPlayer->chrAttackTarget = lpChar;

			GetMoveLocation( 0 , 0, temp , 0, lpCurPlayer->Angle.y , 0 );
			x = lpCurPlayer->pX+GeoResult_X;
			z = lpCurPlayer->pZ+GeoResult_Z;

			cnt = GetRandomPos( lpCurPlayer->smCharInfo.Attack_Damage[0] , lpCurPlayer->smCharInfo.Attack_Damage[1] );
			cnt += (cnt*Expansion_Damage[lpSkill->Point-1])/100;
			Skil_RangeAttack( x, lpCurPlayer->pY, z, temp , cnt , 0 );

			for( cnt=0;cnt<Back_TransSkilAttackData.TargetCount;cnt++) {
				if ( Back_TransSkilAttackData.dwTarObjectSerial[cnt]==lpChar->dwObjectSerial ) break;
			}

			if ( cnt==Back_TransSkilAttackData.TargetCount ) {
				QuickSendTransAttack( lpChar , Back_TransSkilAttackData.Power );
			}
			//처음 공격하는 몬스터 에너지바 출력
			if ( lpChar->smCharInfo.State==smCHAR_STATE_ENEMY && lpChar->smCharInfo.Life[1] && lpChar->smCharInfo.Life[0]==lpChar->smCharInfo.Life[1] ) {
				lpChar->EnableStateBar = TRUE;
			}
*/
/*
			//범위형 공격 범위 선정 (원)
			dm_SelectRange( x, lpCurPlayer->pY, z, temp>>FLOATNS , FALSE );
			//범위공격 설정후 전송
			dm_SendRangeDamage( x, lpCurPlayer->pY, z, lpChar ,0, 0, 0, 0, lpCurPlayer->AttackSkil );
*/

			return TRUE;
		}
		//좌측공격
		if ( lpSkill==sinSkill.pLeftSkill ) {
			if ( RetryPlayAttack( lpChar ) ) break;
			AttCancel = TRUE;					//공격 취소명령 ( 사정거리가 짧은 경우 )
		}
		return TRUE;


		///////////////////////////////////// 나이트 ///////////////////////////

	case SKILL_SWORD_BLAST:				//소드 블래스트
		if ( lpSkill->Skill_Info.FuncPointer()==TRUE ) {
			RetryPlayAttack( lpChar ); //공격 재시도
			lpCurPlayer->AttackSkil = SKILL_PLAY_SWORD_BLAST;
			lpCurPlayer->AttackSkil |= (lpSkill->Point<<8);				//스킬 포인트 상위 바이트에 저장
			lpCurPlayer->BeginSkill( lpCurPlayer->AttackSkil,0,lpChar,0,0,0);
			lpCurPlayer->SetMotionFromCode( CHRMOTION_STATE_SKILL );
/*
			//공격속도
			cnt = lpCurPlayer->smCharInfo.Attack_Speed-6;
			if ( cnt<0 ) cnt=0;
			if ( cnt>6 ) cnt=6;
			cnt+=2;					//속도 +2
			lpCurPlayer->MotionLoopSpeed = (80*(fONE + 32*cnt))>>FLOATNS;
			lpCurPlayer->MotionLoop = 1;
*/
			//cnt = GetRandomPos( Sword_Blast_Damage[ lpSkill->Point-1 ][0] , Sword_Blast_Damage[ lpSkill->Point-1 ][1]+(lpCurPlayer->smCharInfo.Level/2) );
			rect.left = -40;
			rect.right = 40;
			rect.top = 0;
			rect.bottom = Sword_Blast_ShootingRange[lpSkill->Point-1]+50;

			//범위형 공격
			//Skil_RangeBoxAttack( lpCurPlayer , &rect , cnt , 0 , 0 );

			//범위형 공격 범위 선정 (박스)
			dm_SelectRangeBox( lpCurPlayer , &rect , FALSE );
			//범위공격 설정후 전송
			dm_SendRangeDamage( lpCurPlayer->pX, lpCurPlayer->pY, lpCurPlayer->pZ, lpChar ,0, 0, 0, 0, lpCurPlayer->AttackSkil );

			return TRUE;
		}
		//좌측공격
		if ( lpSkill==sinSkill.pLeftSkill && RetryPlayAttack( lpChar ) ) break;
		return TRUE;

	case SKILL_DOUBLE_CRASH:				//더블 크래시
		if ( lpSkill->Skill_Info.FuncPointer()==TRUE ) {
			RetryPlayAttack( lpChar ); //공격 재시도
			lpCurPlayer->AttackSkil = SKILL_PLAY_DOUBLE_CRASH;
			lpCurPlayer->AttackSkil |= (lpSkill->Point<<8);				//스킬 포인트 상위 바이트에 저장
			lpCurPlayer->BeginSkill( lpCurPlayer->AttackSkil,0,lpChar,0,0,0);
			lpCurPlayer->SetMotionFromCode( CHRMOTION_STATE_SKILL );
/*
			//공격속도
			cnt = lpCurPlayer->smCharInfo.Attack_Speed-6;
			if ( cnt<0 ) cnt=0;
			if ( cnt>6 ) cnt=6;
			//cnt+=2;					//속도 +2
			lpCurPlayer->MotionLoopSpeed = (80*(fONE + 32*cnt))>>FLOATNS;
			lpCurPlayer->MotionLoop = 1;
*/
			//공격 연사와 속도 설정
			if ( lpSkill->Point ) {
/*
				if ( (rand()%100)<(sinGetCritical( lpChar->smCharInfo.Level )+D_Crash_Critical[lpSkill->Point-1] ))
					lpCurPlayer->AttackCritcal = TRUE;			//크리티컬 공격 성공
				else
					lpCurPlayer->AttackCritcal = FALSE;

				cnt = lpCurPlayer->SendTransAttack( lpChar , smWsockServer , 0 , D_Crash_Damage[lpSkill->Point-1] );
*/
				cnt = dm_SendTransDamage( lpChar , 0,0 , 0, 0, lpCurPlayer->AttackSkil , TRUE );

				if ( !cnt )
					lpCurPlayer->AttackCritcal = -1;				//빗나감
				else {
					//데미지에 따른 경험치 상승
					DamageExp( lpChar , cnt );
				}
			}

/*
			temp = (LastAttackRange*40)/100;			//40%위치
			lpCurPlayer->AttackCritcal = 2;
			lpCurPlayer->chrAttackTarget = lpChar;

			GetMoveLocation( 0 , 0, temp , 0, lpCurPlayer->Angle.y , 0 );
			x = lpCurPlayer->pX+GeoResult_X;
			z = lpCurPlayer->pZ+GeoResult_Z;
	
			cnt = GetRandomPos( lpCurPlayer->smCharInfo.Attack_Damage[0] , lpCurPlayer->smCharInfo.Attack_Damage[1] );
			Skil_RangeAttack( x, lpCurPlayer->pY, z, temp , cnt , 0 );
*/
			return TRUE;
		}
		//좌측공격
		if ( lpSkill==sinSkill.pLeftSkill && RetryPlayAttack( lpChar ) ) break;
		return TRUE;

	case SKILL_BRANDISH:
		//브랜디쉬
		if ( lpSkill->Skill_Info.FuncPointer()==TRUE ) {

			RetryPlayAttack( lpChar ); //공격 재시도
			lpCurPlayer->AttackSkil = SKILL_PLAY_BRANDISH;
			lpCurPlayer->AttackSkil |= (lpSkill->Point<<8);				//스킬 포인트 상위 바이트에 저장
			lpCurPlayer->BeginSkill( lpCurPlayer->AttackSkil , 0 , lpChar ,0,0,0 );
			lpCurPlayer->SetMotionFromCode( CHRMOTION_STATE_SKILL );

/*
			cnt = GetRandomPos( lpCurPlayer->smCharInfo.Attack_Damage[0] , lpCurPlayer->smCharInfo.Attack_Damage[1] );
			cnt += (cnt*Brandish_Damage[lpSkill->Point-1])/100;
			Skil_RangeAttack( lpChar->pX, lpChar->pY, lpChar->pZ, Brandish_Range[lpSkill->Point-1]*fONE , cnt , 0 );
*/
			//범위형 공격 범위 선정 (원)
			dm_SelectRange( lpChar->pX, lpChar->pY, lpChar->pZ, Brandish_Range[lpSkill->Point-1] , FALSE );
			//범위공격 설정후 전송
			dm_SendRangeDamage( lpChar->pX, lpChar->pY, lpChar->pZ,  0 ,0, 0, 0, 0, lpCurPlayer->AttackSkil );

			return TRUE;
		}
		if ( lpSkill==sinSkill.pLeftSkill && RetryPlayAttack( lpChar ) ) break;
		return TRUE;

	case SKILL_PIERCING:
		//피어싱
		if ( lpSkill->Skill_Info.FuncPointer()==TRUE ) {
			RetryPlayAttack( lpChar ); //공격 재시도
			lpCurPlayer->AttackSkil = SKILL_PLAY_PIERCING;
			lpCurPlayer->AttackSkil |= (lpSkill->Point<<8);				//스킬 포인트 상위 바이트에 저장
			lpCurPlayer->BeginSkill( lpCurPlayer->AttackSkil , 0 , 0 ,0,0,0 );
			lpCurPlayer->SetMotionFromCode( CHRMOTION_STATE_SKILL );


			//명중력 보강
			temp = lpCurPlayer->smCharInfo.Attack_Rating;
			lpCurPlayer->smCharInfo.Attack_Rating += Piercing_Attack_Rating[lpSkill->Point-1];

			//cnt = GetRandomPos( lpCurPlayer->smCharInfo.Attack_Damage[0] , lpCurPlayer->smCharInfo.Attack_Damage[1] );
			//cnt += (cnt*Piercing_Damage[lpSkill->Point-1])/100;
			rect.left = -40;
			rect.right = 40;
			rect.top = 0;
			rect.bottom = 110+30;
			//범위형 공격
			//Skil_RangeBoxAttack( lpCurPlayer , &rect , cnt , 0 , TRUE );

			//범위형 공격 범위 선정 (박스)
			dm_SelectRangeBox( lpCurPlayer , &rect , TRUE );
			//범위공격 설정후 전송
			dm_SendRangeDamage( lpCurPlayer->pX, lpCurPlayer->pY, lpCurPlayer->pZ, 0 ,0, 0, 0, 0, lpCurPlayer->AttackSkil );


			lpCurPlayer->smCharInfo.Attack_Rating = temp;
			return TRUE;
		}
		if ( lpSkill==sinSkill.pLeftSkill && RetryPlayAttack( lpChar ) ) break;
		return TRUE;
/*
	case SKILL_FLAME_BRANDISH:
		//프레임 브랜디쉬
		if ( lpSkill->Skill_Info.FuncPointer()==TRUE ) {

			RetryPlayAttack( lpChar ); //공격 재시도
			lpCurPlayer->AttackSkil = SKILL_PLAY_FLAME_BRANDISH;
			lpCurPlayer->AttackSkil |= (lpSkill->Point<<8);				//스킬 포인트 상위 바이트에 저장
			lpCurPlayer->BeginSkill( lpCurPlayer->AttackSkil , 0 , lpChar ,0,0,0 );
			lpCurPlayer->SetMotionFromCode( CHRMOTION_STATE_SKILL );

			//범위형 공격 범위 선정 (원)
			dm_SelectRange( lpChar->pX, lpChar->pY, lpChar->pZ, 80 , FALSE );
			//범위공격 설정후 전송
			dm_SendRangeDamage( lpChar->pX, lpChar->pY, lpChar->pZ,  lpChar ,cSkill.GetSkillPoint(SKILL_BRANDISH), 0, 0, 0, lpCurPlayer->AttackSkil );

			return TRUE;
		}
		if ( lpSkill==sinSkill.pLeftSkill && RetryPlayAttack( lpChar ) ) break;
		return TRUE;
*/
	case SKILL_HOLY_INCANTATION:
		//홀리인켄테이션
		if ( lpChar->smCharInfo.Brood==smCHAR_MONSTER_UNDEAD && lpSkill->Skill_Info.FuncPointer()==TRUE ) {

			RetryPlayAttack( lpChar ); //공격 재시도
			lpCurPlayer->AttackSkil = SKILL_PLAY_HOLY_INCANTATION;
			lpCurPlayer->AttackSkil |= (lpSkill->Point<<8);				//스킬 포인트 상위 바이트에 저장
			lpCurPlayer->BeginSkill( lpCurPlayer->AttackSkil , 0 , lpChar ,0,0,0 );
			lpCurPlayer->SetMotionFromCode( CHRMOTION_STATE_SKILL );

			return TRUE;
		}
		if ( lpSkill==sinSkill.pLeftSkill && RetryPlayAttack( lpChar ) ) break;
		return TRUE;

	case SKILL_GRAND_CROSS:
		//그랜드크로스
		if ( lpSkill->Skill_Info.FuncPointer()==TRUE ) {

			RetryPlayAttack( lpChar ); //공격 재시도
			lpCurPlayer->AttackSkil = SKILL_PLAY_GRAND_CROSS;
			lpCurPlayer->AttackSkil |= (lpSkill->Point<<8);				//스킬 포인트 상위 바이트에 저장
			lpCurPlayer->BeginSkill( lpCurPlayer->AttackSkil , 0 , lpChar ,0,0,0 );
			lpCurPlayer->SetMotionFromCode( CHRMOTION_STATE_SKILL );

			lpCurPlayer->MotionLoopSpeed = GetAttackSpeedFrame( lpCurPlayer->smCharInfo.Attack_Speed , 1 );
			lpCurPlayer->MotionLoop = 1;

			//명중력 
			temp = lpCurPlayer->smCharInfo.Attack_Rating;

			lpCurPlayer->smCharInfo.Attack_Rating += (lpCurPlayer->smCharInfo.Attack_Rating*Grand_Cross_AttackRate[lpSkill->Point-1])/100;

			cnt = dm_SendTransDamage( lpChar , 0,0 , 0, 0, lpCurPlayer->AttackSkil , TRUE );

			if ( !cnt )	
				lpCurPlayer->AttackCritcal = -1;				//빗나감
			else
				DamageExp( lpChar , cnt );					//데미지에 따른 경험치 상승

			//명중,공격력 수치 복구
			lpCurPlayer->smCharInfo.Attack_Rating = temp;

			return TRUE;
		}
		if ( lpSkill==sinSkill.pLeftSkill && RetryPlayAttack( lpChar ) ) break;
		return TRUE;


		////////////////////////////////// 아탈란타 ///////////////////////////

	case SKILL_SHIELD_STRIKE:	
		//실드 스트라이크
		if ( lpSkill->Skill_Info.FuncPointer()==TRUE ) {
			RetryPlayAttack( lpChar ); //공격 재시도
			lpCurPlayer->AttackSkil = SKILL_PLAY_SHIELD_STRIKE;
			lpCurPlayer->AttackSkil |= (lpSkill->Point<<8);				//스킬 포인트 상위 바이트에 저장
			lpCurPlayer->BeginSkill( lpCurPlayer->AttackSkil,0,lpChar,0,0,0);
			lpCurPlayer->SetMotionFromCode( CHRMOTION_STATE_SKILL );

			lpCurPlayer->MotionLoopSpeed = 90;
			lpCurPlayer->MotionLoop = 1;

			//cnt = GetRandomPos( S_Strike_Damage[ lpSkill->Point-1 ][0] , S_Strike_Damage[ lpSkill->Point-1 ][1] );
			rect.left = -50;
			rect.right = 50;
			rect.top = 0;
			rect.bottom = 160;

			//범위형 공격
			//Skil_RangeBoxAttack( lpCurPlayer , &rect , cnt , 5 , 0 );

			//범위형 공격 범위 선정 (박스)
			dm_SelectRangeBox( lpCurPlayer , &rect , FALSE );
			//범위공격 설정후 전송
			dm_SendRangeDamage( lpCurPlayer->pX, lpCurPlayer->pY, lpCurPlayer->pZ, 0 ,0, 0, 0, 0, lpCurPlayer->AttackSkil );


			return TRUE;
		}
		//좌측공격
		if ( lpSkill==sinSkill.pLeftSkill && RetryPlayAttack( lpChar ) ) break;
		return TRUE;

	case SKILL_FARINA:
		//파리나
		if ( lpCurPlayer->CheckShootingTest(lpChar)==FALSE ) {
			lpCurPlayer->AttackCritcal = -1;				//빗나감
			lpCurPlayer->SetMotionFromCode( CHRMOTION_STATE_STAND );
			return TRUE;
		}

		if ( lpSkill->Point && lpSkill->Skill_Info.FuncPointer()==TRUE ) {
			RetryPlayAttack( lpChar ); //공격 재시도

				lpCurPlayer->SetMotionFromCode( CHRMOTION_STATE_ATTACK );

				lpCurPlayer->AttackSkil = SKILL_PLAY_FARINA;
				lpCurPlayer->AttackSkil |= (lpSkill->Point<<8);				//스킬 포인트 상위 바이트에 저장
				lpCurPlayer->BeginSkill( lpCurPlayer->AttackSkil,0,lpChar,0,0,0);

				temp = lpCurPlayer->smCharInfo.Attack_Rating;

				//공격속도
				/*
				cnt = lpCurPlayer->smCharInfo.Attack_Speed-6;
				cnt += Farina_Speed[lpSkill->Point-1];
				if ( cnt<0 ) cnt=0;
				if ( cnt>6 ) cnt=6;
				lpCurPlayer->MotionLoopSpeed = (80*(fONE + 32*cnt))>>FLOATNS;
				*/
				lpCurPlayer->MotionLoopSpeed = GetAttackSpeedFrame( lpCurPlayer->smCharInfo.Attack_Speed , Farina_Speed[lpSkill->Point-1] );
				lpCurPlayer->MotionLoop = 1;


				//명중력 
				lpCurPlayer->smCharInfo.Attack_Rating += 
					Farina_AttackRate[lpSkill->Point-1] + lpCurPlayer->smCharInfo.Level;

				//cnt = lpCurPlayer->SendTransAttack( lpChar , smWsockServer , 0 );
				cnt = dm_SendTransDamage( lpChar , 0,0 , 0, 0, lpCurPlayer->AttackSkil , TRUE );

				if ( !cnt )	
					lpCurPlayer->AttackCritcal = -1;				//빗나감
				else
					DamageExp( lpChar , cnt );					//데미지에 따른 경험치 상승

				//명중,공격력 수치 복구
				lpCurPlayer->smCharInfo.Attack_Rating = temp;


			return TRUE;
		}
		//좌측공격
		if ( lpSkill==sinSkill.pLeftSkill && RetryPlayAttack( lpChar ) ) break;
		return TRUE;


	case SKILL_VIGOR_SPEAR:
		//비거스피어
		if ( lpCurPlayer->CheckShootingTest(lpChar)==FALSE ) {
			lpCurPlayer->AttackCritcal = -1;				//빗나감
			lpCurPlayer->SetMotionFromCode( CHRMOTION_STATE_STAND );
			return TRUE;
		}

		if ( lpSkill->Point && lpSkill->Skill_Info.FuncPointer()==TRUE ) {
			RetryPlayAttack( lpChar ); //공격 재시도

				lpCurPlayer->SetMotionFromCode( CHRMOTION_STATE_ATTACK );

				lpCurPlayer->AttackSkil = SKILL_PLAY_VIGOR_SPEAR;
				lpCurPlayer->AttackSkil |= (lpSkill->Point<<8);				//스킬 포인트 상위 바이트에 저장
				lpCurPlayer->BeginSkill( lpCurPlayer->AttackSkil,0,lpChar,0,0,0);

/*
				temp = lpCurPlayer->smCharInfo.Attack_Damage[0];
				temp2 = lpCurPlayer->smCharInfo.Attack_Damage[1];

				cnt = GetRandomPos( V_Spear_Damage[lpSkill->Point-1][0] , V_Spear_Damage[lpSkill->Point-1][1] );
				//공격력
				lpCurPlayer->smCharInfo.Attack_Damage[0] += cnt;
				lpCurPlayer->smCharInfo.Attack_Damage[1] += cnt;
*/
				//cnt = lpCurPlayer->SendTransAttack( lpChar , smWsockServer , 0 );
				cnt = dm_SendTransDamage( lpChar , wpDamage[0],0 , 0, 0, lpCurPlayer->AttackSkil , TRUE );
				if ( !cnt )	
					lpCurPlayer->AttackCritcal = -1;				//빗나감
				else
					DamageExp( lpChar , cnt );					//데미지에 따른 경험치 상승
/*
				lpCurPlayer->smCharInfo.Attack_Damage[0] = temp;
				lpCurPlayer->smCharInfo.Attack_Damage[1] = temp2;
*/

			return TRUE;
		}
		//좌측공격
		if ( lpSkill==sinSkill.pLeftSkill && RetryPlayAttack( lpChar ) ) break;
		return TRUE;


	case SKILL_TWIST_JAVELIN:
		//트위스트 재블린
		if ( lpCurPlayer->CheckShootingTest(lpChar)==FALSE ) {
			lpCurPlayer->AttackCritcal = -1;				//빗나감
			lpCurPlayer->SetMotionFromCode( CHRMOTION_STATE_STAND );
			return TRUE;
		}

		if ( lpSkill->Point && lpSkill->Skill_Info.FuncPointer()==TRUE ) {
			RetryPlayAttack( lpChar ); //공격 재시도

				lpCurPlayer->SetMotionFromCode( CHRMOTION_STATE_ATTACK );

				lpCurPlayer->AttackSkil = SKILL_PLAY_TWIST_JAVELIN;
				lpCurPlayer->AttackSkil |= (lpSkill->Point<<8);				//스킬 포인트 상위 바이트에 저장
				lpCurPlayer->BeginSkill( lpCurPlayer->AttackSkil,0,lpChar,0,0,0);

				temp = lpCurPlayer->smCharInfo.Attack_Rating;
				lpCurPlayer->smCharInfo.Attack_Rating += (lpCurPlayer->smCharInfo.Attack_Rating*Twist_Javelin_Attack_Rating[lpSkill->Point-1])/100;

				//cnt = lpCurPlayer->SendTransAttack( lpChar , smWsockServer , 0 , Twist_Javelin_Damage[lpSkill->Point-1] );
				cnt = dm_SendTransDamage( lpChar , 0,0 , 0, 0, lpCurPlayer->AttackSkil , TRUE );

				if ( !cnt )	
					lpCurPlayer->AttackCritcal = -1;				//빗나감
				else
					DamageExp( lpChar , cnt );					//데미지에 따른 경험치 상승

				lpCurPlayer->smCharInfo.Attack_Rating = temp;

			return TRUE;
		}
		//좌측공격
		if ( lpSkill==sinSkill.pLeftSkill && RetryPlayAttack( lpChar ) ) break;
		return TRUE;

	case SKILL_SOUL_SUCKER:	
		//소울 석커
		if ( lpSkill->Skill_Info.FuncPointer()==TRUE ) {
			RetryPlayAttack( lpChar ); //공격 재시도
			lpCurPlayer->AttackSkil = SKILL_PLAY_SOUL_SUCKER;
			lpCurPlayer->AttackSkil |= (lpSkill->Point<<8);				//스킬 포인트 상위 바이트에 저장
			lpCurPlayer->BeginSkill( lpCurPlayer->AttackSkil,0,lpChar,0,0,0);
			lpCurPlayer->SetMotionFromCode( CHRMOTION_STATE_SKILL );

			lpCurPlayer->MotionLoopSpeed = 90;
			lpCurPlayer->MotionLoop = 1;

			rect.left = -30;
			rect.right = 30;
			rect.top = 0;
			rect.bottom = 200;

			//범위형 공격 ( 생명력 흡수 )
			//Skil_RangeBoxAttack( lpCurPlayer , &rect , lpSkill->Point , 4 , 0 );

			//범위형 공격 범위 선정 (박스)
			dm_SelectRangeBox( lpCurPlayer , &rect , FALSE );
			//범위공격 설정후 전송
			dm_SendRangeDamage( lpCurPlayer->pX, lpCurPlayer->pY, lpCurPlayer->pZ, 0 ,0, 0, 0, 0, lpCurPlayer->AttackSkil );

			return TRUE;
		}
		//좌측공격
		if ( lpSkill==sinSkill.pLeftSkill && RetryPlayAttack( lpChar ) ) break;
		return TRUE;

	case SKILL_FIRE_JAVELIN:
		//파이어 재블린
		if ( lpCurPlayer->CheckShootingTest(lpChar)==FALSE ) {
			lpCurPlayer->AttackCritcal = -1;				//빗나감
			lpCurPlayer->SetMotionFromCode( CHRMOTION_STATE_STAND );
			return TRUE;
		}

		if ( lpSkill->Point && lpSkill->Skill_Info.FuncPointer()==TRUE ) {
			RetryPlayAttack( lpChar ); //공격 재시도

				lpCurPlayer->SetMotionFromCode( CHRMOTION_STATE_ATTACK );

				lpCurPlayer->AttackSkil = SKILL_PLAY_FIRE_JAVELIN;
				lpCurPlayer->AttackSkil |= (lpSkill->Point<<8);				//스킬 포인트 상위 바이트에 저장
				lpCurPlayer->BeginSkill( lpCurPlayer->AttackSkil,0,lpChar,0,0,0);

				//불 속성 데미지 추가
				//cnt = (GetRandomPos( Fire_Javelin_Damage[lpSkill->Point-1][0],Fire_Javelin_Damage[lpSkill->Point-1][1] ))<<4;
				//cnt |= sITEMINFO_FIRE+1;

				//cnt = lpCurPlayer->SendTransAttack( lpChar , smWsockServer , 0 , 0 ,  cnt );
				dm_SendTransDamage( lpChar , 0,0 , 0, 0, lpCurPlayer->AttackSkil , FALSE );

			return TRUE;
		}
		//좌측공격
		if ( lpSkill==sinSkill.pLeftSkill && RetryPlayAttack( lpChar ) ) break;
		return TRUE;

	case SKILL_SPLIT_JAVELIN:
		//스플릿 재블린
		if ( lpCurPlayer->CheckShootingTest(lpChar)==FALSE ) {
			lpCurPlayer->AttackCritcal = -1;				//빗나감
			lpCurPlayer->SetMotionFromCode( CHRMOTION_STATE_STAND );
			return TRUE;
		}

		if ( lpSkill->Point && lpSkill->Skill_Info.FuncPointer()==TRUE ) {
			RetryPlayAttack( lpChar ); //공격 재시도

				lpCurPlayer->AttackSkil = SKILL_PLAY_SPLIT_JAVELIN;
				lpCurPlayer->AttackSkil |= (lpSkill->Point<<8);				//스킬 포인트 상위 바이트에 저장
				/////////////////// 공격 명중 미리 판정 ////////////////////

				temp = lpCurPlayer->smCharInfo.Attack_Rating;
				lpCurPlayer->smCharInfo.Attack_Rating += Split_Javelin_AttackRate[lpSkill->Point-1];
				z = Split_Javelin_AttackNum[lpSkill->Point-1];
				x = 0;
				y = sinGetAccuracy( lpChar->smCharInfo.Level , lpChar->smCharInfo.Defence );

				for(cnt=0;cnt<z;cnt++) {
					if ( (rand()%100)<y ) {
						x++;
					}
				}

				if ( x>0 ) {
					dm_SendTransDamage( lpChar , x,0 , 0, 0, lpCurPlayer->AttackSkil , FALSE );
				}
				else {
					lpCurPlayer->AttackCritcal = -1;
				}

				lpCurPlayer->AttackSkil |= ( x<<12 );
				lpCurPlayer->smCharInfo.Attack_Rating = temp;

				lpCurPlayer->BeginSkill( lpCurPlayer->AttackSkil,0,lpChar,0,0,0);
				lpCurPlayer->SetMotionFromCode( CHRMOTION_STATE_SKILL );

				lpCurPlayer->MotionLoopSpeed = GetAttackSpeedFrame( lpCurPlayer->smCharInfo.Attack_Speed , 1 );
				lpCurPlayer->MotionLoopSpeed -= (lpCurPlayer->MotionLoopSpeed*20)/100;	//속도 20%감속
				lpCurPlayer->MotionLoop = 1;
				//lpCurPlayer->MotionLoopSpeed = 200;
				//lpCurPlayer->MotionLoop = 4;

			return TRUE;
		}
		//좌측공격
		if ( lpSkill==sinSkill.pLeftSkill && RetryPlayAttack( lpChar ) ) break;
		return TRUE;

	case SKILL_TRIUMPH_OF_VALHALLA:
		if ( lpChar->smCharInfo.State==smCHAR_STATE_USER && 
			GetAssaUseEffect(lpChar,SKILL_TRIUMPH_OF_VALHALLA)==FALSE && 
			GetAssaUseEffect(lpChar,SKILL_HALL_OF_VALHALLA)==FALSE && 
			lpSkill->Skill_Info.FuncPointer()==TRUE ) {

			RetryPlayAttack( lpChar ); //공격 재시도
			lpCurPlayer->AttackSkil = SKILL_PLAY_TRIUMPH_OF_VALHALLA;
			lpCurPlayer->AttackSkil |= (lpSkill->Point<<8);				//스킬 포인트 상위 바이트에 저장
			lpCurPlayer->BeginSkill( lpCurPlayer->AttackSkil , 0 , lpChar ,0,0,0 );
			lpCurPlayer->SetMotionFromCode( CHRMOTION_STATE_SKILL );

			return TRUE;
		}
		if ( lpSkill==sinSkill.pLeftSkill && RetryPlayAttack( lpChar ) ) break;
		return TRUE;


	case SKILL_LIGHTNING_JAVELIN:
		//라이트닝 재블린
		if ( lpCurPlayer->CheckShootingTest(lpChar)==FALSE ) {
			lpCurPlayer->AttackCritcal = -1;				//빗나감
			lpCurPlayer->SetMotionFromCode( CHRMOTION_STATE_STAND );
			return TRUE;
		}
		if ( lpSkill->Point && lpSkill->Skill_Info.FuncPointer()==TRUE ) {
			RetryPlayAttack( lpChar ); //공격 재시도

			lpCurPlayer->AttackSkil = SKILL_PLAY_LIGHTNING_JAVELIN;
			lpCurPlayer->AttackSkil |= (lpSkill->Point<<8);				//스킬 포인트 상위 바이트에 저장
			lpCurPlayer->BeginSkill( lpCurPlayer->AttackSkil,0,lpChar,0,0,0);
			lpCurPlayer->SetMotionFromCode( CHRMOTION_STATE_SKILL );

			//범위형 공격 범위 선정 (원)
			dm_SelectRange( lpChar->pX, lpChar->pY, lpChar->pZ, 100 , FALSE );
			//범위공격 설정후 전송
			dm_SendRangeDamage( lpChar->pX, lpChar->pY, lpChar->pZ,  lpChar ,0, 0, 0, 0, lpCurPlayer->AttackSkil );


			return TRUE;
		}
		//좌측공격
		if ( lpSkill==sinSkill.pLeftSkill && RetryPlayAttack( lpChar ) ) break;
		return TRUE;




	case SKILL_STORM_JAVELIN:
		//스톰 재블린
		if ( lpCurPlayer->CheckShootingTest(lpChar)==FALSE ) {
			lpCurPlayer->AttackCritcal = -1;				//빗나감
			lpCurPlayer->SetMotionFromCode( CHRMOTION_STATE_STAND );
			return TRUE;
		}

		if ( lpSkill->Point && lpSkill->Skill_Info.FuncPointer()==TRUE ) {
			RetryPlayAttack( lpChar ); //공격 재시도

				//lpCurPlayer->SetMotionFromCode( CHRMOTION_STATE_ATTACK );

				lpCurPlayer->AttackSkil = SKILL_PLAY_STORM_JAVELIN;
				lpCurPlayer->AttackSkil |= (lpSkill->Point<<8);				//스킬 포인트 상위 바이트에 저장
				lpCurPlayer->BeginSkill( lpCurPlayer->AttackSkil,0,lpChar,0,0,0);


				rect.left = -30;
				rect.right = 30;
				rect.top = 0;
				rect.bottom=300;
				rect.bottom+=20;
				rect.bottom += 20;

				//범위형 공격 범위 선정 (박스)
				dm_SelectRangeBox( lpCurPlayer , &rect , FALSE );
				//범위공격 설정후 전송
				dm_SendRangeDamage( lpCurPlayer->pX, lpCurPlayer->pY, lpCurPlayer->pZ, 0 ,0, 0, 0, 0, lpCurPlayer->AttackSkil );


				lpCurPlayer->SetMotionFromCode( CHRMOTION_STATE_SKILL );
				lpCurPlayer->MotionLoopSpeed = GetAttackSpeedFrame( lpCurPlayer->smCharInfo.Attack_Speed );
				lpCurPlayer->MotionLoop = 2;



			return TRUE;
		}
		//좌측공격
		if ( lpSkill==sinSkill.pLeftSkill && RetryPlayAttack( lpChar ) ) break;
		return TRUE;


		/////////////////////////// 프리스티스 //////////////////////////
	case SKILL_HEALING:
		//힐링 ( 상대방을 치료 )
		if ( lpSkill->Skill_Info.FuncPointer()==TRUE ) {
			lpCurPlayer->AttackSkil = SKILL_PLAY_HEALING;
			lpCurPlayer->AttackSkil |= (lpSkill->Point<<8);				//스킬 포인트 상위 바이트에 저장
			lpCurPlayer->BeginSkill( lpCurPlayer->AttackSkil,0,lpChar,0,0,0 );

			lpCurPlayer->SetMotionFromCode( CHRMOTION_STATE_SKILL );
			lpCurPlayer->MotionLoopSpeed = 70;
			lpCurPlayer->MotionLoop = 1;

			dm_SendTransDamage( lpChar , wpDamage[0],wpDamage[1] ,0,0, lpCurPlayer->AttackSkil , FALSE );	


			/*
			cnt = GetRandomPos( Healing_Heal[lpSkill->Point-1][0] , Healing_Heal[lpSkill->Point-1][1] );

			smTransCommand.size = sizeof(smTRANS_COMMAND);
			smTransCommand.code = smTRANSCODE_HEALING;
			smTransCommand.WParam =	cnt;							//생명력
			smTransCommand.LParam =	0;								//
			smTransCommand.SParam =	lpChar->dwObjectSerial;			//치료 대상
			smTransCommand.EParam = lpCurPlayer->dwObjectSerial;	//
			lpsmSock = GetAreaServerSock();		//보낼 서버구하기
			if ( lpsmSock )
				lpsmSock->Send2( (char *)&smTransCommand , smTransCommand.size , TRUE );
			*/

		}
		//좌측공격
		if ( lpSkill==sinSkill.pLeftSkill && RetryPlayAttack( lpChar ) ) break;
		return TRUE;

	case SKILL_HOLY_BOLT:
		//홀리 폴트
		if ( lpSkill->Skill_Info.FuncPointer()==TRUE ) {
			lpCurPlayer->AttackSkil = SKILL_PLAY_HOLY_BOLT;
			lpCurPlayer->AttackSkil |= (lpSkill->Point<<8);				//스킬 포인트 상위 바이트에 저장
			lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil,0,lpChar,0,0,0);
			lpCurPlayer->SetMotionFromCode( CHRMOTION_STATE_SKILL );
			//lpCurPlayer->HideWeapon = TRUE;

			lpCurPlayer->MotionLoopSpeed = GetAttackSpeedFrame( 8 );
			lpCurPlayer->MotionLoop = 1;

/*
			temp  = lpCurPlayer->smCharInfo.Attack_Damage[0];
			temp2 = lpCurPlayer->smCharInfo.Attack_Damage[1];

			if ( sInven[1].ItemIndex ) {
				cnt = GetRandomPos( HolyBolt_Damage[lpSkill->Point-1][0] +  cInvenTory.InvenItem[ sInven[0].ItemIndex-1 ].sItemInfo.Damage[0], 
					HolyBolt_Damage[lpSkill->Point-1][1]+ cInvenTory.InvenItem[ sInven[0].ItemIndex-1 ].sItemInfo.Damage[1]+(lpCurPlayer->smCharInfo.Level/3) );
			}
			else {
				cnt = GetRandomPos( HolyBolt_Damage[lpSkill->Point-1][0], 
					HolyBolt_Damage[lpSkill->Point-1][1]+(lpCurPlayer->smCharInfo.Level/3) );
			}

			//Undead 100% 가중
			if ( lpChar->smCharInfo.Brood==smCHAR_MONSTER_UNDEAD )
				cnt += cnt;


			//공격력
			lpCurPlayer->smCharInfo.Attack_Damage[0] = cnt;
			lpCurPlayer->smCharInfo.Attack_Damage[1] = cnt;

			cnt = lpCurPlayer->SendTransAttack( lpChar , smWsockServer , 0 );
*/

			cnt = dm_SendTransDamage( lpChar , wpDamage[0],wpDamage[1] , 0, 0, lpCurPlayer->AttackSkil , FALSE );

			if ( !cnt )	
				lpCurPlayer->AttackCritcal = -1;				//빗나감
			else
				DamageExp( lpChar , cnt );						//데미지에 따른 경험치 상승
/*
			lpCurPlayer->smCharInfo.Attack_Damage[0] = temp;
			lpCurPlayer->smCharInfo.Attack_Damage[1] = temp2;
*/
			return TRUE;

		}
		//좌측공격
		if ( lpSkill==sinSkill.pLeftSkill && RetryPlayAttack( lpChar ) ) break;
		return TRUE;

	case SKILL_MULTISPARK:
		//멀티 스파크
		if ( lpSkill->Skill_Info.FuncPointer()==TRUE ) {

			cnt = M_Spark_Num[lpSkill->Point-1];

			cnt = GetRandomPos( (cnt/2)+1 , cnt );

			lpCurPlayer->AttackSkil = SKILL_PLAY_MULTI_SPARK;
			lpCurPlayer->AttackSkil |= (lpSkill->Point<<8);				//스킬 포인트 상위 바이트에 저장
			lpCurPlayer->AttackSkil |= (cnt<<12);						//스킬 파라메터 상위 바이트에 저장
			lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil,0,lpChar,0,0,0);
			lpCurPlayer->SetMotionFromCode( CHRMOTION_STATE_SKILL );

			//cnt *= GetRandomPos( M_Spark_Damage[lpSkill->Point-1][0] , M_Spark_Damage[lpSkill->Point-1][1] );
			//QuickSendTransAttack( lpChar , cnt );

			lpCurPlayer->MotionLoopSpeed = GetAttackSpeedFrame( 8 );
			lpCurPlayer->MotionLoop = 1;

			cnt = dm_SendTransDamage( lpChar , wpDamage[0],wpDamage[1] , 0, 0, lpCurPlayer->AttackSkil , FALSE );
			if ( cnt )	
				DamageExp( lpChar , cnt );						//데미지에 따른 경험치 상승
		}
		//좌측공격
		if ( lpSkill==sinSkill.pLeftSkill && RetryPlayAttack( lpChar ) ) break;
		return TRUE;

	case SKILL_HOLY_MIND:
		//홀리 마인드
		if ( lpSkill->Skill_Info.FuncPointer()==TRUE ) {
			lpCurPlayer->AttackSkil = SKILL_PLAY_HOLY_MIND;
			lpCurPlayer->AttackSkil |= (lpSkill->Point<<8);				//스킬 포인트 상위 바이트에 저장
			lpCurPlayer->BeginSkill( lpCurPlayer->AttackSkil,0,lpChar,0,0,0 );

			lpCurPlayer->SetMotionFromCode( CHRMOTION_STATE_SKILL );
			lpCurPlayer->MotionLoopSpeed = 70;
			lpCurPlayer->MotionLoop = 1;

			cnt = HolyMind_DecDamage[lpSkill->Point-1];

			smTransCommand.size = sizeof(smTRANS_COMMAND);
			smTransCommand.code = smTRANSCODE_HOLYMIND;
			smTransCommand.WParam =	HolyMind_DecDamage[lpSkill->Point-1];							//생명력
			smTransCommand.LParam =	15;		//HolyMind_Time[lpSkill->Point-1];								//
			smTransCommand.SParam =	lpChar->dwObjectSerial;			//치료 대상
			smTransCommand.EParam = lpCurPlayer->dwObjectSerial;	//

			lpsmSock = GetAreaServerSock();		//보낼 서버구하기
			if ( lpsmSock )
				lpsmSock->Send2( (char *)&smTransCommand , smTransCommand.size , TRUE );

		}
		//좌측공격
		if ( lpSkill==sinSkill.pLeftSkill && RetryPlayAttack( lpChar ) ) break;
		return TRUE;

	case SKILL_DIVINE_LIGHTNING:
	case SKILL_HOLY_REFLECTION:
	case SKILL_GRAND_HEALING:

	case SKILL_RESURRECTION:
	case SKILL_EXTINCTION:
		return TRUE;

	case SKILL_VIRTUAL_LIFE:
		//버쳘라이프

		if ( lpChar->smCharInfo.State==smCHAR_STATE_USER && GetAssaUseEffect(lpChar,SKILL_VIRTUAL_LIFE)==FALSE && lpSkill->Skill_Info.FuncPointer()==TRUE ) {

			lpCurPlayer->AttackSkil = SKILL_PLAY_VIRTUAL_LIFE;
			lpCurPlayer->AttackSkil |= (lpSkill->Point<<8);				//스킬 포인트 상위 바이트에 저장
			lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil,0,lpChar,0,0,0);
			lpCurPlayer->SetMotionFromCode( CHRMOTION_STATE_SKILL );

			SendProcessSKillToServer( SKILL_PLAY_VIRTUAL_LIFE , lpSkill->Point , lpChar->dwObjectSerial , 0 );
			return TRUE;
		}
		//좌측공격
		if ( lpSkill==sinSkill.pLeftSkill && RetryPlayAttack( lpChar ) ) break;
		return TRUE;


	case SKILL_VIGOR_BALL:
		//비거볼
		if ( lpSkill->Skill_Info.FuncPointer()==TRUE ) {
			lpCurPlayer->AttackSkil = SKILL_PLAY_VIGOR_BALL;
			lpCurPlayer->AttackSkil |= (lpSkill->Point<<8);				//스킬 포인트 상위 바이트에 저장
			lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil,0,lpChar,0,0,0);
			lpCurPlayer->SetMotionFromCode( CHRMOTION_STATE_SKILL );

			lpCurPlayer->MotionLoopSpeed = GetAttackSpeedFrame( 7 );
			lpCurPlayer->MotionLoop = 1;

			DamageExp( lpChar , 1 );						//데미지에 따른 경험치 상승

			return TRUE;

		}
		//좌측공격
		if ( lpSkill==sinSkill.pLeftSkill && RetryPlayAttack( lpChar ) ) break;
		return TRUE;

		//////////////////////////// 매지션 /////////////////////////////////////

	case SKILL_FIRE_BOLT:
		if ( lpSkill->Skill_Info.FuncPointer()==TRUE ) {
			lpCurPlayer->AttackSkil = SKILL_PLAY_FIRE_BOLT;
			lpCurPlayer->AttackSkil |= (lpSkill->Point<<8);				//스킬 포인트 상위 바이트에 저장
			lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil,0,lpChar,0,0,0);
			lpCurPlayer->SetMotionFromCode( CHRMOTION_STATE_SKILL );

			lpCurPlayer->MotionLoopSpeed = 100;
			lpCurPlayer->MotionLoop = 1;

/*
			temp  = lpCurPlayer->smCharInfo.Attack_Damage[0];
			temp2 = lpCurPlayer->smCharInfo.Attack_Damage[1];

			if ( sInven[1].ItemIndex ) {
				cnt = GetRandomPos( FireBolt_Damage[lpSkill->Point-1][0] +  cInvenTory.InvenItem[ sInven[0].ItemIndex-1 ].sItemInfo.Damage[0], 
					FireBolt_Damage[lpSkill->Point-1][1]+ cInvenTory.InvenItem[ sInven[0].ItemIndex-1 ].sItemInfo.Damage[1]+(lpCurPlayer->smCharInfo.Level/2) );
			}
			else {
				cnt = GetRandomPos( FireBolt_Damage[lpSkill->Point-1][0], 
					FireBolt_Damage[lpSkill->Point-1][1]+(lpCurPlayer->smCharInfo.Level/2) );
			}

			lpCurPlayer->smCharInfo.Attack_Damage[0] = cnt;
			lpCurPlayer->smCharInfo.Attack_Damage[1] = cnt;

			lpCurPlayer->SendTransAttack( lpChar , 0, 0 , 0, (sITEMINFO_FIRE+1) );

			lpCurPlayer->smCharInfo.Attack_Damage[0] = temp;
			lpCurPlayer->smCharInfo.Attack_Damage[1] = temp2;
*/
			cnt = dm_SendTransDamage( lpChar , wpDamage[0],wpDamage[1] ,0, 0, lpCurPlayer->AttackSkil , FALSE );
			if ( cnt )	
				DamageExp( lpChar , cnt );						//데미지에 따른 경험치 상승

		}
		//좌측공격
		if ( lpSkill==sinSkill.pLeftSkill && RetryPlayAttack( lpChar ) ) break;
		return TRUE;

	case SKILL_FIRE_BALL:
		if ( lpSkill->Skill_Info.FuncPointer()==TRUE ) {
			lpCurPlayer->AttackSkil = SKILL_PLAY_FIRE_BALL;
			lpCurPlayer->AttackSkil |= (lpSkill->Point<<8);				//스킬 포인트 상위 바이트에 저장
			lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil,0,lpChar,0,0,0);
			lpCurPlayer->SetMotionFromCode( CHRMOTION_STATE_SKILL );

			lpCurPlayer->MotionLoopSpeed = 100;
			lpCurPlayer->MotionLoop = 1;

/*
			if ( sInven[1].ItemIndex ) {
				cnt = GetRandomPos( FireBall_Damage[lpSkill->Point-1][0] +  cInvenTory.InvenItem[ sInven[0].ItemIndex-1 ].sItemInfo.Damage[0], 
					FireBall_Damage[lpSkill->Point-1][1]+ cInvenTory.InvenItem[ sInven[0].ItemIndex-1 ].sItemInfo.Damage[1]);
			}
			else {
				cnt = GetRandomPos( FireBall_Damage[lpSkill->Point-1][0], FireBall_Damage[lpSkill->Point-1][1] );
			}

			cnt>>=1;
			QuickSendTransAttack( lpChar , cnt , 0, (sITEMINFO_FIRE+1) );
			Skil_RangeAttack( lpChar->pX, lpChar->pY, lpChar->pZ, FireBall_Area[lpSkill->Point-1]*fONE , cnt , 100 );//100은 불 속성
*/
			//범위형 공격 범위 선정 (원)
			dm_SelectRange( lpChar->pX, lpChar->pY, lpChar->pZ, FireBall_Area[lpSkill->Point-1] , FALSE );
			//범위공격 설정후 전송
			dm_SendRangeDamage( lpChar->pX, lpChar->pY, lpChar->pZ,  lpChar , wpDamage[0],wpDamage[1] ,	0, 0, lpCurPlayer->AttackSkil );


		}
		//좌측공격
		if ( lpSkill==sinSkill.pLeftSkill && RetryPlayAttack( lpChar ) ) break;
		return TRUE;

	case SKILL_WATORNADO:
		if ( lpSkill->Skill_Info.FuncPointer()==TRUE ) {
			lpCurPlayer->AttackSkil = SKILL_PLAY_COLUMN_OF_WATER;
			lpCurPlayer->AttackSkil |= (lpSkill->Point<<8);				//스킬 포인트 상위 바이트에 저장
			lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil,0,lpChar,0,0,0);
			lpCurPlayer->SetMotionFromCode( CHRMOTION_STATE_SKILL );

			lpCurPlayer->MotionLoopSpeed = 90;
			lpCurPlayer->MotionLoop = 1;
/*
			cnt = GetRandomPos( Watornado_Damage[ lpSkill->Point-1 ][0] , Watornado_Damage[ lpSkill->Point-1 ][1] );
			cnt>>=1;

			QuickSendTransAttack( lpChar , cnt );
			Skil_RangeAttack( lpChar->pX, lpChar->pY, lpChar->pZ, Watornado_Area[ lpSkill->Point-1 ]*fONE , cnt , 104 );//104은 얼음 속성
*/

			//범위형 공격 범위 선정 (원)
			dm_SelectRange( lpChar->pX, lpChar->pY, lpChar->pZ, Watornado_Area[lpSkill->Point-1] , FALSE );
			//범위공격 설정후 전송
			dm_SendRangeDamage( lpChar->pX, lpChar->pY, lpChar->pZ,  lpChar , wpDamage[0],wpDamage[1] ,0,0, lpCurPlayer->AttackSkil );


		}
		//좌측공격
		if ( lpSkill==sinSkill.pLeftSkill && RetryPlayAttack( lpChar ) ) break;
		return TRUE;

	case SKILL_ENCHANT_WEAPON:
		if ( !lpChar->WeaponEffect && lpChar->PartyFlag && lpChar->smCharInfo.State==smCHAR_STATE_USER &&
			lpSkill->Skill_Info.FuncPointer()==TRUE ) {

			lpCurPlayer->AttackSkil = SKILL_PLAY_ENCHANT_WEAPON;
			lpCurPlayer->AttackSkil |= (lpSkill->Point<<8);				//스킬 포인트 상위 바이트에 저장
			cnt = rand()%3;
			lpCurPlayer->AttackSkil |= ( cnt<<12 );
			lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil,0,lpChar,0,0,0);

			lpCurPlayer->SetMotionFromCode( CHRMOTION_STATE_SKILL );

			lpCurPlayer->MotionLoopSpeed = 90;
			lpCurPlayer->MotionLoop = 1;
			return TRUE;

		}
		lpCurPlayer->chrAttackTarget = 0;
		//좌측공격
		if ( lpSkill==sinSkill.pLeftSkill && RetryPlayAttack( lpChar ) ) break;
		return TRUE;

	case SKILL_DEAD_RAY:
		if ( lpSkill->Skill_Info.FuncPointer()==TRUE ) {
			lpCurPlayer->AttackSkil = SKILL_PLAY_DEAD_RAY;
			lpCurPlayer->AttackSkil |= (lpSkill->Point<<8);				//스킬 포인트 상위 바이트에 저장
			lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil,0,lpChar,0,0,0);
			lpCurPlayer->SetMotionFromCode( CHRMOTION_STATE_SKILL );

			lpCurPlayer->MotionLoopSpeed = 100;
			lpCurPlayer->MotionLoop = 1;
/*
			temp = 0;

			//장착 무기 검색
			for(cnt=0;cnt<INVENTORY_MAXITEM;cnt++) {
				if( cInvenTory.InvenItem[cnt].Flag && cInvenTory.InvenItem[cnt].ItemPosition==1 ){
					//공격력 확인
					temp = GetRandomPos(  cInvenTory.InvenItem[cnt].sItemInfo.Damage[0] ,  cInvenTory.InvenItem[cnt].sItemInfo.Damage[1] );
					break;
				}
			}

			cnt = GetRandomPos( Dead_Ray_Damage[ lpSkill->Point-1 ][0] , Dead_Ray_Damage[ lpSkill->Point-1 ][1] );
			cnt += temp;

			QuickSendTransAttack( lpChar , cnt , 0, sITEMINFO_NONE );
*/
			cnt = dm_SendTransDamage( lpChar , wpDamage[0],wpDamage[1] , 0, 0, lpCurPlayer->AttackSkil , FALSE );

			if ( cnt )	
				DamageExp( lpChar , cnt );						//데미지에 따른 경험치 상승

		}
		//좌측공격
		if ( lpSkill==sinSkill.pLeftSkill && RetryPlayAttack( lpChar ) ) break;
		return TRUE;



		// 기타 스킬
	case SKILL_HOLY_BODY:
 	case SKILL_DRASTIC_SPIRIT:
	case SKILL_HOLY_VALOR:
	case SKILL_DIVINE_INHALATION:


	case SKILL_WINDY:

	case SKILL_AGONY:
	case SKILL_ZENITH:

	case SKILL_ENERGY_SHIELD:
	case SKILL_DIASTROPHISM:
	case SKILL_SPIRIT_ELEMENTAL:
	case SKILL_DANCING_SWORD:


		return TRUE;


	////////////////////// 4차 스킬 ////////////////////////
	///메카니션
	case SKILL_IMPULSION:
		//IMPULSION
		if ( lpSkill->Skill_Info.FuncPointer()==TRUE ) {
			lpCurPlayer->AttackSkil = SKILL_PLAY_IMPULSION;
			lpCurPlayer->AttackSkil |= (lpSkill->Point<<8);				//스킬 포인트 상위 바이트에 저장
			lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil,0,lpChar,0,0,0);
			lpCurPlayer->SetMotionFromCode( CHRMOTION_STATE_SKILL );

			//범위형 공격 범위 선정 (원)
			dm_SelectRange( lpChar->pX, lpChar->pY, lpChar->pZ, Impulsion_Range[lpSkill->Point-1] , TRUE , Impulsion_LightingNum[lpSkill->Point-1] );
			//범위공격 설정후 전송
			dm_SendRangeDamage( lpChar->pX, lpChar->pY, lpChar->pZ,  0 , 0 , 0 ,0,0, lpCurPlayer->AttackSkil );

		}
		return TRUE;

	//파이터
	case SKILL_DETORYER:
		//DESTROYER
		if ( lpSkill->Skill_Info.FuncPointer()==TRUE ) {
			lpCurPlayer->AttackSkil = SKILL_PLAY_DESTROYER;
			lpCurPlayer->AttackSkil |= (lpSkill->Point<<8);				//스킬 포인트 상위 바이트에 저장
			lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil,0,lpChar,0,0,0);
			lpCurPlayer->SetMotionFromCode( CHRMOTION_STATE_SKILL );
			//lpCurPlayer->HideWeapon = TRUE;

			cnt = dm_SendTransDamage( lpChar , 0,0 , 0, 0, lpCurPlayer->AttackSkil , TRUE );
			if ( !cnt )
				lpCurPlayer->AttackCritcal = -1;				//빗나감

			//속도 +1 가중
			lpCurPlayer->MotionLoopSpeed = GetAttackSpeedFrame( lpCurPlayer->smCharInfo.Attack_Speed , 1 );
			lpCurPlayer->MotionLoop = 1;
		}
		return TRUE;

	case SKILL_CYCLONE_STRIKE:
		//CYCLONE_STRIKE
		if ( lpSkill->Skill_Info.FuncPointer()==TRUE ) {
			lpCurPlayer->AttackSkil = SKILL_PLAY_CYCLONE_STRIKE;
			lpCurPlayer->AttackSkil |= (lpSkill->Point<<8);				//스킬 포인트 상위 바이트에 저장
			lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil,0,lpChar,0,0,0);
			lpCurPlayer->SetMotionFromCode( CHRMOTION_STATE_SKILL );

			//범위형 공격 범위 선정 (원)
			dm_SelectRange( lpCurPlayer->pX, lpCurPlayer->pY, lpCurPlayer->pZ, Cyclone_Strike_Area[lpSkill->Point-1] , TRUE , Cyclone_Strike_AttackNum[lpSkill->Point-1] );
			dm_AddRangeDamage( lpChar );			//선택된 범위공격 목록에 캐릭 강제 추가
			//범위공격 설정후 전송
			dm_SendRangeDamage( lpCurPlayer->pX, lpCurPlayer->pY, lpCurPlayer->pZ,  lpChar , 0 , 0 ,0,0, lpCurPlayer->AttackSkil );
		}
		return TRUE;

	//Fighter tier 5
	case SKILL_DOWN_HIT:
		//lifts the target and strikes 3 times: now, then on the clip's first two events (like Destroyer)
		if ( lpSkill->Skill_Info.FuncPointer()==TRUE ) {
			lpCurPlayer->AttackSkil = SKILL_PLAY_DOWN_HIT;
			lpCurPlayer->AttackSkil |= (lpSkill->Point<<8);
			lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil,0,lpChar,0,0,0);
			lpCurPlayer->SetMotionFromCode( CHRMOTION_STATE_SKILL );

			cnt = dm_SendTransDamage( lpChar , 0,0 , 0, 0, lpCurPlayer->AttackSkil , TRUE );
			if ( !cnt )
				lpCurPlayer->AttackCritcal = -1;

			lpCurPlayer->MotionLoopSpeed = GetAttackSpeedFrame( lpCurPlayer->smCharInfo.Attack_Speed , 1 );
			lpCurPlayer->MotionLoop = 1;
		}
		return TRUE;

	case SKILL_POWER_DASH:
		//one strike that stuns (the server stuns: monsters only, not bosses)
		if ( lpSkill->Skill_Info.FuncPointer()==TRUE ) {
			lpCurPlayer->AttackSkil = SKILL_PLAY_POWER_DASH;
			lpCurPlayer->AttackSkil |= (lpSkill->Point<<8);
			lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil,0,lpChar,0,0,0);
			lpCurPlayer->SetMotionFromCode( CHRMOTION_STATE_SKILL );

			cnt = dm_SendTransDamage( lpChar , 0,0 , 0, 0, lpCurPlayer->AttackSkil , TRUE );
			if ( !cnt )
				lpCurPlayer->AttackCritcal = -1;

			lpCurPlayer->MotionLoopSpeed = 70;
			lpCurPlayer->MotionLoop = 1;
		}
		return TRUE;

	case SKILL_MORTAL_BLOW:
		//the target, plus the enemies around the caster (like Cyclone Strike)
		if ( lpSkill->Skill_Info.FuncPointer()==TRUE ) {
			lpCurPlayer->AttackSkil = SKILL_PLAY_MORTAL_BLOW;
			lpCurPlayer->AttackSkil |= (lpSkill->Point<<8);
			lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil,0,lpChar,0,0,0);
			lpCurPlayer->SetMotionFromCode( CHRMOTION_STATE_SKILL );

			dm_SelectRange( lpCurPlayer->pX, lpCurPlayer->pY, lpCurPlayer->pZ, Mortal_Blow_Area[lpSkill->Point-1] , TRUE , Mortal_Blow_AttackNum[lpSkill->Point-1] );
			dm_AddRangeDamage( lpChar );
			dm_SendRangeDamage( lpCurPlayer->pX, lpCurPlayer->pY, lpCurPlayer->pZ,  lpChar , 0 , 0 ,0,0, lpCurPlayer->AttackSkil );

			lpCurPlayer->MotionLoopSpeed = 70;
			lpCurPlayer->MotionLoop = 1;
		}
		return TRUE;

	//Mechanician tier 5
	case SKILL_LAND_MINING:
		//mines burst around the target: the target, plus the enemies around it
		if ( lpSkill->Skill_Info.FuncPointer()==TRUE ) {
			lpCurPlayer->AttackSkil = SKILL_PLAY_LAND_MINING;
			lpCurPlayer->AttackSkil |= (lpSkill->Point<<8);
			lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil,0,lpChar,0,0,0);
			lpCurPlayer->SetMotionFromCode( CHRMOTION_STATE_SKILL );

			dm_SelectRange( lpChar->pX, lpChar->pY, lpChar->pZ, Land_Mining_Area[lpSkill->Point-1] , TRUE , Land_Mining_AttackNum[lpSkill->Point-1] );
			dm_AddRangeDamage( lpChar );
			dm_SendRangeDamage( lpChar->pX, lpChar->pY, lpChar->pZ,  lpChar , 0 , 0 ,0,0, lpCurPlayer->AttackSkil );

			lpCurPlayer->MotionLoopSpeed = 70;
			lpCurPlayer->MotionLoop = 1;
		}
		return TRUE;

	case SKILL_ROLLING_SMASH:
		//a spinning combo: the cast's strike, then one on each of the clip's first three events
		if ( lpSkill->Skill_Info.FuncPointer()==TRUE ) {
			RetryPlayAttack( lpChar );
			lpCurPlayer->AttackSkil = SKILL_PLAY_ROLLING_SMASH;
			lpCurPlayer->AttackSkil |= (lpSkill->Point<<8);
			lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil,0,lpChar,0,0,0);
			lpCurPlayer->SetMotionFromCode( CHRMOTION_STATE_SKILL );

			temp = lpCurPlayer->smCharInfo.Attack_Rating;
			lpCurPlayer->smCharInfo.Attack_Rating += Rolling_Smash_AddHit[lpSkill->Point-1];
			cnt = dm_SendTransDamage( lpChar , 0,0 , 0, 0, lpCurPlayer->AttackSkil , TRUE );
			lpCurPlayer->smCharInfo.Attack_Rating = temp;
			if ( !cnt )
				lpCurPlayer->AttackCritcal = -1;

			lpCurPlayer->MotionLoopSpeed = GetAttackSpeedFrame( lpCurPlayer->smCharInfo.Attack_Speed , 1 );
			lpCurPlayer->MotionLoop = 1;
		}
		return TRUE;

	//Pikeman tier 5
	case SKILL_DANCING_REAPER:
	case SKILL_SIDE_SPIN_ATTACK:
		//one heavy strike (Side Spin Attack also drains life: the share of an average hit)
		if ( lpSkill->Skill_Info.FuncPointer()==TRUE ) {
			lpCurPlayer->AttackSkil = (lpSkill->CODE==SKILL_DANCING_REAPER) ? SKILL_PLAY_DANCING_REAPER : SKILL_PLAY_SIDE_SPIN_ATTACK;
			lpCurPlayer->AttackSkil |= (lpSkill->Point<<8);
			lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil,0,lpChar,0,0,0);
			lpCurPlayer->SetMotionFromCode( CHRMOTION_STATE_SKILL );

			cnt = dm_SendTransDamage( lpChar , 0,0 , 0, 0, lpCurPlayer->AttackSkil , TRUE );
			if ( !cnt )
				lpCurPlayer->AttackCritcal = -1;
			else if ( lpSkill->CODE==SKILL_SIDE_SPIN_ATTACK && lpSkill->Point ) {
				int pt = lpSkill->Point-1;
				int hit = (lpCurPlayer->smCharInfo.Attack_Damage[0]+lpCurPlayer->smCharInfo.Attack_Damage[1])/2;
				hit += (hit*Side_Spin_DamagePercent[pt])/100;
				int life = sinGetLife() + (hit*Side_Spin_LifeAbsorb[pt])/100;
				if ( life>lpCurPlayer->smCharInfo.Life[1] ) life = lpCurPlayer->smCharInfo.Life[1];
				if ( sinGetLife()>0 ) sinSetLife( life );
			}

			lpCurPlayer->MotionLoopSpeed = 70;
			lpCurPlayer->MotionLoop = 1;
		}
		return TRUE;

	//Knight tier 5: single strikes (Soul Breaker with more accuracy, Crescent Moon from afar,
	//Saint Blade with more against the undead)
	case SKILL_SOUL_BREAKER:
	case SKILL_CRESCENT_MOON:
	case SKILL_SAINT_BLADE:
		if ( lpSkill->Point && lpSkill->Skill_Info.FuncPointer()==TRUE ) {
			switch( lpSkill->CODE ) {
			case SKILL_SOUL_BREAKER:	lpCurPlayer->AttackSkil = SKILL_PLAY_SOUL_BREAKER;	break;
			case SKILL_CRESCENT_MOON:	lpCurPlayer->AttackSkil = SKILL_PLAY_CRESCENT_MOON;	break;
			default:					lpCurPlayer->AttackSkil = SKILL_PLAY_SAINT_BLADE;	break;
			}
			lpCurPlayer->AttackSkil |= (lpSkill->Point<<8);
			lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil,0,lpChar,0,0,0);
			lpCurPlayer->SetMotionFromCode( CHRMOTION_STATE_SKILL );

			temp = lpCurPlayer->smCharInfo.Attack_Rating;
			if ( lpSkill->CODE==SKILL_SOUL_BREAKER )
				lpCurPlayer->smCharInfo.Attack_Rating += (temp*Soul_Breaker_AddHit[lpSkill->Point-1])/100;
			cnt = dm_SendTransDamage( lpChar , 0,0 , 0, 0, lpCurPlayer->AttackSkil , TRUE );
			lpCurPlayer->smCharInfo.Attack_Rating = temp;
			if ( !cnt )
				lpCurPlayer->AttackCritcal = -1;

			lpCurPlayer->MotionLoopSpeed = 70;
			lpCurPlayer->MotionLoop = 1;
		}
		return TRUE;

	//Priestess and Magician tier 5: single-target magic (the server works out the damage)
	case SKILL_SPIRIT_IMPACT:
	case SKILL_PIERCING_ICE:
	case SKILL_SILRAPHIM:
	case SKILL_VIS_TENUS:			//less life now, less damage for a time
	case SKILL_PRIMA_IGNIS:
		if ( lpSkill->Point && lpSkill->Skill_Info.FuncPointer()==TRUE ) {
			switch( lpSkill->CODE ) {
			case SKILL_SPIRIT_IMPACT:	lpCurPlayer->AttackSkil = SKILL_PLAY_SPIRIT_IMPACT;	break;
			case SKILL_PIERCING_ICE:	lpCurPlayer->AttackSkil = SKILL_PLAY_PIERCING_ICE;	break;
			case SKILL_SILRAPHIM:		lpCurPlayer->AttackSkil = SKILL_PLAY_SILRAPHIM;		break;
			case SKILL_VIS_TENUS:		lpCurPlayer->AttackSkil = SKILL_PLAY_VIS_TENUS;		break;
			default:					lpCurPlayer->AttackSkil = SKILL_PLAY_PRIMA_IGNIS;	break;
			}
			lpCurPlayer->AttackSkil |= (lpSkill->Point<<8);
			lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil,0,lpChar,0,0,0);
			lpCurPlayer->SetMotionFromCode( CHRMOTION_STATE_SKILL );

			lpCurPlayer->MotionLoopSpeed = GetAttackSpeedFrame( 8 );
			lpCurPlayer->MotionLoop = 1;

			cnt = dm_SendTransDamage( lpChar , wpDamage[0],wpDamage[1] , 0, 0, lpCurPlayer->AttackSkil , FALSE );
			if ( !cnt )
				lpCurPlayer->AttackCritcal = -1;
			else
				DamageExp( lpChar , cnt );
		}
		return TRUE;

	//Atalanta tier 5
	case SKILL_GALAXY_COUP:			//a javelin on the target and the enemies around it, more accurate
		if ( lpSkill->Point && lpSkill->Skill_Info.FuncPointer()==TRUE ) {
			lpCurPlayer->AttackSkil = SKILL_PLAY_GALAXY_COUP;
			lpCurPlayer->AttackSkil |= (lpSkill->Point<<8);
			lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil,0,lpChar,0,0,0);
			lpCurPlayer->SetMotionFromCode( CHRMOTION_STATE_SKILL );

			temp = lpCurPlayer->smCharInfo.Attack_Rating;
			lpCurPlayer->smCharInfo.Attack_Rating += (temp*Galaxy_Coup_AddHit[lpSkill->Point-1])/100;
			dm_SelectRange( lpChar->pX, lpChar->pY, lpChar->pZ, Galaxy_Coup_Area[lpSkill->Point-1] , TRUE , 12 );
			dm_AddRangeDamage( lpChar );
			dm_SendRangeDamage( lpChar->pX, lpChar->pY, lpChar->pZ,  lpChar , 0 , 0 ,0,0, lpCurPlayer->AttackSkil );
			lpCurPlayer->smCharInfo.Attack_Rating = temp;

			lpCurPlayer->MotionLoopSpeed = 70;
			lpCurPlayer->MotionLoop = 1;
		}
		return TRUE;

	case SKILL_SNIPPY_FEAR:			//one critical javelin blow, from further away
		if ( lpSkill->Point && lpSkill->Skill_Info.FuncPointer()==TRUE ) {
			lpCurPlayer->AttackSkil = SKILL_PLAY_SNIPPY_FEAR;
			lpCurPlayer->AttackSkil |= (lpSkill->Point<<8);
			lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil,0,lpChar,0,0,0);
			lpCurPlayer->SetMotionFromCode( CHRMOTION_STATE_SKILL );

			cnt = dm_SendTransDamage( lpChar , 0,0 , 0, 0, lpCurPlayer->AttackSkil , TRUE );
			if ( !cnt )
				lpCurPlayer->AttackCritcal = -1;

			lpCurPlayer->MotionLoopSpeed = 70;
			lpCurPlayer->MotionLoop = 1;
		}
		return TRUE;

	//Archer tier 5
	case SKILL_EVADE_SHOT:			//one shot that stuns (the server stuns: monsters only, not bosses)
	case SKILL_THUNDER_ROPE:		//one shot, with more reach and accuracy
		if ( lpCurPlayer->CheckShootingTest(lpChar)==FALSE ) {
			lpCurPlayer->AttackCritcal = -1;
			lpCurPlayer->SetMotionFromCode( CHRMOTION_STATE_STAND );
			return TRUE;
		}
		if ( lpSkill->Point && lpSkill->Skill_Info.FuncPointer()==TRUE ) {
			lpCurPlayer->AttackSkil = (lpSkill->CODE==SKILL_EVADE_SHOT) ? SKILL_PLAY_EVADE_SHOT : SKILL_PLAY_THUNDER_ROPE;
			lpCurPlayer->AttackSkil |= (lpSkill->Point<<8);
			lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil,0,lpChar,0,0,0);
			lpCurPlayer->SetMotionFromCode( CHRMOTION_STATE_SKILL );

			temp = lpCurPlayer->smCharInfo.Attack_Rating;
			if ( lpSkill->CODE==SKILL_THUNDER_ROPE )
				lpCurPlayer->smCharInfo.Attack_Rating += (temp*Thunder_Rope_AddHit[lpSkill->Point-1])/100;
			cnt = dm_SendTransDamage( lpChar , 0,0 , 0, 0, lpCurPlayer->AttackSkil , TRUE );
			lpCurPlayer->smCharInfo.Attack_Rating = temp;
			if ( !cnt )
				lpCurPlayer->AttackCritcal = -1;
			else
				DamageExp( lpChar , cnt );

			lpCurPlayer->MotionLoopSpeed = 70;
			lpCurPlayer->MotionLoop = 1;
		}
		return TRUE;

	case SKILL_NATURE_SPLASH:		//an arrow bursting on the target and the enemies around it
	case SKILL_CIRCLE_TRAP:			//the same, and the server slows them
		if ( lpCurPlayer->CheckShootingTest(lpChar)==FALSE ) {
			lpCurPlayer->AttackCritcal = -1;
			lpCurPlayer->SetMotionFromCode( CHRMOTION_STATE_STAND );
			return TRUE;
		}
		if ( lpSkill->Point && lpSkill->Skill_Info.FuncPointer()==TRUE ) {
			int area = (lpSkill->CODE==SKILL_NATURE_SPLASH) ? Nature_Splash_Area[lpSkill->Point-1] : Circle_Trap_Area[lpSkill->Point-1];
			lpCurPlayer->AttackSkil = (lpSkill->CODE==SKILL_NATURE_SPLASH) ? SKILL_PLAY_NATURE_SPLASH : SKILL_PLAY_CIRCLE_TRAP;
			lpCurPlayer->AttackSkil |= (lpSkill->Point<<8);
			lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil,0,lpChar,0,0,0);
			lpCurPlayer->SetMotionFromCode( CHRMOTION_STATE_SKILL );

			dm_SelectRange( lpChar->pX, lpChar->pY, lpChar->pZ, area , TRUE , 12 );
			dm_AddRangeDamage( lpChar );
			dm_SendRangeDamage( lpChar->pX, lpChar->pY, lpChar->pZ,  lpChar , 0 , 0 ,0,0, lpCurPlayer->AttackSkil );

			lpCurPlayer->MotionLoopSpeed = 70;
			lpCurPlayer->MotionLoop = 1;
		}
		return TRUE;

	case SKILL_FINAL_SPEAR:
		//spears burst from the ground: the target and the enemies around the caster
		if ( lpSkill->Skill_Info.FuncPointer()==TRUE ) {
			lpCurPlayer->AttackSkil = SKILL_PLAY_FINAL_SPEAR;
			lpCurPlayer->AttackSkil |= (lpSkill->Point<<8);
			lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil,0,lpChar,0,0,0);
			lpCurPlayer->SetMotionFromCode( CHRMOTION_STATE_SKILL );

			dm_SelectRange( lpCurPlayer->pX, lpCurPlayer->pY, lpCurPlayer->pZ, Final_Spear_Area[lpSkill->Point-1] , TRUE , 12 );
			dm_AddRangeDamage( lpChar );
			dm_SendRangeDamage( lpCurPlayer->pX, lpCurPlayer->pY, lpCurPlayer->pZ,  lpChar , 0 , 0 ,0,0, lpCurPlayer->AttackSkil );

			lpCurPlayer->MotionLoopSpeed = 70;
			lpCurPlayer->MotionLoop = 1;
		}
		return TRUE;

	//파이크맨
	case SKILL_ASSASSIN_EYE:
		if ( lpSkill->Skill_Info.FuncPointer()==TRUE ) {
			lpCurPlayer->AttackSkil = SKILL_PLAY_ASSASSIN_EYE;
			lpCurPlayer->AttackSkil |= (lpSkill->Point<<8);				//스킬 포인트 상위 바이트에 저장
			lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil,0,lpChar,0,0,0);
			lpCurPlayer->SetMotionFromCode( CHRMOTION_STATE_SKILL );
			//lpCurPlayer->HideWeapon = TRUE;
			dm_SendTransDamage( lpChar , 0,0 , 0, 0, lpCurPlayer->AttackSkil , FALSE );
		}
		return TRUE;

	case SKILL_CHARGING_STRIKE:
		if ( lpSkill->Skill_Info.FuncPointer()==TRUE ) {
			lpCurPlayer->AttackSkil = SKILL_PLAY_CHARGING_STRIKE;
			lpCurPlayer->AttackSkil |= (lpSkill->Point<<8);				//스킬 포인트 상위 바이트에 저장
			lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil,0,lpChar,0,0,0);
			lpCurPlayer->SetMotionFromCode( CHRMOTION_STATE_SKILL );
			//lpCurPlayer->HideWeapon = TRUE;
			SkillChargingFlag = 1;

			lpCurPlayer->MotionLoopSpeed = 60+(Charging_Strike_Time[lpSkill->Point-1]*2);
			lpCurPlayer->MotionLoop = 1;

			SkillPlaySound( SKILL_SOUND_SKILL_CHARGING	 , lpCurPlayer->pX, lpCurPlayer->pY, lpCurPlayer->pZ );
		}
		return TRUE;
	case SKILL_SHADOW_MASTER:
		if ( lpSkill->Skill_Info.FuncPointer()==TRUE ) {
			lpCurPlayer->AttackSkil = SKILL_PLAY_SHADOW_MASTER;
			lpCurPlayer->AttackSkil |= (lpSkill->Point<<8);				//스킬 포인트 상위 바이트에 저장
			lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil,0,lpChar,0,0,0);
			lpCurPlayer->SetMotionFromCode( CHRMOTION_STATE_SKILL );

			//temp = lpCurPlayer->smCharInfo.Attack_Rating;
			//lpCurPlayer->smCharInfo.Attack_Rating += Shadow_Master_AddHit[lpSkill->Point-1];				//명중력 보강
			cnt = dm_SendTransDamage( lpChar , 0,0 , 0, 0, lpCurPlayer->AttackSkil , TRUE );
			//lpCurPlayer->smCharInfo.Attack_Rating = temp;
			if ( !cnt )	
				lpCurPlayer->AttackCritcal = -1;				//빗나감

			lpCurPlayer->MotionLoopSpeed = GetAttackSpeedFrame( 7 );
			lpCurPlayer->MotionLoop = 1;

		}
		return TRUE;
	//아처
	case SKILL_PHOENIX_SHOT:
		if ( lpSkill->Skill_Info.FuncPointer()==TRUE ) {
			lpCurPlayer->AttackSkil = SKILL_PLAY_PHOENIX_SHOT;
			lpCurPlayer->AttackSkil |= (lpSkill->Point<<8);				//스킬 포인트 상위 바이트에 저장
			lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil,0,lpChar,0,0,0);
			lpCurPlayer->SetMotionFromCode( CHRMOTION_STATE_SKILL );
			//lpCurPlayer->HideWeapon = TRUE;
			SkillChargingFlag = 1;
			lpCurPlayer->MotionLoopSpeed = 60+(Phoenix_Shot_Time[lpSkill->Point-1]*2);
			//lpCurPlayer->MotionLoopSpeed = 80+Phoenix_Shot_Time[lpSkill->Point];
			lpCurPlayer->MotionLoop = 1;
			
			SkillPlaySound( SKILL_SOUND_SKILL_CHARGING	 , lpCurPlayer->pX, lpCurPlayer->pY, lpCurPlayer->pZ );
		}
		return TRUE;
	//나이트
	case SKILL_DIVINE_PIERCING:
		if ( lpSkill->Skill_Info.FuncPointer()==TRUE ) {
			lpCurPlayer->AttackSkil = SKILL_PLAY_DIVINE_PIERCING;
			lpCurPlayer->AttackSkil |= (lpSkill->Point<<8);				//스킬 포인트 상위 바이트에 저장
			lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil,0,lpChar,0,0,0);

			if ( Divine_Piercing_AttackNum[lpSkill->Point-1]>=7 ) {
				cnt = lpCurPlayer->AttackSkil;
				lpCurPlayer->AttackSkil = SKILL_PLAY_DIVINE_PIERCING3;			//모션 임시 변경
				lpCurPlayer->SetMotionFromCode( CHRMOTION_STATE_SKILL );
				lpCurPlayer->AttackSkil = cnt;
			}
			else if ( Divine_Piercing_AttackNum[lpSkill->Point-1]>=5 ) {
				cnt = lpCurPlayer->AttackSkil;
				lpCurPlayer->AttackSkil = SKILL_PLAY_DIVINE_PIERCING2;			//모션 임시 변경
				lpCurPlayer->SetMotionFromCode( CHRMOTION_STATE_SKILL );
				lpCurPlayer->AttackSkil = cnt;
			}
			else
				lpCurPlayer->SetMotionFromCode( CHRMOTION_STATE_SKILL );


			//temp = lpCurPlayer->smCharInfo.Attack_Rating;
			//lpCurPlayer->smCharInfo.Attack_Rating += Divine_Piercing_AddHit[lpSkill->Point-1];				//명중력 보강
			cnt = dm_SendTransDamage( lpChar , 0,0 , 0, 0, lpCurPlayer->AttackSkil , TRUE );
			//lpCurPlayer->smCharInfo.Attack_Rating = temp;
			if ( !cnt )	
				lpCurPlayer->AttackCritcal = -1;				//빗나감

			lpCurPlayer->MotionLoopSpeed = GetAttackSpeedFrame( 7 );
			lpCurPlayer->MotionLoop = 1;

		}
		return TRUE;
	case SKILL_SWORD_OF_JUSTICE:
		if ( lpSkill->Skill_Info.FuncPointer()==TRUE ) {
			lpCurPlayer->AttackSkil = SKILL_PLAY_SWORD_OF_JUSTICE;
			lpCurPlayer->AttackSkil |= (lpSkill->Point<<8);				//스킬 포인트 상위 바이트에 저장
			lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil,0,lpChar,0,0,0);
			lpCurPlayer->SetMotionFromCode( CHRMOTION_STATE_SKILL );

			rect.left = -70;
			rect.right = 70;
			rect.top = 0;
			rect.bottom=Sword_Of_Justice_Area[lpSkill->Point-1];
			rect.bottom+=20;
			rect.bottom += 20;

			//범위형 공격 범위 선정 (박스)
			dm_SelectRangeBox( lpCurPlayer , &rect , TRUE );
			dm_AddRangeDamage( lpChar );			//선택된 범위공격 목록에 캐릭 강제 추가
			//범위공격 설정후 전송
			dm_SendRangeDamage( lpCurPlayer->pX, lpCurPlayer->pY, lpCurPlayer->pZ, 0 ,0, 0, 0, 0, lpCurPlayer->AttackSkil );
		}
		return TRUE;
	//아탈란타
	case SKILL_X_RAGE:
		if ( lpSkill->Skill_Info.FuncPointer()==TRUE ) {
			lpCurPlayer->AttackSkil = SKILL_PLAY_X_RAGE;
			lpCurPlayer->AttackSkil |= (lpSkill->Point<<8);				//스킬 포인트 상위 바이트에 저장
			lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil,0,lpChar,0,0,0);
			lpCurPlayer->SetMotionFromCode( CHRMOTION_STATE_SKILL );
			//lpCurPlayer->HideWeapon = TRUE;


			//범위형 공격 범위 선정 (원)
			dm_SelectRange( lpChar->pX, lpChar->pY, lpChar->pZ, X_Rage_Area[lpSkill->Point-1] , FALSE );
			//범위공격 설정후 전송
			dm_SendRangeDamage( lpChar->pX, lpChar->pY, lpChar->pZ,  0 , 0 , 0 ,0,0, lpCurPlayer->AttackSkil );

		}
		return TRUE;
	case SKILL_VENGEANCE:
		if ( lpSkill->Skill_Info.FuncPointer()==TRUE ) {
			lpCurPlayer->AttackSkil = SKILL_PLAY_VENGEANCE;
			lpCurPlayer->AttackSkil |= (lpSkill->Point<<8);				//스킬 포인트 상위 바이트에 저장
			lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil,0,lpChar,0,0,0);
			lpCurPlayer->SetMotionFromCode( CHRMOTION_STATE_SKILL );
			//lpCurPlayer->HideWeapon = TRUE;

			lpCurPlayer->MotionLoopSpeed = GetAttackSpeedFrame( 6 );
			lpCurPlayer->MotionLoop = 1;

			cnt = dm_SendTransDamage( lpChar , 0,0 , 0, 0, lpCurPlayer->AttackSkil , TRUE );
			if ( !cnt )
				lpCurPlayer->AttackCritcal = -1;				//빗나감
		}


		return TRUE;

	//프리스티스
	case SKILL_GLACIAL_SPIKE:
		if ( lpSkill->Skill_Info.FuncPointer()==TRUE ) {
			lpCurPlayer->AttackSkil = SKILL_PLAY_GLACIAL_SPIKE;
			lpCurPlayer->AttackSkil |= (lpSkill->Point<<8);				//스킬 포인트 상위 바이트에 저장
			lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil,0,lpChar,0,0,0);
			lpCurPlayer->SetMotionFromCode( CHRMOTION_STATE_SKILL );
			//lpCurPlayer->HideWeapon = TRUE;
/*
			rect.left = -30;
			rect.right = 30;
			rect.top = 0;
			rect.bottom=300;
			rect.bottom+=20;
			rect.bottom += 20;

			//범위형 공격 범위 선정 (박스)
			dm_SelectRangeBox( lpCurPlayer , &rect , FALSE );
			//범위공격 설정후 전송
			dm_SendRangeDamage( lpCurPlayer->pX, lpCurPlayer->pY, lpCurPlayer->pZ, 0 ,0, 0, 0, 0, lpCurPlayer->AttackSkil );
*/
		}
		return TRUE;

	case SKILL_CHAIN_LIGHTNING:
		if ( lpSkill->Skill_Info.FuncPointer()==TRUE ) {
			lpCurPlayer->AttackSkil = SKILL_PLAY_CHAIN_LIGHTNING;
			lpCurPlayer->AttackSkil |= (lpSkill->Point<<8);				//스킬 포인트 상위 바이트에 저장
			lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil,0,lpChar,0,0,0);
			lpCurPlayer->SetMotionFromCode( CHRMOTION_STATE_SKILL );
			//lpCurPlayer->HideWeapon = TRUE;
		}
		return TRUE;

	//메지션
	case SKILL_FLAME_WAVE:
		if ( lpSkill->Skill_Info.FuncPointer()==TRUE ) {
			lpCurPlayer->AttackSkil = SKILL_PLAY_FLAME_WAVE;
			lpCurPlayer->AttackSkil |= (lpSkill->Point<<8);				//스킬 포인트 상위 바이트에 저장
			lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil,0,lpChar,0,0,0);
			lpCurPlayer->SetMotionFromCode( CHRMOTION_STATE_SKILL );
			//lpCurPlayer->HideWeapon = TRUE;

			rect.left = -60;
			rect.right = 60;
			rect.top = 0;
			rect.bottom=300;
			rect.bottom+=20;
			rect.bottom += 20;

			//범위형 공격 범위 선정 (박스)
			dm_SelectRangeBox( lpCurPlayer , &rect , FALSE );
			//범위공격 설정후 전송
			dm_SendRangeDamage( lpCurPlayer->pX, lpCurPlayer->pY, lpCurPlayer->pZ, 0 ,wpDamage[0],wpDamage[1],  0, 0, lpCurPlayer->AttackSkil );

		}
		return TRUE;
	case SKILL_M_METEO:
		if ( lpSkill->Skill_Info.FuncPointer()==TRUE ) {
			lpCurPlayer->AttackSkil = SKILL_PLAY_METEO;
			lpCurPlayer->AttackSkil |= (lpSkill->Point<<8);				//스킬 포인트 상위 바이트에 저장
			lpCurPlayer->BeginSkill(lpCurPlayer->AttackSkil,0,lpChar,0,0,0);
			lpCurPlayer->SetMotionFromCode( CHRMOTION_STATE_SKILL );
			//lpCurPlayer->HideWeapon = TRUE;
		}
		return TRUE;

		// 기타 스킬
	case SKILL_COMPULSION:
	case SKILL_MAGNETIC_SPHERE:		//댄싱소드랑 비슷하다고 함
	case SKILL_METAL_GOLEM:
	case SKILL_BERSERKER:
	case SKILL_VAGUE:
	case SKILL_RECALL_WOLVERIN:
	case SKILL_FORCE_OF_NATURE:
	case SKILL_GODLY_SHIELD:
	case SKILL_GOD_BLESS:
	case SKILL_HALL_OF_VALHALLA:
	case SKILL_FROST_JAVELIN:
	case SKILL_REGENERATION_FIELD:
	case SKILL_SUMMON_MUSPELL:
	case SKILL_FIRE_ELEMENTAL:
	case SKILL_DISTORTION:
	//tier 5 self casts
	case SKILL_BLOODY_BERSERKER:
	case SKILL_HYPER_SONIC:
	case SKILL_POWER_ENHANCEMENT:
	case SKILL_AMPLIFIED:
	case SKILL_HOLY_BENEDIC:
	case SKILL_TALARIA:
	case SKILL_SUMMON_ARCUDA:
	case SKILL_SUMMON_RAMIEL:
	case SKILL_BLESSING_KRISHNA:
	case SKILL_THIRD_ANIMA:


		return TRUE;


	}

	if ( lpCurPlayer->MotionInfo->State!=CHRMOTION_STATE_ATTACK && lpCurPlayer->MotionInfo->State!=CHRMOTION_STATE_SKILL ) {
		RetryPlayAttack( lpChar );
	}

	return FALSE;
}

//스킬 공격 연속 시도
int PlaySkillLoop( int SkillCode , smCHAR *lpChar )
{
	int point,param;
	int	cnt;
	smCHAR	*chrAttackTarget;
	RECT	rect;

	point = (SkillCode>>8)&0xF;
	param = (SkillCode>>12)&0xF;

	switch( SkillCode&0xFF ) {

	case SKILL_PLAY_CRITICAL_HIT:
		//크리티컬 힛
		if ( lpChar->chrAttackTarget && point ) {
			if ( !lpChar->chrAttackTarget->smCharInfo.Life[0] ) {
				lpChar->MotionLoop = 0;			//연속동작 취소
				break;
			}
/*
			//크리티컬 재설정
			cnt = lpChar->smCharInfo.Critical_Hit;
			lpCurPlayer->smCharInfo.Critical_Hit += Critical_Hit_Critical[point-1];

			if ( (rand()%100)<sinGetCritical(lpChar->smCharInfo.Level)) {
				lpChar->AttackCritcal = TRUE;			//크리티컬 공격 성공
			}
			else {
				lpChar->AttackCritcal = FALSE;
			}
			lpChar->smCharInfo.Critical_Hit = cnt;

			cnt = lpChar->SendTransAttack( lpChar->chrAttackTarget , smWsockServer , 0  );
*/
			cnt = dm_SendTransDamage( lpChar->chrAttackTarget , 0,0 , 0, 0, lpCurPlayer->AttackSkil , TRUE );
			if ( !cnt ) 
				lpChar->AttackCritcal = -1;
			else  {
				lpChar->AttackCritcal = 0;
				DamageExp( lpChar->chrAttackTarget , cnt );					//데미지에 따른 경험치 상승
			}

			//lpChar->Angle.y = GetRadian2D( lpChar->pX,lpChar->pZ , lpChar->chrAttackTarget->pX, lpChar->chrAttackTarget->pZ );
			//lpChar->SetMotionFromCode(CHRMOTION_STATE_ATTACK);
		}
		break;

	case SKILL_PLAY_RAVING:				//레이빙
		if ( lpChar->chrAttackTarget && point ) {

			if ( !lpChar->chrAttackTarget->smCharInfo.Life[0] ) {
				lpChar->MotionLoop = 0;			//연속동작 취소
				break;
			}
/*
			if ( (rand()%100)<sinGetCritical( lpChar->chrAttackTarget->smCharInfo.Level ) )
				lpChar->AttackCritcal = TRUE;			//크리티컬 공격 성공
			else
				lpChar->AttackCritcal = FALSE;

			cnt = lpChar->SendTransAttack( lpChar->chrAttackTarget , smWsockServer , 0 , Raving_Damage[point-1] );
*/
			cnt = dm_SendTransDamage( lpChar->chrAttackTarget , 0,0 , 0, 0, lpCurPlayer->AttackSkil , TRUE );

			if ( !cnt ) lpChar->AttackCritcal = -1;
			else {
				//라이프 감소
				lpChar->AttackCritcal = 0;
				PlaySkillSubLife( ((int)(Raving_UseLife[point-1]*10)*lpChar->smCharInfo.Life[1])/1000 );
				DamageExp( lpChar->chrAttackTarget , cnt );					//데미지에 따른 경험치 상승
			}

			lpChar->AttackAnger = 150;
			lpChar->Angle.y = GetRadian2D( lpChar->pX,lpChar->pZ , lpChar->chrAttackTarget->pX, lpChar->chrAttackTarget->pZ );
			//lpCurPlayer->AttackCritcal = 1;
		}
		break;

	case SKILL_PLAY_AVALANCHE:
			//아발란체
		if ( lpChar->chrAttackTarget && point ) {

			if ( !lpChar->chrAttackTarget->smCharInfo.Life[0] ) {
				lpChar->MotionLoop = 0;			//연속동작 취소
				break;
			}
			/*
			if ( (rand()%100)<sinGetCritical( lpChar->chrAttackTarget->smCharInfo.Level ) && lpChar->AttackCriticalCount==0 ) {
				lpChar->AttackCritcal = TRUE;			//크리티컬 공격 성공
				lpChar->AttackCriticalCount++;
			}
			else
				lpChar->AttackCritcal = FALSE;

			cnt = lpChar->SendTransAttack( lpChar->chrAttackTarget , smWsockServer , 0 , Avalanche_Damage[point-1] );
			if ( !cnt ) lpChar->AttackCritcal = -1;
			*/

			cnt = dm_SendTransDamage( lpChar->chrAttackTarget , 0,0 , 0, 0, lpCurPlayer->AttackSkil , TRUE );
			if ( !cnt ) 
				lpChar->AttackCritcal = -1;
			else { 
				lpChar->AttackCritcal = 0;
				DamageExp( lpChar->chrAttackTarget , cnt );					//데미지에 따른 경험치 상승
			}


			lpChar->Angle.y = GetRadian2D( lpChar->pX,lpChar->pZ , lpChar->chrAttackTarget->pX, lpChar->chrAttackTarget->pZ );
			//lpCurPlayer->AttackCritcal = 1;
		}
		break;


	case SKILL_PLAY_ELEMENTAL_SHOT:
		if ( lpChar->chrAttackTarget && point ) {

			if ( !lpChar->chrAttackTarget->smCharInfo.Life[0] ) {
				lpChar->MotionLoop = 0;			//연속동작 취소
				break;
			}

			chrAttackTarget = lpChar->chrAttackTarget;

			if ( param==0 ) {	//불
				cnt = dm_SendTransDamage( lpChar->chrAttackTarget , 0,0 , 0, 0, lpChar->AttackSkil , FALSE );
				if ( !cnt ) { 
					lpChar->AttackCritcal = -1;				//빗나감
				}
				else {
					lpChar->AttackCritcal = 0;
				}
			}
			else {					//전기
				//범위형 공격 범위 선정 (원)
				dm_SelectRange( chrAttackTarget->pX, chrAttackTarget->pY, chrAttackTarget->pZ, 75 , FALSE );
				//범위공격 설정후 전송
				dm_SendRangeDamage( chrAttackTarget->pX, chrAttackTarget->pY, chrAttackTarget->pZ, chrAttackTarget ,0, 0, 0, 0, lpChar->AttackSkil );
			}
		}
		break;

	case SKILL_PLAY_STORM_JAVELIN:
		if ( lpChar->chrAttackTarget && point ) {

			chrAttackTarget = lpChar->chrAttackTarget;

			rect.left = -30;
			rect.right = 30;
			rect.top = 0;
			rect.bottom=300;
			rect.bottom+=20;
			rect.bottom += 20;

			//범위형 공격 범위 선정 (박스)
			dm_SelectRangeBox( lpChar , &rect , FALSE );
			//범위공격 설정후 전송
			dm_SendRangeDamage( lpChar->pX, lpChar->pY, lpChar->pZ, 0 ,0, 0, 0, 0, lpChar->AttackSkil );
		}
		break;

	case SKILL_PLAY_BOMB_SHOT:
		if ( lpChar->chrAttackTarget && point ) {

			chrAttackTarget = lpChar->chrAttackTarget;

			//범위형 공격 범위 선정 (원)
			dm_SelectRange( chrAttackTarget->pX, chrAttackTarget->pY, chrAttackTarget->pZ, BombShot_Area[point-1] , FALSE );
			//범위공격 설정후 전송
			dm_SendRangeDamage( chrAttackTarget->pX, chrAttackTarget->pY, chrAttackTarget->pZ, chrAttackTarget ,0, 0, 0, 0, lpChar->AttackSkil );
		}
		break;

	}

	return TRUE;
}


//잔상용 스킬색상 선택
int SetSkillMotionBlurColor( int dwSkillCode )
{
	switch( dwSkillCode&0xFF ) {
	case SKILL_PLAY_CRITICAL_HIT:
		//크리티컬 힛
		smRender.Color_R += 256;
		smRender.Color_G += -64;
		smRender.Color_B += 256;
		return TRUE;

	case SKILL_PLAY_RAVING:
		//레이빙
		smRender.Color_R += 256;
		smRender.Color_G += -64;
		smRender.Color_B += -64;
		return TRUE;

	case SKILL_PLAY_IMPACT:
		//임팩트
		smRender.Color_R += 256;
		smRender.Color_G += 256;
		smRender.Color_B += -64;
		return TRUE;

	case SKILL_PLAY_TRIPLE_IMPACT:
		//트리플 임팩트
		smRender.Color_R += 256;
		smRender.Color_G += -64;
		smRender.Color_B += 256;
		return TRUE;

	case SKILL_PLAY_BRUTAL_SWING:
		//트리플 임팩트
		smRender.Color_R += -64;
		smRender.Color_G += 256;
		smRender.Color_B += 128;
		return TRUE;

	case SKILL_PLAY_CHAIN_LANCE:
		//체인랜스
		smRender.Color_R += 256;
		smRender.Color_G -= 64;
		smRender.Color_B -= 64;
		break;

	}

	return FALSE;
}


//범위 스킬 확인
int GetSkillPower( smCHAR *lpChar , smCHAR *lpChar2 , int dx, int dy, int dz )
{
	int x,y,z,dist;

	switch(  lpChar->HoSkillCode ) {
	case SKILL_SCOUT_HAWK:
		if ( Skill_ScoutHawk.Point ) {
			x = (lpChar->HoSkillPos.x - lpChar2->pX)>>FLOATNS;
			y = (lpChar->HoSkillPos.y - lpChar2->pY)>>FLOATNS;
			z = (lpChar->HoSkillPos.z - lpChar2->pZ)>>FLOATNS;
			dist = x*x+y*y+z*z;

			if ( dist<(512*512) ) 
				return Scout_Hawk_Attack_Rate[ Skill_ScoutHawk.Point-1 ];
		}
		break;
	}

	return NULL;
}

//스킬 습득 이펙트
int	LearnSkillEffect( smCHAR *lpChar )
{
	if ( lpChar ) {
		StartSkill( lpChar->pX,lpChar->pY,lpChar->pZ, 0,0,0, SKILL_UP1 );
		SkillPlaySound( SKILL_SOUND_LEARN , lpChar->pX,lpChar->pY,lpChar->pZ );			//스킬 효과음
		//StartEffect( lpChar->pX,lpChar->pY,lpChar->pZ, EFFECT_PHYSICAL_ABSORB_DAMAGE );
	}
	else {
		StartSkill( lpCurPlayer->pX,lpCurPlayer->pY,lpCurPlayer->pZ, 0,0,0, SKILL_UP1 );
		SkillPlaySound( SKILL_SOUND_LEARN , lpCurPlayer->pX,lpCurPlayer->pY,lpCurPlayer->pZ );			//스킬 효과음
		//StartEffect( lpCurPlayer->pX,lpCurPlayer->pY,lpCurPlayer->pZ, EFFECT_PHYSICAL_ABSORB_DAMAGE );
	}

	return TRUE;
}

int LastSkillMastery = (-10000*SIN_MAX_USE_SKILL) ^ ((DWORD)LearnSkillEffect);

//스킬감시
int CheckPlaySkill()
{
	int cnt;
	int point;
	int OverPoint;
	int SkillCount = 0;
	int	LastSkillCount;
	int SkillError = 0;
	int	SubMastery;
	smTRANS_COMMAND	smTransCommand;

	point = sinSkill.SkillPoint;

	for(cnt=1;cnt<13;cnt++) {
		point += sinSkill.UseSkill[cnt].Point;
		SkillCount += sinSkill.UseSkill[cnt].UseSkillCount;
		if ( sinSkill.UseSkill[cnt].Point>10 ) point += 16384;
	}

	LastSkillCount = LastSkillMastery^((DWORD)LearnSkillEffect);
	LastSkillCount = -LastSkillCount;

	SubMastery = SkillCount-LastSkillCount;
	if ( (SubMastery)>300 ) //스킬숙련도 300 이상 급증가
	{	
		// 박재원 - 스킬 마스터 아이템을 사용할 때는 skillError가 아니므로
		if(cInvenTory.InvenItem[SelectInvenItemIndex-1].sItemInfo.CODE==(sinBI1|sin39) ||
		   cInvenTory.InvenItem[SelectInvenItemIndex-1].sItemInfo.CODE==(sinBI1|sin40) ||
		   cInvenTory.InvenItem[SelectInvenItemIndex-1].sItemInfo.CODE==(sinBI1|sin41) )
		{
			SkillError = 0;
		}
		else
			SkillError = 2;

		smTransCommand.LParam = SubMastery;
	}

	LastSkillMastery = (-SkillCount)^((DWORD)LearnSkillEffect);

	if ( lpCurPlayer->smCharInfo.Level>=10 ) {
		OverPoint = ((lpCurPlayer->smCharInfo.Level-8)/2)+1;	//-point;
		OverPoint +=  GetSkillPoint_LevelQuest( lpCurPlayer->smCharInfo.Level , sinQuest_levelLog );	//퀘스트로 획득한 스킬 포인트

		if ( OverPoint<point ) {
			//스킬 포인트 오류
			SkillError = 1;
			smTransCommand.LParam = point;
		}
	}
	else {
		if ( point>0 ) {
			SkillError = 1;
			smTransCommand.LParam = point;
		}
	}

	if ( SkillError ) {
		smTransCommand.size = sizeof(smTRANS_COMMAND);
		smTransCommand.code = smTRANSCODE_WARNING_SKILL;
		smTransCommand.WParam = SkillError;
		//smTransCommand.LParam = SubMastery;
		smTransCommand.SParam = lpCurPlayer->smCharInfo.Level;
		smTransCommand.EParam = 0;
		smTransCommand.EParam = dm_GetCommandChkSum( &smTransCommand , lpCurPlayer->dwObjectSerial );

		if ( fnEncodeDamagePacket ) {
			fnEncodeDamagePacket( &smTransCommand );			//암호화
			if ( smWsockDataServer ) {
				smWsockDataServer->Send2( (char *)&smTransCommand , smTransCommand.size , TRUE );
			}
		}
	}


	return TRUE;
}






