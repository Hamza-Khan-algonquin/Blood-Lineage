// Fill out your copyright notice in the Description page of Project Settings.


#include "AresCharacter.h"
#include "RelicRunners/PlayerState/RelicRunnersPlayerState.h"
#include <RelicRunners/Item/ItemStats.h>
#include "RelicRunners/Inventory/InventoryComponent.h"
#include "RelicRunners/Inventory/Inventory.h"
#include <RelicRunners/Classes/ClassInfo.h>
#include "RelicRunners/Enemy/Characters/EnemyCharacter.h"
#include "RelicRunners/Enemy/Characters/EnemyCharacterAI.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "RelicRunners/Abilities/GuardAbility.h"


AAresCharacter::AAresCharacter()
{
	Damage = 8.0f;
}

void AAresCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (AbilitySystem)
	{
		// Give Earthquake ability
		AbilitySystem->GiveAbility(FGameplayAbilitySpec(UEarthquakeAbility::StaticClass(), 1, 0));
		// Give Guard ability
		AbilitySystem->GiveAbility(FGameplayAbilitySpec(UGuardAbility::StaticClass(), 1, 0));
		// Give War Banner ability
		AbilitySystem->GiveAbility(FGameplayAbilitySpec(UWarBannerAbility::StaticClass(), 1, 0));

		// Give Rage of the Gods ability
		AbilitySystem->GiveAbility(FGameplayAbilitySpec(URageOfTheGods::StaticClass(), 1, 0));
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
		AbilitySystem->TryActivateAbilityByClass(UGuardAbility::StaticClass());
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
		AbilitySystem->TryActivateAbilityByClass(URageOfTheGods::StaticClass());
	}
}

void AAresCharacter::PlayRandomAttackAnimation()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (!AnimInstance) return;

	TArray<UAnimSequence*> SwordAttacks;
	if (SwordBasicAttack0) SwordAttacks.Add(SwordBasicAttack0);
	if (SwordBasicAttack1) SwordAttacks.Add(SwordBasicAttack1);
	if (SwordBasicAttack2) SwordAttacks.Add(SwordBasicAttack2);
	if (SwordBasicAttack3) SwordAttacks.Add(SwordBasicAttack3);

	if (SwordAttacks.Num() > 0)
	{
		int Index = FMath::RandRange(0, SwordAttacks.Num() - 1);
		AnimInstance->PlaySlotAnimationAsDynamicMontage(SwordAttacks[Index], FName("DefaultSlot"));
	}
}

void AAresCharacter::PerformSwingTrace()
{
	FVector Start = GetMesh()->GetSocketLocation("MainhandSocket");
	FVector End = GetMesh()->GetSocketLocation("TipSocket");

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	TArray<FHitResult> Hits;
	GetWorld()->SweepMultiByChannel(
		Hits, Start, End, FQuat::Identity,
		ECC_Pawn, FCollisionShape::MakeSphere(100.0f), Params
	);

	AlreadyHitActors.Empty();

	for (const FHitResult& Hit : Hits)
	{
		if (AActor* HitActor = Hit.GetActor())
		{
			if (!AlreadyHitActors.Contains(HitActor))
			{
				if (AEnemyCharacter* Enemy = Cast<AEnemyCharacter>(HitActor))
				{
					Server_HitEnemy(Enemy);
				}
				else if (AEnemyCharacterAI* TEnemy = Cast<AEnemyCharacterAI>(HitActor))
				{
					Server_HitEnemy(TEnemy);
				}
				AlreadyHitActors.Add(HitActor);
			}
		}
	}
}

void AAresCharacter::BasicAttack()
{
	if (!RelicPlayerState || !RelicPlayerState->GetInventoryComponent()) return;

	if (!CanBasicAttack) return;
	CanBasicAttack = false;

	Damage = 8.0f;

	if (HasValidWeaponEquipped(RelicPlayerState->GetInventoryComponent()))
	{
		PlayRandomAttackAnimation();
	}

	GetWorldTimerManager().SetTimer(TimerHandle_AttackCooldown,this,&ARelicRunnersCharacter::ResetBasicAttack,BasicAttackCooldown,false);
}

void AAresCharacter::HeavyAttack()
{
	if (!RelicPlayerState || !RelicPlayerState->GetInventoryComponent()) return;

	if (!CanHeavyAttack) return;
	CanHeavyAttack = false;

	Damage = 12.0f;

	if (HasValidWeaponEquipped(RelicPlayerState->GetInventoryComponent()))
	{
		PlayRandomAttackAnimation();
	}

	GetWorldTimerManager().SetTimer(TimerHandle_AttackCooldown, this, &ARelicRunnersCharacter::ResetHeavyAttack, HeavyAttackCooldown, false);
}

void AAresCharacter::Server_HitEnemy_Implementation(AActor* HitActor)
{
	if (HitActor)
	{
		UGameplayStatics::ApplyDamage(HitActor, Damage, GetController(), this, nullptr);
	}
}