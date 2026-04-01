// Fill out your copyright notice in the Description page of Project Settings.


#include "HowToPlay.h"
#include "MainMenuWidget.h"
#include "Components/Button.h"
#include "Components/WidgetSwitcher.h"

void UHowToPlay::NativeConstruct()
{
	Super::NativeConstruct();

	if (BackButton)
	{
		BackButton->OnClicked.AddDynamic(this, &UHowToPlay::BackButtonClicked);
	}

	if (B_Items)      B_Items->OnClicked.AddDynamic(this, &UHowToPlay::OnItemsClicked);
	if (B_Objectives) B_Objectives->OnClicked.AddDynamic(this, &UHowToPlay::OnObjectivesClicked);
	if (B_Upgrades)   B_Upgrades->OnClicked.AddDynamic(this, &UHowToPlay::OnUpgradesClicked);
	if (B_Enemies)    B_Enemies->OnClicked.AddDynamic(this, &UHowToPlay::OnEnemiesClicked);
	if (B_Classes)    B_Classes->OnClicked.AddDynamic(this, &UHowToPlay::OnClassesClicked);

	SwitchToPage(0);
}

void UHowToPlay::SetParentMenu(UMainMenuWidget* InParentMenu)
{
	ParentMenu = InParentMenu;
}

void UHowToPlay::BackButtonClicked()
{
	if (ParentMenu)
	{
		ParentMenu->ShowModeSelectionWidget();
	}
}

void UHowToPlay::OnItemsClicked() { SwitchToPage(0); }
void UHowToPlay::OnObjectivesClicked() { SwitchToPage(1); }
void UHowToPlay::OnUpgradesClicked() { SwitchToPage(2); }
void UHowToPlay::OnEnemiesClicked() { SwitchToPage(3); }
void UHowToPlay::OnClassesClicked() { SwitchToPage(4); }

void UHowToPlay::SwitchToPage(int Index)
{
	if (ContentSwitcher)
	{
		ContentSwitcher->SetActiveWidgetIndex(Index);
	}
}