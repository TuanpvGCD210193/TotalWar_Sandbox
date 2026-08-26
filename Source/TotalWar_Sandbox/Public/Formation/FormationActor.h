// Copyright Total War Sandbox. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TotalWar_Sandbox/Public/Core/TWTypes.h"
#include "TotalWar_Sandbox/Public/Entity/SoldierEntity.h"
#include "TotalWar_Sandbox/Public/Entity/FormationSlot.h"
#include "TotalWar_Sandbox/Public/Entity/SoldierHandle.h"
#include "TotalWar_Sandbox/Public/Formation/FormationCommand.h"
#include "FormationActor.generated.h"

class UUnitDataAsset;
class UFormationSlotComponent;
class USoldierVisualComponent;
class UFormationMovementComponent;
class UMaterialInterface;

/**
 * AFormationActor — Đạo Quân (Squad Coordinator).
 * Đóng vai trò hạt nhân điều phối (Coordinator Pattern):
 * Sở hữu mảng lính FSoldierEntity và điều phối 3 Component chuyên biệt (SRP):
 *   1. UFormationSlotComponent (Bộ Quản Lý Vị Trí & Hình Học Đội Hình)
 *   2. USoldierVisualComponent (Bộ Hiển Thị Instanced Static Mesh)
 *   3. UFormationMovementComponent (Bộ Điều Khiển Di Chuyển & Lái Lính)
 * Follows Single Responsibility Principle (SRP) & SOLID architecture.
 */
UCLASS()
class TOTALWAR_SANDBOX_API AFormationActor : public AActor
{
	GENERATED_BODY()

public:
	AFormationActor();

	virtual void Tick(float DeltaTime) override;

	// ========================================================================
	// INITIALIZATION & SQUAD LIFECYCLE
	// ========================================================================

	/** Initializes squad with unit data archetype, team faction, and soldier count */
	void InitializeFormation(UUnitDataAsset* InDataAsset, ETeamID InTeam, int32 InCount = INDEX_NONE);

	/** Rebuilds slot grid and smoothly reassigns soldiers to closest slots (Greedy matching) */
	void RebuildFormationAndRegroup(int32 NewWidth = INDEX_NONE);

	/** Synchronizes all soldier entity transforms to the Visual Component */
	void SyncVisualTransforms();

	/** Handles soldier death: compact swap-remove, visual update, and formation regrouping */
	void KillSoldier(int32 SoldierIndex);

	// ========================================================================
	// STATE, DIRECTION & COMMANDS
	// ========================================================================

	/** Executes a tactical command (Move, Attack, PlacementMove) */
	void ExecuteCommand(const FFormationCommand& Command);

	void SetFormationState(EFormationState NewState);
	void SetFacingDirection(const FVector& NewFacing);
	void SetSelected(bool bInSelected);

	// ========================================================================
	// GETTERS & ENCAPSULATED ACCESSORS (Law of Demeter)
	// ========================================================================

	FORCEINLINE bool IsSelected() const { return bIsSelected; }
	FORCEINLINE int32 GetFormationID() const { return FormationID; }
	FORCEINLINE void SetFormationID(int32 InID) { FormationID = InID; }

	FORCEINLINE ETeamID GetTeamID() const { return TeamID; }
	FORCEINLINE EFormationState GetFormationState() const { return FormationState; }
	FORCEINLINE UUnitDataAsset* GetUnitDataAsset() const { return UnitDataAsset; }

	FORCEINLINE int32 GetAliveCount() const { return SoldierEntities.Num(); }
	FORCEINLINE const FVector& GetFacingDirection() const { return FacingDirection; }
	FVector GetFormationCenter() const;

	/** Encapsulated Forwarders to MovementComponent */
	bool IsMoving() const;
	const FVector& GetTargetDestination() const;
	const FVector& GetFinalDesiredFacing() const;
	AFormationActor* GetTargetEnemyFormation() const;
	bool IsAttacking() const;

	/** Encapsulated Forwarders to SlotComponent */
	int32 GetFormationWidth() const;
	float GetSoldierSpacing() const;
	void SetFormationWidth(int32 NewWidth);

	/** Calculates the total physical bounding box of the formation on the ground */
	void GetFormationGroundBounds(FVector& OutCenter, FVector& OutExtents, FRotator& OutRotation) const;

	FORCEINLINE const TArray<FSoldierEntity>& GetSoldierEntities() const { return SoldierEntities; }
	FORCEINLINE TArray<FSoldierEntity>& GetSoldierEntitiesMutable() { return SoldierEntities; }

	FORCEINLINE UFormationSlotComponent* GetSlotComponent() const { return SlotComponent; }
	FORCEINLINE USoldierVisualComponent* GetVisualComponent() const { return VisualComponent; }
	FORCEINLINE UFormationMovementComponent* GetMovementComponent() const { return MovementComponent; }

	/** Retrieves a reference to a soldier entity by index */
	FSoldierEntity* GetSoldier(int32 Index);

	/** Checks if a given soldier index is currently valid and alive */
	bool IsSoldierAlive(int32 Index) const;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// ========================================================================
	// SPECIALIZED SRP COMPONENTS
	// ========================================================================

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Formation|Components")
	TObjectPtr<USceneComponent> TransformRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Formation|Components")
	TObjectPtr<UFormationSlotComponent> SlotComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Formation|Components")
	TObjectPtr<USoldierVisualComponent> VisualComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Formation|Components")
	TObjectPtr<UFormationMovementComponent> MovementComponent;

	// ========================================================================
	// CONFIGURATION & IDENTITY
	// ========================================================================

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Formation|Archetype")
	TObjectPtr<UUnitDataAsset> UnitDataAsset;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Formation|Identity")
	ETeamID TeamID;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Formation|Identity")
	int32 FormationID;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Formation|Config")
	int32 InitialEntityCountOverride;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation|Visual")
	TObjectPtr<UStaticMesh> SoldierMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation|Visual")
	TObjectPtr<UMaterialInterface> BlueTeamMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation|Visual")
	TObjectPtr<UMaterialInterface> RedTeamMaterial;

private:
	UPROPERTY(Transient)
	EFormationState FormationState;

	bool bIsSelected;
	FVector FacingDirection;

	/** Flat compact array of alive soldier entities */
	TArray<FSoldierEntity> SoldierEntities;
};
