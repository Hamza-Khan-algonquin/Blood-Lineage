// Fill out your copyright notice in the Description page of Project Settings.


#include "UpgradesWidget.h"
#include "MainMenuWidget.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "RelicRunners/Item/ItemStats.h"

void UUpgradesWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (BackButton)
	{
		BackButton->OnClicked.AddDynamic(this, &UUpgradesWidget::BackButtonClicked);
	}

	if (IconHP) IconHP->SetColorAndOpacity(ItemStats::GetIconColor("Health"));
	if (IconDEF) IconDEF->SetColorAndOpacity(ItemStats::GetIconColor("Defence"));
	if (IconLUCK) IconLUCK->SetColorAndOpacity(ItemStats::GetIconColor("Luck"));
	if (IconSTR) IconSTR->SetColorAndOpacity(ItemStats::GetIconColor("Strength"));
	if (IconAGI) IconAGI->SetColorAndOpacity(ItemStats::GetIconColor("Agility"));
	if (IconINT) IconINT->SetColorAndOpacity(ItemStats::GetIconColor("Intelligence"));
}

void UUpgradesWidget::SetParentMenu(UMainMenuWidget* InParentMenu)
{
	ParentMenu = InParentMenu;
}

void UUpgradesWidget::BackButtonClicked()
{
	if (ParentMenu)
	{
		ParentMenu->ShowModeSelectionWidget();
	}
}