// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Sound/SoundBase.h"
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

	UPROPERTY(EditAnywhere, Category = Materials)
	UMaterialInterface* GlowBlue;

	UPROPERTY(EditAnywhere, Category = Materials)
	UMaterialInterface* GlowRed;

	UPROPERTY(EditDefaultsOnly, Category = "Effects")
	class UNiagaraSystem* CrystalExplosion;

	virtual void BeginPlay() override;
	UPROPERTY(VisibleAnywhere)
	bool bWasTriggered = false;

	FTimerHandle RageTimerHandle;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(ReplicatedUsing = OnRep_IsRaging)
	bool bIsRaging = false;

	UPROPERTY(Replicated)
	float CurrentRotationMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
	TObjectPtr<USoundBase> ExplosionSound = nullptr;

	float InitialZ;
	float RunningTime;

	UFUNCTION()
	void OnRep_IsRaging();

	void UpdateRageVisuals();
public:	

	virtual void Tick(float DeltaTime) override;
};
