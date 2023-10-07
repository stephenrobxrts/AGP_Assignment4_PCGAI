// Fill out your copyright notice in the Description page of Project Settings.


#include "MarchingChunkTerrain.h"

#include "GenericPlatform/GenericPlatformChunkInstall.h"


// Sets default values
AMarchingChunkTerrain::AMarchingChunkTerrain()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	Mesh = CreateDefaultSubobject<UProceduralMeshComponent>("Mesh");

	// Mesh Settings
	Mesh->SetCastShadow(true);

	// Set Mesh as root
	SetRootComponent(Mesh);
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
	Voxels.SetNum((VoxelsPerSide + 1) * (VoxelsPerSide + 1) * (VoxelsPerSide + 1));
}

// Called every frame
void AMarchingChunkTerrain::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bUpdateMesh)
	{
		ChunkRatio = ChunkSize/VoxelsPerSide;
		CreateVoxels();
		GenerateHeightMap();

		GenerateMesh();
		ApplyMesh();
		SetActorLocation(ChunkPosition);
		bUpdateMesh = false;
	}
	if (DebugChunk)
	{
		FVector halfSize = FVector(ChunkSize / 2.0, ChunkSize / 2.0, ChunkSize / 2.0);
		DrawDebugBox(GetWorld(), GetActorLocation() + CornerPosition, halfSize, FColor::Silver, false, -1, 0, 2.0f);
	}
	if (DebugVoxels && !Voxels.IsEmpty())
	{
		FVector voxelPosition = FVector(0.0f, 0.0f, 100.0f);
		FColor color = FColor::White;
		// Voxel count, e.g. 64Wide, 64Long, 64High
		for (double x = 0; x <= VoxelsPerSide; ++x)
		{
			for (double y = 0; y <= VoxelsPerSide; ++y)
			{
				for (double z = 0; z <= VoxelsPerSide; ++z)
				{
					if (Voxels[GetVoxelIndex(x, y, z)] == 1 || Voxels[GetVoxelIndex(x, y, z)] == 0)
					{
						voxelPosition = FVector(x*ChunkRatio, y*ChunkRatio, z*ChunkRatio) + ChunkPosition;
						DrawDebugSphere(GetWorld(), voxelPosition, 12.0f, 4, FColor::White, false, -1, 0, 2.0f);
					}
				}
			}
		}
	}
}

void AMarchingChunkTerrain::ClearMesh()
{
	Mesh->ClearAllMeshSections();
	VertexCount = 0;
	MeshData.Clear();
}



bool IsPointInsideBox(const FVector& point, const FVector BoxPosition, FVector BoxSize, FQuat BoxRotation)
{
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
	for (int x = 0; x <= VoxelsPerSide ; ++x)
	{
		for (int y = 0; y <= VoxelsPerSide ; ++y)
		{
			for (int z = 0; z <= VoxelsPerSide ; ++z)
			{
				voxelPosition = FVector(ChunkRatio*x + ChunkPosition.X, ChunkRatio*y + ChunkPosition.Y,
				                        ChunkRatio*z + ChunkPosition.Z);

				bool voxelInside = false;
				for (FLevelBox Box : Boxes)
				{
					if (IsPointInsideBox(voxelPosition, Box.Position, Box.Size, FQuat::Identity))
					{
						Voxels[GetVoxelIndex(x, y, z)] = 1.0f;
						voxelInside = true;
						break; // Exit once found
					}
				}
				if (!voxelInside)
				{
					// Only check tunnels if not already inside a box
					for (FTunnel Tunnel : Tunnels)
					{
						if (IsPointInsideBox(voxelPosition, Tunnel.Position, Tunnel.Size, Tunnel.Rotation))
						{
							Voxels[GetVoxelIndex(x, y, z)] = 1.0f;
							break; 
						}
					}
				}

				//Voxels[GetVoxelIndex(x,y,z)] = Noise->GetNoise(x + Position.X, y + Position.Y, z + Position.Z);	
			}
		}
	}
}

void AMarchingChunkTerrain::GenerateMesh()
{
	// Triangulation order
	if (SurfaceLevel > 0.0f)
	{
		TriangleOrder[0] = 0;
		TriangleOrder[1] = 1;
		TriangleOrder[2] = 2;
	}
	else
	{
		TriangleOrder[0] = 2;
		TriangleOrder[1] = 1;
		TriangleOrder[2] = 0;
	}

	float Cube[8];


	//If inside tunnel/box, set to 1, else 0
	//ToDo: use this value to set the surface level variable differently inside vs outside caves!
	//Voxel will be RGB -> R = SurfaceLevel, G = NoiseVal, B = extra (for now);
	for (int X = 0; X < VoxelsPerSide; ++X)
	{
		for (int Y = 0; Y < VoxelsPerSide; ++Y)
		{
			for (int Z = 0; Z < VoxelsPerSide ; ++Z)
			{
				for (int i = 0; i < 8; ++i)
				{
					//For each X,Y,Z voxel position (within the chunk) get the value of the voxel
					//8 corners correspond to the outward facing 8 vertices of the cube
					//This seems to correctly assign the cube values to the voxel values
					Cube[i] = Voxels[GetVoxelIndex(X + VertexOffset[i][0], Y + VertexOffset[i][1],
					                               Z + VertexOffset[i][2])];
				}
				March(X, Y, Z, Cube);
			}
		}
	}
}

//ToDo: Cube values are correct but X/Y/Z need scaling applied
//Note: Not the same scaling as voxel positions??? Not sure why
void AMarchingChunkTerrain::March(int X, int Y, int Z, const float Cube[8])
{
	int VertexMask = 0;
	FVector EdgeVertex[12];

	//Find which vertices are inside of the surface and which are outside
	for (int i = 0; i < 8; ++i)
	{
		if (Cube[i] <= SurfaceLevel)
		{
			VertexMask |= 1 << i;
		}
	}

	const int EdgeMask = CubeEdgeFlags[VertexMask];

	if (EdgeMask == 0)
	{
		return;
	}

	// Find intersection points
	for (int i = 0; i < 12; ++i)
	{
		if ((EdgeMask & 1 << i) != 0)
		{
			const float Offset = GetInterpolationOffset(Cube[EdgeConnection[i][0]], Cube[EdgeConnection[i][1]]);

			EdgeVertex[i].X = X + (VertexOffset[EdgeConnection[i][0]][0] + Offset * EdgeDirection[i][0]);
			EdgeVertex[i].Y = Y + (VertexOffset[EdgeConnection[i][0]][1] + Offset * EdgeDirection[i][1]);
			EdgeVertex[i].Z = Z + (VertexOffset[EdgeConnection[i][0]][2] + Offset * EdgeDirection[i][2]);
		}
	}

	// Save triangles, at most can be 5
	for (int i = 0; i < 5; ++i)
	{
		if (TriangleConnectionTable[VertexMask][3 * i] < 0)
		{
			break;
		}

		auto V1 = EdgeVertex[TriangleConnectionTable[VertexMask][3 * i + 0]] * ChunkRatio;
		auto V2 = EdgeVertex[TriangleConnectionTable[VertexMask][3 * i + 1]] * ChunkRatio;
		auto V3 = EdgeVertex[TriangleConnectionTable[VertexMask][3 * i + 2]] * ChunkRatio;

		auto Normal = FVector::CrossProduct(V2 - V1, V3 - V1);
		auto Color = FColor::MakeRandomColor();

		Normal.Normalize();

		MeshData.Vertices.Append({V1, V2, V3});

		MeshData.Triangles.Append({
			VertexCount + TriangleOrder[0],
			VertexCount + TriangleOrder[1],
			VertexCount + TriangleOrder[2]
		});

		MeshData.Normals.Append({
			Normal,
			Normal,
			Normal
		});

		MeshData.Colors.Append({
			Color,
			Color,
			Color
		});

		VertexCount += 3;
	}
}

void AMarchingChunkTerrain::ApplyMesh() const
{
	Mesh->SetMaterial(0, Material);
	Mesh->CreateMeshSection(
		0,
		MeshData.Vertices,
		MeshData.Triangles,
		MeshData.Normals,
		MeshData.UV0,
		MeshData.Colors,
		TArray<FProcMeshTangent>(),
		true
	);
}

float AMarchingChunkTerrain::GetInterpolationOffset(float V1, float V2) const
{
	const float Delta = V2 - V1;
	return Delta == 0.0f ? SurfaceLevel : (SurfaceLevel - V1) / Delta;
}

int AMarchingChunkTerrain::GetVoxelIndex(int X, int Y, int Z) const
{
	return Z * (VoxelsPerSide + 1) * (VoxelsPerSide + 1) + Y * (VoxelsPerSide + 1) + X;
}

