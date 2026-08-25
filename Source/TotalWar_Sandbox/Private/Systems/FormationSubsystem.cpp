// Copyright Total War Sandbox. All Rights Reserved.

#include "TotalWar_Sandbox/Public/Systems/FormationSubsystem.h"
#include "TotalWar_Sandbox/Public/Formation/FormationActor.h"

void UFormationSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	RegisteredFormations.Empty();
	NextFormationID = 1;
}

void UFormationSubsystem::Deinitialize()
{
	RegisteredFormations.Empty();
	Super::Deinitialize();
}

void UFormationSubsystem::RegisterFormation(AFormationActor* Formation)
{
	if (!Formation)
	{
		return;
	}

	const int32 AssignedID = NextFormationID++;
	Formation->SetFormationID(AssignedID);
	RegisteredFormations.Add(AssignedID, Formation);
}

void UFormationSubsystem::UnregisterFormation(AFormationActor* Formation)
{
	if (!Formation)
	{
		return;
	}

	RegisteredFormations.Remove(Formation->GetFormationID());
}

AFormationActor* UFormationSubsystem::GetFormationByID(int32 InFormationID) const
{
	if (const TObjectPtr<AFormationActor>* Found = RegisteredFormations.Find(InFormationID))
	{
		return Found->Get();
	}
	return nullptr;
}

void UFormationSubsystem::GetFormationsByTeam(ETeamID InTeam, TArray<AFormationActor*>& OutFormations) const
{
	OutFormations.Empty();
	for (const auto& Pair : RegisteredFormations)
	{
		if (Pair.Value && Pair.Value->GetTeamID() == InTeam)
		{
			OutFormations.Add(Pair.Value.Get());
		}
	}
}

void UFormationSubsystem::GetAllFormations(TArray<AFormationActor*>& OutFormations) const
{
	OutFormations.Empty();
	for (const auto& Pair : RegisteredFormations)
	{
		if (Pair.Value)
		{
			OutFormations.Add(Pair.Value.Get());
		}
	}
}

AFormationActor* UFormationSubsystem::FindNearestEnemyFormation(const FVector& Location, ETeamID MyTeam) const
{
	AFormationActor* BestEnemy = nullptr;
	float BestDistSq = TNumericLimits<float>::Max();

	for (const auto& Pair : RegisteredFormations)
	{
		AFormationActor* Candidate = Pair.Value.Get();
		if (Candidate && Candidate->GetTeamID() != MyTeam && Candidate->GetAliveCount() > 0)
		{
			const float DistSq = FVector::DistSquared(Location, Candidate->GetFormationCenter());
			if (DistSq < BestDistSq)
			{
				BestDistSq = DistSq;
				BestEnemy = Candidate;
			}
		}
	}

	return BestEnemy;
}

void UFormationSubsystem::CalculateSlotLayout(
	int32 EntityCount,
	int32 FormationWidth,
	float Spacing,
	const FVector& Center,
	const FVector& Facing,
	TArray<FFormationSlot>& OutSlots)
{
	OutSlots.Empty(EntityCount);
	if (EntityCount <= 0)
	{
		return;
	}

	const int32 Columns = FMath::Clamp(FormationWidth, TWConstants::MinFormationWidth, TWConstants::MaxFormationWidth);
	const int32 Rows = FMath::CeilToInt(static_cast<float>(EntityCount) / static_cast<float>(Columns));

	const FRotator FacingRot = Facing.GetSafeNormal2D().Rotation();
	int32 SoldiersPlaced = 0;

	for (int32 Row = 0; Row < Rows; ++Row)
	{
		const int32 SoldiersInThisRow = FMath::Min(Columns, EntityCount - SoldiersPlaced);
		if (SoldiersInThisRow <= 0)
		{
			break;
		}

		// Row offset along forward/backward axis (Row 0 is front row at 0, subsequent rows step back along -X)
		const float LocalX = -Row * Spacing;

		for (int32 Col = 0; Col < SoldiersInThisRow; ++Col)
		{
			// Center column horizontally around 0 (Y axis) for this specific row
			const float LocalY = (Col - (SoldiersInThisRow - 1) * 0.5f) * Spacing;
			const FVector LocalOffset(LocalX, LocalY, 0.0f);

			// Transform to World Space
			const FVector WorldPos = Center + FacingRot.RotateVector(LocalOffset);

			FFormationSlot NewSlot(LocalOffset, Col, Row);
			NewSlot.WorldPosition = WorldPos;
			OutSlots.Add(NewSlot);

			++SoldiersPlaced;
		}
	}
}
