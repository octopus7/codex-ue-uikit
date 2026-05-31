#include "UI/CodexUIKitQuestBoardWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/ProgressBar.h"
#include "Components/ScrollBox.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "UI/CodexUIStyle.h"

namespace
{
UTextBlock* QuestText(UWidgetTree& Tree, const FString& Text, float Size, const FLinearColor& Color, FName Typeface = NAME_None)
{
	UTextBlock* TextBlock = Tree.ConstructWidget<UTextBlock>();
	TextBlock->SetText(FText::FromString(Text));
	TextBlock->SetFont(FCodexUIStyle::Font(Size, Typeface));
	TextBlock->SetColorAndOpacity(FSlateColor(Color));
	return TextBlock;
}

UBorder* QuestPanel(UWidgetTree& Tree, const FLinearColor& Fill = FCodexUIColor::SurfaceBase(), float Radius = FCodexUIRadius::LG)
{
	UBorder* Border = Tree.ConstructWidget<UBorder>();
	Border->SetPadding(FMargin(FCodexUISpace::S4));
	Border->SetBrush(FCodexUIStyle::RoundedBrush(Fill, Radius, FCodexUIColor::BorderSoft(), 1.0f));
	return Border;
}

UButton* QuestButton(UWidgetTree& Tree, const FString& Label, ECodexUIButtonKind Kind)
{
	UButton* Button = Tree.ConstructWidget<UButton>();
	Button->SetStyle(FCodexUIStyle::ButtonStyle(Kind));
	UTextBlock* LabelText = QuestText(
		Tree,
		Label,
		FCodexUIFontSize::Caption,
		Kind == ECodexUIButtonKind::Neutral ? FCodexUIColor::TextPrimary() : FCodexUIColor::TextInverse(),
		TEXT("Bold"));
	LabelText->SetJustification(ETextJustify::Center);
	Button->SetContent(LabelText);
	return Button;
}

void AddQuestVBox(UVerticalBox& Box, UWidget* Child, const FMargin Padding = FMargin(0.0f), EHorizontalAlignment Align = HAlign_Fill)
{
	UVerticalBoxSlot* Slot = Box.AddChildToVerticalBox(Child);
	Slot->SetPadding(Padding);
	Slot->SetHorizontalAlignment(Align);
}

void AddQuestHBox(UHorizontalBox& Box, UWidget* Child, const FMargin Padding = FMargin(0.0f), float FillWidth = 0.0f)
{
	UHorizontalBoxSlot* Slot = Box.AddChildToHorizontalBox(Child);
	Slot->SetPadding(Padding);
	FSlateChildSize Size(FillWidth > 0.0f ? ESlateSizeRule::Fill : ESlateSizeRule::Automatic);
	Size.Value = FillWidth > 0.0f ? FillWidth : 1.0f;
	Slot->SetSize(Size);
}

USizeBox* QuestSized(UWidgetTree& Tree, UWidget* Content, float Width, float Height)
{
	USizeBox* SizeBox = Tree.ConstructWidget<USizeBox>();
	SizeBox->SetWidthOverride(Width);
	SizeBox->SetHeightOverride(Height);
	SizeBox->AddChild(Content);
	return SizeBox;
}

FCodexQuestDemoData MakeQuest(
	const TCHAR* Title,
	const TCHAR* Summary,
	const TCHAR* Objective,
	int32 Current,
	int32 Required,
	ECodexQuestState State,
	int32 RewardCoins)
{
	FCodexQuestDemoData Quest;
	Quest.Title = Title;
	Quest.Summary = Summary;
	Quest.Objective = Objective;
	Quest.Current = Current;
	Quest.Required = Required;
	Quest.State = State;
	Quest.RewardCoins = RewardCoins;
	return Quest;
}
}

void UCodexUIKitQuestBoardWidget::ResetDemoQuests()
{
	Quests.Reset();
	SelectedQuestIndex = 0;
	SeedQuestsIfNeeded();
	RefreshQuestList();
	RefreshDetail();
}

TSharedRef<SWidget> UCodexUIKitQuestBoardWidget::RebuildWidget()
{
	SeedQuestsIfNeeded();

	UBorder* RootPanel = QuestPanel(*WidgetTree);
	WidgetTree->RootWidget = RootPanel;

	UHorizontalBox* RootRow = WidgetTree->ConstructWidget<UHorizontalBox>();
	RootPanel->SetContent(RootRow);

	UVerticalBox* ListColumn = WidgetTree->ConstructWidget<UVerticalBox>();
	HeaderLabel = QuestText(*WidgetTree, TEXT("퀘스트 보드"), FCodexUIFontSize::H3, FCodexUIColor::TextPrimary(), TEXT("Bold"));
	AddQuestVBox(*ListColumn, HeaderLabel, FMargin(0.0f, 0.0f, 0.0f, FCodexUISpace::S2));

	QuestScrollBox = WidgetTree->ConstructWidget<UScrollBox>();
	QuestScrollBox->SetScrollBarVisibility(ESlateVisibility::Visible);
	AddQuestVBox(*ListColumn, QuestSized(*WidgetTree, QuestScrollBox, 258.0f, 306.0f), FMargin(0.0f, 0.0f, FCodexUISpace::S3, 0.0f));

	UHorizontalBox* NavRow = WidgetTree->ConstructWidget<UHorizontalBox>();
	UButton* PrevButton = QuestButton(*WidgetTree, TEXT("이전"), ECodexUIButtonKind::Neutral);
	PrevButton->OnClicked.AddDynamic(this, &ThisClass::HandlePreviousQuest);
	UButton* NextButton = QuestButton(*WidgetTree, TEXT("다음"), ECodexUIButtonKind::Neutral);
	NextButton->OnClicked.AddDynamic(this, &ThisClass::HandleNextQuest);
	AddQuestHBox(*NavRow, QuestSized(*WidgetTree, PrevButton, 76.0f, 32.0f), FMargin(0.0f, 0.0f, FCodexUISpace::S2, 0.0f));
	AddQuestHBox(*NavRow, QuestSized(*WidgetTree, NextButton, 76.0f, 32.0f));
	AddQuestVBox(*ListColumn, NavRow, FMargin(0.0f, FCodexUISpace::S2, 0.0f, 0.0f));
	AddQuestHBox(*RootRow, ListColumn, FMargin(0.0f, 0.0f, FCodexUISpace::S4, 0.0f));

	DetailBox = WidgetTree->ConstructWidget<UVerticalBox>();
	AddQuestHBox(*RootRow, DetailBox, FMargin(0.0f), 1.0f);

	RefreshQuestList();
	RefreshDetail();
	return Super::RebuildWidget();
}

void UCodexUIKitQuestBoardWidget::SeedQuestsIfNeeded()
{
	if (!Quests.IsEmpty())
	{
		return;
	}

	Quests.Add(MakeQuest(TEXT("구조 복귀 확인"), TEXT("구급 카트 전송 절차를 점검합니다."), TEXT("구급 카트에 탑승"), 0, 1, ECodexQuestState::InProgress, 50));
	Quests.Add(MakeQuest(TEXT("고장 난 스위퍼 수리"), TEXT("작업 도구를 모아 스위퍼를 다시 작동시킵니다."), TEXT("스위퍼 부품 수집"), 1, 3, ECodexQuestState::InProgress, 80));
	Quests.Add(MakeQuest(TEXT("식량 부족"), TEXT("루나에게 임시 식량을 전달합니다."), TEXT("보급 식량 전달"), 1, 1, ECodexQuestState::ReadyToComplete, 35));
	Quests.Add(MakeQuest(TEXT("해안 관측 기록"), TEXT("안개가 걷힌 해안의 기상 정보를 기록합니다."), TEXT("관측 지점 스캔"), 2, 4, ECodexQuestState::InProgress, 70));
	Quests.Add(MakeQuest(TEXT("분실한 지도 조각"), TEXT("찢어진 지도 조각을 찾아 경로를 복원합니다."), TEXT("지도 조각 회수"), 3, 3, ECodexQuestState::Completed, 120));
	Quests.Add(MakeQuest(TEXT("오염 샘플 회수"), TEXT("푸른 결정 주변의 오염 샘플을 안전하게 담습니다."), TEXT("샘플 캡슐 확보"), 0, 2, ECodexQuestState::Failed, 90));
	Quests.Add(MakeQuest(TEXT("임시 캠프 정리"), TEXT("다음 이동을 위해 캠프 주변 물자를 정리합니다."), TEXT("상자 정리"), 4, 6, ECodexQuestState::InProgress, 45));
	Quests.Add(MakeQuest(TEXT("라디오 주파수 맞추기"), TEXT("끊어진 구조 신호를 다시 잡습니다."), TEXT("주파수 동기화"), 0, 1, ECodexQuestState::InProgress, 65));
}

void UCodexUIKitQuestBoardWidget::RefreshQuestList()
{
	if (!QuestScrollBox || !HeaderLabel)
	{
		return;
	}

	QuestScrollBox->ClearChildren();

	int32 InProgressCount = 0;
	int32 ReadyCount = 0;
	int32 CompletedCount = 0;
	int32 FailedCount = 0;
	for (const FCodexQuestDemoData& Quest : Quests)
	{
		InProgressCount += Quest.State == ECodexQuestState::InProgress ? 1 : 0;
		ReadyCount += Quest.State == ECodexQuestState::ReadyToComplete ? 1 : 0;
		CompletedCount += Quest.State == ECodexQuestState::Completed ? 1 : 0;
		FailedCount += Quest.State == ECodexQuestState::Failed ? 1 : 0;
	}

	HeaderLabel->SetText(FText::FromString(FString::Printf(
		TEXT("퀘스트 보드  진행 %d / 완료 가능 %d / 완료 %d / 실패 %d"),
		InProgressCount,
		ReadyCount,
		CompletedCount,
		FailedCount)));

	for (int32 Index = 0; Index < Quests.Num(); ++Index)
	{
		const FCodexQuestDemoData& Quest = Quests[Index];
		const bool bSelected = Index == SelectedQuestIndex;

		UBorder* Row = QuestPanel(
			*WidgetTree,
			bSelected ? FLinearColor(0.76f, 0.90f, 0.94f, 1.0f) : FCodexUIColor::SurfaceRaised(),
			FCodexUIRadius::MD);
		Row->SetPadding(FMargin(FCodexUISpace::S3, FCodexUISpace::S2));

		UVerticalBox* RowStack = WidgetTree->ConstructWidget<UVerticalBox>();
		UHorizontalBox* TitleRow = WidgetTree->ConstructWidget<UHorizontalBox>();
		AddQuestHBox(*TitleRow, QuestText(*WidgetTree, Quest.Title, FCodexUIFontSize::Body, FCodexUIColor::TextPrimary(), TEXT("Bold")), FMargin(0.0f), 1.0f);
		AddQuestHBox(*TitleRow, QuestText(*WidgetTree, StateText(Quest.State).ToString(), FCodexUIFontSize::Badge, StateColor(Quest.State), TEXT("Bold")));
		AddQuestVBox(*RowStack, TitleRow);
		AddQuestVBox(*RowStack, QuestText(*WidgetTree, Quest.Objective, FCodexUIFontSize::Caption, FCodexUIColor::TextSecondary()), FMargin(0.0f, 4.0f, 0.0f, 0.0f));
		AddQuestVBox(*RowStack, QuestText(*WidgetTree, FString::Printf(TEXT("%d / %d"), Quest.Current, Quest.Required), FCodexUIFontSize::Caption, FCodexUIColor::TextMuted()), FMargin(0.0f, 4.0f, 0.0f, 0.0f), HAlign_Right);
		Row->SetContent(RowStack);
		QuestScrollBox->AddChild(Row);
	}
}

void UCodexUIKitQuestBoardWidget::RefreshDetail()
{
	if (!DetailBox || !Quests.IsValidIndex(SelectedQuestIndex))
	{
		return;
	}

	DetailBox->ClearChildren();
	const FCodexQuestDemoData& Quest = Quests[SelectedQuestIndex];

	UHorizontalBox* HeaderRow = WidgetTree->ConstructWidget<UHorizontalBox>();
	AddQuestHBox(*HeaderRow, QuestText(*WidgetTree, Quest.Title, FCodexUIFontSize::H2, FCodexUIColor::TextPrimary(), TEXT("Bold")), FMargin(0.0f), 1.0f);
	AddQuestHBox(*HeaderRow, QuestText(*WidgetTree, StateText(Quest.State).ToString(), FCodexUIFontSize::Badge, StateColor(Quest.State), TEXT("Bold")));
	AddQuestVBox(*DetailBox, HeaderRow, FMargin(0.0f, 0.0f, 0.0f, FCodexUISpace::S4));

	UTextBlock* Summary = QuestText(*WidgetTree, Quest.Summary, FCodexUIFontSize::Body, FCodexUIColor::TextSecondary());
	Summary->SetWrapTextAt(250.0f);
	AddQuestVBox(*DetailBox, Summary, FMargin(0.0f, 0.0f, 0.0f, FCodexUISpace::S5));

	AddQuestVBox(*DetailBox, QuestText(*WidgetTree, TEXT("목표"), FCodexUIFontSize::Body, FCodexUIColor::TextPrimary(), TEXT("Bold")));
	AddQuestVBox(*DetailBox, QuestText(*WidgetTree, FString::Printf(TEXT("%s (%d/%d)"), *Quest.Objective, Quest.Current, Quest.Required), FCodexUIFontSize::Body, FCodexUIColor::TextPrimary()), FMargin(0.0f, FCodexUISpace::S2, 0.0f, FCodexUISpace::S2));

	UProgressBar* Progress = WidgetTree->ConstructWidget<UProgressBar>();
	Progress->SetWidgetStyle(FCodexUIStyle::ProgressBarStyle(StateColor(Quest.State)));
	Progress->SetPercent(Quest.Required > 0 ? static_cast<float>(Quest.Current) / static_cast<float>(Quest.Required) : 0.0f);
	AddQuestVBox(*DetailBox, QuestSized(*WidgetTree, Progress, 250.0f, 12.0f), FMargin(0.0f, 0.0f, 0.0f, FCodexUISpace::S4));

	AddQuestVBox(*DetailBox, QuestText(*WidgetTree, FString::Printf(TEXT("보상: 코인 %d"), Quest.RewardCoins), FCodexUIFontSize::Body, FCodexUIColor::TextPrimary(), TEXT("Bold")), FMargin(0.0f, 0.0f, 0.0f, FCodexUISpace::S5));

	UHorizontalBox* ButtonRow = WidgetTree->ConstructWidget<UHorizontalBox>();
	UButton* AdvanceButton = QuestButton(*WidgetTree, TEXT("목표 진행"), ECodexUIButtonKind::Primary);
	AdvanceButton->OnClicked.AddDynamic(this, &ThisClass::HandleAdvanceObjective);
	UButton* StateButton = QuestButton(*WidgetTree, TEXT("상태 순환"), ECodexUIButtonKind::Info);
	StateButton->OnClicked.AddDynamic(this, &ThisClass::HandleCycleState);
	AddQuestHBox(*ButtonRow, QuestSized(*WidgetTree, AdvanceButton, 112.0f, 36.0f), FMargin(0.0f, 0.0f, FCodexUISpace::S2, 0.0f));
	AddQuestHBox(*ButtonRow, QuestSized(*WidgetTree, StateButton, 112.0f, 36.0f));
	AddQuestVBox(*DetailBox, ButtonRow);
}

void UCodexUIKitQuestBoardWidget::SelectNextQuest(int32 Direction)
{
	if (Quests.IsEmpty())
	{
		return;
	}

	SelectedQuestIndex = (SelectedQuestIndex + Direction + Quests.Num()) % Quests.Num();
	RefreshQuestList();
	RefreshDetail();
}

FText UCodexUIKitQuestBoardWidget::StateText(ECodexQuestState State)
{
	switch (State)
	{
	case ECodexQuestState::ReadyToComplete:
		return NSLOCTEXT("CodexUIKit", "QuestStateReady", "완료 가능");
	case ECodexQuestState::Completed:
		return NSLOCTEXT("CodexUIKit", "QuestStateCompleted", "완료");
	case ECodexQuestState::Failed:
		return NSLOCTEXT("CodexUIKit", "QuestStateFailed", "실패");
	case ECodexQuestState::InProgress:
	default:
		return NSLOCTEXT("CodexUIKit", "QuestStateInProgress", "진행 중");
	}
}

FLinearColor UCodexUIKitQuestBoardWidget::StateColor(ECodexQuestState State)
{
	switch (State)
	{
	case ECodexQuestState::ReadyToComplete:
		return FCodexUIColor::Accent();
	case ECodexQuestState::Completed:
		return FCodexUIColor::Primary();
	case ECodexQuestState::Failed:
		return FCodexUIColor::Danger();
	case ECodexQuestState::InProgress:
	default:
		return FCodexUIColor::InfoDark();
	}
}

void UCodexUIKitQuestBoardWidget::HandlePreviousQuest()
{
	SelectNextQuest(-1);
}

void UCodexUIKitQuestBoardWidget::HandleNextQuest()
{
	SelectNextQuest(1);
}

void UCodexUIKitQuestBoardWidget::HandleAdvanceObjective()
{
	if (!Quests.IsValidIndex(SelectedQuestIndex))
	{
		return;
	}

	FCodexQuestDemoData& Quest = Quests[SelectedQuestIndex];
	if (Quest.State == ECodexQuestState::Completed || Quest.State == ECodexQuestState::Failed)
	{
		Quest.Current = 0;
		Quest.State = ECodexQuestState::InProgress;
	}
	else
	{
		Quest.Current = FMath::Min(Quest.Current + 1, Quest.Required);
		if (Quest.Current >= Quest.Required)
		{
			Quest.State = ECodexQuestState::ReadyToComplete;
		}
	}

	RefreshQuestList();
	RefreshDetail();
}

void UCodexUIKitQuestBoardWidget::HandleCycleState()
{
	if (!Quests.IsValidIndex(SelectedQuestIndex))
	{
		return;
	}

	FCodexQuestDemoData& Quest = Quests[SelectedQuestIndex];
	switch (Quest.State)
	{
	case ECodexQuestState::InProgress:
		Quest.State = ECodexQuestState::ReadyToComplete;
		Quest.Current = Quest.Required;
		break;
	case ECodexQuestState::ReadyToComplete:
		Quest.State = ECodexQuestState::Completed;
		break;
	case ECodexQuestState::Completed:
		Quest.State = ECodexQuestState::Failed;
		break;
	case ECodexQuestState::Failed:
	default:
		Quest.State = ECodexQuestState::InProgress;
		Quest.Current = 0;
		break;
	}

	RefreshQuestList();
	RefreshDetail();
}
