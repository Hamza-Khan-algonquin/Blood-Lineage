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

#include "RelicRunnersCharacter.h"

#include "PlayerHUD/PlayerHUD.h"
#include "PlayerHUD/PlayerHUDWorld.h"
#include "RelicRunners/LevelUpHUD/LevelUpHUD.h"
#include "PlayerPreview/PlayerPreview.h"
#include "PlayerState/RelicRunnersPlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/TileView.h"
#include "Components/TextBlock.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "PlayerController/RelicRunnersPlayerController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "EngineUtils.h"
#include "Inventory/Inventory.h"
#include "Inventory/InventoryComponent.h"
#include "Inventory/InventoryToolTip.h"
#include "Inventory/InventoryItemOptions.h"
#include "Inventory/InventorySortingOptions.h"
#include "Inventory/InventoryDragOperation.h"
#include "Item/ItemStats.h"
#include "Item/ItemActor.h"
#include "RelicRunners/Vendor/Vendor.h"
#include "Interact/InteractInterface.h"
#include "Menu/PauseMenu.h"
#include "AbilitySystem/AbilityPointCounter.h"
#include "AbilitySystem/AbilitySelection.h"
#include "AbilitySystem/HealthPotion.h"
#include "Item/ItemData.h"
#include "Engine/ActorChannel.h"
#include "Engine/LocalPlayer.h"
#include "Engine/Engine.h"
#include "Game/RelicRunnersGameInstance.h"
#include "Director System/Director.h"
#include "Enemy/EnemyCharacter.h"
#include "Enemy/EnemyCharacterAI.h"

#include "LevelGeneration/CapturableFlag.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

ARelicRunnersCharacter::ARelicRunnersCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Mesh attachments
	UpperMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("UpperMesh"));
	UpperMesh->SetupAttachment(GetMesh());
	UpperMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	UpperMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	UpperMesh->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
	UpperMesh->SetLeaderPoseComponent(GetMesh());
	// Mesh attachments
	LowerMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("LowerMesh"));
	LowerMesh->SetupAttachment(GetMesh());
	LowerMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	LowerMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	LowerMesh->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
	LowerMesh->SetLeaderPoseComponent(GetMesh());
	// Mesh attachments
	ArmsMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("ArmsMesh"));
	ArmsMesh->SetupAttachment(GetMesh());
	ArmsMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ArmsMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	ArmsMesh->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
	ArmsMesh->SetLeaderPoseComponent(GetMesh());
	// Mesh attachments
	HelmetMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("HelmetMesh"));
	HelmetMesh->SetupAttachment(GetMesh());
	HelmetMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	HelmetMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	HelmetMesh->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
	HelmetMesh->SetLeaderPoseComponent(GetMesh());
	// Mesh attachments
	NecklaceMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("NecklaceMesh"));
	NecklaceMesh->SetupAttachment(GetMesh());
	NecklaceMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	NecklaceMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	NecklaceMesh->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
	NecklaceMesh->SetLeaderPoseComponent(GetMesh());
	// Mesh attachments
	MainhandItemMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MainhandMesh"));
	MainhandItemMesh->SetupAttachment(GetMesh(), TEXT("MainhandSocket"));
	MainhandItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MainhandItemMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	MainhandItemMesh->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
	// Mesh attachments
	OffhandItemMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("OffhandMesh"));
	OffhandItemMesh->SetupAttachment(GetMesh(), TEXT("OffhandSocket"));
	OffhandItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	OffhandItemMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	OffhandItemMesh->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);

	//UI
	PlayerHUDWorld = CreateDefaultSubobject<UWidgetComponent>(TEXT("TooltipWidgetComponent"));
	PlayerHUDWorld->SetupAttachment(GetMesh());
	PlayerHUDWorld->SetWorldScale3D(FVector(0.1, 0.1, 0.1));
	PlayerHUDWorld->SetRelativeLocation(FVector(0.f, 0.f, 200.f));
	PlayerHUDWorld->SetWidgetSpace(EWidgetSpace::World);
	PlayerHUDWorld->SetDrawAtDesiredSize(true);
	PlayerHUDWorld->SetVisibility(true);
	PlayerHUDWorld->CastShadow = false;

	//Inventory
	InventoryComponent = CreateDefaultSubobject<UInventoryComponent>(TEXT("InventoryComponent"));
	InventoryComponent->SetIsReplicated(true);
	PlayerNumInventorySlots = 20;
	//Health
	PlayerStartingMaxHealth = 100;
	PlayerMaxHealth = PlayerStartingMaxHealth;
	PlayerHealth = PlayerMaxHealth;
	//Resource
	PlayerStartingMaxResource = 100;
	PlayerMaxResource = PlayerStartingMaxResource;
	PlayerResource = PlayerMaxResource;
	//Level
	PlayerLevel = 1;
	PlayerXP = 0;
	PlayerXPToLevel = 100;
	//Stats
	PlayerStartingArmor = 0;
	PlayerStartingDexterity = 0;
	PlayerStartingStrength = 0;
	PlayerStartingIntelligence = 0;
	PlayerArmor = 0;
	PlayerDexterity = 0;
	PlayerStrength = 0;
	PlayerIntelligence = 0;
	PlayerAbilityPoints = 2;
	//Gold
	PlayerStartingGold = 0;
	PlayerGold = PlayerStartingGold;
	//Potions
	HealthPotionCount = 3;
	HealthGranted = 0.5;

	bAlwaysRelevant = true;
	SetReplicates(true);
	SetReplicateMovement(true);

	Tags.Add("Player");

}

void ARelicRunnersCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	//stats
	DOREPLIFETIME(ARelicRunnersCharacter, PlayerHealth);
	DOREPLIFETIME(ARelicRunnersCharacter, PlayerMaxHealth);
	DOREPLIFETIME(ARelicRunnersCharacter, PlayerResource);
	DOREPLIFETIME(ARelicRunnersCharacter, PlayerMaxResource);
	DOREPLIFETIME(ARelicRunnersCharacter, PlayerLevel);
	DOREPLIFETIME(ARelicRunnersCharacter, PlayerXP);
	DOREPLIFETIME(ARelicRunnersCharacter, PlayerXPToLevel);
	DOREPLIFETIME(ARelicRunnersCharacter, PlayerAbilityPoints);
	DOREPLIFETIME(ARelicRunnersCharacter, PlayerGold);

	//equipped items
	DOREPLIFETIME(ARelicRunnersCharacter, ReplicatedArmsMesh);
	DOREPLIFETIME(ARelicRunnersCharacter, ReplicatedUpperMesh);
	DOREPLIFETIME(ARelicRunnersCharacter, ReplicatedLowerMesh);
	DOREPLIFETIME(ARelicRunnersCharacter, ReplicatedHelmetMesh);
	DOREPLIFETIME(ARelicRunnersCharacter, ReplicatedNecklaceMesh);
	DOREPLIFETIME(ARelicRunnersCharacter, ReplicatedMainhandMesh);
	DOREPLIFETIME(ARelicRunnersCharacter, ReplicatedOffhandMesh);
}

EInventorySorting ARelicRunnersCharacter::GetCurrentSortingMethod() const
{
	if (InventoryComponent)
	{
		return InventoryComponent->CurrentSortingMethod;
	}

	return EInventorySorting::SortByNewest;
}

void ARelicRunnersCharacter::Server_EquipItemByID_Implementation(FGuid ID)
{
	if (!InventoryComponent) return;

	UItemObject* Item = InventoryComponent->FindItemByID(ID);

	if (Item)
	{
		InventoryComponent->EquipItem(Item);
		UpdateItemVisuals(Item);
		InventoryComponent->UpdateTotalEquippedStats(this);
	}
}

void ARelicRunnersCharacter::Server_UnequipItemByID_Implementation(FGuid ID)
{
	if (!InventoryComponent) return;

	UItemObject* EquippedItem = nullptr;
	FString ItemType;

	for (FEquippedItemEntry& Entry : InventoryComponent->EquippedItems)
	{
		if (Entry.Item && Entry.Item->ItemData.UniqueID == ID)
		{
			EquippedItem = Entry.Item;
			ItemType = Entry.ItemType;
			break;
		}
	}

	if (EquippedItem)
	{
		InventoryComponent->UnequipItem(EquippedItem);
		UpdateItemVisuals(nullptr, ItemType);
		InventoryComponent->UpdateTotalEquippedStats(this);
	}

	InventoryComponent->SortInventoryByCurrentMethod();
}

void ARelicRunnersCharacter::Server_SetReplicatedMeshByItemType_Implementation(UObject* MeshAsset, const FString& ItemType)
{
	SetReplicatedMeshByItemType(MeshAsset, ItemType);
}

void ARelicRunnersCharacter::OnRep_PlayerGold()
{
	InventoryComponent->UpdateTotalEquippedStats(this);
}

void ARelicRunnersCharacter::OnRep_UpperMesh()
{
	if (UpperMesh) UpperMesh->SetSkeletalMesh(ReplicatedUpperMesh);
}

void ARelicRunnersCharacter::OnRep_LowerMesh()
{
	if (LowerMesh) LowerMesh->SetSkeletalMesh(ReplicatedLowerMesh);
}

void ARelicRunnersCharacter::OnRep_HelmetMesh()
{
	if (HelmetMesh) HelmetMesh->SetSkeletalMesh(ReplicatedHelmetMesh);
}

void ARelicRunnersCharacter::OnRep_ArmsMesh()
{
	if (ArmsMesh) ArmsMesh->SetSkeletalMesh(ReplicatedArmsMesh);
}

void ARelicRunnersCharacter::OnRep_NecklaceMesh()
{
	if (NecklaceMesh) NecklaceMesh->SetSkeletalMesh(ReplicatedNecklaceMesh);
}

void ARelicRunnersCharacter::OnRep_MainhandMesh()
{
	if (MainhandItemMesh) MainhandItemMesh->SetStaticMesh(ReplicatedMainhandMesh);
}

void ARelicRunnersCharacter::OnRep_OffhandMesh()
{
	if (OffhandItemMesh) OffhandItemMesh->SetStaticMesh(ReplicatedOffhandMesh);
}

void ARelicRunnersCharacter::SetReplicatedMeshByItemType(UObject* MeshAsset, const FString& ItemType)
{

	if (!HasAuthority())
	{
		return;
	}

	if (ItemType == "Upper")
	{
		ReplicatedUpperMesh = Cast<USkeletalMesh>(MeshAsset);
		OnRep_UpperMesh();
	}
	else if (ItemType == "Lower")
	{
		ReplicatedLowerMesh = Cast<USkeletalMesh>(MeshAsset);
		OnRep_LowerMesh();
	}
	else if (ItemType == "Helmet")
	{
		ReplicatedHelmetMesh = Cast<USkeletalMesh>(MeshAsset);
		OnRep_HelmetMesh();
	}
	else if (ItemType == "Arms")
	{
		ReplicatedArmsMesh = Cast<USkeletalMesh>(MeshAsset);
		OnRep_ArmsMesh();
	}
	else if (ItemType == "Necklace")
	{
		ReplicatedNecklaceMesh = Cast<USkeletalMesh>(MeshAsset);
		OnRep_NecklaceMesh();
	}
	else if (ItemType == "Greatsword" || ItemType == "Bow" || ItemType == "Sword" || ItemType == "Staff")
	{
		ReplicatedMainhandMesh = Cast<UStaticMesh>(MeshAsset);
		OnRep_MainhandMesh();
	}
	else if (ItemType == "Offhand")
	{
		ReplicatedOffhandMesh = Cast<UStaticMesh>(MeshAsset);
		OnRep_OffhandMesh();
	}
}

void ARelicRunnersCharacter::OnRep_HUD()
{
	UpdateHUD();
}


void ARelicRunnersCharacter::AddExperience(int Amount)
{
	PlayerXP += Amount;
	if (PlayerXP >= PlayerXPToLevel)
	{
		OnLevelUp();
	}
	UpdateHUD();
}

void ARelicRunnersCharacter::OnLevelUp()
{
	PlayerXP -= PlayerXPToLevel;
	PlayerLevel++;
	PlayerXPToLevel = PlayerLevel * 100;

	//starting stats / base stats
	PlayerStartingMaxHealth++;
	PlayerStartingMaxResource++;
	PlayerStartingArmor++;
	PlayerStartingDexterity++;
	PlayerStartingStrength++;
	PlayerStartingIntelligence++;

	//current health / max health
	PlayerMaxHealth++;
	PlayerHealth++;

	//current resource / max resource
	PlayerMaxResource++;
	PlayerResource++;

	//current stats
	PlayerArmor++;
	PlayerDexterity++;
	PlayerStrength++;
	PlayerIntelligence++;
	PlayerNumInventorySlots++;

	//ability points
	PlayerAbilityPoints++;

	InventoryComponent->UpdateTotalEquippedStats(this);

	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC) return;

	if (LevelUpHUDClass)
	{
		ULevelUpHUD* NewLevelUpHUD = CreateWidget<ULevelUpHUD>(PC, LevelUpHUDClass);
		if (NewLevelUpHUD)
		{
			NewLevelUpHUD->AddToViewport();
		}
	}

	if(Inventory) Inventory->RefreshEquippedUI();
}

float ARelicRunnersCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	float actualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	PlayerHealth -= actualDamage;

	if (PlayerHealth <= 0)
	{
		PlayerHealth = 0;
		Die();
	}

	UpdateHUD();

	return actualDamage;
}

void ARelicRunnersCharacter::Die()
{
	SetActorLocation(respawnPoint);
	PlayerHealth = PlayerMaxHealth;
}

bool ARelicRunnersCharacter::ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags)
{
	bool WroteSomething = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);

	if (InventoryComponent)
	{
		WroteSomething |= Channel->ReplicateSubobject(InventoryComponent, *Bunch, *RepFlags);
	}

	return WroteSomething;
}

void ARelicRunnersCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	Super::OnRep_PlayerState();

	InitLocalUI();
	UpdateHUD();
}

void ARelicRunnersCharacter::OnRep_MeshUpdate(UObject* MeshAsset, const FString& ItemType)
{
	SetReplicatedMeshByItemType(MeshAsset, ItemType);
}

void ARelicRunnersCharacter::TryBindInventoryDelegates()
{
	if (!IsLocallyControlled()) return;

	if (Inventory && InventoryComponent)
	{
		Inventory->SetInventoryComponent(InventoryComponent);
		InventoryComponent->UpdateTotalEquippedStats(this);
		GetWorldTimerManager().ClearTimer(DelegateBindHandle);

		Inventory->RefreshInventoryUI();
		Inventory->RefreshEquippedUI();

		UE_LOG(LogTemp, Warning, TEXT("[UI] Bound inventory component delegates successfully."));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[UI] Waiting for Inventory or InventoryComponent..."));
	}
}

void ARelicRunnersCharacter::BeginPlay()
{
	Super::BeginPlay();

	// Always load ItemMeshData locally
	if (ItemMeshDataClass)
	{
		ItemMeshData = ItemMeshDataClass->GetDefaultObject<UItemMeshData>();
	}

	// === Local client UI setup ===
	APlayerController* PC = Cast<APlayerController>(GetController());
	if (PC && PC->IsLocalController())
	{
		// Delay UI setup until everything else is ready
		GetWorld()->GetTimerManager().SetTimerForNextTick(this, &ARelicRunnersCharacter::InitLocalUI);
	}

	// Attach item mesh components
	MainhandItemMesh->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, TEXT("MainhandSocket"));
	OffhandItemMesh->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, TEXT("OffhandSocket"));

	if (PlayerHUDWorld)
	{
		if (UPlayerHUDWorld* HUDWorldWidget = Cast<UPlayerHUDWorld>(PlayerHUDWorld->GetUserWidgetObject()))
		{
			HUDWorldWidget->InitWidgetWithCharacter(this);
		}
	}

	if (HasAuthority())
	{
		// Generate initial items only once on server
		GetWorld()->GetTimerManager().SetTimerForNextTick(this, &ARelicRunnersCharacter::SpawnStarterItems);

		// Setup health regen loop
		//GetWorld()->GetTimerManager().SetTimer(HealthRegenTimerHandle, this, &ARelicRunnersCharacter::PassiveHealthRegen, 3.0f, true);

		UWorld* World = GetWorld();
		APawn* player = static_cast<APawn*>(this);

		GetWorld()->GetTimerManager().SetTimerForNextTick([World, player] {
			ADirector* Director = static_cast<ADirector*>(UGameplayStatics::GetActorOfClass(World, ADirector::StaticClass()));

			if (Director != nullptr)
			{
				Director->AddPlayer(player);
				player->OnDestroyed.AddDynamic(Director, &ADirector::RemoveEnemy);
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("missing director"));
			}
		});
	}

	respawnPoint = GetActorLocation();

}

void ARelicRunnersCharacter::InitLocalUI()
{
	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC) return;

	// === HUD Widget ===
	if (!PlayerHUD && PlayerHUDClass)
	{
		PlayerHUD = CreateWidget<UPlayerHUD>(PC, PlayerHUDClass);
		if (PlayerHUD)
		{
			PlayerHUD->AddToViewport();
		}
	}
	// === Ability Points Widget ===
	if (!AbilityPointCounter && AbilityPointCounterClass)
	{
		AbilityPointCounter = CreateWidget<UAbilityPointCounter>(PC, AbilityPointCounterClass);
		if (AbilityPointCounter)
		{
			AbilityPointCounter->AddToViewport();
			AbilityPointCounter->SetVisibility(ESlateVisibility::Visible);
		}
	}
	// === Health Potion Widget ===
	if (!HealthPotion && HealthPotionClass)
	{
		HealthPotion = CreateWidget<UHealthPotion>(PC, HealthPotionClass);
		if (HealthPotion)
		{
			HealthPotion->AddToViewport();
			HealthPotion->SetVisibility(ESlateVisibility::Visible);
		}
	}
	// === Inventory Widget ===
	if (!Inventory && InventoryClass)
	{
		Inventory = CreateWidget<UInventory>(PC, InventoryClass);
		if (Inventory)
		{
			Inventory->AddToViewport();
			Inventory->SetVisibility(ESlateVisibility::Hidden);
			Inventory->SetIsEnabled(false);
		}
	}
	// === Pause Widget ===
	if (!PauseMenu && PauseMenuClass)
	{
		PauseMenu = CreateWidget<UPauseMenu>(PC, PauseMenuClass);
		if (PauseMenu)
		{
			PauseMenu->AddToViewport();
			PauseMenu->SetVisibility(ESlateVisibility::Hidden);
			PauseMenu->SetIsEnabled(false);
		}
	}
	// === Ability Selection HUD Widget ===
	if (!AbilitySelection && AbilitySelectionClass)
	{
		{
			AbilitySelection = CreateWidget<UAbilitySelection>(PC, AbilitySelectionClass);
			if (AbilitySelection)
			{
				AbilitySelection->AddToViewport();
				AbilitySelection->SetVisibility(ESlateVisibility::Hidden);
				AbilitySelection->SetIsEnabled(false);
			}
		}
	}

	TryBindInventoryDelegates();
}

void ARelicRunnersCharacter::SpawnStarterItems()
{
	if (!ItemMeshData || !InventoryComponent) return;

	for (int i = 0; i < 50; i++)
	{
		FString RandomType = ItemStats::GetRandomItemType();
		FItemData ItemData = ItemStats::CreateSpecificItemData(1, RandomType, ItemMeshData);
		UItemObject* Item = ItemStats::CreateItemFromData(ItemData, InventoryComponent);
		if (!Item) continue;

		InventoryComponent->AddItem(Item);
	}
}

void ARelicRunnersCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	TraceForInteractables();
	UpdatePlayerHUDWorldFacing();

	if (GetActorLocation().Z <= -100.0f)
	{
		Die();
		UpdateHUD();
	}
}

void ARelicRunnersCharacter::TraceForInteractables()
{
	if (!IsLocallyControlled()) return;

	const FVector PlayerLocation = GetActorLocation();
	const FVector PlayerForward = FollowCamera->GetForwardVector();

	const float MaxDistance = 800.f;
	const float MinFacingDot = 0.f; // 1 = perfectly facing, 0 = 90 degrees off

	for (TActorIterator<AItemActor> It(GetWorld()); It; ++It)
	{
		AItemActor* Item = *It;
		if (!Item || !Item->Implements<UInteractInterface>())
		{
			continue;
		}

		FVector ToItem = Item->GetActorLocation() - PlayerLocation;
		float Distance = ToItem.Size();

		bool bWithinDistance = Distance <= MaxDistance;
		bool bFacingItem = FVector::DotProduct(PlayerForward, ToItem.GetSafeNormal()) > MinFacingDot;

		bool bShouldShow = bWithinDistance && bFacingItem;
		IInteractInterface::Execute_ShowTooltip(Item, bShouldShow);
	}

}

void ARelicRunnersCharacter::UpdatePlayerHUDWorldFacing()
{
	if (!PlayerHUDWorld || !PlayerHUDWorld->GetWidget()) return;

	APlayerController* LocalController = UGameplayStatics::GetPlayerController(this, 0);
	if (!LocalController || !LocalController->PlayerCameraManager) return;

	FVector CameraLocation = LocalController->PlayerCameraManager->GetCameraLocation();
	FVector WidgetLocation = PlayerHUDWorld->GetComponentLocation();

	if (IsLocallyControlled())
	{
		PlayerHUDWorld->SetVisibility(false);
	}
	else 
	{
		FVector ToCamera = CameraLocation - WidgetLocation;
		ToCamera.Z = 0.f;

		if (!ToCamera.IsNearlyZero())
		{
			FRotator LookAtRotation = ToCamera.Rotation();
			PlayerHUDWorld->SetWorldRotation(LookAtRotation);
		}
	}
}

void ARelicRunnersCharacter::PassiveHealthRegen()
{
	if (PlayerHealth < PlayerMaxHealth)
	{
		PlayerHealth += (int)(PlayerMaxHealth * 0.05f);
	}
	if (PlayerHealth > PlayerMaxHealth)
	{
		PlayerHealth = PlayerMaxHealth;
	}
	UpdateHUD();
}

void ARelicRunnersCharacter::Client_UpdateEquippedStats_Implementation(const FEquippedStatsSummary& NewStats)
{
	if (InventoryComponent)
	{
		InventoryComponent->CachedEquippedStats = NewStats;
		InventoryComponent->OnStatsChanged.Broadcast(NewStats);
	}
}

void ARelicRunnersCharacter::PerformSwingTrace()
{
	FVector Start = GetMesh()->GetSocketLocation("MainhandSocket");
	FVector End = GetMesh()->GetSocketLocation("TipSocket");

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	TArray<FHitResult> Hits;
	GetWorld()->SweepMultiByChannel(
		Hits,
		Start,
		End,
		FQuat::Identity,
		ECC_Pawn,
		FCollisionShape::MakeSphere(100.0f),
		Params
	);

	AlreadyHitActors.Empty();

	for (const FHitResult& Hit : Hits)
	{
		//DrawDebugSphere(GetWorld(), Hit.ImpactPoint, 10.f, 12, FColor::Red, false, 1.0f);
		if (AActor* HitActor = Hit.GetActor())
		{
			if (!AlreadyHitActors.Contains(HitActor))
			{
				// Check if it's an enemy before applying damage
				if (AEnemyCharacter* Enemy = Cast<AEnemyCharacter>(HitActor))
				{
					Server_HitEnemy(Enemy);
				}
				else if (AEnemyCharacterAI* TEnemy = Cast<AEnemyCharacterAI>(HitActor))
				{
					Server_HitEnemy(TEnemy);
				}

				AlreadyHitActors.Add(HitActor);
			}
		}
	}

	//DrawDebugLine(GetWorld(), Start, End, FColor::Cyan, false, 0.1f, 0, 2.0f);

}

void ARelicRunnersCharacter::Server_HitEnemy_Implementation(AActor* HitActor)
{
	if (HitActor)
	{
		UGameplayStatics::ApplyDamage(HitActor, 5.f, GetController(), this, nullptr);
	}
}

void ARelicRunnersCharacter::BasicAttack()
{
	UE_LOG(LogTemp, Warning, TEXT("Attacking"));

	if (!Inventory || !InventoryComponent)
	{
		return;
	}
	if (InventoryComponent->GetEquippedItemByType("Greatsword")
		|| InventoryComponent->GetEquippedItemByType("Sword")
		|| InventoryComponent->GetEquippedItemByType("Staff")
		|| InventoryComponent->GetEquippedItemByType("Bow"))
	{
		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		if (!AnimInstance) return;

		TArray<UAnimSequence*> SwordAttacks;
		if (SwordBasicAttack0) SwordAttacks.Add(SwordBasicAttack0);
		if (SwordBasicAttack1) SwordAttacks.Add(SwordBasicAttack1);
		if (SwordBasicAttack2) SwordAttacks.Add(SwordBasicAttack2);
		if (SwordBasicAttack3) SwordAttacks.Add(SwordBasicAttack3);

		if (SwordAttacks.Num() > 0)
		{
			int32 Index = FMath::RandRange(0, SwordAttacks.Num() - 1);
			UAnimSequence* SelectedSequence = SwordAttacks[Index];

			// Play as dynamic montage
			AnimInstance->PlaySlotAnimationAsDynamicMontage(SelectedSequence, FName("DefaultSlot"));
		}
	}
}

void ARelicRunnersCharacter::MoveInDirection(EAxis::Type Axis, float Value)
{
	if (Controller && Value != 0.0f)
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		if (Axis == EAxis::X)
		{
			const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
			AddMovementInput(Direction, Value);
		}
		else if (Axis == EAxis::Y)
		{
			const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
			AddMovementInput(Direction, Value);
		}
	}
}

void ARelicRunnersCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void ARelicRunnersCharacter::Interact()
{
	//FVector Start = FollowCamera->GetComponentLocation();
	FVector Start = GetActorLocation();
	FVector End = Start + (FollowCamera->GetForwardVector() * 500.f); // Match TraceForInteractables

	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	DrawDebugSphere(GetWorld(), End, 40.f, 12, FColor::Orange, false, 0.1f);
	if (GetWorld()->SweepSingleByChannel(Hit, Start, End, FQuat::Identity, ECC_Visibility, FCollisionShape::MakeSphere(40.f), Params))
	{
		DrawDebugSphere(GetWorld(), Hit.Location, 40.f, 12, FColor::Orange, false, 0.1f);
		AActor* HitActor = Hit.GetActor();
		if (HitActor && HitActor->Implements<UInteractInterface>())
		{
			// Call Interact on the hit actor
			if (AItemActor* Item = Cast<AItemActor>(HitActor))
			{
				if (APlayerController* PC = Cast<APlayerController>(GetController()))
				{
					ARelicRunnersPlayerController* MyPC = Cast<ARelicRunnersPlayerController>(PC);
					if (MyPC)
					{
						MyPC->Server_RequestPickup(Item);
					}
				}
			}
			else if (ACapturableFlag* Flag = Cast<ACapturableFlag>(HitActor))
			{
				if (APlayerController* PC = Cast<APlayerController>(GetController()))
				{
					ARelicRunnersPlayerController* MyPC = Cast<ARelicRunnersPlayerController>(PC);
					if (MyPC)
					{
						MyPC->Server_RequestFlagActivation(Flag);
					}
				}

			}
			else
			{
				IInteractInterface::Execute_Interact(HitActor, this);
			}
		}
	}
}

void ARelicRunnersCharacter::ToggleUI(UUserWidget* UIWidget)
{
	if (!IsLocallyControlled() || !UIWidget) return;

	APlayerController* PlayerController = Cast<APlayerController>(Controller);
	if (!PlayerController) return;

	bool bIsVisible = UIWidget->IsVisible();
	ESlateVisibility Visibility = bIsVisible ? ESlateVisibility::Hidden : ESlateVisibility::Visible;
	bool bEnable = !bIsVisible;

	UIWidget->SetVisibility(Visibility);
	UIWidget->SetIsEnabled(bEnable);
	if (bEnable)
	{
		FInputModeGameAndUI InputMode;
		PlayerController->SetInputMode(InputMode);
	}
	else
	{
		FInputModeGameOnly InputMode;
		PlayerController->SetInputMode(InputMode);
	}
	PlayerController->SetShowMouseCursor(bEnable);

	if (UIWidget == Inventory)
	{
		UInventoryItemOptions::CloseAnyOpenPopup();
		UInventorySortingOptions::CloseAnyOpenPopup();
	}
}

void ARelicRunnersCharacter::InventoryUI()
{
	if (!Inventory) return;
	RemoveOtherUI("Inventory", Cast<APlayerController>(Controller));
	ToggleUI(Inventory);
	FSlateApplication::Get().CancelDragDrop();
	Inventory->ToggleVendorUI(false);
}

void ARelicRunnersCharacter::PauseUI()
{
	if (!PauseMenu) return;
	RemoveOtherUI("Pause", Cast<APlayerController>(Controller));
	ToggleUI(PauseMenu);
}

void ARelicRunnersCharacter::AbilitySystemUI()
{
	if (!AbilitySelection || PlayerAbilityPoints < 1) return;
	RemoveOtherUI("Ability", Cast<APlayerController>(Controller));
	ToggleUI(AbilitySelection);
}

void ARelicRunnersCharacter::HideUI(UUserWidget* UIWidget, APlayerController* PlayerController, bool bClosePopups = false)
{
	if (UIWidget->IsVisible())
	{
		UIWidget->SetVisibility(ESlateVisibility::Hidden);
		UIWidget->SetIsEnabled(false);
		PlayerController->SetInputMode(FInputModeGameOnly());
		PlayerController->SetShowMouseCursor(false);

		if (bClosePopups && UIWidget == Inventory)
		{
			UInventoryItemOptions::CloseAnyOpenPopup();
			UInventorySortingOptions::CloseAnyOpenPopup();
		}
	}
}

void ARelicRunnersCharacter::RemoveOtherUI(FString UI, APlayerController* playerController)
{
	if (UI != "Ability") HideUI(AbilitySelection, playerController);
	if (UI != "Inventory") HideUI(Inventory, playerController, true);
	if (UI != "Pause") HideUI(PauseMenu, playerController);
}

void ARelicRunnersCharacter::DamageAbility()
{
	GiveDamageAbilities();
}

void ARelicRunnersCharacter::DefenceAbility()
{
	GiveDefenceAbilities();
}

void ARelicRunnersCharacter::UtilityAbility()
{
	GiveUtilityAbilities();
}

void ARelicRunnersCharacter::UltimateAbility()
{
	GiveUltimateAbilities();
}

void ARelicRunnersCharacter::GiveDamageAbilities()
{

}

void ARelicRunnersCharacter::GiveDefenceAbilities()
{

}

void ARelicRunnersCharacter::GiveUtilityAbilities()
{

}

void ARelicRunnersCharacter::GiveUltimateAbilities()
{

}

void ARelicRunnersCharacter::HealthPotions()
{
	if (HealthPotion)
	{
		int OldHealth = PlayerHealth;
		int OldPotionCount = HealthPotionCount;

		HealthPotion->OnHealthPotionClicked(PlayerHealth, PlayerMaxHealth, HealthPotionCount, HealthGranted);

		if (PlayerHealth != OldHealth || HealthPotionCount != OldPotionCount)
		{
			Server_UseHealthPotion(PlayerHealth, HealthPotionCount);
		}

		UpdateHUD();
	}
}

void ARelicRunnersCharacter::Server_UseHealthPotion_Implementation(int NewHealth, int NewPotionCount)
{

	if (!HasAuthority())
		return;

	PlayerHealth = FMath::Clamp(NewHealth, 0, PlayerMaxHealth);
	HealthPotionCount = FMath::Max(NewPotionCount, 0);

	UpdateHUD();
}


void ARelicRunnersCharacter::Server_SpendAbilityPoints_Implementation()
{
	if (!HasAuthority())
		return;

	if (PlayerAbilityPoints > 0)
	{
		PlayerAbilityPoints--;

		Client_OnAbilityPointsUpdated(PlayerAbilityPoints);

		UpdateHUD();
	}
}

void ARelicRunnersCharacter::Client_OnAbilityPointsUpdated_Implementation(int32 NewAbilityPoints)
{
	APlayerController* PlayerController = Cast<APlayerController>(Controller);
	if (!PlayerController)
		return;

	// Update UI locally on the client
	if (NewAbilityPoints == 0 && AbilitySelection)
	{
		AbilitySelection->SetVisibility(ESlateVisibility::Hidden);
		AbilitySelection->SetIsEnabled(false);
		PlayerController->SetInputMode(FInputModeGameOnly());
		PlayerController->SetShowMouseCursor(false);
	}
}

void ARelicRunnersCharacter::Server_SetMaxHealth_Implementation(int health)
{
	PlayerMaxHealth = health;
	UpdateHUD();
}

void ARelicRunnersCharacter::AddGold(int Value)
{
	Server_AddGold(Value);
}

void ARelicRunnersCharacter::Server_AddGold_Implementation(int Value)
{
	PlayerGold += Value;
	InventoryComponent->UpdateTotalEquippedStats(this);
}

bool ARelicRunnersCharacter::CheckEnoughGold(int Value)
{
	if (PlayerGold >= Value)
	{
		PlayerGold -= Value;
		InventoryComponent->UpdateTotalEquippedStats(this);
		return true;
	}
	return false;
}

void ARelicRunnersCharacter::Server_BuyItem_Implementation(const FItemData& ItemData, AVendor* Vendor)
{
	if (!Vendor || !InventoryComponent) return;

	if (!CheckEnoughGold(ItemData.Gold)) return;

	UItemObject* NewItem = ItemStats::CreateItemFromData(ItemData, InventoryComponent);
	NewItem->IsNewItem = true;
	InventoryComponent->AddItem(NewItem);
	Vendor->RemoveStock(ItemData);

	Client_UpdateVendorUI(Vendor);
}

void ARelicRunnersCharacter::Server_SellItem_Implementation(const FItemData& ItemData, AVendor* Vendor)
{
	if (!Vendor || !InventoryComponent) return;

	//Unequip if equipped
	UItemObject* EquippedItem = InventoryComponent->GetEquippedItemReference(ItemData.ItemType);
	if (EquippedItem && EquippedItem->ItemData.UniqueID == ItemData.UniqueID)
	{
		InventoryComponent->UnequipItem(EquippedItem);
	}

	InventoryComponent->RemoveItemByID(ItemData.UniqueID);
	AddGold(ItemData.Gold);
	Vendor->AddStock(ItemData);

	Client_UpdateVendorUI(Vendor);
}

void ARelicRunnersCharacter::Client_UpdateVendorUI_Implementation(AVendor* Vendor)
{
	if (Inventory)
	{
		Inventory->DisplaySelectedVendorItems(Vendor->GetStock(), Vendor);
	}
}

void  ARelicRunnersCharacter::SetInventoryComponent(UInventoryComponent* InvComp)
{ 
	InventoryComponent = InvComp; 
}

void ARelicRunnersCharacter::Server_DropItem_Implementation(const FItemData& Data)
{
	if (!HasAuthority() || !InventoryComponent) return;

	bool FoundAndRemoved = false;

	//Remove from Inventory
	for (int i = 0; i < InventoryComponent->InventoryItems.Num(); i++)
	{
		if (InventoryComponent->InventoryItems[i]->GetUniqueID() == Data.UniqueID)
		{
			InventoryComponent->InventoryItems.RemoveAt(i);
			FoundAndRemoved = true;
			break;
		}
	}

	//Remove from Equipped
	if (!FoundAndRemoved)
	{
		for (int i = 0; i < InventoryComponent->EquippedItems.Num(); i++)
		{
			if (InventoryComponent->EquippedItems[i].Item &&
				InventoryComponent->EquippedItems[i].Item->GetUniqueID() == Data.UniqueID)
			{
				InventoryComponent->EquippedItems.RemoveAt(i);
				FoundAndRemoved = true;
				break;
			}
		}
	}

	if (!FoundAndRemoved)
	{
		UE_LOG(LogTemp, Warning, TEXT("Could not find item to drop: %s"), *Data.Name);
		return;
	}

	//Desired drop location
	FVector SpawnLoc = GetActorLocation() + GetActorForwardVector() * 100.f;
	const float MinDistance = 50.f;

	//Get all items in the world
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AItemActor::StaticClass(), FoundActors);

	//Handle overlap resolution for existing nearby items
	for (AActor* Actor : FoundActors)
	{
		AItemActor* OtherItem = Cast<AItemActor>(Actor);
		if (!IsValid(Actor)) continue;

		FVector OtherLoc = OtherItem->GetActorLocation();
		if (FVector::Dist(OtherLoc, SpawnLoc) < MinDistance)
		{
			bool Moved = false;

			// Try circular spreading from original location
			for (int Radius = 50; Radius <= 300; Radius += 50)
			{
				for (int Angle = 0; Angle < 360; Angle += 45)
				{
					const float Radians = FMath::DegreesToRadians(Angle);
					FVector Offset = FVector(FMath::Cos(Radians), FMath::Sin(Radians), 0.f) * Radius;
					FVector NewLoc = OtherLoc + Offset;

					// Check if new position is too close to any other item
					bool bTooClose = false;
					for (AActor* CheckActor : FoundActors)
					{
						if (CheckActor != OtherItem && FVector::Dist(NewLoc, CheckActor->GetActorLocation()) < MinDistance)
						{
							bTooClose = true;
							break;
						}
					}

					if (!bTooClose)
					{
						OtherItem->SetActorLocation(NewLoc);
						Moved = true;
						break;
					}
				}
				if (Moved) break;
			}

			if (!Moved)
			{
				UE_LOG(LogTemp, Warning, TEXT("Could not reposition item: %s"), *OtherItem->GetName());
			}
		}
	}

	// Spawn new dropped item
	FActorSpawnParameters Params;
	Params.Owner = this;
	Params.Instigator = this;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	AItemActor* Dropped = GetWorld()->SpawnActor<AItemActor>(ItemActorClass, SpawnLoc, FRotator::ZeroRotator, Params);
	if (Dropped)
	{
		Dropped->Initialize(Data);
		Dropped->SetReplicates(true);
		Dropped->SetReplicateMovement(true);
	}

	InventoryComponent->UpdateTotalEquippedStats(this);
}

void ARelicRunnersCharacter::Server_PickupItem_Implementation(AItemActor* TargetItem)
{
	if (TargetItem)
	{
		TargetItem->Server_HandlePickup(this);
	}
}

void ARelicRunnersCharacter::UpdateItemVisuals(UObject* MeshAsset, const FString& ItemType)
{
	if (ItemType == "Greatsword" || ItemType == "Sword" || ItemType == "Bow" || ItemType == "Staff" || ItemType == "Offhand")
	{
		if (UStaticMeshComponent* TargetMesh = GetStaticMeshComponentByItemType(ItemType))
		{
			TargetMesh->SetStaticMesh(Cast<UStaticMesh>(MeshAsset));
			OnRep_MeshUpdate(MeshAsset, ItemType);
		}
	}
	else
	{
		if (USkeletalMeshComponent* TargetMesh = GetSkeletalMeshComponentByItemType(ItemType))
		{
			TargetMesh->SetSkeletalMesh(Cast<USkeletalMesh>(MeshAsset));
			OnRep_MeshUpdate(MeshAsset, ItemType);
		}
	}
}

void ARelicRunnersCharacter::UpdateItemVisuals(UItemObject* Item)
{
	if (!Item) return;

	const FString& ItemType = Item->ItemData.ItemType;
	const FString MeshPath = GetMeshPathByItemType(ItemType);

	if (ItemType == "Greatsword" || ItemType == "Sword" || ItemType == "Staff" || ItemType == "Bow")
	{
		if (UStaticMesh* LoadedMesh = LoadObject<UStaticMesh>(nullptr, *MeshPath))
		{
			if (UStaticMeshComponent* TargetMesh = GetStaticMeshComponentByItemType(ItemType))
			{
				TargetMesh->SetStaticMesh(LoadedMesh);
				OnRep_MeshUpdate(LoadedMesh, ItemType);
			}
		}
	}
	else
	{
		if (USkeletalMesh* LoadedMesh = LoadObject<USkeletalMesh>(nullptr, *MeshPath))
		{
			if (USkeletalMeshComponent* TargetMesh = GetSkeletalMeshComponentByItemType(ItemType))
			{
				TargetMesh->SetSkeletalMesh(LoadedMesh);
				OnRep_MeshUpdate(LoadedMesh, ItemType);
			}
		}
	}
}

void ARelicRunnersCharacter::UpdateHUD()
{
	if (PlayerHUD)
	{
		PlayerHUD->UpdateHUD(
			PlayerHealth,
			PlayerMaxHealth,
			PlayerResource,
			PlayerMaxResource,
			PlayerLevel,
			PlayerXP,
			PlayerXPToLevel
		);
	}

	if (AbilityPointCounter)
	{
		AbilityPointCounter->UpdateHUD(PlayerAbilityPoints);
	}
}

void ARelicRunnersCharacter::Multicast_UpdateHUD_Implementation()
{
	UpdateHUD();
}

FString ARelicRunnersCharacter::GetMeshPathByItemType(const FString& ItemType)
{
	return FString::Printf(TEXT("/Game/ThirdPerson/Meshes/Items/%s/%s.%s"),
		*ItemType, *ItemType, *ItemType);
}

USkeletalMeshComponent* ARelicRunnersCharacter::GetSkeletalMeshComponentByItemType(const FString& ItemType)
{
	if (ItemType == "Arms") return ArmsMesh;
	if (ItemType == "Helmet") return HelmetMesh;
	if (ItemType == "Lower") return LowerMesh;
	if (ItemType == "Upper") return UpperMesh;
	if (ItemType == "Necklace") return NecklaceMesh;
	return nullptr;
}

UStaticMeshComponent* ARelicRunnersCharacter::GetStaticMeshComponentByItemType(const FString& ItemType)
{
	if (ItemType == "Greatsword" || ItemType == "Sword" || ItemType == "Staff" || ItemType == "Bow") return MainhandItemMesh;
	if (ItemType == "Offhand") return OffhandItemMesh;
	return nullptr;
}
