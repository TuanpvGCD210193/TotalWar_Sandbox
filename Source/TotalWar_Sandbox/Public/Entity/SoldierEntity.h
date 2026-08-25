// Copyright Total War Sandbox. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "TotalWar_Sandbox/Public/Core/TWTypes.h"
#include "TotalWar_Sandbox/Public/Entity/SoldierHandle.h"
#include "SoldierEntity.generated.h"

/**
 * Runtime data struct for an individual soldier entity.
 * Cache-friendly plain data layout (~64 bytes).
 * Follows Single Responsibility Principle (SRP): Only stores individual soldier runtime state.
 * NOT an AActor or UObject.
 */
USTRUCT(BlueprintType)
struct TOTALWAR_SANDBOX_API FSoldierEntity
{
	GENERATED_BODY()

public:
	// ========================================================================
	// SPATIAL & MOVEMENT (Hot Data - Tick every frame)
	// ========================================================================

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transform")
	FVector Position = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transform")
	FVector Velocity = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transform")
	FRotator Rotation = FRotator::ZeroRotator;

	// ========================================================================
	// COMBAT RUNTIME (Tick at 10Hz Combat Subsystem)
	// ========================================================================

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float CurrentHP = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float AttackCooldown = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	FSoldierHandle TargetHandle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float CurrentChargeBonus = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float ChargeTimer = 0.0f;

	// ========================================================================
	// FORMATION & INSTANCE STATE
	// ========================================================================

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
	ESoldierState State = ESoldierState::Idle;

	/** Index of the assigned slot in the parent AFormationActor's slot array */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation")
	int32 SlotIndex = INDEX_NONE;

	/** Index of this soldier's instance in the Instanced Static Mesh Component */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
	int32 VisualInstanceIndex = INDEX_NONE;

	// ========================================================================
	// CONSTRUCTORS
	// ========================================================================

	FSoldierEntity()
		: Position(FVector::ZeroVector)
		, Velocity(FVector::ZeroVector)
		, Rotation(FRotator::ZeroRotator)
		, CurrentHP(100.0f)
		, AttackCooldown(0.0f)
		, TargetHandle()
		, CurrentChargeBonus(0.0f)
		, ChargeTimer(0.0f)
		, State(ESoldierState::Idle)
		, SlotIndex(INDEX_NONE)
		, VisualInstanceIndex(INDEX_NONE)
	{
	}

	FSoldierEntity(const FVector& InPosition, const FRotator& InRotation, float InMaxHP)
		: Position(InPosition)
		, Velocity(FVector::ZeroVector)
		, Rotation(InRotation)
		, CurrentHP(InMaxHP)
		, AttackCooldown(0.0f)
		, TargetHandle()
		, CurrentChargeBonus(0.0f)
		, ChargeTimer(0.0f)
		, State(ESoldierState::Idle)
		, SlotIndex(INDEX_NONE)
		, VisualInstanceIndex(INDEX_NONE)
	{
	}

	// ========================================================================
	// PUBLIC UTILITY METHODS
	// ========================================================================

	/** Checks if the soldier is alive and valid */
	FORCEINLINE bool IsAlive() const
	{
		return CurrentHP > 0.0f && State != ESoldierState::Dead;
	}

	/** Applies damage and returns true if this hit was lethal */
	FORCEINLINE bool TakeDamage(float DamageAmount)
	{
		if (!IsAlive())
		{
			return false;
		}

		CurrentHP = FMath::Max(0.0f, CurrentHP - DamageAmount);
		if (CurrentHP <= 0.0f)
		{
			State = ESoldierState::Dead;
			ResetTarget();
			return true;
		}
		return false;
	}

	/** Assigns this soldier to a formation slot */
	FORCEINLINE void AssignSlot(int32 InSlotIndex)
	{
		SlotIndex = InSlotIndex;
	}

	/** Clears current target reference */
	FORCEINLINE void ResetTarget()
	{
		TargetHandle.Invalidate();
	}

	/** Applies charge bonus with decay duration */
	FORCEINLINE void ApplyChargeBonus(float Bonus)
	{
		CurrentChargeBonus = Bonus;
		ChargeTimer = TWConstants::ChargeDecayDuration;
	}

	/** Ticks down attack cooldowns and charge decay */
	FORCEINLINE void TickCooldowns(float DeltaTime)
	{
		if (AttackCooldown > 0.0f)
		{
			AttackCooldown = FMath::Max(0.0f, AttackCooldown - DeltaTime);
		}

		if (ChargeTimer > 0.0f)
		{
			ChargeTimer = FMath::Max(0.0f, ChargeTimer - DeltaTime);
			// Linear decay of charge bonus over duration
			const float DecayFraction = ChargeTimer / TWConstants::ChargeDecayDuration;
			CurrentChargeBonus = CurrentChargeBonus * DecayFraction;
			if (ChargeTimer <= 0.0f)
			{
				CurrentChargeBonus = 0.0f;
			}
		}
	}
};
