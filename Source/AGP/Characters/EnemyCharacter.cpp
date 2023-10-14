// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyCharacter.h"

#include "../Pathfinding/NavigationNode.h"


// Sets default values
AEnemyCharacter::AEnemyCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	PawnSensingComponent = CreateDefaultSubobject<UPawnSensingComponent>(TEXT("PawnSensingComponent"));
}

// Called when the game starts or when spawned
void AEnemyCharacter::BeginPlay()
{
	Super::BeginPlay();
	FVector Location = GetActorLocation();
	PathfindingSubsystem = GetWorld()->GetSubsystem<UPathfindingSubsystem>();
	CurrentPath = PathfindingSubsystem->GetRandomPath(Location);

	// Get a reference to the player character
	for (TActorIterator<APlayerCharacter> It(GetWorld()); It; ++It)
	{
	 	Player = *It;
	 	if (Player)
	 	{
	 		// Found player and player location
	 		UE_LOG(LogTemp, Display, TEXT("Found player"));
	 		break;
	 	}
	 }
	// Get end node location in end room based off enemy player's starting position
	EndNode = PathfindingSubsystem->FindNearestNode(GetActorLocation());

	if (PawnSensingComponent)
	{
		PawnSensingComponent->OnSeePawn.AddDynamic(this, &AEnemyCharacter::OnSensedPawn);
	}
}

void AEnemyCharacter::TickPatrol()
{
	// If current path is empty
	if (CurrentPath.IsEmpty())
	{
		// Generate random path from enemy player's position
		CurrentPath = PathfindingSubsystem->GetRandomPath(GetActorLocation());
	}
	// Move enemy player along path
	MoveAlongPath();
	
}

void AEnemyCharacter::TickEngage()
{
	// If current path is empty
	if (CurrentPath.IsEmpty())
	{
		CurrentPath = PathfindingSubsystem->GetPath(GetActorLocation(), SensedCharacter->GetActorLocation());
	}
	// Move enemy player along path
	MoveAlongPath();
	if (SensedCharacter)
	{
		// Fire at player
		Fire(SensedCharacter->GetActorLocation());
		// Get direction to player
		FVector DirectionToPlayer = (SensedCharacter->GetActorLocation() - GetActorLocation()).GetSafeNormal();
		// Rotate enemy player towards player
		FRotator NewRotation = FRotationMatrix::MakeFromX(DirectionToPlayer).Rotator();
		// Set the enemy player's rotation
		SetActorRotation(NewRotation);	
	}
	// If weapon is empty, reload
	if (WeaponComponent)
	{
		if (WeaponComponent->GetRoundsRemainingInMagazine() <= 0)
		{
			Reload();
		}
	}
}


void AEnemyCharacter::TickInvestigate()
{
	// If current path is empty
	if (CurrentPath.IsEmpty())
	{
		// Store connected nodes in array (that connect to node nearest to enemy player)
		TArray<ANavigationNode*> ConnectedNodes = PathfindingSubsystem->FindNearestNode(GetActorLocation())->GetConnectedNodes();
		TArray<ANavigationNode*> PossibleNodes;
		// Loop through each connected node
		for (ANavigationNode* ConnectedNode : ConnectedNodes)
		{
			if (PathfindingSubsystem->GetDistance(ConnectedNode->GetActorLocation(), Player->GetActorLocation()) <
				PathfindingSubsystem->GetDistance(GetActorLocation(), Player->GetActorLocation()))
			{
				PossibleNodes.Add(ConnectedNode);
			}
		}
		// Rare but possible occurence, simply generate path from enemy to player
		if (PossibleNodes.IsEmpty())
		{
			CurrentPath = PathfindingSubsystem->GetPath(GetActorLocation(), Player->GetActorLocation());
		}
		else
		{
			// Pick one of the available paths to bring enemy closer to player
			CurrentPath = PathfindingSubsystem->GetPath(GetActorLocation(), PossibleNodes[FMath::RandRange(0, PossibleNodes.Num()-1)]->GetActorLocation());			
		}
	}
	// Move enemy player along path
	MoveAlongPath();
}

void AEnemyCharacter::TickAmbush()
{
	// If current path is empty
	if (CurrentPath.IsEmpty())
	{
		// If player is 3 nodes away from enemy, generate path to player
		if (PathfindingSubsystem->GetDistance(GetActorLocation(), Player->GetActorLocation()) <= 3)
		{
			CurrentPath = PathfindingSubsystem->GetPath(GetActorLocation(), Player->GetActorLocation());
		}
		// Calculate intercept pathway
		else
		{
			// Store connected nodes in array (that connect to node nearest to enemy player)
			TArray<ANavigationNode*> ConnectedNodes = PathfindingSubsystem->FindNearestNode(GetActorLocation())->GetConnectedNodes();
			// Create array for second level connections (in case initial node is a room and you need to determine surrounding rooms
			// Keep in mind that corridors also have nodes)
			TArray<ANavigationNode*> SecondLevelConnectedNodes;
			// Loop through each connected node
			for (ANavigationNode* ConnectedNode : ConnectedNodes)
			{
				// Add second level connection nodes to array
				SecondLevelConnectedNodes.Append(ConnectedNode->GetConnectedNodes());
			}

			// Set target node to null
			ANavigationNode* TargetNode = nullptr;
			// Loop through each secondary connected node
			for (ANavigationNode* ConnectedNode : SecondLevelConnectedNodes)
			{
				// If node is of equal distance to end node as the enemy is AND node is not initial node (secondary connections also included initial node)
				if (PathfindingSubsystem->GetDistance(ConnectedNode->GetActorLocation(), EndNode->GetActorLocation())
					>= PathfindingSubsystem->GetDistance(GetActorLocation(), EndNode->GetActorLocation())
					&& ConnectedNode != PathfindingSubsystem->FindNearestNode(GetActorLocation()))
				{
					// Set target node to node
					TargetNode = ConnectedNode;
				}
			}
			// If target node is not null
			if (TargetNode)
			{
				// Generate path from enemy player's node to target node
				CurrentPath = PathfindingSubsystem->GetPath(GetActorLocation(), TargetNode->GetActorLocation());
			}
			else
			{
				// Generate path from enemy player's node to end room node
				CurrentPath = PathfindingSubsystem->GetPath(GetActorLocation(), EndNode->GetActorLocation());			
			}
			// Shorten path to recheck conditions after enemy has reached new room
			CurrentPath.SetNum(3);
		}
	}
	// Move enemy player along path
	MoveAlongPath();
}

void AEnemyCharacter::TickProtect()
{
	// If current path is empty
	if (CurrentPath.IsEmpty())
	{
		// If equal distance away from end room as player, move towards player first to try and engage
		if (PathfindingSubsystem->GetDistance(GetActorLocation(), EndNode->GetActorLocation())
			== PathfindingSubsystem->GetDistance(Player->GetActorLocation(), EndNode->GetActorLocation())
			&& PathfindingSubsystem->GetDistance(GetActorLocation(), Player->GetActorLocation()) == 3)
		{
			CurrentPath = PathfindingSubsystem->GetPath(GetActorLocation(), Player->GetActorLocation());
		}
		// ELse, retreat back to end room 
		else
		{
			CurrentPath = PathfindingSubsystem->GetPath(GetActorLocation(), EndNode->GetActorLocation());			
		}
	}
	// Move enemy player along path
	MoveAlongPath();
}

void AEnemyCharacter::OnSensedPawn(APawn* Pawn)
{
	if (Pawn)
	{
		SensedCharacter = Cast<APlayerCharacter>(Pawn);
		if (SensedCharacter)
		{
			UE_LOG(LogTemp, Display, TEXT("Sensed player"));
		}
	}
}

void AEnemyCharacter::UpdateSight()
{
	if (!SensedCharacter)
	{
		return;
	}
	if (PawnSensingComponent->HasLineOfSightTo(SensedCharacter))
	{
		//Do nothing
	}
	else
	{
		UE_LOG(LogTemp, Display, TEXT("Lost Player"));
		SensedCharacter = nullptr;
	}
}

// Called every frame
void AEnemyCharacter::Tick(float DeltaTime)
{
	UpdateSight();
	Super::Tick(DeltaTime);
	switch (CurrentState)
	{
	case EEnemyState::Patrol:
		TickPatrol();
		if (SensedCharacter)
		{
			CurrentPath.Empty();
			CurrentState = EEnemyState::Engage;
		}
		else if (PathfindingSubsystem->GetDistance(Player->GetActorLocation(), EndNode->GetActorLocation())
				<= PathfindingSubsystem->GetDistance(GetActorLocation(), EndNode->GetActorLocation()))
		{
			CurrentPath.Empty();
			CurrentState = EEnemyState::Protect;
		}
		else
		{
			float DistanceToPlayer = PathfindingSubsystem->GetDistance(GetActorLocation(), Player->GetActorLocation());
			if ((DistanceToPlayer == 6 || DistanceToPlayer == 7) && Player->IsPlayerMoving())
			{
				CurrentPath.Empty();
				CurrentState = EEnemyState::Investigate;
			}
			else if (DistanceToPlayer <= 5 && Player->IsPlayerMoving())
			{
				CurrentPath.Empty();
				CurrentState = EEnemyState::Ambush;
			}
		}
		break;
	case EEnemyState::Engage:
		TickEngage();
		if (!SensedCharacter)
		{
			if (PathfindingSubsystem->GetDistance(Player->GetActorLocation(), EndNode->GetActorLocation())
				<= PathfindingSubsystem->GetDistance(GetActorLocation(), EndNode->GetActorLocation()))
			{
				CurrentPath.Empty();
				CurrentState = EEnemyState::Protect;
			}
			// Dont think this could ever occur, but just in case
			else if (PathfindingSubsystem->GetDistance(GetActorLocation(), Player->GetActorLocation()) > 7)
			{
				CurrentPath.Empty();
				CurrentState = EEnemyState::Patrol; 
			}
			else if (PathfindingSubsystem->GetDistance(GetActorLocation(), Player->GetActorLocation()) >= 6)
			{
				CurrentPath.Empty();
				CurrentState = EEnemyState::Investigate;
			}
			else if (PathfindingSubsystem->GetDistance(GetActorLocation(), Player->GetActorLocation()) >= 1)
			{
				CurrentPath.Empty();
				CurrentState = EEnemyState::Ambush; 
			}
			break;
		}
		break;
	case EEnemyState::Investigate:
		//IsPlayerMoving() &&
		TickInvestigate();
		if (SensedCharacter)
		{
			CurrentPath.Empty();
			CurrentState = EEnemyState::Engage;
		}
		else if (PathfindingSubsystem->GetDistance(Player->GetActorLocation(), EndNode->GetActorLocation())
			<= PathfindingSubsystem->GetDistance(GetActorLocation(), EndNode->GetActorLocation()))
		{
			CurrentPath.Empty();
			CurrentState = EEnemyState::Protect;
		}
		else if (PathfindingSubsystem->GetDistance(GetActorLocation(), Player->GetActorLocation()) == 5 && Player->IsPlayerMoving())
		{
			CurrentPath.Empty();
			CurrentState = EEnemyState::Ambush; // Implement the logic to move one node closer to the player
		}
		else if (PathfindingSubsystem->GetDistance(GetActorLocation(), Player->GetActorLocation()) > 7)
		{
			CurrentPath.Empty();
			CurrentState = EEnemyState::Patrol; // Switch to Engage when the player is not moving or is too close
		}
		break;
	
	case EEnemyState::Ambush:
		TickAmbush();
		// If player is closer to end room than enemy
		if (SensedCharacter)
		{
			CurrentPath.Empty();
			CurrentState = EEnemyState::Engage;
		}
		else if (PathfindingSubsystem->GetDistance(Player->GetActorLocation(), EndNode->GetActorLocation())
			<= PathfindingSubsystem->GetDistance(GetActorLocation(), EndNode->GetActorLocation()))
		{
			CurrentPath.Empty();
			CurrentState = EEnemyState::Protect;
		}
		else if (PathfindingSubsystem->GetDistance(GetActorLocation(), Player->GetActorLocation()) > 7)
		{
			CurrentPath.Empty();
			CurrentState = EEnemyState::Patrol; // Switch to Engage when the player is not moving or is too close
		}
		else if (PathfindingSubsystem->GetDistance(GetActorLocation(), Player->GetActorLocation()) >= 6)
		{
			CurrentPath.Empty();
			CurrentState = EEnemyState::Investigate; // Implement the logic to move one node closer to the player
		}
		break;
	
	
	case EEnemyState::Protect:
		TickProtect();
		if (SensedCharacter)
		{
			CurrentPath.Empty();
			CurrentState = EEnemyState::Engage;
		}
		else if (PathfindingSubsystem->GetDistance(Player->GetActorLocation(), EndNode->GetActorLocation())
			> PathfindingSubsystem->GetDistance(GetActorLocation(), EndNode->GetActorLocation()))
		{
			if (PathfindingSubsystem->GetDistance(GetActorLocation(), Player->GetActorLocation()) > 7)
			{
				CurrentPath.Empty();
				CurrentState = EEnemyState::Patrol; // Switch to Engage when the player is not moving or is too close
			}
			// Equals 6 or 7
			else if (PathfindingSubsystem->GetDistance(GetActorLocation(), Player->GetActorLocation()) >= 6)
			{
				CurrentPath.Empty();
				CurrentState = EEnemyState::Investigate;
			}
			else if (PathfindingSubsystem->GetDistance(GetActorLocation(), Player->GetActorLocation()) >= 1)
			{
				CurrentPath.Empty();
				CurrentState = EEnemyState::Ambush; // Implement the logic to move one node closer to the player
			}
			break;
		}
	}

	//// DEBUG LOGS ////
	UE_LOG(LogTemp, Display, TEXT("Enemy State is currently: %s"), *UEnum::GetValueAsString(CurrentState));
	UE_LOG(LogTemp, Display, TEXT("Player and enemy node distance:        %f"), PathfindingSubsystem->GetDistance(GetActorLocation(), Player->GetActorLocation()))
	UE_LOG(LogTemp, Display, TEXT("Enemy to end room node distance:       %f"), PathfindingSubsystem->GetDistance(GetActorLocation(), EndNode->GetActorLocation()))
	UE_LOG(LogTemp, Display, TEXT("Player to end room node distance:      %f"), PathfindingSubsystem->GetDistance(Player->GetActorLocation(), EndNode->GetActorLocation()))
	if (Player->IsPlayerMoving())
	{
		UE_LOG(LogTemp, Display, TEXT("Player is currently moving"));
	}
	else
	{
		UE_LOG(LogTemp, Display, TEXT("Player is not moving"));
	}
}

void AEnemyCharacter::MoveAlongPath()
{
	const FVector CurrentLocation = GetActorLocation();
	if (CurrentPath.Num() > 0)
	{
		const FVector NextLocation = CurrentPath[CurrentPath.Num() - 1];
		// Calculate the direction from currentLocation to nextLocation
		const FVector Direction = (NextLocation - CurrentLocation).GetSafeNormal();
		AddMovementInput(Direction, 1.0f);

		FVector CurrentLocation2D = FVector(CurrentLocation.X, CurrentLocation.Y, 0);
		FVector NextLocation2D = FVector(NextLocation.X, NextLocation.Y, 0);

		if (const float Distance = FVector::Dist(CurrentLocation2D, NextLocation2D); Distance < 50.0f)
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
