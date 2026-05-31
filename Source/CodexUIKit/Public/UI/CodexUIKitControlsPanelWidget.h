#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CodexUIKitControlsPanelWidget.generated.h"

class UScrollBox;
class USlider;
class UTextBlock;

UCLASS()
class CODEXUIKIT_API UCodexUIKitControlsPanelWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Codex UIKit")
	void ResetDemoControls();

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;

private:
	UPROPERTY()
	TObjectPtr<UTextBlock> BgmValueLabel;

	UPROPERTY()
	TObjectPtr<UTextBlock> SfxValueLabel;

	UPROPERTY()
	TObjectPtr<UTextBlock> ZoomValueLabel;

	UPROPERTY()
	TObjectPtr<UTextBlock> NoticeHeaderLabel;

	UPROPERTY()
	TObjectPtr<UScrollBox> NoticeScrollBox;

	UPROPERTY()
	TArray<FString> Notices;

	float BgmValue = 0.68f;
	float SfxValue = 0.74f;
	float ZoomValue = 0.38f;
	int32 NoticeSequence = 0;

	void SeedNoticesIfNeeded();
	void RefreshSliderLabels();
	void RefreshNotices();
	void AddNotice(const FString& Notice);

	UFUNCTION()
	void HandleBgmChanged(float NewValue);

	UFUNCTION()
	void HandleSfxChanged(float NewValue);

	UFUNCTION()
	void HandleZoomChanged(float NewValue);

	UFUNCTION()
	void HandleAddNotice();

	UFUNCTION()
	void HandleScrollNoticeTop();

	UFUNCTION()
	void HandleScrollNoticeBottom();

	UFUNCTION()
	void HandleResetControls();
};
