// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "AdvancedFriendsGameInstance.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSubsystem.h"
#include "RelicRunners/Menu/SaveFileListData.h"
#include "OnlineSubsystemUtils.h"
#include "RelicRunnersGameInstance.generated.h"

USTRUCT()
struct FPendingClientTravel
{
    GENERATED_BODY()

public:
    FString TravelURL;
    bool bReady = false;
};

UCLASS()
class RELICRUNNERS_API URelicRunnersGameInstance : public UGameInstance
{
    GENERATED_BODY()

public:

    virtual void Init() override;
    virtual void Shutdown() override;

    //Sessions
    UFUNCTION(BlueprintCallable)
    void CreateNewSession();
    UFUNCTION(BlueprintCallable)
    void StartSessionGame();
    UFUNCTION(BlueprintCallable)
    void HostGame();
    UFUNCTION(BlueprintCallable)
    void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);
    UFUNCTION(BlueprintCallable)
    void BackToMainMenu();
    UFUNCTION(BlueprintCallable)
    void FindGames(class UJoinUserWidget* UserWidget);
    UFUNCTION(BlueprintCallable)
    void OnFindSessionsComplete(bool bWasSuccessful);
    UFUNCTION(BlueprintCallable)
    void JoinGame(int SessionIndex);
    UFUNCTION(BlueprintCallable)
    void OnSessionDestroyedThenJoin(FName SessionName, bool bWasSuccessful);
    UFUNCTION(BlueprintCallable)
    void LeaveSession(bool bQueueHost = false);
    UFUNCTION(BlueprintCallable)
    void OnDestroySessionComplete(FName SessionName, bool bWasSuccessful);
    void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);

    //Getters and Setters
    UFUNCTION(BlueprintCallable)
    void SetCharacterName(const FString& NewName) { PlayerName = NewName; }
    UFUNCTION(BlueprintCallable)
    FString GetCharacterName() const { return PlayerName; }
    void SetSelectedClass(FName NewClass) { SelectedClass = NewClass; }

    //Keybinds
    UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Input")
    class UKeybinds* Keys;
    UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Input")
    class UKeybinds* DefaultKeys;
    UFUNCTION(BlueprintCallable)
    void ResetToDefaults();
    UFUNCTION(BlueprintCallable)
    void ApplyKeybindings();

    //Savefiles
    UFUNCTION(BlueprintCallable)
    class USaveFileListData* CreateSaveFromData(const struct FSaveData& SaveData, UObject* Outer);
    UPROPERTY(BlueprintReadWrite)
    TArray<class USaveFileListData*> SaveSlots;
    UFUNCTION(BlueprintCallable)
    void AddSaveSlot(USaveFileListData* NewSave, UModeSelectionWidget* ParentWidget) {NewSave->IsEmptySlot = false, NewSave->SetParentWidget(ParentWidget); SaveSlots.Add(NewSave); }
    UFUNCTION(BlueprintCallable)
    TArray<class USaveFileListData*> GetSaveSlots() { return SaveSlots; }
    bool HasInitializedSlots = false;
    UFUNCTION()
    void InitializeSaveSlots();
    UPROPERTY(BlueprintReadWrite)
    class USaveFileListData* SelectedSaveSlot = nullptr;

    //Properties
    UPROPERTY(BlueprintReadOnly)
    FName CurrentDifficulty;
    UPROPERTY()
    TArray<FUniqueNetIdRepl> SavedPlayers;
    UPROPERTY(BlueprintReadOnly)
    FString SavedHostAddress;
    UPROPERTY()
    FString PlayerName;
    UPROPERTY()
    FName SelectedClass;
    UPROPERTY()
    TWeakObjectPtr<class UJoinUserWidget> TextRenderWidget;
    UPROPERTY()
    int32 PendingHostAfterLeave = 0;
    UPROPERTY()
    int32 PendingJoinIndex = -1;
    UPROPERTY()
    FString PendingTravelTargetMap;
    TSharedPtr<class FOnlineSessionSearch> SessionSearch;
    UPROPERTY()
    TMap<FUniqueNetIdRepl, FPendingClientTravel> PendingClientTravels;
    TWeakPtr<class IOnlineSession, ESPMode::ThreadSafe> SessionInterface;

    UPROPERTY(Replicated)
    int StageNumber = 0;

private:


};