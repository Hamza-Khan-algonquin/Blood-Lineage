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
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"

AAphroditeCharacter::AAphroditeCharacter()
{
    PrimaryActorTick.bCanEverTick = true;
    ProjectileSpeed = 1200.f;
    ProjectileLifetime = 2.f;
    ProjectileDamage = 50.0f;
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

    AbilityClass_1 = BP_BundleOfJoyClass;
    AbilityClass_2 = BP_MirrorOfDesireClass;
    AbilityClass_3 = BP_KissOfLifeClass;
    AbilityClass_4 = BP_GoddessOfLoveClass;

    InitializeAbilityCosts();
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
        bIsAiming = false;
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

        return; // early exit so normal camera logic doesn�t run
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
    if (!Controller) return;

    FRotator ControlRot = Controller->GetControlRotation();

    // Clamp pitch for camera only
    float ClampedPitch = FMath::Clamp(ControlRot.Pitch, MinPitch, MaxPitch);

    // Smooth camera rotation
    FRotator CurrentCamRot = CameraBoom->GetComponentRotation();

    float YawDelta = FMath::FindDeltaAngleDegrees(CurrentCamRot.Yaw, ControlRot.Yaw);
    float NewYaw = CurrentCamRot.Yaw + YawDelta * DeltaTime * CameraYawSmoothSpeed;

    float NewPitch = FMath::FInterpTo(CurrentCamRot.Pitch, ClampedPitch, DeltaTime, CameraPitchSmoothSpeed);

    CameraBoom->SetWorldRotation(FRotator(NewPitch, NewYaw, 0.f));

    FRotator ActorRot = GetActorRotation();
    float ActorYawDelta = FMath::FindDeltaAngleDegrees(ActorRot.Yaw, ControlRot.Yaw);

    ActorRot.Yaw += ActorYawDelta * DeltaTime * CameraYawSmoothSpeed;

    SetActorRotation(ActorRot);
}

void AAphroditeCharacter::PlayRandomAttackAnimation()
{
    if (!BasicAnim)
        return;

    UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
    if (!AnimInstance)
        return;

    float Duration = AnimInstance->Montage_Play(BasicAnim, 1.5f);

    if (Duration > 0.f)
    {
        FOnMontageEnded EndDelegate;
        EndDelegate.BindUObject(this, &AAphroditeCharacter::OnAttackMontageEnded);

        AnimInstance->Montage_SetEndDelegate(EndDelegate, BasicAnim);
    }
}

void AAphroditeCharacter::BasicAttack()
{
    if (!HasAuthority())
    {
        Server_BasicAttack();
        return;
    }

    HandleBasicAttack();
}

void AAphroditeCharacter::Server_BasicAttack_Implementation()
{
    HandleBasicAttack();
}

void AAphroditeCharacter::HandleBasicAttack()
{
    if (!bCanBasicAttack) return;

    bCanBasicAttack = false;

    Multicast_PlayAttack();

    GetWorldTimerManager().SetTimer(TimerHandle_AttackCooldown, this, &AAphroditeCharacter::ResetBasicAttack, BasicAttackCooldown, false);

    FTimerHandle SpawnHandle;
    GetWorldTimerManager().SetTimer(SpawnHandle, this, &AAphroditeCharacter::SpawnProjectile, 0.5f, false);
}

void AAphroditeCharacter::Multicast_PlayAttack_Implementation()
{
    if (!BasicAnim) return;

    UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
    if (!AnimInstance) return;

    float Duration = AnimInstance->Montage_Play(BasicAnim, 1.5f);

    if (Duration > 0.f)
    {
        FOnMontageEnded EndDelegate;
        EndDelegate.BindUObject(this, &AAphroditeCharacter::OnAttackMontageEnded);
        AnimInstance->Montage_SetEndDelegate(EndDelegate, BasicAnim);
    }

    if (BasicAttackSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, BasicAttackSound, GetActorLocation());
    }
}

void AAphroditeCharacter::SpawnProjectile()
{
    if (!HasAuthority()) return;

    FVector SpawnLocation = GetActorLocation() + GetActorForwardVector() * 50.f + FVector(0, 0, 50.f);
    FVector Velocity = GetActorForwardVector() * ProjectileSpeed;

    Multicast_SpawnProjectile(SpawnLocation, Velocity);
}

void AAphroditeCharacter::Multicast_SpawnProjectile_Implementation(FVector Location, FVector Velocity)
{
    FBasicProjectile NewProj;
    NewProj.Location = Location;
    NewProj.Velocity = Velocity;
    NewProj.ElapsedTime = 0.f;

    if (AbilityVFX1)
    {
        UNiagaraComponent* VFX = UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(),AbilityVFX1,Location,Velocity.Rotation(),FVector(0.5f), true, true);

        NewProj.VFXComponent = VFX;
    }

    ActiveProjectiles.Add(NewProj);
}

void AAphroditeCharacter::OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
    if (Montage == BasicAnim)
    {
        bCanBasicAttack = true;
    }
}

void AAphroditeCharacter::HeavyAttack()
{
    bIsAiming = true;

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

        // Move VFX
        if (Projectile.VFXComponent)
        {
            Projectile.VFXComponent->SetWorldLocation(Projectile.Location);
            Projectile.VFXComponent->SetWorldRotation(Projectile.Velocity.Rotation());
        }

        TArray<FHitResult> Hits;
        FCollisionQueryParams Params;
        Params.AddIgnoredActor(this);
        FCollisionShape Sphere = FCollisionShape::MakeSphere(15.f);

        bool bHit = GetWorld()->SweepMultiByChannel(Hits,Projectile.PreviousLocation,Projectile.Location,FQuat::Identity, ECC_GameTraceChannel1,Sphere, Params);

        for (const FHitResult& Hit : Hits)
        {
            if (AActor* HitActor = Hit.GetActor())
            {
                if (Projectile.AlreadyHitActors.Contains(HitActor))
                    continue;

                bool bEnemyHit = false;

                if (AEnemyCharacterAI* TEnemy = Cast<AEnemyCharacterAI>(HitActor))
                {
                    ExplodeProjectile(HitActor->GetActorLocation());
                    bEnemyHit = true;
                }
                else if (AEnemyPawn* TPEnemy = Cast<AEnemyPawn>(HitActor))
                {
                    ExplodeProjectile(HitActor->GetActorLocation());
                    bEnemyHit = true;
                }
                else if (AEnemyCharacter* Enemy = Cast<AEnemyCharacter>(HitActor))
                {
                    ExplodeProjectile(HitActor->GetActorLocation());
                    bEnemyHit = true;
                }
                else if (!bEnemyHit)
                {
                    ExplodeProjectile(Hit.ImpactPoint);
                }

                Projectile.AlreadyHitActors.Add(HitActor);

                if (Hit.bBlockingHit || bEnemyHit)
                {
                    // Destroy VFX
                    if (Projectile.VFXComponent)
                    {
                        Projectile.VFXComponent->DestroyComponent();
                    }

                    ActiveProjectiles.RemoveAt(i);
                    bRemoved = true;
                    break;
                }
            }
        }

        if (bRemoved)
            continue;

        if (Projectile.ElapsedTime >= ProjectileLifetime)
        {
            if (Projectile.VFXComponent)
            {
                Projectile.VFXComponent->DestroyComponent();
            }

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

    ARelicRunnersPlayerState* PS = GetPlayerState<ARelicRunnersPlayerState>();
    if (!PS) return;

    PS->Multicast_SpawnVFX(ImpactPoint, FRotator::ZeroRotator, FVector(1.f), AbilityVFX, false);

    for (const FHitResult& Hit : Hits)
    {
        if (AActor* HitActor = Hit.GetActor())
        {
            FVector SpawnLocation = HitActor->GetActorLocation();

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
}






