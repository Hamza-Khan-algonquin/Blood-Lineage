// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyZone.h"
#include "Components/SceneComponent.h"
#include "Components/BoxComponent.h"
#include "RelicRunners/RelicRunnersGameState.h"
#include "RelicRunners/Director System/SpawnZone.h"
#include <Net/UnrealNetwork.h>

// Sets default values
AEnemyZone::AEnemyZone():
	SpawnZoneTemplate(nullptr),
	SpawnZone(nullptr),
	IsActive(true)

{
	PrimaryActorTick.bCanEverTick = false;

	Origin = CreateDefaultSubobject<USceneComponent>("Origin");
	RootComponent = Origin;

	TriggerBox = CreateDefaultSubobject<UBoxComponent>("TriggerCollision");
	TriggerBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TriggerBox->SetCollisionProfileName("BlockAll");
	TriggerBox->SetGenerateOverlapEvents(true);
	TriggerBox->SetupAttachment(Origin);

	bReplicates = true;
	bAlwaysRelevant = true;
}

// Called when the game starts or when spawned
void AEnemyZone::BeginPlay()
{
	Super::BeginPlay();

	if (SpawnZoneTemplate == nullptr)
		return;

	SpawnZone = GetWorld()->SpawnActor<ASpawnZone>(SpawnZoneTemplate, GetActorLocation(), GetActorRotation());

	if (SpawnZone)
	{
		SpawnZone->OnEnemyWavesComplete.AddDynamic(this, &AEnemyZone::Server_HandleEnemyWavesCompleted);
	}
}

void AEnemyZone::Server_HandleEnemyWavesCompleted_Implementation()
{

}

void AEnemyZone::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AEnemyZone, SpawnZone);
}