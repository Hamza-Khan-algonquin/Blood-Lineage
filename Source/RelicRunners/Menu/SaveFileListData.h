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

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FGuid UniqueID = FGuid::NewGuid();

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
	void SetTimePlayed(float Time) { SaveData.TimePlayed = Time; }

	UFUNCTION(BlueprintCallable)
	FGuid GetUniqueID() { return SaveData.UniqueID; }

	UFUNCTION(BlueprintCallable)
	int GetNumUpgrades() { return SaveData.NumUpgrades; }
	UFUNCTION(BlueprintCallable, BlueprintPure)
    static FSaveData MakeSaveData(const FString& Name, float TimePlayed, int NumUpgrades, FGuid UniqueID);

	UFUNCTION(BlueprintCallable)
	void SetParentWidget(UModeSelectionWidget* TheParentWidget) { ParentWidget = TheParentWidget; }

	UFUNCTION(BlueprintCallable)
	void SetSaveData(FSaveData data) { SaveData = data; }

	UPROPERTY(BlueprintReadWrite)
	struct FSaveData SaveData;

	UPROPERTY(BlueprintReadWrite)
	class UModeSelectionWidget* ParentWidget;

	UPROPERTY()
	bool IsEmptySlot = true;
};
