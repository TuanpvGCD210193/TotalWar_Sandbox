// Copyright Total War Sandbox. All Rights Reserved.

#include "TotalWar_Sandbox/Public/Player/TWPlayerController.h"
#include "TotalWar_Sandbox/Public/Player/TWSelectionComponent.h"
#include "TotalWar_Sandbox/Public/Player/TWCommandComponent.h"
#include "TotalWar_Sandbox/Public/Player/TWCameraPawn.h"

ATWPlayerController::ATWPlayerController()
{
	bShowMouseCursor = true;
	bEnableClickEvents = true;
	bEnableMouseOverEvents = true;
	DefaultMouseCursor = EMouseCursor::Default;
	bMiddleMouseDown = false;
	LastMousePosition = FVector2D::ZeroVector;
	// Attach Specialized Controller Sub-Components (SRP)
	SelectionComponent = CreateDefaultSubobject<UTWSelectionComponent>(TEXT("SelectionComponent"));
	CommandComponent = CreateDefaultSubobject<UTWCommandComponent>(TEXT("CommandComponent"));
}

void ATWPlayerController::BeginPlay()
{
	Super::BeginPlay();
	FInputModeGameAndUI InputMode;
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	InputMode.SetHideCursorDuringCapture(false);
	SetInputMode(InputMode);
}

void ATWPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);
	// 1. Camera Navigation Input
	PollCameraInput(DeltaTime);
	// 2. Delegate Selection Input to SelectionComponent
	if (SelectionComponent)
	{
		SelectionComponent->PollSelectionInput();
	}
	// 3. Delegate Tactical Command Input to CommandComponent
	if (CommandComponent && SelectionComponent)
	{
		CommandComponent->PollCommandInput(SelectionComponent->GetSelectedFormations());
	}
}

void ATWPlayerController::PollCameraInput(float DeltaTime)
{
	ATWCameraPawn* CamPawn = GetControlledCameraPawn();
	if (!CamPawn)
	{
		return;
	}

	// WASD & Arrow Keys Pan
	float ForwardAxis = 0.0f;
	float RightAxis = 0.0f;
	if (IsInputKeyDown(EKeys::W) || IsInputKeyDown(EKeys::Up)) ForwardAxis += 1.0f;
	if (IsInputKeyDown(EKeys::S) || IsInputKeyDown(EKeys::Down)) ForwardAxis -= 1.0f;
	if (IsInputKeyDown(EKeys::D) || IsInputKeyDown(EKeys::Right)) RightAxis += 1.0f;
	if (IsInputKeyDown(EKeys::A) || IsInputKeyDown(EKeys::Left)) RightAxis -= 1.0f;

	if (ForwardAxis != 0.0f || RightAxis != 0.0f)
	{
		CamPawn->MovePlanar(FVector2D(ForwardAxis, RightAxis));
	}

	// Mouse Wheel Zoom
	const float WheelDelta = GetInputAnalogKeyState(EKeys::MouseWheelAxis);
	if (FMath::Abs(WheelDelta) > KINDA_SMALL_NUMBER)
	{
		CamPawn->AddZoomInput(WheelDelta);
	}
	// Middle Mouse Drag Orbit Yaw
	FVector2D CurrentMousePos;
	GetMousePosition(CurrentMousePos.X, CurrentMousePos.Y);

	const bool bIsMiddleDown = IsInputKeyDown(EKeys::MiddleMouseButton);

	if (bIsMiddleDown && !bMiddleMouseDown)
	{
		bMiddleMouseDown = true;
		LastMousePosition = CurrentMousePos;
	}
	else if (bIsMiddleDown && bMiddleMouseDown)
	{
		const float DeltaX = CurrentMousePos.X - LastMousePosition.X;
		if (FMath::Abs(DeltaX) > KINDA_SMALL_NUMBER)
		{
			CamPawn->AddYawRotationInput(DeltaX);
		}
		LastMousePosition = CurrentMousePos;
	}
	else if (!bIsMiddleDown && bMiddleMouseDown)
	{
		bMiddleMouseDown = false;
	}
}

ATWCameraPawn* ATWPlayerController::GetControlledCameraPawn() const
{
	return Cast<ATWCameraPawn>(GetPawn());
}

// ========================================================================
// ENCAPSULATED FORWARDERS (Law of Demeter & Clean API)
// ========================================================================

bool ATWPlayerController::IsDraggingSelection() const
{
	return SelectionComponent ? SelectionComponent->IsDraggingSelection() : false;
}

const FVector2D& ATWPlayerController::GetSelectionStartPos() const
{
	return SelectionComponent ? SelectionComponent->GetSelectionStartPos() : FVector2D::ZeroVector;
}

const FVector2D& ATWPlayerController::GetSelectionCurrentPos() const
{
	return SelectionComponent ? SelectionComponent->GetSelectionCurrentPos() : FVector2D::ZeroVector;
}

const TArray<TObjectPtr<AFormationActor>>& ATWPlayerController::GetSelectedFormations() const
{
	static const TArray<TObjectPtr<AFormationActor>> EmptyFormations;
	return SelectionComponent ? SelectionComponent->GetSelectedFormations() : EmptyFormations;
}

bool ATWPlayerController::HasSelectedFormations() const
{
	return SelectionComponent ? SelectionComponent->HasSelectedFormations() : false;
}

bool ATWPlayerController::IsDraggingPlacement() const
{
	return CommandComponent ? CommandComponent->IsDraggingPlacement() : false;
}

const FVector& ATWPlayerController::GetPlacementCenter() const
{
	return CommandComponent ? CommandComponent->GetPlacementCenter() : FVector::ZeroVector;
}

const FVector& ATWPlayerController::GetPlacementFacing() const
{
	return CommandComponent ? CommandComponent->GetPlacementFacing() : FVector::ForwardVector;
}

int32 ATWPlayerController::GetPlacementWidth() const
{
	return CommandComponent ? CommandComponent->GetPlacementWidth() : TWConstants::DefaultFormationWidth;
}

float ATWPlayerController::GetPlacementDragLength() const
{
	return CommandComponent ? CommandComponent->GetPlacementDragLength() : 0.0f;
}

bool ATWPlayerController::IsSpaceBarHeld() const
{
	return CommandComponent ? CommandComponent->IsSpaceBarHeld() : false;
}
