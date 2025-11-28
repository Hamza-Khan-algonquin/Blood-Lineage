// Fill out your copyright notice in the Description page of Project Settings.

#include "RelicRunnersGameState.h"
#include <Net/UnrealNetwork.h>

ARelicRunnersGameState::ARelicRunnersGameState()
{
    bReplicates = true;
}

bool ARelicRunnersGameState::InitializeTriggerState()
{
    if (ObjectiveType == 0)
    {
        return true;
    }
    return false;
}

void ARelicRunnersGameState::InformCurrentObjectives()
{
    if (EnumHasAnyFlags(static_cast<EObjectiveType>(ObjectiveType), EObjectiveType::CaptureTheFlag))
    {
        GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Red, FString::Printf(TEXT("Remaining flags: %i"), RemainingCapturableFlags));
    }
    if (EnumHasAnyFlags(static_cast<EObjectiveType>(ObjectiveType), EObjectiveType::DefeatAllEnemies))
    {
        GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Red, FString::Printf(TEXT("Remaining enemy zones: %i"), RemainingEnemyZones));
    }
    if (EnumHasAnyFlags(static_cast<EObjectiveType>(ObjectiveType), EObjectiveType::MoveThePayload))
    {
        GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Red, FString::Printf(TEXT("Remaining Payloads: %i"), RemainingPayloads));
    }
    if (EnumHasAnyFlags(static_cast<EObjectiveType>(ObjectiveType), EObjectiveType::Survival))
    {
        GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Red, FString::Printf(TEXT("Remaining Survival zones: %i"), RemainingSurvivalZones));
    }
}

void ARelicRunnersGameState::Server_SetObjectiveType_Implementation(int newType)
{
    if (HasAuthority())
    {
        ObjectiveType = newType;

        if (ObjectiveType == 0)
        {
            Server_DecrementObjective(EObjectiveType::None);
        }
    }
}

void ARelicRunnersGameState::Server_IncrementObjective_Implementation(EObjectiveType objectiveType)
{
    if (HasAuthority()) {
        switch (objectiveType)
        {
        case EObjectiveType::CaptureTheFlag:
            RemainingCapturableFlags++;
            GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Yellow, FString::Printf(TEXT("Max flags: %i"), RemainingCapturableFlags));
            break;
        case EObjectiveType::DefeatAllEnemies:
            RemainingEnemyZones++;
            GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Yellow, FString::Printf(TEXT("Max enemy zones: %i"), RemainingEnemyZones));
            break;
        case EObjectiveType::MoveThePayload:
            RemainingPayloads++;
            GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Yellow, FString::Printf(TEXT("Max Payloads: %i"), RemainingPayloads));
            break;
        case EObjectiveType::Survival:
            RemainingSurvivalZones++;
            GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Yellow, FString::Printf(TEXT("Max Survival Zones: %i"), RemainingSurvivalZones));
            break;
        }
    }
}

void ARelicRunnersGameState::Server_DecrementObjective_Implementation(EObjectiveType objectiveType)
{
    switch (objectiveType)
    {
    case EObjectiveType::CaptureTheFlag:
        RemainingCapturableFlags--;

        GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Orange, FString::Printf(TEXT("Remaining flags: %i"), RemainingCapturableFlags));
        if (RemainingCapturableFlags <= 0)
        {
            CompletedObjectives |= static_cast<uint8>(EObjectiveType::CaptureTheFlag);
            GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Green, FString::Printf(TEXT("Flag Objective has been reached!")));
        }
        break;
    case EObjectiveType::DefeatAllEnemies:
        RemainingEnemyZones--;

        GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Orange, FString::Printf(TEXT("Remaining enemy zones: %i"), RemainingEnemyZones));
        if (RemainingEnemyZones <= 0)
        {
            CompletedObjectives |= static_cast<uint8>(EObjectiveType::DefeatAllEnemies);
            GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Green, FString::Printf(TEXT("Enemy Objective has been reached!")));
        }
        break;
    case EObjectiveType::MoveThePayload:
        RemainingPayloads--;

        GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Orange, FString::Printf(TEXT("Remaining payloads: %i"), RemainingPayloads));
        if (RemainingPayloads <= 0)
        {
            CompletedObjectives |= static_cast<uint8>(EObjectiveType::MoveThePayload);
            GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Green, FString::Printf(TEXT("Payload Delivery has been reached!")));
        }
        break;
    case EObjectiveType::Survival:
        RemainingSurvivalZones--;

        GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Orange, FString::Printf(TEXT("Remaining survival zones: %i"), RemainingSurvivalZones));
        if (RemainingSurvivalZones <= 0)
        {
            CompletedObjectives |= static_cast<uint8>(EObjectiveType::Survival);
            GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Green, FString::Printf(TEXT("Survival has been reached!")));
        }
        break;
    }

    if (CompletedObjectives == ObjectiveType)
    {
        OnObjectiveActionCompleted.Broadcast();
    }
}

void ARelicRunnersGameState::OnRep_FlagObjectivesChange()
{
    GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Orange, FString::Printf(TEXT("OnRep - Remaining flags: %i"), RemainingCapturableFlags));

    if (RemainingCapturableFlags <= 0)
    {
        GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Green, FString::Printf(TEXT("OnRep - Flag Objective has been reached!")));
    }
}

void ARelicRunnersGameState::OnRep_EnemyObjectivesChange()
{
    GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Orange, FString::Printf(TEXT("OnRep - Remaining enemy zones: %i"), RemainingEnemyZones));

    if (RemainingEnemyZones <= 0)
    {
        GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Green, FString::Printf(TEXT("OnRep - Enemy Objective has been reached!")));
    }
}

void ARelicRunnersGameState::OnRep_PayloadObjectivesChange()
{
    GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Orange, FString::Printf(TEXT("OnRep - Remaining Payloads: %i"), RemainingPayloads));

    if (RemainingPayloads <= 0)
    {
        GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Green, FString::Printf(TEXT("OnRep - Payload Objective has been reached!")));
    }
}

void ARelicRunnersGameState::OnRep_SurvivalObjectivesChange()
{
    GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Orange, FString::Printf(TEXT("OnRep - Remaining Survival Zones: %i"), RemainingSurvivalZones));

    if (RemainingSurvivalZones <= 0)
    {
        GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Green, FString::Printf(TEXT("OnRep - Survival Objective has been reached!")));
    }
}

void ARelicRunnersGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ARelicRunnersGameState, RemainingCapturableFlags);
    DOREPLIFETIME(ARelicRunnersGameState, RemainingEnemyZones);
    DOREPLIFETIME(ARelicRunnersGameState, RemainingPayloads);
    DOREPLIFETIME(ARelicRunnersGameState, RemainingSurvivalZones);
    DOREPLIFETIME(ARelicRunnersGameState, ObjectiveType);
    DOREPLIFETIME(ARelicRunnersGameState, CompletedObjectives);
}