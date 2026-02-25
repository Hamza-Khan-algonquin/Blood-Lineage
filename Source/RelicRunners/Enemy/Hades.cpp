#include "Hades.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "GameFramework/Character.h"
#include "Engine/OverlapResult.h"
#include "CollisionQueryParams.h"

AHades::AHades()
{
    PrimaryActorTick.bCanEverTick = true;
    SkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMesh"));
    RootComponent = SkeletalMesh;
}

void AHades::BeginPlay()
{
    Super::BeginPlay();
    if (GhostCycloneSystem)
    {
        ActiveCycloneComp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), GhostCycloneSystem, FVector(0.f, 0.f, -300.f));
    }
    ScheduleNextAttack(2.0f);
}

void AHades::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bIsSweeping)
    {
        UpdateSweep(DeltaTime);
        return;
    }

    if (bIsInitialSlamming || bIsBusy) return;

    UpdateRotation(DeltaTime);
    UpdateIdleAnimation();
}

void AHades::ScheduleNextAttack(float Delay)
{
    GetWorldTimerManager().SetTimer(AttackLoopTimer, this, &AHades::ExecuteAttack, Delay, false);
}

void AHades::ResetAllStates()
{
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

    int32 Roll = FMath::RandRange(1, 100);
    if (Roll <= 25) { UE_LOG(LogTemp, Log, TEXT("HADES: Chose Slam")); PlaySlam(); }
    else if (Roll <= 60) { UE_LOG(LogTemp, Log, TEXT("HADES: Chose Fireball")); PlayFireball(); }
    else { UE_LOG(LogTemp, Log, TEXT("HADES: Chose Sweep")); PlaySweep(); }
}

void AHades::PlaySlam()
{
    if (!CurrentTarget) { bIsBusy = false; ScheduleNextAttack(1.0f); return; }

    if (SkeletalMesh)
    {
        UAnimInstance* AnimInst = SkeletalMesh->GetAnimInstance();
        if (AnimInst && SlamMontage)
        {
            AnimInst->Montage_Play(SlamMontage, 0.25f);
            UE_LOG(LogTemp, Log, TEXT("Hades: Slam Animation Started"));
        }
    }

    FVector Origin = FVector(0, 0, 0);
    FVector Dir = (CurrentTarget->GetActorLocation() - Origin).GetSafeNormal2D();
    FVector SlamLoc = Origin + (Dir * 3450.0f);
    SlamLoc.Z = 20.0f;

    float Delay = 5.0f - (0.2f * (int)CurrentPhase);
    LockRotationTo(SlamLoc);

    AActor* Pred = SpawnPrediction(SlamPredictionClass, SlamLoc, Delay, FVector(1));
    TWeakObjectPtr<AActor> WeakPred = Pred;
    TWeakObjectPtr<AHades> WeakSelf(this);

    GetWorldTimerManager().SetTimer(AbilityLogicTimer, [WeakSelf, SlamLoc, WeakPred]() {
        if (!WeakSelf.IsValid()) return;
        if (WeakPred.IsValid()) WeakPred->Destroy();
        WeakSelf->ExecuteDamage(SlamLoc, 1800.0f);
        if (!WeakSelf.IsValid()) return;
        WeakSelf->GetWorldTimerManager().SetTimer(WeakSelf->AbilityLogicTimer, [WeakSelf]() {
            if (WeakSelf.IsValid())
            {
                WeakSelf->bIsLookingAtAbilityTarget = false;
                WeakSelf->bIsBusy = false;
                WeakSelf->ScheduleNextAttack(2.0f);
            }
            }, 1.0f, false);

        }, Delay, false);
}

void AHades::PlayFireball()
{
    if (!CurrentTarget)
    {
        ScheduleNextAttack(1.0f);
        return;
    }

    bIsBusy = true;
    bIsLookingAtAbilityTarget = true;
    AbilityLookAtTarget = CurrentTarget->GetActorLocation();

    if (SkeletalMesh)
    {
        UAnimInstance* AnimInst = SkeletalMesh->GetAnimInstance();
        if (AnimInst && FireballMontage)
        {
            AnimInst->Montage_Play(FireballMontage, 1.5f);
            UE_LOG(LogTemp, Log, TEXT("Hades: Fireball Animation Started"));
        }
    }

    GetWorldTimerManager().SetTimer(AbilityLogicTimer, [this]() {

        int Count = ((int)CurrentPhase * 5) + 7;
        FVector Center = CurrentTarget ? CurrentTarget->GetActorLocation() : FVector(0, 0, 0);

        for (int i = 0; i < Count; i++)
        {
            FVector Loc = CalculateFireballLocation(Center);
            Loc.Z = 50.0f;

            float FireballDuration = 2.0f;
            AActor* Pred = SpawnPrediction(FireballPredictionClass, Loc, FireballDuration, FVector(1));
            TWeakObjectPtr<AActor> WeakPred = Pred;

            FTimerHandle FireHandle;
            GetWorldTimerManager().SetTimer(FireHandle, [this, Loc, WeakPred]() {

                DrawDebugSphere(GetWorld(), Loc, 400.0f, 12, FColor::Red, false, 2.0f);
                UGameplayStatics::ApplyRadialDamage(GetWorld(), 30.0f, Loc, 400.0f, nullptr, { this }, nullptr, nullptr, true);

                if (WeakPred.IsValid()) WeakPred->Destroy();
                }, FireballDuration, false);
        }

        GetWorldTimerManager().SetTimer(AbilityLogicTimer, [this]() {
            bIsLookingAtAbilityTarget = false;
            bIsBusy = false;
            ScheduleNextAttack(1.5f);
            }, 2.5f, false);

        }, 0.8f, false);
}

void AHades::PlaySweep()
{
    if (!CurrentTarget) { ScheduleNextAttack(1.0f); return; }

    bIsBusy = true;
    bIsInitialSlamming = true;
    bIsLookingAtAbilityTarget = false;

    FVector DirToPlayer = CurrentTarget->GetActorLocation() - GetActorLocation();
    DirToPlayer.Z = 0;
    float AngleToPlayer = DirToPlayer.Rotation().Yaw;

    SweepStartYaw = AngleToPlayer + 90.0f;

    FRotator InitialRot = FRotator(0, (SweepStartYaw - RotationOffset) - 90.0f, 0);
    SetActorRotation(InitialRot);

    if (SkeletalMesh && SweepMontage)
    {
        UAnimInstance* AnimInst = SkeletalMesh->GetAnimInstance();
        if (AnimInst) AnimInst->Montage_Play(SweepMontage, 1.5f);
    }

    FVector MyLocation = FVector(0,0,0);
    FVector SweepDir = FRotator(0, SweepStartYaw, 0).Vector();
    FVector SlamLoc = MyLocation + (SweepDir * 3450.0f);
    SlamLoc.Z = 20.0f;

    AActor* Pred = SpawnPrediction(SlamSweepPredictionClass, SlamLoc, 1.0f, FVector(1));
    TWeakObjectPtr<AActor> WeakPred = Pred;

    GetWorldTimerManager().SetTimer(AbilityLogicTimer, [this, SlamLoc, WeakPred, MyLocation]() {
        if (WeakPred.IsValid()) WeakPred->Destroy();

        ExecuteDamage(SlamLoc, 800.0f);

        bIsInitialSlamming = false;
        bIsSweeping = true;
        SweepElapsedTime = 0.0f;
        SweepRotationCount = FMath::RandRange(1, (int)CurrentPhase + 2);
        if (SweepPredictionClass)
        {
            FActorSpawnParameters SpawnParams;
            SpawnParams.Owner = this;
            SpawnParams.Instigator = GetInstigator();
            SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

            FRotator FlatRotation = FRotator(0, SweepStartYaw, 0);

            ActiveSweepBeam = GetWorld()->SpawnActor<AActor>(SweepPredictionClass, SlamLoc, FlatRotation, SpawnParams);
        }
        }, 1.0f, false);
}

void AHades::UpdateSweep(float DeltaTime)
{
    if (SkeletalMesh)
    {
        UAnimInstance* AnimInst = SkeletalMesh->GetAnimInstance();
        if (AnimInst && SweepMontage)
        {
            AnimInst->Montage_SetPlayRate(SweepMontage, 0.0f);
        }
    }

    SweepElapsedTime += DeltaTime;
    float Alpha = FMath::Clamp(SweepElapsedTime / SweepDuration, 0.0f, 1);

    float TotalDegrees = 360.0f * (SweepRotationCount);
    SweepCurrentAngle = FMath::Pow(Alpha, 4.0f) * TotalDegrees;

    float MathAngle = SweepStartYaw - SweepCurrentAngle;
    FRotator VisualRot = FRotator(0, (MathAngle - RotationOffset) - 90.0f, 0);
    FRotator BeamRot = FRotator(0, MathAngle, 0);

    SetActorRotation(VisualRot);

    FVector BeamStart = FVector(0,0,20);
    FVector BeamEnd = BeamStart + (BeamRot.Vector() * 3450.0f);
    FVector BoxCenter = (BeamStart + BeamEnd) * 0.77;
    TArray<FOverlapResult> Overlaps;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);
    if (ActiveSweepBeam.IsValid()) Params.AddIgnoredActor(ActiveSweepBeam.Get()); 

    bool bHit = GetWorld()->OverlapMultiByChannel(
        Overlaps,
        BoxCenter,
        BeamRot.Quaternion(),
        ECC_Visibility,
        FCollisionShape::MakeBox(FVector(1725.0f, 100.0f, 200.0f)),
        Params
    );

    //Visualize damage zone
    //DrawDebugBox(GetWorld(), BoxCenter, FVector(1725.0f, 300.0f, 200.0f), BeamRot.Quaternion(), FColor::Yellow, false, -1.0f);

    for (auto& Overlap : Overlaps)
    {
        if (AActor* HitActor = Overlap.GetActor())
        {
            if (HitActor != this)
            {
                UGameplayStatics::ApplyDamage(HitActor, 100.0f, nullptr, this, UDamageType::StaticClass());
            }
        }
    }

    if (ActiveSweepBeam.IsValid())
    {
        ActiveSweepBeam->SetActorLocation(BeamEnd);
        ActiveSweepBeam->SetActorRotation(BeamRot);
    }

    if (Alpha >= 1.0f)
    {
        bIsSweeping = false;
        if (ActiveSweepBeam.IsValid()) ActiveSweepBeam->Destroy();
        bIsBusy = false;
        ScheduleNextAttack(2.0f);
    }
}

void AHades::UpdateRotation(float DeltaTime)
{
    FVector LookTargetLoc;
    if (bIsLookingAtAbilityTarget) LookTargetLoc = AbilityLookAtTarget;
    else if (CurrentTarget)         LookTargetLoc = CurrentTarget->GetActorLocation();
    else return;

    FVector LookAtDir = (LookTargetLoc - GetActorLocation()).GetSafeNormal2D();
    FRotator TargetRotation = LookAtDir.Rotation();
    TargetRotation.Yaw -= RotationOffset;

    float Speed = bIsLookingAtAbilityTarget ? RotationSpeed * 2.0f : RotationSpeed;
    SetActorRotation(FMath::RInterpTo(GetActorRotation(), TargetRotation, DeltaTime, Speed));
}

void AHades::UpdateIdleAnimation()
{
    if (!bIsBusy && !bIsInitialSlamming && !bIsSweeping && SkeletalMesh && IdleMontage)
    {
        UAnimInstance* Anim = SkeletalMesh->GetAnimInstance();
        if (Anim && !Anim->Montage_IsPlaying(IdleMontage))
            Anim->Montage_Play(IdleMontage);
    }
}

AActor* AHades::SpawnPrediction(TSubclassOf<AActor> Class, FVector Location, float Duration, FVector Scale)
{
    if (!Class) return nullptr;
    AActor* Pred = GetWorld()->SpawnActor<AActor>(Class, Location + FVector(0, 0, 5), FRotator::ZeroRotator);
    if (Pred) SetupPredictionScaling(Pred, Duration, Scale);
    return Pred;
}

void AHades::ExecuteDamage(FVector ImpactLocation, float ImpactRadius)
{
    DrawDebugSphere(GetWorld(), ImpactLocation, ImpactRadius, 32, FColor::Orange, false, 2.0f);
    UGameplayStatics::ApplyRadialDamage(GetWorld(), 40.0f * (1 + (int)CurrentPhase), ImpactLocation, ImpactRadius, nullptr, { this }, this, nullptr, true);
}

void AHades::SelectRandomTarget()
{
    TArray<AActor*> Players;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACharacter::StaticClass(), Players);

    if (Players.Num() > 0)
    {
        int32 RandomIndex = FMath::RandRange(0, Players.Num() - 1);
        CurrentTarget = Players[RandomIndex];
    }
}

FVector AHades::CalculateFireballLocation(FVector Center)
{
    float SearchRadius = 1800.0f + ((int)CurrentPhase * 200.0f);

    FVector RandomOffset = FMath::VRand() * FMath::FRandRange(500.f, SearchRadius);
    FVector TargetLoc = Center + RandomOffset;
    TargetLoc.Z = 0;

    FVector DirFromCenter = TargetLoc - FVector::ZeroVector;
    float Dist = DirFromCenter.Size();

    return DirFromCenter.GetSafeNormal() * FMath::Clamp(Dist, 2500.0f, 4400.0f);
}

void AHades::SetupPredictionScaling(AActor* Prediction, float Duration, FVector MaxScale)
{
    if (!Prediction) return;

    TWeakObjectPtr<AActor> WeakPred = Prediction;
    FVector StartScale = FVector(0.1f, 0.1f, 1.0f);
    Prediction->SetActorScale3D(StartScale);

    float Interval = 0.02f;

    FVector GrowthStep = (MaxScale - StartScale) / (Duration / Interval);

    FTimerHandle ScaleTimer;
    GetWorldTimerManager().SetTimer(ScaleTimer, [this, WeakPred, MaxScale, GrowthStep, ScaleTimer]() mutable {
        if (WeakPred.IsValid())
        {
            FVector Current = WeakPred->GetActorScale3D();

            if (Current.X >= MaxScale.X && Current.Y >= MaxScale.Y)
            {
                GetWorldTimerManager().ClearTimer(ScaleTimer);
                return;
            }

            FVector NextScale = Current + GrowthStep;

            NextScale.X = FMath::Min(NextScale.X, MaxScale.X);
            NextScale.Y = FMath::Min(NextScale.Y, MaxScale.Y);

            WeakPred->SetActorScale3D(NextScale);
        }
        else
        {
            GetWorldTimerManager().ClearTimer(ScaleTimer);
        }
        }, Interval, true);
}

void AHades::NotifyObeliskHit(AActor* Obelisk)
{
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

void AHades::Die()
{
    GetWorldTimerManager().ClearAllTimersForObject(this);

    if (ActiveSweepBeam.IsValid())
    {
        ActiveSweepBeam->Destroy();
    }

    if (ActiveCycloneComp)
    {
        ActiveCycloneComp->DestroyComponent();
    }

    Destroy();
}

void AHades::CheckPhaseTransition()
{
    float HPPercent = CurrentHealth / MaxHealth;

    if (HPPercent <= 0.25f) { CurrentPhase = EBossPhase::Phase4; SweepDuration = 5; }
    else if (HPPercent <= 0.50f) { CurrentPhase = EBossPhase::Phase3; SweepDuration = 6; }
    else if (HPPercent <= 0.75f) { CurrentPhase = EBossPhase::Phase2; SweepDuration = 7; }
    else { CurrentPhase = EBossPhase::Phase1; SweepDuration = 8; }
}

void AHades::LockRotationTo(FVector TargetLocation)
{
    AbilityLookAtTarget = TargetLocation;
    bIsLookingAtAbilityTarget = true;
}