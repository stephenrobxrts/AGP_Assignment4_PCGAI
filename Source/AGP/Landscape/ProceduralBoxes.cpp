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
	Boxes = GenerateRandomBoxes(NumBoxes, MinSize, MaxSize);
	Tunnels = GenerateTunnels(Boxes);

	for (const FLevelBox& box : Boxes)
	{
		// Box center
		FVector center = box.Position;

		// Half of the box size in each dimension
		FVector halfSize = box.Size * 0.5f;

		// Draw the box for 10 seconds with a thickness of 2 units
		DrawDebugBox(GetWorld(), center, halfSize, FColor::Red, false, 10.0f, 0, 2.0f);
	}
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

TArray<FLevelBox> AProceduralBoxes::GenerateRandomBoxes(int NumBoxesToGenerate, FVector BoxMinSize, FVector BoxMaxSize)
{
	FVector Start = FVector(0, 0, 0);
	FVector End = FVector(LevelSize, LevelSize, FMath::RandRange(-HeightDifference, HeightDifference));
	TArray<FLevelBox> boxes;
	for (int i = 0; i < NumBoxesToGenerate; i++)
	{
		FLevelBox box;
		box.Position.SetComponentForAxis(EAxis::X, FMath::RandRange(0, 10000));
		box.Position.SetComponentForAxis(EAxis::Y, FMath::RandRange(0, 10000));
		box.Size = FVector(
			FMath::RandRange(BoxMinSize.X, BoxMaxSize.X),
			FMath::RandRange(BoxMinSize.Z, BoxMaxSize.Z),
			FMath::RandRange(BoxMinSize.Y, BoxMaxSize.Y)
		);

		// Check for intersections with other boxes
		bool hasIntersection = false;
		for (const FLevelBox& existingBox : boxes)
		{
			if (AreBoxesIntersecting(box, existingBox))
			{
				hasIntersection = true;
				break;
			}
		}

		if (!hasIntersection)
		{
			//Log the box
			UE_LOG(LogTemp, Warning, TEXT("Box %d: Position: %s, Size: %s"), i, *box.Position.ToString(),
			       *box.Size.ToString());
			boxes.Add(box);
		}
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
	FVector Start = StartBox.Position;
	FVector End = EndBox.Position;
	FVector MidPoint = (Start + End) / 2;
	FVector Direction = (End - Start).GetSafeNormal();
	FVector TunnelDimensions(50, 50, (End - Start).Size());
	FQuat TunnelRotation = FQuat::FindBetweenNormals(FVector::UpVector, Direction);

	DrawDebugBox(GetWorld(), MidPoint, TunnelDimensions / 2, TunnelRotation, FColor::Green, false, 10.0f, 0, 2.0f);
}

void AProceduralBoxes::PopulateConnectedNodes(TArray<FBoxNode>& AllNodes, float MaxConnectionDistance)
{
	for(FBoxNode& NodeA : AllNodes)
	{
		for(FBoxNode& NodeB : AllNodes)
		{
			if(&NodeA != &NodeB && CanConnect(NodeA.Box, NodeB.Box, MaxConnectionDistance))
			{
				NodeA.ConnectedNodes.Add(&NodeB);
			}
		}
	}
}

bool AProceduralBoxes::CanConnect(FLevelBox* BoxA, FLevelBox* BoxB, float MaxConnectionDistance)
{
	FVector CenterA = BoxA->Position;
	FVector CenterB = BoxB->Position;

	float Distance = FVector::Dist(CenterA, CenterB);

	return Distance <= MaxConnectionDistance;
}

bool AProceduralBoxes::AreBoxesIntersecting(const FLevelBox& BoxA, const FLevelBox& BoxB)
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

// Called every frame
void AProceduralBoxes::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}
