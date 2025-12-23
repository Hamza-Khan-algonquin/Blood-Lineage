#include "NemesisCharacter.h"
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

}

// Called when player takes control of this character
void ANemesisCharacter::PossessedBy(AController* NewController)
{
    Super::PossessedBy(NewController);

}

// Called when PlayerState is replicated on clients
void ANemesisCharacter::OnRep_PlayerState()
{
    Super::OnRep_PlayerState();

}

void ANemesisCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);

}


void ANemesisCharacter::GiveDamageAbilities()
{
    Super::GiveDamageAbilities();

}

void ANemesisCharacter::GiveDefenceAbilities()
{
    Super::GiveDefenceAbilities();

}

void ANemesisCharacter::GiveUtilityAbilities()
{
    Super::GiveUtilityAbilities();

}

void ANemesisCharacter::GiveUltimateAbilities()
{
    Super::GiveUltimateAbilities();

}

void ANemesisCharacter::OnLevelUp()
{

}

