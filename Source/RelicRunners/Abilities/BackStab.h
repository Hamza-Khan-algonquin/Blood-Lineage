// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RelicRunners/AbilitySystem/AbilityBase.h"
#include "BackStab.generated.h"


UCLASS()
class RELICRUNNERS_API UBackStab : public UAbilityBase
{
	GENERATED_BODY()

public:
	UBackStab();

    virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
    virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
    virtual FName GetAbilityName() const override { return FName("Back Stab"); }

protected:
    AActor* FindBackstabTarget(AActor* AvatarActor) const;
    void ExecuteBackStabSequence();
    void ReturnPlayerToOriginalPosition();
    void PerformBackStabDamage(AActor* AvatarActor, AActor* TargetEnemy);

    float Range;

    FTimerHandle BackStabDamageTimerHandle;
    int CurrentBackStabCount;
    float SweepRadius = 300.f;
    float DamageDelay = 0.2f;
	float DashDistance = 200.f;
    float BehindOffset;
    int MaxBackstabSteps = 6;
    UPROPERTY()
    FVector OriginalLocation;

    UPROPERTY()
    FRotator OriginalRotation;

};


