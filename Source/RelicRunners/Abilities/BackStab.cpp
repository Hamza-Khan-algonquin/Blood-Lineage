// Fill out your copyright notice in the Description page of Project Settings.


#include "BackStab.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "RelicRunners/PlayerState/RelicRunnersPlayerState.h"
#include "Kismet/KismetSystemLibrary.h"
#include "DrawDebugHelpers.h"
#include "TimerManager.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Engine/StaticMeshActor.h"     
#include "Components/StaticMeshComponent.h"
#include "RelicRunners/Inventory/InventoryComponent.h"
#include "RelicRunners/Item/ItemStats.h"

UBackStab::UBackStab()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

    Damage = 200.f;
    Duration = 5.f;
    LastUseTime = -100.f;
    CooldownTimer = 5.f;
    Cost = 30;
	ConeAngle = 80.f;
	Range = 600.f;
	BehindOffset = 120.f;


}

void UBackStab::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);


    if (!ActorInfo || !ActorInfo->AvatarActor.IsValid())
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

    UWorld* World = GetWorld();
    if (!World) return;

    if (World->GetTimeSeconds() - LastUseTime < CooldownTimer)
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
    {
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

    LastUseTime = World->GetTimeSeconds();

    OriginalLocation = AvatarActor->GetActorLocation();
    OriginalRotation = AvatarActor->GetActorRotation();

    CurrentBackStabCount = 0;
    ExecuteBackStabSequence();
}

AActor* UBackStab::FindBackstabTarget(AActor* AvatarActor) const
{
    UWorld* World = AvatarActor->GetWorld();
    if (!World) return nullptr;

    const FVector Origin = AvatarActor->GetActorLocation();
    const FVector Forward = AvatarActor->GetActorForwardVector();

    TArray<AActor*> FoundActors;
    TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
    ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_GameTraceChannel1));

    UKismetSystemLibrary::SphereOverlapActors(World,Origin, Range,ObjectTypes,APawn::StaticClass(),{ AvatarActor },FoundActors);

    AActor* BestTarget = nullptr;
    float BestDot = -1.f;

    for (AActor* Actor : FoundActors)
    {
        if (!Actor || !Actor->ActorHasTag("Enemy"))
            continue;

        FVector DirToTarget = (Actor->GetActorLocation() - Origin);
        DirToTarget.Z = 0.f;
        DirToTarget.Normalize();

        float Dot = FVector::DotProduct(Forward, DirToTarget);
        float RequiredDot = FMath::Cos(FMath::DegreesToRadians(ConeAngle * 0.5f));

        if (Dot >= RequiredDot && Dot > BestDot)
        {
            BestDot = Dot;
            BestTarget = Actor;
        }
    }

    return BestTarget;
}

void UBackStab::PerformBackStabDamage(AActor* AvatarActor)
{
    if (!AvatarActor || !IsValid(AvatarActor)) return;

    UWorld* World = GetWorld();
    if (!World) return;

    float HalfConeRad = FMath::DegreesToRadians(ConeAngle / 2.f);
    float MaxDistance = SweepRadius;

    TArray<AActor*> OverlappingActors;
    TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
    ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_GameTraceChannel1));

    TArray<AActor*> ActorsToIgnore;
    ActorsToIgnore.Add(AvatarActor);

    UKismetSystemLibrary::SphereOverlapActors(World, AvatarActor->GetActorLocation(), MaxDistance, ObjectTypes, APawn::StaticClass(),ActorsToIgnore,OverlappingActors);

    FVector Forward = AvatarActor->GetActorForwardVector();

    //DrawDebugCone(World, AvatarActor->GetActorLocation(),Forward,MaxDistance,HalfConeRad,HalfConeRad,12,FColor::Green,false,1.f);

    for (AActor* Actor : OverlappingActors)
    {
        if (!Actor || !Actor->ActorHasTag("Enemy")) continue;

        FVector DirToActor = Actor->GetActorLocation() - AvatarActor->GetActorLocation();
        DirToActor.Z = 0.f;

        if (DirToActor.Size() > MaxDistance) continue;

        DirToActor.Normalize();

        if (FVector::DotProduct(Forward, DirToActor) >= FMath::Cos(HalfConeRad))
        {
			CalculateAbilityDamage(AvatarActor, Actor, Damage);

            //if (APawn* EnemyChar = Cast<APawn>(Actor))
            //{
            //    if (EnemyChar->GetCharacterMovement())
            //    {
            //        EnemyChar->GetCharacterMovement()->DisableMovement();

            //        FTimerHandle RestoreHandle;
            //        World->GetTimerManager().SetTimer(
            //            RestoreHandle,
            //            FTimerDelegate::CreateLambda([EnemyChar]()
            //                {
            //                    if (EnemyChar && EnemyChar->GetCharacterMovement())
            //                    {
            //                        EnemyChar->GetCharacterMovement()->SetMovementMode(MOVE_Walking);
            //                    }
            //                }), 0.6f, false);
            //    }
            //}
        }
    }
}


void UBackStab::ExecuteBackStabSequence()
{
    UWorld* World = GetWorld();
    AActor* AvatarActor = GetAvatarActorFromActorInfo();

    if (!World || !AvatarActor)
    {
        ReturnPlayerToOriginalPosition();
        EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, false);
        return;
    }

    if (CurrentBackStabCount >= 3)
    {
        ReturnPlayerToOriginalPosition();
        EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, false);
        return;
    }

    // Find target
    AActor* TargetEnemy = FindBackstabTarget(AvatarActor);
    if (!TargetEnemy || !IsValid(TargetEnemy))
    {
        ReturnPlayerToOriginalPosition();
        EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, false);
        return;
    }

    // Teleport behind enemy
    FVector DashLocation = TargetEnemy->GetActorLocation() - TargetEnemy->GetActorForwardVector() * DashDistance;
    AvatarActor->SetActorLocation(DashLocation, false, nullptr, ETeleportType::TeleportPhysics);
    AvatarActor->SetActorRotation(TargetEnemy->GetActorRotation());

    if (AbilityAnim)
    {
        PlayAbilityAnimation(CurrentActorInfo, AbilityAnim);
    }

    ACharacter* Char = Cast<ACharacter>(AvatarActor);
    if (!Char || !Char->GetCharacterMovement())
    {
        ReturnPlayerToOriginalPosition();
        EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, false);
        return;
    }

    // Stop player movement
    Char->GetCharacterMovement()->DisableMovement();


    if (AvatarActor->HasAuthority())
    {
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
                PS->Multicast_SpawnVFX(FVector(0), FRotator::ZeroRotator, FVector(2.f), AbilityVFX, true);
            }
        }
    }
    
    // Delay damage for 1 second
    World->GetTimerManager().SetTimer(BackStabDamageTimerHandle, FTimerDelegate::CreateLambda([this, AvatarActor, Char]()
            {
                PerformBackStabDamage(AvatarActor);

                // Re-enable movement
                if (Char && Char->GetCharacterMovement())
                {
                    Char->GetCharacterMovement()->SetMovementMode(MOVE_Walking);
                }

                CurrentBackStabCount++;
                ExecuteBackStabSequence(); // Next sequence
            }),0.6f,false);
}


void UBackStab::ReturnPlayerToOriginalPosition()
{
    AActor* AvatarActor = GetAvatarActorFromActorInfo();
    if (!AvatarActor) return;

    AvatarActor->SetActorLocation(OriginalLocation,false,nullptr,ETeleportType::TeleportPhysics);

    AvatarActor->SetActorRotation(OriginalRotation);

    if (ACharacter* Char = Cast<ACharacter>(AvatarActor))
    {
        if (Char->GetCharacterMovement())
        {
            Char->GetCharacterMovement()->SetMovementMode(MOVE_Walking);
        }
    }
}


void UBackStab::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);

    if (ActorInfo && ActorInfo->AvatarActor.IsValid())
    {
        ARelicRunnersPlayerController* RPC = Cast<ARelicRunnersPlayerController>(ActorInfo->PlayerController.Get());

        UWorld* World = ActorInfo->AvatarActor->GetWorld();
        if (World)
        {
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

}


