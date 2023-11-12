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

void ATorchPickup::OnPickedUp(ABaseCharacter* BaseCharacter)
{
	if (bIsHeld)
	{
		return;
	}
	BaseCharacter->EquipTorch(true, bIsLit);
	this->Destroy();
}

void ATorchPickup::OnInteract()
{
	if (bIsHeld)
	{
		return;
	}
	bIsLit ? bIsLit = false : bIsLit = true;
}


void ATorchPickup::OnPickupOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                   UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{

	/*if (ABaseCharacter* BaseCharacter = Cast<ABaseCharacter>(OtherActor))
	{
		//If this is a child actor, don't do anything
		if (this->IsChildActor())
		{
			return;
		}
		
		if (!BaseCharacter->HasTorch())
		{
			BaseCharacter->SetIsOverlappingPickup(true);
			BaseCharacter->EquipTorch(true, bIsLit);
			this->Destroy();
		}

		/*if (BaseCharacter->ToggleTorchAction)
		{
			bIsLit = false;
		}#1#
	}*/
}

void ATorchPickup::OnProximityExit(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	
}

// Called every frame
void ATorchPickup::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

