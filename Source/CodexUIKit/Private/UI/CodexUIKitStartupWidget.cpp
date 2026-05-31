#include "UI/CodexUIKitStartupWidget.h"

#include "Components/Button.h"
#include "UI/CodexUIKitControlsPanelWidget.h"
#include "UI/CodexUIKitInventoryGridWidget.h"
#include "UI/CodexUIKitPopupWidget.h"
#include "UI/CodexUIKitQuestBoardWidget.h"
#include "UI/CodexUIKitStandalonePanelWidget.h"
#include "UI/CodexUIKitToastWidget.h"
#include "UI/CodexUIStyle.h"

namespace
{
template <typename TWidget>
TSubclassOf<TWidget> ResolveWidgetClass(const TCHAR* WidgetBlueprintPath)
{
	if (UClass* WidgetBlueprintClass = LoadClass<TWidget>(nullptr, WidgetBlueprintPath))
	{
		return WidgetBlueprintClass;
	}

	return TWidget::StaticClass();
}
}

void UCodexUIKitStartupWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	FCodexUIStyle::ApplyFontToTextBlocks(GetRootWidget());

	if (QuestBoardButton)
	{
		QuestBoardButton->OnClicked.AddDynamic(this, &ThisClass::HandleQuestBoardClicked);
	}
	if (InventoryButton)
	{
		InventoryButton->OnClicked.AddDynamic(this, &ThisClass::HandleInventoryClicked);
	}
	if (ControlsButton)
	{
		ControlsButton->OnClicked.AddDynamic(this, &ThisClass::HandleControlsClicked);
	}
	if (ConfirmPopupButton)
	{
		ConfirmPopupButton->OnClicked.AddDynamic(this, &ThisClass::HandleConfirmPopupClicked);
	}
	if (RewardPopupButton)
	{
		RewardPopupButton->OnClicked.AddDynamic(this, &ThisClass::HandleRewardPopupClicked);
	}
	if (ToastButton)
	{
		ToastButton->OnClicked.AddDynamic(this, &ThisClass::HandleToastClicked);
	}
	if (CloseButton)
	{
		CloseButton->OnClicked.AddDynamic(this, &ThisClass::HandleCloseClicked);
	}
}

void UCodexUIKitStartupWidget::HandleQuestBoardClicked()
{
	const TSubclassOf<UUserWidget> QuestBoardClass = ResolveWidgetClass<UCodexUIKitQuestBoardWidget>(
		TEXT("/Game/UI/WBP/WBP_CodexUIKitQuestBoard.WBP_CodexUIKitQuestBoard_C"));

	ShowStandalonePanel(
		NSLOCTEXT("CodexUIKit", "StartupQuestBoardTitle", "퀘스트 보드 테스트"),
		QuestBoardClass,
		FVector2D(1120.0f, 640.0f));
}

void UCodexUIKitStartupWidget::HandleInventoryClicked()
{
	const TSubclassOf<UUserWidget> InventoryClass = ResolveWidgetClass<UCodexUIKitInventoryGridWidget>(
		TEXT("/Game/UI/WBP/WBP_CodexUIKitInventoryGrid.WBP_CodexUIKitInventoryGrid_C"));

	ShowStandalonePanel(
		NSLOCTEXT("CodexUIKit", "StartupInventoryTitle", "인벤토리 패널 테스트"),
		InventoryClass,
		FVector2D(340.0f, 416.0f));
}

void UCodexUIKitStartupWidget::HandleControlsClicked()
{
	const TSubclassOf<UUserWidget> ControlsClass = ResolveWidgetClass<UCodexUIKitControlsPanelWidget>(
		TEXT("/Game/UI/WBP/WBP_CodexUIKitControlsPanel.WBP_CodexUIKitControlsPanel_C"));

	ShowStandalonePanel(
		NSLOCTEXT("CodexUIKit", "StartupControlsTitle", "컨트롤/슬라이더 테스트"),
		ControlsClass,
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

	const TSubclassOf<UCodexUIKitStandalonePanelWidget> PanelClass = ResolveWidgetClass<UCodexUIKitStandalonePanelWidget>(
		TEXT("/Game/UI/WBP/WBP_CodexUIKitStandalonePanel.WBP_CodexUIKitStandalonePanel_C"));

	ActiveStandalonePanelWidget = CreateWidget<UCodexUIKitStandalonePanelWidget>(GetOwningPlayer(), PanelClass);
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

	const TSubclassOf<UCodexUIKitPopupWidget> PopupClass = ResolveWidgetClass<UCodexUIKitPopupWidget>(
		TEXT("/Game/UI/WBP/WBP_CodexUIKitPopup.WBP_CodexUIKitPopup_C"));

	ActivePopupWidget = CreateWidget<UCodexUIKitPopupWidget>(GetOwningPlayer(), PopupClass);
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

	const TSubclassOf<UCodexUIKitToastWidget> ToastClass = ResolveWidgetClass<UCodexUIKitToastWidget>(
		TEXT("/Game/UI/WBP/WBP_CodexUIKitToast.WBP_CodexUIKitToast_C"));

	ActiveToastWidget = CreateWidget<UCodexUIKitToastWidget>(GetOwningPlayer(), ToastClass);
	if (!ActiveToastWidget)
	{
		return;
	}

	ActiveToastWidget->ConfigureToast(Title, Message, 3.5f);
	ActiveToastWidget->AddToViewport(60);
}
