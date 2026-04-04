// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Engine/PostProcessVolume.h"
#include "Components/Image.h"
#include <Components/Border.h>

#include "JoinUserWidget.generated.h"

class USoundBase;

USTRUCT()
struct FStandardTooltipData
{
	GENERATED_BODY()

	UPROPERTY()
	FText Title;

	UPROPERTY()
	FText Value;

	UPROPERTY()
	TArray<FText> Descriptions;

	UPROPERTY()
	UTexture2D* Icon = nullptr;

	UPROPERTY()
	FLinearColor IconColor = FLinearColor::White;
};

USTRUCT()
struct FDifficultyWidgets
{
	GENERATED_BODY()

	UPROPERTY()
	class UImage* Icon = nullptr;

	UPROPERTY()
	class UBorder* OuterBorder = nullptr;

	UPROPERTY()
	class UButton* Button = nullptr;

	UPROPERTY()
	class UBorder* SectionBorder = nullptr;   

	float HealthPercent;

	float DamagePercent;

	float LuckAmount;
	
};


class FOnlineSessionSearchResult;

UCLASS()
class RELICRUNNERS_API UJoinUserWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	void NativeConstruct() override;

	virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	void UpdateDifficultyNumbers();
	void ShowTooltipForBorder(class UBorder* HoveredBorder, const FVector2D& CursorPos, const FGeometry& ParentGeometry);
	class UBorder* CurrentHoveredBorder;

	UPROPERTY()
	TArray<AActor*> CachedLobbyPreviews;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* TB_Countdown;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* TB_StartButton;

	bool IsCountingDown = false;

	UPROPERTY(meta = (BindWidget))
	class UImage* TimerImage;

	UPROPERTY(Transient, meta = (BindWidgetAnim))
	class UWidgetAnimation* Timer;
	UPROPERTY(Transient, meta = (BindWidgetAnim))
	class UWidgetAnimation* RightPanelOpen;
	UPROPERTY(Transient, meta = (BindWidgetAnim))
	class UWidgetAnimation* LeftPanelOpen;
	void UpdateCountdownUI(int RemainingTime);
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI|Audio")
	USoundBase* ButtonClickSound = nullptr;

	UPROPERTY()
	TMap<FName, FDifficultyWidgets> DifficultyMap;

	UFUNCTION()
	void OnDifficultyClicked_Boar();

	UFUNCTION()
	void OnDifficultyClicked_Bull();

	UFUNCTION()
	void OnDifficultyClicked_Lion();

	UFUNCTION()
	void OnDifficultyClicked_Hydra();

	UFUNCTION()
	void OnDifficultyClicked_Cerberus();

	void SetButtonColor(class UButton* Button, const FLinearColor& Color);

	void SetBorderColor(class UBorder* Border, const FLinearColor& Color);

	void SetImageColor(class UImage* Image, const FLinearColor& Color);

	void SetBorderOutlineColor(UBorder* Border, const FLinearColor& InsideColor, const FLinearColor& OutlineColor);

	void SetButtonTint(UButton* Button, const FLinearColor& Color);

	void SetButtonOutlineColor(UButton* Button, const FLinearColor& Color);

	void SetSelectedDifficulty(FName DifficultyKey);

	void UpdateClientDifficultyUI();

	void PlayButtonClickSound();

	UFUNCTION()
	void OnAresClicked();
	UFUNCTION()
	void OnArtemisClicked();
	UFUNCTION()
	void OnAphroditeClicked();
	UFUNCTION()
	void OnNemesisClicked();

	FName CurrentClass;

	void SetSelectedClass(FName ClassKey);
	void HandleEntryGenerated(UUserWidget& EntryWidget);



	UPROPERTY(meta = (BindWidget))
	class UTextBlock* TB_BoarHealth;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* TB_BoarDamage;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* TB_BoarLuck;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* TB_BullHealth;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* TB_BullDamage;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* TB_BullLuck;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* TB_LionHealth;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* TB_LionDamage;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* TB_LionLuck;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* TB_HydraHealth;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* TB_HydraDamage;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* TB_HydraLuck;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* TB_CerberusHealth;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* TB_CerberusDamage;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* TB_CerberusLuck;

	UPROPERTY(meta = (BindWidget))
	class UImage* I_CurrentDifficulty;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* TB_CurrentDifficulty;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* TB_CurrentEnemyHealth;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* TB_CurrentEnemyDamage;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* TB_CurrentLuck;

	UPROPERTY(meta = (BindWidget))
	class UBorder* B_CurrentSection;

	UPROPERTY(meta = (BindWidget))
	class UBorder* BoarBorder;

	UPROPERTY(meta = (BindWidget))
	class UButton* B_Boar;

	UPROPERTY(meta = (BindWidget))
	class UBorder* B_BoarSection;
	UPROPERTY(meta = (BindWidget))
	class UBorder* B_BoarSection1;
	UPROPERTY(meta = (BindWidget))
	class UBorder* BullBorder;

	UPROPERTY(meta = (BindWidget))
	class UButton* B_Bull;

	UPROPERTY(meta = (BindWidget))
	class UBorder* B_BullSection;
	UPROPERTY(meta = (BindWidget))
	class UBorder* B_BullSection1;
	UPROPERTY(meta = (BindWidget))
	class UBorder* LionBorder;

	UPROPERTY(meta = (BindWidget))
	class UButton* B_Lion;

	UPROPERTY(meta = (BindWidget))
	class UBorder* B_LionSection;
	UPROPERTY(meta = (BindWidget))
	class UBorder* B_LionSection1;
	UPROPERTY(meta = (BindWidget))
	class UBorder* HydraBorder;

	UPROPERTY(meta = (BindWidget))
	class UButton* B_Hydra;

	UPROPERTY(meta = (BindWidget))
	class UBorder* B_HydraSection;
	UPROPERTY(meta = (BindWidget))
	class UBorder* B_HydraSection1;
	UPROPERTY(meta = (BindWidget))
	class UBorder* CerberusBorder;

	UPROPERTY(meta = (BindWidget))
	class UButton* B_Cerberus;

	UPROPERTY(meta = (BindWidget))
	class UBorder* B_CerberusSection;
	UPROPERTY(meta = (BindWidget))
	class UBorder* B_CerberusSection1;

	UPROPERTY(meta = (BindWidget))
	class UBorder* B_CurrentSection1;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UBorder* B_Start = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UBorder* B_Ability1 = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UBorder* B_Ability2 = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UBorder* B_Ability3 = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UBorder* B_Ability4 = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UBorder* B_Health = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UBorder* B_Defence = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UBorder* B_Strength = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UBorder* B_Agility = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UBorder* B_Intelligence = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UImage* I_Boar = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UImage* I_Bull = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UImage* I_Lion = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UImage* I_Hydra = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UImage* I_Cerberus = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UImage* I_Ares = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UImage* I_Artemis = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UImage* I_Aphrodite = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UImage* I_Nemesis = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UImage* I_Ability1 = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UImage* I_Ability2 = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UImage* I_Ability3 = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UImage* I_Ability4 = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UButton* BackButton = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UButton* FindGames = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UButton* JoinButton = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UButton* LeaveGameButton = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UBorder* JoinBorder = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UBorder* SessionsBorder = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UBorder* B_Ares = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UBorder* B_Artemis = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UBorder* B_Aphrodite = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UBorder* B_Nemesis = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UCircularThrobber* RefreshProgress = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UProgressBar* HealthBar = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UProgressBar* ArmorBar = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UProgressBar* DexterityBar = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UProgressBar* StrengthBar = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UProgressBar* IntelligenceBar = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UTextBlock* TB_ClassName = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UTextBlock* TB_ClassRole = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UTextBlock* TB_Ability1 = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UTextBlock* TB_Ability2 = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UTextBlock* TB_Ability3 = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UTextBlock* TB_Ability4 = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UButton* AresButton = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UButton* ArtemisButton = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UButton* AphroditeButton = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UButton* NemesisButton = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Class")
	class UClassInfo* ClassDataAsset = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UButton* StartButton = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UTextBlock* TB_Health = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UTextBlock* TB_Armor = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UTextBlock* TB_Dexterity = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UTextBlock* TB_Strength = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UTextBlock* TB_Intelligence = nullptr;

protected:
	UFUNCTION()
	void BackButtonClicked();
	UFUNCTION()
	void FindGamesButtonClicked();
	UFUNCTION()
	void LeaveGameButtonClicked();

	UFUNCTION()
	void JoinGameButtonClicked();
	UFUNCTION()
	void StartGameButtonClicked();
	UFUNCTION()
	void HandleSessionClicked(class USessionListItemData* ClickedSession);

	void SearchForLanGames();

	UPROPERTY(meta = (BindWidget))

	class UTileView* SessionTileView;

	UObject* LastSelectedItem = nullptr;
	void ShowJoinButton(bool IsEnabled);

	UPROPERTY()
	class UMainMenuWidget* ParentMenu;

	UPROPERTY(EditDefaultsOnly, Category = "Tooltip")
	TSubclassOf<class UStandardTooltip> StandardTooltipClass;

	UPROPERTY()
	class UStandardTooltip* StandardTooltip;

	FTimerHandle TimerHandle_DifficultyUpdate;

public:
	void OnFindSessionsComplete(FString Str, int PlayerCount);

	void EnableRefresh();

	void SetParentMenu(class UMainMenuWidget* InParentMenu);

	void RefreshMenu();

	UPROPERTY(meta = (BindWidget))
	UImage* DamageIcon;

	UPROPERTY(meta = (BindWidget))
	UImage* DefenceIcon;

	UPROPERTY(meta = (BindWidget))
	UImage* UtilityIcon;

	UPROPERTY(meta = (BindWidget))
	UImage* UltimateIcon;

	UPROPERTY(meta = (BindWidget))
	UImage* HealthIcon;

	UPROPERTY(meta = (BindWidget))
	UImage* DIcon;

	UPROPERTY(meta = (BindWidget))
	UImage* StrengthIcon;

	UPROPERTY(meta = (BindWidget))
	UImage* AgilityIcon;

	UPROPERTY(meta = (BindWidget))
	UImage* IntelligenceIcon;

};
