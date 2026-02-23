// Fill out your copyright notice in the Description page of Project Settings.

#include "MarkOfTheHunt.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "RelicRunners/PlayerState/RelicRunnersPlayerState.h"
#include "Kismet/KismetSystemLibrary.h"
#include "DrawDebugHelpers.h"
#include "TimerManager.h"

UMarkOfTheHunt::UMarkOfTheHunt()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

    Duration = 10.f;
    LastUseTime = -100.f;
    CooldownTimer = 10.f;
    Cost = 50;
    AreaRadius = 1000.f;
    TravelTime = 2.f;
    Gravity = -980.f;
}

void UMarkOfTheHunt::ActivateAbility(const FGameplayAbilitySpecHandle Handle,const FGameplayAbilityActorInfo* ActorInfo,const FGameplayAbilityActivationInfo ActivationInfo,const FGameplayEventData* TriggerEventData)
{
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

    if (!ActorInfo || !ActorInfo->AvatarActor.IsValid())
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    UWorld* World = GetWorld();
    if (!World) return;

    if (World->GetTimeSeconds() - LastUseTime < CooldownTimer)
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    // Get PlayerState safely
    ARelicRunnersPlayerState* PS = nullptr;
    if (ActorInfo->PlayerController.IsValid())
    {
        PS = ActorInfo->PlayerController->GetPlayerState<ARelicRunnersPlayerState>();
    }

    // Must have PlayerState and enough resource
    if (!PS || PS->GetPlayerResource() < Cost)
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    // Spend resource
    PS->SetPlayerResource(PS->GetPlayerResource() - Cost);
    PS->UpdateRemoteHUDResource();

    StartLocation = ActorInfo->AvatarActor->GetActorLocation() + FVector(0, 0, 50.f);

    // Get forward vector
    FVector Forward = ActorInfo->AvatarActor->GetActorForwardVector();

    // Angle it upward
    float LaunchAngle = 2.f; // degrees
    FRotator ForwardRot = Forward.Rotation();
    ForwardRot.Pitch += LaunchAngle; // rotate up
    FVector AdjustedForward = ForwardRot.Vector();

    // Set initial velocity
    float LaunchSpeed = 3800.f;
    InitialVelocity = AdjustedForward * LaunchSpeed;

    ProjectileAlpha = 0.f;

    // Tick projectile every frame
    World->GetTimerManager().SetTimer(ProjectileTickTimer,this,&UMarkOfTheHunt::ProjectileTick,0.02f,true);

    LastUseTime = World->GetTimeSeconds();
}

void UMarkOfTheHunt::ProjectileTick()
{
    UWorld* World = GetWorld();
    if (!World) return;

    ProjectileAlpha += 0.02f;

    FVector PreviousLocation = StartLocation;
    FVector CurrentLocation = StartLocation + InitialVelocity * ProjectileAlpha;
    CurrentLocation.Z += 0.5f * Gravity * ProjectileAlpha * ProjectileAlpha;

    // Draw projectile
    DrawDebugSphere(World, CurrentLocation, 12.f, 8, FColor::Cyan, false, 0.05f);

    // Trace from previous position to current to check for ground collision
    FHitResult Hit;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(GetAvatarActorFromActorInfo());

    bool bHit = World->LineTraceSingleByChannel(Hit,PreviousLocation,CurrentLocation,ECC_WorldStatic,Params);

    if (bHit)
    {
        // Stop projectile at impact
        World->GetTimerManager().ClearTimer(ProjectileTickTimer);
        EndLocation = Hit.ImpactPoint;
        OnProjectileLanded();
    }
}


void UMarkOfTheHunt::OnProjectileLanded()
{
    UWorld* World = GetWorld();
    if (!World) return;

    // Impact visuals
    DrawDebugSphere(World, EndLocation, 25.f, 12, FColor::Red, false, 3.f);

    ScanForEnemies(EndLocation);
    
}

void UMarkOfTheHunt::ScanForEnemies(const FVector& ScanLocation)
{
    UWorld* World = GetWorld();
    if (!World) return;

    // Draw a green sphere to show the scan radius
    DrawDebugSphere(World, ScanLocation, AreaRadius, 32, FColor::Green, false, 3.f, 0, 2.f);

    // Timer handle for ticking spheres
    FTimerHandle EnemySphereTimerHandle;

    // Start ticking spheres 
    World->GetTimerManager().SetTimer(EnemySphereTimerHandle,[World, ScanLocation]()
        {
            TArray<AActor*> FoundActors;
            TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
            ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));
            ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_GameTraceChannel1));

            // Overlap check every tick
            UKismetSystemLibrary::SphereOverlapActors(World,ScanLocation,1000.f, ObjectTypes, APawn::StaticClass(), {}, FoundActors);

            for (AActor* Actor : FoundActors)
            {
                if (Actor && Actor->ActorHasTag("Enemy"))
                {
                    // Draw red debug sphere at enemy's current location
                    DrawDebugSphere(World, Actor->GetActorLocation(), 30.f, 12, FColor::Red, false, 0.02f); }
            }}, 0.02f, true);

    // Timer to stop ticking after ability duration
    FTimerHandle StopTimerHandle;

    World->GetTimerManager().SetTimer( StopTimerHandle,[World, EnemySphereTimerHandle]() mutable {World->GetTimerManager().ClearTimer(EnemySphereTimerHandle);},Duration,false);

    // End the ability immediately after scan starts
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}



void UMarkOfTheHunt::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);

    if (ActorInfo && ActorInfo->AvatarActor.IsValid())
    {
        ARelicRunnersPlayerController* RPC = Cast<ARelicRunnersPlayerController>(ActorInfo->PlayerController.Get());

        UWorld* World = ActorInfo->AvatarActor->GetWorld();
        if (World)
        {
            if (!bWasCancelled)
            {
                LastUseTime = World->GetTimeSeconds();

                if (ActorInfo->IsLocallyControlled())
                {
                    if (RPC)
                    {
                        RPC->PlayerHUD->StartUtilityCooldown(CooldownTimer);

                    }
                }
            }
        }
    }
}
