// Fill out your copyright notice in the Description page of Project Settings.


#include "SaveFileListWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/Border.h"
#include "Components/TileView.h"
#include "Components/Image.h"
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
        TB_Name->SetText(FText::FromString(SaveFileData->GetName()));
        TB_MetaLevel->SetVisibility(ESlateVisibility::Visible);
        TB_MetaLevel->SetText(FText::FromString(FString::FromInt(SaveFileData->GetMetaLevel())));
        TB_CreateNew->SetText(FText::FromString(""));
        B_Upgrades->SetVisibility(ESlateVisibility::Visible);
        B_Edit->SetVisibility(ESlateVisibility::Visible);

        int TotalSeconds = SaveFileData->GetTimePlayed();

        int Hours = TotalSeconds / 3600;
        int Minutes = (TotalSeconds % 3600) / 60;
        int Seconds = TotalSeconds % 60;

        //Format as HH:MM:SS
        FString FormattedTime = FString::Printf(TEXT("%02dh %02dm %02ds"), Hours, Minutes, Seconds);

        TB_TimePlayed->SetText(FText::FromString(FormattedTime));

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

    ParentWidget->HighlightSelectedSlot(this);
}

void USaveFileListWidget::SetSelected(bool bSelected)
{
    FLinearColor DelectedColor = FLinearColor(0.062f, 0.049f, 0.031f, 1);
    FLinearColor MetaColor = FLinearColor(0.0f, 0.309f, 0.753f, 1);
    TB_Name->SetColorAndOpacity(bSelected ? FLinearColor(1,0.5,0,1) : FLinearColor::White);
    TB_TimePlayed->SetColorAndOpacity(bSelected ? FLinearColor(1, 0.5, 0, 1) : FLinearColor::White);
    TB_MetaLevel->SetColorAndOpacity(MetaColor);
    EditIcon->SetColorAndOpacity(bSelected ? FLinearColor(1, 0.5, 0, 1) : FLinearColor::White);
    B_SaveFile->SetBrushColor(bSelected ? FLinearColor(0, 0, 0, 1) : DelectedColor);
    B_Upgrades->SetBrushColor(bSelected ? FLinearColor(0, 0, 0, 1) : MetaColor);
    B_Edit->SetBrushColor(bSelected ? FLinearColor(0, 0, 0, 1) : DelectedColor);
    SaveFileButton->SetBackgroundColor(bSelected ? FLinearColor(1, 0.5, 0, 1) : FLinearColor(1, 1, 1, 1));
    EditButton->SetBackgroundColor(bSelected ? FLinearColor(1, 0.5, 0, 1) : FLinearColor(1, 1, 1, 1));
    Upgrades->SetBackgroundColor(bSelected ? FLinearColor(1, 0.5, 0, 1) : FLinearColor(1, 1, 1, 1));
}


void USaveFileListWidget::OnEditButtonClicked()
{
    if (!ParentWidget || !SaveFileData || SaveFileData->IsEmptySlot) return;

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
    if (!SaveFileData) return;

    SaveFileData->IsEmptySlot = false;

    SaveFileData->SaveData = USaveFileListData::MakeSaveData(PlayerName, SaveFileData->GetTimePlayed(), SaveFileData->GetMetaLevel(), SaveFileData->GetUniqueID());

    Setup(SaveFileData);
}


void USaveFileListWidget::FinalizeSaveCreation(const FString& PlayerName)
{
    SaveFileData->IsEmptySlot = false;
    SaveFileData->SaveData = USaveFileListData::MakeSaveData(PlayerName, 0, SaveFileData->GetMetaLevel(), SaveFileData->GetUniqueID());
    Setup(SaveFileData);

    SpawnEmptySlot();

    int NumRealSaves = 0;
    USaveFileListWidget* OnlyRealSaveWidget = nullptr;

    if (!ParentWidget) return;

    for (int i = 0; i < ParentWidget->SaveFiles->GetNumItems(); i++)
    {
        USaveFileListData* SlotData = Cast<USaveFileListData>(ParentWidget->SaveFiles->GetItemAt(i));
        if (SlotData && !SlotData->IsEmptySlot)
        {
            NumRealSaves++;

            OnlyRealSaveWidget = Cast<USaveFileListWidget>(ParentWidget->SaveFiles->GetEntryWidgetFromItem(SlotData));
        }
    }

    if (NumRealSaves == 1 && OnlyRealSaveWidget)
    {
        ParentWidget->HighlightSelectedSlot(OnlyRealSaveWidget);
    }
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



