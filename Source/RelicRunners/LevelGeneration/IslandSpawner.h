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

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere)
	int NumIslands;

	UPROPERTY(EditAnywhere)
	TSubclassOf<class AIslandGenerationTest> IslandGenerationTestTemplate;

	UPROPERTY(EditAnywhere)
	TSubclassOf<class ALevelGenerator> LevelGeneratorTemplate;

	TArray<class AIslandGenerationTest*> Islands;

	TArray<class ALevelGenerator*> Level;

	UPROPERTY(EditAnywhere, meta = (ClampMin = "1", UIMin = "1"))
	uint8 WidthMin;

	UPROPERTY(EditAnywhere, meta = (ClampMin = "1", UIMin = "1"))
	uint8 WidthMax;

	UPROPERTY(EditAnywhere, meta = (ClampMin = "1", UIMin = "1"))
	uint8 DepthMin;

	UPROPERTY(EditAnywhere, meta = (ClampMin = "1", UIMin = "1"))
	uint8 DepthMax;

	UPROPERTY(EditAnywhere)
	float TileScale;


};
