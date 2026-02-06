#pragma once

#include "CoreMinimal.h"
#include "RelicRunners/RelicRunnersCharacter.h" 
#include "ArtemisCharacter.generated.h"

/**
 *
 */

UCLASS()
class RELICRUNNERS_API AArtemisCharacter : public ARelicRunnersCharacter
{
    GENERATED_BODY()

public:
    AArtemisCharacter();

    virtual void PossessedBy(AController* NewController) override;
    virtual void OnRep_PlayerState() override;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    class UClassInfo* ClassInfo;

	virtual void Server_HitEnemy_Implementation(AActor* HitActor) override;

protected:
    virtual void BeginPlay() override;
	void Tick(float DeltaTime);
    virtual void GiveDamageAbilities() override;
    virtual void GiveDefenceAbilities() override;
    virtual void GiveUtilityAbilities() override;
    virtual void GiveUltimateAbilities() override;
   
    virtual void PlayRandomAttackAnimation() override;
    virtual void BasicAttack() override;

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

	FVector PreviousProjectileLocation;

	FTimerHandle TimerHandle_AttackCooldown;

	float ProjectileGravity = 400.f;

	UPROPERTY(EditAnywhere, Category = "Combat")
	UStaticMesh* ProjectileMeshAsset = nullptr; 

	UStaticMeshComponent* ActiveProjectileMesh = nullptr;

};
