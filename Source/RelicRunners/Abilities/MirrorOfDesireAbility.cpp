#include "MirrorOfDesireAbility.h"
#include "RelicRunners/RelicRunnersCharacter.h"
#include "RelicRunners/PlayerState/RelicRunnersPlayerState.h"
#include "RelicRunners/PlayerController/RelicRunnersPlayerController.h"
#include "RelicRunners/PlayerHUD/PlayerHUD.h"

UMirrorOfDesireAbility::UMirrorOfDesireAbility()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

    Cost = 40;
    CooldownDuration = 10.f;
    LastUseTime = -100.f;

    Duration = 0.f; 
}

void UMirrorOfDesireAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,const FGameplayAbilityActorInfo* ActorInfo,const FGameplayAbilityActivationInfo ActivationInfo,const FGameplayEventData* TriggerEventData)
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

        OwnerChar->MirrorReflectPercent = ReflectPercent;
        OwnerChar->MirrorRadius = MirrorRadius;
        OwnerChar->SetMirrorActive(true);

        OwnerChar->GetWorldTimerManager().SetTimer(EndTimerHandle,FTimerDelegate::CreateUObject(this, &UMirrorOfDesireAbility::EndAbility, Handle, ActorInfo, ActivationInfo, true, false),MirrorDuration,false);
    }
}

void UMirrorOfDesireAbility::EndAbility(const FGameplayAbilitySpecHandle Handle,const FGameplayAbilityActorInfo* ActorInfo,const FGameplayAbilityActivationInfo ActivationInfo,bool bReplicateEndAbility,bool bWasCancelled)
{
    if (ActorInfo && ActorInfo->AvatarActor.IsValid())
    {
        ARelicRunnersCharacter* OwnerChar = Cast<ARelicRunnersCharacter>(ActorInfo->AvatarActor.Get());
        UWorld* World = ActorInfo->AvatarActor->GetWorld();

        if (OwnerChar && World)
        {
            if (OwnerChar->HasAuthority())
            {
                OwnerChar->SetMirrorActive(false);
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
