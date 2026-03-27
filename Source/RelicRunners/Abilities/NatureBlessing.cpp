// Fill out your copyright notice in the Description page of Project Settings.

#include "NatureBlessing.h"
#include "RelicRunners/PlayerState/RelicRunnersPlayerState.h"
#include "TimerManager.h"

UNaturesBlessing::UNaturesBlessing()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	AbilityName = FName("NaturesBlessing");

	InvincibleDuration = 5.0f;
	CooldownTimer = 15.f;
	LastUseTime = -100.f;

}

void UNaturesBlessing::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (!CurrentActorInfo) return;

	AActor* AvatarActor = ActorInfo ? ActorInfo->AvatarActor.Get() : nullptr;
	if (!ActorInfo || !ActorInfo->AvatarActor.IsValid())
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	ARelicRunnersPlayerState* PS = nullptr;
	if (ActorInfo->PlayerController.IsValid())
	{
		PS = ActorInfo->PlayerController->GetPlayerState<ARelicRunnersPlayerState>();
	}

	float FinalManaCost = CalculateAbilityMana(AvatarActor, Cost);

	if (!PS || PS->GetPlayerResource() < FinalManaCost)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// Spend resource
	PS->SetPlayerResource(PS->GetPlayerResource() - FinalManaCost);
	PS->UpdateRemoteHUDResource();

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	PlayAbilityCastSound(ActorInfo);

    // it turns on the invincibility for the character 
	PS->SetInvincible(true);

	AController* Controller = nullptr;

	if (CurrentActorInfo && CurrentActorInfo->PlayerController.IsValid())
	{
		Controller = CurrentActorInfo->PlayerController.Get();
	}

	if (Controller)
	{

		if (PS)
		{
			PS->Multicast_SpawnVFX(FVector(0), FRotator::ZeroRotator, FVector(2.f), AbilityVFX, true);
		}

	}

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