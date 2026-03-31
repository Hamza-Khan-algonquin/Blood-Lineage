// Fill out your copyright notice in the Description page of Project Settings.


#include "HowToPlay.h"
#include "MainMenuWidget.h"
#include "Components/Button.h"

void UHowToPlay::NativeConstruct()
{
	Super::NativeConstruct();

	if (BackButton)
	{
		BackButton->OnClicked.AddDynamic(this, &UHowToPlay::BackButtonClicked);
	}
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
