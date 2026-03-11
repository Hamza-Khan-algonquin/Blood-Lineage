// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HadesPredictionBase.generated.h"

UCLASS()
class RELICRUNNERS_API AHadesPredictionBase : public AActor
{
    GENERATED_BODY()

public:
    AHadesPredictionBase();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Replication")
    bool bShouldScale = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Replication")
    float Duration = 1.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Replication")
    float ElapsedTime = 0.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Replication")
    FVector TargetScale = FVector(1.0f);

protected:
    virtual void Tick(float DeltaTime) override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
