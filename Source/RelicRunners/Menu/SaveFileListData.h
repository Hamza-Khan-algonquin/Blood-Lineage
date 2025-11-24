// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "SaveFileListData.generated.h"

USTRUCT(BlueprintType)
struct FSaveData
{
    GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	FString SavefileName;

	UPROPERTY(BlueprintReadWrite)
	float TimePlayed;

	UPROPERTY(BlueprintReadWrite)
	int NumUpgrades;
};

UCLASS()
class RELICRUNNERS_API USaveFileListData : public UObject
{
	GENERATED_BODY()
public:

	UFUNCTION(BlueprintCallable)
	FString GetName() { return SaveData.SavefileName; }
	UFUNCTION(BlueprintCallable)
	float GetTimePlayed() { return SaveData.TimePlayed; }
	UFUNCTION(BlueprintCallable)
	int GetNumUpgrades() { return SaveData.NumUpgrades; }
	UFUNCTION(BlueprintCallable, BlueprintPure)
    static FSaveData MakeSaveData(const FString& Name, float TimePlayed, int NumUpgrades);

	UPROPERTY(BlueprintReadWrite)
	struct FSaveData SaveData;

	UPROPERTY()
	class UModeSelectionWidget* ParentWidget;

	UPROPERTY()
	bool IsEmptySlot = true;
};
