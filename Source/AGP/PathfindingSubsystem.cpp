// Fill out your copyright notice in the Description page of Project Settings.


#include "PathfindingSubsystem.h"



void UPathfindingSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);

	UE_LOG(LogTemp, Warning, TEXT("World Begin Play"))
	PopulateNodes();
	
	
}

TArray<FVector> UPathfindingSubsystem::GetRandomPath(FVector& StartLocation)
{
	return TArray<FVector>();
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
