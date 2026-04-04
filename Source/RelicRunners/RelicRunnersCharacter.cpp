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
#include "RelicRunnersGameMode.h"
#include "Engine/OverlapResult.h"
#include "CustomDamageEvents.h"
#include "Camera/CameraShakeBase.h"
#include "RelicRunners/RelicRunnersGameState.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "RelicRunners/Enemy/HitStunComponent.h"

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
	bUseControllerRotationYaw = true; 

	GetCharacterMovement()->bOrientRotationToMovement = false;
	CameraBoom->bEnableCameraLag = true;
	CameraBoom->CameraLagSpeed = 10.f;

	CameraBoom->bEnableCameraRotationLag = true;
	CameraBoom->CameraRotationLagSpeed = 10.f;

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

	HitStunComponent = CreateDefaultSubobject<UHitStunComponent>(TEXT("Hit Stun Component"));
	
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

	ARelicRunnersPlayerState* PS = GetPlayerState<ARelicRunnersPlayerState>();
	if (PS)
	{
		AbilitySystem = PS->GetAbilitySystemComponent();
	}

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
		bUseControllerRotationYaw = false;
		GetCharacterMovement()->bOrientRotationToMovement = true;

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
		RelicPlayerState->DeathState = false;
		const TArray<FEquippedItemEntry>& EquippedEntries = RelicPlayerState->GetInventoryComponent()->EquippedItems;
		SyncEquippedVisuals(EquippedEntries);
	}
}

float ARelicRunnersCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (!HasAuthority()) return 0.0f;

	float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	if (RelicPlayerState)
	{
		int Defence = RelicPlayerState->GetInventoryComponent()->CachedEquippedStats.TotalArmor;
		float Reduction = (1.0f - (ItemStats::GetDefencePerc(Defence) / 100.0f));
		float DamageTaken = ActualDamage * Reduction;
		ActualDamage = DamageTaken;
		RelicPlayerState->ModifyHealth(-DamageTaken);

		if (HitCameraShake)
			GetWorld()->GetFirstPlayerController()->PlayerCameraManager->PlayWorldCameraShake(GetWorld(), HitCameraShake, FollowCamera->GetComponentLocation(), 0.0f, 500.0f, 1.0f);

	}

	return ActualDamage;
}

void ARelicRunnersCharacter::Die()
{

	if (!HasAuthority()) return;


	if (RelicPlayerState)
	{
		RelicPlayerState->DeathState = true;
		RelicPlayerState->PlayerDeaths++;
	}

	ARelicRunnersPlayerController* RPC = Cast<ARelicRunnersPlayerController>(GetController());
	if (RPC)
	{
		RPC->ChangeState(NAME_Spectating);
		RPC->Client_RequestSaveStats();

		if (RPC->IsLocalController())
		{
			RPC->CleanUI();
			RPC->SpectatingUI();
		}
	}

	if (ARelicRunnersGameMode* GM = GetWorld()->GetAuthGameMode<ARelicRunnersGameMode>())
	{
		ARelicRunnersGameState* GS = Cast<ARelicRunnersGameState>(GetWorld()->GetGameState());

		if (GS && GS->PlayerArray.Num() > 1)
		{
			FTimerHandle TimerHandle_GameOver;
			GetWorldTimerManager().SetTimer(TimerHandle_GameOver, GM, &ARelicRunnersGameMode::CheckGameOver, 1.5f, false);
		}
		else
		{
			GM->CheckGameOver();
		}
	}

	SetLifeSpan(0.5f);
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

	const FVector TraceStart = FollowCamera->GetComponentLocation();
	const FVector LookDir = FollowCamera->GetForwardVector();
	const float MaxDistance = 800.f;

	TArray<FOverlapResult> OverlapResults;
	FCollisionShape SelectionSphere = FCollisionShape::MakeSphere(MaxDistance);
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	bool bHit = GetWorld()->OverlapMultiByChannel(
		OverlapResults,
		GetActorLocation(),
		FQuat::Identity,
		ECC_Visibility,
		SelectionSphere,
		Params
	);

	AActor* BestTarget = nullptr;
	float BestDot = 0.6f;

	if (bHit)
	{
		for (const FOverlapResult& Result : OverlapResults)
		{
			AActor* FoundActor = Result.GetActor();

			if (IsValid(FoundActor) && FoundActor->Implements<UInteractInterface>())
			{
				FVector ToActor = (FoundActor->GetActorLocation() - TraceStart).GetSafeNormal();
				float DotProduct = FVector::DotProduct(LookDir, ToActor);

				if (DotProduct > BestDot)
				{
					BestDot = DotProduct;
					BestTarget = FoundActor;
				}
			}
		}
	}

	if (BestTarget != CurrentFocusedActor)
	{
		if (CurrentFocusedActor)
		{
			IInteractInterface::Execute_ShowTooltip(CurrentFocusedActor, false);
		}

		CurrentFocusedActor = BestTarget;

		if (CurrentFocusedActor)
		{
			IInteractInterface::Execute_ShowTooltip(CurrentFocusedActor, true);
		}
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

void ARelicRunnersCharacter::Server_HitEnemy_Implementation(AActor* TargetActor, float BaseDamage)
{
	if (!TargetActor) return;

	AController* InstigatorController = GetInstigatorController();
	if (!InstigatorController) return;

	APlayerController* PC = Cast<APlayerController>(InstigatorController);

	UInventoryComponent* InventoryComponent = nullptr;
	if (PC)
	{
		if (ARelicRunnersPlayerState* PS = PC->GetPlayerState<ARelicRunnersPlayerState>())
		{
			InventoryComponent = PS->GetInventoryComponent();
		}
	}

	float PercentStrengthIncrease = 0.f;
	float CritChance = 0.f;
	float CritDamageMultiplier = 1.f;

	if (InventoryComponent)
	{
		int Strength = InventoryComponent->CachedEquippedStats.TotalStrength;
		int CritValue = InventoryComponent->CachedEquippedStats.TotalDexterity;

		PercentStrengthIncrease = ItemStats::GetStrengthPerc(Strength);

		CritChance = ItemStats::GetCritPerc(CritValue);
		CritChance = FMath::Clamp(CritChance, 0.f, 100.f);
		CritDamageMultiplier = ItemStats::GetCritDmgPerc(CritValue);
	}

	float FinalDamage = BaseDamage * (1.f + PercentStrengthIncrease / 100.f);

	// Roll for crit
	float RandomRoll = FMath::FRandRange(1.f, 100.f);
	bool bIsCrit = RandomRoll <= CritChance;

	if (bIsCrit)
	{
		FinalDamage *= CritDamageMultiplier;
	}

	FAttackDamageEvent DamageEvent(EAttackType::LIGHT_ATTACK, 500.f, 500.f);

	TargetActor->TakeDamage(FinalDamage, DamageEvent, InstigatorController, this);
	//UGameplayStatics::ApplyDamage(TargetActor, FinalDamage, InstigatorController, this);

/*	if (!Tags.Contains("Ranage") && HitStunComponent)
	{
		HitStunComponent->Multicast_StartHitStun(0.0005f);
	}*/
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
	if (Value == 0.0f)
	{
		return; 
	}

	if (!Controller)
	{
		return; 
	}

	const FRotator Rotation = Controller->GetControlRotation();
	const FRotator YawRotation(0, Rotation.Yaw, 0);

	float SpeedMultiplier = 1.0f;

	if (Axis == EAxis::X) 
	{
		if (Value < 0.0f)
		{
			SpeedMultiplier = 0.6f; 
			Server_PlayAbilityAnimation(BackStepAnim, 1.f);
		}
	}
	else if (Axis == EAxis::Y) 
	{
		SpeedMultiplier = 0.7f;
		Server_PlayAbilityAnimation(SideStepLeftAnim, 1.f);

	}

	// Base movement
	FVector Direction;
	if (Axis == EAxis::X)
	{
		Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	}
	else if (Axis == EAxis::Y)
	{
		Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
	}
	else
	{
		return; 
	}

	AddMovementInput(Direction, Value * SpeedMultiplier);

	UWorld* World = GetWorld();
	if (World)
	{
		const FString MapName = World->GetMapName();
		if (MapName.Contains(TEXT("BossLevel")))
		{
			AddMovementInput(Direction, Value); 
		}
	}
}

void ARelicRunnersCharacter::Look(const FInputActionValue& Value)
{
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	//Lock look input on boss level
	UWorld* const World = GetWorld();
	if (!World) return;

	if (World->GetMapName().Contains("BossLevel")) return;
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

	UWorld* const World = GetWorld();
	if (!World) return;

	if (World->SweepSingleByChannel(Hit, Start, End, FQuat::Identity, ECC_Visibility, FCollisionShape::MakeSphere(60.f), Params))
	{
		//DrawDebugSphere(World, Hit.Location, 60.f, 12, FColor::Orange, false, 0.1f);
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

void ARelicRunnersCharacter::InitializeAbilityCosts()
{
	ARelicRunnersPlayerController* RPC = Cast<ARelicRunnersPlayerController>(GetController());
	if (!RPC) return;

	RPC->BaseCostAbility1 = GetAbilityCost(AbilityClass_1);
	RPC->BaseCostAbility2 = GetAbilityCost(AbilityClass_2);
	RPC->BaseCostAbility3 = GetAbilityCost(AbilityClass_3);
	RPC->BaseCostAbility4 = GetAbilityCost(AbilityClass_4);

	RPC->UpdateAllAbilityManaUI();
}

int ARelicRunnersCharacter::GetAbilityCost(TSubclassOf<UAbilityBase> AbilityClass)
{
	if (!AbilityClass)
	{
		return 0;
	}

	UObject* CDO = AbilityClass->GetDefaultObject();
	if (!CDO)
	{
		return 0;
	}

	UAbilityBase* DefaultAbility = Cast<UAbilityBase>(CDO);
	if (DefaultAbility)
	{
		return DefaultAbility->Cost;
	}

	return 0;
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

	AItemActor* Dropped = GetWorld()->SpawnActorDeferred<AItemActor>(
			ItemActorClass,
			FTransform(FRotator::ZeroRotator, FinalLoc),
			this,
			this,
			ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn
		);

		if (Dropped)
		{
			//This ensures Auto-Loot can't trigger until the data is valid!
			Dropped->ItemData = Data;
			Dropped->Initialize(Data);
			Dropped->FinishSpawning(FTransform(FRotator::ZeroRotator, FinalLoc));
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
		Server_PlayAbilityAnimation(HealPotionAnim, 1.f);
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
	FString Suffix = "";

	bool IsWeapon = ItemType.Equals("Sword", ESearchCase::IgnoreCase) ||
		ItemType.Equals("Bow", ESearchCase::IgnoreCase) ||
		ItemType.Equals("Greatsword", ESearchCase::IgnoreCase) ||
		ItemType.Equals("Staff", ESearchCase::IgnoreCase) ||
		ItemType.Equals("Shield", ESearchCase::IgnoreCase);

	if (!IsWeapon)
	{
		if (ARelicRunnersPlayerState* RPS = GetPlayerState<ARelicRunnersPlayerState>())
		{
			if (!RPS->SelectedClass.IsNone())
			{
				Suffix = RPS->SelectedClass.ToString();
			}
		}
	}

	return FString::Printf(TEXT("/Game/ThirdPerson/Meshes/Items/%s/%s%s.%s%s"),
		*ItemType, *ItemType, *Suffix, *ItemType, *Suffix);
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

void ARelicRunnersCharacter::Multicast_PlayAbilityAnimation_Implementation(UAnimMontage* Montage, float AnimSpeed)
{

	if (!Montage) return;

	USkeletalMeshComponent* Mesh1 = FindComponentByClass<USkeletalMeshComponent>();
	if (!Mesh1) return;

	UAnimInstance* AnimInstance = Mesh1->GetAnimInstance();
	if (!AnimInstance) return;

	AnimInstance->Montage_Play(Montage, AnimSpeed);
}

void ARelicRunnersCharacter::Server_PlayAbilityAnimation_Implementation(UAnimMontage* Montage, float AnimSpeed)
{
	Multicast_PlayAbilityAnimation(Montage, AnimSpeed);
}

void ARelicRunnersCharacter::Multicast_SpawnBeam_Implementation(AActor* Target, UNiagaraSystem* VFX)
{
	if (!IsValid(this) || !IsValid(Target) || !IsValid(VFX)) return;

	USceneComponent* RootComp = GetRootComponent();
	if (!IsValid(RootComp)) return;

	TWeakObjectPtr<AActor> WeakTarget = Target;

	TWeakObjectPtr<UNiagaraComponent>* FoundBeamPtr = ActiveBeams.Find(WeakTarget);

	if (FoundBeamPtr && FoundBeamPtr->IsValid())
	{
		UNiagaraComponent* Beam = FoundBeamPtr->Get();
		if (IsValid(Beam))
		{
			Beam->SetVectorParameter(TEXT("EndPoint"), Target->GetActorLocation());
			return;
		}
		else
		{
			ActiveBeams.Remove(WeakTarget);
		}
	}
	else if (FoundBeamPtr)
	{
		// Weak pointer expired
		ActiveBeams.Remove(WeakTarget);
	}

	// Spawn new beam
	UNiagaraComponent* BeamComp = UNiagaraFunctionLibrary::SpawnSystemAttached(VFX,RootComp,NAME_None,FVector::ZeroVector,FRotator::ZeroRotator,EAttachLocation::KeepRelativeOffset,true);

	if (IsValid(BeamComp))
	{
		BeamComp->SetVectorParameter(TEXT("EndPoint"), Target->GetActorLocation());
		ActiveBeams.Add(WeakTarget, BeamComp);
	}
}


void ARelicRunnersCharacter::EndAllAbilities()
{
	if (AbilitySystem)
	{
		AbilitySystem->CancelAllAbilities();
	}

	GetWorld()->GetTimerManager().ClearAllTimersForObject(this);
}