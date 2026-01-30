// Fill out your copyright notice in the Description page of Project Settings.


#include "Minimap.h"
#include "Components/Image.h"
#include "Engine/TextureRenderTarget2D.h"
#include <RelicRunners/RelicRunnersCharacter.h>
#include "Components/SceneCaptureComponent2D.h"

void UMinimap::SetMinimapImage(UTextureRenderTarget2D* NewRenderTarget)
{
    if (!NewRenderTarget || !I_Minimap) return;
    
    UMaterialInterface* BaseMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Minimap/M_MiniMap_TextureRenderTarget.M_MiniMap_TextureRenderTarget"));

    if (!BaseMaterial)
    {
        UE_LOG(LogTemp, Error, TEXT("[UI-Minimap] Failed to load base material."));
        return;
    }

    //Create the dynamic material
    UMaterialInstanceDynamic* DynMaterial = UMaterialInstanceDynamic::Create(BaseMaterial, GetOwningPlayer());
    if (!DynMaterial)
    {
        UE_LOG(LogTemp, Error, TEXT("[UI-Minimap] Failed to create dynamic material."));
        return;
    }

    DynMaterial->SetTextureParameterValue(FName("MinimapTexture"), NewRenderTarget);
    I_Minimap->SetBrushFromMaterial(DynMaterial);

    UE_LOG(LogTemp, Warning, TEXT("[UI-Minimap] Set preview image with RenderTarget: %s"), *GetNameSafe(NewRenderTarget));
}

UTextureRenderTarget2D* UMinimap::CreateRenderTarget(ARelicRunnersCharacter* Owner)
{
	//Create and assign a unique render target
	FString UniqueNameStr = FString::Printf(TEXT("RT_Minimap_%s_%u"),
		*Owner->GetName(),
		FPlatformTime::Cycles());

	RenderTarget = NewObject<UTextureRenderTarget2D>(this, FName(*UniqueNameStr));
	RenderTarget->RenderTargetFormat = RTF_RGBA16f;
	RenderTarget->InitAutoFormat(200, 200);
	RenderTarget->ClearColor = FLinearColor(0,0,0,0);
	RenderTarget->bAutoGenerateMips = false;
	RenderTarget->UpdateResourceImmediate(true);

	Owner->MinimapCaptureComponent->TextureTarget = RenderTarget;
	Owner->MinimapCaptureComponent->CaptureSource = ESceneCaptureSource::SCS_SceneColorHDR;
	Owner->MinimapCaptureComponent->ShowFlags.SetAtmosphere(false);
	Owner->MinimapCaptureComponent->ShowFlags.SetFog(false);
	Owner->MinimapCaptureComponent->ShowFlags.SetDynamicShadows(false);

	return RenderTarget;
}