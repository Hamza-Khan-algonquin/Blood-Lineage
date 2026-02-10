// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ObjectiveItemData.h"
#include "TitleData.generated.h"

UCLASS(BlueprintType)
class RELICRUNNERS_API UTitleData : public UObjectiveItemData
{
    GENERATED_BODY()
public:

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    int CurrentZones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    int TotalZones;
};
