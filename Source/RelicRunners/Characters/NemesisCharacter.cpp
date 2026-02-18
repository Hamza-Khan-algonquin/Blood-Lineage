#include "NemesisCharacter.h"
//#include "RelicRunners/AbilitySystem/AbilityPointCounter.h"
#include "RelicRunners/PlayerState/RelicRunnersPlayerState.h"
#include <RelicRunners/Item/ItemStats.h>
#include "RelicRunners/Inventory/InventoryComponent.h"
#include "RelicRunners/Inventory/Inventory.h"
#include <RelicRunners/Classes/ClassInfo.h>
#include "RelicRunners/Enemy/Characters/EnemyCharacter.h"
#include "RelicRunners/Enemy/Characters/EnemyCharacterAI.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "RelicRunners/Enemy/EnemyPawn.h"

ANemesisCharacter::ANemesisCharacter()
{
    Damage = 20.0f;
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

        // Give Back Stab ability
        AbilitySystem->GiveAbility(FGameplayAbilitySpec(UBackStab::StaticClass(), 1, 0));

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
        // Activate Back Stab ability
        AbilitySystem->TryActivateAbilityByClass(UBackStab::StaticClass());
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

void ANemesisCharacter::PlayRandomAttackAnimation()
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

void ANemesisCharacter::PerformSwingTrace()
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
                else if (AEnemyPawn* TPEnemy = Cast<AEnemyPawn>(HitActor))
                {
                    Server_HitEnemy(TPEnemy);
                }
                AlreadyHitActors.Add(HitActor);
            }
        }
    }
}

void ANemesisCharacter::BasicAttack()
{
    if (!RelicPlayerState || !RelicPlayerState->GetInventoryComponent()) return;

    if (!CanBasicAttack) return;
    CanBasicAttack = false;

    Damage = 5.0f;

    if (HasValidWeaponEquipped(RelicPlayerState->GetInventoryComponent()))
    {
        PlayRandomAttackAnimation();
    }

    GetWorldTimerManager().SetTimer(TimerHandle_AttackCooldown, this, &ARelicRunnersCharacter::ResetBasicAttack, BasicAttackCooldown, false);
}

void ANemesisCharacter::HeavyAttack()
{
    if (!RelicPlayerState || !RelicPlayerState->GetInventoryComponent()) return;

    if (!CanHeavyAttack) return;
    CanHeavyAttack = false;

    Damage = 10.0f;

    if (HasValidWeaponEquipped(RelicPlayerState->GetInventoryComponent()))
    {
        PlayRandomAttackAnimation();
    }

    GetWorldTimerManager().SetTimer(TimerHandle_AttackCooldown, this, &ARelicRunnersCharacter::ResetHeavyAttack, HeavyAttackCooldown, false);
}

void ANemesisCharacter::Server_HitEnemy_Implementation(AActor* HitActor)
{
    if (HitActor)
    {
        UGameplayStatics::ApplyDamage(HitActor, Damage, GetController(), this, nullptr);
    }
}