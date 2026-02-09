// Fill out your copyright notice in the Description page of Project Settings.


#include "NewCapturableFlag.h"
#include "Components/SceneComponent.h"
#include "Components/BoxComponent.h"
#include "Components/TextRenderComponent.h"
#include "RelicRunners/RelicRunnersGameState.h"
#include "RelicRunners/Game/RelicRunnersGameInstance.h"
#include "Components/StaticMeshComponent.h"
#include <Net/UnrealNetwork.h>

ANewCapturableFlag::ANewCapturableFlag()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	TriggerBox = CreateDefaultSubobject<UBoxComponent>("TriggerCollision");
	TriggerBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TriggerBox->SetCollisionProfileName("BlockAll");
	TriggerBox->SetGenerateOverlapEvents(true);
	TriggerBox->SetupAttachment(Origin);

	TextComponent = CreateDefaultSubobject<UTextRenderComponent>("TextComponent");
	TextComponent->SetupAttachment(Origin);
	TextComponent->SetText(FText::FromString("Press E to Interact"));

	FlagMesh = CreateDefaultSubobject<USkeletalMeshComponent>("FlagMesh");
	FlagMesh->SetupAttachment(Origin);

	bReplicates = true;
	bAlwaysRelevant = true;
}

void ANewCapturableFlag::Interact_Implementation(ARelicRunnersCharacter* Char)
{
}

void ANewCapturableFlag::Server_HandleInteracted_Implementation(ARelicRunnersCharacter* Char)
{
	if (HasAuthority() && Char)
	{
		HandleInteracted(Char);
	}
}

void ANewCapturableFlag::HandleInteracted(ARelicRunnersCharacter* Char)
{
	if (!Char)
		return;

	//Disable further interactions
	isActive = true;

	TextComponent->SetText(FText::FromString("Complete!"));

	FString text = HasAuthority() ? "Server: " : "Client: ";
	GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Yellow, text + FString::Printf(TEXT("Interacted with Flag")));

	BeaconMesh->SetVisibility(false);
	OnObjectiveActivated.Broadcast();

	if (FlagMesh && ActivateAnimation)
	{
		FlagMesh->PlayAnimation(ActivateAnimation, false);
	}

}

void ANewCapturableFlag::OnActiveChange()
{
	isActive = true;

	TextComponent->SetText(FText::FromString("Complete!"));

	FString text = HasAuthority() ? "Server: " : "Client: ";
	GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Yellow, text + FString::Printf(TEXT("Interacted with Flag")));

	BeaconMesh->SetVisibility(false);
	OnObjectiveActivated.Broadcast();

	if (FlagMesh && ActivateAnimation)
	{
		FlagMesh->PlayAnimation(ActivateAnimation, false);
	}

}

void ANewCapturableFlag::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ANewCapturableFlag, isActive);
}