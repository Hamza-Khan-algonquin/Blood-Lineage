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

#include "InventorySlotsEntry.h"
#include "Styling/SlateTypes.h"
#include "Components/Image.h"
#include "Components/Border.h"
#include "Components/HorizontalBox.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/TileView.h"
#include "Components/TextBlock.h"
#include "Components/CanvasPanel.h"
#include "SlateBasics.h"
#include "InventoryToolTip.h"
#include "InventoryItemOptions.h"
#include "InventorySortingOptions.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include <Blueprint/WidgetBlueprintLibrary.h>
#include "InventoryDragOperation.h"
#include "RelicRunners/Item/ItemStats.h"
#include "RelicRunners/Item/ItemData.h"
#include "Inventory.h"
#include <RelicRunners/PlayerController/RelicRunnersPlayerController.h>
#include "InventoryComponent.h"
#include <RelicRunners/RelicRunnersCharacter.h>
#include "RelicRunners/PlayerState/RelicRunnersPlayerState.h"

void UInventorySlotsEntry::NativeConstruct()
{
    Super::NativeConstruct();

    InventoryItemOptionsClass = LoadClass<UInventoryItemOptions>(nullptr, TEXT("/Game/Inventory/BP_InventoryItemOptions.BP_InventoryItemOptions_C"));
}

void UInventorySlotsEntry::NativeOnListItemObjectSet(UObject* ListItemObject)
{
    Item = Cast<UItemObject>(ListItemObject);
    if (!Item) return;

    ParentInventory = GetTypedOuter<UInventory>();

    TB_Level->SetText(FText::FromString(FString::FromInt(Item->GetLevel())));
    TB_Level->SetColorAndOpacity(FLinearColor(0.7f, 0.7f, 0.7f, 1));

    //Set border color by rarity
    FLinearColor ItemFrameColor = ItemStats::GetRarityDataMap()[Item->GetRarity()].Color;
    B_Item->SetBrushColor(ItemFrameColor);
    B_Color->SetBrushColor(ItemFrameColor);
    B_Level->SetBrushColor(ItemFrameColor);
    ItemFrame->SetBrushTintColor(ItemFrameColor);
    I_Item->SetBrushFromTexture(Item->ItemData.Icon);

    TB_GoldPrice->SetText(FText::FromString(FString::FromInt(Item->ItemData.Gold)));

    if(Item->IsNewItem) B_NewItem->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
    else B_NewItem->SetVisibility(ESlateVisibility::Collapsed);
}

FReply UInventorySlotsEntry::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    if (InMouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton))
    {
        IsPressed = true;
        IsDragging = false;
        PressedTime = GetWorld()->GetRealTimeSeconds();

        FEventReply EventReply = UWidgetBlueprintLibrary::DetectDragIfPressed(InMouseEvent, this, EKeys::LeftMouseButton);
        return EventReply.NativeReply;
    }

    return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

FReply UInventorySlotsEntry::NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    if (!IsPressed) return Super::NativeOnMouseButtonUp(InGeometry, InMouseEvent);

    IsPressed = false;

    float HeldDuration = GetWorld()->GetRealTimeSeconds() - PressedTime;

    if (!IsDragging && HeldDuration < 0.25f)
    {
        OnEntryClicked();
    }

    IsDragging = false;
    return FReply::Handled();
}

void UInventorySlotsEntry::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation)
{
    if (!Item) return;

    IsDragging = true;
    
    //Start drag operation
    UInventoryDragOperation* DragOp = NewObject<UInventoryDragOperation>();
    DragOp->Payload = Item;
    DragOp->DefaultDragVisual = this;
    DragOp->Pivot = EDragPivot::MouseDown;

    //Get starting location 
    UTileView* TileView = Cast<UTileView>(GetOwningListView());
    if (TileView->GetName() == "InventorySlots")
    {
        DragOp->FromWhere = UInventoryDragOperation::Locations::Inventory;
    }
    else if (TileView->GetName() == "VendorSlots")
    {
        DragOp->FromWhere = UInventoryDragOperation::Locations::Shop;
    }
    OutOperation = DragOp;
}

void UInventorySlotsEntry::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    Super::NativeOnMouseEnter(InGeometry, InMouseEvent);

    if (!Item || !TooltipWidgetClass || !B_Item) return;

    UInventoryToolTip* ToolTip = CreateWidget<UInventoryToolTip>(GetWorld(), TooltipWidgetClass);
    if (!ToolTip) return;

    FLinearColor RarityColor = ItemStats::GetRarityDataMap()[Item->GetRarity()].Color;

    //Find equipped item of same type
    ARelicRunnersPlayerController* PC = Cast<ARelicRunnersPlayerController>(GetWorld()->GetFirstPlayerController());
    UItemObject* EquippedItem = nullptr;
    if (ARelicRunnersPlayerState* PS = GetOwningPlayerState<ARelicRunnersPlayerState>())
    {
        if (UInventoryComponent* InvComp = PS->GetInventoryComponent())
        {
            EquippedItem = InvComp->GetEquippedItemByType(Item->GetItemType());
            if (EquippedItem == Item) EquippedItem = nullptr;
        }
    }

    ToolTip->Setup(Item, RarityColor, EquippedItem);
    B_Item->SetToolTip(ToolTip);

    HoveredTime = GetWorld()->GetRealTimeSeconds();
}

void UInventorySlotsEntry::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
    Super::NativeOnMouseLeave(InMouseEvent);
    B_Item->SetToolTip(nullptr);

    float HoveredTimeOnLeave = GetWorld()->GetRealTimeSeconds() - HoveredTime;
    HoveredTime = 0;
    if (Item->IsNewItem && HoveredTimeOnLeave >= 0.25f)
    {
        Item->IsNewItem = false;
        B_NewItem->SetVisibility(ESlateVisibility::Collapsed);
        if (ParentInventory && ParentInventory->InventoryComponent)
        {
            ParentInventory->InventoryComponent->SortInventoryByCurrentMethod();
        }
    }
}

void UInventorySlotsEntry::ToggleGoldPriceBorder(bool value)
{
    if (value)
    {
        B_GoldPrice->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
    }
    else
    {
        B_GoldPrice->SetVisibility(ESlateVisibility::Collapsed);
    }
}

void UInventorySlotsEntry::OnEntryClicked()
{
    UInventoryItemOptions::CloseAnyOpenPopup();
    UInventorySortingOptions::CloseAnyOpenPopup();

    if (!Item) return;
    
    //Determining if vendor is active and which tileview its part of.
    bool VendorActive = false;
    if (ParentInventory)
    {
        if (ParentInventory->ShopCanvas->IsVisible())
        {
            VendorActive = true;
        }
    }

    UTileView* TileView = Cast<UTileView>(GetOwningListView());

    if (VendorActive)
    {
        if (TileView->GetName() == "InventorySlots")
        {
            ParentInventory->CreateItemOptionsPopup(Item, FItemOptionsUIConfig::InventoryItemWithForge());
        }
        else if (TileView->GetName() == "VendorSlots")
        {
            ParentInventory->CreateItemOptionsPopup(Item, FItemOptionsUIConfig::VendorItem());
        }
    }
    else
    {
        ParentInventory->CreateItemOptionsPopup(Item, FItemOptionsUIConfig::InventoryItem());
    }
}