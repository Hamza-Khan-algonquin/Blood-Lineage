// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RelicRunners/Enemy/Characters/EnemyCharacter.h"
#include "MinotaurCharacter.generated.h"

class UCapsuleComponent;
class UNiagaraSystem;
UCLASS()
class RELICRUNNERS_API AMinotaurCharacter : public AEnemyCharacter
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UNiagaraSystem> SlamEffect;

public:
	AMinotaurCharacter();
	UFUNCTION(BlueprintCallable, meta = (ToolTip = "Method that is set for the OnOverlap delegate of a collider, that handles dealing damage to the player.", ShortToolTip = "OnOverlap delegate."))
	//Method that is set for the OnOverlap delegate of a collider, that handles dealing damage to the player
	void BasicAttack(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION(NetMulticast, Reliable, BlueprintCallable)
	void NetMultPlayParticleSlam(FVector position, float raduis, float sizeOfMesh);

	UFUNCTION(BlueprintCallable)
	void DelaySpikeCheck(float delay, float raduis, float damageMult, TArray<FVector> locations);
};
