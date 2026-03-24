// Fill out your copyright notice in the Description page of Project Settings.


#include "StandardTooltip.h"
#include "Components/TextBlock.h"
#include "Components/RichTextBlock.h"
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

	if (Icon1)
	{
		Icon1->SetVisibility(ESlateVisibility::Collapsed);
	}

	if (Icon2)
	{
		Icon2->SetVisibility(ESlateVisibility::Collapsed);
	}

	if (Icon3)
	{
		Icon3->SetVisibility(ESlateVisibility::Collapsed);
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
		if (TB_Desc1)
		{
			TB_Desc1->SetText(NewDescriptions[0]);
			B_Description1->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		}
	}

	//Description 2
	if (NewDescriptions.Num() > 1)
	{
		if (TB_Desc2)
		{
			TB_Desc2->SetText(NewDescriptions[1]);
			B_Description2->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		}
	}

	//Description 3
	if (NewDescriptions.Num() > 2)
	{
		if (TB_Desc3)
		{
			TB_Desc3->SetText(NewDescriptions[2]);
			B_Description3->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		}
	}

	//Description 4
	if (NewDescriptions.Num() > 3)
	{
		if (TB_Desc4)
		{
			TB_Desc4->SetText(NewDescriptions[3]);
			B_Description4->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		}
	}

	//Description 5
	if (NewDescriptions.Num() > 4)
	{
		if (TB_Desc5)
		{
			TB_Desc5->SetText(NewDescriptions[4]);
			B_Description5->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		}
	}
}

void UStandardTooltip::SetAnchor(FVector2D anchor)
{
	this->SetAlignmentInViewport(anchor);
}

void UStandardTooltip::SetIcon(UTexture2D* NewIcon, const FLinearColor& NewColor, int index)
{
	if (!this || !IsValid(this)) return;

	UImage* IconRef = nullptr;
	switch (index)
	{
	case 0: IconRef = Icon;  break;
	case 1: IconRef = Icon1; break;
	case 2: IconRef = Icon2; break;
	case 3: IconRef = Icon3; break;
	default: return;
	}

	if (IsValid(IconRef))
	{
		if (NewIcon)
		{
			IconRef->SetBrushFromTexture(NewIcon);
			IconRef->SetColorAndOpacity(NewColor);
			IconRef->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		}
		else
		{
			// If we have a valid ref but NO icon provided, hide it
			IconRef->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
}


