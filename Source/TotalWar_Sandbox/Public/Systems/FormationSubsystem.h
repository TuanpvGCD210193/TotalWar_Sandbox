// Copyright Total War Sandbox. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "TotalWar_Sandbox/Public/Core/TWTypes.h"
#include "TotalWar_Sandbox/Public/Entity/FormationSlot.h"
#include "FormationSubsystem.generated.h"

class AFormationActor;

/**
 * World Subsystem managing all active formations on the battlefield.
 * Centralizes formation registry, slot geometry calculations, and global squad queries.
 * Follows Single Responsibility Principle (SRP) & Dependency Inversion Principle (DIP).
 */
UCLASS()
class TOTALWAR_SANDBOX_API UFormationSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// ========================================================================
	// REGISTRY MANAGEMENT
	// ========================================================================

	/** Registers an active formation into the subsystem registry */
	void RegisterFormation(AFormationActor* Formation);

	/** Unregisters a destroyed/removed formation */
	void UnregisterFormation(AFormationActor* Formation);

	/** Retrieves a formation by its unique ID */
	AFormationActor* GetFormationByID(int32 InFormationID) const;

	/** Retrieves all active formations matching the given team */
	void GetFormationsByTeam(ETeamID InTeam, TArray<AFormationActor*>& OutFormations) const;

	/** Retrieves all active formations in the battlefield */
	void GetAllFormations(TArray<AFormationActor*>& OutFormations) const;

	/** Finds the nearest enemy formation to a given location */
	AFormationActor* FindNearestEnemyFormation(const FVector& Location, ETeamID MyTeam) const;

	// ========================================================================
	// GEOMETRY & ALGORITHM (Pure Math Helper)
	// ========================================================================

	/**
	 * Calculates the geometric slot grid positions for any given formation parameters.
	 * Used by both live formations and placement preview rendering.
	 */
	static void CalculateSlotLayout(
		int32 EntityCount,
		int32 FormationWidth,
		float Spacing,
		const FVector& Center,
		const FVector& Facing,
		TArray<FFormationSlot>& OutSlots
	);

private:
	int32 NextFormationID = 1;

	/** Map of FormationID to Actor pointer */
	UPROPERTY(Transient)
	TMap<int32, TObjectPtr<AFormationActor>> RegisteredFormations;
};
