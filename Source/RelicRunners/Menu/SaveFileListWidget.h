// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "Blueprint/UserWidget.h"
#include "SaveFileListWidget.generated.h"

/**
 * 
 */
UCLASS()
class RELICRUNNERS_API USaveFileListWidget : public UUserWidget, public IUserObjectListEntry
{
	GENERATED_BODY()
public:
	virtual void NativeConstruct() override;
	virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;
	void Setup(class USaveFileListData* InData);
	UFUNCTION()
	void OnSaveFileButtonClicked();
	class USaveFileListData* GetData() const;
	void RefreshVisual();
	void SetParent(class UModeSelectionWidget* Parent) { ParentWidget = Parent; };
private:
	class USaveFileListData* SaveFileData;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* TB_Name;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* TB_NumUpgrades;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* TB_TimePlayed;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* TB_CreateNew;

	FString CreateNew;

	class UModeSelectionWidget* ParentWidget;

	UPROPERTY(meta = (BindWidget))
	class UButton* SaveFileButton;
};
