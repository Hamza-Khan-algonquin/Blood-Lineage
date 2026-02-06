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