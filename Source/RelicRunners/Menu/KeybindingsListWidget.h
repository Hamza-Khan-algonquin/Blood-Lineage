// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "KeybindingsListData.h"
#include "KeybindingsListWidget.generated.h"

/**
 * 
 */
UCLASS()
class RELICRUNNERS_API UKeybindingsListWidget : public UUserWidget, public IUserObjectListEntry
{
	GENERATED_BODY()
public:
	virtual void NativeConstruct() override;
	virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;
	void Setup(class UKeybindingsListData* InData);
	UFUNCTION()
	void OnBindButtonClicked();
	class UKeybindingsListData* GetKeybindData() const;
	void RefreshVisual();

private:
	class UKeybindingsListData* KeybindData;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* TB_Name;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* TB_Bind;

	UPROPERTY(meta = (BindWidget))
	class UButton* B_BindButton;


};
