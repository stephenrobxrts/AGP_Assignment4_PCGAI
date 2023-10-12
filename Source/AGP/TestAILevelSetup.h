// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Actor.h"
#include "Pathfinding/NavigationNode.h"
#include "TestAILevelSetup.generated.h"

UCLASS()
class AGP_API ATestAILevelSetup : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ATestAILevelSetup();
	virtual bool ShouldTickIfViewportsOnly() const override;

	UPROPERTY(VisibleAnywhere)
	TArray<ARoomOverlap*> Rooms;

	//Manually add your room nodes to this array
	UPROPERTY(EditAnywhere)
	TArray<ANavigationNode*> RoomNodes;

	UPROPERTY(EditAnywhere)
	bool bUpdateNavNodes = false;

	void CheckAgainstNavNodes(UBoxComponent* BoxCollider, ARoomOverlap* Room);
	void AssignNavNodes();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void OnPickupOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
								 UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep,
								 const FHitResult& SweepResult);

public:
	void GetRooms();
	void ClearData();
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
