/***************************************************************************************
Class Name:  ADirector
Description: This Class communicates with the spawner and keep track of what spawner should be spwaning and other factors related to spawning or enemy behavior as a collective
Date Last Changed:        2025/10/27
Author:      Thomas Johnson
Contributors:
	Description Of Contributions:
****************************************************************************************/
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SpawningCard.h"
#include "Director.generated.h"

class ARelicRunnersCharacter;

UCLASS()
class RELICRUNNERS_API ADirector : public AActor
{
	GENERATED_BODY()

protected:
	//The max distance away a spawn point can be to be used
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Stats", meta = (AllowPrivateAccess = "true", ToolTip = "The max distance away a spawn point can be to be used"))
	float MaxSpawnPointActivisionDistance;
	//The time it takes for the director to try and spawn again
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Stats", meta = (AllowPrivateAccess = "true", ToolTip = "The time it takes for the director to try and spawn again"))
	float SpawnWait;
	
	//Keeps track of how long it been sense the last spawn if it needs to spawn again
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Timer", meta = (AllowPrivateAccess = "true", ToolTip = "Keeps track of how long it been sense the last spawn if it needs to spawn again"))
	float SpawnTimer;

	//A list of all the players the director is aware of
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player List", meta = (AllowPrivateAccess = "true", ToolTip = "A list of all the players the director is aware of"))
	TArray<APawn*> Players;

	//If a spawn point does not have it spawn cards set it will use this
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy Pool", meta = (AllowPrivateAccess = "true", ToolTip = "If a spawn point does not have it spawn cards set it will use this"))
	TArray<USpawnCard*> DefaultSpawningCards;

	//The enemies currently in the scene
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy In Scene", meta = (AllowPrivateAccess = "true", ToolTip = "The enemies currently in the scene"))
	TArray<APawn*> EnemiesInWorld;
	
public:	
	//A Method that will remove a provided enemy from EnemiesInWorld. It is meant to be called only by APawn when a OnDestroy event happens
	UFUNCTION(meta = ( ToolTip = "A Method that will remove a provided enemy from EnemiesInWorld. It is meant to be called only by APawn when a OnDestroy event happens", ShortToolTip = "Removes the provided enemy from EnemiesInWorld"))
	void RemoveEnemy(AActor* enemy);
	//A Method that will remove a provided enemy from EnemiesInWorld. It is meant to be called only by APawn when a OnDestroy event happens
	UFUNCTION(meta = ( ToolTip = "A Method that will remove a provided enemy from EnemiesInWorld. It is meant to be called only by APawn when a OnDestroy event happens", ShortToolTip = "Adds the provided enemy from EnemiesInWorld"))
	void AddEnemy(AActor* enemy);
	UFUNCTION(meta = (ToolTip = "A Method to check and see if the director knows about a enemy", ShortToolTip = "A Method to check and see if the director knows about a enemy"))
	bool ContainsEnemy(AActor* enemy);
	//A Method to get the default spawn cards
	UFUNCTION(meta = (ToolTip = "A Method to get the default spawn cards", ShortToolTip = "Thomas was here"))
	TArray<USpawnCard*> GetDefaultSpawnCards();
	//A Method to get all the players that the director is keeping track of
	UFUNCTION(meta = (ToolTip = "A Method to get all the players that the director is keeping track of", ShortToolTip = "Get the players"))
	TArray<APawn*> GetPlayers();
	//A method to add a player to the list of players the director knows of
	UFUNCTION(meta = (ToolTip = "A method to add a player to the list of players the director knows of", ShortToolTip = "Add the players"))
	void AddPlayer(APawn* newPlayer);

public:
	// Sets default values for this actor's properties
	ADirector();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	//A method to try and spawn enemies on near by spawnPoints
	void SpawnCheck();

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
