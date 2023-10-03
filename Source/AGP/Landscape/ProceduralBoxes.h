// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "../Pathfinding/NavigationNode.h"
#include "GameFramework/Actor.h"
#include "ProceduralBoxes.generated.h"

UCLASS()
class AGP_API AProceduralBoxes : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AProceduralBoxes();
	virtual bool ShouldTickIfViewportsOnly() const override;

	

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	//Mesh Generation Settings
	UPROPERTY(EditAnywhere)
	bool bShouldRegenerate = false;
	UPROPERTY(EditAnywhere)
	int32 Width = 10;
	UPROPERTY(EditAnywhere)
	int32 Height = 10;
	UPROPERTY(EditAnywhere)
	int32 Depth = 10;
	UPROPERTY(EditAnywhere)
	float VertexSpacing = 1000.0f;

	//Navigation Nodes
	UPROPERTY(EditAnywhere)
	TArray<ANavigationNode*> Nodes;


	//Debugging
	UPROPERTY(EditAnywhere)
	bool bDebugShowNavNodes = true;
	UPROPERTY(EditAnywhere)
	bool bDebugNeedsUpdate = false;
	void DebugShowNavNodes();
	
	void GenerateLandscape();
	void CreateSimpleBox();
	void ClearLandscape();

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
};
