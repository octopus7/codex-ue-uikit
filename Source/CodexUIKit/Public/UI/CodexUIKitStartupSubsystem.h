#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "CodexUIKitStartupSubsystem.generated.h"

class UCodexUIKitStartupWidget;

UCLASS()
class CODEXUIKIT_API UCodexUIKitStartupSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;

private:
	UPROPERTY()
	TObjectPtr<UCodexUIKitStartupWidget> StartupWidget;
};
