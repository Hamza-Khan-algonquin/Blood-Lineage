// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "EditSaveFilePopupWidget.generated.h"

/**
 * 
 */
UCLASS()
class RELICRUNNERS_API UEditSaveFilePopupWidget : public UUserWidget
{
	GENERATED_BODY()
public:
    UPROPERTY(BlueprintReadWrite)
    class USaveFileListWidget* TargetSlot;
    UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
    class UEditableTextBox* NameTextBox = nullptr;
    UFUNCTION()
    void OnNameTextChanged(const FText& NewText);

    UPROPERTY(meta = (BindWidget))
    class UButton* DeleteButton;

    UPROPERTY(meta = (BindWidget))
    class UButton* CancelButton;

    UPROPERTY(meta = (BindWidget))
    class UButton* ConfirmButton;
    UFUNCTION()
    void OnConfirmClicked();
    virtual void NativeConstruct() override;

    UFUNCTION(BlueprintCallable)
    void OnDeleteClicked();

    UFUNCTION()
    void OnCancelClicked();
};
