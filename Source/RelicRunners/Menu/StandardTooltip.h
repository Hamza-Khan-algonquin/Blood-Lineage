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
	void Setup(const FText& Title, const FText& Value, const TArray<FText>& NewDescriptions);

	void SetAnchor(FVector2D anchor);

	void SetIcon(UTexture2D* NewIcon, const FLinearColor& NewColor);

protected:
	virtual void NativeConstruct() override;

	UPROPERTY(meta = (BindWidget))
	class UImage* Icon;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* TB_Name;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* TB_Value;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* TB_Description1;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* TB_Description2;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* TB_Description3;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* TB_Description4;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* TB_Description5;

	UPROPERTY(meta = (BindWidget))
	class UBorder* B_Description1;

	UPROPERTY(meta = (BindWidget))
	class UBorder* B_Description2;

	UPROPERTY(meta = (BindWidget))
	class UBorder* B_Description3;

	UPROPERTY(meta = (BindWidget))
	class UBorder* B_Description4;

	UPROPERTY(meta = (BindWidget))
	class UBorder* B_Description5;

	UPROPERTY(meta = (BindWidget))
	class UBorder* B_Base;
};
