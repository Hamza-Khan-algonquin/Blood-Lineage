// Fill out your copyright notice in the Description page of Project Settings.


#include "PauseMenuWidget.h"
#include "PauseMenu.h"
#include "SettingsWidget.h"
#include "Keybinds.h"
#include "Components/WidgetSwitcher.h"
#include <RelicRunners/Game/RelicRunnersGameInstance.h>

void UPauseMenuWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (WidgetSwitcher)
    {
        PauseMenu = CreateWidget<UPauseMenu>(GetWorld(), PauseMenuClass);
        SettingsMenu = CreateWidget<USettingsWidget>(GetWorld(), SettingsMenuClass);

        WidgetSwitcher->AddChild(PauseMenu);
        WidgetSwitcher->AddChild(SettingsMenu);

        PauseMenu->SetParentMenu(this);
        SettingsMenu->SetParentMenu(this);

        ShowPauseMenu();
    }
}

void UPauseMenuWidget::ShowPauseMenu()
{
    if (WidgetSwitcher)
    {
        WidgetSwitcher->SetActiveWidget(PauseMenu);
    }
}

void UPauseMenuWidget::ShowSettingsMenu()
{
    if (WidgetSwitcher)
    {
        WidgetSwitcher->SetActiveWidget(SettingsMenu);
    }
}
