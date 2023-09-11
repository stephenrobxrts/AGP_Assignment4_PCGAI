// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseCharacter.h"

// Sets default values
ABaseCharacter::ABaseCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	BulletStartPosition = CreateDefaultSubobject<USceneComponent>(TEXT("BulletStartPosition"));
	BulletStartPosition->SetupAttachment(GetRootComponent());
}

// Called when the game starts or when spawned
void ABaseCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

bool ABaseCharacter::HasWeapon()
{
	return bHasWeaponEquipped;
}

void ABaseCharacter::EquipWeapon(bool bEquipWeapon)
{
	EquipWeaponGraphical(bEquipWeapon);
	bHasWeaponEquipped = bEquipWeapon;
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
	if (TimeSinceLastShot < MinTimeBetweenShots)
	{
		return false;
	}
	FHitResult HitResult;
	const FVector StartLocation = BulletStartPosition->GetComponentLocation();

	
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);
	
	GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, FireAtLocation, ECollisionChannel::ECC_WorldStatic, QueryParams);\

	UE_LOG(LogTemp, Display, TEXT("Fire!"));
	AActor* HitActor = HitResult.GetActor();
	if (HitActor)
	{
		if (ABaseCharacter* BaseCharacter = Cast<ABaseCharacter>(HitActor))
		{
			// The hit result actor is of type ABaseCharacter
			// Draw a green debug line
			DrawDebugLine(GetWorld(), StartLocation, HitResult.ImpactPoint, FColor::Green, false, 1.0f, 0, 1.0f);
		}
		else
		{
			// The hit result actor is NOT of type ABaseCharacter
			// Draw an orange debug line
			DrawDebugLine(GetWorld(), StartLocation, HitResult.ImpactPoint, FColor::Orange, false, 1.0f, 0, 1.0f);
		}
	}
	else
	{
		// The hit result actor is null
		// Draw a red debug line
		DrawDebugLine(GetWorld(), StartLocation, HitResult.ImpactPoint, FColor::Red, false, 1.0f, 0, 1.0f);
	}


	TimeSinceLastShot = 0.0f;
	return true;
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
	if(bHasWeaponEquipped)
	{
		TimeSinceLastShot += DeltaTime;
	}
	

}



