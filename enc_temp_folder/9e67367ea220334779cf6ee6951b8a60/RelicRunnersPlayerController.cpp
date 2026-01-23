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
#include "InputActionValue.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Kismet/KismetRenderingLibrary.h"

#include "RelicRunners/RelicRunnersCharacter.h"
#include "RelicRunners/PlayerState/RelicRunnersPlayerState.h"
#include <RelicRunners/Game/RelicRunnersGameInstance.h>

#include "RelicRunners/LevelGeneration/CapturableFlag.h"

#include "RelicRunners/Item/ItemActor.h"
#include "RelicRunners/Item/ItemStats.h"

#include "RelicRunners/PlayerPreview/PlayerPreview.h"

#include "RelicRunners/Inventory/InventoryComponent.h"
#include "RelicRunners/Inventory/InventoryItemOptions.h"
#include "RelicRunners/Inventory/InventorySortingOptions.h"
#include "RelicRunners/Inventory/Inventory.h"
#include "RelicRunners/Vendor/Vendor.h"

#include "RelicRunners/AbilitySystem/AbilitySelection.h"

#include "RelicRunners/Menu/MainMenuWidget.h"
#include "RelicRunners/Menu/JoinUserWidget.h"
#include "RelicRunners/Menu/Keybinds.h"
#include "RelicRunners/Menu/PauseMenu.h"
#include "RelicRunners/Menu/PauseMenuWidget.h"
#include "RelicRunners/Menu/LoadingScreen.h"
#include "RelicRunners/PlayerHUD/PlayerHUD.h"
#include "RelicRunners/LevelUpHUD/LevelUpHUD.h"
#include <RelicRunners/RelicRunnersGameState.h>
#include <RelicRunners/Game/LobbyGameState.h>
#include <Blueprint/WidgetBlueprintLibrary.h>
#include <RelicRunners/Menu/LobbyGameMode.h>

ARelicRunnersPlayerController::ARelicRunnersPlayerController()
{
	static ConstructorHelpers::FClassFinder<ULoadingScreen> BP_LoadingScreen(TEXT("/Game/Blueprints/Menu/BP_LoadingScreen"));

	if (BP_LoadingScreen.Class)
	{
		LoadingScreenClass = BP_LoadingScreen.Class;
		UE_LOG(LogTemp, Warning, TEXT("Character Assigned Loading Screen"));
	}
}

void ARelicRunnersPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ARelicRunnersPlayerController, PlayerPreviewInstance);
	DOREPLIFETIME(ARelicRunnersPlayerController, LobbyPreviewInstance);
}

void ARelicRunnersPlayerController::OnPossess(APawn* aPawn)
{
	//This runs on server
	Super::OnPossess(aPawn);

	PossessedPawn = Cast<ARelicRunnersCharacter>(aPawn);

	if (ARelicRunnersPlayerState* PS = GetPlayerState<ARelicRunnersPlayerState>())
	{
		if (!PS->HasReceivedStarterItems)
		{
			PS->GiveStarterItems();
		}

		UpdateCharacterVisuals();
	}
}

void ARelicRunnersPlayerController::AcknowledgePossession(APawn* aPawn)
{
	Super::AcknowledgePossession(aPawn);

	if (IsLocalController())
	{
		PossessedPawn = Cast<ARelicRunnersCharacter>(aPawn);
		RelicPlayerState = GetPlayerState<ARelicRunnersPlayerState>();

		InitLocalUI();
		GetWorldTimerManager().SetTimer(SyncDataTimerHandle, this, &ARelicRunnersPlayerController::SyncUI, 0.2f, true);
	}
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
	UE_LOG(LogTemp, Warning, TEXT("OnRep_PlayerPreview. Instance Valid: %s"), PlayerPreviewInstance ? *PlayerPreviewInstance->GetName() : TEXT("FALSE"));
	if (PlayerPreviewInstance)
	{
		UE_LOG(LogTemp, Warning, TEXT("Client: Preview Actor Received! Setting up UI..."));

		TrySetupPreviewRenderTarget();
	}
}

void ARelicRunnersPlayerController::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	if (IsLocalController()) RelicPlayerState = GetPlayerState<ARelicRunnersPlayerState>();
}


void ARelicRunnersPlayerController::PostSeamlessTravel()
{
	Super::PostSeamlessTravel();

	if (IsLocalController()) RelicPlayerState = GetPlayerState<ARelicRunnersPlayerState>();
}

void ARelicRunnersPlayerController::PreClientTravel(const FString& PendingURL, ETravelType TravelType, bool bIsSeamlessTravel)
{
	Super::PreClientTravel(PendingURL, TravelType, bIsSeamlessTravel);

	if (ULocalPlayer* LocalPlayer = GetLocalPlayer())
	{
		if (UWorld* World = GetWorld())
		{
			if (PlayerHUD)
			{
				PlayerHUD->RemoveFromParent();
			}

			if (Inventory)
			{
				Inventory->RemoveFromParent();
			}

			if (PauseMenu)
			{
				PauseMenu->RemoveFromParent();
			}

			if (PauseMenuWidget)
			{
				PauseMenuWidget->RemoveFromParent();
			}

			if (AbilitySelection)
			{
				AbilitySelection->RemoveFromParent();
			}

			if (LoadingScreen)
			{
				LoadingScreen->RemoveFromParent();
			}
		}
	}
}

void ARelicRunnersPlayerController::SyncUI()
{
	if (!RelicPlayerState || !RelicPlayerState->GetInventoryComponent())
	{
		RelicPlayerState = GetPlayerState<ARelicRunnersPlayerState>();
		return;
	}

	if (IsLocalController() && !PlayerPreviewInstance && PlayerPreviewClass)
	{
		int UniqueID = RelicPlayerState->GetPlayerId();
		FVector SpawnLoc = FVector(-50000.0f - (UniqueID * 10000.0f), -50000.0f, -2000.0f);

		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		PlayerPreviewInstance = GetWorld()->SpawnActor<APlayerPreview>(PlayerPreviewClass, SpawnLoc, FRotator(0, 180, 0), Params);

		if (PlayerPreviewInstance)
		{
			UE_LOG(LogTemp, Warning, TEXT("Local Client: Spawned Preview Actor at %s"), *SpawnLoc.ToString());
			TrySetupPreviewRenderTarget();
		}
	}
	if (PlayerPreviewInstance)
	{
		//Ensures ui and meshes are refreshed
		UpdatePreviewWithEquippedItems();
		UpdateCharacterVisuals(); 

		Inventory->RefreshEquippedUI();
		RelicPlayerState->GetInventoryComponent()->UpdateTotalEquippedStats(GetPawn());
		UpdateHUDAll();

		GetWorldTimerManager().ClearTimer(SyncDataTimerHandle);
		UE_LOG(LogTemp, Warning, TEXT("Local Client Sync Complete: Actor spawned and Gear equipped."));
	}
}

void ARelicRunnersPlayerController::TryBindUI()
{
	if (!RelicPlayerState) RelicPlayerState = GetPlayerState<ARelicRunnersPlayerState>();

	if (RelicPlayerState && RelicPlayerState->GetInventoryComponent() && Inventory && PlayerHUD)
	{
		UInventoryComponent* Inv = RelicPlayerState->GetInventoryComponent();
		Inventory->SetInventoryComponent(Inv);

		Inv->OnEquipmentChanged.RemoveDynamic(this, &ARelicRunnersPlayerController::UpdateCharacterVisuals);
		Inv->OnEquipmentChanged.AddDynamic(this, &ARelicRunnersPlayerController::UpdateCharacterVisuals);

		Inv->OnEquipmentChanged.RemoveDynamic(this, &ARelicRunnersPlayerController::UpdatePreviewWithEquippedItems);
		Inv->OnEquipmentChanged.AddDynamic(this, &ARelicRunnersPlayerController::UpdatePreviewWithEquippedItems);

		Inventory->RefreshEquippedUI();
		Inv->UpdateTotalEquippedStats(GetPawn());
		UpdateHUDAll();

		UE_LOG(LogTemp, Warning, TEXT("[UI] Post-Travel: Forced manual UI refresh."));
		GetWorldTimerManager().ClearTimer(UIInitialSyncTimerHandle);
	}
	else
	{
		GetWorldTimerManager().SetTimer(UIInitialSyncTimerHandle, this, &ARelicRunnersPlayerController::TryBindUI, 0.1f, true);
	}
}

void ARelicRunnersPlayerController::Server_SetDifficulty_Implementation(FName NewDifficulty)
{
	if (ALobbyGameMode* GM = GetWorld()->GetAuthGameMode<ALobbyGameMode>())
	{
		GM->SetDifficulty(NewDifficulty);
	}
}

void ARelicRunnersPlayerController::Server_ToggleReady_Implementation(bool NewReadyState)
{
	if (ARelicRunnersPlayerState* PS = GetPlayerState<ARelicRunnersPlayerState>())
	{
		PS->IsReady = NewReadyState;
		PS->OnRep_IsReady();
	}
}

void ARelicRunnersPlayerController::UpdateHUDHealth(float CurrentHealth, float MaxHealth)
{
	if (IsLocalController() && PlayerHUD)
	{
		PlayerHUD->UpdateHUDHealth(CurrentHealth, MaxHealth);
	}
}

void ARelicRunnersPlayerController::UpdateHUDResource(float CurrentResource, float MaxResource)
{
	if (IsLocalController() && PlayerHUD)
	{
		PlayerHUD->UpdateHUDResource(CurrentResource, MaxResource);
	}
}

void ARelicRunnersPlayerController::UpdateHUDXP(int Level, float CurrentXP, float MaxXP)
{
	if (IsLocalController() && PlayerHUD)
	{
		PlayerHUD->UpdateHUDXP(Level, CurrentXP, MaxXP);
	}
}

void ARelicRunnersPlayerController::UpdateHUDAbilityPoints(int Points)
{
	if (IsLocalController() && PlayerHUD)
	{
		PlayerHUD->UpdateHUDAbilityPoints(Points);
	}
}

void ARelicRunnersPlayerController::UpdateHUDAll()
{
	if (!IsLocalController() || !PlayerHUD || !RelicPlayerState) return;

	PlayerHUD->UpdateHUDHealth(RelicPlayerState->GetPlayerHealth(), RelicPlayerState->GetPlayerMaxHealth());
	PlayerHUD->UpdateHUDResource(RelicPlayerState->GetPlayerResource(), RelicPlayerState->GetPlayerMaxResource());
	PlayerHUD->UpdateHUDXP(RelicPlayerState->GetPlayerLevel(), RelicPlayerState->GetPlayerXP(), RelicPlayerState->GetPlayerXPToLevel());
	PlayerHUD->UpdateHUDAbilityPoints(RelicPlayerState->GetPlayerAbilityPoints());
	
	if (Inventory)
	{
		Inventory->RefreshInventoryUI();
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

void ARelicRunnersPlayerController::HandleLevelUpVisuals()
{
	if (!IsLocalController()) return;

	if (LevelUpHUDClass)
	{
		ULevelUpHUD* LevelUpWidget = CreateWidget<ULevelUpHUD>(this, LevelUpHUDClass);
		if (LevelUpWidget)
		{
			LevelUpWidget->AddToViewport();
		}
	}

	if (Inventory) Inventory->RefreshEquippedUI();
}

void ARelicRunnersPlayerController::Client_UpdateVendorUI_Implementation(AVendor* Vendor)
{
	if (Inventory && Vendor) Inventory->DisplaySelectedVendorItems(Vendor->GetStock(), Vendor);
}

void ARelicRunnersPlayerController::UpdateCharacterVisuals()
{
	ARelicRunnersCharacter* Char = Cast<ARelicRunnersCharacter>(GetPawn());

	if (!Char || !RelicPlayerState || !RelicPlayerState->GetInventoryComponent()) return;

	const TArray<FEquippedItemEntry>& EquippedEntries = RelicPlayerState->GetInventoryComponent()->EquippedItems;
	Char->SyncEquippedVisuals(EquippedEntries);
}

void ARelicRunnersPlayerController::UpdatePreviewWithEquippedItems()
{
	if (!PlayerPreviewInstance || !GetPawn()) return;

	ARelicRunnersCharacter* Char = Cast<ARelicRunnersCharacter>(GetPawn());
	if (!Char) return;

	//Clear all visuals on preview first
	PlayerPreviewInstance->ClearAllPreviewMeshes();

	if(!RelicPlayerState) RelicPlayerState = GetPlayerState<ARelicRunnersPlayerState>();
	const TArray<FEquippedItemEntry>& EquippedEntries = RelicPlayerState->GetInventoryComponent()->EquippedItems;

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
	if (Item) Item->HandlePickup(Cast<ARelicRunnersCharacter>(GetPawn()));
}

void ARelicRunnersPlayerController::Server_RequestFlagActivation_Implementation(ACapturableFlag* Flag)
{
	if (Flag) Flag->HandleInteracted(Cast<ARelicRunnersCharacter>(GetPawn()));
}

void ARelicRunnersPlayerController::OnUnPossess()
{
	Super::OnUnPossess();
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

	//If inventory isn't ready, try again shortly
	if (!Inventory)
	{
		UE_LOG(LogTemp, Warning, TEXT("[SetupPreviewRenderTarget] Inventory is null. Retrying in 0.2s..."));

		FTimerDelegate RetryDelegate;
		RetryDelegate.BindUFunction(this, FName("SetupPreviewRenderTarget"), Preview);

		FTimerHandle RetryHandle;
		GetWorld()->GetTimerManager().SetTimer(RetryHandle, RetryDelegate, 0.2f, false);
		return;
	}

	//Assign the render target image to the inventory UI
	UTextureRenderTarget2D* RenderTarget = Preview->CreateRenderTarget();
	if (RenderTarget)
	{
		Inventory->SetPreviewActorImage(RenderTarget);
		UE_LOG(LogTemp, Warning, TEXT("[SetupPreviewRenderTarget] Applied render target to UI."));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[SetupPreviewRenderTarget] Failed to create render target."));
	}
}

void ARelicRunnersPlayerController::TrySetupPreviewRenderTarget()
{
	if (!IsLocalController()) return;

	if (!PlayerPreviewInstance || !Inventory)
	{
		GetWorldTimerManager().SetTimerForNextTick(this, &ARelicRunnersPlayerController::TrySetupPreviewRenderTarget);
		return;
	}

	SetupPreviewRenderTarget(PlayerPreviewInstance);
}

void ARelicRunnersPlayerController::Client_UpdateCountdown_Implementation(int NewTime)
{
	TArray<UUserWidget*> FoundWidgets;
	UWidgetBlueprintLibrary::GetAllWidgetsOfClass(GetWorld(), FoundWidgets, UJoinUserWidget::StaticClass());
	for (UUserWidget* Widget : FoundWidgets)
	{
		if (UJoinUserWidget* LobbyUI = Cast<UJoinUserWidget>(Widget))
		{
			LobbyUI->UpdateCountdownUI(NewTime);
		}
	}
}

void ARelicRunnersPlayerController::Client_UpdateDifficulty_Implementation(FName NewDifficulty)
{
	if (URelicRunnersGameInstance* GI = GetGameInstance<URelicRunnersGameInstance>())
	{
		GI->CurrentDifficulty = NewDifficulty;
	}

	TArray<UUserWidget*> FoundWidgets;
	UWidgetBlueprintLibrary::GetAllWidgetsOfClass(GetWorld(), FoundWidgets, UJoinUserWidget::StaticClass());
	for (UUserWidget* Widget : FoundWidgets)
	{
		if (UJoinUserWidget* LobbyUI = Cast<UJoinUserWidget>(Widget))
		{
			LobbyUI->UpdateClientDifficultyUI();
		}
	}
}

void ARelicRunnersPlayerController::ToggleUI(UUserWidget* UIWidget)
{
	if (!UIWidget) return;

	bool IsVisible = UIWidget->IsVisible();
	ESlateVisibility Visibility = IsVisible ? ESlateVisibility::Hidden : ESlateVisibility::Visible;
	bool Enable = !IsVisible;

	UIWidget->SetVisibility(Visibility);
	UIWidget->SetIsEnabled(Enable);
	if (Enable)
	{
		FInputModeGameAndUI InputMode;
		SetInputMode(InputMode);
	}
	else
	{
		FInputModeGameOnly InputMode;
		SetInputMode(InputMode);
	}
	SetShowMouseCursor(Enable);

	if (UIWidget == Inventory)
	{
		UInventoryItemOptions::CloseAnyOpenPopup();
		UInventorySortingOptions::CloseAnyOpenPopup();
	}

	if (UIWidget == PauseMenuWidget)
	{
		UPauseMenuWidget* pauseMenu = Cast<UPauseMenuWidget>(UIWidget);

		pauseMenu->ShowPauseMenu();
	}
}

void ARelicRunnersPlayerController::InventoryUI()
{
	if (!Inventory) return;
	RemoveOtherUI("Inventory");
	ToggleUI(Inventory);
	FSlateApplication::Get().CancelDragDrop();
	Inventory->ToggleVendorUI(false);
}

void ARelicRunnersPlayerController::PauseUI()
{
	//if (!PauseMenu) return;
	//RemoveOtherUI("Pause");
	//ToggleUI(PauseMenu);

	if (!PauseMenuWidget) return;
	RemoveOtherUI("Pause");
	ToggleUI(PauseMenuWidget);
}

void ARelicRunnersPlayerController::AbilitySystemUI()
{
	if (!AbilitySelection || !RelicPlayerState || RelicPlayerState->GetPlayerAbilityPoints() < 1)
	{
		return;
	}

	RemoveOtherUI("Ability");
	ToggleUI(AbilitySelection);
}

void ARelicRunnersPlayerController::LoadingScreenUI()
{
	FInputModeGameOnly InputMode;
	SetInputMode(InputMode);

	LoadingScreen->RemoveFromParent();
}

void ARelicRunnersPlayerController::HideUI(UUserWidget* UIWidget, bool bClosePopups = false)
{
	if (UIWidget->IsVisible())
	{
		UIWidget->SetVisibility(ESlateVisibility::Hidden);
		UIWidget->SetIsEnabled(false);
		SetInputMode(FInputModeGameOnly());
		SetShowMouseCursor(false);

		if (bClosePopups && UIWidget == Inventory)
		{
			UInventoryItemOptions::CloseAnyOpenPopup();
			UInventorySortingOptions::CloseAnyOpenPopup();
		}
	}
}

void ARelicRunnersPlayerController::RemoveOtherUI(FString UI)
{
	if (UI != "Ability") HideUI(AbilitySelection);
	if (UI != "Inventory") HideUI(Inventory, true);
	if (UI != "Pause") HideUI(PauseMenu);
	if (UI != "Pause") HideUI(PauseMenuWidget);
}

void ARelicRunnersPlayerController::InitLocalUI()
{
	if (!PlayerHUD && PlayerHUDClass)
	{
		PlayerHUD = CreateWidget<UPlayerHUD>(this, PlayerHUDClass);
		if (PlayerHUD)
		{
			PlayerHUD->AddToViewport();
		}
	}

	if (!Inventory && InventoryClass)
	{
		Inventory = CreateWidget<UInventory>(this, InventoryClass);
		if (Inventory)
		{
			Inventory->AddToViewport();
			Inventory->SetVisibility(ESlateVisibility::Hidden);
			Inventory->SetIsEnabled(false);
		}
	}

	if (!PauseMenu && PauseMenuClass)
	{
		PauseMenu = CreateWidget<UPauseMenu>(this, PauseMenuClass);
		if (PauseMenu)
		{
			PauseMenu->AddToViewport();
			PauseMenu->SetVisibility(ESlateVisibility::Hidden);
			PauseMenu->SetIsEnabled(false);
		}
	}

	if (!PauseMenuWidget && PauseMenuWidgetClass)
	{
		PauseMenuWidget = CreateWidget<UPauseMenuWidget>(this, PauseMenuWidgetClass);
		if (PauseMenuWidget)
		{
			PauseMenuWidget->AddToViewport();
			PauseMenuWidget->SetVisibility(ESlateVisibility::Hidden);
			PauseMenuWidget->SetIsEnabled(false);
		}
	}

	if (!AbilitySelection && AbilitySelectionClass)
	{
		{
			AbilitySelection = CreateWidget<UAbilitySelection>(this, AbilitySelectionClass);
			if (AbilitySelection)
			{
				AbilitySelection->AddToViewport();
				AbilitySelection->SetVisibility(ESlateVisibility::Hidden);
				AbilitySelection->SetIsEnabled(false);
			}
		}
	}

	if (!LoadingScreen && LoadingScreenClass)
	{
		FInputModeUIOnly InputMode;
		SetInputMode(InputMode);

		LoadingScreen = CreateWidget<ULoadingScreen>(this, LoadingScreenClass);

		FTimerHandle LoadingScreenTimer;

		if (LoadingScreen)
		{
			LoadingScreen->AddToViewport();
			GetWorld()->GetTimerManager().SetTimer(LoadingScreenTimer, this, &ARelicRunnersPlayerController::LoadingScreenUI, 10.0f, false);
		}
	}

	if (URelicRunnersGameInstance* GI = GetGameInstance<URelicRunnersGameInstance>())
	{
		FTimerHandle DelayHandle;
		GetWorldTimerManager().SetTimer(DelayHandle, [this, GI]() { GI->ApplyKeybindings(); }, 0.2f, false);

		Server_SendPlayerInfo(GI->PlayerName, GI->SelectedClass);
	}

	TryBindUI();
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

	if (!IsLocalController()) return;

	UWorld* World = GetWorld();
	if (!World) return;

	const FString MapName = World->GetMapName();
	UE_LOG(LogTemp, Log, TEXT("BeginPlay LocalController - MapName: %s"), *MapName);

	if (MapName.Contains(TEXT("Lobby")))
	{
		SetupLobbyView();
	}
	else if (MapName.Contains(TEXT("MainMenu")))
	{
		SetupMainMenuView();
	}
}

void ARelicRunnersPlayerController::Server_SendPlayerInfo_Implementation(const FString& PlayerName, const FName& SelectedClass)
{
	if (ARelicRunnersPlayerState* PS = GetPlayerState<ARelicRunnersPlayerState>())
	{
		PS->SetSelectedClass(SelectedClass); 
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

