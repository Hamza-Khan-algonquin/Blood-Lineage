// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyStats.h"
#include <Net/UnrealNetwork.h>

// Sets default values for this component's properties
UEnemyStats::UEnemyStats()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	EnemyName = "Unnamed";
	EnemyLevel = 1;
	EnemyType = EEnemyType::UNKNOWN;
	MaxHealth = 100;
	CurrentHealth = 100;
	RemainingStunTime = 0;
	ChanceToDropItem = 0.01;
}

void UEnemyStats::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UEnemyStats, EnemyName);
	DOREPLIFETIME(UEnemyStats, EnemyLevel);
	DOREPLIFETIME(UEnemyStats, EnemyType);
	DOREPLIFETIME(UEnemyStats, MaxHealth);
	DOREPLIFETIME(UEnemyStats, CurrentHealth);
	DOREPLIFETIME(UEnemyStats, RemainingStunTime);
	DOREPLIFETIME(UEnemyStats, ChanceToDropItem);
}

#if WITH_EDITOR
void UEnemyStats::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	//calling the super
	Super::PostEditChangeProperty(PropertyChangedEvent);

	//crabing the property name
	FName PropertyName = NAME_None;

	//check to make sure this is a actual property
	if (PropertyChangedEvent.Property)
	{
		//update the name
		PropertyName = PropertyChangedEvent.Property->GetFName();
	}

	//go throught and check the possible properties that need to update other variables
	//GET_MEMBER_NAME_CHECKED is a compiler time macro that gets the name of a property for anyone who is readying this and if so thank you for reading my comments
	if (PropertyName == GET_MEMBER_NAME_CHECKED(UEnemyStats, MaxHealth))
	{
		CurrentHealth = FMath::Min(CurrentHealth, MaxHealth);
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(UEnemyStats, CurrentHealth))
	{
		CurrentHealth = FMath::Clamp(CurrentHealth, 0.f, MaxHealth);
	}
}
#endif


