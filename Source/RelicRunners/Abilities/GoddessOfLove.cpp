// Fill out your copyright notice in the Description page of Project Settings.


#include "GoddessOfLove.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "RelicRunners/PlayerState/RelicRunnersPlayerState.h"
#include "Kismet/KismetSystemLibrary.h"
#include "DrawDebugHelpers.h"
#include "TimerManager.h"


UGoddessOfLove::UGoddessOfLove()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	Damage = 400.f;
	AreaRadius = 1000.f;
	LastUseTime = -100.f;
	CooldownTimer = 15.f;
    ElapsedDOTTime = 0.f;
    DOTTickInterval = 0.5f;
    DOTDuration = 0.7f;
    PassiveRegenTickInterval = 1.f;
}

void UGoddessOfLove::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

    AActor* AvatarActor = ActorInfo ? ActorInfo->AvatarActor.Get() : nullptr;
    if (!AvatarActor)
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
            }
        }
    }

    LastUseTime = World->GetTimeSeconds();

    DamageOrigin = ActorInfo->AvatarActor->GetActorLocation();

    ApplyInitialOverlap(ActorInfo);

    // Start DOT ticking
    ElapsedDOTTime = 0.f;
    World->GetTimerManager().SetTimer( DOTTimerHandle, this,&UGoddessOfLove::TickDamage,DOTTickInterval,true);

    World->GetTimerManager().SetTimer(PassiveRegenTimerHandle,this, &UGoddessOfLove::ApplyPassiveHealthRegen,PassiveRegenTickInterval, true);
}

void UGoddessOfLove::ApplyInitialOverlap(const FGameplayAbilityActorInfo* ActorInfo)
{
    if (!ActorInfo || !ActorInfo->AvatarActor.IsValid())
    {
        return;
    }

    AActor* OwnerActor = ActorInfo->AvatarActor.Get();
    DamageOrigin = OwnerActor->GetActorLocation(); // initialize origin at player

    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    // Draw the DOT area around the player at start
    DrawDebugSphere(World, DamageOrigin, AreaRadius, 32, FColor::Purple, false, DOTDuration);

    // Track overlapping enemies initially
    TArray<AActor*> OverlappedActors;
    TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
    ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));
    ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_GameTraceChannel1));

    TArray<AActor*> ActorsToIgnore;
    ActorsToIgnore.Add(OwnerActor);

    UKismetSystemLibrary::SphereOverlapActors(World, DamageOrigin, AreaRadius, ObjectTypes, APawn::StaticClass(), ActorsToIgnore, OverlappedActors);

    for (AActor* Actor : OverlappedActors)
    {
        if (!Actor || !Actor->ActorHasTag("Enemy"))
        {
            continue;
        }

        AffectedEnemies.AddUnique(Actor);

        DrawDebugLine(World, DamageOrigin, Actor->GetActorLocation(), FColor::Red, false, DOTDuration, 0, 2.f);
    }
}

void UGoddessOfLove::ApplyPassiveHealthRegen()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    // Get all player states in the world
    for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
    {
        APlayerController* PC = It->Get();
        if (!PC) continue;

        ARelicRunnersPlayerState* PS = PC->GetPlayerState<ARelicRunnersPlayerState>();
        if (!PS) continue;

        PS->PassiveHealthRegen();
    }
}


void UGoddessOfLove::TickDamage()
{
    if (!CurrentActorInfo || !CurrentActorInfo->AvatarActor.IsValid())
    {
        EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
        return;
    }

    AActor* OwnerActor = CurrentActorInfo->AvatarActor.Get();
    DamageOrigin = OwnerActor->GetActorLocation(); // update origin to player each tick

    UWorld* World = GetWorld();
    if (!World)
    {
        EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
        return;
    }

    const float DeltaTime = World->GetDeltaSeconds();
    ElapsedDOTTime += DeltaTime;

    for (int32 i = AffectedEnemies.Num() - 1; i >= 0; --i)
    {
        AActor* Enemy = AffectedEnemies[i].Get();
        if (!Enemy)
        {
            AffectedEnemies.RemoveAt(i);
            continue;
        }

        const float Distance = FVector::Dist(DamageOrigin, Enemy->GetActorLocation());

        if (Distance >= AreaRadius)
        {
            continue;
        }

        // Linear falloff
        float DamageMultiplier = FMath::Clamp(1.f - (Distance / AreaRadius), 0.f, 1.f);

        // Damage per second with minimum DPS
        const float MinDPS = 50.f;
        float EffectiveDPS = FMath::Max(Damage * DamageMultiplier, MinDPS);

        float ScaledDamage = EffectiveDPS * DeltaTime;

        if (ScaledDamage <= 0.f)
        {
            continue;
        }

        // Draw debug lines and strings from player to each enemy
        const FColor LineColor = FColor::MakeRedToGreenColorFromScalar(DamageMultiplier);
        DrawDebugLine(World, DamageOrigin, Enemy->GetActorLocation(), LineColor, false, DOTTickInterval, 0, 2.f);
        DrawDebugString(World, Enemy->GetActorLocation() + FVector(0, 0, 80.f), FString::Printf(TEXT("DPS x%.2f"), DamageMultiplier), nullptr, LineColor, DOTTickInterval);

        CalculateAbilityDamage(OwnerActor, Enemy, ScaledDamage);
    }

    if (ElapsedDOTTime >= DOTDuration)
    {
        EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
    }
}




void UGoddessOfLove::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);

    if (ActorInfo && ActorInfo->AvatarActor.IsValid())
    {
        ARelicRunnersPlayerController* RPC = Cast<ARelicRunnersPlayerController>(ActorInfo->PlayerController.Get());

        UWorld* World = ActorInfo->AvatarActor->GetWorld();
        if (World)
        {
            World->GetTimerManager().ClearTimer(DOTTimerHandle);
            World->GetTimerManager().ClearTimer(PassiveRegenTimerHandle);

            if (!bWasCancelled)
            {
                LastUseTime = World->GetTimeSeconds();

                if (ActorInfo->IsLocallyControlled())
                {
                    if (RPC)
                    {
                        RPC->PlayerHUD->StartUltimateCooldown(CooldownTimer);

                    }
                }
            }
        }
    }
}
