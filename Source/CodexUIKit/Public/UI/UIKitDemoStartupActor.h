#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "UIKitDemoStartupActor.generated.h"

class UUserWidget;

UCLASS(Blueprintable)
class CODEXUIKIT_API AUIKitDemoStartupActor : public AActor
{
	GENERATED_BODY()

public:
	AUIKitDemoStartupActor();

	static UUserWidget* CreateStartupWidgetForPlayer(APlayerController* PlayerController, TSubclassOf<UUserWidget> WidgetClass, int32 ZOrder);

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY(EditAnywhere, Category = "Codex UIKit")
	TSubclassOf<UUserWidget> StartupWidgetClass;

	UPROPERTY(EditAnywhere, Category = "Codex UIKit")
	int32 ZOrder = 0;

	UPROPERTY()
	TObjectPtr<UUserWidget> StartupWidget;
};
