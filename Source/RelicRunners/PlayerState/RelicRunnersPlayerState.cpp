/************************************************************************************
 * Author:   Tristan Anglin
 * Year:     2025
 *
 * Notice:
 *   This source code is the intellectual property of Tristan Anglin.
 *   It is provided solely for use within:
 *     - The Relic Runners personal project
 *     - The Almost A Dozen project
 *
 *   Any use, distribution, or modification outside of these projects
 *   is strictly prohibited without explicit written permission.
 *
 *   � 2025 Tristan Anglin. All rights reserved.
 ************************************************************************************/

#include "RelicRunnersPlayerState.h"
#include "RelicRunners/Menu/Keybinds.h"
#include "Net/UnrealNetwork.h"
#include <RelicRunners/Menu/LobbyGameMode.h>


ARelicRunnersPlayerState::ARelicRunnersPlayerState()
{
    AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
    AttributeSet = CreateDefaultSubobject<UAttributeSet>(TEXT("AttributeSet"));

    AbilitySystemComponent->SetIsReplicated(true);
    AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);
}

UAbilitySystemComponent* ARelicRunnersPlayerState::GetAbilitySystemComponent() const
{
    return AbilitySystemComponent;
}


ARelicRunnersPlayerState::~ARelicRunnersPlayerState()
{

}

void ARelicRunnersPlayerState::SetSelectedClass(FName NewClass)
{
    if (!HasAuthority()) return;

    SelectedClass = NewClass;
    OnSelectedClassChanged.Broadcast(SelectedClass);
    ForceNetUpdate();

    if (ALobbyGameMode* GM = GetWorld()->GetAuthGameMode<ALobbyGameMode>())
    {
        GM->OnPlayerStateUpdated(this);
    }
}

void ARelicRunnersPlayerState::OnRep_SelectedClass()
{
    OnSelectedClassChanged.Broadcast(SelectedClass);
}

void ARelicRunnersPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(ARelicRunnersPlayerState, SelectedClass);
}

