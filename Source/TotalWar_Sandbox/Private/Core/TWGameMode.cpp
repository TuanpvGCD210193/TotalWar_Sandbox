#include "TotalWar_Sandbox/Public/Core/TWGameMode.h"
#include "TotalWar_Sandbox/Public/Player/TWCameraPawn.h"
#include "TotalWar_Sandbox/Public/Player/TWPlayerController.h"
#include "TotalWar_Sandbox/Public/Player/TWHUD.h"

ATWGameMode::ATWGameMode()
{
	// Set default camera pawn, player controller, and HUD
	DefaultPawnClass = ATWCameraPawn::StaticClass();
	PlayerControllerClass = ATWPlayerController::StaticClass();
	HUDClass = ATWHUD::StaticClass();
}

void ATWGameMode::BeginPlay()
{
	Super::BeginPlay();

	// Additional battle session initialization will be hooked here in Phase 2
}
