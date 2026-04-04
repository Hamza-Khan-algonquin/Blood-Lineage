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
#include "Components/WidgetComponent.h"
#include <Kismet/GameplayStatics.h>

AVendor::AVendor()
{
	PrimaryActorTick.bCanEverTick = true;
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
	TooltipWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("TooltipWidget"));
	TooltipWidgetComponent->SetupAttachment(RootComponent);
	TooltipWidgetComponent->SetWidgetSpace(EWidgetSpace::World);
	TooltipWidgetComponent->SetDrawAtDesiredSize(true);
	TooltipWidgetComponent->SetUsingAbsoluteRotation(true);
	TooltipWidgetComponent->SetRelativeLocation(FVector(0.f, 0.f, 0));
	TooltipWidgetComponent->SetRelativeScale3D(FVector(0.3f));
	TooltipWidgetComponent->SetPivot(FVector2D(0.5f, 1.f));
	TooltipWidgetComponent->CastShadow = false;

	static ConstructorHelpers::FClassFinder<UUserWidget> TooltipWidgetBPClass(TEXT("/Game/Inventory/BP_InventoryToolTipWorld"));
	if (TooltipWidgetBPClass.Succeeded())
	{
		TooltipWidgetClass = TooltipWidgetBPClass.Class;
		TooltipWidgetComponent->SetWidgetClass(TooltipWidgetClass);
	}

	NameWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("NameWidget"));
	NameWidgetComponent->SetupAttachment(RootComponent);
	NameWidgetComponent->SetWidgetSpace(EWidgetSpace::World);
	NameWidgetComponent->SetDrawAtDesiredSize(true);
	NameWidgetComponent->SetUsingAbsoluteRotation(true);
	NameWidgetComponent->SetRelativeLocation(FVector(0.f, 0.f, 0));
	NameWidgetComponent->SetRelativeScale3D(FVector(0.3f));
	NameWidgetComponent->SetPivot(FVector2D(0.5f, 1.f));
	NameWidgetComponent->CastShadow = false;

	static ConstructorHelpers::FClassFinder<UUserWidget> NameWidgetBPClass(TEXT("/Game/Vendor/BP_VendorHUDWorld"));
	if (NameWidgetBPClass.Succeeded())
	{
		NameWidgetClass = NameWidgetBPClass.Class;
		NameWidgetComponent->SetWidgetClass(NameWidgetClass);
	}
}

void AVendor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (!TooltipWidgetComponent) return;

	APlayerCameraManager* CamManager = UGameplayStatics::GetPlayerCameraManager(this, 0);
	if (CamManager)
	{
		FVector CameraLocation = CamManager->GetCameraLocation();
		FVector WidgetLocation = TooltipWidgetComponent->GetComponentLocation();

		FRotator LookAtRotation = (CameraLocation - WidgetLocation).Rotation();

		LookAtRotation.Pitch = 0.f;
		LookAtRotation.Roll = 0.f;

		TooltipWidgetComponent->SetWorldRotation(LookAtRotation);
		if(NameWidgetComponent) NameWidgetComponent->SetWorldRotation(LookAtRotation);
	}
}

void AVendor::InitializeTooltipWidget()
{
	if (!TooltipWidgetComponent->GetUserWidgetObject())
	{
		TooltipWidgetComponent->SetWidgetClass(TooltipWidgetClass);
		TooltipWidgetComponent->InitWidget();
	}

	TooltipWidgetComponent->SetWidgetSpace(EWidgetSpace::World);
	TooltipWidgetComponent->SetDrawAtDesiredSize(true);
	TooltipWidgetComponent->SetRelativeLocation(FVector(0, 0.f, 80));
	TooltipWidgetComponent->SetRelativeScale3D(FVector(0.125f));
	TooltipWidgetComponent->SetPivot(FVector2D(0.5f, 1.f));
	TooltipWidgetComponent->CastShadow = false;
	TooltipWidgetComponent->SetComponentTickEnabled(true);

	if (!TooltipWidgetComponent->IsRegistered())
	{
		TooltipWidgetComponent->RegisterComponent();
	}

	if (!NameWidgetComponent->GetUserWidgetObject())
	{
		NameWidgetComponent->SetWidgetClass(NameWidgetClass);
		NameWidgetComponent->InitWidget();
	}

	NameWidgetComponent->SetWidgetSpace(EWidgetSpace::World);
	NameWidgetComponent->SetDrawAtDesiredSize(true);
	NameWidgetComponent->SetRelativeLocation(FVector(0, 0.f, 100));
	NameWidgetComponent->SetRelativeScale3D(FVector(0.15f));
	NameWidgetComponent->SetPivot(FVector2D(0.5f, 1.f));
	NameWidgetComponent->CastShadow = false;
	NameWidgetComponent->SetComponentTickEnabled(true);

	if (!NameWidgetComponent->IsRegistered())
	{
		NameWidgetComponent->RegisterComponent();
	}
}

void AVendor::ShowTooltip_Implementation(bool bShow)
{
	if (TooltipWidgetComponent && NameWidgetComponent)
	{
		TooltipWidgetComponent->SetVisibility(bShow);
		NameWidgetComponent->SetVisibility(bShow);
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

	InitializeTooltipWidget();
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
	int LootQualityIncrease = 75;

	UIconLibrary* Lib = nullptr;
	if (URelicRunnersGameInstance* GI = GetWorld()->GetGameInstance<URelicRunnersGameInstance>())
	{
		Lib = GI->GlobalIconLibrary;
	}

	for (int i = 0; i < 50; ++i)
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
		if (LinkedInventoryWidget->GetVisibility() != ESlateVisibility::Visible)
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
