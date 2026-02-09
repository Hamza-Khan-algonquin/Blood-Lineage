// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TitleObjective.generated.h"

UCLASS()
class RELICRUNNERS_API UTitleObjective : public UUserWidget
{
	GENERATED_BODY()
public:

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* TB_Title;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* TB_Text;
};
