// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RelicRunners/RelicRunnersCharacter.h" 
#include "AphroditeCharacter.generated.h"

USTRUCT()
struct FBasicProjectile
{
	GENERATED_BODY()

	FVector Location;
	FVector Velocity;
	FVector PreviousLocation;
	float ElapsedTime = 0.f;

	TSet<AActor*> AlreadyHitActors;
};

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
	virtual void ProjectileHeavyAttack() override;
	virtual void UpdateCamera(float DeltaTime) override;

	UPROPERTY()
	bool bCanFire = true;

	UPROPERTY(EditDefaultsOnly)
	float ProjectileSpeed = 1200.f;

	UPROPERTY(EditDefaultsOnly)
	float ProjectileLifetime = 2.0f;

	UPROPERTY(EditDefaultsOnly)
	float ProjectileDamage = 90.f;

	FTimerHandle TimerHandle_AttackCooldown;

	UPROPERTY()
	TArray<FBasicProjectile> ActiveProjectiles;

	bool bIsAiming = false;

	// Camera offset when aiming
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Aiming")
	FVector AimingOffset = FVector(0.f, 0.f, 120.f);

	// Camera distance/speed
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Aiming")
	float AimingInterpSpeed = 8.f;

	float AimingArmLength = 150.f;
};
