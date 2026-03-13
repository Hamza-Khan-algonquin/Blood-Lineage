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

    void ApplyDotTick();

    FTimerHandle ProjectileTimer;
    FTimerHandle DurationTimer;
    FTimerHandle DotTimer;

    FVector ProjectileLocation;
    FVector ProjectileVelocity;
    FVector DotCenter;
    float Gravity;
    float ProjectileRadius;
    float DotTickRate;
    float DotDuration;
    float DotTimeElapsed;


};