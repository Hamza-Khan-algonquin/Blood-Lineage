// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "RelicRunners/Interact/InteractInterface.h"
#include "ForgeActor.generated.h"

UCLASS()
class RELICRUNNERS_API AForgeActor : public ACharacter, public IInteractInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AForgeActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY()
	class UInventory* LinkedInventoryWidget;
	UPROPERTY(VisibleAnywhere)
	class UWidgetComponent* TooltipWidgetComponent;
	UPROPERTY(EditAnywhere)
	TSubclassOf<UUserWidget> TooltipWidgetClass;

	void InitializeTooltipWidget();
	// IInteractInterface
	virtual void Interact_Implementation(ARelicRunnersCharacter* Char) override;
	virtual FItemData GetItemData_Implementation() override;
	virtual void ShowTooltip_Implementation(bool bShow) override;
};
