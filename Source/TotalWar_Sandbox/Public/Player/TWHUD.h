// Copyright Total War Sandbox. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "TotalWar_Sandbox/Public/Core/TWTypes.h"
#include "TWHUD.generated.h"

class ATWPlayerController;

/**
 * ATWHUD — Màn Hình Hiển Thị & Render Canvas (RTS HUD).
 * Trách nhiệm duy nhất (SRP):
 *   - Render các thành phần đồ họa 2D lên màn hình (Canvas): Khung quét chọn quân (Marquee Box),
 *     thanh máu, và các chỉ báo debug.
 * Follows Single Responsibility Principle (SRP) & Dependency Inversion Principle (DIP).
 */
UCLASS()
class TOTALWAR_SANDBOX_API ATWHUD : public AHUD
{
	GENERATED_BODY()

public:
	ATWHUD();

	virtual void DrawHUD() override;

protected:
	// ========================================================================
	// CONFIGURATION & STYLING (Tunable in Editor / BP)
	// ========================================================================

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD|Selection")
	FLinearColor SelectionBoxFillColor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD|Selection")
	FLinearColor SelectionBoxBorderColor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD|Selection")
	float SelectionBoxBorderThickness;

private:
	void DrawSelectionBox(ATWPlayerController* PC);
	void DrawFormationGroundBox(ATWPlayerController* PC, AFormationActor* Formation, const FLinearColor& Color, float Thickness);
};
