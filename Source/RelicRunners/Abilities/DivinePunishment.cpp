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
	CooldownTimer = 10.f;
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


    CachedInstigatorController =
        ActorInfo->PlayerController.IsValid()
        ? ActorInfo->PlayerController.Get()
        : nullptr;

    LastUseTime = World->GetTimeSeconds();

    // ---- Start ticking damage ----
    const FVector Center = ActorInfo->AvatarActor->GetActorLocation();


    if (AbilityVFX)
    {
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            World,
            AbilityVFX,
            Center + FVector(0.f, 0.f, -100.f),
            FRotator::ZeroRotator,
            FVector(2.f),
            true,
            true
        );
    }

    KillEnemiesInRadius(Center);


}

void UDivinePunishment::KillEnemiesInRadius(const FVector& Center)
{
    CachedCenter = Center;
    DamageTicksElapsed = 0;

    UWorld* World = GetWorld();
    if (!World) return;

    World->GetTimerManager().SetTimer(
        DivineDamageTimerHandle,
        this,
        &UDivinePunishment::DivineDamageTick,
        1.f,
        true
    );
}


void UDivinePunishment::DivineDamageTick()
{
    UWorld* World = GetWorld();
    AActor* AvatarActor = GetAvatarActorFromActorInfo();
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
    ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));

    TArray<AActor*> ActorsToIgnore;
    ActorsToIgnore.Add(AvatarActor);

    UKismetSystemLibrary::SphereOverlapActors(
        World,
        CachedCenter,
        AreaRadius,
        ObjectTypes,
        ACharacter::StaticClass(),
        ActorsToIgnore,
        FoundActors
    );

    for (AActor* Actor : FoundActors)
    {
        if (!Actor || !Actor->ActorHasTag("Enemy")) continue;

        UGameplayStatics::ApplyDamage(
            Actor,
            DamageAmount,
            InstigatorController,
            AvatarActor,
            UDamageType::StaticClass()
        );
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
        ARelicRunnersPlayerController* RPC = Cast<ARelicRunnersPlayerController>(ActorInfo->PlayerController.Get());

        UWorld* World = ActorInfo->AvatarActor->GetWorld();
        if (World)
        {
            World->GetTimerManager().ClearTimer(DivineDamageTimerHandle);
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


