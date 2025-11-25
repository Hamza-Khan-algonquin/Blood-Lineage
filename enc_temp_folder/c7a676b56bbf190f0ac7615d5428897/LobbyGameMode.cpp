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

    if (LobbySpawnPositions.IsValidIndex(Index))
    {
        FActorSpawnParameters Params;
        Params.Owner = NewPlayer;
        Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

        ALobbyPreview* Preview = GetWorld()->SpawnActor<ALobbyPreview>(LobbyPreviewClass,LobbySpawnPositions[Index],FRotator::ZeroRotator,Params);

        if (Preview)
        {
            LobbyPreviews.Add(NewPlayer, Preview);

            if (ARelicRunnersPlayerState* PS = NewPlayer->GetPlayerState<ARelicRunnersPlayerState>())
            {
                FTimerHandle T;
                GetWorld()->GetTimerManager().SetTimer(T, FTimerDelegate::CreateLambda([Preview, PS]()
                {
                    Preview->SetupFromPlayerState(PS);
                }), 0.1f, false);
            }
        }
    }

    if (ARelicRunnersPlayerController* PC = Cast<ARelicRunnersPlayerController>(NewPlayer))
    {
        PC->Client_SendPlayerInfo();
        PC->Client_SetupLobby();
    }

    UpdateSetup();
}

void ALobbyGameMode::Logout(AController* Exiting)
{
    Super::Logout(Exiting);

    APlayerController* ExitingPC = Cast<APlayerController>(Exiting);
    if (!ExitingPC) return;

    // Remove from lobby array
    PlayersInLobby.Remove(ExitingPC);

    // Destroy their preview actor if exists
    ALobbyPreview* Preview = nullptr;
    if (LobbyPreviews.RemoveAndCopyValue(ExitingPC, Preview))
    {
        if (HasAuthority() && Preview)
        {
            Preview->Destroy();
        }
    }
    UpdateSetup();
}

void ALobbyGameMode::UpdateSetup()
{
    for (APlayerController* PC : PlayersInLobby)
    {
        if (ARelicRunnersPlayerController* RRPC = Cast<ARelicRunnersPlayerController>(PC))
        {
            RRPC->Client_UpdateLobbyUI();
        }
    }
}
