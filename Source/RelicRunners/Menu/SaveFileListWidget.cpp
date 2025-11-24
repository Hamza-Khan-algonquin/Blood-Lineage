// Fill out your copyright notice in the Description page of Project Settings.


#include "SaveFileListWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/Border.h"
#include "Components/TileView.h"
#include "ModeSelectionWidget.h"
#include "SaveFileListData.h"
#include <RelicRunners/Game/RelicRunnersGameInstance.h>
#include "EditSaveFilePopupWidget.h"

void USaveFileListWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (SaveFileButton) SaveFileButton->OnClicked.AddDynamic(this, &USaveFileListWidget::OnSaveFileButtonClicked);

    if (EditButton) EditButton->OnClicked.AddDynamic(this, &USaveFileListWidget::OnEditButtonClicked);
}

void USaveFileListWidget::NativeOnListItemObjectSet(UObject* ListItemObject)
{
    SaveFileData = Cast<USaveFileListData>(ListItemObject);
    if (!SaveFileData) return;

    ParentWidget = SaveFileData->ParentWidget;

    if (SaveFileData->IsEmptySlot)
    {
        TB_CreateNew->SetText(FText::FromString("+"));
        TB_CreateNew->SetVisibility(ESlateVisibility::Visible);

        TB_Name->SetText(FText::FromString(""));
        TB_TimePlayed->SetText(FText::FromString("Create New"));

        B_Upgrades->SetVisibility(ESlateVisibility::Collapsed);
        B_Edit->SetVisibility(ESlateVisibility::Collapsed);
    }
    else
    {
        TB_CreateNew->SetVisibility(ESlateVisibility::Collapsed);

        Setup(SaveFileData);
    }
}


void USaveFileListWidget::Setup(USaveFileListData* InData)
{
    SaveFileData = InData;

    if (SaveFileData)
    {
        TB_Name->SetText(FText::FromString(SaveFileData->SavefileName));
        TB_NumUpgrades->SetVisibility(ESlateVisibility::Visible);
        TB_NumUpgrades->SetText(FText::FromString(FString::FromInt(SaveFileData->NumUpgrades)));
        TB_CreateNew->SetText(FText::FromString(""));
        TB_TimePlayed->SetText(FText::FromString(FString::FromInt(SaveFileData->TimePlayed)));
        B_Upgrades->SetVisibility(ESlateVisibility::Visible);
        B_Edit->SetVisibility(ESlateVisibility::Visible);
    }
}

void USaveFileListWidget::OnSaveFileButtonClicked()
{
    if (!SaveFileData) return;

    if (SaveFileData->IsEmptySlot)
    {
        if (ParentWidget)
        {
            ParentWidget->OpenSaveFilePopupWidget(this);
            B_SaveFile->SetVisibility(ESlateVisibility::Collapsed);
        }
        return;
    }
}

void USaveFileListWidget::OnEditButtonClicked()
{
    if (!ParentWidget || !SaveFileData || SaveFileData->IsEmptySlot)
        return;

    if (ParentWidget->EditPopupClass)
    {
        UEditSaveFilePopupWidget* Popup = CreateWidget<UEditSaveFilePopupWidget>(GetWorld(), ParentWidget->EditPopupClass);
        Popup->TargetSlot = this;  
        Popup->AddToViewport();
    }
}


void USaveFileListWidget::ResetBorderVisibility()
{
    B_SaveFile->SetVisibility(ESlateVisibility::Visible);
}

void USaveFileListWidget::FinalizeEdit(const FString& PlayerName)
{
    SaveFileData->IsEmptySlot = false;
    SaveFileData->SavefileName = PlayerName;
    SaveFileData->NumUpgrades = 0;

    Setup(SaveFileData);
}

void USaveFileListWidget::FinalizeSaveCreation(const FString& PlayerName)
{
    SaveFileData->IsEmptySlot = false;
    SaveFileData->SavefileName = PlayerName;
    SaveFileData->NumUpgrades = 0;

    Setup(SaveFileData);

    SpawnEmptySlot();
}

void USaveFileListWidget::SpawnEmptySlot()
{
    if (!ParentWidget) return;

    URelicRunnersGameInstance* GI = GetWorld()->GetGameInstance<URelicRunnersGameInstance>();
    if (!GI) return;

    int MaxSlots = 7;

    if (GI->SaveSlots.Num() >= MaxSlots) return;

    for (USaveFileListData* save : GI->SaveSlots)
    {
        if (save && save->IsEmptySlot) return;
    }

    USaveFileListData* NewEmpty = NewObject<USaveFileListData>();
    NewEmpty->IsEmptySlot = true;
    NewEmpty->ParentWidget = ParentWidget;

    GI->SaveSlots.Add(NewEmpty);
    ParentWidget->SaveFiles->AddItem(NewEmpty);
}



