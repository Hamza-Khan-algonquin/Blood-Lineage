#include "ModeSelectionWidget.h"
#include "MainMenuWidget.h"
#include "Components/Button.h"
#include "Components/TileView.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"
#include "RelicRunners/Game/RelicRunnersGameInstance.h"
#include "Components/EditableTextBox.h"
#include <RelicRunners/PlayerState/RelicRunnersPlayerState.h>
#include "SaveFileListData.h"
#include "SaveFileListWidget.h"
#include "SaveFilePopupWidget.h"


void UModeSelectionWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (HostButton)
	{
		HostButton->OnClicked.AddDynamic(this, &UModeSelectionWidget::HostButtonClicked);
	}

	if (UpgradesButton)
	{
		UpgradesButton->OnClicked.AddDynamic(this, &UModeSelectionWidget::UpgradesButtonClicked);
	}

	if (SettingsButton)
	{
		SettingsButton->OnClicked.AddDynamic(this, &UModeSelectionWidget::SettingsButtonClicked);
	}

	if (TutorialButton)
	{
		TutorialButton->OnClicked.AddDynamic(this, &UModeSelectionWidget::TutorialButtonClicked);
	}

	if (CreditsButton)
	{
		CreditsButton->OnClicked.AddDynamic(this, &UModeSelectionWidget::CreditsButtonClicked);
	}

	if (CreditsButton)
	{
		ExitButton->OnClicked.AddDynamic(this, &UModeSelectionWidget::ExitButtonClicked);
	}

	URelicRunnersGameInstance* GI = GetWorld()->GetGameInstance<URelicRunnersGameInstance>();
	if (!GI) return;

	SaveFiles->ClearListItems();

	for (USaveFileListData* Data : GI->SaveSlots)
	{
		Data->ParentWidget = this;
		SaveFiles->AddItem(Data);
	}
}

void UModeSelectionWidget::SetParentMenu(UMainMenuWidget* InParentMenu)
{
	ParentMenu = InParentMenu;
}

void UModeSelectionWidget::HostButtonClicked()
{
	URelicRunnersGameInstance* GameInstance = Cast<URelicRunnersGameInstance>(GetGameInstance());
	if (GameInstance)
	{
		if (HostButton)
		{
			HostButton->SetIsEnabled(false);
			GameInstance->HostGame();
		}

		if (ParentMenu)
		{
			ParentMenu->ShowSessionsWidget();
		}
	}
}

void UModeSelectionWidget::ResetUI()
{
	if (HostButton)
	{
		HostButton->SetIsEnabled(true);
	}
}

void UModeSelectionWidget::SettingsButtonClicked()
{
	if (ParentMenu)
	{
		ParentMenu->ShowSettingsWidget();
	}
}

void UModeSelectionWidget::CreditsButtonClicked()
{
	if (ParentMenu)
	{
		ParentMenu->ShowCreditsWidget();
	}
}

void UModeSelectionWidget::ExitButtonClicked()
{
	
}

void UModeSelectionWidget::TutorialButtonClicked()
{
	if (ParentMenu)
	{
		ParentMenu->ShowTutorialWidget();
	}
}

void UModeSelectionWidget::UpgradesButtonClicked()
{
	if (ParentMenu)
	{
		ParentMenu->ShowUpgradesWidget();
	}
}

void UModeSelectionWidget::OpenSaveFilePopupWidget(USaveFileListWidget* TargetSlot)
{
	if (!SaveFilePopupClass) return;

	USaveFilePopupWidget* Popup = CreateWidget<USaveFilePopupWidget>(GetWorld(), SaveFilePopupClass);
	Popup->TargetSlot = TargetSlot;
	Popup->AddToViewport();
}
