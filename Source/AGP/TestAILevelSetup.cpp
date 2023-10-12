// Fill out your copyright notice in the Description page of Project Settings.


#include "TestAILevelSetup.h"

#include "EngineUtils.h"
#include "Components/BoxComponent.h"
#include "DynamicMesh/ColliderMesh.h"
#include "Engine/StaticMeshActor.h"


// Sets default values
ATestAILevelSetup::ATestAILevelSetup()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

bool ATestAILevelSetup::ShouldTickIfViewportsOnly() const
{
	return true;
}


bool IsNodeInsideBox(ANavigationNode* Node, const UBoxComponent* BoxCollider)
{
	FVector BoxLocation = BoxCollider->GetComponentLocation();
	FVector BoxExtent = BoxCollider->GetScaledBoxExtent();

	FVector NodeLocation = Node->GetActorLocation();
	
	//Check if the node is within the box collider
	//Check X value
	if (NodeLocation.X < BoxLocation.X - BoxExtent.X || NodeLocation.X > BoxLocation.X + BoxExtent.X)
	{
		return false;
	}

	//Check Y value
	if (NodeLocation.Y < BoxLocation.Y - BoxExtent.Y || NodeLocation.Y > BoxLocation.Y + BoxExtent.Y)
	{
		return false;
	}

	// If there's no gap along any axis, return true (point inside box)
	return true;
}

void ATestAILevelSetup::CheckAgainstNavNodes(UBoxComponent* BoxCollider, AStaticMeshActor* Room)
{
	//Box Collider Location
	UE_LOG(LogTemp, Warning, TEXT("Box Collider Location: %s"), *BoxCollider->GetComponentLocation().ToString());
	for (ANavigationNode* RoomNode : RoomNodes)
	{
		if (RoomNode)
		{
			// Check if this location is within the box component's bounds
			if (IsNodeInsideBox(RoomNode, BoxCollider)) 
			{
				UE_LOG(LogTemp, Warning, TEXT("Node is overlapping with box collider"));
				RoomNode->SetRoom(Room);
				RoomNode->SetRoomName(Room->GetName());
			}
		}
	}
}

void ATestAILevelSetup::AssignNavNodes()
{
	

	// Get the components of the actor

	for (AStaticMeshActor* Room : Rooms)
	{
		TArray<UActorComponent*> ActorComponents;
		Room->GetComponents(ActorComponents);

		for (UActorComponent* Component : ActorComponents)
		{
			UBoxComponent* BoxCollider = Cast<UBoxComponent>(Component);
			if (BoxCollider)
			{
				UE_LOG(LogTemp, Warning, TEXT("Box Collider Found"));
				CheckAgainstNavNodes(BoxCollider, Room);
				
				break; // Break if you're only interested in one box collider per actor
			}
		}
	}
}

// Called when the game starts or when spawned
void ATestAILevelSetup::BeginPlay()
{
	Super::BeginPlay();
	
}

void ATestAILevelSetup::OnPickupOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
}

void ATestAILevelSetup::GetRooms()
{
	UE_LOG(LogTemp, Warning, TEXT("Getting Rooms"));
	for (TActorIterator<AStaticMeshActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
	{
		AStaticMeshActor* MeshActor = *ActorItr;
		Rooms.Add(MeshActor);
		UE_LOG(LogTemp, Warning, TEXT("Room Added"));
		// Proceed to check for the box collider
	}
}

void ATestAILevelSetup::ClearData()
{
	Rooms.Empty();
}

// Called every frame
void ATestAILevelSetup::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bUpdateNavNodes)
	{
		ClearData();
		GetRooms();
		AssignNavNodes();
		bUpdateNavNodes = false;
	}
}

