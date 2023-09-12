// Fill out your copyright notice in the Description page of Project Settings.


#include "ProceduralLandscape.h"

// Sets default values
AProceduralLandscape::AProceduralLandscape()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	ProceduralMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("Procedural Mesh")); 
	SetRootComponent(ProceduralMesh);
}

bool AProceduralLandscape::ShouldTickIfViewportsOnly() const
{
	return true;
}

// Called when the game starts or when spawned
void AProceduralLandscape::BeginPlay()
{
	Super::BeginPlay();
	
}

void AProceduralLandscape::CreateSimplePlane()
{
	// Clear the existing vertices if needed
	Vertices.Empty();

	// Add the specified vectors to the Vertices array
	FVector Vector1(0.0f, 0.0f, 0.0f);
	FVector Vector2(1000.0f, 0.0f, 0.0f);
	FVector Vector3(0.0f, 1000.0f, 0.0f);
	FVector Vector4(1000.0f, 1000.0f, 0.0f);

	Vertices.Add(Vector1);
	Vertices.Add(Vector2);
	Vertices.Add(Vector3);
	Vertices.Add(Vector4);

	// Randomize Z-Height of each vertex
	for (FVector& Vertex : Vertices)
	{
		Vertex.Z = FMath::RandRange(-500.0f, 500.0f);
	}

	// Add the specified integers to the Triangles array
	Triangles.Add(0);
	Triangles.Add(2);
	Triangles.Add(1);
	Triangles.Add(1);
	Triangles.Add(2);
	Triangles.Add(3);

	// Add the specified FVector2D to the UVCoords array
	FVector2D UV1(0.0f, 0.0f);
	FVector2D UV2(1.0f, 0.0f);
	FVector2D UV3(0.0f, 1.0f);
	FVector2D UV4(1.0f, 1.0f);

	UVCoords.Add(UV1);
	UVCoords.Add(UV2);
	UVCoords.Add(UV3);
	UVCoords.Add(UV4);

	// Draw debug spheres at each vertex location with a radius of 50.0f
	for (const FVector& Vertex : Vertices)
	{
		DrawDebugSphere(GetWorld(), Vertex, 50.0f, 12, FColor::Green, true, -1.0f, 0, 1.0f);
	}

	if (ProceduralMesh) 
	{ 
		ProceduralMesh->CreateMeshSection(0, Vertices, Triangles, TArray<FVector>(), UVCoords, 
		   TArray<FColor>(), TArray<FProcMeshTangent>(), true); 
	}
	
}

void AProceduralLandscape::ClearLandscape()
{
	Vertices.Empty();
	Triangles.Empty();
	UVCoords.Empty();
	ProceduralMesh->ClearMeshSection(0);
	UKismetSystemLibrary::FlushPersistentDebugLines(GetWorld());
	
}

// Called every frame
void AProceduralLandscape::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bShouldRegenerate)
	{
		ClearLandscape();
		CreateSimplePlane();
		bShouldRegenerate = false;
	}
}

