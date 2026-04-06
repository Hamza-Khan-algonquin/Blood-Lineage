// Fill out your copyright notice in the Description page of Project Settings.

#include "MinotaurCharacter.h"
#include "Components/CapsuleComponent.h"
#include "RelicRunners/Enemy/EnemyStats.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetSystemLibrary.h"

AMinotaurCharacter::AMinotaurCharacter()
{

	//AttackOverlapCollider->SetupAttachment(GetMesh());
	EnemyStatsComponent->SetName("Minotaur");
	EnemyStatsComponent->SetEnemyType(EEnemyType::MEDIUM);
	OverrideCollisionType = true;
}

void AMinotaurCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorld()->GetTimerManager().ClearTimer(SpikeTimer);

	Super::EndPlay(EndPlayReason);
}

void AMinotaurCharacter::BasicAttack(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor->Tags.Contains("Player"))
	{
		UGameplayStatics::ApplyDamage(OtherActor, EnemyStatsComponent->GetAttackDamage(), this->Controller, this, nullptr);
	}
}

void AMinotaurCharacter::DelaySpikeCheck(float delay, float raduis, float damageMult, TArray<FVector> locations)
{
	if (!GetWorld())
	{
		return;
	}

	
	TArray<FVector> newLocations = locations;

	TWeakObjectPtr<AMinotaurCharacter> weakThis = this;
	TWeakObjectPtr<UWorld> weakWorld = GetWorld();

	GetWorld()->GetTimerManager().SetTimer(SpikeTimer, [weakThis, weakWorld, newLocations, delay, raduis, damageMult]()
		{
			if (!weakWorld.IsValid() && !weakThis.IsValid())
			{
				return;
			}

			for (int i = 0; i < newLocations.Num(); i++)
			{
#if WITH_EDITOR
				//DrawDebugSphere(weakWorld.Get(), newLocations[i], raduis, 12, FColor::Red, false, 2, 1);
#endif
				TArray<AActor*> actorToIngore;
				TArray<AActor*> hitActors;
				TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
				ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));
				UKismetSystemLibrary::SphereOverlapActors(weakWorld.Get(), newLocations[i], raduis, ObjectTypes, nullptr, actorToIngore, hitActors);

				for (AActor* hit : hitActors)
				{
					if (hit->Tags.Contains("Player"))
					{
						UGameplayStatics::ApplyDamage(hit, damageMult * weakThis->EnemyStatsComponent->GetAttackDamage(), weakThis->GetController(), weakThis.Get(), nullptr);
					}
				}
			}
		}, delay, false);

	
}

void AMinotaurCharacter::NetMultPlayParticleSlam_Implementation(FVector position, float radius, float sizeOfMesh)
{
	if (SlamEffect)
	{
		UNiagaraComponent* effect = UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), SlamEffect, position, GetActorRotation(), GetActorScale());
		effect->SetNiagaraVariableFloat(FString("SlamRadius"), radius);
		effect->SetNiagaraVariableFloat(FString("SizeOfMesh"), sizeOfMesh);
	}
}
