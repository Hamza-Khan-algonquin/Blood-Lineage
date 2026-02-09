// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
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

	UPROPERTY(ReplicatedUsing = OnRep_CurrentDifficulty, BlueprintReadWrite)
	FName CurrentDifficulty;

	UFUNCTION()
	virtual void OnRep_CurrentDifficulty();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
