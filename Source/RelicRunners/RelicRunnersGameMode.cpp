/************************************************************************************
 * Author:   Tristan Anglin
 * Year:     2025
 *
 * Notice:
 *   This source code is the intellectual property of Tristan Anglin.
 *   It is provided solely for use within:
 *     - The Relic Runners personal project
 *     - The Almost A Dozen project
 *
 *   Any use, distribution, or modification outside of these projects
 *   is strictly prohibited without explicit written permission.
 *
 *   � 2025 Tristan Anglin. All rights reserved.
 ************************************************************************************/

#include "RelicRunnersGameMode.h"
#include "RelicRunnersCharacter.h"
#include "PlayerState/RelicRunnersPlayerState.h"
#include "UObject/ConstructorHelpers.h"
#include "GameFramework/PlayerStart.h"
#include "GameFramework/GameStateBase.h"
#include "Kismet/GameplayStatics.h"
#include "Characters/AphroditeCharacter.h"
#include "Characters/ArtemisCharacter.h"
#include "Characters/AresCharacter.h"
#include "Characters/NemesisCharacter.h"
#include "GameFramework/PlayerController.h"
#include "PlayerController/RelicRunnersPlayerController.h"
#include "Game/RelicRunnersGameInstance.h"
#include "Game/PostGameWidget.h"
#include "RelicRunnersGameState.h"

ARelicRunnersGameMode::ARelicRunnersGameMode()
{
    bUseSeamlessTravel = true;
    bReplicates = true;
}

void ARelicRunnersGameMode::PostLogin(APlayerController* NewPlayer)
{
    Super::PostLogin(NewPlayer);
}

void ARelicRunnersGameMode::HandleSeamlessTravelPlayer(AController*& C)
{
    Super::HandleSeamlessTravelPlayer(C);
}

void ARelicRunnersGameMode::PostSeamlessTravel()
{
    Super::PostSeamlessTravel();

    for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; It++)
    {
        ARelicRunnersPlayerController* RPC = Cast<ARelicRunnersPlayerController>(It->Get());
        if (!RPC) continue;

        if (!RPC->GetPawn())
        {
            RestartPlayer(RPC);
        }
    }
}

APawn* ARelicRunnersGameMode::SpawnDefaultPawnFor_Implementation(AController* NewPlayer, AActor* StartSpot)
{
    if (!NewPlayer) return nullptr;

    ARelicRunnersPlayerState* PS = NewPlayer->GetPlayerState<ARelicRunnersPlayerState>();
    if (!PS)
    {
        return Super::SpawnDefaultPawnFor_Implementation(NewPlayer, StartSpot);
    }

    TSubclassOf<APawn> PawnClassToSpawn = nullptr;

    if (PS->SelectedClass == "Nemesis")         PawnClassToSpawn = NemesisCharacterClass;
    else if (PS->SelectedClass == "Aphrodite")  PawnClassToSpawn = AphroditeCharacterClass;
    else if (PS->SelectedClass == "Artemis")    PawnClassToSpawn = ArtemisCharacterClass;
    else if (PS->SelectedClass == "Ares")       PawnClassToSpawn = AresCharacterClass;
    else                                        PawnClassToSpawn = AresCharacterClass;

    if (!PawnClassToSpawn)
    {
        return Super::SpawnDefaultPawnFor_Implementation(NewPlayer, StartSpot);
    }

    FActorSpawnParameters Params;
    Params.Owner = NewPlayer;
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    return GetWorld()->SpawnActor<APawn>(PawnClassToSpawn, StartSpot->GetActorLocation(), StartSpot->GetActorRotation(), Params);
}

void ARelicRunnersGameMode::CheckGameOver()
{
    bool AllPlayersDead = true;

    for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
    {
        ARelicRunnersPlayerController* RPC = Cast<ARelicRunnersPlayerController>(It->Get());
        if (RPC)
        {
            ARelicRunnersPlayerState* PS = RPC->GetPlayerState<ARelicRunnersPlayerState>();
            if (PS && !PS->DeathState)
            {
                AllPlayersDead = false;
                break;
            }
        }
    }

    if (AllPlayersDead)
    {
        FTimerHandle PostGameHandle;
        GetWorldTimerManager().SetTimer(PostGameHandle, this, &ARelicRunnersGameMode::TriggerPostGameForAll, 0.5f, false);
    }
}

void ARelicRunnersGameMode::TriggerPostGameForAll()
{
    TArray<FPlayerPostGameData> AllPlayersStats;

    for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
    {
        if (ARelicRunnersPlayerController* RPC = Cast<ARelicRunnersPlayerController>(It->Get()))
        {
            if (ARelicRunnersPlayerState* PS = RPC->GetPlayerState<ARelicRunnersPlayerState>())
            {
                FPlayerPostGameData Stats;
                Stats.Username = PS->GetPlayerName();
                Stats.LevelsEarned = PS->GetLevelsEarned();
                Stats.EnemiesKilled = PS->GetEnemiesKilled();
                Stats.Deaths = PS->GetDeaths();
                Stats.MetaLevel = PS->MetaLevel;
                Stats.MetaXP = PS->MetaXP;
                Stats.MetaXPToNextLevel = PS->MetaXPToNextLevel;
                Stats.MetaXPGainInRun = PS->MetaXPGainInRun;
                AllPlayersStats.Add(Stats);
            }
        }
    }

    for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
    {
        if (ARelicRunnersPlayerController* RPC = Cast<ARelicRunnersPlayerController>(It->Get()))
        {
            RPC->Client_ShowPostGame(AllPlayersStats);
        }
    }
}

void ARelicRunnersGameMode::PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage)
{
    Super::PreLogin(Options, Address, UniqueId, ErrorMessage);

    URelicRunnersGameInstance* GI = Cast<URelicRunnersGameInstance>(GetGameInstance());

    if (GI && GI->bIsMatchLocked)
    {
        ErrorMessage = TEXT("Match_Already_In_Progress");
    }
}

void ARelicRunnersGameMode::RecordPawnReadyTime(APlayerController* PC)
{
    if (!PC) return;

    ControllerPawnReadyTimes.Add(PC, GetWorld()->GetTimeSeconds());

    if (ARelicRunnersGameState* GS = GetWorld()->GetGameState<ARelicRunnersGameState>())
    {
        int TotalPlayers = GS->PlayerArray.Num();
        int ReadyPlayers = ControllerPawnReadyTimes.Num();

        UE_LOG(LogTemp, Log, TEXT("Progress: %d/%d players ready."), ReadyPlayers, TotalPlayers);

        if (ReadyPlayers >= TotalPlayers && TotalPlayers > 0)
        {
            OnAllPawnsSpawned();
        }
    }
}

float ARelicRunnersGameMode::GetTimeSinceLastPawnSetup() const
{
    if (ControllerPawnReadyTimes.Num() == 0) return -1.0f;

    float LatestTime = 0.0f;
    for (auto& It : ControllerPawnReadyTimes)
    {
        if (It.Value > LatestTime)
        {
            LatestTime = It.Value;
        }
    }

    return GetWorld()->GetTimeSeconds() - LatestTime;
}

void ARelicRunnersGameMode::OnAllPawnsSpawned()
{
    UE_LOG(LogTemp, Warning, TEXT("All Pawns Acknowledged! Starting game logic..."));
}