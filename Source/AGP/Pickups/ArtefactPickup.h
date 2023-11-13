// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PickupBase.h"
#include "ArtefactPickup.generated.h"


class ABaseCharacter;
/**
 * 
 */
UCLASS()
class AGP_API AArtefactPickup : public APickupBase
{
	GENERATED_BODY()

public:
	AArtefactPickup();

	void AttemptPickUp(ABaseCharacter* BaseCharacter);
	

protected:

	virtual void OnPickupOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
						 UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep,
						 const FHitResult& SweepResult) override;


	void OnPickedUp(ABaseCharacter* BaseCharacter);
	
};
