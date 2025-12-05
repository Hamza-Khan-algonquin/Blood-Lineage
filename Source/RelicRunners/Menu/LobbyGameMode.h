// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "LobbyGameMode.generated.h"

/**
 * 
 */
UCLASS()
class RELICRUNNERS_API ALobbyGameMode : public AGameMode
{
	GENERATED_BODY()
public:
    ALobbyGameMode();
    virtual void PostLogin(APlayerController* NewPlayer) override;
    virtual void Logout(AController* Exiting) override;
    void OnPlayerStateUpdated(class ARelicRunnersPlayerState* RPS);
    virtual void BeginPlay() override;
protected:
    // Where to put players in lobby (pillars etc.)
    UPROPERTY(EditDefaultsOnly, Category = "Lobby")
    TArray<FVector> LobbySpawnPositions;

    UPROPERTY(EditDefaultsOnly, Category = "Lobby")
    TSubclassOf<class ALobbyPreview> LobbyPreviewClass;

private:
    // Track controllers in the lobby
    TArray<APlayerController*> PlayersInLobby;

    UPROPERTY()
    TMap<ARelicRunnersPlayerState*, ALobbyPreview*> LobbyPreviews;

    void UpdateSetup();
};
