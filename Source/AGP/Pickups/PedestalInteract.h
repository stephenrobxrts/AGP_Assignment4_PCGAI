// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PickupBase.h"
#include "../Characters/BaseCharacter.h"

#include "PedestalInteract.generated.h"

UCLASS()
class AGP_API APedestalInteract : public APickupBase
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	APedestalInteract();

	void PlaceArtefacts(TArray<bool> NewArtefacts);
	void AttemptPickUp(ABaseCharacter* BaseCharacter);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable)
	bool HasAllArtefacts();

	UFUNCTION()
	void OnRep_UpdateArtefacts();
	
	UPROPERTY(ReplicatedUsing=OnRep_UpdateArtefacts, EditAnywhere, BlueprintReadWrite)
	TArray<bool> ArtefactsPlaced = {false, false, false, false};

	
	UPROPERTY(ReplicatedUsing=OnRep_UpdateArtefacts, EditAnywhere, BlueprintReadWrite)
	bool bHasAllArtefacts = false;
	
	UFUNCTION(BlueprintCallable)
	TArray<bool> GetArtefactsPlaced();
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsHeld = false;
	
	UFUNCTION(BlueprintImplementableEvent)
	void UpdateVisibility();

	UFUNCTION(BlueprintImplementableEvent)
	void GrabSkull();

	TSubclassOf<APickupBase>* PickupActor;


	virtual void OnPickupOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
						 UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep,
						 const FHitResult& SweepResult) override;
	

	
public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
	
};
