﻿// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "../Pathfinding/NavigationNode.h"
#include "GameFramework/Actor.h"
#include "GameFramework/PlayerStart.h"
#include "ProceduralCaveGen.generated.h"

/**
 * @brief Box type enum
 */
UENUM(BlueprintType)
enum class EBoxType : uint8
{
	Start,
	Normal,
	End
};

/**
 * @brief Level box struct contains Position, Size, Type (Start, Normal, End), Rotation (optional)
 */
USTRUCT(BlueprintType)
struct FLevelBox
{
	GENERATED_BODY()

public:
	FVector Position;
	FVector Size;
	EBoxType Type;
	FQuat Rotation = FQuat::Identity;
	UPROPERTY(EditInstanceOnly)
	ANavigationNode* RoomNode = nullptr;
};

/**
 * @brief Tunnel Strut behaves like a level box but also includes a start and end box
 */
USTRUCT(BlueprintType)
struct FTunnel
{
	GENERATED_BODY()

public:
	const FLevelBox* StartBox;
	const FLevelBox* EndBox;
	FVector Position;
	FVector Size;
	FQuat Rotation = FQuat::Identity;
	//UPROPERTY(EditInstanceOnly)
	//ANavigationNode* TunnelNode = nullptr;
};

/**
 * @brief This is just an Array of Arrays because apparently we can't do that :(
 */
USTRUCT()
struct FInnerArray
{
	GENERATED_BODY()
	UPROPERTY()
	TArray<FLevelBox> Path;
};

/**
 * @brief Procedurally create boxes and tunnels, pass to MarchingChunkTerrain to create mesh
 */
UCLASS()
class AGP_API AProceduralCaveGen : public AActor
{
	GENERATED_BODY()

public:
	AProceduralCaveGen();
	// Sets default values for this actor's properties
	virtual bool ShouldTickIfViewportsOnly() const override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;


	//Box placement logic
	TArray<FLevelBox> GenerateGuaranteedPathBoxes();
	void GenerateInterconnects();
	void CreateBox(FLevelBox& Box);

	//Tunnel Creation
	FVector CalculateBoxOffset(const FLevelBox& Box, const FVector& Direction) const;
	void CreateTunnel(FLevelBox& StartBox, FLevelBox& TargetBox);

	//Create array of all objects for chunks
	void AddAllObjects();

	//Creation Logic
	void ClearMap();
	void GenerateMesh();


	//Helper Functions
	bool BoxPositionValid(const FLevelBox& NewBox, const TArray<FLevelBox>& Boxes);
	bool BoxesIntersect2D(const FLevelBox& BoxA, const FLevelBox& BoxB);
	static bool BoxesIntersect(const FLevelBox& BoxA, const FLevelBox& BoxB);
	void DebugShow();
	void DebugShowNavNodes();
	void AddPlayerStartAtLocation(const FVector& Location);


	UPROPERTY(EditAnywhere)
	bool bShouldRegenerate = true;
	UPROPERTY(EditAnywhere)
	bool bUpdateMesh = true;

	UPROPERTY(EditAnywhere)
	int NumPaths = 2;
	UPROPERTY(EditAnywhere)
	int NumBoxesPerPath = 6;
	UPROPERTY(EditAnywhere)
	float LevelSize = 10000.0f;
	UPROPERTY(EditAnywhere)
	float HeightDifference = 400.0f;

	UPROPERTY(VisibleAnywhere)
	FVector MinBoxSize = FVector(300.0f, 300.0f, 200.0f);
	UPROPERTY(EditAnywhere)
	FVector MaxBoxSize = FVector(1000.0f, 1000.0f, 600.0f);
	UPROPERTY(VisibleAnywhere)
	float AvgConnectionDistance = (LevelSize / (NumBoxesPerPath));
	UPROPERTY(EditAnywhere)
	float TunnelSize = 260.0f;

	UPROPERTY(EditAnywhere)
	float PathInterconnectedness = 0.7f;

	UPROPERTY(EditAnywhere)
	TSubclassOf<class AMarchingChunkTerrain> Marcher;
	TArray<AMarchingChunkTerrain*> Chunks;
	UPROPERTY(EditInstanceOnly, Category="Chunk")
	TObjectPtr<UMaterialInterface> Material;

	//Navigation Nodes
	UPROPERTY(EditAnywhere)
	TArray<ANavigationNode*> RoomNodes;

	//Debug Boxes and Tunnels
	UPROPERTY(EditAnywhere)
	bool bDebugView = true;
	UPROPERTY(EditAnywhere)
	bool bDebugNavNodes = true;

	/**
	 * @brief Disable to render the whole level, enable to render only the first section of it
	 */
	UPROPERTY(EditAnywhere)
	bool bDebugOnly1Chunk = true;
	UPROPERTY(VisibleAnywhere, Category="Level Layout")
	TArray<FLevelBox> Boxes;
	UPROPERTY(VisibleAnywhere, Category="Level Layout")
	TArray<FTunnel> Tunnels;
	UPROPERTY()
	TArray<FLevelBox> AllObjects;

	UPROPERTY(VisibleAnywhere, Category="Level Layout")
	TArray<FInnerArray> Paths;

	//NavNodes
	/*TArray<ANavigationNode*> RoomNodes;
	TArray<ANavigationNode*> NavNodes;*/

	//Chunks
	UPROPERTY(EditAnywhere, Category="Chunk")
	int ChunkSize = 1024;
	UPROPERTY(EditAnywhere, Category="Chunk")
	int VoxelDensity = 32;
	UPROPERTY(EditAnywhere, Category="Chunk")
	bool bDebugChunk = true;
	UPROPERTY()
	FVector OffsetChunkStart;

	UPROPERTY(meta=(EditCondition="bSmallNumVoxels"))
	;
	bool bSmallNumVoxels = false;

	UPROPERTY(EditAnywhere, Category="Chunk", meta=(EditConditionHides="bSmallNumVoxels"))
	bool bDebugVoxels = false;
	UPROPERTY(EditAnywhere, Category="Chunk")
	bool bDebugInvertSolids = true;

	UPROPERTY(EditAnywhere, Category="Noise")
	float NoiseRatio = 0.8f;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
