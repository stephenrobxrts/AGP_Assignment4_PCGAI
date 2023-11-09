// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AGP/Pickups/RoomOverlap.h"
#include "GameFramework/Actor.h"
#include "NavigationNode.generated.h"

UCLASS()
class AGP_API ANavigationNode : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ANavigationNode();
	virtual bool ShouldTickIfViewportsOnly() const override;

	void SetRoom(ARoomOverlap* NewRoom);
	void SetRoomName(FString NewRoomName);
	ARoomOverlap* GetRoom();
	FString GetRoomName();

	
	float GScore;
	float HScore;
	float FScore = 0;
	UPROPERTY()
	ANavigationNode* ParentNode;



	void DebugSetVisibility(const bool bNewVisibility);

	TArray<ANavigationNode*> GetConnectedNodes();
	void SetConnectedNodes(ANavigationNode* NewConnectedNode);
	void DestroyNode();

protected:

	UPROPERTY()
	ARoomOverlap* Room = nullptr;
	UPROPERTY(VisibleAnywhere)
	FString RoomName = "";

	
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	UPROPERTY(EditAnywhere)
	TArray<ANavigationNode*> ConnectedNodes;
	UPROPERTY(VisibleAnywhere)
	USceneComponent* LocationComponent;

	UPROPERTY(EditAnywhere)
	bool bDebugVisible = false;

	/**
	 * @brief Checks if a reciprocal connection exists between two nodes (A,B)
	 * @param NodeA 
	 * @param NodeB 
	 * @return A bool indicating whether reciprocal connection exists between the two nodes
	 */
	bool CheckReciprocalConnection(ANavigationNode* NodeA, ANavigationNode* NodeB);

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
