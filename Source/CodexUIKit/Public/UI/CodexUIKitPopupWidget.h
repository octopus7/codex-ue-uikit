#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CodexUIKitPopupWidget.generated.h"

class UButton;
class UTextBlock;

UCLASS()
class CODEXUIKIT_API UCodexUIKitPopupWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Codex UIKit")
	void ConfigurePopup(
		const FText& InTitle,
		const FText& InMessage,
		const FText& InConfirmText,
		const FText& InCancelText,
		bool bInShowCancel);

protected:
	virtual void NativeConstruct() override;

private:
	UPROPERTY()
	TObjectPtr<UTextBlock> TitleLabel;

	UPROPERTY()
	TObjectPtr<UTextBlock> MessageLabel;

	UPROPERTY()
	TObjectPtr<UTextBlock> ConfirmLabel;

	UPROPERTY()
	TObjectPtr<UTextBlock> CancelLabel;

	UPROPERTY()
	TObjectPtr<UButton> ConfirmButton;

	UPROPERTY()
	TObjectPtr<UButton> CancelButton;

	UPROPERTY()
	TObjectPtr<UButton> CloseButton;

	FText TitleText = NSLOCTEXT("CodexUIKit", "PopupDefaultTitle", "확인");
	FText MessageText = NSLOCTEXT("CodexUIKit", "PopupDefaultMessage", "정말로 진행하시겠습니까?");
	FText ConfirmText = NSLOCTEXT("CodexUIKit", "PopupDefaultConfirm", "확인");
	FText CancelText = NSLOCTEXT("CodexUIKit", "PopupDefaultCancel", "취소");
	bool bShowCancel = true;

	void SyncText();

	UFUNCTION()
	void HandleCloseClicked();
};
