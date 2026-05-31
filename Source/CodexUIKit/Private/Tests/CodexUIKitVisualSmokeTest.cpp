#if WITH_DEV_AUTOMATION_TESTS

#include "AutomationBlueprintFunctionLibrary.h"
#include "AutomationScreenshotOptions.h"
#include "Blueprint/UserWidget.h"
#include "Engine/GameViewportClient.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "UI/CodexUIKitPopupWidget.h"
#include "UI/CodexUIKitQuestBoardWidget.h"
#include "UI/CodexUIKitStandalonePanelWidget.h"
#include "UI/CodexUIKitStartupWidget.h"
#include "UI/CodexUIKitToastWidget.h"

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

class FCodexUIKitVisualSmokeCommand final : public IAutomationLatentCommand
{
public:
	explicit FCodexUIKitVisualSmokeCommand(FAutomationTestBase* InTest)
		: Test(InTest)
		, StartTime(FPlatformTime::Seconds())
	{
	}

	virtual bool Update() override
	{
		if (!Test)
		{
			return true;
		}

		UWorld* World = AutomationCommon::GetAnyGameWorld();
		APlayerController* PlayerController = World ? World->GetFirstPlayerController() : nullptr;
		UGameViewportClient* ViewportClient = AutomationCommon::GetAnyGameViewportClient();

		if (!World || !PlayerController || !ViewportClient)
		{
			if (FPlatformTime::Seconds() - StartTime < 10.0)
			{
				return false;
			}

			Test->AddError(TEXT("Timed out waiting for a game world, player controller, and game viewport."));
			return true;
		}

		if (!StartupWidget.IsValid())
		{
			const TSubclassOf<UCodexUIKitStartupWidget> StartupClass = ResolveWidgetClass<UCodexUIKitStartupWidget>(
				TEXT("/Game/UI/WBP/WBP_CodexUIKitStartup.WBP_CodexUIKitStartup_C"));
			const TSubclassOf<UCodexUIKitStandalonePanelWidget> StandalonePanelClass = ResolveWidgetClass<UCodexUIKitStandalonePanelWidget>(
				TEXT("/Game/UI/WBP/WBP_CodexUIKitStandalonePanel.WBP_CodexUIKitStandalonePanel_C"));
			const TSubclassOf<UCodexUIKitPopupWidget> PopupClass = ResolveWidgetClass<UCodexUIKitPopupWidget>(
				TEXT("/Game/UI/WBP/WBP_CodexUIKitPopup.WBP_CodexUIKitPopup_C"));
			const TSubclassOf<UCodexUIKitToastWidget> ToastClass = ResolveWidgetClass<UCodexUIKitToastWidget>(
				TEXT("/Game/UI/WBP/WBP_CodexUIKitToast.WBP_CodexUIKitToast_C"));
			const TSubclassOf<UUserWidget> QuestBoardClass = ResolveWidgetClass<UCodexUIKitQuestBoardWidget>(
				TEXT("/Game/UI/WBP/WBP_CodexUIKitQuestBoard.WBP_CodexUIKitQuestBoard_C"));

			StartupWidget = CreateWidget<UCodexUIKitStartupWidget>(PlayerController, StartupClass);
			Test->TestNotNull(TEXT("Startup widget can be created"), StartupWidget.Get());

			PopupWidget = CreateWidget<UCodexUIKitPopupWidget>(PlayerController, PopupClass);
			Test->TestNotNull(TEXT("Popup widget can be created"), PopupWidget.Get());

			StandalonePanelWidget = CreateWidget<UCodexUIKitStandalonePanelWidget>(PlayerController, StandalonePanelClass);
			Test->TestNotNull(TEXT("Standalone panel widget can be created"), StandalonePanelWidget.Get());

			ToastWidget = CreateWidget<UCodexUIKitToastWidget>(PlayerController, ToastClass);
			Test->TestNotNull(TEXT("Toast widget can be created"), ToastWidget.Get());

			if (!StartupWidget.IsValid() || !PopupWidget.IsValid() || !StandalonePanelWidget.IsValid() || !ToastWidget.IsValid())
			{
				return true;
			}

			StartupWidget->AddToViewport(80);
			StandalonePanelWidget->ConfigurePanel(
				NSLOCTEXT("CodexUIKitTest", "VisualSmokeStandaloneQuestTitle", "자동화 퀘스트 보드"),
				QuestBoardClass,
				FVector2D(1120.0f, 640.0f));
			StandalonePanelWidget->AddToViewport(85);
			PopupWidget->ConfigurePopup(
				NSLOCTEXT("CodexUIKitTest", "VisualSmokePopupTitle", "자동화 팝업"),
				NSLOCTEXT("CodexUIKitTest", "VisualSmokePopupMessage", "자동화 테스트용 팝업입니다."),
				NSLOCTEXT("CodexUIKitTest", "VisualSmokePopupConfirm", "확인"),
				NSLOCTEXT("CodexUIKitTest", "VisualSmokePopupCancel", "취소"),
				true);
			PopupWidget->AddToViewport(90);
			ToastWidget->ConfigureToast(
				NSLOCTEXT("CodexUIKitTest", "VisualSmokeToastTitle", "자동화 토스트"),
				NSLOCTEXT("CodexUIKitTest", "VisualSmokeToastMessage", "토스트 메시지 테스트입니다."),
				10.0f);
			ToastWidget->AddToViewport(95);

			Test->TestTrue(TEXT("Startup widget is in viewport"), StartupWidget->IsInViewport());
			Test->TestTrue(TEXT("Standalone panel widget is in viewport"), StandalonePanelWidget->IsInViewport());
			Test->TestTrue(TEXT("Popup widget is in viewport"), PopupWidget->IsInViewport());
			Test->TestTrue(TEXT("Toast widget is in viewport"), ToastWidget->IsInViewport());

			WidgetCreateTime = FPlatformTime::Seconds();
			return false;
		}

		if (Phase == 0 && FPlatformTime::Seconds() - WidgetCreateTime < 0.5)
		{
			return false;
		}

		if (Phase == 0)
		{
			const bool bScreenshotRequested = TakeUiScreenshot(World, TEXT("CodexUIKit_StartupAndPopupSmoke"));
			Test->TestTrue(TEXT("Startup, standalone, and popup screenshot request succeeded"), bScreenshotRequested);

			if (PopupWidget.IsValid())
			{
				PopupWidget->RemoveFromParent();
				Test->TestFalse(TEXT("Popup widget can be removed from viewport"), PopupWidget->IsInViewport());
			}
			if (ToastWidget.IsValid())
			{
				ToastWidget->RemoveFromParent();
				Test->TestFalse(TEXT("Toast widget can be removed from viewport"), ToastWidget->IsInViewport());
			}
			if (StandalonePanelWidget.IsValid())
			{
				StandalonePanelWidget->RemoveFromParent();
				Test->TestFalse(TEXT("Standalone panel widget can be removed from viewport"), StandalonePanelWidget->IsInViewport());
			}
			if (StartupWidget.IsValid())
			{
				StartupWidget->RemoveFromParent();
				Test->TestFalse(TEXT("Startup widget can be removed from viewport"), StartupWidget->IsInViewport());
			}
			return true;
		}

		return true;
	}

private:
	bool TakeUiScreenshot(UWorld* World, const FString& Name) const
	{
		FAutomationScreenshotOptions ScreenshotOptions(EComparisonTolerance::Low);
		ScreenshotOptions.Resolution = FVector2D(1280.0, 720.0);
		ScreenshotOptions.Delay = 0.1f;
		ScreenshotOptions.FrameDelay = 2;
		return UAutomationBlueprintFunctionLibrary::TakeAutomationScreenshotOfUI_Immediate(World, Name, ScreenshotOptions);
	}

	FAutomationTestBase* Test = nullptr;
	double StartTime = 0.0;
	double WidgetCreateTime = 0.0;
	int32 Phase = 0;
	TWeakObjectPtr<UCodexUIKitStartupWidget> StartupWidget;
	TWeakObjectPtr<UCodexUIKitPopupWidget> PopupWidget;
	TWeakObjectPtr<UCodexUIKitStandalonePanelWidget> StandalonePanelWidget;
	TWeakObjectPtr<UCodexUIKitToastWidget> ToastWidget;
};
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCodexUIKitVisualSmokeTest,
	"Project.CodexUIKit.UI.VisualSmoke",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FCodexUIKitVisualSmokeTest::RunTest(const FString& Parameters)
{
	const bool bOpenedMap = AutomationOpenMap(TEXT("/Game/Maps/Basic"), true);
	TestTrue(TEXT("Basic map opens for UI visual smoke test"), bOpenedMap);
	if (!bOpenedMap)
	{
		return false;
	}

	ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(0.5f));
	ADD_LATENT_AUTOMATION_COMMAND(FCodexUIKitVisualSmokeCommand(this));
	return true;
}

#endif
