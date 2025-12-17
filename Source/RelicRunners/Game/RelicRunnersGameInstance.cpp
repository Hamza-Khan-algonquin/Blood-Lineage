#include "RelicRunnersGameInstance.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include <Online/OnlineSessionNames.h>
#include "Kismet/GameplayStatics.h"
#include "OnlineSessionSettings.h"
#include "RelicRunners/Menu/JoinUserWidget.h"
#include "RelicRunners/Menu/MainMenuWidget.h"
#include <RelicRunners/Menu/MainMenuGameMode.h>
#include <RelicRunners/PlayerController/RelicRunnersPlayerController.h>
#include "RelicRunners/PlayerPreview/LobbyPreview.h"
#include "GameFramework/GameStateBase.h"
#include "Engine/Engine.h"
#include "RelicRunners/Menu/Keybinds.h"
#include <RelicRunners/Menu/LobbyGameMode.h>
#include <RelicRunners/RelicRunnersGameMode.h>
#include <InputMappingContext.h>
#include <EnhancedInputSubsystems.h>
#include "RelicRunners/Menu/SaveFileListData.h"
#include "RelicRunners/Menu/SaveFilePopupWidget.h"
#include "RelicRunners/Menu/EditSaveFilePopupWidget.h"
#include "RelicRunners/Menu/ModeSelectionWidget.h"
#include <RelicRunners/Menu/KeybindingsListData.h>

void URelicRunnersGameInstance::Init()
{
    Super::Init();

    //Setup online systems
    IOnlineSessionPtr OnlineSubsystem = Online::GetSessionInterface(GetWorld());
    if (OnlineSubsystem)
    {
        SessionInterface = OnlineSubsystem;
    }

    //Keybinds
    Keys = NewObject<UKeybinds>(this);
    DefaultKeys = NewObject<UKeybinds>(this);

    //Savefiles
    InitializeSaveSlots();
}

void URelicRunnersGameInstance::ResetToDefaults()
{
    Keys = DuplicateObject<UKeybinds>(DefaultKeys, this);
    ApplyKeybindings();
}

void URelicRunnersGameInstance::ApplyKeybindings()
{
    APlayerController* PC = GetFirstLocalPlayerController();
    if (!PC) return;

    ARelicRunnersPlayerController* RPC = Cast<ARelicRunnersPlayerController>(PC);
    if (!RPC) return;

    UInputMappingContext* OldMappingContext = RPC->GetInputMappingContext();
    if (!OldMappingContext)
    {
        UE_LOG(LogTemp, Warning, TEXT("ApplyKeybindings: No InputMappingContext set!"));
        return;
    }

    UInputMappingContext* MappingContext = DuplicateObject<UInputMappingContext>(OldMappingContext, this);

    UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer());
    if (!Subsystem) return;

    Subsystem->RemoveMappingContext(OldMappingContext);
    MappingContext->UnmapAll();

    UInputAction* LookAction = LoadObject<UInputAction>(nullptr, TEXT("/Game/ThirdPerson/Input/Actions/IA_Look.IA_Look"));
    if (LookAction)
    {
        FEnhancedActionKeyMapping& Mapping = MappingContext->MapKey(LookAction, EKeys::Mouse2D);

        UInputModifierNegate* NegateModifier = NewObject<UInputModifierNegate>(MappingContext);
        NegateModifier->bX = Keys->InvertedXMouse;
        NegateModifier->bY = Keys->InvertedYMouse;

        Mapping.Modifiers.Add(NegateModifier);
    }

    for (auto& Binding : Keys->KeyBinds)
    {
        FString ActionAssetName = FString::Printf(TEXT("IA_%s"), *Binding.Name.Replace(TEXT(" "), TEXT("")));
        FString AssetPath = FString::Printf(TEXT("/Game/ThirdPerson/Input/Actions/%s.%s"), *ActionAssetName, *ActionAssetName);

        UInputAction* FoundAction = LoadObject<UInputAction>(nullptr, *AssetPath);
        if (!FoundAction)
        {
            UE_LOG(LogTemp, Warning, TEXT("ApplyKeybindings: couldn't load InputAction %s (path: %s)"), *ActionAssetName, *AssetPath);
            continue;
        }

        MappingContext->MapKey(FoundAction, Binding.Bind);


        UE_LOG(LogTemp, Log, TEXT("ApplyKeybindings: %s -> key %s"), *Binding.Name, *Binding.Bind.ToString());

        UE_LOG(LogTemp, Log, TEXT("ApplyKeybindings: %s -> %s"), *Binding.Name, *Binding.ReadableBind);

    }

    Subsystem->AddMappingContext(MappingContext, 0);
}

USaveFileListData* URelicRunnersGameInstance::CreateSaveFromData(const FSaveData& SaveData, UObject* Outer)
{
    if (!Outer)
    {
        UE_LOG(LogTemp, Error, TEXT("CreateItemFromData called with null Outer!"));
        return nullptr;
    }

    USaveFileListData* Save = NewObject<USaveFileListData>(Outer);
    Save->SetSaveData(SaveData);
    return Save;
}

void URelicRunnersGameInstance::InitializeSaveSlots()
{
    if (HasInitializedSlots) return; 
    HasInitializedSlots = true;

    //Create starting data
    USaveFileListData* FirstSlot = NewObject<USaveFileListData>();
    FirstSlot->IsEmptySlot = true;
    SaveSlots.Add(FirstSlot);
}

void URelicRunnersGameInstance::Shutdown()
{
    Super::Shutdown();

    LeaveSession();
}

void URelicRunnersGameInstance::BackToMainMenu()
{
    //Destroy session
    if (TSharedPtr<IOnlineSession> Sess = SessionInterface.Pin())
    {
        if (Sess->GetNamedSession(NAME_GameSession))
        {
            Sess->OnDestroySessionCompleteDelegates.RemoveAll(this);
            Sess->OnDestroySessionCompleteDelegates.AddUObject(this, &URelicRunnersGameInstance::OnDestroySessionComplete);
            Sess->DestroySession(NAME_GameSession);
            return;
        }
    }

    //Travel to main menu
    if (APlayerController* PC = GetFirstLocalPlayerController())
    {
        PC->ClientTravel(TEXT("/Game/ThirdPerson/Maps/MainMenu"), ETravelType::TRAVEL_Absolute);
    }
}

void URelicRunnersGameInstance::HostGame()
{
    if (!SessionInterface.IsValid()) return;

    if (TSharedPtr<IOnlineSession> Sess = SessionInterface.Pin())
    {
        //If a named session exists locally, destroy it first and retry in callback
        if (Sess->GetNamedSession(NAME_GameSession))
        {
            PendingHostAfterLeave = 1;
            Sess->OnDestroySessionCompleteDelegates.RemoveAll(this);
            Sess->OnDestroySessionCompleteDelegates.AddUObject(this, &URelicRunnersGameInstance::OnDestroySessionComplete);
            Sess->DestroySession(NAME_GameSession);
            return;
        }

        //If no existing session, create immediately
        CreateNewSession();
    }
}

void URelicRunnersGameInstance::CreateNewSession()
{
    if (!SessionInterface.IsValid()) return;

    if (TSharedPtr<IOnlineSession> Sess = SessionInterface.Pin())
    {
        //Setup parameters
        int LobbySize = 4;
        FOnlineSessionSettings SessionSettings;
        SessionSettings.NumPublicConnections = LobbySize;
        SessionSettings.bIsLANMatch = false;
        SessionSettings.bShouldAdvertise = true;
        SessionSettings.bUsesPresence = true;
        SessionSettings.bAllowInvites = true;
        SessionSettings.bUseLobbiesIfAvailable = true;
        SessionSettings.bAllowJoinViaPresence = true;
        SessionSettings.bAllowJoinInProgress = true;
        FString LobbyTitle = FString::Printf(TEXT("%s's Lobby"), *PlayerName);
        SessionSettings.Set(SETTING_MAPNAME, LobbyTitle, EOnlineDataAdvertisementType::ViaOnlineService);

        Sess->OnCreateSessionCompleteDelegates.RemoveAll(this);
        Sess->OnCreateSessionCompleteDelegates.AddUObject(this, &URelicRunnersGameInstance::OnCreateSessionComplete);
        Sess->CreateSession(0, NAME_GameSession, SessionSettings);
    }
}

void URelicRunnersGameInstance::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
    UE_LOG(LogTemp, Log, TEXT("OnCreateSessionComplete: %s success=%d"), *SessionName.ToString(), bWasSuccessful);
    if (bWasSuccessful)
    {
        UGameplayStatics::OpenLevel(this, TEXT("Lobby"), true, TEXT("listen"));
    }

    //Unbind delegate 
    if (TSharedPtr<IOnlineSession> Sess = SessionInterface.Pin())
    {
        Sess->OnCreateSessionCompleteDelegates.RemoveAll(this);
    }
}

void URelicRunnersGameInstance::FindGames(UJoinUserWidget* UserWidget)
{
    if (TSharedPtr<IOnlineSession> Session = SessionInterface.Pin())
    {
        //Search parameters
        int NumResults = 20;
        SessionSearch = MakeShareable(new FOnlineSessionSearch());
        SessionSearch->MaxSearchResults = NumResults;
        SessionSearch->bIsLanQuery = false;
        SessionSearch->QuerySettings.Set(FName("PRESENCESEARCH"), true, EOnlineComparisonOp::Equals);

        Session->OnFindSessionsCompleteDelegates.RemoveAll(this);
        Session->OnFindSessionsCompleteDelegates.AddUObject(this, &URelicRunnersGameInstance::OnFindSessionsComplete);
        Session->FindSessions(0, SessionSearch.ToSharedRef());

        TextRenderWidget = UserWidget;
    }
}

void URelicRunnersGameInstance::OnFindSessionsComplete(bool bWasSuccessful)
{
    if (!TextRenderWidget.IsValid())
        return;

    bool AddedAnyResults = false;

    if (bWasSuccessful && SessionSearch.IsValid())
    {
        TSharedPtr<IOnlineSession> Session = SessionInterface.Pin();
        if (!Session.IsValid()) return;

        FNamedOnlineSession* LocalSession = Session->GetNamedSession(NAME_GameSession);

        for (const FOnlineSessionSearchResult& R : SessionSearch->SearchResults)
        {
            //Skip the current session
            if (LocalSession && R.Session.GetSessionIdStr() == LocalSession->GetSessionIdStr()) continue;

            AddedAnyResults = true;

            FString MapName = R.Session.SessionSettings.Settings.Contains("MAPNAME")
                ? *R.Session.SessionSettings.Settings.Find("MAPNAME")->Data.ToString()
                : TEXT("Unknown Map");

            int MaxPlayers = R.Session.SessionSettings.NumPublicConnections;
            int CurrentPlayers = MaxPlayers - R.Session.NumOpenPublicConnections;

            TextRenderWidget->OnFindSessionsComplete(MapName, CurrentPlayers);
        }
    }

    //If nothing was added
    if (!AddedAnyResults)
    {
        TextRenderWidget->EnableRefresh();
    }

    //Unbind delegate
    if (TSharedPtr<IOnlineSession> Sess = SessionInterface.Pin())
    {
        Sess->OnFindSessionsCompleteDelegates.RemoveAll(this);
    }
}


void URelicRunnersGameInstance::LeaveSession(bool bQueueHost)
{
    if (!SessionInterface.IsValid()) return;

    if (TSharedPtr<IOnlineSession> Sess = SessionInterface.Pin())
    {
        if (Sess->GetNamedSession(NAME_GameSession))
        {
            if (bQueueHost)
            {
                PendingHostAfterLeave = 1;
            }

            Sess->OnDestroySessionCompleteDelegates.RemoveAll(this);
            Sess->OnDestroySessionCompleteDelegates.AddUObject(this, &URelicRunnersGameInstance::OnDestroySessionComplete);
            Sess->DestroySession(NAME_GameSession);

            //Clear local state so it won't reuse stale info
            PendingJoinIndex = -1;
            SessionSearch.Reset();
        }
    }
}

void URelicRunnersGameInstance::OnDestroySessionComplete(FName SessionName, bool bWasSuccessful)
{
    UE_LOG(LogTemp, Log, TEXT("OnDestroySessionComplete: %s success=%d"), *SessionName.ToString(), bWasSuccessful);

    //Unbind delegate
    if (TSharedPtr<IOnlineSession> Sess = SessionInterface.Pin())
    {
        Sess->OnDestroySessionCompleteDelegates.RemoveAll(this);
    }

    //Clear stale search/join state
    PendingJoinIndex = -1;
    SessionSearch.Reset();

    if (PendingHostAfterLeave > 0)
    {
        PendingHostAfterLeave = 0;

        HostGame();
        return;
    }

    //Travel to main menu
    if (APlayerController* PC = GetFirstLocalPlayerController())
    {
        PC->ClientTravel(TEXT("/Game/ThirdPerson/Maps/MainMenu"), ETravelType::TRAVEL_Absolute);
    }
}

void URelicRunnersGameInstance::JoinGame(int SessionIndex)
{
    if (!SessionInterface.IsValid() || !SessionSearch.IsValid() || SessionIndex < 0 || SessionIndex >= SessionSearch->SearchResults.Num()) return;

    if (TSharedPtr<IOnlineSession> Sess = SessionInterface.Pin())
    {
        //If local session exists, destroy it first and queue join
        if (Sess->GetNamedSession(NAME_GameSession))
        {
            PendingJoinIndex = SessionIndex;
            Sess->OnDestroySessionCompleteDelegates.RemoveAll(this);
            Sess->OnDestroySessionCompleteDelegates.AddUObject(this, &URelicRunnersGameInstance::OnSessionDestroyedThenJoin);
            Sess->DestroySession(NAME_GameSession);
            return;
        }

        //No existing local session, join immediately
        Sess->OnJoinSessionCompleteDelegates.RemoveAll(this);
        Sess->OnJoinSessionCompleteDelegates.AddUObject(this, &URelicRunnersGameInstance::OnJoinSessionComplete);
        Sess->JoinSession(0, NAME_GameSession, SessionSearch->SearchResults[SessionIndex]);
        ClientTravelToSession(0, NAME_GameSession);
    }
}

void URelicRunnersGameInstance::OnSessionDestroyedThenJoin(FName SessionName, bool bWasSuccessful)
{
    UE_LOG(LogTemp, Log, TEXT("OnSessionDestroyedThenJoin: %s success=%d"), *SessionName.ToString(), bWasSuccessful);

    if (TSharedPtr<IOnlineSession> Sess = SessionInterface.Pin())
    {
        Sess->OnDestroySessionCompleteDelegates.RemoveAll(this);
    }

    if (!bWasSuccessful)
    {
        UE_LOG(LogTemp, Warning, TEXT("Failed to destroy session before join"));
        PendingJoinIndex = -1;
        return;
    }

    if (PendingJoinIndex < 0 || !SessionSearch.IsValid() || PendingJoinIndex >= SessionSearch->SearchResults.Num())
    {
        UE_LOG(LogTemp, Warning, TEXT("PendingJoinIndex invalid or session search gone"));
        PendingJoinIndex = -1;
        return;
    }

    if (TSharedPtr<IOnlineSession> Sess = SessionInterface.Pin())
    {
        Sess->OnJoinSessionCompleteDelegates.RemoveAll(this);
        Sess->OnJoinSessionCompleteDelegates.AddUObject(this, &URelicRunnersGameInstance::OnJoinSessionComplete);
        Sess->JoinSession(0, NAME_GameSession, SessionSearch->SearchResults[PendingJoinIndex]);
    }

    PendingJoinIndex = -1;
}

void URelicRunnersGameInstance::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
    if (TSharedPtr<IOnlineSession> Sess = SessionInterface.Pin())
    {
        FString ConnectInfo;
        if (!Sess->GetResolvedConnectString(SessionName, ConnectInfo, NAME_GamePort) || ConnectInfo.IsEmpty())
        {
            if (!Sess->GetResolvedConnectString(SessionName, ConnectInfo) || ConnectInfo.IsEmpty())
            {
                FString NoConnect = FString::Printf(TEXT("Failed to resolve connect string for session %s"), *SessionName.ToString());
                UE_LOG(LogTemp, Error, TEXT("%s"), *NoConnect);
                return;
            }
        }

        if (APlayerController* PC = GetFirstLocalPlayerController())
        {
            PC->ClientTravel(ConnectInfo, ETravelType::TRAVEL_Absolute);
        }
    }

    if (TSharedPtr<IOnlineSession> Sess = SessionInterface.Pin())
    {
        Sess->OnJoinSessionCompleteDelegates.RemoveAll(this);
    }
}

void URelicRunnersGameInstance::StartSessionGame()
{
    UWorld* World = GetWorld();
    if (!World) return;

    FString TravelURL = TEXT("/Game/ThirdPerson/Maps/ThirdPersonMap?listen");
    //FString TravelURL = TEXT("/Game/ThirdPerson/Maps/GenerateLevelTest?listen");
    UE_LOG(LogTemp, Warning, TEXT("[LobbyGameMode] Host starting travel to %s"), *TravelURL);

    TSharedPtr<IOnlineSession> Sess = SessionInterface.Pin();
    if (!Sess.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("SessionInterface is null in StartSessionGame!"));
        return;
    }

    FOnlineSessionSettings* CurrentSettings = Sess->GetSessionSettings(NAME_GameSession);
    if (CurrentSettings)
    {
        FString GameTitle = FString::Printf(TEXT("%s's Game"), *PlayerName);
        CurrentSettings->Set(SETTING_MAPNAME, GameTitle, EOnlineDataAdvertisementType::ViaOnlineService);
        Sess->UpdateSession(NAME_GameSession, *CurrentSettings, true);
    }

    //Tell all clients to prepare for travel
    for (FConstPlayerControllerIterator Iterator = World->GetPlayerControllerIterator(); Iterator; ++Iterator)
    {
        ARelicRunnersPlayerController* PC = Cast<ARelicRunnersPlayerController>(*Iterator);
        if (PC && !PC->IsLocalController()) //skip host
        {
            PC->ClientTravelToGame();
        }
    }

    //For host
    World->SeamlessTravel(TravelURL);
}