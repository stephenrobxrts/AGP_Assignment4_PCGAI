// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseCharacter.h"

#include "../Pickups/WeaponPickup.h"

// Sets default values
ABaseCharacter::ABaseCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	BulletStartPosition = CreateDefaultSubobject<USceneComponent>(TEXT("BulletStartPosition"));
	BulletStartPosition->SetupAttachment(GetRootComponent());
	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
}

// Called when the game starts or when spawned
void ABaseCharacter::BeginPlay()
{
	Super::BeginPlay();
}

bool ABaseCharacter::HasWeapon()
{
	if (WeaponComponent)
	{
		return true;
	}
	return false;
}

void ABaseCharacter::EquipWeapon(bool bEquipWeapon, const FWeaponStats WeaponStats, const EWeaponRarity WeaponRarity)
{
	//Picking up Weapon - create weapon component and apply stats, materials
	if (bEquipWeapon && !HasWeapon())
	{
		WeaponComponent = NewObject<UWeaponComponent>(this);
		WeaponComponent->RegisterComponent();
		WeaponComponent->ApplyWeaponStats(WeaponStats);
		EquipWeaponGraphical(bEquipWeapon, WeaponRarity);
	}
	//Changing Weapon - change stats and material 
	else if (bEquipWeapon && HasWeapon())
	{
		WeaponComponent->ApplyWeaponStats(WeaponStats);
		EquipWeaponGraphical(bEquipWeapon, WeaponRarity);
	}
	//Dropping Weapon
	else if (!bEquipWeapon && HasWeapon())
	{
		WeaponComponent->UnregisterComponent();
		WeaponComponent = nullptr;
	}

	if (bEquipWeapon)
	{
		UE_LOG(LogTemp, Display, TEXT("Player has equipped weapon"))
	}
	else
	{
		UE_LOG(LogTemp, Display, TEXT("Player has unequipped weapon"))
	}
}

bool ABaseCharacter::Fire(const FVector& FireAtLocation)
{
	if (HasWeapon())
	{
		if (bIsReloading)
		{
			return false;
		}
		return WeaponComponent->Fire(BulletStartPosition->GetComponentLocation(), FireAtLocation);
	}
	return false;
}

bool ABaseCharacter::Reload()
{
	if (!HasWeapon() || bIsReloading)
	{
		return false;
	}

	UE_LOG(LogTemp, Display, TEXT("Player is reloading: %s , ReloadTime: %f"), *GetName(),
	       WeaponComponent->GetReloadTime());
	bIsReloading = true;
	return WeaponComponent->Reload();
}


// Called to bind functionality to input
void ABaseCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

// Called every frame
void ABaseCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsReloading)
	{
		if (TimeSinceReload >= WeaponComponent->GetReloadTime())
		{
			bIsReloading = false;
			TimeSinceReload = 0.0f;
		}
		TimeSinceReload += DeltaTime;
	}
}
