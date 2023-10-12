// Fill out your copyright notice in the Description page of Project Settings.


#include "PickupBase.h"

#include "AGP/Characters/BaseCharacter.h"
#include "Components/BoxComponent.h"

// Sets default values
APickupBase::APickupBase()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	PickupCollider = CreateDefaultSubobject<UBoxComponent>(TEXT("Pickup Collider"));
	PickupMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Pickup Mesh"));
	SetRootComponent(PickupCollider);


	PickupMesh->SetupAttachment(GetRootComponent());
}

// Called when the game starts or when spawned
void APickupBase::BeginPlay()
{
	Super::BeginPlay();

	if (PickupCollider)
	{
		PickupCollider->OnComponentBeginOverlap.AddDynamic(this, &APickupBase::OnPickupOverlap);
	}
}

// Called every frame
void APickupBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void APickupBase::OnPickupOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                  UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep,
                                  const FHitResult& SweepResult)
{
	UE_LOG(LogTemp, Display, TEXT("Overlap event occurred on PickupBase."));
	// Check if the overlapped actor is a BaseCharacter
	// Component->GetParentActor() returns the actor that owns the component which should be static mesh room
	// If so: Set the BaseCharacter's->SetRoom(This Collider's Static Mesh parent)

}
