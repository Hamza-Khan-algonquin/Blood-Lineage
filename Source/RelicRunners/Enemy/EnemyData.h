#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "EnemyData.generated.h"


//same delegate signature as AActor's OnDestroyed but for enemies
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FEnemyCharacterDied, AActor*, DiedActor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FEnemyCharacterDeactivated, AActor*, DeactivatedActor);


//enum for the different types of enemies
UENUM(BlueprintType)
enum class EEnemyType : uint8
{
	LIGHT = 0		UMETA(DisplayName = "light"),
	MEDIUM = 1		UMETA(DisplayName = "medium"),
	BOSS = 2        UMETA(DisplayName = "boss"),
	UNKNOWN = 3		UMETA(DisplayName = "unknown")
};