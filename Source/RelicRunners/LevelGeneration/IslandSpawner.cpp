// Fill out your copyright notice in the Description page of Project Settings.


#include "IslandSpawner.h"
#include "IslandGenerationTest.h"
#include "LevelGenerator.h"
#include "IslandSpawningValuesComponent.h"
#include "PayloadActor.h"


// Sets default values
AIslandSpawner::AIslandSpawner():
	IslandGenerationTestTemplate(nullptr),
	NumIslands(0),
	CurrentSpawnedIsland(0)
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	GenerationValuesComponent = CreateDefaultSubobject<UIslandSpawningValuesComponent>("GenerationValues");

}

// Called when the game starts or when spawned
void AIslandSpawner::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{

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
				newIsland->GenerationValuesComponent->SetVantagePointValues(GenerationValuesComponent->VantageValues);
				newIsland->GenerationValuesComponent->SetFlagObjectiveValues(GenerationValuesComponent->FlagObjectiveValues);
				newIsland->GenerationValuesComponent->SetEnemyZoneObjectiveValues(GenerationValuesComponent->EnemyZoneObjectiveValues);
				newIsland->GenerationValuesComponent->SetPayloadObjectiveValues(GenerationValuesComponent->PayloadObjectiveValues);
				newIsland->GenerationValuesComponent->SetSurvivalZoneObjectiveValues(GenerationValuesComponent->SurvivalZoneObjectiveValues);

				if (CameraActor)
					newIsland->CameraActor = CameraActor;

				newIsland->OnObjectiveIslandObjectiveComplete.AddDynamic(this, &AIslandSpawner::GenerateNewIsland);

				EIslandObjectiveType islandObjective = EIslandObjectiveType::None;
				TArray<EIslandObjectiveType> ActiveFlags;
				for (int32 j = 0; j < 8; ++j)
				{
					uint8 FlagValue = 1 << j;
					if ((GenerationValuesComponent->IslandValues.Objectives & FlagValue) != 0 && FlagValue != (uint8)EIslandObjectiveType::None)
					{
						ActiveFlags.Add((EIslandObjectiveType)FlagValue);
					}
				}

				if (ActiveFlags.Num() > 0)
				{
					int32 RandomIndex = FMath::RandRange(0, ActiveFlags.Num() - 1);
					islandObjective = ActiveFlags[RandomIndex];
				}

				newIsland->InitializeValues(islandObjective);

				Islands.Add(newIsland);
			}


			for (int i = 0; i < NumIslands; i++)
			{

				int offsetX = 0;
				int offsetY = 0;

				for (int j = 0; j < i; j++)
				{
					float tileScale = GenerationValuesComponent->IslandValues.TileScale;

					offsetX = offsetX + ((Islands[j]->IslandWidth + 1) * tileScale * 4);
					offsetY = offsetY + ((Islands[j]->IslandDepth - 1) * tileScale * 4);
					//offsetY = FMath::FRandRange(0.0f, Islands[j]->IslandDepth) * GenerationValuesComponent->IslandValues.TileScale * 4;
				}

				Islands[i]->SetActorLocation(FVector(offsetX, offsetY, Islands[i]->GetActorLocation().Z));
			}

			/*
			for (int i = 0; i < NumIslands; i++)
			{
				if (i == NumIslands-1)
				{
					Islands[i]->SetFinalIsland(true);
				}

				Islands[i]->Generate();
			}
			*/

			if (CurrentSpawnedIsland == NumIslands - 1)
			{
				Islands[CurrentSpawnedIsland]->SetFinalIsland(true);
			}

			Islands[CurrentSpawnedIsland]->Generate();
		}
	}
}

void AIslandSpawner::GenerateNewIsland()
{
	CurrentSpawnedIsland++;
	if (Islands.IsValidIndex(CurrentSpawnedIsland))
	{
		if (CurrentSpawnedIsland == NumIslands - 1)
		{
			Islands[CurrentSpawnedIsland]->SetFinalIsland(true);
		}

		Islands[CurrentSpawnedIsland]->Generate();
	}
}
