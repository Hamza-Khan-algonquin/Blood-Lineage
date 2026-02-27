// Fill out your copyright notice in the Description page of Project Settings.


#include "ObjectiveBase.h"

// Sets default values
AObjectiveBase::AObjectiveBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Origin = CreateDefaultSubobject<USceneComponent>("Origin");
	RootComponent = Origin;

	BeaconMesh = CreateDefaultSubobject<UStaticMeshComponent>("Beacon");
	BeaconMesh->SetCollisionProfileName("NoCollision");
	BeaconMesh->SetupAttachment(RootComponent);

	bReplicates = true;
	bAlwaysRelevant = true;
}

// Called when the game starts or when spawned
void AObjectiveBase::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void AObjectiveBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AObjectiveBase::Activate()
{
	OnObjectiveCreated.Broadcast();
}