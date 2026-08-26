// Copyright Total War Sandbox. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TotalWar_Sandbox/Public/Core/TWTypes.h"
#include "TWSelectionComponent.generated.h"

class AFormationActor;
class APlayerController;

/**
 * UTWSelectionComponent — Bộ Quản Lý Chọn Quân (RTS Selection Sub-Component).
 * Trách nhiệm duy nhất (SRP):
 *   - Lắng nghe thao tác Click chuột trái / Quét hộp Marquee Box.
 *   - Quản lý danh sách các đạo quân đang được chọn (SelectedFormations).
 *   - Tính toán giao điểm Screen Bounding Box 2D và phím Shift chọn thêm.
 * Follows Single Responsibility Principle (SRP) & SOLID architecture.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TOTALWAR_SANDBOX_API UTWSelectionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UTWSelectionComponent();

	// ========================================================================
	// PUBLIC SELECTION API
	// ========================================================================

	/** Polls raw left mouse input for single click and marquee box drag selection */
	void PollSelectionInput();

	/** Selects a single formation actor */
	void SelectFormation(AFormationActor* Formation, bool bAdditive = false);

	/** Deselects all currently selected formations */
	void DeselectAll();

	/** Selects all formations inside a 2D screen coordinate bounding box */
	void SelectFormationsInScreenBox(const FVector2D& ScreenStart, const FVector2D& ScreenEnd, bool bShiftHeld = false, bool bIsSingleClick = false);

	// ========================================================================
	// GETTERS (Encapsulation)
	// ========================================================================

	FORCEINLINE bool IsDraggingSelection() const { return bIsDraggingSelection; }
	FORCEINLINE const FVector2D& GetSelectionStartPos() const { return SelectionStartScreenPos; }
	FORCEINLINE const FVector2D& GetSelectionCurrentPos() const { return SelectionCurrentScreenPos; }
	FORCEINLINE const TArray<TObjectPtr<AFormationActor>>& GetSelectedFormations() const { return SelectedFormations; }
	FORCEINLINE bool HasSelectedFormations() const { return SelectedFormations.Num() > 0; }

protected:
	UPROPERTY(Transient)
	TArray<TObjectPtr<AFormationActor>> SelectedFormations;

	bool bLeftMouseDown;
	bool bIsDraggingSelection;
	FVector2D SelectionStartScreenPos;
	FVector2D SelectionCurrentScreenPos;

private:
	APlayerController* GetOwningPlayerController() const;
};
