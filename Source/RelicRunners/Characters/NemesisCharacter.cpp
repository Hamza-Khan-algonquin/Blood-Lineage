#include "NemesisCharacter.h"
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

    static ConstructorHelpers::FObjectFinder<UAnimMontage> AnimAsset(TEXT("/Game/Assets/Characters/Melee/Animations/melee_lightattack1_Montage.melee_lightattack1_Montage"));

    if (AnimAsset.Succeeded())
    {
        BasicAnim = AnimAsset.Object;
    }
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

void ANemesisCharacter::GiveImpunity()
{
    if (!AbilitySystem) return;

    if (!AbilitySystem->FindAbilitySpecFromClass(BP_NaturesBlessingClass))
    {
        AbilitySystem->GiveAbility(FGameplayAbilitySpec(BP_NaturesBlessingClass, 1, 0));
    }
}

void ANemesisCharacter::GiveBackStab()
{
    if (!AbilitySystem) return;

    if (!AbilitySystem->FindAbilitySpecFromClass(BP_BackStabClass))
    {
        AbilitySystem->GiveAbility(FGameplayAbilitySpec(BP_BackStabClass, 1, 0));
    }
}

void ANemesisCharacter::GiveVengefulDance()
{
    if (!AbilitySystem) return;

    if (!AbilitySystem->FindAbilitySpecFromClass(BP_VengefulDanceClass))
    {
        AbilitySystem->GiveAbility(FGameplayAbilitySpec(BP_VengefulDanceClass, 1, 0));
    }
}

void ANemesisCharacter::GiveDivinePunishment()
{
    if (!AbilitySystem) return;

    if (!AbilitySystem->FindAbilitySpecFromClass(BP_DivinePunishmentClass))
    {
        AbilitySystem->GiveAbility(FGameplayAbilitySpec(BP_DivinePunishmentClass, 1, 0));
    }
}

void ANemesisCharacter::ActivateDamageAbilities()
{
    Super::ActivateDamageAbilities();

    if (AbilitySystem)
    {
        // Activate Back Stab ability
        AbilitySystem->TryActivateAbilityByClass(BP_BackStabClass);
    }
}

void ANemesisCharacter::ActivateDefenceAbilities()
{
    if (AbilitySystem)
    {
        // Activate Impunity ability
        AbilitySystem->TryActivateAbilityByClass(BP_NaturesBlessingClass);

    }
}


void ANemesisCharacter::ActivateUtilityAbilities()
{
    Super::ActivateUtilityAbilities();

    if (AbilitySystem)
    {
        // Activate Vengeful Dance ability
        AbilitySystem->TryActivateAbilityByClass(BP_VengefulDanceClass);

    }
}

void ANemesisCharacter::ActivateUltimateAbilities()
{
    Super::ActivateUltimateAbilities();

    if (AbilitySystem)
    {
        // Activate Vengeful Dance ability
        AbilitySystem->TryActivateAbilityByClass(BP_DivinePunishmentClass);
    }
}

void ANemesisCharacter::PlayRandomAttackAnimation()
{
    if (!BasicAnim)
        return;

    USkeletalMeshComponent* SkeletalMeshComponent = GetMesh();
    if (!SkeletalMeshComponent)
        return;

    UAnimInstance* AnimInstance = SkeletalMeshComponent->GetAnimInstance();
    if (!AnimInstance)
        return;

    AnimInstance->Montage_Play(BasicAnim, 1.f);
}

void ANemesisCharacter::PerformSwingTrace()
{
    FVector Start = GetMesh()->GetSocketLocation("MainhandSocket");
    FVector End = GetMesh()->GetSocketLocation("TipSocket");

    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);

    TArray<FHitResult> Hits;

    GetWorld()->SweepMultiByChannel(Hits, Start, End, FQuat::Identity, ECC_GameTraceChannel1, FCollisionShape::MakeSphere(100.f), Params);

    for (const FHitResult& Hit : Hits)
    {
        if (AActor* HitActor = Hit.GetActor())
        {
            if (!AlreadyHitActors.Contains(HitActor))
            {
                if (AEnemyCharacter* Enemy = Cast<AEnemyCharacter>(HitActor))
                    Server_HitEnemy(Enemy, Damage);
                else if (AEnemyCharacterAI* TEnemy = Cast<AEnemyCharacterAI>(HitActor))
                    Server_HitEnemy(TEnemy, Damage);
                else if (AEnemyPawn* TPEnemy = Cast<AEnemyPawn>(HitActor))
                    Server_HitEnemy(TPEnemy, Damage);

                AlreadyHitActors.Add(HitActor); // mark as hit
            }
        }
    }
}

void ANemesisCharacter::BasicAttack()
{
    if (!RelicPlayerState || !RelicPlayerState->GetInventoryComponent()) return;
    if (!CanBasicAttack) return;

    CanBasicAttack = false;
    Damage = 120.0f;

    AlreadyHitActors.Empty();

    if (HasValidWeaponEquipped(RelicPlayerState->GetInventoryComponent()))
        PlayRandomAttackAnimation();

    GetWorldTimerManager().SetTimer(TimerHandle_AttackCooldown, this, &ARelicRunnersCharacter::ResetBasicAttack, BasicAttackCooldown, false);
}

void ANemesisCharacter::HeavyAttack()
{
    //if (!RelicPlayerState || !RelicPlayerState->GetInventoryComponent()) return;

    //if (!CanHeavyAttack) return;
    //CanHeavyAttack = false;

    //Damage = 10.0f;

    //if (HasValidWeaponEquipped(RelicPlayerState->GetInventoryComponent()))
    //{
    //    PlayRandomAttackAnimation();
    //}

    //GetWorldTimerManager().SetTimer(TimerHandle_HeavyAttackCooldown, this, &ARelicRunnersCharacter::ResetHeavyAttack, HeavyAttackCooldown, false);
}

void ANemesisCharacter::UpdateCamera(float DeltaTime)
{
    const FString MapName = GetWorld()->GetMapName();
    if (MapName.Contains(TEXT("BossLevel"))) return;

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

void ANemesisCharacter::DetectEnemies(bool& bEnemiesNearby, AActor*& NearestEnemy,float& NearestDistSq)
{
    bEnemiesNearby = false;
    NearestEnemy = nullptr;
    NearestDistSq = FLT_MAX;

    TArray<AActor*> OverlappingActors;
    TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
    ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_GameTraceChannel1));

    TArray<AActor*> ActorsToIgnore;
    ActorsToIgnore.Add(this);

    UKismetSystemLibrary::SphereOverlapActors(GetWorld(),GetActorLocation(),500.f,ObjectTypes,APawn::StaticClass(),ActorsToIgnore,OverlappingActors);

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
void ANemesisCharacter::UpdateCameraPosition(float DeltaTime,const FVector& Velocity, float Speed, bool bEnemiesNearby)
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

    CameraBoom->TargetArmLength = FMath::FInterpTo(CameraBoom->TargetArmLength,TargetArm,DeltaTime,3.5f);

    // lag
    float TargetLag = bEnemiesNearby ? 15.f : 10.f;
    float TargetRotLag = bEnemiesNearby ? 16.f : 10.f;

    CameraBoom->CameraLagSpeed = FMath::FInterpTo(CameraBoom->CameraLagSpeed,TargetLag, DeltaTime, 10.f);

    CameraBoom->CameraRotationLagSpeed = FMath::FInterpTo(CameraBoom->CameraRotationLagSpeed,TargetRotLag,DeltaTime,10.f);

    // look ahead
    FVector DesiredLookAhead = Velocity.GetSafeNormal() * LookAheadDistance;

    CurrentLookAhead = FMath::VInterpTo(CurrentLookAhead,DesiredLookAhead,DeltaTime, 5.f);

    float TargetHeightOffset = bEnemiesNearby ? HeightOffset + 60.f : HeightOffset;

    float SmoothedHeight = FMath::FInterpTo(CameraBoom->SocketOffset.Z,TargetHeightOffset,DeltaTime,3.5f);

    FVector TargetSocketOffset = FVector(0.f, 0.f, SmoothedHeight) + CurrentLookAhead;

    CameraBoom->SocketOffset = FMath::VInterpTo(CameraBoom->SocketOffset, TargetSocketOffset, DeltaTime,4.f);
}

void ANemesisCharacter::UpdateCameraRotation( float DeltaTime,const FVector& Velocity,float Speed)
{
    // moving
    if (Speed > 10.f)
    {
        FRotator CurrentRot = CameraBoom->GetComponentRotation();

        FRotator DesiredRot = CurrentRot;

        FRotator MovementRot = Velocity.ToOrientationRotator();

        float MoveDelta = FMath::FindDeltaAngleDegrees(CurrentRot.Yaw,MovementRot.Yaw);

        DesiredRot.Yaw = CurrentRot.Yaw + MoveDelta * 0.35f;

        DesiredRot.Roll = 0.f;

        float FinalDelta = FMath::FindDeltaAngleDegrees(CurrentRot.Yaw,DesiredRot.Yaw);

        float NewYaw = CurrentRot.Yaw + FinalDelta * DeltaTime * 30.f;

        float NewPitch = FMath::FInterpTo(CurrentRot.Pitch,DesiredRot.Pitch,DeltaTime, CameraPitchSmoothSpeed);

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

        float CamDeltaYaw = FMath::FindDeltaAngleDegrees( CurrentCamRot.Yaw, ControlRot.Yaw);

        float NewCamYaw = CurrentCamRot.Yaw + CamDeltaYaw * DeltaTime * CameraYawSmoothSpeed;

        float NewCamPitch = FMath::FInterpTo(CurrentCamRot.Pitch,ControlRot.Pitch, DeltaTime,CameraPitchSmoothSpeed);

        CameraBoom->SetWorldRotation(FRotator(NewCamPitch, NewCamYaw, 0.f));

        FRotator ActorRot = GetActorRotation();

        float ActorDeltaYaw = FMath::FindDeltaAngleDegrees( ActorRot.Yaw, NewCamYaw);

        ActorRot.Yaw += ActorDeltaYaw * DeltaTime * CameraYawSmoothSpeed;

        SetActorRotation(ActorRot);
    }
}

