// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HadesPredictionBase.h"
#include "HadesSweepPrediction.generated.h"

UCLASS()
class RELICRUNNERS_API AHadesSweepPrediction : public AHadesPredictionBase
{
    GENERATED_BODY()

public:
    AHadesSweepPrediction();

    UPROPERTY(Replicated, BlueprintReadWrite, Category = "Sweep")
    bool bIsRotating = false;

    UPROPERTY(Replicated, BlueprintReadWrite, Category = "Sweep")
    int RotationCount = 1;

    UPROPERTY(Replicated, BlueprintReadWrite, Category = "Sweep")
    float StartYaw = 0.0f;

protected:
    virtual void Tick(float DeltaTime) override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
    float ElapsedTime = 0.0f;
};