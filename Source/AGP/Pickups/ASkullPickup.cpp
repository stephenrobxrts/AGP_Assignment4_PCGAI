// Fill out your copyright notice in the Description page of Project Settings.


#include "ASkullPickup.h"
#include "Net/UnrealNetwork.h"
#include "../Characters/BaseCharacter.h"

// Sets default values
AASkullPickup::AASkullPickup()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

void AASkullPickup::AttemptPickup(ABaseCharacter* BaseCharacter)
{
	if (HasAuthority() && bExists)
	{
		BaseCharacter->OnGetSkull();
	}
}

void AASkullPickup::SetExists(bool bNewExists)
{
	bExists = bNewExists;
}

void AASkullPickup::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AASkullPickup, bExists);
}

// Called when the game starts or when spawned
void AASkullPickup::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AASkullPickup::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

