// Copyright Total War Sandbox. All Rights Reserved.

#include "TotalWar_Sandbox/Public/Formation/FormationSlotComponent.h"
#include "TotalWar_Sandbox/Public/Entity/SoldierEntity.h"
#include "TotalWar_Sandbox/Public/Systems/FormationSubsystem.h"
#include "Algo/Sort.h"

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

void UFormationSlotComponent::AssignSlotsByRankAndFileProjection(
	TArray<FSoldierEntity>& Soldiers,
	const FVector& CurrentCenter,
	const FVector& CurrentFacing)
{
	const int32 Count = Soldiers.Num();
	if (Count <= 0 || FormationSlots.Num() <= 0)
	{
		return;
	}

	const FVector ForwardVec = CurrentFacing.GetSafeNormal2D();
	const FVector RightVec = FRotationMatrix(ForwardVec.Rotation()).GetUnitAxis(EAxis::Y);

	// 1. Structure to hold soldier index and its projected coordinates
	struct FSoldierProjectedScore
	{
		int32 SoldierIndex;
		float ForwardScore; // Higher = front
		float RightScore;   // Higher = right
	};

	TArray<FSoldierProjectedScore> ProjectedScores;
	ProjectedScores.Reserve(Count);

	for (int32 i = 0; i < Count; ++i)
	{
		const FSoldierEntity& Soldier = Soldiers[i];
		if (!Soldier.IsAlive())
		{
			continue;
		}

		const FVector RelPos = Soldier.Position - CurrentCenter;
		const float FScore = FVector::DotProduct(RelPos, ForwardVec);
		const float RScore = FVector::DotProduct(RelPos, RightVec);

		ProjectedScores.Add({ i, FScore, RScore });
	}

	const int32 AliveCount = ProjectedScores.Num();
	if (AliveCount == 0)
	{
		return;
	}

	// 2. Sort all soldiers by ForwardScore descending (Front rows first)
	ProjectedScores.Sort([](const FSoldierProjectedScore& A, const FSoldierProjectedScore& B)
	{
		return A.ForwardScore > B.ForwardScore;
	});

	// 3. For each rank/row, sort soldiers by RightScore ascending (Left to Right)
	const int32 Columns = FMath::Clamp(CurrentFormationWidth, MinFormationWidth, MaxFormationWidth);
	const int32 Rows = FMath::CeilToInt(static_cast<float>(AliveCount) / static_cast<float>(Columns));

	// Clear slot assignments
	for (FFormationSlot& Slot : FormationSlots)
	{
		Slot.Clear();
	}

	int32 ProcessedCount = 0;
	int32 SlotIndex = 0;

	for (int32 Row = 0; Row < Rows; ++Row)
	{
		const int32 SoldiersInThisRow = FMath::Min(Columns, AliveCount - ProcessedCount);
		if (SoldiersInThisRow <= 0)
		{
			break;
		}

		// Sort this row slice by RightScore ascending (Left to Right)
		Algo::Sort(MakeArrayView(ProjectedScores.GetData() + ProcessedCount, SoldiersInThisRow), [](const FSoldierProjectedScore& A, const FSoldierProjectedScore& B)
		{
			return A.RightScore < B.RightScore;
		});

		// Assign to slots sequentially for this row
		for (int32 Col = 0; Col < SoldiersInThisRow; ++Col)
		{
			if (FormationSlots.IsValidIndex(SlotIndex))
			{
				const int32 SoldierIdx = ProjectedScores[ProcessedCount + Col].SoldierIndex;
				FormationSlots[SlotIndex].Assign(SoldierIdx);
				Soldiers[SoldierIdx].AssignSlot(SlotIndex);
				++SlotIndex;
			}
		}

		ProcessedCount += SoldiersInThisRow;
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

void UFormationSlotComponent::CalculateFormationGroundBounds(
	const TArray<FSoldierEntity>& Soldiers,
	bool bIsMoving,
	EFormationState State,
	const FVector& ActorLoc,
	const FVector& Facing,
	FVector& OutCenter,
	FVector& OutExtents,
	FRotator& OutRotation) const
{
	const int32 Count = Soldiers.Num();
	if (Count == 0)
	{
		OutCenter = ActorLoc;
		OutExtents = FVector(100.0f, 100.0f, 20.0f);
		OutRotation = Facing.Rotation();
		return;
	}

	const float Spacing = SoldierSpacing;

	// 1. In Idle / Static state -> Rigid geometric formation bounds
	if (!bIsMoving && State == EFormationState::Idle)
	{
		OutRotation = Facing.Rotation();

		const int32 Columns = FMath::Clamp(CurrentFormationWidth, MinFormationWidth, MaxFormationWidth);
		const int32 Rows = FMath::Max(1, FMath::CeilToInt(static_cast<float>(Count) / static_cast<float>(Columns)));

		const float HalfWidth = (Columns * Spacing) * 0.5f + Spacing * 0.25f;
		const float HalfDepth = (Rows * Spacing) * 0.5f + Spacing * 0.25f;

		const float CenterOffsetForward = -((Rows - 1) * Spacing) * 0.5f;
		OutCenter = ActorLoc + Facing * CenterOffsetForward;
		OutExtents = FVector(HalfDepth, HalfWidth, 20.0f);
		return;
	}

	// 2. In March / Dynamic state -> Realtime Oriented Bounding Box enclosing living soldiers
	FVector Centroid = FVector::ZeroVector;
	FVector ForwardSum = FVector::ZeroVector;
	int32 AliveCount = 0;

	for (const FSoldierEntity& Soldier : Soldiers)
	{
		if (Soldier.IsAlive())
		{
			Centroid += Soldier.Position;
			ForwardSum += Soldier.Rotation.Vector();
			++AliveCount;
		}
	}

	if (AliveCount == 0)
	{
		OutCenter = ActorLoc;
		OutExtents = FVector(100.0f, 100.0f, 20.0f);
		OutRotation = Facing.Rotation();
		return;
	}

	Centroid /= static_cast<float>(AliveCount);

	FVector ForwardVec = ForwardSum.GetSafeNormal2D();
	if (ForwardVec.IsNearlyZero())
	{
		ForwardVec = Facing;
	}
	const FVector RightVec = FRotationMatrix(ForwardVec.Rotation()).GetUnitAxis(EAxis::Y);

	float MinF = TNumericLimits<float>::Max(), MaxF = -TNumericLimits<float>::Max();
	float MinR = TNumericLimits<float>::Max(), MaxR = -TNumericLimits<float>::Max();

	for (const FSoldierEntity& Soldier : Soldiers)
	{
		if (Soldier.IsAlive())
		{
			const FVector Rel = Soldier.Position - Centroid;
			const float F = FVector::DotProduct(Rel, ForwardVec);
			const float R = FVector::DotProduct(Rel, RightVec);

			MinF = FMath::Min(MinF, F); MaxF = FMath::Max(MaxF, F);
			MinR = FMath::Min(MinR, R); MaxR = FMath::Max(MaxR, R);
		}
	}

	const float Margin = Spacing * 0.6f;
	const float CenterF = (MinF + MaxF) * 0.5f;
	const float CenterR = (MinR + MaxR) * 0.5f;

	OutCenter = Centroid + ForwardVec * CenterF + RightVec * CenterR;
	OutRotation = ForwardVec.Rotation();
	OutExtents = FVector(
		FMath::Max(Spacing, (MaxF - MinF) * 0.5f + Margin),
		FMath::Max(Spacing, (MaxR - MinR) * 0.5f + Margin),
		20.0f
	);
}
