/***************************************************************************************
Class Name:  UBTTask_SurroundTarget
Description: This class is used in a beehavoir tree as node that will handle movement with it goal to be surrounding the target without touching them
Date Last Changed:        2025/11/21
Author:      Thomas Johnson
Contributors:
	Description Of Contributions:
****************************************************************************************/

#pragma once


#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "Navigation/PathFollowingComponent.h"
#include "BTTask_SurroundTarget.generated.h"

/*
To Do:
- SphereOverlapActors is expensive, to improve preformance it would be better to know all the enemies targetting the player and use circule math for lack of a better turm, but a component would have to be add to player that would keep track of all enemies targeting in order to properly form the circlue
- Add debug gizmos and set the up to not run only in editor using #if WITH_EDITOR
	- Use a debug bool to turn on and off
	- Use EditCondition/EditConditionHides = "Error Bool" For any debug variables so they only show when needed
*/

class APawn;
class AAIController;
class UBlackboardComponent;

UCLASS()
class RELICRUNNERS_API UBTTask_SurroundTarget : public UBTTaskNode
{
	GENERATED_BODY()
public:
	UPROPERTY(DisplayName = "Separation Distance Between Target", EditAnywhere, BlueprintReadWrite, Category = "Behaviour", meta = (ClampMin = 50, ToolTip = "The distance the the enemy will try and maintain between them and the target."))
	//The distance the the enemy will try and maintain between them and the target
	float SeparationDistanceBetweenTarget = 150.f;
	UPROPERTY(DisplayName = "Separation Distance Between Friendly", EditAnywhere, BlueprintReadWrite, Category = "Behaviour", meta = (ClampMin = 50, ToolTip = "The distance the enemy will try and maintain between them and other friendlies."))
	//The distance the enemy will try and maintain between them and other friendlies
	float SeparationDistanceBetweenFriendly = 100.f;
	UPROPERTY(DisplayName = "Move To AcceptanceRadius", EditAnywhere, BlueprintReadWrite, Category = "Behaviour", AdvancedDisplay, meta = (ClampMin = 1, ToolTip = "The distance the enemy needs to be within the target location to be considered there."))
	//The distance the enemy needs to be within the target location to be considered there
	float MoveToAcceptanceRadius = 10.f;
	UPROPERTY(DisplayName = "Repath Distance", EditAnywhere, BlueprintReadWrite, Category = "Behaviour", AdvancedDisplay, meta = (ClampMin = 1, ToolTip = "If the target has moved this far from the last known location the enemy will recalculate a new location to move to."))
	//If the target has moved this far from the last known location the enemy will recalculate a new location to move to
	float RepathDistance = 50.f;
	UPROPERTY(DisplayName = "Nav Extent", EditAnywhere, BlueprintReadWrite, Category = "Behaviour", AdvancedDisplay, meta = (ClampMin = 75, AllowPreserveRatio, ToolTip = "The extents of the navmesh search box when finding a location to move to."))
	//The extents of the navmesh search box when finding a location to move to
	FVector NavExtent = FVector(150.f, 150.f, 150.f);
	UPROPERTY(DisplayName = "Repath Distance", EditAnywhere, BlueprintReadWrite, Category = "Behaviour", meta = (ToolTip = "Object types considered friendly for separation checks."))
	//Object types considered friendly for separation checks
	TArray<TEnumAsByte<EObjectTypeQuery>> FriendlyObjectTypes;

protected:
	UPROPERTY(DisplayName = "Target Actor Key", EditAnywhere, BlueprintReadWrite, Category = "Blackboard", meta = (ToolTip = "For the actor the enemy is trying to surround/target."))
	//for the actor the enemy is trying to surround/target
	FBlackboardKeySelector TargetActorKey; 

	//to keep track of the last location of the target when calculating if a new path is needed
	FVector LastTargetLocation;
	//to know if the path was changed so we don't end the task in OnFinishedMoving when the delegate is called due to a path change
	bool pathWasChanged = false;

	//references related to the enemy using this task
	APawn* ControlledPawn;
	AAIController* OwnerController;
	UBlackboardComponent* BlackBoard;
	UBehaviorTreeComponent* OwnerComponent;

	//the target actor being surrounded
	AActor* Target;

protected:
	//the constructor
	UBTTask_SurroundTarget();
	//Set up the rest of the task and start moving the enemy
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	//incase the node get abourt for what ever resone the delegate need to be cleared
	EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	//this is here so we can check if we need to update the move to location
	void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Finish Task", ToolTip = "Meant to be used instead of FinishLatentTask() so that the delegate can be cleared properly.", ShortToolTip = "Call instead of FinishLatentTask()."))
	//Meant to be used instead of FinishLatentTask() so that the delegate can be cleared properly
	void FinishTask(bool result);
	UFUNCTION()
	//to be called once the move order from OnSeparationLocationReached is done
	void OnFinishedMoving(FAIRequestID RequestID, EPathFollowingResult::Type Result);
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Find Surround Location", ToolTip = "Find the closet valid location to move to that maintains separation from both the target and other friendlies.", ShortToolTip = "Find a suitable location for the owner between the owner and the target."))
	//Find the closet valid location to move to that maintains separation from both the target and other friendlies
	bool FindSurroundLocation(FVector& result);
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Set Up Pawn Move To", ToolTip = "Calls the move to function on the owner controller to start moving to the surround location provided by FindSurroundLocation.", ShortToolTip = "Start moving the owner towards the target to surround it."))
	//Calls the move to function on the owner controller to start moving to the surround location provided by FindSurroundLocation
	EPathFollowingRequestResult::Type SetUpPawnMoveTo();
};
