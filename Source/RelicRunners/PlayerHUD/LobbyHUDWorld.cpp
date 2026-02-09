// Fill out your copyright notice in the Description page of Project Settings.

#include "LobbyHUDWorld.h"
#include <RelicRunners/PlayerState/RelicRunnersPlayerState.h>
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/Border.h"
#include "RelicRunners/PlayerPreview/LobbyPreview.h"

void ULobbyHUDWorld::NativeConstruct()
{
    Super::NativeConstruct();

    if (OwningPreview)
    {
        //If PlayerState already exist
        if (OwningPreview->LinkedPlayerState)
        {
            BindToPlayerState();
        }
        else
        {
            //Wait for OnRep
            OwningPreview->OnPlayerStateReady.AddUObject(this, &ULobbyHUDWorld::BindToPlayerState);
        }
    }
}

void ULobbyHUDWorld::SetHostIconVisibility(bool IsHost)
{
    if (B_HostIcon) B_HostIcon->SetVisibility(IsHost ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
}

void ULobbyHUDWorld::BindToPlayerState()
{
    if (!OwningPreview || !OwningPreview->LinkedPlayerState) return;

    ARelicRunnersPlayerState* PS = OwningPreview->LinkedPlayerState;

    PS->OnSelectedClassChanged.RemoveAll(this);
    PS->OnSelectedClassChanged.AddUObject(this, &ULobbyHUDWorld::UpdatePlayerClassDisplay);

    PS->OnHostStatusChanged.RemoveAll(this);
    PS->OnHostStatusChanged.AddDynamic(this, &ULobbyHUDWorld::SetHostIconVisibility);

    SetHostIconVisibility(PS->IsHost);

    UpdatePlayerClassDisplay(PS->SelectedClass);
}

void ULobbyHUDWorld::UpdatePlayerClassDisplay(FName NewClass)
{
    if (!OwningPreview || !OwningPreview->LinkedPlayerState) return;
    ARelicRunnersPlayerState* PS = OwningPreview->LinkedPlayerState;

    if (TB_Name) TB_Name->SetText(FText::FromString(PS->GetPlayerName()));

    if (TB_Class) TB_Class->SetText(FText::FromName(NewClass));

    if (I_Class)
    {
        UTexture2D* ClassIcon = nullptr;

        if (*NewClass.ToString() == FName("Nemesis"))
        {
            ClassIcon = NemesisIcon;
        }
        if (*NewClass.ToString() == FName("Artemis"))
        {
            ClassIcon = ArtemisIcon;
        }
        if (*NewClass.ToString() == FName("Aphrodite"))
        {
            ClassIcon = AphroditeIcon;
        }
        if (*NewClass.ToString() == FName("Ares"))
        {
            ClassIcon = AresIcon;
        }

        if (ClassIcon)
        {
            I_Class->SetBrushFromTexture(ClassIcon);
        }
    }
}
