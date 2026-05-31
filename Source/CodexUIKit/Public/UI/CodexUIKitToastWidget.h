#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TimerManager.h"
#include "CodexUIKitToastWidget.generated.h"

class UTextBlock;
class UButton;

UCLASS()
class CODEXUIKIT_API UCodexUIKitToastWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Codex UIKit")
	void ConfigureToast(FText InTitle, FText InMessage, float InDuration = 3.0f);

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:
	UPROPERTY()
	FText ToastTitle;

	UPROPERTY()
	FText ToastMessage;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> TitleTextBlock;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> MessageTextBlock;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> CloseButton;

	FTimerHandle AutoDismissTimerHandle;
	float Duration = 3.0f;

	void RefreshText();

	UFUNCTION()
	void HandleDismiss();
};
