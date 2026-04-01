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

    void HandleChargeTick(FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, FGameplayAbilityActivationInfo ActivationInfo);

    FTimerHandle EndTimerHandle;
    FTimerHandle ChargeTickHandle;

    float ChargeDuration = 10.f;
    float ChargeSpeed = 1500.f;
    float KnockbackStrength = 1000.f;
    float CheckRadius = 100.f;
    float CheckDistance = 150.f;
    float OriginalSpeed;

    UPROPERTY()
    UMaterialInstanceDynamic* OverlayMID = nullptr;

    UPROPERTY(EditDefaultsOnly)
    UMaterialInterface* OverlayMaterial;

};
