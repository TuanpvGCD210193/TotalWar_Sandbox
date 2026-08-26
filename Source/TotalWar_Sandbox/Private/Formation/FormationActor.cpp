// Copyright Total War Sandbox. All Rights Reserved.

#include "TotalWar_Sandbox/Public/Formation/FormationActor.h"
#include "TotalWar_Sandbox/Public/Formation/FormationSlotComponent.h"
#include "TotalWar_Sandbox/Public/Formation/SoldierVisualComponent.h"
#include "TotalWar_Sandbox/Public/Formation/FormationMovementComponent.h"
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
	MovementComponent = CreateDefaultSubobject<UFormationMovementComponent>(TEXT("MovementComponent"));

	// Default identity values
	TeamID = ETeamID::Blue;
	FormationID = INDEX_NONE;
	FormationState = EFormationState::Idle;
	FacingDirection = FVector::ForwardVector;
	InitialEntityCountOverride = 0;
	bIsSelected = false;
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

	// Auto-initialize if UnitDataAsset was assigned in Editor
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

	// Delegate movement physics, combat steering & visuals to MovementComponent
	if (MovementComponent)
	{
		if (MovementComponent->IsMoving() || FormationState == EFormationState::Engage || FormationState == EFormationState::Charge)
		{
			MovementComponent->UpdateMovement(DeltaTime, SoldierEntities, SlotComponent, VisualComponent);
		}
	}

	// Always synchronize living soldier transforms to GPU Instanced Static Mesh
	SyncVisualTransforms();
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

	// 2. Setup Movement Component Settings
	MovementComponent->InitializeMovement(this, UnitDataAsset->GetSpeed());

	// 3. Setup Visual Component with editor-assigned Mesh & Materials
	VisualComponent->InitializeVisualComponent(RootComponent, TeamID, SoldierMesh, BlueTeamMaterial, RedTeamMaterial);

	const int32 TargetCount = (InCount > 0) ? InCount : UnitDataAsset->GetDefaultEntityCount();

	// 4. Build Initial Slots
	SlotComponent->RebuildSlots(TargetCount, GetActorLocation(), FacingDirection);

	// 5. Create Soldier Entities at initial slot locations
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

	// 6. Direct Sequential Assignment for initial spawn
	SlotComponent->AssignSoldiersSequentially(SoldierEntities);

	// 7. Build Initial Visual Instances
	VisualComponent->RebuildInstances(SoldierEntities);

	FormationState = EFormationState::Idle;
}

void AFormationActor::ExecuteCommand(const FFormationCommand& Command)
{
	switch (Command.CommandType)
	{
	case EFormationCommandType::Move:
	case EFormationCommandType::PlacementMove:
	case EFormationCommandType::Attack:
	{
		if (MovementComponent)
		{
			MovementComponent->ExecuteMoveCommand(Command, SoldierEntities, SlotComponent);
		}
		break;
	}
	default:
		break;
	}
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
		if (VisualComponent)
		{
			VisualComponent->ClearAllInstances();
		}
		return;
	}

	// Calculate actual in-place centroid of surviving soldiers
	FVector ActualCentroid = FVector::ZeroVector;
	for (const FSoldierEntity& S : SoldierEntities)
	{
		if (S.IsAlive())
		{
			ActualCentroid += S.Position;
		}
	}
	ActualCentroid /= static_cast<float>(AliveCount);
	SetActorLocation(ActualCentroid);

	// Rebuild slots at current in-place centroid
	SlotComponent->RebuildSlots(AliveCount, ActualCentroid, FacingDirection);

	// Reassign living soldiers to closest slots (Greedy matching)
	SlotComponent->ReassignSoldiersToClosestSlots(SoldierEntities);

	// Only snap positions if strictly in Idle state (never during combat or movement)
	if (!IsMoving() && FormationState == EFormationState::Idle)
	{
		for (int32 i = 0; i < SoldierEntities.Num(); ++i)
		{
			if (SoldierEntities[i].IsAlive())
			{
				SoldierEntities[i].Position = SlotComponent->GetSlotWorldPosition(SoldierEntities[i].SlotIndex);
				if (!FacingDirection.IsNearlyZero())
				{
					SoldierEntities[i].Rotation = FacingDirection.Rotation();
				}
			}
		}
	}

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

	// 2. Remove soldier entity
	SoldierEntities.RemoveAt(SoldierIndex, 1, false);

	// 3. Directly rebuild visual instances to guarantee 0 ghost instances
	if (VisualComponent)
	{
		VisualComponent->RebuildInstances(SoldierEntities);
	}

	// 4. Only regroup slots if in Idle state (never disrupt active Combat Envelope)
	if (FormationState == EFormationState::Idle)
	{
		RebuildFormationAndRegroup();
	}
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

void AFormationActor::SetSelected(bool bInSelected)
{
	bIsSelected = bInSelected;
}

bool AFormationActor::IsMoving() const
{
	return MovementComponent ? MovementComponent->IsMoving() : false;
}

const FVector& AFormationActor::GetTargetDestination() const
{
	return MovementComponent ? MovementComponent->GetTargetDestination() : FVector::ZeroVector;
}

const FVector& AFormationActor::GetFinalDesiredFacing() const
{
	return MovementComponent ? MovementComponent->GetFinalDesiredFacing() : FacingDirection;
}

AFormationActor* AFormationActor::GetTargetEnemyFormation() const
{
	return MovementComponent ? MovementComponent->GetTargetEnemyFormation() : nullptr;
}

bool AFormationActor::IsAttacking() const
{
	return MovementComponent ? MovementComponent->IsAttacking() : false;
}

int32 AFormationActor::GetFormationWidth() const
{
	return SlotComponent ? SlotComponent->GetFormationWidth() : TWConstants::DefaultFormationWidth;
}

float AFormationActor::GetSoldierSpacing() const
{
	return SlotComponent ? SlotComponent->GetSoldierSpacing() : TWConstants::DefaultSoldierSpacing;
}

void AFormationActor::SetFormationWidth(int32 NewWidth)
{
	if (SlotComponent)
	{
		SlotComponent->SetFormationWidth(NewWidth);
	}
}

void AFormationActor::GetFormationGroundBounds(FVector& OutCenter, FVector& OutExtents, FRotator& OutRotation) const
{
	if (SlotComponent)
	{
		const bool bMoving = MovementComponent ? MovementComponent->IsMoving() : false;
		SlotComponent->CalculateFormationGroundBounds(
			SoldierEntities,
			bMoving,
			FormationState,
			GetActorLocation(),
			FacingDirection,
			OutCenter,
			OutExtents,
			OutRotation
		);
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
