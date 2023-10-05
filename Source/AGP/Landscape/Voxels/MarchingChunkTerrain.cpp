// Fill out your copyright notice in the Description page of Project Settings.


#include "MarchingChunkTerrain.h"



// Sets default values
AMarchingChunkTerrain::AMarchingChunkTerrain()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AMarchingChunkTerrain::BeginPlay()
{
	Super::BeginPlay();
	
}

void AMarchingChunkTerrain::Setup()
{
	
}

// Called every frame
void AMarchingChunkTerrain::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AMarchingChunkTerrain::March(int X, int Y, int Z, const float Cube[8])
{
}

int AMarchingChunkTerrain::GetVoxelIndex(int X, int Y, int Z) const
{
	return 0;
}

void AMarchingChunkTerrain::ApplyMesh() const
{
}

void AMarchingChunkTerrain::ClearMesh()
{
}

void AMarchingChunkTerrain::GenerateHeightMap()
{
	
}

