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
    if (RemoveButton)
    {
        RemoveButton->OnClicked.AddDynamic(this, &UInventoryItemOptions::OnRemoveClicked);
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

void UInventoryItemOptions::ApplyConfig(const FItemOptionsUIConfig& Config)
{
    SetEquipButtonVisibility(Config.ShowEquip);
    SetUnequipButtonVisibility(Config.ShowUnequip);
    SetDropButtonVisibility(Config.ShowDrop);
    SetSellButtonVisibility(Config.ShowSell);
    SetBuyButtonVisibility(Config.ShowBuy);
    SetForgeButtonVisibility(Config.ShowForge);
    SetRemoveButtonVisibility(Config.ShowRemove);
}

static ESlateVisibility BoolToVisibility(bool b)
{
    return b ? ESlateVisibility::Visible : ESlateVisibility::Collapsed;
}

void UInventoryItemOptions::SetEquipButtonVisibility(bool Visible)
{
    if (EquipButton)
    {
        EquipButton->SetVisibility(BoolToVisibility(Visible));
    }
}

void UInventoryItemOptions::SetUnequipButtonVisibility(bool Visible)
{
    if (UnequipButton)
    {
        UnequipButton->SetVisibility(BoolToVisibility(Visible));
    }
}

void UInventoryItemOptions::SetDropButtonVisibility(bool Visible)
{
    if (DropButton)
    {
        DropButton->SetVisibility(BoolToVisibility(Visible));
    }
}

void UInventoryItemOptions::SetSellButtonVisibility(bool Visible)
{
    if (SellButton)
    {
        SellButton->SetVisibility(BoolToVisibility(Visible));
    }
}

void UInventoryItemOptions::SetBuyButtonVisibility(bool Visible)
{
    if (BuyButton)
    {
        BuyButton->SetVisibility(BoolToVisibility(Visible));
    }
}

void UInventoryItemOptions::SetForgeButtonVisibility(bool Visible)
{
    if (ForgeButton)
    {
        ForgeButton->SetVisibility(BoolToVisibility(Visible));
    }
}

void UInventoryItemOptions::SetRemoveButtonVisibility(bool Visible)
{
    if (RemoveButton)
    {
        RemoveButton->SetVisibility(BoolToVisibility(Visible));
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

void UInventoryItemOptions::OnRemoveClicked()
{
    if (UInventory* Inventory = UInventory::Get(this))
    {
        Inventory->RemoveForgeItem(Item);
        RemoveFromParent();
    }
}
