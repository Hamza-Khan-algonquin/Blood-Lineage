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
    UMaterialInstanceDynamic* DynMaterial = UMaterialInstanceDynamic::Create(BaseMaterial, this);
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
	FName UniqueName = MakeUniqueObjectName(this, UTextureRenderTarget2D::StaticClass(), *FString::Printf(TEXT("MinimapRT_%s"), *Owner->GetName()));
	UTextureRenderTarget2D* NewRenderTarget = NewObject<UTextureRenderTarget2D>(this, UniqueName);
	NewRenderTarget->RenderTargetFormat = RTF_RGBA16f;
	NewRenderTarget->InitAutoFormat(200, 200);
	NewRenderTarget->ClearColor = FLinearColor(0, 0, 0, 0);
	NewRenderTarget->bAutoGenerateMips = false;
	NewRenderTarget->UpdateResourceImmediate(true);

	Owner->MinimapCaptureComponent->TextureTarget = NewRenderTarget;
	Owner->MinimapCaptureComponent->CaptureSource = ESceneCaptureSource::SCS_SceneColorHDR;
	Owner->MinimapCaptureComponent->ShowFlags.SetAtmosphere(false);
	Owner->MinimapCaptureComponent->ShowFlags.SetFog(false);
	Owner->MinimapCaptureComponent->ShowFlags.SetDynamicShadows(false);

	UMaterialInterface* PostMat = LoadObject<UMaterialInterface>(
		nullptr, TEXT("/Game/Minimap/M_PostProcess.M_PostProcess"));

	if (PostMat)
	{
		UE_LOG(LogTemp, Warning, TEXT("[SceneCapture] Successfully set post process material."));
		Owner->MinimapCaptureComponent->PostProcessSettings.AddBlendable(PostMat, 1.0f);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[SceneCapture] Failed to set post process material."));
	}

	RenderTarget = NewRenderTarget;
	return RenderTarget;
}