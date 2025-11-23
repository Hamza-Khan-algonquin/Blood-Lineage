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
    ParentWidget = SaveFileData->ParentWidget;
    CreateNew = "+";
    TB_CreateNew->SetText(FText::FromString(CreateNew));
    TB_Name->SetText(FText::FromString(""));
    TB_NumUpgrades->SetText(FText::FromString(""));
    TB_NumUpgrades->SetVisibility(ESlateVisibility::Collapsed);
    TB_TimePlayed->SetText(FText::FromString(""));
}

void USaveFileListWidget::Setup(USaveFileListData* InData)
{
    SaveFileData = InData;

    if (SaveFileData)
    {
        TB_Name->SetText(FText::FromString(SaveFileData->SavefileName));
        TB_NumUpgrades->SetVisibility(ESlateVisibility::Visible);
        TB_NumUpgrades->SetText(FText::FromString(FString::FromInt(SaveFileData->NumUpgrades)));
        TB_TimePlayed->SetText(FText::FromString(FString::FromInt(SaveFileData->TimePlayed)));
    }
}

void USaveFileListWidget::OnSaveFileButtonClicked()
{
    if (CreateNew == "+")
    {
        //Uninitialized, so create new
        if (SaveFileData)
        {
            Setup(SaveFileData);
        }
        USaveFileListData* NewEmptySlot = NewObject<USaveFileListData>();
        if (ParentWidget)
        {
            ParentWidget->SaveFiles->AddItem(NewEmptySlot);
        }
    }
    else
    {
        //Select the savefile
    }
}