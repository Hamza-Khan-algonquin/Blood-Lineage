#include "NemesisCharacter.h"
#include "RelicRunners/AbilitySystem/AbilityPointCounter.h"
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

	// set cooldown durations
    DamageCooldown = 5.0f;
    DefenceCooldown = 5.0f;
    UtilityCooldown = 5.0f;
    UltimateCooldown = 10.0f;

    //Starting Weapon
    UItemObject* Item = ItemStats::CreateItemFromData(ItemStats::CreateSpecificItemData(1, "Sword", "Common", ItemMeshData), InventoryComponent);
    InventoryComponent->AddItem(Item);
    Server_EquipItemByID(Item->ItemData.UniqueID);

    //Starting Stats
    if (ClassInfo)
    {
        FClassData Stats = ClassInfo->GetStats("Nemesis");
        SetPlayerStartingHealth(Stats.Health);
        PlayerMaxHealth = Stats.Health;
        PlayerHealth = Stats.Health;
        SetPlayerStartingArmor(Stats.Armor);
        SetPlayerStartingStrength(Stats.Strength);
        SetPlayerStartingDexterity(Stats.Dexterity);
        SetPlayerStartingIntelligence(Stats.Intelligence);
    }
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

void ANemesisCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);

    if (AbilitySystem)
    {
        AbilitySystem->CancelAllAbilities();
        AbilitySystem->RemoveAllGameplayCues();
    }
}


void ANemesisCharacter::GiveDamageAbilities()
{
    Super::GiveDamageAbilities();

    if (AbilitySystem)
    {
		// start ability cooldown
        //AbilityPointCounter->StartDamageCooldown(DamageCooldown);
    }
}

void ANemesisCharacter::GiveDefenceAbilities()
{
    Super::GiveDefenceAbilities();

    if (AbilitySystem)
    {
		// Give Impunity ability
        AbilitySystem->GiveAbility(FGameplayAbilitySpec(UImpunityAbility::StaticClass(), 1, 0));

		// Activate Impunity ability
        AbilitySystem->TryActivateAbilityByClass(UImpunityAbility::StaticClass());

		// Start ability cooldown
        //AbilityPointCounter->StartDefenceCooldown(DefenceCooldown);
    }
}

void ANemesisCharacter::GiveUtilityAbilities()
{
    Super::GiveUtilityAbilities();

    if (AbilitySystem)
    {

		// Give Vengeful Dance ability
        AbilitySystem->GiveAbility(FGameplayAbilitySpec(UVengefulDance::StaticClass(), 1, 0));
       
        // Activate Vengeful Dance ability
        AbilitySystem->TryActivateAbilityByClass(UVengefulDance::StaticClass());

        // Start ability cooldown
        //AbilityPointCounter->StartUtilityCooldown(UtilityCooldown);
    }
}

void ANemesisCharacter::GiveUltimateAbilities()
{
    Super::GiveUltimateAbilities();

    if (AbilitySystem)
    {
		// Start ability cooldown
        //AbilityPointCounter->StartUltimateCooldown(UltimateCooldown);
    }
}

void ANemesisCharacter::OnLevelUp()
{
    Super::OnLevelUp();

    UItemObject*& Item = InventoryComponent->GetEquippedItemReference("Sword");
    Item->ItemData.Level++;
    Item->ItemData.Strength += FMath::RandRange(1, 5);
    Item->ItemData.Intelligence += FMath::RandRange(0, 1);
    Item->ItemData.Dexterity += FMath::RandRange(1, 3);
    if (Inventory)
    {
        Inventory->RefreshEquippedUI();
    }
    InventoryComponent->UpdateTotalEquippedStats(this);
}

