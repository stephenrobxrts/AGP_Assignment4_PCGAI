// Fill out your copyright notice in the Description page of Project Settings.


#include "ArtefactPickup.h"
#include "AGP/Characters/PlayerCharacter.h"
#include "Net/UnrealNetwork.h"

AArtefactPickup::AArtefactPickup()
{

	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}


int AArtefactPickup::GetArtefactID()
{
	return ArtefactID;
}

void AArtefactPickup::SetArtefactID(int ID)
{
	ArtefactID = ID;
}

void AArtefactPickup::SetIsPickup(bool bPickup)
{
	bIsPickup = bPickup;
}

void AArtefactPickup::DestroyArtefact()
{
	ServerDestroyArtefact(this);
}

void AArtefactPickup::AttemptPickUp(ABaseCharacter* BaseCharacter)
{
	if (HasAuthority() && bIsPickup)
	{
		OnPickedUp(BaseCharacter);
	}
}


void AArtefactPickup::OnPickedUp(ABaseCharacter* BaseCharacter)
{
	BaseCharacter->PickupArtefact(ArtefactID);
	//Log owner
	UE_LOG(LogTemp, Warning, TEXT("ArtefactPickup: %s"), *this->GetName());
	this->Destroy();
	ServerDestroyArtefact(this);
}

void AArtefactPickup::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AArtefactPickup,ArtefactID);
}


void AArtefactPickup::OnPickupOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                      UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
}

void AArtefactPickup::ServerDestroyArtefact_Implementation(AArtefactPickup* ArtefactPickup)
{
	ArtefactPickup->Destroy();
}

