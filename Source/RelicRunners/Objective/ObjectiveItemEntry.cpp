// Fill out your copyright notice in the Description page of Project Settings.


#include "ObjectiveItemEntry.h"
#include "Components/WidgetSwitcher.h"
#include "Components/ProgressBar.h"
#include "Components/Border.h"
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
#include "Kismet/GameplayStatics.h"
#include "RelicRunners/Item/ItemStats.h"

void UObjectiveItemEntry::NativeOnListItemObjectSet(UObject* ListItemObject)
{
    IUserObjectListEntry::NativeOnListItemObjectSet(ListItemObject);

    if (UTitleData* TitleData = Cast<UTitleData>(ListItemObject)) 
    {
        WidgetSwitcher->SetActiveWidgetIndex(0);

        if (UTitleObjective* Widget = Cast<UTitleObjective>(WidgetSwitcher->GetActiveWidget()))
        {
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
    else if (UEnemiesData* EnemiesData = Cast<UEnemiesData>(ListItemObject))
    {
        WidgetSwitcher->SetActiveWidgetIndex(3);

        if (UEnemiesObjective* Widget = Cast<UEnemiesObjective>(WidgetSwitcher->GetActiveWidget()))
        {
            Widget->TB_Text->SetText(FText::AsNumber(EnemiesData->EnemiesRemaining));
        }
    }
    else if (UPayloadData* PayloadData = Cast<UPayloadData>(ListItemObject))
    {
        WidgetSwitcher->SetActiveWidgetIndex(1);
        if (UPayloadObjective* Widget = Cast<UPayloadObjective>(WidgetSwitcher->GetActiveWidget()))
        {
            if (PayloadData->Completed) 
            {
                Widget->B_Progress->SetVisibility(ESlateVisibility::Collapsed);
                Widget->I_Payload->SetBrushFromTexture(ItemStats::LoadIcon("Checkmark"));
                Widget->I_Payload->SetColorAndOpacity(FLinearColor::Green);
            }
            else 
            {
                Widget->I_Payload->SetBrushFromTexture(ItemStats::LoadIcon("PayloadIcon"));
                Widget->I_Payload->SetColorAndOpacity(FLinearColor::White);
                Widget->B_Progress->SetVisibility(ESlateVisibility::Visible);
                Widget->ProgressionBar->SetPercent(PayloadData->ProgressFraction);
                Widget->TB_Progression->SetText(FText::AsPercent(PayloadData->ProgressFraction));
            }
        }
    }
    else if (UFlagData* FlagData = Cast<UFlagData>(ListItemObject))
    {
        WidgetSwitcher->SetActiveWidgetIndex(2);
        if (UFlagsObjective* FlagWidget = Cast<UFlagsObjective>(WidgetSwitcher->GetActiveWidget()))
        {
            RefreshFlagVisuals(FlagData, FlagWidget);
        }
    }
    else if (UPandoraData* PandoraData = Cast<UPandoraData>(ListItemObject))
    {
        WidgetSwitcher->SetActiveWidgetIndex(4);
        if (UPandorasObjective* Widget = Cast<UPandorasObjective>(WidgetSwitcher->GetActiveWidget()))
        {
            if (PandoraData->Completed) 
            {
                Widget->B_Progress->SetVisibility(ESlateVisibility::Collapsed);
                Widget->I_Pandora->SetBrushFromTexture(ItemStats::LoadIcon("Checkmark"));
                Widget->I_Pandora->SetColorAndOpacity(FLinearColor::Green);
            }
            else 
            {
                Widget->I_Pandora->SetBrushFromTexture(ItemStats::LoadIcon("PandorasBoxOpenIcon"));
                Widget->I_Pandora->SetColorAndOpacity(FLinearColor::White);
                Widget->B_Progress->SetVisibility(ESlateVisibility::Visible);
                Widget->ProgressionBar->SetPercent(PandoraData->Percent);
                Widget->TB_Progression->SetText(FText::FromString(PandoraData->TimeRemainingText));
            }
        }
    }
}

void UObjectiveItemEntry::RefreshFlagVisuals(UFlagData* FlagData, UFlagsObjective* Widget)
{
    if (!Widget || !FlagData) return;

    if (FlagData->Completed)
    {
        Widget->FlagImage_0->SetVisibility(ESlateVisibility::Collapsed);
        Widget->FlagImage_2->SetVisibility(ESlateVisibility::Collapsed);
        Widget->FlagImage_1->SetVisibility(ESlateVisibility::Visible);
        Widget->FlagImage_1->SetBrushFromTexture(ItemStats::LoadIcon("Checkmark"));
        Widget->FlagImage_1->SetColorAndOpacity(FLinearColor::Green);
        return;
    }

    if (!FlagData->FlagUpIcon) FlagData->FlagUpIcon = ItemStats::LoadIcon("FlagUpIcon");
    if (!FlagData->FlagDownIcon) FlagData->FlagDownIcon = ItemStats::LoadIcon("FlagDownIcon");

    auto UpdateFlag = [&](bool bEnabled, bool bActive, UImage* Image) {
        if (bEnabled) {
            Image->SetVisibility(ESlateVisibility::Visible);
            Image->SetColorAndOpacity(FLinearColor::White);
            Image->SetBrushFromTexture(bActive ? FlagData->FlagUpIcon : FlagData->FlagDownIcon);
        }
        else {
            Image->SetVisibility(ESlateVisibility::Collapsed);
        }
        };

    UpdateFlag(FlagData->Flag1Enabled, FlagData->Flag1Active, Widget->FlagImage_0);
    UpdateFlag(FlagData->Flag2Enabled, FlagData->Flag2Active, Widget->FlagImage_1);
    UpdateFlag(FlagData->Flag3Enabled, FlagData->Flag3Active, Widget->FlagImage_2);
}

void UObjectiveItemEntry::CompleteObjective()
{
    PlayAnimation(Completion);
}