// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UpgradesWidget.generated.h"

/**
 * 
 */
UCLASS()
class RELICRUNNERS_API UUpgradesWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	virtual void NativeConstruct() override;
	void SetParentMenu(class UMainMenuWidget* InParentMenu);
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UButton* BackButton = nullptr;
	UPROPERTY()
	class UMainMenuWidget* ParentMenu;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UImage* IconHP = nullptr;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UImage* IconDEF = nullptr;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UImage* IconLUCK = nullptr;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UImage* IconSTR = nullptr;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UImage* IconAGI = nullptr;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UImage* IconINT = nullptr;
protected:
	UFUNCTION()
	void BackButtonClicked();


};
