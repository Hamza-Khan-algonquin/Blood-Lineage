// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RelicRunners/AbilitySystem/AbilityBase.h"
#include "DivinePunishment.generated.h"


UCLASS()
class RELICRUNNERS_API UDivinePunishment : public UAbilityBase
{
    GENERATED_BODY()

public:
	UDivinePunishment();

    virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
    virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
    virtual FName GetAbilityName() const override { return FName("Divine Punishment"); }

protected:
    void KillEnemiesInRadius(const FVector& Center);
    void DivineDamageTick();
    FTimerHandle DivineDamageTimerHandle;
    FTimerHandle PauseMontageTimerHandle;
    FTimerHandle ResumeMontageTimerHandle;
    FTimerHandle DamageDelayHandle;
    FTimerHandle VFXDelayHandle;
    int32 DamageTicksElapsed = 0;

    TWeakObjectPtr<AController> CachedInstigatorController;
	FVector CachedCenter;



};

