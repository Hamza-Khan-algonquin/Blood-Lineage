// Fill out your copyright notice in the Description page of Project Settings.


#include "AresCharacter.h"
#include "RelicRunners/PlayerState/RelicRunnersPlayerState.h"
#include <RelicRunners/Item/ItemStats.h>
#include "RelicRunners/Inventory/InventoryComponent.h"
#include "RelicRunners/Inventory/Inventory.h"
#include <RelicRunners/Classes/ClassInfo.h>

AAresCharacter::AAresCharacter()
{
}

void AAresCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (AbilitySystem)
	{
		// Give Earthquake ability
		AbilitySystem->GiveAbility(FGameplayAbilitySpec(UEarthquakeAbility::StaticClass(), 1, 0));

		// Give War Banner ability
		AbilitySystem->GiveAbility(FGameplayAbilitySpec(UWarBannerAbility::StaticClass(), 1, 0));
	}

}

// Called when player takes control of this character
void AAresCharacter::PossessedBy(AController* NewController)
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
void AAresCharacter::OnRep_PlayerState()
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

void AAresCharacter::GiveDamageAbilities()
{
	Super::GiveDamageAbilities();

	if (AbilitySystem)
	{
		// Activate Earthquake ability 
		AbilitySystem->TryActivateAbilityByClass(UEarthquakeAbility::StaticClass());

	}
}

void AAresCharacter::GiveDefenceAbilities()
{
	Super::GiveDefenceAbilities();

	if (AbilitySystem)
	{
		// start ability cooldown
		//AbilityPointCounter->StartDefenceCooldown(DefenceCooldown);
	}
}

void AAresCharacter::GiveUtilityAbilities()
{
	Super::GiveUtilityAbilities();

	if (AbilitySystem)
	{
		//start ability cooldown
		AbilitySystem->TryActivateAbilityByClass(UWarBannerAbility::StaticClass());
	}
}

void AAresCharacter::GiveUltimateAbilities()
{
	Super::GiveUltimateAbilities();

	if (AbilitySystem)
	{
		// start ability cooldown
		//AbilityPointCounter->StartUltimateCooldown(UltimateCooldown);
	}
}