// Fill out your copyright notice in the Description page of Project Settings.

#include "LobbyHUDWorld.h"
#include <RelicRunners/PlayerState/RelicRunnersPlayerState.h>
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "RelicRunners/PlayerPreview/LobbyPreview.h"

void ULobbyHUDWorld::NativeConstruct()
{
    Super::NativeConstruct();

    if (OwningPreview && OwningPreview->LinkedPlayerState)
    {
        ARelicRunnersPlayerState* PS = OwningPreview->LinkedPlayerState;
        PS->OnSelectedClassChanged.AddUObject(this, &ULobbyHUDWorld::UpdatePlayerClassDisplay);
        UpdatePlayerClassDisplay(PS->SelectedClass);
    }
}

void ULobbyHUDWorld::UpdatePlayerClassDisplay(FName NewClass)
{
    if (!OwningPreview || !OwningPreview->LinkedPlayerState) return;
    ARelicRunnersPlayerState* PS = OwningPreview->LinkedPlayerState;

    if (TB_Name) TB_Name->SetText(FText::FromString(PS->GetPlayerName()));

    if (TB_Class) TB_Class->SetText(FText::FromName(NewClass));

    if (I_Class)
    {
        FString TexturePath = FString::Printf(TEXT("/Game/Blueprints/Menu/Icon_%s.Icon_%s"),
            *NewClass.ToString(), *NewClass.ToString());

        UTexture2D* ClassIcon = LoadObject<UTexture2D>(nullptr, *TexturePath);
        if (ClassIcon)
        {
            FSlateBrush Brush;
            Brush.SetResourceObject(ClassIcon);
            Brush.ImageSize = FVector2D(150.f, 150.f);
            Brush.TintColor = FSlateColor(FLinearColor::Black);
            I_Class->SetBrush(Brush);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Could not find icon for class: %s (Path: %s)"), *NewClass.ToString(), *TexturePath);
        }
    }
}
