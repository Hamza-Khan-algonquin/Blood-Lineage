// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyPawn.h"
#include "GameplayEffect.h"
#include "GameplayEffectExtension.h"
#include <Net/UnrealNetwork.h>
#include "RelicRunners/CustomDamageEvents.h"
#include "RelicRunners/RelicRunnersCharacter.h"
#include "RelicRunners/ObjectPool/ObjectPoolManager.h"
#include "RelicRunners/Director System/Director.h"
#include <Kismet/GameplayStatics.h>
#include "Components/CapsuleComponent.h"
#include "RelicRunners/Enemy/LOD Wrapper Components/EnemyLODSkeletalMeshComponent.h"
#include "RelicRunners/Enemy/LOD Wrapper Components/EnemyLODStaticMeshComponent.h"
#include "RelicRunners/Enemy/EnemyLODComponent.h"
#include "RelicRunners/Enemy/EnemyBoidMovementComponent.h"
#include "RelicRunners/Enemy/Controllers/CommanderController.h"


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
}

void AEnemyPawn::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AEnemyPawn, CurrentHealth);
	DOREPLIFETIME(AEnemyPawn, MaxHealth);
	DOREPLIFETIME(AEnemyPawn, TypeOfEnemy);
}

float AEnemyPawn::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
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
			if (AttackDamageEvent && ((AttackDamageEvent->AttackType == EAttackType::LIGHT_ATTACK || AttackDamageEvent->AttackType == EAttackType::HEAVY_ATTACK) && TypeOfEnemy == EEnemyType::LIGHT))
			{
				//RemainingStunTime = AttackDamageEvent->StunDuration;
			}
		}
	}

	return DamageAmount;
}

void AEnemyPawn::Die(AController* EventInstigator)
{
	//make sure we are server side
	if (HasAuthority())
	{
		//spawn item
		//SpawnItem();

		//check to make sure it a play and if do add experience
		if (EventInstigator)
		{
			if (ARelicRunnersCharacter* player = Cast<ARelicRunnersCharacter>(EventInstigator->GetPawn()))
			{
				//player->AddExperience(CalculateXP());
			}
		}

		OnDied.Broadcast(this);
		//OnDied.Clear();
		//OnDestroyed.Clear();

		AObjectPoolManager* PoolManager = AObjectPoolManager::GetInstance(this);

		if (PoolManager && PoolManager->IsActorClassPooled(this->GetClass()))
		{
			SetIsActive(false);
		}
		else
		{
			//be gone hud of my enemy
			/*if (EnemyHUDWorld)
			{
				EnemyHUDWorld->DestroyComponent();
			}*/

			Destroy();
		}
	}
}



void AEnemyPawn::SetCurrentHealth(const float& NewHealth)
{
	CurrentHealth = FMath::Clamp(NewHealth, 0.f, MaxHealth);
}

void AEnemyPawn::SetMaxHealth(const float& newHealth)
{
	MaxHealth = FMath::Max(0.f, newHealth);
	CurrentHealth = FMath::Min(CurrentHealth, MaxHealth);
}

void AEnemyPawn::ResetEnemy()
{
	CurrentHealth = MaxHealth;
}

void AEnemyPawn::AddEnemyToDirector()
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
		if (AttackMontage.IsValid())
		{
			UAnimMontage* montage = AttackMontage.Get();
			PlayMontage(montage);
			

			FTimerHandle tempHandle;
			FTimerDelegate attackCallBack;
			attackCallBack.BindLambda([this]()
				{
					MovementComponent->SetMovementMode(EBoidMovementMode::Boid);
				});
			GetWorldTimerManager().SetTimer(tempHandle, attackCallBack, montage->GetPlayLength(), false);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Attack Animation Missing"));
			MovementComponent->SetMovementMode(EBoidMovementMode::Boid);
		}

		IsAttacking = false;
		SetActorTickEnabled(false);
	}
}

// Called when the game starts or when spawned
void AEnemyPawn::BeginPlay()
{
	Super::BeginPlay();
	
	if (HasAuthority())
	{
		//get refernce to objects that we need to get the director
		UWorld* World = GetWorld();
		APawn* enemy = static_cast<APawn*>(this);

		//call it next tick so their no problem with stuff not loaded in yet
		GetWorld()->GetTimerManager().SetTimerForNextTick(this, &AEnemyPawn::AddEnemyToDirector);
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

void AEnemyPawn::BasicAttack(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor->Tags.Contains("Player"))
	{
		if (Controller)
		{
			UGameplayStatics::ApplyDamage(OtherActor, AttackDamage, Controller, this, nullptr);
		}
		else if (MovementComponent->Commander)
		{
			UGameplayStatics::ApplyDamage(OtherActor, AttackDamage, MovementComponent->Commander, this, nullptr);
		}
		else
		{
			UGameplayStatics::ApplyDamage(OtherActor, AttackDamage, nullptr, this, nullptr);
		}
	}
}


