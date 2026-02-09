// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "EnemiesObjective.generated.h"

UCLASS()
class RELICRUNNERS_API UEnemiesObjective : public UUserWidget
{
	GENERATED_BODY()
public:

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* TB_Text;

	UPROPERTY(meta = (BindWidget))
	class UImage* EnemyImage;
};
