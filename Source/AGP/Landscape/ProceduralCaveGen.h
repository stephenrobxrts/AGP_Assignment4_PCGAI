// Fill out your copyright notice in the Description page of Project Settings.

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
 * @brief Level box struct contains Position, Size, Type (Start, Normal, End)
 */
USTRUCT(BlueprintType)
struct FLevelBox
{
	GENERATED_BODY()

public:
	FVector Position;
	FVector Size;
	EBoxType Type;
};

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
};

USTRUCT()
struct FInnerArray
{
	GENERATED_BODY()
	UPROPERTY()
	TArray<FLevelBox> Path;
};


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

	UPROPERTY(EditAnywhere)
	bool bShouldRegenerate = true;
	UPROPERTY(EditAnywhere)
	bool bUpdateMesh = true;

	UPROPERTY(EditAnywhere)
	int NumPaths = 2;
	UPROPERTY(EditAnywhere)
	int NumBoxesPerPath = 10;
	UPROPERTY(EditAnywhere)
	float LevelSize = 10000.0f;
	UPROPERTY(EditAnywhere)
	float HeightDifference = 400.0f;
	UPROPERTY(EditAnywhere)
	float MaxConnectionDistance = 2000.0f;
	UPROPERTY(EditAnywhere)
	FVector MinSize = FVector(300.0f, 300.0f, 200.0f);
	UPROPERTY(EditAnywhere)
	FVector MaxSize = FVector(1000.0f, 1000.0f, 600.0f);
	UPROPERTY(EditAnywhere)
	float TunnelSize = 200.0f;
	
	UPROPERTY(EditAnywhere)
	float Connectedness = 0.7f;
	
	
	UPROPERTY(EditAnywhere)
	TSubclassOf<class AMarchingChunkTerrain> Marcher;
	TArray<AMarchingChunkTerrain*> Chunks;
	UPROPERTY(EditInstanceOnly, Category="Chunk")
	TObjectPtr<UMaterialInterface> Material;

	//Debug
	UPROPERTY(EditAnywhere)
	bool bDebugView = true;
	UPROPERTY(EditAnywhere)
	bool bDebugOnly1Chunk = true;
	UPROPERTY(VisibleAnywhere, Category="Level Layout")
	TArray<FLevelBox> Boxes;
	UPROPERTY(VisibleAnywhere, Category="Level Layout")
	TArray<FTunnel> Tunnels;
	
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
	bool DebugChunk = true;
	UPROPERTY(EditAnywhere, Category="Chunk")
	bool DebugVoxels = false;
	UPROPERTY(EditAnywhere, Category="Chunk")
	bool bDebugInvertSolids = true;

	UPROPERTY(EditAnywhere, Category="Noise")
	float NoiseRatio = 0.8f;
	

	TArray<FLevelBox> GenerateGuaranteedPathBoxes(int NumBoxesToGenerate, FVector BoxMinSize, FVector BoxMaxSize);
	void GenerateInterconnects();
	void CreateBox(const FVector& Position, const FVector& Size, EBoxType Type);

	FVector CalculateBoxOffset(const FLevelBox& Box, const FVector& Direction);
	void CreateTunnel(const FLevelBox& StartBox, const FLevelBox& TargetBox);
	bool BoxPositionValid(const FLevelBox& NewBox, const TArray<FLevelBox>& Boxes);
	bool BoxesIntersect2D(const FLevelBox& BoxA, const FLevelBox& BoxB);
	bool BoxesIntersect(const FLevelBox& BoxA, const FLevelBox& BoxB);
	void ClearMap();
	void DebugShow();
	void AddPlayerStartAtLocation(const FVector& Location); 

	void GenerateMesh();

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
