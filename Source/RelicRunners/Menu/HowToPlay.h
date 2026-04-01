// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HowToPlay.generated.h"


UCLASS()
class RELICRUNNERS_API UHowToPlay : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	void SetParentMenu(class UMainMenuWidget* InParentMenu);

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UButton* BackButton = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UWidgetSwitcher* ContentSwitcher = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UButton* B_Items = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UButton* B_Objectives = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UButton* B_Upgrades = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UButton* B_Enemies = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UButton* B_Classes = nullptr;

protected:

	UFUNCTION()
	void BackButtonClicked();

	UFUNCTION()
	void OnItemsClicked();

	UFUNCTION()
	void OnObjectivesClicked();

	UFUNCTION()
	void OnUpgradesClicked();

	UFUNCTION()
	void OnEnemiesClicked();

	UFUNCTION()
	void OnClassesClicked();

private:
	UPROPERTY()
	class UMainMenuWidget* ParentMenu;

	void SwitchToPage(int Index);
};