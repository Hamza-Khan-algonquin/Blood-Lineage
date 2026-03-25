#include "ImpunityAbility.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Character.h"
#include "RelicRunners/PlayerState/RelicRunnersPlayerState.h"
#include "TimerManager.h"

UImpunityAbility::UImpunityAbility()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

    Duration = 5.0f;
    CooldownDuration = 8.0f;
    LastUseTime = -100.f;
    CooldownTimer = 10.f;

}

void UImpunityAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);



    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastUseTime < CooldownTimer)
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }


    AActor* AvatarActor = ActorInfo ? ActorInfo->AvatarActor.Get() : nullptr;
    if (!AvatarActor)
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    UWorld* World = AvatarActor->GetWorld();
    if (!World)
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    ACharacter* AvatarChar = Cast<ACharacter>(GetAvatarActorFromActorInfo());
    if (!AvatarChar)
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    bIsActive = true;

    if (ActorInfo->AvatarActor->HasAuthority())
    {
        ARelicRunnersPlayerState* PS = nullptr;
        if (ActorInfo->PlayerController.IsValid())
        {
            PS = ActorInfo->PlayerController->GetPlayerState<ARelicRunnersPlayerState>();
        }
        PS->Multicast_SpawnVFX(FVector(0, 0, -90), FRotator::ZeroRotator, FVector(1.f), AbilityVFX, true);
        PS->Multicast_SpawnVFX(FVector(0, 0, -90), FRotator::ZeroRotator, FVector(1.f), AbilityVFX1, true);
    }
    

    OriginalSpeed = AvatarChar->GetCharacterMovement()->MaxWalkSpeed;
    AvatarChar->GetCharacterMovement()->MaxWalkSpeed *= 1.75f;
    AvatarChar->GetWorldTimerManager().SetTimer(SpeedResetTimer, FTimerDelegate::CreateUObject(this, &UImpunityAbility::EndAbility, Handle, ActorInfo, ActivationInfo, true, false), Duration, false);


}

void UImpunityAbility::ResetSpeed()
{

    ACharacter* AvatarChar = Cast<ACharacter>(GetAvatarActorFromActorInfo());
    if (AvatarChar)
    {
        AvatarChar->GetCharacterMovement()->MaxWalkSpeed = OriginalSpeed;
    }
}

void UImpunityAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{

    if (ActorInfo && ActorInfo->AvatarActor.IsValid())
    {
        UWorld* World = ActorInfo->AvatarActor->GetWorld();
        if (World)
        {
            if (!bWasCancelled)
            {
                LastUseTime = World->GetTimeSeconds();

                if (ActorInfo->IsLocallyControlled())
                {
                    if (ARelicRunnersPlayerController* RPC =
                        Cast<ARelicRunnersPlayerController>(ActorInfo->PlayerController.Get()))
                    {
                        RPC->PlayerHUD->StartDefenceCooldown(CooldownTimer);
                    }
                }
            }
        }

        ResetSpeed();
    }

    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
