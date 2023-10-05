// Fill out your copyright notice in the Description page of Project Settings.


#include "ProceduralBoxes.h"
#include "EngineUtils.h"
#include "Components/BoxComponent.h"
#include "Builders/CubeBuilder.h"


// Sets default values
AProceduralBoxes::AProceduralBoxes()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

bool AProceduralBoxes::ShouldTickIfViewportsOnly() const
{
	return true;
}

// Called when the game starts or when spawned
void AProceduralBoxes::BeginPlay()
{
	Super::BeginPlay();
	//Boxes = GenerateGuaranteedPathBoxes(NumBoxes, MinSize, MaxSize);
	//Tunnels = GenerateTunnels(Boxes);


}


int AProceduralBoxes::CountConnections(int BoxIndex, const TArray<TArray<bool>>& Connections)
{
	int Count = 0;
	for (bool bIsConnected : Connections[BoxIndex])
	{
		if (bIsConnected)
		{
			Count++;
		}
	}
	return Count;
}

TArray<FLevelBox> AProceduralBoxes::GenerateGuaranteedPathBoxes(int NumBoxesToGenerate, FVector BoxMinSize,
                                                                FVector BoxMaxSize)
{
	FVector Start = FVector(0, 0, 0);
	FVector End = FVector(LevelSize, LevelSize, FMath::RandRange(-HeightDifference, HeightDifference));
	TArray<FLevelBox> boxes;

	FLevelBox StartBox;
	StartBox.Position = Start;
	StartBox.Size = FVector(FMath::RandRange(BoxMinSize.X, BoxMaxSize.X), FMath::RandRange(BoxMinSize.Y, BoxMaxSize.Y),
	                        FMath::RandRange(BoxMinSize.Z, BoxMaxSize.Z));
	StartBox.Type = EBoxType::Start;
	boxes.Add(StartBox);

	FVector LastPosition = Start;

	for (int i = 1; i < NumBoxesToGenerate - 1; i++)
	{
		FLevelBox box;

		// Get a direction biased towards the destination
		FVector BiasedDirection = (End - LastPosition).GetSafeNormal();

		// Randomize within a hemisphere towards the destination
		FVector RandomDirection = FMath::VRandCone(BiasedDirection, FMath::DegreesToRadians(30.f));

		FVector newPosition = LastPosition + RandomDirection * FMath::RandRange(
			0.5f * MaxConnectionDistance, MaxConnectionDistance);
		box.Position = newPosition;
		box.Size = FVector(FMath::RandRange(BoxMinSize.X, BoxMaxSize.X), FMath::RandRange(BoxMinSize.Y, BoxMaxSize.Y),
		                   FMath::RandRange(BoxMinSize.Z, BoxMaxSize.Z));
		box.Type = EBoxType::Normal;
		boxes.Add(box);
		Path1.Add(box);
		CreateTunnel(boxes[i - 1], box);
		LastPosition = newPosition;
	}

	FLevelBox& EndBox = boxes.Last();
	EndBox.Size = FVector
	(
		FMath::RandRange(BoxMinSize.X, BoxMaxSize.X),
		FMath::RandRange(BoxMinSize.Y, BoxMaxSize.Y),
		FMath::RandRange(BoxMinSize.Z, BoxMaxSize.Z)
	);
	EndBox.Type = EBoxType::End;

	LastPosition = Start;
	//Generate Second Path
	for (int i = 1; i < NumBoxesToGenerate - 1; i++)
	{
		FLevelBox box;

		// Get a direction biased towards the destination
		FVector BiasedDirection = (EndBox.Position - LastPosition).GetSafeNormal();
		
		// Randomize within a hemisphere towards the destination
		FVector RandomDirection = FMath::VRandCone(BiasedDirection, FMath::DegreesToRadians(30.f));
		FVector newPosition = LastPosition + RandomDirection * FMath::RandRange(
			0.5f * MaxConnectionDistance, MaxConnectionDistance);
		box.Size = FVector
		(
			FMath::RandRange(BoxMinSize.X, BoxMaxSize.X),
			FMath::RandRange(BoxMinSize.Y, BoxMaxSize.Y),
			FMath::RandRange(BoxMinSize.Z, BoxMaxSize.Z)
		);

		box.Position = newPosition; // <-- Move this here

		while (!PositionValidForSecondPath(box, Path1))
		{
			RandomDirection = FMath::VRandCone(BiasedDirection, FMath::DegreesToRadians(30.f));
			newPosition = LastPosition + RandomDirection * FMath::RandRange(
				0.5f * MaxConnectionDistance, MaxConnectionDistance);
			box.Position = newPosition; // <-- This is necessary to update the box's position
		}

		box.Type = EBoxType::Normal;
		boxes.Add(box);
		Path2.Add(box);
		FLevelBox LastBox;
		if (i == 1)
		{
			LastBox = StartBox;
		}
		else
		{
			LastBox = boxes[Path1.Num() + i - 1];
		}
		CreateTunnel(LastBox, box);
		LastPosition = newPosition;
	}
	return boxes;
}

TArray<FTunnel> AProceduralBoxes::GenerateTunnels(TArray<FLevelBox> InputBoxes)
{
	TArray<FTunnel> TunnelArray;
	for (int i = 0; i < InputBoxes.Num(); i++)
	{
		TArray<bool> Connections;
		for (int j = 0; j < InputBoxes.Num(); j++)
		{
			Connections.Add(false);
		}
		ConnectionMatrix.Add(Connections);
	}

	// For each box, find its nearest neighbor and create a tunnel to it.
	for (int i = 0; i < InputBoxes.Num(); i++)
	{
		if (CountConnections(i, ConnectionMatrix) < 2)
		{
			TArray<int> PotentialConnections;
			for (int j = 0; j < InputBoxes.Num(); j++)
			{
				if (i != j && !ConnectionMatrix[i][j] && CountConnections(j, ConnectionMatrix) < 2)
				{
					PotentialConnections.Add(j);
				}
			}

			while (PotentialConnections.Num() > 0 && CountConnections(i, ConnectionMatrix) < 2)
			{
				int RandomIndex = FMath::RandRange(0, PotentialConnections.Num() - 1);
				int TargetBoxIndex = PotentialConnections[RandomIndex];

				// Mark these two boxes as connected
				ConnectionMatrix[i][TargetBoxIndex] = true;
				ConnectionMatrix[TargetBoxIndex][i] = true;

				// Create tunnel between these boxes
				CreateTunnel(InputBoxes[i], InputBoxes[TargetBoxIndex]);

				// Remove the chosen box from potential connections
				PotentialConnections.RemoveAt(RandomIndex);
			}
		}
	}

	return TunnelArray;
}

void AProceduralBoxes::CreateTunnel(const FLevelBox& StartBox, const FLevelBox& EndBox)
{
	FTunnel Tunnel;
	Tunnel.StartBox = &StartBox;
	Tunnel.EndBox = &EndBox;
	
	FVector Start = StartBox.Position;
	FVector End = EndBox.Position;
	Tunnel.Position = (Start + End) / 2;
	FVector Direction = (End - Start).GetSafeNormal();
	Tunnel.Size = FVector(50, 50, (End - Start).Size());
	Tunnel.Rotation = FQuat::FindBetweenNormals(FVector::UpVector, Direction);

	Tunnels.Add(Tunnel);
}


void AProceduralBoxes::PopulateConnectedNodes(TArray<FBoxNode>& AllNodes)
{
	for (FBoxNode& NodeA : AllNodes)
	{
		for (FBoxNode& NodeB : AllNodes)
		{
			if (&NodeA != &NodeB && CanConnect(NodeA.Box, NodeB.Box))
			{
				NodeA.ConnectedNodes.Add(&NodeB);
			}
		}
	}
}

bool AProceduralBoxes::CanConnect(FLevelBox* BoxA, FLevelBox* BoxB)
{
	FVector CenterA = BoxA->Position;
	FVector CenterB = BoxB->Position;

	float Distance = FVector::Dist(CenterA, CenterB);

	return Distance <= MaxConnectionDistance;
}

bool AProceduralBoxes::BoxesIntersect(const FLevelBox& BoxA, const FLevelBox& BoxB)
{
	// Check for gap along X axis
	if (BoxA.Position.X + BoxA.Size.X < BoxB.Position.X ||
		BoxB.Position.X + BoxB.Size.X < BoxA.Position.X)
	{
		return false;
	}

	// Check for gap along Y axis
	if (BoxA.Position.Y + BoxA.Size.Y < BoxB.Position.Y ||
		BoxB.Position.Y + BoxB.Size.Y < BoxA.Position.Y)
	{
		return false;
	}

	// Check for gap along Z axis
	if (BoxA.Position.Z + BoxA.Size.Z < BoxB.Position.Z ||
		BoxB.Position.Z + BoxB.Size.Z < BoxA.Position.Z)
	{
		return false;
	}

	// If there's no gap along any axis, then the boxes intersect
	return true;
}

bool AProceduralBoxes::PositionValidForSecondPath(const FLevelBox& NewBox, const TArray<FLevelBox>& FirstPathBoxes)
{
	for (const FLevelBox& Box : FirstPathBoxes)
	{
		if (BoxesIntersect(Box, NewBox))
		{
			return false;
		}
	}
	return true;
}

// Called every frame
void AProceduralBoxes::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (bShouldRegenerate)
	{
		Boxes.Empty();
		Tunnels.Empty();
		Path1.Empty();
		Path2.Empty();
		
		Boxes = GenerateGuaranteedPathBoxes(NumBoxes, MinSize, MaxSize);
		//SpawnPickups(); //Now implemented in PickupManagerSubsystem
		bShouldRegenerate = false;

	}

	//Debug to visualize boxes
	if (!Boxes.IsEmpty())
	{
		for (const FLevelBox& box : Boxes)
		{
			// Box center
			FVector center = box.Position;

			// Half of the box size in each dimension
			FVector halfSize = box.Size * 0.5f;

			FColor color = FColor::Red;
			switch (box.Type)
			{
			case EBoxType::Start:
				color = FColor::Blue;
				break;
			case EBoxType::End:
				color = FColor::Yellow;
				break;
			default:
				color = FColor::Red;
				break;
			}
			// Draw the box every frame
			DrawDebugBox(GetWorld(), center, halfSize, color, false, -1, 0, 2.0f);
		}
		if (!Tunnels.IsEmpty())
		{
			for (const FTunnel& tunnel : Tunnels)
			{
				FVector center = tunnel.Position;
				FVector halfSize = tunnel.Size * 0.5f;
				FColor color = FColor::Green;

				DrawDebugBox(GetWorld(), tunnel.Position, tunnel.Size / 2, tunnel.Rotation, FColor::Green, false, -1.0, 0, 10.0f);
			}
		}

	}
}
