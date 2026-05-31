#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CodexUIKitStartupWidget.generated.h"

class UCodexUIKitDemoWidget;
class UCodexUIKitPopupWidget;
class UCodexUIKitStandalonePanelWidget;
class UCodexUIKitToastWidget;

UCLASS()
class CODEXUIKIT_API UCodexUIKitStartupWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;

private:
	UPROPERTY()
	TObjectPtr<UCodexUIKitDemoWidget> ActiveDemoWidget;

	UPROPERTY()
	TObjectPtr<UCodexUIKitPopupWidget> ActivePopupWidget;

	UPROPERTY()
	TObjectPtr<UCodexUIKitStandalonePanelWidget> ActiveStandalonePanelWidget;

	UPROPERTY()
	TObjectPtr<UCodexUIKitToastWidget> ActiveToastWidget;

	int32 ToastSequence = 0;

	UFUNCTION()
	void HandleOpenDemoClicked();

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
