// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "PlayerCharacter.generated.h"

// A set of Forward Declarations which are used to prevent needing to #include these classes in the header file.
// When these classes are used in the .cpp files, they will need to be #included there.
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;

UCLASS()
class AGP_API APlayerCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	APlayerCharacter();

	/**
	 * @brief A getter to retrieve whether the player has a weapon equipped or not.
	 * @return true if player has weapon and false otherwise
	 */
	UFUNCTION(BlueprintCallable)
	bool HasWeapon();
	
	/**
	 * @brief Will either equip or un-equip a weapon on this player.
	 * @param bEquipWeapon Whether to equip (true) or un-equip (false)
	 */
	void EquipWeapon(bool bEquipWeapon);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	/**
	 * @brief Is automatically called by the EquipWeapon function and should be overriden in a blueprint derived
	 * class to drive any graphical changes that are needed when equipping or un-equipping a weapon.
	 * @param bEquipWeapon Whether a weapon is being equipped (true) or un-equipped (false).
	 */
	UFUNCTION(BlueprintImplementableEvent)
	void EquipWeaponGraphical(bool bEquipWeapon);

	UPROPERTY(EditDefaultsOnly)
	UInputAction* MoveAction;
	UPROPERTY(EditDefaultsOnly)
	UInputAction* LookAction;
	UPROPERTY(EditDefaultsOnly)
	UInputAction* JumpAction;
	UPROPERTY(EditDefaultsOnly)
	UInputMappingContext* InputMappingContext;

	UPROPERTY(EditDefaultsOnly)
	float LookSensitivity = 0.5f;

	// NOTE: If you wanted to have multiple different types of weapons, you might want to specify the weapon type
	// in some enum instead of just a boolean. Or alternatively, you could attach what is called a Child Actor Component
	// and define the type of weapon in a separate Weapon Actor class and store a pointer to this weapon.
	// For the purposes of the lab activities currently, we will just be using a boolean for simplicity.
	bool bHasWeaponEquipped = false;

	
	

public:
	
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
private:
	
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);

};
