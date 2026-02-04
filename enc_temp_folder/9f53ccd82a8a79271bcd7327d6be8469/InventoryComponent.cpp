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
#include "RelicRunners/Enemy/Characters/EnemyCharacterAI.h"
#include "Engine/ActorChannel.h"
#include "RelicRunners/Item/ItemStats.h"
#include <RelicRunners/PlayerController/RelicRunnersPlayerController.h>
#include "RelicRunners/PlayerState/RelicRunnersPlayerState.h"

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
        //Client requests the server to sort
        Server_SortInventory(NewMethod);
        return;
    }

    //Server sorts immediately
    SortInventoryByCurrentMethod();
}

void UInventoryComponent::SortInventoryByCurrentMethod()
{
    if (!GetOwner() || !GetOwner()->HasAuthority()) return;

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
    {
        int Rank = 0;
        for (const auto& Elem : RarityMap)
        {
            RarityRank.Add(Elem.Key, Rank++);
        }
    }

    auto CompareBase = [&](const UItemObject& A, const UItemObject& B)
        {
            //Rarity first
            const int ARank = RarityRank.FindRef(A.ItemData.Rarity);
            const int BRank = RarityRank.FindRef(B.ItemData.Rarity);
            if (ARank != BRank) return ARank > BRank;

            //Level second
            if (A.ItemData.Level != B.ItemData.Level)
                return A.ItemData.Level > B.ItemData.Level;

            //ItemType third
            return A.ItemData.ItemType < B.ItemData.ItemType;
        };

    switch (CurrentSortingMethod)
    {
    case EInventorySorting::SortByItemType:
        Items.Sort(
            [&](const UItemObject& A, const UItemObject& B)
            {
                //Itemtype first
                if (A.ItemData.ItemType != B.ItemData.ItemType)
                    return A.ItemData.ItemType < B.ItemData.ItemType;

                return CompareBase(A, B);
            });
        break;

    case EInventorySorting::SortByRarity:
        Items.Sort(
            [&](const UItemObject& A, const UItemObject& B)
            {
                return CompareBase(A, B);
            });
        break;

    case EInventorySorting::SortByNewest:
        Items.Sort(
            [&](const UItemObject& A, const UItemObject& B)
            {
                //New items first
                if (A.IsNewItem != B.IsNewItem)
                    return A.IsNewItem > B.IsNewItem;

                return CompareBase(A, B);
            });
        break;

    case EInventorySorting::SortByLevel:
        Items.Sort(
            [&](const UItemObject& A, const UItemObject& B)
            {
                //Level first
                if (A.ItemData.Level != B.ItemData.Level)
                    return A.ItemData.Level > B.ItemData.Level;

                return CompareBase(A, B);
            });
        break;
    }

    OnInventoryChanged.Broadcast();
}

void UInventoryComponent::EquipItem(UItemObject* Item, ARelicRunnersPlayerController* RPC)
{
    if (!Item) return;

    const FString Slot = Item->ItemData.ItemType;
    bool Found = false;
    
    for (FEquippedItemEntry& Entry : EquippedItems)
    {
        if (Entry.ItemType == Slot)
        {
            if (Entry.Item)
            {
                AddItem(Entry.Item);
            }
            Entry.Item = Item;
            Found = true;
            break;
        }
    }

    if (!Found)
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

    OnEquipmentChanged.Broadcast();
    OnInventoryChanged.Broadcast();

    if (RPC) RPC->UpdatePreviewWithEquippedItems();
}

void UInventoryComponent::UnequipItem(UItemObject* Item, ARelicRunnersPlayerController* RPC)
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

            if (RPC) RPC->UpdatePreviewWithEquippedItems();
            return;
        }
    }
}

void UInventoryComponent::OnRep_Inventory()
{
    OnInventoryChanged.Broadcast();
}

void UInventoryComponent::OnRep_EquippedItems()
{
    if (APawn* OwningPawn = Cast<APawn>(GetOwner()))
    {
        if (ARelicRunnersCharacter* Char = Cast<ARelicRunnersCharacter>(OwningPawn))
        {
            Char->SyncEquippedVisuals(EquippedItems);
        }
    }

    OnEquipmentChanged.Broadcast();
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

void UInventoryComponent::UpdateTotalEquippedStats(APawn* PawnOwner)
{
    if (!PawnOwner) return;

    CachedEquippedStats = CalculateEquippedStats();

    //Player
    if (ARelicRunnersPlayerState* PS = PawnOwner->GetPlayerState<ARelicRunnersPlayerState>())
    {
        if (PS->HasAuthority())
        {
            PS->SetPlayerMaxHealth(CachedEquippedStats.TotalHealth);
            PS->Client_UpdateEquippedStats(CachedEquippedStats);
        }
    }
    
    //Enemies
    else if (AEnemyCharacterAI* Enemy = Cast<AEnemyCharacterAI>(PawnOwner))
    {
        if (Enemy->HasAuthority())
        {
            Enemy->SetMaxHealth(CachedEquippedStats.TotalHealth);
            Enemy->Client_UpdateEquippedStats(CachedEquippedStats);
        }
    }

    OnStatsChanged.Broadcast(CachedEquippedStats);
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

    if (const ARelicRunnersPlayerState* PS = Cast<ARelicRunnersPlayerState>(GetOwner()))
    {
        Summary.TotalHealth = PS->GetPlayerStartingMaxHealth();
        Summary.TotalArmor = PS->GetPlayerStartingArmor();
        Summary.TotalDexterity = PS->GetPlayerStartingDexterity();
        Summary.TotalStrength = PS->GetPlayerStartingStrength();
        Summary.TotalIntelligence = PS->GetPlayerStartingIntelligence();
        Summary.TotalLuck = PS->GetPlayerStartingLuck();
        Summary.TotalResourceRegen = PS->GetPlayerStartingResourceRegen();
        Summary.TotalPotionEfficiency = PS->GetPlayerStartingPotionEfficiency();
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