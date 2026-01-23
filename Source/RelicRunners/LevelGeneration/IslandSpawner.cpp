// Fill out your copyright notice in the Description page of Project Settings.


#include "IslandSpawner.h"
#include "IslandGenerationTest.h"
#include "LevelGenerator.h"
#include "IslandSpawningValuesComponent.h"

// Sets default values
AIslandSpawner::AIslandSpawner():
	ObjectiveType(0),
	NumIslands(0),
	TileScale(0.0f),
	IslandGenerationTestTemplate(nullptr),
	WidthMin(1),
	WidthMax(1),
	DepthMin(1),
	DepthMax(1)

{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	GenerationValuesComponent = CreateDefaultSubobject<UIslandSpawningValuesComponent>("GenerationValues");

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

			newIsland->IslandIndex = i;

			newIsland->GenerationValuesComponent->SetIslandValues(GenerationValuesComponent->IslandValues);
			newIsland->GenerationValuesComponent->SetIslandStartAndEndValues(GenerationValuesComponent->IslandStartEnd);
			newIsland->GenerationValuesComponent->SetFloorMeshes(GenerationValuesComponent->FloorMeshes);
			newIsland->GenerationValuesComponent->SetObstacleValues(GenerationValuesComponent->ObstacleValues);

			newIsland->InitializeValues();

			Islands.Add(newIsland);
		}


		for (int i = 0; i < NumIslands; i++)
		{
			FVector offsetVec = FVector::ZeroVector;
			float offset = 0;

			int offsetX = 0;
			int offsetY = 0;

			for (int j = 0; j < i; j++)
			{
				offsetX = offsetX + (Islands[j]->IslandWidth * GenerationValuesComponent->IslandValues.TileScale * 4) + (GenerationValuesComponent->IslandValues.TileScale * 4);
				offsetY = FMath::FRandRange(0.0f, Islands[j]->IslandDepth) * GenerationValuesComponent->IslandValues.TileScale * 4;
			}

			Islands[i]->SetActorLocation(FVector(offsetX, offsetY, Islands[i]->GetActorLocation().Z));
		}

		for (int i = 0; i < NumIslands; i++)
		{
			Islands[i]->Generate();
		}
	}
}

int AIslandSpawner::GetObjectiveValue()
{
	ObjectiveType = FMath::RandRange(0, 4);

	return ObjectiveType;
}
