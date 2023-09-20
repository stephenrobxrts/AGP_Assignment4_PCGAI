// Fill out your copyright notice in the Description page of Project Settings.


#include "AGPGameInstance.h"
#include "PathfindingSubsystem.h"
#include "PickupManagerSubsystem.h"



void UPickupManagerSubsystem::PopulateSpawnLocations()
{
	//Empty the PossibleSpawnLocations array
	PossibleSpawnLocations.Empty();

	//set PossibleSpawnLocations to the return value of the Pathfinding 
    //Subsystem’s GetWaypointPositions function.
	PossibleSpawnLocations = GetWorld()->GetSubsystem<UPathfindingSubsystem>()->GetWaypointPositions();
}

void UPickupManagerSubsystem::SpawnWeaponPickup()
{
	//If PossibleSpawnLocations is empty, log an error and return.
	if (PossibleSpawnLocations.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("Unable to spawn weapon pickup."))
		return;
	}
	//Otherwise, get a reference to the AGPGameInstance and spawn a weapon 
	//pickup at a random location in PossibleSpawnLocations.
	if (const UAGPGameInstance* GameInstance =
	 GetWorld()->GetGameInstance<UAGPGameInstance>())
	{
		//Get a random location from PossibleSpawnLocations
		FVector SpawnPosition =
		PossibleSpawnLocations[FMath::RandRange(0, PossibleSpawnLocations.Num() - 1)];
		// Add 50 to the height(??)
		SpawnPosition.Z += 50.0f;

		//Spawn a weapon pickup at the location
		UClass* WeaponPickupClass = GameInstance->GetWeaponPickupClass();
		
		AWeaponPickup* Pickup = GetWorld()->SpawnActor<AWeaponPickup>(WeaponPickupClass,SpawnPosition,FRotator::ZeroRotator);
		UE_LOG(LogTemp, Display, TEXT("Weapon Pickup Spawned"))
    }
}

void UPickupManagerSubsystem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	//If PossibleSpawnLocations is empty, populate it.
	if (PossibleSpawnLocations.IsEmpty())
	{
		PopulateSpawnLocations();
	}

	//Then call SpawnWeaponPickup every PickupSpawnRate seconds.
	TimeSinceLastSpawn += DeltaTime;
	if (TimeSinceLastSpawn >= PickupSpawnRate)
	{
		SpawnWeaponPickup();
		TimeSinceLastSpawn = 0.0f;
	}
	
}
