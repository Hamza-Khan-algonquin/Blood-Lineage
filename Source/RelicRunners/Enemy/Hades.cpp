
#include "Hades.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"

AHades::AHades()
{
	PrimaryActorTick.bCanEverTick = true;

}

void AHades::BeginPlay()
{
    Super::BeginPlay();

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
            ActiveCycloneComp->SetVariableFloat(FName("User.VortexStrength"), 100.0f);
            ActiveCycloneComp->SetTickBehavior(ENiagaraTickBehavior::ForceTickFirst);
        }
    }
}

void AHades::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

