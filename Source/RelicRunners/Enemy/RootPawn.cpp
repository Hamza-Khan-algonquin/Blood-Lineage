// Fill out your copyright notice in the Description page of Project Settings.


#include "RootPawn.h"

// Sets default values
ARootPawn::ARootPawn()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ARootPawn::BeginPlay()
{
	Super::BeginPlay();
	

	//SetActorLocation(GetActorLocation() + FVector(0, 0, 100));
}

// Called every frame
void ARootPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (GetActorLocation().Z > 200)
	{
		Destroy();
	}
}

// Called to bind functionality to input
void ARootPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

