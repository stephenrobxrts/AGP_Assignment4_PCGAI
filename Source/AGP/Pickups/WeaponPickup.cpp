// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponPickup.h"
#include "AGP/Characters/PlayerCharacter.h"
#include "Chaos/Utilities.h"

AWeaponPickup::AWeaponPickup()
{
	// Set default values for your arrays here
	AccuracyArray = {{0.9f, 0.98f}, {0.98f, 1.0f}};
	FireRateArray = {{0.05f, 0.2f}, {0.2f, 1.0f}};
	BaseDamageArray = {{5.0f, 15.0f}, {15.0f, 30.0f}};
	MagazineSizeArray = {{1, 19}, {20, 100}};
	ReloadTimeArray = {{1.0f, 4.0f}, {0.1f, 1.0f}};
}

void AWeaponPickup::BeginPlay()
{
	Super::BeginPlay();
	GenerateWeaponPickup();
	UpdateWeaponPickupMaterial();
}

void AWeaponPickup::OnPickupOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                    UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep,
                                    const FHitResult& SweepResult)
{
	UE_LOG(LogTemp, Display, TEXT("Overlap event occurred on WeaponPickup"))

	// Because getting a pointer to something we want to do actions on or get information from is common, and we always
	// want to be sure the pointer is not a nullptr before using it, we can use this pattern where we both get the pointer
	// and check that it is valid inside the if statement. In this case, the PlayerCharacter pointer variable will only
	// exist inside the scope of the if statement.
	if (ABaseCharacter* BaseCharacter = Cast<ABaseCharacter>(OtherActor))
	{
		/*if (!BaseCharacter->HasWeapon())
		{	}*/
		BaseCharacter->EquipWeapon(true, WeaponStats, WeaponRarity);
		this->Destroy();
	}
}

void AWeaponPickup::RollRarity()
{
	int32 RarityRoll = FMath::RandRange(1, 100);
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
	FWeaponStatRange AccuracyRange = AccuracyArray.BadStats;
	FWeaponStatRange FireRateRange = FireRateArray.BadStats;
	FWeaponStatRange BaseDamageRange = BaseDamageArray.BadStats;
	FWeaponStatRange MagazineSizeRange = MagazineSizeArray.BadStats;
	FWeaponStatRange ReloadTimeRange = ReloadTimeArray.BadStats;
	int32 NumGoodStats = 0;

	//Stat arrays hold good and bad stat ranges
	TArray<FWeaponStatArray*> StatArrays = {
		&AccuracyArray, &FireRateArray, &BaseDamageArray, &MagazineSizeArray, &ReloadTimeArray
	};
	//Chosen stat ranges will be either the good or bad stat ranges
	TArray<FWeaponStatRange> ChosenStatRanges = {
		AccuracyRange, FireRateRange, BaseDamageRange, MagazineSizeRange, ReloadTimeRange
	};

	//Number of good stats is 0 if common, 2 if rare, 3 if master, 4 if legendary
	//Determine number of GoodStats - common is already done
	if (WeaponRarity != EWeaponRarity::Common)
	{
		NumGoodStats = static_cast<int32>(WeaponRarity) + 1;
		TArray<int32> StatRolls;
		//Roll for each stat
		for (int32 i = 0; i < NumGoodStats; i++)
		{
			StatRolls.Add(FMath::RandRange(0, NumGoodStats - 1));
		}
		//Set the good stat ranges
		for (int32 i = 0; i < StatRolls.Num(); i++)
		{
			ChosenStatRanges[StatRolls[i]] = StatArrays[StatRolls[i]]->GoodStats;
		}
	}

	//From the chosen stat ranges, set the weapon stats by randomizing within the range
	WeaponStats.Accuracy = FMath::RandRange(AccuracyRange.Min, AccuracyRange.Max);
	WeaponStats.FireRate = FMath::RandRange(FireRateRange.Min, FireRateRange.Max);
	WeaponStats.BaseDamage = FMath::RandRange(BaseDamageRange.Min, BaseDamageRange.Max);
	WeaponStats.MagazineSize = static_cast<int32>(FMath::RandRange(MagazineSizeRange.Min, MagazineSizeRange.Max));
	WeaponStats.ReloadTime = FMath::RandRange(ReloadTimeRange.Min, ReloadTimeRange.Max);
}
