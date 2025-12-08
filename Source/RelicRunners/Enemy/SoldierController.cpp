// Fill out your copyright notice in the Description page of Project Settings.


#include "SoldierController.h"
#include "NavigationSystem.h"
#include "Perception/AIPerceptionComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "RelicRunners/Director System/Director.h"
#include "TormentedSoulCharacter.h"

ASoldierController::ASoldierController()
{
	//set up default values
	PatrolRadius = 1000.f;
	IsInCombat = false;
	OriginPatrolZone = FVector::ZeroVector;
	CurrentPatrolCenter = FVector::ZeroVector;
	AIPerceptionComp = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComp"));

	for (EAILOD lod : TEnumRange<EAILOD>())
	{
		DistanceOfLOD.Add(lod, 10);
	}
}

void ASoldierController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

//It is here and not just in header in chase more logic is add later 
bool ASoldierController::GetIsInCombat() const
{
	return IsInCombat;
}

bool ASoldierController::IsInPatrolRange()
{
	//null check then calculate and check distances
	if (GetPawn())
	{
		float Distance = FVector::Dist(GetPawn()->GetActorLocation(), OriginPatrolZone);
		if (Distance <= PatrolRadius)
		{
			return true;
		}
	}

	return false;
}

FVector ASoldierController::GetRandomPatrolPoint()
{
	//set up variables related to the nav mesh
	FNavLocation randomPoint;
	UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());

	//finding the point
	if (NavSys->GetRandomPointInNavigableRadius(OriginPatrolZone, PatrolRadius, randomPoint))
	{
		return randomPoint.Location;
	}

//error log not need if not playtest build
#if WITH_EDITOR || UE_BUILD_TEST
	UE_LOG(LogTemp, Warning, TEXT("could not find random spawn point on navmesh"));
#endif

	return FVector::ZeroVector;
}

void ASoldierController::LODCheck()
{
	TArray<APawn*> players;
	APawn* pawn = GetPawn();

	if (ADirector* dir = ADirector::GetInstance(this))
	{
		players = dir->GetPlayers();
	}
	else
	{
		return;
	}

	float closeDistance = FVector::Dist(players[0]->GetActorLocation(), pawn->GetActorLocation());

	for (int i = 1; i < players.Num(); i++)
	{
		float newDistance = FVector::Dist(players[i]->GetActorLocation(), pawn->GetActorLocation());

		if (closeDistance > newDistance)
		{
			closeDistance = newDistance;
		}
	}
	
	if (closeDistance <= DistanceOfLOD[EAILOD::L1])
	{
		RunBehaviorTree(StartingBehaviorTree);
		CurrentLOD = EAILOD::L1;

		if (AIPerceptionComp)
		{
			AIPerceptionComp->Activate();
		}
	}
	/*else if (closeDistance <= DistanceOfLOD[EAILOD::L2])
	{
		

		CurrentLOD = EAILOD::L2;
	}
	else if (closeDistance <= DistanceOfLOD[EAILOD::L3])
	{
		CurrentLOD = EAILOD::L3;
	}
	else if (closeDistance <= DistanceOfLOD[EAILOD::L4])
	{
		CurrentLOD = EAILOD::L4;
	}*/
	else
	{
		UBehaviorTreeComponent* behaviorComp = Cast<UBehaviorTreeComponent>(BrainComponent);
		if (behaviorComp)
		{
			behaviorComp->StopTree(EBTStopMode::Safe);
		}

		if (AIPerceptionComp)
		{
			AIPerceptionComp->Deactivate();
		}

		SetActorTickEnabled(false);
		pawn->SetActorTickEnabled(false);
		pawn->PrimaryActorTick.bCanEverTick = false;

		for (UActorComponent* comp : pawn->GetComponents())
		{
			comp->PrimaryComponentTick.bCanEverTick = false;
			comp->SetComponentTickEnabled(false);
		}

		StopMovement();

		if (USkeletalMeshComponent* skel = Cast<ACharacter>(pawn)->GetMesh())
		{
			skel->GlobalAnimRateScale = 0.f;
			skel->bPauseAnims = true;
			skel->SetComponentTickEnabled(false);
			skel->SetSimulatePhysics(false);
			skel->SetVisibility(false, true);
		}

		CurrentLOD = EAILOD::L5;
	}
}

void ASoldierController::BeginPlay()
{
	Super::BeginPlay();
	APawn* pawn = GetPawn();

	//check pawn and then set up the vectors for patrol
	if (pawn)
	{
		OriginPatrolZone = pawn->GetActorLocation();
		CurrentPatrolCenter = OriginPatrolZone;
	}
}

void ASoldierController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (StartingBehaviorTree)
	{
		RunBehaviorTree(StartingBehaviorTree);
	}
}

bool ASoldierController::GetIsActive()
{
	return IsActive;
}

bool ASoldierController::GetShouldTick()
{
	return true;
}

void ASoldierController::SetIsActive(bool newIsActive)
{
	if (IsActive == newIsActive)
	{
		return;
	}

	IsActive = newIsActive;
	IPoolActorInterface::SetIsActive(newIsActive);
	UBehaviorTreeComponent* behaviorComp = Cast<UBehaviorTreeComponent>(BrainComponent);
	if (!IsActive)
	{
		//reset variables
		IsInCombat = false;
		OriginPatrolZone = FVector::ZeroVector;
		CurrentPatrolCenter = FVector::ZeroVector;
		//ClearFocus(EAIFocusPriority::Gameplay);

		if (behaviorComp)
		{
			behaviorComp->StopTree(EBTStopMode::Safe);
		}
	}
	else if (StartingBehaviorTree && CurrentLOD == EAILOD::L1)
	{
		RunBehaviorTree(StartingBehaviorTree);
	}
}

FTimerHandle& ASoldierController::GetLifeSpanTimerHandle()
{
	return LifeSpanTimerHandle;
}
