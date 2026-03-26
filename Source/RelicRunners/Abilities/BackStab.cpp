// Fill out your copyright notice in the Description page of Project Settings.


#include "BackStab.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "RelicRunners/PlayerState/RelicRunnersPlayerState.h"
#include "Kismet/KismetSystemLibrary.h"
#include "DrawDebugHelpers.h"
#include "TimerManager.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Engine/StaticMeshActor.h"     
#include "Components/StaticMeshComponent.h"
#include "RelicRunners/Inventory/InventoryComponent.h"
#include "RelicRunners/Item/ItemStats.h"

UBackStab::UBackStab()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

    Damage = 300.f;
    Duration = 5.f;
    LastUseTime = -100.f;
    CooldownTimer = 10.f;
	ConeAngle = 80.f;
	Range = 600.f;
	BehindOffset = 120.f;
    AreaRadius = 200.f;

}

void UBackStab::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);


    AActor* AvatarActor = GetAvatarActorFromActorInfo(); // Get the actor this ability belongs to
    if (!AvatarActor) // If no valid actor
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true); // End the ability
        return;
    }

    UWorld* World = GetWorld();
    if (!World) return;

    if (World->GetTimeSeconds() - LastUseTime < CooldownTimer)
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    LastUseTime = World->GetTimeSeconds();

    OriginalLocation = AvatarActor->GetActorLocation();
    OriginalRotation = AvatarActor->GetActorRotation();

    CurrentBackStabCount = 0;
    ExecuteBackStabSequence();
}

AActor* UBackStab::FindBackstabTarget(AActor* AvatarActor) const
{
    UWorld* World = AvatarActor->GetWorld();
    if (!World) return nullptr;

    FVector Origin = AvatarActor->GetActorLocation();

    TArray<AActor*> FoundActors;
    TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
    ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_GameTraceChannel1));

    UKismetSystemLibrary::SphereOverlapActors(World,Origin,Range,ObjectTypes, APawn::StaticClass(), { AvatarActor }, FoundActors);

    AActor* Closest = nullptr;
    float BestDistSq = MAX_FLT;

    for (AActor* Actor : FoundActors)
    {
        if (!Actor || !Actor->ActorHasTag("Enemy")) continue;

        float DistSq = FVector::DistSquared(Origin, Actor->GetActorLocation());
        if (DistSq < BestDistSq)
        {
            BestDistSq = DistSq;
            Closest = Actor;
        }
    }

    return Closest;
}
void UBackStab::PerformBackStabDamage(AActor* AvatarActor, AActor* TargetEnemy)
{
    if (!AvatarActor || !TargetEnemy) return;

    UWorld* World = GetWorld();
    if (!World) return;

    FVector Center = TargetEnemy->GetActorLocation();

    TArray<AActor*> OverlappingActors;
    TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
    ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_GameTraceChannel1));

    TArray<AActor*> ActorsToIgnore;
    ActorsToIgnore.Add(AvatarActor);

    UKismetSystemLibrary::SphereOverlapActors(World, Center, AreaRadius, ObjectTypes, APawn::StaticClass(), ActorsToIgnore, OverlappingActors);

    DrawDebugSphere(World, Center, AreaRadius, 16, FColor::Red, false, 1.f);

    for (AActor* Actor : OverlappingActors)
    {
        if (!Actor || !Actor->ActorHasTag("Enemy")) continue;

        CalculateAbilityDamage(AvatarActor, Actor, Damage);
    }
}


void UBackStab::ExecuteBackStabSequence()
{
    UWorld* World = GetWorld();
    AActor* AvatarActor = GetAvatarActorFromActorInfo();

    if (!World || !AvatarActor)
    {
        ReturnPlayerToOriginalPosition();
        EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, false);
        return;
    }

    if (CurrentBackStabCount >= 3)
    {
        ReturnPlayerToOriginalPosition();
        EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, false);
        return;
    }

    // Find target
    AActor* TargetEnemy = FindBackstabTarget(AvatarActor);
    if (!TargetEnemy || !IsValid(TargetEnemy))
    {
        ReturnPlayerToOriginalPosition();
        EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, false);
        return;
    }

    // Teleport behind enemy 
    FVector DashLocation = TargetEnemy->GetActorLocation() - TargetEnemy->GetActorForwardVector() * DashDistance;
    AvatarActor->SetActorLocation(DashLocation, false, nullptr, ETeleportType::TeleportPhysics);

    if (AbilityAnim)
    {
        PlayAbilityAnimation(CurrentActorInfo, AbilityAnim, 1.0f);
    }

    ACharacter* Char = Cast<ACharacter>(AvatarActor);
    if (!Char || !Char->GetCharacterMovement())
    {
        ReturnPlayerToOriginalPosition();
        EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, false);
        return;
    }

    Char->GetCharacterMovement()->DisableMovement();

    // VFX (unchanged)
    if (AvatarActor->HasAuthority())
    {
        AController* Controller = nullptr;

        if (CurrentActorInfo && CurrentActorInfo->PlayerController.IsValid())
        {
            Controller = CurrentActorInfo->PlayerController.Get();
        }

        if (Controller)
        {
            ARelicRunnersPlayerState* PS = Controller->GetPlayerState<ARelicRunnersPlayerState>();

            if (PS)
            {
                // Spawn at target now 
                PS->Multicast_SpawnVFX(TargetEnemy->GetActorLocation(),FRotator::ZeroRotator, FVector(2.f), AbilityVFX,true);
            }
        }
    }

    // Delay damage
    World->GetTimerManager().SetTimer(
        BackStabDamageTimerHandle,
        FTimerDelegate::CreateLambda([this, AvatarActor, TargetEnemy, Char]()
            {
                PerformBackStabDamage(AvatarActor, TargetEnemy);

                // Re-enable movement
                if (Char && Char->GetCharacterMovement())
                {
                    Char->GetCharacterMovement()->SetMovementMode(MOVE_Walking);
                }

                CurrentBackStabCount++;
                ExecuteBackStabSequence();
            }),
        0.6f,
        false
    );
}


void UBackStab::ReturnPlayerToOriginalPosition()
{
    AActor* AvatarActor = GetAvatarActorFromActorInfo();
    if (!AvatarActor) return;

    AvatarActor->SetActorLocation(OriginalLocation, false, nullptr, ETeleportType::TeleportPhysics);
    AvatarActor->SetActorRotation(OriginalRotation);

    if (ACharacter* Char = Cast<ACharacter>(AvatarActor))
    {
        if (Char->GetCharacterMovement())
        {
            Char->GetCharacterMovement()->SetMovementMode(MOVE_Walking);
            Char->GetCharacterMovement()->Velocity = FVector::ZeroVector;
        }
    }
}

void UBackStab::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);

    if (ActorInfo && ActorInfo->AvatarActor.IsValid())
    {
        ARelicRunnersPlayerController* RPC = Cast<ARelicRunnersPlayerController>(ActorInfo->PlayerController.Get());

        UWorld* World = ActorInfo->AvatarActor->GetWorld();
        if (World)
        {
            World->GetTimerManager().ClearTimer(BackStabDamageTimerHandle);

            if (!bWasCancelled)
            {
                LastUseTime = World->GetTimeSeconds();

                if (ActorInfo->IsLocallyControlled())
                {
                    if (RPC)
                    {
                        RPC->PlayerHUD->StartDamageCooldown(CooldownTimer);

                    }
                }
            }
        }
    }

}


