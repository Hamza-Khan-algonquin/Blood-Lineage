// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyGameState.h"
#include <Net/UnrealNetwork.h>
#include "RelicRunnersGameInstance.h"
#include <Blueprint/WidgetBlueprintLibrary.h>
#include "RelicRunners/Menu/JoinUserWidget.h"

void ALobbyGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

void ALobbyGameState::SetDifficulty(FName NewDifficulty)
{
    CurrentDifficulty = NewDifficulty;

    OnRep_CurrentDifficulty();
}

void ALobbyGameState::OnRep_CurrentDifficulty()
{
    if (URelicRunnersGameInstance* GI = Cast<URelicRunnersGameInstance>(GetGameInstance()))
    {
        GI->CurrentDifficulty = CurrentDifficulty;
    }

    if (GetWorld() && !GetWorld()->IsPreparingMapChange())
    {
        TArray<UUserWidget*> FoundWidgets;
        UWidgetBlueprintLibrary::GetAllWidgetsOfClass(GetWorld(), FoundWidgets, UJoinUserWidget::StaticClass());
        for (UUserWidget* Widget : FoundWidgets)
        {
            if (UJoinUserWidget* LobbyWidget = Cast<UJoinUserWidget>(Widget))
            {
                LobbyWidget->UpdateClientDifficultyUI();
            }
        }
    }
}