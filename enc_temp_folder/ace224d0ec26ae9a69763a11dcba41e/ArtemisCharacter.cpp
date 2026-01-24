// Fill out your copyright notice in the Description page of Project Settings.


#include "ArtemisCharacter.h"
//#include "RelicRunners/AbilitySystem/AbilityPointCounter.h"
#include "RelicRunners/PlayerState/RelicRunnersPlayerState.h"
#include <RelicRunners/Item/ItemStats.h>
#include "RelicRunners/Inventory/InventoryComponent.h"
#include "RelicRunners/Inventory/Inventory.h"
#include <RelicRunners/Classes/ClassInfo.h>

AArtemisCharacter::AArtemisCharacter()
{
}

void AArtemisCharacter::BeginPlay()
{
	Super::BeginPlay();

	//DamageCooldown = 5.0f;
	//DefenceCooldown = 5.0f;
	//UtilityCooldown = 5.0f;
	//UltimateCooldown = 10.0f;

	//Starting Weapon
	//UItemObject* Item = ItemStats::CreateItemFromData(ItemStats::CreateSpecificItemData(1, "Bow", "Common", ItemMeshData), InventoryComponent);
	//InventoryComponent->AddItem(Item);
	//Server_EquipItemByID(Item->ItemData.UniqueID);

	//Starting Stats
	if (ClassInfo)
	{
		//FClassData Stats = ClassInfo->GetStats("Artemis");
		//SetPlayerStartingHealth(Stats.Health);
		//PlayerMaxHealth = Stats.Health;
		//PlayerHealth = Stats.Health;
		//SetPlayerStartingArmor(Stats.Armor);
		//SetPlayerStartingStrength(Stats.Strength);
		//SetPlayerStartingDexterity(Stats.Dexterity);
		//SetPlayerStartingIntelligence(Stats.Intelligence);
	}
}

// Called when player takes control of this character
void AArtemisCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	// Get PlayerState and set up ability system
	ARelicRunnersPlayerState* PS = GetPlayerState<ARelicRunnersPlayerState>();
	if (PS)
	{
		AbilitySystem = PS->GetAbilitySystemComponent();
		if (AbilitySystem)
		{
			AbilitySystem->InitAbilityActorInfo(PS, this);
		}
	}
}

// Called when PlayerState is replicated on clients
void AArtemisCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	// Reinitialize ability system
	ARelicRunnersPlayerState* PS = GetPlayerState<ARelicRunnersPlayerState>();
	if (PS)
	{
		AbilitySystem = PS->GetAbilitySystemComponent();
		AbilitySystem->InitAbilityActorInfo(PS, this);
	}
}
void AArtemisCharacter::GiveDamageAbilities()
{
	Super::GiveDamageAbilities();

	if (AbilitySystem)
	{
		// Start ability cooldown
		//AbilityPointCounter->StartDamageCooldown(DamageCooldown);
	}
}

void AArtemisCharacter::GiveDefenceAbilities()
{
	Super::GiveDefenceAbilities();

	if (AbilitySystem)
	{
		// Start ability cooldown
		//AbilityPointCounter->StartDefenceCooldown(DefenceCooldown);
	}
}

void AArtemisCharacter::GiveUtilityAbilities()
{
	Super::GiveUtilityAbilities();

	if (AbilitySystem)
	{
		// Start ability cooldown
		//AbilityPointCounter->StartUtilityCooldown(UtilityCooldown);
	}
}

void AArtemisCharacter::GiveUltimateAbilities()
{
	Super::GiveUltimateAbilities();

	if (AbilitySystem)
	{
		// Give Moonbeam ability
		AbilitySystem->GiveAbility(FGameplayAbilitySpec(UMoonbeam::StaticClass(), 1, 0));

		// Start ability cooldown
		//AbilityPointCounter->StartUltimateCooldown(UltimateCooldown);
	}

}

void AArtemisCharacter::OnLevelUp()
{
	//UItemObject*& Item = InventoryComponent->GetEquippedItemReference("Bow");
	//Item->ItemData.Level++;
	//Item->ItemData.Strength += FMath::RandRange(1, 3);
	//Item->ItemData.Intelligence += FMath::RandRange(0, 1);
	//Item->ItemData.Dexterity += FMath::RandRange(1, 5);

	//Super::OnLevelUp();
}