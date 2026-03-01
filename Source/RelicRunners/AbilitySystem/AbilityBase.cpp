#include "AbilityBase.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"
#include "GameplayTagContainer.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "DrawDebugHelpers.h"

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

