// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
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

	UFUNCTION(BlueprintCallable)
	void ToggleInvertedXMouse();

	UFUNCTION(BlueprintCallable)
	void ToggleInvertedYMouse();

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

	UPROPERTY(meta = (BindWidget))
	class UComboBoxString* ResolutionDropDown;

	UFUNCTION()
	void HandleSelectionChanged(FString Item, ESelectInfo::Type SelectionType);

protected:
	UFUNCTION()
	void BackButtonClicked();



};
