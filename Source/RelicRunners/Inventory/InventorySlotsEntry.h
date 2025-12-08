/************************************************************************************
 * Author:   Tristan Anglin
 * Year:     2025
 *
 * Notice:
 *   This source code is the intellectual property of Tristan Anglin.
 *   It is provided solely for use within:
 *     - The Relic Runners personal project
 *     - The Almost A Dozen project
 *
 *   Any use, distribution, or modification outside of these projects
 *   is strictly prohibited without explicit written permission.
 *
 *   © 2025 Tristan Anglin. All rights reserved.
 ************************************************************************************/

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "InventorySlotsEntry.generated.h"

UCLASS()
class RELICRUNNERS_API UInventorySlotsEntry : public UUserWidget, public IUserObjectListEntry
{
    GENERATED_BODY()

public:
    //Functions
    UFUNCTION()
    void OnEntryClicked();
    void ToggleGoldPriceBorder(bool value);

    //Properties
    bool IsPressed = false;
    bool IsDragging = false;
    float PressedTime = 0.0f;
    float HoveredTime = 0.0f;
    UPROPERTY()
    class UInventory* ParentInventory;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tooltip")
    TSubclassOf<UUserWidget> TooltipWidgetClass;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Popup")
    TSubclassOf<class UInventoryItemOptions> InventoryItemOptionsClass;
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

    UPROPERTY()
    class UItemObject* Item;

protected:
    virtual void NativeConstruct() override;
    virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;
    virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
    virtual FReply NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
    virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;
    virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
    virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;
};
