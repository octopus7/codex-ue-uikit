#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "CodexUIKitInventoryGridWidget.generated.h"

class UScrollBox;
class UTextBlock;
class UVerticalBox;

UENUM()
enum class ECodexInventoryFilter : uint8
{
	All,
	Material,
	Consumable,
	Equipment
};

USTRUCT()
struct FCodexInventoryDemoItem
{
	GENERATED_BODY()

	UPROPERTY()
	FString Name;

	UPROPERTY()
	FString IconLabel;

	UPROPERTY()
	ECodexInventoryFilter Category = ECodexInventoryFilter::Material;

	UPROPERTY()
	int32 Quantity = 1;

	UPROPERTY()
	float Weight = 0.0f;

	UPROPERTY()
	int32 Value = 0;

	UPROPERTY()
	bool bLocked = false;
};

class UCodexUIKitInventoryGridWidget;

UCLASS()
class CODEXUIKIT_API UCodexInventoryItemButton : public UButton
{
	GENERATED_BODY()

public:
	void InitializeItemButton(UCodexUIKitInventoryGridWidget* InOwner, int32 InItemIndex);

private:
	UPROPERTY()
	TObjectPtr<UCodexUIKitInventoryGridWidget> OwnerWidget;

	int32 ItemIndex = INDEX_NONE;

	UFUNCTION()
	void HandleClicked();
};

UCLASS()
class CODEXUIKIT_API UCodexUIKitInventoryGridWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Codex UIKit")
	void ResetDemoInventory();

	void SelectItemByIndex(int32 ItemIndex);

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;

private:
	UPROPERTY()
	TObjectPtr<UScrollBox> ItemScrollBox;

	UPROPERTY()
	TObjectPtr<UVerticalBox> DetailBox;

	UPROPERTY()
	TObjectPtr<UTextBlock> HeaderLabel;

	UPROPERTY()
	TObjectPtr<UTextBlock> FilterLabel;

	UPROPERTY()
	TArray<FCodexInventoryDemoItem> Items;

	ECodexInventoryFilter ActiveFilter = ECodexInventoryFilter::All;
	int32 SelectedItemIndex = 0;

	void SeedItemsIfNeeded();
	void RefreshInventory();
	void RefreshDetail();
	void SetFilter(ECodexInventoryFilter NewFilter);
	void SelectNextVisibleItem(int32 Direction);
	TArray<int32> GetVisibleItemIndexes() const;
	TArray<int32> GetSelectableVisibleItemIndexes() const;
	bool MatchesFilter(const FCodexInventoryDemoItem& Item) const;
	void EnsureSelectionVisible();
	float GetTotalWeight() const;
	int32 GetUnlockedCount() const;

	static FText CategoryText(ECodexInventoryFilter Category);
	static FLinearColor CategoryColor(ECodexInventoryFilter Category);

	UFUNCTION()
	void HandleFilterAll();

	UFUNCTION()
	void HandleFilterMaterial();

	UFUNCTION()
	void HandleFilterConsumable();

	UFUNCTION()
	void HandleFilterEquipment();

	UFUNCTION()
	void HandlePreviousItem();

	UFUNCTION()
	void HandleNextItem();

	UFUNCTION()
	void HandleUseSelectedItem();

	UFUNCTION()
	void HandleToggleLockSelectedItem();
};
