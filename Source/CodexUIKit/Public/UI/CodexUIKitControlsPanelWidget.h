#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CodexUIKitControlsPanelWidget.generated.h"

class UScrollBox;
class USlider;
class UTextBlock;
class UButton;

UCLASS()
class CODEXUIKIT_API UCodexUIKitControlsPanelWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Codex UIKit")
	void ResetDemoControls();

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;

private:
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> BgmValueLabel;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> SfxValueLabel;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> ZoomValueLabel;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> NoticeHeaderLabel;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UScrollBox> NoticeScrollBox;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<USlider> BgmSlider;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<USlider> SfxSlider;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<USlider> ZoomSlider;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> AddNoticeButton;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> ScrollTopButton;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> ScrollBottomButton;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> ResetButton;

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
