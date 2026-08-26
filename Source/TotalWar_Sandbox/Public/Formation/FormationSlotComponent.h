// Copyright Total War Sandbox. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TotalWar_Sandbox/Public/Core/TWTypes.h"
#include "TotalWar_Sandbox/Public/Entity/FormationSlot.h"
#include "FormationSlotComponent.generated.h"

struct FSoldierEntity;

/**
 * UFormationSlotComponent — Bộ Quản Lý Vị Trí Đội Hình.
 * Chuyên trách: Tính toán hình học hàng cột, sinh ô slot, căn giữa, gán lính vào ô gần nhất.
 * Follows Single Responsibility Principle (SRP).
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TOTALWAR_SANDBOX_API UFormationSlotComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UFormationSlotComponent();

	// ========================================================================
	// PUBLIC API
	// ========================================================================

	/** Initializes formation slot settings */
	void SetupFormationSettings(int32 InDefaultWidth, int32 InMinWidth, int32 InMaxWidth, float InSpacing);

	/** Rebuilds slot grid based on current alive soldier count and anchor position */
	void RebuildSlots(int32 AliveCount, const FVector& FormationCenter, const FVector& FacingDirection);

	/**
	 * Total War Rank & File Projection Sorting Algorithm:
	 * Projects soldiers onto Forward and Right axes, then assigns soldiers row-by-row and col-by-col
	 * to destination slots. Guarantees parallel non-crossing paths (O(N log N)).
	 */
	void AssignSlotsByRankAndFileProjection(
		TArray<FSoldierEntity>& Soldiers,
		const FVector& CurrentCenter,
		const FVector& CurrentFacing
	);

	/**
	 * Assigns alive soldiers to the nearest available formation slots (Greedy matching).
	 * Prevents soldiers from criss-crossing when formation shape changes.
	 */
	void ReassignSoldiersToClosestSlots(TArray<FSoldierEntity>& Soldiers);

	/**
	 * Calculates the physical ground bounding box of the formation:
	 * - Idle: Rigid geometric bounds with proper depth center offset.
	 * - March: Realtime Oriented Bounding Box enclosing living soldiers.
	 */
	void CalculateFormationGroundBounds(
		const TArray<FSoldierEntity>& Soldiers,
		bool bIsMoving,
		EFormationState State,
		const FVector& ActorLoc,
		const FVector& Facing,
		FVector& OutCenter,
		FVector& OutExtents,
		FRotator& OutRotation
	) const;

	/** Direct sequential slot assignment (0 -> 0, 1 -> 1) */
	void AssignSoldiersSequentially(TArray<FSoldierEntity>& Soldiers);

	// ========================================================================
	// GETTERS & SETTERS (Encapsulation)
	// ========================================================================

	FORCEINLINE const TArray<FFormationSlot>& GetSlots() const { return FormationSlots; }
	FORCEINLINE TArray<FFormationSlot>& GetSlotsMutable() { return FormationSlots; }

	FORCEINLINE int32 GetFormationWidth() const { return CurrentFormationWidth; }
	FORCEINLINE void SetFormationWidth(int32 NewWidth) { CurrentFormationWidth = FMath::Clamp(NewWidth, MinFormationWidth, MaxFormationWidth); }

	FORCEINLINE float GetSoldierSpacing() const { return SoldierSpacing; }
	FORCEINLINE void SetSoldierSpacing(float NewSpacing) { SoldierSpacing = FMath::Max(10.0f, NewSpacing); }

	FORCEINLINE int32 GetSlotCount() const { return FormationSlots.Num(); }

	/** Retrieves slot world position by index */
	FVector GetSlotWorldPosition(int32 SlotIndex) const;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Formation|Config")
	int32 CurrentFormationWidth;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Formation|Config")
	int32 MinFormationWidth;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Formation|Config")
	int32 MaxFormationWidth;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Formation|Config")
	float SoldierSpacing;

private:
	/** Grid of geometric formation slots */
	TArray<FFormationSlot> FormationSlots;
};
