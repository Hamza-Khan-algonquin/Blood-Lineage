// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HowToPlay.generated.h"

/**
 * 
 */
UCLASS()
class RELICRUNNERS_API UHowToPlay : public UUserWidget
{
	GENERATED_BODY()
public:
	virtual void NativeConstruct() override;
	void SetParentMenu(class UMainMenuWidget* InParentMenu);
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UButton* BackButton = nullptr;
	UPROPERTY()
	class UMainMenuWidget* ParentMenu;
protected:
	UFUNCTION()
	void BackButtonClicked();
};
