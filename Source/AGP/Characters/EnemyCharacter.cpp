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
	
	// Do nothing if not on the server
	if (GetLocalRole() != ROLE_Authority) return;
	
	FVector Location = GetActorLocation();
	PathfindingSubsystem = GetWorld()->GetSubsystem<UPathfindingSubsystem>();
	CurrentPath = PathfindingSubsystem->GetRandomPath(Location);

	// Get end node location in end room based off enemy player's starting position
	EndNode = PathfindingSubsystem->FindNearestNode(GetActorLocation());

	if (PawnSensingComponent)
	{
		PawnSensingComponent->OnSeePawn.AddDynamic(this, &AEnemyCharacter::OnSensedPawn);
	}	

	// Delay before finding the player character
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &AEnemyCharacter::FindPlayerCharacter, 1.0f, false);
}

void AEnemyCharacter::FindPlayerCharacter()
{
	// Get a reference to the player character
	for (TActorIterator<APlayerCharacter> It(GetWorld()); It; ++It)
	{
		TargetPlayer = *It;
		if (TargetPlayer)
		{
			// Found player and player location
			UE_LOG(LogTemp, Display, TEXT("Finally found the player"));
			break;
		}
	}
	if (!TargetPlayer)
	{
		// Player not found, retry after some delay
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &AEnemyCharacter::FindPlayerCharacter, 1.0f, false);
	}
}

void AEnemyCharacter::SetInitialTarget()
{
	TargetPlayerIndex = DefaultTargetIndex;
}

void AEnemyCharacter::UpdatePlayersArray()
{
	// Clear the array
	PlayersArray.Empty();

	// Find all the players in the game and add them to the array
	for (TActorIterator<APlayerCharacter> It(GetWorld()); It; ++It)
	{
		PlayersArray.Add(*It);
	}
}
void AEnemyCharacter::SwitchTarget()
{
	// Toggle between targeting Player 1 and Player 2 (0 to 1, 1 to 0)
	TargetPlayerIndex = (TargetPlayerIndex + 1) % PlayersArray.Num();
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
		if (SensedCharacter == TargetPlayer)
		{
			CurrentPath = PathfindingSubsystem->GetPath(GetActorLocation(), SensedCharacter->GetActorLocation());
		}
		else
		{
			// SWITCH TARGET
			UE_LOG(LogTemp, Display, TEXT("IM SENSING THE OTHER PLAYER I NEED TO SWITCH TARGET!!!!"));
			
		}
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
			if (PathfindingSubsystem->GetPathLength(ConnectedNode->GetActorLocation(), TargetPlayer->GetActorLocation()) <
				PathfindingSubsystem->GetPathLength(GetActorLocation(), TargetPlayer->GetActorLocation()))
			{
				PossibleNodes.Add(ConnectedNode);
			}
		}
		// Rare but possible occurence, simply generate path from enemy to player
		if (PossibleNodes.IsEmpty())
		{
			CurrentPath = PathfindingSubsystem->GetPath(GetActorLocation(), TargetPlayer->GetActorLocation());
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
		if (PathfindingSubsystem->GetPathLength(GetActorLocation(), TargetPlayer->GetActorLocation()) <= 3)
		{
			CurrentPath = PathfindingSubsystem->GetPath(GetActorLocation(), TargetPlayer->GetActorLocation());
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
				if (PathfindingSubsystem->GetPathLength(ConnectedNode->GetActorLocation(), EndNode->GetActorLocation())
					== PathfindingSubsystem->GetPathLength(GetActorLocation(), EndNode->GetActorLocation())
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
			CurrentPath.SetNum(2);
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
		// If enemy is equal distance away from end room as the player, move towards player first to try and engage
		if (PathfindingSubsystem->GetPathLength(GetActorLocation(), EndNode->GetActorLocation())
			== PathfindingSubsystem->GetPathLength(TargetPlayer->GetActorLocation(), EndNode->GetActorLocation())
			&& PathfindingSubsystem->GetPathLength(GetActorLocation(), TargetPlayer->GetActorLocation()) == 3)
		{
			CurrentPath = PathfindingSubsystem->GetPath(GetActorLocation(), TargetPlayer->GetActorLocation());
		}
		// Else, retreat back to end room 
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
	Super::Tick(DeltaTime);
	// Do nothing if not on the server
	if (GetLocalRole() != ROLE_Authority) return;
    // If target player has been found after server and clients have loaded
	if (TargetPlayer)
	{
		UpdateSight();
		switch (CurrentState)
		{
		// If enemy state is patrol
		case EEnemyState::Patrol:
			TickPatrol();
			// If player is sensed by enemy player
			if (SensedCharacter)
			{
				CurrentPath.Empty();
				CurrentState = EEnemyState::Engage;
			}
			// Else if player is closer or of equal distance away from the end room to the enemy
			else if (PathfindingSubsystem->GetPathLength(TargetPlayer->GetActorLocation(), EndNode->GetActorLocation())
					<= PathfindingSubsystem->GetPathLength(GetActorLocation(), EndNode->GetActorLocation()))
			{
				CurrentPath.Empty();
				CurrentState = EEnemyState::Protect;
			}
			else
			{
				// Get node count between enemy and player
				float DistanceToPlayer = PathfindingSubsystem->GetPathLength(GetActorLocation(), TargetPlayer->GetActorLocation());
				// If node distance is 6 or 7 and the player is currently moving ("creating sound")
				if ((DistanceToPlayer == 6 || DistanceToPlayer == 7) && TargetPlayer->IsPlayerMoving())
				{
					CurrentPath.Empty();
					CurrentState = EEnemyState::Investigate;
				}
				// Else if node distance is less than 5 and the player is currently moving ("creating sound")
				else if (DistanceToPlayer <= 5 && TargetPlayer->IsPlayerMoving())
				{
					CurrentPath.Empty();
					CurrentState = EEnemyState::Ambush;
				}
			}
			break;
		// If enemy state is Engage
		case EEnemyState::Engage:
			TickEngage();
			// If enemy has lost sense of player
			if (!SensedCharacter)
			{
				// If player is closer or of equal distance away from the end room to the enemy
				if (PathfindingSubsystem->GetPathLength(TargetPlayer->GetActorLocation(), EndNode->GetActorLocation())
					<= PathfindingSubsystem->GetPathLength(GetActorLocation(), EndNode->GetActorLocation()))
				{
					CurrentPath.Empty();
					CurrentState = EEnemyState::Protect;
				}
				// Although enemy could never go from nearby to greater than 7 nodes away from the player, just for precautions if SensesCharacter view distance is altered
				else if (PathfindingSubsystem->GetPathLength(GetActorLocation(), TargetPlayer->GetActorLocation()) > 7)
				{
					CurrentPath.Empty();
					CurrentState = EEnemyState::Patrol; 
				}
				// Else if enemy is 6 or 7 nodes away from the player
				else if (PathfindingSubsystem->GetPathLength(GetActorLocation(), TargetPlayer->GetActorLocation()) >= 6)
				{
					CurrentPath.Empty();
					CurrentState = EEnemyState::Investigate;
				}
				// Else if enemy is 1, 2, 3, 4, or 5 nodes away from the player
				else if (PathfindingSubsystem->GetPathLength(GetActorLocation(), TargetPlayer->GetActorLocation()) >= 1)
				{
					CurrentPath.Empty();
					CurrentState = EEnemyState::Ambush; 
				}
				break;
			}
			break;
		// If enemy state is Investigate
		case EEnemyState::Investigate:
			TickInvestigate();
			// If player is sensed by enemy player
			if (SensedCharacter)
			{
				CurrentPath.Empty();
				CurrentState = EEnemyState::Engage;
			}
			// Else if player is closer or of equal distance away from the end room to the enemy
			else if (PathfindingSubsystem->GetPathLength(TargetPlayer->GetActorLocation(), EndNode->GetActorLocation())
				<= PathfindingSubsystem->GetPathLength(GetActorLocation(), EndNode->GetActorLocation()))
			{
				CurrentPath.Empty();
				CurrentState = EEnemyState::Protect;
			}
			// Else if enemy is greater than 7 nodes away from the player
			else if (PathfindingSubsystem->GetPathLength(GetActorLocation(), TargetPlayer->GetActorLocation()) > 7)
			{
				CurrentPath.Empty();
				CurrentState = EEnemyState::Patrol;
			}
			// Else if enemy is 1, 2, 3, 4, or 5 nodes away from the player and the player is currently moving ("creating sound")
			else if (PathfindingSubsystem->GetPathLength(GetActorLocation(), TargetPlayer->GetActorLocation()) <= 5 && TargetPlayer->IsPlayerMoving())
			{
				CurrentPath.Empty();
				CurrentState = EEnemyState::Ambush;
			}
			break;
		// If enemy state is Ambush
		case EEnemyState::Ambush:
			TickAmbush();
			// If player is sensed by enemy player
			if (SensedCharacter)
			{
				CurrentPath.Empty();
				CurrentState = EEnemyState::Engage;
			}
			// Else if player is closer or of equal distance away from the end room to the enemy
			else if (PathfindingSubsystem->GetPathLength(TargetPlayer->GetActorLocation(), EndNode->GetActorLocation())
				<= PathfindingSubsystem->GetPathLength(GetActorLocation(), EndNode->GetActorLocation()))
			{
				CurrentPath.Empty();
				CurrentState = EEnemyState::Protect;
			}
			// Else if enemy is greater than 7 nodes away from the player
			else if (PathfindingSubsystem->GetPathLength(GetActorLocation(), TargetPlayer->GetActorLocation()) > 7)
			{
				CurrentPath.Empty();
				CurrentState = EEnemyState::Patrol;
			}
			// Else if enemy is 6 or 7 nodes away from the player
			else if (PathfindingSubsystem->GetPathLength(GetActorLocation(), TargetPlayer->GetActorLocation()) >= 6)
			{
				CurrentPath.Empty();
				CurrentState = EEnemyState::Investigate;
			}
			break;
		// If enemy state is Protect
		case EEnemyState::Protect:
			TickProtect();
			// If player is sensed by enemy player
			if (SensedCharacter)
			{
				CurrentPath.Empty();
				CurrentState = EEnemyState::Engage;
			}
			// If player is further away from the end room to the enemy
			else if (PathfindingSubsystem->GetPathLength(TargetPlayer->GetActorLocation(), EndNode->GetActorLocation())
				> PathfindingSubsystem->GetPathLength(GetActorLocation(), EndNode->GetActorLocation()))
			{
				// If enemy is greater than 7 nodes away from the player
				if (PathfindingSubsystem->GetPathLength(GetActorLocation(), TargetPlayer->GetActorLocation()) > 7)
				{
					CurrentPath.Empty();
					CurrentState = EEnemyState::Patrol;
				}
				// Else if enemy is 6 or 7 nodes away from the player
				else if (PathfindingSubsystem->GetPathLength(GetActorLocation(), TargetPlayer->GetActorLocation()) >= 6)
				{
					CurrentPath.Empty();
					CurrentState = EEnemyState::Investigate;
				}
				// Else enemy is 1, 2, 3, 4, or 5 nodes away from the player
				else if (PathfindingSubsystem->GetPathLength(GetActorLocation(), TargetPlayer->GetActorLocation()) >= 1)
				{
					CurrentPath.Empty();
					CurrentState = EEnemyState::Ambush;
				}
				break;
			}
		}

		//// DEBUG LOGS ////
		UE_LOG(LogTemp, Display, TEXT("Enemy State is currently: %s"), *UEnum::GetValueAsString(CurrentState));
		UE_LOG(LogTemp, Display, TEXT("Player and enemy node distance:        %f"), PathfindingSubsystem->GetPathLength(GetActorLocation(), TargetPlayer->GetActorLocation()))
		UE_LOG(LogTemp, Display, TEXT("Enemy to end room node distance:       %f"), PathfindingSubsystem->GetPathLength(GetActorLocation(), EndNode->GetActorLocation()))
		UE_LOG(LogTemp, Display, TEXT("Player to end room node distance:      %f"), PathfindingSubsystem->GetPathLength(TargetPlayer->GetActorLocation(), EndNode->GetActorLocation()))
		if (TargetPlayer->IsPlayerMoving())
		{
			UE_LOG(LogTemp, Display, TEXT("Player is moving"));
		}
		else
		{
			UE_LOG(LogTemp, Display, TEXT("Player is not moving"));
		}
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
