#include "TotalWar_Sandbox/Public/Formation/FormationMovementComponent.h"
#include "TotalWar_Sandbox/Public/Formation/FormationActor.h"
#include "TotalWar_Sandbox/Public/Formation/FormationSlotComponent.h"
#include "TotalWar_Sandbox/Public/Formation/SoldierVisualComponent.h"
#include "TotalWar_Sandbox/Public/Systems/SpatialGridSubsystem.h"
#include "TotalWar_Sandbox/Public/Systems/FormationSubsystem.h"
#include "TotalWar_Sandbox/Public/Data/UnitDataAsset.h"
#include "TotalWar_Sandbox/Public/Entity/SoldierEntity.h"

UFormationMovementComponent::UFormationMovementComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	BaseMoveSpeed = TWConstants::DefaultMoveSpeed;
	CurrentSpeed = TWConstants::DefaultMoveSpeed;
	ChargeBonusTimer = 0.0f;
	bIsMoving = false;
	bHasImpacted = false;
	TargetDestination = FVector::ZeroVector;
	FinalDesiredFacing = FVector::ForwardVector;
}

void UFormationMovementComponent::InitializeMovement(AFormationActor* InOwnerActor, float InMoveSpeed)
{
	OwnerFormation = InOwnerActor;
	BaseMoveSpeed = (InMoveSpeed > 0.0f) ? InMoveSpeed : TWConstants::DefaultMoveSpeed;
	CurrentSpeed = BaseMoveSpeed;
	ChargeBonusTimer = 0.0f;
	bIsMoving = false;
	bHasImpacted = false;
}

void UFormationMovementComponent::ExecuteMoveCommand(
	const FFormationCommand& Command,
	TArray<FSoldierEntity>& Soldiers,
	UFormationSlotComponent* SlotComponent)
{
	if (!OwnerFormation || !SlotComponent)
	{
		return;
	}

	bHasImpacted = false;

	if (Command.CommandType == EFormationCommandType::Attack)
	{
		TargetEnemyFormation = Command.TargetFormation;
		if (IsValid(TargetEnemyFormation))
		{
			const FVector EnemyLoc = TargetEnemyFormation->GetActorLocation();
			const FVector MyLoc = OwnerFormation->GetActorLocation();
			FVector DirToEnemy = (EnemyLoc - MyLoc).GetSafeNormal2D();
			if (DirToEnemy.IsNearlyZero())
			{
				DirToEnemy = OwnerFormation->GetFacingDirection();
			}

			FinalDesiredFacing = DirToEnemy;
			TargetDestination = EnemyLoc;
		}
		else
		{
			TargetEnemyFormation = nullptr;
			TargetDestination = Command.TargetLocation;
			FinalDesiredFacing = (!Command.FacingDirection.IsNearlyZero()) ? Command.FacingDirection.GetSafeNormal2D() : OwnerFormation->GetFacingDirection();
		}
	}
	else
	{
		TargetEnemyFormation = nullptr;
		TargetDestination = Command.TargetLocation;
		FinalDesiredFacing = (!Command.FacingDirection.IsNearlyZero())
			? Command.FacingDirection.GetSafeNormal2D()
			: OwnerFormation->GetFacingDirection();
	}

	// 1. Update formation width to the new target shape
	if (Command.TargetWidth > 0)
	{
		SlotComponent->SetFormationWidth(Command.TargetWidth);
	}

	// 2. Build destination slots ONCE towards destination
	SlotComponent->RebuildSlots(Soldiers.Num(), TargetDestination, FinalDesiredFacing);

	// 3. Total War Rank & File Projection: Map soldiers cleanly to their closest destination slot
	SlotComponent->AssignSlotsByRankAndFileProjection(
		Soldiers,
		OwnerFormation->GetActorLocation(),
		FinalDesiredFacing
	);

	CurrentSpeed = BaseMoveSpeed;
	bIsMoving = true;
	OwnerFormation->SetFormationState(EFormationState::March);
}

void UFormationMovementComponent::UpdateMovement(
	float DeltaTime,
	TArray<FSoldierEntity>& Soldiers,
	UFormationSlotComponent* SlotComponent,
	USoldierVisualComponent* VisualComponent)
{
	// 1. Tick Charge Bonus Decay Timer
	if (ChargeBonusTimer > 0.0f)
	{
		ChargeBonusTimer = FMath::Max(0.0f, ChargeBonusTimer - DeltaTime);
	}

	const bool bIsEngaged = OwnerFormation && (OwnerFormation->GetFormationState() == EFormationState::Engage);
	if ((!bIsMoving && !bIsEngaged) || !OwnerFormation || !SlotComponent)
	{
		return;
	}

	UWorld* World = OwnerFormation->GetWorld();
	USpatialGridSubsystem* SpatialGrid = World ? World->GetSubsystem<USpatialGridSubsystem>() : nullptr;
	UFormationSubsystem* FormationSubsystem = World ? World->GetSubsystem<UFormationSubsystem>() : nullptr;

	const FVector MyLoc = OwnerFormation->GetActorLocation();

	// 2. Dynamically check Charge & Melee Impact transitions during Attack command
	if (IsValid(TargetEnemyFormation))
	{
		const FVector EnemyLoc = TargetEnemyFormation->GetActorLocation();
		const float DistToEnemy = FVector::Dist2D(MyLoc, EnemyLoc);
		const float DistToFrontline = FVector::Dist2D(MyLoc, TargetDestination);

		const int32 MyCols = FMath::Clamp(SlotComponent->GetFormationWidth(), TWConstants::MinFormationWidth, TWConstants::MaxFormationWidth);
		const int32 MyRows = FMath::Max(1, FMath::CeilToInt(static_cast<float>(Soldiers.Num()) / static_cast<float>(MyCols)));
		const int32 EnemyCols = FMath::Clamp(TargetEnemyFormation->GetFormationWidth(), TWConstants::MinFormationWidth, TWConstants::MaxFormationWidth);
		const int32 EnemyRows = FMath::Max(1, FMath::CeilToInt(static_cast<float>(TargetEnemyFormation->GetAliveCount()) / static_cast<float>(EnemyCols)));

		const float Spacing = SlotComponent->GetSoldierSpacing();
		const float MyHalfDepth = (MyRows * Spacing) * 0.5f;
		const float EnemyHalfDepth = (EnemyRows * Spacing) * 0.5f;

		// Case 1: Enter Charge Sprint when within 30m of enemy
		const float ChargeTriggerDistance = TWConstants::MinChargeDistance * 10.0f; // 3000cm = 30m
		if (DistToEnemy <= ChargeTriggerDistance && OwnerFormation->GetFormationState() == EFormationState::March)
		{
			OwnerFormation->SetFormationState(EFormationState::Charge);
			CurrentSpeed = BaseMoveSpeed * TWConstants::ChargeSpeedMultiplier; // +80% Speed Boost!
		}

		// Case 2: Frontline Physical Melee Impact (Front rows touch at < 120cm)
		if (DistToFrontline <= 120.0f || DistToEnemy <= 150.0f)
		{
			if (!bHasImpacted)
			{
				bHasImpacted = true;
				ChargeBonusTimer = TWConstants::ChargeDecayDuration; // Start 10s countdown!

				// Mass Ratio & Knockback
				UUnitDataAsset* MyData = OwnerFormation->GetUnitDataAsset();
				UUnitDataAsset* EnemyData = TargetEnemyFormation->GetUnitDataAsset();

				const float MyMass = MyData ? MyData->GetMass() : 100.0f;
				const float EnemyMass = EnemyData ? EnemyData->GetMass() : 100.0f;
				const float MassRatio = FMath::Clamp(MyMass / FMath::Max(10.0f, EnemyMass), 0.5f, 4.0f);

				// Knockback impulse vector on defending frontline (2D only)
				const FVector KnockbackDir = (!FinalDesiredFacing.IsNearlyZero()) ? FinalDesiredFacing.GetSafeNormal2D() : OwnerFormation->GetFacingDirection();
				const float KnockbackDist = 60.0f * MassRatio;

				TArray<FSoldierEntity>& EnemySoldiers = TargetEnemyFormation->GetSoldierEntitiesMutable();
				for (int32 k = 0; k < FMath::Min(20, EnemySoldiers.Num()); ++k)
				{
					if (EnemySoldiers[k].IsAlive())
					{
						EnemySoldiers[k].Position += KnockbackDir * (KnockbackDist * FMath::FRandRange(0.7f, 1.2f));
					}
				}
			}

			if (OwnerFormation)
			{
				OwnerFormation->SetFormationState(EFormationState::Engage);
			}
			if (IsValid(TargetEnemyFormation))
			{
				TargetEnemyFormation->SetFormationState(EFormationState::Engage);
			}
		}
	}

	// 3. Terrain Height Detection: Sample ground height once per squad
	float GroundZ = OwnerFormation->GetActorLocation().Z;
	if (World)
	{
		FHitResult GroundHit;
		const FVector TraceStart = OwnerFormation->GetActorLocation() + FVector(0.0f, 0.0f, 500.0f);
		const FVector TraceEnd = OwnerFormation->GetActorLocation() - FVector(0.0f, 0.0f, 1000.0f);
		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(OwnerFormation);

		if (World->LineTraceSingleByChannel(GroundHit, TraceStart, TraceEnd, ECC_Visibility, QueryParams))
		{
			GroundZ = GroundHit.ImpactPoint.Z;
		}
	}

	bool bAllSoldiersArrived = true;
	const float Spacing = SlotComponent->GetSoldierSpacing();
	const float PersonalRadius = 38.0f;
	const float MinDist = PersonalRadius * 2.0f;
	const float MinDistSq = MinDist * MinDist;
	const ETeamID MyTeam = OwnerFormation->GetTeamID();

	// 4. Update each soldier's continuous kinematic velocity (Zero Snapping!)
	for (int32 i = 0; i < Soldiers.Num(); ++i)
	{
		FSoldierEntity& Soldier = Soldiers[i];
		if (!Soldier.IsAlive())
		{
			continue;
		}

		// Strictly adhere to ground height (0% flying or sinking)
		Soldier.Position.Z = GroundZ;

		const FVector TargetSlotPos = SlotComponent->GetSlotWorldPosition(Soldier.SlotIndex);
		const FVector ToSlot = (TargetSlotPos - Soldier.Position);
		const float DistToSlot = ToSlot.Size2D();

		// 4a. Intra-Squad Soft Separation (Allies)
		FVector SeparationForce = FVector::ZeroVector;
		for (int32 j = 0; j < Soldiers.Num(); ++j)
		{
			if (i == j || !Soldiers[j].IsAlive())
			{
				continue;
			}

			const FVector Diff = (Soldier.Position - Soldiers[j].Position).GetSafeNormal2D();
			const float DistSq = FVector::DistSquared2D(Soldier.Position, Soldiers[j].Position);

			if (DistSq > 0.001f && DistSq < MinDistSq)
			{
				const float Dist = FMath::Sqrt(DistSq);
				const float PushRatio = (MinDist - Dist) / MinDist;
				SeparationForce += Diff * (PushRatio * 75.0f);
			}
		}

		// 4b. Obstacle Avoidance (Rocks, trees, buildings)
		if (SpatialGrid)
		{
			SeparationForce += SpatialGrid->CalculateObstacleRepulsion(Soldier.Position, PersonalRadius);

			// 4c. Inter-Squad Hard Collision (Enemies) — Soft barrier at 50cm allowing melee reach
			TArray<FSoldierHandle> NearbyEnemies;
			SpatialGrid->QueryEnemiesInRadius(Soldier.Position, 65.0f, MyTeam, NearbyEnemies);

			for (const FSoldierHandle& EnemyHandle : NearbyEnemies)
			{
				if (FormationSubsystem)
				{
					AFormationActor* EnemySquad = FormationSubsystem->GetFormationByID(EnemyHandle.FormationID);
					if (EnemySquad && EnemySquad->IsSoldierAlive(EnemyHandle.SoldierIndex))
					{
						FSoldierEntity* EnemySoldier = EnemySquad->GetSoldier(EnemyHandle.SoldierIndex);
						if (EnemySoldier)
						{
							const FVector EnemyDiff = (Soldier.Position - EnemySoldier->Position).GetSafeNormal2D();
							const float EnemyDistSq = FVector::DistSquared2D(Soldier.Position, EnemySoldier->Position);
							const float EnemyMinDist = 50.0f; // 50cm contact boundary

							if (EnemyDistSq > 0.001f && EnemyDistSq < (EnemyMinDist * EnemyMinDist))
							{
								const float EnemyDist = FMath::Sqrt(EnemyDistSq);
								const float PushRatio = (EnemyMinDist - EnemyDist) / EnemyMinDist;
								SeparationForce += EnemyDiff * (PushRatio * 60.0f);
							}
						}
					}
				}
			}
		}

		SeparationForce.Z = 0.0f; // Pure 2D horizontal force

		FVector DesiredVelocity = FVector::ZeroVector;
		FRotator TargetRot = FinalDesiredFacing.Rotation();

		if (OwnerFormation->GetFormationState() == EFormationState::Engage)
		{
			// ================================================================
			// COMBAT ENVELOPE AGENT-LEVEL STEERING:
			// Each soldier finds nearest enemy within 2.5m and maintains melee engagement
			// ================================================================
			TArray<FSoldierHandle> LocalEnemies;
			if (SpatialGrid)
			{
				SpatialGrid->QueryEnemiesInRadius(Soldier.Position, 250.0f, MyTeam, LocalEnemies);
			}

			if (LocalEnemies.Num() > 0 && FormationSubsystem)
			{
				float ClosestEnemyDistSq = TNumericLimits<float>::Max();
				FSoldierEntity* ClosestEnemy = nullptr;

				for (const FSoldierHandle& Handle : LocalEnemies)
				{
					AFormationActor* TargetSquad = FormationSubsystem->GetFormationByID(Handle.FormationID);
					if (TargetSquad && TargetSquad->IsSoldierAlive(Handle.SoldierIndex))
					{
						FSoldierEntity* Enemy = TargetSquad->GetSoldier(Handle.SoldierIndex);
						if (Enemy)
						{
							const float DistSq = FVector::DistSquared2D(Soldier.Position, Enemy->Position);
							if (DistSq < ClosestEnemyDistSq)
							{
								ClosestEnemyDistSq = DistSq;
								ClosestEnemy = Enemy;
							}
						}
					}
				}

				if (ClosestEnemy)
				{
					const float EnemyDist = FMath::Sqrt(ClosestEnemyDistSq);
					const FVector DirToEnemy = (ClosestEnemy->Position - Soldier.Position).GetSafeNormal2D();
					TargetRot = DirToEnemy.Rotation();

					if (EnemyDist > 85.0f)
					{
						// Step towards enemy to close melee weapon reach
						DesiredVelocity = DirToEnemy * (BaseMoveSpeed * 0.6f);
						Soldier.State = ESoldierState::Moving;
					}
					else
					{
						// In weapon strike reach
						DesiredVelocity = FVector::ZeroVector;
						Soldier.State = ESoldierState::Fighting;
					}
				}
			}
			else
			{
				// Rear-rank reinforcement: push forward in formation facing to close frontline gaps
				DesiredVelocity = FinalDesiredFacing * (BaseMoveSpeed * 0.45f);
				Soldier.State = ESoldierState::Moving;
				TargetRot = FinalDesiredFacing.Rotation();
			}
		}
		else
		{
			// ================================================================
			// RIGID SLOT NAVIGATION (March / Charge)
			// ================================================================
			if (DistToSlot > TWConstants::SlotArrivalThreshold)
			{
				bAllSoldiersArrived = false;
				DesiredVelocity = ToSlot.GetSafeNormal2D() * CurrentSpeed;
			}
			else if (DistToSlot > 1.0f)
			{
				// Smooth braking zone within arrival threshold
				DesiredVelocity = (ToSlot / TWConstants::SlotArrivalThreshold) * (CurrentSpeed * 0.5f);
			}

			if (DistToSlot > Spacing * 1.2f && !DesiredVelocity.IsNearlyZero())
			{
				TargetRot = DesiredVelocity.Rotation();
			}
			else if (!FinalDesiredFacing.IsNearlyZero())
			{
				TargetRot = FinalDesiredFacing.Rotation();
			}
			else
			{
				TargetRot = OwnerFormation->GetActorRotation();
			}

			if (OwnerFormation->GetFormationState() == EFormationState::Charge)
			{
				Soldier.State = ESoldierState::Charging;
			}
			else if (DistToSlot > TWConstants::SlotArrivalThreshold)
			{
				Soldier.State = ESoldierState::Moving;
			}
			else
			{
				Soldier.State = ESoldierState::Idle;
			}
		}

		const FVector CombinedVelocity = (DesiredVelocity + SeparationForce).GetClampedToMaxSize2D(CurrentSpeed * 1.35f);
		Soldier.Position += CombinedVelocity * DeltaTime;
		Soldier.Rotation = FMath::RInterpTo(Soldier.Rotation, TargetRot, DeltaTime, 12.0f);
	}

	// 4. Batch update visual transforms
	if (VisualComponent)
	{
		VisualComponent->SyncVisualTransforms(Soldiers);
	}

	// 5. Arrival Confirmation for Normal Move (Ground Move)
	if (bAllSoldiersArrived && !IsValid(TargetEnemyFormation))
	{
		StopMovement(Soldiers, SlotComponent, VisualComponent);
		return;
	}
}

void UFormationMovementComponent::StopMovement(
	TArray<FSoldierEntity>& Soldiers,
	UFormationSlotComponent* SlotComponent,
	USoldierVisualComponent* VisualComponent)
{
	bIsMoving = false;

	if (OwnerFormation && OwnerFormation->GetFormationState() != EFormationState::Engage)
	{
		OwnerFormation->SetFormationState(EFormationState::Idle);
	}
}
