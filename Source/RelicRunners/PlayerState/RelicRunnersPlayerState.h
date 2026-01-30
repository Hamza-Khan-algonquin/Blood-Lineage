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
#include "GameFramework/PlayerState.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"
#include "AttributeSet.h"
#include "RelicRunnersPlayerState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHostStatusChanged, bool, NewIsHost);

enum class EInventorySorting : uint8;

UCLASS()
class RELICRUNNERS_API ARelicRunnersPlayerState : public APlayerState, public IAbilitySystemInterface
{
    GENERATED_BODY()
public:

    //Functions
    ARelicRunnersPlayerState();
    ~ARelicRunnersPlayerState();
	void GiveStarterItems();
    UFUNCTION()
    void OnRep_SelectedClass();
	UFUNCTION()
	void AddGold(int Value);
	UFUNCTION(Server, Reliable)
	void Server_AddGold(int Value);
	UFUNCTION()
	bool CheckEnoughGold(int Value);
	UFUNCTION(Server, Reliable)
	void Server_BuyItem(const FItemData& ItemData, class AVendor* Vendor);
	UFUNCTION(Server, Reliable)
	void Server_SellItem(const FItemData& ItemData, class AVendor* Vendor);
	UFUNCTION(Server, Reliable)
	void Server_SpendAbilityPoints();
	UFUNCTION(Client, Reliable)
	void Client_UpdateEquippedStats(const FEquippedStatsSummary& NewStats);
	UFUNCTION(Client, Reliable)
	void Client_OnAbilityPointsUpdated(int NewAbilityPoints);
	UFUNCTION(Server, Reliable)
	void Server_AddExperience(int Amount);
	UFUNCTION(Server, Reliable)
	void Server_EquipItemByID(FGuid ID);
	UFUNCTION(Server, Reliable)
	void Server_UnequipItemByID(FGuid ID);
	UFUNCTION(Server, Reliable)
	void Server_UseHealthPotion();
	UFUNCTION(Client, Reliable)
	void Client_OnLevelUp();
	UFUNCTION()
	void PassiveHealthRegen();
	UFUNCTION()
	void PassiveResourceRegen();
	virtual void OnLevelUp();
	void AddExperience(int Amount);

    //Properties
    UPROPERTY(ReplicatedUsing = OnRep_SelectedClass)
    FName SelectedClass;
	UPROPERTY()
	UAttributeSet* Attributes;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	UAbilitySystemComponent* AbilitySystem;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	class UItemMeshData* ItemMeshData;
	UPROPERTY(ReplicatedUsing = OnRep_IsHost, BlueprintReadOnly, Category = "Lobby")
	bool IsHost = false;
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Stats")
	bool HasReceivedStarterItems = false;

	//Stats
	void UpdateRemoteHUDHealth();
	void UpdateRemoteHUDResource();
	void UpdateRemoteHUDExperience();
	void UpdateRemoteHUDAbility();
	UFUNCTION()
	void OnRep_Health();
	UFUNCTION()
	void OnRep_Resource();
	UFUNCTION()
	void OnRep_Experience();
	UFUNCTION()
	void OnRep_Ability();
	UFUNCTION()
	void OnRep_PlayerGold();
	UFUNCTION()
	void OnRep_IsHost();
	void SetIsHost(bool NewIsHost);
	UFUNCTION()
	void OnRep_PlayerLevel(int OldLevel);
	void ModifyHealth(float Value);
	UPROPERTY()
	int PlayerNumInventorySlots;
	UPROPERTY(ReplicatedUsing = OnRep_Health)
	int PlayerHealth;
	UPROPERTY(ReplicatedUsing = OnRep_Health)
	int PlayerMaxHealth;
	UPROPERTY(ReplicatedUsing = OnRep_Resource)
	int PlayerResource;
	UPROPERTY(ReplicatedUsing = OnRep_Resource)
	int PlayerMaxResource;
	UPROPERTY(ReplicatedUsing = OnRep_Experience)
	int PlayerXP;
	UPROPERTY(ReplicatedUsing = OnRep_Experience)
	int PlayerXPToLevel;
	UPROPERTY(ReplicatedUsing = OnRep_PlayerLevel)
	int PlayerLevel;
	UPROPERTY(ReplicatedUsing = OnRep_Ability)
	int PlayerAbilityPoints;
	UPROPERTY(ReplicatedUsing = OnRep_PlayerGold, BlueprintReadOnly, Category = "Player")
	int PlayerGold;
	UPROPERTY()
	int PlayerArmor;
	UPROPERTY()
	int PlayerDexterity;
	UPROPERTY()
	int PlayerStrength;
	UPROPERTY()
	int PlayerIntelligence;
	UPROPERTY()
	int PlayerStartingMaxHealth;
	UPROPERTY()
	int PlayerStartingMaxResource;
	UPROPERTY()
	int PlayerStartingArmor;
	UPROPERTY()
	int PlayerStartingDexterity;
	UPROPERTY()
	int PlayerStartingStrength;
	UPROPERTY()
	int PlayerStartingIntelligence;
	UPROPERTY()
	int PlayerStartingGold;
	UPROPERTY()
	int HealthPotionCount;
	UPROPERTY()
	float HealthGranted;

    //Getters
	EInventorySorting GetCurrentSortingMethod() const;
    UAttributeSet* GetAttributeSet() const { return AttributeSet; }
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override { return AbilitySystemComponent; }
	class UInventoryComponent* GetInventoryComponent() { return InventoryComponent; };
	UFUNCTION()
	int GetPlayerStartingMaxHealth() const { return PlayerStartingMaxHealth; }
	UFUNCTION()
	int GetPlayerNumInventorySlots() const { return PlayerNumInventorySlots; }
	UFUNCTION()
	int GetPlayerStartingArmor() const { return PlayerStartingArmor; }
	UFUNCTION()
	int GetPlayerStartingDexterity() const { return PlayerStartingDexterity; }
	UFUNCTION()
	int GetPlayerStartingStrength() const { return PlayerStartingStrength; }
	UFUNCTION()
	int GetPlayerStartingIntelligence() const { return PlayerStartingIntelligence; }
	UFUNCTION()
	int GetPlayerGold() const { return PlayerGold; }
	int GetPlayerLevel() { return PlayerLevel; };
	int GetPlayerHealth() { return PlayerHealth; }
	int GetPlayerMaxHealth() { return PlayerMaxHealth; }
	int GetPlayerResource() { return PlayerResource; }
	int GetPlayerMaxResource() { return PlayerMaxResource; }
	int GetPlayerXP() { return PlayerXP; }
	int GetPlayerXPToLevel() { return PlayerXPToLevel; }
	int GetPlayerAbilityPoints() { return PlayerAbilityPoints; }
	bool IsSessionHost() const { return IsHost; }

    //Setters
    void SetSelectedClass(FName NewClass);
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void SetInventoryComponent(UInventoryComponent* InvComp) { InventoryComponent = InvComp; }
	UFUNCTION(BlueprintCallable, Category = "Stats")
	void SetPlayerStartingArmor(int StartingArmor) { PlayerStartingArmor = StartingArmor; }
	UFUNCTION(BlueprintCallable, Category = "Stats")
	void SetPlayerStartingDexterity(int StartingDexterity) { PlayerStartingDexterity = StartingDexterity; }
	UFUNCTION(BlueprintCallable, Category = "Stats")
	void SetPlayerStartingIntelligence(int StartingIntelligence) { PlayerStartingIntelligence = StartingIntelligence; }
	UFUNCTION(BlueprintCallable, Category = "Stats")
	void SetPlayerStartingHealth(int StartingHealth) { PlayerStartingMaxHealth = StartingHealth; }
	UFUNCTION(BlueprintCallable, Category = "Stats")
	void SetPlayerStartingStrength(int StartingStrength) { PlayerStartingStrength = StartingStrength; }
	void SetPlayerResource(int Resource) { PlayerResource = Resource; }
	void SetPlayerMaxHealth(int value);
	void ResetHealth() { PlayerHealth = PlayerMaxHealth; }

    //Delegates
	FOnHostStatusChanged OnHostStatusChanged;
    DECLARE_MULTICAST_DELEGATE_OneParam(FOnSelectedClassChanged, FName);
    FOnSelectedClassChanged OnSelectedClassChanged;

protected:


	FTimerHandle RegenTimer;

	//Virtual Functions
	virtual void BeginPlay() override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void CopyProperties(APlayerState* PlayerState) override;
	virtual bool ReplicateSubobjects(class UActorChannel* Channel, class FOutBunch* Bunch, struct FReplicationFlags* RepFlags) override;

    //Properties
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
    TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
    TObjectPtr<UAttributeSet> AttributeSet;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
    class UInventoryComponent* InventoryComponent;
};

