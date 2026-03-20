#pragma once

#include "CoreMinimal.h"
#include "RelicRunners/RelicRunnersCharacter.h" 
#include "Sound/SoundBase.h"
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


protected:
    virtual void BeginPlay() override;
	void Tick(float DeltaTime);
	virtual void ActivateDamageAbilities() override;
	virtual void ActivateDefenceAbilities() override;
	virtual void ActivateUtilityAbilities() override;
	virtual void ActivateUltimateAbilities() override;

	void GivePiercingArrow();
	void GiveNaturesBlessing();
	void GiveMarkOfTheHunt();
	void GiveMoonBeam(); 
   
	void DetectEnemies(bool& bEnemiesNearby, AActor*& NearestEnemy, float& NearestDistSq);
	void UpdateCameraPosition(float DeltaTime, const FVector& Velocity, float Speed, bool bEnemiesNearby);
	void UpdateCameraRotation(float DeltaTime, const FVector& Velocity, float Speed);

    virtual void PlayRandomAttackAnimation() override;
	virtual void PerformSwingTrace() override;
	void ShowReticle();
	void HideReticle();
    virtual void BasicAttack() override;
	virtual void HeavyAttack() override;
	virtual void ProjectileHeavyAttack() override;
	virtual void UpdateCamera(float DeltaTime) override;

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

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Audio")
	TObjectPtr<USoundBase> BasicAttackSound = nullptr;

	FVector PreviousProjectileLocation;

	FTimerHandle TimerHandle_AttackCooldown;

	float ProjectileGravity = 400.f;

	UPROPERTY(EditAnywhere, Category = "Combat")
	UStaticMesh* ProjectileMeshAsset = nullptr; 

	UStaticMeshComponent* ActiveProjectileMesh = nullptr;

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

	bool bExitingAim = false;

	int32 MaxPierces = 10;
	int32 CurrentPierces = 0;

	UPROPERTY(EditDefaultsOnly, Category = "Artemis")
	TSubclassOf<UPiercingArrow> BP_PiercingArrowClass;

	UPROPERTY(EditDefaultsOnly, Category = "Artemis")
	TSubclassOf<UImpunityAbility> BP_ImpunityClass;

	UPROPERTY(EditDefaultsOnly, Category = "Artemis")
	TSubclassOf<UMarkOfTheHunt> BP_MarkOfTheHuntClass;

	UPROPERTY(EditDefaultsOnly, Category = "Artemis")
	TSubclassOf<UMoonbeam> BP_MoonbeamClass;
};
