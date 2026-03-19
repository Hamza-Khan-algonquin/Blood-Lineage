// Fill out your copyright notice in the Description page of Project Settings.


#include "KeybindingsListWidget.h"
#include "KeybindingsListData.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "SettingsWidget.h"

void UKeybindingsListWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (!B_BindButton) return;

    B_BindButton->OnClicked.AddDynamic(this, &UKeybindingsListWidget::OnBindButtonClicked);
}

void UKeybindingsListWidget::NativeOnListItemObjectSet(UObject* ListItemObject)
{
    UKeybindingsListData* SessionData = Cast<UKeybindingsListData>(ListItemObject);
    if (SessionData)
    {
        Setup(SessionData);
    }
    if (KeybindData)
    {
        KeybindData->BoundWidget = this;
    }
}

void UKeybindingsListWidget::Setup(UKeybindingsListData* InData)
{
    KeybindData = InData;

    if (KeybindData)
    {
        TB_Name->SetText(FText::FromString(KeybindData->Name));
        TB_Bind->SetText(FText::FromString(KeybindData->ReadableBind));
    }
}

void UKeybindingsListWidget::OnBindButtonClicked()
{
    USettingsWidget* Settings = Cast<USettingsWidget>(GetTypedOuter(USettingsWidget::StaticClass()));
    if (Settings)
    {
        Settings->StartRebinding(this);
    }

    //if (TB_Bind) TB_Bind->SetText(FText::FromString(TEXT("???")));
}

UKeybindingsListData* UKeybindingsListWidget::GetKeybindData() const
{
    return KeybindData;
}

void UKeybindingsListWidget::RefreshVisual()
{
    if (!KeybindData || !TB_Bind) return;

    FText DisplayText = KeybindData->Keybind.IsValid() ? FText::FromString(KeybindData->ReadableBind) : FText::FromString(TEXT("None"));
    TB_Bind->SetText(DisplayText);
}
