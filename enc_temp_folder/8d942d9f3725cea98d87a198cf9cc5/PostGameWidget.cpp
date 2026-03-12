// Fill out your copyright notice in the Description page of Project Settings.


#include "PostGameWidget.h"
#include "Components/VerticalBox.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"

void UPostGameWidget::UpdateScoreboard(const TArray<FPlayerPostGameData>& PlayersData)
{
    TArray<FPlayerColumn> Columns;
    Columns.Add({ Player1Box, TB_Username1, TB_MetaLevel1, TB_MetaNextLevel1, TB_MetaXP1, TB_LevelsEarned1, TB_EnemiesKilled1, TB_PlayerDeaths1, MetaExperienceBar1 });
    Columns.Add({ Player2Box, TB_Username2, TB_MetaLevel2, TB_MetaNextLevel2, TB_MetaXP2, TB_LevelsEarned2, TB_EnemiesKilled2, TB_PlayerDeaths2, MetaExperienceBar2 });
    Columns.Add({ Player3Box, TB_Username3, TB_MetaLevel3, TB_MetaNextLevel3, TB_MetaXP3, TB_LevelsEarned3, TB_EnemiesKilled3, TB_PlayerDeaths3, MetaExperienceBar3 });
    Columns.Add({ Player4Box, TB_Username4, TB_MetaLevel4, TB_MetaNextLevel4, TB_MetaXP4, TB_LevelsEarned4, TB_EnemiesKilled4, TB_PlayerDeaths4, MetaExperienceBar4 });

    for (int i = 0; i < Columns.Num(); i++)
    {
        FPlayerColumn& Col = Columns[i];

        if (!Col.Box) continue;

        if (PlayersData.IsValidIndex(i))
        {
            Col.Box->SetVisibility(ESlateVisibility::Visible);
            const FPlayerPostGameData& Data = PlayersData[i];

            if (Col.Username)       Col.Username->SetText(FText::FromString(Data.Username));
            if (Col.MetaLevel)      Col.MetaLevel->SetText(FText::AsNumber(Data.MetaLevel));
            if (Col.MetaNextLevel)  Col.MetaNextLevel->SetText(FText::AsNumber(Data.MetaLevel + 1));
            if (Col.MetaXP)         Col.MetaXP->SetText(FText::AsNumber(Data.MetaXP));
            if (Col.LevelsEarned)   Col.LevelsEarned->SetText(FText::AsNumber(Data.LevelsEarned));
            if (Col.EnemiesKilled)  Col.EnemiesKilled->SetText(FText::AsNumber(Data.EnemiesKilled));
            if (Col.PlayerDeaths)   Col.PlayerDeaths->SetText(FText::AsNumber(Data.Deaths));
            if (Col.XPBar && Data.MetaXPToNextLevel > 0)
            {
                Col.XPBar->SetPercent((float)Data.MetaXP / (float)Data.MetaXPToNextLevel);
            }
        }
        else
        {
            Col.Box->SetVisibility(ESlateVisibility::Collapsed);
        }
    }
}