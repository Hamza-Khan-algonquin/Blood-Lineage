#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Sound/SoundBase.h"
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

    UPROPERTY()
    class UNiagaraSystem* BridgeCrumbleFX;

    UPROPERTY(EditAnywhere, Category = "Animations")
    class UAnimMontage* FireballMontage;

    UPROPERTY(EditAnywhere, Category = "Animations")
    class UAnimMontage* IdleMontage;

    UPROPERTY(EditAnywhere, Category = "Animations")
    class UAnimMontage* SlamChargeMontage;

    UPROPERTY(EditAnywhere, Category = "Animations")
    class UAnimMontage* SlamMontage;

    UPROPERTY(EditAnywhere, Category = "Animations")
    class UAnimMontage* DeathMontage;

    UPROPERTY(EditAnywhere, Category = "Animations")
    class UAnimMontage* DamagedMontage;

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

    UPROPERTY(EditAnywhere, Category = "Classes")
    TSubclassOf<AActor> FireballClass;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
    TObjectPtr<USoundBase> SlamImpactSound = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
    TObjectPtr<USoundBase> SweepAttackSound = nullptr;

    UPROPERTY(Replicated)
    EBossPhase CurrentPhase = EBossPhase::Phase1;

    FString LastAttack = "";

    UPROPERTY(Replicated)
    bool bIsBusy = false;      

    UPROPERTY(Replicated)
    bool bIsSweeping = false;     

    UPROPERTY(Replicated)
    bool bIsInitialSlamming = false; 

    float BaseDamageFromDifficulty = 50;

    float MaxHealth = 10000.0f;
    UPROPERTY(ReplicatedUsing = OnRep_CurrentHealth)
    float CurrentHealth = MaxHealth;
    UFUNCTION()
    void OnRep_CurrentHealth();
    int ObelisksHit = 0;

    bool InCinematic = true;
    int DifficultyStrength = 1;

    UPROPERTY(Replicated, BlueprintReadOnly, Category = "AI")
    AActor* CurrentTarget;

    UPROPERTY(EditAnywhere, Category = "Combat")
    float RotationSpeed = 5.0f;
    UPROPERTY(EditAnywhere, Category = "Combat")
    float SweepTotalTime;
    UPROPERTY(EditAnywhere, Category = "Combat")
    float RotationOffset = 90.0f;
    UPROPERTY(Replicated)
    FRotator QuickTurnTarget;

    UPROPERTY(Replicated)
    bool bIsQuickTurning = false;
    FTimerHandle AttackLoopTimer;     
    FTimerHandle AbilityLogicTimer;   
    FTimerHandle RotationLockTimer;  

    TWeakObjectPtr<AActor> ActiveSweepBeam;

    FTimerHandle ShockwaveTimerHandle;
    int ShockwavesSpawned = 0;
    bool bIsSpawningShockwaves = false;

    void ExecuteRepeatedShockwave();

    UFUNCTION(NetMulticast, Unreliable)
    void MulticastPlayAttackMontage(UAnimMontage* Montage, float Speed, float StartPosition);
    UPROPERTY(ReplicatedUsing = OnRep_VortexForce)
    float ReplicatedVortexForce;
    UFUNCTION()
    void OnRep_VortexForce();

    UPROPERTY(EditAnywhere)
    TSubclassOf<class UCameraShakeBase> SlamCameraShake;

    UPROPERTY(EditAnywhere)
    TSubclassOf<class UCameraShakeBase> SpinCameraShake;
    UPROPERTY(EditAnywhere, Category = "Combat")
    TSubclassOf<class AShockwaveRing> ShockwaveBP;
protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    void TriggerAllPlayersCinematicWalk();

    void SpawnShockwave();

    void ExecuteAttack();
    void SpawnBridge();
    void SpawnBlocking();
    void CinematicSlam();
    void ScheduleNextAttack(float Delay);
    void ScheduleCinematicSlam(float Delay);
    void ResetAllStates(); 
    void CheckPhaseTransition();
    void SelectRandomTarget();
    void OnChargeFinished();
    void PlaySlam();
    void PlayFireball();
    void PlaySweep();
    void UpdateSweep(float DeltaTime);
    void UpdateRotation(float DeltaTime);
    void UpdateIdleAnimation();
    void ExecuteDamage(FVector ImpactLocation, float ImpactRadius);
    void LockRotationTo(FVector TargetLocation, bool bInstant);
    UFUNCTION(NetMulticast, Unreliable)
    void MulticastOnBridgeDestroyed(FVector Location);
    UFUNCTION(NetMulticast, Unreliable)
    void MulticastQuickTurn(FRotator NewRotation);
    UFUNCTION(NetMulticast, Unreliable)
    void MulticastSnapRotation(FRotator NewRotation);

    FVector CalculateFireballLocation(FVector Center);

    UPROPERTY(EditAnywhere, Category = "Boss Mechanics")
    TSubclassOf<AActor> BridgeClass;

    UPROPERTY(BlueprintReadOnly, Category = "Boss Mechanics")
    AActor* BridgeToDestroy;

    UPROPERTY(EditAnywhere, Category = "Boss Mechanics")
    TSubclassOf<AActor> BlockingClass;

    UPROPERTY(BlueprintReadOnly, Category = "Boss Mechanics")
    AActor* BlockingToSpawn;

    UPROPERTY(Replicated)
    FVector AbilityLookAtTarget;
    UPROPERTY(Replicated)
    bool bIsLookingAtAbilityTarget = false;
    float SweepStartYaw = 0.0f;
    float SweepCurrentAngle = 0.0f;
    float SweepElapsedTime = 0.0f;
    float SweepDuration = 8.0f;
    float SweepRotationCount = 1.0f;
    void Die();
public:
    void NotifyObeliskHit(AActor* Obelisk);
};