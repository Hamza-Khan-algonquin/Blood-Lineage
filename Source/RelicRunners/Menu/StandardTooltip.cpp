// Fill out your copyright notice in the Description page of Project Settings.


#include "StandardTooltip.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/Border.h"
#include "Components/CanvasPanelSlot.h"

void UStandardTooltip::NativeConstruct()
{
	Super::NativeConstruct();

	if (Icon)
	{
		Icon->SetVisibility(ESlateVisibility::Collapsed);
	}

	if (TB_Value)
	{
		TB_Value->SetVisibility(ESlateVisibility::Collapsed);
	}

	if (B_Description1)
	{
		B_Description1->SetVisibility(ESlateVisibility::Collapsed);
	}

	if (B_Description2)
	{
		B_Description2->SetVisibility(ESlateVisibility::Collapsed);
	}

	if (B_Description3)
	{
		B_Description3->SetVisibility(ESlateVisibility::Collapsed);
	}

	if (B_Description4)
	{
		B_Description4->SetVisibility(ESlateVisibility::Collapsed);
	}

	if (B_Description5)
	{
		B_Description5->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UStandardTooltip::Setup(const FText& Title, const FText& Value, const TArray<FText>& NewDescriptions)
{
	if (TB_Name)
	{
		TB_Name->SetText(Title);
	}

	if (!Value.IsEmpty() && TB_Value)
	{
		TB_Value->SetText(Value);
		TB_Value->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}
	else if (TB_Value)
	{
		TB_Value->SetVisibility(ESlateVisibility::Collapsed);
	}

	//Description 1
	if (NewDescriptions.Num() > 0)
	{
		if (TB_Description1)
		{
			TB_Description1->SetText(NewDescriptions[0]);
		}

		if (B_Description1)
		{
			B_Description1->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		}
	}

	//Description 2
	if (NewDescriptions.Num() > 1)
	{
		if (TB_Description2)
		{
			TB_Description2->SetText(NewDescriptions[1]);
		}

		if (B_Description2)
		{
			B_Description2->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		}
	}

	//Description 3
	if (NewDescriptions.Num() > 2)
	{
		if (TB_Description3)
		{
			TB_Description3->SetText(NewDescriptions[2]);
		}

		if (B_Description3)
		{
			B_Description3->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		}
	}

	//Description 4
	if (NewDescriptions.Num() > 3)
	{
		if (TB_Description4)
		{
			TB_Description4->SetText(NewDescriptions[3]);
		}

		if (B_Description4)
		{
			B_Description4->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		}
	}

	//Description 5
	if (NewDescriptions.Num() > 4)
	{
		if (TB_Description5)
		{
			TB_Description5->SetText(NewDescriptions[4]);
		}

		if (B_Description5)
		{
			B_Description5->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		}
	}
}

void UStandardTooltip::SetAnchor(FVector2D anchor)
{
	this->SetAlignmentInViewport(anchor);
}

void UStandardTooltip::SetIcon(UTexture2D* NewIcon, const FLinearColor& NewColor)
{
	if (!Icon) return;

	if (NewIcon)
	{
		Icon->SetBrushFromTexture(NewIcon);
		Icon->SetColorAndOpacity(NewColor);
		Icon->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}
	else
	{
		Icon->SetVisibility(ESlateVisibility::Collapsed);
	}
}


