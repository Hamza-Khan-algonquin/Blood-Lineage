// Fill out your copyright notice in the Description page of Project Settings.

#include "PauseMenu.h"
#include "Components/Button.h"
#include <RelicRunners/RelicRunnersCharacter.h>
#include <RelicRunners/PlayerController/RelicRunnersPlayerController.h>
#include <RelicRunners/Game/RelicRunnersGameInstance.h>
#include "MainMenuWidget.h"
#include <RelicRunners/RelicRunnersGameMode.h>
#include "RelicRunners/Menu/SettingsWidget.h"
#include "PauseMenuWidget.h"

void UPauseMenu::NativeConstruct()
{
    Super::NativeConstruct();

    if (ResumeButton)
    {
        ResumeButton->OnClicked.AddDynamic(this, &UPauseMenu::OnResumeButtonClicked);
    }

    if (SettingsButton)
    {
        SettingsButton->OnClicked.AddDynamic(this, &UPauseMenu::OnSettingsButtonClicked);
    }

    if (RecallButton)
    {
        RecallButton->OnClicked.AddDynamic(this, &UPauseMenu::OnRecallButtonClicked);
    }

    if (QuitButton)
    {
        QuitButton->OnClicked.AddDynamic(this, &UPauseMenu::OnQuitButtonClicked);
    }
}

void UPauseMenu::SetParentMenu(UPauseMenuWidget* InParentMenu)
{
    PauseParentMenu = InParentMenu;
}

void UPauseMenu::OnResumeButtonClicked()
{
    ARelicRunnersCharacter* Character = Cast<ARelicRunnersCharacter>(GetOwningPlayerPawn());
    if (Character)
    {
        Character->PauseUI();
    }
}

void UPauseMenu::OnRecallButtonClicked()
{

}

void UPauseMenu::OnSettingsButtonClicked()
{
    if (PauseParentMenu)
    {
        PauseParentMenu->ShowSettingsMenu();
    }
}

void UPauseMenu::OnQuitButtonClicked()
{
    UWorld* World = GetWorld();
    if (World)
    {
        URelicRunnersGameInstance* GameInstance = Cast<URelicRunnersGameInstance>(World->GetGameInstance());
        if (GameInstance)
        {
            GameInstance->BackToMainMenu();
        }
    }
}