// Copyright Total War Sandbox. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "TotalWar_Sandbox/Public/Core/TWTypes.h"
#include "CombatSubsystem.generated.h"

class AFormationActor;
class USpatialGridSubsystem;
class UFormationSubsystem;

/**
 * UCombatSubsystem — Hệ Thống Giao Tranh Cận Chiến Thế Giới (Combat Core Subsystem).
 * Trách nhiệm duy nhất (SRP):
 *   - Quản lý chu kỳ giao tranh cận chiến (Combat Tick ~0.2s).
 *   - Phát hiện tiền tuyến (Frontline Detection) qua USpatialGridSubsystem (O(1)).
 *   - Áp dụng công thức Hit Chance & Armor Piercing Damage chuẩn Total War.
 *   - Xử lý lính tử trận và tự động thu gọn đội hình (Regroup).
 * Follows Single Responsibility Principle (SRP) & Dependency Inversion Principle (DIP).
 */
UCLASS()
class TOTALWAR_SANDBOX_API UCombatSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	UCombatSubsystem();

	// ========================================================================
	// UTickableWorldSubsystem INTERFACE
	// ========================================================================

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;
	virtual bool IsTickable() const override { return !IsTemplate(); }

	// ========================================================================
	// PUBLIC COMBAT API
	// ========================================================================

	/** Executes one discrete combat evaluation tick across all frontline soldiers */
	void ProcessCombatTick();

	/** Resolves a single melee attack exchange between an attacker and defender */
	void ResolveMeleeAttack(
		AFormationActor* AttackerFormation,
		int32 AttackerSoldierIndex,
		AFormationActor* DefenderFormation,
		int32 DefenderSoldierIndex
	);

	// ========================================================================
	// GETTERS & CONFIG (Encapsulation)
	// ========================================================================

	FORCEINLINE float GetCombatTickInterval() const { return CombatTickInterval; }
	FORCEINLINE void SetCombatTickInterval(float NewInterval) { CombatTickInterval = FMath::Max(0.05f, NewInterval); }

protected:
	/** Melee combat tick evaluation interval in seconds (Default: 0.2s for 60 FPS performance) */
	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	float CombatTickInterval;

	float CombatTimer;

	UPROPERTY(Transient)
	TObjectPtr<USpatialGridSubsystem> SpatialGridSubsystem;

	UPROPERTY(Transient)
	TObjectPtr<UFormationSubsystem> FormationSubsystem;
};
