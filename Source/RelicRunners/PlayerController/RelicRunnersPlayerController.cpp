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

#include "RelicRunnersPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "RelicRunners/RelicRunnersCharacter.h"
#include "RelicRunners/Inventory/Inventory.h"
#include "InputActionValue.h"
#include "RelicRunners/Item/ItemActor.h"
#include "RelicRunners/LevelGeneration/CapturableFlag.h"
#include "Net/UnrealNetwork.h"
#include "RelicRunners/PlayerPreview/PlayerPreview.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Kismet/KismetRenderingLibrary.h"
#include "RelicRunners/PlayerState/RelicRunnersPlayerState.h"
#include "RelicRunners/Inventory/InventoryComponent.h"
#include "RelicRunners/AbilitySystem/AbilitySelection.h"
#include <RelicRunners/Item/ItemStats.h>
#include <Kismet/GameplayStatics.h>
#include <RelicRunners/Game/RelicRunnersGameInstance.h>
#include "RelicRunners/Menu/MainMenuWidget.h"
#include "RelicRunners/Menu/JoinUserWidget.h"
#include "RelicRunners/Menu/Keybinds.h"

ARelicRunnersPlayerController::ARelicRunnersPlayerController()
{

}

void ARelicRunnersPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ARelicRunnersPlayerController, PlayerPreviewInstance);
	DOREPLIFETIME(ARelicRunnersPlayerController, LobbyPreviewInstance);
}

void ARelicRunnersPlayerController::AcknowledgePossession(APawn* aPawn)
{
	Super::AcknowledgePossession(aPawn);

	if (IsLocalController())
	{
		UE_LOG(LogTemp, Log, TEXT("AcknowledgePossession: %s"), *GetNameSafe(aPawn));
	}
}

void ARelicRunnersPlayerController::OnPossess(APawn* aPawn)
{
	Super::OnPossess(aPawn);

	PossessedPawn = Cast<ARelicRunnersCharacter>(aPawn);
}

void ARelicRunnersPlayerController::ClientRestart_Implementation(APawn* NewPawn)
{
	Super::ClientRestart_Implementation(NewPawn);

	FString MapName = GetWorld()->GetMapName();
	MapName.RemoveFromStart(GetWorld()->StreamingLevelsPrefix);

	if (MapName.Contains("Lobby"))
	{
		SetupLobbyView();
	}
}

void ARelicRunnersPlayerController::ClientTravelToGame_Implementation()
{
	URelicRunnersGameInstance* GI = Cast<URelicRunnersGameInstance>(GetGameInstance());
	GI->ClientTravelToSession(0, NAME_GameSession);
}

void ARelicRunnersPlayerController::OnRep_Pawn()
{
	Super::OnRep_Pawn();

	if (IsLocalController())
	{
		PossessedPawn = Cast<ARelicRunnersCharacter>(GetPawn());
		UE_LOG(LogTemp, Warning, TEXT("OnRep_Pawn: LocalController detected"));
	}
}

void ARelicRunnersPlayerController::OnRep_PlayerPreview()
{
	if (IsLocalController())
	{
		UE_LOG(LogTemp, Warning, TEXT("OnRep_PlayerPreviewInstance: Preview instance replicated"));

		if (PlayerPreviewInstance)
		{
			SetupPreviewRenderTarget(PlayerPreviewInstance);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("OnRep_PlayerPreviewInstance: Still null"));
		}
	}
}

void ARelicRunnersPlayerController::Server_SetSelectedClass_Implementation(FName NewClass)
{
	ARelicRunnersPlayerState* PS = GetPlayerState<ARelicRunnersPlayerState>();
	if (PS)
	{
		PS->SetSelectedClass(NewClass);
	}
}

void ARelicRunnersPlayerController::OnRep_LobbyPreview()
{

}

void ARelicRunnersPlayerController::Server_RequestStartGame_Implementation()
{
	if (URelicRunnersGameInstance* GI = GetGameInstance<URelicRunnersGameInstance>())
	{
		GI->StartSessionGame();
	}
}

void ARelicRunnersPlayerController::Client_LeaveSession_Implementation()
{
	URelicRunnersGameInstance* GI = Cast<URelicRunnersGameInstance>(GetGameInstance());
	if (GI)
	{
		UE_LOG(LogTemp, Log, TEXT("Client %s leaving current session."), *GetName());

		//Leave the session without queuing as host
		GI->LeaveSession(false);
	}
}

void ARelicRunnersPlayerController::UpdatePreviewWithEquippedItems()
{
	if (!PlayerPreviewInstance || !GetPawn()) return;

	ARelicRunnersCharacter* Char = Cast<ARelicRunnersCharacter>(GetPawn());
	if (!Char) return;

	//Clear all visuals on preview first
	PlayerPreviewInstance->ClearAllPreviewMeshes();

	const TArray<FEquippedItemEntry>& EquippedEntries = Char->GetInventoryComponent()->EquippedItems;

	for (const FEquippedItemEntry& Entry : EquippedEntries)
	{
		if (!Entry.Item) continue;

		const FString& ItemType = Entry.ItemType;
		const FString MeshPath = Char->GetMeshPathByItemType(ItemType);

		UObject* LoadedMesh = nullptr;
		if (ItemType == "Staff" || ItemType == "Sword" || ItemType == "Greatsword"  || ItemType == "Bow" || ItemType == "Offhand")
		{
			LoadedMesh = StaticLoadObject(UStaticMesh::StaticClass(), nullptr, *MeshPath);
		}
		else
		{
			LoadedMesh = StaticLoadObject(USkeletalMesh::StaticClass(), nullptr, *MeshPath);
		}

		if (!LoadedMesh) continue;

		UpdatePreviewItemVisual(LoadedMesh, ItemType);
	}
}

void ARelicRunnersPlayerController::UpdatePreviewItemVisual(UObject* MeshAsset, const FString& ItemType)
{
	if (!PlayerPreviewInstance) return;

	//Set or clear the mesh on the preview actor based on asset type
	if (ItemType == "Sword" || ItemType == "Greatsword" || ItemType == "Staff" || ItemType == "Bow" || ItemType == "Offhand")
	{
		PlayerPreviewInstance->SetPreviewMeshByItemType(ItemType, Cast<UStaticMesh>(MeshAsset));
	}
	else
	{
		PlayerPreviewInstance->SetPreviewMeshByItemType(ItemType, Cast<USkeletalMesh>(MeshAsset));
	}
}

void ARelicRunnersPlayerController::Server_RequestPickup_Implementation(AItemActor* Item)
{
	if (Item)
	{
		Item->HandlePickup(Cast<ARelicRunnersCharacter>(GetPawn()));
	}
}

void ARelicRunnersPlayerController::Server_RequestFlagActivation_Implementation(ACapturableFlag* Flag)
{
	if (Flag)
	{
		Flag->HandleInteracted(Cast<ARelicRunnersCharacter>(GetPawn()));
	}
}

void ARelicRunnersPlayerController::OnUnPossess()
{
	APawn* MyPawn = Cast<APawn>(GetOwner());

	Super::OnUnPossess();
}

void ARelicRunnersPlayerController::Client_UpdateLobbyUI_Implementation()
{
	TryUpdateLobbyUI(0);
}

void ARelicRunnersPlayerController::TryUpdateLobbyUI(int Attempt)
{
	int MaxAttempts = 5;
	if (Attempt >= MaxAttempts) return;

	if (LobbyWidget)
	{
		LobbyWidget->UpdateFindGamesButtonVisibility();
	}

	FTimerHandle RetryHandle;
	GetWorld()->GetTimerManager().SetTimer(RetryHandle, FTimerDelegate::CreateUObject(this, &ARelicRunnersPlayerController::TryUpdateLobbyUI, Attempt + 1), 0.2f, false);
}

void ARelicRunnersPlayerController::SetupPreviewRenderTarget(APlayerPreview* Preview)
{
	if (!IsLocalController() || !Preview)
	{
		UE_LOG(LogTemp, Warning, TEXT("[SetupPreviewRenderTarget] Invalid call or not local controller."));
		return;
	}

	ARelicRunnersCharacter* Char = Cast<ARelicRunnersCharacter>(GetPawn());
	if (!Char)
	{
		UE_LOG(LogTemp, Warning, TEXT("[SetupPreviewRenderTarget] Missing character."));
		return;
	}

	// If inventory isn't ready, try again shortly
	if (!Char->Inventory)
	{
		UE_LOG(LogTemp, Warning, TEXT("[SetupPreviewRenderTarget] Inventory is null. Retrying in 0.2s..."));

		FTimerDelegate RetryDelegate;
		RetryDelegate.BindUFunction(this, FName("SetupPreviewRenderTarget"), Preview);

		FTimerHandle RetryHandle;
		GetWorld()->GetTimerManager().SetTimer(RetryHandle, RetryDelegate, 0.2f, false);
		return;
	}

	// Assign the render target image to the inventory UI
	UTextureRenderTarget2D* RenderTarget = Preview->CreateRenderTarget();
	if (RenderTarget)
	{
		Char->Inventory->SetPreviewActorImage(RenderTarget);
		UE_LOG(LogTemp, Warning, TEXT("[SetupPreviewRenderTarget] Applied render target to UI."));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[SetupPreviewRenderTarget] Failed to create render target."));
	}
}

void ARelicRunnersPlayerController::TrySetupPreviewRenderTarget()
{
	if (!IsLocalController())
		return;

	if (!PlayerPreviewInstance)
	{
		// Retry in a moment if not replicated yet
		GetWorld()->GetTimerManager().SetTimerForNextTick(this, &ARelicRunnersPlayerController::TrySetupPreviewRenderTarget);
		return;
	}

	SetupPreviewRenderTarget(PlayerPreviewInstance);
}

void ARelicRunnersPlayerController::SetupLobbyView()
{
	UWorld* World = GetWorld();
	if (!World) return;

	TArray<AActor*> Cameras;
	UGameplayStatics::GetAllActorsWithTag(World, FName("LobbyCamera"), Cameras);

	if (Cameras.Num() == 0)
	{
		//Retry until the camera exists
		UE_LOG(LogTemp, Warning, TEXT("LobbyCamera not found, retrying..."));
		FTimerHandle RetryHandle;
		GetWorldTimerManager().SetTimer(RetryHandle, this, &ARelicRunnersPlayerController::SetupLobbyView, 0.1f, false);
		return;
	}

	SetViewTargetWithBlend(Cameras[0], 0.0f);

	if (!LobbyWidget && LobbyWidgetClass)
	{
		LobbyWidget = CreateWidget<UJoinUserWidget>(this, LobbyWidgetClass);
		if (LobbyWidget)
		{
			LobbyWidget->AddToViewport();
			LobbyWidget->SetVisibility(ESlateVisibility::Visible);
			bShowMouseCursor = true;
			SetInputMode(FInputModeUIOnly());
		}
	}
	if (URelicRunnersGameInstance* GI = GetGameInstance<URelicRunnersGameInstance>())
	{
		Server_SendPlayerInfo(GI->PlayerName, GI->SelectedClass);
	}
	UE_LOG(LogTemp, Log, TEXT("Lobby setup complete for %s"), *GetNameSafe(this));
}

void ARelicRunnersPlayerController::SetupMainMenuView()
{
	UWorld* World = GetWorld();
	if (!World) return;

	TArray<AActor*> Cameras;
	UGameplayStatics::GetAllActorsWithTag(World, FName("MainMenuCamera"), Cameras);
	if (Cameras.Num() > 0 && Cameras[0])
	{
		SetViewTargetWithBlend(Cameras[0], 0.0f);
	}

	if (MainMenuWidgetClass && !MainMenuWidget)
	{
		MainMenuWidget = CreateWidget<UMainMenuWidget>(this, MainMenuWidgetClass);
		if (MainMenuWidget)
		{
			MainMenuWidget->AddToViewport();
			MainMenuWidget->SetIsEnabled(true);
			MainMenuWidget->SetVisibility(ESlateVisibility::Visible);
			bShowMouseCursor = true;
			SetInputMode(FInputModeUIOnly());
		}
	}
}

void ARelicRunnersPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (!IsLocalController())
	{
		UE_LOG(LogTemp, Log, TEXT("BeginPlay: Not a local controller - skipping UI setup"));
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("BeginPlay: GetWorld() returned null"));
		return;
	}

	const FString MapName = World->GetMapName();
	UE_LOG(LogTemp, Log, TEXT("BeginPlay LocalController - MapName: %s"), *MapName);

	if (World && MapName.Contains(TEXT("Lobby")))
	{
		SetupLobbyView();
	}
	else if (World && MapName.Contains(TEXT("MainMenu")))
	{
		SetupMainMenuView();
	}
	else
	{
		InitializePawnDependentSystems();
	}

	if (URelicRunnersGameInstance* GI = GetGameInstance<URelicRunnersGameInstance>())
	{
		FTimerHandle DelayHandle;
		GetWorldTimerManager().SetTimer(DelayHandle, [this, GI]() { GI->ApplyKeybindings(); }, 0.2f, false);
	}
}

void ARelicRunnersPlayerController::Server_SendPlayerInfo_Implementation(const FString& PlayerName, const FName& SelectedClass)
{
	if (ARelicRunnersPlayerState* PS = GetPlayerState<ARelicRunnersPlayerState>())
	{
		PS->SelectedClass = SelectedClass;
		PS->SetPlayerName(PlayerName);
		PS->ForceNetUpdate();
	}
}

void ARelicRunnersPlayerController::Client_SetupLobby_Implementation()
{
	SetupLobbyView();

	if (URelicRunnersGameInstance* GI = GetGameInstance<URelicRunnersGameInstance>())
	{
		Server_SendPlayerInfo(GI->PlayerName, GI->SelectedClass);
	}
}

void ARelicRunnersPlayerController::Client_SendPlayerInfo_Implementation()
{
	if (URelicRunnersGameInstance* GI = GetGameInstance<URelicRunnersGameInstance>())
	{
		Server_SendPlayerInfo(GI->PlayerName, GI->SelectedClass);
	}
}

void ARelicRunnersPlayerController::InitializePawnDependentSystems()
{
	if (ULocalPlayer* LocalPlayer = GetLocalPlayer())
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LocalPlayer))
		{
			Subsystem->AddMappingContext(InputMappingContext, 0);
			UE_LOG(LogTemp, Log, TEXT("Pawn init: Added input mapping context"));
		}
	}

	// Inventory binding
	FTimerHandle TimerHandle;
	GetWorldTimerManager().SetTimer(TimerHandle, [this]()
		{
			if (ARelicRunnersCharacter* Char = Cast<ARelicRunnersCharacter>(GetPawn()))
			{
				if (UInventoryComponent* Inv = Char->GetInventoryComponent())
				{
					Inv->OnEquipmentChanged.AddUniqueDynamic(this, &ARelicRunnersPlayerController::UpdatePreviewWithEquippedItems);
					UE_LOG(LogTemp, Log, TEXT("Pawn init: Bound inventory OnEquipmentChanged"));
				}
			}
		}, 0.5f, false);
}

void ARelicRunnersPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent))
	{
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ARelicRunnersPlayerController::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ARelicRunnersPlayerController::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveForwardAction, ETriggerEvent::Triggered, this, &ARelicRunnersPlayerController::MoveForward);
		EnhancedInputComponent->BindAction(MoveBackwardAction, ETriggerEvent::Triggered, this, &ARelicRunnersPlayerController::MoveBackward);
		EnhancedInputComponent->BindAction(MoveLeftAction, ETriggerEvent::Triggered, this, &ARelicRunnersPlayerController::MoveLeft);
		EnhancedInputComponent->BindAction(MoveRightAction, ETriggerEvent::Triggered, this, &ARelicRunnersPlayerController::MoveRight);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ARelicRunnersPlayerController::Look);

		// Interact
		EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Started, this, &ARelicRunnersPlayerController::Interact);

		// Inventory
		EnhancedInputComponent->BindAction(InventoryAction, ETriggerEvent::Started, this, &ARelicRunnersPlayerController::InventoryUI);

		// Pause
		EnhancedInputComponent->BindAction(PauseAction, ETriggerEvent::Started, this, &ARelicRunnersPlayerController::PauseUI);

		// Ability System 
		EnhancedInputComponent->BindAction(AbilitySelectionAction, ETriggerEvent::Started, this, &ARelicRunnersPlayerController::AbilitySystemUI);

		// BasicAttack
		EnhancedInputComponent->BindAction(BasicAttackAction, ETriggerEvent::Started, this, &ARelicRunnersPlayerController::BasicAttack);
		
		// Abilities
		EnhancedInputComponent->BindAction(DamageAbilityAction, ETriggerEvent::Started, this, &ARelicRunnersPlayerController::DamageAbility);
		EnhancedInputComponent->BindAction(DefenceAbilityAction, ETriggerEvent::Started, this, &ARelicRunnersPlayerController::DefenceAbility);
		EnhancedInputComponent->BindAction(UtilityAbilityAction, ETriggerEvent::Started, this, &ARelicRunnersPlayerController::UtilityAbility);
		EnhancedInputComponent->BindAction(UltimateAbilityAction, ETriggerEvent::Started, this, &ARelicRunnersPlayerController::UltimateAbility);

		// Health Potion
		EnhancedInputComponent->BindAction(HealthPotionAction, ETriggerEvent::Started, this, &ARelicRunnersPlayerController::HealthPotion);
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input Component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void ARelicRunnersPlayerController::MoveForward(const FInputActionValue& Value)
{
	if (PossessedPawn)
	{
		PossessedPawn->MoveInDirection(EAxis::X, +1.0f);
	}
}

void ARelicRunnersPlayerController::MoveBackward(const FInputActionValue& Value)
{
	if (PossessedPawn)
	{
		PossessedPawn->MoveInDirection(EAxis::X, -1.0f);
	}
}

void ARelicRunnersPlayerController::MoveLeft(const FInputActionValue& Value)
{
	if (PossessedPawn)
	{
		PossessedPawn->MoveInDirection(EAxis::Y, -1.0f);
	}
}

void ARelicRunnersPlayerController::MoveRight(const FInputActionValue& Value)
{
	if (PossessedPawn)
	{
		PossessedPawn->MoveInDirection(EAxis::Y, +1.0f);
	}
}

void ARelicRunnersPlayerController::Look(const FInputActionValue& Value)
{
	if (PossessedPawn)
	{
		PossessedPawn->Look(Value);
	}
}

void ARelicRunnersPlayerController::Interact()
{
	if (PossessedPawn)
	{
		PossessedPawn->Interact();
	}
}

void ARelicRunnersPlayerController::InventoryUI()
{
	if (PossessedPawn)
	{
		PossessedPawn->InventoryUI();
	}
}

void ARelicRunnersPlayerController::PauseUI()
{
	if (PossessedPawn)
	{
		PossessedPawn->PauseUI();
	}
}

void ARelicRunnersPlayerController::AbilitySystemUI()
{
	if (PossessedPawn)
	{
		PossessedPawn->AbilitySystemUI();
	}
}

void ARelicRunnersPlayerController::DamageAbility()
{
	if (PossessedPawn)
	{
		PossessedPawn->DamageAbility();
	}
}
void ARelicRunnersPlayerController::DefenceAbility()
{
	if (PossessedPawn)
	{
		PossessedPawn->DefenceAbility();
	}
}

void ARelicRunnersPlayerController::UtilityAbility()
{
	if (PossessedPawn)
	{
		PossessedPawn->UtilityAbility();
	}
}

void ARelicRunnersPlayerController::UltimateAbility()
{
	if (PossessedPawn)
	{
		PossessedPawn->UltimateAbility();
	}
}

void ARelicRunnersPlayerController::BasicAttack()
{
	if (PossessedPawn)
	{
		PossessedPawn->BasicAttack();
	}
}

void ARelicRunnersPlayerController::Jump(const FInputActionValue& Value)
{
	if (PossessedPawn)
	{
		PossessedPawn->Jump();
	}
}

void ARelicRunnersPlayerController::StopJumping(const FInputActionValue& Value)
{
	if (PossessedPawn)
	{
		PossessedPawn->StopJumping();
	}
}

void ARelicRunnersPlayerController::HealthPotion()
{
	if (PossessedPawn)
	{
		PossessedPawn->HealthPotions();
	}
}

