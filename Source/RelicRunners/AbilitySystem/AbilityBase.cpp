#include "AbilityBase.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"
#include "GameplayTagContainer.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "DrawDebugHelpers.h"
#include "RelicRunners/Inventory/InventoryComponent.h"
#include "RelicRunners/Item/ItemStats.h"
#include "RelicRunners/PlayerState/RelicRunnersPlayerState.h"

UAbilityBase::UAbilityBase()
{
    // Set ability to be unique per actor
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

}

void UAbilityBase::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

void UAbilityBase::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}



void UAbilityBase::PlayAbilityCastSound(const FGameplayAbilityActorInfo* ActorInfo) const
{
    if (!AbilityCastSound || !ActorInfo) return;

    AActor* Avatar = ActorInfo->AvatarActor.Get();
    if (!Avatar) return;

    UGameplayStatics::PlaySoundAtLocation(Avatar, AbilityCastSound, Avatar->GetActorLocation());
}


void UAbilityBase::CalculateAbilityDamage(AActor* InstigatorActor, AActor* TargetActor, float BaseDamage)
{
    if (!InstigatorActor || !TargetActor) return;

    AController* InstigatorController = InstigatorActor->GetInstigatorController();
    if (!InstigatorController) return;

    APlayerController* PC = Cast<APlayerController>(InstigatorController);

    UInventoryComponent* InventoryComponent = nullptr;
    if (PC)
    {
        if (ARelicRunnersPlayerState* PS = PC->GetPlayerState<ARelicRunnersPlayerState>())
        {
            InventoryComponent = PS->GetInventoryComponent();
        }
    }

    float PercentStrengthIncrease = 0.f;
    float CritChance = 0.f;
    float CritDamageMultiplier = 1.f;

    if (InventoryComponent)
    {
        int Strength = InventoryComponent->CachedEquippedStats.TotalStrength;
        int CritValue = InventoryComponent->CachedEquippedStats.TotalDexterity;
        int CritDamageValue = InventoryComponent->CachedEquippedStats.TotalDexterity;

        PercentStrengthIncrease = ItemStats::GetStrengthPerc(Strength);

        CritChance = ItemStats::GetCritPerc(CritValue);
        CritChance = FMath::Clamp(CritChance, 0.f, 100.f);
        CritDamageMultiplier = ItemStats::GetCritDmgPerc(CritDamageValue);
    }

    float FinalDamage = BaseDamage * (1.f + PercentStrengthIncrease / 100.f);

    // Roll for crit
    float RandomRoll = FMath::FRandRange(0.f, 100.f);
    bool bIsCrit = RandomRoll <= CritChance;

    if (bIsCrit)
    {
        FinalDamage *= CritDamageMultiplier;
    }

    UGameplayStatics::ApplyDamage(TargetActor, FinalDamage, InstigatorController, InstigatorActor, UDamageType::StaticClass());
}


float UAbilityBase::CalculateAbilityMana(AActor* InstigatorActor, float BaseMana)
{
    if (!InstigatorActor) return BaseMana;

    AController* InstigatorController = InstigatorActor->GetInstigatorController();
    if (!InstigatorController) return BaseMana;

    APlayerController* PC = Cast<APlayerController>(InstigatorController);

    UInventoryComponent* InventoryComponent = nullptr;
    if (PC)
    {
        if (ARelicRunnersPlayerState* PS = PC->GetPlayerState<ARelicRunnersPlayerState>())
        {
            InventoryComponent = PS->GetInventoryComponent();
        }
    }

    float ManaReductionPercent = 0.f;

    if (InventoryComponent)
    {
        float ManaReductionValue = InventoryComponent->CachedEquippedStats.TotalIntelligence;
        ManaReductionPercent = ItemStats::GetAbilityCostPerc(ManaReductionValue);
    }

    // Apply reduction
    float FinalManaCost = BaseMana * (1.f - ManaReductionPercent / 100.f);

    FinalManaCost = FMath::Max(FinalManaCost, 0.f);

    return FinalManaCost;

}

UAnimMontage* UAbilityBase::PlayAbilityAnimation(const FGameplayAbilityActorInfo* ActorInfo, UAnimMontage* Montage)
{
    if (!ActorInfo || !ActorInfo->AvatarActor.IsValid() || !Montage)
        return nullptr;

    AActor* Avatar = ActorInfo->AvatarActor.Get();
    if (!Avatar) return nullptr;

    USkeletalMeshComponent* Mesh = Avatar->FindComponentByClass<USkeletalMeshComponent>();
    if (!Mesh) return nullptr;

    UAnimInstance* AnimInstance = Mesh->GetAnimInstance();
    if (!AnimInstance) return nullptr;

    AnimInstance->Montage_Play(Montage, 1.f);
    return Montage;
}



