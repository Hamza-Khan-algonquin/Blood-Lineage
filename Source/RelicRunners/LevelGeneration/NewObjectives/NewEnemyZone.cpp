// Fill out your copyright notice in the Description page of Project Settings.


#include "NewEnemyZone.h"
#include "Components/SceneComponent.h"
#include "RelicRunners/RelicRunnersGameState.h"
#include "RelicRunners/Director System/SpawnZone.h"
#include <Net/UnrealNetwork.h>

ANewEnemyZone::ANewEnemyZone() :
	SpawnZoneTemplate(nullptr),
	IsActive(true)

{
	PrimaryActorTick.bCanEverTick = false;

	bReplicates = true;
	bAlwaysRelevant = true;
}

void ANewEnemyZone::BeginPlay()
{
	Super::BeginPlay();

	if (SpawnZoneTemplate == nullptr)
		return;

	ASpawnZone* spawnZone = GetWorld()->SpawnActor<ASpawnZone>(SpawnZoneTemplate, GetActorLocation(), GetActorRotation());

	if (spawnZone)
	{
		spawnZone->OnEnemyWavesComplete.AddDynamic(this, &ANewEnemyZone::Server_HandleEnemyWavesCompleted);
	}
}

void ANewEnemyZone::Server_HandleEnemyWavesCompleted_Implementation()
{
	if (!IsActive)
		return;

	IsActive = false;
	BeaconMesh->SetVisibility(false);
	OnObjectiveActivated.Broadcast();
}

void ANewEnemyZone::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ANewEnemyZone, IsActive);
}