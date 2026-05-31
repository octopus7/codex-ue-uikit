#include "UI/CodexUIKitPopupWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/SizeBox.h"
#include "Components/Spacer.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "UI/CodexUIStyle.h"

namespace
{
UTextBlock* MakePopupText(UWidgetTree& Tree, const FText& Text, float Size, const FLinearColor& Color, FName Typeface = NAME_None)
{
	UTextBlock* TextBlock = Tree.ConstructWidget<UTextBlock>();
	TextBlock->SetText(Text);
	TextBlock->SetColorAndOpacity(FSlateColor(Color));
	TextBlock->SetFont(FCodexUIStyle::Font(Size, Typeface));
	TextBlock->SetWrapTextAt(300.0f);
	return TextBlock;
}

UButton* MakePopupButton(UWidgetTree& Tree, const FText& Label, ECodexUIButtonKind Kind, UTextBlock*& OutLabel)
{
	UButton* Button = Tree.ConstructWidget<UButton>();
	Button->SetStyle(FCodexUIStyle::ButtonStyle(Kind));

	OutLabel = MakePopupText(
		Tree,
		Label,
		FCodexUIFontSize::Body,
		Kind == ECodexUIButtonKind::Neutral ? FCodexUIColor::TextPrimary() : FCodexUIColor::TextInverse(),
		TEXT("Bold"));
	OutLabel->SetJustification(ETextJustify::Center);
	Button->SetContent(OutLabel);
	return Button;
}
}

void UCodexUIKitPopupWidget::ConfigurePopup(
	const FText& InTitle,
	const FText& InMessage,
	const FText& InConfirmText,
	const FText& InCancelText,
	bool bInShowCancel)
{
	TitleText = InTitle;
	MessageText = InMessage;
	ConfirmText = InConfirmText;
	CancelText = InCancelText;
	bShowCancel = bInShowCancel;
	SyncText();
}

TSharedRef<SWidget> UCodexUIKitPopupWidget::RebuildWidget()
{
	UOverlay* Root = WidgetTree->ConstructWidget<UOverlay>();
	WidgetTree->RootWidget = Root;

	UBorder* Dimmer = WidgetTree->ConstructWidget<UBorder>();
	Dimmer->SetBrush(FCodexUIStyle::RoundedBrush(FLinearColor(0.0f, 0.0f, 0.0f, 0.28f), 0.0f));
	UOverlaySlot* DimmerSlot = Root->AddChildToOverlay(Dimmer);
	DimmerSlot->SetHorizontalAlignment(HAlign_Fill);
	DimmerSlot->SetVerticalAlignment(VAlign_Fill);

	USizeBox* PanelSize = WidgetTree->ConstructWidget<USizeBox>();
	PanelSize->SetWidthOverride(380.0f);

	UBorder* Panel = WidgetTree->ConstructWidget<UBorder>();
	Panel->SetPadding(FMargin(FCodexUISpace::S6));
	Panel->SetBrush(FCodexUIStyle::RoundedBrush(
		FCodexUIColor::SurfaceBase(),
		FCodexUIRadius::LG,
		FCodexUIColor::BorderSoft(),
		1.0f));
	PanelSize->AddChild(Panel);

	UOverlaySlot* PanelSlot = Root->AddChildToOverlay(PanelSize);
	PanelSlot->SetHorizontalAlignment(HAlign_Center);
	PanelSlot->SetVerticalAlignment(VAlign_Center);

	UVerticalBox* Stack = WidgetTree->ConstructWidget<UVerticalBox>();
	Panel->SetContent(Stack);

	UHorizontalBox* Header = WidgetTree->ConstructWidget<UHorizontalBox>();
	UVerticalBoxSlot* HeaderSlot = Stack->AddChildToVerticalBox(Header);
	HeaderSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, FCodexUISpace::S3));
	HeaderSlot->SetHorizontalAlignment(HAlign_Fill);

	TitleLabel = MakePopupText(*WidgetTree, TitleText, FCodexUIFontSize::H2, FCodexUIColor::TextPrimary(), TEXT("Bold"));
	UHorizontalBoxSlot* TitleSlot = Header->AddChildToHorizontalBox(TitleLabel);
	FSlateChildSize FillSize(ESlateSizeRule::Fill);
	FillSize.Value = 1.0f;
	TitleSlot->SetSize(FillSize);
	TitleSlot->SetVerticalAlignment(VAlign_Center);

	UTextBlock* CloseLabel = nullptr;
	CloseButton = MakePopupButton(*WidgetTree, FText::FromString(TEXT("X")), ECodexUIButtonKind::Neutral, CloseLabel);
	CloseButton->OnClicked.AddDynamic(this, &ThisClass::HandleCloseClicked);
	UHorizontalBoxSlot* CloseSlot = Header->AddChildToHorizontalBox(CloseButton);
	CloseSlot->SetSize(FSlateChildSize(ESlateSizeRule::Automatic));
	CloseSlot->SetVerticalAlignment(VAlign_Center);

	UTextBlock* Icon = MakePopupText(*WidgetTree, FText::FromString(TEXT("!")), 28.0f, FCodexUIColor::Warning(), TEXT("Bold"));
	Icon->SetJustification(ETextJustify::Center);
	UVerticalBoxSlot* IconSlot = Stack->AddChildToVerticalBox(Icon);
	IconSlot->SetHorizontalAlignment(HAlign_Center);
	IconSlot->SetPadding(FMargin(0.0f, FCodexUISpace::S2, 0.0f, FCodexUISpace::S3));

	MessageLabel = MakePopupText(*WidgetTree, MessageText, FCodexUIFontSize::Body, FCodexUIColor::TextSecondary());
	MessageLabel->SetJustification(ETextJustify::Center);
	UVerticalBoxSlot* MessageSlot = Stack->AddChildToVerticalBox(MessageLabel);
	MessageSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, FCodexUISpace::S5));
	MessageSlot->SetHorizontalAlignment(HAlign_Fill);

	UHorizontalBox* Buttons = WidgetTree->ConstructWidget<UHorizontalBox>();
	UVerticalBoxSlot* ButtonsSlot = Stack->AddChildToVerticalBox(Buttons);
	ButtonsSlot->SetHorizontalAlignment(HAlign_Fill);

	UTextBlock* RawCancelLabel = nullptr;
	CancelButton = MakePopupButton(*WidgetTree, CancelText, ECodexUIButtonKind::Neutral, RawCancelLabel);
	CancelLabel = RawCancelLabel;
	CancelButton->OnClicked.AddDynamic(this, &ThisClass::HandleCloseClicked);
	UHorizontalBoxSlot* CancelSlot = Buttons->AddChildToHorizontalBox(CancelButton);
	CancelSlot->SetSize(FillSize);
	CancelSlot->SetPadding(FMargin(0.0f, 0.0f, FCodexUISpace::S2, 0.0f));

	UTextBlock* RawConfirmLabel = nullptr;
	ConfirmButton = MakePopupButton(*WidgetTree, ConfirmText, ECodexUIButtonKind::Primary, RawConfirmLabel);
	ConfirmLabel = RawConfirmLabel;
	ConfirmButton->OnClicked.AddDynamic(this, &ThisClass::HandleCloseClicked);
	UHorizontalBoxSlot* ConfirmSlot = Buttons->AddChildToHorizontalBox(ConfirmButton);
	ConfirmSlot->SetSize(FillSize);
	ConfirmSlot->SetPadding(FMargin(FCodexUISpace::S2, 0.0f, 0.0f, 0.0f));

	SyncText();
	return Super::RebuildWidget();
}

void UCodexUIKitPopupWidget::SyncText()
{
	if (TitleLabel)
	{
		TitleLabel->SetText(TitleText);
	}
	if (MessageLabel)
	{
		MessageLabel->SetText(MessageText);
	}
	if (ConfirmLabel)
	{
		ConfirmLabel->SetText(ConfirmText);
	}
	if (CancelLabel)
	{
		CancelLabel->SetText(CancelText);
	}
	if (CancelButton)
	{
		CancelButton->SetVisibility(bShowCancel ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}
}

void UCodexUIKitPopupWidget::HandleCloseClicked()
{
	RemoveFromParent();
}
