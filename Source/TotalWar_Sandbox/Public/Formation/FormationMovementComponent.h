// Copyright Total War Sandbox. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TotalWar_Sandbox/Public/Core/TWTypes.h"
#include "TotalWar_Sandbox/Public/Formation/FormationCommand.h"
#include "FormationMovementComponent.generated.h"

class AFormationActor;
class UFormationSlotComponent;
class USoldierVisualComponent;
struct FSoldierEntity;

/**
 * UFormationMovementComponent — Bộ Điều Khiển Di Chuyển & Lái Lính.
 * Trách nhiệm duy nhất (SRP):
 *   - Quản lý trạng thái di chuyển của đạo quân (March / Idle / Charge).
 *   - Tính toán dịch chuyển tâm đạo quân (Formation Pivot Translation).
 *   - Thuật toán Local Steering cho từng lính bám slot, xoay góc mặt và kiểm tra đến đích.
 * Follows Single Responsibility Principle (SRP) & SOLID architecture.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TOTALWAR_SANDBOX_API UFormationMovementComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UFormationMovementComponent();

	// ========================================================================
	// PUBLIC API
	// ========================================================================

	/** Initializes movement settings with owning formation actor and speed */
	void InitializeMovement(AFormationActor* InOwnerActor, float InMoveSpeed);

	/** Executes a tactical movement or placement command */
	void ExecuteMoveCommand(
		const FFormationCommand& Command,
		TArray<FSoldierEntity>& Soldiers,
		UFormationSlotComponent* SlotComponent
	);

	/** Updates formation pivot and soldier steering per tick */
	void UpdateMovement(
		float DeltaTime,
		TArray<FSoldierEntity>& Soldiers,
		UFormationSlotComponent* SlotComponent,
		USoldierVisualComponent* VisualComponent
	);

	/** Immediately halts squad movement and snaps soldiers to slots */
	void StopMovement(
		TArray<FSoldierEntity>& Soldiers,
		UFormationSlotComponent* SlotComponent,
		USoldierVisualComponent* VisualComponent
	);

	// ========================================================================
	// GETTERS & SETTERS (Encapsulation)
	// ========================================================================

	FORCEINLINE bool IsMoving() const { return bIsMoving; }
	FORCEINLINE const FVector& GetTargetDestination() const { return TargetDestination; }
	FORCEINLINE const FVector& GetFinalDesiredFacing() const { return FinalDesiredFacing; }
	FORCEINLINE float GetCurrentSpeed() const { return CurrentSpeed; }
	FORCEINLINE void SetCurrentSpeed(float NewSpeed) { CurrentSpeed = FMath::Max(0.0f, NewSpeed); }

	FORCEINLINE AFormationActor* GetTargetEnemyFormation() const { return TargetEnemyFormation.Get(); }
	FORCEINLINE bool IsAttacking() const { return TargetEnemyFormation != nullptr; }

	/** Returns current Charge Bonus multiplier (1.0 at impact, linearly decaying to 0.0 over 10s) */
	FORCEINLINE float GetChargeBonusRatio() const
	{
		return (TWConstants::ChargeDecayDuration > 0.0f)
			? FMath::Clamp(ChargeBonusTimer / TWConstants::ChargeDecayDuration, 0.0f, 1.0f)
			: 0.0f;
	}

	FORCEINLINE bool HasChargeBonus() const { return ChargeBonusTimer > 0.0f; }

protected:
	UPROPERTY(Transient)
	TObjectPtr<AFormationActor> OwnerFormation;

	UPROPERTY(Transient)
	TObjectPtr<AFormationActor> TargetEnemyFormation;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	float BaseMoveSpeed;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Movement")
	float CurrentSpeed;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Combat")
	float ChargeBonusTimer;

	bool bIsMoving;
	bool bHasImpacted;
	FVector TargetDestination;
	FVector FinalDesiredFacing;
};
