// Copyright Total War Sandbox. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TotalWar_Sandbox/Public/Core/TWTypes.h"
#include "TotalWar_Sandbox/Public/Formation/FormationCommand.h"
#include "TWCommandComponent.generated.h"

class AFormationActor;
class APlayerController;

/**
 * UTWCommandComponent — Bộ Điều Lệnh Tác Chiến (RTS Tactical Command Sub-Component).
 * Trách nhiệm duy nhất (SRP):
 *   - Lắng nghe thao tác Click chuột phải (Quick Move / Attack) & Giữ kéo chuột phải dàn trận (Placement Drag).
 *   - Nhận diện quân Đỏ dưới con trỏ chuột để phát lệnh Attack.
 *   - Theo dõi trạng thái phím Space cho Tactical Overlay.
 * Follows Single Responsibility Principle (SRP) & SOLID architecture.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TOTALWAR_SANDBOX_API UTWCommandComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UTWCommandComponent();

	// ========================================================================
	// PUBLIC COMMAND API
	// ========================================================================

	/** Polls raw right mouse input and dispatches move/attack/placement commands */
	void PollCommandInput(const TArray<TObjectPtr<AFormationActor>>& SelectedFormations);

	/** Finds an enemy Red formation actor under the mouse cursor */
	AFormationActor* GetEnemyFormationUnderCursor() const;

	// ========================================================================
	// GETTERS (Encapsulation)
	// ========================================================================

	FORCEINLINE bool IsDraggingPlacement() const { return bIsDraggingPlacement; }
	FORCEINLINE const FVector& GetPlacementCenter() const { return PlacementCenter; }
	FORCEINLINE const FVector& GetPlacementFacing() const { return PlacementFacing; }
	FORCEINLINE int32 GetPlacementWidth() const { return PlacementWidth; }
	FORCEINLINE float GetPlacementDragLength() const { return (RightDragCurrentGroundWorldPos - RightDragStartGroundWorldPos).Size2D(); }
	FORCEINLINE bool IsSpaceBarHeld() const { return bIsSpaceHeld; }

protected:
	bool bIsSpaceHeld;
	bool bRightMouseDown;
	bool bIsDraggingPlacement;

	FVector2D RightClickStartPos;
	FVector RightDragStartGroundWorldPos;
	FVector RightDragCurrentGroundWorldPos;
	FVector PlacementCenter;
	FVector PlacementFacing;
	int32 PlacementWidth;

private:
	APlayerController* GetOwningPlayerController() const;
};
