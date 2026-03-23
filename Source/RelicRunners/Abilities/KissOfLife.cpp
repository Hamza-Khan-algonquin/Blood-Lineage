// Fill out your copyright notice in the Description page of Project Settings.

#include "KissOfLife.h"
#include "GameFramework/Character.h"
#include "Kismet/KismetSystemLibrary.h"
#include "RelicRunners/PlayerState/RelicRunnersPlayerState.h"
#include "DrawDebugHelpers.h"
#include "TimerManager.h"

UKissOfLife::UKissOfLife()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

    AbilityName = FName("KissOfLife");

    Duration = 10.f;
    CooldownTimer = 12.f;
    LastUseTime = -100.f;
    AreaRadius = 600.f;

}

void UKissOfLife::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

    if (!ActorInfo || !ActorInfo->AvatarActor.IsValid())
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    AActor* AvatarActor = ActorInfo->AvatarActor.Get();
    UWorld* World = AvatarActor->GetWorld();
    if (!World)
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    ARelicRunnersPlayerState* PS = ActorInfo->PlayerController.IsValid() ? ActorInfo->PlayerController->GetPlayerState<ARelicRunnersPlayerState>() : nullptr;

    if (!PS)
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    if (AbilityAnim)
    {
        PlayAbilityAnimation(CurrentActorInfo, AbilityAnim);
    }

    APawn* OwnerPawn = Cast<APawn>(AvatarActor);
    if (!OwnerPawn) return;

    const FVector PawnLocation = OwnerPawn->GetActorLocation();
    const FVector Center = ActorInfo->AvatarActor->GetActorLocation();

    //DrawDebugCircle(World, Center, AreaRadius, 64, FColor::Green, false, Duration, 0, 2.f, OwnerPawn->GetActorRightVector(), OwnerPawn->GetActorForwardVector(), false);

    // This is where is starts healing the players
    World->GetTimerManager().SetTimer(HealTickTimer, this, &UKissOfLife::ApplyHealTick, HealInterval, true);

    if (ActorInfo->AvatarActor->HasAuthority())
    {
        PS->Multicast_SpawnVFX(FVector(0, 0, -90), FRotator::ZeroRotator, FVector(1.f), AbilityVFX, true);
    }

    // This is where is ends the healing stuff
    World->GetTimerManager().SetTimer(EndTimer,FTimerDelegate::CreateUObject(this,&UKissOfLife::EndAbility,Handle,ActorInfo,ActivationInfo,true,false),Duration,false);
}

void UKissOfLife::ApplyHealTick()
{
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

    FVector Origin = AvatarActor->GetActorLocation();

    TArray<AActor*> OverlappingActors;
    TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
    ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));
    ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_GameTraceChannel1));

    // Healing part that heals both allies and self
    UKismetSystemLibrary::SphereOverlapActors(World, Origin, AreaRadius, ObjectTypes, APawn::StaticClass(), TArray<AActor*>(), OverlappingActors);

    for (AActor* Actor : OverlappingActors)
    {
        ACharacter* Char = Cast<ACharacter>(Actor);
        if (!Char)
        {
            continue;
        }

        ARelicRunnersPlayerState* PS = Char->GetPlayerState<ARelicRunnersPlayerState>();
        if (!PS) 
        {
            continue;
        }

        float HealAmount = PS->GetPlayerMaxHealth() * HealPercentPerTick;

        PS->ModifyHealth(HealAmount);

    }
}

void UKissOfLife::EndAbility(const FGameplayAbilitySpecHandle Handle,const FGameplayAbilityActorInfo* ActorInfo,const FGameplayAbilityActivationInfo ActivationInfo,bool bReplicateEndAbility,bool bWasCancelled)
{
    if (ActorInfo && ActorInfo->AvatarActor.IsValid())
    {
        UWorld* World = ActorInfo->AvatarActor->GetWorld();
        if (World)
        {
            World->GetTimerManager().ClearTimer(HealTickTimer);
            World->GetTimerManager().ClearTimer(EndTimer);

            if (!bWasCancelled)
            {
                LastUseTime = World->GetTimeSeconds();

                if (ActorInfo->IsLocallyControlled())
                {
                    if (ARelicRunnersPlayerController* RPC = Cast<ARelicRunnersPlayerController>(ActorInfo->PlayerController.Get()))
                    {
                        RPC->PlayerHUD->StartUtilityCooldown(CooldownTimer);
                    }
                }
            }
        }
    }

    // This deletes the debug sphere when the ability ends
    if (DebugHealSphere)
    {
        DebugHealSphere->DestroyComponent();
        DebugHealSphere = nullptr;
    }

    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
