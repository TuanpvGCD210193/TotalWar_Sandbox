// Copyright Total War Sandbox. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "TotalWar_Sandbox/Public/Core/TWTypes.h"
#include "TWCameraPawn.generated.h"

class USceneComponent;
class USpringArmComponent;
class UCameraComponent;

/**
 * RTS Camera Pawn providing top-down isometric/orbit view.
 * Handles planar movement (WASD), zoom (Mouse Wheel), and orbit yaw (MMB drag).
 * Follows Single Responsibility Principle (SRP): Only manages camera viewpoint and navigation.
 */
UCLASS()
class TOTALWAR_SANDBOX_API ATWCameraPawn : public APawn
{
	GENERATED_BODY()

public:
	ATWCameraPawn();

	virtual void Tick(float DeltaTime) override;

	// ========================================================================
	// PUBLIC API (Called by PlayerController via Raw Input Polling)
	// ========================================================================

	/** Moves camera along the ground plane based on forward/right input (+X = Forward, +Y = Right) */
	void MovePlanar(const FVector2D& DirectionInput);

	/** Adjusts target zoom level (+1 = Zoom In, -1 = Zoom Out) */
	void AddZoomInput(float AxisValue);

	/** Rotates camera yaw around current focus point */
	void AddYawRotationInput(float DeltaYaw);

	/** Instantly teleports camera focus to a specific world location */
	void FocusOnLocation(const FVector& WorldLocation);

	// ========================================================================
	// GETTERS (Encapsulation)
	// ========================================================================

	FORCEINLINE USpringArmComponent* GetSpringArm() const { return SpringArmComponent; }
	FORCEINLINE UCameraComponent* GetCameraComponent() const { return CameraComponent; }
	FORCEINLINE float GetCurrentZoom() const { return CurrentArmLength; }

protected:
	virtual void BeginPlay() override;

	// ========================================================================
	// COMPONENTS
	// ========================================================================

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<USceneComponent> TransformRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<USpringArmComponent> SpringArmComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<UCameraComponent> CameraComponent;

	// ========================================================================
	// CONFIGURATION & TUNING
	// ========================================================================

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Speed")
	float MoveSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Speed")
	float FastMoveMultiplier;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Speed")
	float ZoomSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Speed")
	float RotateSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Limits")
	float MinZoomDistance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Limits")
	float MaxZoomDistance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Smoothing")
	float MoveInterpolationSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Smoothing")
	float ZoomInterpolationSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Smoothing")
	float RotateInterpolationSpeed;

private:
	// ========================================================================
	// INTERNAL RUNTIME STATE
	// ========================================================================

	FVector TargetLocation;
	FVector2D PendingMoveInput;
	float TargetArmLength;
	float CurrentArmLength;
	float TargetYaw;
	float CurrentYaw;

	void UpdateCameraTransform(float DeltaTime);
};
