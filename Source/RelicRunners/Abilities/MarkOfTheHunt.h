// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RelicRunners/AbilitySystem/AbilityBase.h"
#include "MarkOfTheHunt.generated.h"

struct FMarkProjectile
{
	FVector Location;
	FVector Velocity;
	FVector PreviousLocation;

	float Elapsed = 0.f;

	UStaticMeshComponent* Mesh = nullptr;
	TArray<AActor*> HitActors;
};

UCLASS()
class RELICRUNNERS_API UMarkOfTheHunt : public UAbilityBase
{
	GENERATED_BODY()

public:
	UMarkOfTheHunt();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

private:

	void ProjectileTick();

	UPROPERTY(EditAnywhere, Category = "Combat")
	UStaticMesh* ProjectileMeshAsset = nullptr;

	UStaticMeshComponent* ActiveProjectileMesh = nullptr;

	TArray<FMarkProjectile> ActiveProjectiles;
	int MaxPierces = 10;
	int NumProjectiles = 10;
	float ProjectileSpeed = 3800.f;
	float ProjectileGravity = 980.f;
	float ProjectileLifetime = 3.f;

	FTimerHandle ProjectileTickTimer;
	FTimerHandle DurationTimer;
};