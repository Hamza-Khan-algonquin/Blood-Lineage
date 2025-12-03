// Fill out your copyright notice in the Description page of Project Settings.


#include "AresCharacter.h"
#include "RelicRunners/AbilitySystem/AbilityPointCounter.h"
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

	// set cooldown durations
	DamageCooldown = 5.0f;
	DefenceCooldown = 5.0f;
	UtilityCooldown = 5.0f;
	UltimateCooldown = 10.0f;

	if (AbilitySystem)
	{
		// Give Earthquake ability
		AbilitySystem->GiveAbility(FGameplayAbilitySpec(UEarthquakeAbility::StaticClass(), 1, 0));
	}

	//Starting Weapon
	UItemObject* Item = ItemStats::CreateItemFromData(ItemStats::CreateSpecificItemData(1, "Greatsword", "Common", ItemMeshData), InventoryComponent);
	InventoryComponent->AddItem(Item);
	Server_EquipItemByID(Item->ItemData.UniqueID);

	//Starting Stats
	if (ClassInfo)
	{
		FClassData Stats = ClassInfo->GetStats("Ares");
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
		// Give Earthquake ability
		//AbilitySystem->GiveAbility(FGameplayAbilitySpec(UEarthquakeAbility::StaticClass(), 1, 0));
		// Activate Earthquake ability 
		AbilitySystem->TryActivateAbilityByClass(UEarthquakeAbility::StaticClass());
		// start ability cooldown
		//AbilityPointCounter->StartDamageCooldown(DamageCooldown);
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

	//press ability key
	//IsWarBannerActive = !IsWarBannerActive;

	//if (WarBannerAbility == nullptr)
	//	return;

	//if (IsWarBannerActive)
	//{
	//	WarBannerAbility->ActivateAbility();
	//}
	//else
	//{
	//	WarBannerAbility->CancelAbility();
	//}

	//// aiming with the mouse
	//if (IsWarBannerActive)
	//{
	//	FHitResult HitResult;
	//	FCollisionQueryParams TraceParams(FName(TEXT("LineTrace")), true, this);

	//	bool bHit = GetWorld()->LineTraceSingleByChannel(
	//		HitResult,
	//		PlayerLocation,
	//		PlayerLocation + (PlayerForward * 1000.0f),
	//		ECC_Visibility,
	//		TraceParams);

	//	FVector targetPosition = bHit ? HitResult.Location : PlayerLocation + (PlayerForward * 1000.0f);

	//	DrawDebugLine(GetWorld(), PlayerLocation, targetPosition, FColor::Blue);
	//	WarBannerAbility->SetActorLocation(targetPosition);
	//	WarBannerAbility->SetActorRotation(FRotator(0.0f, GetControlRotation().Yaw, 0.0f));

	//	//on let click
	//	if (IsWarBannerActive && WarBannerAbility != nullptr)
	//	{
	//		WarBannerAbility->Server_SpawnBanner();
	//	}
	//}

	//spawning the actor
	//if (WarBannerAbilityTemplate != nullptr)
	//{
	//	WarBannerAbility = GetWorld()->SpawnActor<AWarBannerAbility>(WarBannerAbilityTemplate, FVector::ZeroVector, FRotator::ZeroRotator);
	//	WarBannerAbility->Server_Initialize(this);
	//}

	if (AbilitySystem)
	{
		//start ability cooldown
		//AbilityPointCounter->StartUtilityCooldown(UtilityCooldown);
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

void AAresCharacter::OnLevelUp()
{
	Super::OnLevelUp();

	UItemObject*& Item = InventoryComponent->GetEquippedItemReference("Greatsword");
	Item->ItemData.Level++;
	Item->ItemData.Health += FMath::RandRange(1, 5);
	Item->ItemData.Armor += FMath::RandRange(1, 3);
	Item->ItemData.Strength += FMath::RandRange(0, 1);
	Item->ItemData.Intelligence += FMath::RandRange(0, 1);
	Item->ItemData.Dexterity += FMath::RandRange(0, 1);
	if (Inventory)
	{
		Inventory->RefreshEquippedUI();
	}
	
	InventoryComponent->UpdateTotalEquippedStats(this);
}




