// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ObjectiveItemData.h"
#include "PayloadData.generated.h"

UCLASS()
class RELICRUNNERS_API UPayloadData : public UObjectiveItemData
{
	GENERATED_BODY()
public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ProgressFraction;

	UPROPERTY()
	TWeakObjectPtr<UUserWidget> AssociatedWidget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
	class UTexture2D* PayloadImage;
};
