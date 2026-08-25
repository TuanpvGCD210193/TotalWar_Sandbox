// Copyright Total War Sandbox. All Rights Reserved.

#include "TotalWar_Sandbox/Public/Player/TWHUD.h"
#include "TotalWar_Sandbox/Public/Player/TWPlayerController.h"
#include "TotalWar_Sandbox/Public/Formation/FormationActor.h"
#include "Engine/Canvas.h"
#include "CanvasItem.h"

ATWHUD::ATWHUD()
{
	PrimaryActorTick.bCanEverTick = false;

	// Semi-transparent light cyan fill
	SelectionBoxFillColor = FLinearColor(0.0f, 0.7f, 1.0f, 0.15f);

	// Crisp bright cyan border
	SelectionBoxBorderColor = FLinearColor(0.1f, 0.9f, 1.0f, 1.0f);

	SelectionBoxBorderThickness = 1.5f;
}

void ATWHUD::DrawHUD()
{
	Super::DrawHUD();

	if (ATWPlayerController* PC = Cast<ATWPlayerController>(GetOwningPlayerController()))
	{
		// 1. Draw Selection Marquee Box
		if (PC->IsDraggingSelection())
		{
			DrawSelectionBox(PC);
		}

		// 2. Draw Ground Selection Highlights for all selected formations
		const TArray<TObjectPtr<AFormationActor>>& Selecteds = PC->GetSelectedFormations();
		for (AFormationActor* Formation : Selecteds)
		{
			if (Formation && Formation->GetAliveCount() > 0)
			{
				FVector Center, Extents;
				FRotator Rotation;
				Formation->GetFormationGroundBounds(Center, Extents, Rotation);

				const FVector ForwardVec = Rotation.Vector() * Extents.X;
				const FVector RightVec = FRotationMatrix(Rotation).GetUnitAxis(EAxis::Y) * Extents.Y;

				const FVector P1 = Center + ForwardVec + RightVec;
				const FVector P2 = Center + ForwardVec - RightVec;
				const FVector P3 = Center - ForwardVec - RightVec;
				const FVector P4 = Center - ForwardVec + RightVec;

				FVector2D S1, S2, S3, S4;
				const bool b1 = PC->ProjectWorldLocationToScreen(P1, S1);
				const bool b2 = PC->ProjectWorldLocationToScreen(P2, S2);
				const bool b3 = PC->ProjectWorldLocationToScreen(P3, S3);
				const bool b4 = PC->ProjectWorldLocationToScreen(P4, S4);

				if (b1 && b2 && b3 && b4)
				{
					const FLinearColor HighlightColor(0.2f, 1.0f, 0.3f, 0.9f); // Bright Green
					const float Thickness = 2.0f;

					FCanvasLineItem L1(S1, S2); L1.SetColor(HighlightColor); L1.LineThickness = Thickness; Canvas->DrawItem(L1);
					FCanvasLineItem L2(S2, S3); L2.SetColor(HighlightColor); L2.LineThickness = Thickness; Canvas->DrawItem(L2);
					FCanvasLineItem L3(S3, S4); L3.SetColor(HighlightColor); L3.LineThickness = Thickness; Canvas->DrawItem(L3);
					FCanvasLineItem L4(S4, S1); L4.SetColor(HighlightColor); L4.LineThickness = Thickness; Canvas->DrawItem(L4);
				}
			}
		}
	}
}

void ATWHUD::DrawSelectionBox(ATWPlayerController* PC)
{
	if (!PC || !Canvas)
	{
		return;
	}

	const FVector2D StartPos = PC->GetSelectionStartPos();
	const FVector2D CurrentPos = PC->GetSelectionCurrentPos();

	const float MinX = FMath::Min(StartPos.X, CurrentPos.X);
	const float MinY = FMath::Min(StartPos.Y, CurrentPos.Y);
	const float Width = FMath::Abs(CurrentPos.X - StartPos.X);
	const float Height = FMath::Abs(CurrentPos.Y - StartPos.Y);

	if (Width <= 0.0f || Height <= 0.0f)
	{
		return;
	}

	// 1. Draw Semi-transparent Filled Background
	FCanvasTileItem FillTile(FVector2D(MinX, MinY), FVector2D(Width, Height), SelectionBoxFillColor);
	FillTile.BlendMode = SE_BLEND_Translucent;
	Canvas->DrawItem(FillTile);

	// 2. Draw 4 Border Lines (Top, Bottom, Left, Right)
	// Top Line
	FCanvasLineItem TopLine(FVector2D(MinX, MinY), FVector2D(MinX + Width, MinY));
	TopLine.SetColor(SelectionBoxBorderColor);
	TopLine.LineThickness = SelectionBoxBorderThickness;
	Canvas->DrawItem(TopLine);

	// Bottom Line
	FCanvasLineItem BottomLine(FVector2D(MinX, MinY + Height), FVector2D(MinX + Width, MinY + Height));
	BottomLine.SetColor(SelectionBoxBorderColor);
	BottomLine.LineThickness = SelectionBoxBorderThickness;
	Canvas->DrawItem(BottomLine);

	// Left Line
	FCanvasLineItem LeftLine(FVector2D(MinX, MinY), FVector2D(MinX, MinY + Height));
	LeftLine.SetColor(SelectionBoxBorderColor);
	LeftLine.LineThickness = SelectionBoxBorderThickness;
	Canvas->DrawItem(LeftLine);

	// Right Line
	FCanvasLineItem RightLine(FVector2D(MinX + Width, MinY), FVector2D(MinX + Width, MinY + Height));
	RightLine.SetColor(SelectionBoxBorderColor);
	RightLine.LineThickness = SelectionBoxBorderThickness;
	Canvas->DrawItem(RightLine);
}
