// Fill out your copyright notice in the Description page of Project Settings.


#include "PostGameWidget.h"
#include "Components/VerticalBox.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include <RelicRunners/Menu/SaveFileListData.h>

void UPostGameWidget::UpdateScoreboard(const TArray<FPlayerPostGameData>& PlayersData)
{
    CachedStats = PlayersData;
    AnimatingXP.Empty();
    AnimatingLevel.Empty();
    AnimationTimer = 0.0f;
    bIsAnimating = true;

    BarScaleTimers.Empty();
    BarScaleTimers.Init(0.0f, PlayersData.Num());

    TArray<FPlayerColumn> Columns;
    Columns.Add({ Player1Box, TB_Username1, TB_MetaLevel1, TB_MetaNextLevel1, TB_MetaXP1, TB_LevelsEarned1, TB_EnemiesKilled1, TB_PlayerDeaths1, MetaExperienceBar1 });
    Columns.Add({ Player2Box, TB_Username2, TB_MetaLevel2, TB_MetaNextLevel2, TB_MetaXP2, TB_LevelsEarned2, TB_EnemiesKilled2, TB_PlayerDeaths2, MetaExperienceBar2 });
    Columns.Add({ Player3Box, TB_Username3, TB_MetaLevel3, TB_MetaNextLevel3, TB_MetaXP3, TB_LevelsEarned3, TB_EnemiesKilled3, TB_PlayerDeaths3, MetaExperienceBar3 });
    Columns.Add({ Player4Box, TB_Username4, TB_MetaLevel4, TB_MetaNextLevel4, TB_MetaXP4, TB_LevelsEarned4, TB_EnemiesKilled4, TB_PlayerDeaths4, MetaExperienceBar4 });

    for (int i = 0; i < Columns.Num(); i++)
    {
        if (!PlayersData.IsValidIndex(i))
        {
            if (Columns[i].Box) Columns[i].Box->SetVisibility(ESlateVisibility::Collapsed);
            continue;
        }

        if (Columns[i].XPBar) Columns[i].XPBar->SetRenderScale(FVector2D(1.0f, 1.0f));

        const FPlayerPostGameData& Data = PlayersData[i];

        AnimatingXP.Add((float)Data.MetaXP);
        AnimatingLevel.Add(Data.MetaLevel);

        if (Columns[i].Box)           Columns[i].Box->SetVisibility(ESlateVisibility::Visible);
        if (Columns[i].Username)      Columns[i].Username->SetText(FText::FromString(Data.Username));
        if (Columns[i].EnemiesKilled)  Columns[i].EnemiesKilled->SetText(FText::AsNumber(Data.EnemiesKilled));
        if (Columns[i].PlayerDeaths)   Columns[i].PlayerDeaths->SetText(FText::AsNumber(Data.Deaths));
        if (Columns[i].MetaXPText)    Columns[i].MetaXPText->SetText(FText::AsNumber(Data.MetaXPGainInRun));
        if (Columns[i].LevelsEarned)  Columns[i].LevelsEarned->SetText(FText::AsNumber(Data.LevelsEarned));
        if (Columns[i].MetaLevel)     Columns[i].MetaLevel->SetText(FText::AsNumber(Data.MetaLevel));
        if (Columns[i].MetaNextLevel)  Columns[i].MetaNextLevel->SetText(FText::AsNumber(Data.MetaLevel + 1));
        if (Columns[i].XPBar && Data.MetaXPToNextLevel > 0)
        {
            float StartPercent = (float)Data.MetaXP / (float)Data.MetaXPToNextLevel;
            Columns[i].XPBar->SetPercent(FMath::Clamp(StartPercent, 0.0f, 1.0f));
        }
    }
}

void UPostGameWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    if (!bIsAnimating) return;

    AnimationTimer += InDeltaTime;

    TArray<FPlayerColumn> Columns;
    Columns.Add({ Player1Box, TB_Username1, TB_MetaLevel1, TB_MetaNextLevel1, TB_MetaXP1, TB_LevelsEarned1, TB_EnemiesKilled1, TB_PlayerDeaths1, MetaExperienceBar1 });
    Columns.Add({ Player2Box, TB_Username2, TB_MetaLevel2, TB_MetaNextLevel2, TB_MetaXP2, TB_LevelsEarned2, TB_EnemiesKilled2, TB_PlayerDeaths2, MetaExperienceBar2 });
    Columns.Add({ Player3Box, TB_Username3, TB_MetaLevel3, TB_MetaNextLevel3, TB_MetaXP3, TB_LevelsEarned3, TB_EnemiesKilled3, TB_PlayerDeaths3, MetaExperienceBar3 });
    Columns.Add({ Player4Box, TB_Username4, TB_MetaLevel4, TB_MetaNextLevel4, TB_MetaXP4, TB_LevelsEarned4, TB_EnemiesKilled4, TB_PlayerDeaths4, MetaExperienceBar4 });

    for (int i = 0; i < CachedStats.Num(); i++)
    {
        if (!Columns.IsValidIndex(i)) break;

        const FPlayerPostGameData& Data = CachedStats[i];
        FPlayerColumn& Col = Columns[i];

        float CurrentMax = USaveFileListData::GetRequiredMetaXP(AnimatingLevel[i]);
        float XpIncrement = (float)Data.MetaXPGainInRun * (InDeltaTime / MaxAnimTime);
        AnimatingXP[i] += XpIncrement;

        while (AnimatingXP[i] >= CurrentMax)
        {
            AnimatingXP[i] -= CurrentMax;
            AnimatingLevel[i]++;

            if (BarScaleTimers.IsValidIndex(i))
            {
                BarScaleTimers[i] = MaxPopTime;
            }

            CurrentMax = USaveFileListData::GetRequiredMetaXP(AnimatingLevel[i]);

            if (Col.MetaLevel) Col.MetaLevel->SetText(FText::AsNumber(AnimatingLevel[i]));
            if (Col.MetaNextLevel) Col.MetaNextLevel->SetText(FText::AsNumber(AnimatingLevel[i] + 1));
        }
        if (Col.XPBar)
        {
            Col.XPBar->SetPercent(FMath::Clamp(AnimatingXP[i] / CurrentMax, 0.0f, 1.0f));

            if (BarScaleTimers.IsValidIndex(i) && BarScaleTimers[i] > 0.f)
            {
                BarScaleTimers[i] -= InDeltaTime;

                float Alpha = BarScaleTimers[i] / MaxPopTime;
                float CurvedAlpha = Alpha * Alpha;
                float TargetScale = FMath::Lerp(1.0f, 1.2f, CurvedAlpha);
                Col.XPBar->SetRenderScale(FVector2D(TargetScale, TargetScale));
            }
            else
            {
                Col.XPBar->SetRenderScale(FVector2D(1.0f, 1.0f));
            }
        }
    }

    if (AnimationTimer >= MaxAnimTime)
    {
        bIsAnimating = false;
    }
}