// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ObjectiveBase.h"
#include "NewEnemyZone.generated.h"

/**
 * 
 */
UCLASS()
class RELICRUNNERS_API ANewEnemyZone : public AObjectiveBase
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ANewEnemyZone();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION(Server, Reliable)
	void Server_HandleEnemyWavesCompleted();

public:
	UPROPERTY(EditAnywhere, Category = "Default Values | Enemy Spawning Zone Actor");
	TSubclassOf<class ASpawnZone> SpawnZoneTemplate;

	UPROPERTY(Replicated)
	bool IsActive;
};
