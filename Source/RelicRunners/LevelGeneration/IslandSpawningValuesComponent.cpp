// Fill out your copyright notice in the Description page of Project Settings.


#include "IslandSpawningValuesComponent.h"
#include <Net/UnrealNetwork.h>

// Sets default values for this component's properties
UIslandSpawningValuesComponent::UIslandSpawningValuesComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

}

void UIslandSpawningValuesComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UIslandSpawningValuesComponent, IslandValues);
	DOREPLIFETIME(UIslandSpawningValuesComponent, IslandStartEnd);
	DOREPLIFETIME(UIslandSpawningValuesComponent, FloorMeshes);
	DOREPLIFETIME(UIslandSpawningValuesComponent, ObstacleValues);
	DOREPLIFETIME(UIslandSpawningValuesComponent, VantageValues);
	DOREPLIFETIME(UIslandSpawningValuesComponent, FlagObjectiveValues);
	DOREPLIFETIME(UIslandSpawningValuesComponent, PayloadObjectiveValues);
	DOREPLIFETIME(UIslandSpawningValuesComponent, SurvivalZoneObjectiveValues);
}