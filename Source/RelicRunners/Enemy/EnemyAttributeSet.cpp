// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyAttributeSet.h"
#include "Net/UnrealNetwork.h"

UEnemyAttributeSet::UEnemyAttributeSet() : CurrentHealth(100.f), MaxHealth(100.f)
{
}

void UEnemyAttributeSet::OnRep_CurrentHealth(const FGameplayAttributeData oldCurrentHealth)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UEnemyAttributeSet, CurrentHealth, oldCurrentHealth);
}

void UEnemyAttributeSet::OnRep_MaxHealth(const FGameplayAttributeData oldMaxHealth)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UEnemyAttributeSet, MaxHealth, oldMaxHealth);
}

void UEnemyAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME_CONDITION_NOTIFY(UEnemyAttributeSet, CurrentHealth, COND_None, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(UEnemyAttributeSet, MaxHealth, COND_None, REPNOTIFY_Always);
}
