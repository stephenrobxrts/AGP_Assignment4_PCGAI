// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "../Pathfinding/NavigationNode.h"
#include "GameFramework/Actor.h"
#include "ProceduralBoxes.generated.h"

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
struct FBoxNode
{
	GENERATED_BODY()
	
public:
	FLevelBox* Box;
	FBoxNode* Parent;
	float GScore;
	float HScore;
	float FScore;

	TArray<FBoxNode*> ConnectedNodes; // This will have immediate neighboring boxes
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
class AGP_API AProceduralBoxes : public AActor
{
	GENERATED_BODY()

public:
	AProceduralBoxes();
	// Sets default values for this actor's properties
	virtual bool ShouldTickIfViewportsOnly() const override;

	

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere)
	bool bShouldRegenerate = false;
	
	UPROPERTY(EditAnywhere)
		TArray<FLevelBox> Boxes;
	UPROPERTY(EditAnywhere)
		TArray<FTunnel> Tunnels;

	UPROPERTY(VisibleAnywhere)
		TArray<FLevelBox> Path1;
	UPROPERTY(VisibleAnywhere)
		TArray<FLevelBox> Path2;
	


	UPROPERTY(EditAnywhere)
		int NumBoxes = 10;
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
	

	TArray<FLevelBox> GenerateGuaranteedPathBoxes(int NumBoxesToGenerate, FVector BoxMinSize, FVector BoxMaxSize);
	bool PositionValidForSecondPath(const FLevelBox& NewBox, const TArray<FLevelBox>& FirstPathBoxes);

	void PopulateConnectedNodes(TArray<FBoxNode>& AllNodes);
	bool CanConnect(FLevelBox* BoxA, FLevelBox* BoxB);
	TArray<FVector> GetPath(FBoxNode* StartNode, FBoxNode* EndNode, const TSet<FBoxNode*>& AvoidNodes);
	
	TArray<FTunnel> GenerateTunnels(TArray<FLevelBox> InputBoxes);
	void CreateTunnel(const FLevelBox& StartBox, const FLevelBox& EndBox);

	void DebugBox(FLevelBox& inputBox, FColor Color);

	TArray<TArray<bool>> ConnectionMatrix;
	int CountConnections(int BoxIndex, const TArray<TArray<bool>>& Connections);
	bool BoxesIntersect(const FLevelBox& BoxA, const FLevelBox& BoxB);

	
public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
