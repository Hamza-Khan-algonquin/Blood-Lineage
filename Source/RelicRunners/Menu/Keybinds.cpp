
#include "Keybinds.h"

UKeybinds::UKeybinds()
{
    KeyBinds.Add({ TEXT("Walk Forward"),       EKeys::W });
    KeyBinds.Add({ TEXT("Walk Left"),          EKeys::A });
    KeyBinds.Add({ TEXT("Walk Backward"),      EKeys::S });
    KeyBinds.Add({ TEXT("Walk Right"),         EKeys::D });
    KeyBinds.Add({ TEXT("Jump"),               EKeys::SpaceBar });
    KeyBinds.Add({ TEXT("Inventory"),          EKeys::Tab });
    KeyBinds.Add({ TEXT("Ability Selection"),  EKeys::U });
    KeyBinds.Add({ TEXT("Interact"),           EKeys::E });
    KeyBinds.Add({ TEXT("Map"),                EKeys::M });
    KeyBinds.Add({ TEXT("Pause"),              EKeys::Escape });
    KeyBinds.Add({ TEXT("Ping"),               EKeys::MiddleMouseButton });
    KeyBinds.Add({ TEXT("Basic Attack"),       EKeys::LeftMouseButton });
    KeyBinds.Add({ TEXT("Heavy Attack"),       EKeys::RightMouseButton });
    KeyBinds.Add({ TEXT("Damage Ability"),     EKeys::One });
    KeyBinds.Add({ TEXT("Defence Ability"),    EKeys::Two });
    KeyBinds.Add({ TEXT("Utility Ability"),    EKeys::Three });
    KeyBinds.Add({ TEXT("Ultimate Ability"),   EKeys::Four });
    KeyBinds.Add({ TEXT("Health Potion"),      EKeys::H });
    KeyBinds.Add({ TEXT("Zoom"),               EKeys::MouseWheelAxis });
    KeyBinds.Add({ TEXT("Drag Map"),           EKeys::LeftMouseButton });
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