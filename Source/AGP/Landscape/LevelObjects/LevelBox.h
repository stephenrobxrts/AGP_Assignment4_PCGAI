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

	FVector CalculateBoxOffset(const ALevelBox& Box, const FVector& Direction);
	bool BoxesIntersect2D(const ALevelBox& BoxA, const ALevelBox& BoxB);
	bool BoxesIntersect(const ALevelBox& BoxA, const ALevelBox& BoxB);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;


public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
