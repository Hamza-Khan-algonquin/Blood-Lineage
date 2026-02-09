// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ObjectiveItemData.h"
#include "FlagData.generated.h"

UCLASS()
class RELICRUNNERS_API UFlagData : public UObjectiveItemData
{
	GENERATED_BODY()
public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool Flag1Active;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool Flag2Active;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool Flag3Active;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
	class UTexture2D* FlagUpIcon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
	class UTexture2D* FlagDownIcon;
};
