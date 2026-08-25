// Copyright Total War Sandbox. All Rights Reserved.

#include "TotalWar_Sandbox/Public/Player/TWPlayerController.h"
#include "TotalWar_Sandbox/Public/Player/TWCameraPawn.h"
#include "TotalWar_Sandbox/Public/Formation/FormationActor.h"
#include "TotalWar_Sandbox/Public/Systems/FormationSubsystem.h"
#include "Engine/World.h"

ATWPlayerController::ATWPlayerController()
{
	bShowMouseCursor = true;
	bEnableClickEvents = true;
	bEnableMouseOverEvents = true;
	DefaultMouseCursor = EMouseCursor::Default;

	bLeftMouseDown = false;
	bIsDraggingSelection = false;
	bMiddleMouseDown = false;
	SelectionStartScreenPos = FVector2D::ZeroVector;
	SelectionCurrentScreenPos = FVector2D::ZeroVector;
	LastMousePosition = FVector2D::ZeroVector;
}

void ATWPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// Set input mode to Game and UI so mouse isn't captured invisibly
	FInputModeGameAndUI InputMode;
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	InputMode.SetHideCursorDuringCapture(false);
	SetInputMode(InputMode);
}

void ATWPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);

	PollCameraInput(DeltaTime);
	PollSelectionInput();
}

void ATWPlayerController::PollCameraInput(float DeltaTime)
{
	ATWCameraPawn* CameraPawn = GetControlledCameraPawn();
	if (!CameraPawn)
	{
		return;
	}

	// 1. Planar Pan Input (WASD / Arrow Keys)
	float ForwardInput = 0.0f;
	float RightInput = 0.0f;

	if (IsInputKeyDown(EKeys::W) || IsInputKeyDown(EKeys::Up))    { ForwardInput += 1.0f; }
	if (IsInputKeyDown(EKeys::S) || IsInputKeyDown(EKeys::Down))  { ForwardInput -= 1.0f; }
	if (IsInputKeyDown(EKeys::D) || IsInputKeyDown(EKeys::Right)) { RightInput += 1.0f; }
	if (IsInputKeyDown(EKeys::A) || IsInputKeyDown(EKeys::Left))  { RightInput -= 1.0f; }

	if (!FMath::IsNearlyZero(ForwardInput) || !FMath::IsNearlyZero(RightInput))
	{
		CameraPawn->MovePlanar(FVector2D(ForwardInput, RightInput));
	}

	// 2. Zoom Input (Mouse Wheel)
	const float MouseWheelAxis = GetInputAnalogKeyState(EKeys::MouseWheelAxis);
	if (!FMath::IsNearlyZero(MouseWheelAxis))
	{
		CameraPawn->AddZoomInput(MouseWheelAxis);
	}

	// 3. Orbit Rotation Input (Middle Mouse Drag)
	FVector2D CurrentMousePos;
	GetMousePosition(CurrentMousePos.X, CurrentMousePos.Y);

	if (IsInputKeyDown(EKeys::MiddleMouseButton))
	{
		if (bMiddleMouseDown)
		{
			const float DeltaX = CurrentMousePos.X - LastMousePosition.X;
			if (!FMath::IsNearlyZero(DeltaX))
			{
				CameraPawn->AddYawRotationInput(DeltaX);
			}
		}
		bMiddleMouseDown = true;
	}
	else
	{
		bMiddleMouseDown = false;
	}

	LastMousePosition = CurrentMousePos;
}

void ATWPlayerController::PollSelectionInput()
{
	FVector2D CurrentMousePos;
	GetMousePosition(CurrentMousePos.X, CurrentMousePos.Y);

	const bool bIsLeftDownNow = IsInputKeyDown(EKeys::LeftMouseButton);

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
		const bool bShiftHeld = IsInputKeyDown(EKeys::LeftShift) || IsInputKeyDown(EKeys::RightShift);

		if (!bShiftHeld)
		{
			DeselectAll();
		}

		if (bIsDraggingSelection)
		{
			// Marquee Box Selection: Intersect with formation screen boundary
			SelectFormationsInScreenBox(SelectionStartScreenPos, SelectionCurrentScreenPos);
		}
		else
		{
			// Single Click Selection: Unified as small screen point box with 4px tolerance
			const float ClickTolerance = 4.0f;
			SelectFormationsInScreenBox(
				CurrentMousePos - FVector2D(ClickTolerance, ClickTolerance),
				CurrentMousePos + FVector2D(ClickTolerance, ClickTolerance)
			);
		}

		bLeftMouseDown = false;
		bIsDraggingSelection = false;
	}
}

void ATWPlayerController::SelectFormation(AFormationActor* Formation, bool bAdditive)
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

void ATWPlayerController::DeselectAll()
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

void ATWPlayerController::SelectFormationsInScreenBox(const FVector2D& ScreenStart, const FVector2D& ScreenEnd)
{
	UWorld* World = GetWorld();
	if (!World)
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

		// Calculate 4 3D ground corners of the formation
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

		// Project all 4 corners to 2D screen coordinates
		float MinX = TNumericLimits<float>::Max(), MaxX = -TNumericLimits<float>::Max();
		float MinY = TNumericLimits<float>::Max(), MaxY = -TNumericLimits<float>::Max();
		bool bAnyProjected = false;

		for (int32 i = 0; i < 4; ++i)
		{
			FVector2D ScreenCorner;
			if (ProjectWorldLocationToScreen(Corners[i], ScreenCorner))
			{
				MinX = FMath::Min(MinX, ScreenCorner.X);
				MaxX = FMath::Max(MaxX, ScreenCorner.X);
				MinY = FMath::Min(MinY, ScreenCorner.Y);
				MaxY = FMath::Max(MaxY, ScreenCorner.Y);
				bAnyProjected = true;
			}
		}

		// If any corner is visible on screen, test 2D bounding box intersection
		if (bAnyProjected)
		{
			const FBox2D FormationScreenBox(FVector2D(MinX, MinY), FVector2D(MaxX, MaxY));

			if (SelectionBox.Intersect(FormationScreenBox))
			{
				Formation->SetSelected(true);
				SelectedFormations.AddUnique(Formation);
			}
		}
	}
}

ATWCameraPawn* ATWPlayerController::GetControlledCameraPawn() const
{
	return Cast<ATWCameraPawn>(GetPawn());
}
