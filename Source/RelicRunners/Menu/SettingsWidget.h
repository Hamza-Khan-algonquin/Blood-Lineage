// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include <Components/Slider.h>
#include <Components/TextBlock.h>
#include <Components/CheckBox.h>
#include "AudioMixerBlueprintLibrary.h"
#include "SettingsWidget.generated.h"



/**
 * 
 */
UCLASS()
class RELICRUNNERS_API USettingsWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	virtual void NativeConstruct() override;
	FReply HandleKeyBindPressed(FKey PressedKey);

	UFUNCTION(BlueprintCallable)
	void UpdateBinds();

	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	void SetParentMenu(class UMainMenuWidget* InParentMenu);
	void SetParentMenu(class UPauseMenuWidget* InParentMenu);

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UTextBlock* TB_BackButton = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UButton* BackButton = nullptr;
	UPROPERTY()
	class UMainMenuWidget* MainParentMenu;
	UPROPERTY()
	class UPauseMenuWidget* PauseParentMenu;

	UPROPERTY(meta = (BindWidget))

	class UTileView* KeybindingsTileView;

	//mouse controls
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UCheckBox* B_InvertedXMouse = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UCheckBox* B_InvertedYMouse = nullptr;
	UFUNCTION(BlueprintCallable)
	void ToggleInvertedXMouse(bool isChecked);

	UFUNCTION(BlueprintCallable)
	void ToggleInvertedYMouse(bool isChecked);

	

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	USlider* XSlider = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	USlider* YSlider = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UTextBlock* TB_XSensValue = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UTextBlock* TB_YSensValue = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|Mouse")
	float MinSens = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|Mouse")
	float MaxSens = 1.0f;

	UFUNCTION()
	void OnSensXChanged(float Value);

	UFUNCTION()
	void OnSensYChanged(float Value);

	void UpdateSensText(UTextBlock* TextBlock, float Sens);

	//brightness setting
	UPROPERTY(BlueprintReadOnly, meta =(BindWidget))
	USlider* BrightnessSlider = nullptr;

	UPROPERTY(BlueprintReadOnly, meta =(BindWidget))
	UTextBlock* TB_BrightnessValue = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|Brightness")
	float MinBrightness = -2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|Brightness")
	float MaxBrightnes = 0.f;

	UFUNCTION()
	void OnBrightnessChanged(float Value);

	void ApplyBrightness(float ExposureComp);
	void UpdateBrightnessText(float ExposureComp);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|Keybindings")
	TArray<class UKeybindingsListData*> DefaultKeybindings;

	UPROPERTY()
	class UKeybindingsListWidget* WaitingForKeyEntry = nullptr;

	void StartRebinding(class UKeybindingsListWidget* Entry);
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

	UPROPERTY(meta = (BindWidget))
	class UButton* B_RestoreDefaults;

	UFUNCTION()
	void OnRestoreDefaultsClicked();

	void InitializeDefaultKeybindings();

	//Resolution setting
	UPROPERTY(meta = (BindWidget))
	class UComboBoxString* ResolutionDropDown = nullptr;

	TMap<FString, FIntPoint> ResMap;
	
	UFUNCTION()
	void HandleResolutionSelectionChanged(FString SelectedItem, ESelectInfo::Type SelectionType);

	//Window mode setting
	UPROPERTY(meta = (BindWidget))
	class UComboBoxString* WindowModeDropDown;
	
	UFUNCTION()
	void HandleWindowModeSelectionChanged(FString SelectedItem, ESelectInfo::Type SelectionType);

	void FillWindowOptions();
	void SetWindowMode();


	//FOV SLIDER setting
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	USlider* FOVSlider = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category ="Settings|FOV")
	float MaxFOV = 85.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|FOV")
	float MinFOV = 80.f;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UTextBlock* TB_FOVValue = nullptr;

	void UpdateFOVText(float NewFOV);

	UFUNCTION()
	void OnFOVSliderChanged(float Value);

	void ApplyFOV(float NewFOV);


	//Motion blur setting

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UCheckBox* B_MotionBlur = nullptr;

	UFUNCTION()
	void OnMotionBlurClicked(bool isChecked);

	void ApplyMotionBlur(bool bEnabled);

	//Colourblind settings
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UComboBoxString* ColourModeDropDown = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Settings|Colour", meta = (AllowPrivateAccess="true"))
	UTexture2D* LUT_Normal = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Settings|Colour", meta = (AllowPrivateAccess = "true"))
	UTexture2D* LUT_Protanopia = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Settings|Colour", meta = (AllowPrivateAccess = "true"))
	UTexture2D* LUT_Deuternopia = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Settings|Colour", meta = (AllowPrivateAccess = "true"))
	UTexture2D* LUT_Tritanopia = nullptr;

	UFUNCTION()
	void HandleColorModeChanged(FString Item, ESelectInfo::Type SelectionType);
	
	void ApplyColorLUT(UTexture* LUT);

	//Output devices
	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	UComboBoxString* OutputDeviceDropDown = nullptr;

	TMap<FString, FString> OutputDeviceMap;

	UFUNCTION()
	void HandleOutputDeviceChanged(FString Item, ESelectInfo::Type SelectionType);

	UFUNCTION()
	void OnAudioDevicesFound(const TArray<FAudioOutputDeviceInfo>& Devices);

protected:
	UFUNCTION()
	void BackButtonClicked();



};
