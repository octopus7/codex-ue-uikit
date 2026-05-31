#include "UI/CodexUIKitToastWidget.h"

#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "UI/CodexUIStyle.h"

void UCodexUIKitToastWidget::ConfigureToast(FText InTitle, FText InMessage, float InDuration)
{
	ToastTitle = MoveTemp(InTitle);
	ToastMessage = MoveTemp(InMessage);
	Duration = FMath::Max(InDuration, 0.1f);
	RefreshText();
}

void UCodexUIKitToastWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (CloseButton)
	{
		CloseButton->OnClicked.AddDynamic(this, &ThisClass::HandleDismiss);
	}
}

void UCodexUIKitToastWidget::NativeConstruct()
{
	Super::NativeConstruct();
	FCodexUIStyle::ApplyFontToTextBlocks(GetRootWidget());

	RefreshText();
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
