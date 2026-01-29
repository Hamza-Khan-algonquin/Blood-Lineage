// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Minimap.generated.h"

/**
 * 
 */
UCLASS()
class RELICRUNNERS_API UMinimap : public UUserWidget
{
	GENERATED_BODY()
	
	void SetMinimapImage(UTextureRenderTarget2D* RenderTarget);

	UPROPERTY(meta = (BindWidget))
	class UImage* I_Minimap;
};
