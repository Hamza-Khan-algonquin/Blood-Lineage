// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "JoinUserWidget.generated.h"

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


class FOnlineSessionSearchResult;

UCLASS()
class RELICRUNNERS_API UJoinUserWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	void NativeConstruct() override;

	virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	void ShowTooltipForBorder(class UBorder* HoveredBorder, const FVector2D& CursorPos, const FGeometry& ParentGeometry);
	class UBorder* CurrentHoveredBorder;

	void SetButtonColor(class UButton* Button, const FLinearColor& Color);

	void SetBorderColor(class UBorder* Border, const FLinearColor& Color);

	void SetImageColor(class UImage* Image, const FLinearColor& Color);

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
	class UImage* I_Ares = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UImage* I_Artemis = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UImage* I_Aphrodite = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UImage* I_Nemesis = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UButton* BackButton = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UButton* FindGames = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UButton* JoinButton = nullptr;

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

public:
	void OnFindSessionsComplete(FString Str, int PlayerCount);

	void EnableRefresh();

	void SetParentMenu(class UMainMenuWidget* InParentMenu);

	void RefreshMenu();
};
