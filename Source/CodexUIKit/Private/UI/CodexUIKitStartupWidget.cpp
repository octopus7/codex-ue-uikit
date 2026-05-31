#include "UI/CodexUIKitStartupWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "UI/CodexUIKitDemoWidget.h"
#include "UI/CodexUIKitPopupWidget.h"
#include "UI/CodexUIStyle.h"

namespace
{
UTextBlock* MakeStartupText(UWidgetTree& Tree, const FString& Text, float Size, const FLinearColor& Color, FName Typeface = NAME_None)
{
	UTextBlock* TextBlock = Tree.ConstructWidget<UTextBlock>();
	TextBlock->SetText(FText::FromString(Text));
	TextBlock->SetColorAndOpacity(FSlateColor(Color));
	TextBlock->SetFont(FCodexUIStyle::Font(Size, Typeface));
	TextBlock->SetWrapTextAt(300.0f);
	return TextBlock;
}

UButton* MakeStartupButton(UWidgetTree& Tree, const FString& Label, ECodexUIButtonKind Kind)
{
	UButton* Button = Tree.ConstructWidget<UButton>();
	Button->SetStyle(FCodexUIStyle::ButtonStyle(Kind));

	UTextBlock* LabelText = MakeStartupText(
		Tree,
		Label,
		FCodexUIFontSize::Body,
		Kind == ECodexUIButtonKind::Neutral ? FCodexUIColor::TextPrimary() : FCodexUIColor::TextInverse(),
		TEXT("Bold"));
	LabelText->SetJustification(ETextJustify::Center);
	Button->SetContent(LabelText);
	return Button;
}

void AddStartupRow(UVerticalBox& Stack, UWidget* Child, const FMargin Padding = FMargin(0.0f))
{
	UVerticalBoxSlot* RowSlot = Stack.AddChildToVerticalBox(Child);
	RowSlot->SetPadding(Padding);
	RowSlot->SetHorizontalAlignment(HAlign_Fill);
}
}

TSharedRef<SWidget> UCodexUIKitStartupWidget::RebuildWidget()
{
	UCanvasPanel* Root = WidgetTree->ConstructWidget<UCanvasPanel>();
	WidgetTree->RootWidget = Root;

	UBorder* Panel = WidgetTree->ConstructWidget<UBorder>();
	Panel->SetPadding(FMargin(FCodexUISpace::S5));
	Panel->SetBrush(FCodexUIStyle::RoundedBrush(
		FCodexUIColor::SurfaceBase(),
		FCodexUIRadius::LG,
		FCodexUIColor::BorderSoft(),
		1.0f));

	UVerticalBox* Stack = WidgetTree->ConstructWidget<UVerticalBox>();
	Panel->SetContent(Stack);

	AddStartupRow(*Stack, MakeStartupText(*WidgetTree, TEXT("Codex UIKit"), FCodexUIFontSize::H2, FCodexUIColor::TextPrimary(), TEXT("Bold")));
	AddStartupRow(
		*Stack,
		MakeStartupText(*WidgetTree, TEXT("테스트할 UI를 선택하세요."), FCodexUIFontSize::Caption, FCodexUIColor::TextSecondary()),
		FMargin(0.0f, FCodexUISpace::S1, 0.0f, FCodexUISpace::S4));

	UButton* DemoButton = MakeStartupButton(*WidgetTree, TEXT("UI Kit Demo 열기"), ECodexUIButtonKind::Primary);
	DemoButton->OnClicked.AddDynamic(this, &ThisClass::HandleOpenDemoClicked);
	AddStartupRow(*Stack, DemoButton, FMargin(0.0f, 0.0f, 0.0f, FCodexUISpace::S2));

	UButton* ConfirmButton = MakeStartupButton(*WidgetTree, TEXT("확인 팝업 테스트"), ECodexUIButtonKind::Accent);
	ConfirmButton->OnClicked.AddDynamic(this, &ThisClass::HandleConfirmPopupClicked);
	AddStartupRow(*Stack, ConfirmButton, FMargin(0.0f, 0.0f, 0.0f, FCodexUISpace::S2));

	UButton* RewardButton = MakeStartupButton(*WidgetTree, TEXT("보상 팝업 테스트"), ECodexUIButtonKind::Info);
	RewardButton->OnClicked.AddDynamic(this, &ThisClass::HandleRewardPopupClicked);
	AddStartupRow(*Stack, RewardButton, FMargin(0.0f, 0.0f, 0.0f, FCodexUISpace::S2));

	UButton* CloseButton = MakeStartupButton(*WidgetTree, TEXT("런처 닫기"), ECodexUIButtonKind::Neutral);
	CloseButton->OnClicked.AddDynamic(this, &ThisClass::HandleCloseClicked);
	AddStartupRow(*Stack, CloseButton);

	UCanvasPanelSlot* PanelSlot = Root->AddChildToCanvas(Panel);
	PanelSlot->SetAnchors(FAnchors(0.0f, 0.0f));
	PanelSlot->SetPosition(FVector2D(32.0f, 32.0f));
	PanelSlot->SetSize(FVector2D(320.0f, 245.0f));

	return Super::RebuildWidget();
}

void UCodexUIKitStartupWidget::HandleOpenDemoClicked()
{
	if (!ActiveDemoWidget)
	{
		ActiveDemoWidget = CreateWidget<UCodexUIKitDemoWidget>(GetOwningPlayer(), UCodexUIKitDemoWidget::StaticClass());
	}

	if (ActiveDemoWidget && !ActiveDemoWidget->IsInViewport())
	{
		ActiveDemoWidget->AddToViewport(10);
	}

	RemoveFromParent();
}

void UCodexUIKitStartupWidget::HandleConfirmPopupClicked()
{
	ShowPopup(
		NSLOCTEXT("CodexUIKit", "StartupConfirmPopupTitle", "이동 확인"),
		NSLOCTEXT("CodexUIKit", "StartupConfirmPopupMessage", "확인 또는 취소를 누르면 이 팝업만 닫힙니다."),
		NSLOCTEXT("CodexUIKit", "StartupConfirmPopupConfirm", "확인"),
		NSLOCTEXT("CodexUIKit", "StartupConfirmPopupCancel", "취소"),
		true);
}

void UCodexUIKitStartupWidget::HandleRewardPopupClicked()
{
	ShowPopup(
		NSLOCTEXT("CodexUIKit", "StartupRewardPopupTitle", "획득 완료"),
		NSLOCTEXT("CodexUIKit", "StartupRewardPopupMessage", "보상 획득처럼 보이지만 외부 상태는 변경하지 않습니다."),
		NSLOCTEXT("CodexUIKit", "StartupRewardPopupConfirm", "닫기"),
		FText::GetEmpty(),
		false);
}

void UCodexUIKitStartupWidget::HandleCloseClicked()
{
	RemoveFromParent();
}

void UCodexUIKitStartupWidget::ShowPopup(const FText& Title, const FText& Message, const FText& Confirm, const FText& Cancel, bool bShowCancel)
{
	if (ActivePopupWidget && ActivePopupWidget->IsInViewport())
	{
		ActivePopupWidget->RemoveFromParent();
	}

	ActivePopupWidget = CreateWidget<UCodexUIKitPopupWidget>(GetOwningPlayer(), UCodexUIKitPopupWidget::StaticClass());
	if (!ActivePopupWidget)
	{
		return;
	}

	ActivePopupWidget->ConfigurePopup(Title, Message, Confirm, Cancel, bShowCancel);
	ActivePopupWidget->AddToViewport(30);
}
