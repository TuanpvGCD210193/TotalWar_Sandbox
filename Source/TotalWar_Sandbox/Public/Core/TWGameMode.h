// Copyright Total War Sandbox. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "TWGameMode.generated.h"

/**
 * Main Game Mode for the Total War Sandbox.
 * Configures default classes (Camera Pawn, Player Controller, HUD) and handles battle initialization.
 * Follows Single Responsibility Principle (SRP): Only manages game lifecycle and session configuration.
 */
UCLASS()
class TOTALWAR_SANDBOX_API ATWGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ATWGameMode();

protected:
	virtual void BeginPlay() override;
};
