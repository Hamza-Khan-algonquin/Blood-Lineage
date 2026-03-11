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
#include "GameFramework/CharacterMovementComponent.h" 

UMoonbeam::UMoonbeam()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

    Cost = 25;              
    CooldownDuration = 10.f;   
    LastUseTime = -100.f;   
    Duration = 5.f;
    AreaRadius = 100.f;
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


    PlayerState->SetPlayerResource(PlayerState->GetPlayerResource() - Cost);
    PlayerState->UpdateRemoteHUDResource();
    
    if (AbilityAnim)
    {
        PlayAbilityAnimation(CurrentActorInfo, AbilityAnim);
    }

    if (OwnerChar->HasAuthority())
    {
        // Disable movement
        OwnerChar->GetCharacterMovement()->DisableMovement();
        OwnerChar->GetCharacterMovement()->StopMovementImmediately();

        // Start ticking beam
        OwnerChar->GetWorld()->GetTimerManager().SetTimer(BeamTickHandle,this, &UMoonbeam::BeamTick,TickInterval, true);

        // End ability after duration
        OwnerChar->GetWorld()->GetTimerManager().SetTimer( DurationHandle,[this, Handle, ActorInfo, ActivationInfo](){EndAbility(Handle, ActorInfo, ActivationInfo, true, false);},Duration,false);
    }

}

void UMoonbeam::BeamTick()
{
    if (!CurrentActorInfo || !CurrentActorInfo->AvatarActor.IsValid())
        return;

    ACharacter* OwnerChar = Cast<ACharacter>(CurrentActorInfo->AvatarActor.Get());
    if (!OwnerChar)
        return;

    UWorld* World = OwnerChar->GetWorld();
    if (!World)
        return;

    FVector Start = OwnerChar->GetActorLocation();

    // Lock beam to control yaw 
    FRotator ControlRot = OwnerChar->GetControlRotation();
    ControlRot.Pitch = 0.f;
    ControlRot.Roll = 0.f;

    FVector Forward = ControlRot.Vector();
    FVector End = Start + (Forward * BeamLength);

    // Draw main beam line
    DrawDebugLine( World,Start, End, FColor::Purple, false,TickInterval,0,4.f);

    // Draw spheres along beam path 
    const int32 Steps = 10;
    for (int32 i = 0; i <= Steps; i++)
    {
        float Alpha = (float)i / (float)Steps;
        FVector Point = FMath::Lerp(Start, End, Alpha);

        DrawDebugSphere(World,Point,AreaRadius,12, FColor::Blue, false, TickInterval);
    }

    TArray<FHitResult> Hits;
    FCollisionShape Sphere = FCollisionShape::MakeSphere(AreaRadius);

    bool bHit = World->SweepMultiByChannel(Hits,Start,End, FQuat::Identity, ECC_GameTraceChannel1,Sphere);

    if (!bHit) return;

    for (const FHitResult& Hit : Hits)
    {
        APawn* Pawn = Cast<APawn>(Hit.GetActor());
        if (!Pawn) continue;

        if (!Pawn->Tags.Contains("Enemy")) continue;

        if (Pawn == OwnerChar) continue;

        CalculateAbilityDamage(OwnerChar, Pawn, Damage);
    }
}

void UMoonbeam::EndAbility(const FGameplayAbilitySpecHandle Handle,const FGameplayAbilityActorInfo* ActorInfo,const FGameplayAbilityActivationInfo ActivationInfo,bool bReplicateEndAbility,bool bWasCancelled)
{
    UWorld* World = ActorInfo->AvatarActor->GetWorld();
    if (World && !bWasCancelled)
    {       
        World->GetTimerManager().ClearTimer(BeamTickHandle);
        World->GetTimerManager().ClearTimer(DurationHandle);

        ACharacter* OwnerChar = Cast<ACharacter>(ActorInfo->AvatarActor.Get());
        if (OwnerChar)
        {
            OwnerChar->GetCharacterMovement()->SetMovementMode(MOVE_Walking);
        }

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


