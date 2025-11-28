// Fill out your copyright notice in the Description page of Project Settings.

#include "BTTask_SurroundTarget.h"
#include "AIController.h"
#include "NavigationSystem.h"
#include "Kismet/KismetSystemLibrary.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Character.h"

UBTTask_SurroundTarget::UBTTask_SurroundTarget()
{
	//this make so their is a instance of this node for the behavoir tree. This make it safer to bind the delgates to this node for a few reason but with out it unreal occisually crash when binding delegate. This most likely because node can be share amoung multiple AIs
	bCreateNodeInstance = true;
	bNotifyTick = true;
}

EBTNodeResult::Type UBTTask_SurroundTarget::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	//getting and checking all references to the owner that are needed
	OwnerComponent = &OwnerComp;

	OwnerController = OwnerComp.GetAIOwner(); 
	BlackBoard = OwnerComp.GetBlackboardComponent();

	if (!OwnerController || !BlackBoard)
	{
		return EBTNodeResult::Failed;
	}

	ControlledPawn = OwnerController->GetPawn();

	if (!ControlledPawn)
	{
		return EBTNodeResult::Failed;
	}

	//getting and checking the target actor
	Target = Cast<AActor>(BlackBoard->GetValueAsObject(TargetActorKey.SelectedKeyName));

	if (!Target)
	{
		return EBTNodeResult::Failed;
	}
	//This may seem like a reduntaded check but I already did this to my self once and on the first test non the less
	else if (Target == Cast<AActor>(ControlledPawn))
	{
		UE_LOG(LogTemp, Warning, TEXT("Target actor is the same as controlled pawn in surrounding target task"));
		return EBTNodeResult::Failed;
	}
	

	//Set up last target location for later use in tick
	LastTargetLocation = Target->GetActorLocation();

	//stop any movement before setting up new movement
	OwnerController->StopMovement();

	//call set up pawn move to, to start moving to surround target and check to make sure it was a success
	if (SetUpPawnMoveTo() == EPathFollowingRequestResult::RequestSuccessful)
	{
		//bind the delegate so we know when the move is complete
		OwnerController->ReceiveMoveCompleted.AddDynamic(this, &UBTTask_SurroundTarget::OnFinishedMoving);
		return EBTNodeResult::InProgress;
	}

	return EBTNodeResult::Failed;
}

void UBTTask_SurroundTarget::OnFinishedMoving(FAIRequestID RequestID, EPathFollowingResult::Type Result)
{
	//check to see if we are finished moving to the surround location or if we need to recalculate a new location
	if (Result == EPathFollowingResult::Success)
	{
		FinishTask(true);
	}
	else if(Result == EPathFollowingResult::Aborted && !pathWasChanged)
	{
		FinishTask(false);
	}
	else
	{
		pathWasChanged = false;
	}
}

bool UBTTask_SurroundTarget::FindSurroundLocation(FVector& result)
{
	//checking all needed references
	if (!OwnerController || !BlackBoard || !ControlledPawn)
	{
		return false;
	}

	//finding the point to move to 
	FVector targetLocation = Target->GetActorLocation() + ((ControlledPawn->GetActorLocation() - Target->GetActorLocation()).GetSafeNormal() * SeparationDistanceBetweenTarget);
	FVector worldMoveLocation = targetLocation;

	//set up for sphere overlap
	TArray<AActor*> overlappingActors;
	TArray<AActor*> ignoredActors;
	ignoredActors.Add(ControlledPawn);
	ignoredActors.Add(Target);

	//check to see if their are other friendly actors within the separation distance at the target location
	bool sphereResult = UKismetSystemLibrary::SphereOverlapActors(GetWorld(), targetLocation, SeparationDistanceBetweenFriendly, FriendlyObjectTypes, nullptr, ignoredActors, overlappingActors);

	//if their are other friendly actors within the separation distance adjust the move to location
	if (sphereResult || overlappingActors.Num() > 0)
	{
		//using a weighted average to find the direction to move away from other enemies with the weight being the inverse of the distance to the other enemy so that closer enemies have more influence
		float totalWeights = 0.f;
		FVector moveDirection = FVector::ZeroVector;
		float closestDistance = TNumericLimits<float>::Max();

		//loop through all other actors and find the direction
		for (AActor* actor : overlappingActors)
		{
			//make sure we are only considering enemies
			if (actor->Tags.Contains("Enemy") || actor->Tags.Contains("Player"))
			{

				//check the distance to make sure their is nothing funcky going on
				float distance = FVector::Dist(actor->GetActorLocation(), targetLocation);

				if (distance > 0.f)
				{
					//calculate weight and direction away from the other enemy
					float weight = 1.f / distance;
					FVector direction = (ControlledPawn->GetActorLocation() - actor->GetActorLocation()).GetSafeNormal();

					//add everything up
					moveDirection += direction * weight;
					totalWeights += weight;

					//check for closest distance for later use
					closestDistance = FMath::Min(closestDistance, distance);
				}
			}
		}

		//make sure move direction is valid then calculate the new world move location
		if (!moveDirection.IsNearlyZero())
		{
			moveDirection /= totalWeights;
			worldMoveLocation = (moveDirection.GetSafeNormal() * FMath::Max(0, (SeparationDistanceBetweenFriendly - closestDistance))) + ControlledPawn->GetActorLocation();
		}
	}

	//make sure we update the result
	result = worldMoveLocation;

	return true;
}

EPathFollowingRequestResult::Type UBTTask_SurroundTarget::SetUpPawnMoveTo()
{
	//set up all need varialbes for moving the pawn
	UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
	FVector worldMoveLocation;
	FNavLocation moveLocation;
	FAIMoveRequest requestLocation;
	FNavPathSharedPtr navPath;

	//find the location to move to and the check to make sure NavSys is valid and the location is on the navmesh
	if (FindSurroundLocation(worldMoveLocation) && NavSys && !NavSys->ProjectPointToNavigation(worldMoveLocation, moveLocation, NavExtent))
	{
		return EPathFollowingRequestResult::Type::Failed;
	}

	//set up the request
	requestLocation.SetGoalLocation(moveLocation);
	requestLocation.SetAcceptanceRadius(MoveToAcceptanceRadius);

	//start the move to
	return OwnerController->MoveTo(requestLocation, &navPath);;
}

EBTNodeResult::Type UBTTask_SurroundTarget::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	//make sure we owner controller is still a thing and then banish the delegate to oblivion
	if (OwnerController)
	{
		OwnerController->ReceiveMoveCompleted.RemoveDynamic(this, &UBTTask_SurroundTarget::OnFinishedMoving);
	}
	
	return EBTNodeResult::Aborted;
}

void UBTTask_SurroundTarget::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	//UE_LOG(LogTemp, Error, TEXT("Should tick"));

	//checking all needed references
	if (!OwnerController || !BlackBoard || !ControlledPawn || !Target)
	{
		FinishTask(false);
		return;
	}

	//check to see if the target has moved enough to recalculate a new location to move to
	if (FVector::Dist(LastTargetLocation, Target->GetActorLocation()) > RepathDistance)
	{
		LastTargetLocation = Target->GetActorLocation();

		//stop current movement before setting up a new one
		OwnerController->StopMovement();

		//set up the new move to and check to make sure it was a success
		if (SetUpPawnMoveTo() != EPathFollowingRequestResult::RequestSuccessful)
		{
			FinishTask(false);
			return;
		}

		//make sure we know the path was changed so we don't end the task in OnFinishedMoving
		pathWasChanged = true;
	}
}

void UBTTask_SurroundTarget::FinishTask(bool result)
{
	//banish the delegate to Florida
	if (OwnerController)
	{
		OwnerController->ReceiveMoveCompleted.RemoveDynamic(this, &UBTTask_SurroundTarget::OnFinishedMoving);
	}

	//finish the task with the given result
	if (result)
	{
		FinishLatentTask(*OwnerComponent, EBTNodeResult::Succeeded);
	}
	else
	{
		FinishLatentTask(*OwnerComponent, EBTNodeResult::Failed);
	}
}
