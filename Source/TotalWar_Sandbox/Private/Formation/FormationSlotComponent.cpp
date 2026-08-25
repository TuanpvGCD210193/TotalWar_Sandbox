// Copyright Total War Sandbox. All Rights Reserved.

#include "TotalWar_Sandbox/Public/Formation/FormationSlotComponent.h"
#include "TotalWar_Sandbox/Public/Entity/SoldierEntity.h"

UFormationSlotComponent::UFormationSlotComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	CurrentFormationWidth = TWConstants::DefaultFormationWidth;
	MinFormationWidth = TWConstants::MinFormationWidth;
	MaxFormationWidth = TWConstants::MaxFormationWidth;
	SoldierSpacing = TWConstants::DefaultSoldierSpacing;
}

void UFormationSlotComponent::SetupFormationSettings(int32 InDefaultWidth, int32 InMinWidth, int32 InMaxWidth, float InSpacing)
{
	CurrentFormationWidth = InDefaultWidth;
	MinFormationWidth = InMinWidth;
	MaxFormationWidth = InMaxWidth;
	SoldierSpacing = InSpacing;
}

void UFormationSlotComponent::RebuildSlots(int32 AliveCount, const FVector& FormationCenter, const FVector& FacingDirection)
{
	FormationSlots.Empty(AliveCount);
	if (AliveCount <= 0)
	{
		return;
	}

	const int32 Columns = FMath::Clamp(CurrentFormationWidth, MinFormationWidth, MaxFormationWidth);
	const int32 Rows = FMath::CeilToInt(static_cast<float>(AliveCount) / static_cast<float>(Columns));

	const FRotator FacingRot = FacingDirection.GetSafeNormal2D().Rotation();
	int32 SoldiersPlaced = 0;

	for (int32 Row = 0; Row < Rows; ++Row)
	{
		const int32 SoldiersInThisRow = FMath::Min(Columns, AliveCount - SoldiersPlaced);
		if (SoldiersInThisRow <= 0)
		{
			break;
		}

		// Row 0 is front row at offset 0, subsequent rows step back along -X
		const float LocalX = -Row * SoldierSpacing;

		for (int32 Col = 0; Col < SoldiersInThisRow; ++Col)
		{
			// Center column horizontally around 0 (Y axis)
			const float LocalY = (Col - (SoldiersInThisRow - 1) * 0.5f) * SoldierSpacing;
			const FVector LocalOffset(LocalX, LocalY, 0.0f);

			// Transform to World Space
			const FVector WorldPos = FormationCenter + FacingRot.RotateVector(LocalOffset);

			FFormationSlot NewSlot(LocalOffset, Col, Row);
			NewSlot.WorldPosition = WorldPos;
			FormationSlots.Add(NewSlot);

			++SoldiersPlaced;
		}
	}
}

void UFormationSlotComponent::ReassignSoldiersToClosestSlots(TArray<FSoldierEntity>& Soldiers)
{
	// 1. Clear existing slot assignments
	for (FFormationSlot& Slot : FormationSlots)
	{
		Slot.Clear();
	}

	// 2. Greedy Nearest Slot Matching: Each soldier gets the closest unoccupied slot
	TArray<bool> SlotAssigned;
	SlotAssigned.Init(false, FormationSlots.Num());

	for (int32 SoldierIdx = 0; SoldierIdx < Soldiers.Num(); ++SoldierIdx)
	{
		FSoldierEntity& Soldier = Soldiers[SoldierIdx];
		if (!Soldier.IsAlive())
		{
			continue;
		}

		int32 BestSlotIdx = INDEX_NONE;
		float BestDistSq = TNumericLimits<float>::Max();

		for (int32 SlotIdx = 0; SlotIdx < FormationSlots.Num(); ++SlotIdx)
		{
			if (SlotAssigned[SlotIdx])
			{
				continue;
			}

			const float DistSq = FVector::DistSquared(Soldier.Position, FormationSlots[SlotIdx].WorldPosition);
			if (DistSq < BestDistSq)
			{
				BestDistSq = DistSq;
				BestSlotIdx = SlotIdx;
			}
		}

		if (BestSlotIdx != INDEX_NONE)
		{
			SlotAssigned[BestSlotIdx] = true;
			FormationSlots[BestSlotIdx].Assign(SoldierIdx);
			Soldier.AssignSlot(BestSlotIdx);
		}
	}
}

void UFormationSlotComponent::AssignSoldiersSequentially(TArray<FSoldierEntity>& Soldiers)
{
	for (int32 i = 0; i < FormationSlots.Num(); ++i)
	{
		if (i < Soldiers.Num())
		{
			FormationSlots[i].Assign(i);
			Soldiers[i].AssignSlot(i);
		}
		else
		{
			FormationSlots[i].Clear();
		}
	}
}

FVector UFormationSlotComponent::GetSlotWorldPosition(int32 SlotIndex) const
{
	if (FormationSlots.IsValidIndex(SlotIndex))
	{
		return FormationSlots[SlotIndex].WorldPosition;
	}
	return FVector::ZeroVector;
}
