#include "UI/CodexUIKitControlsPanelWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/ProgressBar.h"
#include "Components/ScrollBox.h"
#include "Components/SizeBox.h"
#include "Components/Slider.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "UI/CodexUIStyle.h"

namespace
{
UTextBlock* ControlText(UWidgetTree& Tree, const FString& Text, float Size, const FLinearColor& Color, FName Typeface = NAME_None)
{
	UTextBlock* TextBlock = Tree.ConstructWidget<UTextBlock>();
	TextBlock->SetText(FText::FromString(Text));
	TextBlock->SetFont(FCodexUIStyle::Font(Size, Typeface));
	TextBlock->SetColorAndOpacity(FSlateColor(Color));
	return TextBlock;
}

UBorder* ControlPanel(UWidgetTree& Tree, const FLinearColor& Fill = FCodexUIColor::SurfaceBase(), float Radius = FCodexUIRadius::LG)
{
	UBorder* Border = Tree.ConstructWidget<UBorder>();
	Border->SetPadding(FMargin(FCodexUISpace::S4));
	Border->SetBrush(FCodexUIStyle::RoundedBrush(Fill, Radius, FCodexUIColor::BorderSoft(), 1.0f));
	return Border;
}

UButton* ControlButton(UWidgetTree& Tree, const FString& Label, ECodexUIButtonKind Kind)
{
	UButton* Button = Tree.ConstructWidget<UButton>();
	Button->SetStyle(FCodexUIStyle::ButtonStyle(Kind));
	UTextBlock* LabelText = ControlText(
		Tree,
		Label,
		FCodexUIFontSize::Caption,
		Kind == ECodexUIButtonKind::Neutral ? FCodexUIColor::TextPrimary() : FCodexUIColor::TextInverse(),
		TEXT("Bold"));
	LabelText->SetJustification(ETextJustify::Center);
	Button->SetContent(LabelText);
	return Button;
}

USizeBox* ControlSized(UWidgetTree& Tree, UWidget* Content, float Width, float Height)
{
	USizeBox* SizeBox = Tree.ConstructWidget<USizeBox>();
	SizeBox->SetWidthOverride(Width);
	SizeBox->SetHeightOverride(Height);
	SizeBox->AddChild(Content);
	return SizeBox;
}

void AddControlVBox(UVerticalBox& Box, UWidget* Child, const FMargin Padding = FMargin(0.0f), EHorizontalAlignment Align = HAlign_Fill)
{
	UVerticalBoxSlot* Slot = Box.AddChildToVerticalBox(Child);
	Slot->SetPadding(Padding);
	Slot->SetHorizontalAlignment(Align);
}

void AddControlHBox(UHorizontalBox& Box, UWidget* Child, const FMargin Padding = FMargin(0.0f), float FillWidth = 0.0f)
{
	UHorizontalBoxSlot* Slot = Box.AddChildToHorizontalBox(Child);
	Slot->SetPadding(Padding);
	FSlateChildSize Size(FillWidth > 0.0f ? ESlateSizeRule::Fill : ESlateSizeRule::Automatic);
	Size.Value = FillWidth > 0.0f ? FillWidth : 1.0f;
	Slot->SetSize(Size);
}

USlider* MakeSlider(UWidgetTree& Tree, float Value)
{
	USlider* Slider = Tree.ConstructWidget<USlider>();
	Slider->SetValue(Value);
	Slider->SetStepSize(0.01f);
	Slider->SetSliderBarColor(FCodexUIColor::InfoDark());
	Slider->SetSliderHandleColor(FCodexUIColor::Primary());
	return Slider;
}

UWidget* MakeSliderRow(
	UWidgetTree& Tree,
	const FString& Label,
	float Value,
	TObjectPtr<UTextBlock>& OutValueLabel,
	const FScriptDelegate& ChangeDelegate)
{
	UHorizontalBox* Row = Tree.ConstructWidget<UHorizontalBox>();
	AddControlHBox(*Row, ControlText(Tree, Label, FCodexUIFontSize::Body, FCodexUIColor::TextPrimary(), TEXT("Bold")), FMargin(0.0f), 0.25f);

	USlider* Slider = MakeSlider(Tree, Value);
	Slider->OnValueChanged.Add(ChangeDelegate);
	AddControlHBox(*Row, ControlSized(Tree, Slider, 280.0f, 26.0f), FMargin(FCodexUISpace::S2, 0.0f), 1.0f);

	UTextBlock* ValueLabel = ControlText(Tree, TEXT("0"), FCodexUIFontSize::Caption, FCodexUIColor::TextSecondary(), TEXT("Bold"));
	ValueLabel->SetJustification(ETextJustify::Right);
	OutValueLabel = ValueLabel;
	AddControlHBox(*Row, ControlSized(Tree, ValueLabel, 46.0f, 22.0f));
	return Row;
}
}

void UCodexUIKitControlsPanelWidget::ResetDemoControls()
{
	BgmValue = 0.68f;
	SfxValue = 0.74f;
	ZoomValue = 0.38f;
	Notices.Reset();
	NoticeSequence = 0;
	SeedNoticesIfNeeded();
	RefreshSliderLabels();
	RefreshNotices();
}

TSharedRef<SWidget> UCodexUIKitControlsPanelWidget::RebuildWidget()
{
	SeedNoticesIfNeeded();

	UBorder* RootPanel = ControlPanel(*WidgetTree);
	WidgetTree->RootWidget = RootPanel;

	UHorizontalBox* RootRow = WidgetTree->ConstructWidget<UHorizontalBox>();
	RootPanel->SetContent(RootRow);

	UVerticalBox* SliderColumn = WidgetTree->ConstructWidget<UVerticalBox>();
	AddControlVBox(*SliderColumn, ControlText(*WidgetTree, TEXT("컨트롤 테스트"), FCodexUIFontSize::H3, FCodexUIColor::TextPrimary(), TEXT("Bold")), FMargin(0.0f, 0.0f, 0.0f, FCodexUISpace::S2));

	FScriptDelegate BgmDelegate;
	BgmDelegate.BindUFunction(this, GET_FUNCTION_NAME_CHECKED(UCodexUIKitControlsPanelWidget, HandleBgmChanged));
	AddControlVBox(*SliderColumn, MakeSliderRow(*WidgetTree, TEXT("BGM"), BgmValue, BgmValueLabel, BgmDelegate), FMargin(0.0f, 0.0f, 0.0f, FCodexUISpace::S1));

	FScriptDelegate SfxDelegate;
	SfxDelegate.BindUFunction(this, GET_FUNCTION_NAME_CHECKED(UCodexUIKitControlsPanelWidget, HandleSfxChanged));
	AddControlVBox(*SliderColumn, MakeSliderRow(*WidgetTree, TEXT("효과음"), SfxValue, SfxValueLabel, SfxDelegate), FMargin(0.0f, 0.0f, 0.0f, FCodexUISpace::S1));

	FScriptDelegate ZoomDelegate;
	ZoomDelegate.BindUFunction(this, GET_FUNCTION_NAME_CHECKED(UCodexUIKitControlsPanelWidget, HandleZoomChanged));
	AddControlVBox(*SliderColumn, MakeSliderRow(*WidgetTree, TEXT("줌"), ZoomValue, ZoomValueLabel, ZoomDelegate), FMargin(0.0f, 0.0f, 0.0f, FCodexUISpace::S2));

	UHorizontalBox* ButtonRow = WidgetTree->ConstructWidget<UHorizontalBox>();
	UButton* ResetButton = ControlButton(*WidgetTree, TEXT("초기화"), ECodexUIButtonKind::Neutral);
	ResetButton->OnClicked.AddDynamic(this, &ThisClass::HandleResetControls);
	UButton* AddNoticeButton = ControlButton(*WidgetTree, TEXT("알림 추가"), ECodexUIButtonKind::Primary);
	AddNoticeButton->OnClicked.AddDynamic(this, &ThisClass::HandleAddNotice);
	AddControlHBox(*ButtonRow, ControlSized(*WidgetTree, ResetButton, 82.0f, 32.0f), FMargin(0.0f, 0.0f, FCodexUISpace::S2, 0.0f));
	AddControlHBox(*ButtonRow, ControlSized(*WidgetTree, AddNoticeButton, 96.0f, 32.0f));
	AddControlVBox(*SliderColumn, ButtonRow);
	AddControlHBox(*RootRow, SliderColumn, FMargin(0.0f, 0.0f, FCodexUISpace::S5, 0.0f), 1.0f);

	UVerticalBox* NoticeColumn = WidgetTree->ConstructWidget<UVerticalBox>();
	UHorizontalBox* NoticeHeaderRow = WidgetTree->ConstructWidget<UHorizontalBox>();
	NoticeHeaderLabel = ControlText(*WidgetTree, TEXT("알림 로그"), FCodexUIFontSize::H3, FCodexUIColor::TextPrimary(), TEXT("Bold"));
	AddControlHBox(*NoticeHeaderRow, NoticeHeaderLabel, FMargin(0.0f), 1.0f);
	UButton* TopButton = ControlButton(*WidgetTree, TEXT("위"), ECodexUIButtonKind::Neutral);
	TopButton->OnClicked.AddDynamic(this, &ThisClass::HandleScrollNoticeTop);
	UButton* BottomButton = ControlButton(*WidgetTree, TEXT("아래"), ECodexUIButtonKind::Neutral);
	BottomButton->OnClicked.AddDynamic(this, &ThisClass::HandleScrollNoticeBottom);
	AddControlHBox(*NoticeHeaderRow, ControlSized(*WidgetTree, TopButton, 44.0f, 30.0f), FMargin(0.0f, 0.0f, FCodexUISpace::S1, 0.0f));
	AddControlHBox(*NoticeHeaderRow, ControlSized(*WidgetTree, BottomButton, 50.0f, 30.0f));
	AddControlVBox(*NoticeColumn, NoticeHeaderRow, FMargin(0.0f, 0.0f, 0.0f, FCodexUISpace::S2));

	NoticeScrollBox = WidgetTree->ConstructWidget<UScrollBox>();
	NoticeScrollBox->SetScrollBarVisibility(ESlateVisibility::Visible);
	AddControlVBox(*NoticeColumn, ControlSized(*WidgetTree, NoticeScrollBox, 344.0f, 126.0f));
	AddControlHBox(*RootRow, NoticeColumn, FMargin(0.0f), 0.9f);

	RefreshSliderLabels();
	RefreshNotices();
	return Super::RebuildWidget();
}

void UCodexUIKitControlsPanelWidget::SeedNoticesIfNeeded()
{
	if (!Notices.IsEmpty())
	{
		return;
	}

	Notices.Add(TEXT("퀘스트 목표가 갱신되었습니다."));
	Notices.Add(TEXT("인벤토리 필터가 전체로 설정되었습니다."));
	Notices.Add(TEXT("슬라이더 값 변경은 외부 설정을 저장하지 않습니다."));
	Notices.Add(TEXT("테스트 알림은 이 패널 안에서만 누적됩니다."));
	Notices.Add(TEXT("스크롤 박스가 충분한 로그를 표시합니다."));
	Notices.Add(TEXT("보상 팝업은 확인 시 닫힙니다."));
	Notices.Add(TEXT("루나의 레벨 표시가 갱신되었습니다."));
	Notices.Add(TEXT("데모 UI가 입력 모드를 UI 우선으로 전환했습니다."));
	NoticeSequence = Notices.Num();
}

void UCodexUIKitControlsPanelWidget::RefreshSliderLabels()
{
	if (BgmValueLabel)
	{
		BgmValueLabel->SetText(FText::FromString(FString::Printf(TEXT("%d"), FMath::RoundToInt(BgmValue * 100.0f))));
	}
	if (SfxValueLabel)
	{
		SfxValueLabel->SetText(FText::FromString(FString::Printf(TEXT("%d"), FMath::RoundToInt(SfxValue * 100.0f))));
	}
	if (ZoomValueLabel)
	{
		ZoomValueLabel->SetText(FText::FromString(FString::Printf(TEXT("%d%%"), 80 + FMath::RoundToInt(ZoomValue * 80.0f))));
	}
}

void UCodexUIKitControlsPanelWidget::RefreshNotices()
{
	if (!NoticeScrollBox)
	{
		return;
	}

	NoticeScrollBox->ClearChildren();
	for (int32 Index = 0; Index < Notices.Num(); ++Index)
	{
		UBorder* Notice = ControlPanel(*WidgetTree, Index % 2 == 0 ? FLinearColor(1.0f, 0.96f, 0.78f, 0.92f) : FCodexUIColor::SurfaceRaised(), FCodexUIRadius::SM);
		Notice->SetPadding(FMargin(FCodexUISpace::S2, FCodexUISpace::S1));
		Notice->SetContent(ControlText(*WidgetTree, FString::Printf(TEXT("%02d  %s"), Index + 1, *Notices[Index]), FCodexUIFontSize::Caption, FCodexUIColor::TextPrimary()));
		NoticeScrollBox->AddChild(Notice);
	}

	if (NoticeHeaderLabel)
	{
		NoticeHeaderLabel->SetText(FText::FromString(FString::Printf(TEXT("알림 로그 %d"), Notices.Num())));
	}
}

void UCodexUIKitControlsPanelWidget::AddNotice(const FString& Notice)
{
	Notices.Add(Notice);
	RefreshNotices();
	if (NoticeScrollBox)
	{
		NoticeScrollBox->ScrollToEnd();
	}
}

void UCodexUIKitControlsPanelWidget::HandleBgmChanged(float NewValue)
{
	BgmValue = NewValue;
	RefreshSliderLabels();
}

void UCodexUIKitControlsPanelWidget::HandleSfxChanged(float NewValue)
{
	SfxValue = NewValue;
	RefreshSliderLabels();
}

void UCodexUIKitControlsPanelWidget::HandleZoomChanged(float NewValue)
{
	ZoomValue = NewValue;
	RefreshSliderLabels();
}

void UCodexUIKitControlsPanelWidget::HandleAddNotice()
{
	++NoticeSequence;
	AddNotice(FString::Printf(TEXT("테스트 알림 %d: 버튼 입력으로 추가되었습니다."), NoticeSequence));
}

void UCodexUIKitControlsPanelWidget::HandleScrollNoticeTop()
{
	if (NoticeScrollBox)
	{
		NoticeScrollBox->ScrollToStart();
	}
}

void UCodexUIKitControlsPanelWidget::HandleScrollNoticeBottom()
{
	if (NoticeScrollBox)
	{
		NoticeScrollBox->ScrollToEnd();
	}
}

void UCodexUIKitControlsPanelWidget::HandleResetControls()
{
	ResetDemoControls();
}
