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

    Duration = 2.f;
    LastUseTime = -100.f;
    CooldownTimer = 10.f;
    Cost = 50;
	ConeAngle = 40.f;
    Damage = 100.f;
}

void UMarkOfTheHunt::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
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

    AActor* AvatarActor = ActorInfo ? ActorInfo->AvatarActor.Get() : nullptr;
    ACharacter* OwnerChar = Cast<ACharacter>(AvatarActor);
    if (!OwnerChar)
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }


    if (OwnerChar->GetWorld()->GetTimeSeconds() - LastUseTime < CooldownDuration)
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    ARelicRunnersPlayerState* PlayerState = OwnerChar->GetPlayerState<ARelicRunnersPlayerState>();

    if (!PlayerState || PlayerState->GetPlayerResource() < Cost)
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }


    PlayerState->SetPlayerResource(PlayerState->GetPlayerResource() - Cost);
    PlayerState->UpdateRemoteHUDResource();

    AActor* Avatar = ActorInfo->AvatarActor.Get();

    FVector BaseLocation = Avatar->GetActorLocation() + Avatar->GetActorForwardVector() * 100.f + FVector(0, 0, 50);
    FVector Forward = Avatar->GetActorForwardVector();

    ActiveProjectiles.Empty();

    for (int i = 0; i < NumProjectiles; i++)
    {
        float Alpha = (float)i / (NumProjectiles - 1);
        float YawOffset = FMath::Lerp(-ConeAngle * 0.5f, ConeAngle * 0.5f, Alpha);

        FRotator Rot = Forward.Rotation();
        Rot.Yaw += YawOffset;

        FMarkProjectile P;

        P.Location = BaseLocation;
        P.PreviousLocation = BaseLocation;
        P.Velocity = Rot.Vector() * ProjectileSpeed;

        // Spawn mesh
        if (ProjectileMeshAsset)
        {
            P.Mesh = NewObject<UStaticMeshComponent>(Avatar);
            P.Mesh->RegisterComponent();
            P.Mesh->SetStaticMesh(ProjectileMeshAsset);
            P.Mesh->SetWorldLocation(P.Location);
            P.Mesh->SetWorldRotation(P.Velocity.Rotation());
            P.Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        }

        ActiveProjectiles.Add(P);
    }

    if (AbilityAnim)
    {
        PlayAbilityAnimation(CurrentActorInfo, AbilityAnim);
    }

    World->GetTimerManager().SetTimer(ProjectileTickTimer,this, &UMarkOfTheHunt::ProjectileTick,0.02f,true);

    World->GetTimerManager().SetTimer(DurationTimer, [this, Handle, ActorInfo, ActivationInfo](){EndAbility(Handle, ActorInfo, ActivationInfo, false, false);},Duration,false);

    LastUseTime = World->GetTimeSeconds();
}

void UMarkOfTheHunt::ProjectileTick()
{
    UWorld* World = GetWorld();
    if (!World) return;

    for (int i = ActiveProjectiles.Num() - 1; i >= 0; i--)
    {
        FMarkProjectile& P = ActiveProjectiles[i];

        P.Elapsed += 0.02f;

        // Gravity
        P.Velocity.Z -= ProjectileGravity * 0.02f;

        P.PreviousLocation = P.Location;
        P.Location += P.Velocity * 0.02f;

        if (P.Mesh)
        {
            P.Mesh->SetWorldLocation(P.Location);
            P.Mesh->SetWorldRotation(P.Velocity.Rotation());
        }

        // Collision
        TArray<FHitResult> Hits;
        FCollisionShape Sphere = FCollisionShape::MakeSphere(15.f);

        bool bHit = World->SweepMultiByChannel(Hits,P.PreviousLocation, P.Location, FQuat::Identity, ECC_GameTraceChannel1, Sphere);

        for (const FHitResult& Hit : Hits)
        {
            AActor* HitActor = Hit.GetActor();
            if (!HitActor) continue;

            if (P.HitActors.Contains(HitActor))
                continue;

            if (HitActor->ActorHasTag("Enemy"))
            {
                DrawDebugSphere(World, Hit.ImpactPoint, 25.f, 12, FColor::Red, false, 1.f);

                UGameplayStatics::ApplyDamage( HitActor, Damage, GetAvatarActorFromActorInfo()->GetInstigatorController(), GetAvatarActorFromActorInfo(), UDamageType::StaticClass());

                P.HitActors.Add(HitActor);

                if (P.HitActors.Num() >= MaxPierces)
                {
                    if (P.Mesh)
                        P.Mesh->DestroyComponent();

                    ActiveProjectiles.RemoveAt(i);
                    break;
                }
            }
        }

        // Lifetime
        if (P.Elapsed >= ProjectileLifetime)
        {
            if (P.Mesh)
            {
                P.Mesh->DestroyComponent();
            }

            ActiveProjectiles.RemoveAt(i);
        }
    }

    if (ActiveProjectiles.Num() == 0)
    {
        World->GetTimerManager().ClearTimer(ProjectileTickTimer);
    }
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
            World->GetTimerManager().ClearTimer(ProjectileTickTimer);
            World->GetTimerManager().ClearTimer(DurationTimer);
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
