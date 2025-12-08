// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "RelicRunners/ObjectPool/PoolActorInterface.h"
#include "RelicRunners/Enemy/EnemyData.h"
#include "EnemyPawn.generated.h"

UCLASS()
class RELICRUNNERS_API AEnemyPawn : public APawn, public IPoolActorInterface
{
	GENERATED_BODY()
public:
	//Delegate that is called when the enemy dies
	UPROPERTY(BlueprintAssignable, Category = "Enemy|Delegates")
	FEnemyCharacterDied OnDied;

	UPROPERTY(BlueprintAssignable, Category = "Enemy|Delegates")
	FEnemyCharacterDeactivated OnDeactivated;

public:
	// Sets default values for this pawn's properties
	AEnemyPawn();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
};
