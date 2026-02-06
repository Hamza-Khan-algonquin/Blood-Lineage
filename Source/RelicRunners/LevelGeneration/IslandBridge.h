// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "IslandBridge.generated.h"

UCLASS()
class RELICRUNNERS_API AIslandBridge : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AIslandBridge();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UStaticMeshComponent* Bridge;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UStaticMeshComponent* Barrier;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UStaticMeshComponent* BackBarrier;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	void Activate();

	void SetFinalBridge(bool isFinal);

	UFUNCTION()
	void OnRep_OnBlockedChange();

	UPROPERTY(ReplicatedUsing = OnRep_OnBlockedChange)
	bool IsBlocked;

	UFUNCTION()
	void OnRep_OnFinalBridgeChange();

	UPROPERTY(ReplicatedUsing = OnRep_OnFinalBridgeChange)
	bool IsFinalBridge;
};
