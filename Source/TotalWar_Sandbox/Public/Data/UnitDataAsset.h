// Copyright Total War Sandbox. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "TotalWar_Sandbox/Public/Core/TWTypes.h"
#include "UnitDataAsset.generated.h"

/**
 * Defines the static archetype, combat stats, and default formation settings for a unit type.
 * All soldier entities in a squad share these base stats (Flyweight Pattern).
 * Follows Single Responsibility Principle (SRP): Only stores archetype configuration data.
 */
UCLASS(BlueprintType)
class TOTALWAR_SANDBOX_API UUnitDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UUnitDataAsset();

	// ========================================================================
	// GETTERS (Encapsulation)
	// ========================================================================

	FORCEINLINE const FText& GetDisplayName() const { return DisplayName; }
	FORCEINLINE EUnitType GetUnitType() const { return UnitType; }

	FORCEINLINE int32 GetDefaultEntityCount() const { return DefaultEntityCount; }
	FORCEINLINE int32 GetDefaultFormationWidth() const { return DefaultFormationWidth; }
	FORCEINLINE int32 GetPreferredRankCount() const { return PreferredRankCount; }
	FORCEINLINE int32 GetMinFormationWidth() const { return MinFormationWidth; }
	FORCEINLINE int32 GetMaxFormationWidth() const { return MaxFormationWidth; }
	FORCEINLINE float GetSoldierSpacing() const { return SoldierSpacing; }

	FORCEINLINE float GetHealthPerSoldier() const { return HealthPerSoldier; }
	FORCEINLINE float GetArmour() const { return Armour; }
	FORCEINLINE float GetLeadership() const { return Leadership; }
	FORCEINLINE float GetSpeed() const { return Speed; }
	FORCEINLINE float GetMeleeAttack() const { return MeleeAttack; }
	FORCEINLINE float GetMeleeDefence() const { return MeleeDefence; }
	FORCEINLINE float GetBaseDamage() const { return BaseDamage; }
	FORCEINLINE float GetArmourPiercingDamage() const { return ArmourPiercingDamage; }
	FORCEINLINE float GetChargeBonus() const { return ChargeBonus; }
	FORCEINLINE float GetMass() const { return Mass; }
	FORCEINLINE float GetAttackInterval() const { return AttackInterval; }
	FORCEINLINE float GetAttackRange() const { return AttackRange; }

	/** Total Weapon Strength = Base Damage + Armour-Piercing Damage */
	UFUNCTION(BlueprintPure, Category = "Stats|Damage")
	FORCEINLINE float GetWeaponStrength() const { return BaseDamage + ArmourPiercingDamage; }

protected:
	// ========================================================================
	// GENERAL & IDENTITY
	// ========================================================================

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Identity")
	FText DisplayName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Identity")
	EUnitType UnitType;

	// ========================================================================
	// FORMATION DEFAULTS
	// ========================================================================

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Formation", meta = (ClampMin = "1", ClampMax = "500"))
	int32 DefaultEntityCount;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Formation", meta = (ClampMin = "1", ClampMax = "100"))
	int32 DefaultFormationWidth;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Formation", meta = (ClampMin = "1", ClampMax = "50"))
	int32 PreferredRankCount;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Formation", meta = (ClampMin = "1", ClampMax = "100"))
	int32 MinFormationWidth;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Formation", meta = (ClampMin = "1", ClampMax = "100"))
	int32 MaxFormationWidth;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Formation", meta = (ClampMin = "30.0", ClampMax = "500.0"))
	float SoldierSpacing;

	// ========================================================================
	// ATTRIBUTES & STATS
	// ========================================================================

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats|Health", meta = (ClampMin = "1.0"))
	float HealthPerSoldier;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats|Armour", meta = (ClampMin = "0.0", ClampMax = "200.0"))
	float Armour;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats|Morale", meta = (ClampMin = "0.0", ClampMax = "100.0"))
	float Leadership;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats|Movement", meta = (ClampMin = "50.0", ClampMax = "2000.0"))
	float Speed;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats|Melee", meta = (ClampMin = "0.0", ClampMax = "200.0"))
	float MeleeAttack;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats|Melee", meta = (ClampMin = "0.0", ClampMax = "200.0"))
	float MeleeDefence;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats|Damage", meta = (ClampMin = "0.0"))
	float BaseDamage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats|Damage", meta = (ClampMin = "0.0"))
	float ArmourPiercingDamage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats|Charge", meta = (ClampMin = "0.0"))
	float ChargeBonus;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats|Physics", meta = (ClampMin = "10.0", ClampMax = "5000.0"))
	float Mass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats|Combat", meta = (ClampMin = "0.1", ClampMax = "10.0"))
	float AttackInterval;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats|Combat", meta = (ClampMin = "0.0"))
	float AttackRange;
};
