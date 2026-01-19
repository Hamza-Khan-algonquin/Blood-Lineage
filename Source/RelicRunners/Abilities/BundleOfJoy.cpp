#include "BundleOfJoy.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "DrawDebugHelpers.h"
#include "TimerManager.h"

UBundleOfJoy::UBundleOfJoy()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

    AttractionRadius = 600.f;
    ExplosionRadius = 400.f;
    AttractionStrength = 1500.f;
    DamageAmount = 100.f;
    Duration = 3.f;
    LastUseTime = -100.f;
    CooldownTimer = 15.f;
}
void UBundleOfJoy::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{

    float CurrentTime = GetWorld()->GetTimeSeconds();
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

    if (!CommitAbility(Handle, ActorInfo, ActivationInfo)) // Try to commit the ability
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true); // If commit fails, end the ability 
        return; // Exit early
    }

    AttractionCenter = AvatarActor->GetActorLocation() + AvatarActor->GetActorForwardVector() * 300.f; // Set spawn point 300 units in front of the actor


    StartLocation = AvatarActor->GetActorLocation() + AvatarActor->GetActorForwardVector() * 100.f + FVector(0, 0, 50);
    Velocity = AvatarActor->GetActorForwardVector() * 700.f + FVector(0, 0, 600.f);
    bIsFlying = true;

    AvatarActor->GetWorldTimerManager().SetTimer(AttractTimer, this, &UBundleOfJoy::TickThrown, 0.016f, true);

    LastUseTime = CurrentTime;

}

void UBundleOfJoy::TickThrown()
{
    if (!bIsFlying) return;

    AActor* Avatar = GetAvatarActorFromActorInfo();
    UWorld* World = Avatar->GetWorld();
    float Delta = World->GetDeltaSeconds();

    // projectile physics
    Velocity += FVector(0, 0, -980.f) * Delta;  // gravity
    StartLocation += Velocity * Delta;

    // see throw trajectory
    DrawDebugSphere(World, StartLocation, 10.f, 12, FColor::Yellow, false, 0.05f);

    // Check ground hit
    FVector TraceStart = StartLocation;
    FVector TraceEnd = StartLocation - FVector(0, 0, 50);

    FHitResult Hit;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(Avatar);

    if (World->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Visibility, Params))
    {
        // Landed
        bIsFlying = false;
        World->GetTimerManager().ClearTimer(AttractTimer);

        OnThrownLanded(Hit.ImpactPoint);
    }
}

void UBundleOfJoy::OnThrownLanded(const FVector& LandLocation)
{
    AttractionCenter = LandLocation;

    AActor* Avatar = GetAvatarActorFromActorInfo();
    UWorld* World = Avatar->GetWorld();

    // start attraction loop
    World->GetTimerManager().SetTimer(AttractTimer, this, &UBundleOfJoy::Attract, 0.016f, true);

    // Timer for explosion
    World->GetTimerManager().SetTimer(ExplosionTimer, FTimerDelegate::CreateUObject(this, &UBundleOfJoy::Explode), Duration, false);

    DrawDebugSphere(World, LandLocation, 25.f, 12, FColor::Purple, false, 5.f);
}

void UBundleOfJoy::Attract()
{
    AActor* AvatarActor = GetAvatarActorFromActorInfo(); // Get the actor this ability belongs to
    if (!AvatarActor) return; // Exit if no valid actor

    UWorld* World = AvatarActor->GetWorld(); // Get the world the actor is in
    if (!World) return; // Exit if no world

    FVector Center = AttractionCenter; // Set the attraction center 

    DrawDebugCircle(World, Center, AttractionRadius, 32, FColor::Purple, false, 0.1f, 0, 5.f, FVector(1, 0, 0), FVector(0, 1, 0), false); // Draw the debug circle showing the attraction radius

    TArray<AActor*> OverlappingActors; // Array to store actors within the radius
    TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
    ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_Pawn)); // Only check for pawns

    UKismetSystemLibrary::SphereOverlapActors(World, Center, AttractionRadius, ObjectTypes, ACharacter::StaticClass(), { AvatarActor }, OverlappingActors); // Find all characters within the radius

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
    }
}


void UBundleOfJoy::Explode()
{
    AActor* AvatarActor = GetAvatarActorFromActorInfo(); // Get the actor this ability belongs to
    if (!AvatarActor) return; // Exit if no actor

    UWorld* World = AvatarActor->GetWorld(); // Get the world the actor is in
    if (!World) return; // Exit if no world

    FVector Center = AttractionCenter; // Set the center for the explosion
    TArray<AActor*> OverlappingActors; // Array to store actors within explosion radius
    TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
    ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_Pawn)); // Only check for pawns

    // Find all characters within the explosion radius
    UKismetSystemLibrary::SphereOverlapActors(World, Center, ExplosionRadius, ObjectTypes, ACharacter::StaticClass(), { AvatarActor }, OverlappingActors);

    for (AActor* Actor : OverlappingActors) // Loop through overlapping actors
    {
        if (!Actor) continue; // Skip null actors
        if (!Actor->Tags.Contains("Enemy")) continue;
        UGameplayStatics::ApplyDamage(Actor, DamageAmount, nullptr, AvatarActor, nullptr); // Apply damage to each actor
    }

    DrawDebugSphere(World, Center, ExplosionRadius, 32, FColor::Red, false, 2.f); // Draw debug sphere for explosion radius

    // End the ability after explosion
    EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true, false);

}


void UBundleOfJoy::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
    AActor* AvatarActor = GetAvatarActorFromActorInfo();
    if (AvatarActor)
    {
        // Clear timers
        AvatarActor->GetWorldTimerManager().ClearTimer(AttractTimer);
        AvatarActor->GetWorldTimerManager().ClearTimer(ExplosionTimer);
    }

    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}