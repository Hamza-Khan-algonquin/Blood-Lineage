// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RelicRunners/AbilitySystem/AbilityBase.h"
#include "MarkOfTheHunt.generated.h"


UCLASS()
class RELICRUNNERS_API UMarkOfTheHunt : public UAbilityBase
{
	GENERATED_BODY()

public:
	UMarkOfTheHunt();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

private:

	void OnProjectileLanded();
	void ScanForEnemies(const FVector& ScanLocation);
	void ProjectileTick();

	FVector StartLocation;
	FVector EndLocation;
	float ProjectileAlpha;
	FTimerHandle TravelTimer;
	FTimerHandle ProjectileTickTimer;
	FVector InitialVelocity;
	UPROPERTY(EditDefaultsOnly, Category = "Ability")
	float TravelTime;

	UPROPERTY(EditDefaultsOnly, Category = "Ability|Projectile")
	float Gravity;
};