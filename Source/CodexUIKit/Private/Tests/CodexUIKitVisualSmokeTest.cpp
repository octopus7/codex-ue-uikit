#if WITH_DEV_AUTOMATION_TESTS

#include "AutomationBlueprintFunctionLibrary.h"
#include "AutomationScreenshotOptions.h"
#include "Blueprint/UserWidget.h"
#include "Engine/GameViewportClient.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "UI/CodexUIKitDemoWidget.h"
#include "UI/CodexUIKitPopupWidget.h"
#include "UI/CodexUIKitStartupWidget.h"

namespace
{
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
			StartupWidget = CreateWidget<UCodexUIKitStartupWidget>(PlayerController, UCodexUIKitStartupWidget::StaticClass());
			Test->TestNotNull(TEXT("Startup widget can be created"), StartupWidget.Get());

			DemoWidget = CreateWidget<UCodexUIKitDemoWidget>(PlayerController, UCodexUIKitDemoWidget::StaticClass());
			Test->TestNotNull(TEXT("Demo widget can be created"), DemoWidget.Get());

			PopupWidget = CreateWidget<UCodexUIKitPopupWidget>(PlayerController, UCodexUIKitPopupWidget::StaticClass());
			Test->TestNotNull(TEXT("Popup widget can be created"), PopupWidget.Get());

			if (!StartupWidget.IsValid() || !DemoWidget.IsValid() || !PopupWidget.IsValid())
			{
				return true;
			}

			DemoWidget->AddToViewport(70);
			StartupWidget->AddToViewport(80);
			PopupWidget->ConfigurePopup(
				NSLOCTEXT("CodexUIKitTest", "VisualSmokePopupTitle", "자동화 팝업"),
				NSLOCTEXT("CodexUIKitTest", "VisualSmokePopupMessage", "자동화 테스트용 팝업입니다."),
				NSLOCTEXT("CodexUIKitTest", "VisualSmokePopupConfirm", "확인"),
				NSLOCTEXT("CodexUIKitTest", "VisualSmokePopupCancel", "취소"),
				true);
			PopupWidget->AddToViewport(90);

			Test->TestTrue(TEXT("Demo widget is in viewport"), DemoWidget->IsInViewport());
			Test->TestTrue(TEXT("Startup widget is in viewport"), StartupWidget->IsInViewport());
			Test->TestTrue(TEXT("Popup widget is in viewport"), PopupWidget->IsInViewport());

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
			Test->TestTrue(TEXT("Demo, startup, and popup screenshot request succeeded"), bScreenshotRequested);

			if (PopupWidget.IsValid())
			{
				PopupWidget->RemoveFromParent();
				Test->TestFalse(TEXT("Popup widget can be removed from viewport"), PopupWidget->IsInViewport());
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
	TWeakObjectPtr<UCodexUIKitDemoWidget> DemoWidget;
	TWeakObjectPtr<UCodexUIKitPopupWidget> PopupWidget;
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
