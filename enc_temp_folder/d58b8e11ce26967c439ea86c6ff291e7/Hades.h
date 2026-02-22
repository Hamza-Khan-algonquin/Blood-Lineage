// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Hades.generated.h"

UENUM(BlueprintType)
enum class EBossPhase : uint8
{
	Phase1,
	Phase2,
	Phase3,
	Phase4      
};

UCLASS()
class RELICRUNNERS_API AHades : public AActor
{
	GENERATED_BODY()
	
public:	
	AHades();

	UPROPERTY(EditAnywhere, Category = "Effects")
	class UNiagaraSystem* GhostCycloneSystem;

	UPROPERTY()
	class UNiagaraComponent* ActiveCycloneComp;

	EBossPhase CurrentPhase;
	float MaxHealth = 10000.0f;
	float CurrentHealth = MaxHealth;

	FTimerHandle AttackTimerHandle;

	void CheckPhaseTransition();
	void ExecuteAttack();
	void ScheduleNextAttack(float Delay);


	void PlaySlam();
	void ExecuteSlamImpact(FVector ImpactLocation, float ImpactRadius);
	void PlaySweep();
	void UpdateSweep(float DeltaTime);
	void PlayFireball();
	void SpawnScatterFireball(FVector TargetLocation);

	UPROPERTY(BlueprintReadOnly, Category = "Hades|AI", meta = (AllowPrivateAccess = "true"))
	AActor* CurrentTarget;

	float RotationSpeed = 5.0f;
	void SelectRandomTarget();
	FVector AbilityLookAtTarget;
	bool bIsLookingAtAbilityTarget = false;
	FTimerHandle SlamDelayTimerHandle;
	FVector DelayedSlamLocation;
	float SlamRadius;

	float RotationOffset = 90.0f;

	bool bIsSweeping = false;
	float SweepCurrentAngle = 0.0f;
	float SweepSpeed = 0.0f;
	float ArenaRadius = 2500.0f;
	bool bIsInitialSlamming = false;

	FTimerHandle FireballDelayTimerHandle;
	FVector DelayedFireballLocation;
	float FireballRadius = 300.0f;
	float SweepStartYaw = 0.0f;
	float SweepElapsedTime = 0.0f;
	float SweepDuration = 5.0f;

	int ObelisksHit = 0;
protected:
	virtual void BeginPlay() override;
	void Die();
public:	
	virtual void Tick(float DeltaTime) override;
	void NotifyObeliskHit(AActor* Obelisk);

};
