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
	UFUNCTION(BlueprintCallable)
	void Setup(class USaveFileListData* InData);
	UFUNCTION(BlueprintCallable)
	void OnSaveFileButtonClicked();
	UFUNCTION(BlueprintCallable)
	void OnEditButtonClicked();
	void SetSelected(bool bSelected);
	void ResetBorderVisibility();
	void FinalizeEdit(const FString& PlayerName);
	void FinalizeSaveCreation(const FString& PlayerName);
	void SpawnEmptySlot();
	void RefreshVisual();
	void SetParent(class UModeSelectionWidget* Parent) { ParentWidget = Parent; };
	class UModeSelectionWidget* ParentWidget;
	UPROPERTY(BlueprintReadWrite)
	class USaveFileListData* SaveFileData;
	public :


	UPROPERTY(meta = (BindWidget), BlueprintReadWrite)
	class UTextBlock* TB_Name;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* TB_NumUpgrades;

	UPROPERTY(meta = (BindWidget), BlueprintReadWrite)
	class UTextBlock* TB_TimePlayed;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* TB_CreateNew;

	UPROPERTY(meta = (BindWidget))
	class UBorder* B_SaveFile;
	UPROPERTY(meta = (BindWidget))
	class UBorder* B_Upgrades;
	UPROPERTY(meta = (BindWidget))
	class UBorder* B_Edit;



	UPROPERTY(meta = (BindWidget), BlueprintReadWrite)
	class UButton* SaveFileButton;
	UPROPERTY(meta = (BindWidget))
	class UButton* EditButton;
	UPROPERTY(meta = (BindWidget))
	class UButton* Upgrades;
};
