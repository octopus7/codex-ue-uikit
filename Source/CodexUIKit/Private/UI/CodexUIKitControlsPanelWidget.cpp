#include "UI/CodexUIKitControlsPanelWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/ScrollBox.h"
#include "Components/Slider.h"
#include "Components/TextBlock.h"
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

}

void UCodexUIKitControlsPanelWidget::ResetDemoControls()
{
	BgmValue = 0.68f;
	SfxValue = 0.74f;
	ZoomValue = 0.38f;
	Notices.Reset();
	NoticeSequence = 0;
	SeedNoticesIfNeeded();
	if (BgmSlider)
	{
		BgmSlider->SetValue(BgmValue);
	}
	if (SfxSlider)
	{
		SfxSlider->SetValue(SfxValue);
	}
	if (ZoomSlider)
	{
		ZoomSlider->SetValue(ZoomValue);
	}
	RefreshSliderLabels();
	RefreshNotices();
}

void UCodexUIKitControlsPanelWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (BgmSlider)
	{
		BgmSlider->OnValueChanged.AddDynamic(this, &ThisClass::HandleBgmChanged);
	}
	if (SfxSlider)
	{
		SfxSlider->OnValueChanged.AddDynamic(this, &ThisClass::HandleSfxChanged);
	}
	if (ZoomSlider)
	{
		ZoomSlider->OnValueChanged.AddDynamic(this, &ThisClass::HandleZoomChanged);
	}
	if (ResetButton)
	{
		ResetButton->OnClicked.AddDynamic(this, &ThisClass::HandleResetControls);
	}
	if (AddNoticeButton)
	{
		AddNoticeButton->OnClicked.AddDynamic(this, &ThisClass::HandleAddNotice);
	}
	if (ScrollTopButton)
	{
		ScrollTopButton->OnClicked.AddDynamic(this, &ThisClass::HandleScrollNoticeTop);
	}
	if (ScrollBottomButton)
	{
		ScrollBottomButton->OnClicked.AddDynamic(this, &ThisClass::HandleScrollNoticeBottom);
	}
}

void UCodexUIKitControlsPanelWidget::NativeConstruct()
{
	Super::NativeConstruct();
	FCodexUIStyle::ApplyFontToTextBlocks(GetRootWidget());

	SeedNoticesIfNeeded();
	if (BgmSlider)
	{
		BgmSlider->SetValue(BgmValue);
	}
	if (SfxSlider)
	{
		SfxSlider->SetValue(SfxValue);
	}
	if (ZoomSlider)
	{
		ZoomSlider->SetValue(ZoomValue);
	}

	RefreshSliderLabels();
	RefreshNotices();
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
