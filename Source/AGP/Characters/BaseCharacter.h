// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AGP/Pickups/TorchPickup.h"
#include "AGP/Pickups/WeaponPickup.h"
#include "Components/HealthComponent.h"
#include "Components/WeaponComponent.h"
#include "GameFramework/Character.h"
#include "BaseCharacter.generated.h"

UCLASS()
class AGP_API ABaseCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ABaseCharacter();

	void SetCurrentRoom(AActor* NewRoom);
	AActor* GetCurrentRoom();
	bool IsPlayerMoving();

	/**
	 * @brief A getter to retrieve whether the player has a weapon equipped or not.
	 * @return true if player has weapon and false otherwise
	 */
	UFUNCTION(BlueprintCallable)
	bool HasWeapon();


	UFUNCTION(BlueprintCallable)
	bool HasTorch();

	void SetIsOverlappingPickup(bool bIsOverlapping);
	TSubclassOf<APickupBase>* PickupActor;
	
	
	/**
	 * @brief Will either equip or un-equip a weapon on this player.
	 * @param bEquipWeapon Whether to equip (true) or un-equip (false)
	 * @param WeaponStats The stats of the weapon to equip
	 * @param WeaponRarity The rarity of the weapon to equip
	 */
	void EquipWeapon(bool bEquipWeapon, const FWeaponStats WeaponStats, const EWeaponRarity WeaponRarity);

	void EquipTorch(bool bEquipTorch, bool bIsLit);

	void InteractWithSelf();


protected:
	// NOTE: If you wanted to have multiple different types of weapons, you might want to specify the weapon type
	// in some enum instead of just a boolean. Or alternatively, you could attach what is called a Child Actor Component
	// and define the type of weapon in a separate Weapon Actor class and store a pointer to this weapon.
	// For the purposes of the lab activities currently, we will just be using a boolean for simplicity.

	UPROPERTY(VisibleAnywhere)
	AActor* CurrentRoom = nullptr;
	
	UPROPERTY(VisibleAnywhere)
	USceneComponent* BulletStartPosition;

	UPROPERTY(VisibleAnywhere)
	UHealthComponent* HealthComponent;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const override;

	bool bIsOverlappingPickup;

	/**
 * @brief Is automatically called by the EquipWeapon function and should be overriden in a blueprint derived
 * class to drive any graphical changes that are needed when equipping or un-equipping a weapon.
 * @param bEquipWeapon Whether a weapon is being equipped (true) or un-equipped (false).
 */
	UPROPERTY(Replicated, VisibleAnywhere)
	UWeaponComponent* WeaponComponent = nullptr;

	UFUNCTION(BlueprintImplementableEvent)
	void EquipWeaponGraphical(bool bEquipWeapon, EWeaponRarity WeaponRarity);
	void EquipWeaponImplementation(bool bEquipWeapon, const FWeaponStats& WeaponStats = FWeaponStats(), const EWeaponRarity WeaponRarity = EWeaponRarity::Common);
	UFUNCTION(NetMulticast, Reliable)
	void MulticastEquipWeapon(bool bEquipWeapon, EWeaponRarity WeaponRarity = EWeaponRarity::Common);

	
	UFUNCTION(BlueprintImplementableEvent)
	void EquipTorchGraphical(bool bEquipTorch, bool bIsLit);
	void EquipTorchImplementation(bool bEquipTorch, bool bIsLit);
	UFUNCTION(NetMulticast, Reliable)
	void MulticastEquipTorch(bool bEquipTorch, bool bIsLit);
	UFUNCTION(Server, Reliable)
	void ServerEquipTorch(ATorchPickup* TorchPickup);

	UFUNCTION(BlueprintImplementableEvent)
	void ToggleOwnTorch();
	UFUNCTION(Server, Reliable)
	void ServerInteractTorch(ATorchPickup* TorchPickup);

	UFUNCTION(Server, Reliable)
	void ServerInteractSelf();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastToggleTorch(bool bEquipTorch, bool bIsLit);
	void ToggleTorchGraphical();

	

	

	
	/*UFUNCTION(BlueprintImplementableEvent)
	void InteractSelfGraphical(bool bIsLit);
	void InteractSelfImplementation(bool bIsLit);
	UFUNCTION(BlueprintImplementableEvent)
	void MulticastInteractSelf(bool bIsLit);*/
	



	
	
	bool Fire(const FVector& FireAtLocation);
	bool Reload();
	bool bIsReloading;
	float TimeSinceReload;

	bool bHasTorch = false;

	bool Interact();

	bool Pickup();

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
};
