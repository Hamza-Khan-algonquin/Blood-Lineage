#pragma once

#pragma once

#include "CoreMinimal.h"
#include "RelicRunners/AbilitySystem/AbilityBase.h"
#include "VengefulDance.generated.h"

UCLASS()
class RELICRUNNERS_API UVengefulDance : public UAbilityBase
{
    GENERATED_BODY()

public:
    UVengefulDance();

    virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

    virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

    virtual FName GetAbilityName() const override { return FName("Vengeful Dance"); }

protected:

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VengefulDance")
    float DamageInterval = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VengefulDance")
    float KnockbackStrength = 1800.f;

private:

    FTimerHandle DamageTickTimer;
    FTimerHandle EndTimer;

    void ApplyRingDamage();
};
