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

#include "RelicRunnersPlayerState.h"
#include "RelicRunners/Menu/Keybinds.h"
#include "Net/UnrealNetwork.h"
#include "RelicRunners/Inventory/InventoryComponent.h"
#include "RelicRunners/RelicRunnersCharacter.h"
#include <RelicRunners/Menu/LobbyGameMode.h>
#include <RelicRunners/PlayerController/RelicRunnersPlayerController.h>
#include <RelicRunners/Inventory/Inventory.h>
#include "RelicRunners/Inventory/InventorySortingOptions.h"
#include "RelicRunners/Item/ItemData.h"
#include <RelicRunners/Item/ItemStats.h>
#include "RelicRunners/Vendor/Vendor.h"
#include "RelicRunners/AbilitySystem/AbilitySelection.h"
#include "RelicRunners/Item/ItemMeshData.h"
#include "Engine/ActorChannel.h"
#include <Blueprint/WidgetBlueprintLibrary.h>
#include "RelicRunners/Menu/JoinUserWidget.h"

ARelicRunnersPlayerState::ARelicRunnersPlayerState()
{
    AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
    AttributeSet = CreateDefaultSubobject<UAttributeSet>(TEXT("AttributeSet"));

    AbilitySystemComponent->SetIsReplicated(true);
    AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	//Inventory
	InventoryComponent = CreateDefaultSubobject<UInventoryComponent>(TEXT("InventoryComponent"));
	InventoryComponent->SetIsReplicated(true);
	PlayerNumInventorySlots = 20;

	//Health
	PlayerStartingMaxHealth = 100;
	PlayerMaxHealth = PlayerStartingMaxHealth;
	PlayerHealth = PlayerMaxHealth;

	//Resource
	PlayerStartingMaxResource = 100;
	PlayerMaxResource = PlayerStartingMaxResource;
	PlayerResource = PlayerMaxResource;

	//Level
	PlayerLevel = 1;
	PlayerXP = 0;
	PlayerXPToLevel = 100;

	//Stats
	PlayerStartingArmor = 0;
	PlayerStartingDexterity = 0;
	PlayerStartingStrength = 0;
	PlayerStartingIntelligence = 0;
	PlayerArmor = 0;
	PlayerDexterity = 0;
	PlayerStrength = 0;
	PlayerIntelligence = 0;
	PlayerAbilityPoints = 2;

	//Gold
	PlayerStartingGold = 0;
	PlayerGold = PlayerStartingGold;

	//Potions
	HealthPotionCount = 3;
	HealthGranted = 0.5;
}

void ARelicRunnersPlayerState::CopyProperties(APlayerState* PlayerState)
{
	Super::CopyProperties(PlayerState);

	ARelicRunnersPlayerState* PS = Cast<ARelicRunnersPlayerState>(PlayerState);
	if (PS && this->InventoryComponent && PS->InventoryComponent)
	{
		PS->PlayerGold = this->PlayerGold;
		PS->PlayerLevel = this->PlayerLevel;
		PS->PlayerXP = this->PlayerXP;
		PS->PlayerAbilityPoints = this->PlayerAbilityPoints;
		PS->HasReceivedStarterItems = this->HasReceivedStarterItems;
		PS->SelectedClass = this->SelectedClass;

		PS->InventoryComponent->InventoryItems = this->InventoryComponent->InventoryItems;
		for (UItemObject* Item : PS->InventoryComponent->InventoryItems)
		{
			if (Item)
			{
				Item->Rename(nullptr, PS);
			}
		}

		PS->InventoryComponent->EquippedItems = this->InventoryComponent->EquippedItems;
		for (FEquippedItemEntry& Entry : PS->InventoryComponent->EquippedItems)
		{
			if (Entry.Item)
			{
				Entry.Item->Rename(nullptr, PS);
			}
		}

		UE_LOG(LogTemp, Warning, TEXT("CopyProperties: Transferred %d items and %d equipped slots to new PS."),
			PS->InventoryComponent->InventoryItems.Num(),
			PS->InventoryComponent->EquippedItems.Num());
	}

	PS->ForceNetUpdate();
}

bool ARelicRunnersPlayerState::ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags)
{
	bool bWroteSomething = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);

	if (InventoryComponent)
	{
		for (UItemObject* Item : InventoryComponent->InventoryItems)
		{
			if (Item)
			{
				bWroteSomething |= Channel->ReplicateSubobject(Item, *Bunch, *RepFlags);
			}
		}
	}

	return bWroteSomething;
}

ARelicRunnersPlayerState::~ARelicRunnersPlayerState()
{

}

void ARelicRunnersPlayerState::GiveStarterItems()
{
	if (!HasAuthority() || HasReceivedStarterItems) return;

	if (!ItemMeshData)
	{
		UE_LOG(LogTemp, Error, TEXT("GiveStarterItems: ItemMeshData is missing on PlayerState!"));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("SERVER: Generating items using PlayerState MeshData."));

	for (int i = 0; i < 50; i++)
	{
		FString RandomType = ItemStats::GetRandomItemType();
		FItemData ItemData = ItemStats::CreateSpecificItemData(1, RandomType, ItemMeshData);
		UItemObject* Item = ItemStats::CreateItemFromData(ItemData, InventoryComponent);
		if (!Item) continue;

		InventoryComponent->AddItem(Item);
	}

	//Starting Weapon
	FString StartingWeapon = "";
	if (SelectedClass == "Ares")
	{
		StartingWeapon = "Greatsword";
	}
	else if (SelectedClass == "Artemis")
	{
		StartingWeapon = "Bow";
	}
	else if (SelectedClass == "Aphrodite")
	{
		StartingWeapon = "Staff";
	}
	else if (SelectedClass == "Nemesis")
	{
		StartingWeapon = "Sword";
	}
	UItemObject* Item = ItemStats::CreateItemFromData(ItemStats::CreateSpecificItemData(1, StartingWeapon, "Common", ItemMeshData), InventoryComponent);
	InventoryComponent->AddItem(Item);
	Server_EquipItemByID(Item->ItemData.UniqueID);

	HasReceivedStarterItems = true;
}

void ARelicRunnersPlayerState::PassiveHealthRegen()
{
	if (!HasAuthority()) return;

	if (PlayerHealth < PlayerMaxHealth)
	{
		int RegenAmount = FMath::CeilToInt(PlayerMaxHealth * 0.05f);
		PlayerHealth = FMath::Min(PlayerHealth + RegenAmount, PlayerMaxHealth);

		//For server
		UpdateRemoteHUDHealth();
	}
}

void ARelicRunnersPlayerState::Client_UpdateEquippedStats_Implementation(const FEquippedStatsSummary& NewStats)
{
	if (InventoryComponent)
	{
		InventoryComponent->CachedEquippedStats = NewStats;
		InventoryComponent->OnStatsChanged.Broadcast(NewStats);
	}
}

void ARelicRunnersPlayerState::SetPlayerMaxHealth(int value)
{
	if (!HasAuthority()) return;

	PlayerMaxHealth = value;

	//For server
	UpdateRemoteHUDHealth();
}

void ARelicRunnersPlayerState::Server_UseHealthPotion_Implementation()
{
	if (!HasAuthority()) return;

	if (HealthPotionCount > 0 && PlayerHealth < PlayerMaxHealth)
	{
		HealthPotionCount--;
		PlayerHealth = FMath::Clamp(PlayerHealth + (int)HealthGranted, 0, PlayerMaxHealth);

		//For server
		UpdateRemoteHUDHealth();
	}
}

void ARelicRunnersPlayerState::OnLevelUp()
{
	PlayerXP -= PlayerXPToLevel;
	PlayerLevel++;
	PlayerXPToLevel = PlayerLevel * 100;

	//Starting stats / base stats
	PlayerStartingMaxHealth++;
	PlayerStartingMaxResource++;
	PlayerStartingArmor++;
	PlayerStartingDexterity++;
	PlayerStartingStrength++;
	PlayerStartingIntelligence++;

	//Current health / max health
	PlayerMaxHealth++;
	PlayerHealth = PlayerMaxHealth;

	//Current resource / max resource
	PlayerMaxResource++;
	PlayerResource = PlayerMaxResource;

	//Current stats
	PlayerArmor++;
	PlayerDexterity++;
	PlayerStrength++;
	PlayerIntelligence++;
	PlayerNumInventorySlots++;

	//Ability points
	PlayerAbilityPoints++;

	InventoryComponent->UpdateTotalEquippedStats(GetPawn());

	Client_OnLevelUp();
}

void ARelicRunnersPlayerState::Server_AddExperience_Implementation(int Amount)
{
	AddExperience(Amount);
}

void ARelicRunnersPlayerState::AddExperience(int Amount)
{
	if (!HasAuthority())
	{
		Server_AddExperience(Amount);
		return;
	}

	PlayerXP += Amount;

	while (PlayerXP >= PlayerXPToLevel)
	{
		OnLevelUp();
	}

	//For server
	UpdateRemoteHUDAbility();
	UpdateRemoteHUDExperience();
}

void ARelicRunnersPlayerState::Client_OnLevelUp_Implementation()
{
	if (ARelicRunnersPlayerController* PC = Cast<ARelicRunnersPlayerController>(GetPlayerController()))
	{
		PC->HandleLevelUpVisuals();
	}
}

void ARelicRunnersPlayerState::SetSelectedClass(FName NewClass)
{
    if (!HasAuthority()) return;

    SelectedClass = NewClass;
    OnSelectedClassChanged.Broadcast(SelectedClass);
    ForceNetUpdate();

    if (ALobbyGameMode* GM = GetWorld()->GetAuthGameMode<ALobbyGameMode>())
    {
        GM->OnPlayerStateUpdated(this);
    }
}

void ARelicRunnersPlayerState::Client_OnAbilityPointsUpdated_Implementation(int NewAbilityPoints)
{
	ARelicRunnersPlayerController* PC = Cast<ARelicRunnersPlayerController>(GetPlayerController());
	if (!PC) return;

	PC->UpdateHUDAbilityPoints(NewAbilityPoints);

	if (NewAbilityPoints <= 0 && PC->AbilitySelection)
	{
		PC->HideUI(PC->AbilitySelection, false);
	}
}

void ARelicRunnersPlayerState::AddGold(int Value)
{
	Server_AddGold(Value);
}

void ARelicRunnersPlayerState::Server_AddGold_Implementation(int Value)
{
	PlayerGold += Value;
	InventoryComponent->UpdateTotalEquippedStats(GetPawn());
}

bool ARelicRunnersPlayerState::CheckEnoughGold(int Value)
{
	if (PlayerGold >= Value)
	{
		PlayerGold -= Value;
		InventoryComponent->UpdateTotalEquippedStats(GetPawn());
		return true;
	}
	return false;
}

void ARelicRunnersPlayerState::Server_SpendAbilityPoints_Implementation()
{
	if (!HasAuthority()) return;

	if (PlayerAbilityPoints > 0)
	{
		PlayerAbilityPoints--;

		Client_OnAbilityPointsUpdated(PlayerAbilityPoints);
	}

	//For server
	UpdateRemoteHUDAbility();
}

void ARelicRunnersPlayerState::Server_BuyItem_Implementation(const FItemData& ItemData, AVendor* Vendor)
{
	if (!Vendor || !InventoryComponent) return;

	if (!CheckEnoughGold(ItemData.Gold)) return;

	UItemObject* NewItem = ItemStats::CreateItemFromData(ItemData, InventoryComponent);
	NewItem->IsNewItem = true;
	InventoryComponent->AddItem(NewItem);
	Vendor->RemoveStock(ItemData);

	if (ARelicRunnersPlayerController* RPC = Cast<ARelicRunnersPlayerController>(GetPlayerController()))
	{
		RPC->Client_UpdateVendorUI(Vendor);
	}
}

void ARelicRunnersPlayerState::Server_SellItem_Implementation(const FItemData& ItemData, AVendor* Vendor)
{
	if (!Vendor || !InventoryComponent) return;

	ARelicRunnersPlayerController* RPC = Cast<ARelicRunnersPlayerController>(GetPlayerController());

	//Unequip if equipped
	UItemObject* EquippedItem = InventoryComponent->GetEquippedItemReference(ItemData.ItemType);
	if (EquippedItem && EquippedItem->ItemData.UniqueID == ItemData.UniqueID)
	{
		InventoryComponent->UnequipItem(EquippedItem, RPC);
	}

	InventoryComponent->RemoveItemByID(ItemData.UniqueID);
	AddGold(ItemData.Gold);
	Vendor->AddStock(ItemData);

	RPC->Client_UpdateVendorUI(Vendor);
}

EInventorySorting ARelicRunnersPlayerState::GetCurrentSortingMethod() const
{
	return InventoryComponent ? InventoryComponent->CurrentSortingMethod : EInventorySorting::SortByNewest;
}

void ARelicRunnersPlayerState::Server_EquipItemByID_Implementation(FGuid ID)
{
	if (!InventoryComponent) return;

	UItemObject* Item = InventoryComponent->FindItemByID(ID);
	if (Item)
	{
		if (ARelicRunnersCharacter* Char = Cast<ARelicRunnersCharacter>(GetPawn()))
		{
			Char->UpdateItemVisuals(Item);
			InventoryComponent->EquipItem(Item, Cast<ARelicRunnersPlayerController>(Char->GetController()));
			InventoryComponent->UpdateTotalEquippedStats(GetPawn());
		}
	}
}

void ARelicRunnersPlayerState::Server_UnequipItemByID_Implementation(FGuid ID)
{
	if (!InventoryComponent) return;

	UItemObject* EquippedItem = nullptr;
	FString ItemType;

	for (FEquippedItemEntry& Entry : InventoryComponent->EquippedItems)
	{
		if (Entry.Item && Entry.Item->ItemData.UniqueID == ID)
		{
			EquippedItem = Entry.Item;
			ItemType = Entry.ItemType;
			break;
		}
	}

	if (EquippedItem)
	{
		if (ARelicRunnersCharacter* Char = Cast<ARelicRunnersCharacter>(GetPawn()))
		{
			Char->UpdateItemVisuals(nullptr, ItemType);
			InventoryComponent->UnequipItem(EquippedItem, Cast<ARelicRunnersPlayerController>(Char->GetController()));
			InventoryComponent->UpdateTotalEquippedStats(GetPawn());
		}
	}

	InventoryComponent->SortInventoryByCurrentMethod();
}

void ARelicRunnersPlayerState::OnRep_SelectedClass()
{
    OnSelectedClassChanged.Broadcast(SelectedClass);
}

void ARelicRunnersPlayerState::OnRep_PlayerGold()
{
	if (ARelicRunnersCharacter* Char = Cast<ARelicRunnersCharacter>(GetPawn()))
	{
		InventoryComponent->UpdateTotalEquippedStats(Char);
	}
}

void ARelicRunnersPlayerState::OnRep_IsHost()
{
	OnHostStatusChanged.Broadcast(IsHost);
}

void ARelicRunnersPlayerState::SetIsHost(bool NewIsHost)
{
	if (HasAuthority())
	{
		IsHost = NewIsHost;
		OnHostStatusChanged.Broadcast(IsHost);
	}
}

void ARelicRunnersPlayerState::OnRep_PlayerLevel(int OldLevel)
{
	if (PlayerLevel > OldLevel)
	{
		//Show UI Popup
		Client_OnLevelUp();
	}
}

void ARelicRunnersPlayerState::ModifyHealth(float Value)
{
	if (!HasAuthority()) return;

	PlayerHealth = FMath::Clamp(PlayerHealth + Value, 0.f, (float)PlayerMaxHealth);

	if (PlayerHealth <= 0)
	{
		ARelicRunnersCharacter* Char = Cast<ARelicRunnersCharacter>(GetPawn());
		if (Char)
		{
			Char->Die();
		}
	}

	//For server
	UpdateRemoteHUDHealth();
}

void ARelicRunnersPlayerState::UpdateRemoteHUDHealth()
{
	if (ARelicRunnersPlayerController* PC = Cast<ARelicRunnersPlayerController>(GetPlayerController()))
	{
		PC->UpdateHUDHealth(PlayerHealth, PlayerMaxHealth);
	}
}

void ARelicRunnersPlayerState::UpdateRemoteHUDResource()
{
	if (ARelicRunnersPlayerController* PC = Cast<ARelicRunnersPlayerController>(GetPlayerController()))
	{
		PC->UpdateHUDResource(PlayerResource, PlayerMaxResource);
	}
}

void ARelicRunnersPlayerState::UpdateRemoteHUDExperience()
{
	if (ARelicRunnersPlayerController* PC = Cast<ARelicRunnersPlayerController>(GetPlayerController()))
	{
		PC->UpdateHUDXP(PlayerLevel, PlayerXP, PlayerXPToLevel);
	}
}

void ARelicRunnersPlayerState::UpdateRemoteHUDAbility()
{
	if (ARelicRunnersPlayerController* PC = Cast<ARelicRunnersPlayerController>(GetPlayerController()))
	{
		PC->UpdateHUDAbilityPoints(PlayerAbilityPoints);
	}
}

void ARelicRunnersPlayerState::OnRep_Health()
{
	UpdateRemoteHUDHealth();
}

void ARelicRunnersPlayerState::OnRep_Resource()
{
	UpdateRemoteHUDResource();
}

void ARelicRunnersPlayerState::OnRep_Experience()
{
	UpdateRemoteHUDExperience();
}

void ARelicRunnersPlayerState::OnRep_Ability()
{
	UpdateRemoteHUDAbility();
}

void ARelicRunnersPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(ARelicRunnersPlayerState, SelectedClass);
	DOREPLIFETIME(ARelicRunnersPlayerState, PlayerHealth);
	DOREPLIFETIME(ARelicRunnersPlayerState, PlayerMaxHealth);
	DOREPLIFETIME(ARelicRunnersPlayerState, PlayerResource);
	DOREPLIFETIME(ARelicRunnersPlayerState, PlayerMaxResource);
	DOREPLIFETIME(ARelicRunnersPlayerState, PlayerLevel);
	DOREPLIFETIME(ARelicRunnersPlayerState, PlayerXP);
	DOREPLIFETIME(ARelicRunnersPlayerState, PlayerXPToLevel);
	DOREPLIFETIME(ARelicRunnersPlayerState, PlayerAbilityPoints);
	DOREPLIFETIME(ARelicRunnersPlayerState, PlayerGold);
	DOREPLIFETIME(ARelicRunnersPlayerState, HasReceivedStarterItems);
	DOREPLIFETIME(ARelicRunnersPlayerState, IsHost);
}

