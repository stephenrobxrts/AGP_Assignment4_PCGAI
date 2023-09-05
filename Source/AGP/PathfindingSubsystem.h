// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "NavigationNode.h"
#include "EngineUtils.h"
#include "PathfindingSubsystem.generated.h"

/**
 * 
 */
UCLASS()
class AGP_API UPathfindingSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;
	TArray<FVector> GetRandomPath(FVector& StartLocation);

protected:
	UPROPERTY(EditAnywhere)
	TArray<ANavigationNode*> Nodes;

private:
	void PopulateNodes();
	ANavigationNode* GetRandomNode();
	ANavigationNode* FindNearestNode(FVector& TargetLocation);
	TArray<FVector> GetPath(ANavigationNode* StartNode, ANavigationNode* EndNode);
	TArray<FVector> ReconstructPath(const ANavigationNode* StartNode, ANavigationNode* EndNode);
};
