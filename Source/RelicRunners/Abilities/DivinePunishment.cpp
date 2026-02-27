#include "DivinePunishment.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "RelicRunners/PlayerState/RelicRunnersPlayerState.h"
#include "Kismet/KismetSystemLibrary.h"
#include "DrawDebugHelpers.h"
#include "RelicRunners/RelicRunnersCharacter.h"
#include "TimerManager.h"
#include "GameFramework/CharacterMovementComponent.h" 

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

    static ConstructorHelpers::FObjectFinder<UAnimMontage> AnimAsset(TEXT("/Game/Assets/Characters/Melee/Animations/melee_divinepunishment_down_Montage.melee_divinepunishment_down_Montage"));

    if (AnimAsset.Succeeded())
    {
        AbilityAnim = AnimAsset.Object;
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

    CachedInstigatorController = ActorInfo->PlayerController.IsValid() ? ActorInfo->PlayerController.Get() : nullptr;

    if (AbilityAnim)
    {
        UAnimMontage* Montage = PlayAbilityAnimation(CurrentActorInfo, AbilityAnim);

        if (Montage)
        {
            if (!World) return;

            // Timer to pause after 2 seconds
            World->GetTimerManager().SetTimer(PauseMontageTimerHandle,FTimerDelegate::CreateLambda([this, Montage]()
                    {
                        if (ARelicRunnersCharacter* Char = Cast<ARelicRunnersCharacter>(CurrentActorInfo->AvatarActor.Get()))
                        {
                            if (UAnimInstance* AnimInstance = Char->GetMesh()->GetAnimInstance())
                            {
                                AnimInstance->Montage_Pause(Montage);

                                // Timer to resume after 3 seconds
                                GetWorld()->GetTimerManager().SetTimer( ResumeMontageTimerHandle,FTimerDelegate::CreateLambda([AnimInstance, Montage](){AnimInstance->Montage_Resume(Montage); }),4.5f,false);
                            }
                        }
                    }), 2.f, false);
        }
    }

    if (ACharacter* Char = Cast<ACharacter>(ActorInfo->AvatarActor.Get()))
    {
        if (UCharacterMovementComponent* MoveComp = Char->GetCharacterMovement())
        {
            MoveComp->DisableMovement();
        }
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



    UWorld* World = GetWorld();
    if (!World) return;

    World->GetTimerManager().SetTimer(DivineDamageTimerHandle,this, &UDivinePunishment::DivineDamageTick, 1.f,true);
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
    ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_GameTraceChannel1));

    TArray<AActor*> ActorsToIgnore;
    ActorsToIgnore.Add(AvatarActor);

    UKismetSystemLibrary::SphereOverlapActors(World,CachedCenter,AreaRadius,ObjectTypes,APawn::StaticClass(), ActorsToIgnore, FoundActors);

    for (AActor* Actor : FoundActors)
    {
        if (!Actor || !Actor->ActorHasTag("Enemy")) continue;

        UGameplayStatics::ApplyDamage(Actor,DamageAmount,InstigatorController, AvatarActor,UDamageType::StaticClass());
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
        if (ACharacter* Char = Cast<ACharacter>(ActorInfo->AvatarActor.Get()))
        {
            if (UCharacterMovementComponent* MoveComp = Char->GetCharacterMovement())
            {
                MoveComp->SetMovementMode(MOVE_Walking);
            }
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




