// Fill out your copyright notice in the Description page of Project Settings.


#include "TormentedSoulCharacter.h"
#include <RelicRunners/Item/ItemActor.h>
#include "RelicRunners/RelicRunnersCharacter.h"
#include <Net/UnrealNetwork.h>
#include "Components/WidgetComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"
#include "SoldierController.h"
#include "RelicRunners/RelicRunnersCharacter.h"

ATormentedSoulCharacter::ATormentedSoulCharacter() : Super()
{
	//set up the collider
	AttackOverlapCollider = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Attack Collider"));
	AttackOverlapCollider->BodyInstance.SetCollisionProfileName("OverlapAll");
	AttackOverlapCollider->SetupAttachment(GetMesh());
	EnemyName = "Tormented Soul";

	//make sure to set the default controller to the right one
	AIControllerClass = ASoldierController::StaticClass();
}

float ATormentedSoulCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	float actualDamage = DamageAmount;  

	//ask Anto if I can fix his applay damage call because he is not assigning the EventInstigator properly
	if (EventInstigator && !EventInstigator->Tags.Contains("Player") && !IsAttacking)
	{
		actualDamage *= 0.5;
	}

	return Super::TakeDamage(actualDamage, DamageEvent, EventInstigator, DamageCauser);
}

void ATormentedSoulCharacter::BasicAttack(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	UE_LOG(LogTemp, Warning, TEXT("Basic Attacked called"));
	//check to make sure we are attacking and that we did overlap with the player
	if (IsAttacking && OtherActor->Tags.Contains("Player"))
	{
		UGameplayStatics::ApplyDamage(OtherActor, AttackDamage, this->Controller, this, nullptr);
	}
}
