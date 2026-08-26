// Copyright Total War Sandbox. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "TotalWar_Sandbox/Public/Core/TWTypes.h"
#include "TotalWar_Sandbox/Public/Formation/FormationCommand.h"
#include "FormationGroupPlacement.generated.h"

class AFormationActor;

/**
 * UFormationGroupPlacement — Bộ Tính Toán Dàn Trận Nhóm (Group Battleline Calculator).
 * Trách nhiệm duy nhất (SRP):
 *   - Sắp xếp thứ tự các đạo quân theo trục Trái -> Phải.
 *   - Tính toán phân bổ vị trí chiến tuyến (Battleline) song song cho nhiều squad,
 *     đảm bảo cự ly an toàn (Squad Gap), không bao giờ bị đè/chồng lấn lên nhau (chuẩn Total War).
 * Follows Single Responsibility Principle (SRP) & Dependency Inversion Principle (DIP).
 */
UCLASS()
class TOTALWAR_SANDBOX_API UFormationGroupPlacement : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** Tactical gap between adjacent squads in centimeters (Default: 250cm = 2.5m) */
	static constexpr float DefaultSquadGap = 250.0f;

	/**
	 * Sorts a list of formation actors from Left to Right relative to a given facing direction.
	 */
	static void SortFormationsLeftToRight(
		const TArray<TObjectPtr<AFormationActor>>& InFormations,
		const FVector& FacingDirection,
		TArray<AFormationActor*>& OutSortedFormations
	);

	/**
	 * Calculates multi-squad placement commands when dragging right mouse button (Total War Battleline).
	 */
	static void CalculateGroupPlacementCommands(
		const TArray<TObjectPtr<AFormationActor>>& Formations,
		const FVector& GroupCenter,
		const FVector& FacingDirection,
		float TotalDragLength,
		TMap<AFormationActor*, FFormationCommand>& OutCommands
	);

	/**
	 * Calculates multi-squad quick move commands preserving relative tactical formation offsets.
	 */
	static void CalculateGroupQuickMoveCommands(
		const TArray<TObjectPtr<AFormationActor>>& Formations,
		const FVector& TargetDestination,
		const FVector& FacingDirection,
		TMap<AFormationActor*, FFormationCommand>& OutCommands
	);
};
