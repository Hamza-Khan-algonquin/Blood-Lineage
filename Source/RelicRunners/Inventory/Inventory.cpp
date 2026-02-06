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
#include "RelicRunners/PlayerController/RelicRunnersPlayerController.h"
#include <RelicRunners/Game/RelicRunnersGameInstance.h>
#include "InventoryDragOperation.h"
#include "InventorySlotsEntry.h"
#include "RelicRunners/Menu/StandardTooltip.h"
#include <Blueprint/SlateBlueprintLibrary.h>

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
    if (ShopCanvas) ShopCanvas->SetVisibility(ESlateVisibility::Collapsed);
    if (ForgeCanvas) ForgeCanvas->SetVisibility(ESlateVisibility::Collapsed);
    if (VendorSlots) VendorSlots->ClearListItems();
    if (FuseButton) FuseButton->OnClicked.AddDynamic(this, &UInventory::OnFuseButtonPressed);
    if (B_ForgeItem1) B_ForgeItem1->OnMouseButtonDownEvent.BindUFunction(this, "OnForgeItem1Clicked");
    if (B_ForgeItem2) B_ForgeItem2->OnMouseButtonDownEvent.BindUFunction(this, "OnForgeItem2Clicked");
    if (B_ForgeItemResult) B_ForgeItemResult->OnMouseButtonDownEvent.BindUFunction(this, "OnForgeItemResultClicked");
}

FEventReply UInventory::OnForgeItem1Clicked(FGeometry MyGeometry, const FPointerEvent& MouseEvent)
{
    if (MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton && ForgeItemLeft)
    {
        CreateItemOptionsPopup(ForgeItemLeft, FItemOptionsUIConfig::ForgeItem(), MyGeometry);
        return UWidgetBlueprintLibrary::Handled();
    }

    return UWidgetBlueprintLibrary::Unhandled();
}

FEventReply UInventory::OnForgeItem2Clicked(FGeometry MyGeometry, const FPointerEvent& MouseEvent)
{
    FEventReply Reply;

    if (MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton && ForgeItemRight)
    {
        CreateItemOptionsPopup(ForgeItemRight, FItemOptionsUIConfig::ForgeItem(), MyGeometry);
        return UWidgetBlueprintLibrary::Handled();
    }

    return UWidgetBlueprintLibrary::Unhandled();
}

FEventReply UInventory::OnForgeItemResultClicked(FGeometry MyGeometry, const FPointerEvent& MouseEvent)
{
    FEventReply Reply;

    if (MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton && ForgeItemResult)
    {
        CreateItemOptionsPopup(ForgeItemResult, FItemOptionsUIConfig::ForgeResultItem(), MyGeometry);
        return UWidgetBlueprintLibrary::Handled();
    }

    return UWidgetBlueprintLibrary::Unhandled();
}

void UInventory::ClearForgeItemLeft()
{
    if (ForgeItemLeft)
    {
        InventoryComponent->AddItem(ForgeItemLeft);
        ForgeItemLeft = nullptr;
        SetForgeTooltip(I_ForgeItem1, B_ForgeItem1, nullptr);
    }
}
void UInventory::ClearForgeItemRight()
{
    if (ForgeItemRight)
    {
        InventoryComponent->AddItem(ForgeItemRight);
        ForgeItemRight = nullptr;
        SetForgeTooltip(I_ForgeItem2, B_ForgeItem2, nullptr);
    }
}
void UInventory::ClearForgeItemResult()
{
    if (ForgeItemResult)
    {
        InventoryComponent->AddItem(ForgeItemResult);
        ForgeItemResult = nullptr;
        SetForgeTooltip(I_ForgeItemResult, B_ForgeItemResult, nullptr);
    }
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

        ClearForgeItemLeft();
        ClearForgeItemRight();
        ClearForgeItemResult();

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
    }

    InventoryComponent->SetSortingMethod(EInventorySorting::SortByNewest);
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
    if (Item->IsNewItem) Item->IsNewItem = false;

    if (ForgeItemResult == Item)
    {
        ClearForgeItemResult();
        UpdateForgeUI();
    }

    if (OwningCharacter && Item)
    {
        if (OwningCharacter->RelicPlayerState)
        {
            OwningCharacter->RelicPlayerState->Server_EquipItemByID(Item->GetUniqueID());
        }
    }
}

void UInventory::UnequipItem(UItemObject* Item)
{
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

    if (ARelicRunnersPlayerState* PS = GetOwningPlayerState<ARelicRunnersPlayerState>())
    {
        AVendor* Shop = Cast<AVendor>(CurrentVendor);
        PS->Server_BuyItem(Item->ItemData, Shop);
    }
}

void UInventory::SellItem(UItemObject* Item)
{
    if (!Item) return;

    if (ARelicRunnersPlayerState* PS = GetOwningPlayerState<ARelicRunnersPlayerState>())
    {
        AVendor* Shop = Cast<AVendor>(CurrentVendor);
        PS->Server_SellItem(Item->ItemData, Shop);
    }
}

void UInventory::RemoveForgeItem(UItemObject* ItemToRemove)
{
    if (!ItemToRemove) return;

    //Check which forge slot it belongs to
    if (ForgeItemLeft == ItemToRemove)
    {
        ClearForgeItemLeft();
    }
    else if (ForgeItemRight == ItemToRemove)
    {
        ClearForgeItemRight();
    }
    else if (ForgeItemResult == ItemToRemove)
    {
        ClearForgeItemResult();
    }

    UpdateForgeUI();
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
        }
    }
    else
    {
        ToggleShopUI(false);
        ToggleForgeUI(false);
    }
}

FReply UInventory::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    if (InMouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton))
    {
        FEventReply EventReply = UWidgetBlueprintLibrary::DetectDragIfPressed(InMouseEvent, this, EKeys::LeftMouseButton);
        return EventReply.NativeReply;
    }

    return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

FReply UInventory::NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    Super::NativeOnMouseButtonUp(InGeometry, InMouseEvent);

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

    TArray<UBorder*> Borders = { B_Health, B_Defence, B_Strength, B_Agility, B_Intelligence};

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

    return FReply::Handled();
}

void UInventory::ShowTooltipForBorder(UBorder* HoveredBorder, const FVector2D& CursorPos, const FGeometry& ParentGeometry)
{
    if (!StandardTooltipClass) return;

    if (!HoveredBorder) return;

    APlayerController* PC = GetOwningPlayer();
    if (!PC) return;

    if (!StandardTooltip)
    {
        StandardTooltip = CreateWidget<UStandardTooltip>(PC, StandardTooltipClass);
        StandardTooltip->AddToViewport();
    }

    FEquippedStatsSummary Stats = InventoryComponent->GetCachedEquippedStats();

    FText TooltipName = FText::GetEmpty();
    FText TooltipValue = FText::GetEmpty();
    TArray<FText> Descriptions;

    if (HoveredBorder == B_Health)
    {
        TooltipName = FText::FromString("Health");
        TooltipValue = FText::AsNumber(Stats.TotalHealth);
        StandardTooltip->SetIcon(ItemStats::LoadIcon("HealthIcon"), ItemStats::GetIconColor("Health"));

        Descriptions.Add(FText::FromString("Maximum health pool."));
    }
    else if (HoveredBorder == B_Defence)
    {
        TooltipName = FText::FromString("Defence");
        TooltipValue = FText::AsNumber(Stats.TotalArmor);
        StandardTooltip->SetIcon(ItemStats::LoadIcon("DefenceIcon"), ItemStats::GetIconColor("Defence"));

        float Percentage = ItemStats::GetDefencePerc(Stats.TotalArmor);
        FText FormatString = FText::FromString("Damage taken reduced by {0}%.");
        FText FormattedDescription = FText::Format(FormatString, FText::AsNumber(Percentage));
        Descriptions.Add(FormattedDescription);
    }
    else if (HoveredBorder == B_Strength)
    {
        TooltipName = FText::FromString("Strength");
        TooltipValue = FText::AsNumber(Stats.TotalStrength);
        StandardTooltip->SetIcon(ItemStats::LoadIcon("StrengthIcon"), ItemStats::GetIconColor("Strength"));

        float Percentage = ItemStats::GetStrengthPerc(Stats.TotalStrength);
        FText FormatString = FText::FromString("Attack and ability damage +{0}%.");
        FText FormattedDescription = FText::Format(FormatString, FText::AsNumber(Percentage));
        Descriptions.Add(FormattedDescription);
    }
    else if (HoveredBorder == B_Agility)
    {
        TooltipName = FText::FromString("Agility");
        TooltipValue = FText::AsNumber(Stats.TotalDexterity);
        StandardTooltip->SetIcon(ItemStats::LoadIcon("AgilityIcon"), ItemStats::GetIconColor("Agility"));

        float Percentage = ItemStats::GetCritPerc(Stats.TotalDexterity);
        FText FormatString = FText::FromString("Critical strike chance increased by {0}%.");
        FText FormattedDescription = FText::Format(FormatString, FText::AsNumber(Percentage));
        Descriptions.Add(FormattedDescription);

        float PercentageDamage = ItemStats::GetCritDmgPerc(Stats.TotalDexterity);
        FormatString = FText::FromString("Critical strikes deal {0}x damage.");
        FormattedDescription = FText::Format(FormatString, FText::AsNumber(PercentageDamage));
        Descriptions.Add(FormattedDescription);

        Percentage = ItemStats::GetSpeedPerc(Stats.TotalDexterity);
        FormatString = FText::FromString("Movement speed increased by {0}%.");
        FormattedDescription = FText::Format(FormatString, FText::AsNumber(Percentage));
        Descriptions.Add(FormattedDescription);
    }
    else if (HoveredBorder == B_Intelligence)
    {
        TooltipName = FText::FromString("Intelligence");
        TooltipValue = FText::AsNumber(Stats.TotalIntelligence);
        StandardTooltip->SetIcon(ItemStats::LoadIcon("IntelligenceIcon"), ItemStats::GetIconColor("Intelligence"));

        int Num = ItemStats::GetManaPoolIncrease(Stats.TotalIntelligence);
        FText FormatString = FText::FromString("Maximum mana pool increased by {0}.");
        FText FormattedDescription = FText::Format(FormatString, FText::AsNumber(Num));
        Descriptions.Add(FormattedDescription);

        float Percentage = ItemStats::GetAbilityCostPerc(Stats.TotalIntelligence);
        FormatString = FText::FromString("Reduces all ability costs by {0}%.");
        FormattedDescription = FText::Format(FormatString, FText::AsNumber(Percentage));
        Descriptions.Add(FormattedDescription);
    }

    FVector2D ViewportPosition = ParentGeometry.AbsoluteToLocal(CursorPos);

    const FVector2D Offset(-5.0f, 5.0f);
    const FVector2D Anchor(1.f, 0.f);

    StandardTooltip->SetAnchor(Anchor);
    StandardTooltip->Setup(TooltipName, TooltipValue, Descriptions);
    StandardTooltip->SetPositionInViewport(ViewportPosition + Offset, false);
}

void UInventory::HelmetSlotClicked()
{
    if (B_Helmet)
    {
        OnEquippedSlotClick(InventoryComponent->GetEquippedItemByType("Helmet"), B_Helmet->GetCachedGeometry());
    }
}

void UInventory::ArmsSlotClicked()
{
    if (B_Arms)
    {
        OnEquippedSlotClick(InventoryComponent->GetEquippedItemByType("Arms"), B_Arms->GetCachedGeometry());
    }
}

void UInventory::UpperSlotClicked()
{
    if (B_Upper)
    {
        OnEquippedSlotClick(InventoryComponent->GetEquippedItemByType("Upper"), B_Upper->GetCachedGeometry());
    }
}

void UInventory::LowerSlotClicked()
{
    if (B_Lower)
    {
        OnEquippedSlotClick(InventoryComponent->GetEquippedItemByType("Lower"), B_Lower->GetCachedGeometry());
    }
}

void UInventory::NecklaceSlotClicked()
{
    if (B_Necklace)
    {
        OnEquippedSlotClick(InventoryComponent->GetEquippedItemByType("Necklace"), B_Necklace->GetCachedGeometry());
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
    else if (InsideForge && ForgeCanvas->IsVisible() && DragOp->FromWhere == UInventoryDragOperation::Locations::Inventory)
    {
        ForgeItem(DraggedItem);
    }
    else if (InsideShop && ShopCanvas->IsVisible() && DragOp->FromWhere == UInventoryDragOperation::Locations::Inventory)
    {
        SellItem(DraggedItem);
    }
    else if (InsideInventory && DragOp->FromWhere == UInventoryDragOperation::Locations::Shop)
    {
        BuyItem(DraggedItem);
    }
    else if (InsideInventory && DragOp->FromWhere == UInventoryDragOperation::Locations::Inventory)
    {
        return true;
    }
    else
    {
        DropItem(DraggedItem);
    }

    return true;
}

void UInventory::ForgeItem(UItemObject* Item)
{
    if (!ForgeCanvas->IsVisible()) return;
    if (Item->GetRarity() == "Relic") return;

    ClearForgeItemResult();

    if (!ForgeItemLeft)
    {
        ForgeItemLeft = Item;
        SetForgeTooltip(I_ForgeItem1, B_ForgeItem1, ForgeItemLeft);
        InventoryComponent->RemoveItemByID(Item->ItemData.UniqueID);
    }
    else if (!ForgeItemRight)
    {
        ForgeItemRight = Item;
        SetForgeTooltip(I_ForgeItem2, B_ForgeItem2, ForgeItemRight);
        InventoryComponent->RemoveItemByID(Item->ItemData.UniqueID);
    }

    UpdateForgeUI();
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
    if (!ForgeItemLeft || !ForgeItemRight) return;

    FString NewRarity = RollForgeRarity(ForgeRarity1, ForgeChance1, ForgeRarity2, ForgeChance2, ForgeRarity3, ForgeChance3);

    FString NewType;
    if (FMath::RandBool())
    {
        NewType = ForgeItemLeft->ItemData.ItemType;
    }
    else
    {
        NewType = ForgeItemRight->ItemData.ItemType;
    }
    int NewLevel = FMath::RandRange(ForgeItemLeft->ItemData.Level, ForgeItemRight->ItemData.Level);

    FItemData NewItemData = ItemStats::CreateSpecificItemData(NewLevel, NewType, NewRarity, OwningCharacter->RelicPlayerState->ItemMeshData);
    ForgeItemResult = ItemStats::CreateItemFromData(NewItemData, InventoryComponent);

    ForgeItemLeft = nullptr;
    ForgeItemRight = nullptr;
    UpdateForgeUI();

    PlayAnimation(RollRarity);

    CycleCount = 0;

    GetWorld()->GetTimerManager().SetTimer(FuseTimerHandle, this, &UInventory::CycleRarityVisuals, 0.1f, true);
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
    if (!ForgeItemResult) return;

    FLinearColor FinalColor = ItemStats::GetRarityDataMap()[ForgeItemResult->ItemData.Rarity].Color;

    UpdateForgeUI();

    if (ForgeItemResult->ItemData.Rarity == ForgeRarity1)
    {
        TB_Odds->SetColorAndOpacity(ItemStats::GetRarityDataMap()[ForgeItemResult->ItemData.Rarity].Color);
        TB_Odds->SetVisibility(ESlateVisibility::Visible);
        TB_Odds->SetText(FText::FromString(FString::Printf(TEXT("Quality Brittled"))));
    }
    if (ForgeItemResult->ItemData.Rarity == ForgeRarity2)
    {
        TB_Odds->SetColorAndOpacity(ItemStats::GetRarityDataMap()[ForgeItemResult->ItemData.Rarity].Color);
        TB_Odds->SetVisibility(ESlateVisibility::Visible);
        TB_Odds->SetText(FText::FromString(FString::Printf(TEXT("Quality Unaffected"))));
    }
    if (ForgeItemResult->ItemData.Rarity == ForgeRarity3)
    {
        TB_Odds->SetColorAndOpacity(ItemStats::GetRarityDataMap()[ForgeItemResult->ItemData.Rarity].Color);
        TB_Odds->SetVisibility(ESlateVisibility::Visible);
        TB_Odds->SetText(FText::FromString(FString::Printf(TEXT("Quality Polished"))));
    }

    TB_Odds1->SetVisibility(ESlateVisibility::Visible);
    TB_Odds1->SetText(FText::FromString(FString::Printf(TEXT("%s item!"), *ForgeItemResult->ItemData.Rarity)));
    TB_Odds1->SetColorAndOpacity(ItemStats::GetRarityDataMap()[ForgeItemResult->ItemData.Rarity].Color);

    if (ItemFrame_2) ItemFrame_2->SetBrushTintColor(FinalColor);
    if (B_ForgeItemResult) B_ForgeItemResult->SetBrushColor(FinalColor);

    SetForgeTooltip(I_ForgeItemResult, B_ForgeItemResult, ForgeItemResult);
    SetForgeTooltip(I_ForgeItem2, B_ForgeItem2, nullptr);
    SetForgeTooltip(I_ForgeItem1, B_ForgeItem1, nullptr);

    if (ForgeCompleted)
    {
        PlayAnimation(ForgeCompleted);
        PlayAnimation(ForgeShake);
    }
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

    Throbber1->SetRenderOpacity(LeftFilled ? 1.0f : 0.0f);
    Throbber2->SetRenderOpacity(RightFilled ? 1.0f : 0.0f);

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
        ClearBrush.ImageSize = FVector2D(100.f, 100.f);
        ClearBrush.TintColor = FSlateColor(FLinearColor(1, 1, 1, 0));
        I_ForgeItemResult->SetBrush(ClearBrush);
        B_Level_2->SetVisibility(ESlateVisibility::Hidden);
        ItemFrame_2->SetBrushTintColor(FLinearColor(0.2, 0.2, 0.2, 1));
    }
    if (ForgeItemResult)
    {
        FSlateBrush ResultBrush;
        ResultBrush.SetResourceObject(ForgeItemResult->ItemData.Icon);
        ResultBrush.ImageSize = FVector2D(100.f, 100.f);
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