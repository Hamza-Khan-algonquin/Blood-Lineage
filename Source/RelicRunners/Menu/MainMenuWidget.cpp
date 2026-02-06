
#include "MainMenuWidget.h"
#include "Components/WidgetSwitcher.h"
#include "JoinUserWidget.h"
#include "ModeSelectionWidget.h"
#include "UpgradesWidget.h"
#include "SettingsWidget.h"
#include "CreditsWidget.h"
#include <RelicRunners/Game/RelicRunnersGameInstance.h>

void UMainMenuWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (WidgetSwitcher)
    {
        ModeSelectionWidget = CreateWidget<UModeSelectionWidget>(GetWorld(), ModeSelectionUserWidgetClass);
        JoinGameWidget = CreateWidget<UJoinUserWidget>(GetWorld(), JoinGameUserWidgetClass);
        CreditsWidget = CreateWidget<UCreditsWidget>(GetWorld(), CreditsWidgetClass);
        UpgradesWidget = CreateWidget<UUpgradesWidget>(GetWorld(), UpgradesWidgetClass);
        SettingsWidget = CreateWidget<USettingsWidget>(GetWorld(), SettingsWidgetClass);

        if (ModeSelectionWidget && JoinGameWidget)
        {
            WidgetSwitcher->AddChild(ModeSelectionWidget);
            WidgetSwitcher->AddChild(JoinGameWidget);
            WidgetSwitcher->AddChild(CreditsWidget);
            WidgetSwitcher->AddChild(UpgradesWidget);
            WidgetSwitcher->AddChild(SettingsWidget);

            ModeSelectionWidget->SetParentMenu(this);
            JoinGameWidget->SetParentMenu(this);
            CreditsWidget->SetParentMenu(this);
            UpgradesWidget->SetParentMenu(this);
            SettingsWidget->SetParentMenu(this);

            // Start by showing ModeSelectionWidget
            ShowModeSelectionWidget();
        }
    }
}

void UMainMenuWidget::ShowModeSelectionWidget()
{
    if (WidgetSwitcher && ModeSelectionWidget)
    {
        WidgetSwitcher->SetActiveWidget(ModeSelectionWidget);
    }
}

void UMainMenuWidget::ShowSessionsWidget()
{
    if (WidgetSwitcher && JoinGameWidget)
    {
        WidgetSwitcher->SetActiveWidget(JoinGameWidget);
    }
}

void UMainMenuWidget::ShowUpgradesWidget()
{
    if (WidgetSwitcher && UpgradesWidget)
    {
        WidgetSwitcher->SetActiveWidget(UpgradesWidget);

        URelicRunnersGameInstance* GI = Cast<URelicRunnersGameInstance>(GetGameInstance());
        UpgradesWidget->InitializeUpgradeData(GI->SelectedSaveSlot);
    }
}

void UMainMenuWidget::ShowSettingsWidget()
{
    if (WidgetSwitcher && SettingsWidget)
    {
        WidgetSwitcher->SetActiveWidget(SettingsWidget);
    }
}

void UMainMenuWidget::ShowTutorialWidget()
{
    if (WidgetSwitcher && TutorialWidget)
    {
        WidgetSwitcher->SetActiveWidget(TutorialWidget);
    }
}

void UMainMenuWidget::ShowCreditsWidget()
{
    if (WidgetSwitcher && CreditsWidget)
    {
        WidgetSwitcher->SetActiveWidget(CreditsWidget);
    }
}
