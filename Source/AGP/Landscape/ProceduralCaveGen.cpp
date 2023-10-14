// Fill out your copyright notice in the Description page of Project Settings.


#include "ProceduralCaveGen.h"


#include "Algo/RandomShuffle.h"
#include "Kismet/GameplayStatics.h"
#include "Voxels/MarchingChunkTerrain.h"
#include "EngineUtils.h"
#include "Components/PointLightComponent.h"
#include "Engine/PointLight.h"
#include "Engine/DirectionalLight.h"

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
	//Level Generation is handled in Tick
}

// Called every frame
void AProceduralCaveGen::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bDebugOnly1Chunk && VoxelDensity < 12)
	{
		bSmallNumVoxels = true;
	}
	else
	{
		bSmallNumVoxels = false;
	}


	//If you click regenerate - clears map and generates the level
	if (bShouldRegenerate)
	{
		AvgConnectionDistance = (LevelSize / (NumBoxesPerPath));
		ClearMap();

		//Level Generation
		Boxes = GenerateGuaranteedPathBoxes();
		GenerateInterconnects();

		//Add All generated items to an array for chunk check
		AddAllObjects();
		//Spawn Meshes
		GenerateMesh();
		bShouldRegenerate = false;
	}

	//Debug to visualize boxes
	if (!Boxes.IsEmpty() && bDebugView)
	{
		DebugShow();
	}

	if (!RoomNodes.IsEmpty() && bDebugNavNodes)
	{
		DebugShowNavNodes();
	}
}

/**
 * @brief Helper Function to calculate a gradient between two boxes
 * @param BoxA FLevelBox Starting point
 * @param BoxB FLevelBox End point
 * @return float Gradient between the two boxes (deltaY/deltaX), or a large number if deltaX is very small (to avoid division by zero)
 */
float CalculateGradient(const FLevelBox& BoxA, const FLevelBox& BoxB)
{
	float deltaY = BoxB.Position.Z - BoxA.Position.Z;

	// Compute horizontal distance
	float deltaX = FVector(BoxA.Position.X - BoxB.Position.X, BoxA.Position.Y - BoxB.Position.Y, 0).Size();

	// Check for a very small deltaX to avoid division by zero
	if (FMath::Abs(deltaX) < KINDA_SMALL_NUMBER)
	{
		return MAX_FLT; // return a large number to indicate a very steep gradient
	}

	return deltaY / deltaX;
}

//ToDo: LevelBoxes as a class - Move repetitive code to CreateBox / internal code
/**
 * @brief Creates the boxes (Rooms and tunnels) for the world
 * @return The array of boxes created
 */
TArray<FLevelBox> AProceduralCaveGen::GenerateGuaranteedPathBoxes()
{
	//Start and End Locations
	FVector Start = FVector(GetActorLocation());
	AddPlayerStartAtLocation(Start);
	FVector End = FVector(LevelSize, 0, FMath::RandRange(-HeightDifference, 0.0f));

	TArray<FLevelBox> boxes;

	//Start Box
	FLevelBox StartBox
	{
		Start,
		FVector(
			FMath::RandRange(MinBoxSize.X, MaxBoxSize.X),
			FMath::RandRange(MinBoxSize.Y, MaxBoxSize.Y),
			FMath::RandRange(MinBoxSize.Z, MaxBoxSize.Z)),
		EBoxType::Start
	};
	boxes.Add(StartBox);
	CreateBox(StartBox);

	//Log Start box position and size
	UE_LOG(LogTemp, Warning, TEXT("Start Box Position is %s"), *StartBox.Position.ToString());

	//Optional entrance sunlight - if DirectionalLight Present Get Direction of sunlight. Point box towards sun
	FVector SunDirection = FVector::ZeroVector;
	ADirectionalLight* Sun;
	for (TActorIterator<ADirectionalLight> ActorItr(GetWorld()); ActorItr; ++ActorItr)
	{
		if (*ActorItr)
		{
			Sun = *ActorItr;
			SunDirection = -Sun->GetActorRotation().Vector();
			UE_LOG(LogTemp, Warning, TEXT("Sun Direction is %s"), *SunDirection.ToString());
			//FVector SunDirection = GetSunDirection();
			FLevelBox Entrance{
				Start + SunDirection * ChunkSize * 2,
				FVector(
					StartBox.Size.X / 4,
					StartBox.Size.Y / 4,
					StartBox.Size.Z / 4),
				EBoxType::Normal, SunDirection.ToOrientationQuat()
			};
			boxes.Add(Entrance);

			CreateTunnel(StartBox, Entrance);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("No Sun Found"));
		}
	}


	//Find paths up to number of paths
	FVector LastPosition = Start;
	for (int i = 0; i < NumPaths; i++)
	{
		FInnerArray Path;
		Paths.Add(Path);
		LastPosition = Start;
		for (int j = 0; j < NumBoxesPerPath; j++)
		{
			FLevelBox box;

			// Get a direction biased towards the destination
			FVector BiasedDirection = (End - LastPosition).GetSafeNormal();

			// Randomize within a hemisphere towards the destination
			FVector RandomDirection = FMath::VRandCone(BiasedDirection, FMath::DegreesToRadians(75.0f));

			// Constrain the vertical movement
			float MaxVerticalOffset = 200.0f; // Set this to the max vertical difference you want between rooms
			float verticalComponent = FMath::Clamp(RandomDirection.Z, -MaxVerticalOffset, MaxVerticalOffset);
			RandomDirection.Z = verticalComponent;

			FVector newPosition = LastPosition + RandomDirection * FMath::RandRange(
				0.5f * AvgConnectionDistance, AvgConnectionDistance);
			box.Position = newPosition;
			box.Size = FVector(FMath::RandRange(MinBoxSize.X, MaxBoxSize.X),
			                   FMath::RandRange(MinBoxSize.Y, MaxBoxSize.Y),
			                   FMath::RandRange(MinBoxSize.Z, MaxBoxSize.Z));
			box.Type = EBoxType::Normal;

			FLevelBox lastBox = (j == 0) ? StartBox : Paths[i].Path[j - 1];

			//If box doesn't collide with any other boxes Or Gradient is too steep
			//Add it to the list of boxes	
			if (BoxPositionValid(box, boxes) && FMath::Abs(CalculateGradient(box, lastBox)) <= 0.3)
			{
				boxes.Add(box);
				CreateBox(box);

				Paths[i].Path.Add(box);
				CreateTunnel(lastBox, box);
				LastPosition = newPosition;
			}
			else
			{
				j--;
			}
		}
	}

	//End Box
	FLevelBox EndBox{End, FVector(0, 0, 0), EBoxType::End};
	EndBox.Size = FVector
	(
		FMath::RandRange(MinBoxSize.X, MaxBoxSize.X),
		FMath::RandRange(MinBoxSize.Y, MaxBoxSize.Y),
		FMath::RandRange(MinBoxSize.Z, MaxBoxSize.Z)
	);
	EndBox.Type = EBoxType::End;
	CreateBox(EndBox);
	boxes.Add(EndBox);

	//Join Paths to end box
	for (int i = 0; i < NumPaths; i++)
	{
		FLevelBox lastBox = Paths[i].Path[Paths[i].Path.Num() - 1];
		CreateTunnel(lastBox, EndBox);
	}


	return boxes;
}

/**
 * @brief Generate Mesh using MarchingChunkTerrain
 */
void AProceduralCaveGen::GenerateMesh()
{
	//For loop for X/Y/Z chunks within levelSize
	int ChunkAmounts = (LevelSize) / ChunkSize;

	int StartChunkXY = -ChunkAmounts / 2;
	int StartChunkZ = -ChunkAmounts / 2;
	if (bDebugOnly1Chunk)
	{
		ChunkAmounts = 0;
		StartChunkXY = 0;
		StartChunkZ = 0;
	}
	//OffsetChunkStart = FVector(-ChunkAmounts, -ChunkAmounts, -ChunkAmounts) * ChunkSize;

	for (int x = StartChunkXY; x <= ChunkAmounts; x++)
	{
		for (int y = StartChunkXY; y <= ChunkAmounts; y++)
		{
			for (int z = StartChunkZ; z <= ChunkAmounts; z++)
			{
				FVector ChunkOffset = FVector(ChunkSize / 2, ChunkSize / 2, ChunkSize / 2);

				FVector ChunkPosition = FVector(x * ChunkSize - ChunkSize / 2, y * ChunkSize - ChunkSize / 2,
				                                z * ChunkSize - ChunkSize / 2);
				//Check if chunk intersects with any boxes - if not, skip it
				//ChunkSize is doubled to be safe
				FLevelBox ChunkBox{
					ChunkPosition + ChunkOffset, FVector(ChunkSize * 1.2, ChunkSize * 1.2, ChunkSize * 1.2),
					EBoxType::Normal
				};
				for (FLevelBox Object : AllObjects)
				{
					if (BoxesIntersect(ChunkBox, Object))
					{
						const auto Chunk = GetWorld()->SpawnActorDeferred<AMarchingChunkTerrain>
						(
							AMarchingChunkTerrain::StaticClass(),
							FTransform(),
							this
						);
						Chunk->bUpdateMesh = bUpdateMesh;

						Chunk->LevelSize = static_cast<int>(LevelSize);
						Chunk->Boxes = Boxes;
						Chunk->Tunnels = Tunnels;
						Chunk->Material = Material;

						Chunk->ChunkPosition = ChunkPosition;
						Chunk->ChunkSize = ChunkSize;
						Chunk->VoxelsPerSide = VoxelDensity;

						Chunk->NoiseRatio = NoiseRatio;

						Chunk->bDebugInvertSolids = bDebugInvertSolids;
						Chunk->bDebugChunk = bDebugChunk;
						Chunk->bDebugVoxels = bDebugVoxels;

						UGameplayStatics::FinishSpawningActor(Chunk, FTransform());

						Chunks.Add(Chunk);

						Chunk->AttachToActor(this, FAttachmentTransformRules::KeepRelativeTransform);

						break;
					}
				}
			}
		}
	}
}


/**
 * @brief Creates tunnels between boxes with the same i \n
 * Uses the Paths and Connectedness (0->1) to determine how many tunnels to create \n
 * At 1.0 Connectedness it will attempt to generate all possible tunnels
 */
void AProceduralCaveGen::GenerateInterconnects()
{
	if (Paths.Num() == 1)
	{
		return;
	}
	const int MaxInterconnects = Paths[0].Path.Num() * PathInterconnectedness;
	TArray<int> InterconnectIndices;
	for (int i = 0; i < Paths[0].Path.Num(); i++)
	{
		InterconnectIndices.Add(i);
	}
	Algo::RandomShuffle(InterconnectIndices);
	int NumInterconnects = 0;
	int AttemptNum = 0;
	while (AttemptNum < Paths[0].Path.Num() && NumInterconnects < MaxInterconnects)
	{
		FLevelBox& BoxA = Paths[0].Path[InterconnectIndices[AttemptNum]];
		FLevelBox& BoxB = Paths[1].Path[InterconnectIndices[AttemptNum]];

		if (FMath::Abs(CalculateGradient(BoxA, BoxB)) <= 0.3) // 30% gradient or 0.3 in decimal form
		{
			NumInterconnects++;
			CreateTunnel(BoxA, BoxB);
		}
		AttemptNum++;
	}
}

/**
 * @brief Helper function that creates associated items to be spawned at each room.
 * @param Box is the room to be spawned in.
 */
void AProceduralCaveGen::CreateBox(FLevelBox& Box)
{
	if (UWorld* World = GetWorld())
	{
		APointLight* Light = World->SpawnActor<APointLight>(Box.Position, FRotator::ZeroRotator);
		Light->SetBrightness(20000.0f);
		Light->PointLightComponent->bUseTemperature = 1.0;
		Light->PointLightComponent->SetTemperature(2500);
	}
	if (ANavigationNode* RoomNode = GetWorld()->SpawnActor<ANavigationNode>(
		ANavigationNode::StaticClass(), Box.Position, FRotator::ZeroRotator))
	{
		Box.RoomNode = RoomNode;
		RoomNodes.Add(Box.RoomNode);
		RoomNode->AttachToActor(this, FAttachmentTransformRules::KeepRelativeTransform);
	}
}

/**
 * @brief Calculates an offset from the center of the box, towards the wall. This is the point a tunnel will spawn at
 * @param Box - The box to calculate the offset from
 * @param Direction - The direction the tunnel goes (usually center of this box towards center of another box)
 * @return FVector Offset, the offset vector from the center of the box towards the wall
 */
FVector AProceduralCaveGen::CalculateBoxOffset(const FLevelBox& Box, const FVector& Direction) const
{
	FVector Offset;

	Offset.X = (Direction.X > 0) ? Box.Size.X / 2 : (Direction.X < 0) ? -Box.Size.X / 2 : 0;
	Offset.Y = (Direction.Y > 0) ? Box.Size.Y / 2 : (Direction.Y < 0) ? -Box.Size.Y / 2 : 0;
	Offset.Z = (Direction.Z > 0) ? Box.Size.Z / 2 : (Direction.Z < 0) ? -Box.Size.Z / 2 : 0;

	//Move XY offset points towards the center of the box by the tunnelSize
	Offset.X += (Direction.X > 0) ? -TunnelSize / 2 : (Direction.X < 0) ? 150 / 2 : 0;
	Offset.Y += (Direction.Y > 0) ? -TunnelSize / 2 : (Direction.Y < 0) ? 150 / 2 : 0;


	// Adjust for Z position to be closer to the ground
	Offset.Z = -Box.Size.Z / 2;

	// Optional: Add a fixed elevation offset so that the tunnel is slightly above the ground// adjust as needed
	Offset.Z += TunnelSize / 2;

	return Offset;
}

/**
 * @brief Creates a tunnel and adds it to the tunnel array
 * @param StartBox 
 * @param TargetBox 
 */
void AProceduralCaveGen::CreateTunnel(FLevelBox& StartBox, FLevelBox& TargetBox)
{
	FTunnel Tunnel;
	Tunnel.StartBox = &StartBox;
	Tunnel.EndBox = &TargetBox;

	FVector Direction = (TargetBox.Position - StartBox.Position).GetSafeNormal();
	FVector StartOffset = CalculateBoxOffset(StartBox, Direction);
	FVector EndOffset = CalculateBoxOffset(TargetBox, -Direction); // Notice the direction is negated

	FVector Start = StartBox.Position + StartOffset;
	FVector End = TargetBox.Position + EndOffset;

	Direction = (End - Start).GetSafeNormal();

	Tunnel.Position = (Start + End) / 2;
	Tunnel.Size = FVector((End - Start).Size() + TunnelSize, TunnelSize, TunnelSize);
	Tunnel.Rotation = FQuat::FindBetweenNormals(FVector::ForwardVector, Direction);

	Tunnels.Add(Tunnel);

	//CreateNavNode connections reciprocally
	//Add both start node and end node to an array
	TArray<ANavigationNode*> Nodes = {StartBox.RoomNode, TargetBox.RoomNode};

	//Unrolled loop to connect both nodes to each other
	if (!StartBox.RoomNode->GetConnectedNodes().Contains(TargetBox.RoomNode))
	{
		StartBox.RoomNode->SetConnectedNodes(TargetBox.RoomNode);
	}
	if (!TargetBox.RoomNode->GetConnectedNodes().Contains(StartBox.RoomNode))
	{
		TargetBox.RoomNode->SetConnectedNodes(StartBox.RoomNode);
	}
}

/**
 * @brief Adds all rooms, tunnels to a list of one type of object - AllObjects
 */
void AProceduralCaveGen::AddAllObjects()
{
	AllObjects.Append(Boxes);

	//ToDo Overlap Checks for rotated boxes
	for (FTunnel Tunnel : Tunnels)
	{
		FLevelBox TempBox{Tunnel.Position, Tunnel.Size, EBoxType::Normal, Tunnel.Rotation};
		AllObjects.Add(TempBox);
	}
}

/**
 * @brief Check if two boxes intersect from topdown (XY only) NOT INCLUDING ROTATION
 * @param BoxA FLevelBox to check
 * @param BoxB Other FLevelBox
 * @return True if boxes intersect, false otherwise
 */
bool AProceduralCaveGen::BoxesIntersect2D(const FLevelBox& BoxA, const FLevelBox& BoxB)
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
	/*if (BoxA.Position.Z + BoxA.Size.Z < BoxB.Position.Z ||
		BoxB.Position.Z + BoxB.Size.Z < BoxA.Position.Z)
	{
		return false;
	}*/

	// If there's no gap along any axis, then the boxes intersect
	return true;
}


/**
 * @brief Check if box position is valid
 * @param NewBox Box to check
 * @param AllBoxes All boxes to check against
 * @return True if Valid, false otherwise
 */
bool AProceduralCaveGen::BoxPositionValid(const FLevelBox& NewBox, const TArray<FLevelBox>& AllBoxes)
{
	for (const FLevelBox& Box : AllBoxes)
	{
		if (BoxesIntersect2D(Box, NewBox))
		{
			return false;
		}
	}
	return true;
}


/**
 * @brief Empties Boxes,Tunnels, Paths, Associated objects /n
 * and clears the mesh
 */
void AProceduralCaveGen::ClearMap()
{
	Boxes.Empty();
	Tunnels.Empty();
	AllObjects.Empty();
	RoomNodes.Empty();
	if (!Paths.IsEmpty())
	{
		for (FInnerArray& Path : Paths)
		{
			Path.Path.Empty();
		}
	}
	Paths.Empty();

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

	//Iterate through all APickupBase actors in the world and destroy them
	for (TActorIterator<AMarchingChunkTerrain> It(GetWorld()); It; ++It)
	{
		if (*It)
		{
			(*It)->ClearMesh();
			(*It)->Destroy();
		}
	}

	//Iterate through all APickupBase actors in the world and destroy them
	for (TActorIterator<ANavigationNode> It(GetWorld()); It; ++It)
	{
		if (*It)
		{
			(*It)->Destroy();
		}
	}
	for (TActorIterator<APlayerStart> It(GetWorld()); It; ++It)
	{
		if (*It)
		{
			(*It)->Destroy();
		}
	}

	for (TActorIterator<APointLight> It(GetWorld()); It; ++It)
	{
		if (*It)
		{
			(*It)->Destroy();
		}
	}
}

/**
 * @brief Adds a player spawn point
 * @param Location Location to add player spawn point
 */
void AProceduralCaveGen::AddPlayerStartAtLocation(const FVector& Location)
{
	if (GetWorld())
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		APlayerStart* NewPlayerStart = GetWorld()->SpawnActor<APlayerStart>(
			APlayerStart::StaticClass(), Location, FRotator(0, 0, 0), SpawnParams);

		if (NewPlayerStart)
		{
			// Optionally, you can set any other properties on NewPlayerStart
		}
	}
}

/**
 * @brief Draw Boxes and Tunnel widgets
 */
void AProceduralCaveGen::DebugShow()
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

void AProceduralCaveGen::DebugShowNavNodes()
{
	if (!RoomNodes.IsEmpty())
	{
		for (ANavigationNode* RoomNode : RoomNodes)
		{
			RoomNode->DebugSetVisibility(true);
		}
	}
}


//Box intersection functions follow below - I may switch to UE's functions but these are pretty fast
//They check if box intersects box incl rotation over XYZ. 
TArray<FVector> CalculateBoxCorners(const FLevelBox& Box)
{
	TArray<FVector> Corners;

	// Half sizes for each dimension
	FVector HalfSize = Box.Size * 0.5f;

	// Calculate the local position of each corner as if the box were axis-aligned
	Corners.Add(FVector(-HalfSize.X, -HalfSize.Y, -HalfSize.Z));
	Corners.Add(FVector(-HalfSize.X, HalfSize.Y, -HalfSize.Z));
	Corners.Add(FVector(HalfSize.X, HalfSize.Y, -HalfSize.Z));
	Corners.Add(FVector(HalfSize.X, -HalfSize.Y, -HalfSize.Z));
	Corners.Add(FVector(-HalfSize.X, -HalfSize.Y, HalfSize.Z));
	Corners.Add(FVector(-HalfSize.X, HalfSize.Y, HalfSize.Z));
	Corners.Add(FVector(HalfSize.X, HalfSize.Y, HalfSize.Z));
	Corners.Add(FVector(HalfSize.X, -HalfSize.Y, HalfSize.Z));

	// Now, rotate each corner around the box's center position
	for (FVector& Corner : Corners)
	{
		// Rotate the corner around the box center
		Corner = Box.Rotation.RotateVector(Corner);

		// Translate the corner to the box's world position
		Corner += Box.Position;
	}

	return Corners;
}

TArray<FVector> GetOrientationVectors(const FQuat& Rotation)
{
	// Define the local axes
	const FVector LocalX(1.0f, 0.0f, 0.0f);
	const FVector LocalY(0.0f, 1.0f, 0.0f);
	const FVector LocalZ(0.0f, 0.0f, 1.0f);

	TArray<FVector> Orientation;
	Orientation.Add(Rotation.RotateVector(LocalX)); // Local X-axis
	Orientation.Add(Rotation.RotateVector(LocalY)); // Local Y-axis
	Orientation.Add(Rotation.RotateVector(LocalZ)); // Local Z-axis

	return Orientation;
}

TArray<FVector> FindAxesToTest(const FLevelBox& BoxA, const FLevelBox& BoxB)
{
	TArray<FVector> Axes;

	// Get the orientation vectors (local axes) for each box
	TArray<FVector> BoxA_Orientation = GetOrientationVectors(BoxA.Rotation);
	TArray<FVector> BoxB_Orientation = GetOrientationVectors(BoxB.Rotation);

	// Add the face normals of BoxA to the list
	Axes.Append(BoxA_Orientation);

	// Add the face normals of BoxB to the list
	Axes.Append(BoxB_Orientation);

	// Compute the cross products of each pair of edges from both boxes
	for (int i = 0; i < 3; ++i)
	{
		for (int j = 0; j < 3; ++j)
		{
			FVector CrossProduct = FVector::CrossProduct(BoxA_Orientation[i], BoxB_Orientation[j]);

			// If the cross product is non-zero, add it to the list
			if (!CrossProduct.IsNearlyZero())
			{
				Axes.Add(CrossProduct.GetSafeNormal());
			}
		}
	}

	return Axes;
}

// ProjectCornersOntoAxis projects the corners of a box onto an axis and returns the min and max of the projection.
FVector2D ProjectCornersOntoAxis(const TArray<FVector>& Corners, const FVector& Axis)
{
	float MinDotProduct = FLT_MAX;
	float MaxDotProduct = FLT_MIN;

	for (const FVector& Corner : Corners)
	{
		// The dot product of the corner and the axis gives the projection along the axis.
		float DotProduct = FVector::DotProduct(Corner, Axis);

		MinDotProduct = FMath::Min(MinDotProduct, DotProduct);
		MaxDotProduct = FMath::Max(MaxDotProduct, DotProduct);
	}

	return FVector2D(MinDotProduct, MaxDotProduct); // This 2D vector holds the min and max of the projection.
}

// ProjectionsOverlap checks if the projections of two boxes onto an axis overlap.
bool ProjectionsOverlap(const FVector2D& ProjectionA, const FVector2D& ProjectionB)
{
	// If one projection is entirely less than the other, they don't overlap.
	if (ProjectionA.Y < ProjectionB.X || ProjectionB.Y < ProjectionA.X)
	{
		return false;
	}

	// Otherwise, they do overlap.
	return true;
}


/**
 * @brief Check if two boxes intersect (XYZ + Rotations)
 * @param BoxA FLevelBox to check
 * @param BoxB Other FLevelBox
 * @return True if boxes intersect, false otherwise
 */
bool AProceduralCaveGen::BoxesIntersect(const FLevelBox& BoxA, const FLevelBox& BoxB)
{
	// Step 1: Calculate the corners of each box.
	TArray<FVector> BoxACorners = CalculateBoxCorners(BoxA);
	TArray<FVector> BoxBCorners = CalculateBoxCorners(BoxB);

	// Step 2: Find axes to test.
	TArray<FVector> Axes = FindAxesToTest(BoxA, BoxB);

	for (FVector Axis : Axes)
	{
		// Step 3: Project corners onto the axis.
		FVector2D ProjectionA = ProjectCornersOntoAxis(BoxACorners, Axis);
		FVector2D ProjectionB = ProjectCornersOntoAxis(BoxBCorners, Axis);

		// Step 4: Check for overlap.
		if (!ProjectionsOverlap(ProjectionA, ProjectionB))
		{
			return false; // There's a separating axis, so the boxes don't intersect.
		}
	}

	// Step 5: If no separating axis was found, the boxes intersect.
	return true;
}
