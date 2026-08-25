// Copyright Total War Sandbox. All Rights Reserved.

#include "TotalWar_Sandbox/Public/Formation/FormationActor.h"
#include "TotalWar_Sandbox/Public/Formation/FormationSlotComponent.h"
#include "TotalWar_Sandbox/Public/Formation/SoldierVisualComponent.h"
#include "TotalWar_Sandbox/Public/Data/UnitDataAsset.h"
#include "TotalWar_Sandbox/Public/Systems/FormationSubsystem.h"
#include "Components/SceneComponent.h"

AFormationActor::AFormationActor()
{
	PrimaryActorTick.bCanEverTick = true;

	// 1. Root Component
	TransformRoot = CreateDefaultSubobject<USceneComponent>(TEXT("FormationRoot"));
	RootComponent = TransformRoot;

	// 2. Attach Specialized Sub-Components (SRP)
	SlotComponent = CreateDefaultSubobject<UFormationSlotComponent>(TEXT("SlotComponent"));
	VisualComponent = CreateDefaultSubobject<USoldierVisualComponent>(TEXT("VisualComponent"));

	// Default values
	TeamID = ETeamID::Blue;
	FormationID = INDEX_NONE;
	FormationState = EFormationState::Idle;
	FacingDirection = FVector::ForwardVector;
	InitialEntityCountOverride = 0;
}

void AFormationActor::BeginPlay()
{
	Super::BeginPlay();

	// Auto-register with World Formation Subsystem
	if (UWorld* World = GetWorld())
	{
		if (UFormationSubsystem* FormationSubsystem = World->GetSubsystem<UFormationSubsystem>())
		{
			FormationSubsystem->RegisterFormation(this);
		}
	}

	// If a UnitDataAsset was assigned in Editor, initialize immediately
	if (UnitDataAsset)
	{
		const int32 Count = (InitialEntityCountOverride > 0) ? InitialEntityCountOverride : UnitDataAsset->GetDefaultEntityCount();
		InitializeFormation(UnitDataAsset, TeamID, Count);
	}
}

void AFormationActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		if (UFormationSubsystem* FormationSubsystem = World->GetSubsystem<UFormationSubsystem>())
		{
			FormationSubsystem->UnregisterFormation(this);
		}
	}

	Super::EndPlay(EndPlayReason);
}

void AFormationActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AFormationActor::InitializeFormation(UUnitDataAsset* InDataAsset, ETeamID InTeam, int32 InCount)
{
	if (!InDataAsset)
	{
		return;
	}

	UnitDataAsset = InDataAsset;
	TeamID = InTeam;
	FacingDirection = GetActorForwardVector();

	// 1. Setup Slot Component Settings
	SlotComponent->SetupFormationSettings(
		UnitDataAsset->GetDefaultFormationWidth(),
		UnitDataAsset->GetMinFormationWidth(),
		UnitDataAsset->GetMaxFormationWidth(),
		UnitDataAsset->GetSoldierSpacing()
	);

	// 2. Setup Visual Component with editor-assigned Mesh & Materials
	VisualComponent->InitializeVisualComponent(RootComponent, TeamID, SoldierMesh, BlueTeamMaterial, RedTeamMaterial);

	const int32 TargetCount = (InCount > 0) ? InCount : UnitDataAsset->GetDefaultEntityCount();

	// 3. Build Initial Slots
	SlotComponent->RebuildSlots(TargetCount, GetActorLocation(), FacingDirection);

	// 4. Create Soldier Entities at initial slot locations
	SoldierEntities.Empty(TargetCount);
	const FRotator InitialRotation = FacingDirection.Rotation();
	const float MaxHP = UnitDataAsset->GetHealthPerSoldier();

	for (int32 i = 0; i < TargetCount; ++i)
	{
		const FVector SpawnWorldPos = SlotComponent->GetSlotWorldPosition(i);

		FSoldierEntity NewSoldier(SpawnWorldPos, InitialRotation, MaxHP);
		NewSoldier.SlotIndex = i;
		NewSoldier.VisualInstanceIndex = i;
		SoldierEntities.Add(NewSoldier);
	}

	// 5. Direct Sequential Assignment for initial spawn
	SlotComponent->AssignSoldiersSequentially(SoldierEntities);

	// 6. Build Initial Visual Instances
	VisualComponent->RebuildInstances(SoldierEntities);

	FormationState = EFormationState::Idle;
}

void AFormationActor::RebuildFormationAndRegroup(int32 NewWidth)
{
	if (NewWidth > 0)
	{
		SlotComponent->SetFormationWidth(NewWidth);
	}

	const int32 AliveCount = SoldierEntities.Num();
	if (AliveCount <= 0)
	{
		return;
	}

	// Rebuild slots at current location/facing
	SlotComponent->RebuildSlots(AliveCount, GetActorLocation(), FacingDirection);

	// Reassign living soldiers to closest slots (Greedy matching)
	SlotComponent->ReassignSoldiersToClosestSlots(SoldierEntities);

	// Sync visuals
	SyncVisualTransforms();
}

void AFormationActor::SyncVisualTransforms()
{
	if (VisualComponent)
	{
		VisualComponent->SyncVisualTransforms(SoldierEntities);
	}
}

void AFormationActor::KillSoldier(int32 SoldierIndex)
{
	if (!SoldierEntities.IsValidIndex(SoldierIndex))
	{
		return;
	}

	// 1. Mark dead
	SoldierEntities[SoldierIndex].TakeDamage(999999.0f);

	// 2. Compact swap-remove from array (O(1))
	const int32 LastIndex = SoldierEntities.Num() - 1;
	const bool bWasLast = (SoldierIndex == LastIndex);

	SoldierEntities.RemoveAtSwap(SoldierIndex, 1, false);

	// Remove visual instance from ISMC
	if (VisualComponent)
	{
		VisualComponent->RemoveInstance(LastIndex);
	}

	// If we swapped a soldier from the end into SoldierIndex, update its visual index
	if (!bWasLast && SoldierEntities.IsValidIndex(SoldierIndex))
	{
		SoldierEntities[SoldierIndex].VisualInstanceIndex = SoldierIndex;
	}

	// 3. Regroup formation without dead soldier
	RebuildFormationAndRegroup();
}

void AFormationActor::SetFormationState(EFormationState NewState)
{
	FormationState = NewState;
}

void AFormationActor::SetFacingDirection(const FVector& NewFacing)
{
	if (!NewFacing.IsNearlyZero())
	{
		FacingDirection = NewFacing.GetSafeNormal2D();
		SetActorRotation(FacingDirection.Rotation());
	}
}

FSoldierEntity* AFormationActor::GetSoldier(int32 Index)
{
	if (SoldierEntities.IsValidIndex(Index))
	{
		return &SoldierEntities[Index];
	}
	return nullptr;
}

bool AFormationActor::IsSoldierAlive(int32 Index) const
{
	if (SoldierEntities.IsValidIndex(Index))
	{
		return SoldierEntities[Index].IsAlive();
	}
	return false;
}
