// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PauseMenu.generated.h"

/**
 * 
 */
UCLASS()
class RELICRUNNERS_API UPauseMenu : public UUserWidget
{
	GENERATED_BODY()
public:

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UButton* ResumeButton = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UButton* SettingsButton = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UButton* RecallButton = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UButton* QuitButton = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pause Menu")
	TSubclassOf<class USettingsWidget> SettingsWidgetClass;

	UPROPERTY()
	class USettingsWidget* SettingsWidget;

	UPROPERTY()
	class UPauseMenuWidget* PauseParentMenu;

protected:
	virtual void NativeConstruct() override;

public:
	void SetParentMenu(class UPauseMenuWidget* InParentMenu);

	UFUNCTION()
	void OnResumeButtonClicked();

	UFUNCTION()
	void OnSettingsButtonClicked();

	UFUNCTION()
	void OnRecallButtonClicked();

	UFUNCTION()
	void OnQuitButtonClicked();
};
