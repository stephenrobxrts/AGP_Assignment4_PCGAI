// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponComponent.h"
#include "BaseCharacter.h"


// Sets default values for this component's properties
UWeaponComponent::UWeaponComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}

bool UWeaponComponent::Fire(const FVector& BulletStart, const FVector& FireAtLocation)
{
	if (TimeSinceLastShot < WeaponStats.FireRate)
	{
		return false;
	}
	FHitResult HitResult;
	const FVector StartLocation = BulletStart;

	
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(GetOwner());
	
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
			HitActor->GetComponentByClass<UHealthComponent>()->UHealthComponent::ApplyDamage(WeaponStats.BaseDamage);
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


// Called when the game starts
void UWeaponComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UWeaponComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                     FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);


	TimeSinceLastShot += DeltaTime;
}

