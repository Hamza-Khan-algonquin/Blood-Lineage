// Fill out your copyright notice in the Description page of Project Settings.


#include "AphroditeCharacter.h"
#include "RelicRunners/PlayerState/RelicRunnersPlayerState.h"
#include <RelicRunners/Item/ItemStats.h>
#include "RelicRunners/Inventory/InventoryComponent.h"
#include "RelicRunners/Inventory/Inventory.h"
#include <RelicRunners/Classes/ClassInfo.h>
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "RelicRunners/Enemy/Characters/EnemyCharacter.h"
#include "RelicRunners/Enemy/Characters/EnemyCharacterAI.h"
#include "Net/UnrealNetwork.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "RelicRunners/Enemy/EnemyPawn.h"
#include "Engine/CollisionProfile.h"
#include "Components/CapsuleComponent.h"

AAphroditeCharacter::AAphroditeCharacter()
{
    PrimaryActorTick.bCanEverTick = true;
    ProjectileSpeed = 1200.f;
    ProjectileLifetime = 2.f;
    ProjectileDamage = 90.0f;
}

void AAphroditeCharacter::BeginPlay()
{
    Super::BeginPlay();

}

// Called when player takes control of this character
void AAphroditeCharacter::PossessedBy(AController* NewController)
{
    Super::PossessedBy(NewController);

    // Get PlayerState and set up ability system
    ARelicRunnersPlayerState* PS = GetPlayerState<ARelicRunnersPlayerState>();
    if (PS)
    {
        AbilitySystem = PS->GetAbilitySystemComponent();
        if (AbilitySystem)
        {                       
            AbilitySystem->InitAbilityActorInfo(PS, this);
        }
    }

    if (AbilitySystem)
    {
        // Give Abilities
        GiveBundleOfJoy();
        GiveMirrorOfDesire();
        GiveKissOfLife();
        GiveGoddessOfLove();
    }
}

// Called when PlayerState is replicated on clients
void AAphroditeCharacter::OnRep_PlayerState()
{
    Super::OnRep_PlayerState();

    // Reinitialize ability system
    ARelicRunnersPlayerState* PS = GetPlayerState<ARelicRunnersPlayerState>();
    if (PS)
    {
        AbilitySystem = PS->GetAbilitySystemComponent();
        AbilitySystem->InitAbilityActorInfo(PS, this);
    }
}

void AAphroditeCharacter::ActivateDamageAbilities()
{
    Super::ActivateDamageAbilities();

    if (AbilitySystem)
    {
        // Activate Back Stab ability
        AbilitySystem->TryActivateAbilityByClass(BP_BundleOfJoyClass);
    }

}

void AAphroditeCharacter::ActivateDefenceAbilities()
{
    Super::ActivateDefenceAbilities();

    if (AbilitySystem)
    {
        // Activate Back Stab ability
        AbilitySystem->TryActivateAbilityByClass(BP_MirrorOfDesireClass);
    }
}

void AAphroditeCharacter::ActivateUtilityAbilities()
{
    Super::ActivateUtilityAbilities();

    if (AbilitySystem)
    {
        // Activate Back Stab ability
        AbilitySystem->TryActivateAbilityByClass(BP_KissOfLifeClass);
    }
}

void AAphroditeCharacter::ActivateUltimateAbilities()
{
    Super::ActivateUltimateAbilities();

    if (AbilitySystem)
    {
        // Activate Back Stab ability
        AbilitySystem->TryActivateAbilityByClass(BP_GoddessOfLoveClass);
    }
}

void AAphroditeCharacter::GiveBundleOfJoy()
{
    if (!AbilitySystem) return;

    if (!AbilitySystem->FindAbilitySpecFromClass(BP_BundleOfJoyClass))
    {
        AbilitySystem->GiveAbility(FGameplayAbilitySpec(BP_BundleOfJoyClass, 1, 0));
    }
}

void AAphroditeCharacter::GiveMirrorOfDesire()
{
    if (!AbilitySystem) return;

    if (!AbilitySystem->FindAbilitySpecFromClass(BP_MirrorOfDesireClass))
    {
        AbilitySystem->GiveAbility(FGameplayAbilitySpec(BP_MirrorOfDesireClass, 1, 0));
    }
}

void AAphroditeCharacter::GiveKissOfLife()
{
    if (!AbilitySystem) return;

    if (!AbilitySystem->FindAbilitySpecFromClass(BP_KissOfLifeClass))
    {
        AbilitySystem->GiveAbility(FGameplayAbilitySpec(BP_KissOfLifeClass, 1, 0));
    }
}

void AAphroditeCharacter::GiveGoddessOfLove()
{
    if (!AbilitySystem) return;

    if (!AbilitySystem->FindAbilitySpecFromClass(BP_GoddessOfLoveClass))
    {
        AbilitySystem->GiveAbility(FGameplayAbilitySpec(BP_GoddessOfLoveClass, 1, 0));
    }
}

void AAphroditeCharacter::PerformSwingTrace()
{
}

void AAphroditeCharacter::ShowReticle()
{
    if (!ReticleWidget && ReticleWidgetClass)
    {
        ReticleWidget = CreateWidget<UUserWidget>(GetWorld(), ReticleWidgetClass);
        if (ReticleWidget)
        {
            ReticleWidget->AddToViewport();
            ReticleWidget->SetRenderTranslation(FVector2D(-80.f, 0.f));
        }
    }
}

void AAphroditeCharacter::HideReticle()
{
    if (ReticleWidget)
    {
        ReticleWidget->RemoveFromParent();
        ReticleWidget = nullptr;
    }
}

void AAphroditeCharacter::UpdateCamera(float DeltaTime)
{
    const FString MapName = GetWorld()->GetMapName();
    if (MapName.Contains(TEXT("BossLevel"))) return;

    if (bIsAiming)
    {
        ShowReticle();

        // Disable lag while aiming
        CameraBoom->bEnableCameraLag = false;
        CameraBoom->bEnableCameraRotationLag = false;

        // Smoothly interpolate arm length and offset
        CameraBoom->TargetArmLength = FMath::FInterpTo(CameraBoom->TargetArmLength, AimingArmLength,DeltaTime,AimingInterpSpeed);

        CameraBoom->SocketOffset = FMath::VInterpTo( CameraBoom->SocketOffset,AimingOffset,DeltaTime, AimingInterpSpeed);

        // Rotate player to controller yaw
        if (Controller)
        {
            FRotator ControlRot = Controller->GetControlRotation();
            FRotator ActorRot = GetActorRotation();
            ActorRot.Yaw = ControlRot.Yaw;
            SetActorRotation(ActorRot);
        }

        // Rotate camera to face player
        FRotator CurrentRot = CameraBoom->GetComponentRotation();
        FRotator TargetRot = GetActorRotation();
        TargetRot.Pitch = 0.f;
        TargetRot.Roll = 0.f;

        CameraBoom->SetWorldRotation(FMath::RInterpTo(CurrentRot,TargetRot, DeltaTime,AimingInterpSpeed));

        return; // early exit so normal camera logic doesn’t run
    }
    else
    {
        HideReticle();
    }

    FVector Velocity = GetVelocity();
    Velocity.Z = 0.f;
    float Speed = Velocity.Size();

    bool bEnemiesNearby = false;
    AActor* NearestEnemy = nullptr;
    float NearestDistSq = FLT_MAX;

    DetectEnemies(bEnemiesNearby, NearestEnemy, NearestDistSq);

    UpdateCameraPosition(DeltaTime, Velocity, Speed, bEnemiesNearby);

    UpdateCameraRotation(DeltaTime, Velocity, Speed);
}

void AAphroditeCharacter::DetectEnemies(bool& bEnemiesNearby, AActor*& NearestEnemy, float& NearestDistSq)
{
    bEnemiesNearby = false;
    NearestEnemy = nullptr;
    NearestDistSq = FLT_MAX;

    TArray<AActor*> OverlappingActors;
    TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
    ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_GameTraceChannel1));

    TArray<AActor*> ActorsToIgnore;
    ActorsToIgnore.Add(this);

    UKismetSystemLibrary::SphereOverlapActors(GetWorld(), GetActorLocation(), 500.f, ObjectTypes, APawn::StaticClass(), ActorsToIgnore, OverlappingActors);

    for (AActor* Actor : OverlappingActors)
    {
        if (!Actor || !Actor->ActorHasTag("Enemy")) continue;

        bEnemiesNearby = true;

        float DistSq = FVector::DistSquared(Actor->GetActorLocation(), GetActorLocation());

        if (DistSq < NearestDistSq)
        {
            NearestDistSq = DistSq;
            NearestEnemy = Actor;
        }
    }

}
void AAphroditeCharacter::UpdateCameraPosition(float DeltaTime, const FVector& Velocity, float Speed, bool bEnemiesNearby)
{
    // arm length adjustments
    float CombatArmLength = BaseArmLength + 150.f;

    float TargetArm;
    if (bEnemiesNearby)
        TargetArm = CombatArmLength;
    else if (Speed > 600.f)
        TargetArm = SprintArmLength;
    else
        TargetArm = BaseArmLength;

    CameraBoom->TargetArmLength = FMath::FInterpTo(CameraBoom->TargetArmLength, TargetArm, DeltaTime, 3.5f);

    // lag
    float TargetLag = bEnemiesNearby ? 15.f : 10.f;
    float TargetRotLag = bEnemiesNearby ? 16.f : 10.f;

    CameraBoom->CameraLagSpeed = FMath::FInterpTo(CameraBoom->CameraLagSpeed, TargetLag, DeltaTime, 10.f);

    CameraBoom->CameraRotationLagSpeed = FMath::FInterpTo(CameraBoom->CameraRotationLagSpeed, TargetRotLag, DeltaTime, 10.f);

    // look ahead
    FVector DesiredLookAhead = Velocity.GetSafeNormal() * LookAheadDistance;

    CurrentLookAhead = FMath::VInterpTo(CurrentLookAhead, DesiredLookAhead, DeltaTime, 5.f);

    float TargetHeightOffset = bEnemiesNearby ? HeightOffset + 60.f : HeightOffset;

    float SmoothedHeight = FMath::FInterpTo(CameraBoom->SocketOffset.Z, TargetHeightOffset, DeltaTime, 3.5f);

    FVector TargetSocketOffset = FVector(0.f, 0.f, SmoothedHeight) + CurrentLookAhead;

    CameraBoom->SocketOffset = FMath::VInterpTo(CameraBoom->SocketOffset, TargetSocketOffset, DeltaTime, 4.f);
}

void AAphroditeCharacter::UpdateCameraRotation(float DeltaTime, const FVector& Velocity, float Speed)
{
    // moving
    if (Speed > 10.f)
    {
        FRotator CurrentRot = CameraBoom->GetComponentRotation();

        FRotator DesiredRot = CurrentRot;

        FRotator MovementRot = Velocity.ToOrientationRotator();

        float MoveDelta = FMath::FindDeltaAngleDegrees(CurrentRot.Yaw, MovementRot.Yaw);

        DesiredRot.Yaw = CurrentRot.Yaw + MoveDelta * 0.35f;

        DesiredRot.Roll = 0.f;

        float FinalDelta = FMath::FindDeltaAngleDegrees(CurrentRot.Yaw, DesiredRot.Yaw);

        float NewYaw = CurrentRot.Yaw + FinalDelta * DeltaTime * 30.f;

        float NewPitch = FMath::FInterpTo(CurrentRot.Pitch, DesiredRot.Pitch, DeltaTime, CameraPitchSmoothSpeed);

        CameraBoom->SetWorldRotation(FRotator(NewPitch, NewYaw, 0.f));
    }
    // idle
    else
    {
        if (!Controller) return;

        FRotator ControlRot = Controller->GetControlRotation();

        ControlRot.Pitch = FMath::Clamp(ControlRot.Pitch, MinPitch, MaxPitch);
        ControlRot.Roll = 0.f;

        FRotator CurrentCamRot = CameraBoom->GetComponentRotation();

        float CamDeltaYaw = FMath::FindDeltaAngleDegrees(CurrentCamRot.Yaw, ControlRot.Yaw);

        float NewCamYaw = CurrentCamRot.Yaw + CamDeltaYaw * DeltaTime * CameraYawSmoothSpeed;

        float NewCamPitch = FMath::FInterpTo(CurrentCamRot.Pitch, ControlRot.Pitch, DeltaTime, CameraPitchSmoothSpeed);

        CameraBoom->SetWorldRotation(FRotator(NewCamPitch, NewCamYaw, 0.f));

        FRotator ActorRot = GetActorRotation();

        float ActorDeltaYaw = FMath::FindDeltaAngleDegrees(ActorRot.Yaw, NewCamYaw);

        ActorRot.Yaw += ActorDeltaYaw * DeltaTime * CameraYawSmoothSpeed;

        SetActorRotation(ActorRot);
    }
}


void AAphroditeCharacter::PlayRandomAttackAnimation()
{

    UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
    if (!AnimInstance) return;

    TArray<UAnimSequence*> SwordAttacks;
    if (SwordBasicAttack0) SwordAttacks.Add(SwordBasicAttack0);
    if (SwordBasicAttack1) SwordAttacks.Add(SwordBasicAttack1);
    if (SwordBasicAttack2) SwordAttacks.Add(SwordBasicAttack2);
    if (SwordBasicAttack3) SwordAttacks.Add(SwordBasicAttack3);

    if (SwordAttacks.Num() > 0)
    {
        int Index = FMath::RandRange(0, SwordAttacks.Num() - 1);
        AnimInstance->PlaySlotAnimationAsDynamicMontage(SwordAttacks[Index], FName("DefaultSlot"));
    }
}

void AAphroditeCharacter::BasicAttack()
{
    //if (!bIsAiming) return;
    if (!RelicPlayerState || !RelicPlayerState->GetInventoryComponent()) return;
    //if (!CanBasicAttack) return;

    CanBasicAttack = true;

    ProjectileDamage = 40.0f;

    if (!HasValidWeaponEquipped(RelicPlayerState->GetInventoryComponent())) return;

    FBasicProjectile NewProjectile;
    NewProjectile.Location = GetActorLocation() + GetActorForwardVector() * 100.f + FVector(0, 0, 50.f);
    NewProjectile.Velocity = GetActorForwardVector() * ProjectileSpeed;
    NewProjectile.ElapsedTime = 0.f;

    ActiveProjectiles.Add(NewProjectile);

    PlayRandomAttackAnimation();

    GetWorldTimerManager().SetTimer(TimerHandle_AttackCooldown,this,&AAphroditeCharacter::ResetBasicAttack,BasicAttackCooldown,false );
}


void AAphroditeCharacter::HeavyAttack()
{
    bIsAiming = true;

    //if (!RelicPlayerState || !RelicPlayerState->GetInventoryComponent()) return;
    //if (!CanHeavyAttack) return;

    //CanHeavyAttack = false;

    //ProjectileDamage = 40.0f;

    //if (!HasValidWeaponEquipped(RelicPlayerState->GetInventoryComponent())) return;

    //FBasicProjectile NewProjectile;
    //NewProjectile.Location =GetActorLocation()+ GetActorForwardVector() * 100.f + FVector(0, 0, 50.f);

    //NewProjectile.Velocity =
    //    GetActorForwardVector() * ProjectileSpeed;

    //NewProjectile.ElapsedTime = 0.f;

    //ActiveProjectiles.Add(NewProjectile);

    //PlayRandomAttackAnimation();

    //// Cooldown
    //GetWorldTimerManager().SetTimer(TimerHandle_HeavyAttackCooldown,this,&AAphroditeCharacter::ResetHeavyAttack,HeavyAttackCooldown,false);
}


void AAphroditeCharacter::ProjectileHeavyAttack()
{
    bIsAiming = false;
}



void AAphroditeCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    for (int32 i = ActiveProjectiles.Num() - 1; i >= 0; --i)
    {
        FBasicProjectile& Projectile = ActiveProjectiles[i];

        Projectile.ElapsedTime += DeltaTime;

        Projectile.PreviousLocation = Projectile.Location;
        Projectile.Location += Projectile.Velocity * DeltaTime;

        DrawDebugSphere(GetWorld(), Projectile.Location, 15.f, 12, FColor::Red, false, 0.f);

        TArray<FHitResult> Hits;
        FCollisionQueryParams Params;
        Params.AddIgnoredActor(this);
        FCollisionShape Sphere = FCollisionShape::MakeSphere(15.f);

        bool bHit = GetWorld()->SweepMultiByChannel(Hits, Projectile.PreviousLocation, Projectile.Location, FQuat::Identity, ECC_GameTraceChannel1, Sphere, Params);

        for (const FHitResult& Hit : Hits)
        {
            if (AActor* HitActor = Hit.GetActor())
            {
                if (Projectile.AlreadyHitActors.Contains(HitActor))
                    continue;

                else if (AEnemyCharacterAI* TEnemy = Cast<AEnemyCharacterAI>(HitActor))
                {
                    Server_HitEnemy(TEnemy, ProjectileDamage);
                    ExplodeProjectile(HitActor->GetActorLocation());

                }
                else if (AEnemyPawn* TPEnemy = Cast<AEnemyPawn>(HitActor))
                {
                    Server_HitEnemy(TPEnemy, ProjectileDamage);
                    ExplodeProjectile(HitActor->GetActorLocation());

                }
                else if (AEnemyCharacter* Enemy = Cast<AEnemyCharacter>(HitActor))
                {
                    Server_HitEnemy(Enemy, ProjectileDamage);
                    ExplodeProjectile(HitActor->GetActorLocation());

                }

                ExplodeProjectile(Hit.ImpactPoint);

                Projectile.AlreadyHitActors.Add(HitActor);
                // Remove projectile after hit
                ActiveProjectiles.RemoveAt(i);
                break;
            }
        }

        if (Projectile.ElapsedTime >= ProjectileLifetime)
        {
            ActiveProjectiles.RemoveAt(i);
        }
    }
}

void AAphroditeCharacter::ExplodeProjectile(const FVector& ImpactPoint)
{
    TArray<FHitResult> Hits;

    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);

    FCollisionShape Sphere = FCollisionShape::MakeSphere(ExplosionRadius);

    bool bHit = GetWorld()->SweepMultiByChannel(Hits,ImpactPoint,ImpactPoint, FQuat::Identity,ECC_GameTraceChannel1,Sphere,Params);

    if (!bHit) return;

    for (const FHitResult& Hit : Hits)
    {
        if (AActor* HitActor = Hit.GetActor())
        {
            if (AEnemyCharacterAI* TEnemy = Cast<AEnemyCharacterAI>(HitActor))
            {
                Server_HitEnemy(TEnemy, ProjectileDamage);
            }
            else if (AEnemyPawn* TPEnemy = Cast<AEnemyPawn>(HitActor))
            {
                Server_HitEnemy(TPEnemy, ProjectileDamage);
            }
            else if (AEnemyCharacter* Enemy = Cast<AEnemyCharacter>(HitActor))
            {
                Server_HitEnemy(Enemy, ProjectileDamage);
            }
        }
    }

    DrawDebugSphere(GetWorld(), ImpactPoint, ExplosionRadius, 20, FColor::Orange, false, 1.f);
}






