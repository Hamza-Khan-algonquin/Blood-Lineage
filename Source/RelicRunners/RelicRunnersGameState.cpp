// Fill out your copyright notice in the Description page of Project Settings.

#include "RelicRunnersGameState.h"
#include <Net/UnrealNetwork.h>
#include "PlayerController/RelicRunnersPlayerController.h"

ARelicRunnersGameState::ARelicRunnersGameState()
{
    bReplicates = true;             //      HP      DMG     LUCK    M-XP    -LOOT   BOSS-DMG
	DifficultyModifiers.Add("Boar", {       1,      1,      0,      1,      0,      10});
    DifficultyModifiers.Add("Bull", {       1.5f,   2.f,    1,      2,      2,      25 });
    DifficultyModifiers.Add("Lion", {       2.f,    3.f,    2,      3,      3,      40 });
    DifficultyModifiers.Add("Hydra", {      3.f,    4.f,    3,      5,      5,      60 });
    DifficultyModifiers.Add("Cerberus", {   4.f,    6.f,    5,      10,     8,      80 });
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