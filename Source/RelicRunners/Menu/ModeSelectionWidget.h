#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ModeSelectionWidget.generated.h"

/**
 *
 */
UCLASS()
class RELICRUNNERS_API UModeSelectionWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	void NativeConstruct() override;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UButton* HostButton = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UButton* UpgradesButton = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UButton* SettingsButton = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UButton* TutorialButton = nullptr;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UTileView* SaveFiles;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UButton* CreditsButton = nullptr;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UButton* ExitButton = nullptr;
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UEditableTextBox* NameTextBox = nullptr;

	void SetParentMenu(class UMainMenuWidget* InParentMenu);

	void ResetUI();

protected:
	UFUNCTION()
	void HostButtonClicked();

	UFUNCTION()
	void SettingsButtonClicked();
	UFUNCTION()
	void CreditsButtonClicked();
	UFUNCTION()
	void TutorialButtonClicked();
	UFUNCTION()
	void UpgradesButtonClicked();
	UFUNCTION()
	void ExitButtonClicked();
	UPROPERTY()
	class UMainMenuWidget* ParentMenu;

	UFUNCTION()
	void OnPlayerNameTextChanged(const FText& Text);

	int32 MaxCharacterLimit;
};
