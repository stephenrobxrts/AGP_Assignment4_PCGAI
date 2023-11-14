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
	
	WalkableNodes = PathfindingSubsystem->GetWalkableNodes();
	RoomNodes = PathfindingSubsystem->GetRoomNodes();
	
	CurrentPath = PathfindingSubsystem->GetRandomPath(Location, RoomNodes);

	// Get end node location in end room based off enemy player's starting position
	EndNode = PathfindingSubsystem->FindNearestNode(GetActorLocation(), RoomNodes);

	if (PawnSensingComponent)
	{
		PawnSensingComponent->OnSeePawn.AddDynamic(this, &AEnemyCharacter::OnSensedPawn);
	}	

	// Delay before finding the player character
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &AEnemyCharacter::FindPlayerCharacters, 1.0f, false);
}

void AEnemyCharacter::FindPlayerCharacters()
{
	// Find all the players in the game and add them to the players array
	for (TActorIterator<APlayerCharacter> It(GetWorld()); It; ++It)
	{
		UE_LOG(LogTemp, Display, TEXT("Found player %d"), PlayersArray.Num() + 1);
		PlayersArray.Add(*It);
	}
	if (PlayersArray.Num() == 0) // HARDCODE TO 2 LATER JUST IN CASE ONLY 1 PLAYER IS FOUND AND IT SKIPS 2ND
	{
		// Clear the array if 0 or only 1 player has been found (rare edge case)
		PlayersArray.Empty();
		// Players not found, retry after some delay
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &AEnemyCharacter::FindPlayerCharacters, 1.0f, false);
	}
	else
	{
		// Set target player to the first player in the players array
		TargetPlayerIndex = 0;
		TargetPlayer = PlayersArray[TargetPlayerIndex];
	}
}

int32 AEnemyCharacter::OtherTargetPlayerIndex()
{
	// Return index to other targeted Player (0 to 1, 1 to 0)
	return (TargetPlayerIndex + 1) % PlayersArray.Num();
}

void AEnemyCharacter::TickPatrol()
{
	// If current path is empty
	if (CurrentPath.IsEmpty())
	{
		// Generate random path from enemy player's position
		CurrentPath = PathfindingSubsystem->GetRandomPath(GetActorLocation(), RoomNodes);
	}
	// Move enemy player along path
	MoveAlongPath();
	
}

void AEnemyCharacter::TickEngage()
{
	// If current path is empty
	if (CurrentPath.IsEmpty())
	{
		if (SensedCharacter)
		{
			// If sensed character is not the target player -> switch target player
			if (SensedCharacter != TargetPlayer)
			{
				TargetPlayerIndex = OtherTargetPlayerIndex();
				UE_LOG(LogTemp, Display, TEXT("Switching Target Player to Player Number %d"), OtherTargetPlayerIndex() + 1);
				TargetPlayer = PlayersArray[TargetPlayerIndex];
			}
			// Generate path to target player
			CurrentPath = PathfindingSubsystem->GetPath(PathfindingSubsystem->FindNearestNode(GetActorLocation(), WalkableNodes),
				PathfindingSubsystem->FindNearestNode(SensedCharacter->GetActorLocation(), WalkableNodes), WalkableNodes);
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
		TArray<ANavigationNode*> ConnectedNodes = PathfindingSubsystem->FindNearestNode(GetActorLocation(), RoomNodes)->GetConnectedNodes();
		TArray<ANavigationNode*> PossibleNodes;
		// Loop through each connected node
		for (ANavigationNode* ConnectedNode : ConnectedNodes)
		{
			if (PathfindingSubsystem->GetPathLength(ConnectedNode->GetActorLocation(), TargetPlayer->GetActorLocation(), RoomNodes) <
				PathfindingSubsystem->GetPathLength(GetActorLocation(), TargetPlayer->GetActorLocation(), RoomNodes))
			{
				PossibleNodes.Add(ConnectedNode);
			}
		}
		// Rare but possible occurence, simply generate path from enemy to player
		if (PossibleNodes.IsEmpty())
		{
			CurrentPath = PathfindingSubsystem->GetPath(PathfindingSubsystem->FindNearestNode(GetActorLocation(), WalkableNodes),
			                                            PathfindingSubsystem->FindNearestNode(TargetPlayer->GetActorLocation(), WalkableNodes), WalkableNodes);
		}
		else
		{
			// Pick one of the available paths to bring enemy closer to player
			CurrentPath = PathfindingSubsystem->GetPath(GetActorLocation(), PossibleNodes[FMath::RandRange(0, PossibleNodes.Num()-1)]->GetActorLocation(), RoomNodes);			
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
		if (PathfindingSubsystem->GetPathLength(GetActorLocation(), TargetPlayer->GetActorLocation(), RoomNodes) <= 3)
		{
			CurrentPath = PathfindingSubsystem->GetPath(PathfindingSubsystem->FindNearestNode(GetActorLocation(), WalkableNodes),
			                                            PathfindingSubsystem->FindNearestNode(TargetPlayer->GetActorLocation(), WalkableNodes), WalkableNodes);
		}
		// Calculate intercept pathway
		else
		{
			// Store connected nodes in array (that connect to node nearest to enemy player)
			TArray<ANavigationNode*> ConnectedNodes = PathfindingSubsystem->FindNearestNode(GetActorLocation(), RoomNodes)->GetConnectedNodes();
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
				if (PathfindingSubsystem->GetPathLength(ConnectedNode->GetActorLocation(), EndNode->GetActorLocation(), RoomNodes)
					== PathfindingSubsystem->GetPathLength(GetActorLocation(), EndNode->GetActorLocation(), RoomNodes)
					&& ConnectedNode != PathfindingSubsystem->FindNearestNode(GetActorLocation(), RoomNodes))
				{
					// Set target node to node
					TargetNode = ConnectedNode;
				}
			}
			// If target node is not null
			if (TargetNode)
			{
				// Generate path from enemy player's node to target node
				CurrentPath = PathfindingSubsystem->GetPath(PathfindingSubsystem->FindNearestNode(GetActorLocation(), WalkableNodes),
				                                            PathfindingSubsystem->FindNearestNode(TargetPlayer->GetActorLocation(), WalkableNodes), WalkableNodes);
			}
			else
			{
				// Generate path from enemy player's node to end room node
				CurrentPath = PathfindingSubsystem->GetPath(PathfindingSubsystem->FindNearestNode(GetActorLocation(), WalkableNodes),
				                                            PathfindingSubsystem->FindNearestNode(EndNode->GetActorLocation(), WalkableNodes), WalkableNodes);		
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
		if (PathfindingSubsystem->GetPathLength(GetActorLocation(), EndNode->GetActorLocation(), RoomNodes)
			== PathfindingSubsystem->GetPathLength(TargetPlayer->GetActorLocation(), EndNode->GetActorLocation(), RoomNodes)
			&& PathfindingSubsystem->GetPathLength(GetActorLocation(), TargetPlayer->GetActorLocation(), RoomNodes) == 3)
		{
			CurrentPath = PathfindingSubsystem->GetPath(PathfindingSubsystem->FindNearestNode(GetActorLocation(), WalkableNodes),
			                                            PathfindingSubsystem->FindNearestNode(TargetPlayer->GetActorLocation(), WalkableNodes), WalkableNodes);
		}
		// Else, retreat back to end room 
		else
		{
			CurrentPath = PathfindingSubsystem->GetPath(PathfindingSubsystem->FindNearestNode(GetActorLocation(), WalkableNodes),
			                                            PathfindingSubsystem->FindNearestNode(EndNode->GetActorLocation(), WalkableNodes), WalkableNodes);			
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
		// If non-target player is closer to enemy than target player and non-target player is currently moving ("creating sound")
		if (PathfindingSubsystem->GetPathLength(PlayersArray[OtherTargetPlayerIndex()]->GetActorLocation(), GetActorLocation(), WalkableNodes)
				< PathfindingSubsystem->GetPathLength(TargetPlayer->GetActorLocation(), GetActorLocation(), WalkableNodes)
				&& PlayersArray[OtherTargetPlayerIndex()]->IsPlayerMoving())
		{
			// Switch target player
			TargetPlayerIndex = OtherTargetPlayerIndex();
			UE_LOG(LogTemp, Display, TEXT("Switching Target Player to Player Number %d"), OtherTargetPlayerIndex() + 1);
			TargetPlayer = PlayersArray[TargetPlayerIndex];
		}
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
			else if (PathfindingSubsystem->GetPathLength(TargetPlayer->GetActorLocation(), EndNode->GetActorLocation(), RoomNodes)
					<= PathfindingSubsystem->GetPathLength(GetActorLocation(), EndNode->GetActorLocation(), RoomNodes))
			{
				CurrentPath.Empty();
				CurrentState = EEnemyState::Protect;
			}
			else
			{
				// Get node count between enemy and player
				float DistanceToPlayer = PathfindingSubsystem->GetPathLength(GetActorLocation(), TargetPlayer->GetActorLocation(), RoomNodes);
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
				if (PathfindingSubsystem->GetPathLength(TargetPlayer->GetActorLocation(), EndNode->GetActorLocation(), RoomNodes)
					<= PathfindingSubsystem->GetPathLength(GetActorLocation(), EndNode->GetActorLocation(), RoomNodes))
				{
					CurrentPath.Empty();
					CurrentState = EEnemyState::Protect;
				}
				// Although enemy could never go from nearby to greater than 7 nodes away from the player, just for precautions if SensesCharacter view distance is altered
				else if (PathfindingSubsystem->GetPathLength(GetActorLocation(), TargetPlayer->GetActorLocation(), RoomNodes) > 7)
				{
					CurrentPath.Empty();
					CurrentState = EEnemyState::Patrol; 
				}
				// Else if enemy is 6 or 7 nodes away from the player
				else if (PathfindingSubsystem->GetPathLength(GetActorLocation(), TargetPlayer->GetActorLocation(), RoomNodes) >= 6)
				{
					CurrentPath.Empty();
					CurrentState = EEnemyState::Investigate;
				}
				// Else if enemy is 1, 2, 3, 4, or 5 nodes away from the player
				else if (PathfindingSubsystem->GetPathLength(GetActorLocation(), TargetPlayer->GetActorLocation(), RoomNodes) >= 1)
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
			else if (PathfindingSubsystem->GetPathLength(TargetPlayer->GetActorLocation(), EndNode->GetActorLocation(), RoomNodes)
				<= PathfindingSubsystem->GetPathLength(GetActorLocation(), EndNode->GetActorLocation(), RoomNodes))
			{
				CurrentPath.Empty();
				CurrentState = EEnemyState::Protect;
			}
			// Else if enemy is greater than 7 nodes away from the player
			else if (PathfindingSubsystem->GetPathLength(GetActorLocation(), TargetPlayer->GetActorLocation(), RoomNodes) > 7)
			{
				CurrentPath.Empty();
				CurrentState = EEnemyState::Patrol;
			}
			// Else if enemy is 1, 2, 3, 4, or 5 nodes away from the player and the player is currently moving ("creating sound")
			else if (PathfindingSubsystem->GetPathLength(GetActorLocation(), TargetPlayer->GetActorLocation(), RoomNodes) <= 5 && TargetPlayer->IsPlayerMoving())
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
			else if (PathfindingSubsystem->GetPathLength(TargetPlayer->GetActorLocation(), EndNode->GetActorLocation(), RoomNodes)
				<= PathfindingSubsystem->GetPathLength(GetActorLocation(), EndNode->GetActorLocation(), RoomNodes))
			{
				CurrentPath.Empty();
				CurrentState = EEnemyState::Protect;
			}
			// Else if enemy is greater than 7 nodes away from the player
			else if (PathfindingSubsystem->GetPathLength(GetActorLocation(), TargetPlayer->GetActorLocation(), RoomNodes) > 7)
			{
				CurrentPath.Empty();
				CurrentState = EEnemyState::Patrol;
			}
			// Else if enemy is 6 or 7 nodes away from the player
			else if (PathfindingSubsystem->GetPathLength(GetActorLocation(), TargetPlayer->GetActorLocation(), RoomNodes) >= 6)
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
			else if (PathfindingSubsystem->GetPathLength(TargetPlayer->GetActorLocation(), EndNode->GetActorLocation(), RoomNodes)
				> PathfindingSubsystem->GetPathLength(GetActorLocation(), EndNode->GetActorLocation(), RoomNodes))
			{
				// If enemy is greater than 7 nodes away from the player
				if (PathfindingSubsystem->GetPathLength(GetActorLocation(), TargetPlayer->GetActorLocation(), RoomNodes) > 7)
				{
					CurrentPath.Empty();
					CurrentState = EEnemyState::Patrol;
				}
				// Else if enemy is 6 or 7 nodes away from the player
				else if (PathfindingSubsystem->GetPathLength(GetActorLocation(), TargetPlayer->GetActorLocation(), RoomNodes) >= 6)
				{
					CurrentPath.Empty();
					CurrentState = EEnemyState::Investigate;
				}
				// Else enemy is 1, 2, 3, 4, or 5 nodes away from the player
				else if (PathfindingSubsystem->GetPathLength(GetActorLocation(), TargetPlayer->GetActorLocation(), RoomNodes) >= 1)
				{
					CurrentPath.Empty();
					CurrentState = EEnemyState::Ambush;
				}
				break;
			}
		}

		//// DEBUG LOGS ////
		UE_LOG(LogTemp, Display, TEXT("Enemy State is currently: %s"), *UEnum::GetValueAsString(CurrentState));
		UE_LOG(LogTemp, Display, TEXT("Player and enemy node distance:        %f"), PathfindingSubsystem->GetPathLength(GetActorLocation(), TargetPlayer->GetActorLocation(), RoomNodes))
		UE_LOG(LogTemp, Display, TEXT("Enemy to end room node distance:       %f"), PathfindingSubsystem->GetPathLength(GetActorLocation(), EndNode->GetActorLocation(), RoomNodes))
		UE_LOG(LogTemp, Display, TEXT("Player to end room node distance:      %f"), PathfindingSubsystem->GetPathLength(TargetPlayer->GetActorLocation(), EndNode->GetActorLocation(), RoomNodes))
		if (TargetPlayer->IsPlayerMoving())
		{
			UE_LOG(LogTemp, Display, TEXT("Player is moving"));
		}
		else
		{
			UE_LOG(LogTemp, Display, TEXT("Player is not moving"));
		}

		UE_LOG(LogTemp, Display, TEXT("Room Node Count      %d"), RoomNodes.Num());
		UE_LOG(LogTemp, Display, TEXT("Walkable Node Count      %d"), WalkableNodes.Num());
		UE_LOG(LogTemp, Display, TEXT("Player Count:     %d"), PlayersArray.Num());
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
