// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PickupBase.generated.h"

// This is a forward declaration. It tells this classes header file that a class called UBoxComponent exists without
// needing to use a #include directive.
// We could #include "Components/BoxComponent.h" but it is preferred to forward declare then #include in the .cpp file.
// For this subject either is acceptable but using #include less in headers improves compile time.
class UBoxComponent;

UCLASS()
class AGP_API APickupBase : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APickupBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* PickupMesh;
	UPROPERTY(VisibleAnywhere)
	UBoxComponent* PickupCollider;

    UFUNCTION()
	virtual void OnPickupOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
