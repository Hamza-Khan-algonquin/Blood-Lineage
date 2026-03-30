
#include "MainMenuWidget.h"
#include "Components/WidgetSwitcher.h"
#include "JoinUserWidget.h"
#include "ModeSelectionWidget.h"
#include "UpgradesWidget.h"
#include "SettingsWidget.h"
#include "CreditsWidget.h"
#include "Kismet/GameplayStatics.h"
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
            ApplyColorblindStyleToChildren();
        }
    }

    
}

void UMainMenuWidget::ShowModeSelectionWidget()
{
    PlayButtonClickSound();

    if (WidgetSwitcher && ModeSelectionWidget)
    {
        WidgetSwitcher->SetActiveWidget(ModeSelectionWidget);
    }
}

void UMainMenuWidget::ShowSessionsWidget()
{
    PlayButtonClickSound();

    if (WidgetSwitcher && JoinGameWidget)
    {
        WidgetSwitcher->SetActiveWidget(JoinGameWidget);
    }
}

void UMainMenuWidget::ShowUpgradesWidget()
{
    PlayButtonClickSound();

    if (WidgetSwitcher && UpgradesWidget)
    {
        WidgetSwitcher->SetActiveWidget(UpgradesWidget);

        URelicRunnersGameInstance* GI = Cast<URelicRunnersGameInstance>(GetGameInstance());
        UpgradesWidget->InitializeUpgradeData(GI->SelectedSaveSlot);
    }
}

void UMainMenuWidget::ShowSettingsWidget()
{
    PlayButtonClickSound();

    if (WidgetSwitcher && SettingsWidget)
    {
        WidgetSwitcher->SetActiveWidget(SettingsWidget);
        SettingsWidget->RefreshFOVUI();
        SettingsWidget->RefreshBrightnessUI();
    }
}

void UMainMenuWidget::ShowTutorialWidget()
{
    PlayButtonClickSound();

    if (WidgetSwitcher && TutorialWidget)
    {
        WidgetSwitcher->SetActiveWidget(TutorialWidget);
    }
}

void UMainMenuWidget::ShowCreditsWidget()
{
    PlayButtonClickSound();

    if (WidgetSwitcher && CreditsWidget)
    {
        WidgetSwitcher->SetActiveWidget(CreditsWidget);
    }
}

void UMainMenuWidget::PlayButtonClickSound()
{   
    if (!ButtonClickSound) return;

    if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
    {
        UGameplayStatics::PlaySound2D(PC, ButtonClickSound);
    }
}

void UMainMenuWidget::ApplyColorblindStyleToChildren()
{
    if (ModeSelectionWidget)
    {
        ModeSelectionWidget->ApplyColorblindStyle();
    }

    if (JoinGameWidget)
    {
        JoinGameWidget->ApplyColorblindStyle();
    }

    if (CreditsWidget)
    {
        CreditsWidget->ApplyColorblindStyle();
    }

    if (UpgradesWidget)
    {
        UpgradesWidget->ApplyColorblindStyle();
    }

    if (SettingsWidget)
    {
        SettingsWidget->ApplyColorblindStyle();
    }
}
