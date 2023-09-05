// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponPickup.h"

#include "AGP/PlayerCharacter.h"

void AWeaponPickup::OnPickupOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                    UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	UE_LOG(LogTemp, Display, TEXT("Overlap event occurred on WeaponPickup"))

	// Because getting a pointer to something we want to do actions on or get information from is common, and we always
	// want to be sure the pointer is not a nullptr before using it, we can use this pattern where we both get the pointer
	// and check that it is valid inside the if statement. In this case, the PlayerCharacter pointer variable will only
	// exist inside the scope of the if statement.
	if (APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(OtherActor))
	{
		if (!PlayerCharacter->HasWeapon())
		{
			PlayerCharacter->EquipWeapon(true);
			this->Destroy();
		}
	}
}
