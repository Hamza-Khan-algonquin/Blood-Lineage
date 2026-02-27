
#include "HadesObelisk.h"
#include "Hades.h"
#include "Engine/DamageEvents.h"
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
}

void AHadesObelisk::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    float Speed = 90.0f * CurrentRotationMultiplier;
    AddActorLocalRotation(FRotator(0, Speed * DeltaTime, 0));
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

        Destroy();
    }

    return Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
}

void AHadesObelisk::TriggerRageMode()
{
    bIsRaging = true;
    CurrentRotationMultiplier = 3.0f;

    if (ShieldComponent)
    {
        ShieldComponent->SetVisibility(true);
    }

    GetWorldTimerManager().SetTimer(RageTimerHandle, this, &AHadesObelisk::EndRageMode, 20.0f, false);
}

void AHadesObelisk::EndRageMode()
{
    bIsRaging = false;
    CurrentRotationMultiplier = 1.0f;

    if (ShieldComponent)
    {
        ShieldComponent->SetVisibility(false);
    }
}