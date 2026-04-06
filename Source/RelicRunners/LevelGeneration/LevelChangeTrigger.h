// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Sound/SoundBase.h"
#include "LevelChangeTrigger.generated.h"

UCLASS()
class RELICRUNNERS_API ALevelChangeTrigger : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ALevelChangeTrigger();

	virtual void OnConstruction(const FTransform& transform) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnTriggerOverlap(UPrimitiveComponent* OverlapComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

public:	

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Default Values")
	class USceneComponent* Origin;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Default Values")
	class UBoxComponent* TriggerBox;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Default Values")
	class UTextRenderComponent* LevelTargetTextRender; //For debugging to make sure the box will send players to the right level

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Default Values")
	FString TargetLevel; //Level name players will be sent to. Recieved from the level generator.

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Default Values")
	FString BossLevel; //Boss Level name players will be sent to. Recieved from the level generator.

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
	TObjectPtr<USoundBase> PortalEnterSound = nullptr;

	FTimerHandle LevelTravelHandle;

	UFUNCTION(Server, Reliable)
	void Server_SetTargetLevel(FName newTargetLevel);

	UFUNCTION()
	void ChangeLevel();

	UFUNCTION(Server, Reliable)
	void Server_ChangeLevel();

	UFUNCTION(Server, Reliable)
	void Server_Activate();

	UPROPERTY(Replicated)
	bool IsActive = false;

	UPROPERTY(EditAnywhere)
	bool CanGoToBoss = false;
};
