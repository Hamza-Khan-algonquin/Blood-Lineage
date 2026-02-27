// Fill out your copyright notice in the Description page of Project Settings.

#include "MinotaurCharacter.h"
#include "Components/CapsuleComponent.h"
#include "RelicRunners/Enemy/EnemyStats.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

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
