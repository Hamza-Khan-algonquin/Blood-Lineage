/************************************************************************************
 * Author:   Tristan Anglin
 * Year:     2025
 *
 * Notice:
 *   This source code is the intellectual property of Tristan Anglin.
 *   It is provided solely for use within:
 *     - The Relic Runners personal project
 *     - The Almost A Dozen project
 *
 *   Any use, distribution, or modification outside of these projects
 *   is strictly prohibited without explicit written permission.
 *
 *   � 2025 Tristan Anglin. All rights reserved.
 ************************************************************************************/

#include "RelicRunnersPlayerState.h"
#include "RelicRunners/Menu/Keybinds.h"
#include "Engine/World.h"
#include "Net/UnrealNetwork.h"
#include "RelicRunners/Inventory/InventoryComponent.h"
#include "RelicRunners/RelicRunnersCharacter.h"
#include <RelicRunners/Menu/LobbyGameMode.h>
#include <RelicRunners/PlayerController/RelicRunnersPlayerController.h>
#include <RelicRunners/Inventory/Inventory.h>
#include "RelicRunners/Inventory/InventorySortingOptions.h"
#include "RelicRunners/Item/ItemData.h"
#include <RelicRunners/Item/ItemStats.h>
#include "RelicRunners/Vendor/Vendor.h"
#include "RelicRunners/AbilitySystem/AbilitySelection.h"
#include "RelicRunners/Item/ItemMeshData.h"
#include "Engine/ActorChannel.h"
#include "TimerManager.h"
#include <Blueprint/WidgetBlueprintLibrary.h>
#include "RelicRunners/Menu/JoinUserWidget.h"
#include <Kismet/GameplayStatics.h>
#include <ClassInfo.h>
#include "RelicRunners/Game/RelicRunnersGameInstance.h"
#include <RelicRunners/RelicRunnersGameState.h>

ARelicRunnersPlayerState::ARelicRunnersPlayerState()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

    AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
    AttributeSet = CreateDefaultSubobject<UAttributeSet>(TEXT("AttributeSet"));

    AbilitySystemComponent->SetIsReplicated(true);
    AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);

	//Inventory
	InventoryComponent = CreateDefaultSubobject<UInventoryComponent>(TEXT("InventoryComponent"));
	InventoryComponent->SetIsReplicated(true);
	PlayerNumInventorySlots = 20;

	//Level
	PlayerStartingLevel = 1;
	PlayerLevel = PlayerStartingLevel;
	PlayerXP = 0;
	PlayerXPToLevel = 100;

	//Health
	PlayerStartingMaxHealth = 100;
	PlayerMaxHealth = PlayerStartingMaxHealth;
	PlayerHealth = PlayerMaxHealth;

	//Resource
	PlayerStartingMaxResource = 100;
	PlayerMaxResource = PlayerStartingMaxResource;
	PlayerResource = PlayerMaxResource;
	PlayerStartingResourceRegen = 5; //Percent
	PassiveResourceTime = 3; //Time in seconds

	//Stats
	PlayerStartingArmor = 0;
	PlayerStartingDexterity = 0;
	PlayerStartingStrength = 0;
	PlayerStartingIntelligence = 0;
	PlayerStartingLuck = 0;
	PlayerArmor = 0;
	PlayerDexterity = 0;
	PlayerStrength = 0;
	PlayerIntelligence = 0;
	//Upgraded
	UpgradedHealth = 0;
	UpgradedArmor = 0;
	UpgradedDexterity = 0;
	UpgradedStrength = 0;
	UpgradedIntelligence = 0;
	//Buffed
	BuffedHealth = 0;
	BuffedArmor = 0;
	BuffedDexterity = 0;
	BuffedStrength = 0;
	BuffedIntelligence = 0;
	//Gold
	PlayerStartingGold = 0;
	PlayerGold = 0;

	//Abilities
	PlayerAbilityPoints = PlayerStartingLevel;

	//Potions
	HealthPotionCount = 3;
	PlayerStartingPotionEfficiency = 30; //Percent
	HealthGranted = PlayerStartingPotionEfficiency;
}

void ARelicRunnersPlayerState::BeginPlay()
{
	Super::BeginPlay();

	GetWorldTimerManager().SetTimer(RegenTimer, this, &ARelicRunnersPlayerState::PassiveResourceRegen, PassiveResourceTime, true);
}

void ARelicRunnersPlayerState::CopyProperties(APlayerState* PlayerState)
{
	Super::CopyProperties(PlayerState);

	ARelicRunnersPlayerState* PS = Cast<ARelicRunnersPlayerState>(PlayerState);
	if (PS && this->InventoryComponent && PS->InventoryComponent)
	{
		PS->PlayerGold = this->PlayerGold;
		PS->PlayerLevel = this->PlayerLevel;
		PS->PlayerXP = this->PlayerXP;
		PS->PlayerAbilityPoints = this->PlayerAbilityPoints;
		PS->HasReceivedStarterItems = this->HasReceivedStarterItems;
		PS->SelectedClass = this->SelectedClass;
		PS->UpgradedHealth = this->UpgradedHealth;
		PS->UpgradedArmor = this->UpgradedArmor;
		PS->UpgradedDexterity = this->UpgradedDexterity;
		PS->UpgradedStrength = this->UpgradedStrength;
		PS->UpgradedIntelligence = this->UpgradedIntelligence;
		PS->BuffedHealth = this->BuffedHealth;
		PS->BuffedArmor = this->BuffedArmor;
		PS->BuffedDexterity = this->BuffedDexterity;
		PS->BuffedStrength = this->BuffedStrength;
		PS->BuffedIntelligence = this->BuffedIntelligence;
		PS->PlayerStartingLevel = this->PlayerStartingLevel;
		PS->PlayerStartingMaxHealth = this->PlayerStartingMaxHealth;
		PS->PlayerStartingArmor = this->PlayerStartingArmor;
		PS->PlayerStartingDexterity = this->PlayerStartingDexterity;
		PS->PlayerStartingStrength = this->PlayerStartingStrength;
		PS->PlayerStartingIntelligence = this->PlayerStartingIntelligence;
		PS->PlayerStartingPotionEfficiency = this->PlayerStartingPotionEfficiency;
		PS->PlayerStartingResourceRegen = this->PlayerStartingResourceRegen;
		PS->LevelsEarned = this->LevelsEarned;
		PS->PlayerDeaths = this->PlayerDeaths;
		PS->EnemiesKilled = this->EnemiesKilled;
		PS->MetaXPGainInRun = this->MetaXPGainInRun;

		PS->InventoryComponent->InventoryItems = this->InventoryComponent->InventoryItems;
		for (UItemObject* Item : PS->InventoryComponent->InventoryItems)
		{
			if (Item)
			{
				Item->Rename(nullptr, PS);
			}
		}

		PS->InventoryComponent->EquippedItems = this->InventoryComponent->EquippedItems;
		for (FEquippedItemEntry& Entry : PS->InventoryComponent->EquippedItems)
		{
			if (Entry.Item)
			{
				Entry.Item->Rename(nullptr, PS);
			}
		}

		UE_LOG(LogTemp, Warning, TEXT("CopyProperties: Transferred %d items and %d equipped slots to new PS."),
			PS->InventoryComponent->InventoryItems.Num(),
			PS->InventoryComponent->EquippedItems.Num());
	}
	PS->ForceNetUpdate();
}

void ARelicRunnersPlayerState::Multicast_SpawnVFX_Implementation(FVector Location, FRotator Rotation, FVector Scale, UNiagaraSystem* NiagaraAsset, bool IsAttached)
{
	if (!NiagaraAsset)
		return;

	UWorld* World = GetWorld();
	if (!World)
		return;

	if (!IsAttached)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(World, NiagaraAsset, Location, Rotation, Scale, true, true);
	}
	else
	{
		APawn* Pawn = GetPawn();
		if (!Pawn) return;

		USceneComponent* RootComp = Pawn->GetRootComponent();
		if (!RootComp) return;

		UNiagaraFunctionLibrary::SpawnSystemAttached(NiagaraAsset,RootComp, NAME_None,Location,Rotation,EAttachLocation::KeepRelativeOffset,true, true,ENCPoolMethod::None );
	}
}

void ARelicRunnersPlayerState::Multicast_SpawnStaticMesh_Implementation(FVector Location,FRotator Rotation,FVector Scale,UStaticMesh* Mesh)
{
	if (!Mesh) return;

	AActor* OwnerActor = GetPawn();
	if (!OwnerActor) return;

	UStaticMeshComponent* MeshComp = NewObject<UStaticMeshComponent>(OwnerActor);
	if (!MeshComp) return;

	MeshComp->SetStaticMesh(Mesh);
	MeshComp->SetWorldLocation(Location);
	MeshComp->SetWorldRotation(Rotation);
	MeshComp->SetWorldScale3D(Scale);

	MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	MeshComp->RegisterComponent();

	SpawnedStaticMeshes.Add(MeshComp);
}

void ARelicRunnersPlayerState::Multicast_DestroyAllStaticMeshes_Implementation()
{
	for (UStaticMeshComponent* MeshComp : SpawnedStaticMeshes)
	{
		if (MeshComp)
		{
			MeshComp->DestroyComponent();
		}
	}

	SpawnedStaticMeshes.Empty();
}

bool ARelicRunnersPlayerState::ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags)
{
	bool bWroteSomething = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);

	if (InventoryComponent)
	{
		for (UItemObject* Item : InventoryComponent->InventoryItems)
		{
			if (Item)
			{
				bWroteSomething |= Channel->ReplicateSubobject(Item, *Bunch, *RepFlags);
			}
		}
	}

	return bWroteSomething;
}

ARelicRunnersPlayerState::~ARelicRunnersPlayerState()
{

}

void ARelicRunnersPlayerState::InitializeMetaStats(FSaveData MetaData)
{
	PlayerStartingLuck = MetaData.MetaLuck;
	PlayerStartingMaxHealth = 100 + (MetaData.MetaHealth * 10);
	PlayerStartingArmor = MetaData.MetaDefence * 3;
	PlayerStartingStrength = MetaData.MetaStrength * 3;
	PlayerStartingDexterity = MetaData.MetaAgility * 3;
	PlayerStartingIntelligence = MetaData.MetaIntelligence * 3;
	PlayerStartingPotionEfficiency = 30 + MetaData.MetaPotionEfficiency * 5;
	PlayerStartingResourceRegen = 5 + MetaData.MetaResourceRegen;
	PlayerStartingLevel = 1 + MetaData.MetaStartingLevel;
}

void ARelicRunnersPlayerState::GiveStarterItems()
{
	if (!HasAuthority() || HasReceivedStarterItems) return;

	if (!ItemMeshData || !InventoryComponent)
	{
		UE_LOG(LogTemp, Error, TEXT("GiveStarterItems: Missing Dependencies!"));
		return;
	}

	URelicRunnersGameInstance* GI = GetWorld()->GetGameInstance<URelicRunnersGameInstance>();
	UIconLibrary* Lib = GI ? GI->GlobalIconLibrary : nullptr;

	FString StartingWeapon = "Greatsword";
	FName ClassName = SelectedClass;

	if (ClassName == "Artemis") StartingWeapon = "Bow";
	else if (ClassName == "Aphrodite") StartingWeapon = "Staff";
	else if (ClassName == "Nemesis") StartingWeapon = "Sword";
	else SelectedClass = "Ares";

	FItemData WeaponData = ItemStats::CreateSpecificItemData(1, StartingWeapon, "Common", ItemMeshData, Lib);
	UItemObject* Weapon = ItemStats::CreateItemFromData(WeaponData, InventoryComponent);
	if (Weapon)
	{
		InventoryComponent->AddItem(Weapon);
		Server_EquipItemByID(Weapon->ItemData.UniqueID);
	}

	//Start with a few random items
	for (int i = 0; i < 3; i++)
	{
		FString RandomType = ItemStats::GetRandomItemType();
		FItemData RandData = ItemStats::CreateSpecificItemData(FMath::RandRange(1, PlayerLevel), RandomType, "Common", ItemMeshData, Lib);
		InventoryComponent->AddItem(ItemStats::CreateItemFromData(RandData, InventoryComponent));
	}

	//For Meta levels
	while (PlayerLevel != PlayerStartingLevel)
	{
		OnStartingLevelUp();
	}

	//Give Randomized Armor based on Meta Level
	if (PlayerLevel > 1)
	{
		TArray<FString> ItemTypes = { TEXT("Arms"), TEXT("Helmet"), TEXT("Necklace"), TEXT("Lower"), TEXT("Upper") };

		for (int i = ItemTypes.Num() - 1; i > 0; i--) {
			ItemTypes.Swap(i, FMath::RandRange(0, i));
		}

		int ItemsToGive = FMath::Min(PlayerLevel - 1, 5);
		for (int i = 0; i < ItemsToGive; i++)
		{
			FItemData ArmorData = ItemStats::CreateSpecificItemData(FMath::RandRange(1, PlayerLevel), ItemTypes[i], ItemStats::GetRandomRarity(), ItemMeshData, Lib);
			UItemObject* Armor = ItemStats::CreateItemFromData(ArmorData, InventoryComponent);
			if (Armor)
			{
				InventoryComponent->AddItem(Armor);
				Server_EquipItemByID(Armor->ItemData.UniqueID);
			}
		}
	}

	//Difficulty
	AGameStateBase* GameState = GetWorld()->GetGameState();
	if (ARelicRunnersGameState* RunnersGameState = Cast<ARelicRunnersGameState>(GameState))
	{
		FDifficultyInfo diffInfo = RunnersGameState->GetDifficultyInfo(GI->CurrentDifficulty);
		PlayerStartingLuck += diffInfo.LuckAmount;
	}

	//Stats from ClassInfo
	if (InventoryComponent->ClassInfo)
	{
		FClassData Stats = InventoryComponent->ClassInfo->GetStats(SelectedClass.ToString());
		float BaseHealth = 100;
		PlayerStartingMaxHealth += (Stats.Health - BaseHealth);
		PlayerStartingArmor += Stats.Armor;
		PlayerStartingDexterity += Stats.Dexterity;
		PlayerStartingStrength += Stats.Strength;
		PlayerStartingIntelligence += Stats.Intelligence;
	}

	//Force an update of the Cached Stats
	InventoryComponent->UpdateTotalEquippedStats(GetPawn());

	//Ensures the player starts the match at 100% health
	PlayerMaxHealth = InventoryComponent->CachedEquippedStats.TotalHealth;
	PlayerHealth = PlayerMaxHealth;

	HasReceivedStarterItems = true;
}

void ARelicRunnersPlayerState::RefreshStatsForNewLevel()
{
	if (!HasAuthority()) return;

	if (InventoryComponent)
	{
		InventoryComponent->UpdateTotalEquippedStats(GetPawn());

		PlayerMaxHealth = InventoryComponent->CachedEquippedStats.TotalHealth;
		PlayerHealth = PlayerMaxHealth;

		UpdateRemoteHUDHealth();

		UE_LOG(LogTemp, Warning, TEXT("Server: Stats Refreshed for New Level. MaxHP: %d"), PlayerMaxHealth);
	}
}

void ARelicRunnersPlayerState::PassiveHealthRegen()
{
	if (!HasAuthority()) return;

	if (PlayerHealth < PlayerMaxHealth)
	{
		int RegenAmount = FMath::CeilToInt(PlayerMaxHealth * 0.05f);
		PlayerHealth = FMath::Min(PlayerHealth + RegenAmount, PlayerMaxHealth);

		//For server
		UpdateRemoteHUDHealth();
	}
}

void ARelicRunnersPlayerState::PassiveResourceRegen()
{
	if (!HasAuthority()) return;

	if (!InventoryComponent) return;

	if (PlayerResource < PlayerMaxResource)
	{
		int RegenAmount = FMath::CeilToInt(PlayerMaxResource * (InventoryComponent->CachedEquippedStats.TotalResourceRegen/100.0f));
		PlayerResource = FMath::Min(PlayerResource + RegenAmount, PlayerMaxResource);

		//For server
		UpdateRemoteHUDResource();
	}
}

void ARelicRunnersPlayerState::Client_UpdateEquippedStats_Implementation(const FEquippedStatsSummary& NewStats)
{
	if (InventoryComponent)
	{
		InventoryComponent->CachedEquippedStats = NewStats;
		InventoryComponent->OnStatsChanged.Broadcast(NewStats);
	}
}

void ARelicRunnersPlayerState::SetPlayerMaxHealth(int value)
{
	if (!HasAuthority()) return;

	PlayerMaxHealth = value;
	if (PlayerHealth > PlayerMaxHealth) PlayerHealth = PlayerMaxHealth;
	//For server
	UpdateRemoteHUDHealth();
}

void ARelicRunnersPlayerState::SetPlayerMaxResource(int value)
{
	if (!HasAuthority()) return;

	PlayerMaxResource = value;
	if (PlayerResource > PlayerMaxResource) PlayerResource = PlayerMaxResource;

	//For server
	UpdateRemoteHUDResource();
}

void ARelicRunnersPlayerState::Server_UseHealthPotion_Implementation()
{
	if (!HasAuthority()) return;

	if (HealthPotionCount > 0 && PlayerHealth < PlayerMaxHealth)
	{
		int HealthToGrant = FMath::RoundToInt(PlayerMaxHealth * (HealthGranted / 100.0f));


		PlayerHealth += HealthToGrant;
		
		int Efficiency = InventoryComponent->CachedEquippedStats.TotalPotionEfficiency;
		int ChanceToNotConsume = ItemStats::GetChanceToNotConsume(Efficiency);
		int Roll = FMath::RandRange(1, 100);
		if (Roll > ChanceToNotConsume)
		{
			HealthPotionCount--;
		}

		if (PlayerHealth > PlayerMaxHealth)
		{
			PlayerHealth = PlayerMaxHealth;
		}

		PlayHealPotionSound();

		// Notify owning client to update HUD
		UpdateRemoteHUDHealth();
		UpdateRemoteHUDHealthPotions();
	}
}

void ARelicRunnersPlayerState::OnStartingLevelUp()
{
	PlayerLevel++;
	PlayerXPToLevel = PlayerLevel * 100;

	//Ability points
	PlayerAbilityPoints++;

	//Weapon Upgrade
	int HealthIncrease = 0;
	int ArmorIncrease = 0;
	int DexterityIncrease = 0;
	int StrengthIncrease = 0;
	int IntelligenceIncrease = 0;
	FString StartingWeapon = "";
	if (SelectedClass == "Ares")
	{
		StartingWeapon = "Greatsword";
		HealthIncrease = 5;
		ArmorIncrease = 2;
		StrengthIncrease = 1;
	}
	else if (SelectedClass == "Artemis")
	{
		StartingWeapon = "Bow";
		DexterityIncrease = 3;
	}
	else if (SelectedClass == "Aphrodite")
	{
		StartingWeapon = "Staff";
		IntelligenceIncrease = 3;
	}
	else if (SelectedClass == "Nemesis")
	{
		StartingWeapon = "Sword";
		DexterityIncrease = 3;
	}
	else
	{
		StartingWeapon = "Greatsword";
		HealthIncrease = 5;
		ArmorIncrease = 2;
		StrengthIncrease = 1;
	}

	UItemObject*& Item = InventoryComponent->GetEquippedItemReference(StartingWeapon);
	if (!Item) return;
	Item->ItemData.Level++;

	if (Item->ItemData.Level >= 3 && Item->ItemData.Rarity == "Common")
	{
		Item->ItemData.Rarity = "Uncommon";
	}
	else if (Item->ItemData.Level >= 6 && Item->ItemData.Rarity == "Uncommon")
	{
		Item->ItemData.Rarity = "Rare";
	}
	else if (Item->ItemData.Level >= 9 && Item->ItemData.Rarity == "Rare")
	{
		Item->ItemData.Rarity = "Epic";
	}
	else if (Item->ItemData.Level >= 12 && Item->ItemData.Rarity == "Epic")
	{
		Item->ItemData.Rarity = "Legendary";
	}
	else if (Item->ItemData.Level >= 15 && Item->ItemData.Rarity == "Legendary")
	{
		Item->ItemData.Rarity = "Mythic";
	}
	else if (Item->ItemData.Level >= 18 && Item->ItemData.Rarity == "Mythic")
	{
		Item->ItemData.Rarity = "Relic";
	}
	Item->ItemData.Health += FMath::RandRange(2, 5 + HealthIncrease);
	Item->ItemData.Armor += FMath::RandRange(0, 1 + ArmorIncrease);
	Item->ItemData.Dexterity += FMath::RandRange(0, 1 + DexterityIncrease);
	Item->ItemData.Strength += FMath::RandRange(0, 1 + StrengthIncrease);
	Item->ItemData.Intelligence += FMath::RandRange(0, 1 + IntelligenceIncrease);
}

void ARelicRunnersPlayerState::OnLevelUp()
{
	PlayerXP -= PlayerXPToLevel;
	PlayerLevel++;
	LevelsEarned++;
	PlayerStartingLevel++;
	PlayerXPToLevel = PlayerLevel * 100;

	if (HealthPotionCount < 3)
	{
		HealthPotionCount++;
		UpdateRemoteHUDHealthPotions();
	}

	//Ability points
	PlayerAbilityPoints++;

	//Weapon Upgrade
	int HealthIncrease = 0;
	int ArmorIncrease = 0;
	int DexterityIncrease = 0;
	int StrengthIncrease = 0;
	int IntelligenceIncrease = 0;
	FString StartingWeapon = "";
	if (SelectedClass == "Ares")
	{
		StartingWeapon = "Greatsword";
		HealthIncrease = 5;
		ArmorIncrease = 2;
		StrengthIncrease = 1;
	}
	else if (SelectedClass == "Artemis")
	{
		StartingWeapon = "Bow";
		DexterityIncrease = 3;
		StrengthIncrease = 2;
	}
	else if (SelectedClass == "Aphrodite")
	{
		StartingWeapon = "Staff";
		IntelligenceIncrease = 3;
		StrengthIncrease = 1;
	}
	else if (SelectedClass == "Nemesis")
	{
		StartingWeapon = "Sword";
		DexterityIncrease = 3;
		StrengthIncrease = 6;
	}
	else
	{
		StartingWeapon = "Greatsword";
		HealthIncrease = 5;
		ArmorIncrease = 2;
		StrengthIncrease = 1;
	}

	UItemObject*& Item = InventoryComponent->GetEquippedItemReference(StartingWeapon);
	Item->ItemData.Level++;

	if (Item->ItemData.Level >= 3 && Item->ItemData.Rarity == "Common")
	{
		Item->ItemData.Rarity = "Uncommon";
	}
	else if (Item->ItemData.Level >= 6 && Item->ItemData.Rarity == "Uncommon")
	{
		Item->ItemData.Rarity = "Rare";
	}
	else if (Item->ItemData.Level >= 9 && Item->ItemData.Rarity == "Rare")
	{
		Item->ItemData.Rarity = "Epic";
	}
	else if (Item->ItemData.Level >= 12 && Item->ItemData.Rarity == "Epic")
	{
		Item->ItemData.Rarity = "Legendary";
	}
	else if (Item->ItemData.Level >= 15 && Item->ItemData.Rarity == "Legendary")
	{
		Item->ItemData.Rarity = "Mythic";
	}
	else if (Item->ItemData.Level >= 18 && Item->ItemData.Rarity == "Mythic")
	{
		Item->ItemData.Rarity = "Relic";
	}
	Item->ItemData.Health += FMath::RandRange(2, 5 + HealthIncrease);
	Item->ItemData.Armor += FMath::RandRange(0, 1 + ArmorIncrease);
	Item->ItemData.Dexterity += FMath::RandRange(0, 1 + DexterityIncrease);
	Item->ItemData.Strength += FMath::RandRange(0, 1 + StrengthIncrease);
	Item->ItemData.Intelligence += FMath::RandRange(0, 1 + IntelligenceIncrease);

	InventoryComponent->UpdateTotalEquippedStats(GetPawn());

	if (ARelicRunnersPlayerController* PC = Cast<ARelicRunnersPlayerController>(GetPlayerController()))
	{
		PC->HandleLevelUpVisuals();
	}
	PlayLevelUpSound();
}

void ARelicRunnersPlayerState::Server_AddExperience_Implementation(int Amount)
{
	AddExperience(Amount);
}

void ARelicRunnersPlayerState::AddExperience(int Amount)
{
	if (!HasAuthority())
	{
		Server_AddExperience(Amount);
		return;
	}

	PlayerXP += Amount;
	while (PlayerXP >= PlayerXPToLevel)
	{
		OnLevelUp();
	}

	UpdateRemoteHUDAbility();
	UpdateRemoteHUDExperience();

	int AddedMeta = 0;
	if (ARelicRunnersGameState* GS = GetWorld()->GetGameState<ARelicRunnersGameState>())
	{
		if (URelicRunnersGameInstance* GI = GetGameInstance<URelicRunnersGameInstance>())
		{
			FDifficultyInfo diffInfo = GS->GetDifficultyInfo(GI->CurrentDifficulty);
			AddedMeta = FMath::RandRange(1, 3) * diffInfo.MetaXPIncrease;
			MetaXPGainInRun += AddedMeta;
		}
	}

	if (ARelicRunnersPlayerController* PC = Cast<ARelicRunnersPlayerController>(GetOwner()))
	{
		PC->Client_RegisterMetaXPGain(AddedMeta);
	}

	float Chance = 10.0f;
	if (FMath::FRandRange(0.0f, 100.0f) < Chance)
	{
		AddGold(1);
	}
}

void ARelicRunnersPlayerState::PlayHealPotionSound_Implementation()
{
	if (!HealPotionSound) return;

	APawn* Pawn = GetPawn();
	if (!Pawn) return;

	UGameplayStatics::PlaySoundAtLocation(Pawn,HealPotionSound,Pawn->GetActorLocation());
}

void ARelicRunnersPlayerState::PlayLevelUpSound_Implementation()
{
	if (!LevelUpSound) return;

	APawn* Pawn = GetPawn();

	if (!Pawn) return;

	UGameplayStatics::PlaySoundAtLocation(Pawn, LevelUpSound, Pawn->GetActorLocation());
}

void ARelicRunnersPlayerState::Client_OnLevelUp_Implementation()
{
	if (ARelicRunnersPlayerController* PC = Cast<ARelicRunnersPlayerController>(GetPlayerController()))
	{
		PC->HandleLevelUpVisuals();
	}
}

void ARelicRunnersPlayerState::SetSelectedClass(FName NewClass)
{
    if (!HasAuthority()) return;

    SelectedClass = NewClass;
    OnSelectedClassChanged.Broadcast(SelectedClass);
    ForceNetUpdate();

    if (ALobbyGameMode* GM = GetWorld()->GetAuthGameMode<ALobbyGameMode>())
    {
        GM->OnPlayerStateUpdated(this);
    }
}

void ARelicRunnersPlayerState::Client_OnAbilityPointsUpdated_Implementation(int NewAbilityPoints)
{
	ARelicRunnersPlayerController* PC = Cast<ARelicRunnersPlayerController>(GetPlayerController());
	if (!PC) return;

	PC->UpdateHUDAbilityPoints(NewAbilityPoints);

	if (NewAbilityPoints <= 0 && PC->AbilitySelection)
	{
		PC->HideUI(PC->AbilitySelection);
	}
}

void ARelicRunnersPlayerState::AddGold(int Value)
{
	Server_AddGold(Value);
}

void ARelicRunnersPlayerState::Server_AddGold_Implementation(int Value)
{
	PlayerGold += Value;
	InventoryComponent->UpdateTotalEquippedStats(GetPawn());
}

bool ARelicRunnersPlayerState::CheckEnoughGold(int Value)
{
	if (PlayerGold >= Value)
	{
		PlayerGold -= Value;
		InventoryComponent->UpdateTotalEquippedStats(GetPawn());
		return true;
	}
	return false;
}

void ARelicRunnersPlayerState::Server_SpendAbilityPoints_Implementation()
{
	if (!HasAuthority()) return;

	if (PlayerAbilityPoints > 0)
	{
		PlayerAbilityPoints--;

		Client_OnAbilityPointsUpdated(PlayerAbilityPoints);
	}

	//For server
	UpdateRemoteHUDAbility();
}

void ARelicRunnersPlayerState::Server_BuyItem_Implementation(const FItemData& ItemData, AVendor* Vendor)
{
	if (!Vendor || !InventoryComponent) return;

	if (!CheckEnoughGold(ItemData.Gold)) return;

	UItemObject* NewItem = ItemStats::CreateItemFromData(ItemData, InventoryComponent);
	NewItem->IsNewItem = true;
	InventoryComponent->AddItem(NewItem);
	Vendor->RemoveStock(ItemData);

	if (ARelicRunnersPlayerController* RPC = Cast<ARelicRunnersPlayerController>(GetPlayerController()))
	{
		RPC->Client_UpdateVendorUI(Vendor);
	}
}

void ARelicRunnersPlayerState::Server_SellItem_Implementation(const FItemData& ItemData, AVendor* Vendor)
{
	if (!Vendor || !InventoryComponent) return;

	ARelicRunnersPlayerController* RPC = Cast<ARelicRunnersPlayerController>(GetPlayerController());

	//Unequip if equipped
	UItemObject* EquippedItem = InventoryComponent->GetEquippedItemReference(ItemData.ItemType);
	if (EquippedItem && EquippedItem->ItemData.UniqueID == ItemData.UniqueID)
	{
		InventoryComponent->UnequipItem(EquippedItem, RPC);
	}

	InventoryComponent->RemoveItemByID(ItemData.UniqueID);
	AddGold(ItemData.Gold);
	Vendor->AddStock(ItemData);

	RPC->Client_UpdateVendorUI(Vendor);
}

void ARelicRunnersPlayerState::OnRep_DeathState()
{
	if (DeathState && GetPlayerController() && GetPlayerController()->IsLocalController())
	{
		if (ARelicRunnersPlayerController* RPC = Cast<ARelicRunnersPlayerController>(GetPlayerController()))
		{
			RPC->CleanUI();
			RPC->ChangeState(NAME_Spectating);
			RPC->SpectatingUI();
			RPC->Client_RequestSaveStats();
		}
	}
}

EInventorySorting ARelicRunnersPlayerState::GetCurrentSortingMethod() const
{
	return InventoryComponent ? InventoryComponent->CurrentSortingMethod : EInventorySorting::SortByLevel;
}

void ARelicRunnersPlayerState::Server_EquipItemByID_Implementation(FGuid ID)
{
	if (!InventoryComponent) return;

	UItemObject* Item = InventoryComponent->FindItemByID(ID);
	if (Item)
	{
		if (ARelicRunnersCharacter* Char = Cast<ARelicRunnersCharacter>(GetPawn()))
		{
			Char->UpdateItemVisuals(Item);
			InventoryComponent->EquipItem(Item, Cast<ARelicRunnersPlayerController>(Char->GetController()));
			InventoryComponent->UpdateTotalEquippedStats(Char);
		}
	}
}

void ARelicRunnersPlayerState::Server_UnequipItemByID_Implementation(FGuid ID)
{
	if (!InventoryComponent) return;

	UItemObject* EquippedItem = nullptr;
	FString ItemType;

	for (FEquippedItemEntry& Entry : InventoryComponent->EquippedItems)
	{
		if (Entry.Item && Entry.Item->ItemData.UniqueID == ID)
		{
			EquippedItem = Entry.Item;
			ItemType = Entry.ItemType;
			break;
		}
	}

	if (EquippedItem)
	{
		if (ARelicRunnersCharacter* Char = Cast<ARelicRunnersCharacter>(GetPawn()))
		{
			Char->UpdateItemVisuals(nullptr, ItemType);
			InventoryComponent->UnequipItem(EquippedItem, Cast<ARelicRunnersPlayerController>(Char->GetController()));
			InventoryComponent->UpdateTotalEquippedStats(Char);
		}
	}

	InventoryComponent->SortInventoryByCurrentMethod();
}

void ARelicRunnersPlayerState::OnRep_SelectedClass()
{
    OnSelectedClassChanged.Broadcast(SelectedClass);
}

void ARelicRunnersPlayerState::OnRep_PlayerGold()
{
	if (ARelicRunnersCharacter* Char = Cast<ARelicRunnersCharacter>(GetPawn()))
	{
		InventoryComponent->UpdateTotalEquippedStats(Char);
	}
}

void ARelicRunnersPlayerState::OnRep_IsHost()
{
	OnHostStatusChanged.Broadcast(IsHost);
}

void ARelicRunnersPlayerState::SetIsHost(bool NewIsHost)
{
	if (HasAuthority())
	{
		IsHost = NewIsHost;
		OnHostStatusChanged.Broadcast(IsHost);
	}
}

void ARelicRunnersPlayerState::OnRep_PlayerLevel(int OldLevel)
{
	if (bIsInitialSync)
	{
		bIsInitialSync = false;
		return;
	}

	if (OldLevel > 0 && PlayerLevel > OldLevel)
	{
		Client_OnLevelUp();
	}
}

void ARelicRunnersPlayerState::ModifyHealth(float Value)
{
	if (!HasAuthority() || DeathState) return;

	if (bIsInvincible && Value < 0.f) return;

	PlayerHealth = FMath::Clamp(PlayerHealth + Value, 0.f, (float)PlayerMaxHealth);

	if (PlayerHealth <= 0)
	{
		ARelicRunnersCharacter* Char = Cast<ARelicRunnersCharacter>(GetPawn());
		if (Char)
		{
			Char->Die();
		}
	}

	//For server
	UpdateRemoteHUDHealth();
}

void ARelicRunnersPlayerState::UpdateRemoteHUDHealth()
{
	if (ARelicRunnersPlayerController* PC = Cast<ARelicRunnersPlayerController>(GetPlayerController()))
	{
		PC->UpdateHUDHealth(PlayerHealth, PlayerMaxHealth);
	}
}

void ARelicRunnersPlayerState::UpdateRemoteHUDHealthPotions()
{
	if (ARelicRunnersPlayerController* PC = Cast<ARelicRunnersPlayerController>(GetPlayerController()))
	{
		PC->UpdateHUDHealthPotions(HealthPotionCount);
	}
}

void ARelicRunnersPlayerState::UpdateRemoteHUDResource()
{
	if (ARelicRunnersPlayerController* PC = Cast<ARelicRunnersPlayerController>(GetPlayerController()))
	{
		PC->UpdateHUDResource(PlayerResource, PlayerMaxResource);
	}
}

void ARelicRunnersPlayerState::UpdateRemoteHUDExperience()
{
	if (ARelicRunnersPlayerController* PC = Cast<ARelicRunnersPlayerController>(GetPlayerController()))
	{
		PC->UpdateHUDXP(PlayerLevel, PlayerXP, PlayerXPToLevel);
	}
}

void ARelicRunnersPlayerState::UpdateRemoteHUDAbility()
{
	if (ARelicRunnersPlayerController* PC = Cast<ARelicRunnersPlayerController>(GetPlayerController()))
	{
		PC->UpdateHUDAbilityPoints(PlayerAbilityPoints);
	}
}

void ARelicRunnersPlayerState::OnRep_Health()
{
	UpdateRemoteHUDHealth();

	if (PlayerHealth < PlayerMaxHealth)
	{
		APawn* MyPawn = GetPawn();
		if (MyPawn && MyPawn->IsLocallyControlled())
		{
			if (APlayerController* PC = Cast<APlayerController>(MyPawn->GetController()))
			{
				if (HitCameraShakeClass) PC->ClientStartCameraShake(HitCameraShakeClass);
			}
		}
	}
}

void ARelicRunnersPlayerState::OnRep_HealthPotions()
{
	UpdateRemoteHUDHealthPotions();
}

void ARelicRunnersPlayerState::OnRep_Resource()
{
	UpdateRemoteHUDResource();
}

void ARelicRunnersPlayerState::OnRep_Experience()
{
	UpdateRemoteHUDExperience();
}

void ARelicRunnersPlayerState::OnRep_Ability()
{
	UpdateRemoteHUDAbility();
}

void ARelicRunnersPlayerState::OnRep_StatsChanged()
{
	UpdateRemoteHUDHealth();
	UpdateRemoteHUDResource();

	InventoryComponent->UpdateTotalEquippedStats(GetPawn());
}

void ARelicRunnersPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(ARelicRunnersPlayerState, SelectedClass);
	DOREPLIFETIME(ARelicRunnersPlayerState, PlayerHealth);
	DOREPLIFETIME(ARelicRunnersPlayerState, HealthPotionCount);
	DOREPLIFETIME(ARelicRunnersPlayerState, PlayerMaxHealth);
	DOREPLIFETIME(ARelicRunnersPlayerState, PlayerResource);
	DOREPLIFETIME(ARelicRunnersPlayerState, PlayerMaxResource);
	DOREPLIFETIME(ARelicRunnersPlayerState, PlayerLevel);
	DOREPLIFETIME(ARelicRunnersPlayerState, PlayerXP);
	DOREPLIFETIME(ARelicRunnersPlayerState, PlayerXPToLevel);
	DOREPLIFETIME(ARelicRunnersPlayerState, PlayerAbilityPoints);
	DOREPLIFETIME(ARelicRunnersPlayerState, PlayerGold);
	DOREPLIFETIME(ARelicRunnersPlayerState, PlayerStartingLevel);
	DOREPLIFETIME(ARelicRunnersPlayerState, PlayerStartingMaxHealth);
	DOREPLIFETIME(ARelicRunnersPlayerState, PlayerStartingArmor);
	DOREPLIFETIME(ARelicRunnersPlayerState, PlayerStartingDexterity);
	DOREPLIFETIME(ARelicRunnersPlayerState, PlayerStartingStrength);
	DOREPLIFETIME(ARelicRunnersPlayerState, PlayerStartingIntelligence);
	DOREPLIFETIME(ARelicRunnersPlayerState, BuffedHealth);
	DOREPLIFETIME(ARelicRunnersPlayerState, BuffedArmor);
	DOREPLIFETIME(ARelicRunnersPlayerState, BuffedDexterity);
	DOREPLIFETIME(ARelicRunnersPlayerState, BuffedStrength);
	DOREPLIFETIME(ARelicRunnersPlayerState, BuffedIntelligence);
	DOREPLIFETIME(ARelicRunnersPlayerState, UpgradedHealth);
	DOREPLIFETIME(ARelicRunnersPlayerState, UpgradedArmor);
	DOREPLIFETIME(ARelicRunnersPlayerState, UpgradedDexterity);
	DOREPLIFETIME(ARelicRunnersPlayerState, UpgradedStrength);
	DOREPLIFETIME(ARelicRunnersPlayerState, UpgradedIntelligence);
	DOREPLIFETIME(ARelicRunnersPlayerState, HasReceivedStarterItems);
	DOREPLIFETIME(ARelicRunnersPlayerState, IsHost);
	DOREPLIFETIME(ARelicRunnersPlayerState, DeathState);
	DOREPLIFETIME(ARelicRunnersPlayerState, bIsInvincible);
	DOREPLIFETIME(ARelicRunnersPlayerState, PlayerDeaths);
	DOREPLIFETIME(ARelicRunnersPlayerState, LevelsEarned);
	DOREPLIFETIME(ARelicRunnersPlayerState, EnemiesKilled);
	DOREPLIFETIME(ARelicRunnersPlayerState, MetaXP);
	DOREPLIFETIME(ARelicRunnersPlayerState, MetaXPGainInRun);
	DOREPLIFETIME(ARelicRunnersPlayerState, MetaLevel);
	DOREPLIFETIME(ARelicRunnersPlayerState, MetaXPToNextLevel);
}

