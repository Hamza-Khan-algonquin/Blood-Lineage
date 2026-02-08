// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ObjectiveItemData.h"
#include "PandoraData.generated.h"

UCLASS()
class RELICRUNNERS_API UPandoraData : public UObjectiveItemData
{
	GENERATED_BODY()
public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Percent;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString TimeRemainingText;
};
