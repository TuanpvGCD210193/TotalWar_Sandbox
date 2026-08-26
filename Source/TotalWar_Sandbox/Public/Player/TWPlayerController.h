// Copyright Total War Sandbox. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "TotalWar_Sandbox/Public/Core/TWTypes.h"
#include "TWPlayerController.generated.h"

class ATWCameraPawn;
class AFormationActor;
class UTWSelectionComponent;
class UTWCommandComponent;

/**
 * ATWPlayerController — Bộ Điều Khiển Người Chơi (Thin Controller Hub).
 * Trách nhiệm duy nhất (SRP):
 *   - Lắng nghe Input Camera (WASD/Zoom/Orbit).
 *   - Điều phối 2 Sub-Components chuyên biệt (SRP):
 *       1. UTWSelectionComponent (Quét/chọn quân phe ta)
 *       2. UTWCommandComponent (Phát lệnh di chuyển/dàn trận/tấn công)
 * Follows Single Responsibility Principle (SRP) & SOLID architecture.
 */
UCLASS()
class TOTALWAR_SANDBOX_API ATWPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ATWPlayerController();

	virtual void PlayerTick(float DeltaTime) override;

	// ========================================================================
	// GETTERS & ENCAPSULATED ACCESSORS (Forwarders for HUD)
	// ========================================================================

	FORCEINLINE UTWSelectionComponent* GetSelectionComponent() const { return SelectionComponent; }
	FORCEINLINE UTWCommandComponent* GetCommandComponent() const { return CommandComponent; }

	// Forwarders to SelectionComponent
	bool IsDraggingSelection() const;
	const FVector2D& GetSelectionStartPos() const;
	const FVector2D& GetSelectionCurrentPos() const;
	const TArray<TObjectPtr<AFormationActor>>& GetSelectedFormations() const;
	bool HasSelectedFormations() const;

	// Forwarders to CommandComponent
	bool IsDraggingPlacement() const;
	const FVector& GetPlacementCenter() const;
	const FVector& GetPlacementFacing() const;
	int32 GetPlacementWidth() const;
	float GetPlacementDragLength() const;
	bool IsSpaceBarHeld() const;

protected:
	virtual void BeginPlay() override;

	// ========================================================================
	// SPECIALIZED CONTROLLER COMPONENTS
	// ========================================================================

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player|Components")
	TObjectPtr<UTWSelectionComponent> SelectionComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player|Components")
	TObjectPtr<UTWCommandComponent> CommandComponent;

private:
	FVector2D LastMousePosition;
	bool bMiddleMouseDown;

	void PollCameraInput(float DeltaTime);
	ATWCameraPawn* GetControlledCameraPawn() const;
};
