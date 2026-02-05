#include "DivinePunishment.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "RelicRunners/PlayerState/RelicRunnersPlayerState.h"
#include "Kismet/KismetSystemLibrary.h"
#include "DrawDebugHelpers.h"
#include "TimerManager.h"

UDivinePunishment::UDivinePunishment()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	Duration = 5.f;
	LastUseTime = -100.f;
	CooldownTimer = 5.f;
	Cost = 20;
	DamageAmount = 100.f;
    AreaRadius = 500.f;

    static ConstructorHelpers::FObjectFinder<UNiagaraSystem> VFXAsset(TEXT("/Game/Particles/NiagaraSystems/NS_DivinePunishment.NS_DivinePunishment"));

    if (VFXAsset.Succeeded())
    {
        AbilityVFX = VFXAsset.Object;
    }
}

void UDivinePunishment::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

    if (!ActorInfo || !ActorInfo->AvatarActor.IsValid())
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

    if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
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

    // Must have PlayerState and enough resource
    if (!PS || PS->GetPlayerResource() < Cost)
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    // Spend resource
    PS->SetPlayerResource(PS->GetPlayerResource() - Cost);
    PS->UpdateRemoteHUDResource();

    KillEnemiesInRadius(ActorInfo->AvatarActor->GetActorLocation());

}

void UDivinePunishment::KillEnemiesInRadius(const FVector& Center)
{
    UWorld* World = GetWorld();
    if (!World) return;

    AActor* AvatarActor = GetAvatarActorFromActorInfo();
    if (!AvatarActor) return;

    //DrawDebugSphere(World, Center, AreaRadius, 32, FColor::Purple, false, Duration, 0, 4.f);

    UNiagaraFunctionLibrary::SpawnSystemAtLocation(AvatarActor->GetWorld(),AbilityVFX,FVector(Center.X, Center.Y, Center.Z - 100),FRotator(0, 0, 0),FVector(2.f),true,true);

    DamageTicksElapsed = 0;

    World->GetTimerManager().SetTimer(DivineDamageTimerHandle,[this, World, Center, AvatarActor]()
        {
            if (!World || !AvatarActor)
            {
                World->GetTimerManager().ClearTimer(DivineDamageTimerHandle);
                return;
            }

            DamageTicksElapsed++;

            TArray<AActor*> FoundActors;

            TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
            ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));

            TArray<AActor*> ActorsToIgnore;
            ActorsToIgnore.Add(AvatarActor);

            UKismetSystemLibrary::SphereOverlapActors(World,Center,AreaRadius, ObjectTypes, ACharacter::StaticClass(), ActorsToIgnore, FoundActors);

            for (AActor* Actor : FoundActors)
            {
                if (!Actor || !Actor->ActorHasTag("Enemy")) continue;

                UGameplayStatics::ApplyDamage( Actor,DamageAmount,AvatarActor->GetInstigatorController(), AvatarActor,UDamageType::StaticClass());
            }

            // Stop after 5 seconds
            if (DamageTicksElapsed >= Duration)
            {
                World->GetTimerManager().ClearTimer(DivineDamageTimerHandle);

                EndAbility(CurrentSpecHandle,CurrentActorInfo,CurrentActivationInfo,false,false );
            }
        }, 1.f, true);
}


void UDivinePunishment::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);

    if (ActorInfo && ActorInfo->AvatarActor.IsValid())
    {
        ARelicRunnersPlayerController* RPC = Cast<ARelicRunnersPlayerController>(ActorInfo->PlayerController.Get());

        UWorld* World = ActorInfo->AvatarActor->GetWorld();
        if (World)
        {
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


