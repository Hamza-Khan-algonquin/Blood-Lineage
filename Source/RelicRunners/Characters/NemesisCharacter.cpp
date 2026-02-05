#include "NemesisCharacter.h"
//#include "RelicRunners/AbilitySystem/AbilityPointCounter.h"
#include "RelicRunners/PlayerState/RelicRunnersPlayerState.h"
#include <RelicRunners/Item/ItemStats.h>
#include "RelicRunners/Inventory/InventoryComponent.h"
#include "RelicRunners/Inventory/Inventory.h"
#include <RelicRunners/Classes/ClassInfo.h>

ANemesisCharacter::ANemesisCharacter()
{
}

void ANemesisCharacter::BeginPlay()
{
    Super::BeginPlay();

	if (AbilitySystem)
	{
        // Give Divine Punishment Dance ability
        AbilitySystem->GiveAbility(FGameplayAbilitySpec(UDivinePunishment::StaticClass(), 1, 0));

        // Give Impunity ability
        AbilitySystem->GiveAbility(FGameplayAbilitySpec(UImpunityAbility::StaticClass(), 1, 0));

        // Give Vengeful Dance ability
        AbilitySystem->GiveAbility(FGameplayAbilitySpec(UVengefulDance::StaticClass(), 1, 0));

	}

    ////Starting Stats
    //if (ClassInfo)
    //{
    //    FClassData Stats = ClassInfo->GetStats("Nemesis");
    //    SetPlayerStartingHealth(Stats.Health);
    //    PlayerMaxHealth = Stats.Health;
    //    PlayerHealth = Stats.Health;
    //    SetPlayerStartingArmor(Stats.Armor);
    //    SetPlayerStartingStrength(Stats.Strength);
    //    SetPlayerStartingDexterity(Stats.Dexterity);
    //    SetPlayerStartingIntelligence(Stats.Intelligence);
    //}
}

// Called when player takes control of this character
void ANemesisCharacter::PossessedBy(AController* NewController)
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
void ANemesisCharacter::OnRep_PlayerState()
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


void ANemesisCharacter::GiveDamageAbilities()
{
    Super::GiveDamageAbilities();

    if (AbilitySystem)
    {

    }
}

void ANemesisCharacter::GiveDefenceAbilities()
{
    Super::GiveDefenceAbilities();

    if (AbilitySystem)
    {

        // Activate Impunity ability
        AbilitySystem->TryActivateAbilityByClass(UImpunityAbility::StaticClass());

    }
}

void ANemesisCharacter::GiveUtilityAbilities()
{
    Super::GiveUtilityAbilities();

    if (AbilitySystem)
    {
        // Activate Vengeful Dance ability
        AbilitySystem->TryActivateAbilityByClass(UVengefulDance::StaticClass());

    }
}

void ANemesisCharacter::GiveUltimateAbilities()
{
    Super::GiveUltimateAbilities();

    if (AbilitySystem)
    {

        // Activate Vengeful Dance ability
        AbilitySystem->TryActivateAbilityByClass(UDivinePunishment::StaticClass());

    }
}