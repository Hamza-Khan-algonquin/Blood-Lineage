// Fill out your copyright notice in the Description page of Project Settings.


#include "ArtemisCharacter.h"
#include "RelicRunners/PlayerState/RelicRunnersPlayerState.h"
#include <RelicRunners/Item/ItemStats.h>
#include "RelicRunners/Inventory/InventoryComponent.h"
#include "RelicRunners/Inventory/Inventory.h"
#include <RelicRunners/Classes/ClassInfo.h>
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "RelicRunners/Enemy/Characters/EnemyCharacter.h"
#include "RelicRunners/Enemy/Characters/EnemyCharacterAI.h"
#include "Net/UnrealNetwork.h"
#include "RelicRunners/Abilities/PiercingArrow.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "RelicRunners/Enemy/EnemyPawn.h"


AArtemisCharacter::AArtemisCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	bProjectileActive = false;
	ProjectileSpeed = 5000.f;
	ProjectileLifetime = 2.f;
	ProjectileDamage = 70.0f;

}

void AArtemisCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (AbilitySystem)
	{
		// Give Mark of the Hunt ability
		AbilitySystem->GiveAbility(FGameplayAbilitySpec(UMarkOfTheHunt::StaticClass(), 1, 0));

		// Give Moonbeam ability
		AbilitySystem->GiveAbility(FGameplayAbilitySpec(UMoonbeam::StaticClass(), 1, 0));

		// Give NatureBlessing ability
		AbilitySystem->GiveAbility(FGameplayAbilitySpec(UNaturesBlessing::StaticClass(), 1, 0));

        // Give PiercingArrow ability
		AbilitySystem->GiveAbility(FGameplayAbilitySpec(UPiercingArrow::StaticClass(), 1, 0));
	}
}

// Called when player takes control of this character
void AArtemisCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	// Get PlayerState and set up ability system
	ARelicRunnersPlayerState* PS = GetPlayerState<ARelicRunnersPlayerState>();
	if (PS)
	{
		AbilitySystem = PS->GetAbilitySystemComponent();
		if (AbilitySystem)
		{
			AbilitySystem->InitAbilityActorInfo(PS, this);
		}
	}

	if (AbilitySystem)
	{
		// Give Abilities
		GivePiercingArrow();
		GiveNaturesBlessing();
		GiveMarkOfTheHunt();
		GiveMoonBeam();
	}
}

// Called when PlayerState is replicated on clients
void AArtemisCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	// Reinitialize ability system
	ARelicRunnersPlayerState* PS = GetPlayerState<ARelicRunnersPlayerState>();
	if (PS)
	{
		AbilitySystem = PS->GetAbilitySystemComponent();
		AbilitySystem->InitAbilityActorInfo(PS, this);
	}
}
void AArtemisCharacter::ActivateDamageAbilities()
{
	Super::ActivateDamageAbilities();

	if (AbilitySystem)
	{
		AbilitySystem->TryActivateAbilityByClass(UPiercingArrow::StaticClass());
	}
}

void AArtemisCharacter::ActivateDefenceAbilities()
{
	Super::ActivateDefenceAbilities();

	if (AbilitySystem)
	{

		AbilitySystem->TryActivateAbilityByClass(UNaturesBlessing::StaticClass());
	}
}

void AArtemisCharacter::ActivateUtilityAbilities()
{
	Super::ActivateUtilityAbilities();

	if (AbilitySystem)
	{
		// Activate Mark of the Hunt
		AbilitySystem->TryActivateAbilityByClass(UMarkOfTheHunt::StaticClass());
	}
}

void AArtemisCharacter::ActivateUltimateAbilities()
{
	Super::ActivateUltimateAbilities();

	if (AbilitySystem)
	{
		// Activate Moonbeam now (fires the beam)
		AbilitySystem->TryActivateAbilityByClass(UMoonbeam::StaticClass());
	}

}

void AArtemisCharacter::GivePiercingArrow()
{
	if (!AbilitySystem) return;

	if (!AbilitySystem->FindAbilitySpecFromClass(UPiercingArrow::StaticClass()))
	{
		AbilitySystem->GiveAbility(FGameplayAbilitySpec(UPiercingArrow::StaticClass(), 1, 0));
	}
}

void AArtemisCharacter::GiveNaturesBlessing()
{
	if (!AbilitySystem) return;

	if (!AbilitySystem->FindAbilitySpecFromClass(UNaturesBlessing::StaticClass()))
	{
		AbilitySystem->GiveAbility(FGameplayAbilitySpec(UNaturesBlessing::StaticClass(), 1, 0));
	}
}

void AArtemisCharacter::GiveMarkOfTheHunt()
{
	if (!AbilitySystem) return;

	if (!AbilitySystem->FindAbilitySpecFromClass(UMarkOfTheHunt::StaticClass()))
	{
		AbilitySystem->GiveAbility(FGameplayAbilitySpec(UMarkOfTheHunt::StaticClass(), 1, 0));
	}
}

void AArtemisCharacter::GiveMoonBeam()
{
	if (!AbilitySystem) return;

	if (!AbilitySystem->FindAbilitySpecFromClass(UMoonbeam::StaticClass()))
	{
		AbilitySystem->GiveAbility(FGameplayAbilitySpec(UMoonbeam::StaticClass(), 1, 0));
	}
}



void AArtemisCharacter::PlayRandomAttackAnimation()
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

void AArtemisCharacter::PerformSwingTrace()
{
}
void AArtemisCharacter::ShowReticle()
{
	if (!ReticleWidget && ReticleWidgetClass)
	{
		ReticleWidget = CreateWidget<UUserWidget>(GetWorld(), ReticleWidgetClass);
		if (ReticleWidget)
		{
			ReticleWidget->AddToViewport();
			ReticleWidget->SetRenderTranslation(FVector2D(-80.f, 0.f));
		}
	}
}

void AArtemisCharacter::HideReticle()
{
	if (ReticleWidget)
	{
		ReticleWidget->RemoveFromParent();
		ReticleWidget = nullptr;
	}
}

void AArtemisCharacter::UpdateCamera(float DeltaTime)
{
	if (!CameraBoom) return;

	if (bIsAiming)
	{
		ShowReticle();

		CameraBoom->bEnableCameraRotationLag = false;
		CameraBoom->bUsePawnControlRotation = false;
		CameraBoom->bEnableCameraLag = false;
		CameraBoom->TargetArmLength = FMath::FInterpTo(CameraBoom->TargetArmLength, AimingArmLength, DeltaTime, AimingInterpSpeed);

		// Smoothly interpolate socket offset 
		CameraBoom->SocketOffset = FMath::VInterpTo(CameraBoom->SocketOffset,AimingOffset, DeltaTime,AimingInterpSpeed);

		// Rotate player using controller input
		if (Controller)
		{
			FRotator ControlRot = Controller->GetControlRotation();
			FRotator ActorRot = GetActorRotation();

			ActorRot.Yaw = ControlRot.Yaw;
			SetActorRotation(ActorRot);
		}

		// Camera strictly follows player facing direction
		FRotator FacingRot = GetActorRotation();
		FacingRot.Pitch = 0.f;
		FacingRot.Roll = 0.f;
		CameraBoom->SetWorldRotation(FacingRot);

	}
	else
	{
		HideReticle();
		CameraBoom->bEnableCameraLag = true;
		CameraBoom->bEnableCameraRotationLag = true;
		CameraBoom->bUsePawnControlRotation = true;
		// Movement info 
		FVector Velocity = GetVelocity();
		Velocity.Z = 0.f;
		float Speed = Velocity.Size();

		// Detect nearby enemies 
		bool bEnemiesNearby = false;
		AActor* NearestEnemy = nullptr;
		float NearestDistSq = FLT_MAX;

		TArray<AActor*> OverlappingActors;
		TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
		ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));

		TArray<AActor*> ActorsToIgnore;
		ActorsToIgnore.Add(this);

		UKismetSystemLibrary::SphereOverlapActors(GetWorld(), GetActorLocation(), 300.f, ObjectTypes, ACharacter::StaticClass(), ActorsToIgnore, OverlappingActors);

		for (AActor* Actor : OverlappingActors)
		{
			if (!Actor || !Actor->ActorHasTag("Enemy")) continue;

			bEnemiesNearby = true;

			float DistSq = FVector::DistSquared(Actor->GetActorLocation(), GetActorLocation());
			if (DistSq < NearestDistSq)
			{
				NearestDistSq = DistSq;
				NearestEnemy = Actor;
			}
		}

		// Dynamic zoom 
		float CombatArmLength = BaseArmLength + 50.f;

		float TargetArm;
		if (bEnemiesNearby)
		{
			TargetArm = CombatArmLength;
		}
		else if (Speed > 600.f)
		{
			TargetArm = SprintArmLength;
		}
		else
		{
			TargetArm = BaseArmLength;
		}

		if (bEnemiesNearby)
		{
			TargetArm = FMath::Max(TargetArm, BaseArmLength + 75.f);
		}

		CameraBoom->TargetArmLength = FMath::FInterpTo(CameraBoom->TargetArmLength, TargetArm, DeltaTime, 4.f);

		CameraBoom->TargetArmLength = FMath::Clamp(CameraBoom->TargetArmLength, BaseArmLength * 0.9f, SprintArmLength);

		// Camera lag
		float TargetCameraLagSpeed = bEnemiesNearby ? 2.f : 8.f;
		float TargetCameraRotationLagSpeed = bEnemiesNearby ? 2.f : 6.f;

		CameraBoom->CameraLagSpeed = FMath::FInterpTo(CameraBoom->CameraLagSpeed, TargetCameraLagSpeed, DeltaTime, 8.f);

		CameraBoom->CameraRotationLagSpeed = FMath::FInterpTo(CameraBoom->CameraRotationLagSpeed, TargetCameraRotationLagSpeed, DeltaTime, 8.f);

		// Look-ahead
		FVector DesiredLookAhead = Velocity.GetSafeNormal() * LookAheadDistance;
		CurrentLookAhead = FMath::VInterpTo(CurrentLookAhead, DesiredLookAhead, DeltaTime, 3.f);

		float AdjustedHeight = HeightOffset + (bEnemiesNearby ? 20.f : 0.f);
		CameraBoom->SocketOffset = FVector(0.f, 0.f, AdjustedHeight) + CurrentLookAhead;

		if (Speed > 10.f)
		{
			FRotator DesiredRot = Velocity.ToOrientationRotator();

			if (NearestEnemy && NearestDistSq > FMath::Square(150.f))
			{
				FVector DirToEnemy = NearestEnemy->GetActorLocation() - GetActorLocation();
				DirToEnemy.Z = 0.f;
				DirToEnemy.Normalize();

				float EnemyYaw = DirToEnemy.Rotation().Yaw;
				float DeltaYaw = FMath::FindDeltaAngleDegrees(DesiredRot.Yaw, EnemyYaw);
				DesiredRot.Yaw += DeltaYaw * 0.2f;
			}

			FRotator CurrentRot = CameraBoom->GetComponentRotation();

			DesiredRot.Pitch = FMath::Clamp(CurrentRot.Pitch, MinPitch, MaxPitch);
			DesiredRot.Roll = 0.f;

			float DeltaYaw = FMath::FindDeltaAngleDegrees(CurrentRot.Yaw, DesiredRot.Yaw);

			float NewYaw = CurrentRot.Yaw + DeltaYaw * DeltaTime * CameraYawSmoothSpeed;

			float NewPitch = FMath::FInterpTo(CurrentRot.Pitch, DesiredRot.Pitch, DeltaTime, CameraPitchSmoothSpeed);

			float BounceOffset = FMath::Sin(GetWorld()->GetTimeSeconds() * BounceSpeed) * BounceAmplitude;

			FRotator Smoothed(NewPitch + BounceOffset * 0.1f, NewYaw, 0.f);
			CameraBoom->SetWorldRotation(Smoothed);
		}
		else
		{
			if (!Controller) return;
			FRotator ControlRot = Controller->GetControlRotation();
			ControlRot.Pitch = FMath::Clamp(ControlRot.Pitch, MinPitch, MaxPitch);
			ControlRot.Roll = 0.f;

			FRotator CurrentCamRot = CameraBoom->GetComponentRotation();

			float CamDeltaYaw = FMath::FindDeltaAngleDegrees(CurrentCamRot.Yaw, ControlRot.Yaw);

			float NewCamYaw = CurrentCamRot.Yaw + CamDeltaYaw * DeltaTime * CameraYawSmoothSpeed;

			float NewCamPitch = FMath::FInterpTo(CurrentCamRot.Pitch, ControlRot.Pitch, DeltaTime, CameraPitchSmoothSpeed);

			CameraBoom->SetWorldRotation(FRotator(NewCamPitch, NewCamYaw, 0.f));


			FRotator ActorRot = GetActorRotation();

			float ActorDeltaYaw = FMath::FindDeltaAngleDegrees(ActorRot.Yaw, NewCamYaw);

			ActorRot.Yaw += ActorDeltaYaw * DeltaTime * 2.5f;
			SetActorRotation(ActorRot);
		}
	}
}


void AArtemisCharacter::BasicAttack()
{
	//if (!bIsAiming) return;

	if (!RelicPlayerState || !RelicPlayerState->GetInventoryComponent()) return;
	//if (!CanBasicAttack) return;
	if (!HasValidWeaponEquipped(RelicPlayerState->GetInventoryComponent())) return;

	CanBasicAttack = true;

	ProjectileDamage = 70.0f;

	// Spawn projectile
	bProjectileActive = true;
	ProjectileLocation = GetActorLocation() + GetActorForwardVector() * 100.f + FVector(0, 0, 50.f);
	ProjectileVelocity = GetActorForwardVector() * ProjectileSpeed;
	ProjectileElapsed = 0.f;

	PlayRandomAttackAnimation();
	AlreadyHitActors.Empty();

	// Spawn static mesh
	if (ProjectileMeshAsset)
	{
		if (!ActiveProjectileMesh)
		{
			ActiveProjectileMesh = NewObject<UStaticMeshComponent>(this);
			ActiveProjectileMesh->RegisterComponent();
			ActiveProjectileMesh->SetStaticMesh(ProjectileMeshAsset);
			ActiveProjectileMesh->SetWorldLocation(ProjectileLocation);
			ActiveProjectileMesh->SetWorldScale3D(FVector(1.f));
			ActiveProjectileMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

			// Initial rotation based on velocity
			ActiveProjectileMesh->SetWorldRotation(ProjectileVelocity.Rotation());
		}
	}

	// Cooldown
	GetWorldTimerManager().SetTimer(TimerHandle_AttackCooldown, this, &AArtemisCharacter::ResetBasicAttack, BasicAttackCooldown, false);
}

void AArtemisCharacter::HeavyAttack()
{
	bIsAiming = true;

	if (!RelicPlayerState || !RelicPlayerState->GetInventoryComponent()) return;
	if (!CanHeavyAttack) return;
	if (!HasValidWeaponEquipped(RelicPlayerState->GetInventoryComponent())) return;

	CanHeavyAttack = false;

	ProjectileDamage = 100.0f;

	// Spawn projectile
	bProjectileActive = true;
	ProjectileLocation = GetActorLocation() + GetActorForwardVector() * 100.f + FVector(0, 0, 50.f);
	ProjectileVelocity = GetActorForwardVector() * ProjectileSpeed;
	ProjectileElapsed = 0.f;

	PlayRandomAttackAnimation();
	AlreadyHitActors.Empty();

	// Spawn static mesh
	if (ProjectileMeshAsset)
	{
		if (!ActiveProjectileMesh)
		{
			ActiveProjectileMesh = NewObject<UStaticMeshComponent>(this);
			ActiveProjectileMesh->RegisterComponent();
			ActiveProjectileMesh->SetStaticMesh(ProjectileMeshAsset);
			ActiveProjectileMesh->SetWorldLocation(ProjectileLocation);
			ActiveProjectileMesh->SetWorldScale3D(FVector(1.f));
			ActiveProjectileMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

			// Initial rotation based on velocity
			ActiveProjectileMesh->SetWorldRotation(ProjectileVelocity.Rotation());
		}
	}

	// Cooldown
	GetWorldTimerManager().SetTimer(TimerHandle_AttackCooldown, this, &AArtemisCharacter::ResetHeavyAttack, HeavyAttackCooldown, false);
}

void AArtemisCharacter::ProjectileHeavyAttack()
{
	bIsAiming = false;
}


void AArtemisCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bProjectileActive) return;

	ProjectileElapsed += DeltaTime;

	// Apply gravity
	ProjectileVelocity.Z -= ProjectileGravity * DeltaTime;

	PreviousProjectileLocation = ProjectileLocation;
	ProjectileLocation += ProjectileVelocity * DeltaTime;

	// Move and rotate the mesh to match velocity
	if (ActiveProjectileMesh)
	{
		ActiveProjectileMesh->SetWorldLocation(ProjectileLocation);
		ActiveProjectileMesh->SetWorldRotation(ProjectileVelocity.Rotation());
	}

	// Collision
	TArray<FHitResult> Hits;
	FCollisionShape Sphere = FCollisionShape::MakeSphere(15.f);

	bool bHit = GetWorld()->SweepMultiByChannel(Hits,PreviousProjectileLocation,ProjectileLocation,FQuat::Identity,ECC_GameTraceChannel1,Sphere);

	for (const FHitResult& Hit : Hits)
	{
		if (AActor* HitActor = Hit.GetActor())
		{
			if (AlreadyHitActors.Contains(HitActor))
				continue;

			if (AEnemyCharacter* Enemy = Cast<AEnemyCharacter>(HitActor))
			{
				Server_HitEnemy(Enemy);
			}
			else if (AEnemyCharacterAI* TEnemy = Cast<AEnemyCharacterAI>(HitActor))
			{
				Server_HitEnemy(TEnemy);
			}
			else if (AEnemyPawn* TPEnemy = Cast<AEnemyPawn>(HitActor))
			{
				Server_HitEnemy(TPEnemy);
			}

			AlreadyHitActors.Add(HitActor);

			// Stop projectile on hit
			bProjectileActive = false;

			if (ActiveProjectileMesh)
			{
				ActiveProjectileMesh->DestroyComponent();
				ActiveProjectileMesh = nullptr;
			}
			break;
		}
	}

	// Stop projectile after lifetime
	if (ProjectileElapsed >= ProjectileLifetime)
	{
		bProjectileActive = false;
		if (ActiveProjectileMesh)
		{
			ActiveProjectileMesh->DestroyComponent();
			ActiveProjectileMesh = nullptr;
		}
	}
}


void AArtemisCharacter::Server_HitEnemy_Implementation(AActor* HitActor)
{
	if (HitActor)
	{
		UGameplayStatics::ApplyDamage(HitActor, ProjectileDamage, GetController(), this, nullptr);
	}
}