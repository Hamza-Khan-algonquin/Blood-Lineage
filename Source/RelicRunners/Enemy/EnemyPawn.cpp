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

	AttackCollider = CreateDefaultSubobject<USphereComponent>("Attack Collider");
	AttackCollider->SetupAttachment(RootComponent);
	AttackCollider->SetRelativeLocation(FVector(0, 25, 0));
	AttackCollider->SetSphereRadius(25);

	EnemyStatsComponent = CreateDefaultSubobject<UEnemyStats>(TEXT("Enemy Stats Component"));
	EnemyStatsComponent->SetEnemyType(EEnemyType::LIGHT);
	EnemyStatsComponent->SetChanceToDropItem(0.02f);
	EnemyStatsComponent->SetMaxHealth(100.f);
	EnemyStatsComponent->SetCurrentHealth(100.f);

	bReplicates = true;
	SetReplicateMovement(true);

	//times per second it replicates
	SetNetUpdateFrequency(5.f);
	SetMinNetUpdateFrequency(2.f);
	bAlwaysRelevant = false;
	SetNetCullDistanceSquared(FMath::Square(LODComponent->GetDistanceLOD(LODComponent->GetNumberOfLODs() - 1)));

	KnockBackStrength = 1000;
	AdditionalUpwardsKnockBackForces = 50;
	Tags.Add("Enemy");
}

void AEnemyPawn::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorld()->GetTimerManager().ClearTimer(LifeSpanTimerHandle);
	GetWorld()->GetTimerManager().ClearTimer(AttackHandle);
	Super::EndPlay(EndPlayReason);
}

void AEnemyPawn::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

}

float AEnemyPawn::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (!HasAuthority() || DamageAmount <= 0)
	{
		return 0;
	}

	if (HasAuthority())
	{
		HitFlash();

		FVector Direction = (GetActorLocation() - DamageCauser->GetActorLocation()).GetSafeNormal();
		//CapsuleComponent->AddImpulse(Direction * KnockBackStrength + FVector(0,0, AdditionalUpwardsKnockBackForces), NAME_None, true);

		//this->MovementComponent->IsKnockedBack = true;

		FTimerHandle temp;
	/*	GetWorldTimerManager().SetTimer(temp, [this]() {
				if (this)
				{
					if (!this)
						return;
					if(this->MovementComponent)
						this->MovementComponent->IsKnockedBack = false;
					if (this->CapsuleComponent)
						this->CapsuleComponent->SetPhysicsLinearVelocity(FVector::ZeroVector);
				}
			}, HitFlashTimeline.GetTimelineLength(), false);*/
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
			if (AttackDamageEvent && ((AttackDamageEvent->AttackType == EAttackType::LIGHT_ATTACK || AttackDamageEvent->AttackType == EAttackType::HEAVY_ATTACK) && EnemyStatsComponent->GetEnemyType() == EEnemyType::LIGHT))
			{
				//RemainingStunTime = AttackDamageEvent->StunDuration;
			}
		}
	}

	return DamageAmount;
}

void AEnemyPawn::HitFlash_Implementation()
{
	HitFlashTimeline.PlayFromStart();
	SetActorTickEnabled(true);
	//HitFlashTimeline.SetPlaybackPosition(0.0f, false);
	//HitFlashTimeline.Play();
/*	for (UMaterialInstanceDynamic* element : DynamicMaterials)
	{
		if (element)
		{
			element->SetScalarParameterValue("HitFlashValue", 10.0f);
		}
	}
	//DynamicMaterial->SetScalarParameterValue("HitFlashValue", 10.0f);

	FTimerHandle temp;
	GetWorldTimerManager().SetTimer(temp, [this]() {
		for (UMaterialInstanceDynamic* element : this->DynamicMaterials)
		{
			if (element)
			{
				element->SetScalarParameterValue("HitFlashValue", 0.0f);
			}
		}
	}, 0.5f, false);*/
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
	if (!IsAttacking)
	{
		SetActorTickEnabled(false);
	}
}

void AEnemyPawn::Die(AController* EventInstigator)
{
	//make sure we are server side
	if (HasAuthority())
	{
		//spawn item
		SpawnItem();

		//check to make sure it a play and if do add experience
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
		//OnDestroyed.Clear();

		AObjectPoolManager* PoolManager = AObjectPoolManager::GetInstance(this);

		if (PoolManager && PoolManager->IsActorClassPooled(this->GetClass()))
		{
			SetIsActive(false);
		}
		else
		{
			Destroy();
		}
	}
}

inline bool AEnemyPawn::IsDead()
{
	return EnemyStatsComponent->GetCurrentHealth() <= 0;
}

void AEnemyPawn::ResetEnemy()
{
	EnemyStatsComponent->SetCurrentHealth(EnemyStatsComponent->GetMaxHealth());
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
	}
}

float AEnemyPawn::CalculateXP() const
{
	return FMath::RandRange(10, 25);
}

void AEnemyPawn::SpawnItem()
{
	if (!IsValid(EnemyStatsComponent)) return;

	float chance = FMath::FRandRange(0.f, 1.f);

	if (chance <= EnemyStatsComponent->GetChanceToDropItem())
	{
		if (ItemMeshDataClass)
		{
			ItemMeshData = ItemMeshDataClass->GetDefaultObject<UItemMeshData>();
		}

		UItemObject* DroppedItem = ItemStats::CreateItemFromData(
			ItemStats::CreateSpecificItemData(EnemyStatsComponent->GetLevel(), ItemStats::GetRandomItemType(), ItemMeshData),
			this
		);

		if (!DroppedItem) return;

		FVector InitialSpawnLoc = GetActorLocation() + GetActorForwardVector() * 100.f;
		FVector FinalLoc = CalculateValidDropLocation(InitialSpawnLoc);

		FActorSpawnParameters Params;
		Params.Owner = this;
		Params.Instigator = this;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

		AItemActor* Dropped = GetWorld()->SpawnActor<AItemActor>(ItemActorClass, FinalLoc, FRotator::ZeroRotator, Params);

		if (Dropped)

		{
			Dropped->Initialize(DroppedItem->ItemData);
			Dropped->SetReplicates(true);
			Dropped->SetReplicateMovement(true);
		}

		/*FVector SpawnLoc = GetActorLocation() + GetActorForwardVector() * 100.f;
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
		}*/
	}
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

	if (HitFlashCurve)
	{
		HitFlashTimeline.SetTimelineLengthMode(ETimelineLengthMode::TL_LastKeyFrame);

		FOnTimelineFloat inProgressFunc;
		inProgressFunc.BindUFunction(this, FName("UpdateHitFlash"));

		HitFlashTimeline.AddInterpFloat(HitFlashCurve, inProgressFunc);

		//putting this here for a future note of what do if I want to call something at the end
		//HitFlashTimeline.SetTimelineFinishedFunc(FinishedFunction);
	}
	
	if (HasAuthority())
	{
		//get refernce to objects that we need to get the director
		UWorld* World = GetWorld();
		APawn* enemy = static_cast<APawn*>(this);

		//call it next tick so their no problem with stuff not loaded in yet
		GetWorld()->GetTimerManager().SetTimerForNextTick(this, &AEnemyPawn::AddEnemyToDirector);

		IsAttacking = false;
		SetActorTickEnabled(false);
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

	IsActive = newIsActive;
	IPoolActorInterface::SetIsActive(newIsActive);

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

FTimerHandle& AEnemyPawn::GetLifeSpanTimerHandle()
{
	return LifeSpanTimerHandle;
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




