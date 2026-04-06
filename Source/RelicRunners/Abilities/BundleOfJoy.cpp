#include "BundleOfJoy.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "RelicRunners/PlayerState/RelicRunnersPlayerState.h"
#include "Kismet/KismetSystemLibrary.h"
#include "DrawDebugHelpers.h"
#include "TimerManager.h"
#include "RelicRunners/Enemy/EnemyPawn.h"
#include "RelicRunners/Enemy/EnemyBoidMovementComponent.h"

UBundleOfJoy::UBundleOfJoy()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

    AttractionRadius = 600.f;
    ExplosionRadius = 400.f;
    AttractionStrength = 1500.f;
    Damage = 200.f;
    Duration = 5.f;
    LastUseTime = -100.f;
    CooldownTimer = 12.f;
}

void UBundleOfJoy::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

    if (!CurrentActorInfo) return;

    float CurrentTime = ActorInfo->AvatarActor->GetWorld()->GetTimeSeconds();

    if (CurrentTime - LastUseTime < CooldownTimer)
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }
    
    AActor* AvatarActor = GetAvatarActorFromActorInfo(); // Get the actor this ability belongs to
    if (!AvatarActor) // If no valid actor
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true); // End the ability
        return;
    }

    UWorld* World = AvatarActor->GetWorld();
    if (!World)
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }


    ARelicRunnersPlayerState* PS = nullptr;
    if (ActorInfo->PlayerController.IsValid())
    {
        PS = ActorInfo->PlayerController->GetPlayerState<ARelicRunnersPlayerState>();
    }

    float FinalManaCost = CalculateAbilityMana(AvatarActor, Cost);

    if (!PS || PS->GetPlayerResource() < FinalManaCost)
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    // Spend resource
    PS->SetPlayerResource(PS->GetPlayerResource() - FinalManaCost);
    PS->UpdateRemoteHUDResource();

    PlayAbilityCastSound(ActorInfo);

    AttractionCenter = AvatarActor->GetActorLocation() + AvatarActor->GetActorForwardVector() * 300.f; // Set spawn point 300 units in front of the actor


    StartLocation = AvatarActor->GetActorLocation() + AvatarActor->GetActorForwardVector() * 100.f + FVector(0, 0, 50);
    Velocity = AvatarActor->GetActorForwardVector() * 700.f + FVector(0, 0, 600.f);
    bIsFlying = true;

    if (AbilityAnim)
    {
        PlayAbilityAnimation(CurrentActorInfo, AbilityAnim, 1.0f);
    }

    AvatarActor->GetWorldTimerManager().SetTimer(AttractTimer, this, &UBundleOfJoy::TickThrown, 0.016f, true);

}

void UBundleOfJoy::TickThrown()
{
    if (!bIsFlying) return;

    const FGameplayAbilityActorInfo* ActorInfo = GetCurrentActorInfo();
    if (!ActorInfo)
    {
        EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
        return;
    }

    AActor* AvatarActor = ActorInfo->AvatarActor.Get();
    if (!IsValid(AvatarActor))
    {
        EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
        return;
    }

    UWorld* World = AvatarActor->GetWorld();
    if (!World) return;

    float Delta = World->GetDeltaSeconds();

    Velocity += FVector(0, 0, -980.f) * Delta;  // gravity
    StartLocation += Velocity * Delta;

    if (!SpawnedProjectile && BPProjectileActorClass)
    {
        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        SpawnParams.Owner = AvatarActor;

        SpawnedProjectile = World->SpawnActor<AActor>(BPProjectileActorClass,StartLocation,Velocity.Rotation(), SpawnParams);
    }

    if (SpawnedProjectile)
    {
        SpawnedProjectile->SetActorLocation(StartLocation);
        SpawnedProjectile->SetActorRotation(Velocity.Rotation()); 
    }

    FVector TraceStart = StartLocation;
    FVector TraceEnd = StartLocation - FVector(0, 0, 50.f);

    FHitResult Hit;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(AvatarActor);

    if (World->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Visibility, Params))
    {
        // Landed
        bIsFlying = false;
        World->GetTimerManager().ClearTimer(AttractTimer);

        if (SpawnedProjectile)
        {
            SpawnedProjectile->SetActorLocation(Hit.ImpactPoint);
        }

        OnThrownLanded(Hit.ImpactPoint);

        if (SpawnedProjectile && IsValid(SpawnedProjectile))
        {
            SpawnedProjectile->Destroy();
            SpawnedProjectile = nullptr;
        }
    }
}

void UBundleOfJoy::OnThrownLanded(const FVector& LandLocation)
{
    AttractionCenter = LandLocation;

    const FGameplayAbilityActorInfo* ActorInfo = GetCurrentActorInfo();
    if (!ActorInfo)
    {
        EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
        return;
    }

    AActor* AvatarActor = ActorInfo->AvatarActor.Get();
    if (!IsValid(AvatarActor))
    {
        EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
        return;
    }

    UWorld* World = AvatarActor->GetWorld();
    if (!World) return;

    AController* Controller = nullptr;

    if (CurrentActorInfo && CurrentActorInfo->PlayerController.IsValid())
    {
        Controller = CurrentActorInfo->PlayerController.Get();
    }

    if (Controller)
    {
        ARelicRunnersPlayerState* PS = Controller->GetPlayerState<ARelicRunnersPlayerState>();

        if (PS)
        {
            PS->Multicast_SpawnVFX(AttractionCenter, FRotator(0, 0, 0), FVector(1.f), AbilityVFX, false);
        }
    }
    

    // start attraction loop
    World->GetTimerManager().SetTimer(AttractTimer, this, &UBundleOfJoy::Attract, 0.016f, true);

    // Timer for explosion
    if (AvatarActor->HasAuthority())
    {
        World->GetTimerManager().SetTimer(ExplosionTimer, FTimerDelegate::CreateUObject(this, &UBundleOfJoy::Server_Explode), Duration, false);
    }
}


void UBundleOfJoy::Server_Explode_Implementation()
{
    Explode(); 
}

void UBundleOfJoy::Attract()
{
    const FGameplayAbilityActorInfo* ActorInfo = GetCurrentActorInfo();
    if (!ActorInfo)
    {
        EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
        return;
    }

    AActor* AvatarActor = ActorInfo->AvatarActor.Get();
    if (!IsValid(AvatarActor))
    {
        EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
        return;
    }

    UWorld* World = AvatarActor->GetWorld(); // Get the world the actor is in
    if (!World) return; // Exit if no world

    FVector Center = AttractionCenter; // Set the attraction center 

   
    TArray<AActor*> OverlappingActors; // Array to store actors within the radius
    TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
    ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_Pawn)); // Only check for pawns
    ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_GameTraceChannel1));

    UKismetSystemLibrary::SphereOverlapActors(World, Center, AttractionRadius, ObjectTypes, APawn::StaticClass(), { AvatarActor }, OverlappingActors); // Find all characters within the radius

    for (AActor* Actor : OverlappingActors) // Loop through overlapping actors
    {
        if (!Actor) continue; // Skip null actors

        if (!Actor->Tags.Contains("Enemy")) continue;

        if (ACharacter* Char = Cast<ACharacter>(Actor)) // Only process characters
        {
            FVector Dir = (Center - Char->GetActorLocation()); // Direction towards the attraction center
            Dir.Z = 0.f; // Ignore vertical movement
            Dir.Normalize(); // Normalize direction

            Char->AddMovementInput(Dir, AttractionStrength * World->GetDeltaSeconds()); // Move character toward center

            FRotator TargetRot = Dir.Rotation(); // Get rotation to face center
            TargetRot.Pitch = 0.f;
            TargetRot.Roll = 0.f;
            Char->SetActorRotation(TargetRot); // Rotate character toward center
        }
        else if (AEnemyPawn* enemy = Cast<AEnemyPawn>(Actor))
        {
            enemy->MovementComponent->IsAffectiveByBundleOfJoy = true;
            enemy->MovementComponent->BundleOfJoyLocation = Center;
            EffectedEnemies.Add(enemy);
        }
    }
}


void UBundleOfJoy::Explode()
{
    const FGameplayAbilityActorInfo* ActorInfo = GetCurrentActorInfo();
    if (!ActorInfo)
    {
        EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
        return;
    }

    AActor* AvatarActor = ActorInfo->AvatarActor.Get();
    if (!IsValid(AvatarActor))
    {
        EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
        return;
    }

    UWorld* World = AvatarActor->GetWorld(); // Get the world the actor is in
    if (!World) return; // Exit if no world

    FVector Center = AttractionCenter; // Set the center for the explosion
    TArray<AActor*> OverlappingActors; // Array to store actors within explosion radius
    TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
    ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_Pawn)); // Only check for pawns
    ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_GameTraceChannel1));

    // Find all characters within the explosion radius
    UKismetSystemLibrary::SphereOverlapActors(World, Center, ExplosionRadius, ObjectTypes, APawn::StaticClass(), { AvatarActor }, OverlappingActors);

    for (AActor* Actor : OverlappingActors) // Loop through overlapping actors
    {
        if (!Actor) continue; // Skip null actors
        if (!Actor->Tags.Contains("Enemy")) continue;
        CalculateAbilityDamage(AvatarActor, Actor, Damage);
    }

    for (TWeakObjectPtr<AEnemyPawn> weakEnemy : EffectedEnemies)
    {
        if (weakEnemy.IsValid())
        {
            AEnemyPawn* enemy = weakEnemy.Get();

            if (IsValid(enemy->MovementComponent))
            {
                enemy->MovementComponent->IsAffectiveByBundleOfJoy = false;
            }
        }
    }
    
	// Spawn explosion VFX

    AController* Controller = nullptr;

    if (CurrentActorInfo && CurrentActorInfo->PlayerController.IsValid())
    {
        Controller = CurrentActorInfo->PlayerController.Get();
    }

    if (Controller)
    {
        ARelicRunnersPlayerState* PS = Controller->GetPlayerState<ARelicRunnersPlayerState>();

        if (PS)
        {
            PS->Multicast_SpawnVFX(AttractionCenter, FRotator(0, 0, 0), FVector(2.f), AbilityVFX1, false);
        }
    }
    

    // End the ability after explosion
    EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true, false);

}


void UBundleOfJoy::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
    if (ActorInfo && ActorInfo->AvatarActor.IsValid())
    {
        ARelicRunnersPlayerController* RPC = Cast<ARelicRunnersPlayerController>(ActorInfo->PlayerController.Get());

        UWorld* World = ActorInfo->AvatarActor->GetWorld();
        if (World)
        {
            World->GetTimerManager().ClearTimer(AttractTimer);
            World->GetTimerManager().ClearTimer(ExplosionTimer);

            if (SpawnedProjectile && IsValid(SpawnedProjectile))
            {
                SpawnedProjectile->Destroy();
                SpawnedProjectile = nullptr;
            }

            if (!bWasCancelled)
            {
                LastUseTime = World->GetTimeSeconds();

                if (ActorInfo->IsLocallyControlled())
                {
                    if (RPC)
                    {
                        RPC->PlayerHUD->StartDamageCooldown(CooldownTimer);

                    }
                }
            }
        }


    }

    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}