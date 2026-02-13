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
    UPROPERTY(EditDefaultsOnly, Category = "Mirror")
    float MirrorDuration = 5.f;

    UPROPERTY(EditDefaultsOnly, Category = "Mirror")
    float ReflectPercent = 5.35f;

    UPROPERTY(EditDefaultsOnly, Category = "Mirror")
    float MirrorRadius = 150.f;

private:
    FTimerHandle EndTimerHandle;
};
