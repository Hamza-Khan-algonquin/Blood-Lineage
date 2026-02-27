// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "IslandSpawner.generated.h"

UCLASS()
class RELICRUNNERS_API AIslandSpawner : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AIslandSpawner();

	void UpdateControllerZonesUI(int CurrentZone, int MaxZones);

	void UpdateControllerStageUI(int StageNumber);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GenerationValues")
	class UIslandSpawningValuesComponent* GenerationValuesComponent;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, Category = "GenerationValues")
	TSubclassOf<class AIslandGenerationTest> IslandGenerationTestTemplate;

	TArray<class AIslandGenerationTest*> Islands;

	UPROPERTY(EditAnywhere, Category = "GenerationValues")
	class AActor* CameraActor;

	UPROPERTY(EditAnywhere, Category = "GenerationValues", meta = (ClampMin = "1", UIMin = "1"))
	uint8 NumIslands;

	UFUNCTION()
	void GenerateNewIsland();

	int CurrentSpawnedIsland;

	UFUNCTION()
	void ChangeIslandCamera(int islandIndex);
	UFUNCTION()
	void ChangeIslandVisibility(int islandIndex);
};