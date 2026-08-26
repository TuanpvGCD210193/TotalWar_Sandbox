// Copyright Total War Sandbox. All Rights Reserved.

#include "TotalWar_Sandbox/Public/Player/TWSelectionComponent.h"
#include "TotalWar_Sandbox/Public/Formation/FormationActor.h"
#include "TotalWar_Sandbox/Public/Systems/FormationSubsystem.h"
#include "GameFramework/PlayerController.h"

UTWSelectionComponent::UTWSelectionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	bLeftMouseDown = false;
	bIsDraggingSelection = false;
	SelectionStartScreenPos = FVector2D::ZeroVector;
	SelectionCurrentScreenPos = FVector2D::ZeroVector;
}

APlayerController* UTWSelectionComponent::GetOwningPlayerController() const
{
	return Cast<APlayerController>(GetOwner());
}

void UTWSelectionComponent::PollSelectionInput()
{
	APlayerController* PC = GetOwningPlayerController();
	if (!PC)
	{
		return;
	}

	FVector2D CurrentMousePos;
	PC->GetMousePosition(CurrentMousePos.X, CurrentMousePos.Y);

	const bool bIsLeftDownNow = PC->IsInputKeyDown(EKeys::LeftMouseButton);

	// 1. Mouse Button Pressed
	if (bIsLeftDownNow && !bLeftMouseDown)
	{
		bLeftMouseDown = true;
		bIsDraggingSelection = false;
		SelectionStartScreenPos = CurrentMousePos;
		SelectionCurrentScreenPos = CurrentMousePos;
	}
	// 2. Mouse Button Held
	else if (bIsLeftDownNow && bLeftMouseDown)
	{
		SelectionCurrentScreenPos = CurrentMousePos;
		const float DragDistance = FVector2D::Distance(SelectionStartScreenPos, SelectionCurrentScreenPos);
		if (DragDistance > TWConstants::SelectionDragThreshold)
		{
			bIsDraggingSelection = true;
		}
	}
	// 3. Mouse Button Released — Unified Selection Execution (DRY Principle)
	else if (!bIsLeftDownNow && bLeftMouseDown)
	{
		const bool bShiftHeld = PC->IsInputKeyDown(EKeys::LeftShift) || PC->IsInputKeyDown(EKeys::RightShift);

		if (!bShiftHeld)
		{
			DeselectAll();
		}

		if (bIsDraggingSelection)
		{
			SelectFormationsInScreenBox(SelectionStartScreenPos, SelectionCurrentScreenPos, bShiftHeld, false);
		}
		else
		{
			const float ClickTolerance = 4.0f;
			SelectFormationsInScreenBox(
				CurrentMousePos - FVector2D(ClickTolerance, ClickTolerance),
				CurrentMousePos + FVector2D(ClickTolerance, ClickTolerance),
				bShiftHeld,
				true
			);
		}

		bLeftMouseDown = false;
		bIsDraggingSelection = false;
	}
}

void UTWSelectionComponent::SelectFormation(AFormationActor* Formation, bool bAdditive)
{
	if (!Formation || Formation->GetTeamID() != ETeamID::Blue)
	{
		return;
	}

	if (!bAdditive)
	{
		DeselectAll();
	}

	Formation->SetSelected(true);
	SelectedFormations.AddUnique(Formation);
}

void UTWSelectionComponent::DeselectAll()
{
	for (AFormationActor* Formation : SelectedFormations)
	{
		if (Formation)
		{
			Formation->SetSelected(false);
		}
	}
	SelectedFormations.Empty();
}

void UTWSelectionComponent::SelectFormationsInScreenBox(
	const FVector2D& ScreenStart,
	const FVector2D& ScreenEnd,
	bool bShiftHeld,
	bool bIsSingleClick)
{
	APlayerController* PC = GetOwningPlayerController();
	UWorld* World = GetWorld();
	if (!PC || !World)
	{
		return;
	}

	UFormationSubsystem* FormationSubsystem = World->GetSubsystem<UFormationSubsystem>();
	if (!FormationSubsystem)
	{
		return;
	}

	const FBox2D SelectionBox(
		FVector2D(FMath::Min(ScreenStart.X, ScreenEnd.X), FMath::Min(ScreenStart.Y, ScreenEnd.Y)),
		FVector2D(FMath::Max(ScreenStart.X, ScreenEnd.X), FMath::Max(ScreenStart.Y, ScreenEnd.Y))
	);

	TArray<AFormationActor*> PlayerFormations;
	FormationSubsystem->GetFormationsByTeam(ETeamID::Blue, PlayerFormations);

	for (AFormationActor* Formation : PlayerFormations)
	{
		if (!Formation || Formation->GetAliveCount() <= 0)
		{
			continue;
		}

		FVector Center, Extents;
		FRotator Rotation;
		Formation->GetFormationGroundBounds(Center, Extents, Rotation);

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
			const FBox2D FormationScreenBox(FVector2D(MinX, MinY), FVector2D(MaxX, MaxY));

			if (SelectionBox.Intersect(FormationScreenBox))
			{
				// Shift + Single Click on already selected formation toggles/deselects it!
				if (bShiftHeld && bIsSingleClick && SelectedFormations.Contains(Formation))
				{
					Formation->SetSelected(false);
					SelectedFormations.Remove(Formation);
				}
				else
				{
					Formation->SetSelected(true);
					SelectedFormations.AddUnique(Formation);
				}
			}
		}
	}
}
