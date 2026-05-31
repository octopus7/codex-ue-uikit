#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TimerManager.h"
#include "CodexUIKitToastWidget.generated.h"

class UTextBlock;

UCLASS()
class CODEXUIKIT_API UCodexUIKitToastWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Codex UIKit")
	void ConfigureToast(FText InTitle, FText InMessage, float InDuration = 3.0f);

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:
	UPROPERTY()
	FText ToastTitle;

	UPROPERTY()
	FText ToastMessage;

	UPROPERTY()
	TObjectPtr<UTextBlock> TitleTextBlock;

	UPROPERTY()
	TObjectPtr<UTextBlock> MessageTextBlock;

	FTimerHandle AutoDismissTimerHandle;
	float Duration = 3.0f;

	void RefreshText();

	UFUNCTION()
	void HandleDismiss();
};
