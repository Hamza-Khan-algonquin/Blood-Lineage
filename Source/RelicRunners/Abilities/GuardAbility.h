#pragma once

#include "CoreMinimal.h"
#include "RelicRunners/AbilitySystem/AbilityBase.h"
#include "GuardAbility.generated.h"

UCLASS()
class RELICRUNNERS_API UGuardAbility : public UAbilityBase
{
    GENERATED_BODY()

public:
    UGuardAbility();

    virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,const FGameplayAbilityActorInfo* ActorInfo,const FGameplayAbilityActivationInfo ActivationInfo,const FGameplayEventData* TriggerEventData) override;

    virtual void EndAbility(const FGameplayAbilitySpecHandle Handle,const FGameplayAbilityActorInfo* ActorInfo,const FGameplayAbilityActivationInfo ActivationInfo,bool bReplicateEndAbility,bool bWasCancelled) override;

    virtual FName GetAbilityName() const override { return FName("Guard"); }

protected:
    UPROPERTY(EditDefaultsOnly, Category = "Guard")
    float GuardDuration = 4.f;
   
    UPROPERTY(EditDefaultsOnly, Category = "Guard")
    float GuardAngleDegrees = 100.f;

    UPROPERTY(EditDefaultsOnly, Category = "Guard")
    float DebugLineLength = 200.f;

private:
    FTimerHandle EndTimerHandle;
};
