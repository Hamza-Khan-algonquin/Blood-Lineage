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
}

// Called when player takes control of this character
void AAresCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

}

// Called when PlayerState is replicated on clients
void AAresCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

}

void AAresCharacter::GiveDamageAbilities()
{
	Super::GiveDamageAbilities();

}

void AAresCharacter::GiveDefenceAbilities()
{
	Super::GiveDefenceAbilities();

}

void AAresCharacter::GiveUtilityAbilities()
{
	Super::GiveUtilityAbilities();

}

void AAresCharacter::GiveUltimateAbilities()
{
	Super::GiveUltimateAbilities();

}

void AAresCharacter::OnLevelUp()
{

}




