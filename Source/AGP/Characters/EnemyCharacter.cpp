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
	 		//found player and player location
	 		UE_LOG(LogTemp, Display, TEXT("Found player"));
	 		break;
	 	}
	 }
	EndNode = PathfindingSubsystem->FindNearestNode(GetActorLocation());

	if (PawnSensingComponent)
	{
		PawnSensingComponent->OnSeePawn.AddDynamic(this, &AEnemyCharacter::OnSensedPawn);
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
	}
}

void AEnemyCharacter::TickEngage()
{
	if (CurrentPath.Num() <= 0)
	{
		if (SensedCharacter)
		{
			CurrentPath = PathfindingSubsystem->GetPath(GetActorLocation(), SensedCharacter->GetActorLocation());

			Fire(SensedCharacter->GetActorLocation());
		}
	}
	else
	{
		MoveAlongPath();
		if (SensedCharacter)
		{			
			Fire(SensedCharacter->GetActorLocation());
		}
	}
	if (SensedCharacter)
	{
		FVector DirectionToPlayer = (SensedCharacter->GetActorLocation() - GetActorLocation()).GetSafeNormal();

		// Use LookAt to compute the rotation
		FRotator NewRotation = FRotationMatrix::MakeFromX(DirectionToPlayer).Rotator();

		// Set the enemy character's rotation
		SetActorRotation(NewRotation);		
	}

	//if weapon is empty, reload
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
	if (CurrentPath.Num() == 0)
	{
		// Store connected nodes in array
		TArray<ANavigationNode*> ConnectedNodes = PathfindingSubsystem->FindNearestNode(GetActorLocation())->GetConnectedNodes();
		// Create array for second level connections (in case initial node is a room and you need to determine surrounding rooms)
		TArray<ANavigationNode*> SecondLevelConnectedNodes;
		// For each loop iterating through each connected node
		for (ANavigationNode* ConnectedNode : ConnectedNodes)
		{
			// Add second level connection nodes to array
			SecondLevelConnectedNodes.Append(ConnectedNode->GetConnectedNodes());
		}

		ANavigationNode* TargetNode = nullptr;
		for (ANavigationNode* ConnectedNode : SecondLevelConnectedNodes)
		{
			if (PathfindingSubsystem->GetDistance(ConnectedNode->GetActorLocation(), EndNode->GetActorLocation())
				== PathfindingSubsystem->GetDistance(GetActorLocation(), EndNode->GetActorLocation())
				&& ConnectedNode != PathfindingSubsystem->FindNearestNode(GetActorLocation()))
			{
				TargetNode = ConnectedNode;
			}
		}
		if (TargetNode)
		{
			CurrentPath = PathfindingSubsystem->GetPath(GetActorLocation(), TargetNode->GetActorLocation());
		}
		else
		{
			CurrentPath = PathfindingSubsystem->GetPath(GetActorLocation(), Player->GetActorLocation());			
		}
		MoveAlongPath();
	}
	else
	{
		MoveAlongPath();
	}
}

void AEnemyCharacter::TickAmbush()
{
	// If current path is empty
	if (CurrentPath.Num() == 0)
	{
		// Store connected nodes in array
		TArray<ANavigationNode*> ConnectedNodes = PathfindingSubsystem->FindNearestNode(GetActorLocation())->GetConnectedNodes();
		// Create array for second level connections (in case initial node is a room and you need to determine surrounding rooms)
		TArray<ANavigationNode*> SecondLevelConnectedNodes;
		// For each loop iterating through each connected node
		for (ANavigationNode* ConnectedNode : ConnectedNodes)
		{
			// Add second level connection nodes to array
			SecondLevelConnectedNodes.Append(ConnectedNode->GetConnectedNodes());
		}

		//if player is one away from enemy and enemy moving to player makes enemy further away from end (that means enemy is in front of player)

		if (PathfindingSubsystem->GetDistance(Player->GetActorLocation(), EndNode->GetActorLocation())
			> PathfindingSubsystem->GetDistance(GetActorLocation(), EndNode->GetActorLocation()))
		{
			CurrentPath = PathfindingSubsystem->GetPath(GetActorLocation(), Player->GetActorLocation());
		}
		else
		{
			ANavigationNode* TargetNode = nullptr;
			for (ANavigationNode* ConnectedNode : SecondLevelConnectedNodes)
			{
				if (PathfindingSubsystem->GetDistance(ConnectedNode->GetActorLocation(), EndNode->GetActorLocation())
					== PathfindingSubsystem->GetDistance(GetActorLocation(), EndNode->GetActorLocation())
					&& ConnectedNode != PathfindingSubsystem->FindNearestNode(GetActorLocation()))
				{
					TargetNode = ConnectedNode;
				}
			}
			if (TargetNode)
			{
				CurrentPath = PathfindingSubsystem->GetPath(GetActorLocation(), TargetNode->GetActorLocation());
			}
			else
			{
				CurrentPath = PathfindingSubsystem->GetPath(GetActorLocation(), EndNode->GetActorLocation());			
			}
			// Sets array size to first node so it rechecks above stuff after each node 
			CurrentPath.SetNum(2);
			MoveAlongPath();
		}
	
	}
	else
	{
		MoveAlongPath();
	}
}

void AEnemyCharacter::TickProtect()
{
	if (CurrentPath.Num() == 0)
	{
		// If equal distance away from end room as player and in middle connecting rooms corridors, move towards player first to try and engage
		if (PathfindingSubsystem->GetDistance(GetActorLocation(), EndNode->GetActorLocation())
			== PathfindingSubsystem->GetDistance(Player->GetActorLocation(), EndNode->GetActorLocation())
			&& PathfindingSubsystem->GetDistance(GetActorLocation(), Player->GetActorLocation()) == 3)
		{
			CurrentPath = PathfindingSubsystem->GetPath(GetActorLocation(), Player->GetActorLocation());
		}
		else
		{
			CurrentPath = PathfindingSubsystem->GetPath(GetActorLocation(), EndNode->GetActorLocation());			
		}
		MoveAlongPath();
	}
	else
	{
		MoveAlongPath();
	}
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

//add transitions from engage to and patrol to the other 3 based on sensed character and distance to end node
//consider if intercept is necessary
//program logic

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
	// If player 
	
	UE_LOG(LogTemp, Display, TEXT("Enemy current state is %s"), *UEnum::GetValueAsString(CurrentState));
	UE_LOG(LogTemp, Display, TEXT("Player and enemy are %f nodes away from each other"), PathfindingSubsystem->GetDistance(GetActorLocation(), Player->GetActorLocation()))
	UE_LOG(LogTemp, Display, TEXT("Enemy is %f nodes away from the end room"), PathfindingSubsystem->GetDistance(GetActorLocation(), EndNode->GetActorLocation()))
	UE_LOG(LogTemp, Display, TEXT("Player is %f nodes away from the end room"), PathfindingSubsystem->GetDistance(Player->GetActorLocation(), EndNode->GetActorLocation()))
	if (Player->IsPlayerMoving())
	{
		UE_LOG(LogTemp, Display, TEXT("Player is currently moving"));
	}
	else
	{
		UE_LOG(LogTemp, Display, TEXT("Player is not moving"));
	}
	

	//UE_LOG(LogTemp, Display, TEXT("Enemy previous state is %s"), *UEnum::GetValueAsString(PreviousState));
	
	//if (CurrentState != PreviousState)
	//{
	//	UE_LOG(LogTemp, Display, TEXT("Enemy state changed from %s to %s"),
	//		*UEnum::GetValueAsString(CurrentState), *UEnum::GetValueAsString(PreviousState));
	//	PreviousState = CurrentState;
	//
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
