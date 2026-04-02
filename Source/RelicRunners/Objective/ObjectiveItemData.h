// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "ObjectiveItemData.generated.h"

UCLASS(BlueprintType)
class RELICRUNNERS_API UObjectiveItemData : public UObject
{
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    FText ObjectiveTitle;

    //This array is how the TreeView knows when a Title element has children
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    TArray<UObjectiveItemData*> SubObjectives;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool Completed = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    class UTexture2D* CompletedImage;

    UPROPERTY()
    bool bCompletionSoundPlayed = false;
};
