// Fill out your copyright notice in the Description page of Project Settings.


#include "NavigationNode.h"

// Sets default values
ANavigationNode::ANavigationNode()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	;
	LocationComponent = CreateDefaultSubobject<USceneComponent>(TEXT("LocationComponent"));
	SetRootComponent(LocationComponent);
	
}

bool ANavigationNode::ShouldTickIfViewportsOnly() const
{
	return true;
}

TArray<ANavigationNode*> ANavigationNode::GetConnectedNodes()
{
	return ConnectedNodes;
}

void ANavigationNode::SetConnectedNodes(ANavigationNode* NewConnectedNode)
{
	ConnectedNodes.Add(NewConnectedNode);
}

void ANavigationNode::DestroyNode()
{
	for (ANavigationNode* ConnectedNode : ConnectedNodes)
	{
		ConnectedNode->ConnectedNodes.Remove(this);
	}

	// Destroy this actor
	Destroy();
}

// Called when the game starts or when spawned
void ANavigationNode::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ANavigationNode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (ConnectedNodes.Num() == 0){
		DrawDebugSphere(GetWorld(), GetActorLocation(), 25.0f, 4, FColor::Red, false, -1.0f, 0, 1.0f);
	}
	if(ConnectedNodes.Num() > 0)
	{
		DrawDebugSphere(GetWorld(), GetActorLocation(), 25.0f, 4, FColor::Green, false, -1.0f, 0, 1.0f);
	}

	//Draw lines, red if not reciprocated
	for (int i = 0; i < ConnectedNodes.Num(); i++)
	{
		if(ConnectedNodes[i] != nullptr)
		{
			FColor Color = FColor::Green;
			if (!CheckReciprocalConnection(this, ConnectedNodes[i]))
			{
				Color = FColor::Red;
			}
			DrawDebugLine(GetWorld(), GetActorLocation(), ConnectedNodes[i]->GetActorLocation(), Color, false, -1.0f, 0, 1.0f);
		}
		
	}
}

bool ANavigationNode::CheckReciprocalConnection(ANavigationNode* NodeA, ANavigationNode* NodeB)
{
	if (NodeA->ConnectedNodes.Contains(NodeB) && NodeB->ConnectedNodes.Contains(NodeA))
	{
		return true;
	}
	return false;

}

