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
 *   � 2025 Tristan Anglin. All rights reserved.
 ************************************************************************************/

#include "Inventory.h"
#include "RelicRunners/Vendor/Vendor.h"
#include "Components/TileView.h"
#include "Components/TextBlock.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/HorizontalBox.h"
#include "Components/Button.h"
#include "Components/Border.h"
#include "Components/VerticalBox.h"
#include "Components/Image.h"
#include "Components/Throbber.h"
#include "Components/CanvasPanel.h"
#include "InventoryToolTip.h"
#include "InventorySortingOptions.h"
#include "InventoryComponent.h"
#include "Styling/SlateTypes.h"
#include "SlateBasics.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Kismet/KismetRenderingLibrary.h"
#include "RelicRunners/RelicRunnersCharacter.h"
#include "InventoryItemOptions.h"
#include "RelicRunners/Item/ItemActor.h"
#include "RelicRunners/Item/ItemStats.h"
#include "RelicRunners/Item/ItemData.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Blueprint/DragDropOperation.h"
#include "RelicRunners/PlayerPreview/PlayerPreview.h"
#include "RelicRunners/PlayerState/RelicRunnersPlayerState.h"
#include "RelicRunners/PlayerController/RelicRunnersPlayerController.h"
#include <RelicRunners/Game/RelicRunnersGameInstance.h>
#include "InventoryDragOperation.h"
#include "InventorySlotsEntry.h"
#include "RelicRunners/Menu/StandardTooltip.h"
#include <Blueprint/SlateBlueprintLibrary.h>
#include "Components/AudioComponent.h"
#include "Animation/WidgetAnimation.h"
#include "RelicRunners/Menu/SaveFileListData.h"
#include <ClassData.h>
#include "ClassInfo.h"
#include "InventoryDragVisual.h"
#include "Layout/Geometry.h" 

void UInventory::NativeConstruct()
{
    Super::NativeConstruct();

    TB_InventorySlots->SetText(FText::FromString(FString::Printf(TEXT("%d / %d"), 0, 0)));

    InventoryItemOptionsClass = LoadClass<UInventoryItemOptions>(nullptr, TEXT("/Game/Inventory/BP_InventoryItemOptions.BP_InventoryItemOptions_C"));
    InventorySortingOptionsClass = LoadClass<UInventorySortingOptions>(nullptr, TEXT("/Game/Inventory/BP_InventorySortingOptions.BP_InventorySortingOptions_C"));

    if (B_SortingType) B_SortingType->OnClicked.AddDynamic(this, &UInventory::SortingTypeClicked);
    if (B_AutoEquip) B_AutoEquip->OnClicked.AddDynamic(this, &UInventory::OnAutoEquip);

    //Vendor
    if (ShopCanvas) ShopCanvas->SetVisibility(ESlateVisibility::Collapsed);
    if (ForgeCanvas) ForgeCanvas->SetVisibility(ESlateVisibility::Collapsed);
    if (VendorSlots) VendorSlots->ClearListItems();
    if (FuseButton) FuseButton->OnClicked.AddDynamic(this, &UInventory::OnFuseButtonPressed);

    if (B_Odds) B_Odds->SetVisibility(ESlateVisibility::Hidden);
    if (B_Odds1) B_Odds1->SetVisibility(ESlateVisibility::Hidden);
    if (B_Odds2) B_Odds2->SetVisibility(ESlateVisibility::Hidden);
}

void UInventory::OnAutoEquip()
{
    InventoryComponent->RequestAutoEquip();
}

FEventReply UInventory::OnForgeItem1Clicked(FGeometry MyGeometry, const FPointerEvent& MouseEvent)
{
    if (MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton && InventoryComponent->ForgeItemLeft)
    {
        CreateItemOptionsPopup(InventoryComponent->ForgeItemLeft, FItemOptionsUIConfig::ForgeItem(), MyGeometry);
        return UWidgetBlueprintLibrary::Handled();
    }

    return UWidgetBlueprintLibrary::Unhandled();
}

FEventReply UInventory::OnForgeItem2Clicked(FGeometry MyGeometry, const FPointerEvent& MouseEvent)
{
    FEventReply Reply;

    if (MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton && InventoryComponent->ForgeItemRight)
    {
        CreateItemOptionsPopup(InventoryComponent->ForgeItemRight, FItemOptionsUIConfig::ForgeItem(), MyGeometry);
        return UWidgetBlueprintLibrary::Handled();
    }

    return UWidgetBlueprintLibrary::Unhandled();
}

FEventReply UInventory::OnForgeItemResultClicked(FGeometry MyGeometry, const FPointerEvent& MouseEvent)
{
    FEventReply Reply;

    if (MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton && InventoryComponent->ForgeItemResult)
    {
        CreateItemOptionsPopup(InventoryComponent->ForgeItemResult, FItemOptionsUIConfig::ForgeResultItem(), MyGeometry);
        return UWidgetBlueprintLibrary::Handled();
    }

    return UWidgetBlueprintLibrary::Unhandled();
}

void UInventory::ToggleShopUI(bool value)
{
    if (!ShopCanvas) return;

    if (!value)
    {
        ShopCanvas->SetVisibility(ESlateVisibility::Collapsed);
        VendorSlots->ClearListItems();
        CurrentVendor = nullptr;
    }
    else
    {
        ShopCanvas->SetVisibility(ESlateVisibility::Visible);

        if (ForgeCanvas->IsVisible()) ToggleForgeUI(false);
    }
}

void UInventory::ToggleForgeUI(bool value)
{
    if (!ForgeCanvas) return;

    if (!value)
    {
        ForgeCanvas->SetVisibility(ESlateVisibility::Collapsed);
        CurrentVendor = nullptr;

        if (InventoryComponent) InventoryComponent->Server_ClearAllForgeSlots();

        if (ForgeCompleted) StopAnimation(ForgeCompleted);
        UpdateForgeUI();
    }
    else
    {
        ForgeCanvas->SetVisibility(ESlateVisibility::Visible);

        UpdateForgeUI();

        if (ShopCanvas->IsVisible()) ToggleShopUI(false);
    }
}

void UInventory::DisplaySelectedVendorItems(const TArray<FItemData>& Items, AVendor* Vendor)
{
    if (!Vendor || !VendorSlots) return;

    //Reserving array space
    TArray<UItemObject*> VendorItems;
    VendorItems.Reserve(Items.Num());

    for (const FItemData& Data : Items)
    {
        UItemObject* NewItem = NewObject<UItemObject>(this);
        NewItem->ItemData = Data;
        VendorItems.Add(NewItem);
    }
    VendorSlots->SetListItems(VendorItems);
}


void UInventory::HandleSortSelected(EInventorySorting Method)
{
    if (TB_SortingType)
    {
        TB_SortingType->SetText(FText::FromString("Sort by : " + UInventorySortingOptions::GetSortingMethodDisplayName(Method)));
    }

    if (InventorySortingOptions)
    {
        InventorySortingOptions->RemoveFromParent();
        InventorySortingOptions = nullptr;
    }

    if (InventoryComponent->GetCurrentSortingMethod() != Method)
    {
        InventoryComponent->SetSortingMethod(Method);
    }

    RefreshInventoryUI();
}

void UInventory::RefreshInventoryUI()
{
    if (!InventorySlots || !InventoryComponent) return;

    InventorySlots->ClearListItems();
    InventorySlots->SetListItems(InventoryComponent->InventoryItems);

    OnEquippedStatsUpdated(InventoryComponent->CachedEquippedStats);
}

void UInventory::RefreshEquippedUI()
{
    if (!InventoryComponent) return;

    TArray<FString> AllItemTypes = {
        "Helmet", "Upper", "Lower", "Arms",
        "Necklace", "Greatsword", "Sword", "Staff", "Bow"
    };

    for (const FString& Type : AllItemTypes)
    {
        UpdateSlotVisuals(nullptr, Type, nullptr, FLinearColor::Gray);
    }

    for (const FEquippedItemEntry& Entry : InventoryComponent->EquippedItems)
    {
        UItemObject* Item = Entry.Item;
        const FString& ItemType = Entry.ItemType;

        if (!Item) continue;

        if (TooltipWidgetClass)
        {
            UInventoryToolTip* ToolTip = CreateWidget<UInventoryToolTip>(this, TooltipWidgetClass);
            if (ToolTip)
            {
                FLinearColor RarityColor = ItemStats::GetRarityDataMap()[Item->GetRarity()].Color;

                ToolTip->Setup(Item, RarityColor, nullptr);

                UpdateSlotVisuals(Item, ItemType, ToolTip, RarityColor);
            }
        }
    }

    OnEquippedStatsUpdated(InventoryComponent->CachedEquippedStats);
}

UInventory::FSlotUIComponents UInventory::GetUIComponentsForItemType(const FString& ItemType)
{
    if (ItemType == "Helmet") return { B_Helmet1, I_Helmet, ItemFrameHelmet, TB_LevelHelmet, B_LevelHelmet };
    if (ItemType == "Arms") return { B_Arms1, I_Arms, ItemFrameArms, TB_LevelArms, B_LevelArms };
    if (ItemType == "Upper") return { B_Upper1, I_Upper, ItemFrameUpper, TB_LevelUpper, B_LevelUpper };
    if (ItemType == "Lower") return { B_Lower1, I_Lower, ItemFrameLower, TB_LevelLower, B_LevelLower };
    if (ItemType == "Necklace") return { B_Necklace1, I_Necklace, ItemFrameNecklace, TB_LevelNecklace, B_LevelNecklace };
    if (ItemType == "Greatsword"
        || ItemType == "Sword"
        || ItemType == "Bow"
        || ItemType == "Staff") return { B_Mainhand1, I_Mainhand, ItemFrameMainhand, TB_LevelMainhand, B_LevelMainhand };

    return { nullptr, nullptr, nullptr };
}

void UInventory::UpdateSlotVisuals(UItemObject* Item, const FString& ItemType, UInventoryToolTip* ToolTip, const FLinearColor& Color)
{
    auto Components = GetUIComponentsForItemType(ItemType);

    if (!Components.Border || !Components.Image || !Components.ItemFrame) return;

    const bool bHasItem = (Item != nullptr);

    const FLinearColor FrameColor = bHasItem ? Color : FLinearColor(0.2f, 0.2f, 0.2f, 1.f);
    Components.ItemFrame->SetBrushTintColor(FrameColor);
    Components.Border->SetBrushColor(FrameColor);

    if (bHasItem && Item->ItemData.Icon)
    {
        Components.Image->SetBrushFromTexture(Item->ItemData.Icon, true);

        Components.Image->SetColorAndOpacity(FLinearColor::White);
    }
    else
    {
        Components.Image->SetColorAndOpacity(FLinearColor(1.0f, 1.0f, 1.0f, 0.3f));
    }

    FVector2D ForcedSize(75.f, 75.f);
    Components.Image->SetDesiredSizeOverride(ForcedSize);

    Components.Border->SetToolTip(ToolTip);

    if (Components.LevelBorder && Components.LevelText)
    {
        if (bHasItem)
        {
            Components.LevelText->SetText(FText::AsNumber(Item->GetLevel()));
            Components.LevelBorder->SetVisibility(ESlateVisibility::SelfHitTestInvisible);

            Components.LevelBorder->SetBrushColor(FLinearColor(0.8f, 0.8f, 0.6f, 1.0f));
        }
        else
        {
            Components.LevelBorder->SetVisibility(ESlateVisibility::Collapsed);
        }
    }
}

void UInventory::SetInventoryComponent(UInventoryComponent* NewComponent)
{
    if (!NewComponent || !NewComponent->IsRegistered() || !NewComponent->GetOwner())
    {
        //Try again next frame if it's not ready
        UE_LOG(LogTemp, Warning, TEXT("[UI] InventoryComponent not ready, retrying..."));

        FTimerHandle RetryHandle;
        GetWorld()->GetTimerManager().SetTimer(RetryHandle, [this, NewComponent]() { SetInventoryComponent(NewComponent); }, 0.1f, false);

        return;
    }

    InventoryComponent = NewComponent;

    //Prevent double binding
    InventoryComponent->OnInventoryChanged.RemoveDynamic(this, &UInventory::RefreshInventoryUI);
    InventoryComponent->OnEquipmentChanged.RemoveDynamic(this, &UInventory::RefreshEquippedUI);
    InventoryComponent->OnStatsChanged.RemoveDynamic(this, &UInventory::OnEquippedStatsUpdated);
    InventoryComponent->OnForgeUpdated.RemoveDynamic(this, &UInventory::UpdateForgeUI);
    //Add the new binds
    InventoryComponent->OnInventoryChanged.AddDynamic(this, &UInventory::RefreshInventoryUI);
    InventoryComponent->OnEquipmentChanged.AddDynamic(this, &UInventory::RefreshEquippedUI);
    InventoryComponent->OnStatsChanged.AddDynamic(this, &UInventory::OnEquippedStatsUpdated);
    InventoryComponent->OnForgeUpdated.AddDynamic(this, &UInventory::UpdateForgeUI);

    UE_LOG(LogTemp, Log, TEXT("[UI] InventoryComponent successfully bound to delegates."));
    
    APlayerController* PC = GetOwningPlayer();
    PC = GetOwningPlayer();
    if (PC)
    {
        APawn* Pawn = PC->GetPawn();
        OwningCharacter = Cast<ARelicRunnersCharacter>(Pawn);
    }

    InventoryComponent->SetSortingMethod(EInventorySorting::SortByLevel);
    OnEquippedStatsUpdated(InventoryComponent->GetCachedEquippedStats());
}

void UInventory::SetPreviewActorImage(UTextureRenderTarget2D* RenderTarget)
{
    UE_LOG(LogTemp, Warning, TEXT("[UI] Trying to set preview image with RenderTarget: %s"), *GetNameSafe(RenderTarget));

    if (!RenderTarget || !RenderTarget->GetResource())
    {
        UE_LOG(LogTemp, Warning, TEXT("[UI] RenderTarget resource not ready yet."));
        return;
    }

    //Create the dynamic material
    if (!PreviewActorMaterial)
    {
        UMaterialInterface* BaseMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/ThirdPerson/Blueprints/PlayerPreview/M_PlayerPreview.M_PlayerPreview"));
        if (BaseMaterial)
        {
            PreviewActorMaterial = UMaterialInstanceDynamic::Create(BaseMaterial, this);
            I_PlayerPreview->SetBrushFromMaterial(PreviewActorMaterial);
        }
    }

    if (PreviewActorMaterial)
    {
        PreviewActorMaterial->SetTextureParameterValue(FName("PreviewTexture"), RenderTarget);
        I_PlayerPreview->SetBrushFromMaterial(PreviewActorMaterial);
    }

    UE_LOG(LogTemp, Warning, TEXT("[UI] Set preview image with RenderTarget: %s"), *GetNameSafe(RenderTarget));
}

void UInventory::EquipItem(UItemObject* Item)
{
    PlayUISound(ArmorEquipSound);

    if (Item->IsNewItem) Item->IsNewItem = false;

    if (OwningCharacter->RelicPlayerState)
    {
        OwningCharacter->RelicPlayerState->Server_EquipItemByID(Item->GetUniqueID());
    }

    UpdateForgeUI();
}

void UInventory::UnequipItem(UItemObject* Item)
{
    PlayUISound(ArmorUnequipSound);

    if (OwningCharacter && Item)
    {
        OwningCharacter->RelicPlayerState->Server_UnequipItemByID(Item->GetUniqueID());
    }
}

void UInventory::DropItem(UItemObject* Item)
{
    if (!Item) return;

    if (InventoryComponent->GetEquippedItemReference(Item->ItemData.ItemType) == Item)
    {
        UnequipItem(Item);
    }
    InventorySlots->RemoveItem(Item);

    OwningCharacter->Server_SpawnDroppedItemWorld(Item->ItemData);
}

void UInventory::BuyItem(UItemObject* Item)
{
    if (!Item) return;

    PlayUISound(BuySound);

    if (ARelicRunnersPlayerState* PS = GetOwningPlayerState<ARelicRunnersPlayerState>())
    {
        AVendor* Shop = Cast<AVendor>(CurrentVendor);
        PS->Server_BuyItem(Item->ItemData, Shop);
    }
}

void UInventory::SellItem(UItemObject* Item)
{
    if (!Item) return;

    PlayUISound(SellSound);

    if (ARelicRunnersPlayerState* PS = GetOwningPlayerState<ARelicRunnersPlayerState>())
    {
        AVendor* Shop = Cast<AVendor>(CurrentVendor);
        PS->Server_SellItem(Item->ItemData, Shop);
    }
}

void UInventory::RemoveForgeItem(UItemObject* ItemToRemove)
{
    if (!ItemToRemove || !OwningCharacter) return;

    InventoryComponent->Server_RemoveForgeItem(ItemToRemove->GetUniqueID());
}

void UInventory::NativeTick(const FGeometry& MyGeometry, float DeltaTime)
{
    Super::NativeTick(MyGeometry, DeltaTime);

    if (!CurrentVendor) return;

    float VendorMaxDistance = 500.f;
    APawn* OwningPawn = GetOwningPlayerPawn();
    
    if (OwningPawn)
    {
        float Dist = FVector::Dist(OwningPawn->GetActorLocation(), CurrentVendor->GetActorLocation());

        if (Dist > VendorMaxDistance)
        {
            ToggleShopUI(false);
            ToggleForgeUI(false);
            UInventoryItemOptions::CloseAnyOpenPopup();
            UInventorySortingOptions::CloseAnyOpenPopup();
        }
    }
    else
    {
        ToggleShopUI(false);
        ToggleForgeUI(false);
    }
}

FSlotResult UInventory::GetSlotUnderCursor(FVector2D CursorPos)
{
    TMap<UBorder*, FString> EquippedBorders;
    EquippedBorders.Add(B_Helmet1, "Helmet");
    EquippedBorders.Add(B_Upper1, "Upper");
    EquippedBorders.Add(B_Lower1, "Lower");
    EquippedBorders.Add(B_Arms1, "Arms");
    EquippedBorders.Add(B_Necklace1, "Necklace");

    for (auto& Elem : EquippedBorders)
    {
        if (Elem.Key && Elem.Key->GetCachedGeometry().IsUnderLocation(CursorPos))
        {
            UItemObject* FoundItem = InventoryComponent->GetEquippedItemByType(Elem.Value);
            return { FoundItem, Elem.Value, Elem.Key };
        }
    }

    if (B_ForgeItem1 && B_ForgeItem1->GetCachedGeometry().IsUnderLocation(CursorPos))
    {
        return { InventoryComponent->ForgeItemLeft, TEXT("Forge0"), B_ForgeItem1 };
    }
    if (B_ForgeItem2 && B_ForgeItem2->GetCachedGeometry().IsUnderLocation(CursorPos))
    {
        return { InventoryComponent->ForgeItemRight, TEXT("Forge1"), B_ForgeItem2 };
    }
    if (B_ForgeItemResult && B_ForgeItemResult->GetCachedGeometry().IsUnderLocation(CursorPos))
    {
        return { InventoryComponent->ForgeItemResult, TEXT("Forge2"), B_ForgeItemResult };
    }

    return FSlotResult();
}

FReply UInventory::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    if (InMouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton))
    {
        FVector2D CursorPos = InMouseEvent.GetScreenSpacePosition();
        auto SlotInfo = GetSlotUnderCursor(CursorPos);

        if (SlotInfo.Item)
        {
            UE_LOG(LogTemp, Warning, TEXT("Inventory: Mouse Down on %s"), *SlotInfo.SlotTag);

            bIsPressed = true;
            bIsDragging = false;
            PressedTime = GetWorld()->GetRealTimeSeconds();
            PendingDragItem = SlotInfo.Item;
            PendingSourceTag = SlotInfo.SlotTag;

            FEventReply Reply = UWidgetBlueprintLibrary::DetectDragIfPressed(InMouseEvent, this, EKeys::LeftMouseButton);
            return Reply.NativeReply;
        }
        else
        {
            UE_LOG(LogTemp, Log, TEXT("Inventory: Mouse Down but no item found at cursor."));
        }
    }
    return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

void UInventory::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation)
{
    if (!PendingDragItem) return;

    bIsDragging = true;
    UInventoryDragOperation* DragOp = NewObject<UInventoryDragOperation>();
    DragOp->Payload = PendingDragItem;
    DragOp->Pivot = EDragPivot::CenterCenter;

    if (PendingSourceTag.StartsWith("Forge"))
        DragOp->FromWhere = UInventoryDragOperation::Locations::Forge;
    else
        DragOp->FromWhere = UInventoryDragOperation::Locations::Equipped;

    if (DragVisualClass)
    {
        UInventoryDragVisual* VisualWidget = CreateWidget<UInventoryDragVisual>(this, DragVisualClass);
        if (VisualWidget)
        {
            VisualWidget->OnSetupVisual(PendingDragItem);
            VisualWidget->ForceLayoutPrepass();
            DragOp->DefaultDragVisual = VisualWidget;
        }
    }

    OutOperation = DragOp;
}

FReply UInventory::NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    if (!bIsPressed) return Super::NativeOnMouseButtonUp(InGeometry, InMouseEvent);

    bIsPressed = false;
    float HeldDuration = GetWorld()->GetRealTimeSeconds() - PressedTime;

    if (!bIsDragging && HeldDuration < 0.25f && PendingDragItem)
    {
        if (PendingSourceTag == "Helmet")   HelmetSlotClicked();
        else if (PendingSourceTag == "Upper")    UpperSlotClicked();
        else if (PendingSourceTag == "Lower")    LowerSlotClicked();
        else if (PendingSourceTag == "Arms")     ArmsSlotClicked();
        else if (PendingSourceTag == "Necklace") NecklaceSlotClicked();

        else if (PendingSourceTag.StartsWith("Forge"))
        {
            FString IndexStr = PendingSourceTag.RightChop(5);
            int ForgeIndex = FCString::Atoi(*IndexStr);

            UItemObject* TargetItem = nullptr;
            UBorder* TargetBorder = nullptr;

            if (ForgeIndex == 0) { TargetItem = InventoryComponent->ForgeItemLeft; TargetBorder = B_ForgeItem1; }
            else if (ForgeIndex == 1) { TargetItem = InventoryComponent->ForgeItemRight; TargetBorder = B_ForgeItem2; }
            else if (ForgeIndex == 2) { TargetItem = InventoryComponent->ForgeItemResult; TargetBorder = B_ForgeItemResult; }

            if (TargetItem && TargetBorder)
            {
                FItemOptionsUIConfig Config = (ForgeIndex == 2) ? FItemOptionsUIConfig::ForgeResultItem() : FItemOptionsUIConfig::ForgeItem();
                CreateItemOptionsPopup(TargetItem, Config, TargetBorder->GetCachedGeometry());
            }
        }

        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Inventory: Clicked slot with untagged source: %s"), *PendingSourceTag);
        }
    }

    bIsDragging = false;
    PendingDragItem = nullptr;
    return FReply::Handled();
}

FReply UInventory::NativeOnMouseWheel(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    UInventoryItemOptions::CloseAnyOpenPopup();
    UInventorySortingOptions::CloseAnyOpenPopup();
    return FReply::Unhandled(); 
}

FReply UInventory::NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    Super::NativeOnMouseMove(InGeometry, InMouseEvent);

    if (!IsVisible()) return FReply::Handled();

    FVector2D CursorPos = InMouseEvent.GetScreenSpacePosition();

    TArray<UBorder*> Borders = {
        B_Health,
        B_Defence,
        B_Strength,
        B_Agility,
        B_Intelligence,
        B_Luck,           
        B_Mana,           
        B_PlayerLevel,    
        B_Purity,
        B_MetaLevel,
        B_Ability1,
        B_Ability2,
        B_Ability3,
        B_Ability4,
        B_AbilityUpgrade,
        B_HealthPotion
    };

    UBorder* HoveredBorder = nullptr;
    for (UBorder* Border : Borders)
    {
        if (!Border) continue;

        FGeometry BorderGeom = Border->GetCachedGeometry();
        if (BorderGeom.IsUnderLocation(CursorPos))
        {
            HoveredBorder = Border;
            break;
        }
    }

    if (HoveredBorder != CurrentHoveredBorder)
    {

        if (StandardTooltip && CurrentHoveredBorder)
        {
            StandardTooltip->RemoveFromParent();
            StandardTooltip = nullptr;
        }

        CurrentHoveredBorder = HoveredBorder;
    }

    if (CurrentHoveredBorder)
    {
        ShowTooltipForBorder(CurrentHoveredBorder, CursorPos, InGeometry);
    }

    if(CurrentHoveredBorder) return FReply::Handled();
    else return FReply::Unhandled();
}

void UInventory::ShowTooltipForBorder(UBorder* HoveredBorder, const FVector2D& CursorPos, const FGeometry& ParentGeometry)
{
    if (!StandardTooltipClass) return;

    if (!HoveredBorder) return;

    if (!StandardTooltipClass || !ClassDataAsset || !HoveredBorder) return;

    APlayerController* PC = GetOwningPlayer();
    if (!PC) return;
    ARelicRunnersPlayerController* RPC = Cast<ARelicRunnersPlayerController>(PC);
    if (!RPC) return;
    ARelicRunnersPlayerState* RPS = Cast<ARelicRunnersPlayerState>(RPC->PlayerState);
    if (!RPS) return;

    FClassData* Info = nullptr;
    if (RPS->SelectedClass != "None") Info = &ClassDataAsset->Classes[RPS->SelectedClass];

    if (!StandardTooltip)
    {
        StandardTooltip = CreateWidget<UStandardTooltip>(PC, StandardTooltipClass);
        StandardTooltip->AddToViewport();
    }
    FEquippedStatsSummary Stats;
    if (InventoryComponent) Stats = InventoryComponent->GetCachedEquippedStats();

    FString StatKey = "";
    int RawValue = 0;
    FString AbilityName;
    FString AbilityDesc;

    URelicRunnersGameInstance* GI = Cast<URelicRunnersGameInstance>(GetGameInstance());

    if      (HoveredBorder == B_Health)         { StatKey = "Health";       RawValue = Stats.TotalHealth; }
    else if (HoveredBorder == B_Defence)        { StatKey = "Defence";      RawValue = Stats.TotalArmor; }
    else if (HoveredBorder == B_Strength)       { StatKey = "Strength";     RawValue = Stats.TotalStrength; }
    else if (HoveredBorder == B_Agility)        { StatKey = "Agility";      RawValue = Stats.TotalDexterity; }
    else if (HoveredBorder == B_Intelligence)   { StatKey = "Intelligence"; RawValue = Stats.TotalIntelligence; }
    else if (HoveredBorder == B_Luck)           { StatKey = "Luck";         RawValue = Stats.TotalLuck; }
    else if (HoveredBorder == B_Mana)           { StatKey = "Mana";         RawValue = Stats.TotalResource; }
    else if (HoveredBorder == B_PlayerLevel)    { StatKey = "Level";        RawValue = Stats.PlayerLevel; }
    else if (HoveredBorder == B_Purity)         { StatKey = "Purity";       RawValue = Stats.TotalPotionEfficiency; }
    else if (HoveredBorder == B_MetaLevel)      { StatKey = "MetaLevel";    if(GI->SelectedSaveSlot) RawValue = GI->SelectedSaveSlot->SaveData.MetaLevel; }
    else if (HoveredBorder == B_AbilityUpgrade) { StatKey = "AbilityUpgrade"; }
    else if (HoveredBorder == B_HealthPotion) { StatKey = "HealthPotion"; }
    else if (HoveredBorder == B_Ability1) { StatKey = "Ability1"; if (Info) AbilityName = Info->Ability1; if (Info) AbilityDesc = Info->Ability1Description1; }
    else if (HoveredBorder == B_Ability2) { StatKey = "Ability2"; if (Info) AbilityName = Info->Ability2; if (Info) AbilityDesc = Info->Ability2Description1; }
    else if (HoveredBorder == B_Ability3) { StatKey = "Ability3"; if (Info) AbilityName = Info->Ability3; if (Info) AbilityDesc = Info->Ability3Description1; }
    else if (HoveredBorder == B_Ability4) { StatKey = "Ability4"; if (Info) AbilityName = Info->Ability4; if (Info) AbilityDesc = Info->Ability4Description1; }

    FVector2D Offset(-5.0f, 5.0f);
    FVector2D Anchor(1.f, 0.f);
    FTooltipData TD;
    if (HoveredBorder == B_Mana)
    {
        TD = ItemStats::GetStatTooltipData(StatKey, RawValue, Stats.TotalResourceRegen, Stats.TotalResourceTime, "", "");
    }
    else if (HoveredBorder == B_PlayerLevel)
    {
        if (GI->SelectedSaveSlot)
        {
            TD = ItemStats::GetStatTooltipData(StatKey, RawValue, GI->SelectedSaveSlot->SaveData.MetaStartingLevel, 0, "", "");
        }
        else
        {
            TD = ItemStats::GetStatTooltipData(StatKey, RawValue, 0, 0, "", "");
        }
    }
    else if (HoveredBorder == B_MetaLevel)
    {
        if (GI->SelectedSaveSlot)
        {
            TD = ItemStats::GetStatTooltipData(StatKey, RawValue, 0, 0, "", "");
        }
        else
        {
            TD = ItemStats::GetStatTooltipData(StatKey, 0, 0, 0, "", "");
        }
    }
    else if (HoveredBorder == B_Ability1 || HoveredBorder == B_Ability2 || HoveredBorder == B_Ability3 || HoveredBorder == B_Ability4 || HoveredBorder == B_AbilityUpgrade || HoveredBorder == B_HealthPotion)
    {
        Offset = FVector2D(0, -50.0f);
        Anchor = FVector2D(0.5f, 1);
        TD = ItemStats::GetStatTooltipData(StatKey, RawValue, 0, 0, AbilityName, AbilityDesc);
    }
    else
    {
        TD = ItemStats::GetStatTooltipData(StatKey, RawValue, 0, 0, "", "");
    }

    FVector2D ViewportPosition = ParentGeometry.AbsoluteToLocal(CursorPos);

    StandardTooltip->SetAnchor(Anchor);
    StandardTooltip->SetIcon(TD.Icon, TD.IconColor, 0);
    StandardTooltip->Setup(TD.Name, TD.Value, TD.Descriptions);
    StandardTooltip->SetPositionInViewport(ViewportPosition + Offset, false);
}

void UInventory::PlayUISound(USoundBase* Sound)
{
    if (!Sound) return;

    if (APlayerController* PC = GetOwningPlayer())
    {
        const float RandomPitch = FMath::FRandRange(0.80f, 1.80f);
        
        if (UAudioComponent* AC = UGameplayStatics::SpawnSound2D(PC, Sound))
        {
            AC->SetPitchMultiplier(RandomPitch);
            return;
        }

        UGameplayStatics::PlaySound2D(PC, Sound);
    }
}

void UInventory::HelmetSlotClicked()
{
    if (B_Helmet1)
    {
        OnEquippedSlotClick(InventoryComponent->GetEquippedItemByType("Helmet"), B_Helmet1->GetCachedGeometry());
    }
}

void UInventory::ArmsSlotClicked()
{
    if (B_Arms1)
    {
        OnEquippedSlotClick(InventoryComponent->GetEquippedItemByType("Arms"), B_Arms1->GetCachedGeometry());
    }
}

void UInventory::UpperSlotClicked()
{
    if (B_Upper1)
    {
        OnEquippedSlotClick(InventoryComponent->GetEquippedItemByType("Upper"), B_Upper1->GetCachedGeometry());
    }
}

void UInventory::LowerSlotClicked()
{
    if (B_Lower1)
    {
        OnEquippedSlotClick(InventoryComponent->GetEquippedItemByType("Lower"), B_Lower1->GetCachedGeometry());
    }
}

void UInventory::NecklaceSlotClicked()
{
    if (B_Necklace1)
    {
        OnEquippedSlotClick(InventoryComponent->GetEquippedItemByType("Necklace"), B_Necklace1->GetCachedGeometry());
    }
}

void UInventory::MainhandSlotClicked()
{
    //OnEquippedSlotClick(InventoryComponent->GetEquippedItemByType("Mainhand"));
}

void UInventory::SortingTypeClicked()
{
    if (InventorySortingOptions && InventorySortingOptions->IsInViewport())
    {
        InventorySortingOptions->RemoveFromParent();
        InventorySortingOptions = nullptr;
        return;
    }

    if (InventorySortingOptionsClass)
    {
        InventorySortingOptions = CreateWidget<UInventorySortingOptions>(GetWorld(), InventorySortingOptionsClass);
        if (InventorySortingOptions)
        {
            InventorySortingOptions->OnSortingSelected.AddDynamic(this, &UInventory::HandleSortSelected);

            InventorySortingOptions->Setup();
            const FGeometry& ButtonGeometry = B_SortingType->GetCachedGeometry();
            FVector2D PixelPos;
            FVector2D ViewportPos;
            FVector2D Offset = FVector2D(0,60);
            InventorySortingOptions->AddToViewport();
            USlateBlueprintLibrary::LocalToViewport(GetWorld(), ButtonGeometry, FVector2D(0.f, 0.f), PixelPos, ViewportPos);
            InventorySortingOptions->SetPositionInViewport(PixelPos + Offset, true);
            InventorySortingOptions->SetAlignmentInViewport(FVector2D(0.0f, 0.f));
        }
    }
}

void UInventory::OnEquippedSlotClick(UItemObject* EquippedItem, const FGeometry& SlotGeometry)
{
    UInventorySortingOptions::CloseAnyOpenPopup();
    UInventoryItemOptions::CloseAnyOpenPopup();

    if (EquippedItem)
    {
        CreateItemOptionsPopup(EquippedItem, FItemOptionsUIConfig::EquippedItem(), SlotGeometry);
    }
}

void UInventory::CreateItemOptionsPopup(UItemObject* Item, const FItemOptionsUIConfig& Config, const FGeometry& InGeometry)
{
    UInventoryItemOptions* Popup = CreateWidget<UInventoryItemOptions>(GetWorld(), InventoryItemOptionsClass);

    if (Popup)
    {
        Popup->Setup(Item);
        Popup->ApplyConfig(Config);
        Popup->AddToViewport();

        FVector2D PixelPos;
        FVector2D ViewportPos;

        USlateBlueprintLibrary::LocalToViewport(GetWorld(), InGeometry, FVector2D(0.f, 0.f), PixelPos, ViewportPos);
        Popup->SetPositionInViewport(PixelPos, true);
        Popup->SetAlignmentInViewport(FVector2D(0.f, 0.f));
    }
}

bool UInventory::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
    Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);

    if (!InOperation) return false;

    UInventoryDragOperation* DragOp = Cast<UInventoryDragOperation>(InOperation);
    if (!DragOp) return false;

    UItemObject* DraggedItem = Cast<UItemObject>(DragOp->Payload);
    if (!DraggedItem) return false; 

    FVector2D DropPosition = InGeometry.AbsoluteToLocal(InDragDropEvent.GetScreenSpacePosition());

    //Equipped Items Location
    FGeometry EquippedGeometry = EquippedCanvas->GetCachedGeometry();
    const FVector2D EquippedLocal = EquippedGeometry.AbsoluteToLocal(InDragDropEvent.GetScreenSpacePosition());
    const FVector2D EquippedSize = EquippedGeometry.GetLocalSize();
    const bool InsideEquipped =
        EquippedLocal.X >= 0.f &&
        EquippedLocal.Y >= 0.f &&
        EquippedLocal.X <= EquippedSize.X &&
        EquippedLocal.Y <= EquippedSize.Y;

    //Inventory Items Location
    FGeometry InventoryGeometry = InventoryBox->GetCachedGeometry();
    const FVector2D InventoryLocal = InventoryGeometry.AbsoluteToLocal(InDragDropEvent.GetScreenSpacePosition());
    const FVector2D InventorySize = InventoryGeometry.GetLocalSize();
    const bool InsideInventory =
        InventoryLocal.X >= 0.f &&
        InventoryLocal.Y >= 0.f &&
        InventoryLocal.X <= InventorySize.X &&
        InventoryLocal.Y <= InventorySize.Y;

    //Vendor Forge Location
    FGeometry ForgeGeometry = ForgeBox->GetCachedGeometry();
    const FVector2D ForgeLocal = ForgeGeometry.AbsoluteToLocal(InDragDropEvent.GetScreenSpacePosition());
    const FVector2D ForgeSize = ForgeGeometry.GetLocalSize();
    const bool InsideForge =
        ForgeLocal.X >= 0.f &&
        ForgeLocal.Y >= 0.f &&
        ForgeLocal.X <= ForgeSize.X &&
        ForgeLocal.Y <= ForgeSize.Y;

    //Vendor Shop Location
    FGeometry ShopGeometry = ShopBox->GetCachedGeometry();
    const FVector2D ShopLocal = ShopGeometry.AbsoluteToLocal(InDragDropEvent.GetScreenSpacePosition());
    const FVector2D ShopSize = ShopGeometry.GetLocalSize();
    const bool InsideShop =
        ShopLocal.X >= 0.f &&
        ShopLocal.Y >= 0.f &&
        ShopLocal.X <= ShopSize.X &&
        ShopLocal.Y <= ShopSize.Y;

    if (InsideEquipped)
    {
        EquipItem(DraggedItem);
    }
    else if (InsideForge && ForgeCanvas->IsVisible() && DragOp->FromWhere == UInventoryDragOperation::Locations::Inventory || 
            InsideForge && ForgeCanvas->IsVisible() && DragOp->FromWhere == UInventoryDragOperation::Locations::Equipped)
    {
        ForgeItem(DraggedItem);
    }
    else if (InsideShop && ShopCanvas->IsVisible() && DragOp->FromWhere == UInventoryDragOperation::Locations::Inventory ||
            InsideShop && ShopCanvas->IsVisible() && DragOp->FromWhere == UInventoryDragOperation::Locations::Equipped)
    {
        SellItem(DraggedItem);
    }
    else if (InsideInventory && DragOp->FromWhere == UInventoryDragOperation::Locations::Shop)
    {
        BuyItem(DraggedItem);
    }
    else if (InsideInventory && DragOp->FromWhere == UInventoryDragOperation::Locations::Equipped)
    {
        UnequipItem(DraggedItem);
    }
    else if (InsideInventory && DragOp->FromWhere == UInventoryDragOperation::Locations::Inventory)
    {
        return true;
    }
    else if (InsideInventory && DragOp->FromWhere == UInventoryDragOperation::Locations::Forge)
    {
        RemoveForgeItem(DraggedItem);
        InventoryComponent->AddItem(DraggedItem);
    }
    else
    {
        DropItem(DraggedItem);
    }

    InventorySlots->RequestRefresh();

    return true;
}

void UInventory::ForgeItem(UItemObject* Item)
{
    if (!ForgeCanvas->IsVisible() || !Item) return;
    if (Item->GetRarity() == "Relic") return;

    PlayUISound(ForgePlaceSound);
    
    if (InventoryComponent->ForgeItemResult)
    {
        CancelForgingAnimation();
    }
    
    UnequipItem(Item);
    InventoryComponent->Server_AddToForge(Item->GetUniqueID());

    UpdateForgeUI();
}

void UInventory::SetEquippedTooltip(UBorder* BorderWidget, FString ItemType)
{
    if (!BorderWidget || !InventoryComponent || !TooltipWidgetClass) return;

    UItemObject* Item = InventoryComponent->GetEquippedItemByType(ItemType);

    if (!Item)
    {
        BorderWidget->SetToolTip(nullptr);
        return;
    }

    UInventoryToolTip* ToolTip = CreateWidget<UInventoryToolTip>(this, TooltipWidgetClass);
    if (ToolTip)
    {
        const FLinearColor RarityColor = ItemStats::GetRarityDataMap()[Item->GetRarity()].Color;

        ToolTip->Setup(Item, RarityColor, nullptr);

        BorderWidget->SetToolTip(ToolTip);
    }
}

void UInventory::SetForgeTooltip(UImage* IconWidget, UBorder* BorderWidget, UItemObject* Item)
{
    if (!Item)
    {
        if (BorderWidget) BorderWidget->SetToolTip(nullptr);
        return;
    }

    if (!TooltipWidgetClass || !BorderWidget) return;

    UInventoryToolTip* ToolTip = CreateWidget<UInventoryToolTip>(GetWorld(), TooltipWidgetClass);
    if (!ToolTip) return;

    const FLinearColor RarityColor = ItemStats::GetRarityDataMap()[Item->GetRarity()].Color;

    UItemObject* EquippedItem = InventoryComponent->GetEquippedItemByType(Item->GetItemType());
    
    ToolTip->Setup(Item, RarityColor, EquippedItem);

    BorderWidget->SetToolTip(ToolTip);
}

void UInventory::OnFuseButtonPressed()
{
    if (!InventoryComponent->ForgeItemLeft || !InventoryComponent->ForgeItemRight) return;

    PlayUISound(ForgePressSound);

    InventoryComponent->Server_FuseItems();

    UpdateForgeUI();

    PlayAnimation(RollRarity);

    CycleCount = 0;
    GetWorld()->GetTimerManager().SetTimer(FuseTimerHandle, this, &UInventory::CycleRarityVisuals, 0.1f, true);
}

void UInventory::CancelForgingAnimation()
{
    StopAnimation(RollRarity);
    GetWorld()->GetTimerManager().ClearTimer(FuseTimerHandle);
}

void UInventory::CycleRarityVisuals()
{
    CycleCount++;

    FString DisplayRarity;
    float Roll = FMath::FRandRange(0.f, 100.f);

    if (Roll <= ForgeChance1) DisplayRarity = ForgeRarity1;
    else if (Roll <= (ForgeChance1 + ForgeChance2)) DisplayRarity = ForgeRarity2;
    else DisplayRarity = ForgeRarity3;

    FLinearColor Color = ItemStats::GetRarityDataMap()[DisplayRarity].Color;
    if (ItemFrame_2) ItemFrame_2->SetBrushTintColor(Color);
    if (B_ForgeItemResult) B_ForgeItemResult->SetBrushColor(Color);
    if (LightBeam) LightBeam->SetBrushTintColor(Color);
    if (LightBeam_1) LightBeam_1->SetBrushTintColor(Color);
    if (LightBeam_2) LightBeam_2->SetBrushTintColor(Color);
    if (LightBeam_3) LightBeam_3->SetBrushTintColor(Color);

    if (CycleCount >= 10)
    {
        GetWorld()->GetTimerManager().ClearTimer(FuseTimerHandle);
        CompleteForgeSequence();
    }
}

void UInventory::CompleteForgeSequence()
{
    if (!InventoryComponent->ForgeItemResult) return;

    PlayUISound(ForgeSuccessSound);

    FLinearColor FinalColor = ItemStats::GetRarityDataMap()[InventoryComponent->ForgeItemResult->ItemData.Rarity].Color;

    if (B_Odds) B_Odds->SetVisibility(ESlateVisibility::Hidden);
    if (B_Odds1) B_Odds1->SetVisibility(ESlateVisibility::Hidden);
    if (B_Odds2) B_Odds2->SetVisibility(ESlateVisibility::Hidden);

    UpdateForgeUI();

    if (InventoryComponent->ForgeItemResult->ItemData.Rarity == ForgeRarity1)
    {
        FLinearColor RarityColor = ItemStats::GetRarityDataMap()[ForgeRarity1].Color;
        if (B_Odds) B_Odds->SetVisibility(ESlateVisibility::Visible);
        if (B_Odds1) B_Odds1->SetVisibility(ESlateVisibility::Visible);
        TB_Odds->SetColorAndOpacity(RarityColor);

        FSlateFontInfo FontInfo = TB_Odds->GetFont();
        FontInfo.OutlineSettings.OutlineColor = RarityColor;
        TB_Odds->SetFont(FontInfo);

        TB_Odds->SetVisibility(ESlateVisibility::Visible);
        TB_Odds->SetText(FText::FromString(TEXT("Quality Decreased")));
    }

    if (InventoryComponent->ForgeItemResult->ItemData.Rarity == ForgeRarity2)
    {
        FLinearColor RarityColor = ItemStats::GetRarityDataMap()[ForgeRarity2].Color;
        if (B_Odds) B_Odds->SetVisibility(ESlateVisibility::Visible);
        if (B_Odds1) B_Odds1->SetVisibility(ESlateVisibility::Visible);
        TB_Odds->SetColorAndOpacity(RarityColor);

        FSlateFontInfo FontInfo = TB_Odds->GetFont();
        FontInfo.OutlineSettings.OutlineColor = RarityColor;
        TB_Odds->SetFont(FontInfo);

        TB_Odds->SetVisibility(ESlateVisibility::Visible);
        TB_Odds->SetText(FText::FromString(TEXT("Quality Unaffected")));
    }

    if (InventoryComponent->ForgeItemResult->ItemData.Rarity == ForgeRarity3)
    {
        FLinearColor RarityColor = ItemStats::GetRarityDataMap()[ForgeRarity3].Color;
        if (B_Odds) B_Odds->SetVisibility(ESlateVisibility::Visible);
        if (B_Odds1) B_Odds1->SetVisibility(ESlateVisibility::Visible);
        TB_Odds->SetColorAndOpacity(RarityColor);

        FSlateFontInfo FontInfo = TB_Odds->GetFont();
        FontInfo.OutlineSettings.OutlineColor = RarityColor;
        TB_Odds->SetFont(FontInfo);

        TB_Odds->SetVisibility(ESlateVisibility::Visible);
        TB_Odds->SetText(FText::FromString(TEXT("Quality Increased")));
    }

    FinalColor = ItemStats::GetRarityDataMap()[InventoryComponent->ForgeItemResult->ItemData.Rarity].Color;

    TB_Odds1->SetVisibility(ESlateVisibility::Visible);
    TB_Odds1->SetText(FText::FromString(FString::Printf(TEXT("%s item"), *InventoryComponent->ForgeItemResult->ItemData.Rarity)));
    TB_Odds1->SetColorAndOpacity(FinalColor);

    FSlateFontInfo FontInfo1 = TB_Odds1->GetFont();
    FontInfo1.OutlineSettings.OutlineColor = FinalColor;
    TB_Odds1->SetFont(FontInfo1);

    if (ItemFrame_2) ItemFrame_2->SetBrushTintColor(FinalColor);
    if (B_ForgeItemResult) B_ForgeItemResult->SetBrushColor(FinalColor);

    SetForgeTooltip(I_ForgeItemResult, B_ForgeItemResult, InventoryComponent->ForgeItemResult);
    SetForgeTooltip(I_ForgeItem2, B_ForgeItem2, nullptr);
    SetForgeTooltip(I_ForgeItem1, B_ForgeItem1, nullptr);

    if (ForgeCompleted)
    {
        PlayAnimation(ForgeCompleted);
        PlayAnimation(ForgeShake);
    }
}

void UInventory::UpdateForgeOdds(const FString& LeftRarity, const FString& RightRarity)
{
    FForgeResultOdds Odds = ItemStats::CalculateForgeOdds(LeftRarity, RightRarity);

    ForgeRarity1 = Odds.Rarity1; ForgeChance1 = Odds.Chance1;
    ForgeRarity2 = Odds.Rarity2; ForgeChance2 = Odds.Chance2;
    ForgeRarity3 = Odds.Rarity3; ForgeChance3 = Odds.Chance3;

    if (Odds.Rarity1 == Odds.Rarity2)
    {
        if (B_Odds) B_Odds->SetVisibility(ESlateVisibility::Visible);
        if (B_Odds1) B_Odds1->SetVisibility(ESlateVisibility::Visible);
        if (B_Odds2) B_Odds2->SetVisibility(ESlateVisibility::Hidden);

        TB_Odds->SetText(FText::FromString(FString::Printf(TEXT("%s: %.1f%%"), *Odds.Rarity2, Odds.Chance1 + Odds.Chance2)));
        TB_Odds1->SetText(FText::FromString(FString::Printf(TEXT("%s: %.1f%%"), *Odds.Rarity3, Odds.Chance3)));
        TB_Odds2->SetVisibility(ESlateVisibility::Hidden);

        auto& RarityMap = ItemStats::GetRarityDataMap();

        FLinearColor Color0 = RarityMap[Odds.Rarity2].Color;
        TB_Odds->SetColorAndOpacity(Color0);
        FSlateFontInfo Font0 = TB_Odds->GetFont();
        Font0.OutlineSettings.OutlineColor = Color0;
        TB_Odds->SetFont(Font0);

        FLinearColor Color1 = RarityMap[Odds.Rarity3].Color;
        TB_Odds1->SetColorAndOpacity(Color1);
        FSlateFontInfo Font1 = TB_Odds1->GetFont();
        Font1.OutlineSettings.OutlineColor = Color1;
        TB_Odds1->SetFont(Font1);
    }
    else
    {
        if (B_Odds) B_Odds->SetVisibility(ESlateVisibility::Visible);
        if (B_Odds1) B_Odds1->SetVisibility(ESlateVisibility::Visible);
        if (B_Odds2) B_Odds2->SetVisibility(ESlateVisibility::Visible);

        TB_Odds->SetText(FText::FromString(FString::Printf(TEXT("%s: %.1f%%"), *Odds.Rarity1, Odds.Chance1)));
        TB_Odds1->SetText(FText::FromString(FString::Printf(TEXT("%s: %.1f%%"), *Odds.Rarity2, Odds.Chance2)));
        TB_Odds2->SetText(FText::FromString(FString::Printf(TEXT("%s: %.1f%%"), *Odds.Rarity3, Odds.Chance3)));
        TB_Odds2->SetVisibility(ESlateVisibility::Visible);

        auto& RarityMap = ItemStats::GetRarityDataMap();

        FLinearColor Color0 = RarityMap[Odds.Rarity1].Color;
        TB_Odds->SetColorAndOpacity(Color0);
        FSlateFontInfo Font0 = TB_Odds->GetFont();
        Font0.OutlineSettings.OutlineColor = Color0;
        TB_Odds->SetFont(Font0);

        FLinearColor Color1 = RarityMap[Odds.Rarity2].Color;
        TB_Odds1->SetColorAndOpacity(Color1);
        FSlateFontInfo Font1 = TB_Odds1->GetFont();
        Font1.OutlineSettings.OutlineColor = Color1;
        TB_Odds1->SetFont(Font1);

        FLinearColor Color2 = RarityMap[Odds.Rarity3].Color;
        TB_Odds2->SetColorAndOpacity(Color2);
        FSlateFontInfo Font2 = TB_Odds2->GetFont();
        Font2.OutlineSettings.OutlineColor = Color2;
        TB_Odds2->SetFont(Font2);
    }
}

void UInventory::UpdateForgeUI()
{
    if (!InventoryComponent) return;

    bool LeftFilled = (InventoryComponent->ForgeItemLeft != nullptr);
    bool RightFilled = (InventoryComponent->ForgeItemRight != nullptr);
    bool BothFilled = LeftFilled && RightFilled;

    Throbber1->SetRenderOpacity(LeftFilled ? 1.0f : 0.0f);
    Throbber2->SetRenderOpacity(RightFilled ? 1.0f : 0.0f);

    if (B_Odds) B_Odds->SetVisibility(ESlateVisibility::Hidden);
    if (B_Odds1) B_Odds1->SetVisibility(ESlateVisibility::Hidden);
    if (B_Odds2) B_Odds2->SetVisibility(ESlateVisibility::Hidden);

    SetForgeTooltip(I_ForgeItem1, B_ForgeItem1, nullptr);
    SetForgeTooltip(I_ForgeItem2, B_ForgeItem2, nullptr);
    SetForgeTooltip(I_ForgeItemResult, B_ForgeItemResult, nullptr);

    if (LeftFilled)
    {
        TB_ForgeItemLevel1->SetText(FText::AsNumber(InventoryComponent->ForgeItemLeft->ItemData.Level));

        FSlateBrush Brush;
        Brush.SetResourceObject(InventoryComponent->ForgeItemLeft->ItemData.Icon);
        Brush.ImageSize = FVector2D(75.f, 75.f);
        Brush.TintColor = FSlateColor(FLinearColor::White);
        I_ForgeItem1->SetBrush(Brush);
        FLinearColor color = ItemStats::GetRarityDataMap()[InventoryComponent->ForgeItemLeft->GetRarity()].Color;
        ItemFrame->SetBrushTintColor(color);
        B_ForgeItem1->SetBrushColor(color);
        B_Level->SetBrushColor(FLinearColor(0.8, 0.8, 0.6, 1));
        B_Level->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
        SetForgeTooltip(I_ForgeItem1, B_ForgeItem1, InventoryComponent->ForgeItemLeft);
    }
    else
    {
        TB_ForgeItemLevel1->SetText(FText::GetEmpty());

        FSlateBrush Brush;
        Brush.SetResourceObject(nullptr);
        Brush.ImageSize = FVector2D(75.f, 75.f);
        Brush.TintColor = FSlateColor(FLinearColor(1, 1, 1, 0)); 
        I_ForgeItem1->SetBrush(Brush);
        FLinearColor color = FLinearColor::Transparent;
        ItemFrame->SetBrushTintColor(FLinearColor(0.2, 0.2, 0.2, 1));
        B_ForgeItem1->SetBrushColor(color);
        B_Level->SetVisibility(ESlateVisibility::Hidden);
    }

    if (RightFilled)
    {
        TB_ForgeItemLevel2->SetText(FText::AsNumber(InventoryComponent->ForgeItemRight->ItemData.Level));

        FSlateBrush Brush;
        Brush.SetResourceObject(InventoryComponent->ForgeItemRight->ItemData.Icon);
        Brush.ImageSize = FVector2D(75.f, 75.f);
        Brush.TintColor = FSlateColor(FLinearColor::White);
        I_ForgeItem2->SetBrush(Brush);
        FLinearColor color = ItemStats::GetRarityDataMap()[InventoryComponent->ForgeItemRight->GetRarity()].Color;
        ItemFrame_1->SetBrushTintColor(color);
        B_ForgeItem2->SetBrushColor(color);
        B_Level_1->SetBrushColor(FLinearColor(0.8, 0.8, 0.6, 1));
        B_Level_1->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
        SetForgeTooltip(I_ForgeItem2, B_ForgeItem2, InventoryComponent->ForgeItemRight);
    }
    else
    {
        TB_ForgeItemLevel2->SetText(FText::GetEmpty());

        FSlateBrush Brush;
        Brush.SetResourceObject(nullptr);
        Brush.ImageSize = FVector2D(75.f, 75.f);
        Brush.TintColor = FSlateColor(FLinearColor(1, 1, 1, 0));
        I_ForgeItem2->SetBrush(Brush);
        FLinearColor color = FLinearColor::Transparent;
        ItemFrame_1->SetBrushTintColor(FLinearColor(0.2, 0.2, 0.2, 1));
        B_ForgeItem2->SetBrushColor(color);
        B_Level_1->SetVisibility(ESlateVisibility::Hidden);
    }

    FuseBorder->SetVisibility(BothFilled ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
    TB_Odds->SetVisibility(BothFilled ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
    TB_Odds1->SetVisibility(BothFilled ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
    TB_Odds2->SetVisibility(BothFilled ? ESlateVisibility::Visible : ESlateVisibility::Hidden);

    if (BothFilled)
    {
        UpdateForgeOdds(InventoryComponent->ForgeItemLeft->ItemData.Rarity, InventoryComponent->ForgeItemRight->ItemData.Rarity);

        if (B_Odds) B_Odds->SetVisibility(ESlateVisibility::Visible);
        if (B_Odds1) B_Odds1->SetVisibility(ESlateVisibility::Visible);
    }

    if (!LeftFilled || !RightFilled)
    {
        B_ForgeItemResult->SetBrushColor(FLinearColor::Transparent);
        TB_ForgeItemResultLevel->SetText(FText::GetEmpty());

        //Clear icon
        FSlateBrush ClearBrush;
        ClearBrush.SetResourceObject(nullptr);
        ClearBrush.ImageSize = FVector2D(100.f, 100.f);
        ClearBrush.TintColor = FSlateColor(FLinearColor(1, 1, 1, 0));
        I_ForgeItemResult->SetBrush(ClearBrush);
        B_Level_2->SetVisibility(ESlateVisibility::Hidden);
        ItemFrame_2->SetBrushTintColor(FLinearColor(0.2, 0.2, 0.2, 1));
    }
    if (InventoryComponent->ForgeItemResult)
    {
        FSlateBrush ResultBrush;
        ResultBrush.SetResourceObject(InventoryComponent->ForgeItemResult->ItemData.Icon);
        ResultBrush.ImageSize = FVector2D(100.f, 100.f);
        ResultBrush.TintColor = FSlateColor(FLinearColor::White);
        I_ForgeItemResult->SetBrush(ResultBrush);
        TB_ForgeItemResultLevel->SetText(FText::AsNumber(InventoryComponent->ForgeItemResult->ItemData.Level));
        FLinearColor RarityColor = ItemStats::GetRarityDataMap()[InventoryComponent->ForgeItemResult->GetRarity()].Color;
        B_ForgeItemResult->SetBrushColor(RarityColor);
        B_Level_2->SetBrushColor(FLinearColor(0.8, 0.8, 0.6, 1));
        B_Level_2->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
        ItemFrame_2->SetBrushTintColor(RarityColor);
        SetForgeTooltip(I_ForgeItemResult, B_ForgeItemResult, InventoryComponent->ForgeItemResult);
    }
}

void UInventory::OnEquippedStatsUpdated(const FEquippedStatsSummary& Stats)
{
    if (TB_Level)           TB_Level->SetText(FText::AsNumber(Stats.PlayerLevel));
    if (TB_Health)          TB_Health->SetText(FText::AsNumber(Stats.TotalHealth));
    if (TB_Armor)           TB_Armor->SetText(FText::AsNumber(Stats.TotalArmor));
    if (TB_Dexterity)       TB_Dexterity->SetText(FText::AsNumber(Stats.TotalDexterity));
    if (TB_Strength)        TB_Strength->SetText(FText::AsNumber(Stats.TotalStrength));
    if (TB_Intelligence)    TB_Intelligence->SetText(FText::AsNumber(Stats.TotalIntelligence));
    if (TB_Luck)            TB_Luck->SetText(FText::AsNumber(Stats.TotalLuck));
    if (TB_Purity)          TB_Purity->SetText(FText::AsNumber(Stats.TotalPotionEfficiency));
    if (TB_Mana)            TB_Mana->SetText(FText::AsNumber(Stats.TotalResource));

    URelicRunnersGameInstance* GI = Cast<URelicRunnersGameInstance>(GetGameInstance());
    if (TB_MetaLevel && GI->SelectedSaveSlot)       TB_MetaLevel->SetText(FText::AsNumber(GI->SelectedSaveSlot->GetMetaLevel()));

    if (ARelicRunnersPlayerState* PS = GetOwningPlayerState<ARelicRunnersPlayerState>())
    {
        if (TB_Gold)
        {
            TB_Gold->SetText(FText::AsNumber(PS->GetPlayerGold()));
        }
        if (TB_Gold_Armory)
        {
            TB_Gold_Armory->SetText(FText::AsNumber(PS->GetPlayerGold()));
        }
        if (TB_Username)
        {
            TB_Username->SetText(FText::FromString(PS->GetPlayerName()));
        }
        if (InventoryComponent && TB_InventorySlots)
        {
            int NumItems = InventoryComponent->InventoryItems.Num();
            int MaxSlots = PS->GetPlayerNumInventorySlots();
            TB_InventorySlots->SetText(FText::FromString(FString::Printf(TEXT("%d / %d"), NumItems, MaxSlots)));
        }
    }
}

UInventory* UInventory::Get(UObject* ContextObject)
{
    if (!ContextObject) return nullptr;

    if (APlayerController* PC = ContextObject->GetWorld()->GetFirstPlayerController())
    {
        if (ARelicRunnersPlayerController* RRPC = Cast<ARelicRunnersPlayerController>(PC))
        {
            return RRPC->GetInventory();
        }
    }
    return nullptr;
}