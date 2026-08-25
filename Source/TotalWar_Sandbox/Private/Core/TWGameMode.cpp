// Copyright Total War Sandbox. All Rights Reserved.

#include "TotalWar_Sandbox/Public/Core/TWGameMode.h"
#include "TotalWar_Sandbox/Public/Player/TWCameraPawn.h"

ATWGameMode::ATWGameMode()
{
	// Set default camera pawn
	DefaultPawnClass = ATWCameraPawn::StaticClass();
}

void ATWGameMode::BeginPlay()
{
	Super::BeginPlay();

	// Additional battle session initialization will be hooked here in Phase 2
}
