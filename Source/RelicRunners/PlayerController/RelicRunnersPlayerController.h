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
 *   © 2025 Tristan Anglin. All rights reserved.
 ************************************************************************************/

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "RelicRunnersPlayerController.generated.h"

class UInputMappingContext;
class UInventoryComponent;

UCLASS()
class RELICRUNNERS_API ARelicRunnersPlayerController : public APlayerController
{
	GENERATED_BODY()

public:

	//Functions
	void UpdateHUDHealth(float CurrentHealth, float MaxHealth);
	void UpdateHUDResource(float CurrentResource, float MaxResource);
	void UpdateHUDXP(int Level, float CurrentXP, float MaxXP);
	void UpdateHUDAbilityPoints(int Points);
	void UpdateHUDAll();
	void InventoryUI();
	void PauseUI();
	void HideUI(UUserWidget* UIWidget, bool bClosePopups);
	void HandleLevelUpVisuals();
	void UpdatePreviewItemVisual(UObject* MeshAsset, const FString& ItemType);
	UFUNCTION()
	void UpdateCharacterVisuals();
	UFUNCTION()
	void SyncUI();
	UFUNCTION()
	void TryBindUI();
	UFUNCTION()
	void OnRep_LobbyPreview();
	UFUNCTION()
	void OnRep_PlayerPreview();
	UFUNCTION(Server, Reliable)
	void Server_SetSelectedClass(FName NewClass);
	UFUNCTION(Server, Reliable)
	void Server_RequestStartGame();
	UFUNCTION(Server, Reliable)
	void Server_SendPlayerInfo(const FString& PlayerName, const FName& SelectedClass);
	UFUNCTION(Server, Reliable)
	void Server_RequestFlagActivation(class ACapturableFlag* Flag);
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

	//Getters
	UFUNCTION(BlueprintCallable, Category = "Input")
	UInputMappingContext* GetInputMappingContext() const { return InputMappingContext; }
	class UInventory* GetInventory() { return Inventory; };
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
	UPROPERTY(EditDefaultsOnly, Category = "Preview")
	TSubclassOf<class APlayerPreview> PlayerPreviewClass;


	//Properties
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
	UPROPERTY(ReplicatedUsing = OnRep_PlayerPreview)
	class APlayerPreview* PlayerPreviewInstance;
	UPROPERTY(ReplicatedUsing = OnRep_LobbyPreview)
	class ALobbyPreview* LobbyPreviewInstance;
	FTimerHandle UIInitialSyncTimerHandle;
	FTimerHandle SyncDataTimerHandle;

protected:

	//Basic Functions
	ARelicRunnersPlayerController();
	void MoveForward(const struct FInputActionValue& Value);
	void MoveBackward(const struct FInputActionValue& Value);
	void MoveLeft(const struct FInputActionValue& Value);
	void MoveRight(const struct FInputActionValue& Value);
	void Look(const struct FInputActionValue& Value);
	void Interact();
	void Jump(const struct FInputActionValue& Value);
	void StopJumping(const struct FInputActionValue& Value);
	void InitLocalUI();
	void ToggleUI(UUserWidget* UIWidget);
	void AbilitySystemUI();
	void LoadingScreenUI();
	void RemoveOtherUI(FString UI);
	void DamageAbility();
	void DefenceAbility();
	void UtilityAbility();
	void UltimateAbility();
	void HealthPotion();
	void BasicAttack();

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

	//Properties
	class ARelicRunnersCharacter* PossessedPawn;
	UPROPERTY(BlueprintReadOnly, Category = "Player")
	class ARelicRunnersPlayerState* RelicPlayerState;

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
