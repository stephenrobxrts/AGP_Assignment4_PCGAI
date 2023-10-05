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
	
	if (bUpdateMesh)
	{

		//Log Boxes (Position, Size)
		UE_LOG(LogTemp, Warning, TEXT("Box Count: %d"), Boxes.Num());
		for (FLevelBox Box : Boxes)
		{
			UE_LOG(LogTemp, Warning, TEXT("Box Position: %s"), *Box.Position.ToString());
		}
		UE_LOG(LogTemp, Warning, TEXT("Box Count: %d"), Tunnels.Num());
		CreateVoxels();
		GenerateHeightMap();
		bUpdateMesh = false;
	}
	if (DebugChunk)
	{
		float ChunkRatio = ChunkVoxelDensity / LevelSize;
		FVector halfSize = FVector(ChunkSize/2.0, ChunkSize/2.0, ChunkSize/2.0);
		DrawDebugBox(GetWorld(), CornerPosition, halfSize, FColor::Silver, false, -1, 0, 2.0f);
	}
	if (DebugVoxels && !Voxels.IsEmpty())
	{
		FVector voxelPosition = FVector(0.0f, 0.0f, 100.0f);
		FColor color = FColor::White;
		// Voxel count, e.g. 64Wide, 64Long, 64High
		for (int x = 0; x <= ChunkVoxelDensity; ++x)
		{
			for (int y = 0; y <= ChunkVoxelDensity; ++y)
			{
				for (int z = 0; z <= ChunkVoxelDensity; ++z)
				{
					Voxels[GetVoxelIndex(x, y, z)] == 0 ? color = FColor::Black : color = FColor::White;
					int index = GetVoxelIndex(x, y, z);
					float voxelValue = Voxels[index];
					
					voxelPosition = FVector(ChunkSize * x/ChunkVoxelDensity ,ChunkSize *  y/ChunkVoxelDensity, ChunkSize * z/ChunkVoxelDensity - ChunkSize/2);
					
					/*FColor color = FColor()*/
					DrawDebugSphere(GetWorld(), voxelPosition, 12.0f, 4, color, false, -1, 0, 2.0f);	
				}
			}
		}
	}

}

void AMarchingChunkTerrain::March(int X, int Y, int Z, const float Cube[8])
{
}

int AMarchingChunkTerrain::GetVoxelIndex(int X, int Y, int Z) const
{
	return Z * (ChunkVoxelDensity + 1) * (ChunkVoxelDensity + 1) + Y * (ChunkVoxelDensity + 1) + X;
}

void AMarchingChunkTerrain::ApplyMesh() const
{
}

void AMarchingChunkTerrain::ClearMesh()
{
}

bool IsPointInsideBox(const FVector& point, const FVector BoxPosition, FVector BoxSize, FQuat BoxRotation) {
	FVector halfSize = BoxSize * 0.5f;
	FQuat InvRotation = BoxRotation.Inverse();
	FVector LocalPoint = InvRotation.RotateVector(point - BoxPosition);
    
	// Notice here that we're checking against halfSize, not BoxPosition anymore.
	return LocalPoint.X >= -halfSize.X && LocalPoint.X <= halfSize.X &&
		   LocalPoint.Y >= -halfSize.Y && LocalPoint.Y <= halfSize.Y &&
		   LocalPoint.Z >= -halfSize.Z && LocalPoint.Z <= halfSize.Z;
}

void AMarchingChunkTerrain::GenerateHeightMap()
{
	FVector voxelPosition = FVector(0.0f, 0.0f, 100.0f);

	// Voxel count, 64Wide, 64Long, 64High
	for (int x = 0; x <= ChunkVoxelDensity; ++x)
	{
		for (int y = 0; y <= ChunkVoxelDensity; ++y)
		{
			for (int z = 0; z <= ChunkVoxelDensity; ++z)
			{
				voxelPosition = FVector(ChunkSize * x/ChunkVoxelDensity ,ChunkSize *  y/ChunkVoxelDensity, ChunkSize * z/ChunkVoxelDensity - ChunkSize/2);
				
				bool voxelInside = false;
				for (FLevelBox Box : Boxes) {
					if (IsPointInsideBox(voxelPosition, Box.Position, Box.Size, FQuat::Identity)) {
						UE_LOG(LogTemp, Warning, TEXT("Voxel inside box"));
						Voxels[GetVoxelIndex(x,y,z)] = 1.0f;
						voxelInside = true;
						break;  // Exit once found
					}
				}
				if(!voxelInside) {  // Only check tunnels if not already inside a box
					for (FTunnel Tunnel : Tunnels) {
						if (IsPointInsideBox(voxelPosition, Tunnel.Position, Tunnel.Size, Tunnel.Rotation )) {
							UE_LOG(LogTemp, Warning, TEXT("Voxel inside tunnel"));
							Voxels[GetVoxelIndex(x,y,z)] = 1.0f;
							break;  // Exit once found
						}
					}
				}

				//Voxels[GetVoxelIndex(x,y,z)] = Noise->GetNoise(x + Position.X, y + Position.Y, z + Position.Z);	
			}
		}
	}
}

