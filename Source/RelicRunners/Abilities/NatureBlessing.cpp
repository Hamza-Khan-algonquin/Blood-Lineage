// Fill out your copyright notice in the Description page of Project Settings.

#include "NatureBlessing.h"
#include "RelicRunners/PlayerState/RelicRunnersPlayerState.h"
#include "TimerManager.h"

UNaturesBlessing::UNaturesBlessing()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	AbilityName = FName("NaturesBlessing");

	InvincibleDuration = 3.0f;
	CooldownTimer = 14.f;
	Cost = 25;
	LastUseTime = -100.f;

	static ConstructorHelpers::FObjectFinder<UNiagaraSystem> VFXAsset(TEXT("/Game/Particles/NatureBlessing/NewNiagaraSystem.NewNiagaraSystem"));

	if (VFXAsset.Succeeded())
	{
		AbilityVFX = VFXAsset.Object;
	}
}

void UNaturesBlessing::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (!ActorInfo || !ActorInfo->AvatarActor.IsValid())
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	ARelicRunnersPlayerState* PS =ActorInfo->PlayerController.IsValid() ? ActorInfo->PlayerController->GetPlayerState<ARelicRunnersPlayerState>() : nullptr;

	if (!PS || PS->GetPlayerResource() < Cost)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

    // Mana deduction for the player
	PS->SetPlayerResource(PS->GetPlayerResource() - Cost);
	PS->UpdateRemoteHUDResource();

    // it turns on the invincibility for the character 
	PS->SetInvincible(true);

	ActorInfo->AvatarActor->GetWorldTimerManager().SetTimer(EndTimer,FTimerDelegate::CreateUObject(this, &UNaturesBlessing::EndAbility, Handle, ActorInfo, ActivationInfo, true, false), InvincibleDuration, false);
}

void UNaturesBlessing::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{ 
	if (ActorInfo && ActorInfo->PlayerController.IsValid())
	{
		if (ARelicRunnersPlayerState* PS = ActorInfo->PlayerController->GetPlayerState<ARelicRunnersPlayerState>())
		{
			PS->SetInvincible(false);
		}

		if (!bWasCancelled)
		{
			LastUseTime = ActorInfo->AvatarActor->GetWorld()->GetTimeSeconds();

			if (ActorInfo->IsLocallyControlled())
			{
				if (ARelicRunnersPlayerController* RPC = Cast<ARelicRunnersPlayerController>(ActorInfo->PlayerController.Get()))
				{
					RPC->PlayerHUD->StartDefenceCooldown(CooldownTimer);
				}
			}
		}
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}