
#include "Hades.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include <Kismet/GameplayStatics.h>
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

AHades::AHades()
{
	PrimaryActorTick.bCanEverTick = true;

}

void AHades::BeginPlay()
{
    Super::BeginPlay();

    //Soul well animation
    if (GhostCycloneSystem)
    {
        FVector ArenaCenter = FVector(0.f, 0.f, -300.f);

        ActiveCycloneComp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            GetWorld(),
            GhostCycloneSystem,
            ArenaCenter,
            FRotator::ZeroRotator,
            FVector(1.f),
            false
        );

        if (ActiveCycloneComp)
        {
            ActiveCycloneComp->SetVariableFloat(FName("User.SpawnRate"), 1000.0f);
            ActiveCycloneComp->SetVariableFloat(FName("User.VortexStrength"), 125.0f);
            ActiveCycloneComp->SetTickBehavior(ENiagaraTickBehavior::ForceTickFirst);
        }
    }

    GetWorldTimerManager().SetTimer(AttackTimerHandle, this, &AHades::ExecuteAttack, 2.0f, false);
}

void AHades::CheckPhaseTransition()
{
    float HealthPercentage = CurrentHealth / MaxHealth;

    if (HealthPercentage <= 0.25f) CurrentPhase = EBossPhase::Phase4;
    else if (HealthPercentage <= 0.50f) CurrentPhase = EBossPhase::Phase3;
    else if (HealthPercentage <= 0.75f) CurrentPhase = EBossPhase::Phase2;
}

void AHades::SelectRandomTarget()
{
    TArray<AActor*> Players;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACharacter::StaticClass(), Players);

    if (Players.Num() > 0)
    {
        int RandomIndex = FMath::RandRange(0, Players.Num() - 1);
        CurrentTarget = Players[RandomIndex];
    }
}

void AHades::ExecuteAttack()
{
    CheckPhaseTransition();
    SelectRandomTarget();

    int Roll = FMath::RandRange(1, 100);

    if (Roll <= 30)           PlaySlam();     
    else if (Roll <= 80)      PlayFireball(); 
    else                      PlaySweep();    
}

void AHades::ScheduleNextAttack(float Delay)
{
    GetWorldTimerManager().SetTimer(AttackTimerHandle, this, &AHades::ExecuteAttack, Delay, false);
}

void AHades::PlaySlam()
{
    if (!CurrentTarget) { ScheduleNextAttack(1.0f); return; }

    FVector ToPlayer = (CurrentTarget->GetActorLocation() - GetActorLocation()).GetSafeNormal2D();
    FVector SlamLocation = GetActorLocation() + (ToPlayer * 3450.0f);

    AbilityLookAtTarget = SlamLocation;
    bIsLookingAtAbilityTarget = true;

    float Radius = 1800.0f;
    DrawDebugCircle(GetWorld(), SlamLocation, Radius, 100, FColor::Red, false, 5.0f, 0, 20.0f, FVector(0, 1, 0), FVector(1, 0, 0));

    FTimerHandle UniqueSlamHandle;
    GetWorldTimerManager().SetTimer(UniqueSlamHandle, FTimerDelegate::CreateLambda([this, SlamLocation, Radius]()
        {
            ExecuteSlamImpact(SlamLocation, Radius);

            bIsLookingAtAbilityTarget = false;
            ScheduleNextAttack(2.0f);
        }), 4.0f, false);
}

void AHades::ExecuteSlamImpact(FVector ImpactLocation, float ImpactRadius)
{
    if (CurrentTarget)
    {
        ImpactLocation.Z = CurrentTarget->GetActorLocation().Z;
    }

    DrawDebugSphere(GetWorld(), ImpactLocation, ImpactRadius, 32, FColor::Orange, false, 2.0f);

    TArray<AActor*> IgnoredActors;
    IgnoredActors.Add(this);

    UGameplayStatics::ApplyRadialDamage(
        GetWorld(),
        80.0f,             
        ImpactLocation,    
        ImpactRadius,       
        UDamageType::StaticClass(),
        IgnoredActors,
        this,
        nullptr,    
        true                
    );
}

void AHades::PlaySweep()
{
    if (bIsSweeping) return;

    float SlamYaw = FMath::RandRange(0.f, 360.f);
    SweepStartYaw = SlamYaw;

    SetActorRotation(FRotator(0, SlamYaw, 0));

    bIsInitialSlamming = true;

    FVector SlamDir = FRotator(0, SlamYaw, 0).Vector();
    FVector SweepSlamLoc = GetActorLocation() + (SlamDir * 3450.0f);
    float LocalSlamRadius = (CurrentPhase == EBossPhase::Phase4) ? 1000.0f : 800.0f;

    DrawDebugCircle(GetWorld(), SweepSlamLoc, LocalSlamRadius, 50, FColor::Cyan, false, 1.0f, 0, 15.0f, FVector(0, 1, 0), FVector(1, 0, 0));

    FTimerHandle SweepTransitionHandle;
    GetWorldTimerManager().SetTimer(SweepTransitionHandle, FTimerDelegate::CreateLambda([this, SweepSlamLoc, LocalSlamRadius]()
        {
            ExecuteSlamImpact(SweepSlamLoc, LocalSlamRadius);

            bIsInitialSlamming = false;
            bIsSweeping = true;

            SweepElapsedTime = 0.0f;

            SweepCurrentAngle = 0.0f;
            SweepSpeed = 40.0f;
        }), 1.0f, false);
}

void AHades::UpdateSweep(float DeltaTime)
{
    SweepElapsedTime += DeltaTime;

    float Alpha = FMath::Clamp(SweepElapsedTime / SweepDuration, 0.0f, 1.0f);
    float CurvedAlpha = FMath::Pow(Alpha, 4.0f);

    SweepCurrentAngle = CurvedAlpha * 360.0f;

    float NewYaw = SweepStartYaw + SweepCurrentAngle;
    SetActorRotation(FRotator(0, NewYaw, 0));

    FVector ScytheDir = GetActorForwardVector();
    float InnerRadius = 2500.0f;
    float OuterRadius = 4400.0f;

    FVector LineStart = GetActorLocation() + (ScytheDir * InnerRadius);
    FVector LineEnd = GetActorLocation() + (ScytheDir * OuterRadius);
    DrawDebugLine(GetWorld(), LineStart, LineEnd, FColor::Red, false, -1, 0, 40.f);

    TArray<AActor*> Players;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACharacter::StaticClass(), Players);

    for (AActor* P : Players)
    {
        FVector ToPlayer = P->GetActorLocation() - GetActorLocation();
        float Dist = ToPlayer.Size();

        if (Dist >= InnerRadius && Dist <= OuterRadius)
        {
            ToPlayer.Normalize();
            float Dot = FVector::DotProduct(ScytheDir, ToPlayer);
            if (Dot > 0.998f)
            {
                UGameplayStatics::ApplyDamage(P, 15.0f, nullptr, this, UDamageType::StaticClass());
            }
        }
    }

    if (Alpha >= 1.0f)
    {
        bIsSweeping = false;
        ScheduleNextAttack(2.0f);
    }
}

void AHades::PlayFireball()
{
    if (!CurrentTarget) { ScheduleNextAttack(1.0f); return; }

    FVector PlayerLoc = CurrentTarget->GetActorLocation();

    AbilityLookAtTarget = PlayerLoc;
    bIsLookingAtAbilityTarget = true;

    int FireballCount = (int)CurrentPhase * 3 + 4;
    float ScatterRadius = 1200.0f;

    for (int i = 0; i < FireballCount; i++)
    {
        FVector RandomOffset = FMath::VRand() * FMath::FRandRange(0.f, ScatterRadius);
        RandomOffset.Z = 0;
        FVector FinalDropLoc = PlayerLoc + RandomOffset;

        FVector DirFromCenter = FinalDropLoc - GetActorLocation();
        float DistFromCenter = DirFromCenter.Size();
        DirFromCenter.Normalize();

        float ClampedDist = FMath::Clamp(DistFromCenter, 2500.0f, 4400.0f);
        FinalDropLoc = GetActorLocation() + (DirFromCenter * ClampedDist);

        SpawnScatterFireball(FinalDropLoc);
    }

    ScheduleNextAttack(4.0f);
}

void AHades::SpawnScatterFireball(FVector TargetLocation)
{
    float Radius = 400.0f;

    DrawDebugCircle(GetWorld(), TargetLocation, Radius, 40, FColor::Yellow, false, 2.0f, 0, 8.0f, FVector(0, 1, 0), FVector(1, 0, 0));

    FTimerHandle ScatterTimer;
    FTimerDelegate TimerDel;
    TimerDel.BindLambda([this, TargetLocation, Radius]()
        {
            DrawDebugSphere(GetWorld(), TargetLocation, Radius, 16, FColor::Orange, false, 0.5f);

            TArray<AActor*> IgnoredActors;
            IgnoredActors.Add(this);

            UGameplayStatics::ApplyRadialDamage(
                GetWorld(),
                30.0f,
                TargetLocation,
                Radius,
                UDamageType::StaticClass(),
                IgnoredActors,
                nullptr,
                nullptr,
                true
            );
        });

    GetWorldTimerManager().SetTimer(ScatterTimer, TimerDel, 2.0f, false);
}

void AHades::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bIsSweeping || bIsInitialSlamming)
    {
        if (bIsSweeping)
        {
            UpdateSweep(DeltaTime);
        }
        return;
    }

    FVector LookTargetLoc;

    if (bIsLookingAtAbilityTarget)
    {
        LookTargetLoc = AbilityLookAtTarget;
    }
    else if (CurrentTarget)
    {
        LookTargetLoc = CurrentTarget->GetActorLocation();
    }
    else { return; }

    FVector LookAtDir = (LookTargetLoc - GetActorLocation()).GetSafeNormal2D();
    FRotator TargetRotation = LookAtDir.Rotation();
    TargetRotation.Yaw -= RotationOffset;

    SetActorRotation(FMath::RInterpTo(GetActorRotation(), TargetRotation, DeltaTime, RotationSpeed));
}

void AHades::NotifyObeliskHit(AActor* Obelisk)
{
    ObelisksHit++;

    float DamageToSelf = MaxHealth * 0.25f;
    CurrentHealth -= DamageToSelf;

    UE_LOG(LogTemp, Warning, TEXT("Hades hit an obelisk! Total: %d"), ObelisksHit);

    if (ObelisksHit >= 4 || CurrentHealth <= 0)
    {
        Die();
    }
}

void AHades::Die()
{
    GetWorldTimerManager().ClearAllTimersForObject(this);
    bIsSweeping = false;
    Destroy();
}