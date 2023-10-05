// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "../Pathfinding/NavigationNode.h"
#include "GameFramework/Actor.h"
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
	bool bShouldRegenerate = false;
	UPROPERTY(EditAnywhere)
	bool bUpdateMesh = false;

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
		TSubclassOf<class AMarchingChunkTerrain> Marcher;
	AMarchingChunkTerrain* MarcherInstance;
	
	UPROPERTY(VisibleAnywhere, Category="Level Layout")
	TArray<FLevelBox> Boxes;
	UPROPERTY(VisibleAnywhere, Category="Level Layout")
	TArray<FTunnel> Tunnels;

	UPROPERTY(VisibleAnywhere, Category="Level Layout")
	TArray<FLevelBox> Path1;
	UPROPERTY(VisibleAnywhere, Category="Level Layout")
	TArray<FLevelBox> Path2;
	

	TArray<FLevelBox> GenerateGuaranteedPathBoxes(int NumBoxesToGenerate, FVector BoxMinSize, FVector BoxMaxSize);
	bool PositionValidForSecondPath(const FLevelBox& NewBox, const TArray<FLevelBox>& FirstPathBoxes);
	void CreateTunnel(const FLevelBox& StartBox, const FLevelBox& EndBox);
	bool BoxesIntersect(const FLevelBox& BoxA, const FLevelBox& BoxB);

	void GenerateMesh();

	
public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
