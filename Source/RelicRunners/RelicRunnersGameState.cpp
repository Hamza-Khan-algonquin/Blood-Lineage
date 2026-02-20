// Fill out your copyright notice in the Description page of Project Settings.

#include "RelicRunnersGameState.h"
#include <Net/UnrealNetwork.h>

ARelicRunnersGameState::ARelicRunnersGameState()
{
    bReplicates = true;
	DifficultyModifiers.Add("Boar", {1, 1, 0});
    DifficultyModifiers.Add("Bull", { 1.25f, 1.25f, 1 });
    DifficultyModifiers.Add("Lion", { 1.5f, 1.5f, 2 });
    DifficultyModifiers.Add("Hydra", { 1.75f, 1.75f, 3 });
    DifficultyModifiers.Add("Cerberus", { 2.5f, 2.5f, 5 });
}

FDifficultyInfo ARelicRunnersGameState::GetDifficultyInfo(FName DifficultyName) const
{
    if (DifficultyModifiers.Contains(DifficultyName))
    {
        return DifficultyModifiers[DifficultyName];
    }
    return { 1, 1, 0 };   
}

void ARelicRunnersGameState::OnRep_CurrentDifficulty()
{

}

void ARelicRunnersGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ARelicRunnersGameState, CurrentDifficulty);
}