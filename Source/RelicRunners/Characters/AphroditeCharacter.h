// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RelicRunners/RelicRunnersCharacter.h" 
#include "AphroditeCharacter.generated.h"

/**
 * 
 */
UCLASS()
class RELICRUNNERS_API AAphroditeCharacter : public ARelicRunnersCharacter
{
	GENERATED_BODY()

public:
	AAphroditeCharacter();

	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_PlayerState() override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	class UClassInfo* ClassInfo;

protected:
	virtual void BeginPlay() override;

	virtual void GiveDamageAbilities() override;
	virtual void GiveDefenceAbilities() override;
	virtual void GiveUtilityAbilities() override;
	virtual void GiveUltimateAbilities() override;
};
