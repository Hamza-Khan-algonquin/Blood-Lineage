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
#include "RelicRunners/PlayerState/RelicRunnersPlayerState.h"
#include "GameFramework/CharacterMovementComponent.h"
#include <RelicRunners/Game/RelicRunnersGameInstance.h>

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
	
	int MinLevel = 999;
	int MaxLevel = 1;
	bool FoundPlayers = false;

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		ARelicRunnersPlayerController* TargetPC = Cast<ARelicRunnersPlayerController>(It->Get());
		if (TargetPC && TargetPC->PlayerState)
		{
			int PlayerLevel = TargetPC->GetPlayerState<ARelicRunnersPlayerState>()->PlayerLevel;

			MinLevel = FMath::Min(MinLevel, PlayerLevel);
			MaxLevel = FMath::Max(MaxLevel, PlayerLevel);
			FoundPlayers = true;
		}
	}

	if (!FoundPlayers)
	{
		MinLevel = 1;
		MaxLevel = 1;
	}

	//Force no common items in shop
	int LootQualityIncrease = 70;

	UIconLibrary* Lib = nullptr;
	if (URelicRunnersGameInstance* GI = GetWorld()->GetGameInstance<URelicRunnersGameInstance>())
	{
		Lib = GI->GlobalIconLibrary;
	}

	for (int i = 0; i < 20; ++i)
	{
		int ItemLevel = FMath::RandRange(MinLevel, MaxLevel);

		Stock.Add(ItemStats::CreateSpecificItemData(
			ItemLevel,
			ItemStats::GetRandomItemType(),
			ItemStats::GetRandomRarityWithLuck(LootQualityIncrease),
			ItemMeshData,
			Lib
		));
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
