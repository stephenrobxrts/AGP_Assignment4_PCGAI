// Fill out your copyright notice in the Description page of Project Settings.


#include "TorchPickup.h"
#include "AGP/Characters/PlayerCharacter.h"
#include "Net/UnrealNetwork.h"


// Sets default values
ATorchPickup::ATorchPickup()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;  // Enable replication for this actor
	bAlwaysRelevant = true;  // Make sure it's always sent to clients

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

	if (bIsHeld)
	{
		//UELOG this actor's Collision Stats
		UE_LOG(LogTemp, Warning, TEXT("TorchPickup: %s"), *this->GetName());
		//Set collision to ignore all
	}

	
}

void ATorchPickup::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ATorchPickup, bIsLit);
	DOREPLIFETIME(ATorchPickup, bIsHeld);
}

bool ATorchPickup::GetIsHeld()
{
	return bIsHeld;
}

void ATorchPickup::AttemptPickUp(ABaseCharacter* BaseCharacter)
{
	if (HasAuthority())
	{
		OnPickedUp(BaseCharacter);
	}
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
	return;
}


