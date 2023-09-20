// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PickupBase.h"
#include "AGP/WeaponComponent.h"
#include "WeaponPickup.generated.h"

UENUM(BlueprintType) // Allows us to use this enum in blueprints.
enum class EWeaponRarity : uint8 {
	Common,
	Rare,
	Master,
	Legendary
};

USTRUCT(BlueprintType)
struct FWeaponStatRange
{
	GENERATED_BODY()
public:
	float Min;
	float Max;
};

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

	UFUNCTION(BlueprintImplementableEvent)
	void UpdateWeaponPickupMaterial();
	
	virtual void OnPickupOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep,
		const FHitResult& SweepResult) override;

	UPROPERTY(BlueprintReadOnly)
	EWeaponRarity WeaponRarity = EWeaponRarity::Common;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon Stats")
	FWeaponStats WeaponStats;

	TEnumAsByte<EWeaponRarity> IWeaponRarity;

	// Arrays to store stat ranges for each weapon tier
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Stats")
	FWeaponStatArray AccuracyArray;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Stats")
	FWeaponStatArray FireRateArray;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Stats")
	FWeaponStatArray BaseDamageArray;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Stats")
	FWeaponStatArray MagazineSizeArray;	
	

private:
	
	void RollRarity();
	void GenerateWeaponPickup();
};
