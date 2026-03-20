// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyStats.h"
#include <Net/UnrealNetwork.h>
#include "RelicRunners/RelicRunnersGameState.h"
#include "RelicRunners/Game/RelicRunnersGameInstance.h"
#include "RelicRunners/Director System/Director.h"
#include "RelicRunners/PlayerState/RelicRunnersPlayerState.h"

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
	ChanceToDropItem = 30;
	AttackDamage = 3;
	SetIsReplicatedByDefault(true);
}

void UEnemyStats::BeginPlay()
{
	Super::BeginPlay();
	ResetStats();
	
}

void UEnemyStats::ResetStats()
{
	if(!GetOwner()->HasAuthority())
	{
		return;
	}

	URelicRunnersGameInstance* gameInst = GetWorld()->GetGameInstance<URelicRunnersGameInstance>();
	ADirector* director = ADirector::GetInstance(this);

	if (!gameInst || !director || director->GetPlayers().Num() == 0)
	{
		return;
	}

	if (ARelicRunnersGameState* gameState = GetWorld()->GetGameState<ARelicRunnersGameState>())
	{
		int newLevel = 0;

		for (APawn* playerCharacter : director->GetPlayers())
		{
			if (playerCharacter)
			{
				ARelicRunnersPlayerState* PS = playerCharacter->GetPlayerState<ARelicRunnersPlayerState>();

				if (PS)
				{
					newLevel += PS->GetPlayerLevel();
				}
			}
		}

		EnemyLevel = FMath::Max(1, newLevel / director->GetPlayers().Num());


		FDifficultyInfo DifficultyInfo = gameState->GetDifficultyInfo(gameInst->CurrentDifficulty);
		if (EnemyLevel > 1)
		{
			MaxHealth *= DifficultyInfo.HealthPercent * HealthMultiplier * EnemyLevel;
			CurrentHealth = MaxHealth;
			AttackDamage *= DifficultyInfo.DamagePercent * AttackDamageMultiplier * EnemyLevel;
		}
		else
		{
			MaxHealth *= DifficultyInfo.HealthPercent;
			CurrentHealth = MaxHealth;
			AttackDamage *= DifficultyInfo.DamagePercent;
		}
		
	}
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
	DOREPLIFETIME(UEnemyStats, AttackDamage);
	DOREPLIFETIME(UEnemyStats, HealthMultiplier);
	DOREPLIFETIME(UEnemyStats, AttackDamageMultiplier);
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


