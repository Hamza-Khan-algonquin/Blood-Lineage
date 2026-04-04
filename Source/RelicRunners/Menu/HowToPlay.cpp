// Fill out your copyright notice in the Description page of Project Settings.


#include "HowToPlay.h"
#include "MainMenuWidget.h"
#include "Components/Button.h"
#include "Components/WidgetSwitcher.h"
#include <Kismet/GameplayStatics.h>

void UHowToPlay::NativeConstruct()
{
	Super::NativeConstruct();

	if (BackButton)
	{
		BackButton->OnClicked.AddDynamic(this, &UHowToPlay::BackButtonClicked);
	}

	if (B_Items)      B_Items->OnClicked.AddDynamic(this, &UHowToPlay::OnItemsClicked);
	if (B_Objectives) B_Objectives->OnClicked.AddDynamic(this, &UHowToPlay::OnObjectivesClicked);
	if (B_Upgrades)   B_Upgrades->OnClicked.AddDynamic(this, &UHowToPlay::OnUpgradesClicked);
	if (B_Enemies)    B_Enemies->OnClicked.AddDynamic(this, &UHowToPlay::OnEnemiesClicked);
	if (B_Classes)    B_Classes->OnClicked.AddDynamic(this, &UHowToPlay::OnClassesClicked);

	SwitchToPage(0);
}

void UHowToPlay::SetParentMenu(UMainMenuWidget* InParentMenu)
{
	ParentMenu = InParentMenu;
}

void UHowToPlay::BackButtonClicked()
{
	BlendToMainMenuCamera();
	SwitchToPage(0);
	if (ParentMenu)
	{
		ParentMenu->ShowModeSelectionWidget();
	}
}

void UHowToPlay::OnItemsClicked() { SwitchToPage(0); BlendToForgeCamera(); }
void UHowToPlay::OnObjectivesClicked() { SwitchToPage(1); BlendToObjectivesCamera(); }
void UHowToPlay::OnUpgradesClicked() { SwitchToPage(2); BlendToUpgradesCamera();}
void UHowToPlay::OnEnemiesClicked() { SwitchToPage(3); BlendToEnemiesCamera(); }
void UHowToPlay::OnClassesClicked() { SwitchToPage(4); BlendToClassesCamera(); }

void UHowToPlay::SwitchToPage(int Index)
{
	if (ContentSwitcher)
	{
		ContentSwitcher->SetActiveWidgetIndex(Index);
	}
	TArray<UButton*> Tabs = { B_Items, B_Objectives, B_Upgrades, B_Enemies, B_Classes };

	for (int i = 0; i < Tabs.Num(); i++)
	{
		if (Tabs[i])
		{
			FLinearColor Tint = (i == Index) ? FLinearColor::Yellow : FLinearColor::White;
			Tabs[i]->SetBackgroundColor(Tint);
		}
	}
}

void UHowToPlay::ExecuteCameraBlend(FName TargetTag)
{
	APlayerController* PC = GetOwningPlayer();
	if (!PC || !PC->PlayerCameraManager) return;

	AActor* TargetCamera = FindActorByTag(TargetTag);
	if (TargetCamera)
	{
		if (PC->GetViewTarget() == TargetCamera)
		{
			return;
		}

		PC->SetViewTargetWithBlend(TargetCamera, 1.0f, EViewTargetBlendFunction::VTBlend_Cubic);
		PC->PlayerCameraManager->StartCameraFade(0.0f, 1.0f, 0.3f, FLinearColor::Black, false, true);
		FTimerHandle FadeInTimer;
		GetWorld()->GetTimerManager().SetTimer(FadeInTimer, [PC]()
			{
				if (PC && PC->PlayerCameraManager)
				{
					PC->PlayerCameraManager->StartCameraFade(1.0f, 0.0f, 0.3f, FLinearColor::Black, false, false);
				}
			}, 0.7f, false);
	}
}

void UHowToPlay::BlendToForgeCamera() { ExecuteCameraBlend(FName("ForgeCamera")); }
void UHowToPlay::BlendToObjectivesCamera() { ExecuteCameraBlend(FName("ObjectiveCamera")); }
void UHowToPlay::BlendToEnemiesCamera() { ExecuteCameraBlend(FName("EnemiesCamera")); }
void UHowToPlay::BlendToClassesCamera() { ExecuteCameraBlend(FName("ClassesCamera")); }
void UHowToPlay::BlendToMainMenuCamera() { ExecuteCameraBlend(FName("MainMenuCamera")); }
void UHowToPlay::BlendToUpgradesCamera() { ExecuteCameraBlend(FName("CreditsCamera")); }

AActor* UHowToPlay::FindActorByTag(FName TargetTag)
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