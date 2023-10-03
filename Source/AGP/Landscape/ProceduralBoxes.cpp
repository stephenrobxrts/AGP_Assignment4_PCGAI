// Fill out your copyright notice in the Description page of Project Settings.


#include "ProceduralBoxes.h"
#include "EngineUtils.h"
#include "Components/BoxComponent.h"
#include "Builders/CubeBuilder.h"


// Sets default values
AProceduralBoxes::AProceduralBoxes()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
}

bool AProceduralBoxes::ShouldTickIfViewportsOnly() const
{
	
	return true;
}

// Called when the game starts or when spawned
void AProceduralBoxes::BeginPlay()
{
	
	
	Super::BeginPlay();
	
}

void AProceduralBoxes::DebugShowNavNodes()
{
	//Call the DebugShowNavNodes function on each NavNode
	for (ANavigationNode* Node : Nodes)
	{
		Node->DebugSetVisibility(bDebugShowNavNodes);
	}
	
}

void AProceduralBoxes::GenerateLandscape()
{
	CreateSimpleBox();
}

void AProceduralBoxes::CreateSimpleBox()
{
	UWorld* World = GetWorld();
	UCubeBuilder* cubeBuilder = Cast<UCubeBuilder>(GEditor->FindBrushBuilder(UCubeBuilder::StaticClass()));
	cubeBuilder->X = 1024.0f;
	cubeBuilder->Y = 1024.0f;
	cubeBuilder->Z = 226.0f;
	cubeBuilder->Hollow = true;
	cubeBuilder->WallThickness = 15.0f;
	cubeBuilder->Build(World);
	GEditor->Exec(World, TEXT("BRUSH MOVETO X=0 Y=0 Z=0"));
	GEditor->Exec(World, TEXT("BRUSH ADD"));
}

void AProceduralBoxes::ClearLandscape()
{

}

// Called every frame
void AProceduralBoxes::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (bShouldRegenerate)
	{
		ClearLandscape();
		GenerateLandscape();
		//SpawnPickups(); //Now implemented in PickupManagerSubsystem
		bShouldRegenerate = false;
	}
}

