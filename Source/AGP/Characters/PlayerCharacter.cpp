// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerCharacter.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

// Sets default values
APlayerCharacter::APlayerCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}


// Called when the game starts or when spawned
void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (const APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(InputMappingContext, 0);
		}
	}
}

// Called every frame
void APlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* Input = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		Input->BindAction(MoveAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Move);
		Input->BindAction(LookAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Look);
		Input->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ACharacter::Jump);
		Input->BindAction(FireAction, ETriggerEvent::Triggered, this, &APlayerCharacter::FireWeapon);
		Input->BindAction(InteractAction, ETriggerEvent::Triggered, this, &APlayerCharacter::InteractObj);
		Input->BindAction(InteractSelfAction, ETriggerEvent::Triggered, this, &APlayerCharacter::InteractSelf);
		Input->BindAction(PickupAction, ETriggerEvent::Triggered, this, &APlayerCharacter::PickupObj);
		Input->BindAction(ReloadAction, ETriggerEvent::Triggered, this, &APlayerCharacter::ReloadWeapon);
	}
}

void APlayerCharacter::Move(const FInputActionValue& Value)
{
	const FVector2D MovementVector = Value.Get<FVector2D>();
	const FVector ForwardVector = GetActorForwardVector();
	AddMovementInput(ForwardVector, MovementVector.X);
	const FVector RightVector = GetActorRightVector();
	AddMovementInput(RightVector, MovementVector.Y);
}

void APlayerCharacter::Look(const FInputActionValue& Value)
{
	const FVector2D LookVector = Value.Get<FVector2D>() * LookSensitivity;
	AddControllerYawInput(LookVector.X);
	AddControllerPitchInput(LookVector.Y);
}

void APlayerCharacter::FireWeapon(const FInputActionValue& Value)
{
	FVector CameraPosition;
	FRotator CameraRotation;
	//GetActorEyesViewPoint(CameraPosition, CameraRotation);
	GetWorld()->GetFirstPlayerController()->GetPlayerViewPoint(CameraPosition, CameraRotation);
	const FVector CameraForward = UKismetMathLibrary::GetForwardVector(CameraRotation);
	if (BulletStartPosition)
	{
		Fire(BulletStartPosition->GetComponentLocation() + 10000.0f * CameraForward);
	}
}

void APlayerCharacter::ReloadWeapon(const FInputActionValue& Value)
{
	Reload();
}

void APlayerCharacter::InteractObj(const FInputActionValue& Value)
{
	Interact();
}

void APlayerCharacter::InteractSelf(const FInputActionValue& Value)
{
	InteractWithSelf();
}

void APlayerCharacter::PickupObj(const FInputActionValue& Value)
{
	Pickup();
}
