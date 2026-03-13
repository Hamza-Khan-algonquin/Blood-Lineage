// Fill out your copyright notice in the Description page of Project Settings.


#include "IslandBridge.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
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

	FrontCameraTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("FrontTrigger"));
	FrontCameraTrigger->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	FrontCameraTrigger->SetCollisionProfileName("OverlapAll");
	FrontCameraTrigger->SetGenerateOverlapEvents(true);
	FrontCameraTrigger->OnComponentBeginOverlap.AddDynamic(this, &AIslandBridge::OnFrontTriggerOverlap);
	FrontCameraTrigger->SetupAttachment(Bridge);

	BackCameraTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("BackTrigger"));
	BackCameraTrigger->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	BackCameraTrigger->SetCollisionProfileName("OverlapAll");
	BackCameraTrigger->SetGenerateOverlapEvents(true);
	BackCameraTrigger->OnComponentBeginOverlap.AddDynamic(this, &AIslandBridge::OnBackTriggerOverlap);
	BackCameraTrigger->SetupAttachment(Bridge);

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
		BackBarrier->SetVisibility(false);
		BackBarrier->SetCollisionProfileName("NoCollision");
	}
}

void AIslandBridge::OnFrontTriggerOverlap(UPrimitiveComponent* OverlapComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor->ActorHasTag("Player"))
		return;

	APawn* player = Cast<APawn>(OtherActor);

	if (player == nullptr)
		return;

	if (player->GetController() == nullptr)
		return; 

	if (player->GetController()->IsLocalPlayerController())
	{
		OnFrontTrigger.Broadcast();
	}
}

void AIslandBridge::OnBackTriggerOverlap(UPrimitiveComponent* OverlapComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor->ActorHasTag("Player"))
		return;

	APawn* player = Cast<APawn>(OtherActor);

	if (player == nullptr)
		return;

	if (player->GetController() == nullptr)
		return;

	if (player->GetController()->IsLocalPlayerController())
	{
		OnBackTrigger.Broadcast();
	}
}

void AIslandBridge::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AIslandBridge, IsBlocked);
	DOREPLIFETIME(AIslandBridge, IsFinalBridge);
}