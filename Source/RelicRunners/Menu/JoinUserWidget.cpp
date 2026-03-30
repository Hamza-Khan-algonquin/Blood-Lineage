// Fill out your copyright notice in the Description page of Project Settings.


#include "JoinUserWidget.h"
#include "Sockets.h"
#include "SocketSubsystem.h"
#include "RelicRunners/Game/RelicRunnersGameInstance.h"
#include "Components/Button.h"
#include "Components/TileView.h"
#include "SessionListItemWidget.h"
#include "SessionListItemData.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/Border.h"
#include "Components/ProgressBar.h"
#include <Camera/CameraComponent.h>

#include "MainMenuWidget.h"
#include "Components/CircularThrobber.h"
#include <Kismet/GameplayStatics.h>
#include "ModeSelectionWidget.h"
#include "GameFramework/GameStateBase.h"
#include "RelicRunners/Classes/ClassData.h"
#include "RelicRunners/Classes/ClassInfo.h"
#include <RelicRunners/PlayerController/RelicRunnersPlayerController.h>
#include <RelicRunners/PlayerState/RelicRunnersPlayerState.h>
#include "StandardTooltip.h"
#include <RelicRunners/Item/ItemStats.h>
#include "LobbyGameMode.h"
#include "RelicRunners/PlayerPreview/LobbyPreview.h"
#include <RelicRunners/PlayerHUD/LobbyHUDWorld.h>
#include "Components/WidgetComponent.h"
#include "RelicRunners/RelicRunnersGameState.h"
#include "Sound/SoundBase.h"
#include "Animation/WidgetAnimation.h"

void UJoinUserWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (StartButton) StartButton->OnClicked.AddDynamic(this, &UJoinUserWidget::StartGameButtonClicked);
	if (LeaveGameButton) LeaveGameButton->OnClicked.AddDynamic(this, &UJoinUserWidget::LeaveGameButtonClicked);
	if (FindGames) FindGames->OnClicked.AddDynamic(this, &UJoinUserWidget::FindGamesButtonClicked);
	if (SessionTileView) SessionTileView->OnEntryWidgetGenerated().AddUObject(this, &UJoinUserWidget::HandleEntryGenerated);
	if (BackButton) BackButton->OnClicked.AddDynamic(this, &UJoinUserWidget::BackButtonClicked);
	if (JoinButton) JoinButton->OnClicked.AddDynamic(this, &UJoinUserWidget::JoinGameButtonClicked);
	if (AresButton) AresButton->OnClicked.AddDynamic(this, &UJoinUserWidget::OnAresClicked);
	if (ArtemisButton) ArtemisButton->OnClicked.AddDynamic(this, &UJoinUserWidget::OnArtemisClicked);
	if (AphroditeButton) AphroditeButton->OnClicked.AddDynamic(this, &UJoinUserWidget::OnAphroditeClicked);
	if (NemesisButton) NemesisButton->OnClicked.AddDynamic(this, &UJoinUserWidget::OnNemesisClicked);
	SetSelectedClass("Ares");

	bool IsHost = true;
	if (const UWorld* World = GetWorld())
	{
		//If not host then collapse these elements
		IsHost = World->GetAuthGameMode() != nullptr;
		
		StartButton->SetVisibility(IsHost ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
		B_Start->SetVisibility(IsHost ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
		B_BoarSection->SetVisibility(IsHost ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
		B_BullSection->SetVisibility(IsHost ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
		B_LionSection->SetVisibility(IsHost ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
		B_HydraSection->SetVisibility(IsHost ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
		B_CerberusSection->SetVisibility(IsHost ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);

		//Client Only
		B_CurrentSection->SetVisibility(IsHost ? ESlateVisibility::Collapsed : ESlateVisibility::Visible);
	}

	AGameStateBase* GameState = GetWorld()->GetGameState();

	if (ARelicRunnersGameState* RunnersGameState = Cast<ARelicRunnersGameState>(GameState))
	{
		FDifficultyInfo diffInfo = RunnersGameState->GetDifficultyInfo("Boar");
		DifficultyMap.Add("Boar", { I_Boar, BoarBorder, B_Boar, B_BoarSection, diffInfo.HealthPercent, diffInfo.DamagePercent, diffInfo.LuckAmount });
		diffInfo = RunnersGameState->GetDifficultyInfo("Bull");
		DifficultyMap.Add("Bull", { I_Bull, BullBorder, B_Bull, B_BullSection, diffInfo.HealthPercent, diffInfo.DamagePercent, diffInfo.LuckAmount });
		diffInfo = RunnersGameState->GetDifficultyInfo("Lion");
		DifficultyMap.Add("Lion", { I_Lion, LionBorder, B_Lion, B_LionSection, diffInfo.HealthPercent, diffInfo.DamagePercent, diffInfo.LuckAmount });
		diffInfo = RunnersGameState->GetDifficultyInfo("Hydra");
		DifficultyMap.Add("Hydra", { I_Hydra, HydraBorder, B_Hydra, B_HydraSection, diffInfo.HealthPercent, diffInfo.DamagePercent, diffInfo.LuckAmount });
		diffInfo = RunnersGameState->GetDifficultyInfo("Cerberus");
		DifficultyMap.Add("Cerberus", { I_Cerberus, CerberusBorder, B_Cerberus, B_CerberusSection, diffInfo.HealthPercent, diffInfo.DamagePercent, diffInfo.LuckAmount });
	}
	else
	{
		DifficultyMap.Add("Boar", { I_Boar, BoarBorder, B_Boar, B_BoarSection, 100, 100, 0 });
		DifficultyMap.Add("Bull", { I_Bull, BullBorder, B_Bull, B_BullSection, 140, 125, 1 });
		DifficultyMap.Add("Lion", { I_Lion, LionBorder, B_Lion, B_LionSection, 180, 150, 2 });
		DifficultyMap.Add("Hydra", { I_Hydra, HydraBorder, B_Hydra, B_HydraSection, 220, 175, 3 });
		DifficultyMap.Add("Cerberus", { I_Cerberus, CerberusBorder, B_Cerberus, B_CerberusSection, 300, 250, 5 });
	}
	UpdateDifficultyNumbers();

	if (B_Boar)     B_Boar->OnClicked.AddDynamic(this, &UJoinUserWidget::OnDifficultyClicked_Boar);
	if (B_Bull)     B_Bull->OnClicked.AddDynamic(this, &UJoinUserWidget::OnDifficultyClicked_Bull);
	if (B_Lion)     B_Lion->OnClicked.AddDynamic(this, &UJoinUserWidget::OnDifficultyClicked_Lion);
	if (B_Hydra)    B_Hydra->OnClicked.AddDynamic(this, &UJoinUserWidget::OnDifficultyClicked_Hydra);
	if (B_Cerberus) B_Cerberus->OnClicked.AddDynamic(this, &UJoinUserWidget::OnDifficultyClicked_Cerberus);

	if (!IsHost)
	{
		GetWorld()->GetTimerManager().SetTimer(
			TimerHandle_DifficultyUpdate,
			this,
			&UJoinUserWidget::UpdateClientDifficultyUI,
			0.5f,
			true
		);
	}
	else
	{
		SetSelectedDifficulty("Boar");
	}



	PlayAnimation(LeftPanelOpen);
	PlayAnimation(RightPanelOpen);
	ApplyColorblindStyle();
	HandleColorModeChanged();
}

FReply UJoinUserWidget::NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseMove(InGeometry, InMouseEvent);

	FVector2D CursorPos = InMouseEvent.GetScreenSpacePosition();

	TArray<UBorder*> Borders = { B_Health, B_Defence, B_Strength, B_Agility, B_Intelligence,
								 B_Ability1, B_Ability2, B_Ability3, B_Ability4, B_BoarSection1, B_BullSection1, B_LionSection1, B_HydraSection1, B_CerberusSection1, B_CurrentSection1 };

	UBorder* HoveredBorder = nullptr;
	for (UBorder* Border : Borders)
	{
		if (!Border) continue;

		FGeometry BorderGeom = Border->GetCachedGeometry();
		if (BorderGeom.IsUnderLocation(CursorPos))
		{
			HoveredBorder = Border;
			break;
		}
	}

	if (HoveredBorder != CurrentHoveredBorder)
	{

		if (StandardTooltip && CurrentHoveredBorder)
		{
			StandardTooltip->RemoveFromParent();
			StandardTooltip = nullptr;
		}

		CurrentHoveredBorder = HoveredBorder;
	}

	if (CurrentHoveredBorder)
	{
		ShowTooltipForBorder(CurrentHoveredBorder, CursorPos, InGeometry);
	}

	if (CurrentHoveredBorder) return FReply::Handled();
	else return FReply::Unhandled();
}

void UJoinUserWidget::UpdateDifficultyNumbers()
{
	TB_BoarHealth->SetText(FText::AsNumber(DifficultyMap["Boar"].HealthPercent));
	TB_BoarDamage->SetText(FText::AsNumber(DifficultyMap["Boar"].DamagePercent));
	TB_BoarLuck->SetText(FText::AsNumber(DifficultyMap["Boar"].LuckAmount));

	TB_BullHealth->SetText(FText::AsNumber(DifficultyMap["Bull"].HealthPercent));
	TB_BullDamage->SetText(FText::AsNumber(DifficultyMap["Bull"].DamagePercent));
	TB_BullLuck->SetText(FText::AsNumber(DifficultyMap["Bull"].LuckAmount));

	TB_LionHealth->SetText(FText::AsNumber(DifficultyMap["Lion"].HealthPercent));
	TB_LionDamage->SetText(FText::AsNumber(DifficultyMap["Lion"].DamagePercent));
	TB_LionLuck->SetText(FText::AsNumber(DifficultyMap["Lion"].LuckAmount));

	TB_HydraHealth->SetText(FText::AsNumber(DifficultyMap["Hydra"].HealthPercent));
	TB_HydraDamage->SetText(FText::AsNumber(DifficultyMap["Hydra"].DamagePercent));
	TB_HydraLuck->SetText(FText::AsNumber(DifficultyMap["Hydra"].LuckAmount));

	TB_CerberusHealth->SetText(FText::AsNumber(DifficultyMap["Cerberus"].HealthPercent));
	TB_CerberusDamage->SetText(FText::AsNumber(DifficultyMap["Cerberus"].DamagePercent));
	TB_CerberusLuck->SetText(FText::AsNumber(DifficultyMap["Cerberus"].LuckAmount));
}

void UJoinUserWidget::ShowTooltipForBorder(UBorder* HoveredBorder, const FVector2D& CursorPos, const FGeometry& ParentGeometry)
{
	if (!StandardTooltipClass || !ClassDataAsset || !HoveredBorder) return;

	const FClassData& Info = ClassDataAsset->Classes[CurrentClass];
	APlayerController* PC = GetOwningPlayer();
	if (!PC) return;

	if (!StandardTooltip)
	{
		StandardTooltip = CreateWidget<UStandardTooltip>(PC, StandardTooltipClass);
		StandardTooltip->AddToViewport();
	}

	FString StatKey = "";
	int RawValue = 0;
	int RawValue1 = 0;
	int RawValue2 = 0;
	FString AbilityName = "";
	FString AbilityDesc = "";
	bool Difficulty = false;
	FName DifficultyName = "Boar";

	if (HoveredBorder == B_Health)				{ StatKey = "Health";       RawValue = Info.Health; }
	else if (HoveredBorder == B_Defence)		{ StatKey = "Defence";      RawValue = Info.Armor; }
	else if (HoveredBorder == B_Strength)		{ StatKey = "Strength";     RawValue = Info.Strength; }
	else if (HoveredBorder == B_Agility)		{ StatKey = "Agility";      RawValue = Info.Dexterity; }
	else if (HoveredBorder == B_Intelligence)	{ StatKey = "Intelligence"; RawValue = Info.Intelligence; }

	else if (HoveredBorder == B_Ability1)		{ StatKey = "Ability1"; AbilityName = Info.Ability1; AbilityDesc = Info.Ability1Description1; }
	else if (HoveredBorder == B_Ability2)		{ StatKey = "Ability2"; AbilityName = Info.Ability2; AbilityDesc = Info.Ability2Description1; }
	else if (HoveredBorder == B_Ability3)		{ StatKey = "Ability3"; AbilityName = Info.Ability3; AbilityDesc = Info.Ability3Description1; }
	else if (HoveredBorder == B_Ability4)		{ StatKey = "Ability4"; AbilityName = Info.Ability4; AbilityDesc = Info.Ability4Description1; }

	else if (HoveredBorder == B_BoarSection1) { StatKey = "BoarDifficulty"; Difficulty = true; DifficultyName = "Boar"; }
	else if (HoveredBorder == B_BullSection1) { StatKey = "BullDifficulty"; Difficulty = true; DifficultyName = "Bull";}
	else if (HoveredBorder == B_LionSection1) { StatKey = "LionDifficulty"; Difficulty = true; DifficultyName = "Lion";}
	else if (HoveredBorder == B_HydraSection1) { StatKey = "HydraDifficulty"; Difficulty = true; DifficultyName = "Hydra";}
	else if (HoveredBorder == B_CerberusSection1) { StatKey = "CerberusDifficulty"; Difficulty = true; DifficultyName = "Cerberus";}
	else if (HoveredBorder == B_CurrentSection1) { StatKey = "CurrentDifficulty"; Difficulty = true; DifficultyName = "Current"; }

	FVector2D Anchor(0.f, 1.f);
	FVector2D Offset(5.0f, 5.0f);

	FTooltipData TD;
	if (!Difficulty)
	{
		TD = ItemStats::GetStatTooltipData(StatKey, RawValue, 0, 0, AbilityName, AbilityDesc);
		StandardTooltip->SetIcon(TD.Icon, TD.IconColor, 0);
	}
	else
	{
		ARelicRunnersGameState* RunnersGameState = GetWorld()->GetGameState<ARelicRunnersGameState>();
		if (!RunnersGameState) return;

		FDifficultyInfo diffInfo = RunnersGameState->GetDifficultyInfo("Boar");
		if (DifficultyName == "Current")
		{
			diffInfo = RunnersGameState->GetDifficultyInfo(RunnersGameState->CurrentDifficulty);
		}
		else
		{
			diffInfo = RunnersGameState->GetDifficultyInfo(DifficultyName);
		}

		Anchor = FVector2D(1.f, 1.f);
		Offset = FVector2D(-5.0f, 5.0f);
		TD = ItemStats::GetStatTooltipData(StatKey, (diffInfo.HealthPercent*100), (diffInfo.DamagePercent * 100), diffInfo.LuckAmount, RunnersGameState->CurrentDifficulty.ToString(), AbilityDesc);
		StandardTooltip->SetIcon(TD.Icon, TD.IconColor, 0);
		StandardTooltip->SetIcon(TD.Icon1, TD.IconColor1, 1);
		StandardTooltip->SetIcon(TD.Icon2, TD.IconColor2, 2);
		StandardTooltip->SetIcon(TD.Icon3, TD.IconColor3, 3);
	}

	StandardTooltip->Setup(TD.Name, TD.Value, TD.Descriptions);

	FVector2D ViewportPosition = ParentGeometry.AbsoluteToLocal(CursorPos);

	StandardTooltip->SetAnchor(Anchor);
	StandardTooltip->SetPositionInViewport(ViewportPosition + Offset, false);
}

void UJoinUserWidget::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseLeave(InMouseEvent);

	if (StandardTooltip)
	{
		StandardTooltip->RemoveFromParent();
		StandardTooltip = nullptr;
	}

	CurrentHoveredBorder = nullptr;
}

void UJoinUserWidget::SetButtonColor(UButton* Button, const FLinearColor& Color)
{
	if (!Button) return;

	Button->SetBackgroundColor(Color);
}

void UJoinUserWidget::SetBorderColor(UBorder* Border, const FLinearColor& Color)
{
	if (!Border) return;

	Border->SetBrushColor(Color);
}

void UJoinUserWidget::SetImageColor(UImage* Image, const FLinearColor& Color)
{
	if (!Image) return;

	Image->SetColorAndOpacity(Color);
}

void UJoinUserWidget::SetBorderOutlineColor(UBorder* Border, const FLinearColor& InsideColor, const FLinearColor& OutlineColor)
{
	if (Border)
	{
		Border->SetBrushColor(InsideColor);
		FSlateBrush Brush = Border->Background;
		Brush.OutlineSettings.Color = FSlateColor(OutlineColor);
		Border->SetBrush(Brush);
	}
}

void UJoinUserWidget::SetButtonTint(UButton* Button, const FLinearColor& Color)
{
	if (!Button) return;

	FButtonStyle Style = Button->GetStyle();
	Style.Normal.TintColor = FSlateColor(Color);
	Style.Hovered.TintColor = FSlateColor(Color);
	Style.Pressed.TintColor = FSlateColor(Color);
	Button->SetStyle(Style);
}

void UJoinUserWidget::SetButtonOutlineColor(UButton* Button, const FLinearColor& Color)
{
	if (!Button) return;

	FButtonStyle Style = Button->GetStyle();

	Style.Normal.OutlineSettings.Color = Color;
	Style.Hovered.OutlineSettings.Color = Color;
	Style.Pressed.OutlineSettings.Color = Color;
	Style.Disabled.OutlineSettings.Color = Color;

	Button->SetStyle(Style);
}

void UJoinUserWidget::OnDifficultyClicked_Boar()
{
	PlayButtonClickSound();
	SetSelectedDifficulty("Boar");
}

void UJoinUserWidget::OnDifficultyClicked_Bull()
{
	PlayButtonClickSound();
	SetSelectedDifficulty("Bull");
}

void UJoinUserWidget::OnDifficultyClicked_Lion()
{
	PlayButtonClickSound();
	SetSelectedDifficulty("Lion");
}

void UJoinUserWidget::OnDifficultyClicked_Hydra()
{
	PlayButtonClickSound();
	SetSelectedDifficulty("Hydra");
}

void UJoinUserWidget::OnDifficultyClicked_Cerberus()
{
	PlayButtonClickSound();
	SetSelectedDifficulty("Cerberus");
}

void UJoinUserWidget::SetSelectedDifficulty(FName DifficultyKey)
{
	APlayerController* PC = GetOwningPlayer();
	if (PC)
	{
		ARelicRunnersPlayerController* RPC = Cast<ARelicRunnersPlayerController>(PC);
		RPC->Server_SetDifficulty(DifficultyKey);
	}

	const FLinearColor Silver(0.624f, 0.624f, 0.624f, 1.f);
	const FLinearColor DarkGold(0.43f, 0.29f, 0.1f, 1.f);
	const FLinearColor Gold(1.f, 0.5f, 0.f, 1.f);
	const FLinearColor Black(0.f, 0.f, 0.f, 1.f);
	const FLinearColor SectionBrush_Unselected(0.292f, 0.23f, 0.145f, 1.f);
	const FLinearColor SectionOutline_Unselected(0.429f, 0.338f, 0.214f, 1.f);
	const FLinearColor OuterBrush_Unselected(0.062f, 0.049f, 0.031f, 1.f);
	const FLinearColor ButtonTint_Unselected(1.f, 1.f, 1.f, 0.6f);

	//Reset
	for (auto& Pair : DifficultyMap)
	{
		const FDifficultyWidgets& Widgets = Pair.Value;
		if (!Widgets.Icon || !Widgets.Button || !Widgets.OuterBorder || !Widgets.SectionBorder) continue;

		Widgets.Icon->SetColorAndOpacity(Silver);
		SetBorderOutlineColor(Widgets.SectionBorder, SectionBrush_Unselected, SectionOutline_Unselected);
		SetButtonTint(Widgets.Button, ButtonTint_Unselected);
	}
	//Set
	if (FDifficultyWidgets* Selected = DifficultyMap.Find(DifficultyKey))
	{
		Selected->Icon->SetColorAndOpacity(Gold);
		SetBorderOutlineColor(Selected->SectionBorder, Gold, DarkGold);
		SetButtonTint(Selected->Button, Gold);
	}
}

void UJoinUserWidget::UpdateClientDifficultyUI()
{
	ARelicRunnersGameState* RunnersGameState = GetWorld()->GetGameState<ARelicRunnersGameState>();
	if (!RunnersGameState) return;

	FName ActualDifficulty = RunnersGameState->CurrentDifficulty;

	if (ActualDifficulty.IsNone()) return;

	if (URelicRunnersGameInstance* GI = Cast<URelicRunnersGameInstance>(GetGameInstance()))
	{
		GI->CurrentDifficulty = ActualDifficulty;
	}

	const FLinearColor DarkGold(0.6f, 0.3f, 0.f, 1.f);
	FString IconName = ActualDifficulty.ToString() + "Difficulty";

	I_CurrentDifficulty->SetBrushFromTexture(ItemStats::LoadIcon(IconName));
	I_CurrentDifficulty->SetColorAndOpacity(DarkGold);

	FDifficultyInfo diffInfo = RunnersGameState->GetDifficultyInfo(ActualDifficulty);
	TB_CurrentDifficulty->SetText(FText::FromName(ActualDifficulty));
	TB_CurrentEnemyDamage->SetText(FText::AsNumber(diffInfo.HealthPercent));
	TB_CurrentEnemyHealth->SetText(FText::AsNumber(diffInfo.DamagePercent));
	TB_CurrentLuck->SetText(FText::AsNumber(diffInfo.LuckAmount));

	GetWorld()->GetTimerManager().ClearTimer(TimerHandle_DifficultyUpdate);
}
void UJoinUserWidget::PlayButtonClickSound()
{
	if (!ButtonClickSound) return;

	if (UWorld* World = GetWorld())
	{
		UGameplayStatics::PlaySound2D(World, ButtonClickSound);
	}
}

void UJoinUserWidget::OnAresClicked() { PlayButtonClickSound(); SetSelectedClass("Ares"); }
void UJoinUserWidget::OnArtemisClicked() { PlayButtonClickSound(); SetSelectedClass("Artemis"); }
void UJoinUserWidget::OnAphroditeClicked() { PlayButtonClickSound(); SetSelectedClass("Aphrodite"); }
void UJoinUserWidget::OnNemesisClicked() { PlayButtonClickSound(); SetSelectedClass("Nemesis"); }

void UJoinUserWidget::SetSelectedClass(FName ClassKey)
{
	CurrentClass = ClassKey;
	FLinearColor Gold(1.f, 0.5f, 0, 1.f);
	FLinearColor Cyan(0, 1, 0.714, 1.f);
	FLinearColor Silver(0.5, 0.5, 0.5, 1.f);
	FLinearColor Black(0,0,0,1);
	FLinearColor White(1, 1, 1, 1);

	//Reset all buttons
	SetButtonColor(AresButton, Cyan);
	SetButtonColor(ArtemisButton, Cyan);
	SetButtonColor(AphroditeButton, Cyan);
	SetButtonColor(NemesisButton, Cyan);
	SetBorderColor(B_Ares, Black);
	SetBorderColor(B_Artemis, Black);
	SetBorderColor(B_Aphrodite, Black);
	SetBorderColor(B_Nemesis, Black);
	SetImageColor(I_Ares, White);
	SetImageColor(I_Artemis, White);
	SetImageColor(I_Aphrodite, White);
	SetImageColor(I_Nemesis, White);

	//Highlight the selected one gold
	if (ClassKey == "Ares")
	{
		SetButtonColor(AresButton, Gold);
		SetBorderColor(B_Ares, Black);
		SetImageColor(I_Ares, Black);
	}
	else if (ClassKey == "Artemis")
	{
		SetButtonColor(ArtemisButton, Gold);
		SetBorderColor(B_Artemis, Black);
		SetImageColor(I_Artemis, Black);
	}
	else if (ClassKey == "Aphrodite")
	{
		SetButtonColor(AphroditeButton, Gold);
		SetBorderColor(B_Aphrodite, Black);
		SetImageColor(I_Aphrodite, Black);
	}
	else if (ClassKey == "Nemesis")
	{
		SetButtonColor(NemesisButton, Gold);
		SetBorderColor(B_Nemesis, Black);
		SetImageColor(I_Nemesis, Black);
	}

	if (!ClassDataAsset || !ClassDataAsset->Classes.Contains(ClassKey))
	{
		UE_LOG(LogTemp, Warning, TEXT("Invalid class key: %s"), *ClassKey.ToString());
		return;
	}

	const FClassData& Info = ClassDataAsset->Classes[ClassKey];
	const FClassData MaxValues = ClassDataAsset->GetMaxStats();

	if (TB_ClassName) TB_ClassName->SetText(FText::FromName(Info.ClassName));
	if (TB_ClassRole) TB_ClassRole->SetText(FText::FromString(Info.Role));
	if (TB_Ability1)  TB_Ability1->SetText(FText::FromString(Info.Ability1));
	if (TB_Ability2)  TB_Ability2->SetText(FText::FromString(Info.Ability2));
	if (TB_Ability3)  TB_Ability3->SetText(FText::FromString(Info.Ability3));
	if (TB_Ability4)  TB_Ability4->SetText(FText::FromString(Info.Ability4));

	auto Normalize = [](float Value, float Max) { return (Max > 0.f) ? (Value / Max) : 0.f; };

	if (HealthBar)       HealthBar->SetPercent(Normalize(Info.Health, MaxValues.Health)); TB_Health->SetText(FText::FromString(FString::FromInt(Info.Health)));
	if (ArmorBar)        ArmorBar->SetPercent(Normalize(Info.Armor, MaxValues.Armor)); TB_Armor->SetText(FText::FromString(FString::FromInt(Info.Armor)));
	if (DexterityBar)    DexterityBar->SetPercent(Normalize(Info.Dexterity, MaxValues.Dexterity)); TB_Dexterity->SetText(FText::FromString(FString::FromInt(Info.Dexterity)));
	if (StrengthBar)     StrengthBar->SetPercent(Normalize(Info.Strength, MaxValues.Strength)); TB_Strength->SetText(FText::FromString(FString::FromInt(Info.Strength)));
	if (IntelligenceBar) IntelligenceBar->SetPercent(Normalize(Info.Intelligence, MaxValues.Intelligence)); TB_Intelligence->SetText(FText::FromString(FString::FromInt(Info.Intelligence)));

	APlayerController* PC = GetOwningPlayer();
	if (PC)
	{
		ARelicRunnersPlayerState* PS = PC->GetPlayerState<ARelicRunnersPlayerState>();
		if (PS)
		{
			if (ARelicRunnersPlayerController* RPC = Cast<ARelicRunnersPlayerController>(PC))
			{
				RPC->Server_SetSelectedClass(ClassKey);
			}
		}
	}

	URelicRunnersGameInstance* GameInstance = Cast<URelicRunnersGameInstance>(GetGameInstance());
	if (GameInstance)
	{
		GameInstance->SetSelectedClass(ClassKey);
	}
}

void UJoinUserWidget::HandleEntryGenerated(UUserWidget& EntryWidget)
{
	if (USessionListItemWidget* SessionWidget = Cast<USessionListItemWidget>(&EntryWidget))
	{
		SessionWidget->OnSessionClicked.BindUObject(this, &UJoinUserWidget::HandleSessionClicked);
	}
}

void UJoinUserWidget::SetParentMenu(UMainMenuWidget* InParentMenu)
{
	ParentMenu = InParentMenu;
}

void UJoinUserWidget::RefreshMenu()
{
	ShowJoinButton(false);
	SearchForLanGames();
}

void UJoinUserWidget::BackButtonClicked()
{
	PlayButtonClickSound();

	URelicRunnersGameInstance* GameInstance = GetGameInstance<URelicRunnersGameInstance>();
	if (GameInstance)
	{
		GameInstance->BackToMainMenu();
	}
}

void UJoinUserWidget::FindGamesButtonClicked()
{
	PlayButtonClickSound();

	URelicRunnersGameInstance* GameInstance = GetGameInstance<URelicRunnersGameInstance>();
	if (!GameInstance) return;

	SearchForLanGames();
}

void UJoinUserWidget::LeaveGameButtonClicked()
{
	PlayButtonClickSound();

	URelicRunnersGameInstance* GameInstance = GetGameInstance<URelicRunnersGameInstance>();
	if (!GameInstance) return;

	GameInstance->HostGame();
}

void UJoinUserWidget::JoinGameButtonClicked()
{
	PlayButtonClickSound();

	if (LastSelectedItem && SessionTileView)
	{
		int32 ItemIndex = SessionTileView->GetIndexForItem(LastSelectedItem);
		URelicRunnersGameInstance* GameInstance = GetGameInstance<URelicRunnersGameInstance>();
		if (GameInstance)
		{
			GameInstance->JoinGame(ItemIndex);
		}
	}
}

void UJoinUserWidget::StartGameButtonClicked()
{
	PlayButtonClickSound();

	APlayerController* PC = GetOwningPlayer();
	if (!PC) return;

	ALobbyGameMode* GM = GetWorld()->GetAuthGameMode<ALobbyGameMode>();
	if (!GM) return;

	if (!IsCountingDown)
	{
		GM->StartCountdown();
		IsCountingDown = true;

		if (TB_StartButton) TB_StartButton->SetText(FText::FromString("CANCEL"));
		StartButton->SetBackgroundColor(FLinearColor(1,0.3,0));
	}
	else
	{
		GM->CancelCountdown();
		IsCountingDown = false;

		if (TB_StartButton) TB_StartButton->SetText(FText::FromString("START"));
		StartButton->SetBackgroundColor(FLinearColor(0, 1, 0.714));
	}
}

void UJoinUserWidget::UpdateCountdownUI(int RemainingTime) {
	if (!TB_Countdown) return;

	if (RemainingTime > 0) {
		TB_Countdown->SetText(FText::AsNumber(RemainingTime));
		PlayAnimation(Timer);
		TimerImage->SetOpacity(1);
	}
	else
	{
		StopAnimation(Timer);
		TimerImage->SetOpacity(0);
	}
}

void UJoinUserWidget::HandleSessionClicked(USessionListItemData* ClickedSession)
{
	PlayButtonClickSound();

	LastSelectedItem = ClickedSession;
	ShowJoinButton(true);

	if (SessionTileView)
	{
		for (UObject* Item : SessionTileView->GetListItems())
		{
			if (USessionListItemWidget* Entry = Cast<USessionListItemWidget>(SessionTileView->GetEntryWidgetFromItem(Item)))
			{
				Entry->SetSelected(Item == LastSelectedItem);
			}
		}
	}
}

void UJoinUserWidget::SearchForLanGames()
{
	if (!IsValid(this) || !SessionTileView) return;

	URelicRunnersGameInstance* GameInstance = Cast<URelicRunnersGameInstance>(GetGameInstance());
	if (GameInstance)
	{
		if (FindGames && SessionTileView)
		{
			FindGames->SetIsEnabled(false);
			SessionTileView->ClearListItems();
			SessionTileView->RequestRefresh();
			GameInstance->FindGames(this);
			if (RefreshProgress)
			{
				RefreshProgress->SetVisibility(ESlateVisibility::Visible);
			}
			ShowJoinButton(false);
		}
	}
}

void UJoinUserWidget::ShowJoinButton(bool IsEnabled)
{
	if (JoinButton)
	{
		if (IsEnabled)
		{
			JoinButton->SetIsEnabled(true);
			JoinButton->SetVisibility(ESlateVisibility::Visible);
			JoinBorder->SetVisibility(ESlateVisibility::Visible);
		}
		else
		{
			JoinButton->SetIsEnabled(false);
			JoinButton->SetVisibility(ESlateVisibility::Collapsed);
			JoinBorder->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
}

void UJoinUserWidget::OnFindSessionsComplete(FString Str, int PlayerCount)
{
	if (!IsValid(this) || !SessionTileView) return;

	USessionListItemData* NewSessionItem = NewObject<USessionListItemData>();
	if (NewSessionItem)
	{
		NewSessionItem->SessionName = Str;
		NewSessionItem->SessionPlayerCount = PlayerCount;
	}

	if (SessionTileView)
	{
		SessionTileView->AddItem(NewSessionItem);

		if (USessionListItemWidget* Entry = Cast<USessionListItemWidget>(SessionTileView->GetEntryWidgetFromItem(NewSessionItem)))
		{
			Entry->OnSessionClicked.BindUObject(this, &UJoinUserWidget::HandleSessionClicked);
		}

	}

	EnableRefresh();
}


void UJoinUserWidget::EnableRefresh()
{
	if (!IsValid(this) || !SessionTileView) return;

	if (FindGames)
	{
		FindGames->SetIsEnabled(true);
		RefreshProgress->SetVisibility(ESlateVisibility::Visible);
	}

	if (RefreshProgress)
	{
		RefreshProgress->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UJoinUserWidget::ApplyColorLUT(UTexture* LUT)
{

	if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
	{
		if (APlayerCameraManager* CameraManager = PC->PlayerCameraManager)
		{
			FPostProcessSettings& PPS = CameraManager->ViewTarget.POV.PostProcessSettings;

			PPS.bOverride_ColorGradingLUT = true;
			PPS.ColorGradingLUT = LUT;

			PPS.bOverride_ColorGradingIntensity = true;

			if (URelicRunnersGameInstance* GI = Cast<URelicRunnersGameInstance>(GetGameInstance()))
			{
				PPS.ColorGradingIntensity = (GI->ColorblindMode == EColorblindMode::Normal) ? 0.0f : 1.f;
			}
			else
			{
				PPS.ColorGradingIntensity = 0.0f;
			}

			CameraManager->ViewTarget.POV.PostProcessBlendWeight = 1.0f;
		}
	}

}
void UJoinUserWidget::HandleColorModeChanged()
{
	URelicRunnersGameInstance* GI = Cast<URelicRunnersGameInstance>(GetGameInstance());
	if (!GI) return;

	if (GI->ColorblindMode == EColorblindMode::Normal)
	{
		ApplyColorLUT(LUT_Normal);
	}
	if (GI->ColorblindMode == EColorblindMode::Tritanopia)
	{
		ApplyColorLUT(LUT_Tritanopia);
	}
	
}

void UJoinUserWidget::ApplyColorblindStyle()
{
	URelicRunnersGameInstance* GI = Cast<URelicRunnersGameInstance>(GetGameInstance());
	if (!GI) return;

	FLinearColor PrimaryAccent;
	FLinearColor SecondaryAccent;
	FLinearColor Highlight;
	FLinearColor TextColor = FLinearColor::White;
	FLinearColor PanelColor;
	FLinearColor OutlineColor;


	FLinearColor DamageIconColor;
	FLinearColor DefenceIconColor;
	FLinearColor UtilityIconColor;
	FLinearColor UltimateIconColor;

	FLinearColor HealthIconColor;
	FLinearColor DIconColor;
	FLinearColor StrengthIconColor;
	FLinearColor AgilityIconColor;
	FLinearColor IntelligenceIconColor;

	switch (GI->ColorblindMode)
	{
	case EColorblindMode::Protanopia:
		PrimaryAccent = FLinearColor(1.0f, 0.55f, 0.0f, 1.0f);
		SecondaryAccent = FLinearColor(0.0f, 0.75f, 1.0f, 1.0f);
		Highlight = FLinearColor(1.0f, 1.0f, 0.35f, 1.0f);
		PanelColor = FLinearColor(0.18f, 0.18f, 0.18f, 1.0f);
		OutlineColor = FLinearColor(1.0f, 0.55f, 0.0f, 1.0f);

		DamageIconColor = FLinearColor(0.0f, 0.85f, 1.0f, 1.0f);
		DefenceIconColor = FLinearColor(0.0f, 1.0f, 0.25f, 1.0f);
		UtilityIconColor = FLinearColor(0.6f, 0.3f, 1.0f, 1.0f);
		UltimateIconColor = FLinearColor(1.0f, 1.0f, 0.0f, 1.0f);

		HealthIconColor = FLinearColor(1.0f, 0.8f, 0.0f, 1.0f);
		DIconColor = FLinearColor(0.0f, 1.0f, 0.25f, 1.0f);
		StrengthIconColor = FLinearColor(0.0f, 0.85f, 1.0f, 1.0f);
		AgilityIconColor = FLinearColor(1.0f, 1.0f, 0.0f, 1.0f);
		IntelligenceIconColor = FLinearColor(0.6f, 0.3f, 1.0f, 1.0f);
		break;

	case EColorblindMode::Deuteranopia:
		PrimaryAccent = FLinearColor(1.0f, 0.6f, 0.0f, 1.0f);
		SecondaryAccent = FLinearColor(0.15f, 0.8f, 1.0f, 1.0f);
		Highlight = FLinearColor(1.0f, 0.95f, 0.45f, 1.0f);
		PanelColor = FLinearColor(0.18f, 0.18f, 0.18f, 1.0f);
		OutlineColor = FLinearColor(1.0f, 0.6f, 0.0f, 1.0f);

		DamageIconColor = FLinearColor(0.0f, 0.85f, 1.0f, 1.0f);
		DefenceIconColor = FLinearColor(0.2f, 1.0f, 0.2f, 1.0f);
		UtilityIconColor = FLinearColor(0.65f, 0.35f, 1.0f, 1.0f);
		UltimateIconColor = FLinearColor(1.0f, 1.0f, 0.0f, 1.0f);

		HealthIconColor = FLinearColor(1.0f, 0.75f, 0.0f, 1.0f);
		DIconColor = FLinearColor(0.2f, 1.0f, 0.2f, 1.0f);
		StrengthIconColor = FLinearColor(0.0f, 0.85f, 1.0f, 1.0f);
		AgilityIconColor = FLinearColor(1.0f, 1.0f, 0.0f, 1.0f);
		IntelligenceIconColor = FLinearColor(0.65f, 0.35f, 1.0f, 1.0f);
		break;

	case EColorblindMode::Tritanopia:
		PrimaryAccent = FLinearColor(0.1f, 1.0f, 0.35f, 1.0f);
		SecondaryAccent = FLinearColor(1.0f, 0.35f, 0.35f, 1.0f);
		Highlight = FLinearColor(1.0f, 1.0f, 1.0f, 1.0f);
		PanelColor = FLinearColor(0.16f, 0.16f, 0.16f, 1.0f);
		OutlineColor = FLinearColor(0.1f, 1.0f, 0.35f, 1.0f);

		DamageIconColor = FLinearColor(1.0f, 0.35f, 0.35f, 1.0f);
		DefenceIconColor = FLinearColor(0.1f, 1.0f, 0.35f, 1.0f);
		UtilityIconColor = FLinearColor(0.85f, 0.45f, 1.0f, 1.0f);
		UltimateIconColor = FLinearColor(1.0f, 1.0f, 1.0f, 1.0f);

		HealthIconColor = FLinearColor(1.0f, 0.35f, 0.6f, 1.0f);
		DIconColor = FLinearColor(0.1f, 1.0f, 0.35f, 1.0f);
		StrengthIconColor = FLinearColor(1.0f, 0.35f, 0.35f, 1.0f);
		AgilityIconColor = FLinearColor(0.1f, 1.0f, 0.35f, 1.0f);
		IntelligenceIconColor = FLinearColor(0.85f, 0.45f, 1.0f, 1.0f);
		break;

	case EColorblindMode::Normal:
	default:
		PrimaryAccent = FLinearColor(0.38f, 0.24f, 0.12f, 1.0f);
		SecondaryAccent = FLinearColor(0.28f, 0.18f, 0.10f, 1.0f);
		Highlight = FLinearColor(0.85f, 0.65f, 0.25f, 1.0f);
		PanelColor = FLinearColor(0.16f, 0.12f, 0.08f, 1.0f);
		OutlineColor = FLinearColor(0.55f, 0.38f, 0.18f, 1.0f);

		DamageIconColor = FLinearColor(0.462f, 0.0f, 1.0f, 1.0f);
		DefenceIconColor = FLinearColor(0.0f, 1.0f, 0.14f, 1.0f);
		UtilityIconColor = FLinearColor(0.0f, 0.55f, 1.0f, 1.0f);
		UltimateIconColor = FLinearColor(1.0f, 0.82f, 0.0f, 1.0f);

		HealthIconColor = FLinearColor(1.0f, 0.0f, 0.02f, 1.0f);
		DIconColor = FLinearColor(0.0f, 1.0f, 0.03f, 1.0f);
		StrengthIconColor = FLinearColor(0.46f, 0.0f, 1.0f, 1.0f);
		AgilityIconColor = FLinearColor(1.0f, 0.85f, 0.0f, 1.0f);
		IntelligenceIconColor = FLinearColor(0.0f, 0.55f, 1.0f, 1.0f);
		break;
	}


	SetButtonColor(StartButton, PrimaryAccent);
	SetButtonColor(JoinButton, PrimaryAccent);
	SetButtonColor(FindGames, SecondaryAccent);
	SetButtonColor(LeaveGameButton, SecondaryAccent);
	SetButtonColor(BackButton, SecondaryAccent);


	SetButtonColor(AresButton, SecondaryAccent);
	SetButtonColor(ArtemisButton, SecondaryAccent);
	SetButtonColor(AphroditeButton, SecondaryAccent);
	SetButtonColor(NemesisButton, SecondaryAccent);

	SetButtonTint(B_Boar, SecondaryAccent);
	SetButtonTint(B_Bull, SecondaryAccent);
	SetButtonTint(B_Lion, SecondaryAccent);
	SetButtonTint(B_Hydra, SecondaryAccent);
	SetButtonTint(B_Cerberus, SecondaryAccent);

	SetButtonOutlineColor(B_Boar, OutlineColor);
	SetButtonOutlineColor(B_Bull, OutlineColor);
	SetButtonOutlineColor(B_Lion, OutlineColor);
	SetButtonOutlineColor(B_Hydra, OutlineColor);
	SetButtonOutlineColor(B_Cerberus, OutlineColor);

	SetBorderColor(BoarBorder, PanelColor);
	SetBorderColor(BullBorder, PanelColor);
	SetBorderColor(LionBorder, PanelColor);
	SetBorderColor(HydraBorder, PanelColor);
	SetBorderColor(CerberusBorder, PanelColor);

	SetBorderColor(B_BoarSection, PanelColor);
	SetBorderColor(B_BullSection, PanelColor);
	SetBorderColor(B_LionSection, PanelColor);
	SetBorderColor(B_HydraSection, PanelColor);
	SetBorderColor(B_CerberusSection, PanelColor);
	SetBorderColor(B_CurrentSection, PanelColor);

	SetImageColor(I_Ares, Highlight);
	SetImageColor(I_Artemis, Highlight);
	SetImageColor(I_Aphrodite, Highlight);
	SetImageColor(I_Nemesis, Highlight);

	SetImageColor(I_Boar, Highlight);
	SetImageColor(I_Bull, Highlight);
	SetImageColor(I_Lion, Highlight);
	SetImageColor(I_Hydra, Highlight);
	SetImageColor(I_Cerberus, Highlight);
	SetImageColor(I_CurrentDifficulty, Highlight);

	if (TB_ClassName) TB_ClassName->SetColorAndOpacity(FSlateColor(TextColor));
	if (TB_ClassRole) TB_ClassRole->SetColorAndOpacity(FSlateColor(TextColor));
	if (TB_Ability1) TB_Ability1->SetColorAndOpacity(FSlateColor(TextColor));
	if (TB_Ability2) TB_Ability2->SetColorAndOpacity(FSlateColor(TextColor));
	if (TB_Ability3) TB_Ability3->SetColorAndOpacity(FSlateColor(TextColor));
	if (TB_Ability4) TB_Ability4->SetColorAndOpacity(FSlateColor(TextColor));

	if (TB_BoarHealth) TB_BoarHealth->SetColorAndOpacity(FSlateColor(TextColor));
	if (TB_BoarDamage) TB_BoarDamage->SetColorAndOpacity(FSlateColor(TextColor));
	if (TB_BoarLuck) TB_BoarLuck->SetColorAndOpacity(FSlateColor(TextColor));

	if (TB_BullHealth) TB_BullHealth->SetColorAndOpacity(FSlateColor(TextColor));
	if (TB_BullDamage) TB_BullDamage->SetColorAndOpacity(FSlateColor(TextColor));
	if (TB_BullLuck) TB_BullLuck->SetColorAndOpacity(FSlateColor(TextColor));

	if (TB_LionHealth) TB_LionHealth->SetColorAndOpacity(FSlateColor(TextColor));
	if (TB_LionDamage) TB_LionDamage->SetColorAndOpacity(FSlateColor(TextColor));
	if (TB_LionLuck) TB_LionLuck->SetColorAndOpacity(FSlateColor(TextColor));

	if (TB_HydraHealth) TB_HydraHealth->SetColorAndOpacity(FSlateColor(TextColor));
	if (TB_HydraDamage) TB_HydraDamage->SetColorAndOpacity(FSlateColor(TextColor));
	if (TB_HydraLuck) TB_HydraLuck->SetColorAndOpacity(FSlateColor(TextColor));

	if (TB_CerberusHealth) TB_CerberusHealth->SetColorAndOpacity(FSlateColor(TextColor));
	if (TB_CerberusDamage) TB_CerberusDamage->SetColorAndOpacity(FSlateColor(TextColor));
	if (TB_CerberusLuck) TB_CerberusLuck->SetColorAndOpacity(FSlateColor(TextColor));

	if (TB_CurrentDifficulty) TB_CurrentDifficulty->SetColorAndOpacity(FSlateColor(TextColor));
	if (TB_CurrentEnemyDamage) TB_CurrentEnemyDamage->SetColorAndOpacity(FSlateColor(TextColor));
	if (TB_CurrentEnemyHealth) TB_CurrentEnemyHealth->SetColorAndOpacity(FSlateColor(TextColor));
	if (TB_CurrentLuck) TB_CurrentLuck->SetColorAndOpacity(FSlateColor(TextColor));
	if (TB_Countdown) TB_Countdown->SetColorAndOpacity(FSlateColor(Highlight));

	if (DamageIcon) DamageIcon->SetColorAndOpacity(DamageIconColor * 1.5f);
	if (DefenceIcon) DefenceIcon->SetColorAndOpacity(DefenceIconColor * 1.5f);
	if (UtilityIcon) UtilityIcon->SetColorAndOpacity(UtilityIconColor * 1.5f);
	if (UltimateIcon) UltimateIcon->SetColorAndOpacity(UltimateIconColor * 1.5f);

	if (HealthIcon) HealthIcon->SetColorAndOpacity(HealthIconColor);
	if (DIcon) DIcon->SetColorAndOpacity(DIconColor * 1.5f);
	if (StrengthIcon) StrengthIcon->SetColorAndOpacity(StrengthIconColor * 1.5f);
	if (AgilityIcon)  AgilityIcon->SetColorAndOpacity(AgilityIconColor);
	if (IntelligenceIcon) IntelligenceIcon->SetColorAndOpacity(IntelligenceIconColor);

	SetSelectedClass(CurrentClass);

	ARelicRunnersGameState* RunnersGameState = GetWorld() ? GetWorld()->GetGameState<ARelicRunnersGameState>() : nullptr;
	if (RunnersGameState)
	{
		SetSelectedDifficulty(RunnersGameState->CurrentDifficulty.IsNone() ? FName("Boar") : RunnersGameState->CurrentDifficulty);
	}
}