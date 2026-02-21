#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Keybinds.generated.h"

USTRUCT(BlueprintType)
struct FKeybinding
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Keybinding")
    FString Name;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Keybinding")
    FKey Bind;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Keybinding")
    FString ReadableBind;
};

UCLASS(Blueprintable, BlueprintType)
class RELICRUNNERS_API UKeybinds : public UObject
{
    GENERATED_BODY()

public:
    UKeybinds();

    FKey GetKeyForAction(const FString& ActionName) const;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
    TArray<FKeybinding> KeyBinds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
    bool InvertedXMouse = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
    bool InvertedYMouse = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|Mouse Sens")
    float HorizSens = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|Mouse Sens")
    float VertSens = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|Brightness")
    float Brightness = 0.0f;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    bool motionBlurEnabled = true;
};

static FString GetReadableKeyName(const FKey& Key)
{
    FString Name = Key.GetDisplayName().ToString();

    static TMap<FString, FString> KeyReplacements = {
        {TEXT("One"), TEXT("1")},
        {TEXT("Two"), TEXT("2")},
        {TEXT("Three"), TEXT("3")},
        {TEXT("Four"), TEXT("4")},
        {TEXT("Five"), TEXT("5")},
        {TEXT("Six"), TEXT("6")},
        {TEXT("Seven"), TEXT("7")},
        {TEXT("Eight"), TEXT("8")},
        {TEXT("Nine"), TEXT("9")},
        {TEXT("Zero"), TEXT("0")},
        {TEXT("Left Bracket"), TEXT("[")},
        {TEXT("Right Bracket"), TEXT("]")},
        {TEXT("Semicolon"), TEXT(";")},
        {TEXT("Apostrophe"), TEXT("'")},
        {TEXT("Comma"), TEXT(",")},
        {TEXT("Period"), TEXT(".")},
        {TEXT("Slash"), TEXT("/")},
        {TEXT("Backslash"), TEXT("\\")},
        {TEXT("Minus"), TEXT("-")},
        {TEXT("Equals"), TEXT("=")},
        {TEXT("Grave"), TEXT("~")},
        {TEXT("Space Bar"), TEXT("Space")},
        {TEXT("CapsLock"), TEXT("Caps")},
        {TEXT("LeftShift"), TEXT("LShift")},
        {TEXT("RightShift"), TEXT("RShift")},
        {TEXT("LeftControl"), TEXT("LCtrl")},
        {TEXT("RightControl"), TEXT("RCtrl")},
        {TEXT("LeftAlt"), TEXT("LAlt")},
        {TEXT("RightAlt"), TEXT("RAlt")},
        {TEXT("Escape"), TEXT("Esc")},
        {TEXT("Up"), TEXT("Up")},
        {TEXT("Down"), TEXT("Down")},
        {TEXT("Left"), TEXT("Left")},
        {TEXT("Right"), TEXT("Right")},
        {TEXT("Middle Mouse Button"), TEXT("MMB")},
        {TEXT("Right Mouse Button"), TEXT("RMB")},
        {TEXT("Left Mouse Button"), TEXT("LMB")},
        {TEXT("MouseWheelAxis"), TEXT("MWA")},
    };

    if (const FString* Replacement = KeyReplacements.Find(Name))
    {
        return *Replacement;
    }

    return Name;
}

