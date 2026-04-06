// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RelicRunners/AbilitySystem/AbilityBase.h"
#include "Components/SphereComponent.h"
#include "KissOfLife.generated.h"

UCLASS()
class RELICRUNNERS_API UKissOfLife : public UAbilityBase
{
    GENERATED_BODY()

public:
    UKissOfLife();

    virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,const FGameplayAbilityActorInfo* ActorInfo,const FGameplayAbilityActivationInfo ActivationInfo,const FGameplayEventData* TriggerEventData) override;

    virtual void EndAbility(const FGameplayAbilitySpecHandle Handle,const FGameplayAbilityActorInfo* ActorInfo,const FGameplayAbilityActivationInfo ActivationInfo,bool bReplicateEndAbility,bool bWasCancelled) override;

    virtual FName GetAbilityName() const override { return FName("KissOfLife"); }

protected:

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KissOfLife")
    float HealInterval = 1.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KissOfLife")
    float HealPercentPerTick = 0.05f; // +5%/HP 

private:
    FTimerHandle HealTickTimer;
    FTimerHandle EndTimer;

    void ApplyHealTick();
    bool bIsInvincible = false;
    UPROPERTY()
    USphereComponent* DebugHealSphere; 
};