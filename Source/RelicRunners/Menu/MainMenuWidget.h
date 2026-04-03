#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MainMenuWidget.generated.h"

/**
 *
 */
UCLASS()
class RELICRUNNERS_API UMainMenuWidget : public UUserWidget
{
    GENERATED_BODY()
protected:
    virtual void NativeConstruct() override;
public:
    class UModeSelectionWidget* ModeSelectionWidget;
    class UJoinUserWidget* JoinGameWidget;
    class USettingsWidget* SettingsWidget;
    class UUpgradesWidget* UpgradesWidget;
    class UCreditsWidget* CreditsWidget;

    //need to be fixed later 
    class UJoinUserWidget* TutorialWidget;

public:
    UPROPERTY(meta = (BindWidget))
    class UWidgetSwitcher* WidgetSwitcher;

    UFUNCTION(BlueprintCallable)
    class UModeSelectionWidget* GetModeSelectionWidget() { return ModeSelectionWidget; }

    UFUNCTION(BlueprintCallable)
    void ShowModeSelectionWidget();

    UFUNCTION(BlueprintCallable)
    void ShowSessionsWidget();

    UFUNCTION(BlueprintCallable)
    void ShowUpgradesWidget();

    UFUNCTION(BlueprintCallable)
    void ShowSettingsWidget();

    UFUNCTION(BlueprintCallable)
    void ShowTutorialWidget();

    UFUNCTION(BlueprintCallable)
    void ShowCreditsWidget();

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void PlayButtonClickSound();


    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
    TSubclassOf<UUserWidget> ModeSelectionUserWidgetClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
    TSubclassOf<UUserWidget> JoinGameUserWidgetClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
    TSubclassOf<UUserWidget> CreditsWidgetClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
    TSubclassOf<UUserWidget> UpgradesWidgetClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
    TSubclassOf<UUserWidget> SettingsWidgetClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    USoundBase* ButtonClickSound = nullptr;


};
