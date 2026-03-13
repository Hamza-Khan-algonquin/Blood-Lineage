#include "VengefulDance.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "AbilitySystemComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "RelicRunners/PlayerState/RelicRunnersPlayerState.h"
#include "DrawDebugHelpers.h"
#include "TimerManager.h"
#include "RelicRunners/Enemy/EnemyPawn.h"

UVengefulDance::UVengefulDance()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

    AbilityName = FName("VengefulDance");

    DamageInterval = 0.5f;
    KnockbackStrength = 3000.f;
    Duration = 1.f;
    Damage = 25.f;
    AreaRadius = 300.f;
    LastUseTime = -100.f;
    CooldownTimer = 10.f;
    Cost = 40;


}

void UVengefulDance::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

    if (!ActorInfo || !ActorInfo->AvatarActor.IsValid())
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

    if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
    {
        UE_LOG(LogTemp, Warning, TEXT("[VengefulDance] CommitAbility failed"));
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

    float FinalManaCost = CalculateAbilityMana(AvatarActor, Cost);

    // Spend resource
    PS->SetPlayerResource(PS->GetPlayerResource() - FinalManaCost);
    PS->UpdateRemoteHUDResource();

    if (AbilityAnim)
    {
        PlayAbilityAnimation(CurrentActorInfo, AbilityAnim);

    }

    AvatarActor->GetWorldTimerManager().SetTimer(DamageTickTimer, [this]() { ApplyRingDamage(); }, DamageInterval, true);

    if (ActorInfo->AvatarActor->HasAuthority())
    {
        PS->Multicast_SpawnVFX(AvatarActor->GetActorLocation(), AvatarActor->GetActorRotation() + FRotator(0, 0, 90), FVector(1.5f), AbilityVFX, false);
    }

    AvatarActor->GetWorldTimerManager().SetTimer(EndTimer, FTimerDelegate::CreateUObject(this, &UVengefulDance::EndAbility, Handle, ActorInfo, ActivationInfo, true, false), Duration, false);

    //DrawDebugCircle(AvatarActor->GetWorld(), AvatarActor->GetActorLocation(), AreaRadius, 64, FColor::Red, false, Duration, 0, 5.f, FVector(1, 0, 0), FVector(0, 1, 0), false);

}

void UVengefulDance::ApplyRingDamage()
{
    AActor* AvatarActor = GetAvatarActorFromActorInfo(); // Get the actor that owns this ability
    if (!AvatarActor) return; // Exit if invalid

    UWorld* World = AvatarActor->GetWorld(); // Get the world
    if (!World) return; // Exit if world is invalid

    FVector Origin = AvatarActor->GetActorLocation(); // Center of the damage/knockback
    TArray<AActor*> OverlappingActors; // Array to store actors within the radius

    TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes; // Which kinds of objects the sphere check should detect
    ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_GameTraceChannel1));

    // Find all actors overlapping a sphere around the origin, ignoring the owner
    UKismetSystemLibrary::SphereOverlapActors(World, Origin, AreaRadius, ObjectTypes, APawn::StaticClass(), { AvatarActor }, OverlappingActors);

    for (AActor* Actor : OverlappingActors) // Loop through all overlapping actors
    {
        if (!Actor || Actor == AvatarActor) // Skip invalid actors or the owner
            continue;

        if (!Actor->Tags.Contains("Enemy")) continue;
        FVector Diff = Actor->GetActorLocation() - Origin; // Vector from origin to actor
        Diff.Z = 0.f; // Ignore vertical distance

        if (Diff.Size() > AreaRadius) // Skip actors outside the area radius
            continue;

		CalculateAbilityDamage(AvatarActor, Actor, Damage); 

        if (AEnemyPawn* HitCharacter = Cast<AEnemyPawn>(Actor)) // Cast as EnemyCharacter
        {
            FVector Direction = Diff.GetSafeNormal();
            HitCharacter->ApplyForceToEnemy(Direction * KnockbackStrength + FVector(0, 0, 400));
        }
    }

}

void UVengefulDance::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{

    if (ActorInfo && ActorInfo->AvatarActor.IsValid())
    {
        UWorld* World = ActorInfo->AvatarActor->GetWorld();
        if (World)
        {
            World->GetTimerManager().ClearTimer(DamageTickTimer);
            World->GetTimerManager().ClearTimer(EndTimer);

            if (!bWasCancelled)
            {
                LastUseTime = World->GetTimeSeconds();

                if (ActorInfo->IsLocallyControlled())
                {
                    if (ARelicRunnersPlayerController* RPC =
                        Cast<ARelicRunnersPlayerController>(ActorInfo->PlayerController.Get()))
                    {
                        RPC->PlayerHUD->StartUtilityCooldown(CooldownTimer);
                    }
                }
            }
        }
    }

    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
