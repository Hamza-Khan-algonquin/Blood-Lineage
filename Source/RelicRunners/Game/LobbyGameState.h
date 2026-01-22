// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "RelicRunners/RelicRunnersGameState.h"
#include "LobbyGameState.generated.h"

/**
 * 
 */
UCLASS()
class RELICRUNNERS_API ALobbyGameState : public ARelicRunnersGameState
{
	GENERATED_BODY()
public:

	void SetDifficulty(FName NewDifficulty);

	virtual void OnRep_CurrentDifficulty() override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
