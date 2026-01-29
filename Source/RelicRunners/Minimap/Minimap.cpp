// Fill out your copyright notice in the Description page of Project Settings.


#include "Minimap.h"

void UMinimap::SetMinimapImage(UTextureRenderTarget2D* RenderTarget)
{
    //if (!RenderTarget || !I_Minimap) return;
    /*
    UMaterialInterface* BaseMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/ThirdPerson/Blueprints/PlayerPreview/M_MiniMap_TextureRenderTarget.M_MiniMap_TextureRenderTarget"));

    if (!BaseMaterial)
    {
        UE_LOG(LogTemp, Error, TEXT("[UI] Failed to load base material."));
        return;
    }

    //Create the dynamic material
    UMaterialInstanceDynamic* DynMaterial = UMaterialInstanceDynamic::Create(BaseMaterial, this);
    if (!DynMaterial)
    {
        UE_LOG(LogTemp, Error, TEXT("[UI] Failed to create dynamic material."));
        return;
    }

    DynMaterial->SetTextureParameterValue(FName("Minimap"), RenderTarget);
    //I_Minimap->SetBrushFromMaterial(DynMaterial);

    UE_LOG(LogTemp, Warning, TEXT("[UI] Set preview image with RenderTarget: %s"), *GetNameSafe(RenderTarget));*/
}