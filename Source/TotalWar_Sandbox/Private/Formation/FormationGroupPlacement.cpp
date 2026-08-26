// Copyright Total War Sandbox. All Rights Reserved.

#include "TotalWar_Sandbox/Public/Formation/FormationGroupPlacement.h"
#include "TotalWar_Sandbox/Public/Formation/FormationActor.h"

void UFormationGroupPlacement::SortFormationsLeftToRight(
	const TArray<TObjectPtr<AFormationActor>>& InFormations,
	const FVector& FacingDirection,
	TArray<AFormationActor*>& OutSortedFormations)
{
	OutSortedFormations.Empty();

	for (const TObjectPtr<AFormationActor>& Formation : InFormations)
	{
		if (Formation && Formation->GetAliveCount() > 0)
		{
			OutSortedFormations.Add(Formation.Get());
		}
	}

	if (OutSortedFormations.Num() <= 1)
	{
		return;
	}

	// Calculate Right Vector perpendicular to facing
	const FVector RightVec = FRotationMatrix(FacingDirection.GetSafeNormal2D().Rotation()).GetUnitAxis(EAxis::Y);

	// Sort ascending by projection along Right Vector (Leftmost squad has smallest dot product)
	OutSortedFormations.Sort([&RightVec](const AFormationActor& A, const AFormationActor& B)
	{
		const float DotA = FVector::DotProduct(A.GetActorLocation(), RightVec);
		const float DotB = FVector::DotProduct(B.GetActorLocation(), RightVec);
		return DotA < DotB;
	});
}

void UFormationGroupPlacement::CalculateGroupPlacementCommands(
	const TArray<TObjectPtr<AFormationActor>>& Formations,
	const FVector& GroupCenter,
	const FVector& FacingDirection,
	float TotalDragLength,
	TMap<AFormationActor*, FFormationCommand>& OutCommands)
{
	OutCommands.Empty();

	TArray<AFormationActor*> SortedFormations;
	SortFormationsLeftToRight(Formations, FacingDirection, SortedFormations);

	const int32 NumSquads = SortedFormations.Num();
	if (NumSquads == 0)
	{
		return;
	}

	const FVector FacingNorm = FacingDirection.GetSafeNormal2D();
	const FVector RightVec = FRotationMatrix(FacingNorm.Rotation()).GetUnitAxis(EAxis::Y);

	// Single Squad Case
	if (NumSquads == 1)
	{
		AFormationActor* SingleFormation = SortedFormations[0];
		const float Spacing = SingleFormation->GetSoldierSpacing();
		const int32 Columns = FMath::Clamp(
			FMath::RoundToInt(TotalDragLength / Spacing),
			TWConstants::MinFormationWidth,
			TWConstants::MaxFormationWidth
		);

		OutCommands.Add(SingleFormation, FFormationCommand::CreatePlacementCommand(GroupCenter, FacingNorm, Columns));
		return;
	}

	// Multi-Squad Battleline Case (Total War Style)
	const float TotalGapsWidth = (NumSquads - 1) * DefaultSquadGap;
	const float AvailableLengthForSquads = FMath::Max(100.0f * NumSquads, TotalDragLength - TotalGapsWidth);
	const float LengthPerSquad = AvailableLengthForSquads / NumSquads;

	// Calculate column counts and physical widths for each squad
	TArray<int32> SquadColumns;
	TArray<float> SquadPhysicalWidths;
	float TotalBattlelineWidth = 0.0f;

	for (int32 i = 0; i < NumSquads; ++i)
	{
		AFormationActor* Squad = SortedFormations[i];
		const float Spacing = Squad->GetSoldierSpacing();
		const int32 DesiredCols = FMath::Clamp(
			FMath::RoundToInt(LengthPerSquad / Spacing),
			TWConstants::MinFormationWidth,
			TWConstants::MaxFormationWidth
		);
		const float PhysicalWidth = DesiredCols * Spacing;

		SquadColumns.Add(DesiredCols);
		SquadPhysicalWidths.Add(PhysicalWidth);
		TotalBattlelineWidth += PhysicalWidth;
	}

	TotalBattlelineWidth += TotalGapsWidth;

	// Distribute squads from Left to Right along the Battleline
	float CurrentOffsetFromLeft = -TotalBattlelineWidth * 0.5f;

	for (int32 i = 0; i < NumSquads; ++i)
	{
		AFormationActor* Squad = SortedFormations[i];
		const float HalfSquadWidth = SquadPhysicalWidths[i] * 0.5f;
		const float SquadCenterOffset = CurrentOffsetFromLeft + HalfSquadWidth;

		const FVector SquadTargetLocation = GroupCenter + RightVec * SquadCenterOffset;

		FFormationCommand SquadCmd = FFormationCommand::CreatePlacementCommand(
			SquadTargetLocation,
			FacingNorm,
			SquadColumns[i]
		);

		OutCommands.Add(Squad, SquadCmd);

		// Advance offset for the next squad in the battleline
		CurrentOffsetFromLeft += SquadPhysicalWidths[i] + DefaultSquadGap;
	}
}

void UFormationGroupPlacement::CalculateGroupQuickMoveCommands(
	const TArray<TObjectPtr<AFormationActor>>& Formations,
	const FVector& TargetDestination,
	const FVector& FacingDirection,
	TMap<AFormationActor*, FFormationCommand>& OutCommands)
{
	OutCommands.Empty();

	TArray<AFormationActor*> ActiveFormations;
	for (const TObjectPtr<AFormationActor>& Formation : Formations)
	{
		if (Formation && Formation->GetAliveCount() > 0)
		{
			ActiveFormations.Add(Formation.Get());
		}
	}

	const int32 NumSquads = ActiveFormations.Num();
	if (NumSquads == 0)
	{
		return;
	}

	const FVector FacingNorm = FacingDirection.GetSafeNormal2D();

	// Single squad case
	if (NumSquads == 1)
	{
		AFormationActor* Squad = ActiveFormations[0];
		OutCommands.Add(Squad, FFormationCommand::CreateMoveCommand(TargetDestination, FacingNorm));
		return;
	}

	// Multi-Squad Case: Calculate Group Centroid and maintain relative tactical offsets
	FVector GroupCentroid = FVector::ZeroVector;
	for (AFormationActor* Squad : ActiveFormations)
	{
		GroupCentroid += Squad->GetActorLocation();
	}
	GroupCentroid /= static_cast<float>(NumSquads);

	const FRotator NewFacingRot = FacingNorm.Rotation();
	const FRotator OldGroupFacingRot = (ActiveFormations[0]->GetFacingDirection()).Rotation();
	const FRotator DeltaRot = NewFacingRot - OldGroupFacingRot;

	for (AFormationActor* Squad : ActiveFormations)
	{
		// Relative offset vector from old centroid
		const FVector RelativeOffset = Squad->GetActorLocation() - GroupCentroid;

		// Rotate offset to match new movement facing
		const FVector RotatedOffset = DeltaRot.RotateVector(RelativeOffset);

		const FVector SquadTargetLocation = TargetDestination + RotatedOffset;

		FFormationCommand SquadCmd = FFormationCommand::CreateMoveCommand(
			SquadTargetLocation,
			FacingNorm,
			Squad->GetFormationWidth()
		);

		OutCommands.Add(Squad, SquadCmd);
	}
}
