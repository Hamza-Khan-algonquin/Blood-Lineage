// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ClassData.generated.h"

USTRUCT(BlueprintType)
struct FClassData
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName ClassName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString Role;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString Ability1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString Ability1Description1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString Ability2;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString Ability2Description1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString Ability3;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString Ability3Description1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString Ability4;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString Ability4Description1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float Health = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float Armor = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float Dexterity = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float Strength = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float Intelligence = 0.f;
};