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
		AbilitySystem->TryActivateAbilityByClass(BP_PiercingArrowClass);
	}
}

void AArtemisCharacter::ActivateDefenceAbilities()
{
	Super::ActivateDefenceAbilities();

	if (AbilitySystem)
	{

		AbilitySystem->TryActivateAbilityByClass(BP_ImpunityClass);
	}
}

void AArtemisCharacter::ActivateUtilityAbilities()
{
	Super::ActivateUtilityAbilities();

	if (AbilitySystem)
	{
		// Activate Mark of the Hunt
		AbilitySystem->TryActivateAbilityByClass(BP_MarkOfTheHuntClass);
	}
}

void AArtemisCharacter::ActivateUltimateAbilities()
{
	Super::ActivateUltimateAbilities();

	if (AbilitySystem)
	{
		// Activate Moonbeam now (fires the beam)
		AbilitySystem->TryActivateAbilityByClass(BP_MoonbeamClass);
	}

}

void AArtemisCharacter::GivePiercingArrow()
{
	if (!AbilitySystem) return;

	if (!AbilitySystem->FindAbilitySpecFromClass(BP_PiercingArrowClass))
	{
		AbilitySystem->GiveAbility(FGameplayAbilitySpec(BP_PiercingArrowClass, 1, 0));
	}
}

void AArtemisCharacter::GiveNaturesBlessing()
{
	if (!AbilitySystem) return;

	if (!AbilitySystem->FindAbilitySpecFromClass(BP_ImpunityClass))
	{
		AbilitySystem->GiveAbility(FGameplayAbilitySpec(BP_ImpunityClass, 1, 0));
	}
}

void AArtemisCharacter::GiveMarkOfTheHunt()
{
	if (!AbilitySystem) return;

	if (!AbilitySystem->FindAbilitySpecFromClass(BP_MarkOfTheHuntClass))
	{
		AbilitySystem->GiveAbility(FGameplayAbilitySpec(BP_MarkOfTheHuntClass, 1, 0));
	}
}

void AArtemisCharacter::GiveMoonBeam()
{
	if (!AbilitySystem) return;

	if (!AbilitySystem->FindAbilitySpecFromClass(BP_MoonbeamClass))
	{
		AbilitySystem->GiveAbility(FGameplayAbilitySpec(BP_MoonbeamClass, 1, 0));
	}
}



void AArtemisCharacter::PlayRandomAttackAnimation()
{
	if (!BasicAnim)
		return;

	USkeletalMeshComponent* SkeletalMeshComponent = GetMesh();
	if (!SkeletalMeshComponent)
		return;

	UAnimInstance* AnimInstance = SkeletalMeshComponent->GetAnimInstance();
	if (!AnimInstance)
		return;

	AnimInstance->Montage_Play(BasicAnim, 1.f);
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
	const FString MapName = GetWorld()->GetMapName();
	if (MapName.Contains(TEXT("BossLevel"))) return;

	if (bIsAiming)
	{
		ShowReticle();

		// Disable lag while aiming
		CameraBoom->bEnableCameraLag = false;
		CameraBoom->bEnableCameraRotationLag = false;

		// Smoothly interpolate arm length and offset
		CameraBoom->TargetArmLength = FMath::FInterpTo(CameraBoom->TargetArmLength, AimingArmLength, DeltaTime, AimingInterpSpeed);

		CameraBoom->SocketOffset = FMath::VInterpTo(CameraBoom->SocketOffset, AimingOffset, DeltaTime, AimingInterpSpeed);

		// Rotate player to controller yaw
		if (Controller)
		{
			FRotator ControlRot = Controller->GetControlRotation();
			FRotator ActorRot = GetActorRotation();
			ActorRot.Yaw = ControlRot.Yaw;
			SetActorRotation(ActorRot);
		}

		// Rotate camera to face player
		FRotator CurrentRot = CameraBoom->GetComponentRotation();
		FRotator TargetRot = GetActorRotation();
		TargetRot.Pitch = 0.f;
		TargetRot.Roll = 0.f;

		CameraBoom->SetWorldRotation(FMath::RInterpTo(CurrentRot, TargetRot, DeltaTime, AimingInterpSpeed));

		return; // early exit so normal camera logic doesn’t run
	}
	else
	{
		HideReticle();
	}

	FVector Velocity = GetVelocity();
	Velocity.Z = 0.f;
	float Speed = Velocity.Size();

	bool bEnemiesNearby = false;
	AActor* NearestEnemy = nullptr;
	float NearestDistSq = FLT_MAX;

	DetectEnemies(bEnemiesNearby, NearestEnemy, NearestDistSq);

	UpdateCameraPosition(DeltaTime, Velocity, Speed, bEnemiesNearby);

	UpdateCameraRotation(DeltaTime, Velocity, Speed);
}

void AArtemisCharacter::DetectEnemies(bool& bEnemiesNearby, AActor*& NearestEnemy, float& NearestDistSq)
{
	bEnemiesNearby = false;
	NearestEnemy = nullptr;
	NearestDistSq = FLT_MAX;

	TArray<AActor*> OverlappingActors;
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_GameTraceChannel1));

	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(this);

	UKismetSystemLibrary::SphereOverlapActors(GetWorld(), GetActorLocation(), 500.f, ObjectTypes, APawn::StaticClass(), ActorsToIgnore, OverlappingActors);

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

}
void AArtemisCharacter::UpdateCameraPosition(float DeltaTime, const FVector& Velocity, float Speed, bool bEnemiesNearby)
{
	// arm length adjustments
	float CombatArmLength = BaseArmLength + 150.f;

	float TargetArm;
	if (bEnemiesNearby)
		TargetArm = CombatArmLength;
	else if (Speed > 600.f)
		TargetArm = SprintArmLength;
	else
		TargetArm = BaseArmLength;

	CameraBoom->TargetArmLength = FMath::FInterpTo(CameraBoom->TargetArmLength, TargetArm, DeltaTime, 3.5f);

	// lag
	float TargetLag = bEnemiesNearby ? 15.f : 10.f;
	float TargetRotLag = bEnemiesNearby ? 16.f : 10.f;

	CameraBoom->CameraLagSpeed = FMath::FInterpTo(CameraBoom->CameraLagSpeed, TargetLag, DeltaTime, 10.f);

	CameraBoom->CameraRotationLagSpeed = FMath::FInterpTo(CameraBoom->CameraRotationLagSpeed, TargetRotLag, DeltaTime, 10.f);

	// look ahead
	FVector DesiredLookAhead = Velocity.GetSafeNormal() * LookAheadDistance;

	CurrentLookAhead = FMath::VInterpTo(CurrentLookAhead, DesiredLookAhead, DeltaTime, 5.f);

	float TargetHeightOffset = bEnemiesNearby ? HeightOffset + 60.f : HeightOffset;

	float SmoothedHeight = FMath::FInterpTo(CameraBoom->SocketOffset.Z, TargetHeightOffset, DeltaTime, 3.5f);

	FVector TargetSocketOffset = FVector(0.f, 0.f, SmoothedHeight) + CurrentLookAhead;

	CameraBoom->SocketOffset = FMath::VInterpTo(CameraBoom->SocketOffset, TargetSocketOffset, DeltaTime, 4.f);
}

void AArtemisCharacter::UpdateCameraRotation(float DeltaTime, const FVector& Velocity, float Speed)
{
	// moving
	if (Speed > 10.f)
	{
		FRotator CurrentRot = CameraBoom->GetComponentRotation();

		FRotator DesiredRot = CurrentRot;

		FRotator MovementRot = Velocity.ToOrientationRotator();

		float MoveDelta = FMath::FindDeltaAngleDegrees(CurrentRot.Yaw, MovementRot.Yaw);

		DesiredRot.Yaw = CurrentRot.Yaw + MoveDelta * 0.35f;

		DesiredRot.Roll = 0.f;

		float FinalDelta = FMath::FindDeltaAngleDegrees(CurrentRot.Yaw, DesiredRot.Yaw);

		float NewYaw = CurrentRot.Yaw + FinalDelta * DeltaTime * 30.f;

		float NewPitch = FMath::FInterpTo(CurrentRot.Pitch, DesiredRot.Pitch, DeltaTime, CameraPitchSmoothSpeed);

		CameraBoom->SetWorldRotation(FRotator(NewPitch, NewYaw, 0.f));
	}
	// idle
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

		ActorRot.Yaw += ActorDeltaYaw * DeltaTime * CameraYawSmoothSpeed;

		SetActorRotation(ActorRot);
	}
}

void AArtemisCharacter::BasicAttack()
{
	FProjectileData NewProj;

	NewProj.Location = GetActorLocation() + GetActorForwardVector() * 50.f + FVector(0, 0, 50.f);
	NewProj.Velocity = GetActorForwardVector() * ProjectileSpeed;
	NewProj.Elapsed = 0.f;
	NewProj.CurrentPierces = 0;

	// Spawn mesh
	if (ProjectileMeshAsset)
	{
		UStaticMeshComponent* ProjectileMesh = NewObject<UStaticMeshComponent>(this);
		ProjectileMesh->RegisterComponent();
		ProjectileMesh->SetStaticMesh(ProjectileMeshAsset);
		ProjectileMesh->SetWorldLocation(NewProj.Location);
		ProjectileMesh->SetWorldScale3D(FVector(1.f));
		ProjectileMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		ProjectileMesh->SetWorldRotation(NewProj.Velocity.Rotation());

		NewProj.Mesh = ProjectileMesh;
	}

	ActiveProjectiles.Add(NewProj);
	PlayRandomAttackAnimation();
}

void AArtemisCharacter::HeavyAttack()
{
	bIsAiming = true;

}

void AArtemisCharacter::ProjectileHeavyAttack()
{
	bIsAiming = false;
}


void AArtemisCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	for (int32 i = ActiveProjectiles.Num() - 1; i >= 0; i--)
	{
		FProjectileData& P = ActiveProjectiles[i];

		P.Elapsed += DeltaTime;

		// Apply gravi
		P.Velocity.Z -= ProjectileGravity * DeltaTime;

		FVector PreviousLocation = P.Location;
		P.Location += P.Velocity * DeltaTime;

		// Move mesh
		if (P.Mesh)
		{
			P.Mesh->SetWorldLocation(P.Location);
			P.Mesh->SetWorldRotation(P.Velocity.Rotation());
		}

		// Collision
		TArray<FHitResult> Hits;
		FCollisionShape Sphere = FCollisionShape::MakeSphere(15.f);

		bool bHit = GetWorld()->SweepMultiByChannel(Hits,PreviousLocation,P.Location,FQuat::Identity,ECC_GameTraceChannel1,Sphere);

		for (const FHitResult& Hit : Hits)
		{
			AActor* HitActor = Hit.GetActor();
			if (!HitActor || P.HitActors.Contains(HitActor))
				continue;

			bool bEnemyHit = false;

			if (AEnemyCharacter* Enemy = Cast<AEnemyCharacter>(HitActor))
			{
				Server_HitEnemy(Enemy, ProjectileDamage);
				bEnemyHit = true;
			}
			else if (AEnemyCharacterAI* TEnemy = Cast<AEnemyCharacterAI>(HitActor))
			{
				Server_HitEnemy(TEnemy, ProjectileDamage);
				bEnemyHit = true;
			}
			else if (AEnemyPawn* TPEnemy = Cast<AEnemyPawn>(HitActor))
			{
				Server_HitEnemy(TPEnemy, ProjectileDamage);
				bEnemyHit = true;
			}

			if (bEnemyHit)
			{
				P.HitActors.Add(HitActor);
				P.CurrentPierces++;

				if (P.CurrentPierces >= MaxPierces)
				{
					if (P.Mesh)
					{
						P.Mesh->DestroyComponent();
					}

					ActiveProjectiles.RemoveAt(i);
					break;
				}
			}
		}

		// Lifetime check
		if (i < ActiveProjectiles.Num() && P.Elapsed >= ProjectileLifetime)
		{
			if (P.Mesh)
			{
				P.Mesh->DestroyComponent();
			}

			ActiveProjectiles.RemoveAt(i);
		}
	}
}


