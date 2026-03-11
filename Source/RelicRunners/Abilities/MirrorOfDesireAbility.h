#pragma once

#include "CoreMinimal.h"
#include "RelicRunners/AbilitySystem/AbilityBase.h"
#include "MirrorOfDesireAbility.generated.h"

UCLASS()
class RELICRUNNERS_API UMirrorOfDesireAbility : public UAbilityBase
{
    GENERATED_BODY()

public:
    UMirrorOfDesireAbility();

    virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,const FGameplayAbilityActorInfo* ActorInfo,const FGameplayAbilityActivationInfo ActivationInfo,const FGameplayEventData* TriggerEventData) override;

    virtual void EndAbility(const FGameplayAbilitySpecHandle Handle,const FGameplayAbilityActorInfo* ActorInfo,const FGameplayAbilityActivationInfo ActivationInfo,bool bReplicateEndAbility,bool bWasCancelled) override;

    virtual FName GetAbilityName() const override { return FName("MirrorOfDesire"); }

protected:
    UFUNCTION()
    void OnOwnerTakeDamage(AActor* DamagedActor, float DamageAmount, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser);

    FTimerHandle AbilityTimerHandle;
};
