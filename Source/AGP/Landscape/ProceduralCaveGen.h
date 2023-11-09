// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Voxels/VoxelUtils/FastNoiseLite.h" 
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
	TArray<ANavigationNode*> WalkNodes;
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
	TArray<ANavigationNode*> WalkNodes;
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

//UE friendly Fractal typing
UENUM(BlueprintType)
enum class EFractalType : uint8
{
	FractalType_None,
	FractalType_FBm,
	FractalType_Ridged,
	FractalType_PingPong,
	FractalType_DomainWarpProgressive,
	FractalType_DomainWarpIndependent
};

UENUM(BlueprintType)
enum class ENoiseType : uint8
{
	NoiseType_OpenSimplex2,
	NoiseType_OpenSimplex2S,
	NoiseType_Cellular,
	NoiseType_Perlin,
	NoiseType_ValueCubic,
	NoiseType_Value
};

USTRUCT()
struct FNoiseParams
{
	GENERATED_BODY()
	ENoiseType NoiseType = ENoiseType::NoiseType_Perlin;
	float NoiseRatio = 0.8f;
	EFractalType FractalType = EFractalType::FractalType_FBm;
	int mOctaves = 3;
	float mLacunarity = 2.0f;
	float mGain = 0.5f;
	float mWeightedStrength = 0.0f;
	float mPingPongStength = 2.0f;

public:
	void SetParams(ENoiseType _NoiseType, float _NoiseRatio, EFractalType _FractalType, int _mOctaves, float _mLacunarity, float _mGain, float _mWeightedStrength, float _mPingPongStength)
	{
		this->NoiseType = _NoiseType;
		this->NoiseRatio = _NoiseRatio;
		this->FractalType = _FractalType;
		this->mOctaves = _mOctaves;
		this->mLacunarity = _mLacunarity;
		this->mGain = _mGain;
		this->mWeightedStrength = _mWeightedStrength;
		this->mPingPongStength = _mPingPongStength;
	}
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

	//NavigationNodes
	void GenerateWalkableNodes(FLevelBox& Box);


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
	UPROPERTY(EditAnywhere)
	TArray<ANavigationNode*> WalkNodes;

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
	float NoiseBlendRatio = 0.8f;
	FastNoiseLite* Noise = new FastNoiseLite();
	UPROPERTY(EditAnywhere, Category="Noise")
	ENoiseType NoiseType = ENoiseType::NoiseType_Perlin;
	UPROPERTY(EditAnywhere, Category="Noise")
	float NoiseRatio = 0.8f;
	UPROPERTY(EditAnywhere, Category="Noise")
	EFractalType FractalType = EFractalType::FractalType_FBm;
	UPROPERTY(EditAnywhere, Category="Noise")
	int mOctaves = 3;
	UPROPERTY(EditAnywhere, Category="Noise")
	float mLacunarity = 2.0f;
	UPROPERTY(EditAnywhere, Category="Noise")
	float mGain = 0.5f;
	UPROPERTY(EditAnywhere, Category="Noise")
	float mWeightedStrength = 0.0f;
	UPROPERTY(EditAnywhere, Category="Noise")
	float mPingPongStength = 2.0f;

	UPROPERTY()
	FNoiseParams NoiseParams;




public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
