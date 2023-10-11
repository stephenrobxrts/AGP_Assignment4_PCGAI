// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LevelBox.h"
#include "Room.generated.h"


/**
 * @brief Box type enum
 */
UENUM(BlueprintType)
enum class EBoxType : uint8
{
	Start,
	Normal,
	End
};


UCLASS()
class AGP_API ARoom : public ALevelBox
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ARoom();

	FVector Position;
	FVector Size;
	EBoxType Type;
	FQuat Rotation = FQuat::Identity;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
