// Fill out your copyright notice in the Description page of Project Settings.


#include "ForgeActor.h"
#include <RelicRunners/PlayerController/RelicRunnersPlayerController.h>
#include "RelicRunners/Inventory/InventoryComponent.h"
#include "RelicRunners/RelicRunnersCharacter.h"
#include "RelicRunners/Item/ItemStats.h"
#include "RelicRunners/Item/ItemData.h"
#include "RelicRunners/Inventory/Inventory.h"
#include "Components/CanvasPanel.h"
#include "Components/CapsuleComponent.h"

// Sets default values
AForgeActor::AForgeActor()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	if (UCapsuleComponent* Capsule = GetCapsuleComponent())
	{
		Capsule->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		Capsule->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	}

	if (GetMesh())
	{
		GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		GetMesh()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	}
}

// Called when the game starts or when spawned
void AForgeActor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AForgeActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

FItemData AForgeActor::GetItemData_Implementation()
{
	return FItemData();
}

void AForgeActor::ShowTooltip_Implementation(bool bShow)
{

}

void AForgeActor::Interact_Implementation(ARelicRunnersCharacter* Char)
{
	if (!Char) return;

	ARelicRunnersPlayerController* PC = Char->GetController<ARelicRunnersPlayerController>();
	if (!PC) return;

	LinkedInventoryWidget = PC->GetInventory();

	if (LinkedInventoryWidget)
	{
		if (!LinkedInventoryWidget->IsVisible())
		{
			PC->InventoryUI();
		}
		if (!LinkedInventoryWidget->ForgeCanvas->IsVisible())
		{
			LinkedInventoryWidget->CurrentVendor = this;
			LinkedInventoryWidget->ToggleForgeUI(true);
		}
		else
		{
			LinkedInventoryWidget->ToggleForgeUI(false);
		}

	}
}

