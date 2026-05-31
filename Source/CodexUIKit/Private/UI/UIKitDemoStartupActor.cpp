#include "UI/UIKitDemoStartupActor.h"

#include "Blueprint/UserWidget.h"
#include "GameFramework/PlayerController.h"
#include "UI/CodexUIKitStartupWidget.h"

AUIKitDemoStartupActor::AUIKitDemoStartupActor()
{
	PrimaryActorTick.bCanEverTick = false;
	StartupWidgetClass = UCodexUIKitStartupWidget::StaticClass();
}

UUserWidget* AUIKitDemoStartupActor::CreateStartupWidgetForPlayer(APlayerController* PlayerController, TSubclassOf<UUserWidget> WidgetClass, int32 ZOrder)
{
	if (!PlayerController)
	{
		return nullptr;
	}

	TSubclassOf<UUserWidget> ResolvedClass = WidgetClass;
	if (!ResolvedClass)
	{
		ResolvedClass = UCodexUIKitStartupWidget::StaticClass();
	}
	UUserWidget* Widget = CreateWidget<UUserWidget>(PlayerController, ResolvedClass);
	if (!Widget)
	{
		return nullptr;
	}

	Widget->AddToViewport(ZOrder);

	FInputModeGameAndUI InputMode;
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	PlayerController->SetInputMode(InputMode);
	PlayerController->SetShowMouseCursor(true);
	return Widget;
}

void AUIKitDemoStartupActor::BeginPlay()
{
	Super::BeginPlay();

	if (GetNetMode() == NM_DedicatedServer)
	{
		return;
	}

	StartupWidget = CreateStartupWidgetForPlayer(GetWorld()->GetFirstPlayerController(), StartupWidgetClass, ZOrder);
}
