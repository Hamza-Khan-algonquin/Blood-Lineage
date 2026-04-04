// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyPawn.h"
#include "Components/CapsuleComponent.h"
#include "GameplayEffect.h"
#include "GameplayEffectExtension.h"
#include "RelicRunners/CustomDamageEvents.h"
#include "RelicRunners/Director System/Director.h"
#include "RelicRunners/Enemy/Controllers/CommanderController.h"
#include "RelicRunners/Enemy/EnemyBoidMovementComponent.h"
#include "RelicRunners/Enemy/EnemyLODComponent.h"
#include "RelicRunners/Enemy/EnemyStats.h"
#include "RelicRunners/Enemy/LOD Wrapper Components/EnemyLODSkeletalMeshComponent.h"
#include "RelicRunners/Enemy/LOD Wrapper Components/EnemyLODStaticMeshComponent.h"
#include "RelicRunners/Item/ItemActor.h"
#include "RelicRunners/Item/ItemStats.h"
#include "RelicRunners/ObjectPool/ObjectPoolManager.h"
#include "RelicRunners/Item/ItemMeshData.h"
#include "NavigationSystem.h"
#include "RelicRunners/PlayerState/RelicRunnersPlayerState.h"
#include "RelicRunners/RelicRunnersCharacter.h"
#include <Kismet/GameplayStatics.h>
#include <Net/UnrealNetwork.h>
#include "Components/SphereComponent.h"
#include "NiagaraComponent.h"
#include "RelicRunners/Enemy/HitFlashComponent.h"
#include "RelicRunners/Enemy/HitStunComponent.h"

#include "RelicRunners/Inventory/InventoryComponent.h"
#include <RelicRunners/Game/RelicRunnersGameInstance.h>
#include <RelicRunners/RelicRunnersGameState.h>


// Sets default values
AEnemyPawn::AEnemyPawn()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	SetActorTickEnabled(false);

	this->AutoPossessAI = EAutoPossessAI::Disabled;

	CapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>("Capsule Component");
	RootComponent = CapsuleComponent;

	LODComponent = CreateDefaultSubobject<UEnemyLODComponent>("LOD Component");

	SkeletalMesh = CreateDefaultSubobject<UEnemyLODSkeletalMeshComponent>("Skeletal Mesh");
	SkeletalMesh->SetupAttachment(RootComponent);
	LODComponent->OnUpdateLODEvent.AddDynamic(SkeletalMesh, &UEnemyLODSkeletalMeshComponent::LODUpdate);

	StaticMesh = CreateDefaultSubobject<UEnemyLODStaticMeshComponent>("Static Mesh");
	StaticMesh->SetupAttachment(RootComponent);
	LODComponent->OnUpdateLODEvent.AddDynamic(StaticMesh, &UEnemyLODStaticMeshComponent::LODUpdate);

	MovementComponent = CreateDefaultSubobject<UEnemyBoidMovementComponent>("Movement Component");
	MovementComponent->UpdateNavAgent(*CapsuleComponent);

	//BasicAttackCollider = CreateDefaultSubobject<USphereComponent>("Basic Attack Collider");

	EnemyStatsComponent = CreateDefaultSubobject<UEnemyStats>(TEXT("Enemy Stats Component"));
	EnemyStatsComponent->SetEnemyType(EEnemyType::LIGHT);
	EnemyStatsComponent->SetChanceToDropItem(0.02f);
	EnemyStatsComponent->SetMaxHealth(100.f);
	EnemyStatsComponent->SetCurrentHealth(100.f);

	HitFlashComponent = CreateDefaultSubobject<UHitFlashComponent>(TEXT("Hit Flash Component"));
	
	HitStunComponent = CreateDefaultSubobject<UHitStunComponent>(TEXT("Hit Stun Component"));

	bReplicates = true;
	SetReplicateMovement(false);

	//times per second it replicates
	SetNetUpdateFrequency(5.f);
	SetMinNetUpdateFrequency(2.f);
	bAlwaysRelevant = false;
	SetNetCullDistanceSquared(FMath::Square(LODComponent->GetDistanceLOD(LODComponent->GetNumberOfLODs() - 1)));

	CapsuleComponent->OnComponentHit.AddDynamic(this, &AEnemyPawn::GroundCheck);

	KnockBackStrength = 5000;
	AdditionalUpwardsKnockBackForces = 50;
	HitStunTime = 0.3f;
	Tags.Add("Enemy");
}

void AEnemyPawn::UpdateEmissiveHealth()
{
	float HealthPercent = EnemyStatsComponent->GetCurrentHealth() / EnemyStatsComponent->GetMaxHealth();

	float NewIntensity = HealthPercent * 10.0f - 1.0f;
	for (UMaterialInstanceDynamic* element : DynamicMaterials)
	{
		if (element)
		{
			element->SetScalarParameterValue("GlowIntensity", NewIntensity);
		}
	}
}

void AEnemyPawn::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorld()->GetTimerManager().ClearTimer(LifeSpanTimerHandle);
	//GetWorld()->GetTimerManager().ClearTimer(AttackHandle);
	GetWorld()->GetTimerManager().ClearTimer(KnockBackFallBackHandle);
	GetWorld()->GetTimerManager().ClearTimer(HitStunTimer);
	Super::EndPlay(EndPlayReason);
}

float AEnemyPawn::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (!HasAuthority() || DamageAmount <= 0 || !IsActive || !MovementComponent || !MovementComponent->Commander || !GetWorld())
	{
		return 0;
	}
	
	//call the super for important stuff
	DamageAmount = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	if (APawn* pawn = EventInstigator ? EventInstigator->GetPawn() : nullptr)
	{
		if (ARelicRunnersCharacter* PlayerCharacter = Cast<ARelicRunnersCharacter>(pawn))
		{
			MovementComponent->Commander->ResetTimeToAttackForPlayer(pawn);
		}
	}
	
	//add the damage
	EnemyStatsComponent->AddToCurrentHealth(-DamageAmount);

	//check if we meet the conditions to be dead and then die
	if (IsDead() && IsActive)
	{
		Die(EventInstigator, (GetActorLocation() - DamageCauser->GetActorLocation().GetSafeNormal()));
	}
	//if we are not dead check the damage events for stun
	else
	{
		bool shouldKnockBack = false;
		const FAttackDamageEvent* AttackDamageEvent = nullptr;

		if (DamageEvent.IsOfType(FAttackDamageEvent::ClassID))
		{
			AttackDamageEvent = static_cast<const FAttackDamageEvent*>(&DamageEvent);
			shouldKnockBack = AttackDamageEvent && ((AttackDamageEvent->AttackType == EAttackType::LIGHT_ATTACK || AttackDamageEvent->AttackType == EAttackType::HEAVY_ATTACK) && EnemyStatsComponent->GetEnemyType() == EEnemyType::LIGHT);
		}

		if (HitSound)
		{
			UGameplayStatics::PlaySoundAtLocation(this, HitSound, GetActorLocation());
		}

		if(HitFlashComponent)
		{
			HitFlashComponent->Multicast_StartHitFlash();
		}

		if (HitStunComponent)
		{
			HitStunComponent->Multicast_StartHitStun(HitStunTime);

			if (AttackDamageEvent && shouldKnockBack)
			{
				FVector Direction = (GetActorLocation() - DamageCauser->GetActorLocation()).GetSafeNormal();
				FTimerDelegate HitSunDel;

				HitSunDel.BindUObject(this, &AEnemyPawn::KnockBack, Direction * AttackDamageEvent->KnockBackStrength + FVector(0, 0, AttackDamageEvent->AdditionalUpwardsKnockBackForces));
				GetWorld()->GetTimerManager().SetTimer(HitStunTimer, HitSunDel, HitStunTime, false);
			}
		}
		else if (AttackDamageEvent && shouldKnockBack)
		{
			FVector Direction = (GetActorLocation() - DamageCauser->GetActorLocation()).GetSafeNormal();
			KnockBack(Direction * AttackDamageEvent->KnockBackStrength + FVector(0, 0, AttackDamageEvent->AdditionalUpwardsKnockBackForces));
		}

		UpdateEmissiveHealth();

		IsAttacking = false;
		if (MovementComponent)
		{
			MovementComponent->SetMovementMode(EBoidMovementMode::Boid);
		}
	}

	return DamageAmount;
}

void AEnemyPawn::Die(AController* EventInstigator, FVector hitDirection)
{
	//make sure we are server side
	if (HasAuthority() && !IsHidden())
	{
		Multicast_PlayDeathEffect(hitDirection);

		//check to make sure it a play and if do add experience
		if (EventInstigator)
		{
			ARelicRunnersPlayerState* PS = EventInstigator->GetPlayerState<ARelicRunnersPlayerState>();

			if (PS)
			{
				PS->AddExperience(CalculateXP() * (EnemyStatsComponent->GetLevel()));
				PS->EnemiesKilled++;
				int PlayerLuck = PS->GetInventoryComponent()->CachedEquippedStats.TotalLuck;
				SpawnItem(PlayerLuck);
			}
		}

		if (DeathSound)
		{
			UGameplayStatics::PlaySoundAtLocation(this, DeathSound, GetActorLocation());
		}

		SetActorLocation(GetActorLocation() - FVector(0, 0, 10000));

		AObjectPoolManager* PoolManager = AObjectPoolManager::GetInstance(this);

		OnDied.Broadcast(this);
		OnDied.Clear();

		if (PoolManager && PoolManager->IsActorClassPooled(this->GetClass()))
		{
			SetIsActive(false);
		}
		else
		{
			//Gives enough time to multicast effect
			float DeathDelay = 0.2f;
			FTimerHandle TearOffTimer;
			GetWorldTimerManager().SetTimer(TearOffTimer, this, &AEnemyPawn::TearOff, DeathDelay, false);

			//Calls destory
			SetLifeSpan(DeathDelay);
		}
	}
}

void AEnemyPawn::Multicast_PlayDeathEffect_Implementation(FVector impactDirection)
{
	if (UEnemyBoidMovementComponent* BoidComp = FindComponentByClass<UEnemyBoidMovementComponent>())
	{
		BoidComp->Deactivate();
	}
	


	if (IsValid(DeathEffect))
	{
		if (GetActorLocation().ContainsNaN() ||
			GetActorRotation().ContainsNaN() ||
			GetActorScale().ContainsNaN())
		{
			UE_LOG(LogTemp, Error, TEXT("Invalid transform for Niagara spawn"));
			return;
		}

		UNiagaraComponent* deathEffect = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			DeathEffect,
			GetActorLocation(),
			GetActorRotation(),
			GetActorScale()
		);
	}
}

void AEnemyPawn::Multicast_GoldStuff_Implementation()
{
	for (UMaterialInstanceDynamic* element : DynamicMaterials)
	{
		if (element)
		{
			element->SetScalarParameterValue("IsGold", EnemyStatsComponent->IsGold);
		}
	}
}

inline bool AEnemyPawn::IsDead()
{
	return EnemyStatsComponent->GetCurrentHealth() <= 0;
}

void AEnemyPawn::ResetEnemy()
{
	EnemyStatsComponent->ResetStats();

	Multicast_GoldStuff();

	if (IsValid(MovementComponent))
	{
		MovementComponent->SetActive(true);
		MovementComponent->ClearMoveBuffer();
		MovementComponent->SetMovementMode(EBoidMovementMode::Boid);
	}
}

void AEnemyPawn::AddEnemyToDirector()
{
	ADirector* Director = ADirector::GetInstance(this);

	if (Director && !Director->ContainsEnemy(this))
	{
		//add the enemy to the director
		Director->AddEnemy(this);
		//add the remove enemy method to the OnDestroyed delegate/ on die
		this->OnDied.AddDynamic(Director, &ADirector::RemoveEnemy);
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

void AEnemyPawn::StartAttack()
{
	MovementComponent->SetMovementMode(EBoidMovementMode::DirectMoveTo);
	IsAttacking = true;
	SetActorTickEnabled(true);
}

void AEnemyPawn::AttackTick()
{
	if (!HasAuthority())
	{
		return;
	}

	if (MovementComponent->IsAtTarget())
	{
		if (!AttackMontage.IsNull())
		{

			if (!AttackMontage.IsValid())
			{
				AttackMontage.LoadSynchronous();
			}

			UAnimMontage* montage = AttackMontage.Get();
			PlayMontage(montage);

			float time = montage->GetPlayLength();
			if (!GetWorldTimerManager().IsTimerActive(AttackHandle))
			{
				GetWorldTimerManager().SetTimer(AttackHandle, this, &AEnemyPawn::EndAttackTick, time, false);
			}
			SetActorTickEnabled(false);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Attack Animation Missing"));
			MovementComponent->SetMovementMode(EBoidMovementMode::Boid);
		}
	}
}

void AEnemyPawn::EndAttackTick()
{
	IsAttacking = false;
	if (MovementComponent)
	{
		MovementComponent->SetMovementMode(EBoidMovementMode::Boid);
	}
	
}

void AEnemyPawn::BasicAttack(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor->Tags.Contains("Player"))
	{
		if (Controller)
		{
			UGameplayStatics::ApplyDamage(OtherActor, EnemyStatsComponent->GetAttackDamage(), Controller, this, nullptr);
		}
		else if (MovementComponent->Commander)
		{
			UGameplayStatics::ApplyDamage(OtherActor, EnemyStatsComponent->GetAttackDamage(), MovementComponent->Commander, this, nullptr);
		}
		else
		{
			UGameplayStatics::ApplyDamage(OtherActor, EnemyStatsComponent->GetAttackDamage(), nullptr, this, nullptr);
		}

		if (AttackHitPlayerSound)
		{
			UGameplayStatics::PlaySoundAtLocation(this, AttackHitPlayerSound, GetActorLocation());
		}
	}
}

float AEnemyPawn::CalculateXP() const
{
	return FMath::RandRange(5, 20);
}

void AEnemyPawn::SpawnItem(int Luck)
{
	if (!IsValid(EnemyStatsComponent)) return;

	//Difficulty
	URelicRunnersGameInstance* GI = GetWorld()->GetGameInstance<URelicRunnersGameInstance>();
	AGameStateBase* GameState = GetWorld()->GetGameState();
	FDifficultyInfo diffInfo = FDifficultyInfo();
	if (ARelicRunnersGameState* RunnersGameState = Cast<ARelicRunnersGameState>(GameState))
	{
		diffInfo = RunnersGameState->GetDifficultyInfo(GI->CurrentDifficulty);
	}

	float chance = FMath::FRandRange(0.f, 100.f);
	float chanceToDropWithDifficulty = EnemyStatsComponent->GetChanceToDropItem() - diffInfo.ItemDropReduction;
	float chanceToDropOverall = chanceToDropWithDifficulty + ItemStats::GetLootChance(Luck);
	if (EnemyStatsComponent->IsGold) chance = 0;

	if (chance <= chanceToDropOverall)
	{
		if (ItemMeshDataClass)
		{
			ItemMeshData = ItemMeshDataClass->GetDefaultObject<UItemMeshData>();
		}

		UIconLibrary* Lib = GI->GlobalIconLibrary;

		float GoldLuck = 0;
		float UsedLuck = Luck;
		if (EnemyStatsComponent->IsGold) GoldLuck = 95; UsedLuck = GoldLuck;

		UItemObject* DroppedItem = ItemStats::CreateItemFromData(
			ItemStats::CreateSpecificItemData(EnemyStatsComponent->GetLevel(), ItemStats::GetRandomItemType(), ItemStats::GetRandomRarityWithLuck(UsedLuck), ItemMeshData, Lib),
			this
		);

		if (!DroppedItem) return;

		FVector InitialSpawnLoc = GetActorLocation() + GetActorForwardVector() * 100.f;
		FVector FinalLoc = CalculateValidDropLocation(InitialSpawnLoc);

		FActorSpawnParameters Params;
		Params.Owner = this;
		Params.Instigator = this;

		//Use Deferred Spawning
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
			Dropped->ItemData = DroppedItem->ItemData;
			Dropped->Initialize(DroppedItem->ItemData);
			Dropped->FinishSpawning(FTransform(FRotator::ZeroRotator, FinalLoc));
			Dropped->SetReplicates(true);
			Dropped->SetReplicateMovement(true);
		}
	}
}

void AEnemyPawn::ApplyForceToEnemy(FVector forces)
{
	if (CapsuleComponent->GetCollisionEnabled() == ECollisionEnabled::PhysicsOnly || CapsuleComponent->GetCollisionEnabled() == ECollisionEnabled::QueryAndPhysics)
	{
		CapsuleComponent->SetSimulatePhysics(true);
		CapsuleComponent->AddImpulse(forces, NAME_None, true);
		CapsuleComponent->SetNotifyRigidBodyCollision(true);

		MovementComponent->IsKnockedBack = true;

		if (GetWorld())
		{
			GetWorld()->GetTimerManager().SetTimer(KnockBackFallBackHandle, this, &AEnemyPawn::KnockBackFallBack, 20.f, false);
		}
	}
}

void AEnemyPawn::KnockBack_Implementation(FVector forces)
{
	ApplyForceToEnemy(forces);
}

void AEnemyPawn::GroundCheck(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (!MovementComponent || !CapsuleComponent || !OtherComp)
	{
		return;
	}

	if (!FMath::IsNearlyZero(CapsuleComponent->GetComponentVelocity().Length()) && Hit.Normal.Z < 0.1)
	{
		return;
	}

	ECollisionChannel otherChannel = OtherComp->GetCollisionObjectType();

	if (MovementComponent->IsKnockedBack && otherChannel == ECollisionChannel::ECC_GameTraceChannel3/*(otherChannel == ECollisionChannel::ECC_WorldStatic || otherChannel == ECollisionChannel::ECC_WorldDynamic)*/)
	{
		ResetKnock();

		if (GetWorld() && GetWorld()->GetTimerManager().IsTimerActive(KnockBackFallBackHandle))
		{
			GetWorld()->GetTimerManager().ClearTimer(KnockBackFallBackHandle);
		}
	}
}

void AEnemyPawn::ResetKnock_Implementation()
{
	if (!MovementComponent || !CapsuleComponent)
	{
		return;
	}

	MovementComponent->IsKnockedBack = false;
	CapsuleComponent->SetPhysicsLinearVelocity(FVector::ZeroVector);
	CapsuleComponent->SetSimulatePhysics(false);
	CapsuleComponent->SetNotifyRigidBodyCollision(false);
}

void AEnemyPawn::KnockBackFallBack()
{
	FNavLocation navLocation;

	UNavigationSystemV1* navSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());

	if (navSys->ProjectPointToNavigation(GetActorLocation(), navLocation, {5000,5000,5000}))
	{
		SetActorLocation(navLocation.Location);
	}

	ResetKnock();
}



FVector AEnemyPawn::CalculateValidDropLocation(FVector DesiredLoc) const
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

	return DesiredLoc;
}


// Called when the game starts or when spawned
void AEnemyPawn::BeginPlay()
{
	Super::BeginPlay();

	for(int i = 0; i < SkeletalMesh->GetNumMaterials(); i++)
	{
		DynamicMaterials.Add(SkeletalMesh->CreateAndSetMaterialInstanceDynamic(i));
	}


	
	//get refernce to objects that we need to get the director
	UWorld* World = GetWorld();
	APawn* enemy = static_cast<APawn*>(this);

	//call it next tick so their no problem with stuff not loaded in yet
	GetWorld()->GetTimerManager().SetTimerForNextTick(this, &AEnemyPawn::AddEnemyToDirector);

	IsAttacking = false;
	SetActorTickEnabled(false);

	

	

	if (HasAuthority())
	{
		HitFlashComponent->InitializeNewDynamicMaterials(SkeletalMesh);
		HitFlashComponent->InitializeNewDynamicMaterials(StaticMesh);
		ResetEnemy();
		MovementComponent->InitializeNavMeshPointer();
	}
}

void AEnemyPawn::PlayMontage_Implementation(UAnimMontage* Montage, float PlayRate)
{
	if (!SkeletalMesh || !SkeletalMesh->IsActive())
	{
		return;
	}

	UAnimInstance* AnimInstance = SkeletalMesh->GetAnimInstance();

	if (AnimInstance)
	{
		AnimInstance->Montage_Play(Montage, PlayRate);
	}
}



// Called every frame
void AEnemyPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (IsActive)
	{
		AttackTick();
	}
}


void AEnemyPawn::OnRep_IsActive()
{
	if (IsActive)
	{
		this->SetActorHiddenInGame(false);
		this->SetActorEnableCollision(true);
		//this->SetActorTickEnabled(this->GetShouldTick());
	}
	else
	{
		this->SetActorHiddenInGame(true);
		this->SetActorEnableCollision(false);
		//this->SetActorTickEnabled(false);
	}
}

bool AEnemyPawn::GetIsActive()
{
	return IsActive;
}

bool AEnemyPawn::GetShouldTick()
{
	return false;
}

void AEnemyPawn::SetIsActive(bool newIsActive)
{
	if (IsActive == newIsActive)
	{
		return;
	}

	FTimerHandle tempHandle;
	if (!newIsActive)
	{
		NetMultSetIsActive(newIsActive);
		GetWorld()->GetTimerManager().SetTimer(tempHandle, [this, newIsActive]()
			{
				this->IsActive = newIsActive;
			}, 2.f, false);
	}
	else
	{
		GetWorld()->GetTimerManager().SetTimer(tempHandle, [this, newIsActive]()
			{
				this->NetMultSetIsActive(newIsActive);
			}, 2.f, false);
		IsActive = newIsActive;
	}
	
/*	NetMultSetIsActive(newIsActive);
	IsActive = newIsActive;*/

	if (!newIsActive)
	{
		OnDeactivated.Broadcast(this);
		if (IsValid(MovementComponent))
		{
			MovementComponent->SetActive(false);
		}

		/*this->SetActorHiddenInGame(true);
		this->SetActorEnableCollision(false);*/
	}
	else
	{
		/*this->SetActorHiddenInGame(false);
		this->SetActorEnableCollision(true);*/
		ResetEnemy();
	}

	/*AObjectPoolManager* pool = AObjectPoolManager::GetInstance(this);

	if (IsValid(pool) && SetControllerActiveStatus && GetController()->GetClass()->ImplementsInterface(UPoolActorInterface::StaticClass()))
	{
		pool->SetActorActive(GetController(), newIsActive);
	}*/
}

void AEnemyPawn::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	//DOREPLIFETIME(AEnemyPawn, GoldEnemy);
	//DOREPLIFETIME(AEnemyPawn, IsActive);
}

FTimerHandle& AEnemyPawn::GetLifeSpanTimerHandle()
{
	return LifeSpanTimerHandle;
}

void AEnemyPawn::NetMultSetIsActive_Implementation(bool newIsActive)
{
	if (newIsActive)
	{
		this->SetActorHiddenInGame(false);
		this->SetActorEnableCollision(true);
		this->CapsuleComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		//this->SetActorTickEnabled(this->GetShouldTick());
	}
	else
	{
		this->SetActorHiddenInGame(true);
		this->SetActorEnableCollision(false);
		this->CapsuleComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		//this->SetActorTickEnabled(false);
	}
}

#if WITH_EDITOR

void AEnemyPawn::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	FName PropertyName = NAME_None;

	if (PropertyChangedEvent.Property)
	{
		PropertyName = PropertyChangedEvent.Property->GetFName();
	}

	if (PropertyName == GET_MEMBER_NAME_CHECKED(AEnemyPawn, LODComponent))
	{
		SetNetCullDistanceSquared(FMath::Square(LODComponent->GetDistanceLOD(LODComponent->GetNumberOfLODs() - 1)));
	}
}
#endif




