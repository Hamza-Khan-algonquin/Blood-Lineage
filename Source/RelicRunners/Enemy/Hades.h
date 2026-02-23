#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Hades.generated.h"

UENUM(BlueprintType)
enum class EBossPhase : uint8
{
    Phase1, Phase2, Phase3, Phase4
};

UCLASS()
class RELICRUNNERS_API AHades : public AActor
{
    GENERATED_BODY()

public:
    AHades();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USkeletalMeshComponent* SkeletalMesh;

    UPROPERTY(EditAnywhere, Category = "Effects")
    class UNiagaraSystem* GhostCycloneSystem;

    UPROPERTY()
    class UNiagaraComponent* ActiveCycloneComp;

    UPROPERTY(EditAnywhere, Category = "Animations")
    class UAnimMontage* FireballMontage;

    UPROPERTY(EditAnywhere, Category = "Animations")
    class UAnimMontage* IdleMontage;

    UPROPERTY(EditAnywhere, Category = "Animations")
    class UAnimMontage* SlamMontage;

    UPROPERTY(EditAnywhere, Category = "Animations")
    class UAnimMontage* SweepMontage;

    UPROPERTY(EditAnywhere, Category = "Classes")
    TSubclassOf<AActor> FireballPredictionClass;

    UPROPERTY(EditAnywhere, Category = "Classes")
    TSubclassOf<AActor> SlamPredictionClass;

    UPROPERTY(EditAnywhere, Category = "Classes")
    TSubclassOf<AActor> SlamSweepPredictionClass;

    UPROPERTY(EditAnywhere, Category = "Classes")
    TSubclassOf<AActor> SweepPredictionClass;

    EBossPhase CurrentPhase = EBossPhase::Phase1;

    bool bIsBusy = false;           
    bool bIsSweeping = false;       
    bool bIsInitialSlamming = false; 

    float MaxHealth = 10000.0f;
    float CurrentHealth = MaxHealth;
    int32 ObelisksHit = 0;

    UPROPERTY(BlueprintReadOnly, Category = "AI")
    AActor* CurrentTarget;

    UPROPERTY(EditAnywhere, Category = "Combat")
    float RotationSpeed = 5.0f;

    UPROPERTY(EditAnywhere, Category = "Combat")
    float RotationOffset = 90.0f;

    FTimerHandle AttackLoopTimer;     
    FTimerHandle AbilityLogicTimer;   
    FTimerHandle RotationLockTimer;  

    TWeakObjectPtr<AActor> ActiveSweepBeam;

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    void ExecuteAttack();
    void ScheduleNextAttack(float Delay);
    void ResetAllStates(); 
    void CheckPhaseTransition();
    void SelectRandomTarget();
    void PlaySlam();
    void PlayFireball();
    void PlaySweep();
    void UpdateSweep(float DeltaTime);
    void UpdateRotation(float DeltaTime);
    void UpdateIdleAnimation();
    void ExecuteDamage(FVector ImpactLocation, float ImpactRadius);
    void LockRotationTo(FVector TargetLocation);
    FVector CalculateFireballLocation(FVector Center);
    void SetupPredictionScaling(AActor* Prediction, float Duration, FVector MaxScale);
    AActor* SpawnPrediction(TSubclassOf<AActor> Class, FVector Location, float Duration, FVector Scale);

    FVector AbilityLookAtTarget;
    bool bIsLookingAtAbilityTarget = false;
    float SweepStartYaw = 0.0f;
    float SweepCurrentAngle = 0.0f;
    float SweepElapsedTime = 0.0f;
    float SweepDuration = 8.0f;

    void Die();
public:
    void NotifyObeliskHit(AActor* Obelisk);
};