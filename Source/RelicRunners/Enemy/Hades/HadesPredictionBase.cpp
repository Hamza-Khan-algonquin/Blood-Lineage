// Fill out your copyright notice in the Description page of Project Settings.


#include "HadesPredictionBase.h"
#include "Net/UnrealNetwork.h"

AHadesPredictionBase::AHadesPredictionBase()
{
    PrimaryActorTick.bCanEverTick = true;
    bReplicates = true;
    bAlwaysRelevant = true;

    SetActorScale3D(FVector(0.01f, 0.01f, 1.0f));
}

void AHadesPredictionBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AHadesPredictionBase, Duration);
    DOREPLIFETIME(AHadesPredictionBase, ElapsedTime);
    DOREPLIFETIME(AHadesPredictionBase, TargetScale);
    DOREPLIFETIME(AHadesPredictionBase, bShouldScale);
}

void AHadesPredictionBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bShouldScale)
    {
        ElapsedTime += DeltaTime;
        float Alpha = FMath::Clamp(ElapsedTime / Duration, 0.0f, 1.0f);

        FVector NewScale = FMath::Lerp(FVector(0.01f, 0.01f, 1.0f), TargetScale, Alpha);
        SetActorScale3D(NewScale);

        if (Alpha >= 1.0f)
        {
            bShouldScale = false;
            PrimaryActorTick.SetTickFunctionEnable(false);
        }
    }
    else if(GetActorScale3D() != TargetScale)
    {
        SetActorScale3D(TargetScale);
    }
}
