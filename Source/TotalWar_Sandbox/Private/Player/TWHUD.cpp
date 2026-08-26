// Copyright Total War Sandbox. All Rights Reserved.

#include "TotalWar_Sandbox/Public/Player/TWHUD.h"
#include "TotalWar_Sandbox/Public/Player/TWPlayerController.h"
#include "TotalWar_Sandbox/Public/Formation/FormationActor.h"
#include "TotalWar_Sandbox/Public/Formation/FormationSlotComponent.h"
#include "TotalWar_Sandbox/Public/Formation/FormationGroupPlacement.h"
#include "TotalWar_Sandbox/Public/Systems/FormationSubsystem.h"
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
			DrawFormationGroundBox(PC, Formation, FLinearColor(0.2f, 1.0f, 0.3f, 0.9f), 2.0f);
		}

		// 3. Draw Total War Group Placement Preview (Destination Grid + Arrow for ALL squads)
		if (PC->IsDraggingPlacement() && Selecteds.Num() > 0)
		{
			const FVector Center = PC->GetPlacementCenter();
			const FVector Facing = PC->GetPlacementFacing();
			const float DragLength = PC->GetPlacementDragLength();

			TMap<AFormationActor*, FFormationCommand> GroupCmds;
			UFormationGroupPlacement::CalculateGroupPlacementCommands(
				Selecteds,
				Center,
				Facing,
				DragLength,
				GroupCmds
			);

			const FLinearColor PreviewSlotColor(1.0f, 0.85f, 0.1f, 0.85f); // Bright Gold/Yellow
			const float DotSize = 4.0f;

			for (const auto& Pair : GroupCmds)
			{
				AFormationActor* Squad = Pair.Key;
				const FFormationCommand& SquadCmd = Pair.Value;

				if (Squad && Squad->GetAliveCount() > 0)
				{
					TArray<FFormationSlot> PreviewSlots;
					UFormationSubsystem::CalculateSlotLayout(
						Squad->GetAliveCount(),
						SquadCmd.TargetWidth,
						Squad->GetSoldierSpacing(),
						SquadCmd.TargetLocation,
						SquadCmd.FacingDirection,
						PreviewSlots
					);

					for (const FFormationSlot& Slot : PreviewSlots)
					{
						FVector2D ScreenPos;
						if (PC->ProjectWorldLocationToScreen(Slot.WorldPosition, ScreenPos))
						{
							FCanvasTileItem Dot(ScreenPos - FVector2D(DotSize * 0.5f, DotSize * 0.5f), FVector2D(DotSize, DotSize), PreviewSlotColor);
							Dot.BlendMode = SE_BLEND_Translucent;
							Canvas->DrawItem(Dot);
						}
					}

					// Draw Movement Arrow from Formation Center to Preview Center
					FVector2D StartScreen, EndScreen;
					if (PC->ProjectWorldLocationToScreen(Squad->GetFormationCenter(), StartScreen) &&
						PC->ProjectWorldLocationToScreen(SquadCmd.TargetLocation, EndScreen))
					{
						const FLinearColor ArrowColor(1.0f, 0.9f, 0.2f, 0.9f);
						FCanvasLineItem MoveArrow(StartScreen, EndScreen);
						MoveArrow.SetColor(ArrowColor);
						MoveArrow.LineThickness = 2.0f;
						Canvas->DrawItem(MoveArrow);

						// Draw Facing indicator arrow at destination
						FVector2D FacingEndScreen;
						if (PC->ProjectWorldLocationToScreen(SquadCmd.TargetLocation + SquadCmd.FacingDirection * 250.0f, FacingEndScreen))
						{
							FCanvasLineItem FacingArrow(EndScreen, FacingEndScreen);
							FacingArrow.SetColor(FLinearColor(0.2f, 1.0f, 0.4f, 1.0f)); // Green facing pointer
							FacingArrow.LineThickness = 2.5f;
							Canvas->DrawItem(FacingArrow);
						}
					}
				}
			}
		}

		// 3. Movement & Attack Indicators for Selected Formations
		if (!PC->IsDraggingPlacement())
		{
			for (AFormationActor* Squad : Selecteds)
			{
				if (!Squad || Squad->GetAliveCount() <= 0)
				{
					continue;
				}

				// 3a. Attack Order Indicator: Red Arrow + Red Target Bounding Box
				if (Squad->IsAttacking() && IsValid(Squad->GetTargetEnemyFormation()))
				{
					AFormationActor* Enemy = Squad->GetTargetEnemyFormation();
					FVector2D StartScreen, EnemyScreen;
					if (PC->ProjectWorldLocationToScreen(Squad->GetFormationCenter(), StartScreen) &&
						PC->ProjectWorldLocationToScreen(Enemy->GetFormationCenter(), EnemyScreen))
					{
						// Crimson Red Attack Arrow
						const FLinearColor RedAttackColor(1.0f, 0.12f, 0.12f, 0.95f);
						FCanvasLineItem AttackArrow(StartScreen, EnemyScreen);
						AttackArrow.SetColor(RedAttackColor);
						AttackArrow.LineThickness = 3.0f;
						Canvas->DrawItem(AttackArrow);

						// Draw Red Attack Arrow Head
						const FVector2D Dir = (EnemyScreen - StartScreen).GetSafeNormal();
						const FVector2D Perp(-Dir.Y, Dir.X);
						const FVector2D ArrowHead1 = EnemyScreen - Dir * 18.0f + Perp * 10.0f;
						const FVector2D ArrowHead2 = EnemyScreen - Dir * 18.0f - Perp * 10.0f;

						FCanvasLineItem HeadLine1(EnemyScreen, ArrowHead1);
						HeadLine1.SetColor(RedAttackColor);
						HeadLine1.LineThickness = 3.0f;
						Canvas->DrawItem(HeadLine1);

						FCanvasLineItem HeadLine2(EnemyScreen, ArrowHead2);
						HeadLine2.SetColor(RedAttackColor);
						HeadLine2.LineThickness = 3.0f;
						Canvas->DrawItem(HeadLine2);

						// Draw Red Target Bounding Box around targeted enemy
						DrawFormationGroundBox(PC, Enemy, RedAttackColor, 2.5f);
					}
				}
				// 3b. Move Order Indicator: Gold Destination Grid + Arrow
				else if (Squad->IsMoving())
				{
					const FVector TargetLoc = Squad->GetTargetDestination();
					const FVector TargetFacing = Squad->GetFinalDesiredFacing();
					const int32 Width = Squad->GetFormationWidth();

					TArray<FFormationSlot> DestSlots;
					UFormationSubsystem::CalculateSlotLayout(
						Squad->GetAliveCount(),
						Width,
						Squad->GetSoldierSpacing(),
						TargetLoc,
						TargetFacing,
						DestSlots
					);

					const FLinearColor GoldGridColor(1.0f, 0.85f, 0.1f, 0.8f);
					const float DotSize = 4.0f;

					for (const FFormationSlot& Slot : DestSlots)
					{
						FVector2D ScreenPos;
						if (PC->ProjectWorldLocationToScreen(Slot.WorldPosition, ScreenPos))
						{
							FCanvasTileItem Dot(ScreenPos - FVector2D(DotSize * 0.5f, DotSize * 0.5f), FVector2D(DotSize, DotSize), GoldGridColor);
							Dot.BlendMode = SE_BLEND_Translucent;
							Canvas->DrawItem(Dot);
						}
					}

					// Draw Move Arrow
					FVector2D StartScreen, EndScreen;
					if (PC->ProjectWorldLocationToScreen(Squad->GetFormationCenter(), StartScreen) &&
						PC->ProjectWorldLocationToScreen(TargetLoc, EndScreen))
					{
						const FLinearColor ArrowColor(1.0f, 0.9f, 0.2f, 0.9f);
						FCanvasLineItem MoveArrow(StartScreen, EndScreen);
						MoveArrow.SetColor(ArrowColor);
						MoveArrow.LineThickness = 2.0f;
						Canvas->DrawItem(MoveArrow);

						// Green facing pointer
						FVector2D FacingEndScreen;
						if (PC->ProjectWorldLocationToScreen(TargetLoc + TargetFacing * 250.0f, FacingEndScreen))
						{
							FCanvasLineItem FacingArrow(EndScreen, FacingEndScreen);
							FacingArrow.SetColor(FLinearColor(0.2f, 1.0f, 0.4f, 1.0f));
							FacingArrow.LineThickness = 2.5f;
							Canvas->DrawItem(FacingArrow);
						}
					}
				}
			}
		}

		// 4. Space Bar Tactical Overlay (Hold Space to reveal Formation Markers for ALL player squads)
		if (PC->IsSpaceBarHeld())
		{
			UWorld* World = GetWorld();
			UFormationSubsystem* FormationSubsystem = World ? World->GetSubsystem<UFormationSubsystem>() : nullptr;

			if (FormationSubsystem)
			{
				TArray<AFormationActor*> PlayerFormations;
				FormationSubsystem->GetFormationsByTeam(ETeamID::Blue, PlayerFormations);

				const FLinearColor SpaceGridColor(1.0f, 0.85f, 0.1f, 0.8f); // Luminous Gold
				const float DotSize = 4.0f;

				for (AFormationActor* Squad : PlayerFormations)
				{
					if (!Squad || Squad->GetAliveCount() <= 0)
					{
						continue;
					}

					// If this squad is currently having its drag placement preview drawn in Section 3, skip to avoid double rendering
					if (PC->IsDraggingPlacement() && Selecteds.Contains(Squad))
					{
						continue;
					}

					// 4a. Squad is Attacking -> Draw Red Attack Arrow + Red Target Box
					if (Squad->IsAttacking() && IsValid(Squad->GetTargetEnemyFormation()))
					{
						AFormationActor* Enemy = Squad->GetTargetEnemyFormation();
						FVector2D StartScreen, EnemyScreen;
						if (PC->ProjectWorldLocationToScreen(Squad->GetFormationCenter(), StartScreen) &&
							PC->ProjectWorldLocationToScreen(Enemy->GetFormationCenter(), EnemyScreen))
						{
							const FLinearColor RedAttackColor(1.0f, 0.15f, 0.15f, 0.95f);
							FCanvasLineItem AttackArrow(StartScreen, EnemyScreen);
							AttackArrow.SetColor(RedAttackColor);
							AttackArrow.LineThickness = 2.5f;
							Canvas->DrawItem(AttackArrow);

							DrawFormationGroundBox(PC, Enemy, RedAttackColor, 2.0f);
						}
					}
					// 4b. Squad is Moving -> Draw Destination Grid + Arrow
					else if (Squad->IsMoving())
					{
						const FVector TargetLoc = Squad->GetTargetDestination();
						const FVector TargetFacing = Squad->GetFinalDesiredFacing();
						const int32 Width = Squad->GetFormationWidth();

						TArray<FFormationSlot> DestSlots;
						UFormationSubsystem::CalculateSlotLayout(
							Squad->GetAliveCount(),
							Width,
							Squad->GetSoldierSpacing(),
							TargetLoc,
							TargetFacing,
							DestSlots
						);

						for (const FFormationSlot& Slot : DestSlots)
						{
							FVector2D ScreenPos;
							if (PC->ProjectWorldLocationToScreen(Slot.WorldPosition, ScreenPos))
							{
								FCanvasTileItem Dot(ScreenPos - FVector2D(DotSize * 0.5f, DotSize * 0.5f), FVector2D(DotSize, DotSize), SpaceGridColor);
								Dot.BlendMode = SE_BLEND_Translucent;
								Canvas->DrawItem(Dot);
							}
						}

						// Draw movement path arrow
						FVector2D StartScreen, EndScreen;
						if (PC->ProjectWorldLocationToScreen(Squad->GetFormationCenter(), StartScreen) &&
							PC->ProjectWorldLocationToScreen(TargetLoc, EndScreen))
						{
							FCanvasLineItem MoveArrow(StartScreen, EndScreen);
							MoveArrow.SetColor(FLinearColor(1.0f, 0.9f, 0.2f, 0.75f));
							MoveArrow.LineThickness = 1.5f;
							Canvas->DrawItem(MoveArrow);

							FVector2D FacingEndScreen;
							if (PC->ProjectWorldLocationToScreen(TargetLoc + TargetFacing * 250.0f, FacingEndScreen))
							{
								FCanvasLineItem FacingArrow(EndScreen, FacingEndScreen);
								FacingArrow.SetColor(FLinearColor(0.2f, 1.0f, 0.4f, 0.9f));
								FacingArrow.LineThickness = 2.0f;
								Canvas->DrawItem(FacingArrow);
							}
						}
					}
					else
					{
						// Idle Squad -> Draw current tactical slot dots directly on battlefield
						const FVector CurrentLoc = Squad->GetActorLocation();
						const FVector CurrentFacing = Squad->GetFacingDirection();
						const int32 Width = Squad->GetFormationWidth();

						TArray<FFormationSlot> CurrentSlots;
						UFormationSubsystem::CalculateSlotLayout(
							Squad->GetAliveCount(),
							Width,
							Squad->GetSoldierSpacing(),
							CurrentLoc,
							CurrentFacing,
							CurrentSlots
						);

						for (const FFormationSlot& Slot : CurrentSlots)
						{
							FVector2D ScreenPos;
							if (PC->ProjectWorldLocationToScreen(Slot.WorldPosition, ScreenPos))
							{
								FCanvasTileItem Dot(ScreenPos - FVector2D(DotSize * 0.5f, DotSize * 0.5f), FVector2D(DotSize, DotSize), SpaceGridColor);
								Dot.BlendMode = SE_BLEND_Translucent;
								Canvas->DrawItem(Dot);
							}
						}
					}
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

void ATWHUD::DrawFormationGroundBox(
	ATWPlayerController* PC,
	AFormationActor* Formation,
	const FLinearColor& Color,
	float Thickness)
{
	if (!PC || !Canvas || !Formation || Formation->GetAliveCount() <= 0)
	{
		return;
	}

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
		FCanvasLineItem L1(S1, S2); L1.SetColor(Color); L1.LineThickness = Thickness; Canvas->DrawItem(L1);
		FCanvasLineItem L2(S2, S3); L2.SetColor(Color); L2.LineThickness = Thickness; Canvas->DrawItem(L2);
		FCanvasLineItem L3(S3, S4); L3.SetColor(Color); L3.LineThickness = Thickness; Canvas->DrawItem(L3);
		FCanvasLineItem L4(S4, S1); L4.SetColor(Color); L4.LineThickness = Thickness; Canvas->DrawItem(L4);
	}
}
