// Fill out your copyright notice in the Description page of Project Settings.


#include "AphroditeCharacter.h"
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

AAphroditeCharacter::AAphroditeCharacter()
{
    PrimaryActorTick.bCanEverTick = true;
    bProjectileActive = false;
    ProjectileSpeed = 1200.f;
    ProjectileLifetime = 2.f;
    ProjectileDamage = 90.0f;
}

void AAphroditeCharacter::BeginPlay()
{
    Super::BeginPlay();

    if (AbilitySystem)
    {
        // Give Bundle of Joy ability 
        AbilitySystem->GiveAbility(FGameplayAbilitySpec(UBundleOfJoy::StaticClass(), 1, 0));
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
		// Activate Bundle of Joy ability
        AbilitySystem->TryActivateAbilityByClass(UBundleOfJoy::StaticClass());

    }
}
void AAphroditeCharacter::GiveDefenceAbilities()
{
	Super::GiveDefenceAbilities();

    if (AbilitySystem)
    {

    }
}

void AAphroditeCharacter::GiveUtilityAbilities()
{
    Super::GiveUtilityAbilities();

    if (AbilitySystem)
    {
        // Activate KissOfLife ability 
        AbilitySystem->TryActivateAbilityByClass(UKissOfLife::StaticClass());
    }
}

void AAphroditeCharacter::GiveUltimateAbilities()
{
	Super::GiveUltimateAbilities();

    if (AbilitySystem)
    {

    }
}

void AAphroditeCharacter::PerformSwingTrace()
{
}

void AAphroditeCharacter::PlayRandomAttackAnimation()
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

void AAphroditeCharacter::BasicAttack()
{
    if (!RelicPlayerState || !RelicPlayerState->GetInventoryComponent()) return;

    if (!CanBasicAttack) return;
    CanBasicAttack = false;

    ProjectileDamage = 90.0f;

    if (!HasValidWeaponEquipped(RelicPlayerState->GetInventoryComponent())) return;

    // Spawn projectile
    bProjectileActive = true;
    ProjectileLocation = GetActorLocation() + GetActorForwardVector() * 100.f + FVector(0, 0, 50.f);
    ProjectileVelocity = GetActorForwardVector() * ProjectileSpeed;
    ProjectileElapsed = 0.f;

     PlayRandomAttackAnimation();

     AlreadyHitActors.Empty();

    // Cooldown
    GetWorldTimerManager().SetTimer(TimerHandle_AttackCooldown, this, &AAphroditeCharacter::ResetBasicAttack, BasicAttackCooldown, false);
}

void AAphroditeCharacter::HeavyAttack()
{
    if (!RelicPlayerState || !RelicPlayerState->GetInventoryComponent()) return;

    if (!CanHeavyAttack) return;
    CanHeavyAttack = false;

    ProjectileDamage = 120.0f;

    if (!HasValidWeaponEquipped(RelicPlayerState->GetInventoryComponent())) return;

    // Spawn projectile
    bProjectileActive = true;
    ProjectileLocation = GetActorLocation() + GetActorForwardVector() * 100.f + FVector(0, 0, 50.f);
    ProjectileVelocity = GetActorForwardVector() * ProjectileSpeed;
    ProjectileElapsed = 0.f;

    PlayRandomAttackAnimation();

    AlreadyHitActors.Empty();

    // Cooldown
    GetWorldTimerManager().SetTimer(TimerHandle_AttackCooldown, this, &AAphroditeCharacter::ResetHeavyAttack, HeavyAttackCooldown, false);
}

void AAphroditeCharacter::Server_HitEnemy_Implementation(AActor* HitActor)
{
    if (HitActor)
    {
        UGameplayStatics::ApplyDamage(HitActor, 90.f, GetController(), this, nullptr);
    }
}

void AAphroditeCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!bProjectileActive) return;

    ProjectileElapsed += DeltaTime;

    PreviousProjectileLocation = ProjectileLocation;
    ProjectileLocation += ProjectileVelocity * DeltaTime;

    DrawDebugSphere(GetWorld(), ProjectileLocation, 15.f, 12, FColor::Red, false, 0.f);

    TArray<FHitResult> Hits;
    FCollisionShape Sphere = FCollisionShape::MakeSphere(15.f);

    bool bHit = GetWorld()->SweepMultiByChannel(
        Hits,
        PreviousProjectileLocation,
        ProjectileLocation,
        FQuat::Identity,
        ECC_Pawn,
        Sphere
    );

    if (bHit)
    {
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

                AlreadyHitActors.Add(HitActor);

                bProjectileActive = false;
                break;
            }
        }
    }

    if (ProjectileElapsed >= ProjectileLifetime)
    {
        bProjectileActive = false;
    }
}






