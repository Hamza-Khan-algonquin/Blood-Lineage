// Fill out your copyright notice in the Description page of Project Settings.


#include "HadesFireball.h"
#include "NiagaraComponent.h"
#include <Net/UnrealNetwork.h>
#include "HadesFireballPrediction.h"
#include <Kismet/GameplayStatics.h>
#include <NiagaraFunctionLibrary.h>

AHadesFireball::AHadesFireball()
{
    PrimaryActorTick.bCanEverTick = true;
    bReplicates = true;
    SetReplicateMovement(true);

    FireballVFX = CreateDefaultSubobject<UNiagaraComponent>(TEXT("FireballVFX"));
    RootComponent = FireballVFX;
}

void AHadesFireball::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AHadesFireball, TargetLocation);
    DOREPLIFETIME(AHadesFireball, FlightDuration);
}

void AHadesFireball::Multicast_PlayExplosionEffects_Implementation(FVector Location)
{
    if (ExplosionEffect)
    {
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), ExplosionEffect, Location, FRotator::ZeroRotator);
    }
}

void AHadesFireball::BeginPlay()
{
    Super::BeginPlay();
    StartingLocation = GetActorLocation();

    if (HasAuthority() && PredictionClass && TargetLocation != FVector::ZeroVector)
    {
        FActorSpawnParameters Params;
        Params.Owner = GetOwner();

        InternalPrediction = GetWorld()->SpawnActor<AHadesFireballPrediction>(
            PredictionClass, TargetLocation, FRotator::ZeroRotator, Params);

        if (InternalPrediction)
        {
            InternalPrediction->Duration = FlightDuration;
            InternalPrediction->TargetScale = FVector(4.0f, 4.0f, 1.0f);
            InternalPrediction->bShouldScale = true; 
        }
    }
}

void AHadesFireball::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (GetAttachParentActor())
    {
        StartingLocation = GetActorLocation();
        return;
    }

    ElapsedTime += DeltaTime;
    float Alpha = FMath::Clamp(ElapsedTime / FlightDuration, 0.0f, 1.0f);

    FVector CurrentPos = FMath::Lerp(StartingLocation, TargetLocation, Alpha);

    SetActorLocation(CurrentPos);

    FVector Dir = (TargetLocation - StartingLocation).GetSafeNormal();
    if (!Dir.IsNearlyZero())
    {
        SetActorRotation(Dir.Rotation());
    }

    if (Alpha >= 1.0f && HasAuthority())
    {
        //DrawDebugSphere(GetWorld(), TargetLocation, 400.0f, 32, FColor::Orange, false, 2.0f);
        UGameplayStatics::ApplyRadialDamage(GetWorld(), 80.0f, TargetLocation, 400.0f, nullptr, { GetOwner() }, this);

        Multicast_PlayExplosionEffects(TargetLocation);

        if (FireBallCameraShake)
            GetWorld()->GetFirstPlayerController()->PlayerCameraManager->PlayWorldCameraShake(GetWorld(), FireBallCameraShake, GetActorLocation(), 10000.0f, 10000.0f, 1.0f);

        if (InternalPrediction) InternalPrediction->Destroy();
        Destroy();
    }
}