#include "EarthquakeAbility.h"
#include "GameFramework/Character.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "RelicRunners/PlayerState/RelicRunnersPlayerState.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "DrawDebugHelpers.h"
#include "TimerManager.h"

UEarthquakeAbility::UEarthquakeAbility()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

    Duration = 0.25f;
    ConeAngle = 90.f;
    LastUseTime = -100.f;
    CooldownTimer = 5.0f;
	Damage = 100.f;
}

void UEarthquakeAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

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

    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastUseTime < CooldownTimer)
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    LastUseTime = World->GetTimeSeconds();

    ApplyDamageAndStun();

    World->GetTimerManager().SetTimer(CooldownTimerHandle,[this, Handle, ActorInfo, ActivationInfo](){EndAbility(Handle, ActorInfo, ActivationInfo, true, false);}, Duration, false );
}


void UEarthquakeAbility::ApplyDamageAndStun()
{
    AActor* AvatarActor = GetAvatarActorFromActorInfo();
    if (!AvatarActor) return;

    FVector Origin = AvatarActor->GetActorLocation();
    FVector Forward = AvatarActor->GetActorForwardVector();
    Forward.Z = 0.f;
    Forward.Normalize();

    UWorld* World = AvatarActor->GetWorld();
    if (!World) return;

    TArray<AActor*> OverlappingActors;
    TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
    ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));
    ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_GameTraceChannel1));

    UKismetSystemLibrary::SphereOverlapActors(World, Origin, Radius, ObjectTypes, APawn::StaticClass(), { AvatarActor }, OverlappingActors);

    for (AActor* Actor : OverlappingActors)
    {
        if (!Actor) continue;

        if (!Actor->Tags.Contains("Enemy")) continue;

        FVector DirToTarget = Actor->GetActorLocation() - Origin;
        DirToTarget.Z = 0.f;
        float Distance = DirToTarget.Size();
        if (Distance > Radius) continue;

        DirToTarget.Normalize();


        float Dot = FVector::DotProduct(Forward, DirToTarget);
        float HalfAngleRad = FMath::DegreesToRadians(ConeAngle / 2.f);
        if (Dot < FMath::Cos(HalfAngleRad)) continue;


        CalculateAbilityDamage(AvatarActor, Actor, Damage);


        if (ACharacter* HitChar = Cast<ACharacter>(Actor))
        {
            FVector Knockback = DirToTarget * KnockbackStrength;
            Knockback.Z = VerticalKnockback;
            HitChar->LaunchCharacter(Knockback, true, true);

            /*if (auto Enemy = Cast<AEnemyCharacter>(HitChar))
            {
                Enemy->ReduceStunTime(StunDuration);
            }*/
        }
    }
    DrawDebugCone(World, Origin, Forward, Radius, FMath::DegreesToRadians(ConeAngle / 2.f), FMath::DegreesToRadians(ConeAngle / 2.f), 32, FColor::Orange, false, Duration + 0.1f);
}

void UEarthquakeAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);

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
                    if (ARelicRunnersPlayerController* RPC = Cast<ARelicRunnersPlayerController>(ActorInfo->PlayerController.Get()))
                    {
                        RPC->PlayerHUD->StartDamageCooldown(CooldownTimer);
                    }
                }
            }
        }
    }
    
}
