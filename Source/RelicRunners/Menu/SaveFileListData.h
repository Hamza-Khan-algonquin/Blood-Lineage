// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "SaveFileListData.generated.h"

/**
 * 
 */
UCLASS()
class RELICRUNNERS_API USaveFileListData : public UObject
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadWrite)
	FString SavefileName;

	UPROPERTY(BlueprintReadWrite)
	float TimePlayed;

	UPROPERTY(BlueprintReadWrite)
	int NumUpgrades;

	class UModeSelectionWidget* ParentWidget;

	UPROPERTY()
	bool IsEmptySlot = true;
};
