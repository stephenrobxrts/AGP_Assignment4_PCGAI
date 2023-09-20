// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseCharacter.h"

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
	else
	{
		return false;	
	}
}

void ABaseCharacter::EquipWeapon(bool bEquipWeapon)
{
	EquipWeaponGraphical(bEquipWeapon);
	if (bEquipWeapon && !HasWeapon())
    {
     WeaponComponent = NewObject<UWeaponComponent>(this);
     WeaponComponent->RegisterComponent();
    }
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
		return WeaponComponent->Fire(BulletStartPosition->GetComponentLocation(), FireAtLocation);
	}
	else
	{
		return false;
	}
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

	

}



