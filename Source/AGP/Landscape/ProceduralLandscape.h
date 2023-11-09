// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ProceduralMeshComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "KismetProceduralMeshLibrary.h"
#include "../Pathfinding/NavigationNode.h"
#include "GameFramework/Actor.h"
#include "ProceduralLandscape.generated.h"

class APickupBase;
class AWeaponPickup;

UCLASS()
class AGP_API AProceduralLandscape : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AProceduralLandscape();
	virtual bool ShouldTickIfViewportsOnly() const override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;


	//Procedural Mesh Properties
	UPROPERTY(VisibleAnywhere)
	UProceduralMeshComponent* ProceduralMesh;
	UPROPERTY()
	TArray<FVector> Vertices;
	UPROPERTY()
	TArray<int32> Triangles;
	UPROPERTY()
	TArray<FVector2D> UVCoords;
	UPROPERTY()
	TArray<FVector> Normals;
	UPROPERTY()
	TArray<FProcMeshTangent> Tangents;

	//Mesh Generation Settings
	UPROPERTY(EditAnywhere)
	bool bShouldRegenerate = false;
	UPROPERTY(EditAnywhere)
	int32 Width = 10;
	UPROPERTY(EditAnywhere)
	int32 Height = 10;
	UPROPERTY(EditAnywhere)
	float VertexSpacing = 100.0f;

	//Navigation Nodes
	UPROPERTY(EditAnywhere)
	TArray<ANavigationNode*> Nodes;
	//Spawnable Pickups - place blueprint in this slot
	UPROPERTY(EditAnywhere)
	TSubclassOf<class AWeaponPickup> PickupBlueprint;
	UPROPERTY()
	TArray<APickupBase*> Pickups;


	//Debugging
	UPROPERTY(EditAnywhere)
	bool bDebugShowNavNodes = true;
	UPROPERTY(EditAnywhere)
	bool bDebugNeedsUpdate = false;
	void DebugShowNavNodes();
	
	void GenerateLandscape();
	void CreateSimplePlane();
	void ClearLandscape();
	void SpawnPickups();

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
