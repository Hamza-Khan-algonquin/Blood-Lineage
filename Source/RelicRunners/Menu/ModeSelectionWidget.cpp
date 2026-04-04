#include "ModeSelectionWidget.h"
#include "MainMenuWidget.h"
#include "Components/Button.h"
#include "Components/HorizontalBox.h"
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

	if (HostButton)      HostButton->OnClicked.AddDynamic(this, &UModeSelectionWidget::HostButtonClicked);
	if (UpgradesButton)  UpgradesButton->OnClicked.AddDynamic(this, &UModeSelectionWidget::UpgradesButtonClicked);
	if (SettingsButton)  SettingsButton->OnClicked.AddDynamic(this, &UModeSelectionWidget::SettingsButtonClicked);
	if (TutorialButton)  TutorialButton->OnClicked.AddDynamic(this, &UModeSelectionWidget::TutorialButtonClicked);
	if (CreditsButton)   CreditsButton->OnClicked.AddDynamic(this, &UModeSelectionWidget::CreditsButtonClicked);
	if (ExitButton)      ExitButton->OnClicked.AddDynamic(this, &UModeSelectionWidget::ExitButtonClicked);

	URelicRunnersGameInstance* GI = GetWorld()->GetGameInstance<URelicRunnersGameInstance>();
	if (!GI) return;

	SaveFiles->ClearListItems();

	//Re-populate any previously made save slots
	for (USaveFileListData* Data : GI->SaveSlots)
	{
		Data->ParentWidget = this;
		SaveFiles->AddItem(Data);
	}

	//Re-highlight if previously selected slot
	FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this, GI]() 
	{
		if (GI->SelectedSaveSlot)
		{
			if (USaveFileListWidget* Widget = Cast<USaveFileListWidget>(SaveFiles->GetEntryWidgetFromItem(GI->SelectedSaveSlot)))
			{
				HighlightSelectedSlot(Widget);
			}
		}
	}, 0.05f, false);

	ShowUpgradesAvailable();
}

void UModeSelectionWidget::ShowUpgradesAvailable()
{
	URelicRunnersGameInstance* GI = GetWorld()->GetGameInstance<URelicRunnersGameInstance>();
	if (!GI) return;

	if (GI->SelectedSaveSlot)
	{
		if (GI->SelectedSaveSlot->SaveData.MetaPoints > 0)
		{
			UpgradeAvailable1->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
			UpgradeAvailable2->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		}
		else
		{
			UpgradeAvailable1->SetVisibility(ESlateVisibility::Collapsed);
			UpgradeAvailable2->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
}

void UModeSelectionWidget::HighlightSelectedSlot(USaveFileListWidget* NewSelection)
{
	if (!NewSelection) return;

	if (CurrentSelectedSlot) CurrentSelectedSlot->SetSelected(false);

	CurrentSelectedSlot = NewSelection;
	CurrentSelectedSlot->SetSelected(true);

	if (URelicRunnersGameInstance* GI = GetWorld()->GetGameInstance<URelicRunnersGameInstance>())
	{
		GI->SelectedSaveSlot = CurrentSelectedSlot->SaveFileData;
		GI->PlayerName = CurrentSelectedSlot->SaveFileData ? CurrentSelectedSlot->SaveFileData->GetName() : "";
	}

	ShowUpgradesAvailable();
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
		if(!GameInstance->SelectedSaveSlot)
		{
			//No savefile so prompt player to make one
			PromptCreateSaveFile();
		}
		else
		{
			//Play game
			GameInstance->PlayerName = GameInstance->SelectedSaveSlot->GetName();

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
	APlayerController* PC = GetWorld()->GetFirstPlayerController();

	UKismetSystemLibrary::QuitGame(this,PC,EQuitPreference::Quit, false);
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
	URelicRunnersGameInstance* GameInstance = Cast<URelicRunnersGameInstance>(GetGameInstance());
	if (GameInstance)
	{
		if (!GameInstance->SelectedSaveSlot)
		{
			//No savefile so prompt player to make one
			PromptCreateSaveFile();
		}
		else
		{
			if (ParentMenu)
			{
				ParentMenu->ShowUpgradesWidget();
			}
		}
	}
}

void UModeSelectionWidget::OpenSaveFilePopupWidget(USaveFileListWidget* TargetSlot)
{
	if (!SaveFilePopupClass) return;

	USaveFilePopupWidget* Popup = CreateWidget<USaveFilePopupWidget>(GetWorld(), SaveFilePopupClass);
	
	Popup->TargetSlot = TargetSlot;
	Popup->AddToViewport();
}

void UModeSelectionWidget::PromptCreateSaveFile()
{
	if (!SaveFilePopupClass || !SaveFiles) return;

	for (int i = 0; i < SaveFiles->GetNumItems(); i++)
	{
		USaveFileListData* Data = Cast<USaveFileListData>(SaveFiles->GetItemAt(i));
		if (Data && Data->IsEmptySlot)
		{
			USaveFileListWidget* Widget = Cast<USaveFileListWidget>(SaveFiles->GetEntryWidgetFromItem(Data));

			if (Widget)
			{
				OpenSaveFilePopupWidget(Widget);
				return;
			}
		}
	}
}
