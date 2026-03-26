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


#include "RelicRunnersPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "InputActionValue.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Kismet/KismetRenderingLibrary.h"
#include "EngineUtils.h"
#include "RelicRunners/RelicRunnersCharacter.h"
#include "RelicRunners/PlayerState/RelicRunnersPlayerState.h"
#include <RelicRunners/Game/RelicRunnersGameInstance.h>
#include "RelicRunners/Game/PostGameWidget.h"
#include "RelicRunners/Game/SpectatingWidget.h"

#include "RelicRunners/LevelGeneration/NewObjectives/NewCapturableFlag.h"

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
#include "RelicRunners/Minimap/Minimap.h"
#include "RelicRunners/Menu/StandardTooltip.h"
#include "RelicRunners/Objective/ObjectiveLog.h"

#include "RelicRunners/Enemy/Hades/Hades.h"

#include "Components/TreeView.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "Components/SceneCaptureComponent2D.h"
#include <RelicRunners/Objective/TitleData.h>
#include <RelicRunners/Objective/PayloadData.h>
#include <RelicRunners/Objective/PandoraData.h>
#include <RelicRunners/Objective/EnemiesData.h>
#include <RelicRunners/Objective/FlagData.h>
#include <RelicRunners/Objective/ObjectiveItemEntry.h>
#include <RelicRunners/Objective/PandorasObjective.h>
#include <NavigationSystem.h>
#include <Blueprint/AIBlueprintHelperLibrary.h>
#include "NavigationPath.h"
#include "GameFramework/CharacterMovementComponent.h"
#include <RelicRunners/LevelGeneration/IslandSpawner.h>

ARelicRunnersPlayerController::ARelicRunnersPlayerController()
{
	static ConstructorHelpers::FClassFinder<ULoadingScreen> BP_LoadingScreen(TEXT("/Game/Blueprints/Menu/BP_LoadingScreen"));

	if (BP_LoadingScreen.Class)
	{
		LoadingScreenClass = BP_LoadingScreen.Class;
		UE_LOG(LogTemp, Warning, TEXT("Character Assigned Loading Screen"));
	}
}

void ARelicRunnersPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UWorld* World = GetWorld();
	if (!World) return;

	const FString MapName = World->GetMapName();
	if (MapName.Contains(TEXT("BossLevel")))
	{
		if (!CachedHades)
		{
			CachedHades = Cast<AHades>(UGameplayStatics::GetActorOfClass(GetWorld(), AHades::StaticClass()));
		}

		if (CachedHades && PlayerHUD)
		{
			if (PlayerHUD->BossBar)
			{
				float NewPercent = CachedHades->CurrentHealth / CachedHades->MaxHealth;
				if (!FMath::IsNearlyEqual(PlayerHUD->BossBar->GetPercent(), NewPercent, 0.001f))
				{
					PlayerHUD->BossBar->SetPercent(NewPercent);
				}
			}
			if (PlayerHUD->TB_BossName)
			{
				FText NewName = FText::FromString("HADES");
				if (!PlayerHUD->TB_BossName->GetText().EqualTo(NewName))
				{
					PlayerHUD->TB_BossName->SetText(NewName);
				}
			}
		}
	}
}

void ARelicRunnersPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ARelicRunnersPlayerController, PlayerPreviewInstance);
	DOREPLIFETIME(ARelicRunnersPlayerController, LobbyPreviewInstance);
	DOREPLIFETIME(ARelicRunnersPlayerController, ObjectiveBuffer);
	DOREPLIFETIME(ARelicRunnersPlayerController, BaseCostAbility1);
	DOREPLIFETIME(ARelicRunnersPlayerController, BaseCostAbility2);
	DOREPLIFETIME(ARelicRunnersPlayerController, BaseCostAbility3);
	DOREPLIFETIME(ARelicRunnersPlayerController, BaseCostAbility4);
}

void ARelicRunnersPlayerController::OnPossess(APawn* aPawn)
{
	//This runs on server
	Super::OnPossess(aPawn);

	PossessedPawn = Cast<ARelicRunnersCharacter>(aPawn);

	UpdateCharacterVisuals();

}

void ARelicRunnersPlayerController::Server_InitializePlayerData_Implementation(FSaveData ClientMeta)
{
	if (ARelicRunnersPlayerState* PS = GetPlayerState<ARelicRunnersPlayerState>())
	{
		if (!PS->HasReceivedStarterItems)
		{
			PS->InitializeMetaStats(ClientMeta);
			PS->GiveStarterItems();
		}
	}
}

void ARelicRunnersPlayerController::AcknowledgePossession(APawn* aPawn)
{
	Super::AcknowledgePossession(aPawn);

	if (IsLocalController())
	{
		PlayerPreviewInstance = nullptr;

		PossessedPawn = Cast<ARelicRunnersCharacter>(aPawn);
		RelicPlayerState = GetPlayerState<ARelicRunnersPlayerState>();

		InitLocalUI();
		GetWorldTimerManager().SetTimer(SyncDataTimerHandle, this, &ARelicRunnersPlayerController::SyncUI, 0.2f, true);
		TrySetupMinimap();
	}

	if (HasAuthority())
	{
		if (ARelicRunnersPlayerState* PS = GetPlayerState<ARelicRunnersPlayerState>())
		{
			PS->RefreshStatsForNewLevel();
		}
	}
}

void ARelicRunnersPlayerController::ClientRestart_Implementation(APawn* NewPawn)
{
	Super::ClientRestart_Implementation(NewPawn);

	FString MapName = GetWorld()->GetMapName();
	MapName.RemoveFromStart(GetWorld()->StreamingLevelsPrefix);

	if (MapName.Contains("Lobby"))
	{
		
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

	PlayerPreviewInstance = nullptr;

	if (IsLocalController()) RelicPlayerState = GetPlayerState<ARelicRunnersPlayerState>();

	if (HasAuthority())
	{
		FTimerHandle RefreshTimer;
		GetWorldTimerManager().SetTimer(RefreshTimer, this, &ARelicRunnersPlayerController::OnTravelRefreshDelay, 0.2f, false);
	}

	EnableInput(this);
}

void ARelicRunnersPlayerController::OnTravelRefreshDelay()
{
	if (ARelicRunnersPlayerState* PS = GetPlayerState<ARelicRunnersPlayerState>())
	{
		PS->RefreshStatsForNewLevel();
	}
}

void ARelicRunnersPlayerController::PreClientTravel(const FString& PendingURL, ETravelType TravelType, bool bIsSeamlessTravel)
{
	Super::PreClientTravel(PendingURL, TravelType, bIsSeamlessTravel);
}

void ARelicRunnersPlayerController::CleanUI()
{
	//Clean up UI
	if (UWorld* World = GetWorld())
	{
		if (!IsLocalController()) return;

		DisableInput(this);

		TArray<UUserWidget*> FoundWidgets;
		UWidgetBlueprintLibrary::GetAllWidgetsOfClass(World, FoundWidgets, UUserWidget::StaticClass(), false);

		UE_LOG(LogTemp, Warning, TEXT("--- TRAVEL CLEANUP: Checking %d Widgets ---"), FoundWidgets.Num());

		for (int i = 0; i < FoundWidgets.Num(); i++)
		{
			UUserWidget* CurrentWidget = FoundWidgets[i];
			if (!IsValid(CurrentWidget)) continue;

			APlayerController* WidgetOwner = CurrentWidget->GetOwningPlayer();

			if (WidgetOwner == this)
			{
				if (CurrentWidget->IsInViewport() || CurrentWidget->GetParent() != nullptr)
				{
					CleanupWidget(FoundWidgets[i]);
				}
				else
				{
					CurrentWidget->MarkAsGarbage();
					FoundWidgets[i] = nullptr;
					UE_LOG(LogTemp, Log, TEXT("SILENT: [%s] (Non-Viewport / 3D Widget)"), *CurrentWidget->GetName());
				}
			}
			else
			{
				FString OwnerName = WidgetOwner ? WidgetOwner->GetName() : TEXT("NO OWNER");
				UE_LOG(LogTemp, Log, TEXT("SKIPPING: [%s] (Owned by: %s)"), *CurrentWidget->GetName(), *OwnerName);
			}
		}
		UE_LOG(LogTemp, Warning, TEXT("---------------------------------------------"));
	}

	FInputModeGameOnly GameOnly;
	SetInputMode(GameOnly);
}

void ARelicRunnersPlayerController::SyncUI()
{
	if (!RelicPlayerState || !RelicPlayerState->GetInventoryComponent())
	{
		RelicPlayerState = GetPlayerState<ARelicRunnersPlayerState>();
		return;
	}

	if (IsLocalController())
	{
		if (!PlayerPreviewInstance && PlayerPreviewClass)
		{
			int UniqueID = RelicPlayerState->GetPlayerId();

			//Playerstate isnt ready yet retry
			if (UniqueID <= 0) return;

			FVector SpawnLoc = FVector(-50000.0f - (UniqueID * 10000.0f), -50000.0f, -2000.0f);

			FActorSpawnParameters Params;
			Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

			PlayerPreviewInstance = GetWorld()->SpawnActor<APlayerPreview>(PlayerPreviewClass, SpawnLoc, FRotator(0, 180, 0), Params);

			if (PlayerPreviewInstance)
			{
				PlayerPreviewInstance->LinkedPlayerState = RelicPlayerState;
				//Manual trigger since BeginPlay might have already fired
				PlayerPreviewInstance->RefreshPreview();

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
			UpdateHUDAll();

			GetWorldTimerManager().ClearTimer(SyncDataTimerHandle);
			UE_LOG(LogTemp, Warning, TEXT("Local Client Sync Complete: Actor spawned and Gear equipped."));
		}
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
		Inv->UpdateTotalEquippedStats(PossessedPawn);
		UpdateHUDAll();

		UE_LOG(LogTemp, Warning, TEXT("[UI] Post-Travel: Forced manual UI refresh."));
		GetWorldTimerManager().ClearTimer(UIInitialSyncTimerHandle);
	}
	else
	{
		GetWorldTimerManager().SetTimer(UIInitialSyncTimerHandle, this, &ARelicRunnersPlayerController::TryBindUI, 0.1f, true);
	}
}

void ARelicRunnersPlayerController::Client_RegisterMetaXPGain_Implementation(int Amount)
{
	if (URelicRunnersGameInstance* GI = Cast<URelicRunnersGameInstance>(GetGameInstance()))
	{
		if (GI->SelectedSaveSlot)
		{
			GI->SelectedSaveSlot->AddMetaExp(Amount);
		}
	}
}

void ARelicRunnersPlayerController::Server_SetDifficulty_Implementation(FName NewDifficulty)
{
	if (ALobbyGameMode* GM = GetWorld()->GetAuthGameMode<ALobbyGameMode>())
	{
		GM->SetDifficulty(NewDifficulty);
	}
}

void ARelicRunnersPlayerController::UpdateHUDHealth(float CurrentHealth, float MaxHealth)
{
	if (IsLocalController() && PlayerHUD)
	{
		PlayerHUD->UpdateHUDHealth(CurrentHealth, MaxHealth);
	}
}

void ARelicRunnersPlayerController::UpdateHUDHealthPotions(int HealthPotions)
{
	if (IsLocalController() && PlayerHUD)
	{
		PlayerHUD->UpdateHUDHealthPotions(HealthPotions);
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

	if (GetWorld() == nullptr || GetWorld()->IsBeingCleanedUp() == true) return;

	if (!LevelUpHUDClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("LevelUpHUDClass is null on Controller!"));
		return;
	}

	if (LevelUpHUD)
	{
		LevelUpHUD->RemoveFromParent();
		LevelUpHUD = nullptr;
	}

	if (LevelUpHUDClass)
	{
		LevelUpHUD = CreateWidget<ULevelUpHUD>(this, LevelUpHUDClass);
		if (LevelUpHUD)
		{
			LevelUpHUD->AddToViewport();
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

void ARelicRunnersPlayerController::UpdateAllAbilityManaUI()
{
	if (!PlayerHUD || !GetPawn()) return;

	int Cost1 = GetCalculatedManaCost(BaseCostAbility1);
	int Cost2 = GetCalculatedManaCost(BaseCostAbility2);
	int Cost3 = GetCalculatedManaCost(BaseCostAbility3);
	int Cost4 = GetCalculatedManaCost(BaseCostAbility4);

	PlayerHUD->UpdateAbilityManaCosts(Cost1, Cost2, Cost3, Cost4);
}

void ARelicRunnersPlayerController::OnRep_AbilityCosts()
{
	UpdateAllAbilityManaUI();
}

int ARelicRunnersPlayerController::GetCalculatedManaCost(float BaseMana)
{
	int Intelligence = 0;

	if (ARelicRunnersPlayerState* PS = GetPlayerState<ARelicRunnersPlayerState>())
	{
		if (UInventoryComponent* Inv = PS->GetInventoryComponent())
		{
			Intelligence = Inv->CachedEquippedStats.TotalIntelligence;
		}
	}

	float ReductionPercent = ItemStats::GetAbilityCostPerc(Intelligence);

	int FinalCost = BaseMana * (1.f - (ReductionPercent / 100.f));

	return FMath::Max(FinalCost, 0);
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

void ARelicRunnersPlayerController::Server_RequestFlagActivation_Implementation(ANewCapturableFlag* Flag)
{
	if (Flag) Flag->HandleInteracted(Cast<ARelicRunnersCharacter>(GetPawn()));
}

void ARelicRunnersPlayerController::OnUnPossess()
{
	Super::OnUnPossess();
}

void ARelicRunnersPlayerController::TrySetupMinimap()
{
	if (!IsLocalController()) return;

	ARelicRunnersCharacter* Char = Cast<ARelicRunnersCharacter>(GetPawn());

	if (!Char)
	{
		UE_LOG(LogTemp, Error, TEXT("Minimap Setup Failed: Pawn is null. Retrying..."));
		RetrySetup();
		return;
	}

	if (!MinimapWidget)
	{
		UE_LOG(LogTemp, Error, TEXT("Minimap Setup Failed: Widget is null. Retrying..."));
		RetrySetup();
		return;
	}

	if (!Char->MinimapCaptureComponent)
	{
		UE_LOG(LogTemp, Error, TEXT("Minimap Setup Failed: Component missing!"));
		return;
	}

	SetupMinimapRenderTarget();
}

void ARelicRunnersPlayerController::Client_UpdateZones_Implementation(int CurrentZone, int MaxZones)
{
	if (ObjectiveLog)
	{
		ObjectiveStageTitle->CurrentZones = CurrentZone;
		ObjectiveStageTitle->TotalZones = MaxZones;
		ObjectiveLog->RefreshLog(ObjectiveStageTitle);
		ObjectiveLog->UpdateObjective(ObjectiveStageTitle);
	}
}

void ARelicRunnersPlayerController::Client_UpdateStage_Implementation(int StageNumber)
{
	if (MinimapWidget)
	{
		FText FinalText = FText::Format(FText::FromString("Stage {0}"), StageNumber);
		MinimapWidget->SetLevelTitle(FinalText);
	}
}

void ARelicRunnersPlayerController::RetrySetup()
{
	FTimerHandle RetryHandle;
	GetWorldTimerManager().SetTimer(RetryHandle, this, &ARelicRunnersPlayerController::TrySetupMinimap, 0.2f, false);
}

void ARelicRunnersPlayerController::SetupMinimapRenderTarget()
{
	if (!IsLocalController() || !MinimapWidget) return;

	ARelicRunnersCharacter* Char = Cast<ARelicRunnersCharacter>(GetPawn());
	if (!Char || !Char->IsLocallyControlled()) return;

	UTextureRenderTarget2D* NewRT = MinimapWidget->CreateRenderTarget(Char);

	if (NewRT)
	{
		Char->MinimapCaptureComponent->TextureTarget = NewRT;
		Char->MinimapCaptureComponent->Activate();
		Char->MinimapCaptureComponent->bCaptureEveryFrame = true;
		MinimapWidget->SetMinimapImage(NewRT);

		UE_LOG(LogTemp, Warning, TEXT("SERVER: Minimap successfully linked to RT: %s"), *NewRT->GetName());
	}
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
	if (CountdownTickSound && NewTime > 0)
	{
		UGameplayStatics::PlaySound2D(this, CountdownTickSound);
	}

	if (LobbyWidget)
	{
		LobbyWidget->UpdateCountdownUI(NewTime);
	}
}

void ARelicRunnersPlayerController::Client_UpdateDifficulty_Implementation(FName NewDifficulty)
{
	if (URelicRunnersGameInstance* GI = GetGameInstance<URelicRunnersGameInstance>())
	{
		GI->CurrentDifficulty = NewDifficulty;
	}

	if (LobbyWidget)
	{
		LobbyWidget->UpdateClientDifficultyUI();
	}
}

void ARelicRunnersPlayerController::ToggleUI(UUserWidget* UIWidget)
{
	if (!UIWidget) return;

	bool IsOpening = !UIWidget->IsVisible();

	if (IsOpening)
	{
		RemoveOtherUI(UIWidget);

		if (UIWidget != MinimapWidget)
		{
			SetInputMode(FInputModeGameAndUI());
			SetShowMouseCursor(true);
		}
		if (UIWidget == PauseMenuWidget)
		{
			if (UPauseMenuWidget* Pause = Cast<UPauseMenuWidget>(UIWidget)) Pause->ShowPauseMenu();
		}
		else if (UIWidget == Inventory)
		{
			Inventory->PlayAnimation(Inventory->InventoryOpen);

			if (InventoryOpenSound)
			{
				UGameplayStatics::PlaySound2D(this, InventoryOpenSound);
			}
		}
		UIWidget->SetVisibility(ESlateVisibility::Visible);
		UIWidget->SetIsEnabled(true);
	}
	else
	{
		HideUI(UIWidget);
	}
}

void ARelicRunnersPlayerController::HideUI(UUserWidget* UIWidget)
{
	if (!UIWidget || !UIWidget->IsVisible()) return;

	SetInputMode(FInputModeGameOnly());
	SetShowMouseCursor(false);

	if (UIWidget == Inventory)
	{
		UInventoryItemOptions::CloseAnyOpenPopup();
		UInventorySortingOptions::CloseAnyOpenPopup();

		Inventory->PlayAnimation(Inventory->InventoryClose);

		if (InventoryCloseSound)
		{
			UGameplayStatics::PlaySound2D(this, InventoryCloseSound);
		}

		Inventory->ToggleShopUI(false);
		Inventory->ToggleForgeUI(false);

		float AnimDelay = 0.25f;
		GetWorld()->GetTimerManager().SetTimer(DisableUITimer, this, &ARelicRunnersPlayerController::DisableInventoryUIFromTimer, AnimDelay, false);

		if (Inventory->StandardTooltip) Inventory->StandardTooltip->RemoveFromParent(); Inventory->StandardTooltip = nullptr;
	}
	else
	{
		UIWidget->SetVisibility(ESlateVisibility::Hidden);
		UIWidget->SetIsEnabled(false);
	}
}

void ARelicRunnersPlayerController::DisableInventoryUIFromTimer()
{
	if (Inventory)
	{
		Inventory->SetVisibility(ESlateVisibility::Hidden);
		Inventory->SetIsEnabled(false);

		GetWorld()->GetTimerManager().ClearTimer(DisableUITimer);
	}
}

void ARelicRunnersPlayerController::RemoveOtherUI(UUserWidget* KeepAlive)
{
	TArray<UUserWidget*> AllMenus = { Inventory, PauseMenuWidget, AbilitySelection };

	for (UUserWidget* Menu : AllMenus)
	{
		if (Menu && Menu != KeepAlive)
		{
			HideUI(Menu);
		}
	}
}

void ARelicRunnersPlayerController::InventoryUI()
{
	if (!Inventory) return;
	FSlateApplication::Get().CancelDragDrop();
	Inventory->ToggleShopUI(false);
	Inventory->ToggleForgeUI(false);
	ToggleUI(Inventory);
}

void ARelicRunnersPlayerController::SetGameplayUIActive(bool bActive)
{
	ESlateVisibility NewVisibility = bActive ? ESlateVisibility::Visible : ESlateVisibility::Collapsed;

	if (MinimapWidget)
	{
		if (bActive && !MinimapWidget->IsInViewport()) MinimapWidget->AddToViewport();
		MinimapWidget->SetVisibility(NewVisibility);
	}

	if (PlayerHUD)
	{
		if (bActive && !PlayerHUD->IsInViewport()) PlayerHUD->AddToViewport();
		PlayerHUD->SetVisibility(NewVisibility);
	}

	if (Inventory)
	{
		if (bActive) {
			Inventory->SetVisibility(ESlateVisibility::Collapsed);
		}
		else {
			Inventory->SetVisibility(ESlateVisibility::Collapsed);
			Inventory->SetKeyboardFocus();
		}
	}
}

void ARelicRunnersPlayerController::PauseUI()
{
	ToggleUI(PauseMenuWidget);
}

void ARelicRunnersPlayerController::AbilitySystemUI()
{
	if (AbilitySelection && RelicPlayerState && RelicPlayerState->GetPlayerAbilityPoints() >= 1)
	{
		ToggleUI(AbilitySelection);
	}
}

void ARelicRunnersPlayerController::LoadingScreenUI()
{
	if (LoadingScreen)
	{
		LoadingScreen->RemoveFromParent();

		if (!GetWorld()->GetMapName().Contains(TEXT("BossLevel"))) ForceGameModeOnly();
	}
}

void ARelicRunnersPlayerController::ForceGameModeOnly()
{
	SetInputMode(FInputModeGameOnly());
	SetShowMouseCursor(false);
}

void ARelicRunnersPlayerController::SetupObjectives()
{
	if (ObjectiveLogClass && !ObjectiveLog)
	{
		ObjectiveLog = CreateWidget<UObjectiveLog>(this, ObjectiveLogClass);
		ObjectiveLog->AddToViewport();

		ObjectiveStageTitle = NewObject<UTitleData>(this);

		UWorld* World = GetWorld();
		if (!World) return;

		const FString MapName = World->GetMapName();

		if (MapName.Contains(TEXT("ShopDemo")))
		{
			ObjectiveStageTitle->TotalZones = 0;
			ObjectiveStageTitle->CurrentZones = 0;
		}
		else if (MapName.Contains(TEXT("ThirdPersonMap")))
		{
			ObjectiveStageTitle->TotalZones = 0;
			ObjectiveStageTitle->CurrentZones = 0;
		}
		else if (MapName.Contains(TEXT("IslandTest")))
		{
			ObjectiveStageTitle->TotalZones = 5;
			ObjectiveStageTitle->CurrentZones = 0;
		}
		else if (MapName.Contains(TEXT("BossLevel")))
		{
			ObjectiveStageTitle->TotalZones = 0;
			ObjectiveStageTitle->CurrentZones = 0;
		}
		if (ObjectiveStageTitle->TotalZones != 0)
		{
			ObjectiveLog->InitializeObjectives(ObjectiveStageTitle);
		}
	}

	if (IsLocalController())
	{
		Server_RequestCurrentObjective();
	}
}

void ARelicRunnersPlayerController::Server_RequestCurrentObjective_Implementation()
{
	//Find the Spawner in the world
	for (TActorIterator<AIslandSpawner> It(GetWorld()); It; ++It)
	{
		It->SyncObjectiveToSinglePlayer(this);
		break;
	}
}

void ARelicRunnersPlayerController::UpdatePayloadObjective(float NewProgress)
{
	Internal_SetPayloadUI(NewProgress);

	if (HasAuthority())
	{
		Client_UpdatePayloadUI(NewProgress);
	}
}

void ARelicRunnersPlayerController::Client_UpdatePayloadUI_Implementation(float NewProgress)
{
	Internal_SetPayloadUI(NewProgress);
}

void ARelicRunnersPlayerController::Client_UpdatePandoraUI_Implementation(float NewPercent, const FString& NewTimeText)
{
	Internal_SetPandoraUI(NewPercent, NewTimeText);
}

void ARelicRunnersPlayerController::Internal_SetPandoraUI(float NewPercent, const FString& NewTimeText)
{
	if (!ObjectiveStageTitle || !ObjectiveLog) return;

	for (UObjectiveItemData* SubObj : ObjectiveStageTitle->SubObjectives)
	{
		if (UPandoraData* PandoraGoal = Cast<UPandoraData>(SubObj))
		{
			PandoraGoal->Percent = NewPercent;
			PandoraGoal->TimeRemainingText = NewTimeText;

			if (PandoraGoal->Percent >= 1.0f && !PandoraGoal->Completed)
			{
				PandoraGoal->Completed = true;
			}

			ObjectiveLog->UpdateObjective(PandoraGoal);
			return;
		}
	}
}

void ARelicRunnersPlayerController::Internal_SetPayloadUI(float NewProgress)
{
	if (!ObjectiveStageTitle || !ObjectiveLog) return;

	for (UObjectiveItemData* SubObj : ObjectiveStageTitle->SubObjectives)
	{
		if (UPayloadData* PayloadGoal = Cast<UPayloadData>(SubObj))
		{
			PayloadGoal->ProgressFraction = NewProgress;

			if (PayloadGoal->ProgressFraction >= 1.0f && !PayloadGoal->Completed)
			{
				PayloadGoal->Completed = true;
			}

			ObjectiveLog->UpdateObjective(PayloadGoal);
			return;
		}
	}
}

void ARelicRunnersPlayerController::UpdateEnemiesObjective(int EnemiesLeft)
{
	if (!ObjectiveStageTitle || !ObjectiveLog) return;

	for (UObjectiveItemData* SubObj : ObjectiveStageTitle->SubObjectives)
	{
		if (UEnemiesData* EnemiesGoal = Cast<UEnemiesData>(SubObj))
		{
			EnemiesGoal->EnemiesRemaining = EnemiesLeft;
			ObjectiveLog->UpdateObjective(EnemiesGoal);
			return;
		}
	}
}

void ARelicRunnersPlayerController::UpdatePandoraObjective(float NewPercent, FString NewTimeText)
{
	if (!ObjectiveStageTitle || !ObjectiveLog) return;

	for (UObjectiveItemData* SubObj : ObjectiveStageTitle->SubObjectives)
	{
		if (UPandoraData* PandoraGoal = Cast<UPandoraData>(SubObj))
		{
			PandoraGoal->Percent = NewPercent;
			PandoraGoal->TimeRemainingText = NewTimeText;

			ObjectiveLog->UpdateObjective(PandoraGoal);
			return;
		}
	}
}

void ARelicRunnersPlayerController::FlushObjectiveBuffer()
{
	if (!ObjectiveLog || !ObjectiveStageTitle || !ObjectiveLog->ObjectiveTreeView) return;

	if (ObjectiveBuffer.Num() > 0)
	{
		GetWorldTimerManager().ClearTimer(TimerHandle_UIWait);

		ObjectiveLog->ObjectiveTreeView->ClearListItems();
		ObjectiveLog->ObjectiveTreeView->AddItem(ObjectiveStageTitle);
		ObjectiveStageTitle->SubObjectives.Empty();

		for (int i = 0; i < ObjectiveBuffer.Num(); ++i)
		{
			UObjectiveItemData* NewGoal = Internal_AddObjectiveFromBuffer(ObjectiveBuffer[i]);

			if (NewGoal && i < ObjectiveBuffer.Num() - 1)
			{
				NewGoal->Completed = true;
			}
		}

		ObjectiveLog->ObjectiveTreeView->SetItemExpansion(ObjectiveStageTitle, true);
		ObjectiveLog->ObjectiveTreeView->RequestRefresh();

		UE_LOG(LogTemp, Warning, TEXT("CLIENT: Catch-up Successful. Buffer Flushed."));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("CLIENT: Waiting for Objective Data from Server..."));
	}
}

UObjectiveItemData* ARelicRunnersPlayerController::Internal_AddObjectiveFromBuffer(const FPendingObjective& Data)
{
	UObjectiveItemData* NewGoal = nullptr;

	if (Data.Type == EObjectiveType::Enemies)
	{
		UEnemiesData* EnemiesGoal = NewObject<UEnemiesData>(this);
		EnemiesGoal->EnemiesRemaining = Data.EnemyCount;
		NewGoal = EnemiesGoal;
	}
	else if (Data.Type == EObjectiveType::Pandora)
	{
		UPandoraData* PandoraGoal = NewObject<UPandoraData>(this);
		PandoraGoal->Percent = 1.0f;
		int Seconds = FMath::CeilToInt(FMath::Fmod(Data.PandoraTime, 60.0f));
		PandoraGoal->TimeRemainingText = FString::Printf(TEXT("%d"), Seconds);
		NewGoal = PandoraGoal;
	}
	else if (Data.Type == EObjectiveType::Payload)
	{
		UPayloadData* PayloadGoal = NewObject<UPayloadData>(this);
		PayloadGoal->ProgressFraction = 0.0f;
		NewGoal = PayloadGoal;
	}
	else if (Data.Type == EObjectiveType::Flag)
	{
		UFlagData* FlagGoal = NewObject<UFlagData>(this);
		FlagGoal->Flag1Active = false;
		FlagGoal->Flag2Active = false;
		FlagGoal->Flag3Active = false;
		FlagGoal->Flag1Enabled = (Data.FlagCount >= 1);
		FlagGoal->Flag2Enabled = (Data.FlagCount >= 2);
		FlagGoal->Flag3Enabled = (Data.FlagCount == 3);
		NewGoal = FlagGoal;
	}

	if (NewGoal)
	{
		ObjectiveLog->AddObjective(ObjectiveStageTitle, NewGoal);
	}

	return NewGoal;
}

void ARelicRunnersPlayerController::AddObjectiveToBuffer(FPendingObjective NewObjective)
{
	if (HasAuthority())
	{
		ObjectiveBuffer.Add(NewObjective);

		MARK_PROPERTY_DIRTY_FROM_NAME(ARelicRunnersPlayerController, ObjectiveBuffer, this);

		if (IsLocalController())
		{
			FlushObjectiveBuffer();
		}
	}
}

void ARelicRunnersPlayerController::Server_DecrementFlagObjective_Implementation()
{
	if (!ObjectiveStageTitle) return;

	for (UObjectiveItemData* SubObj : ObjectiveStageTitle->SubObjectives)
	{
		if (UFlagData* FlagGoal = Cast<UFlagData>(SubObj))
		{
			int NewCapturedCount = 0;

			if (!FlagGoal->Flag1Active) { FlagGoal->Flag1Active = true; NewCapturedCount = 1; }
			else if (!FlagGoal->Flag2Active) { FlagGoal->Flag2Active = true; NewCapturedCount = 2; }
			else if (!FlagGoal->Flag3Active) { FlagGoal->Flag3Active = true; NewCapturedCount = 3; }
			else { NewCapturedCount = 3; } 

			Internal_SetFlagUI(NewCapturedCount);

			Client_UpdateFlagUI(NewCapturedCount);
			return;
		}
	}
}

void ARelicRunnersPlayerController::Client_UpdateFlagUI_Implementation(int CapturedCount)
{
	FString RoleAuth = HasAuthority() ? TEXT("SERVER") : TEXT("CLIENT");
	UE_LOG(LogTemp, Warning, TEXT("[%s] RPC RECEIVED: Flags = %d"), *RoleAuth, CapturedCount);

	Internal_SetFlagUI(CapturedCount);
}

void ARelicRunnersPlayerController::Internal_SetFlagUI(int CapturedCount)
{
	if (!ObjectiveStageTitle || !ObjectiveLog) return;

	for (UObjectiveItemData* SubObj : ObjectiveStageTitle->SubObjectives)
	{
		if (UFlagData* FlagGoal = Cast<UFlagData>(SubObj))
		{
			FlagGoal->Flag1Active = (CapturedCount >= 1);
			FlagGoal->Flag2Active = (CapturedCount >= 2);
			FlagGoal->Flag3Active = (CapturedCount >= 3);

			bool AllClear = true;
			if (FlagGoal->Flag1Enabled && !FlagGoal->Flag1Active) AllClear = false;
			if (FlagGoal->Flag2Enabled && !FlagGoal->Flag2Active) AllClear = false;
			if (FlagGoal->Flag3Enabled && !FlagGoal->Flag3Active) AllClear = false;

			if (AllClear && !FlagGoal->Completed)
			{
				FlagGoal->Completed = true;
			}
			ObjectiveLog->UpdateObjective(FlagGoal);
			return;
		}
	}
}

void ARelicRunnersPlayerController::OnRep_ObjectiveBuffer()
{
	if (!ObjectiveLog || !ObjectiveStageTitle) return;

	FlushObjectiveBuffer();
}

void ARelicRunnersPlayerController::SpectatingUI()
{
	if (!SpectatingWidget && SpectatingWidgetClass)
	{
		SpectatingWidget = CreateWidget<USpectatingWidget>(this, SpectatingWidgetClass);
		if (SpectatingWidget)
		{
			SpectatingWidget->AddToViewport();
		}
	}
}

void ARelicRunnersPlayerController::PostGameUI()
{
	if (!PostGameWidget && PostGameWidgetClass)
	{
		PostGameWidget = CreateWidget<UPostGameWidget>(this, PostGameWidgetClass);
		if (PostGameWidget)
		{
			PostGameWidget->AddToViewport();

			SetInputMode(FInputModeUIOnly());
			bShowMouseCursor = true;

			FTimerHandle UnusedHandle;
			GetWorldTimerManager().SetTimer(UnusedHandle, [this]()
				{
					if (URelicRunnersGameInstance* GI = GetGameInstance<URelicRunnersGameInstance>())
					{
						GI->BackToMainMenu();
					}
				}, 5.0f, false);
		}
	}
}

void ARelicRunnersPlayerController::Client_ShowPostGame_Implementation(const TArray<FPlayerPostGameData>& FinalStats)
{
	if (SpectatingWidget)
	{
		SpectatingWidget->RemoveFromParent();
	}

	PostGameUI();
	if (PostGameWidget) PostGameWidget->UpdateScoreboard(FinalStats);
}

void ARelicRunnersPlayerController::Server_UpdateMetaFromSave_Implementation(int MetaLevel, int MetaXP, int MetaXPToNextLevel)
{
	if (ARelicRunnersPlayerState* PS = GetPlayerState<ARelicRunnersPlayerState>())
	{
		PS->MetaLevel = MetaLevel;
		PS->MetaXP = MetaXP;
		PS->MetaXPToNextLevel = MetaXPToNextLevel;
	}
}

void ARelicRunnersPlayerController::Client_RequestSaveStats_Implementation()
{
	URelicRunnersGameInstance* GI = Cast<URelicRunnersGameInstance>(GetGameInstance());
	if (GI && GI->SelectedSaveSlot)
	{
		Server_UpdateMetaFromSave(
			GI->SelectedSaveSlot->GetMetaLevel(),
			GI->SelectedSaveSlot->GetMetaExp(),
			GI->SelectedSaveSlot->GetMetaExpToLevel()
		);
	}
}

void ARelicRunnersPlayerController::InitLocalUI()
{
	if (!IsLocalController()) return; 

	UWorld* World = GetWorld();
	if (!World) return;

	const FString MapName = World->GetMapName();

	if (BuildVersion && !BuildVersion->IsInViewport())
	{
		BuildVersion->AddToViewport();
	}

	if (!PlayerHUD && PlayerHUDClass)
	{
		PlayerHUD = CreateWidget<UPlayerHUD>(this, PlayerHUDClass);
		if (PlayerHUD)
		{
			PlayerHUD->AddToViewport();
			PlayerHUD->UpdateBindText();
		}
		if (MapName.Contains(TEXT("BossLevel")))
		{
			PlayerHUD->ToggleBossBar(true);
			PlayerHUD->ToggleExpBar(true);
		}
		else
		{
			PlayerHUD->ToggleBossBar(false);
			PlayerHUD->ToggleExpBar(true);
		}
		UpdateAllAbilityManaUI();
	}

	SetupObjectives();

	if (!MinimapWidget && MinimapClass)
	{
		MinimapWidget = CreateWidget<UMinimap>(this, MinimapClass);
		if (MinimapWidget)
		{
			MinimapWidget->AddToViewport();

			if (MapName.Contains(TEXT("ShopDemo")))
			{
				MinimapWidget->SetLevelTitle(FText::FromString("Forge"));
			}
			else if (MapName.Contains(TEXT("IslandTest")))
			{
				MinimapWidget->SetLevelTitle(FText::FromString("Stage"));
			}
			else if (MapName.Contains(TEXT("BossLevel")))
			{
				MinimapWidget->SetLevelTitle(FText::FromString("Underworld"));
			}
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

	float LoadingScreenTime = 6.0f;
	if (!LoadingScreen && LoadingScreenClass)
	{
		FInputModeUIOnly InputMode;
		SetInputMode(InputMode);

		LoadingScreen = CreateWidget<ULoadingScreen>(this, LoadingScreenClass);

		FTimerHandle LoadingScreenTimer;

		if (LoadingScreen)
		{
			LoadingScreen->AddToViewport();
			GetWorld()->GetTimerManager().SetTimer(LoadingScreenTimer, this, &ARelicRunnersPlayerController::LoadingScreenUI, LoadingScreenTime, false);
		}
	}

	if (URelicRunnersGameInstance* GI = GetGameInstance<URelicRunnersGameInstance>())
	{
		FTimerHandle DelayHandle;
		GetWorldTimerManager().SetTimer(DelayHandle, [this, GI]() { GI->ApplyKeybindings(); }, LoadingScreenTime, false);

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

		SetViewTargetWithBlend(Cameras[1], 0.0f);
		SetViewTargetWithBlend(Cameras[0], 1.5f, EViewTargetBlendFunction::VTBlend_Cubic);
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
	else
	{
		if (URelicRunnersGameInstance* GI = GetWorld()->GetGameInstance<URelicRunnersGameInstance>())
		{
			if (GI->SelectedSaveSlot)
			{
				Server_InitializePlayerData(GI->SelectedSaveSlot->SaveData);
			}
			else
			{
				//Testing path: Send a default/empty struct
				UE_LOG(LogTemp, Warning, TEXT("SelectedSaveSlot is NULL. Starting with default testing stats."));
				FSaveData DefaultData;
				Server_InitializePlayerData(DefaultData);
			}
		}
	}

	if (PlayerCameraManager)
	{
		PlayerCameraManager->ViewPitchMin = -60.f;
		PlayerCameraManager->ViewPitchMax = 10.f;
	}
}

void ARelicRunnersPlayerController::Client_StartCinematicWalk_Implementation(FVector TargetLocation)
{
	CinematicTargetLocation = TargetLocation;

	SetGameplayUIActive(false);

	FInputModeUIOnly UIOnlyMode;
	UIOnlyMode.SetLockMouseToViewportBehavior(EMouseLockMode::LockAlways);
	SetInputMode(UIOnlyMode);
	SetIgnoreMoveInput(true);

	Server_ExecuteMove(TargetLocation);

	GetWorldTimerManager().SetTimer(CinematicTimerHandle, this, &ARelicRunnersPlayerController::CheckCinematicArrival, 0.1f, true);
}

void ARelicRunnersPlayerController::Server_ExecuteMove_Implementation(FVector Target)
{
	UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
	if (NavSys)
	{
		UAIBlueprintHelperLibrary::SimpleMoveToLocation(this, Target);
	}
}

void ARelicRunnersPlayerController::RestoreGameAndUI()
{
	if (FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().ReleaseAllPointerCapture();
		FSlateApplication::Get().ClearUserFocus(0);
	}

	FInputModeGameAndUI InputMode;
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::LockOnCapture);
	InputMode.SetHideCursorDuringCapture(false);

	SetInputMode(InputMode);

	bShowMouseCursor = true;
	bEnableClickEvents = true;
	bEnableMouseOverEvents = true;

	SetIgnoreMoveInput(false);
	SetIgnoreLookInput(false);

	FSlateApplication::Get().SetAllUserFocusToGameViewport();
}

void ARelicRunnersPlayerController::CheckCinematicArrival()
{
	APawn* ControlledPawn = GetPawn();
	if (!ControlledPawn) return;

	float Distance = FVector::Dist2D(ControlledPawn->GetActorLocation(), CinematicTargetLocation);

	if (Distance < 150.0f)
	{
		GetWorldTimerManager().ClearTimer(CinematicTimerHandle);

		SetGameplayUIActive(true);
		RestoreGameAndUI();

		UE_LOG(LogTemp, Log, TEXT("Cinematic Walk Complete - Local Control Restored."));
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
		//EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ARelicRunnersPlayerController::Jump);
		//EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ARelicRunnersPlayerController::StopJumping);

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

		// HeavyAttack
		EnhancedInputComponent->BindAction(HeavyAttackAction, ETriggerEvent::Started, this, &ARelicRunnersPlayerController::HeavyAttack);
		EnhancedInputComponent->BindAction(HeavyAttackAction, ETriggerEvent::Completed, this, &ARelicRunnersPlayerController::ProjectileHeavyAttack);
		


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
//void ARelicRunnersPlayerController::DefenceAbility()
//{
//	if (PossessedPawn)
//	{
//		PossessedPawn->DefenceAbility();
//	}
//}

void ARelicRunnersPlayerController::DefenceAbility()
{
	if (!IsLocalController())
		return;

	if (ARelicRunnersCharacter* Char = Cast<ARelicRunnersCharacter>(GetPawn()))
	{
		Char->DefenceAbility();
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

void ARelicRunnersPlayerController::HeavyAttack()
{
	if (PossessedPawn)
	{
		PossessedPawn->HeavyAttack();
	}
}

void ARelicRunnersPlayerController::ProjectileHeavyAttack()
{
	if (PossessedPawn)
	{
		PossessedPawn->ProjectileHeavyAttack();
	}
}

//void ARelicRunnersPlayerController::Jump(const FInputActionValue& Value)
//{
//	if (PossessedPawn)
//	{
//		PossessedPawn->Jump();
//	}
//}

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

