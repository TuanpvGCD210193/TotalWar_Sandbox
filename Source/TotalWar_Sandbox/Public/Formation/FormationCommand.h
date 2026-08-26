// Copyright Total War Sandbox. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "TotalWar_Sandbox/Public/Core/TWTypes.h"
#include "FormationCommand.generated.h"

class AFormationActor;

/**
 * FFormationCommand — Cấu Trúc Lệnh Điều Khiển Đội Hình (Command Pattern).
 * Đóng gói mọi chỉ thị từ người chơi (Move, Attack, PlacementMove) gửi tới AFormationActor.
 * Follows Single Responsibility Principle (SRP) & Dependency Inversion Principle (DIP).
 */
USTRUCT(BlueprintType)
struct TOTALWAR_SANDBOX_API FFormationCommand
{
	GENERATED_BODY()

public:
	/** Type of tactical command issued */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Command")
	EFormationCommandType CommandType = EFormationCommandType::Move;

	/** Destination center point on the ground plane (World Space) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Command")
	FVector TargetLocation = FVector::ZeroVector;

	/** Desired facing orientation vector upon arriving at destination */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Command")
	FVector FacingDirection = FVector::ForwardVector;

	/** Desired formation width (number of columns / soldiers per rank). If INDEX_NONE, keeps current width */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Command")
	int32 TargetWidth = INDEX_NONE;

	/** Target enemy squad when CommandType == Attack */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Command")
	TObjectPtr<AFormationActor> TargetFormation = nullptr;

	/** Whether soldiers must rigidly stick to their assigned slots during march */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Command")
	bool bMaintainFormation = true;

	// ========================================================================
	// CONSTRUCTORS
	// ========================================================================

	FFormationCommand()
		: CommandType(EFormationCommandType::Move)
		, TargetLocation(FVector::ZeroVector)
		, FacingDirection(FVector::ForwardVector)
		, TargetWidth(INDEX_NONE)
		, TargetFormation(nullptr)
		, bMaintainFormation(true)
	{
	}

	/** Factory method for Quick Move Command (Right-click ground) */
	static FFormationCommand CreateMoveCommand(const FVector& InTargetLocation, const FVector& InFacingDirection, int32 InWidth = INDEX_NONE)
	{
		FFormationCommand Cmd;
		Cmd.CommandType = EFormationCommandType::Move;
		Cmd.TargetLocation = InTargetLocation;
		Cmd.FacingDirection = InFacingDirection;
		Cmd.TargetWidth = InWidth;
		Cmd.bMaintainFormation = true;
		return Cmd;
	}

	/** Factory method for Placement Move Command (Right-hold drag preview confirmation) */
	static FFormationCommand CreatePlacementCommand(const FVector& InTargetLocation, const FVector& InFacingDirection, int32 InTargetWidth)
	{
		FFormationCommand Cmd;
		Cmd.CommandType = EFormationCommandType::PlacementMove;
		Cmd.TargetLocation = InTargetLocation;
		Cmd.FacingDirection = InFacingDirection;
		Cmd.TargetWidth = InTargetWidth;
		Cmd.bMaintainFormation = true;
		return Cmd;
	}

	/** Factory method for Attack Command (Right-click enemy formation) */
	static FFormationCommand CreateAttackCommand(AFormationActor* InTargetEnemyFormation)
	{
		FFormationCommand Cmd;
		Cmd.CommandType = EFormationCommandType::Attack;
		Cmd.TargetFormation = InTargetEnemyFormation;
		Cmd.bMaintainFormation = false; // Loosened for combat engagement
		return Cmd;
	}
};
