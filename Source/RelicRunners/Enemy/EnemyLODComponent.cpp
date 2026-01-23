// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyLODComponent.h"
#include "RelicRunners/Director System/Director.h"

// Sets default values for this component's properties
UEnemyLODComponent::UEnemyLODComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PrePhysics;
	
	DistanceOfLOD.Add(1000.f);
	DistanceOfLOD.Add(4000.f);
	DistanceOfLOD.Add(10000.f);
	DistanceOfLOD.Add(15000.f);
}


void UEnemyLODComponent::UpdateLOD()
{
	TArray<APawn*> players;
	//APawn* closestPlayer = nullptr;
	int newLOD = -1;

	if (ADirector* dir = ADirector::GetInstance(this))
	{
		players = dir->GetPlayers();
	}
	else
	{
		return;
	}

	if (players.Num() <= 0)
	{
		return;
	}

	float closeDistance = FVector::Dist(players[0]->GetActorLocation(), GetOwner()->GetActorLocation());
	//closestPlayer = players[0];

	for (int i = 1; i < players.Num(); i++)
	{
		float newDistance = FVector::Dist(players[i]->GetActorLocation(), GetOwner()->GetActorLocation());

		if (closeDistance > newDistance)
		{
			//closestPlayer = players[i];
			closeDistance = newDistance;
		}
	}

	for (int i = 0; i < DistanceOfLOD.Num(); i++)
	{
		if (closeDistance <= DistanceOfLOD[i])
		{
			newLOD = i;
			break;
		}
	}

	if (newLOD == -1 && IsUpdatingLOD)
	{
		newLOD = DistanceOfLOD.Num();
	}
	else if (newLOD == -1)
	{
		newLOD = DistanceOfLOD.Num() - 1;
	}

	SetLOD(newLOD);
}

void UEnemyLODComponent::SetLOD(int newLOD)
{
	if (newLOD != CurrentLOD)
	{
		
		OnUpdateLODEvent.Broadcast(newLOD, CurrentLOD);
		CurrentLOD = newLOD;

		switch (CurrentLOD)
		{
		case 0:
			PrimaryComponentTick.TickInterval = 0.5f;
			break;
		case 1:
			PrimaryComponentTick.TickInterval = 0.5f;
			break;
		case 2:
			PrimaryComponentTick.TickInterval = 1.0f;
			break;
		case 3:
			PrimaryComponentTick.TickInterval = 2.0f;
			break;
		case 4:
			PrimaryComponentTick.TickInterval = 60.f;
			break;
		}
	}
}

void UEnemyLODComponent::SetIsUpdatingLOD(bool isUpdating)
{
	IsUpdatingLOD = isUpdating;

	if(isUpdating)
	{	
		SetComponentTickEnabled(true);
	}
	else
	{
		SetComponentTickEnabled(false);
	}
}

// Called when the game starts
void UEnemyLODComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UEnemyLODComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	UpdateLOD();
}

