// Fill out your copyright notice in the Description page of Project Settings.


#include "TorchComponent.h"


// Sets default values for this component's properties
UTorchComponent::UTorchComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


void UTorchComponent::SetTorchLit(bool bLit)
{
	bIsLit = bLit;
}

// Called when the game starts
void UTorchComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UTorchComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

