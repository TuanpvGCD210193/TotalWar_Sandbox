// Copyright Total War Sandbox. All Rights Reserved.

#include "TotalWar_Sandbox/Public/Player/TWCommandComponent.h"
#include "TotalWar_Sandbox/Public/Formation/FormationActor.h"
#include "TotalWar_Sandbox/Public/Formation/FormationGroupPlacement.h"
#include "TotalWar_Sandbox/Public/Systems/FormationSubsystem.h"
#include "GameFramework/PlayerController.h"

UTWCommandComponent::UTWCommandComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	bIsSpaceHeld = false;
	bRightMouseDown = false;
	bIsDraggingPlacement = false;
	RightClickStartPos = FVector2D::ZeroVector;
	RightDragStartGroundWorldPos = FVector::ZeroVector;
	RightDragCurrentGroundWorldPos = FVector::ZeroVector;
	PlacementCenter = FVector::ZeroVector;
	PlacementFacing = FVector::ForwardVector;
	PlacementWidth = TWConstants::DefaultFormationWidth;
}

APlayerController* UTWCommandComponent::GetOwningPlayerController() const
{
	return Cast<APlayerController>(GetOwner());
}

void UTWCommandComponent::PollCommandInput(const TArray<TObjectPtr<AFormationActor>>& SelectedFormations)
{
	APlayerController* PC = GetOwningPlayerController();
	if (!PC)
	{
		return;
	}

	bIsSpaceHeld = PC->IsInputKeyDown(EKeys::SpaceBar);

	FVector2D CurrentMousePos;
	PC->GetMousePosition(CurrentMousePos.X, CurrentMousePos.Y);

	const bool bIsRightDownNow = PC->IsInputKeyDown(EKeys::RightMouseButton);

	FHitResult HitResult;
	PC->GetHitResultUnderCursor(ECC_Visibility, false, HitResult);

	// 1. Right Mouse Pressed
	if (bIsRightDownNow && !bRightMouseDown)
	{
		bRightMouseDown = true;
		bIsDraggingPlacement = false;
		RightClickStartPos = CurrentMousePos;

		if (HitResult.bBlockingHit)
		{
			RightDragStartGroundWorldPos = HitResult.ImpactPoint;
			RightDragCurrentGroundWorldPos = HitResult.ImpactPoint;
			PlacementCenter = HitResult.ImpactPoint;

			if (SelectedFormations.IsValidIndex(0) && SelectedFormations[0])
			{
				PlacementFacing = SelectedFormations[0]->GetFacingDirection();
				PlacementWidth = SelectedFormations[0]->GetFormationWidth();
			}
			else
			{
				PlacementFacing = FVector::ForwardVector;
				PlacementWidth = TWConstants::DefaultFormationWidth;
			}
		}
	}
	// 2. Right Mouse Held (Drag Placement State)
	else if (bIsRightDownNow && bRightMouseDown && SelectedFormations.Num() > 0)
	{
		const float ScreenDragDist = FVector2D::Distance(RightClickStartPos, CurrentMousePos);
		if (ScreenDragDist > TWConstants::PlacementDragThreshold && HitResult.bBlockingHit)
		{
			bIsDraggingPlacement = true;
			RightDragCurrentGroundWorldPos = HitResult.ImpactPoint;

			const FVector DragVector = RightDragCurrentGroundWorldPos - RightDragStartGroundWorldPos;
			const float DragLength = DragVector.Size2D();

			float Spacing = TWConstants::DefaultSoldierSpacing;
			if (SelectedFormations.IsValidIndex(0) && SelectedFormations[0])
			{
				Spacing = SelectedFormations[0]->GetSoldierSpacing();
			}

			const int32 DesiredColumns = FMath::Max(1, FMath::RoundToInt(DragLength / Spacing));
			PlacementWidth = FMath::Clamp(DesiredColumns, TWConstants::MinFormationWidth, TWConstants::MaxFormationWidth);

			PlacementCenter = (RightDragStartGroundWorldPos + RightDragCurrentGroundWorldPos) * 0.5f;

			if (!DragVector.IsNearlyZero())
			{
				const FVector DragDir = DragVector.GetSafeNormal2D();
				PlacementFacing = FVector(DragDir.Y, -DragDir.X, 0.0f).GetSafeNormal2D();
			}
		}
	}
	// 3. Right Mouse Released — Execute Commands
	else if (!bIsRightDownNow && bRightMouseDown)
	{
		if (SelectedFormations.Num() > 0)
		{
			// 1. Check Enemy Red Formation Click or Drag (Attack Order has highest priority)
			AFormationActor* ClickedEnemy = GetEnemyFormationUnderCursor();
			if (ClickedEnemy)
			{
				FFormationCommand AttackCmd = FFormationCommand::CreateAttackCommand(ClickedEnemy);
				for (AFormationActor* Formation : SelectedFormations)
				{
					if (Formation && Formation->GetAliveCount() > 0)
					{
						Formation->ExecuteCommand(AttackCmd);
					}
				}
			}
			else if (bIsDraggingPlacement)
			{
				const FVector DragVector = RightDragCurrentGroundWorldPos - RightDragStartGroundWorldPos;
				const float TotalDragLength = DragVector.Size2D();

				TMap<AFormationActor*, FFormationCommand> GroupCmds;
				UFormationGroupPlacement::CalculateGroupPlacementCommands(
					SelectedFormations,
					PlacementCenter,
					PlacementFacing,
					TotalDragLength,
					GroupCmds
				);

				for (const auto& Pair : GroupCmds)
				{
					if (Pair.Key)
					{
						Pair.Key->ExecuteCommand(Pair.Value);
					}
				}
			}
			else if (HitResult.bBlockingHit)
			{
				// Ground Click Quick Move
				const FVector TargetGroundPos = HitResult.ImpactPoint;
				FVector FacingDir = PlacementFacing;

				if (SelectedFormations.IsValidIndex(0) && SelectedFormations[0])
				{
					FacingDir = (TargetGroundPos - SelectedFormations[0]->GetActorLocation()).GetSafeNormal2D();
					if (FacingDir.IsNearlyZero())
					{
						FacingDir = SelectedFormations[0]->GetFacingDirection();
					}
				}

				TMap<AFormationActor*, FFormationCommand> QuickMoveCmds;
				UFormationGroupPlacement::CalculateGroupQuickMoveCommands(
					SelectedFormations,
					TargetGroundPos,
					FacingDir,
					QuickMoveCmds
				);

				for (const auto& Pair : QuickMoveCmds)
				{
					if (Pair.Key)
					{
						Pair.Key->ExecuteCommand(Pair.Value);
					}
				}
			}
		}

		bRightMouseDown = false;
		bIsDraggingPlacement = false;
	}
}

AFormationActor* UTWCommandComponent::GetEnemyFormationUnderCursor() const
{
	APlayerController* PC = GetOwningPlayerController();
	UWorld* World = GetWorld();
	if (!PC || !World)
	{
		return nullptr;
	}

	UFormationSubsystem* FormationSubsystem = World->GetSubsystem<UFormationSubsystem>();
	if (!FormationSubsystem)
	{
		return nullptr;
	}

	FVector2D MousePos;
	PC->GetMousePosition(MousePos.X, MousePos.Y);
	const FBox2D CursorBox(MousePos - FVector2D(6.0f, 6.0f), MousePos + FVector2D(6.0f, 6.0f));

	TArray<AFormationActor*> EnemyFormations;
	FormationSubsystem->GetFormationsByTeam(ETeamID::Red, EnemyFormations);

	for (AFormationActor* EnemyFormation : EnemyFormations)
	{
		if (!EnemyFormation || EnemyFormation->GetAliveCount() <= 0)
		{
			continue;
		}

		FVector Center, Extents;
		FRotator Rotation;
		EnemyFormation->GetFormationGroundBounds(Center, Extents, Rotation);

		const FVector ForwardVec = Rotation.Vector() * Extents.X;
		const FVector RightVec = FRotationMatrix(Rotation).GetUnitAxis(EAxis::Y) * Extents.Y;

		const FVector Corners[4] = {
			Center + ForwardVec + RightVec,
			Center + ForwardVec - RightVec,
			Center - ForwardVec - RightVec,
			Center - ForwardVec + RightVec
		};

		float MinX = TNumericLimits<float>::Max(), MaxX = -TNumericLimits<float>::Max();
		float MinY = TNumericLimits<float>::Max(), MaxY = -TNumericLimits<float>::Max();
		bool bAnyProjected = false;

		for (int32 i = 0; i < 4; ++i)
		{
			FVector2D ScreenCorner;
			if (PC->ProjectWorldLocationToScreen(Corners[i], ScreenCorner))
			{
				MinX = FMath::Min(MinX, ScreenCorner.X);
				MaxX = FMath::Max(MaxX, ScreenCorner.X);
				MinY = FMath::Min(MinY, ScreenCorner.Y);
				MaxY = FMath::Max(MaxY, ScreenCorner.Y);
				bAnyProjected = true;
			}
		}

		if (bAnyProjected)
		{
			const FBox2D EnemyScreenBox(FVector2D(MinX, MinY), FVector2D(MaxX, MaxY));
			if (CursorBox.Intersect(EnemyScreenBox))
			{
				return EnemyFormation;
			}
		}
	}

	return nullptr;
}
