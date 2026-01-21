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

ALobbyGameMode::ALobbyGameMode()
{
    LobbySpawnPositions = {
        FVector(-2235,110,330),
        FVector(-2235,230,330),
        FVector(-2319,340,340),
        FVector(-2320,0,340)
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
