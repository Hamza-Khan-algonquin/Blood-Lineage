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

    SetSelected(false);

    if (SaveFileData->IsEmptySlot)
    {
        TB_CreateNew->SetText(FText::FromString("+"));
        TB_CreateNew->SetVisibility(ESlateVisibility::Visible);

        TB_Name->SetText(FText::FromString(""));
        TB_TimePlayed->SetText(FText::FromString("Create New"));
        TB_TimePlayed->SetVisibility(ESlateVisibility::Visible);

        B_Upgrades->SetVisibility(ESlateVisibility::Collapsed);
        B_Edit->SetVisibility(ESlateVisibility::Collapsed);

        TB_CreateNew->SetColorAndOpacity(FLinearColor(0.82f, 0.82f, 0.82f, 1.0f)); 
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
        TB_CreateNew->SetVisibility(ESlateVisibility::Collapsed);

        B_Upgrades->SetVisibility(ESlateVisibility::Visible);
        B_Edit->SetVisibility(ESlateVisibility::Visible);
        TB_TimePlayed->SetVisibility(ESlateVisibility::Hidden);
        int TotalSeconds = SaveFileData->GetTimePlayed();
        int Hours = TotalSeconds / 3600;
        int Minutes = (TotalSeconds % 3600) / 60;
        int Seconds = TotalSeconds % 60;

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
    //Greek Theme Palette
    FLinearColor OlympicGold = FLinearColor(1.0f, 0.84f, 0.0f, 1.0f);   // #FFD700
    FLinearColor MarbleWhite = FLinearColor(0.82f, 0.82f, 0.82f, 1.0f); // #D1D1D1
    FLinearColor DeepObsidian = FLinearColor(0.02f, 0.02f, 0.02f, 1.0f); // Very dark background
    FLinearColor Terracotta = FLinearColor(0.12f, 0.08f, 0.05f, 1.0f);  // Subtle warm tint

    FLinearColor ActiveAccent = bSelected ? OlympicGold : MarbleWhite;
    FLinearColor ActiveBG = bSelected ? Terracotta : DeepObsidian;

    //Text & Icons
    TB_Name->SetColorAndOpacity(ActiveAccent);
    //TB_TimePlayed->SetColorAndOpacity(ActiveAccent);
    EditText->SetColorAndOpacity(ActiveAccent);
    //Font Outlines (Gold glow when selected)
    FSlateFontInfo NameFont = TB_Name->GetFont();
    NameFont.OutlineSettings.OutlineColor = bSelected ? OlympicGold : FLinearColor::Black;
    TB_Name->SetFont(NameFont);

    //FSlateFontInfo TimeFont = TB_TimePlayed->GetFont();
    //TimeFont.OutlineSettings.OutlineColor = bSelected ? OlympicGold : FLinearColor::Black;
    //TB_TimePlayed->SetFont(TimeFont);
    TB_TimePlayed->SetVisibility(ESlateVisibility::Hidden);

    //Button Backgrounds
    B_SaveFile->SetBrushColor(ActiveBG);
    B_Upgrades->SetBrushColor(ActiveBG);
    B_Edit->SetBrushColor(ActiveBG);

    //Button Borders/Frames
    SaveFileButton->SetBackgroundColor(ActiveAccent);
    EditButton->SetBackgroundColor(ActiveAccent);
    Upgrades->SetBackgroundColor(ActiveAccent);
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

    SaveFileData->SaveData = USaveFileListData::MakeSaveData(PlayerName, SaveFileData->GetTimePlayed(), SaveFileData->GetMetaLevel(), SaveFileData->GetUniqueID(), SaveFileData->GetMetaExp(),
        SaveFileData->GetMetaExpToLevel(), SaveFileData->GetMetaPoints(), SaveFileData->GetMetaHealth(), SaveFileData->GetMetaDefence(), SaveFileData-> GetMetaLuck(), SaveFileData->GetMetaStrength(),
        SaveFileData->GetMetaAgility(), SaveFileData->GetMetaIntelligence(), SaveFileData->GetMetaStartingLevel(), SaveFileData->GetMetaPotionEfficiency(), SaveFileData->GetMetaResourceRegen());

    Setup(SaveFileData);
}


void USaveFileListWidget::FinalizeSaveCreation(const FString& PlayerName)
{
    SaveFileData->IsEmptySlot = false;
    SaveFileData->SaveData = USaveFileListData::MakeSaveData(PlayerName, SaveFileData->GetTimePlayed(), SaveFileData->GetMetaLevel(), SaveFileData->GetUniqueID(), SaveFileData->GetMetaExp(),
        SaveFileData->GetMetaExpToLevel(), SaveFileData->GetMetaPoints(), SaveFileData->GetMetaHealth(), SaveFileData->GetMetaDefence(), SaveFileData->GetMetaLuck(), SaveFileData->GetMetaStrength(),
        SaveFileData->GetMetaAgility(), SaveFileData->GetMetaIntelligence(), SaveFileData->GetMetaStartingLevel(), SaveFileData->GetMetaPotionEfficiency(), SaveFileData->GetMetaResourceRegen());
    Setup(SaveFileData);
    SaveFileData->InitializeNewSave();

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



