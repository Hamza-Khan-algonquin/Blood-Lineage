// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Hades.generated.h"

UCLASS()
class RELICRUNNERS_API AHades : public AActor
{
	GENERATED_BODY()
	
public:	
	AHades();

	UPROPERTY(EditAnywhere, Category = "Effects")
	class UNiagaraSystem* GhostCycloneSystem;

	UPROPERTY()
	class UNiagaraComponent* ActiveCycloneComp;

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;
};
