// Fill out your copyright notice in the Description page of Project Settings.


#include "AphroditeCharacter.h"
#include "RelicRunners/PlayerState/RelicRunnersPlayerState.h"
#include <RelicRunners/Item/ItemStats.h>
#include "RelicRunners/Inventory/InventoryComponent.h"
#include "RelicRunners/Inventory/Inventory.h"
#include <RelicRunners/Classes/ClassInfo.h>
#include "RelicRunners/Abilities/KissOfLife.h" 

AAphroditeCharacter::AAphroditeCharacter()
{
}

void AAphroditeCharacter::BeginPlay()
{
    Super::BeginPlay();

    if (AbilitySystem)
    {
        // Give KissOfLife ability
        AbilitySystem->GiveAbility(FGameplayAbilitySpec(UKissOfLife::StaticClass(), 1, 0));
    }

 //   //Starting Stats
 //   if (ClassInfo)
 //   {
 //       FClassData Stats = ClassInfo->GetStats("Aphrodite");
 //       SetPlayerStartingHealth(Stats.Health);
 //       PlayerMaxHealth = Stats.Health;
 //       PlayerHealth = PlayerMaxHealth;
 //       SetPlayerStartingArmor(Stats.Armor);
 //       SetPlayerStartingStrength(Stats.Strength);
 //       SetPlayerStartingDexterity(Stats.Dexterity);
 //       SetPlayerStartingIntelligence(Stats.Intelligence);
 //   }
}


// Called when player takes control of this character
void AAphroditeCharacter::PossessedBy(AController* NewController)
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
void AAphroditeCharacter::OnRep_PlayerState()
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

void AAphroditeCharacter::GiveDamageAbilities()
{
    Super::GiveDamageAbilities();

    if (AbilitySystem)
    {
		// Give Bundle of Joy ability 
        AbilitySystem->GiveAbility(FGameplayAbilitySpec(UBundleOfJoy::StaticClass(), 1, 0));
		// Activate Bundle of Joy ability
        AbilitySystem->TryActivateAbilityByClass(UBundleOfJoy::StaticClass());
		// start ability cooldown
        //AbilityPointCounter->StartDamageCooldown(DamageCooldown);
    }
}
void AAphroditeCharacter::GiveDefenceAbilities()
{
	Super::GiveDefenceAbilities();

    if (AbilitySystem)
    {
		// start ability cooldown
        //AbilityPointCounter->StartDefenceCooldown(DefenceCooldown);
    }
}

void AAphroditeCharacter::GiveUtilityAbilities()
{
    Super::GiveUtilityAbilities();

    if (AbilitySystem)
    {
        // start ability cooldown
        //AbilityPointCounter->StartUtilityCooldown(UtilityCooldown);
        
        // Activate KissOfLife ability 
        AbilitySystem->TryActivateAbilityByClass(UKissOfLife::StaticClass());
    }
}

void AAphroditeCharacter::GiveUltimateAbilities()
{
	Super::GiveUltimateAbilities();

    if (AbilitySystem)
    {
		// start ability cooldown
        //AbilityPointCounter->StartUltimateCooldown(UltimateCooldown);
    }
}


