#include "Vendor.h"

#include "Components/CapsuleComponent.h"
#include "Components/CanvasPanel.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/PlayerController.h"
#include "Net/UnrealNetwork.h"

#include "RelicRunners/RelicRunnersCharacter.h"
#include "RelicRunners/Inventory/InventoryComponent.h"
#include "RelicRunners/Item/ItemStats.h"
#include "RelicRunners/Item/ItemData.h"
#include "RelicRunners/Inventory/Inventory.h"
#include <RelicRunners/PlayerController/RelicRunnersPlayerController.h>
#include "GameFramework/CharacterMovementComponent.h"

AVendor::AVendor()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	if (UCapsuleComponent* Capsule = GetCapsuleComponent())
	{
		Capsule->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		Capsule->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
		Capsule->SetSimulatePhysics(false);
		Capsule->SetEnableGravity(false);
	}

	if (GetMesh())
	{
		GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		GetMesh()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	}
}

void AVendor::BeginPlay()
{
	Super::BeginPlay();

	if (ItemMeshDataClass)
	{
		ItemMeshData = ItemMeshDataClass->GetDefaultObject<UItemMeshData>();
	}

	if (HasAuthority())
	{
		GenerateStock();
	}
}

void AVendor::GenerateStock()
{
	Stock.Empty();

	if (!ItemMeshData)
	{
		return;
	}
	
	int LootQualityIncrease = 60;

	for (int i = 0; i < 20; ++i)
	{
		Stock.Add(ItemStats::CreateSpecificItemData(1, ItemStats::GetRandomItemType(), ItemStats::GetRandomRarityWithLuck(LootQualityIncrease), ItemMeshData));
	}
}

void AVendor::RerollStock()
{
	if (HasAuthority())
	{
		GenerateStock();
	}
}

void AVendor::AddStock(const FItemData& ItemData)
{
	if (HasAuthority())
	{
		Stock.Add(ItemData);
		OnRep_Stock();
	}
}

void AVendor::RemoveStock(const FItemData& ItemData)
{
	for (int i = 0; i < Stock.Num(); ++i)
	{
		if (Stock[i].UniqueID == ItemData.UniqueID)
		{
			Stock.RemoveAt(i);
			OnRep_Stock();
			return;
		}
	}
}

void AVendor::Interact_Implementation(ARelicRunnersCharacter* Char)
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
		if (!LinkedInventoryWidget->ShopCanvas->IsVisible())
		{
			LinkedInventoryWidget->ToggleShopUI(true);
			LinkedInventoryWidget->CurrentVendor = this;
			LinkedInventoryWidget->DisplaySelectedVendorItems(Stock, this);
		}
		else
		{
			LinkedInventoryWidget->ToggleShopUI(false);
		}

	}
}

FItemData AVendor::GetItemData_Implementation()
{
	return FItemData();
}

void AVendor::ShowTooltip_Implementation(bool bShow)
{
	
}

void AVendor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AVendor, Stock);
}

void AVendor::OnRep_Stock()
{
	if (LinkedInventoryWidget)
	{
		LinkedInventoryWidget->DisplaySelectedVendorItems(Stock, this);
	}
}
