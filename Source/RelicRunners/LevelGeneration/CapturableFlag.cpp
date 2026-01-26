// Fill out your copyright notice in the Description page of Project Settings.


#include "CapturableFlag.h"
#include "Components/SceneComponent.h"
#include "Components/BoxComponent.h"
#include "Components/TextRenderComponent.h"
#include "RelicRunners/RelicRunnersGameState.h"
#include "RelicRunners/Game/RelicRunnersGameInstance.h"
#include "Components/StaticMeshComponent.h"
#include <Net/UnrealNetwork.h>

// Sets default values
ACapturableFlag::ACapturableFlag():
	Origin(nullptr),
	TriggerBox(nullptr),
	TextComponent(nullptr)
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Origin = CreateDefaultSubobject<USceneComponent>("Origin");
	RootComponent = Origin;

	TriggerBox = CreateDefaultSubobject<UBoxComponent>("TriggerCollision");
	TriggerBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TriggerBox->SetCollisionProfileName("BlockAll");
	TriggerBox->SetGenerateOverlapEvents(true);
	TriggerBox->SetupAttachment(Origin);

	TextComponent = CreateDefaultSubobject<UTextRenderComponent>("TextComponent");
	TextComponent->SetupAttachment(Origin);
	TextComponent->SetText(FText::FromString("Press E to Interact"));

	BeaconMesh = CreateDefaultSubobject<UStaticMeshComponent>("Beacon");
	BeaconMesh->SetCollisionProfileName("NoCollision");
	BeaconMesh->SetupAttachment(Origin);

	bReplicates = true;
	bAlwaysRelevant = true;
}

// Called when the game starts or when spawned
void ACapturableFlag::BeginPlay()
{
	Super::BeginPlay();

	ARelicRunnersGameState* gameState = Cast<ARelicRunnersGameState>(GetWorld()->GetGameState());

	if (gameState)
	{
		//Increment the Replicated Objective variable in the Gamestate for every flag that is spawned
		gameState->Server_IncrementObjective(EObjectiveType::CaptureTheFlag);
	}	
}

// Called every frame
void ACapturableFlag::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ACapturableFlag::Interact_Implementation(ARelicRunnersCharacter* Char)
{
}

void ACapturableFlag::Server_HandleInteracted_Implementation(ARelicRunnersCharacter* Char)
{
	if (HasAuthority() && Char)
	{
		HandleInteracted(Char);
	}
}

void ACapturableFlag::HandleInteracted(ARelicRunnersCharacter* Char)
{
	if (!Char || isActive)
		return;

	ARelicRunnersGameState* gameState = Cast<ARelicRunnersGameState>(GetWorld()->GetGameState());

	if (gameState)
	{
		//Decrement the Replicated Objective variable in the Gamestate
		gameState->Server_DecrementObjective(EObjectiveType::CaptureTheFlag);
	}

	//Disable further interactions
	isActive = true;

	TextComponent->SetText(FText::FromString("Complete!"));

	FString text = HasAuthority() ? "Server: " : "Client: ";
	GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Yellow, text + FString::Printf(TEXT("Interacted with Flag")));

	BeaconMesh->SetVisibility(false);
	OnFlagObjectiveActivated.Broadcast();
}

void ACapturableFlag::Activate()
{
	OnFlagObjectiveCreated.Broadcast();
}

void ACapturableFlag::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ACapturableFlag, isActive);
}