#include "UI/CodexUIKitDemoWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/GridPanel.h"
#include "Components/GridSlot.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Image.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/ProgressBar.h"
#include "Components/ScaleBox.h"
#include "Components/SizeBox.h"
#include "Components/Spacer.h"
#include "Components/TextBlock.h"
#include "Components/UniformGridPanel.h"
#include "Components/UniformGridSlot.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Engine/Texture2D.h"
#include "HAL/PlatformFileManager.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Modules/ModuleManager.h"
#include "UI/CodexUIKitControlsPanelWidget.h"
#include "UI/CodexUIStyle.h"
#include "UI/CodexUIKitInventoryGridWidget.h"
#include "UI/CodexUIKitPopupWidget.h"
#include "UI/CodexUIKitQuestBoardWidget.h"

namespace
{
constexpr float StageWidth = 1536.0f;
constexpr float StageHeight = 1024.0f;

UCanvasPanelSlot* AddCanvasChild(UCanvasPanel& Canvas, UWidget* Widget, const FVector2D Position, const FVector2D Size, int32 ZOrder = 0)
{
	UCanvasPanelSlot* Slot = Canvas.AddChildToCanvas(Widget);
	Slot->SetPosition(Position);
	Slot->SetSize(Size);
	Slot->SetZOrder(ZOrder);
	return Slot;
}

UTextBlock* MakeText(UWidgetTree& Tree, const FString& Text, float Size, const FLinearColor& Color, FName Typeface = NAME_None)
{
	UTextBlock* TextBlock = Tree.ConstructWidget<UTextBlock>();
	TextBlock->SetText(FText::FromString(Text));
	TextBlock->SetColorAndOpacity(FSlateColor(Color));
	TextBlock->SetFont(FCodexUIStyle::Font(Size, Typeface));
	return TextBlock;
}

UBorder* MakePanel(UWidgetTree& Tree, const FLinearColor& Fill = FCodexUIColor::SurfaceBase(), float Radius = FCodexUIRadius::LG)
{
	UBorder* Border = Tree.ConstructWidget<UBorder>();
	Border->SetBrush(FCodexUIStyle::RoundedBrush(Fill, Radius, FCodexUIColor::BorderSoft(), 1.0f));
	Border->SetPadding(FMargin(FCodexUISpace::S4));
	return Border;
}

USizeBox* Sized(UWidgetTree& Tree, UWidget* Content, float Width, float Height)
{
	USizeBox* SizeBox = Tree.ConstructWidget<USizeBox>();
	SizeBox->SetWidthOverride(Width);
	SizeBox->SetHeightOverride(Height);
	SizeBox->AddChild(Content);
	return SizeBox;
}

UButton* MakeButton(UWidgetTree& Tree, const FString& Label, ECodexUIButtonKind Kind, bool bEnabled = true)
{
	UButton* Button = Tree.ConstructWidget<UButton>();
	Button->SetStyle(FCodexUIStyle::ButtonStyle(bEnabled ? Kind : ECodexUIButtonKind::Disabled));
	Button->SetIsEnabled(bEnabled);

	const bool bInverse = Kind != ECodexUIButtonKind::Neutral && bEnabled;
	UTextBlock* ButtonText = MakeText(
		Tree,
		Label,
		FCodexUIFontSize::Body,
		bInverse ? FCodexUIColor::TextInverse() : FCodexUIColor::TextPrimary(),
		TEXT("Bold"));
	ButtonText->SetJustification(ETextJustify::Center);
	Button->SetContent(ButtonText);
	return Button;
}

UBorder* MakeTag(UWidgetTree& Tree, const FString& Label, const FLinearColor& Color)
{
	UBorder* Tag = Tree.ConstructWidget<UBorder>();
	Tag->SetPadding(FMargin(14.0f, 7.0f));
	Tag->SetBrush(FCodexUIStyle::RoundedBrush(Color, FCodexUIRadius::SM));
	UTextBlock* Text = MakeText(Tree, Label, FCodexUIFontSize::Badge, FCodexUIColor::TextInverse(), TEXT("Bold"));
	Text->SetJustification(ETextJustify::Center);
	Tag->SetContent(Text);
	return Tag;
}

UProgressBar* MakeProgress(UWidgetTree& Tree, float Percent, const FLinearColor& Fill)
{
	UProgressBar* Bar = Tree.ConstructWidget<UProgressBar>();
	Bar->SetPercent(Percent);
	Bar->SetWidgetStyle(FCodexUIStyle::ProgressBarStyle(Fill));
	return Bar;
}

void AddVBoxChild(UVerticalBox& Box, UWidget* Child, const FMargin Padding = FMargin(0.0f), EHorizontalAlignment HAlign = HAlign_Fill)
{
	UVerticalBoxSlot* Slot = Box.AddChildToVerticalBox(Child);
	Slot->SetPadding(Padding);
	Slot->SetHorizontalAlignment(HAlign);
}

void AddHBoxChild(UHorizontalBox& Box, UWidget* Child, const FMargin Padding = FMargin(0.0f), float FillWidth = 0.0f)
{
	UHorizontalBoxSlot* BoxSlot = Box.AddChildToHorizontalBox(Child);
	BoxSlot->SetPadding(Padding);
	FSlateChildSize ChildSize(FillWidth > 0.0f ? ESlateSizeRule::Fill : ESlateSizeRule::Automatic);
	if (FillWidth > 0.0f)
	{
		ChildSize.Value = FillWidth;
	}
	BoxSlot->SetSize(ChildSize);
}
}

TSharedRef<SWidget> UCodexUIKitDemoWidget::RebuildWidget()
{
	LoadedSourceTextures.Reset();

	UScaleBox* RootScale = WidgetTree->ConstructWidget<UScaleBox>();
	RootScale->SetStretch(EStretch::ScaleToFit);
	RootScale->SetStretchDirection(EStretchDirection::Both);
	WidgetTree->RootWidget = RootScale;

	USizeBox* StageSize = WidgetTree->ConstructWidget<USizeBox>();
	StageSize->SetWidthOverride(StageWidth);
	StageSize->SetHeightOverride(StageHeight);
	RootScale->AddChild(StageSize);

	UOverlay* Stage = WidgetTree->ConstructWidget<UOverlay>();
	StageSize->AddChild(Stage);

	UBorder* Background = WidgetTree->ConstructWidget<UBorder>();
	Background->SetBrush(FCodexUIStyle::RoundedBrush(FCodexUIColor::BgMist(), 0.0f));
	UOverlaySlot* BackgroundSlot = Stage->AddChildToOverlay(Background);
	BackgroundSlot->SetHorizontalAlignment(HAlign_Fill);
	BackgroundSlot->SetVerticalAlignment(VAlign_Fill);

	UCanvasPanel* Canvas = WidgetTree->ConstructWidget<UCanvasPanel>();
	Stage->AddChildToOverlay(Canvas);

	UBorder* TopWash = WidgetTree->ConstructWidget<UBorder>();
	TopWash->SetBrush(FCodexUIStyle::RoundedBrush(FLinearColor(0.10f, 0.42f, 0.39f, 0.92f), 0.0f));
	AddCanvasChild(*Canvas, TopWash, FVector2D(0.0f, 0.0f), FVector2D(StageWidth, 285.0f), -2);

	UBorder* MistWash = WidgetTree->ConstructWidget<UBorder>();
	MistWash->SetBrush(FCodexUIStyle::RoundedBrush(FLinearColor(0.92f, 0.94f, 0.78f, 0.72f), 0.0f));
	AddCanvasChild(*Canvas, MistWash, FVector2D(0.0f, 235.0f), FVector2D(StageWidth, StageHeight - 235.0f), -1);

	BuildHeader(*Canvas);
	BuildButtons(*Canvas);
	BuildIconAndStatus(*Canvas);
	BuildSideMenu(*Canvas);
	BuildMainPanels(*Canvas);
	BuildBottomExamples(*Canvas);

	return Super::RebuildWidget();
}

void UCodexUIKitDemoWidget::BuildHeader(UCanvasPanel& Canvas)
{
	UVerticalBox* TitleStack = WidgetTree->ConstructWidget<UVerticalBox>();
	AddVBoxChild(*TitleStack, MakeText(*WidgetTree, TEXT("TUNA SWEEPER"), FCodexUIFontSize::H3, FCodexUIColor::TextInverse(), TEXT("Bold")));
	AddVBoxChild(*TitleStack, MakeText(*WidgetTree, TEXT("UI KIT DEMO"), FCodexUIFontSize::Display, FCodexUIColor::TextInverse(), TEXT("Bold")), FMargin(0.0f, 6.0f, 0.0f, 8.0f));
	AddVBoxChild(*TitleStack, MakeTag(*WidgetTree, TEXT("# Piece Theme"), FCodexUIColor::InfoDark()), FMargin(0.0f, 0.0f, 0.0f, 12.0f), HAlign_Left);
	UTextBlock* Description = MakeText(*WidgetTree, TEXT("자연의 생명력과 따뜻한 햇살을 담은 피스 테마의 UI 키트 데모입니다."), FCodexUIFontSize::Body, FCodexUIColor::TextInverse());
	Description->SetWrapTextAt(360.0f);
	AddVBoxChild(*TitleStack, Description);
	AddCanvasChild(Canvas, TitleStack, FVector2D(32.0f, 28.0f), FVector2D(380.0f, 170.0f));

	UBorder* NavPanel = WidgetTree->ConstructWidget<UBorder>();
	NavPanel->SetPadding(FMargin(0.0f));
	NavPanel->SetBrush(FCodexUIStyle::RoundedBrush(FLinearColor(0.22f, 0.68f, 0.63f, 0.58f), FCodexUIRadius::MD));
	UHorizontalBox* Nav = WidgetTree->ConstructWidget<UHorizontalBox>();
	NavPanel->SetContent(Nav);
	const FString Tabs[] = { TEXT("메뉴"), TEXT("인벤토리"), TEXT("퀘스트"), TEXT("지도"), TEXT("도감"), TEXT("설정") };
	for (int32 Index = 0; Index < UE_ARRAY_COUNT(Tabs); ++Index)
	{
		UButton* Tab = MakeButton(*WidgetTree, Tabs[Index], Index == 0 ? ECodexUIButtonKind::Accent : ECodexUIButtonKind::Neutral);
		Tab->SetStyle(FCodexUIStyle::ButtonStyle(Index == 0 ? ECodexUIButtonKind::Accent : ECodexUIButtonKind::Neutral));
		AddHBoxChild(*Nav, Sized(*WidgetTree, Tab, Index == 0 ? 106.0f : 92.0f, FCodexUISize::ButtonLG), FMargin(0.0f), 0.0f);
	}
	AddCanvasChild(Canvas, NavPanel, FVector2D(454.0f, 26.0f), FVector2D(560.0f, 54.0f));

	UBorder* QKey = MakeTag(*WidgetTree, TEXT("Q"), FLinearColor(0.68f, 0.82f, 0.65f, 0.92f));
	AddCanvasChild(Canvas, QKey, FVector2D(418.0f, 40.0f), FVector2D(28.0f, 28.0f));
	UBorder* EKey = MakeTag(*WidgetTree, TEXT("E"), FLinearColor(0.68f, 0.82f, 0.65f, 0.92f));
	AddCanvasChild(Canvas, EKey, FVector2D(1023.0f, 40.0f), FVector2D(28.0f, 28.0f));

	UBorder* Profile = MakePanel(*WidgetTree, FCodexUIColor::SurfaceRaised(), FCodexUIRadius::XL);
	UHorizontalBox* ProfileRow = WidgetTree->ConstructWidget<UHorizontalBox>();
	Profile->SetContent(ProfileRow);

	if (UTexture2D* AvatarTexture = LoadSourceTexture(TEXT("avatar-luna.png")))
	{
		UImage* Avatar = WidgetTree->ConstructWidget<UImage>();
		Avatar->SetBrushFromTexture(AvatarTexture, true);
		AddHBoxChild(*ProfileRow, Sized(*WidgetTree, Avatar, 64.0f, 64.0f), FMargin(0.0f, 0.0f, 16.0f, 0.0f));
	}

	UVerticalBox* ProfileInfo = WidgetTree->ConstructWidget<UVerticalBox>();
	AddVBoxChild(*ProfileInfo, MakeText(*WidgetTree, TEXT("루나"), FCodexUIFontSize::H3, FCodexUIColor::TextPrimary(), TEXT("Bold")));
	AddVBoxChild(*ProfileInfo, MakeText(*WidgetTree, TEXT("Lv. 18"), FCodexUIFontSize::Caption, FCodexUIColor::TextSecondary()), FMargin(0.0f, 8.0f, 0.0f, 4.0f));
	AddVBoxChild(*ProfileInfo, Sized(*WidgetTree, MakeProgress(*WidgetTree, 0.52f, FCodexUIColor::Primary()), 190.0f, 10.0f));
	AddHBoxChild(*ProfileRow, ProfileInfo, FMargin(0.0f), 1.0f);

	UVerticalBox* Currency = WidgetTree->ConstructWidget<UVerticalBox>();
	AddVBoxChild(*Currency, MakeText(*WidgetTree, TEXT("CO  12,850"), FCodexUIFontSize::Body, FCodexUIColor::TextPrimary(), TEXT("Bold")));
	AddVBoxChild(*Currency, MakeText(*WidgetTree, TEXT("GE  230     +"), FCodexUIFontSize::Body, FCodexUIColor::TextPrimary(), TEXT("Bold")), FMargin(0.0f, 16.0f, 0.0f, 0.0f));
	AddHBoxChild(*ProfileRow, Currency, FMargin(24.0f, 0.0f, 0.0f, 0.0f));
	AddCanvasChild(Canvas, Profile, FVector2D(1066.0f, 26.0f), FVector2D(448.0f, 102.0f));
}

void UCodexUIKitDemoWidget::BuildButtons(UCanvasPanel& Canvas)
{
	AddCanvasChild(Canvas, MakeText(*WidgetTree, TEXT("BUTTONS"), FCodexUIFontSize::H3, FCodexUIColor::TextInverse(), TEXT("Bold")), FVector2D(450.0f, 112.0f), FVector2D(160.0f, 24.0f));

	UBorder* Panel = MakePanel(*WidgetTree);
	Panel->SetPadding(FMargin(8.0f));
	UGridPanel* Grid = WidgetTree->ConstructWidget<UGridPanel>();
	Panel->SetContent(Grid);

	struct FButtonSpec { const TCHAR* Label; ECodexUIButtonKind Kind; bool bEnabled; };
	const FButtonSpec Specs[] =
	{
		{ TEXT("기본 버튼"), ECodexUIButtonKind::Primary, true },
		{ TEXT("강조 버튼"), ECodexUIButtonKind::Accent, true },
		{ TEXT("보조 버튼"), ECodexUIButtonKind::Info, true },
		{ TEXT("비활성 버튼"), ECodexUIButtonKind::Neutral, false },
		{ TEXT("기본 버튼 Hover"), ECodexUIButtonKind::Primary, true },
		{ TEXT("강조 버튼 Hover"), ECodexUIButtonKind::Accent, true },
		{ TEXT("보조 버튼 Hover"), ECodexUIButtonKind::Info, true },
		{ TEXT("비활성 버튼"), ECodexUIButtonKind::Neutral, false },
		{ TEXT("확인"), ECodexUIButtonKind::Primary, true },
		{ TEXT("취소"), ECodexUIButtonKind::Neutral, true },
		{ TEXT("삭제"), ECodexUIButtonKind::Danger, true },
		{ TEXT("받기"), ECodexUIButtonKind::Info, true }
	};

	for (int32 Index = 0; Index < UE_ARRAY_COUNT(Specs); ++Index)
	{
		UButton* Button = MakeButton(*WidgetTree, Specs[Index].Label, Specs[Index].Kind, Specs[Index].bEnabled);
		if (Specs[Index].bEnabled)
		{
			Button->OnClicked.AddDynamic(this, &ThisClass::HandleOpenActionPopup);
		}
		UGridSlot* GridCellSlot = Grid->AddChildToGrid(Sized(*WidgetTree, Button, 116.0f, 40.0f), Index / 4, Index % 4);
		GridCellSlot->SetPadding(FMargin(5.0f));
	}

	AddCanvasChild(Canvas, Panel, FVector2D(440.0f, 136.0f), FVector2D(505.0f, 166.0f));
}

void UCodexUIKitDemoWidget::BuildIconAndStatus(UCanvasPanel& Canvas)
{
	AddCanvasChild(Canvas, MakeText(*WidgetTree, TEXT("ICON SET"), FCodexUIFontSize::H3, FCodexUIColor::TextInverse(), TEXT("Bold")), FVector2D(1054.0f, 142.0f), FVector2D(160.0f, 24.0f));

	UBorder* IconPanel = MakePanel(*WidgetTree);
	IconPanel->SetPadding(FMargin(8.0f));
	UHorizontalBox* Icons = WidgetTree->ConstructWidget<UHorizontalBox>();
	IconPanel->SetContent(Icons);
	const FString IconLabels[] = { TEXT("가"), TEXT("검"), TEXT("방"), TEXT("준"), TEXT("약"), TEXT("상"), TEXT("컵"), TEXT("편"), TEXT("설") };
	for (const FString& IconLabel : IconLabels)
	{
		UBorder* Cell = MakePanel(*WidgetTree, FCodexUIColor::SurfaceRaised(), FCodexUIRadius::SM);
		Cell->SetPadding(FMargin(0.0f));
		UTextBlock* Icon = MakeText(*WidgetTree, IconLabel, 22.0f, FCodexUIColor::TextSecondary(), TEXT("Bold"));
		Icon->SetJustification(ETextJustify::Center);
		Cell->SetContent(Icon);
		AddHBoxChild(*Icons, Sized(*WidgetTree, Cell, 48.0f, 48.0f), FMargin(4.0f));
	}
	AddCanvasChild(Canvas, IconPanel, FVector2D(1044.0f, 162.0f), FVector2D(472.0f, 58.0f));

	AddCanvasChild(Canvas, MakeText(*WidgetTree, TEXT("STATUS / TAG"), FCodexUIFontSize::H3, FCodexUIColor::BgTealDeep(), TEXT("Bold")), FVector2D(1054.0f, 232.0f), FVector2D(180.0f, 24.0f));

	UBorder* StatusPanel = MakePanel(*WidgetTree);
	StatusPanel->SetPadding(FMargin(8.0f));
	UHorizontalBox* Tags = WidgetTree->ConstructWidget<UHorizontalBox>();
	StatusPanel->SetContent(Tags);
	AddHBoxChild(*Tags, Sized(*WidgetTree, MakeTag(*WidgetTree, TEXT("진행 중"), FCodexUIColor::InfoDark()), 84.0f, 36.0f), FMargin(4.0f));
	AddHBoxChild(*Tags, Sized(*WidgetTree, MakeTag(*WidgetTree, TEXT("완료 가능"), FCodexUIColor::Accent()), 94.0f, 36.0f), FMargin(4.0f));
	AddHBoxChild(*Tags, Sized(*WidgetTree, MakeTag(*WidgetTree, TEXT("완료"), FCodexUIColor::Primary()), 84.0f, 36.0f), FMargin(4.0f));
	AddHBoxChild(*Tags, Sized(*WidgetTree, MakeTag(*WidgetTree, TEXT("실패"), FCodexUIColor::Danger()), 84.0f, 36.0f), FMargin(4.0f));
	AddHBoxChild(*Tags, Sized(*WidgetTree, MakeTag(*WidgetTree, TEXT("NEW"), FCodexUIColor::NewTag()), 74.0f, 36.0f), FMargin(4.0f));
	AddCanvasChild(Canvas, StatusPanel, FVector2D(1044.0f, 252.0f), FVector2D(472.0f, 50.0f));
}

void UCodexUIKitDemoWidget::BuildSideMenu(UCanvasPanel& Canvas)
{
	AddCanvasChild(Canvas, MakeText(*WidgetTree, TEXT("COLOR PALETTE"), FCodexUIFontSize::H3, FCodexUIColor::TextInverse(), TEXT("Bold")), FVector2D(30.0f, 206.0f), FVector2D(180.0f, 24.0f));
	UHorizontalBox* Palette = WidgetTree->ConstructWidget<UHorizontalBox>();
	const FLinearColor Colors[] =
	{
		FCodexUIColor::Primary(), FCodexUIColor::BgTeal(), FCodexUIColor::Info(), FCodexUIColor::SurfaceBase(),
		FCodexUIColor::SurfaceRaised(), FCodexUIColor::Accent(), FCodexUIColor::TextMuted()
	};
	for (const FLinearColor& Color : Colors)
	{
		UBorder* Swatch = WidgetTree->ConstructWidget<UBorder>();
		Swatch->SetBrush(FCodexUIStyle::RoundedBrush(Color, FCodexUIRadius::SM));
		AddHBoxChild(*Palette, Sized(*WidgetTree, Swatch, 44.0f, 44.0f), FMargin(0.0f, 0.0f, 10.0f, 0.0f));
	}
	AddCanvasChild(Canvas, Palette, FVector2D(30.0f, 240.0f), FVector2D(390.0f, 48.0f));

	AddCanvasChild(Canvas, MakeText(*WidgetTree, TEXT("SIDE MENU"), FCodexUIFontSize::H3, FCodexUIColor::BgTealDeep(), TEXT("Bold")), FVector2D(30.0f, 318.0f), FVector2D(140.0f, 24.0f));
	UBorder* MenuPanel = MakePanel(*WidgetTree);
	UVerticalBox* Menu = WidgetTree->ConstructWidget<UVerticalBox>();
	MenuPanel->SetContent(Menu);
	const FString MenuItems[] = { TEXT("퀘스트        !"), TEXT("인벤토리"), TEXT("지도"), TEXT("업적"), TEXT("도감"), TEXT("설정") };
	for (int32 Index = 0; Index < UE_ARRAY_COUNT(MenuItems); ++Index)
	{
		UBorder* Row = WidgetTree->ConstructWidget<UBorder>();
		Row->SetPadding(FMargin(12.0f, 9.0f));
		Row->SetBrush(FCodexUIStyle::RoundedBrush(Index == 0 ? FCodexUIColor::SurfaceRaised() : FLinearColor::Transparent, FCodexUIRadius::SM));
		Row->SetContent(MakeText(*WidgetTree, MenuItems[Index], FCodexUIFontSize::Body, FCodexUIColor::TextPrimary(), Index == 0 ? FName(TEXT("Bold")) : NAME_None));
		AddVBoxChild(*Menu, Row, FMargin(0.0f, 0.0f, 0.0f, 5.0f));
	}
	AddCanvasChild(Canvas, MenuPanel, FVector2D(18.0f, 336.0f), FVector2D(198.0f, 280.0f));

	AddCanvasChild(Canvas, MakeText(*WidgetTree, TEXT("DIALOG / POPUP"), FCodexUIFontSize::H3, FCodexUIColor::BgTealDeep(), TEXT("Bold")), FVector2D(30.0f, 630.0f), FVector2D(180.0f, 24.0f));
	UBorder* Dialog = MakePanel(*WidgetTree);
	UVerticalBox* DialogStack = WidgetTree->ConstructWidget<UVerticalBox>();
	Dialog->SetContent(DialogStack);
	UTextBlock* Warn = MakeText(*WidgetTree, TEXT("!"), 24.0f, FCodexUIColor::Warning(), TEXT("Bold"));
	Warn->SetJustification(ETextJustify::Center);
	AddVBoxChild(*DialogStack, Warn, FMargin(0.0f, 0.0f, 0.0f, 8.0f), HAlign_Center);
	UTextBlock* DialogText = MakeText(*WidgetTree, TEXT("정말로 이동하시겠습니까?"), FCodexUIFontSize::Caption, FCodexUIColor::TextSecondary());
	DialogText->SetJustification(ETextJustify::Center);
	AddVBoxChild(*DialogStack, DialogText, FMargin(0.0f, 0.0f, 0.0f, 12.0f));
	UHorizontalBox* DialogButtons = WidgetTree->ConstructWidget<UHorizontalBox>();
	UButton* Confirm = MakeButton(*WidgetTree, TEXT("확인"), ECodexUIButtonKind::Primary);
	Confirm->OnClicked.AddDynamic(this, &ThisClass::HandleOpenConfirmPopup);
	UButton* Cancel = MakeButton(*WidgetTree, TEXT("취소"), ECodexUIButtonKind::Neutral);
	Cancel->OnClicked.AddDynamic(this, &ThisClass::HandleOpenConfirmPopup);
	AddHBoxChild(*DialogButtons, Sized(*WidgetTree, Confirm, 68.0f, 34.0f), FMargin(0.0f, 0.0f, 14.0f, 0.0f));
	AddHBoxChild(*DialogButtons, Sized(*WidgetTree, Cancel, 68.0f, 34.0f));
	AddVBoxChild(*DialogStack, DialogButtons, FMargin(0.0f), HAlign_Center);
	AddCanvasChild(Canvas, Dialog, FVector2D(28.0f, 650.0f), FVector2D(202.0f, 118.0f));

	UBorder* Reward = MakePanel(*WidgetTree);
	UVerticalBox* RewardStack = WidgetTree->ConstructWidget<UVerticalBox>();
	Reward->SetContent(RewardStack);
	UTextBlock* Check = MakeText(*WidgetTree, TEXT("OK"), 28.0f, FCodexUIColor::Primary(), TEXT("Bold"));
	Check->SetJustification(ETextJustify::Center);
	AddVBoxChild(*RewardStack, Check, FMargin(0.0f, 0.0f, 0.0f, 8.0f), HAlign_Center);
	UTextBlock* RewardTitle = MakeText(*WidgetTree, TEXT("아이템을 획득했습니다!"), FCodexUIFontSize::Caption, FCodexUIColor::BgTealDeep(), TEXT("Bold"));
	RewardTitle->SetJustification(ETextJustify::Center);
	AddVBoxChild(*RewardStack, RewardTitle, FMargin(0.0f, 0.0f, 0.0f, 12.0f));
	UHorizontalBox* RewardRow = WidgetTree->ConstructWidget<UHorizontalBox>();
	if (UTexture2D* ChestTexture = LoadSourceTexture(TEXT("reward-chest.png")))
	{
		UImage* Chest = WidgetTree->ConstructWidget<UImage>();
		Chest->SetBrushFromTexture(ChestTexture, true);
		AddHBoxChild(*RewardRow, Sized(*WidgetTree, Chest, 54.0f, 54.0f), FMargin(0.0f, 0.0f, 12.0f, 0.0f));
	}
	UVerticalBox* RewardText = WidgetTree->ConstructWidget<UVerticalBox>();
	AddVBoxChild(*RewardText, MakeText(*WidgetTree, TEXT("보급 상자"), FCodexUIFontSize::Body, FCodexUIColor::TextPrimary(), TEXT("Bold")));
	AddVBoxChild(*RewardText, MakeText(*WidgetTree, TEXT("x 1"), FCodexUIFontSize::Body, FCodexUIColor::TextPrimary()));
	AddHBoxChild(*RewardRow, RewardText, FMargin(0.0f), 1.0f);
	AddVBoxChild(*RewardStack, RewardRow, FMargin(0.0f, 0.0f, 0.0f, 12.0f));
	UButton* RewardButton = MakeButton(*WidgetTree, TEXT("확인"), ECodexUIButtonKind::Primary);
	RewardButton->OnClicked.AddDynamic(this, &ThisClass::HandleOpenRewardPopup);
	AddVBoxChild(*RewardStack, Sized(*WidgetTree, RewardButton, 110.0f, 36.0f), FMargin(0.0f), HAlign_Center);
	AddCanvasChild(Canvas, Reward, FVector2D(28.0f, 780.0f), FVector2D(202.0f, 200.0f));
}

void UCodexUIKitDemoWidget::BuildMainPanels(UCanvasPanel& Canvas)
{
	AddCanvasChild(Canvas, MakeText(*WidgetTree, TEXT("PANEL / WINDOW EXAMPLE"), FCodexUIFontSize::H3, FCodexUIColor::BgTealDeep(), TEXT("Bold")), FVector2D(258.0f, 330.0f), FVector2D(260.0f, 24.0f));
	UBorder* ItemPanel = MakePanel(*WidgetTree);
	UVerticalBox* ItemStack = WidgetTree->ConstructWidget<UVerticalBox>();
	ItemPanel->SetContent(ItemStack);
	UHorizontalBox* ItemHeader = WidgetTree->ConstructWidget<UHorizontalBox>();
	AddHBoxChild(*ItemHeader, MakeText(*WidgetTree, TEXT("보급 상자"), FCodexUIFontSize::H2, FCodexUIColor::TextPrimary(), TEXT("Bold")), FMargin(0.0f), 1.0f);
	AddHBoxChild(*ItemHeader, MakeText(*WidgetTree, TEXT("X"), FCodexUIFontSize::H2, FCodexUIColor::TextMuted(), TEXT("Bold")));
	AddVBoxChild(*ItemStack, ItemHeader, FMargin(0.0f, 0.0f, 0.0f, 16.0f));
	UHorizontalBox* ItemBody = WidgetTree->ConstructWidget<UHorizontalBox>();
	if (UTexture2D* ChestTexture = LoadSourceTexture(TEXT("reward-chest.png")))
	{
		UImage* Chest = WidgetTree->ConstructWidget<UImage>();
		Chest->SetBrushFromTexture(ChestTexture, true);
		AddHBoxChild(*ItemBody, Sized(*WidgetTree, Chest, 176.0f, 270.0f), FMargin(0.0f, 0.0f, 16.0f, 0.0f));
	}
	UVerticalBox* ItemText = WidgetTree->ConstructWidget<UVerticalBox>();
	AddVBoxChild(*ItemText, MakeTag(*WidgetTree, TEXT("일반"), FCodexUIColor::SurfaceSunken()), FMargin(0.0f, 0.0f, 0.0f, 18.0f), HAlign_Left);
	AddVBoxChild(*ItemText, MakeText(*WidgetTree, TEXT("낡은 보급 상자"), FCodexUIFontSize::H3, FCodexUIColor::TextPrimary(), TEXT("Bold")), FMargin(0.0f, 0.0f, 0.0f, 18.0f));
	UTextBlock* ItemDesc = MakeText(*WidgetTree, TEXT("어디선가 굴러다니던 낡은 상자.\n무언가 유용한 물건이 들어있을지도 모른다."), FCodexUIFontSize::Caption, FCodexUIColor::TextSecondary());
	ItemDesc->SetWrapTextAt(120.0f);
	AddVBoxChild(*ItemText, ItemDesc, FMargin(0.0f, 0.0f, 0.0f, 38.0f));
	AddVBoxChild(*ItemText, MakeText(*WidgetTree, TEXT("보유 수량  12"), FCodexUIFontSize::Caption, FCodexUIColor::TextMuted()), FMargin(0.0f, 0.0f, 0.0f, 12.0f));
	UButton* UseButton = MakeButton(*WidgetTree, TEXT("사용"), ECodexUIButtonKind::Primary);
	UseButton->OnClicked.AddDynamic(this, &ThisClass::HandleOpenActionPopup);
	AddVBoxChild(*ItemText, Sized(*WidgetTree, UseButton, 132.0f, 40.0f));
	AddHBoxChild(*ItemBody, ItemText, FMargin(0.0f), 1.0f);
	AddVBoxChild(*ItemStack, ItemBody);
	AddCanvasChild(Canvas, ItemPanel, FVector2D(248.0f, 354.0f), FVector2D(345.0f, 356.0f));

	AddCanvasChild(Canvas, MakeText(*WidgetTree, TEXT("QUEST LIST"), FCodexUIFontSize::H3, FCodexUIColor::BgTealDeep(), TEXT("Bold")), FVector2D(620.0f, 326.0f), FVector2D(160.0f, 24.0f));
	UBorder* QuestListPanel = MakePanel(*WidgetTree);
	UVerticalBox* QuestList = WidgetTree->ConstructWidget<UVerticalBox>();
	QuestListPanel->SetContent(QuestList);
	AddVBoxChild(*QuestList, MakeText(*WidgetTree, TEXT("진행 중 (2)                                      v"), FCodexUIFontSize::Body, FCodexUIColor::TextPrimary(), TEXT("Bold")), FMargin(0.0f, 0.0f, 0.0f, 10.0f));
	const FString QuestRows[] =
	{
		TEXT("!   구조 복귀 확인\n     구급 카트에 실려 병원 돌아오기        0/1"),
		TEXT("!   고장 난 스위퍼 수리\n     스위퍼 부품 수집                         0/3"),
		TEXT("완료 가능 (1)                                v"),
		TEXT("!   식량 부족\n     식량을 루나에게 전달하기"),
		TEXT("완료 (12)                                    v"),
		TEXT("실패 (1)                                    v")
	};
	for (int32 Index = 0; Index < UE_ARRAY_COUNT(QuestRows); ++Index)
	{
		UBorder* Row = WidgetTree->ConstructWidget<UBorder>();
		Row->SetPadding(FMargin(12.0f, 10.0f));
		Row->SetBrush(FCodexUIStyle::RoundedBrush(Index == 0 ? FLinearColor(0.76f, 0.90f, 0.94f, 1.0f) : FCodexUIColor::SurfaceRaised(), FCodexUIRadius::MD, FCodexUIColor::BorderSoft(), 1.0f));
		Row->SetContent(MakeText(*WidgetTree, QuestRows[Index], Index == 0 ? FCodexUIFontSize::Caption : FCodexUIFontSize::Body, FCodexUIColor::TextPrimary(), Index == 0 ? FName(TEXT("Bold")) : NAME_None));
		AddVBoxChild(*QuestList, Row, FMargin(0.0f, 0.0f, 0.0f, 8.0f));
	}
	AddCanvasChild(Canvas, QuestListPanel, FVector2D(610.0f, 347.0f), FVector2D(258.0f, 420.0f));

	UBorder* QuestDetail = MakePanel(*WidgetTree);
	UVerticalBox* Detail = WidgetTree->ConstructWidget<UVerticalBox>();
	QuestDetail->SetContent(Detail);
	UHorizontalBox* DetailHeader = WidgetTree->ConstructWidget<UHorizontalBox>();
	AddHBoxChild(*DetailHeader, MakeText(*WidgetTree, TEXT("구조 복귀 확인"), FCodexUIFontSize::H2, FCodexUIColor::TextPrimary(), TEXT("Bold")), FMargin(0.0f), 1.0f);
	AddHBoxChild(*DetailHeader, MakeTag(*WidgetTree, TEXT("진행 중"), FCodexUIColor::InfoDark()));
	AddVBoxChild(*Detail, DetailHeader, FMargin(0.0f, 0.0f, 0.0f, 28.0f));
	UTextBlock* DetailText = MakeText(*WidgetTree, TEXT("작전 중 이상 상태가 감지되면 구조 카트 전송 절차에\n따라 병원으로 돌아옵니다."), FCodexUIFontSize::Caption, FCodexUIColor::TextSecondary());
	DetailText->SetWrapTextAt(230.0f);
	AddVBoxChild(*Detail, DetailText, FMargin(0.0f, 0.0f, 0.0f, 32.0f));
	AddVBoxChild(*Detail, MakeText(*WidgetTree, TEXT("목표"), FCodexUIFontSize::Body, FCodexUIColor::TextPrimary(), TEXT("Bold")), FMargin(0.0f, 0.0f, 0.0f, 12.0f));
	AddVBoxChild(*Detail, MakeText(*WidgetTree, TEXT("• 구급 카트에 실려 병원으로 돌아오기 (0/1)"), FCodexUIFontSize::Body, FCodexUIColor::TextPrimary()), FMargin(0.0f, 0.0f, 0.0f, 38.0f));
	AddVBoxChild(*Detail, MakeText(*WidgetTree, TEXT("보상"), FCodexUIFontSize::Body, FCodexUIColor::TextPrimary(), TEXT("Bold")), FMargin(0.0f, 0.0f, 0.0f, 12.0f));
	AddVBoxChild(*Detail, MakeText(*WidgetTree, TEXT("코인 50"), FCodexUIFontSize::Body, FCodexUIColor::TextPrimary()), FMargin(0.0f, 0.0f, 0.0f, 66.0f));
	UButton* RouteButton = MakeButton(*WidgetTree, TEXT("길찾기 시작"), ECodexUIButtonKind::Info);
	RouteButton->OnClicked.AddDynamic(this, &ThisClass::HandleOpenActionPopup);
	AddVBoxChild(*Detail, Sized(*WidgetTree, RouteButton, 126.0f, 44.0f), FMargin(0.0f), HAlign_Center);
	AddCanvasChild(Canvas, QuestDetail, FVector2D(878.0f, 345.0f), FVector2D(280.0f, 422.0f));

	AddCanvasChild(Canvas, MakeText(*WidgetTree, TEXT("INVENTORY GRID"), FCodexUIFontSize::H3, FCodexUIColor::BgTealDeep(), TEXT("Bold")), FVector2D(1186.0f, 320.0f), FVector2D(180.0f, 24.0f));
	UBorder* InventoryPanel = MakePanel(*WidgetTree);
	UVerticalBox* Inventory = WidgetTree->ConstructWidget<UVerticalBox>();
	InventoryPanel->SetContent(Inventory);
	UHorizontalBox* InvHeader = WidgetTree->ConstructWidget<UHorizontalBox>();
	AddHBoxChild(*InvHeader, MakeText(*WidgetTree, TEXT("인벤토리"), FCodexUIFontSize::H3, FCodexUIColor::TextPrimary(), TEXT("Bold")), FMargin(0.0f), 1.0f);
	AddHBoxChild(*InvHeader, MakeText(*WidgetTree, TEXT("24/40   +"), FCodexUIFontSize::Body, FCodexUIColor::TextSecondary(), TEXT("Bold")));
	AddVBoxChild(*Inventory, InvHeader, FMargin(0.0f, 0.0f, 0.0f, 14.0f));
	for (int32 RowIndex = 0; RowIndex < 2; ++RowIndex)
	{
		UHorizontalBox* LockedRow = WidgetTree->ConstructWidget<UHorizontalBox>();
		for (int32 Column = 0; Column < 5; ++Column)
		{
			UBorder* Cell = MakePanel(*WidgetTree, FCodexUIColor::SurfaceSunken(), FCodexUIRadius::SM);
			Cell->SetContent(MakeText(*WidgetTree, TEXT("잠김"), FCodexUIFontSize::Caption, FCodexUIColor::TextMuted()));
			AddHBoxChild(*LockedRow, Sized(*WidgetTree, Cell, 56.0f, 52.0f), FMargin(0.0f, 0.0f, 7.0f, 7.0f));
		}
		AddVBoxChild(*Inventory, LockedRow);
	}
	UHorizontalBox* InvFooter = WidgetTree->ConstructWidget<UHorizontalBox>();
	AddHBoxChild(*InvFooter, MakeText(*WidgetTree, TEXT("CO 12,850"), FCodexUIFontSize::Body, FCodexUIColor::TextPrimary()), FMargin(0.0f), 1.0f);
	AddHBoxChild(*InvFooter, MakeText(*WidgetTree, TEXT("18.6 / 40kg      삭제"), FCodexUIFontSize::Body, FCodexUIColor::TextPrimary(), TEXT("Bold")));
	AddVBoxChild(*Inventory, InvFooter, FMargin(0.0f, 8.0f, 0.0f, 0.0f));
	AddCanvasChild(Canvas, InventoryPanel, FVector2D(1176.0f, 342.0f), FVector2D(340.0f, 416.0f));

	if (UCodexUIKitQuestBoardWidget* QuestBoard = CreateWidget<UCodexUIKitQuestBoardWidget>(GetOwningPlayer(), UCodexUIKitQuestBoardWidget::StaticClass()))
	{
		AddCanvasChild(Canvas, QuestBoard, FVector2D(610.0f, 345.0f), FVector2D(548.0f, 422.0f), 20);
	}

	if (UCodexUIKitInventoryGridWidget* InventoryGrid = CreateWidget<UCodexUIKitInventoryGridWidget>(GetOwningPlayer(), UCodexUIKitInventoryGridWidget::StaticClass()))
	{
		AddCanvasChild(Canvas, InventoryGrid, FVector2D(1176.0f, 342.0f), FVector2D(340.0f, 416.0f), 20);
	}
}

void UCodexUIKitDemoWidget::BuildBottomExamples(UCanvasPanel& Canvas)
{
	auto AddMetric = [this, &Canvas](const FString& Title, const FString& Icon, float Percent, const FLinearColor& Color, const FVector2D Pos, const FString& Value)
	{
		UVerticalBox* Stack = WidgetTree->ConstructWidget<UVerticalBox>();
		AddVBoxChild(*Stack, MakeText(*WidgetTree, Title, FCodexUIFontSize::Caption, FCodexUIColor::TextPrimary(), TEXT("Bold")), FMargin(0.0f, 0.0f, 0.0f, 8.0f));
		UHorizontalBox* Row = WidgetTree->ConstructWidget<UHorizontalBox>();
		AddHBoxChild(*Row, MakeText(*WidgetTree, Icon, 22.0f, Color, TEXT("Bold")), FMargin(0.0f, 0.0f, 10.0f, 0.0f));
		AddHBoxChild(*Row, Sized(*WidgetTree, MakeProgress(*WidgetTree, Percent, Color), 160.0f, 12.0f), FMargin(0.0f, 5.0f, 10.0f, 0.0f));
		AddHBoxChild(*Row, MakeText(*WidgetTree, Value, FCodexUIFontSize::Caption, FCodexUIColor::TextPrimary()));
		AddVBoxChild(*Stack, Row);
		AddCanvasChild(Canvas, Stack, Pos, FVector2D(220.0f, 56.0f));
	};
	AddMetric(TEXT("HEALTH BAR"), TEXT("HP"), 0.70f, FCodexUIColor::Primary(), FVector2D(276.0f, 796.0f), TEXT("360 / 500"));
	AddMetric(TEXT("ENERGY BAR"), TEXT("EN"), 0.78f, FCodexUIColor::Warning(), FVector2D(500.0f, 796.0f), TEXT("72 / 100"));
	AddMetric(TEXT("PROGRESS BAR"), TEXT(""), 0.60f, FCodexUIColor::InfoDark(), FVector2D(720.0f, 796.0f), TEXT("60%"));
	AddMetric(TEXT("SLIDER"), TEXT("BGM"), 0.68f, FCodexUIColor::BgTeal(), FVector2D(976.0f, 796.0f), TEXT("30"));

	UBorder* NoticePanel = MakePanel(*WidgetTree, FLinearColor::Transparent, 0.0f);
	NoticePanel->SetPadding(FMargin(0.0f));
	UVerticalBox* Notices = WidgetTree->ConstructWidget<UVerticalBox>();
	NoticePanel->SetContent(Notices);
	const FString NoticeTexts[] = { TEXT("OK  퀘스트를 완료했습니다.          x"), TEXT("!  인벤토리가 가득 찼습니다.       x"), TEXT("!  새로운 메시지가 도착했습니다.    x") };
	const FLinearColor NoticeColors[] = { FCodexUIColor::Primary(), FCodexUIColor::Warning(), FCodexUIColor::InfoDark() };
	for (int32 Index = 0; Index < UE_ARRAY_COUNT(NoticeTexts); ++Index)
	{
		UBorder* Notice = MakePanel(*WidgetTree, FLinearColor(1.0f, 0.96f, 0.78f, 0.92f), FCodexUIRadius::SM);
		Notice->SetPadding(FMargin(10.0f, 7.0f));
		Notice->SetContent(MakeText(*WidgetTree, NoticeTexts[Index], FCodexUIFontSize::Caption, NoticeColors[Index], TEXT("Bold")));
		AddVBoxChild(*Notices, Notice, FMargin(0.0f, 0.0f, 0.0f, 6.0f));
	}
	AddCanvasChild(Canvas, MakeText(*WidgetTree, TEXT("NOTIFICATION"), FCodexUIFontSize::Caption, FCodexUIColor::TextPrimary(), TEXT("Bold")), FVector2D(257.0f, 876.0f), FVector2D(140.0f, 20.0f));
	AddCanvasChild(Canvas, NoticePanel, FVector2D(255.0f, 895.0f), FVector2D(210.0f, 104.0f));

	AddCanvasChild(Canvas, MakeText(*WidgetTree, TEXT("HOTBAR"), FCodexUIFontSize::Caption, FCodexUIColor::TextInverse(), TEXT("Bold")), FVector2D(486.0f, 886.0f), FVector2D(100.0f, 20.0f));
	UHorizontalBox* Hotbar = WidgetTree->ConstructWidget<UHorizontalBox>();
	const FString Slots[] = { TEXT("1\n검\n30"), TEXT("3\n상\n5"), TEXT("3\n약\n2"), TEXT("4\n병\n7"), TEXT("5\n탄\n3"), TEXT("7\n망\n1"), TEXT("+") };
	for (const FString& SlotText : Slots)
	{
		UBorder* HotbarSlotBorder = MakePanel(*WidgetTree, FCodexUIColor::SurfaceRaised(), FCodexUIRadius::SM);
		UTextBlock* SlotLabel = MakeText(*WidgetTree, SlotText, FCodexUIFontSize::Caption, FCodexUIColor::TextPrimary(), TEXT("Bold"));
		SlotLabel->SetJustification(ETextJustify::Center);
		HotbarSlotBorder->SetContent(SlotLabel);
		AddHBoxChild(*Hotbar, Sized(*WidgetTree, HotbarSlotBorder, FCodexUISize::HotbarCell, FCodexUISize::HotbarCell), FMargin(0.0f, 0.0f, 8.0f, 0.0f));
	}
	AddCanvasChild(Canvas, Hotbar, FVector2D(486.0f, 906.0f), FVector2D(516.0f, 64.0f));

	if (UTexture2D* MinimapTexture = LoadSourceTexture(TEXT("minimap-preview.png")))
	{
		UImage* Minimap = WidgetTree->ConstructWidget<UImage>();
		Minimap->SetBrushFromTexture(MinimapTexture, true);
		AddCanvasChild(Canvas, MakeText(*WidgetTree, TEXT("MINI MAP"), FCodexUIFontSize::Caption, FCodexUIColor::BgTealDeep(), TEXT("Bold")), FVector2D(1048.0f, 876.0f), FVector2D(100.0f, 18.0f));
		AddCanvasChild(Canvas, Sized(*WidgetTree, Minimap, 132.0f, 132.0f), FVector2D(1042.0f, 892.0f), FVector2D(132.0f, 132.0f));
		AddCanvasChild(Canvas, MakeText(*WidgetTree, TEXT("+\n-"), 26.0f, FCodexUIColor::TextPrimary(), TEXT("Bold")), FVector2D(1178.0f, 945.0f), FVector2D(40.0f, 70.0f));
	}

	AddCanvasChild(Canvas, MakeText(*WidgetTree, TEXT("TOOLTIP EXAMPLE"), FCodexUIFontSize::Caption, FCodexUIColor::TextPrimary(), TEXT("Bold")), FVector2D(1224.0f, 788.0f), FVector2D(160.0f, 20.0f));
	UBorder* Tooltip = MakePanel(*WidgetTree);
	UVerticalBox* TooltipStack = WidgetTree->ConstructWidget<UVerticalBox>();
	Tooltip->SetContent(TooltipStack);
	UHorizontalBox* TooltipHeader = WidgetTree->ConstructWidget<UHorizontalBox>();
	if (UTexture2D* CrystalTexture = LoadSourceTexture(TEXT("data-crystal.png")))
	{
		UImage* Crystal = WidgetTree->ConstructWidget<UImage>();
		Crystal->SetBrushFromTexture(CrystalTexture, true);
		AddHBoxChild(*TooltipHeader, Sized(*WidgetTree, Crystal, 78.0f, 86.0f), FMargin(0.0f, 0.0f, 14.0f, 0.0f));
	}
	UVerticalBox* TooltipText = WidgetTree->ConstructWidget<UVerticalBox>();
	AddVBoxChild(*TooltipText, MakeText(*WidgetTree, TEXT("데이터 크리스탈"), FCodexUIFontSize::H3, FCodexUIColor::TextPrimary(), TEXT("Bold")));
	UTextBlock* TooltipDesc = MakeText(*WidgetTree, TEXT("고대 문명의 유적에서 발견되는\n신비한 크리스탈. 분석 장비로\n스캔할 수 있다."), FCodexUIFontSize::Caption, FCodexUIColor::TextSecondary());
	TooltipDesc->SetWrapTextAt(170.0f);
	AddVBoxChild(*TooltipText, TooltipDesc, FMargin(0.0f, 14.0f, 0.0f, 0.0f));
	AddHBoxChild(*TooltipHeader, TooltipText, FMargin(0.0f), 1.0f);
	AddHBoxChild(*TooltipHeader, MakeTag(*WidgetTree, TEXT("희귀"), FCodexUIColor::Info()), FMargin(10.0f, 0.0f, 0.0f, 0.0f));
	AddVBoxChild(*TooltipStack, TooltipHeader, FMargin(0.0f, 0.0f, 0.0f, 22.0f));
	AddVBoxChild(*TooltipStack, MakeText(*WidgetTree, TEXT("판매가                                      CO 150"), FCodexUIFontSize::Caption, FCodexUIColor::TextPrimary(), TEXT("Bold")));
	AddCanvasChild(Canvas, Tooltip, FVector2D(1222.0f, 806.0f), FVector2D(292.0f, 193.0f));

	if (UCodexUIKitControlsPanelWidget* ControlsPanel = CreateWidget<UCodexUIKitControlsPanelWidget>(GetOwningPlayer(), UCodexUIKitControlsPanelWidget::StaticClass()))
	{
		AddCanvasChild(Canvas, ControlsPanel, FVector2D(248.0f, 778.0f), FVector2D(930.0f, 218.0f), 20);
	}
}

UTexture2D* UCodexUIKitDemoWidget::LoadSourceTexture(const FString& FileName)
{
	const FString FullPath = FPaths::ProjectContentDir() / TEXT("UI/SourceArt") / FileName;
	TArray<uint8> CompressedData;
	if (!FFileHelper::LoadFileToArray(CompressedData, *FullPath))
	{
		return nullptr;
	}

	IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(TEXT("ImageWrapper"));
	TSharedPtr<IImageWrapper> ImageWrapper = ImageWrapperModule.CreateImageWrapper(EImageFormat::PNG);
	if (!ImageWrapper.IsValid() || !ImageWrapper->SetCompressed(CompressedData.GetData(), CompressedData.Num()))
	{
		return nullptr;
	}

	TArray64<uint8> RawData;
	if (!ImageWrapper->GetRaw(ERGBFormat::BGRA, 8, RawData))
	{
		return nullptr;
	}

	UTexture2D* Texture = UTexture2D::CreateTransient(ImageWrapper->GetWidth(), ImageWrapper->GetHeight(), PF_B8G8R8A8);
	if (!Texture || !Texture->GetPlatformData() || Texture->GetPlatformData()->Mips.Num() == 0)
	{
		return nullptr;
	}

	void* TextureData = Texture->GetPlatformData()->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
	FMemory::Memcpy(TextureData, RawData.GetData(), RawData.Num());
	Texture->GetPlatformData()->Mips[0].BulkData.Unlock();
	Texture->SRGB = true;
	Texture->UpdateResource();

	LoadedSourceTextures.Add(Texture);
	return Texture;
}

void UCodexUIKitDemoWidget::ShowPopup(const FText& Title, const FText& Message, const FText& Confirm, const FText& Cancel, bool bShowCancel)
{
	UCodexUIKitPopupWidget* Popup = CreateWidget<UCodexUIKitPopupWidget>(GetOwningPlayer(), UCodexUIKitPopupWidget::StaticClass());
	if (!Popup)
	{
		return;
	}

	Popup->ConfigurePopup(Title, Message, Confirm, Cancel, bShowCancel);
	Popup->AddToViewport(100);
}

void UCodexUIKitDemoWidget::HandleOpenConfirmPopup()
{
	ShowPopup(
		NSLOCTEXT("CodexUIKit", "ConfirmPopupTitle", "이동 확인"),
		NSLOCTEXT("CodexUIKit", "ConfirmPopupMessage", "동작을 시뮬레이션했습니다. 외부 게임 상태는 변경하지 않습니다."),
		NSLOCTEXT("CodexUIKit", "ConfirmPopupConfirm", "확인"),
		NSLOCTEXT("CodexUIKit", "ConfirmPopupCancel", "취소"),
		true);
}

void UCodexUIKitDemoWidget::HandleOpenRewardPopup()
{
	ShowPopup(
		NSLOCTEXT("CodexUIKit", "RewardPopupTitle", "획득 완료"),
		NSLOCTEXT("CodexUIKit", "RewardPopupMessage", "보급 상자를 획득한 것처럼 표시했습니다."),
		NSLOCTEXT("CodexUIKit", "RewardPopupConfirm", "닫기"),
		FText::GetEmpty(),
		false);
}

void UCodexUIKitDemoWidget::HandleOpenActionPopup()
{
	ShowPopup(
		NSLOCTEXT("CodexUIKit", "ActionPopupTitle", "데모 동작"),
		NSLOCTEXT("CodexUIKit", "ActionPopupMessage", "버튼 동작을 시뮬레이션했습니다. 확인하면 팝업만 닫힙니다."),
		NSLOCTEXT("CodexUIKit", "ActionPopupConfirm", "확인"),
		NSLOCTEXT("CodexUIKit", "ActionPopupCancel", "취소"),
		true);
}
