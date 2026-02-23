#include "Moonbeam.h"

#include "GameFramework/Character.h"
#include "GameFramework/Controller.h"
#include "DrawDebugHelpers.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "RelicRunners/PlayerState/RelicRunnersPlayerState.h"
#include "RelicRunners/PlayerController/RelicRunnersPlayerController.h"
#include "RelicRunners/PlayerHUD/PlayerHUD.h"

//#include "RelicRunners/Enemy/EnemyCharacter.h"
//#include "RelicRunners/Enemy/EnemyCharacterAI.h"

UMoonbeam::UMoonbeam()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

    Cost = 25;              
    CooldownDuration = 6.f;   
    LastUseTime = -100.f;   
}

void UMoonbeam::ActivateAbility(const FGameplayAbilitySpecHandle Handle,const FGameplayAbilityActorInfo* ActorInfo,const FGameplayAbilityActivationInfo ActivationInfo,const FGameplayEventData* TriggerEventData)
{
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

    // Get the character using the ability
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

    if (OwnerChar->HasAuthority())
    {
        PlayerState->SetPlayerResource(PlayerState->GetPlayerResource() - Cost);
        PlayerState->UpdateRemoteHUDResource();
    }

    FVector CamLoc;
    FRotator ViewRot;

    if (AController* C = OwnerChar->GetController())
    {
        C->GetPlayerViewPoint(CamLoc, ViewRot);
    }
    else
    {
        OwnerChar->GetActorEyesViewPoint(CamLoc, ViewRot);
    }

    // Force beam to start from player (not camera)
    FVector Start = OwnerChar->GetActorLocation() + FVector(0.f, 0.f, 60.f); // chest height

    const FVector End = Start + (ViewRot.Vector() * BeamRange);

    SpawnBeamEffect(Start, End);
  
    if (OwnerChar->HasAuthority())
    {
        TArray<FHitResult> Hits;

    // Visual-only version: end instantly
    TArray<AActor*> ActorsToIgnore;ActorsToIgnore.Add(OwnerChar);

        const bool bHit = UKismetSystemLibrary::SphereTraceMulti(OwnerChar,Start,End,BeamRadius,UEngineTypes::ConvertToTraceType(ECC_GameTraceChannel1),false,ActorsToIgnore,EDrawDebugTrace::None,Hits,true);

       /* if (bHit)
        {*/
            for (const FHitResult& Hit : Hits)
            {
                AActor* HitActor = Hit.GetActor();

                if (!HitActor)
                {
                    continue;
                }
              
                if (!HitActor->ActorHasTag("Enemy"))
                {
                    continue;
                }

                UGameplayStatics::ApplyDamage(HitActor,BeamDamage,OwnerChar->GetController(),OwnerChar,nullptr);
            }
       // }
    }

    EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

void UMoonbeam::EndAbility(const FGameplayAbilitySpecHandle Handle,const FGameplayAbilityActorInfo* ActorInfo,const FGameplayAbilityActivationInfo ActivationInfo,bool bReplicateEndAbility,bool bWasCancelled)
{
    UWorld* World = ActorInfo->AvatarActor->GetWorld();
    if (World && !bWasCancelled)
    {       
        LastUseTime = World->GetTimeSeconds();

        if (ActorInfo->IsLocallyControlled())
        {
            if (ARelicRunnersPlayerController* RPC =Cast<ARelicRunnersPlayerController>(ActorInfo->PlayerController.Get()))
            {
                if (RPC->PlayerHUD)
                {
                    RPC->PlayerHUD->StartUltimateCooldown(CooldownDuration);
                }
            }
        }
    }

    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UMoonbeam::SpawnBeamEffect(const FVector& Start, const FVector& End)
{
    UWorld* World = GetWorld();
    if (!World) return;

    DrawDebugLine(World, Start, End, FColor::Blue, false, BeamDuration, 0, BeamRadius);
}

