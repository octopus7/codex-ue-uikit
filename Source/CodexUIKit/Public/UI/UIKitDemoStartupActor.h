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

	static UUserWidget* CreateDemoWidgetForPlayer(APlayerController* PlayerController, TSubclassOf<UUserWidget> WidgetClass, int32 ZOrder);

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY(EditAnywhere, Category = "Codex UIKit")
	TSubclassOf<UUserWidget> DemoWidgetClass;

	UPROPERTY(EditAnywhere, Category = "Codex UIKit")
	int32 ZOrder = 0;

	UPROPERTY()
	TObjectPtr<UUserWidget> DemoWidget;
};
