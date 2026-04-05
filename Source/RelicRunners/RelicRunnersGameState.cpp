// Fill out your copyright notice in the Description page of Project Settings.

#include "RelicRunnersGameState.h"
#include <Net/UnrealNetwork.h>
#include "PlayerController/RelicRunnersPlayerController.h"

ARelicRunnersGameState::ARelicRunnersGameState()
{
    bReplicates = true;

    // Structure: { HP_Mult, DMG_Mult, LUCK, XP_Mult, LOOT_Bonus, BOSS_DMG, TIER }
    DifficultyModifiers.Add("Boar", { 0.8f,  0.5f,  0,  1.0f,  0,  20, 1 });

    DifficultyModifiers.Add("Bull", { 1.25f,  1.f,  1,  1.75f,  0,  40, 2 });

    DifficultyModifiers.Add("Lion", { 2.5f,  2.0f,  1.5,  3.0f,  0, 50, 3 });

    DifficultyModifiers.Add("Hydra", { 4.25f,  3.25f,  2.8,  4.25f,  0, 70, 4 });

    DifficultyModifiers.Add("Cerberus", { 7.f,  5.25f,  4.5,  6.5f,  0, 90, 5 });
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