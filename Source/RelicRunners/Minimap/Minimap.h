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

public:
	void SetMinimapImage(UTextureRenderTarget2D* RenderTarget);

	class UTextureRenderTarget2D* CreateRenderTarget(class ARelicRunnersCharacter* Owner);

	void SetLevelTitle(FText title);

	UPROPERTY(meta = (BindWidget))
	class UImage* I_Minimap;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UTextureRenderTarget2D* RenderTarget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Minimap")
	TObjectPtr<UMaterialInterface> MinimapBaseMaterial;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* TB_Title;
};
