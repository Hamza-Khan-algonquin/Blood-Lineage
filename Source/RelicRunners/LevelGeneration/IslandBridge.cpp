// Fill out your copyright notice in the Description page of Project Settings.


#include "IslandBridge.h"
#include <Net/UnrealNetwork.h>

// Sets default values
AIslandBridge::AIslandBridge():
	IsBlocked(true)
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	Bridge = CreateDefaultSubobject<UStaticMeshComponent>("Bridge Mesh");
	Bridge->SetupAttachment(RootComponent);

	Barrier = CreateDefaultSubobject<UStaticMeshComponent>("Barrier Mesh");
	Barrier->SetupAttachment(Bridge);
	Barrier->SetIsReplicated(true);

	BackBarrier = CreateDefaultSubobject<UStaticMeshComponent>("Back Barrier Mesh");
	BackBarrier->SetupAttachment(Bridge);
	BackBarrier->SetIsReplicated(true);

	bReplicates = true;
	bAlwaysRelevant = true;
}

// Called when the game starts or when spawned
void AIslandBridge::BeginPlay()
{
	Super::BeginPlay();

	Barrier->SetVisibility(true);
	Barrier->SetCollisionProfileName("BlockAll");
}

void AIslandBridge::Activate()
{
	if (HasAuthority())
	{
		IsBlocked = false;
		Barrier->SetVisibility(IsBlocked);
		Barrier->SetCollisionProfileName("NoCollision");
	}
}

void AIslandBridge::SetFinalBridge(bool isFinal)
{
	if (HasAuthority())
	{
		IsFinalBridge = isFinal;
		if (IsFinalBridge)
		{
			BackBarrier->SetVisibility(true);
			BackBarrier->SetCollisionProfileName("BlockAll");
		}
		else
		{
			BackBarrier->SetVisibility(false);
			BackBarrier->SetCollisionProfileName("NoCollision");
		}
	}
}

void AIslandBridge::OnRep_OnBlockedChange()
{
	Barrier->SetVisibility(false);
	Barrier->SetCollisionProfileName("NoCollision");
}

void AIslandBridge::OnRep_OnFinalBridgeChange()
{
	if (IsFinalBridge)
	{
		BackBarrier->SetVisibility(true);
		BackBarrier->SetCollisionProfileName("BlockAll");
	}
	else
	{
		Barrier->SetVisibility(false);
		Barrier->SetCollisionProfileName("NoCollision");
	}
}

void AIslandBridge::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AIslandBridge, IsBlocked);
	DOREPLIFETIME(AIslandBridge, IsFinalBridge);
}