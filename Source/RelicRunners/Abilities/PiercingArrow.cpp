// Fill out your copyright notice in the Description page of Project Settings.


#include "PiercingArrow.h"
#include "GameFramework/Character.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "RelicRunners/PlayerState/RelicRunnersPlayerState.h"
#include "RelicRunners/Enemy/Characters/EnemyCharacter.h"
#include "RelicRunners/Enemy/Characters/EnemyCharacterAI.h"
#include "RelicRunners/Enemy/EnemyPawn.h"
#include "DrawDebugHelpers.h"
#include "TimerManager.h"


UPiercingArrow::UPiercingArrow()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

    Duration = 10.f;    
    CooldownTimer = 10.f;
    Cost = 25;
    LastUseTime = -100.f;
    AreaRadius = 600.f;

    Gravity = -980.f;
    ProjectileRadius = 40.f;
    DotTickRate = 0.5f;
    DotDuration = 6.f;
    DamagePerTick = 15.f;
    DotTimeElapsed = 0.f;
}

void UPiercingArrow::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

    AActor* AvatarActor = GetAvatarActorFromActorInfo();
    if (!AvatarActor)
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    UWorld* World = AvatarActor->GetWorld();
    if (!World)
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    float CurrentTime = World->GetTimeSeconds();
    if (CurrentTime - LastUseTime < CooldownTimer)
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    ARelicRunnersPlayerState* PS = nullptr;
    if (ActorInfo->PlayerController.IsValid())
    {
        PS = ActorInfo->PlayerController->GetPlayerState<ARelicRunnersPlayerState>();
    }

    if (!PS || PS->GetPlayerResource() < Cost)
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    PS->SetPlayerResource(PS->GetPlayerResource() - Cost);
    PS->UpdateRemoteHUDResource();



    // Initial projectile setup
    ProjectileLocation = AvatarActor->GetActorLocation() + AvatarActor->GetActorForwardVector() * 100.f + FVector(0, 0, 50.f);

    // Arc trajectory
    ProjectileVelocity = AvatarActor->GetActorForwardVector() * 3200.f + FVector(0, 0, 100.f);

    // Start ticking projectile
    World->GetTimerManager().SetTimer(ProjectileTimer, this, &UPiercingArrow::TickArrow, 0.016f, true);

    // Duration failsafe 
    World->GetTimerManager().SetTimer(DurationTimer,[this](){Explode(ProjectileLocation);},3.f,false);
}

void UPiercingArrow::TickArrow()
{
    AActor* AvatarActor = GetAvatarActorFromActorInfo();
    if (!AvatarActor) return;

    UWorld* World = AvatarActor->GetWorld();
    if (!World) return;

    const float DeltaTime = 0.016f;

    // Apply gravity
    ProjectileVelocity.Z += Gravity * DeltaTime;

    FVector NewLocation = ProjectileLocation + ProjectileVelocity * DeltaTime;

    FHitResult Hit;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(AvatarActor);

    bool bHit = World->SweepSingleByChannel(Hit,ProjectileLocation,NewLocation,FQuat::Identity, ECC_GameTraceChannel1, FCollisionShape::MakeSphere(ProjectileRadius),Params);

    //debug sphere at the projectile location
    DrawDebugSphere(World, NewLocation, ProjectileRadius, 12, FColor::Yellow, false, 0.1f);

    if (bHit)
    {
        // debug sphere
        DrawDebugSphere(World, Hit.ImpactPoint, AreaRadius, 16, FColor::Red, false, 2.f);

        Explode(Hit.ImpactPoint);
        return;
    }

    ProjectileLocation = NewLocation;
}

void UPiercingArrow::Explode(FVector ImpactPoint)
{
    AActor* AvatarActor = GetAvatarActorFromActorInfo();
    if (!AvatarActor) return;

    UWorld* World = AvatarActor->GetWorld();
    if (!World) return;

    World->GetTimerManager().ClearTimer(ProjectileTimer);
    World->GetTimerManager().ClearTimer(DurationTimer);

    DotCenter = ImpactPoint;
    DotTimeElapsed = 0.f;

    // Draw debug explosion field
    DrawDebugSphere(World, DotCenter, AreaRadius, 16, FColor::Red, false, DotDuration);

    // Start DoT ticking
    World->GetTimerManager().SetTimer(DotTimer,this,&UPiercingArrow::ApplyDotTick,DotTickRate,true);
}

void UPiercingArrow::ApplyDotTick()
{
    AActor* AvatarActor = GetAvatarActorFromActorInfo();
    if (!AvatarActor) return;

    UWorld* World = AvatarActor->GetWorld();
    if (!World) return;

    DotTimeElapsed += DotTickRate;

    TArray<FHitResult> HitResults;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(AvatarActor);

    bool bHitAny = World->SweepMultiByChannel(HitResults, DotCenter, DotCenter + FVector(0, 0, 1.f), FQuat::Identity,ECC_GameTraceChannel1, FCollisionShape::MakeSphere(AreaRadius), Params);

    if (bHitAny)
    {
        for (const FHitResult& Hit : HitResults)
        {
            APawn* Pawn = Cast<APawn>(Hit.GetActor());
            if (!Pawn) continue;

            if (!Pawn->Tags.Contains("Enemy")) continue;

            UGameplayStatics::ApplyDamage(Pawn,DamagePerTick, AvatarActor->GetInstigatorController(), AvatarActor, nullptr);
        }
    }

    // End DoT after duration
    if (DotTimeElapsed >= DotDuration)
    {
        World->GetTimerManager().ClearTimer(DotTimer);

        EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
    }
}

void UPiercingArrow::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
    if (ActorInfo && ActorInfo->AvatarActor.IsValid())
    {
        UWorld* World = ActorInfo->AvatarActor->GetWorld();
        if (World)
        {
            World->GetTimerManager().ClearTimer(ProjectileTimer);
            World->GetTimerManager().ClearTimer(DurationTimer);
            World->GetTimerManager().ClearTimer(DotTimer);
            if (!bWasCancelled)
            {
                LastUseTime = World->GetTimeSeconds();

                if (ActorInfo->IsLocallyControlled())
                {
                    if (ARelicRunnersPlayerController* RPC =
                        Cast<ARelicRunnersPlayerController>(ActorInfo->PlayerController.Get()))
                    {
                        RPC->PlayerHUD->StartDamageCooldown(CooldownTimer);
                    }
                }
            }
        }
    }

    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
