// Fill out your copyright notice in the Description page of Project Settings.


#include "Minimap.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Engine/TextureRenderTarget2D.h"
#include <RelicRunners/RelicRunnersCharacter.h>
#include "GameFramework/SpringArmComponent.h"
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

	UWorld* World = GetWorld();
	FString MapName = World ? World->GetMapName() : "";

	bool IsBossLevel = MapName.Contains(TEXT("BossLevel"));

	float NormalZoom = 2000.f;
	float BossZoom = 5000.f;

	RenderTarget = NewObject<UTextureRenderTarget2D>(this, FName(*UniqueNameStr));
	RenderTarget->RenderTargetFormat = RTF_RGBA16f;
	RenderTarget->InitAutoFormat(1000, 1000);
	RenderTarget->ClearColor = FLinearColor(0,0,0,0);
	RenderTarget->UpdateResource();
	RenderTarget->Filter = TF_Bilinear;

	Owner->MinimapCaptureComponent->OrthoWidth = IsBossLevel ? BossZoom : NormalZoom;
	Owner->MinimapCaptureComponent->TextureTarget = RenderTarget;
	Owner->MinimapCaptureComponent->CaptureSource = ESceneCaptureSource::SCS_SceneColorHDR;
	Owner->MinimapCaptureComponent->ShowFlags.SetDynamicShadows(false);
	Owner->MinimapCaptureComponent->ShowFlags.SetContactShadows(false);
	Owner->MinimapCaptureComponent->ShowFlags.SetAmbientOcclusion(false);
	Owner->MinimapCaptureComponent->ShowFlags.SetScreenSpaceReflections(false);
	Owner->MinimapCaptureComponent->bUseRayTracingIfEnabled = false; 
	Owner->MinimapCaptureComponent->ShowFlags.SetTemporalAA(false); 
	Owner->MinimapCaptureComponent->ShowFlags.SetAntiAliasing(true);
	Owner->MinimapCaptureComponent->ShowFlags.SetSpecular(false);
	Owner->MinimapCaptureComponent->ShowFlags.SetFog(false);
	Owner->MinimapCaptureComponent->bCaptureEveryFrame = true;

	MinimapToonPostProcess = LoadObject<UMaterialInterface>(
		nullptr, TEXT("/Game/Minimap/M_MinimapPostProcess.M_MinimapPostProcess"));

	FPostProcessSettings& Settings = Owner->MinimapCaptureComponent->PostProcessSettings;
	Settings.FilmGrainIntensity = 0;
	Settings.bOverride_AutoExposureMethod = true;
	Settings.AutoExposureMethod = EAutoExposureMethod::AEM_Manual;
	Settings.bOverride_AutoExposureBias = true;
	Settings.AutoExposureBias = 1.0f;

	if (MinimapToonPostProcess)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Minimap] Successfully set post process material."));
		Owner->MinimapCaptureComponent->PostProcessSettings.AddBlendable(MinimapToonPostProcess, 1.0f);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[Minimap] Failed to set post process material."));
	}

	return RenderTarget;
}

void UMinimap::SetLevelTitle(FText title)
{
	TB_Title->SetText(title);
}