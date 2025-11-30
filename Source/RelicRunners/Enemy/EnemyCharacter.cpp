// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyCharacter.h"
#include "RelicRunners/CustomDamageEvents.h"
#include "RelicRunners/Director System/Director.h"
#include <Kismet/GameplayStatics.h>
#include <RelicRunners/Item/ItemActor.h>
#include "RelicRunners/RelicRunnersCharacter.h"
#include "Components/WidgetComponent.h"
#include <Net/UnrealNetwork.h>
#include "EnemyHUDWorld.h"
#include "RelicRunners/Item/ItemStats.h"
#include "Components/CapsuleComponent.h"
#include "RelicRunners/ObjectPool/ObjectPoolManager.h"

// Sets default values
AEnemyCharacter::AEnemyCharacter() : ACharacter()
{
 	
	//setting up default values
	CurrentHealth = 100;
	MaxHealth = 100;
	RemainingStunTime = 0;
	Level = 1;
	TypeOfEnemy = EEnemyType::UNKNOWN;

	//Make the collision profile is set to the right one
	GetCapsuleComponent()->SetCollisionProfileName("Enemy");

	//tristan UI stuff
	EnemyHUDWorld = CreateDefaultSubobject<UWidgetComponent>(TEXT("TooltipWidgetComponent"));
	EnemyHUDWorld->SetupAttachment(GetMesh());
	EnemyHUDWorld->SetWorldScale3D(FVector(0.1, 0.1, 0.1));
	EnemyHUDWorld->SetRelativeLocation(FVector(0.f, 0.f, 200.f));
	EnemyHUDWorld->SetWidgetSpace(EWidgetSpace::World);
	EnemyHUDWorld->SetDrawAtDesiredSize(true);
	EnemyHUDWorld->SetVisibility(true);
	EnemyHUDWorld->CastShadow = false;

	//tick moment :O
	PrimaryActorTick.bCanEverTick = true;
	//Setting up networking for the actor
	bReplicates = true;
	SetReplicateMovement(true);
	//add the holy tag
	Tags.Add("Enemy");
}

void AEnemyCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AEnemyCharacter, CurrentHealth);
	DOREPLIFETIME(AEnemyCharacter, MaxHealth);
	DOREPLIFETIME(AEnemyCharacter, RemainingStunTime);
	DOREPLIFETIME(AEnemyCharacter, Level);
	DOREPLIFETIME(AEnemyCharacter, TypeOfEnemy);
	DOREPLIFETIME(AEnemyCharacter, EnemyHUDWorld);
	DOREPLIFETIME(AEnemyCharacter, EnemyResource);
	DOREPLIFETIME(AEnemyCharacter, EnemyMaxResource);
	DOREPLIFETIME(AEnemyCharacter, IsActive);
}

float AEnemyCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	//call the super for important stuff
	DamageAmount = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	//add the damage
	CurrentHealth -= DamageAmount;

	//make sure we don't go below zero
	if (DamageAmount < 0)
	{
		DamageAmount = 0;
	}

	//check if we meet the conditions to be dead and then die
	if (IsDead())
	{
		Die(EventInstigator);
	}
	//if we are not dead check the damage events for stun
	else
	{
		if (DamageEvent.IsOfType(FAttackDamageEvent::ClassID))
		{
			FAttackDamageEvent* AttackDamageEvent = (FAttackDamageEvent*)&DamageEvent;
			if (AttackDamageEvent && (
				//light enemy stunned by light or heavy attack
				(AttackDamageEvent->AttackType == EAttackType::LIGHT_ATTACK || AttackDamageEvent->AttackType == EAttackType::HEAVY_ATTACK) && TypeOfEnemy == EEnemyType::LIGHT || 
				//medium enemy stunned by heavy attack
				AttackDamageEvent->AttackType == EAttackType::HEAVY_ATTACK && TypeOfEnemy == EEnemyType::MEDIUM))
			{
				RemainingStunTime = AttackDamageEvent->StunDuration;
			}
		}
	}

	return DamageAmount;
}

void AEnemyCharacter::Die(AController* EventInstigator)
{
	//make sure we are server side
	if (HasAuthority())
	{
		//spawn item
		SpawnItem();

		//check to make sure it a play and if do add experience
		if (EventInstigator)
		{
			if (ARelicRunnersCharacter* player = Cast<ARelicRunnersCharacter>(EventInstigator->GetPawn()))
			{
				player->AddExperience(CalculateXP());
			}
		}
		
		OnDied.Broadcast(this);
		OnDied.Clear();
		OnDestroyed.Clear();

		AObjectPoolManager* PoolManager = AObjectPoolManager::GetInstance(this);

		if (PoolManager && PoolManager->IsActorClassPooled(this->GetClass()))
		{
			SetIsActive(false);
			ResetEnemy();
		}
		else
		{
			//be gone hud of my enemy
			if (EnemyHUDWorld)
			{
				EnemyHUDWorld->DestroyComponent();
			}

			Destroy();
		}
	}
}

void AEnemyCharacter::ReduceStunTime(float DeltaTime)
{
	//do I really need to explain this?
	if (RemainingStunTime > 0)
	{
		RemainingStunTime -= DeltaTime;
		if (RemainingStunTime < 0)
		{
			RemainingStunTime = 0;
		}
	}
}

// Called when the game starts or when spawned
void AEnemyCharacter::BeginPlay()
{
	Super::BeginPlay();

	CurrentHealth = MaxHealth;

	//set up the enemy hud
	if (EnemyHUDWorld)
	{
		UUserWidget* Widget = EnemyHUDWorld->GetUserWidgetObject();
		if (Widget)
		{
			UEnemyHUDWorld* EnemyHUD = Cast<UEnemyHUDWorld>(Widget);
			if (EnemyHUD)
			{
				EnemyHUD->InitWidgetWithCharacter(this);
			}
		}
	}


	//mesh data
	if (ItemMeshDataClass)
	{
		ItemMeshData = ItemMeshDataClass->GetDefaultObject<UItemMeshData>();
	}
	//make sure we are client side

	if (HasAuthority())
	{
		//get refernce to objects that we need to get the director
		UWorld* World = GetWorld();
		APawn* enemy = static_cast<APawn*>(this);

		//call it next tick so their no problem with stuff not loaded in yet
		GetWorld()->GetTimerManager().SetTimerForNextTick(this, &AEnemyCharacter::AddEnemyToDirector);
	}
}

void AEnemyCharacter::SpawnItem()
{
	if (ItemLootPool.Num() > 0)
	{
		float chance = FMath::FRandRange(0.f, 1.f);

		if (chance <= ChanceToDropItem)
		{
			UItemObject* DroppedItem = ItemStats::CreateItemFromData(ItemStats::CreateSpecificItemData(Level, ItemStats::GetRandomItemType(), ItemMeshData), this);

			// Spawn new dropped item
			FActorSpawnParameters Params;
			Params.Owner = this;
			Params.Instigator = this;
			Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
			// Desired drop location
			FVector SpawnLoc = GetActorLocation() + GetActorForwardVector() * 100.f;
			AItemActor* Dropped = GetWorld()->SpawnActor<AItemActor>(ItemActorClass, SpawnLoc, FRotator::ZeroRotator, Params);

			if (Dropped && DroppedItem)
			{
				Dropped->Initialize(DroppedItem->ItemData);
			}
		}
	}
}

float AEnemyCharacter::CalculateXP() const
{
	//calculate the xp
	return Level * FMath::RandRange(100, 150);
}

void AEnemyCharacter::PlayMontageOnClient_Implementation(UAnimMontage* Montage, float PlayRate)
{
	//if we are not server side run montage
	//This is because in the engine I have a blueprint that runs play montage for the server. Once I get more time I will rewrite this function to have the functionality as the blueprint node but also run the client
	if (!HasAuthority())
	{
		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

		if (AnimInstance)
		{
			AnimInstance->Montage_Play(Montage, PlayRate);
		}
	}
}

void AEnemyCharacter::UpdateEnemyHUDWorldFacing()
{
	//tristan's code
	if (GetNetMode() == NM_DedicatedServer)
	{
		return;
	}
	
	if (EnemyHUDWorld || EnemyHUDWorld->GetWidget())
	{
		APlayerController* LocalController = UGameplayStatics::GetPlayerController(GetWorld(), 0);

		if (LocalController || LocalController->PlayerCameraManager)
		{
			FVector CameraLocation = LocalController->PlayerCameraManager->GetCameraLocation();
			FVector WidgetLocation = EnemyHUDWorld->GetComponentLocation();

			FVector ToCamera = CameraLocation - WidgetLocation;
			ToCamera.Z = 0.f;

			if (!ToCamera.IsNearlyZero())
			{
				FRotator LookAtRotation = ToCamera.Rotation();
				EnemyHUDWorld->SetWorldRotation(LookAtRotation);
			}

			EnemyHUDWorld->SetVisibility(true);
		}
	}
}

#if WITH_EDITOR
void AEnemyCharacter::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	//calling the super
	Super::PostEditChangeProperty(PropertyChangedEvent);

	//crabing the property name
	FName PropertyName = NAME_None; 
	
	//check to make sure this is a actual property
	if (PropertyChangedEvent.Property)
	{
		//update the name
		PropertyName = PropertyChangedEvent.Property->GetFName();
	}

	//go throught and check the possible properties that need to update other variables
	//GET_MEMBER_NAME_CHECKED is a compiler time macro that gets the name of a property for anyone who is readying this and if so thank you for reading my comments
	if (PropertyName == GET_MEMBER_NAME_CHECKED(AEnemyCharacter, MaxHealth))
	{
		CurrentHealth = FMath::Min(CurrentHealth, MaxHealth);
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(AEnemyCharacter, CurrentHealth))
	{
		CurrentHealth = FMath::Clamp(CurrentHealth, 0.f, MaxHealth);
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(AEnemyCharacter, EnemyMaxResource))
	{
		EnemyResource = FMath::Min(EnemyResource, EnemyMaxResource);
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(AEnemyCharacter, EnemyResource))
	{
		EnemyResource = FMath::Clamp(EnemyResource, 0, EnemyMaxResource);
	}
}
#endif

// Called every frame
void AEnemyCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	UpdateEnemyHUDWorldFacing();
}

// Called to bind functionality to input
void AEnemyCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void AEnemyCharacter::SetCurrentHealth(const float& NewHealth)
{
	CurrentHealth = FMath::Clamp(NewHealth, 0.f, MaxHealth);
}

void AEnemyCharacter::SetMaxHealth(const float& newHealth)
{
	MaxHealth = FMath::Max(0.f, newHealth);
	CurrentHealth = FMath::Min(CurrentHealth, MaxHealth);
}

void AEnemyCharacter::ResetEnemy()
{
	CurrentHealth = MaxHealth;
	RemainingStunTime = 0.f;
}

void AEnemyCharacter::AddEnemyToDirector()
{
	ADirector* Director = static_cast<ADirector*>(UGameplayStatics::GetActorOfClass(this->GetWorld(), ADirector::StaticClass()));

	if (Director && !Director->ContainsEnemy(this))
	{
		//add the enemy to the director
		Director->AddEnemy(this);
		//add the remove enemy method to the OnDestroyed delegate/ on die
		this->OnDied.AddDynamic(Director, &ADirector::RemoveEnemy);
		this->OnDestroyed.AddDynamic(Director, &ADirector::RemoveEnemy);
	}
#if WITH_EDITOR || UE_BUILD_TEST
	else
	{
		UE_LOG(LogTemp, Error, TEXT("missing director"));
	}
#endif
}

bool AEnemyCharacter::GetIsActive()
{
	return IsActive;
}

bool AEnemyCharacter::GetShouldTick()
{
	return true;
}

void AEnemyCharacter::SetIsActive(bool newIsActive)
{
	if (IsActive == newIsActive)
	{
		return;
	}

	IsActive = newIsActive;
	IPoolActorInterface::SetIsActive(newIsActive);

	if (SetControllerActiveStatus && GetController()->GetClass()->ImplementsInterface(UPoolActorInterface::StaticClass()))
	{
		AObjectPoolManager::GetInstance(this)->SetActorActive(GetController(), newIsActive);
	}
}