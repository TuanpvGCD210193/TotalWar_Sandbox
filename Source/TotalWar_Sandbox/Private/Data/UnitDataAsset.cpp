// Copyright Total War Sandbox. All Rights Reserved.

#include "TotalWar_Sandbox/Public/Data/UnitDataAsset.h"

UUnitDataAsset::UUnitDataAsset()
{
	DisplayName = FText::FromString(TEXT("Melee Infantry"));
	UnitType = EUnitType::MeleeInfantry;

	// Formation defaults
	DefaultEntityCount = TWConstants::DefaultEntityCount;
	DefaultFormationWidth = TWConstants::DefaultFormationWidth;
	PreferredRankCount = 6;
	MinFormationWidth = TWConstants::MinFormationWidth;
	MaxFormationWidth = TWConstants::MaxFormationWidth;
	SoldierSpacing = TWConstants::DefaultSoldierSpacing;

	// Base stats for prototype melee infantry
	HealthPerSoldier = 100.0f;
	Armour = 30.0f;
	Leadership = 100.0f;
	Speed = TWConstants::DefaultMoveSpeed;
	MeleeAttack = 32.0f;
	MeleeDefence = 28.0f;
	BaseDamage = 20.0f;
	ArmourPiercingDamage = 8.0f;
	ChargeBonus = 20.0f;
	Mass = 100.0f;
	AttackInterval = TWConstants::DefaultAttackInterval;
	AttackRange = TWConstants::DefaultMeleeRange;
}
