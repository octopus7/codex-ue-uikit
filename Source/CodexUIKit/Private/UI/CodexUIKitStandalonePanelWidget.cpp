#include "UI/CodexUIKitStandalonePanelWidget.h"

#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "UI/CodexUIStyle.h"

void UCodexUIKitStandalonePanelWidget::ConfigurePanel(FText InTitle, TSubclassOf<UUserWidget> InContentWidgetClass, FVector2D InContentSize)
{
	PanelTitle = MoveTemp(InTitle);
	ContentWidgetClass = InContentWidgetClass;
	ContentSize = InContentSize;
	SyncPanel();
}

void UCodexUIKitStandalonePanelWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (CloseButton)
	{
		CloseButton->OnClicked.AddDynamic(this, &ThisClass::HandleCloseClicked);
	}
}

void UCodexUIKitStandalonePanelWidget::NativeConstruct()
{
	Super::NativeConstruct();
	FCodexUIStyle::ApplyFontToTextBlocks(GetRootWidget());
	SyncPanel();
}

void UCodexUIKitStandalonePanelWidget::RebuildContent()
{
	if (!ContentWidgetClass)
	{
		ContentWidget = nullptr;
		return;
	}

	ContentWidget = CreateWidget<UUserWidget>(GetOwningPlayer(), ContentWidgetClass);
}

void UCodexUIKitStandalonePanelWidget::SyncPanel()
{
	if (TitleTextBlock)
	{
		TitleTextBlock->SetText(PanelTitle.IsEmpty()
			? NSLOCTEXT("CodexUIKit", "StandalonePanelDefaultTitle", "패널 테스트")
			: PanelTitle);
	}

	if (!ContentHost)
	{
		return;
	}

	ContentHost->SetWidthOverride(ContentSize.X);
	ContentHost->SetHeightOverride(ContentSize.Y);
	ContentHost->ClearChildren();

	RebuildContent();
	if (ContentWidget)
	{
		ContentHost->AddChild(ContentWidget);
	}
}

void UCodexUIKitStandalonePanelWidget::HandleCloseClicked()
{
	RemoveFromParent();
}
