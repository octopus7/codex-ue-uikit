#include "UI/CodexUIKitDemoGameMode.h"

#include "EngineUtils.h"
#include "UI/CodexUIKitDemoWidget.h"
#include "UI/UIKitDemoStartupActor.h"

ACodexUIKitDemoGameMode::ACodexUIKitDemoGameMode()
{
	DemoWidgetClass = UCodexUIKitDemoWidget::StaticClass();
}

void ACodexUIKitDemoGameMode::BeginPlay()
{
	Super::BeginPlay();

	for (TActorIterator<AUIKitDemoStartupActor> It(GetWorld()); It; ++It)
	{
		return;
	}

	DemoWidget = AUIKitDemoStartupActor::CreateDemoWidgetForPlayer(GetWorld()->GetFirstPlayerController(), DemoWidgetClass, ZOrder);
}
