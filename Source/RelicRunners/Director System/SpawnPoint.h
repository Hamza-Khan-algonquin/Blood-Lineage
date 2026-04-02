/***************************************************************************************
Class Name:  ASpawnPoint
Author:      Thomas Johnson
Description: This Class is incharge of spawn enemies when told to by the director in a radius.
Date Last Changed:        2025/10/27
****************************************************************************************/
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SpawningCard.h"
#include "SpawnPoint.generated.h"

class ADirector;

//Justin added this
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnEnemyWavesComplete);

UCLASS()
class RELICRUNNERS_API ASpawnPoint : public AActor
{
	GENERATED_BODY()
protected:
	UPROPERTY(EditAnywhere)
	bool SpawnInfinitely;

	//The allowance to use for spawn on each wave
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wave Properties", meta = (AllowPrivateAccess = "true", ToolTip = "The allowance to use for spawn on each wave"))
	float SpawnAllowance;
	//The current allowance left to use for spawning
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wave Properties", meta = (AllowPrivateAccess = "true", ToolTip = "The current allowance left to use for spawning"))
	float CurrentAllowance;
	//The number of waves of enemies this spawn point will spawn
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wave Properties", meta = (AllowPrivateAccess = "true", ToolTip = "The number of waves of enemies this spawn point will spawn"))
	int NumOfWaves;
	//The Current Wave
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wave Properties", meta = (AllowPrivateAccess = "true", ToolTip = "The Current Wave"))
	int CurrentWaves;
	//The cards to use when deciding what to spawn
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy Pool", meta = (AllowPrivateAccess = "true", ToolTip = "The cards to use when deciding what to spawn"))
	TArray<USpawnCard*> SpawningCards;
	//The enemies currently in the scene
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy In Scene", meta = (AllowPrivateAccess = "true", ToolTip = "The enemies currently in the scene"))
	TArray<APawn*> SpawnedEnemies;
	

	//I had to put this here because of a weird bug where it would not let me resign element into it after removing those same elements on a previous run of the spawn enemies otherwise it would be a local variable
	TArray<USpawnCard*> PickableCards;

	//The maximum number of enemies that can spawn at the spawn point at the same time
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Point Properties", meta = (AllowPrivateAccess = "true", ClampMin = "1", ToolTip = "The maximum number of enemies that can spawn at the spawn point at the same time"))
	int MaxNumOfEnemies;
	//The raduis at which enemies can spawn around the spawn point
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Point Properties", meta = (AllowPrivateAccess = "true", ClampMin = "10", ToolTip = "The raduis at which enemies can spawn around the spawn point"))
	float Radius;
	//How far away the enemies have to be in oder to spawn
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Point Properties", meta = (AllowPrivateAccess = "true", ToolTip = "How far away the enemies have to be in oder to spawn"))
	float SpawnSeparationDistance;
	//The height at which to start the raycast that looks for sold ground
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Point Properties", meta = (AllowPrivateAccess = "true", ClampMin = "10", ToolTip = "The height at which to start the raycast that looks for sold ground"))
	float HeightToCheckFrom;
	//The depth at which to stop the raycast that looks for sold ground
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Point Properties", meta = (AllowPrivateAccess = "true", ClampMax = "-10", ToolTip = "The depth at which to stop the raycast that looks for sold ground"))
	float DepthToCheckFrom;
	//Just to make it easier to move around
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true", ToolTip = "Just to make it easier to move around"))
	class UStaticMeshComponent* Mesh;
	//The Director
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Director", meta = (AllowPrivateAccess = "true", ToolTip = "The Director"))
	TObjectPtr<ADirector> Director;

public:	
	// Sets default values for this actor's properties
	ASpawnPoint();

	//A method to check if the spawn point is maxed out on how many enemies it can spawn
	UFUNCTION(meta = (ToolTip = "A method to check if the spawn point is maxed out on how many enemies it can spawn", ShortToolTip = "Checks if spawner is at max capacity"))
	bool IsAtMaxCapacity();

	//Get a random position based on the spawn point properties to use for spawning
	UFUNCTION(meta = (ToolTip = "Get a random position based on the spawn point properties to use for spawning", ShortToolTip = "Get random point to spawn"))
	virtual FVector GetRandomPosition();

	//A method to try and spawn enemies at random points using GetRandomPosition()
	UFUNCTION(meta = (ToolTip = "A method to try and spawn enemies at random points using GetRandomPosition()", ShortToolTip = "A method to spawn enemies"))
	void SpawnEnemies();

	//A Method that will remove a provided enemy from SpawnedEnemies. It is meant to be called only by APawn when a OnDestroy event happens
	UFUNCTION(meta = (ToolTip = "A Method that will remove a provided enemy from SpawnedEnemies. It is meant to be called only by APawn when a OnDestroy event happens", ShortToolTip = "Remove a enemy from SpawnedEnemies"))
	void RemoveEnemy(AActor* enemy);

	//A method to check if the spawn point has finished all its waves
	UFUNCTION(BlueprintCallable, meta = (ToolTip = "A method to check if the spawn point has finished all its waves", ShortToolTip = "Check if it is finished"))
	bool FinishedAllWaves();

	//Justin added this
	UPROPERTY(BlueprintAssignable)
	FOnEnemyWavesComplete OnEnemyWavesComplete;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
};
