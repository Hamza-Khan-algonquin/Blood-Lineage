// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "IconLibrary.generated.h"

UCLASS(Blueprintable, BlueprintType)
class RELICRUNNERS_API UIconLibrary : public UDataAsset
{
	GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Icons")
    TMap<FName, UTexture2D*> IconMap;
};
