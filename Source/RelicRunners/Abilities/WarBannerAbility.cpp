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
    CooldownTimer = 10.f;
    AreaRadius = 600.f;

}

void UWarBannerAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

    float CurrentTime = ActorInfo->AvatarActor->GetWorld()->GetTimeSeconds();

    if (CurrentTime - LastUseTime < CooldownTimer)
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }


    AActor* AvatarActor = GetAvatarActorFromActorInfo(); // Get the actor this ability belongs to
    if (!AvatarActor) // If no valid actor
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true); // End the ability
        return;
    }

    UWorld* World = AvatarActor->GetWorld();
    if (!World)
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

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

    // Spend resource
    PS->SetPlayerResource(PS->GetPlayerResource() - FinalManaCost);
    PS->UpdateRemoteHUDResource();


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
                TWeakObjectPtr<UWorld> WeakWorld = GetWorld();

                FTimerHandle InputTimerHandle;

                GetWorld()->GetTimerManager().SetTimer(
                    InputTimerHandle,
                    [WeakPC, WeakWorld]()
                    {
                        if (!WeakWorld.IsValid())
                            return;

                        if (!WeakPC.IsValid())
                            return;

                        if (!WeakPC->GetPawn())
                            return;

                        WeakPC->EnableInput(WeakPC.Get());
                    },
                    AnimDuration,
                    false
                );

                SpawnBanner();
            }
        }
    }

    FVector BannerLocation = GetBannerLocation();

    

    if (World)
    {
        World->GetTimerManager().SetTimer(BuffTickHandle, this, &UWarBannerAbility::UpdateBuffs,0.2f,true);
    }

}

void UWarBannerAbility::SpawnBanner()
{
    if (!CurrentActorInfo) return;

    AActor* AvatarActor = GetAvatarActorFromActorInfo();
    if (!IsValid(AvatarActor)) return;

    UWorld* World = AvatarActor->GetWorld();
    if (!World) return;

    APawn* OwnerPawn = Cast<APawn>(AvatarActor);
    if (!OwnerPawn) return;

    const FVector PawnLocation = OwnerPawn->GetActorLocation();
    const FVector ForwardVector = OwnerPawn->GetActorForwardVector();

    const FVector BannerLocation = PawnLocation + ForwardVector * SpawnDistance + FVector(0.f, 0.f, -100.f);

    if (AvatarActor->HasAuthority())
    {
        if (CurrentActorInfo && CurrentActorInfo->PlayerController.IsValid())
        {
            if (ARelicRunnersPlayerState* PS = CurrentActorInfo->PlayerController->GetPlayerState<ARelicRunnersPlayerState>())
            {
                PS->Multicast_SpawnVFX( BannerLocation, FRotator::ZeroRotator, FVector(1.2f),AbilityVFX,false);
				PS->Multicast_SpawnStaticMesh(BannerLocation, FRotator::ZeroRotator, FVector(1.0f), BannerMesh);
            }
        }
    }
}


void UWarBannerAbility::UpdateBuffs()
{
    AActor* AvatarActor = GetAvatarActorFromActorInfo();
    if (!AvatarActor) return;

    UWorld* World = AvatarActor->GetWorld();
    if (!World) return;

    const FVector Center = GetBannerLocation();

    TArray<AActor*> FoundActors;
    TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
    ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));

    TArray<AActor*> ActorsToIgnore;
    // Only ignore yourself
    ActorsToIgnore.Add(AvatarActor);

    UKismetSystemLibrary::SphereOverlapActors(World,Center, AreaRadius,ObjectTypes,APawn::StaticClass(), ActorsToIgnore, FoundActors);

    APawn* MyPawn = Cast<APawn>(AvatarActor);
    if (!MyPawn) return;

    bool bInRadius = false;
    for (AActor* Actor : FoundActors)
    {
        if (Actor == MyPawn)
        {
            bInRadius = true;
            break;
        }
    }

    // Track if we already buffed ourselves
    bool bAlreadyBuffed = BuffedActors.Contains(MyPawn);

    if (bInRadius && !bAlreadyBuffed)
    {
        ApplyBuffToActor(MyPawn);
        BuffedActors.Add(MyPawn);
    }
    else if (!bInRadius && bAlreadyBuffed)
    {
        RemoveBuffFromActor(MyPawn);
        BuffedActors.Remove(MyPawn);
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
        PS->GetInventoryComponent()->UpdateTotalEquippedStats(
            Actor->GetInstigatorController()->GetPawn()
        );
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
        PS->GetInventoryComponent()->UpdateTotalEquippedStats(
            Actor->GetInstigatorController()->GetPawn()
        );
    }
}

FVector UWarBannerAbility::GetBannerLocation() const
{
    AActor* AvatarActor = GetAvatarActorFromActorInfo();
    if (!AvatarActor) return FVector::ZeroVector;

    return AvatarActor->GetActorLocation() + AvatarActor->GetActorForwardVector() * SpawnDistance + FVector(0.f, 0.f, 20.f);
}


void UWarBannerAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);


    if (ActorInfo && ActorInfo->AvatarActor.IsValid())
    {
        ARelicRunnersPlayerController* RPC = Cast<ARelicRunnersPlayerController>(ActorInfo->PlayerController.Get());
        if (!RPC) return;

        UWorld* World = ActorInfo->AvatarActor->GetWorld();
        if (World)
        {
            World->GetTimerManager().ClearTimer(DurationTimerHandle);
            World->GetTimerManager().ClearTimer(BuffTickHandle);

            // Remove buffs from all remaining actors
            for (auto& WeakActor : BuffedActors)
            {
                if (WeakActor.IsValid())
                {
                    RemoveBuffFromActor(WeakActor.Get());
                }
            }

            BuffedActors.Empty();

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
