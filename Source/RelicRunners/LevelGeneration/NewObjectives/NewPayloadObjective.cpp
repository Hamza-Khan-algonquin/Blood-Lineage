// Fill out your copyright notice in the Description page of Project Settings.


#include "NewPayloadObjective.h"
#include "Components/SplineComponent.h"
#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "RelicRunners/RelicRunnersGameState.h"
#include "RelicRunners/Game/RelicRunnersGameInstance.h"
#include <Net/UnrealNetwork.h>
#include <RelicRunners/PlayerController/RelicRunnersPlayerController.h>

// Sets default values
ANewPayloadObjective::ANewPayloadObjective() :
	Distance(0),
	Acceleration(0),
	Deceleration(0),
	MaxSpeed(0),
	TargetSpeed(0),
	CurrentSpeed(0),
	IsActive(false),
	HasReachedDestination(false)
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	PayloadOrigin = CreateDefaultSubobject<USceneComponent>("PayloadOrigin");
	PayloadOrigin->SetupAttachment(Origin);
	PayloadOrigin->SetIsReplicated(true);

	PayloadBody = CreateDefaultSubobject<UStaticMeshComponent>("PayloadBody");
	PayloadBody->SetupAttachment(PayloadOrigin);

	RadiusMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RadiusMesh"));
	RadiusMesh->SetCollisionProfileName("NoCollision");
	RadiusMesh->SetupAttachment(PayloadOrigin);

	PayloadBox = CreateDefaultSubobject<UBoxComponent>("PayloadBox");
	PayloadBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	PayloadBox->SetCollisionProfileName("BlockAllDynamic");
	PayloadBox->SetGenerateOverlapEvents(true);
	PayloadBox->SetupAttachment(PayloadOrigin);

	TriggerCollision = CreateDefaultSubobject<USphereComponent>("TriggerCollision");
	TriggerCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TriggerCollision->SetCollisionProfileName("OverlapAll");
	TriggerCollision->SetGenerateOverlapEvents(true);
	TriggerCollision->OnComponentBeginOverlap.AddDynamic(this, &ANewPayloadObjective::OnTriggerOverlap);
	TriggerCollision->OnComponentEndOverlap.AddDynamic(this, &ANewPayloadObjective::OnOverlapEnd);
	TriggerCollision->SetupAttachment(PayloadOrigin);

	BeaconMesh->SetupAttachment(PayloadOrigin);

	InterpDelay = 0.1f;
	MaxClientLocationDeviation = 200.f;
	AvgReplicationDelta = 0.1f;

	bReplicates = true;
	bAlwaysRelevant = true;
	//SetReplicateMovement(true);
}

// Called when the game starts or when spawned
void ANewPayloadObjective::BeginPlay()
{
	Super::BeginPlay();

	UMaterialInterface* OutlineOverlay = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Shader/M_Outline"));

	if (OutlineOverlay)
	{
		PayloadBody->SetOverlayMaterial(OutlineOverlay);
		PayloadBody->SetForceDisableNanite(true);
		UE_LOG(LogTemp, Warning, TEXT("DropOutlineApplied"));
	}

}

void ANewPayloadObjective::UpdateObjectiveToAllControllers(float NewPercentage)
{
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		ARelicRunnersPlayerController* TargetPC = Cast<ARelicRunnersPlayerController>(It->Get());
		if (TargetPC)
		{
			TargetPC->UpdatePayloadObjective(NewPercentage);
		}
	}
}

// Called every frame
void ANewPayloadObjective::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!HasAuthority())
	{
		ClientMovementInterpolation(DeltaTime);
		return;
	}

	if (SplinePath)
	{

		if (CollidingActors.IsEmpty())
		{
			CurrentSpeed = FMath::FInterpTo(CurrentSpeed, TargetSpeed, DeltaTime, Deceleration);
		}
		else
		{
			CurrentSpeed = FMath::FInterpTo(CurrentSpeed, TargetSpeed, DeltaTime, Acceleration);
		}

		Distance = FMath::FInterpConstantTo(Distance, SplinePath->GetSplineLength(), DeltaTime, CurrentSpeed);

		float TotalLength = SplinePath->GetSplineLength();
		float NewPercentage = (TotalLength > 0.0f) ? FMath::Clamp(Distance / TotalLength, 0.0f, 100.0f) : 0.0f;

		//Update UI
		if (!FMath::IsNearlyEqual(NewPercentage, CompletionPercentage, 0.001f))
		{
			CompletionPercentage = NewPercentage;
			UpdateObjectiveToAllControllers(CompletionPercentage);
		}
		FTransform newTransform = SplinePath->GetTransformAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World);
		SetActorTransform(newTransform);
		ServerMoveData = FPayloadMoveData(newTransform.GetLocation(), FRotator(newTransform.GetRotation()), 0.0f);

		if (Distance >= SplinePath->GetSplineLength() && !HasReachedDestination)
		{
			HasReachedDestination = true;
			TriggerCollision->SetGenerateOverlapEvents(false);
			PrimaryActorTick.bCanEverTick = false;

			BeaconMesh->SetVisibility(false);
			RadiusMesh->SetVisibility(false);

			OnObjectiveActivated.Broadcast();
		}
	}
}

void ANewPayloadObjective::Initialize(float maxSpeed, float acceleration, float deceleration, class USplineComponent* splinePath)
{
	MaxSpeed = maxSpeed;
	Acceleration = acceleration;
	Deceleration = deceleration;
	SplinePath = splinePath;
}

void ANewPayloadObjective::Multicast_Initialize_Implementation(float newSpeed, class USplineComponent* newSplinePath)
{
	MaxSpeed = newSpeed;
	SplinePath = newSplinePath;
}

void ANewPayloadObjective::OnTriggerOverlap(UPrimitiveComponent* OverlapComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor->ActorHasTag("Player") && HasAuthority())
	{
		Multicast_OnTriggerChange(OtherActor, true);
	}
}

void ANewPayloadObjective::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor->ActorHasTag("Player") && HasAuthority())
	{
		AActor* actorToRemove = nullptr;
		for (AActor* actor : CollidingActors)
		{
			if (actor == OtherActor)
			{
				actorToRemove = OtherActor;
			}
		}

		if (actorToRemove)
		{
			Multicast_OnTriggerChange(actorToRemove, false);
		}
	}
}

void ANewPayloadObjective::OnRep_HasReachedDestination()
{
	BeaconMesh->SetVisibility(false);
	RadiusMesh->SetVisibility(false);
}

void ANewPayloadObjective::Multicast_OnTriggerChange_Implementation(AActor* OtherActor, bool OnOverlapBegin)
{
	if (OnOverlapBegin)
	{
		CollidingActors.AddUnique(OtherActor);
		TargetSpeed = MaxSpeed;
	}
	else
	{
		CollidingActors.Remove(OtherActor);

		if (CollidingActors.IsEmpty())
		{
			TargetSpeed = 0.0f;
		}
	}
}

void ANewPayloadObjective::OnRep_ServerPayloadMoveData()
{
	ServerMoveData.TimeStamp = GetWorld()->GetTimeSeconds();

	MoveDataBuffer.Add(ServerMoveData);

	if (MoveDataBuffer.Num() > 6)
	{
		MoveDataBuffer.RemoveAt(0);
	}
}

void ANewPayloadObjective::ClientMovementInterpolation(float delta)
{
	if (MoveDataBuffer.Num() < 2)
	{
		return;
	}

	float clientDelta = MoveDataBuffer.Last().TimeStamp - MoveDataBuffer[MoveDataBuffer.Num() - 2].TimeStamp;

	AvgReplicationDelta = FMath::FInterpTo(AvgReplicationDelta, clientDelta, delta, 1.0f);

	TargetInterpDelay = FMath::Clamp(AvgReplicationDelta * 1.5f, 0.05f, 0.5f);

	InterpDelay = FMath::FInterpTo(InterpDelay, TargetInterpDelay, delta, 5.0f);

	float clientRenderTime = GetWorld()->GetTimeSeconds() - (InterpDelay);

	//loop through the move data buffer and find the two move data that the client render time is in between
	for (int i = 0; i < MoveDataBuffer.Num() - 1; i++)
	{
		if (MoveDataBuffer[i].TimeStamp <= clientRenderTime && MoveDataBuffer[i + 1].TimeStamp >= clientRenderTime)
		{
			float denominator = (MoveDataBuffer[i + 1].TimeStamp - MoveDataBuffer[i].TimeStamp);

			if (denominator < KINDA_SMALL_NUMBER)
			{
				continue;
			}

			float interpAlpha = (clientRenderTime - MoveDataBuffer[i].TimeStamp) / denominator;

			//lerp!!!!!!!!!!!!!!!!!!!!
			FVector interpLocation = FMath::Lerp(MoveDataBuffer[i].Location, MoveDataBuffer[i + 1].Location, interpAlpha);
			FRotator interpRotation = FMath::Lerp(MoveDataBuffer[i].Rotation, MoveDataBuffer[i + 1].Rotation, interpAlpha);

			//check if the interpolation is too far from the last move data to prevent interpolation from going crazy when there is a big delay between the server and client
			if (FVector::Dist(interpLocation, MoveDataBuffer.Last().Location) > MaxClientLocationDeviation)
			{
				SetActorLocation(MoveDataBuffer.Last().Location);
				SetActorRotation(interpRotation);
				//clear so we don't get rubber banding effect
				MoveDataBuffer.Empty();
			}
			else
			{
				SetActorLocation(interpLocation);
				SetActorRotation(interpRotation);
			}

			return;
		}
	}

	if (clientRenderTime > MoveDataBuffer.Last().TimeStamp)
	{
		SetActorLocation(MoveDataBuffer.Last().Location);
		SetActorRotation(MoveDataBuffer.Last().Rotation);
	}
}

void ANewPayloadObjective::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ANewPayloadObjective, Distance);
	DOREPLIFETIME(ANewPayloadObjective, Acceleration);
	DOREPLIFETIME(ANewPayloadObjective, Deceleration);
	DOREPLIFETIME(ANewPayloadObjective, MaxSpeed);
	DOREPLIFETIME(ANewPayloadObjective, TargetSpeed);
	DOREPLIFETIME(ANewPayloadObjective, CurrentSpeed);
	DOREPLIFETIME(ANewPayloadObjective, IsActive);
	DOREPLIFETIME(ANewPayloadObjective, CompletionPercentage);
	DOREPLIFETIME(ANewPayloadObjective, HasReachedDestination);
	DOREPLIFETIME(ANewPayloadObjective, CollidingActors);
}