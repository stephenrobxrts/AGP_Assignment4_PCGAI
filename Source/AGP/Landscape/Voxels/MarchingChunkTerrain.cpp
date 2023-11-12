// Fill out your copyright notice in the Description page of Project Settings.

#include "MarchingChunkTerrain.h"

#include "FrameTypes.h"
#include "./VoxelUtils/FastNoiseLite.h"
#include "Field/FieldSystemNoiseAlgo.h"


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
	for (float& Voxel : Voxels)
	{
		Voxel = 1.0f * InverseMultiplier;
	}
}

// Called every frame
void AMarchingChunkTerrain::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bUpdateMesh)
	{
		Noise = new FastNoiseLite();
		Noise->SetFrequency(0.005);
		Noise->SetNoiseType(static_cast<FastNoiseLite::NoiseType>(static_cast<int32>(NoiseParams->NoiseType)));
		Noise->SetFractalOctaves(NoiseParams->mOctaves);
		Noise->SetFractalLacunarity(NoiseParams->mLacunarity);
		Noise->SetFractalGain(NoiseParams->mGain);
		Noise->SetFractalType(static_cast<FastNoiseLite::FractalType>(static_cast<int32>(NoiseParams->FractalType)));
		Noise->SetFractalWeightedStrength(NoiseParams->mWeightedStrength);
		Noise->SetFractalPingPongStrength(NoiseParams->mPingPongStength);
		SetActorLocation(ChunkPosition);
		InverseMultiplier = bDebugInvertSolids ? -1.0f : 1.0f;
		SurfaceLevel = 0.0;
		VoxelDiameter = ChunkSize / VoxelsPerSide;
		CreateVoxels();
		GenerateHeightMap();

		GenerateMesh();
		ApplyMesh();

		bUpdateMesh = false;
	}
	ShowDebug();
}

void AMarchingChunkTerrain::ClearMesh()
{
	if (Mesh->GetNumSections() > 0)
	{
		Mesh->ClearAllMeshSections();
	}
	VertexCount = 0;
	MeshData.Clear();
}

/**
 * @brief UNUSED - Checks whether a point is inside a box - even if the box is rotated
 * @param point The point (Position) to check
 * @param BoxPosition Box Position
 * @param BoxSize Box Size
 * @param BoxRotation Box Rotation
 * @return True if the point is inside the box
 */
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
	CornerPosition = ChunkPosition + FVector(ChunkSize / 2.0f, ChunkSize / 2.0f, ChunkSize / 2.0f);
	FVector voxelPosition = FVector(0.0f, 0.0f, 100.0f);
	// For each voxel in the cube (+1 to overlap with next chunk)
	for (int x = 0; x <= VoxelsPerSide; ++x)
	{
		for (int y = 0; y <= VoxelsPerSide; ++y)
		{
			for (int z = 0; z <= VoxelsPerSide; ++z)
			{
				voxelPosition = FVector(VoxelDiameter * x + ChunkPosition.X,
				                        VoxelDiameter * y + ChunkPosition.Y,
				                        VoxelDiameter * z + ChunkPosition.Z);

				float VoxelSDF = UE_MAX_FLT;
				float TunnelSDF = UE_MAX_FLT;
				for (FLevelBox Box : Boxes)
				{
					float TempSDF = BoxSDF(voxelPosition, Box.Position, Box.Size, FQuat::Identity);
					if (TempSDF < VoxelSDF)
					{
						VoxelSDF = TempSDF;
					}
				}

				for (FTunnel Tunnel : Tunnels)
				{
					float TempSDF = BoxSDF(voxelPosition, Tunnel.Position, Tunnel.Size, Tunnel.Rotation);

					//If the value is more internal, take it
					//Stricter adherence to tunnels *3
					if (TempSDF < TunnelSDF)
					{
						TunnelSDF = TempSDF;
					}
				}

				//If the value is more internal, take it
				if (TunnelSDF < VoxelSDF)
				{
					VoxelSDF = TunnelSDF;
				}

				//ScaleSDF - The SDF is scale to approximately one human sized voxel - prevents getting stuck!
				float SDFScale = 180;
				VoxelSDF *= 1 / SDFScale;
				// Clamp SDF between -1, 1 to make it valid and interact with noise
				VoxelSDF = (VoxelSDF > 1.0f) ? 1.0f : VoxelSDF;
				VoxelSDF = (VoxelSDF < -1.0f) ? -1.0f : VoxelSDF;
				VoxelSDF *= InverseMultiplier;

				//Noise.SetNoiseType(FastNoiseLite::NoiseType_Cellular);
				float NoiseVal = Noise->GetNoise((VoxelDiameter * x + ChunkPosition.X),
				                                 (VoxelDiameter * y + ChunkPosition.Y),
				                                 (VoxelDiameter * z + ChunkPosition.Z));
				NoiseVal *= NoiseRatio * InverseMultiplier;
				float NoiseDifference = VoxelSDF - NoiseVal;
				Voxels[GetVoxelIndex(x, y, z)] = VoxelSDF + NoiseRatio * NoiseDifference;
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


	//For each voxel in the chunk (including the last one)
	for (int X = 0; X < VoxelsPerSide; ++X)
	{
		for (int Y = 0; Y < VoxelsPerSide; ++Y)
		{
			for (int Z = 0; Z < VoxelsPerSide; ++Z)
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

/**
 * @brief Calculate a single voxels Signed Distance Field value in relation to a box
 * @param Point 
 * @param BoxPosition 
 * @param BoxSize 
 * @param BoxRotation 
 * @return 
 */
float AMarchingChunkTerrain::BoxSDF(const FVector& Point, const FVector BoxPosition, FVector BoxSize, FQuat BoxRotation)
{
	FVector HalfSize = BoxSize * 0.5f;
	FQuat InvRotation = BoxRotation.Inverse();
	FVector LocalPoint = InvRotation.RotateVector(Point - BoxPosition);


	FVector d = FVector(
		FMath::Abs(LocalPoint.X) - HalfSize.X,
		FMath::Abs(LocalPoint.Y) - HalfSize.Y,
		FMath::Abs(LocalPoint.Z) - HalfSize.Z
	);

	float outsideDistance = d.GetMax();
	float insideDistance = FMath::Max(d.X, FMath::Max(d.Y, d.Z));

	return outsideDistance < 0 ? insideDistance : outsideDistance;
}

enum class PredominantOrientation
{
	XY,
	XZ,
	YZ
};

PredominantOrientation GetPredominantOrientation(const FVector& Normal)
{
	FVector AbsNormal = FVector(FMath::Abs(Normal.X), FMath::Abs(Normal.Y), FMath::Abs(Normal.Z));
	if (AbsNormal.X > AbsNormal.Y && AbsNormal.X > AbsNormal.Z)
	{
		return PredominantOrientation::YZ;
	}
	if (AbsNormal.Y > AbsNormal.X && AbsNormal.Y > AbsNormal.Z)
	{
		return PredominantOrientation::XZ;
	}
	return PredominantOrientation::XY;
}

FVector2D ComputeUV(const FVector& Vertex, PredominantOrientation Orientation)
{
	// Scale and offset values can be adjusted as needed
	float ScaleFactor = 0.01f; // Adjust to achieve desired texture tiling
	switch (Orientation)
	{
	case PredominantOrientation::XY:
		return FVector2D(Vertex.X * ScaleFactor, Vertex.Y * ScaleFactor);
	case PredominantOrientation::XZ:
		return FVector2D(Vertex.X * ScaleFactor, Vertex.Z * ScaleFactor);
	case PredominantOrientation::YZ:
		return FVector2D(Vertex.Y * ScaleFactor, Vertex.Z * ScaleFactor);
	default:
		return FVector2D(0, 0);
	}
}

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

		auto V1 = EdgeVertex[TriangleConnectionTable[VertexMask][3 * i + 0]] * VoxelDiameter;
		auto V2 = EdgeVertex[TriangleConnectionTable[VertexMask][3 * i + 1]] * VoxelDiameter;
		auto V3 = EdgeVertex[TriangleConnectionTable[VertexMask][3 * i + 2]] * VoxelDiameter;


		auto Normal = FVector::CrossProduct(V2 - V1, V3 - V1);

		PredominantOrientation Orientation = GetPredominantOrientation(Normal);

		//Compute UVs
		auto UV1 = ComputeUV(V1, Orientation);
		auto UV2 = ComputeUV(V2, Orientation);
		auto UV3 = ComputeUV(V3, Orientation);
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

		MeshData.UVs.Append({
			UV1,
			UV2,
			UV3
		});

		VertexCount += 3;
	}
}

void AMarchingChunkTerrain::ApplyMesh() const
{
	Mesh->CreateMeshSection(
		0,
		MeshData.Vertices,
		MeshData.Triangles,
		MeshData.Normals,
		MeshData.UVs,
		MeshData.Colors,
		Tangents,
		true
	);
	Mesh->SetMaterial(0, Material);
	Mesh->bCastShadowAsTwoSided = true;
	//esh->CreateStat
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

void AMarchingChunkTerrain::ShowDebug()
{
	if (bDebugChunk)
	{
		FVector halfSize = FVector(ChunkSize / 2.0, ChunkSize / 2.0, ChunkSize / 2.0);
		DrawDebugBox(GetWorld(), ChunkPosition + halfSize.X, halfSize, FColor::Silver, false, -1, 0, 2.0f);
	}
	if (bDebugVoxels && !Voxels.IsEmpty())
	{
		FVector voxelPosition = FVector::ZeroVector;
		// For each Voxel, show the position and the SDF value as a color. 
		for (double x = 0; x <= VoxelsPerSide; ++x)
		{
			for (double y = 0; y <= VoxelsPerSide; ++y)
			{
				for (double z = 0; z <= VoxelsPerSide; ++z)
				{
					int VoxelValue = (Voxels[GetVoxelIndex(x, y, z)]) * 255;
					const float VoxelSign = VoxelValue > 0 ? 1.0f : 0.10f;
					VoxelValue = abs(VoxelValue);
					FColor Color = FColor(VoxelValue * (1 - VoxelSign), VoxelValue * VoxelSign, 0, VoxelValue);
					voxelPosition = FVector(x * VoxelDiameter, y * VoxelDiameter, z * VoxelDiameter) + ChunkPosition;
					DrawDebugSphere(GetWorld(), voxelPosition, 12.0f, 4, Color, false, -1, 0, 2.0f);
				}
			}
		}
	}
}
