// Fill out your copyright notice in the Description page of Project Settings.


#include "PayloadActor.h"
#include "Components/SplineComponent.h"
#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "RelicRunners/RelicRunnersGameState.h"
#include "RelicRunners/Game/RelicRunnersGameInstance.h"
#include <Net/UnrealNetwork.h>

// Sets default values
APayloadActor::APayloadActor() :
	DeliverySpline(nullptr),
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

	Origin = CreateDefaultSubobject<USceneComponent>("Origin");
	RootComponent = Origin;

	PayloadBody = CreateDefaultSubobject<UBoxComponent>("PayloadBody");
	PayloadBody->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	PayloadBody->SetCollisionProfileName("BlockAllDynamic");
	PayloadBody->SetGenerateOverlapEvents(true);
	PayloadBody->SetupAttachment(Origin);

	TriggerCollision = CreateDefaultSubobject<USphereComponent>("TriggerCollision");
	TriggerCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TriggerCollision->SetCollisionProfileName("OverlapAll");
	TriggerCollision->SetGenerateOverlapEvents(true);
	TriggerCollision->OnComponentBeginOverlap.AddDynamic(this, &APayloadActor::OnTriggerOverlap);
	TriggerCollision->OnComponentEndOverlap.AddDynamic(this, &APayloadActor::OnOverlapEnd);
	TriggerCollision->SetupAttachment(Origin);

	bReplicates = true;
	bAlwaysRelevant = true;
	SetReplicateMovement(true);
}

// Called when the game starts or when spawned
void APayloadActor::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void APayloadActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (DeliverySpline)
	{
		if (CollidingActors.IsEmpty())
		{
			CurrentSpeed = FMath::FInterpTo(CurrentSpeed, TargetSpeed, DeltaTime, Deceleration);
		}
		else
		{
			CurrentSpeed = FMath::FInterpTo(CurrentSpeed, TargetSpeed, DeltaTime, Acceleration);
		}

		Distance = FMath::FInterpConstantTo(Distance, DeliverySpline->GetSplineLength(), DeltaTime, CurrentSpeed);

		SetActorTransform(DeliverySpline->GetTransformAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World));

		if (Distance >= DeliverySpline->GetSplineLength() && !HasReachedDestination)
		{
			HasReachedDestination = true;
			GEngine->AddOnScreenDebugMessage(-1, DeltaTime, FColor::Yellow, FString::Printf(TEXT("Payload has reached destination")));
			TriggerCollision->SetGenerateOverlapEvents(false);
			PrimaryActorTick.bCanEverTick = false;
		}
	}
}

void APayloadActor::Multicast_Initialize_Implementation(USplineComponent* newSpline, float newSpeed)
{
	DeliverySpline = newSpline;
	MaxSpeed = newSpeed;
}

void APayloadActor::OnTriggerOverlap(UPrimitiveComponent* OverlapComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor->ActorHasTag("Player") && HasAuthority())
	{
		Multicast_OnTriggerChange(OtherActor, true);
	}
}

void APayloadActor::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor->ActorHasTag("Player") && HasAuthority())
	{
		AActor* actorToRemove = nullptr;
		for (AActor* actor: CollidingActors)
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

void APayloadActor::Multicast_OnTriggerChange_Implementation(AActor* OtherActor, bool OnOverlapBegin)
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

void APayloadActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APayloadActor, Distance);
	DOREPLIFETIME(APayloadActor, Acceleration);
	DOREPLIFETIME(APayloadActor, Deceleration);
	DOREPLIFETIME(APayloadActor, MaxSpeed);
	DOREPLIFETIME(APayloadActor, TargetSpeed);
	DOREPLIFETIME(APayloadActor, CurrentSpeed);
	DOREPLIFETIME(APayloadActor, IsActive);
	DOREPLIFETIME(APayloadActor, HasReachedDestination);
	DOREPLIFETIME(APayloadActor, CollidingActors);
}