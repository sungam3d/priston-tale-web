#include "smlib3d\\smd3d.h"
#include "smwsock.h"

#include "character.h"
#include "srcsound\\dxwav.h"
#include "particle.h"
#include "fileread.h"
#include "playmain.h"
#include "drawsub.h"
#include "netplay.h"
#include "field.h"
#include "hobaram\\holinkheader.h"
#include "sinbaram\\sinlinkheader.h"

#include "effectsnd.h"
#include "playsub.h"
#include "skillsub.h"
#include "language.h"
#include "damage.h"
#include "areaserver.h"

/*
struct TRANS_SKIL_ATTACKDATA2 {
	int size,code;

	DWORD	dwDestObjectSerial;	//공격하는 캐릭터 고유번호

	DWORD	dwChkSum;

	int		x,y,z;			// 좌표
	int		AttackState;	// 속성
	int		AttackSize;		// 공격 범위
	short	Power[2];		// 공격 파워
	short	Power2[2];		// 공격 파워 ( 추가 공격 )
	short	Critical[2];	// 크리티컬
	int		SkillCode;		// 스킬코드
	DWORD	dwTime;			// 클라이언트 시간
	int		AttackCount;	// 공격 카운터
	int		Temp[4];		// 예비

	DWROD	dwMainTargetObject;		//주 타겟

	int	TargetCount;
	DWORD	dwTarObjectSerial[SKIL_ATTACK_CHAR_MAX];	//공격대상 캐릭터 고유번호
};

//새 공격 패킷
#define smTRANSCODE_ATTACK_DAMAGE		0x50322000
#define smTRANSCODE_RANGE_DAMAGE		0x50322010

struct TRANS_ATTACKDATA2 {
	int size,code;

	DWORD	dwChkSum;

	DWORD	dwDestObjectSerial;	//공격하는 캐릭터 고유번호
	DWORD	dwTarObjectSerial;	//공격대상 캐릭터 고유번호

	int		x,y,z;			// 좌표
	int		AttackState;	// 속성
	int		AttackSize;		// 공격 범위
	short	Power[2];		// 공격 파워
	short	Power2[2];		// 공격 파워 ( 추가 공격 )
	short	Critical[2];	// 크리티컬
	int		SkillCode;		// 스킬코드
	DWORD	dwTime;			// 클라이언트 시간
	int		AttackCount;	// 공격 카운터
	int		Temp[4];		// 예비
};

*/

//범위공격 설정후 전송
//int dm_SendRangeDamage( int x, int y, int z, smCHAR *lpMainTraget ,int PowParam1, int PowParam2, int AttackState , int Registance ,  DWORD dwSkillCode )
//int dm_QuickSendTransAttack( smCHAR *lpChar , int power , int AttackState , int Resistance )

LPFN_EncodeDamagePacket	fnEncodeDamagePacket = 0;
LPFN_DecodeDamagePacket	fnDecodeDamagePacket = 0;


TRANS_SKIL_ATTACKDATA2	*lpTransSkill_AttackData = 0;
smCHAR	*lpSelected_Char[SKIL_ATTACK_CHAR_MAX];
int	dmSelected_CharCnt;
int	dmUseAccuracy;
DWORD	dmAttackCount = 0;

//범위 공격 첵크섬
DWORD	dm_GetRangeDamgeChkSum( TRANS_SKIL_ATTACKDATA2 *lpTransSkilAttackData2 )
{
	DWORD	dwChkSum;

	dwChkSum = lpTransSkilAttackData2->code;
	dwChkSum += lpTransSkilAttackData2->x^lpTransSkilAttackData2->y^lpTransSkilAttackData2->z;
	dwChkSum += lpTransSkilAttackData2->AttackCount^lpTransSkilAttackData2->dwTime;
	dwChkSum ^= ((DWORD *)lpTransSkilAttackData2->Power)[0];
	dwChkSum ^= ((DWORD *)lpTransSkilAttackData2->Power2)[0];
	dwChkSum ^= ((DWORD *)lpTransSkilAttackData2->Critical)[0];
	dwChkSum ^= ((DWORD *)lpTransSkilAttackData2->MotionCount)[0];
	dwChkSum += lpTransSkilAttackData2->SkillCode;
	dwChkSum ^= 0x65028086;

	return dwChkSum;
}

//범위 공격 첵크섬
DWORD	dm_GetDamgeChkSum( TRANS_ATTACKDATA2 *lpTransAttackData2 )
{
	DWORD	dwChkSum;

	dwChkSum = lpTransAttackData2->code;
	dwChkSum += lpTransAttackData2->x^lpTransAttackData2->y^lpTransAttackData2->z;
	dwChkSum += lpTransAttackData2->AttackCount^lpTransAttackData2->dwTime;
	dwChkSum ^= ((DWORD *)lpTransAttackData2->Power)[0];
	dwChkSum ^= ((DWORD *)lpTransAttackData2->Power2)[0];
	dwChkSum ^= ((DWORD *)lpTransAttackData2->Critical)[0];
	dwChkSum ^= ((DWORD *)lpTransAttackData2->MotionCount)[0];
	dwChkSum += lpTransAttackData2->SkillCode;
	dwChkSum ^= 0x68098051;

	return dwChkSum;
}

//코맨드 첵크섬
DWORD	dm_GetCommandChkSum( smTRANS_COMMAND *lpTransCommand , DWORD dwObjectSerial )
{
	DWORD	dwChkSum;

	dwChkSum = lpTransCommand->code&0x037C70FF;
	dwChkSum += dwObjectSerial<<5;
	dwChkSum ^= lpTransCommand->WParam ^ lpTransCommand->SParam;
	dwChkSum ^= lpTransCommand->code<<2;
	dwChkSum ^= lpTransCommand->LParam ^ lpTransCommand->EParam;
	dwChkSum ^= 0x009821CC;

	return dwChkSum;
}

//S2C 공격 첵크섬
DWORD	dm_GetDamgeChkSum_S2V( TRANS_ATTACKDATA *lpTransAttackData )
{
	DWORD	dwChkSum;

	dwChkSum = lpTransAttackData->code;
	dwChkSum += lpTransAttackData->x^lpTransAttackData->y^lpTransAttackData->z;
	dwChkSum ^= lpTransAttackData->AttackState+lpTransAttackData->AttackSize;
	dwChkSum ^= lpTransAttackData->Power<<5;
	dwChkSum ^= lpTransAttackData->dwChkSum;
	dwChkSum ^= ((DWORD *)lpTransAttackData->sRating)[0];
	dwChkSum ^= 0x80516809;

	return dwChkSum;
}

//처음 공격하는 몬스터 에너지바 출력
int dm_OpenFireAttack( smCHAR *lpChar , DWORD SkillCode )
{

	lpChar->dwLastRecvAttackTime = dwPlayTime;		//해당 몬스터를 공격한 시간

	if ( lpChar->smCharInfo.State==smCHAR_STATE_ENEMY && lpChar->smCharInfo.Life[1] && lpChar->smCharInfo.Life[0]==lpChar->smCharInfo.Life[1] ) {
		lpChar->EnableStateBar = TRUE;
		return TRUE;
	}
	return FALSE;
}

//처음 공격하는 몬스터 에너지바 출력 ( 범위 )
int dm_OpenFireAttackRange( DWORD dwSkillCode )
{
	DWORD SkillCode;
	int cnt;
	int OpenCount=0;
	smCHAR *lpChar;



	SkillCode = dwSkillCode&0xFF;

	if ( SkillCode==SKILL_PLAY_ROAR ) return FALSE;

	for( cnt=0;cnt<dmSelected_CharCnt;cnt++ ) {
		//처음 공격하는 몬스터 에너지바 출력
		lpChar = lpSelected_Char[cnt];
		if ( lpChar ) {
			lpChar->dwLastRecvAttackTime = dwPlayTime;		//해당 몬스터를 공격한 시간
			if ( lpChar->smCharInfo.State==smCHAR_STATE_ENEMY && lpChar->smCharInfo.Life[1] && lpChar->smCharInfo.Life[0]==lpChar->smCharInfo.Life[1] ) {
				lpChar->EnableStateBar = TRUE;
				OpenCount++;

				AddAttMonsterCode( lpChar->smCharInfo.StatePoint );	//몬스터 공격목록 큐에저장
			}
		}
	}

	return OpenCount;
}


//상대 캐릭터에 공격 가함 
int dm_SendTransDamage( smCHAR *lpChar , int PowParam1, int PowParam2, int AttackState, int Resistance, DWORD dwSkillCode , int UseAccuracy ,  smWINSOCK *lpsmSock )
{

	TRANS_ATTACKDATA2	TransAttackData;
	smWINSOCK *lpsmSendSock =0;
	int	result;
	int dmg;
	int rnd;

//	if ( RestartPlayCount!=0 ) return FALSE;

	if ( (abs(dwPlayTime-dwLastMouseMoveTime)>1024*60*3 ||
		 abs(dwPlayTime-dwLastCharMoveTime)>1024*60*3 || 
		 abs(dwFuncChkTime-dwPlayTime)>14*60*1024 ) ||
		 RestartPlayCount!=0 ) {
		//2~3분 이상 마우스 움직임 없을땐 공격 무효 ( 보츠 플레이 자동 방어 )
		//2~3분 이상 캐릭터 제자리에 서있을때 공격 무효 ( 보츠 플레이 자동 방어 )
		return FALSE;
	}

	dmAttackCount++;

	ZeroMemory( &TransAttackData , sizeof(TRANS_ATTACKDATA2) );

	if ( UseAccuracy ) {
		int	temp , accuracy , pow;

		switch( lpChar->smCharInfo.State ) {
			case smCHAR_STATE_ENEMY:
				//명중률을 리턴한다 
				dwBattleTime = dwPlayTime;						//전투 시간 기록

				temp = lpCurPlayer->smCharInfo.Attack_Rating;
				//스카우트호크 ( 명중율 증가 )
				if ( lpCurPlayer->HoSkillCode ) {
					switch( lpCurPlayer->HoSkillCode ) {
						case SKILL_SCOUT_HAWK:
							pow = GetSkillPower( lpCurPlayer , lpChar , 0,0,0 );
							if ( pow ) {
								lpCurPlayer->smCharInfo.Attack_Rating += (lpCurPlayer->smCharInfo.Attack_Rating*pow)/100;
							}
							break;				
					}
				}

				accuracy = sinGetAccuracy( lpChar->smCharInfo.Level , lpChar->smCharInfo.Defence );
				lpCurPlayer->smCharInfo.Attack_Rating = temp;

				rnd = rand()%100;
#ifdef	_LANGUAGE_KOREAN
				if ( smConfig.DebugMode && VRKeyBuff[VK_SHIFT] ) {
					char szBuff[256];
					wsprintf( szBuff , "\310\256\267\374 : %d/%d" , rnd , accuracy );
					AddChatBuff( szBuff , 0 );
				}
#endif
				if ( rnd>accuracy ) {
					return FALSE;		//방어율 실패
				} 
				break;

			case smCHAR_STATE_NPC:
				//TransAttackData.AttackState = Resistance;
				if ( MouseItem.Flag ) return FALSE;				//마우스에 아이템 들고 있으면 NPC 호출 금지

				AttackState = Resistance;
				Resistance = 0;
				break;
		}
	}

	AddAttMonsterCode( lpChar->smCharInfo.StatePoint );	//몬스터 공격목록 큐에저장

	TransAttackData.code = smTRANSCODE_ATTACK_DAMAGE;
	TransAttackData.size = sizeof(TRANS_ATTACKDATA2);

	TransAttackData.x = lpChar->pX;
	TransAttackData.y = lpChar->pY;
	TransAttackData.z = lpChar->pZ;

	TransAttackData.Power[0] = lpCurPlayer->smCharInfo.Attack_Damage[0];
	TransAttackData.Power[1] = lpCurPlayer->smCharInfo.Attack_Damage[1];

	if ( PowParam1 || PowParam2 ) {
		TransAttackData.Power2[0] = PowParam1;
		TransAttackData.Power2[1] = PowParam2;
	}
	else {
		if ( !dwSkillCode ) {
			//무기 데미지만 따로 저장
			TransAttackData.Power2[0] = TransAttackData.Power[0]-cInvenTory.InvenItem[ sInven[0].ItemIndex-1 ].sItemInfo.Damage[0];
			TransAttackData.Power2[1] = TransAttackData.Power[1]-cInvenTory.InvenItem[ sInven[0].ItemIndex-1 ].sItemInfo.Damage[1];
		}
	}


	if ( ((lpCurPlayer->dwActionItemCode^sinWM1)&sinITEM_MASK2)!=0 )		//지팡이 크리티컬 없음
		TransAttackData.Critical[0] = lpCurPlayer->smCharInfo.Critical_Hit;

	switch( lpCurPlayer->smCharInfo.JOB_CODE ) {
		case JOBCODE_PRIESTESS:
		case JOBCODE_MAGICIAN:
		case JOBCODE_SHAMAN:
			TransAttackData.Critical[1] = lpCurPlayer->smCharInfo.Spirit;			//정신력 저장
			break;

		case JOBCODE_MECHANICIAN:
		case JOBCODE_FIGHTER:
		case JOBCODE_PIKEMAN:
		case JOBCODE_KNIGHT:
		case JOBCODE_ASSASSIN:
		case JOBCODE_MARTIALARTIST:
			TransAttackData.Critical[1] = lpCurPlayer->smCharInfo.Strength;			//힘 저장
			break;

		case JOBCODE_ARCHER:
		case JOBCODE_ATALANTA:
			TransAttackData.Critical[1] = lpCurPlayer->smCharInfo.Dexterity;		//민첩 저장
			break;
	}

	TransAttackData.AttackState = AttackState;
	TransAttackData.AttackSize  = 48*fONE;

	TransAttackData.MotionCount[0] = lpCurPlayer->MotionEvent;
	TransAttackData.AttackCount = dmAttackCount;
	TransAttackData.dwTime = dwPlayTime;
	TransAttackData.SkillCode = dwSkillCode;

	TransAttackData.dwWeaponCode = lpCurPlayer->dwActionItemCode;

	if ( lpCurPlayer->OnStageField>=0 && lpChar->smCharInfo.State==smCHAR_STATE_ENEMY ) 
		TransAttackData.Area[0] = StageField[ lpCurPlayer->OnStageField ]->FieldCode;
	else
		TransAttackData.Area[0] = -1;

	TransAttackData.dwDestObjectSerial = lpCurPlayer->dwObjectSerial;
	TransAttackData.dwTarObjectSerial = lpChar->dwObjectSerial;
	lpChar->TryAttackCount++;


	TransAttackData.dwChkSum = dm_GetDamgeChkSum(&TransAttackData);


	CheckMaxDamage();		//처음 한번만 적용되게 되있음

	if ( lpChar!=lpCurPlayer )
		lpsmSendSock = GetServerSock(lpChar->ServerCode);		//서버 코드 구하기
	else 
		lpsmSendSock = smWsockUserServer;

	if ( lpsmSock ) lpsmSendSock=lpsmSock;

	if ( lpsmSendSock && fnEncodeDamagePacket && fnDecodeDamagePacket ) {
		dmg = GetRandomPos( TransAttackData.Power[0] , TransAttackData.Power[1] );

		//dm_EncodePacket( &TransAttackData );				//암호화
		fnEncodeDamagePacket( &TransAttackData );			//암호화
		result = lpsmSendSock->Send2( (char *)&TransAttackData , TransAttackData.size , TRUE );
		if ( result ) {
			dm_OpenFireAttack( lpChar , dwSkillCode );		//처음 공격하는 몬스터 에너지바 출력
			return dmg;
		}
	}

	return NULL;
}


//상대 캐릭터에 공격 가함 
int dm_SendTransDamage( smCHAR *lpChar , int PowParam1, int PowParam2, int AttackState, int Resistance, DWORD dwSkillCode , int UseAccuracy )
{

	return dm_SendTransDamage( lpChar , PowParam1, PowParam2, AttackState, Resistance, dwSkillCode , UseAccuracy , 0 );
}

int dm_OpenTransRangeDamage()
{
	if ( !lpTransSkill_AttackData ) lpTransSkill_AttackData = new TRANS_SKIL_ATTACKDATA2;

	ZeroMemory( lpTransSkill_AttackData , sizeof(TRANS_SKIL_ATTACKDATA2) );
	dmSelected_CharCnt = 0;

	if ( lpTransSkill_AttackData ) return TRUE;

	return FALSE;
}

int dm_SendTransRangeDamage()
{
	smWINSOCK *lpsmSendSock;
	int result;

	if ( RestartPlayCount!=0 ) return FALSE;
	if ( !lpTransSkill_AttackData ) return FALSE;

	CheckMaxDamage();		//처음 한번만 적용되게 되있음

	lpsmSendSock = GetAreaServerSock();		//보낼 서버구하기

	if ( lpsmSendSock && lpTransSkill_AttackData->TargetCount>0 && fnEncodeDamagePacket && fnDecodeDamagePacket ) {
		//보낸 공격력 기록
		//Record_SendDamage( lpsmSendSock , lpTransSkill_AttackData->TargetCount );

		//dm_EncodePacket( lpTransSkill_AttackData );
		fnEncodeDamagePacket( lpTransSkill_AttackData );			//암호화

		result = lpsmSendSock->Send2( (char *)lpTransSkill_AttackData , lpTransSkill_AttackData->size , TRUE );

		//처음 공격하는 몬스터 에너지바 출력 ( 범위 )
		dm_OpenFireAttackRange(lpTransSkill_AttackData->SkillCode);
	}


	ZeroMemory( lpTransSkill_AttackData , sizeof(TRANS_SKIL_ATTACKDATA2) );
	delete	lpTransSkill_AttackData;
	lpTransSkill_AttackData = 0;

	return TRUE;
}



//범위형 공격 범위 선정 (원)
int dm_SelectRange( int x, int y, int z , int range , int UseAttackRating , int SelCount )
{

	int cnt;
	int	dRange;
	int dx,dy,dz,dist;
	smCHAR	*lpChar;
	int Attack;
	int accuracy;
	int ChkCount = 0;

	dm_OpenTransRangeDamage();
	dmSelected_CharCnt = 0;
	dmUseAccuracy = UseAttackRating;

	dRange = range*range;

	for( cnt=0;cnt<OTHER_PLAYER_MAX;cnt++ ) {
		if ( lpTransSkill_AttackData->TargetCount>=SKIL_ATTACK_CHAR_MAX ) break;
		if ( chrOtherPlayer[cnt].Flag && 
			chrOtherPlayer[cnt].smCharInfo.State==smCHAR_STATE_ENEMY && 
			chrOtherPlayer[cnt].smCharInfo.Brood!=smCHAR_MONSTER_USER && 
			chrOtherPlayer[cnt].smCharInfo.Life[0]>0 ) 
		{
			lpChar = &chrOtherPlayer[cnt];

			dx = (x-lpChar->pX)>>FLOATNS;
			dy = (y-lpChar->pY)>>FLOATNS;
			dz = (z-lpChar->pZ)>>FLOATNS;
			dist = dx*dx+dy*dy+dz*dz;

			if ( dist<dRange && abs(dy)<65) {
				if ( !SelCount || SelCount>ChkCount ) {

					Attack = TRUE;

					if ( UseAttackRating ) {
						accuracy = sinGetAccuracy( chrOtherPlayer[cnt].smCharInfo.Level , chrOtherPlayer[cnt].smCharInfo.Defence );
						if ( (rand()%100)>accuracy ) 
							Attack = FALSE;
					}

					if ( Attack ) {	
						lpTransSkill_AttackData->dwTarObjectSerial[lpTransSkill_AttackData->TargetCount++] = chrOtherPlayer[cnt].dwObjectSerial;
						lpSelected_Char[dmSelected_CharCnt++] = &chrOtherPlayer[cnt];
					}
				}

				ChkCount++;
			}
		}
	}

	return dmSelected_CharCnt;
}

//범위형 공격 범위 선정 (원)
int dm_SelectRange( int x, int y, int z , int range , int UseAttackRating )
{
	return dm_SelectRange( x,y,z, range, UseAttackRating , 0 );

}

//범위형 공격 범위 선정 (박스)
int dm_SelectRangeBox( smCHAR *lpCurChar , RECT *lpRect , int UseAttackRating )
{

	int cnt;
	int dx,dy,dz;
	int Attack,accuracy;
	smCHAR	*lpChar;

	dm_OpenTransRangeDamage();
	dmSelected_CharCnt = 0;
	dmUseAccuracy = UseAttackRating;

	for( cnt=0;cnt<OTHER_PLAYER_MAX;cnt++ ) {
		if ( lpTransSkill_AttackData->TargetCount>=SKIL_ATTACK_CHAR_MAX ) break;
		if ( chrOtherPlayer[cnt].Flag && chrOtherPlayer[cnt].smCharInfo.State==smCHAR_STATE_ENEMY && chrOtherPlayer[cnt].smCharInfo.Life[0]>0
			&& chrOtherPlayer[cnt].smCharInfo.Brood!=smCHAR_MONSTER_USER) { // 장별 - 범위형 공격에 크리스탈 몬스터는 맞지 않게 한다
			lpChar = &chrOtherPlayer[cnt];

			dx = (lpChar->pX-lpCurChar->pX)>>FLOATNS;
			dy = (lpChar->pY-lpCurChar->pY)>>FLOATNS;
			dz = (lpChar->pZ-lpCurChar->pZ)>>FLOATNS;

			// 이동된 지역 좌표값을 구하는 것이구나! 놀랍다.
			GetMoveLocation( dx, dy, dz, 0, (-lpCurChar->Angle.y)&ANGCLIP , 0 );

			if ( GeoResult_X>lpRect->left && GeoResult_X<lpRect->right  &&
				GeoResult_Z>lpRect->top && GeoResult_Z<lpRect->bottom && abs(dy)<256 ) {

				Attack = TRUE;

				if ( UseAttackRating ) {
					accuracy = sinGetAccuracy( chrOtherPlayer[cnt].smCharInfo.Level , chrOtherPlayer[cnt].smCharInfo.Defence );
					if ( (rand()%100)>accuracy ) 
						Attack = FALSE;
				}

				if ( Attack ) {
					lpTransSkill_AttackData->dwTarObjectSerial[lpTransSkill_AttackData->TargetCount++] = chrOtherPlayer[cnt].dwObjectSerial;
					lpSelected_Char[dmSelected_CharCnt++] = &chrOtherPlayer[cnt];
				}
			}

		}
	}

	return dmSelected_CharCnt;
}

//범위형 공격 범위 선정 (디바인 라이트닝)
int dm_SelectDamageCount( smCHAR *lpCurChar , int SelCount , int Size )
{

	int cnt;
	int dx,dy,dz,dist;
	int cCount;
	smCHAR	*lpChar;
	int dSize;

	dm_OpenTransRangeDamage();
	dmSelected_CharCnt = 0;
	dmUseAccuracy = 0;

	if ( lpCurChar!=lpCurPlayer ) return FALSE;

	dSize = Size*Size;

	for( cnt=0;cnt<OTHER_PLAYER_MAX;cnt++ ) {

		cCount = (DivineLightning_FindCount+cnt)&OTHER_PLAYER_MASK;

		if ( chrOtherPlayer[cCount].Flag && chrOtherPlayer[cCount].dwObjectSerial && 
			lpCurChar!=&chrOtherPlayer[cCount] && chrOtherPlayer[cCount].smCharInfo.State==smCHAR_STATE_ENEMY &&
			chrOtherPlayer[cCount].smCharInfo.Brood!=smCHAR_MONSTER_USER &&
			chrOtherPlayer[cCount].smCharInfo.Life[0]>0 ) {

				lpChar = &chrOtherPlayer[cCount];

				dx = (lpChar->pX-lpCurChar->pX)>>FLOATNS;
				dy = (lpChar->pY-lpCurChar->pY)>>FLOATNS;
				dz = (lpChar->pZ-lpCurChar->pZ)>>FLOATNS;

				dist = dx*dx+dy*dy+dz*dz;

				if ( dist<dSize && abs(dy)<65) {
					lpTransSkill_AttackData->dwTarObjectSerial[lpTransSkill_AttackData->TargetCount++] = lpChar->dwObjectSerial;

					lpSelected_Char[dmSelected_CharCnt] = lpChar;
					dwSkill_DivineLightning_Target[dmSelected_CharCnt] = lpChar->dwObjectSerial;
					dmSelected_CharCnt++;

					if ( dmSelected_CharCnt>=SelCount ) {
						DivineLightning_FindCount = cCount;
						break;
					}

				}
			}
	}
	DivineLightning_Target_Count = dmSelected_CharCnt;

	return dmSelected_CharCnt;
}

//범위형 공격 범위 선정 (체인라이트닝)
int dm_SelectDamageCount( smCHAR *lpCurChar , smCHAR *lpTragetChar, int SelCount , int Size )
{

	int cnt;
	int dx,dy,dz,dist;
	int cCount;
	smCHAR	*lpChar;
	int dSize;

	dm_OpenTransRangeDamage();
	dmSelected_CharCnt = 0;
	dmUseAccuracy = 0;

	if ( lpCurChar!=lpCurPlayer ) return FALSE;

	dSize = Size*Size;

	lpSelected_Char[dmSelected_CharCnt] = lpTragetChar;
	dwSkill_DivineLightning_Target[dmSelected_CharCnt] = lpTragetChar->dwObjectSerial;
	lpTransSkill_AttackData->dwTarObjectSerial[lpTransSkill_AttackData->TargetCount++] = lpTragetChar->dwObjectSerial;
	dmSelected_CharCnt++;

	for( cnt=0;cnt<OTHER_PLAYER_MAX;cnt++ ) {

		cCount = (DivineLightning_FindCount+cnt)&OTHER_PLAYER_MASK;

		if ( chrOtherPlayer[cCount].Flag && chrOtherPlayer[cCount].dwObjectSerial && 
			lpTragetChar!=&chrOtherPlayer[cCount] && chrOtherPlayer[cCount].smCharInfo.State==smCHAR_STATE_ENEMY &&
			chrOtherPlayer[cCount].smCharInfo.Brood!=smCHAR_MONSTER_USER &&
			chrOtherPlayer[cCount].smCharInfo.Life[0]>0 ) {

				lpChar = &chrOtherPlayer[cCount];

				dx = (lpChar->pX-lpTragetChar->pX)>>FLOATNS;
				dy = (lpChar->pY-lpTragetChar->pY)>>FLOATNS;
				dz = (lpChar->pZ-lpTragetChar->pZ)>>FLOATNS;

				dist = dx*dx+dy*dy+dz*dz;

				if ( dist<dSize && abs(dy)<70 ) {
					lpTransSkill_AttackData->dwTarObjectSerial[lpTransSkill_AttackData->TargetCount++] = lpChar->dwObjectSerial;

					lpSelected_Char[dmSelected_CharCnt] = &chrOtherPlayer[cCount];
					dwSkill_DivineLightning_Target[dmSelected_CharCnt] = lpChar->dwObjectSerial;
					dmSelected_CharCnt++;

					if ( dmSelected_CharCnt>=SelCount ) {
						DivineLightning_FindCount = cCount;
						break;
					}

				}
			}
	}
	DivineLightning_Target_Count = dmSelected_CharCnt;

	return dmSelected_CharCnt;
}


//범위형 공격 범위 선정 (체인라이트닝)
int dm_SelectDamageChainCount( smCHAR *lpCurChar , smCHAR *lpTragetChar, int SelCount , int Size )
{

	int cnt,cnt2,cnt3;
	int dx,dy,dz,dist;
	int cCount;
	smCHAR	*lpChar;
	smCHAR	*lpLinkChar;
	int dSize;
	int minDist;
	smCHAR *lpMinChar;

	dm_OpenTransRangeDamage();
	dmSelected_CharCnt = 0;
	dmUseAccuracy = 0;

	//if ( lpCurChar!=lpCurPlayer ) return FALSE;

	dSize = Size*Size;

	lpSelected_Char[dmSelected_CharCnt] = lpTragetChar;
	dwSkill_DivineLightning_Target[dmSelected_CharCnt] = lpTragetChar->dwObjectSerial;
	lpTransSkill_AttackData->dwTarObjectSerial[lpTransSkill_AttackData->TargetCount++] = lpTragetChar->dwObjectSerial;
	dmSelected_CharCnt++;

	lpLinkChar = lpTragetChar;

	for( cnt2=1;cnt2<SelCount;cnt2++) {
		minDist = dSize;
		lpMinChar = 0;

		for( cnt=0;cnt<OTHER_PLAYER_MAX;cnt++ ) {

			cCount = cnt;

			if ( chrOtherPlayer[cCount].Flag && chrOtherPlayer[cCount].dwObjectSerial && 
				lpLinkChar!=&chrOtherPlayer[cCount] && chrOtherPlayer[cCount].smCharInfo.State==smCHAR_STATE_ENEMY &&
				chrOtherPlayer[cCount].smCharInfo.Brood!=smCHAR_MONSTER_USER &&
				chrOtherPlayer[cCount].smCharInfo.Life[0]>0 &&
				lpCurChar!=&chrOtherPlayer[cCount] ) {

					lpChar = &chrOtherPlayer[cCount];

					for(cnt3=0;cnt3<dmSelected_CharCnt;cnt3++) {
						if ( lpSelected_Char[cnt3]==lpChar ) { 
							lpChar = 0;
							break;
						}
					}

					if ( lpChar ) {
						dx = (lpChar->pX-lpLinkChar->pX)>>FLOATNS;
						dy = (lpChar->pY-lpLinkChar->pY)>>FLOATNS;
						dz = (lpChar->pZ-lpLinkChar->pZ)>>FLOATNS;

						dist = dx*dx+dz*dz;

						if ( dist<minDist && abs(dy)<70 ) {
							lpMinChar = lpChar;
							minDist = dist;
						}
					}
				}
		}
		if ( lpMinChar ) {

			lpTransSkill_AttackData->dwTarObjectSerial[lpTransSkill_AttackData->TargetCount++] = lpMinChar->dwObjectSerial;

			lpSelected_Char[dmSelected_CharCnt] = lpMinChar;
			dwSkill_DivineLightning_Target[dmSelected_CharCnt] = lpMinChar->dwObjectSerial;
			dmSelected_CharCnt++;

			lpLinkChar = lpMinChar;
		}
		//if ( cnt>=OTHER_PLAYER_MAX ) break;
	}

	DivineLightning_Target_Count = dmSelected_CharCnt;

	return dmSelected_CharCnt;
}

//선택된 범위공격 목록에 캐릭 강제 추가
int dm_AddRangeDamage( smCHAR *lpChar )
{
	int cnt;

	if ( lpTransSkill_AttackData->TargetCount>=SKIL_ATTACK_CHAR_MAX || !lpChar ) return FALSE;

	for(cnt=0;cnt<lpTransSkill_AttackData->TargetCount;cnt++) {
		if ( lpTransSkill_AttackData->dwTarObjectSerial[cnt]==lpChar->dwObjectSerial ) 
			return dmSelected_CharCnt;		//이미 추가되 있음
	}

	//목록에 없는 경우 추가
	lpTransSkill_AttackData->dwTarObjectSerial[lpTransSkill_AttackData->TargetCount++] = lpChar->dwObjectSerial;
	lpSelected_Char[dmSelected_CharCnt++] = lpChar;

	return dmSelected_CharCnt;
}


//dm_SelectRange

//범위공격 설정후 전송
int dm_SendRangeDamage( int x, int y, int z, smCHAR *lpMainTraget ,int PowParam1, int PowParam2, int AttackState , int Registance ,  DWORD dwSkillCode )
{
	int cnt;

	if ( !lpTransSkill_AttackData ) return FALSE;
	if ( !lpTransSkill_AttackData->TargetCount && !lpMainTraget ) return FALSE;

	dmAttackCount++;

	lpTransSkill_AttackData->code = smTRANSCODE_RANGE_DAMAGE;
	lpTransSkill_AttackData->size = (sizeof(TRANS_SKIL_ATTACKDATA2)-sizeof(DWORD)*SKIL_ATTACK_CHAR_MAX);
	lpTransSkill_AttackData->size += sizeof(DWORD)*lpTransSkill_AttackData->TargetCount+16;

	lpTransSkill_AttackData->x = x;
	lpTransSkill_AttackData->y = y;
	lpTransSkill_AttackData->z = z;
	lpTransSkill_AttackData->AttackCount = dmAttackCount;
	lpTransSkill_AttackData->dwTime = dwPlayTime;
	lpTransSkill_AttackData->AttackState = AttackState;
	if ( lpMainTraget ) {
		if ( dmUseAccuracy ) {
			for(cnt=0;cnt<lpTransSkill_AttackData->TargetCount;cnt++) {
				if ( lpTransSkill_AttackData->dwTarObjectSerial[cnt]==lpMainTraget->dwObjectSerial ) {
					lpTransSkill_AttackData->dwMainTargetObject = lpMainTraget->dwObjectSerial;
					break;
				}
			}
		}
		else
			lpTransSkill_AttackData->dwMainTargetObject = lpMainTraget->dwObjectSerial;

		lpMainTraget->TryAttackCount++;
	}
	lpTransSkill_AttackData->SkillCode = dwSkillCode;
	lpTransSkill_AttackData->Critical[0] = lpCurPlayer->smCharInfo.Critical_Hit;
	lpTransSkill_AttackData->dwDestObjectSerial = lpCurPlayer->dwObjectSerial;
	lpTransSkill_AttackData->Power[0] = lpCurPlayer->smCharInfo.Attack_Damage[0];
	lpTransSkill_AttackData->Power[1] = lpCurPlayer->smCharInfo.Attack_Damage[1];
	lpTransSkill_AttackData->Power2[0] = PowParam1;
	lpTransSkill_AttackData->Power2[1] = PowParam2;

	switch( lpCurPlayer->smCharInfo.JOB_CODE ) {
		case JOBCODE_PRIESTESS:
		case JOBCODE_MAGICIAN:
		case JOBCODE_SHAMAN:
			lpTransSkill_AttackData->Critical[1] = lpCurPlayer->smCharInfo.Spirit;			//정신력 저장
			break;

		case JOBCODE_MECHANICIAN:
		case JOBCODE_FIGHTER:
		case JOBCODE_PIKEMAN:
		case JOBCODE_KNIGHT:
		case JOBCODE_ASSASSIN:
		case JOBCODE_MARTIALARTIST:
			lpTransSkill_AttackData->Critical[1] = lpCurPlayer->smCharInfo.Strength;			//힘 저장
			break;

		case JOBCODE_ARCHER:
		case JOBCODE_ATALANTA:
			lpTransSkill_AttackData->Critical[1] = lpCurPlayer->smCharInfo.Dexterity;		//민첩 저장
			break;
	}

	lpTransSkill_AttackData->AttackSize = 240;

	lpTransSkill_AttackData->dwWeaponCode = lpCurPlayer->dwActionItemCode;

	if ( lpCurPlayer->OnStageField>=0 )
		lpTransSkill_AttackData->Area[0] = StageField[ lpCurPlayer->OnStageField ]->FieldCode;

	lpTransSkill_AttackData->dwChkSum = dm_GetRangeDamgeChkSum(lpTransSkill_AttackData);

	return dm_SendTransRangeDamage();
}



//							N M C F  S X
WORD	wLimitDamage[8] = { 0,0,0,0, 0,0,0,0 };
//							0 1 2 3  4 5

#ifdef PT_WEB
//////////////////////////////////////////////////////////////////////////////
// The server sends these three cipher modules as x86 machine code
// (dm_EncodePacket_AsmCode, dm_DecodePacket_AsmCode in SrcServer\Svr_Damge.cpp,
// smwsock_EncodePacket_AsmCode in smwsock.cpp) with its keys patched into the
// code. A browser cannot run x86 code: the same routines in C, with the keys
// read from where the server put them.
//////////////////////////////////////////////////////////////////////////////
static DWORD s_dmEncKey1, s_dmEncKey2;			// code+0x13: imul size,KEY1   code+0x1F: add code,KEY2
static DWORD s_dmDecKey1, s_dmDecKey2;
static DWORD s_PacketCode;						// code+0x0B: or dwKey,PACCODE
static BYTE  s_PacketEncXor, s_PacketDecXor;	// code+0x74, code+0x11A

static DWORD ModuleDword( TRANS_FUNC_MEMORY *m, int pos ) { DWORD v; memcpy( &v, m->szData+pos, 4 ); return v; }

static DWORD Web_EncodeDamagePacket( void *lpPacket )
{
	DWORD *lpDword = (DWORD *)lpPacket;
	DWORD size = lpDword[0]>>2;
	DWORD code = lpDword[1];
	DWORD dwCode = ( code^(code<<7) ) + size*s_dmEncKey1;
	DWORD cnt2 = ( code+s_dmEncKey2 )&0xF5ABCF;
	DWORD cnt3 = cnt2&0x3C;
	for ( DWORD cnt=2; cnt<size; cnt++ ) {
		dwCode ^= lpDword[cnt]^(cnt2<<6);
		lpDword[cnt] = dwCode;
		cnt2 += cnt2+cnt3;
	}
	return TRUE;
}

static DWORD Web_DecodeDamagePacket( rsPLAYINFO *, void *lpPacket )
{
	DWORD *lpDword = (DWORD *)lpPacket;
	DWORD size = lpDword[0]>>2;
	DWORD code = lpDword[1];
	DWORD dwCode = ( code^(code<<5) ) + size*s_dmDecKey1;
	DWORD cnt2 = ( code+s_dmDecKey2 )&0xBCAFF5;
	DWORD cnt3 = cnt2&0x5E;
	for ( DWORD cnt=2; cnt<size; cnt++ ) {
		DWORD dwCode2 = lpDword[cnt];
		lpDword[cnt] = dwCode2^(cnt2<<7)^dwCode;
		dwCode = dwCode2;
		cnt2 += cnt2+cnt3;
	}
	return TRUE;
}

// EncodePacket2/DecodePacket2 (smwsock.cpp) with the server's packet code and xor bytes
static DWORD Web_EncodePacket( DWORD dwKey, DWORD PacketCnt, BYTE *lpPacket, BYTE *lpEncBuff )
{
	dwKey |= s_PacketCode;
	DWORD sKey = (dwKey<<16)|(PacketCnt&0xFFFF);
	int len = ((int *)lpPacket)[0];
	((int *)lpEncBuff)[0] = len+8;
	((int *)lpEncBuff)[2] = sKey;
	DWORD rKey = (sKey>>4)+len;
	BYTE bNear = (BYTE)(PacketCnt+dwKey);
	WORD wTotal = 0;
	for ( int cnt=4; cnt<len; cnt++ ) {
		wTotal += lpPacket[cnt];
		BYTE ch = lpPacket[cnt]^((BYTE)rKey)^bNear;
		lpEncBuff[cnt+8] = ch;
		rKey += cnt*(cnt>>1);
		bNear = ch^s_PacketEncXor;
	}
	((int *)lpEncBuff)[1] = (dwKey&0xFFFF0000)|wTotal;
	return TRUE;
}

static DWORD Web_DecodePacket( BYTE *lpPacket, BYTE *lpDecBuff )
{
	int len = ((int *)lpPacket)[0]-8;
	DWORD sKey = ((int *)lpPacket)[2];
	DWORD dwKey = sKey>>16;
	DWORD PacketCnt = sKey&0xFFFF;
	DWORD rKey = (sKey>>4)+len;
	BYTE bNear = (BYTE)(PacketCnt+dwKey);
	WORD wTotal = 0;
	((int *)lpDecBuff)[0] = len;
	for ( int cnt=4; cnt<len; cnt++ ) {
		BYTE ch2 = lpPacket[cnt+8];
		BYTE ch = ch2^((BYTE)rKey)^bNear;
		lpDecBuff[cnt] = ch;
		rKey += cnt*(cnt>>1);
		bNear = ch2^s_PacketDecXor;
		wTotal += ch;
	}
	if ( (WORD)((int *)lpPacket)[1]!=wTotal ) return 0;
	return sKey;
}

int RecvDamagePacketModule( TRANS_FUNC_MEMORY *lpTransFuncModule )
{
	s_dmEncKey1 = ModuleDword( lpTransFuncModule, 0x13 );
	s_dmEncKey2 = ModuleDword( lpTransFuncModule, 0x1F );
	fnEncodeDamagePacket = Web_EncodeDamagePacket;
	ZeroMemory( wLimitDamage , sizeof(WORD)*8 );
	return TRUE;
}

int RecvDamagePacketModule2( TRANS_FUNC_MEMORY *lpTransFuncModule )
{
	s_dmDecKey1 = ModuleDword( lpTransFuncModule, 0x13 );
	s_dmDecKey2 = ModuleDword( lpTransFuncModule, 0x1F );
	fnDecodeDamagePacket = Web_DecodeDamagePacket;
	return TRUE;
}

int RecvDynPacketModule( TRANS_FUNC_MEMORY *lpTransFuncModule )
{
	s_PacketCode = ModuleDword( lpTransFuncModule, 0x0B );
	s_PacketEncXor = (BYTE)lpTransFuncModule->szData[0x74];
	s_PacketDecXor = (BYTE)lpTransFuncModule->szData[0x11A];
	fnEncodePacket = Web_EncodePacket;
	fnDecodePacket = Web_DecodePacket;
	return TRUE;
}

#else

//데미지 암호화 모듈 수신
int RecvDamagePacketModule( TRANS_FUNC_MEMORY *lpTransFuncModule )
{
	char *lpBuff;

	lpBuff = new char[lpTransFuncModule->size];
	memcpy( lpBuff , lpTransFuncModule->szData , lpTransFuncModule->Param[0] );

	fnEncodeDamagePacket = (LPFN_EncodeDamagePacket)lpBuff;

//	if ( (wLimitDamage[3]^wLimitDamage[5])==0 )
//		SendMaxDamageToServer( 0,0,0 );

	ZeroMemory( wLimitDamage , sizeof(WORD)*8 );

	return TRUE;
}

//데미지 암호화 모듈 수신
int RecvDamagePacketModule2( TRANS_FUNC_MEMORY *lpTransFuncModule )
{
	char *lpBuff;

	if ( !fnDecodeDamagePacket )
		lpBuff = new char[lpTransFuncModule->size];
	else
		lpBuff = (char *)fnDecodeDamagePacket;

	memcpy( lpBuff , lpTransFuncModule->szData , lpTransFuncModule->Param[0] );
	fnDecodeDamagePacket = (LPFN_DecodeDamagePacket)lpBuff;

	return TRUE;
}

//패킷 동적 암호화 모듈 수신
int RecvDynPacketModule( TRANS_FUNC_MEMORY *lpTransFuncModule )
{
	char *lpBuff;

	if ( !fnEncodePacket )
		lpBuff = new char[lpTransFuncModule->size];
	else
		lpBuff = (char *)fnEncodePacket;

	memcpy( lpBuff , lpTransFuncModule->szData , lpTransFuncModule->Param[0] );
	fnEncodePacket = (LPFN_EncodePacket)(lpBuff+lpTransFuncModule->Param[1]);
	fnDecodePacket = (LPFN_DecodePacket)(lpBuff+lpTransFuncModule->Param[2]);

	return TRUE;
}

#endif // PT_WEB


int CheckMaxDamage()
{
	if ( (wLimitDamage[3]^wLimitDamage[5])==0 && lpCurPlayer->smCharInfo.JOB_CODE>0 )
		cInvenTory.CheckDamage();

	return TRUE;
}

int SetMaxDamage( smCHAR_INFO *lpCharInfo )
{
	WORD	wDamage[8];
	WORD	wSum;

	wDamage[0] = wLimitDamage[0] ^ wLimitDamage[5];
	wDamage[1] = wLimitDamage[1] ^ wLimitDamage[5];
	wDamage[2] = wLimitDamage[2] ^ wLimitDamage[5];
	wDamage[3] = wLimitDamage[3] ^ wLimitDamage[5];
	wDamage[4] = wLimitDamage[4] ^ wLimitDamage[5];

	wSum = wDamage[0]*3+wDamage[1]*5+wDamage[2]*7+wDamage[3]*9;

	if ( wSum!=wDamage[4] || lpCurPlayer->smCharInfo.JOB_CODE==0 ) return FALSE;

	lpCharInfo->Attack_Damage[0] = wDamage[0];
	lpCharInfo->Attack_Damage[1] = wDamage[1];
	lpCharInfo->Critical_Hit = wDamage[2];

	return TRUE;
}

int	SendMaxDamageToServer( WORD MinDamage , WORD MaxDamage , WORD Critical )
{
	WORD	wTime = (WORD)dwPlayTime;

	WORD	wDamage[8];
	WORD	wSum;
	smTRANS_COMMAND	smTransCommand;

	wDamage[0] = wLimitDamage[0] ^ wLimitDamage[5];
	wDamage[1] = wLimitDamage[1] ^ wLimitDamage[5];
	wDamage[2] = wLimitDamage[2] ^ wLimitDamage[5];
	wDamage[3] = wLimitDamage[3] ^ wLimitDamage[5];
	wDamage[4] = wLimitDamage[4] ^ wLimitDamage[5];

	wSum = wDamage[0]*3+wDamage[1]*5+wDamage[2]*7+wDamage[3]*9;
	if ( wSum!=wDamage[4] || lpCurPlayer->smCharInfo.JOB_CODE==0 ) return FALSE;
/*
	if ( wDamage[3]==0 && !MinDamage && !MaxDamage && !Critical ) {
		MinDamage = wDamage[0];
		MaxDamage = wDamage[1];
		Critical = wDamage[2];

		wDamage[0]=0;
		wDamage[1]=0;
		wDamage[2]=0;
	}
*/
	if ( wDamage[0]<MinDamage || wDamage[1]<MaxDamage || wDamage[2]<Critical || wDamage[3]==0 ) {
		//신규 변경
		smTransCommand.code = smTRANSCODE_LIMIT_DAMAGE;
		smTransCommand.size = sizeof(smTRANS_COMMAND);
		smTransCommand.WParam = 0;
		smTransCommand.LParam = MinDamage;
		smTransCommand.SParam = MaxDamage;
		smTransCommand.EParam = Critical;
		smTransCommand.WParam = dm_GetCommandChkSum( &smTransCommand , lpCurPlayer->dwObjectSerial );
		if ( fnEncodeDamagePacket ) {
			fnEncodeDamagePacket( &smTransCommand );			//암호화

			if ( smWsockServer ) 
				smWsockServer->Send2( (char *)&smTransCommand , smTransCommand.size , TRUE );
			if ( smWsockUserServer && smWsockServer!=smWsockUserServer ) 
				smWsockUserServer->Send2( (char *)&smTransCommand , smTransCommand.size , TRUE );
			if ( smWsockExtendServer && smWsockServer!=smWsockExtendServer ) 
				smWsockExtendServer->Send2( (char *)&smTransCommand , smTransCommand.size , TRUE );

			if ( lpWSockServer_Area[0] )
				lpWSockServer_Area[0]->Send2( (char *)&smTransCommand , smTransCommand.size , TRUE );
			if ( lpWSockServer_Area[1] && lpWSockServer_Area[0]!=lpWSockServer_Area[1] )
				lpWSockServer_Area[1]->Send2( (char *)&smTransCommand , smTransCommand.size , TRUE );

			wLimitDamage[3] = wDamage[3]+1;
		}
		wLimitDamage[0] = MinDamage;
		wLimitDamage[1] = MaxDamage;
		wLimitDamage[2] = Critical;
		wLimitDamage[4] = wLimitDamage[0]*3+wLimitDamage[1]*5+wLimitDamage[2]*7+wLimitDamage[3]*9;
		wLimitDamage[5] = wTime;

		wLimitDamage[0] ^= wLimitDamage[5];
		wLimitDamage[1] ^= wLimitDamage[5];
		wLimitDamage[2] ^= wLimitDamage[5];
		wLimitDamage[3] ^= wLimitDamage[5];
		wLimitDamage[4] ^= wLimitDamage[5];
	}

	return TRUE;
}

DWORD dwSpeedProtectCode = 0;
DWORD dwSkillProtectCode = 0;

DWORD GetSpeedProtectCode( smCHAR *lpChar )
{
	DWORD	dwCode;

	dwCode = lpChar->frame;
	dwCode += lpChar->FrameStep<<2;
	dwCode += lpChar->MoveSpeed<<5;
	dwCode += lpChar->AttackSpeed<<7;
	dwCode = ( dwCode&0xFFFFEFFF );

	if ( (dwSpeedProtectCode&0x1000)==0 ) return dwCode;

	return NULL;
}

DWORD GetSkillProtectCode( smCHAR *lpChar )
{
	DWORD	dwCode;

	dwCode = lpChar->AttackSkil;
	dwCode += lpChar->frame<<4;
	dwCode = ( dwCode&0xFFFFEFFF );

	if ( (dwSkillProtectCode&0x1000)==0 ) return dwCode;

	return NULL;
}


//속도 변수 Lock
int	LockSpeedProtect( smCHAR *lpChar )
{
	DWORD	dwCode;

	dwCode = GetSpeedProtectCode( lpChar );
	if ( dwCode )
		dwSpeedProtectCode = dwCode;

	dwCode = GetSkillProtectCode( lpChar );
	if ( dwCode )
		dwSkillProtectCode = dwCode;


	return TRUE;
}

//속도 변수 UnLock
int UnlockSpeedProtect( smCHAR *lpChar )
{
	DWORD dwCode;

	smTRANS_COMMAND	smTransCommand;
	ZeroMemory( &smTransCommand , sizeof(smTRANS_COMMAND) );


	dwCode = GetSpeedProtectCode( lpChar );

	if ( dwCode==NULL ) {
		dwSpeedProtectCode++;
		return TRUE;
	}

	if ( dwSpeedProtectCode!=dwCode ) {
		//스피드 코드 오류
		smTransCommand.code = TRUE;
		smTransCommand.WParam = 100;
		smTransCommand.LParam = lpChar->FrameStep;
		smTransCommand.SParam = 0;

		dwSpeedProtectCode|=0x1000;
	}

	dwCode = GetSkillProtectCode( lpChar );

	if ( dwCode==NULL ) {
		dwSkillProtectCode++;
		return TRUE;
	}

	if ( dwSkillProtectCode!=dwCode ) {
		//스킬 코드 오류
		smTransCommand.code = TRUE;
		smTransCommand.WParam = 101;
		smTransCommand.LParam = lpChar->AttackSkil;
		smTransCommand.SParam = 0;

		dwSkillProtectCode|=0x1000;
	}

	if ( smTransCommand.code==TRUE ) {

		smTransCommand.code = smTRANSCODE_WARNING_CLIENT;
		smTransCommand.EParam = 0;
		smTransCommand.EParam = dm_GetCommandChkSum( &smTransCommand , lpCurPlayer->dwObjectSerial );
		smTransCommand.size = sizeof(smTRANS_COMMAND);

		if ( fnEncodeDamagePacket ) {
			fnEncodeDamagePacket( &smTransCommand );			//암호화
			if ( smWsockDataServer ) {
				smWsockDataServer->Send2( (char *)&smTransCommand , smTransCommand.size , TRUE );
			}
		}
	}

	return TRUE;
}




/*
int	dm_EncodePacket( void *lpPacket )
{
	DWORD size,code;
	DWORD cnt,cnt2,cnt3;
	DWORD dwCode;
	DWORD *lpDword = (DWORD *)lpPacket;

	size = lpDword[0];
	code = lpDword[1];
	size /=4;


	dwCode = (code^(code<<7)) + size*6604;
	cnt2 = (code+lpCurPlayer->dwObjectSerial)&0xFFCF;
	cnt3 = cnt2&0x3C;

	for( cnt=2;cnt<size;cnt++ ) {
		dwCode = dwCode^(cnt2<<13)^lpDword[cnt];
		lpDword[cnt] = dwCode;
		cnt2+=cnt2+cnt3;
	}

	return TRUE;
}


#define	DAMAGE_PACKET_KEY	0xBBAA3840
#define	DAMAGE_PACKET_MUL	0x11223344

int	dm_DecodePacket2( DWORD dwKey1, DWORD dwKey2 , void *lpPacket );

int	dm_EncodePacket2( void *lpPacket )
{
	DWORD size,code;
	DWORD cnt,cnt2,cnt3;
	DWORD dwCode;
	DWORD *lpDword = (DWORD *)lpPacket;

	size = lpDword[0];
	code = lpDword[1];
	size /=4;


	dwCode = (code^(code<<7)) + size*DAMAGE_PACKET_MUL;
	cnt2 = (code+DAMAGE_PACKET_KEY)&0xF5ABCF;
	cnt3 = cnt2&0x3C;

	for( cnt=2;cnt<size;cnt++ ) {
		dwCode = dwCode^(cnt2<<6)^lpDword[cnt];
		lpDword[cnt] = dwCode;
		cnt2+=cnt2+cnt3;
	}

	return TRUE;
}


int	dm_DecodePacket2( DWORD dwKey1, DWORD dwKey2 , void *lpPacket )
{

#ifdef _W_SERVER
	DWORD size,code;
	DWORD cnt,cnt2,cnt3;
	DWORD dwCode;
	DWORD dwCode2;
	DWORD *lpDword = (DWORD *)lpPacket;

	size = lpDword[0];
	code = lpDword[1];
	size /=4;

	dwCode = (code^(code<<7)) + size*dwKey2;
	cnt2 = (code+dwKey1)&0xF5ABCF;
	cnt3 = cnt2&0x3C;

	for( cnt=2;cnt<size;cnt++ ) {
		dwCode2 = lpDword[cnt];
		lpDword[cnt] = dwCode2^(cnt2<<6)^dwCode;
		dwCode = dwCode2;
		cnt2+=cnt2+cnt3;
	}
#endif
	return TRUE;
}



int	dm_DecodePacket( DWORD dwObjectSerial , void *lpPacket )
{

#ifdef _W_SERVER
	DWORD size,code;
	DWORD cnt,cnt2,cnt3;
	DWORD dwCode;
	DWORD dwCode2;
	DWORD *lpDword = (DWORD *)lpPacket;

	size = lpDword[0];
	code = lpDword[1];
	size /=4;

	dwCode = (code^(code<<7)) + size*6604;
	cnt2 = (code+dwObjectSerial)&0xFFCF;
	cnt3 = cnt2&0x3C;

	for( cnt=2;cnt<size;cnt++ ) {
		dwCode2 = lpDword[cnt];
		lpDword[cnt] = dwCode2^(cnt2<<13)^dwCode;
		dwCode = dwCode2;
		cnt2+=cnt2+cnt3;
	}
#endif
	return TRUE;
}
*/

//0x1f - ffaa3840
//0x13 - 11223344
/*
struct	TRANS_FUNC_MEMORY	{
	int		size,code;
	int		Param[8];
	char	szData[6000];
};
	fnChkMem = (LPFN_CheckMem)((void *)TransFuncMem->szData);

	smTransCommand.size = 0;

	fnChkMem( TransFuncMem , &smTransCommand );

typedef DWORD (*LPFN_CheckMem)( TRANS_FUNC_MEMORY *TransFuncMem , smTRANS_COMMAND	*lpTransCommand );
LPFN_CheckMem	fnChkMem2;

*/

