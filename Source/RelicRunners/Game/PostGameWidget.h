// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PostGameWidget.generated.h"

USTRUCT(BlueprintType)
struct FPlayerPostGameData
{
    GENERATED_BODY()

    UPROPERTY() FString Username = "";
    UPROPERTY() int MetaLevel = 0;
    UPROPERTY() int MetaXP = 0;
    UPROPERTY() int MetaXPToNextLevel = 0;

    UPROPERTY() int MetaXPGainInRun = 0;
    UPROPERTY() int LevelsEarned = 0;
    UPROPERTY() int EnemiesKilled = 0;
    UPROPERTY() int Deaths = 0;
};

UCLASS()
class RELICRUNNERS_API UPostGameWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    // Call this from your PlayerController after creating the widget
    UFUNCTION(BlueprintCallable, Category = "PostGame")
    void UpdateScoreboard(const TArray<FPlayerPostGameData>& PlayersData);

protected:

    struct FPlayerColumn
    {
        class UVerticalBox* Box;
        class UTextBlock* Username;
        class UTextBlock* MetaLevel;
        class UTextBlock* MetaNextLevel;
        class UTextBlock* MetaXP;
        class UTextBlock* LevelsEarned;
        class UTextBlock* EnemiesKilled;
        class UTextBlock* PlayerDeaths;
        class UProgressBar* XPBar;
    };

    UPROPERTY(meta = (BindWidget)) class UVerticalBox* Player1Box;
    UPROPERTY(meta = (BindWidget)) class UVerticalBox* Player2Box;
    UPROPERTY(meta = (BindWidget)) class UVerticalBox* Player3Box;
    UPROPERTY(meta = (BindWidget)) class UVerticalBox* Player4Box;

    UPROPERTY(meta = (BindWidget)) class UTextBlock* TB_Username1;
    UPROPERTY(meta = (BindWidget)) class UTextBlock* TB_MetaLevel1;
    UPROPERTY(meta = (BindWidget)) class UTextBlock* TB_MetaNextLevel1;
    UPROPERTY(meta = (BindWidget)) class UTextBlock* TB_MetaXP1;
    UPROPERTY(meta = (BindWidget)) class UTextBlock* TB_LevelsEarned1;
    UPROPERTY(meta = (BindWidget)) class UTextBlock* TB_EnemiesKilled1;
    UPROPERTY(meta = (BindWidget)) class UTextBlock* TB_PlayerDeaths1;
    UPROPERTY(meta = (BindWidget)) class UProgressBar* MetaExperienceBar1;

    UPROPERTY(meta = (BindWidget)) class UTextBlock* TB_Username2;
    UPROPERTY(meta = (BindWidget)) class UTextBlock* TB_MetaLevel2;
    UPROPERTY(meta = (BindWidget)) class UTextBlock* TB_MetaNextLevel2;
    UPROPERTY(meta = (BindWidget)) class UTextBlock* TB_MetaXP2;
    UPROPERTY(meta = (BindWidget)) class UTextBlock* TB_LevelsEarned2;
    UPROPERTY(meta = (BindWidget)) class UTextBlock* TB_EnemiesKilled2;
    UPROPERTY(meta = (BindWidget)) class UTextBlock* TB_PlayerDeaths2;
    UPROPERTY(meta = (BindWidget)) class UProgressBar* MetaExperienceBar2;

    UPROPERTY(meta = (BindWidget)) class UTextBlock* TB_Username3;
    UPROPERTY(meta = (BindWidget)) class UTextBlock* TB_MetaLevel3;
    UPROPERTY(meta = (BindWidget)) class UTextBlock* TB_MetaNextLevel3;
    UPROPERTY(meta = (BindWidget)) class UTextBlock* TB_MetaXP3;
    UPROPERTY(meta = (BindWidget)) class UTextBlock* TB_LevelsEarned3;
    UPROPERTY(meta = (BindWidget)) class UTextBlock* TB_EnemiesKilled3;
    UPROPERTY(meta = (BindWidget)) class UTextBlock* TB_PlayerDeaths3;
    UPROPERTY(meta = (BindWidget)) class UProgressBar* MetaExperienceBar3;

    UPROPERTY(meta = (BindWidget)) class UTextBlock* TB_Username4;
    UPROPERTY(meta = (BindWidget)) class UTextBlock* TB_MetaLevel4;
    UPROPERTY(meta = (BindWidget)) class UTextBlock* TB_MetaNextLevel4;
    UPROPERTY(meta = (BindWidget)) class UTextBlock* TB_MetaXP4;
    UPROPERTY(meta = (BindWidget)) class UTextBlock* TB_LevelsEarned4;
    UPROPERTY(meta = (BindWidget)) class UTextBlock* TB_EnemiesKilled4;
    UPROPERTY(meta = (BindWidget)) class UTextBlock* TB_PlayerDeaths4;
    UPROPERTY(meta = (BindWidget)) class UProgressBar* MetaExperienceBar4;
};