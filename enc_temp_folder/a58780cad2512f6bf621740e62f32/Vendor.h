#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "RelicRunners/Interact/InteractInterface.h"
#include "Vendor.generated.h"

class UUserWidget;
class UItemMeshData;
struct FItemData;
class UInventoryComponent;
class ARelicRunnersCharacter;

UCLASS()
class RELICRUNNERS_API AVendor : public ACharacter, public IInteractInterface
{
	GENERATED_BODY()

public:
	AVendor();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vendor|Stock")
	TSubclassOf<UItemMeshData> ItemMeshDataClass;

	UFUNCTION(BlueprintCallable, Category = "Vendor")
	void RerollStock();

	void AddStock(const struct FItemData& ItemData);

	void RemoveStock(const struct FItemData& ItemData);

	// IInteractInterface
	virtual void Interact_Implementation(ARelicRunnersCharacter* Char) override;
	virtual FItemData GetItemData_Implementation() override;
	virtual void ShowTooltip_Implementation(bool bShow) override; // Empty

	TArray<struct FItemData> GetStock() { return Stock; }

protected:
	virtual void BeginPlay() override;

	void GenerateStock();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// Generated stock
	UPROPERTY(ReplicatedUsing = OnRep_Stock, VisibleAnywhere, BlueprintReadOnly, Category = "Vendor|Stock")
	TArray<struct FItemData> Stock;

	UFUNCTION()
	void OnRep_Stock();

	UPROPERTY()
	class UInventory* LinkedInventoryWidget;

	UPROPERTY(Transient)
	UItemMeshData* ItemMeshData = nullptr;
};
