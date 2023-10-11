// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LevelBox.generated.h"

UCLASS()
class AGP_API ALevelBox : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ALevelBox();

	FVector CalculateBoxOffset(const FLevelBox& Box, const FVector& Direction);
	bool BoxesIntersect2D(const FLevelBox& BoxA, const FLevelBox& BoxB);
	bool BoxesIntersect(const FLevelBox& BoxA, const FLevelBox& BoxB);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;


public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
