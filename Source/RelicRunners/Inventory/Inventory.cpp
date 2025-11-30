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
#include <RelicRunners/Game/RelicRunnersGameInstance.h>
#include "InventoryDragOperation.h"

void UInventory::NativeConstruct()
{
    Super::NativeConstruct();

    TB_InventorySlots->SetText(FText::FromString(FString::Printf(TEXT("%d / %d"), 0, 0)));

    InventoryItemOptionsClass = LoadClass<UInventoryItemOptions>(nullptr, TEXT("/Game/Inventory/BP_InventoryItemOptions.BP_InventoryItemOptions_C"));
    InventorySortingOptionsClass = LoadClass<UInventorySortingOptions>(nullptr, TEXT("/Game/Inventory/BP_InventorySortingOptions.BP_InventorySortingOptions_C"));

    if (B_Helmet) B_Helmet->OnClicked.AddDynamic(this, &UInventory::HelmetSlotClicked);
    if (B_Arms) B_Arms->OnClicked.AddDynamic(this, &UInventory::ArmsSlotClicked);
    if (B_Upper) B_Upper->OnClicked.AddDynamic(this, &UInventory::UpperSlotClicked);
    if (B_Lower) B_Lower->OnClicked.AddDynamic(this, &UInventory::LowerSlotClicked);
    if (B_Mainhand) B_Mainhand->OnClicked.AddDynamic(this, &UInventory::MainhandSlotClicked);
    if (B_Necklace) B_Necklace->OnClicked.AddDynamic(this, &UInventory::NecklaceSlotClicked);
    if (B_SortingType) B_SortingType->OnClicked.AddDynamic(this, &UInventory::SortingTypeClicked);
    
    //Vendor
    if (VendorCanvas) VendorCanvas->SetVisibility(ESlateVisibility::Collapsed);
    if (VendorSlots) VendorSlots->ClearListItems();
    if (FuseButton) FuseButton->OnClicked.AddDynamic(this, &UInventory::OnFuseButtonPressed);
}

void UInventory::ToggleVendorUI(bool value)
{
    if (!VendorCanvas) return;

    if (!value)
    {
        VendorCanvas->SetVisibility(ESlateVisibility::Collapsed);
        VendorSlots->ClearListItems();
        CurrentVendor = nullptr;
        if (ForgeItemLeft)
        {
            InventoryComponent->AddItem(ForgeItemLeft);
            ForgeItemLeft = nullptr;
        }

        if (ForgeItemRight)
        {
            InventoryComponent->AddItem(ForgeItemRight);
            ForgeItemRight = nullptr;
        }
        ForgeItemResult = nullptr;
        SetForgeTooltip(I_ForgeItemResult, B_ForgeItemResult, nullptr);
        SetForgeTooltip(I_ForgeItem2, B_ForgeItem2, nullptr);
        SetForgeTooltip(I_ForgeItem1, B_ForgeItem1, nullptr);
        if (ForgeCompleted) StopAnimation(ForgeCompleted);
        UpdateForgeUI();
    }
    else
    {
        VendorCanvas->SetVisibility(ESlateVisibility::Visible);
        UpdateForgeUI();
    }
}

void UInventory::DisplaySelectedVendorItems(const TArray<FItemData>& Items, AVendor* Vendor)
{
    if (!Vendor || !VendorSlots) return;

    CurrentVendor = Vendor;

    //Reserving array space
    TArray<UItemObject*> VendorItems;
    VendorItems.Reserve(Items.Num());

    for (const FItemData& Data : Items)
    {
        UItemObject* NewItem = NewObject<UItemObject>(this);
        NewItem->ItemData = Data;

        VendorItems.Add(NewItem);
    }
    //Set all items at once
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
    TArray<FString> AllItemTypes = {
        "Helmet", "Upper", "Lower", "Arms",
        "Necklace", "Greatsword", "Sword", "Staff", "Bow"
    };

    for (const FString& Type : AllItemTypes)
    {
        UpdateButtonWithTooltip(nullptr, Type, nullptr, FLinearColor::Gray);
    }

    for (const FEquippedItemEntry& Pair : InventoryComponent->EquippedItems)
    {
        UItemObject* Item = Pair.Item;
        const FString& ItemType = Pair.ItemType;

        if (!Item) continue;

        if (TooltipWidgetClass)
        {
            if (UInventoryToolTip* ToolTip = CreateWidget<UInventoryToolTip>(GetWorld(), TooltipWidgetClass))
            {
                FLinearColor RarityColor = ItemStats::GetRarityDataMap()[Item->GetRarity()].Color;

                //Avoid comparing the item to itself
                UItemObject* EquippedItem = InventoryComponent->GetEquippedItemByType(ItemType);
                if (EquippedItem == Item)
                {
                    EquippedItem = nullptr;
                }

                ToolTip->Setup(Item, RarityColor, EquippedItem);
                UpdateButtonWithTooltip(Item, ItemType, ToolTip, RarityColor);
            }
        }
    }

    OnEquippedStatsUpdated(InventoryComponent->CachedEquippedStats);
}

UInventory::FSlotUIComponents UInventory::GetUIComponentsForItemType(const FString& ItemType)
{
    if (ItemType == "Helmet") return { B_Helmet, B_Helmet1, I_Helmet, ItemFrameHelmet, TB_LevelHelmet, B_LevelHelmet };
    if (ItemType == "Arms") return { B_Arms, B_Arms1, I_Arms, ItemFrameArms, TB_LevelArms, B_LevelArms };
    if (ItemType == "Upper") return { B_Upper, B_Upper1, I_Upper, ItemFrameUpper, TB_LevelUpper, B_LevelUpper };
    if (ItemType == "Lower") return { B_Lower, B_Lower1, I_Lower, ItemFrameLower, TB_LevelLower, B_LevelLower };
    if (ItemType == "Necklace") return { B_Necklace, B_Necklace1, I_Necklace, ItemFrameNecklace, TB_LevelNecklace, B_LevelNecklace };
    if (ItemType == "Greatsword"
        || ItemType == "Sword"
        || ItemType == "Bow"
        || ItemType == "Staff") return { B_Mainhand, B_Mainhand1, I_Mainhand, ItemFrameMainhand, TB_LevelMainhand, B_LevelMainhand };

    return { nullptr, nullptr, nullptr };
}

void UInventory::UpdateButtonWithTooltip(UItemObject* Item, const FString& itemType, UInventoryToolTip* ToolTip, const FLinearColor& Color)
{
    const FString& ItemType = itemType;

    auto Components = GetUIComponentsForItemType(itemType);
    if (!Components.Border || !Components.Button || !Components.Image || !Components.LevelText) return;

    Components.Button->SetToolTip(ToolTip);

    const bool HasEquipped = (ToolTip != nullptr);
    const float Opacity = HasEquipped ? 1.0f : 0.3f;
    const FLinearColor Tint = HasEquipped ? Color : FLinearColor(0.3f, 0.3f, 0.3f, Opacity);
    Components.Border->SetBrushColor(Tint);
    Components.ItemFrame->SetBrushTintColor(Color);
    Components.Image->SetColorAndOpacity(FLinearColor(1, 1, 1, Opacity));
    if (Item && Item->ItemData.Icon)
    {
        Components.Image->SetBrushFromTexture(Item->ItemData.Icon, true);
        FVector2D NewSize(75.f, 75.f);
        Components.Image->SetDesiredSizeOverride(NewSize);
    }

    Components.LevelText->SetVisibility(ESlateVisibility::SelfHitTestInvisible);

    if (Item)
    {
        Components.LevelText->SetText(FText::AsNumber(Item->GetLevel()));
        Components.LevelBorder->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
        Components.LevelBorder->SetBrushColor(Color);
    }
    else
    {
        Components.LevelText->SetText(FText::FromString(""));
        Components.LevelBorder->SetVisibility(ESlateVisibility::Collapsed);
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

    // Prevent double binding
    InventoryComponent->OnInventoryChanged.RemoveDynamic(this, &UInventory::RefreshInventoryUI);
    InventoryComponent->OnEquipmentChanged.RemoveDynamic(this, &UInventory::RefreshEquippedUI);
    InventoryComponent->OnStatsChanged.RemoveDynamic(this, &UInventory::OnEquippedStatsUpdated);
    // Add the new binds
    InventoryComponent->OnInventoryChanged.AddDynamic(this, &UInventory::RefreshInventoryUI);
    InventoryComponent->OnEquipmentChanged.AddDynamic(this, &UInventory::RefreshEquippedUI);
    InventoryComponent->OnStatsChanged.AddDynamic(this, &UInventory::OnEquippedStatsUpdated);

    UE_LOG(LogTemp, Log, TEXT("[UI] InventoryComponent successfully bound to delegates."));

    APlayerController* PC = GetOwningPlayer();
    PC = GetOwningPlayer();
    if (PC)
    {
        APawn* Pawn = PC->GetPawn();
        OwningCharacter = Cast<ARelicRunnersCharacter>(Pawn);
        if (OwningCharacter)
        {
            LocalInventory = OwningCharacter->GetInventoryComponent();
        }
    }
    
    InventoryComponent->SetSortingMethod(EInventorySorting::SortByNewest);
    OnEquippedStatsUpdated(InventoryComponent->GetCachedEquippedStats());
}

void UInventory::SetPreviewActorImage(UTextureRenderTarget2D* RenderTarget)
{
    UE_LOG(LogTemp, Warning, TEXT("[UI] Trying to set preview image with RenderTarget: %s"), *GetNameSafe(RenderTarget));

    if (!RenderTarget || !I_PlayerPreview) return;

    UMaterialInterface* BaseMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/ThirdPerson/Blueprints/PlayerPreview/M_PlayerPreview.M_PlayerPreview"));

    if (!BaseMaterial)
    {
        UE_LOG(LogTemp, Error, TEXT("[UI] Failed to load base material."));
        return;
    }

    //Create the dynamic material
    UMaterialInstanceDynamic* DynMaterial = UMaterialInstanceDynamic::Create(BaseMaterial, this);
    if (!DynMaterial)
    {
        UE_LOG(LogTemp, Error, TEXT("[UI] Failed to create dynamic material."));
        return;
    }

    DynMaterial->SetTextureParameterValue(FName("PreviewTexture"), RenderTarget);
    I_PlayerPreview->SetBrushFromMaterial(DynMaterial);

    UE_LOG(LogTemp, Warning, TEXT("[UI] Set preview image with RenderTarget: %s"), *GetNameSafe(RenderTarget));
}

UInventory* UInventory::Get(UObject* Context)
{
    if (!Context) return nullptr;

    APlayerController* PC = UGameplayStatics::GetPlayerController(Context, 0);
    if (!PC) return nullptr;

    ARelicRunnersCharacter* Character = Cast<ARelicRunnersCharacter>(PC->GetPawn());
    if (!Character) return nullptr;

    return Character->GetInventory();
}

void UInventory::EquipItem(UItemObject* Item)
{
    if (OwningCharacter && Item)
    {
        OwningCharacter->Server_EquipItemByID(Item->ItemData.UniqueID);
    }
}

void UInventory::UnequipItem(UItemObject* Item)
{
    if (OwningCharacter && Item)
    {
        OwningCharacter->Server_UnequipItemByID(Item->ItemData.UniqueID);
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

    OwningCharacter->Server_DropItem(Item->ItemData);
}

void UInventory::BuyItem(UItemObject* Item)
{
    if (!Item) return;

    if (OwningCharacter)
    {
        OwningCharacter->Server_BuyItem(Item->ItemData, CurrentVendor);
    }
}

void UInventory::SellItem(UItemObject* Item)
{
    if (!Item) return;

    if (OwningCharacter)
    {
        OwningCharacter->Server_SellItem(Item->ItemData, CurrentVendor);
    }
}

void UInventory::NativeTick(const FGeometry& MyGeometry, float DeltaTime)
{
    Super::NativeTick(MyGeometry, DeltaTime);

}

FReply UInventory::NativeOnMouseWheel(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    UInventoryItemOptions::CloseAnyOpenPopup();
    UInventorySortingOptions::CloseAnyOpenPopup();
    return FReply::Unhandled(); 
}

void UInventory::HelmetSlotClicked()
{
    OnEquippedSlotClick(InventoryComponent->GetEquippedItemByType("Helmet"));
}

void UInventory::ArmsSlotClicked()
{
    OnEquippedSlotClick(InventoryComponent->GetEquippedItemByType("Arms"));
}

void UInventory::UpperSlotClicked()
{
    OnEquippedSlotClick(InventoryComponent->GetEquippedItemByType("Upper"));
}

void UInventory::LowerSlotClicked()
{
    OnEquippedSlotClick(InventoryComponent->GetEquippedItemByType("Lower"));
}

void UInventory::NecklaceSlotClicked()
{
    OnEquippedSlotClick(InventoryComponent->GetEquippedItemByType("Necklace"));
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
            FVector2D ScreenPosition;
            UWidgetLayoutLibrary::GetMousePositionScaledByDPI(GetWorld()->GetFirstPlayerController(), ScreenPosition.X, ScreenPosition.Y);
            InventorySortingOptions->AddToViewport();
            InventorySortingOptions->SetPositionInViewport(ScreenPosition + FVector2D(0, 30.f), false);
        }
    }
}

void UInventory::OnEquippedSlotClick(UItemObject* EquippedItem)
{
    UInventorySortingOptions::CloseAnyOpenPopup();
    UInventoryItemOptions::CloseAnyOpenPopup();
    if (EquippedItem)
    {
        UInventoryItemOptions* Popup = CreateWidget<UInventoryItemOptions>(GetWorld(), InventoryItemOptionsClass);
        if (Popup)
        {
            Popup->AddToViewport();
            Popup->Setup(EquippedItem);

            //(Hide Equip, Show Unequip, Show Sell, Show Buy) for equipped items
            Popup->ConfigureButtons(false, true, false, false);

            FVector2D ScreenPosition;
            UWidgetLayoutLibrary::GetMousePositionScaledByDPI(GetWorld()->GetFirstPlayerController(), ScreenPosition.X, ScreenPosition.Y);
            Popup->SetPositionInViewport(ScreenPosition, false);
        }
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
    FGeometry InventoryGeometry = InventorySlots->GetCachedGeometry();
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
    else if (InsideForge && VendorCanvas->IsVisible() && DragOp->FromWhere == UInventoryDragOperation::Locations::Inventory)
    {
        if (DraggedItem->GetRarity() == "Relic") return true;

        if (!ForgeItemLeft)
        {
            ForgeItemLeft = DraggedItem;
            SetForgeTooltip(I_ForgeItem1, B_ForgeItem1, ForgeItemLeft);
            InventoryComponent->RemoveItemByID(DraggedItem->ItemData.UniqueID);
        }
        else if (!ForgeItemRight)
        {
            ForgeItemRight = DraggedItem;
            SetForgeTooltip(I_ForgeItem2, B_ForgeItem2, ForgeItemRight);
            InventoryComponent->RemoveItemByID(DraggedItem->ItemData.UniqueID);
        }
        ForgeItemResult = nullptr;    
        SetForgeTooltip(I_ForgeItemResult, B_ForgeItemResult, nullptr);
        UpdateForgeUI();
    }
    else if (InsideShop && VendorCanvas->IsVisible() && DragOp->FromWhere == UInventoryDragOperation::Locations::Inventory)
    {
        SellItem(DraggedItem);
    }
    else if (InsideInventory && DragOp->FromWhere == UInventoryDragOperation::Locations::Shop)
    {
        BuyItem(DraggedItem);
    }
    else
    {
        DropItem(DraggedItem);
    }

    return true;
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

    ToolTip->Setup(Item, RarityColor, nullptr);

    BorderWidget->SetToolTip(ToolTip);
}


void UInventory::OnFuseButtonPressed()
{
    if (!ForgeItemLeft || !ForgeItemRight) return;

    FString Rarity1 = ForgeRarity1;
    FString Rarity2 = ForgeRarity2;
    FString Rarity3 = ForgeRarity3;

    float Chance1 = ForgeChance1;
    float Chance2 = ForgeChance2;
    float Chance3 = ForgeChance3;

    //Determine new data
    FString NewRarity = RollForgeRarity(Rarity1, Chance1, Rarity2, Chance2, Rarity3, Chance3);
    FString NewType = ForgeItemLeft->ItemData.ItemType;
    int NewLevel = FMath::RandRange(ForgeItemLeft->ItemData.Level, ForgeItemRight->ItemData.Level);
    FItemData NewItemData = ItemStats::CreateSpecificItemData(NewLevel, NewType, NewRarity, OwningCharacter->ItemMeshData);
    UItemObject* NewItem = ItemStats::CreateItemFromData(NewItemData, InventoryComponent);

    //Clear forge items from inventory and UI
    ForgeItemLeft = nullptr;
    ForgeItemRight = nullptr;

    NewItem->IsNewItem = true;
    InventoryComponent->AddItem(NewItem);
    ForgeItemResult = NewItem;
    //Clear the forge result after 2 seconds
    GetWorld()->GetTimerManager().SetTimer(ForgeResultTimer,this,&UInventory::ClearForgeResult,2.f,false);

    UpdateForgeUI();

    if (NewRarity == Rarity1)
    {
        TB_Odds->SetColorAndOpacity(ItemStats::GetRarityDataMap()[NewRarity].Color);
        TB_Odds->SetVisibility(ESlateVisibility::Visible);
        TB_Odds->SetText(FText::FromString(FString::Printf(TEXT("Diminished"))));
    }
    if (NewRarity == Rarity2)
    {
        TB_Odds->SetColorAndOpacity(ItemStats::GetRarityDataMap()[NewRarity].Color);
        TB_Odds->SetVisibility(ESlateVisibility::Visible);
        TB_Odds->SetText(FText::FromString(FString::Printf(TEXT("Unaltered"))));
    }
    if (NewRarity == Rarity3)
    {
        TB_Odds->SetColorAndOpacity(ItemStats::GetRarityDataMap()[NewRarity].Color);
        TB_Odds->SetVisibility(ESlateVisibility::Visible);
        TB_Odds->SetText(FText::FromString(FString::Printf(TEXT("Refined"))));
    }

    TB_Odds1->SetVisibility(ESlateVisibility::Visible);
    TB_Odds1->SetText(FText::FromString(FString::Printf(TEXT("%s item added to Inventory"), *NewRarity)));
    TB_Odds1->SetColorAndOpacity(ItemStats::GetRarityDataMap()[NewRarity].Color);

    SetForgeTooltip(I_ForgeItemResult, B_ForgeItemResult, ForgeItemResult);
    SetForgeTooltip(I_ForgeItem2, B_ForgeItem2, nullptr);
    SetForgeTooltip(I_ForgeItem1, B_ForgeItem1, nullptr);

    if (ForgeCompleted)
    {
        PlayAnimation(ForgeCompleted, 0.f, 1); 
    }
}

void UInventory::ClearForgeResult()
{
    ForgeItemResult = nullptr;

    SetForgeTooltip(I_ForgeItemResult, B_ForgeItemResult, nullptr);

    UpdateForgeUI();
}


FString UInventory::RollForgeRarity(const FString R1, float C1, const FString R2, float C2, const FString R3, float C3)
{
    float Roll = FMath::FRandRange(0.f, 100.f);

    if (Roll <= C1) return R1;
    if (Roll <= C1 + C2) return R2;
    return R3;
}


void UInventory::UpdateForgeOdds(const FString& LeftRarity, const FString& RightRarity)
{
    static const TArray<FString> RarityOrder = {
        "Common",
        "Uncommon",
        "Rare",
        "Epic",
        "Legendary",
        "Mythic",
        "Relic"
    };

    static const float UpgradeSameTier[7] = {
        90.f, // Common
        80.f, // Uncommon
        70.f, // Rare
        60.f, // Epic
        50.f, // Legendary
        40.f, // Mythic
        0.f   // Relic
    };

    auto GetIndex = [&](const FString& R) { return RarityOrder.IndexOfByKey(R); };

    int IndexA = GetIndex(LeftRarity);
    int IndexB = GetIndex(RightRarity);

    int Highest = FMath::Max(IndexA, IndexB);
    int Lowest = FMath::Min(IndexA, IndexB);
    int Distance = Highest - Lowest;

    int DownIndex = FMath::Clamp(Highest - 1, 0, RarityOrder.Num() - 1);
    int StayIndex = Highest;
    int UpIndex = FMath::Clamp(Highest + 1, 0, RarityOrder.Num() - 1);

    ForgeRarity1 = RarityOrder[DownIndex];  
    ForgeRarity2 = RarityOrder[StayIndex]; 
    ForgeRarity3 = RarityOrder[UpIndex];   

    float Chance1 = 0.f;
    float Chance2 = 0.f; 
    float Chance3 = 0.f; 

    //Same rarity
    if (IndexA == IndexB)
    {
        float Up = UpgradeSameTier[Highest];
        float RemainingPercent = 100.0f - Up;
        float Stay = RemainingPercent * 0.6f;
        float Down = RemainingPercent * 0.4f;

        Chance1 = Down;
        Chance2 = Stay;
        Chance3 = Up;
    }
    //Different rarity
    else
    {
        float Proximity = 1.f / float(Distance);

        float Up = UpgradeSameTier[Highest] * Proximity * 0.8f;
        float RemainingPercent = 100.0f - Up;
        float Stay = RemainingPercent * 0.6f;
        float Down = RemainingPercent * 0.4f;

        Chance1 = Down;
        Chance2 = Stay;
        Chance3 = Up;
    }

    //Normalize
    float Total = Chance1 + Chance2 + Chance3;
    Chance1 = (Chance1 / Total) * 100.f;
    Chance2 = (Chance2 / Total) * 100.f;
    Chance3 = (Chance3 / Total) * 100.f;

    ForgeChance1 = Chance1;
    ForgeChance2 = Chance2;
    ForgeChance3 = Chance3;

    //Check if textblocks are showing the same rarity and merge
    bool MergeDownAndStay = (DownIndex == StayIndex);
    if (MergeDownAndStay)
    {
        float Combined = Chance1 + Chance2;

        TB_Odds->SetVisibility(ESlateVisibility::Hidden);
        TB_Odds1->SetText(FText::FromString(FString::Printf(TEXT("%s: %.1f%%"), *ForgeRarity2, Combined)));
        TB_Odds2->SetText(FText::FromString(FString::Printf(TEXT("%s: %.1f%%"), *ForgeRarity3, Chance3)));
    }
    else
    {
        TB_Odds->SetText(FText::FromString(FString::Printf(TEXT("%s: %.1f%%"), *ForgeRarity1, Chance1)));
        TB_Odds->SetVisibility(ESlateVisibility::Visible);
        TB_Odds1->SetText(FText::FromString(FString::Printf(TEXT("%s: %.1f%%"), *ForgeRarity2, Chance2)));
        TB_Odds2->SetText(FText::FromString(FString::Printf(TEXT("%s: %.1f%%"), *ForgeRarity3, Chance3)));
    }

    TB_Odds1->SetVisibility(ESlateVisibility::Visible);
    TB_Odds2->SetVisibility(ESlateVisibility::Visible);

    TB_Odds->SetColorAndOpacity(ItemStats::GetRarityDataMap()[ForgeRarity1].Color);
    TB_Odds1->SetColorAndOpacity(ItemStats::GetRarityDataMap()[ForgeRarity2].Color);
    TB_Odds2->SetColorAndOpacity(ItemStats::GetRarityDataMap()[ForgeRarity3].Color);
}

void UInventory::UpdateForgeUI()
{
    bool LeftFilled = (ForgeItemLeft != nullptr);
    bool RightFilled = (ForgeItemRight != nullptr);
    bool BothFilled = LeftFilled && RightFilled;

    Throbber1->SetRenderOpacity(LeftFilled ? 1.0f : 0.2f);
    Throbber2->SetRenderOpacity(RightFilled ? 1.0f : 0.2f);

    if (LeftFilled)
    {
        TB_ForgeItemLevel1->SetText(FText::AsNumber(ForgeItemLeft->ItemData.Level));

        FSlateBrush Brush;
        Brush.SetResourceObject(ForgeItemLeft->ItemData.Icon);
        Brush.ImageSize = FVector2D(75.f, 75.f);
        Brush.TintColor = FSlateColor(FLinearColor::White);
        I_ForgeItem1->SetBrush(Brush);
        FLinearColor color = ItemStats::GetRarityDataMap()[ForgeItemLeft->GetRarity()].Color;
        ItemFrame->SetBrushTintColor(color);
        B_ForgeItem1->SetBrushColor(color);
        B_Level->SetBrushColor(color);
        B_Level->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
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
        TB_ForgeItemLevel2->SetText(FText::AsNumber(ForgeItemRight->ItemData.Level));

        FSlateBrush Brush;
        Brush.SetResourceObject(ForgeItemRight->ItemData.Icon);
        Brush.ImageSize = FVector2D(75.f, 75.f);
        Brush.TintColor = FSlateColor(FLinearColor::White);
        I_ForgeItem2->SetBrush(Brush);
        FLinearColor color = ItemStats::GetRarityDataMap()[ForgeItemRight->GetRarity()].Color;
        ItemFrame_1->SetBrushTintColor(color);
        B_ForgeItem2->SetBrushColor(color);
        B_Level_1->SetBrushColor(color);
        B_Level_1->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
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
        UpdateForgeOdds(ForgeItemLeft->ItemData.Rarity, ForgeItemRight->ItemData.Rarity);
    }

    if (!LeftFilled || !RightFilled)
    {
        B_ForgeItemResult->SetBrushColor(FLinearColor::Transparent);
        TB_ForgeItemResultLevel->SetText(FText::GetEmpty());

        // Clear icon
        FSlateBrush ClearBrush;
        ClearBrush.SetResourceObject(nullptr);
        ClearBrush.ImageSize = FVector2D(75.f, 75.f);
        ClearBrush.TintColor = FSlateColor(FLinearColor(1, 1, 1, 0));
        I_ForgeItemResult->SetBrush(ClearBrush);
        B_Level_2->SetVisibility(ESlateVisibility::Hidden);
        ItemFrame_2->SetBrushTintColor(FLinearColor(0.2, 0.2, 0.2, 1));
    }
    if (ForgeItemResult)
    {
        FSlateBrush ResultBrush;
        ResultBrush.SetResourceObject(ForgeItemResult->ItemData.Icon);
        ResultBrush.ImageSize = FVector2D(75.f, 75.f);
        ResultBrush.TintColor = FSlateColor(FLinearColor::White);
        I_ForgeItemResult->SetBrush(ResultBrush);
        TB_ForgeItemResultLevel->SetText(FText::AsNumber(ForgeItemResult->ItemData.Level));
        FLinearColor RarityColor = ItemStats::GetRarityDataMap()[ForgeItemResult->GetRarity()].Color;
        B_ForgeItemResult->SetBrushColor(RarityColor);
        B_Level_2->SetBrushColor(RarityColor);
        B_Level_2->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
        ItemFrame_2->SetBrushTintColor(RarityColor);
    }
}

void UInventory::OnEquippedStatsUpdated(const FEquippedStatsSummary& Stats)
{
    if (TB_Health)       TB_Health->SetText(FText::AsNumber(Stats.TotalHealth));
    if (TB_Armor)        TB_Armor->SetText(FText::AsNumber(Stats.TotalArmor));
    if (TB_Dexterity)    TB_Dexterity->SetText(FText::AsNumber(Stats.TotalDexterity));
    if (TB_Strength)     TB_Strength->SetText(FText::AsNumber(Stats.TotalStrength));
    if (TB_Intelligence) TB_Intelligence->SetText(FText::AsNumber(Stats.TotalIntelligence));

    if (OwningCharacter)
    {
        if (TB_Gold) TB_Gold->SetText(FText::AsNumber(OwningCharacter->GetPlayerGold()));
        if (ARelicRunnersPlayerState* PS = Cast<ARelicRunnersPlayerState>(OwningCharacter->GetPlayerState()))
        {
            if(TB_Username) TB_Username->SetText(FText::FromString(PS->GetPlayerName()));
        }
    }

    int NumItems = InventoryComponent->InventoryItems.Num();
    int MaxSlots = OwningCharacter->GetPlayerNumInventorySlots();

    TB_InventorySlots->SetText(FText::FromString(FString::Printf(TEXT("%d / %d"), NumItems, MaxSlots)));
}