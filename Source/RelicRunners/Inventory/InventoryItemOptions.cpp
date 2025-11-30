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

#include "InventoryItemOptions.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"
#include "Inventory.h"
#include "RelicRunners/Item/ItemData.h"
#include <Blueprint/WidgetLayoutLibrary.h>
#include "Blueprint/WidgetBlueprintLibrary.h"             

UInventoryItemOptions* UInventoryItemOptions::CurrentPopup = nullptr;

void UInventoryItemOptions::NativeConstruct()
{
    Super::NativeConstruct();

    if (EquipButton)
    {
        EquipButton->OnClicked.AddDynamic(this, &UInventoryItemOptions::OnEquipClicked);
    }
    if (UnequipButton)
    {
        UnequipButton->OnClicked.AddDynamic(this, &UInventoryItemOptions::OnUnequipClicked);
    }
    if (DropButton)
    {
        DropButton->OnClicked.AddDynamic(this, &UInventoryItemOptions::OnDropClicked);
    }
    if (BuyButton)
    {
        BuyButton->OnClicked.AddDynamic(this, &UInventoryItemOptions::OnBuyClicked);
    }
    if (SellButton)
    {
        SellButton->OnClicked.AddDynamic(this, &UInventoryItemOptions::OnSellClicked);
    }
    if (ForgeButton)
    {
        ForgeButton->OnClicked.AddDynamic(this, &UInventoryItemOptions::OnForgeClicked);
    }
}

void UInventoryItemOptions::Setup(UItemObject* InItem)
{
    InitialScreenPosition = UWidgetLayoutLibrary::GetMousePositionOnViewport(this);
    Item = InItem;
    CurrentPopup = this;
}

void UInventoryItemOptions::NativeDestruct()
{
    Super::NativeDestruct();
    if (CurrentPopup == this)
    {
        CurrentPopup = nullptr;
    }
}

void UInventoryItemOptions::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    // Close if mouse moved too far
    FVector2D CurrentMousePos = UWidgetLayoutLibrary::GetMousePositionOnViewport(this);
    float Distance = FVector2D::Distance(CurrentMousePos, InitialScreenPosition);

    float PixelDistance = 100.0f;
    if (Distance > PixelDistance)
    {
        ClosePopup();
        return;
    }

    if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
    {
        if (PC->WasInputKeyJustPressed(EKeys::LeftMouseButton) ||
            PC->WasInputKeyJustPressed(EKeys::RightMouseButton))
        {
            ClosePopup();
        }
    }
}

void UInventoryItemOptions::ConfigureButtons(bool ShowEquip, bool ShowUnequip, bool ShowSell, bool ShowBuy)
{
    if (EquipButton)
    {
        EquipButton->SetVisibility(ShowEquip ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
    }

    if (UnequipButton)
    {
        UnequipButton->SetVisibility(ShowUnequip ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
    }

    if (DropButton)
    {
        if (ShowEquip || ShowUnequip)
        {
            DropButton->SetVisibility(ESlateVisibility::Visible);
        }
        else
        {
            DropButton->SetVisibility(ESlateVisibility::Collapsed);
        }
    }

    if (SellButton)
    {
        SellButton->SetVisibility(ShowSell ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
    }

    if (BuyButton)
    {
        BuyButton->SetVisibility(ShowBuy ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
    }
}

void UInventoryItemOptions::SetForgeButtonVisibility(bool value)
{
    if (ForgeButton)
    {
        ForgeButton->SetVisibility(value ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
    }
}

void UInventoryItemOptions::ClosePopup()
{
    if (CurrentPopup == this)
    {
        CurrentPopup = nullptr;
    }

    RemoveFromParent();
}

void UInventoryItemOptions::CloseAnyOpenPopup()
{
    if (CurrentPopup && CurrentPopup->IsInViewport())
    {
        CurrentPopup->RemoveFromParent();
        CurrentPopup = nullptr;
    }
}

void UInventoryItemOptions::OnEquipClicked()
{
    if (!Item) return;

    if (UInventory* inventory = UInventory::Get(this))
    {
        inventory->EquipItem(Item);
        RemoveFromParent();
    }
}

void UInventoryItemOptions::OnUnequipClicked()
{
    if (!Item) return;

    if (UInventory* Inventory = UInventory::Get(this))
    {
        Inventory->UnequipItem(Item);
        RemoveFromParent();
    }
}

void UInventoryItemOptions::OnDropClicked()
{
    if (!Item) return;

    if (UInventory* Inventory = UInventory::Get(this))
    {
        Inventory->DropItem(Item);
        RemoveFromParent();
    }
}

void UInventoryItemOptions::OnBuyClicked()
{
    if (!Item) return;

    if (UInventory* Inventory = UInventory::Get(this))
    {
        Inventory->BuyItem(Item);
        RemoveFromParent();
    }
}

void UInventoryItemOptions::OnSellClicked()
{
    if (!Item) return;

    if (UInventory* Inventory = UInventory::Get(this))
    {
        Inventory->SellItem(Item);
        RemoveFromParent();
    }
}

void UInventoryItemOptions::OnForgeClicked()
{
    if (UInventory* Inventory = UInventory::Get(this))
    {
        Inventory->ForgeItem(Item);
        RemoveFromParent();
    }
}
