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

	if (const UWorld* World = GetWorld())
	{
		//If not host then collapse these elements
		bool IsHost = World->GetAuthGameMode() != nullptr;
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

	SetSelectedDifficulty("Boar");

	PlayAnimation(LeftPanelOpen);
	PlayAnimation(RightPanelOpen);
}

FReply UJoinUserWidget::NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseMove(InGeometry, InMouseEvent);

	FVector2D CursorPos = InMouseEvent.GetScreenSpacePosition();

	TArray<UBorder*> Borders = { B_Health, B_Defence, B_Strength, B_Agility, B_Intelligence,
								 B_Ability1, B_Ability2, B_Ability3, B_Ability4 };

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

	return FReply::Handled();
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
	FString AbilityName = "";
	FString AbilityDesc = "";

	if (HoveredBorder == B_Health)				{ StatKey = "Health";       RawValue = Info.Health; }
	else if (HoveredBorder == B_Defence)		{ StatKey = "Defence";      RawValue = Info.Armor; }
	else if (HoveredBorder == B_Strength)		{ StatKey = "Strength";     RawValue = Info.Strength; }
	else if (HoveredBorder == B_Agility)		{ StatKey = "Agility";      RawValue = Info.Dexterity; }
	else if (HoveredBorder == B_Intelligence)	{ StatKey = "Intelligence"; RawValue = Info.Intelligence; }

	else if (HoveredBorder == B_Ability1)		{ StatKey = "Ability1"; AbilityName = Info.Ability1; AbilityDesc = Info.Ability1Description1; }
	else if (HoveredBorder == B_Ability2)		{ StatKey = "Ability2"; AbilityName = Info.Ability2; AbilityDesc = Info.Ability2Description1; }
	else if (HoveredBorder == B_Ability3)		{ StatKey = "Ability3"; AbilityName = Info.Ability3; AbilityDesc = Info.Ability3Description1; }
	else if (HoveredBorder == B_Ability4)		{ StatKey = "Ability4"; AbilityName = Info.Ability4; AbilityDesc = Info.Ability4Description1; }

	FTooltipData TD = ItemStats::GetStatTooltipData(StatKey, RawValue, 0, 0, AbilityName, AbilityDesc);

	StandardTooltip->SetIcon(TD.Icon, TD.IconColor);
	StandardTooltip->Setup(TD.Name, TD.Value, TD.Descriptions);

	FVector2D ViewportPosition = ParentGeometry.AbsoluteToLocal(CursorPos);
	const FVector2D Offset(5.0f, 5.0f);
	const FVector2D Anchor(0.f, 1.f);

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
	URelicRunnersGameInstance* GameInstance = Cast<URelicRunnersGameInstance>(GetGameInstance());
	if (!GameInstance) return;

	const FLinearColor DarkGold(0.6f, 0.3f, 0.f, 1.f);
	FString IconName = GameInstance->CurrentDifficulty.ToString() + "Difficulty";
	I_CurrentDifficulty->SetBrushFromTexture(ItemStats::LoadIcon(IconName));
	I_CurrentDifficulty->SetColorAndOpacity(DarkGold);
	TB_CurrentDifficulty->SetText(FText::FromName(GameInstance->CurrentDifficulty));
	TB_CurrentEnemyDamage->SetText(FText::AsNumber(DifficultyMap[GameInstance->CurrentDifficulty].HealthPercent));
	TB_CurrentEnemyHealth->SetText(FText::AsNumber(DifficultyMap[GameInstance->CurrentDifficulty].DamagePercent));
	TB_CurrentLuck->SetText(FText::AsNumber(DifficultyMap[GameInstance->CurrentDifficulty].LuckAmount));
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

	if (ALobbyGameMode* GM = GetWorld()->GetAuthGameMode<ALobbyGameMode>()) 
	{
		GM->StartCountdown();
	}
}

void UJoinUserWidget::UpdateCountdownUI(int RemainingTime) {
	if (!TB_Countdown) return;

	if (RemainingTime > 0) {
		TB_Countdown->SetText(FText::AsNumber(RemainingTime));
		PlayAnimation(Timer);
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