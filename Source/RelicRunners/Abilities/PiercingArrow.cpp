// Fill out your copyright notice in the Description page of Project Settings.


#include "PiercingArrow.h"
#include "GameFramework/Character.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "RelicRunners/PlayerState/RelicRunnersPlayerState.h"
#include "RelicRunners/Enemy/Characters/EnemyCharacter.h"
#include "RelicRunners/Enemy/Characters/EnemyCharacterAI.h"
#include "DrawDebugHelpers.h"
#include "TimerManager.h"


UPiercingArrow::UPiercingArrow()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

    DamageAmount = 150.f;    
    ArrowSpeed = 2200.f;     
    MaxRange = 3000.f;        

    CooldownTimer = 6.f;
    Cost = 25;
    LastUseTime = -100.f;
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

    ArrowLocation = AvatarActor->GetActorLocation()
        + AvatarActor->GetActorForwardVector() * 100.f
        + FVector(0, 0, 50.f);

    ArrowDirection = AvatarActor->GetActorForwardVector();
    TraveledDistance = 0.f;
    HitActors.Empty();
    bIsFlying = true;

    World->GetTimerManager().SetTimer(ArrowTickTimer, this, &UPiercingArrow::TickArrow, 0.016f, true);
}

void UPiercingArrow::TickArrow()
{
    if (!bIsFlying)
    {
        return;
    }

    AActor* AvatarActor = GetAvatarActorFromActorInfo();

    if (!AvatarActor)
    {
        return;
    }

    UWorld* World = AvatarActor->GetWorld();
    if (!World)
    {
        return;
    }

    float Delta = World->GetDeltaSeconds();

    FVector Move = ArrowDirection * ArrowSpeed * Delta;
    ArrowLocation += Move;
    TraveledDistance += Move.Size();

    float CurrentRadius = ArrowRadius + (TraveledDistance / MaxRange) * GrowthRate;

    // the visual Debug thing for the arrow
    DrawDebugSphere(World, ArrowLocation, CurrentRadius, 12, FColor::Cyan, false, 0.05f);

    TArray<AActor*> HitActorsArray;
    TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
    ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));
    ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_GameTraceChannel1));

    UKismetSystemLibrary::SphereOverlapActors(World, ArrowLocation, CurrentRadius, ObjectTypes, APawn::StaticClass(), { AvatarActor }, HitActorsArray);

    for (AActor* Actor : HitActorsArray)
    {
        if (!Actor || HitActors.Contains(Actor))
        {
            continue;
        }

        if (!Actor->IsA(AEnemyCharacter::StaticClass()) && !Actor->IsA(AEnemyCharacterAI::StaticClass()))
        {
            continue;
        }

        HitActors.Add(Actor);
        UGameplayStatics::ApplyDamage(Actor, DamageAmount, GetCurrentActorInfo()->PlayerController.Get(), AvatarActor, nullptr);
    }

    // i gave a max range for the piercing arrow ability so it has a limit ;>
    if (TraveledDistance >= MaxRange)
    {
        EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true, false);
    }
}

void UPiercingArrow::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
    if (ActorInfo && ActorInfo->AvatarActor.IsValid())
    {
        UWorld* World = ActorInfo->AvatarActor->GetWorld();
        if (World)
        {
            World->GetTimerManager().ClearTimer(ArrowTickTimer);

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

    bIsFlying = false;
    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
