/***************************************************************************************
Class Name:  ASpawnZone
Author:      Thomas Johnson
Description: This Class is incharge of spawn enemies when told to by the director in a radius.
Date Last Changed:        2025/10/27
****************************************************************************************/
#pragma once

#include "CoreMinimal.h"
#include "SpawnPoint.h"
#include "SpawnZone.generated.h"

class ANavMeshBoundsVolume;

UCLASS()
class RELICRUNNERS_API ASpawnZone : public ASpawnPoint
{
	GENERATED_BODY()
protected:
	//The nav mesh bounds the spawn zone will use. If not set it will use any nav mesh bounds the spawn zone actor is in
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Point Properties", meta = (AllowPrivateAccess = "true", ToolTip = "The nav mesh bounds the spawn zone will use. If not set it will use any nav mesh bounds the spawn zone actor is in"))
	ANavMeshBoundsVolume* SpawningBounds;

	float MinDistanceFromPlayer = 200;

public:
	virtual FVector GetRandomPosition() override;


};
