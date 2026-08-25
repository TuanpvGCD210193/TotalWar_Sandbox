// Copyright Total War Sandbox. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SoldierHandle.generated.h"

/**
 * Lightweight, safe reference handle to a specific soldier in a formation.
 * Size: 8 bytes.
 * Avoids holding raw pointers across frames (safe against array compaction / swap-remove).
 * Follows Single Responsibility Principle (SRP).
 */
USTRUCT(BlueprintType)
struct TOTALWAR_SANDBOX_API FSoldierHandle
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Entity")
	int32 FormationID = INDEX_NONE;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Entity")
	int32 SoldierIndex = INDEX_NONE;

	FSoldierHandle()
		: FormationID(INDEX_NONE)
		, SoldierIndex(INDEX_NONE)
	{
	}

	FSoldierHandle(int32 InFormationID, int32 InSoldierIndex)
		: FormationID(InFormationID)
		, SoldierIndex(InSoldierIndex)
	{
	}

	/** Returns true if this handle points to a potentially valid entity */
	FORCEINLINE bool IsValid() const
	{
		return FormationID != INDEX_NONE && SoldierIndex != INDEX_NONE;
	}

	/** Resets the handle to an invalid state */
	FORCEINLINE void Invalidate()
	{
		FormationID = INDEX_NONE;
		SoldierIndex = INDEX_NONE;
	}

	FORCEINLINE bool operator==(const FSoldierHandle& Other) const
	{
		return FormationID == Other.FormationID && SoldierIndex == Other.SoldierIndex;
	}

	FORCEINLINE bool operator!=(const FSoldierHandle& Other) const
	{
		return !(*this == Other);
	}

	friend FORCEINLINE uint32 GetTypeHash(const FSoldierHandle& Handle)
	{
		return HashCombine(GetTypeHash(Handle.FormationID), GetTypeHash(Handle.SoldierIndex));
	}
};
