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
             
    CooldownDuration = 15.f;   
    LastUseTime = -100.f;   
    Duration = 5.f;
    AreaRadius = 100.f;
	Damage = 40.f;
}

void UMoonbeam::ActivateAbility(const FGameplayAbilitySpecHandle Handle,const FGameplayAbilityActorInfo* ActorInfo,const FGameplayAbilityActivationInfo ActivationInfo,const FGameplayEventData* TriggerEventData)
{
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

    if (!CurrentActorInfo) return;

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
    


    PlayAbilityCastSound(ActorInfo);

    if (OwnerChar->HasAuthority())
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

                float FinalManaCost = CalculateAbilityMana(AvatarActor, Cost);

                if (!PS || PS->GetPlayerResource() < FinalManaCost)
                {
                    EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
                    return;
                }
                PS->SetInvincible(true);
                PS->Multicast_SpawnVFX(FVector(0), FRotator(0, 0, 0), FVector(1.f), AbilityVFX, true);

                // Spend resource
                PS->SetPlayerResource(PS->GetPlayerResource() - FinalManaCost);
                PS->UpdateRemoteHUDResource();
            }
        }

        if (AbilityAnim)
        {
            PlayAbilityAnimation(CurrentActorInfo, AbilityAnim, 1.f);
        }

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
    if (!ActorInfo->AvatarActor.IsValid())
        return;

    UWorld* World = ActorInfo->AvatarActor->GetWorld();
    if (World && !bWasCancelled)
    {       
        ARelicRunnersPlayerState* PS = nullptr;
        if (ActorInfo->PlayerController.IsValid())
        {
            PS = ActorInfo->PlayerController->GetPlayerState<ARelicRunnersPlayerState>();
        }

        if (PS)
        {
            PS->SetInvincible(false);
        }

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


