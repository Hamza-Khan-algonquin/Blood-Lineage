#include "GuardAbility.h"

#include "RelicRunners/RelicRunnersCharacter.h"
#include "RelicRunners/PlayerState/RelicRunnersPlayerState.h"
#include "RelicRunners/PlayerController/RelicRunnersPlayerController.h"
#include "RelicRunners/PlayerHUD/PlayerHUD.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "RelicRunners/Enemy/EnemyPawn.h"

UGuardAbility::UGuardAbility()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

    CooldownDuration = 12.f;
    LastUseTime = -100.f;
    Duration = 0.f;
    Damage = 25.f;
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

    ACharacter* AvatarChar = Cast<ACharacter>(GetAvatarActorFromActorInfo());
    if (!AvatarChar)
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

    float FinalManaCost = CalculateAbilityMana(AvatarChar, Cost);

    if (!PS || PS->GetPlayerResource() < FinalManaCost)
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    // Spend resource
    PS->SetPlayerResource(PS->GetPlayerResource() - FinalManaCost);
    PS->UpdateRemoteHUDResource();

    if (AbilityAnim)
    {
        PlayAbilityAnimation(CurrentActorInfo, AbilityAnim, 1.f);
    }


    OriginalSpeed = AvatarChar->GetCharacterMovement()->MaxWalkSpeed;
    AvatarChar->GetCharacterMovement()->MaxWalkSpeed *= 1.5f;

    OwnerChar->GetWorldTimerManager().SetTimer(ChargeTickHandle,FTimerDelegate::CreateUObject(this,&UGuardAbility::HandleChargeTick,Handle,ActorInfo,ActivationInfo),0.05f,true);

    // End ability after duration
    OwnerChar->GetWorldTimerManager().SetTimer(EndTimerHandle,FTimerDelegate::CreateUObject(this,&UGuardAbility::EndAbility, Handle,ActorInfo, ActivationInfo,true,false),ChargeDuration, false);
    
}

void UGuardAbility::ResetSpeed()
{
    ACharacter* AvatarChar = Cast<ACharacter>(GetAvatarActorFromActorInfo());
    if (AvatarChar)
    {
        AvatarChar->GetCharacterMovement()->MaxWalkSpeed = OriginalSpeed;
    }
}

void UGuardAbility::HandleChargeTick(FGameplayAbilitySpecHandle Handle,const FGameplayAbilityActorInfo* ActorInfo, FGameplayAbilityActivationInfo ActivationInfo)
{
    if (!ActorInfo || !ActorInfo->AvatarActor.IsValid())
        return;

    ARelicRunnersCharacter* OwnerChar = Cast<ARelicRunnersCharacter>(ActorInfo->AvatarActor.Get());

    if (!OwnerChar)
        return;

    // Only work if player is actually moving
    FVector Velocity = OwnerChar->GetVelocity();
    Velocity.Z = 0.f;

    if (Velocity.SizeSquared() < 25.f) // small threshold
        return;

    FVector Start = OwnerChar->GetActorLocation();
    FVector Forward = OwnerChar->GetActorForwardVector();
    FVector End = Start + Forward * CheckDistance;

    FCollisionShape Sphere = FCollisionShape::MakeSphere(CheckRadius);

    TArray<FHitResult> Hits;

    bool bHit = OwnerChar->GetWorld()->SweepMultiByChannel(Hits, Start, End, FQuat::Identity, ECC_GameTraceChannel1, Sphere);

    if (!bHit)
        return;

    for (const FHitResult& Hit : Hits)
    {
        APawn* HitPawn = Cast<APawn>(Hit.GetActor());

        if (!HitPawn)
            continue;

        if (!HitPawn->ActorHasTag("Enemy"))
            continue;

        FVector ToEnemy = HitPawn->GetActorLocation() - Start;
        ToEnemy.Z = 0.f;

        FVector DirToEnemy = ToEnemy.GetSafeNormal();

        float Dot = FVector::DotProduct(Forward, DirToEnemy);

        const float ConeThreshold = 0.7f; 

        if (Dot < ConeThreshold)
            continue;

        // Apply damage
        CalculateAbilityDamage(OwnerChar, HitPawn, Damage);

        // Apply knockback
        if (AEnemyPawn* HitCharacter = Cast<AEnemyPawn>(HitPawn))
        {
            FVector Direction = DirToEnemy;

            HitCharacter->ApplyForceToEnemy(Direction * KnockbackStrength + FVector(0.f, 0.f, 400.f));
        }
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
            World->GetTimerManager().ClearTimer(ChargeTickHandle);
            World->GetTimerManager().ClearTimer(EndTimerHandle);

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
        ResetSpeed();
    }

    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
