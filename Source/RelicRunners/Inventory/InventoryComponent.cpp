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

#include "InventoryComponent.h"
#include "Net/UnrealNetwork.h"
#include "Inventory.h"
#include "InventorySortingOptions.h"
#include "RelicRunners/Item/ItemData.h"
#include "RelicRunners/RelicRunnersCharacter.h"
#include "RelicRunners/Enemy/EnemyCharacterAI.h"
#include "Engine/ActorChannel.h"
#include "RelicRunners/Item/ItemStats.h"
#include <RelicRunners/PlayerController/RelicRunnersPlayerController.h>

UInventoryComponent::UInventoryComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    SetIsReplicatedByDefault(true);
}

void UInventoryComponent::BeginPlay()
{
    Super::BeginPlay();

    UE_LOG(LogTemp, Warning, TEXT("InventoryComponent BeginPlay on %s (Role: %d)"), *GetOwner()->GetName(), (int32)GetOwnerRole());
}

bool UInventoryComponent::ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags)
{
    bool WroteSomething = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);

    for (UItemObject* Item : InventoryItems)
    {
        if (Item)
        {
            WroteSomething |= Channel->ReplicateSubobject(Item, *Bunch, *RepFlags);
        }
    }

    for (const FEquippedItemEntry& Entry : EquippedItems)
    {
        if (Entry.Item)
        {
            WroteSomething |= Channel->ReplicateSubobject(Entry.Item, *Bunch, *RepFlags);
        }
    }

    return WroteSomething;
}

void UInventoryComponent::AddItem(UItemObject* Item)
{
    if (!Item) return;

    InventoryItems.Add(Item);
    SortInventoryByCurrentMethod();
    OnInventoryChanged.Broadcast();
}

void UInventoryComponent::RemoveItem(UItemObject* Item)
{
    if (!Item) return;

    for (int i = 0; i < InventoryItems.Num(); i++)
    {
        if (InventoryItems[i] && InventoryItems[i]->ItemData.UniqueID == Item->ItemData.UniqueID)
        {
            InventoryItems.RemoveAt(i);
            SortInventoryByCurrentMethod();
            OnInventoryChanged.Broadcast();
            return;
        }
    }
}


void UInventoryComponent::OnRep_SortingMethod()
{
    HandleSortingNow();
}

void UInventoryComponent::SetSortingMethod(EInventorySorting NewMethod)
{
    if (CurrentSortingMethod == NewMethod) return;

    CurrentSortingMethod = NewMethod;

    if (!GetOwner()->HasAuthority())
    {
        // Client requests the server to sort
        Server_SortInventory(NewMethod);
        return;
    }

    // Server sorts immediately
    SortInventoryByCurrentMethod();
}

void UInventoryComponent::SortInventoryByCurrentMethod()
{
    if (!GetOwner() || !GetOwner()->HasAuthority())
    {
        return;
    }

    HandleSortingNow();
    OnInventoryChanged.Broadcast();
}

void UInventoryComponent::Server_SortInventory_Implementation(EInventorySorting Method)
{
    SetSortingMethod(Method);
}

void UInventoryComponent::HandleSortingNow()
{
    TArray<UItemObject*>& Items = InventoryItems;

    const TMap<FString, ItemStats::FRarityData>& RarityMap = ItemStats::GetRarityDataMap();
    TMap<FString, int> RarityRank;

    int Index = 0;
    for (const auto& Elem : RarityMap)
    {
        RarityRank.Add(Elem.Key, Index++);
    }

    //Different sorting methods
    switch (CurrentSortingMethod)
    {
    case EInventorySorting::SortByItemType:
        Items.Sort([&RarityRank](UItemObject& A, UItemObject& B)
            {
                //Sort itemtype first
                if (A.ItemData.ItemType != B.ItemData.ItemType) return A.ItemData.ItemType < B.ItemData.ItemType;

                //Sort rarity second
                int ARank = RarityRank.FindRef(A.ItemData.Rarity);
                int BRank = RarityRank.FindRef(B.ItemData.Rarity);
                if (ARank != BRank) return ARank > BRank;

                //Sort level third
                return A.ItemData.Level > B.ItemData.Level;
            });
        break;

    case EInventorySorting::SortByRarity:
        Items.Sort([&RarityRank](UItemObject& A, UItemObject& B)
            {
                //Sort rarity first
                int ARank = RarityRank.FindRef(A.ItemData.Rarity);
                int BRank = RarityRank.FindRef(B.ItemData.Rarity);
                if (ARank != BRank) return ARank > BRank;

                //Sort level second
                if (A.ItemData.Level != B.ItemData.Level) return A.ItemData.Level > B.ItemData.Level;

                //Sort itemtype third
                return A.ItemData.ItemType < B.ItemData.ItemType;
            });
        break;
    case EInventorySorting::SortByNewest:
        Items.Sort([&RarityRank](UItemObject& A, UItemObject& B)
            {
                //If new
                //Sort rarity first
                if (A.IsNewItem)
                {
                    int ARank = RarityRank.FindRef(A.ItemData.Rarity);
                    int BRank = RarityRank.FindRef(B.ItemData.Rarity);
                    if (ARank != BRank) return ARank > BRank;

                    //Sort level second
                    if (A.ItemData.Level != B.ItemData.Level) return A.ItemData.Level > B.ItemData.Level;

                    //Sort itemtype third
                    return A.ItemData.ItemType < B.ItemData.ItemType;
                }

                //If not new
                //Sort rarity first
                int ARank = RarityRank.FindRef(A.ItemData.Rarity);
                int BRank = RarityRank.FindRef(B.ItemData.Rarity);
                if (ARank != BRank) return ARank > BRank;

                //Sort level second
                if (A.ItemData.Level != B.ItemData.Level) return A.ItemData.Level > B.ItemData.Level;

                //Sort itemtype third
                return A.ItemData.ItemType < B.ItemData.ItemType;
            });
        break;

    case EInventorySorting::SortByLevel:
        Items.Sort([&RarityRank](UItemObject& A, UItemObject& B)
            {
                //Sort level first
                if (A.ItemData.Level != B.ItemData.Level) return A.ItemData.Level > B.ItemData.Level;

                //Sort rarity second
                int ARank = RarityRank.FindRef(A.ItemData.Rarity);
                int BRank = RarityRank.FindRef(B.ItemData.Rarity);
                if (ARank != BRank) return ARank > BRank;

                //Sort itemtype third
                return A.ItemData.ItemType < B.ItemData.ItemType;
            });
        break;
    default:
        break;
    }

    OnInventoryChanged.Broadcast();
}

void UInventoryComponent::EquipItem(UItemObject* Item)
{
    if (!Item) return;

    const FString Slot = Item->ItemData.ItemType;
    bool bFound = false;
    
    for (FEquippedItemEntry& Entry : EquippedItems)
    {
        if (Entry.ItemType == Slot)
        {
            if (Entry.Item)
            {
                AddItem(Entry.Item);
            }
            Entry.Item = Item;
            bFound = true;
            break;
        }
    }

    if (!bFound)
    {
        EquippedItems.Add(FEquippedItemEntry(Slot, Item));
    }

    int Index = InventoryItems.IndexOfByPredicate([&](const UItemObject* OtherItem)
        {
            return OtherItem && OtherItem->ItemData.UniqueID == Item->ItemData.UniqueID;
        });

    if (Index != INDEX_NONE)
    {
        InventoryItems.RemoveAt(Index);
    }

    if (ARelicRunnersPlayerController* PC = Cast<ARelicRunnersPlayerController>(GetWorld()->GetFirstPlayerController()))
    {
        PC->UpdatePreviewWithEquippedItems();
    }

    OnEquipmentChanged.Broadcast();
    OnInventoryChanged.Broadcast();
}

void UInventoryComponent::OnRep_Inventory()
{
    OnInventoryChanged.Broadcast();
}

void UInventoryComponent::OnRep_EquippedItems()
{
    OnEquipmentChanged.Broadcast();
}

void UInventoryComponent::UnequipItem(UItemObject* Item)
{
    if (!Item) return;

    for (int i = 0; i < EquippedItems.Num(); ++i)
    {
        if (EquippedItems[i].Item == nullptr) continue;

        if (EquippedItems[i].Item->GetUniqueID() == Item->GetUniqueID())
        {
            InventoryItems.Add(Item);
            EquippedItems.RemoveAt(i);

            OnEquipmentChanged.Broadcast();
            OnInventoryChanged.Broadcast();
            return;
        }
    }

    if (ARelicRunnersPlayerController* PC = Cast<ARelicRunnersPlayerController>(GetWorld()->GetFirstPlayerController()))
    {
        PC->UpdatePreviewWithEquippedItems();
    }
}

UItemObject* UInventoryComponent::GetEquippedItemByType(const FString& ItemType) const
{
    for (const FEquippedItemEntry& Entry : EquippedItems)
    {
        if (Entry.ItemType == ItemType)
        {
            return Entry.Item;
        }
    }
    return nullptr;
}

UItemObject*& UInventoryComponent::GetEquippedItemReference(const FString& ItemType)
{
    for (FEquippedItemEntry& Entry : EquippedItems)
    {
        if (Entry.ItemType == ItemType)
        {
            return Entry.Item;
        }
    }

    EquippedItems.Add(FEquippedItemEntry(ItemType, nullptr));
    return EquippedItems.Last().Item;
}

void UInventoryComponent::UpdateTotalEquippedStats(ARelicRunnersCharacter* Char)
{
    if (!Char) return;

    FEquippedStatsSummary Stats = CalculateEquippedStats();

    if (Char->HasAuthority())
    {
        Char->Server_SetMaxHealth(Stats.TotalHealth);
        Char->Client_UpdateEquippedStats(Stats);
    }

    CachedEquippedStats = Stats;
    OnStatsChanged.Broadcast(Stats);
}

void UInventoryComponent::SwapItems(UItemObject* ItemA, UItemObject* ItemB)
{
    if (!ItemA || !ItemB)
        return;

    int IndexA = InventoryItems.IndexOfByKey(ItemA);
    int IndexB = InventoryItems.IndexOfByKey(ItemB);

    if (IndexA != INDEX_NONE && IndexB != INDEX_NONE)
    {
        InventoryItems.Swap(IndexA, IndexB);
        OnInventoryChanged.Broadcast();
    }
}

void UInventoryComponent::UpdateTotalEquippedStats(AEnemyCharacterAI* Char)
{
    if (!Char) return;

    FEquippedStatsSummary Stats = CalculateEquippedStats();

    if (Char->HasAuthority())
    {
        Char->SetMaxHealth(Stats.TotalHealth);
    }

    Char->Client_UpdateEquippedStats(Stats);

    CachedEquippedStats = Stats;
    OnStatsChanged.Broadcast(Stats);
}

UItemObject* UInventoryComponent::FindItemByID(FGuid ID) const
{
    //Check inventory items
    for (UItemObject* Item : InventoryItems)
    {
        if (Item && Item->ItemData.UniqueID == ID)
        {
            return Item;
        }
    }

    //Check equipped items
    for (const FEquippedItemEntry& Entry : EquippedItems)
    {
        if (Entry.Item && Entry.Item->ItemData.UniqueID == ID)
        {
            return Entry.Item;
        }
    }

    return nullptr;
}

void UInventoryComponent::RemoveItemByID(FGuid ItemID)
{
    for (int i = 0; i < InventoryItems.Num(); ++i)
    {
        UItemObject* Item = InventoryItems[i];
        if (Item && Item->ItemData.UniqueID == ItemID)
        {
            InventoryItems.RemoveAt(i);
            OnInventoryChanged.Broadcast();
            return; 
        }
    }
}


FEquippedStatsSummary UInventoryComponent::CalculateEquippedStats() const
{
    FEquippedStatsSummary Summary;

    if (const ARelicRunnersCharacter* OwnerChar = Cast<ARelicRunnersCharacter>(GetOwner()))
    {
        Summary.TotalHealth = OwnerChar->GetPlayerStartingMaxHealth();
        Summary.TotalArmor = OwnerChar->GetPlayerStartingArmor();
        Summary.TotalDexterity = OwnerChar->GetPlayerStartingDexterity();
        Summary.TotalStrength = OwnerChar->GetPlayerStartingStrength();
        Summary.TotalIntelligence = OwnerChar->GetPlayerStartingIntelligence();
    }

    for (const FEquippedItemEntry& Entry : EquippedItems)
    {
        if (UItemObject* Item = Entry.Item)
        {
            Summary.TotalHealth += Item->ItemData.Health;
            Summary.TotalArmor += Item->ItemData.Armor;
            Summary.TotalDexterity += Item->ItemData.Dexterity;
            Summary.TotalStrength += Item->ItemData.Strength;
            Summary.TotalIntelligence += Item->ItemData.Intelligence;
        }
    }

    return Summary;
}

void UInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(UInventoryComponent, InventoryItems);
    DOREPLIFETIME(UInventoryComponent, EquippedItems);
    DOREPLIFETIME(UInventoryComponent, CurrentSortingMethod);
}