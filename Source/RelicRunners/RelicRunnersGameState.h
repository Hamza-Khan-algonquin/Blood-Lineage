// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "RelicRunnersGameState.generated.h"

/**
 * 
 */

USTRUCT()
struct FDifficultyInfo
{
	GENERATED_BODY()
	float HealthPercent;

	float DamagePercent;

	float LuckAmount;

	float MetaXPIncrease;
};


UCLASS()
class RELICRUNNERS_API ARelicRunnersGameState : public AGameStateBase
{
	GENERATED_BODY()

public:

	ARelicRunnersGameState();

	UPROPERTY(ReplicatedUsing = OnRep_CurrentDifficulty, BlueprintReadWrite)
	FName CurrentDifficulty;

	//UPROPERTY(BlueprintReadWrite)
	TMap<FName, FDifficultyInfo> DifficultyModifiers;

	FDifficultyInfo GetDifficultyInfo(FName DifficultyName) const;
	FDifficultyInfo GetCurrentDifficultyInfo() const { return GetDifficultyInfo(CurrentDifficulty); }

	UFUNCTION()
	virtual void OnRep_CurrentDifficulty();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
