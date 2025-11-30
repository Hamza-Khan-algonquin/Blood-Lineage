// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SaveFilePopupWidget.generated.h"

/**
 * 
 */
UCLASS()
class RELICRUNNERS_API USaveFilePopupWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UEditableTextBox* NameTextBox = nullptr;

	UPROPERTY(BlueprintReadWrite)
	class USaveFileListWidget* TargetSlot;

	virtual void NativeConstruct() override;

	UPROPERTY(meta = (BindWidget))
	class UButton* ConfirmButton;

	UPROPERTY(meta = (BindWidget))
	class UButton* CancelButton;

	UFUNCTION(BlueprintCallable)
	void OnConfirmClicked();

	UFUNCTION(BlueprintCallable)
	void OnCancelClicked();
	UFUNCTION(BlueprintCallable)
	void OnNameTextChanged(const FText& NewText);
};
