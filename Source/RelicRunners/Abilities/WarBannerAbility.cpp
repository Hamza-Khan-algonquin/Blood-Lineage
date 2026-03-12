#include "WarBannerAbility.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "RelicRunners/PlayerState/RelicRunnersPlayerState.h"
#include "Kismet/KismetSystemLibrary.h"
#include "DrawDebugHelpers.h"
#include "TimerManager.h"

UWarBannerAbility::UWarBannerAbility()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

    Duration = 15.f;
    LastUseTime = -100.f;
    CooldownTimer = 10.f;
    Cost = 50;
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

    if (!CommitAbility(Handle, ActorInfo, ActivationInfo)) // Try to commit the ability
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true); // If commit fails, end the ability 
        return; // Exit early
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

    if (AbilityAnim)
    {
        PlayAbilityAnimation(CurrentActorInfo, AbilityAnim);
    }

    FVector BannerLocation = GetBannerLocation();

    SpawnBanner();

    ApplyAllyBuffs(BannerLocation);

    //PS->SetPlayerStrength(PS->PlayerStrength * BuffAmount);

}

void UWarBannerAbility::SpawnBanner()
{
    AActor* AvatarActor = GetAvatarActorFromActorInfo();
    if (!AvatarActor) return;

    UWorld* World = AvatarActor->GetWorld();
    if (!World) return;

    APawn* OwnerPawn = Cast<APawn>(AvatarActor);
    if (!OwnerPawn) return;

    const FVector PawnLocation = OwnerPawn->GetActorLocation();
    const FVector ForwardVector = OwnerPawn->GetActorForwardVector();

    // Final spawn location
    const FVector BannerLocation = PawnLocation + ForwardVector * SpawnDistance + FVector(0.f, 0.f, 20.f);

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
                PS->Multicast_SpawnVFX(BannerLocation + FVector(0, 0, -100), FRotator(0, 0, 0), FVector(1.2f), AbilityVFX, false);
            }
        }
    }

}

void UWarBannerAbility::ApplyAllyBuffs(const FVector& BannerLocation)
{
    UWorld* World = GetWorld();
    if (!World) return;


    if (AActor* Owner = GetAvatarActorFromActorInfo())
    {
        if (Owner->Tags.Contains("Player"))
        {
            UE_LOG(LogTemp, Log, TEXT("Buff applied to SELF: %s"), *Owner->GetName());
            // Apply self buff here
        }
    }

    TArray<AActor*> OverlappingActors;

    // Object types to consider
    TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
    ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));
    ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_EngineTraceChannel1));

    // Actors to ignore
    TArray<AActor*> ActorsToIgnore;
    ActorsToIgnore.Add(GetAvatarActorFromActorInfo());

    // Perform the sphere overlap
    bool bHit = UKismetSystemLibrary::SphereOverlapActors(World,BannerLocation,AreaRadius,ObjectTypes,APawn::StaticClass(),ActorsToIgnore,OverlappingActors);

    for (AActor* Actor : OverlappingActors)
    {
        ACharacter* Character = Cast<ACharacter>(Actor);
        if (!Character) continue; // skip if not a character

        if (!Actor->Tags.Contains("Player")) continue;


        UE_LOG(LogTemp, Log, TEXT("Buff applied to %s"), *Character->GetName());

    }

    if (World)
    {
        World->GetTimerManager().SetTimer(DurationTimerHandle,[this](){EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);},Duration,false );
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
