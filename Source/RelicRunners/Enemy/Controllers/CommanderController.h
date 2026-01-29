// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "CommanderController.generated.h"

class AEnemyPawn;
class UAIPerceptionComponent;

UCLASS()
class RELICRUNNERS_API ACommanderController : public AAIController
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadWrite)
	TArray<TObjectPtr<AEnemyPawn>> ControlledEnemies;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int MaxNumberOfEnemies;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int CurrentLifeTimeNumberOfEnemies;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int MaxLifeTimeNumberOfEnemies;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SpawnRadius;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ActiveDistanceFromPlayer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AttackDistanceFromPlayer;

	UPROPERTY(VisibleAnywhere)
	bool EnemiesSetToLowestLOD;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<AEnemyPawn> PawnEnemyToSpawn;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	APawn* TargetPlayer;

protected:
	
public: 
	ACommanderController();
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable)
	void AddPawnEnemy(AEnemyPawn* enemy);

	UFUNCTION(BlueprintCallable)
	TArray<AEnemyPawn*> GetPawnEnemies();

	UFUNCTION(BlueprintCallable, CallInEditor)
	void SpawnPawnEnemy();
protected:
	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* InPawn) override;

	UFUNCTION(BlueprintCallable)
	void UpdateLODs(float closestDistance);
	UFUNCTION(BlueprintCallable)
	void UpdatePlayerTracking(float closestDistance, APawn* ClosetPlayer);
};
