// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RelicRunners/Interact/InteractInterface.h"
#include "CapturableFlag.generated.h"

UCLASS()
class RELICRUNNERS_API ACapturableFlag : public AActor, public IInteractInterface
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ACapturableFlag();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual void Interact_Implementation(class ARelicRunnersCharacter* Char) override;

	UFUNCTION(Server, Reliable)
	void Server_HandleInteracted(class ARelicRunnersCharacter* Character);

	void HandleInteracted(class ARelicRunnersCharacter* Character);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Default Values")
	class USceneComponent* Origin;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Default Values")
	class UBoxComponent* TriggerBox;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Default Values")
	class UTextRenderComponent* TextComponent;

private:
	UPROPERTY(Replicated)
	bool isActive = false;

};
