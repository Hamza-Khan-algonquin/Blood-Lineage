#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "SessionListItemData.generated.h"

/**
 *
 */
UCLASS()
class RELICRUNNERS_API USessionListItemData : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite)
	FString SessionName;
	UPROPERTY(BlueprintReadWrite)
	int SessionPlayerCount;
};

