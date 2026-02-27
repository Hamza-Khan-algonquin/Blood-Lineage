// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InventoryDragVisual.generated.h"

/**
 * 
 */
UCLASS()
class RELICRUNNERS_API UInventoryDragVisual : public UUserWidget
{
	GENERATED_BODY()
public:
	UFUNCTION()
	void OnSetupVisual(class UItemObject* Item);

    UPROPERTY(meta = (BindWidget))
    class UTextBlock* TB_Level;
    UPROPERTY(meta = (BindWidget))
    class UTextBlock* TB_GoldPrice;
    UPROPERTY(meta = (BindWidget))
    class UBorder* B_Item;
    UPROPERTY(meta = (BindWidget))
    class UBorder* B_NewItem;
    UPROPERTY(meta = (BindWidget))
    class UBorder* B_GoldPrice;
    UPROPERTY(meta = (BindWidget))
    class UBorder* B_Color;
    UPROPERTY(meta = (BindWidget))
    class UImage* I_Item;
    UPROPERTY(meta = (BindWidget))
    class UImage* ItemFrame;
    UPROPERTY(meta = (BindWidget))
    class UBorder* B_Level;
    UPROPERTY(meta = (BindWidget))
    class UBorder* SelectedBorder;
    UPROPERTY()
    class UItemObject* ItemObject;
};
