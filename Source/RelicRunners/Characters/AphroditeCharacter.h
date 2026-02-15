// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RelicRunners/RelicRunnersCharacter.h" 
#include "AphroditeCharacter.generated.h"

/**
 * 
 */
UCLASS()
class RELICRUNNERS_API AAphroditeCharacter : public ARelicRunnersCharacter
{
	GENERATED_BODY()

public:
	AAphroditeCharacter();

	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_PlayerState() override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	class UClassInfo* ClassInfo;

	virtual void Server_HitEnemy_Implementation(AActor* HitActor) override;

protected:
	virtual void BeginPlay() override;
	void Tick(float DeltaTime);
	virtual void ActivateDamageAbilities() override;
	virtual void ActivateDefenceAbilities() override;
	virtual void ActivateUtilityAbilities() override;
	virtual void ActivateUltimateAbilities() override;

	void GiveBundleOfJoy();
	void GiveMirrorOfDesire();
	void GiveKissOfLife();
	void GiveGoddessOfLove();

	virtual void PlayRandomAttackAnimation() override;
	virtual void PerformSwingTrace() override;
	virtual void BasicAttack() override;
	virtual void HeavyAttack() override;

	UPROPERTY()
	bool bCanFire = true;

	UPROPERTY()
	FVector ProjectileLocation;

	UPROPERTY()
	FVector ProjectileVelocity;

	UPROPERTY()
	bool bProjectileActive;

	UPROPERTY()
	float ProjectileSpeed; 

	UPROPERTY()
	float ProjectileLifetime; 

	UPROPERTY()
	float ProjectileElapsed = 0.f;

	UPROPERTY()
	float ProjectileDamage;

	FVector PreviousProjectileLocation;

	FTimerHandle TimerHandle_AttackCooldown;

};
