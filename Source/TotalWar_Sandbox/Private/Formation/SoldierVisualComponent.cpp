#include "TotalWar_Sandbox/Public/Formation/SoldierVisualComponent.h"
#include "TotalWar_Sandbox/Public/Entity/SoldierEntity.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Engine/StaticMesh.h"

USoldierVisualComponent::USoldierVisualComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	CurrentTeam = ETeamID::Blue;
}

void USoldierVisualComponent::InitializeVisualComponent(USceneComponent* AttachToRoot, ETeamID InTeam, UStaticMesh* CustomMesh, UMaterialInterface* CustomBlueMat, UMaterialInterface* CustomRedMat)
{
	CurrentTeam = InTeam;

	if (CustomMesh)
	{
		SoldierMesh = CustomMesh;
	}
	if (CustomBlueMat)
	{
		BlueTeamMaterial = CustomBlueMat;
	}
	if (CustomRedMat)
	{
		RedTeamMaterial = CustomRedMat;
	}

	if (!SoldierMeshComponent && GetOwner() && AttachToRoot)
	{
		SoldierMeshComponent = NewObject<UInstancedStaticMeshComponent>(GetOwner(), TEXT("SoldierMeshComp"));
		SoldierMeshComponent->SetupAttachment(AttachToRoot);
		SoldierMeshComponent->RegisterComponent();
		SoldierMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		SoldierMeshComponent->SetCastShadow(true);
		SoldierMeshComponent->SetMobility(EComponentMobility::Movable);

		// Apply static mesh if assigned by hand in Editor
		if (SoldierMesh)
		{
			SoldierMeshComponent->SetStaticMesh(SoldierMesh);
		}

		ApplyTeamMaterial();
	}
}

void USoldierVisualComponent::SyncVisualTransforms(const TArray<FSoldierEntity>& Soldiers)
{
	if (!SoldierMeshComponent)
	{
		return;
	}

	const int32 Count = Soldiers.Num();
	if (Count != SoldierMeshComponent->GetInstanceCount())
	{
		RebuildInstances(Soldiers);
		return;
	}

	const FVector CubeScale(
		TWConstants::SoldierCubeSize / 50.0f,
		TWConstants::SoldierCubeSize / 50.0f,
		TWConstants::SoldierCubeSize / 50.0f
	);

	for (int32 i = 0; i < Count; ++i)
	{
		const FTransform Transform(Soldiers[i].Rotation, Soldiers[i].Position, CubeScale);
		SoldierMeshComponent->UpdateInstanceTransform(i, Transform, true, (i == Count - 1), true);
	}
}

void USoldierVisualComponent::RebuildInstances(const TArray<FSoldierEntity>& Soldiers)
{
	if (!SoldierMeshComponent)
	{
		return;
	}

	SoldierMeshComponent->ClearInstances();

	const FVector CubeScale(
		TWConstants::SoldierCubeSize / 50.0f,
		TWConstants::SoldierCubeSize / 50.0f,
		TWConstants::SoldierCubeSize / 50.0f
	);

	for (int32 i = 0; i < Soldiers.Num(); ++i)
	{
		const FTransform Transform(Soldiers[i].Rotation, Soldiers[i].Position, CubeScale);
		SoldierMeshComponent->AddInstanceWorldSpace(Transform);
	}
}

void USoldierVisualComponent::RemoveInstance(int32 InstanceIndex)
{
	if (SoldierMeshComponent && SoldierMeshComponent->GetInstanceCount() > 0)
	{
		SoldierMeshComponent->RemoveInstance(InstanceIndex);
	}
}

void USoldierVisualComponent::ClearAllInstances()
{
	if (SoldierMeshComponent)
	{
		SoldierMeshComponent->ClearInstances();
	}
}

int32 USoldierVisualComponent::GetInstanceCount() const
{
	return SoldierMeshComponent ? SoldierMeshComponent->GetInstanceCount() : 0;
}

void USoldierVisualComponent::ApplyTeamMaterial()
{
	if (!SoldierMeshComponent)
	{
		return;
	}

	UMaterialInterface* MaterialToApply = (CurrentTeam == ETeamID::Blue) ? BlueTeamMaterial : RedTeamMaterial;

	if (MaterialToApply)
	{
		SoldierMeshComponent->SetMaterial(0, MaterialToApply);
	}
}
