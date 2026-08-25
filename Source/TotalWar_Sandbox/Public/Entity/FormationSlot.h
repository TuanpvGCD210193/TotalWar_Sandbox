// Copyright Total War Sandbox. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "FormationSlot.generated.h"

/**
 * Represents a single geometric slot inside a formation grid.
 * Follows Single Responsibility Principle (SRP): Only manages geometric slot data.
 */
USTRUCT(BlueprintType)
struct TOTALWAR_SANDBOX_API FFormationSlot
{
	GENERATED_BODY()

public:
	/** Local offset relative to the formation anchor/center (X = Forward, Y = Right, Z = Up) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation")
	FVector LocalOffset = FVector::ZeroVector;

	/** Calculated world space position of this slot */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation")
	FVector WorldPosition = FVector::ZeroVector;

	/** Index of the soldier currently assigned to this slot (INDEX_NONE if unoccupied) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation")
	int32 AssignedSoldierIndex = INDEX_NONE;

	/** Grid column index (0 = leftmost) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation")
	int32 ColumnIndex = 0;

	/** Grid rank/row index (0 = front row) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation")
	int32 RankIndex = 0;

	/** Whether this slot is actively occupied by a living soldier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation")
	bool bOccupied = false;

	FFormationSlot()
		: LocalOffset(FVector::ZeroVector)
		, WorldPosition(FVector::ZeroVector)
		, AssignedSoldierIndex(INDEX_NONE)
		, ColumnIndex(0)
		, RankIndex(0)
		, bOccupied(false)
	{
	}

	FFormationSlot(const FVector& InLocalOffset, int32 InColumn, int32 InRank)
		: LocalOffset(InLocalOffset)
		, WorldPosition(FVector::ZeroVector)
		, AssignedSoldierIndex(INDEX_NONE)
		, ColumnIndex(InColumn)
		, RankIndex(InRank)
		, bOccupied(false)
	{
	}

	/** Assigns a living soldier to this slot */
	FORCEINLINE void Assign(int32 InSoldierIndex)
	{
		AssignedSoldierIndex = InSoldierIndex;
		bOccupied = (InSoldierIndex != INDEX_NONE);
	}

	/** Clears slot assignment */
	FORCEINLINE void Clear()
	{
		AssignedSoldierIndex = INDEX_NONE;
		bOccupied = false;
	}
};
