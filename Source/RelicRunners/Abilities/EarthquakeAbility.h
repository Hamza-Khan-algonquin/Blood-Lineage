// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RelicRunners/AbilitySystem/AbilityBase.h"
#include "EarthquakeAbility.generated.h"

/**
 * 
 */
UCLASS()
class RELICRUNNERS_API UEarthquakeAbility : public UAbilityBase
{
    GENERATED_BODY()

public:
    UEarthquakeAbility();

    virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

    virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
    virtual FName GetAbilityName() const override { return FName("Earthquake"); }

protected:
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Earthquake")
    float Radius = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Earthquake|Knockback")
    float KnockbackStrength = 600.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Earthquake|Knockback")
    float VerticalKnockback = 500.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Earthquake")
    float StunDuration = 3.f;

    FGameplayAbilityActivationInfo CachedActivationInfo;



private:
    bool bIsOnCooldown = false;
    FTimerHandle CooldownTimerHandle;

    void ApplyDamageAndStun();
};