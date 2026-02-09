// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ObjectiveItemData.h"
#include "EnemiesData.generated.h"

UCLASS()
class RELICRUNNERS_API UEnemiesData : public UObjectiveItemData
{
	GENERATED_BODY()
public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int EnemiesRemaining;
};
