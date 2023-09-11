// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyCharacter.h"



// Sets default values
AEnemyCharacter::AEnemyCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AEnemyCharacter::BeginPlay()
{
	Super::BeginPlay();
	FVector Location = GetActorLocation();
	PathfindingSubsystem = GetWorld()->GetSubsystem<UPathfindingSubsystem>();
	CurrentPath = PathfindingSubsystem->GetRandomPath(Location);

	//Get a reference to the player character
	for (TActorIterator<APlayerCharacter> It(GetWorld()); It; ++It)
	{
		Player = *It;
		if (Player)
		{
			//found player and player location
			UE_LOG(LogTemp, Display, TEXT("Found player"));
			break;
		}
		
	}
}

void AEnemyCharacter::TickPatrol()
{
	if (CurrentPath.Num() == 0)
	{
		FVector Location = GetActorLocation();
		CurrentPath = PathfindingSubsystem->GetRandomPath(Location);
		MoveAlongPath();
	}
	else
	{
		MoveAlongPath();
	};
}

void AEnemyCharacter::TickEngage()
{
	if (CurrentPath.Num() <= 0)
	{
		CurrentPath = PathfindingSubsystem->GetPath(GetActorLocation(), Player->GetActorLocation());
		Fire(Player->GetActorLocation());
	}
	else
	{
		MoveAlongPath();
		Fire(Player->GetActorLocation());
	};
}

void AEnemyCharacter::TickEvade()
{
	if (CurrentPath.Num() <= 0)
	{
		CurrentPath = PathfindingSubsystem->GetPathAway(GetActorLocation(), Player->GetActorLocation());
	}
	else
	{
		MoveAlongPath();
	};
}

// Called every frame
void AEnemyCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	switch (CurrentState)
	{
	case EEnemyState::Patrol:
		TickPatrol();
		break;
	case EEnemyState::Engage:
		TickEngage();
		break;
	case EEnemyState::Evade:
		TickEvade();
		break;
	};
}

void AEnemyCharacter::MoveAlongPath()
{
	FVector currentLocation = GetActorLocation();
	if (CurrentPath.Num() > 0)
	{
		FVector nextLocation = CurrentPath[CurrentPath.Num() - 1];
		// Calculate the direction from currentLocation to nextLocation
		FVector Direction = (nextLocation - currentLocation).GetSafeNormal();
		AddMovementInput(Direction, 1.0f);

		float distance = FVector::Dist(currentLocation, nextLocation);
		if (distance < 90.0f)
		{
			CurrentPath.Pop();
		}
	}
}


// Called to bind functionality to input
void AEnemyCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

