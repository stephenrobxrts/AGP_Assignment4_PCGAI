// Fill out your copyright notice in the Description page of Project Settings.


#include "PathfindingSubsystem.h"

#include "NavigationSystem.h"


void UPathfindingSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);

	UE_LOG(LogTemp, Warning, TEXT("World Begin Play"))
	PopulateNodes();
}

TArray<FVector> UPathfindingSubsystem::GetRandomPath(const FVector& StartLocation)
{
	ANavigationNode* StartNode = FindNearestNode(StartLocation);
	ANavigationNode* EndNode = GetRandomNode();
	if (StartNode && EndNode)
	{
		return GetPath(StartNode, EndNode);
	}
	return TArray<FVector>();
}

TArray<FVector> UPathfindingSubsystem::GetPath(const FVector& StartLocation, const FVector& TargetLocation)
{
	ANavigationNode* StartNode = FindNearestNode(StartLocation);
	ANavigationNode* EndNode = FindNearestNode(TargetLocation);
	if (StartNode && EndNode)
	{
		return GetPath(StartNode, EndNode);
	}
	return TArray<FVector>();
}

TArray<FVector> UPathfindingSubsystem::GetPathAway(const FVector& StartLocation, const FVector& TargetLocation)
{
	ANavigationNode* StartNode = FindNearestNode(StartLocation);
	ANavigationNode* EndNode = FindFurthestNode(TargetLocation);
	if (StartNode && EndNode)
	{
		return GetPath(StartNode, EndNode);
	}
	return TArray<FVector>();
}

TArray<FVector> UPathfindingSubsystem::GetWaypointPositions()
{
	//Return an array of all the nodes' locations
	TArray<FVector> WaypointPositions;
	for (int i = 0; i < Nodes.Num(); i++)
	{
		WaypointPositions.Add(Nodes[i]->GetActorLocation());
	}
	return WaypointPositions;
}

void UPathfindingSubsystem::PopulateNodes()
{
	//empty the node array
	Nodes.Empty();

	//Find all ANavigationNode actors add them to the array
	for (TActorIterator<ANavigationNode> It(GetWorld()); It; ++It)
	{
		Nodes.Add(*It);
	}

	//debug text for each node
	for (int i = 0; i < Nodes.Num(); i++)
	{
		//Debug - show node locations
		UE_LOG(LogTemp, Warning, TEXT("Node: %s"), *Nodes[i]->GetActorLocation().ToString());
		///Debug
	}
}

ANavigationNode* UPathfindingSubsystem::GetRandomNode()
{
	if (Nodes.Num() > 0)
	{
		int RandomIndex = FMath::RandRange(0, Nodes.Num() - 1);
		return Nodes[RandomIndex];
	}
	return nullptr;
}

ANavigationNode* UPathfindingSubsystem::FindNearestNode(const FVector& TargetLocation)
{
	//Loop through nodes, get node with minimum distance from target
	float MinDistance = UE_MAX_FLT;
	ANavigationNode* NearestNode = nullptr;
	for (int i = 0; i < Nodes.Num(); i++)
	{
		FVector NodeLocation = Nodes[i]->GetActorLocation();
		float Distance = FVector::Dist(TargetLocation, NodeLocation);
		if (Distance < MinDistance)
		{
			MinDistance = Distance;
			NearestNode = Nodes[i];
		}
	}
	return NearestNode;
}

ANavigationNode* UPathfindingSubsystem::FindFurthestNode(const FVector& TargetLocation)
{
	//Loop through nodes, get node with minimum distance from target
	float MaxDistance = 0.0f;
	ANavigationNode* FurthestNode = nullptr;
	for (int i = 0; i < Nodes.Num(); i++)
	{
		float Distance = FVector::Dist(TargetLocation, Nodes[i]->GetActorLocation());
		if (Distance > MaxDistance)
		{
			MaxDistance = Distance;
			FurthestNode = Nodes[i];
		}
	}
	return FurthestNode;
}

TArray<FVector> UPathfindingSubsystem::GetPath(ANavigationNode* StartNode, ANavigationNode* EndNode)
{
	//Use A* to find path
	TArray<ANavigationNode*> OpenSet;
	OpenSet.Add(StartNode);

	TArray<FVector> Path;
	// Set all Gscores to infinity and the HScores to correct heuristic
	for (int i = 0; i < Nodes.Num(); i++)
	{
		Nodes[i]->GScore = UE_MAX_FLT - 1;
		Nodes[i]->HScore = FVector::Dist(Nodes[i]->GetActorLocation(), EndNode->GetActorLocation());
		Nodes[i]->FScore = Nodes[i]->GScore + Nodes[i]->HScore;
	}

	//Set the startNode GScore to 0
	for (int i = 0; i < Nodes.Num(); i++)
	{
		if (Nodes[i] == StartNode)
		{
			Nodes[i]->GScore = 0;
			Nodes[i]->FScore = Nodes[i]->GScore + Nodes[i]->HScore;
		}
	}

	//While the open set is not empty
	while (OpenSet.Num() > 0)
	{
		//Set current node to node with lowest F score.
		float MinFScore = UE_MAX_FLT;
		ANavigationNode* CurrentNode = nullptr;
		for (int i = 0; i < OpenSet.Num(); i++)
		{
			if (OpenSet[i]->GScore + OpenSet[i]->FScore <= MinFScore)
			{
				MinFScore = OpenSet[i]->GScore + OpenSet[i]->HScore;
				CurrentNode = OpenSet[i];
				break;
			}
		}

		OpenSet.Remove(CurrentNode);

		//reconstruct path if current node is the end node
		if (CurrentNode == EndNode)
		{
			return ReconstructPath(StartNode, EndNode);
		}

		//for each connectedNode in CurrentNode: Generate tentative GScore for each neighbour
		for (int i = 0; i < CurrentNode->GetConnectedNodes().Num(); i++)
		{
			ANavigationNode* ConnectedNode = CurrentNode->GetConnectedNodes()[i];
			float TentativeGScore = CurrentNode->GScore + FVector::Dist(CurrentNode->GetActorLocation(),
			                                                            ConnectedNode->GetActorLocation());
			//if tentative GScore is less than connectedNode GScore, set connectedNode GScore to tentative GScore and set connectedNode parent to current node
			if (TentativeGScore < ConnectedNode->GScore)
			{
				ConnectedNode->ParentNode = CurrentNode;
				ConnectedNode->GScore = TentativeGScore;
				ConnectedNode->HScore = FVector::Dist(ConnectedNode->GetActorLocation(), EndNode->GetActorLocation());
				ConnectedNode->FScore = ConnectedNode->GScore + ConnectedNode->HScore;
				//if connectedNode is not in the open set, add it
				if (!OpenSet.Contains(ConnectedNode))
				{
					OpenSet.Add(ConnectedNode);
				}
			}
		}
	}
	//If it leaves this loop it has not found a valid path
	return Path;
}

float UPathfindingSubsystem::GetDistance(const FVector& StartLocation, const FVector& TargetLocation)
{
	return GetPath(FindNearestNode(StartLocation), FindNearestNode(TargetLocation)).Num();

}

TArray<FVector> UPathfindingSubsystem::ReconstructPath(const ANavigationNode* StartNode, ANavigationNode* EndNode)
{
	//Create an array of vectors to store the path
	TArray<FVector> Path;
	Path.Add(EndNode->GetActorLocation());
	ANavigationNode* CurrentNode = EndNode;
	while (CurrentNode != StartNode)
	{
		CurrentNode = CurrentNode->ParentNode;
		Path.Add(CurrentNode->GetActorLocation());
	}

	return Path;
}
