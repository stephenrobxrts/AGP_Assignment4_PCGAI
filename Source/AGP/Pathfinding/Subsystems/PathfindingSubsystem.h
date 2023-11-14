// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "../NavigationNode.h"
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
	TArray<FVector> GetRandomPath(const FVector& StartLocation, const TArray<ANavigationNode*>& NodeArray);

	TArray<FVector> GetPath(const FVector& StartLocation, const FVector& TargetLocation, const TArray<ANavigationNode*>& NodeArray);
	TArray<FVector> GetPath(ANavigationNode* StartNode, ANavigationNode* EndNode, const TArray<ANavigationNode*>& NodeArray);
	TArray<FVector> GetPathAway(const FVector& StartLocation, const FVector& TargetLocation, const TArray<ANavigationNode*>& NodeArray);

	TArray<FVector> GetWaypointPositions();

	float GetPathLength(const FVector& StartLocation, const FVector& TargetLocation, const TArray<ANavigationNode*>& NodeArray);
	ANavigationNode* FindNearestNode(const FVector& TargetLocation, const TArray<ANavigationNode*>& NodeArray);

	TArray<ANavigationNode*>& GetWalkableNodes();
	TArray<ANavigationNode*>& GetRoomNodes();

protected:
	UPROPERTY(EditAnywhere)
	TArray<ANavigationNode*> WalkableNodes;

	UPROPERTY(VisibleAnywhere)
	TArray<ANavigationNode*> RoomNodes;

private:
	void PopulateNodes();
	ANavigationNode* GetRandomNode(const TArray<ANavigationNode*>& NodeArray);
	ANavigationNode* FindFurthestNode(const FVector& TargetLocation, const TArray<ANavigationNode*>& NodeArray);


	TArray<FVector> ReconstructPath(const ANavigationNode* StartNode, ANavigationNode* EndNode);
};
