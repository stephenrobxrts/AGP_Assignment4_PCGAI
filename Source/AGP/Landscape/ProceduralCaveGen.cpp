// Fill out your copyright notice in the Description page of Project Settings.


#include "ProceduralCaveGen.h"

#include "Algo/RandomShuffle.h"
#include "Kismet/GameplayStatics.h"
#include "Voxels/MarchingChunkTerrain.h"
#include "EngineUtils.h"
#include "../Pickups/ArtefactPickup.h"
#include "Engine/PointLight.h"
#include "Engine/DirectionalLight.h"
#include "../Pickups/TorchPickup.h"
#include "AGP/Pickups/PedestalInteract.h"

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
		if (Mode == EGenerationMode::GridBased)
		{
			RoomBoxes = GenGridBasedLevel();
		}
		else
		{
			RoomBoxes = GenRandomTraversalLevel();
		}
		
		GenerateInterconnects();

		//Add All generated items to an array for chunk check
		AddAllObjects();
		//Spawn Meshes
		GenerateMesh();
		bShouldRegenerate = false;

		AttachTorchToWalls();
	}

	//Debug to visualize boxes
	if (!RoomBoxes.IsEmpty() && bDebugView)
	{
		DebugShow();
	}

	if ((!RoomNodes.IsEmpty() || !WalkNodes.IsEmpty()) && bDebugNavNodes)
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

/**
 * @brief Generate the level by randomly walking towards the end node with inaccuracy built in \n
 * This makes the level more interesting but can easily break AI or make rooms inaccessible
 * @return Array of Room Boxes
 */
TArray<FLevelBox> AProceduralCaveGen::GenRandomTraversalLevel()
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
		FQuat::Identity,
		TArray<ANavigationNode*>(),
		EBoxType::Start

	};
	boxes.Add(StartBox);
	CreateBox(StartBox);

	//Log Start box position and size
	//(LogTemp, Warning, TEXT("Start Box Position is %s"), *StartBox.Position.ToString());

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
				SunDirection.ToOrientationQuat(),
				TArray<ANavigationNode*>(),
				EBoxType::Normal,
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
			float verticalComponent = FMath::Clamp(RandomDirection.Z, static_cast<double>(-MaxVerticalOffset), static_cast<double>(MaxVerticalOffset));
			RandomDirection.Z = verticalComponent;

			FVector newPosition = LastPosition + RandomDirection * StaticCast<double>(FMath::RandRange(
				0.5f * AvgConnectionDistance, AvgConnectionDistance));
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
	FLevelBox EndBox{End, FVector(0, 0, 0), FQuat::Identity, TArray<ANavigationNode*>(), EBoxType::End};
	EndBox.Size = FVector
	(
		FMath::RandRange(MinBoxSize.X, MaxBoxSize.X),
		FMath::RandRange(MinBoxSize.Y, MaxBoxSize.Y),
		FMath::RandRange(MinBoxSize.Z, MaxBoxSize.Z)
	);
	EndBox.Type = EBoxType::End;
	CreateBox(EndBox);
	GenerateEndPedestal(EndBox);
	boxes.Add(EndBox);

	//Join Paths to end box
	for (int i = 0; i < NumPaths; i++)
	{
		FLevelBox lastBox = Paths[i].Path[Paths[i].Path.Num() - 1];
		CreateTunnel(lastBox, EndBox);
	}


	GenerateLevelItems(boxes);
	return boxes;
}

/**
 * @brief Generate the level based on a grid - with rooms then displaced randomly \n
 * This is the default mode - works well with AI and is more predictable
 * @return Array of all room Boxes
 */TArray<FLevelBox> AProceduralCaveGen::GenGridBasedLevel()
{
	//create start room
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
		FQuat::Identity,
		TArray<ANavigationNode*>(),
		EBoxType::Start

	};
	boxes.Add(StartBox);
	CreateBox(StartBox);

	//Get direction and distance between start and end
	FVector Direction = (End - Start).GetSafeNormal();
	float Distance = (End - Start).Size();

	//Create a box at the end of the path
	FLevelBox EndBox
	{
		End,
		FVector(
			500.0f,
			500.0f,
			500.0f),
		FQuat::Identity,
		TArray<ANavigationNode*>(),
		EBoxType::End
	};
	boxes.Add(EndBox);
	GenerateEndPedestal(EndBox);
	CreateBox(EndBox);

	//Find average tunnel length by dividing distance by number of boxes per path
	float AvgTunnelLength = Distance / (NumBoxesPerPath + 1);

	//Find the angle from start -> leftmost path first box such that the tunnel length
	// between leftmost path Box1 and second path Box1 is equal to the average tunnel length
	//So leftmost box will be NumPaths/2 * AvgTunnelLength away from the centerline
	//And leftmost box will be avgTunnelLength away from StartBox
	float Angle = FMath::Atan2((NumPaths - 1.0f) / 2.0f * AvgTunnelLength, AvgTunnelLength);
	float LeftmostPathY = -((NumPaths - 1.0f) / 2.0f * AvgTunnelLength);


	for (int i = 0; i < NumPaths; i++)
	{
		FInnerArray Path;
		Paths.Add(Path);

		float PathY = LeftmostPathY + i * AvgTunnelLength;
		FVector PathStart = Start + FVector(0, PathY, 0);
		FVector PathEnd = End + FVector(0, PathY, 0);
		FVector PathDirection = (PathEnd - PathStart).GetSafeNormal();
		//for each box in NumBoxesPerPath - create a box avgPathLength away in X from previous box
		for (int j = 0; j < NumBoxesPerPath; j++)
		{
			FLevelBox Box;
			Box.Position = PathStart + PathDirection * StaticCast<double>(AvgTunnelLength) * (j + 1);
			Box.Size = FVector(FMath::RandRange(MinBoxSize.X, MaxBoxSize.X),
			                   FMath::RandRange(MinBoxSize.Y, MaxBoxSize.Y),
			                   FMath::RandRange(MinBoxSize.Z, MaxBoxSize.Z));
			Box.Type = EBoxType::Normal;
			FLevelBox lastBox = (j == 0) ? StartBox : Paths[i].Path[j - 1];

			//Shift the box by up to 1/3 of AvgTunnelPath +/- in X and Y
			Box.Position.X += FMath::RandRange(-AvgTunnelLength / 3, AvgTunnelLength / 3);
			Box.Position.Y += FMath::RandRange(-AvgTunnelLength / 3, AvgTunnelLength / 3);
			//Shift the height of the box by up to 10% of AvgTunnelPath +/- in Z
			Box.Position.Z += FMath::RandRange(-AvgTunnelLength / 10, AvgTunnelLength / 10);
			
			boxes.Add(Box);
			CreateBox(Box);

			Paths[i].Path.Add(Box);
			CreateTunnel(lastBox, Box);
		}
	}

	//Join Paths to end box
	for (int i = 0; i < NumPaths; i++)
	{
		FLevelBox lastBox = Paths[i].Path[Paths[i].Path.Num() - 1];
		CreateTunnel(lastBox, EndBox);
	}

	GenerateLevelItems(boxes);
	return boxes;
}

/**
 * @brief Places 4 artefacts around the level, each one in a random room \n
 * Does not place artefact in start/end rooms
 * @param Rooms Room Box Objects
 */
void AProceduralCaveGen::GenerateLevelItems(TArray<FLevelBox>& Rooms)
{
	for (int i = 0 ; i < NumArtefactsToPlace ; i++)
	{
		int rngRoom = FMath::RandRange(0, Rooms.Num() - 1);
		if (Rooms[rngRoom].Artefact != nullptr || Rooms[rngRoom].Type != EBoxType::Normal)
		{
			i--;
		}
		else
		{
			FVector ArtefactLocation = FVector(Rooms[rngRoom].Position.X, Rooms[rngRoom].Position.Y, Rooms[rngRoom].Position.Z - Rooms[rngRoom].Size.Z / 2);
			ArtefactLocation.Z += 100.0f;
			AArtefactPickup* Artefact = GetWorld()->SpawnActor<AArtefactPickup>(ArtefactBP, ArtefactLocation, FRotator::ZeroRotator);
			Artefact->AttachToActor(this, FAttachmentTransformRules::KeepRelativeTransform);
			Artefact->SetArtefactID(i);
			Rooms[rngRoom].Artefact = Artefact;
		}
	}
}

/**
 * @brief Generates the pedestal for the end room
 * @param Room The end Room FLevelBox
 */
void AProceduralCaveGen::GenerateEndPedestal(FLevelBox& Room)
{
	FVector PedestalLocation = FVector(Room.Position.X, Room.Position.Y, Room.Position.Z - Room.Size.Z / 2);
	APedestalInteract* Pedestal = GetWorld()->SpawnActor<APedestalInteract>(PedestalBP, PedestalLocation, FVector(0, 1, 0).ToOrientationRotator());
	Pedestal->AttachToActor(this, FAttachmentTransformRules::KeepRelativeTransform);
	Room.Pedestal = Pedestal;
}


/**
 * @brief Generate Mesh using MarchingChunkTerrain
 */
void AProceduralCaveGen::GenerateMesh()
{
	//For loop for X/Y/Z chunks within levelSize
	int ChunkAmounts = (LevelSize + 0.2f * LevelSize) / ChunkSize;

	int StartChunkXY = -ChunkAmounts / 2;
	int StartChunkZ = -ChunkAmounts / 2;
	if (bDebugOnly1Chunk)
	{
		ChunkAmounts = 0;
		StartChunkXY = 0;
		StartChunkZ = 0;
	}
	//OffsetChunkStart = FVector(-ChunkAmounts, -ChunkAmounts, -ChunkAmounts) * ChunkSize;


	NoiseParams.SetParams(NoiseType, NoiseRatio, FractalType, mOctaves, mLacunarity, mGain, mWeightedStrength,
	                      mPingPongStength);

	//Spawn chunks
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
				FBoxBase ChunkBox{
					ChunkPosition + ChunkOffset, FVector(ChunkSize * 1.2, ChunkSize * 1.2, ChunkSize * 1.2),
					FQuat::Identity, TArray<ANavigationNode*>()
				};

				//Pass all level features to the chunk - Marching Cubes will handle the rest
				for (FBoxBase Object : AllObjects)
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
						Chunk->Boxes = RoomBoxes;
						Chunk->Tunnels = Tunnels;
						Chunk->Material = Material;

						Chunk->ChunkPosition = ChunkPosition;
						Chunk->ChunkSize = ChunkSize;
						Chunk->VoxelsPerSide = VoxelDensity;
						Chunk->NoiseRatio = NoiseBlendRatio;
						Chunk->NoiseParams = &NoiseParams;

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

	//At 1.0 it will be Paths - 1 interconnects per box on the path
	const int MaxInterconnects = (Paths.Num() - 1) * Paths[0].Path.Num() * PathInterconnectedness;
	TArray<TArray<int>> InterconnectIndices;

	//Shuffle the "box number in path" so that the interconnects are random
	for (int i = 0; i < Paths.Num() - 1; i++)
	{
		InterconnectIndices.Add(TArray<int>());
		for (int j = 0; j < Paths[0].Path.Num(); j++)
		{
			InterconnectIndices[i].Add(j);
		}
		Algo::RandomShuffle(InterconnectIndices[i]);
	}

	//Create tunnels between boxes on different paths with the same indices
	int NumInterconnects = 0;
	int AttemptNum = 0;
	while (AttemptNum < Paths[0].Path.Num() && NumInterconnects < MaxInterconnects)
	{
		//For each box on the path, interconnect based on the shuffled indices
		for (int i = 0; i < Paths.Num() - 1; i++)
		{
			FLevelBox& BoxA = Paths[i].Path[InterconnectIndices[i][AttemptNum]];
			FLevelBox& BoxB = Paths[i + 1].Path[InterconnectIndices[i][AttemptNum]];

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
		/*APointLight* Light = World->SpawnActor<APointLight>(Box.Position, FRotator::ZeroRotator);
		Light->PointLightComponent->AttenuationRadius = 1500.0f;
		Light->PointLightComponent->SetAttenuationRadius(2000.0f);
		Light->SetBrightness(400.0f);
		Light->PointLightComponent->bUseTemperature = 1.0;
		Light->PointLightComponent->SetTemperature(2500);
		Light->PointLightComponent->SetCastVolumetricShadow(true);*/

		// Log box type
		FVector SpawnPos = FVector(Box.Position.X, Box.Position.Y, Box.Position.Z - 0.5 * Box.Size.Z + 100.0f);

		int rngDirection = FMath::RandRange(0, 3);
		FVector SpawnOffset = FVector(0.0f, 0.0f, 0.0f);
		//switch case on RNG direction to spawn torch +X, +Y, -x, -Y
		switch (rngDirection)
		{
		case 0:
			SpawnOffset.X += Box.Size.X / 2 - 30.0f;
			SpawnOffset.Y += Box.Size.Y / 2 - 30.0f;
			break;
		case 1:
			SpawnOffset.X += Box.Size.X / 2 - 30.0f;
			SpawnOffset.Y -= Box.Size.Y / 2 - 30.0f;
			break;
		case 2:
			SpawnOffset.X -= Box.Size.X / 2 - 30.0f;
			SpawnOffset.Y += Box.Size.Y / 2 - 30.0f;
			break;
		case 3:
			SpawnOffset.X -= Box.Size.X / 2 - 30.0f;
			SpawnOffset.Y -= Box.Size.Y / 2 - 30.0f;
			break;
		default:
			break;
		}
		SpawnPos += SpawnOffset;

		//The torch is vertical, angle it towards the center of the room
		FRotator SpawnRot = FRotator(0.0f, 0.0f, 0.0f);
		SpawnRot.Yaw = FMath::Atan2(-SpawnOffset.Y, -SpawnOffset.X) * 180.0f / PI;

		//Now that it points towards the center of the room, tilt it 45degrees in that direction
		SpawnRot.Pitch = -45.0f;

		Box.Torch = World->SpawnActor<ATorchPickup>(TorchBP, SpawnPos, SpawnRot);

		//Set torch lit based on random boolean (50%)
		if (Box.Type == EBoxType::Normal)
		{
			bool bIsLit = true;
			float LitPerc = FMath::FRandRange(0.0f, 1.0f);
			if (LitPerc < 0.25f)
			{
				bIsLit = false;
			}
			Box.Torch->SetTorchLit(bIsLit);

			//UE_LOG(LogTemp, Warning, TEXT("Torch Lit is %s"), Box.Torch->bIsLit ? TEXT("True") : TEXT("False"));
		}
		// If start room or end room set torch lit and spawn artefacts temp
		else
		{
			Box.Torch->SetTorchLit(true);
		}
	}
	if (ANavigationNode* RoomNode = GetWorld()->SpawnActor<ANavigationNode>(
		ANavigationNode::StaticClass(), Box.Position, FRotator::ZeroRotator))
	{
		Box.RoomNode = RoomNode;
		RoomNode->IsWalkable = false;
		RoomNodes.Add(Box.RoomNode);
		RoomNode->AttachToActor(this, FAttachmentTransformRules::KeepRelativeTransform);
	}

	GenerateWalkableNodes(Box);
}

void AProceduralCaveGen::GenerateWalkableNodes(FBoxBase& Box)
{
	//These will go to the header
	float ShrinkAmount = 80.0f;
	float NodeDensity = 70.0f;

	//Create a rect that has the same size as the box x/y and is rotated to match
	FVector2D RectSize = FVector2D(Box.Size.X - ShrinkAmount, Box.Size.Y - ShrinkAmount);
	FVector2d RectSizeInNodes = FVector2d(RectSize.X / NodeDensity, RectSize.Y / NodeDensity);


	//Place the rect centered on the box
	FVector RectCenter = FVector(Box.Position.X, Box.Position.Y, Box.Position.Z - Box.Size.Z / 2.0f + ShrinkAmount);
	
	//DrawDebugBox(GetWorld(), RectCenter, size3d, Box.Rotation, FColor::White, false, 10.0f);

	int RectY = StaticCast<int>(RectSizeInNodes.Y);
	int RectX = StaticCast<int>(RectSizeInNodes.X);

	//Add NavNodes
	for (int Y = 0; Y < RectY; Y++)
	{
		for (int X = 0; X < RectX; X++)
		{
			// Calculate local position within the rectangle
			FVector NodePositionLocal = FVector(X * NodeDensity + ShrinkAmount, Y * NodeDensity + ShrinkAmount, 0.0f) - FVector(
				RectSize.X  / 2.0f, RectSize.Y  / 2.0f, 0.0f);

			// Convert local position to world space, taking into account the rotation and the true center of the box
			FVector NodePositionWorld = Box.Rotation.RotateVector(NodePositionLocal) + RectCenter;


			//Spawn a NavNode at that position
			if (ANavigationNode* NavNode = GetWorld()->SpawnActor<ANavigationNode>(
				ANavigationNode::StaticClass(), NodePositionWorld, FRotator::ZeroRotator))
			{
				Box.WalkNodes.Add(NavNode);
				WalkNodes.Add(NavNode);
				NavNode->AttachToActor(this, FAttachmentTransformRules::KeepRelativeTransform);
			}
		}
	}

	for (int Y = 0; Y < RectY - 1; Y++)
	{
		for (int X = 0; X < RectX - 1; X++)
		{
			// Define the indices of the four vertices of the current quad
			int32 BottomRight = X + Y * RectX;
			int32 BottomLeft = (X + 1) + Y * RectX;
			int32 TopRight = X + (Y + 1) * RectX;
			int32 TopLeft = (X + 1) + (Y + 1) * RectX;

			//Define the 4 NavNode vertices
			ANavigationNode* BottomRightNode = Box.WalkNodes[BottomRight];
			ANavigationNode* BottomLeftNode = Box.WalkNodes[BottomLeft];
			ANavigationNode* TopRightNode = Box.WalkNodes[TopRight];
			ANavigationNode* TopLeftNode = Box.WalkNodes[TopLeft];

			TArray<ANavigationNode*> QuadNodes = {BottomRightNode, BottomLeftNode, TopRightNode, TopLeftNode};

			for (ANavigationNode* Node : QuadNodes)
			{
				for (ANavigationNode* OtherNode : QuadNodes)
				{
					if (Node != OtherNode && !Node->GetConnectedNodes().Contains(OtherNode))
					{
						Node->SetConnectedNodes(OtherNode);
					}
				}
			}
		}
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


	//Update direction now that we offset the start/end points
	Direction = (End - Start).GetSafeNormal();

	Tunnel.Position = (Start + End) / 2;
	Tunnel.Size = FVector((End - Start).Size() + TunnelSize, TunnelSize, TunnelSize);
	Tunnel.Rotation = FQuat::FindBetweenNormals(FVector::ForwardVector, Direction);

	Tunnels.Add(Tunnel);

	GenerateWalkableNodes(Tunnel);

	//Generate NavNodes at start and end of tunnel
	ANavigationNode* StartNode = GetWorld()->SpawnActor<ANavigationNode>(
		ANavigationNode::StaticClass(), Start, FRotator::ZeroRotator);
	ANavigationNode* EndNode = GetWorld()->SpawnActor<ANavigationNode>(
		ANavigationNode::StaticClass(), End, FRotator::ZeroRotator);

	MeshRoomNodes(StartNode, StartBox);
	MeshRoomNodes(EndNode, TargetBox);

	InsertNode(EndNode, Tunnel);
	InsertNode(StartNode, Tunnel);

	if (ANavigationNode* TunnelNode = GetWorld()->SpawnActor<ANavigationNode>(
		ANavigationNode::StaticClass(), Tunnel.Position, FRotator::ZeroRotator))
	{
		Tunnel.TunnelNode = TunnelNode;
		TunnelNode->IsWalkable = false;
		RoomNodes.Add(TunnelNode);
		TunnelNode->AttachToActor(this, FAttachmentTransformRules::KeepRelativeTransform);
	}

	//Unrolled loop to connect both room nodes to tunnel node
	if (!StartBox.RoomNode->GetConnectedNodes().Contains(Tunnel.TunnelNode))
	{
		StartBox.RoomNode->SetConnectedNodes(Tunnel.TunnelNode);
	}
	if (!TargetBox.RoomNode->GetConnectedNodes().Contains(Tunnel.TunnelNode))
	{
		TargetBox.RoomNode->SetConnectedNodes(Tunnel.TunnelNode);
	}
	if (!Tunnel.TunnelNode->GetConnectedNodes().Contains(StartBox.RoomNode))
	{
		Tunnel.TunnelNode->SetConnectedNodes(StartBox.RoomNode);
	}
	if (!Tunnel.TunnelNode->GetConnectedNodes().Contains(TargetBox.RoomNode))
	{
		Tunnel.TunnelNode->SetConnectedNodes(TargetBox.RoomNode);
	}
}

void AProceduralCaveGen::InsertNode(ANavigationNode* Node, FBoxBase& Box)
{
	ANavigationNode* ClosestNode = nullptr;
	float ClosestDistance = MAX_FLT;
	for (ANavigationNode* BoxNode : Box.WalkNodes)
	{
		float Distance = FVector::Distance(BoxNode->GetActorLocation(), Node->GetActorLocation());
		if (Distance < ClosestDistance)
		{
			ClosestDistance = Distance;
			ClosestNode = BoxNode;
		}
	}
	Node->SetConnectedNodes(ClosestNode);
	ClosestNode->SetConnectedNodes(Node);

	Box.WalkNodes.Add(Node);
	WalkNodes.Add(Node);
}

void AProceduralCaveGen::MeshRoomNodes(ANavigationNode* JoiningNode, FLevelBox& Box)
{
	ANavigationNode* ClosestNode = nullptr;
	float ClosestDistance = MAX_FLT;
	for (ANavigationNode* BoxNode : Box.WalkNodes)
	{
		float Distance = FVector::Distance(BoxNode->GetActorLocation(), JoiningNode->GetActorLocation());
		if (Distance < ClosestDistance)
		{
			ClosestDistance = Distance;
			ClosestNode = BoxNode;
		}
	}
	JoiningNode->SetConnectedNodes(ClosestNode);
	ClosestNode->SetConnectedNodes(JoiningNode);
}

/**
 * @brief Adds all rooms, tunnels to a list of one type of object - AllObjects
 */
void AProceduralCaveGen::AddAllObjects()
{
	AllObjects.Append(RoomBoxes);

	//ToDo Overlap Checks for rotated boxes
	for (FTunnel Tunnel : Tunnels)
	{
		AllObjects.Add(Tunnel);
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
	RoomBoxes.Empty();
	Tunnels.Empty();
	AllObjects.Empty();
	RoomNodes.Empty();
	WalkNodes.Empty();
	Artefacts.Empty();
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

	for (TActorIterator<ATorchPickup> It(GetWorld()); It; ++It)
	{
		if (*It)
		{
			(*It)->Destroy();
		}
	}

	for (TActorIterator<AArtefactPickup> It(GetWorld()); It; ++It)
	{
		if (*It)
		{
			(*It)->Destroy();
		}
	}
	for (TActorIterator<APedestalInteract> It(GetWorld()); It; ++It)
	{
		if (*It)
		{
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

		APlayerStart* NewPlayer2Start = GetWorld()->SpawnActor<APlayerStart>(
			APlayerStart::StaticClass(), FVector(Location.X, Location.Y + 100.0f, Location.Z), FRotator(0, 0, 0),
			SpawnParams);

		if (NewPlayerStart)
		{
			// Optionally, you can set any other properties on NewPlayerStart
		}
	}
}

void AProceduralCaveGen::AttachTorchToWalls()
{
	for (FLevelBox& Box : RoomBoxes)
	{
		if (Box.Torch)
		{
			FHitResult HitResult;
			const FVector StartLocation = Box.Torch->GetActorLocation();

			//Fire direction to be random direction in XY plane
			const FVector FireDirection = (FVector(FMath::RandRange(-1.0f, 1.0f), FMath::RandRange(-1.0f, 1.0f),
			                                       0.0f)).GetSafeNormal();

			//End location is max of box size X/Y/Z away in direction
			const FVector EndLocation = StartLocation + (FireDirection * Box.Size.GetMax());

			//Log trace length
			//UE_LOG(LogTemp, Warning, TEXT("Trace Length is %f"), Box.Size.GetMax());

			FCollisionQueryParams QueryParams;
			//Log trace and owner
			//UE_LOG(LogTemp, Warning, TEXT("Tracing, Owner is %s"), *GetOwner()->GetName());
			QueryParams.AddIgnoredActor(Box.Torch);

			GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECC_WorldStatic, QueryParams);

			UE_LOG(LogTemp, Display, TEXT("Trace for torch"));
			DrawDebugLine(GetWorld(), StartLocation, EndLocation, FColor::Red, false, 1.0f, 0, 1.0f);
			if (AActor* HitActor = HitResult.GetActor())
			{
				//Show debug of object type hit
				UE_LOG(LogTemp, Warning, TEXT("Hit Actor is %s"), *HitActor->GetName());
				UE_LOG(LogTemp, Warning, TEXT("Hit Actor is %s"), *HitActor->GetClass()->GetName());
			}
		}
	}
}

/**
 * @brief Draw Boxes and Tunnel widgets
 */
void AProceduralCaveGen::DebugShow()
{
	for (const FLevelBox& box : RoomBoxes)
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
			RoomNode->DebugSetVisibility(bDebugNavNodes);
		}
	}
	if (!WalkNodes.IsEmpty())
	{
		for (ANavigationNode* WalkNode : WalkNodes)
		{
			WalkNode->DebugSetVisibility(bDebugNavNodes);
		}
	}
}


//Box intersection functions follow below - I may switch to UE's functions but these are pretty fast
//They check if box intersects box incl rotation over XYZ. 
TArray<FVector> CalculateBoxCorners(const FBoxBase& Box)
{
	TArray<FVector> Corners;

	// Half sizes for each dimension
	FVector HalfSize = Box.Size * StaticCast<double>(0.5f);

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

TArray<FVector> FindAxesToTest(const FBoxBase& BoxA, const FBoxBase& BoxB)
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
bool AProceduralCaveGen::BoxesIntersect(const FBoxBase& BoxA, const FBoxBase& BoxB)
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
