// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HitStunComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class RELICRUNNERS_API UHitStunComponent : public UActorComponent
{
	GENERATED_BODY()

protected:
	FTimerHandle HitStunTimer;

public:	
	// Sets default values for this component's properties
	UHitStunComponent();

protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason);
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	UFUNCTION(BlueprintCallable)
	void StartHitStun(float stunTime);
	UFUNCTION(BlueprintCallable, Reliable, NetMulticast)
	void Multicast_StartHitStun(float stunTime);
	UFUNCTION(BlueprintCallable)
	void EndHitStun();
	UFUNCTION(BlueprintCallable, Reliable, NetMulticast)
	void Multicast_EndHitStun();
};
