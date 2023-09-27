// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PickupBase.h"
#include "AGP/Characters/Components/WeaponComponent.h"
#include "WeaponPickup.generated.h"

UENUM(BlueprintType) // Allows us to use this enum in blueprints.
enum class EWeaponRarity : uint8
{
	Common,
	Rare,
	Master,
	Legendary
};

/**
 * @brief Struct stores min/max floats for weapon stats
 */
USTRUCT(BlueprintType)
struct FWeaponStatRange
{
	GENERATED_BODY()

public:
	float Min;
	float Max;
};

/**
 * @brief Struct stores both the good(min/max) and bad(min/max) stat ranges for a weapon.
 */
USTRUCT(BlueprintType)
struct FWeaponStatArray
{
	GENERATED_BODY()

public:
	FWeaponStatRange BadStats;
	FWeaponStatRange GoodStats;
};

/**
 * 
 */
UCLASS()
class AGP_API AWeaponPickup : public APickupBase
{
	GENERATED_BODY()

public:
	AWeaponPickup();

protected:
	virtual void BeginPlay() override;

	/**
	 * @brief Used in BluePrint to change the texture of the pickup based on the rarity of the weapon.
	*/
	UFUNCTION(BlueprintImplementableEvent)
	void UpdateWeaponPickupMaterial();

	virtual void OnPickupOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	                             UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep,
	                             const FHitResult& SweepResult) override;

	/**
	 * @brief WeaponRarity Range: 0 - 3 \n Common, Rare, Master, Legendary
	*/
	UPROPERTY(BlueprintReadOnly)
	EWeaponRarity WeaponRarity = EWeaponRarity::Common;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon Stats")
	FWeaponStats WeaponStats;

	// Arrays to store stat ranges for each weapon tier
	FWeaponStatArray AccuracyArray;
	FWeaponStatArray FireRateArray;
	FWeaponStatArray BaseDamageArray;
	FWeaponStatArray MagazineSizeArray;
	FWeaponStatArray ReloadTimeArray;

private:
	/**
	 * @brief 
	 */
	void RollRarity();
	void GenerateWeaponPickup();
};
