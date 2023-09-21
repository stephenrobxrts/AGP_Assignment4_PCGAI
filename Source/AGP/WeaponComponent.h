// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "WeaponComponent.generated.h"


UENUM(BlueprintType) // Allows us to use this enum in blueprints.
enum class EWeaponType : uint8 {
	Rifle,
	Pistol
};

USTRUCT(BlueprintType)
struct FWeaponStats
{
	 GENERATED_BODY()
public:
	 
	 EWeaponType WeaponType = EWeaponType::Rifle;
	 float Accuracy = 1.0f;
	 float FireRate = 0.2f;
	 float BaseDamage = 10.0f;
	 int32 MagazineSize = 5;
	float ReloadTime = 1.0f;
};


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class AGP_API UWeaponComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UWeaponComponent();

	bool Fire(const FVector& BulletStart, const FVector& FireAtLocation);
	bool Reload();

	int32 GetRoundsRemainingInMagazine() const;
	float GetReloadTime() const;

	void ApplyWeaponStats(const FWeaponStats& NewWeaponStats);

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon Stats")
	FWeaponStats WeaponStats;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon Stats")
	int32 RoundsRemainingInMagazine;
	float TimeSinceLastShot = 0.0f;
	float TimeSinceReload = 0.0f;
	bool bIsReloading = false;
	

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

	
};
