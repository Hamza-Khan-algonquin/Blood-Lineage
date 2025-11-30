// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ClassData.h"
#include "ClassInfo.generated.h"

UCLASS()
class RELICRUNNERS_API UClassInfo : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TMap<FName, struct FClassData> Classes;

	struct FClassData GetMaxStats() const;
	struct FClassData GetStats(FString ClassName) const;
};
