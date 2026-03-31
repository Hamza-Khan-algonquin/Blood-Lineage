// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "CommanderController.generated.h"

class AEnemyPawn;
class APawn;
class UAIPerceptionComponent;
class UFlockComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTargetChangeSignature, APawn*, target);



UCLASS()
class RELICRUNNERS_API ACommanderController : public AAIController
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadWrite)
	TArray<AEnemyPawn*> ControlledEnemies;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int MaxNumberOfEnemies;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int CurrentLifeTimeNumberOfEnemies;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int MaxLifeTimeNumberOfEnemies;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int EnemyToSpawnPerLoop;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TimeBetweenSpawnChecks;

	FTimerHandle SpawnTimer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SpawnRadius;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ActiveDistanceFromPlayer;

	UPROPERTY(VisibleAnywhere)
	bool EnemiesSetToLowestLOD;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<AEnemyPawn> PawnEnemyToSpawn;
	//Not attached to the commander but to the possed pawn
	TObjectPtr<UFlockComponent> PawnFlockComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<APawn> TargetPlayer;

	float TargetDistance;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TMap<TObjectPtr<APawn>, float> RemainingTimeToAttack;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TimeToAttack;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DistanceToTargetPlayer;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DistanceForBoidsToFocusePlayer;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool BoidShouldFocusePlayer;

	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FOnTargetChangeSignature TargetChange;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	//The Behavoir tree to use on being play
	UBehaviorTree* StartingBehaviorTree;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float SpawnDelayAfterWorldStart;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float MinDistanceFromPlayer;

	bool ShouldUpdateBoid;

protected:
	bool IsTargeting;
public: 
	ACommanderController();
	~ACommanderController();
	void EndPlay(const EEndPlayReason::Type EndPlayReason);
	void ResetController();
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
	UFUNCTION()
	bool InactiveCheck();

	UFUNCTION(BlueprintCallable)
	void AddPawnEnemy(AEnemyPawn* enemy);
	UFUNCTION(BlueprintCallable)
	void RemovePawnEnemy(AActor* enemy);

	UFUNCTION(BlueprintCallable)
	TArray<AEnemyPawn*> GetPawnEnemies();

	UFUNCTION(BlueprintCallable, CallInEditor)
	void SpawnPawnEnemy();
	void SpawmLoop();

	UFUNCTION(BlueprintCallable)
	void AddPlayerToTimeToAttack(APawn* player);
	UFUNCTION(BlueprintCallable)
	void RemovePlayerFromTimeToAttack(APawn* player);

	UFUNCTION(BlueprintCallable)
	APawn* GetCurrentTarget();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void UpdateTarget(APawn* newTarget);
protected:
	virtual void BeginPlay() override;

	virtual void OnPossess(APawn* InPawn) override;

	UFUNCTION(BlueprintCallable)
	void UpdateLODs(TArray<APawn*> players, FVector refernceLocation);
	UFUNCTION(BlueprintCallable)
	void AttackCheck(TArray<APawn*> players, float delta);
	UFUNCTION(BlueprintCallable)
	void UpdatePlayerTracking(float closestDistance, APawn* ClosetPlayer);
	//This should go into the new flock component but I am leaving it here from now to save time as I would have to make a new custom tick struct
	void UpdateBoidMovement(float delta);
};

