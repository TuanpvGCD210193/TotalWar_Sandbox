// Copyright Total War Sandbox. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TotalWar_Sandbox/Public/Core/TWTypes.h"
#include "SoldierVisualComponent.generated.h"

struct FSoldierEntity;
class UInstancedStaticMeshComponent;
class UMaterialInterface;

/**
 * USoldierVisualComponent — Bộ Hiển Thị & Render Lính.
 * Chuyên trách: Quản lý Instanced Static Mesh Component (1 Draw Call / Formation),
 * đồng bộ transform lính, quản lý màu sắc phe Xanh/Đỏ.
 * Follows Single Responsibility Principle (SRP).
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TOTALWAR_SANDBOX_API USoldierVisualComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USoldierVisualComponent();

	// ========================================================================
	// PUBLIC API
	// ========================================================================

	/** Initializes visual component and attaches the InstancedStaticMeshComponent to the actor root */
	void InitializeVisualComponent(USceneComponent* AttachToRoot, ETeamID InTeam, UStaticMesh* CustomMesh = nullptr, UMaterialInterface* CustomBlueMat = nullptr, UMaterialInterface* CustomRedMat = nullptr);

	/** Synchronizes all alive soldier entities to ISMC instance transforms */
	void SyncVisualTransforms(const TArray<FSoldierEntity>& Soldiers);

	/** Rebuilds all instances from scratch */
	void RebuildInstances(const TArray<FSoldierEntity>& Soldiers);

	/** Removes an instance upon soldier death */
	void RemoveInstance(int32 InstanceIndex);

	/** Clears all rendered instances */
	void ClearAllInstances();

	// ========================================================================
	// GETTERS (Encapsulation)
	// ========================================================================

	FORCEINLINE UInstancedStaticMeshComponent* GetMeshComponent() const { return SoldierMeshComponent; }
	FORCEINLINE int32 GetInstanceCount() const;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Visual")
	TObjectPtr<UInstancedStaticMeshComponent> SoldierMeshComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
	TObjectPtr<UStaticMesh> SoldierMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
	TObjectPtr<UMaterialInterface> BlueTeamMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
	TObjectPtr<UMaterialInterface> RedTeamMaterial;

private:
	ETeamID CurrentTeam;

	void ApplyTeamMaterial();
};
