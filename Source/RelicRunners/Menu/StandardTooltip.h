// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "StandardTooltip.generated.h"

/**
 * 
 */
UCLASS()
class RELICRUNNERS_API UStandardTooltip : public UUserWidget
{
	GENERATED_BODY()
public:

	UPROPERTY(meta = (BindWidget))

	class UTextBlock* TB_Name;

	UPROPERTY(meta = (BindWidget))

	class UTextBlock* TB_Description1;

	UPROPERTY(meta = (BindWidget))

	class UTextBlock* TB_Description2;

	UPROPERTY(meta = (BindWidget))

	class UBorder* B_Description1;

	UPROPERTY(meta = (BindWidget))

	class UBorder* B_Description2;
};
