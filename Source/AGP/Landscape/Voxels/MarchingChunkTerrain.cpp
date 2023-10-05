// Fill out your copyright notice in the Description page of Project Settings.


#include "MarchingChunkTerrain.h"



// Sets default values
AMarchingChunkTerrain::AMarchingChunkTerrain()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

bool AMarchingChunkTerrain::ShouldTickIfViewportsOnly() const
{
	return true;
}

// Called when the game starts or when spawned
void AMarchingChunkTerrain::BeginPlay()
{
	Super::BeginPlay();
	
}

void AMarchingChunkTerrain::CreateVoxels()
{
	Voxels.SetNum((ChunkVoxelDensity + 1) * (ChunkVoxelDensity + 1) * (ChunkVoxelDensity + 1));
}

// Called every frame
void AMarchingChunkTerrain::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	float ChunkRatio = ChunkVoxelDensity / LevelSize;
	FVector halfSize = FVector(ChunkSize/2.0, ChunkSize/2.0, ChunkSize/2.0);
	FVector CornerPosition = FVector (Position.X + ChunkSize/2.0,
									  Position.Y + ChunkSize/2.0,
									  Position.Z);
	DrawDebugBox(GetWorld(), CornerPosition, halfSize, FColor::Silver, false, -1, 0, 2.0f);
}

void AMarchingChunkTerrain::March(int X, int Y, int Z, const float Cube[8])
{
}

int AMarchingChunkTerrain::GetVoxelIndex(int X, int Y, int Z) const
{
	return 0;
}

void AMarchingChunkTerrain::ApplyMesh() const
{
}

void AMarchingChunkTerrain::ClearMesh()
{
}

bool IsPointInsideBox(const FVector& point, const FLevelBox& box) {
	FVector halfSize = box.Size * 0.5f;
	return point.X >= (box.Position.X - halfSize.X) && point.X <= (box.Position.X + halfSize.X) &&
		   point.Y >= (box.Position.Y - halfSize.Y) && point.Y <= (box.Position.Y + halfSize.Y) &&
		   point.Z >= (box.Position.Z - halfSize.Z) && point.Z <= (box.Position.Z + halfSize.Z);
}

void AMarchingChunkTerrain::GenerateHeightMap()
{
	FVector voxelPosition = FVector(0.0f, 0.0f, 100.0f);
	FLevelBox Box;
	Box.Position = FVector(0.0f, 0.0f, 0.0f);
	Box.Size = FVector(10.0f, 10.0f, 10.0f);

	// Voxel count, 64Wide, 64Long, 64High
	for (int x = 0; x <= ChunkVoxelDensity; ++x)
	{
		for (int y = 0; y <= ChunkVoxelDensity; ++y)
		{
			for (int z = 0; z <= ChunkVoxelDensity; ++z)
			{
				voxelPosition = FVector(x, y, z);

				if (IsPointInsideBox(voxelPosition + Position, Box))
				{
					Voxels[GetVoxelIndex(x,y,z)] = 1.0f;
				}
				//Voxels[GetVoxelIndex(x,y,z)] = Noise->GetNoise(x + Position.X, y + Position.Y, z + Position.Z);	
			}
		}
	}
}

