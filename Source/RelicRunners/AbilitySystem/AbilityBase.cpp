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

// In your Character class or a Utility header
void UAbilityBase::CalculateAbilityDamage(AActor* InstigatorActor, AActor* TargetActor, float BaseDamage)
{
    if (!InstigatorActor || !TargetActor) return;

    // Get the controller safely
    AController* InstigatorController = InstigatorActor->GetInstigatorController();
    if (!InstigatorController) return;

    // Try to cast to player controller
    APlayerController* PC = Cast<APlayerController>(InstigatorController);

    // Get player state and inventory component
    UInventoryComponent* InventoryComponent = nullptr;
    if (PC)
    {
        if (ARelicRunnersPlayerState* PS = PC->GetPlayerState<ARelicRunnersPlayerState>())
        {
            InventoryComponent = PS->GetInventoryComponent();
        }
    }

    // Compute strength-based scaling
    float PercentStrengthIncrease = 0.f;
    if (InventoryComponent)
    {
        PercentStrengthIncrease = ItemStats::GetStrengthPerc(InventoryComponent->CachedEquippedStats.TotalStrength);
    }

    // Final damage
    float FinalDamage = BaseDamage * (1.f + PercentStrengthIncrease / 100.f);

    // Apply damage
    UGameplayStatics::ApplyDamage(TargetActor, FinalDamage, InstigatorController, InstigatorActor, UDamageType::StaticClass());
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







