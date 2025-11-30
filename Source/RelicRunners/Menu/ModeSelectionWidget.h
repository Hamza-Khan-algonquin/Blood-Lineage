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
	void HighlightSelectedSlot(class USaveFileListWidget* NewSelection);
	UPROPERTY(BlueprintReadWrite)
	class USaveFileListWidget* CurrentSelectedSlot = nullptr;
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
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<class USaveFilePopupWidget> SaveFilePopupClass;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save System")
	TSubclassOf<class UEditSaveFilePopupWidget> EditPopupClass;

	void SetParentMenu(class UMainMenuWidget* InParentMenu);

	void ResetUI();
	UFUNCTION(BlueprintCallable)
	void OpenSaveFilePopupWidget(class USaveFileListWidget* TargetSlot);
	UFUNCTION(BlueprintCallable)
	void PromptCreateSaveFile();
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


	int32 MaxCharacterLimit;
};
