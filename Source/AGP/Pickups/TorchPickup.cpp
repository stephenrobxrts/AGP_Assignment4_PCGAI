// Fill out your copyright notice in the Description page of Project Settings.


#include "TorchPickup.h"
#include "AGP/Characters/PlayerCharacter.h"


// Sets default values
ATorchPickup::ATorchPickup()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

bool ATorchPickup::ShouldTickIfViewportsOnly() const
{
	return true;
}

void ATorchPickup::SetTorchLit(bool bLit)
{
	bIsLit = bLit;
}

// Called when the game starts or when spawned
void ATorchPickup::BeginPlay()
{
	Super::BeginPlay();
	
}

void ATorchPickup::OnPickupOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (ABaseCharacter* BaseCharacter = Cast<ABaseCharacter>(OtherActor))
	{
		//If this is a child actor, don't do anything
		if (this->IsChildActor())
		{
			return;
		}
		
		if (!BaseCharacter->HasTorch() && !BaseCharacter->bEquipTorchAction)
		{
			BaseCharacter->EquipTorch(true, bIsLit);
			this->Destroy();
		}
	}
}

// Called every frame
void ATorchPickup::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

