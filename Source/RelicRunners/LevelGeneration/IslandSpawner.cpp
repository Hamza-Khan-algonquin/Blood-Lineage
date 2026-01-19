// Fill out your copyright notice in the Description page of Project Settings.


#include "IslandSpawner.h"
#include "IslandGenerationTest.h"

// Sets default values
AIslandSpawner::AIslandSpawner():
	NumIslands(0),
	IslandGenerationTestTemplate(nullptr),
	WidthMin(0),
	WidthMax(0),
	DepthMin(0),
	DepthMax(0)

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

			newIsland->SetSizeRange(WidthMin, WidthMax, DepthMin, DepthMax);
			newIsland->SetTileScale(TileScale);

			Islands.Add(newIsland);
		}

		for (int i = 0; i < NumIslands; i++)
		{
			float offset = 0;

			for (int j = 0; j < i; j++)
			{
				offset = offset + (Islands[j]->MapSizeRange.WidthFinal * Islands[j]->TileScale);
			}

			Islands[i]->SetActorLocation(FVector(offset, Islands[i]->GetActorLocation().Y, Islands[i]->GetActorLocation().Z));
		}

		for (int i = 0; i < NumIslands; i++)
		{
			Islands[i]->Generate();
		}
	}
}