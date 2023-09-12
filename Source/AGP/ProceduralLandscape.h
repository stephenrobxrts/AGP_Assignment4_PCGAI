// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ProceduralMeshComponent.h"
#include "GameFramework/Actor.h"
#include "ProceduralLandscape.generated.h"

UCLASS()
class AGP_API AProceduralLandscape : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AProceduralLandscape();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere)
	UProceduralMeshComponent* ProceduralMesh;
	UPROPERTY()
    TArray<FVector> Vertices;
    UPROPERTY()
    TArray<int32> Triangles;
    UPROPERTY()
    TArray<FVector2D> UVCoords;

	void CreateSimplePlane();


public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
