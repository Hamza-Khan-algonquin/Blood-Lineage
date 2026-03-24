// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ObjectiveBase.h"
#include "RelicRunners/Interact/InteractInterface.h"
#include "NewCapturableFlag.generated.h"

/**
 * 
 */
UCLASS()
class RELICRUNNERS_API ANewCapturableFlag : public AObjectiveBase, public IInteractInterface
{
	GENERATED_BODY()
	
public:

	ANewCapturableFlag();

	virtual void Interact_Implementation(class ARelicRunnersCharacter* Char) override;
	virtual void ShowTooltip_Implementation(bool bShow) override;

	UPROPERTY(VisibleAnywhere)
	class UWidgetComponent* TooltipWidgetComponent;
	UPROPERTY(EditAnywhere)
	TSubclassOf<UUserWidget> TooltipWidgetClass;
	UFUNCTION(Server, Reliable)
	void Server_HandleInteracted(class ARelicRunnersCharacter* Character);

	void HandleInteracted(class ARelicRunnersCharacter* Character);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Default Values")
	class UBoxComponent* TriggerBox;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Default Values")
	class UTextRenderComponent* TextComponent;

	UFUNCTION()
	void OnActiveChange();

	UFUNCTION(BlueprintImplementableEvent)
	void PlayFlagAnimation();
	virtual void BeginPlay() override;
	void InitializeTooltipWidget();
private:
	virtual void Tick(float DeltaTime) override;
	UPROPERTY(ReplicatedUsing = OnActiveChange)
	bool isActive = false;
};