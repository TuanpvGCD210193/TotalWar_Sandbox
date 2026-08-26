#include "TotalWar_Sandbox/Public/Systems/CombatSubsystem.h"
#include "TotalWar_Sandbox/Public/Systems/SpatialGridSubsystem.h"
#include "TotalWar_Sandbox/Public/Systems/FormationSubsystem.h"
#include "TotalWar_Sandbox/Public/Formation/FormationActor.h"
#include "TotalWar_Sandbox/Public/Formation/FormationMovementComponent.h"
#include "TotalWar_Sandbox/Public/Data/UnitDataAsset.h"
#include "TotalWar_Sandbox/Public/Entity/SoldierEntity.h"

UCombatSubsystem::UCombatSubsystem()
{
	CombatTickInterval = 0.2f;
	CombatTimer = 0.0f;
}

void UCombatSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	Collection.InitializeDependency(USpatialGridSubsystem::StaticClass());
	Collection.InitializeDependency(UFormationSubsystem::StaticClass());

	SpatialGridSubsystem = GetWorld()->GetSubsystem<USpatialGridSubsystem>();
	FormationSubsystem = GetWorld()->GetSubsystem<UFormationSubsystem>();
}

void UCombatSubsystem::Deinitialize()
{
	SpatialGridSubsystem = nullptr;
	FormationSubsystem = nullptr;

	Super::Deinitialize();
}

void UCombatSubsystem::Tick(float DeltaTime)
{
	CombatTimer += DeltaTime;
	if (CombatTimer >= CombatTickInterval)
	{
		CombatTimer = 0.0f;
		ProcessCombatTick();
	}
}

TStatId UCombatSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UCombatSubsystem, STATGROUP_Tickables);
}

void UCombatSubsystem::ProcessCombatTick()
{
	if (!SpatialGridSubsystem || !FormationSubsystem)
	{
		return;
	}

	// 1. Rebuild spatial hash grid with latest soldier positions
	SpatialGridSubsystem->RebuildGrid();

	TArray<AFormationActor*> AllFormations;
	FormationSubsystem->GetAllFormations(AllFormations);

	if (AllFormations.Num() <= 1)
	{
		return;
	}

	// 2. Evaluate melee frontline engagements across all formations
	for (AFormationActor* AttackerSquad : AllFormations)
	{
		if (!AttackerSquad || AttackerSquad->GetAliveCount() <= 0)
		{
			continue;
		}

		TArray<FSoldierEntity>& Soldiers = AttackerSquad->GetSoldierEntitiesMutable();
		const ETeamID MyTeam = AttackerSquad->GetTeamID();
		bool bAnyEngaged = false;

		for (int32 i = 0; i < Soldiers.Num(); ++i)
		{
			FSoldierEntity& Soldier = Soldiers[i];
			if (!Soldier.IsAlive())
			{
				continue;
			}

			// Query enemy soldiers within melee reach
			TArray<FSoldierHandle> NearbyEnemies;
			SpatialGridSubsystem->QueryEnemiesInRadius(
				Soldier.Position,
				TWConstants::DefaultMeleeRange,
				MyTeam,
				NearbyEnemies
			);

			if (NearbyEnemies.Num() > 0)
			{
				bAnyEngaged = true;
				Soldier.State = ESoldierState::Fighting;

				// Pick closest enemy target
				int32 BestEnemyIdx = INDEX_NONE;
				float MinDistSq = TNumericLimits<float>::Max();
				AFormationActor* DefenderSquad = nullptr;

				for (const FSoldierHandle& Handle : NearbyEnemies)
				{
					AFormationActor* TargetSquad = FormationSubsystem->GetFormationByID(Handle.FormationID);
					if (TargetSquad && TargetSquad->IsSoldierAlive(Handle.SoldierIndex))
					{
						FSoldierEntity* EnemySoldier = TargetSquad->GetSoldier(Handle.SoldierIndex);
						if (EnemySoldier)
						{
							const float DistSq = FVector::DistSquared2D(Soldier.Position, EnemySoldier->Position);
							if (DistSq < MinDistSq)
							{
								MinDistSq = DistSq;
								BestEnemyIdx = Handle.SoldierIndex;
								DefenderSquad = TargetSquad;
							}
						}
					}
				}

				if (DefenderSquad && BestEnemyIdx != INDEX_NONE)
				{
					// Face target
					FSoldierEntity* EnemySoldier = DefenderSquad->GetSoldier(BestEnemyIdx);
					if (EnemySoldier)
					{
						const FVector DirToEnemy = (EnemySoldier->Position - Soldier.Position).GetSafeNormal2D();
						if (!DirToEnemy.IsNearlyZero())
						{
							Soldier.Rotation = DirToEnemy.Rotation();
						}
					}

					// Defender squad auto-braces and engages at its current position
					if (DefenderSquad->GetFormationState() == EFormationState::Idle)
					{
						DefenderSquad->SetFormationState(EFormationState::Engage);
						const FVector DirToAttacker = (AttackerSquad->GetActorLocation() - DefenderSquad->GetActorLocation()).GetSafeNormal2D();
						if (!DirToAttacker.IsNearlyZero())
						{
							DefenderSquad->SetFacingDirection(DirToAttacker);
						}
					}

					// Resolve melee attack exchange
					ResolveMeleeAttack(AttackerSquad, i, DefenderSquad, BestEnemyIdx);
				}
			}
			else if (Soldier.State == ESoldierState::Fighting)
			{
				Soldier.State = ESoldierState::Idle;
			}
		}

		if (bAnyEngaged)
		{
			if (AttackerSquad->GetFormationState() != EFormationState::Engage)
			{
				AttackerSquad->SetFormationState(EFormationState::Engage);
			}
		}
		else
		{
			// Post-Combat Regroup: Only return to Idle when the targeted enemy squad is completely destroyed or gone
			AFormationActor* TargetEnemy = AttackerSquad->GetTargetEnemyFormation();
			if (AttackerSquad->GetFormationState() == EFormationState::Engage && (!TargetEnemy || TargetEnemy->GetAliveCount() <= 0))
			{
				AttackerSquad->SetFormationState(EFormationState::Idle);
				AttackerSquad->RebuildFormationAndRegroup();
			}
		}
	}
}

void UCombatSubsystem::ResolveMeleeAttack(
	AFormationActor* AttackerFormation,
	int32 AttackerSoldierIndex,
	AFormationActor* DefenderFormation,
	int32 DefenderSoldierIndex)
{
	if (!AttackerFormation || !DefenderFormation)
	{
		return;
	}

	UUnitDataAsset* AttackerData = AttackerFormation->GetUnitDataAsset();
	UUnitDataAsset* DefenderData = DefenderFormation->GetUnitDataAsset();

	if (!AttackerData || !DefenderData)
	{
		return;
	}

	FSoldierEntity* DefenderSoldier = DefenderFormation->GetSoldier(DefenderSoldierIndex);
	if (!DefenderSoldier || !DefenderSoldier->IsAlive())
	{
		return;
	}

	// 1. Charge Bonus calculation (Linearly decays from 1.0 -> 0.0 over 10s after impact)
	UFormationMovementComponent* MoveComp = AttackerFormation->GetMovementComponent();
	const float ChargeRatio = MoveComp ? MoveComp->GetChargeBonusRatio() : 0.0f;
	const float ActiveChargeBonus = AttackerData->GetChargeBonus() * ChargeRatio;

	// 2. Total War Hit Chance Formula with Charge Bonus
	const float BaseHitChance = static_cast<float>(TWConstants::BaseHitChance); // 35%
	const float MeleeAttack = AttackerData->GetMeleeAttack() + ActiveChargeBonus;
	const float MeleeDefense = DefenderData->GetMeleeDefence();

	const float RawHitChance = BaseHitChance + (MeleeAttack - MeleeDefense);
	const float FinalHitChance = FMath::Clamp(RawHitChance, static_cast<float>(TWConstants::MinHitChance), static_cast<float>(TWConstants::MaxHitChance)); // 8% to 90%

	// Roll Hit D100
	const float HitRoll = FMath::FRandRange(0.0f, 100.0f);
	if (HitRoll <= FinalHitChance)
	{
		// 3. Total War Armor Reduction & Damage Formula with Charge Bonus
		const float Armor = DefenderData->GetArmour();
		const float ArmorRoll = FMath::FRandRange(Armor * 0.5f, Armor);
		const float DamageReductionPercent = FMath::Clamp(ArmorRoll / 100.0f, 0.0f, 1.0f);

		const float BaseDamage = AttackerData->GetBaseDamage() + (ActiveChargeBonus * 0.5f);
		const float ArmorPiercingDamage = AttackerData->GetArmourPiercingDamage() + (ActiveChargeBonus * 0.5f);

		const float EffectiveBaseDamage = BaseDamage * (1.0f - DamageReductionPercent);
		const float TotalDamage = FMath::Max(1.0f, EffectiveBaseDamage) + ArmorPiercingDamage;

		// 4. Apply Damage
		DefenderSoldier->TakeDamage(TotalDamage);

		// 5. Check for Death
		if (!DefenderSoldier->IsAlive())
		{
			DefenderFormation->KillSoldier(DefenderSoldierIndex);
		}
	}
}
