#include "EarthquakeAbility.h"
#include "GameFramework/Character.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "RelicRunners/PlayerState/RelicRunnersPlayerState.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "DrawDebugHelpers.h"
#include "TimerManager.h"

UEarthquakeAbility::UEarthquakeAbility()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

    Duration = 0.25f;
    ConeAngle = 90.f;
    LastUseTime = -100.f;
    CooldownTimer = 10.0f;
	Damage = 200.f;
    AreaRadius = 800.f;
}

void UEarthquakeAbility::ActivateAbility( const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
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

    // Cooldown check
    float CurrentTime = World->GetTimeSeconds();
    if (CurrentTime - LastUseTime < CooldownTimer)
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    // Spend resource and update HUD
    ARelicRunnersPlayerState* PS = ActorInfo->PlayerController.IsValid() ? ActorInfo->PlayerController->GetPlayerState<ARelicRunnersPlayerState>() : nullptr;
    float FinalManaCost = CalculateAbilityMana(AvatarActor, Cost);

    if (!PS || PS->GetPlayerResource() < FinalManaCost)
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    PS->SetPlayerResource(PS->GetPlayerResource() - FinalManaCost);
    PS->UpdateRemoteHUDResource();
    PS->SetInvincible(true);
    // Local input disable during animation
    if (ActorInfo->IsLocallyControlled())
    {
        if (APlayerController* PC = Cast<APlayerController>(ActorInfo->PlayerController.Get()))
        {
            PC->DisableInput(PC);
        }

        if (AbilityAnim)
        {
            float AnimDuration = AbilityAnim->GetPlayLength() * 0.5f;
            PlayAbilityAnimation(CurrentActorInfo, AbilityAnim, 2.f);

            

            if (APlayerController* PC = Cast<APlayerController>(ActorInfo->PlayerController.Get()))
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

            FTimerHandle ShockwaveTimerHandle;
            World->GetTimerManager().SetTimer(
                ShockwaveTimerHandle,
                [this]()
                {
                    LaunchShockwave();
                    
                },
                AnimDuration,
                false
            );
        }
    }


    PlayAbilityCastSound(ActorInfo);

    LastUseTime = World->GetTimeSeconds();

    World->GetTimerManager().SetTimer(CooldownTimerHandle,[this, Handle, ActorInfo, ActivationInfo]() { EndAbility(Handle, ActorInfo, ActivationInfo, true, false); },Duration,false);
}


void UEarthquakeAbility::LaunchShockwave()
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

    ARelicRunnersPlayerState* RRPS = CurrentActorInfo->PlayerController.IsValid() ? CurrentActorInfo->PlayerController->GetPlayerState<ARelicRunnersPlayerState>() : nullptr;
    RRPS->SetInvincible(false);

    UWorld* World = AvatarActor->GetWorld();
    if (!World) return;

    // Shockwave parameters
    const int NumSteps = 5;
    const float StepDistance = AreaRadius / NumSteps; 
    const float StepInterval = 0.3f; 

    FVector StartLocation = AvatarActor->GetActorLocation();
    FVector Forward = AvatarActor->GetActorForwardVector();
    Forward.Z = 0.f;
    Forward.Normalize();

    for (int i = 1; i <= NumSteps; ++i)
    {
        FTimerHandle StepTimer;
        FVector StepLocation = StartLocation + Forward * StepDistance * i;

        World->GetTimerManager().SetTimer(
            StepTimer,
            [this, StepLocation, Forward, World]()
            {
                ApplyShockwaveStep(StepLocation, Forward, World);
            },
            StepInterval * i,
            false
        );
    }
}

void UEarthquakeAbility::ApplyShockwaveStep(FVector Location, FVector Forward, UWorld* World)
{
    if (!World || !CurrentActorInfo) return;

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


    TArray<AActor*> OverlappingActors;
    TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
    ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));
    ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_GameTraceChannel1));

    UKismetSystemLibrary::SphereOverlapActors(World, Location, AreaRadius, ObjectTypes, APawn::StaticClass(), { AvatarActor }, OverlappingActors);
    float HalfAngleRad = FMath::DegreesToRadians(ConeAngle / 2.f);

    for (AActor* Actor : OverlappingActors)
    {
        if (!Actor || !Actor->Tags.Contains("Enemy")) continue;

        FVector DirToTarget = Actor->GetActorLocation() - Location;
        DirToTarget.Z = 0.f;
        float Distance = DirToTarget.Size();
        if (Distance > AreaRadius) continue;

        DirToTarget.Normalize();
        float Dot = FVector::DotProduct(Forward, DirToTarget);
        
        if (Dot < FMath::Cos(HalfAngleRad)) continue;

        CalculateAbilityDamage(AvatarActor, Actor, Damage);

        if (ACharacter* HitChar = Cast<ACharacter>(Actor))
        {
            FVector Knockback = DirToTarget * KnockbackStrength;
            Knockback.Z = VerticalKnockback;
            HitChar->LaunchCharacter(Knockback, true, true);
        }
    }

    // Debug
    DrawDebugCone(World, Location, Forward, AreaRadius, HalfAngleRad, HalfAngleRad, 16, FColor::Red, false, StepInterval1);
}

void UEarthquakeAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);

    if (ActorInfo && ActorInfo->AvatarActor.IsValid())
    {

        UWorld* World = ActorInfo->AvatarActor->GetWorld();
        if (World)
        {
            if (!bWasCancelled)
            {
                LastUseTime = World->GetTimeSeconds();
                World->GetTimerManager().ClearTimer(CooldownTimerHandle);


                if (ActorInfo->IsLocallyControlled())
                {
                    if (ARelicRunnersPlayerController* RPC = Cast<ARelicRunnersPlayerController>(ActorInfo->PlayerController.Get()))
                    {

                        RPC->PlayerHUD->StartDamageCooldown(CooldownTimer);
                    }
                }
            }
        }
    }
    
}
