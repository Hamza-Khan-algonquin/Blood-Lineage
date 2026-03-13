#pragma once

#include "CoreMinimal.h"
#include "RelicRunners/AbilitySystem/AbilityBase.h"
#include "Moonbeam.generated.h"

UCLASS()
class RELICRUNNERS_API UMoonbeam : public UAbilityBase
{
    GENERATED_BODY()

public:
    UMoonbeam();

    virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
    virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
    virtual FName GetAbilityName() const override { return FName("Moonbeam"); }

protected:
    FTimerHandle BeamTickHandle;
    FTimerHandle DurationHandle;

    float BeamLength = 3000.f;
    float TickInterval = 0.2f;
    void BeamTick();
};
