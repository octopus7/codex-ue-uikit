#include "UI/CodexUIKitStandalonePanelWidget.h"

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
#include "UI/CodexUIStyle.h"

namespace
{
UTextBlock* MakeStandaloneText(UWidgetTree& Tree, const FText& Text, float Size, const FLinearColor& Color, FName Typeface = NAME_None)
{
	UTextBlock* TextBlock = Tree.ConstructWidget<UTextBlock>();
	TextBlock->SetText(Text);
	TextBlock->SetColorAndOpacity(FSlateColor(Color));
	TextBlock->SetFont(FCodexUIStyle::Font(Size, Typeface));
	return TextBlock;
}

UButton* MakeStandaloneButton(UWidgetTree& Tree, const FString& Label, ECodexUIButtonKind Kind)
{
	UButton* Button = Tree.ConstructWidget<UButton>();
	Button->SetStyle(FCodexUIStyle::ButtonStyle(Kind));

	UTextBlock* LabelText = MakeStandaloneText(
		Tree,
		FText::FromString(Label),
		FCodexUIFontSize::Body,
		Kind == ECodexUIButtonKind::Neutral ? FCodexUIColor::TextPrimary() : FCodexUIColor::TextInverse(),
		TEXT("Bold"));
	LabelText->SetJustification(ETextJustify::Center);
	Button->SetContent(LabelText);
	return Button;
}

void AddStandaloneVBox(UVerticalBox& Stack, UWidget* Child, const FMargin Padding = FMargin(0.0f), EHorizontalAlignment Align = HAlign_Fill)
{
	UVerticalBoxSlot* Slot = Stack.AddChildToVerticalBox(Child);
	Slot->SetPadding(Padding);
	Slot->SetHorizontalAlignment(Align);
}

void AddStandaloneHBox(UHorizontalBox& Row, UWidget* Child, const FMargin Padding = FMargin(0.0f), float FillWidth = 0.0f)
{
	UHorizontalBoxSlot* Slot = Row.AddChildToHorizontalBox(Child);
	Slot->SetPadding(Padding);
	FSlateChildSize Size(FillWidth > 0.0f ? ESlateSizeRule::Fill : ESlateSizeRule::Automatic);
	Size.Value = FillWidth > 0.0f ? FillWidth : 1.0f;
	Slot->SetSize(Size);
}

USizeBox* MakeStandaloneSized(UWidgetTree& Tree, UWidget* Content, float Width, float Height)
{
	USizeBox* SizeBox = Tree.ConstructWidget<USizeBox>();
	SizeBox->SetWidthOverride(Width);
	SizeBox->SetHeightOverride(Height);
	SizeBox->AddChild(Content);
	return SizeBox;
}
}

void UCodexUIKitStandalonePanelWidget::ConfigurePanel(FText InTitle, TSubclassOf<UUserWidget> InContentWidgetClass, FVector2D InContentSize)
{
	PanelTitle = MoveTemp(InTitle);
	ContentWidgetClass = InContentWidgetClass;
	ContentSize = InContentSize;
	RebuildContent();
}

TSharedRef<SWidget> UCodexUIKitStandalonePanelWidget::RebuildWidget()
{
	UCanvasPanel* Root = WidgetTree->ConstructWidget<UCanvasPanel>();
	Root->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	WidgetTree->RootWidget = Root;

	UBorder* Panel = WidgetTree->ConstructWidget<UBorder>();
	Panel->SetPadding(FMargin(FCodexUISpace::S4));
	Panel->SetBrush(FCodexUIStyle::RoundedBrush(
		FCodexUIColor::SurfaceBase(),
		FCodexUIRadius::LG,
		FCodexUIColor::BorderSoft(),
		1.0f));

	UVerticalBox* Stack = WidgetTree->ConstructWidget<UVerticalBox>();
	Panel->SetContent(Stack);

	UHorizontalBox* HeaderRow = WidgetTree->ConstructWidget<UHorizontalBox>();
	const FText Title = PanelTitle.IsEmpty() ? NSLOCTEXT("CodexUIKit", "StandalonePanelDefaultTitle", "패널 테스트") : PanelTitle;
	AddStandaloneHBox(*HeaderRow, MakeStandaloneText(*WidgetTree, Title, FCodexUIFontSize::H2, FCodexUIColor::TextPrimary(), TEXT("Bold")), FMargin(0.0f), 1.0f);

	UButton* CloseButton = MakeStandaloneButton(*WidgetTree, TEXT("닫기"), ECodexUIButtonKind::Neutral);
	CloseButton->OnClicked.AddDynamic(this, &ThisClass::HandleCloseClicked);
	AddStandaloneHBox(*HeaderRow, MakeStandaloneSized(*WidgetTree, CloseButton, 74.0f, 34.0f));
	AddStandaloneVBox(*Stack, HeaderRow, FMargin(0.0f, 0.0f, 0.0f, FCodexUISpace::S3));

	RebuildContent();
	if (ContentWidget)
	{
		AddStandaloneVBox(*Stack, MakeStandaloneSized(*WidgetTree, ContentWidget, ContentSize.X, ContentSize.Y));
	}
	else
	{
		AddStandaloneVBox(*Stack, MakeStandaloneText(
			*WidgetTree,
			NSLOCTEXT("CodexUIKit", "StandalonePanelMissingContent", "열 수 있는 패널 위젯이 없습니다."),
			FCodexUIFontSize::Body,
			FCodexUIColor::TextSecondary()));
	}

	const FVector2D PanelSize(ContentSize.X + 32.0f, ContentSize.Y + 84.0f);
	UCanvasPanelSlot* PanelSlot = Root->AddChildToCanvas(Panel);
	PanelSlot->SetAnchors(FAnchors(0.5f, 0.5f));
	PanelSlot->SetAlignment(FVector2D(0.5f, 0.5f));
	PanelSlot->SetPosition(FVector2D::ZeroVector);
	PanelSlot->SetSize(PanelSize);

	return Super::RebuildWidget();
}

void UCodexUIKitStandalonePanelWidget::RebuildContent()
{
	if (!WidgetTree || !ContentWidgetClass)
	{
		ContentWidget = nullptr;
		return;
	}

	ContentWidget = CreateWidget<UUserWidget>(GetOwningPlayer(), ContentWidgetClass);
}

void UCodexUIKitStandalonePanelWidget::HandleCloseClicked()
{
	RemoveFromParent();
}
