#include "UI/CodexUIKitStartupSubsystem.h"

#include "Blueprint/UserWidget.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "UI/CodexUIKitStartupWidget.h"

bool UCodexUIKitStartupSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	const UWorld* World = Cast<UWorld>(Outer);
	return World && World->IsGameWorld();
}

void UCodexUIKitStartupSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);

	if (InWorld.GetNetMode() == NM_DedicatedServer)
	{
		return;
	}

	APlayerController* PlayerController = InWorld.GetFirstPlayerController();
	if (!PlayerController)
	{
		return;
	}

	StartupWidget = CreateWidget<UCodexUIKitStartupWidget>(PlayerController, UCodexUIKitStartupWidget::StaticClass());
	if (!StartupWidget)
	{
		return;
	}

	StartupWidget->AddToViewport(20);

	FInputModeGameAndUI InputMode;
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	PlayerController->SetInputMode(InputMode);
	PlayerController->SetShowMouseCursor(true);
}
