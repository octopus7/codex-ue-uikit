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
#include "UI/CodexUIKitControlsPanelWidget.h"
#include "UI/CodexUIKitDemoWidget.h"
#include "UI/CodexUIKitInventoryGridWidget.h"
#include "UI/CodexUIKitPopupWidget.h"
#include "UI/CodexUIKitQuestBoardWidget.h"
#include "UI/CodexUIKitStandalonePanelWidget.h"
#include "UI/CodexUIKitToastWidget.h"
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

	UButton* QuestButton = MakeStartupButton(*WidgetTree, TEXT("퀘스트 보드 테스트"), ECodexUIButtonKind::Info);
	QuestButton->OnClicked.AddDynamic(this, &ThisClass::HandleQuestBoardClicked);
	AddStartupRow(*Stack, QuestButton, FMargin(0.0f, 0.0f, 0.0f, FCodexUISpace::S2));

	UButton* InventoryButton = MakeStartupButton(*WidgetTree, TEXT("인벤토리 패널 테스트"), ECodexUIButtonKind::Info);
	InventoryButton->OnClicked.AddDynamic(this, &ThisClass::HandleInventoryClicked);
	AddStartupRow(*Stack, InventoryButton, FMargin(0.0f, 0.0f, 0.0f, FCodexUISpace::S2));

	UButton* ControlsButton = MakeStartupButton(*WidgetTree, TEXT("컨트롤/슬라이더 테스트"), ECodexUIButtonKind::Info);
	ControlsButton->OnClicked.AddDynamic(this, &ThisClass::HandleControlsClicked);
	AddStartupRow(*Stack, ControlsButton, FMargin(0.0f, 0.0f, 0.0f, FCodexUISpace::S2));

	UButton* ConfirmButton = MakeStartupButton(*WidgetTree, TEXT("확인 팝업 테스트"), ECodexUIButtonKind::Accent);
	ConfirmButton->OnClicked.AddDynamic(this, &ThisClass::HandleConfirmPopupClicked);
	AddStartupRow(*Stack, ConfirmButton, FMargin(0.0f, 0.0f, 0.0f, FCodexUISpace::S2));

	UButton* RewardButton = MakeStartupButton(*WidgetTree, TEXT("보상 팝업 테스트"), ECodexUIButtonKind::Info);
	RewardButton->OnClicked.AddDynamic(this, &ThisClass::HandleRewardPopupClicked);
	AddStartupRow(*Stack, RewardButton, FMargin(0.0f, 0.0f, 0.0f, FCodexUISpace::S2));

	UButton* ToastButton = MakeStartupButton(*WidgetTree, TEXT("토스트 메시지 테스트"), ECodexUIButtonKind::Primary);
	ToastButton->OnClicked.AddDynamic(this, &ThisClass::HandleToastClicked);
	AddStartupRow(*Stack, ToastButton, FMargin(0.0f, 0.0f, 0.0f, FCodexUISpace::S2));

	UButton* CloseButton = MakeStartupButton(*WidgetTree, TEXT("런처 닫기"), ECodexUIButtonKind::Neutral);
	CloseButton->OnClicked.AddDynamic(this, &ThisClass::HandleCloseClicked);
	AddStartupRow(*Stack, CloseButton);

	UCanvasPanelSlot* PanelSlot = Root->AddChildToCanvas(Panel);
	PanelSlot->SetAnchors(FAnchors(0.0f, 0.0f));
	PanelSlot->SetPosition(FVector2D(32.0f, 32.0f));
	PanelSlot->SetSize(FVector2D(320.0f, 405.0f));

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
}

void UCodexUIKitStartupWidget::HandleQuestBoardClicked()
{
	ShowStandalonePanel(
		NSLOCTEXT("CodexUIKit", "StartupQuestBoardTitle", "퀘스트 보드 테스트"),
		UCodexUIKitQuestBoardStandaloneWidget::StaticClass(),
		FVector2D(1120.0f, 640.0f));
}

void UCodexUIKitStartupWidget::HandleInventoryClicked()
{
	ShowStandalonePanel(
		NSLOCTEXT("CodexUIKit", "StartupInventoryTitle", "인벤토리 패널 테스트"),
		UCodexUIKitInventoryGridWidget::StaticClass(),
		FVector2D(340.0f, 416.0f));
}

void UCodexUIKitStartupWidget::HandleControlsClicked()
{
	ShowStandalonePanel(
		NSLOCTEXT("CodexUIKit", "StartupControlsTitle", "컨트롤/슬라이더 테스트"),
		UCodexUIKitControlsPanelWidget::StaticClass(),
		FVector2D(930.0f, 218.0f));
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

void UCodexUIKitStartupWidget::HandleToastClicked()
{
	++ToastSequence;
	ShowToast(
		NSLOCTEXT("CodexUIKit", "StartupToastTitle", "토스트 메시지"),
		FText::FromString(FString::Printf(TEXT("테스트 알림 %d번입니다. 잠시 후 자동으로 닫힙니다."), ToastSequence)));
}

void UCodexUIKitStartupWidget::HandleCloseClicked()
{
	RemoveFromParent();
}

void UCodexUIKitStartupWidget::ShowStandalonePanel(const FText& Title, TSubclassOf<UUserWidget> ContentWidgetClass, FVector2D ContentSize)
{
	if (ActiveStandalonePanelWidget && ActiveStandalonePanelWidget->IsInViewport())
	{
		ActiveStandalonePanelWidget->RemoveFromParent();
	}

	ActiveStandalonePanelWidget = CreateWidget<UCodexUIKitStandalonePanelWidget>(GetOwningPlayer(), UCodexUIKitStandalonePanelWidget::StaticClass());
	if (!ActiveStandalonePanelWidget)
	{
		return;
	}

	ActiveStandalonePanelWidget->ConfigurePanel(Title, ContentWidgetClass, ContentSize);
	ActiveStandalonePanelWidget->AddToViewport(20);
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

void UCodexUIKitStartupWidget::ShowToast(const FText& Title, const FText& Message)
{
	if (ActiveToastWidget && ActiveToastWidget->IsInViewport())
	{
		ActiveToastWidget->RemoveFromParent();
	}

	ActiveToastWidget = CreateWidget<UCodexUIKitToastWidget>(GetOwningPlayer(), UCodexUIKitToastWidget::StaticClass());
	if (!ActiveToastWidget)
	{
		return;
	}

	ActiveToastWidget->ConfigureToast(Title, Message, 3.5f);
	ActiveToastWidget->AddToViewport(60);
}
