#include "UI/CodexUIKitToastWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "UI/CodexUIStyle.h"

namespace
{
UTextBlock* MakeToastText(UWidgetTree& Tree, const FText& Text, float Size, const FLinearColor& Color, FName Typeface = NAME_None)
{
	UTextBlock* TextBlock = Tree.ConstructWidget<UTextBlock>();
	TextBlock->SetText(Text);
	TextBlock->SetColorAndOpacity(FSlateColor(Color));
	TextBlock->SetFont(FCodexUIStyle::Font(Size, Typeface));
	return TextBlock;
}

void AddToastVBox(UVerticalBox& Stack, UWidget* Child, const FMargin Padding = FMargin(0.0f))
{
	UVerticalBoxSlot* Slot = Stack.AddChildToVerticalBox(Child);
	Slot->SetPadding(Padding);
	Slot->SetHorizontalAlignment(HAlign_Fill);
}

void AddToastHBox(UHorizontalBox& Row, UWidget* Child, const FMargin Padding = FMargin(0.0f), float FillWidth = 0.0f)
{
	UHorizontalBoxSlot* Slot = Row.AddChildToHorizontalBox(Child);
	Slot->SetPadding(Padding);
	FSlateChildSize Size(FillWidth > 0.0f ? ESlateSizeRule::Fill : ESlateSizeRule::Automatic);
	Size.Value = FillWidth > 0.0f ? FillWidth : 1.0f;
	Slot->SetSize(Size);
}
}

void UCodexUIKitToastWidget::ConfigureToast(FText InTitle, FText InMessage, float InDuration)
{
	ToastTitle = MoveTemp(InTitle);
	ToastMessage = MoveTemp(InMessage);
	Duration = FMath::Max(InDuration, 0.1f);
	RefreshText();
}

TSharedRef<SWidget> UCodexUIKitToastWidget::RebuildWidget()
{
	UCanvasPanel* Root = WidgetTree->ConstructWidget<UCanvasPanel>();
	Root->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	WidgetTree->RootWidget = Root;

	UBorder* ToastPanel = WidgetTree->ConstructWidget<UBorder>();
	ToastPanel->SetPadding(FMargin(FCodexUISpace::S4, FCodexUISpace::S3));
	ToastPanel->SetBrush(FCodexUIStyle::RoundedBrush(
		FLinearColor(1.0f, 0.96f, 0.82f, 0.98f),
		FCodexUIRadius::LG,
		FCodexUIColor::BorderSoft(),
		1.0f));

	UHorizontalBox* Row = WidgetTree->ConstructWidget<UHorizontalBox>();
	ToastPanel->SetContent(Row);

	UBorder* AccentBar = WidgetTree->ConstructWidget<UBorder>();
	AccentBar->SetBrush(FCodexUIStyle::RoundedBrush(FCodexUIColor::Primary(), FCodexUIRadius::SM));
	USizeBox* AccentSizeBox = WidgetTree->ConstructWidget<USizeBox>();
	AccentSizeBox->SetWidthOverride(5.0f);
	AccentSizeBox->SetHeightOverride(56.0f);
	AccentSizeBox->AddChild(AccentBar);
	AddToastHBox(*Row, AccentSizeBox, FMargin(0.0f, 0.0f, FCodexUISpace::S3, 0.0f));

	UVerticalBox* TextStack = WidgetTree->ConstructWidget<UVerticalBox>();
	TitleTextBlock = MakeToastText(*WidgetTree, ToastTitle, FCodexUIFontSize::Body, FCodexUIColor::TextPrimary(), TEXT("Bold"));
	MessageTextBlock = MakeToastText(*WidgetTree, ToastMessage, FCodexUIFontSize::Caption, FCodexUIColor::TextSecondary());
	MessageTextBlock->SetWrapTextAt(250.0f);
	AddToastVBox(*TextStack, TitleTextBlock);
	AddToastVBox(*TextStack, MessageTextBlock, FMargin(0.0f, FCodexUISpace::S1, 0.0f, 0.0f));
	AddToastHBox(*Row, TextStack, FMargin(0.0f), 1.0f);

	UButton* CloseButton = WidgetTree->ConstructWidget<UButton>();
	CloseButton->SetStyle(FCodexUIStyle::ButtonStyle(ECodexUIButtonKind::Neutral));
	CloseButton->OnClicked.AddDynamic(this, &ThisClass::HandleDismiss);
	UTextBlock* CloseText = MakeToastText(*WidgetTree, FText::FromString(TEXT("x")), FCodexUIFontSize::Caption, FCodexUIColor::TextPrimary(), TEXT("Bold"));
	CloseText->SetJustification(ETextJustify::Center);
	CloseButton->SetContent(CloseText);
	AddToastHBox(*Row, CloseButton, FMargin(FCodexUISpace::S3, 0.0f, 0.0f, 0.0f));

	UCanvasPanelSlot* ToastSlot = Root->AddChildToCanvas(ToastPanel);
	ToastSlot->SetAnchors(FAnchors(1.0f, 0.0f));
	ToastSlot->SetAlignment(FVector2D(1.0f, 0.0f));
	ToastSlot->SetPosition(FVector2D(-32.0f, 32.0f));
	ToastSlot->SetSize(FVector2D(360.0f, 92.0f));

	RefreshText();
	return Super::RebuildWidget();
}

void UCodexUIKitToastWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(AutoDismissTimerHandle);
		World->GetTimerManager().SetTimer(AutoDismissTimerHandle, this, &ThisClass::HandleDismiss, Duration, false);
	}
}

void UCodexUIKitToastWidget::NativeDestruct()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(AutoDismissTimerHandle);
	}

	Super::NativeDestruct();
}

void UCodexUIKitToastWidget::RefreshText()
{
	if (TitleTextBlock)
	{
		TitleTextBlock->SetText(ToastTitle.IsEmpty() ? NSLOCTEXT("CodexUIKit", "ToastDefaultTitle", "토스트 메시지") : ToastTitle);
	}
	if (MessageTextBlock)
	{
		MessageTextBlock->SetText(ToastMessage.IsEmpty() ? NSLOCTEXT("CodexUIKit", "ToastDefaultMessage", "테스트 알림입니다.") : ToastMessage);
	}
}

void UCodexUIKitToastWidget::HandleDismiss()
{
	RemoveFromParent();
}
