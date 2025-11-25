// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "RelicRunnersGameState.generated.h"

/**
 * 
 */

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnObjectiveActionCompleted);

UENUM(BlueprintType, meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = "true"))
enum class EObjectiveType : uint8
{
	None = 0 UMETA(Hidden),
	CaptureTheFlag = 1 << 0,
	DefeatAllEnemies = 1 << 1,
	MoveThePayload = 1 << 2,
	Survival = 1 << 3,
};
ENUM_CLASS_FLAGS(EObjectiveType);

inline bool operator== (int num, EObjectiveType type)
{
	return num == static_cast<uint8>(type);
}

inline bool operator|= (int num, EObjectiveType type)
{
	return num |= static_cast<uint8>(type);
}


UCLASS()
class RELICRUNNERS_API ARelicRunnersGameState : public AGameStateBase
{
	GENERATED_BODY()

public:

	ARelicRunnersGameState();

	UFUNCTION(Server, Reliable)
	void Server_IncrementObjective(EObjectiveType objectiveType);

	UFUNCTION(Server, Reliable)
	void Server_DecrementObjective(EObjectiveType objectiveType);

	UFUNCTION(Server, Reliable)
	void Server_SetObjectiveType(int newType);

	bool InitializeTriggerState();

	void InformCurrentObjectives();

	UPROPERTY(ReplicatedUsing = OnRep_FlagObjectivesChange)
	int RemainingCapturableFlags = 0;

	UFUNCTION()
	void OnRep_FlagObjectivesChange();

	UPROPERTY(ReplicatedUsing = OnRep_EnemyObjectivesChange)
	int RemainingEnemyZones= 0;

	UFUNCTION()
	void OnRep_EnemyObjectivesChange();

	bool PayloadHadBeenDelivered = false;

	UPROPERTY(BlueprintAssignable)
	FOnObjectiveActionCompleted OnObjectiveActionCompleted;

	UPROPERTY(Replicated, meta = (Bitmask, BitmaskEnum = EObjectiveType))
	int ObjectiveType = 0;

	UPROPERTY(Replicated, meta = (Bitmask, BitmaskEnum = EObjectiveType))
	int CompletedObjectives = 0;
};
