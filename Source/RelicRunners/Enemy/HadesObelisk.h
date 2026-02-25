// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HadesObelisk.generated.h"

UCLASS()
class RELICRUNNERS_API AHadesObelisk : public AActor
{
	GENERATED_BODY()
	
public:	

	AHadesObelisk();
	void TriggerRageMode();

	void EndRageMode();

	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;
protected:

	UPROPERTY(EditAnywhere, Category = "Visuals")
	UStaticMesh* ObeliskMeshAsset;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UStaticMeshComponent* ObeliskComponent;

	UPROPERTY(EditAnywhere, Category = "Visuals")
	UStaticMesh* ShieldMeshAsset;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UStaticMeshComponent* ShieldComponent;

	virtual void BeginPlay() override;
	UPROPERTY(VisibleAnywhere)
	bool bWasTriggered = false;

	bool bIsRaging = false;
	float CurrentRotationMultiplier = 1.0f;
	FTimerHandle RageTimerHandle;
public:	

	virtual void Tick(float DeltaTime) override;
};
