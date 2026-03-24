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

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Sound/SoundBase.h"
#include "RelicRunnersPlayerController.generated.h"

class UInputMappingContext;
class UInventoryComponent;

UENUM(BlueprintType)
enum class EObjectiveType : uint8 {
	None,
	Flag,
	Payload,
	Enemies,
	Pandora
};

USTRUCT(BlueprintType)
struct FPendingObjective {
	GENERATED_BODY()

		UPROPERTY()
	int FlagCount = 0;

	UPROPERTY()
	int EnemyCount = 0;

	UPROPERTY()
	float PandoraTime = 0.0f;

	UPROPERTY()
	EObjectiveType Type = EObjectiveType::None;
};

UCLASS()
class RELICRUNNERS_API ARelicRunnersPlayerController : public APlayerController
{
	GENERATED_BODY()

public:

	//Functions
	void UpdateHUDHealth(float CurrentHealth, float MaxHealth);
	void UpdateHUDHealthPotions(int HealthPotions);
	void UpdateHUDResource(float CurrentResource, float MaxResource);
	void UpdateHUDXP(int Level, float CurrentXP, float MaxXP);
	void UpdateHUDAbilityPoints(int Points);
	void UpdateHUDAll();
	void InventoryUI();
	void SetGameplayUIActive(bool bActive);
	void PauseUI();
	void HideUI(UUserWidget* UIWidget);
	void HandleLevelUpVisuals();
	void UpdatePreviewItemVisual(UObject* MeshAsset, const FString& ItemType);
	void CleanUI();
	void UpdatePayloadObjective(float NewProgress);
	void UpdateEnemiesObjective(int EnemiesLeft);
	void UpdatePandoraObjective(float NewPercent, FString NewTimeText);

	enum class EObjectiveType { None, Flag, Payload, Enemies, Pandora };
	struct FPendingObjective {
		int FlagCount = 0;
		int EnemyCount = 0;
		float PandoraTime = 0.0f;
		EObjectiveType Type = EObjectiveType::None;
	};

	FTimerHandle TimerHandle_UIWait;

	UPROPERTY(ReplicatedUsing = OnRep_ObjectiveBuffer)
	TArray<FPendingObjective> ObjectiveBuffer;

	void AddObjectiveToBuffer(FPendingObjective NewObjective);

	void Internal_SetFlagUI(int CapturedCount);

	UFUNCTION()
	void OnRep_ObjectiveBuffer();

	void FlushObjectiveBuffer();

	void Internal_AddObjectiveFromBuffer(const FPendingObjective& Data);

	void SpectatingUI();

	void PostGameUI();

	UFUNCTION(Client, Reliable)
	void Client_ShowPostGame(const TArray<FPlayerPostGameData>& FinalStats);

	UFUNCTION(Server, Reliable)
	void Server_DecrementFlagObjective();

	UFUNCTION()
	void UpdateCharacterVisuals();
	void UpdateAllAbilityManaUI();
	UFUNCTION()
	void DisableInventoryUIFromTimer();
	UFUNCTION()
	void SyncUI();
	UFUNCTION()
	void TryBindUI();
	UFUNCTION(Server, Reliable)
	void Server_UpdateMetaFromSave(int MetaLevel, int MetaXP, int MetaXPToNextLevel);
	UFUNCTION(Server, Reliable)
	void Server_SetDifficulty(FName NewDifficulty);
	UFUNCTION()
	void OnRep_LobbyPreview();
	UFUNCTION(Client, Reliable)
	void Client_RequestSaveStats();

	UFUNCTION(Client, Reliable)
	void Client_UpdatePayloadUI(float NewProgress);

	UFUNCTION(Client, Reliable)
	void Client_UpdateFlagUI(int FlagsCaptured);

	UFUNCTION(Client, Reliable)
	void Client_UpdatePandoraUI(float NewPercent, const FString& NewTimeText);

	//UFUNCTION(Client, Reliable)
	//void Client_UpdateEnemiesUI(int EnemiesLeft);

	void Internal_SetPandoraUI(float NewPercent, const FString& NewTimeText);

	void Internal_SetPayloadUI(float NewProgress);

	UFUNCTION()
	void OnRep_PlayerPreview();

	UFUNCTION(Server, Reliable)
	void Server_SetSelectedClass(FName NewClass);
	UFUNCTION(Server, Reliable)
	void Server_RequestStartGame();
	UFUNCTION(Server, Reliable)
	void Server_SendPlayerInfo(const FString& PlayerName, const FName& SelectedClass);
	UFUNCTION(Server, Reliable)
	void Server_RequestFlagActivation(class ANewCapturableFlag* Flag);
	UFUNCTION(Server, Reliable)
	void Server_RequestPickup(class AItemActor* Item);
	UFUNCTION(Client, Reliable)
	void Client_SetupLobby();
	UFUNCTION(Client, Reliable)
	void Client_SendPlayerInfo();
	UFUNCTION(Client, Reliable)
	void Client_LeaveSession();
	UFUNCTION(Client, Reliable)
	void Client_UpdateVendorUI(class AVendor* Vendor);
	UFUNCTION(Client, Reliable)
	void Client_UpdateCountdown(int NewTime);
	UFUNCTION(Client, Reliable)
	void Client_UpdateDifficulty(FName NewDifficulty);

	UFUNCTION(Client, Reliable)
	void Client_RegisterMetaXPGain(int Amount);
	UFUNCTION()
	void UpdatePreviewWithEquippedItems();
	UFUNCTION()
	void SetupLobbyView();
	UFUNCTION()
	void SetupMainMenuView();
	UFUNCTION()
	void TrySetupPreviewRenderTarget();
	UFUNCTION()
	void SetupPreviewRenderTarget(APlayerPreview* Preview);
	UFUNCTION()
	void TrySetupMinimap();

	UFUNCTION(Client, Reliable)
	void Client_UpdateStage(int StageNumber);
	UFUNCTION(Client, Reliable)
	void Client_UpdateZones(int CurrentZone, int MaxZones);
	//Getters
	UFUNCTION(BlueprintCallable, Category = "Input")
	UInputMappingContext* GetInputMappingContext() const { return InputMappingContext; }
	class UInventory* GetInventory() { return Inventory; };
	UFUNCTION(BlueprintCallable)
	class UMinimap* GetMinimap() { return MinimapWidget; };
	class ARelicRunnersCharacter* GetPossessedPawn() { return PossessedPawn; }

	//Stored Classes
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	TSubclassOf<class UPauseMenu> PauseMenuClass;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	TSubclassOf<class UPauseMenuWidget> PauseMenuWidgetClass;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	TSubclassOf<class ULoadingScreen> LoadingScreenClass;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	TSubclassOf<class ULevelUpHUD> LevelUpHUDClass;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	TSubclassOf<class UJoinUserWidget> LobbyWidgetClass;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	TSubclassOf<class UAbilitySelection> AbilitySelectionClass;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	TSubclassOf<class UPlayerHUD> PlayerHUDClass;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	TSubclassOf<class UMainMenuWidget> MainMenuWidgetClass;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	TSubclassOf<class UInventory> InventoryClass;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	TSubclassOf<class UObjectiveLog> ObjectiveLogClass;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	TSubclassOf<class UMinimap> MinimapClass;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	TSubclassOf<class USpectatingWidget> SpectatingWidgetClass;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	TSubclassOf<class UPostGameWidget> PostGameWidgetClass;
	UPROPERTY(EditDefaultsOnly, Category = "Preview")
	TSubclassOf<class APlayerPreview> PlayerPreviewClass;
	//Properties
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	class USpectatingWidget* SpectatingWidget;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	class UPostGameWidget* PostGameWidget;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	class UUserWidget* BuildVersion;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	class UMinimap* MinimapWidget;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	class UPauseMenu* PauseMenu;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	class UPauseMenuWidget* PauseMenuWidget;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	class UPlayerHUD* PlayerHUD;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	class UAbilitySelection* AbilitySelection;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	class ULoadingScreen* LoadingScreen;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	class ULevelUpHUD* LevelUpHUD;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	class UJoinUserWidget* LobbyWidget;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	class UMainMenuWidget* MainMenuWidget;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	class UInventory* Inventory;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	class UObjectiveLog* ObjectiveLog;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI|Audio")
	USoundBase* InventoryOpenSound = nullptr;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI|Audio")
	USoundBase* InventoryCloseSound = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI|Audio")
	TObjectPtr<USoundBase> CountdownTickSound = nullptr;
	class AHades* CachedHades;

	UPROPERTY(ReplicatedUsing = OnRep_PlayerPreview)
	class APlayerPreview* PlayerPreviewInstance;
	UPROPERTY(ReplicatedUsing = OnRep_LobbyPreview)
	class ALobbyPreview* LobbyPreviewInstance;

	UFUNCTION(Server, Reliable)
	void Server_ExecuteMove(FVector Target);
	UFUNCTION(Client, Reliable)
	void Client_StartCinematicWalk(FVector TargetLocation);

	float BaseCostAbility1 = 0.f;
	float BaseCostAbility2 = 0.f;
	float BaseCostAbility3 = 0.f;
	float BaseCostAbility4 = 0.f;

	UFUNCTION(BlueprintCallable, Category = "Abilities")
	int GetCalculatedManaCost(float BaseMana);

	FVector CinematicTargetLocation;

	//Timers
	FTimerHandle UIInitialSyncTimerHandle;
	FTimerHandle SyncDataTimerHandle;
	FTimerHandle DisableUITimer;
	FTimerHandle ObjectiveClearTimerHandle;
	FTimerHandle CinematicTimerHandle;

protected:

	//Template
	template<typename T>
	void CleanupWidget(T*& WidgetPtr)
	{
		if (WidgetPtr)
		{
			WidgetPtr->RemoveFromParent();
			WidgetPtr->MarkAsGarbage();
			WidgetPtr = nullptr;
		}
	}

	//Basic Functions
	ARelicRunnersPlayerController();
	void MoveForward(const struct FInputActionValue& Value);
	void MoveBackward(const struct FInputActionValue& Value);
	void MoveLeft(const struct FInputActionValue& Value);
	void MoveRight(const struct FInputActionValue& Value);
	void Look(const struct FInputActionValue& Value);
	void Interact();
	//void Jump(const struct FInputActionValue& Value);
	void StopJumping(const struct FInputActionValue& Value);
	void InitLocalUI();
	void ToggleUI(UUserWidget* UIWidget);
	void AbilitySystemUI();
	void LoadingScreenUI();
	void ForceGameModeOnly();
	void SetupObjectives();
	UFUNCTION(Server, Reliable)
	void Server_RequestCurrentObjective();
	void RemoveOtherUI(UUserWidget* KeepAlive);
	void DamageAbility();
	void DefenceAbility();
	void UtilityAbility();
	void UltimateAbility();
	void HealthPotion();
	void BasicAttack();
	void HeavyAttack();
	void ProjectileHeavyAttack();
	void SetupMinimapRenderTarget();
	void RetrySetup();
	void RestoreGameAndUI();
	void CheckCinematicArrival();

	//Virtual Functions
	virtual void BeginPlay() override;
	virtual void OnRep_Pawn() override;
	virtual void OnRep_PlayerState() override;
	virtual void AcknowledgePossession(APawn* Pawn) override;
	virtual void OnUnPossess() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void OnPossess(APawn* aPawn) override;
	virtual void ClientRestart_Implementation(APawn* NewPawn) override;
	virtual void SetupInputComponent() override;
	virtual void PostSeamlessTravel() override;
	virtual void PreClientTravel(const FString& PendingURL, ETravelType TravelType, bool bIsSeamlessTravel) override;
	virtual void Tick(float DeltaTime) override;

	//Properties
	class ARelicRunnersCharacter* PossessedPawn;
	UPROPERTY(BlueprintReadOnly, Category = "Player")
	class ARelicRunnersPlayerState* RelicPlayerState;

	UPROPERTY()
	class UTitleData* ObjectiveStageTitle;

	//Input
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputMappingContext* InputMappingContext;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputMappingContext* GamepadInputMappingContext;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* JumpAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveForwardAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveBackwardAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveLeftAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveRightAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* InteractAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* InventoryAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* PauseAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* AbilitySelectionAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* LookAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* BasicAttackAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* HeavyAttackAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* DamageAbilityAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* DefenceAbilityAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* UtilityAbilityAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* UltimateAbilityAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* HealthPotionAction;
};
