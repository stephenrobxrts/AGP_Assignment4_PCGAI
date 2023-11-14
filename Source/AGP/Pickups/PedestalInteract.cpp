// Fill out your copyright notice in the Description page of Project Settings.


#include "PedestalInteract.h"

#include "Net/UnrealNetwork.h"


// Sets default values
APedestalInteract::APedestalInteract()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;  // Enable replication for this actor
	bAlwaysRelevant = true;  // Make sure it's always sent to clients
}

// Called when the game starts or when spawned
void APedestalInteract::BeginPlay()
{
	Super::BeginPlay();
	ArtefactsPlaced = {false, false, false, false};
	
}

void APedestalInteract::PlaceArtefacts(TArray<bool> NewArtefacts)
{
	if (HasAuthority())
	{
		// If the artefact is not placed, place it
		for (int i = 0 ; i < NewArtefacts.Num() ; i++)
		{
			if (ArtefactsPlaced[i] == false)
			{
				ArtefactsPlaced[i] = NewArtefacts[i];
			}
		}
		UpdateVisibility();
	}
}


//TODO Implement as part of update visibility
bool APedestalInteract::HasAllArtefacts()
{
	//If artefactsPlaced is all trues, return true
	for (bool ArtefactPlaced : ArtefactsPlaced)
	{
		if (!ArtefactPlaced)
		{
			return false;
		}
	}
	return true;
}

TArray<bool> APedestalInteract::GetArtefactsPlaced()
{
	return ArtefactsPlaced;
}

void APedestalInteract::OnRep_UpdateArtefacts()
{
	UpdateVisibility();
}


void APedestalInteract::OnPickupOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	return;
}

// Called every frame
void APedestalInteract::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void APedestalInteract::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(APedestalInteract, ArtefactsPlaced);
	DOREPLIFETIME(APedestalInteract, bHasAllArtefacts);
}

