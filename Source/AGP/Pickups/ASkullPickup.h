// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PickupBase.h"
#include "ASkullPickup.generated.h"

class ABaseCharacter;

UCLASS()
class AGP_API AASkullPickup : public APickupBase
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AASkullPickup();

	void AttemptPickup(ABaseCharacter* BaseCharacter);

	UFUNCTION(BlueprintCallable)
	void SetExists(bool bNewExists);

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite)
	bool bExists = false;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
