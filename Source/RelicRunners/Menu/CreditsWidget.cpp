// Fill out your copyright notice in the Description page of Project Settings.


#include "CreditsWidget.h"
#include "MainMenuWidget.h"
#include "RelicRunners/Game/RelicRunnersGameInstance.h"
#include "Components/TextBlock.h"
#include "Components/Border.h"
#include "Components/Image.h"
#include "Components/Button.h"

void UCreditsWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (BackButton)
	{
		BackButton->OnClicked.AddDynamic(this, &UCreditsWidget::BackButtonClicked);
	}

	ApplyColorblindStyle();
}

void UCreditsWidget::SetParentMenu(UMainMenuWidget* InParentMenu)
{
	ParentMenu = InParentMenu;
}

void UCreditsWidget::BackButtonClicked()
{
	if (ParentMenu)
	{
		ParentMenu->ShowModeSelectionWidget();
	}
}
void UCreditsWidget::ApplyColorblindStyle()
{
	URelicRunnersGameInstance* GI = Cast<URelicRunnersGameInstance>(GetGameInstance());
	if (!GI) return;

	FLinearColor AccentColor;

	switch (GI->ColorblindMode)
	{
	case EColorblindMode::Protanopia:
		AccentColor = FLinearColor(1.0f, 0.55f, 0.0f, 1.0f);
		break;

	case EColorblindMode::Deuteranopia:
		AccentColor = FLinearColor(1.0f, 0.6f, 0.0f, 1.0f);
		break;

	case EColorblindMode::Tritanopia:
		AccentColor = FLinearColor(0.1f, 1.0f, 0.35f, 1.0f);
		break;

	case EColorblindMode::Normal:
	default:
		AccentColor = FLinearColor(0.38f, 0.24f, 0.12f, 1.0f);
		break;
	}

	if (BackButton)
	{
		BackButton->SetBackgroundColor(AccentColor);
	}
}

