// Fill out your copyright notice in the Description page of Project Settings.


#include "SettingsWidget.h"
#include "MainMenuWidget.h"
#include "Components/Button.h"
#include "Components/TileView.h"
#include "Components/TextBlock.h"
#include "KeybindingsListData.h"
#include <EnhancedInputSubsystems.h>
#include "Keybinds.h"
#include "KeybindingsListWidget.h"
#include <RelicRunners/PlayerController/RelicRunnersPlayerController.h>
#include <Kismet/GameplayStatics.h>
#include "RelicRunners/Game/RelicRunnersGameInstance.h"
#include "Components/ComboBoxString.h"
#include "PauseMenuWidget.h"

void USettingsWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (BackButton)
	{
		BackButton->OnClicked.AddDynamic(this, &USettingsWidget::BackButtonClicked);
	}

	if (B_RestoreDefaults)
	{
		B_RestoreDefaults->OnClicked.AddDynamic(this, &USettingsWidget::OnRestoreDefaultsClicked);
	}

	if (ResolutionDropDown)
	{
		ResolutionDropDown->OnSelectionChanged.AddDynamic(this, &USettingsWidget::HandleSelectionChanged);
	}

	InitializeDefaultKeybindings();
}

void USettingsWidget::OnRestoreDefaultsClicked()
{
	URelicRunnersGameInstance* GI = Cast<URelicRunnersGameInstance>(GetGameInstance());
	if (GI)
	{
		GI->ResetToDefaults();
	}

	InitializeDefaultKeybindings();
}

void USettingsWidget::StartRebinding(UKeybindingsListWidget* Entry)
{
	WaitingForKeyEntry = Entry;
}

FReply USettingsWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	if (!WaitingForKeyEntry)
		return Super::NativeOnKeyDown(InGeometry, InKeyEvent);

	FKey PressedKey = InKeyEvent.GetKey();
	if (!PressedKey.IsValid())
		return FReply::Handled();

	return HandleKeyBindPressed(PressedKey);
}

FReply USettingsWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (!WaitingForKeyEntry)
		return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);

	FKey PressedKey = InMouseEvent.GetEffectingButton();
	if (!PressedKey.IsValid())
		return FReply::Handled();

	return HandleKeyBindPressed(PressedKey);
}

FReply USettingsWidget::HandleKeyBindPressed(FKey PressedKey)
{
	UKeybindingsListData* Data = WaitingForKeyEntry->GetKeybindData();
	if (!Data)
		return FReply::Handled();

	//Prevent duplicate keys
	for (UKeybindingsListData* Binding : DefaultKeybindings)
	{
		if (Binding != Data && Binding->Keybind == PressedKey)
		{
			Binding->Keybind = EKeys::Invalid;
			Binding->ReadableBind = TEXT("None");
		}
	}

	Data->Keybind = PressedKey;
	Data->ReadableBind = GetReadableKeyName(PressedKey);

	if (URelicRunnersGameInstance* GI = Cast<URelicRunnersGameInstance>(GetGameInstance()))
	{
		if (UKeybinds* Keys = GI->Keys)
		{
			for (auto& Bind : Keys->KeyBinds)
			{
				if (Bind.Name != Data->Name && Bind.Bind == PressedKey)
				{
					Bind.Bind = EKeys::Invalid;
				}

				if (Bind.Name == Data->Name)
				{
					Bind.Bind = PressedKey;
				}
			}
		}
	}

	UpdateBinds();

	for (UKeybindingsListData* Binding : DefaultKeybindings)
	{
		if (Binding->BoundWidget)
		{
			Binding->BoundWidget->RefreshVisual();
		}
	}

	WaitingForKeyEntry = nullptr;

	return FReply::Handled();
}

void USettingsWidget::UpdateBinds()
{
	URelicRunnersGameInstance* GI = Cast<URelicRunnersGameInstance>(GetGameInstance());
	if (GI)
	{
		GI->ApplyKeybindings();
	}
}

void USettingsWidget::SetParentMenu(UMainMenuWidget* InParentMenu)
{
	MainParentMenu = InParentMenu;
	PauseParentMenu = nullptr;
	if (MainParentMenu)
	{
		if (TB_BackButton)
		{
			TB_BackButton->SetText(FText::FromString("Main Menu"));
		}
		MainParentMenu->ShowModeSelectionWidget();
		return;
	}
}

void USettingsWidget::SetParentMenu(UPauseMenuWidget* InParentMenu)
{
	MainParentMenu = nullptr;
	PauseParentMenu = InParentMenu;
	if (PauseParentMenu)
	{
		if (TB_BackButton)
		{
			TB_BackButton->SetText(FText::FromString("Return"));
		}
		
		return;
	}
}

void USettingsWidget::HandleSelectionChanged(FString Item, ESelectInfo::Type SelectionType)
{
	if (Item == "1280x720")
	{
		UE_LOG(LogTemp, Warning, TEXT("Resolution has been set to 1080x720."));
	}
	else if (Item == "1920x1080")
	{
		UE_LOG(LogTemp, Warning, TEXT("Resolution has been set to 1920x1080."));
	}
	else if (Item == "2560x1440")
	{
		UE_LOG(LogTemp, Warning, TEXT("Resolution has been set to 2560x1440."));
	}
	else if (Item == "3840x2160")
	{
		UE_LOG(LogTemp, Warning, TEXT("Resolution has been set to 3840x2160."));
	}
}

void USettingsWidget::BackButtonClicked()
{
	if (MainParentMenu)
	{
		MainParentMenu->ShowModeSelectionWidget();
	}
	if (PauseParentMenu)
	{
		PauseParentMenu->ShowPauseMenu();
	}
}

void USettingsWidget::InitializeDefaultKeybindings()
{
	DefaultKeybindings.Empty();

	auto AddBinding = [&](const FString& ActionName, const FKey& Key) 
	{ 
		UKeybindingsListData* NewEntry = NewObject<UKeybindingsListData>(this); 
		NewEntry->Name = ActionName; 
		NewEntry->Keybind = Key; 
		NewEntry->ReadableBind = GetReadableKeyName(Key);  
		DefaultKeybindings.Add(NewEntry); 
	};

	URelicRunnersGameInstance* GI = Cast<URelicRunnersGameInstance>(GetGameInstance());
	if (!GI) return;

	for (auto Binds : GI->Keys->KeyBinds)
	{
		AddBinding(Binds.Name, Binds.Bind);
	}

	if (KeybindingsTileView)
	{
		KeybindingsTileView->ClearListItems();

		for (UKeybindingsListData* Binding : DefaultKeybindings)
		{
			KeybindingsTileView->AddItem(Binding);
		}
	}
}

void USettingsWidget::ToggleInvertedXMouse()
{
	URelicRunnersGameInstance* GI = Cast<URelicRunnersGameInstance>(GetGameInstance());
	if (GI)
	{
		GI->Keys->InvertedXMouse = !GI->Keys->InvertedXMouse;
	}
}

void USettingsWidget::ToggleInvertedYMouse()
{
	URelicRunnersGameInstance* GI = Cast<URelicRunnersGameInstance>(GetGameInstance());
	if (GI)
	{
		GI->Keys->InvertedYMouse = !GI->Keys->InvertedYMouse;
	}
}
