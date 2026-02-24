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
        AbilitySystem->TryActivateAbilityByClass(UBundleOfJoy::StaticClass());
    }

}

void AAphroditeCharacter::ActivateDefenceAbilities()
{
    Super::ActivateDefenceAbilities();

    if (AbilitySystem)
    {
        // Activate Back Stab ability
        AbilitySystem->TryActivateAbilityByClass(UMirrorOfDesireAbility::StaticClass());
    }
}

void AAphroditeCharacter::ActivateUtilityAbilities()
{
    Super::ActivateUtilityAbilities();

    if (AbilitySystem)
    {
        // Activate Back Stab ability
        AbilitySystem->TryActivateAbilityByClass(UKissOfLife::StaticClass());
    }
}

void AAphroditeCharacter::ActivateUltimateAbilities()
{
    Super::ActivateUltimateAbilities();

    if (AbilitySystem)
    {
        // Activate Back Stab ability
        AbilitySystem->TryActivateAbilityByClass(UGoddessOfLove::StaticClass());
    }
}

void AAphroditeCharacter::GiveBundleOfJoy()
{
    if (!AbilitySystem) return;

    if (!AbilitySystem->FindAbilitySpecFromClass(UBundleOfJoy::StaticClass()))
    {
        AbilitySystem->GiveAbility(FGameplayAbilitySpec(UBundleOfJoy::StaticClass(), 1, 0));
    }
}

void AAphroditeCharacter::GiveMirrorOfDesire()
{
    if (!AbilitySystem) return;

    if (!AbilitySystem->FindAbilitySpecFromClass(UMirrorOfDesireAbility::StaticClass()))
    {
        AbilitySystem->GiveAbility(FGameplayAbilitySpec(UMirrorOfDesireAbility::StaticClass(), 1, 0));
    }
}

void AAphroditeCharacter::GiveKissOfLife()
{
    if (!AbilitySystem) return;

    if (!AbilitySystem->FindAbilitySpecFromClass(UKissOfLife::StaticClass()))
    {
        AbilitySystem->GiveAbility(FGameplayAbilitySpec(UKissOfLife::StaticClass(), 1, 0));
    }
}

void AAphroditeCharacter::GiveGoddessOfLove()
{
    if (!AbilitySystem) return;

    if (!AbilitySystem->FindAbilitySpecFromClass(UGoddessOfLove::StaticClass()))
    {
        AbilitySystem->GiveAbility(FGameplayAbilitySpec(UGoddessOfLove::StaticClass(), 1, 0));
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
    if (!CameraBoom) return;

    // Aiming mode
    if (bIsAiming)
    {
        ShowReticle();

        // Disable lag while aiming
        CameraBoom->bEnableCameraLag = false;
        CameraBoom->bEnableCameraRotationLag = false;

        // Interpolate arm length
        CameraBoom->TargetArmLength = FMath::FInterpTo(CameraBoom->TargetArmLength, AimingArmLength, DeltaTime, AimingInterpSpeed);

        // Interpolate socket offset
        CameraBoom->SocketOffset = FMath::VInterpTo(CameraBoom->SocketOffset, AimingOffset, DeltaTime, AimingInterpSpeed);

        // Rotate player to controller yaw
        if (Controller)
        {
            FRotator ControlRot = Controller->GetControlRotation();
            FRotator ActorRot = GetActorRotation();
            ActorRot.Yaw = ControlRot.Yaw;
            SetActorRotation(ActorRot);
        }

        // Smoothly rotate camera to face player
        FRotator CurrentRot = CameraBoom->GetComponentRotation();
        FRotator TargetRot = GetActorRotation();
        TargetRot.Pitch = 0.f;
        TargetRot.Roll = 0.f;

        CameraBoom->SetWorldRotation(FMath::RInterpTo(CurrentRot, TargetRot, DeltaTime, AimingInterpSpeed));
    }
    // non-aiming mode
    else
    {
        // Camera logic
        HideReticle();

        CameraBoom->bUsePawnControlRotation = true;

        // Re-enable lag smoothly
        CameraBoom->bEnableCameraLag = true;
        CameraBoom->bEnableCameraRotationLag = true;

        // Movement info
        FVector Velocity = GetVelocity();
        Velocity.Z = 0.f;
        float Speed = Velocity.Size();

        // Detect nearby enemies
        bool bEnemiesNearby = false;
        AActor* NearestEnemy = nullptr;
        float NearestDistSq = FLT_MAX;

        TArray<AActor*> OverlappingActors;
        TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
        ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_GameTraceChannel1));
        TArray<AActor*> ActorsToIgnore;
        ActorsToIgnore.Add(this);

        UKismetSystemLibrary::SphereOverlapActors(GetWorld(), GetActorLocation(), 300.f, ObjectTypes, APawn::StaticClass(), ActorsToIgnore, OverlappingActors);

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
        // Smoothly interpolate camera distance
        float CombatArmLength = BaseArmLength + 50.f;
        float TargetArm = BaseArmLength;

        if (bEnemiesNearby)
            TargetArm = FMath::Max(CombatArmLength, BaseArmLength + 75.f);
        else if (Speed > 600.f)
            TargetArm = SprintArmLength;

        CameraBoom->TargetArmLength = FMath::FInterpTo(CameraBoom->TargetArmLength, TargetArm, DeltaTime, 4.f);

        // Camera lag speeds
        float TargetCameraLagSpeed = bEnemiesNearby ? 2.f : 8.f;
        float TargetCameraRotationLagSpeed = bEnemiesNearby ? 2.f : 6.f;

        CameraBoom->CameraLagSpeed = FMath::FInterpTo(CameraBoom->CameraLagSpeed, TargetCameraLagSpeed, DeltaTime, 8.f);
        CameraBoom->CameraRotationLagSpeed = FMath::FInterpTo(CameraBoom->CameraRotationLagSpeed, TargetCameraRotationLagSpeed, DeltaTime, 8.f);

        // Look-ahead
        FVector DesiredLookAhead = Velocity.GetSafeNormal() * LookAheadDistance;
        CurrentLookAhead = FMath::VInterpTo(CurrentLookAhead, DesiredLookAhead, DeltaTime, 3.f);

        float AdjustedHeight = HeightOffset + (bEnemiesNearby ? 20.f : 0.f);
        FVector TargetOffset = FVector(0.f, 0.f, AdjustedHeight) + CurrentLookAhead;

        // Smoothly interpolate socket offset
        CameraBoom->SocketOffset = FMath::VInterpTo(CameraBoom->SocketOffset, TargetOffset, DeltaTime, 6.f);

        // Rotation logic
        if (Speed > 10.f)
        {
            FRotator DesiredRot = Velocity.ToOrientationRotator();

            if (NearestEnemy && NearestDistSq > FMath::Square(150.f))
            {
                FVector DirToEnemy = NearestEnemy->GetActorLocation() - GetActorLocation();
                DirToEnemy.Z = 0.f;
                DirToEnemy.Normalize();

                float EnemyYaw = DirToEnemy.Rotation().Yaw;
                float DeltaYaw = FMath::FindDeltaAngleDegrees(DesiredRot.Yaw, EnemyYaw);
                DesiredRot.Yaw += DeltaYaw * 0.2f;
            }

            FRotator CurrentRot = CameraBoom->GetComponentRotation();
            DesiredRot.Pitch = FMath::Clamp(CurrentRot.Pitch, MinPitch, MaxPitch);
            DesiredRot.Roll = 0.f;

            float DeltaYaw = FMath::FindDeltaAngleDegrees(CurrentRot.Yaw, DesiredRot.Yaw);
            float NewYaw = CurrentRot.Yaw + DeltaYaw * DeltaTime * CameraYawSmoothSpeed;
            float NewPitch = FMath::FInterpTo(CurrentRot.Pitch, DesiredRot.Pitch, DeltaTime, CameraPitchSmoothSpeed);

            float BounceOffset = FMath::Sin(GetWorld()->GetTimeSeconds() * BounceSpeed) * BounceAmplitude;
            CameraBoom->SetWorldRotation(FRotator(NewPitch + BounceOffset * 0.1f, NewYaw, 0.f));
        }
        else if (Controller)
        {
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
            ActorRot.Yaw += ActorDeltaYaw * DeltaTime * 2.5f;
            SetActorRotation(ActorRot);
        }
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

    ProjectileDamage = 90.0f;

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

    if (!RelicPlayerState || !RelicPlayerState->GetInventoryComponent()) return;
    if (!CanHeavyAttack) return;

    CanHeavyAttack = false;

    ProjectileDamage = 120.0f;

    if (!HasValidWeaponEquipped(RelicPlayerState->GetInventoryComponent())) return;

    FBasicProjectile NewProjectile;
    NewProjectile.Location =GetActorLocation()+ GetActorForwardVector() * 100.f + FVector(0, 0, 50.f);

    NewProjectile.Velocity =
        GetActorForwardVector() * ProjectileSpeed;

    NewProjectile.ElapsedTime = 0.f;

    ActiveProjectiles.Add(NewProjectile);

    PlayRandomAttackAnimation();

    // Cooldown
    GetWorldTimerManager().SetTimer(TimerHandle_AttackCooldown,this,&AAphroditeCharacter::ResetHeavyAttack,HeavyAttackCooldown,false);
}


void AAphroditeCharacter::ProjectileHeavyAttack()
{
    bIsAiming = false;
}


void AAphroditeCharacter::Server_HitEnemy_Implementation(AActor* HitActor)
{
    if (HitActor)
    {
        UGameplayStatics::ApplyDamage(HitActor, 90.f, GetController(), this, nullptr);

    }
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
                //Server_HitEnemy(Enemy);

                else if (AEnemyCharacterAI* TEnemy = Cast<AEnemyCharacterAI>(HitActor))
                {
                    Server_HitEnemy(TEnemy);
                }
                else if (AEnemyPawn* TPEnemy = Cast<AEnemyPawn>(HitActor))
                {
                    Server_HitEnemy(TPEnemy);
                }
                else if (AEnemyCharacter* Enemy = Cast<AEnemyCharacter>(HitActor))
                {
                    Server_HitEnemy(Enemy);

                }

                Projectile.AlreadyHitActors.Add(HitActor);
                // Remove projectile after hit
                ActiveProjectiles.RemoveAt(i);
                break;
            }
        }
        continue;

        if (Projectile.ElapsedTime >= ProjectileLifetime)
        {
            ActiveProjectiles.RemoveAt(i);
        }
    }
}







