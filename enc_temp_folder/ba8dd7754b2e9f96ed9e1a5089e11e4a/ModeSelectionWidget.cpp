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

	if (NameTextBox)
	{
		NameTextBox->OnTextChanged.AddDynamic(this, &UModeSelectionWidget::OnPlayerNameTextChanged);
	}

	MaxCharacterLimit = 12;

	USaveFileListData* NewSave = NewObject<USaveFileListData>();

	if (NewSave)
	{
		NewSave->ParentWidget = this;
		SaveFiles->AddItem(NewSave);
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
			static int32 PlayerIndex = 1;
			FString EnteredName = TEXT("Player") + FString::FromInt(PlayerIndex++);
			if (NameTextBox)
			{
				EnteredName = NameTextBox->GetText().ToString();
			}

			// Set GameInstance name
			GameInstance->SetCharacterName(EnteredName);

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


//void UModeSelectionWidget::SessionsButtonClicked()
//{
//	if (ParentMenu)
//	{
//		URelicRunnersGameInstance* GameInstance = Cast<URelicRunnersGameInstance>(GetGameInstance());
//
//		if (GameInstance && NameTextBox)
//		{
//			GameInstance->SetCharacterName(NameTextBox->GetText().ToString());
//		}
//		ParentMenu->ShowSessionsWidget();
//	}
//}

void UModeSelectionWidget::SettingsButtonClicked()
{
	if (ParentMenu)
	{
		URelicRunnersGameInstance* GameInstance = Cast<URelicRunnersGameInstance>(GetGameInstance());

		if (GameInstance && NameTextBox)
		{
			GameInstance->SetCharacterName(NameTextBox->GetText().ToString());
		}
		ParentMenu->ShowSettingsWidget();
	}
}

void UModeSelectionWidget::CreditsButtonClicked()
{
	if (ParentMenu)
	{
		URelicRunnersGameInstance* GameInstance = Cast<URelicRunnersGameInstance>(GetGameInstance());

		if (GameInstance && NameTextBox)
		{
			GameInstance->SetCharacterName(NameTextBox->GetText().ToString());
		}
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
		URelicRunnersGameInstance* GameInstance = Cast<URelicRunnersGameInstance>(GetGameInstance());

		if (GameInstance && NameTextBox)
		{
			GameInstance->SetCharacterName(NameTextBox->GetText().ToString());
		}
		ParentMenu->ShowTutorialWidget();
	}
}

void UModeSelectionWidget::UpgradesButtonClicked()
{
	if (ParentMenu)
	{
		URelicRunnersGameInstance* GameInstance = Cast<URelicRunnersGameInstance>(GetGameInstance());

		if (GameInstance && NameTextBox)
		{
			GameInstance->SetCharacterName(NameTextBox->GetText().ToString());
		}
		ParentMenu->ShowUpgradesWidget();
	}
}

void UModeSelectionWidget::OnPlayerNameTextChanged(const FText& Text)
{
	FString CurrentText = Text.ToString();

	if (CurrentText.Len() > MaxCharacterLimit)
	{
		// Truncate the text
		FString TruncatedText = CurrentText.Left(MaxCharacterLimit);
		NameTextBox->SetText(FText::FromString(TruncatedText));
	}
}

