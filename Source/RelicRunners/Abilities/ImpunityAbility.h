// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RelicRunners/AbilitySystem/AbilityBase.h"
#include "ImpunityAbility.generated.h"

/**
 * 
 */
UCLASS()
class RELICRUNNERS_API UImpunityAbility : public UAbilityBase
{
    GENERATED_BODY()

public:
    UImpunityAbility();

    virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;



    virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

    virtual FName GetAbilityName() const override { return FName("Impunity"); }

    float GetDamageReductionMultiplier() const { return bIsActive ? 0.5f : 1.0f; }

protected:

    void ResetSpeed();

    float OriginalSpeed;

    FTimerHandle SpeedResetTimer;

    bool bIsOnCooldown = false;

};