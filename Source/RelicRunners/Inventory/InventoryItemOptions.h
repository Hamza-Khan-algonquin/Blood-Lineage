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
#include "InventoryItemOptions.generated.h"

class UIVendor;

UCLASS()
class RELICRUNNERS_API UInventoryItemOptions : public UUserWidget
{
    GENERATED_BODY()
public:

    //Functions
    UFUNCTION(BlueprintCallable, Category = "Item Options")
    void Setup(class UItemObject* InItem);
    void ConfigureButtons(bool ShowEquip, bool ShowUnequip, bool ShowSell, bool ShowBuy);
    void SetForgeButtonVisibility(bool value);
    void ClosePopup();
    static void CloseAnyOpenPopup();
    UFUNCTION()
    void OnEquipClicked();
    UFUNCTION()
    void OnUnequipClicked();
    UFUNCTION()
    void OnDropClicked();
    UFUNCTION()
    void OnBuyClicked();
    UFUNCTION()
    void OnSellClicked();
    UFUNCTION()
    void OnForgeClicked();

protected:

    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

    //UI Elements
    UPROPERTY(meta = (BindWidget))
    class UButton* EquipButton;

    UPROPERTY(meta = (BindWidget))
    class UButton* UnequipButton;

    UPROPERTY(meta = (BindWidget))
    class UButton* DropButton;

    UPROPERTY(meta = (BindWidget))
    class UButton* BuyButton;

    UPROPERTY(meta = (BindWidget))
    class UButton* SellButton;
    UPROPERTY(meta = (BindWidget))
    class UButton* ForgeButton;
private:

    //Properties
    FVector2D InitialScreenPosition;
    static class UInventoryItemOptions* CurrentPopup;
    UPROPERTY()
    class UItemObject* Item;
};
