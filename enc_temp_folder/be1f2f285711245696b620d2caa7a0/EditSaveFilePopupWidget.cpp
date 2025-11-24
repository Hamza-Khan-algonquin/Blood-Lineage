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
    DeleteButton->OnClicked.AddDynamic(this, &UEditSaveFilePopupWidget::OnDeleteClicked);
    CancelButton->OnClicked.AddDynamic(this, &UEditSaveFilePopupWidget::OnCancelClicked);
    if (ConfirmButton) ConfirmButton->OnClicked.AddDynamic(this, &UEditSaveFilePopupWidget::OnConfirmClicked);
}

void UEditSaveFilePopupWidget::OnDeleteClicked()
{
    if (!TargetSlot || !TargetSlot->ParentWidget || !TargetSlot->SaveFileData)
        return;

    URelicRunnersGameInstance* GI = GetWorld()->GetGameInstance<URelicRunnersGameInstance>();

    GI->SaveSlots.Remove(TargetSlot->SaveFileData);

    UObject* ObjectData = Cast<UObject>(TargetSlot->SaveFileData);
    TargetSlot->ParentWidget->SaveFiles->RemoveItem(ObjectData);


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