// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RelicRunners/RelicRunnersCharacter.h" 
#include "AresCharacter.generated.h"

/**
 * 
 */

UCLASS()
class RELICRUNNERS_API AAresCharacter : public ARelicRunnersCharacter
{
	GENERATED_BODY()

public:
	AAresCharacter();

	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_PlayerState() override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	class UClassInfo* ClassInfo;

	virtual void Server_HitEnemy_Implementation(AActor* HitActor) override;

protected:
	virtual void BeginPlay() override;

	virtual void ActivateDamageAbilities() override;
	virtual void ActivateDefenceAbilities() override;
	virtual void ActivateUtilityAbilities() override;
	virtual void ActivateUltimateAbilities() override;

	void GiveEarthquake();
	void GiveGuard();
	void GiveWarBanner();
	void GiveRageofTheGods();

	virtual void PlayRandomAttackAnimation() override;
	virtual void PerformSwingTrace() override;
	virtual void BasicAttack() override;
	virtual void HeavyAttack() override;
	virtual void UpdateCamera(float DeltaTime) override;
	void DetectEnemies(bool& bEnemiesNearby, AActor*& NearestEnemy, float& NearestDistSq);
	void UpdateCameraPosition(float DeltaTime, const FVector& Velocity, float Speed, bool bEnemiesNearby);
	void UpdateCameraRotation(float DeltaTime, const FVector& Velocity, float Speed);

	UPROPERTY()
	float Damage;

	UPROPERTY(EditDefaultsOnly, Category = "Ares")
	TSubclassOf<UEarthquakeAbility> BP_EarthquakeClass;

	UPROPERTY(EditDefaultsOnly, Category = "Ares")
	TSubclassOf<UGuardAbility> BP_GuardClass;

	UPROPERTY(EditDefaultsOnly, Category = "Ares")
	TSubclassOf<UWarBannerAbility> BP_WarBannerClass;

	UPROPERTY(EditDefaultsOnly, Category = "Ares")
	TSubclassOf<URageOfTheGods> BP_RageOfTheGodsClass;
};
