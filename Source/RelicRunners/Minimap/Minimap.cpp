// Fill out your copyright notice in the Description page of Project Settings.


#include "Minimap.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Engine/TextureRenderTarget2D.h"
#include <RelicRunners/RelicRunnersCharacter.h>
#include "Components/SceneCaptureComponent2D.h"

void UMinimap::SetMinimapImage(UTextureRenderTarget2D* NewRenderTarget)
{
	if (!NewRenderTarget || !I_Minimap) return;

	if (!MinimapBaseMaterial)
	{
		UE_LOG(LogTemp, Error, TEXT("[UI-Minimap] Base Material is null! Assign it in the Widget Blueprint."));
		return;
	}

	UMaterialInstanceDynamic* DynMaterial = UMaterialInstanceDynamic::Create(MinimapBaseMaterial, GetOwningPlayer());
	if (DynMaterial)
	{
		DynMaterial->SetTextureParameterValue(FName("MinimapTexture"), NewRenderTarget);
		I_Minimap->SetBrushFromMaterial(DynMaterial);
		UE_LOG(LogTemp, Warning, TEXT("[UI-Minimap] Success! RenderTarget set."));
	}
}

UTextureRenderTarget2D* UMinimap::CreateRenderTarget(ARelicRunnersCharacter* Owner)
{
	//Create and assign a unique render target
	FString UniqueNameStr = FString::Printf(TEXT("RT_Minimap_%s_%u"),
		*Owner->GetName(),
		FPlatformTime::Cycles());

	RenderTarget = NewObject<UTextureRenderTarget2D>(this, FName(*UniqueNameStr));
	RenderTarget->RenderTargetFormat = RTF_RGBA16f;
	RenderTarget->InitAutoFormat(2000, 2000);
	RenderTarget->ClearColor = FLinearColor(0,0,0,0);
	RenderTarget->bAutoGenerateMips = false;
	RenderTarget->UpdateResource();

	Owner->MinimapCaptureComponent->TextureTarget = RenderTarget;
	Owner->MinimapCaptureComponent->CaptureSource = ESceneCaptureSource::SCS_SceneColorHDR;
	Owner->MinimapCaptureComponent->ShowFlags.SetAtmosphere(false);
	Owner->MinimapCaptureComponent->ShowFlags.SetFog(false);
	Owner->MinimapCaptureComponent->ShowFlags.SetDynamicShadows(false);

	return RenderTarget;
}

void UMinimap::SetLevelTitle(FText title)
{
	TB_Title->SetText(title);
}