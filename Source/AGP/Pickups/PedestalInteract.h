// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PickupBase.h"
#include "PedestalInteract.generated.h"

UCLASS()
class AGP_API APedestalInteract : public APickupBase
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	APedestalInteract();

	void PlaceArtefacts(TArray<bool> NewArtefacts);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;



	bool HasAllArtefacts();
	
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite)
	TArray<bool> ArtefactsPlaced = {false, false, false, false};

	UFUNCTION(BlueprintCallable)
	TArray<bool> GetArtefactsPlaced();


	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsHeld = false;
	
	UFUNCTION(BlueprintImplementableEvent)
	void UpdateVisibility();

	TSubclassOf<APickupBase>* PickupActor;

	/*UFUNCTION(BlueprintCallable)
	void SetGoldenEgg(bool bCompleteSet);*/
	
public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
