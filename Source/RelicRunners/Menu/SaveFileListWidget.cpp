// Fill out your copyright notice in the Description page of Project Settings.


#include "SaveFileListWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/TileView.h"
#include "ModeSelectionWidget.h"
#include "SaveFileListData.h"

void USaveFileListWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (!SaveFileButton) return;

    SaveFileButton->OnClicked.AddDynamic(this, &USaveFileListWidget::OnSaveFileButtonClicked);
}

void USaveFileListWidget::NativeOnListItemObjectSet(UObject* ListItemObject)
{
    SaveFileData = Cast<USaveFileListData>(ListItemObject);
    if (!SaveFileData) return;

    ParentWidget = SaveFileData->ParentWidget;

    if (SaveFileData->bIsEmptySlot)
    {
        TB_CreateNew->SetText(FText::FromString("+"));
        TB_CreateNew->SetVisibility(ESlateVisibility::Visible);

        TB_Name->SetText(FText::FromString(""));
        TB_NumUpgrades->SetVisibility(ESlateVisibility::Collapsed);
        TB_TimePlayed->SetText(FText::FromString("Create New"));
        CreateNew = "+";
    }
    else
    {
        TB_CreateNew->SetVisibility(ESlateVisibility::Collapsed);
        CreateNew = "";

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
    }
}

void USaveFileListWidget::OnSaveFileButtonClicked()
{
    if (!SaveFileData) return;

    // How many total items are in the list?
    int CurrentCount = ParentWidget ? ParentWidget->SaveFiles->GetNumItems() : 0;
    int MaxSlots = 7;

    if (SaveFileData->bIsEmptySlot)
    {
        // Convert this slot into a real save
        SaveFileData->bIsEmptySlot = false;
        SaveFileData->SavefileName = "New Save";
        SaveFileData->NumUpgrades = 0;
        SaveFileData->TimePlayed = 0;

        Setup(SaveFileData);

        if (ParentWidget && CurrentCount < MaxSlots)
        {
            USaveFileListData* NewEmptySlot = NewObject<USaveFileListData>();
            NewEmptySlot->bIsEmptySlot = true;
            NewEmptySlot->ParentWidget = ParentWidget;

            ParentWidget->SaveFiles->AddItem(NewEmptySlot);
        }
    }
    else
    {
        // Handle selecting an existing save
    }
}

