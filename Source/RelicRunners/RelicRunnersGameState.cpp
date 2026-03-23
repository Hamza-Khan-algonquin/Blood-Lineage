// Fill out your copyright notice in the Description page of Project Settings.

#include "RelicRunnersGameState.h"
#include <Net/UnrealNetwork.h>
#include "PlayerController/RelicRunnersPlayerController.h"

ARelicRunnersGameState::ARelicRunnersGameState()
{
    bReplicates = true;
	DifficultyModifiers.Add("Boar", {1, 1, 0, 1});
    DifficultyModifiers.Add("Bull", { 1.25f, 1.25f, 1, 2 });
    DifficultyModifiers.Add("Lion", { 1.5f, 1.5f, 2, 3 });
    DifficultyModifiers.Add("Hydra", { 1.75f, 1.75f, 3, 5 });
    DifficultyModifiers.Add("Cerberus", { 2.5f, 2.5f, 5, 10 });
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
    if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
    {
        if (ARelicRunnersPlayerController* RPC = Cast<ARelicRunnersPlayerController>(PC))
        {
            RPC->Client_UpdateDifficulty(CurrentDifficulty);
        }
    }
}

void ARelicRunnersGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ARelicRunnersGameState, CurrentDifficulty);
}