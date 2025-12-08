#include "EarthquakeAbility.h"
#include "GameFramework/Character.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
//#include "RelicRunners/Enemy/EnemyCharacter.h"
#include "DrawDebugHelpers.h"
#include "TimerManager.h"

UEarthquakeAbility::UEarthquakeAbility()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

    Duration = 8.0f;
    CooldownDuration = 5.0f;
    ConeAngle = 90.f;

}

void UEarthquakeAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{

    if (bIsOnCooldown)
    {
        if (GEngine)
            GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, TEXT("Earthquake ON COOLDOWN"));
        return;
    }

    bIsOnCooldown = true; 

    AActor* AvatarActor = ActorInfo->AvatarActor.Get();
    if (!AvatarActor) return;


    ApplyDamageAndStun();

    if (UWorld* World = AvatarActor->GetWorld())
    {
        World->GetTimerManager().ClearTimer(CooldownTimerHandle);
        World->GetTimerManager().SetTimer(CooldownTimerHandle, [this](){bIsOnCooldown = false;if (GEngine)GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Green, TEXT("Earthquake READY"));}, CooldownDuration,false);
    }

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

    UKismetSystemLibrary::SphereOverlapActors(World, Origin, Radius, ObjectTypes, ACharacter::StaticClass(), { AvatarActor }, OverlappingActors);

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

        
        UGameplayStatics::ApplyDamage(Actor, DamagePerTick, AvatarActor->GetInstigatorController(), AvatarActor, nullptr);

        
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
    DrawDebugCone(World, Origin, Forward, Radius, FMath::DegreesToRadians(ConeAngle / 2.f), FMath::DegreesToRadians(ConeAngle / 2.f), 32, FColor::Orange, false, StunDuration + 0.1f);
}

void UEarthquakeAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
