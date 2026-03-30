// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyCharacter.h"
#include "RelicRunners/CustomDamageEvents.h"
#include "RelicRunners/Director System/Director.h"
#include <Kismet/GameplayStatics.h>
#include <RelicRunners/Item/ItemActor.h>
#include "RelicRunners/RelicRunnersCharacter.h"
#include "Components/WidgetComponent.h"
#include <Net/UnrealNetwork.h>
#include "RelicRunners/Enemy/EnemyHUDWorld.h"
#include "RelicRunners/Item/ItemStats.h"
#include "Components/CapsuleComponent.h"
#include "RelicRunners/ObjectPool/ObjectPoolManager.h"
#include "RelicRunners/PlayerState/RelicRunnersPlayerState.h"
#include "RelicRunners/Enemy/EnemyStats.h"
#include <RelicRunners/Game/RelicRunnersGameInstance.h>

AEnemyCharacter::AEnemyCharacter() : ACharacter()
{
 	
	//setting up default values
	EnemyStatsComponent = CreateDefaultSubobject<UEnemyStats>(TEXT("Enemy Stats Component"));
	EnemyStatsComponent->SetEnemyType(EEnemyType::MEDIUM);
	EnemyStatsComponent->SetChanceToDropItem(0.2f);

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

	ShouldDieIfCommanderIsDone = false;

	//tick moment :O
	PrimaryActorTick.bCanEverTick = false;
	//Setting up networking for the actor
	bReplicates = true;
	SetReplicateMovement(true);
	//add the holy tag
	Tags.Add("Enemy");
}

void AEnemyCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	/*DOREPLIFETIME(AEnemyCharacter, CurrentHealth);
	DOREPLIFETIME(AEnemyCharacter, MaxHealth);
	DOREPLIFETIME(AEnemyCharacter, RemainingStunTime);
	DOREPLIFETIME(AEnemyCharacter, Level);
	DOREPLIFETIME(AEnemyCharacter, TypeOfEnemy);
	DOREPLIFETIME(AEnemyCharacter, EnemyHUDWorld);
	DOREPLIFETIME(AEnemyCharacter, EnemyResource);
	DOREPLIFETIME(AEnemyCharacter, EnemyMaxResource);*/
	DOREPLIFETIME(AEnemyCharacter, IsActive);
}

float AEnemyCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (!HasAuthority())
	{
		return 0;
	}

	//call the super for important stuff
	DamageAmount = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	//add the damage
	EnemyStatsComponent->AddToCurrentHealth(-DamageAmount);

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
				(AttackDamageEvent->AttackType == EAttackType::LIGHT_ATTACK || AttackDamageEvent->AttackType == EAttackType::HEAVY_ATTACK) && EnemyStatsComponent->GetEnemyType() == EEnemyType::LIGHT ||
				//medium enemy stunned by heavy attack
				AttackDamageEvent->AttackType == EAttackType::HEAVY_ATTACK && EnemyStatsComponent->GetEnemyType() == EEnemyType::MEDIUM))
			{
				EnemyStatsComponent->SetRemainingStunTime(AttackDamageEvent->StunDuration);
			}
		}

		TArray<FHitResult> HitResult;
		FCollisionObjectQueryParams objCollParams;
		objCollParams.AddObjectTypesToQuery(ECC_GameTraceChannel1);

		
		bool hit = GetWorld()->LineTraceMultiByObjectType(HitResult, DamageCauser->GetActorLocation(), GetActorLocation(), objCollParams);

		if (hit)
		{
			for (FHitResult result : HitResult)
			{
				AActor* hitActor = result.GetActor();
				if (hitActor == this)
				{
					NetMultPlayParticleEffect(DamageEffect, result.Location);
					break;
				}
			}
		}
		
	}

	return DamageAmount;
}

void AEnemyCharacter::Die(AController* EventInstigator)
{
	if (HasAuthority())
	{
		SpawnItem();

		if (EventInstigator)
		{
			ARelicRunnersPlayerState* PS = EventInstigator->GetPlayerState<ARelicRunnersPlayerState>();

			if (PS)
			{
				PS->AddExperience(CalculateXP());
			}
		}

		OnDied.Broadcast(this);
		OnDied.Clear();

		NetMultPlayParticleEffect(DeathEffect, GetActorLocation());

		AObjectPoolManager* PoolManager = AObjectPoolManager::GetInstance(this);
		
		if (PoolManager && PoolManager->IsActorClassPooled(this->GetClass()))
		{
			SetIsActive(false);
		}
		else
		{
			if (EnemyHUDWorld)
			{
				EnemyHUDWorld->DestroyComponent();
			}

			//Gives enough time to multicast effect
			float DeathDelay = 0.2f;
			FTimerHandle TearOffTimer;
			GetWorldTimerManager().SetTimer(TearOffTimer, this, &AEnemyCharacter::TearOff, DeathDelay, false);

			//Calls destory
			SetLifeSpan(DeathDelay);
		}
	}
}

inline bool AEnemyCharacter::IsDead()
{
	return EnemyStatsComponent->GetCurrentHealth() <= 0;
}

void AEnemyCharacter::ReduceStunTime(float DeltaTime)
{
	//do I really need to explain this?
	if (EnemyStatsComponent->GetRemainingStunTime() > 0)
	{
		EnemyStatsComponent->AddToRemainingStunTime(-DeltaTime);
	}
}

// Called when the game starts or when spawned
void AEnemyCharacter::BeginPlay()
{
	Super::BeginPlay();

	EnemyStatsComponent->SetCurrentHealth(EnemyStatsComponent->GetMaxHealth());

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

		if (chance <= EnemyStatsComponent->GetChanceToDropItem())
		{
			UIconLibrary* Lib = nullptr;
			if (URelicRunnersGameInstance* GI = GetWorld()->GetGameInstance<URelicRunnersGameInstance>())
			{
				Lib = GI->GlobalIconLibrary;
			}

			UItemObject* DroppedItem = ItemStats::CreateItemFromData(ItemStats::CreateSpecificItemData(EnemyStatsComponent->GetLevel(), ItemStats::GetRandomItemType(), ItemMeshData, Lib), this);

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
	return EnemyStatsComponent->GetLevel() * FMath::RandRange(100, 150);
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

void AEnemyCharacter::NetMultPlayParticleEffect_Implementation(UNiagaraSystem* effect, FVector position)
{
	if (effect)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), effect, position, GetActorRotation(), GetActorScale());
	}
}

void AEnemyCharacter::NetMultSetIsActive_Implementation(bool newIsActive)
{
	if (newIsActive)
	{
		this->SetActorHiddenInGame(false);
		this->SetActorEnableCollision(true);
		this->SetActorTickEnabled(this->GetShouldTick());
	}
	else
	{
		this->SetActorHiddenInGame(true);
		this->SetActorEnableCollision(false);
		this->SetActorTickEnabled(false);
	}
}



// Called every frame
void AEnemyCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	UpdateEnemyHUDWorldFacing();
}

void AEnemyCharacter::ResetEnemy()
{
	EnemyStatsComponent->SetCurrentHealth(EnemyStatsComponent->GetMaxHealth());
	EnemyStatsComponent->SetRemainingStunTime(0.f);
}

void AEnemyCharacter::AddEnemyToDirector()
{
	ADirector* Director = ADirector::GetInstance(this);

	if (Director && !Director->ContainsEnemy(this))
	{
		//add the enemy to the director
		Director->AddEnemy(this);
		//add the remove enemy method to the OnDestroyed delegate/ on die
		//this->OnDied.AddDynamic(Director, &ADirector::RemoveEnemy);
		this->OnDeactivated.AddDynamic(Director, &ADirector::RemoveEnemy);
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
	return false;
}

void AEnemyCharacter::SetIsActive(bool newIsActive)
{
	if (IsActive == newIsActive)
	{
		return;
	}

	IsActive = newIsActive;
	NetMultSetIsActive(newIsActive);

	if (!newIsActive)
	{
		OnDeactivated.Broadcast(this);
		ResetEnemy();
	}

	AObjectPoolManager* pool = AObjectPoolManager::GetInstance(this);

	if (pool && SetControllerActiveStatus && GetController()->GetClass()->ImplementsInterface(UPoolActorInterface::StaticClass()))
	{
		pool->SetActorActive(GetController(), newIsActive);
	}
}

FTimerHandle& AEnemyCharacter::GetLifeSpanTimerHandle()
{
	return LifeSpanTimerHandle;
}
