// Fill out your copyright notice in the Description page of Project Settings.


#include "StandardTooltip.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/Border.h"

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


