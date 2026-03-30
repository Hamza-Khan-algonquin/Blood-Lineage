#include "MirrorOfDesireAbility.h"
#include "RelicRunners/RelicRunnersCharacter.h"
#include "RelicRunners/PlayerState/RelicRunnersPlayerState.h"
#include "RelicRunners/PlayerController/RelicRunnersPlayerController.h"
#include "RelicRunners/PlayerHUD/PlayerHUD.h"
#include "Kismet/GameplayStatics.h"

UMirrorOfDesireAbility::UMirrorOfDesireAbility()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

    CooldownDuration = 10.f;
    LastUseTime = -100.f;
    AreaRadius = 300.f;
    Duration = 6.f; 
}

void UMirrorOfDesireAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

    if (!CurrentActorInfo) return;

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

    ARelicRunnersPlayerState* PS = nullptr;
    if (ActorInfo->PlayerController.IsValid())
    {
        PS = ActorInfo->PlayerController->GetPlayerState<ARelicRunnersPlayerState>();
    }

    float FinalManaCost = CalculateAbilityMana(OwnerChar, Cost);

    if (!PS || PS->GetPlayerResource() < FinalManaCost)
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    // Spend resource
    PS->SetPlayerResource(PS->GetPlayerResource() - FinalManaCost);
    PS->UpdateRemoteHUDResource();
    AController* Controller = nullptr;

    if (CurrentActorInfo && CurrentActorInfo->PlayerController.IsValid())
    {
        Controller = CurrentActorInfo->PlayerController.Get();
    }

    if (Controller)
    {

        if (PS)
        {
            PS->Multicast_SpawnVFX(FVector(0), FRotator::ZeroRotator, FVector(2.f), AbilityVFX, true);
        }

    }

    PlayAbilityCastSound(ActorInfo);

    // Set timer to end ability automatically after Duration
    OwnerChar->GetWorldTimerManager().SetTimer(AbilityTimerHandle, [this, Handle, ActorInfo, ActivationInfo]()
        {
            EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
        }, Duration, false);

    // Hook into owner's damage event
    OwnerChar->OnTakeAnyDamage.AddDynamic(this, &UMirrorOfDesireAbility::OnOwnerTakeDamage);
}

// Called when the owner takes damage
void UMirrorOfDesireAbility::OnOwnerTakeDamage(AActor* DamagedActor, float DamageAmount, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser)
{
    if (!CurrentActorInfo) return;

    if (!DamagedActor) return;

    ARelicRunnersCharacter* OwnerChar = Cast<ARelicRunnersCharacter>(DamagedActor);
    if (!OwnerChar || !OwnerChar->HasAuthority()) return;

    ARelicRunnersPlayerState* PS = OwnerChar->GetPlayerState<ARelicRunnersPlayerState>();
    if (!PS) return;

    if (!DamageCauser || !DamageCauser->ActorHasTag("Enemy")) return;

    APawn* EnemyPawn = Cast<APawn>(DamageCauser);
    if (!EnemyPawn || EnemyPawn->GetRootComponent()->GetCollisionObjectType() != ECC_GameTraceChannel1)
        return;


    float DamageToReflect = DamageAmount * 50.f; 

    // Apply reflected damage
    CalculateAbilityDamage(OwnerChar, EnemyPawn, DamageToReflect);

    DrawDebugLine(DamagedActor->GetWorld(), DamagedActor->GetActorLocation(), EnemyPawn->GetActorLocation(), FColor::Red, false, 1.f, 0, 2.f);
}



void UMirrorOfDesireAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
    if (ActorInfo && ActorInfo->AvatarActor.IsValid())
    {
        ARelicRunnersCharacter* OwnerChar = Cast<ARelicRunnersCharacter>(ActorInfo->AvatarActor.Get());
        UWorld* World = ActorInfo->AvatarActor->GetWorld();

        if (OwnerChar && World)
        {
            // Clear the timer
            World->GetTimerManager().ClearTimer(AbilityTimerHandle);

            // Unhook from damage event
            OwnerChar->OnTakeAnyDamage.RemoveDynamic(this, &UMirrorOfDesireAbility::OnOwnerTakeDamage);

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
