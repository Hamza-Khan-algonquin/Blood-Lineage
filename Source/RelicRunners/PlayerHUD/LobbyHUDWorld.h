// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LobbyHUDWorld.generated.h"

/**
 * 
 */
UCLASS()
class RELICRUNNERS_API ULobbyHUDWorld : public UUserWidget
{
	GENERATED_BODY()

public:
	void NativeConstruct();

	void UpdateHUDReadyStatus(APlayerState* TargetPS, bool IsReady);

	UFUNCTION()
	void SetHostIconVisibility(bool IsHost);
	UFUNCTION()
	void BindToPlayerState();

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* TB_Class;

	UPROPERTY(meta = (BindWidget))
	class UImage* I_Class;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* TB_Name;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* TB_Status;

	UPROPERTY(meta = (BindWidget))
	class UBorder* B_HostIcon;

	UPROPERTY()
	TMap<FName, UTexture2D*> IconCache;

	UPROPERTY()
	class ALobbyPreview* OwningPreview;

	UFUNCTION()
	void UpdatePlayerClassDisplay(FName NewClass);
	

};