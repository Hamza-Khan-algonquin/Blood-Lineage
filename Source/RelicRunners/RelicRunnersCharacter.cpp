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
#include "PlayerHUD/PlayerHUDWorld.h"
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
#include "Inventory/InventoryDragOperation.h"
#include "Item/ItemStats.h"
#include "Item/ItemActor.h"
#include "RelicRunners/Vendor/Vendor.h"
#include "Interact/InteractInterface.h"
#include "AbilitySystem/AbilitySelection.h"
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
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
	
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

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

	LowerMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("LowerMesh"));
	LowerMesh->SetupAttachment(GetMesh());
	LowerMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	LowerMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	LowerMesh->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
	LowerMesh->SetLeaderPoseComponent(GetMesh());

	ArmsMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("ArmsMesh"));
	ArmsMesh->SetupAttachment(GetMesh());
	ArmsMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ArmsMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	ArmsMesh->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
	ArmsMesh->SetLeaderPoseComponent(GetMesh());

	HelmetMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("HelmetMesh"));
	HelmetMesh->SetupAttachment(GetMesh());
	HelmetMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	HelmetMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	HelmetMesh->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
	HelmetMesh->SetLeaderPoseComponent(GetMesh());

	NecklaceMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("NecklaceMesh"));
	NecklaceMesh->SetupAttachment(GetMesh());
	NecklaceMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	NecklaceMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	NecklaceMesh->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
	NecklaceMesh->SetLeaderPoseComponent(GetMesh());

	MainhandItemMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MainhandMesh"));
	MainhandItemMesh->SetupAttachment(GetMesh(), TEXT("MainhandSocket"));
	MainhandItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MainhandItemMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	MainhandItemMesh->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);

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

	bAlwaysRelevant = true;
	SetReplicates(true);
	SetReplicateMovement(true);

	Tags.Add("Player");
}

void ARelicRunnersCharacter::BeginPlay()
{
	Super::BeginPlay();

	InitializeWeaponSockets();
	InitializeWorldHUD();

	//Server
	if (HasAuthority())
	{
		RegisterDirector();
	}

	respawnPoint = GetActorLocation();
}


void ARelicRunnersCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ARelicRunnersCharacter, ReplicatedArmsMesh);
	DOREPLIFETIME(ARelicRunnersCharacter, ReplicatedUpperMesh);
	DOREPLIFETIME(ARelicRunnersCharacter, ReplicatedLowerMesh);
	DOREPLIFETIME(ARelicRunnersCharacter, ReplicatedHelmetMesh);
	DOREPLIFETIME(ARelicRunnersCharacter, ReplicatedNecklaceMesh);
	DOREPLIFETIME(ARelicRunnersCharacter, ReplicatedMainhandMesh);
	DOREPLIFETIME(ARelicRunnersCharacter, ReplicatedOffhandMesh);
}

void ARelicRunnersCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	//Server
	RelicPlayerState = GetPlayerState<ARelicRunnersPlayerState>();
}

float ARelicRunnersCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (!HasAuthority()) return 0.0f;

	float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	if (RelicPlayerState) RelicPlayerState->ModifyHealth(-ActualDamage);

	return ActualDamage;
}

void ARelicRunnersCharacter::Die()
{
	SetActorLocation(respawnPoint);
	if (RelicPlayerState) RelicPlayerState->ResetHealth();
}

void ARelicRunnersCharacter::Server_EquipItemByID_Implementation(FGuid ID)
{
	if (RelicPlayerState) RelicPlayerState->Server_EquipItemByID(ID);
}

void ARelicRunnersCharacter::Server_UnequipItemByID_Implementation(FGuid ID)
{
	if (RelicPlayerState) RelicPlayerState->Server_UnequipItemByID(ID);
}

void ARelicRunnersCharacter::Server_SetReplicatedMeshByItemType_Implementation(UObject* MeshAsset, const FString& ItemType)
{
	SetReplicatedMeshByItemType(MeshAsset, ItemType);
}

void ARelicRunnersCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	//Client
	RelicPlayerState = GetPlayerState<ARelicRunnersPlayerState>();
}

void ARelicRunnersCharacter::OnRep_MeshUpdate(UObject* MeshAsset, const FString& ItemType)
{
	SetReplicatedMeshByItemType(MeshAsset, ItemType);
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

void ARelicRunnersCharacter::InitializeWeaponSockets()
{
	MainhandItemMesh->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, TEXT("MainhandSocket"));
	OffhandItemMesh->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, TEXT("OffhandSocket"));
}

void ARelicRunnersCharacter::InitializeWorldHUD()
{
	if (PlayerHUDWorld)
	{
		if (UPlayerHUDWorld* HUDWorldWidget = Cast<UPlayerHUDWorld>(PlayerHUDWorld->GetUserWidgetObject()))
		{
			HUDWorldWidget->InitWidgetWithCharacter(this);
		}
	}
}

void ARelicRunnersCharacter::RegisterDirector()
{
	GetWorldTimerManager().SetTimerForNextTick([this]()
		{
			ADirector* Director = Cast<ADirector>(UGameplayStatics::GetActorOfClass(GetWorld(), ADirector::StaticClass()));
			if (Director)
			{
				Director->AddPlayer(this);

				this->OnDestroyed.AddDynamic(Director, &ADirector::RemoveEnemy);
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("Character: Missing Director in World!"));
			}
		});
}

void ARelicRunnersCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	TraceForInteractables();
	UpdatePlayerHUDWorldFacing();

	if (GetActorLocation().Z <= -200.0f)
	{
		Die();
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

		if (!Item || !Item->Implements<UInteractInterface>()) continue;

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
		if (AActor* HitActor = Hit.GetActor())
		{
			if (!AlreadyHitActors.Contains(HitActor))
			{
				//Check if it's an enemy before applying damage
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
}

void ARelicRunnersCharacter::Server_HitEnemy_Implementation(AActor* HitActor)
{
	if (HitActor)
	{
		UGameplayStatics::ApplyDamage(HitActor, 5.f, GetController(), this, nullptr);
	}
}

void ARelicRunnersCharacter::PlayRandomAttackAnimation()
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
		int Index = FMath::RandRange(0, SwordAttacks.Num() - 1);
		AnimInstance->PlaySlotAnimationAsDynamicMontage(SwordAttacks[Index], FName("DefaultSlot"));
	}
}

bool ARelicRunnersCharacter::HasValidWeaponEquipped(UInventoryComponent* InvComp) const
{
	if (!InvComp) return false;

	const TArray<FString> WeaponTypes = { "Greatsword", "Sword", "Staff", "Bow" };

	for (const FString& Type : WeaponTypes)
	{
		if (InvComp->GetEquippedItemByType(Type))
		{
			return true;
		}
	}

	return false;
}

void ARelicRunnersCharacter::BasicAttack()
{
	if (!RelicPlayerState || !RelicPlayerState->GetInventoryComponent()) return;

	if (HasValidWeaponEquipped(RelicPlayerState->GetInventoryComponent()))
	{
		PlayRandomAttackAnimation();
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
	if (!IsLocallyControlled()) return;

	FVector Start = GetActorLocation();
	FVector End = Start + (FollowCamera->GetForwardVector() * 1000.f); //Match TraceForInteractables

	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	if (GetWorld()->SweepSingleByChannel(Hit, Start, End, FQuat::Identity, ECC_Visibility, FCollisionShape::MakeSphere(60.f), Params))
	{
		DrawDebugSphere(GetWorld(), Hit.Location, 60.f, 12, FColor::Orange, false, 0.1f);
		AActor* HitActor = Hit.GetActor();
		if (HitActor && HitActor->Implements<UInteractInterface>())
		{
			// Call Interact on the hit actor
			if (AItemActor* Item = Cast<AItemActor>(HitActor))
			{
				if (APlayerController* PC = Cast<APlayerController>(GetController()))
				{
					ARelicRunnersPlayerController* RPC = Cast<ARelicRunnersPlayerController>(PC);
					if (RPC)
					{
						RPC->Server_RequestPickup(Item);
						Item->SetActorEnableCollision(false);
						Item->SetActorHiddenInGame(true);
					}
				}
			}
			else if (ACapturableFlag* Flag = Cast<ACapturableFlag>(HitActor))
			{
				if (APlayerController* PC = Cast<APlayerController>(GetController()))
				{
					ARelicRunnersPlayerController* RPC = Cast<ARelicRunnersPlayerController>(PC);
					if (RPC)
					{
						RPC->Server_RequestFlagActivation(Flag);
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

void ARelicRunnersCharacter::Server_SpawnDroppedItemWorld_Implementation(const FItemData& Data)
{
	bool FoundAndRemoved = false;

	//Remove from Inventory
	for (int i = 0; i < RelicPlayerState->GetInventoryComponent()->InventoryItems.Num(); i++)
	{
		if (RelicPlayerState->GetInventoryComponent()->InventoryItems[i]->GetUniqueID() == Data.UniqueID)
		{
			RelicPlayerState->GetInventoryComponent()->InventoryItems.RemoveAt(i);
			FoundAndRemoved = true;
			break;
		}
	}

	//Remove from Equipped
	if (!FoundAndRemoved)
	{
		for (int i = 0; i < RelicPlayerState->GetInventoryComponent()->EquippedItems.Num(); i++)
		{
			if (RelicPlayerState->GetInventoryComponent()->EquippedItems[i].Item &&
				RelicPlayerState->GetInventoryComponent()->EquippedItems[i].Item->GetUniqueID() == Data.UniqueID)
			{
				RelicPlayerState->GetInventoryComponent()->EquippedItems.RemoveAt(i);
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


	FVector SpawnLoc = GetActorLocation() + GetActorForwardVector() * 100.f;
	FVector FinalLoc = CalculateValidDropLocation(SpawnLoc);

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

	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		ARelicRunnersPlayerController* RPC = Cast<ARelicRunnersPlayerController>(PC);
		if (RPC)
		{
			RPC->UpdatePreviewWithEquippedItems();
		}
	}
}

void ARelicRunnersCharacter::HealthPotions()
{
	if (RelicPlayerState)
	{
		RelicPlayerState->Server_UseHealthPotion();
	}
}

FVector ARelicRunnersCharacter::CalculateValidDropLocation(FVector DesiredLoc) const
{
	const float MinDistance = 50.f;
	TArray<AActor*> FoundItems;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AItemActor::StaticClass(), FoundItems);

	//If no items exist, the desired location is perfectly fine
	if (FoundItems.Num() == 0) return DesiredLoc;

	FVector CurrentBestLoc = DesiredLoc;
	bool NeedsRepositioning = false;

	//Check if our desired spot is blocked
	for (AActor* Item : FoundItems)
	{
		if (IsValid(Item) && FVector::Dist(Item->GetActorLocation(), DesiredLoc) < MinDistance)
		{
			NeedsRepositioning = true;
			break;
		}
	}

	if (!NeedsRepositioning) return DesiredLoc;

	//Spiral search
	for (int Radius = 60; Radius <= 300; Radius += 60)
	{
		for (int Angle = 0; Angle < 360; Angle += 45)
		{
			const float Radians = FMath::DegreesToRadians(Angle);
			FVector Offset(FMath::Cos(Radians) * Radius, FMath::Sin(Radians) * Radius, 0.f);
			FVector CandidateLoc = DesiredLoc + Offset;

			//Check candidate against all world items
			bool CollisionFound = false;
			for (AActor* Item : FoundItems)
			{
				if (FVector::Dist(CandidateLoc, Item->GetActorLocation()) < MinDistance)
				{
					CollisionFound = true;
					break;
				}
			}

			if (!CollisionFound)
			{
				return CandidateLoc;
			}
		}
	}

	//Fallback: If everywhere is crowded
	return DesiredLoc;
}

void ARelicRunnersCharacter::SyncEquippedVisuals(const TArray<FEquippedItemEntry>& EquippedEntries)
{
	for (const FEquippedItemEntry& Entry : EquippedEntries)
	{
		if (!Entry.Item) continue;

		const FString& ItemType = Entry.ItemType;
		const FString MeshPath = GetMeshPathByItemType(ItemType);

		if (ItemType == "Staff" || ItemType == "Sword" || ItemType == "Greatsword" || ItemType == "Bow" || ItemType == "Offhand")
		{
			UStaticMesh* NewMesh = Cast<UStaticMesh>(StaticLoadObject(UStaticMesh::StaticClass(), nullptr, *MeshPath));
			if (NewMesh)
			{
				UpdateItemVisuals(NewMesh, ItemType);
			}
		}
		else
		{
			USkeletalMesh* NewMesh = Cast<USkeletalMesh>(StaticLoadObject(USkeletalMesh::StaticClass(), nullptr, *MeshPath));
			if (NewMesh)
			{
				UpdateItemVisuals(NewMesh, ItemType);
			}
		}
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

void ARelicRunnersCharacter::DamageAbility()
{
	UE_LOG(LogTemp, Log, TEXT("Damage Ability Activated on Character"));
}

void ARelicRunnersCharacter::DefenceAbility()
{
	UE_LOG(LogTemp, Log, TEXT("Defence Ability Activated on Character"));
}

void ARelicRunnersCharacter::UtilityAbility()
{
	UE_LOG(LogTemp, Log, TEXT("Utility Ability Activated on Character"));
}

void ARelicRunnersCharacter::UltimateAbility()
{
	UE_LOG(LogTemp, Log, TEXT("Ultimate Ability Activated on Character"));
}