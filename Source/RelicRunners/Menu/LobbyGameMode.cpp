// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyGameMode.h"
#include "RelicRunners/PlayerPreview/LobbyPreview.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "RelicRunners/PlayerController/RelicRunnersPlayerController.h"
#include "RelicRunners/Menu/JoinUserWidget.h"
#include "GameFramework/HUD.h"
#include "OnlineSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "RelicRunners/PlayerState/RelicRunnersPlayerState.h"
#include <OnlineSessionSettings.h>
#include <RelicRunners/Game/RelicRunnersGameInstance.h>
#include "RelicRunners/PlayerHUD/LobbyHUDWorld.h"
#include "Components/WidgetComponent.h"
#include <RelicRunners/Game/LobbyGameState.h>
#include "RelicRunners/RelicRunnersGameState.h"

ALobbyGameMode::ALobbyGameMode()
{
    LobbySpawnPositions = {
        FVector(-2230,230,270),
        FVector(-2300,100,280),
        FVector(-2300,360,280),
        FVector(-2370,-30,290)
    };

    PlayerControllerClass = ARelicRunnersPlayerController::StaticClass();

    bUseSeamlessTravel = true;
    bReplicates = true;
}

void ALobbyGameMode::BeginPlay()
{
    Super::BeginPlay();
}

void ALobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
    Super::PostLogin(NewPlayer);
    if (!HasAuthority() || !NewPlayer || !LobbyPreviewClass) return;

    PlayersInLobby.Add(NewPlayer);

    int Index = PlayersInLobby.Num() - 1;
    if (!LobbySpawnPositions.IsValidIndex(Index)) return;

    ARelicRunnersPlayerState* RPS = NewPlayer->GetPlayerState<ARelicRunnersPlayerState>();
    if (!RPS) return;

    RPS->SetIsHost(Index == 0);

    FActorSpawnParameters Params;
    Params.Owner = NewPlayer;
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    ALobbyPreview* SpawnedPreview = GetWorld()->SpawnActor<ALobbyPreview>(
        LobbyPreviewClass,
        LobbySpawnPositions[Index],
        FRotator::ZeroRotator,
        Params
    );

    if (!SpawnedPreview) return;

    SpawnedPreview->SetupFromPlayerState(RPS);
    LobbyPreviews.Add(RPS, SpawnedPreview);

    if (ARelicRunnersPlayerController* PC = Cast<ARelicRunnersPlayerController>(NewPlayer))
    {
        PC->Client_SetupLobby();
    }

    UpdateSetup();
}

void ALobbyGameMode::Logout(AController* Exiting)
{
    Super::Logout(Exiting);

    APlayerController* ExitingPC = Cast<APlayerController>(Exiting);
    if (!ExitingPC) return;

    PlayersInLobby.Remove(ExitingPC);

    ARelicRunnersPlayerState* RPS = ExitingPC->GetPlayerState<ARelicRunnersPlayerState>();
    if (!RPS) return;

    //Remove their preview
    ALobbyPreview* Preview = nullptr;
    if (LobbyPreviews.RemoveAndCopyValue(RPS, Preview))
    {
        if (HasAuthority() && Preview)
        {
            Preview->Destroy();
        }
    }

    UpdateSetup();
}

void ALobbyGameMode::OnPlayerStateUpdated(ARelicRunnersPlayerState* RPS)
{
    if (!RPS) return;

    if (ALobbyPreview* Preview = LobbyPreviews.FindRef(RPS))
    {
        Preview->SetupFromPlayerState(RPS);
    }

    UpdateSetup();
}

void ALobbyGameMode::UpdateSetup()
{
    for (APlayerController* PC : PlayersInLobby)
    {
        if (ARelicRunnersPlayerController* RRPC = Cast<ARelicRunnersPlayerController>(PC))
        {
            if (ARelicRunnersPlayerState* RPS = RRPC->GetPlayerState<ARelicRunnersPlayerState>())
            {
                RPS->OnSelectedClassChanged.Broadcast(RPS->SelectedClass);
            }
        }
    }
}

void ALobbyGameMode::StartCountdown() 
{
    CurrentCountdown = 5;
    GetWorldTimerManager().SetTimer(CountdownTimerHandle, this, &ALobbyGameMode::AdvanceTimer, 1.0f, true);
}

void ALobbyGameMode::AdvanceTimer()
{
    for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
    {
        if (ARelicRunnersPlayerController* PC = Cast<ARelicRunnersPlayerController>(It->Get()))
        {
            PC->Client_UpdateCountdown(CurrentCountdown);
        }
    }

    if (CurrentCountdown <= 0)
    {
        GetWorldTimerManager().ClearTimer(CountdownTimerHandle);

        if (URelicRunnersGameInstance* GI = GetGameInstance<URelicRunnersGameInstance>())
        {
            GI->StartSessionGame();
        }
        return; 
    }

    CurrentCountdown--;
}

void ALobbyGameMode::SetDifficulty(FName NewDifficulty)
{
    if (URelicRunnersGameInstance* GI = GetGameInstance<URelicRunnersGameInstance>())
    {
        GI->CurrentDifficulty = NewDifficulty;
    }

    if (ARelicRunnersGameState* GS = GetGameState<ARelicRunnersGameState>())
    {
        GS->CurrentDifficulty = NewDifficulty;
    }

    for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
    {
        if (ARelicRunnersPlayerController* PC = Cast<ARelicRunnersPlayerController>(It->Get()))
        {
            PC->Client_UpdateDifficulty(NewDifficulty);
        }
    }
}