// Fill out your copyright notice in the Description page of Project Settings.


#include "IslandSpawner.h"
#include "IslandGenerationTest.h"
#include "LevelGenerator.h"

// Sets default values
AIslandSpawner::AIslandSpawner():
	NumIslands(0),
	IslandGenerationTestTemplate(nullptr),
	WidthMin(1),
	WidthMax(1),
	DepthMin(1),
	DepthMax(1)

{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AIslandSpawner::BeginPlay()
{
	Super::BeginPlay();

	if (IslandGenerationTestTemplate != nullptr)
	{

		for (int i = 0; i < NumIslands; i++)
		{
			AIslandGenerationTest* newIsland = GetWorld()->SpawnActor<AIslandGenerationTest>(IslandGenerationTestTemplate, FVector::ZeroVector, FRotator::ZeroRotator);
			ALevelGenerator* newLevel = GetWorld()->SpawnActor<ALevelGenerator>(LevelGeneratorTemplate, FVector::ZeroVector, FRotator::ZeroRotator);

			newIsland->SetSizeRange(DepthMin, DepthMax, WidthMin, WidthMax);
			newIsland->SetTileScale(TileScale);

			Islands.Add(newIsland);
			Level.Add(newLevel);
		}

		for (int i = 0; i < NumIslands; i++)
		{
			FVector offsetVec = FVector::ZeroVector;
			float offset = 0;

			int offsetX = 0;
			int offsetY = 0;

			for (int j = 0; j < i; j++)
			{
				offsetX = offsetX + (Islands[j]->MapSizeRange.WidthFinal * Islands[j]->TileScale * 4) + (Islands[j]->TileScale * 4);
				offsetY = FMath::FRandRange(0.0f, Islands[j]->MapSizeRange.DepthFinal) * Islands[j]->TileScale * 4;
			}

			Islands[i]->SetActorLocation(FVector(offsetX, offsetY, Islands[i]->GetActorLocation().Z));
		}

		for (int i = 0; i < NumIslands; i++)
		{
			Islands[i]->Generate();
		}
	}
}