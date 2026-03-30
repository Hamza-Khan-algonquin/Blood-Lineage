#include "IslandGenerationTest.h"
#include "Net/UnrealNetwork.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "IslandSpawningValuesComponent.h"
#include "Net/UnrealNetwork.h"
#include "PackedLevelActor/PackedLevelActor.h"
#include "NewObjectives/ObjectiveBase.h"
#include "NewObjectives/NewCapturableFlag.h"
#include "NewObjectives/NewEnemyZone.h"
#include "NewObjectives/NewPayloadObjective.h"
#include "NewObjectives/NewSurvivalZone.h"
#include "IslandBridge.h"
#include "LevelChangeTrigger.h"
#include "Components/SplineComponent.h"
#include "NavigationSystem.h"
#include <RelicRunners/PlayerController/RelicRunnersPlayerController.h>
#include "RelicRunners/Director System/SpawnZone.h"
#include "RelicRunners/Game/RelicRunnersGameInstance.h"
#include "CullingToggleInterface.h"

/* Maybe TODO

Handle different difficulty

Add mesh for the payload track

Set up spawning decals

Set up material that randomly offsets the texture to make it look more dynamic

*/

// Sets default values
AIslandGenerationTest::AIslandGenerationTest() :
	IslandWidth(2),
	IslandDepth(2),
	IslandIndex(0),
	CurrentObjectives(0),
	isFinalIsland(false),
	ObjectiveAmount(0),
	CameraActor(nullptr),
	LevelTrigger(nullptr),
	IsActivated(false)
{
	USceneComponent* RootScene = CreateDefaultSubobject<USceneComponent>(TEXT("RootScene"));
	RootComponent = RootScene;

	PrimaryActorTick.bCanEverTick = false;

	GenerationValuesComponent = CreateDefaultSubobject<UIslandSpawningValuesComponent>("GenerationValues");
	GenerationValuesComponent->SetIsReplicated(true);

	SplinePath = CreateDefaultSubobject<USplineComponent>("SplinePath");

	FloorISMComp = CreateDefaultSubobject<UInstancedStaticMeshComponent>("FloorISMComp");
	FloorISMComp->SetupAttachment(RootComponent);
	FloorISMComp->SetIsReplicated(true);
	FloorISMComp->SetNetAddressable();

	bReplicates = true;
	bAlwaysRelevant = true;
}

void AIslandGenerationTest::Generate()
{
	if (HasAuthority())
	{
		//Initialize the starting and ending tiles.
		int startingIndex = 0, endingIndex = (IslandDepth * IslandWidth) - 1;

		for (int y = 0; y < IslandDepth; y++)
		{
			for (int x = 0; x < IslandWidth; x++)
			{
				//Initialize each tile and set them to either walkable or non-walkable.
				GenerateFloor(x, y);
			}
		}

		if (IslandDepth * IslandWidth > 1)
		{
			//Override starting and ending tiles initialized above and create a path from one to the other.
			SetStartingAndEndingPoints(startingIndex, endingIndex);

			FindStartToEndPath(startingIndex, endingIndex, IslandWidth);
		}

		InitializeObjectives(startingIndex, endingIndex);

		int maxVantagePoint = FMath::RandRange(GenerationValuesComponent->VantageValues.MaxVantageAmountMin, GenerationValuesComponent->VantageValues.MaxVantageAmountMax);

		//Override previously initialized tiles with Vantage Point tiles
		for (int i = 0; i < maxVantagePoint; i++)
		{
			int keyTileIndex = FMath::RandRange(0, (IslandDepth * IslandWidth) - 1);

			int whileLoopLimit = 0;

			//If the currently selected tile can be overriden, do that, if not, find another tile. Repeat 10 time. if failed every time, give up.
			while (FloorValuesArray[keyTileIndex].IslandFloorObstacle > EIslandFloorObstacle::Vantage)
			{
				keyTileIndex = FMath::RandRange(0, (IslandDepth * IslandWidth) - 1);

				whileLoopLimit++;

				if (whileLoopLimit >= 10)
				{
					break;
				}
			}

			//If it can, override the currently selected tile with the key tile and create a path from the starting tile to it.
			if (whileLoopLimit < 10)
			{
				SetVantagePointTile(keyTileIndex);

				FindStartToEndPath(startingIndex, keyTileIndex, IslandWidth);
			}
		}

		int maxSafeTiles = FMath::RandRange(GenerationValuesComponent->SafeSpotValues.MaxSafeSpotAmountMin, GenerationValuesComponent->SafeSpotValues.MaxSafeSpotAmountMax);

		//Override previously initialized tiles with Safe tiles
		for (int i = 0; i < maxSafeTiles; i++)
		{
			int keyTileIndex = FMath::RandRange(0, (IslandDepth * IslandWidth) - 1);

			int whileLoopLimit = 0;

			//If the currently selected tile can be overriden, do that, if not, find another tile. Repeat 10 time. if failed every time, give up.
			while (FloorValuesArray[keyTileIndex].IslandFloorObstacle >= EIslandFloorObstacle::Safe)
			{
				keyTileIndex = FMath::RandRange(0, (IslandDepth * IslandWidth) - 1);

				whileLoopLimit++;

				if (whileLoopLimit >= 10)
				{
					break;
				}
			}

			//If it can, override the currently selected tile with the key tile and create a path from the starting tile to it.
			if (whileLoopLimit < 10)
			{
				SetSafeTile(keyTileIndex);

				FindStartToEndPath(startingIndex, keyTileIndex, IslandWidth);
			}
		}

		if (!FloorValuesArray.IsEmpty())
		{
			for (int y = 0; y < IslandDepth; y++)
			{
				for (int x = 0; x < IslandWidth; x++)
				{
					//Spawn the floor tiles with the instanced static mesh component.
					CheckFloor(x, y, IslandWidth, IslandDepth);

					//Spawn each modular obstacle based on previously defined variable.
					Server_SpawnFloorObstacles(x, y, IslandWidth);
				}
			}
		}

		if (GenerationValuesComponent->IslandStartEnd.BridgePackedActor)
		{
			float tileScale = GenerationValuesComponent->IslandValues.TileScale;
			FVector posOffset = FVector((tileScale * IslandWidth * 4) + tileScale, (tileScale * (IslandDepth - 1) * 4) + tileScale, 0.0f) + GetActorLocation();

			Bridge = GetWorld()->SpawnActor<AIslandBridge>(GenerationValuesComponent->IslandStartEnd.BridgePackedActor, posOffset, FRotator::ZeroRotator);

			Bridge->SetActorHiddenInGame(true);
			Bridge->SetFinalBridge(isFinalIsland);
			Bridge->OnFrontTrigger.AddDynamic(this, &AIslandGenerationTest::UpdateCameraPos);
			Bridge->OnBackTrigger.AddDynamic(this, &AIslandGenerationTest::SetNextCameraPos);

			if (Bridge->Implements<UCullingToggleInterface>())
			{
				ICullingToggleInterface::Execute_ToggleVisibility(Bridge, false);
			}

			URelicRunnersGameInstance* gameInstance = Cast<URelicRunnersGameInstance>(GetGameInstance());
			if (gameInstance)
			{
				if (GenerationValuesComponent->IslandValues.LevelChangeTrigger && isFinalIsland)
				{
					LevelTrigger = GetWorld()->SpawnActor<ALevelChangeTrigger>(GenerationValuesComponent->IslandValues.LevelChangeTrigger, posOffset, FRotator::ZeroRotator);

					if (LevelTrigger->Implements<UCullingToggleInterface>())
					{
						ICullingToggleInterface::Execute_ToggleVisibility(LevelTrigger, false);
					}

					FString TargetLevel = gameInstance->StageNumber >= NumStages ?
						GenerationValuesComponent->IslandValues.BossLevel :
						GenerationValuesComponent->IslandValues.TargetLevel;

					LevelTrigger->Server_SetTargetLevel(FName(TargetLevel));
				}
			}
		}
	}

	//Navmesh stuff. Thanks Konstantin
	UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
	if (NavSys)
	{
		NavSys->Build();
	}
}

void AIslandGenerationTest::ActivateIsland()
{
	IsActivated = true;

	SetIslandVisibility(true);

	for (AObjectiveBase* obj : ObjectiveArrays)
	{
		obj->Activate();
	}

	switch (IslandObjective)
	{
	case EIslandObjectiveType::CaptureTheFlag:
		AddFlagObjectiveToAll(CurrentObjectives);
		break;
	case EIslandObjectiveType::MoveThePayload:
		AddPayloadObjectiveToAll();
		break;
	case EIslandObjectiveType::Survival:
		auto& Survival = GenerationValuesComponent->SurvivalZoneObjectiveValues;
		AddPandoraObjectiveToAll(Survival.SurvivalDuration);
	}

	if (Bridge)
	{
		Bridge->Create();
		Bridge->SetActorHiddenInGame(false);
	}

	CreateFloor();

	if (SpawnZoneTemplate == nullptr)
		return;
	float tileScale = GenerationValuesComponent->IslandValues.TileScale;
	FVector enemyZoneOffset = FVector((tileScale * IslandWidth * 4) / 2, (tileScale * (IslandDepth) * 4) / 2, 0.0f);

	ASpawnZone* spawnZone = GetWorld()->SpawnActor<ASpawnZone>(SpawnZoneTemplate, enemyZoneOffset + GetActorLocation(), GetActorRotation());
}

void AIslandGenerationTest::SetIslandVisibility(bool visible)
{

	if (Bridge)
	{
		if (Bridge->Implements<UCullingToggleInterface>())
		{
			ICullingToggleInterface::Execute_ToggleVisibility(Bridge, visible);
		}
	}

	if (LevelTrigger)
	{
		if (LevelTrigger->Implements<UCullingToggleInterface>())
		{
			ICullingToggleInterface::Execute_ToggleVisibility(LevelTrigger, visible);
		}
	}

	for (APackedLevelActor* obstac : ObstacleArrays)
	{
		if (obstac->Implements<UCullingToggleInterface>())
		{
			ICullingToggleInterface::Execute_ToggleVisibility(obstac, visible);
		}
	}

	for (AObjectiveBase* obj : ObjectiveArrays)
	{
		obj->SetActorHiddenInGame(!visible);
	}

	for (UInstancedStaticMeshComponent* ismComp : InstanceMeshComponentArray)
	{
		ismComp->SetVisibility(visible);
	}

	if (FloorISMComp)
	{
		FloorISMComp->SetVisibility(visible);
	}
}

void AIslandGenerationTest::InitializeValues(EIslandObjectiveType islandObjective)
{
	IslandObjective = islandObjective;
	UIslandSpawningValuesComponent* ValuesComp = GenerationValuesComponent;

	switch (IslandObjective)
	{
	case EIslandObjectiveType::CaptureTheFlag:
		IslandDepth = FMath::RandRange(ValuesComp->FlagObjectiveValues.FlagDepthMin, ValuesComp->FlagObjectiveValues.FlagDepthMax);
		IslandWidth = FMath::RandRange(ValuesComp->FlagObjectiveValues.FlagWidthMin, ValuesComp->FlagObjectiveValues.FlagWidthMax);
		break;
	case EIslandObjectiveType::MoveThePayload:
		IslandDepth = FMath::RandRange(ValuesComp->PayloadObjectiveValues.PayloadDepthMin, ValuesComp->PayloadObjectiveValues.PayloadDepthMax);
		IslandWidth = FMath::RandRange(ValuesComp->PayloadObjectiveValues.PayloadWidthMin, ValuesComp->PayloadObjectiveValues.PayloadWidthMax);
		break;
	case EIslandObjectiveType::Survival:
		IslandDepth = FMath::RandRange(ValuesComp->SurvivalZoneObjectiveValues.SurvivalDepthMin, ValuesComp->SurvivalZoneObjectiveValues.SurvivalDepthMax);
		IslandWidth = FMath::RandRange(ValuesComp->SurvivalZoneObjectiveValues.SurvivalWidthMin, ValuesComp->SurvivalZoneObjectiveValues.SurvivalWidthMax);
		break;
	default:
		IslandDepth = 2;
		IslandWidth = 2;
		break;
	}
}

void AIslandGenerationTest::GenerateFloor(int x, int y)
{
	if (FloorValuesArray.Num() == x + (IslandWidth * y))
	{
		//Set current tile as walkable or non-walkable
		InitializeFloor();
	}
}

bool AIslandGenerationTest::CalculateDistanceBetweenTiles(int firstIndex, int secondIndex) const
{
	uint8 minDistance = GenerationValuesComponent->IslandStartEnd.StartingToEndingMinDistance;
	if ((minDistance + (minDistance * IslandWidth)) > IslandWidth * IslandDepth)
	{
		return false;
	}

	for (int y = -minDistance; y < minDistance; y++)
	{
		for (int x = -minDistance; x < minDistance; x++)
		{
			int currentX = firstIndex % IslandWidth + x;
			int currentY = firstIndex / IslandWidth + y;

			bool validX = currentX >= 0 && currentX < IslandWidth;
			bool validY = currentY >= 0 && currentY < IslandDepth;

			if (validX && validY)
			{
				if ((currentX + (currentY * IslandWidth)) == secondIndex)
				{
					return true;
				}
			}
		}
	}
	return false;
}

void AIslandGenerationTest::SetStartingAndEndingPoints(int startingIndex, int endingIndex)
{
	FloorValuesArray[startingIndex].IsFullTile = true;
	FloorValuesArray[startingIndex].IslandFloorObstacle = EIslandFloorObstacle::Start;
	FloorValuesArray[startingIndex].randomFloorToSpawn = 0;
	FloorValuesArray[startingIndex].obstacleYaw = FMath::RandRange(0, 3) * 90;


	FloorValuesArray[endingIndex].IsFullTile = true;
	FloorValuesArray[endingIndex].IslandFloorObstacle = EIslandFloorObstacle::End;
	FloorValuesArray[endingIndex].randomFloorToSpawn = 0;
	FloorValuesArray[endingIndex].obstacleYaw = FMath::RandRange(0, 3) * 90;
}

void AIslandGenerationTest::SetSafeTile(int index)
{
	FloorValuesArray[index].IsFullTile = true;
	FloorValuesArray[index].IslandFloorObstacle = EIslandFloorObstacle::Safe;
	FloorValuesArray[index].randomFloorToSpawn = 0;
	FloorValuesArray[index].obstacleYaw = FMath::RandRange(0, 3) * 90;
}

void AIslandGenerationTest::SetFlagTile(int index)
{
	FloorValuesArray[index].IsFullTile = true;
	FloorValuesArray[index].IslandFloorObstacle = EIslandFloorObstacle::CapturableFlag;
	FloorValuesArray[index].randomFloorToSpawn = 0;
	FloorValuesArray[index].obstacleYaw = FMath::RandRange(0, 3) * 90;
}

void AIslandGenerationTest::SetEnemyZoneTile(int index)
{
	FloorValuesArray[index].IsFullTile = true;
	FloorValuesArray[index].IslandFloorObstacle = EIslandFloorObstacle::EnemyZone;
	FloorValuesArray[index].randomFloorToSpawn = 0;
	FloorValuesArray[index].obstacleYaw = FMath::RandRange(0, 3) * 90;
}

void AIslandGenerationTest::SetSurvivalZoneTile(int index)
{
	FloorValuesArray[index].IsFullTile = true;
	FloorValuesArray[index].IslandFloorObstacle = EIslandFloorObstacle::SurvivalZone;
	FloorValuesArray[index].randomFloorToSpawn = 0;
	FloorValuesArray[index].obstacleYaw = FMath::RandRange(0, 3) * 90;
}

void AIslandGenerationTest::SetVantagePointTile(int index)
{
	FloorValuesArray[index].IsFullTile = true;
	FloorValuesArray[index].IslandFloorObstacle = EIslandFloorObstacle::Vantage;
	FloorValuesArray[index].randomFloorToSpawn = 0;
	FloorValuesArray[index].obstacleYaw = FMath::RandRange(0, 3) * 90;
}

void AIslandGenerationTest::InitializeFloor()
{	
	//Based on the percentage, set floor tile to walkable or non-walkable.
	FSIslandFloorValues newFloorValues;
	float randomFloor = FMath::RandRange(0.1f, 100.0f);

	float fullPercentage = GenerationValuesComponent->IslandValues.FullPercentage;

	if (randomFloor > fullPercentage)
	{
		newFloorValues.IsFullTile = false;
	}
	else
	{
		newFloorValues.IsFullTile = true;

		InitializeModularObstacle(newFloorValues, true);
	}

	//Add initialized floor to array
	FloorValuesArray.Add(newFloorValues);
}

void AIslandGenerationTest::ForceFloorBool(bool forcedFloor, int x, int y, int width, bool canSpawnObstacle, bool isPayloadPath)
{
	int index = x + (y * width);

	FloorValuesArray[index].IsFullTile = forcedFloor;

	if (!forcedFloor)
		return;

	//if (FloorValuesArray[index].IslandFloorObstacle == EIslandFloorObstacle::None)
	//	return;

	//if (FloorValuesArray[index].IslandFloorObstacle == EIslandFloorObstacle::PayloadPath)
	//	return;

	if (FloorValuesArray[index].IslandFloorObstacle >= EIslandFloorObstacle::CapturableFlag)
		return;

	if (!canSpawnObstacle)
	{
		FloorValuesArray[index].IslandFloorObstacle = isPayloadPath ?
			EIslandFloorObstacle::PayloadPath :
			EIslandFloorObstacle::None;

		return;
	}

	InitializeModularObstacle(FloorValuesArray[index], false);
}

void AIslandGenerationTest::InitializeModularObstacle(FSIslandFloorValues& floorValue, bool canGenerateHoles) const
{
	float randomObstaclePerc = FMath::RandRange(0.1f, 100.0f);
	if (randomObstaclePerc < GenerationValuesComponent->ObstacleValues.ObstaclePercentage)
	{
		floorValue.IslandFloorObstacle = EIslandFloorObstacle::Basic;

		floorValue.randomFloorToSpawn = FMath::RandRange(0, GenerationValuesComponent->ObstacleValues.ObstacleActor.Num() - 1);

		floorValue.obstacleYaw = FMath::RandRange(0, 3) * 90;

		floorValue.ObstacleType = EObstacleType(FMath::RandRange(int(EObstacleType::Building), int(EObstacleType::Environment)));

		floorValue.randomBuildingObstacleToSpawn = FMath::RandRange(0, GenerationValuesComponent->ObstacleTypeValues.BuildingObstacles.ObstacleActor.Num() - 1);
		floorValue.randomEnvironmentObstacleToSpawn = FMath::RandRange(0, GenerationValuesComponent->ObstacleTypeValues.EnvironmentObstacles.ObstacleActor.Num() - 1);

	}
	else
	{
		floorValue.IslandFloorObstacle = EIslandFloorObstacle::None;
	}
}

void AIslandGenerationTest::FindStartToEndPath(int startingIndex, int targetIndex, int width)
{
	int currentX = startingIndex % width;
	int currentY = startingIndex / width;

	int targetX = targetIndex % width;
	int targetY = targetIndex / width;

	bool bReachEndX = currentX == targetX;

	bool bReachEndY = currentY == targetY;

	//To avoid repetition, randomly picks between moving horizontally or vertically closer to the target
	while (!(bReachEndX && bReachEndY))
	{
		float random = FMath::FRandRange(0.0f, 100.0f);

		if (random >= 50.0f)
		{
			if (currentX > targetX)
			{
				currentX--;
			}
			else if (currentX < targetX)
			{
				currentX++;
			}
		}
		else
		{
			if (currentY > targetY)
			{
				currentY--;
			}
			else if (currentY < targetY)
			{
				currentY++;
			}
		}

		bReachEndX = currentX == targetX;
		bReachEndY = currentY == targetY;
		if (!(bReachEndX && bReachEndY))
		{
			ForceFloorBool(true, currentX, currentY, width, true, false);
		}
	}
}

void AIslandGenerationTest::CreateSplinePathStartToEnd(int startingIndex, int targetIndex, int width, ANewPayloadObjective* payloadActor)
{
	int currentX = startingIndex % width;
	int currentY = startingIndex / width;

	int targetX = targetIndex % width;
	int targetY = targetIndex / width;

	bool bReachEndX = currentX == targetX;

	bool bReachEndY = currentY == targetY;

	bool bValueHasChanged = false;
	//To avoid repetition, randomly picks between moving horizontally or vertically closer to the target
	while (!(bReachEndX && bReachEndY))
	{
		float random = FMath::FRandRange(0.0f, 100.0f);

		if (random >= 50.0f)
		{
			if (currentX > targetX)
			{
				currentX--;
				bValueHasChanged = true;
			}
			else if (currentX < targetX)
			{
				currentX++;
				bValueHasChanged = true;
			}
		}
		else
		{
			if (currentY > targetY)
			{
				currentY--;
				bValueHasChanged = true;
			}
			else if (currentY < targetY)
			{
				currentY++;
				bValueHasChanged = true;
			}
		}

		if (!(bReachEndX && bReachEndY))
		{
			ForceFloorBool(true, currentX, currentY, width, false, true);

			if (payloadActor && bValueHasChanged)
			{
				float tileScale = GenerationValuesComponent->IslandValues.TileScale;
				FVector pointPos = FVector((tileScale * currentX * 4) + tileScale, (tileScale * currentY * 4) + tileScale, 0.0f) + GetActorLocation();

				SplinePath->AddSplinePoint(pointPos, ESplineCoordinateSpace::World, true);

				bValueHasChanged = false;

				if (GenerationValuesComponent->PayloadObjectiveValues.PayloadPathModuleActor.Num())
				{
					int moduleIndex = FMath::RandRange(0, GenerationValuesComponent->PayloadObjectiveValues.PayloadPathModuleActor.Num() - 1);
					if (GenerationValuesComponent->PayloadObjectiveValues.PayloadPathModuleActor[moduleIndex] != nullptr)
					{
						int randYaw = FMath::RandRange(0, 3) * 90;
						APackedLevelActor* aModule = GetWorld()->SpawnActor<APackedLevelActor>(GenerationValuesComponent->PayloadObjectiveValues.PayloadPathModuleActor[moduleIndex], pointPos, FRotator(0.0f, randYaw, 0.0f));

						if (aModule->Implements<UCullingToggleInterface>())
						{
							ICullingToggleInterface::Execute_ToggleVisibility(aModule, false);
						}

						ObstacleArrays.Add(aModule);
					}
				}

			}
		}
		bReachEndX = currentX == targetX;
		bReachEndY = currentY == targetY;
	}
}

void AIslandGenerationTest::AddFlagObjectiveToAll(int NumFlags)
{
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		if (ARelicRunnersPlayerController* TargetPC = Cast<ARelicRunnersPlayerController>(It->Get()))
		{
			ARelicRunnersPlayerController::FPendingObjective NewObjective;
			NewObjective.Type = ARelicRunnersPlayerController::EObjectiveType::Flag;
			NewObjective.FlagCount = NumFlags;

			TargetPC->AddObjectiveToBuffer(NewObjective);
		}
	}
}

void AIslandGenerationTest::DecrementFlagObjectiveForAll()
{
	int TotalFlags = ObjectiveAmount;
	int CapturedCount = TotalFlags - CurrentObjectives;

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		if (ARelicRunnersPlayerController* TargetPC = Cast<ARelicRunnersPlayerController>(It->Get()))
		{
			//Update the Server's local UI for this PC
			TargetPC->Internal_SetFlagUI(CapturedCount);

			TargetPC->Client_UpdateFlagUI(CapturedCount);
		}
	}
}

void AIslandGenerationTest::AddEnemiesObjectiveToAll(int TotalEnemies)
{
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		if (ARelicRunnersPlayerController* TargetPC = Cast<ARelicRunnersPlayerController>(It->Get()))
		{
			ARelicRunnersPlayerController::FPendingObjective NewObjective;
			NewObjective.Type = ARelicRunnersPlayerController::EObjectiveType::Enemies;
			NewObjective.EnemyCount = TotalEnemies;

			TargetPC->AddObjectiveToBuffer(NewObjective);
		}
	}
}

void AIslandGenerationTest::AddPandoraObjectiveToAll(float Time)
{
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		if (ARelicRunnersPlayerController* TargetPC = Cast<ARelicRunnersPlayerController>(It->Get()))
		{
			ARelicRunnersPlayerController::FPendingObjective NewObjective;
			NewObjective.Type = ARelicRunnersPlayerController::EObjectiveType::Pandora;
			NewObjective.PandoraTime = Time;

			TargetPC->AddObjectiveToBuffer(NewObjective);
		}
	}
}

void AIslandGenerationTest::AddPayloadObjectiveToAll()
{
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		if (ARelicRunnersPlayerController* TargetPC = Cast<ARelicRunnersPlayerController>(It->Get()))
		{
			ARelicRunnersPlayerController::FPendingObjective NewObjective;
			NewObjective.Type = ARelicRunnersPlayerController::EObjectiveType::Payload;

			TargetPC->AddObjectiveToBuffer(NewObjective);
		}
	}
}

void AIslandGenerationTest::InitializeObjectives(int startingIndex, int endingIndex)
{
	auto& Flag = GenerationValuesComponent->FlagObjectiveValues;
	int flagAmount = FMath::RandRange(Flag.FlagAmountMin, Flag.FlagAmountMax);
	int enemyZoneAmount = 1;
	int survivalZoneAmount = 1;

	switch (IslandObjective)
	{
	case EIslandObjectiveType::CaptureTheFlag:
		ObjectiveAmount = flagAmount;

		//Override previously initialized tiles with capturable flag tiles
		for (int i = 0; i < ObjectiveAmount; i++)
		{
			int keyTileIndex = FMath::RandRange(0, (IslandDepth * IslandWidth) - 1);

			int whileLoopLimit = 0;

			//If the currently selected tile can be overriden, do that, if not, find another tile. Repeat 10 time. if failed every time, give up.
			while (FloorValuesArray[keyTileIndex].IslandFloorObstacle > EIslandFloorObstacle::Basic)
			{
				keyTileIndex = FMath::RandRange(0, (IslandDepth * IslandWidth) - 1);

				whileLoopLimit++;

				if (whileLoopLimit >= 10)
				{
					break;
				}
			}

			//If it can, override the currently selected tile with the key tile and create a path from the starting tile to it.
			if (whileLoopLimit < 10)
			{
				SetFlagTile(keyTileIndex);
				FindStartToEndPath(startingIndex, keyTileIndex, IslandWidth);
			}
		}
		break;
	case EIslandObjectiveType::MoveThePayload:
		if (IslandObjective == EIslandObjectiveType::MoveThePayload)
		{
			if (IslandDepth * IslandWidth > 1)
			{
				if (GenerationValuesComponent->PayloadObjectiveValues.Payload != nullptr)
				{
					SplinePath->SetLocationAtSplinePoint(0, GetActorLocation(), ESplineCoordinateSpace::World, true);

					float tileScale = GenerationValuesComponent->IslandValues.TileScale;
					int x = startingIndex % IslandWidth;
					int y = startingIndex / IslandWidth;

					FVector posOffset = FVector((tileScale * x * 4) + tileScale, (tileScale * y * 4) + tileScale, 0.0f) + GetActorLocation();

					ANewPayloadObjective* payload = GetWorld()->SpawnActor<ANewPayloadObjective>(GenerationValuesComponent->PayloadObjectiveValues.Payload, posOffset, FRotator::ZeroRotator);

					CreateSplinePathStartToEnd(startingIndex, endingIndex, IslandWidth, payload);

					FObjectivePayloadValues val = GenerationValuesComponent->PayloadObjectiveValues;

					payload->Initialize(val.PayloadSpeed, val.Acceleration, val.Deceleration, SplinePath);
					payload->Multicast_Initialize(val.PayloadSpeed, SplinePath);
					payload->OnObjectiveCreated.AddDynamic(this, &AIslandGenerationTest::IncrementObjective);
					payload->OnObjectiveActivated.AddDynamic(this, &AIslandGenerationTest::DecrementObjective);
					payload->SetActorHiddenInGame(true);

					ObjectiveArrays.Add(payload);
				}
			}
		}

		break;
	case EIslandObjectiveType::Survival:

		auto& Survival = GenerationValuesComponent->SurvivalZoneObjectiveValues;
		ObjectiveAmount = survivalZoneAmount;

		//Override previously initialized tiles with survival zone tiles
		for (int i = 0; i < ObjectiveAmount; i++)
		{
			int keyTileIndex = FMath::RandRange(0, (IslandDepth * IslandWidth) - 1);

			int whileLoopLimit = 0;

			//If the currently selected tile can be overriden, do that, if not, find another tile. Repeat 10 time. if failed every time, give up.
			while (FloorValuesArray[keyTileIndex].IslandFloorObstacle > EIslandFloorObstacle::Basic)
			{
				keyTileIndex = FMath::RandRange(0, (IslandDepth * IslandWidth) - 1);

				whileLoopLimit++;

				if (whileLoopLimit >= 10)
				{
					break;
				}
			}

			//If it can, override the currently selected tile with the key tile and create a path from the starting tile to it.
			if (whileLoopLimit < 10)
			{
				SetSurvivalZoneTile(keyTileIndex);

				FindStartToEndPath(startingIndex, keyTileIndex, IslandWidth);
			}
		}
		break;
	}
}

void AIslandGenerationTest::CheckFloor(int x, int y, int width, int depth)
{
	bool checkLeft = x > 0;
	bool checkRight = x < width - 1;

	bool checkUp = y > 0;
	bool checkDown = y < depth - 1;

	if (checkUp)
	{
		FloorCheckTopNeighbours(checkLeft, checkRight, -width, x, y, width);
	}

	FloorCheckMiddleNeighbours(checkLeft, checkRight, 0, x, y, width);

	if (checkDown)
	{
		FloorCheckBottomNeighbours(checkLeft, checkRight, width, x, y, width);
	}

	SetFloorShape(x, y, width);
}

void AIslandGenerationTest::FloorCheckTopNeighbours(bool checkLeft, bool checkRight, int indexOffset, int x, int y, int width)
{
	int indexToCheck = x + (y * width) + indexOffset;
	int index = x + (y * width);

	if (checkLeft)
	{
		if (FloorValuesArray[indexToCheck - 1].IsFullTile)
		{
			FloorValuesArray[index].FloorNeighbours |= EIslandFloorNeighbours::TopLeft;
			FloorValuesArray[index].ObstacleNeighbours |= EIslandFloorNeighbours::TopLeft;
		}
		else
		{
			FloorValuesArray[index].FloorNeighbours &= ~EIslandFloorNeighbours::TopLeft;
		}
	}

	if (FloorValuesArray[indexToCheck].IsFullTile)
	{
		FloorValuesArray[index].FloorNeighbours |= EIslandFloorNeighbours::TopMiddle;
		FloorValuesArray[index].ObstacleNeighbours |= EIslandFloorNeighbours::TopMiddle;
	}
	else
	{
		FloorValuesArray[index].FloorNeighbours &= ~EIslandFloorNeighbours::TopMiddle;
	}

	if (checkRight)
	{
		if (FloorValuesArray[indexToCheck + 1].IsFullTile)
		{
			FloorValuesArray[index].FloorNeighbours |= EIslandFloorNeighbours::TopRight;
			FloorValuesArray[index].ObstacleNeighbours |= EIslandFloorNeighbours::TopRight;
		}
		else
		{
			FloorValuesArray[index].FloorNeighbours &= ~EIslandFloorNeighbours::TopRight;
		}
	}
}

void AIslandGenerationTest::FloorCheckMiddleNeighbours(bool checkLeft, bool checkRight, int indexOffset, int x, int y, int width)
{
	int indexToCheck = x + (y * width) + indexOffset;
	int index = x + (y * width);

	if (checkLeft)
	{
		if (FloorValuesArray[indexToCheck - 1].IsFullTile)
		{
			FloorValuesArray[index].FloorNeighbours |= EIslandFloorNeighbours::MiddleLeft;
			FloorValuesArray[index].ObstacleNeighbours |= EIslandFloorNeighbours::MiddleLeft;
		}
		else
		{
			FloorValuesArray[index].FloorNeighbours &= ~EIslandFloorNeighbours::MiddleLeft;
		}
	}

	if (checkRight)
	{
		if (FloorValuesArray[indexToCheck + 1].IsFullTile)
		{
			FloorValuesArray[index].FloorNeighbours |= EIslandFloorNeighbours::MiddleRight;
			FloorValuesArray[index].ObstacleNeighbours |= EIslandFloorNeighbours::MiddleRight;
		}
		else
		{
			FloorValuesArray[index].FloorNeighbours &= ~EIslandFloorNeighbours::MiddleRight;
		}
	}
}

void AIslandGenerationTest::FloorCheckBottomNeighbours(bool checkLeft, bool checkRight, int indexOffset, int x, int y, int width)
{
	int indexToCheck = x + (y * width) + indexOffset;
	int index = x + (y * width);

	if (checkLeft)
	{
		if (FloorValuesArray[indexToCheck - 1].IsFullTile)
		{
			FloorValuesArray[index].FloorNeighbours |= EIslandFloorNeighbours::BottomLeft;
			FloorValuesArray[index].ObstacleNeighbours |= EIslandFloorNeighbours::BottomLeft;
		}
		else
		{
			FloorValuesArray[index].FloorNeighbours &= ~EIslandFloorNeighbours::BottomLeft;
		}
	}

	if (FloorValuesArray[indexToCheck].IsFullTile)
	{
		FloorValuesArray[index].FloorNeighbours |= EIslandFloorNeighbours::BottomMiddle;
		FloorValuesArray[index].ObstacleNeighbours |= EIslandFloorNeighbours::BottomMiddle;
	}
	else
	{
		FloorValuesArray[index].FloorNeighbours &= ~EIslandFloorNeighbours::BottomMiddle;
	}

	if (checkRight)
	{
		if (FloorValuesArray[indexToCheck + 1].IsFullTile)
		{
			FloorValuesArray[index].FloorNeighbours |= EIslandFloorNeighbours::BottomRight;
			FloorValuesArray[index].ObstacleNeighbours |= EIslandFloorNeighbours::BottomRight;
		}
		else
		{
			FloorValuesArray[index].FloorNeighbours &= ~EIslandFloorNeighbours::BottomRight;
		}
	}
}

void AIslandGenerationTest::SetFloorShape(int x, int y, int width)
{
	SetTopLeftFloorShape(x, y, width);
	SetTopRightFloorShape(x, y, width);
	SetBottomLeftFloorShape(x, y, width);
	SetBottomRightFloorShape(x, y, width);
}

void AIslandGenerationTest::SetTopLeftFloorShape(int x, int y, int width)
{
	float tileScale = GenerationValuesComponent->IslandValues.TileScale;

	int index = x + (y * width);
	FVector posOffset = FVector((x * 2), (y * 2), 0.0f) * (tileScale * 2);
	FRotator rotationOffset = FRotator(0.0f, 0.0f, 0.0f);
	float scaleOffset = tileScale * 2;

	FTransform floorTransform;
	floorTransform.SetLocation(posOffset);
	floorTransform.SetRotation(rotationOffset.Quaternion());
	floorTransform.SetScale3D(FVector(scaleOffset));

	FVector topPosOffset = FVector((x * 2) - 0.5f, (y * 2), -0.5f) * (tileScale * 2);
	FRotator topRotationOffset = FRotator(90.0f, 0.0f, 0.0f);
	float topScaleOffset = tileScale * 2;

	FTransform topTransform;
	topTransform.SetLocation(topPosOffset);
	topTransform.SetRotation(topRotationOffset.Quaternion());
	topTransform.SetScale3D(FVector(topScaleOffset));


	FVector leftPosOffset = FVector((x * 2), (y * 2) - 0.5f, -0.5f) * (tileScale * 2);
	FRotator leftRotationOffset = FRotator(0.0f, 0.0f, 90.0f);
	float leftScaleOffset = tileScale * 2;

	FTransform leftTransform;
	leftTransform.SetLocation(leftPosOffset);
	leftTransform.SetRotation(leftRotationOffset.Quaternion());
	leftTransform.SetScale3D(FVector(leftScaleOffset));

	EIslandFloorNeighbours topLeftCornerCheck = FloorValuesArray[index].FloorNeighbours & (EIslandFloorNeighbours::TopLeft | EIslandFloorNeighbours::TopMiddle | EIslandFloorNeighbours::MiddleLeft);

	if (FloorValuesArray[index].IsFullTile)
	{
		switch (topLeftCornerCheck)
		{
		case EIslandFloorNeighbours::TopMiddle:
		case EIslandFloorNeighbours::TopMiddle | EIslandFloorNeighbours::TopLeft:
			FullFloorPieceTransforms.Add(floorTransform);
			FullFloorPieceTransforms.Add(topTransform);
			break;
		case EIslandFloorNeighbours::MiddleLeft:
		case EIslandFloorNeighbours::MiddleLeft | EIslandFloorNeighbours::TopLeft:
			FullFloorPieceTransforms.Add(floorTransform);
			FullFloorPieceTransforms.Add(leftTransform);
			break;
		case EIslandFloorNeighbours::TopMiddle | EIslandFloorNeighbours::MiddleLeft:
		case EIslandFloorNeighbours::TopLeft | EIslandFloorNeighbours::TopMiddle | EIslandFloorNeighbours::MiddleLeft:
			FullFloorPieceTransforms.Add(floorTransform);
			break;
		case EIslandFloorNeighbours::TopLeft:
		default:
			FullFloorPieceTransforms.Add(floorTransform);
			FullFloorPieceTransforms.Add(topTransform);
			FullFloorPieceTransforms.Add(leftTransform);
		}
	}
}

void AIslandGenerationTest::SetTopRightFloorShape(int x, int y, int width)
{
	float tileScale = GenerationValuesComponent->IslandValues.TileScale;

	int index = x + (y * width);
	FVector posOffset = FVector((x * 2) + 1.0f, (y * 2), 0.0f) * (tileScale * 2);
	FRotator rotationOffset = FRotator(0.0f, 0.0f, 0.0f);
	float scaleOffset = tileScale * 2;

	FTransform floorTransform;
	floorTransform.SetLocation(posOffset);
	floorTransform.SetRotation(rotationOffset.Quaternion());
	floorTransform.SetScale3D(FVector(scaleOffset));

	FVector topPosOffset = FVector((x * 2)+1.5f, (y * 2), -0.5f) * (tileScale * 2);
	FRotator topRotationOffset = FRotator(90.0f, 0.0f, 0.0f);
	float topScaleOffset = tileScale * 2;

	FTransform topTransform;
	topTransform.SetLocation(topPosOffset);
	topTransform.SetRotation(topRotationOffset.Quaternion());
	topTransform.SetScale3D(FVector(topScaleOffset));


	FVector rightPosOffset = FVector((x * 2)+1, (y * 2)-0.5f, -0.5f) * (tileScale * 2);
	FRotator rightRotationOffset = FRotator(0.0f, 0.0f, 90.0f);
	float rightScaleOffset = tileScale * 2;

	FTransform rightTransform;
	rightTransform.SetLocation(rightPosOffset);
	rightTransform.SetRotation(rightRotationOffset.Quaternion());
	rightTransform.SetScale3D(FVector(rightScaleOffset));

	EIslandFloorNeighbours topRightCornerCheck = FloorValuesArray[index].FloorNeighbours & (EIslandFloorNeighbours::TopRight | EIslandFloorNeighbours::TopMiddle | EIslandFloorNeighbours::MiddleRight);

	if (FloorValuesArray[index].IsFullTile)
	{
		switch (topRightCornerCheck)
		{
		case EIslandFloorNeighbours::TopMiddle:
		case EIslandFloorNeighbours::TopMiddle | EIslandFloorNeighbours::TopRight:
			FullFloorPieceTransforms.Add(topTransform);
			FullFloorPieceTransforms.Add(floorTransform);
			break;
		case EIslandFloorNeighbours::MiddleRight:
		case EIslandFloorNeighbours::MiddleRight | EIslandFloorNeighbours::TopRight:
			FullFloorPieceTransforms.Add(rightTransform);
			FullFloorPieceTransforms.Add(floorTransform);
			break;
		case EIslandFloorNeighbours::TopMiddle | EIslandFloorNeighbours::MiddleRight:
		case EIslandFloorNeighbours::TopRight | EIslandFloorNeighbours::TopMiddle | EIslandFloorNeighbours::MiddleRight:
			FullFloorPieceTransforms.Add(floorTransform);
			break;
		case EIslandFloorNeighbours::TopRight:
		default:
			FullFloorPieceTransforms.Add(floorTransform);
			FullFloorPieceTransforms.Add(rightTransform);
			FullFloorPieceTransforms.Add(topTransform);
			break;
		}
	}
}

void AIslandGenerationTest::SetBottomLeftFloorShape(int x, int y, int width)
{
	float tileScale = GenerationValuesComponent->IslandValues.TileScale;

	int index = x + (y * width);
	FVector posOffset = FVector((x * 2), (y * 2) + 1.0f, 0.0f) * (tileScale * 2);
	FRotator rotationOffset = FRotator(0.0f, 0.0f, 0.0f);
	float scaleOffset = tileScale * 2;

	FTransform floorTransform;
	floorTransform.SetLocation(posOffset);
	floorTransform.SetRotation(rotationOffset.Quaternion());
	floorTransform.SetScale3D(FVector(scaleOffset));

	FVector bottomPosOffset = FVector((x * 2) - 0.5f, (y * 2)+1.0f, -0.5f) * (tileScale * 2);
	FRotator bottomRotationOffset = FRotator(90.0f, 0.0f, 0.0f);
	float bottomScaleOffset = tileScale * 2;

	FTransform bottomTransform;
	bottomTransform.SetLocation(bottomPosOffset);
	bottomTransform.SetRotation(bottomRotationOffset.Quaternion());
	bottomTransform.SetScale3D(FVector(bottomScaleOffset));

	FVector leftPosOffset = FVector((x * 2), (y * 2)+1.5f, -0.5f) * (tileScale * 2);
	FRotator leftRotationOffset = FRotator(0.0f, 0.0f, 90.0f);
	float leftScaleOffset = tileScale * 2;

	FTransform leftTransform;
	leftTransform.SetLocation(leftPosOffset);
	leftTransform.SetRotation(leftRotationOffset.Quaternion());
	leftTransform.SetScale3D(FVector(leftScaleOffset));

	EIslandFloorNeighbours bottomLeftCornerCheck = FloorValuesArray[index].FloorNeighbours & (EIslandFloorNeighbours::BottomLeft | EIslandFloorNeighbours::BottomMiddle | EIslandFloorNeighbours::MiddleLeft);

	if (FloorValuesArray[index].IsFullTile)
	{
		switch (bottomLeftCornerCheck)
		{
		case EIslandFloorNeighbours::BottomMiddle:
		case EIslandFloorNeighbours::BottomMiddle | EIslandFloorNeighbours::BottomLeft:
			FullFloorPieceTransforms.Add(bottomTransform);
			FullFloorPieceTransforms.Add(floorTransform);
			break;
		case EIslandFloorNeighbours::MiddleLeft:
		case EIslandFloorNeighbours::MiddleLeft | EIslandFloorNeighbours::BottomLeft:
			FullFloorPieceTransforms.Add(leftTransform);
			FullFloorPieceTransforms.Add(floorTransform);
			break;
		case EIslandFloorNeighbours::BottomMiddle | EIslandFloorNeighbours::MiddleLeft:
		case EIslandFloorNeighbours::BottomLeft | EIslandFloorNeighbours::BottomMiddle | EIslandFloorNeighbours::MiddleLeft:
			FullFloorPieceTransforms.Add(floorTransform);
			break;
		case EIslandFloorNeighbours::BottomLeft:
		default:
			FullFloorPieceTransforms.Add(floorTransform);
			FullFloorPieceTransforms.Add(leftTransform);
			FullFloorPieceTransforms.Add(bottomTransform);
			break;
		}
	}
}

void AIslandGenerationTest::SetBottomRightFloorShape(int x, int y, int width)
{
	float tileScale = GenerationValuesComponent->IslandValues.TileScale;

	int index = x + (y * width);
	FVector posOffset = FVector((x * 2) + 1.0, (y * 2) + 1.0f, 0.0f) * (tileScale * 2);
	FRotator rotationOffset = FRotator(0.0f, 0.0f, 0.0f);
	float scaleOffset = tileScale * 2;

	FTransform floorTransform;
	floorTransform.SetLocation(posOffset);
	floorTransform.SetRotation(rotationOffset.Quaternion());
	floorTransform.SetScale3D(FVector(scaleOffset));

	FVector bottomPosOffset = FVector((x * 2) + 1.5f, (y * 2) + 1.0f, -0.5f) * (tileScale * 2);
	FRotator bottomRotationOffset = FRotator(90.0f, 0.0f, 0.0f);
	float bottomScaleOffset = tileScale * 2;

	FTransform bottomTransform;
	bottomTransform.SetLocation(bottomPosOffset);
	bottomTransform.SetRotation(bottomRotationOffset.Quaternion());
	bottomTransform.SetScale3D(FVector(bottomScaleOffset));


	FVector rightPosOffset = FVector((x * 2) + 1.0f, (y * 2)+1.5f, -0.5f) * (tileScale * 2);
	FRotator rightRotationOffset = FRotator(0.0f, 0.0f, 90.0f);
	float rightScaleOffset = tileScale * 2;

	FTransform rightTransform;
	rightTransform.SetLocation(rightPosOffset);
	rightTransform.SetRotation(rightRotationOffset.Quaternion());
	rightTransform.SetScale3D(FVector(rightScaleOffset));

	EIslandFloorNeighbours bottomRightCornerCheck = FloorValuesArray[index].FloorNeighbours & (EIslandFloorNeighbours::BottomRight | EIslandFloorNeighbours::BottomMiddle | EIslandFloorNeighbours::MiddleRight);

	if (FloorValuesArray[index].IsFullTile)
	{
		switch (bottomRightCornerCheck)
		{
		case EIslandFloorNeighbours::BottomMiddle:
		case EIslandFloorNeighbours::BottomMiddle | EIslandFloorNeighbours::BottomRight:
			FullFloorPieceTransforms.Add(floorTransform);
			FullFloorPieceTransforms.Add(bottomTransform);
			break;
		case EIslandFloorNeighbours::MiddleRight:
		case EIslandFloorNeighbours::MiddleRight | EIslandFloorNeighbours::BottomRight:
			rotationOffset.Yaw = -90.0f;
			FullFloorPieceTransforms.Add(floorTransform);
			FullFloorPieceTransforms.Add(rightTransform);
			break;
		case EIslandFloorNeighbours::BottomMiddle | EIslandFloorNeighbours::MiddleRight:
		case EIslandFloorNeighbours::BottomRight | EIslandFloorNeighbours::BottomMiddle | EIslandFloorNeighbours::MiddleRight:
			FullFloorPieceTransforms.Add(floorTransform);
			break;
		case EIslandFloorNeighbours::BottomRight:
		default:
			FullFloorPieceTransforms.Add(floorTransform);
			FullFloorPieceTransforms.Add(rightTransform);
			FullFloorPieceTransforms.Add(bottomTransform);
			break;
		}
	}
}

void AIslandGenerationTest::SetWall(int tileIndex, FVector posOffset, TSubclassOf<class APackedLevelActor> CornerWall, TSubclassOf<class APackedLevelActor> SideWall)
{
	SetTopLeftWall(tileIndex, posOffset, CornerWall, SideWall);
	SetTopRightWall(tileIndex, posOffset, CornerWall, SideWall);
	SetBottomLeftWall(tileIndex, posOffset, CornerWall, SideWall);
	SetBottomRightWall(tileIndex, posOffset, CornerWall, SideWall);
}

void AIslandGenerationTest::SetTopLeftWall(int tileIndex, FVector posOffset, TSubclassOf<class APackedLevelActor> CornerWall, TSubclassOf<class APackedLevelActor> SideWall)
{
	float tileScale = GenerationValuesComponent->IslandValues.TileScale;

	FVector newPos = FVector(posOffset.X - tileScale, posOffset.Y - tileScale, 0.0f);

	EIslandFloorNeighbours topLeftCornerCheck = FloorValuesArray[tileIndex].FloorNeighbours & (EIslandFloorNeighbours::TopLeft | EIslandFloorNeighbours::TopMiddle | EIslandFloorNeighbours::MiddleLeft);

	switch (topLeftCornerCheck)
	{
	case EIslandFloorNeighbours::TopMiddle:
	case EIslandFloorNeighbours::TopMiddle | EIslandFloorNeighbours::TopLeft:
		if (SideWall)
		{
			APackedLevelActor* side = GetWorld()->SpawnActor<APackedLevelActor>(SideWall, newPos, FRotator(0.0f, 0.0f, 0.0f));
			if (side->Implements<UCullingToggleInterface>())
			{
				ICullingToggleInterface::Execute_ToggleVisibility(side, false);
			}
			ObstacleArrays.Add(side);

		}
		break;
	case EIslandFloorNeighbours::MiddleLeft:
	case EIslandFloorNeighbours::MiddleLeft | EIslandFloorNeighbours::TopLeft:
		if (SideWall)
		{
			APackedLevelActor* side = GetWorld()->SpawnActor<APackedLevelActor>(SideWall, newPos, FRotator(0.0f, 90.0f, 0.0f));
			if (side->Implements<UCullingToggleInterface>())
			{
				ICullingToggleInterface::Execute_ToggleVisibility(side, false);
			}
			ObstacleArrays.Add(side);
		}
		break;
	case EIslandFloorNeighbours::TopMiddle | EIslandFloorNeighbours::MiddleLeft:
	case EIslandFloorNeighbours::TopLeft | EIslandFloorNeighbours::TopMiddle | EIslandFloorNeighbours::MiddleLeft:
		break;
	case EIslandFloorNeighbours::TopLeft:
		if (CornerWall)
		{
			APackedLevelActor* corner = GetWorld()->SpawnActor<APackedLevelActor>(CornerWall, newPos, FRotator(0.0f, 0.0f, 0.0f));
			if (corner->Implements<UCullingToggleInterface>())
			{
				ICullingToggleInterface::Execute_ToggleVisibility(corner, false);
			}
			ObstacleArrays.Add(corner);
		}
		break;
	default:
		if (CornerWall)
		{
			APackedLevelActor* corner = GetWorld()->SpawnActor<APackedLevelActor>(CornerWall, newPos, FRotator(0.0f, 0.0f, 0.0f));
			if (corner->Implements<UCullingToggleInterface>())
			{
				ICullingToggleInterface::Execute_ToggleVisibility(corner, false);
			}
			ObstacleArrays.Add(corner);
		}
	}
}

void AIslandGenerationTest::SetTopRightWall(int tileIndex, FVector posOffset, TSubclassOf<class APackedLevelActor> CornerWall, TSubclassOf<class APackedLevelActor> SideWall)
{
	float tileScale = GenerationValuesComponent->IslandValues.TileScale;

	FVector newPos = FVector(posOffset.X + tileScale, posOffset.Y - tileScale, 0.0f);

	EIslandFloorNeighbours topRightCornerCheck = FloorValuesArray[tileIndex].FloorNeighbours & (EIslandFloorNeighbours::TopRight | EIslandFloorNeighbours::TopMiddle | EIslandFloorNeighbours::MiddleRight);

	switch (topRightCornerCheck)
	{
	case EIslandFloorNeighbours::TopMiddle:
	case EIslandFloorNeighbours::TopMiddle | EIslandFloorNeighbours::TopRight:
		if (SideWall)
		{
			APackedLevelActor* side = GetWorld()->SpawnActor<APackedLevelActor>(SideWall, newPos, FRotator(0.0f, 180.0f, 0.0f));
			if (side->Implements<UCullingToggleInterface>())
			{
				ICullingToggleInterface::Execute_ToggleVisibility(side, false);
			}
			ObstacleArrays.Add(side);
		}
		break;
	case EIslandFloorNeighbours::MiddleRight:
	case EIslandFloorNeighbours::MiddleRight | EIslandFloorNeighbours::TopRight:
		if (SideWall)
		{
			APackedLevelActor* side = GetWorld()->SpawnActor<APackedLevelActor>(SideWall, newPos, FRotator(0.0f, 90.0f, 0.0f));
			if (side->Implements<UCullingToggleInterface>())
			{
				ICullingToggleInterface::Execute_ToggleVisibility(side, false);
			}
			ObstacleArrays.Add(side);
		}
		break;
	case EIslandFloorNeighbours::TopMiddle | EIslandFloorNeighbours::MiddleRight:
	case EIslandFloorNeighbours::TopRight | EIslandFloorNeighbours::TopMiddle | EIslandFloorNeighbours::MiddleRight:
		break;
	case EIslandFloorNeighbours::TopRight:
		if (CornerWall)
		{
			APackedLevelActor* corner = GetWorld()->SpawnActor<APackedLevelActor>(CornerWall, newPos, FRotator(0.0f, 90.0f, 0.0f));
			if (corner->Implements<UCullingToggleInterface>())
			{
				ICullingToggleInterface::Execute_ToggleVisibility(corner, false);
			}
			ObstacleArrays.Add(corner);
		}
		break;
	default:
		if (CornerWall)
		{
			APackedLevelActor* corner = GetWorld()->SpawnActor<APackedLevelActor>(CornerWall, newPos, FRotator(0.0f, 90.0f, 0.0f));
			if (corner->Implements<UCullingToggleInterface>())
			{
				ICullingToggleInterface::Execute_ToggleVisibility(corner, false);
			}
			ObstacleArrays.Add(corner);
		}
		break;
	}
}

void AIslandGenerationTest::SetBottomLeftWall(int tileIndex, FVector posOffset, TSubclassOf<class APackedLevelActor> CornerWall, TSubclassOf<class APackedLevelActor> SideWall)
{
	float tileScale = GenerationValuesComponent->IslandValues.TileScale;

	FVector newPos = FVector(posOffset.X - tileScale, posOffset.Y + tileScale, 0.0f);

	EIslandFloorNeighbours bottomLeftCornerCheck = FloorValuesArray[tileIndex].FloorNeighbours & (EIslandFloorNeighbours::BottomLeft | EIslandFloorNeighbours::BottomMiddle | EIslandFloorNeighbours::MiddleLeft);

	switch (bottomLeftCornerCheck)
	{
	case EIslandFloorNeighbours::BottomMiddle:
	case EIslandFloorNeighbours::BottomMiddle | EIslandFloorNeighbours::BottomLeft:
		if (SideWall)
		{
			APackedLevelActor* side = GetWorld()->SpawnActor<APackedLevelActor>(SideWall, newPos, FRotator(0.0f, 0.0f, 0.0f));
			if (side->Implements<UCullingToggleInterface>())
			{
				ICullingToggleInterface::Execute_ToggleVisibility(side, false);
			}
			ObstacleArrays.Add(side);
		}
		break;
	case EIslandFloorNeighbours::MiddleLeft:
	case EIslandFloorNeighbours::MiddleLeft | EIslandFloorNeighbours::BottomLeft:
		if (SideWall)
		{
			APackedLevelActor* side = GetWorld()->SpawnActor<APackedLevelActor>(SideWall, newPos, FRotator(0.0f, 270.0f, 0.0f));
			if (side->Implements<UCullingToggleInterface>())
			{
				ICullingToggleInterface::Execute_ToggleVisibility(side, false);
			}
			ObstacleArrays.Add(side);
		}
		break;
	case EIslandFloorNeighbours::BottomMiddle | EIslandFloorNeighbours::MiddleLeft:
	case EIslandFloorNeighbours::BottomLeft | EIslandFloorNeighbours::BottomMiddle | EIslandFloorNeighbours::MiddleLeft:
		break;
	case EIslandFloorNeighbours::BottomLeft:
		if (CornerWall)
		{
			APackedLevelActor* corner = GetWorld()->SpawnActor<APackedLevelActor>(CornerWall, newPos, FRotator(0.0f, 270.0f, 0.0f));
			if (corner->Implements<UCullingToggleInterface>())
			{
				ICullingToggleInterface::Execute_ToggleVisibility(corner, false);
			}
			ObstacleArrays.Add(corner);
		}
		break;
	default:
		if (CornerWall)
		{
			APackedLevelActor* corner = GetWorld()->SpawnActor<APackedLevelActor>(CornerWall, newPos, FRotator(0.0f, 270.0f, 0.0f));
			if (corner->Implements<UCullingToggleInterface>())
			{
				ICullingToggleInterface::Execute_ToggleVisibility(corner, false);
			}
			ObstacleArrays.Add(corner);
		}
		break;
	}
}

void AIslandGenerationTest::SetBottomRightWall(int tileIndex, FVector posOffset, TSubclassOf<class APackedLevelActor> CornerWall, TSubclassOf<class APackedLevelActor> SideWall)
{
	float tileScale = GenerationValuesComponent->IslandValues.TileScale;

	FVector newPos = FVector(posOffset.X + tileScale, posOffset.Y + tileScale, 0.0f);

	EIslandFloorNeighbours bottomRightCornerCheck = FloorValuesArray[tileIndex].FloorNeighbours & (EIslandFloorNeighbours::BottomRight | EIslandFloorNeighbours::BottomMiddle | EIslandFloorNeighbours::MiddleRight);

	switch (bottomRightCornerCheck)
	{
	case EIslandFloorNeighbours::BottomMiddle:
	case EIslandFloorNeighbours::BottomMiddle | EIslandFloorNeighbours::BottomRight:
		if (SideWall)
		{
			APackedLevelActor* side = GetWorld()->SpawnActor<APackedLevelActor>(SideWall, newPos, FRotator(0.0f, 180.0f, 0.0f));
			if (side->Implements<UCullingToggleInterface>())
			{
				ICullingToggleInterface::Execute_ToggleVisibility(side, false);
			}
			ObstacleArrays.Add(side);
		}
		break;
	case EIslandFloorNeighbours::MiddleRight:
	case EIslandFloorNeighbours::MiddleRight | EIslandFloorNeighbours::BottomRight:
		if (SideWall)
		{
			APackedLevelActor* side = GetWorld()->SpawnActor<APackedLevelActor>(SideWall, newPos, FRotator(0.0f, -90.0f, 0.0f));
			if (side->Implements<UCullingToggleInterface>())
			{
				ICullingToggleInterface::Execute_ToggleVisibility(side, false);
			}
			ObstacleArrays.Add(side);
		}
		break;
	case EIslandFloorNeighbours::BottomMiddle | EIslandFloorNeighbours::MiddleRight:
	case EIslandFloorNeighbours::BottomRight | EIslandFloorNeighbours::BottomMiddle | EIslandFloorNeighbours::MiddleRight:
		break;
	case EIslandFloorNeighbours::BottomRight:
		if (CornerWall)
		{
			APackedLevelActor* corner = GetWorld()->SpawnActor<APackedLevelActor>(CornerWall, newPos, FRotator(0.0f, 180.0f, 0.0f));
			if (corner->Implements<UCullingToggleInterface>())
			{
				ICullingToggleInterface::Execute_ToggleVisibility(corner, false);
			}
			ObstacleArrays.Add(corner);
		}
		break;
	default:
		if (CornerWall)
		{
			APackedLevelActor* corner = GetWorld()->SpawnActor<APackedLevelActor>(CornerWall, newPos, FRotator(0.0f, 180.0f, 0.0f));
			if (corner->Implements<UCullingToggleInterface>())
			{
				ICullingToggleInterface::Execute_ToggleVisibility(corner, false);
			}
			ObstacleArrays.Add(corner);
		}
		break;
	}
}

void AIslandGenerationTest::Server_SpawnFloorObstacles_Implementation(int x, int y, int width)
{
	SpawnFloorObstacles(x, y, width);
}

void AIslandGenerationTest::SpawnFloorObstacles(int x, int y, int width)
{
	int index = x + (y * width);

	float tileScale = GenerationValuesComponent->IslandValues.TileScale;

	float yaw = FloorValuesArray[index].obstacleYaw;
	FVector posOffset = FVector((tileScale * x * 4) + tileScale, (tileScale * y * 4) + tileScale, 0.0f) + GetActorLocation();

	TArray<TSubclassOf<APackedLevelActor>> vantageArray = GenerationValuesComponent->VantageValues.VantageActor;

	TArray<TSubclassOf<APackedLevelActor>> buildingArray = GenerationValuesComponent->ObstacleTypeValues.BuildingObstacles.ObstacleActor;
	TArray<TSubclassOf<APackedLevelActor>> environmentArray = GenerationValuesComponent->ObstacleTypeValues.EnvironmentObstacles.ObstacleActor;
	TSubclassOf<class APackedLevelActor> buildingWallTemplate = GenerationValuesComponent->ObstacleTypeValues.EnvironmentObstacles.ObstacleWall;
	TSubclassOf<class APackedLevelActor> environmentWallTemplate = GenerationValuesComponent->ObstacleTypeValues.EnvironmentObstacles.ObstacleWall;

	uint8 max = 0;
	uint8 remaining = 0;
	TSubclassOf<APackedLevelActor> packedActorTemp = nullptr;
	TSubclassOf<APackedLevelActor> packedWallTemp = nullptr;

	TSubclassOf<class APackedLevelActor> obstacleWallTemplate = GenerationValuesComponent->ObstacleValues.ObstacleWall;
	TSubclassOf<class APackedLevelActor> basicWallTemplate = GenerationValuesComponent->IslandValues.BasicWall;
	TSubclassOf<class APackedLevelActor> cornerWallTemplate = GenerationValuesComponent->IslandValues.CornerWall;
	TSubclassOf<class APackedLevelActor> startEndWallTemplate = GenerationValuesComponent->IslandStartEnd.StartEndWall;

	switch (FloorValuesArray[index].IslandFloorObstacle)
	{
	case EIslandFloorObstacle::Basic:

		switch (FloorValuesArray[index].ObstacleType)
		{
		case EObstacleType::Building:
			max = GenerationValuesComponent->ObstacleTypeValues.BuildingObstacleMaxAmount;
			remaining = GenerationValuesComponent->ObstacleTypeValues.RemainingBuildingObstacleAmount;
			if (remaining >= max)
			{
				if (cornerWallTemplate != nullptr && basicWallTemplate != nullptr)
				{
					if (FloorValuesArray[index].IsFullTile)
					{
						SetWall(index, posOffset, cornerWallTemplate, basicWallTemplate);
					}
				}

				return;
			}

			GenerationValuesComponent->ObstacleTypeValues.RemainingBuildingObstacleAmount += 1;

			if (buildingArray.IsValidIndex(FloorValuesArray[index].randomBuildingObstacleToSpawn))
			{
				packedActorTemp = buildingArray[FloorValuesArray[index].randomBuildingObstacleToSpawn];
			}

			if (buildingWallTemplate)
			{
				packedWallTemp = buildingWallTemplate;
			}
			break;
		case EObstacleType::Environment:
			max = GenerationValuesComponent->ObstacleTypeValues.EnvironmentObstacleMaxAmount;
			remaining = GenerationValuesComponent->ObstacleTypeValues.RemainingEnvironmentObstacleAmount;
			if (remaining >= max)
			{
				if (cornerWallTemplate != nullptr && basicWallTemplate != nullptr)
				{
					if (FloorValuesArray[index].IsFullTile)
					{
						SetWall(index, posOffset, cornerWallTemplate, basicWallTemplate);
					}
				}

				return;
			}

			GenerationValuesComponent->ObstacleTypeValues.RemainingEnvironmentObstacleAmount += 1;

			if (environmentArray.IsValidIndex(FloorValuesArray[index].randomEnvironmentObstacleToSpawn))
			{
				packedActorTemp = environmentArray[FloorValuesArray[index].randomEnvironmentObstacleToSpawn];

			}

			if (buildingWallTemplate)
			{
				packedWallTemp = environmentWallTemplate;
			}
			break;
		}

		if (packedActorTemp)
		{
			APackedLevelActor* packedActor = GetWorld()->SpawnActor<APackedLevelActor>(packedActorTemp, posOffset, FRotator(0.0f, yaw, 0.0f));
			if (packedActor->Implements<UCullingToggleInterface>())
			{
				ICullingToggleInterface::Execute_ToggleVisibility(packedActor, false);
			}
			ObstacleArrays.Add(packedActor);

		}

		if (packedWallTemp)
		{
			APackedLevelActor* wall = nullptr;
			if (!EnumHasAnyFlags(FloorValuesArray[index].ObstacleNeighbours, EIslandFloorNeighbours::MiddleLeft))
			{
				wall = GetWorld()->SpawnActor<APackedLevelActor>(packedWallTemp, posOffset, FRotator(0.0f, 0.0f, 0.0f));
				if (wall->Implements<UCullingToggleInterface>())
				{
					ICullingToggleInterface::Execute_ToggleVisibility(wall, false);
				}
				ObstacleArrays.Add(wall);
			}
			if (!EnumHasAnyFlags(FloorValuesArray[index].ObstacleNeighbours, EIslandFloorNeighbours::TopMiddle))
			{
				wall = GetWorld()->SpawnActor<APackedLevelActor>(packedWallTemp, posOffset, FRotator(0.0f, 90.0f, 0.0f));
				if (wall->Implements<UCullingToggleInterface>())
				{
					ICullingToggleInterface::Execute_ToggleVisibility(wall, false);
				}
				ObstacleArrays.Add(wall);
			}
			if (!EnumHasAnyFlags(FloorValuesArray[index].ObstacleNeighbours, EIslandFloorNeighbours::MiddleRight))
			{
				wall = GetWorld()->SpawnActor<APackedLevelActor>(packedWallTemp, posOffset, FRotator(0.0f, 180.0f, 0.0f));
				if (wall->Implements<UCullingToggleInterface>())
				{
					ICullingToggleInterface::Execute_ToggleVisibility(wall, false);
				}
				ObstacleArrays.Add(wall);
			}
			if (!EnumHasAnyFlags(FloorValuesArray[index].ObstacleNeighbours, EIslandFloorNeighbours::BottomMiddle))
			{
				wall = GetWorld()->SpawnActor<APackedLevelActor>(packedWallTemp, posOffset, FRotator(0.0f, 270.0f, 0.0f));
				if (wall->Implements<UCullingToggleInterface>())
				{
					ICullingToggleInterface::Execute_ToggleVisibility(wall, false);
				}
				ObstacleArrays.Add(wall);
			}
		}

		break;
	case EIslandFloorObstacle::Vantage:
		if (vantageArray.IsValidIndex(FloorValuesArray[index].randomFloorToSpawn))
		{
			APackedLevelActor* packed = GetWorld()->SpawnActor<APackedLevelActor>(vantageArray[FloorValuesArray[index].randomFloorToSpawn], posOffset, FRotator(0.0f, yaw, 0.0f));
			if (packed->Implements<UCullingToggleInterface>())
			{
				ICullingToggleInterface::Execute_ToggleVisibility(packed, false);
			}
			ObstacleArrays.Add(packed);
		}

		if (cornerWallTemplate != nullptr && basicWallTemplate != nullptr)
		{
			SetWall(index, posOffset, cornerWallTemplate, basicWallTemplate);
		}

		break;

	case EIslandFloorObstacle::Safe:
		if (GenerationValuesComponent->SafeSpotValues.SafeActor)
		{
			APackedLevelActor* packed = GetWorld()->SpawnActor<APackedLevelActor>(GenerationValuesComponent->SafeSpotValues.SafeActor, posOffset, FRotator(0.0f, yaw, 0.0f));
			if (packed->Implements<UCullingToggleInterface>())
			{
				ICullingToggleInterface::Execute_ToggleVisibility(packed, false);
			}
			ObstacleArrays.Add(packed);
		}

		if (obstacleWallTemplate != nullptr)
		{
			APackedLevelActor* wall = nullptr;
			if (!EnumHasAnyFlags(FloorValuesArray[index].ObstacleNeighbours, EIslandFloorNeighbours::MiddleLeft))
			{
				wall = GetWorld()->SpawnActor<APackedLevelActor>(obstacleWallTemplate, posOffset, FRotator(0.0f, 0.0f, 0.0f));
				if (wall->Implements<UCullingToggleInterface>())
				{
					ICullingToggleInterface::Execute_ToggleVisibility(wall, false);
				}
				ObstacleArrays.Add(wall);
			}
			if (!EnumHasAnyFlags(FloorValuesArray[index].ObstacleNeighbours, EIslandFloorNeighbours::TopMiddle))
			{
				wall = GetWorld()->SpawnActor<APackedLevelActor>(obstacleWallTemplate, posOffset, FRotator(0.0f, 90.0f, 0.0f));
				if (wall->Implements<UCullingToggleInterface>())
				{
					ICullingToggleInterface::Execute_ToggleVisibility(wall, false);
				}
				ObstacleArrays.Add(wall);
			}
			if (!EnumHasAnyFlags(FloorValuesArray[index].ObstacleNeighbours, EIslandFloorNeighbours::MiddleRight))
			{
				wall = GetWorld()->SpawnActor<APackedLevelActor>(obstacleWallTemplate, posOffset, FRotator(0.0f, 180.0f, 0.0f));
				if (wall->Implements<UCullingToggleInterface>())
				{
					ICullingToggleInterface::Execute_ToggleVisibility(wall, false);
				}
				ObstacleArrays.Add(wall);
			}
			if (!EnumHasAnyFlags(FloorValuesArray[index].ObstacleNeighbours, EIslandFloorNeighbours::BottomMiddle))
			{
				wall = GetWorld()->SpawnActor<APackedLevelActor>(obstacleWallTemplate, posOffset, FRotator(0.0f, 270.0f, 0.0f));
				if (wall->Implements<UCullingToggleInterface>())
				{
					ICullingToggleInterface::Execute_ToggleVisibility(wall, false);
				}
				ObstacleArrays.Add(wall);
			}
		}
		break;
	case EIslandFloorObstacle::CapturableFlag:
		if (GenerationValuesComponent->FlagObjectiveValues.Flag != nullptr)
		{
			ANewCapturableFlag* newFlag = GetWorld()->SpawnActor<ANewCapturableFlag>(GenerationValuesComponent->FlagObjectiveValues.Flag, posOffset, FRotator(0.0f, yaw, 0.0f));

			newFlag->OnObjectiveCreated.AddDynamic(this, &AIslandGenerationTest::IncrementObjective);
			newFlag->OnObjectiveActivated.AddDynamic(this, &AIslandGenerationTest::DecrementObjective);
			newFlag->SetActorHiddenInGame(true);

			ObjectiveArrays.Add(newFlag);
		}

		if (GenerationValuesComponent->FlagObjectiveValues.FlagModuleActor.Num())
		{
			int moduleIndex = FMath::RandRange(0, GenerationValuesComponent->FlagObjectiveValues.FlagModuleActor.Num()-1);
			if (GenerationValuesComponent->FlagObjectiveValues.FlagModuleActor[moduleIndex] != nullptr)
			{
				int randYaw = FMath::RandRange(0, 3) * 90;
				APackedLevelActor* aModule = GetWorld()->SpawnActor<APackedLevelActor>(GenerationValuesComponent->FlagObjectiveValues.FlagModuleActor[moduleIndex], posOffset, FRotator(0.0f, randYaw, 0.0f));
				if (aModule->Implements<UCullingToggleInterface>())
				{
					ICullingToggleInterface::Execute_ToggleVisibility(aModule, false);
				}
				ObstacleArrays.Add(aModule);
			}
		}

		if (cornerWallTemplate != nullptr && basicWallTemplate != nullptr)
		{
			SetWall(index, posOffset, cornerWallTemplate, basicWallTemplate);
		}
		break;
	case EIslandFloorObstacle::SurvivalZone:
		if (GenerationValuesComponent->SurvivalZoneObjectiveValues.SurvivalZone != nullptr)
		{
			ANewSurvivalZone* newZone = GetWorld()->SpawnActor<ANewSurvivalZone>(GenerationValuesComponent->SurvivalZoneObjectiveValues.SurvivalZone, posOffset, FRotator(0.0f, yaw, 0.0f));

			newZone->OnObjectiveCreated.AddDynamic(this, &AIslandGenerationTest::IncrementObjective);
			newZone->OnObjectiveActivated.AddDynamic(this, &AIslandGenerationTest::DecrementObjective);
			newZone->Duration = GenerationValuesComponent->SurvivalZoneObjectiveValues.SurvivalDuration;
			newZone->SetActorHiddenInGame(true);

			ObjectiveArrays.Add(newZone);
		}

		if (GenerationValuesComponent->SurvivalZoneObjectiveValues.SurvivalZoneModuleActor.Num())
		{
			int moduleIndex = FMath::RandRange(0, GenerationValuesComponent->SurvivalZoneObjectiveValues.SurvivalZoneModuleActor.Num() - 1);
			if (GenerationValuesComponent->SurvivalZoneObjectiveValues.SurvivalZoneModuleActor[moduleIndex] != nullptr)
			{
				int randYaw = FMath::RandRange(0, 3) * 90;
				APackedLevelActor* aModule = GetWorld()->SpawnActor<APackedLevelActor>(GenerationValuesComponent->SurvivalZoneObjectiveValues.SurvivalZoneModuleActor[moduleIndex], posOffset, FRotator(0.0f, randYaw, 0.0f));
				if (aModule->Implements<UCullingToggleInterface>())
				{
					ICullingToggleInterface::Execute_ToggleVisibility(aModule, false);
				}
				ObstacleArrays.Add(aModule);
			}
		}

		if (cornerWallTemplate != nullptr && basicWallTemplate != nullptr)
		{
			SetWall(index, posOffset, cornerWallTemplate, basicWallTemplate);
		}

		break;
	case EIslandFloorObstacle::Start:

		if (startEndWallTemplate != nullptr)
		{
			APackedLevelActor* wall = nullptr;

			if (!EnumHasAnyFlags(FloorValuesArray[index].ObstacleNeighbours, EIslandFloorNeighbours::MiddleLeft) && IslandIndex == 0)
			{
				wall = GetWorld()->SpawnActor<APackedLevelActor>(startEndWallTemplate, posOffset, FRotator(0.0f, 0.0f, 0.0f));
				if (wall->Implements<UCullingToggleInterface>())
				{
					ICullingToggleInterface::Execute_ToggleVisibility(wall, false);
				}
				ObstacleArrays.Add(wall);
			}
			if (!EnumHasAnyFlags(FloorValuesArray[index].ObstacleNeighbours, EIslandFloorNeighbours::TopMiddle))
			{
				wall = GetWorld()->SpawnActor<APackedLevelActor>(startEndWallTemplate, posOffset, FRotator(0.0f, 90.0f, 0.0f));
				if (wall->Implements<UCullingToggleInterface>())
				{
					ICullingToggleInterface::Execute_ToggleVisibility(wall, false);
				}
				ObstacleArrays.Add(wall);
			}
			if (!EnumHasAnyFlags(FloorValuesArray[index].ObstacleNeighbours, EIslandFloorNeighbours::MiddleRight))
			{
				wall = GetWorld()->SpawnActor<APackedLevelActor>(startEndWallTemplate, posOffset, FRotator(0.0f, 180.0f, 0.0f));
				if (wall->Implements<UCullingToggleInterface>())
				{
					ICullingToggleInterface::Execute_ToggleVisibility(wall, false);
				}
				ObstacleArrays.Add(wall);
			}
			if (!EnumHasAnyFlags(FloorValuesArray[index].ObstacleNeighbours, EIslandFloorNeighbours::BottomMiddle))
			{
				wall = GetWorld()->SpawnActor<APackedLevelActor>(startEndWallTemplate, posOffset, FRotator(0.0f, 270.0f, 0.0f));
				if (wall->Implements<UCullingToggleInterface>())
				{
					ICullingToggleInterface::Execute_ToggleVisibility(wall, false);
				}
				ObstacleArrays.Add(wall);
			}
		}

		break;
	case EIslandFloorObstacle::End:

		if (startEndWallTemplate != nullptr)
		{
			APackedLevelActor* wall = nullptr;
			if (!EnumHasAnyFlags(FloorValuesArray[index].ObstacleNeighbours, EIslandFloorNeighbours::MiddleLeft))
			{
				wall = GetWorld()->SpawnActor<APackedLevelActor>(startEndWallTemplate, posOffset, FRotator(0.0f, 0.0f, 0.0f));
				if (wall->Implements<UCullingToggleInterface>())
				{
					ICullingToggleInterface::Execute_ToggleVisibility(wall, false);
				}
				ObstacleArrays.Add(wall);
			}
			if (!EnumHasAnyFlags(FloorValuesArray[index].ObstacleNeighbours, EIslandFloorNeighbours::TopMiddle))
			{
				wall = GetWorld()->SpawnActor<APackedLevelActor>(startEndWallTemplate, posOffset, FRotator(0.0f, 90.0f, 0.0f));
				if (wall->Implements<UCullingToggleInterface>())
				{
					ICullingToggleInterface::Execute_ToggleVisibility(wall, false);
				}
				ObstacleArrays.Add(wall);
			}
			if (!EnumHasAnyFlags(FloorValuesArray[index].ObstacleNeighbours, EIslandFloorNeighbours::BottomMiddle))
			{
				wall = GetWorld()->SpawnActor<APackedLevelActor>(startEndWallTemplate, posOffset, FRotator(0.0f, 270.0f, 0.0f));
				if (wall->Implements<UCullingToggleInterface>())
				{
					ICullingToggleInterface::Execute_ToggleVisibility(wall, false);
				}
				ObstacleArrays.Add(wall);
			}
		}

		break;
	case EIslandFloorObstacle::None:
	case EIslandFloorObstacle::PayloadPath:

		if (cornerWallTemplate != nullptr && basicWallTemplate != nullptr)
		{
			if (FloorValuesArray[index].IsFullTile)
			{
				SetWall(index, posOffset, cornerWallTemplate, basicWallTemplate);
			}
		}

		break;
	}
}

void AIslandGenerationTest::CreateFloor()
{
	if (FullFloorPieceTransforms.IsEmpty())
		return;

	UInstancedStaticMeshComponent* ISMComp = NewObject<UInstancedStaticMeshComponent>(this, TEXT("IslandFloorComponent"));
	ISMComp->SetIsReplicated(true);
	ISMComp->SetupAttachment(RootComponent);
	ISMComp->SetCollisionObjectType(ECC_GameTraceChannel3);

	ISMComp->SetNetAddressable();
	AddInstanceComponent(ISMComp);

	if (GenerationValuesComponent->FloorMeshes.FullFloorMesh)
	{
		ISMComp->SetStaticMesh(GenerationValuesComponent->FloorMeshes.FullFloorMesh);
	}
	if (GenerationValuesComponent->FloorMeshes.FloorMaterial)
	{
		ISMComp->SetMaterial(0, GenerationValuesComponent->FloorMeshes.FloorMaterial);
	}

	ISMComp->AddInstances(FullFloorPieceTransforms, false, false, true);

	ISMComp->RegisterComponent();

	InstanceMeshComponentArray.Add(ISMComp);

	//if (GenerationValuesComponent->FloorMeshes.FullFloorMesh)
	//{
	//	if (FloorISMComp->GetStaticMesh() != nullptr && FloorISMComp->GetStaticMesh() != GenerationValuesComponent->FloorMeshes.FullFloorMesh)
	//	{
	//		FloorISMComp->SetStaticMesh(GenerationValuesComponent->FloorMeshes.FullFloorMesh);
	//	}
	//}
	//if (GenerationValuesComponent->FloorMeshes.FloorMaterial)
	//{
	//	if (FloorISMComp->GetMaterial(0) != nullptr && FloorISMComp->GetMaterial(0) != GenerationValuesComponent->FloorMeshes.FloorMaterial)
	//	{
	//		FloorISMComp->SetMaterial(0, GenerationValuesComponent->FloorMeshes.FloorMaterial);
	//	}
	//}

	//FloorISMComp->AddInstances(FullFloorPieceTransforms, false, false, true);
	//FloorISMComp->RegisterComponent();
	//FloorISMComp->SetCanEverAffectNavigation(true);

}

void AIslandGenerationTest::IncrementObjective()
{
	CurrentObjectives++;
	FString objectiveName = "Nothing";

	switch (IslandObjective)
	{
	case EIslandObjectiveType::CaptureTheFlag:
		objectiveName = "Flags";
		break;
	case EIslandObjectiveType::MoveThePayload:
		objectiveName = "Payload";
		break;
	case EIslandObjectiveType::Survival:
		objectiveName = "Survival Zones";
		break;
	}

	UE_LOG(LogTemp, Warning, TEXT("Island: %i, Island Objective: %s, Objective Amount: %i"), IslandIndex, *FString(UEnum::GetValueAsString(IslandObjective)), ObjectiveAmount);
	UE_LOG(LogTemp, Warning, TEXT("Island: %i, Actual Objective Amount: %i"), IslandIndex, CurrentObjectives);
}

void AIslandGenerationTest::DecrementObjective()
{
	CurrentObjectives--;
	FString objectiveName = "Nothing";

	switch (IslandObjective)
	{
	case EIslandObjectiveType::CaptureTheFlag:
		objectiveName = "Flags";
		DecrementFlagObjectiveForAll();
		break;
	case EIslandObjectiveType::MoveThePayload:
		objectiveName = "Payload";
		break;
	case EIslandObjectiveType::Survival:
		objectiveName = "Survival Zones";
		break;
	}

	if (CurrentObjectives <= 0)
	{
		if (Bridge)
		{
			Bridge->Activate();
		}

		OnObjectiveIslandObjectiveComplete.Broadcast();
	}
}

void AIslandGenerationTest::UpdateCameraPos()
{
	//Set Camera Position relative to size of map
	if (CameraActor)
	{
		float tileScale = GenerationValuesComponent->IslandValues.TileScale;
		float cameraX = IslandWidth * tileScale * 2 - tileScale;
		float cameraY = IslandDepth * tileScale * 2 - tileScale;
		CameraActor->SetActorLocation(FVector(cameraX + GetActorLocation().X, cameraY + GetActorLocation().Y, 0.0f));
	}

	OnSetCameraChangeBack.Broadcast(IslandIndex);
}

void AIslandGenerationTest::SetNextCameraPos()
{
	OnSetCameraChange.Broadcast(IslandIndex);
	OnSetCameraChangeBack.Broadcast(IslandIndex + 1);
}

void AIslandGenerationTest::OnRep_FloorValuesArrayChange()
{
	for (int y = 0; y < IslandDepth; y++)
	{
		for (int x = 0; x < IslandWidth; x++)
		{
			CheckFloor(x, y, IslandWidth, IslandDepth);
		}
	}
	CreateFloor();
}

void AIslandGenerationTest::OnRep_BridgeSpawned()
{
	if (Bridge)
	{
		Bridge->OnFrontTrigger.AddDynamic(this, &AIslandGenerationTest::UpdateCameraPos);
		Bridge->OnBackTrigger.AddDynamic(this, &AIslandGenerationTest::SetNextCameraPos);
	}
}

void AIslandGenerationTest::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AIslandGenerationTest, FloorValuesArray);
	DOREPLIFETIME(AIslandGenerationTest, IslandWidth);
	DOREPLIFETIME(AIslandGenerationTest, IslandDepth);
	DOREPLIFETIME(AIslandGenerationTest, GenerationValuesComponent);
	DOREPLIFETIME(AIslandGenerationTest, Bridge);
}