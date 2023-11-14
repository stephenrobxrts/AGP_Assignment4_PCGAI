// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseCharacter.h"

#include "../Pickups/WeaponPickup.h"
#include "AGP/Pickups/TorchPickup.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/KismetMathLibrary.h"

// Sets default values
ABaseCharacter::ABaseCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	BulletStartPosition = CreateDefaultSubobject<USceneComponent>(TEXT("BulletStartPosition"));
	BulletStartPosition->SetupAttachment(GetRootComponent());
	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
	GetCharacterMovement()->MaxWalkSpeed = 400.0f;
}

void ABaseCharacter::SetCurrentRoom(AActor* NewRoom)
{
	CurrentRoom = NewRoom;
}

AActor* ABaseCharacter::GetCurrentRoom()
{
	return CurrentRoom;
}

bool ABaseCharacter::IsPlayerMoving()
{
	if (Controller)
	{
		// Get the player character's velocity
		FVector CharacterVelocity = GetCharacterMovement()->Velocity;
		// Small moving threshold
		const float MovingThreshold = 10.0f;
		// Check if the character's velocity magnitude is above the threshold
		if (CharacterVelocity.SizeSquared() > FMath::Square(MovingThreshold))
		{
			// Player is moving
			return true;
		}
	}
	// Player is not moving
	return false;
}

// Called when the game starts or when spawned
void ABaseCharacter::BeginPlay()
{
	Super::BeginPlay();
}

void ABaseCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ABaseCharacter, WeaponComponent);
}

bool ABaseCharacter::HasWeapon()
{
	if (WeaponComponent)
	{
		return true;
	}
	return false;
}

bool ABaseCharacter::HasTorch()
{
	return bHasTorch;
}

void ABaseCharacter::SetIsOverlappingPickup(bool bIsOverlapping)
{
	if (bIsOverlappingPickup == bIsOverlapping)
	{
		return;
	}
	UE_LOG(LogTemp, Display, TEXT("Player is overlapping pickup: %s"), *GetName());
	bIsOverlappingPickup = bIsOverlapping;
}

void ABaseCharacter::EquipWeapon(bool bEquipWeapon, const FWeaponStats WeaponStats, const EWeaponRarity WeaponRarity)
{
	if (GetLocalRole() == ROLE_Authority)
	{
		EquipWeaponImplementation(bEquipWeapon, WeaponStats, WeaponRarity);
		MulticastEquipWeapon(bEquipWeapon, WeaponRarity);
	}
}

void ABaseCharacter::EquipTorch(bool bEquipTorch, bool bIsLit)
{
	EquipTorchImplementation(bEquipTorch, bIsLit);
	MulticastEquipTorch(bEquipTorch, bIsLit);
}


void ABaseCharacter::InteractWithSelf()
{
	UE_LOG(LogTemp, Display, TEXT("Player is interacting with self: %s"), *GetName());
	if (GetLocalRole() == ROLE_Authority)
	{
		ToggleOwnTorch();
		bHasTorch = true;
	}
	ServerInteractSelf();
}

void ABaseCharacter::EquipWeaponImplementation(bool bEquipWeapon, const FWeaponStats& WeaponStats,
                                               const EWeaponRarity WeaponRarity)
{
	//Picking up Weapon - create weapon component and apply stats, materials
	if (bEquipWeapon && !HasWeapon())
	{
		WeaponComponent = NewObject<UWeaponComponent>(this);
		WeaponComponent->RegisterComponent();
		WeaponComponent->ApplyWeaponStats(WeaponStats);
	}
	//Changing Weapon - change stats and material 
	else if (bEquipWeapon && HasWeapon())
	{
		WeaponComponent->ApplyWeaponStats(WeaponStats);
	}
	//Dropping Weapon
	else if (!bEquipWeapon && HasWeapon())
	{
		WeaponComponent->UnregisterComponent();
		WeaponComponent = nullptr;
	}

	EquipWeaponGraphical(bEquipWeapon, WeaponRarity);

	if (bEquipWeapon)
	{
		UE_LOG(LogTemp, Display, TEXT("Player has equipped weapon"))
	}
	else
	{
		UE_LOG(LogTemp, Display, TEXT("Player has unequipped weapon"))
	}
}

void ABaseCharacter::EquipTorchImplementation(bool bEquipTorch, bool bIsLit)
{
	//EquipTorchGraphical(bEquipTorch, bIsLit);
}

void ABaseCharacter::ServerEquipTorch_Implementation(ATorchPickup* TorchPickup)
{
	if (TorchPickup)
	{
		TorchPickup->AttemptPickUp(this);
		bHasTorch = true;
	}
}


void ABaseCharacter::MulticastEquipWeapon_Implementation(bool bEquipWeapon, EWeaponRarity WeaponRarity)
{
	EquipWeaponGraphical(bEquipWeapon, WeaponRarity);
	//EquipWeaponImplementation(bEquipWeapon, WeaponStats, WeaponRarity);
}

void ABaseCharacter::MulticastEquipTorch_Implementation(bool bEquipTorch, bool bIsLit)
{
	EquipTorchGraphical(bEquipTorch, bIsLit);
}


void ABaseCharacter::ServerInteractTorch_Implementation(ATorchPickup* TorchPickup)
{
	TorchPickup->OnInteract();

}

void ABaseCharacter::ServerInteractSelf_Implementation()
{
	//UE_LOG(LogTemp, Display, TEXT("ServerAck interacting with self: %s"), *GetName());
	if (bHasTorch)
	{
		//UE_LOG(LogTemp, Display, TEXT("Player is toggling torch: %s"), *GetName());
		ToggleOwnTorch();
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

bool ABaseCharacter::Interact()
{
	FVector CameraPosition;
	FRotator CameraRotation;
	//GetActorEyesViewPoint(CameraPosition, CameraRotation);
	GetWorld()->GetFirstPlayerController()->GetPlayerViewPoint(CameraPosition, CameraRotation);

	FVector Start = GetActorLocation() + (GetActorUpVector() * 50.0f);
	FVector ForwardVector = GetActorForwardVector();
	const FVector CameraForward = UKismetMathLibrary::GetForwardVector(CameraRotation);
	FVector End = ((CameraForward * 100.f) + Start); // Change 1000.f to your desired distance

	FHitResult HitResult;

	// Setup the query parameters
	FCollisionQueryParams CollisionParams;
	CollisionParams.AddIgnoredActor(GetOwner()); // Ignore the player
	CollisionParams.AddIgnoredActors(GetOwner()->Children);

	// Perform the line trace
	bool bIsHit = GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, CollisionParams);

	if (bIsHit)
	{
		if (ATorchPickup* PickupObject = Cast<ATorchPickup>(HitResult.GetActor()))
		{
			UE_LOG(LogTemp, Display, TEXT("Player is Interacting with torch: %s"), *GetName());
			// Call a method on the pickup object to handle being picked up
			ServerInteractTorch(PickupObject);
		}
	}
	DrawDebugLine(GetWorld(), Start, End, FColor::Red, false, 1.0f, 0, 1.0f);
	return true;
}


bool ABaseCharacter::Pickup()
{
	FVector CameraPosition;
	FRotator CameraRotation;
	//GetActorEyesViewPoint(CameraPosition, CameraRotation);
	GetWorld()->GetFirstPlayerController()->GetPlayerViewPoint(CameraPosition, CameraRotation);

	FVector Start = GetActorLocation() + (GetActorUpVector() * 50.0f);
	FVector ForwardVector = GetActorForwardVector();
	const FVector CameraForward = UKismetMathLibrary::GetForwardVector(CameraRotation);
	FVector End = ((CameraForward * 100.f) + Start); // Change 1000.f to your desired distance

	FHitResult HitResult;

	// Setup the query parameters
	FCollisionQueryParams CollisionParams;
	CollisionParams.AddIgnoredActor(GetOwner()); // Ignore the player
	CollisionParams.AddIgnoredActors(GetOwner()->Children);


	// Perform the line trace
	bool bIsHit = GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, CollisionParams);

	if (bIsHit)
	{
		if (ATorchPickup* PickupObject = Cast<ATorchPickup>(HitResult.GetActor()))
		{
			UE_LOG(LogTemp, Display, TEXT("Player is picking up torch: %s"), *this->GetActorLabel());
			// Call a method on the pickup object to handle being picked up
			ServerEquipTorch(PickupObject);
			//PickupObject->AttemptPickUp(this);
			//EquipTorch(true, PickupObject->bIsLit);
		}
	}
	DrawDebugLine(GetWorld(), Start, End, FColor::Red, false, 1.0f, 0, 1.0f);
	return true;
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
