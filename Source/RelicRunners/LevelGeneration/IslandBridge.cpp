// Fill out your copyright notice in the Description page of Project Settings.


#include "IslandBridge.h"

// Sets default values
AIslandBridge::AIslandBridge()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Bridge = CreateDefaultSubobject<UStaticMeshComponent>("Bridge Mesh");
	Bridge->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void AIslandBridge::BeginPlay()
{
	Super::BeginPlay();

	Bridge->SetVisibility(false);
	Bridge->SetCollisionProfileName("NoCollision");

}

// Called every frame
void AIslandBridge::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AIslandBridge::Activate()
{
	Bridge->SetVisibility(true);
	Bridge->SetCollisionProfileName("BlockAll");
}

