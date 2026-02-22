#include "NemesisCharacter.h"
//#include "RelicRunners/AbilitySystem/AbilityPointCounter.h"
#include "RelicRunners/PlayerState/RelicRunnersPlayerState.h"
#include <RelicRunners/Item/ItemStats.h>
#include "RelicRunners/Inventory/InventoryComponent.h"
#include "RelicRunners/Inventory/Inventory.h"
#include <RelicRunners/Classes/ClassInfo.h>
#include "RelicRunners/Enemy/Characters/EnemyCharacter.h"
#include "RelicRunners/Enemy/Characters/EnemyCharacterAI.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "RelicRunners/Enemy/EnemyPawn.h"

ANemesisCharacter::ANemesisCharacter()
{
    Damage = 20.0f;
}

void ANemesisCharacter::BeginPlay()
{
    Super::BeginPlay();
}

// Called when player takes control of this character
void ANemesisCharacter::PossessedBy(AController* NewController)
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

    // Give Abilities
    if (AbilitySystem)
    {
        GiveImpunity();
        GiveBackStab();
        GiveVengefulDance();
        GiveDivinePunishment();
    }
}

// Called when PlayerState is replicated on clients
void ANemesisCharacter::OnRep_PlayerState()
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


void ANemesisCharacter::ActivateDamageAbilities()
{
    Super::ActivateDamageAbilities();

    if (AbilitySystem)
    {
        // Activate Back Stab ability
        AbilitySystem->TryActivateAbilityByClass(UBackStab::StaticClass());
    }
}

void ANemesisCharacter::GiveImpunity()
{
    if (!AbilitySystem) return;

    if (!AbilitySystem->FindAbilitySpecFromClass(UImpunityAbility::StaticClass()))
    {
        AbilitySystem->GiveAbility(FGameplayAbilitySpec(UImpunityAbility::StaticClass(), 1, 0));
    }
}

void ANemesisCharacter::GiveBackStab()
{
    if (!AbilitySystem) return;

    if (!AbilitySystem->FindAbilitySpecFromClass(UBackStab::StaticClass()))
    {
        AbilitySystem->GiveAbility(FGameplayAbilitySpec(UBackStab::StaticClass(), 1, 0));
    }
}

void ANemesisCharacter::GiveVengefulDance()
{
    if (!AbilitySystem) return;

    if (!AbilitySystem->FindAbilitySpecFromClass(UVengefulDance::StaticClass()))
    {
        AbilitySystem->GiveAbility(FGameplayAbilitySpec(UVengefulDance::StaticClass(), 1, 0));
    }
}

void ANemesisCharacter::GiveDivinePunishment()
{
    if (!AbilitySystem) return;

    if (!AbilitySystem->FindAbilitySpecFromClass(UDivinePunishment::StaticClass()))
    {
        AbilitySystem->GiveAbility(FGameplayAbilitySpec(UDivinePunishment::StaticClass(), 1, 0));
    }
}

void ANemesisCharacter::ActivateDefenceAbilities()
{
    if (AbilitySystem)
    {
        // Activate Impunity ability
        AbilitySystem->TryActivateAbilityByClass(UImpunityAbility::StaticClass());

    }
}


void ANemesisCharacter::ActivateUtilityAbilities()
{
    Super::ActivateUtilityAbilities();

    if (AbilitySystem)
    {
        // Activate Vengeful Dance ability
        AbilitySystem->TryActivateAbilityByClass(UVengefulDance::StaticClass());

    }
}

void ANemesisCharacter::ActivateUltimateAbilities()
{
    Super::ActivateUltimateAbilities();

    if (AbilitySystem)
    {
        // Activate Vengeful Dance ability
        AbilitySystem->TryActivateAbilityByClass(UDivinePunishment::StaticClass());

    }
}

void ANemesisCharacter::PlayRandomAttackAnimation()
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

void ANemesisCharacter::PerformSwingTrace()
{
    FVector Start = GetMesh()->GetSocketLocation("MainhandSocket");
    FVector End = GetMesh()->GetSocketLocation("TipSocket");

    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);

    TArray<FHitResult> Hits;
    GetWorld()->SweepMultiByChannel(
        Hits, Start, End, FQuat::Identity,
        ECC_Pawn, FCollisionShape::MakeSphere(100.0f), Params
    );

    AlreadyHitActors.Empty();

    for (const FHitResult& Hit : Hits)
    {
        if (AActor* HitActor = Hit.GetActor())
        {
            if (!AlreadyHitActors.Contains(HitActor))
            {
                if (AEnemyCharacter* Enemy = Cast<AEnemyCharacter>(HitActor))
                {
                    Server_HitEnemy(Enemy);
                }
                else if (AEnemyCharacterAI* TEnemy = Cast<AEnemyCharacterAI>(HitActor))
                {
                    Server_HitEnemy(TEnemy);
                }
                else if (AEnemyPawn* TPEnemy = Cast<AEnemyPawn>(HitActor))
                {
                    Server_HitEnemy(TPEnemy);
                }
                AlreadyHitActors.Add(HitActor);
            }
        }
    }
}

void ANemesisCharacter::BasicAttack()
{
    if (!RelicPlayerState || !RelicPlayerState->GetInventoryComponent()) return;

    if (!CanBasicAttack) return;
    CanBasicAttack = false;

    Damage = 5.0f;

    if (HasValidWeaponEquipped(RelicPlayerState->GetInventoryComponent()))
    {
        PlayRandomAttackAnimation();
    }

    GetWorldTimerManager().SetTimer(TimerHandle_AttackCooldown, this, &ARelicRunnersCharacter::ResetBasicAttack, BasicAttackCooldown, false);
}

void ANemesisCharacter::HeavyAttack()
{
    if (!RelicPlayerState || !RelicPlayerState->GetInventoryComponent()) return;

    if (!CanHeavyAttack) return;
    CanHeavyAttack = false;

    Damage = 10.0f;

    if (HasValidWeaponEquipped(RelicPlayerState->GetInventoryComponent()))
    {
        PlayRandomAttackAnimation();
    }

    GetWorldTimerManager().SetTimer(TimerHandle_AttackCooldown, this, &ARelicRunnersCharacter::ResetHeavyAttack, HeavyAttackCooldown, false);
}

void ANemesisCharacter::UpdateCamera(float DeltaTime)
{
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
    ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));

    TArray<AActor*> ActorsToIgnore;
    ActorsToIgnore.Add(this);

    UKismetSystemLibrary::SphereOverlapActors(GetWorld(), GetActorLocation(), 300.f, ObjectTypes, ACharacter::StaticClass(), ActorsToIgnore,OverlappingActors);

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

    // Dynamic zoom 
    float CombatArmLength = BaseArmLength + 50.f;

    float TargetArm;
    if (bEnemiesNearby)
    {
        TargetArm = CombatArmLength;
    }
    else if (Speed > 600.f)
    {
        TargetArm = SprintArmLength;
    }
    else
    {
        TargetArm = BaseArmLength;
    }

    if (bEnemiesNearby)
    {
        TargetArm = FMath::Max(TargetArm, BaseArmLength + 75.f);
    }

    CameraBoom->TargetArmLength = FMath::FInterpTo(CameraBoom->TargetArmLength, TargetArm, DeltaTime, 4.f);

    CameraBoom->TargetArmLength = FMath::Clamp(CameraBoom->TargetArmLength, BaseArmLength * 0.9f, SprintArmLength);

    // Camera lag
    float TargetCameraLagSpeed = bEnemiesNearby ? 2.f : 8.f;
    float TargetCameraRotationLagSpeed = bEnemiesNearby ? 2.f : 6.f;

    CameraBoom->CameraLagSpeed = FMath::FInterpTo(CameraBoom->CameraLagSpeed, TargetCameraLagSpeed, DeltaTime, 8.f);

    CameraBoom->CameraRotationLagSpeed = FMath::FInterpTo(CameraBoom->CameraRotationLagSpeed, TargetCameraRotationLagSpeed, DeltaTime, 8.f);

    // Look-ahead
    FVector DesiredLookAhead = Velocity.GetSafeNormal() * LookAheadDistance;
    CurrentLookAhead = FMath::VInterpTo(CurrentLookAhead, DesiredLookAhead, DeltaTime, 3.f);

    float AdjustedHeight = HeightOffset + (bEnemiesNearby ? 20.f : 0.f);
    CameraBoom->SocketOffset = FVector(0.f, 0.f, AdjustedHeight) + CurrentLookAhead;

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

        float NewPitch = FMath::FInterpTo( CurrentRot.Pitch,DesiredRot.Pitch, DeltaTime, CameraPitchSmoothSpeed);

        float BounceOffset = FMath::Sin(GetWorld()->GetTimeSeconds() * BounceSpeed) * BounceAmplitude;

        FRotator Smoothed(NewPitch + BounceOffset * 0.1f, NewYaw, 0.f);
        CameraBoom->SetWorldRotation(Smoothed);
    }
    else
    {
        if (!Controller) return;
        FRotator ControlRot = Controller->GetControlRotation();
        ControlRot.Pitch = FMath::Clamp(ControlRot.Pitch, MinPitch, MaxPitch);
        ControlRot.Roll = 0.f;

        FRotator CurrentCamRot = CameraBoom->GetComponentRotation();

        float CamDeltaYaw = FMath::FindDeltaAngleDegrees(CurrentCamRot.Yaw, ControlRot.Yaw);

        float NewCamYaw = CurrentCamRot.Yaw + CamDeltaYaw * DeltaTime * CameraYawSmoothSpeed;

        float NewCamPitch = FMath::FInterpTo(CurrentCamRot.Pitch,ControlRot.Pitch,DeltaTime,CameraPitchSmoothSpeed);

        CameraBoom->SetWorldRotation( FRotator(NewCamPitch, NewCamYaw, 0.f));


        FRotator ActorRot = GetActorRotation();

        float ActorDeltaYaw = FMath::FindDeltaAngleDegrees(ActorRot.Yaw, NewCamYaw);

        ActorRot.Yaw += ActorDeltaYaw * DeltaTime * 2.5f;
        SetActorRotation(ActorRot);
    }

}


void ANemesisCharacter::Server_HitEnemy_Implementation(AActor* HitActor)
{
    if (HitActor)
    {
        UGameplayStatics::ApplyDamage(HitActor, Damage, GetController(), this, nullptr);
    }
}