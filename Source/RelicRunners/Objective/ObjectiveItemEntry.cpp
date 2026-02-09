// Fill out your copyright notice in the Description page of Project Settings.


#include "ObjectiveItemEntry.h"
#include "Components/WidgetSwitcher.h"
#include "Components/ProgressBar.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "TitleData.h"
#include "PayloadData.h"
#include "PandoraData.h"
#include "EnemiesData.h"
#include "FlagData.h"
#include "PayloadObjective.h"
#include "FlagsObjective.h"
#include "EnemiesObjective.h"
#include "PandorasObjective.h"
#include "TitleObjective.h"
#include "RelicRunners/Item/ItemStats.h"

void UObjectiveItemEntry::NativeOnListItemObjectSet(UObject* ListItemObject)
{
    IUserObjectListEntry::NativeOnListItemObjectSet(ListItemObject);

    if (UTitleData* TitleData = Cast<UTitleData>(ListItemObject)) 
    {
        WidgetSwitcher->SetActiveWidgetIndex(0);

        if (UTitleObjective* Widget = Cast<UTitleObjective>(WidgetSwitcher->GetActiveWidget()))
        {
            Widget->TB_Title->SetText(TitleData->LevelName);

            FString ZoneString;
            if (TitleData->TotalZones == 0)
            {
                ZoneString = FString::Printf(TEXT(""), TitleData->CurrentZones, TitleData->TotalZones);
            }
            else
            {
                ZoneString = FString::Printf(TEXT("%d / %d Zones"), TitleData->CurrentZones, TitleData->TotalZones);
            }
            Widget->TB_Text->SetText(FText::FromString(ZoneString));
        }
    }
    else if (UPayloadData* PayloadData = Cast<UPayloadData>(ListItemObject)) 
    {
        WidgetSwitcher->SetActiveWidgetIndex(1);

        //Update visuals with data
        if (UPayloadObjective* Widget = Cast<UPayloadObjective>(WidgetSwitcher->GetActiveWidget())) 
        {
            Widget->ProgressionBar->SetPercent(PayloadData->ProgressFraction);
            Widget->TB_Progression->SetText(FText::AsPercent(PayloadData->ProgressFraction));
        }
    }
    else if (UFlagData* FlagData = Cast<UFlagData>(ListItemObject)) 
    {
        WidgetSwitcher->SetActiveWidgetIndex(2);

        FlagData->FlagDownIcon = ItemStats::LoadIcon("FlagDownIcon");
        FlagData->FlagUpIcon = ItemStats::LoadIcon("FlagUpIcon");

        //Update visuals with data
        if (UFlagsObjective* Widget = Cast<UFlagsObjective>(WidgetSwitcher->GetActiveWidget()))
        {
            UTexture2D* Texture;
            Texture = FlagData->Flag1Active ? FlagData->FlagUpIcon : FlagData->FlagDownIcon;
            Widget->FlagImage_0->SetBrushFromTexture(Texture);
            Texture = FlagData->Flag2Active ? FlagData->FlagUpIcon : FlagData->FlagDownIcon;
            Widget->FlagImage_1->SetBrushFromTexture(Texture);
            Texture = FlagData->Flag3Active ? FlagData->FlagUpIcon : FlagData->FlagDownIcon;
            Widget->FlagImage_2->SetBrushFromTexture(Texture);
        }
    }
    else if (UEnemiesData* EnemiesData = Cast<UEnemiesData>(ListItemObject)) 
    {
        WidgetSwitcher->SetActiveWidgetIndex(3);

        //Update visuals with data
        if (UEnemiesObjective* Widget = Cast<UEnemiesObjective>(WidgetSwitcher->GetActiveWidget())) 
        {
            Widget->TB_Text->SetText(FText::AsNumber(EnemiesData->EnemiesRemaining));
        }
    }
    else if (UPandoraData* PandoraData = Cast<UPandoraData>(ListItemObject)) 
    {
        WidgetSwitcher->SetActiveWidgetIndex(4);

        //Update visuals with data
        if (UPandorasObjective* Widget = Cast<UPandorasObjective>(WidgetSwitcher->GetActiveWidget())) 
        {
            Widget->ProgressionBar->SetPercent(PandoraData->Percent);
            Widget->TB_Progression->SetText(FText::FromString(PandoraData->TimeRemainingText));
        }
    }
}
