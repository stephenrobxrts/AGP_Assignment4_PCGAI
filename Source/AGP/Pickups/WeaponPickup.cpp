// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponPickup.h"

#include "AGP/PlayerCharacter.h"
#include "Chaos/Utilities.h"

AWeaponPickup::AWeaponPickup()
{
	// Set default values for your arrays here
	AccuracyArray = {{0.9f, 0.98f}, {0.98f, 1.0f}};
	FireRateArray = {{0.05f, 0.2f}, {0.2f, 1.0f}};
	BaseDamageArray = {{5.0f, 15.0f}, {15.0f, 30.0f}};
	MagazineSizeArray = {{1, 19}, {20, 100}};
}

void AWeaponPickup::BeginPlay()
{
	Super::BeginPlay();
	GenerateWeaponPickup();
	UpdateWeaponPickupMaterial();
}

void AWeaponPickup::OnPickupOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                    UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	UE_LOG(LogTemp, Display, TEXT("Overlap event occurred on WeaponPickup"))

	// Because getting a pointer to something we want to do actions on or get information from is common, and we always
	// want to be sure the pointer is not a nullptr before using it, we can use this pattern where we both get the pointer
	// and check that it is valid inside the if statement. In this case, the PlayerCharacter pointer variable will only
	// exist inside the scope of the if statement.
	if (ABaseCharacter* BaseCharacter = Cast<ABaseCharacter>(OtherActor))
	{
		if (!BaseCharacter->HasWeapon())
		{
			BaseCharacter->EquipWeapon(true);
			this->Destroy();
		}
	}
}

void AWeaponPickup::RollRarity()
{
	int32 RarityRoll = FMath::RandRange(0, 100);
	if (RarityRoll <= 50)
	{
		WeaponRarity = EWeaponRarity::Common;
	}
	else if (RarityRoll <= 80)
	{
		WeaponRarity = EWeaponRarity::Rare;
	}
	else if (RarityRoll <= 95)
	{
		WeaponRarity = EWeaponRarity::Master;
	}
	else
	{
		WeaponRarity = EWeaponRarity::Legendary;
	}
}

void AWeaponPickup::GenerateWeaponPickup()
{
	//Modifies WeaponRarity Enum
	RollRarity();
	
	//Set Default ranges
	//If legendary all good, if not all bad (adjust for master, rare)
	FWeaponStatRange AccuracyRange = WeaponRarity == EWeaponRarity::Legendary ? AccuracyArray.GoodStats : AccuracyArray.BadStats;
	FWeaponStatRange FireRateRange = WeaponRarity == EWeaponRarity::Legendary ? FireRateArray.GoodStats : FireRateArray.BadStats;
	FWeaponStatRange BaseDamageRange = WeaponRarity == EWeaponRarity::Legendary ? BaseDamageArray.GoodStats : BaseDamageArray.BadStats;
	FWeaponStatRange MagazineSizeRange = WeaponRarity == EWeaponRarity::Legendary ? MagazineSizeArray.GoodStats : MagazineSizeArray.BadStats;

	//Stat arrays hold good and bad stat ranges
	TArray<FWeaponStatArray> StatArrays = {AccuracyArray, FireRateArray, BaseDamageArray, MagazineSizeArray};
	//Chosen stat ranges will be either the good or bad stat ranges
	TArray<FWeaponStatRange> ChosenStatRanges = {AccuracyRange, FireRateRange, BaseDamageRange, MagazineSizeRange};

	
	//Adjust for master, rare
	if (WeaponRarity == EWeaponRarity::Master)
	{
		int32 BadStatRoll = FMath::RandRange(0, 3);
		for (int32 i = 0; i < 4; i++)
		{
			ChosenStatRanges[i] = i == BadStatRoll ? StatArrays[i].BadStats : StatArrays[i].GoodStats;
		}
	}
	else if (WeaponRarity == EWeaponRarity::Rare)
	{
		int32 BadStatRoll1 = FMath::RandRange(0, 3);
		int32 BadStatRoll2;
		do {
			BadStatRoll2 = FMath::RandRange(0, 3);
		} while (BadStatRoll2 == BadStatRoll1);

		for (int32 i = 0; i < 4; i++)
		{
			ChosenStatRanges[i] = (i == BadStatRoll1 || i == BadStatRoll2) ? StatArrays[i].BadStats : StatArrays[i].GoodStats;
		}
	}

	//From the chosen stat ranges, set the weapon stats by randomizing within the range
	WeaponStats.Accuracy = FMath::RandRange(AccuracyRange.Min, AccuracyRange.Max);
	WeaponStats.FireRate = FMath::RandRange(FireRateRange.Min, FireRateRange.Max);
	WeaponStats.BaseDamage = FMath::RandRange(BaseDamageRange.Min, BaseDamageRange.Max);
	WeaponStats.MagazineSize = static_cast<int32>(FMath::RandRange(MagazineSizeRange.Min, MagazineSizeRange.Max));
}
