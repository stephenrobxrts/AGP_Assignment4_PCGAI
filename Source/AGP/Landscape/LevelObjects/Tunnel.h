// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LevelBox.h"
#include "Tunnel.generated.h"

UCLASS()
class AGP_API ATunnel : public ALevelBox
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ATunnel();

	UPROPERTY()
	const ALevelBox* StartBox;
	UPROPERTY()
	const ALevelBox* EndBox;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
