// Fill out your copyright notice in the Description page of Project Settings.


#include "RageOfTheGods.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "RelicRunners/PlayerState/RelicRunnersPlayerState.h"
#include "Kismet/KismetSystemLibrary.h"
#include "DrawDebugHelpers.h"
#include "TimerManager.h"
URageOfTheGods::URageOfTheGods()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

    Damage = 100.f;
    Duration = 5.f;
    LastUseTime = -100.f;
    CooldownTimer = 10.f;
    Cost = 60;
    AreaRadius = 500.f;

}

void URageOfTheGods::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
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

    LastUseTime = World->GetTimeSeconds();

    PulsesExecuted = 0;

    GetWorld()->GetTimerManager().SetTimer(PulseTimerHandle,this,&URageOfTheGods::StartPulse,PulseInterval,true);
}

void URageOfTheGods::StartPulse()
{
   
    TWeakObjectPtr<AActor> WeakAvatar = GetAvatarActorFromActorInfo();
    if (!WeakAvatar.IsValid())
    {
        EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, false);
        return;
    }

    PulsesExecuted++;
    HitActorsThisPulse.Reset();
    ExpansionElapsedTime = 0.f;

    if (PulsesExecuted * PulseInterval > TotalDuration)
    {
        EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, false);
        return;
    }

    TWeakObjectPtr<URageOfTheGods> WeakThis(this);
    GetWorld()->GetTimerManager().SetTimer(ExpansionTimerHandle, [WeakThis, WeakAvatar](){if (!WeakThis.IsValid() || !WeakAvatar.IsValid()) return;WeakThis->ExpandPulse(WeakAvatar.Get());}, 0.05f, true);
}

void URageOfTheGods::ExpandPulse(AActor* AvatarActor)
{
    UWorld* World = GetWorld();
    if (!World || !AvatarActor)
    {
        EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, false);
        return;
    }

    ExpansionElapsedTime += 0.05f;

    float Alpha = FMath::Clamp(ExpansionElapsedTime / PulseDuration, 0.f, 1.f);
    float CurrentRadius = FMath::Lerp(StartRadius, MaxRadius, Alpha);

    FVector Center = AvatarActor->GetActorLocation();

    DrawDebugSphere(World, Center, CurrentRadius, 32, FColor::Red, false, 0.06f, 0, 3.f);

    TArray<AActor*> FoundActors;
    TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
    ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));
    ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_GameTraceChannel1));

    TArray<AActor*> ActorsToIgnore;
    ActorsToIgnore.Add(AvatarActor);

    UKismetSystemLibrary::SphereOverlapActors(World, Center, CurrentRadius, ObjectTypes, APawn::StaticClass(), ActorsToIgnore, FoundActors);

    for (AActor* Actor : FoundActors)
    {
        if (!Actor || !Actor->ActorHasTag("Enemy")) continue;
        if (HitActorsThisPulse.Contains(Actor)) continue;

        HitActorsThisPulse.Add(Actor);

        UGameplayStatics::ApplyDamage(Actor, Damage, AvatarActor->GetInstigatorController(), AvatarActor, UDamageType::StaticClass());

        if (ACharacter* EnemyChar = Cast<ACharacter>(Actor))
        {
            FVector KnockDir = (EnemyChar->GetActorLocation() - Center).GetSafeNormal();
            EnemyChar->LaunchCharacter(KnockDir * KnockbackStrength, true, true);
        }
    }

    if (Alpha >= 1.f)
    {
        World->GetTimerManager().ClearTimer(ExpansionTimerHandle);
    }
}


void URageOfTheGods::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearAllTimersForObject(this);
    }

    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);

    if (!ActorInfo || !ActorInfo->AvatarActor.IsValid())
    {
        return;
    }

    UWorld* World = ActorInfo->AvatarActor->GetWorld();
    if (!World)
    {
        return;
    }

    if (!bWasCancelled)
    {
        LastUseTime = World->GetTimeSeconds();

        if (ActorInfo->IsLocallyControlled())
        {
            if (ARelicRunnersPlayerController* RPC = Cast<ARelicRunnersPlayerController>(ActorInfo->PlayerController.Get()))
            {
                if (RPC->PlayerHUD)
                {
                    RPC->PlayerHUD->StartUltimateCooldown(CooldownTimer);
                }
            }
        }
    }
}


