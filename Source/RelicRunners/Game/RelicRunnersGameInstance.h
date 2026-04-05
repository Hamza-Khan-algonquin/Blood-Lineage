// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
//#include "AdvancedFriendsGameInstance.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSubsystem.h"
#include "RelicRunners/Menu/SaveFileListData.h"
#include "OnlineSubsystemUtils.h"
#include <RelicRunners/Menu/SettingsWidget.h>
#include "RelicRunnersGameInstance.generated.h"

/*UENUM(BlueprintType)
enum class EGameLanguage : uint8
{
    English UMETA(DisplayName = "English"),
    French  UMETA(DisplayName = "French")
};
*/
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
    UTexture* GetLUTForMode(EColorblindMode Colorblind);
    UFUNCTION(BlueprintCallable)
    void StartSessionGame();
    UFUNCTION(BlueprintCallable)
    void HostGame();
    UFUNCTION(BlueprintCallable)
    void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);
    UFUNCTION(BlueprintCallable)
    void BackToMainMenu();

    UFUNCTION(BlueprintCallable, Category = "Network")
    void JoinGameByIP(FString IPAddress);
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



    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|Color")
    UTexture* LUT_Normal;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|Color")
    UTexture* LUT_Protanopia;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|Color")
    UTexture* LUT_Deuteranopia;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|Color")
    UTexture* LUT_Tritanopia;


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

    UPROPERTY()
    class UInputMappingContext* ActiveMappingContext = nullptr;

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
    FName CurrentDifficulty = "Boar";
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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot")
    TSubclassOf<class UIconLibrary> IconLibraryClass;

    UPROPERTY()
    class UIconLibrary* GlobalIconLibrary;

    UPROPERTY(Replicated)
    int StageNumber = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Session")
    bool bIsMatchLocked = false;



    UPROPERTY(BlueprintReadWrite)
    float MasterVolume = 1.0f;

    UPROPERTY(BlueprintReadWrite)
    float MusicVolume = 1.0f;

    UPROPERTY(BlueprintReadWrite)
    float SFXVolume = 1.0f;

    UPROPERTY(BlueprintReadWrite)
    float Brightness = 1.0f;

    UPROPERTY(BlueprintReadWrite)
    float FOV = 90.0f;

    UPROPERTY(BlueprintReadWrite)
    float MouseSensitivityX = 1.0f;

    UPROPERTY(BlueprintReadWrite)
    float MouseSensitivityY = 1.0f;

    UPROPERTY(BlueprintReadWrite)
    bool bMotionBlurEnabled = true;

    UPROPERTY(BlueprintReadWrite)
    FIntPoint Resolution = FIntPoint(1920, 1080);

    UPROPERTY(BlueprintReadWrite)
    TEnumAsByte<EWindowMode::Type> WindowMode = EWindowMode::Windowed;

    UFUNCTION()
    void ApplyAllSettings(APlayerController* PC);

    UPROPERTY(BlueprintReadWrite)
    float SavedFOV;

    UPROPERTY(BlueprintReadWrite)
    float SavedBrightness;

    UPROPERTY(BlueprintReadWrite)
    bool bSavedMotionBlur;

    UPROPERTY(BlueprintReadWrite)
    float SavedSensX;

    UPROPERTY(BlueprintReadWrite)
    float SavedSensY;

    UPROPERTY(BlueprintReadWrite)
    bool bInvertX;

    UPROPERTY(BlueprintReadWrite)
    bool bInvertY;

    UPROPERTY(BlueprintReadWrite)
    EColorblindMode ColorblindMode = EColorblindMode::Normal;

    UPROPERTY(BlueprintReadWrite)
    bool bAutoLoot;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Accessibility")
    FUIColorblindStyle NormalUIStyle;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Accessibility")
    FUIColorblindStyle ProtanopiaUIStyle;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Accessibility")
    FUIColorblindStyle DeuteranopiaUIStyle;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Accessibility")
    FUIColorblindStyle TritanopiaUIStyle;

    UFUNCTION(BlueprintCallable)
    FUIColorblindStyle GetCurrentUIStyle() const;


    //language accessibility
    /*UPROPERTY(BlueprintReadWrite, Category = "Localization")
    EGameLanguage CurrentLanguage = EGameLanguage::English;

    UFUNCTION(BlueprintCallable, Category = "Localization")
    void ApplyLanguage();

    UFUNCTION(BlueprintCallable, Category = "Localization")
    void SetLanguage(EGameLanguage NewLanguage);

    UFUNCTION(BlueprintCallable, Category = "Localization")
    FString GetCultureCodeFromLanguage(EGameLanguage Language) const;
    */
private:


};