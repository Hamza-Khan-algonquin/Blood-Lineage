
#include "MainMenuWidget.h"
#include "Components/WidgetSwitcher.h"
#include "JoinUserWidget.h"
#include "ModeSelectionWidget.h"
#include "UpgradesWidget.h"
#include "SettingsWidget.h"
#include "CreditsWidget.h"
#include "HowToPlay.h"
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
        HowToPlayWidget = CreateWidget<UHowToPlay>(GetWorld(), HowToPlayWidgetClass);

        if (ModeSelectionWidget && JoinGameWidget)
        {
            WidgetSwitcher->AddChild(ModeSelectionWidget);
            WidgetSwitcher->AddChild(JoinGameWidget);
            WidgetSwitcher->AddChild(CreditsWidget);
            WidgetSwitcher->AddChild(UpgradesWidget);
            WidgetSwitcher->AddChild(SettingsWidget);
            WidgetSwitcher->AddChild(HowToPlayWidget);

            ModeSelectionWidget->SetParentMenu(this);
            JoinGameWidget->SetParentMenu(this);
            CreditsWidget->SetParentMenu(this);
            UpgradesWidget->SetParentMenu(this);
            SettingsWidget->SetParentMenu(this);
            HowToPlayWidget->SetParentMenu(this);

            // Start by showing ModeSelectionWidget
            ShowModeSelectionWidget();
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
    }
}

void UMainMenuWidget::ShowTutorialWidget()
{
    PlayButtonClickSound();

    if (WidgetSwitcher && HowToPlayWidget)
    {
        WidgetSwitcher->SetActiveWidget(HowToPlayWidget);
        HowToPlayWidget->BlendToForgeCamera();
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
