// Fill out your copyright notice in the Description page of Project Settings.


#include "CreditsWidget.h"
#include "MainMenuWidget.h"
#include "RelicRunners/Game/RelicRunnersGameInstance.h"
#include "Components/TextBlock.h"
#include "Components/Border.h"
#include "Components/Image.h"
#include "Components/Button.h"
#include <Kismet/GameplayStatics.h>

void UCreditsWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (BackButton)
	{
		BackButton->OnClicked.AddDynamic(this, &UCreditsWidget::BackButtonClicked);
	}

}

void UCreditsWidget::SetParentMenu(UMainMenuWidget* InParentMenu)
{
	ParentMenu = InParentMenu;
}

void UCreditsWidget::BackButtonClicked()
{
	BlendToMainMenuCamera();
	if (ParentMenu)
	{
		ParentMenu->ShowModeSelectionWidget();
	}
}

void UCreditsWidget::BlendToCreditsCamera()
{
	APlayerController* PC = GetOwningPlayer();
	if (!PC) return;

	AActor* TargetCamera = FindActorByTag(FName("CreditsCamera"));
	if (TargetCamera)
	{
		PC->SetViewTargetWithBlend(TargetCamera, 1.0f, EViewTargetBlendFunction::VTBlend_Cubic);
	}
}

void UCreditsWidget::BlendToMainMenuCamera()
{
	APlayerController* PC = GetOwningPlayer();
	if (!PC) return;

	AActor* TargetCamera = FindActorByTag(FName("MainMenuCamera"));
	if (TargetCamera)
	{
		PC->SetViewTargetWithBlend(TargetCamera, 1.0f, EViewTargetBlendFunction::VTBlend_Cubic);
	}
}

AActor* UCreditsWidget::FindActorByTag(FName TargetTag)
{
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsWithTag(GetWorld(), TargetTag, FoundActors);

	if (FoundActors.Num() > 0)
	{
		return FoundActors[0];
	}

	UE_LOG(LogTemp, Warning, TEXT("HowToPlay: Could not find actor with tag %s"), *TargetTag.ToString());
	return nullptr;
}

