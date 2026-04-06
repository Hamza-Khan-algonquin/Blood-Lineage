// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RelicRunners/AbilitySystem/AbilityBase.h"
#include "GoddessOfLove.generated.h"

UCLASS()
class RELICRUNNERS_API UGoddessOfLove : public UAbilityBase
{
    GENERATED_BODY()

public:

    UGoddessOfLove();
	
    virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

    virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
    virtual FName GetAbilityName() const override { return FName("Goddess Of Love"); }


protected:
    void ApplyInitialOverlap();
    void ApplyPassiveHealthRegen();
    void TickDamage();

    void HandleDurationEnd();


    FTimerHandle DOTTimerHandle;
    FTimerHandle OverlapTimer;
    FTimerHandle PassiveRegenTimerHandle;
    FTimerHandle DurationHandle;

    TArray<TWeakObjectPtr<AActor>> AffectedEnemies;

    float ElapsedDOTTime;
    float DOTTickInterval;
    float DOTDuration;
    float PassiveRegenTickInterval;
    FVector DamageOrigin;

    UPROPERTY()
    UMaterialInstanceDynamic* OverlayMID = nullptr;

    UPROPERTY(EditDefaultsOnly)
    UMaterialInterface* OverlayMaterial;
    TMap<TWeakObjectPtr<AActor>, UNiagaraComponent*> ActiveBeams;

    TSet<AActor*> SpawnedBeams;
};

