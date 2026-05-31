#include "UI/CodexUIKitInventoryGridWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/ScrollBox.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Components/UniformGridPanel.h"
#include "Components/UniformGridSlot.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "UI/CodexUIStyle.h"

namespace
{
constexpr int32 InventoryColumns = 5;

UTextBlock* InventoryText(UWidgetTree& Tree, const FString& Text, float Size, const FLinearColor& Color, FName Typeface = NAME_None)
{
	UTextBlock* TextBlock = Tree.ConstructWidget<UTextBlock>();
	TextBlock->SetText(FText::FromString(Text));
	TextBlock->SetFont(FCodexUIStyle::Font(Size, Typeface));
	TextBlock->SetColorAndOpacity(FSlateColor(Color));
	return TextBlock;
}

USizeBox* InventorySized(UWidgetTree& Tree, UWidget* Content, float Width, float Height)
{
	USizeBox* SizeBox = Tree.ConstructWidget<USizeBox>();
	SizeBox->SetWidthOverride(Width);
	SizeBox->SetHeightOverride(Height);
	SizeBox->AddChild(Content);
	return SizeBox;
}

void AddInventoryVBox(UVerticalBox& Box, UWidget* Child, const FMargin Padding = FMargin(0.0f), EHorizontalAlignment Align = HAlign_Fill)
{
	UVerticalBoxSlot* Slot = Box.AddChildToVerticalBox(Child);
	Slot->SetPadding(Padding);
	Slot->SetHorizontalAlignment(Align);
}

void AddInventoryHBox(UHorizontalBox& Box, UWidget* Child, const FMargin Padding = FMargin(0.0f), float FillWidth = 0.0f)
{
	UHorizontalBoxSlot* Slot = Box.AddChildToHorizontalBox(Child);
	Slot->SetPadding(Padding);
	FSlateChildSize Size(FillWidth > 0.0f ? ESlateSizeRule::Fill : ESlateSizeRule::Automatic);
	Size.Value = FillWidth > 0.0f ? FillWidth : 1.0f;
	Slot->SetSize(Size);
}

FCodexInventoryDemoItem MakeInventoryItem(
	const TCHAR* Name,
	const TCHAR* IconLabel,
	ECodexInventoryFilter Category,
	int32 Quantity,
	float Weight,
	int32 Value,
	bool bLocked = false)
{
	FCodexInventoryDemoItem Item;
	Item.Name = Name;
	Item.IconLabel = IconLabel;
	Item.Category = Category;
	Item.Quantity = Quantity;
	Item.Weight = Weight;
	Item.Value = Value;
	Item.bLocked = bLocked;
	return Item;
}
}

void UCodexInventoryItemButton::InitializeItemButton(UCodexUIKitInventoryGridWidget* InOwner, int32 InItemIndex)
{
	OwnerWidget = InOwner;
	ItemIndex = InItemIndex;
	OnClicked.Clear();
	OnClicked.AddDynamic(this, &ThisClass::HandleClicked);
}

void UCodexInventoryItemButton::HandleClicked()
{
	if (OwnerWidget)
	{
		OwnerWidget->SelectItemByIndex(ItemIndex);
	}
}

void UCodexUIKitInventoryGridWidget::ResetDemoInventory()
{
	Items.Reset();
	ActiveFilter = ECodexInventoryFilter::All;
	SelectedItemIndex = 0;
	SeedItemsIfNeeded();
	RefreshInventory();
	RefreshDetail();
}

void UCodexUIKitInventoryGridWidget::SelectItemByIndex(int32 ItemIndex)
{
	if (!Items.IsValidIndex(ItemIndex) || Items[ItemIndex].bLocked)
	{
		return;
	}

	SelectedItemIndex = ItemIndex;
	RefreshInventory();
	RefreshDetail();
}

void UCodexUIKitInventoryGridWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (AllFilterButton)
	{
		AllFilterButton->OnClicked.AddDynamic(this, &ThisClass::HandleFilterAll);
	}
	if (MaterialFilterButton)
	{
		MaterialFilterButton->OnClicked.AddDynamic(this, &ThisClass::HandleFilterMaterial);
	}
	if (ConsumableFilterButton)
	{
		ConsumableFilterButton->OnClicked.AddDynamic(this, &ThisClass::HandleFilterConsumable);
	}
	if (EquipmentFilterButton)
	{
		EquipmentFilterButton->OnClicked.AddDynamic(this, &ThisClass::HandleFilterEquipment);
	}
	if (PreviousItemButton)
	{
		PreviousItemButton->OnClicked.AddDynamic(this, &ThisClass::HandlePreviousItem);
	}
	if (NextItemButton)
	{
		NextItemButton->OnClicked.AddDynamic(this, &ThisClass::HandleNextItem);
	}
	if (UseItemButton)
	{
		UseItemButton->OnClicked.AddDynamic(this, &ThisClass::HandleUseSelectedItem);
	}
	if (LockItemButton)
	{
		LockItemButton->OnClicked.AddDynamic(this, &ThisClass::HandleToggleLockSelectedItem);
	}
}

void UCodexUIKitInventoryGridWidget::NativeConstruct()
{
	Super::NativeConstruct();
	FCodexUIStyle::ApplyFontToTextBlocks(GetRootWidget());

	SeedItemsIfNeeded();
	EnsureSelectionVisible();
	RefreshInventory();
	RefreshDetail();
}

void UCodexUIKitInventoryGridWidget::SeedItemsIfNeeded()
{
	if (!Items.IsEmpty())
	{
		return;
	}

	Items.Add(MakeInventoryItem(TEXT("낡은 검"), TEXT("검"), ECodexInventoryFilter::Equipment, 1, 5.2f, 120));
	Items.Add(MakeInventoryItem(TEXT("작은 회복약"), TEXT("약"), ECodexInventoryFilter::Consumable, 6, 0.2f, 12));
	Items.Add(MakeInventoryItem(TEXT("청록 결정"), TEXT("정"), ECodexInventoryFilter::Material, 9, 0.4f, 35));
	Items.Add(MakeInventoryItem(TEXT("보급 식량"), TEXT("식"), ECodexInventoryFilter::Consumable, 4, 0.7f, 18));
	Items.Add(MakeInventoryItem(TEXT("스위퍼 부품"), TEXT("부"), ECodexInventoryFilter::Material, 3, 1.1f, 42));
	Items.Add(MakeInventoryItem(TEXT("탐사용 장갑"), TEXT("장"), ECodexInventoryFilter::Equipment, 1, 1.4f, 80));
	Items.Add(MakeInventoryItem(TEXT("말린 허브"), TEXT("잎"), ECodexInventoryFilter::Material, 12, 0.1f, 5));
	Items.Add(MakeInventoryItem(TEXT("임시 배터리"), TEXT("전"), ECodexInventoryFilter::Consumable, 2, 0.8f, 28));
	Items.Add(MakeInventoryItem(TEXT("방수 천"), TEXT("천"), ECodexInventoryFilter::Material, 5, 0.5f, 20));
	Items.Add(MakeInventoryItem(TEXT("가벼운 헬멧"), TEXT("모"), ECodexInventoryFilter::Equipment, 1, 2.0f, 95));
	Items.Add(MakeInventoryItem(TEXT("빈 슬롯"), TEXT("-"), ECodexInventoryFilter::Material, 0, 0.0f, 0, true));
	Items.Add(MakeInventoryItem(TEXT("오래된 지도"), TEXT("도"), ECodexInventoryFilter::Material, 1, 0.1f, 60));
	Items.Add(MakeInventoryItem(TEXT("정화 캡슐"), TEXT("캡"), ECodexInventoryFilter::Consumable, 3, 0.3f, 30));
	Items.Add(MakeInventoryItem(TEXT("광석 조각"), TEXT("광"), ECodexInventoryFilter::Material, 8, 0.9f, 16));
	Items.Add(MakeInventoryItem(TEXT("신호기"), TEXT("신"), ECodexInventoryFilter::Equipment, 1, 1.8f, 140));
	Items.Add(MakeInventoryItem(TEXT("고운 모래"), TEXT("사"), ECodexInventoryFilter::Material, 14, 0.1f, 3));
	Items.Add(MakeInventoryItem(TEXT("응급 붕대"), TEXT("붕"), ECodexInventoryFilter::Consumable, 5, 0.2f, 10));
	Items.Add(MakeInventoryItem(TEXT("잠긴 슬롯"), TEXT("잠"), ECodexInventoryFilter::Equipment, 0, 0.0f, 0, true));
	Items.Add(MakeInventoryItem(TEXT("수정 렌즈"), TEXT("렌"), ECodexInventoryFilter::Material, 2, 0.4f, 70));
	Items.Add(MakeInventoryItem(TEXT("강화 부츠"), TEXT("부"), ECodexInventoryFilter::Equipment, 1, 2.6f, 150));
	Items.Add(MakeInventoryItem(TEXT("바닷물 병"), TEXT("병"), ECodexInventoryFilter::Consumable, 7, 0.5f, 8));
	Items.Add(MakeInventoryItem(TEXT("기록 칩"), TEXT("칩"), ECodexInventoryFilter::Material, 4, 0.1f, 25));
	Items.Add(MakeInventoryItem(TEXT("다용도 로프"), TEXT("줄"), ECodexInventoryFilter::Equipment, 1, 1.2f, 55));
	Items.Add(MakeInventoryItem(TEXT("잠긴 슬롯"), TEXT("잠"), ECodexInventoryFilter::Consumable, 0, 0.0f, 0, true));
}

void UCodexUIKitInventoryGridWidget::RefreshInventory()
{
	if (!ItemScrollBox || !HeaderLabel || !FilterLabel)
	{
		return;
	}

	EnsureSelectionVisible();
	ItemScrollBox->ClearChildren();

	HeaderLabel->SetText(FText::FromString(FString::Printf(TEXT("인벤토리 %d/%d"), GetUnlockedCount(), Items.Num())));
	FilterLabel->SetText(CategoryText(ActiveFilter));

	UUniformGridPanel* Grid = WidgetTree->ConstructWidget<UUniformGridPanel>();
	const TArray<int32> VisibleIndexes = GetVisibleItemIndexes();
	for (int32 VisibleIndex = 0; VisibleIndex < VisibleIndexes.Num(); ++VisibleIndex)
	{
		const int32 ItemIndex = VisibleIndexes[VisibleIndex];
		const FCodexInventoryDemoItem& Item = Items[ItemIndex];
		const bool bSelected = ItemIndex == SelectedItemIndex;

		UCodexInventoryItemButton* CellButton = WidgetTree->ConstructWidget<UCodexInventoryItemButton>();
		CellButton->InitializeItemButton(this, ItemIndex);
		CellButton->SetIsEnabled(!Item.bLocked);
		CellButton->SetStyle(FCodexUIStyle::ButtonStyle(bSelected ? ECodexUIButtonKind::Accent : ECodexUIButtonKind::Neutral));

		UVerticalBox* CellStack = WidgetTree->ConstructWidget<UVerticalBox>();
		UTextBlock* IconLabel = InventoryText(*WidgetTree, Item.bLocked ? TEXT("잠김") : Item.IconLabel, 18.0f, Item.bLocked ? FCodexUIColor::TextMuted() : CategoryColor(Item.Category), TEXT("Bold"));
		IconLabel->SetJustification(ETextJustify::Center);
		AddInventoryVBox(*CellStack, IconLabel, FMargin(0.0f, 2.0f, 0.0f, 2.0f), HAlign_Center);

		UTextBlock* QtyLabel = InventoryText(*WidgetTree, Item.bLocked ? TEXT("-") : FString::Printf(TEXT("x%d"), Item.Quantity), FCodexUIFontSize::Caption, FCodexUIColor::TextSecondary(), TEXT("Bold"));
		QtyLabel->SetJustification(ETextJustify::Center);
		AddInventoryVBox(*CellStack, QtyLabel, FMargin(0.0f), HAlign_Center);

		CellButton->SetContent(CellStack);
		UUniformGridSlot* GridSlot = Grid->AddChildToUniformGrid(InventorySized(*WidgetTree, CellButton, 54.0f, 54.0f), VisibleIndex / InventoryColumns, VisibleIndex % InventoryColumns);
		GridSlot->SetHorizontalAlignment(HAlign_Center);
		GridSlot->SetVerticalAlignment(VAlign_Center);
	}

	ItemScrollBox->AddChild(Grid);
}

void UCodexUIKitInventoryGridWidget::RefreshDetail()
{
	if (!DetailBox)
	{
		return;
	}

	DetailBox->ClearChildren();
	if (!Items.IsValidIndex(SelectedItemIndex))
	{
		AddInventoryVBox(*DetailBox, InventoryText(*WidgetTree, TEXT("표시할 아이템이 없습니다."), FCodexUIFontSize::Body, FCodexUIColor::TextMuted()));
		return;
	}

	const FCodexInventoryDemoItem& Item = Items[SelectedItemIndex];
	UHorizontalBox* TitleRow = WidgetTree->ConstructWidget<UHorizontalBox>();
	AddInventoryHBox(*TitleRow, InventoryText(*WidgetTree, Item.Name, FCodexUIFontSize::Body, FCodexUIColor::TextPrimary(), TEXT("Bold")), FMargin(0.0f), 1.0f);
	AddInventoryHBox(*TitleRow, InventoryText(*WidgetTree, CategoryText(Item.Category).ToString(), FCodexUIFontSize::Badge, CategoryColor(Item.Category), TEXT("Bold")));
	AddInventoryVBox(*DetailBox, TitleRow, FMargin(0.0f, 0.0f, 0.0f, FCodexUISpace::S1));

	AddInventoryVBox(*DetailBox, InventoryText(
		*WidgetTree,
		FString::Printf(TEXT("수량 %d  무게 %.1fkg  가치 %d"), Item.Quantity, Item.Weight, Item.Value),
		FCodexUIFontSize::Caption,
		FCodexUIColor::TextSecondary()));
	AddInventoryVBox(*DetailBox, InventoryText(
		*WidgetTree,
		FString::Printf(TEXT("총 무게 %.1f / 40.0kg"), GetTotalWeight()),
		FCodexUIFontSize::Caption,
		FCodexUIColor::TextMuted()),
		FMargin(0.0f, FCodexUISpace::S1, 0.0f, 0.0f));
}

void UCodexUIKitInventoryGridWidget::SetFilter(ECodexInventoryFilter NewFilter)
{
	ActiveFilter = NewFilter;
	EnsureSelectionVisible();
	RefreshInventory();
	RefreshDetail();
}

void UCodexUIKitInventoryGridWidget::SelectNextVisibleItem(int32 Direction)
{
	const TArray<int32> SelectableIndexes = GetSelectableVisibleItemIndexes();
	if (SelectableIndexes.IsEmpty())
	{
		return;
	}

	int32 CurrentVisibleIndex = SelectableIndexes.IndexOfByKey(SelectedItemIndex);
	if (CurrentVisibleIndex == INDEX_NONE)
	{
		CurrentVisibleIndex = 0;
	}
	else
	{
		CurrentVisibleIndex = (CurrentVisibleIndex + Direction + SelectableIndexes.Num()) % SelectableIndexes.Num();
	}

	SelectedItemIndex = SelectableIndexes[CurrentVisibleIndex];
	RefreshInventory();
	RefreshDetail();
}

TArray<int32> UCodexUIKitInventoryGridWidget::GetVisibleItemIndexes() const
{
	TArray<int32> VisibleIndexes;
	for (int32 Index = 0; Index < Items.Num(); ++Index)
	{
		if (MatchesFilter(Items[Index]))
		{
			VisibleIndexes.Add(Index);
		}
	}
	return VisibleIndexes;
}

TArray<int32> UCodexUIKitInventoryGridWidget::GetSelectableVisibleItemIndexes() const
{
	TArray<int32> SelectableIndexes;
	for (int32 Index = 0; Index < Items.Num(); ++Index)
	{
		if (MatchesFilter(Items[Index]) && !Items[Index].bLocked)
		{
			SelectableIndexes.Add(Index);
		}
	}
	return SelectableIndexes;
}

bool UCodexUIKitInventoryGridWidget::MatchesFilter(const FCodexInventoryDemoItem& Item) const
{
	return ActiveFilter == ECodexInventoryFilter::All || Item.Category == ActiveFilter;
}

void UCodexUIKitInventoryGridWidget::EnsureSelectionVisible()
{
	if (Items.IsEmpty())
	{
		SelectedItemIndex = INDEX_NONE;
		return;
	}

	if (Items.IsValidIndex(SelectedItemIndex) && MatchesFilter(Items[SelectedItemIndex]) && !Items[SelectedItemIndex].bLocked)
	{
		return;
	}

	const TArray<int32> SelectableIndexes = GetSelectableVisibleItemIndexes();
	if (!SelectableIndexes.IsEmpty())
	{
		SelectedItemIndex = SelectableIndexes[0];
	}
}

float UCodexUIKitInventoryGridWidget::GetTotalWeight() const
{
	float TotalWeight = 0.0f;
	for (const FCodexInventoryDemoItem& Item : Items)
	{
		if (!Item.bLocked)
		{
			TotalWeight += Item.Weight * static_cast<float>(Item.Quantity);
		}
	}
	return TotalWeight;
}

int32 UCodexUIKitInventoryGridWidget::GetUnlockedCount() const
{
	int32 Count = 0;
	for (const FCodexInventoryDemoItem& Item : Items)
	{
		Count += Item.bLocked ? 0 : 1;
	}
	return Count;
}

FText UCodexUIKitInventoryGridWidget::CategoryText(ECodexInventoryFilter Category)
{
	switch (Category)
	{
	case ECodexInventoryFilter::Material:
		return NSLOCTEXT("CodexUIKit", "InventoryFilterMaterial", "재료");
	case ECodexInventoryFilter::Consumable:
		return NSLOCTEXT("CodexUIKit", "InventoryFilterConsumable", "소모품");
	case ECodexInventoryFilter::Equipment:
		return NSLOCTEXT("CodexUIKit", "InventoryFilterEquipment", "장비");
	case ECodexInventoryFilter::All:
	default:
		return NSLOCTEXT("CodexUIKit", "InventoryFilterAll", "전체");
	}
}

FLinearColor UCodexUIKitInventoryGridWidget::CategoryColor(ECodexInventoryFilter Category)
{
	switch (Category)
	{
	case ECodexInventoryFilter::Consumable:
		return FCodexUIColor::Primary();
	case ECodexInventoryFilter::Equipment:
		return FCodexUIColor::InfoDark();
	case ECodexInventoryFilter::Material:
		return FCodexUIColor::Accent();
	case ECodexInventoryFilter::All:
	default:
		return FCodexUIColor::TextSecondary();
	}
}

void UCodexUIKitInventoryGridWidget::HandleFilterAll()
{
	SetFilter(ECodexInventoryFilter::All);
}

void UCodexUIKitInventoryGridWidget::HandleFilterMaterial()
{
	SetFilter(ECodexInventoryFilter::Material);
}

void UCodexUIKitInventoryGridWidget::HandleFilterConsumable()
{
	SetFilter(ECodexInventoryFilter::Consumable);
}

void UCodexUIKitInventoryGridWidget::HandleFilterEquipment()
{
	SetFilter(ECodexInventoryFilter::Equipment);
}

void UCodexUIKitInventoryGridWidget::HandlePreviousItem()
{
	SelectNextVisibleItem(-1);
}

void UCodexUIKitInventoryGridWidget::HandleNextItem()
{
	SelectNextVisibleItem(1);
}

void UCodexUIKitInventoryGridWidget::HandleUseSelectedItem()
{
	if (!Items.IsValidIndex(SelectedItemIndex) || Items[SelectedItemIndex].bLocked)
	{
		return;
	}

	FCodexInventoryDemoItem& Item = Items[SelectedItemIndex];
	Item.Quantity = FMath::Max(Item.Quantity - 1, 0);
	if (Item.Quantity == 0)
	{
		Item.bLocked = true;
	}

	EnsureSelectionVisible();
	RefreshInventory();
	RefreshDetail();
}

void UCodexUIKitInventoryGridWidget::HandleToggleLockSelectedItem()
{
	if (!Items.IsValidIndex(SelectedItemIndex))
	{
		return;
	}

	FCodexInventoryDemoItem& Item = Items[SelectedItemIndex];
	Item.bLocked = !Item.bLocked;
	if (!Item.bLocked && Item.Quantity == 0)
	{
		Item.Quantity = 1;
	}

	EnsureSelectionVisible();
	RefreshInventory();
	RefreshDetail();
}
