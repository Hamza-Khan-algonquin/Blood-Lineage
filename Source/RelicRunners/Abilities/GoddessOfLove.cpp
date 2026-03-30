// Fill out your copyright notice in the Description page of Project Settings.


#include "GoddessOfLove.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "RelicRunners/PlayerState/RelicRunnersPlayerState.h"
#include "Kismet/KismetSystemLibrary.h"
#include "DrawDebugHelpers.h"
#include "TimerManager.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"

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

    if (!CurrentActorInfo) return;

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


    PlayAbilityCastSound(ActorInfo);


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

    ACharacter* Character = Cast<ACharacter>(AvatarActor);
    if (Character)
    {
        USkeletalMeshComponent* Mesh1 = Character->GetMesh();
        if (Mesh1 && OverlayMaterial)
        {
            // Create dynamic material
            OverlayMID = UMaterialInstanceDynamic::Create(OverlayMaterial, this);

            // Apply to mesh (overlay or replace slot)
            Mesh1->SetOverlayMaterial(OverlayMID);
        }
    }

    LastUseTime = World->GetTimeSeconds();

    DamageOrigin = ActorInfo->AvatarActor->GetActorLocation();

    World->GetTimerManager().SetTimer(OverlapTimer, this, &UGoddessOfLove::ApplyInitialOverlap, DOTTickInterval, true);

    // Start DOT ticking
    ElapsedDOTTime = 0.f;
    World->GetTimerManager().SetTimer(DOTTimerHandle, this,&UGoddessOfLove::TickDamage,DOTTickInterval,true);

    World->GetTimerManager().SetTimer(PassiveRegenTimerHandle,this, &UGoddessOfLove::ApplyPassiveHealthRegen,PassiveRegenTickInterval, true);
}

void UGoddessOfLove::ApplyInitialOverlap()
{
    if (!GetCurrentActorInfo() || !GetCurrentActorInfo()->AvatarActor.IsValid())
    {
        return;
    }

    if (!CurrentActorInfo) return;

    AActor* OwnerActor = GetCurrentActorInfo()->AvatarActor.Get();
    DamageOrigin = OwnerActor->GetActorLocation(); // initialize origin at player

    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    // Draw the DOT area around the player at start
    //DrawDebugSphere(World, DamageOrigin, AreaRadius, 32, FColor::Purple, false, DOTDuration);

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

        //DrawDebugLine(World, DamageOrigin, Actor->GetActorLocation(), FColor::Red, false, DOTDuration, 0, 2.f);
    }
}

void UGoddessOfLove::ApplyPassiveHealthRegen()
{
    if (!CurrentActorInfo) return;

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


// In your UGoddessOfLove class
TMap<TWeakObjectPtr<AActor>, UNiagaraComponent*> ActiveBeams;

void UGoddessOfLove::TickDamage()
{
    if (!CurrentActorInfo || !CurrentActorInfo->AvatarActor.IsValid())
    {
        EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
        return;
    }

    AActor* OwnerActor = CurrentActorInfo->AvatarActor.Get();
    DamageOrigin = OwnerActor->GetActorLocation();

    UWorld* World = GetWorld();
    if (!World)
    {
        EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
        return;
    }

    const float DeltaTime = World->GetDeltaSeconds();
    ElapsedDOTTime += DeltaTime;

    ARelicRunnersPlayerState* PS = nullptr;
    if (CurrentActorInfo->PlayerController.IsValid())
    {
        PS = CurrentActorInfo->PlayerController->GetPlayerState<ARelicRunnersPlayerState>();
    }

    for (int32 i = AffectedEnemies.Num() - 1; i >= 0; --i)
    {
        AActor* Enemy = AffectedEnemies[i].Get();
        if (!Enemy)
        {
            // Destroy beam if enemy is gone
            if (ActiveBeams.Contains(Enemy))
            {
                if (UNiagaraComponent* Beam = ActiveBeams[Enemy])
                {
                    Beam->DestroyComponent();
                }
                ActiveBeams.Remove(Enemy);
            }
            AffectedEnemies.RemoveAt(i);
            continue;
        }

        const float Distance = FVector::Dist(DamageOrigin, Enemy->GetActorLocation());
        if (Distance >= AreaRadius)
            continue;

        float DamageMultiplier = FMath::Clamp(1.f - (Distance / AreaRadius), 0.f, 1.f);
        float EffectiveDPS = FMath::Max(Damage * DamageMultiplier, 50.f);
        float ScaledDamage = EffectiveDPS * DeltaTime;

        if (ScaledDamage > 0.f)
        {
            CalculateAbilityDamage(OwnerActor, Enemy, ScaledDamage);

            if (PS && AbilityVFX)
            {
                UNiagaraComponent* BeamComp = nullptr;

                APawn* Pawn = PS->GetPawn();
                if (!Pawn) continue;

                USceneComponent* RootComp = Pawn->GetRootComponent();
                if (!RootComp) continue;

                // Spawn new beam if it doesn't exist yet
                if (!ActiveBeams.Contains(Enemy) || !ActiveBeams[Enemy])
                {
                    BeamComp = UNiagaraFunctionLibrary::SpawnSystemAttached(
                        AbilityVFX,
                        RootComp,
                        NAME_None,
                        FVector::ZeroVector,
                        FRotator::ZeroRotator,
                        EAttachLocation::KeepRelativeOffset,
                        true,
                        true,
                        ENCPoolMethod::None
                    );

                    ActiveBeams.Add(Enemy, BeamComp);
                }
                else
                {
                    BeamComp = ActiveBeams[Enemy];
                }

                // Update endpoint to enemy location each tick
                if (BeamComp)
                {
                    BeamComp->SetVectorParameter(TEXT("EndPoint"), Enemy->GetActorLocation());
                }
            }
        }
    }

    // Destroy any remaining beams after DOT ends
    if (ElapsedDOTTime >= DOTDuration)
    {
        for (auto& Pair : ActiveBeams)
        {
            if (UNiagaraComponent* Beam = Pair.Value)
            {
                Beam->DestroyComponent();
            }
        }
        ActiveBeams.Empty();

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

            ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo());
            if (Character)
            {
                USkeletalMeshComponent* Mesh = Character->GetMesh();
                if (Mesh)
                {
                    // Remove overlay material
                    Mesh->SetOverlayMaterial(nullptr);
                }
            }

            OverlayMID = nullptr;


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
