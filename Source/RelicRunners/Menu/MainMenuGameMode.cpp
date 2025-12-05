#include "MainMenuGameMode.h"
#include "MainMenuWidget.h" 
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "JoinUserWidget.h"
#include <RelicRunners/PlayerController/RelicRunnersPlayerController.h>
#include "RelicRunners/PlayerPreview/LobbyPreview.h"

AMainMenuGameMode::AMainMenuGameMode()
{
    DefaultPawnClass = nullptr;
    PlayerControllerClass = ARelicRunnersPlayerController::StaticClass();

    bUseSeamlessTravel = true;
    bReplicates = false;
}

void AMainMenuGameMode::BeginPlay()
{
    Super::BeginPlay();
}