// Fill out your copyright notice in the Description page of Project Settings.


#include "SaveFilePopupWidget.h"
#include "Components/EditableTextBox.h"
#include "Components/Button.h"
#include "SaveFileListWidget.h"

void USaveFilePopupWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (NameTextBox) NameTextBox->OnTextChanged.AddDynamic(this, &USaveFilePopupWidget::OnNameTextChanged);

    if (ConfirmButton) ConfirmButton->OnClicked.AddDynamic(this, &USaveFilePopupWidget::OnConfirmClicked);

    if (CancelButton) CancelButton->OnClicked.AddDynamic(this, &USaveFilePopupWidget::OnCancelClicked);
}

void USaveFilePopupWidget::OnConfirmClicked()
{
    FString EnteredName = NameTextBox->GetText().ToString();

    if (EnteredName.IsEmpty())
        return;

    if (TargetSlot)
    {
        TargetSlot->FinalizeSaveCreation(EnteredName);
        TargetSlot->ResetBorderVisibility();
    }

    RemoveFromParent();
}

void USaveFilePopupWidget::OnCancelClicked()
{
    RemoveFromParent();
    TargetSlot->ResetBorderVisibility();
}

void USaveFilePopupWidget::OnNameTextChanged(const FText& NewText)
{
    FString String = NewText.ToString();
    int MaxNameLength = 12;

    if (String.Len() > MaxNameLength)
    {
        String = String.Left(MaxNameLength);
        NameTextBox->SetText(FText::FromString(String));
    }
}