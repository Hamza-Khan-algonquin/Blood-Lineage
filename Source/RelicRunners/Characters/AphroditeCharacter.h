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


protected:
	virtual void BeginPlay() override;
	void Tick(float DeltaTime);
	void ExplodeProjectile(const FVector& ImpactPoint);
	virtual void ActivateDamageAbilities() override;
	virtual void ActivateDefenceAbilities() override;
	virtual void ActivateUtilityAbilities() override;
	virtual void ActivateUltimateAbilities() override;

	void GiveBundleOfJoy();
	void GiveMirrorOfDesire();
	void GiveKissOfLife();
	void GiveGoddessOfLove();
	void ShowReticle();
	void HideReticle();

	void DetectEnemies(bool& bEnemiesNearby, AActor*& NearestEnemy, float& NearestDistSq);
	void UpdateCameraPosition(float DeltaTime, const FVector& Velocity, float Speed, bool bEnemiesNearby);
	void UpdateCameraRotation(float DeltaTime, const FVector& Velocity, float Speed);

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

	UPROPERTY(EditAnywhere, Category = "Projectile")
	float ExplosionRadius = 100.f;

	FTimerHandle TimerHandle_AttackCooldown;

	UPROPERTY()
	TArray<FBasicProjectile> ActiveProjectiles;

	bool bIsAiming = false;

	// Camera offset when aiming
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Aiming")
	FVector AimingOffset = FVector(-20.f, 80.f, 100.f);

	// Camera distance/speed
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Aiming")
	float AimingInterpSpeed = 8.f;

	float AimingArmLength = 100.f;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> ReticleWidgetClass;

	UPROPERTY()
	UUserWidget* ReticleWidget;

	UPROPERTY(EditDefaultsOnly, Category = "Aphrodite")
	TSubclassOf<UBundleOfJoy> BP_BundleOfJoyClass;

	UPROPERTY(EditDefaultsOnly, Category = "Aphrodite")
	TSubclassOf<UMirrorOfDesireAbility> BP_MirrorOfDesireClass;

	UPROPERTY(EditDefaultsOnly, Category = "Aphrodite")
	TSubclassOf<UKissOfLife> BP_KissOfLifeClass;

	UPROPERTY(EditDefaultsOnly, Category = "Aphrodite")
	TSubclassOf<UGoddessOfLove> BP_GoddessOfLoveClass;
};
