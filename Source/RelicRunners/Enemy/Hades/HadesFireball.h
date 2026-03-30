// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Sound/SoundBase.h"
#include "HadesFireball.generated.h"

UCLASS()
class RELICRUNNERS_API AHadesFireball : public AActor
{
    GENERATED_BODY()

public:
    AHadesFireball();

    UPROPERTY(Replicated, BlueprintReadWrite, Category = "Combat")
    FVector TargetLocation;

    UPROPERTY(Replicated, BlueprintReadWrite, Category = "Combat")
    float FlightDuration = 2.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Effects")
    class UNiagaraComponent* FireballVFX;

    UPROPERTY(EditDefaultsOnly, Category = "Effects")
    class UNiagaraSystem* ExplosionEffect;

    UPROPERTY(EditAnywhere, Category = "Fireball")
    TSubclassOf<class AHadesPredictionBase> PredictionClass;

    UPROPERTY()
    class AHadesPredictionBase* InternalPrediction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
    TObjectPtr<USoundBase> SpawnSound = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
    TObjectPtr<USoundBase> ImpactSound = nullptr;

    UFUNCTION(NetMulticast, Reliable)
    void Multicast_PlayExplosionEffects(FVector Location);

    UPROPERTY(EditAnywhere)
    TSubclassOf<class UCameraShakeBase> FireBallCameraShake;

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
    float ElapsedTime = 0.0f;
    FVector StartingLocation;
};