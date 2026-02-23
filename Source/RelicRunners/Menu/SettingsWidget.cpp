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
#include <RelicRunners/RelicRunnersCharacter.h>
#include <Kismet/GameplayStatics.h>
#include "Components/Slider.h"
#include <Camera/CameraComponent.h>
#include "RelicRunners/Game/RelicRunnersGameInstance.h"
#include "Components/ComboBoxString.h"
#include "PauseMenuWidget.h"
#include "Engine/Engine.h"
#include "GameFramework/GameUserSettings.h"


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

	if (FOVSlider)
	{
		FOVSlider->OnValueChanged.AddDynamic(this, &USettingsWidget::OnFOVSliderChanged);

		if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
		{
			if (PC->PlayerCameraManager)
			{
				const float CurrentFOV = PC->PlayerCameraManager->GetFOVAngle();
				float T = (CurrentFOV - MinFOV) / (MaxFOV - MinFOV);
				FOVSlider->SetValue(FMath::Clamp(T, 0.f, 1.f));

			}
		}
	}

	if (B_InvertedXMouse)
	{
		B_InvertedXMouse->OnCheckStateChanged.AddDynamic(this, &USettingsWidget::ToggleInvertedXMouse);

		if (URelicRunnersGameInstance* GI = Cast<URelicRunnersGameInstance>(GetGameInstance()))
		{
			if (GI->Keys)
			{
				B_InvertedXMouse->SetIsChecked(GI->Keys->InvertedXMouse);
			}
		}
	}

	if (B_InvertedYMouse)
	{
		B_InvertedYMouse->OnCheckStateChanged.AddDynamic(this, &USettingsWidget::ToggleInvertedYMouse);

		if (URelicRunnersGameInstance* GI = Cast<URelicRunnersGameInstance>(GetGameInstance()))
		{
			if (GI->Keys)
			{
				B_InvertedYMouse->SetIsChecked(GI->Keys->InvertedYMouse);
			}
			
		}
	}

	if (XSlider)
	{
		XSlider->OnValueChanged.AddDynamic(this, &USettingsWidget::OnSensXChanged);
	}
	if (YSlider)
	{
		YSlider->OnValueChanged.AddDynamic(this, &USettingsWidget::OnSensYChanged);
	}


	if (URelicRunnersGameInstance* GI = Cast<URelicRunnersGameInstance>(GetGameInstance()))
	{
		if (GI->Keys)
		{
			const float XSens = GI->Keys->HorizSens;
			const float YSens = GI->Keys->VertSens;

			const float XNorm = FMath::Clamp((XSens - MinSens) / (MaxSens - MinSens), 0.f, 1.f);
			const float YNorm = FMath::Clamp((YSens - MinSens) / (MaxSens - MinSens), 0.f, 1.f);

			if (XSlider) XSlider->SetValue(XNorm);
			if (YSlider) YSlider->SetValue(YNorm);

			UpdateSensText(TB_XSensValue, XSens);
			UpdateSensText(TB_YSensValue, YSens);
		}
	}

	if (BrightnessSlider)
	{
		BrightnessSlider->OnValueChanged.RemoveDynamic(this, &USettingsWidget::OnBrightnessChanged);
		
		if (URelicRunnersGameInstance* GI = Cast<URelicRunnersGameInstance>(GetGameInstance()))
		{
			if (GI->Keys)
			{
				const float B = GI->Keys->Brightness;
				const float T = FMath::Clamp((B - MinBrightness) / (MaxBrightnes - MinBrightness), 0.f, 1.f);
				BrightnessSlider->SetValue(T);
				ApplyBrightness(B);
				UpdateBrightnessText(B);
			}
		}
		BrightnessSlider->OnValueChanged.AddDynamic(this, &USettingsWidget::OnBrightnessChanged);
	}

	if (B_MotionBlur)
	{
		B_MotionBlur->OnCheckStateChanged.AddDynamic(this, &USettingsWidget::OnMotionBlurClicked);

		if (URelicRunnersGameInstance* GI = Cast<URelicRunnersGameInstance>(GetGameInstance()))
		{
			if (GI->Keys)
			{
				ApplyMotionBlur(GI->Keys->motionBlurEnabled);
			}
		}
	}

	if (ResolutionDropDown)
	{
		
		ResolutionDropDown->ClearOptions();

		if (UGameUserSettings* GS = GEngine->GetGameUserSettings())
		{
			const FIntPoint cur = GS->GetScreenResolution();
			const FString current = FString::Printf(TEXT("%dx%d"), cur.X, cur.Y);

			TArray<FIntPoint> Options = { {1280, 720}, {1920,1080}, {2560, 1440},{3840, 2160} };

			for (const FIntPoint& R : Options)
			{
				ResolutionDropDown->AddOption(FString::Printf(TEXT("%dx%d"), R.X, R.Y));
			}
			if (ResolutionDropDown->FindOptionIndex(current) != INDEX_NONE) 
			{
				ResolutionDropDown->SetSelectedOption(current); 
			}
			else {
				ResolutionDropDown->AddOption(current);
				ResolutionDropDown->SetSelectedOption(current);
			}
			

		}
		ResolutionDropDown->OnSelectionChanged.AddDynamic(this, &USettingsWidget::HandleSelectionChanged);
	}

	if (WindowModeDropDown)
	{
		FillWindowOptions();
		SetWindowMode();
		WindowModeDropDown->OnSelectionChanged.AddDynamic(this, &USettingsWidget::HandleWindowModeSelectionChanged);
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

void USettingsWidget::OnSensXChanged(float Value)
{
	const float NewSens = FMath::Lerp(MinSens, MaxSens, Value);

	if (URelicRunnersGameInstance* GI = Cast<URelicRunnersGameInstance>(GetGameInstance()))
	{
		if (GI->Keys)
		{
			GI->Keys->HorizSens = NewSens;
		}
	}

	UpdateSensText(TB_XSensValue, NewSens);
}

void USettingsWidget::OnSensYChanged(float Value)
{
	const float NewSens = FMath::Lerp(MinSens, MaxSens, Value);

	if (URelicRunnersGameInstance* GI = Cast<URelicRunnersGameInstance>(GetGameInstance()))
	{
		if (GI->Keys)
		{
			GI->Keys->VertSens = NewSens;
		}
	}

	UpdateSensText(TB_YSensValue, NewSens);
}

void USettingsWidget::UpdateSensText(UTextBlock* TextBlock, float Sens)
{
	if (!TextBlock) return;
	TextBlock->SetText(FText::FromString(FString::Printf(TEXT("%.2f"), Sens)));
}

void USettingsWidget::OnBrightnessChanged(float Value)
{
	const float NewExposure = FMath::Lerp(MinBrightness, MaxBrightnes, Value);

	if (URelicRunnersGameInstance* GI = Cast<URelicRunnersGameInstance>(GetGameInstance()))
	{
		if (GI->Keys)
		{
			GI->Keys->Brightness = NewExposure;
		}

	}

	ApplyBrightness(NewExposure);
	UpdateBrightnessText(NewExposure);
}

void USettingsWidget::ApplyBrightness(float ExposureComp)
{
	if (ARelicRunnersCharacter* Character = Cast<ARelicRunnersCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0)))
	{
		if (UCameraComponent* Camera = Character->FindComponentByClass<UCameraComponent>())
		{
			FPostProcessSettings& PPS = Camera->PostProcessSettings;

			PPS.bOverride_AutoExposureBias = true;
			PPS.AutoExposureBias = ExposureComp;

			Camera->PostProcessBlendWeight = 1.0f;
		}
	}
}

void USettingsWidget::UpdateBrightnessText(float Gamma)
{
	if (!TB_BrightnessValue) return;
	TB_BrightnessValue->SetText(FText::FromString(FString::Printf(TEXT("%.2f"), Gamma)));
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

void USettingsWidget::ApplyResolution(const FString& Resolution)
{

}

void USettingsWidget::HandleWindowModeSelectionChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
	if(SelectionType == ESelectInfo::OnMouseClick)
		if (UGameUserSettings* GS = GEngine->GetGameUserSettings())
		{
			EWindowMode::Type Mode = EWindowMode::Windowed;
			if (SelectedItem == "FullScreen")
			{
				FIntPoint NewRes(2560, 1440);
				GS->SetFullscreenMode(EWindowMode::Fullscreen);
				GS->SetScreenResolution(NewRes);
			}
			else if (SelectedItem == "WindowedFullscreen")
			{
				GS->SetFullscreenMode(EWindowMode::WindowedFullscreen);
			}
			else if (SelectedItem == "Windowed")
			{
				GS->SetFullscreenMode(EWindowMode::Windowed);
			}

			GS->ApplySettings(false);
			GS->SaveSettings();

		}
}

void USettingsWidget::FillWindowOptions()
{
	WindowModeDropDown->ClearOptions();

	WindowModeDropDown->AddOption(TEXT("FullScreen"));
	WindowModeDropDown->AddOption(TEXT("WindowedFullscreen"));
	WindowModeDropDown->AddOption(TEXT("Windowed"));
}

//to make it show the current mode selected whne the menu opens
void USettingsWidget::SetWindowMode()
{
	if (!WindowModeDropDown) return;

	if (UGameUserSettings* GS = GEngine->GetGameUserSettings())
	{
		const EWindowMode::Type Mode = GS->GetFullscreenMode();

		FString Label = "Windowed";
		if (Mode == EWindowMode::Fullscreen)
		{
			Label = "FullScreen";
		}
		else if (Mode == EWindowMode::WindowedFullscreen)
		{
			Label = "WindowedFullscreen";
		}
		else if (Mode == EWindowMode::Windowed)
		{
			Label = "Windowed";
		}
		WindowModeDropDown->SetSelectedOption(Label);
	}
}

void USettingsWidget::HandleSelectionChanged(FString Item, ESelectInfo::Type SelectionType)
{
}



//fov settings
void USettingsWidget::OnFOVSliderChanged(float Value)
{

	const float NewFOV = FMath::Lerp(MinFOV, MaxFOV, Value);
	ApplyFOV(NewFOV);
	UpdateFOVText(NewFOV);
}

void USettingsWidget::ApplyFOV(float NewFOV)
{
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
	{
		if (PC->PlayerCameraManager)
		{
			PC->PlayerCameraManager->SetFOV(NewFOV);
		}
	}
}

void USettingsWidget::UpdateFOVText(float NewFOV)
{
	if (!TB_FOVValue) return;

	const int32 Rounded = FMath::RoundToInt(NewFOV);
	TB_FOVValue->SetText(FText::AsNumber(Rounded));
}
void USettingsWidget::OnMotionBlurClicked(bool isChecked)
{
	if (URelicRunnersGameInstance* GI = Cast<URelicRunnersGameInstance>(GetGameInstance()))
	{
		if (GI->Keys)
		{
			GI->Keys->motionBlurEnabled = isChecked;
		}
	}
	ApplyMotionBlur(isChecked);
}
void USettingsWidget::ApplyMotionBlur(bool bEnabled)
{
	if (ARelicRunnersCharacter* Character = Cast<ARelicRunnersCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0)))
	{
		if (UCameraComponent* Camera = Character->FindComponentByClass<UCameraComponent>())
		{
			FPostProcessSettings& PPS = Camera->PostProcessSettings;

			PPS.bOverride_MotionBlurAmount = true;
			PPS.MotionBlurAmount = bEnabled ? 0.5f : 0.0f;

			Camera->PostProcessBlendWeight = 1.0f;
		}
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

void USettingsWidget::ToggleInvertedXMouse(bool isChecked)
{
	URelicRunnersGameInstance* GI = Cast<URelicRunnersGameInstance>(GetGameInstance());
	if (GI->Keys)
	{
		GI->Keys->InvertedXMouse = isChecked;
		GI->ApplyKeybindings();
	}
}

void USettingsWidget::ToggleInvertedYMouse(bool isChecked)
{
	URelicRunnersGameInstance* GI = Cast<URelicRunnersGameInstance>(GetGameInstance());
	if (GI->Keys)
	{
		GI->Keys->InvertedYMouse = isChecked;
		GI->ApplyKeybindings();
	}
}
