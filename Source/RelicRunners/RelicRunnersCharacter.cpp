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
#include "Enemy/Characters/EnemyCharacter.h"
#include "Enemy/Characters/EnemyCharacterAI.h"
#include "Components/SceneCaptureComponent2D.h"
#include "DrawDebugHelpers.h"
#include "RelicRunners/Menu/Keybinds.h"
#include "LevelGeneration/NewObjectives/NewCapturableFlag.h"
#include <Kismet/KismetMathLibrary.h>
#include "Components/SkeletalMeshComponent.h"
#include "TimerManager.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

ARelicRunnersCharacter::ARelicRunnersCharacter()
{
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
	
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate
	//GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);

	CameraBoom->TargetArmLength = 500.0f;

	CameraBoom->bUsePawnControlRotation = true;

	CameraBoom->bEnableCameraLag = true;
	CameraBoom->CameraLagSpeed = 8.f;

	CameraBoom->bEnableCameraRotationLag = true;
	CameraBoom->CameraRotationLagSpeed = 6.f;

	CameraBoom->bDoCollisionTest = false;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;


	MinimapSpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("MinimapSpringArm"));
	MinimapSpringArm->SetupAttachment(RootComponent);
	MinimapSpringArm->SetRelativeRotation(FRotator(-90.0f, 0.0f, 0.0f)); 
	MinimapSpringArm->TargetArmLength = 450.0; 
	MinimapSpringArm->bInheritPitch = false;
	MinimapSpringArm->bInheritRoll = false;
	MinimapSpringArm->bInheritYaw = false; 

	//Capture Component
	MinimapCaptureComponent = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("MinimapCaptureComponent"));
	MinimapCaptureComponent->SetupAttachment(MinimapSpringArm);
	MinimapCaptureComponent->ProjectionType = ECameraProjectionMode::Orthographic;
	MinimapCaptureComponent->OrthoWidth = 5000.0f; 
	MinimapCaptureComponent->CaptureSource = SCS_FinalColorLDR;
	MinimapCaptureComponent->bAutoActivate = false; 
	MinimapCaptureComponent->SetIsReplicated(false);
	MinimapCaptureComponent->HideComponent(GetMesh());

	//Mesh attachments
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
	PlayerHUDWorld->SetWorldScale3D(FVector(0.15, 0.15, 0.15));
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

	InitializeWorldHUD();
	InitializeWeaponSockets();

	//Server
	if (HasAuthority())
	{
		RegisterDirector();
	}

	respawnPoint = GetActorLocation();

	const FString MapName = GetWorld()->GetMapName();
	if (MapName.Contains(TEXT("BossLevel")))
	{
		CameraBoom->TargetArmLength = 3000;
		CameraBoom->SetRelativeLocation(FVector(0.f, 0.f, 800.f));
	}

	
}

void ARelicRunnersCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ARelicRunnersCharacter, ReplicatedVisuals);	
}

void ARelicRunnersCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	RelicPlayerState = GetPlayerState<ARelicRunnersPlayerState>();

	if (RelicPlayerState)
	{
		const TArray<FEquippedItemEntry>& EquippedEntries = RelicPlayerState->GetInventoryComponent()->EquippedItems;
		SyncEquippedVisuals(EquippedEntries);
	}
}

float ARelicRunnersCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (!HasAuthority()) return 0.0f;

	float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	//if (RelicPlayerState) RelicPlayerState->ModifyHealth(-ActualDamage);
	
	if (bMirrorActive && ActualDamage > 0.f)
	{
		AActor* Attacker = DamageCauser;
	
		if (!Attacker && EventInstigator)
		{
			Attacker = EventInstigator->GetPawn();
		}

		if (Attacker && Attacker != this && Attacker->ActorHasTag("Enemy"))
		{
			const float Reflected = ActualDamage * MirrorReflectPercent;

			UGameplayStatics::ApplyDamage(Attacker,Reflected,GetController(),this,nullptr);
		}
	}

	if (bGuardActive && ActualDamage > 0.f)
	{
		AActor* Attacker = DamageCauser;

		if (!Attacker && EventInstigator)
		{
			Attacker = EventInstigator->GetPawn();
		}

		if (Attacker && Attacker != this && Attacker->ActorHasTag("Enemy"))
		{
			if (IsDamageFromFront(Attacker))
			{
				DrawDebugLine(GetWorld(),GetActorLocation() + FVector(0, 0, 60.f),Attacker->GetActorLocation(),FColor::Green,false, 0.15f, 0, 2.f);

				ActualDamage = 0.f;
			}
		}
	}
	
	if (RelicPlayerState)
	{
		int Defence = RelicPlayerState->GetInventoryComponent()->CachedEquippedStats.TotalArmor;
		float Reduction = (1.0f - (ItemStats::GetDefencePerc(Defence) / 100.0f));
		int DamageTaken = ActualDamage * Reduction;
		RelicPlayerState->ModifyHealth(-DamageTaken);
	}

	return ActualDamage;
}

void ARelicRunnersCharacter::Die()
{
	SetActorLocation(respawnPoint);
	if (RelicPlayerState) RelicPlayerState->ResetHealth();
}

void ARelicRunnersCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	//Client
	RelicPlayerState = GetPlayerState<ARelicRunnersPlayerState>();
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

void ARelicRunnersCharacter::InitializeWeaponSockets()
{
	MainhandItemMesh->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, TEXT("MainhandSocket"));
	OffhandItemMesh->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, TEXT("OffhandSocket"));
}

void ARelicRunnersCharacter::RegisterDirector()
{
	GetWorldTimerManager().SetTimerForNextTick([this]()
		{
			ADirector* Director = ADirector::GetInstance(this);
			if (Director)
			{
				Director->AddPlayer(this);

				this->OnDestroyed.AddDynamic(Director, &ADirector::RemovePlayer);
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("Character: Missing Director in World!"));
			}
		});
}

void ARelicRunnersCharacter::OnRep_VisualsChanged()
{
	//Update Static Meshes
	if (MainhandItemMesh) MainhandItemMesh->SetStaticMesh(ReplicatedVisuals.Mainhand);
	if (OffhandItemMesh) OffhandItemMesh->SetStaticMesh(ReplicatedVisuals.Offhand);

	//Update Skeletal Meshes
	if (UpperMesh) UpperMesh->SetSkeletalMesh(ReplicatedVisuals.Upper);
	if (LowerMesh) LowerMesh->SetSkeletalMesh(ReplicatedVisuals.Lower);
	if (HelmetMesh) HelmetMesh->SetSkeletalMesh(ReplicatedVisuals.Helmet);
	if (ArmsMesh) ArmsMesh->SetSkeletalMesh(ReplicatedVisuals.Arms);
	if (NecklaceMesh) NecklaceMesh->SetSkeletalMesh(ReplicatedVisuals.Necklace);
}

void ARelicRunnersCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	TraceForInteractables();
	UpdatePlayerHUDWorldFacing();
	UpdateCamera(DeltaTime);

	if (GetActorLocation().Z <= -200.0f)
	{
		Die();
	}

	if (IsLocallyControlled() && Controller && GetWorld()->GetMapName().Contains("BossLevel"))
	{
		FVector CurrentLocation = GetActorLocation();
		FVector Target(0.f, 0.f, -1000.f);
		FRotator LookAtRot = UKismetMathLibrary::FindLookAtRotation(CurrentLocation, Target);

		Controller->SetControlRotation(LookAtRot);
	}
}

void ARelicRunnersCharacter::SetMovementSpeed(float value)
{
	if (GetCharacterMovement())
	{
		GetCharacterMovement()->MaxWalkSpeed = value;
	}
}

void ARelicRunnersCharacter::UpdateCamera(float DeltaTime)
{
	
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
	
}

void ARelicRunnersCharacter::ResetBasicAttack()
{
	CanBasicAttack = true;
}

void ARelicRunnersCharacter::BasicAttack()
{
	
}

void ARelicRunnersCharacter::ResetHeavyAttack()
{
	CanHeavyAttack = true;
}

void ARelicRunnersCharacter::HeavyAttack()
{

}

void ARelicRunnersCharacter::ProjectileHeavyAttack()
{

}

void ARelicRunnersCharacter::PlayRandomAttackAnimation()
{
	
}

void ARelicRunnersCharacter::Server_HitEnemy_Implementation(AActor* HitActor)
{

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
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	//Lock look input on boss level
	if (GetWorld()->GetMapName().Contains("BossLevel")) return;
	else if (Controller != nullptr)
	{
		float XSens = 1.0f;
		float YSens = 1.0f;

		if (URelicRunnersGameInstance* GI = GetGameInstance<URelicRunnersGameInstance>())
		{
			if (GI->Keys)
			{
				XSens = GI->Keys->HorizSens;
				YSens = GI->Keys->VertSens;

				if (GI->Keys->InvertedXMouse) XSens *= -1.f;
				if (GI->Keys->InvertedYMouse) YSens *= -1.f;
			
			}
		}

		AddControllerYawInput(LookAxisVector.X * XSens);
		
		AddControllerPitchInput(LookAxisVector.Y * YSens);
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
			else if (ANewCapturableFlag* Flag = Cast<ANewCapturableFlag>(HitActor))
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

	AItemActor* Dropped = GetWorld()->SpawnActor<AItemActor>(ItemActorClass, FinalLoc, FRotator::ZeroRotator, Params);
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
	const float ItemRadius = 50.f; 
	FCollisionShape CheckSphere = FCollisionShape::MakeSphere(ItemRadius);
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);

	if (!GetWorld()->OverlapAnyTestByChannel(DesiredLoc, FQuat::Identity, ECC_Visibility, CheckSphere, QueryParams))
	{
		return DesiredLoc;
	}

	for (int Radius = 80; Radius <= 400; Radius += 80)
	{
		for (int Angle = 0; Angle < 360; Angle += 45)
		{
			const float Radians = FMath::DegreesToRadians(static_cast<float>(Angle));
			FVector Offset(FMath::Cos(Radians) * Radius, FMath::Sin(Radians) * Radius, 0.f);
			FVector CandidateLoc = DesiredLoc + Offset;

			bool bIsBlocked = GetWorld()->OverlapAnyTestByChannel(CandidateLoc, FQuat::Identity, ECC_Visibility, CheckSphere, QueryParams);

			if (!bIsBlocked)
			{
				FHitResult FloorHit;
				FVector TraceStart = CandidateLoc + FVector(0, 0, 100);
				FVector TraceEnd = CandidateLoc - FVector(0, 0, 200);

				if (GetWorld()->LineTraceSingleByChannel(FloorHit, TraceStart, TraceEnd, ECC_Visibility, QueryParams))
				{
					return FloorHit.ImpactPoint + FVector(0, 0, 100);
				}

				return CandidateLoc;
			}
		}
	}
	return DesiredLoc + (GetActorForwardVector() * 200.f);
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

void ARelicRunnersCharacter::UpdateItemVisuals(UItemObject* Item)
{
	if (!Item) return;

	const FString& ItemType = Item->ItemData.ItemType;
	const FString MeshPath = GetMeshPathByItemType(ItemType);

	UClass* MeshClass = (ItemType == "Upper" || ItemType == "Lower" || ItemType == "Helmet" || ItemType == "Arms")
		? USkeletalMesh::StaticClass() : UStaticMesh::StaticClass();

	UObject* LoadedMesh = StaticLoadObject(MeshClass, nullptr, *MeshPath);

	if (LoadedMesh) UpdateItemVisuals(LoadedMesh, ItemType);
}

void ARelicRunnersCharacter::UpdateItemVisuals(UObject* MeshAsset, const FString& ItemType)
{
	if (!HasAuthority()) return;

	if (ItemType == "Sword" || ItemType == "Bow" || ItemType == "Staff") 
		ReplicatedVisuals.Mainhand = Cast<UStaticMesh>(MeshAsset);
	else if (ItemType == "Greatsword")
	{
		ReplicatedVisuals.Mainhand = Cast<UStaticMesh>(MeshAsset);
		const FString MeshPath = GetMeshPathByItemType("Shield");
		UStaticMesh* NewMesh = Cast<UStaticMesh>(StaticLoadObject(UStaticMesh::StaticClass(), nullptr, *MeshPath));
		ReplicatedVisuals.Offhand = Cast<UStaticMesh>(NewMesh);
	}
	else if (ItemType == "Offhand") 
		ReplicatedVisuals.Offhand = Cast<UStaticMesh>(MeshAsset);
	else if (ItemType == "Upper")    
		ReplicatedVisuals.Upper = Cast<USkeletalMesh>(MeshAsset);
	else if (ItemType == "Lower")    
		ReplicatedVisuals.Lower = Cast<USkeletalMesh>(MeshAsset);
	else if (ItemType == "Helmet")   
		ReplicatedVisuals.Helmet = Cast<USkeletalMesh>(MeshAsset);
	else if (ItemType == "Arms")     
		ReplicatedVisuals.Arms = Cast<USkeletalMesh>(MeshAsset);
	else if (ItemType == "Necklace") 
		ReplicatedVisuals.Necklace = Cast<USkeletalMesh>(MeshAsset);

	//Server
	OnRep_VisualsChanged();
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

void ARelicRunnersCharacter::ActivateDamageAbilities()
{

}

void ARelicRunnersCharacter::ActivateDefenceAbilities()
{

}

void ARelicRunnersCharacter::ActivateUtilityAbilities()
{

}

void ARelicRunnersCharacter::ActivateUltimateAbilities()
{

}

void ARelicRunnersCharacter::DamageAbility()
{
	ActivateDamageAbilities();
	UE_LOG(LogTemp, Log, TEXT("Damage Ability Activated on Character"));
}


void ARelicRunnersCharacter::DefenceAbility()
{
	if (!IsLocallyControlled())
		return;

	ActivateDefenceAbilities();
	UE_LOG(LogTemp, Log, TEXT("Defence Ability Activated on Character"));
}

void ARelicRunnersCharacter::UtilityAbility()
{
	ActivateUtilityAbilities();
	UE_LOG(LogTemp, Log, TEXT("Utility Ability Activated on Character"));
}

void ARelicRunnersCharacter::UltimateAbility()
{
	ActivateUltimateAbilities();
	UE_LOG(LogTemp, Log, TEXT("Ultimate Ability Activated on Character"));
}

void ARelicRunnersCharacter::SetMirrorActive(bool bActive)
{
	if (!HasAuthority())
	{
		return;
	}

	bMirrorActive = bActive;

	if (!bMirrorActive)
	{
		GetWorldTimerManager().ClearTimer(MirrorDebugDrawHandle);
	}
	else
	{
		GetWorldTimerManager().SetTimer(MirrorDebugDrawHandle,this,&ARelicRunnersCharacter::DrawMirrorDebug,0.1f,true);
	}
}

void ARelicRunnersCharacter::DrawMirrorDebug()
{
	if (!bMirrorActive)
	{
		return;
	}

	DrawDebugSphere(GetWorld(),GetActorLocation(),MirrorRadius,16,FColor::Purple,false,0.11f);
}

void ARelicRunnersCharacter::SetGuardActive(bool bActive)
{
    if (!HasAuthority()) return;

    bGuardActive = bActive;

    if (!bGuardActive)
    {
        GetWorldTimerManager().ClearTimer(GuardDebugDrawHandle);
    }
    else
    {        
        GetWorldTimerManager().SetTimer(GuardDebugDrawHandle,this,&ARelicRunnersCharacter::DrawGuardDebug,0.1f,true);
    }
}

bool ARelicRunnersCharacter::IsDamageFromFront(AActor* DamageCauser) const
{
	if (!DamageCauser)
	{
		return false;
	}

    const FVector Forward = GetActorForwardVector().GetSafeNormal();
    const FVector ToCauser = (DamageCauser->GetActorLocation() - GetActorLocation()).GetSafeNormal();

    const float Dot = FVector::DotProduct(Forward, ToCauser);

    const float HalfAngleRad = FMath::DegreesToRadians(GuardAngleDegrees * 0.5f);
    const float Threshold = FMath::Cos(HalfAngleRad);

    return Dot >= Threshold;
}

void ARelicRunnersCharacter::DrawGuardDebug()
{
	if (!bGuardActive)
	{
		return;
	}

    const FVector Start = GetActorLocation() + FVector(0,0,60.f);
    const FVector End = Start + GetActorForwardVector() * 200.f;

    DrawDebugLine(GetWorld(), Start, End, FColor::Green, false, 0.11f, 0, 4.f);
}


//void ARelicRunnersCharacter::OnPause()
//{
//	UWorld* World = GetWorld();
//	FTimerHandle PauseHandle;
//	World->GetTimerManager().SetTimer(PauseHandle,this,&ARelicRunnersCharacter::ContinueMontage,5.f,false);
//}
//
//void ARelicRunnersCharacter::ContinueMontage()
//{
//	
//}

