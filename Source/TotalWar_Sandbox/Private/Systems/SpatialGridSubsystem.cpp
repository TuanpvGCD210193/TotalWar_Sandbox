// Copyright Total War Sandbox. All Rights Reserved.

#include "TotalWar_Sandbox/Public/Systems/SpatialGridSubsystem.h"
#include "TotalWar_Sandbox/Public/Systems/FormationSubsystem.h"
#include "TotalWar_Sandbox/Public/Formation/FormationActor.h"

void USpatialGridSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	CellSize = TWConstants::SpatialGridCellSize;
	TotalSoldierCount = 0;
	GridMap.Empty();
}

void USpatialGridSubsystem::Deinitialize()
{
	GridMap.Empty();
	Super::Deinitialize();
}

void USpatialGridSubsystem::ClearGrid()
{
	GridMap.Empty();
	TotalSoldierCount = 0;
}

void USpatialGridSubsystem::RebuildGrid()
{
	ClearGrid();

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	UFormationSubsystem* FormationSubsystem = World->GetSubsystem<UFormationSubsystem>();
	if (!FormationSubsystem)
	{
		return;
	}

	TArray<AFormationActor*> AllFormations;
	FormationSubsystem->GetAllFormations(AllFormations);

	for (AFormationActor* Formation : AllFormations)
	{
		if (!Formation)
		{
			continue;
		}

		const int32 FormationID = Formation->GetFormationID();
		const TArray<FSoldierEntity>& Soldiers = Formation->GetSoldierEntities();

		for (int32 SoldierIdx = 0; SoldierIdx < Soldiers.Num(); ++SoldierIdx)
		{
			const FSoldierEntity& Soldier = Soldiers[SoldierIdx];
			if (!Soldier.IsAlive())
			{
				continue;
			}

			int32 CellX = 0;
			int32 CellY = 0;
			WorldToCellCoords(Soldier.Position, CellX, CellY);

			const int64 CellKey = EncodeCellKey(CellX, CellY);
			GridMap.FindOrAdd(CellKey).Add(FSoldierHandle(FormationID, SoldierIdx));
			++TotalSoldierCount;
		}
	}
}

void USpatialGridSubsystem::QuerySoldiersInRadius(const FVector& Center, float Radius, TArray<FSoldierHandle>& OutHandles) const
{
	OutHandles.Empty();

	if (Radius <= 0.0f || GridMap.Num() == 0)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	UFormationSubsystem* FormationSubsystem = World->GetSubsystem<UFormationSubsystem>();
	if (!FormationSubsystem)
	{
		return;
	}

	const float RadiusSq = FMath::Square(Radius);

	int32 MinCellX = 0, MaxCellX = 0;
	int32 MinCellY = 0, MaxCellY = 0;
	WorldToCellCoords(Center - FVector(Radius, Radius, 0.0f), MinCellX, MinCellY);
	WorldToCellCoords(Center + FVector(Radius, Radius, 0.0f), MaxCellX, MaxCellY);

	for (int32 X = MinCellX; X <= MaxCellX; ++X)
	{
		for (int32 Y = MinCellY; Y <= MaxCellY; ++Y)
		{
			const int64 CellKey = EncodeCellKey(X, Y);
			if (const TArray<FSoldierHandle>* HandlesInCell = GridMap.Find(CellKey))
			{
				for (const FSoldierHandle& Handle : *HandlesInCell)
				{
					if (AFormationActor* Formation = FormationSubsystem->GetFormationByID(Handle.FormationID))
					{
						if (FSoldierEntity* Soldier = Formation->GetSoldier(Handle.SoldierIndex))
						{
							if (Soldier->IsAlive())
							{
								const float DistSq = FVector::DistSquared2D(Center, Soldier->Position);
								if (DistSq <= RadiusSq)
								{
									OutHandles.Add(Handle);
								}
							}
						}
					}
				}
			}
		}
	}
}

void USpatialGridSubsystem::QueryEnemiesInRadius(const FVector& Center, float Radius, ETeamID MyTeam, TArray<FSoldierHandle>& OutEnemyHandles) const
{
	OutEnemyHandles.Empty();

	if (Radius <= 0.0f || GridMap.Num() == 0)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	UFormationSubsystem* FormationSubsystem = World->GetSubsystem<UFormationSubsystem>();
	if (!FormationSubsystem)
	{
		return;
	}

	const float RadiusSq = FMath::Square(Radius);

	int32 MinCellX = 0, MaxCellX = 0;
	int32 MinCellY = 0, MaxCellY = 0;
	WorldToCellCoords(Center - FVector(Radius, Radius, 0.0f), MinCellX, MinCellY);
	WorldToCellCoords(Center + FVector(Radius, Radius, 0.0f), MaxCellX, MaxCellY);

	for (int32 X = MinCellX; X <= MaxCellX; ++X)
	{
		for (int32 Y = MinCellY; Y <= MaxCellY; ++Y)
		{
			const int64 CellKey = EncodeCellKey(X, Y);
			if (const TArray<FSoldierHandle>* HandlesInCell = GridMap.Find(CellKey))
			{
				for (const FSoldierHandle& Handle : *HandlesInCell)
				{
					if (AFormationActor* Formation = FormationSubsystem->GetFormationByID(Handle.FormationID))
					{
						// Filter enemy team only
						if (Formation->GetTeamID() != MyTeam)
						{
							if (FSoldierEntity* Soldier = Formation->GetSoldier(Handle.SoldierIndex))
							{
								if (Soldier->IsAlive())
								{
									const float DistSq = FVector::DistSquared2D(Center, Soldier->Position);
									if (DistSq <= RadiusSq)
									{
										OutEnemyHandles.Add(Handle);
									}
								}
							}
						}
					}
				}
			}
		}
	}
}
