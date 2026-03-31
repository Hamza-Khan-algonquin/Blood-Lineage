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

#include "RelicRunners/PlayerState/RelicRunnersPlayerState.h"
#include "RelicRunners/RelicRunnersCharacter.h"
#include <Kismet/GameplayStatics.h>
#include <Net/UnrealNetwork.h>
#include "Components/SphereComponent.h"
#include "NiagaraComponent.h"

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

	HitFlashTimeline.SetLooping(false);

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
	GetWorld()->GetTimerManager().ClearTimer(AttackHandle);
	Super::EndPlay(EndPlayReason);
}

float AEnemyPawn::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (!HasAuthority() || DamageAmount <= 0)
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
		Die(EventInstigator, (GetActorLocation() - DamageCauser->GetActorLocation().GetSafeNormal()));
	}
	//if we are not dead check the damage events for stun
	else
	{
		HitFlash();

		if (DamageEvent.IsOfType(FAttackDamageEvent::ClassID))
		{
			const FAttackDamageEvent* AttackDamageEvent = static_cast<const FAttackDamageEvent*>(&DamageEvent);
			if (AttackDamageEvent && ((AttackDamageEvent->AttackType == EAttackType::LIGHT_ATTACK || AttackDamageEvent->AttackType == EAttackType::HEAVY_ATTACK) && EnemyStatsComponent->GetEnemyType() == EEnemyType::LIGHT))
			{
				FVector Direction = (GetActorLocation() - DamageCauser->GetActorLocation()).GetSafeNormal();
				KnockBack(Direction * AttackDamageEvent->KnockBackStrength + FVector(0, 0, AttackDamageEvent->AdditionalUpwardsKnockBackForces));
			}
		}

		UpdateEmissiveHealth();
	}

	return DamageAmount;
}

void AEnemyPawn::HitFlash_Implementation()
{
	if (HitSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, HitSound, GetActorLocation());
	}

	HitFlashTimeline.PlayFromStart();
	SetActorTickEnabled(true);
}

void AEnemyPawn::UpdateHitFlash(float strength)
{
	for (UMaterialInstanceDynamic* element : DynamicMaterials)
	{
		if (element)
		{
			element->SetScalarParameterValue("HitFlashValue", strength);
		}
	}
}

void AEnemyPawn::EndHitFlash()
{
	HitFlashTimeline.Stop();
	UpdateHitFlash(0);
	if (!IsAttacking)
	{
		SetActorTickEnabled(false);
	}
}

void AEnemyPawn::Die(AController* EventInstigator, FVector hitDirection)
{
	//make sure we are server side
	if (HasAuthority())
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
	
	EndHitFlash();

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

		//if (deathEffect)
		//{
		//	deathEffect->SetVectorParameter(TEXT("HitDirection"), -impactDirection.GetSafeNormal());
		//}
	}
}

inline bool AEnemyPawn::IsDead()
{
	return EnemyStatsComponent->GetCurrentHealth() <= 0;
}

void AEnemyPawn::ResetEnemy()
{
	EnemyStatsComponent->SetCurrentHealth(EnemyStatsComponent->GetMaxHealth());

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

			FTimerDelegate attackCallBack;
			attackCallBack.BindLambda([this]()
				{
					if (this)
					{
						this->IsAttacking = false;
						if(this->MovementComponent)
							MovementComponent->SetMovementMode(EBoidMovementMode::Boid);
					}
				});
			GetWorldTimerManager().SetTimer(AttackHandle, attackCallBack, montage->GetPlayLength(), false);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Attack Animation Missing"));
			MovementComponent->SetMovementMode(EBoidMovementMode::Boid);
		}

		
		if (!HitFlashTimeline.IsPlaying())
		{
			SetActorTickEnabled(false);
		}
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
	if (GoldEnemy) chance = 0;

	if (chance <= chanceToDropOverall)
	{
		if (ItemMeshDataClass)
		{
			ItemMeshData = ItemMeshDataClass->GetDefaultObject<UItemMeshData>();
		}

		UIconLibrary* Lib = GI->GlobalIconLibrary;

		float GoldLuck = 0;
		float UsedLuck = Luck;
		if (GoldEnemy) GoldLuck = 95; UsedLuck = GoldLuck;

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
	CapsuleComponent->SetSimulatePhysics(true);
	CapsuleComponent->AddImpulse(forces, NAME_None, true);
	CapsuleComponent->SetNotifyRigidBodyCollision(true);

	MovementComponent->IsKnockedBack = true;
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
		ResetKnockBackStuff();
	}
}

void AEnemyPawn::ResetKnockBackStuff_Implementation()
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

	float hueOffset = FMath::FRand();
	float goldChance = FMath::FRandRange(0.f, 100.0f);
	if (goldChance <= 1.f) GoldEnemy = true;

	if (GoldEnemy)
	{
		EnemyStatsComponent->SetMaxHealth(EnemyStatsComponent->GetMaxHealth() * 4);
		EnemyStatsComponent->SetCurrentHealth(EnemyStatsComponent->GetMaxHealth());
		SetActorScale3D(GetActorScale3D() * 1.25);
	}

	for (UMaterialInstanceDynamic* element : DynamicMaterials)
	{
		if (element)
		{
			//element->SetScalarParameterValue("HueOffset", hueOffset);
			element->SetScalarParameterValue("IsGold", GoldEnemy);
		}
	}
	
	//get refernce to objects that we need to get the director
	UWorld* World = GetWorld();
	APawn* enemy = static_cast<APawn*>(this);

	//call it next tick so their no problem with stuff not loaded in yet
	GetWorld()->GetTimerManager().SetTimerForNextTick(this, &AEnemyPawn::AddEnemyToDirector);

	IsAttacking = false;
	SetActorTickEnabled(false);

	if (HitFlashCurve)
	{
		HitFlashTimeline.SetTimelineLengthMode(ETimelineLengthMode::TL_LastKeyFrame);

		FOnTimelineFloat inProgressFunc;
		inProgressFunc.BindUFunction(this, FName("UpdateHitFlash"));
		HitFlashTimeline.AddInterpFloat(HitFlashCurve, inProgressFunc);

		//FOnTimelineEvent endFunc;
		//endFunc.BindUFunction(this, FName("EndHitFlash"));
		//HitFlashTimeline.SetTimelineFinishedFunc(endFunc);
	}

	

	if (HasAuthority())
	{
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

	HitFlashTimeline.TickTimeline(DeltaTime);
}

void AEnemyPawn::OnRep_IsActive()
{
	if (IsActive)
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

	//NetMultSetIsActive(newIsActive);
	IsActive = newIsActive;
	

	if (!newIsActive)
	{
		OnDeactivated.Broadcast(this);
		if (IsValid(MovementComponent))
		{
			MovementComponent->SetActive(false);
		}

		this->SetActorHiddenInGame(true);
		this->SetActorEnableCollision(false);
	}
	else
	{
		this->SetActorHiddenInGame(false);
		this->SetActorEnableCollision(true);
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
	DOREPLIFETIME(AEnemyPawn, GoldEnemy);
	DOREPLIFETIME(AEnemyPawn, IsActive);
}

FTimerHandle& AEnemyPawn::GetLifeSpanTimerHandle()
{
	return LifeSpanTimerHandle;
}

void AEnemyPawn::NetMultSetIsActive_Implementation(bool newIsActive)
{
	if (newIsActive)
	{

		this->SetActorEnableCollision(true);
		this->SetActorTickEnabled(this->GetShouldTick());
	}
	else
	{

		this->SetActorEnableCollision(false);
		this->SetActorTickEnabled(false);
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




