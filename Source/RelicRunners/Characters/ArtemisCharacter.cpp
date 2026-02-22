// Fill out your copyright notice in the Description page of Project Settings.


#include "ArtemisCharacter.h"
#include "RelicRunners/PlayerState/RelicRunnersPlayerState.h"
#include <RelicRunners/Item/ItemStats.h>
#include "RelicRunners/Inventory/InventoryComponent.h"
#include "RelicRunners/Inventory/Inventory.h"
#include <RelicRunners/Classes/ClassInfo.h>
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "RelicRunners/Enemy/Characters/EnemyCharacter.h"
#include "RelicRunners/Enemy/Characters/EnemyCharacterAI.h"
#include "Net/UnrealNetwork.h"
#include "RelicRunners/Abilities/PiercingArrow.h"
#include "RelicRunners/Enemy/EnemyPawn.h"


AArtemisCharacter::AArtemisCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	bProjectileActive = false;
	ProjectileSpeed = 5000.f;
	ProjectileLifetime = 2.f;
	ProjectileDamage = 70.0f;

}

void AArtemisCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (AbilitySystem)
	{
		// Give Mark of the Hunt ability
		AbilitySystem->GiveAbility(FGameplayAbilitySpec(UMarkOfTheHunt::StaticClass(), 1, 0));

		// Give Moonbeam ability
		AbilitySystem->GiveAbility(FGameplayAbilitySpec(UMoonbeam::StaticClass(), 1, 0));

		// Give NatureBlessing ability
		AbilitySystem->GiveAbility(FGameplayAbilitySpec(UNaturesBlessing::StaticClass(), 1, 0));

        // Give PiercingArrow ability
		AbilitySystem->GiveAbility(FGameplayAbilitySpec(UPiercingArrow::StaticClass(), 1, 0));
	}

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
		AbilitySystem->TryActivateAbilityByClass(UPiercingArrow::StaticClass());
	}
}

void AArtemisCharacter::GiveDefenceAbilities()
{
	Super::GiveDefenceAbilities();

	if (AbilitySystem)
	{

		AbilitySystem->TryActivateAbilityByClass(UNaturesBlessing::StaticClass());
	}
}

void AArtemisCharacter::GiveUtilityAbilities()
{
	Super::GiveUtilityAbilities();

	if (AbilitySystem)
	{
		// Activate Mark of the Hunt
		AbilitySystem->TryActivateAbilityByClass(UMarkOfTheHunt::StaticClass());
	}
}

void AArtemisCharacter::GiveUltimateAbilities()
{
	Super::GiveUltimateAbilities();

	if (AbilitySystem)
	{
		// Activate Moonbeam now (fires the beam)
		AbilitySystem->TryActivateAbilityByClass(UMoonbeam::StaticClass());
	}

}

void AArtemisCharacter::PlayRandomAttackAnimation()
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

void AArtemisCharacter::PerformSwingTrace()
{
}

void AArtemisCharacter::BasicAttack()
{
	if (!RelicPlayerState || !RelicPlayerState->GetInventoryComponent()) return;
	if (!CanBasicAttack) return;
	if (!HasValidWeaponEquipped(RelicPlayerState->GetInventoryComponent())) return;

	CanBasicAttack = false;

	ProjectileDamage = 70.0f;

	// Spawn projectile
	bProjectileActive = true;
	ProjectileLocation = GetActorLocation() + GetActorForwardVector() * 100.f + FVector(0, 0, 50.f);
	ProjectileVelocity = GetActorForwardVector() * ProjectileSpeed;
	ProjectileElapsed = 0.f;

	PlayRandomAttackAnimation();
	AlreadyHitActors.Empty();

	// Spawn static mesh
	if (ProjectileMeshAsset)
	{
		if (!ActiveProjectileMesh)
		{
			ActiveProjectileMesh = NewObject<UStaticMeshComponent>(this);
			ActiveProjectileMesh->RegisterComponent();
			ActiveProjectileMesh->SetStaticMesh(ProjectileMeshAsset);
			ActiveProjectileMesh->SetWorldLocation(ProjectileLocation);
			ActiveProjectileMesh->SetWorldScale3D(FVector(1.f));
			ActiveProjectileMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

			// Initial rotation based on velocity
			ActiveProjectileMesh->SetWorldRotation(ProjectileVelocity.Rotation());
		}
	}

	// Cooldown
	GetWorldTimerManager().SetTimer(TimerHandle_AttackCooldown, this, &AArtemisCharacter::ResetBasicAttack, BasicAttackCooldown, false);
}

void AArtemisCharacter::HeavyAttack()
{
	if (!RelicPlayerState || !RelicPlayerState->GetInventoryComponent()) return;
	if (!CanHeavyAttack) return;
	if (!HasValidWeaponEquipped(RelicPlayerState->GetInventoryComponent())) return;

	CanHeavyAttack = false;

	ProjectileDamage = 100.0f;

	// Spawn projectile
	bProjectileActive = true;
	ProjectileLocation = GetActorLocation() + GetActorForwardVector() * 100.f + FVector(0, 0, 50.f);
	ProjectileVelocity = GetActorForwardVector() * ProjectileSpeed;
	ProjectileElapsed = 0.f;

	PlayRandomAttackAnimation();
	AlreadyHitActors.Empty();

	// Spawn static mesh
	if (ProjectileMeshAsset)
	{
		if (!ActiveProjectileMesh)
		{
			ActiveProjectileMesh = NewObject<UStaticMeshComponent>(this);
			ActiveProjectileMesh->RegisterComponent();
			ActiveProjectileMesh->SetStaticMesh(ProjectileMeshAsset);
			ActiveProjectileMesh->SetWorldLocation(ProjectileLocation);
			ActiveProjectileMesh->SetWorldScale3D(FVector(1.f));
			ActiveProjectileMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

			// Initial rotation based on velocity
			ActiveProjectileMesh->SetWorldRotation(ProjectileVelocity.Rotation());
		}
	}

	// Cooldown
	GetWorldTimerManager().SetTimer(TimerHandle_AttackCooldown, this, &AArtemisCharacter::ResetHeavyAttack, HeavyAttackCooldown, false);
}

void AArtemisCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bProjectileActive) return;

	ProjectileElapsed += DeltaTime;

	// Apply gravity
	ProjectileVelocity.Z -= ProjectileGravity * DeltaTime;

	PreviousProjectileLocation = ProjectileLocation;
	ProjectileLocation += ProjectileVelocity * DeltaTime;

	// Move and rotate the mesh to match velocity
	if (ActiveProjectileMesh)
	{
		ActiveProjectileMesh->SetWorldLocation(ProjectileLocation);
		ActiveProjectileMesh->SetWorldRotation(ProjectileVelocity.Rotation());
	}

	// Collision
	TArray<FHitResult> Hits;
	FCollisionShape Sphere = FCollisionShape::MakeSphere(15.f);

	bool bHit = GetWorld()->SweepMultiByChannel(
		Hits,
		PreviousProjectileLocation,
		ProjectileLocation,
		FQuat::Identity,
		ECC_GameTraceChannel1,
		Sphere
	);

	for (const FHitResult& Hit : Hits)
	{
		if (AActor* HitActor = Hit.GetActor())
		{
			if (AlreadyHitActors.Contains(HitActor))
				continue;

			if (AEnemyCharacter* Enemy = Cast<AEnemyCharacter>(HitActor))
			{
				Server_HitEnemy(Enemy);
			}
			else if (AEnemyCharacterAI* TEnemy = Cast<AEnemyCharacterAI>(HitActor))
			{
				Server_HitEnemy(TEnemy);
			}
			else if (AEnemyPawn* TPEnemy = Cast<AEnemyPawn>(HitActor))
			{
				Server_HitEnemy(TPEnemy);
			}

			AlreadyHitActors.Add(HitActor);

			// Stop projectile on hit
			bProjectileActive = false;

			if (ActiveProjectileMesh)
			{
				ActiveProjectileMesh->DestroyComponent();
				ActiveProjectileMesh = nullptr;
			}
			break;
		}
	}

	// Stop projectile after lifetime
	if (ProjectileElapsed >= ProjectileLifetime)
	{
		bProjectileActive = false;
		if (ActiveProjectileMesh)
		{
			ActiveProjectileMesh->DestroyComponent();
			ActiveProjectileMesh = nullptr;
		}
	}
}


void AArtemisCharacter::Server_HitEnemy_Implementation(AActor* HitActor)
{
	if (HitActor)
	{
		UGameplayStatics::ApplyDamage(HitActor, ProjectileDamage, GetController(), this, nullptr);
	}
}