#include "DivinePunishment.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h" 
#include "Kismet/GameplayStatics.h"
#include "RelicRunners/PlayerState/RelicRunnersPlayerState.h"
#include "Kismet/KismetSystemLibrary.h"
#include "DrawDebugHelpers.h"
#include "RelicRunners/RelicRunnersCharacter.h"
#include "TimerManager.h"


UDivinePunishment::UDivinePunishment()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	Duration = 5.f;
	LastUseTime = -100.f;
	CooldownTimer = 15.f;
	Damage = 300.f;
    AreaRadius = 600.f;
}

void UDivinePunishment::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
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

    UWorld* World = GetWorld();
    if (!World) return;

    if (World->GetTimeSeconds() - LastUseTime < CooldownTimer)
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

    float FinalManaCost = CalculateAbilityMana(AvatarActor, Cost);

    if (!PS || PS->GetPlayerResource() < FinalManaCost)
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    // Spend resource
    PS->SetPlayerResource(PS->GetPlayerResource() - FinalManaCost);
    PS->UpdateRemoteHUDResource();
    PS->SetInvincible(true);
    PlayAbilityCastSound(ActorInfo);

    CachedInstigatorController = ActorInfo->PlayerController.IsValid() ? ActorInfo->PlayerController.Get() : nullptr;

    if (AbilityAnim)
    {
        PlayAbilityAnimation(CurrentActorInfo, AbilityAnim, 1.f);
    }

    if (APlayerController* PC = Cast<APlayerController>(AvatarActor->GetInstigatorController()))
    {
        PC->DisableInput(PC);
    }

    LastUseTime = World->GetTimeSeconds();

    //  Start ticking damage
    const FVector Center = ActorInfo->AvatarActor->GetActorLocation();


    if (ActorInfo->AvatarActor->HasAuthority())
    {
        if (!World) return;

        // Set a 2-second delay before spawning VFX

        World->GetTimerManager().SetTimer(VFXDelayHandle,FTimerDelegate::CreateLambda([PS, Center, this]()
                {
                    if (PS)
                    {
                        PS->Multicast_SpawnVFX(Center + FVector(0.f, 0.f, -100.f),FRotator::ZeroRotator,FVector(2.f),AbilityVFX,false);
                    }
                }),1.5f, false );
    }

    if (World)
    {

        World->GetTimerManager().SetTimer(DamageDelayHandle, FTimerDelegate::CreateLambda([this, Center](){KillEnemiesInRadius(Center);}),1.5f,false );
    }

}



void UDivinePunishment::KillEnemiesInRadius(const FVector& Center)
{
    CachedCenter = Center;
    DamageTicksElapsed = 0;

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


    UWorld* World = GetWorld();
    if (!World) return;

    World->GetTimerManager().SetTimer(DivineDamageTimerHandle,this, &UDivinePunishment::DivineDamageTick, 1.f,true);
}


void UDivinePunishment::DivineDamageTick()
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

    UWorld* World = AvatarActor->GetWorld();
    if (!World) return;

    AController* InstigatorController = CachedInstigatorController.Get();

    if (!World || !AvatarActor || !InstigatorController)
    {
        World->GetTimerManager().ClearTimer(DivineDamageTimerHandle);
        EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
        return;
    }

    DamageTicksElapsed++;

    TArray<AActor*> FoundActors;
    TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
    ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_GameTraceChannel1));

    TArray<AActor*> ActorsToIgnore;
    ActorsToIgnore.Add(AvatarActor);

    UKismetSystemLibrary::SphereOverlapActors(World,CachedCenter,AreaRadius,ObjectTypes,APawn::StaticClass(), ActorsToIgnore, FoundActors);

    for (AActor* Actor : FoundActors)
    {
        if (!Actor || !Actor->ActorHasTag("Enemy")) continue;

        CalculateAbilityDamage(AvatarActor, Actor, Damage);
    }

    if (DamageTicksElapsed >= Duration)
    {
        World->GetTimerManager().ClearTimer(DivineDamageTimerHandle);
        EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
    }
}



void UDivinePunishment::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);

    if (ActorInfo && ActorInfo->AvatarActor.IsValid())
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

        if (APlayerController* PC = Cast<APlayerController>(ActorInfo->AvatarActor->GetInstigatorController()))
        {
            PC->EnableInput(PC);
        }

        ARelicRunnersPlayerController* RPC = Cast<ARelicRunnersPlayerController>(ActorInfo->PlayerController.Get());

        UWorld* World = ActorInfo->AvatarActor->GetWorld();

        if (World)
        {
            World->GetTimerManager().ClearTimer(DivineDamageTimerHandle);
            World->GetTimerManager().ClearTimer(VFXDelayHandle);
            World->GetTimerManager().ClearTimer(ResumeMontageTimerHandle);
            World->GetTimerManager().ClearTimer(PauseMontageTimerHandle);
            World->GetTimerManager().ClearTimer(DamageDelayHandle);
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




