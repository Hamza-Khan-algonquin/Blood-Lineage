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

	ApplyColorblindStyle();
	
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
}

void UModeSelectionWidget::SetParentMenu(UMainMenuWidget* InParentMenu)
{
	ParentMenu = InParentMenu;
}

void UModeSelectionWidget::ApplyColorblindStyle()
{
	URelicRunnersGameInstance* GI = GetWorld()->GetGameInstance<URelicRunnersGameInstance>();
	if (!GI) return;

	FLinearColor AccentColor;
	FLinearColor TextColor = FLinearColor::White;
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
		//AccentColor = FLinearColor(0.0625, 0.049255, 0.031143);
		break;
	}

	if (HostButton)     HostButton->SetBackgroundColor(AccentColor);
	if (UpgradesButton) UpgradesButton->SetBackgroundColor(AccentColor);
	if (SettingsButton) SettingsButton->SetBackgroundColor(AccentColor);
	if (TutorialButton) TutorialButton->SetBackgroundColor(AccentColor);
	if (CreditsButton)  CreditsButton->SetBackgroundColor(AccentColor);
	if (ExitButton)     ExitButton->SetBackgroundColor(AccentColor);

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
