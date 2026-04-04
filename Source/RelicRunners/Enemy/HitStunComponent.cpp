// Fill out your copyright notice in the Description page of Project Settings.


#include "HitStunComponent.h"

// Sets default values for this component's properties
UHitStunComponent::UHitStunComponent()
{
	SetIsReplicatedByDefault(true);
	PrimaryComponentTick.bCanEverTick = false;	
}


void UHitStunComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorld()->GetTimerManager().ClearTimer(HitStunTimer);
	Super::EndPlay(EndPlayReason);
}

// Called when the game starts
void UHitStunComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UHitStunComponent::StartHitStun(float stunTime)
{
	GetWorld()->GetTimerManager().SetTimer(HitStunTimer, this, &UHitStunComponent::EndHitStun, stunTime, false);
	
	if (APawn* ownerPawn = Cast<APawn>(GetOwner()))
	{
		GetOwner()->CustomTimeDilation = 0.f;
	}
}

void UHitStunComponent::Multicast_StartHitStun_Implementation(float stunTime)
{
	StartHitStun(stunTime);
}

void UHitStunComponent::EndHitStun()
{
	if (APawn* ownerPawn = Cast<APawn>(GetOwner()))
	{
		GetOwner()->CustomTimeDilation = 1.f;
	}
}

void UHitStunComponent::Multicast_EndHitStun_Implementation()
{
	EndHitStun();
}



