// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "PickupManagerSubsystem.generated.h"

/**
 * 
 */
UCLASS()
class AGP_API UPickupManagerSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	//Register this as a tickable object
	// (this is default for many classes, but obscure ones may need this explicitly)
	virtual TStatId GetStatId() const override
	{
		return TStatId();
	}

protected:
	TArray<FVector> PossibleSpawnLocations;
	float PickupSpawnRate = 5.0;
	float TimeSinceLastSpawn = 0.0f;

private:
	void PopulateSpawnLocations();
	void SpawnWeaponPickup();

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
