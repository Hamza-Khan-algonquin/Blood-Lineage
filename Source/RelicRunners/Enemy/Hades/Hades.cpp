#include "Hades.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "GameFramework/Character.h"
#include "Engine/OverlapResult.h"
#include "CollisionQueryParams.h"
#include <GameFramework/ProjectileMovementComponent.h>
#include "Net/UnrealNetwork.h"
#include "HadesPredictionBase.h"
#include "HadesFireball.h"
#include "HadesSweepPrediction.h"
#include "HadesSlamPrediction.h"
#include "HadesSlamSweepPrediction.h"
#include <RelicRunners/PlayerController/RelicRunnersPlayerController.h>
#include <RelicRunners/Game/RelicRunnersGameInstance.h>
#include <RelicRunners/RelicRunnersGameState.h>
#include "RelicRunners/PlayerState/RelicRunnersPlayerState.h"

AHades::AHades()
{
    PrimaryActorTick.bCanEverTick = true;
    SkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMesh"));
    RootComponent = SkeletalMesh;
    bReplicates = true;
    bAlwaysRelevant = true;
    SetReplicateMovement(true);
}

void AHades::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AHades, CurrentPhase);
    DOREPLIFETIME(AHades, CurrentHealth);
    DOREPLIFETIME(AHades, bIsSweeping);
    DOREPLIFETIME(AHades, bIsInitialSlamming);
    DOREPLIFETIME(AHades, bIsBusy);
    DOREPLIFETIME(AHades, bIsLookingAtAbilityTarget);
    DOREPLIFETIME(AHades, AbilityLookAtTarget);
    DOREPLIFETIME(AHades, CurrentTarget);
    DOREPLIFETIME(AHades, ReplicatedVortexForce);
}

void AHades::MulticastPlayAttackMontage_Implementation(UAnimMontage* Montage, float Speed, float StartPosition)
{
    if (SkeletalMesh && Montage)
    {
        UAnimInstance* AnimInst = SkeletalMesh->GetAnimInstance();
        AnimInst->Montage_Play(Montage, Speed);
        if (StartPosition > 0.f)
        {
            AnimInst->Montage_SetPosition(Montage, StartPosition);
        }
    }
}

void AHades::BeginPlay()
{
    Super::BeginPlay();
    if(GhostCycloneSystem)
    {
        ActiveCycloneComp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), GhostCycloneSystem, FVector(0.f, 0.f, -300.f));

        ActiveCycloneComp->SetIsReplicated(true);

    }

    if (HasAuthority())
    {
        FTimerHandle StartDelayHandle;
        GetWorldTimerManager().SetTimer(StartDelayHandle, this, &AHades::TriggerAllPlayersCinematicWalk, 5.0f, false);

        SpawnBridge();
        SelectRandomTarget();
    }
}

void AHades::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bIsSweeping)
    {
        UpdateSweep(DeltaTime);
        return;
    }

    UpdateRotation(DeltaTime);

    if (!bIsBusy)
    {
        UpdateIdleAnimation();
    }
}

void AHades::TriggerAllPlayersCinematicWalk()
{
    if (!HasAuthority()) return;

    ScheduleCinematicSlam(2.0f);

    TArray<FVector> CinematicSpots;
    CinematicSpots.Add(FVector(3000.f, -1410.f, 90));
    CinematicSpots.Add(FVector(3650.f, -1410.f, 90));
    CinematicSpots.Add(FVector(1410.f, -3000.f, 90));
    CinematicSpots.Add(FVector(1410.f, -3650.f, 90));

    for (int i = CinematicSpots.Num() - 1; i > 0; i--)
    {
        int j = FMath::RandRange(0, i);
        CinematicSpots.Swap(i, j);
    }

    int SpotIndex = 0;
    for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
    {
        if (ARelicRunnersPlayerController* PC = Cast<ARelicRunnersPlayerController>(It->Get()))
        {
            if (SpotIndex < CinematicSpots.Num())
            {
                PC->Client_StartCinematicWalk(CinematicSpots[SpotIndex]);
                SpotIndex++;
            }
        }
    }
}

void AHades::OnRep_VortexForce()
{
    if (ActiveCycloneComp)
    {
        ActiveCycloneComp->SetFloatParameter(FName("VortexForce"), ReplicatedVortexForce);
    }
}

void AHades::ScheduleNextAttack(float Delay)
{
    if (!HasAuthority()) return;
    GetWorldTimerManager().SetTimer(AttackLoopTimer, this, &AHades::ExecuteAttack, Delay, false);
}

void AHades::ScheduleCinematicSlam(float Delay)
{
    if (!HasAuthority()) return;
    GetWorldTimerManager().SetTimer(AttackLoopTimer, this, &AHades::CinematicSlam, Delay, false);
}

void AHades::ResetAllStates()
{
    if (!HasAuthority()) return;

    GetWorldTimerManager().ClearTimer(AbilityLogicTimer);
    GetWorldTimerManager().ClearTimer(AttackLoopTimer);

    bIsBusy = false;
    bIsSweeping = false;
    bIsInitialSlamming = false;
    bIsLookingAtAbilityTarget = false;

    if (ActiveSweepBeam.IsValid())
    {
        ActiveSweepBeam->Destroy();
        ActiveSweepBeam = nullptr;
    }
}

void AHades::ExecuteAttack()
{
    if (!HasAuthority()) return;

    UE_LOG(LogTemp, Warning, TEXT("HADES: ExecuteAttack"));

    if (bIsBusy)
    {
        UE_LOG(LogTemp, Error, TEXT("HADES: Busy"));
        return;
    }

    if (GetWorldTimerManager().IsTimerActive(AbilityLogicTimer))
    {
        UE_LOG(LogTemp, Error, TEXT("HADES: Aborted - AbilityLogicTimer is still active!"));
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("HADES: Passed checks. Rolling for attack..."));

    bIsBusy = true; 
    GetWorldTimerManager().ClearTimer(AttackLoopTimer);

    CheckPhaseTransition();
    SelectRandomTarget();

    FString NewAttack = "";
    do
    {
        int SlamOdds = 50 - ((int)CurrentPhase * 10);
        int RemainingOdds = 100 - SlamOdds;
        int FireballOdds = RemainingOdds * 0.6;
        int SweepOdds = RemainingOdds * 0.4;

        int Roll = FMath::RandRange(1, 100);

        if (Roll <= SlamOdds)                       NewAttack = "Slam";
        else if (Roll <= SlamOdds + FireballOdds)   NewAttack = "Fireball";
        else                                        NewAttack = "Sweep";
    } while (NewAttack == LastAttack && NewAttack != "Fireball");

    if (NewAttack == "Slam")
    {
        PlaySlam(); 
        LastAttack = "Slam";
    }
    else if (NewAttack == "Fireball")
    {
        PlayFireball();
        LastAttack = "Fireball";
    }
    else if (NewAttack == "Sweep")
    {
        PlaySweep();
        LastAttack = "Sweep";
    }
}

void AHades::SpawnBridge()
{
    FVector TargetLocation(4650.0f, -4620.0f, -112.0f);
    FRotator TargetRotation = FRotator::ZeroRotator;

    if (BridgeClass)
    {
        FActorSpawnParameters Params;
        Params.Owner = this;
        Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

        BridgeToDestroy = GetWorld()->SpawnActor<AActor>(BridgeClass, TargetLocation, TargetRotation, Params);
        BridgeToDestroy->SetReplicates(true);

        UE_LOG(LogTemp, Log, TEXT("Hades: Bridge spawned at X=4650, Y=-4620, Z=-112"));
    }
}

void AHades::CinematicSlam()
{
    if (!HasAuthority()) return;

    if (!CurrentTarget) { ScheduleNextAttack(1.0f); return; }

    MulticastPlayAttackMontage(SlamMontage, 0.25f, 0);

    LastAttack = "Slam";
    bIsBusy = true;

    FVector Origin = FVector::ZeroVector;
    FVector Dir = (CurrentTarget->GetActorLocation() - Origin).GetSafeNormal2D();
    FVector SlamLoc = FVector(3400, -3400, 20.0f);
    LockRotationTo(SlamLoc, true);

    float Delay = 8;

    AHadesSlamPrediction* Pred = GetWorld()->SpawnActor<AHadesSlamPrediction>(SlamPredictionClass, SlamLoc, FRotator::ZeroRotator);
    if (Pred)
    {
        Pred->Duration = Delay;
        Pred->TargetScale = FVector(18.0f, 18.0f, 1.0f);
        Pred->bShouldScale = true;
        Pred->SetLifeSpan(Delay + 0.2f);
    }

    TWeakObjectPtr<AHades> WeakSelf(this);
    GetWorldTimerManager().SetTimer(AbilityLogicTimer, [WeakSelf, SlamLoc]() {
        AHades* StrongSelf = WeakSelf.Get();

        if (StrongSelf)
        {
            StrongSelf->ExecuteDamage(SlamLoc, 1800.0f);

            StrongSelf->bIsBusy = false;
            StrongSelf->bIsLookingAtAbilityTarget = false;
            StrongSelf->ScheduleNextAttack(2.0f);

            StrongSelf->MulticastOnBridgeDestroyed(StrongSelf->BridgeToDestroy->GetActorLocation());
            if (StrongSelf->BridgeToDestroy)
            {
                StrongSelf->BridgeToDestroy->Destroy();
                StrongSelf->BridgeToDestroy = nullptr;
            }
        }
        }, Delay, false);
}

void AHades::MulticastOnBridgeDestroyed_Implementation(FVector Location)
{
    UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), BridgeCrumbleFX, Location);
}

void AHades::PlaySlam()
{
    if (!HasAuthority()) return;

    if (!CurrentTarget) { ScheduleNextAttack(1.0f); return; }

    MulticastPlayAttackMontage(SlamMontage, 0.25f, 0);

    FVector Origin = FVector::ZeroVector;
    FVector Dir = (CurrentTarget->GetActorLocation() - Origin).GetSafeNormal2D();
    FVector SlamLoc = Origin + (Dir * 3450.0f);
    SlamLoc.Z = 20.0f;

    float Delay = 5.0f - (0.2f * (int)CurrentPhase);
    LockRotationTo(SlamLoc, true);

    AHadesSlamPrediction* Pred = GetWorld()->SpawnActor<AHadesSlamPrediction>(SlamPredictionClass, SlamLoc, FRotator::ZeroRotator);
    if (Pred)
    {
        Pred->Duration = Delay;
        Pred->TargetScale = FVector(18.0f, 18.0f, 1.0f);
        Pred->bShouldScale = true;
        Pred->SetLifeSpan(Delay + 0.2f);
    }

    TWeakObjectPtr<AHades> WeakSelf(this);
    GetWorldTimerManager().SetTimer(AbilityLogicTimer, [WeakSelf, SlamLoc]() {
        AHades* StrongSelf = WeakSelf.Get();

        if (StrongSelf)
        {
            StrongSelf->ExecuteDamage(SlamLoc, 1800.0f);

            StrongSelf->bIsBusy = false;
            StrongSelf->bIsLookingAtAbilityTarget = false;
            StrongSelf->ScheduleNextAttack(2.0f);

            if (StrongSelf->SlamCameraShake)
                StrongSelf->GetWorld()->GetFirstPlayerController()->PlayerCameraManager->PlayWorldCameraShake(StrongSelf->GetWorld(), StrongSelf->SlamCameraShake, StrongSelf->GetActorLocation(), 10000.0f, 10000.0f, 1.0f);

        }
        }, Delay, false);
}

void AHades::PlayFireball()
{
    if (!HasAuthority()) return;
    if (!CurrentTarget) { ScheduleNextAttack(1.0f); return; }

    bIsBusy = true;
    LockRotationTo(CurrentTarget->GetActorLocation(), true);

    MulticastPlayAttackMontage(FireballMontage, 2.5f, 0);

    TWeakObjectPtr<AHades> WeakSelf(this);

    GetWorldTimerManager().SetTimer(AbilityLogicTimer, [WeakSelf]() {
        if (!WeakSelf.IsValid() || !WeakSelf->CurrentTarget) return;

        int Count = ((int)WeakSelf->CurrentPhase * 5) + 7;
        FVector Center = WeakSelf->CurrentTarget->GetActorLocation();

        for (int i = 0; i < Count; i++)
        {
            FVector TargetLoc = WeakSelf->CalculateFireballLocation(Center);
            TargetLoc.Z = 20.0f;

            float FlightTime = FMath::FRandRange(1.f, 2.0f);
            float LaunchDelay = 0.1f + FMath::FRandRange(0.0f, 0.5f);
            FName SocketName = (i % 2 == 0) ? FName("MainhandSocket") : FName("OffhandSocket");

            AHadesFireball* FB = WeakSelf->GetWorld()->SpawnActorDeferred<AHadesFireball>(
                WeakSelf->FireballClass, FTransform::Identity, WeakSelf.Get());

            if (FB)
            {
                FB->TargetLocation = TargetLoc;
                FB->FlightDuration = FlightTime;
                FB->SetOwner(WeakSelf.Get());

                UGameplayStatics::FinishSpawningActor(FB, FTransform(WeakSelf->SkeletalMesh->GetSocketLocation(SocketName)));
                FB->AttachToComponent(WeakSelf->SkeletalMesh, FAttachmentTransformRules::SnapToTargetIncludingScale, SocketName);

                FTimerHandle DetachHandle;
                WeakSelf->GetWorldTimerManager().SetTimer(DetachHandle, [FB]() {
                    if (IsValid(FB)) FB->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
                    }, LaunchDelay, false);
            }
        }

        FTimerHandle ResetHandle;
        WeakSelf->GetWorldTimerManager().SetTimer(ResetHandle, [WeakSelf]() {
            if (WeakSelf.IsValid())
            {
                WeakSelf->bIsLookingAtAbilityTarget = false;
                WeakSelf->bIsBusy = false;
                WeakSelf->ScheduleNextAttack(1.5f);
            }
            }, 2.5f, false);

        }, 0.8f, false);
}

void AHades::PlaySweep()
{
    if (!HasAuthority() || !CurrentTarget) { ScheduleNextAttack(1.0f); return; }

    bIsBusy = true;
    bIsInitialSlamming = true;

    FVector DirToPlayer = (CurrentTarget->GetActorLocation() - GetActorLocation()).GetSafeNormal2D();
    SweepStartYaw = DirToPlayer.Rotation().Yaw + 90.0f;

    FVector SlamDirVector = FRotator(0, SweepStartYaw, 0).Vector();

    FVector SlamLoc = GetActorLocation() + (SlamDirVector * 3450.0f);
    SlamLoc.Z = 20.0f;

    LockRotationTo(CurrentTarget->GetActorLocation(), true);

    MulticastPlayAttackMontage(SweepMontage, 1.5f, 0);

    AHadesSlamSweepPrediction* SlamPred = GetWorld()->SpawnActor<AHadesSlamSweepPrediction>(SlamSweepPredictionClass, SlamLoc, FRotator::ZeroRotator);
    if (SlamPred)
    {
        SlamPred->Duration = 1.0f;
        SlamPred->TargetScale = FVector(8.0f, 8.0f, 1.0f);
        SlamPred->bShouldScale = true;
        SlamPred->SetLifeSpan(1.2f);
    }

    GetWorldTimerManager().SetTimer(AbilityLogicTimer, [this, SlamLoc]() {
        ExecuteDamage(SlamLoc, 800.0f);

        bIsInitialSlamming = false;
        bIsSweeping = true;
        SweepElapsedTime = 0.0f;
        SweepRotationCount = FMath::RandRange(1, (int)CurrentPhase + 2);
        SweepTotalTime = SweepDuration + ((SweepDuration / 1.5f) * (SweepRotationCount - 1));

        if (SpinCameraShake)
            GetWorld()->GetFirstPlayerController()->PlayerCameraManager->PlayWorldCameraShake(GetWorld(), SpinCameraShake, GetActorLocation(), 10000.0f, 10000.0f, 1.0f);

        if (SweepPredictionClass)
        {
            FVector BeamSpawnPos = FVector(0, 0, 100);

            ActiveSweepBeam = GetWorld()->SpawnActor<AHadesSweepPrediction>(SweepPredictionClass, BeamSpawnPos, FRotator(0, SweepStartYaw, 0));
            if (AHadesSweepPrediction* Beam = Cast<AHadesSweepPrediction>(ActiveSweepBeam.Get()))
            {
                Beam->Duration = SweepTotalTime;
                Beam->bShouldScale = false;
                Beam->RotationCount = SweepRotationCount;
                Beam->StartYaw = SweepStartYaw;
                Beam->bIsRotating = true;
                Beam->SetOwner(this);
                Beam->AttachToActor(this, FAttachmentTransformRules::KeepWorldTransform);
            }
        }
        }, 1.0f, false);
}

void AHades::UpdateSweep(float DeltaTime)
{
    if (!HasAuthority()) return;

    SweepElapsedTime += DeltaTime;
    if (ActiveCycloneComp)
    {
        float Force = ((SweepElapsedTime + 3) * 100.0f);
        ActiveCycloneComp->SetFloatParameter(FName("VortexForce"), Force);
        ReplicatedVortexForce = Force;

        OnRep_VortexForce();
    }
    float TotalTime = SweepDuration + ((SweepDuration / 1.5f) * (SweepRotationCount - 1));
    float Alpha = FMath::Clamp(SweepElapsedTime / TotalTime, 0.0f, 1.0f);

    if (UAnimInstance* AnimInst = SkeletalMesh->GetAnimInstance())
    {
        if (!AnimInst->Montage_IsPlaying(SweepMontage))
        {
            float StartPos = SweepMontage->GetPlayLength() - 0.1f; 
            MulticastPlayAttackMontage(SweepMontage, 0.0f, StartPos);
        }
    }

    if (ActiveSweepBeam.IsValid())
    {
        FVector DirToBeam = (ActiveSweepBeam->GetActorLocation() - GetActorLocation()).GetSafeNormal2D();

        if (!DirToBeam.IsNearlyZero())
        {
            FRotator NewRot = DirToBeam.Rotation();
            NewRot.Yaw -= RotationOffset*2;

            SetActorRotation(NewRot);
        }
    }

    if (Alpha >= 1.0f)
    {
        bIsSweeping = false;

        bIsLookingAtAbilityTarget = false;

        if (ActiveSweepBeam.IsValid())
        {
            ActiveSweepBeam->Destroy();
            ActiveSweepBeam = nullptr;
        }

        if (ActiveCycloneComp)
        {
            float Force = (200.f);
            ActiveCycloneComp->SetFloatParameter(FName("VortexForce"), Force);
            ReplicatedVortexForce = Force;

            OnRep_VortexForce();
        }

        bIsBusy = false;
        ScheduleNextAttack(2.0f);
    }
}

void AHades::UpdateRotation(float DeltaTime)
{
    if (bIsSweeping) return;

    FVector TargetPos;
    float Speed = RotationSpeed;

    if (bIsQuickTurning)
    {
        Speed = 15.0f; 
    }
    else if (bIsLookingAtAbilityTarget)
    {
        TargetPos = AbilityLookAtTarget;
        Speed *= 2.0f;
    }
    else if (CurrentTarget)
    {
        TargetPos = CurrentTarget->GetActorLocation();
    }
    else
    {
        return; 
    }

    FRotator DesiredRot;
    if (bIsQuickTurning)
    {
        DesiredRot = QuickTurnTarget;
    }
    else
    {
        FVector Dir = (TargetPos - GetActorLocation()).GetSafeNormal2D();
        if (Dir.IsNearlyZero()) return;

        DesiredRot = Dir.Rotation();
        DesiredRot.Yaw -= RotationOffset;
    }

    FRotator CurrentRot = GetActorRotation();

    DesiredRot.Pitch = CurrentRot.Pitch;
    DesiredRot.Roll = CurrentRot.Roll;

    FRotator NewRot = FMath::RInterpTo(CurrentRot, DesiredRot, DeltaTime, Speed);
    SetActorRotation(NewRot);

    if (bIsQuickTurning && CurrentRot.Equals(QuickTurnTarget, 2.0f))
    {
        bIsQuickTurning = false;
    }
}

void AHades::UpdateIdleAnimation()
{
    if (!HasAuthority() || !IdleMontage) return;

    UAnimInstance* AnimInst = SkeletalMesh->GetAnimInstance();
    if (AnimInst && !AnimInst->Montage_IsPlaying(IdleMontage))
    {
        MulticastPlayAttackMontage(IdleMontage, 1.0f, 0);
    }
}

void AHades::ExecuteDamage(FVector ImpactLocation, float ImpactRadius)
{
    if (!HasAuthority()) return;

    DrawDebugSphere(GetWorld(), ImpactLocation, ImpactRadius, 32, FColor::Orange, false, 2.0f);
    UGameplayStatics::ApplyRadialDamage(GetWorld(), 40.0f * (1 + (int)CurrentPhase), ImpactLocation, ImpactRadius, nullptr, { this }, this, nullptr, true);
}

void AHades::SelectRandomTarget()
{
    if (!HasAuthority()) return;

    TArray<AActor*> ValidPlayers;

    for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
    {
        APlayerController* PC = It->Get();

        if (IsValid(PC) && IsValid(PC->GetPawn()))
        {
            ValidPlayers.Add(PC->GetPawn());
        }
    }

    if (ValidPlayers.Num() > 0)
    {
        int RandomIndex = FMath::RandRange(0, ValidPlayers.Num() - 1);
        CurrentTarget = ValidPlayers[RandomIndex];
    }
    else
    {
        CurrentTarget = nullptr;
    }
}

FVector AHades::CalculateFireballLocation(FVector Center)
{
    if (!HasAuthority()) return FVector();

    float SearchRadius = 1800.0f + ((int)CurrentPhase * 200.0f);

    FVector RandomOffset = FMath::VRand() * FMath::FRandRange(500.f, SearchRadius);
    FVector TargetLoc = Center + RandomOffset;
    TargetLoc.Z = 0;

    FVector DirFromCenter = TargetLoc - FVector::ZeroVector;
    float Dist = DirFromCenter.Size();

    return DirFromCenter.GetSafeNormal() * FMath::Clamp(Dist, 2500.0f, 4400.0f);
}

void AHades::NotifyObeliskHit(AActor* Obelisk)
{
    if (!HasAuthority()) return;

    ObelisksHit++;

    float DamageToSelf = MaxHealth * 0.25f;
    CurrentHealth = FMath::Max(0.f, CurrentHealth - DamageToSelf);

    UE_LOG(LogTemp, Warning, TEXT("Hades hit an obelisk! Current Health: %f"), CurrentHealth);

    if (bIsBusy || bIsSweeping)
    {
        ResetAllStates();
        ScheduleNextAttack(3.0f);
    }

    if (ObelisksHit >= 4 || CurrentHealth <= 0)
    {
        Die();
    }
}

void AHades::OnRep_CurrentHealth()
{
    if (CurrentHealth <= 0) Die();
}

void AHades::Die()
{
    if (!HasAuthority()) return;

    GetWorldTimerManager().ClearAllTimersForObject(this);

    if (ActiveSweepBeam.IsValid())
    {
        ActiveSweepBeam->Destroy();
    }

    if (ActiveCycloneComp)
    {
        ActiveCycloneComp->DestroyComponent();
    }

    //Meta XP
    ARelicRunnersGameState* GS = GetWorld()->GetGameState<ARelicRunnersGameState>();
    URelicRunnersGameInstance* GI = GetWorld()->GetGameInstance<URelicRunnersGameInstance>();

    if (GS && GI)
    {
        FDifficultyInfo DiffInfo = GS->GetDifficultyInfo(GI->CurrentDifficulty);

        for (APlayerState* PS : GS->PlayerArray)
        {
            if (ARelicRunnersPlayerState* RRPS = Cast<ARelicRunnersPlayerState>(PS))
            {
                int AddedMeta = FMath::RandRange(400, 500) * DiffInfo.MetaXPIncrease;

                RRPS->MetaXPGainInRun += AddedMeta;

                if (ARelicRunnersPlayerController* PC = Cast<ARelicRunnersPlayerController>(RRPS->GetOwner()))
                {
                    PC->Client_RegisterMetaXPGain(AddedMeta);
                }
            }
        }
    }

    Destroy();
}

void AHades::CheckPhaseTransition()
{
    if (!HasAuthority()) return;

    float HPPercent = CurrentHealth / MaxHealth;

    if (HPPercent <= 0.25f) { CurrentPhase = EBossPhase::Phase4; SweepDuration = 5; }
    else if (HPPercent <= 0.50f) { CurrentPhase = EBossPhase::Phase3; SweepDuration = 6; }
    else if (HPPercent <= 0.75f) { CurrentPhase = EBossPhase::Phase2; SweepDuration = 7; }
    else { CurrentPhase = EBossPhase::Phase1; SweepDuration = 8; }
}

void AHades::LockRotationTo(FVector TargetLocation, bool bInstant)
{
    if (!HasAuthority()) return;

    AbilityLookAtTarget = TargetLocation;
    bIsLookingAtAbilityTarget = true;

    FVector LookAtDir = (TargetLocation - GetActorLocation()).GetSafeNormal2D();
    FRotator TargetRotation = LookAtDir.Rotation();
    TargetRotation.Yaw -= RotationOffset;

    float AngleDelta = FMath::Abs(FMath::FindDeltaAngleDegrees(GetActorRotation().Yaw, TargetRotation.Yaw));

    if (bInstant)
    {
        if (AngleDelta < 15.0f)
        {
            SetActorRotation(TargetRotation);
            MulticastSnapRotation(TargetRotation);
        }
        else
        {
            MulticastQuickTurn(TargetRotation);
        }
    }
}

void AHades::MulticastQuickTurn_Implementation(FRotator NewRotation)
{
    QuickTurnTarget = NewRotation;
    bIsQuickTurning = true;
}

void AHades::MulticastSnapRotation_Implementation(FRotator NewRotation)
{
    SetActorRotation(NewRotation);
}