// Copyright Total War Sandbox. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "TotalWar_Sandbox/Public/Core/TWTypes.h"
#include "TWPlayerController.generated.h"

class ATWCameraPawn;
class AFormationActor;

/**
 * ATWPlayerController — Bộ Điều Khiển Người Chơi (RTS Player Controller).
 * Trách nhiệm duy nhất (SRP):
 *   - Lắng nghe raw input (bàn phím WASD, cuộn chuột, chuột trái/phải/giữa) mỗi frame qua PlayerTick.
 *   - Điều khiển Camera Pawn.
 *   - Quản lý trạng thái chọn quân (Selection state machine: Click đơn lẻ hoặc Quét hộp chữ nhật).
 * Follows Single Responsibility Principle (SRP) & Dependency Inversion Principle (DIP).
 */
UCLASS()
class TOTALWAR_SANDBOX_API ATWPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ATWPlayerController();

	virtual void PlayerTick(float DeltaTime) override;

	// ========================================================================
	// SELECTION PUBLIC API
	// ========================================================================

	/** Selects a single formation, optionally adding to existing selection (Shift/Ctrl) */
	void SelectFormation(AFormationActor* Formation, bool bAdditive = false);

	/** Clears all currently selected formations */
	void DeselectAll();

	/** Selects all player (Blue) formations whose world bounds intersect the 2D screen box */
	void SelectFormationsInScreenBox(const FVector2D& ScreenStart, const FVector2D& ScreenEnd);

	// ========================================================================
	// GETTERS FOR HUD & EXTERNAL SYSTEMS (Encapsulation)
	// ========================================================================

	FORCEINLINE bool IsDraggingSelection() const { return bIsDraggingSelection; }
	FORCEINLINE const FVector2D& GetSelectionStartPos() const { return SelectionStartScreenPos; }
	FORCEINLINE const FVector2D& GetSelectionCurrentPos() const { return SelectionCurrentScreenPos; }
	FORCEINLINE const TArray<TObjectPtr<AFormationActor>>& GetSelectedFormations() const { return SelectedFormations; }
	FORCEINLINE bool HasSelectedFormations() const { return SelectedFormations.Num() > 0; }

protected:
	virtual void BeginPlay() override;

private:
	// ========================================================================
	// RUNTIME SELECTION DATA
	// ========================================================================

	UPROPERTY(Transient)
	TArray<TObjectPtr<AFormationActor>> SelectedFormations;

	bool bLeftMouseDown;
	bool bIsDraggingSelection;
	FVector2D SelectionStartScreenPos;
	FVector2D SelectionCurrentScreenPos;

	FVector2D LastMousePosition;
	bool bMiddleMouseDown;

	// ========================================================================
	// INPUT POLLING METHODS (Raw Input Handlers)
	// ========================================================================

	void PollCameraInput(float DeltaTime);
	void PollSelectionInput();
	ATWCameraPawn* GetControlledCameraPawn() const;
};
