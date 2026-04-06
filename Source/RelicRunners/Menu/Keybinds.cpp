
#include "Keybinds.h"

UKeybinds::UKeybinds()
{
    KeyBinds.Add({ TEXT("Walk Forward"),       EKeys::W });
    KeyBinds.Add({ TEXT("Walk Left"),          EKeys::A });
    KeyBinds.Add({ TEXT("Walk Backward"),      EKeys::S });
    KeyBinds.Add({ TEXT("Walk Right"),         EKeys::D });
    KeyBinds.Add({ TEXT("Inventory"),          EKeys::Tab });
    KeyBinds.Add({ TEXT("Ability Selection"),  EKeys::U });
    KeyBinds.Add({ TEXT("Interact"),           EKeys::E });
    KeyBinds.Add({ TEXT("Map"),                EKeys::M });
    KeyBinds.Add({ TEXT("Pause"),              EKeys::Escape });
    KeyBinds.Add({ TEXT("Ping"),               EKeys::MiddleMouseButton });
    KeyBinds.Add({ TEXT("Basic Attack"),       EKeys::LeftMouseButton });
    KeyBinds.Add({ TEXT("Aim"),                EKeys::RightMouseButton });
    KeyBinds.Add({ TEXT("Damage Ability"),     EKeys::One });
    KeyBinds.Add({ TEXT("Defence Ability"),    EKeys::Two });
    KeyBinds.Add({ TEXT("Utility Ability"),    EKeys::Three });
    KeyBinds.Add({ TEXT("Ultimate Ability"),   EKeys::Four });
    KeyBinds.Add({ TEXT("Health Potion"),      EKeys::F });
    KeyBinds.Add({ TEXT("Zoom"),               EKeys::MouseWheelAxis });
}

FKey UKeybinds::GetKeyForAction(const FString& ActionName) const
{
    for (const FKeybinding& Bind : KeyBinds)
    {
        if (Bind.Name.Equals(ActionName, ESearchCase::IgnoreCase))
        {
            return Bind.Bind;
        }
    }
    return EKeys::Invalid;
}

int UKeybinds::GetBindIndex(const FString& ActionName)
{
    int index = 0;
    for (const FKeybinding& Bind : KeyBinds)
    {
        if (Bind.Name.Equals(ActionName, ESearchCase::IgnoreCase))
        {
            return index;
        }
        index++;
    }
    return 0;
}
