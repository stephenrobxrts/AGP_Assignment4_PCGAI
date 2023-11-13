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
}

void ATorchPickup::AttemptPickUp(ABaseCharacter* BaseCharacter)
{
	if (HasAuthority())
	{
		OnPickedUp(BaseCharacter);
	}
	/*else
	{
		ServerAttemptPickup(BaseCharacter);
	}*/

	/*UE_LOG(LogTemp, Warning, TEXT("TorchPickup: %s"), *BaseCharacter->GetActorLabel());
	BaseCharacter->EquipTorch(true, bIsLit);*/
	
}

void ATorchPickup::ServerAttemptPickup_Implementation(ABaseCharacter* BaseCharacter)
{
	if (bIsHeld)
	{
		return;
	}
	OnPickedUp(BaseCharacter);
}

bool ATorchPickup::ServerAttemptPickup_Validate(ABaseCharacter* BaseCharacter)
{
	return true;
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

