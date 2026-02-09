// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "FlagsObjective.generated.h"

UCLASS()
class RELICRUNNERS_API UFlagsObjective : public UUserWidget
{
	GENERATED_BODY()
public:

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* TB_Text;

	UPROPERTY(meta = (BindWidget))
	class UImage* FlagImage_0;

	UPROPERTY(meta = (BindWidget))
	class UImage* FlagImage_1;

	UPROPERTY(meta = (BindWidget))
	class UImage* FlagImage_2;
};
