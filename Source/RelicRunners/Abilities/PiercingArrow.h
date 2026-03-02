// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RelicRunners/AbilitySystem/AbilityBase.h"
#include "PiercingArrow.generated.h"

UCLASS()
class RELICRUNNERS_API UPiercingArrow : public UAbilityBase
{
    GENERATED_BODY()

public:
    UPiercingArrow();

    virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

    virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

    virtual FName GetAbilityName() const override { return FName("Piercing Arrow"); }

protected:
    void TickArrow();

    void Explode(FVector ImpactPoint);

    FTimerHandle ProjectileTimer;
    FTimerHandle DurationTimer;

    FVector ProjectileLocation;
    FVector ProjectileVelocity;

    float Gravity = -980.f;
    float ProjectileRadius = 40.f;
    float ExplosionRadius = 500.f;
    float DamageAmount = 100.f;
};