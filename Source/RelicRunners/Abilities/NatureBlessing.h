// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RelicRunners/AbilitySystem/AbilityBase.h"

#include "NatureBlessing.generated.h"

UCLASS()
class RELICRUNNERS_API UNaturesBlessing : public UAbilityBase
{
	GENERATED_BODY()

public:
	UNaturesBlessing();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

	virtual FName GetAbilityName() const override { return FName("NaturesBlessing"); }

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NatureBlessing")
	float InvincibleDuration = 3.0f;

private:
	FTimerHandle EndTimer;
};