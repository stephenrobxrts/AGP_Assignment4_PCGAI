// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NiagaraComponent.h"
#include "Engine/PointLight.h"
#include "Components/ActorComponent.h"

#include "TorchComponent.generated.h"


UCLASS(ClassGroup=(Custom), Blueprintable)
class AGP_API UTorchComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UTorchComponent();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UStaticMeshComponent* TorchMesh;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	APointLight* Light = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UNiagaraEmitter* TorchParticle = nullptr;

	UNiagaraComponent* TorchParticleComponent = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Speed = 1.0f;
	//UPROPERTY(EditAnywhere, BlueprintReadWrite)
	//UNiagaraComponent* TorchParticle;	
	

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsLit = true;
	void SetTorchLit(bool bLit);

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;
};
