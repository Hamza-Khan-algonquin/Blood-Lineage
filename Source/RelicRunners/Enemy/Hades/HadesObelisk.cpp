
#include "HadesObelisk.h"
#include "Hades.h"
#include "Engine/DamageEvents.h"
#include "Net/UnrealNetwork.h"
#include <Kismet/GameplayStatics.h>

AHadesObelisk::AHadesObelisk()
{
	PrimaryActorTick.bCanEverTick = true;
    ObeliskComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ObeliskComponent"));
    RootComponent = ObeliskComponent;
    ShieldComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ShieldComponent"));
    ShieldComponent->SetupAttachment(RootComponent);
    ShieldComponent->SetVisibility(false);
    ShieldComponent->SetCollisionResponseToAllChannels(ECR_Ignore);

    bReplicates = true;
}

void AHadesObelisk::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AHadesObelisk, bIsRaging);
    DOREPLIFETIME(AHadesObelisk, CurrentRotationMultiplier);
}

void AHadesObelisk::OnRep_IsRaging()
{
    UpdateRageVisuals();
}

void AHadesObelisk::UpdateRageVisuals()
{
    if (ShieldComponent)
    {
        ShieldComponent->SetVisibility(bIsRaging);

        UMaterialInterface* TargetMat = bIsRaging ? GlowRed : GlowBlue;
        if (ObeliskComponent && TargetMat)
        {
            ObeliskComponent->SetMaterial(1, TargetMat);
        }
    }
}

void AHadesObelisk::BeginPlay()
{
	Super::BeginPlay();
	
    if (ObeliskMeshAsset)
    {
        ObeliskComponent->SetStaticMesh(ObeliskMeshAsset);
    }

    if (ShieldMeshAsset)
    {
        ShieldComponent->SetStaticMesh(ShieldMeshAsset);
    }

    InitialZ = GetActorLocation().Z;
    RunningTime = 0.0f;
}

void AHadesObelisk::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    float RotationSpeed = 200.0f * CurrentRotationMultiplier;
    AddActorLocalRotation(FRotator(0, RotationSpeed * DeltaTime, 0));

    FVector CurrentLocation = GetActorLocation();
    float TargetZ = InitialZ;

    if (!bIsRaging)
    {
        RunningTime += DeltaTime;

        float BobSpeed = 2.0f;
        float BobAmount = 100.0f;

        float DeltaZ = FMath::Sin(RunningTime * BobSpeed) * BobAmount;
        TargetZ = InitialZ + DeltaZ;

        CurrentLocation.Z = TargetZ;
    }
    else
    {
        CurrentLocation.Z = FMath::FInterpTo(CurrentLocation.Z, InitialZ, DeltaTime, 5.0f);
    }

    SetActorLocation(CurrentLocation);
}

float AHadesObelisk::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
    if (bIsRaging) return 0.0f;

    AHades* HadesBoss = Cast<AHades>(DamageCauser);
    if (HadesBoss && !bWasTriggered)
    {
        bWasTriggered = true;

        HadesBoss->NotifyObeliskHit(this);

        TArray<AActor*> OutObelisks;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), AHadesObelisk::StaticClass(), OutObelisks);

        for (AActor* Actor : OutObelisks)
        {
            AHadesObelisk* OtherObelisk = Cast<AHadesObelisk>(Actor);

            if (OtherObelisk && OtherObelisk != this)
            {
                OtherObelisk->TriggerRageMode();
            }
        }

        if (ExplosionSound)
        {
            UGameplayStatics::PlaySoundAtLocation(this, ExplosionSound, GetActorLocation());
        }

        Destroy();
    }

    return Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
}

void AHadesObelisk::TriggerRageMode()
{
    if (!HasAuthority()) return;

    bIsRaging = true;
    CurrentRotationMultiplier = 3.0f;

    UpdateRageVisuals();

    GetWorldTimerManager().SetTimer(RageTimerHandle, this, &AHadesObelisk::EndRageMode, 20.0f, false);
}

void AHadesObelisk::EndRageMode()
{
    if (!HasAuthority()) return;

    bIsRaging = false;
    CurrentRotationMultiplier = 1.0f;

    UpdateRageVisuals();
}