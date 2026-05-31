#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CodexUIKitStartupWidget.generated.h"

class UCodexUIKitDemoWidget;
class UCodexUIKitPopupWidget;

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

	UFUNCTION()
	void HandleOpenDemoClicked();

	UFUNCTION()
	void HandleConfirmPopupClicked();

	UFUNCTION()
	void HandleRewardPopupClicked();

	UFUNCTION()
	void HandleCloseClicked();

	void ShowPopup(const FText& Title, const FText& Message, const FText& Confirm, const FText& Cancel, bool bShowCancel);
};
