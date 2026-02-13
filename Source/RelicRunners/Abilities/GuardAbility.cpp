#include "GuardAbility.h"

#include "RelicRunners/RelicRunnersCharacter.h"
#include "RelicRunners/PlayerState/RelicRunnersPlayerState.h"
#include "RelicRunners/PlayerController/RelicRunnersPlayerController.h"
#include "RelicRunners/PlayerHUD/PlayerHUD.h"

UGuardAbility::UGuardAbility()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

    Cost = 30;
    CooldownDuration = 10.f;
    LastUseTime = -100.f;

    Duration = 0.f;
}

void UGuardAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,const FGameplayAbilityActorInfo* ActorInfo,const FGameplayAbilityActivationInfo ActivationInfo,const FGameplayEventData* TriggerEventData)
{
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

    ARelicRunnersCharacter* OwnerChar = Cast<ARelicRunnersCharacter>(ActorInfo ? ActorInfo->AvatarActor.Get() : nullptr);

    if (!OwnerChar || !OwnerChar->GetWorld())
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    const float Now = OwnerChar->GetWorld()->GetTimeSeconds();
    if (Now - LastUseTime < CooldownDuration)
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    ARelicRunnersPlayerState* PS = OwnerChar->GetPlayerState<ARelicRunnersPlayerState>();

    if (!PS || PS->GetPlayerResource() < Cost)
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    if (OwnerChar->HasAuthority())
    {
        PS->SetPlayerResource(PS->GetPlayerResource() - Cost);
        PS->UpdateRemoteHUDResource();

        OwnerChar->SetGuardActive(true);
        OwnerChar->GuardAngleDegrees = GuardAngleDegrees; 

        OwnerChar->GetWorldTimerManager().SetTimer(EndTimerHandle,FTimerDelegate::CreateUObject(this, &UGuardAbility::EndAbility,Handle, ActorInfo, ActivationInfo, true, false),GuardDuration,false);
    }
}

void UGuardAbility::EndAbility(const FGameplayAbilitySpecHandle Handle,const FGameplayAbilityActorInfo* ActorInfo,const FGameplayAbilityActivationInfo ActivationInfo,bool bReplicateEndAbility,bool bWasCancelled)
{
    if (ActorInfo && ActorInfo->AvatarActor.IsValid())
    {
        ARelicRunnersCharacter* OwnerChar = Cast<ARelicRunnersCharacter>(ActorInfo->AvatarActor.Get());

        UWorld* World = ActorInfo->AvatarActor->GetWorld();

        if (OwnerChar && World)
        {
            if (OwnerChar->HasAuthority())
            {
                OwnerChar->SetGuardActive(false);
            }

            if (!bWasCancelled)
            {
                LastUseTime = World->GetTimeSeconds();

                if (ActorInfo->IsLocallyControlled())
                {
                    if (ARelicRunnersPlayerController* RPC = Cast<ARelicRunnersPlayerController>(ActorInfo->PlayerController.Get()))
                    {
                        if (RPC->PlayerHUD)
                        {
                            RPC->PlayerHUD->StartDefenceCooldown(CooldownDuration);
                        }
                    }
                }
            }
        }
    }

    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
