﻿// Fill out your copyright notice in the Description page of Project Settings.


#include "ProceduralCaveGen.h"

#include "ContentStreaming.h"
#include "EngineUtils.h"
#include "Components/BoxComponent.h"
#include "Builders/CubeBuilder.h"
#include "Kismet/GameplayStatics.h"
#include "Voxels/MarchingChunkTerrain.h"


// Sets default values
AProceduralCaveGen::AProceduralCaveGen()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

bool AProceduralCaveGen::ShouldTickIfViewportsOnly() const
{
	return true;
}

// Called when the game starts or when spawned
void AProceduralCaveGen::BeginPlay()
{
	Super::BeginPlay();
	//Boxes = GenerateGuaranteedPathBoxes(NumBoxes, MinSize, MaxSize);
	//Tunnels = GenerateTunnels(Boxes);
}

//ToDo: Tidy up second path by generalizing the function
TArray<FLevelBox> AProceduralCaveGen::GenerateGuaranteedPathBoxes(int NumBoxesToGenerate, FVector BoxMinSize,
                                                                  FVector BoxMaxSize)
{
	FVector Start = FVector(GetActorLocation());
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

		box.Position = newPosition;

		while (!PositionValidForSecondPath(box, Path1))
		{
			RandomDirection = FMath::VRandCone(BiasedDirection, FMath::DegreesToRadians(80.f));
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

void AProceduralCaveGen::CreateTunnel(const FLevelBox& StartBox, const FLevelBox& EndBox)
{
	FTunnel Tunnel;
	Tunnel.StartBox = &StartBox;
	Tunnel.EndBox = &EndBox;

	FVector Start = StartBox.Position;
	FVector End = EndBox.Position;
	Tunnel.Position = (Start + End) / 2;
	FVector Direction = (End - Start).GetSafeNormal();
	Tunnel.Size = FVector(200, 200, (End - Start).Size());
	Tunnel.Rotation = FQuat::FindBetweenNormals(FVector::UpVector, Direction);

	Tunnels.Add(Tunnel);
}

bool AProceduralCaveGen::BoxesIntersect(const FLevelBox& BoxA, const FLevelBox& BoxB)
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

void AProceduralCaveGen::GenerateMesh()
{
	//For loop for X/Y/Z chunks within levelSize

	if (bDebugOnly1Chunk)
	{
		for (int i = 0 ; i < 1; i++)
		{
			FVector ChunkPosition = FVector( ChunkSize/2, i*ChunkSize + ChunkSize/2, ChunkSize/2);
			FLevelBox ChunkBox{ChunkPosition, FVector(ChunkSize*2, ChunkSize*2, ChunkSize*2), EBoxType::Normal};
			for (FLevelBox box : Boxes)
			{
				if (BoxesIntersect(ChunkBox, box))
				{
					const auto Chunk = GetWorld()->SpawnActorDeferred<AMarchingChunkTerrain>
					(
						AMarchingChunkTerrain::StaticClass(),
						FTransform(), //Replace this with a transform that places the chunk in the correct location
						this
					);
					Chunk->ChunkPosition = ChunkPosition;
					Chunk->ChunkSize = ChunkSize;
					Chunk->VoxelsPerSide = VoxelDensity;
					Chunk->Boxes = Boxes;
					Chunk->Tunnels = Tunnels;
					Chunk->Material = Material;
					Chunk->LevelSize = static_cast<int>(LevelSize);
					Chunk->bUpdateMesh = bUpdateMesh;
					Chunk->DebugChunk = DebugChunk;
					Chunk->DebugVoxels = DebugVoxels;
						
					UGameplayStatics::FinishSpawningActor(Chunk, FTransform());

					Chunks.Add(Chunk);
					break;
				}
			}
		}
		
	}
	else
	{
		//ToDo:  Optimize by checking if chunks intersect with any boxes or tunnels
		for (int x = -1 ; x <= LevelSize/ChunkSize ; x++)
		{
			for (int y = -1 ; y <= LevelSize/ChunkSize ; y++)
			{
				for (int z = -3*HeightDifference/ChunkSize ; z <= 3 * HeightDifference/ChunkSize ; z++)
				{
					FVector ChunkPosition = FVector(x * ChunkSize + ChunkSize/2, y * ChunkSize + ChunkSize/2, z * ChunkSize + ChunkSize/2);
					//Check if chunk intersects with any boxes - if not, skip it
					//ChunkSize is doubled to be safe
					FLevelBox ChunkBox{ChunkPosition, FVector(ChunkSize*2, ChunkSize*2, ChunkSize*2), EBoxType::Normal};
					for (FLevelBox box : Boxes)
					{
						if (BoxesIntersect(ChunkBox, box))
						{
							const auto Chunk = GetWorld()->SpawnActorDeferred<AMarchingChunkTerrain>
							(
								AMarchingChunkTerrain::StaticClass(),
								FTransform(), //Replace this with a transform that places the chunk in the correct location
								this
							);
							Chunk->ChunkPosition = ChunkPosition;
							Chunk->ChunkSize = ChunkSize;
							Chunk->VoxelsPerSide = VoxelDensity;
							Chunk->Boxes = Boxes;
							Chunk->Tunnels = Tunnels;
							Chunk->Material = Material;
							Chunk->LevelSize = static_cast<int>(LevelSize);
							Chunk->bUpdateMesh = bUpdateMesh;
							Chunk->DebugChunk = DebugChunk;
							Chunk->DebugVoxels = DebugVoxels;
						
							UGameplayStatics::FinishSpawningActor(Chunk, FTransform());

							Chunks.Add(Chunk);
							break;
						}
					}
				}
			}
		}
	}
	

}

bool AProceduralCaveGen::PositionValidForSecondPath(const FLevelBox& NewBox, const TArray<FLevelBox>& FirstPathBoxes)
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
void AProceduralCaveGen::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (bShouldRegenerate)
	{
		Boxes.Empty();
		Tunnels.Empty();
		Path1.Empty();
		Path2.Empty();
		//If the marched terrain chunk has been spawned, destroy it
		if (!Chunks.IsEmpty())
		{
			for (AMarchingChunkTerrain* Chunk : Chunks)
			{
				if (Chunk)
				{
					Chunk->ClearMesh();
					Chunk->Destroy();
				}
			}
			Chunks.Empty();
		}

		Boxes = GenerateGuaranteedPathBoxes(NumBoxesPerPath, MinSize, MaxSize);

		GenerateMesh();
		//SpawnPickups(); //Now implemented in PickupManagerSubsystem
		bShouldRegenerate = false;
	}

	//Debug to visualize boxes
	if (!Boxes.IsEmpty() && bDebugView)
	{
		for (const FLevelBox& box : Boxes)
		{
			// Box center
			FVector center = box.Position;

			// Half of the box size in each dimension
			FVector halfSize = box.Size * 0.5;

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
				DrawDebugBox(GetWorld(), tunnel.Position, tunnel.Size / 2, tunnel.Rotation, FColor::Green, false, -1.0,
				             0, 10.0f);
			}
		}
	}
}
