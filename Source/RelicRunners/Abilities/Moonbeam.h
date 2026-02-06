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
    UPROPERTY(EditDefaultsOnly, Category = "Moonbeam")
    float BeamRange = 1200.f;

    UPROPERTY(EditDefaultsOnly, Category = "Moonbeam")
    float BeamRadius = 50.f;

    UPROPERTY(EditDefaultsOnly, Category = "Moonbeam")
    float BeamDuration = 3.f;

private:
    FTimerHandle CooldownTimerHandle;
    bool bIsOnCooldown = false;

    void SpawnBeamEffect(const FVector& Start, const FVector& End);
};
