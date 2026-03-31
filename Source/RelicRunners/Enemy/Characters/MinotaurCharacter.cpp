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

	FTimerHandle temp;

	GetWorld()->GetTimerManager().SetTimer(temp, [&]() {
			if (!GetWorld() && !IsValid(this) && !IsValid(EnemyStatsComponent))
			{
				return;
			}

			for (FVector location : locations)
			{
#if WITH_EDITOR
				DrawDebugSphere(GetWorld(), location, raduis, 12, FColor::Red, 2, 1);
#endif
				TArray<AActor*> actorToIngore;
				TArray<AActor*> hitActors;
				TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
				ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));
				UKismetSystemLibrary::SphereOverlapActors(GetWorld(), location, raduis, ObjectTypes, nullptr, actorToIngore, hitActors);

				for (AActor* hit : hitActors)
				{
					if (hit->Tags.Contains("Player"))
					{
						UGameplayStatics::ApplyDamage(hit, damageMult * EnemyStatsComponent->GetAttackDamage(), GetController(), this, nullptr);
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
