// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseCharacter.h"
#include "GameFramework/Character.h"
#include "PlayerCharacter.generated.h"

// A set of Forward Declarations which are used to prevent needing to #include these classes in the header file.
// When these classes are used in the .cpp files, they will need to be #included there.
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;

UCLASS()
class AGP_API APlayerCharacter : public ABaseCharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	APlayerCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;


	UPROPERTY(EditDefaultsOnly)
	UInputAction* MoveAction;
	UPROPERTY(EditDefaultsOnly)
	UInputAction* LookAction;
	UPROPERTY(EditDefaultsOnly)
	UInputAction* JumpAction;
	UPROPERTY(EditDefaultsOnly)
	UInputAction* FireAction;
	UPROPERTY(EditDefaultsOnly)
	UInputAction* ReloadAction;
	UPROPERTY(EditDefaultsOnly)
	UInputMappingContext* InputMappingContext;


	UPROPERTY(EditDefaultsOnly)
	float LookSensitivity = 0.5f;

public:
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

private:
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void FireWeapon(const FInputActionValue& Value);
	void ReloadWeapon(const FInputActionValue& Value);
};
