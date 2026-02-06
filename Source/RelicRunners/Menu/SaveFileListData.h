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
	FString SavefileName = "";

	UPROPERTY(BlueprintReadWrite)
	float TimePlayed = 0.0f;

	//Meta
	UPROPERTY(BlueprintReadWrite)
	int MetaLevel = 1;
	UPROPERTY(BlueprintReadWrite)
	float MetaExperience = 0;
	UPROPERTY(BlueprintReadWrite)
	float MetaExperienceToLevel = 50;
	UPROPERTY(BlueprintReadWrite)
	int MetaPoints = 1;

	//Meta Stats
	UPROPERTY(BlueprintReadWrite)
	int MetaHealth = 0;
	UPROPERTY(BlueprintReadWrite)
	int MetaDefence = 0;
	UPROPERTY(BlueprintReadWrite)
	int MetaLuck = 0;
	UPROPERTY(BlueprintReadWrite)
	int MetaStrength = 0;
	UPROPERTY(BlueprintReadWrite)
	int MetaAgility = 0;
	UPROPERTY(BlueprintReadWrite)
	int MetaIntelligence = 0;
	UPROPERTY(BlueprintReadWrite)
	int MetaStartingLevel = 0;
	UPROPERTY(BlueprintReadWrite)
	int MetaPotionEfficiency = 0;
	UPROPERTY(BlueprintReadWrite)
	int MetaResourceRegen = 0;

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
	int GetMetaLevel() { return SaveData.MetaLevel; }
	UFUNCTION(BlueprintCallable, BlueprintPure)
    static FSaveData MakeSaveData(const FString& Name, float TimePlayed, int MetaLevel, FGuid UniqueID);

	UFUNCTION(BlueprintCallable)
	void SetParentWidget(UModeSelectionWidget* TheParentWidget) { ParentWidget = TheParentWidget; }

	UFUNCTION(BlueprintCallable)
	void SetSaveData(FSaveData data) { SaveData = data; }

	UPROPERTY(BlueprintReadWrite)
	struct FSaveData SaveData;

	UPROPERTY(BlueprintReadWrite)
	class UModeSelectionWidget* ParentWidget = nullptr;

	UPROPERTY()
	bool IsEmptySlot = true;
};
