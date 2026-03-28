// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CreditsWidget.generated.h"

/**
 * 
 */
UCLASS()
class RELICRUNNERS_API UCreditsWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	virtual void NativeConstruct() override;
	void SetParentMenu(class UMainMenuWidget* InParentMenu);
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UButton* BackButton = nullptr;
	UPROPERTY()
	class UMainMenuWidget* ParentMenu;

	UFUNCTION(BlueprintCallable)
	void ApplyColorblindStyle();
protected:
	UFUNCTION()
	void BackButtonClicked();
};
