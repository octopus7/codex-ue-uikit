#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CodexUIKitStartupWidget.generated.h"

class UCodexUIKitPopupWidget;
class UCodexUIKitStandalonePanelWidget;
class UCodexUIKitToastWidget;
class UButton;

UCLASS()
class CODEXUIKIT_API UCodexUIKitStartupWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeOnInitialized() override;

private:
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> QuestBoardButton;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> InventoryButton;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> ControlsButton;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> ConfirmPopupButton;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> RewardPopupButton;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> ToastButton;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> CloseButton;

	UPROPERTY()
	TObjectPtr<UCodexUIKitPopupWidget> ActivePopupWidget;

	UPROPERTY()
	TObjectPtr<UCodexUIKitStandalonePanelWidget> ActiveStandalonePanelWidget;

	UPROPERTY()
	TObjectPtr<UCodexUIKitToastWidget> ActiveToastWidget;

	int32 ToastSequence = 0;

	UFUNCTION()
	void HandleQuestBoardClicked();

	UFUNCTION()
	void HandleInventoryClicked();

	UFUNCTION()
	void HandleControlsClicked();

	UFUNCTION()
	void HandleConfirmPopupClicked();

	UFUNCTION()
	void HandleRewardPopupClicked();

	UFUNCTION()
	void HandleToastClicked();

	UFUNCTION()
	void HandleCloseClicked();

	void ShowStandalonePanel(const FText& Title, TSubclassOf<UUserWidget> ContentWidgetClass, FVector2D ContentSize);
	void ShowPopup(const FText& Title, const FText& Message, const FText& Confirm, const FText& Cancel, bool bShowCancel);
	void ShowToast(const FText& Title, const FText& Message);
};
