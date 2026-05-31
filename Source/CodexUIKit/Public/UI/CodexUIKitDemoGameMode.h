#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "CodexUIKitDemoGameMode.generated.h"

class UUserWidget;

UCLASS()
class CODEXUIKIT_API ACodexUIKitDemoGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ACodexUIKitDemoGameMode();

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY(EditDefaultsOnly, Category = "Codex UIKit")
	TSubclassOf<UUserWidget> DemoWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "Codex UIKit")
	int32 ZOrder = 0;

	UPROPERTY()
	TObjectPtr<UUserWidget> DemoWidget;
};
