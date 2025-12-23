// Fill out your copyright notice in the Description page of Project Settings.


#include "ArtemisCharacter.h"
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
}

void AArtemisCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
}

void AArtemisCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

}
void AArtemisCharacter::GiveDamageAbilities()
{
	Super::GiveDamageAbilities();
}

void AArtemisCharacter::GiveDefenceAbilities()
{
	Super::GiveDefenceAbilities();
}

void AArtemisCharacter::GiveUtilityAbilities()
{
	Super::GiveUtilityAbilities();
}

void AArtemisCharacter::GiveUltimateAbilities()
{
	Super::GiveUltimateAbilities();
}

void AArtemisCharacter::OnLevelUp()
{

}
