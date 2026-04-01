#include "WarBannerAbility.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "RelicRunners/PlayerState/RelicRunnersPlayerState.h"
#include "Kismet/KismetSystemLibrary.h"
#include "DrawDebugHelpers.h"
#include "TimerManager.h"
#include "RelicRunners/Inventory/InventoryComponent.h"

UWarBannerAbility::UWarBannerAbility()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

    Duration = 15.f;
    LastUseTime = -100.f;
    CooldownTimer = 15.f;
    AreaRadius = 600.f;

}

void UWarBannerAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

    if (!ActorInfo)
    {
        EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
        return;
    }

    AActor* AvatarActor = ActorInfo->AvatarActor.Get();
    if (!IsValid(AvatarActor))
    {
        EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
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

    PlayAbilityCastSound(ActorInfo);

    ARelicRunnersPlayerState* PS = nullptr;
    if (ActorInfo->PlayerController.IsValid())
    {
        PS = ActorInfo->PlayerController->GetPlayerState<ARelicRunnersPlayerState>();
    }

    float FinalManaCost = CalculateAbilityMana(AvatarActor, Cost);

    if (!PS || PS->GetPlayerResource() < FinalManaCost)
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    PS->SetPlayerResource(PS->GetPlayerResource() - FinalManaCost);
    PS->UpdateRemoteHUDResource();

    const FVector PawnLocation = AvatarActor->GetActorLocation();
    const FVector ForwardVector = AvatarActor->GetActorForwardVector();

    CachedBannerLocation = PawnLocation + ForwardVector * SpawnDistance + FVector(0.f, 0.f, 20.f);

    if (AvatarActor->HasAuthority())
    {
        SpawnBanner();

        World->GetTimerManager().SetTimer(BuffTickHandle,this,&UWarBannerAbility::UpdateBuffs, 0.2f, true);
    }

    if (ActorInfo->IsLocallyControlled())
    {
        APlayerController* PC = Cast<APlayerController>(ActorInfo->PlayerController.Get());

        if (PC)
        {
            PC->DisableInput(PC);
        }

        if (AbilityAnim)
        {
            float AnimDuration = AbilityAnim->GetPlayLength();

            PlayAbilityAnimation(CurrentActorInfo, AbilityAnim, 1.f);

            if (PC)
            {
                TWeakObjectPtr<APlayerController> WeakPC = PC;

                FTimerHandle InputTimerHandle;
                World->GetTimerManager().SetTimer(
                    InputTimerHandle,
                    [WeakPC]()
                    {
                        if (WeakPC.IsValid() && WeakPC->GetPawn())
                        {
                            WeakPC->EnableInput(WeakPC.Get());
                        }
                    },
                    AnimDuration,
                    false
                ); 
            }
        }
    }

    World->GetTimerManager().SetTimer( DurationTimerHandle,[this, Handle, ActorInfo, ActivationInfo]() {EndAbility(Handle, ActorInfo, ActivationInfo, true, false);},Duration,  false);

}

void UWarBannerAbility::SpawnBanner()
{
    const FGameplayAbilityActorInfo* ActorInfo = GetCurrentActorInfo();
    if (!ActorInfo)
    {
        EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
        return;
    }

    AActor* AvatarActor = ActorInfo->AvatarActor.Get();
    if (!IsValid(AvatarActor))
    {
        EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
        return;
    }


    if (!AvatarActor->HasAuthority()) return;

    bBannerActive = true;

    if (CurrentActorInfo->PlayerController.IsValid())
    {
        if (ARelicRunnersPlayerState* PS =
            CurrentActorInfo->PlayerController->GetPlayerState<ARelicRunnersPlayerState>())
        {
            PS->Multicast_SpawnVFX( CachedBannerLocation, FRotator::ZeroRotator, FVector(1.2f),AbilityVFX,false);

            PS->Multicast_SpawnStaticMesh(CachedBannerLocation, FRotator::ZeroRotator, FVector(1.0f), BannerMesh);
        }
    }
}


void UWarBannerAbility::UpdateBuffs()
{
    if (!bBannerActive)
    {
        return;
    }

    const FGameplayAbilityActorInfo* ActorInfo = GetCurrentActorInfo();
    if (!ActorInfo)
    {
        EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
        return;
    }

    AActor* AvatarActor = ActorInfo->AvatarActor.Get();
    if (!IsValid(AvatarActor))
    {
        EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
        return;
    }

    if (!AvatarActor || !AvatarActor->HasAuthority()) return;

    UWorld* World = AvatarActor->GetWorld();
    if (!World) return;

    TArray<AActor*> FoundActors;
    TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
    ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));

    TArray<AActor*> ActorsToIgnore;

    UKismetSystemLibrary::SphereOverlapActors( World, CachedBannerLocation, AreaRadius,  ObjectTypes, APawn::StaticClass(),ActorsToIgnore, FoundActors);

    TSet<AActor*> CurrentActorsInRadius(FoundActors);

    for (AActor* Actor : FoundActors)
    {
        bool bAlreadyBuffed = false;

        for (const TWeakObjectPtr<AActor>& WeakActor : BuffedActors)
        {
            if (WeakActor.Get() == Actor)
            {
                bAlreadyBuffed = true;
                break;
            }
        }

        if (!bAlreadyBuffed)
        {
            ApplyBuffToActor(Actor);
            BuffedActors.Add(Actor);
        }
    }


    for (auto It = BuffedActors.CreateIterator(); It; ++It)
    {
        AActor* Actor = It->Get();

        if (!Actor || !CurrentActorsInRadius.Contains(Actor))
        {
            if (Actor)
            {
                RemoveBuffFromActor(Actor);
            }

            It.RemoveCurrent();
        }
    }
}

void UWarBannerAbility::ApplyBuffToActor(AActor* Actor)
{
    if (!Actor) return;

    ARelicRunnersPlayerState* PS = nullptr;

    if (AController* Controller = Actor->GetInstigatorController())
    {
        PS = Controller->GetPlayerState<ARelicRunnersPlayerState>();
    }

    if (PS)
    {
        PS->BuffedStrength += BuffAmount;
        PS->GetInventoryComponent()->UpdateTotalEquippedStats(Actor->GetInstigatorController()->GetPawn());
    }
}

void UWarBannerAbility::RemoveBuffFromActor(AActor* Actor)
{
    if (!Actor) return;

    ARelicRunnersPlayerState* PS = nullptr;

    if (AController* Controller = Actor->GetInstigatorController())
    {
        PS = Controller->GetPlayerState<ARelicRunnersPlayerState>();
    }

    if (PS)
    {
        PS->BuffedStrength -= BuffAmount;
        PS->GetInventoryComponent()->UpdateTotalEquippedStats(Actor->GetInstigatorController()->GetPawn());
    }
}


void UWarBannerAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);

    if (ActorInfo && ActorInfo->AvatarActor.IsValid())
    {
        AActor* AvatarActor = ActorInfo->AvatarActor.Get();

        ARelicRunnersPlayerController* RPC = Cast<ARelicRunnersPlayerController>(ActorInfo->PlayerController.Get());

        UWorld* World = AvatarActor->GetWorld();
        if (World)
        {
            World->GetTimerManager().ClearTimer(DurationTimerHandle);
            World->GetTimerManager().ClearTimer(BuffTickHandle);

            for (auto& WeakActor : BuffedActors)
            {
                if (WeakActor.IsValid())
                {
                    RemoveBuffFromActor(WeakActor.Get());
                }
            }

            BuffedActors.Empty();

            if (AvatarActor->HasAuthority())
            {
                if (ActorInfo->PlayerController.IsValid())
                {
                    if (ARelicRunnersPlayerState* PS = ActorInfo->PlayerController->GetPlayerState<ARelicRunnersPlayerState>())
                    {
                        PS->Multicast_DestroyAllStaticMeshes();
                    }
                }
            }

            if (!bWasCancelled)
            {
                LastUseTime = World->GetTimeSeconds();

                if (ActorInfo->IsLocallyControlled() && RPC && RPC->PlayerHUD)
                {
                    RPC->PlayerHUD->StartUtilityCooldown(CooldownTimer);
                }
            }
        }
    }
}
