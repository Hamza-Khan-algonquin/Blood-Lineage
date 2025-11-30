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
protected:

	/** Input Mapping Context to be used for player input */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputMappingContext* InputMappingContext;

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* JumpAction;

	/** Move Forward Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveForwardAction;

	/** Move Backward Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveBackwardAction;

	/** Move Left Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveLeftAction;

	/** Move Right Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveRightAction;


	/** Interact Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* InteractAction;

	/** Inventory Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* InventoryAction;

	/** Pause Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* PauseAction;

	/** AbilitySelection Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* AbilitySelectionAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* LookAction;

	/** Basic attack Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* BasicAttackAction;

	/** Damage Ability Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* DamageAbilityAction;

	/** Defence Ability Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* DefenceAbilityAction;

	/** Utility Ability Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* UtilityAbilityAction;

	/** Ultimate Ability Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* UltimateAbilityAction;

	/** Health Potion Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* HealthPotionAction;

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void OnPossess(APawn* aPawn) override;

	UFUNCTION(BlueprintCallable, Category = "Input")
	UInputMappingContext* GetInputMappingContext() const { return InputMappingContext; }

	virtual void ClientRestart_Implementation(APawn* NewPawn) override;

	class ARelicRunnersCharacter* GetPossessedPawn() { return PossessedPawn; }

	UFUNCTION(Server, Reliable)
	void Server_SetSelectedClass(FName NewClass);

	virtual void OnRep_Pawn() override;

	virtual void AcknowledgePossession(APawn* Pawn) override;

	virtual void OnUnPossess() override;
	UFUNCTION()
	void TryUpdateLobbyUI(int Attempt);

	UFUNCTION()
	void SetupPreviewRenderTarget(APlayerPreview* Preview);

	UFUNCTION()
	void TrySetupPreviewRenderTarget();

	UFUNCTION()
	void SetupLobbyView();

	void SetupMainMenuView();

	UFUNCTION(Server, Reliable)
	void Server_RequestPickup(class AItemActor* Item);

	UFUNCTION(Server, Reliable)
	void Server_RequestFlagActivation(class ACapturableFlag* Flag);

	UPROPERTY(ReplicatedUsing = OnRep_PlayerPreview)
	class APlayerPreview* PlayerPreviewInstance;

	UFUNCTION()
	void OnRep_PlayerPreview();

	UPROPERTY(ReplicatedUsing = OnRep_LobbyPreview)
	class ALobbyPreview* LobbyPreviewInstance;

	UFUNCTION()
	void OnRep_LobbyPreview();

	UFUNCTION(Server, Reliable)
	void Server_RequestStartGame();

	UFUNCTION(Server, Reliable)
	void Server_SendPlayerInfo(const FString& PlayerName, const FName& SelectedClass);

	UFUNCTION(Client, Reliable)
	void Client_LeaveSession();

	UFUNCTION()
	void UpdatePreviewWithEquippedItems();

	void UpdatePreviewItemVisual(UObject* MeshAsset, const FString& ItemType);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	TSubclassOf<class UJoinUserWidget> LobbyWidgetClass = nullptr;

	UPROPERTY()
	class UJoinUserWidget* LobbyWidget = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	TSubclassOf<class UMainMenuWidget> MainMenuWidgetClass = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	class UMainMenuWidget* MainMenuWidget = nullptr;

	UFUNCTION(Client, Reliable)
	void Client_SetupLobby();

	UFUNCTION(Client, Reliable)
	void Client_UpdateLobbyUI();

	UFUNCTION(Client, Reliable)
	void ClientTravelToGame();

	UFUNCTION(Client, Reliable)
	void Client_SendPlayerInfo();
protected:
	ARelicRunnersPlayerController();
	virtual void BeginPlay() override;


	void InitializePawnDependentSystems();

	virtual void SetupInputComponent() override;

	void MoveForward(const struct FInputActionValue& Value);

	void MoveBackward(const struct FInputActionValue& Value);

	void MoveLeft(const struct FInputActionValue& Value);

	void MoveRight(const struct FInputActionValue& Value);

	void Run(const struct FInputActionValue& Value);
	void Look(const struct FInputActionValue& Value);
	void Interact();
	void Jump(const struct FInputActionValue& Value);
	void StopJumping(const struct FInputActionValue& Value);
	void InventoryUI();
	void PauseUI();
	void AbilitySystemUI();
	void DamageAbility();
	void DefenceAbility();
	void UtilityAbility();
	void UltimateAbility();
	void HealthPotion();
	void BasicAttack();

	// End Actor interface

	class ARelicRunnersCharacter* PossessedPawn;


};
