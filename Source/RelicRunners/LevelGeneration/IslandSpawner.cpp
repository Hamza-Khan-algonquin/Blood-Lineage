// Fill out your copyright notice in the Description page of Project Settings.


#include "IslandSpawner.h"
#include "IslandGenerationTest.h"
#include "LevelGenerator.h"
#include "IslandSpawningValuesComponent.h"
#include "PayloadActor.h"
#include "RelicRunners/Game/RelicRunnersGameInstance.h"
#include "RelicRunners/PlayerController/RelicRunnersPlayerController.h"
#include "Net/UnrealNetwork.h"

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

void AIslandSpawner::UpdateControllerZonesUI(int CurrentZone, int MaxZones)
{
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		if (ARelicRunnersPlayerController* TargetPC = Cast<ARelicRunnersPlayerController>(It->Get()))
		{
			TargetPC->Client_UpdateZones(CurrentZone, MaxZones);
		}
	}
}

void AIslandSpawner::UpdateControllerStageUI(int StageNumber)
{
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		if (ARelicRunnersPlayerController* TargetPC = Cast<ARelicRunnersPlayerController>(It->Get()))
		{
			TargetPC->Client_UpdateStage(StageNumber);
		}
	}
}

// Called when the game starts or when spawned
void AIslandSpawner::BeginPlay()
{
	Super::BeginPlay();

	URelicRunnersGameInstance* gameInstance = Cast<URelicRunnersGameInstance>(GetGameInstance());

	if (HasAuthority())
	{
		if (IslandGenerationTestTemplate != nullptr)
		{
			if (gameInstance)
			{
				gameInstance->StageNumber += 1;
				//GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Cyan, FString::Printf(TEXT("Server Stage Number: %i"), gameInstance->StageNumber));
			}

			for (int i = 0; i < NumIslands; i++)
			{
				AIslandGenerationTest* newIsland = GetWorld()->SpawnActor<AIslandGenerationTest>(IslandGenerationTestTemplate, FVector::ZeroVector, FRotator::ZeroRotator);

				newIsland->IslandIndex = i;

				newIsland->GenerationValuesComponent->SetIslandValues(GenerationValuesComponent->IslandValues);
				newIsland->GenerationValuesComponent->SetIslandStartAndEndValues(GenerationValuesComponent->IslandStartEnd);
				newIsland->GenerationValuesComponent->SetFloorMeshes(GenerationValuesComponent->FloorMeshes);
				newIsland->GenerationValuesComponent->SetObstacleValues(GenerationValuesComponent->ObstacleValues);
				newIsland->GenerationValuesComponent->SetVantagePointValues(GenerationValuesComponent->VantageValues);
				newIsland->GenerationValuesComponent->SetSafeSpotValues(GenerationValuesComponent->SafeSpotValues);
				newIsland->GenerationValuesComponent->SetFlagObjectiveValues(GenerationValuesComponent->FlagObjectiveValues);
				newIsland->GenerationValuesComponent->SetPayloadObjectiveValues(GenerationValuesComponent->PayloadObjectiveValues);
				newIsland->GenerationValuesComponent->SetSurvivalZoneObjectiveValues(GenerationValuesComponent->SurvivalZoneObjectiveValues);
				newIsland->SetNumStages(NumStages);

				if (CameraActor)
					newIsland->CameraActor = CameraActor;

				newIsland->OnObjectiveIslandObjectiveComplete.AddDynamic(this, &AIslandSpawner::GenerateNewIsland);
				newIsland->OnSetCameraChange.AddDynamic(this, &AIslandSpawner::ChangeIslandCamera);
				newIsland->OnSetCameraChangeBack.AddDynamic(this, &AIslandSpawner::ChangeIslandVisibility);

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

			if (CurrentSpawnedIsland == NumIslands - 1)
			{
				Islands[CurrentSpawnedIsland]->SetFinalIsland(true);
			}

			Islands[CurrentSpawnedIsland]->Generate();
			Islands[CurrentSpawnedIsland]->UpdateCameraPos();
			Islands[CurrentSpawnedIsland]->ActivateIsland();

			for (int i = 1; i < NumIslands; i++)
			{
				if (i == NumIslands - 1)
				{
					Islands[i]->SetFinalIsland(true);
				}

				Islands[i]->Generate();
			}
		}
	}
	
	UpdateControllerStageUI(gameInstance->StageNumber);
	UpdateControllerZonesUI(CurrentSpawnedIsland, NumIslands);
}

void AIslandSpawner::OnRep_IslandArrayChange()
{
	if (Islands.IsEmpty())
		return;

	for (AIslandGenerationTest* isl : Islands)
	{
		isl->OnObjectiveIslandObjectiveComplete.AddDynamic(this, &AIslandSpawner::GenerateNewIsland);
		isl->OnSetCameraChange.AddDynamic(this, &AIslandSpawner::ChangeIslandCamera);
		isl->OnSetCameraChangeBack.AddDynamic(this, &AIslandSpawner::ChangeIslandVisibility);

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
		Islands[CurrentSpawnedIsland]->ActivateIsland();
	}
	UpdateControllerZonesUI(CurrentSpawnedIsland, NumIslands);
}

void AIslandSpawner::ChangeIslandCamera(int islandIndex)
{
	if (Islands.IsValidIndex(islandIndex+1))
	{
		Islands[islandIndex+1]->UpdateCameraPos();
	}
}

void AIslandSpawner::ChangeIslandVisibility(int islandIndex)
{
	for (int i = 0; i < NumIslands;i++)
	{
		if (i == islandIndex - 1 ||
			i == islandIndex ||
			i == islandIndex + 1 && Islands[i]->IsActivated)
		{
			Islands[i]->SetIslandVisibility(true);
		}
		else
		{
			Islands[i]->SetIslandVisibility(false);
		}

	}
}

void AIslandSpawner::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AIslandSpawner, Islands);
}