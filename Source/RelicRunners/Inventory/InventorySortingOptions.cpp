/************************************************************************************
 * Author:   Tristan Anglin
 * Year:     2025
 *
 * Notice:
 *   This source code is the intellectual property of Tristan Anglin.
 *   It is provided solely for use within:
 *     - The Relic Runners personal project
 *     - The Almost A Dozen project
 *
 *   Any use, distribution, or modification outside of these projects
 *   is strictly prohibited without explicit written permission.
 *
 *   © 2025 Tristan Anglin. All rights reserved.
 ************************************************************************************/

#include "InventorySortingOptions.h"
#include "Components/Button.h"
#include <Blueprint/WidgetLayoutLibrary.h>

UInventorySortingOptions* UInventorySortingOptions::CurrentPopup = nullptr;

FString UInventorySortingOptions::GetSortingMethodDisplayName(EInventorySorting Method)
{
	switch (Method)
	{
		case EInventorySorting::SortByItemType:		return TEXT("ItemType");
		case EInventorySorting::SortByRarity:		return TEXT("Rarity");
		case EInventorySorting::SortByLevel:		return TEXT("Level");
		case EInventorySorting::SortByNewest:		return TEXT("Newest");
		default: return TEXT("Sort");
	}
}

void UInventorySortingOptions::Setup()
{
	InitialScreenPosition = UWidgetLayoutLibrary::GetMousePositionOnViewport(this);
	CurrentPopup = this;
}

void UInventorySortingOptions::ClosePopup()
{
	if (UInventorySortingOptions::CurrentPopup == this)
	{
		UInventorySortingOptions::CurrentPopup = nullptr;
	}

	RemoveFromParent();
}

void UInventorySortingOptions::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	// Close if mouse moved too far
	FVector2D CurrentMousePos = UWidgetLayoutLibrary::GetMousePositionOnViewport(this);
	float Distance = FVector2D::Distance(CurrentMousePos, InitialScreenPosition);

	float PixelDistance = 250.0f;
	if (Distance > PixelDistance)
	{
		ClosePopup();
		return;
	}

	if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		if (PC->WasInputKeyJustPressed(EKeys::LeftMouseButton) ||
			PC->WasInputKeyJustPressed(EKeys::RightMouseButton))
		{
			ClosePopup();
		}
	}
}

void UInventorySortingOptions::CloseAnyOpenPopup()
{
	if (!CurrentPopup) return;
	if (CurrentPopup->IsInViewport())
	{
		CurrentPopup->RemoveFromParent();
		CurrentPopup = nullptr;
	}
}

void UInventorySortingOptions::NativeConstruct()
{
	Super::NativeConstruct();

	if (B_SortByType) B_SortByType->OnClicked.AddDynamic(this, &UInventorySortingOptions::SortByItemTypeClicked);
	if (B_SortByRarity) B_SortByRarity->OnClicked.AddDynamic(this, &UInventorySortingOptions::SortByRarityClicked);
	if (B_SortByLevel) B_SortByLevel->OnClicked.AddDynamic(this, &UInventorySortingOptions::SortByLevelClicked);
	if (B_SortByNewest) B_SortByNewest->OnClicked.AddDynamic(this, &UInventorySortingOptions::SortByNewestClicked);
}

void UInventorySortingOptions::SortByItemTypeClicked()
{
	OnSortingSelected.Broadcast(EInventorySorting::SortByItemType);
}

void UInventorySortingOptions::SortByRarityClicked()
{
	OnSortingSelected.Broadcast(EInventorySorting::SortByRarity);
}
void UInventorySortingOptions::SortByNewestClicked()
{
	OnSortingSelected.Broadcast(EInventorySorting::SortByNewest);
}
void UInventorySortingOptions::SortByLevelClicked()
{
	OnSortingSelected.Broadcast(EInventorySorting::SortByLevel);
}