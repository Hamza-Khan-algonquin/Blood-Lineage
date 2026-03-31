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

	FTimerHandle UIDelayHandle;
	FTimerHandle UIZoneDelayHandle;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GenerationValues")
	class UIslandSpawningValuesComponent* GenerationValuesComponent;
	void SyncObjectiveToSinglePlayer(class ARelicRunnersPlayerController* TargetPC);
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, Category = "GenerationValues")
	TSubclassOf<class AIslandGenerationTest> IslandGenerationTestTemplate;

	UFUNCTION()
	void OnRep_IslandArrayChange();



	UPROPERTY(ReplicatedUsing = OnRep_IslandArrayChange)
	TArray<class AIslandGenerationTest*> Islands;

	UPROPERTY(EditAnywhere, Category = "GenerationValues")
	class AActor* CameraActor;

	UPROPERTY(EditAnywhere, Category = "GenerationValues", meta = (ClampMin = "1", UIMin = "1"))
	uint8 NumIslands;

	UPROPERTY(EditAnywhere, Category = "GenerationValues", meta = (ClampMin = "1", UIMin = "1"))
	uint8 NumStages;

	UPROPERTY(EditAnywhere, Category = "GenerationValues")
	bool IsObjectivesRandom = false;

	UFUNCTION()
	void GenerateNewIsland();

	int CurrentSpawnedIsland;

	UFUNCTION()
	void ChangeIslandCamera(int islandIndex);
	UFUNCTION()
	void ChangeIslandVisibility(int islandIndex);
};