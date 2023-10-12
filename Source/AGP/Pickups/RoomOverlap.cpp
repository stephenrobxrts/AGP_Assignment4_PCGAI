// Fill out your copyright notice in the Description page of Project Settings.


#include "RoomOverlap.h"

#include "AGP/Characters/BaseCharacter.h"
#include "Engine/StaticMeshActor.h"


void ARoomOverlap::BeginPlay()
{
	Super::BeginPlay();
}

void ARoomOverlap::OnPickupOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep,
                                const FHitResult& SweepResult)
{
	// Log the name of the overlapped component
	FString ComponentName = OverlappedComponent->GetName();
	UE_LOG(LogTemp, Warning, TEXT("Overlapped Component Name: %s"), *ComponentName);

	ABaseCharacter* Character = Cast<ABaseCharacter>(OtherActor);
	if (Character)
	{
		UE_LOG(LogTemp, Warning, TEXT("Custom Message: This is an overlap of player or enemy"));
		AActor* ParentActor = OverlappedComponent->GetOwner();
		
		UE_LOG(LogTemp, Warning, TEXT("Custom Message: This is a room"));
		if (ParentActor)
		{
			Character->SetCurrentRoom(ParentActor);
		}
	}
}

