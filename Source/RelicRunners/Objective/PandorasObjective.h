// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PandorasObjective.generated.h"

UCLASS()
class RELICRUNNERS_API UPandorasObjective : public UUserWidget
{
	GENERATED_BODY()
public:

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* TB_Progression;

	UPROPERTY(meta = (BindWidget))
	class UProgressBar* ProgressionBar;
};
