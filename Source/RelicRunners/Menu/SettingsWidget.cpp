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
#include "EngineUtils.h"
#include <Camera/CameraComponent.h>
#include "Camera/CameraActor.h"
#include "RelicRunners/Game/RelicRunnersGameInstance.h"
#include "Components/ComboBoxString.h"
#include "PauseMenuWidget.h"
#include "Engine/Engine.h"
#include "GameFramework/GameUserSettings.h"
#include "AudioMixerBlueprintLibrary.h"

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
		
		FOVSlider->OnValueChanged.RemoveDynamic(this, &USettingsWidget::OnFOVSliderChanged);


		if (URelicRunnersGameInstance* GI = Cast<URelicRunnersGameInstance>(GetGameInstance()))
		{
			const float CurrentFOV = GI->SavedFOV > 0.f ? GI->SavedFOV : 85.f;
			float FOVNorm = FMath::Clamp((CurrentFOV - MinFOV) / (MaxFOV - MinFOV), 0.f, 1.f);
			FOVSlider->SetValue(FOVNorm);
			UpdateFOVText(CurrentFOV);
		}
		FOVSlider->OnValueChanged.AddDynamic(this, &USettingsWidget::OnFOVSliderChanged);
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
				const float B = GI->SavedBrightness;
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
		ResMap.Empty();

		TArray<FIntPoint> Resolutions = { {1280, 720}, {1600, 900},{ 1920, 1080 }, {2560, 1440}};

		for (const FIntPoint& Res : Resolutions)
		{
			FString Label = FString::Printf(TEXT("%dx%d"), Res.X, Res.Y);
			ResolutionDropDown->AddOption(Label);
			ResMap.Add(Label, Res);
		}

		FString DefaultRes = TEXT("1920X1080");
		if (ResolutionDropDown)
		{
			ResolutionDropDown->SetSelectedOption(DefaultRes);
		}

		ResolutionDropDown->OnSelectionChanged.AddDynamic(this, &USettingsWidget::HandleResolutionSelectionChanged);
	}

	if (WindowModeDropDown)
	{
		FillWindowOptions();
		SetWindowMode();
		WindowModeDropDown->OnSelectionChanged.AddDynamic(this, &USettingsWidget::HandleWindowModeSelectionChanged);
	}

	if (ColourModeDropDown)
	{
		ColourModeDropDown->ClearOptions();
		ColourModeDropDown->AddOption("Normal");
		ColourModeDropDown->AddOption("Protanopia");
		ColourModeDropDown->AddOption("Deuteranopia");
		ColourModeDropDown->AddOption("Tritanopia");
		ColourModeDropDown->RefreshOptions();
		URelicRunnersGameInstance* GI = Cast<URelicRunnersGameInstance>(GetGameInstance());


		if (GI)
		{
			FString SelectedOption = "Normal";

			switch (GI->ColorblindMode)
			{
			case EColorblindMode::Normal:
				SelectedOption = "Normal";
				break;
			case EColorblindMode::Protanopia:
				SelectedOption = "Protanopia";
				break;
			case EColorblindMode::Deuteranopia:
				SelectedOption = "Deuteranopia";
				break;
			case EColorblindMode::Tritanopia:
				SelectedOption = "Tritanopia";
				break;
			}

			ColourModeDropDown->SetSelectedOption(SelectedOption);
		}
		ColourModeDropDown->OnSelectionChanged.AddDynamic(this, &USettingsWidget::HandleColorModeChanged);

	}


	if (OutputDeviceDropDown)
	{
		OutputDeviceDropDown->OnSelectionChanged.AddDynamic(this, &USettingsWidget::HandleOutputDeviceChanged);
		FOnAudioOutputDevicesObtained DevicesDelegate;
		DevicesDelegate.BindUFunction(this, FName("OnAudioDevicesFound"));

		UAudioMixerBlueprintLibrary::GetAvailableAudioOutputDevices(this, DevicesDelegate);
	}

	if (MasterVolumeSlider)
	{
		MasterVolumeSlider->OnValueChanged.AddDynamic(this, &USettingsWidget::OnMasterVolumeChanged);

		float CurrentVolume = 1.0f;

		if (URelicRunnersGameInstance* GI = Cast<URelicRunnersGameInstance>(GetGameInstance()))
		{
			if (GI->Keys)
			{
				CurrentVolume = GI->Keys->MasterVolume;
			}
		}

		const float Normalized = FMath::Clamp(
			(CurrentVolume - MinVolume) / (MaxVolume - MinVolume),
			0.f, 1.f);

		MasterVolumeSlider->SetValue(Normalized);
		ApplyMasterVolume(CurrentVolume);
		UpdateMasterVolumeText(CurrentVolume);
	}

	if (AntiAliasingDropDown)
	{
		AntiAliasingDropDown->ClearOptions();

		AntiAliasingDropDown->AddOption(TEXT("Low"));
		AntiAliasingDropDown->AddOption(TEXT("Medium"));
		AntiAliasingDropDown->AddOption(TEXT("High"));
		AntiAliasingDropDown->AddOption(TEXT("Epic"));
		AntiAliasingDropDown->AddOption(TEXT("Cinematic"));

		if (UGameUserSettings* GS = GEngine->GetGameUserSettings())
		{
			const int32 AAQuality = GS->GetAntiAliasingQuality();

			switch (AAQuality)
			{
			case 0: AntiAliasingDropDown->SetSelectedOption(TEXT("Low")); break;
			case 1: AntiAliasingDropDown->SetSelectedOption(TEXT("Medium")); break;
			case 2: AntiAliasingDropDown->SetSelectedOption(TEXT("High")); break;
			case 3: AntiAliasingDropDown->SetSelectedOption(TEXT("Epic")); break;
			case 4: AntiAliasingDropDown->SetSelectedOption(TEXT("Cinematic")); break;
			default: AntiAliasingDropDown->SetSelectedOption(TEXT("High")); break;
			}
		}
		AntiAliasingDropDown->OnSelectionChanged.AddDynamic(this, &USettingsWidget::HandleAntiAliasingChanged);

	}

	if (AutoLoot)
	{

		AutoLoot->OnCheckStateChanged.AddDynamic(this, &USettingsWidget::OnAutoLootChanged);

		if (URelicRunnersGameInstance* GI = Cast<URelicRunnersGameInstance>(GetGameInstance()))
		{
			if (GI->Keys)
			{
				AutoLoot->SetIsChecked(GI->Keys->bAutoLoot);
				AutoLoot->SetIsChecked(GI->bAutoLoot);
			}
		}

	}
	/*if (LanguageDropDown)
	{
		LanguageDropDown->ClearOptions();
		LanguageDropDown->AddOption("English");
		LanguageDropDown->AddOption("French");
		LanguageDropDown->RefreshOptions();

		if (URelicRunnersGameInstance* GI = Cast<URelicRunnersGameInstance>(GetGameInstance()))
		{
			FString SelectedOption = "English";

			switch (GI->CurrentLanguage)
			{
			case EGameLanguage::French:
				SelectedOption = "French";
				break;

			case EGameLanguage::English:
			default:
				SelectedOption = "English";
				break;
			}

			LanguageDropDown->SetSelectedOption(SelectedOption);
		}

		LanguageDropDown->OnSelectionChanged.AddDynamic(this, &USettingsWidget::HandleLanguageChanged);
	}
	*/
	if (URelicRunnersGameInstance* GI = Cast<URelicRunnersGameInstance>(GetGameInstance()))
	{
		InitializeDefaultKeybindings();
	}

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
			GI->SavedSensX = NewSens;
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
			GI->SavedSensY = NewSens;
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
		GI->SavedBrightness = NewExposure;
	}

	ApplyBrightness(NewExposure);
	UpdateBrightnessText(NewExposure);
}

void USettingsWidget::ApplyBrightness(float ExposureComp)
{
	for (TObjectIterator<UCameraComponent> CamItr; CamItr; ++CamItr)
	{
		if (CamItr->GetWorld() != GetWorld()) continue;

		FPostProcessSettings& PPS = CamItr->PostProcessSettings;

		PPS.bOverride_AutoExposureBias = true;
		PPS.AutoExposureBias = ExposureComp;

		CamItr->PostProcessBlendWeight = 1.0f;
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
		if (Binding->Name != Data->Name && Binding->Keybind == PressedKey)
		{
			Binding->Keybind = EKeys::Invalid;
			Binding->ReadableBind = TEXT("None");
		}
		else if (Binding->Name == Data->Name)
		{
			Binding->Keybind = PressedKey;
			Binding->ReadableBind = GetReadableKeyName(PressedKey);
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

void USettingsWidget::HandleResolutionSelectionChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
	if (SelectionType == ESelectInfo::Direct) return;
	if (!ResMap.Contains(SelectedItem)) return;

	if (UGameUserSettings* GS = GEngine->GetGameUserSettings())
	{
		const FIntPoint NewRes = ResMap[SelectedItem];

		GS->SetScreenResolution(NewRes);
		GS->ApplyResolutionSettings(false);
		GS->SaveSettings();

		if (URelicRunnersGameInstance* GI = Cast<URelicRunnersGameInstance>(GetGameInstance()))
		{
			GI->Resolution = NewRes;
		}
	}
}

void USettingsWidget::HandleWindowModeSelectionChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
	if(SelectionType == ESelectInfo::OnMouseClick)
		if (UGameUserSettings* GS = GEngine->GetGameUserSettings())
		{
			FIntPoint ResolutionToUse = GS->GetScreenResolution();

			if (URelicRunnersGameInstance* GI = Cast<URelicRunnersGameInstance>(GetGameInstance()))
			{
				if (GI->Resolution.X > 0 && GI->Resolution.Y > 0)
				{
					ResolutionToUse = GI->Resolution;
				}
			}

			EWindowMode::Type Mode = EWindowMode::Windowed;
			if (SelectedItem == "FullScreen")
			{				
				GS->SetScreenResolution(ResolutionToUse);
				GS->SetFullscreenMode(EWindowMode::Fullscreen);
				
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
			if (URelicRunnersGameInstance* GI = Cast<URelicRunnersGameInstance>(GetGameInstance()))
			{
				GI->WindowMode = GS->GetFullscreenMode();
			}
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

//fov settings
void USettingsWidget::OnFOVSliderChanged(float Value)
{

	float NewFOV = FMath::Lerp(MinFOV, MaxFOV, Value);

	if (URelicRunnersGameInstance* GI = Cast<URelicRunnersGameInstance>(GetGameInstance()))
	{
		GI->SavedFOV = NewFOV;
		ApplyFOV(GI->SavedFOV);
		UpdateFOVText(GI->SavedFOV);

	}
	
}

void USettingsWidget::ApplyFOV(float NewFOV)
{

	if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
	{
		if (PC->PlayerCameraManager)
		{
			PC->PlayerCameraManager->SetFOV(NewFOV);
		}
		if (ARelicRunnersCharacter* RRCharacter = Cast<ARelicRunnersCharacter>(PC->GetPawn()))
		{
			if (RRCharacter->GetFollowCamera())
			{
				RRCharacter->GetFollowCamera()->SetFieldOfView(NewFOV);
			}
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
			GI->bSavedMotionBlur = isChecked;

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
			PPS.MotionBlurAmount = bEnabled ? 1.5f : 0.0f;

			Camera->PostProcessBlendWeight = 1.0f;
		}
	}
}
void USettingsWidget::HandleColorModeChanged(FString Item, ESelectInfo::Type SelectionType)
{
	URelicRunnersGameInstance* GI = Cast<URelicRunnersGameInstance>(GetGameInstance());
	if (!GI) return;

	if (Item == "Normal")
	{
		GI->ColorblindMode = EColorblindMode::Normal;
	}
	else if (Item == "Protanopia")
	{
		GI->ColorblindMode = EColorblindMode::Protanopia;
	}
	else if (Item == "Deuteranopia")
	{
		GI->ColorblindMode = EColorblindMode::Deuteranopia;
	}
	else if (Item == "Tritanopia")
	{
		GI->ColorblindMode = EColorblindMode::Tritanopia;
	}
	else
	{
		return;
	}
	if (ARelicRunnersPlayerController* RPC =
		Cast<ARelicRunnersPlayerController>(UGameplayStatics::GetPlayerController(this, 0)))
	{
		RPC->ApplyColorblindMode();
	}
	if (GI->Keys)
	{
		GI->Keys->ColorBlindMode = Item;
	}

	
}

void USettingsWidget::HandleOutputDeviceChanged(FString Item, ESelectInfo::Type SelectionType)
{
	if (SelectionType == ESelectInfo::Direct) return;

	const FString* DeviceId = OutputDeviceMap.Find(Item);
	if (!DeviceId) return;

	FOnCompletedDeviceSwap SwapDelegate;
	SwapDelegate.BindUFunction(this, FName("OnOutputChanged"));

	UAudioMixerBlueprintLibrary::SwapAudioOutputDevice(this, *DeviceId, SwapDelegate);

}

void USettingsWidget::OnAudioDevicesFound(const TArray<FAudioOutputDeviceInfo>& Devices)
{
	if (!OutputDeviceDropDown) return;

	OutputDeviceDropDown->ClearOptions();
	OutputDeviceMap.Empty();

	for (const FAudioOutputDeviceInfo& D : Devices)
	{
		OutputDeviceDropDown->AddOption(D.Name);
		OutputDeviceMap.Add(D.Name, D.DeviceId);
	}

	for (const FAudioOutputDeviceInfo& Device : Devices)
	{
		if (Device.bIsCurrentDevice)
		{
			OutputDeviceDropDown->SetSelectedOption(Device.Name);
			break;
		}
	}
}

void USettingsWidget::OnOutputDevicesSwapped(const FSwapAudioOutputResult& Result)
{
	UE_LOG(LogTemp, Warning, TEXT("Swap finished"));

}

void USettingsWidget::OnMasterVolumeChanged(float Value)
{
	const float NewVolume = FMath::Lerp(MinVolume, MaxVolume, Value);

	if (URelicRunnersGameInstance* GI = Cast<URelicRunnersGameInstance>(GetGameInstance()))
	{
		if (GI->Keys)
		{
			GI->Keys->MasterVolume = NewVolume;
		}
	}

	ApplyMasterVolume(NewVolume);
	UpdateMasterVolumeText(NewVolume);
}

void USettingsWidget::ApplyMasterVolume(float Volume)
{
	if (!MasterSoundClass || !MasterSoundMix) return;

	UGameplayStatics::PushSoundMixModifier(GetWorld(), MasterSoundMix);

	UGameplayStatics::SetSoundMixClassOverride(GetWorld(), MasterSoundMix, MasterSoundClass, Volume, 1.0f, 0.0f, true);
}

void USettingsWidget::UpdateMasterVolumeText(float Volume)
{
	if (!TB_MasterVolumeValue) return;

	const int32 Percent = FMath::RoundToInt(Volume * 100.f);
	TB_MasterVolumeValue->SetText(FText::FromString(FString::Printf(TEXT("%d%%"), Percent)));
}

void USettingsWidget::HandleAntiAliasingChanged(FString Item, ESelectInfo::Type SelectionType)
{
	if (SelectionType == ESelectInfo::Direct) return;

	if (UGameUserSettings* GS = GEngine->GetGameUserSettings())
	{
		int32 Quality = 2; // default High

		if (Item == TEXT("Low")) Quality = 0;
		else if (Item == TEXT("Medium")) Quality = 1;
		else if (Item == TEXT("High")) Quality = 2;
		else if (Item == TEXT("Epic")) Quality = 3;
		else if (Item == TEXT("Cinematic")) Quality = 4;

		GS->SetAntiAliasingQuality(Quality);
		GS->ApplyNonResolutionSettings();
		GS->SaveSettings();
	}
}
void USettingsWidget::OnAutoLootChanged(bool bIsChecked)
{
	if (URelicRunnersGameInstance* GI = Cast<URelicRunnersGameInstance>(GetGameInstance()))
	{
		if (GI->Keys)
		{
			GI->Keys->bAutoLoot = bIsChecked;
			GI->bAutoLoot = bIsChecked;

		}
	}
}
/*void USettingsWidget::HandleLanguageChanged(FString Item, ESelectInfo::Type SelectionType)
{
	if (SelectionType == ESelectInfo::Direct) return;

	URelicRunnersGameInstance* GI = Cast<URelicRunnersGameInstance>(GetGameInstance());
	if (!GI) return;

	if (Item == "English")
	{
		GI->SetLanguage(EGameLanguage::English);
	}
	else if (Item == "French")
	{
		GI->SetLanguage(EGameLanguage::French);
	}
}
*/
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
		GI->bInvertX = isChecked;
	}
}

void USettingsWidget::ToggleInvertedYMouse(bool isChecked)
{
	URelicRunnersGameInstance* GI = Cast<URelicRunnersGameInstance>(GetGameInstance());
	if (GI->Keys)
	{
		GI->Keys->InvertedYMouse = isChecked;
		GI->ApplyKeybindings();
		GI->bInvertY = isChecked;
	}
}



void USettingsWidget::RefreshFOVUI()
{
	if (!FOVSlider) return;

	if (URelicRunnersGameInstance* GI = Cast<URelicRunnersGameInstance>(GetGameInstance()))
	{
		const float CurrentFOV = GI->SavedFOV > 0.f ? GI->SavedFOV : 90.f;
		const float FOVNorm = FMath::Clamp((CurrentFOV - MinFOV) / (MaxFOV - MinFOV), 0.f, 1.f);

		FOVSlider->OnValueChanged.RemoveDynamic(this, &USettingsWidget::OnFOVSliderChanged);
		FOVSlider->SetValue(FOVNorm);
		UpdateFOVText(CurrentFOV);
		FOVSlider->OnValueChanged.AddDynamic(this, &USettingsWidget::OnFOVSliderChanged);
	}
}

void USettingsWidget::RefreshBrightnessUI()
{
	if (!BrightnessSlider) return;

	if (URelicRunnersGameInstance* GI = Cast<URelicRunnersGameInstance>(GetGameInstance()))
	{
		const float CurrentBrightness = GI->SavedBrightness;
		const float BrightnessNorm = FMath::Clamp((CurrentBrightness - MinBrightness) / (MaxBrightnes - MinBrightness), 0.f, 1.f);

		BrightnessSlider->OnValueChanged.RemoveDynamic(this, &USettingsWidget::OnBrightnessChanged);
		BrightnessSlider->SetValue(BrightnessNorm);
		UpdateBrightnessText(CurrentBrightness);
		BrightnessSlider->OnValueChanged.AddDynamic(this, &USettingsWidget::OnBrightnessChanged);
	}
}