// Fill out your copyright notice in the Description page of Project Settings.


#include "EditSaveFilePopupWidget.h"
#include "Components/Button.h"
#include "SaveFileListWidget.h"
#include "SaveFileListData.h"
#include "ModeSelectionWidget.h"
#include "Components/TileView.h"
#include "Components/EditableTextbox.h"
#include <RelicRunners/Game/RelicRunnersGameInstance.h>

void UEditSaveFilePopupWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (NameTextBox) NameTextBox->OnTextChanged.AddDynamic(this, &UEditSaveFilePopupWidget::OnNameTextChanged);
    if (DeleteButton) DeleteButton->OnClicked.AddDynamic(this, &UEditSaveFilePopupWidget::OnDeleteClicked);
    if (CancelButton) CancelButton->OnClicked.AddDynamic(this, &UEditSaveFilePopupWidget::OnCancelClicked);
    if (ConfirmButton) ConfirmButton->OnClicked.AddDynamic(this, &UEditSaveFilePopupWidget::OnConfirmClicked);
}

void UEditSaveFilePopupWidget::OnDeleteClicked()
{
    if (!TargetSlot || !TargetSlot->ParentWidget || !TargetSlot->SaveFileData)return;

    URelicRunnersGameInstance* GI = GetWorld()->GetGameInstance<URelicRunnersGameInstance>();

    GI->SaveSlots.Remove(TargetSlot->SaveFileData);

    UObject* ObjectData = Cast<UObject>(TargetSlot->SaveFileData);
    TargetSlot->ParentWidget->SaveFiles->RemoveItem(ObjectData);

    //If the deleted slot was the selected one, auto select a new one
    bool SelectedNew = false;
    if (GI->SelectedSaveSlot == TargetSlot->SaveFileData)
    {
        for (int i = 0; i < TargetSlot->ParentWidget->SaveFiles->GetNumItems(); i++)
        {
            USaveFileListData* SlotData = Cast<USaveFileListData>(TargetSlot->ParentWidget->SaveFiles->GetItemAt(i));
            if (SlotData && !SlotData->IsEmptySlot)
            {
                if (USaveFileListWidget* Widget = Cast<USaveFileListWidget>(TargetSlot->ParentWidget->SaveFiles->GetEntryWidgetFromItem(SlotData)))
                {
                    TargetSlot->ParentWidget->HighlightSelectedSlot(Widget); 
                    SelectedNew = true;
                }
                break;
            }
        }
    }

    if (!SelectedNew)
    {
        GI->SelectedSaveSlot = nullptr;
        GI->PlayerName = "";
    }

    TargetSlot->SpawnEmptySlot();
    TargetSlot->ParentWidget->ShowUpgradesAvailable();
    RemoveFromParent();
}

void UEditSaveFilePopupWidget::OnCancelClicked()
{
    RemoveFromParent();
}

void UEditSaveFilePopupWidget::OnNameTextChanged(const FText& NewText)
{
    FString String = NewText.ToString();
    int MaxNameLength = 12;

    if (String.Len() > MaxNameLength)
    {
        String = String.Left(MaxNameLength);
        NameTextBox->SetText(FText::FromString(String));
    }
}

void UEditSaveFilePopupWidget::OnConfirmClicked()
{
    FString EnteredName = NameTextBox->GetText().ToString();

    if (EnteredName.IsEmpty())
        return;

    if (TargetSlot)
    {
        TargetSlot->FinalizeEdit(EnteredName);
        TargetSlot->ResetBorderVisibility();
    }

    RemoveFromParent();
}