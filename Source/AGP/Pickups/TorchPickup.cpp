// Fill out your copyright notice in the Description page of Project Settings.


#include "TorchPickup.h"


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

// Called every frame
void ATorchPickup::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

