// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RelicRunners/AbilitySystem/AbilityBase.h"
#include "RageOfTheGods.generated.h"


UCLASS()
class RELICRUNNERS_API URageOfTheGods : public UAbilityBase
{
	GENERATED_BODY()

public:
	URageOfTheGods();
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	virtual FName GetAbilityName() const override { return FName("Rage Of The Gods"); }

protected:
	void StartPulse();
	void ExpandPulse();

	FTimerHandle PulseTimerHandle;
	FTimerHandle ExpansionTimerHandle;

	TSet<TWeakObjectPtr<AActor>> HitActorsThisPulse;

	float PulseElapsedTime = 0.f;
	float ExpansionElapsedTime = 0.f;

	int32 PulsesExecuted = 0;

	float PulseInterval = 2.f;
	float PulseDuration = 4.f;
	float TotalDuration = 10.f;

	float StartRadius = 200.f;
	float MaxRadius = 1000.f;

	float KnockbackStrength = 5000.f;

};
