#include "UI/UIKitDemoStartupActor.h"

#include "Blueprint/UserWidget.h"
#include "GameFramework/PlayerController.h"
#include "UI/CodexUIKitStartupWidget.h"

namespace
{
TSubclassOf<UUserWidget> ResolveStartupWidgetClass(TSubclassOf<UUserWidget> WidgetClass)
{
	if (WidgetClass)
	{
		return WidgetClass;
	}

	if (UClass* WidgetBlueprintClass = LoadClass<UCodexUIKitStartupWidget>(
		nullptr,
		TEXT("/Game/UI/WBP/WBP_CodexUIKitStartup.WBP_CodexUIKitStartup_C")))
	{
		return WidgetBlueprintClass;
	}

	return UCodexUIKitStartupWidget::StaticClass();
}
}

AUIKitDemoStartupActor::AUIKitDemoStartupActor()
{
	PrimaryActorTick.bCanEverTick = false;
}

UUserWidget* AUIKitDemoStartupActor::CreateStartupWidgetForPlayer(APlayerController* PlayerController, TSubclassOf<UUserWidget> WidgetClass, int32 ZOrder)
{
	if (!PlayerController)
	{
		return nullptr;
	}

	const TSubclassOf<UUserWidget> ResolvedClass = ResolveStartupWidgetClass(WidgetClass);
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
