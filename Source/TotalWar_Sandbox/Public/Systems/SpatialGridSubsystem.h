// Copyright Total War Sandbox. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "TotalWar_Sandbox/Public/Core/TWTypes.h"
#include "TotalWar_Sandbox/Public/Entity/SoldierHandle.h"
#include "SpatialGridSubsystem.generated.h"

class UFormationSubsystem;

USTRUCT(BlueprintType)
struct FTWObstacle
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Obstacle")
	FVector Position = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Obstacle")
	float Radius = 100.0f;
};

/**
 * USpatialGridSubsystem — Hệ Thống Phân Vùng Không Gian (Spatial Hash Grid).
 * Trách nhiệm duy nhất (SRP):
 *   - Chia không gian battlefield thành các ô cell kích thước 200cm (2m).
 *   - Cung cấp các hàm truy vấn lân cận O(1) phục vụ: Chống đè lính (Separation steering),
 *     tìm mục tiêu cận chiến (Melee targeting), né vật cản (Obstacle avoidance), và tiền tuyến (Frontline detection).
 * Follows Single Responsibility Principle (SRP) & Dependency Inversion Principle (DIP).
 */
UCLASS()
class TOTALWAR_SANDBOX_API USpatialGridSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// ========================================================================
	// PUBLIC API
	// ========================================================================

	/** Registers a static obstacle (rock, tree, building) for avoidance */
	void RegisterObstacle(const FVector& Position, float Radius);

	/** Clears registered obstacles */
	void ClearObstacles();

	/** Calculates repulsive soft avoidance force from nearby obstacles (O(1)) */
	FVector CalculateObstacleRepulsion(const FVector& SoldierPos, float SoldierRadius = 45.0f) const;

	/** Rebuilds the spatial grid by indexing all alive soldiers across all active formations */
	void RebuildGrid();

	/** Clears all entries in the spatial hash grid */
	void ClearGrid();

	/**
	 * Queries all living soldiers within a given sphere/cylinder radius.
	 * Complexity: O(1) average — only inspects adjacent grid cells.
	 */
	void QuerySoldiersInRadius(const FVector& Center, float Radius, TArray<FSoldierHandle>& OutHandles) const;

	/**
	 * Queries all enemy soldiers within a given radius.
	 * Filters out friendly soldiers belonging to MyTeam.
	 */
	void QueryEnemiesInRadius(const FVector& Center, float Radius, ETeamID MyTeam, TArray<FSoldierHandle>& OutEnemyHandles) const;

	// ========================================================================
	// UTILITY & ACCESSORS (Encapsulation)
	// ========================================================================

	FORCEINLINE float GetCellSize() const { return CellSize; }
	FORCEINLINE void SetCellSize(float NewSize) { CellSize = FMath::Max(50.0f, NewSize); }

	FORCEINLINE int32 GetTotalIndexedSoldiers() const { return TotalSoldierCount; }

private:
	/** Grid cell dimension in centimeters (Default: 200cm = 2m) */
	UPROPERTY(EditDefaultsOnly, Category = "SpatialGrid")
	float CellSize;

	int32 TotalSoldierCount;

	/** Static obstacles on battlefield */
	UPROPERTY(Transient)
	TArray<FTWObstacle> RegisteredObstacles;

	/** Hash map mapping 64-bit Cell Key (CellX << 32 | CellY) to array of soldier handles */
	TMap<int64, TArray<FSoldierHandle>> GridMap;

	// ========================================================================
	// INTERNAL HELPERS
	// ========================================================================

	/** Converts world position to 2D cell grid coordinates */
	FORCEINLINE void WorldToCellCoords(const FVector& Position, int32& OutCellX, int32& OutCellY) const
	{
		OutCellX = FMath::FloorToInt(Position.X / CellSize);
		OutCellY = FMath::FloorToInt(Position.Y / CellSize);
	}

	/** Combines 2D cell coordinates into a unique 64-bit integer hash key */
	static FORCEINLINE int64 EncodeCellKey(int32 CellX, int32 CellY)
	{
		return (static_cast<int64>(CellX) << 32) | (static_cast<uint32>(CellY));
	}
};
