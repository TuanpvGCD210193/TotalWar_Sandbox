// Copyright Total War Sandbox. All Rights Reserved.

#include "TotalWar_Sandbox/Public/Player/TWCameraPawn.h"
#include "Components/SceneComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"

ATWCameraPawn::ATWCameraPawn()
{
	PrimaryActorTick.bCanEverTick = true;

	// 1. Root Component
	TransformRoot = CreateDefaultSubobject<USceneComponent>(TEXT("TransformRoot"));
	RootComponent = TransformRoot;

	// 2. Spring Arm (Boom)
	SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArmComponent->SetupAttachment(RootComponent);
	SpringArmComponent->SetRelativeRotation(FRotator(TWConstants::CameraDefaultPitch, 0.0f, 0.0f));
	SpringArmComponent->TargetArmLength = TWConstants::CameraDefaultHeight;
	SpringArmComponent->bDoCollisionTest = false; // Do not clip on ground/units
	SpringArmComponent->bEnableCameraLag = false;  // Handled via custom math interpolation
	SpringArmComponent->bInheritPitch = false;
	SpringArmComponent->bInheritRoll = false;
	SpringArmComponent->bInheritYaw = true;

	// 3. Camera Component
	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));
	CameraComponent->SetupAttachment(SpringArmComponent, USpringArmComponent::SocketName);
	CameraComponent->bUsePawnControlRotation = false;

	// Defaults from constants
	MoveSpeed = TWConstants::CameraMoveSpeed;
	FastMoveMultiplier = 2.0f;
	ZoomSpeed = TWConstants::CameraZoomSpeed;
	RotateSpeed = TWConstants::CameraRotateSpeed;
	MinZoomDistance = TWConstants::CameraMinHeight;
	MaxZoomDistance = TWConstants::CameraMaxHeight;

	MoveInterpolationSpeed = 12.0f;
	ZoomInterpolationSpeed = 10.0f;
	RotateInterpolationSpeed = 10.0f;

	TargetArmLength = TWConstants::CameraDefaultHeight;
	CurrentArmLength = TargetArmLength;
	TargetYaw = 0.0f;
	CurrentYaw = 0.0f;
	PendingMoveInput = FVector2D::ZeroVector;
}

void ATWCameraPawn::BeginPlay()
{
	Super::BeginPlay();

	TargetLocation = GetActorLocation();
	CurrentArmLength = SpringArmComponent->TargetArmLength;
	TargetArmLength = CurrentArmLength;
	CurrentYaw = GetActorRotation().Yaw;
	TargetYaw = CurrentYaw;
}

void ATWCameraPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateCameraTransform(DeltaTime);
}

void ATWCameraPawn::MovePlanar(const FVector2D& DirectionInput)
{
	PendingMoveInput += DirectionInput;
}

void ATWCameraPawn::AddZoomInput(float AxisValue)
{
	if (FMath::IsNearlyZero(AxisValue))
	{
		return;
	}

	// -AxisValue because WheelUp (+1) zooms IN (reduces arm length)
	TargetArmLength = FMath::Clamp(
		TargetArmLength - (AxisValue * ZoomSpeed),
		MinZoomDistance,
		MaxZoomDistance
	);
}

void ATWCameraPawn::AddYawRotationInput(float DeltaYaw)
{
	TargetYaw += DeltaYaw * RotateSpeed;
}

void ATWCameraPawn::FocusOnLocation(const FVector& WorldLocation)
{
	TargetLocation = WorldLocation;
	SetActorLocation(WorldLocation);
}

void ATWCameraPawn::UpdateCameraTransform(float DeltaTime)
{
	// 1. Process Planar Movement
	if (!PendingMoveInput.IsNearlyZero())
	{
		const FRotator YawRotation(0.0f, CurrentYaw, 0.0f);
		const FVector ForwardDir = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector RightDir = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// Speed scales slightly with camera height for uniform screen-space movement feeling
		const float HeightScale = FMath::Lerp(0.6f, 1.8f, (CurrentArmLength - MinZoomDistance) / FMath::Max(1.0f, MaxZoomDistance - MinZoomDistance));
		const FVector MoveDelta = (ForwardDir * PendingMoveInput.X + RightDir * PendingMoveInput.Y).GetSafeNormal() * (MoveSpeed * HeightScale * DeltaTime);

		TargetLocation += MoveDelta;
		PendingMoveInput = FVector2D::ZeroVector;
	}

	// Smoothly interpolate position
	const FVector NewLocation = FMath::VInterpTo(GetActorLocation(), TargetLocation, DeltaTime, MoveInterpolationSpeed);
	SetActorLocation(NewLocation);

	// 2. Smoothly interpolate Zoom
	CurrentArmLength = FMath::FInterpTo(CurrentArmLength, TargetArmLength, DeltaTime, ZoomInterpolationSpeed);
	SpringArmComponent->TargetArmLength = CurrentArmLength;

	// 3. Smoothly interpolate Yaw Rotation
	CurrentYaw = FMath::FInterpTo(CurrentYaw, TargetYaw, DeltaTime, RotateInterpolationSpeed);
	SetActorRotation(FRotator(0.0f, CurrentYaw, 0.0f));
}
