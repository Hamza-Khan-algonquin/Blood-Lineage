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
#include "Components/ActorComponent.h"
#include "InventoryComponent.generated.h"

enum class EInventorySorting : uint8;

class UItemObject;

USTRUCT(BlueprintType)
struct FEquippedStatsSummary
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly) 
    int TotalHealth = 0;
    UPROPERTY(BlueprintReadOnly) 
    int TotalArmor = 0;
    UPROPERTY(BlueprintReadOnly) 
    int TotalDexterity = 0;
    UPROPERTY(BlueprintReadOnly) 
    int TotalStrength = 0;
    UPROPERTY(BlueprintReadOnly) 
    int TotalIntelligence = 0;
    UPROPERTY(BlueprintReadOnly)
    int TotalLuck = 0;
    UPROPERTY(BlueprintReadOnly)
    float TotalResourceRegen = 0.0f;
    UPROPERTY(BlueprintReadOnly)
    float TotalPotionEfficiency = 0.0f;
};

USTRUCT(BlueprintType)
struct FEquippedItemEntry
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    FString ItemType;
    UPROPERTY(BlueprintReadWrite)
    UItemObject* Item = nullptr;
    FEquippedItemEntry() {}
    FEquippedItemEntry(const FString& InItemType, UItemObject* InItem) : ItemType(InItemType), Item(InItem) {}
};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class RELICRUNNERS_API UInventoryComponent : public UActorComponent
{
    GENERATED_BODY()

public:

    UInventoryComponent();

    //Properties
    UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Inventory, Category="Inventory", meta=(AllowPrivateAccess = "true"))
    TArray<UItemObject*> InventoryItems;
    UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_EquippedItems, Category = "Inventory", meta = (AllowPrivateAccess = "true"))
    TArray<FEquippedItemEntry> EquippedItems;
    UPROPERTY(ReplicatedUsing = OnRep_SortingMethod)
    EInventorySorting CurrentSortingMethod;
    UPROPERTY(ReplicatedUsing = OnRep_ForgeItems)
    class UItemObject* ForgeItemLeft = nullptr;
    UPROPERTY(ReplicatedUsing = OnRep_ForgeItems)
    class UItemObject* ForgeItemRight = nullptr;
    UPROPERTY(ReplicatedUsing = OnRep_ForgeItems)
    class UItemObject* ForgeItemResult = nullptr;
    UPROPERTY()
    class UInventory* LinkedInventory = nullptr;
    UPROPERTY()
    class UItemMeshData* ItemMeshData;

    //Functions
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void RemoveItemByID(FGuid ItemID);
    UFUNCTION(BlueprintCallable)
    void AddItem(UItemObject* Item);
    void SortInventoryByCurrentMethod();
    UFUNCTION(BlueprintCallable)
    void SwapItems(UItemObject* ItemA, UItemObject* ItemB);
    UFUNCTION(Server, Reliable)
    void Server_SortInventory(EInventorySorting NewMethod);
    void SetSortingMethod(EInventorySorting NewMethod);
    void HandleSortingNow();
    UFUNCTION(Server, Reliable)
    void Server_ClearAllForgeSlots();
    UFUNCTION(BlueprintCallable)
    void EquipItem(class UItemObject* Item, class ARelicRunnersPlayerController* RPC);
    UFUNCTION(BlueprintCallable)
    void UnequipItem(class UItemObject* Item, class ARelicRunnersPlayerController* RPC);
    UFUNCTION()
    void OnRep_Inventory();
    UFUNCTION()
    void OnRep_EquippedItems();
    void UpdateTotalEquippedStats(APawn* PawnOwner);
    UFUNCTION()
    UItemObject* FindItemByID(FGuid ItemID) const;
    FEquippedStatsSummary CalculateEquippedStats() const;
    FEquippedStatsSummary CachedEquippedStats;
    UFUNCTION()
    void OnRep_SortingMethod();
    UFUNCTION()
    void OnRep_ForgeItems();
    UFUNCTION(Server, Reliable)
    void Server_RemoveForgeItem(FGuid ItemID);
    UFUNCTION(Server, Reliable)
    void Server_AddToForge(FGuid ItemID);
    UFUNCTION(Server, Reliable)
    void Server_FuseItems();

    //Getters and Setters
    TArray<FEquippedItemEntry> GetEquippedItems() { return EquippedItems; };
    UFUNCTION(BlueprintCallable)
    UItemObject* GetEquippedItemByType(const FString& ItemType) const;
    UFUNCTION(BlueprintCallable)
    UItemObject*& GetEquippedItemReference(const FString& ItemType);
    FEquippedStatsSummary GetCachedEquippedStats() { return CachedEquippedStats; }
    EInventorySorting GetCurrentSortingMethod() { return CurrentSortingMethod; }
    void SetInventoryWidget(class UInventory* InventoryWidget) { LinkedInventory = InventoryWidget; }

    // Delegates
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStatsChanged, const FEquippedStatsSummary&, NewStats);
    UPROPERTY(BlueprintAssignable)
    FOnStatsChanged OnStatsChanged;
    DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryChanged);
    UPROPERTY(BlueprintAssignable)
    FOnInventoryChanged OnInventoryChanged;
    DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnEquipmentChanged);
    UPROPERTY(BlueprintAssignable)
    FOnEquipmentChanged OnEquipmentChanged;

protected:
    virtual void BeginPlay() override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    virtual bool ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags) override;
    
};